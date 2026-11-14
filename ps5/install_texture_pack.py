#!/usr/bin/env python3
"""Install/update a data-only Egg of Time texture pack in a local data folder."""
import argparse
import json
from pathlib import Path, PurePosixPath
import re
import shutil
import tempfile
from zipfile import ZipFile


def install(archive, data_root, activate=False):
    with ZipFile(archive) as source:
        manifest = json.loads(source.read('pack.json'))
        pack_id = manifest.get('id', '')
        if (not isinstance(pack_id, str) or not re.fullmatch(r'[a-z0-9][a-z0-9_-]{0,62}', pack_id)
                or pack_id == 'default'):
            raise ValueError('pack.json needs a non-reserved lowercase id (letters, numbers, _ or -).')
        if manifest.get('format_version') != 1 or manifest.get('game_code') != 'YQUE':
            raise ValueError('Expected format_version 1 and game_code YQUE.')
        entries = source.infolist()
        if len(entries) > 20000 or sum(e.file_size for e in entries) > 512 * 1024 * 1024:
            raise ValueError('Pack exceeds 20,000 entries or 512 MiB unpacked.')
        for entry in entries:
            p = PurePosixPath(entry.filename)
            if p.is_absolute() or '..' in p.parts or '\\' in entry.filename or not p.parts:
                raise ValueError('Pack contains an invalid path: ' + entry.filename)
            if entry.is_dir():
                continue
            if p.parts[0] != 'graphics' and p.name not in ('pack.json', 'README.md', 'LICENSE.txt'):
                raise ValueError('Unexpected pack file: ' + entry.filename)
            if p.suffix.lower() not in ('.png', '.txt', '.json', '.md'):
                raise ValueError('Packs may contain only PNG images and text/metadata.')
            if ((entry.external_attr >> 16) & 0o170000) == 0o120000:
                raise ValueError('Pack contains a symbolic link.')
        packs = Path(data_root) / 'mods' / 'packs'
        packs.mkdir(parents=True, exist_ok=True)
        staging = Path(tempfile.mkdtemp(prefix='.' + pack_id + '-', dir=packs))
        target, backup = packs / pack_id, packs / (pack_id + '.backup')
        try:
            source.extractall(staging)
            if not (staging / 'graphics').is_dir():
                raise ValueError('Pack needs a graphics/ directory.')
            if backup.exists():
                shutil.rmtree(backup)
            if target.exists():
                target.rename(backup)
            try:
                staging.rename(target)
            except OSError:
                if backup.exists() and not target.exists():
                    backup.rename(target)
                raise
        finally:
            if staging.exists():
                shutil.rmtree(staging)
    if activate:
        config = Path(data_root) / 'ps5.ini'
        text = config.read_text() if config.exists() else 'output_height=2160\ntarget_fps=60\nrender_scale=3\n'
        if re.search(r'^\s*active_pack\s*=', text, flags=re.M):
            text = re.sub(r'^\s*active_pack\s*=.*$', 'active_pack=' + pack_id, text, flags=re.M)
        else:
            text = text.rstrip() + '\nactive_pack=' + pack_id + '\n'
        temporary = config.with_suffix('.ini.tmp')
        temporary.write_text(text)
        temporary.replace(config)
    return target


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('archive', type=Path)
    parser.add_argument('--data-root', required=True, type=Path, help='Local eggoftime data folder to prepare for FTP')
    parser.add_argument('--activate', action='store_true')
    args = parser.parse_args()
    try:
        print(install(args.archive, args.data_root, args.activate))
    except (OSError, ValueError, KeyError) as error:
        parser.exit(1, str(error) + '\n')
