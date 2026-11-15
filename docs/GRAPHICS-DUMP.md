# Graphics dump and FTP access — host 01.006.008

The reported FTP window said **Permission denied** while listing the dump.
The supplied game log recorded **30,577 images from 3,648 files**, after
2,377.6 seconds, under `/data/eggoftime/mods/graphics/dump/`. That count
established that the encoder reported success; it did not establish that a
separate FTP service could list or read the output. The previous SDL folder
creation path used mode `0770`, which can exclude that service.

This host uses **0755 for dump directories** and **0644 for exported files**.
It checks directory failures, publishes complete indexed PNGs through a
checked temporary file, reopens the final file to validate its header and
end marker, and reports the full output path. The original indexed PNG
encoder, palette indexes and transparency rules are retained. Failed or
partial output is reported as incomplete, with the first error recorded.

At PS5 startup it also repairs permissions on an **existing dump**, on a
background thread. This traverses only the dump subtree, skips symbolic
links, and leaves image contents unchanged. It verifies the existing PNGs
and announces **FTP access repaired** when successful. It does not repeat
the full export. No dump repair runs when that directory is absent.

## Install and find the files

Fully close CTPS5 Mods. Merge the included `PPSA90436` folder into
`/data/homebrew/`, replacing its matching executable, runtime and `sce_sys`
files. Avoid an extra nested `PPSA90436/PPSA90436` directory. Refresh
ShadowMountPlus, then launch **CTPS5 Mods** and allow the access repair to
finish. Refresh the FTP listing or reconnect afterward.

Keep `/data/eggoftime/` intact. There is no need to delete an existing dump,
replace settings or supply the NDS again. Both the mod host and base app use
that shared data folder. The separate base `PPSA90435` app is not changed.

The dump root holds subdirectories; PNGs are nested, for example:

```
/data/eggoftime/mods/graphics/dump/
  frames/Chara/Chara_0000/199.png
  sheets/Chara/Chara_0000.NCGR.png
  screens/...
  fonts/...
  palettes.txt
```

The dedicated report is **`/data/eggoftime/graphics-dump.log`**, with the
operation, output path, verified PNG count, errors and elapsed time. The
usual `eggoftime.log` also receives the result. Startup identifies
**CTPS5 graphics dumper fix 1 (01.006.008)**.

For a new export, use **L3+R3 → Settings → Dump graphics**, then confirm.
The game may keep running while it works. This reads each player's supported
USA/YQUE NDS; no game dump or extracted images are distributed with the fix.

## Verification and limits

The desktop own-ROM test wrote and decoded **30,577 indexed PNGs**, checked
every PNG chunk CRC and decompressed pixel stream, and matched the on-disk
count to the completion report. Tests cover exact indexes/palettes/alpha,
large images, denied directories, incomplete/corrupt PNGs, write/close/rename
failures, preservation of a prior valid file, and menu completion messages.
A repair test changed the dump to restricted permissions, restored every
folder to 0755 and PNG to 0644, checked unchanged image bytes and an untouched
external symlink target, and verified that the ROM memory stayed unchanged.

Native container/runtime and import checks are distinct from hardware
validation. **PS5 FTP confirmation of this new build remains pending.**
Desktop durations do not predict console export or repair time.

This update is limited to the dumper/access repair on the 01.006.007 host.
The unfinished Crono sprite work is paused and excluded. Original PC source
and notices remain under `upstream/`: **Egg of Time v1.3.5 by Power Pudu**.
See [credits](CREDITS.md).
