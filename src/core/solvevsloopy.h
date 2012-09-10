
#ifndef GMCORE_SOLVEVSLOOPY_H
#define GMCORE_SOLVEVSLOOPY_H

VALUE           VSDetermineLoopyValue           (POSITION position);

//void		InitializeVisitedArray		(void);
//void		FreeVisitedArray		(void);

void            VSMyPrintParents                        (void);

void            VSNumberChildrenInitialize      (void);
void            VSNumberChildrenFree            (void);
void            VSInitializeFR                  (void);

POSITION        VSDeQueueWinFR                  (void);
POSITION        VSDeQueueLoseFR                 (void);
POSITION        VSDeQueueTieFR                  (void);
void            VSInsertWinFR                   (POSITION pos);
void            VSInsertLoseFR                  (POSITION pos);
void            VSInsertTieFR                   (POSITION pos);

/* Loopy globals */

extern FRnode*          gVSHeadWinFR;
extern FRnode*          gVSTailWinFR;
extern FRnode*          gVSHeadLoseFR;
extern FRnode*          gVSTailLoseFR;
extern FRnode*          gVSHeadTieFR;
extern FRnode*          gVSTailTieFR;

extern POSITIONLIST**   gVSParents;
extern char*            gVSNumberChildren;

#endif /* GMCORE_SOLVEVSLOOPY_H */
