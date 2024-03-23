#!/bin/bash
find . -type f -name "*~" -print -exec rm -f {} \;
cd __build
cmake --build . --target clean
