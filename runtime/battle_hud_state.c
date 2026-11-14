/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "battle_hud_state.h"
#include <stdio.h>
#include <string.h>
#define RAM_BASE 0x02000000u
#define RAM_SIZE 0x00400000u
static unsigned get8(const unsigned char*r,unsigned a){return r[a-RAM_BASE];}
static unsigned get16(const unsigned char*r,unsigned a){return get8(r,a)|(get8(r,a+1)<<8);}
static unsigned get32(const unsigned char*r,unsigned a){return get16(r,a)|(get16(r,a+2)<<16);}
static int span(unsigned a,unsigned n){return a>=RAM_BASE&&a-RAM_BASE<=RAM_SIZE-n;}
int ct_hud_read_state(CtHudState*out,const unsigned char*r,size_t size){
    static const char *names[]={"Crono","Marle","Lucca","Robo","Frog","Ayla","Magus"};
    CtHudState s={.selected=-1,.command=-1};if(!out)return 0;memset(out,0,sizeof *out);
    if(!r||size<RAM_SIZE)return 0;
    unsigned scene=get32(r,0x0207088c),party=get32(r,0x020728fc),members=get32(r,0x02118c0c);
    if(!span(scene,4)||get32(r,scene)!=0x02115558u||!span(party,3)||!span(members,0x1942))return 0;
    unsigned seen=0,count=0;
    for(unsigned i=0;i<3;i++){
        unsigned c=get8(r,party+i);if(c==0x80)continue;
        if(c>=7||(seen&(1u<<c)))return 0;
        seen|=1u<<c;
        if(!get8(r,members+0x1545+i))continue; /* Same occupied-slot gate as the game's gauge updater. */
        unsigned b=members+i*0x80;CtHudMember*m=&s.member[i];
        m->hp=get16(r,b+3);m->max_hp=get16(r,b+5);m->mp=get16(r,b+7);m->max_mp=get16(r,b+9);
        if(!m->max_hp||m->max_hp>9999||m->hp>m->max_hp||m->max_mp>999||m->mp>m->max_mp)return 0;
        /* ov003 02144028: the game's displayed countdown +15d2 and maximum
         * +15e0. Its own 32-pixel gauge uses 32-min(remaining*32/max,32).
         * Reading this mirror preserves the game's update/Wait timing. */
        unsigned left=get8(r,members+0x15d2+i),max=get8(r,members+0x15e0+i);
        m->atb_max=max;m->atb=m->hp&&max?(left<max?max-left:0):0;
        m->character=(int)c;m->present=1;snprintf(m->name,sizeof m->name,"%s",names[c]);count++;
    }
    if(!count)return 0;
    unsigned selected=get8(r,members+0x14eb);
    if(selected<3&&s.member[selected].present)s.selected=(int)selected;
    *out=s;return 1;
}
