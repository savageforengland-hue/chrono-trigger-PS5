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
static int mode,overlays=1,fail_texture,fail_upload,face_calls,log_calls,save_ok=1;
FILE *__wrap_fopen(const char *p,const char *m){(void)p;(void)m;if(!mode)return NULL;FILE*f=tmpfile();assert(f);fputs(mode==1?"enabled=1\n":mode==2?"enabled=0\n":"enabled=bad\n",f);rewind(f);return f;}
void rt_log(const char *fmt,...){(void)fmt;log_calls++;}
int rt_overlay_resident(int id){(void)id;return overlays;}
int ct_hud_load_faces(uint32_t p[7][48*48]){face_calls++;memset(p,0,7*48*48*4);return 1;}
int ct_hud_load_window_skin(uint32_t p[80*32]){(void)p;return 0;}
int ct_hud_load_command_icons(uint32_t p[3][32*32]){memset(p,0,3*32*32*4);return 1;}
int egg_ps5_ini_set(const char*p,const char*k,const char*v){(void)p;(void)k;(void)v;return save_ok;}
int SDL_GetAtomicInt(SDL_AtomicInt*a){return a->value;}
int SDL_SetAtomicInt(SDL_AtomicInt*a,int v){int old=a->value;a->value=v;return old;}
SDL_Mutex *SDL_CreateMutex(void){return (SDL_Mutex*)1;}
void SDL_LockMutex(SDL_Mutex*m){assert(m);}
void SDL_UnlockMutex(SDL_Mutex*m){assert(m);}
SDL_Texture *SDL_CreateTexture(SDL_Renderer*r,SDL_PixelFormat f,SDL_TextureAccess a,int w,int h){(void)r;assert(f==SDL_PIXELFORMAT_ABGR8888&&a==SDL_TEXTUREACCESS_STREAMING&&((w==CT_HUD_W&&h==CT_HUD_H)||(w==CT_CMD_W&&h==CT_CMD_H)));return fail_texture?NULL:(SDL_Texture*)2;}
bool SDL_SetTextureScaleMode(SDL_Texture*t,SDL_ScaleMode m){assert(t&&m==SDL_SCALEMODE_NEAREST);return true;}
bool SDL_SetTextureBlendMode(SDL_Texture*t,SDL_BlendMode m){assert(t&&(m==SDL_BLENDMODE_BLEND||m==SDL_BLENDMODE_NONE));return true;}
bool SDL_UpdateTexture(SDL_Texture*t,const SDL_Rect*r,const void*p,int pitch){assert(t&&!r&&p&&(pitch==CT_HUD_W*4||pitch==CT_CMD_W*4));return !fail_upload;}
static void w8(unsigned a,unsigned v){rt_main_ram[a-0x2000000]=(uint8_t)v;}
static void w16(unsigned a,unsigned v){w8(a,v);w8(a+1,v>>8);}
static void w32(unsigned a,unsigned v){w16(a,v);w16(a+2,v>>16);}
static void observe(int row,int on){
 unsigned base=0x2200000+0xaf64;w16(base+20*0x9c+0x22,on);
 ctps5_battle_hud_entry(base+20*0x9c);ctps5_battle_hud_cell(base,200,row+9);
 ctps5_battle_hud_cell(base+22*0x9c,0,row+1);
 ctps5_battle_hud_frame();
}
int main(void){
 SDL_FRect rect={0,0,3840,2160},before=rect;CtHudLayout layout;SDL_Texture*texture=NULL,*commands=NULL;
 #define PREP(c,r,z) ctps5_battle_hud_prepare(NULL,&texture,&commands,&rect,c,r,z,&layout)
 for(mode=0;mode<=3;mode++){
  if(mode==1)continue;ctps5_battle_hud_init();ctps5_battle_hud_frame();
  assert(!PREP(340,192,0));assert(!memcmp(&rect,&before,sizeof rect));assert(!face_calls);
 }
 mode=1;strcpy(rt_rom.code,"YQUP");ctps5_battle_hud_init();assert(!face_calls);
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
  SDL_FRect cover=layout.cover,hud=layout.hud,bar=layout.commands;
  float old_ui=256*(zoom==1?(float)rows/192:1)*rect.w/340;
  assert(fabs(cover.x+cover.w/2-1920)<.001&&fabs(cover.w-old_ui)<.001);
  assert(cover.y>=0&&cover.y+cover.h<=2160.001);
  assert(cover.y+cover.h>=hud.y+hud.h); /* Erase the old panel's final border. */
  assert(fabs(hud.x+hud.w/2-1920)<.001);assert(fabs(bar.x+bar.w/2-1920)<.001);
  assert(hud.x>=rect.x&&hud.x+hud.w<=rect.x+rect.w&&hud.y>=0&&hud.y+hud.h<=2160.001);
  assert(fabs(hud.w/hud.h-(float)CT_HUD_W/CT_HUD_H)<.001);
  assert(bar.y>=0&&bar.y+bar.h<=2160.001);
  assert(bar.y+bar.h<=cover.y||bar.y>=cover.y+cover.h);
  assert(ctps5_battle_hud_input(KEY_LEFT|KEY_A)==(KEY_UP|KEY_A));
  assert(ctps5_battle_hud_input(KEY_RIGHT|KEY_B)==(KEY_DOWN|KEY_B));
  assert(ctps5_battle_hud_input(KEY_L)==KEY_LEFT&&ctps5_battle_hud_input(KEY_R)==KEY_RIGHT);
 }
 observe(146,1);ctps5_battle_hud_cell(0x2200000+0xaf64+22*0x9c,-1,147);ctps5_battle_hud_frame();
 assert(PREP(340,192,0)&&layout.commands.w==0);assert(ctps5_battle_hud_input(KEY_RIGHT|KEY_R)==(KEY_RIGHT|KEY_R));
 observe(146,0);assert(!PREP(340,192,0));assert(ctps5_battle_hud_input(KEY_LEFT|KEY_L)==(KEY_LEFT|KEY_L)); /* Tech/Item use their original panels */
 observe(146,1);rt_frame_count+=3;ctps5_battle_hud_frame();assert(!PREP(340,192,0));
 observe(146,1);overlays=0;ctps5_battle_hud_frame();assert(!PREP(340,192,0));
 assert(!memcmp(&rect,&before,sizeof rect));
 for(int scale=75;scale<=125;scale+=25){
  assert(ctps5_battle_hud_set_scale(0,scale));overlays=1;observe(146,1);assert(PREP(340,192,0));
  assert(layout.hud.y>=0&&layout.hud.y+layout.hud.h<=2160.001&&fabs(layout.hud.x+layout.hud.w/2-1920)<.001);
  assert(!memcmp(&rect,&before,sizeof rect));
 }
 assert(!ctps5_battle_hud_set_scale(0,999));assert(!ctps5_battle_hud_set_scale(1,0));
 assert(ctps5_battle_hud_set_enabled(0)&&!ctps5_battle_hud_enabled());
 save_ok=0;assert(!ctps5_battle_hud_set_enabled(1)&&!ctps5_battle_hud_enabled());
 save_ok=1;assert(ctps5_battle_hud_set_enabled(1)&&ctps5_battle_hud_enabled());
 puts("PASS: switch/ROM guards, actual stock-header/digit visibility, Tech/Item yield, stale-frame/battle-exit cleanup, texture fallbacks, immutable game view, full old-panel cover, centered portrait/action row, command/party remapping, live/persisted HUD toggle, top/bottom/zoom/4K geometry and portrait proportions");
}
