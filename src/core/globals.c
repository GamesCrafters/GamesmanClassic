/************************************************************************
**
** NAME:	global.c
**
** DESCRIPTION:	Allocates memory for and sets the defaults of globals
**
** AUTHOR:	GamesCrafters Research Group, UC Berkeley
**		Supervised by Dan Garcia <ddgarcia@cs.berkeley.edu>
**
** DATE:	2005-01-11
**
** LICENSE:	This file is part of GAMESMAN,
**		The Finite, Two-person Perfect-Information Game Generator
**		Released under the GPL:
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program, in COPYING; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
**************************************************************************/

#include "types.h"
#include "constants.h"

VALUE (*gSolver)(POSITION) = NULL;
BOOLEAN (*gGoAgain)(POSITION,MOVE) = NULL;
POSITION (*gCanonicalPosition)(POSITION) = NULL;
STRING (*gCustomUnhash)(POSITION) = NULL;
char (*gReturnTurn)(POSITION) = NULL;
void* (*linearUnhash)(POSITION) = NULL;
featureEvaluatorCustom (*gGetSEvalCustomFnPtr)(STRING) = NULL;
POSITIONLIST *(*gEnumerateWithinStage)(int) = NULL;
void (*gUndoMove)(MOVE move) = NULL;
STRING (*GetHelpTextInterface)() = NULL;
STRING (*GetHelpOnYourTurn)() = NULL;
STRING (*GetHelpObjective)() = NULL;
STRING (*GetHelpTieOccursWhen)() = NULL;
STRING (*gMoveToStringFunPtr)(MOVE) = NULL;
STRING (*gGetVarStringPtr)() = NULL;
WINBY (*gPutWinBy)(POSITION) = NULL;

POSITION (*gActualNumberOfPositionsOptFunPtr)(int variant) = NULL;

VALUE gValue = undecided;               /* The value of the game */
BOOLEAN gAgainstComputer = TRUE;        /* TRUE iff the user is playing the computer */
BOOLEAN gHumanGoesFirst = TRUE;         /* TRUE iff the user goes first vs. computer */
BOOLEAN gPrintPredictions = TRUE;       /* TRUE iff the predictions should be printed */
BOOLEAN gPrintSEvalPredictions = FALSE; /* TRUE iff the seval predictions should be printed */
BOOLEAN gSEvalLoaded = FALSE;           /* TRUE iff an evaluator is successfully loaded */
BOOLEAN gSEvalPerfect = FALSE;          /* TRUE iff an evaluator is never wrong */
BOOLEAN gWinBy = FALSE;               /* TRUE iff the computer is playing with WinBy */
BOOLEAN gWinByClose = FALSE;          /* TRUE iff the computer is playing with WinByClose */
BOOLEAN gHints = FALSE;                 /* TRUE iff possible moves should be printed */
BOOLEAN gUnsolved = FALSE;              /* TRUE iff playing without solving */

BOOLEAN gStandardGame = TRUE;               /* TRUE iff game is STANDARD (not REVERSE) */
BOOLEAN gZeroMemPlayer = FALSE;         /*TRUE if we are reading from the gzip file when using memdb*/
//TEMPORARALLY DISABLE FILE DATABASES TILL MOVED TO NEW DB ARCH.
// 12.2.2005 - first attempt to fix memdb so it can be saved.
//	     - This brings along a few big changes that will break all other dbs
//           - But nothing else.
BOOLEAN gSaveDatabase = TRUE;    /* Default is to write the database */
BOOLEAN gLoadDatabase = TRUE;     /* Default is to read the database if it exists */
BOOLEAN gPrintDatabaseInfo = TRUE; /* Print to the console */
BOOLEAN gJustSolving = FALSE;     /* Default is playing game, not just solving*/
BOOLEAN gMessage = FALSE;         /* Default is no message */
BOOLEAN gSolvingAll = FALSE;      /* Default is to not solve all */
BOOLEAN gBitPerfectDB = TRUE;
BOOLEAN gBitPerfectDBSolver = TRUE;
BOOLEAN gBitPerfectDBAdjust = TRUE;
BOOLEAN gBitPerfectDBSchemes = FALSE;
BOOLEAN gBitPerfectDBAllSchemes = FALSE;
BOOLEAN gBitPerfectDBZeroMemoryPlayer = FALSE;
BOOLEAN gBitPerfectDBVerbose = FALSE;
BOOLEAN gTwoBits = FALSE;             /* Two bit solver, default: FALSE */
BOOLEAN gCollDB = FALSE;
BOOLEAN gUnivDB = FALSE;
BOOLEAN gFileDB = FALSE;
BOOLEAN gAlphaBeta = FALSE;
BOOLEAN gGlobalPositionSolver = FALSE;
BOOLEAN gUseGPS = FALSE;
BOOLEAN gBottomUp = FALSE;        /* Default is no bottom up solving, should enable for only win4 */
BOOLEAN gZeroMemSolver = FALSE;   /* Zero Memory Overhead Solver, default: FALSE */
BOOLEAN gAnalyzing = FALSE;       /* Write analysis for each variant
                                   * solved, default: FALSE */
BOOLEAN gVisualizing = FALSE;     /* Write visualization for each variant solved,
                                                                         default: FALSE */
BOOLEAN gAnalysisLoaded = FALSE;    /* Has an analysis file been loaded */
BOOLEAN gInterestingness = FALSE;  /* use interestingness solver after regular solver */
BOOLEAN gIncludeInterestingnessWithAnalysis = TRUE;
BOOLEAN gSymmetries = FALSE;
BOOLEAN gUseOpen = FALSE;       /* Use open positions solver, will be set to TRUE once solver is finalised */
BOOLEAN gVisTiers = FALSE;
BOOLEAN gVisTiersPlain = FALSE;
BOOLEAN gSolveOnlyTier = FALSE;

char gPlayerName[2][MAXNAME] = {"", ""};    /* The names of the players user/user or comp/user */

REMOTENESS gMaxRemoteness = 0;
REMOTENESS gMinRemoteness = 0;

int smartness = SMART;
int scalelvl = MAXSCALE;
int remainingGivebacks = 0;
int initialGivebacks = 0;
int gTierToOnlySolve = -1;
VALUE oldValueOfPosition = tie;

MENU gMenuMode = BeforeEvaluation;
BOOLEAN gPrintHints = TRUE;

BOOLEAN gSkipInputOnSingleMove = FALSE; /*prompt the user even when there is only one move to take*/
BOOLEAN gPlaying = FALSE;

/* Visualization Globals */
BOOLEAN gDrawEdges = TRUE;
BOOLEAN gRemotenessOrder = TRUE;
BOOLEAN gGenerateNodeViz = FALSE;

/* NetworkDB Globals */
BOOLEAN gNetworkDB = FALSE;
STRING ServerAddress = "nyc.cs.berkeley.edu:8080/GamesmanServlet";

/* MP over network Globals */
STRING gMPServerAddress = "127.0.0.1:3000/game/request_game_url";
STRING gRemoteGameURL = NULL;
int gTurnNumber = 0;

/* Tier Gamesman globals and function pointers */
// For the core
BOOLEAN gTierGamesman = TRUE;
BOOLEAN gDBLoadMainTier = TRUE;
TIERPOSITION gDBTierStart = -1;
TIERPOSITION gDBTierEnd = -1;
TIER kBadTier = -1;
BOOLEAN gTierSolverMenu = TRUE;
BOOLEAN gDontLoadTierDB = FALSE;
unsigned int HASHTABLE_BUCKETS = 1024;
BOOLEAN gTierSolvePrint = TRUE;
BOOLEAN gTotalTiers = 0;
// For the hash window
BOOLEAN gHashWindowInitialized = FALSE;
BOOLEAN gCurrentTierIsLoopy = FALSE;
TIER*                   gTierInHashWindow = NULL;
TIERPOSITION*   gMaxPosOffset = NULL;
int gNumTiersInHashWindow = 0;
TIER gCurrentTier = -1;
TIERPOSITION gCurrentTierSize = 0;
BOOLEAN*        gTierDBExists = NULL;
// For the modules
BOOLEAN kSupportsTierGamesman = FALSE;
BOOLEAN kExclusivelyTierGamesman = FALSE;
BOOLEAN kDebugTierMenu = FALSE;
TIERPOSITION gInitialTierPosition = -1;
TIER gInitialTier = -1;
TIERLIST*               (*gTierChildrenFunPtr)(TIER) = NULL;
TIERPOSITION (*gNumberOfTierPositionsFunPtr)(TIER) = NULL;
void (*gGetInitialTierPositionFunPtr)(TIER*,TIERPOSITION*) = NULL;
BOOLEAN (*gIsLegalFunPtr)(POSITION) = NULL;
UNDOMOVELIST*   (*gGenerateUndoMovesToTierFunPtr)(POSITION,TIER) = NULL;
POSITION (*gUnDoMoveFunPtr)(POSITION,UNDOMOVE) = NULL;
STRING (*gTierToStringFunPtr)(TIER) = NULL;
// For the experimental GenerateMoves
int (*gGenerateMovesEfficientFunPtr)(POSITION) = NULL;
MOVE*           gGenerateMovesArray = NULL;
int MAXFANOUT = 100;

/* Variables for the parallelized solver */
BOOLEAN gParallelizing = FALSE;

/* Tcl interp for making calls to Tcl_Eval */
Tcl_Interp *gTclInterp = NULL;

/* Static Evaluator list of custom trait function names */
STRING  *gCustomTraits = NULL;

/* Anoto Pen support */
STRING gPenFile = NULL;
BOOLEAN gPenDebug = FALSE;

/* Loopy Analysis */
BOOLEAN gPure = FALSE;
BOOLEAN gCheckPure = TRUE;
