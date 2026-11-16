# CTPS5 modding guide — host 01.006.018

You can change supported graphics by editing PNGs and copying them into a
folder. You do not need AI, game-source changes or one JSON entry per image.
Use your own USA/YQUE game dump. Existing saves and settings stay in
`/data/eggoftime/` across app updates.

## Install and enable a mod

1. Extract the mod ZIP on your computer. Merge its `eggoftime` folder into
   the PS5's `/data/` folder. Do not create `eggoftime/eggoftime/`.
2. Open **L3+R3 → Mods**. Choose **Characters**, **Backgrounds** or
   **Texture Packs**. Turn **Texture mods On** and press A on each wanted
   pack to turn it **On**.
3. Fully close and restart CTPS5 Mods. Opening and closing the menu alone
   does not reload character or scenery PNGs.

Several packs can be On together. Their choices are shared between category
lists and saved in `mods/pack-states.ini`. You do not need to edit that file.
The Default folder loads first, followed by named folders in alphabetical
order. Later names win overlapping replacements. Different assets combine;
missing, inapplicable or invalid replacements retain a lower enabled pack or
the original game asset. See [priority details](MODS-MULTIPLE-PACKS.md).

## Make your first PNG mod

1. In **L3+R3 → Settings → Dump graphics → Yes**, export the game's graphics.
   Refresh your FTP listing and copy the relevant files from
   `/data/eggoftime/graphics-export/` to your computer. Normal launch does
   not run a dump or write check.
2. Edit only the images you want to replace. Retain their complete canvas,
   transparent padding, tile arrangement, capitalization and filenames.
3. Create a pack with a short folder ID using letters, digits, `_` or `-`,
   such as `my-crono`. Put your edited files at the same relative paths
   beneath `mods/packs/my-crono/graphics/`.
4. Copy that pack into `/data/eggoftime/`, enable it and restart.

For example:

```text
Dump:
/data/eggoftime/graphics-export/frames/Chara/Chara_0000/199.png

Your replacement:
/data/eggoftime/mods/packs/my-crono/graphics/frames/Chara/Chara_0000/199.png
```

`Chara_0000` is Crono. `199.png` is one stored pose; replacing it does not
replace his entire animation set or his portrait. `000.png` means pose 000
inside its character folder, not character 000. Other characters also have
files called `000.png`. Never flatten the dump folders.

With all PNG packs Off, original graphics are used. Empty folders, absent
PNGs and removing a PNG followed by a restart also retain or restore the
original, unless another enabled pack supplies that asset. Separate HUD and
older character enhancement switches retain their own settings.

## What can be changed

| Feature | Available in this PS5 host | How |
| --- | --- | --- |
| Party character poses | Yes, supported layouts | Exact `frames/Chara/…` paths |
| Character and object sheets | Yes | Exact `sheets/Chara/…` paths; shared tiles can affect several poses |
| Room and scenery art | Yes | Dump-layout sheets/screens, with optional location restrictions |
| 1000 AD world-map textures | Yes | `sheets/WorldMap/WorldBg_…` paths and a scoped background policy |
| Portraits and battle icons | Yes | Standard `hud/portraits/` and `hud/icons/` PNGs |
| HUD background, borders and bars | Yes | PNGs and optional theme JSON |
| HUD colours, size, layout, labels | Yes | Offline editor, JSON and live menu size controls |
| HUD font atlas | Yes | Optional ASCII atlas and font settings |
| Native fonts | Limited | Original canvas and dumped width TXT files |
| Room brightness / curtain response | Yes, with authored policy | Optional `backgrounds.json`; a verified native state probe is needed |
| Independent character/scenery packs | Yes | Several pack On/Off switches |
| Gameplay rules, new quests or maps | No PNG interface | Requires source/code and game-data work |
| 3D geometry / HD-2D camera | No | The current renderer remains 2D |
| PS5 music file replacements | Not implemented in this host | See music section below |
| PC shaders, external videos, translation patches, online achievements | Disabled in PS5 core | Desktop integrations do not automatically work on PS5 |

Higher-resolution art does not change collision, event positions, camera
rules, animation timing or world layout. The game remains in control of them.

## PNG sizes and palettes

Keep the original full canvas, or enlarge both dimensions by the same whole
number from **1× to 8×**. Room-scoped sheets use **2× to 8×**. Fonts keep
their native dimensions. A sprite that looks small inside its canvas should
not be cropped: the padding sets its position relative to other frames.

Use **RGBA PNG** for your own painted colours and transparent background.
Indexed character PNGs can preserve native palette changes when their palette
indexes are retained; index zero is transparent. Special indexed blends used
by supplied enhancement packs need their matching policy. Do not convert a
special indexed pack to a different palette and expect the same lighting.

A room illustration is not automatically a valid tile sheet. The original
room may use several layers, repeated/mirrored tiles, foreground stairs,
animated curtains and palette changes. Artwork must be mapped into those
layers. Merely renaming a full room picture to `Bg_0135_ncg.bin.png` will not
convert it. Shared tile art can repeat in multiple places; per-room policies
restrict where it appears but do not create unique furniture geometry.

## Use the offline mod editor

Open **`tools/mod-editor/index.html`** in a browser on your computer. It runs
locally without uploading your PNGs.

For character/scenery graphics, choose the folder containing your edited
`frames/`, `sheets/`, `screens/`, `fonts/` or `hud/` folders. Enter a pack ID
to make a named mod, then download the graphics ZIP. Leave the ID blank to
use the Default folder. The editor preserves PNG bytes, font width TXT files,
and supplied `backgrounds.json` / `hud/theme.json` files. Select the pack's
whole `graphics` folder so its policy is included. The generated inventory
is informational; the game discovers the actual files at startup.

This packages already edited graphics. It does not infer pose directions,
correct sprite proportions or convert a whole room illustration into tiles.

## Change portraits, colours and the HUD

For a simple face swap, place your PNG at:

```text
mods/packs/my-faces/graphics/hud/portraits/crono.png
mods/packs/my-faces/graphics/hud/portraits/marle.png
mods/packs/my-faces/graphics/hud/portraits/lucca.png
```

Enable the pack and the portrait battle HUD, choose **classic** as the HUD
theme, and restart. Other supported names are `robo`, `frog`, `ayla` and
`magus`. Missing faces retain the original portraits. Portraits can use
different dimensions up to 2048×2048; the HUD fits them into its slots.

For a complete theme, use the editor's **Portrait HUD theme** section. Choose
colours, select each PNG's role, drop the images in and download the theme
ZIP. It writes `mods/battle-hud/themes/<theme-id>/`. Choose that theme in
**Mods → Battle HUD**. Press A on the theme setting to reload edits live.
HUD size and command-symbol size retain separate menu controls.

The selected dedicated theme has priority over graphics-pack HUD images;
absent or invalid individual images fall back through enabled packs and then
original game art. The shipped **Timepiece/remaster** theme supplies styling,
not replacement faces: it uses original portraits until you add PNGs.
See the [full theme settings and limits](HUD-THEMES-AND-PNG-MODS.md).

## Optional JSON

Ordinary replacements need no registration JSON. There are two different
optional configuration files:

* `theme.json` changes HUD colours, layout and image roles. The editor
  generates it; `mods/battle-hud/hud-theme.schema.json` documents its fields.
* `backgrounds.json` restricts scenery to verified location IDs and can
  define live-palette brightness, transparent-cell fallback and native
  curtain-state lighting. Keep a supplied room policy with its art. Copying
  its art into an unrelated pack without the policy can change other rooms.

The **Cronos House Downstairs** starter uses folder ID
`cronos-house-downstairs` and restricts backgrounds to location **1**. It
does not change the upstairs bedroom, actors or its curtain lighting. The
starter is configuration only until converted room PNGs are supplied.

## Music

Music replacement is possible as a future PS5 feature, but **host
01.006.018 does not load replacement audio**. `ps5/optional_features.c`
retains native sequence identification and uses original game music.

The desktop project has an FFmpeg-based replacement player. It matches sound
archive symbols such as `BGM_18`, loads files from `mods/music/`, and supports
formats including Ogg, FLAC, MP3, WAV and MSU-style PCM. It can read sample-based
`LOOPSTART`, `LOOPEND` and `LOOPLENGTH` metadata. These are desktop capabilities;
the folder and formats are not promises for a future PS5 implementation.

A PS5 implementation would need a linked native decoder or a streaming PCM
reader, buffered disk I/O, looping, volume/fades, pause and sequence-transition
handling, with original-music fallback and battle/long-session audio tests.
It would then be reasonable to expose named music packs with independent
switches. This release does not implement that work or fix the reported
music/Aura Whirl breakup. Only distribute music you are entitled to share.

## Diagnose and remove a mod

If art is unchanged, check the exact path, leading zeroes, global Texture mods
switch, pack's own switch and full restart. Check for a later enabled pack
replacing the same asset. If using a selected HUD theme, check its priority.
Read `/data/eggoftime/eggoftime.log` for PNG or policy errors.

If art looks distorted, check its complete canvas, scale, tile arrangement,
alpha and shared-tile uses before increasing resolution. Report the host
version, pack IDs, affected room/pose and a screenshot. For audio breakup,
include a fresh log after reproducing it; texture mods Off is a useful
comparison, but the current issue was also reported with them Off.

To remove a pack, turn it Off and restart; its files can then be removed.
Delete only that pack's directory. Keep the NDS, saves and settings. Back up
`/data/eggoftime/` before app updates. The latest build still needs PS5
hardware validation; this is a release candidate, not a verified stable build.
