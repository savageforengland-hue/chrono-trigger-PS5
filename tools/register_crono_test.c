/* SPDX-License-Identifier: GPL-3.0-or-later
 * Register external authored PNGs on the player's original cell canvases.
 * This performs asset slicing/registration, not an art redraw. ROM untouched.
 * Jobs TSV: cell, input PNG, columns, rows, slot. No game/art data embedded. */
#define _POSIX_C_SOURCE 200809L
#include "gfx_rom.h"
#include "rt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
RtRom rt_rom;
static int save(const char*path,uint32_t*px,int w,int h){SDL_Surface*s=SDL_CreateSurfaceFrom(w,h,SDL_PIXELFORMAT_ABGR8888,px,w*4);int okay=s&&SDL_SavePNG(s,path);SDL_DestroySurface(s);return okay;}
static void register_frame(int id,const char*input,int cols,int rows,int slot,const char*out,const GfxBlob*cb,const GfxTiles*t){
 GfxLayout l;assert(gfx_cell_layout(cb,(unsigned)id,&l));uint8_t*original=calloc((size_t)l.w*l.h,1);assert(original);gfx_draw(&l,t,original);
 int nx0=l.w,ny0=l.h,nx1=0,ny1=0;
 for(int y=0;y<l.h;y++)for(int x=0;x<l.w;x++)if(original[y*l.w+x]&15){if(x<nx0)nx0=x;if(y<ny0)ny0=y;if(x+1>nx1)nx1=x+1;if(y+1>ny1)ny1=y+1;}
 assert(nx1>nx0&&ny1>ny0);free(original);free(l.tiles);
 SDL_Surface*raw=SDL_LoadPNG(input);assert(raw);SDL_Surface*s=SDL_ConvertSurface(raw,SDL_PIXELFORMAT_ABGR8888);SDL_DestroySurface(raw);assert(s);
 assert(cols>0&&rows>0&&slot>=0&&slot<cols*rows);
 int ox=(slot%cols)*s->w/cols,oy=(slot/cols)*s->h/rows,w=((slot%cols+1)*s->w/cols)-ox,h=((slot/cols+1)*s->h/rows)-oy;
 int*labels=calloc((size_t)w*h,sizeof(int));int*stack=malloc((size_t)w*h*sizeof(int));assert(labels&&stack);
 int best=0,best_area=0,idc=0,bx0=0,by0=0,bx1=0,by1=0;
 for(int at=0;at<w*h;at++){
  int x=at%w,y=at/w;uint32_t c=((uint32_t*)((uint8_t*)s->pixels+(oy+y)*s->pitch))[ox+x];if((c>>24)<128||labels[at])continue;
  int used=0,area=0,x0=x,y0=y,x1=x+1,y1=y+1;labels[at]=++idc;stack[used++]=at;
  while(used){int a=stack[--used],xx=a%w,yy=a/w;area++;if(xx<x0)x0=xx;if(yy<y0)y0=yy;if(xx+1>x1)x1=xx+1;if(yy+1>y1)y1=yy+1;
   for(int dy=-1;dy<=1;dy++)for(int dx=-1;dx<=1;dx++){int tx=xx+dx,ty=yy+dy;if(tx<0||ty<0||tx>=w||ty>=h)continue;int b=ty*w+tx;if(labels[b])continue;uint32_t word=((uint32_t*)((uint8_t*)s->pixels+(oy+ty)*s->pitch))[ox+tx];if((word>>24)<128)continue;labels[b]=idc;stack[used++]=b;}
  }
  if(area>best_area){best=idc;best_area=area;bx0=x0;by0=y0;bx1=x1;by1=y1;}
 }
 assert(best_area>200);free(stack);
 enum {SCALE=4};int tw=l.w*SCALE,th=l.h*SCALE;uint32_t*target=calloc((size_t)tw*th,4);assert(target);
 /* Uniform scale preserves anatomy. Original opaque height, centre and foot
  * baseline establish actor size/origin; transparent ROM padding is retained. */
 double factor=(double)((ny1-ny0)*SCALE)/(by1-by0),left=(nx0+nx1)*SCALE*.5-(bx1-bx0)*factor*.5,top=ny0*SCALE;
 assert(left>=-.5&&left+(bx1-bx0)*factor<=tw+.5);
 for(int y=0;y<th;y++)for(int x=0;x<tw;x++){
  unsigned sum[4]={0};
  for(int sy=0;sy<4;sy++)for(int sx=0;sx<4;sx++){
   double ax=bx0+((x+(sx+.5)/4)-left)/factor,ay=by0+((y+(sy+.5)/4)-top)/factor;int ix=(int)floor(ax),iy=(int)floor(ay);
   if(ix<bx0||iy<by0||ix>=bx1||iy>=by1||labels[iy*w+ix]!=best)continue;
   uint32_t c=((uint32_t*)((uint8_t*)s->pixels+(oy+iy)*s->pitch))[ox+ix];for(int k=0;k<4;k++)sum[k]+=(c>>(k*8))&255;
  }
  unsigned alpha=(sum[3]+8)/16;if(alpha>=128){uint32_t word=alpha<<24;for(int k=0;k<3;k++)word|=((sum[k]+8)/16)<<(k*8);target[y*tw+x]=word;}
 }
 char path[1024];snprintf(path,sizeof path,"%s/%03d.png",out,id);assert(save(path,target,tw,th));
 fprintf(stderr,"Registered %03d.png: %dx%d, uniform art scale %.4f, source opaque [%d,%d,%d,%d], original canvas/actor origin retained\n",id,tw,th,factor,nx0,ny0,nx1,ny1);
 free(target);free(labels);SDL_DestroySurface(s);
}
int main(int argc,char**argv){
 if(argc!=4){fprintf(stderr,"Usage: %s own-CT.nds private-jobs.tsv output-directory\n",argv[0]);return 2;}assert(SDL_Init(0));
 FILE*f=fopen(argv[1],"rb");assert(f);fseek(f,0,SEEK_END);rt_rom.size=(size_t)ftell(f);rewind(f);rt_rom.data=malloc(rt_rom.size);assert(rt_rom.data&&fread(rt_rom.data,1,rt_rom.size,f)==rt_rom.size);fclose(f);assert(!memcmp(rt_rom.data+12,"YQUE",4));
 gfx_list_files();GfxBlob tb=gfx_open(gfx_find("Chara/Chara_0000.NCGR")),cb=gfx_open(gfx_find("Chara/Chara_0000.NCER"));GfxTiles t;assert(gfx_read_tiles(&tb,&t));
 assert(SDL_CreateDirectory(argv[3]));f=fopen(argv[2],"r");assert(f);char line[2048],input[1024];int id,cols,rows,slot;
 while(fgets(line,sizeof line,f)){if(line[0]=='#')continue;assert(sscanf(line,"%d\t%1023s\t%d\t%d\t%d",&id,input,&cols,&rows,&slot)==5);register_frame(id,input,cols,rows,slot,argv[3],&cb,&t);}
 fclose(f);gfx_close(&tb);gfx_close(&cb);free(rt_rom.data);SDL_Quit();return 0;
}
