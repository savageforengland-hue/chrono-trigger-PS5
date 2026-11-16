#!/usr/bin/env python3
"""Exercise real PNG discovery without JSON, exact party identity and fallback."""
from pathlib import Path
import argparse,subprocess,tempfile,json
p=argparse.ArgumentParser(description=__doc__);p.add_argument('--sdl-source',type=Path,required=True);p.add_argument('--sdl-library',type=Path,required=True);p.add_argument('--rom',type=Path,required=True);a=p.parse_args();r=Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory() as tmp:
 exe=Path(tmp)/'dropin';sources=['ps5/tests/png_dropin.c','runtime/redraw.c','runtime/redraw_manifest.c','runtime/redraw_match.c','runtime/gfx_rom.c','runtime/gfx_mod.c','runtime/sprite_hd.c']
 subprocess.run(['cc','-std=c11','-O1','-DEGG_PS5_CORE','-ffunction-sections','-fdata-sections','-I',str(r),'-I',str(a.sdl_source/'include'),*[str(r/s) for s in sources],str(a.sdl_library),'-Wl,--gc-sections','-lm','-ldl','-pthread','-o',str(exe)],check=True)
 for variant in ['empty','bad','rgba','indexed']:
  data=Path(tmp)/variant;data.mkdir();subprocess.run([str(exe),str(a.rom),str(data)+'/',variant],check=True,timeout=60)
  m=json.loads((data/'mods/graphics/manifest.generated.json').read_text());assert m['missing']=='original-game-assets' and m['files']==([] if variant=='empty' else ['frames/Chara/Chara_0000/000.png'])
 print('PASS: no-JSON PNG discovery, empty/bad original fallback, native indexed and 3x RGBA exact cell replacements, all 248 Crono cells audited, zero shared-tile/ROM contamination, generated inventory')
