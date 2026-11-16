/* SPDX-License-Identifier: GPL-3.0-or-later
 * Real loader + matcher + HD texel audit for the two-frame or full visible-pose diagnostic test. */
#include "runtime/rt.h"
#include "runtime/gfx_rom.h"
#include "runtime/gfx_mod.h"
#include "runtime/redraw.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
RtRom rt_rom;static const char*data;static char graphics[1400];
const char*rt_data_dir(void){return data;}
const char*egg_ps5_graphics_root(void){return graphics;}
void rt_log(const char*fmt,...){va_list a;va_start(a,fmt);vfprintf(stderr,fmt,a);fputc('\n',stderr);va_end(a);}
static void fixture(int cell,RedrawObject live[128],const GfxBlob*cb,const GfxTiles*t,int*x0,int*y0){GfxObject obj[8];int n=gfx_cell_objects(cb,(unsigned)cell,obj,8);assert(n);memset(live,0,128*sizeof *live);*x0=*y0=999;int count=0;for(int k=0;k<n;k++){assert(obj[k].w==16&&obj[k].h==16&&obj[k].off+128<=t->size);if(obj[k].x<*x0)*x0=obj[k].x;if(obj[k].y<*y0)*y0=obj[k].y;int opaque=0;for(int b=0;b<128;b++)opaque|=t->data[obj[k].off+b];if(!opaque)continue;RedrawObject*a=live+5+count++*11;*a=(RedrawObject){.valid=1,.x=(136+obj[k].x)&511,.y=(152+obj[k].y)&255,.flip=obj[k].hflip|obj[k].vflip<<1,.palette=3,.priority=1};memcpy(a->bytes,t->data+obj[k].off,128);}}
static uint32_t rgb(SDL_Color c){return 0xff000000u|c.r|((uint32_t)c.g<<8)|((uint32_t)c.b<<16);}
static uint32_t expected(unsigned index,const GfxPalette*p){if(!(index&15))return 0;return (index&15)==8?0xffff78f0u:(index&15)==9?0xffbc3092u:(index&15)==10?0xff4d1439u:rgb(p->c[index]);}
int main(int argc,char**argv){
 assert(argc==5);data=argv[2];snprintf(graphics,sizeof graphics,"%smods/packs/crono-png-test/graphics/",data);int expect=atoi(argv[3]),variant=atoi(argv[4]);assert(expect>=0&&expect<=244);int all=variant>=100;assert(SDL_Init(0));
 FILE*f=fopen(argv[1],"rb");assert(f);fseek(f,0,SEEK_END);rt_rom.size=(size_t)ftell(f);rewind(f);rt_rom.data=malloc(rt_rom.size);assert(rt_rom.data&&fread(rt_rom.data,1,rt_rom.size,f)==rt_rom.size);fclose(f);memcpy(rt_rom.code,"YQUE",5);uint8_t*before=malloc(rt_rom.size);assert(before);memcpy(before,rt_rom.data,rt_rom.size);assert(gfx_list_files());
 GfxBlob tb=gfx_open(gfx_find("Chara/Chara_0000.NCGR")),cb=gfx_open(gfx_find("Chara/Chara_0000.NCER"));GfxTiles t;GfxPalette pal;assert(gfx_read_tiles(&tb,&t)&&gfx_read_palette(gfx_find("Chara/ObjPlt_0000.NCLR"),&pal));
 gfxmod_load();assert(ct_redraw_count()==expect&&ct_redraw_module_count(CT_REDRAW_DROPIN)==expect);uint16_t palette[256]={0};for(int i=0;i<16;i++)palette[48+i]=(pal.c[i].r>>3)|((pal.c[i].g>>3)<<5)|((pal.c[i].b>>3)<<10);
 int covered=0;unsigned checked=0,changed=0;
 for(unsigned cell=0;cell<gfx_cell_count(&cb);cell++){
  RedrawObject live[128],copy[128];int x0,y0;fixture((int)cell,live,&cb,&t,&x0,&y0);memcpy(copy,live,sizeof live);int groups=ct_redraw_prepare(0,live,palette),wanted=all?expect>0&&!(cell>=131&&cell<=134)&&!(variant==101&&cell==198)&&!(variant==102&&cell==199):(cell==198||cell==199)&&expect>0&&!(variant==1&&cell==198)&&!(variant==2&&cell==199);
  if(groups!=wanted){fprintf(stderr,"UNEXPECTED: stored cell=%u groups=%d wanted=%d\n",cell,groups,wanted);assert(0);}assert(!memcmp(copy,live,sizeof live));if(!groups)continue;covered++;
  const RedrawBinding*b=ct_redraw_binding(0,5);assert(b&&b->action==2);int source=ct_redraw_pose_cell(b->pose);
  if(all&&source!=(int)cell){GfxLayout target,ref;assert(gfx_cell_layout(&cb,cell,&target)&&gfx_cell_layout(&cb,(unsigned)source,&ref));uint8_t*pt=calloc((size_t)target.w*target.h,1),*pr=calloc((size_t)ref.w*ref.h,1);assert(pt&&pr);gfx_draw(&target,&t,pt);gfx_draw(&ref,&t,pr);RedrawObject rr[128];int rx,ry;fixture(source,rr,&cb,&t,&rx,&ry);rx=((b->x-136+256)&511)-256;ry=((b->y-152+128)&255)-128;
   for(int y=-96;y<96;y++)for(int x=-96;x<96;x++){int tx=x-x0,ty=y-y0,sx=x-rx,sy=y-ry;int a=tx>=0&&ty>=0&&tx<target.w&&ty<target.h?pt[ty*target.w+tx]:0,z=sx>=0&&sy>=0&&sx<ref.w&&sy<ref.h?pr[sy*ref.w+sx]:0;if(a!=z){fprintf(stderr,"FALSE MATCH target=%u source=%d actor_pixel=%d,%d target_index=%d source_index=%d\n",cell,source,x,y,a,z);assert(0);}}free(pt);free(pr);free(target.tiles);free(ref.tiles);}
  if(!all)assert(source==(int)cell);if(source==(int)cell)assert(b->x==((136+x0)&511)&&b->y==((152+y0)&255));
  GfxLayout l;assert(gfx_cell_layout(&cb,(unsigned)source,&l));assert(b->w==l.w&&b->h==l.h);uint8_t*indices=calloc((size_t)l.w*l.h,1);assert(indices);gfx_draw(&l,&t,indices);
  for(int y=0;y<l.h;y++)for(int x=0;x<l.w;x++){
   uint32_t texel,id=ct_redraw_sample(b->pose,x,y,&texel),want=expected(indices[y*l.w+x],&pal);if(!want){assert(!id);continue;}assert(id&&ct_redraw_color(texel,palette+48)==want);const GfxHdTile*hd=gfxmod_tile(id);assert(hd&&hd->indexed==3&&hd->scale==2);int dim=8*hd->scale;for(int dy=0;dy<2;dy++)for(int dx=0;dx<2;dx++){uint32_t c=((const uint32_t*)hd->px)[((y%8)*2+dy)*dim+(x%8)*2+dx];assert(ct_redraw_color(c,palette+48)==want);checked++;}if(want!=rgb(pal.c[indices[y*l.w+x]]))changed++;
  }
  ct_redraw_set_visible(0);assert(!ct_redraw_prepare(0,live,palette));ct_redraw_set_visible(1);assert(ct_redraw_prepare(0,live,palette)==1);free(indices);free(l.tiles);
 }
 unsigned other_cells=0;
 if(all)for(int ch=1;ch<7;ch++){char name[96];snprintf(name,sizeof name,"Chara/Chara_%04d.NCGR",ch);GfxBlob otb=gfx_open(gfx_find(name));snprintf(name,sizeof name,"Chara/Chara_%04d.NCER",ch);GfxBlob ocb=gfx_open(gfx_find(name));GfxTiles ot;GfxPalette op;snprintf(name,sizeof name,"Chara/ObjPlt_%04d.NCLR",ch);assert(gfx_read_tiles(&otb,&ot)&&gfx_read_palette(gfx_find(name),&op));uint16_t other_palette[256]={0};for(int i=0;i<16;i++)other_palette[48+i]=(op.c[i].r>>3)|((op.c[i].g>>3)<<5)|((op.c[i].b>>3)<<10);
  for(unsigned cell=0;cell<gfx_cell_count(&ocb);cell++){GfxObject object[8];int n=gfx_cell_objects(&ocb,cell,object,8),ok=n>0;for(int i=0;i<n;i++)ok&=object[i].w==16&&object[i].h==16&&object[i].off+128<=ot.size;if(!ok)continue;RedrawObject live[128];int x0,y0;fixture((int)cell,live,&ocb,&ot,&x0,&y0);assert(!ct_redraw_prepare(0,live,other_palette));other_cells++;}gfx_close(&otb);gfx_close(&ocb);
 }
 fprintf(stderr,"OTHER_CHARACTERS cells_checked=%u no_false_replacements=1\n",other_cells);
 assert(covered==expect);for(unsigned off=0;off+32<=t.size;off+=32)assert(!gfxmod_find(t.data+off,0));assert(!memcmp(before,rt_rom.data,rt_rom.size));
 printf("PASS: loaded=%d intended_cells=%d all_248_cells_audited=1 native_and_hd_texels_checked=%u purple_tunic_pixels=%u alpha_canvas_origin_preserved=1 shared_tile_spill=0 original_rom_unchanged=1\n",expect,covered,checked,changed);gfx_close(&cb);gfx_close(&tb);free(before);free(rt_rom.data);SDL_Quit();return 0;
}
