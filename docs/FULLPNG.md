# Full-canvas PNG rooms (experimental, native PS5/SDL3)

This optional renderer keeps the DS game engine, scripts, collision, tile maps, UI, and
native sprite priorities intact, but replaces the **visible pixels** of a background
layer using an artist-authored full-canvas RGBA PNG instead of HD tiles selected by
tile hashes.

## How to use it

Place a PNG in the existing graphics mod directory:

```text
/data/eggoftime/mods/graphics/rooms/0x1f0_bg2_x2.png
```

Filename = `0x<location>_bg<0..3>_x<scale>.png`.

- `0x1f0` is an **illustrative** location ID. Replace it with a verified field-room ID; this example does not identify a confirmed room.
- `bg2` is one of the DS engine's background layers; BG2 is the main map in
  many field rooms, but the appropriate BG may vary by scene.
- `x2` tells the loader the art is exactly 2x the native world dimensions.
  Supported factors are 2 through 8. A 1536x1024 native room canvas is
  3072x2048 pixels at x2.
- PNG alpha below 128 at a native sampling point leaves the legacy tile
  pixel in place. At or above 128, the PNG replaces that BG pixel. The
  upscaled compositor respects per-texel alpha for edge blending.
- The PNG must be aligned to **room-map coordinates** (0,0 at the native
  room map's origin). The renderer reconstructs each BG's scroll position
  from its tile ring, including independently moving layers. This first
  version has no artist-defined origin offset. Start with BG2 in a simple room.
- Image native dimensions must not exceed 4096x4096. Total uncompressed
  PNG memory budget is 128 MiB, with at most 127 images loaded per restart.

Named packs can include `graphics/rooms/` alongside other PNGs, using the existing `pack.json` and `ps5/install_texture_pack.py` workflow.

No PNG files are distributed here. Use your own artwork.

## Optional world-space alignment sidecar

A PNG can be accompanied by a `.cfg` file with the same basename:

```text
mods/graphics/rooms/0x001_bg2_x2.png
mods/graphics/rooms/0x001_bg2_x2.cfg
```

```ini
origin_x=0
origin_y=0
```

`origin_x` and `origin_y` specify the PNG's top-left in the original
DS game's **world pixels**, rather than texture pixels. Adjust these two
values and restart the test title without editing the PNG or rebuilding
`eboot.bin`. Valid offsets are -4096 to +4096. The default is zero.

The approved downstairs image is currently 1254x1254 at x2, producing a
627x627 native canvas. Actual camera coordinates, room dimensions, scene
identity and BG layer must still be checked against the running DS game.
The runtime logs the current scene and camera on room transitions:

```text
fullpng: scene 0x001, camera (X,Y)
```

The line above is an example of the format, not an observed PS5 result.
The filename `0x001_bg2_x2.png` is an **unverified candidate** for Crono's
downstairs room in the DS runtime. Do not assume this scene ID or BG2 is
correct without a console log and visual inspection.

## Compatibility

- Full-canvas PNG art is optional. Without files in `rooms/`, the original
  tile renderer and existing HD mod packs continue to work.
- The GPU/CPU composition preserves the DS layer priority, OAM sprites,
  windows, fades and most original blending operations. The collision,
  triggers, interactions, camera rules and gameplay still use original maps.
- The first version targets **field rooms only**, not the overworld. Affine and
  bitmap BG layers use the existing renderer unchanged.
- Not all game effects, animation sequences, layer offsets, translucency
  interactions or scene transitions have been verified. Test at least one
  room in hardware before replacing large portions of the world.
- Images are loaded once at mod startup; restart after changes.
- The PNG loader shares the normal graphics-mod root. Named mod-pack
  installers must stage/merge `rooms/` into the effective graphics root.

## Rebuild the experimental PS5 executable

This repository's native build depends on the same SDK, pinned SDL3 source
and Nintendo DS USA YQUE dump as the working CTPS5 source. It cannot rebuild
from an existing `eboot.bin` alone.

On an SDK-equipped Linux machine:

```sh
bash tools/bootstrap-ps5-deps.sh
export PS5_PAYLOAD_SDK="$PWD/.deps/sdk/opt/ps5-payload-sdk"
export SDL3_SOURCE="$PWD/.deps/SDL3"
export LLVM_CONFIG=llvm-config-19
export DSD=dsd
bash ps5/build_fullpng.sh /absolute/path/to/your/CT.nds
```

### Rebuilding later without the ROM as a build input

If you've **kept a previous `build-ps5/gen/` directory** that contains
`dispatch.c` and the other generated C sources, subsequent source changes
can be compiled with:

```sh
export PS5_PAYLOAD_SDK="$PWD/.deps/sdk/opt/ps5-payload-sdk"
export SDL3_SOURCE="$PWD/.deps/SDL3"
bash ps5/rebuild_cached.sh
```

The script does not invoke `dsd`, does not invoke `recomp.py`, and does
not read or hash a DS ROM. It needs the retained generated source, **the PS5
SDK**, SDL3 and a compatible Linux build host. If the generated C is missing,
it stops rather than assembling an incomplete game. Your pre-existing
`eboot.bin` is not a substitute for generated C or relocatable object files.

The game's code is still linked into the new `eboot.bin`. The game still
needs your own original DS ROM *at runtime* for assets and data. This is
not a ROM-free game.

The script invokes the *existing* native CRT/SDL3/SELF pipeline and stages
a **separate** `build-ps5/fullpng-test/PPSA90440` test folder. The
existing release folders are not overwritten. The rebuilt test application
should be copied to `/data/homebrew/PPSA90440` and registered using the
same ShadowMount method you used for the working DS title. Back up all
saves and data before testing. Do not merge into the working title until
a console test is successful.

`CT.nds` is your own private dump, is used only on your build machine,
and must not be committed or uploaded to GitHub.

The SHA-1 requirement is the same as `ps5/build.sh`: USA YQUE,
`11f9e8204e484eab42181b210c9f214584be75bf`.
If you lack the ROM, SDK, SDL source, dsd, or LLVM, the build cannot
produce a new working PS5 executable. Passing GitHub's host checks is
not proof that the final executable boots on hardware.

## Rebuild requirement

The *engine code* needs one new PS5 `eboot.bin` build to gain this feature.
After that, changing or adding PNGs **does not require another rebuild**.
Do not overwrite your known-good `PPSA90436` folder during testing.

## Engineering details

`runtime/fullpng.c` loads immutable surfaces with SDL3's `SDL_LoadPNG`.
`video.c` uses the room camera and the game location ID to associate
each drawn BG pixel with a full-image coordinate. The image source is
encoded in the existing `VideoHd.tile` 32-bit provenance field. The
ordinary compositor still resolves BG priority against sprites, text
boxes, windows and other layers. `display.c` then samples the
full-resolution PNG by its **world coordinates**, without hashing or
reassembling 8x8 tiles.

`FULLPNG_FLAG` (bit 31) separates full-image sources from legacy
tile-hash sources; bits 24-30 give the image ID, 12-23 the x coordinate
and 0-11 the y coordinate.

These changes are source-only, not a tested PS5 launch.
