/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "runtime/sprite_hd.h"
#include "runtime/gfx_mod.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int gfxmod_background_allowed(void){return 0;}
int gfxmod_policy_allowed(int p){(void)p;return 0;}
int main(void){
    uint8_t tile[32]={0};uint32_t pal[16],out[1026];
    for(int i=0;i<16;i++)pal[i]=(unsigned)i*0x101010u;
    for(int y=1;y<7;y++)for(int x=1;x<7;x++)tile[y*4+x/2]|=(uint8_t)((x+y<7?4:5)<<(x%2*4));
    for(int s=2;s<=4;s+=2){
        for(int i=0;i<1026;i++)out[i]=0xdeadbeef;
        sprite_hd_make(out+1,tile,s,100,pal);assert(out[0]==0xdeadbeef&&out[1+64*s*s]==0xdeadbeef);
        int changed=0;
        for(int y=0;y<8*s;y++)for(int x=0;x<8*s;x++){
            uint32_t t=out[1+y*8*s+x];unsigned src=(tile[(y/s)*4+(x/s)/2]>>((x/s)%2*4))&15;
            if(!src)assert(!t);
            else assert(t); /* Retain thin original lines and every opaque pixel. */
            if(t){assert((t&255)<16&&((t>>8)&255)<16&&((t>>16)&255)<=48);changed+=t!=(src|(src<<8));}
        }assert(changed);
    }
    memset(tile,0x44,sizeof tile);sprite_hd_make(out,tile,4,0,pal);
    uint16_t live[16];for(int i=0;i<16;i++)live[i]=0x7fff;
    for(int i=0;i<1024;i++)assert(sprite_hd_color(out[i],live)==0xffffffffu);
    memset(live,0,sizeof live);for(int i=0;i<1024;i++)assert(sprite_hd_color(out[i],live)==0xff000000u);
    assert(!sprite_hd_color(0,live));live[4]=31;uint32_t red=sprite_hd_color(out[0],live);live[4]=31<<5;
    assert(red==0xff0000ffu&&sprite_hd_color(out[0],live)==0xff00ff00u);
    GfxHdTile t={4,2,0,out,1,1,0};
    assert(gfxmod_allowed(&t,1,1,0)&&!gfxmod_allowed(&t,1,0,0)&&!gfxmod_allowed(&t,0,1,0));
    assert(!gfxmod_allowed(&t,1,1,-1)&&!gfxmod_allowed(&t,1,1,1));
    t.scope=0;assert(gfxmod_allowed(&t,0,0,-1));
    puts("PASS: 2x/4x bounds, original alpha footprint, indexed shading, live palettes/white hit-flash/black fade, battle-only sprite scope and matching character palette guards");
}
