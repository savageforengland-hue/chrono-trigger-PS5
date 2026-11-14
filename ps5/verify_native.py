#!/usr/bin/env python3
"""Reject incomplete native folders and audit imports against public SDK exports.

This is an offline artifact check, not a console boot or firmware certification.
"""
import argparse
import base64
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import tempfile

SHIM_SHA256 = 'e6ff45d16adf687855cc3b33b0c8a4132b6504360b221e0a34c7e99fb3ba0036'
FSELF_MAGICS = (bytes.fromhex('4f153d1d'), bytes.fromhex('5414f5ee'))


def require(condition, message):
    if not condition:
        raise ValueError(message)


class Elf:
    def __init__(self, data):
        self.data = data
        require(data[:6] == b'\x7fELF\x02\x01', 'Expected ELF64 little-endian')
        self.kind = struct.unpack_from('<H', data, 16)[0]
        phoff = struct.unpack_from('<Q', data, 32)[0]
        phsize, phnum = struct.unpack_from('<HH', data, 54)
        self.ph = [struct.unpack_from('<IIQQQQQQ', data, phoff + i * phsize)
                   for i in range(phnum)]
        dyn = next(p for p in self.ph if p[0] == 2)
        self.dynamic = []
        for at in range(dyn[2], dyn[2] + dyn[5], 16):
            tag, value = struct.unpack_from('<qQ', data, at)
            if not tag:
                break
            self.dynamic.append((tag, value))
        self.tags = dict(self.dynamic)
        self.strings = self.read(self.tags[5], self.tags[10])

    def read(self, address, size):
        for p in self.ph:
            if p[0] == 1 and p[3] <= address and address + size <= p[3] + p[5]:
                start = p[2] + address - p[3]
                return self.data[start:start + size]
        raise ValueError(f'Unmapped ELF address {address:#x} size {size}')

    def string(self, offset):
        return self.strings[offset:self.strings.index(0, offset)].decode()

    def records(self, tag):
        return {value >> 48: self.string(value & 0xffffffff)
                for key, value in self.dynamic if key == tag}

    def symbols(self):
        # SCE publishes size explicitly; generic SDK stubs have SYSV hash nchain.
        count = self.tags.get(0x6100003f)
        if count is not None:
            count //= self.tags[11]
        else:
            count = struct.unpack('<II', self.read(self.tags[4], 8))[1]
        data = self.read(self.tags[6], count * self.tags[11])
        result = []
        for i in range(1, count):
            name, info, other, section, value, size = struct.unpack_from('<IBBHQQ', data, i * 24)
            result.append((self.string(name), section))
        return result


def nid(name):
    suffix = bytes.fromhex('518d64a635ded8c1e6b039b1c3e55230')
    return base64.b64encode(hashlib.sha1(name.encode() + suffix).digest()[:8][::-1]).decode()[:11].replace('/', '-')


def decode_id(value):
    result = 0
    for char in value:
        result = result * 64 + 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+-'.index(char)
    return result


def audit_elf_layout(image):
    loads = [p for p in image.ph if p[0] == 1]
    require(bool(loads), 'ELF has no loadable segments')
    for index, p in enumerate(loads):
        require(p[5] <= p[6], f'PT_LOAD {index}: file size exceeds memory size')
        require(p[2] <= len(image.data) and p[5] <= len(image.data) - p[2],
                f'PT_LOAD {index}: file range exceeds ELF')
        require(p[7] in (0, 1) or (p[7] & (p[7] - 1)) == 0,
                f'PT_LOAD {index}: alignment is not a power of two')
        require(p[7] <= 1 or p[2] % p[7] == p[3] % p[7],
                f'PT_LOAD {index}: file offset/address alignment mismatch')
    entry = struct.unpack_from('<Q', image.data, 24)[0]
    require(any(p[1] & 1 and p[3] <= entry < p[3] + p[5] for p in loads),
            'ELF entry point is outside executable file-backed memory')
    for p in image.ph:
        if p[0] == 0x61000001:
            require(image.read(p[3], p[5]) == image.data[p[2]:p[2] + p[5]],
                    'Process parameters do not map to their advertised bytes')
    return {'load_segments': len(loads), 'offset_address_congruence': 'pass',
            'file_ranges_and_entry': 'pass'}


def audit_linked_mapping(linked_path, native):
    """Compare all retained allocated sections against what the loader maps."""
    data = Path(linked_path).read_bytes()
    shoff = struct.unpack_from('<Q', data, 40)[0]
    size, count, names_index = struct.unpack_from('<HHH', data, 58)
    require(shoff > 0 and count > 0, 'Expected unstripped LLVM input section table')
    sections = [struct.unpack_from('<IIQQQQIIQQ', data, shoff + i * size)
                for i in range(count)]
    names_section = sections[names_index]
    names = data[names_section[4]:names_section[4] + names_section[5]]
    checked = []
    for section in sections:
        offset, kind, flags, address, file, length = section[:6]
        name = names[offset:names.index(0, offset)].decode()
        if not flags & 2 or kind == 8 or length == 0:
            continue
        if name in ('.dynstr', '.dynsym', '.dynamic', '.hash', '.gnu.hash') or name.startswith('.rela.'):
            continue
        require(native.read(address, length) == data[file:file + length],
                'Native loader mapping changes linked section bytes: ' + name)
        checked.append(name)
    require('.data.rel.ro' in checked and '.got' in checked,
            'Missing independent RELRO/GOT mapping verification')
    return {'allocated_sections_verified': checked, 'byte_comparison': 'pass'}


def extract(module, tool, temporary):
    data = module.read_bytes()
    require(data[:4] in FSELF_MAGICS, f'{module.name}: expected signed FSELF, not raw ELF')
    result = subprocess.run([str(tool), 'self', '--inspect', '--file', str(module)],
                            check=True, capture_output=True, text=True).stdout
    require('integrity: valid' in result, f'{module.name}: SELF integrity failed')
    subprocess.run([str(tool), 'self', '--extract', '--file', str(module), '--out', str(temporary)],
                   check=True, capture_output=True)
    return Elf(temporary.read_bytes())


def audit_imports(image, sdk, application=True):
    needed = [image.string(value) for tag, value in image.dynamic if tag == 1]
    require('libc.prx' not in needed, 'Application C routines must not bind to the loader-only libc shim')
    modules, libraries = image.records(0x61000045), image.records(0x61000049)
    exports = {}
    for name in needed:
        stub = sdk / 'target/lib' / name.replace('.prx', '.so')
        require(stub.is_file(), f'No public SDK provider for {name}')
        elf = Elf(stub.read_bytes())
        names = {nid(symbol): symbol for symbol, section in elf.symbols() if section}
        exports[name.removesuffix('.prx')] = names
    imports = []
    for symbol, section in image.symbols():
        if section:
            continue
        hashed, library_id, module_id = symbol.split('#')
        module = modules[decode_id(module_id)]
        library = libraries[decode_id(library_id)]
        require(module in exports, f'Unknown module {module}')
        require(hashed in exports[module], f'Unknown import {symbol} from {module}')
        name = exports[module][hashed]
        imports.append({'symbol': name, 'module': module, 'library': library, 'nid': hashed})
    for required in (('malloc', 'free', 'fopen', 'fwrite', '_init_env', '__isthreaded') if application else ()):
        entries = [i for i in imports if i['symbol'] == required]
        require(entries and all(i['module'] == 'libSceLibcInternal' for i in entries),
                f'{required} must resolve to libSceLibcInternal')
    if application:
        refused = {'dup2', 'opendir', 'fdopendir', 'readdir', 'rewinddir', 'dirfd', 'closedir'}
        found = refused & {entry['symbol'] for entry in imports}
        require(not found, f'Use native-title compatibility paths instead of these imports: {sorted(found)}')
        for required, provider in [('sceKernelDebugOutText', 'libkernel'),
                                   ('sceSystemServiceLoadExec', 'libSceSystemService')]:
            require(any(i['symbol'] == required and i['module'] == provider for i in imports),
                    f'Missing native diagnostic/lifecycle import {required}')
    return needed, imports


def verify(folder, tool, sdk, controller_only=False):
    module = folder / 'sce_module/libc.prx'
    require(module.is_file(), 'Missing required runtime: sce_module/libc.prx')
    require(module.read_bytes()[:4] in FSELF_MAGICS, 'libc.prx must be signed FSELF, not raw ELF')
    require(hashlib.sha256(module.read_bytes()).hexdigest() == SHIM_SHA256,
            'libc.prx does not match the pinned clean-room runtime digest')
    require((folder / 'eboot.bin').read_bytes()[:4] == bytes.fromhex('5414f5ee'),
            'This native PS5 profile requires the working reference SELF magic 0xEEF51454')
    with tempfile.TemporaryDirectory() as tmp:
        runtime = extract(module, tool, Path(tmp) / 'runtime.elf')
        app = extract(folder / 'eboot.bin', tool, Path(tmp) / 'app.elf')
        require(app.kind == 0xfe10, 'Wrong application ELF type')
        app_layout = audit_elf_layout(app)
        runtime_layout = audit_elf_layout(runtime)
        needed, imports = audit_imports(app, sdk)
        if controller_only:
            require(not ({'libSceImeDialog.prx','libSceKeyboard.prx'} & set(needed)),
                    'Controller-only profile must not hard-load keyboard or IME')
        # The pinned companion has its own system imports; keep that graph visible.
        runtime_needed, runtime_imports = audit_imports(runtime, sdk, application=False)
    metadata = json.loads((folder / 'sce_sys/param.json').read_text())
    require(len(metadata['conceptId']) == 5 and metadata['conceptId'].isdigit(), 'conceptId must have five digits')
    require(metadata['gameIntent']['permittedIntents'][0]['intentType'] == 'launchActivity', 'Missing game launch intent')
    return {'offline_validation': 'pass', 'console_boot_tested': False,
            'runtime_sha256': SHIM_SHA256, 'elf_type': '0xFE10', 'self_magic': '0xEEF51454',
            'needed_system_modules': needed, 'runtime_system_modules': runtime_needed,
            'imports_verified_against_public_sdk': len(imports), 'imports': imports,
            'runtime_imports_verified_against_public_sdk': len(runtime_imports),
            'runtime_imports': runtime_imports,
            'app_layout': app_layout, 'runtime_layout': runtime_layout}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('folder', type=Path)
    parser.add_argument('--tool', required=True, type=Path)
    parser.add_argument('--sdk', required=True, type=Path)
    parser.add_argument('--report', type=Path)
    parser.add_argument('--controller-only', action='store_true')
    args = parser.parse_args()
    try:
        report = verify(args.folder, args.tool, args.sdk, args.controller_only)
    except (ValueError, KeyError, StopIteration, subprocess.CalledProcessError) as error:
        raise SystemExit('Native package rejected: ' + str(error))
    if args.report:
        args.report.write_text(json.dumps(report, indent=2) + '\n')
    print(f"Offline package check passed: runtime present; {len(report['imports'])} imports verified. PS5 boot remains untested.")


if __name__ == '__main__':
    main()
