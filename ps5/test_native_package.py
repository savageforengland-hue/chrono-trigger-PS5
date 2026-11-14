#!/usr/bin/env python3
"""Regression checks for the incomplete-folder release, using real built artifacts."""
import argparse
from pathlib import Path
import shutil
import subprocess
import tempfile
from verify_native import verify


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('folder', type=Path)
    parser.add_argument('--tool', required=True, type=Path)
    parser.add_argument('--sdk', required=True, type=Path)
    args = parser.parse_args()
    verify(args.folder, args.tool, args.sdk)
    results = ['complete package accepted']
    with tempfile.TemporaryDirectory() as temp:
        folder = Path(temp) / 'title'
        shutil.copytree(args.folder, folder)
        module = folder / 'sce_module/libc.prx'
        good = module.read_bytes()
        module.unlink()

        def rejected(expected):
            try:
                verify(folder, args.tool, args.sdk)
            except ValueError as error:
                assert expected in str(error), str(error)
                results.append(expected + ': rejected')
            else:
                raise AssertionError('Broken package was accepted: ' + expected)

        rejected('Missing required runtime')
        raw = Path(temp) / 'raw.elf'
        subprocess.run([str(args.tool), 'self', '--extract', '--file',
                        str(args.folder / 'sce_module/libc.prx'), '--out', str(raw)],
                       check=True, capture_output=True)
        module.write_bytes(raw.read_bytes())
        rejected('must be signed FSELF')
        damaged = bytearray(good)
        damaged[-64] ^= 1
        module.write_bytes(damaged)
        rejected('pinned clean-room runtime digest')
        module.write_bytes(good)
        verify(folder, args.tool, args.sdk)
        results.append('restored package accepted')
        original_app = (folder / 'eboot.bin').read_bytes()
        app_raw = Path(temp) / 'app.elf'
        subprocess.run([str(args.tool), 'self', '--extract', '--file', str(folder / 'eboot.bin'),
                        '--out', str(app_raw)], check=True, capture_output=True)
        subprocess.run([str(args.tool), 'self', '--sign', '--in', str(app_raw),
                        '--out', str(folder / 'eboot.bin'), '--magic', '0x1D3D154F'],
                       check=True, capture_output=True)
        rejected('requires the working reference SELF magic')
        (folder / 'eboot.bin').write_bytes(original_app)
        verify(folder, args.tool, args.sdk)
        results.append('PS5 container restored and accepted')
    print('\n'.join(results))


if __name__ == '__main__':
    main()
