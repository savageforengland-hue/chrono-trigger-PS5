#!/usr/bin/env python3
"""Create the public source and original PC-source downloads for GitHub Pages."""
from pathlib import Path
import hashlib,json,shutil,zipfile
from check_public_source import ROOT,check,check_bytes
files=check()
out=ROOT/'site/downloads';out.mkdir(parents=True,exist_ok=True)
archive=out/'CTPS5-0.1.0-rc1-source.zip'
with zipfile.ZipFile(archive,'w',zipfile.ZIP_DEFLATED,compresslevel=9) as z:
 for p in sorted(files):
  name=p.relative_to(ROOT).as_posix()
  if name.startswith('site/downloads/'):continue
  info=zipfile.ZipInfo('CTPS5/'+name,date_time=(2026,11,14,0,0,0));info.compress_type=zipfile.ZIP_DEFLATED
  info.external_attr=(0o100755 if name.endswith('.sh') else 0o100644)<<16
  z.writestr(info,p.read_bytes())
check_bytes(archive.name,archive.read_bytes())
original=ROOT/'upstream/egg-of-time-v1.3.5-source.tar.gz'
shutil.copyfile(original,out/original.name)
items=[{'file':p.name,'bytes':p.stat().st_size,'sha256':hashlib.sha256(p.read_bytes()).hexdigest()} for p in (archive,out/original.name)]
(out/'SHA256SUMS.txt').write_text(''.join(f"{x['sha256']}  {x['file']}\n" for x in items))
(out/'manifest.json').write_text(json.dumps({'release':'0.1.0-rc1','distribution':'authored source only; build game locally from own NDS','files':items},indent=2)+'\n')
print(json.dumps(items,indent=2))
