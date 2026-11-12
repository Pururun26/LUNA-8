// luna8_api.h
#pragma once

#include "luna8_conf.h"

#include <stdint.h>

/* framebuffer: индексы в палитру, 0..PALETTE_SIZE-1 */
extern uint8_t  luna8_fb[SCREEN_W * SCREEN_H];

/* палитра: 0xRRGGBB */
extern uint32_t luna8_palette[PALETTE_SIZE];

void luna8_api_init(void);              /* SOBEACHY8 + очистка fb */
void luna8_cls(int color);
void luna8_pset(int x, int y, int color);
int  luna8_pget(int x, int y);
void luna8_text(const char *s, int x, int y, int color);
void luna8_pal(int idx, uint32_t hex);

void luna8_line(int x0, int y0, int x1, int y1, int color);
void luna8_rect(int x, int y, int w, int h, int color, int filled);
void luna8_oval(int x, int y, int w, int h, int color, int filled);

/* 0 = left, 1 = right, 2 = up, 3 = down, 4 = O, 5 = X */
int luna8_btn(int id, int player);
int luna8_btnp(int id, int player);

void luna8_sprdata(int id, const char* data, int w, int h);
void luna8_spr(int id, int x, int y, int flip_x, int flip_y, int colorkey);

void    luna8_time_reset(void);
int64_t luna8_time(void);       /* мс с момента init */
int64_t luna8_utime(void);      /* unix timestamp, секунды */

void luna8_audio_init(void);
void luna8_audio_close(void);
void luna8_tone(int frequency, int duration, int volume, int channel);

void    luna8_dset(int slot, int64_t value);
int64_t luna8_dget(int slot);