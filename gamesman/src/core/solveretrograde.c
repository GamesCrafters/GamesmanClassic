// $Id: solveretrograde.c,v 1.37 2007-05-01 01:26:59 max817 Exp $

/************************************************************************
**
** NAME:	solveretrograde.c
**
** DESCRIPTION:	The Retrograde Solver.
**
** AUTHOR:	Max Delgadillo
**		GamesCrafters Research Group, UC Berkeley
**		Supervised by Dan Garcia <ddgarcia@cs.berkeley.edu>
**
** DATE:	2006-10-11
**
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

#include "gamesman.h"
#include "solveretrograde.h"
#include "tierdb.h"
#include "dirent.h"

// TIER VARIABLES
TIERLIST* tierSolveList; // the total list for the game, w/initial at start
TIERLIST* solveList; // only the tiers we have yet to solve, REVERSED
TIERLIST* solvedList; // the tiers we have already solved
TIER numTiers; // the total number of tiers for this game
TIER tiersSolved; // the number of tiers we've solved so far
// PRE-SOLVER VARIABLES:
int variant; // The variant of this game, used so getOption() is only called once
char filename[80]; // a global filename variable for re-use.
STRING tierStr; // a global tier string variable for re-use.
BOOLEAN tierNames; // Whether or not to display names of tiers.
BOOLEAN checkLegality; // Whether or not to check legality of tierpositions.
BOOLEAN useUndo; // Whether or not to use undomove functions.
BOOLEAN forceLoopy; // Whether or not to force the loopy solver on non-loopy tiers.
BOOLEAN checkCorrectness; // Whether or not to check correctness after the solve.
BOOLEAN useLevelFiles; // Whether or not to use Level Files in this solve.

// Solver procs
void checkExistingDB();
void AutoSolveAllTiers();
BOOLEAN gotoNextTier();
void solveFirst(TIER);
void PrepareToSolveNextTier();
void changeTierSolveList();
BOOLEAN setInitialTierPosition();
POSITION GetMyPosition();
BOOLEAN ConfirmAction(char);
// Solver Heart
void SolveTier(POSITION, POSITION);
void SolveWithNonLoopyAlgorithm(POSITION, POSITION);
void SolveWithLoopyAlgorithm(POSITION, POSITION);
void LoopyParentsHelper(POSITIONLIST*, VALUE, REMOTENESS);
// Solver ChildCounter and Hashtable functions
void rInitFRStuff();
void rFreeFRStuff();
POSITIONLIST* rRemoveFRList(VALUE, REMOTENESS);
void rInsertFR(VALUE, POSITION, REMOTENESS);
// Sanity Checkers
void checkForCorrectness(POSITION, POSITION);
TIERLIST* checkAndDefineTierTree();
BOOLEAN checkTierTree();
// Debug Stuff
void debugMenu();
// HAXX for comparing two databases
void writeCurrentDBToFile();
void writeTierDBToFile(TIER tier);
void compareTwoFiles(char*, char*, BOOLEAN);
void skipToNewline(FILE*);
// Remote function helpers
void TestRemote();
void r_getBounds(TIER, char*);
BOOLEAN r_checkChar(char, char, char*, int*);
BOOLEAN r_checkStr(char*, char*, int*);
char* r_intToString(int);


/************************************************************************
**
** NAME:        DetermineRetrogradeValue
**
** DESCRIPTION: Called by Gamesman, the solver goes into a menu
**				rather than just solving.
**
************************************************************************/

VALUE DetermineRetrogradeValue(POSITION position) {
    gDontLoadTierDB = FALSE;
    // for the experimental GenerateMoves (commented out for now)
    //if (gGenerateMovesEfficientFunPtr != NULL)
    //    gGenerateMovesArray = (MOVE*) SafeMalloc(MAXFANOUT * sizeof(MOVE));
	// initialize global variables
	variant = getOption();
	tierNames = TRUE;
    checkLegality = useUndo = forceLoopy = checkCorrectness = FALSE;
    // initialize local variables
	BOOLEAN cont = TRUE, isLegalGiven = TRUE, undoGiven = TRUE;

	printf("\n\n===== Welcome to the TIER-GAMESMAN Retrograde Solver! =====\n");
	printf("Currently solving game (%s) with variant (%d)\n", kGameName, variant);
    printf("Initial Position: %llu, in Initial Tier: %llu\n", gInitialTierPosition, gInitialTier);

	printf("\n----- Checking the REQUIRED API Functions: -----\n\n");

	if (gInitialTier == -1) {
		printf("-gInitialTier NOT GIVEN\n"); cont = FALSE;
	}
	if (gInitialTierPosition == -1) {
		printf("-gInitialTierPosition NOT GIVEN\n"); cont = FALSE;
	}
	if (gTierChildrenFunPtr == NULL) {
		printf("-TierChildren NOT GIVEN\n"); cont = FALSE;
	}
	if (gNumberOfTierPositionsFunPtr == NULL) {
		printf("-NumberOfTierPositions NOT GIVEN\n"); cont = FALSE;
	}
	if (cont == FALSE) {
		printf("\nOne or more required parts of the API not given...\n"
				"Exiting Retrograde Solver (WITHOUT Solving)...\n");
		exit(0);
	} else printf("API Required Functions Confirmed.\n");

	printf("\n----- Checking the OPTIONAL API Functions: -----\n\n");
	if (gIsLegalFunPtr == NULL) {
		printf("-IsLegal NOT GIVEN\nLegality Checking Disabled\n");
		isLegalGiven = FALSE;
	}
	if (gGenerateUndoMovesToTierFunPtr == NULL) {
		printf("-GenerateUndoMovesToTier NOT GIVEN\nUndoMove Use Disabled\n");
		undoGiven = FALSE;
	}
	if (gUnDoMoveFunPtr == NULL) {
		printf("-UnDoMove NOT GIVEN\nUndoMove Use Disabled\n");
		undoGiven = FALSE;
	}
	if (gTierToStringFunPtr == NULL) {
		printf("-TierToString NOT GIVEN\nTier Name Printing Disabled\n");
		tierNames = FALSE;
	}
	if (isLegalGiven && undoGiven && tierNames)
		printf("API Optional Functions Confirmed.\n");

	printf("\n----- Checking and Generating the Tier Tree: -----\n\n");

	solveList = checkAndDefineTierTree();
	if (solveList == NULL) {
		printf("\nPlease fix gTierChildren before attempting to solve!\n"
				"Exiting Retrograde Solver (WITHOUT Solving)...\n");
		ExitStageRight();
	} else printf("No Errors Found! Tier Tree generated successfully.\n");
	tierSolveList = CopyTierlist(solveList);
	solvedList = NULL;
	tiersSolved = 0;

	printf("-Tier Solve Order:\n");
	TIERLIST* ptr = solveList;
	numTiers = 0;
	for (; ptr != NULL; ptr = ptr->next) {
		printf("%llu ", ptr->tier);
		if (tierNames) {
			tierStr = gTierToStringFunPtr(ptr->tier);
			printf(": %s\n", tierStr);
			if (tierStr != NULL) SafeFree(tierStr);
		}
		numTiers++;
	}
	printf("\n   %llu Tiers are confirmed to be solved.\n", numTiers);

	printf("\n----- Checking for existing Tier DBs: -----\n\n");
	checkExistingDB();
	if (solveList == NULL) {
		printf("\nLooks like the game is already fully solved! Enjoy the game!\n");
	} else {
        if (tiersSolved == 0) // No DBs loaded, a fresh solve
            printf("No DBs Found! Starting a fresh solve...\n");

        if (gTierSolverMenu) {
            PrepareToSolveNextTier();

            char c;
            while(cont) {
                printf("\n\n\t----- RETROGRADE SOLVER MENU for game: %s -----\n", kGameName);
                printf("\tReady to solve %sLOOPY tier %llu", (gCurrentTierIsLoopy ? "" : "NON-"), gCurrentTier);
                if (tierNames) {
                    tierStr = gTierToStringFunPtr(gCurrentTier);
                    printf(" (%s)", tierStr);
                    if (tierStr != NULL) SafeFree(tierStr);
                }
                printf("\n\tThe tier hash contains (%lld) positions.", gCurrentTierSize);
                printf("\n\tTiers left: %llu (%.1f%c Solved)\n\n", numTiers-tiersSolved, 100*(double)tiersSolved/numTiers, '%');
                if (isLegalGiven)
                    printf("\tl)\tCheck (L)egality using IsLegal? Currently: %s\n", (checkLegality ? "YES" : "NO"));
                else printf("\t\t(Legality Checking using IsLegal DISABLED)\n");
                if (undoGiven)
                    printf("\tu)\t(U)se UndoMove functions for Loopy Solve? Currently: %s\n", (useUndo ? "YES" : "NO"));
                else printf("\t\t(Undomove functions for Loopy Solve DISABLED)\n");
                printf("\tc)\tCheck (C)orrectness after solve? Currently: %s\n"
                       "\tf)\t(F)orce Loopy solve for Non-Loopy tiers? Currently: %s\n\n"
                       "\ts)\t(S)olve the next tier.\n"
                       "\ta)\t(A)utomate the solving for all the tiers left.\n\n"
                       "\tt)\tChange the (T)ier Solve Order.\n\n"
                       "\tb)\t(B)egin the Game before fully solving!\n\n"
                       "\tq)\t(Q)uit the Retrograde Solver.\n"
                       "\nSelect an option:  ", (checkCorrectness ? "YES" : "NO"), (forceLoopy ? "YES" : "NO"));
                c = GetMyChar();
                switch(c) {
                    case 'l': case 'L':
                        if (isLegalGiven) //IsLegal is given
                            checkLegality = !checkLegality;
                        else printf("IsLegal isn't written! Thus, you can't check legality!\n");
                        break;
                    case 'u': case 'U':
                        if (undoGiven) //Undo stuff is given
                            useUndo = !useUndo;
                        else printf("UndoMove function(s) not written! Thus, you can't use the UndoMove Algorithm!\n");
                        break;
                    case 'c': case 'C':
                        checkCorrectness = !checkCorrectness;
                        break;
                    case 'f': case 'F':
                        forceLoopy = !forceLoopy;
                        break;
                    case 's': case 'S':
                        SolveTier(0,gCurrentTierSize);
                        if (!gotoNextTier()) {
                            printf("\n%s is now fully solved!\n", kGameName);
                            cont = FALSE;
                        } else PrepareToSolveNextTier();
                        break;
                    case 'a': case 'A':
                        printf("Fully Solving starting from Tier %llu...\n\n",gCurrentTier);
                        BOOLEAN loop = TRUE;
                        while (loop) {
                            PrepareToSolveNextTier();
                            SolveTier(0,gCurrentTierSize);
                            loop = gotoNextTier();
                            printf("\n\n---Tiers left: %llu (%.1f%c Solved)", numTiers-tiersSolved, 100*(double)tiersSolved/numTiers, '%');
                        }
                        printf("\n%s is now fully solved!\n", kGameName);
                        cont = FALSE;
                        break;
                    case 't': case 'T':
                        changeTierSolveList();
                        break;
                    case 'b': case 'B':
                        if (setInitialTierPosition()) {
                            cont = FALSE;
                        } break;
                    case 'q': case 'Q':
                        printf("Exiting Retrograde Solver (WITHOUT Fully Solving)...\n"
                                "Keep in mind that next time you start the solve, you will\n"
                                "continue from this exact point, if all the databases are there.\n"
                                "To ensure correct solving, make sure that the already-written\n"
                                "databases (in your data/m%s_%d_tierdb directory) are not altered,\n"
                                "and the API functions are unchanged from their current state.\n", kDBName, variant);
                        ExitStageRight();
                    case 'h': case 'H':
                        for (ptr = tierSolveList; ptr != NULL; ptr = ptr->next) {
                            char f1[80], f2[80];
                            sprintf(f1, "./data/a_%llu.txt", ptr->tier);
                            sprintf(f2, "./data/b_%llu.txt", ptr->tier);
                            compareTwoFiles(f1, f2, TRUE);
                        }
                        //compareTwoFiles("./a.txt", "./b.txt", FALSE);
                        break;
                    case 'p': case 'P':
                        TestRemote();
                        break;
                    default:
                        printf("Invalid option!\n");
                }
            }
        } else AutoSolveAllTiers(); // if no menu, go straight to auto solve!
    }
	printf("Exiting Retrograde Solver...\n\n");
	FreeTierList(tierSolveList);
	FreeTierList(solveList);
	FreeTierList(solvedList);
    return undecided; //just bitter at the fact that this is ignored
}

// Set on by the command line (or the GUI) when no menu must appear
void AutoSolveAllTiers() {
    printf("Fully Solving the game...\n\n");
	BOOLEAN loop = TRUE;
	while (loop) {
		PrepareToSolveNextTier();
		SolveTier(0,gCurrentTierSize);
		loop = gotoNextTier();
		printf("\n\n---Tiers left: %llu (%.1f%c Solved)", numTiers-tiersSolved, 100*(double)tiersSolved/numTiers, '%');
	}
	printf("\n%s is now fully solved!\n", kGameName);
}

// Inits the hash window/database and prepares to solve tier
void PrepareToSolveNextTier() {
	printf("\n------Preparing to solve tier: %llu\n", solveList->tier);
	gInitializeHashWindow(solveList->tier, TRUE);
	PercentDone(Clean); //reset percentage bar
	printf("  Done! Hash Window initialized and Database loaded and prepared!\n");
}

// we just solved the current tier, now go to the next
// returns TRUE if there's more tiers to solve, false otherwise
BOOLEAN gotoNextTier() {
	TIERLIST* temp = solveList;
	solveList = solveList->next;
	solvedList = CreateTierlistNode(temp->tier, solvedList);
	SafeFree(temp);
	tiersSolved++;
	return (solveList != NULL);
}

// Alters solveList so that tier goes on Front of list
void solveFirst(TIER tier) {
	solveList = MoveToFrontOfTierlist(tier, solveList);
}

// weed existing DBs out of the solveList
void checkExistingDB() {
	TIERLIST* ptr = solveList, *tmp;
	int result;

	while(ptr != NULL) {
		result = CheckTierDB(ptr->tier, variant); // check the tier's DB
		if(result == 0) {
			ptr = ptr->next;
			continue;
		} else if (result == -1) {
			printf("--%llu's Tier DB appears incorrect/corrupted. Re-solving...\n", ptr->tier);
			ptr = ptr->next;
			continue;
		} else if (result == 1) {
			printf("  %llu's Tier DB Found!\n", ptr->tier);
			if (ptr->tier != solveList->tier) {// if this isn't next to solve!
                tmp = ptr->next;
				solveFirst(ptr->tier);
				gotoNextTier();
				ptr = tmp;
			} else {//this is first on the list
				gotoNextTier();
				ptr = solveList;
			}
		}
	}
}

// A helper which tells what tiers CAN be set to be solved next
TIERLIST* possibleNextTiers() {
	TIERLIST* nexts = NULL, *ptr = solveList, *children, *childptr;
	TIER t, ct;
	for (; ptr != NULL; ptr = ptr->next) {
		t = ptr->tier;
		children = childptr = gTierChildrenFunPtr(t);
		for (; childptr != NULL; childptr = childptr->next) {
			ct = childptr->tier;
			if (TierInList(ct, solveList) && ct != t)
				break;
		}
		if (childptr == NULL) //all children solved!
			nexts = CreateTierlistNode(t, nexts);
		FreeTierList(children);
	}
	return nexts;
}

// A menu with which to change TierSolveList
void changeTierSolveList() {
	TIERLIST* nexts = possibleNextTiers();
	if (nexts->next == NULL) {
		printf("There's only that one tier to choose from!\n");
		FreeTierList(nexts);
		return;
	}
	while (TRUE) {
		TIERLIST* ptr = nexts;
		printf("You can choose from these tiers which to solve next:\n");
		for (; ptr != NULL; ptr = ptr->next) {
			printf("  %llu", ptr->tier);
			if (tierNames) {
				tierStr = gTierToStringFunPtr(ptr->tier);
				printf(" (%s)\n", tierStr);
				if (tierStr != NULL) SafeFree(tierStr);
			}
		}
		printf("\nEnter a TIER number to solve next, or non-number to go back:\n> ");
		POSITION p = GetMyPosition();
		if (p == kBadPosition) break;
		TIER t = (TIER)p;
		if (TierInList(t, nexts)) {
			solveFirst(t);
			PrepareToSolveNextTier();
			break;
		} else printf("That's not a tier to be solved next! Try again...\n\n");
	}
	FreeTierList(nexts);
}


BOOLEAN setInitialTierPosition() {
	gDBLoadMainTier = TRUE; // trick tierdb into loading main tier temporarily
	TIERPOSITION tp; TIER t;
	POSITION p, max;
	VALUE value; REMOTENESS remoteness;

	if (gGetInitialTierPositionFunPtr != NULL) {
		while (TRUE) {
			gGetInitialTierPositionFunPtr(&t, &tp);
			// check that tier is solved, and tierpos in range
			if (TierInList(t, solvedList) == FALSE) {
				printf("Tier %llu either isn't solved yet, or is an illegal tier!\n", t);
				continue;
			}
			if (tp < 0 || tp >= gNumberOfTierPositionsFunPtr(t)) {
				printf("Tierposition %llu isn't a valid position in tier %llu!\n", tp, t);
				continue;
			}
			// switch to hash window, and final check:
			gInitializeHashWindow(t, TRUE);
			printf("\nYou chose: gInitialTierPosition = %llu, gInitialTier = %llu:\n", tp, t);
			PrintPosition(tp, "Gamesman", TRUE);
			value = GetValueOfPosition(tp);
			remoteness = Remoteness(tp);
			if(remoteness == REMOTENESS_MAX)
				printf("This Position has value: Draw\n");
			else printf("This Position has value: %s in %d\n", gValueString[(int)value], remoteness);
			if (remoteness == 0) {
				if (value == undecided) {
					printf("This is an UNSOLVED (undecided) position! Choose another one!\n");
					continue;
				} else printf("This is a Primitive Position! Are you SURE you wish to\n");
			}
			printf("Exit the solver and begin the game from this position?\n");
			if (ConfirmAction('c')) {
				gInitialTier = t;
				gInitialTierPosition = tp;
				return TRUE;
			}
		}
	} else {
		TIERPOSITION tps[10];
		VALUE tpValues[10];
		REMOTENESS tpRems[10];

		int i; POSITION ptr;
		while(TRUE) {
			printf("\n--You can choose to start from one of these tiers:\n");
			TIERLIST* list;
			for (list = solvedList; list != NULL; list = list->next) {
				printf("%llu ", list->tier);
				if (tierNames) {
					tierStr = gTierToStringFunPtr(list->tier);
					printf(": %s\n", tierStr);
					if (tierStr != NULL) SafeFree(tierStr);
				}
			}
			printf("\n");

			// get the tier
			while(TRUE) {
				printf("\nNow enter a TIER number, or non-number to go back:\n> ");
				p = GetMyPosition();
				if (p == kBadPosition) break;
				t = (TIER)p;
				if (TierInList(t,solvedList) == TRUE) // found it
					break;
				printf("Tier %llu wasn't in the list!\n", t);
			}
			if (p == kBadPosition) break;

			// switch to hash window
			gInitializeHashWindow(t, TRUE);
			// now the tierposition, and confirm
			max = gNumberOfTierPositionsFunPtr(t);
			ptr = 0;
			while (TRUE) {
				printf("\nNow finding 10 playable positions...\n");
				/*
				  MAX'S NOTE: Yes, this infinite loops for a Tier that doesn't
				  have at least 10 non-undecided or Primitive positions. But then
				  again, it's the user's fault for picking such a boring tier, so ha.
				*/
				i = 0;
				while (i < 10) {
					value = GetValueOfPosition(ptr);
					if (value != undecided) {
						remoteness = Remoteness(ptr);
						if (remoteness > 0) {//store it
							tps[i] = ptr;
							tpValues[i] = value;
							tpRems[i] = remoteness;
							i++;
						}
					}
					ptr++;
					if (ptr >= max)
						ptr = 0;
				}
				printf("Found these positions:\n");
				for (i = 0; i < 10; i++) {
					if (tpRems[i] == REMOTENESS_MAX)
						printf("%d = Draw\n", i+1);
					else printf("%d = %s in %d\n", i+1, gValueString[(int)tpValues[i]], tpRems[i]);
				}
				printf("11 = See more positions");
				printf("\nNow pick a number above, or anything else to go back:\n> ");
				p = GetMyPosition();
				if (p == kBadPosition || p < 1 || p > 11) break;
				if (p == 11) continue;
				i = p-1;
				tp = tps[i]; value = tpValues[i]; remoteness = tpRems[i];
				printf("\nYou chose: gInitialTierPosition = %llu, gInitialTier = %llu:\n", tp, t);
				PrintPosition(tp, "Gamesman", TRUE);
				if(remoteness == REMOTENESS_MAX)
					printf("This Position has value: Draw\n");
				else printf("This Position has value: %s in %d\n", gValueString[(int)value], remoteness);
				printf("Exit the solver and begin the game from this position?\n");
				if (ConfirmAction('c')) {
					gInitialTier = t;
					gInitialTierPosition = tp;
					return TRUE;
				}
			}
		}
	}
	// switch back to the current hash window:
	gDBLoadMainTier = FALSE;
	gInitializeHashWindow(gCurrentTier, TRUE);
	return FALSE;
}

POSITION GetMyPosition() {
    char inString[MAXINPUTLENGTH];
    POSITION p; int i;
    while(TRUE) {
		GetMyStr(inString, MAXINPUTLENGTH);
		if (inString[0] == '\0') continue;
		p = 0;
		i = 0;
		while (inString[i] >= '0' && inString[i] <='9') {
			p = (p*10) + (inString[i]-'0');
			i++;
		}
		if (inString[i] != '\0' && inString[i] != '\n') {
			return kBadPosition;
		}
		return p;
	}
}

BOOLEAN ConfirmAction(char c) {
	printf("Enter '%c' to confirm, or anything else to cancel\n > ", c);
	return (GetMyChar() == c);
}

/************************************************************************
**
** NAME:        InitTierGamesman
**
** DESCRIPTION: Called by textui.c when NOT solving, just playing
**
************************************************************************/

// ONLY check tier tree. Called by textui.c
POSITION InitTierGamesman() {
    gDontLoadTierDB = TRUE;
	printf("\n-----Checking the REQUIRED TIER GAMESMAN API Functions:-----\n\n");
	BOOLEAN cont = TRUE;

	if (gInitialTier == -1) {
		printf("-gInitialTier NOT GIVEN\n"); cont = FALSE;
	}
	if (gInitialTierPosition == -1) {
		printf("-gInitialTierPosition NOT GIVEN\n"); cont = FALSE;
	}
	if (gTierChildrenFunPtr == NULL) {
		printf("-TierChildren NOT GIVEN\n"); cont = FALSE;
	}
	if (gNumberOfTierPositionsFunPtr == NULL) {
		printf("-NumberOfTierPositions NOT GIVEN\n"); cont = FALSE;
	}
	if (cont == FALSE) {
		printf("\nOne or more required parts of the API not given...\n");
		ExitStageRight();
	} else printf("API Required Functions Confirmed.\n");

	printf("\n-----Checking the Tier Tree for correctness:-----\n\n");
	if (!checkTierTree()) {
		printf("\nPlease fix gTierChildren before attempting to play!\n");
		ExitStageRight();
	} else printf("No Errors Found! Tier Tree correctness confirmed.\n");
	gInitializeHashWindow(gInitialTier, TRUE);
	return gHashToWindowPosition(gInitialTierPosition, gInitialTier);
}

/************************************************************************
**
** SOLVER CHILDCOUNTER AND HASHTABLE FUNCTIONS
**
************************************************************************/

//This will be my "children counter" until this goes to file
//or I think of something better.
//Oh, and I use chars as a hack way to get 8-bits
typedef unsigned char CHILDCOUNT;
CHILDCOUNT* childCounts;

//The Parent Pointers
POSITIONLIST** rParents;

/* Rather than a Frontier Queue, this uses a sort of hashtable,
with a POSITIONLIST for every REMOTENESS from 0 to REMOTENESS_MAX-1.
It's constant time insert and remove, so it works just fine. */
FRnode**	rWinFR = NULL;	// The FRontier Win Hashtable
FRnode**	rLoseFR = NULL;	// The FRontier Lose Hashtable
FRnode**	rTieFR = NULL;	// The FRontier Tie Hashtable

// A "hack" for dealing with a later-explained partial solve case
POSITIONLIST* solveTheseTooList;

/*
PROOFS OF CORRECTNESS.
I realized the following things about the Queue:

Processing WIN frontier, rem. R adds to LOSE, R+1.
    ""     LOSE   ""      ""  "" ""  "" WIN, R+1.
    ""     TIE    ""      ""  "" ""  "" TIE, R+1.
LOSES: When processing list R, will only be added to WINS list R+1.
WINS: The most complicated... when processing list R, it adds to
      LOSES list R+1. So if we process that LOSE list IMMEDIATELY,
      we'll be adding to the WINS list R+2.
TIES: When processing list R, will only be added to self list R+1.

So, have a list for every remoteness; order inside that list doesn't matter.
-Start by going through List Remoteness 0, then 1, then 2, etc...
As we go through frontier, things are only added ABOVE THE REMOTENESS
WE'RE CURRENTLY EXPLORING (the reason this all works). This is because
we always add to the "queue" as currentChild'sRemoteness+1. Therefore,
we can just iterate through a list one by one really fast and it all
works out.
-So, first go through the LOSE/WIN lists alternately, then TIE lists.
LOSE and TIE are basically the same; WIN is more interesting. Whenever
we get a win generating LOSEs, note that they're ALL the same remoteness.
So instead of putting them back in into the queue (since we're going to
see all of them next ANYWAY, and it doesn't matter the order) just have
a local "LOSE" list that you go through immediately!
-So it's proven that on an INSERT, you're NEVER going to be inserting
equal to lowestList or currentList.
*/

void rInitFRStuff() {
	int i;
	childCounts = (CHILDCOUNT*) SafeMalloc (gCurrentTierSize * sizeof(CHILDCOUNT));
    for (i = 0; i < gCurrentTierSize; i++)
        childCounts[i] = 0;
	if (!useUndo) {
		rParents = (POSITIONLIST**) SafeMalloc (gNumberOfPositions * sizeof(POSITIONLIST*));
		for (i = 0; i < gNumberOfPositions; i++)
			rParents[i] = NULL;
	}
	// 255 * 4 bytes = 1,020 bytes = ~1 KB
	rWinFR = (FRnode**) SafeMalloc (REMOTENESS_MAX * sizeof(FRnode*));
	rLoseFR = (FRnode**) SafeMalloc (REMOTENESS_MAX * sizeof(FRnode*)); // ~1 KB
	rTieFR = (FRnode**) SafeMalloc (REMOTENESS_MAX * sizeof(FRnode*)); // ~1 KB
	for (i = 0; i < REMOTENESS_MAX; i++)
		rWinFR[i] = rLoseFR[i] = rTieFR[i] = NULL;
    solveTheseTooList = NULL;
}

void rFreeFRStuff() {
	if (childCounts != NULL) SafeFree(childCounts);
	if (!useUndo) {
		// Free the Position Lists
		int i;
		for (i = 0; i < gNumberOfPositions; i++)
			FreePositionList(rParents[i]);
		if (rParents != NULL) SafeFree(rParents);
	}
	// Free the Position Lists
	if (rWinFR != NULL) SafeFree(rWinFR);
	if (rLoseFR != NULL) SafeFree(rLoseFR);
	if (rTieFR != NULL) SafeFree(rTieFR);
}

POSITIONLIST* rRemoveFRList(VALUE value, REMOTENESS r) {
	if (value == win)
		return rWinFR[r];
	else if (value == lose)
		return rLoseFR[r];
	else if (value == tie)
		return rTieFR[r];
	return NULL;
}

void rInsertFR(VALUE value, POSITION position, REMOTENESS r) {
	// this is probably the best place to put this:
	assert(r >= 0 && r < REMOTENESS_MAX);
	if(value == win)
		rWinFR[r] = StorePositionInList(position, rWinFR[r]);
	else if (value == lose)
		rLoseFR[r] = StorePositionInList(position, rLoseFR[r]);
	else if (value == tie)
		rTieFR[r] = StorePositionInList(position, rTieFR[r]);
}



/************************************************************************
**
** NAME:        SolveTier
**
** DESCRIPTION: The heart of the solver. This uses the current tier number,
**				then calls the "loopy solver" algorithm
**				to solve the current tier.
**				If non loopy, then it calls the non-loopy solver.
**              Expects gInitializeHashWindow to have already been
**              called with the correct tier, and the databases loaded
**              properly.
**
************************************************************************/

POSITION numSolved, trueSizeOfTier;

void SolveTier(POSITION start, POSITION end) {
	numSolved = trueSizeOfTier = 0;

    BOOLEAN partialSolve = FALSE;
    if (start != 0 || end != gCurrentTierSize) // we're only solving a partial tier!
        partialSolve = TRUE;
    // header
	printf("\n----- Solving Tier %llu",gCurrentTier);
	if (tierNames) {
		tierStr = gTierToStringFunPtr(gCurrentTier);
		printf(" (%s)", tierStr);
		SafeFree(tierStr);
	}
    if (partialSolve) {
        printf(" from %llu to %llu -----\n", start, end);
    } else printf("-----\n");
    // print flags information
	printf("Size of current hash window: %llu\n",gNumberOfPositions);
	printf("Size of tier %llu's hash: %llu\n",gCurrentTier,gCurrentTierSize);
	printf("\nSolver Type: %sLOOPY\n",((forceLoopy||gCurrentTierIsLoopy) ? "" : "NON-"));
	printf("Using Symmetries: %s\n",(gSymmetries ? "YES" : "NO"));
	printf("Checking Legality (using IsLegal): %s\n",(checkLegality ? "YES" : "NO"));
    // now actually SOLVE depending on which solver to use
	if (forceLoopy || gCurrentTierIsLoopy) {// LOOPY SOLVER
		printf("Using UndoMove Functions: %s\n",(useUndo ? "YES" : "NO"));
		SolveWithLoopyAlgorithm(start,end);
		printf("--Freeing Child Counters and Frontier Hashtables...\n");
		rFreeFRStuff();
	} else SolveWithNonLoopyAlgorithm(start,end);// NON-LOOPY SOLVER
	// successfully finished solving!
    if (partialSolve)
        printf("\nPartial Tier solved!\n");
    else printf("\nTier fully solved!\n");
	if (checkCorrectness) {
		printf("--Checking Correctness...\n");
		checkForCorrectness(start,end);
	}
    // now save to database
	printf("--Saving the Tier Database to File...\n");
    if (partialSolve) { // set the vars so tierdb knows what to do
        gDBTierStart = start;
        gDBTierEnd = end;
    }
    if (SaveDatabase())
		printf ("Database successfully saved!\n");
	else {
		printf("Couldn't save tierDB!\n");
		ExitStageRight();
	}
}

// Note, the NonLoopyAlgorithm works regardless of whether this
// is a partial tier or not (that's what's nice about it)...
void SolveWithNonLoopyAlgorithm(POSITION start, POSITION end) {
	printf("\n-----PREPARING NON-LOOPY SOLVER-----\n");
	POSITION pos, child;
	MOVELIST *moves, *movesptr;
	VALUE value;
	REMOTENESS remoteness;
	REMOTENESS maxWinRem, minLoseRem, minTieRem;
	BOOLEAN seenLose, seenTie;
	printf("Doing an sweep of the tier, and solving it in one go...\n");
	for (pos = start; pos < end; pos++) { // Solve only parents
        // the BYTEARRAY Level File stuff goes here
		if (checkLegality && !gIsLegalFunPtr(pos)) continue; //skip
        if (gSymmetries) {
            if (pos != gCanonicalPosition(pos))
			    continue; // skip, since we'll do canon one later
		    else trueSizeOfTier++;
        }
		value = Primitive(pos);
		if (value != undecided) { // check for primitive-ness
			SetRemoteness(pos,0);
			StoreValueOfPosition(pos,value);
		} else {
			moves = movesptr = GenerateMoves(pos);
			if (moves == NULL) { // no chillins
                printf("ERROR: GenerateMoves on %llu returned NULL\n", pos);
                ExitStageRight();
			} else {// else, solve me
				maxWinRem = -1;
				minLoseRem = minTieRem = REMOTENESS_MAX;
				seenLose = seenTie = FALSE;
				for (; movesptr != NULL; movesptr = movesptr->next) {
					child = DoMove(pos, movesptr->move);
					if (gSymmetries)
						child = gCanonicalPosition(child);
					value = GetValueOfPosition(child);
					if (value != undecided) {
						remoteness = Remoteness(child);
						if (value == tie) {
							seenTie = TRUE;
							if (remoteness < minTieRem)
								minTieRem = remoteness;
							continue;
						} else if (value == lose) {
							seenLose = TRUE;
							if (remoteness < minLoseRem)
								minLoseRem = remoteness;
							continue;
						} else if (remoteness > maxWinRem) //win
							maxWinRem = remoteness;
					} else {
						printf("ERROR: GenerateMoves on %llu found undecided child, %llu!\n", pos, child);
						ExitStageRight();
					}
				}
				FreeMoveList(moves);
				if (seenLose) {
					SetRemoteness(pos,minLoseRem+1);
					StoreValueOfPosition(pos, win);
				} else if (seenTie) {
					if (minTieRem == REMOTENESS_MAX)
						SetRemoteness(pos,REMOTENESS_MAX);// a draw
					else SetRemoteness(pos,minTieRem+1);// else a tie
					StoreValueOfPosition(pos, tie);
				} else {
					SetRemoteness(pos,maxWinRem+1);
					StoreValueOfPosition(pos, lose);
				}
			}
		}
	}
	if (checkLegality) {
		printf("--True size of tier: %lld\n",trueSizeOfTier);
		printf("--Tier %llu's hash efficiency: %.1f%c\n",gCurrentTier, 100*(double)trueSizeOfTier/gCurrentTierSize, '%');
	}
}

/************************************************************************
**
** NAME:        SolveWithLoopyAlgorithm
**
** DESCRIPTION: A Retrograde implementation of the loopy solver algorithm
**				(which is MUCH better/more efficient than the normal
**				loopy solver, dare I say).
**
************************************************************************/

void SolveWithLoopyAlgorithm(POSITION start, POSITION end) {
    BOOLEAN partialSolve = FALSE;
    if (start != 0 || end != gCurrentTierSize) // we're only solving a partial tier!
        partialSolve = TRUE;
	printf("\n-----PREPARING LOOPY SOLVER-----\n");
	POSITION pos, posSaver, canonPos, child;
    POSITIONLIST* tmp;
	MOVELIST *moves, *movesptr;
	VALUE value;
	REMOTENESS remoteness;
    //int i,numMoves; // the generateMovesEfficient stuff is commented out for now
	printf("--Setting up Child Counters and Frontier Hashtables...\n");
	rInitFRStuff();
	printf("--Doing an sweep of the tier, and setting up the frontier...\n");
	for (pos = start; pos < end; pos++) { // SET UP PARENTS
        posSaver = pos;
        // the BYTEARRAY Level File stuff goes here
solve_start: // GASP!! A LABEL!!
        if (childCounts[pos] == 0) { // else, ignore this child, it was already solved
		    if (checkLegality && !gIsLegalFunPtr(pos)) continue; //skip
		    if (gSymmetries && pos != gCanonicalPosition(pos))
			    continue; // skip, since we'll do canon one later
		    trueSizeOfTier++;
		    value = Primitive(pos);
		    if (value != undecided) { // check for primitive-ness
			    SetRemoteness(pos,0);
			    StoreValueOfPosition(pos,value);
			    numSolved++;
			    rInsertFR(value, pos, 0);
		    } else {
                //if (gGenerateMovesEfficientFunPtr == NULL) { // do the normal stuff
                    moves = movesptr = GenerateMoves(pos);
                    if (moves == NULL) { // no chillins
                        printf("ERROR: GenerateMoves on %llu returned NULL\n", pos);
                        ExitStageRight();
                    } else {
                        //otherwise, make a Child Counter for it
                        movesptr = moves;
                        for (; movesptr != NULL; movesptr = movesptr->next) {
                            childCounts[pos]++;
                            child = DoMove(pos, movesptr->move);
                            // here's the "partial solving" complication: to solve a position,
                            // we might have to solve another in this tier that's not part of our
                            // bounds! So, we need to run this loop for those guys too. To do this,
                            // we maintain a list of guys to run it for too.
                            // It uses the childCounts to ensure no duplicate iterations
                            if (partialSolve && child < gCurrentTierSize && childCounts[child] == 0
                                    && (child < start || child >= end)) {
                                solveTheseTooList = StorePositionInList(child, solveTheseTooList);
                            }
                            if (!useUndo) {// if parent pointers, add to parent pointer list
                                rParents[child] = StorePositionInList(pos, rParents[child]);
                            }
                        }
                        FreeMoveList(moves);
                    }
                //} else { // do the experimental stuff!
                //    numMoves = gGenerateMovesEfficientFunPtr(pos);
                //    if (numMoves == 0) { // no chillins
                //        printf("ERROR: GenerateMoves on %llu returned NULL\n", pos);
                //        ExitStageRight();
                //    } else {
                //        //otherwise, make a Child Counter for it
                //        for (i = 0; i < numMoves; i++) {
                //            childCounts[pos]++;
                //            child = DoMove(pos, gGenerateMovesArray[i]);
                //            if (partialSolve && child < gCurrentTierSize && childCounts[child] == 0
                //                    && child < start && child >= end) {
                //                solveTheseTooList = StorePositionInList(child, solveTheseTooList);
                //            }
                //            if (!useUndo) {// if parent pointers, add to parent pointer list
                //                rParents[child] = StorePositionInList(pos, rParents[child]);
                //            }
                //        }
                //    }
                //}
            }
        }
        // before ending the for-loop, we need to go through the solveTheseTooList
        if (partialSolve && solveTheseTooList != NULL) {
            // remove from head
            tmp = solveTheseTooList;
            pos = tmp->position;
            solveTheseTooList = tmp->next;
            SafeFree(tmp);
            // now, iterate for the new pos
            goto solve_start; // HOLY CRAP, A GOTO!
        }
        pos = posSaver;
    }
	if (checkLegality) {
		printf("True size of tier: %lld\n",trueSizeOfTier);
		printf("Tier %llu's hash efficiency: %.1f%c\n",gCurrentTier, 100*(double)trueSizeOfTier/gCurrentTierSize, '%');
	}
	printf("Amount now solved (primitives): %lld (%.1f%c)\n",numSolved, 100*(double)numSolved/trueSizeOfTier, '%');
	if (numSolved == trueSizeOfTier) {
		printf("Tier is all primitives! No loopy algorithm needed!\n");
		return;
	}
	// SET UP FRONTIER!
	printf("--Doing an sweep of child tiers, and setting up the frontier...\n");
	for (pos = gCurrentTierSize; pos < gNumberOfPositions; pos++) {
        if (!useUndo && rParents[pos] == NULL) // if we didn't even see this child, don't put it on frontier!
            continue;
		if (gSymmetries) // use the canonical position's values
			canonPos = gCanonicalPosition(pos); //to tell where i go!
		else canonPos = pos; // else ignore
		value = GetValueOfPosition(canonPos);
		remoteness = Remoteness(canonPos);
		if (!((value == tie && remoteness == REMOTENESS_MAX)
				|| value == undecided))
			rInsertFR(value, pos, remoteness);
	}
	printf("\n--Beginning the loopy algorithm...\n");
	REMOTENESS r; POSITIONLIST* list;
	printf("--Processing Lose/Win Frontiers!\n");
	for (r = 0; r <= REMOTENESS_MAX; r++) {
        if (r!=REMOTENESS_MAX) {
            list = rRemoveFRList(lose,r);
            if (list != NULL)
                LoopyParentsHelper(list, win, r);
        }
        if (r!=0) {
            list = rRemoveFRList(win,r-1);
            if (list != NULL)
                LoopyParentsHelper(list, lose, r-1);
        }
	}
	printf("Amount now solved: %lld (%.1f%c)\n",numSolved, 100*(double)numSolved/trueSizeOfTier, '%');
	if (numSolved == trueSizeOfTier)
		return;// Else, we must process ties!
	printf("--Processing Tie Frontier!\n");
    for (r = 0; r < REMOTENESS_MAX; r++) {
        list = rRemoveFRList(tie,r);
        if (list != NULL)
		    LoopyParentsHelper(list, tie, r);
    }

	printf("Amount now solved: %lld (%.1f%c)\n",numSolved, 100*(double)numSolved/trueSizeOfTier, '%');
	if (numSolved == trueSizeOfTier)
		return;// Else, we have undecideds... must make them DRAWs
	printf("--Setting undecided to DRAWs...\n");
	for(pos = 0; pos < gCurrentTierSize; pos++) {
		if (childCounts[pos] > 0) { // no lose/tie children, no/some wins = draw
			SetRemoteness(pos,REMOTENESS_MAX); // a draw
			StoreValueOfPosition(pos, tie);
			numSolved++;
		}
	}
	assert(numSolved == trueSizeOfTier);
}

void LoopyParentsHelper(POSITIONLIST* list, VALUE valueParents, REMOTENESS remotenessChild) {
	POSITION child, parent;
	FRnode *miniLoseFR = NULL;
	UNDOMOVELIST *parents, *parentsPtr;
	POSITIONLIST *parentList;
	for (; list != NULL; list = list->next) {
		child = list->position;
		if (useUndo) { // use the UndoMove lists
			parents = parentsPtr = gGenerateUndoMovesToTierFunPtr(child, gCurrentTier);
			for (; parentsPtr != NULL; parentsPtr = parentsPtr->next) {
				parent = gUnDoMoveFunPtr(child, parentsPtr->undomove);
				if (parent < 0 || parent >= gCurrentTierSize) {
					TIERPOSITION tp; TIER t;
					gUnhashToTierPosition(parent, &tp, &t);
					printf("ERROR: %llu generated undo-parent %llu (Tier: %llu, TierPosition: %llu),\n"
							"which is not in the current tier being solved!\n", child, parent, t, tp);
					ExitStageRight();
				}
				// if childCounts is already 0, we don't mess with this parent
				// (already dealt with OR illegal)
				if (childCounts[parent] != 0) {
					// With losing children, every parent is winning, so we just go through
					// all the parents and declare them winning.
					// Same with tie children.
					if (valueParents == win || valueParents == tie) {
						childCounts[parent] = 0; // reset child counter
						rInsertFR(valueParents, parent, remotenessChild+1);
					// With winning children, first decrement the child counter by one. If
					// child counter reaches 0, put the parent not in the FR but in the miniFR.
					} else if (valueParents == lose) {
						childCounts[parent] -= 1;
						if (childCounts[parent] != 0) continue;
						miniLoseFR = StorePositionInList(parent, miniLoseFR);
					}
					SetRemoteness(parent, remotenessChild+1);
					StoreValueOfPosition(parent, valueParents);
					numSolved++;
				}
			}
			FreeUndoMoveList(parents);
		} else { // use the parents pointers
			parentList = rParents[child];
			for (; parentList != NULL; parentList = parentList->next) {
				parent = parentList->position;
				if (childCounts[parent] != 0) {
					if (valueParents == win || valueParents == tie) {
						childCounts[parent] = 0;
						rInsertFR(valueParents, parent, remotenessChild+1);
					} else if (valueParents == lose) {
						childCounts[parent] -= 1;
						if (childCounts[parent] != 0) continue;
						miniLoseFR = StorePositionInList(parent, miniLoseFR);
					}
					SetRemoteness(parent, remotenessChild+1);
					StoreValueOfPosition(parent, valueParents);
					numSolved++;
				}
			}
		}
		// if we inserted into LOSE, deal with them now
		if (valueParents == lose && miniLoseFR != NULL) {
			LoopyParentsHelper(miniLoseFR, win, remotenessChild+1); // will be freed here too
			miniLoseFR = NULL;
		}
	}
    FreePositionList(list); // no longer need it!
}


/************************************************************************
**
** SANITY CHECKERS
**
************************************************************************/

// correctness checker
void checkForCorrectness(POSITION start, POSITION end) {
	BOOLEAN check = TRUE;
	POSITION pos, child;
	REMOTENESS maxWinRem, minLoseRem, minTieRem;
	BOOLEAN seenLose, seenTie, okay;
	MOVELIST *moves, *children;
	VALUE value, valueP, valueC; REMOTENESS remoteness, remotenessC;
	for (pos = start; pos < end; pos++) {
		value = GetValueOfPosition(pos);
		if (value == undecided) {
			if ((checkLegality && !gIsLegalFunPtr(pos)) ||
				(gSymmetries && pos != gCanonicalPosition(pos)))
				continue; // correct to be undecided
			else {
				printf("CORRUPTION: (%llu) is UNDECIDED, but shouldn't be!\n", pos);
				check = FALSE;
			}
		}
		remoteness = Remoteness(pos);
		valueP = Primitive(pos);

		if (remoteness == 0) {// better be a primitive!
			if (valueP == undecided) {
				printf("CORRUPTION: (%llu) is a non-Primitive with Remoteness 0!\n", pos);
				check = FALSE;
			} else if (value != valueP) {
				printf("CORRUPTION: (%llu) is Primitive with value %s, but db says %s!\n",
						pos, gValueString[(int)valueP], gValueString[(int)value]);
				check = FALSE;
			}
		} else {
			if (valueP != undecided) {
				printf("CORRUPTION: (%llu) is a Primitive with Remoteness %d, not 0!\n",
						pos, remoteness);
				check = FALSE;
			} else {
				moves = children = GenerateMoves(pos);
				if (moves == NULL) { // no children!
					printf("CORRUPTION: (%llu) has no GenerateMoves, yet is a %s in %d!\n",
							pos, gValueString[(int)value], remoteness);
					check = FALSE;
				} else { //the REALLY annoying part, actually checking the children:
					minLoseRem = minTieRem = REMOTENESS_MAX;
					maxWinRem = 0;
					seenLose = seenTie = FALSE; okay = TRUE;
					for (; children != NULL; children = children->next) {
						child = DoMove(pos, children->move);
						if (gSymmetries)
							child = gCanonicalPosition(child);
						valueC = GetValueOfPosition(child);
						if (valueC != undecided) {
							remotenessC = Remoteness(child);
							if (valueC == tie) { //this COULD be tie OR draw
								seenTie = TRUE;
								if (remotenessC < minTieRem)
									minTieRem = remotenessC;
							} else if (valueC == lose) {
								seenLose = TRUE;
								if (remotenessC < minLoseRem)
									minLoseRem = remotenessC;
							} else if (valueC == win) {
								if (remotenessC > maxWinRem)
									maxWinRem = remotenessC;
							}
						} else {
							printf("CORRUPTION: (%llu) has UNDECIDED child, (%llu)!\n", pos, child);
							check = okay = FALSE;
						}
					}
					FreeMoveList(moves);
					if (okay) { // No undecided children
						if (seenLose) { // better be WIN!
							if (value != win || remoteness != minLoseRem+1) {
								printf("CORRUPTION: (%llu) SHOULD be a %s in %d, but it is a %s in %d!\n",
									pos, gValueString[(int)win], minLoseRem+1, gValueString[(int)value], remoteness);
								check = FALSE;
							}
						} else if (seenTie) {
							if (minTieRem == REMOTENESS_MAX) {// a draw
								if (value != tie || remoteness != minTieRem) {
									printf("CORRUPTION: (%llu) SHOULD be a Draw, but it is a %s in %d!\n",
										pos, gValueString[(int)value], remoteness);
									check = FALSE;
								}
							} else {// a tie
								if (value != tie || remoteness != minTieRem+1) {
									printf("CORRUPTION: (%llu) SHOULD be a %s in %d, but it is a %s in %d!\n",
										pos, gValueString[(int)tie], minTieRem+1, gValueString[(int)value], remoteness);
									check = FALSE;
								}
							}
						} else { // better be LOSE!
							if (value != lose || remoteness != maxWinRem+1) {
								printf("CORRUPTION: (%llu) SHOULD be a %s in %d, but it is a %s in %d!\n",
									pos, gValueString[(int)lose], maxWinRem+1, gValueString[(int)value], remoteness);
								check = FALSE;
							}
						}
					}
				}
			}
		}
	}
	if (check)
		printf("Congratulations! No Corruption found!\n");
	else printf("There appears to be some corruption...\n");
}

/* TIER LIST GENERATION SECTION */

// a hashtable datastructure of tiernodes
typedef struct tiernode_list
{
        TIER tier;
        BOOLEAN solved;
        struct tiernode_list* next;
}
TIERNODELIST;

void FreeTierNodeList(TIERNODELIST* ptr){
    TIERNODELIST *last;
    while (ptr != NULL) {
        last = ptr;
        ptr = ptr->next;
        SafeFree((GENERIC_PTR)last);
    }
}

TIERNODELIST *CreateTierNodelistNode(TIER theTier, TIERNODELIST* theNextTier){
    TIERNODELIST *theHead = (TIERNODELIST *) SafeMalloc (sizeof(TIERNODELIST));
    theHead->tier = theTier;
    theHead->solved = FALSE;
    theHead->next = theNextTier;
    return theHead;
}

TIERLIST *list = NULL; // the list to be generated
TIERNODELIST** nodesHashTable = NULL; // a hashtable to keep track of nodes

void initDFSStuff() {
    list = NULL;
	nodesHashTable = (TIERNODELIST**) SafeMalloc(HASHTABLE_BUCKETS * sizeof(TIERNODELIST*));
	int i;
	for (i = 0; i < HASHTABLE_BUCKETS; i++)
		nodesHashTable[i] = NULL;
}

void freeDFSStuff() {
	FreeTierList(list); 
	int i;
	for (i = 0; i < HASHTABLE_BUCKETS; i++)
		FreeTierNodeList(nodesHashTable[i]);
	if (nodesHashTable != NULL)
		SafeFree(nodesHashTable);
}

// puts a node into the hashtable, returns the node pointer
// invariant: node NOT in hashtable already
TIERNODELIST* putNode(TIER tier) {
    int tierBucket = tier % HASHTABLE_BUCKETS;
	nodesHashTable[tierBucket] =
		CreateTierNodelistNode(tier, nodesHashTable[tierBucket]);
    return nodesHashTable[tierBucket];
}

// gets node from a hashtable
// if solved, return 1, else return 0
// if the node isn't there, return -1
int getNode(TIER tier) {
	TIERNODELIST* bucket = nodesHashTable[tier % HASHTABLE_BUCKETS];
    for (; bucket != NULL; bucket = bucket->next) {
        if (tier == bucket->tier) {
			if (bucket->solved)
                return 1;
            else return 0;
        }
    }
	return -1;
}

// the actual DFS traverser
BOOLEAN tierDFS(TIER tier, BOOLEAN defineList) {
	TIERLIST *children, *cptr;
	TIER child;
	int childResult;

	TIERNODELIST* node = putNode(tier);

	children = cptr = gTierChildrenFunPtr(tier);
	for (; cptr != NULL; cptr = cptr->next) {
		child = cptr->tier;
		if (tier == child) continue;

        childResult = getNode(child);
        // if child not visited, DFS
		if (childResult == -1) {
            if (!tierDFS(child, defineList)) {
                FreeTierList(children);
                return FALSE;
            }
		// else check if there's a cycle
		} else if (childResult == 0) {
			printf("ERROR! Tier %llu leads back to higher Tier %llu!\n", child, tier);
            FreeTierList(children);
			return FALSE;
		}
	}
	FreeTierList(children);
	node->solved = TRUE;
	if (defineList)
		list = CreateTierlistNode(tier, list);
	return TRUE;
}

// NOW, the actual API functions for checking/generating the tier solve list

// Check tier hierarchy AND define tierSolveList
TIERLIST* checkAndDefineTierTree() {
    initDFSStuff();
	BOOLEAN check = tierDFS(gInitialTier, TRUE);
	TIERLIST* toReturn = CopyTierlist(list);
	freeDFSStuff();
	if (check)
		return toReturn;
	else return NULL;
}

// ONLY check tier hierarchy
BOOLEAN checkTierTree() {
    initDFSStuff();
	BOOLEAN check = tierDFS(gInitialTier, FALSE);
	freeDFSStuff();
	return check;
}

/************************************************************************
**
** HAXX Checkers
**
************************************************************************/

// The following are just helpers I use to check the correctness
// of the solver, vs. the normal loopy solver.

/* HAXX that writes database to file, usually placed in module's "ValidTextInput()" */
void writeCurrentDBToFile() {
	FILE *fp; POSITION p;
	fp = fopen("./data/a.txt","w");
	for (p = 0; p < gNumberOfPositions; p++)
		fprintf (fp, "%d\t%lld\t%d\n", GetValueOfPosition(p), p, Remoteness(p));
	fclose(fp);
}

// Here's one that writes it for all tiers:
void writeTierDBsToFile() {
	TIERLIST *list = checkAndDefineTierTree();
	FILE *fp; POSITION p;
    char filename[80];
    TIER tier; TIERLIST *ptr;
    for (ptr = list; ptr != NULL; ptr = ptr->next) {
        tier = ptr->tier;
        printf("Writing DB for Tier %llu...\n", tier);
        sprintf(filename, "./data/b_%llu.txt", tier);
	    fp = fopen(filename,"w");
        gInitializeHashWindow(tier, TRUE);
	    for (p = 0; p < gCurrentTierSize; p++)
		    fprintf (fp, "%d\t%lld\t%d\n", GetValueOfPosition(p), p, Remoteness(p));
	    fclose(fp);
    }
    FreeTierList(list);
}

/* // wrote this for Tic Tac Tier - should work for most games
void writeCurrentDBToTierFiles() {
	if (gHashWindowInitialized) {
		FILE *fp; POSITION p, p2;

  		int game[10] = { o, 0, 4, x, 0, 5, Blank, 0, 9, -1 };
  		POSITION globalPos = generic_hash_init(BOARDSIZE, game, vcfg, 1);

		VALUE* dbv = (VALUE*) SafeMalloc(globalPos * sizeof(VALUE));
		REMOTENESS* dbr = (REMOTENESS*) SafeMalloc(globalPos * sizeof(REMOTENESS));
		for (p = 0; p < globalPos; p++)
			dbv[p] = dbr[p] = 0;
		int i, GlobalContext = generic_hash_cur_context();

		for (i = 0; i <= 9; i++) {
			char *board;
			printf("Tier %llu, with %llu...\n", i, gNumberOfTierPositionsFunPtr(i));
			gInitializeHashWindow(i, TRUE);
			for (p = 0; p < gNumberOfTierPositionsFunPtr(i); p++) {
				generic_hash_context_switch(Tier0Context+i);
				board = PositionToBlankOX(p);
				generic_hash_context_switch(GlobalContext);
				gHashWindowInitialized = FALSE;
				p2 = BlankOXToPosition(board);
				gHashWindowInitialized = TRUE;
				dbv[p2] = GetValueOfPosition(p);
				dbr[p2] = Remoteness(p);
			}
		}
		fp = fopen("./a.txt","w");
		for (p = 0; p < globalPos; p++)
			fprintf (fp, "%d\t%lld\t%d\n", dbv[p], p, dbr[p]);
		fclose(fp);
		SafeFree(dbv); SafeFree(dbr);
	} else writeCurrentDBToFile();
}*/

/* Here's the Bagh Chal version:
void writeCurrentDBToTierFiles() {
	if (gHashWindowInitialized) {
		FILE *fp; POSITION p, p2;

		int game[10] = {TIGER, tigers, tigers, GOAT, 0, goats, SPACE, boardSize - tigers - goats, boardSize - tigers, -1};
		genericHashMaxPos = generic_hash_init(boardSize, game, vcfg_board, 0);
		POSITION globalPos = genericHashMaxPos * (goats + 1);

		VALUE* dbv = (VALUE*) SafeMalloc(globalPos * sizeof(VALUE));
		REMOTENESS* dbr = (REMOTENESS*) SafeMalloc(globalPos * sizeof(REMOTENESS));
		for (p = 0; p < globalPos; p++)
			dbv[p] = dbr[p] = 0;
		int i, GlobalContext = generic_hash_cur_context();

		TIERLIST* list = gTierSolveListPtr;
		for (; list != NULL; list = list->next) {
			i = list->tier;
			int goatsOnBoard, gL, t;
			unhashTier(i, &goatsOnBoard, &gL, &t);
			char *board;
			printf("Tier %llu, with %llu...\n", i, gNumberOfTierPositionsFunPtr(i));
			gInitializeHashWindow(i, TRUE);
			for (p = 0; p < gNumberOfTierPositionsFunPtr(i); p++) {
				generic_hash_context_switch(Tier0Context+goatsOnBoard);
				int turn, goatsLeft;
				board = unhash(p, &turn, &goatsLeft);
				generic_hash_context_switch(GlobalContext);
				gHashWindowInitialized = FALSE;
				p2 = hash(board, turn, goatsLeft);
				gHashWindowInitialized = TRUE;
				dbv[p2] = GetValueOfPosition(p);
				dbr[p2] = Remoteness(p);
			}
		}
		fp = fopen("./a.txt","w");
		for (p = 0; p < globalPos; p++)
			fprintf (fp, "%d\t%lld\t%d\n", dbv[p], p, dbr[p]);
		fclose(fp);
		SafeFree(dbv); SafeFree(dbr);
	} else writeCurrentDBToFile();
}*/

// a helper that compares two database outfiles
void compareTwoFiles(char *mine, char *theirs, BOOLEAN exact) {
	FILE *my, *his;
	BOOLEAN errors = FALSE;
	if ((my = fopen(mine, "r")) == NULL) {
		printf("Couldn't open %s\n", mine);
		return;
	}
	if ((his = fopen(theirs, "r")) == NULL) {
		printf("Couldn't open %s\n", theirs);
		return;
	}
	int c, line = 0, efficiency = 0;
	while ((c = getc(his)) != EOF) {
		if (!exact && c == '0') {
			skipToNewline(his);
			if (getc(my) == c)
				efficiency++;
			skipToNewline(my);
		} else {
			efficiency++;
			if (getc(my) != c) {
				printf("%d\n",line);
				errors = TRUE;
				skipToNewline(my);
				skipToNewline(his);
			} else {
				while (c != '\n') {
					if (getc(my) != (c = getc(his))) {
						printf("%d\n",line);
						errors = TRUE;
						skipToNewline(my);
						skipToNewline(his);
						break;
					}
				}
			}
		}
		line++;
	} if ((c = getc(my)) != EOF) printf("Files don't match up!\n");
	else if (!errors) {
		printf("FILES ARE THE SAME, BABY!!!\n");
		if (!exact) printf("My Efficiency: %.1f\n", 100*(double)efficiency/(double)line);
	}
}

void skipToNewline(FILE* fp) {
	int c = getc(fp);
	while (c != '\n') c = getc(fp);
}


/************************************************************************
**
** ALL THE PARALLELIZATION STUFF
**
************************************************************************/

/***LEVEL FILES***/

void solveLevelFiles() {

}

/***ODEEPA BLUE FUNCTIONS***/

// This is a simple UI for testing the parallelize stuff. All it REALLY
// does is call the functions.

void TestRemote() {
    BOOLEAN cont = TRUE;
    TIERLIST *list, *ptr;
    TIERPOSITION tier, start, finish;
    int comps, i;
    POSITION end, *starts;
	char c;
    RemoteInitialize();
    printf("\nWELCOME TO THE (not really) PARALLELIZED SOLVER INTERFACE!");
    while(cont) {
        printf("\n\n\t----- PARALLEL SOLVER MENU -----\n"
               "\t1)\t(1)GetTierSolveOrder()\n"
               "\t2)\t(2)GetTierSize(TIER)\n"
               "\t3)\t(3)GetTierDependencies(TIER)\n\n"
               "\t4)\t(4)CanISolveTier(TIER)\n"
               "\t5)\t(5)SolveTier(TIER,START,FINISH)\n\n"
               "\t6)\t(6)IsTierReadyToMerge(TIER)\n"
               "\t7)\t(7)MergeToMakeTierDB(TIER)\n\n"
               "\tC)\t(C)alculate Next Tiers\n"
               "\tS)\t(S)imulate ODeepaBlue\n\n"
               "\tq)\t(Q)uit using the interface\n"
               "\nSelect an option:  ");
        c = GetMyChar();
        switch(c) {
			case '1':
                printf("TIER: > ");
	            list = RemoteGetTierSolveOrder();
                printf("Returned:");
	            for (; list != NULL; list = list->next)
		            printf(" %llu", list->tier);
                FreeTierList(list);
				break;
			case '2':
                printf("TIER: > ");
                tier = GetMyPosition();
                if (tier == kBadPosition) break;
                printf("Returned: %llu", RemoteGetTierSize(tier));
				break;
			case '3':
                printf("TIER: > ");
                tier = GetMyPosition();
                if (tier == kBadPosition) break;
                printf("Returned: %d", RemoteGetTierDependencies(tier));
				break;
			case '4':
                printf("TIER: > ");
                tier = GetMyPosition();
                if (tier == kBadPosition) break;
                printf("Returned: %d", RemoteCanISolveTier(tier));
				break;
			case '5':
                printf("TIER: > ");
                tier = GetMyPosition();
                if (tier == kBadPosition) break;
                printf("START: > ");
                start = GetMyPosition();
                if (start == kBadPosition) break;
                printf("FINISH: > ");
                finish = GetMyPosition();
                if (finish == kBadPosition) break;
                RemoteSolveTier(tier, start, finish);
                printf("Returned.");
				break;
			case '6':
                printf("TIER: > ");
                tier = GetMyPosition();
                if (tier == kBadPosition) break;
                printf("Returned: %d", RemoteIsTierReadyToMerge(tier));
				break;
			case '7':
                printf("TIER: > ");
                tier = GetMyPosition();
                if (tier == kBadPosition) break;
                printf("Returned: %d", RemoteMergeToMakeTierDB(tier));
				break;
            case 'c': case 'C':
                list = RemoteGetTierSolveOrder();
                printf("Here's the tiers that can currently be solved:\n");
                for (ptr = list; ptr != NULL; ptr = ptr->next) {
                    if (RemoteCanISolveTier(ptr->tier))
                        printf("  Tier %llu, size %llu with %d dependencies\n", ptr->tier,
                                RemoteGetTierSize(ptr->tier), RemoteGetTierDependencies(ptr->tier));
                }
                FreeTierList(list);
                break;
            case 's': case 'S':
                printf("Number of CPUs: > ");
                tier = GetMyPosition();
                if (tier == kBadPosition) break;
                comps = (int)tier;
                printf("Simulating a run for the following tiers:\n> ");
                list = NULL;
                starts = NULL;
                i = 0;
                while((tier = GetMyPosition()) != kBadPosition) {
                    if (TierInList(tier, tierSolveList)) {
                        list = CreateTierlistNode(tier, list);
                        i++;
                    }
                    printf("> ");
                }
                if (list == NULL) {
                    printf("No tiers to run it for!\n");
                    break;
                }
                starts = (POSITION*) SafeMalloc(i * sizeof(POSITION));
                for (tier = 0; tier < i; tier++)
                    starts[tier] = 0;
                // now simulate the run:
                for (i = 0; i < comps; i++) {
                    tier = 0;
                    for (ptr = list; ptr != NULL; ptr = ptr->next, tier++) {
                        end = (RemoteGetTierSize(ptr->tier)*(i+1))/comps;
                        RemoteSolveTier(ptr->tier, starts[tier], end);
                        printf("CPU %d solved Tier %llu, from %llu to %llu\n", i+1, ptr->tier, starts[tier], end);
                        starts[tier] = end;
                    }
                }
                // merging
                printf("Merge: > ");
                GetMyPosition();
                for (ptr = list; ptr != NULL; ptr = ptr->next, tier++)
                    RemoteMergeToMakeTierDB(ptr->tier);
                FreeTierList(list);
                SafeFree(starts);
                break;
            case 'q': case 'Q':
                ExitStageRight();
            default:
                printf("Invalid option!\n");
        }
    }
}


// should only be called once
void RemoteInitialize() {
	solveList = checkAndDefineTierTree();
	tierSolveList = CopyTierlist(solveList); // this will actually never be changed
	solvedList = NULL; // so this will stay null forever
	numTiers = 1; tiersSolved = 0;
    gDBLoadMainTier = FALSE; // just in case

	variant = getOption();
	tierNames = checkLegality = forceLoopy = checkCorrectness = FALSE;
	useUndo = TRUE;
	if (gGenerateUndoMovesToTierFunPtr == NULL || gUnDoMoveFunPtr == NULL) {
		useUndo = FALSE;
	}

    gDBLoadMainTier = FALSE; // initialize main tier as undecided rather than load
}

// get the tierlist, in proper solve order:
TIERLIST* RemoteGetTierSolveOrder() {
	return CopyTierlist(tierSolveList);
}

// this tells you, given the current files, if you can solve this tier.
// for simplicity, if tier is ALREADY solved, returns false
BOOLEAN RemoteCanISolveTier(TIER tier) {
    if (!TierInList(tier, tierSolveList) || CheckTierDB(tier, variant) == 1)
        return FALSE;
	TIERLIST* childs = gTierChildrenFunPtr(tier);
	TIERLIST* ptr;
	for (ptr = childs; ptr != NULL; ptr = ptr->next) {
		if (ptr->tier == tier) continue;
		else if (CheckTierDB(ptr->tier, variant) != 1) {
			FreeTierList(childs);
			return FALSE;
		}
	}
	FreeTierList(childs);
	return TRUE;
}

// get the number of positions in this tier
TIERPOSITION RemoteGetTierSize(TIER tier) {
    if (!TierInList(tier, tierSolveList))
        return -1;
	return gNumberOfTierPositionsFunPtr(tier);
}

// this tells the number of dependencies this tier has
int RemoteGetTierDependencies(TIER tier) {
	TIERLIST *ptr, *childs, *childPtr;
	int dependencies = 0;
	for (ptr = solveList; ptr != NULL; ptr = ptr->next) {
        if (ptr->tier == tier) continue;
		childs = gTierChildrenFunPtr(ptr->tier);
		for (childPtr = childs; childPtr != NULL; childPtr = childPtr->next) {
			if (childPtr->tier == tier) {
				dependencies++;
				break;
			}
		}
		FreeTierList(childs);
	}
	return dependencies;
}

// new DetermineRetrogradeValue, solves [ start, finish )
void RemoteSolveTier(TIER tier, TIERPOSITION start, TIERPOSITION finish) {
    // two sanity checkers, to check if we actually solve
    // first can I actually solve this tier?
    // second, are the args correct?
    if (!RemoteCanISolveTier(tier) || start < 0 || start >= finish
            || finish > gNumberOfTierPositionsFunPtr(tier))
        return;
	gInitializeHashWindow(tier, TRUE);
	PercentDone(Clean); //reset percentage bar
	SolveTier(start, finish);
}

BOOLEAN RemoteIsTierReadyToMerge(TIER tier) {
    if (!TierInList(tier, tierSolveList))
        return FALSE;
	char directory[MAXINPUTLENGTH];
	char* filename;
    sprintf(directory,"./data/m%s_%d_tierdb",kDBName,variant);
    // init the hash window
    gInitializeHashWindow(tier, TRUE);
	// keep an array of visited nums
	BOOLEAN* seen = (BOOLEAN*) SafeMalloc(gCurrentTierSize * sizeof(BOOLEAN));
	int i;
	for (i = 0; i < gCurrentTierSize; i++)
		seen[i] = FALSE;
    // check through all the minifiles
	struct dirent *dp;
	DIR *dfd = opendir(directory);
	if(dfd != NULL) {
		while((dp = readdir(dfd)) != NULL) {
			filename = dp->d_name;
			r_getBounds(tier, filename);
			if (gDBTierStart != -1 && tierdb_load_minifile(filename)) { // is a correct minitierdb!
				for (i = gDBTierStart; i < gDBTierEnd; i++)
					seen[i] = TRUE;
			}
		}
		closedir(dfd);
	}
	for (i = 0; i < gCurrentTierSize; i++) {
		if (!seen[i]) // don't have all the files!
            return FALSE;
	}
	return TRUE;
}

// calls RemoteIsTierReadyToMerge() - yes, repeats work, but a good sanity check!
BOOLEAN RemoteMergeToMakeTierDB(TIER tier) {
    if (!RemoteIsTierReadyToMerge(tier)) // sets up whole database!
        return FALSE;
    // finally, save this DB as a complete DB
    gDBTierStart = -1;
    gDBTierEnd = -1;
    if (!SaveDatabase())
        return FALSE;
    // now, we delete all minifiles and return true
    char directory[MAXINPUTLENGTH], removeFile[MAXINPUTLENGTH];
    char* filename;
    sprintf(directory,"./data/m%s_%d_tierdb",kDBName,variant);
    struct dirent *dp;
    DIR *dfd = opendir(directory);
    if(dfd != NULL) {
        while((dp = readdir(dfd)) != NULL) {
            filename = dp->d_name;
            r_getBounds(tier, filename);
            if (gDBTierStart != -1) { // is a minitierdb!
                sprintf(removeFile, "%s/%s", directory, filename);
                remove(removeFile);
            }
        }
        closedir(dfd);
    }
    return TRUE;
}

// HELPERS

// this is a helper that parses the mini-tierdb filename for info
void r_getBounds(TIER tier, char* name) {
    gDBTierStart = -1;
    int i = 0;
    char* str;
    char startStr[MAXINPUTLENGTH], endStr[MAXINPUTLENGTH];
    BOOLEAN check;
    // WARNING: ugly parsing code below!
    // form: m(kDBName)_(variant)_(tier)__(start)_(end)_minitierdb.dat.gz
    // check m
    if (r_checkChar('m','m',name,&i)) return;
    // check kDBName
    if (r_checkStr(kDBName,name,&i)) return;
    // check _
    if (r_checkChar('_','_',name,&i)) return;
    // check variant
    str = r_intToString(variant);
    check = r_checkStr(str,name,&i);
    SafeFree(str);
    if (check) return;
    // check _
    if (r_checkChar('_','_',name,&i)) return;
    // check tier
    str = r_intToString(tier);
    check = r_checkStr(str,name,&i);
    SafeFree(str);
    if (check) return;
    // check __
    if (r_checkChar('_','_',name,&i)) return;
    if (r_checkChar('_','_',name,&i)) return;
    // check start
    int index = 0;
    while (name[i] >= '0' && name[i] <= '9') {
		startStr[index] = name[i];
		index++; i++;
	}
	startStr[index] = '\0';
    int start = atoi(startStr);
    // check _
    if (r_checkChar('_','_',name,&i)) return;
    // check end
    index = 0;
    while (name[i] >= '0' && name[i] <= '9') {
		endStr[index] = name[i];
		index++; i++;
	}
	endStr[index] = '\0';
    int end = atoi(endStr);
    if (start < 0 || start >= end || end > gCurrentTierSize) return;
    // check _minitierdb.dat.gz
    if (r_checkStr("_minitierdb.dat.gz",name,&i)) return;
    // sucess! set the vars and return
    gDBTierStart = start;
    gDBTierEnd = end;
}

// helps above, tells if ch[i] is NOT in [chStart, chEnd], and auto-advances i
BOOLEAN r_checkChar(char chStart, char chEnd, char* str, int* i) {
    BOOLEAN result = !(str[(*i)] >= chStart && str[(*i)] <= chEnd);
    (*i) = (*i) + 1;
    return result;
}

// also helps above, uses checkChar on a whole string
// (so, checks that it's NOT equal)
BOOLEAN r_checkStr(char* check, char* str, int* i) {
	int j;
	for (j = 0; j < strlen(check); j++) {
        if (r_checkChar(check[j],check[j],str,i))
	        return TRUE;
    }
    return FALSE;
}

// yet another helper, converts a positive int to
// a string in a very elegant fashion
char* r_intToString(int n) {
	if (n < 0) // just in case
		n = -n;
	// we need to find out how many digits it is:
	int tmp = n, digits = 0;
	while (tmp != 0) {
		tmp /= 10;
		digits++;
	}
	char* str = (char*) SafeMalloc(digits+1 * sizeof(char));
	sprintf(str, "%d", n);
	return str;
}
