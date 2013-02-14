#ifndef GMCORE_TYPES_H
#define GMCORE_TYPES_H

/* type definitions for all of gamesman */

#ifndef NO_GRAPHICS
#include "tcl.h"
#include "tk.h"
#else   /* Dummy types */
typedef int Tcl_Interp;
typedef int Tk_Window;
typedef int ClientData;
#define TCL_OK          0
#define TCL_ERROR       1
#endif

typedef signed char MEX;                 /* The max mex value is 31 */
typedef int WINBY;                       /* WinBy can be positive or negative */
typedef unsigned long int MEXCALC;       /* ...since we use a long int here */

typedef unsigned long long POSITION;           /* This used to be determined by mTempTypedef.h */
#define INVALID_POSITION ~((POSITION)0)
typedef int MOVE;                              /* but I decided to hardcode it for now, so as  */
typedef int MOVES;                              /* to make it a leeeetle bit easier and cleaner */

typedef int REMOTENESS;
typedef char*                   STRING;
typedef void*                   GENERIC_PTR;

// Tier Gamesman typedefs
typedef unsigned long long TIER;
typedef POSITION TIERPOSITION; /* Used to differentiate "tier-specific" positions */
typedef int UNDOMOVE; /* Just like MOVE but used specifically for undo-moves */

// BPDB
typedef unsigned char BYTE;
typedef unsigned char UINT8;
typedef unsigned int UINT32;
typedef long long int INT64;
typedef unsigned long long int UINT64;

// SEval
typedef float (*featureEvaluatorCustom)(POSITION);

typedef UINT32 GMSTATUS;

#ifndef BOOLEAN             /* To satisfy Visual C++ 6.0 compiler */
#ifdef WIN32
typedef unsigned char BOOLEAN;
#else /* WIN32 */
typedef int BOOLEAN;
#endif /* WIN32 */
#endif /* BOOLEAN */

typedef enum value_enum
{
	undecided, win, lose, tie
}
VALUE;

typedef enum menu_enum
{
	BeforeEvaluation, Evaluated, Analysis
}
MENU;

typedef enum user_input_enum
{
	Undo, Abort, Continue, Move, Configure, Switch

}
USERINPUT;

typedef enum static_func_msg_enum
{
	Init, Clean, Save, Update, Poll, InitVar, CleanVar, SaveVar, AdvanceTier
}
STATICMESSAGE;

typedef struct positionlist_item
{
	POSITION position;
	struct positionlist_item *next;
}
POSITIONLIST, FRnode, POSITIONQUEUE;

typedef struct undolist_item
{
	POSITION position;
	TIERPOSITION tierposition; //Added for
	TIER tier; //Tier-Gamesman undo
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

typedef struct tierlist_item
{
	TIER tier;
	struct tierlist_item *next;
}
TIERLIST;

typedef struct undomovelist_item
{
	UNDOMOVE undomove;
	struct undomovelist_item *next;
}
UNDOMOVELIST;

typedef enum play_opponent
{
	AgainstComputer,AgainstHuman,ComputerComputer, AgainstEvaluator
}
OPPONENT;

#endif /* GMCORE_TYPES_H */
