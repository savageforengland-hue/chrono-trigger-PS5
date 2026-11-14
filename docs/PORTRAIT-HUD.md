# Optional portrait battle HUD — replacement preview 2

This branch extends the user-confirmed CTPS5 0.1.0-rc3 baseline with an optional
**replacement for the normal party-status panel**, inspired by the supplied
three-character HUD reference. Original PC runtime/recompiler:
**Egg of Time v1.3.5 by Power Pudu**;
[original source and credits](CREDITS.md).
Chrono Trigger artwork and characters belong to their respective rights holders.

The three cards use the original 48×48 **menu portraits**, decoded in memory
from each player's own supported USA/YQUE `CT.nds`. No portraits or ROM are
included. Cards follow active party order and show current/maximum HP and MP,
the game's own displayed ATB countdown, and a gold outline for the selected
member. ATB replaces the reference game's SP statistic. Empty slots remain
empty; a knocked-out member has a grey portrait and an empty ATB gauge.
Names currently use canonical English names, including when a save uses renames.

## Presentation

Preview 2 supersedes the earlier additive/docked experiment. It covers the
**complete old normal HP/MP/ATB status box** with an opaque portrait panel.
The original commands remain beside it. The replacement also uses the right
widescreen margin in the same band, rather than adding a strip below the game.
**The gameplay rectangle and scale are unchanged.** Portrait proportions are
preserved; cards are smaller at narrower aspect ratios.

The module observes the original normal HP header and first HP digit being
drawn, in addition to validating the battle state. It yields to the original
Tech/Combo and Item selection screens, including MP costs and item quantities.
Those are selection interfaces, not a duplicate normal status HUD. Target
cursors, actors and battle messages retain the original game logic and graphics.
When the status panel hides, observations expire within two game frames.
The layout supports the normal panel's top/bottom positions and the existing
zoom/UI-size mappings. The portrait surface is 480×128 with nearest scaling.
PS5's existing unsupported/off shader path retains the original presentation.

## Enable or disable

The native executable must contain this extension; copying the INI alone into
an unmodified RC3 installation cannot enable it. After installing your locally
built executable, copy [`mods/battle-hud/hud.ini`](../mods/battle-hud/hud.ini) to:

```
/data/eggoftime/mods/battle-hud/hud.ini
```

```
enabled=1
```

Set `enabled=0`, or remove that file, and fully close/restart CTPS5 to restore
the original presentation. No main INI, ROM or save is rewritten by the mod.
Keep your working executable before testing a candidate. Title ID remains
PPSA90435; this candidate's application version is **01.006.002**.
The startup log includes `portrait HUD: enabled` and `replacement panel`.
Missing/malformed config, unsupported ROM, invalid battle state, unavailable
portraits or texture failures retain the original interface. The RC3 startup
logging, requested icon and DualSense button graphics remain present.

## Validation and limits

The same decoder, read-only snapshot, renderer and host bridge were exercised
in a private desktop battle with Crono, Marle and Lucca. The normal old status
panel was visibly replaced; commands, Tech/Combo MP costs, Item lists, targeting
and battle messages were inspected. Private input/capture/observation hooks
and a high-HP test fixture were used; those are not part of the PS5 build.
The provided battle preview is a **desktop integration capture, not a PS5
screenshot**. No test ROM, save, state, portraits or preview capture belongs in
the public source repository.

Authored-data tests cover immutable RAM, all seven identities, invalid pointers,
KO and ATB values, selected-member state, renderer boundaries, switch/failure
fallbacks, original-header visibility, Tech/Item yield, stale observations,
unchanged gameplay geometry, full status-box coverage, command/message
separation, portrait proportions, top/bottom placement and zoom at 4K.
The native package is checked for section mapping, signed containers and all
186 imports. **PS5 battle validation remains pending; sustained 4K60 is not a
measured claim.** RC3 remains the known working baseline.

## Build, test and preview

Follow [the native build guide](BUILDING.md) using your own USA ROM.

```sh
python3 ps5/test_battle_hud.py --sdl-source /path/to/SDL3
```

A standalone layout preview uses the same decoder and renderer with example
statistics (it is not a gameplay capture):

```sh
cc -std=c11 -I /path/to/SDL3/include -I runtime \
  tools/preview_battle_hud.c runtime/battle_hud_faces.c runtime/gfx_rom.c \
  runtime/battle_hud_draw.c runtime/ui_font.c -o /tmp/ctps5-hud-preview
/tmp/ctps5-hud-preview /path/to/your/CT.nds /tmp/ctps5-hud.ppm
```

The PPM is 480×128. Enlarge with nearest-neighbour scaling for pixel art.
The original source archive and notices remain in `upstream/`.
