#!/usr/bin/env python3
"""Test real SDL PNG decoding and the configurable HUD renderer."""
from pathlib import Path
import argparse,subprocess,tempfile
p=argparse.ArgumentParser(description=__doc__);p.add_argument('--sdl-source',type=Path,required=True);p.add_argument('--sdl-library',type=Path,required=True);a=p.parse_args();r=Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory() as tmp:
 exe=Path(tmp)/'themes';data=Path(tmp)/'data';data.mkdir()
 subprocess.run(['cc','-std=c11','-O1','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-I',str(r),'-I',str(a.sdl_source/'include'),str(r/'ps5/tests/hud_theme.c'),str(r/'runtime/hud_theme.c'),str(r/'runtime/ui_font.c'),str(a.sdl_library),'-lm','-ldl','-pthread','-o',str(exe)],check=True)
 subprocess.run([str(exe),str(data),str(r/'mods/battle-hud/themes/remaster')],check=True,timeout=30)

with tempfile.TemporaryDirectory() as tmp:
 exe=Path(tmp)/'service'
 subprocess.run(['cc','-std=c11','-O1','-I',str(r),'-I',str(a.sdl_source/'include'),*[str(r/s) for s in ['ps5/tests/hud_theme_service.c','ps5/hud_themes.c','ps5/config_file.c','runtime/hud_theme.c','runtime/ui_font.c']],*[f'-Wl,--wrap={s}' for s in ['fopen','opendir','mkdir','rename','unlink','SDL_GetPathInfo']],str(a.sdl_library),'-lm','-ldl','-pthread','-o',str(exe)],check=True)
 subprocess.run([str(exe),tmp],check=True,timeout=30)
