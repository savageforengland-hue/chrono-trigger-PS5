/* SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once
#include <stddef.h>
#include "battle_hud_draw.h"
/* Read-only USA/YQUE battle snapshot. Call on the game thread between frames.
 * Returns zero during incomplete/invalid battle state; never reads outside RAM. */
int ct_hud_read_state(CtHudState *state, const unsigned char *ram, size_t size);
