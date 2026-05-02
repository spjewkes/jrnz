#!/bin/zsh

cd __build/ || exit 1
ctest --output-on-failure
