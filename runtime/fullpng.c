/* Full-canvas PNG backgrounds keyed by the game's location and BG layer.
 * Uses immutable, separately allocated RGBA surfaces rather than tile hashes.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <SDL3/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fullpng.h"
#include "rt.h"

#define FULLPNG_IMAGE_BUDGET (128u * 1024u * 1024u)
#define FULLPNG_DISCOVERY_MAX 256

typedef struct Candidate {
    char path[1024];
    int location, bg, scale;
} Candidate;
typedef struct Candidates {
    Candidate items[FULLPNG_DISCOVERY_MAX];
    int count;
} Candidates;

static FullPng images[FULLPNG_MAX_SURFACES + 1];
static int image_count, largest_scale;
static size_t used_bytes;

static SDL_EnumerationResult SDLCALL collect_room(void *userdata, const char *directory, const char *name) {
    Candidates *c = (Candidates *)userdata;
    unsigned location = 0;
    int bg = -1, scale = 0, consumed = -1;
    if (sscanf(name, "0x%x_bg%d_x%d.png%n", &location, &bg, &scale, &consumed) != 3 ||
        consumed < 0 || (size_t)consumed != strlen(name) || location > 0xffff ||
        bg < 0 || bg > 3 || scale < 2 || scale > 8)
        return SDL_ENUM_CONTINUE;
    if (c->count >= FULLPNG_DISCOVERY_MAX) return SDL_ENUM_CONTINUE;
    Candidate *entry = &c->items[c->count];
    int n = snprintf(entry->path, sizeof entry->path, "%s%s", directory, name);
    if (n <= 0 || (size_t)n >= sizeof entry->path) return SDL_ENUM_CONTINUE;
    entry->location = (int)location;
    entry->bg = bg;
    entry->scale = scale;
    c->count++;
    return SDL_ENUM_CONTINUE;
}

static int candidate_order(const void *aa, const void *bb) {
    return strcmp(((const Candidate *)aa)->path, ((const Candidate *)bb)->path);
}

static void reset_images(void) {
    for (int i = 1; i <= image_count; i++) {
        SDL_free((void *)images[i].rgba);
        images[i] = (FullPng){0};
    }
    image_count = largest_scale = 0;
    used_bytes = 0;
}

int fullpng_max_scale(void) { return largest_scale; }
int fullpng_find(int location, int bg) {
    /* Later filenames win when multiple copies target the same layer. */
    for (int i = image_count; i > 0; --i)
        if (images[i].location == location && images[i].bg == bg) return i;
    return 0;
}
const FullPng *fullpng_surface(unsigned id) {
    return id > 0 && id <= (unsigned)image_count ? &images[id] : NULL;
}
uint32_t fullpng_source(unsigned id, int x, int y) {
    const FullPng *p = fullpng_surface(id);
    if (!p) return 0;
    /* Transform from game-world coordinates to the PNG's own origin.
     * This keeps camera scrolling separate from artist asset alignment. */
    int64_t local_x = (int64_t)x - p->origin_x;
    int64_t local_y = (int64_t)y - p->origin_y;
    if (local_x < 0 || local_y < 0 || local_x >= p->w || local_y >= p->h ||
        local_x > FULLPNG_XY_MASK || local_y > FULLPNG_XY_MASK)
        return 0;
    return FULLPNG_FLAG | (id << 24) | ((uint32_t)local_x << 12) | (uint32_t)local_y;
}
uint32_t fullpng_sample(uint32_t source, int sx, int sy, int k) {
    if (!fullpng_is_source(source) || k <= 0 || sx < 0 || sy < 0 || sx >= k || sy >= k) return 0;
    const FullPng *p = fullpng_surface(fullpng_id(source));
    if (!p || !p->rgba) return 0;
    unsigned x = fullpng_x(source), y = fullpng_y(source);
    if (x >= (unsigned)p->w || y >= (unsigned)p->h) return 0;
    int scale = p->scale;
    size_t offset = ((size_t)y * (size_t)scale + (size_t)sy * (size_t)scale / (size_t)k) *
                    (size_t)p->w * (size_t)scale +
                    (size_t)x * (size_t)scale + (size_t)sx * (size_t)scale / (size_t)k;
    return p->rgba[offset];
}
uint32_t fullpng_native_color(uint32_t source) {
    const FullPng *p = fullpng_surface(fullpng_id(source));
    return p ? fullpng_sample(source, p->scale / 2, p->scale / 2, p->scale) : 0;
}

/* Sidecar: same basename as the PNG, ending .cfg.
 * Supported keys: origin_x and origin_y, in native game pixels.
 * Malformed sidecars use (0,0). A valid sidecar does not modify any ROM data. */
static void read_origin(const char *png_path, int *x, int *y) {
    char path[1024];
    size_t n = strlen(png_path);
    if (n < 4 || n + 1 >= sizeof path) return;
    memcpy(path, png_path, n - 4);
    memcpy(path + n - 4, ".cfg", 5);
    size_t bytes = 0;
    char *data = SDL_LoadFile(path, &bytes);
    if (!data) return;
    if (bytes > 4096) { SDL_free(data); return; }
    char *line = data;
    while (*line) {
        char *end = strchr(line, '\n');
        if (end) *end++ = '\0';
        char *key = line;
        while (*key == ' ' || *key == '\t') key++;
        int *target = NULL;
        if (!strncmp(key, "origin_x=", 9)) { target = x; key += 9; }
        else if (!strncmp(key, "origin_y=", 9)) { target = y; key += 9; }
        if (target) {
            char *tail = NULL;
            long value = strtol(key, &tail, 10);
            if (tail != key && (*tail == '\0' || *tail == '\r') &&
                value >= -4096 && value <= 4096)
                *target = (int)value;
        }
        if (!end) break;
        line = end;
    }
    SDL_free(data);
}

void fullpng_load(const char *graphics_root) {
    reset_images();
    if (!graphics_root || !*graphics_root) return;
    char directory[1024];
    int n = snprintf(directory, sizeof directory, "%srooms/", graphics_root);
    if (n <= 0 || (size_t)n >= sizeof directory) return;
    SDL_PathInfo info;
    if (!SDL_GetPathInfo(directory, &info) || info.type != SDL_PATHTYPE_DIRECTORY) return;
    /* The candidate list is on the heap: no large stack allocation on PS5 startup. */
    Candidates *c = (Candidates *)SDL_calloc(1, sizeof *c);
    if (!c) return;
    if (!SDL_EnumerateDirectory(directory, collect_room, c)) {
        rt_log("fullpng: failed to enumerate %s: %s", directory, SDL_GetError());
        SDL_free(c);
        return;
    }
    qsort(c->items, (size_t)c->count, sizeof c->items[0], candidate_order);
    for (int i = 0; i < c->count && image_count < FULLPNG_MAX_SURFACES; ++i) {
        const Candidate *candidate = &c->items[i];
        SDL_Surface *s = SDL_LoadPNG(candidate->path);
        if (!s) {
            rt_log("fullpng: cannot load %s: %s", candidate->path, SDL_GetError());
            continue;
        }
        int scale = candidate->scale;
        if (s->w <= 0 || s->h <= 0 || s->w % scale || s->h % scale ||
            s->w / scale > (int)FULLPNG_XY_MASK + 1 || s->h / scale > (int)FULLPNG_XY_MASK + 1) {
            rt_log("fullpng: invalid dimensions for %s (%dx%d, x%d)", candidate->path, s->w, s->h, scale);
            SDL_DestroySurface(s);
            continue;
        }
        size_t bytes = (size_t)s->w * (size_t)s->h * 4u;
        if (bytes > FULLPNG_IMAGE_BUDGET || used_bytes > FULLPNG_IMAGE_BUDGET - bytes) {
            rt_log("fullpng: memory budget exceeded by %s", candidate->path);
            SDL_DestroySurface(s);
            continue;
        }
        SDL_Surface *converted = SDL_ConvertSurface(s, SDL_PIXELFORMAT_ABGR8888);
        SDL_DestroySurface(s);
        if (!converted) continue;
        uint32_t *pixels = (uint32_t *)SDL_malloc(bytes);
        if (pixels) {
            for (int y = 0; y < converted->h; ++y)
                memcpy((uint8_t *)pixels + (size_t)y * (size_t)converted->w * 4u,
                       (const uint8_t *)converted->pixels + (size_t)y * (size_t)converted->pitch,
                       (size_t)converted->w * 4u);
            FullPng *dst = &images[++image_count];
            int origin_x = 0, origin_y = 0;
            read_origin(candidate->path, &origin_x, &origin_y);
            *dst = (FullPng){candidate->location, candidate->bg, scale,
                             converted->w / scale, converted->h / scale,
                             origin_x, origin_y, pixels};
            used_bytes += bytes;
            if (scale > largest_scale) largest_scale = scale;
            rt_log("fullpng: location 0x%x BG%d canvas %dx%d at x%d: %s",
                   dst->location, dst->bg, dst->w, dst->h, scale, candidate->path);
        }
        SDL_DestroySurface(converted);
    }
    SDL_free(c);
    if (image_count) rt_log("fullpng: %d full-canvas layers loaded (%zu MiB budget used)",
                             image_count, used_bytes / (1024u * 1024u));
}
