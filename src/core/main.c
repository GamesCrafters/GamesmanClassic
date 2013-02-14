/************************************************************************
**
** NAME:	main.c
**
** DESCRIPTION:	Program entry code and initialization.
**
** AUTHOR:	GamesCrafters Research Group, UC Berkeley
**		Supervised by Dan Garcia <ddgarcia@cs.berkeley.edu>
**
** DATE:	2006-03-06
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

#include <time.h>
#include "gamesman.h"
#include "solveloopyga.h"
#include "solveloopy.h"
#include "solvezero.h"
#include "solvestd.h"
#include "solvevsstd.h"
#include "solvevsloopy.h"
#include "solvebottomup.h"
#include "solveweakab.h"
#include "solveretrograde.h"
#include "hash.h"
#include "visualization.h"
#include "openPositions.h"
//#include "Parallel.h"
extern POSITION StringToPosition(STRING str);

/*
** Globals
*/

/* these have been moved to globals.c - 12/2/2005 */


/*
** Local function prototypes
*/

static void    SetSolver ();

/*
** Code
*/

void Initialize()
{
	/* For TIER GAMESMAN */
	gHashWindowInitialized = FALSE;

	gGetVarStringPtr = &get_var_string;
	if (gPutWinBy) {
		gWinBy = TRUE;
		gWinByClose = FALSE;
	}
	srand(time(NULL));

	/* set default solver */
	/* already done in globals.c */
	//    gSolver = NULL;

	/* set default go again */
	gGoAgain=DefaultGoAgain;

	sprintf(gPlayerName[kPlayerOneTurn],"Player");
	sprintf(gPlayerName[kPlayerTwoTurn],"Computer");

	/* generic hash */
	//generic_hash_context_init();

	/* get the DB function table with all pointers to default */
	CreateDatabases();

	/* game-specific variabless */
	InitializeGame();

	/* set the solver */
	SetSolver();
}

void SetSolver()
{
	/* if solver set externally, leave alone */
	if (gSolver != NULL)
		return;
	else if(kLoopy) {
		if (gGoAgain == DefaultGoAgain) {
			if(gBitPerfectDBSolver) {
				gSolver = &VSDetermineLoopyValue;
			} else {
				gSolver = &DetermineLoopyValue;
			}
		} else {
			gBitPerfectDBSolver = FALSE;
			gSolver = &lgas_DetermineValue;
		}
	} else if(gZeroMemSolver) {
		gSolver = &DetermineZeroValue;
	} else if(gBottomUp) {
		gSolver = &DetermineValueBU;
	} else if(gAlphaBeta) {
		gSolver = &DetermineValueAlphaBeta;
	} else if(gBitPerfectDBSolver) {
		gSolver = &DetermineValueVSSTD;
	} else {
		gSolver = &DetermineValueSTD;
	}
}

BOOLEAN InitializeGetMoveValuesStuff()
{
	gUseGPS = gGlobalPositionSolver && gUndoMove != NULL;
	//gSaveDatabase = FALSE;
	if (gAnalyzing && !LoadAnalysis()) {
		gLoadDatabase = FALSE;
	}

	return gLoadDatabase && LoadDatabase() && LoadOpenPositionsData();

}
VALUE DetermineValue(POSITION position)
{
	gUseGPS = gGlobalPositionSolver && gUndoMove != NULL;

	if (gAnalyzing && !LoadAnalysis()) {
		gLoadDatabase = FALSE;
	}

	if(kSupportsTierGamesman && gTierGamesman) { //TIER GAMESMAN
		gSolver = &DetermineRetrogradeValue; // force the retrograde solver
		gZeroMemPlayer = FALSE; // make sure tierdb behaves properly
		if (gPrintDatabaseInfo)
			printf("\nEvaluating the value of %s...", kGameName);
		gDBLoadMainTier = FALSE; // initialize main tier as undecided rather than load
		gSolver(position);
		gDBLoadMainTier = TRUE; // from now on, tierdb loads main tier too
		gInitializeHashWindow(gInitialTier, TRUE);
		position = gHashToWindowPosition(gInitialTierPosition, gInitialTier);
		gInitialPosition = position; // saves a LOT of little changes

		showStatus(Clean);
		AnalysisCollation();
		gAnalysisLoaded = TRUE;
		printf("done in %u seconds!\e[K", gAnalysis.TimeToSolve = Stopwatch()); /* Extra Spacing to Clear Status Printing */
		//if(gSaveDatabase) {
		//    if(gUseOpen) {
		//        SaveOpenPositionsData();
		//    }
		//    SaveAnalysis();
		//}

	} else if(gLoadDatabase && LoadDatabase() && LoadOpenPositionsData()) {
		if (GetValueOfPosition(position) == undecided) {
			if (gPrintDatabaseInfo)
				printf("\nRe-evaluating the value of %s...", kGameName);
			gSolver(position);
			AnalysisCollation();
			gAnalysisLoaded = TRUE;
			printf("done in %u seconds!\e[K", gAnalysis.TimeToSolve = Stopwatch()); /* Extra Spacing to Clear Status Printing */

			if(gSaveDatabase) {
				printf("\nWriting the values of %s into a database...", kGameName);
				SaveDatabase();
				if(gUseOpen) {
					SaveOpenPositionsData();
				}
				SaveAnalysis();
			}
		}
	} else {
		if (gPrintDatabaseInfo)
			printf("\nEvaluating the value of %s...", kGameName);
		// Ken Elkabany removed the line below, tell him if you want to undo it
		// bpdb will not work with it since it doesn't allocate itself until gSolver(position)
		// is called
		//StoreValueOfPosition(position, undecided);
		gSolver(position);
		showStatus(Clean);
		AnalysisCollation();
		gAnalysisLoaded = TRUE;
		printf("done in %u seconds!\e[K", gAnalysis.TimeToSolve = Stopwatch()); /* Extra Spacing to Clear Status Printing */

		if(gSaveDatabase) {
			SaveDatabase();
			if(gUseOpen) {
				SaveOpenPositionsData();
			}
			SaveAnalysis();
		}
	}
	gUseGPS = FALSE;
	gValue = GetValueOfPosition(position);

	return gValue;
}

/* Starts a normal textbased game. */
void StartGame(STRING executableName)
{
	Initialize();
	Menus(executableName);
}

/* Solves the game and stores it, without anybody actually playing it */
void SolveAndStore()
{
	Initialize();
	printf("\nInitialized..\n");
	if (gVisTiers || gVisTiersPlain)
	{
		//Don't initialize DB since we don't need to, it seems.
		printf("Skipping db initialzation... heh\n");
	}
	else
	{
		InitializeDatabases();
		printf("Initialized DBs..\n");
	}
	InitializeAnalysis();
	printf("Initialized Analysis...\n");
	gAnalysis.TotalMoves = 0;
	Stopwatch();
	printf("Going into solver....");
	DetermineValue(gInitialPosition);

	if (gAnalyzing) {
		// Writing HTML Has Now Been Deprecated
		// createAnalysisVarDir();
		// writeVarHTML();
		if (gIncludeInterestingnessWithAnalysis)
			DetermineInterestingness(gInitialPosition);
		writeXML(Save);
		writeXML(SaveVar);
		writeXML(CleanVar);
	}

	if (gVisualizing) {
		Visualize();
	}
}

void RemoteStartGamesman(BOOLEAN admin) {
	Initialize();
	InitializeDatabases();
	InitializeAnalysis();
	gAnalysis.TotalMoves = 0;

	gZeroMemPlayer = FALSE; // make sure tierdb behaves properly
	if (gPrintDatabaseInfo)
		printf("\nEvaluating the value of %s...", kGameName);
	gDBLoadMainTier = FALSE; // initialize main tier as undecided rather than load
	gTierSolvePrint = FALSE;

	RemoteInitialize(); //init the retrograde solver
	//if (admin)
	//    runAdmin_Main(NULL);
	//else runClient_Main(NULL);
}


/* Handles the command line arguments by setting flags and options
   declared in in globals.h */
void HandleArguments (int argc, char *argv[])
{
	int i, option;
	for(i = 1; i < argc; i++) {
		if(!strcasecmp(argv[i], "--nodb")) {
			gSaveDatabase = FALSE;
			gLoadDatabase = FALSE;
		} else if(!strcasecmp(argv[i], "--newdb"))
			gLoadDatabase = FALSE;
		else if(!strcasecmp(argv[i], "--filedb")) {
			gFileDB = TRUE;
			gBitPerfectDB = FALSE;
			gBitPerfectDBSolver = FALSE;
		}
		else if(!strcasecmp(argv[i], "--numoptions")) {
			fprintf(stderr, "\nNumber of Options: %d\n", NumberOfOptions());
			gMessage = TRUE;
		} else if(!strcasecmp(argv[i], "--curroption")) {
			fprintf(stderr, "\nCurrent Option: %d\n", getOption());
			gMessage = TRUE;
		} else if(!strcasecmp(argv[i], "--option")) {
			if(argc < (i + 2)) {
				fprintf(stderr, "\nUsage: %s --option <n>\n\n", argv[0]);
				gMessage = TRUE;
			} else {
				option = atoi(argv[++i]);
				if(!option || option > NumberOfOptions()) {
					fprintf(stderr, "\nInvalid option configuration!\n\n");
					gMessage = TRUE;
				} else
					setOption(option);
			}
		} else if(!strcasecmp(argv[i], "--nobpdb")) {
			gBitPerfectDB = FALSE;
			gBitPerfectDBSolver = FALSE;
		} else if(!strcasecmp(argv[i], "--2bit")) {
			gTwoBits = TRUE;
		} else if(!strcasecmp(argv[i], "--colldb")) {
			gCollDB = TRUE;
		}
		/* Enable usage of UnivDB - randomized hashing, collision database,
		   dependent on GMP support
		 */
		else if(!strcasecmp(argv[i], "--univdb")) {
#ifdef HAVE_GMP
			gUnivDB = TRUE;
#else
			fprintf(stderr, "\nGMP support must be compiled in to use --univdb option\n\n");
#endif
		} else if(!strcasecmp(argv[i], "--gps")) {
			gGlobalPositionSolver = TRUE;
		} else if(!strcasecmp(argv[i], "--bottomup")) {
			gBottomUp = TRUE;
		} else if(!strcasecmp(argv[i], "--alpha-beta")) {
			gAlphaBeta = TRUE;
		} else if(!strcasecmp(argv[i], "--lowmem")) {
			gZeroMemSolver = TRUE;
		} else if(!strcasecmp(argv[i], "--slicessolver")) {
			/*if(kLoopy) {
			   fprintf(stderr, "\nCannot use slices solver since this game is loopy\n\n");
			   gMessage = TRUE;
			   i += argc;
			   }*/
			gBitPerfectDBSolver = TRUE;
		} else if(!strcasecmp(argv[i], "--schemes")) {
			gBitPerfectDBSchemes = TRUE;
		} else if(!strcasecmp(argv[i], "--allschemes")) {
			gBitPerfectDBSchemes = TRUE;
			gBitPerfectDBAllSchemes = TRUE;
		} else if(!strcasecmp(argv[i], "--adjust")) {
			gBitPerfectDBAdjust = TRUE;
		} else if(!strcasecmp(argv[i], "--noadjust")) {
			gBitPerfectDBAdjust = FALSE;
		} else if(!strcasecmp(argv[i], "--bpdbverbose")) {
			gBitPerfectDBVerbose = TRUE;
		} else if(!strcasecmp(argv[i], "--bpdbzeroplayer")) {
			gBitPerfectDBZeroMemoryPlayer = TRUE;
		} else if(!strcasecmp(argv[i], "--notiers")) {
			gTierGamesman = FALSE;
		} else if(!strcasecmp(argv[i], "--vt")) {
			gVisTiers = TRUE; //Generates dotty file for tier tree visualization
		} else if(!strcasecmp(argv[i], "--vtp")) {
			gVisTiersPlain = TRUE; //outputs to stdout a plain tier tree for easy parsing
		} else if(!strcasecmp(argv[i], "--onlytier")) {
			if ((i + 1) < argc) {
				gTierToOnlySolve = atoi(argv[++i]);
				if (gTierToOnlySolve < 0) {
					fprintf(stderr, "Tier to solve is not valid\n\n");
					gMessage = TRUE;
				} else { //We're good to go
					gSolveOnlyTier = TRUE;
				}
			} else {
				fprintf(stderr, "No tier given for solve only tier option\n\n");
				gMessage = TRUE;
			}
		} else if(!strcasecmp(argv[i], "--notiermenu")) {
			gTierSolverMenu = FALSE;
		} else if(!strcasecmp(argv[i], "--notierprint")) {
			gTierSolvePrint = FALSE;
			gTierSolverMenu = FALSE;
		} else if(!strcasecmp(argv[i], "--solve")) {
			gJustSolving = TRUE;
			if((i + 1) < argc && !strcasecmp(argv[++i], "all"))
				gSolvingAll = TRUE;
			option = atoi(argv[i]);
			if(option > NumberOfOptions()) {
				fprintf(stderr, "Invalid option configuration!\n\n");
				gMessage = TRUE;
			} else if(option)
				setOption(option);
		} else if(!strcasecmp(argv[i], "--analyze")) {
			gJustSolving = TRUE;
			gAnalyzing = TRUE;
			//gSolvingAll = TRUE;
			createAnalysisGameDir();
			//writeGameHTML(); DEPRECATED
			//createVarTable(); DEPRECATED
		} else if(!strcasecmp(argv[i], "--nointerestingness")) {
			gIncludeInterestingnessWithAnalysis = FALSE;
			gInterestingness = FALSE;
		} else if(!strcasecmp(argv[i], "--open")) {
			gUseOpen = TRUE;
		} else if(!strcasecmp(argv[i], "--visualize")) {
			gVisualizing = TRUE;
		} else if(!strcasecmp(argv[i], "--DoMove")) {
			InitializeGame();
			if(argc != 4)
				fprintf(stderr, "\nInvalid arguments!\n\n");
			else {
				printf("\nDoMove returns: " POSITION_FORMAT "\n\n",
				       DoMove(atoi(argv[2]), atoi(argv[3])));
			}
			i += argc;
			gMessage = TRUE;
		} else if(!strcasecmp(argv[i], "--Primitive")) {
			InitializeGame();
			if(argc != 3)
				fprintf(stderr, "\nInvalid arguments!\n\n");
			else {
				printf("\nPrimitive returns: %u\n\n",
				       Primitive(atoi(argv[2])));
			}
			i += argc;
			gMessage = TRUE;
		} else if(!strcasecmp(argv[i], "--PrintPosition")) {
			InitializeGame();
			if(argc != 5)
				fprintf(stderr, "\nInvalid arguments!\n\n");
			else {
				printf("\nPrintPosition:\n\n");
				PrintPosition(atoi(argv[2]), argv[3], atoi(argv[4]));
			}
			i += argc;
			gMessage = TRUE;
		} else if(!strcasecmp(argv[i], "--GenerateMoves")) {
			InitializeGame();
			if (argc != 3)
				fprintf(stderr, "\nInvalid arguments!\n\n");
			else {
				printf("\nGenerateMoves returns: [ ");
				MOVELIST *moves = GenerateMoves(atoi(argv[2])), *ptr;
				for (ptr = moves; ptr != NULL; ptr = ptr->next) {
					PrintMove(ptr->move);
					printf(" ");
				}
				printf("]\n\n");
				FreeMoveList(moves);
			}
			i += argc;
			gMessage = TRUE;
		} else if(!strcasecmp(argv[i],"--withPen")) {
			if ((i+1) < argc) {
				gPenFile = argv[i++];
			}
		} else if(!strcasecmp(argv[i], "--penDebug")) {
			gPenDebug = TRUE;
		} else if(!strcasecmp(argv[i], "--lightplayer")) {
			i += argc;
			gZeroMemPlayer = TRUE;
		} else if(!strcasecmp(argv[i], "--help")) {
			printf(kCommandSyntaxHelp, argv[0], argv[0], argv[0], argv[0]);
			gMessage = TRUE;
			i += argc;
		} else if(!strcasecmp(argv[i], "--netDb")) {
			gNetworkDB = TRUE;
			gBitPerfectDB = FALSE;
			gBitPerfectDBSolver = FALSE;
			if ((i + 1) < argc) {
				ServerAddress = argv[i++];
			}
		} else if(!strcasecmp(argv[i],"--hashCounting")) {
			hashCounting();
			return;
		} else if(!strcasecmp(argv[i],"--hashtable_buckets")) {
			if(argc < (i + 2)) {
				fprintf(stderr, "\nUsage: %s --hashtable_buckets <n>\n\n",
				        argv[0]);
			} else {
				HASHTABLE_BUCKETS = atoi(argv[2]);
			}
			i++;
		} else if(!strcasecmp(argv[i],"--parallel")) { // for PARALLELIZATION
			gMessage = TRUE;
			//initializeODeepaBlue(argc,argv);
			return;
		} else if(!strcasecmp(argv[i],"--printdefault")) {
			/* Some games initialize help strings inside InitializeGame() */
			InitializeGame();
			/* prints long name, short name, default option,
			 * C author(s), and some k* values, delimited with ^
			 * Current k* values are kDebugMenu. kDebugDetermineValue,
			 * and any kHelp* that aren't written*/
			printf("%s^%s^%d^%s^%s^%s^%s\n", kGameName, kDBName, getOption(),
			       kAuthorName,
			       kDebugMenu ? "True" : "False",
			       kDebugDetermineValue ? "True" : "False",
			       kHelpStarWritten());
			gMessage = TRUE;
		}
		/* Stateless move-value querying for Thrift. */
		/* ./game --GetMoveValue <boardString> <whoseMove> <option> */
		else if(!strcasecmp(argv[i], "--GetMoveValue") ||
		        !strcasecmp(argv[i], "--GetNextMoveValues")) {
			if (argc != 5) {
				fprintf(stderr, "\nInvalid arguments!\n\n");
			} else {
				BOOLEAN getNextMoveValues =
				        !strcasecmp(argv[i], "--GetNextMoveValues");
				char *boardStr = argv[2];
				/* int whoseMove = atoi(argv[3]); */
				int option = atoi(argv[4]);

				setOption(option);
				Initialize();
				InitializeDatabases();

				// We want to print out tuples in the form of
				//    (position, move, value)
				// where "position" is a board string, "move" is a move string
				// that describes how to reach the specified board from its
				// parent, and "value" specifies the win/lose/tie value.
				/* POSITION pos = StringToPosition(boardStr, whoseMove, option); */
				POSITION pos = StringToPosition(boardStr);
				VALUE primitiveValue = Primitive(pos);
				if (getNextMoveValues) {
					if (primitiveValue) {
						printf("{}\n"); // Primitives have no children
					} else {
						printf("{");
						MOVELIST *moves = GenerateMoves(pos);
						// LOAD EVERYTHING
						if (InitializeGetMoveValuesStuff()) {
							while (moves != NULL) {
								POSITION child = DoMove(pos, moves->move);
								/* Broken. */
								/* char* childPosition = PositionToString(child, whoseMove, option); */

								// flip move values to change perspective
								VALUE value = GetValueOfPosition(child);
								if (value == win) {
									value = lose;
								} else if (value == lose) {
									value = win;
								}
								/* Broken. */
								/* printf("(%s, %s, %d)", childPosition, MoveToString(moves->move), */
								/*        GetValueOfPosition(child)); */
								moves = moves->next;
								if (moves != NULL) {
									printf(", ");
								}
							}
						} else {
							printf("error");
						}
						printf("}\n");
					}
				} else {
					if (InitializeGetMoveValuesStuff()) {
						// Output the move value for the current position.
						VALUE positionValue = primitiveValue ? primitiveValue :
						                      GetValueOfPosition(pos);
						// We omit the "move" field from the tuple.
						printf("(%s, , %d)\n", boardStr, positionValue);
					}
				}
			}
			i += argc;
			gMessage = TRUE;
		} else if (!strcasecmp(argv[i], "--export")) {
			i += 1;
			if (argc > i) {
				/* Use gamesman_main to run the solver. */
				gJustSolving = TRUE;
				gamesman_main(argv[0]);
				printf("\n");
				PrintBinaryGameValuesToFile(argv[i]);
				/* Prevent gamesman_main in main from also running the solver. */
				gMessage = TRUE;
			} else {
				printf("--export requires a filname.");
			}
		} else if (!strcasecmp(argv[i], "--interact")) {
			gJustSolving = TRUE;
			gamesman_main(argv[0]);
			ServerInteractLoop();
			gMessage = TRUE;
		} else {
			fprintf(stderr, "\nInvalid option or missing parameter: %s, use %s --help for help\n\n", argv[i], argv[0]);
			gMessage = TRUE;
			i += argc;
		}
	}
}


/* main() serves as a wrapper to gamesman_main() */
int main(int argc, char *argv[])
{
	HandleArguments(argc, argv);
	return gamesman_main(argv[0]);
}

/* main() is not exported in shared libraries, thus gamesman_main will handle everything */
/* This is needed for external modules (e.g. python) to call it if necessary */
int gamesman_main(STRING executableName)
{
	//Initialize();
	if(!gMessage) {
		if(!gJustSolving)
			StartGame(executableName);
		else {
			gTierSolverMenu = FALSE; // TIER-GAMESMAN: if just going to solve, turn off the menu
			if(!gSolvingAll) {
				fprintf(stderr, "Solving \"%s\" option %u....",
				        kGameName, getOption());
				fflush(stderr);
				if (gAnalyzing) {
					writeXML(Init);
					writeXML(InitVar);
				}
				SolveAndStore();
				fprintf(stderr, "done.\n");
			} else {
				int i;
				fprintf(stderr, "Solving \"%s\" option ", kGameName);

				if (gAnalyzing) {
					writeXML(Init);
				}

				for(i = 1; i <= NumberOfOptions(); i++) {
					fprintf(stderr, "%c[s%u of %u....",
					        27, i, NumberOfOptions());
					fflush(stderr);
					setOption(i);
					if (gAnalyzing) {
						writeXML(InitVar);
					}
					SolveAndStore();
					fprintf(stderr, "%c[u", 27);
				}
				if (gAnalyzing) {
					writeXML(Clean);
				}
				fprintf(stderr, "%u of %u....done.\n",
				        i - 1, NumberOfOptions());
			}
		}
	}
	return 0;
}
