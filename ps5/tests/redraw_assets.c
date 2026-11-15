/* SPDX-License-Identifier: GPL-3.0-or-later
 * Optional own-ROM/own-art integration test; no game data embedded. */
#include "runtime/rt.h"
#include "runtime/redraw.h"
#include "runtime/gfx_rom.h"
#include "runtime/gfx_mod.h"
#include "runtime/sprite_hd.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
RtRom rt_rom;static const char*root;
const char*rt_data_dir(void){return root;}
void rt_log(const char*f,...){va_list a;va_start(a,f);vfprintf(stderr,f,a);fputc('\n',stderr);va_end(a);}
static int fixture(int cell,RedrawObject live[128],const GfxBlob*cb,const GfxTiles*t,int *x0,int*y0){
    GfxObject obj[8];int n=gfx_cell_objects(cb,(unsigned)cell,obj,8),count=0;assert(n);
    memset(live,0,128*sizeof *live);*x0=*y0=999;
    for(int k=0;k<n;k++){
        assert(obj[k].w==16&&obj[k].h==16&&obj[k].off+128<=t->size);
        if(obj[k].x<*x0)*x0=obj[k].x;if(obj[k].y<*y0)*y0=obj[k].y;
        int opaque=0;for(int i=0;i<128;i++)opaque|=t->data[obj[k].off+i];if(!opaque)continue;
        RedrawObject*b=live+3+count*17;*b=(RedrawObject){.valid=1,.x=(120+obj[k].x)&511,.y=(144+obj[k].y)&255,.flip=obj[k].hflip|obj[k].vflip<<1,.palette=2,.priority=1};
        memcpy(b->bytes,t->data+obj[k].off,128);count++;
    }return count;
}
int main(int argc,char**argv){
    assert(argc==3||argc==4);root=argv[2];int missing=argc==4;
    FILE*f=fopen(argv[1],"rb");assert(f);fseek(f,0,SEEK_END);long size=ftell(f);assert(size>512);rewind(f);rt_rom.size=(size_t)size;rt_rom.data=malloc(size);assert(rt_rom.data);assert(fread(rt_rom.data,1,size,f)==(size_t)size);fclose(f);memcpy(rt_rom.code,"YQUE",5);
    uint8_t*before=malloc(size);assert(before);memcpy(before,rt_rom.data,size);
    ct_redraw_load();assert(ct_redraw_count()==(missing?3:4));assert(!gfxmod_hd_active(0)&&!gfxmod_hd_active(1));
    GfxBlob tb=gfx_open(gfx_find("Chara/Chara_0001.NCGR")),cb=gfx_open(gfx_find("Chara/Chara_0001.NCER"));GfxTiles tiles;GfxPalette pal;assert(gfx_read_tiles(&tb,&tiles));assert(gfx_read_palette(gfx_find("Chara/ObjPlt_0001.NCLR"),&pal));
    uint16_t palette[256]={0};for(int i=0;i<16;i++)palette[32+i]=(pal.c[i].r>>3)|((pal.c[i].g>>3)<<5)|((pal.c[i].b>>3)<<10);
    int ids[]={89,90,91,211};
    for(int q=0;q<4;q++){
        RedrawObject live[128],copy[128];int x0,y0,n=fixture(ids[q],live,&cb,&tiles,&x0,&y0);memcpy(copy,live,sizeof live);
        if((missing&&q==0)||ids[q]==211){assert(!ct_redraw_prepare(0,live,palette));assert(!ct_redraw_binding(0,3));continue;}
        assert(ct_redraw_prepare(0,live,palette)==1&&!memcmp(copy,live,sizeof live));
        const RedrawBinding*b=ct_redraw_binding(0,3);assert(b&&b->action==2&&b->x==((120+x0)&511)&&b->y==((144+y0)&255));
        for(int k=1;k<n;k++){const RedrawBinding*s=ct_redraw_binding(0,3+k*17);assert(s&&s->action==1);}
        GfxLayout l;assert(gfx_cell_layout(&cb,(unsigned)ids[q],&l));assert(b->w==l.w&&b->h==l.h);free(l.tiles);
        uint32_t word=0;unsigned visible=0;
        for(int y=0;y<b->h;y++)for(int x=0;x<b->w;x++){
            uint32_t texel,id=ct_redraw_sample(b->pose,x,y,&texel);if(!id)continue;visible++;assert((texel>>24)>=128);
            const GfxHdTile*t=gfxmod_tile(id);assert(t&&t->indexed==3&&t->scale==4);
            for(int i=0;i<64*t->scale*t->scale;i++)if(((const uint32_t*)t->px)[i]){word=((const uint32_t*)t->px)[i];break;}
        }assert(visible&&word);
        uint32_t texel;assert(!ct_redraw_sample(b->pose,-1,0,&texel)&&!ct_redraw_sample(b->pose,b->w,0,&texel));
        uint16_t effects[16];for(int i=0;i<16;i++)effects[i]=0x7fff;assert(ct_redraw_color(word,effects)==0xffffffffu);
        memset(effects,0,sizeof effects);assert(ct_redraw_color(word,effects)==0xff000000u);
        memcpy(effects,palette+32,sizeof effects);uint32_t body=0xff7999c3u;
        assert(ct_redraw_color(body,effects)==body);
        effects[13]=0x5717;effects[14]=0x31f0;effects[15]=0x252b;
        assert(ct_redraw_color(body,effects)==body); /* Weapon slots do not recolour body shadows. */
        effects[1]=0x280;assert(ct_redraw_color(0xfe010203u,effects)==0xff00a500u);
        uint16_t changed_weapon[256];memcpy(changed_weapon,palette,sizeof palette);memcpy(changed_weapon+32,effects,sizeof effects);
        assert(ct_redraw_prepare(0,live,changed_weapon)==1);
        live[3].bytes[0]^=1;assert(!ct_redraw_prepare(0,live,palette)&&!ct_redraw_binding(0,3));memcpy(live,copy,sizeof live);
        uint16_t foreign[256];memcpy(foreign,palette,sizeof foreign);for(int i=0;i<16;i++)foreign[32+i]=(uint16_t)(i*37);
        assert(!ct_redraw_prepare(0,live,foreign));
        ct_redraw_set_visible(0);assert(!ct_redraw_prepare(0,live,palette));ct_redraw_set_visible(1);
        ct_redraw_set_enabled(0);assert(!ct_redraw_enabled()&&!ct_redraw_prepare(0,live,palette));ct_redraw_set_enabled(1);assert(ct_redraw_enabled());
    }
    for(unsigned off=0;off+32<=tiles.size;off+=32)assert(!gfxmod_find(tiles.data+off,0)); /* No tile-hash contamination. */
    RedrawObject ordinary[128];int x0,y0;fixture(0,ordinary,&cb,&tiles,&x0,&y0);assert(!ct_redraw_prepare(0,ordinary,palette));
    assert(!memcmp(before,rt_rom.data,size));assert(ct_redraw_viewer_open()&&ct_redraw_viewer_active());ct_redraw_viewer_step(1);ct_redraw_viewer_close();assert(!ct_redraw_viewer_active());
    gfx_close(&tb);gfx_close(&cb);free(before);free(rt_rom.data);
    puts("PASS: own-ROM PNG registration, real cell groups, actor origins, direct frame tiles, missing-art/foreign-pose/palette fallback, white/black effects, disabled module and exact ROM preservation");
}
