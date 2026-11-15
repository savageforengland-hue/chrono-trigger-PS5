# Crono HD-2D pose test — host 01.006.011

This is a separate **eight-pose test mod**, not the complete 248-cell redraw.
The PNGs use the original dump filenames: **000, 001, 002, 003, 005, 006,
011 and 056**. The other 240 cells keep their original rendering in the
own-ROM fixture audit. The full redraw remains unfinished. Standing views,
two front-facing steps, a left-profile runner and a rear ready stance are
included so placement, proportions, facing and renderer behavior can be
tested before more drawings are registered.

The earlier full-sheet drafts had pose and direction mistakes. They are
excluded from this test pack. The supplied test drawings still need visual
review of gait and anatomy. No claim of complete animation fidelity is made.
The game selects and times its original poses; mixed original/redrawn art
is expected while this small pack is installed.

## Two separate downloads

The **art mod ZIP** contains its own JSON, INI and eight PNGs, independently
removable from the game. The **01.006.011 host ZIP** adds the JSON reader,
Crono pose registration, live switch and comparison-page support. Older
hosts only hardcode the Marle pilot; placing JSON beside PNGs cannot add
Crono support to those executables. Keep each executable and its matching
runtime together. The separate base app PPSA90435 is unchanged.

Close CTPS5 Mods, retain the previous PPSA90436 folder as rollback, merge
the new `PPSA90436` folder into `/data/homebrew/`, and refresh ShadowMountPlus.
Then merge the mod ZIP's `eggoftime` folder into `/data/`:

```
/data/homebrew/PPSA90436/eboot.bin
/data/homebrew/PPSA90436/sce_module/libc.prx
/data/homebrew/PPSA90436/sce_sys/param.json
/data/eggoftime/mods/crono-hd/mod.json
/data/eggoftime/mods/crono-hd/pack.ini
/data/eggoftime/mods/crono-hd/frames/Chara/Chara_0000/000.png
/data/eggoftime/mods/crono-hd/frames/Chara/Chara_0000/001.png
/data/eggoftime/mods/crono-hd/frames/Chara/Chara_0000/002.png
/data/eggoftime/mods/crono-hd/frames/Chara/Chara_0000/003.png
/data/eggoftime/mods/crono-hd/frames/Chara/Chara_0000/005.png
/data/eggoftime/mods/crono-hd/frames/Chara/Chara_0000/006.png
/data/eggoftime/mods/crono-hd/frames/Chara/Chara_0000/011.png
/data/eggoftime/mods/crono-hd/frames/Chara/Chara_0000/056.png
```

Do not nest PPSA90436 or eggoftime twice. Keep the NDS, saves, settings,
existing HUD and Marle folders. Keep **Texture mods On** and fully restart.
**L3+R3 → Mods → Crono redraw** should show **On (8 poses)** and switches
live, separately from Marle. For a clean comparison, turn Sprite smoothing
Off and restart; the portrait HUD can remain On.

**Mods → Detailed pose test** opens the existing comparison page, now with
both modules' loaded poses. Left/Right or L1/R1 changes the stored pose;
the title identifies the character/cell. The game stays paused while this
page is open. Stored cell order is not animation order. Compare facing,
boots, size and position against the original shown alongside it. Resume
ordinary play to inspect the poses when the game selects them.

Startup identifies **CTPS5 Crono JSON redraw test (01.006.011)**. The usual
`/data/eggoftime/eggoftime.log` records loaded PNGs/cells, original geometry,
JSON errors and the first matched Crono cell during rendering. Missing,
wrong-sized, empty, disabled or unmatched art keeps the original rendering.
If the menu says Not installed, send that log. Refresh FTP before reading
new files. The working graphics dumper changes are retained; it still uses
`/data/eggoftime/graphics-export/`.

To remove the art test, close the app, remove only `mods/crono-hd/`, and
restart. The host runs without it. A live toggle changes `gameplay` in the
pack's own INI; `enabled: false` in JSON unloads the pack after a restart.
Neither method edits ROM bytes or saves.

## JSON actively selects files

Unlike the older Marle metadata JSON, this schema is consumed by the host:

```json
{
  "schema": 2,
  "character": 0,
  "scale": 4,
  "enabled": true,
  "frames": [
    {"cell": 0, "file": "frames/Chara/Chara_0000/000.png", "enabled": true}
  ]
}
```

Paths are relative to `mods/crono-hd/`. Each listed PNG must be RGBA on the
original full cell canvas at exactly 4× width and height, with transparent
padding and the original actor origin retained. Artwork is uniformly scaled
and positioned within that canvas by the registration tool. File dimensions
can differ between poses because original padding differs. Do not resize
every PNG to one common rectangle: that changes the frame geometry.

Add a new registered PNG and a JSON entry, then fully restart. The reader
supports up to 512 entries for Crono, validates complete JSON before loading,
and rejects duplicate cell/file mappings, absolute/traversal paths, invalid
schema/scale and oversized input. An entry with `enabled: false` is skipped.
Changing only the descriptive `art_files` list from the old Marle schema
has no effect on this reader. The original Marle INI/loader remains supported.

## Validation and limits

The own-ROM integration test loaded all eight JSON-selected images, matched
each complete original object group, verified actor origins/canvases, checked
direct HD tiles and kept ROM memory unchanged. Auditing all **248 stored
Crono cells** found exactly the eight expected replacements, with no matches
to different source poses. Missing PNG, malformed JSON, disabled entry/pack,
unlisted poses, foreign palettes and live switches retained their expected
fallback behavior. Marle and Crono controls, body/outline palette effects,
white flashes, menu navigation and the nine-row layout were checked.

Native container/runtime/import checks are separate from console testing.
**This new host and art pack still need PS5 boot and visual testing.** The
static pose fixtures do not prove smooth gait, every live palette/equipment
variant, unusual overlaps, or sustained 4K60.

The original PC project is **Egg of Time v1.3.5 by Power Pudu**; pristine
source and notices remain in `upstream/`. See [credits](CREDITS.md). Each
player supplies their own supported USA/YQUE NDS. Generated game C, ROMs,
private test fixtures, personal executable and external illustrated PNGs
are excluded from the public repository.

```sh
python3 ps5/test_redraw.py --sdl-source /path/to/SDL3
python3 ps5/test_crono_redraw.py --sdl-source /path/to/SDL3 \
  --sdl-library /path/to/Linux/libSDL3.a --rom /path/to/your/CT.nds \
  --data-root /path/to/private/test-data-with-Marle-and-Crono
```

`tools/register_crono_test.c` performs external artwork slicing/registration
against original ROM geometry; it does not generate drawings or invent an
animation sequence. Its private input job list names authored atlases and
their row/column slots. Source and output artwork remain external.
