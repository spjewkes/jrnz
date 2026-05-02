#!/bin/zsh

cd __build/ || exit 1
ctest --progress --output-on-failure
