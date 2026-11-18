/* Tile-ring to world-pixel coordinates for full PNG backgrounds.
 * No SDL or game state: independently testable camera math.
 * SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once
/* ring_tiles must be a power of two; 64 horizontal, 32 vertical on DS BGs. */
static inline int fullpng_ring_origin(int camera_tile, int ring_slot, unsigned scroll, unsigned ring_tiles) {
    int delta = ((int)(scroll >> 3) - ring_slot) & (int)(ring_tiles - 1);
    if (delta >= (int)(ring_tiles / 2)) delta -= (int)ring_tiles;
    return (camera_tile + delta) * 8 + (int)(scroll & 7u);
}
