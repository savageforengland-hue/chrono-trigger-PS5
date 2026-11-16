# Crono's bedroom: PNG replacement test

The private interior test is a first texture pass on **Crono's upstairs bedroom**.
It refines the wood, furniture, curtains and bedding using the existing map,
tile positions and transparency. It does not change collision, exits, stairs,
actors or map data. This is a tiled graphics pack, not a full-room illustration
renamed as a game file. The downstairs room has not been remastered in this test.

## Exact installation paths

Merge the ZIP's `eggoftime` folder into `/data/`. The six replacement files are:

```
/data/eggoftime/mods/graphics/sheets/Map/Bg/Bg_0135_ncg.bin.png
/data/eggoftime/mods/graphics/sheets/Map/Bg/Bg_0136_ncg.bin.png
/data/eggoftime/mods/graphics/sheets/Map/Bg/Bg_0137_ncg.bin.png
/data/eggoftime/mods/graphics/sheets/Map/Bg/Bg_0138_ncg.bin.png
/data/eggoftime/mods/graphics/sheets/Map/Bg/Bg_0139_ncg.bin.png
/data/eggoftime/mods/graphics/sheets/Map/Bg/Bg_0170_ncg.bin.png
```

These are the exact `sheets/Map/Bg/` filenames in the dump, including the
`_ncg.bin.png` suffix and case. The original sheets are 128x64; these indexed
PNGs are **512x256 (4x)** with the same 16-column tile layout. Palette indexes
retain the room's live colours; the actual game has a more muted palette than
the separate artwork guide. Unedited sheet cells retain their original indexes.
Repeated floor cells use one consistent plank material to avoid furniture
fragments repeating across the floor.

In **L3+R3 -> Mods**, set **Texture mods = On** and **Texture pack = default**.
Fully close and restart, then visit the upstairs bedroom. A selected named
pack such as `crono-png-test` uses its own graphics folder instead. To combine
these sheets with a named pack, put the same `sheets/Map/Bg/` files inside
`mods/packs/<your-pack>/graphics/` instead of the default folder.

The generated `manifest.generated.json` lists the PNGs for reference. It is not
a registration manifest: the runtime discovers the PNGs by their relative
filenames. Replace a PNG under the same path and restart; no AI or JSON edit is
needed. Preserve the full canvas, tile layout and alpha. Keep copies of any
existing files with these names before replacing them.

## Undo and scope

Close the game, remove these six PNGs (or restore the versions you backed up),
and restart. Removing the entire test restores the original bedroom. The ZIP
contains no settings, HUD configuration, ROM or save files. The HUD and earlier
sprite packs remain independent.

The game's tile artwork is shared: other rooms using identical tile bytes may
also show the changed materials. This pack is not isolated by room ID. Other
rooms, transitions, long sessions and performance need console testing.

## Host 01.006.014

The companion native host removes the automatic startup single-sprite write
check at the user's request. It retains the working PNG loader and remastered
HUD, plus the manually selected full graphics dumper. Existing exports are
left in place. The host update is optional for the artwork if PNG replacement
already works in 01.006.013; it is needed to stop that build's startup check.

Desktop checks loaded all six actual files through the real runtime, captured
the bedroom at the same saved frame before/after/removal, and confirmed exact
restoration after removing the files. Native container, runtime and import
checks passed separately. These are offline and desktop results; the new host
and room pack still require a PS5 test.

Original PC source and notices remain under `upstream/`: **Egg of Time v1.3.5
by Power Pudu**. Players supply their own supported USA/YQUE NDS. The private
artwork test is not bundled with the public source package.
