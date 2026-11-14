#!/usr/bin/env python3
"""Build native startup, link real SDK providers, and include the clean-room shim."""
import argparse
import hashlib
import os
from pathlib import Path
import shlex
import subprocess
from verify_native import Elf, audit_elf_layout, audit_linked_mapping

RAW_SHA256 = '8ee6e124993e1af26420cb455890fd002f5d6c7e78883c860ce45734e7d002bb'
SHIM_SHA256 = 'e6ff45d16adf687855cc3b33b0c8a4132b6504360b221e0a34c7e99fb3ba0036'
PROVIDERS = ['libSceLibcInternal', 'libkernel', 'libSceAudioOut',
             'libScePad', 'libSceSystemService', 'libSceUserService',
             'libSceVideoOut', 'libSceSysmodule', 'libSceNet']
HEAP_WRAPS = ['malloc', 'calloc', 'realloc', 'free', 'posix_memalign',
              'aligned_alloc', 'memalign', 'malloc_usable_size', 'reallocf',
              'reallocarray', 'getline', 'getdelim']


def run(command):
    subprocess.run([str(s) for s in command], check=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ['build', 'sdl-build', 'sdk', 'out']:
        parser.add_argument('--' + name, required=True, type=Path)
    parser.add_argument('--system-keyboard', action='store_true',
                        help='Match an EGG_PS5_SYSTEM_KEYBOARD=ON CMake build')
    args = parser.parse_args()
    here = Path(__file__).resolve().parent
    native = here / 'native'
    build, sdk = args.build.resolve(), args.sdk.resolve()
    out = args.out.resolve()
    objects = [Path(p) for p in (build / 'egg_objects.list').read_text().splitlines() if p]
    if not objects or not all(p.is_file() for p in objects):
        parser.error('Build the egg_objects CMake target first.')
    host = build / 'native-tools'
    host.mkdir(exist_ok=True)
    cxx = shlex.split(os.environ.get('CXX_FOR_BUILD', 'c++'))
    common = cxx + ['-std=c++20', '-O2']
    run(common + [native / 'libc_builder.cpp', '-o', host / 'libc-builder'])
    run(common + [native / n for n in ['native_app_builder.cpp', 'self_container.cpp',
                                     'elf_object.cpp', 'sce_module_writer.cpp']]
        + ['-lz', '-o', host / 'ps5-native-tool'])
    tool = host / 'ps5-native-tool'
    raw = host / 'libc.raw.elf'
    run([host / 'libc-builder', native / 'runtime/api-surface.txt',
         native / 'runtime/imports.txt', raw])
    if hashlib.sha256(raw.read_bytes()).hexdigest() != RAW_SHA256:
        raise SystemExit('Clean-room runtime did not reproduce its pinned raw digest.')
    module = out.parent / 'sce_module/libc.prx'
    module.parent.mkdir(parents=True, exist_ok=True)
    run([tool, 'self', '--sign', '--in', raw, '--out', module])
    if hashlib.sha256(module.read_bytes()).hexdigest() != SHIM_SHA256:
        raise SystemExit('Clean-room runtime did not reproduce its pinned signed digest.')
    crt = host / 'app_crt.o'
    run([sdk / 'bin/prospero-clang++', '-std=c++20', '-O2', '-fno-exceptions',
         '-fno-rtti', '-ffunction-sections', '-fdata-sections', '-c',
         native / 'app_crt.cpp', '-o', crt])
    llvm = build / 'eggoftime-llvm.elf'
    directory_names = ['opendir', 'fdopendir', 'readdir', 'rewinddir', 'dirfd', 'closedir']
    directory_map = build / 'directory-compat.map'
    directory_map.write_text('{ local: ' + '; '.join(directory_names) + '; };\n')
    # The loader companion libc.prx is not a malloc/stdio implementation.
    run([sdk / 'bin/prospero-lld', '-T', native / 'ps5-pie.ld', '--eh-frame-hdr',
         '--gc-sections', '--exclude-libs=ALL',
         '--version-script', native / 'app-symbols.map',
         '--version-script', directory_map,
         *['--wrap=' + name for name in HEAP_WRAPS],
         *['--defsym=' + name + '=ps5_' + name for name in directory_names],
         '-e', '_start', '-o', llvm,
         crt] + objects + [args.sdl_build.resolve() / 'libSDL3.a',
                          sdk / 'target/lib/libc.a', '--as-needed']
        + [sdk / ('target/lib/' + p + '.so') for p in
           PROVIDERS + (['libSceImeDialog', 'libSceKeyboard'] if args.system_keyboard else [])])
    elf = build / 'eggoftime-native.elf'
    run([tool, 'link', '--in', llvm, '--out', elf, '--stub-dir', sdk / 'target/lib',
         '--module-sdk', '0x02000009', '--companion-sdk', '0x08050001',
         '--file-name', 'eboot.elf'])
    native_image = Elf(elf.read_bytes())
    geometry = audit_elf_layout(native_image)
    mapping = audit_linked_mapping(llvm, native_image)
    import json
    (build / 'native-mapping-validation.json').write_text(
        json.dumps({'geometry': geometry, 'mapped_sections': mapping}, indent=2) + '\n')
    run([tool, 'self', '--sign', '--in', elf, '--out', out, '--magic', '0xEEF51454'])
    run([os.sys.executable, here / 'verify_native.py', out.parent,
         '--tool', tool, '--sdk', sdk, '--report', build / 'native-validation.json']
        + ([] if args.system_keyboard else ['--controller-only']))


if __name__ == '__main__':
    try:
        main()
    except subprocess.CalledProcessError as error:
        raise SystemExit(f'Native build failed (exit {error.returncode}).')
