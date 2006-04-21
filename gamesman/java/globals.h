
#ifndef GMCORE_GLOBALS_H
#define GMCORE_GLOBALS_H

/*
** Solver globals
*/

extern VALUE		gValue;
extern BOOLEAN		gHumanGoesFirst, gPrintPredictions, gHints, gUnsolved;

extern BOOLEAN		gStandardGame, gSaveDatabase, gLoadDatabase,
	gPrintDatabaseInfo, gJustSolving, gMessage, gSolvingAll,
	gBitPerfectDB, gTwoBits, gCollDB, gUnivDB, gGlobalPositionSolver, gZeroMemSolver,
	gAnalyzing, gSymmetries, gUseGPS, gBottomUp, gAlphaBeta;

extern char		gPlayerName[2][MAXNAME];

extern int		smartness, scalelvl, remainingGivebacks, initialGivebacks;
extern VALUE		oldValueOfPosition;

extern MENU		gMenuMode;

extern REMOTENESS 	gMaxRemoteness, gMinRemoteness;


/*
** Miscellaneous globals
*/

extern BOOLEAN  	gPlaying, gSkipInputOnSingleMove, gZeroMemPlayer;
extern int		gTimer;


/*
** Analysis globals
*/

extern long		gTotalMoves, gTotalPositions;
extern BOOLEAN		gAnalysisLoaded;


/*
** Game module externs
*/

/* constant string literals */
extern STRING   	kHelpGraphicInterface; /* The Graphical Interface Help string. */
extern STRING   	kHelpTextInterface;    /* The Help for Text Interface string. */
extern STRING   	kHelpOnYourTurn;       /* The Help for Your turn string. */
extern STRING  		kHelpStandardObjective;/* The Help for Objective string. */
extern STRING   	kHelpReverseObjective; /* The Help for reverse Objective string. */
extern STRING   	kHelpTieOccursWhen;    /* The Help for Tie occuring string. */
extern STRING   	kHelpExample;          /* The Help for Exmaples string. */
extern STRING   	kAuthorName;	       /* Name of the Author ... */
extern STRING   	kGameName;
extern STRING   	kDBName;	       /* supposed to be the name of the DB?*/

/* constant numeric and boolean values*/
extern POSITION 	gInitialPosition;      /* The initial position of the game */
extern POSITION 	gMinimalPosition;
extern POSITION 	gNumberOfPositions;    /* The number of positions in the game */
extern BOOLEAN  	gPrintHints;
extern BOOLEAN  	kDebugDetermineValue;
extern BOOLEAN  	kDebugMenu;	       /* whether to display the debug menu or not*/
extern POSITION 	kBadPosition;
extern BOOLEAN  	kPartizan;             /* TRUE <==> module is a Partizan game */
extern BOOLEAN  	kGameSpecificMenu;     /* TRUE <==> module supports GameSpecificMenu() */
extern BOOLEAN  	kTieIsPossible;        /* TRUE <==> A Tie is possible */
extern BOOLEAN  	kLoopy;                /* TRUE <==> Game graph has cycles */

/* solver function pointer */
extern VALUE		(*gSolver)(POSITION);

/* go again function pointer */
extern BOOLEAN		(*gGoAgain)(POSITION,MOVE);

/* symmetries function pointer */
extern POSITION 	(*gCanonicalPosition)(POSITION);

/* Custom unhash into string function pointer (useful for TCL interoperability) */
extern STRING 		(*gCustomUnhash)(POSITION);

/* enumerate all positions that result from the same stage in a game */
/* argument and return value will have their typedefs later */
extern POSITIONLIST    *(*gEnumerateWithinStage)(int);

/* Global position solver function pointer. */
extern void 		(*gUndoMove)(MOVE move);

/* Retrograde Solver function pointer. */
extern int			(*gRetrogradeTierValue)(POSITION);

/* tcl initialization function pointer (needs to be void* so games don't need tcl) */
extern void*		gGameSpecificTclInit;

/* Help string function pointers */
extern STRING 		(*GetHelpTextInterface)();
extern STRING 		(*GetHelpOnYourTurn)();
extern STRING 		(*GetHelpObjective)();
extern STRING 		(*GetHelpTieOccursWhen)();

extern STRING 		(*gMoveToStringFunPtr)(MOVE);

/* Variant string function pointer */
extern STRING (*gGetVarStringPtr)();

extern OPPONENT 	gOpponent;

/* NetworkDB Globals */
extern BOOLEAN 		gNetworkDB;
extern STRING		ServerAddress;

#endif /* GMCORE_GLOBALS_H */
