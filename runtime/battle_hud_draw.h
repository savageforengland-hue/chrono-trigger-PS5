/* SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once
#include <stdint.h>
#define CT_HUD_W 480
#define CT_HUD_H 80
#define CT_CMD_W 240
#define CT_CMD_H 54
#define CT_HUD_FACE 48
#define CT_HUD_MEMBERS 3
typedef struct CtHudMember {
    int present, character;
    unsigned hp, max_hp, mp, max_mp, atb, atb_max;
    char name[24];
} CtHudMember;
typedef struct CtHudState { CtHudMember member[CT_HUD_MEMBERS]; int selected, command; } CtHudState;
/* Faces are ABGR8888 menu portraits. No ROM artwork is compiled into this module. */
void ct_hud_draw(uint32_t pixels[CT_HUD_W * CT_HUD_H], const CtHudState *state,
                 const uint32_t faces[7][CT_HUD_FACE * CT_HUD_FACE]);

/* Original game menu icons: sword, Tech lightning, inventory bag. */
void ct_hud_draw_commands(uint32_t pixels[CT_CMD_W*CT_CMD_H], int command,
                          const uint32_t icons[3][32*32]);
