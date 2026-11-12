#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "raylib.h"

#include "luna8_conf.h"
#include "luna8_api.h"
#include "luna8_script.h"

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
    printf("LUNA-8: starting\n");

    /* палитра + fb ДО загрузки скрипта */
    luna8_api_init();

    lua_State* L = luna8_script_create();
    if (!L) {
        fprintf(stderr, "[lua] cannot create state\n");
        return 1;
    }

    const char* script;
    char default_script[2048];

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
            lua_close(L);
            return 1;
        }
        script = default_script;
#endif
    }
    printf("LUNA-8: script = %s\n", script);

    if (luna8_script_load(L, script) != 0) {
        lua_close(L);
        return 1;
    }

    /* есть ли luna()? */
    lua_getglobal(L, "luna");
    int has_luna = lua_isfunction(L, -1);
    lua_pop(L, 1);

    if (!has_luna) {
        printf("LUNA-8: no luna() in script, exiting.\n");
        lua_close(L);
        return 0;
    }

    InitWindow(SCREEN_W * SCALE, SCREEN_H * SCALE, WINDOW_TITLE);
#ifndef __EMSCRIPTEN__
    SetWindowState(FLAG_WINDOW_RESIZABLE);
#endif
    SetTargetFPS(FPS);
    SetWindowMinSize(SCREEN_W, SCREEN_H);

    Image img = GenImageColor(SCREEN_W, SCREEN_H, BLACK);
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    if (!IsTextureValid(tex)) {
        fprintf(stderr, "[LUNA-8] cannot create texture\n");
        lua_close(L);
        CloseWindow();
        return 1;
    }
    SetTextureFilter(tex, TEXTURE_FILTER_POINT);

    while (!WindowShouldClose()) {
#ifndef __EMSCRIPTEN__
        if (IsKeyPressed(KEY_F11)) ToggleFullscreen();
#endif

        int rc = luna8_script_call(L, "luna");
        if (rc != 0) break;

        present(&tex);

        BeginDrawing();
            draw_pixel_perfect(&tex);
        EndDrawing();
    }

    lua_close(L);
    UnloadTexture(tex);
    CloseWindow();
    return 0;
}