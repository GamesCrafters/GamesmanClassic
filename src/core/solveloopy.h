
#ifndef GMCORE_SOLVELOOPY_H
#define GMCORE_SOLVELOOPY_H

VALUE           DetermineLoopyValue             (POSITION position);

//void		InitializeVisitedArray		(void);
//void		FreeVisitedArray		(void);

void            MyPrintParents                  (void);

void            NumberChildrenInitialize        (void);
void            NumberChildrenFree              (void);
void            InitializeFR                    (void);

POSITION        DeQueueWinFR                    (void);
POSITION        DeQueueLoseFR                   (void);
POSITION        DeQueueTieFR                    (void);
void            InsertWinFR                     (POSITION pos);
void            InsertLoseFR                    (POSITION pos);
void            InsertTieFR                     (POSITION pos);

/* Loopy globals */

extern FRnode*          gHeadWinFR;
extern FRnode*          gTailWinFR;
extern FRnode*          gHeadLoseFR;
extern FRnode*          gTailLoseFR;
extern FRnode*          gHeadTieFR;
extern FRnode*          gTailTieFR;

extern POSITIONLIST**   gParents;
extern char*            gNumberChildren;

#endif /* GMCORE_SOLVELOOPY_H */
