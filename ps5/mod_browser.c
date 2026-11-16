/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "mod_menu.h"
/* Packs are independent toggles. Categories share the same saved state. */
static int pack_at(int page,int row){
    int mask=page==MOD_CHARACTER_PACKS?MOD_PACK_CHARACTERS:page==MOD_BACKGROUNDS?MOD_PACK_BACKGROUNDS:0;
    for(int i=0;i<egg_ps5_mod_pack_count();i++)if(!i||!mask||(egg_ps5_mod_pack_flags(i)&mask)){if(!row--)return i;}
    return -1;
}
static int setting_at(int page,int row){
    static const int hud[]={0,1,2,9},characters[]={5,8,6,7};
    if(page==MOD_HUD)return row>=0&&row<4?hud[row]:-1;
    if(page==MOD_CHARACTERS){
        if(row--==0)return -1;
        for(int i=0;i<4;i++)if(egg_ps5_mod_option_available(characters[i])){if(!row--)return characters[i];}
        return -1;
    }
    return page>=MOD_BACKGROUNDS&&page!=MOD_HUD&&row==0?3:-1;
}
const char *egg_ps5_mod_page_title(int page){
    static const char*t[]={"Mods","Characters","Backgrounds","Battle HUD","Texture Packs","Character packs"};
    return page>=0&&page<MOD_PAGE_COUNT?t[page]:"Mods";
}
int egg_ps5_mod_page_count(int page){
    if(page==MOD_ROOT||page==MOD_HUD)return 4;
    if(page==MOD_CHARACTERS){int n=1;for(int i=5;i<=8;i++)n+=egg_ps5_mod_option_available(i);return n;}
    int n=1;while(pack_at(page,n-1)>=0)n++;return n;
}
int egg_ps5_mod_page_target(int page,int row){
    if(page==MOD_ROOT&&row>=0&&row<4)return row+1;
    return page==MOD_CHARACTERS&&row==0?MOD_CHARACTER_PACKS:-1;
}
const char *egg_ps5_mod_page_label(int page,int row){
    int target=egg_ps5_mod_page_target(page,row);if(target>=0)return egg_ps5_mod_page_title(target);
    int setting=setting_at(page,row);if(setting>=0)return egg_ps5_mod_menu_title(setting);
    return egg_ps5_mod_pack_name(pack_at(page,row-1));
}
const char *egg_ps5_mod_page_value(int page,int row){
    if(egg_ps5_mod_page_target(page,row)>=0)return "Open >";
    int setting=setting_at(page,row);if(setting>=0)return egg_ps5_mod_menu_value(setting);
    return egg_ps5_mod_pack_selected(pack_at(page,row-1))?"On":"Off";
}
const char *egg_ps5_mod_page_description(int page,int row){
    if(page==MOD_ROOT){static const char*d[]={"Character packs, redraws and sprite options.","Room, scenery and background packs.","Portraits, colours, sizing and HUD themes.","Browse every installed graphics pack."};return row>=0&&row<4?d[row]:"";}
    if(egg_ps5_mod_page_target(page,row)>=0)return "Browse packs containing character PNGs.";
    int setting=setting_at(page,row);if(setting>=0)return egg_ps5_mod_menu_description(setting);
    return "A: On/Off. Restart to apply. Later names win overlaps.";
}
void egg_ps5_mod_page_change(int page,int row,int direction){
    if(egg_ps5_mod_page_target(page,row)>=0)return;
    int setting=setting_at(page,row);
    if(setting>=0)egg_ps5_mod_menu_change(setting,direction);
    else if(!direction)egg_ps5_mod_pack_select(pack_at(page,row-1));
}
