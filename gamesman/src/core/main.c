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
#include "hash.h"

/*
** Globals
*/

VALUE (*gSolver)(POSITION) = NULL;
BOOLEAN (*gGoAgain)(POSITION,MOVE) = NULL;
POSITION (*gCanonicalPosition)(POSITION) = NULL;
void (*gUndoMove)(MOVE move) = NULL;
STRING (*GetHelpTextInterface)() = NULL;
STRING (*GetHelpOnYourTurn)() = NULL;
STRING (*GetHelpObjective)() = NULL;
STRING (*GetHelpTieOccursWhen)() = NULL;

VALUE   gValue = undecided;          /* The value of the game */
BOOLEAN gAgainstComputer = TRUE;     /* TRUE iff the user is playing the computer */
BOOLEAN gHumanGoesFirst = TRUE;      /* TRUE iff the user goes first vs. computer */
BOOLEAN gPrintPredictions = TRUE;    /* TRUE iff the predictions should be printed */
BOOLEAN gHints = FALSE;              /* TRUE iff possible moves should be printed */
BOOLEAN gUnsolved = FALSE;           /* TRUE iff playing without solving */

BOOLEAN gStandardGame = TRUE;               /* TRUE iff game is STANDARD (not REVERSE) */
//TEMPORARALLY DISABLE FILE DATABASES TILL MOVED TO NEW DB ARCH.
BOOLEAN gWriteDatabase = FALSE;    /* Default is to write the database */
BOOLEAN gReadDatabase = FALSE;     /* Default is to read the database if it exists */
BOOLEAN gPrintDatabaseInfo = FALSE; /* Print to the console */
BOOLEAN gJustSolving = FALSE;     /* Default is playing game, not just solving*/
BOOLEAN gMessage = FALSE;         /* Default is no message */
BOOLEAN gSolvingAll = FALSE;      /* Default is to not solve all */
BOOLEAN gTwoBits = FALSE;	      /* Two bit solver, default: FALSE */
BOOLEAN gCollDB = FALSE;
BOOLEAN gGlobalPositionSolver = FALSE;
BOOLEAN gUseGPS = FALSE;
BOOLEAN kZeroMemSolver = FALSE;	  /* Zero Memory Overhead Solver, default: FALSE */
BOOLEAN gAnalyzing = FALSE;       /* Write analysis for each variant 
				   * solved, default: FALSE */
BOOLEAN gSymmetries = FALSE;

char    gPlayerName[2][MAXNAME] = {"", ""}; /* The names of the players user/user or comp/user */
VALUE * gDatabase = NULL;
char *  gVisited = NULL;
STRING  kSolveVersion = "2005.02.09" ;

int   smartness = SMART;
int   scalelvl = MAXSCALE;
int   remainingGivebacks = 0;
int   initialGivebacks = 0;
VALUE oldValueOfPosition = tie;

MENU gMenuMode = BeforeEvaluation;
BOOLEAN gPrintHints = TRUE;

BOOLEAN gSkipInputOnSingleMove = FALSE; /*prompt the user even when there is only one move to take*/

/*
** Local function prototypes
*/

static void	SetSolver ();


/*
** Code
*/

void InitializeDatabases()
{
    db_initialize();
}

void Initialize()
{
    srand(time(NULL));
    
    /* set default solver */
    gSolver = NULL;
    
    /* set default go again */
    gGoAgain=DefaultGoAgain;
    
    sprintf(gPlayerName[kPlayerOneTurn],"Player");
    sprintf(gPlayerName[kPlayerTwoTurn],"Computer");
    
    generic_hash_context_init();
    InitializeGame();
    SetSolver();
}

void SetSolver()
{
    /* if solver set externally, leave alone */
    if (gSolver != NULL)
        return;
    else if(kZeroMemSolver)
        gSolver = DetermineZeroValue;
    else if(kLoopy) {
        if (gGoAgain == DefaultGoAgain)
            gSolver = DetermineLoopyValue;
        else
            gSolver = lgas_DetermineValue;
    }
    else
        gSolver = DetermineValue1;
}

VALUE DetermineValue(POSITION position)
{
    gUseGPS = gGlobalPositionSolver && gUndoMove != NULL;

    if(gReadDatabase && loadDatabase()) {
        if (gPrintDatabaseInfo) printf("\nLoading %s from Database...",kGameName);
	
        if (GetValueOfPosition(position) == undecided) {
            if (gPrintDatabaseInfo) printf("\nRe-evaluating the value of %s...", kGameName);
            gSolver(position);
            if(gWriteDatabase)
                writeDatabase();
        }
    }
    else {
        if (gPrintDatabaseInfo) printf("\nEvaluating the value of %s...", kGameName);
        gSolver(position);
        showStatus(Clean);
        if(gWriteDatabase)
            writeDatabase();
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
    Initialize();
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

/* Handles the command line arguments by setting flags and options */
void HandleArguments (int argc, char *argv[])
{
    int i, option;
    for(i = 1; i < argc; i++) {
        if(!strcasecmp(argv[i], "--nodb")) {
            gWriteDatabase = FALSE;
            gReadDatabase = FALSE;
        }
        else if(!strcasecmp(argv[i], "--newdb"))
            gReadDatabase = FALSE;
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
        else if(!strcasecmp(argv[i], "--gps")) {
            gGlobalPositionSolver = TRUE;
        }
        else if(!strcasecmp(argv[i], "--lowmem")) {
            kZeroMemSolver = TRUE;
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
            printf("\nSyntax:\n"
		   "%s {--nodb | --newdb | --curroption | --numoptions |"
		   "\t\t --option <n> | --solve [<n> | <all>] | --analyze [ <linkname> ]|\n"
		   "\t\t--DoMove <args> <move> | --Primitive <args> | \n"
		   "\t\t--PrintPosition <args> --GenerateMoves <args>} | --help}\n\n"
		   "--nodb\t\t\tStarts game without loading or saving to the database.\n"
		   "--newdb\t\t\tStarts game and clobbers the old database.\n"
		   "--numoptions\t\tPrints the number of options.\n"
		   "--curroption\t\tPrints the current option.\n"
		   "--option <n>\t\tStarts game with the n option configuration.\n"
		   "--solve [<n> | <all>]\tSolves game with the n option configuration.\n"
		   "--2bit\t\t\tStarts game with two-bit solving enabled.\n"
		   "--colldb\t\tStarts game with Collision based Database. Currently Experimental. \n"
		   "--gps\t\t\tStarts game with global position solver enabled.\n"
		   "--lowmem\t\tStarts game with low memory overhead solver enabled.\n"
		   "\t\t\tTo solve all option configurations of game, use <all>.\n"
		   "\t\t\tIf <n> and <all> are ommited, it will solve the default\n"
		   "\t\t\tconfiguration.\n"
		   "\t\t\tExamples:\n"
		   "\t\t\t%s --solve\n"
		   "\t\t\t%s --solve 2\n"
		   "\t\t\t%s --solve all\n"
		   "--analyze\t\tCreates the analysis directory with info on all variants\n"
		   "--DoMove <args>\n"
		   "--Primitive <args>\n"
		   "--PrintPosition <args>\n"
		   "--GenerateMoves <args>\n\n", argv[0], argv[0], argv[0], argv[0]);
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
	return gamesman_main(argc, argv);
}

/* main() is not exported in shared libraries, thus gamesman_main will handle everything */
/* This is needed for external modules (e.g. python) to call it if necessary */
int gamesman_main(int argc, char *argv[])
{
    HandleArguments(argc, argv);
    
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
