/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "runtime/rt.h"
#include "runtime/gfx_mod.h"
#include "runtime/gfx_rom.h"
#include "runtime/redraw.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
RtRom rt_rom;static const char*base;static char roots[2][1200];
int egg_ps5_graphics_root_count(void){return 2;}const char*egg_ps5_graphics_root_at(int i){return roots[i];}const char*rt_data_dir(void){return base;}
void rt_log(const char*f,...){va_list a;va_start(a,f);vfprintf(stderr,f,a);fputc('\n',stderr);va_end(a);}
int main(int argc,char**argv){assert(argc==4&&SDL_Init(0));base=argv[2];int bad=!strcmp(argv[3],"bad");FILE*f=fopen(argv[1],"rb");assert(f);fseek(f,0,SEEK_END);rt_rom.size=ftell(f);rewind(f);rt_rom.data=malloc(rt_rom.size);assert(fread(rt_rom.data,1,rt_rom.size,f)==rt_rom.size);fclose(f);memcpy(rt_rom.code,"YQUE",5);unsigned char*before=malloc(rt_rom.size);assert(before);memcpy(before,rt_rom.data,rt_rom.size);gfx_list_files();GfxBlob tb=gfx_open(gfx_find("Chara/Chara_0000.NCGR")),cb=gfx_open(gfx_find("Chara/Chara_0000.NCER"));GfxTiles tiles;GfxLayout layout;assert(gfx_read_tiles(&tb,&tiles)&&gfx_cell_layout(&cb,0,&layout));unsigned char*mask=calloc((size_t)layout.w*layout.h,1);assert(mask);gfx_draw(&layout,&tiles,mask);
 for(int i=0;i<2;i++){char path[1400];snprintf(roots[i],sizeof roots[i],"%s/%c/",base,'a'+i);snprintf(path,sizeof path,"%sframes/Chara/Chara_0000",roots[i]);assert(SDL_CreateDirectory(path));snprintf(path,sizeof path,"%sframes/Chara/Chara_0000/000.png",roots[i]);if(i&&bad){f=fopen(path,"w");assert(f);fputs("invalid",f);fclose(f);continue;}SDL_Surface*s=SDL_CreateSurface(layout.w*3,layout.h*3,SDL_PIXELFORMAT_ABGR8888);assert(s);for(int y=0;y<s->h;y++)for(int x=0;x<s->w;x++)((unsigned*)((unsigned char*)s->pixels+y*s->pitch))[x]=mask[(y/3)*layout.w+x/3]?(i?0xff55aa22u:0xffaa7733u):0;assert(SDL_SavePNG(s,path));SDL_DestroySurface(s);}
 gfxmod_load();assert(ct_redraw_count()==1&&ct_redraw_pose_cell(0)==0&&!memcmp(before,rt_rom.data,rt_rom.size));uint16_t palette[16];for(int i=0;i<16;i++)palette[i]=(uint16_t)i;int found=0;for(int y=0;y<layout.h;y++)for(int x=0;x<layout.w;x++){uint32_t tex;if(!ct_redraw_sample(0,x,y,&tex))continue;unsigned c=ct_redraw_color(tex,palette);assert(c==(bad?0xffaa7733u:0xff55aa22u));found++;}assert(found>0);printf("PASS: character frame layering %s; one pose entry, last valid PNG wins, invalid later PNG retains prior art, unchanged ROM\n",argv[3]);return 0;}
