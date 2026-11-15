/* SPDX-License-Identifier: GPL-3.0-or-later
 * Render the real HUD module using menu faces from your own USA ROM.
 * Example statistics are authored preview values, not a gameplay capture.
 * See docs/PORTRAIT-HUD.md for the build command. */
#include "battle_hud_faces.h"
#include "battle_hud_draw.h"
#include "rt.h"
#include <stdio.h>
#include <stdlib.h>
RtRom rt_rom;
int main(int argc,char**argv){
    if(argc!=3&&argc!=4){fprintf(stderr,"Usage: %s own-CT.nds hud.ppm [commands.ppm]\n",argv[0]);return 2;}
    FILE*f=fopen(argv[1],"rb");if(!f)return 3;
    if(fseek(f,0,SEEK_END)){fclose(f);return 3;}
    long size=ftell(f);if(size<512||size>512*1024*1024L){fclose(f);return 3;}
    rewind(f);rt_rom.size=(size_t)size;rt_rom.data=malloc(rt_rom.size);
    if(!rt_rom.data){fclose(f);return 3;}
    size_t read=fread(rt_rom.data,1,rt_rom.size,f);fclose(f);
    if(read!=rt_rom.size||memcmp(rt_rom.data+12,"YQUE",4)){free(rt_rom.data);return 3;}
    uint32_t faces[7][48*48]={0};
    if(!ct_hud_load_faces(faces)){free(rt_rom.data);return 4;}
    CtHudState s={.member={{1,0,742,850,52,75,1000,1000,"Crono"},
        {1,1,516,640,81,95,610,1000,"Marle"},{1,2,398,610,47,90,280,1000,"Lucca"}},.selected=0,.command=1};
    uint32_t skin[80*32];int has_skin=ct_hud_load_window_skin(skin);
    uint32_t p[CT_HUD_W*CT_HUD_H];ct_hud_draw(p,&s,faces,has_skin?skin:NULL);
    f=fopen(argv[2],"wb");if(!f){free(rt_rom.data);return 5;}
    fprintf(f,"P6\n%d %d\n255\n",CT_HUD_W,CT_HUD_H);
    for(int k=0;k<CT_HUD_W*CT_HUD_H;k++){
        fputc(p[k]&255,f);fputc((p[k]>>8)&255,f);fputc((p[k]>>16)&255,f);
    }
    int ok=!ferror(f);if(fclose(f))ok=0;
    if(ok&&argc==4){
        uint32_t icons[3][32*32],commands[CT_CMD_W*CT_CMD_H];
        if(!ct_hud_load_command_icons(icons)){free(rt_rom.data);return 4;}
        ct_hud_draw_commands(commands,s.command,icons);
        f=fopen(argv[3],"wb");if(!f){free(rt_rom.data);return 5;}
        fprintf(f,"P6\n%d %d\n255\n",CT_CMD_W,CT_CMD_H);
        for(int i=0;i<CT_CMD_W*CT_CMD_H;i++){
            fputc(commands[i]&255,f);fputc((commands[i]>>8)&255,f);fputc((commands[i]>>16)&255,f);
        }
        if(ferror(f))ok=0;if(fclose(f))ok=0;
    }
    free(rt_rom.data);return ok?0:5;
}
