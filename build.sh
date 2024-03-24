#!/bin/bash
mkdir -p __build
cd __build
cmake ..
time -p cmake --build . --parallel 16
