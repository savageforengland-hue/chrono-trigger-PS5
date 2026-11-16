/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "hud_themes.h"
#include "config_file.h"
#include "../runtime/rt.h"
#include <dirent.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BASE "/data/eggoftime/mods/battle-hud"
#include "config.h"
static char ids[64][64],chosen[64]="classic";
static int count;
static CtHudTheme *current;
static int valid(const char*s){size_t n=strlen(s);if(!n||n>63)return 0;for(size_t i=0;i<n;i++)if(!((s[i]>='a'&&s[i]<='z')||(s[i]>='A'&&s[i]<='Z')||(s[i]>='0'&&s[i]<='9')||s[i]=='_'||s[i]=='-'))return 0;return 1;}
static int compare(const void*a,const void*b){return strcmp(a,b);}
static void log_message(const char*s){rt_log("%s",s);}
void ctps5_hud_themes_refresh(void){
 strcpy(ids[0],"classic");count=1;DIR*d=opendir(BASE "/themes");if(!d)return;
 struct dirent*e;while((e=readdir(d))&&count<64){if(!valid(e->d_name)||!strcmp(e->d_name,"classic"))continue;char path[512];snprintf(path,sizeof path,BASE "/themes/%.63s",e->d_name);SDL_PathInfo info;if(SDL_GetPathInfo(path,&info)&&info.type==SDL_PATHTYPE_DIRECTORY)snprintf(ids[count++],64,"%.63s",e->d_name);}closedir(d);qsort(ids+1,(size_t)(count-1),64,compare);
}
static int apply(const char*requested_id,int save){
 char id[64];snprintf(id,sizeof id,"%s",requested_id);
 char root[512],fallback[512],error[192];int classic=!strcmp(id,"classic");
 snprintf(root,sizeof root,BASE "/themes/%s",id);
#ifdef EGG_PS5_PACK_STACK
 char paths[64][512];const char*fallbacks[64];int n=egg_ps5_graphics_root_count();if(n>64)n=64;
 for(int i=0;i<n;i++){snprintf(paths[i],sizeof paths[i],"%shud",egg_ps5_graphics_root_at(n-1-i));fallbacks[i]=paths[i];}
 CtHudTheme*next=ct_hud_theme_load_stack(classic?NULL:root,fallbacks,n,error,sizeof error,log_message);
#else
 snprintf(fallback,sizeof fallback,"%shud",egg_ps5_graphics_root());
 CtHudTheme*next=ct_hud_theme_load(classic?fallback:root,classic?NULL:fallback,error,sizeof error,log_message);
#endif
 if(error[0]){rt_log("HUD theme '%s' rejected: %s; current appearance retained",id,error);return 0;}
 if(save){mkdir(BASE,0777);if(!egg_ps5_ini_set(BASE "/hud.ini","theme",id)){ct_hud_theme_free(next);return 0;}}
 ct_hud_theme_free(current);current=next;snprintf(chosen,sizeof chosen,"%s",id);
 rt_log("HUD theme: %s, %d PNG overrides; missing graphics use original assets",id,next?next->overrides:0);return 1;
}
void ctps5_hud_themes_init(void){
 ctps5_hud_themes_refresh();char wanted[64]="classic",line[192];FILE*f=fopen(BASE "/hud.ini","r");if(f){while(fgets(line,sizeof line,f)){char v[64],extra;if(sscanf(line," theme = %63s %c",v,&extra)==1&&valid(v))strcpy(wanted,v);}fclose(f);}
 int found=0;for(int i=0;i<count;i++)found|=!strcmp(ids[i],wanted);if(!found)strcpy(wanted,"classic");if(!apply(wanted,0))apply("classic",0);
}
CtHudTheme*ctps5_hud_theme(void){return current;}
const char*ctps5_hud_theme_name(void){return chosen;}
int ctps5_hud_theme_change(int direction){
 ctps5_hud_themes_refresh();if(!direction)return apply(chosen,0);int at=0;for(int i=0;i<count;i++)if(!strcmp(ids[i],chosen))at=i;return apply(ids[(at+count+(direction<0?-1:1))%count],1);
}
