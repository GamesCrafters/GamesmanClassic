#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include "Game.h"
#include "memory.h"

void discoverfragment(char** inputfiles, char* outputfolder, gamehash minhash, char fragmentsize);

void solvefragment(char** inputfiles, char* solvedfragmentfolder, gamehash minhash, char fragmentsize);
