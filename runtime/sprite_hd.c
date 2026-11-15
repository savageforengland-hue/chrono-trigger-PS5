/* SPDX-License-Identifier: GPL-3.0-or-later
 * Faithful HD: deterministic, palette-relative detail from the player's tiles.
 * No animation, cell layout, coordinates, game data or ROM bytes are changed. */
#include "sprite_hd.h"
#include "gfx_mod.h"
#include "gfx_rom.h"
#include "rt.h"
#include "dispatch.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static uint16_t canonical[7][16];
static int palettes_loaded;
static unsigned valid_palettes;
static unsigned logged_owners;
static signed char palette_owner[2][16];
int sprite_hd_classify_palette(const uint16_t pal[16]){
    if(!palettes_loaded)return -1;
    for(int c=0;c<7;c++){
        if(!(valid_palettes&(1u<<c)))continue;
        int match=1;for(int i=1;i<16;i++)if((pal[i]&0x7fff)!=canonical[c][i]){match=0;break;}
        if(match)return c;
    }return -1;
}
void sprite_hd_begin_frame(const uint8_t*pal,int battle){
    if(!battle){memset(palette_owner,-1,sizeof palette_owner);logged_owners=0;return;}
    unsigned owners=0;
    for(int e=0;e<2;e++)for(int r=0;r<16;r++){
        uint16_t p[16];memcpy(p,pal+e*0x400+0x200+r*32,sizeof p);
        int c=sprite_hd_classify_palette(p);
        /* Party OBJ palette slots keep their identity during palette flashes
         * in the same battle; a normal player palette rebinds the slot. */
        if(c>=0)palette_owner[e][r]=(signed char)c;
        if(palette_owner[e][r]>=0)owners|=1u<<palette_owner[e][r];
    }
    if(owners&~logged_owners){rt_log("battle sprites: matched player palette mask 0x%02x",owners);logged_owners|=owners;}
}
int sprite_hd_palette(int engine,int row){return palette_owner[engine&1][row&15];}
static uint8_t at(const uint8_t*p,int n,int x,int y){
    if(x<0)x=0;
    if(y<0)y=0;
    if(x>=n)x=n-1;
    if(y>=n)y=n-1;
    return p[y*n+x];
}
static void double_pixels(uint8_t*out,const uint8_t*in,int n){
    for(int y=0;y<n;y++)for(int x=0;x<n;x++){
        uint8_t b=at(in,n,x,y-1),d=at(in,n,x-1,y),e=at(in,n,x,y),f=at(in,n,x+1,y),h=at(in,n,x,y+1);
        uint8_t a=e,c=e,g=e,i=e;
        if(b!=h&&d!=f){if(d==b)a=d;if(b==f)c=f;if(d==h)g=d;if(h==f)i=f;}
        int k=y*2*n*2+x*2;out[k]=a;out[k+1]=c;out[k+n*2]=g;out[k+n*2+1]=i;
    }
}
static int similar(uint32_t a,uint32_t b){
    int sum=0;for(int s=0;s<24;s+=8){int d=(int)((a>>s)&255)-(int)((b>>s)&255);sum+=d*d;}
    return sum<=5200;
}
void sprite_hd_make(uint32_t*out,const uint8_t tile[32],int scale,int strength,const uint32_t pal[16]){
    uint8_t src[64],twice[256],four[1024];
    if(scale!=2&&scale!=4)return;
    if(strength<0)strength=0;
    if(strength>100)strength=100;
    for(int i=0;i<64;i++)src[i]=(tile[i/2]>>(i%2*4))&15;
    double_pixels(twice,src,8);if(scale==4)double_pixels(four,twice,16);
    const uint8_t*up=scale==4?four:twice;int n=8*scale;
    for(int y=0;y<n;y++)for(int x=0;x<n;x++){
        int nx=x/scale,ny=y/scale;uint8_t native=src[ny*8+nx],base=up[y*n+x];
        /* The original transparent footprint is an absolute clipping mask.
         * Native tile provenance cannot expand into a transparent DS pixel. */
        if(!native){out[y*n+x]=0;continue;}
        /* Preserve fine one-pixel lines as well as outer alpha. Do not cut
         * holes in glasses, weapons, fingers or other narrow details. */
        if(!base||!similar(pal[native],pal[base]))base=native;
        int dx=x%scale<scale/2?-1:1,dy=y%scale<scale/2?-1:1;
        uint8_t near=at(src,8,nx,ny+dy);
        if(!near||near==base||!similar(pal[base],pal[near]))near=at(src,8,nx+dx,ny);
        if(!near||!similar(pal[base],pal[near]))near=base;
        unsigned weight=(unsigned)(strength*48/100);
        out[y*n+x]=base|((uint32_t)near<<8)|(weight<<16);
    }
}
static uint32_t rgb(uint16_t c){
    unsigned r=c&31,g=(c>>5)&31,b=(c>>10)&31;
    return (r<<3|r>>2)|((g<<3|g>>2)<<8)|((b<<3|b>>2)<<16);
}
uint32_t sprite_hd_color(uint32_t t,const uint16_t*pal){
    if(!(t&255))return 0;
    uint32_t a=rgb(pal[t&255]),b=rgb(pal[(t>>8)&255]),mix=(t>>16)&255,out=0xff000000u;
    for(int s=0;s<24;s+=8)out|=((((a>>s)&255)*(256-mix)+((b>>s)&255)*mix)/256)<<s;
    return out;
}
int sprite_hd_battle(void){
    if(!rt_overlay_resident(0)||!rt_overlay_resident(3))return 0;
    uint32_t scene=rt_r32(0x0207088c);
    return scene>=0x02000000&&scene<=0x023ffffc&&rt_r32(scene)==0x02115558;
}
void sprite_hd_load(void){
    char path[1024];snprintf(path,sizeof path,"%smods/battle-sprites/sprites.ini",rt_data_dir());
    FILE*f=fopen(path,"r");if(!f)return;
    int enabled=0,scale=4,strength=65;char line[192];
    while(fgets(line,sizeof line,f)){
        int v;char extra;
        if(sscanf(line," enabled = %d %c",&v,&extra)==1)enabled=v==1;
        else if(sscanf(line," scale = %d %c",&v,&extra)==1&&(v==2||v==4))scale=v;
        else if(sscanf(line," shading = %d %c",&v,&extra)==1&&v>=0&&v<=100)strength=v;
    }fclose(f);if(!enabled)return;
    if(strcmp(rt_rom.code,"YQUE")||!gfx_list_files())return;
    memset(palette_owner,-1,sizeof palette_owner);
    unsigned added=0,poses=0;
    for(int c=0;c<7;c++){
        snprintf(path,sizeof path,"Chara/Chara_%04d.NCGR",c);
        GfxBlob b=gfx_open(gfx_find(path));GfxTiles t;GfxPalette p;
        if(!gfx_read_tiles(&b,&t)||t.bpp8){gfx_close(&b);rt_log("battle sprites: character %d unavailable; original retained",c);continue;}
        gfx_palette_for(path,0,&p);uint32_t palette[16];
        if(!p.from||p.colors<16){gfx_close(&b);continue;}
        for(int i=0;i<16;i++){
            palette[i]=p.c[i].r|((uint32_t)p.c[i].g<<8)|((uint32_t)p.c[i].b<<16);
            canonical[c][i]=(p.c[i].r>>3)|((p.c[i].g>>3)<<5)|((p.c[i].b>>3)<<10);
        }
        valid_palettes|=1u<<c;
        snprintf(path,sizeof path,"Chara/Chara_%04d.NCER",c);GfxBlob cells=gfx_open(gfx_find(path));
        unsigned pose_count=gfx_cell_count(&cells);poses+=pose_count;gfx_close(&cells);
        unsigned before=added;
        for(uint32_t off=0;off+32<=t.size;off+=32){
            const uint8_t*tile=t.data+off;int opaque=0;for(int i=0;i<32;i++)opaque|=tile[i];
            if(!opaque)continue;
            if(gfxmod_find(tile,0)){gfxmod_sprite_character(tile,c);continue;} /* Existing texture packs win. */
            uint32_t*px=malloc((size_t)8*scale*8*scale*4);if(!px)break;
            sprite_hd_make(px,tile,scale,strength,palette);
            added+=gfxmod_add_sprite(tile,scale,px,c);
        }
        rt_log("battle sprites: character %d, %u unique tiles, %u poses",c,added-before,pose_count);
        gfx_close(&b);
    }
    palettes_loaded=1;
    rt_log("battle sprites: Faithful HD %dx, shading %d, %u tiles, %u source poses; battle and matching player palette only",scale,strength,added,poses);
}
