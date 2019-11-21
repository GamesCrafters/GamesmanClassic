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
POSITIONLIST *(*gEnumerateWithinStage)(int) = NULL;
void (*gUndoMove)(MOVE move) = NULL;
int (*gRetrogradeTierValue)(POSITION) = NULL;
STRING (*GetHelpTextInterface)() = NULL;
STRING (*GetHelpOnYourTurn)() = NULL;
STRING (*GetHelpObjective)() = NULL;
STRING (*GetHelpTieOccursWhen)() = NULL;
STRING (*gMoveToStringFunPtr)(MOVE) = NULL;
STRING (*gGetVarStringPtr)() = NULL;

VALUE   gValue = undecided;          /* The value of the game */
BOOLEAN gAgainstComputer = TRUE;     /* TRUE iff the user is playing the computer */
BOOLEAN gHumanGoesFirst = TRUE;      /* TRUE iff the user goes first vs. computer */
BOOLEAN gPrintPredictions = TRUE;    /* TRUE iff the predictions should be printed */
BOOLEAN gHints = FALSE;              /* TRUE iff possible moves should be printed */
BOOLEAN gUnsolved = FALSE;           /* TRUE iff playing without solving */

BOOLEAN gStandardGame = TRUE;               /* TRUE iff game is STANDARD (not REVERSE) */
BOOLEAN gZeroMemPlayer = FALSE;		/*TRUE if we are reading from the gzip file when using memdb*/
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
BOOLEAN gBitPerfectDB = FALSE;
BOOLEAN gTwoBits = FALSE;	      /* Two bit solver, default: FALSE */
BOOLEAN gCollDB = FALSE;
BOOLEAN gUnivDB = FALSE;
BOOLEAN gAlphaBeta = FALSE;
BOOLEAN gGlobalPositionSolver = FALSE;
BOOLEAN gUseGPS = FALSE;
BOOLEAN gBottomUp = FALSE;    	  /* Default is no bottom up solving, should enable for only win4 */
BOOLEAN gZeroMemSolver = FALSE;	  /* Zero Memory Overhead Solver, default: FALSE */
BOOLEAN gAnalyzing = FALSE;       /* Write analysis for each variant
				   * solved, default: FALSE */
BOOLEAN gAnalysisLoaded = FALSE;    /* Has an analysis file been loaded */
BOOLEAN gSymmetries = FALSE;

char    gPlayerName[2][MAXNAME] = {"", ""}; /* The names of the players user/user or comp/user */

REMOTENESS gMaxRemoteness = 0;
REMOTENESS gMinRemoteness = 0;

int   smartness = SMART;
int   scalelvl = MAXSCALE;
int   remainingGivebacks = 0;
int   initialGivebacks = 0;
VALUE oldValueOfPosition = tie;

MENU gMenuMode = BeforeEvaluation;
BOOLEAN gPrintHints = TRUE;

BOOLEAN gSkipInputOnSingleMove = FALSE; /*prompt the user even when there is only one move to take*/
BOOLEAN gPlaying = FALSE;

/* NetworkDB Globals */
BOOLEAN gNetworkDB = FALSE; 
STRING  ServerAddress = "";
