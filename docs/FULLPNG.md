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
