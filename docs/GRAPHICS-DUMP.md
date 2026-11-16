# Graphics dumper

## Current behavior — host 01.006.014 and later

The automatic startup write check has been removed at the user's request.
Normal launch no longer writes the diagnostic sprite or starts a graphics
export. Existing exports are retained. The full dumper remains available at
**L3+R3 -> Settings -> Dump graphics -> Yes** and writes to
`/data/eggoftime/graphics-export/` using the same kernel filesystem route.
The filename-based PNG loader and remastered HUD are unchanged.

The material below records the earlier 01.006.010 diagnostic workflow; its
startup-check instructions do not apply to 01.006.014 or later.

## Historical diagnostic workflow — host 01.006.010

The 01.006.009 console log confirms that the one-sprite check wrote,
reopened and decoded its PNG, yet the user could not find the export folder
over FTP. In-app readback alone therefore does not establish that another
process sees the output. The diagnostic log itself remains FTP-readable.

This update routes dump directories, PNGs, reports and metadata through
explicit **sceKernel** pathname APIs, matching the persistent diagnostic
log's `sceKernelOpen` route. PNG encoding happens in memory with
`SDL_SavePNG_IO`; SDL never opens the output pathname. Encoded bytes are
written through a kernel-opened descriptor, flushed, synced, closed and
reopened for a complete byte comparison before publication. The startup
PNG is also decoded through bytes from that same adapter. Ordinary libc
pathname calls and `SDL_SavePNG(path)` / `SDL_LoadPNG(path)` are excluded
from this export path. These changes are confined to the dumper; saves,
settings and installed mod loading keep their existing implementations.

Before the game thread starts, the dumper writes **one real sprite from the player's ROM**, reopens the PNG,
decodes it and compares every pixel with the source. It uses this export root:

```
/data/eggoftime/graphics-export/frames/Chara/Chara_0000/199.png
```

The startup notification explicitly says **Write check passed (not a full
dump)**. This is a small filesystem/encoder check, not a claim that all
graphics were exported. There is no background thread to wait for during
this check. A failure includes the first error in the notification.

## Install and check

1. Fully close CTPS5 Mods and keep the old host folder for rollback.
2. Merge the supplied `PPSA90436` into `/data/homebrew/`, replacing the
   executable, matching runtime and `sce_sys` together. The executable is
   `/data/homebrew/PPSA90436/eboot.bin`; avoid nested PPSA90436 folders.
3. Refresh ShadowMountPlus and launch **CTPS5 Mods**.
4. Refresh FTP and check the exact PNG path above. It should exist after
   the startup write check; no full export is needed for this first check.

Keep `/data/eggoftime/` intact. Settings, saves, ROM, installed mods and the
old `mods/graphics/dump/` directory are retained. The separate base app
`PPSA90435` is unchanged. No new sprite artwork is included.

## Full export and diagnostics

Use **L3+R3 → Settings → Dump graphics → Yes**. The full export runs on a
background thread and uses the **new** `graphics-export/` root. It holds
`frames/`, `sheets/`, `screens/`, `fonts/` and `palettes.txt`. Paths below
those subdirectories retain the graphics-mod naming scheme.

The reliable diagnostic destination is the existing
**`/data/eggoftime/eggoftime.log`**. It records the request before worker
creation, worker entry, the first saved/reopened PNG, progress every 128
PNGs, and the result. If the dedicated
`/data/eggoftime/graphics-dump.log` cannot be opened, the usual log records
its path and error and continues logging there. The dedicated report is
replaced by each operation; the usual log also distinguishes startup checks
from full exports. Startup identifies **CTPS5 kernel graphics export
(01.006.010)**.

The main log identifies the explicit kernel filesystem route.
New directories use 0755 and output files 0644. PNGs are published through
checked temporary files and reopened before the count increases. The
original indexed encoder, palette indexes and transparency are retained.
Zero output or partial failure is reported as failure. A saved count means
files reopened successfully inside the app; FTP visibility must still be
confirmed on the console.

## Verification and limits

The PS5-adapter routing test replaces kernel functions with checked host
providers while deliberately denying ordinary pathname APIs. It writes,
reads and decodes the PNG and report, verifies file modes, normalizes native
error codes, and checks that a sync failure preserves the prior valid PNG.
This is an API-routing test, not an emulation of the PS5 filesystem.

Desktop tests also cover the real-ROM single-sprite encode/decode comparison,
a full export with every PNG chunk CRC and decompressed stream checked,
exact palette indexes/colours/transparency, blocked output, write/close/
rename failures, corrupt PNGs, preservation of a prior valid file, and
completion messages. The explicit legacy permission-repair test verifies
unchanged image bytes and skips external symbolic links. ROM memory stays
unchanged. Native container/runtime/import validation is separate from
**PS5 hardware and FTP validation, which remain pending**.

Original PC source and notices remain under `upstream/`: **Egg of Time
v1.3.5 by Power Pudu**. See [credits](CREDITS.md). Players supply their own
supported USA/YQUE NDS; no ROM or extracted PNGs are distributed.

## HUD aliases (01.006.012)

Full exports now also include 13 RGBA HUD PNGs under `hud/`: seven named portraits, three command icons, the original window skin, an optional transparent-centre frame and a font atlas. An optional `hud/theme.json` template accompanies them. The existing indexed sprite/background exports are unchanged. See [HUD themes and PNG replacement paths](HUD-THEMES-AND-PNG-MODS.md). The own-ROM full export test now verifies 30,590 PNGs and preserves ROM memory.
