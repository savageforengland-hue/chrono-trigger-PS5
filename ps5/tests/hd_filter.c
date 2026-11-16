/* SPDX-License-Identifier: GPL-3.0-or-later
 * Exercise the real HD upload path with authored scene pixels. A single
 * sprite override must not change the first-stage sampling of other pixels. */
#include "runtime/display.c"
#include <assert.h>
const GfxHdTile*gfxmod_tile(uint32_t id){(void)id;assert(0);return NULL;}
uint32_t ct_redraw_color(uint32_t texel,const uint16_t palette[16]){(void)texel;(void)palette;assert(0);return 0;}
uint32_t sprite_hd_color(uint32_t texel,const uint16_t*palette){(void)texel;(void)palette;assert(0);return 0;}
int main(void){
 assert(SDL_Init(0));hd_workers=0;
 enum{W=SCREEN_W,H=SCREEN_H,N=3};SDL_Surface*output=SDL_CreateSurface(W*N,H*N,SDL_PIXELFORMAT_ABGR8888);assert(output);SDL_Renderer*ren=SDL_CreateSoftwareRenderer(output);assert(ren);View v={.ren=ren};
 uint32_t*px=malloc(W*H*4);VideoHd*source=calloc(W*H,sizeof *source);assert(px&&source);for(int y=0;y<H;y++)for(int x=0;x<W;x++)px[y*W+x]=0xff000000u|(unsigned)((x*71+y*17)&255)|(unsigned)((x*13+y*97)&255)<<8|(unsigned)((x*113+y*37)&255)<<16;
 VideoHdFrame frame={0};hd_frame=&frame;settings[SET_COLOR].value=0;
 const int sequence[]={FILTER_PIXEL,FILTER_SHARP,FILTER_SMOOTH,FILTER_LCD,FILTER_SHARP};SDL_Texture*previous=NULL;
 for(unsigned i=0;i<SDL_arraysize(sequence);i++){
  settings[SET_FILTER].value=sequence[i];assert(upload_hd(&v,0,px,source,2));SDL_ScaleMode mode;assert(SDL_GetTextureScaleMode(v.hd_tex[0],&mode));assert(mode==(sequence[i]==FILTER_SMOOTH?SDL_SCALEMODE_LINEAR:SDL_SCALEMODE_NEAREST));if(previous)assert(previous==v.hd_tex[0]);previous=v.hd_tex[0];
  assert(SDL_SetRenderDrawColor(ren,0,0,0,255)&&SDL_RenderClear(ren));assert(SDL_RenderTexture(ren,v.hd_tex[0],&(SDL_FRect){0,0,W*2,H*2},&(SDL_FRect){0,0,W*N,H*N}));assert(SDL_RenderPresent(ren));
  unsigned differences=0;for(int y=0;y<H*N;y++)for(int x=0;x<W*N;x++){uint32_t actual=((uint32_t*)((uint8_t*)output->pixels+y*output->pitch))[x];differences+=actual!=px[(y/N)*W+x/N];}
  if(sequence[i]==FILTER_SMOOTH)assert(differences>W*H);else assert(differences==0);
 }
 SDL_DestroyTexture(v.hd_tex[0]);SDL_DestroyRenderer(ren);SDL_DestroySurface(output);free(px);free(source);SDL_free(hd_px);SDL_Quit();puts("PASS: real HD upload preserves unmodified scene pixels for Pixel/Sharp/LCD, Smooth retains interpolation, live filter changes reuse and update the same texture");
}
