/* SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once
#include <SDL3/SDL.h>
#include <stdint.h>
void ctps5_battle_hud_init(void);
/* Game-thread observations of the original normal HP header and first HP digit.
 * Hidden/Tech/Item panels do not get replaced. No game memory is modified. */
void ctps5_battle_hud_entry(uint32_t entry);
void ctps5_battle_hud_cell(uint32_t entry, int ds_row);
void ctps5_battle_hud_frame(void);
/* Host thread: opaque replacement area and aspect-preserved portrait image.
 * game is immutable. zoom_ui: 0 unzoomed, 1 original-size UI, 2 smaller UI. */
int ctps5_battle_hud_prepare(SDL_Renderer *ren, SDL_Texture **texture,
    const SDL_FRect *game, int columns, int rows, int zoom_ui,
    SDL_FRect *cover, SDL_FRect *image);
