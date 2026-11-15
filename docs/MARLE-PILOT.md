# Marle detailed redraw — focused test pilot

**Host 01.006.007 correction:** only fear cells 89/90/91 can replace gameplay
art. Cell 211 is comparison-only after a reported waiting/hit mismatch; its
original gameplay art is restored. The crossbow redraw is not pose-approved.
See [HUD/status fixes](HUD-FIXES-007.md). This correction supersedes the original
four-pose gameplay scope described by the first pilot.


This pilot loads **new illustrated artwork** for Marle's stored cells
**89, 90, 91 and 211**. It is intended to test the integration before drawing
the rest of the game. It includes the corrected single curled ponytail on
the side fear view, frightened acting and three source-positioned sweat marks
on each of cells 90/91. The two side views face opposite directions, matching
their respective original cells. Cell 211 supplies a crossbow pose that can
also appear in battles. Stored cell numbers do not establish animation order.

Built on **Egg of Time v1.3.5 by Power Pudu**. See [credits](CREDITS.md).
Each player still needs their own supported **USA/YQUE CT.nds**. The public
source contains the loader/tools and configuration; illustrated PNGs and the
personal native executable are distributed separately from public GitHub.

## Install the focused test

The **01.006.007 mod host** is required. Close CTPS5 Mods, update matching app
files under `/data/homebrew/PPSA90436/`, and refresh ShadowMountPlus. Retain
all your settings, saves and portrait HUD configuration. PPSA90435 is the
separate base app; both apps share `/data/eggoftime/` user data.

Merge the pilot pack's `eggoftime/` contents into `/data/eggoftime/`:

```
/data/eggoftime/mods/marle-pilot/pilot.ini
/data/eggoftime/mods/marle-pilot/mod.json
/data/eggoftime/mods/marle-pilot/marle-089.png
/data/eggoftime/mods/marle-pilot/marle-090.png
/data/eggoftime/mods/marle-pilot/marle-091.png
/data/eggoftime/mods/marle-pilot/marle-211.png
```

Keep **Texture mods On**, then fully restart the host. This module is
independent of the named texture-pack selector. For a clean comparison,
turn the older **HD battle sprites** enhancement Off and restart; the
portrait HUD can stay On. This does not require changing its INI.

Open **L3+R3 → Mods → Detailed pose test**. Confirm opens the page.
Left/Right or L1/R1 selects a stored pose; Back returns to Mods. L3+R3 closes
the host menu. The game stays paused while the page is open, using the
existing pause/clock-rebase behavior. The page renders the original and
registered artwork side by side at their original actor origins. It is a
static comparison of stored poses, not a fabricated animation sequence.

During ordinary play, the pilot replaces the three fear poses when the
incoming original objects match. Cell 211 stays original. You do not need to replay a rare story
reaction just to inspect the fear drawings. Other stored poses retain their
original art (or any independently enabled existing graphics mod).

Use **Mods → Marle redraw** to toggle gameplay art live. Its persisted
`gameplay` switch is independent from the comparison page. Set `enabled=0`
in `pilot.ini` and fully restart to unload this pilot entirely,
or remove its directory and restart. Existing graphics/HUD settings are
separate. The previous 01.006.006 mod host can be retained as a rollback.

## What the renderer matches

The loader decodes original object layouts and patterns from the player's
ROM. It requires the complete set of **nonblank 16×16 original objects**
for a pose, their common actor origin, flips, palette, priority, mode and
coordinate space. Empty padding objects do not identify an actor. Missing,
ambiguous, unrelated or unsupported groups retain the original rendering.
The pilot is bounded to four Marle poses; this is not a general arbitrary
sprite-pack ABI yet.

Each matched group is replaced once at its original position and priority.
Frame-scoped HD tiles are excluded from the global native tile-byte hash,
avoiding changes to unrelated frames that reuse source tiles. The new art
can occupy transparent pixels within its original full cell canvas; it is
not clipped to the old individual opaque pixels. The game still selects
and times the poses. No animation tables, durations, frame order, ROM bytes,
saves or game logic are rewritten.

Illustrated RGB colours remain intact. The game controls some outline and
weapon palette entries independently from the body. Matching uses stable
body entries 2–11; changed native outline colours transfer to tagged dark
art edges. Flat palette flashes/fades and the existing brightness/blending
compositor remain supported. Unrecognised nonuniform body-palette variants
fall back to original art. The pilot's crossbow illustration is one weapon
design; it does not cover every equipment variant. Subpixel sprite overlap,
camera/zoom edges and uncommon palette effects still need broader testing.

## Checks and current limits

Authored tests cover exact group matching, coordinate wraps, flips,
palette/priority/space differences, incomplete and ambiguous groups,
immutable inputs and unrelated-pose fallback. The private own-ROM/art test
loads all four registered PNGs, checks original actor origins and canvases,
direct-tile isolation, missing-art fallback, live outline/weapon palette
changes, white/black effects, module disabling and unchanged ROM memory.
Existing HUD and host-menu tests cover the combined eight-row menu,
comparison-page navigation, pause state and input release guards.

The initial 01.006.006 desktop run loaded all four authored poses and matched cell 211
in ordinary battle rendering. The 01.006.007 host quarantines that reported
pose mismatch, retaining the original in gameplay. It displays the comparison page through its
normal menu/input path. Its Linux adapter maps filesystem/platform services,
supplies controller-like keyboard input and captures SDL output; it does
not inject pose-match observations or substitute a preview for gameplay.
The private battle fixture is not distributed. **PS5 hardware boot/visual
testing of this new host remains pending.** Native checks do not establish
full animation correctness or sustained 4K60. This is the focused test
requested before extending the detailed redraw to more poses.

```sh
python3 ps5/test_redraw.py --sdl-source /path/to/SDL3
python3 ps5/test_redraw.py --sdl-source /path/to/SDL3 \
  --sdl-library /path/to/Linux/libSDL3.a --rom /path/to/your/CT.nds \
  --data-root /path/to/private/eggoftime
```

`tools/build_marle_pilot.c` is the registration compiler for this artwork
revision. It uses the original cell canvases, source detached components and
manually reviewed eye/foot landmarks for these particular inputs. It is not
a generic automatic pose-correction tool. Given the external corrected
three-view PNG and crossbow PNG, build it against SDL3 and run:

```sh
cc -std=c11 -O2 -I /path/to/SDL3/include -I runtime \
  tools/build_marle_pilot.c runtime/gfx_rom.c /path/to/Linux/libSDL3.a \
  -lm -ldl -pthread -o /tmp/register-marle
/tmp/register-marle /path/to/your/CT.nds /path/to/corrected-fear-sheet.png \
  /path/to/crossbow211.png /path/to/output/marle-pilot
```

Keep generated artwork/captures, ROM, game-derived test states, generated
game C and the personal native executable out of the public repository.
Startup identifies **CTPS5 HUD and sprite fixes 7 (01.006.007)**. Usual log:
`/data/eggoftime/eggoftime.log`. It lists loaded cells, errors with the image
path/SDL reason, and the first successful renderer match of each pose.
