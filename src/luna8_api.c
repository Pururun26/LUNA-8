// luna8_api.c
#include "luna8_api.h"
#include "luna8_font3x4.h"
#include "luna8_save.h"
#include "raylib.h"

#include <string.h>
#include <stdlib.h>   /* abs */
#include <math.h>     /* sqrtf */
#include <time.h>

uint8_t  luna8_fb[SCREEN_W * SCREEN_H];
uint32_t luna8_palette[PALETTE_SIZE];

static const uint32_t SOBEACHY8[PALETTE_SIZE] = {
    0xE55388, 0xE57D88, 0xE59F88, 0xE5D988,
    0xE3D5CC, 0xBAD5CC, 0x6DD5CC, 0x5AC5CC,
};

void luna8_api_init(void) {
    for (int i = 0; i < PALETTE_SIZE; i++)
        luna8_palette[i] = SOBEACHY8[i];
    memset(luna8_fb, 0, sizeof(luna8_fb));
    luna8_time_reset();
}

static int clamp_color(int c) {
    return (c < 0 || c >= PALETTE_SIZE) ? 0 : c;
}

void luna8_cls(int c) {
    memset(luna8_fb, (uint8_t)clamp_color(c), sizeof(luna8_fb));
}

void luna8_pset(int x, int y, int c) {
    if (x < 0 || x >= SCREEN_W || y < 0 || y >= SCREEN_H) return;
    luna8_fb[y * SCREEN_W + x] = (uint8_t)clamp_color(c);
}

int luna8_pget(int x, int y) {
    if (x < 0 || x >= SCREEN_W || y < 0 || y >= SCREEN_H) return 0;
    return luna8_fb[y * SCREEN_W + x];
}

void luna8_text(const char *s, int x, int y, int color) {
    if (!s) return;

    int cx = x, cy = y;

    for (const unsigned char *p = (const unsigned char *)s; *p; p++) {
        if (*p == '\n') { cx = x; cy += FONT_CELL_H + 1; continue; }
        if (*p == '\r') continue;

        unsigned char u = *p;
        if (u >= 'a' && u <= 'z') u -= 32;      /* lowercase → uppercase */

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
        cx += w + 1;    /* +1 — межбуквенный интервал, иначе слипается */
    }
}

void luna8_pal(int idx, uint32_t hex) {
    if (idx < 0 || idx >= PALETTE_SIZE) return;
    luna8_palette[idx] = hex & 0xFFFFFFu;
}


void luna8_line(int x0, int y0, int x1, int y1, int color) {
    int dx =  abs(x1 - x0);
    int dy = -abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx + dy;

    for (;;) {
        luna8_pset(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void luna8_rect(int x, int y, int w, int h, int color, int filled) {
    if (w <= 0 || h <= 0) return;

    if (filled) {
        for (int j = 0; j < h; j++)
            for (int i = 0; i < w; i++)
                luna8_pset(x + i, y + j, color);
    } else {
        for (int i = 0; i < w; i++) {
            luna8_pset(x + i, y,         color);
            luna8_pset(x + i, y + h - 1, color);
        }
        for (int j = 0; j < h; j++) {
            luna8_pset(x,         y + j, color);
            luna8_pset(x + w - 1, y + j, color);
        }
    }
}

void luna8_oval(int x, int y, int w, int h, int color, int filled) {
    if (w <= 0 || h <= 0) return;

    /* вырожденные и слишком маленькие — рисуем как прямоугольник */
    if (w <= 2 || h <= 2) {
        luna8_rect(x, y, w, h, color, 1);
        return;
    }

    if (filled) {
        float rx = (w - 1) / 2.0f;
        float ry = (h - 1) / 2.0f;
        float cx = x + rx;

        for (int j = 0; j < h; j++) {
            float dy = (j - ry) / ry;
            float dy2 = dy * dy;
            if (dy2 > 1.0f) continue;
            float dx = sqrtf(1.0f - dy2) * rx;

            int x0 = (int)(cx - dx + 0.5f);
            int x1 = (int)(cx + dx + 0.5f);

            if (x0 < x)       x0 = x;
            if (x1 > x + w - 1) x1 = x + w - 1;

            for (int i = x0; i <= x1; i++)
                luna8_pset(i, y + j, color);
        }
    } else {
        /* midpoint ellipse (Zingl) */
        int x0 = x, y0 = y, x1 = x + w - 1, y1 = y + h - 1;
        long a  = abs(x1 - x0);
        long b  = abs(y1 - y0);
        long b1 = b & 1;
        long dx = 4 * (1 - a) * b * b;
        long dy = 4 * (b1 + 1) * a * a;
        long err = dx + dy + b1 * a * a;

        if (x0 > x1) { x0 = x1; x1 += a; }
        if (y0 > y1) y0 = y1;
        y0 += (b + 1) / 2;
        y1 = y0 - b1;
        a *= 8 * a;
        b1 = 8 * b * b;

        do {
            luna8_pset(x1, y0, color); luna8_pset(x0, y0, color);
            luna8_pset(x0, y1, color); luna8_pset(x1, y1, color);
            long e2 = 2 * err;
            if (e2 <= dy) { y0++; y1--; err += dy += a; }
            if (e2 >= dx || 2 * err > dy) { x0++; x1--; err += dx += b1; }
        } while (x0 <= x1);

        while (y0 - y1 < b) {
            luna8_pset(x0 - 1, y0, color); luna8_pset(x1 + 1, y0++, color);
            luna8_pset(x0 - 1, y1, color); luna8_pset(x1 + 1, y1--, color);
        }
    }
}


/* --- input --- */

static const int MAP_P1[BTN_COUNT] = {
    KEY_A, KEY_D, KEY_W, KEY_S, KEY_J, KEY_K
};
static const int MAP_P2[BTN_COUNT] = {
    KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN, KEY_KP_4, KEY_KP_5
};

static const int* map_for(int player) {
    switch (player) {
        case 0: return MAP_P1;
        case 1: return MAP_P2;
        default: return NULL;
    }
}

int luna8_btn(int id, int player) {
    if (id < 0 || id >= BTN_COUNT) return 0;
    const int* m = map_for(player);
    if (!m) return 0;
    return IsKeyDown(m[id]) ? 1 : 0;
}

int luna8_btnp(int id, int player) {
    if (id < 0 || id >= BTN_COUNT) return 0;
    const int* m = map_for(player);
    if (!m) return 0;
    return IsKeyPressed(m[id]) ? 1 : 0;
}


/* --- sprites --- */

typedef struct {
    uint8_t* data;
    int w;
    int h;
} L8Sprite;

static L8Sprite g_sprites[SPR_COUNT];

void luna8_sprdata(int id, const char* data, int w, int h) {
    if (id < 0 || id >= SPR_COUNT) return;
    if (!data || w <= 0 || h <= 0 || w > SPR_MAX_W || h > SPR_MAX_H) return;

    L8Sprite* s = &g_sprites[id];
    if (s->data) { free(s->data); s->data = NULL; }
    s->w = 0;
    s->h = 0;

    size_t total = (size_t)w * (size_t)h;
    uint8_t* buf = (uint8_t*)calloc(total, 1);
    if (!buf) return;

    /* парсим: '0'..'7' = palette index, всё остальное (пробелы, \n, \r) — пропускаем */
    size_t n = 0;
    for (const char* p = data; *p && n < total; p++) {
        char c = *p;
        if (c >= '0' && c <= '7') {
            buf[n++] = (uint8_t)(c - '0');
        }
    }

    s->data = buf;
    s->w = w;
    s->h = h;
}

void luna8_spr(int id, int x, int y, int flip_x, int flip_y, int colorkey) {
    if (id < 0 || id >= SPR_COUNT) return;

    const L8Sprite* s = &g_sprites[id];
    if (!s->data || s->w <= 0 || s->h <= 0) return;

    for (int row = 0; row < s->h; row++) {
        for (int col = 0; col < s->w; col++) {
            int sx = flip_x ? (s->w - 1 - col) : col;
            int sy = flip_y ? (s->h - 1 - row) : row;
            uint8_t c = s->data[sy * s->w + sx];
            if (colorkey >= 0 && c == (uint8_t)colorkey) continue;
            luna8_pset(x + col, y + row, (int)c);
        }
    }
}


/* --- time --- */

static int64_t now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64_t)ts.tv_sec * 1000000000LL + (int64_t)ts.tv_nsec;
}

static int64_t g_time_start_ns = 0;

void luna8_time_reset(void) {
    g_time_start_ns = now_ns();
}

int64_t luna8_time(void) {
    return (now_ns() - g_time_start_ns) / 1000000LL;
}

int64_t luna8_utime(void) {
    return (int64_t)time(NULL);
}


/* --- audio --- */

typedef struct {
    Sound sound;
    int   ready;
} ToneChannel;

static ToneChannel g_channels[TONE_CHANNELS];
static short       g_buffer[TONE_MAX_SAMPLES];   /* один на всё, 176 КБ */
static int         g_audio_ready = 0;
static int         g_seeded      = 0;

void luna8_audio_init(void) {
    InitAudioDevice();
    g_audio_ready = IsAudioDeviceReady();
}

void luna8_audio_close(void) {
    for (int i = 0; i < TONE_CHANNELS; i++) {
        if (g_channels[i].ready) {
            StopSound(g_channels[i].sound);
            UnloadSound(g_channels[i].sound);
            g_channels[i].ready = 0;
        }
    }
    if (g_audio_ready) CloseAudioDevice();
    g_audio_ready = 0;
}

static float env(int i, int n) {
    if (i < TONE_FADE)       return (float)i / TONE_FADE;
    if (i >= n - TONE_FADE)  return (float)(n - i) / TONE_FADE;
    return 1.0f;
}

static void gen_pulse(short *s, int n, float freq, float vol, float duty) {
    float high  = 1.0f;
    float low   = -duty / (1.0f - duty);   /* DC = 0 */
    float inc   = freq / (float)TONE_SAMPLE_RATE;
    float gain  = vol / 100.0f * 32767.0f;
    float phase = 0.0f;
    for (int i = 0; i < n; i++) {
        float v = (phase < duty) ? high : low;
        s[i] = (short)(v * env(i, n) * gain);
        phase += inc;
        if (phase >= 1.0f) phase -= 1.0f;
    }
}

static void gen_triangle(short *s, int n, float freq, float vol) {
    float inc   = freq / (float)TONE_SAMPLE_RATE;
    float gain  = vol / 100.0f * 32767.0f;
    float phase = 0.0f;
    for (int i = 0; i < n; i++) {
        float v = 2.0f * fabsf(2.0f * phase - 1.0f) - 1.0f;
        s[i] = (short)(v * env(i, n) * gain);
        phase += inc;
        if (phase >= 1.0f) phase -= 1.0f;
    }
}

static void gen_noise(short *s, int n, float vol) {
    float gain = vol / 100.0f * 32767.0f;
    for (int i = 0; i < n; i++) {
        float v = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
        s[i] = (short)(v * env(i, n) * gain);
    }
}

void luna8_tone(int frequency, int duration, int volume, int channel) {
    if (channel < 0 || channel >= TONE_CHANNELS) return;
    if (!g_audio_ready) return;
    if (volume < 0)   volume = 0;
    if (volume > 100) volume = 100;

    ToneChannel* ch = &g_channels[channel];

    /* freq/vol/dur <= 0 — стоп канала */
    if (frequency <= 0 || volume <= 0 || duration <= 0) {
        if (ch->ready) {
            StopSound(ch->sound);
            UnloadSound(ch->sound);
            ch->ready = 0;
        }
        return;
    }

    if (!g_seeded) { srand((unsigned)time(NULL)); g_seeded = 1; }

    int n = (int)((float)TONE_SAMPLE_RATE * (duration / 60.0f));
    if (n > TONE_MAX_SAMPLES) n = TONE_MAX_SAMPLES;
    if (n < 2 * TONE_FADE) return;

    switch (channel) {
        case 0: gen_pulse(g_buffer, n, (float)frequency, (float)volume, 0.125f); break;
        case 1: gen_pulse(g_buffer, n, (float)frequency, (float)volume, 0.250f); break;
        case 2: gen_triangle(g_buffer, n, (float)frequency, (float)volume);      break;
        case 3: gen_noise(g_buffer, n, (float)volume);                           break;
    }

    /* точная длина n — как в старом коде, без UpdateSound */
    Wave wave = {
        .frameCount = (unsigned int)n,
        .sampleRate = TONE_SAMPLE_RATE,
        .sampleSize = 16,
        .channels   = 1,
        .data       = g_buffer,
    };
    Sound s = LoadSoundFromWave(wave);
    if (s.frameCount == 0) return;

    if (ch->ready) {
        StopSound(ch->sound);
        UnloadSound(ch->sound);
    }
    ch->sound = s;
    ch->ready = 1;
    PlaySound(s);
}


/* --- save --- */

void luna8_dset(int slot, int64_t value) {
    luna8_save_dset(slot, value);
}

int64_t luna8_dget(int slot) {
    return luna8_save_dget(slot);
}