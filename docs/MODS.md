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
