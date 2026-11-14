#!/usr/bin/env python3
"""Exercise actual display setting load/save and the one-time CTPS5 profile."""
import argparse
from pathlib import Path
import subprocess,tempfile
HARNESS=r'''
#include <assert.h>
#include <stdarg.h>
#include "display.c"
static int log_count;
void SDL_Log(const char *format,...){assert(strstr(format,"CTPS5 widescreen profile"));log_count++;}
int main(int argc,char **argv){
 assert(argc==2);snprintf(ini_path,sizeof ini_path,"%s/settings.ini",argv[1]);
 settings[SET_SHADER]=(Setting){"shader","Shader",1,0,off_names,NULL};
 FILE *f=fopen(ini_path,"w");assert(f);
 fputs("aspect = original\nfillsides = no\nscreenfit = crop\nminimap = off\n",f);fclose(f);
 load_settings();assert(!ct_wide_profile && SETTING(SET_ASPECT)==ASPECT_ORIGINAL && SETTING(SET_FILL)==0 && SETTING(SET_MINIMAP)==0);
 assert(apply_ctps5_wide_profile()==1 && log_count==1);
 assert(SETTING(SET_ASPECT)==ASPECT_16_9 && SETTING(SET_FILL)==1 && SETTING(SET_SCREEN_FIT)==1 && SETTING(SET_MINIMAP)==0);
 assert(apply_ctps5_wide_profile()==0 && log_count==1);
 save_settings();ct_wide_profile=0;SETTING(SET_ASPECT)=ASPECT_ORIGINAL;load_settings();
 assert(ct_wide_profile==1 && SETTING(SET_ASPECT)==ASPECT_16_9);
 SETTING(SET_ASPECT)=ASPECT_ORIGINAL;SETTING(SET_FILL)=0;SETTING(SET_SCREEN_FIT)=2;save_settings();
 ct_wide_profile=0;SETTING(SET_FILL)=1;SETTING(SET_SCREEN_FIT)=1;load_settings();
 assert(!apply_ctps5_wide_profile());
 assert(SETTING(SET_ASPECT)==ASPECT_ORIGINAL && SETTING(SET_FILL)==0 && SETTING(SET_SCREEN_FIT)==2 && SETTING(SET_MINIMAP)==0);
 puts("PASS: actual legacy settings migrate once; unrelated settings and subsequent user choices survive save/reload");
}
'''
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--sdl-source',type=Path,required=True);a=p.parse_args()
 here=Path(__file__).resolve().parent
 with tempfile.TemporaryDirectory() as tmp:
  root=Path(tmp);source=root/'test.c';source.write_text(HARNESS);exe=root/'test'
  subprocess.run(['cc','-D_GNU_SOURCE','-DEGG_PS5_CORE','-DEGG_SINGLE_SCREEN=1','-std=c11','-O1','-ffunction-sections','-fdata-sections','-I',str(here.parent/'runtime'),'-I',str(a.sdl_source/'include'),str(source),'-Wl,--gc-sections','-o',str(exe)],check=True)
  subprocess.run([str(exe),str(root)],check=True,timeout=10)
if __name__=='__main__':main()
