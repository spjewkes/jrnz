#!/bin/zsh

cd __build/ || exit 1
time ctest --output-on-failure
