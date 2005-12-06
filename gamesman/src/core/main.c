/************************************************************************
**
** NAME:	main.c
**
** DESCRIPTION:	Program entry code and initialization.
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

#include <time.h>
#include "gamesman.h"
#include "solveloopyga.h"
#include "solveloopy.h"
#include "solvezero.h"
#include "solvestd.h"
#include "solvebottomup.h"
#include "hash.h"

/*
** Globals
*/

/* these have been moved to globals.c - 12/2/2005 */


/*
** Local function prototypes
*/

static void	SetSolver ();

/*
** Code
*/

void Initialize()
{
    srand(time(NULL));
    
    /* set default solver */
    /* already done in globals.c */
    //    gSolver = NULL;
    
    /* set default go again */
    gGoAgain=DefaultGoAgain;
    
    sprintf(gPlayerName[kPlayerOneTurn],"Player");
    sprintf(gPlayerName[kPlayerTwoTurn],"Computer");
    
    /* generic hash */
    generic_hash_context_init();

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
    else if(gZeroMemSolver)
        gSolver = DetermineZeroValue;
    else if(kLoopy) {
        if (gGoAgain == DefaultGoAgain)
            gSolver = DetermineLoopyValue;
        else
            gSolver = lgas_DetermineValue;
    }
    else if(gBottomUp)
	gSolver = DetermineValueBU;
    else
        gSolver = DetermineValueSTD;
}

VALUE DetermineValue(POSITION position)
{
    gUseGPS = gGlobalPositionSolver && gUndoMove != NULL;

    if(gLoadDatabase && loadDatabase()) {
        if (gPrintDatabaseInfo) printf("\nLoading %s from Database...",kGameName);
	
        if (GetValueOfPosition(position) == undecided) {
            if (gPrintDatabaseInfo) printf("\nRe-evaluating the value of %s...", kGameName);
            gSolver(position);
            if(gSaveDatabase) {
		printf("\nWriting the values of %s into a database...", kGameName);
                saveDatabase();
	    }
        }
    }
    else {
        if (gPrintDatabaseInfo) printf("\nEvaluating the value of %s...", kGameName);
        gSolver(position);
        showStatus(Clean);
        if(gSaveDatabase)
            saveDatabase();
    }

    gUseGPS = FALSE;
    gValue = GetValueOfPosition(position);
    return gValue;
}

/* Starts a normal textbased game. */
void StartGame()
{
    Initialize();
    Menus();
}

/* Solves the game and stores it, without anybody actually playing it */
void SolveAndStore()
{
    InitializeGame();
    InitializeDatabases();
    gAnalysis.TotalMoves = 0;
    DetermineValue(gInitialPosition);
    gAnalysis.TimeToSolve = Stopwatch();
    // analysis
    if (gAnalyzing) {
        analyze(); // sets global variables
        
        // Writing HTML Has Now Been Deprecated
        // createAnalysisVarDir();
        // writeVarHTML();
	    
        writeXML(Save);
    }
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
        }
        else if(!strcasecmp(argv[i], "--newdb"))
            gLoadDatabase = FALSE;
        else if(!strcasecmp(argv[i], "--numoptions")) {
            fprintf(stderr, "\nNumber of Options: %d\n", NumberOfOptions());
            gMessage = TRUE;
        }
        else if(!strcasecmp(argv[i], "--curroption")) {
            fprintf(stderr, "\nCurrent Option: %d\n", getOption());
            gMessage = TRUE;
        }
        else if(!strcasecmp(argv[i], "--option")) {
            if(argc < (i + 2)) {
                fprintf(stderr, "\nUsage: %s --option <n>\n\n", argv[0]);
                gMessage = TRUE;
            }
            else {
                option = atoi(argv[++i]);
                if(!option || option > NumberOfOptions()) {
                    fprintf(stderr, "\nInvalid option configuration!\n\n");
                    gMessage = TRUE;
                }
                else
                    setOption(option);
            }
        }
        else if(!strcasecmp(argv[i], "--2bit")) {
            gTwoBits = TRUE;
        }else if(!strcasecmp(argv[i], "--colldb")) {
            gCollDB = TRUE;
        }
	/* Enable usage of UnivDB - randomized hashing, collision database, dependent on GMP support */
	else if(!strcasecmp(argv[i], "--univdb")) {
#ifdef HAVE_GMP
	    gUnivDB = TRUE;
#else
	    fprintf(stderr, "\nGMP support must be compiled in to use --univdb option\n\n");
#endif
	}
        else if(!strcasecmp(argv[i], "--gps")) {
            gGlobalPositionSolver = TRUE;
        }
	else if(!strcasecmp(argv[i], "--bottomup")) {
	    gBottomUp = TRUE;
	}
        else if(!strcasecmp(argv[i], "--lowmem")) {
            gZeroMemSolver = TRUE;
        }
        else if(!strcasecmp(argv[i], "--solve")) {
            gJustSolving = TRUE;
            if((i + 1) < argc && !strcasecmp(argv[++i], "all"))
                gSolvingAll = TRUE;
            option = atoi(argv[i]);
            if(option > NumberOfOptions()) {
                fprintf(stderr, "Invalid option configuration!\n\n");
                gMessage = TRUE;
            }
            else if(option)
                setOption(option);
        }
        else if(!strcasecmp(argv[i], "--analyze")) {
            gJustSolving = TRUE;
            gAnalyzing = TRUE;
            gSolvingAll = TRUE;
            createAnalysisGameDir();
            writeGameHTML();
            createVarTable();
        }
        else if(!strcasecmp(argv[i], "--DoMove")) {
            InitializeGame();
            if(argc != 4)
                fprintf(stderr, "\nInvalid arguments!\n\n");
            else
                printf("\nDoMove returns: " POSITION_FORMAT "\n\n", DoMove(atoi(argv[2]), atoi(argv[3])));
            i += argc;
            gMessage = TRUE;
        }
        else if(!strcasecmp(argv[i], "--Primitive")) {
            InitializeGame();
            if(argc != 3)
                fprintf(stderr, "\nInvalid arguments!\n\n");
            else 
                printf("\nPrimitive returns: %u\n\n", Primitive(atoi(argv[2])));
            i += argc;
            gMessage = TRUE;
        }
        else if(!strcasecmp(argv[i], "--PrintPosition")) {
            InitializeGame();
            if(argc != 5)
                fprintf(stderr, "\nInvalid arguments!\n\n");
            else {
                printf("\nPrintPosition:\n\n");
                PrintPosition(atoi(argv[2]), argv[3], atoi(argv[4]));
            }
            i += argc;
            gMessage = TRUE;
        }
        else if(!strcasecmp(argv[i], "--GenerateMoves")) {
            i += argc;
            gMessage = TRUE;
        }
        else if(!strcasecmp(argv[i], "--help")) {
            printf(kCommandSyntaxHelp, argv[0], argv[0], argv[0], argv[0]);
            gMessage = TRUE;
            i += argc;
        }
        else {
            fprintf(stderr, "\nInvalid option or missing parameter, use %s --help for help\n\n", argv[0]);
            gMessage = TRUE;
            i += argc;
        }
    }
}


/* main() serves as a wrapper to gamesman_main() */
int main(int argc, char *argv[])
{
    HandleArguments(argc, argv);

    return gamesman_main();
}

/* main() is not exported in shared libraries, thus gamesman_main will handle everything */
/* This is needed for external modules (e.g. python) to call it if necessary */
int gamesman_main()
{
    //Initialize();

    if(!gMessage) {
        if(!gJustSolving)
            StartGame();
        else if(!gSolvingAll) {
            fprintf(stderr, "Solving \"%s\" option %u....", kGameName, getOption());
            fflush(stderr);
            SolveAndStore();
            fprintf(stderr, "done.\n");
        }
        else {
            int i;
            fprintf(stderr, "Solving \"%s\" option ", kGameName);
            
            if (gAnalyzing)
            {
                writeXML(Init);
            }
            
            for(i = 1; i <= NumberOfOptions(); i++) {
                fprintf(stderr, "%c[s%u of %u....", 27, i, NumberOfOptions());
                fflush(stderr);
                setOption(i);
                SolveAndStore();
                fprintf(stderr, "%c[u", 27);
            }
            
            if (gAnalyzing)
            {
                writeXML(Init);
            }
            
            fprintf(stderr, "%u of %u....done.\n", i - 1, NumberOfOptions());
        }
    }
    return 0;
}
