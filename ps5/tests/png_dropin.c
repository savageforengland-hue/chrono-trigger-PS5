/* SPDX-License-Identifier: GPL-3.0-or-later
 * Creates local test PNGs from a player's ROM, never embedded game data. */
#include "runtime/rt.h"
#include "runtime/gfx_rom.h"
#include "runtime/gfx_mod.h"
#include "runtime/redraw.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
RtRom rt_rom;static const char*root;static char graphics[1200];
const char*rt_data_dir(void){return root;}
const char*egg_ps5_graphics_root(void){return graphics;}
void rt_log(const char*fmt,...){va_list a;va_start(a,fmt);vfprintf(stderr,fmt,a);fputc('\n',stderr);va_end(a);}
static int fixture(int cell,RedrawObject live[128],const GfxBlob*cb,const GfxTiles*t,int*x0,int*y0){GfxObject obj[8];int n=gfx_cell_objects(cb,(unsigned)cell,obj,8);assert(n);memset(live,0,128*sizeof *live);*x0=*y0=999;int count=0;for(int k=0;k<n;k++){assert(obj[k].w==16&&obj[k].h==16&&obj[k].off+128<=t->size);if(obj[k].x<*x0)*x0=obj[k].x;if(obj[k].y<*y0)*y0=obj[k].y;int opaque=0;for(int b=0;b<128;b++)opaque|=t->data[obj[k].off+b];if(!opaque)continue;RedrawObject*a=live+5+count++*11;*a=(RedrawObject){.valid=1,.x=(136+obj[k].x)&511,.y=(152+obj[k].y)&255,.flip=obj[k].hflip|obj[k].vflip<<1,.palette=3,.priority=1};memcpy(a->bytes,t->data+obj[k].off,128);}return count;}
int main(int argc,char**argv){
 assert(argc==4);assert(SDL_Init(0));root=argv[2];int indexed=!strcmp(argv[3],"indexed"),bad=!strcmp(argv[3],"bad"),empty=!strcmp(argv[3],"empty");snprintf(graphics,sizeof graphics,"%smods/graphics/",root);assert(SDL_CreateDirectory(graphics));
 FILE*f=fopen(argv[1],"rb");assert(f);fseek(f,0,SEEK_END);rt_rom.size=(size_t)ftell(f);rewind(f);rt_rom.data=malloc(rt_rom.size);assert(rt_rom.data&&fread(rt_rom.data,1,rt_rom.size,f)==rt_rom.size);fclose(f);memcpy(rt_rom.code,"YQUE",5);uint8_t*before=malloc(rt_rom.size);assert(before);memcpy(before,rt_rom.data,rt_rom.size);
 assert(gfx_list_files());GfxBlob tb=gfx_open(gfx_find("Chara/Chara_0000.NCGR")),cb=gfx_open(gfx_find("Chara/Chara_0000.NCER"));GfxTiles t;GfxPalette pal;GfxLayout l;assert(gfx_read_tiles(&tb,&t)&&gfx_read_palette(gfx_find("Chara/ObjPlt_0000.NCLR"),&pal)&&gfx_cell_layout(&cb,0,&l));
 char dir[1300],path[1400];snprintf(dir,sizeof dir,"%sframes/Chara/Chara_0000",graphics);assert(SDL_CreateDirectory(dir));snprintf(path,sizeof path,"%s/000.png",dir);
 uint8_t*original=calloc((size_t)l.w*l.h,1);assert(original);gfx_draw(&l,&t,original);
 if(bad){f=fopen(path,"w");assert(f);fputs("invalid PNG",f);fclose(f);}else if(!empty){
  int scale=indexed?1:3;SDL_Surface*s=SDL_CreateSurface(l.w*scale,l.h*scale,indexed?SDL_PIXELFORMAT_INDEX8:SDL_PIXELFORMAT_ABGR8888);assert(s);
  if(indexed){SDL_Palette*p=SDL_CreateSurfacePalette(s);assert(p);SDL_Color colors[256];memcpy(colors,pal.c,sizeof colors);for(int i=0;i<256;i++)colors[i].a=(i&15)?255:0;assert(SDL_SetPaletteColors(p,colors,0,256));}
  for(int y=0;y<s->h;y++)for(int x=0;x<s->w;x++){unsigned value=original[(y/scale)*l.w+x/scale];if(indexed)((uint8_t*)s->pixels)[y*s->pitch+x]=(uint8_t)value;else ((uint32_t*)((uint8_t*)s->pixels+y*s->pitch))[x]=value?0xff987654u:0;}
  assert(SDL_SavePNG(s,path));SDL_DestroySurface(s);
 }
 free(l.tiles);gfxmod_load();int expected=!(bad||empty);assert(ct_redraw_module_count(CT_REDRAW_DROPIN)==expected&&ct_redraw_count()==expected);uint16_t palette[256]={0};for(int i=0;i<16;i++)palette[48+i]=(pal.c[i].r>>3)|((pal.c[i].g>>3)<<5)|((pal.c[i].b>>3)<<10);
 int coverage=0;for(unsigned cell=0;cell<gfx_cell_count(&cb);cell++){
  RedrawObject live[128];int x0,y0;fixture((int)cell,live,&cb,&t,&x0,&y0);int groups=ct_redraw_prepare(0,live,palette);if(!groups){if(cell==0)assert(!expected);continue;}assert(expected&&groups==1);coverage++;const RedrawBinding*b=ct_redraw_binding(0,5);assert(b&&ct_redraw_pose_cell(b->pose)==0);GfxLayout current;assert(gfx_cell_layout(&cb,cell,&current));uint8_t*px=calloc((size_t)current.w*current.h,1);assert(px);gfx_draw(&current,&t,px);
  int rx,ry;RedrawObject reference[128];fixture(0,reference,&cb,&t,&rx,&ry);GfxLayout ref;assert(gfx_cell_layout(&cb,0,&ref));
  for(int y=-96;y<96;y++)for(int x=-96;x<96;x++){int ax=x-x0,ay=y-y0,bx=x-rx,by=y-ry;assert((ax>=0&&ay>=0&&ax<current.w&&ay<current.h?px[ay*current.w+ax]:0)==(bx>=0&&by>=0&&bx<ref.w&&by<ref.h?original[by*ref.w+bx]:0));}
  int pixels=0;for(int y=0;y<b->h;y++)for(int x=0;x<b->w;x++){uint32_t texel,id=ct_redraw_sample(b->pose,x,y,&texel);if(!id)continue;const GfxHdTile*hd=gfxmod_tile(id);assert(hd&&hd->scale==(indexed?2:3)&&hd->indexed==3);if(indexed)assert((texel>>24)==246);else assert(ct_redraw_color(texel,palette+48)==0xff987654u);pixels++;}assert(pixels>20);free(current.tiles);free(ref.tiles);free(px);
 }
 assert(coverage==expected);for(unsigned off=0;off+32<=t.size;off+=32)assert(!gfxmod_find(t.data+off,0));assert(!memcmp(before,rt_rom.data,rt_rom.size));
 fprintf(stderr,"DROPIN variant=%s coverage=%d/%u no_shared_tile_changes=1 immutable_rom=1\n",argv[3],coverage,gfx_cell_count(&cb));free(original);gfx_close(&tb);gfx_close(&cb);free(before);free(rt_rom.data);SDL_Quit();return 0;
}
