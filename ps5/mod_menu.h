/* SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once
void egg_ps5_mod_menu_refresh(void);
int egg_ps5_mod_menu_count(void);
const char *egg_ps5_mod_menu_title(int row);
const char *egg_ps5_mod_menu_value(int row);
const char *egg_ps5_mod_menu_description(int row);
void egg_ps5_mod_menu_change(int row,int direction);
