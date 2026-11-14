#!/usr/bin/env python3
"""Run the actual L3+R3 host-menu state machine, renderer and scheduler pause."""
from pathlib import Path
import argparse,subprocess,tempfile
p=argparse.ArgumentParser(description=__doc__);p.add_argument('--sdl-source',type=Path,required=True);p.add_argument('--preview-dir',type=Path);a=p.parse_args()
root=Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory() as tmp:
 for name,extra in [('system_menu_host',['runtime/ui_font.c','ps5/prompts.c']),('menu_pause',[]),('mod_settings',['ps5/config_file.c','ps5/mod_menu.c'])]:
  exe=Path(tmp)/name
  subprocess.run(['cc','-std=c11','-O1','-DEGG_PS5','-DEGG_PS5_CORE','-ffunction-sections','-fdata-sections','-I',str(root),'-I',str(a.sdl_source/'include'),str(root/'ps5/tests'/f'{name}.c'),*[str(root/x) for x in extra],'-Wl,--gc-sections',*(['-Wl,--wrap=fopen','-Wl,--wrap=rename','-Wl,--wrap=unlink','-Wl,--wrap=opendir'] if name=='mod_settings' else []),'-o',str(exe)],check=True)
  if a.preview_dir:a.preview_dir.mkdir(parents=True,exist_ok=True)
  if name=='mod_settings':
   for pack in ['a-pack','z-pack','invalid.name']:(Path(tmp)/'mods/packs'/pack/'graphics').mkdir(parents=True)
   (Path(tmp)/'mods/packs/without-graphics').mkdir()
  subprocess.run([str(exe),*([tmp] if name=='mod_settings' else []),*([str(a.preview_dir)] if a.preview_dir and name=='system_menu_host' else [])],check=True,timeout=10)
