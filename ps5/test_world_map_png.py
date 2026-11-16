#!/usr/bin/env python3
"""Verify world-map PNG support against actual decoding and private player ROM."""
from pathlib import Path
import argparse,subprocess,tempfile
p=argparse.ArgumentParser(description=__doc__)
for n in ['sdl-source','sdl-library','rom']:p.add_argument('--'+n,type=Path,required=True)
a=p.parse_args();r=Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory() as tmp:
 d=Path(tmp);exe=d/'world-png'
 subprocess.run(['cc','-std=c11','-O1','-DEGG_PS5_CORE','-ffunction-sections','-fdata-sections','-I',str(r),'-I',str(a.sdl_source/'include'),str(r/'ps5/tests/world_map_png.c'),str(r/'runtime/gfx_rom.c'),str(r/'runtime/gfx_mod.c'),str(a.sdl_library),'-Wl,--gc-sections','-lm','-ldl','-pthread','-o',str(exe)],check=True)
 for variant in ['valid','rgba','empty','bad','zero']:
  folder=d/variant;folder.mkdir();subprocess.run([str(exe),str(a.rom),str(folder)+'/',variant],check=True,timeout=20)
