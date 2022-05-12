#!/bin/bash

echo "Starting compilation."
mkdir -p build
gcc -c -funsigned-char Connect4.c -o build/Game.o
gcc -c -funsigned-char memoryfastretrieval.c -o build/memory.o
gcc -c -funsigned-char solversinglethreaded.c -o build/solver.o
gcc -o build/connect4memtest.exe build/solver.o build/Game.o build/memory.o
echo "Compilation complete."
