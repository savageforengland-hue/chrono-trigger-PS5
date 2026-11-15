/* SPDX-License-Identifier: GPL-3.0-or-later
 * Own-ROM/art integration: exact selected-cell identity, canvas/origin,
 * unlisted/missing fallback, live switches, palette separation and immutability. */
#include "runtime/rt.h"
#include "runtime/redraw.h"
#include "runtime/redraw_manifest.h"
#include "runtime/gfx_rom.h"
#include "runtime/gfx_mod.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
RtRom rt_rom;static const char*root;
const char*rt_data_dir(void){return root;}
void rt_log(const char*fmt,...){va_list a;va_start(a,fmt);vfprintf(stderr,fmt,a);fputc('\n',stderr);va_end(a);}
static void fixture(int cell,RedrawObject live[128],const GfxBlob*cb,const GfxTiles*t,int*x0,int*y0){
 GfxObject obj[8];int n=gfx_cell_objects(cb,(unsigned)cell,obj,8);assert(n);memset(live,0,128*sizeof *live);*x0=*y0=999;int count=0;
 for(int k=0;k<n;k++){
  assert(obj[k].w==16&&obj[k].h==16&&obj[k].off+128<=t->size);if(obj[k].x<*x0)*x0=obj[k].x;if(obj[k].y<*y0)*y0=obj[k].y;
  int opaque=0;for(int b=0;b<128;b++)opaque|=t->data[obj[k].off+b];if(!opaque)continue;
  RedrawObject*a=live+5+count++*11;*a=(RedrawObject){.valid=1,.x=(136+obj[k].x)&511,.y=(152+obj[k].y)&255,.flip=obj[k].hflip|obj[k].vflip<<1,.palette=3,.priority=1};memcpy(a->bytes,t->data+obj[k].off,128);
 }
}
int main(int argc,char**argv){
 assert(argc==3||argc==4);root=argv[2];int missing=argc==4&&!strcmp(argv[3],"missing"),invalid=argc==4&&!strcmp(argv[3],"invalid");assert(SDL_Init(0));
 FILE*f=fopen(argv[1],"rb");assert(f);fseek(f,0,SEEK_END);rt_rom.size=(size_t)ftell(f);rewind(f);rt_rom.data=malloc(rt_rom.size);assert(rt_rom.data&&fread(rt_rom.data,1,rt_rom.size,f)==rt_rom.size);fclose(f);memcpy(rt_rom.code,"YQUE",5);
 uint8_t*before=malloc(rt_rom.size);assert(before);memcpy(before,rt_rom.data,rt_rom.size);
 ct_redraw_load();int expected=invalid?0:missing?7:8;assert(ct_redraw_module_count(CT_REDRAW_CRONO)==expected);assert(ct_redraw_module_count(CT_REDRAW_MARLE)==4);assert(ct_redraw_count()==expected+4);
 GfxBlob tb=gfx_open(gfx_find("Chara/Chara_0000.NCGR")),cb=gfx_open(gfx_find("Chara/Chara_0000.NCER"));GfxTiles tiles;GfxPalette pal;assert(gfx_read_tiles(&tb,&tiles)&&gfx_read_palette(gfx_find("Chara/ObjPlt_0000.NCLR"),&pal));
 uint16_t palette[256]={0};for(int i=0;i<16;i++)palette[48+i]=(pal.c[i].r>>3)|((pal.c[i].g>>3)<<5)|((pal.c[i].b>>3)<<10);
 const int ids[]={0,1,2,3,5,6,11,56};
 for(unsigned k=0;k<sizeof ids/sizeof *ids;k++){
  RedrawObject live[128],copy[128];int x0,y0;fixture(ids[k],live,&cb,&tiles,&x0,&y0);memcpy(copy,live,sizeof live);
  int groups=ct_redraw_prepare(0,live,palette);
  if(invalid||(missing&&ids[k]==0)){assert(!groups&&!ct_redraw_binding(0,5));continue;}
  assert(groups==1&&!memcmp(copy,live,sizeof live));const RedrawBinding*b=ct_redraw_binding(0,5);assert(b&&b->action==2&&ct_redraw_pose_cell(b->pose)==ids[k]);
  GfxLayout layout;assert(gfx_cell_layout(&cb,(unsigned)ids[k],&layout));assert(b->w==layout.w&&b->h==layout.h&&b->x==((136+x0)&511)&&b->y==((152+y0)&255));free(layout.tiles);
  int pixels=0;for(int y=0;y<b->h;y++)for(int x=0;x<b->w;x++){uint32_t texel,tile=ct_redraw_sample(b->pose,x,y,&texel);if(!tile)continue;pixels++;const GfxHdTile*hd=gfxmod_tile(tile);assert(hd&&hd->scale==4&&hd->indexed==3);}
  assert(pixels>20);ct_redraw_module_set_enabled(CT_REDRAW_CRONO,0);assert(!ct_redraw_prepare(0,live,palette)&&ct_redraw_module_enabled(CT_REDRAW_MARLE));ct_redraw_module_set_enabled(CT_REDRAW_CRONO,1);
  ct_redraw_set_visible(0);assert(!ct_redraw_prepare(0,live,palette));ct_redraw_set_visible(1);
  live[5].bytes[0]^=1;assert(!ct_redraw_prepare(0,live,palette));memcpy(live,copy,sizeof live);
  uint16_t foreign[256];memcpy(foreign,palette,sizeof palette);for(int i=2;i<=11;i++)foreign[48+i]^=0x111;assert(!ct_redraw_prepare(0,live,foreign));
  uint16_t changed[256];memcpy(changed,palette,sizeof palette);changed[49]^=0x421;assert(ct_redraw_prepare(0,live,changed)==1);assert(ct_redraw_color(0xfd010203,changed+48)!=0xff010203);assert(ct_redraw_color(0xff227799,changed+48)==0xff227799);
  for(int i=0;i<16;i++)changed[48+i]=0x7fff;assert(ct_redraw_prepare(0,live,changed)==1&&ct_redraw_color(0xff227799,changed+48)==0xffffffffu);
 }
 /* Audit every stored Crono cell. A reused whole-object match must decode
  * to the same original pixels at the same actor coordinates, never a subset
  * of a different pose. This detects accidental frame/tile contamination. */
 int covered=0;
 for(unsigned cell=0;cell<gfx_cell_count(&cb);cell++){
  RedrawObject live[128];int ax,ay;fixture((int)cell,live,&cb,&tiles,&ax,&ay);int groups=ct_redraw_prepare(0,live,palette);if(!groups)continue;
  covered++;const RedrawBinding*b=ct_redraw_binding(0,5);assert(b&&b->action==2);int source=ct_redraw_pose_cell(b->pose),rx,ry;RedrawObject reference[128];fixture(source,reference,&cb,&tiles,&rx,&ry);
  GfxLayout a,z;assert(gfx_cell_layout(&cb,cell,&a)&&gfx_cell_layout(&cb,(unsigned)source,&z));uint8_t*pa=calloc((size_t)a.w*a.h,1),*pz=calloc((size_t)z.w*z.h,1);assert(pa&&pz);gfx_draw(&a,&tiles,pa);gfx_draw(&z,&tiles,pz);
  for(int y=-96;y<96;y++)for(int x=-96;x<96;x++){
   int xx=x-ax,yy=y-ay,zx=x-rx,zy=y-ry;
   int va=xx>=0&&yy>=0&&xx<a.w&&yy<a.h?pa[yy*a.w+xx]:0,vz=zx>=0&&zy>=0&&zx<z.w&&zy<z.h?pz[zy*z.w+zx]:0;
   if(va!=vz){fprintf(stderr,"ERROR cell %u falsely matched source %d at actor %d,%d: %d != %d\n",cell,source,x,y,va,vz);assert(0);}
  }
  fprintf(stderr,"COVERAGE stored_cell=%u registered_cell=%d original_pixels_identical=1\n",cell,source);free(pa);free(pz);free(a.tiles);free(z.tiles);
 }
 fprintf(stderr,"COVERAGE total=%d of %u stored cells (includes exact reused poses)\n",covered,gfx_cell_count(&cb));
 /* Unlisted stored cell uses the original rendering. */
 RedrawObject ordinary[128];int x0,y0;fixture(80,ordinary,&cb,&tiles,&x0,&y0);assert(!ct_redraw_prepare(0,ordinary,palette));
 for(unsigned off=0;off+32<=tiles.size;off+=32)assert(!gfxmod_find(tiles.data+off,0));
 assert(!ct_redraw_prepare(-1,ordinary,palette)&&!ct_redraw_prepare(2,ordinary,palette));
 assert(!memcmp(before,rt_rom.data,rt_rom.size));assert(ct_redraw_viewer_open());for(int i=0;i<ct_redraw_count();i++)ct_redraw_viewer_step(1);ct_redraw_viewer_close();
 gfx_close(&tb);gfx_close(&cb);free(before);free(rt_rom.data);SDL_Quit();puts("PASS: Crono JSON-selected real poses, exact identity/origin/canvas, HD tiles, switches and Marle coexistence, missing/invalid/unlisted fallback, palette effects and immutable ROM");return 0;
}
