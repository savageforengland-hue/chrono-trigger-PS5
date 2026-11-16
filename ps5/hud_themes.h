/* SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once
#include "../runtime/hud_theme.h"
void ctps5_hud_themes_init(void);
void ctps5_hud_themes_refresh(void);
CtHudTheme *ctps5_hud_theme(void); /* host thread only */
const char *ctps5_hud_theme_name(void);
int ctps5_hud_theme_change(int direction); /* 0 reloads; +/- cycles */
