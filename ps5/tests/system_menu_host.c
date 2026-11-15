/* SPDX-License-Identifier: GPL-3.0-or-later
 * Actual system-menu state machine and drawing with authored host dependencies. */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "runtime/system_menu.c"
#include "runtime/menu_chord.h"
static Uint64 ticks;
static int scene_place,mod_change,sets,capture,pilot_page,pilot_pose;
static int dump_notice,dump_images,dump_errors,dump_repair;static char last_message[256];
const char*rt_data_dir(void){return "/data/eggoftime/";}
int ct_redraw_viewer_active(void){return pilot_page;}
int ct_redraw_viewer_open(void){pilot_page=1;pilot_pose=0;return 1;}
void ct_redraw_viewer_close(void){pilot_page=0;}
void ct_redraw_viewer_step(int d){pilot_pose+=d;}
const char*ct_redraw_viewer_title(void){return "Marle cell 089 - art pilot";}
volatile int rt_state_request;
Uint64 SDL_GetTicks(void){return ticks;}
int SDL_GetAtomicInt(SDL_AtomicInt*a){return a->value;}
int SDL_SetAtomicInt(SDL_AtomicInt*a,int v){int old=a->value;a->value=v;return old;}
size_t SDL_strlen(const char*s){return strlen(s);}
size_t SDL_strlcpy(char*d,const char*s,size_t n){size_t len=strlen(s);if(n){size_t k=len<n-1?len:n-1;memcpy(d,s,k);d[k]=0;}return len;}
size_t SDL_strlcat(char*d,const char*s,size_t n){size_t len=strlen(d);return len+SDL_strlcpy(d+len,s,len<n?n-len:0);}
int SDL_snprintf(char*d,size_t n,const char*f,...){va_list a;va_start(a,f);int r=vsnprintf(d,n,f,a);va_end(a);return r;}
int ss_menu_place(void){return scene_place;}
int display_language(void){return 1;}
int rt_language(void){return 1;}
void display_message(const char*s,unsigned ms){snprintf(last_message,sizeof last_message,"%s",s);(void)ms;}
void display_text_input(int on){(void)on;}
int display_setting_shown(int i){return i==SET_ASPECT||i==SET_FRAMERATE;}
int display_setting_count(int i){(void)i;return 2;}
int display_setting_get(int i){(void)i;return 0;}
void display_setting_set(int i,int v){(void)i;(void)v;sets++;}
const char *display_setting_title(int i){return i==SET_ASPECT?"Aspect ratio":"Framerate";}
const char *display_setting_label(int i,int v){(void)v;return i==SET_ASPECT?"16:9":"60 FPS";}
const char *display_shader_path(void){return NULL;}
void controls_capture_cancel(void){capture=0;}
void controls_capture_start(int a,int p){(void)a;(void)p;capture=1;}
int controls_capturing(void){return capture;}
const char *controls_capture_conflict(void){return NULL;}
void controls_reset(void){}
const char *controls_name(int i){(void)i;return "Confirm";}
const char *controls_pad_label(int i){(void)i;return i==CTL_B?"\xC3\x97":"\xE2\x97\x8B";}
int gfxdump_repairing(void){return dump_repair;}
int gfxdump_errors(void){return dump_errors;}
const char*gfxdump_error(void){return "";}
int gfxdump_finished(int*i){if(!dump_notice)return 0;dump_notice=0;*i=dump_images;return 1;}
void gfxdump_start(void){}
int gfxdump_running(void){return 0;}
void gfxdump_progress(int*d,int*t,float*s){*d=*t=0;*s=0;}
int cheevos_state(void){return CHEEVOS_LOGGED_OUT;}
const char *cheevos_user(void){return "";}
void cheevos_login(const char*u,const char*p){(void)u;(void)p;}
void cheevos_logout(void){}
int cheevos_list(const CheevosItem**i){*i=NULL;return 0;}
const uint32_t *cheevos_badge(const char*u){(void)u;return NULL;}
const char *ui_text(const char*s){return s;}
void egg_ps5_mod_menu_refresh(void){}
int egg_ps5_mod_menu_count(void){return 8;}
const char *egg_ps5_mod_menu_title(int i){const char*n[]={"Portrait battle HUD","HUD size","Symbol size","Texture mods","Texture pack","Sprite smoothing","Marle redraw","Detailed pose test"};return n[i];}
const char *egg_ps5_mod_menu_value(int i){return i==4?"default":i==1?"100%":i==2?"70%":"On";}
const char *egg_ps5_mod_menu_description(int i){(void)i;return "Menu portraits and action icons. Applies now.";}
void egg_ps5_mod_menu_change(int i,int d){mod_change=i+1;if(i==7)ct_redraw_viewer_open();(void)d;}
static void step(unsigned keys){ticks+=120;sysmenu_input(keys);}
static void tap(unsigned keys){step(keys);step(0);}
static void shot(const char*directory,const char*name){
 if(!directory)return;char path[1024];snprintf(path,sizeof path,"%s/%s.ppm",directory,name);
 uint32_t p[256*192];sysmenu_draw(p);FILE*f=fopen(path,"wb");assert(f);fprintf(f,"P6\n256 192\n255\n");
 for(int i=0;i<256*192;i++){fputc(p[i]&255,f);fputc((p[i]>>8)&255,f);fputc((p[i]>>16)&255,f);}fclose(f);
}
int main(int argc,char**argv){
 CtMenuChord chord={0};assert(!ct_menu_chord_update(&chord,1,0));assert(!ct_menu_chord_update(&chord,0,1));
 assert(ct_menu_chord_update(&chord,1,1));assert(!ct_menu_chord_update(&chord,1,1));
 assert(!ct_menu_chord_update(&chord,1,0));assert(!ct_menu_chord_update(&chord,1,1));
 assert(!ct_menu_chord_update(&chord,0,0));assert(ct_menu_chord_update(&chord,1,1));
 scene_place=SS_MENU_NONE;sysmenu_host_toggle();step(KEY_A);assert(sysmenu_is_open()&&sysmenu_pauses_game());
 step(0);assert(hub_page&&hub_cursor==0);shot(argc>1?argv[1]:NULL,"settings-hub");
 tap(KEY_DOWN);tap(KEY_A);assert(mods_page&&!hub_page);shot(argc>1?argv[1]:NULL,"mods-menu");
 tap(KEY_RIGHT);assert(mod_change==1);tap(KEY_DOWN);tap(KEY_A);assert(mod_change==2);
 for(int i=0;i<6;i++)tap(KEY_DOWN);tap(KEY_A);assert(pilot_page&&sysmenu_pauses_game());
 tap(KEY_RIGHT);assert(pilot_pose==1);tap(KEY_B);assert(!pilot_page&&mods_page);
 for(int i=0;i<5;i++)tap(KEY_UP);
 tap(KEY_B);assert(hub_page);tap(KEY_UP);tap(KEY_A);assert(!hub_page&&!mods_page);tap(KEY_RIGHT);assert(sets==1);
 tap(KEY_B);assert(hub_page);tap(KEY_DOWN);tap(KEY_DOWN);tap(KEY_A);assert(controls_page);
 tap(KEY_A);assert(capture);sysmenu_host_toggle();step(KEY_A);assert(!sysmenu_is_open()&&!sysmenu_pauses_game()&&!capture);
 assert(sysmenu_input(KEY_A)==0);step(0);assert(sysmenu_input(KEY_A)==KEY_A); /* closing input cannot leak */
 scene_place=SS_MENU_PAUSE;tap(KEY_SELECT);assert(sysmenu_is_open()&&!sysmenu_pauses_game());tap(KEY_B);assert(!sysmenu_is_open());
 dump_notice=1;dump_images=30577;dump_repair=1;sysmenu_input(0);assert(strstr(last_message,"FTP access repaired")&&strstr(last_message,"30577")&&strstr(last_message,"/data/eggoftime/mods/graphics/dump/"));
 dump_notice=1;dump_images=0;dump_errors=1;dump_repair=0;sysmenu_input(0);assert(strstr(last_message,"incomplete")&&strstr(last_message,"graphics-dump.log"));
 puts("PASS: L3+R3 one-shot/release latch, open from active gameplay, host pause state, settings/mods/controls navigation, live mod actions, capture cancellation, close/release input guard, legacy Select menu");
}
