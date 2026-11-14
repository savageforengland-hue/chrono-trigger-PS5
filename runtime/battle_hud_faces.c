/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "battle_hud_faces.h"
#include "gfx_rom.h"
#include <stdlib.h>
#include <stdio.h>
int ct_hud_load_faces(uint32_t portraits[7][48*48]){
    gfx_list_files();GfxBlob tiles=gfx_open(gfx_find("menu/obj/face.NCGR"));
    GfxBlob cells=gfx_open(gfx_find("menu/obj/face.NCER"));GfxTiles t;GfxPalette pal;int ok=0;
    if(!gfx_read_tiles(&tiles,&t)||!gfx_read_palette(gfx_find("menu/obj/face.NCLR"),&pal)||gfx_cell_count(&cells)<7||pal.colors<7*16)goto done;
    for(unsigned c=0;c<7;c++){
        GfxLayout l;if(!gfx_cell_layout(&cells,c,&l))goto done;
        if(l.w!=48||l.h!=48){free(l.tiles);goto done;}
        unsigned char px[48*48]={0};gfx_draw(&l,&t,px);free(l.tiles);
        for(int k=0;k<48*48;k++){
            SDL_Color col=pal.c[c*16+(px[k]&15)];portraits[c][k]=(px[k]&15)?0xff000000u|((uint32_t)col.b<<16)|((uint32_t)col.g<<8)|col.r:0;
        }
    }ok=1;
done:gfx_close(&tiles);gfx_close(&cells);return ok;
}

static int command_icon(const char *name,unsigned cell,unsigned keep,int framed,uint32_t out[32*32]){
    char path[128];snprintf(path,sizeof path,"menu/obj/%s.NCGR",name);
    GfxBlob tb=gfx_open(gfx_find(path));snprintf(path,sizeof path,"menu/obj/%s.NCER",name);
    GfxBlob cb=gfx_open(gfx_find(path));GfxTiles tiles;GfxPalette pal;GfxLayout l;int ok=0;
    snprintf(path,sizeof path,"menu/obj/%s.NCLR",name);
    if(!gfx_read_tiles(&tb,&tiles)||tiles.bpp8||!gfx_read_palette(gfx_find(path),&pal)||!gfx_cell_layout(&cb,cell,&l))goto done;
    if(l.w>32||l.h>32||l.w<=0||l.h<=0){free(l.tiles);goto done;}
    uint8_t px[32*32]={0};gfx_draw(&l,&tiles,px);free(l.tiles);
    /* The command symbols retain the game's pixels; omit the inventory
     * button's bevel/background. Its symbol uses these palette entries. */
    for(int y=0;y<32;y++)for(int x=0;x<32;x++){
        int sx=x*(framed?24:l.w)/32,sy=y*(framed?24:l.h)/32;
        unsigned index=px[sy*l.w+sx]&15;
        int inside=!framed||(sx>=3&&sx<=20&&sy>=3&&sy<=20);
        SDL_Color c=pal.c[index];
        out[y*32+x]=index&&inside&&(keep&(1u<<index))?
            0xff000000u|((uint32_t)c.b<<16)|((uint32_t)c.g<<8)|c.r:0;
    }ok=1;
done:gfx_close(&tb);gfx_close(&cb);return ok;
}
int ct_hud_load_command_icons(uint32_t icons[3][32*32]){
    gfx_list_files();
    return command_icon("icon",20,0xffff,0,icons[0])&& /* unframed weapon sword */
        command_icon("micon",3,(1u<<1)|(1u<<7)|(1u<<8),1,icons[1])&& /* Tech lightning */
        command_icon("micon",2,(1u<<1)|(1u<<7)|(1u<<8)|(1u<<11)|(1u<<13),1,icons[2]); /* bag */
}
