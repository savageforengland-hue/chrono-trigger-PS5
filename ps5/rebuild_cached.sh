#!/usr/bin/env bash
# Rebuild the complete PS5 eboot from retained *generated C sources*.
# No DS ROM is read or extracted by this script. A prior ROM-based generation
# is still necessary, and the PS5 game still reads a ROM for assets at runtime.
set -euo pipefail

root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
: "${PS5_PAYLOAD_SDK:?Set PS5_PAYLOAD_SDK to the public SDK prefix}"
: "${SDL3_SOURCE:?Set SDL3_SOURCE to the pinned OpenAGC SDL3 checkout}"
build="${EGG_BUILD_DIR:-$root/build-ps5}"
gen="${EGG_CACHED_GEN:-$build/gen}"
app="$build/app-cached"
sdl="$build/sdl3"
out="$build/fullpng-test/PPSA90440"
python="${PYTHON:-python3}"
cmake="${CMAKE:-cmake}"

test -f "$gen/dispatch.c" || {
    echo "Missing $gen/dispatch.c. No generated game code available to rebuild from." >&2
    echo "Your old eboot.bin cannot substitute for these C files." >&2
    exit 2
}
test -f "$SDL3_SOURCE/include/SDL3/SDL.h" || {
    echo "SDL3_SOURCE does not contain SDL3 headers: $SDL3_SOURCE" >&2
    exit 2
}
test -f "$PS5_PAYLOAD_SDK/toolchain/prospero.cmake" || {
    echo "PS5 Payload SDK toolchain missing; cannot build native eboot.bin" >&2
    exit 2
}
test -x "$PS5_PAYLOAD_SDK/bin/prospero-clang" || {
    echo "PS5 compiler missing: $PS5_PAYLOAD_SDK/bin/prospero-clang" >&2
    exit 2
}
test -f "$root/runtime/fullpng.c" || {
    echo "The full-PNG renderer is absent from this source checkout." >&2
    exit 2
}
# No ROM paths are accepted, requested or accessed here.
# Never repurpose a previously compiled SELF as object code.
mkdir -p "$build"
if git -C "$SDL3_SOURCE" apply --check "$root/ps5/sdl3-static.patch" 2>/dev/null; then
    git -C "$SDL3_SOURCE" apply "$root/ps5/sdl3-static.patch"
elif ! git -C "$SDL3_SOURCE" apply --reverse --check "$root/ps5/sdl3-static.patch" 2>/dev/null; then
    echo "SDL3 source differs from the pinned checkout (patch cannot be applied)." >&2
    exit 2
fi
"$cmake" -S "$SDL3_SOURCE" -B "$sdl" -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE="$PS5_PAYLOAD_SDK/toolchain/prospero.cmake" \
    -DCMAKE_BUILD_TYPE=Release -DSDL_SHARED=OFF -DSDL_STATIC=ON \
    -DSDL_OPENGL=OFF -DSDL_OPENGLES=OFF -DSDL_VULKAN=OFF -DSDL_GPU=OFF \
    -DSDL_SYSTEM_ICONV=OFF -DSDL_TEST_LIBRARY=OFF -DSDL_TESTS=OFF -DSDL_EXAMPLES=OFF
"$cmake" --build "$sdl" -j "${EGG_JOBS:-2}"
"$cmake" -S "$root/ps5" -B "$app" -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE="$PS5_PAYLOAD_SDK/toolchain/prospero.cmake" \
    -DCMAKE_BUILD_TYPE=Release -DEGG_PS5_SYSTEM_KEYBOARD=OFF -DEGG_GEN="$gen" \
    -DSDL3_SOURCE="$SDL3_SOURCE" -DSDL3_BUILD="$sdl"
"$cmake" --build "$app" -j "${EGG_JOBS:-2}"

# Native builder creates a new, isolated SELF title. Never overwrite the
# working installed PPSA90436 app, or any existing test output.
test ! -e "$out" || {
    echo "Test output already exists: $out; refusing overwrite." >&2
    exit 2
}
staging="$build/fullpng-rebuild-candidate/PPSA90435"
test ! -e "$staging" || {
    echo "Candidate staging already exists: $staging; refusing overwrite." >&2
    exit 2
}
mkdir -p "$staging"
cp -R "$root/ps5/sce_sys" "$staging/"
"$python" "$root/ps5/link_native.py" \
    --build "$app" --sdl-build "$sdl" --sdk "$PS5_PAYLOAD_SDK" \
    --out "$staging/eboot.bin"
"$python" "$root/ps5/stage_fullpng_test.py" "$staging" "$out"
echo "Native eboot rebuilt from cached generated C, no ROM read by build."
echo "Test folder: $out"
echo "ROM still required at /data/eggoftime/rom/CT.nds at runtime."
echo "Hardware launch remains unverified."
