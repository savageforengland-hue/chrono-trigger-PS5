/* SPDX-License-Identifier: GPL-3.0-or-later */
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
uint64_t rt_frame_count;
uint32_t rt_dtcm_base=0x027e0000;
uint32_t rt_r32_slow(uint32_t a){(void)a;assert(0);return 0;}
uint32_t rt_r16_slow(uint32_t a){(void)a;assert(0);return 0;}
static int mode,overlays=1,fail_texture,fail_upload,face_calls,log_calls;
FILE *__wrap_fopen(const char *p,const char *m){(void)p;(void)m;if(!mode)return NULL;FILE*f=tmpfile();assert(f);fputs(mode==1?"enabled=1\n":mode==2?"enabled=0\n":"enabled=bad\n",f);rewind(f);return f;}
void rt_log(const char *fmt,...){(void)fmt;log_calls++;}
int rt_overlay_resident(int id){(void)id;return overlays;}
int ct_hud_load_faces(uint32_t p[7][48*48]){face_calls++;memset(p,0,7*48*48*4);return 1;}
SDL_Mutex *SDL_CreateMutex(void){return (SDL_Mutex*)1;}
void SDL_LockMutex(SDL_Mutex*m){assert(m);}
void SDL_UnlockMutex(SDL_Mutex*m){assert(m);}
SDL_Texture *SDL_CreateTexture(SDL_Renderer*r,SDL_PixelFormat f,SDL_TextureAccess a,int w,int h){(void)r;assert(f==SDL_PIXELFORMAT_ABGR8888&&a==SDL_TEXTUREACCESS_STREAMING&&w==CT_HUD_W&&h==CT_HUD_H);return fail_texture?NULL:(SDL_Texture*)2;}
bool SDL_SetTextureScaleMode(SDL_Texture*t,SDL_ScaleMode m){assert(t&&m==SDL_SCALEMODE_NEAREST);return true;}
bool SDL_UpdateTexture(SDL_Texture*t,const SDL_Rect*r,const void*p,int pitch){assert(t&&!r&&p&&pitch==CT_HUD_W*4);return !fail_upload;}
static void w8(unsigned a,unsigned v){rt_main_ram[a-0x2000000]=(uint8_t)v;}
static void w16(unsigned a,unsigned v){w8(a,v);w8(a+1,v>>8);}
static void w32(unsigned a,unsigned v){w16(a,v);w16(a+2,v>>16);}
static void observe(int row,int on){
 unsigned base=0x2200000+0xaf64;w16(base+20*0x9c+0x22,on);
 ctps5_battle_hud_entry(base+20*0x9c);ctps5_battle_hud_cell(base,row+9);
 ctps5_battle_hud_frame();
}
int main(void){
 SDL_FRect rect={0,0,3840,2160},hud={0},cover={0},before=rect;SDL_Texture*texture=NULL;
 #define PREP(c,r,z) ctps5_battle_hud_prepare(NULL,&texture,&rect,c,r,z,&cover,&hud)
 for(mode=0;mode<=3;mode++){
  if(mode==1)continue;ctps5_battle_hud_init();ctps5_battle_hud_frame();
  assert(!PREP(340,192,0));assert(!memcmp(&rect,&before,sizeof rect));assert(!face_calls);
 }
 mode=1;strcpy(rt_rom.code,"YQUP");ctps5_battle_hud_init();assert(!face_calls);assert(log_calls);
 strcpy(rt_rom.code,"YQUE");ctps5_battle_hud_init();assert(face_calls==1);
 w32(0x207088c,0x2200000);w32(0x2200000,0x2115558);w32(0x20728fc,0x2200100);w32(0x2118c0c,0x2210000);
 w8(0x2200101,0x80);w8(0x2200102,0x80);w16(0x2210003,50);w16(0x2210005,70);w16(0x2210007,3);w16(0x2210009,8);w8(0x2211545,1);
 ctps5_battle_hud_frame();assert(!PREP(340,192,0)); /* battle alone is insufficient */
 observe(146,1);fail_texture=1;assert(!PREP(340,192,0));assert(!memcmp(&rect,&before,sizeof rect));
 fail_texture=0;fail_upload=1;assert(!PREP(340,192,0));assert(!memcmp(&rect,&before,sizeof rect));
 fail_upload=0;
 for(int zoom=0;zoom<3;zoom++)for(int top=0;top<2;top++){
  int row=top?2:146,rows=zoom?240:192;observe(row,1);assert(PREP(340,rows,zoom));
  assert(!memcmp(&rect,&before,sizeof rect));
  float k=zoom==1?(float)rows/192:1;
  assert(fabs(cover.x-(170+24*k)*3840/340)<.001);
  assert(cover.y>=0&&cover.y+cover.h<=2160.001);
  assert(cover.x>=0&&cover.x+cover.w<=3840.001);
  /* The entire original status box is covered. Commands at DS x<152
   * and the message panel at the opposite edge remain outside it. */
  assert(fabs(cover.w-(170-24*k)*3840/340)<.001);
  assert(hud.x>=cover.x-.001&&hud.y>=cover.y-.001&&hud.x+hud.w<=cover.x+cover.w+.001&&hud.y+hud.h<=cover.y+cover.h+.001);
  assert(fabs(hud.w/hud.h-(float)CT_HUD_W/CT_HUD_H)<.001);
 }
 observe(146,0);assert(!PREP(340,192,0)); /* Tech/Item use their original panels */
 observe(146,1);rt_frame_count+=3;ctps5_battle_hud_frame();assert(!PREP(340,192,0));
 observe(146,1);overlays=0;ctps5_battle_hud_frame();assert(!PREP(340,192,0));
 assert(!memcmp(&rect,&before,sizeof rect));
 puts("PASS: switch/ROM guards, actual stock-header/digit visibility, Tech/Item yield, stale-frame/battle-exit cleanup, texture fallbacks, immutable game view, complete status-box cover, command/message separation, top/bottom/zoom/4K geometry and portrait proportions");
}
