#!/usr/bin/env python3
"""Test authored whole-pose matching; optional private ROM/art integration."""
from pathlib import Path
import argparse,subprocess,tempfile,shutil
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--sdl-source',type=Path,required=True)
p.add_argument('--sdl-library',type=Path)
p.add_argument('--rom',type=Path)
p.add_argument('--data-root',type=Path)
a=p.parse_args();r=Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory() as tmp:
    exe=Path(tmp)/'match'
    subprocess.run(['cc','-std=c11','-O1','-Wall','-Wextra','-Werror','-I',str(r),str(r/'ps5/tests/redraw_match.c'),str(r/'runtime/redraw_match.c'),'-o',str(exe)],check=True)
    subprocess.run([str(exe)],check=True,timeout=10)
    if a.rom:
        if not (a.sdl_library and a.data_root):p.error('--rom also needs --sdl-library and --data-root')
        exe=Path(tmp)/'assets'
        src=['ps5/tests/redraw_assets.c','runtime/redraw.c','runtime/redraw_match.c','runtime/gfx_rom.c','runtime/gfx_mod.c','runtime/sprite_hd.c']
        subprocess.run(['cc','-std=c11','-O1','-ffunction-sections','-fdata-sections','-I',str(r),'-I',str(a.sdl_source/'include'),*[str(r/s) for s in src],str(a.sdl_library),'-Wl,--gc-sections','-lm','-ldl','-pthread','-o',str(exe)],check=True)
        subprocess.run([str(exe),str(a.rom),str(a.data_root.resolve())+'/'],check=True,timeout=60)
        data=Path(tmp)/'data';shutil.copytree(a.data_root/'mods/marle-pilot',data/'mods/marle-pilot')
        (data/'mods/marle-pilot/marle-089.png').unlink()
        subprocess.run([str(exe),str(a.rom),str(data)+'/', 'missing'],check=True,timeout=60)
