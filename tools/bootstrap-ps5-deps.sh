#!/usr/bin/env bash
set -euo pipefail
root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
deps="$root/.deps"
mkdir -p "$deps/sdk"
archive="$deps/ps5-payload-dev-v0.41.1.tar.gz"
if [[ ! -f "$archive" ]]; then
 curl --fail --location --retry 3 'https://github.com/ps5-payload-dev/pacbrew-repo/releases/download/v0.41.1/ps5-payload-dev.tar.gz' --output "$archive.part"
 mv -- "$archive.part" "$archive"
fi
printf '%s  %s\n' 'e811bc2e4676c5bb7f80a8fe928d2985dce599996a5ca6b54da3e72699287887' "$archive" | sha256sum --check --status
tar -xzf "$archive" -C "$deps/sdk"
if [[ ! -d "$deps/SDL3/.git" ]]; then
 git clone --no-checkout https://github.com/OpenAGC/SDL3.git "$deps/SDL3"
 git -C "$deps/SDL3" checkout --detach 37ff89ab9233e49a39b362b2023cab64520d9cca
fi
if [[ -n "$(git -C "$deps/SDL3" status --porcelain)" ]]; then
 echo 'SDL3 has local changes (possibly the downstream patch). Keeping existing checkout.'
 test "$(git -C "$deps/SDL3" rev-parse HEAD)" = '37ff89ab9233e49a39b362b2023cab64520d9cca'
else
 git -C "$deps/SDL3" checkout --detach 37ff89ab9233e49a39b362b2023cab64520d9cca
fi
printf 'Dependencies ready. Install dsd v0.12.1 and LLVM 19, then follow docs/BUILDING.md.\n'
