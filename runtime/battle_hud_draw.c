/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "battle_hud_draw.h"
#include "ui_font.h"
#include <stdio.h>
#include <string.h>
#define RGB(r,g,b) (0xff000000u | (uint32_t)(b)<<16 | (uint32_t)(g)<<8 | (r))
static void rect(uint32_t *p,int x,int y,int w,int h,uint32_t c) {
    for(int r=y;r<y+h;r++)for(int k=x;k<x+w;k++)
        if(r>=0&&r<CT_HUD_H&&k>=0&&k<CT_HUD_W)p[r*CT_HUD_W+k]=c;
}
static void frame(uint32_t*p,int x,int y,int w,int h,uint32_t c) {
    rect(p,x,y,w,1,c);rect(p,x,y+h-1,w,1,c);rect(p,x,y,1,h,c);rect(p,x+w-1,y,1,h,c);
}
static void text(uint32_t*p,int x,int y,const char*s,uint32_t c,int scale) {
    uint32_t glyphs[128*16]={0};
    ui_draw_text_in(glyphs,128,s,0,0,c);
    int w=ui_text_width(s)+1;
    if(w>128)w=128;
    /* scale 2: 1x; 3: 1.5x; 4: 2x. Preserve the source pixel font. */
    for(int r=0;r<9*scale/2;r++)for(int k=0;k<w*scale/2;k++) {
        uint32_t ink=glyphs[(r*2/scale)*128+k*2/scale];
        if(ink)rect(p,x+k,y+r,1,1,ink);
    }
}
static void bar(uint32_t*p,int x,int y,int w,unsigned now,unsigned max,uint32_t c) {
    rect(p,x,y,w,4,RGB(4,6,18));
    if(max){unsigned n=now>max?max:now;int fill=(int)((uint64_t)n*(unsigned)(w-2)/max);
        rect(p,x+1,y+1,fill,2,c);}
}
void ct_hud_draw(uint32_t p[CT_HUD_W*CT_HUD_H],const CtHudState*s,const uint32_t faces[7][48*48]) {
    for(int y=0;y<CT_HUD_H;y++)rect(p,0,y,CT_HUD_W,1,RGB(15+y/16,18+y/20,48+y/6));
    frame(p,0,0,CT_HUD_W,CT_HUD_H,RGB(237,221,184));
    frame(p,1,1,CT_HUD_W-2,CT_HUD_H-2,RGB(124,43,49));
    for(int i=0;i<3;i++){
        int x=160*i;const CtHudMember*m=&s->member[i];
        if(i)rect(p,x,3,1,CT_HUD_H-6,RGB(113,104,126));
        if(!m->present||m->character<0||m->character>6)continue;
        uint32_t edge=m->hp?RGB(238,223,187):RGB(131,127,144);
        if(s->selected==i&&m->hp)frame(p,x+2,2,156,CT_HUD_H-4,RGB(238,195,91));
        text(p,x+6,5,m->name,edge,4);
        frame(p,x+4,23,74,74,edge);
        rect(p,x+5,24,72,72,RGB(7,12,28));
        for(int y=0;y<72;y++)for(int k=0;k<72;k++){
            uint32_t c=faces[m->character][(y*2/3)*48+k*2/3];
            if(!(c>>24))continue;
            if(!m->hp){unsigned g=((c&255)+((c>>8)&255)+((c>>16)&255))/3;c=RGB(g,g,g);}
            p[(y+24)*CT_HUD_W+x+5+k]=c;
        }
        char value[32];snprintf(value,sizeof value,"%u/%u",m->hp,m->max_hp);
        uint32_t hp=m->max_hp&&(uint64_t)m->hp*4<=m->max_hp?RGB(255,187,102):RGB(242,232,208);
        text(p,x+83,26,"HP",RGB(210,205,228),3);
        text(p,x+83,42,value,hp,3);
        bar(p,x+83,58,69,m->hp,m->max_hp,RGB(104,205,130));
        snprintf(value,sizeof value,"%u/%u",m->mp,m->max_mp);
        text(p,x+83,67,"MP",RGB(210,205,228),3);
        text(p,x+83,83,value,RGB(242,232,208),3);
        bar(p,x+83,99,69,m->mp,m->max_mp,RGB(111,157,231));
        text(p,x+6,111,"ATB",RGB(210,205,228),3);
        bar(p,x+40,114,112,m->atb,m->atb_max,m->atb_max&&m->atb>=m->atb_max?RGB(238,195,91):RGB(206,67,83));
    }
}
