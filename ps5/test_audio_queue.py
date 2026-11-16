#!/usr/bin/env python3
"""Check actual audio queue output and bounded PS5 diagnostics without a device."""
from pathlib import Path
import argparse, subprocess, tempfile
p=argparse.ArgumentParser(description=__doc__);p.add_argument('--sdl-source',required=True,type=Path);a=p.parse_args();r=Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory() as tmp:
 exe=Path(tmp)/'audio-queue'
 subprocess.run(['cc','-std=c11','-O1','-DEGG_PS5','-ffunction-sections','-fdata-sections','-I',str(r),'-I',str(a.sdl_source/'include'),str(r/'ps5/tests/audio_queue.c'),'-Wl,--gc-sections','-o',str(exe)],check=True)
 subprocess.run([str(exe)],check=True,timeout=10)
