# CTPS5 host 01.006.007 — HUD and sprite status fixes

Shrinking the portrait HUD previously exposed a fixed black rectangle. That
rectangle covered the stock panel; it was not stale resized texture memory.
The host now omits the original panel's screen-space objects before composing
the picture. Both the 41-widget list and separate window-piece lists are
handled. Scenery remains around the smaller centered portrait HUD. There is
no black cover rectangle. Turning the portrait HUD Off restores stock UI.
Tech/Item panels remain available when their original game state opens them;
world-space damage numbers and target brackets are retained.

The HUD snapshot is latched before scanline composition, published under the
same lock as the game image, and copied even when no HD frame is active.
Rendering uses that immutable displayed snapshot instead of a separately
advancing game-thread snapshot. This addresses a frame mismatch that could
be exposed by slower HD composition. PS5 confirmation of the reported
flicker remains necessary; a desktop run cannot establish every console case.

The eight-row Mods page now distinguishes **Sprite smoothing** (the older
optional own-ROM filter) from **Marle redraw** (illustrated fear poses).
Missing smoothing configuration no longer appears to describe the separate
redraw. Smoothing reports the loaded runtime state and pending restart;
Texture mods Off is shown as a blocker. Marle redraw has its own live,
persisted gameplay switch. **Detailed pose test** still opens comparisons.

The reported Marle crossbow/waiting/hit illustration is **excluded from
normal gameplay**, including already installed pilot PNGs. The original art
is restored for that cell. It stays available on the static comparison page
for review; it has not been claimed corrected or animation-approved. Only
fear cells 89/90/91 are eligible for illustrated replacement. No animation
order/timing, game logic, saves, ROM bytes or actor positions are rewritten.

## Update

Fully close CTPS5 Mods. Merge the included **PPSA90436** app folder into
`/data/homebrew/`, replacing the executable, its matching `sce_module/libc.prx`
and included `sce_sys` files. Refresh ShadowMountPlus. The home-screen tile
remains CTPS5 Mods. Avoid nested `PPSA90436/PPSA90436` folders.

Keep `/data/eggoftime/` intact, including HUD settings, artwork, ROM, saves
and controls. No user configuration templates need replacing. No new art
installation is required for these host fixes. PPSA90435 is the separate
base app; both apps share `/data/eggoftime/`. The older host can be retained
as a rollback. Startup identifies **CTPS5 HUD and sprite fixes 7 (01.006.007)**.
Usual log: `/data/eggoftime/eggoftime.log`.

## Verification

Authored checks exercise resize geometry without an opaque cover, copied
HUD state surviving later game updates, stock-widget/target-cursor guards,
Tech/Item yield, battle exit, menu pause/input release, eight-row navigation,
mod setting persistence, sprite palette effects and original-art fallback
for the quarantined cell. Own-ROM tests still verify PNG registration,
original actor origins, missing-art behavior and unchanged ROM memory.
Desktop screenshots are labeled as desktop output. Native container/runtime
and import checks are separate from hardware validation. **PS5 visual testing
of this new host remains pending.** Sustained 4K60 and a full animation
playthrough are not established.

Built on Egg of Time v1.3.5 by **Power Pudu**. Original PC source and notices
remain under `upstream/`; see [credits](CREDITS.md). Players provide their own
supported USA/YQUE `CT.nds`.
