/* Pure ABGR8888 full PNG alpha composite, for deterministic host tests.
 * SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once
#include <stdint.h>
static inline uint32_t fullpng_blend_abgr(uint32_t fore, uint32_t original, unsigned alpha) {
    if (alpha == 0) return original;
    if (alpha >= 255) return fore | 0xFF000000u;
    uint32_t out = 0xFF000000u;
    for (unsigned shift = 0; shift < 24; shift += 8) {
        unsigned v = (((fore >> shift) & 255u) * alpha +
                      ((original >> shift) & 255u) * (255u - alpha) + 127u) / 255u;
        out |= (uint32_t)v << shift;
    }
    return out;
}
