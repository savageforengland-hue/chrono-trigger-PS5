#!/usr/bin/env bash
# Build the DS game with the experimental full-PNG renderer and stage a
# DIFFERENT application title so the installed working build stays untouched.
set -euo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
ROM="${1:?Usage: bash ps5/build_fullpng.sh /absolute/path/to/your/CT.nds}"
: "${PS5_PAYLOAD_SDK:?Set PS5_PAYLOAD_SDK to your installed SDK prefix}"
: "${SDL3_SOURCE:?Set SDL3_SOURCE to the pinned OpenAGC SDL3 checkout}"
command -v dsd >/dev/null || { echo "Missing dsd; follow docs/BUILDING.md" >&2; exit 1; }
test -f "$ROM" || { echo "ROM not found: $ROM" >&2; exit 1; }
test -x "$PS5_PAYLOAD_SDK/bin/prospero-clang" || {
    echo "No PS5 toolchain: $PS5_PAYLOAD_SDK/bin/prospero-clang" >&2; exit 1;
}
test -f "$ROOT/runtime/fullpng.c" || { echo "Full-PNG sources not present" >&2; exit 1; }
test -f "$ROOT/runtime/fullpng.h" || { echo "Full-PNG headers not present" >&2; exit 1; }
test -f "$SDL3_SOURCE/include/SDL3/SDL.h" || { echo "SDL3_SOURCE is not a usable SDL3 checkout" >&2; exit 1; }
OUT="${EGG_BUILD_DIR:-$ROOT/build-ps5}"
export EGG_BUILD_DIR="$OUT"
echo "Building native DS port with PS5 SDK. Existing installed apps will not be modified."
bash "$ROOT/ps5/build.sh" "$ROM"
echo "Checking native build outputs and staging separate full-PNG test title..."
python3 "$ROOT/ps5/stage_fullpng_test.py" \
    "$OUT/package/PPSA90435" "$OUT/fullpng-test/PPSA90440"
echo "Build ready to copy for console testing: $OUT/fullpng-test/PPSA90440"
echo "THIS HAS NOT BEEN TESTED ON PS5. Keep your working title installed."
