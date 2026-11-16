/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "mod_menu.h"
#include "battle_hud.h"
#include "hud_themes.h"
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
static unsigned char pack_flags[MAX_PACKS];
static int count,selected,textures_changed,pack_changed,sprites_installed,sprites_enabled,sprites_changed;
static int valid(const char*s){
    size_t n=strlen(s);if(!n||n>=64)return 0;
    for(size_t i=0;i<n;i++)if(!isalnum((unsigned char)s[i])&&s[i]!='-'&&s[i]!='_')return 0;
    return 1;
}
static int compare(const void*a,const void*b){return strcmp(a,b);}
/* Scan only when opening Mods, never during gameplay. Bound recursion and
 * path size; ignore empty directories and unrelated non-PNG files. */
static int contains_png(const char *path,int depth){
    if(depth>8)return 0;
    DIR*d=opendir(path);if(!d)return 0;int found=0;struct dirent*e;
    while(!found&&(e=readdir(d))){
        if(e->d_name[0]=='.')continue;
        char child[1024];int n=snprintf(child,sizeof child,"%s/%s",path,e->d_name);
        if(n<0||(size_t)n>=sizeof child)continue;
        SDL_PathInfo info;if(!SDL_GetPathInfo(child,&info))continue;
        size_t len=strlen(e->d_name);
        if(info.type==SDL_PATHTYPE_DIRECTORY)found=contains_png(child,depth+1);
        else if(info.type==SDL_PATHTYPE_FILE&&len>4){const char*x=e->d_name+len-4;found=x[0]=='.'&&tolower((unsigned char)x[1])=='p'&&tolower((unsigned char)x[2])=='n'&&tolower((unsigned char)x[3])=='g';}
    }
    closedir(d);return found;
}
static int classify(const char*root){
    static const char*paths[]={"sheets/Chara","frames/Chara","sheets/Map","screens/Map"};
    int flags=0;for(int i=0;i<4;i++){char p[512];snprintf(p,sizeof p,"%s/%s",root,paths[i]);if(contains_png(p,0))flags|=i<2?MOD_PACK_CHARACTERS:MOD_PACK_BACKGROUNDS;}
    return flags;
}
void egg_ps5_mod_menu_refresh(void){
    ctps5_hud_themes_refresh();
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
    for(int i=0;i<count;i++){
        char root[256];
        if(i)snprintf(root,sizeof root,"/data/eggoftime/mods/packs/%s/graphics",packs[i]);
        else snprintf(root,sizeof root,"/data/eggoftime/mods/graphics");
        pack_flags[i]=(unsigned char)classify(root);
        if(!strcmp(packs[i],egg_ps5_config.active_pack))selected=i;
    }
}
int egg_ps5_mod_menu_count(void){return 10;}
const char *egg_ps5_mod_menu_title(int row){
    static const char*names[]={"Portrait battle HUD","HUD size","Symbol size","Texture mods","Texture pack","Sprite smoothing","Marle redraw","Detailed pose test","Crono redraw","HUD theme"};return row>=0&&row<10?names[row]:"";
}
const char *egg_ps5_mod_menu_value(int row){
    if(row==9)return ctps5_hud_theme_name();
    if(row==0)return !ctps5_battle_hud_available()?"Unavailable":ctps5_battle_hud_enabled()?"On":"Off";
    if(row==1||row==2){static char value[16];snprintf(value,sizeof value,"%d%%",ctps5_battle_hud_scale(row==2));return value;}
    if(row==3)return display_setting_get(SET_MODS)?"On":"Off";
    if(row==5){
        if(!sprites_installed)return "Not installed";
        if(!display_setting_get(SET_MODS))return "Textures Off";
        if(sprites_changed)return sprites_enabled?"On; restart":"Off; restart";
        return sprite_hd_loaded()?"On":sprites_enabled?"Not loaded":"Off";
    }
    if(row==6)return !ct_redraw_module_count(CT_REDRAW_MARLE)?"Not installed":!display_setting_get(SET_MODS)?"Textures Off":ct_redraw_enabled()?"On (fear poses)":"Off";
    if(row==7)return ct_redraw_count()?"Open pilot":"Not installed";
    if(row==8){static char value[32];int n=ct_redraw_module_count(CT_REDRAW_CRONO);if(!n)return "Not installed";if(!display_setting_get(SET_MODS))return "Textures Off";if(!ct_redraw_module_enabled(CT_REDRAW_CRONO))return "Off";snprintf(value,sizeof value,"On (%d poses)",n);return value;}
    return row==4?packs[selected]:"";
}
const char *egg_ps5_mod_menu_description(int row){
    if(row==9)return "Left/Right: theme. A: reload PNGs and JSON.";
    if(row==0)return "Menu portraits and action icons. Applies now.";
    if(row==1)return "Portrait HUD size. Applies now.";
    if(row==2)return "Transparent action symbols. Applies now.";
    if(row==3)return textures_changed?"Restart CTPS5 to fully apply texture changes.":"Texture replacements. Restart after changes.";
    if(row==4)return pack_changed?"Pack saved. Restart CTPS5 to load it.":"Installed packs. Restart to load your choice.";
    if(row==5)return sprites_changed?"Saved. Restart CTPS5 to apply sprite changes.":"Battle-sprite smoothing. Restart after changes.";
    if(row==6)return "Marle fear poses. Other poses use original art.";
    if(row==7)return "Compare installed redraws with original poses.";
    if(row==8)return "Crono redraws apply now; other poses original.";
    return "";
}
void egg_ps5_mod_menu_change(int row,int direction){
    if(row==9){if(!ctps5_hud_theme_change(direction))display_message("Theme could not load; check eggoftime.log.",3000);}
    else if(row==0){
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
        egg_ps5_mod_pack_select(next);
    }else if(row==5&&sprites_installed){
        if(!egg_ps5_ini_set("/data/eggoftime/mods/battle-sprites/sprites.ini","enabled",sprites_enabled?"0":"1")){
            display_message("Sprite setting could not be saved.",3000);return;
        }
        sprites_enabled=!sprites_enabled;sprites_changed=1;
    }else if(row==6&&ct_redraw_module_count(CT_REDRAW_MARLE)){
        int next=!ct_redraw_enabled();
        if(!egg_ps5_ini_set("/data/eggoftime/mods/marle-pilot/pilot.ini","gameplay",next?"1":"0")){
            display_message("Redraw setting could not be saved.",3000);return;
        }ct_redraw_set_enabled(next);
    }else if(row==7){ct_redraw_viewer_open();}
    else if(row==8&&ct_redraw_module_count(CT_REDRAW_CRONO)){
        int next=!ct_redraw_module_enabled(CT_REDRAW_CRONO);
        if(!egg_ps5_ini_set("/data/eggoftime/mods/crono-hd/pack.ini","gameplay",next?"1":"0")){display_message("Crono setting could not be saved.",3000);return;}
        ct_redraw_module_set_enabled(CT_REDRAW_CRONO,next);
    }
}

int egg_ps5_mod_pack_count(void){return count;}
const char *egg_ps5_mod_pack_name(int pack){return pack>=0&&pack<count?pack?packs[pack]:"Default folder":"";}
int egg_ps5_mod_pack_flags(int pack){return pack>=0&&pack<count?pack_flags[pack]:0;}
int egg_ps5_mod_pack_selected(int pack){return pack>=0&&pack<count&&pack==selected;}
void egg_ps5_mod_pack_select(int pack){
    if(pack<0||pack>=count||pack==selected)return;
    if(!egg_ps5_ini_set("/data/eggoftime/ps5.ini","active_pack",packs[pack])){
        display_message("Texture pack choice could not be saved.",3000);return;
    }
    selected=pack;snprintf(egg_ps5_config.active_pack,sizeof egg_ps5_config.active_pack,"%s",packs[selected]);pack_changed=1;
}
int egg_ps5_mod_option_available(int setting){
    return setting==5?sprites_installed:setting==6?ct_redraw_module_count(CT_REDRAW_MARLE)>0:setting==7?ct_redraw_count()>0:setting==8?ct_redraw_module_count(CT_REDRAW_CRONO)>0:1;
}
