#ifndef __GAMESMAN_H__
#define __GAMESMAN_H__

#ifndef NO_GRAPHICS
#include "tcl.h"
#include "tk.h"
#else	/* Dummy variables */
typedef int Tcl_Interp;
typedef int Tk_Window;
typedef int ClientData;
#define	TCL_OK		0
#define TCL_ERROR	1
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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
#include <limits.h>



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

typedef long POSITION;      /* This used to be determined by mTempTypedef.h */
typedef int MOVE;           /* but I decided to hardcode it for now, so as  */
typedef int MOVES;          /* to make it a leeeetle bit easier and cleaner */
#define POSITION_FORMAT "%ld"

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

/* gamesman.c globals */
extern VALUE   gValue;                  /* The value of the game */
extern MENU    gMenuMode;               /* The state of the main menu */
extern BOOLEAN gAgainstComputer;        /* TRUE iff the user is playing the computer */
extern BOOLEAN gHumanGoesFirst;         /* TRUE iff the user goes first vs. computer */
extern BOOLEAN gStandardGame;           /* TRUE iff game is STANDARD (not REVERSE) */
extern BOOLEAN gPrintPredictions;            /* TRUE iff the predictions should be printed */
extern BOOLEAN gPrintPossibleMoves;          /* TRUE iff possible moves should be printed */
extern char    gPlayerName[2][MAXNAME]; /* The names of the players user/user or comp/user */
extern char*   gValueString[];

/* module globals */
extern STRING   kHelpGraphicInterface; /* The Graphical Interface Help string. */
extern STRING   kHelpTextInterface;    /* The Help for Text Interface string. */
extern STRING   kHelpOnYourTurn;       /* The Help for Your turn string. */
extern STRING   kHelpStandardObjective;/* The Help for Objective string. */
extern STRING   kHelpReverseObjective; /* The Help for reverse Objective string. */
extern STRING   kHelpTieOccursWhen;    /* The Help for Tie occuring string. */
extern STRING   kHelpExample;          /* The Help for Exmaples string. */
extern STRING   kAuthorName;	       /* Name of the Author ... */

extern POSITION gInitialPosition;      /* The initial position of the game */
extern POSITION gMinimalPosition;
extern POSITION kBadPosition;
extern BOOLEAN  kPartizan;             /* TRUE <==> module is a Partizan game */
extern BOOLEAN  kGameSpecificMenu;     /* TRUE <==> module supports GameSpecificMenu() */
extern BOOLEAN  kTieIsPossible;        /* TRUE <==> A Tie is possible */
extern BOOLEAN  kLoopy;                /* TRUE <==> Game graph has cycles */
extern POSITION gNumberOfPositions;    /* The number of positions in the game */
extern BOOLEAN  gPrintHints ;
extern STRING   kGameName ;
extern STRING   kAuthorName ;
extern BOOLEAN  kDebugMenu ;
extern STRING   kDBName ;
extern BOOLEAN  kDebugDetermineValue ;

/* solver function pointer */
extern VALUE (*gSolver)(POSITION);

/* go again function pointer */
extern BOOLEAN (*gGoAgain)(POSITION,MOVE);

/* tcl initialization function pointer (needs to be void* so games don't need tcl) */
extern void* gGameSpecificTclInit;

/* memory-related function prototypes for games */
GENERIC_PTR SafeMalloc(size_t amount);
void        SafeFree(GENERIC_PTR ptr);
void        FreeMoveList(MOVELIST* ptr);
void        FreeRemotenessList(REMOTENESSLIST* ptr);
void        FreePositionList(POSITIONLIST* ptr);
void        FreeValueMoves(VALUE_MOVES *ptr);

/* utility function prototypes for games */
char          GetMyChar();
int           GetRandomNumber(int n);
int           GetSmallRandomNumber(int n);
void          ExitStageRight();
void          ExitStageRightErrorString(char errorMsg[]);
MOVELIST*     CreateMovelistNode(MOVE theMove, MOVELIST* theNextMove);
MOVELIST*     CopyMovelist(MOVELIST* theMovelist);
void          BadElse(STRING function);
void          HitAnyKeyToContinue();
USERINPUT     HandleDefaultTextInput(POSITION thePosition, MOVE* theMove, STRING playerName);
void          GetMyString(char* name, int size, BOOLEAN eatFirstChar, BOOLEAN putCarraigeReturnBack);
POSITIONLIST* StorePositionInList(POSITION thePosition, POSITIONLIST* thePositionList);
POSITIONLIST* CopyPositionlist(POSITIONLIST* thePositionlist);
STRING        GetPrediction(POSITION position, STRING playerName, BOOLEAN usersTurn);
BOOLEAN       DefaultGoAgain(POSITION pos, MOVE move);

/* function prototypes for tcl */
void        InitializeDatabases();
void        Initialize();
VALUE       DetermineValue(POSITION position);
VALUE       StoreValueOfPosition(POSITION position, VALUE value);
VALUE       GetValueOfPosition(POSITION position);
REMOTENESS  Remoteness(POSITION position);
void        SetRemoteness (POSITION position, REMOTENESS remoteness);
BOOLEAN     Visited(POSITION position);
void        MarkAsVisited (POSITION position);
void        UnMarkAsVisited (POSITION position);
MOVE        GetComputersMove(POSITION thePosition);

/* function prototypes for functions included in modules */
void      InitializeGame();
void      FreeGame();
void      DebugMenu();
void      GameSpecificMenu();
POSITION  DoMove(POSITION thePosition, MOVE theMove);
POSITION  GetInitialPosition();
void      PrintComputersMove(MOVE computersMove, STRING computersName);
VALUE     Primitive(POSITION position);
void      PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn);
MOVELIST* GenerateMoves(POSITION position);
USERINPUT GetAndPrintPlayersMove(POSITION thePosition, MOVE* theMove, 
				 STRING playerName);
BOOLEAN   ValidTextInput(STRING input);
MOVE      ConvertTextInputToMove(STRING input);
void      PrintMove(MOVE theMove);
int       NumberOfOptions();
int       getOption();
void      setOption(int option);

/* deprecated function prototypes for functions included in modules */
void SetTclCGameSpecificOptions(int theOptions[]);

/* Status Meter */
void showStatus(int done);

/* analysis functions */
void analyze();
void createVarTable();
void writeVarHTML();
void writeVarStat(char * statName, char * text, FILE *rowp);
void createAnalysisGameDir();
void createAnalysisVarDir();
void writeGameHTML();
//void createAnalysisLink();
#endif
