#ifndef __GAMESMAN_H__
#define __GAMESMAN_H__

#include<math.h>

#define  MAXINT2          1073741823 /* 2^30 - 1 */
#define  MAXNAME          15         /* arbitrary */
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

typedef int POSITION;       /* This used to be determined by mTempTypedef.h */
typedef int MOVE;           /* but I decided to hardcode it for now, so as  */
typedef int MOVES;          /* to make it a leeeetle bit easier and cleaner */

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

/* smarter computer */
#define WINMOVE 0
#define TIEMOVE 1
#define LOSEMOVE 2
#define SMART 0
#define RANDOM 1
#define DUMB 2
#define MAXSCALE 100
#define MAXGIVEBACKS 9

typedef enum value_enum
{
	win, lose, tie, undecided, visited  /* visited = 4 */
}
VALUE;

typedef enum menu_enum
{
	BeforeEvaluation, Evaluated
}
MENU;

typedef enum user_input_enum
{
	Undo, Abort, Continue, Move
}
USERINPUT;

typedef struct positionlist_item
{
	POSITION position;
	struct positionlist_item *next;
}
POSITIONLIST, FRnode;

typedef struct undolist_item
{
        POSITION position;
        BOOLEAN givebackUsed;
        struct undolist_item *next;
}
UNDO;

typedef struct booleanlist_item
{
        BOOLEAN value;
        struct booleanlist_item *next;
}
BOOLEANLIST;

typedef struct movelist_item
{
	MOVE move;
	struct movelist_item *next;
}
MOVELIST;

typedef struct remotenesslist_item
{
        REMOTENESS remoteness;
        struct remotenesslist_item *next;
}
REMOTENESSLIST;

typedef struct valuemoves_item
{
        MOVELIST* moveList[3];
        REMOTENESSLIST* remotenessList[3];
}
VALUE_MOVES;

typedef signed char MEX;           /* The max mex value is 31 */
typedef unsigned long int MEXCALC; /* ...since we use a long int here */

extern VALUE   gValue;                  /* The value of the game */
extern MENU    gMenuMode;               /* The state of the main menu */
extern BOOLEAN gAgainstComputer;        /* TRUE iff the user is playing the computer */
extern BOOLEAN gHumanGoesFirst;         /* TRUE iff the user goes first vs. computer */
extern BOOLEAN gStandardGame;           /* TRUE iff game is STANDARD (not REVERSE) */
extern BOOLEAN gPrintPredictions;            /* TRUE iff the predictions should be printed */
extern BOOLEAN gPrintPossibleMoves;          /* TRUE iff possible moves should be printed */
extern char    gPlayerName[2][MAXNAME]; /* The names of the players user/user or comp/user */

// Start for loopy games
extern FRnode *gHeadWinFR;               /* The FRontier Win Queue */
extern FRnode *gTailWinFR;
extern FRnode *gHeadLoseFR;              /* The FRontier Lose Queue */
extern FRnode *gTailLoseFR;
extern POSITIONLIST **gParents;         /* The Parent of each node in a list */
// End for loopy games

#include "tcl.h"
#include "tk.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/utsname.h>
#include <sys/time.h>   

#endif
