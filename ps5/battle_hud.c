/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "battle_hud.h"
#include "../runtime/battle_hud_state.h"
#include "../runtime/battle_hud_faces.h"
#include "../runtime/rt.h"
#include "../runtime/dispatch.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int enabled,visible,header_on,panel_row,published_row;
static uint64_t header_frame,digit_frame;
static int slot(uint32_t entry){
    uint32_t secondary=rt_r32(0x0207088c),base=secondary+0xaf64;
    if(secondary<0x02000008||secondary>0x023f0000||entry<base||entry>=base+41*0x9c)return -1;
    return (entry-base)%0x9c?-1:(int)((entry-base)/0x9c);
}
void ctps5_battle_hud_entry(uint32_t entry){
    if(!enabled||slot(entry)!=20)return;
    header_on=(rt_r16(entry+0x22)&1)!=0;header_frame=rt_frame_count;
}
void ctps5_battle_hud_cell(uint32_t entry,int ds_row){
    if(!enabled||slot(entry)!=0)return;
    int top=ds_row-9;
    if(top<0||top+44>192)return;
    panel_row=top;digit_frame=rt_frame_count;
}
static SDL_Mutex *lock;
static CtHudState published;
static uint32_t portraits[7][48*48];
static int requested(void){
    FILE*f=fopen("/data/eggoftime/mods/battle-hud/hud.ini","r");if(!f)return 0;
    char line[192];int on=0;while(fgets(line,sizeof line,f)){
        int value;char extra;
        /* One dedicated switch; absent or malformed config retains the stock UI. */
        if(sscanf(line," enabled = %d %c",&value,&extra)==1)on=value==1;
    }fclose(f);return on;
}

void ctps5_battle_hud_init(void){
    enabled=visible=header_on=0;header_frame=digit_frame=UINT64_MAX;
    if(!requested())return;
    if(strcmp(rt_rom.code,"YQUE")){rt_log("portrait HUD: disabled, supported USA ROM required");return;}
    if(!ct_hud_load_faces(portraits)){rt_log("portrait HUD: disabled, menu portraits unavailable");return;}
    lock=SDL_CreateMutex();if(!lock){rt_log("portrait HUD: disabled, snapshot mutex failed");return;}
    enabled=1;rt_log("portrait HUD: enabled (menu faces, live HP/MP/ATB, replacement panel)");
}
void ctps5_battle_hud_frame(void){
    if(!enabled)return;
    CtHudState s={0};
    int on=header_on&&header_frame<=rt_frame_count&&rt_frame_count-header_frame<=2&&
        digit_frame<=rt_frame_count&&rt_frame_count-digit_frame<=2&&rt_overlay_resident(0)&&rt_overlay_resident(3)&&ct_hud_read_state(&s,rt_main_ram,RT_MAIN_RAM_SIZE);
    SDL_LockMutex(lock);published=s;published_row=panel_row;visible=on;SDL_UnlockMutex(lock);
}
int ctps5_battle_hud_prepare(SDL_Renderer*ren,SDL_Texture**texture,const SDL_FRect*game,
    int columns,int rows,int zoom_ui,SDL_FRect*cover,SDL_FRect*image){
    if(!enabled)return 0;
    CtHudState s;SDL_LockMutex(lock);int on=visible,y=published_row;s=published;SDL_UnlockMutex(lock);
    if(!on||game->w<=0||game->h<=0||columns<256||rows<192)return 0;
    if(!*texture){
        *texture=SDL_CreateTexture(ren,SDL_PIXELFORMAT_ABGR8888,SDL_TEXTUREACCESS_STREAMING,CT_HUD_W,CT_HUD_H);
        if(!*texture)return 0;
        SDL_SetTextureScaleMode(*texture,SDL_SCALEMODE_NEAREST);
    }
    uint32_t pixels[CT_HUD_W*CT_HUD_H];ct_hud_draw(pixels,&s,portraits);
    if(!SDL_UpdateTexture(*texture,NULL,pixels,CT_HUD_W*4))return 0;
    float ui_scale=zoom_ui==1?(float)rows/192:1;
    float x=columns/2.0f+(152-128)*ui_scale;
    float top=zoom_ui==1?y*ui_scale:y+(zoom_ui==2&&y>=96?rows-192:0);
    *cover=(SDL_FRect){game->x+x*game->w/columns,game->y+top*game->h/rows,
        (columns-x)*game->w/columns,44*ui_scale*game->h/rows};
    float k=cover->w/CT_HUD_W;
    if(k>cover->h/CT_HUD_H)k=cover->h/CT_HUD_H;
    *image=(SDL_FRect){cover->x+(cover->w-CT_HUD_W*k)/2,
        cover->y+(cover->h-CT_HUD_H*k)/2,CT_HUD_W*k,CT_HUD_H*k};
    return 1;
}
