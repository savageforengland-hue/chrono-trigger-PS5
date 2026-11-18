/* Headless smoke test: uses stand-in SDL filesystem/PNG surfaces. */
#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL3/SDL.h"
#include "fullpng.h"

void rt_log(const char *format, ...) { (void)format; }
void *SDL_malloc(size_t bytes) { return malloc(bytes); }
void *SDL_calloc(size_t n, size_t bytes) { return calloc(n, bytes); }
void SDL_free(void *p) { free(p); }
const char *SDL_GetError(void) { return "mock error"; }

bool SDL_GetPathInfo(const char *path, SDL_PathInfo *info) {
    (void)path;
    info->type = SDL_PATHTYPE_DIRECTORY;
    return true;
}
bool SDL_EnumerateDirectory(const char *path, SDL_EnumerateDirectoryCallback cb, void *userdata) {
    cb(userdata, path, "0x1f0_bg2_x2.png");
    cb(userdata, path, "evil/../../file.png");
    cb(userdata, path, "0x1f0_bg2_x9.png");
    return true;
}
void *SDL_LoadFile(const char *path, size_t *datasize) {
    if (!strstr(path, "0x1f0_bg2_x2.cfg")) return NULL;
    const char data[] = "origin_x=100\norigin_y=-20\n";
    *datasize = sizeof data - 1;
    char *contents = malloc(sizeof data);
    assert(contents);
    memcpy(contents, data, sizeof data);
    return contents;
}
SDL_Surface *SDL_LoadPNG(const char *path) {
    if (!strstr(path, "0x1f0_bg2_x2.png")) return NULL;
    SDL_Surface *s = (SDL_Surface *)calloc(1, sizeof *s);
    assert(s);
    s->w = s->h = 4;
    s->pitch = 16;
    s->format = SDL_PIXELFORMAT_ABGR8888;
    s->pixels = malloc(64);
    assert(s->pixels);
    for (int y = 0; y < 4; ++y)
        for (int x = 0; x < 4; ++x)
            ((uint32_t *)s->pixels)[y * 4 + x] =
                x < 2 ? 0xFF0000FFu : 0xFF00FF00u; /* ABGR red / green */
    return s;
}
SDL_Surface *SDL_ConvertSurface(SDL_Surface *src, uint32_t format) {
    SDL_Surface *out = (SDL_Surface *)malloc(sizeof *out);
    assert(out);
    *out = *src;
    out->pixels = malloc(64);
    assert(out->pixels);
    memcpy(out->pixels, src->pixels, 64);
    out->format = format;
    return out;
}
void SDL_DestroySurface(SDL_Surface *s) {
    if (!s) return;
    free(s->pixels);
    free(s);
}

int main(void) {
    fullpng_load("/tmp/mods/graphics/");
    assert(fullpng_find(0x1f0, 2) == 1);
    assert(fullpng_find(0x1f0, 1) == 0);
    assert(fullpng_find(0x1ef, 2) == 0);
    assert(fullpng_max_scale() == 2);
    const FullPng *p = fullpng_surface(1);
    assert(p && p->location == 0x1f0 && p->bg == 2);
    assert(p->w == 2 && p->h == 2 && p->scale == 2);
    assert(p->origin_x == 100 && p->origin_y == -20);
    uint32_t left = fullpng_source(1, 100, -20);
    uint32_t right = fullpng_source(1, 101, -20);
    assert(fullpng_is_source(left));
    assert(fullpng_id(right) == 1);
    assert(fullpng_x(right) == 1 && fullpng_y(right) == 0);
    assert(fullpng_native_color(left) == 0xFF0000FFu);
    assert(fullpng_sample(right, 0, 0, 2) == 0xFF00FF00u);
    assert(fullpng_sample(left, 2, 0, 2) == 0);
    assert(fullpng_source(1, 99, -20) == 0);
    assert(fullpng_source(1, 102, -20) == 0);
    assert(fullpng_source(0, 0, 0) == 0);
    assert(fullpng_surface(127) == NULL);
    fullpng_load(NULL);
    assert(fullpng_find(0x1f0, 2) == 0);
    puts("PASS: full-canvas PNG loader, source coordinates, sampling and fallback");
    return 0;
}
