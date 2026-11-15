/* SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once
#include <stdint.h>
#define REDRAW_OBJECTS 8
typedef struct RedrawObject {
    int valid,x,y,flip,palette,priority,mode,above,space;
    uint8_t bytes[128]; /* pilot: a complete original 16x16, 4bpp object */
} RedrawObject;
typedef struct RedrawReference {int count;RedrawObject object[REDRAW_OBJECTS];} RedrawReference;
/* Exact group, including wraps, flips, shared origin and palette. No writes
 * to the supplied objects. A partial or duplicate assignment is rejected. */
int redraw_match(const RedrawReference *reference,const RedrawObject live[128],int anchor,int found[REDRAW_OBJECTS]);
