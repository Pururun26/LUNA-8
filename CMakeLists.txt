cmake_minimum_required(VERSION 3.20)
project(LUNA8 VERSION 0.9.1 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------- Release по умолчанию ----------
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

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
target_compile_options(lua_static PRIVATE -w)
target_compile_options(lua_static PRIVATE
    -O3 -ffunction-sections -fdata-sections
)

if(WIN32)
    target_compile_definitions(lua_static PRIVATE LUA_USE_WINDOWS)
elseif(UNIX AND NOT EMSCRIPTEN)
    target_compile_definitions(lua_static PRIVATE LUA_USE_POSIX)
endif()

# ---------- raylib: windows / linux / wasm ----------
if(EMSCRIPTEN)
    set(RAYLIB_DIR ${CMAKE_SOURCE_DIR}/raylib/raylib-6.0_webassembly)
    set(RAYLIB_LIB "${RAYLIB_DIR}/lib/libraylib.web.a")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
elseif(WIN32)
    set(RAYLIB_DIR ${CMAKE_SOURCE_DIR}/raylib/raylib-6.0_win64_mingw-w64)
    set(RAYLIB_LIB "${RAYLIB_DIR}/lib/libraylib.a")
elseif(UNIX)
    set(RAYLIB_DIR ${CMAKE_SOURCE_DIR}/raylib/raylib-6.0_linux_amd64)
    set(RAYLIB_LIB "${RAYLIB_DIR}/lib/libraylib.a")
else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

add_library(raylib STATIC IMPORTED)
set_target_properties(raylib PROPERTIES
    IMPORTED_LOCATION             "${RAYLIB_LIB}"
    INTERFACE_INCLUDE_DIRECTORIES "${RAYLIB_DIR}/include"
)

if(WIN32)
    target_link_libraries(raylib INTERFACE
        opengl32 gdi32 winmm user32 shell32
    )
elseif(UNIX AND NOT EMSCRIPTEN)
    target_link_libraries(raylib INTERFACE
        GL m pthread dl rt X11
    )
endif()

# ---------- executable ----------
add_executable(luna8
    src/luna8.c
    src/luna8_api.c
    src/luna8_api_lua.c
    src/luna8_script.c
    src/luna8_save.c
)
target_include_directories(luna8 PRIVATE src)
target_link_libraries(luna8 PRIVATE lua_static raylib)

target_compile_options(luna8 PRIVATE
    -O3 -ffunction-sections -fdata-sections
)

if(EMSCRIPTEN)
    set_target_properties(luna8 PROPERTIES OUTPUT_NAME "index")
    target_link_options(luna8 PRIVATE
        -O3
        -Wl,--gc-sections
        -sUSE_GLFW=3
        -sALLOW_MEMORY_GROWTH=1
        -sASYNCIFY
        -sASYNCIFY_STACK_SIZE=16384
        -sASSERTIONS=0
        -sMALLOC=emmalloc
        "-sEXPORTED_RUNTIME_METHODS=['callMain']"
        --shell-file ${CMAKE_SOURCE_DIR}/web/shell.html
    )
elseif(MINGW)
    target_link_options(luna8 PRIVATE
        -Wl,--gc-sections
        -s
        -static-libgcc -static-libstdc++
    )
elseif(UNIX AND NOT EMSCRIPTEN)
    target_link_options(luna8 PRIVATE
        -Wl,--gc-sections
        -s
    )
endif()