
#ifndef GMCORE_GLOBALS_H
#define GMCORE_GLOBALS_H

/*
** Solver globals
*/

extern VALUE	gValue;
extern BOOLEAN	gHumanGoesFirst, gPrintPredictions, gHints, gUnsolved;

extern BOOLEAN	gStandardGame, gSaveDatabase, gLoadDatabase,
		gPrintDatabaseInfo, gJustSolving, gMessage, gSolvingAll,
                gTwoBits, gCollDB, gUnivDB, gGlobalPositionSolver, gZeroMemSolver,
                gAnalyzing, gSymmetries, gUseGPS, gBottomUp;

extern char	gPlayerName[2][MAXNAME];
extern VALUE*	gDatabase;
extern char*	gVisited;

extern int	smartness, scalelvl, remainingGivebacks, initialGivebacks;
extern VALUE	oldValueOfPosition;

extern MENU	gMenuMode;
extern BOOLEAN	gPrintHints;


/*
** Miscellaneous globals
*/

extern int	gTimer;
extern BOOLEAN  gSkipInputOnSingleMove;


/*
** Analysis globals
*/

extern long	gTotalMoves,
		gTotalPositions;


/*
** Game module externs
*/

void		InitializeGame			(void);
POSITION	DoMove				(POSITION thePosition, MOVE theMove);
POSITION	GetInitialPosition		(void);
void		PrintComputersMove		(MOVE computersMove, STRING computersName);
VALUE		Primitive			(POSITION position);
void		PrintPosition			(POSITION position, STRING playerName, BOOLEAN usersTurn);
MOVELIST*	GenerateMoves			(POSITION position);
USERINPUT	GetAndPrintPlayersMove		(POSITION thePosition, MOVE* theMove, STRING playerName);
BOOLEAN		ValidTextInput			(STRING input);
MOVE		ConvertTextInputToMove		(STRING input);
void		PrintMove			(MOVE theMove);
int		NumberOfOptions			(void);
int		getOption			(void);
void		setOption			(int option);
void		SetTclCGameSpecificOptions	(int theOptions[]);
void		GameSpecificMenu		(void);
void		DebugMenu			(void);

/* constant string literals */
extern STRING   kHelpGraphicInterface; /* The Graphical Interface Help string. */
extern STRING   kHelpTextInterface;    /* The Help for Text Interface string. */
extern STRING   kHelpOnYourTurn;       /* The Help for Your turn string. */
extern STRING   kHelpStandardObjective;/* The Help for Objective string. */
extern STRING   kHelpReverseObjective; /* The Help for reverse Objective string. */
extern STRING   kHelpTieOccursWhen;    /* The Help for Tie occuring string. */
extern STRING   kHelpExample;          /* The Help for Exmaples string. */
extern STRING   kAuthorName;	       /* Name of the Author ... */
extern STRING   kGameName;
extern STRING   kDBName;	       /* supposed to be the name of the DB?*/

/* constant numeric and boolean values*/
extern POSITION gInitialPosition;      /* The initial position of the game */
extern POSITION gMinimalPosition;
extern POSITION gNumberOfPositions;    /* The number of positions in the game */
extern BOOLEAN  gPrintHints;
extern BOOLEAN  kDebugDetermineValue;
extern BOOLEAN  kDebugMenu;	       /* whether to display the debug menu or not*/
extern POSITION kBadPosition;
extern BOOLEAN  kPartizan;             /* TRUE <==> module is a Partizan game */
extern BOOLEAN  kGameSpecificMenu;     /* TRUE <==> module supports GameSpecificMenu() */
extern BOOLEAN  kTieIsPossible;        /* TRUE <==> A Tie is possible */
extern BOOLEAN  kLoopy;                /* TRUE <==> Game graph has cycles */

/* solver function pointer */
extern VALUE	(*gSolver)(POSITION);

/* go again function pointer */
extern BOOLEAN	(*gGoAgain)(POSITION,MOVE);

/* symmetries function pointer */
extern POSITION (*gCanonicalPosition)(POSITION);

/* Custom unhash into string function pointer (useful for TCL interoperability) */
extern STRING (*gCustomUnhash)(POSITION);

/* enumerate all positions that result from the same stage in a game */
/* argument and return value will have their typedefs later */
extern POSITIONLIST *(*gEnumerateWithinStage)(int);

/* Global position solver function pointer. */
extern void (*gUndoMove)(MOVE move);

/* tcl initialization function pointer (needs to be void* so games don't need tcl) */
extern void*	gGameSpecificTclInit;

/* Help string function pointers */
extern STRING (*GetHelpTextInterface)();
extern STRING (*GetHelpOnYourTurn)();
extern STRING (*GetHelpObjective)();
extern STRING (*GetHelpTieOccursWhen)();

typedef enum play_opponent { AgainstComputer,AgainstHuman,ComputerComputer } OPPONENT;

extern OPPONENT gOpponent;

#endif /* GMCORE_GLOBALS_H */
