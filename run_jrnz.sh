#!/bin/bash
set -euo pipefail

config="${1:-release}"
case "$config" in
  debug|release|relwithdebinfo)
    shift || true
    ;;
  *)
    config="release"
    ;;
esac

time "__build/$config/run_jrnz" "$@"
