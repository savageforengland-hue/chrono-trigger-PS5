#!/usr/bin/env python3
"""Exercise actual independent pack states, root resolution and layered PNG loading."""
from pathlib import Path
import argparse,subprocess,tempfile
p=argparse.ArgumentParser(description=__doc__)
for key in ['sdl-source','sdl-library','rom']:p.add_argument('--'+key,type=Path,required=True)
a=p.parse_args();r=Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory() as tmp:
 d=Path(tmp);common=['cc','-std=c11','-O1','-ffunction-sections','-fdata-sections','-I',str(r),'-I',str(a.sdl_source/'include')]
 exe=d/'roots';subprocess.run([*common,*[str(r/x) for x in ['ps5/tests/pack_roots.c','ps5/pack_state.c','ps5/mods.c','ps5/config_file.c']],'-Wl,--gc-sections',*[f'-Wl,--wrap={x}' for x in ['fopen','opendir','rename','unlink']],'-o',str(exe)],check=True)
 data=d/'data';(data/'mods/graphics').mkdir(parents=True)
 for name in ['a-char','b-house','c-world']:(data/'mods/packs'/name/'graphics').mkdir(parents=True)
 ini=data/'ps5.ini';original='# Keep output and old selection\nactive_pack=b-house\noutput_height=2160\n';ini.write_text(original)
 for phase in ['migration','reload','all-off']:subprocess.run([str(exe),str(data),phase],check=True)
 assert ini.read_text()==original
 state=data/'mods/pack-states.ini';state.unlink();subprocess.run([str(exe),str(data),'absent-legacy'],check=True)
 state.write_text('a-char=0\n../escape=1\na-char=1\nb-house=3\n'+'x'*400+'c-world=1\n')
 subprocess.run([str(exe),str(data),'bad-lines'],check=True)
 exe=d/'stack';subprocess.run([*common,'-DEGG_PS5_CORE','-DEGG_PS5_PACK_STACK',*[str(r/x) for x in ['ps5/tests/pack_stack.c','runtime/gfx_rom.c','runtime/gfx_mod.c']],str(a.sdl_library),'-Wl,--gc-sections','-lm','-ldl','-pthread','-o',str(exe)],check=True)
 for variant in ['normal','empty','bad','global','none']:
  folder=d/variant;folder.mkdir();subprocess.run([str(exe),str(a.rom),str(folder),variant],check=True,timeout=30)

with tempfile.TemporaryDirectory() as tmp:
 d=Path(tmp);common=['cc','-std=c11','-O1','-ffunction-sections','-fdata-sections','-I',str(r),'-I',str(a.sdl_source/'include')];libs=[str(a.sdl_library),'-Wl,--gc-sections','-lm','-ldl','-pthread']
 exe=d/'hud';subprocess.run([*common,*[str(r/x) for x in ['ps5/tests/hud_stack.c','runtime/hud_theme.c','runtime/ui_font.c']],*libs,'-o',str(exe)],check=True);subprocess.run([str(exe),str(d)],check=True,timeout=20)
 exe=d/'redraw';subprocess.run([*common,'-DEGG_PS5_CORE','-DEGG_PS5_PACK_STACK',*[str(r/x) for x in ['ps5/tests/stack_redraw.c','runtime/gfx_rom.c','runtime/gfx_mod.c','runtime/redraw.c','runtime/redraw_manifest.c','runtime/redraw_match.c','runtime/sprite_hd.c']],*libs,'-o',str(exe)],check=True)
 for variant in ['valid','bad']:
  folder=d/variant;folder.mkdir();subprocess.run([str(exe),str(a.rom),str(folder)+'/',variant],check=True,timeout=30)
