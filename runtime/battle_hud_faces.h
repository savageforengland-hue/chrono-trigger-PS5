/* SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once
#include <stdint.h>
/* Decode all seven portraits from the player's menu files. No artwork bundled. */
int ct_hud_load_faces(uint32_t faces[7][48*48]);

int ct_hud_load_command_icons(uint32_t icons[3][32*32]);
