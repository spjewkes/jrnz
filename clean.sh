#!/bin/bash
set -euo pipefail

find . -type f -name "*~" -print -exec rm -f {} \;

for build_dir in __build/debug __build/release __build/relwithdebinfo; do
  if [ -d "$build_dir" ]; then
    cmake --build "$build_dir" --target clean
  fi
done
