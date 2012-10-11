#ifndef GMCORE_INTERACT_H
#define GMCORE_INTERACT_H
#include "types.h"
#include "constants.h"
#include "misc.h"
#include "openPositions.h"
#include <string.h>
#include <stdlib.h>


BOOLEAN InteractReadPosition(STRING input, POSITION * result);
void InteractCheckErrantExtra(STRING input, int max_words);
void ServerInteractLoop(void);
extern POSITION gInitialPosition;

#endif /* GMCORE_INTERACT_H */
