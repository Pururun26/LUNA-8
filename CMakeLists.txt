cmake_minimum_required(VERSION 3.20)
project(LUNA8 VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------- Release по умолчанию ----------
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

# ---------- Опция для сборки Dev-версии (с консолью) ----------
option(LUNA8_DEV_BUILD "Build for development (with console window and logs)" OFF)

# ---------- Lua 5.4.9 ----------
set(LUA_DIR ${CMAKE_SOURCE_DIR}/lua-5.4.9)
set(LUA_SRC_DIR "${LUA_DIR}")

set(LUA_SOURCES
    lapi.c lcode.c lctype.c ldebug.c ldo.c ldump.c lfunc.c lgc.c
    llex.c lmem.c lobject.c lopcodes.c lparser.c lstate.c lstring.c
    ltable.c ltm.c lundump.c lvm.c lzio.c
    lauxlib.c
    lbaselib.c lcorolib.c lmathlib.c lstrlib.c ltablib.c
    lutf8lib.c
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
if(MINGW)
    enable_language(RC)
    set(LUNA8_RESOURCES icon.rc)
else()
    set(LUNA8_RESOURCES)
endif()

# Собираем все исходники в одну переменную
set(LUNA8_SOURCES
    src/luna8.c
    src/luna8_api.c
    src/luna8_api_lua.c
    src/luna8_script.c
    src/luna8_save.c
    ${LUNA8_RESOURCES}
)

# Список целей, которые мы будем настраивать
set(TARGETS_TO_BUILD luna8)

if(WIN32 AND NOT EMSCRIPTEN)
    # 1. Релизная версия (БЕЗ консоли, GUI)
    add_executable(luna8 WIN32 ${LUNA8_SOURCES})

    # 2. Девелоперская версия (С консолью)
    add_executable(luna8_dev ${LUNA8_SOURCES})
    target_compile_definitions(luna8_dev PRIVATE LUNA8_DEV)
    
    # Добавляем обе цели в список для общей настройки
    list(APPEND TARGETS_TO_BUILD luna8_dev)
else()
    # Для Linux и Web просто собираем одну стандартную версию
    add_executable(luna8 ${LUNA8_SOURCES})
endif()

# ==========================================
# Общие настройки для ВСЕХ целей (и luna8, и luna8_dev)
# ==========================================
foreach(tgt ${TARGETS_TO_BUILD})
    target_include_directories(${tgt} PRIVATE src)
    target_link_libraries(${tgt} PRIVATE lua_static raylib)
    target_compile_options(${tgt} PRIVATE -O3 -ffunction-sections -fdata-sections)

    if(EMSCRIPTEN)
        set_target_properties(${tgt} PROPERTIES OUTPUT_NAME "index")
        target_link_options(${tgt} PRIVATE
            -O3 -Wl,--gc-sections
            -sUSE_GLFW=3 -sALLOW_MEMORY_GROWTH=1
            -sASYNCIFY -sASYNCIFY_STACK_SIZE=16384
            -sASSERTIONS=0 -sMALLOC=emmalloc
            "-sEXPORTED_RUNTIME_METHODS=['callMain']"
            --shell-file ${CMAKE_SOURCE_DIR}/web/shell.html
        )
    elseif(MINGW)
        target_link_options(${tgt} PRIVATE
            -Wl,--gc-sections -s
            -static-libgcc -static-libstdc++
        )
    elseif(UNIX AND NOT EMSCRIPTEN)
        target_link_options(${tgt} PRIVATE
            -Wl,--gc-sections -s
        )
    endif()
endforeach()