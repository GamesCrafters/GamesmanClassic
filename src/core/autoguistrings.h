#ifndef GMCORE_AUTOGUI_STRINGS_H
#define GMCORE_AUTOGUI_STRINGS_H

#include "types.h"
#include <string.h>
#include <stdio.h>

#define MAX_POSITION_STRING_LENGTH 128
#define MAX_MOVE_BUTTON_STRING_LENGTH 32

BOOLEAN ParseAutoGUIFormattedPositionString(char *positionString, int *turn, char **entityString);

void AutoGUIMakePositionString(int turn, char *entityString, char *buf);

void AutoGUIMakeMoveButtonStringA(char token, unsigned int at, char sound, char *buf);
void AutoGUIMakeMoveButtonStringT(char *text, unsigned int at, char sound, char *buf);
void AutoGUIMakeMoveButtonStringM(unsigned int from, unsigned int to, char sound, char *buf);
void AutoGUIMakeMoveButtonStringL(unsigned int p1, unsigned int p2, char sound, char *buf);

#endif // GMCORE_AUTOGUI_STRINGS_H