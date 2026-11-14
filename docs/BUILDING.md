# Building CTPS5 on Linux

This builds the Nintendo DS USA version from your own unmodified dump. No ROM or generated game code is downloaded by these tools.

## Dependencies

| Dependency | Version / source |
| --- | --- |
| Linux host tools | Git, curl, tar, C/C++ compiler with C++20 support, zlib development headers |
| Python | 3.11 or later |
| CMake / Ninja | CMake 3.25 or later, Ninja |
| LLVM development tools | LLVM 19.1.7; set LLVM_CONFIG to llvm-config-19 or its full path |
| Public PS5 SDK | [pacbrew-repo v0.41.1](https://github.com/ps5-payload-dev/pacbrew-repo/releases/tag/v0.41.1) |
| SDL | [OpenAGC/SDL3](https://github.com/OpenAGC/SDL3), commit 37ff89ab9233e49a39b362b2023cab64520d9cca |
| dsd | [AetiasHax/ds-decomp v0.12.1](https://github.com/AetiasHax/ds-decomp/releases/tag/v0.12.1) |

On Ubuntu, install the equivalents of `build-essential git curl cmake ninja-build python3 zlib1g-dev llvm-19-dev`. Install LLVM from your distribution or the official LLVM packages if necessary. Install the dsd release for your host and put its executable on PATH. Capstone is only needed for optional disassembly comments; this build uses `--no-comments`.

## Fetch pinned platform dependencies

```sh
bash tools/bootstrap-ps5-deps.sh
export PS5_PAYLOAD_SDK="$PWD/.deps/sdk/opt/ps5-payload-sdk"
export SDL3_SOURCE="$PWD/.deps/SDL3"
export LLVM_CONFIG=llvm-config-19
export DSD=dsd
```

The helper verifies the SDK archive SHA-256 and checks out the pinned SDL commit in an isolated local folder. `ps5/build.sh` applies the included downstream SDL patch. Do not point SDL3_SOURCE at a checkout containing unrelated changes.

## Recompile and package

```sh
bash ps5/build.sh /absolute/path/to/CT.nds
```

The input must have game code `YQUE` and SHA-1 `11f9e8204e484eab42181b210c9f214584be75bf`. The script validates the hash, extracts with dsd, generates C locally, builds SDL and the runtime, and writes a native application folder:

```text
build-ps5/package/PPSA90435/
  eboot.bin
  sce_sys/
  sce_module/
```

It also builds the optional independent boot-check folder PPSA90436. These generated executables contain locally derived code; do not upload them to the public source repository. Set `EGG_JOBS=2` to limit parallel compiler memory use. The build directory can be changed with `EGG_BUILD_DIR`.

The application applies its built-in PS5 game prompts to the verified NDS in memory at startup; no separate controller texture pack is required. Continue with [installation](INSTALLING.md). First builds are substantial and require free disk space and RAM. This is a developer build process, not a one-click PS5 installer.

## ROM-free checks

```sh
python3 tools/check_public_source.py
python3 tools/check_site.py
python3 ps5/test_diagnostics.py
python3 ps5/test_memory.py
python3 ps5/test_presentation.py
python3 ps5/test_prompts.py --sdl-source "$SDL3_SOURCE"
python3 ps5/test_game_prompts.py --sdl-source "$SDL3_SOURCE"
python3 ps5/test_settings_migration.py --sdl-source "$SDL3_SOURCE"
```

Scanout and video failure tests additionally need the patched SDL checkout; see `.github/workflows/checks.yml`. These checks do not establish full game compatibility or 4K60 performance. See [release notes](RELEASES.md).
