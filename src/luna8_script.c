#include <stdio.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "luna8_script.h"
#include "luna8_api_lua.h"

/* --- error handling --- */

int luna8_msgh(lua_State *L) {
    const char* msg = lua_tostring(L, 1);
    if (msg == NULL) {
        if (luaL_callmeta(L, 1, "__tostring") && lua_type(L, -1) == LUA_TSTRING)
            return 1;
        msg = lua_pushfstring(L, "(error object is a %s value)",
                              luaL_typename(L, 1));
    }
    luaL_traceback(L, L, msg, 1);
    return 1;
}

void luna8_report_error(lua_State *L, const char* where) {
    const char* msg = lua_tostring(L, -1);
    fprintf(stderr, "[lua] %s: %s\n", where, msg ? msg : "(non-string error)");
    lua_pop(L, 1);
}

/* --- sandbox --- */

static void luna8_open_libs(lua_State *L) {
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

    /* Убираем файловый доступ и загрузку байткода.
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

/* --- public API --- */

lua_State *luna8_script_create(void) {
    lua_State *L = luaL_newstate();
    if (!L) return NULL;
    luna8_open_libs(L);
    luna8_register_api(L);
    return L;
}

int luna8_script_load(lua_State *L, const char *path) {
    if (luaL_loadfile(L, path) != LUA_OK) {
        luna8_report_error(L, "load error");
        return 1;
    }
    lua_pushcfunction(L, luna8_msgh);
    lua_insert(L, -2);
    if (lua_pcall(L, 0, 0, -2) != LUA_OK) {
        luna8_report_error(L, "runtime error");
        lua_pop(L, 1);      /* msgh */
        return 1;
    }
    lua_pop(L, 1);          /* msgh */
    return 0;
}

int luna8_script_call(lua_State *L, const char *name) {
    lua_getglobal(L, name);
    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 1);
        return 1;
    }
    lua_pushcfunction(L, luna8_msgh);
    lua_insert(L, -2);
    if (lua_pcall(L, 0, 0, -2) != LUA_OK) {
        luna8_report_error(L, "error in luna()");
        lua_pop(L, 1);      /* msgh */
        return 2;
    }
    lua_pop(L, 1);          /* msgh */
    return 0;
}