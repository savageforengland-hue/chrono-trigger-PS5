#!/usr/bin/env python3
"""Build an independent native folder title, launched from the PS5 home screen."""
import argparse,hashlib,json,os,shutil,subprocess
from pathlib import Path
from verify_native import Elf,audit_elf_layout,audit_linked_mapping,audit_imports

def run(args):subprocess.run([str(a) for a in args],check=True)
def main():
 p=argparse.ArgumentParser(description=__doc__)
 for name in ('sdk','tool','build','out','runtime'):p.add_argument('--'+name,required=True,type=Path)
 a=p.parse_args();here=Path(__file__).resolve().parent
 a.build.mkdir(parents=True,exist_ok=True);a.out.mkdir(parents=True,exist_ok=True)
 obj=a.build/'boot_check.o';llvm=a.build/'boot_check-llvm.elf';native=a.build/'boot_check-native.elf'
 run([a.sdk/'bin/prospero-clang','-std=c11','-O1','-fno-builtin','-fPIC',
      '-ffunction-sections','-fdata-sections','-c',here/'boot_check.c','-o',obj])
 run([a.sdk/'bin/prospero-lld','-T',here/'native/ps5-pie.ld','--eh-frame-hdr',
      '--version-script',here/'native/app-symbols.map','-e','_start','-o',llvm,obj,
      '--as-needed',a.sdk/'target/lib/libkernel.so',a.sdk/'target/lib/libSceSystemService.so'])
 run([a.tool,'link','--in',llvm,'--out',native,'--stub-dir',a.sdk/'target/lib',
      '--module-sdk','0x02000009','--companion-sdk','0x08050001','--file-name','eboot.elf'])
 e=Elf(native.read_bytes());geometry=audit_elf_layout(e);mapping=audit_linked_mapping(llvm,e)
 needed,imports=audit_imports(e,a.sdk,application=False)
 assert set(needed)=={'libkernel.prx','libSceSystemService.prx'},needed
 run([a.tool,'self','--sign','--in',native,'--out',a.out/'eboot.bin','--magic','0xEEF51454'])
 assert (a.out/'eboot.bin').read_bytes()[:4]==bytes.fromhex('5414f5ee')
 run([a.tool,'self','--inspect','--file',a.out/'eboot.bin'])
 sce=a.out/'sce_sys';sce.mkdir(exist_ok=True)
 meta=json.loads((here/'sce_sys/param.json').read_text())
 meta.update(titleId='PPSA90436',conceptId='90436',contentId='UP9000-PPSA90436_00-EGGBOOTPROBE0000',contentVersion='01.000.001')
 meta['localizedParameters']['en-US']['titleName']='Egg Native Boot Check'
 (sce/'param.json').write_text(json.dumps(meta,indent=2)+'\n');shutil.copy2(here/'sce_sys/icon0.png',sce/'icon0.png')
 (a.out/'sce_module').mkdir(exist_ok=True)
 # The caller supplies the reproducibly built, shared clean-room companion.
 shim=a.runtime
 if not shim.is_file():raise SystemExit('Build the sibling PPSA90435 title/runtime first.')
 shutil.copy2(shim,a.out/'sce_module/libc.prx')
 extracted=a.build/'signed-extracted.elf'
 run([a.tool,'self','--extract','--file',a.out/'eboot.bin','--out',extracted])
 audit_elf_layout(Elf(extracted.read_bytes()));audit_linked_mapping(llvm,Elf(extracted.read_bytes()))
 report={'console_verified':False,'purpose':'Only tests native entry without C init/SDL/game/ROM','needed':needed,'imports':imports,'geometry':geometry,'mapping':mapping,'eboot_sha256':hashlib.sha256((a.out/'eboot.bin').read_bytes()).hexdigest()}
 (a.build/'validation.json').write_text(json.dumps(report,indent=2)+'\n')
 print('Independent home-screen Boot Check built; PS5 execution unverified.')
if __name__=='__main__':main()
