# Credits and original source

## Power Pudu — Egg of Time (original PC project)

**[Egg of Time](https://github.com/power-pudu/egg-of-time) by Power Pudu** is the original native PC Chrono Trigger Nintendo DS recompilation project. Its recompiler, runtime, reverse-engineering work, tools and build foundation make this PS5 port possible. CTPS5 is a downstream port and does not claim authorship of that original work.

- [Original PC source repository](https://github.com/power-pudu/egg-of-time)
- [Original PC releases](https://github.com/power-pudu/egg-of-time/releases)
- [Exact v1.3.5 source used](https://github.com/power-pudu/egg-of-time/tree/d294c017e53d137b3729bddcd120e0f7a2b82590)
- Pristine bundled source: `upstream/egg-of-time-v1.3.5-source.tar.gz`
- Original notice: [Copyright 2026 Power Pudu, MIT](../upstream/EGG-OF-TIME-LICENSE.md)
- [Support the original author](https://ko-fi.com/powerpudu)

The pristine archive is generated directly from upstream commit `d294c017e53d137b3729bddcd120e0f7a2b82590`. Its SHA-256 and provenance are in `upstream/UPSTREAM.json`. It includes the original desktop README and licensing. Modified runtime files remain in `runtime/`; tooling is in `tools/`.

## PS5 port and dependencies

CTPS5 adds PS5 platform integration, native packaging, direct-memory allocation, display addressing fixes, diagnostics, texture pack configuration, widescreen presentation and DualSense runtime prompts. The port was developed with OpenAI Codex assistance at the project owner's direction.

| Component | Credit / source | License / notice |
| --- | --- | --- |
| Native app tooling / platform integration foundation | BlackBearReloaded and drakmor, [ps5-native-app-boilerplate](https://github.com/drakmor/ps5-native-app-boilerplate) | `ps5/native/NOTICE.md`, `ps5/native/LICENSE` |
| Direct memory heap foundation | Mihawk, [PS5_PayloadSDK](https://github.com/mihawk-99/PS5_PayloadSDK) / pinned provenance in source | `ps5/compat/UPSTREAM.txt`, GPL-3.0-or-later source headers |
| SDL PS5 backend | [OpenAGC/SDL3](https://github.com/OpenAGC/SDL3) and SDL contributors | zlib; patch at `ps5/sdl3-static.patch` |
| Public PS5 toolchain | [ps5-payload-dev](https://github.com/ps5-payload-dev/pacbrew-repo) | Upstream SDK component licenses |
| NDS extraction / decomp tooling | [AetiasHax/ds-decomp](https://github.com/AetiasHax/ds-decomp) | Upstream component licenses |
| Allocator | Doug Lea's dlmalloc | MIT-0 notice in source |
| Fonts | PixelMplus and the font authors credited upstream | Original source notices retained |

Component-specific authorship, exact revisions and notices in the source take precedence over this abbreviated table. The in-game PS5 button art is authored procedurally in `ps5/game_prompt_icons.c`; the port does not bundle original DS button textures. The icon uses the project owner's supplied clock/portal artwork; its preparation provenance is in `ps5/art/ICON-PROVENANCE.json`.

Chrono Trigger is owned by its respective rights holders, including Square Enix. Nintendo, PlayStation, PS5 and DualSense marks belong to their owners. This is an unofficial project with no endorsement implied.
