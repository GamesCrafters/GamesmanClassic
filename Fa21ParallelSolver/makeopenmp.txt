#!/bin/bash

echo "Starting compilation."
mkdir -p build
gcc -c -funsigned-char -fopenmp solver.c -o build/solver.o
gcc -c -funsigned-char -fopenmp Connect4.c -o build/Game.o
gcc -c -funsigned-char -fopenmp memoryfastretrieval.c -o build/memory.o
gcc -c -funsigned-char -fopenmp maindriveropenmp.c -o build/maindriver.o
gcc -fopenmp -o build/connect4openmp.exe build/maindriver.o build/solver.o build/Game.o build/memory.o
echo "Compilation complete."
