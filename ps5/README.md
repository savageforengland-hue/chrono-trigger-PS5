# CTPS5 PS5 platform source

Native PS5 homebrew integration for **Egg of Time v1.3.5 by Power Pudu**. See the [original PC project](https://github.com/power-pudu/egg-of-time), [credits](../docs/CREDITS.md) and preserved notices.

The current source is **0.1.0-rc1**. The public project contains authored source only. Build locally from your own unmodified Nintendo DS USA YQUE input; the generated game C and statically linked executable remain local.

- [Build guide and pinned dependencies](../docs/BUILDING.md)
- [Installation and complete folder layout](../docs/INSTALLING.md)
- [Controls and widescreen presentation](../docs/CONTROLS.md)
- [Texture packs and configuration](../docs/MODS.md)
- [Diagnostics and logs](../docs/TROUBLESHOOTING.md)
- [Release status and remaining validation](../docs/RELEASES.md)
- [Historical development and loader investigations](../docs/HISTORY-PS5.md)

## Modules

| Module | Responsibility |
| --- | --- |
| game_prepare.c | Validate the local NDS and bind the static game table |
| platform.c / diagnostics.c | Native initialization, persistent logs, stages and failure reporting |
| memory.c / compat/ | Direct-memory heap and native compatibility support |
| config.c / mods.c | Display configuration and named graphics packs |
| presentation.c / prompts.c | Fixed-screen framing and binding-aware DualSense symbols |
| controller_only.c | Controller-only native input profile |
| optional_features.c | Core profile adapters for disabled PC integrations |
| native/ / link_native.py | Native metadata, clean-room runtime, linking and package auditing |
| sdl3-static.patch | Pinned downstream SDL backend and scanout changes |

Platform features do not modify locally generated game C. EGG_PLATFORM_MODULE and EGG_FEATURE_MODULE are CMake cache paths for alternate implementations of the same entry points. New dependencies must be linked through real SDK providers in both link paths.

The current renderer uses the SDL CPU framebuffer backend and native PS5 VideoOut. High-resolution output support does not establish sustained 4K60. Earlier builds reached game startup and music on the target console; the latest candidate still requires the hardware validation recorded in the release notes.
