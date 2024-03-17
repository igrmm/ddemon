#!/bin/sh

mkdir -p build
cd build

if [ "$1" = "valgrind" ]; then
    cmake -DCMAKE_BUILD_TYPE=Debug ..
    cmake --build .
    cd ../assets
    valgrind \
        --suppressions=../valgrind.sup \
        --leak-check=full \
        --show-reachable=yes \
        --show-leak-kinds=all \
        --error-limit=no \
        --gen-suppressions=all \
        --track-origins=yes \
        --keep-debuginfo=yes \
        --log-file=../build/supdata.log \
        ../build/ddemon
    exit
fi

if [ "$1" = "debug" ]; then
    export SANITIZERS="-fsanitize=address,undefined"
    cmake -DCMAKE_BUILD_TYPE=Debug ..
else
    cmake ..
fi
cmake --build .
cd ../assets
../build/ddemon
