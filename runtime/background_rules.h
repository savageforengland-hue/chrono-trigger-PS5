/* SPDX-License-Identifier: GPL-3.0-or-later
 * Optional HD background restrictions. Plain dumped PNGs remain unrestricted. */
#pragma once
#include "mod_json.h"
#define BG_RULE_LOCATIONS 16
#define BG_RULE_BYTES 32768
/* Reference light is the weighted sum of colours 1..15 in a native 16-colour
 * row: sum(5*r5 + 9*g5 + 2*b5). Zero disables palette-relative lighting. */
typedef struct BgLocation { int id; int light[16]; } BgLocation;
typedef struct BgWindow {int enabled,location,bg,cell[2],control,open,closed,brightness,frames;} BgWindow;
typedef struct BgRules { int count,characters,sprite_blend,background_blend; BgLocation locations[BG_RULE_LOCATIONS]; BgWindow window; } BgRules;
static inline int bg_rules_parse(const char*json,size_t n,BgRules*out){
 if(!out||n>BG_RULE_BYTES)return 0;BgRules next={0};MjReader r;if(!mj_open(&r,json,n))return 0;
 int ok=0,schema;int a=mj_field(&r,0,"locations");
 if(!mj_integer(&r,mj_field(&r,0,"schema"),1,1,&schema)||!mj_equal(&r,mj_field(&r,0,"transparentTiles")<0?0:mj_field(&r,0,"transparentTiles"),"original")||a<0||r.t[a].type!='[')goto done;
 for(int k=a+1;k<r.count&&r.t[k].start<r.t[a].end;k=mj_next(&r,k)){
  if(r.t[k].type!='{'||next.count==BG_RULE_LOCATIONS)goto done;BgLocation*l=next.locations+next.count;
  if(!mj_integer(&r,mj_field(&r,k,"id"),0,65535,&l->id))goto done;
  for(int i=0;i<next.count;i++)if(next.locations[i].id==l->id)goto done;
  int p=mj_field(&r,k,"paletteLight");if(p!=-1&&!mj_array_int(&r,p,l->light,16,0,7440))goto done;
  next.count++;
 }
 if(!next.count||!mj_boolean(&r,mj_field(&r,0,"includeCharacterSheets"),&next.characters))goto done;
 int blend=mj_field(&r,0,"spriteBlendWeight");if(blend!=-1&&!mj_integer(&r,blend,1,127,&next.sprite_blend))goto done;
 int bgblend=mj_field(&r,0,"backgroundBlendWeight");if(bgblend!=-1&&!mj_integer(&r,bgblend,1,127,&next.background_blend))goto done;
 int window=mj_field(&r,0,"windowLight");if(window!=-1){
  BgWindow*w=&next.window;if(window<0||r.t[window].type!='{'||!mj_integer(&r,mj_field(&r,window,"location"),0,65535,&w->location)||!mj_integer(&r,mj_field(&r,window,"bg"),0,3,&w->bg)||!mj_array_int(&r,mj_field(&r,window,"mapCell"),w->cell,2,0,63)||!mj_integer(&r,mj_field(&r,window,"bgControl"),0,65535,&w->control)||!mj_integer(&r,mj_field(&r,window,"openTile"),0,1023,&w->open)||!mj_integer(&r,mj_field(&r,window,"closedTile"),0,1023,&w->closed)||w->open==w->closed||!mj_integer(&r,mj_field(&r,window,"closedBrightness"),0,100,&w->brightness)||!mj_integer(&r,mj_field(&r,window,"fadeFrames"),0,120,&w->frames))goto done;
  int listed=0;for(int i=0;i<next.count;i++)listed|=next.locations[i].id==w->location;if(!listed)goto done;w->enabled=1;
 }
 *out=next;ok=1;
done:mj_close(&r);return ok;
}
static inline int bg_rules_index(const BgRules*r,int location){for(int i=0;i<r->count;i++)if(r->locations[i].id==location)return i;return -1;}
