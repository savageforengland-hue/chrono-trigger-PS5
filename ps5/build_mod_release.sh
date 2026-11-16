#!/usr/bin/env bash
# Build the current mod host locally from the player's own verified NDS.
set -euo pipefail
root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
rom="${1:?Usage: ps5/build_mod_release.sh /path/to/CT.nds}"
build="${EGG_BUILD_DIR:-$root/build-ps5}"
python="${PYTHON:-python3}"
# PPSA90436 belongs to CTPS5 Mods in this workflow, not the boot-check app.
EGG_BUILD_BOOT_CHECK=0 bash "$root/ps5/build.sh" "$rom"
out="$build/package-mods/PPSA90436"
"$python" "$root/ps5/package_mod_host.py" "$build/package/PPSA90435" "$out"
echo "Built CTPS5 Mods: $out"
echo "Keep the generated executable private. See docs/RELEASE-KIT.md."
