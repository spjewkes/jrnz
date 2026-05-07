#!/bin/bash
set -euo pipefail

build_dir="__build/debug"
mkdir -p "$build_dir"
cd "$build_dir"
cmake ../.. -DCMAKE_BUILD_TYPE=Debug
time -p cmake --build . --parallel 16
