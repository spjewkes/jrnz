#!/bin/zsh

cd __build/ || exit 1
time -p ctest --output-on-failure
