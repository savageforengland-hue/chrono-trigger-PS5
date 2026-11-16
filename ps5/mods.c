#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <ctype.h>
#include <SDL3/SDL.h>
#include "config.h"
#include "pack_state.h"
static char roots[CTPS5_PACK_LIMIT][256];
static int count=-1;
static int compare(const void*a,const void*b){return strcmp(a,b);}
static int valid(const char*s){size_t n=strlen(s);if(!n||n>=64)return 0;for(size_t i=0;i<n;i++)if(!isalnum((unsigned char)s[i])&&s[i]!='-'&&s[i]!='_')return 0;return 1;}
static void refresh_roots(void){
 ctps5_pack_states_refresh();count=0;
 if(ctps5_pack_enabled("default"))snprintf(roots[count++],sizeof roots[0],"/data/eggoftime/mods/graphics/");
 char names[CTPS5_PACK_LIMIT][64];int n=0;DIR*d=opendir("/data/eggoftime/mods/packs");
 if(d){struct dirent*e;while((e=readdir(d))&&n<CTPS5_PACK_LIMIT-1){if(!valid(e->d_name)||!strcmp(e->d_name,"default")||!ctps5_pack_enabled(e->d_name))continue;char p[256];SDL_PathInfo info;snprintf(p,sizeof p,"/data/eggoftime/mods/packs/%.63s/graphics",e->d_name);if(SDL_GetPathInfo(p,&info)&&info.type==SDL_PATHTYPE_DIRECTORY)snprintf(names[n++],64,"%.63s",e->d_name);}closedir(d);}
 qsort(names,(size_t)n,64,compare);
 for(int i=0;i<n&&count<CTPS5_PACK_LIMIT;i++)snprintf(roots[count++],sizeof roots[0],"/data/eggoftime/mods/packs/%s/graphics/",names[i]);
 for(int i=0;i<count;i++)SDL_Log("graphics pack layer %d (later wins): %s",i+1,roots[i]);
 if(!count)SDL_Log("graphics packs: all off; original game assets");
}
int egg_ps5_graphics_root_count(void){if(count<0)refresh_roots();return count;}
const char*egg_ps5_graphics_root_at(int i){if(count<0)refresh_roots();return i>=0&&i<count?roots[i]:NULL;}
/* Legacy single-root consumers use the highest enabled layer; a disabled
 * empty state has no folder and cannot accidentally load the default pack. */
const char *egg_ps5_graphics_root(void){if(count<0)refresh_roots();return count?roots[count-1]:"/data/eggoftime/mods/.no-enabled-packs/";}
