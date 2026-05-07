#!/bin/zsh

set -euo pipefail

config="${1:-debug}"
case "$config" in
  debug|release|relwithdebinfo)
    shift || true
    ;;
  *)
    config="debug"
    ;;
esac

cd "__build/$config" || exit 1
time ctest --output-on-failure "$@"
