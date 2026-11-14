/* SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once
/* Changes the verified ROM's graphics/text in memory before the game starts.
 * No on-disk ROM changes or bundled game assets. Uses the saved host bindings. */
int egg_ps5_game_prompts_init(void);
/* Authored monochrome art, exposed for ROM-free rendering tests. */
void egg_ps5_game_prompt_mask(const char *label, unsigned char *out, int width, int height, int compact);
