#!/usr/bin/env python3
"""Tests for a safe ROM-input-free cached PS5 rebuild wrapper."""
from pathlib import Path
import os
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
script = root / "ps5" / "rebuild_cached.sh"
s = script.read_text()
assert 'rom extract' not in s, "Must not invoke the ROM extractor"
assert 'recomp.py' not in s, "Must not generate C from a ROM"
assert 'link_native.py' in s, "Must reuse native PS5 linker"
assert 'stage_fullpng_test.py' in s, "Must use isolated app title"
assert 'EGG_CACHED_GEN' in s, "Must allow cached generated C from another build"
assert 'dispatch.c' in s, "Must require precompiled game source"
assert 'PPSA90440' in s, "Must not overwrite working app"
assert 'PS5_PAYLOAD_SDK' in s
assert 'SDL3_SOURCE' in s
subprocess.run(["bash", "-n", str(script)], check=True)
with tempfile.TemporaryDirectory() as td:
    # A mock SDK/source is enough to confirm early failure. No games or SDK
    # files are installed, and this test must never invoke cmake/link.
    tmp = Path(td)
    (tmp / "sdk").mkdir()
    (tmp / "sdl").mkdir()
    (tmp / "build").mkdir()
    env = dict(os.environ,
        PS5_PAYLOAD_SDK=str(tmp / "sdk"),
        SDL3_SOURCE=str(tmp / "sdl"),
        EGG_BUILD_DIR=str(tmp / "build"),
        EGG_CACHED_GEN=str(tmp / "build" / "gen"))
    r = subprocess.run(["bash", str(script)], text=True,
                       capture_output=True, env=env, check=False)
    assert r.returncode == 2, (r.returncode, r.stderr)
    assert "Missing" in r.stderr and "dispatch.c" in r.stderr
    assert not (tmp / "build" / "fullpng-test").exists()
print("PASS: ROM-free cached rebuild validation and safe missing-cache failure")
