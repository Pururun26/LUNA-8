#include "luna8_api_lua.h"
#include "luna8_api.h"

static int l_cls(lua_State *L) {
    int c = (int)luaL_optinteger(L, 1, 0);
    luna8_cls(c);
    return 0;
}

static int l_pset(lua_State *L) {
    int x = (int)luaL_checkinteger(L, 1);
    int y = (int)luaL_checkinteger(L, 2);
    int c = (int)luaL_optinteger(L, 3, 0);
    luna8_pset(x, y, c);
    return 0;
}

static int l_pget(lua_State *L) {
    int x = (int)luaL_checkinteger(L, 1);
    int y = (int)luaL_checkinteger(L, 2);
    lua_pushinteger(L, luna8_pget(x, y));
    return 1;
}

static int l_print(lua_State *L) {
    luaL_checkany(L, 1);
    int x = (int)luaL_optinteger(L, 2, 0);
    int y = (int)luaL_optinteger(L, 3, 0);
    int c = (int)luaL_optinteger(L, 4, 12);   /* 12 = f4f4f4 белый */

    size_t len;
    const char *s = luaL_tolstring(L, 1, &len);
    luna8_print(s, x, y, c);
    lua_pop(L, 1);
    return 0;
}

static const luaL_Reg luna8_fns[] = {
    { "cls",  l_cls  },
    { "pset", l_pset },
    { "pget", l_pget },
    { "print", l_print },
    { NULL, NULL }
};

void luna8_register_api(lua_State *L) {
    for (const luaL_Reg *r = luna8_fns; r->name; r++) {
        lua_pushcfunction(L, r->func);
        lua_setglobal(L, r->name);
    }
}