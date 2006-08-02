#ifndef GMCORE_HASHWINDOW_H
#define GMCORE_HASHWINDOW_H

#include "gamesman.h"

void gUnhashToTierPosition(POSITION, TIERPOSITION*, TIER*);
POSITION gHashToWindowPosition(TIERPOSITION, TIER);
void gInitializeHashWindow(TIER);

extern BOOLEAN			gHashWindowInitialized;
extern BOOLEAN			gCurrentTierIsLoopy;
extern TIER*			gTierInHashWindow;
extern TIERPOSITION*	gMaxPosOffset;
extern int				gNumTiersInHashWindow;

#endif /* GMCORE_HASHWINDOW_H */

