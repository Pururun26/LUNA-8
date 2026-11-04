cmake_minimum_required(VERSION 3.20)
project(LUNA8 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(RAYLIB_DIR "${CMAKE_SOURCE_DIR}/raylib/raylib-6.0_win64_mingw-w64")
set(LUA_DIR    "${CMAKE_SOURCE_DIR}/lua-5.4.9")

# ---------- Lua 5.4 (static) ----------
# без lua.c (standalone) и luac.c (компилятор)
set(LUA_SOURCES
    # --- core ---
    ${LUA_DIR}/lapi.c
    ${LUA_DIR}/lcode.c
    ${LUA_DIR}/lctype.c
    ${LUA_DIR}/ldebug.c
    ${LUA_DIR}/ldo.c
    ${LUA_DIR}/ldump.c
    ${LUA_DIR}/lfunc.c
    ${LUA_DIR}/lgc.c
    ${LUA_DIR}/llex.c
    ${LUA_DIR}/lmem.c
    ${LUA_DIR}/lobject.c
    ${LUA_DIR}/lopcodes.c
    ${LUA_DIR}/lparser.c
    ${LUA_DIR}/lstate.c
    ${LUA_DIR}/lstring.c
    ${LUA_DIR}/ltable.c
    ${LUA_DIR}/ltm.c
    ${LUA_DIR}/lundump.c
    ${LUA_DIR}/lvm.c
    ${LUA_DIR}/lzio.c
    # --- aux ---
    ${LUA_DIR}/lauxlib.c
    # --- stdlibs we actually open ---
    ${LUA_DIR}/lbaselib.c
    ${LUA_DIR}/lmathlib.c
    ${LUA_DIR}/lstrlib.c
    ${LUA_DIR}/ltablib.c
    ${LUA_DIR}/lcorolib.c
)

add_library(lua_static STATIC ${LUA_SOURCES})
target_include_directories(lua_static PUBLIC ${LUA_DIR})

# ---------- LUNA-8 ----------
add_executable(luna8
    src/luna8.c
    src/luna8_api.c
    src/luna8_api_lua.c
)

target_include_directories(luna8 PRIVATE
    ${RAYLIB_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

target_link_directories(luna8 PRIVATE ${RAYLIB_DIR}/lib)

target_link_libraries(luna8 PRIVATE
    lua_static
    raylib
    winmm
    gdi32
    opengl32
    user32
    shell32
)