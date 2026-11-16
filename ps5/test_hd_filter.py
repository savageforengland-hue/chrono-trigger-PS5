#!/usr/bin/env python3
"""Check HD upload sampling and unchanged scene pixels with actual SDL."""
from pathlib import Path
import argparse,subprocess,tempfile
p=argparse.ArgumentParser(description=__doc__);p.add_argument('--sdl-source',type=Path,required=True);p.add_argument('--sdl-library',type=Path,required=True);a=p.parse_args();r=Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory() as tmp:
 exe=Path(tmp)/'hd-filter'
 subprocess.run(['cc','-std=c11','-O1','-ffunction-sections','-fdata-sections','-I',str(r),'-I',str(a.sdl_source/'include'),str(r/'ps5/tests/hd_filter.c'),str(a.sdl_library),'-Wl,--gc-sections','-lm','-ldl','-pthread','-o',str(exe)],check=True)
 subprocess.run([str(exe)],check=True,timeout=20)
