#!/bin/bash

echo "Starting compilation."
mkdir -p build
gcc -c -funsigned-char solver.c -o build/solver.o
gcc -c -funsigned-char Connect4.c -o build/Game.o
gcc -c -funsigned-char memoryfastretrieval.c -o build/memory.o
gcc -c -funsigned-char maindriversinglethreaded.c -o build/maindriver.o
gcc -o build/connect4singlethreaded.exe build/maindriver.o build/solver.o build/Game.o build/memory.o
echo "Compilation complete."
