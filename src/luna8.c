#include <stdio.h>
#include <string.h>

#include "raylib.h"
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "luna8_conf.h"

/* --- traceback handler ---
   Вызывается lua_pcall при ошибке. Кладёт на стек строку
   с сообщением и стеком вызовов Lua. */
static int msgh(lua_State* L) {
    const char* msg = lua_tostring(L, 1);
    if (msg == NULL) {
        /* error() мог получить таблицу/число — пробуем __tostring */
        if (luaL_callmeta(L, 1, "__tostring") && lua_type(L, -1) == LUA_TSTRING) {
            return 1;
        }
        msg = lua_pushfstring(L, "(error object is a %s value)",
                              luaL_typename(L, 1));
    }
    luaL_traceback(L, L, msg, 1);
    return 1;
}

/* Безопасно печатает ошибку с верхушки стека и снимает её.
   Не падает, если lua_tostring вернул NULL. */
static void report_lua_error(lua_State* L, const char* where) {
    const char* msg = lua_tostring(L, -1);
    fprintf(stderr, "[lua] %s: %s\n", where, msg ? msg : "(non-string error)");
    lua_pop(L, 1);
}

static void luna_open_libs(lua_State* L) {
    static const luaL_Reg libs[] = {
        {LUA_GNAME,       luaopen_base},
        {LUA_TABLIBNAME,  luaopen_table},
        {LUA_STRLIBNAME,  luaopen_string},
        {LUA_MATHLIBNAME, luaopen_math},
        {LUA_COLIBNAME,   luaopen_coroutine},
        {NULL, NULL}
    };
    for (const luaL_Reg* lib = libs; lib->func; lib++) {
        luaL_requiref(L, lib->name, lib->func, 1);
        lua_pop(L, 1);
    }

    /* --- sandbox ---
       Убираем файловый доступ и загрузку байткода.
       io/os/debug/package/utf8 отсутствуют в сборке вообще,
       а эти функции живут в base/string — снимаем руками. */
    static const char* banned[] = {
        "dofile", "loadfile", "load", "loadstring", NULL
    };
    for (const char** p = banned; *p; p++) {
        lua_pushnil(L);
        lua_setglobal(L, *p);
    }

    lua_getglobal(L, LUA_STRLIBNAME);
    if (lua_istable(L, -1)) {
        lua_pushnil(L);
        lua_setfield(L, -2, "dump");
    }
    lua_pop(L, 1);
}

/* Склеивает путь game.lua рядом с exe, корректно обрабатывая
   завершающий '/' или '\'. Возвращает 0 при успехе. */
static int build_default_script(char* buf, size_t buf_size) {
    const char* app_dir = GetApplicationDirectory();
    size_t dlen = strlen(app_dir);
    int need_sep = (dlen > 0 && app_dir[dlen - 1] != '/' && app_dir[dlen - 1] != '\\');
    int n = snprintf(buf, buf_size, "%s%sgame.lua", app_dir, need_sep ? "/" : "");
    if (n < 0 || (size_t)n >= buf_size) return -1;
    return 0;
}

int main(int argc, char** argv) {
    char default_script[2048];
    const char* script;

    if (argc >= 2) {
        script = argv[1];
    } else {
        if (build_default_script(default_script, sizeof(default_script)) != 0) {
            fprintf(stderr, "[luna8] script path too long\n");
            return 1;
        }
        script = default_script;
    }
    printf("LUNA-8: script = %s\n", script);

    lua_State* L = luaL_newstate();
    if (!L) { fprintf(stderr, "[lua] cannot create state\n"); return 1; }
    luna_open_libs(L);

    /* loadfile: ошибки компиляции — без стека, msgh не нужен */
    if (luaL_loadfile(L, script) != LUA_OK) {
        report_lua_error(L, "load error");
        lua_close(L);
        return 1;
    }

    /* top-level pcall с msgh для трейсбека */
    lua_pushcfunction(L, msgh);       /* [chunk][msgh] */
    lua_insert(L, -2);                /* [msgh][chunk] */
    if (lua_pcall(L, 0, 0, -2) != LUA_OK) {
        report_lua_error(L, "runtime error");  /* снимает err, остаётся [msgh] */
        lua_pop(L, 1);                         /* msgh */
        lua_close(L);
        return 1;
    }
    lua_pop(L, 1);                             /* msgh */

    lua_getglobal(L, "luna");
    int has_luna = lua_isfunction(L, -1);
    lua_pop(L, 1);

    if (!has_luna) {
        printf("LUNA-8: no luna() in script, exiting.\n");
        lua_close(L);
        return 0;
    }

    InitWindow(SCREEN_W * SCALE, SCREEN_H * SCALE, WINDOW_TITLE);
    SetTargetFPS(FPS);

    RenderTexture2D target = LoadRenderTexture(SCREEN_W, SCREEN_H);
    if (!IsRenderTextureValid(target)) {
        fprintf(stderr, "[luna8] cannot create render target\n");
        UnloadRenderTexture(target);
        lua_close(L);
        CloseWindow();
        return 1;
    }
    SetTextureFilter(target.texture, TEXTURE_FILTER_POINT);

    while (!WindowShouldClose()) {
        int ok = 1;

        BeginTextureMode(target);
            lua_getglobal(L, "luna");
            if (!lua_isfunction(L, -1)) {
                fprintf(stderr, "[lua] 'luna' is not a function\n");
                lua_pop(L, 1);
                ok = 0;
            } else {
                lua_pushcfunction(L, msgh);   /* [f][msgh] */
                lua_insert(L, -2);            /* [msgh][f] */
                if (lua_pcall(L, 0, 0, -2) != LUA_OK) {
                    report_lua_error(L, "error in luna()");
                    lua_pop(L, 1);            /* msgh */
                    ok = 0;
                } else {
                    lua_pop(L, 1);            /* msgh */
                }
            }
        EndTextureMode();

        if (!ok) break;

        Rectangle source = { 0.0f, 0.0f, (float)SCREEN_W, -(float)SCREEN_H };
        Rectangle dest   = { 0.0f, 0.0f, (float)GetScreenWidth(), (float)GetScreenHeight() };
        Vector2   origin = { 0.0f, 0.0f };

        BeginDrawing();
            DrawTexturePro(target.texture, source, dest, origin, 0.0f, WHITE);
        EndDrawing();
    }

    lua_close(L);
    UnloadRenderTexture(target);
    CloseWindow();
    return 0;
}