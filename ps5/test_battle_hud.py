#!/usr/bin/env python3
"""Exercise the real read-only battle snapshot and renderer with authored RAM."""
from pathlib import Path
import argparse,subprocess,tempfile
ROOT=Path(__file__).resolve().parents[1]
HARNESS=r'''
#include "battle_hud_state.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
static unsigned char ram[0x400000];
static void w8(unsigned a,unsigned v){ram[a-0x2000000]=(unsigned char)v;}
static void w16(unsigned a,unsigned v){w8(a,v);w8(a+1,v>>8);}
static void w32(unsigned a,unsigned v){w16(a,v);w16(a+2,v>>16);}
static void setup(void){
 memset(ram,0,sizeof ram);w32(0x207088c,0x2200000);w32(0x2200000,0x2115558);
 w32(0x20728fc,0x2200100);w32(0x2118c0c,0x2210000);
 for(unsigned i=0;i<3;i++){
  w8(0x2200100+i,i);unsigned b=0x2210000+i*0x80;
  w16(b+3,500+i);w16(b+5,9999);w16(b+7,30+i);w16(b+9,99);
  w8(0x2211545+i,1);w8(0x22115d2+i,25*i);w8(0x22115e0+i,100);
 }
}
int main(void){
 CtHudState s;setup();unsigned char original[sizeof ram];memcpy(original,ram,sizeof ram);
 assert(ct_hud_read_state(&s,ram,sizeof ram));assert(!memcmp(ram,original,sizeof ram));
 for(int i=0;i<3;i++){
  assert(s.member[i].present&&s.member[i].character==i);assert(s.member[i].hp==500u+i);
  assert(s.member[i].max_hp==9999&&s.member[i].mp==30u+i&&s.member[i].max_mp==99);
  assert(s.member[i].atb==100u-25u*i&&s.member[i].atb_max==100);
 }
 assert(s.selected==0);w8(0x22114eb,2);assert(ct_hud_read_state(&s,ram,sizeof ram)&&s.selected==2);
 w8(0x22114eb,9);assert(ct_hud_read_state(&s,ram,sizeof ram)&&s.selected==-1);
 for(unsigned c=0;c<7;c++){w8(0x2200100,c);w8(0x2200101,0x80);w8(0x2200102,0x80);assert(ct_hud_read_state(&s,ram,sizeof ram));assert(s.member[0].character==(int)c);assert(!s.member[1].present&&!s.member[2].present);}
 setup();w16(0x2210003,0);assert(ct_hud_read_state(&s,ram,sizeof ram));assert(s.member[0].atb==0);
 w8(0x22115d2,200);assert(ct_hud_read_state(&s,ram,sizeof ram));assert(s.member[0].atb==0);
 setup();w8(0x22115e0,0);assert(ct_hud_read_state(&s,ram,sizeof ram));assert(s.member[0].atb_max==0);
 setup();w32(0x2118c0c,0x23ffffe);assert(!ct_hud_read_state(&s,ram,sizeof ram));assert(!s.member[0].present);
 setup();w32(0x20728fc,0xffffffff);assert(!ct_hud_read_state(&s,ram,sizeof ram));
 setup();w32(0x2200000,0);assert(!ct_hud_read_state(&s,ram,sizeof ram));
 setup();w8(0x2200101,0);assert(!ct_hud_read_state(&s,ram,sizeof ram));
 setup();w8(0x2200100,7);assert(!ct_hud_read_state(&s,ram,sizeof ram));
 setup();w16(0x2210005,0);assert(!ct_hud_read_state(&s,ram,sizeof ram));
 setup();w16(0x2210007,100);assert(!ct_hud_read_state(&s,ram,sizeof ram));
 assert(!ct_hud_read_state(&s,ram,3));assert(!ct_hud_read_state(NULL,ram,sizeof ram));
 setup();assert(ct_hud_read_state(&s,ram,sizeof ram));
 uint32_t guarded[CT_HUD_W*CT_HUD_H+32],faces[7][48*48];
 for(unsigned i=0;i<sizeof guarded/4;i++)guarded[i]=0xdeadbeef;
 memset(faces,0,sizeof faces);for(int c=0;c<7;c++)for(int i=0;i<48*48;i++)faces[c][i]=0xff204080u+c;
 ct_hud_draw(guarded+16,&s,faces);
 for(int i=0;i<16;i++)assert(guarded[i]==0xdeadbeef&&guarded[16+CT_HUD_W*CT_HUD_H+i]==0xdeadbeef);
 uint32_t*p=guarded+16;
 assert(p[24*CT_HUD_W+5]==faces[0][0]);assert(p[24*CT_HUD_W+165]==faces[1][0]);assert(p[24*CT_HUD_W+325]==faces[2][0]);
 s.member[0].hp=0;ct_hud_draw(p,&s,faces);unsigned color=p[24*CT_HUD_W+5];assert((color&255)==((color>>8)&255)&&((color>>8)&255)==((color>>16)&255));
 s.member[0].present=0;ct_hud_draw(p,&s,faces);assert(p[24*CT_HUD_W+5]!=faces[0][0]);
 puts("PASS: live unaligned HP/MP, all seven party identities, ATB countdown/ready/KO, empty slots, invalid pointers/state, immutable RAM and renderer guards");
}
'''
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--sdl-source',type=Path)
args=parser.parse_args()
with tempfile.TemporaryDirectory() as tmp:
 p=Path(tmp);(p/'check.c').write_text(HARNESS)
 subprocess.run(['cc','-std=c11','-O1','-Wall','-Wextra','-Werror','-I',str(ROOT/'runtime'),str(p/'check.c'),str(ROOT/'runtime/battle_hud_state.c'),str(ROOT/'runtime/battle_hud_draw.c'),str(ROOT/'runtime/ui_font.c'),'-o',str(p/'check')],check=True)
 subprocess.run([str(p/'check')],check=True,timeout=10)

 if args.sdl_source:
  subprocess.run(['cc','-std=c11','-O1','-I',str(ROOT),'-I',str(args.sdl_source/'include'),str(ROOT/'ps5/tests/battle_hud_bridge.c'),str(ROOT/'ps5/battle_hud.c'),str(ROOT/'runtime/battle_hud_state.c'),str(ROOT/'runtime/battle_hud_draw.c'),str(ROOT/'runtime/ui_font.c'),'-Wl,--wrap=fopen','-lm','-o',str(p/'bridge')],check=True)
  subprocess.run([str(p/'bridge')],check=True,timeout=10)
