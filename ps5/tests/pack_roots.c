/* SPDX-License-Identifier: GPL-3.0-or-later */
#define _POSIX_C_SOURCE 200809L
#include "ps5/config.h"
#include "ps5/pack_state.h"
#include <SDL3/SDL.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
EggPs5Config egg_ps5_config={2160,60,4,"b-house"};static const char*root;static int fail_rename;
static const char*map(const char*p,char b[2048]){if(!strncmp(p,"/data/eggoftime",15)){snprintf(b,2048,"%s%s",root,p+15);return b;}return p;}
FILE*__real_fopen(const char*,const char*);FILE*__wrap_fopen(const char*p,const char*m){char b[2048];return __real_fopen(map(p,b),m);}
DIR*__real_opendir(const char*);DIR*__wrap_opendir(const char*p){char b[2048];return __real_opendir(map(p,b));}
int __real_rename(const char*,const char*);int __wrap_rename(const char*a,const char*b){char x[2048],y[2048];return fail_rename?-1:__real_rename(map(a,x),map(b,y));}
int __real_unlink(const char*);int __wrap_unlink(const char*p){char b[2048];return __real_unlink(map(p,b));}
bool SDL_GetPathInfo(const char*p,SDL_PathInfo*i){char b[2048];struct stat s;if(stat(map(p,b),&s))return false;i->type=S_ISDIR(s.st_mode)?SDL_PATHTYPE_DIRECTORY:SDL_PATHTYPE_FILE;return true;}
void SDL_Log(const char*f,...){(void)f;}
int main(int argc,char**argv){assert(argc==3);root=argv[1];const char*phase=argv[2];ctps5_pack_states_refresh();
 if(!strcmp(phase,"migration")){
  assert(ctps5_pack_enabled("b-house")&&!ctps5_pack_enabled("default")&&!ctps5_pack_enabled("a-char"));
  assert(ctps5_pack_set_enabled("a-char",1));assert(ctps5_pack_enabled("b-house")&&ctps5_pack_enabled("a-char"));
  assert(ctps5_pack_set_enabled("c-world",1)&&ctps5_pack_set_enabled("b-house",0));ctps5_pack_states_refresh();
  assert(ctps5_pack_enabled("a-char")&&ctps5_pack_enabled("c-world")&&!ctps5_pack_enabled("b-house"));
  assert(egg_ps5_graphics_root_count()==2&&strstr(egg_ps5_graphics_root_at(0),"a-char")&&strstr(egg_ps5_graphics_root_at(1),"c-world"));
  assert(!ctps5_pack_set_enabled("../escape",1));fail_rename=1;assert(!ctps5_pack_set_enabled("c-world",0)&&ctps5_pack_enabled("c-world"));fail_rename=0;
  assert(ctps5_pack_set_enabled("a-char",0));assert(egg_ps5_graphics_root_count()==2); /* loaded roots stable until restart */
 }else if(!strcmp(phase,"reload")){
  assert(egg_ps5_graphics_root_count()==1&&strstr(egg_ps5_graphics_root_at(0),"c-world"));assert(ctps5_pack_set_enabled("c-world",0));
 }else if(!strcmp(phase,"all-off")){
  assert(egg_ps5_graphics_root_count()==0&&!egg_ps5_graphics_root_at(0)&&strstr(egg_ps5_graphics_root(),".no-enabled-packs"));
 }else if(!strcmp(phase,"absent-legacy")){
  strcpy(egg_ps5_config.active_pack,"not-installed");ctps5_pack_states_refresh();assert(ctps5_pack_enabled("default"));assert(egg_ps5_graphics_root_count()==1&&strstr(egg_ps5_graphics_root_at(0),"mods/graphics/"));
 }else if(!strcmp(phase,"bad-lines")){
  assert(ctps5_pack_enabled("a-char")&&!ctps5_pack_enabled("b-house")&&!ctps5_pack_enabled("c-world"));assert(egg_ps5_graphics_root_count()==1&&strstr(egg_ps5_graphics_root_at(0),"a-char"));
 }else assert(0);
 printf("PASS: real independent pack state/root resolver %s\n",phase);return 0;}
