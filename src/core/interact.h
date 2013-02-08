#ifndef GMCORE_INTERACT_H
#define GMCORE_INTERACT_H
#include "types.h"
#include "constants.h"
#include "misc.h"
#include "openPositions.h"
#include "globals.h"
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


BOOLEAN InteractReadPosition(STRING input, POSITION * result);
BOOLEAN InteractReadBoardString(STRING input, STRING * result);
STRING InteractValueCharToValueString(char value_char);
void InteractPrintJSONPositionValue(POSITION pos);
void InteractFreeBoardSting(STRING board);
void InteractCheckErrantExtra(STRING input, int max_words);
void ServerInteractLoop(void);
extern POSITION gInitialPosition;
int StringLookup(char* string, ...);
char * MakeBoardString(char * first, ...);
char * StringDup( char * s );

#endif /* GMCORE_INTERACT_H */
