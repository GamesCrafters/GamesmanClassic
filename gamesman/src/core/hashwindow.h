#ifndef GMCORE_HASHWINDOW_H
#define GMCORE_HASHWINDOW_H

#include "gamesman.h"

void gUnhashToTierPosition(POSITION, TIERPOSITION*, TIER*);
POSITION gHashToWindowPosition(TIERPOSITION, TIER);
void gInitializeHashWindow(TIER, BOOLEAN);
void gInitializeHashWindowToPosition(POSITION*);

#endif /* GMCORE_HASHWINDOW_H */
