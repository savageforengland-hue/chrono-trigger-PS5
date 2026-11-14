# Troubleshooting and logs

## App does not start

Check the full folder layout in [installation](INSTALLING.md). `eboot.bin`, matching runtime modules and `sce_sys/param.json` must come from the same build. Avoid duplicated nested title folders. A payload ELF and a native home-screen application are different launch formats; use the generated home-screen folder.

The app's logger can only run after the loader reaches the executable. A loader failure may therefore produce no CTPS5 log. Capture Shadow Mount / loader diagnostics if startup never reaches CTPS5. Do not change firmware or replace working system modules based only on the generic “Can't start” dialog.

## Log locations

Primary: `/data/eggoftime/eggoftime.log`.
Fallback: `/data/eggoftime-startup.log`. Restricted environments may use sandbox, `/download0` or `/temp0` fallback paths recorded by the logger. The filename retains the historical Egg of Time name.

The candidate begins with `CTPS5 0.1.0-rc3 entry reached` and uses `[CTPS5 RC3]` for kernel diagnostics. Startup failures report their actual initialization stage. If no file is created, check launcher/debug logs and whether the application reached its entry point.

## Scrambled graphics

The port includes a scanout layout fix for native tiled output. Rebuild the complete candidate and use its matching modules. If corruption persists, record output resolution, scene, a photo and the startup log. Audio continuing does not identify the precise display fault.

## Narrow scenes

Some original layouts are fixed. Select Clean panels, Original, Crop or Stretch in System Settings; see [controls](CONTROLS.md). Cropping may hide UI and stretching changes proportions.

## Report a problem

Include CTPS5 version, firmware, kstuff / Shadow Mount versions, display mode, reproduction steps, scene, configuration changes and a sanitized log. Remove personal paths and identifiers. Do not attach ROMs, saves, extracted assets or generated game code. The project does not yet have full-playthrough or sustained 4K60 validation.

## Repeated interior walls or blurred intro borders (RC1 regression)

RC1 enabled Fill screen sides, bypassing normal room-boundary clipping, and drew stretched edge columns in fixed-scene side panels. RC2 turns that forced setting off and removes blurred edge copies. Existing RC1 settings migrate once; saves and unrelated settings remain. For an immediate RC1 workaround, fully close the game and change `fillsides = no` and `screenfit = original` in `/data/eggoftime/eggoftime.ini`, or use the equivalent System Settings options. Keep the aspect ratio at 16:9. Actual PS5 confirmation of RC2 remains pending.

## DS button icons remain in the game

RC1/RC2 changed runtime prompts but did not replace the shared game sprites or encoded instructions. RC3 applies built-in PS5 art to the verified NDS data in memory before starting the game, independently of the optional Mods setting. The log should contain `PS5 game prompts:` with the sprite, text-reference and file counts (97 / 103 / 22 for the unmodified USA input with default bindings). The on-disk NDS and saves are not edited. Fully close and relaunch after changing bindings. If a particular screen still shows DS buttons, report the scene and show that screen so its additional artwork can be identified.
