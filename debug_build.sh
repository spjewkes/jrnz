#!/bin/bash
mkdir -p __build
cd __build
cmake .. -DCMAKE_BUILD_TYPE=Debug
time -p cmake --build . --parallel 16
