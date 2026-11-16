/* Graphics mods (gfx_mod.c): PNGs in mods/graphics, at the dump's paths, replacing the game's graphics. */
#pragma once

#include <stdint.h>

/* At start, after the game is built and before it runs (while Mods is on): applies the same-size images and
 * edited palettes to the ROM in memory, and loads the larger (HD) ones. Host thread. */
void gfxmod_load(void);

/* HD art for one 8x8 tile: scale * 8 pixels square, unflipped; indexed (palette indexes as the tile's own: 0-15
 * in its row, or 0-255; 0 transparent) or RGBA (ABGR8888, alpha 0 transparent). */
typedef struct GfxHdTile { int scale, indexed, bpp8; const void *px; int scope; unsigned characters; } GfxHdTile;
/* Authored palette-relative sprite texels (indexed=2). Scope 1 additionally
 * requires a battle, sprite layer and matching player palette/character mask.
 * Scope 2 is a room background; scope 3 a room actor. Indexed=4 is painted
 * room RGBA with optional native palette brightness response. */
int gfxmod_add_sprite(const uint8_t tile[32],int scale,uint32_t *pixels,int character);
void gfxmod_sprite_character(const uint8_t tile[32],int character);
/* A frame-scoped tile has no native-byte key: only the matched whole-pose
 * renderer may reference it. indexed=3 holds RGBA pilot pixels (alpha254 is
 * an outline tag); redraw.c applies live outline/flat-palette effects.
 * Takes ownership of pixels. */
uint32_t gfxmod_add_direct(int scale,int indexed,void *pixels);

/* The scale HD frames are drawn at: the largest of the HD tiles (0: none loaded). Any thread. */
int gfxmod_hd_scale(void);
int gfxmod_hd_active(int battle);
/* The HD tile replacing the native tile with these bytes (32, or 64 at 8 bits per pixel); 0 if none. Any thread
 * (the table does not change after gfxmod_load). */
uint32_t gfxmod_find(const uint8_t *tile, int bpp8);
const GfxHdTile *gfxmod_tile(uint32_t n);
uint32_t gfxmod_fallback(uint32_t n);
/* Optional backgrounds.json limits Map/Bg and opted-in Chara sheets to listed locations. */
void gfxmod_background_location(int location);
int gfxmod_background_allowed(void);
int gfxmod_window_probe(int *bg,int *x,int *y,int *control);
void gfxmod_window_state(int valid,int tile,uint64_t frame);
int gfxmod_window_gain(void); /* 256 daylight; applies to world textures, not HUD/minimap. */
void gfxmod_background_gains(const uint16_t *palette,uint16_t gains[32]);
static inline int gfxmod_allowed(const GfxHdTile*t,int sprite,int battle,int character){
    return t&&(!t->scope||(((t->scope==2&&!sprite)||(t->scope==3&&sprite))&&gfxmod_background_allowed())||(t->scope==1&&sprite&&battle&&character>=0&&character<7&&(t->characters&(1u<<character))));
}
