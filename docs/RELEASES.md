# Release notes and validation

## 0.1.0-rc2 — 14 November 2026

Presentation regression correction; metadata version 01.005.000. RC1 switched Fill screen sides on, which bypasses room-boundary clipping. A user reported overlapping room edges in houses and blurred intro side bars. RC2 restores room clipping by default, migrates RC1's forced fill/framed settings once and removes stretched scene-edge copies from panels. Native wider rendering remains enabled where supported. Saves, input mappings and unrelated settings are retained; later explicit presentation choices are respected.

Host checks exercise actual settings load/save, RC1 migration and panel draw calls without a ROM. The published RC1 source was also rebuilt from a clean directory through both native application folders and passed package/layout regression checks. RC2 rebuilt and relinked successfully, with the clean-room runtime reproduced and 186 imports verified; see the [RC2 build report](RC2-BUILD-REPORT.json). Console confirmation remains pending. Full-playthrough, save/load and sustained 4K60 validation remain outstanding. This is a release candidate, not a confirmed console fix.


## 0.1.0-rc1 — 14 November 2026

Release candidate for the native PS5 home-screen application. Metadata version 01.004.000; title ID PPSA90435.

Changes include a native direct-memory allocator, corrected tiled scanout addressing, adaptive fixed-screen presentation, binding-aware DualSense prompts, the requested clock/portal icon and retained settings / data paths under the CTPS5 name. Normal startup notifications are quiet; failures retain diagnostic messages and logs.

Earlier builds reached the native game thread, ARM9 startup and music on the user's PS5. A later report described gameplay with narrow scenes. Host checks cover allocator ownership, error paths, scanout addressing, presentation geometry, prompt bindings and settings migration. These are not a full console playthrough.

Before calling the release stable, validate on hardware: repeated cold boots, save/load across restarts, menus and dialogue, field transitions, battles, long sessions, each output resolution, mod loading and widescreen choices. Sustained 4K60 and all-area widescreen have not been established. The software framebuffer is the current rendering path.

The local build translated 9,022 functions. The whole-module decode also contains 1,073 unimplemented markers, including code/data ambiguity; this count does not prove those paths are reachable, nor that every gameplay path is implemented. There is no CPU interpreter fallback.

[Historical build report](RC1-BUILD-REPORT.json) records the local candidate's build evidence. The public source package excludes the ROM-derived executable and generated game C; each player builds locally from their own input.
