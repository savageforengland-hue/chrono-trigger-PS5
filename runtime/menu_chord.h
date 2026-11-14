/* SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once
/* Reserved L3+R3 chord: one toggle, re-arm only after BOTH sticks release.
 * While latched, consume game inputs to prevent rebound actions leaking. */
typedef struct CtMenuChord { int latched; } CtMenuChord;
static inline int ct_menu_chord_update(CtMenuChord*s,int left,int right){
    if(!left&&!right){s->latched=0;return 0;}
    if(left&&right&&!s->latched){s->latched=1;return 1;}
    return 0;
}
