#!/usr/bin/env sh
set -eu
cd "$(dirname "$0")/../.."
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT HUP INT TERM
"${CC:-cc}" -std=c11 -Wall -Wextra -Werror -O1 -g \
    -Itests/fullpng/stub -Iruntime \
    runtime/fullpng.c tests/fullpng/test_fullpng.c -o "$tmp/test_fullpng"
"$tmp/test_fullpng"
