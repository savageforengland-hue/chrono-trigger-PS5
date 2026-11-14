/* SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once
#include <SDL3/SDL.h>
#include <stdint.h>
void ctps5_battle_hud_init(void);
void ctps5_battle_hud_entry(uint32_t entry);
void ctps5_battle_hud_cell(uint32_t entry,int ds_column,int ds_row);
void ctps5_battle_hud_frame(void); /* game thread */
int ctps5_battle_hud_enabled(void);
int ctps5_battle_hud_scale(int commands);
int ctps5_battle_hud_set_scale(int commands,int value);
int ctps5_battle_hud_available(void);
int ctps5_battle_hud_set_enabled(int on); /* host: apply and persist */
/* Only the normal icon-command state: Left/Right select action, L1/R1 party.
 * Tech/Item/target selection gets the original input unchanged. */
uint32_t ctps5_battle_hud_input(uint32_t keys);
typedef struct CtHudLayout { SDL_FRect cover,hud,commands; } CtHudLayout;
/* Host: full old normal panel cover, centered portraits and real command icons.
 * The gameplay rectangle is immutable. zoom_ui: 0 original, 1 large UI, 2 small. */
int ctps5_battle_hud_prepare(SDL_Renderer *ren,SDL_Texture **hud,SDL_Texture **commands,
    const SDL_FRect *game,int columns,int rows,int zoom_ui,CtHudLayout *layout);
