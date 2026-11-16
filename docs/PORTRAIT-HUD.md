# Centred portrait HUD mod — v0.3.1 visual update

Optional battle presentation for CTPS5, built on **Egg of Time v1.3.5 by
Power Pudu**. [Original PC source and credits](CREDITS.md) are preserved.
Each player supplies their own USA/YQUE `CT.nds`; no ROM, portraits or game
symbols are supplied in the mod pack.

## Separate mod and host

The **mod-only ZIP** contains `mods/battle-hud/hud.ini`, `mod.json` and
instructions. It contains no `eboot.bin`. Its settings and version metadata
are separate from the game. The descriptor identifies this supported module;
the compatible host implements its renderer and hooks.

The current styling needs **mod host 01.006.004**. Version 01.006.003 supplied
the layout, action controls and L3+R3 menu; its HUD now has user-confirmed PS5
operation. This update changes presentation, preserving those working hooks.
An older RC3 or portrait-preview executable cannot acquire those hooks merely
by copying configuration files. A locally built compatible host can be staged
as a separate **CTPS5 Mods** app, **PPSA90436**, alongside the working CTPS5 app
**PPSA90435**. The base app's installed files are left intact.

Both apps use `/data/eggoftime/`, including your own ROM, saves, controls,
settings and installed texture packs. Progress and settings are shared; this
is not a separate save profile. Install one app folder per title ID, with
`eboot.bin`, `sce_module/` and `sce_sys/` immediately inside it.

To update an existing working mod host, close the app and replace its
`PPSA90436` app files with the matching new host files, then refresh
ShadowMountPlus. Preserve `/data/eggoftime/mods/battle-hud/hud.ini` and all
other user configuration. Updating the descriptor alone cannot change the
renderer. The separate mod ZIP supplies defaults for a new installation.

## Presentation and controls

The normal old command/status band is covered. Three horizontal portrait
cards are **centred in that band** (at the bottom by default), with transparent
**Attack, Tech and Item** symbols centred just above. When the game places
its interface at the top, symbols move below it to stay on screen.
The gameplay rectangle is not resized. The selected command has a gold
underline; the selected party member has a small white pointer and a subtly
highlighted portrait bevel.

Portraits come from the original 48×48 menu faces. Each character uses its
**own 16-colour palette bank**, correcting the previous Marle/Lucca colour
error and the same error for other party members. Action symbols come from
the game's weapon sword, Tech lightning and inventory bag. The original
inventory button background/bevel is omitted. A bold black outer outline and thin light inner edge keep symbols visible
on light and dark battlefields. Labels also have black outlines. There are no
blue action-button boxes.

The panel uses the original battle-window tiles and default `win_1` palette,
decoded from the player's ROM. Its bevel and corners remain intact when
resized; the middle repeats the original texture. Soft inset dividers replace
the hard card outlines, with silver-white text and muted recessed gauges.
This uses the default window theme rather than automatically following a
custom in-game theme. If that window art cannot be decoded, a subdued
procedural background keeps the otherwise available HUD usable.

Cards follow party order and show current/maximum HP/MP and the game's own
ATB countdown. KO portraits are grey; unused slots are empty. Names currently
use canonical English names, including when saves rename characters.

While the normal icon row is active:

| Input | Action |
| --- | --- |
| Left / Right | Select Attack, Tech or Item |
| Up / Down | Also select the command, as before |
| L1 / R1 | Select another party command column |
| Mapped Confirm / Back | Confirm or cancel; defaults Circle / Cross |
| L3 + R3 together | Open or close Settings and Mods |

Tech/Combo lists retain their MP costs; Item lists retain quantities. These
selection screens temporarily use the original panels and controls. Target
selection and battle messages keep the original behavior. Target hands are
excluded from icon-row input remapping, even when they cross the HUD band.

## Settings and mods menu

Press **L3 and R3 together**, then release both. The menu has **Settings**,
**Mods**, **Controls** and **Resume game**. Gameplay parks at a runtime poll
while this menu is open; its clock is rebased on resume so battle/ATB time
does not catch up. The chord also closes a submenu. Cross returns one page;
from the hub it resumes play. Confirm/Back labels follow saved bindings.
Stick clicks are reserved for this shortcut in the mod host.

The Mods page contains:

| Setting | Behavior |
| --- | --- |
| Portrait battle HUD | On/off, applied and saved immediately |
| HUD size | 75–125%, applied and saved immediately |
| Symbol size | 50–100% of the HUD's pixel scale, applied/saved immediately |
| Texture mods | Saved; restart CTPS5 Mods to fully apply changes |
| Texture pack | Choose a named installed pack; restart to load it |

Installed packs are read from `mods/packs/<id>/graphics/`, with `default`
using `mods/graphics/`. The selector supports up to 63 named packs plus the
default. Unrelated PS5 settings and comments are preserved when saving a pack.

## Module configuration

Install the contents of the mod ZIP's `eggoftime/` into `/data/eggoftime/`:

```
/data/eggoftime/mods/battle-hud/hud.ini
/data/eggoftime/mods/battle-hud/mod.json
```

```ini
enabled=1
scale_percent=100
command_scale_percent=70
```

Manual INI changes are read on the next launch; menu changes apply live.
Set `enabled=0`, or disable it under Mods, for the original battle interface.
Missing/malformed configuration, unsupported ROM, invalid battle state,
unavailable artwork or texture-upload failures retain the original interface.
No game assets or saves are rewritten by the HUD module.

The implementation is split into a read-only battle snapshot, pure pixel
renderer, ROM-art decoder, platform bridge and independent settings module.
Layout uses the viewport and bounded scale settings, preserving proportions
at different output resolutions and both zoom/UI-size choices.

## Build and validation

Follow [BUILDING](BUILDING.md) to build from your own ROM, then stage a
side-by-side app without changing the built base folder:

```sh
python3 ps5/package_mod_host.py /path/to/built/PPSA90435 /path/to/output/PPSA90436
python3 ps5/test_battle_hud.py --sdl-source /path/to/SDL3
python3 ps5/test_system_menu.py --sdl-source /path/to/SDL3
```

Tests exercise per-character palette banks, transparent action artwork,
window texture/palette decoding, invalid skin fallback, resized corner
preservation, renderer bounds,
immutable RAM, HP/MP/ATB/KO, selected actor/command, invalid state, fallback,
input routing, full old-panel cover, centred geometry at 4K, top/bottom/zoom,
size limits, menu navigation, chord/release guards, paused clock rebasing,
pack selection and atomic configuration preservation.

A standalone preview uses the same decoder and renderer with sample stats:

```sh
cc -std=c11 -I /path/to/SDL3/include -I runtime \
  tools/preview_battle_hud.c runtime/battle_hud_faces.c runtime/gfx_rom.c \
  runtime/battle_hud_draw.c runtime/ui_font.c -o /tmp/ctps5-hud-preview
/tmp/ctps5-hud-preview /path/to/your/CT.nds /tmp/hud.ppm /tmp/actions.ppm
```

A private desktop battle exercises the full host initialization and normal
game/render path, including the decoder, snapshot, renderer and bridge. Its
adapter supplies Linux filesystem/platform services and captures SDL output;
it does not inject HUD visibility or compositing. It uses a private high-HP
battle fixture.
Menu previews exercise the actual renderer with authored host dependencies.
These previews are **not PS5 captures**. Keep game-derived captures, test
fixtures, generated game C and game executables out of the public repository.
The user confirmed that the previous host displays this HUD on PS5.
**The v0.3.1 restyle still needs a PS5 visual check.**
Native package checks do not establish sustained 4K60 performance.

## Themes and drag-and-drop graphics (01.006.012)

The existing HUD now supports optional PNG portraits/icons/backgrounds and JSON colours/layout. Its current enable/size settings are preserved. See [HUD themes and PNG replacements](HUD-THEMES-AND-PNG-MODS.md) for the offline editor, full paths, fallback rules and examples.
