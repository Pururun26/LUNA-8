#include "luna8_conf.h"
#include "luna8_api.h"
#include "luna8_font3x4.h"

#include "raylib.h"

#include <string.h>
#include <stdint.h>
#include <time.h>
#include <stdio.h>

/* Палитра: 0xAABBGGRR, т.к. формат R8G8B8A8 little-endian.
 * В памяти получается RR GG BB AA — то, что ждёт raylib. */
static const uint32_t LUNA8_PALETTE[16] = {
    0xFF2c1c1a,  /* 0  1a1c2c */
    0xFF5d275d,  /* 1  5d275d */
    0xFF533eb1,  /* 2  b13e53 */
    0xFF577def,  /* 3  ef7d57 */
    0xFF75cdff,  /* 4  ffcd75 */
    0xFF70f0a7,  /* 5  a7f070 */
    0xFF64b738,  /* 6  38b764 */
    0xFF797125,  /* 7  257179 */
    0xFF6f3629,  /* 8  29366f */
    0xFFc95d3b,  /* 9  3b5dc9 */
    0xFFf6a641,  /* 10 41a6f6 */
    0xFFf7ef73,  /* 11 73eff7 */
    0xFFf4f4f4,  /* 12 f4f4f4 */
    0xFFc2b094,  /* 13 94b0c2 */
    0xFF866c56,  /* 14 566c86 */
    0xFF573c33,  /* 15 333c57 */
};

static uint8_t   g_fb[LUNA8_W * LUNA8_H];     /* индексы палитры */
static uint32_t  g_pixels[LUNA8_W * LUNA8_H]; /* RGBA для текстуры */
static Texture2D g_screen;

static int    g_fps_value  = 0;
static int    g_fps_frames = 0;
static time_t g_fps_last   = 0;
static bool   g_fps_show   = false;

void luna8_fps_tick(void) {
    g_fps_frames++;
    time_t now = time(NULL);
    if (g_fps_last == 0) g_fps_last = now;
    if (now - g_fps_last >= 1) {
        g_fps_value  = g_fps_frames;
        g_fps_frames = 0;
        g_fps_last   = now;
    }
}

void luna8_fps_toggle(void) { g_fps_show = !g_fps_show; }

void luna8_fps_draw(void) {
    if (!g_fps_show) return;

    char buf[16];
    snprintf(buf, sizeof buf, "%d", g_fps_value);

    int w = 0;
    for (const char *p = buf; *p; p++) {
        int idx = (int)(unsigned char)*p - FONT_FIRST;
        if (idx < 0 || idx >= FONT_COUNT) idx = '?' - FONT_FIRST;
        w += luna8_font_w[idx] + 1;
    }
    w -= 1;

    int x = LUNA8_W - w - 2;
    if (x < 0) x = 0;
    luna8_print(buf, x, 2, 12);
}

void luna8_api_init(void) {
    memset(g_fb, 0, sizeof g_fb);

    /* первый кадр сразу корректный, не ждём luna8_present */
    for (int i = 0; i < LUNA8_W * LUNA8_H; i++) g_pixels[i] = LUNA8_PALETTE[0];

    Image img = {
        .data    = g_pixels,
        .width   = LUNA8_W,
        .height  = LUNA8_H,
        .format  = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
        .mipmaps = 1,
    };
    g_screen = LoadTextureFromImage(img);
    SetTextureFilter(g_screen, TEXTURE_FILTER_POINT);
}

void luna8_api_shutdown(void) {
    UnloadTexture(g_screen);
}

void luna8_present(void) {
    /* fb (индексы) -> g_pixels (RGBA) */
    for (int i = 0; i < LUNA8_W * LUNA8_H; i++) {
        g_pixels[i] = LUNA8_PALETTE[g_fb[i] & 0x0F];
    }
    UpdateTexture(g_screen, g_pixels);

    BeginDrawing();
    ClearBackground(BLACK);

    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    int scale_x = screenW / LUNA8_W;
    int scale_y = screenH / LUNA8_H;
    int scale   = (scale_x < scale_y) ? scale_x : scale_y;
    if (scale < 1) scale = 1;

    int draw_w  = LUNA8_W * scale;
    int draw_h  = LUNA8_H * scale;
    int offsetX = (screenW - draw_w) / 2;
    int offsetY = (screenH - draw_h) / 2;

    DrawTexturePro(
        g_screen,
        (Rectangle){ 0, 0, LUNA8_W, LUNA8_H },
        (Rectangle){ (float)offsetX, (float)offsetY,
                     (float)draw_w,  (float)draw_h },
        (Vector2){ 0, 0 }, 0.0f, WHITE);

    EndDrawing();
}

void luna8_cls(int c) {
    memset(g_fb, (uint8_t)(c & 0x0F), sizeof g_fb);
}

void luna8_pset(int x, int y, int c) {
    if ((unsigned)x >= LUNA8_W || (unsigned)y >= LUNA8_H) return;
    g_fb[y * LUNA8_W + x] = (uint8_t)(c & 0x0F);
}

int luna8_pget(int x, int y) {
    if ((unsigned)x >= LUNA8_W || (unsigned)y >= LUNA8_H) return 0;
    return g_fb[y * LUNA8_W + x];
}

void luna8_print(const char *s, int x, int y, int color) {
    if (!s) return;
    int cx = x, cy = y;
    for (const unsigned char *p = (const unsigned char *)s; *p; p++) {
        if (*p == '\n') { cx = x; cy += FONT_CELL_H + 1; continue; }
        if (*p == '\r') continue;

        unsigned char u = *p;
        if (u >= 'a' && u <= 'z') u -= 32;

        int idx = (int)u - FONT_FIRST;
        if (idx < 0 || idx >= FONT_COUNT) idx = '?' - FONT_FIRST;

        int w = luna8_font_w[idx];
        for (int row = 0; row < FONT_CELL_H; row++) {
            unsigned char bits = luna8_font[idx][row];
            for (int col = 0; col < FONT_CELL_W; col++) {
                if ((bits >> (FONT_CELL_W - 1 - col)) & 1)
                    luna8_pset(cx + col, cy + row, color);
            }
        }
        cx += w + 1;
    }
}