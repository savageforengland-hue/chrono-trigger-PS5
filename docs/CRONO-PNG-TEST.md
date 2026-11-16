# Crono battle-ready PNG test

A small loading test for the PNG-folder workflow: Crono's **two battle-ready frames (198 and 199)** use a bright purple tunic. His original pixels, hair, face, boots, transparency, canvas and actor origin are retained. These are diagnostic recolours, not new illustrated artwork or a complete battle animation replacement. Attacking, running and other poses retain their current art.

Use the matching **CTPS5 Mods host 01.006.013**. It retains the existing HUD theme/PNG features and fixes an issue found by this test: enabling an HD frame used linear sampling for the entire scene, softening Sharp/Pixel/LCD output. Those filters now retain their nearest first stage; Smooth still interpolates. Changing the filter live updates already created HD textures.

## Install and test

1. Close CTPS5 Mods. Keep the previous PPSA90436 app folder for rollback.
2. Merge the matching host ZIP's `PPSA90436/` into `/data/homebrew/`. Replace `eboot.bin`, `sce_module/libc.prx` and the supplied `sce_sys/` together. The base PPSA90435 app stays separate.
3. Merge the test ZIP's `eggoftime/` into `/data/`. It adds a separate named pack:

   ```text
   /data/eggoftime/mods/packs/crono-png-test/graphics/
     frames/Chara/Chara_0000/198.png
     frames/Chara/Chara_0000/199.png
     manifest.generated.json
   ```

4. Launch CTPS5 Mods. **L3+R3 → Mods → Texture mods = On** and **Texture pack = crono-png-test**. Note your previous pack so you can restore it. Close and relaunch the app after selecting the pack.
5. Enter a battle with Crono. His tunic should be purple during the battle-ready poses. Other poses keep their current appearance; this is only a two-frame test. His HUD portrait is unchanged.
6. To test fallback, close the app, move both PNGs outside the pack's `graphics/` folder and relaunch. His original battle-ready art should return. For a one-file test, remove just `198.png` or `199.png`; its corresponding frame returns to its original appearance.

The JSON is a generated inventory, not a hand-written registration list. The loader discovers the same relative PNG paths used by the dumper. Editing/deleting the inventory alone does not disable the PNGs.

The pack does not overwrite default `mods/graphics/`, your current pack, `hud.ini`, saves, controls or the NDS. Selecting it temporarily changes the active graphics pack; other standalone mods can continue to apply. To undo the test, restore your previous Texture pack, restart, and optionally delete only `mods/packs/crono-png-test/`. If needed, restore your older PPSA90436 app folder. Keep shared user data intact.

For a full debug report, send the new `/data/eggoftime/eggoftime.log`. Startup should identify `01.006.013`. When the pack loads it logs Crono cells 198/199; the first use logs a matched cell. The detailed-pose viewer also lists the loaded frames, if you want to inspect them while the game is paused.

## What was checked

- The actual loader discovers both native-size RGBA PNGs without a registration JSON, preserves transparency/origins and internally doubles them without changing displayed size.
- All **248 stored Crono cells** were audited. Exactly 198 and 199 match; the other 246 use their original rendering in that fixture. All 2,696 HD texels from the two opaque poses were checked against the expected diagnostic colours. No original ROM bytes or shared-tile entries changed.
- One missing PNG, one damaged PNG, an empty pack and the global texture switch all retain the appropriate original rendering.
- The actual SDL HD upload path retains all authored unmodified scene pixels under Pixel/Sharp/LCD, retains Smooth interpolation and handles live changes using the same texture.
- A **desktop run of the actual recompiled game**, using a private battle save-state fixture, was captured at the same game frame with the pack absent, installed and removed. After the sampling fix, changed pixels were confined to Crono's tunic: `[586,472,629,522)` in a 1280×720 capture. Everything outside that region matched the original capture exactly. Removing the PNGs restored a pixel-identical original battle capture.

Those are desktop/fixture checks, not a PS5 hardware result. The screenshots use original game art and private example battle stats. New **PS5 boot, appearance, gameplay and performance testing remain necessary**. No state file or original ROM is supplied.

## Recreate the test from your own ROM

The source contains no game PNGs. Supply your own supported USA/YQUE NDS and SDL source/static library:

```sh
python3 ps5/test_crono_png_pack.py \
  --sdl-source /path/to/SDL \
  --sdl-library /path/to/libSDL3.a \
  --rom /path/to/CT.nds \
  --output /path/to/new-empty-output

python3 ps5/test_hd_filter.py \
  --sdl-source /path/to/SDL \
  --sdl-library /path/to/libSDL3.a
```

The first command creates a ready-to-copy `eggoftime/mods/packs/crono-png-test/` folder and runs the valid/missing/damaged/empty fixture checks. Palette entries 8, 9 and 10 are recoloured to `#F078FF`, `#9230BC` and `#39144D`; all other entries keep their original colours.

Original PC project: **Egg of Time v1.3.5 by Power Pudu**. Its pristine source and notices remain under `upstream/`; see [credits](CREDITS.md). Generated test art and the personal native host are supplied privately, separate from the source archive.

## Expanded test v0.2.0 — visible during normal play

The user confirmed the remastered HUD works on PS5, but reported no visible Crono change from v0.1.0. Their host 01.006.013 log confirms `crono-png-test` was selected and both `198.png` and `199.png` loaded. It contains no rendering-match entry for those two poses in that session. The loader therefore succeeded; the log does not establish whether those poses were used or why they did not match. A two-pose test was too narrow to check ordinary play reliably.

**v0.2.0 keeps the same host and pack folder.** It adds native-size RGBA PNGs for all **244 nonblank Crono cells**; the four empty cells 131–134 stay blank. His original pixel artwork remains, with the same three tunic colours changed to purple. You should now be able to check the colour after loading a save and walking a few steps, as well as during battles. This is a colour diagnostic, not a remaster art pack.

Merge the revised ZIP's `eggoftime/` folder into `/data/` over the previous test pack and fully restart. Keep host **01.006.013**, **Texture mods On**, **Texture pack = crono-png-test**, and your working HUD configuration. The archive contains no host files, INIs, saves, HUD files or other characters' images. The `Crono redraw` menu switch belongs to the older JSON art pack; this PNG test is controlled by the Texture mods/Texture pack settings.

The expanded PNGs take precedence over the older Crono JSON pack for the same stored cells. Consequently `Crono JSON registered 0/8` can be expected while the expanded pack is installed: those cells are already supplied by the PNG pack. Removing the test reveals the older art pack again if it is still enabled. To return to your previous selection, restore that Texture pack and restart; no older mod files are deleted.

The expanded fixture audit checks every stored Crono cell, including identical poses stored with different actor-relative offsets. It verifies the rendered placement after matching, 371,336 opaque HD texels, original transparency and canvas geometry, and no shared-tile/ROM edits. All 1,437 stored cells of the other six party members were also checked for false replacements. Missing/corrupt PNGs and an empty folder retain their appropriate fallbacks.

A desktop run with the older eight-pose Crono pack enabled was checked in both field and battle scenes. The expanded PNGs match a normal field pose (cell 2) and the battle-ready pose (cell 199). Removing the expanded pack restores pixel-identical captures of the previous graphics in both scenes. The remastered HUD is retained during these checks. This is further desktop evidence; visible operation of v0.2.0 on PS5 remains to be tested.

To generate and verify the full test from your own supported NDS, add `--all` to the `ps5/test_crono_png_pack.py` command above. Without that flag it retains the original two-frame test for reproduction. No hand-written registration JSON is needed in either mode.
