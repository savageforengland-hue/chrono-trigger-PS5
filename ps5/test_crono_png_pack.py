#!/usr/bin/env python3
"""Build and audit purple-tunic Crono PNG tests; --all covers every visible pose."""
from pathlib import Path
import argparse,subprocess,tempfile,json,shutil
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--sdl-source',type=Path,required=True);p.add_argument('--sdl-library',type=Path,required=True);p.add_argument('--rom',type=Path,required=True)
p.add_argument('--all',action='store_true',help='Test every visible Crono pose, including walking and all battle poses')
p.add_argument('--output',type=Path,help='Optional new/empty folder retaining the generated eggoftime/mods test pack')
a=p.parse_args();r=Path(__file__).resolve().parents[1]
if a.output and a.output.exists() and any(a.output.iterdir()):p.error('--output must be new or empty')
with tempfile.TemporaryDirectory() as tmp:
 d=Path(tmp);data=((a.output or d/'package')/'eggoftime').resolve();data.mkdir(parents=True,exist_ok=True);graphics=data/'mods/packs/crono-png-test/graphics'
 common=['cc','-std=c11','-O1','-ffunction-sections','-fdata-sections','-I',str(r),'-I',str(r/'runtime'),'-I',str(a.sdl_source/'include')];libs=[str(a.sdl_library),'-Wl,--gc-sections','-lm','-ldl','-pthread']
 maker=d/'make-test';subprocess.run([*common,str(r/'tools/make_crono_png_test.c'),str(r/'runtime/gfx_rom.c'),*libs,'-o',str(maker)],check=True)
 subprocess.run([str(maker),str(a.rom),str(graphics),*(['--all'] if a.all else [])],check=True,timeout=20)
 checker=d/'verify-test';src=['ps5/tests/crono_png_pack.c','runtime/gfx_rom.c','runtime/gfx_mod.c','runtime/redraw.c','runtime/redraw_manifest.c','runtime/redraw_match.c','runtime/sprite_hd.c']
 subprocess.run([*common,'-DEGG_PS5_CORE',*[str(r/s) for s in src],*libs,'-o',str(checker)],check=True)
 subprocess.run([str(checker),str(a.rom),str(data)+'/', '244' if a.all else '2','100' if a.all else '0'],check=True,timeout=30)
 manifest=json.loads((graphics/'manifest.generated.json').read_text());assert manifest['files']==([f'frames/Chara/Chara_0000/{i:03d}.png' for i in range(248) if i not in range(131,135)] if a.all else ['frames/Chara/Chara_0000/198.png','frames/Chara/Chara_0000/199.png'])
 for case,count,variant in ([('missing-198',243,101),('bad-199',243,102),('empty',0,100)] if a.all else [('missing-198',1,1),('bad-199',1,2),('empty',0,0)]):
  other=d/case;shutil.copytree(data,other);frames=other/'mods/packs/crono-png-test/graphics/frames/Chara/Chara_0000'
  if case=='missing-198':(frames/'198.png').unlink()
  elif case=='bad-199':(frames/'199.png').write_bytes(b'Deliberately invalid PNG')
  else:
   for file in frames.glob('*.png'):file.unlink()
  subprocess.run([str(checker),str(a.rom),str(other)+'/',str(count),str(variant)],check=True,timeout=30)
 print('PASS: '+('244-PNG full visible-pose pack' if a.all else 'two-PNG named pack')+', actual no-JSON loader, 248-cell identity audit, native/HD colour and alpha checks, original origins, missing/bad/empty fallback and zero original-ROM/shared-tile changes')
