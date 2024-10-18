#ifndef GMCORE_HASHWINDOW_H
#define GMCORE_HASHWINDOW_H

#include "gamesman.h"

void gUnhashToTierPosition(POSITION, TIERPOSITION*, TIER*);
POSITION gHashToWindowPosition(TIERPOSITION, TIER);
void gInitializeHashWindow(TIER, BOOLEAN);
void gInitializeHashWindowToPosition(POSITION*, BOOLEAN loadDB);
BOOLEAN gTierDBExistsForPosition(POSITION);

#endif /* GMCORE_HASHWINDOW_H */
