#!/usr/bin/env python3
"""Stage a side-by-side CTPS5 Mods app from a locally built native app.

No game data is generated or downloaded. Keep the staged executable private.
The original PPSA90435 folder is never changed.
"""
from pathlib import Path
import argparse,json,shutil,re
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('built_app',type=Path)
p.add_argument('output',type=Path,help='New empty PPSA90436 folder')
p.add_argument('--title-id',default='PPSA90436')
a=p.parse_args()
if not re.fullmatch(r'PPSA\d{5}',a.title_id) or a.title_id=='PPSA90435':p.error('Choose a separate PPSA title ID; PPSA90435 is the base app.')
source=a.built_app.resolve();out=a.output.resolve()
if out==source or source in out.parents:p.error('Output must be separate from the built app.')
if out.exists() and any(out.iterdir()):p.error('Output must be empty; existing apps are not overwritten.')
for name in ['eboot.bin','sce_module/libc.prx','sce_sys/param.json','sce_sys/icon0.png']:
 if not (source/name).is_file():p.error('Missing required native app file: '+name)
meta=json.loads((source/'sce_sys/param.json').read_text());old=meta['titleId']
meta['titleId']=a.title_id;meta['conceptId']=a.title_id[4:]
meta['contentId']=meta['contentId'].replace(old,a.title_id)
meta['contentVersion']='01.006.003'
for item in meta['localizedParameters'].values():
 if isinstance(item,dict) and 'titleName' in item:item['titleName']='CTPS5 Mods'
out.mkdir(parents=True,exist_ok=True)
shutil.copy2(source/'eboot.bin',out/'eboot.bin')
for folder in ['sce_module','sce_sys']:shutil.copytree(source/folder,out/folder,dirs_exist_ok=True)
(out/'sce_sys/param.json').write_text(json.dumps(meta,indent=2)+'\n')
print(f'Staged {a.title_id} (CTPS5 Mods); PPSA90435 unchanged. Shared user data: /data/eggoftime/')
