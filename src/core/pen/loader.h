// functions used by game parsers

#ifndef LOADER_H
#define LOADER_H

#include "stroke.h"
#include <stdio.h>

extern int startPenLoader(char *filename, int debug);
extern BOOL isPenLoaderStarted();
extern BOOL hasPenLoaderFinished();
extern Stroke getNextAvailableStroke();
extern void startNewPenOutputFile();
extern void printLog(char *formatString, ...);

#endif //LOADER_H
