#!/bin/bash

echo "Starting compilation."
mkdir -p build
mpicc -c -funsigned-char solver.c -o build/solver.o
mpicc -c -funsigned-char Connect4.c -o build/Game.o
mpicc -c -funsigned-char memoryfastretrieval.c -o build/memory.o
mpicc -c -funsigned-char maindrivermpi.c -o build/maindriver.o
mpicc -o build/connect4mpi.exe build/maindriver.o build/solver.o build/Game.o build/memory.o
echo "Compilation complete."
