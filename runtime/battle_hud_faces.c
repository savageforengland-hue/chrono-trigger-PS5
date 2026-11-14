/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "battle_hud_faces.h"
#include "gfx_rom.h"
#include <stdlib.h>
int ct_hud_load_faces(uint32_t portraits[7][48*48]){
    gfx_list_files();GfxBlob tiles=gfx_open(gfx_find("menu/obj/face.NCGR"));
    GfxBlob cells=gfx_open(gfx_find("menu/obj/face.NCER"));GfxTiles t;GfxPalette pal;int ok=0;
    if(!gfx_read_tiles(&tiles,&t)||!gfx_read_palette(gfx_find("menu/obj/face.NCLR"),&pal)||gfx_cell_count(&cells)<7)goto done;
    for(unsigned c=0;c<7;c++){
        GfxLayout l;if(!gfx_cell_layout(&cells,c,&l))goto done;
        if(l.w!=48||l.h!=48){free(l.tiles);goto done;}
        unsigned char px[48*48]={0};gfx_draw(&l,&t,px);free(l.tiles);
        for(int k=0;k<48*48;k++){
            SDL_Color col=pal.c[px[k]];portraits[c][k]=px[k]?0xff000000u|((uint32_t)col.b<<16)|((uint32_t)col.g<<8)|col.r:0;
        }
    }ok=1;
done:gfx_close(&tiles);gfx_close(&cells);return ok;
}
