/* SPDX-License-Identifier: GPL-3.0-or-later
 * Real PNG decoding, world-map scoping, live palette pairs and native fallback. */
#include "runtime/rt.h"
#include "runtime/gfx_mod.h"
#include "runtime/gfx_rom.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
RtRom rt_rom;static const char*root;
const char*egg_ps5_graphics_root(void){return root;}
const char*rt_data_dir(void){return root;}
void rt_log(const char*f,...){va_list a;va_start(a,f);vfprintf(stderr,f,a);fputc('\n',stderr);va_end(a);}
void sprite_hd_load(void){}void ct_redraw_load(void){}int ct_redraw_dropin(const char*p,const char*r){(void)p;(void)r;return 0;}
int main(int argc,char**argv){assert(argc==4&&SDL_Init(0));root=argv[2];int empty=!strcmp(argv[3],"empty"),bad=!strcmp(argv[3],"bad")||!strcmp(argv[3],"zero"),rgba=!strcmp(argv[3],"rgba");FILE*f=fopen(argv[1],"rb");assert(f);fseek(f,0,SEEK_END);rt_rom.size=ftell(f);rewind(f);rt_rom.data=malloc(rt_rom.size);assert(fread(rt_rom.data,1,rt_rom.size,f)==rt_rom.size);fclose(f);unsigned char*before=malloc(rt_rom.size);assert(before);memcpy(before,rt_rom.data,rt_rom.size);gfx_list_files();GfxBlob blob=gfx_open(gfx_find("WorldMap/WorldBg_0000_ncg.bin"));GfxTiles t;GfxLayout l;assert(gfx_read_tiles(&blob,&t)&&gfx_sheet_layout(&t,&l));unsigned off=0;while(off<t.size){int any=0;for(int i=0;i<32;i++)any|=t.data[off+i];if(any)break;off+=32;}assert(off<t.size);
 char path[2048];snprintf(path,sizeof path,"%ssheets/WorldMap",root);assert(SDL_CreateDirectory(path));SDL_Surface*s=SDL_CreateSurface(l.w*4,l.h*4,rgba?SDL_PIXELFORMAT_ABGR8888:SDL_PIXELFORMAT_INDEX8);assert(s);if(!rgba){SDL_Palette*p=SDL_CreatePalette(256);SDL_Color c[256];for(int i=0;i<256;i++)c[i]=(SDL_Color){i,i,i,i?255:0};assert(SDL_SetPaletteColors(p,c,0,256)&&SDL_SetSurfacePalette(s,p));SDL_DestroyPalette(p);}SDL_FillSurfaceRect(s,NULL,0);if(!empty)SDL_FillSurfaceRect(s,&(SDL_Rect){(int)(off/32%16)*32,(int)(off/32/16)*32,32,32},rgba?0xff2678b1u:17);snprintf(path,sizeof path,"%ssheets/WorldMap/WorldBg_0000_ncg.bin.png",root);assert(SDL_SavePNG(s,path));SDL_DestroySurface(s);
 snprintf(path,sizeof path,"%sbackgrounds.json",root);f=fopen(path,"w");assert(f);fprintf(f,"{\"schema\":1,\"transparentTiles\":\"original\",\"includeCharacterSheets\":false,\"backgroundBlendWeight\":%d,\"locations\":[{\"id\":496}]}",bad?(!strcmp(argv[3],"zero")?0:128):96);fclose(f);gfxmod_load();assert(!memcmp(before,rt_rom.data,rt_rom.size));unsigned key=gfxmod_find(t.data+off,0);
 if(empty||bad){assert(!key);gfxmod_background_location(496);assert(!gfxmod_hd_active(0)||empty);}
 else{assert(key);const GfxHdTile*art=gfxmod_tile(key);assert(art->scope==2&&art->indexed==(rgba?4:2));assert(((const unsigned*)art->px)[0]==(rgba?0xff2678b1u:1u|2u<<8|96u<<16));for(int where=0;where<4;where++){int location=where==0?496:where==1?497:where==2?1:65535;gfxmod_background_location(location);assert(!!gfxmod_hd_active(0)==(where==0));assert(!!gfxmod_allowed(art,0,0,0)==(where==0));assert(!gfxmod_allowed(art,1,0,0));} }
 printf("PASS: world PNG %s; scoped BG-only live palette encoding, 1000AD vs other location fallback, unchanged ROM\n",argv[3]);return 0;}
