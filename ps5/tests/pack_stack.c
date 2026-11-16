/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "runtime/rt.h"
#include "runtime/gfx_mod.h"
#include "runtime/gfx_rom.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
RtRom rt_rom;static char roots[4][1200];static const char*base;static int nroots=3,sprite_loads,redraw_loads;
int egg_ps5_graphics_root_count(void){return nroots;}const char*egg_ps5_graphics_root_at(int i){return roots[i];}
const char*rt_data_dir(void){return base;}
void rt_log(const char*f,...){va_list a;va_start(a,f);vfprintf(stderr,f,a);fputc('\n',stderr);va_end(a);}
void sprite_hd_load(void){sprite_loads++;}void ct_redraw_load(void){redraw_loads++;}int ct_redraw_dropin(const char*p,const char*r){(void)p;(void)r;return 0;}
int main(int argc,char**argv){assert(argc==4&&SDL_Init(0));base=argv[2];const char*variant=argv[3];int empty=!strcmp(variant,"empty"),bad=!strcmp(variant,"bad"),global=!strcmp(variant,"global");if(global)nroots=4;if(!strcmp(variant,"none"))nroots=0;
 FILE*f=fopen(argv[1],"rb");assert(f);fseek(f,0,SEEK_END);rt_rom.size=ftell(f);rewind(f);rt_rom.data=malloc(rt_rom.size);assert(fread(rt_rom.data,1,rt_rom.size,f)==rt_rom.size);fclose(f);unsigned char*before=malloc(rt_rom.size);assert(before);memcpy(before,rt_rom.data,rt_rom.size);gfx_list_files();GfxBlob b=gfx_open(gfx_find("WorldMap/WorldBg_0000_ncg.bin"));GfxTiles t;GfxLayout l;assert(gfx_read_tiles(&b,&t)&&gfx_sheet_layout(&t,&l));unsigned off=0;for(;off<t.size;off+=32){int any=0;for(int i=0;i<32;i++)any|=t.data[off+i];if(any)break;}assert(off<t.size);
 unsigned colors[]={0xff225588u,0xffaa7733u,0xff55aa22u,0xffbb44ddu};
 for(int layer=0;layer<nroots;layer++){char p[1400];snprintf(roots[layer],sizeof roots[layer],"%s/%c/",base,'a'+layer);snprintf(p,sizeof p,"%ssheets/WorldMap",roots[layer]);assert(SDL_CreateDirectory(p));SDL_Surface*s=SDL_CreateSurface(l.w*4,l.h*4,SDL_PIXELFORMAT_ABGR8888);assert(s);SDL_FillSurfaceRect(s,NULL,0);if(!(layer==2&&empty))SDL_FillSurfaceRect(s,&(SDL_Rect){(int)(off/32%16)*32,(int)(off/32/16)*32,32,32},colors[layer]);snprintf(p,sizeof p,"%ssheets/WorldMap/WorldBg_0000_ncg.bin.png",roots[layer]);assert(SDL_SavePNG(s,p));SDL_DestroySurface(s);
  if(layer<3){snprintf(p,sizeof p,"%sbackgrounds.json",roots[layer]);f=fopen(p,"w");assert(f);if(layer==2&&bad)fputs("bad JSON",f);else fprintf(f,"{\"schema\":1,\"transparentTiles\":\"original\",\"includeCharacterSheets\":false,%s\"locations\":[{\"id\":%d,\"paletteLight\":[%d,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]}]}",layer==0?"\"windowLight\":{\"location\":2,\"bg\":3,\"mapCell\":[15,5],\"bgControl\":17418,\"openTile\":299,\"closedTile\":302,\"closedBrightness\":65,\"fadeFrames\":18},":"",layer==1?496:2,layer==0?7440:layer==2?3720:0);fclose(f);}
 }
 gfxmod_load();assert(sprite_loads==1&&redraw_loads==1&&!memcmp(before,rt_rom.data,rt_rom.size));unsigned head=gfxmod_find(t.data+off,0);if(!nroots){assert(!head&&!gfxmod_hd_scale());puts("PASS: no enabled roots uses original assets; optional modules initialized once");return 0;}assert(head);
 for(int where=0;where<3;where++){int location=where==0?2:where==1?496:1;gfxmod_background_location(location);const GfxHdTile*art=NULL;for(unsigned k=head;k;k=gfxmod_fallback(k))if(gfxmod_allowed(gfxmod_tile(k),0,0,0)){art=gfxmod_tile(k);break;}
  if(global){assert(art&&art->scope==0&&((const unsigned*)art->px)[0]==colors[3]);continue;}
  if(where==2){assert(!art&&!gfxmod_hd_active(0));continue;}assert(art&&art->scope==2&&art->policy>0);unsigned expected=where==1?colors[1]:(empty||bad)?colors[0]:colors[2];assert(((const unsigned*)art->px)[0]==expected&&!gfxmod_allowed(art,1,0,0));
 }
 gfxmod_background_location(2);int bg,x,y,control;assert(gfxmod_window_probe(&bg,&x,&y,&control)&&control==17418);gfxmod_window_state(1,299,100);for(int i=101;i<=118;i++)gfxmod_window_state(1,302,(uint64_t)i);assert(gfxmod_window_gain()==166);gfxmod_background_location(496);assert(!gfxmod_window_probe(&bg,&x,&y,&control)&&gfxmod_window_gain()==256);
 uint16_t palette[1024]={0},gains[(GFXMOD_POLICY_LIMIT+1)*32];for(int i=1;i<16;i++)palette[i]=0x7fff;gfxmod_background_location(2);gfxmod_background_gains_all(palette,gains);assert(gains[32]==256&&gains[64]==256);if(!bad&&!empty)assert(gains[96]==512);
 printf("PASS: actual multi-root PNG loader %s; per-pack scopes/priority/window/gains, invalid/empty higher-layer fallback and unchanged ROM\n",variant);return 0;}
