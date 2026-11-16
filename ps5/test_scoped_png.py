#!/usr/bin/env python3
"""Exercise optional room-scoped PNG loading and live lighting with actual SDL."""
from pathlib import Path
import argparse,subprocess,tempfile
p=argparse.ArgumentParser(description=__doc__);p.add_argument('--sdl-source',type=Path,required=True);p.add_argument('--sdl-library',type=Path,required=True);p.add_argument('--rom',type=Path,required=True);a=p.parse_args();r=Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory() as tmp:
 d=Path(tmp);common=['cc','-std=c11','-O1','-ffunction-sections','-fdata-sections','-I',str(r),'-I',str(a.sdl_source/'include')]
 subprocess.run([*common,str(r/'ps5/tests/background_rules.c'),'-o',str(d/'rules')],check=True);subprocess.run([str(d/'rules')],check=True)
 subprocess.run([*common,'-DEGG_PS5_CORE',str(r/'ps5/tests/scoped_png.c'),str(r/'runtime/gfx_mod.c'),str(r/'runtime/gfx_rom.c'),str(a.sdl_library),'-Wl,--gc-sections','-lm','-ldl','-pthread','-o',str(d/'scoped')],check=True)
 subprocess.run([*common,str(r/'ps5/tests/room_lighting.c'),str(a.sdl_library),'-Wl,--gc-sections','-lm','-ldl','-pthread','-o',str(d/'lighting')],check=True);subprocess.run([str(d/'lighting')],check=True)
 for case in ['valid','empty','invalid','native','plain']:
  root=d/case;root.mkdir();subprocess.run([str(d/'scoped'),str(a.rom),str(root)+'/',case],check=True,timeout=30)
