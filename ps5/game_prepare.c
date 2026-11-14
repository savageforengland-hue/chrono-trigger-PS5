/* Local PS5 AOT boot path. SHA-1 implementation from Power Pudu's MIT-licensed game_build.c. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL3/SDL.h>
#include "dispatch.h"
#include "display.h"
#include "game_build.h"
#include "rt.h"
#include "ui_font.h"

static uint32_t rol(uint32_t v, int s) { return v << s | v >> (32 - s); }

static void sha1_block(uint32_t h[5], const uint8_t *p) {
    uint32_t w[80];
    for (int i = 0; i < 16; i++) w[i] = (uint32_t)p[4 * i] << 24 | p[4 * i + 1] << 16 | p[4 * i + 2] << 8 | p[4 * i + 3];
    for (int i = 16; i < 80; i++) w[i] = rol(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
    uint32_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4];
    for (int i = 0; i < 80; i++) {
        uint32_t f, k;
        if (i < 20) f = (b & c) | (~b & d), k = 0x5A827999u;
        else if (i < 40) f = b ^ c ^ d, k = 0x6ED9EBA1u;
        else if (i < 60) f = (b & c) | (b & d) | (c & d), k = 0x8F1BBCDCu;
        else f = b ^ c ^ d, k = 0xCA62C1D6u;
        uint32_t t = rol(a, 5) + f + e + k + w[i];
        e = d, d = c, c = rol(b, 30), b = a, a = t;
    }
    h[0] += a, h[1] += b, h[2] += c, h[3] += d, h[4] += e;
}

static void sha1_hex(const uint8_t *data, size_t n, char out[41]) {
    uint32_t h[5] = {0x67452301u, 0xEFCDAB89u, 0x98BADCFEu, 0x10325476u, 0xC3D2E1F0u};
    size_t full = n & ~(size_t)63;
    for (size_t i = 0; i < full; i += 64) sha1_block(h, data + i);
    uint8_t tail[128] = {0};
    size_t rest = n - full, len = rest < 56 ? 64 : 128;
    memcpy(tail, data + full, rest);
    tail[rest] = 0x80;
    for (int i = 0; i < 8; i++) tail[len - 1 - i] = (uint8_t)((uint64_t)n * 8 >> (8 * i));
    for (size_t i = 0; i < len; i += 64) sha1_block(h, tail + i);
    for (int i = 0; i < 5; i++) snprintf(out + 8 * i, 9, "%08x", h[i]);
}

static int startup_error(const char *detail) {
    rt_log("startup failed: %s", detail);
    static uint32_t screen[SCREEN_W * SCREEN_H];
    for (unsigned i = 0; i < SDL_arraysize(screen); ++i) screen[i] = 0xff101018u;
    const char *lines[] = {"Egg of Time - PS5", detail, "Copy your USA DS ROM to:",
        "/data/eggoftime/rom/CT.nds", "See eggoftime.log for details."};
    for (int i = 0; i < 5; ++i)
        ui_draw_text(screen, lines[i], 4, 40 + 22 * i, 0xffffffffu);
    const uint32_t *screens[3] = {screen, screen, screen};
    Uint64 until = SDL_GetTicks() + 15000;
    while (SDL_GetTicks() < until) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) if (e.type == SDL_EVENT_QUIT) return 0;
        display_frame(screens, NULL, NULL, NULL, SCREEN_W, SCREEN_H, 1, 0, 0, NULL);
        SDL_Delay(16);
    }
    return 0;
}

int game_build_prepare(const char *rom_arg) {
    const char *path = rom_arg && rom_arg[0] ? rom_arg : "/data/eggoftime/rom/CT.nds";
    if (!rt_load_rom(path)) return startup_error("ROM could not be loaded.");
    char sha[41];
    sha1_hex(rt_rom.data, rt_rom.size, sha);
    if (rt_rom.size != 134217728 || strcmp(sha, "11f9e8204e484eab42181b210c9f214584be75bf")) {
        rt_log("ROM rejected: size=%zu SHA-1=%s", rt_rom.size, sha);
        free(rt_rom.data);
        rt_rom.data = NULL;
        return startup_error("Wrong ROM revision.");
    }
    const char *error = NULL;
    if (!rt_game_load("static", &error)) return startup_error(error);
    rt_log("USA ROM verified; 9022 functions compiled into the PS5 executable");
    return 1;
}
