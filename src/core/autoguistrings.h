#ifndef GMCORE_AUTOGUI_STRINGS_H
#define GMCORE_AUTOGUI_STRINGS_H

#include "types.h"
#include <string.h>
#include <stdio.h>

#define MAX_POSITION_STRING_LENGTH 128
#define MAX_MOVE_STRING_LENGTH 32

/*
    `positionString` is assumed to be in AutoGUI format

    `turn` shall be set to 1 or 2
    `entityString` shall be 
*/
BOOLEAN ParseAutoGUIFormattedPositionString(char *positionString, int *turn, char **entityString);

void AutoGUIMakePositionString(int turn, char *entityString, char *buf);
/* 
"Autogui Format":
<turn>_<entity string>
turn = 1 or 2
entity string = xoo-xooxx

2_-oox-oxox

*/

void AutoGUIMakeMoveButtonStringA(char token, unsigned int at, char sound, char *buf); /* A_x_30 (point (single) move button) */
/*
   A_-_<center> (circle)
   A_<char>_<center> (image)

*/

void AutoGUIMakeMoveButtonStringT(char *text, unsigned int at, char sound, char *buf); /* T_<string>_30 (text point (single coord) move button)*/ 
void AutoGUIMakeMoveButtonStringM(unsigned int from, unsigned int to, char sound, char *buf); /* M_2_5 (arrow (double coord) move button) */
void AutoGUIMakeMoveButtonStringL(unsigned int p1, unsigned int p2, char sound, char *buf); /* L_2_5 (line (double coord) move button) */

void AutoGUIWriteEmptyString(char *buf);

#endif // GMCORE_AUTOGUI_STRINGS_H