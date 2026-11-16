CRONOS HOUSE DOWNSTAIRS — CONFIGURATION STARTER, NOT FINISHED ART
================================================================
Pack folder ID: cronos-house-downstairs
Minimum host: 01.006.018 for independent multiple-pack switches.

Install converted room sheets beneath:
/data/eggoftime/mods/packs/cronos-house-downstairs/graphics/
  backgrounds.json
  sheets/Map/Bg/Bg_NNNN_ncg.bin.png

Retain exact dumped sheet names, full canvas, tile layout and 2x–8x equal
integer scale. Use RGBA PNG for painted room colours. Empty/transparent cells
fall back to original art. Never rename a whole room illustration to one of
these sheet names: the art must first be mapped into the game's actual tiles.

The supplied policy scopes background sheets to native location 1 only
(Crono's downstairs). It leaves upstairs, characters and upstairs curtains
alone. Its palette reference values let painted art follow native brightness.

This starter contains no converted PNGs and will not change the game by itself.
It is not automatically installed or enabled by the release kit. The original
reference PNG must be supplied as a file before the exact conversion can be
finished. Do not substitute the older house artwork and label it as this image.

Once art is ready, copy the WHOLE graphics folder to the pack path above.
Texture mods On -> Backgrounds -> cronos-house-downstairs On -> full restart.
Disable older house packs for a clean comparison; they can stay installed.
Later applicable folder names win overlapping art when several packs are On.

The offline editor packages an edited graphics folder and preserves this
backgrounds.json. See docs/MODDING-GUIDE.md in the release/source kit.
