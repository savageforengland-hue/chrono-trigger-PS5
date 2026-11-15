#!/usr/bin/env python3
"""Check external Crono test-pack files and actual ROM-backed redraw loader."""
from pathlib import Path
import argparse,subprocess,tempfile,shutil,json
p=argparse.ArgumentParser(description=__doc__);p.add_argument('--sdl-source',type=Path,required=True);p.add_argument('--sdl-library',type=Path,required=True);p.add_argument('--rom',type=Path,required=True);p.add_argument('--data-root',type=Path,required=True);a=p.parse_args();root=Path(__file__).resolve().parents[1]
manifest=json.loads((a.data_root/'mods/crono-hd/mod.json').read_text());assert manifest['schema']==2 and len(manifest['frames'])==8
files=[]
from PIL import Image
for spec in manifest['frames']:
 path=a.data_root/'mods/crono-hd'/spec['file'];assert path.name==f"{spec['cell']:03d}.png";image=Image.open(path);assert image.mode=='RGBA' and image.size==(spec['width'],spec['height']);assert image.getextrema()[3][0]==0 and image.getextrema()[3][1]>=128;files.append(path)
assert len(set(files))==8 and set((a.data_root/'mods/crono-hd/frames/Chara/Chara_0000').glob('*.png'))==set(files)
with tempfile.TemporaryDirectory() as tmp:
 exe=Path(tmp)/'assets';sources=['ps5/tests/crono_redraw_assets.c','runtime/redraw.c','runtime/redraw_manifest.c','runtime/redraw_match.c','runtime/gfx_rom.c','runtime/gfx_mod.c','runtime/sprite_hd.c']
 subprocess.run(['cc','-std=c11','-O1','-ffunction-sections','-fdata-sections','-I',str(root),'-I',str(a.sdl_source/'include'),*[str(root/s) for s in sources],str(a.sdl_library),'-Wl,--gc-sections','-lm','-ldl','-pthread','-o',str(exe)],check=True)
 subprocess.run([str(exe),str(a.rom),str(a.data_root.resolve())+'/'],check=True,timeout=60)
 data=Path(tmp)/'missing';shutil.copytree(a.data_root/'mods',data/'mods');(data/'mods/crono-hd'/manifest['frames'][0]['file']).unlink();subprocess.run([str(exe),str(a.rom),str(data)+'/', 'missing'],check=True,timeout=60)
 data=Path(tmp)/'invalid';shutil.copytree(a.data_root/'mods',data/'mods');(data/'mods/crono-hd/mod.json').write_text('{not JSON');subprocess.run([str(exe),str(a.rom),str(data)+'/', 'invalid'],check=True,timeout=60)
 data=Path(tmp)/'disabled-frame';shutil.copytree(a.data_root/'mods',data/'mods');changed=json.loads((data/'mods/crono-hd/mod.json').read_text());changed['frames'][0]['enabled']=False;(data/'mods/crono-hd/mod.json').write_text(json.dumps(changed));subprocess.run([str(exe),str(a.rom),str(data)+'/', 'missing'],check=True,timeout=60)
 data=Path(tmp)/'disabled-pack';shutil.copytree(a.data_root/'mods',data/'mods');changed['enabled']=False;(data/'mods/crono-hd/mod.json').write_text(json.dumps(changed));subprocess.run([str(exe),str(a.rom),str(data)+'/', 'invalid'],check=True,timeout=60)
 print('PASS: eight RGBA PNG filenames, JSON coverage, 4x dimensions, transparent backgrounds and actual loader/fallback integration')
