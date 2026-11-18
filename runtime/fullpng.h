/* Optional full-canvas room backgrounds. Game logic and VRAM tiles remain unchanged.
 * SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once
#include <stdint.h>

#define FULLPNG_FLAG 0x80000000u
#define FULLPNG_XY_MASK 0xFFFu
#define FULLPNG_MAX_SURFACES 127

typedef struct FullPng {
    int location, bg, scale, w, h; /* w/h in original game pixels */
    int origin_x, origin_y;     /* world pixel coordinate of top-left of PNG */
    const uint32_t *rgba;          /* ABGR8888 pixel words, image w*scale by h*scale */
} FullPng;

/* Load at mod startup only, before rendering threads read these immutable surfaces. */
void fullpng_load(const char *graphics_root);
int fullpng_max_scale(void);
int fullpng_find(int location, int bg); /* 0: no replacement */
const FullPng *fullpng_surface(unsigned id);

/* Packed 32-bit per-pixel provenance, carried by VideoHd.tile:
 * bit 31: full PNG, bits 24..30: surface id, 12..23: x, 0..11: y.
 * x/y are the unscaled, world-space canvas coordinates (up to 4095).
 */
static inline int fullpng_is_source(uint32_t source) { return (source & FULLPNG_FLAG) != 0; }
static inline unsigned fullpng_id(uint32_t source) { return (source >> 24) & 127u; }
static inline unsigned fullpng_x(uint32_t source) { return (source >> 12) & FULLPNG_XY_MASK; }
static inline unsigned fullpng_y(uint32_t source) { return source & FULLPNG_XY_MASK; }

uint32_t fullpng_source(unsigned id, int x, int y);
/* Source pixel at the centre of its native game pixel, or 0 outside the image. */
uint32_t fullpng_native_color(uint32_t source);
/* Subpixel at destination upscale coordinate sx/sy in a k by k pixel. */
uint32_t fullpng_sample(uint32_t source, int sx, int sy, int k);
