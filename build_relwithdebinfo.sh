#!/bin/bash
set -euo pipefail

build_dir="__build/relwithdebinfo"
mkdir -p "$build_dir"
cd "$build_dir"
cmake ../.. -DCMAKE_BUILD_TYPE=RelWithDebInfo
time -p cmake --build . --parallel 16
