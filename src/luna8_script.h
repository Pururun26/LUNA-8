// luna8_script.h
#pragma once
#include <lua.h>

/* Создаёт lua_State, ставит песочницу, регистрирует API.
   Возвращает NULL при ошибке. */
lua_State *luna8_script_create(void);

/* Загружает и исполняет файл. Работает и на нативе, и в wasm:
   на нативе — путь с диска, в wasm — /game.lua из виртуального FS. */
int luna8_script_load(lua_State *L, const char *path);

/* Вызывает глобальную функцию без аргументов.
   0 = ок, 1 = функции нет, 2 = ошибка при вызове. */
int luna8_script_call(lua_State *L, const char *name);

/* traceback-хендлер + логгер — экспорт на случай своего pcall */
int  luna8_msgh(lua_State *L);
void luna8_report_error(lua_State *L, const char *where);