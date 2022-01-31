
#ifndef GMCORE_GLOBALS_H
#define GMCORE_GLOBALS_H

/*
** Solver globals
*/

extern VALUE gValue;
extern BOOLEAN gHumanGoesFirst, gPrintPredictions, gPrintSEvalPredictions,
               gSEvalLoaded, gSEvalPerfect, gHints, gUnsolved;

extern BOOLEAN gStandardGame, gSaveDatabase, gLoadDatabase,
               gPrintDatabaseInfo, gJustSolving, gMessage, gSolvingAll,
               gBitPerfectDB, gBitPerfectDBSolver, gBitPerfectDBSchemes, gBitPerfectDBAllSchemes, gBitPerfectDBAdjust, gBitPerfectDBVerbose, gBitPerfectDBZeroMemoryPlayer,
               gTwoBits, gCollDB, gUnivDB, gFileDB,
               gGlobalPositionSolver, gZeroMemSolver,
               gAnalyzing, gSymmetries, gUseGPS, gBottomUp, gAlphaBeta, gUseOpen, gWinBy, gInterestingness, gWinByClose,
               gIncludeInterestingnessWithAnalysis,
               gVisTiers, gVisTiersPlain, gSolveOnlyTier;

extern char gPlayerName[2][MAXNAME];

extern int smartness, scalelvl, remainingGivebacks, initialGivebacks, gTierToOnlySolve;
extern VALUE oldValueOfPosition;

extern MENU gMenuMode;

extern REMOTENESS gMaxRemoteness, gMinRemoteness;

extern WINBY (*gPutWinBy)(POSITION);

// Array of custom static evaluator function names
extern STRING *gCustomTraits;

/*
** Visualization globals
*/
extern BOOLEAN gDrawEdges, gRemotenessOrder, gGenerateNodeViz, gVisualizing;


/*
** Miscellaneous globals
*/

extern BOOLEAN gPlaying, gSkipInputOnSingleMove, gZeroMemPlayer;
extern int gTimer;


/*
** Analysis globals
*/

extern long gTotalMoves, gTotalPositions;
extern BOOLEAN gAnalysisLoaded;


/*
** Game module externs
*/

/* constant string literals */
extern STRING kHelpGraphicInterface;           /* The Graphical Interface Help string. */
extern STRING kHelpTextInterface;              /* The Help for Text Interface string. */
extern STRING kHelpOnYourTurn;                 /* The Help for Your turn string. */
extern STRING kHelpStandardObjective;          /* The Help for Objective string. */
extern STRING kHelpReverseObjective;           /* The Help for reverse Objective string. */
extern STRING kHelpTieOccursWhen;              /* The Help for Tie occuring string. */
extern STRING kHelpExample;                    /* The Help for Exmaples string. */
extern STRING kAuthorName;                     /* Name of the Author ... */
extern STRING kGameName;
extern STRING kDBName;                         /* supposed to be the name of the DB?*/

/* constant numeric and boolean values*/
extern POSITION gInitialPosition;              /* The initial position of the game */
extern POSITION gMinimalPosition;
extern POSITION gNumberOfPositions;            /* The number of positions in the game */
extern POSITION (*gActualNumberOfPositionsOptFunPtr)(int variant);            /* function pointer to function that returns the actual number of positions in the game, based on the variant */
extern BOOLEAN gPrintHints;
extern BOOLEAN kDebugDetermineValue;
extern BOOLEAN kDebugMenu;                     /* whether to display the debug menu or not*/
extern POSITION kBadPosition;
extern BOOLEAN kPartizan;                      /* TRUE <==> module is a Partizan game */
extern BOOLEAN kGameSpecificMenu;              /* TRUE <==> module supports GameSpecificMenu() */
extern BOOLEAN kTieIsPossible;                 /* TRUE <==> A Tie is possible */
extern BOOLEAN kLoopy;                         /* TRUE <==> Game graph has cycles */

/* solver function pointer */
extern VALUE (*gSolver)(POSITION);

/* go again function pointer */
extern BOOLEAN (*gGoAgain)(POSITION,MOVE);

/* symmetries function pointer */
extern POSITION (*gCanonicalPosition)(POSITION);

/* Custom unhash into string function pointer (useful for TCL interoperability) */
extern STRING (*gCustomUnhash)(POSITION);
extern char (*gReturnTurn)(POSITION);

/* Custom unhash into void* function pointer (used in Static Evaluator) */
extern void*            (*linearUnhash)(POSITION);
extern featureEvaluatorCustom (*gGetSEvalCustomFnPtr)(STRING);

/* enumerate all positions that result from the same stage in a game */
/* argument and return value will have their typedefs later */
extern POSITIONLIST    *(*gEnumerateWithinStage)(int);

/* Global position solver function pointer. */
extern void (*gUndoMove)(MOVE move);

/* tcl initialization function pointer (needs to be void* so games don't need tcl) */
extern void*            gGameSpecificTclInit;

/* Help string function pointers */
extern STRING (*GetHelpTextInterface)();
extern STRING (*GetHelpOnYourTurn)();
extern STRING (*GetHelpObjective)();
extern STRING (*GetHelpTieOccursWhen)();

extern STRING (*gMoveToStringFunPtr)(MOVE);

/* Variant string function pointer */
extern STRING (*gGetVarStringPtr)();

extern OPPONENT gOpponent;

/* NetworkDB Globals */
extern BOOLEAN gNetworkDB;
extern STRING ServerAddress;

/* MP over network Globals */
extern STRING gMPServerAddress;
extern STRING gRemoteGameURL;
extern int gTurnNumber;

/* Tier Gamesman globals and function pointers */
// For the core
extern BOOLEAN gTierGamesman;
extern BOOLEAN gDBLoadMainTier;
extern TIERPOSITION gDBTierStart;
extern TIERPOSITION gDBTierEnd;
extern TIER kBadTier;
extern BOOLEAN gTierSolverMenu;
extern BOOLEAN gDontLoadTierDB;
extern unsigned int HASHTABLE_BUCKETS;
extern BOOLEAN gTierSolvePrint;
extern BOOLEAN gTotalTiers;
// For the hash window
extern BOOLEAN gHashWindowInitialized;
extern BOOLEAN gCurrentTierIsLoopy;
extern TIER*                    gTierInHashWindow;
extern TIERPOSITION*    gMaxPosOffset;
extern int gNumTiersInHashWindow;
extern TIER gCurrentTier;
extern TIERPOSITION gCurrentTierSize;
extern BOOLEAN*         gTierDBExists;
// For the modules
extern BOOLEAN kSupportsTierGamesman;
extern BOOLEAN kExclusivelyTierGamesman;
extern BOOLEAN kDebugTierMenu;
extern TIERPOSITION gInitialTierPosition;
extern TIER gInitialTier;
extern TIERLIST*                (*gTierChildrenFunPtr)(TIER);
extern TIERPOSITION (*gNumberOfTierPositionsFunPtr)(TIER);
extern void (*gGetInitialTierPositionFunPtr)(TIER*,TIERPOSITION*);
extern BOOLEAN (*gIsLegalFunPtr)(POSITION);
extern UNDOMOVELIST*    (*gGenerateUndoMovesToTierFunPtr)(POSITION,TIER);
extern POSITION (*gUnDoMoveFunPtr)(POSITION,UNDOMOVE);
extern STRING (*gTierToStringFunPtr)(TIER);
// For the experimental GenerateMoves
extern int (*gGenerateMovesEfficientFunPtr)(POSITION);
extern MOVE*            gGenerateMovesArray;
extern int MAXFANOUT;

/* Variables for the parallelized solver */
extern BOOLEAN gParallelizing;

/* Tcl interp for making calls to Tcl_Eval */
extern Tcl_Interp*              gTclInterp;

/* Anoto Pen support */
extern STRING gPenFile;
extern BOOLEAN gPenDebug;

/* Loopy Analysis */
extern BOOLEAN gPure;
extern BOOLEAN gCheckPure;


#endif /* GMCORE_GLOBALS_H */
