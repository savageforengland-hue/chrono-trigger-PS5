/* SPDX-License-Identifier: GPL-3.0-or-later
 * Register authored art to the player's original cell canvases. No ROM edits.
 * Input art remains external; the compiler performs layout/format conversion.
 * See docs/MARLE-PILOT.md. */
#include "gfx_rom.h"
#include "rt.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
RtRom rt_rom;
typedef struct Component {int x0,y0,x1,y1,n,id;} Component;
typedef struct Plane {int w,h;uint32_t*p;int*label;Component c[64];int count;} Plane;
static int compare(const void*a,const void*b){return ((const Component*)b)->n-((const Component*)a)->n;}
static void components(Plane*p){
    p->label=calloc((size_t)p->w*p->h,sizeof(int));int*stack=malloc((size_t)p->w*p->h*sizeof(int));assert(p->label&&stack);
    int id=0;
    for(int start=0;start<p->w*p->h;start++)if((p->p[start]>>24)>=128&&!p->label[start]){
        Component c={p->w,p->h,0,0,0,++id};int used=0;stack[used++]=start;p->label[start]=id;
        while(used){int k=stack[--used],x=k%p->w,y=k/p->w;c.n++;
            if(x<c.x0)c.x0=x;if(y<c.y0)c.y0=y;if(x+1>c.x1)c.x1=x+1;if(y+1>c.y1)c.y1=y+1;
            for(int dy=-1;dy<=1;dy++)for(int dx=-1;dx<=1;dx++){
                int xx=x+dx,yy=y+dy;if(xx<0||yy<0||xx>=p->w||yy>=p->h)continue;int n=yy*p->w+xx;
                if((p->p[n]>>24)>=128&&!p->label[n]){p->label[n]=id;stack[used++]=n;}
            }
        }
        if(p->count<64)p->c[p->count++]=c;
    }free(stack);qsort(p->c,p->count,sizeof(Component),compare);
}
static Plane read_art(const char*path,int column,int columns){
    SDL_Surface*a=SDL_LoadPNG(path);assert(a);SDL_Surface*b=SDL_ConvertSurface(a,SDL_PIXELFORMAT_ABGR8888);SDL_DestroySurface(a);assert(b);
    assert(b->w%columns==0);Plane p={.w=b->w/columns,.h=b->h};p.p=malloc((size_t)p.w*p.h*4);assert(p.p);
    for(int y=0;y<p.h;y++)memcpy(p.p+y*p.w,(uint8_t*)b->pixels+y*b->pitch+column*p.w*4,(size_t)p.w*4);
    SDL_DestroySurface(b);components(&p);assert(p.count);return p;
}
static int component_y(const void*a,const void*b){const Component*x=a,*y=b;return x->y0!=y->y0?x->y0-y->y0:x->x0-y->x0;}
static void pose(int cell,Plane*art,const GfxBlob*cb,const GfxTiles*t,const GfxPalette*pal,const char*out){
    GfxLayout l;assert(gfx_cell_layout(cb,(unsigned)cell,&l));Plane native={.w=l.w,.h=l.h};
    uint8_t*px=calloc((size_t)l.w*l.h,1);native.p=calloc((size_t)l.w*l.h,4);assert(px&&native.p);gfx_draw(&l,t,px);free(l.tiles);
    for(int i=0;i<l.w*l.h;i++)if(px[i])native.p[i]=0xff000000u;free(px);components(&native);
    assert(native.count&&art->count);Component a=art->c[0],n=native.c[0];
    const int scale=4;int w=l.w*scale,h=l.h*scale;uint32_t*p=calloc((size_t)w*h,4);assert(p);
    double ax=(a.x0+a.x1)/2.0,ay=a.y1-1,tx=(n.x0+n.x1)*scale/2.0,ty=(n.y1-1)*scale;
    double k=(double)(n.y1-n.y0)*scale/(a.y1-a.y0),kx=k,top=k;
    if(cell==89){ax=450;ay=279;tx=13*scale;ty=17*scale;k=(double)(38-17)*scale/(a.y1-1-ay);kx=(double)(13-n.x0)*scale/(ax-a.x0);top=(double)(17-n.y0)*scale/(ay-a.y0);}
    if(cell==90){ax=1048-724;ay=366;tx=16*scale;ty=26*scale;k=(double)(47-26)*scale/(a.y1-1-ay);kx=(double)(16-n.x0)*scale/(ax-a.x0);top=(double)(26-n.y0)*scale/(ay-a.y0);}
    if(cell==91){ax=1595-1448;ay=319;tx=4*scale;ty=18*scale;k=(double)(39-18)*scale/(a.y1-1-ay);kx=(double)(4-n.x0)*scale/(ax-a.x0);top=k;}
    if(cell==211){double kw=(double)(n.x1-n.x0)*scale/(a.x1-a.x0);if(kw<k)k=kw;kx=top=k;}
    assert(k>0&&kx>0&&top>0);
    for(int y=0;y<h;y++)for(int x=0;x<w;x++){
        int sx=(int)(ax+(x-tx)/kx),sy=(int)(ay+(y-ty)/(y<ty?top:k));
        if(sx<0||sy<0||sx>=art->w||sy>=art->h||art->label[sy*art->w+sx]!=a.id)continue;
        uint32_t c=art->p[sy*art->w+sx];if((c>>24)>=128)p[y*w+x]=c|0xff000000u;
    }
    Component marks[8],drawn[8];int mn=0,dn=0;
    for(int i=1;i<native.count;i++)if(native.c[i].n<=8&&mn<8)marks[mn++]=native.c[i];
    for(int i=1;i<art->count;i++)if(art->c[i].n>=80&&dn<8)drawn[dn++]=art->c[i];
    qsort(marks,mn,sizeof(Component),component_y);qsort(drawn,dn,sizeof(Component),component_y);assert(mn==dn);
    for(int i=0;i<mn;i++){
        int x0=marks[i].x0*scale,y0=marks[i].y0*scale,mw=(marks[i].x1-marks[i].x0)*scale,mh=(marks[i].y1-marks[i].y0)*scale;
        for(int y=y0-1;y<=y0+mh;y++)for(int x=x0-1;x<=x0+mw;x++)if(x>=0&&y>=0&&x<w&&y<h)p[y*w+x]=0;
        for(int y=0;y<mh;y++)for(int x=0;x<mw;x++){
            int sx=drawn[i].x0+x*(drawn[i].x1-drawn[i].x0)/mw,sy=drawn[i].y0+y*(drawn[i].y1-drawn[i].y0)/mh;
            uint32_t c=art->p[sy*art->w+sx];if(art->label[sy*art->w+sx]==drawn[i].id&&(c>>24)>=128)p[(y0+y)*w+x0+x]=c|0xff000000u;
        }
    }
    char path[1024];snprintf(path,sizeof path,"%s/marle-%03d.png",out,cell);
    SDL_Surface*s=SDL_CreateSurfaceFrom(w,h,SDL_PIXELFORMAT_ABGR8888,p,w*4);assert(s&&SDL_SavePNG(s,path));SDL_DestroySurface(s);
    fprintf(stderr,"Registered cell %d: %dx%d, %d source/drawn detached marks, eye/foot anchor or whole-pose fit\n",cell,w,h,mn);
    free(p);free(native.p);free(native.label);
}
int main(int argc,char**argv){
    if(argc!=5){fprintf(stderr,"Usage: %s own-CT.nds corrected-fear-sheet.png crossbow211.png output-directory\n",argv[0]);return 2;}
    FILE*f=fopen(argv[1],"rb");assert(f);assert(!fseek(f,0,SEEK_END));long size=ftell(f);assert(size>512&&size<=512*1024*1024L);rewind(f);
    rt_rom.size=(size_t)size;rt_rom.data=malloc(size);assert(rt_rom.data&&fread(rt_rom.data,1,size,f)==(size_t)size);fclose(f);assert(!memcmp(rt_rom.data+12,"YQUE",4));gfx_list_files();
    GfxBlob tb=gfx_open(gfx_find("Chara/Chara_0001.NCGR")),cb=gfx_open(gfx_find("Chara/Chara_0001.NCER"));GfxTiles t;GfxPalette pal;assert(gfx_read_tiles(&tb,&t)&&gfx_read_palette(gfx_find("Chara/ObjPlt_0001.NCLR"),&pal));
    for(int i=0;i<4;i++){int id=i<3?89+i:211;Plane a=read_art(i<3?argv[2]:argv[3],i<3?i:0,i<3?3:1);pose(id,&a,&cb,&t,&pal,argv[4]);free(a.p);free(a.label);}
    gfx_close(&tb);gfx_close(&cb);free(rt_rom.data);return 0;
}
