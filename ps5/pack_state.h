/* SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once
#define CTPS5_PACK_LIMIT 64
void ctps5_pack_states_refresh(void);
int ctps5_pack_enabled(const char *id);
int ctps5_pack_set_enabled(const char *id,int enabled);
