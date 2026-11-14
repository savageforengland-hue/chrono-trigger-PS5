# Troubleshooting and logs

## App does not start

Check the full folder layout in [installation](INSTALLING.md). `eboot.bin`, matching runtime modules and `sce_sys/param.json` must come from the same build. Avoid duplicated nested title folders. A payload ELF and a native home-screen application are different launch formats; use the generated home-screen folder.

The app's logger can only run after the loader reaches the executable. A loader failure may therefore produce no CTPS5 log. Capture Shadow Mount / loader diagnostics if startup never reaches CTPS5. Do not change firmware or replace working system modules based only on the generic “Can't start” dialog.

## Log locations

Primary: `/data/eggoftime/eggoftime.log`.
Fallback: `/data/eggoftime-startup.log`. Restricted environments may use sandbox, `/download0` or `/temp0` fallback paths recorded by the logger. The filename retains the historical Egg of Time name.

The candidate begins with `CTPS5 0.1.0-rc1 entry reached` and uses `[CTPS5 RC1]` for kernel diagnostics. Startup failures report their actual initialization stage. If no file is created, check launcher/debug logs and whether the application reached its entry point.

## Scrambled graphics

The port includes a scanout layout fix for native tiled output. Rebuild the complete candidate and use its matching modules. If corruption persists, record output resolution, scene, a photo and the startup log. Audio continuing does not identify the precise display fault.

## Narrow scenes

Some original layouts are fixed. Select Matching panels, Original, Crop or Stretch in System Settings; see [controls](CONTROLS.md). Cropping may hide UI and stretching changes proportions.

## Report a problem

Include CTPS5 version, firmware, kstuff / Shadow Mount versions, display mode, reproduction steps, scene, configuration changes and a sanitized log. Remove personal paths and identifiers. Do not attach ROMs, saves, extracted assets or generated game code. The project does not yet have full-playthrough or sustained 4K60 validation.
