/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "runtime/redraw_match.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void){
    RedrawReference r={.count=4};RedrawObject live[128]={0},copy[128];int slots[]={3,17,54,100},found[8];
    for(int k=0;k<4;k++){
        r.object[k]=(RedrawObject){.valid=1,.x=(k%2)*16-8,.y=(k/2)*16-40,.flip=k};memset(r.object[k].bytes,k+1,128);
        live[slots[k]]=r.object[k];live[slots[k]].x=(r.object[k].x+508)&511;live[slots[k]].y=(r.object[k].y+4)&255;
        live[slots[k]].palette=2;live[slots[k]].priority=1;live[slots[k]].space=1;
    }
    memcpy(copy,live,sizeof live);assert(redraw_match(&r,live,3,found));assert(!memcmp(found,slots,sizeof slots));assert(!memcmp(live,copy,sizeof live));
    live[17].valid=0;assert(!redraw_match(&r,live,3,found));memcpy(live,copy,sizeof live);
    live[17].x++;assert(!redraw_match(&r,live,3,found));memcpy(live,copy,sizeof live);
    live[17].flip^=1;assert(!redraw_match(&r,live,3,found));memcpy(live,copy,sizeof live);
    live[17].bytes[30]^=1;assert(!redraw_match(&r,live,3,found));memcpy(live,copy,sizeof live);
    live[17].palette=3;assert(!redraw_match(&r,live,3,found));memcpy(live,copy,sizeof live);
    live[17].priority=2;assert(!redraw_match(&r,live,3,found));memcpy(live,copy,sizeof live);
    live[17].space=0;assert(!redraw_match(&r,live,3,found));memcpy(live,copy,sizeof live);
    live[18]=live[17];assert(!redraw_match(&r,live,3,found));
    assert(!redraw_match(&r,live,-1,found)&&!redraw_match(&r,live,128,found));
    puts("PASS: exact whole-pose bytes, wraps, flips, palette/priority/coordinate-space consistency, missing/foreign/ambiguous groups and immutable inputs");
}
