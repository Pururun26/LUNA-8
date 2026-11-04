#include "raylib.h"
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "luna8_conf.h"
#include "luna8_api.h"
#include "luna8_api_lua.h"

#include <stdio.h>

static void luna8_openlibs(lua_State *L) {
    luaL_requiref(L, LUA_GNAME,        luaopen_base,       1); lua_pop(L, 1);
    luaL_requiref(L, LUA_MATHLIBNAME,  luaopen_math,       1); lua_pop(L, 1);
    luaL_requiref(L, LUA_STRLIBNAME,   luaopen_string,     1); lua_pop(L, 1);
    luaL_requiref(L, LUA_TABLIBNAME,   luaopen_table,      1); lua_pop(L, 1);
    luaL_requiref(L, LUA_COLIBNAME,    luaopen_coroutine,  1); lua_pop(L, 1);

    lua_pushnil(L); lua_setglobal(L, "loadfile");
    lua_pushnil(L); lua_setglobal(L, "dofile");
}

/* возвращает ref на функцию update в registry или LUA_NOREF если её нет */
static int luna8_load_update(lua_State *L) {
    lua_getglobal(L, "update");
    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 1);
        return LUA_NOREF;
    }
    return luaL_ref(L, LUA_REGISTRYINDEX);
}

/* 0 = ок, 1 = ошибка (сообщение уже напечатано) */
static int luna8_call_update(lua_State *L, int ref) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        TraceLog(LOG_ERROR, "update: %s", lua_tostring(L, -1));
        lua_pop(L, 1);
        return 1;
    }
    return 0;
}

int main(int argc, char **argv) {
    const char *cart_path = (argc > 1) ? argv[1] : "cart.l8";

    /* --- Raylib + framebuffer --- */
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(LUNA8_W * LUNA8_SCALE, LUNA8_H * LUNA8_SCALE, "LUNA-8");
    SetTargetFPS(60);
    luna8_api_init();

    /* --- Lua --- */
    lua_State *L = luaL_newstate();
    luna8_openlibs(L);
    luna8_register_api(L);

    if (luaL_loadfile(L, cart_path) != LUA_OK) {
        TraceLog(LOG_ERROR, "Lua: %s", lua_tostring(L, -1));
        lua_close(L);
        luna8_api_shutdown();
        CloseWindow();
        return 1;
    }
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        TraceLog(LOG_ERROR, "Lua: %s", lua_tostring(L, -1));
        lua_close(L);
        luna8_api_shutdown();
        CloseWindow();
        return 1;
    }
    fflush(stdout);

    int update_ref = luna8_load_update(L);
    if (update_ref == LUA_NOREF) {
        TraceLog(LOG_ERROR, "cart: no update() function");
        lua_close(L);
        luna8_api_shutdown();
        CloseWindow();
        return 1;
    }

    /* --- main loop --- */
    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_F11)) ToggleFullscreen();
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_P))  luna8_fps_toggle();

        if (luna8_call_update(L, update_ref)) break;   /* ошибка в update — выходим */
        luna8_fps_tick();
        luna8_fps_draw();
        luna8_present();
    }

    luaL_unref(L, LUA_REGISTRYINDEX, update_ref);
    lua_close(L);
    luna8_api_shutdown();
    CloseWindow();
    return 0;
}