/* SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once
#include <SDL3/SDL.h>
#include "redraw_match.h"
typedef struct RedrawBinding {int action,pose,x,y,w,h,above;} RedrawBinding;
void ct_redraw_load(void);
int ct_redraw_count(void);
void ct_redraw_set_visible(int on);
int ct_redraw_prepare(int engine,const RedrawObject live[128],const uint16_t palette[256]);
const RedrawBinding *ct_redraw_binding(int engine,int slot);
uint32_t ct_redraw_sample(int pose,int x,int y,uint32_t *texel);
uint32_t ct_redraw_color(uint32_t texel,const uint16_t palette[16]);
int ct_redraw_viewer_open(void);
int ct_redraw_viewer_active(void);
void ct_redraw_viewer_close(void);
void ct_redraw_viewer_step(int direction);
const char *ct_redraw_viewer_title(void);
void ct_redraw_viewer_draw(SDL_Renderer *renderer,const SDL_FRect *viewport);
