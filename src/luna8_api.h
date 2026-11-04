#pragma once

void luna8_api_init(void);
void luna8_api_shutdown(void);
void luna8_present(void);

void luna8_fps_tick(void);
void luna8_fps_toggle(void);
void luna8_fps_draw(void);

/* работа с фреймбуфером индексов палитры (0..15) */
void luna8_cls(int c);
void luna8_pset(int x, int y, int c);
int  luna8_pget(int x, int y);
void luna8_print(const char *s, int x, int y, int color);