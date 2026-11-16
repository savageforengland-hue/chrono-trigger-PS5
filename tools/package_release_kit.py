#!/usr/bin/env python3
"""Package audited source/tools, or a separate private local-play kit.

Does not build, download, upload, install, or change user settings.
"""
from pathlib import Path
import argparse
import hashlib
import json
import os
import re
import subprocess
import tempfile
import zipfile
from check_public_source import ROOT, check, check_bytes

RUNTIME_SHA256 = "e6ff45d16adf687855cc3b33b0c8a4132b6504360b221e0a34c7e99fb3ba0036"


def personal_files(app, version):
    app = app.resolve()
    for name in ["eboot.bin", "sce_module/libc.prx", "sce_sys/param.json", "sce_sys/icon0.png"]:
        if not (app / name).is_file():
            raise ValueError("Missing complete app file: " + name)
    files = sorted(p for p in app.rglob("*") if p.is_file())
    if any(p.is_symlink() or not p.resolve().is_relative_to(app) for p in app.rglob("*")):
        raise ValueError("App symlinks are not supported")
    for path in files:
        name = path.relative_to(app).as_posix()
        if name != "eboot.bin" and not name.startswith(("sce_module/", "sce_sys/")):
            raise ValueError("Unexpected app file: " + name)
        if path.suffix.lower() in {".nds", ".sav", ".state", ".ini"}:
            raise ValueError("User/game data in app folder: " + name)
    meta = json.loads((app / "sce_sys/param.json").read_text())
    if meta.get("titleId") != "PPSA90436" or meta.get("contentVersion") != version:
        raise ValueError("Expected current PPSA90436 mod host " + version)
    with (app / "eboot.bin").open("rb") as stream:
        if stream.read(4) != bytes.fromhex("5414f5ee"):
            raise ValueError("Expected native FSELF app")
    if hashlib.sha256((app / "sce_module/libc.prx").read_bytes()).hexdigest() != RUNTIME_SHA256:
        raise ValueError("Runtime does not match the pinned mod host")
    return files


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--personal-app", type=Path, help="Private locally built PPSA90436 only")
    args = parser.parse_args()
    output = args.output.absolute()
    if output.exists():
        parser.error("Output already exists; choose a new filename")
    output.parent.mkdir(parents=True, exist_ok=True)
    try:
        files = check()
        version = json.loads((ROOT / "ps5/sce_sys/param.json").read_text())["contentVersion"]
        native = personal_files(args.personal_app, version) if args.personal_app else []
        entries = {}
        for path in sorted(files):
            name = path.relative_to(ROOT).as_posix()
            if not name.startswith("site/downloads/"):
                entries["CTPS5/" + name] = (path.read_bytes(), name.endswith(".sh"))
        # When building a kit from a checkout, render offline documentation if
        # the documentation renderer dependency is installed. The Markdown
        # sources and plain-text guides are always available.
        if (ROOT / ".git").exists():
            try:
                import markdown  # noqa: F401
            except ImportError:
                raise ValueError("Install Markdown for offline documentation: python3 -m pip install Markdown")
            subprocess.run([os.sys.executable, str(ROOT / "tools/build_site_docs.py")], check=True)
            subprocess.run([os.sys.executable, str(ROOT / "tools/check_site.py")], check=True)
        for path in sorted((ROOT / "site/docs").rglob("*")):
            if path.is_file():
                entries["CTPS5/" + path.relative_to(ROOT).as_posix()] = (path.read_bytes(), False)
        # The source checkout's website offers older hosted downloads. In an
        # offline kit, direct those links to this kit's actual setup guide.
        if "CTPS5/site/index.html" in entries:
            data = entries["CTPS5/site/index.html"][0].decode()
            data = re.sub(r'href="downloads/[^"#]+"', 'href="docs/RELEASE-KIT.html"', data)
            entries["CTPS5/site/index.html"] = (data.encode(), False)
        theme_root = ROOT / "mods/battle-hud/themes"
        for path in sorted(theme_root.rglob("*")):
            if path.is_file():
                entries["starter-data/eggoftime/mods/battle-hud/themes/" + path.relative_to(theme_root).as_posix()] = (path.read_bytes(), False)
        for source, target in [("docs/MODDING-GUIDE.md", "MODDING-GUIDE.txt"), ("docs/RELEASE-KIT.md", "RELEASE-GUIDE.txt")]:
            entries[target] = ((ROOT / source).read_bytes(), False)
        mode = (
            "PERSONAL PLAY KIT: copy the COMPLETE PPSA90436 folder into\n"
            "/data/homebrew/. Put your own USA/YQUE CT.nds at\n"
            "/data/eggoftime/rom/CT.nds, refresh the launcher and run CTPS5 Mods.\n"
            "The executable contains ROM-derived code: KEEP THIS KIT PRIVATE."
        ) if native else (
            "PUBLIC SOURCE KIT: build locally from your own supported NDS.\n"
            "Follow CTPS5/docs/BUILDING.md to install dependencies, then run\n"
            "bash ps5/build_mod_release.sh /absolute/path/to/CT.nds from CTPS5/.\n"
            "There is no prebuilt game executable in this public kit."
        )
        start = f"""CTPS5 MODS — RELEASE CANDIDATE {version}
==============================================
Read RELEASE-GUIDE.txt for setup and MODDING-GUIDE.txt for PNG mods, themes,
multiple On/Off switches, original fallback, limits and music support.

{mode}

Included: complete authored source under CTPS5/, the offline editor at
CTPS5/tools/mod-editor/index.html, build/package helpers, documentation,
licenses and original Egg of Time 1.3.5 PC source/notices by Power Pudu.

starter-data/ contains optional authored HUD themes. On an existing setup,
copy only wanted new theme folders; do not overwrite your customized themes.
No ROM, saves, controls, hud.ini or pack-states.ini are supplied.

Cronos House Downstairs in CTPS5/modding/templates/ is a scoped configuration
starter, not the finished art mod. It needs the original reference PNG
converted into the game's tile sheets and checked in game.

Replacement music playback is not implemented on PS5 in this version.
Console testing and music-stutter investigation remain pending. This is not
a verified stable or sustained-4K60 release. Unpacking installs/uploads nothing.
"""
        entries["START-HERE.txt"] = (start.encode(), False)
        for path in native:
            entries["PPSA90436/" + path.relative_to(args.personal_app.resolve()).as_posix()] = (path.read_bytes(), False)
        commit = None
        if (ROOT / ".git").exists():
            commit = subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=ROOT, text=True).strip()
            if subprocess.check_output(["git", "status", "--porcelain"], cwd=ROOT, text=True):
                raise ValueError("Commit source changes locally before producing a release kit")
        inventory = {name: {"bytes": len(data), "sha256": hashlib.sha256(data).hexdigest()} for name, (data, _) in sorted(entries.items())}
        manifest = {"schema": 1, "version": version, "source_commit": commit,
                    "distribution": "personal-play-private" if native else "public-authored-source",
                    "console_verified": False, "replacement_music_on_ps5": False,
                    "cronos_house_downstairs_art_included": False, "files": inventory}
        entries["KIT-MANIFEST.json"] = ((json.dumps(manifest, indent=2) + "\n").encode(), False)
        entries["SHA256SUMS"] = ("".join(hashlib.sha256(data).hexdigest() + "  " + name + "\n" for name, (data, _) in sorted(entries.items())).encode(), False)
        if not native:
            for name, (data, _) in entries.items():
                check_bytes(name, data)
        fd, temporary = tempfile.mkstemp(prefix=".ctps5-kit-", suffix=".zip", dir=output.parent)
        os.close(fd)
        try:
            with zipfile.ZipFile(temporary, "w", zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
                for name, (data, executable) in sorted(entries.items()):
                    info = zipfile.ZipInfo(name, date_time=(2026, 11, 16, 0, 0, 0))
                    info.compress_type = zipfile.ZIP_DEFLATED
                    info.external_attr = (0o100755 if executable else 0o100644) << 16
                    archive.writestr(info, data)
            with zipfile.ZipFile(temporary) as archive:
                if archive.testzip() is not None:
                    raise ValueError("ZIP integrity failure")
            os.link(temporary, output)
        finally:
            os.unlink(temporary)
        print(json.dumps({"file": str(output), "bytes": output.stat().st_size,
                          "sha256": hashlib.sha256(output.read_bytes()).hexdigest(),
                          "distribution": manifest["distribution"], "entries": len(entries)}, indent=2))
    except (ValueError, OSError, AssertionError, subprocess.CalledProcessError) as error:
        parser.exit(1, "Release kit rejected: " + str(error) + "\n")


if __name__ == "__main__":
    main()
