#!/usr/bin/env bash
set -euo pipefail
root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
: "${PS5_PAYLOAD_SDK:?Set PS5_PAYLOAD_SDK to the public SDK prefix}"
: "${SDL3_SOURCE:?Set SDL3_SOURCE to the pinned OpenAGC/SDL3 checkout}"
rom="${1:?Usage: ps5/build.sh /path/to/CT.nds}"
rom="$(realpath -- "$rom")"
build="${EGG_BUILD_DIR:-$root/build-ps5}"
python="${PYTHON:-python3}"
cmake="${CMAKE:-cmake}"
dsd="${DSD:-dsd}"
"$python" - "$rom" <<'PY'
import hashlib, sys
with open(sys.argv[1], 'rb') as f:
    digest = hashlib.file_digest(f, 'sha1').hexdigest()
if digest != '11f9e8204e484eab42181b210c9f214584be75bf':
    raise SystemExit('Expected the unmodified USA YQUE ROM; received SHA-1 ' + digest)
PY
mkdir -p "$build"
if git -C "$SDL3_SOURCE" apply --check "$root/ps5/sdl3-static.patch" 2>/dev/null; then
    git -C "$SDL3_SOURCE" apply "$root/ps5/sdl3-static.patch"
elif ! git -C "$SDL3_SOURCE" apply --reverse --check "$root/ps5/sdl3-static.patch" 2>/dev/null; then
    echo 'SDL3 checkout does not match the supplied patch.' >&2
    exit 1
fi
"$dsd" rom extract --rom "$rom" --output-path "$build/extract"
"$python" -I "$root/tools/recomp/recomp.py" "$root" "$build/gen" --extract "$build/extract" --no-comments
"$cmake" -S "$SDL3_SOURCE" -B "$build/sdl3" -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE="$PS5_PAYLOAD_SDK/toolchain/prospero.cmake" \
    -DCMAKE_BUILD_TYPE=Release -DSDL_SHARED=OFF -DSDL_STATIC=ON \
    -DSDL_OPENGL=OFF -DSDL_OPENGLES=OFF -DSDL_VULKAN=OFF -DSDL_GPU=OFF \
    -DSDL_SYSTEM_ICONV=OFF -DSDL_TEST_LIBRARY=OFF -DSDL_TESTS=OFF -DSDL_EXAMPLES=OFF
"$cmake" --build "$build/sdl3" -j "${EGG_JOBS:-2}"
"$cmake" -S "$root/ps5" -B "$build/app" -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE="$PS5_PAYLOAD_SDK/toolchain/prospero.cmake" \
    -DCMAKE_BUILD_TYPE=Release -DEGG_PS5_SYSTEM_KEYBOARD=OFF -DEGG_GEN="$build/gen" \
    -DSDL3_SOURCE="$SDL3_SOURCE" -DSDL3_BUILD="$build/sdl3"
"$cmake" --build "$build/app" -j "${EGG_JOBS:-2}"
mkdir -p "$build/package/PPSA90435"
cp -R "$root/ps5/sce_sys" "$build/package/PPSA90435/"
"$python" "$root/ps5/link_native.py" \
    --build "$build/app" --sdl-build "$build/sdl3" --sdk "$PS5_PAYLOAD_SDK" \
    --out "$build/package/PPSA90435/eboot.bin"
echo "Built: $build/package/PPSA90435 (hardware validation required)"
"$python" "$root/ps5/build_boot_check.py" --sdk "$PS5_PAYLOAD_SDK" \
    --tool "$build/app/native-tools/ps5-native-tool" --build "$build/boot-check" \
    --runtime "$build/package/PPSA90435/sce_module/libc.prx" \
    --out "$build/package/PPSA90436"
echo "Built: $build/package/PPSA90436 (independent home-screen boot check)"
