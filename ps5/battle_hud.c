/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "battle_hud.h"
#include "config_file.h"
#include <sys/stat.h>
#include "../runtime/battle_hud_state.h"
#include "../runtime/battle_hud_faces.h"
#include "../runtime/rt.h"
#include "../runtime/dispatch.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static SDL_AtomicInt enabled,hud_scale,command_scale;
static int available,visible,header_on,panel_row,published_row,cursor_column,cursor_row;
static uint64_t cursor_frame;
static int on(void){return SDL_GetAtomicInt(&enabled);}
static uint64_t header_frame,digit_frame;
static int slot(uint32_t entry){
    uint32_t secondary=rt_r32(0x0207088c),base=secondary+0xaf64;
    if(secondary<0x02000008||secondary>0x023f0000||entry<base||entry>=base+41*0x9c)return -1;
    return (entry-base)%0x9c?-1:(int)((entry-base)/0x9c);
}
int ctps5_battle_hud_stock_entry(uint32_t entry,int world,int actor){
    if(!available||world||actor||strcmp(rt_rom.code,"YQUE"))return 0;
    int id=slot(entry);
    /* Digits and window pieces belong to separate screen-space lists.
     * The caller supplies original actor/world call context, never palette
     * guesses: actors can also be drawn from managed UI entries. */
    return id==22?cursor_column>=0:entry>=0x02000000&&entry<=0x023fff64;
}
void ctps5_battle_hud_entry(uint32_t entry){
    if(!on())return;
    int id=slot(entry);
    if(id==22)cursor_frame=UINT64_MAX;
    if(id!=20)return;
    header_on=(rt_r16(entry+0x22)&1)!=0;header_frame=rt_frame_count;
}
void ctps5_battle_hud_cell(uint32_t entry,int ds_column,int ds_row){
    if(!on())return;
    int id=slot(entry);
    if(id==22){cursor_column=ds_column;cursor_row=ds_row;cursor_frame=rt_frame_count;return;}
    if(id!=0)return;
    int top=ds_row-9;
    if(top<0||top+46>192)return;
    panel_row=top;digit_frame=rt_frame_count;
}
static SDL_Mutex *lock;
static CtHudState published;
static uint32_t portraits[7][48*48],icons[3][32*32],window_skin[80*32];
static int has_window_skin;
static int requested(void){
    FILE*f=fopen("/data/eggoftime/mods/battle-hud/hud.ini","r");if(!f)return 0;
    char line[192];int on=0;while(fgets(line,sizeof line,f)){
        int value;char extra;
        /* One dedicated switch; absent or malformed config retains the stock UI. */
        if(sscanf(line," enabled = %d %c",&value,&extra)==1)on=value==1;
        else if(sscanf(line," scale_percent = %d %c",&value,&extra)==1&&value>=75&&value<=125)SDL_SetAtomicInt(&hud_scale,value);
        else if(sscanf(line," command_scale_percent = %d %c",&value,&extra)==1&&value>=50&&value<=100)SDL_SetAtomicInt(&command_scale,value);
    }fclose(f);return on;
}

void ctps5_battle_hud_init(void){
    SDL_SetAtomicInt(&hud_scale,100);SDL_SetAtomicInt(&command_scale,70);
    SDL_SetAtomicInt(&enabled,0);available=visible=header_on=0;
    header_frame=digit_frame=cursor_frame=UINT64_MAX;
    if(strcmp(rt_rom.code,"YQUE"))return;
    /* Load before game threads start, so the Mods page can enable it live. */
    if(!ct_hud_load_faces(portraits)||!ct_hud_load_command_icons(icons)){
        rt_log("portrait HUD: unavailable, original menu art could not be decoded");return;
    }
    has_window_skin=ct_hud_load_window_skin(window_skin);
    rt_log("portrait HUD: %s window styling",has_window_skin?"original battle":"soft fallback");
    lock=SDL_CreateMutex();if(!lock)return;
    available=1;SDL_SetAtomicInt(&enabled,requested());
    rt_log("portrait HUD: %s (centered, original menu icons, live HP/MP/ATB)",on()?"enabled":"off");
}
int ctps5_battle_hud_available(void){return available;}
int ctps5_battle_hud_enabled(void){return on();}
int ctps5_battle_hud_set_enabled(int value){
    if(!available)return 0;
    mkdir("/data/eggoftime/mods/battle-hud",0777);
    if(!egg_ps5_ini_set("/data/eggoftime/mods/battle-hud/hud.ini","enabled",value?"1":"0"))return 0;
    SDL_SetAtomicInt(&enabled,value!=0);
    if(!value){SDL_LockMutex(lock);visible=0;SDL_UnlockMutex(lock);}
    rt_log("portrait HUD: %s via Mods menu",value?"enabled":"off");return 1;
}
int ctps5_battle_hud_scale(int commands){return SDL_GetAtomicInt(commands?&command_scale:&hud_scale);}
int ctps5_battle_hud_set_scale(int commands,int value){
    int lo=commands?50:75,hi=commands?100:125;
    if(!available||value<lo||value>hi)return 0;
    char v[16];snprintf(v,sizeof v,"%d",value);
    mkdir("/data/eggoftime/mods/battle-hud",0777);
    if(!egg_ps5_ini_set("/data/eggoftime/mods/battle-hud/hud.ini",commands?"command_scale_percent":"scale_percent",v))return 0;
    SDL_SetAtomicInt(commands?&command_scale:&hud_scale,value);return 1;
}
void ctps5_battle_hud_frame(void){
    if(!available)return;
    CtHudState s={.selected=-1,.command=-1};
    int show=on()&&header_on&&header_frame<=rt_frame_count&&rt_frame_count-header_frame<=2&&
        digit_frame<=rt_frame_count&&rt_frame_count-digit_frame<=2&&rt_overlay_resident(0)&&rt_overlay_resident(3)&&ct_hud_read_state(&s,rt_main_ram,RT_MAIN_RAM_SIZE);
    if(show&&cursor_frame<=rt_frame_count&&rt_frame_count-cursor_frame<=2){
        int d=cursor_row-panel_row-1;
        if(cursor_column>=0&&cursor_column<144&&d>=0&&d<=24&&d%12==0)s.command=d/12;
    }
    SDL_LockMutex(lock);published=s;published_row=panel_row;visible=show;SDL_UnlockMutex(lock);
}
void ctps5_battle_hud_capture(CtHudSnapshot *s){
    memset(s,0,sizeof *s);if(!available)return;
    SDL_LockMutex(lock);s->state=published;s->row=published_row;s->active=visible;SDL_UnlockMutex(lock);
}
uint32_t ctps5_battle_hud_input(uint32_t keys){
    if(!on()||!available)return keys;
    SDL_LockMutex(lock);int active=visible&&published.command>=0;SDL_UnlockMutex(lock);
    if(!active)return keys;
    uint32_t result=keys&~(KEY_LEFT|KEY_RIGHT|KEY_L|KEY_R);
    if(keys&KEY_LEFT)result|=KEY_UP;
    if(keys&KEY_RIGHT)result|=KEY_DOWN;
    if(keys&KEY_L)result|=KEY_LEFT;
    if(keys&KEY_R)result|=KEY_RIGHT;
    return result;
}
static int upload(SDL_Renderer*ren,SDL_Texture**tex,int w,int h,const uint32_t*p){
    if(!*tex){
        *tex=SDL_CreateTexture(ren,SDL_PIXELFORMAT_ABGR8888,SDL_TEXTUREACCESS_STREAMING,w,h);
        if(!*tex)return 0;
        SDL_SetTextureScaleMode(*tex,SDL_SCALEMODE_NEAREST);
        SDL_SetTextureBlendMode(*tex,w==CT_CMD_W?SDL_BLENDMODE_BLEND:SDL_BLENDMODE_NONE);
    }
    return SDL_UpdateTexture(*tex,NULL,p,w*4);
}
int ctps5_battle_hud_prepare(SDL_Renderer*ren,SDL_Texture**texture,SDL_Texture**commands,
    const SDL_FRect*game,int columns,int rows,int zoom_ui,CtHudLayout*layout,const CtHudSnapshot *snapshot){
    if(!on()||!available)return 0;
    if(!snapshot)return 0;
    CtHudState s=snapshot->state;int show=snapshot->active,y=snapshot->row;
    if(!show||game->w<=0||game->h<=0||columns<256||rows<192)return 0;
    uint32_t pixels[CT_HUD_W*CT_HUD_H];ct_hud_draw(pixels,&s,portraits,has_window_skin?window_skin:NULL);
    if(!upload(ren,texture,CT_HUD_W,CT_HUD_H,pixels))return 0;
    if(s.command>=0){
        uint32_t p[CT_CMD_W*CT_CMD_H];ct_hud_draw_commands(p,s.command,icons);
        if(!upload(ren,commands,CT_CMD_W,CT_CMD_H,p))return 0;
    }
    float ui_scale=zoom_ui==1?(float)rows/192:1;
    float top=zoom_ui==1?y*ui_scale:y+(zoom_ui==2&&y>=96?rows-192:0);
    /* Stock UI is removed before composition, leaving real scenery around
     * the resized card. No opaque cover or previous-frame patch is drawn. */
    float panel_top=game->y+top*game->h/rows;
    float card_height=44*ui_scale*game->h/rows;
    float k=card_height/CT_HUD_H*ctps5_battle_hud_scale(0)/100;
    if(k>game->w*.94f/CT_HUD_W)k=game->w*.94f/CT_HUD_W;
    float hud_y=panel_top+card_height-CT_HUD_H*k;
    if(hud_y<game->y)hud_y=game->y;
    if(hud_y+CT_HUD_H*k>game->y+game->h)hud_y=game->y+game->h-CT_HUD_H*k;
    layout->hud=(SDL_FRect){game->x+(game->w-CT_HUD_W*k)/2,hud_y,CT_HUD_W*k,CT_HUD_H*k};
    float command_scale=k*ctps5_battle_hud_scale(1)/100;
    float cmd_y=hud_y-CT_CMD_H*command_scale-2*k;
    if(cmd_y<game->y)cmd_y=hud_y+layout->hud.h+2*k;
    layout->commands=(SDL_FRect){game->x+(game->w-CT_CMD_W*command_scale)/2,cmd_y,
        s.command>=0?CT_CMD_W*command_scale:0,s.command>=0?CT_CMD_H*command_scale:0};
    return 1;
}
