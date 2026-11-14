#!/usr/bin/env python3
"""Guard public source packages against known private game/build artifacts."""
from pathlib import Path, PurePosixPath
import io,json,hashlib,subprocess,tarfile,zipfile
ROOT=Path(__file__).resolve().parents[1]
IGNORED={'.git','.deps','__pycache__','build-ps5','build','game','gen','extract','rom'}
BANNED={'.nds','.sav','.state','.elf','.prx','.o','.a','.so','.dll','.exe'}
def check_name(name):
 p=PurePosixPath(name)
 if p.is_absolute() or '..' in p.parts: raise ValueError('Unsafe path: '+name)
 if p.suffix.lower() in BANNED or p.name.lower() in {'eboot.bin','dispatch.c'}:
  raise ValueError('Private game or build artifact: '+name)
 if any(x in {'rom','extract','gen','game','build-ps5'} for x in p.parts):
  raise ValueError('Private game/build directory: '+name)
def check_bytes(name,data):
 check_name(name)
 if name.endswith(('.tar.gz','.tgz')):
  with tarfile.open(fileobj=io.BytesIO(data),mode='r:gz') as archive:
   for entry in archive.getmembers():
    if entry.isfile():check_bytes(entry.name,archive.extractfile(entry).read())
 elif name.endswith('.zip'):
  with zipfile.ZipFile(io.BytesIO(data)) as archive:
   for entry in archive.infolist():
    if not entry.is_dir():check_bytes(entry.filename,archive.read(entry))
def source_files():
 # In a checkout, audit tracked files (including forbidden files forced into Git).
 if (ROOT/'.git').exists():
  names=subprocess.check_output(['git','-C',str(ROOT),'ls-files','-z']).decode().split('\0')
  if any(names):return [ROOT/n for n in names if n]
 return sorted(p for p in ROOT.rglob('*') if p.is_file() and not any(x in IGNORED for x in p.relative_to(ROOT).parts) and not str(p.relative_to(ROOT)).startswith('site/downloads/'))
def check():
 files=source_files()
 for p in files:
  if p.is_symlink():raise ValueError('Public source symlink requires review: '+str(p))
  check_bytes(p.relative_to(ROOT).as_posix(),p.read_bytes())
 meta=json.loads((ROOT/'upstream/UPSTREAM.json').read_text())
 archive=ROOT/'upstream'/meta['archive']
 assert hashlib.sha256(archive.read_bytes()).hexdigest()==meta['sha256'],'Upstream archive checksum mismatch'
 print(f'PASS: {len(files)} public source files and nested archives; pinned original PC source verified')
 return files
if __name__=='__main__':check()
