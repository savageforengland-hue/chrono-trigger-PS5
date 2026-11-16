# Installing and updating

For the current **CTPS5 Mods 01.006.018** app (PPSA90436), use the
[release-kit setup guide](RELEASE-KIT.md). The instructions below describe the
base PPSA90435 app; do not substitute its folder name when updating CTPS5 Mods.

The public release is source only. First [build locally](BUILDING.md) using your own USA YQUE NDS file.

The target setup is PS5 firmware 13.20 with kstuff and Shadow Mount Plus 1.7beta11. Other combinations have not been established. Use your existing working homebrew environment.

1. Back up your saves and settings under `/data/eggoftime/` before changing builds.
2. Copy the **whole** generated `PPSA90435` folder into `/data/homebrew/`. Keep `eboot.bin`, `sce_module/` and `sce_sys/` together. Do not nest it under another PPSA90435 folder.
3. Put your own game dump at `/data/eggoftime/rom/CT.nds`.
4. Refresh/register the folder using your installed homebrew launcher or Shadow Mount workflow and launch **CTPS5** from the home screen.

```text
/data/homebrew/PPSA90435/
  eboot.bin
  sce_module/
    libc.prx
  sce_sys/
    param.json
    icon0.png
/data/eggoftime/
  rom/
    CT.nds
```

Retain every other module emitted by the build, if present. The generated metadata uses title ID PPSA90435 and version 01.006.000. The existing `/data/eggoftime/` data path is retained after renaming the project to CTPS5.

Copy the complete rebuilt app folder when updating; preserve your personal data directory. Texture packs, saves and input files are not supplied. Earlier native builds booted on the target console; the release candidate still needs the hardware checks in [release notes](RELEASES.md).
