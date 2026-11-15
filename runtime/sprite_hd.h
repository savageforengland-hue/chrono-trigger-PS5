/* SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once
#include <stdint.h>
/* Palette-relative texel: base index, neighbour index, blend/256. Zero is
 * transparent. Colours remain live, including battle flashes and fades. */
void sprite_hd_make(uint32_t *out,const uint8_t tile[32],int scale,int strength,
                    const uint32_t palette[16]);
uint32_t sprite_hd_color(uint32_t texel,const uint16_t *palette);
void sprite_hd_load(void);
int sprite_hd_battle(void);
int sprite_hd_classify_palette(const uint16_t palette[16]);
void sprite_hd_begin_frame(const uint8_t *palette,int battle);
int sprite_hd_palette(int engine,int row);
