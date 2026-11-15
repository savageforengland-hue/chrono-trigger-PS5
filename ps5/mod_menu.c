/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "mod_menu.h"
#include "battle_hud.h"
#include "config.h"
#include "config_file.h"
#include "../runtime/display.h"
#include "../runtime/redraw.h"
#include "../runtime/sprite_hd.h"
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#define MAX_PACKS 64
static char packs[MAX_PACKS][64];
static int count,selected,textures_changed,pack_changed,sprites_installed,sprites_enabled,sprites_changed;
static int valid(const char*s){
    size_t n=strlen(s);if(!n||n>=64)return 0;
    for(size_t i=0;i<n;i++)if(!isalnum((unsigned char)s[i])&&s[i]!='-'&&s[i]!='_')return 0;
    return 1;
}
static int compare(const void*a,const void*b){return strcmp(a,b);}
void egg_ps5_mod_menu_refresh(void){
    sprites_installed=sprites_enabled=0;
    FILE*f=fopen("/data/eggoftime/mods/battle-sprites/sprites.ini","r");
    if(f){sprites_installed=1;char line[192];while(fgets(line,sizeof line,f)){int v;char extra;
        if(sscanf(line," enabled = %d %c",&v,&extra)==1)sprites_enabled=v==1;
    }fclose(f);}
    strcpy(packs[0],"default");count=1;selected=0;
    DIR*d=opendir("/data/eggoftime/mods/packs");
    if(d){struct dirent*e;while((e=readdir(d))&&count<MAX_PACKS){
        if(!valid(e->d_name)||!strcmp(e->d_name,"default"))continue;
        char p[256];snprintf(p,sizeof p,"/data/eggoftime/mods/packs/%.63s/graphics",e->d_name);
        SDL_PathInfo info;if(!SDL_GetPathInfo(p,&info)||info.type!=SDL_PATHTYPE_DIRECTORY)continue;
        snprintf(packs[count++],64,"%.63s",e->d_name);
    }closedir(d);}
    qsort(packs+1,(size_t)(count-1),64,compare);
    for(int i=0;i<count;i++)if(!strcmp(packs[i],egg_ps5_config.active_pack))selected=i;
}
int egg_ps5_mod_menu_count(void){return 8;}
const char *egg_ps5_mod_menu_title(int row){
    static const char*names[]={"Portrait battle HUD","HUD size","Symbol size","Texture mods","Texture pack","Sprite smoothing","Marle redraw","Detailed pose test"};return row>=0&&row<8?names[row]:"";
}
const char *egg_ps5_mod_menu_value(int row){
    if(row==0)return !ctps5_battle_hud_available()?"Unavailable":ctps5_battle_hud_enabled()?"On":"Off";
    if(row==1||row==2){static char value[16];snprintf(value,sizeof value,"%d%%",ctps5_battle_hud_scale(row==2));return value;}
    if(row==3)return display_setting_get(SET_MODS)?"On":"Off";
    if(row==5){
        if(!sprites_installed)return "Not installed";
        if(!display_setting_get(SET_MODS))return "Textures Off";
        if(sprites_changed)return sprites_enabled?"On; restart":"Off; restart";
        return sprite_hd_loaded()?"On":sprites_enabled?"Not loaded":"Off";
    }
    if(row==6)return !ct_redraw_count()?"Not installed":!display_setting_get(SET_MODS)?"Textures Off":ct_redraw_enabled()?"On (fear poses)":"Off";
    if(row==7)return ct_redraw_count()?"Open pilot":"Not installed";
    return row==4?packs[selected]:"";
}
const char *egg_ps5_mod_menu_description(int row){
    if(row==0)return "Menu portraits and action icons. Applies now.";
    if(row==1)return "Portrait HUD size. Applies now.";
    if(row==2)return "Transparent action symbols. Applies now.";
    if(row==3)return textures_changed?"Restart CTPS5 to fully apply texture changes.":"Texture replacements. Restart after changes.";
    if(row==4)return pack_changed?"Pack saved. Restart CTPS5 to load it.":"Installed packs. Restart to load your choice.";
    if(row==5)return sprites_changed?"Saved. Restart CTPS5 to apply sprite changes.":"Smoothing module only; separate from Marle art.";
    if(row==6)return "Live fear art. Waiting/hit use original poses.";
    if(row==7)return "Original/detailed poses; crossbow review only.";
    return "";
}
void egg_ps5_mod_menu_change(int row,int direction){
    if(row==0){
        if(!ctps5_battle_hud_set_enabled(!ctps5_battle_hud_enabled()))display_message("HUD setting could not be saved.",3000);
    }else if(row==1||row==2){
        int commands=row==2,lo=commands?50:75,hi=commands?100:125,step=commands?10:5;
        int v=ctps5_battle_hud_scale(commands)+(direction<0?-step:step);
        if(v<lo)v=lo;
        if(v>hi)v=hi;
        if(!ctps5_battle_hud_set_scale(commands,v))display_message("HUD size could not be saved.",3000);
    }else if(row==3){
        display_setting_set(SET_MODS,!display_setting_get(SET_MODS));textures_changed=1;
        ct_redraw_set_visible(display_setting_get(SET_MODS));
    }else if(row==4&&count>0){
        int next=(selected+count+(direction<0?-1:1))%count;
        if(!egg_ps5_ini_set("/data/eggoftime/ps5.ini","active_pack",packs[next])){
            display_message("Texture pack choice could not be saved.",3000);return;
        }
        selected=next;snprintf(egg_ps5_config.active_pack,sizeof egg_ps5_config.active_pack,"%s",packs[selected]);pack_changed=1;
    }else if(row==5&&sprites_installed){
        if(!egg_ps5_ini_set("/data/eggoftime/mods/battle-sprites/sprites.ini","enabled",sprites_enabled?"0":"1")){
            display_message("Sprite setting could not be saved.",3000);return;
        }
        sprites_enabled=!sprites_enabled;sprites_changed=1;
    }else if(row==6&&ct_redraw_count()){
        int next=!ct_redraw_enabled();
        if(!egg_ps5_ini_set("/data/eggoftime/mods/marle-pilot/pilot.ini","gameplay",next?"1":"0")){
            display_message("Redraw setting could not be saved.",3000);return;
        }ct_redraw_set_enabled(next);
    }else if(row==7){ct_redraw_viewer_open();}
}
