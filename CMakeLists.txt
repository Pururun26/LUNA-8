cmake_minimum_required(VERSION 3.20)
project(LUNA8 VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------- Lua 5.4.9 ----------
set(LUA_DIR ${CMAKE_SOURCE_DIR}/lua-5.4.9)
set(LUA_SRC_DIR "${LUA_DIR}")

set(LUA_SOURCES
    lapi.c lcode.c lctype.c ldebug.c ldo.c ldump.c lfunc.c lgc.c
    llex.c lmem.c lobject.c lopcodes.c lparser.c lstate.c lstring.c
    ltable.c ltm.c lundump.c lvm.c lzio.c
    lauxlib.c
    lbaselib.c lcorolib.c lmathlib.c lstrlib.c ltablib.c
)
list(TRANSFORM LUA_SOURCES PREPEND "${LUA_SRC_DIR}/")

add_library(lua_static STATIC ${LUA_SOURCES})
target_include_directories(lua_static PUBLIC "${LUA_SRC_DIR}")
target_compile_definitions(lua_static PRIVATE LUA_USE_WINDOWS)
target_compile_options(lua_static PRIVATE -w)

# ---------- raylib (prebuilt) ----------
set(RAYLIB_DIR ${CMAKE_SOURCE_DIR}/raylib/raylib-6.0_win64_mingw-w64)
add_library(raylib STATIC IMPORTED)
set_target_properties(raylib PROPERTIES
    IMPORTED_LOCATION             "${RAYLIB_DIR}/lib/libraylib.a"
    INTERFACE_INCLUDE_DIRECTORIES "${RAYLIB_DIR}/include"
)
target_link_libraries(raylib INTERFACE
    opengl32 gdi32 winmm user32 shell32
)

# ---------- executable ----------
add_executable(luna8 src/luna8.c)
target_include_directories(luna8 PRIVATE src)
target_link_libraries(luna8 PRIVATE lua_static raylib)

if(MINGW)
    target_link_options(luna8 PRIVATE -static-libgcc -static-libstdc++)
endif()