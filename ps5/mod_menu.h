/* SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once
void egg_ps5_mod_menu_refresh(void);
int egg_ps5_mod_menu_count(void);
const char *egg_ps5_mod_menu_title(int row);
const char *egg_ps5_mod_menu_value(int row);
const char *egg_ps5_mod_menu_description(int row);
void egg_ps5_mod_menu_change(int row,int direction);

/* Browser pages keep the legacy setting IDs stable. Navigation never saves. */
enum { MOD_ROOT, MOD_CHARACTERS, MOD_BACKGROUNDS, MOD_HUD, MOD_PACKS, MOD_CHARACTER_PACKS, MOD_PAGE_COUNT };
enum { MOD_PACK_CHARACTERS=1, MOD_PACK_BACKGROUNDS=2 };
int egg_ps5_mod_pack_count(void);
const char *egg_ps5_mod_pack_name(int pack);
int egg_ps5_mod_pack_flags(int pack);
int egg_ps5_mod_pack_selected(int pack);
void egg_ps5_mod_pack_select(int pack);
int egg_ps5_mod_option_available(int setting);
const char *egg_ps5_mod_page_title(int page);
int egg_ps5_mod_page_count(int page);
int egg_ps5_mod_page_target(int page,int row); /* -1 for a setting/pack */
const char *egg_ps5_mod_page_label(int page,int row);
const char *egg_ps5_mod_page_value(int page,int row);
const char *egg_ps5_mod_page_description(int page,int row);
void egg_ps5_mod_page_change(int page,int row,int direction); /* 0 = A */
