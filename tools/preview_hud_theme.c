/* SPDX-License-Identifier: GPL-3.0-or-later
 * Local renderer preview, using the player's own supported ROM. */
#include "hud_theme.h"
#include "battle_hud_faces.h"
#include "rt.h"
#include <stdio.h>
#include <stdlib.h>
RtRom rt_rom;
int main(int argc,char**argv){
 if(argc!=4){fprintf(stderr,"Usage: %s own-CT.nds theme-folder output.png\n",argv[0]);return 2;}
 FILE*f=fopen(argv[1],"rb");if(!f)return 3;fseek(f,0,SEEK_END);long n=ftell(f);rewind(f);if(n<512||n>512*1024*1024L){fclose(f);return 3;}rt_rom.size=(size_t)n;rt_rom.data=malloc(rt_rom.size);if(!rt_rom.data){fclose(f);return 3;}int good=fread(rt_rom.data,1,rt_rom.size,f)==rt_rom.size;fclose(f);if(!good||memcmp(rt_rom.data+12,"YQUE",4))return 3;
 uint32_t faces[7][48*48],skin[80*32];if(!ct_hud_load_faces(faces))return 4;int has_skin=ct_hud_load_window_skin(skin);char error[192];CtHudTheme*t=ct_hud_theme_load(argv[2],NULL,error,sizeof error,NULL);if(!t){fprintf(stderr,"Theme: %s\n",error);return 5;}
 CtHudState s={.member={{1,0,742,850,52,75,1000,1000,"Crono"},{1,1,516,640,81,95,610,1000,"Marle"},{1,2,398,610,47,90,280,1000,"Lucca"}},.selected=0,.command=1};
 const uint32_t*p=ct_hud_theme_draw(t,&s,faces,has_skin?skin:NULL);SDL_Surface*image=SDL_CreateSurfaceFrom(t->pixel_w,t->pixel_h,SDL_PIXELFORMAT_ABGR8888,(void*)p,t->pixel_w*4);good=image&&SDL_SavePNG(image,argv[3]);SDL_DestroySurface(image);ct_hud_theme_free(t);free(rt_rom.data);return good?0:6;
}
