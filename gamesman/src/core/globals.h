
#ifndef GMCORE_GLOBALS_H
#define GMCORE_GLOBALS_H

/*
** Solver globals
*/

extern VALUE	gValue;
extern BOOLEAN	gAgainstComputer, gHumanGoesFirst, gPrintPredictions, gHints,
		gUnsolved;

extern BOOLEAN	gStandardGame, gWriteDatabase, gReadDatabase,
		gPrintDatabaseInfo, gJustSolving, gMessage, gSolvingAll,
		gTwoBits, kZeroMemSolver, gAnalyzing;

extern char	gPlayerName[2][MAXNAME];
extern VALUE*	gDatabase;
extern char*	gVisited;
extern STRING	kSolveVersion;

extern int	smartness, scalelvl, remainingGivebacks, initialGivebacks;
extern VALUE	oldValueOfPosition;

extern MENU	gMenuMode;
extern BOOLEAN	gPrintHints;


/*
** Miscellaneous globals
*/

extern int	gTimer;


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
extern BOOLEAN  gPrintHints;
extern STRING   kGameName;
extern STRING   kAuthorName;
extern BOOLEAN  kDebugMenu;
extern STRING   kDBName;
extern BOOLEAN  kDebugDetermineValue;

/* solver function pointer */
extern VALUE	(*gSolver)(POSITION);

/* go again function pointer */
extern BOOLEAN	(*gGoAgain)(POSITION,MOVE);

/* tcl initialization function pointer (needs to be void* so games don't need tcl) */
extern void*	gGameSpecificTclInit;


#endif /* GMCORE_GLOBALS_H */
