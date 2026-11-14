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
static void text(uint32_t*p,int x,int y,const char*s,uint32_t c) {
    ui_draw_text_in(p,CT_HUD_W,s,x,y,c);
}
static void bar(uint32_t*p,int x,int y,int w,unsigned now,unsigned max,uint32_t c) {
    rect(p,x,y,w,4,RGB(4,6,18));
    if(max){unsigned n=now>max?max:now;int fill=(int)((uint64_t)n*(unsigned)(w-2)/max);
        rect(p,x+1,y+1,fill,2,c);}
}
void ct_hud_draw(uint32_t p[CT_HUD_W*CT_HUD_H],const CtHudState*s,const uint32_t faces[7][48*48]) {
    for(int y=0;y<CT_HUD_H;y++)rect(p,0,y,CT_HUD_W,1,RGB(15+y/8,18+y/10,48+y/3));
    frame(p,0,0,CT_HUD_W,CT_HUD_H,RGB(237,221,184));
    frame(p,1,1,CT_HUD_W-2,CT_HUD_H-2,RGB(124,43,49));
    for(int i=0;i<3;i++){
        int x=160*i;const CtHudMember*m=&s->member[i];
        if(i)rect(p,x,3,1,CT_HUD_H-6,RGB(113,104,126));
        if(!m->present)continue;
        if(m->character<0||m->character>6)continue;
        uint32_t edge=m->hp?RGB(238,223,187):RGB(131,127,144);
        frame(p,x+4,6,52,52,edge);rect(p,x+5,7,50,50,RGB(7,12,28));
        for(int y=0;y<48;y++)for(int k=0;k<48;k++){
            uint32_t c=faces[m->character][y*48+k];if(!(c>>24))continue;
            if(!m->hp){unsigned g=((c&255)+((c>>8)&255)+((c>>16)&255))/3;c=RGB(g,g,g);}
            p[(y+8)*CT_HUD_W+x+6+k]=c;
        }
        text(p,x+61,6,m->name,edge);
        char value[32];snprintf(value,sizeof value,"%u/%u",m->hp,m->max_hp);
        uint32_t hp=m->max_hp&&(uint64_t)m->hp*4<=m->max_hp?RGB(255,187,102):RGB(242,232,208);
        text(p,x+61,20,"HP",RGB(210,205,228));text(p,x+77,20,value,hp);
        bar(p,x+61,29,92,m->hp,m->max_hp,RGB(104,205,130));
        snprintf(value,sizeof value,"%u/%u",m->mp,m->max_mp);
        text(p,x+61,35,"MP",RGB(210,205,228));text(p,x+77,35,value,RGB(242,232,208));
        bar(p,x+61,44,92,m->mp,m->max_mp,RGB(111,157,231));
        text(p,x+61,51,"ATB",RGB(210,205,228));
        bar(p,x+80,52,73,m->atb,m->atb_max,m->atb_max&&m->atb>=m->atb_max?RGB(238,195,91):RGB(206,67,83));
    }
}
