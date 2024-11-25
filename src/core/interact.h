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

STRING InteractReadPosition(STRING input, POSITION * result);
STRING InteractReadLong(STRING input, long * result);
STRING InteractReadBoardString(STRING input, char ** result);
STRING InteractValueCharToValueString(char value_char);
void InteractPrintJSONPositionValue(VALUE value);
void InteractCheckErrantExtra(STRING input, int max_words);
void ServerInteractLoop(void);
extern POSITION gInitialPosition;

typedef BOOLEAN (* get_value_func_t)( char *, void *);

#define GetValue(board_string, key, func, target) \
	(0 ? func("", target) : GetValueInner(board_string, key, (get_value_func_t) func, (void *) target))

BOOLEAN GetValueInner(char * board_string, char * key, get_value_func_t func, void * target);

BOOLEAN GetInt(char* value, int* placeholder);
BOOLEAN GetUnsignedLongLong(char* value, unsigned long long* placeholder);
BOOLEAN GetChar(char* value, char* placeholder);

#endif /* GMCORE_INTERACT_H */
