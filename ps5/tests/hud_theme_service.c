/* SPDX-License-Identifier: GPL-3.0-or-later */
#define _POSIX_C_SOURCE 200809L
#include "ps5/hud_themes.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
static const char*root;static char graphics[1200];
const char*egg_ps5_graphics_root(void){return graphics;}
void rt_log(const char*fmt,...){(void)fmt;}
static const char*map(const char*p,char out[1500]){if(!strncmp(p,"/data/eggoftime",15)){snprintf(out,1500,"%s%s",root,p+15);return out;}return p;}
FILE*__real_fopen(const char*,const char*);FILE*__wrap_fopen(const char*p,const char*m){char out[1500];return __real_fopen(map(p,out),m);}
DIR*__real_opendir(const char*);DIR*__wrap_opendir(const char*p){char out[1500];return __real_opendir(map(p,out));}
int __real_mkdir(const char*,mode_t);int __wrap_mkdir(const char*p,mode_t m){char out[1500];return __real_mkdir(map(p,out),m);}
int __real_rename(const char*,const char*);int __wrap_rename(const char*a,const char*b){char ma[1500],mb[1500];return __real_rename(map(a,ma),map(b,mb));}
int __real_unlink(const char*);int __wrap_unlink(const char*p){char out[1500];return __real_unlink(map(p,out));}
bool __wrap_SDL_GetPathInfo(const char*p,SDL_PathInfo*i){char out[1500];struct stat s;if(stat(map(p,out),&s))return false;i->type=S_ISDIR(s.st_mode)?SDL_PATHTYPE_DIRECTORY:SDL_PATHTYPE_FILE;return true;}
static void write_file(const char*p,const char*s){FILE*f=fopen(p,"w");assert(f&&fputs(s,f)>=0&&!fclose(f));}
int main(int argc,char**argv){assert(argc==2);root=argv[1];snprintf(graphics,sizeof graphics,"%s/mods/graphics/",root);assert(SDL_CreateDirectory(graphics));char path[1500];const char*ids[]={"bad","good","plain","invalid.name"};for(int i=0;i<4;i++){snprintf(path,sizeof path,"%s/mods/battle-hud/themes/%s",root,ids[i]);assert(SDL_CreateDirectory(path));}
 write_file("/data/eggoftime/mods/battle-hud/themes/good/theme.json","{\"schema\":1,\"colors\":{\"background\":\"#123456\"}}");write_file("/data/eggoftime/mods/battle-hud/themes/bad/theme.json","bad JSON");
 write_file("/data/eggoftime/mods/battle-hud/hud.ini","# Keep comment\nenabled=1\nscale_percent=115\ncommand_scale_percent=80\nunknown=keep\ntheme=good\n");
 ctps5_hud_themes_init();assert(!strcmp(ctps5_hud_theme_name(),"good")&&ctps5_hud_theme());assert(ctps5_hud_theme_change(0));assert(!strcmp(ctps5_hud_theme_name(),"good"));CtHudTheme*before=ctps5_hud_theme();assert(!ctps5_hud_theme_change(-1));assert(before==ctps5_hud_theme()&&!strcmp(ctps5_hud_theme_name(),"good"));
 assert(ctps5_hud_theme_change(1)&&!strcmp(ctps5_hud_theme_name(),"plain")&&!ctps5_hud_theme());assert(ctps5_hud_theme_change(1)&&!strcmp(ctps5_hud_theme_name(),"classic")&&!ctps5_hud_theme());assert(ctps5_hud_theme_change(-1)&&!strcmp(ctps5_hud_theme_name(),"plain"));assert(ctps5_hud_theme_change(-1)&&ctps5_hud_theme());
 FILE*f=fopen("/data/eggoftime/mods/battle-hud/hud.ini","r");assert(f);char text[1024];size_t n=fread(text,1,sizeof text-1,f);text[n]=0;fclose(f);assert(strstr(text,"# Keep comment\n")&&strstr(text,"enabled=1\n")&&strstr(text,"scale_percent=115\n")&&strstr(text,"command_scale_percent=80\n")&&strstr(text,"unknown=keep\n")&&strstr(text,"theme=good\n"));
 puts("PASS: theme discovery/filtering/cycling, live reload retains selection, bad JSON retains current theme, PNG-only empty theme restores originals, classic fallback, existing HUD settings/comments preserved");
}
