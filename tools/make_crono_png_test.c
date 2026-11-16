/* SPDX-License-Identifier: GPL-3.0-or-later
 * Reproducible PNG loading test from the player's own USA/YQUE ROM.
 * Only the two battle-ready canvases are exported. Original pixels and alpha
 * are retained; the three tunic colours are replaced by diagnostic purple.
 * No illustrated art, geometry or animation changes. */
#include "gfx_rom.h"
#include "rt.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
RtRom rt_rom;
int main(int argc,char**argv){
 if(argc!=3){fprintf(stderr,"Usage: %s own-CT.nds output-test-graphics-folder\n",argv[0]);return 2;}
 FILE*f=fopen(argv[1],"rb");assert(f);fseek(f,0,SEEK_END);rt_rom.size=(size_t)ftell(f);rewind(f);rt_rom.data=malloc(rt_rom.size);assert(rt_rom.data&&fread(rt_rom.data,1,rt_rom.size,f)==rt_rom.size);fclose(f);assert(rt_rom.size>512&&!memcmp(rt_rom.data+12,"YQUE",4));assert(gfx_list_files());
 GfxBlob tb=gfx_open(gfx_find("Chara/Chara_0000.NCGR")),cb=gfx_open(gfx_find("Chara/Chara_0000.NCER"));GfxTiles tiles;GfxPalette pal;assert(gfx_read_tiles(&tb,&tiles)&&gfx_read_palette(gfx_find("Chara/ObjPlt_0000.NCLR"),&pal));
 char dir[1200],path[1300];snprintf(dir,sizeof dir,"%s/frames/Chara/Chara_0000",argv[2]);assert(SDL_CreateDirectory(dir));
 for(int cell=198;cell<=199;cell++){
  GfxLayout l;GfxObject obj[8];assert(gfx_cell_layout(&cb,(unsigned)cell,&l));int n=gfx_cell_objects(&cb,(unsigned)cell,obj,8),x0=999,y0=999;assert(n>0);for(int k=0;k<n;k++){assert(obj[k].w==16&&obj[k].h==16);if(obj[k].x<x0)x0=obj[k].x;if(obj[k].y<y0)y0=obj[k].y;}
  uint8_t*indices=calloc((size_t)l.w*l.h,1);uint32_t*px=calloc((size_t)l.w*l.h,4);assert(indices&&px);gfx_draw(&l,&tiles,indices);unsigned changed=0,opaque=0;
  for(int i=0;i<l.w*l.h;i++){unsigned index=indices[i];if(!(index&15))continue;SDL_Color c=pal.c[index];if((index&15)==8)c=(SDL_Color){240,120,255,255};if((index&15)==9)c=(SDL_Color){146,48,188,255};if((index&15)==10)c=(SDL_Color){57,20,77,255};changed+=(index&15)>=8&&(index&15)<=10;opaque++;px[i]=0xff000000u|c.r|((uint32_t)c.g<<8)|((uint32_t)c.b<<16);}
  assert(changed>20);SDL_Surface*s=SDL_CreateSurfaceFrom(l.w,l.h,SDL_PIXELFORMAT_ABGR8888,px,l.w*4);snprintf(path,sizeof path,"%s/%03d.png",dir,cell);assert(s&&SDL_SavePNG(s,path));SDL_DestroySurface(s);
  printf("cell=%d canvas=%dx%d origin=%d,%d opaque=%u changed_tunic_pixels=%u file=%s\n",cell,l.w,l.h,x0,y0,opaque,changed,path);free(px);free(indices);free(l.tiles);
 }
 gfx_close(&cb);gfx_close(&tb);free(rt_rom.data);return 0;
}
