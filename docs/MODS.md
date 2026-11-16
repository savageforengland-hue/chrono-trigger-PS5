# Configuration and texture packs

Persistent data lives in `/data/eggoftime/`. Keep this directory when updating the app.

| File / directory | Purpose |
| --- | --- |
| `ps5.ini` | PS5 display and platform configuration |
| `eggoftime.ini` | Game presentation and settings |
| `controls.ini` | Input mappings |
| `mods/graphics/` | Flat graphics replacement pack |
| `mods/packs/<pack-id>/graphics/` | Named modular graphics pack |

See the annotated templates in `ps5/ps5.ini` and the texture-pack template shipped with the source. The included installer validates the pack manifest and paths:

```sh
python3 ps5/install_texture_pack.py --help
```

Texture pack format v1 targets USA YQUE, with 2x–8x replacement scales. Keep packs independently named so they can be updated or removed without modifying game code. Supply your own replacement art; original extracted assets are not included in public downloads.

This PS5 core profile disables the PC shader, FFmpeg media replacement, online RetroAchievements and translation patch integrations. Do not assume a desktop mod depending on those modules works on PS5. Test a pack with the unmodified game first, and include its ID and version when reporting a problem.

## Categorized Mods menu (01.006.016)

Open **L3+R3 → Mods**:

| Category | Options |
| --- | --- |
| Characters | Character packs, plus installed sprite smoothing, Crono/Marle redraws and the pose viewer |
| Backgrounds | Packs containing room or scenery PNGs |
| Battle HUD | Portrait HUD, HUD size, symbol size and theme |
| Texture Packs | All installed named graphics packs, including unclassified packs |

Choose a category with A. Up/Down moves through a list; A selects a pack.
Left/Right changes a setting, but never enters a category or selects a pack.
Back returns one level. L3+R3 closes from any level. Long lists scroll with
arrows and retain readable row sizes. Absent optional character modules are hidden.

Character/background lists are discovered from actual PNGs under the dumped
`sheets/Chara`, `frames/Chara`, `sheets/Map` and `screens/Map` paths when opening
Mods. Empty folders do not count. Mixed packs appear in both lists. There is
**one shared active graphics pack**: selecting a mixed house pack enables all
its character and scenery replacements together. Categories do not introduce
independent pack stacking. **Default folder** means `mods/graphics/`; it may
contain earlier PNGs and does not necessarily mean unmodified artwork.

Keep the same folder structure and PNG filenames. Selecting a pack or changing
Texture mods requires a full app restart; HUD/redraw settings retain their
existing live behavior. No configuration migration or pack reorganization is
required. Existing active pack, HUD theme, controls, ROM and saves are preserved.

The same host skips room-only HD composition outside permitted locations,
while retaining unrestricted PNGs, battle sprite modules and active redraws.
This avoids unnecessary work; it is **not a verified music-stutter fix**.
For the reported music breakup, including Marle's Aura Whirl, host 016 logs
source audio queue observations at most once per five seconds. `empty` counts
empty source-queue observations after startup, `dropped` counts queue-cap skips,
`errors` counts queue/put failures, and `max_producer_gap_ms` includes menu pauses
and loading. An empty SDL source queue alone does not prove a device underrun.
Music mixing, pitch, latency targets and gameplay timing are unchanged.
Reproduce, then retrieve a new `/data/eggoftime/eggoftime.log`. Console audio
verification remains pending; host/offline tests cannot establish the cause.
The automatic startup write check stays removed; full graphics dumping remains manual.

## Illustrated Crono test

The separate [eight-pose Crono HD-2D test](CRONO-POSE-TEST.md) uses mod host
01.006.011 and an operative JSON manifest under `mods/crono-hd/`. Its PNGs
retain the dump filenames. The full 248-cell redraw is unfinished; this
small pack tests registration and rendering first. Crono and Marle have
independent live controls.

## Battle sprite enhancement prototype

The separate [Marle detailed-redraw pilot](MARLE-PILOT.md) tests four
illustrated replacements with mod host 01.006.007. Its files live under
`mods/marle-pilot/`. Open **L3+R3 → Mods → Characters → Detailed pose test** to inspect
them before expanding the artwork. This is separate from the cleanup filter.

[Faithful HD battle sprites](BATTLE-SPRITES.md) has a separate configuration
under `mods/battle-sprites/`, supported by host 01.006.005. It refines original
sprites at 2×/4× while retaining live palettes and animation data. This is an
enhancement prototype rather than the illustrated redraw from the concept.
Use the additional **HD battle sprites** switch and restart. No art is shipped.

## Portrait battle HUD module

The [centred portrait HUD](PORTRAIT-HUD.md) is a separate configuration pack
for mod host 01.006.003. The host can be installed as CTPS5 Mods (PPSA90436)
alongside the base app (PPSA90435). Press L3+R3 for Settings, Mods and Controls.
The Mods page changes portrait HUD visibility/size, transparent symbol size,
texture visibility and the installed texture pack. HUD changes apply live;
restart after texture or pack changes. Both apps share `/data/eggoftime/`.

The mod-only ZIP contains no executable, ROM or extracted artwork. Its
`mods/battle-hud/` folder has its own version descriptor and INI settings.
Artwork is decoded from the player's own USA NDS. The compatible host supplies
the renderer, observation and input hooks; the pack is not a binary plugin for
older builds.

Host 01.006.007 lists **Sprite smoothing** and **Marle redraw** separately. The latter toggles the illustrated fear poses live; the reported crossbow pose is comparison-only. See [HUD fixes](HUD-FIXES-007.md).

For extracted PNGs and FTP access, see the [graphics dumper guide](GRAPHICS-DUMP.md). Host 01.006.014 retains the manual kernel-path dumper and removes the earlier automatic startup write check. Full exports use `graphics-export/`.

## PNG-only graphics and HUD themes (01.006.012)

Use the same dumped relative PNG paths under `mods/graphics/`. No hand-written manifest is required. An empty folder retains original assets; individual missing/invalid replacements fall back. HUD themes extend the existing `mods/battle-hud/` and preserve its settings. See [the paths, local editor and theme reference](HUD-THEMES-AND-PNG-MODS.md).

## House interior test (01.006.014)

The private [Crono bedroom test](HOUSE-INTERIOR-TEST.md) uses six exact dumped
PNG sheet paths under `mods/graphics/sheets/Map/Bg/`. It can be merged into
`/data/` without replacing settings or the HUD. The companion host stops
automatic startup write checks; the working PNG loader is retained.

## Full house HD-2D style test (01.006.015)

The private [two-room house pack](HOUSE-HD2D.md) adds painted room textures
and faithful Crono/mum/cat sprite refinement under the independent named pack
`crono-house-hd2d`. Exact dump names are retained. An optional room policy
prevents shared sheets changing other locations, preserves native curtain
animation, and keeps fully transparent sheet cells original.
