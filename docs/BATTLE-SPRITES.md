# Faithful HD battle sprites — prototype 0.1.0

This optional module enhances the existing player sprites at 2× or 4× detail,
with conservative contour refinement and subtle live-palette shading. It is
an **enhancement prototype, not the illustrated character redraw shown in the
art-direction concept**. That detailed redraw remains unfinished and needs
authored artwork for the original poses. The actual comparison exported by
the tool below shows what this implementation produces.

Built on **Egg of Time v1.3.5 by Power Pudu**; see [credits](CREDITS.md).
Each player supplies their own supported USA/YQUE `CT.nds`. No game sprites,
ROM or generated game executable belong in the public source or mod pack.

## Separate module

The configuration-only module contains:

```
/data/eggoftime/mods/battle-sprites/mod.json
/data/eggoftime/mods/battle-sprites/sprites.ini
```

It requires **CTPS5 Mods host 01.006.005**, title **PPSA90436**. Copying this
configuration into an older host does not add its renderer. Update the
matching host app files under `/data/homebrew/PPSA90436/` while the app is
closed, and refresh ShadowMountPlus. Preserve all user settings, saves and
the existing portrait HUD's INI. The base app PPSA90435 remains installed.
Both apps share `/data/eggoftime/` user data.

This module has its own switch and works alongside the portrait HUD and a
named texture pack. Existing HD replacement tiles from that pack take
precedence. In **L3+R3 → Mods**, leave **Texture mods On** and set **HD battle
sprites On**, then fully restart the host. Disable HD battle sprites and
restart to restore the original sprites. All module changes need a restart.

```ini
enabled=1
scale=4
shading=65
```

`scale` accepts 2 or 4 and changes detail, not the actor's screen size.
`shading` accepts 0–100. It mixes nearby colours from the live game palette;
it does not add painted detail or a new lighting engine. Use 2× for lower
memory/workload. The 4× source tile data occupies about 29 MiB in the tested
USA ROM, plus the host's existing HD-frame/compositing buffers.

## Animation and display behavior

The module covers the seven main character sheets: Crono, Marle, Lucca,
Robo, Frog, Ayla and Magus. It builds HD tiles in memory, keyed by the
original bytes, with a character mask for shared tiles. It does not rewrite
the ROM, animation records, cell layouts, durations, frame order, actor
positions, flips, battle logic or saves. Thin lines and the original opaque
footprint are retained. Blank tiles stay blank.

Generated tiles require a battle scene, a sprite layer, and a palette slot
identified as the corresponding playable character. Unknown slots fall back
to original pixels. Palette identity persists through colour changes within
the battle and resets on exit; normal player palettes can rebind a slot.
The HD texels refer to live palette indices rather than fixed RGB artwork,
retaining palette colour changes, white flashes and black fades. The existing
compositor still applies brightness, blending, occlusion and flips.

Affine sprites retain the renderer's original fallback. NPCs using shared
player artwork and palettes during a battle may receive the same enhancement.
This is not a complete hand-painted replacement for every pose or variant.

## Checks and remaining work

The private own-ROM audit rendered **1,685 cells**: 248 Crono, 241 Marle,
244 Lucca, 198 Robo, 245 Frog, 255 Ayla and 254 Magus. It checked that ROM
memory remained identical and no enhanced pixels extended outside original
transparency. Empty or unchanged cells are legitimate and remain unchanged.
This audits cell coverage; it is **not** a playthrough of every animation.

Authored tests check 2×/4× bounds, alpha, live colours, hit flashes/fades and
sprite/battle/character guards. Existing HUD and host-menu tests cover the
combined menu, persisted sprite toggle and preservation of other settings.
Desktop battle captures test the actual host render path. Neither those nor
native-container checks establish PS5 correctness or sustained 4K60. PS5
visual/performance testing and extended battle coverage remain necessary.

```sh
python3 ps5/test_sprite_hd.py --sdl-source /path/to/SDL3
python3 ps5/test_battle_hud.py --sdl-source /path/to/SDL3
python3 ps5/test_system_menu.py --sdl-source /path/to/SDL3
```

Build an own-ROM audit/preview tool (Linux, ELF section garbage collection):

```sh
cc -std=c11 -O2 -ffunction-sections -fdata-sections \
  -I /path/to/SDL3/include -I runtime \
  tools/preview_battle_sprites.c runtime/sprite_hd.c runtime/gfx_mod.c \
  runtime/gfx_rom.c runtime/ui_font.c -Wl,--gc-sections -o /tmp/sprite-preview
/tmp/sprite-preview /path/to/your/CT.nds /path/to/your/data/ /tmp/private-sprites.ppm
```

The data root needs its own `mods/battle-sprites/sprites.ini` and a trailing
slash. Keep its game-derived output, ROM, generated game C and executables
private. The tool renders every cell and exports an original/enhanced
comparison; it does not generate a redistributable texture atlas.
