/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "redraw_match.h"
#include <string.h>
static int same(const RedrawObject*a,const RedrawObject*b){return b->valid&&a->flip==b->flip&&!memcmp(a->bytes,b->bytes,128);}
int redraw_match(const RedrawReference*r,const RedrawObject live[128],int anchor,int found[REDRAW_OBJECTS]){
    if(!r||r->count<1||r->count>REDRAW_OBJECTS||anchor<0||anchor>=128||!same(r->object,live+anchor))return 0;
    int ox=(live[anchor].x-r->object[0].x)&511,oy=(live[anchor].y-r->object[0].y)&255;
    found[0]=anchor;
    for(int k=1;k<r->count;k++){
        int hit=-1;
        for(int j=0;j<128;j++){
            const RedrawObject*b=live+j;
            if(!same(r->object+k,b)||b->palette!=live[anchor].palette||b->priority!=live[anchor].priority||b->mode!=live[anchor].mode||b->space!=live[anchor].space||((b->x-r->object[k].x)&511)!=ox||((b->y-r->object[k].y)&255)!=oy)continue;
            int used=0;for(int p=0;p<k;p++)used|=found[p]==j;
            if(used)continue;
            if(hit>=0)return 0; /* Ambiguous live group: retain original. */
            hit=j;
        }
        if(hit<0)return 0;
        found[k]=hit;
    }return 1;
}
