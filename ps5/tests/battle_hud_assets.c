/* SPDX-License-Identifier: GPL-3.0-or-later
 * Authored indexed tiles/palette exercise the real decoder without game art. */
#include "runtime/battle_hud_faces.h"
#include "runtime/gfx_rom.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
static GfxFile files[12];static int used,palette_size=128,invalid_window;
int gfx_list_files(void){return 1;}
const GfxFile *gfx_find(const char*name){for(int i=0;i<used;i++)if(!strcmp(files[i].path,name))return files+i;assert(used<12);strcpy(files[used].path,name);return files+used++;}
GfxBlob gfx_open(const GfxFile*f){return (GfxBlob){.data=(const uint8_t*)f};}
void gfx_close(GfxBlob*b){(void)b;}
int gfx_read_tiles(const GfxBlob*b,GfxTiles*t){(void)b;memset(t,0,sizeof *t);return 1;}
int gfx_read_palette(const GfxFile*f,GfxPalette*p){(void)f;memset(p,0,sizeof *p);p->colors=palette_size;for(int i=0;i<256;i++)p->c[i]=(SDL_Color){i,i+1,i+2,255};return 1;}
uint32_t gfx_cell_count(const GfxBlob*b){(void)b;return 8;}
int gfx_cell_layout(const GfxBlob*b,uint32_t cell,GfxLayout*l){
 const GfxFile*f=(const GfxFile*)b->data;int face=strstr(f->path,"face")!=NULL,micon=strstr(f->path,"micon")!=NULL;
 memset(l,0,sizeof *l);l->w=l->h=face?48:micon?32:16;
 if(strstr(f->path,"obj_win_btl")){l->w=invalid_window?79:80;l->h=32;}
 l->count=1;l->tiles=calloc(1,sizeof *l->tiles);assert(l->tiles);
 l->tiles[0].off=face?0x100:cell;return 1;
}
void gfx_draw(const GfxLayout*l,const GfxTiles*t,uint8_t*px){
 (void)t;if(l->w==48){memset(px,1,48*48);px[1]=0;return;}
 if(l->w==80){memset(px,3,80*32);px[0]=0;px[80*32-1]=15;return;}
 memset(px,2,l->w*l->h);px[10*l->w+10]=1;px[10*l->w+11]=13;
 if(l->w==16){memset(px,0,16*16);px[8*16+8]=15;}
}
static uint32_t color(unsigned i){return 0xff000000u|i|((i+1)<<8)|((i+2)<<16);}
int main(void){
 uint32_t faces[7][48*48],icons[3][32*32],skin[80*32];assert(ct_hud_load_faces(faces));
 for(int c=0;c<7;c++){assert(faces[c][0]==color(c*16+1));assert(faces[c][1]==0);}
 palette_size=16;assert(!ct_hud_load_faces(faces));palette_size=128;
 assert(ct_hud_load_command_icons(icons));
 for(int c=0;c<3;c++){assert(icons[c][0]==0);int opaque=0;for(int i=0;i<32*32;i++)opaque+=icons[c][i]!=0;assert(opaque);}
 int bag_color=0;for(int i=0;i<32*32;i++){assert(icons[1][i]!=color(13));if(icons[2][i]==color(13))bag_color=1;}assert(bag_color);
 assert(ct_hud_load_window_skin(skin));assert(skin[0]==0&&skin[1]==color(3)&&skin[80*32-1]==color(15));
 invalid_window=1;assert(!ct_hud_load_window_skin(skin));invalid_window=0;
 palette_size=8;assert(!ct_hud_load_window_skin(skin));
 puts("PASS: seven portrait palette banks, alpha, background-free sword/Tech/bag, original window decoding and invalid window/palette fallback");
}
