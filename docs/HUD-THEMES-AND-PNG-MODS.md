# HUD themes and PNG replacements

Host **01.006.012** adds themes to the **existing portrait battle HUD** and automatic graphics discovery. The base PPSA90435 app stays separate from CTPS5 Mods (PPSA90436). Keep your own supported USA/YQUE `CT.nds` and existing saves/settings in `/data/eggoftime/`.

## Replace a graphic

Copy your edited PNG to the **same relative path** beneath `/data/eggoftime/mods/graphics/` as in `graphics-export/`. Restart CTPS5 with **Texture mods On**.

| Dumped file | Replacement file |
|---|---|
| `graphics-export/frames/Chara/Chara_0000/000.png` | `mods/graphics/frames/Chara/Chara_0000/000.png` |
| `graphics-export/hud/portraits/crono.png` | `mods/graphics/hud/portraits/crono.png` |
| `graphics-export/hud/icons/attack.png` | `mods/graphics/hud/icons/attack.png` |
| `graphics-export/hud/background.png` | `mods/graphics/hud/background.png` |

`Chara_0000` identifies Crono; `000.png` identifies **one stored pose**, not his entire animation set or his HUD portrait. Other characters also have a `000.png`. Keep the folders and the full filename, including leading zeroes and `.png`.

- **Empty mod folder:** original game assets.
- **Missing replacement:** original asset for that graphic; other replacements still work.
- **Damaged, wrongly sized or unsupported PNG:** log the failure and retain its original asset.
- **Remove a replacement and restart:** the original asset returns. No source asset on disk is overwritten.
- **No hand-written JSON is required** for these PNG replacements. The host scans filenames. `manifest.generated.json` is an informational inventory, regenerated at startup; editing it does not enable/disable PNGs.

An active named texture pack uses `mods/packs/<pack>/graphics/` instead of the default `mods/graphics/` folder. It has the same hierarchy. Missing files use original assets, rather than silently borrowing a different texture pack. HUD themes may additionally inherit the active pack's `hud/` assets as described below.

Only copy graphics you want to override. Copying the entire dump overrides everything it contains, including its original/grey placeholder palettes. This workflow covers **supported graphics**, not code/gameplay/music/save modifications.

## Sprite and background dimensions

Keep the full dumped canvas, including transparent padding and its original actor origin. Sprite/sheet/background PNGs accept their original dimensions or an equal whole-number scale in both directions, **1× through 8×**. Do not trim each pose to its visible artwork or force differently sized canvases to one size. Fonts retain the original size and can also use their dumped `.txt` widths.

Party frames for `Chara_0000` through `Chara_0006` take an exact whole-pose path, before shared-tile graphics edits. This retains original timing/origins and prevents a frame replacement from changing another pose merely because it shares a tile. It supports up to eight original 16×16 objects and a 96×96 source canvas; unsupported object layouts retain original rendering and are logged. Exact reused poses with identical source objects cannot be distinguished. Other sheets/screens/frames retain the existing tile-based mod behavior, where shared source tiles can affect multiple uses.

Indexed party PNGs preserve their 4-bit palette indexes and use the game's live palette; index 0 is transparent. Use **RGBA PNG** for custom fixed colours. Native-size party PNGs are internally doubled without changing their displayed size. Drop-in party frames take precedence over the earlier Marle/Crono JSON art packs for the same character/cell; the Texture mods switch controls them. They are not controlled by the separate Crono/Marle art-pack toggles.

The dumper exports supported backgrounds under `screens/` and their tiles under `sheets/`. A scene may comprise several layers; a complete screenshot is not necessarily one replaceable PNG.

## Customize the existing HUD

This is an extension of `mods/battle-hud/`, not a second HUD. Its existing `hud.ini` keeps `enabled`, `scale_percent` and `command_scale_percent`. The theme update ZIP does **not** overwrite that file.

For a simple portrait swap, put a PNG in `mods/graphics/hud/portraits/crono.png`, leave **HUD theme = classic**, and use the already enabled portrait HUD. You can use arbitrary portrait dimensions up to 2048×2048; the HUD fits them into its portrait slot. PNGs keep their alpha. Only the default names are needed.

For a complete theme, add a folder:

```text
mods/battle-hud/themes/my-theme/
  theme.json                   optional appearance settings
  background.png               optional background
  card.png                     optional party-card texture
  frame.png                    optional outer border, transparent centre
  overlay.png                  optional decoration
  portraits/
    crono.png                  optional individual portrait
    marle.png
    lucca.png
    robo.png
    frog.png
    ayla.png
    magus.png
    frame.png                  optional portrait border
  icons/
    attack.png
    tech.png
    item.png
  bars/
    track.png
    hp.png
    mp.png
    atb.png
    atb_ready.png
  font.png                     optional ASCII atlas
```

Every PNG is optional. A PNG-only theme works without JSON. **L3+R3 → Mods → HUD theme**: Left/Right selects a theme; **A reloads** its PNGs and JSON. Choosing a theme persists only `theme=<folder>` in the existing `hud.ini`. Newly added folders appear when reopening Mods or cycling themes. The portrait-HUD switch must be On; the existing HUD and symbol size controls still apply.

For each asset the order is: chosen theme PNG → active graphics pack's standard `hud/` PNG → original decoded ROM graphic. For decorative elements with no original counterpart, absence simply omits the decoration. A missing selected theme at startup uses classic. Classic with no `hud/` files calls the unchanged original HUD renderer. Bad theme JSON on live reload leaves the current working theme intact and logs the error. An invalid PNG falls back individually; it does not reject all the other valid images. Nothing is silently redrawn with a missing-image rectangle.

Install the included `remaster` (Timepiece) or `original-template` folder, then select it in Mods. Timepiece supplies navy/ivory/gold colours and small clock decorations. **It uses the original game portraits unless you add your PNGs**; illustrated concept portraits are not bundled.

## Generate JSON without AI

Open **`tools/mod-editor/index.html`** in a local browser. It works offline and sends no files to a service.

1. Choose a theme folder name, colours and the PNG role.
2. Drop one PNG into the selected role. Repeat for any other roles.
3. Download the HUD theme ZIP. Its `eggoftime/` folder merges into `/data/`.
4. For advanced settings, edit the generated JSON panel and apply it. The ZIP uses those settings and preserves PNG bytes.

The graphics-folder section creates a separate graphics mod ZIP and inventory. Choose a folder containing the edited `frames/`, `sheets/`, `screens/`, `fonts/` or `hud/` paths. Folder selection preserves the hierarchy. A loose `000.png` is intentionally not guessed. Keep an installed original-template copy as a starting point; do not edit an original game file.

The editor preview uses labelled placeholders for any portraits you have not supplied. The game uses the original portraits. It previews layout with example stats; the native renderer determines the final game appearance. It validates basic JSON/paths; the host also checks complete geometry and font bounds.

## Optional theme JSON (schema 1)

A minimal example:

```json
{
  "schema": 1,
  "name": "My theme",
  "colors": { "background": "#101831", "border": "#BA955D" },
  "assets": { "crono": "portraits/crono.png" }
}
```

All settings except `schema` are optional. The shipped theme is a working layout example. The full machine-readable reference is [`hud-theme.schema.json`](../mods/battle-hud/hud-theme.schema.json).

| Setting | Meaning / limits |
|---|---|
| `canvas` | Logical `[width,height]`, 480–1200 × 80–240; divided into three cards |
| `pixel_scale` | 1–4, at most 2,000,000 output pixels; controls renderer detail |
| `opacity` | HUD opacity 0–255; command icons remain separately transparent |
| `colors` | `#RRGGBB` or `#RRGGBBAA`: background, text, labels, border, divider, portrait_border, selected, bar_track, hp, mp, atb, atb_ready, low_hp, outline, ko |
| `layout` | `gap`, `padding` (0–40); portrait/hp_bar/mp_bar/atb_bar `[x,y,w,h]`; name/hp_label/hp_value/mp_label/mp_value/atb_label `[x,y]`, relative to each card |
| `text` | name_size/value_size/label_size 2–8 in half steps (2=1×); optional hp/mp/atb/attack/tech/item labels |
| `show` | portraits, names, labels, numbers, hp, mp, atb, dividers, selection, commands: booleans |
| `style` | border_width 0–8, grain 0–24, clocks, linear_sampling, portrait_fit cover/contain, background_mode auto/tile/stretch/window, background_tint 0–255 |
| `font` | columns/rows 1–64, first 0–127, count 1–128; ASCII range/grid must fit; tint boolean; default atlas 16×6 cells for ASCII32–127 |
| `anchor` | max_width_percent 40–100, offset_x -256..256, offset_y -192..192, command_gap 0–32; final HUD placement is bounded to the game view |
| `assets` | standard roles to relative PNG paths, or null to disable that external image; no absolute paths or traversal |
| `names` | crono/marle/lucca/robo/frog/ayla/magus display-name overrides; live game identity/stats unchanged |

The original 80×32 skin, or an integer enlargement of it, is treated as a window skin in auto mode. Other backgrounds stretch unless configured to tile. Background colour is used directly when specified without a PNG. `background_tint` overlays the configured colour on a PNG. `frame.png` and `overlay.png` stretch across the full canvas: leave their centre transparent. PNG assets are limited to 2048×2048, 16 MiB per file, and 32 MiB decoded across a theme. Config labels/names use printable ASCII; gameplay localization is outside this theme layer.

## Testing and rollback

Offline checks cover missing/invalid files, single-portrait fallback, JSON validation, alpha, resized textures, unchanged game bounds and live HUD input/state; exact native-indexed and 3×RGBA Crono frame overrides were audited against all 248 stored cells. Existing Crono JSON/Marle modules and dumper checks remain. New host boots, appearance, sustained performance and gameplay still need **PS5 hardware testing**.

Fresh startup identifies `01.006.012` in `/data/eggoftime/eggoftime.log`. Theme errors and PNG registration/fallback go to that log. The dump still uses its verified explicit kernel write/readback route; refresh FTP listings after export.

To undo appearance changes, choose classic, remove only your overrides and restart. Removing `theme=` is optional. To undo the host update, restore your previous **PPSA90436 app folder**. Leave the NDS, saves, control mappings and shared settings intact.
