/* SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once
#include <SDL3/SDL.h>
#include "battle_hud_draw.h"
#define CT_THEME_PATH 192
#define CT_THEME_IMAGES 23
enum {TH_BG,TH_CARD,TH_FRAME,TH_OVERLAY,TH_PORTRAIT_FRAME,TH_FONT,TH_TRACK,TH_HP,TH_MP,TH_ATB,TH_READY,TH_ATTACK,TH_TECH,TH_ITEM,TH_CRONO,TH_MARLE,TH_LUCCA,TH_ROBO,TH_FROG,TH_AYLA,TH_MAGUS,TH_UNUSED1,TH_UNUSED2};
enum {TC_BG,TC_TEXT,TC_LABEL,TC_BORDER,TC_DIVIDER,TC_FACE_BORDER,TC_SELECTED,TC_TRACK,TC_HP,TC_MP,TC_ATB,TC_READY,TC_LOW_HP,TC_OUTLINE,TC_KO,TC_COLOURS};
typedef struct CtThemeRect {int x,y,w,h;} CtThemeRect;
typedef struct CtThemePoint {int x,y;} CtThemePoint;
typedef struct CtThemeImage {SDL_Surface *surface;} CtThemeImage;
typedef struct CtHudThemeConfig {
 int width,height,scale,gap,padding,border,grain,clocks,portrait_fit,linear,opacity;
 int background_mode,background_tint,background_colour_set;
 uint32_t colour[TC_COLOURS];
 CtThemeRect portrait,hp_bar,mp_bar,atb_bar;
 CtThemePoint name,hp_label,hp_value,mp_label,mp_value,atb_label;
 int name_size,value_size,label_size,show_faces,show_names,show_labels,show_numbers,show_hp,show_mp,show_atb,show_dividers,show_selection,show_commands;
 int font_columns,font_rows,font_first,font_count,font_tint;
 int max_width_percent,offset_x,offset_y,command_gap;
 char name_label[48],labels[6][16],names[7][24];
 char files[CT_THEME_IMAGES][CT_THEME_PATH];
} CtHudThemeConfig;
typedef struct CtHudTheme {
 CtHudThemeConfig cfg;CtThemeImage image[CT_THEME_IMAGES];size_t asset_bytes;
 uint32_t *pixels,*commands;SDL_Surface *surface,*command_surface;int pixel_w,pixel_h,command_w,command_h,overrides;
} CtHudTheme;
typedef void (*CtThemeLog)(const char *message);
void ct_hud_theme_defaults(CtHudThemeConfig *cfg);
int ct_hud_theme_parse(const char *json,size_t length,CtHudThemeConfig *cfg,char *error,size_t capacity);
/* root: chosen theme; fallback: active graphics/hud PNG overrides. No JSON
 * is needed for standard PNG names. Missing/invalid PNGs retain ROM assets.
 * NULL with empty error means no overrides; NULL with error means bad JSON. */
CtHudTheme *ct_hud_theme_load(const char *root,const char *fallback,char *error,size_t capacity,CtThemeLog log);
void ct_hud_theme_free(CtHudTheme *theme);
const uint32_t *ct_hud_theme_draw(CtHudTheme *theme,const CtHudState *state,const uint32_t faces[7][48*48],const uint32_t *skin);
const uint32_t *ct_hud_theme_commands(CtHudTheme *theme,int selected,const uint32_t icons[3][32*32]);
/* Default original-art aliases produced by the graphics dumper/editor. */
const char *ct_hud_theme_asset_name(int index);

/* Highest-priority image fallback first; explicit selected theme stays on top. */
CtHudTheme *ct_hud_theme_load_stack(const char *root,const char *const *fallbacks,int count,char *error,size_t capacity,CtThemeLog log);
