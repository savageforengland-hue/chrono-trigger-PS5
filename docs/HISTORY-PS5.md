# Historical PS5 development notes

These notes preserve the investigation history and observations at each older revision. Statements that a feature was untested describe that revision; consult RELEASES.md and the main README for current candidate status.

# CTPS5 — native PS5 port

Experimental source port of Egg of Time v1.3.5 for a native ShadowMount folder title. The supported input is the unmodified USA DS ROM, YQUE, SHA-1 `11f9e8204e484eab42181b210c9f214584be75bf`. This is not a PS1 port.

| Module | Responsibility |
|---|---|
| `game_prepare.c` | Verify the supplied ROM and bind the statically linked game table. |
| `diagnostics.c` / `diagnostics.h` | Entry-point kernel logging, sandbox fallbacks, startup checkpoints, fatal-signal records. |
| `platform.c` | Native host initialization, log sink, and SDL backend hints. |
| `memory.c` / `memory.h`, `compat/heap*`, `compat/placement*`, `compat/dlmalloc/` | Direct-memory application heap, allocator preflight and usage diagnostics. |
| `config.c` / `config.h` | Output profiles, target frame pacing, internal scale, and active pack. |
| `mods.c` | Resolve the selected named graphics pack, with the default folder as fallback. |
| `optional_features.c` | Original-game audio/cutscene route and disabled optional PC integrations. |
| `install_texture_pack.py` | Install/update data-only packs on the PC, with a previous-version backup. |
| `link_native.py` | Link SDK providers through LLVM/lld, convert native SCE metadata, sign the app, and build/include the clean-room runtime. |
| `verify_native.py` / `test_native_package.py` | Audit the folder and regress missing/raw/corrupt module failures. |
| `native/` | Pinned GPL native tooling, startup, runtime manifests, licenses and provenance. |
| `sdl3-static.patch` | Static SDL API and runtime 1080p/1440p/4K framebuffer profiles. |

The generated game C is not edited for platform features. Runtime/platform changes rebuild their own translation units. `EGG_PLATFORM_MODULE` and `EGG_FEATURE_MODULE` are CMake cache paths for alternate implementations of the same entry points. The defaults are the included PS5 bootstrap and core-only feature adapter. Additional native dependencies must be added to both the payload target and native link stage, with actual SDK exports checked rather than invented import names.

## Pinned dependencies

R7 diagnostic update (14 November 2026): R6 has now reached SDL_Init on the
user's console, but its advertised app log could not be found through FTP.
R7 puts the actual startup error in a PS5 notification instead of showing only
the stage name. It also includes the corrected SDL boolean error returns and
native framebuffer error codes described in the PE2 follow-up below. These
changes improve correctness and diagnosis; R7 boot/gameplay is unverified.

- Egg of Time v1.3.5: `d294c017e53d137b3729bddcd120e0f7a2b82590`.
- [OpenAGC/SDL3](https://github.com/OpenAGC/SDL3): `37ff89ab9233e49a39b362b2023cab64520d9cca`, plus the supplied downstream patch. This is the public homebrew backend.
- [drakmor/ps5-native-app-boilerplate](https://github.com/drakmor/ps5-native-app-boilerplate): `470695e0c557f99ff2df0e36e4df713c5e636526`; native tooling sources and runtime manifests vendored under `ps5/native/`. Authored by BlackBearReloaded, GPL-3.0-or-later.
- [ps5-payload-dev/pacbrew-repo](https://github.com/ps5-payload-dev/pacbrew-repo/releases/tag/v0.41.1), `ps5-payload-dev.tar.gz`, SHA-256 `e811bc2e4676c5bb7f80a8fe928d2985dce599996a5ca6b54da3e72699287887`.
- Host tools: clang/LLVM/lld 19.1.7, CMake 3.31.6, Ninja 1.11.1, Python 3.11+ with Capstone 5.0.7, dsd 0.12.1, a host C++20 compiler and zlib development headers/library (`-lz`).

Install/extract the public SDK with its `bin/`, `target/`, and `toolchain/` under one prefix. Its compiler wrappers use LLVM tools; `LLVM_CONFIG` can select a non-system LLVM installation. The native linker tools build from the included source with the host compiler. No .NET toolchain is needed.

```sh
export PS5_PAYLOAD_SDK=/path/to/ps5-payload-sdk
export SDL3_SOURCE=/path/to/pinned/SDL3
export DSD=/path/to/dsd
# Optional: PYTHON, CMAKE, CXX_FOR_BUILD, LLVM_CONFIG, EGG_JOBS (default 2), EGG_BUILD_DIR.
bash ps5/build.sh /path/to/CT.nds
```

Result: `build-ps5/package/PPSA90435/eboot.bin`, `sce_sys/param.json`, `icon0.png`, and the required `sce_module/libc.prx`. The linked payload ELF is also built under `build-ps5/app/`, but the native folder title is the intended ShadowMount entry. Copying the ROM is a separate installation step.

Keep optimization at `-O1` for generated code, as upstream does. An unoptimized Linux reference run crashed; the optimized reference passed the arithmetic test and startup/menu smoke test. Unknown translated call targets fail explicitly; the port adds no CPU interpreter fallback.

## Display and scaling

The current presentation module is SDL3's CPU framebuffer backend, with 1080p/1440p/4K scanout at 60 Hz and a selectable 30/60 presentation target. Internal scale 1–11 is independent of output resolution. High-scale PNG art is supported by the retained graphics pipeline. Hardware throughput has not been measured; adding a GPU presenter is a separate future module, not a capability claimed by this build.

Do not treat Linux FPS logs, a zero-unresolved-symbol link, or a valid SELF digest as a console compatibility or performance test. Verify boot, video, input, audio, a save/load cycle, and frame pacing on the actual PS5 before calling a release playable.

## Texture pack format v1

`pack.json` at the ZIP root declares `format_version: 1`, a lowercase `id`, `name`, `version`, and `game_code: "YQUE"`. Replacement PNGs and font-width text files use `graphics/{sheets,frames,screens,fonts}/` paths from the upstream dump. The `default` ID is reserved for the flat graphics folder. Packs replace assets; they do not carry executable code or rewrite translated instructions. Runtime selection is in `ps5.ini`; restart after changing packs or files.

The installer validates paths and game/format metadata, stages an update, retains one previous version, and changes only the active-pack line when requested. Transfer the resulting data directory to the console using your normal file-transfer method.

The R8 package uses the user-requested gold clock/portal Chrono Trigger PS5 artwork in place of the earlier abstract clock icon. The app icon is a 512x512 PNG; the source includes its prepared master and desktop ICO. No textures, screenshots, music, movies, ROM images, or other assets extracted from the DS game are supplied.

## R2 native loader repair

The old build omitted `sce_module/libc.prx` and routed C library calls to the missing module. R2 uses the included native startup to call `_init_env` before `main`; it imports the real `__isthreaded` flag rather than exporting a local copy. Standard C functions bind to `libSceLibcInternal`. The clean-room loader companion is not a functional replacement for that C library and must never receive those imports.

`link_native.py` builds host tools, reproduces and checks the raw shim SHA-256 `8ee6e124993e1af26420cb455890fd002f5d6c7e78883c860ce45734e7d002bb`, and checks signed shim SHA-256 `e6ff45d16adf687855cc3b33b0c8a4132b6504360b221e0a34c7e99fb3ba0036`. It then links the existing generated objects, converts imports from real SDK providers, signs the title, and runs the artifact audit. The loader companion contains no Sony implementation code. Its upstream reports tests on firmware 6.02 and 12.70; our firmware 13.20 target remains untested.

The offline verifier extracts both SELF files, validates their integrity, and checks every undefined import NID against public SDK exports. It also checks required app metadata. Run the real-artifact regression check after building:

```sh
python3 ps5/test_native_package.py build-ps5/package/PPSA90435 \
  --tool build-ps5/app/native-tools/ps5-native-tool --sdk "$PS5_PAYLOAD_SDK"
```

The corrected metadata uses a five-digit concept ID and `gameIntent` for its Games entry. Copy the full folder when updating. None of these offline checks establish on-console filesystem access, rendering, input, sound or gameplay.

## R3 diagnostics

R2 failed the user console launch test despite successful offline checks. Other native apps work on their firmware 13.20 / ShadowMountPlus v1.7 beta 11 setup. R3 instruments the native entry point before `_init_env`, then installs signal handlers after initialization and records all main startup stages. It keeps the ROM and existing user data paths unchanged. `downloadDataSize` is positive to enable the `/download0` logging fallback. See `DIAGNOSTICS.txt` for paths, fields, signal limitations and the continued need for loader logs when entry is never reached.

`native/app_crt.cpp` now has downstream logging hooks; the converter, SELF writer and clean-room runtime manifests remain pinned upstream sources. `diagnostics.c` uses `-fno-builtin` so its pre-runtime path does not synthesize libc calls. Before `_init_env` it calls only its own allocation-free formatters and public kernel file APIs. The later signal handler uses no stdio, SDL or heap allocation. It restores the default fatal action through SA_RESETHAND and signal redelivery so the OS can retain normal crash handling.

Run `python3 ps5/test_diagnostics.py` on a Linux host to exercise actual normal/fatal paths with filesystem API shims. This validates log formatting and control flow, not PS5 runtime initialization, signal ABI, sandbox mounts or console launch. Do not describe R3 as a proven launch fix.

## R4 compatibility reference

`APRICOT-COMPARISON.txt` records the uploaded source comparison. The selected
GPL native directory and IEEE FP code from its pinned SDK is under `compat/`,
with author notices, exact commit, license, upstream tests and the small local
conditional noted in `UPSTREAM.txt`. CMake compiles that code; the native link
binds directory APIs locally to the compatible family. The native artifact gate
rejects direct `opendir`/`readdir`/related/`dup2` imports and checks the kernel
logging and shell-exit providers. A matching stub export still does not prove
that arbitrary APIs are granted to a native title on firmware 13.20.

R4 replaces `dup2` stdio capture with `fdopen` streams and adds kernel-log
checkpoints. Its `catchReturnFromMain` asks the PS5 shell to close the process
and waits rather than returning immediately to `exit`. Fatal-signal handling
keeps its earlier termination behavior. Logging must remain in future builds.

Host checks: `python3 ps5/test_diagnostics.py` and
`python3 ps5/test_compat.py --sdl-source /path/to/pinned/SDL3`.
These cover logging under denied dup2, crafted FreeBSD directory records,
floating-point state, and the shell-close API/control flow. The selected public
SDK code uses the existing public toolchain; no full SDK or graphics-stack
migration was performed. Console boot, actual directory access and closure
remain untested. Preserve these distinctions in release notes.

## R5 RELRO converter correction

`LOADER-FIX.txt` describes the malformed load segment found by comparing the
actual user-reported working Parasitic Event Revisited binary with our R4.
`native/sce_module_writer.cpp` now derives the RELRO file start from the GOT's
file/address delta rather than assuming the GOT starts the region. It also
checks PT_LOAD congruence and each retained allocated section's mapped bytes.
`verify_native.py` independently validates geometry, and `link_native.py`
compares the converted image to the LLVM section bytes before signing.

Regression command after building:

```sh
python3 ps5/test_native_layout.py \
  --llvm-input build-ps5/app/eggoftime-llvm.elf \
  --native-elf build-ps5/app/eggoftime-native.elf
```

It checks the corrected output and recreates the old error to confirm rejection.
Optional `--known-broken-elf` and `--working-reference-elf` inputs were used during
this repair; these third-party/old binaries are not needed for the source test
and are not bundled. Game code is unaffected by this converter correction.
SELF integrity or stub resolution alone must never again be described as an
ELF loadability check. A PS5 test remains required after offline checks pass.

## R6 controller profile and native home-screen Boot Check

The R5 user retest still failed, with no application log and no More Info option.
`BOOT-CHECK.txt` and `R6-CHANGES.txt` describe the new two-title diagnostic setup.
R6 changes the container magic from the alternate 0x1D3D154F to the working
reference's 0xEEF51454 and makes that the native package gate requirement.
The renderer/game logic/asset requirement remain the existing DS USA port.

`controller_only.c` replaces private keyboard callbacks before linking static
SDL, avoiding hard requirements on the native keyboard/IME modules. Default
CMake EGG_PS5_SYSTEM_KEYBOARD=OFF matches native link without --system-keyboard.
Advanced builds can select ON and pass --system-keyboard together. Default
build.sh explicitly selects OFF. This disables native USB/system text keyboard
input; controller support remains. Unused sections are garbage-collected.

`boot_check.c` is an independent, no-C-runtime native entry point with six
kernel/SystemService imports. It never opens the ROM or calls SDL. Its used
entry-reference table also exercises the corrected data-before-GOT mapping.
`build_boot_check.py` creates the second home-screen folder with the same
reproduced loader companion. build.sh now builds both PPSA90435 and PPSA90436.
The probe is not the external .elf logger and does not need Payload Manager.

Tests: test_boot_check.py covers entry logging/notification behavior under host
shims, including unavailable file access and failed notifications. Existing
native packaging tests now recreate and reject the previous SELF magic, then
restore the correct profile. No tests here certify console execution.

The experimental metadata profile follows the supplied working release's
UP9000 prefix, attribute3 and downloadDataSize while retaining separate title
IDs. These are observed compatibility choices, not a proven cause or entitlement
guarantee. Before C initialization the logger now avoids mkdir/debug output,
uses raw kernel file APIs and offers a parent-free /data fallback. Keep logs
and the limitations in future builds.

## PE2 comparison follow-up (not included in delivered R6 binary)

The user supplied a second booting binary, pe2.zip/PPSA77023. Its SELF header,
metadata, companion runtime, normalized startup parameter blocks and shared
import providers match R6. Its runtime support instead uses the PsyZ/SDL_GPU
Vulkan path and different native filesystem/memory support. The static
comparison did not establish the console launch root cause.

Investigation found an independent SDL3 bool-return bug in the pinned PS5
backend. The updated sdl3-static.patch now corrects false-as-success error
returns in creation, presentation and audio opening, and reports exact video
API/status on setup failure. test_video_failure_paths.py exercises the actual
creation function under nine mocked boundary scenarios. The original function
fails the expected-error assertion; the corrected one passes. The SDL archive
and a private relink passed the native gates. This follow-up is prepared source,
not a silent replacement of the R6 download or a verified console fix.

## R8 direct-memory heap (14 November 2026)

Actual user logs now confirm both R6 and R7 reach native startup but return
`SDL_Init: Out of memory`, before any ROM loading. This does not identify the
failed allocation or prove that the console lacks physical RAM. R8 replaces
application allocator routing with the direct-memory heap from the exact public
Mihawk-99 PS5_PayloadSDK commit already pinned by ApricotGS. Sources, headers,
notices and hashes are retained under `compat/`; `HEAP-UPSTREAM.json` records
provenance. The platform files are GPL-3.0-or-later, and Doug Lea's unmodified
dlmalloc source is MIT-0.

Both link paths wrap malloc/calloc/realloc/free and the platform's documented
aligned, usable-size, array and line-reader helpers. Wrappers recognize their
own blocks and return foreign blocks to libc; they fall back to libc when
direct allocation is refused. The virtual range reserves address space only;
16 MiB arena segments and larger blocks acquire physical memory on demand.
Preflight runs before SDL logging/hints and checks malloc, preserved realloc
and cleared calloc data. Usage/fallback counts are logged before SDL and on
ordinary startup failures. R8 has since reached the game thread and audible music
on the user's PS5; its display corruption is addressed separately in R9.

`test_memory.py` runs the actual public heap and our preflight with the public
host kernel fixture, including 128 MiB allocate/release, cross-arena frees,
fallbacks and refused mappings. `test_heap_routes.py` checks that the four SDL
allocator adapters actually branch to the wrappers in the signed extracted
ELF, and rejects an altered branch target. These are host/artifact checks;
they do not certify PS5 memory APIs or gameplay.

## R9 scanout repair (14 November 2026)

The user's R8 log and photo now establish native startup, verified ROM input,
game-thread entry and audible music on PS5, with scrambled display output.
The software uploader's 512-pixel grouping truncated the row stride at
3840 and 1920 widths. R9 keeps the existing tile lookup's 128x128/64KiB
subtile mapping and uses the native number of tile columns/rows instead.
Both frame allocation and copying account for padded edge tiles. The source
pitch is explicit. Tiles are written sequentially using non-temporal SSE2
stores with fences before presentation. A failed worker launch joins existing
workers and runs the same upload routine synchronously.

`test_scanout.py` compiles the actual SDL uploader and checks every visible
and padded pixel against an independent native address oracle, guards and
non-packed source pitches at 4K, 1440p, 1080p and small partial-tile sizes,
including two worker-failure paths. The old uploader fails the 4K comparison.
These checks establish a concrete scanout bug and a host-verified correction;
R9 still needs a console retest. 4K/widescreen and the user-requested icon are
kept, and sustained 60fps is not yet established. The full SDL downstream
patch includes the header change; this work is not submitted upstream.

## CTPS5 0.1.0-rc1

The project and PS5 title are now CTPS5. It remains a downstream port of Egg of
Time 1.3.5, with upstream credits/licenses retained. The existing title ID and
data paths preserve the installed game's state. Native metadata version is
01.004.000; the release is a candidate, not a hardware-certified stable build.

Fixed-screen fit is configurable: Original, Matching panels (default),
Fill (crop) and Fill (stretch). Native-wide scenes retain their actual wider
view; fixed-size scenes preserve their picture within decorative matching
panels by default. Crop/stretch are explicit user choices and can remove UI
or alter proportions. The one-time `ctwide` profile enables 16:9 and room-edge
filling on upgrade. Later user choices are saved and respected; unrelated
settings are retained. Filling narrow rooms repeats existing tiles rather
than introducing new level geometry.

Runtime prompts use pixel-drawn DualSense symbols and Create/Options/L1/R1/
L2/R2 labels. Labels follow current bindings, published atomically for game
and host threads. The native Controls page shows the DualSense column and
accepts controller rebinding, without advertising an unavailable keyboard.
Original button icons embedded in the game's own graphics are not all
replaced; the controls guide maps those letters to the physical controller.
Normal boot notifications are quiet while file logging and failure messages
remain enabled.

Host tests compile actual label/font/prompt code, presentation geometry,
side-panel drawing and setting load/save/profile migration. Native ELF,
allocator routing and package gates pass. R8 reached native game execution
and music on the user's PS5; CTPS5 RC1 still needs console checks for these
new UI/presentation paths, save/load and extended gameplay. Sustained 4K60
has not been established.
