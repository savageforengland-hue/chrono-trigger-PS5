/* SPDX-License-Identifier: GPL-3.0-or-later */
#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include "ps5/config.h"
#include "ps5/config_file.h"
#include "ps5/mod_menu.h"
#include "runtime/display.h"
EggPs5Config egg_ps5_config={2160,60,3,"default"};
static const char*root;static int hud,texture=1,scales[2]={100,70};
static const char *path(const char*p,char out[2048]){
 if(!strncmp(p,"/data/eggoftime",15)){snprintf(out,2048,"%s%s",root,p+15);return out;}return p;
}
FILE *__real_fopen(const char*,const char*);
FILE *__wrap_fopen(const char*p,const char*m){char mapped[2048];return __real_fopen(path(p,mapped),m);}
int __real_rename(const char*,const char*);
int __wrap_rename(const char*a,const char*b){char ma[2048],mb[2048];return __real_rename(path(a,ma),path(b,mb));}
int __real_unlink(const char*);
int __wrap_unlink(const char*p){char mapped[2048];return __real_unlink(path(p,mapped));}
DIR *__real_opendir(const char*);
DIR *__wrap_opendir(const char*p){char mapped[2048];return __real_opendir(path(p,mapped));}
bool SDL_GetPathInfo(const char*p,SDL_PathInfo*i){char mapped[2048];struct stat s;if(stat(path(p,mapped),&s))return false;i->type=S_ISDIR(s.st_mode)?SDL_PATHTYPE_DIRECTORY:SDL_PATHTYPE_FILE;return true;}
int ctps5_battle_hud_enabled(void){return hud;}
int ctps5_battle_hud_available(void){return 1;}
int ctps5_battle_hud_set_enabled(int on){hud=on;return 1;}
int ctps5_battle_hud_scale(int c){return scales[c!=0];}
int ctps5_battle_hud_set_scale(int c,int v){scales[c!=0]=v;return 1;}
int display_setting_get(int id){assert(id==SET_MODS);return texture;}
void display_setting_set(int id,int v){assert(id==SET_MODS);texture=v;}
void display_message(const char*p,unsigned ms){(void)p;(void)ms;assert(0);}
static void read_ini(char b[4096]){FILE*f=fopen("/data/eggoftime/ps5.ini","r");assert(f);size_t n=fread(b,1,4095,f);b[n]=0;fclose(f);}
int main(int argc,char**argv){
 assert(argc==2);root=argv[1];char b[4096];
 FILE*f=fopen("/data/eggoftime/ps5.ini","w");assert(f);fputs("# Keep comment\noutput_height=2160\nactive_pack=default\ntarget_fps=60\nunknown=keep\n",f);fclose(f);
 egg_ps5_mod_menu_refresh();assert(!strcmp(egg_ps5_mod_menu_value(4),"default"));
 egg_ps5_mod_menu_change(4,1);assert(!strcmp(egg_ps5_mod_menu_value(4),"a-pack"));
 read_ini(b);assert(strstr(b,"active_pack=a-pack\n")&&strstr(b,"# Keep comment\n")&&strstr(b,"output_height=2160\n")&&strstr(b,"unknown=keep\n"));
 egg_ps5_mod_menu_change(4,1);assert(!strcmp(egg_ps5_mod_menu_value(4),"z-pack"));
 egg_ps5_mod_menu_change(4,1);assert(!strcmp(egg_ps5_mod_menu_value(4),"default"));
 egg_ps5_mod_menu_change(4,-1);assert(!strcmp(egg_ps5_mod_menu_value(4),"z-pack"));
 egg_ps5_mod_menu_change(0,1);assert(hud);egg_ps5_mod_menu_change(0,-1);assert(!hud);
 egg_ps5_mod_menu_change(3,1);assert(!texture);
 egg_ps5_mod_menu_change(1,1);assert(scales[0]==105);egg_ps5_mod_menu_change(2,-1);assert(scales[1]==60);
 assert(!egg_ps5_ini_set("/data/eggoftime/ps5.ini","active_pack","bad\nother=value"));
 read_ini(b);assert(strstr(b,"active_pack=z-pack\n"));
 f=fopen("/data/eggoftime/ps5.ini","w");assert(f);for(int i=0;i<1500;i++)fputc('x',f);fputs("\nactive_pack=z-pack\n",f);fclose(f);
 assert(!egg_ps5_ini_set("/data/eggoftime/ps5.ini","active_pack","default"));read_ini(b);assert(strlen(b)==1520&&strstr(b,"active_pack=z-pack"));
 puts("PASS: installed pack filtering/sorting/cycling, HUD/texture switches, persistent pack choice, unrelated settings/comments preserved, newline injection and oversized-line failures leave original intact");
}
