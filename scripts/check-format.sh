#!/usr/bin/env bash
set -euo pipefail

clang_format=$(which clang-format 2>/dev/null || echo /opt/homebrew/opt/llvm/bin/clang-format)

exit_code=0

for file in *.c *.h; do
  [ -f "$file" ] || continue

  if diff -u "$file" <("$clang_format" "$file") > /dev/null 2>&1; then
    echo "OK:   $file"
  else
    echo "FAIL: $file"

    set +e
    diff -u "$file" <("$clang_format" "$file")
    set -e

    exit_code=1
  fi
done

exit "$exit_code"
