#!/bin/sh

mkdir -p build/wobu
cd build/wobu

if [ "$1" = "valgrind" ]; then
    cmake -DCMAKE_BUILD_TYPE=Debug ../../src/wobu
    cmake --build .
    valgrind \
        --suppressions=../../valgrind.sup \
        --leak-check=full \
        --show-reachable=yes \
        --show-leak-kinds=all \
        --error-limit=no \
        --gen-suppressions=all \
        --track-origins=yes \
        --keep-debuginfo=yes \
        --log-file=supdata.log \
        ./wobu
    exit
fi

if [ "$1" = "debug" ]; then
    export SANITIZERS="-fsanitize=address,undefined"
    cmake -DCMAKE_BUILD_TYPE=Debug ../../src/wobu
else
    cmake ../../src/wobu
fi
cmake --build .
./wobu
