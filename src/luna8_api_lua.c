// luna8_api_lua.c
#include "luna8_api.h"
#include "luna8_api_lua.h"

#include <math.h>

static int l_cls(lua_State* L) {
    luna8_cls((int)luaL_optinteger(L, 1, 0));
    return 0;
}

static int l_pset(lua_State* L) {
    int x = (int)floor(luaL_checknumber(L, 1));
    int y = (int)floor(luaL_checknumber(L, 2));
    int c = (int)luaL_optinteger(L, 3, 0);
    luna8_pset(x, y, c);
    return 0;
}

static int l_pget(lua_State* L) {
    int x = (int)floor(luaL_checknumber(L, 1));
    int y = (int)floor(luaL_checknumber(L, 2));
    lua_pushinteger(L, luna8_pget(x, y));
    return 1;
}

static int l_text(lua_State* L) {
    const char* s = luaL_checkstring(L, 1);
    int x = (int)floor(luaL_optnumber(L, 2, 0));
    int y = (int)floor(luaL_optnumber(L, 3, 0));
    int c = (int)luaL_optnumber(L, 4, 7);
    luna8_text(s, x, y, c);
    return 0;
}

static int l_pal(lua_State* L) {
    int idx = (int)luaL_checkinteger(L, 1);
    uint32_t hex = (uint32_t)luaL_checkinteger(L, 2);
    luna8_pal(idx, hex);
    return 0;
}


static int l_line(lua_State* L) {
    int x0 = (int)floor(luaL_checknumber(L, 1));
    int y0 = (int)floor(luaL_checknumber(L, 2));
    int x1 = (int)floor(luaL_checknumber(L, 3));
    int y1 = (int)floor(luaL_checknumber(L, 4));
    int c  = (int)luaL_optnumber(L, 5, 0);
    luna8_line(x0, y0, x1, y1, c);
    return 0;
}

static int l_rect(lua_State* L) {
    int x = (int)floor(luaL_checknumber(L, 1));
    int y = (int)floor(luaL_checknumber(L, 2));
    int w = (int)floor(luaL_checknumber(L, 3));
    int h = (int)floor(luaL_checknumber(L, 4));
    int c = (int)luaL_optnumber(L, 5, 0);
    int filled = 1;
    if (!lua_isnoneornil(L, 6)) filled = lua_toboolean(L, 6);
    luna8_rect(x, y, w, h, c, filled);
    return 0;
}

static int l_oval(lua_State* L) {
    int x = (int)floor(luaL_checknumber(L, 1));
    int y = (int)floor(luaL_checknumber(L, 2));
    int w = (int)floor(luaL_checknumber(L, 3));
    int h = (int)floor(luaL_checknumber(L, 4));
    int c = (int)luaL_optnumber(L, 5, 0);
    int filled = 1;
    if (!lua_isnoneornil(L, 6)) filled = lua_toboolean(L, 6);
    luna8_oval(x, y, w, h, c, filled);
    return 0;
}


static int l_btn(lua_State* L) {
    int id = (int)luaL_checkinteger(L, 1);
    int player = (int)luaL_optinteger(L, 2, 0);
    lua_pushboolean(L, luna8_btn(id, player));
    return 1;
}

static int l_btnp(lua_State* L) {
    int id = (int)luaL_checkinteger(L, 1);
    int player = (int)luaL_optinteger(L, 2, 0);
    lua_pushboolean(L, luna8_btnp(id, player));
    return 1;
}


static int l_sprdata(lua_State* L) {
    int id = (int)luaL_checkinteger(L, 1);
    const char* data = luaL_checkstring(L, 2);
    int w = (int)luaL_optinteger(L, 3, 8);
    int h = (int)luaL_optinteger(L, 4, 8);
    luna8_sprdata(id, data, w, h);
    return 0;
}

static int l_spr(lua_State* L) {
    int id = (int)luaL_checkinteger(L, 1);
    int x = (int)floor(luaL_optnumber(L, 2, 0));
    int y = (int)floor(luaL_optnumber(L, 3, 0));
    int flip_x = lua_toboolean(L, 4);
    int flip_y = lua_toboolean(L, 5);
    int colorkey = (int)luaL_optinteger(L, 6, -1);
    luna8_spr(id, x, y, flip_x, flip_y, colorkey);
    return 0;
}


static int l_time(lua_State* L) {
    lua_pushinteger(L, (lua_Integer)luna8_time());
    return 1;
}

static int l_utime(lua_State* L) {
    lua_pushinteger(L, (lua_Integer)luna8_utime());
    return 1;
}


void luna8_register_api(lua_State *L) {
    lua_register(L, "cls",     l_cls);
    lua_register(L, "pset",    l_pset);
    lua_register(L, "pget",    l_pget);
    lua_register(L, "text",    l_text);
    lua_register(L, "pal",     l_pal);
    lua_register(L, "line",    l_line);
    lua_register(L, "rect",    l_rect);
    lua_register(L, "oval",    l_oval);
    lua_register(L, "btn",     l_btn);
    lua_register(L, "btnp",    l_btnp);
    lua_register(L, "sprdata", l_sprdata);
    lua_register(L, "spr",     l_spr);
    lua_register(L, "time",    l_time);
    lua_register(L, "utime",   l_utime);
}