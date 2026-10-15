#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
cc -std=gnu11 -g -O1 -Wall -Wextra -Werror -DSLISK_TEST -fsanitize=address,undefined -fno-omit-frame-pointer \
   -Itests/fakes -Isrc src/xw_core.c src/slisk_usb.c tests/test_xbox_direct.c -ldl \
   -o /tmp/slisk-test-xbox
ASAN_OPTIONS=detect_leaks=1:abort_on_error=1 /tmp/slisk-test-xbox

# Compile the actual PS4 payload entrypoint against host stubs to catch
# syntax and ABI declaration errors without running privileged calls.
cc -std=gnu11 -Wall -Wextra -Werror -fsyntax-only -Itests/fakes -Isrc src/main.c
