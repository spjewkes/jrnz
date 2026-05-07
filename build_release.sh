#!/bin/bash
set -euo pipefail

build_dir="__build/release"
mkdir -p "$build_dir"
cd "$build_dir"
cmake ../.. -DCMAKE_BUILD_TYPE=Release
time -p cmake --build . --parallel 16
