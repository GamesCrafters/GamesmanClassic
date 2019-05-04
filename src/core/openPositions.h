/*
 * Experimental Open Positions code by David Eitan Poll
 * Exploring re-evaluation of Draw Positions
 */
#ifndef OPEN_POSITIONS

#define OPEN_POSITIONS

#include <inttypes.h>
#include <stdio.h>
#include "types.h"
#include "db.h"
#include "gamesman.h"
#include "misc.h"

/*
 * use 32 bits for storage of openPos info as follows:
 * Starting from LSB=bit 0
 *
 * bit 0..1   : openPositions value (win, loss, undecided)
 * bit 2      : fringe bit
 * bit 3..11  : corruption level
 * bit 12..21 : level number
 * bit 22..31 : fringe remoteness (fremoteness)
 */
#define OPEN_POS_DATA unsigned int


/* constants */
/*#define DRAW_UNDEC  0
  #define DRAW_WIN    1
  #define DRAW_LOSE   2
  #define PARITY_EVEN 0
#define PARITY_ODD  1*/

/*enum open_pos_value_enum { draw_undecided, draw_win, draw_lose };*/

/* masks */
/*#define DV_MASK 0x3
   #define IV_MASK 0xC
   #define C_MASK  0x10
   #define CP_MASK 0x20
   #define DR_MASK 0x00007FC0
   #define IR_MASK 0x00FF8000
 #define OL_MASK 0xFF000000*/

#define DV_MASK 0x3
#define FB_MASK 0x4
#define CL_MASK 0x00000FF8
#define LN_MASK 0x003FF000
#define FR_MASK 0xFFC00000

/* bit shift amounts (based on first bit position */
/*#define DV_SHIFT 0
   #define IV_SHIFT 2
   #define C_SHIFT  4
   #define CP_SHIFT 5
   #define DR_SHIFT 6
   #define IR_SHIFT 15
 #define OL_SHIFT 24*/

#define DV_SHIFT 0
#define FB_SHIFT 2
#define CL_SHIFT 3
#define LN_SHIFT 12
#define FR_SHIFT 22

#define CORRUPTION_MAX ((0x1<<9)-1)
#define FREMOTENESS_MAX ((0x1<<10)-1)

/* macros */
/*#define drawValue(entry)          (((entry)&DV_MASK)>>DV_SHIFT)
   #define infiniteValue(entry)      (((entry)&IV_MASK)>>IV_SHIFT)
   #define isCorrupted(entry)        (((entry)&C_MASK)>>C_SHIFT)
   #define corruptedParity(entry)    (((entry)&CP_MASK)>>CP_SHIFT)
   #define drawRemoteness(entry)     (((entry)&DR_MASK)>>DR_SHIFT)
   #define infiniteRemoteness(entry) (((entry)&IR_MASK)>>IR_SHIFT)
   #define openPosLevel(entry)       (((entry)&OL_MASK)>>OL_SHIFT)

   #define setDrawValue(entry, newval)          (((entry)|DV_MASK)&(((newval)<<DV_SHIFT)|(~DV_MASK)))
   #define setInfiniteValue(entry, newval)      (((entry)|IV_MASK)&(((newval)<<IV_SHIFT)|(~IV_MASK)))
   #define setCorrupted(entry, newval)          (((entry)|C_MASK)&(((newval)<<C_SHIFT)|(~C_MASK)))
   #define setCorruptedParity(entry, newval)    (((entry)|CP_MASK)&(((newval)<<CP_SHIFT)|(~CP_MASK)))
   #define setDrawRemoteness(entry, newval)     (((entry)|DR_MASK)&(((newval)<<DR_SHIFT)|(~DR_MASK)))
   #define setInfiniteRemoteness(entry, newval) (((entry)|IR_MASK)&(((newval)<<IR_SHIFT)|(~IR_MASK)))
 #define setOpenPosLevel(entry, newval)       (((entry)|OL_MASK)&(((newval)<<OL_SHIFT)|(~OL_MASK)))*/

#define GetDrawValue(entry)          (((entry)&DV_MASK)>>DV_SHIFT)
#define GetCorruptionLevel(entry)    (((entry)&CL_MASK)>>CL_SHIFT)
#define GetLevelNumber(entry)        (((entry)&LN_MASK)>>LN_SHIFT)
#define GetFremoteness(entry)        (((entry)&FR_MASK)>>FR_SHIFT)
#define GetFringe(entry)             (((entry)&FB_MASK)>>FB_SHIFT)

#define SetDrawValue(entry, newval)          (((entry)|DV_MASK)&(((newval)<<DV_SHIFT)|(~DV_MASK)))
#define SetCorruptionLevel(entry, newval)    (((entry)|CL_MASK)&(((newval)<<CL_SHIFT)|(~CL_MASK)))
#define SetLevelNumber(entry, newval)        (((entry)|LN_MASK)&(((newval)<<LN_SHIFT)|(~LN_MASK)))
#define SetFremoteness(entry, newval)        (((entry)|FR_MASK)&(((newval)<<FR_SHIFT)|(~FR_MASK)))
#define SetFringe(entry, newval)             (((entry)|FB_MASK)&(((newval)<<FB_SHIFT)|(~FB_MASK)))

#define OPEN_FILE_VER 1         /* Open positions DB file version */
/* headers */

/* These are the core library functions for use elsewhere */

/* call before doing any other operations.  This is safe to call more than once, although
   it will wipe out any progress made if you do */
void InitializeOpenPositions(int numPossiblePositions);

/* call when you want to free up the massive amounts of memory this analysis will do.
   make sure you're done with all openPositions data first! */
void CleanupOpenPositions(void);
void FreeOpenPositions(void);

/* do the actual work! */
void ComputeOpenPositions();

/* call this with each draw position before calling computeOpenPositions.  This keeps us from having
   search for draw positions before we can do work */
void RegisterDrawPosition(POSITION pos);

/* call to set the open positions value of a position.  Right now, this just makes an assignment to an array,
   but in the future this might access the value in the values database */
void SetOpenData(POSITION pos, OPEN_POS_DATA value);

/* call to get the open positions value of a position.  Right now, this just accesses an array,
   but in the future this might access the value in the values database */
OPEN_POS_DATA GetOpenData(POSITION pos);

/* call to print the drawvalues of positions.  This won't be useful for long, but it's great for debugging
   and getting a point across */
void PrintOpenDataFormatted(void);
void PrintOpenAnalysis(void);
void PrintSingleOpenData(POSITION p);

/* this is to allow the computer to choose where to go using openPositions */
MOVE ChooseSmartComputerMove(POSITION from, MOVELIST * moves, REMOTENESSLIST * remotenesses);

/* Open positions DB save/load functions */
BOOLEAN SaveOpenPositionsData();
BOOLEAN LoadOpenPositionsData();

/* These are the "private" functions for openPositions' consumption only */
void EnqueueDP(POSITION pos);
POSITION DequeueDP(void);
void CleanUpBeneathCL(int cl);
void PropogateFreAndCorUp(POSITION p);
void PropogateFreAndCorUpFringe(POSITION p, char fringe);
int OpenIsInitialized(void);
OPEN_POS_DATA DetermineFreAndCorDown1LevelForWin(POSITION p);


/* globals */
extern OPEN_POS_DATA* openPosData;
extern POSITION openPosArrLen;
extern BOOLEAN gOpenDataLoaded;

/* Globals relative to purity */
extern VALUE*              gPositionValue;
extern REMOTENESS*         gPositionLevel;
extern POSITIONLIST**      gDrawParents;

/* Functions relative to purity */
BOOLEAN RunPurityCheck();
//void SetDrawParents(POSITION parent, POSITION root);
//void DrawParentInitialize();

/** PURITY */
BOOLEAN           DeterminePure             (POSITION position);

//void      InitializeVisitedArray     (void);
//void      FreeVisitedArray     (void);

void            DrawNumberChildrenInitialize        (void);
void            DrawNumberChildrenFree              (void);
void            InitializeDR                    (void);

POSITION        DeQueueWinDR                    (void);
POSITION        DeQueueLoseDR                   (void);
POSITION        DeQueueTieDR                    (void);
void            InsertWinDR                     (POSITION pos);
void            InsertLoseDR                    (POSITION pos);
void            InsertTieDR                     (POSITION pos);
void            SetNewLevelFringe               (void);
BOOLEAN         ExistsUnsolvedPosition          (void);
BOOLEAN         isPure                          (void);
void            SetAnalysisOfDrawPositions      (void);
void            PrintDrawAnalysis               (void);

/* Loopy globals */

extern FRnode*          gHeadWinDR;
extern FRnode*          gTailWinDR;
extern FRnode*          gHeadLoseDR;
extern FRnode*          gTailLoseDR;
extern FRnode*          gHeadTieDR;
extern FRnode*          gTailTieDR;

extern POSITIONLIST**   gDrawParents;
extern char*            gDrawNumberChildren;
extern POSITION         gNumLoses[101]; // Notes explaining the 101 are in openPositions.c
extern POSITION         gNumWins[101];

#endif
