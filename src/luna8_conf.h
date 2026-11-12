// luna8_conf.h
#pragma once

/* ---- Display ---- */
#define SCREEN_W   160
#define SCREEN_H   120
#define SCALE      4
#define FPS        60
#define WINDOW_TITLE "LUNA-8"

#define PALETTE_SIZE  8

#define BTN_COUNT   6
#define PLAYER_MAX  2

#define SPR_COUNT    512
#define SPR_MAX_W    64
#define SPR_MAX_H    64

#define TONE_SAMPLE_RATE   44100
#define TONE_MAX_FRAMES    60
#define TONE_MAX_SAMPLES   (TONE_SAMPLE_RATE * TONE_MAX_FRAMES / 60)
#define TONE_FADE          88          /* 2 ms при 44100 */
#define TONE_CHANNELS    4

#define SAVE_SLOTS  64