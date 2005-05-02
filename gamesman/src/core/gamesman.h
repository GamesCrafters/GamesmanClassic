#ifndef GMCORE_GAMESMAN_H
#define GMCORE_GAMESMAN_H

#ifndef NO_GRAPHICS
#include "tcl.h"
#include "tk.h"
#else	/* Dummy types */
typedef int Tcl_Interp;
typedef int Tk_Window;
typedef int ClientData;
#define TCL_OK		0
#define	TCL_ERROR	1
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/utsname.h>
#include <sys/time.h> 
#include <limits.h>




#define  MAXINT2          1073741823 /* 2^30 - 1 */
#define  MAXNAME          15         /* arbitrary */
#define  MAXINPUTLENGTH   80         /* arbitrary */
#define  VISITED_MASK     4          /* ... 100 */
#define  VALUE_MASK       3          /* ... 011 */
#define  REMOTENESS_SHIFT 8          /* 1111111100000000 */
#define  REMOTENESS_MAX     255      /* We encode draws as this */
#define  REMOTENESS_TWOBITS 254      /* we encode TwoBits as this */
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

typedef unsigned long long POSITION;      /* This used to be determined by mTempTypedef.h */
typedef int MOVE;           /* but I decided to hardcode it for now, so as  */
typedef int MOVES;          /* to make it a leeeetle bit easier and cleaner */
#define POSITION_FORMAT "%llu"

typedef int   REMOTENESS;
typedef char* STRING;
typedef void* GENERIC_PTR;

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

/*
** Data structures
*/

typedef enum value_enum
{
	win, lose, tie, undecided, visited  /* visited = 4 */
}
VALUE;

typedef enum menu_enum
{
  BeforeEvaluation, Evaluated, Analysis
}
MENU;

typedef enum user_input_enum
{
	Undo, Abort, Continue, Move
}
USERINPUT;

typedef enum static_func_msg_enum
{
	Init, Clean, Save, Update, Poll
}
STATICMESSAGE;

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

typedef signed char		MEX;     /* The max mex value is 31 */
typedef unsigned long int	MEXCALC; /* ...since we use a long int here */

#include "setup.h"
#include "analysis.h"
#include "constants.h"
#include "globals.h"
#include "debug.h"
#include "gameplay.h"
#include "misc.h"
#include "db.h"	// Should be temporary
#include "textui.h"
#include "filedb.h"
#include "main.h"
#include "solveloopyup.h"

#endif /* GMCORE_GAMESMAN_H */
