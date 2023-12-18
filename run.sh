#!/bin/sh

mkdir -p build
cd build
cmake ..
cmake --build .
cd ../assets
../build/isgt
