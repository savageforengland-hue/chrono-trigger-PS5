/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "runtime/hud_theme.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static void image(const char*root,const char*name,unsigned color){char p[1200];snprintf(p,sizeof p,"%s/portraits",root);assert(SDL_CreateDirectory(p));snprintf(p,sizeof p,"%s/%s",root,name);SDL_Surface*s=SDL_CreateSurface(24,32,SDL_PIXELFORMAT_ABGR8888);assert(s);SDL_FillSurfaceRect(s,NULL,color);assert(SDL_SavePNG(s,p));SDL_DestroySurface(s);}
static unsigned color(CtHudTheme*t,int image){assert(t&&t->image[image].surface);return *(unsigned*)t->image[image].surface->pixels;}
int main(int argc,char**argv){assert(argc==2&&SDL_Init(0));char a[1024],b[1024],c[1024],theme[1024],p[1200],error[192];snprintf(a,sizeof a,"%s/a",argv[1]);snprintf(b,sizeof b,"%s/b",argv[1]);snprintf(c,sizeof c,"%s/c",argv[1]);snprintf(theme,sizeof theme,"%s/theme",argv[1]);assert(SDL_CreateDirectory(a)&&SDL_CreateDirectory(b)&&SDL_CreateDirectory(c)&&SDL_CreateDirectory(theme));image(a,"portraits/crono.png",0xff225588);image(b,"portraits/marle.png",0xff55aa22);snprintf(p,sizeof p,"%s/portraits/crono.png",b);FILE*f=fopen(p,"w");assert(f);fputs("invalid PNG",f);fclose(f);
 const char*layers[]={b,a};CtHudTheme*t=ct_hud_theme_load_stack(NULL,layers,2,error,sizeof error,NULL);assert(!error[0]&&color(t,TH_CRONO)==0xff225588&&color(t,TH_MARLE)==0xff55aa22);ct_hud_theme_free(t);
 image(c,"portraits/crono.png",0xffbb44dd);const char*three[]={c,b,a};t=ct_hud_theme_load_stack(NULL,three,3,error,sizeof error,NULL);assert(!error[0]&&color(t,TH_CRONO)==0xffbb44dd&&color(t,TH_MARLE)==0xff55aa22);ct_hud_theme_free(t);
 image(theme,"portraits/crono.png",0xffaa7733);snprintf(p,sizeof p,"%s/theme.json",theme);f=fopen(p,"w");assert(f);fputs("{\"schema\":1,\"colors\":{\"background\":\"#123456\"}}",f);fclose(f);t=ct_hud_theme_load_stack(theme,three,3,error,sizeof error,NULL);assert(!error[0]&&color(t,TH_CRONO)==0xffaa7733&&color(t,TH_MARLE)==0xff55aa22);ct_hud_theme_free(t);
 t=ct_hud_theme_load_stack(NULL,NULL,0,error,sizeof error,NULL);assert(!t&&!error[0]);puts("PASS: HUD multi-pack faces combine, higher valid image wins, corrupt higher image falls back, explicit theme stays on top and zero packs restores originals");return 0;}
