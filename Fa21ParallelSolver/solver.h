#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include "Game.h"
#include "memory.h"

void discoverstartingfragment(char* workingfolder, char fragmentsize);

void discoverfragment(char* workingfolder, gamehash minhash, char fragmentsize);

void solvefragment(char* workingfolder, gamehash minhash, char fragmentsize);
