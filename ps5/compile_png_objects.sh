#!/bin/sh
# Compile the PNG rendering integration as actual PS5 target C objects.
# No Nintendo DS ROM, executable, SDK redistribution or PS5 hardware.
set -eu
: "${PS5_PAYLOAD_SDK:?Set PS5_PAYLOAD_SDK to the PS5 Payload SDK prefix}"
: "${SDL3_SOURCE:?Set SDL3_SOURCE to the pinned CTPS5 SDL3 source}"
sdk="$PS5_PAYLOAD_SDK"
src="$SDL3_SOURCE"
test -x "$sdk/bin/prospero-clang"
test -f "$src/include/SDL3/SDL.h"
mkdir -p build-ps5-fullpng
for file in fullpng.c gfx_mod.c video.c display.c widescreen.c; do
    echo "Compiling PS5 target: runtime/$file"
    "$sdk/bin/prospero-clang" \
        -std=c11 -O1 -fPIC -fwrapv -ffunction-sections -fdata-sections \
        -DEGG_PS5=1 -DEGG_PS5_CORE=1 -DEGG_STATIC_GAME=1 -DEGG_SINGLE_SCREEN=1 \
        -DSDL_MAIN_HANDLED=1 -DEGG_VERSION='"png-ci"' \
        -Iruntime -Ips5 -I"$src/include" \
        -I"$sdk/target/user/homebrew/include" \
        -c "runtime/$file" -o "build-ps5-fullpng/${file%.c}.o"
done
echo "PS5 target graphics C compilation complete; SDL linking / game / eboot not tested."
