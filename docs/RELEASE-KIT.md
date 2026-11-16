# CTPS5 Mods 01.006.018 release kit

This kit collects the current mod host's source, build workflow, offline
mod editor, examples, documentation and license notices. Several PNG packs
can run together. PS5 hardware validation and the reported music breakup
remain outstanding, so label it **release candidate**, not stable.

## Public distribution and personal installation

The public kit includes authored source and tooling. Each player supplies
their own unmodified Nintendo DS USA/YQUE `CT.nds` (SHA-1
`11f9e8204e484eab42181b210c9f214584be75bf`). It does not include a ROM,
extracted artwork, saves or generated game code/executable.

The separate personal-play ZIP contains the locally built native app and
its matching runtime, plus the same guides/editor. Keep that executable
private. Power Pudu's original license says: “Code generated from a ROM,
extracted files, ROMs and saves are not part of this project and must not be
distributed with it.” The native executable links ROM-derived game code;
loading assets from a separate NDS does not make it a public runtime-only
download. See [distribution](DISTRIBUTION.md) and [licenses](../LICENSING.md).

Both forms require the player's supported PS5 homebrew setup. The established
target is firmware 13.20 with kstuff and Shadow Mount Plus 1.7beta11. No console
system software or homebrew launcher is bundled. Compiler/SDK dependencies
are pinned and fetched by the source build helper; they are not preinstalled
inside the ZIP. Internet access and a Linux build machine are needed for a
first source build.

## Build the mod host

Follow [the dependency setup](BUILDING.md), then use:

```sh
bash ps5/build_mod_release.sh /absolute/path/to/CT.nds
```

Output: `build-ps5/package-mods/PPSA90436/`. This builds the game locally,
packages **CTPS5 Mods** with the complete native runtime and uses PPSA90436
for the game app rather than the separate boot-check utility. Its version
comes from `ps5/sce_sys/param.json`. Existing nonempty output folders are
rejected to avoid overwriting a previous build; move your previous generated
folder aside before repeating the packaging step. `EGG_BUILD_DIR` selects a
different build directory.

## Install the app and optional themes

1. Fully close CTPS5. Back up `/data/eggoftime/` and keep the previous complete
   app folder for rollback.
2. Copy the complete `PPSA90436` folder into `/data/homebrew/`. Replace its
   executable, runtime and metadata together. Avoid a nested PPSA90436 folder.
3. Put your own supported dump at `/data/eggoftime/rom/CT.nds`.
4. Refresh Shadow Mount Plus/the homebrew launcher and launch **CTPS5 Mods**.
5. Optionally copy the starter data's `eggoftime` folder into `/data/` on a
   fresh setup. It supplies authored HUD themes only. Existing users should
   copy just wanted new theme folders to preserve their edited themes.

```text
/data/homebrew/PPSA90436/
  eboot.bin
  sce_module/libc.prx
  sce_sys/param.json
  sce_sys/icon0.png
/data/eggoftime/
  rom/CT.nds                         your own input
  mods/packs/<pack-id>/graphics/      optional PNG mods
  mods/battle-hud/themes/<theme-id>/  optional HUD themes
```

Retain any other supplied app files. The kit does not replace `ps5.ini`,
`eggoftime.ini`, controls, `hud.ini`, `pack-states.ini`, saves or ROMs. The
base PPSA90435 can stay installed; it shares `/data/eggoftime/`.

**L3+R3** opens Settings / Mods / Controls. In Mods, enable wanted packs
separately, keep Texture mods On and fully restart. Later pack names win
overlaps. Start with [the modding guide](MODDING-GUIDE.md), which covers PNG
names, dimensions, original fallback, HUD editing, room limits and music.

## Package source for sharing

```sh
python3 tools/package_release_kit.py --output /absolute/path/CTPS5-Mods-018-public-kit.zip
```

Install the Python `Markdown` package before packaging from a Git checkout
(`python3 -m pip install Markdown`). The packager renders and checks offline
HTML documentation, audits authored source and nested archives, includes the original
PC source/notices, marks shell helpers executable and adds a SHA-256 inventory.
It refuses an existing destination. It never includes an executable, ROM or
personal game state and does not publish or upload anything.

For a private local-play bundle only:

```sh
python3 tools/package_release_kit.py \
  --personal-app build-ps5/package-mods/PPSA90436 \
  --output /absolute/path/CTPS5-Mods-018-personal-play.zip
```

This requires the complete expected app and verifies its title/version/SELF
header and pinned runtime hash before packaging. Source and modding tools are
included, but the generated executable remains unsuitable for public sharing.

## Release checks

Host 018 previously passed native build/container/runtime checks, 199 public
SDK import checks and desktop tests of pack state persistence, multiple pack
composition, HUD/character priority, room restrictions and curtain lighting.
Actual port captures exercised three packs together and 4K output. This kit's
editor and packaging changes do not change the executable or claim new console
results. Each kit includes a machine-readable manifest of its contents.

Before declaring a stable PS5 release, verify cold boots, saves/load across
restarts, menus/dialogue, field/room transitions, battle effects including Aura
Whirl, long music sessions, output resolutions, multiple packs and rollback.
Sustained 4K60 and a fix for music stutter have not been established.

The new **Cronos House Downstairs** template is supplied in
`modding/templates/`. It is not the finished artwork mod. Its exact conversion
requires the original supplied room PNG and a checked in-game capture; do not
label a template or concept picture as a completed playable room replacement.
