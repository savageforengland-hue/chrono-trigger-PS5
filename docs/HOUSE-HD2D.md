# Crono's house: HD-2D style test, host 01.006.015

This private PNG pack refines **both interiors**: the upstairs bedroom and the
downstairs kitchen, dining area, stairs and seating. Painted full-colour room
textures replace the earlier palette-limited pass. Crono, his mum and the cat
family receive a faithful 4x edge/shading refinement using their original
animation shapes. The character reference illustration is a style guide;
the installed sprites retain the game's designs and poses, rather than using
newly invented walking or interaction frames.

This is a 2D texture/sprite test, not a new 3D renderer. The original curtain
art and animation are deliberately retained to protect closing and opening.
The optional window-light rule reads the original curtain state: closing dims
the room and actors to 65% over 18 native frames, and opening restores daylight.
It tints the world layers together; the separate HUD, minimap and UI layers
retain their brightness. Static room artwork is based on aligned guides,
converted into the original tile layouts. Shared tiles use consistent material art. Camera, exits,
collision, actor positions and animation timing are unchanged.

## Install

Use **host 01.006.015** with this pack. Older hosts do not understand its room
restrictions and indexed sprite blends. Keep the previous PPSA90436 app folder
for rollback; replace its executable, matching runtime and sce_sys together.
The base app PPSA90435 is separate. Keep `/data/eggoftime/` and all saves,
settings, controls, ROM and existing packs.

Merge the pack ZIP's `eggoftime` folder into `/data/`. Its independent root is:

```
/data/eggoftime/mods/packs/crono-house-hd2d/graphics/
  backgrounds.json
  manifest.generated.json
  sheets/
    Map/Bg/Bg_NNNN_ncg.bin.png
    Chara/Chara_NNNN.NCGR.png
```

All PNGs retain exact dump-relative names and sheet layouts. The pack includes
22 background sheets and six character sheets; the character sheets cover
Crono, mum and the cat graphic variants loaded by the downstairs scene.
Multiple background files contain shared keys. Transparent, unused HD cells
keep the original graphics under this pack's rules.

In **L3+R3 -> Mods**, select **Texture mods = On** and **Texture pack =
crono-house-hd2d**, then fully close and restart. Do not merge the new rule
file into an unrelated pack by accident: it deliberately limits its sheets
to the house. The earlier default-folder house test can remain installed;
it is inactive while this named pack is selected. The HUD's theme/settings
remain independent.

## Editing PNGs without AI

Replace any supplied PNG under the same name/path and restart. Preserve its
whole canvas, transparent padding and tile arrangement. These are sheets,
not one standalone character or furniture image per file. Ordinary RGBA
replacement art is accepted. The supplied character PNGs use an indexed
palette with live two-colour blends; retaining that indexed format preserves
the game's palette changes. Exporting custom art as RGBA instead uses the
colours painted into that PNG.

`manifest.generated.json` is an automatically generated inventory, not a
registration manifest. `backgrounds.json` is the operative, optional room
policy. Keep the supplied policy for this test. In normal packs without
that file, the existing global PNG behavior is retained.

The policy uses schema 1, `transparentTiles: "original"`, and location objects
with IDs **1 (downstairs)** and **2 (bedroom)**. `includeCharacterSheets`
opts `sheets/Chara/` into the same room restrictions; `spriteBlendWeight: 36`
selects the shipped indexed sprite format. Each location's optional
`paletteLight` array records 16 native palette-row reference sums so painted
backgrounds can follow palette brightness changes. Zero disables a row's
adjustment. The optional `windowLight` object adds a native background/map-cell
probe, expected open/closed tile IDs and control value, `closedBrightness`
(percent) and `fadeFrames`. Unknown intermediate tiles hold the previous
lighting target; unrelated scenes/controls reset the effect. End users can
change the closed-room brightness or fade time without editing PNGs or using
AI. PNG replacement needs no JSON edits.

The policy applies to HD sheets under `sheets/Map/Bg/` and, when opted in,
`sheets/Chara/`. It does not change independent HUD themes or the ordinary
per-pose drop-in module. Scoped sheets must be equal whole-number HD scales
2x through 8x. Missing or all-transparent tiles fall back to original art.
A malformed present policy skips scoped source categories instead of making
room art global. Optional battle sprite refinement retains a context-specific
fallback when its tile bytes are also used by house art.

## Verified house character sources

Actual game cartridge reads on entering downstairs loaded:

| Graphic source | In-game palette | Use |
| --- | --- | --- |
| `Chara/Chara_0000.NCGR` | `Chara/ObjPlt_0000.NCLR` | Crono |
| `Chara/Chara_0044.NCGR` | `Chara/ObjPlt_0009.NCLR` | Mum |
| `Chara/Chara_0066.NCGR` | `Chara/ObjPlt_0161.NCLR` | Cat family |
| `Chara/Chara_0183.NCGR` | `Chara/ObjPlt_0162.NCLR` | Cat family |
| `Chara/Chara_0182.NCGR` | `Chara/ObjPlt_0163.NCLR` | Cat family |
| `Chara/Chara_0184.NCGR` | `Chara/ObjPlt_0164.NCLR` | Cat family |

An asset number does not generally imply the same numbered palette. The
full naming TXT includes every dump path; unidentified assets remain marked
as such. These observations do not identify every NPC or monster.

## Removal and evidence

To test original fallback, close the app, move the named pack's `sheets`
folder outside its `graphics` root, and restart with the same pack selected.
The game uses original art and original lighting. Restore the folder to re-enable the test.
Selecting a previous pack restores that pack's art, which may include the
older house test. No existing configuration file is supplied or overwritten.

Real desktop captures cover both rooms, closed/open curtains, removal, and
an unrelated area. Original and restored bedroom captures match exactly;
the unrelated area also matches exactly with this pack enabled. Across the
repeated curtain interactions, both runs produced the same compressed native
window state sequence, and the revised pack replaced none of the window's
animated tiles in any audited frame. The downloadable clip contains actual
desktop renders; playback is a preview, not a performance measurement.

Parser/PNG tests exercise empty and malformed rules, native-size rejection,
room and layer guards, transparent-tile fallback, live-palette actor texels,
background lighting, optional battle fallbacks, and unchanged original ROM
memory. Existing PNG, sprite, HUD, sampling and controller tests pass. Native
container/runtime/import checks are separate from **the pending PS5 test of
this build, visuals, transitions and sustained performance**.

Startup write checks remain removed; the manual full dumper remains available.
Original PC source/notices are preserved: **Egg of Time v1.3.5 by Power Pudu**.
No ROM, state, extracted art or personal executable is in the source archive.
