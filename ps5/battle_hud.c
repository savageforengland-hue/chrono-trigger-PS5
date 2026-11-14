/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "battle_hud.h"
#include "../runtime/battle_hud_state.h"
#include "../runtime/battle_hud_faces.h"
#include "../runtime/rt.h"
#include "../runtime/dispatch.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int enabled,visible;
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
    enabled=visible=0;
    if(!requested())return;
    if(strcmp(rt_rom.code,"YQUE")){rt_log("portrait HUD: disabled, supported USA ROM required");return;}
    if(!ct_hud_load_faces(portraits)){rt_log("portrait HUD: disabled, menu portraits unavailable");return;}
    lock=SDL_CreateMutex();if(!lock){rt_log("portrait HUD: disabled, snapshot mutex failed");return;}
    enabled=1;rt_log("portrait HUD: enabled (menu faces, live HP/MP/ATB, docked layout)");
}
void ctps5_battle_hud_frame(void){
    if(!enabled)return;
    CtHudState s={0};
    int on=rt_overlay_resident(0)&&rt_overlay_resident(3)&&ct_hud_read_state(&s,rt_main_ram,RT_MAIN_RAM_SIZE);
    SDL_LockMutex(lock);published=s;visible=on;SDL_UnlockMutex(lock);
}
int ctps5_battle_hud_prepare(SDL_Renderer*ren,SDL_Texture**texture,SDL_FRect*game,SDL_FRect*hud){
    if(!enabled)return 0;
    CtHudState s;SDL_LockMutex(lock);int on=visible;s=published;SDL_UnlockMutex(lock);
    if(!on||game->w<=0||game->h<=0)return 0;
    if(!*texture){
        *texture=SDL_CreateTexture(ren,SDL_PIXELFORMAT_ABGR8888,SDL_TEXTUREACCESS_STREAMING,CT_HUD_W,CT_HUD_H);
        if(!*texture)return 0;
        SDL_SetTextureScaleMode(*texture,SDL_SCALEMODE_NEAREST);
    }
    uint32_t pixels[CT_HUD_W*CT_HUD_H];ct_hud_draw(pixels,&s,portraits);
    if(!SDL_UpdateTexture(*texture,NULL,pixels,CT_HUD_W*4))return 0;
    float old_w=game->w,old_h=game->h,extra=old_w*CT_HUD_H/CT_HUD_W;
    float scale=old_h/(old_h+extra);
    game->w=old_w*scale;game->h=old_h*scale;game->x+=(old_w-game->w)/2;
    *hud=(SDL_FRect){game->x,game->y+game->h,game->w,extra*scale};return 1;
}
