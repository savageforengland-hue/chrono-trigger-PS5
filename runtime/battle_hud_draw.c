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
static void text_impl(uint32_t*p,int x,int y,const char*s,uint32_t c,int scale,int outline) {
    uint32_t glyphs[128*16]={0};
    ui_draw_text_in(glyphs,128,s,0,0,c);
    int w=ui_text_width(s)+1;
    if(w>128)w=128;
    /* scale 2: 1x; 3: 1.5x; 4: 2x. Preserve the source pixel font. */
    if(outline)for(int r=0;r<9*scale/2;r++)for(int k=0;k<w*scale/2;k++){
        if(glyphs[(r*2/scale)*128+k*2/scale])rect(p,x+k-outline,y+r-outline,1+outline*2,1+outline*2,RGB(0,0,0));
    }
    for(int r=0;r<9*scale/2;r++)for(int k=0;k<w*scale/2;k++) {
        uint32_t ink=glyphs[(r*2/scale)*128+k*2/scale];
        if(ink)rect(p,x+k,y+r,1,1,ink);
    }
}
static void text(uint32_t*p,int x,int y,const char*s,uint32_t c,int scale){text_impl(p,x,y,s,c,scale,0);}
static uint32_t shade(uint32_t c,int amount){
    int r=(int)(c&255)+amount,g=(int)((c>>8)&255)+amount,b=(int)((c>>16)&255)+amount;
    r=r<0?0:r>255?255:r;g=g<0?0:g>255?255:g;b=b<0?0:b>255?255:b;
    return RGB(r,g,b);
}
static uint32_t blend(uint32_t a,uint32_t b,int part){
    unsigned r=((a&255)*(256-part)+(b&255)*part)/256;
    unsigned g=(((a>>8)&255)*(256-part)+((b>>8)&255)*part)/256;
    unsigned v=(((a>>16)&255)*(256-part)+((b>>16)&255)*part)/256;
    return RGB(r,g,v);
}
static void window(uint32_t*p,const uint32_t*skin){
    for(int y=0;y<CT_HUD_H;y++)for(int x=0;x<CT_HUD_W;x++){
        uint32_t c;
        if(skin){
            /* Preserve the game's bevel/corners; repeat its grain horizontally
             * and stretch only the middle vertically. All art stays ROM-owned. */
            int sx=x<16?x/2:x>=CT_HUD_W-16?80-(CT_HUD_W-x+1)/2:8+(x-16)/2%64;
            int sy=y<16?y/2:y>=CT_HUD_H-16?32-(CT_HUD_H-y+1)/2:8+(y-16)*16/(CT_HUD_H-32);
            c=skin[sy*80+sx];if(!(c>>24))c=RGB(20,26,31);
        }else{
            unsigned h=(unsigned)x*374761393u+(unsigned)y*668265263u;h=(h^(h>>13))*1274126177u;
            int grain=(int)((h^(h>>16))%11)-5;
            int v=62-y*22/CT_HUD_H+grain;c=RGB(v,v+7,v+14);
            int edge=x<y?x:y;if(CT_HUD_W-1-x<edge)edge=CT_HUD_W-1-x;if(CT_HUD_H-1-y<edge)edge=CT_HUD_H-1-y;
            if(edge==0)c=RGB(35,43,50);
            else if(edge==1)c=x<CT_HUD_W/2&&y<CT_HUD_H/2?RGB(156,164,172):RGB(53,61,69);
        }
        p[y*CT_HUD_W+x]=c;
    }
}
static void bar(uint32_t*p,int x,int y,int w,unsigned now,unsigned max,uint32_t c){
    rect(p,x,y,w,4,RGB(24,30,36));rect(p,x,y+3,w,1,RGB(100,108,116));
    rect(p,x+1,y+1,w-2,2,RGB(46,53,60));
    if(max){unsigned n=now>max?max:now;int fill=(int)((uint64_t)n*(unsigned)(w-2)/max);
        rect(p,x+1,y+1,fill,1,shade(c,17));rect(p,x+1,y+2,fill,1,c);}
}
void ct_hud_draw(uint32_t p[CT_HUD_W*CT_HUD_H],const CtHudState*s,const uint32_t faces[7][48*48],const uint32_t*skin) {
    window(p,skin);
    for(int i=0;i<3;i++){
        int x=160*i;const CtHudMember*m=&s->member[i];
        if(i)for(int y=12;y<CT_HUD_H-12;y++){
            p[y*CT_HUD_W+x]=blend(p[y*CT_HUD_W+x],RGB(22,30,37),88);
            p[y*CT_HUD_W+x+1]=blend(p[y*CT_HUD_W+x+1],RGB(160,169,178),40);
        }
        if(!m->present||m->character<0||m->character>6)continue;
        uint32_t edge=m->hp?RGB(239,242,245):RGB(143,151,159);
        if(s->selected==i&&m->hp)text(p,x+57,6,">",edge,2);
        text(p,x+66,6,m->name,edge,3);
        frame(p,x+5,12,50,50,RGB(28,35,42));
        rect(p,x+5,12,50,1,s->selected==i?RGB(196,204,212):RGB(137,146,155));
        rect(p,x+5,12,1,50,RGB(129,139,149));
        rect(p,x+6,13,48,48,RGB(22,29,35));
        for(int y=0;y<48;y++)for(int k=0;k<48;k++){
            uint32_t c=faces[m->character][y*48+k];
            if(!(c>>24))continue;
            if(!m->hp){unsigned g=((c&255)+((c>>8)&255)+((c>>16)&255))/3;c=RGB(g,g,g);}
            p[(y+13)*CT_HUD_W+x+6+k]=c;
        }
        char value[32];snprintf(value,sizeof value,"%u/%u",m->hp,m->max_hp);
        uint32_t hp=m->max_hp&&(uint64_t)m->hp*4<=m->max_hp?RGB(255,187,102):RGB(237,241,245);
        text(p,x+60,25,"HP",RGB(195,206,216),2);
        text(p,x+80,25,value,hp,2);
        bar(p,x+60,35,92,m->hp,m->max_hp,RGB(105,164,119));
        snprintf(value,sizeof value,"%u/%u",m->mp,m->max_mp);
        text(p,x+60,45,"MP",RGB(195,206,216),2);
        text(p,x+80,45,value,RGB(237,241,245),2);
        bar(p,x+60,55,92,m->mp,m->max_mp,RGB(115,151,181));
        text(p,x+6,66,"ATB",RGB(195,206,216),2);
        bar(p,x+34,68,118,m->atb,m->atb_max,m->atb_max&&m->atb>=m->atb_max?RGB(209,224,231):RGB(139,165,181));
    }
}
void ct_hud_draw_commands(uint32_t p[CT_CMD_W*CT_CMD_H],int command,const uint32_t icons[3][32*32]){
    uint32_t canvas[CT_HUD_W*CT_HUD_H]={0};
    static const char *names[]={"Attack","Tech","Item"};
    for(int i=0;i<3;i++){
        int x=i*80;uint32_t edge=i==command?RGB(238,195,91):RGB(238,223,187);
        if(i==command)rect(canvas,x+24,35,32,2,edge);
        /* Three-pixel black silhouette, with one light inner pixel so dark
         * Tech artwork remains legible on dark as well as light battlefields. */
        for(int y=-3;y<35;y++)for(int k=-3;k<35;k++){
            uint32_t c=0;int inner=0,outer=0;
            if(y>=0&&y<32&&k>=0&&k<32)c=icons[i][y*32+k];
            if(!(c>>24))for(int dy=-3;dy<=3;dy++)for(int dx=-3;dx<=3;dx++){
                int sy=y+dy,sx=k+dx;
                if(dx*dx+dy*dy>9||sy<0||sy>=32||sx<0||sx>=32||!(icons[i][sy*32+sx]>>24))continue;
                outer=1;if(dx*dx+dy*dy<=2)inner=1;
            }
            if(!(c>>24)&&outer)c=inner?RGB(238,223,187):RGB(0,0,0);
            if(c>>24)canvas[(y+3)*CT_HUD_W+x+24+k]=c;
        }
        int w=ui_text_width(names[i])*3/2;
        text_impl(canvas,x+(80-w)/2,38,names[i],edge,3,2);
    }
    for(int y=0;y<CT_CMD_H;y++)memcpy(p+y*CT_CMD_W,canvas+y*CT_HUD_W,CT_CMD_W*4);
}
