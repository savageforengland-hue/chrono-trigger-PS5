/* SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once
#include <SDL3/SDL.h>
void ctps5_battle_hud_init(void);
void ctps5_battle_hud_frame(void); /* game thread */
/* Called on the host thread. A visible strip docks below the complete game
 * image; its reserved area never obscures command, item or target selection. */
int ctps5_battle_hud_prepare(SDL_Renderer *ren, SDL_Texture **texture,
                            SDL_FRect *game, SDL_FRect *hud);
