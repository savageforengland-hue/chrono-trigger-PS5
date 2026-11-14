#!/usr/bin/env python3
"""Compile actual control labels, prompt adaptation and font drawing on the host."""
import argparse
from pathlib import Path
import subprocess,tempfile

HARNESS=r'''
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "controls.c"
#include "ui_font.h"
#include "video.h"
#include "prompts.h"
const char *SDL_GetGamepadStringForAxis(SDL_GamepadAxis axis){(void)axis;return "axis";}
const char *SDL_GetGamepadStringForButton(SDL_GamepadButton button){(void)button;return "button";}
SDL_GamepadButtonLabel SDL_GetGamepadButtonLabelForType(SDL_GamepadType type,SDL_GamepadButton button){(void)type;(void)button;return SDL_GAMEPAD_BUTTON_LABEL_UNKNOWN;}
int main(void){
 memcpy(binds,defaults,sizeof binds);
 publish_prompt_binds();
 assert(!strcmp(controls_name(CTL_A),"Confirm") && !strcmp(controls_name(CTL_START),"Pause"));
 assert(!strcmp(controls_pad_label(CTL_A),"\xE2\x97\x8B"));
 assert(!strcmp(controls_pad_label(CTL_B),"\xC3\x97"));
 assert(!strcmp(controls_pad_label(CTL_X),"\xE2\x96\xB3"));
 assert(!strcmp(controls_pad_label(CTL_Y),"\xE2\x96\xA1"));
 assert(!strcmp(controls_pad_label(CTL_L),"L1") && !strcmp(controls_pad_label(CTL_R),"R1"));
 assert(!strcmp(controls_pad_label(CTL_SELECT),"Create") && !strcmp(controls_pad_label(CTL_START),"Options"));
 assert(!strcmp(controls_pad_label(CTL_FAST_FORWARD),"R2"));
 char buffer[512];const char *source="A: confirm. B: back. SELECT / START";
 const char *expected="\xE2\x97\x8B: confirm. \xC3\x97: back. Create / Options";
 assert(!strcmp(egg_ps5_prompt_text(source,buffer,sizeof buffer),expected));
 assert(ui_text_width(source)==ui_text_width(expected));
 assert(!strcmp(egg_ps5_prompt_text("SELECTOR STARTER DATA: ABXY",buffer,sizeof buffer),"SELECTOR STARTER DATA: ABXY"));
 assert(egg_ps5_prompt_text(source,buffer,1)==source);
 binds[CTL_A].pad=SDL_GAMEPAD_BUTTON_SOUTH;
 binds[CTL_B].pad=SDL_GAMEPAD_BUTTON_NORTH;
 publish_prompt_binds();
 assert(!strcmp(egg_ps5_prompt_text("A: confirm B: back",buffer,sizeof buffer),"\xC3\x97: confirm \xE2\x96\xB3: back"));
 const char *symbols[]={"\xC3\x97","\xE2\x97\x8B","\xE2\x96\xA1","\xE2\x96\xB3"};
 const size_t pixels=256*VIDEO_ROWS_MAX;
 uint32_t *storage=calloc(pixels+32,sizeof *storage);assert(storage);
 for(unsigned i=0;i<16;i++)storage[i]=storage[pixels+16+i]=0xdeadbeefu;
 uint32_t *screen=storage+16;
 for(unsigned k=0;k<4;k++){
  memset(screen,0,pixels*4);assert(ui_text_width(symbols[k])==7);
  ui_draw_text_flat(screen,symbols[k],4,4,0xffffffffu);
  unsigned count=0;for(size_t i=0;i<pixels;i++)count+=screen[i]!=0;
  assert(count>=13 && count<=30);
 }
 memset(screen,0,pixels*4);ui_draw_text_flat(screen,"@",0,0,0xffffffffu);
 for(size_t i=0;i<pixels;i++)assert(screen[i]==0); /* unknown remains blank */
 ui_draw_text(screen,source,250,VIDEO_ROWS_MAX-2,0xffffffffu);
 for(unsigned i=0;i<16;i++)assert(storage[i]==0xdeadbeefu && storage[pixels+16+i]==0xdeadbeefu);
 free(storage);
 puts("PASS: real DualSense labels, binding-aware prompt conversion, token boundaries, short buffers, font width/glyphs and clipped pixel guards");
}
'''
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--sdl-source',type=Path,required=True);a=p.parse_args()
 here=Path(__file__).resolve().parent
 with tempfile.TemporaryDirectory() as tmp:
  root=Path(tmp);source=root/'test.c';source.write_text(HARNESS);exe=root/'test'
  subprocess.run(['cc','-D_GNU_SOURCE','-DEGG_PS5','-std=c11','-O1','-ffunction-sections','-fdata-sections','-I',str(here),'-I',str(here.parent/'runtime'),'-I',str(a.sdl_source/'include'),str(source),str(here/'prompts.c'),str(here.parent/'runtime/ui_font.c'),'-Wl,--gc-sections','-o',str(exe)],check=True)
  subprocess.run([str(exe)],check=True,timeout=10)
if __name__=='__main__':main()
