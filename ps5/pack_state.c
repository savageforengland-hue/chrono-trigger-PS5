/* SPDX-License-Identifier: GPL-3.0-or-later
 * Explicit per-pack state; a missing state file preserves the legacy selection. */
#include "pack_state.h"
#include "config.h"
#include "config_file.h"
#include <SDL3/SDL.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#define STATES "/data/eggoftime/mods/pack-states.ini"
#define STATE_LIMIT 256
static struct State {char id[64];int enabled;} states[STATE_LIMIT];
static int count,explicit_states;
static char legacy[64]="default";
static int valid(const char*s){size_t n=strlen(s);if(!n||n>=64)return 0;for(size_t i=0;i<n;i++)if(!isalnum((unsigned char)s[i])&&s[i]!='_'&&s[i]!='-')return 0;return 1;}
static void remember(const char*id,int on){int i=0;for(;i<count;i++)if(!strcmp(states[i].id,id))break;if(i==STATE_LIMIT)return;if(i==count)count++;snprintf(states[i].id,sizeof states[i].id,"%s",id);states[i].enabled=on!=0;}
void ctps5_pack_states_refresh(void){
 count=explicit_states=0;snprintf(legacy,sizeof legacy,"%s",egg_ps5_config.active_pack);
 if(strcmp(legacy,"default")){char p[256];SDL_PathInfo info;snprintf(p,sizeof p,"/data/eggoftime/mods/packs/%s/graphics",legacy);if(!SDL_GetPathInfo(p,&info)||info.type!=SDL_PATHTYPE_DIRECTORY)strcpy(legacy,"default");}
 FILE*f=fopen(STATES,"r");if(!f)return;explicit_states=1;char line[256];
 while(fgets(line,sizeof line,f)){
  if(!strchr(line,'\n')&&!feof(f)){int ch;while((ch=fgetc(f))!=EOF&&ch!='\n'){}continue;}
  char*p=line;while(isspace((unsigned char)*p))p++;if(!*p||*p=='#'||*p==';')continue;
  char*eq=strchr(p,'=');if(!eq)continue;*eq=0;char*end=eq;while(end>p&&isspace((unsigned char)end[-1]))*--end=0;
  int on;char extra;if(valid(p)&&sscanf(eq+1," %d %c",&on,&extra)==1&&(on==0||on==1))remember(p,on);
 }fclose(f);
}
int ctps5_pack_enabled(const char*id){if(!explicit_states)return !strcmp(id,legacy);for(int i=0;i<count;i++)if(!strcmp(id,states[i].id))return states[i].enabled;return 0;}
int ctps5_pack_set_enabled(const char*id,int enabled){
 if(!valid(id))return 0;
 /* Preserve the previously selected pack before adding a second one. A failed
  * second write leaves the former selection active, never silently disables it. */
 if(!explicit_states&&strcmp(id,legacy)){
  if(!egg_ps5_ini_set(STATES,legacy,"1"))return 0;
  explicit_states=1;remember(legacy,1);
 }
 if(!egg_ps5_ini_set(STATES,id,enabled?"1":"0"))return 0;
 explicit_states=1;remember(id,enabled);return 1;
}
