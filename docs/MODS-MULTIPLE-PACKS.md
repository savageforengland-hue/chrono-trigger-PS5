# Multiple enabled PNG packs (host 01.006.018)

Each installed graphics pack now has its own **On/Off** switch. Open
**L3+R3 → Mods → Characters → Character packs**, **Backgrounds**, or
**Texture Packs**, select a pack and press A. The same pack has the same state
in every category. Enable the house, 1000 AD world map and character packs
together, then fully close and restart CTPS5 to apply the changes.

The global **Texture mods** switch must also be On. HUD themes and the older
Crono/Marle/sprite modules retain their independent controls. Switching one
named PNG pack Off does not change another pack's saved state.

## Existing settings and original fallback

The previous `active_pack` selection is retained on the first launch. The first
new toggle writes `/data/eggoftime/mods/pack-states.ini`, preserving the former
enabled selection when a second pack is enabled. No migration changes ROM,
saves, controls, HUD INIs or `ps5.ini`. Example state file:

```ini
# Each key is the exact pack folder name. Restart after editing.
default=0
1000ad-remaster=1
crono-house-hd2d=1
crono-png-test=1
```

`default` controls `mods/graphics/`. Other IDs control
`mods/packs/<id>/graphics/`. Only installed/enabled folders are loaded. All
PNG pack switches Off loads zero graphics roots; the original game assets
remain. Missing PNGs retain an applicable lower enabled replacement or the
original asset. Independently enabled legacy modules and HUD keep their own
switches. Empty scoped tiles and invalid higher PNGs retain lower/original art.

State changes are atomic and preserve comments and uninstalled-pack entries.
A full restart is required; the running scene keeps its previously loaded art
until then. Removing `pack-states.ini` returns to the saved legacy `active_pack`
selection. Missing legacy packs retain the previous default-folder fallback
only before explicit states exist.

## Priority when assets overlap

The Default folder loads first. Named packs load in the same alphabetical
folder-ID order as the menu. **Later names in the list win overlapping assets.**
Packs replacing different assets combine. Where two packs replace the same
native tile, character pose or HUD image, the later applicable valid art wins.
Location restrictions remain part of applicability: a house tile cannot override
an overworld tile while outside the house just because its name sorts later.
Disabling the higher pack restores the lower replacement after restarting.

Each pack keeps its own `backgrounds.json`, room restrictions, palette-relative
lighting and indexed blend settings. House curtain lighting remains active
when a separate world-map or character pack loads afterwards. Higher invalid
background rules skip that pack's scoped art and leave earlier valid packs.
PNG character frames use the last valid replacement for each exact pose;
invalid later frames keep the earlier art. Frame identity and game animation
selection remain unchanged.

HUD PNG assets also combine across enabled roots. The selected dedicated HUD
theme retains priority; missing images can come from any enabled pack, using
the same list order. A bad or missing higher PNG can fall back to a lower one.

## Validation

Tests cover actual state-file migration/reload, simultaneous enables,
disabling one while retaining another, zero enabled roots, failed writes,
path validation, malformed/oversized state lines, deterministic root order,
per-pack location/window/light rules, shared tile conflicts, empty/invalid
higher layers, character-pose priority and HUD image layering. Actual desktop
captures exercise three packs together, the 1000 AD world at 3840×2160,
combined house curtain open/close/reopen, and the real menu with several On
switches. The private delivery report records the final native package checks.
PS5 hardware and sustained performance tests remain pending.
