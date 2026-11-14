#include "ps5/battle_hud.h"
#include "runtime/battle_hud_state.h"
#include "runtime/rt.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
RtRom rt_rom;
uint8_t rt_main_ram[RT_MAIN_RAM_SIZE];
static int mode,overlays=1,fail_texture,fail_upload,face_calls,log_calls;
FILE *__wrap_fopen(const char *p,const char *m){(void)p;(void)m;if(!mode)return NULL;FILE*f=tmpfile();assert(f);fputs(mode==1?"enabled=1\n":mode==2?"enabled=0\n":"enabled=bad\n",f);rewind(f);return f;}
void rt_log(const char *fmt,...){(void)fmt;log_calls++;}
int rt_overlay_resident(int id){(void)id;return overlays;}
int ct_hud_load_faces(uint32_t p[7][48*48]){face_calls++;memset(p,0,7*48*48*4);return 1;}
SDL_Mutex *SDL_CreateMutex(void){return (SDL_Mutex*)1;}
void SDL_LockMutex(SDL_Mutex*m){assert(m);}
void SDL_UnlockMutex(SDL_Mutex*m){assert(m);}
SDL_Texture *SDL_CreateTexture(SDL_Renderer*r,SDL_PixelFormat f,SDL_TextureAccess a,int w,int h){(void)r;assert(f==SDL_PIXELFORMAT_ABGR8888&&a==SDL_TEXTUREACCESS_STREAMING&&w==480&&h==64);return fail_texture?NULL:(SDL_Texture*)2;}
bool SDL_SetTextureScaleMode(SDL_Texture*t,SDL_ScaleMode m){assert(t&&m==SDL_SCALEMODE_NEAREST);return true;}
bool SDL_UpdateTexture(SDL_Texture*t,const SDL_Rect*r,const void*p,int pitch){assert(t&&!r&&p&&pitch==480*4);return !fail_upload;}
static void w8(unsigned a,unsigned v){rt_main_ram[a-0x2000000]=(uint8_t)v;}
static void w16(unsigned a,unsigned v){w8(a,v);w8(a+1,v>>8);}
static void w32(unsigned a,unsigned v){w16(a,v);w16(a+2,v>>16);}
int main(void){
 SDL_FRect rect={0,0,3840,2160},hud={0},before=rect;SDL_Texture*texture=NULL;
 for(mode=0;mode<=3;mode++){
  if(mode==1)continue;ctps5_battle_hud_init();ctps5_battle_hud_frame();
  assert(!ctps5_battle_hud_prepare(NULL,&texture,&rect,&hud));assert(!memcmp(&rect,&before,sizeof rect));assert(!face_calls);
 }
 mode=1;strcpy(rt_rom.code,"YQUP");ctps5_battle_hud_init();assert(!face_calls);assert(log_calls);
 strcpy(rt_rom.code,"YQUE");ctps5_battle_hud_init();assert(face_calls==1);
 w32(0x207088c,0x2200000);w32(0x2200000,0x2115558);w32(0x20728fc,0x2200100);w32(0x2118c0c,0x2210000);
 w8(0x2200101,0x80);w8(0x2200102,0x80);w16(0x2210003,50);w16(0x2210005,70);w16(0x2210007,3);w16(0x2210009,8);w8(0x2211545,1);
 ctps5_battle_hud_frame();fail_texture=1;assert(!ctps5_battle_hud_prepare(NULL,&texture,&rect,&hud));assert(!memcmp(&rect,&before,sizeof rect));
 fail_texture=0;fail_upload=1;assert(!ctps5_battle_hud_prepare(NULL,&texture,&rect,&hud));assert(!memcmp(&rect,&before,sizeof rect));
 fail_upload=0;assert(ctps5_battle_hud_prepare(NULL,&texture,&rect,&hud));
 assert(fabs(rect.w/rect.h-16.0/9)<.0001);assert(rect.x>=0&&hud.y>=rect.y+rect.h-.001);assert(hud.y+hud.h<=2160.001);assert(fabs(hud.w/hud.h-7.5)<.001);
 overlays=0;ctps5_battle_hud_frame();rect=before;assert(!ctps5_battle_hud_prepare(NULL,&texture,&rect,&hud));assert(!memcmp(&rect,&before,sizeof rect));
 puts("PASS: mod absent/off/malformed/unsupported-ROM guards, live frame publication, texture failures retain original layout, 4K dock bounds and aspect, battle-exit cleanup");
}
