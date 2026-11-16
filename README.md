# CTPS5

<img src="ps5/sce_sys/icon0.png" align="right" width="160" alt="CTPS5 clock and time portal artwork">

Native PS5 homebrew port of **Chrono Trigger for Nintendo DS (USA)**, based on **[Egg of Time](https://github.com/power-pudu/egg-of-time) by Power Pudu**. The original PC project supplies the static recompiler and game runtime. This repository adds the PS5 platform integration and presentation changes.

**[Project website](https://savageforengland-hue.github.io/chrono-trigger-PS5/) · [Build](docs/BUILDING.md) · [Install](docs/INSTALLING.md) · [Controls](docs/CONTROLS.md) · [Credits](docs/CREDITS.md)**

> **Feature branch:** optional [centred portrait HUD v0.3.1](docs/PORTRAIT-HUD.md), original game window styling, transparent action symbols and an L3+R3 settings/mods menu. Separate mod configuration and a side-by-side CTPS5 Mods host keep the base app installed. The previous HUD has user-confirmed PS5 operation; this restyle awaits a console visual check.

> **Sprite prototype:** [Faithful HD battle sprites](docs/BATTLE-SPRITES.md) adds
> a separate 2×/4× enhancement module for the seven main character sheets.
> It preserves animation data and live palettes. This is a conservative
> enhancement, **not the unfinished illustrated redraw from the concept art**.

> **Authored-art pilot:** [Marle detailed redraw test](docs/MARLE-PILOT.md)
> loads four illustrated poses through whole-object matching and adds an
> in-game comparison page. The reported crossbow mismatch now stays original in gameplay. Mod host 01.006.007 is required. This is a focused
> test build; the rest of the detailed redraw is unfinished.

## Release status

**0.1.0-rc3 — release candidate, 14 November 2026.** Earlier PS5 builds reached gameplay and audio on the target console. The candidate includes native scanout fixes, a direct-memory heap, DualSense prompts and adaptive widescreen presentation. RC2 restored room-boundary clipping and removed blurred scene-edge side copies after an RC1 regression report. RC3 adds PS5 replacements for identified game-owned button sprites and instructions, using each player’s NDS data in memory. Fully relaunch after changing bindings to refresh in-game art. Console save/load, extended gameplay, the latest presentation changes and sustained 4K60 still need hardware validation. This is not a stable release.

This is the **Nintendo DS version**, not the PS1 version. Functions are translated to C and compiled ahead of time. The renderer uses a software framebuffer with native VideoOut. Output supports high resolutions; sustained 4K at 60 fps is a target, not a measured guarantee. See [release notes](docs/RELEASES.md).

## Build from your own game

You supply an unmodified USA YQUE `.nds` dump. [Build instructions](docs/BUILDING.md) cover the public SDK, pinned SDL source, extraction, recompilation and native packaging. The public download contains authored source, tools and documentation; **no ROM, extracted game assets, saves, generated game code or game executable**. Generated code and the statically linked game executable must remain local. See [distribution details](docs/DISTRIBUTION.md).

## Original PC project — Power Pudu

| Resource | Link |
| --- | --- |
| Original PC source | [power-pudu/egg-of-time](https://github.com/power-pudu/egg-of-time) |
| Original PC releases | [Egg of Time releases](https://github.com/power-pudu/egg-of-time/releases) |
| Exact source used | [v1.3.5 / d294c017](https://github.com/power-pudu/egg-of-time/tree/d294c017e53d137b3729bddcd120e0f7a2b82590) |
| Pristine source included | [Unmodified original source archive](upstream/egg-of-time-v1.3.5-source.tar.gz) |
| Original copyright and license | [Copyright 2026 Power Pudu — MIT](upstream/EGG-OF-TIME-LICENSE.md) |

The pristine upstream archive preserves the original PC README, source and notices. The modified runtime and recompiler live in `runtime/` and `tools/recomp/`. PS5 additions live in `ps5/`. [Credits](docs/CREDITS.md) distinguish the original project from the port.

## Documentation

- [Installation and updates](docs/INSTALLING.md)
- [DualSense controls and widescreen choices](docs/CONTROLS.md)
- [Texture packs and modular configuration](docs/MODS.md)
- [Logs and troubleshooting](docs/TROUBLESHOOTING.md)
- [Release status and validation](docs/RELEASES.md)
- [Licensing and third-party notices](LICENSING.md)
- [Contributing and reporting issues](CONTRIBUTING.md)
- [Publishing / previewing the GitHub Pages site](docs/PUBLISHING.md)

## Licenses

The combined PS5 distribution includes GPL-3.0-or-later components; see [LICENSE](LICENSE), [LICENSING.md](LICENSING.md) and component notices. Power Pudu's original code remains MIT licensed, with its notice preserved verbatim. SDL is under the zlib license; other component licenses remain in their source directories.

Unofficial fan project, not affiliated with or endorsed by Square Enix, Nintendo or Sony. Chrono Trigger and the associated game content belong to their respective owners.

The [01.006.007 HUD fixes](docs/HUD-FIXES-007.md) remove the fixed black cover and stock battle widgets when the portrait HUD is active, and synchronize its state with the displayed frame.

The [Crono eight-pose art test](docs/CRONO-POSE-TEST.md) adds JSON-selected redraws with host 01.006.011. This is a focused test; the complete sprite set remains unfinished.

The [01.006.010 graphics dumper update](docs/GRAPHICS-DUMP.md) uses explicit PS5 kernel paths for exports and reports, checks a real sprite at startup, and logs requests and failures in `eggoftime.log`. The working dump path is retained by the Crono test host.

The [01.006.012 HUD themes and PNG workflow](docs/HUD-THEMES-AND-PNG-MODS.md) updates the existing HUD with optional portrait/icon/background PNGs and configurable colours/layout. The included offline editor generates JSON and mod ZIPs. Supported graphics use their dumped relative paths; missing overrides use original game assets. This host update awaits PS5 validation.

The [Crono two-frame PNG test](docs/CRONO-PNG-TEST.md) uses a purple tunic to verify same-path battle-ready replacements and removal fallback. Host 01.006.013 fixes scene softening found during the desktop battle comparison; PS5 testing remains pending.

The expanded [Crono PNG test v0.2.0](docs/CRONO-PNG-TEST.md#expanded-test-v020--visible-during-normal-play) covers all 244 visible poses so the colour change can be checked while walking. The user confirmed the remastered HUD works on PS5; the first two PNGs loaded there but had no logged rendering match in the supplied session. The expanded pack retains host 01.006.013 and awaits a console visual result.
