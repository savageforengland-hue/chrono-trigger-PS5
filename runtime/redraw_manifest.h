/* SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once
#include <stddef.h>
#define REDRAW_MANIFEST_FRAMES 512
#define REDRAW_MANIFEST_PATH 192
typedef struct RedrawFrameSpec { int cell,enabled; char file[REDRAW_MANIFEST_PATH]; } RedrawFrameSpec;
typedef struct RedrawManifest { int character,scale,enabled,count; RedrawFrameSpec frames[REDRAW_MANIFEST_FRAMES]; } RedrawManifest;
/* Schema 2. Relative PNG paths only. No filesystem writes or partial success. */
int redraw_manifest_parse(const char *json,size_t length,RedrawManifest *out,char *error,size_t capacity);
