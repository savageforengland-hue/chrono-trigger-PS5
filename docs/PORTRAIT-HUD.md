# Optional portrait battle HUD — preview 1

This branch extends the user-confirmed CTPS5 0.1.0-rc3 baseline with an optional
portrait status strip inspired by the supplied three-character HUD reference.
Original PC runtime/recompiler: **Egg of Time v1.3.5 by Power Pudu**;
[upstream credits and original source](CREDITS.md).
Chrono Trigger artwork and characters belong to their respective rights holders.

The strip uses the seven original 48×48 **menu portraits**, decoded in memory
from each player's own supported USA/YQUE `CT.nds`. No portraits or ROM are
included. It follows the active party slots, reads current and maximum HP/MP
from their battle copies, and displays the game's own ATB countdown mirror.
ATB replaces the reference game's SP statistic. Empty slots remain empty; a
knocked-out member has a grey portrait and an empty ATB gauge. Names currently
use the canonical English character names, including when a save uses renames.

## Presentation

This first mod is a **docked status strip**, not a full replacement of the game's
command/status panel. The original battle interface remains usable above it.
The battle picture becomes slightly smaller to reserve space; its aspect ratio
is preserved. The strip does not cover actors, dialogue, command menus, item
lists or targeting. Outside supported field battles, the original presentation
is used. The module draws a 480×64 surface with nearest-neighbour scaling.

This is a test candidate. Host snapshot/renderer/layout checks and native
package verification are available; PS5 battle playtesting has not yet been
performed. A rendered preview uses example statistics and is not a PS5 capture.

## Enable or disable

The native executable must contain this runtime extension; copying the INI
alone into an unmodified RC3 installation cannot enable it.

After installing your locally built executable, copy
[`mods/battle-hud/hud.ini`](../mods/battle-hud/hud.ini) to:

```
/data/eggoftime/mods/battle-hud/hud.ini
```

```
enabled=1
```

Set `enabled=0`, or remove that file, and fully close/restart CTPS5 to restore
the original presentation. No main `ps5.ini`, `eggoftime.ini`, ROM or save file
is rewritten. Preserve your current working executable before trying a new
native build. Title ID stays PPSA90435; application version is 01.006.001.
The startup log includes `portrait HUD: enabled` when the switch and faces load.

Missing or malformed config, unsupported ROM, invalid battle data, missing
portraits or texture failures fall back to the original presentation. The
existing RC3 startup logging and DualSense button graphics remain present.

## Build, test and preview

Follow [the native build guide](BUILDING.md) using your own USA ROM.
Run the authored-data host tests:

```sh
python3 ps5/test_battle_hud.py --sdl-source /path/to/SDL3
```

A reproducible preview uses the same face decoder and renderer as the mod:

```sh
cc -std=c11 -I /path/to/SDL3/include -I runtime \
  tools/preview_battle_hud.c runtime/battle_hud_faces.c runtime/gfx_rom.c \
  runtime/battle_hud_draw.c runtime/ui_font.c -o /tmp/ctps5-hud-preview
/tmp/ctps5-hud-preview /path/to/your/CT.nds /tmp/ctps5-hud.ppm
```

The PPM is 480×64. Enlarge with nearest-neighbour scaling for a pixel-art
preview. Keep ROM-derived portraits and preview images out of the public
source repository. The original source archive and notices remain in `upstream/`.
