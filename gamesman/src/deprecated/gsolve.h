/************************************************************************
**
** NAME:         gsolve.h
**
** DESCRIPTION:  The necessary typedefs and global defs that solve.c needs.
**
** AUTHOR:       Dan Garcia  -  University of California at Berkeley
**               Copyright (C) Dan Garcia, 1995. All rights reserved.
**
** DATE:         07/15/91
**
** UPDATE HIST:
**
** 05-15-95 1.0   : Final release code for M.S.
**
**************************************************************************/

#define  kSolveVersion  "2.0 (1999-07-04)"

#define  MAXINT2          1073741823 /* 2^30 - 1 */
#define  MAXNAME          9          /* formats nice */
#define  MAXINPUTLENGTH   80         /* arbitrary */
#define  VISITED_MASK     4          /* ... 100 */
#define  VALUE_MASK       3          /* ... 011 */
#define  REMOTENESS_SHIFT 8          /* 1111111100000000 */
#define  REMOTENESS_MAX   255
#define  REMOTENESS_MASK  (REMOTENESS_MAX << REMOTENESS_SHIFT)

#define  kPlayerOneTurn 1 /* used for human/human games */
#define  kPlayerTwoTurn 0
#define  kHumansTurn    1 /* used for computer/human games */
#define  kComputersTurn 0 

#define  kBadMexValue  -1 /* -1 Can never be a valid mex value */

#ifndef  FALSE
#define  FALSE          0
#endif

#ifndef  TRUE
#define  TRUE           1
#endif

#define MIN_FUZZY_VALUE 0.0 /* used for fuzzy heuristic static evaluators */
#define TIE_FUZZY_VALUE 0.5
#define MAX_FUZZY_VALUE 1.0

typedef int POSITION;       /* This used to be determined by mTempTypedef.h */
typedef int MOVE;           /* but I decided to hardcode it for now, so as  */
typedef int MOVES;          /* to make it a leeeetle bit easier and cleaner */

typedef enum minimax_enum { /* used for the minimax procedure */
  minimizing, maximizing
  } MINIMAX;

typedef float FUZZY;        /* used to describe fuzzy values */

typedef int  REMOTENESS;
typedef char *STRING;
typedef char *GENERIC_PTR;

#ifndef BOOLEAN             /* To satisfy Visual C++ 6.0 compiler */
#ifdef WIN32
typedef unsigned char BOOLEAN;
#else /* WIN32 */
typedef int  BOOLEAN;
#endif /* WIN32 */
#endif /* BOOLEAN */

typedef enum value_enum {
	win, lose, tie, undecided, visited  /* visited = 4 */
} VALUE;

typedef enum menu_enum {
	BeforeEvaluation, Evaluated
} MENU;

typedef enum user_input_enum {
	Undo, Abort, Continue, Move
} USERINPUT;

typedef struct positionlist_item {
	POSITION position;
	struct positionlist_item *next;
} UNDO, POSITIONLIST;

typedef struct movelist_item {
	MOVE move;
	struct movelist_item *next;
} MOVELIST;

typedef signed char MEX;           /* The max mex value is 31 */
typedef unsigned long int MEXCALC; /* ...since we use a long int here */

extern VALUE   gValue;                  /* The value of the game */
extern MENU    gMenuMode;               /* The state of the main menu */
extern BOOLEAN gAgainstComputer;        /* TRUE iff the user is playing the computer */
extern BOOLEAN gHumanGoesFirst;         /* TRUE iff the user goes first vs. computer */
extern BOOLEAN gStandardGame;           /* TRUE iff game is STANDARD (not REVERSE) */
extern BOOLEAN gExhaustiveSearch;       /* TRUE iff game uses Exhaustive Search to solve */
extern BOOLEAN gSymmetries;             /* TRUE iff the symmetries should be searched */
extern BOOLEAN gPredictions;            /* TRUE iff the predictions should be printed */
extern BOOLEAN gPossibleMoves;          /* TRUE iff possible moves should be printed */
extern char    gPlayerName[2][MAXNAME]; /* The names of the players user/user or comp/user */
extern int     gLookAheadLevel;         /* The lookahead level for fuzzy heuristics */
extern int     gBytesMalloced;          /* The number of bytes SafeMalloc requested */
extern MEX    *gMexVals;                /* The array which stores the Mex vals */
extern POSITION  gHeadFR;               /* The FRontier Head position */
extern POSITION  gTailFR;               /* The FRontier Tail position */
extern POSITION *gPrevFR;               /* The FRontier Prev array */
extern POSITION *gNextFR;               /* The FRontier Next array */
extern POSITIONLIST **gParents;         /* The Parent of each node in a list */
