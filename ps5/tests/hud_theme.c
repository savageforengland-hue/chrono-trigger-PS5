/* SPDX-License-Identifier: GPL-3.0-or-later */
#define _POSIX_C_SOURCE 200809L
#include "runtime/hud_theme.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
static int logs;
static void log_line(const char*s){assert(strstr(s,"original asset retained"));logs++;}
static void write_text(const char*path,const char*s){FILE*f=fopen(path,"w");assert(f);assert(fputs(s,f)>=0);assert(!fclose(f));}
static void save(const char*path,int w,int h,uint32_t colour){SDL_Surface*s=SDL_CreateSurface(w,h,SDL_PIXELFORMAT_ABGR8888);assert(s);for(int y=0;y<h;y++)for(int x=0;x<w;x++)((uint32_t*)((uint8_t*)s->pixels+y*s->pitch))[x]=colour;assert(SDL_SavePNG(s,path));SDL_DestroySurface(s);}
int main(int argc,char**argv){
 assert(argc==3);char error[192],path[1200],fallback[1200];const char*root=argv[1];snprintf(fallback,sizeof fallback,"%s/fallback",root);assert(!mkdir(fallback,0755));
 CtHudThemeConfig c,before;ct_hud_theme_defaults(&c);before=c;
 const char*bad[]={"{", "[]", "{\"schema\":2}","{\"schema\":1,\"schema\":1}","{\"schema\":1,\"colors\":{\"background\":\"red\"}}","{\"schema\":1,\"assets\":{\"crono\":\"../evil.png\"}}","{\"schema\":1,\"assets\":{\"crono\":\"/evil.png\"}}","{\"schema\":1,\"canvas\":[1200,240],\"pixel_scale\":4}","{\"schema\":1,\"layout\":{\"portrait\":[159,0,48,48]}}","{\"schema\":1,\"font\":{\"count\":128}}","{\"schema\":1,\"show\":{\"hp\":1}}"};
 for(size_t i=0;i<sizeof bad/sizeof *bad;i++){assert(!ct_hud_theme_parse(bad[i],strlen(bad[i]),&c,error,sizeof error));assert(error[0]&&!memcmp(&c,&before,sizeof c));}
 const char*escaped="{\"schema\":1,\"name\":\"A \\\"test\\\" \\u0026 more\"}";assert(ct_hud_theme_parse(escaped,strlen(escaped),&c,error,sizeof error)&&!strcmp(c.name_label,"A \"test\" & more"));
 assert(!ct_hud_theme_load(root,NULL,error,sizeof error,log_line)&&!error[0]); /* empty is original */
 snprintf(path,sizeof path,"%s/portraits",root);assert(!mkdir(path,0755));snprintf(path,sizeof path,"%s/portraits/crono.png",root);save(path,27,59,0xff332211u);
 CtHudTheme*t=ct_hud_theme_load(root,NULL,error,sizeof error,log_line);assert(t&&!error[0]&&t->overrides==1);
 uint32_t faces[7][48*48],skin[80*32],icons[3][32*32];for(int ch=0;ch<7;ch++)for(int i=0;i<48*48;i++)faces[ch][i]=0xff554400u+ch;for(int i=0;i<80*32;i++)skin[i]=0xff112233u;for(int ch=0;ch<3;ch++)for(int i=0;i<32*32;i++)icons[ch][i]=0xff123456u+ch;
 CtHudState state={.selected=-1,.command=1};for(int i=0;i<3;i++){state.member[i]=(CtHudMember){.present=1,.character=i,.hp=742,.max_hp=850,.mp=52,.max_mp=75,.atb=50,.atb_max=100};snprintf(state.member[i].name,24,"Member%d",i);}
 const uint32_t*p=ct_hud_theme_draw(t,&state,faces,skin);int s=t->cfg.scale,w=t->pixel_w;
 assert(p[(35*s)*w+25*s]==0xff332211u);assert(p[(35*s)*w+185*s]==faces[1][0]);assert(p[(35*s)*w+345*s]==faces[2][0]);assert(p[40*s*w+55*s]==skin[0]);
 const uint32_t*cmd=ct_hud_theme_commands(t,1,icons);assert(!cmd[0]&&cmd[3*s*t->command_w+24*s]==icons[0][0]);
 ct_hud_theme_free(t);
 /* A malformed image does not hide the original or the fallback image. */
 write_text(path,"bad PNG");t=ct_hud_theme_load(root,NULL,error,sizeof error,log_line);assert(!t&&!error[0]&&logs==1);
 char fdir[1300];snprintf(fdir,sizeof fdir,"%s/portraits",fallback);assert(!mkdir(fdir,0755));snprintf(fdir,sizeof fdir,"%s/portraits/marle.png",fallback);save(fdir,64,64,0xff998877u);
 t=ct_hud_theme_load(root,fallback,error,sizeof error,log_line);assert(t&&t->overrides==1);p=ct_hud_theme_draw(t,&state,faces,skin);assert(p[35*s*w+25*s]==faces[0][0]&&p[35*s*w+185*s]==0xff998877u);ct_hud_theme_free(t);
 snprintf(path,sizeof path,"%s/theme.json",root);write_text(path,"{\"schema\":1,\"colors\":{\"background\":\"#12345680\"},\"opacity\":128}");t=ct_hud_theme_load(root,NULL,error,sizeof error,log_line);assert(t&&t->cfg.colour[TC_BG]==0x80563412u);p=ct_hud_theme_draw(t,&state,faces,skin);assert(p[40*s*w+55*s]>>24==64);ct_hud_theme_free(t);
 write_text(path,"broken JSON");assert(!ct_hud_theme_load(root,NULL,error,sizeof error,log_line)&&error[0]);assert(!unlink(path));
 /* The shipped remaster is itself accepted by the real loader/renderer. */
 t=ct_hud_theme_load(argv[2],NULL,error,sizeof error,log_line);assert(t&&!error[0]&&t->cfg.width==600&&t->cfg.height==100);p=ct_hud_theme_draw(t,&state,faces,skin);assert(p&&t->pixel_w==1200);ct_hud_theme_commands(t,2,icons);
 snprintf(path,sizeof path,"%s/theme-preview.png",root);SDL_Surface*preview=SDL_CreateSurfaceFrom(t->pixel_w,t->pixel_h,SDL_PIXELFORMAT_ABGR8888,t->pixels,t->pixel_w*4);assert(preview&&SDL_SavePNG(preview,path));SDL_DestroySurface(preview);ct_hud_theme_free(t);
 puts("PASS: bounded transactional JSON, empty folders use originals, arbitrary-size single-portrait override, other portraits/icons/skin retain originals, damaged/missing PNG fallback, theme layering, alpha, canvas resize and shipped theme render");
}
