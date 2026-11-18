#!/usr/bin/env python3
"""Stage a separately titled test application from a successfully built CTPS5 folder.
Does not patch, rebuild, or substitute a binary. It refuses missing build outputs.
"""
import argparse
import hashlib
import json
import shutil
from pathlib import Path

TEST_TITLE = "PPSA90440"

def stage(source: Path, destination: Path):
    source = source.resolve()
    for rel in ("eboot.bin", "sce_sys/param.json", "sce_sys/icon0.png",
                "sce_module/libc.prx"):
        if not (source / rel).is_file():
            raise FileNotFoundError("Native build missing " + str(source / rel))
    if destination.exists():
        raise FileExistsError("Refusing to overwrite existing test folder: " + str(destination))
    data = (source / "eboot.bin").read_bytes()
    if data[:4] != bytes.fromhex("5414f5ee"):
        raise ValueError("eboot.bin has no PS5 SELF magic (EEF51454)")
    shutil.copytree(source, destination)
    meta = json.loads((destination / "sce_sys/param.json").read_text())
    meta["titleId"] = TEST_TITLE
    meta["conceptId"] = "90440"
    meta["contentId"] = "UP9000-PPSA90440_00-CTFULLPNGTEST000"
    meta["contentVersion"] = "01.007.000"
    meta["localizedParameters"]["en-US"]["titleName"] = "CTPS5 Full PNG Test"
    (destination / "sce_sys/param.json").write_text(json.dumps(meta, indent=2) + "\n")
    manifest = {
        "title": TEST_TITLE, "source": str(source),
        "ps5_console_tested": False, "fullpng_native_compile_verified": False,
        "files": {}
    }
    for f in sorted(destination.rglob("*")):
        if f.is_file():
            manifest["files"][str(f.relative_to(destination))] = hashlib.sha256(f.read_bytes()).hexdigest()
    (destination / "BUILD_MANIFEST.json").write_text(json.dumps(manifest, indent=2) + "\n")
    return manifest

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path, help="Existing newly built PPSA90435 folder")
    parser.add_argument("destination", type=Path, help="New PPSA90440 test folder")
    args = parser.parse_args()
    result = stage(args.source, args.destination)
    print("Created separate test folder:", args.destination)
    print("eboot SHA-256:", result["files"]["eboot.bin"])
    print("Console verification: required; do not overwrite any working title")

if __name__ == "__main__":
    main()
