#!/usr/bin/env python3
"""Check sprite geometry, transparency and live-palette effects without game art."""
from pathlib import Path
import argparse,subprocess,tempfile
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--sdl-source',type=Path,required=True)
a=p.parse_args();r=Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory() as tmp:
    exe=Path(tmp)/'sprite-hd'
    subprocess.run(['cc','-std=c11','-O1','-Wall','-Wextra','-Werror','-ffunction-sections','-fdata-sections','-I',str(r),'-I',str(a.sdl_source/'include'),str(r/'ps5/tests/sprite_hd.c'),str(r/'runtime/sprite_hd.c'),'-Wl,--gc-sections','-o',str(exe)],check=True)
    subprocess.run([str(exe)],check=True,timeout=10)
