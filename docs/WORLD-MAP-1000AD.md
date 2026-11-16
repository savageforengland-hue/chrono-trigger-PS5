# 1000 AD world-map remaster test

Private artwork pack **1000ad-remaster v0.1.0**, requiring mod host **01.006.018**.
The port's original world-map tiles, map entries, collision, entrances and event
code remain authoritative. The pack adds 4× palette-relative terrain/building
detail while retaining native character sprites, clouds, moving map objects
and animation data. It is a faithful pixel-art enhancement of the initial
1000 AD map, rather than a new world layout.

## Installation

Install the matching **PPSA90436** host update first, keeping `/data/eggoftime/`
intact. Merge the mod ZIP's `eggoftime` folder into `/data/`. Then open
**L3+R3 → Mods → Backgrounds**, turn Texture mods On and switch
**1000ad-remaster** On with A. Fully close and restart the app to load it.
The pack is also listed under Texture Packs. House and character packs can
remain On at the same time; see [independent switches](MODS-MULTIPLE-PACKS.md).
HUD configuration remains independent. No INI, ROM or save files are included.

```text
/data/eggoftime/mods/packs/1000ad-remaster/graphics/
  backgrounds.json
  manifest.generated.json
  sheets/WorldMap/
    WorldBg_0000_ncg.bin.png
    WorldBg_0001_ncg.bin.png
    WorldBg_0002_ncg.bin.png
    WorldBg_0003_ncg.bin.png
    WorldBg_0004_ncg.bin.png
    WorldBg_0005_ncg.bin.png
    WorldBg_0006_ncg.bin.png
```

All seven PNGs retain the exact dumped names and 16-tile-wide sheet layout.
Their native canvas is 128×64; this pack uses **512×256 (4×)** for every sheet.
`manifest.generated.json` is the generated inventory, not a registration list.
Additional same-path PNG replacements are discovered at startup.

## Map restriction and live colours

The optional policy in `backgrounds.json` contains:

```json
{
  "schema": 1,
  "transparentTiles": "original",
  "includeCharacterSheets": false,
  "backgroundBlendWeight": 96,
  "locations": [{"id": 496}]
}
```

Location **496 (0x1f0)** was verified by entering the 1000 AD overworld in the
actual port and tracing its WorldBg, Chip_0000 and Map_0000 asset reads. It is
not guessed from the PNG numbers. The host extends location restrictions to
`sheets/WorldMap/WorldBg_…` alongside existing Map/Bg support. Missing or invalid
present policy files follow the existing fallback rules; invalid present rules
skip the scoped images. Outside permitted locations, HD-only pack processing
is inactive. The two-room house policy and its curtain light behaviour retain
their existing defaults.

The new **optional** `backgroundBlendWeight` field is an integer from 1 to 127.
Absent means ordinary indexed PNG behaviour. With the field present, an indexed
4-bpp background PNG may use native indices 1–15 directly, or entries 16–240
for two-index detail blends. Index 0 remains transparent. For entry `v`, the
base index is `1 + (v-16)/15` and the second is `1 + (v-16)%15`. Weight 96 means
62.5% base colour and 37.5% second colour, evaluated from the game's live palette.
Entries 241–255 retain the native texel. Sprite blending keeps its independent
existing field. Ordinary RGBA PNGs remain supported without blend decoding.

For this authored pack, every nonempty tile retains the native base index and
transparent mask at every 4× texel. Repeated byte-identical tile keys share art.
Empty sheet cells use original assets. This protects original geometry and
blue/magenta palette variants while allowing fine texture detail. Unlisted
story-dependent tiles retain original graphics. Keep the supplied policy when
using the indexed art; dropping another PNG under the same filename needs no
registration or AI step. Preserve its tile layout and canvas.

## Previews and validation

The 3840×2160 port capture is an actual desktop run of the PS5-profile runtime.
The full-map tile preview is reconstructed from the real native world maps,
VRAM, live palettes and the loaded replacement tiles; it is not a gameplay
screenshot. Separate 3840×2560/3840×2160 artistic concept exports retain their
role as visual guides. A full-map preview PNG is a viewing file; install the
mod ZIP for the working tile replacements.

Host checks cover actual PNG decoding, live-palette pair encoding, RGBA edits,
empty/invalid fallback, other-location/sprite exclusion, unchanged ROM bytes,
menu classification, previous house/curtain rules, HD filters, audio diagnostics
and native package checks. The private delivery report records exact counts
and capture checks. PS5 hardware navigation, performance and long-session
coverage remain pending. This update makes no claim to fix the reported
music breakup during Aura Whirl; the existing bounded audio diagnostics remain.
