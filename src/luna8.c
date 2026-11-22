// luna8.c
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "raylib.h"

#include "luna8_conf.h"
#include "luna8_api.h"
#include "luna8_script.h"
#include "luna8_save.h"
#include "luna8_icon.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

/* Синхронный XHR: читает URL, возвращает malloc-строку или NULL. */
EM_JS(char*, luna8_fetch_sync, (const char* url), {
    const xhr = new XMLHttpRequest();
    xhr.open('GET', UTF8ToString(url), false);
    try { xhr.send(null); } catch (e) { return 0; }
    if (xhr.status !== 200) return 0;
    const text = xhr.responseText;
    const len = lengthBytesUTF8(text) + 1;
    const ptr = _malloc(len);
    stringToUTF8(text, ptr, len);
    return ptr;
});
#else
/* ==========================================
   CLI ИНТЕРФЕЙС (только для Native)
   ========================================== */
static void print_help(void) {
    printf("LUNA-8: A tiny retro game framework for Lua.\n\n");
    printf("USAGE: luna8 my_game.lua    Run a specific game file\n");
    printf("USAGE: luna8                Run game.lua next to the luna8 executable\n\n");
}

static int file_exists(const char *path) {
    FILE *f = fopen(path, "r");
    if (f) {
        fclose(f);
        return 1;
    }
    return 0;
}
#endif

/* Временный RGBA-буфер: сюда разворачиваем fb[] по палитре
   перед отправкой в UpdateTexture. */
static Color pixels[SCREEN_W * SCREEN_H];

static void present(Texture2D *tex) {
    for (int i = 0; i < SCREEN_W * SCREEN_H; i++) {
        uint32_t rgb = luna8_palette[luna8_fb[i]];
        pixels[i].r = (rgb >> 16) & 0xFF;
        pixels[i].g = (rgb >>  8) & 0xFF;
        pixels[i].b =  rgb        & 0xFF;
        pixels[i].a = 255;
    }
    UpdateTexture(*tex, pixels);
}

/* Integer-scale + центрирование: пиксель-перфект при любом размере окна.
   Полосы вокруг закрашиваются чёрным, иначе виден мусор прошлого кадра. */
static void draw_pixel_perfect(Texture2D *tex) {
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    int s = sw / SCREEN_W;
    int sy = sh / SCREEN_H;
    if (sy < s) s = sy;
    if (s < 1) s = 1;

    int dw = SCREEN_W * s;
    int dh = SCREEN_H * s;
    int ox = (sw - dw) / 2;
    int oy = (sh - dh) / 2;

    Rectangle src = { 0.0f, 0.0f, (float)SCREEN_W, (float)SCREEN_H };
    Rectangle dst = { (float)ox, (float)oy, (float)dw, (float)dh };
    Vector2   org = { 0.0f, 0.0f };

    ClearBackground(BLACK);
    DrawTexturePro(*tex, src, dst, org, 0.0f, WHITE);
}

static int build_default_script(char* buf, size_t buf_size) {
    const char* app_dir = GetApplicationDirectory();
    size_t dlen = strlen(app_dir);
    int need_sep = (dlen > 0 && app_dir[dlen - 1] != '/' && app_dir[dlen - 1] != '\\');
    int n = snprintf(buf, buf_size, "%s%sgame.lua", app_dir, need_sep ? "/" : "");
    if (n < 0 || (size_t)n >= buf_size) return -1;
    return 0;
}

int main(int argc, char** argv) {
    const char* script = NULL;
    char default_script[2048];

    // ==========================================
    // ПЕРЕНАПРАВЛЕНИЕ ВЫВОДА В ФАЙЛ (для релиза)
    // ==========================================
#ifndef LUNA8_DEV
    // Открываем файл luna8_log.txt и делаем его консолью
    if (freopen("luna8_log.txt", "w", stdout) == NULL) {
        // Если не удалось открыть файл (например, нет прав на запись),
        // просто продолжаем работу без логов.
    }
    if (freopen("luna8_log.txt", "w", stderr) == NULL) {
        // То же самое для ошибок
    }
#endif

    // ==========================================
    // 1. ПРОВЕРКА АРГУМЕНТОВ (ДО инициализации движка)
    // ==========================================
    if (argc >= 2) {
        /* native: luna8.exe foo.lua
           wasm:   callMain(['/game.lua']) → argv[1] = "/game.lua" */
        script = argv[1];
    } else {
#ifdef __EMSCRIPTEN__
        /* страховка: shell.html всегда передаёт argv[1], сюда не попадём */
        script = "/game.lua";
#else
        if (build_default_script(default_script, sizeof(default_script)) != 0) {
            fprintf(stderr, "[LUNA-8] script path too long\n");
            return 1;
        }
        if (!file_exists(default_script)) {
            print_help();
            return 0;
        }
        script = default_script;
#endif
    }

    // ==========================================
    // 2. ИНИЦИАЛИЗАЦИЯ ДВИЖКА
    // ==========================================
    // палитра + fb ДО загрузки скрипта
    luna8_api_init();

    lua_State* L = luna8_script_create();
    if (!L) {
        fprintf(stderr, "[LUNA-8] cannot create Lua state\n");
        return 1;
    }

    // загружаем save до скрипта
    luna8_save_open(script);

    if (luna8_script_load(L, script) != 0) {
        luna8_save_close();
        lua_close(L);
        return 1;
    }

    /* есть ли LUNA()? */
    lua_getglobal(L, "LUNA");
    int has_luna = lua_isfunction(L, -1);
    lua_pop(L, 1);

    if (!has_luna) {
        // Если LUNA() нет, значит это просто скрипт (например, тест или утилита).
        // Топ-левел код уже выполнился, окно открывать не нужно.
        luna8_save_close();
        lua_close(L);
        return 0;
    }

    // ==========================================
    // 3. ИНИЦИАЛИЗАЦИЯ RAYLIB И ИГРОВОЙ ЦИКЛ
    // ==========================================
    InitWindow(SCREEN_W * SCALE, SCREEN_H * SCALE, WINDOW_TITLE);
#ifndef __EMSCRIPTEN__
    SetWindowState(FLAG_WINDOW_RESIZABLE);

    /* иконка окна — вкомпилирована из luna8_icon.h */
    Image icon = {
        .data    = (void*)luna8_icon_rgba,
        .width   = ICON_W,
        .height  = ICON_H,
        .mipmaps = 1,
        .format  = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };
    SetWindowIcon(icon);
#endif
    SetTargetFPS(FPS);
    SetWindowMinSize(SCREEN_W, SCREEN_H);

    luna8_audio_init();

    Image img = GenImageColor(SCREEN_W, SCREEN_H, BLACK);
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    if (!IsTextureValid(tex)) {
        fprintf(stderr, "[LUNA-8] cannot create texture\n");
        luna8_save_close();
        luna8_audio_close();
        lua_close(L);
        CloseWindow();
        return 1;
    }
    SetTextureFilter(tex, TEXTURE_FILTER_POINT);

    while (!WindowShouldClose()) {
#ifndef __EMSCRIPTEN__
        if (IsKeyPressed(KEY_F11)) ToggleFullscreen();
#endif

        int rc = luna8_script_call(L, "LUNA");
        if (rc != 0) break;

        present(&tex);

        BeginDrawing();
            draw_pixel_perfect(&tex);
        EndDrawing();
    }

    // ==========================================
    // 4. ОЧИСТКА ПАМЯТИ
    // ==========================================
    luna8_audio_close();
    lua_close(L);
    luna8_save_close();
    UnloadTexture(tex);
    CloseWindow();
    return 0;
}