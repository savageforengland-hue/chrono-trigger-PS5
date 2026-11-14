/* SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once
#include <stdint.h>
#define CT_HUD_W 480
#define CT_HUD_H 64
#define CT_HUD_FACE 48
#define CT_HUD_MEMBERS 3
typedef struct CtHudMember {
    int present, character;
    unsigned hp, max_hp, mp, max_mp, atb, atb_max;
    char name[24];
} CtHudMember;
typedef struct CtHudState { CtHudMember member[CT_HUD_MEMBERS]; } CtHudState;
/* Faces are ABGR8888 menu portraits. No ROM artwork is compiled into this module. */
void ct_hud_draw(uint32_t pixels[CT_HUD_W * CT_HUD_H], const CtHudState *state,
                 const uint32_t faces[7][CT_HUD_FACE * CT_HUD_FACE]);
