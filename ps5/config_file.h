/* SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once
/* Atomic one-key update; preserve unrelated keys/comments. 0 leaves original intact. */
int egg_ps5_ini_set(const char *path,const char *key,const char *value);
