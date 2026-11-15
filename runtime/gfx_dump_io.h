/* SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once
#include <stddef.h>
#include <stdint.h>
#include <SDL3/SDL_pixels.h>

/* Native directory handling with an indexed SDL encoder.
 * PNGs retain palette indexes and per-row transparency for round-trip mods.
 * Directories are 0755 and published files 0644 for FTP readers. */
int gfxdump_mkdirs(const char *path,char *error,size_t capacity);
int gfxdump_write_png(const char *path,int w,int h,const uint8_t *pixels,
                     int bpp8,const SDL_Color colors[256],char *error,size_t capacity);
int gfxdump_verify_png(const char *path,int w,int h,char *error,size_t capacity);
