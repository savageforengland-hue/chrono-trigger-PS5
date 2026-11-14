#!/usr/bin/env python3
"""Check presentation geometry and the actual decorative-panel draw routine."""
from pathlib import Path
import subprocess,tempfile

PREFIX=r'''
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include "presentation.h"
typedef unsigned char Uint8;
typedef int SDL_BlendMode;
typedef struct {float x,y,w,h;} SDL_FRect;
typedef struct {int unused;} SDL_Renderer;
typedef struct {int blend;Uint8 alpha;} SDL_Texture;
typedef struct {SDL_Renderer *ren;} View;
enum {SET_SCREEN_FIT,SET_ASPECT,ASPECT_ORIGINAL=0};
#define SDL_BLENDMODE_BLEND 1
static int settings[2]={1,2},fills,copies;
static int menu_open;
static int sysmenu_is_open(void){return menu_open;}
#define SETTING(i) settings[i]
static SDL_Renderer renderer;
static View primary={&renderer};
static SDL_FRect drawn[2];
static void SDL_GetRenderOutputSize(SDL_Renderer *r,int *w,int *h){(void)r;*w=3840;*h=2160;}
static void SDL_GetTextureBlendMode(SDL_Texture *t,SDL_BlendMode *b){*b=t->blend;}
static void SDL_GetTextureAlphaMod(SDL_Texture *t,Uint8 *a){*a=t->alpha;}
static void SDL_SetTextureBlendMode(SDL_Texture *t,SDL_BlendMode b){t->blend=b;}
static void SDL_SetTextureAlphaMod(SDL_Texture *t,Uint8 a){t->alpha=a;}
static void SDL_SetRenderDrawColor(SDL_Renderer *r,int a,int b,int c,int d){(void)r;(void)a;(void)b;(void)c;(void)d;}
static void SDL_RenderFillRect(SDL_Renderer *r,const SDL_FRect *p){(void)r;assert(fills<2);drawn[fills++]=*p;}
static void SDL_RenderTexture(SDL_Renderer *r,SDL_Texture *t,const SDL_FRect *src,const SDL_FRect *dst){
 (void)r;assert(t->alpha==72 && t->blend==SDL_BLENDMODE_BLEND);assert(src->w==1 && src->h==576);assert(dst->w>0);copies++;
}
'''
HARNESS=r'''
static int near(float a,float b){return fabsf(a-b)<0.1f;}
int main(void){
 CtPresentRect normal=ctps5_present_rect(3840,2160,256,192,0,0,1);
 assert(near(normal.x,480)&&near(normal.y,0)&&near(normal.w,2880)&&near(normal.h,2160));
 CtPresentRect framed=ctps5_present_rect(3840,2160,256,192,1,0,1);
 assert(near(framed.x,normal.x)&&near(framed.w,normal.w));
 CtPresentRect crop=ctps5_present_rect(3840,2160,256,192,2,0,1);
 assert(near(crop.x,0)&&near(crop.y,-360)&&near(crop.w,3840)&&near(crop.h,2880));
 CtPresentRect stretch=ctps5_present_rect(3840,2160,256,192,3,0,1);
 assert(near(stretch.w,3840)&&near(stretch.h,2160));
 for(int mode=0;mode<4;mode++){
  CtPresentRect wide=ctps5_present_rect(3840,2160,340,192,mode,0,1);
  assert(near(wide.h,2160)&&near(wide.w,3825));
  CtPresentRect off=ctps5_present_rect(3840,2160,256,192,mode,0,0);
  assert(near(off.x,normal.x)&&near(off.w,normal.w));
 }
 CtPresentRect pixel=ctps5_present_rect(1920,1080,256,192,1,1,1);
 assert(near(pixel.w,1280)&&near(pixel.h,960)&&near(pixel.x,320)&&near(pixel.y,60));
 CtPresentRect small=ctps5_present_rect(128,96,256,192,0,1,1);
 assert(near(small.w,128)&&near(small.h,96));
 assert(ctps5_present_rect(0,0,0,0,0,0,0).w==0);
 SDL_Texture texture={7,93};SDL_FRect target={normal.x,normal.y,normal.w,normal.h};
 draw_fixed_screen_panels(&primary,&texture,768,576,&target);
 assert(fills==2&&copies==0&&texture.blend==7&&texture.alpha==93);
 assert(near(drawn[0].x,0)&&near(drawn[0].w,480)&&near(drawn[1].x,3360)&&near(drawn[1].w,480));
 settings[SET_SCREEN_FIT]=0;fills=copies=0;draw_fixed_screen_panels(&primary,&texture,768,576,&target);assert(!fills&&!copies);
 settings[SET_SCREEN_FIT]=2;menu_open=1;draw_fixed_screen_panels(&primary,&texture,768,576,&target);assert(fills==2&&copies==0);
 fills=copies=0;menu_open=0;
 settings[SET_SCREEN_FIT]=1;settings[SET_ASPECT]=ASPECT_ORIGINAL;draw_fixed_screen_panels(&primary,&texture,768,576,&target);assert(!fills&&!copies);
 puts("PASS: proportional framing, crop/stretch choices, native-wide and original-aspect preservation, integer scaling, clean panel bounds, no scene-edge copies and unchanged texture state");
}
'''
def main():
 here=Path(__file__).resolve().parent
 text=(here.parent/'runtime/display.c').read_text();start=text.index('static void draw_fixed_screen_panels(');body=text.index('{',start);depth=0
 for i in range(body,len(text)):
  if text[i]=='{':depth+=1
  elif text[i]=='}':
   depth-=1
   if not depth:break
 function=text[start:i+1]
 with tempfile.TemporaryDirectory() as tmp:
  root=Path(tmp);source=root/'test.c';source.write_text(PREFIX+function+HARNESS);exe=root/'test'
  subprocess.run(['cc','-std=c11','-O1','-I',str(here),str(source),str(here/'presentation.c'),'-lm','-o',str(exe)],check=True)
  subprocess.run([str(exe)],check=True,timeout=10)
if __name__=='__main__':main()
