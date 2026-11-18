// luna8_api_lua.c
#include "luna8_api.h"
#include "luna8_api_lua.h"

#include <math.h>


// --- graphics ---

static int l_cls(lua_State* L) {
    luna8_cls((int)luaL_optinteger(L, 1, 0));
    return 0;
}

static int l_pset(lua_State* L) {
    int x = (int)floor(luaL_optnumber(L, 1, 0));
    int y = (int)floor(luaL_optnumber(L, 2, 0));
    int c = (int)luaL_optinteger(L, 3, 15);
    luna8_pset(x, y, c);
    return 0;
}

static int l_pget(lua_State* L) {
    int x = (int)floor(luaL_optnumber(L, 1, 0));
    int y = (int)floor(luaL_optnumber(L, 2, 0));
    lua_pushinteger(L, luna8_pget(x, y));
    return 1;
}

static int l_text(lua_State* L) {
    const char* s = luaL_checkstring(L, 1);
    int x = (int)floor(luaL_optnumber(L, 2, 0));
    int y = (int)floor(luaL_optnumber(L, 3, 0));
    int c = (int)luaL_optnumber(L, 4, 15);
    luna8_text(s, x, y, c);
    return 0;
}

static int l_pal(lua_State* L) {
    int idx = (int)luaL_checkinteger(L, 1);
    uint32_t hex = (uint32_t)luaL_checkinteger(L, 2);
    luna8_pal(idx, hex);
    return 0;
}


// --- primitives ---

static int l_line(lua_State* L) {
    int x0 = (int)floor(luaL_checknumber(L, 1));
    int y0 = (int)floor(luaL_checknumber(L, 2));
    int x1 = (int)floor(luaL_checknumber(L, 3));
    int y1 = (int)floor(luaL_checknumber(L, 4));
    int c  = (int)luaL_optnumber(L, 5, 15);
    luna8_line(x0, y0, x1, y1, c);
    return 0;
}

static int l_rect(lua_State* L) {
    int x = (int)floor(luaL_checknumber(L, 1));
    int y = (int)floor(luaL_checknumber(L, 2));
    int w = (int)floor(luaL_checknumber(L, 3));
    int h = (int)floor(luaL_checknumber(L, 4));
    int c = (int)luaL_optnumber(L, 5, 15);
    int filled = 0;
    if (!lua_isnoneornil(L, 6)) filled = lua_toboolean(L, 6);
    luna8_rect(x, y, w, h, c, filled);
    return 0;
}

static int l_oval(lua_State* L) {
    int x = (int)floor(luaL_checknumber(L, 1));
    int y = (int)floor(luaL_checknumber(L, 2));
    int w = (int)floor(luaL_checknumber(L, 3));
    int h = (int)floor(luaL_checknumber(L, 4));
    int c = (int)luaL_optnumber(L, 5, 15);
    int filled = 0;
    if (!lua_isnoneornil(L, 6)) filled = lua_toboolean(L, 6);
    luna8_oval(x, y, w, h, c, filled);
    return 0;
}


// --- btn/btnp ---

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


// --- sprite ---

// Парсер colorkey: принимает число (-1, 0, 2...) или таблицу {1, 3, 5}
static uint16_t parse_colorkey(lua_State* L, int idx) {
    // Если аргумент не передан или nil - прозрачности нет
    if (lua_isnoneornil(L, idx)) return 0;
    
    // Если это число
    if (lua_isinteger(L, idx) || lua_isnumber(L, idx)) {
        int c = (int)lua_tointeger(L, idx);
        if (c >= 0 && c < 16) return (uint16_t)(1 << c);
        return 0; // Если -1 или некорректное число
    }
    
    // Если это таблица (например, {1, 3, 2})
    if (lua_istable(L, idx)) {
        uint16_t mask = 0;
        lua_pushnil(L);
        while (lua_next(L, idx) != 0) {
            if (lua_isinteger(L, -1)) {
                int c = (int)lua_tointeger(L, -1);
                if (c >= 0 && c < 16) mask |= (1 << c);
            }
            lua_pop(L, 1); // Удаляем значение, оставляем ключ для lua_next
        }
        return mask;
    }
    
    return 0;
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

    uint16_t mask = parse_colorkey(L, 6);
    luna8_spr(id, x, y, flip_x, flip_y, mask);
    return 0;
}


// --- map ---

static int l_mapdata(lua_State* L) {
    const char* data = luaL_checkstring(L, 1);
    // По умолчанию 20x15 (ровно один экран 160x120)
    int w = (int)luaL_optinteger(L, 2, 20);
    int h = (int)luaL_optinteger(L, 3, 15);
    luna8_mapdata(data, w, h);
    return 0;
}

static int l_map(lua_State* L) {
    int celx = (int)floor(luaL_optnumber(L, 1, 0));
    int cely = (int)floor(luaL_optnumber(L, 2, 0));
    int sx   = (int)floor(luaL_optnumber(L, 3, 0));
    int sy   = (int)floor(luaL_optnumber(L, 4, 0));
    // По умолчанию рисуем ровно один экран (160/8=20, 120/8=15)
    int celw = (int)luaL_optinteger(L, 5, 20);
    int celh = (int)luaL_optinteger(L, 6, 15);

    uint16_t mask = parse_colorkey(L, 7);

    luna8_map(celx, cely, sx, sy, celw, celh, mask);
    return 0;
}

static int l_mget(lua_State* L) {
    int x = (int)floor(luaL_optnumber(L, 1, 0));
    int y = (int)floor(luaL_optnumber(L, 2, 0));
    lua_pushinteger(L, luna8_mget(x, y));
    return 1;
}

static int l_mset(lua_State* L) {
    int x = (int)floor(luaL_optnumber(L, 1, 0));
    int y = (int)floor(luaL_optnumber(L, 2, 0));
    int id = (int)luaL_optnumber(L, 3, 0);
    luna8_mset(x, y, id);
    return 0;
}


// --- time ---

static int l_time(lua_State* L) {
    lua_pushinteger(L, (lua_Integer)luna8_time());
    return 1;
}

static int l_utime(lua_State* L) {
    lua_pushinteger(L, (lua_Integer)luna8_utime());
    return 1;
}


// --- tone ---

static int l_tone(lua_State* L) {
    int freq = (int)luaL_checkinteger(L, 1);
    int dur  = (int)luaL_optinteger(L, 2, 15);
    int vol  = (int)luaL_optinteger(L, 3, 30);
    int ch   = (int)luaL_optinteger(L, 4, 0);
    luna8_tone(freq, dur, vol, ch);
    return 0;
}


// --- save/load ---

static int l_dset(lua_State* L) {
    int slot = (int)luaL_checkinteger(L, 1);
    lua_Integer v = luaL_checkinteger(L, 2);
    luna8_dset(slot, (int64_t)v);
    return 0;
}

static int l_dget(lua_State* L) {
    int slot = (int)luaL_checkinteger(L, 1);
    lua_pushinteger(L, (lua_Integer)luna8_dget(slot));
    return 1;
}


// --- register ---

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
    lua_register(L, "mapdata", l_mapdata);
    lua_register(L, "map",     l_map);
    lua_register(L, "mget",    l_mget);
    lua_register(L, "mset",    l_mset);
    lua_register(L, "time",    l_time);
    lua_register(L, "utime",   l_utime);
    lua_register(L, "tone",    l_tone);
    lua_register(L, "dset",    l_dset);
    lua_register(L, "dget",    l_dget);
}