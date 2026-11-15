/* SPDX-License-Identifier: GPL-3.0-or-later
 * Private own-ROM preview and exhaustive cell-layout/alpha audit.
 * Never commit its output: it contains artwork from your own game file.
 * Build instructions: docs/BATTLE-SPRITES.md. */
#include "rt.h"
#include "gfx_rom.h"
#include "gfx_mod.h"
#include "sprite_hd.h"
#include "ui_font.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
RtRom rt_rom;
static const char*data_root;
const char*rt_data_dir(void){return data_root;}
void rt_log(const char*fmt,...){va_list ap;va_start(ap,fmt);vfprintf(stderr,fmt,ap);fputc('\n',stderr);va_end(ap);}
#define WIDTH 1120
#define HEIGHT 640
static uint32_t canvas[WIDTH*HEIGHT];
static void label(int x,int y,const char*s){
    uint32_t p[256*16]={0};ui_draw_text_in(p,256,s,0,0,0xffe1ded5u);
    for(int r=0;r<12;r++)for(int k=0;k<256;k++)if(p[r*256+k])
        for(int dy=0;dy<2;dy++)for(int dx=0;dx<2;dx++){
            int xx=x+k*2+dx,yy=y+r*2+dy;if(xx>=0&&xx<WIDTH&&yy>=0&&yy<HEIGHT)canvas[yy*WIDTH+xx]=p[r*256+k];
        }
}
static uint32_t rgba(SDL_Color c){return 0xff000000u|c.r|((uint32_t)c.g<<8)|((uint32_t)c.b<<16);}
static uint32_t*render(const GfxLayout*l,const GfxTiles*t,const GfxPalette*p,int enhanced,int character){
    int w=l->w*4,h=l->h*4;uint32_t*out=calloc((size_t)w*h,4);assert(out);
    uint16_t pal[256];for(int i=0;i<256;i++)pal[i]=(p->c[i].r>>3)|((p->c[i].g>>3)<<5)|((p->c[i].b>>3)<<10);
    for(int k=0;k<l->count;k++){
        const GfxPlace*g=l->tiles+k;if(g->off+32>t->size)continue;
        const GfxHdTile*hd=enhanced?gfxmod_tile(gfxmod_find(t->data+g->off,g->bpp8)):NULL;
        if(!gfxmod_allowed(hd,1,1,character))hd=NULL;
        for(int y=0;y<32;y++)for(int x=0;x<32;x++){
            int sx=g->hflip?31-x:x,sy=g->vflip?31-y:y;
            int index=gfx_tile_px(t,g->off,g->bpp8,sx/4,sy/4);if(!index)continue;
            int dx=g->x*4+x,dy=g->y*4+y;if(dx<0||dy<0||dx>=w||dy>=h)continue;
            uint32_t color=rgba(p->c[index+g->row*16]);
            if(hd&&hd->indexed==2){int n=hd->scale*8;unsigned texel=((const uint32_t*)hd->px)[sy*hd->scale/4*n+sx*hd->scale/4];color=sprite_hd_color(texel,pal+g->row*16);}
            if(color)out[dy*w+dx]=color;
        }
    }return out;
}
int main(int argc,char**argv){
    if(argc!=4){fprintf(stderr,"Usage: %s own-CT.nds data-root-with-trailing-slash private-preview.ppm\n",argv[0]);return 2;}
    data_root=argv[2];FILE*f=fopen(argv[1],"rb");assert(f);assert(!fseek(f,0,SEEK_END));long size=ftell(f);assert(size>512&&size<=512*1024*1024L);rewind(f);
    rt_rom.size=(size_t)size;rt_rom.data=malloc(size);assert(rt_rom.data&&fread(rt_rom.data,1,size,f)==(size_t)size);fclose(f);
    assert(!memcmp(rt_rom.data+12,"YQUE",4));memcpy(rt_rom.code,"YQUE",5);
    uint8_t*original=malloc(size);assert(original);memcpy(original,rt_rom.data,size);
    sprite_hd_load();assert(gfxmod_hd_scale()&&gfxmod_hd_active(1)&&!gfxmod_hd_active(0));
    for(int i=0;i<WIDTH*HEIGHT;i++)canvas[i]=0xff211c19u;
    label(22,10,"CTPS5 Faithful HD - actual renderer comparison");
    static const char*names[]={"Crono","Marle","Lucca","Robo","Frog","Ayla","Magus"};
    unsigned total=0,visible=0,shaded=0;
    for(int c=0;c<7;c++){
        char path[128];snprintf(path,sizeof path,"Chara/Chara_%04d.NCGR",c);GfxBlob b=gfx_open(gfx_find(path));GfxTiles t;GfxPalette p;assert(gfx_read_tiles(&b,&t));gfx_palette_for(path,0,&p);
        snprintf(path,sizeof path,"Chara/Chara_%04d.NCER",c);GfxBlob cb=gfx_open(gfx_find(path));unsigned count=gfx_cell_count(&cb);assert(count);
        unsigned changed_frames=0;
        uint16_t normal[16];uint8_t live[0x800]={0};
        for(int i=0;i<16;i++)normal[i]=(p.c[i].r>>3)|((p.c[i].g>>3)<<5)|((p.c[i].b>>3)<<10);
        assert(sprite_hd_classify_palette(normal)==c);
        sprite_hd_begin_frame(live,0);assert(sprite_hd_palette(0,c)==-1);
        memcpy(live+0x200+c*32,normal,sizeof normal);sprite_hd_begin_frame(live,1);assert(sprite_hd_palette(0,c)==c);
        memset(live+0x200+c*32,0xff,32);sprite_hd_begin_frame(live,1);assert(sprite_hd_palette(0,c)==c);
        sprite_hd_begin_frame(live,0);assert(sprite_hd_palette(0,c)==-1);
        for(unsigned frame=0;frame<count;frame++){
            GfxLayout l;assert(gfx_cell_layout(&cb,frame,&l));uint32_t*old=render(&l,&t,&p,0,c),*now=render(&l,&t,&p,1,c);int changed=0;
            for(int i=0;i<l.w*l.h*16;i++){
                assert((old[i]!=0)==(now[i]!=0)); /* Exact native alpha footprint. */
                visible+=now[i]!=0;shaded+=old[i]!=now[i];changed|=old[i]!=now[i];
            }
            changed_frames+=changed;total++;
            if(frame==0){
                int x0=4*(c%4)*70,y0=44+(c/4)*278;label(x0+12,y0,names[c]);label(x0+12,y0+25,"Original");label(x0+142,y0+25,"HD 4x");
                int left=l.w*4,right=0,top=l.h*4,bottom=0;
                for(int y=0;y<l.h*4;y++)for(int x=0;x<l.w*4;x++)if(old[y*l.w*4+x]){if(x<left)left=x;if(x>right)right=x;if(y<top)top=y;if(y>bottom)bottom=y;}
                for(int v=0;v<2;v++)for(int y=top;y<=bottom;y++)for(int x=left;x<=right;x++){
                    int dx=x0+v*140+70-(right-left+1)/2+x-left,dy=y0+240-(bottom-top)+y-top;
                    uint32_t color=(v?now:old)[y*l.w*4+x];if(color&&dx>=0&&dx<WIDTH&&dy>=0&&dy<HEIGHT)canvas[dy*WIDTH+dx]=color;
                }
            }
            free(old);free(now);free(l.tiles);
        }
        fprintf(stderr,"AUDIT %s: %u/%u cells rendered, %u cells enhanced\n",names[c],count,count,changed_frames);assert(changed_frames);gfx_close(&b);gfx_close(&cb);
    }
    assert(!memcmp(original,rt_rom.data,size));free(original);
    label(22,603,"Own-ROM render; no frame/timing/layout changes. Not a PS5 capture.");
    f=fopen(argv[3],"wb");assert(f);fprintf(f,"P6\n%d %d\n255\n",WIDTH,HEIGHT);
    for(int i=0;i<WIDTH*HEIGHT;i++){fputc(canvas[i]&255,f);fputc((canvas[i]>>8)&255,f);fputc((canvas[i]>>16)&255,f);}assert(!ferror(f));assert(!fclose(f));
    fprintf(stderr,"PASS: %u cells, %u visible subpixels, %u changed subpixels; exact ROM memory unchanged; native transparent footprint preserved\n",total,visible,shaded);free(rt_rom.data);return 0;
}
