// $Id: solveretrograde.c,v 1.24 2006-10-17 10:45:22 max817 Exp $

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

#include <zlib.h>
#include <netinet/in.h>
#include "gamesman.h"
#include "solveretrograde.h"

/* Global variables */
int variant;
char filename[80];
TIER tierToSolve, tiersSoFar;
POSITION tierSize;
BOOLEAN tierNames, checkLegality, useUndo, forceLoopy, checkCorrectness;
TIERLIST* tierSolveList = NULL;

// Solver procs
void checkExistingDB();
void PrepareToSolveNextTier();
BOOLEAN setInitialTierPosition();
POSITION GetMyPosition();
BOOLEAN ConfirmAction(char);
// Solver Heart
void SolveTier();
void SolveWithNonLoopyAlgorithm();
void SolveWithLoopyAlgorithm();
void LoopyParentsHelper(POSITIONLIST*, VALUE, REMOTENESS);
// Solver ChildCounter and Hashtable functions
void rInitFRStuff();
void rFreeFRStuff();
POSITIONLIST* rRemoveFRList(VALUE, REMOTENESS);
void rInsertFR(VALUE, POSITION, REMOTENESS);
// Sanity Checkers
void checkForCorrectness();
BOOLEAN checkAndDefineTierTree();
void clearNodes();
BOOLEAN tierDFS(TIER, BOOLEAN);
// Debug Stuff
void debugMenu();
// HAXX for comparing two databases
void writeCurrentDBToFile();
void compareTwoFiles(char*, char*);
void skipToNewline(FILE*);

/************************************************************************
**
** NAME:        DetermineRetrogradeValue
**
** DESCRIPTION: Called by Gamesman, the solver goes into a menu
**				rather than just solving.
**
************************************************************************/

VALUE DetermineRetrogradeValue(POSITION position) {
	variant = getOption();
	STRING tierStr;
	BOOLEAN cont = TRUE, isLegalGiven = TRUE, undoGiven = TRUE;
	TIER numTiers;
    char c;
    checkLegality = useUndo = checkCorrectness = FALSE;
    forceLoopy = FALSE;
    tierNames = (gTierToStringFunPtr != NULL);

	printf("\n\n=====Welcome to the TIER-GAMESMAN Retrograde Solver!=====\n");
	printf("Currently solving game (%s) with variant (%d)\n", kGameName, variant);
    printf("Initial Position: %llu, in Initial Tier: %d\n", gInitialTierPosition, gInitialTier);

	printf("\n-----Checking the REQUIRED API Functions:-----\n\n");

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

	/*if (kDebugTierMenu) {
		printf("\nRedirecting to the Debug Menu...\n");
		debugMenu();
		printf("Exiting Retrograde Solver (WITHOUT Solving)...\n");
		exit(0);
	}*/

	// make gTierSolveList
	if (!checkAndDefineTierTree()) {
		printf("\nPlease fix gTierChildren before attempting to solve!\n"
				"Exiting Retrograde Solver (WITHOUT Solving)...\n");
		exit(0);
	} else {
		printf("-Tier Solve Order:\n");
		TIERLIST* ptr = tierSolveList;
		numTiers = 0;
		for (; ptr != NULL; ptr = ptr->next) {
			printf("%d ", ptr->tier);
			if (tierNames) {
				tierStr = gTierToStringFunPtr(ptr->tier);
				printf(": %s\n", tierStr);
				if (tierStr != NULL) SafeFree(tierStr);
			}
			numTiers++;
		}
		printf("\n   %d Tiers are confirmed to be solved.\n", numTiers);
	}


	printf("\n-----Checking the OPTIONAL API Functions:-----\n\n");
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
	if (gTierToStringFunPtr == NULL)
		printf("-TierToString NOT GIVEN\nTier Name Printing Disabled\n");
	if (isLegalGiven && undoGiven && tierNames)
		printf("API Optional Functions Confirmed.\n");


	printf("\n-----Checking for existing Tier DBs:-----\n\n");
	tiersSoFar = 0;

	checkExistingDB();
	if (tierSolveList == NULL) {
		printf("\nLooks like the game is already fully solved! Enjoy the game!\n");
		printf("Exiting Retrograde Solver...\n\n");
		return undecided;
	} else if (tiersSoFar == 0) // No DBs loaded, a fresh solve
		printf("No DBs Found! Starting a fresh solve...\n");

	PrepareToSolveNextTier();
    while(cont) {
        printf("\n\n\t----- RETROGRADE SOLVER MENU for game: %s -----\n", kGameName);
        printf("\tReady to solve %sLOOPY tier %d", (gCurrentTierIsLoopy ? "" : "NON-"), tierToSolve);
        if (tierNames) {
			tierStr = gTierToStringFunPtr(tierToSolve);
			printf(" (%s)", tierStr);
			if (tierStr != NULL) SafeFree(tierStr);
		}
        printf("\n\tThe tier hash contains (%lld) positions.", tierSize);
        printf("\n\tTiers left: %d (%.1f%c Solved)\n\n", numTiers-tiersSoFar, 100*(double)tiersSoFar/numTiers, '%');
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
	            SolveTier();
	            tierSolveList = tierSolveList->next;
	            tiersSoFar++;
	            if (tierSolveList == NULL) {
	            	printf("\n%s is now fully solved!\n", kGameName);
                	cont = FALSE;
                } else PrepareToSolveNextTier();
                break;
            case 'a': case 'A':
            	printf("Fully Solving starting from Tier %d...\n\n",tierToSolve);
            	for (; tierSolveList != NULL; tierSolveList = tierSolveList->next) {
					PrepareToSolveNextTier();
            		SolveTier();
            		tiersSoFar++;
            		printf("\n\n---Tiers left: %d (%.1f%c Solved)", numTiers-tiersSoFar, 100*(double)tiersSoFar/numTiers, '%');
				}
            	printf("\n%s is now fully solved!\n", kGameName);
                cont = FALSE;
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
				compareTwoFiles("./a.txt", "./b.txt");
				break;
            default:
                printf("Invalid option!\n");
        }
    }
	printf("Exiting Retrograde Solver...\n\n");
    return undecided; //just bitter at the fact that this is ignored
}

// copied from memdb_init, checks to see which files are already solved
void checkExistingDB() {
	int FILEVER = 1;
	gzFile* filep;
	short dbVer[1];
	POSITION numPos[1];
	int	goodDecompression;

	for (; tierSolveList != NULL; tierSolveList = tierSolveList->next) {
		sprintf(filename, "./data/m%s_%d_tierdb/m%s_%d_%d_tierdb.dat.gz",
					kDBName, variant, kDBName, variant, tierSolveList->tier);
		if((filep = gzopen(filename, "rb")) == NULL) {
			break;
		} else {
			goodDecompression = gzread(filep,dbVer,sizeof(short));
			goodDecompression = gzread(filep,numPos,sizeof(POSITION));
			*dbVer = ntohs(*dbVer);
			*numPos = ntohl(*numPos);
			if(!goodDecompression || (*numPos != gNumberOfTierPositionsFunPtr(tierSolveList->tier))
				|| (*dbVer != FILEVER)) {
				printf("--%d's Tier DB appears incorrect/corrupted. Re-solving...\n", tierSolveList->tier);
				break;
			}
		}
		gzclose(filep);
		printf("  %d's Tier DB Found!\n", tierSolveList->tier);
		tiersSoFar++;
	}
}

// Inits the hash window/database and prepares to solve tier
void PrepareToSolveNextTier() {
	tierToSolve = tierSolveList->tier;
	printf("\n------Preparing to solve tier: %d\n", tierToSolve);
	gInitializeHashWindow(tierToSolve, TRUE);
	tierSize = gMaxPosOffset[1];
	printf("  Done! Hash Window initialized and Database loaded and prepared!\n");
}

// Alters tierSolveList so that
//solveFirst(TIER tier) {
//	MoveToFrontOfTierlist(tier, tierSolveList);
//}


BOOLEAN setInitialTierPosition() {
	gDBLoadMainTier = TRUE; // trick tierdb into loading main tier temporarily
	TIERPOSITION tp; TIER t;
	VALUE value; REMOTENESS remoteness;
/*	printf("\nYou can choose to start from one of these tiers:\n   ");
	for (tierlist = gTierSolveListPtr; tierlist != NULL; tierlist = tierlist->next) {
		if (tierlist->tier == tierToSolve) {//can't play unsolved tier!
			printf("\n");
			break;
		}
		printf("%d ", tierlist->tier);
	}*/
	while (TRUE) {
		gGetInitialTierPositionFunPtr(&t, &tp);
		// switch to hash window, and final check:
		gInitializeHashWindow(t, TRUE);
		printf("\nYou chose: gInitialTierPosition = %llu, gInitialTier = %d:\n", tp, t);
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
	// switch back to the current hash window:
	gDBLoadMainTier = FALSE;
	gInitializeHashWindow(tierToSolve, TRUE);
	return FALSE;
}

POSITION GetMyPosition() {
    char inString[MAXINPUTLENGTH];
    GetMyStr(inString, MAXINPUTLENGTH);
    POSITION p = 0;
    int i = 0;
	while (inString[i] >= '0' && inString[i] <='9') {
		p = (p*10) + (inString[i]-'0');
		i++;
	}
	if (inString[i] != '\0') {
		return kBadPosition;
	}
	return p;
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
	BOOLEAN check = tierDFS(gInitialTier, FALSE);
	clearNodes();
	if (!check) {
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
-So, first go through the LOSE lists, then WIN lists, then TIE lists.
LOSE and TIE are basically the same; WIN is more interesting. Whenever
we get a win generating LOSEs, note that they're ALL the same remoteness.
So instead of putting them back in into the queue (since we're going to
see all of them next ANYWAY, and it doesn't matter the order) just have
a local "LOSE" list that you go through immediately!
-So it's proven that on an INSERT, you're NEVER going to be inserting
equal to lowestList or currentList.
*/

void rInitFRStuff() {
	//tierSize bytes = (tierSize / 1,028) KB = (tierSize / 1,056,784) GB
	childCounts = (CHILDCOUNT*) SafeMalloc (tierSize * sizeof(CHILDCOUNT));
	//gNumOfPoss bytes = (gNumOfPos / 1,028) KB = (gNumOfPos / 1,056,784) GB <-- this isn't right yet...
	if (!useUndo) {
		rParents = (POSITIONLIST**) SafeMalloc (gNumberOfPositions * sizeof(POSITIONLIST*));
		int i;
		for (i = 0; i < gNumberOfPositions; i++)
			rParents[i] = NULL;
	}
	// 255 * 4 bytes = 1,020 bytes = ~1 KB
	rWinFR = (FRnode**) SafeMalloc (REMOTENESS_MAX * sizeof(FRnode*));
	rLoseFR = (FRnode**) SafeMalloc (REMOTENESS_MAX * sizeof(FRnode*)); // ~1 KB
	rTieFR = (FRnode**) SafeMalloc (REMOTENESS_MAX * sizeof(FRnode*)); // ~1 KB
	int r;
	for (r = 0; r < REMOTENESS_MAX; r++)
		rWinFR[r] = rLoseFR[r] = rTieFR[r] = NULL;
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
	int r;
	// Free the Position Lists
	for (r = 0; r < REMOTENESS_MAX; r++) {
		FreePositionList(rWinFR[r]);
		FreePositionList(rLoseFR[r]);
		FreePositionList(rTieFR[r]);
	}
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
** DESCRIPTION: The heart of the solver. This use the current tier number,
**				then calls the "loopy solver" algorithm
**				to solve the current tier.
**				If non loopy, then it calls the non-loopy solver.
**
************************************************************************/

POSITION numSolved, trueSizeOfTier;

void SolveTier() {
	numSolved = trueSizeOfTier = 0;

	printf("\n-----Solving Tier %d",tierToSolve);
	if (tierNames) {
		STRING tierStr = gTierToStringFunPtr(tierToSolve);
		printf(" (%s)-----\n", tierStr);
		SafeFree(tierStr);
	}
	else printf("-----\n");
	printf("Size of current hash window: %llu\n",gNumberOfPositions);
	printf("Size of tier %d's hash: %llu\n",tierToSolve,tierSize);
	printf("\nSolver Type: %sLOOPY\n",((forceLoopy||gCurrentTierIsLoopy) ? "" : "NON-"));
	printf("Using Symmetries: %s\n",(gSymmetries ? "YES" : "NO"));
	printf("Checking Legality (using IsLegal): %s\n",(checkLegality ? "YES" : "NO"));
	if (forceLoopy || gCurrentTierIsLoopy) {// LOOPY SOLVER
		printf("Using UndoMove Functions: %s\n",(useUndo ? "YES" : "NO"));
		SolveWithLoopyAlgorithm();
		printf("--Freeing Child Counters and Frontier Hashtables...\n");
		rFreeFRStuff();
	} else SolveWithNonLoopyAlgorithm();// NON-LOOPY SOLVER
	printf("\nTier fully solved!\n");
	if (checkCorrectness) {
		printf("--Checking Correctness...\n");
		checkForCorrectness();
	}
	printf("--Saving the Tier Database to File...\n");
	if (SaveDatabase())
		printf ("Database successfully saved!\n");
	else {
		printf("Couldn't save tierDB!\n");
		ExitStageRight();
	}
}

void SolveWithNonLoopyAlgorithm() {
	printf("\n-----PREPARING NON-LOOPY SOLVER-----\n");
	POSITION pos, child;
	MOVELIST *moves, *movesptr;
	VALUE value;
	REMOTENESS remoteness;
	REMOTENESS maxWinRem, minLoseRem, minTieRem;
	BOOLEAN seenLose, seenTie;
	printf("Doing an sweep of the tier, and solving it in one go...\n");
	for (pos = 0; pos < tierSize; pos++) { // Solve only parents
		if (checkLegality && !gIsLegalFunPtr(pos)) continue; //skip
		if (gSymmetries && pos != gCanonicalPosition(pos))
			continue; // skip, since we'll do canon one later
		trueSizeOfTier++;
		value = Primitive(pos);
		if (value != undecided) { // check for primitive-ness
			SetRemoteness(pos,0);
			StoreValueOfPosition(pos,value);
		} else {
			moves = movesptr = GenerateMoves(pos);
			if (moves == NULL) { // no chillins
				SetRemoteness(pos,0);
				StoreValueOfPosition(pos,lose);
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
		printf("--Tier %d's hash efficiency: %.1f%c\n",tierToSolve, 100*(double)trueSizeOfTier/tierSize, '%');
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

void SolveWithLoopyAlgorithm() {
	printf("\n-----PREPARING LOOPY SOLVER-----\n");
	POSITION pos, canonPos, child;
	MOVELIST *moves, *movesptr;
	VALUE value;
	REMOTENESS remoteness;
	printf("--Setting up Child Counters and Frontier Hashtables...\n");
	rInitFRStuff();
	printf("--Doing an sweep of the tier, and setting up the frontier...\n");
	for (pos = 0; pos < tierSize; pos++) { // SET UP PARENTS
		childCounts[pos] = 0;
		if (checkLegality && !gIsLegalFunPtr(pos)) continue; //skip
		if (gSymmetries && pos != gCanonicalPosition(pos))
			continue; // skip, since we'll do canon one later
		trueSizeOfTier++;
		// should check in tierdb that this is all empty (undecided)
		value = Primitive(pos);
		if (value != undecided) { // check for primitive-ness
			SetRemoteness(pos,0);
			StoreValueOfPosition(pos,value);
			numSolved++;
			rInsertFR(value, pos, 0);
		} else {
			moves = movesptr = GenerateMoves(pos);
			if (moves == NULL) { // no chillins
				SetRemoteness(pos,0);
				StoreValueOfPosition(pos,lose);
				numSolved++;
				rInsertFR(lose, pos, 0);
			} else {
				//otherwise, make a Child Counter for it
				movesptr = moves;
				for (; movesptr != NULL; movesptr = movesptr->next) {
					childCounts[pos]++;
					if (!useUndo) {// if parent pointers, add to parent pointer list
						child = DoMove(pos, movesptr->move);
						rParents[child] = StorePositionInList(pos, rParents[child]);
					}
				}
				FreeMoveList(moves);
			}
		}
	}
	if (checkLegality) {
		printf("True size of tier: %lld\n",trueSizeOfTier);
		printf("Tier %d's hash efficiency: %.1f%c\n",tierToSolve, 100*(double)trueSizeOfTier/tierSize, '%');
	}
	printf("Amount now solved (primitives): %lld (%.1f%c)\n",numSolved, 100*(double)numSolved/trueSizeOfTier, '%');
	if (numSolved == trueSizeOfTier) {
		printf("Tier is all primitives! No loopy algorithm needed!\n");
		return;
	}
	// SET UP FRONTIER!
	printf("--Doing an sweep of child tiers, and setting up the frontier...\n");
	for (pos = tierSize; pos < gNumberOfPositions; pos++) {
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
	REMOTENESS r;
	printf("--Processing Lose/Win Frontiers!\n");
	for (r = 0; r <= REMOTENESS_MAX; r++) {
		if (r!=REMOTENESS_MAX) LoopyParentsHelper(rRemoveFRList(lose,r), win, r);
		if (r!=0) LoopyParentsHelper(rRemoveFRList(win,r-1), lose, r-1);
	}
	printf("Amount now solved: %lld (%.1f%c)\n",numSolved, 100*(double)numSolved/trueSizeOfTier, '%');
	if (numSolved == trueSizeOfTier)
		return;// Else, we must process ties!
	printf("--Processing Tie Frontier!\n");
	for (r = 0; r < REMOTENESS_MAX; r++)
		LoopyParentsHelper(rRemoveFRList(tie,r), tie, r);

	printf("Amount now solved: %lld (%.1f%c)\n",numSolved, 100*(double)numSolved/trueSizeOfTier, '%');
	if (numSolved == trueSizeOfTier)
		return;// Else, we have undecideds... must make them DRAWs
	printf("--Setting undecided to DRAWs...\n");
	for(pos = 0; pos < tierSize; pos++) {
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
			parents = parentsPtr = gGenerateUndoMovesToTierFunPtr(child, tierToSolve);
			for (; parentsPtr != NULL; parentsPtr = parentsPtr->next) {
				parent = gUnDoMoveFunPtr(child, parentsPtr->undomove);
				if (parent < 0 || parent >= tierSize) {
					TIERPOSITION tp; TIER t;
					gUnhashToTierPosition(parent, &tp, &t);
					printf("ERROR: %llu generated undo-parent %llu (Tier: %d, TierPosition: %llu),\n"
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
			LoopyParentsHelper(miniLoseFR, win, remotenessChild+1);
			FreePositionList(miniLoseFR);
			miniLoseFR = NULL;
		}
	}
}


/************************************************************************
**
** SANITY CHECKERS
**
************************************************************************/

// correctness checker
void checkForCorrectness() {
	BOOLEAN check = TRUE;
	POSITION pos, child;
	REMOTENESS maxWinRem, minLoseRem, minTieRem;
	BOOLEAN seenLose, seenTie, okay;
	MOVELIST *moves, *children;
	VALUE value, valueP, valueC; REMOTENESS remoteness, remotenessC;
	for (pos = 0; pos < tierSize; pos++) {
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
				// COULD have NULL GenerateMoves
				moves = GenerateMoves(pos);
				if (moves == NULL) continue;
				SafeFree(moves);
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
				if (moves == NULL) { // no children, better be a LOSE!
					if (value != lose || remoteness != 0) {
						printf("CORRUPTION: (%llu) has no GenerateMoves, yet is a %s in %d!\n",
								pos, gValueString[(int)value], remoteness);
						check = FALSE;
					}
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

/*
to find others that can solve at same time: check which have ALL kids solved.
*/
// for my list work

TIERLIST *nodes = NULL,
		*preList = NULL, *postList = NULL; //I use TIER since there's no "BooleanList"

// if tier's in list, returns TRUE and sets variables
// else, return FALSE
BOOLEAN getNode(TIER tier, BOOLEAN* postValue) {
	TIERLIST *nodesPtr = nodes, *postPtr = postList;
	for (; nodesPtr != NULL; nodesPtr = nodesPtr->next, postPtr = postPtr->next)
		if (nodesPtr->tier == tier) {
			(*postValue) = postPtr->tier;
			return TRUE;
		}
	return FALSE;
}

// invariant: tier is not in list
void previsit(TIER tier) {
	nodes = CreateTierlistNode(tier, nodes);
	preList = CreateTierlistNode(1, preList);
	postList = CreateTierlistNode(0, postList);
}

// invariant: tier is already in list
void postvisit(TIER tier) {
	TIERLIST *nodesPtr = nodes, *postPtr = postList;
	for (; nodesPtr != NULL; nodesPtr = nodesPtr->next, postPtr = postPtr->next)
		if (nodesPtr->tier == tier) {
			postPtr->tier = 1;
			return;
		}
	BadElse("If you see this, be sure to bug Max about it! :)\n");
	exit(1);
}

void clearNodes() {
	FreeTierList(nodes); nodes = NULL;
	FreeTierList(preList); preList = NULL;
	FreeTierList(postList); postList = NULL;
}

// Check tier hierarchy AND define tierSolveList
BOOLEAN checkAndDefineTierTree() {
	printf("\n-----Checking the Tier Tree for correctness:-----\n\n");
	BOOLEAN check = tierDFS(gInitialTier, TRUE);
	clearNodes();
	if (check) {
		printf("No Errors Found! Tier Tree correctness confirmed.\n");
		printf("\n-----Generating the Tier Solve List:-----\n\n");
		// The list is in reverse order, due to topological sort! reverse + return:
		tierSolveList = ReverseTierlist(tierSolveList);
		printf("Tier Solve List created.\n");
		return TRUE;
	} else return FALSE;
}

BOOLEAN tierDFS(TIER tier, BOOLEAN defineList) {
	TIERLIST *children, *cptr;
	TIER child;
	BOOLEAN childPost;

	previsit(tier);
	children = cptr = gTierChildrenFunPtr(tier);
	for (; cptr != NULL; cptr = cptr->next) {
		child = cptr->tier;
		if (tier == child) continue;

		// if child not visited, DFS
		if (!getNode(child, &childPost)) {
			if (!tierDFS(child, defineList)) return FALSE;

		// else check if there's a "back edge" == cycle
		// back edge from (v,w) : post[w] false
		} else if (!childPost) {
			printf("ERROR! Tier %d leads back to higher Tier %d!\n", child, tier);
			return FALSE;
		}
	}
	FreeTierList(children);
	postvisit(tier);
	if (defineList)
		tierSolveList = CreateTierlistNode(tier, tierSolveList);
	return TRUE;
}

/************************************************************************
**
** DEBUG (TO HELP MODULE WRITERS)
**
************************************************************************/
/*
TIERLIST* gTierSolveListPtr; // have to get rid of this

//when a user enters a tier number, this checks to make sure
//that tier is in the current hash window (valid).
BOOLEAN validTier(TIER tier, BOOLEAN global) {
	if (global) {
		TIERLIST* list = gTierSolveListPtr;
		for (; list != NULL; list = list->next)
			if (list->tier == tier)
				return TRUE;
	} else {
		int i;
		for (i = 1; i < gNumTiersInHashWindow; i++)
			if (gTierInHashWindow[i] == tier)
				return TRUE;
	}
	return FALSE;
}

void debugMenu() {
	printf(	"\n\n=====Welcome to the Debug Menu!=====\n"
			"--Automatically initializing HASH WINDOW for Tier: %d\n", gTierSolveListPtr->tier);
	gInitializeHashWindow(gTierSolveListPtr->tier, FALSE);

	BOOLEAN IL, GUMTT, UDM, TTS;
	STRING tierStr;

	IL = !(gIsLegalFunPtr == NULL);
	GUMTT = !(gGenerateUndoMovesToTierFunPtr == NULL);
	UDM = !(gUnDoMoveFunPtr == NULL);
	TTS = !(gTierToStringFunPtr == NULL);

	char c; POSITION p, p2, p3; TIER t; BOOLEAN check;
	TIERLIST *list;//, *listptr; TIERPOSITION tp;
	//UNDOMOVELIST *ulist, *ulistptr; MOVELIST *mlist, *mlistptr;

	// keep compiler happy
	p2=p3 = 0; check=FALSE;


	while(TRUE) {
		printf("\n\t----- Debug Options: -----\n"
			"\t(Current HASH WINDOW: Tier %d)\n"
			"\tr)\t(R)eprint gTierSolveList Information\n"
			"\td)\t(D)raw and Check Tier \"Tree\"\n\n"
			"\tt)\tGo to g(T)ierChildren Menu\n"
			"\tn)\tGo to g(N)umberOfTierPositions Menu\n"
			"\ti)\tGo to g(I)sLegal Menu\n"
			"\tg)\tGo to g(G)enerateUndoMovesToTier Menu\n"
			"\tu)\tGo to g(U)nDoMove Menu\n\n"
			"\tp)\tGo to (P)OSITION-DEBUG Menu\n\n"
			"\tc)\t(C)hange current HASH WINDOW\n"
			"\tb)\t(B)ack to main solver menu\n"
			"\nSelect an option:  ", gTierInHashWindow[1]);
		c = GetMyChar();
		switch(c) {
			case 'r': case 'R':
				printf("-Tier Solve Order:\n");
				list = gTierSolveListPtr;
				for (; list != NULL; list = list->next) {
					printf("%d ", list->tier);
					if (TTS) {
						tierStr = gTierToStringFunPtr(list->tier);
						printf(": %s\n", tierStr);
						if (tierStr != NULL) SafeFree(tierStr);
					}
				}
				break;
			case 'd': case 'D':
				checkTierTree();
				break;

			case 't': case 'T':
				while(TRUE) {
					printf("--gTierChildren TEST MENU--\n"
							"Enter any TIER number to check, \"%llu\" to test all valid tiers,\n"
							"or non-number to go back:\n> ", gNumberOfPositions);
					p = GetMyPosition();
					if (p == kBadPosition) break;
					if (p == gNumberOfPositions) {
						printf("-Results for ALL Valid Tiers:\n");
						list = gTierSolveListPtr;
						for (; list != NULL; list = list->next) {
							printf("%d ", list->tier);
							// print the kiddies
						}
						continue;
					}
					t = (TIER)p;
					if (!validTier(t,TRUE)) {
						printf("Tier (%d) isn't a valid tier! (It's not found in gTierSolveList)\n", t);
						continue;
					}
					// print the kiddies
				}
			case 'n': case 'N':
			case 'i': case 'I':
			case 'g': case 'G':
				if (!GUMTT) {
					printf("gGenerateUndoMovesToTier isn't written...\n");
					break;
				}
				while (TRUE) {
					printf("\nEnter a POSITION number to check, or non-number to go back:\n> ");
					p = GetMyPosition();
					if (p == kBadPosition) break;
					printf("\nNow enter a TIER number (in current hash window!), or non-number to go back:\n> ");
					p2 = GetMyPosition();
					if (p2 == kBadPosition) break;
					t = (TIER)p2;
					if (TC && hashWindowTier != kBadTier) {
						printf("Calling gTierChildren on hash window TIER %d for safety check:\n", hashWindowTier);
						list = listptr = gTierChildrenFunPtr(t);
						check = FALSE;
						if (t == hashWindowTier) check = TRUE;
						else {
							for (; listptr != NULL; listptr = listptr->next) {
								if (t == listptr->tier) {
									check = TRUE;
									break;
								}
							}
						}
						FreeTierList(list);
						if (!check) {
							printf("The TIER you gave isn't in the current hash window!\n");
							break;
						}
					}
					printf("\nCalling gGenerateUndoMovesToTier on POSITION %llu to TIER %d:\n", p, t);
					ulist = ulistptr = gGenerateUndoMovesToTierFunPtr(p,t);
					if (ulist == NULL) {
						printf("Returned an empty list!\n");
						continue;
					} else {
						printf("Returned the following list:");
						for (; ulistptr != NULL; ulistptr = ulistptr->next)
							printf(" %d", ulistptr->undomove);
						ulistptr = ulist;
						if (UDM) {
							printf("\n\nNow exhaustively checking the list:\n");
							for (; ulistptr != NULL; ulistptr = ulistptr->next) {
								printf("\n\n-----\n\nCalling gUnDoMove on UNDOMOVE: %d\n", ulistptr->undomove);
								p2 = gUnDoMoveFunPtr(p, ulistptr->undomove);
								printf("This returned POSITION: %llu. Calling PrintPosition on it:\n", p2);
								PrintPosition (p2, "Debug", 0);
								if (!Primitive(p2)) {
									mlist = mlistptr = GenerateMoves(p2);
									check = FALSE;
									for (; mlistptr != NULL; mlistptr = mlistptr->next) {
										if (p == DoMove(p2, mlistptr->move)) {
											check = TRUE;
											break;
										}
									}
									FreeMoveList(mlist);
									if (!check) printf("---ERROR FOUND: %llu wasn't found in %llu's GenerateMoves!\n", p, p2);
									else printf("%llu is indeed in %llu's GenerateMoves.\n", p, p2);
								} else printf("---ERROR FOUND: %llu generated an undomove to %llu, a primitive!\n", p, p2);
							}
						}
						FreeUndoMoveList(ulist);
					}
				} break;
			case 'u': case 'U':
				if (!UDM) {
					printf("gUnDoMove isn't written, so...\n");
					break;
				}
				while (TRUE) {
					printf("\nEnter a POSITION number to check, or non-number to go back:\n> ");
					p = GetMyPosition();
					if (p == kBadPosition) break;
					printf("\nEnter an UNDOMOVE number to check, or non-number to go back:\n> ");
					p2 = GetMyPosition();
					if (p2 == kBadPosition) break;
					printf("Calling gUnDoMove on POSITION %llu, with UNDOMOVE: %d\n", p, (UNDOMOVE)p2);
					p = gUnDoMoveFunPtr(p, (UNDOMOVE)p2);
					printf("This returned POSITION: %llu. Calling PrintPosition on it:\n", p);
					PrintPosition (p, "Debug", 0);
				} break;
			case 'p': case 'P': // TEST BY POSITION
				while (TRUE) {
					printf("\nEnter a POSITION number to check, or non-number to go back:\n> ");
					p = GetMyPosition();
					if (p == kBadPosition) break;
					else {
						printf("Calling PrintPosition on POSITION: %llu\n", p);
						PrintPosition (p, "Debug", 0);
					}
					if (PTT) {
						printf("\n-----TIER TESTS-----\n");
						printf("\nCalling gPositionToTier...\n");
						t = gPositionToTierFunPtr(p);
						printf("gPositionToTier says this is in TIER %d\n", t);
						if (PTTP) {
							printf("\nCalling gPositionToTierPosition...\n");
							tp = gPositionToTierPositionFunPtr(p, t);
							printf("Resulting TIERPOSITION is: %llu\n", tp);
						}
						if (TC) {
							printf("\nCalling gTierChildren...\n");
							list = listptr = gTierChildrenFunPtr(t);
							printf("Returned list:");
							for (; listptr != NULL; listptr = listptr->next)
								printf(" %d", listptr->tier);
							printf("\n");
							if (!Primitive(p)) {
								HitAnyKeyToContinue();
								printf("\n-----GENERATE MOVE TESTS-----\n");
								mlist = mlistptr = GenerateMoves(p);
								for (; mlistptr != NULL; mlistptr = mlistptr->next) {
									p2 = DoMove(p, mlistptr->move);
									printf("\n\n-----\nChild: %llu. PrintPosition is:\n", p2);
									PrintPosition(p2, "Debug", 0);
									printf("\nCalling gPositionToTier on child...\n");
									t2 = gPositionToTierFunPtr(p2);
									listptr = list; check = FALSE;
									for (; listptr != NULL; listptr = listptr->next) {
										if (t2 == listptr->tier) {
											check = TRUE;
											break;
										}
									}
									if (!check) printf("---ERROR FOUND: %llu has TIER: %d, which isn't in %llu's gTierChildren!\n", p2, t2, p);
									else printf("%llu has TIER: %d, which is indeed in %llu's gTierChidren.\n", p2, t2, p);

									if (GUMTT && UDM) {
										printf("\nNow Calling gGenerateUndoMovesToTier to TIER: %d\n", t);
										ulist = ulistptr = gGenerateUndoMovesToTierFunPtr(p2, t);
										printf("\nNow calling gUnDoMove to exhaustively check the returned list:\n");
										check = FALSE;
										for (; ulistptr != NULL; ulistptr = ulistptr->next) {
											p3 = gUnDoMoveFunPtr(p2, ulistptr->undomove);
											if (p3 == p) {
												check = TRUE;
												break;
											}
										}
										FreeUndoMoveList(ulist);
										if (!check) printf("---ERROR FOUND: %llu isn't in %llu's gGenerateUndoMoves!\n", p, p2);
										else printf("%llu is indeed in %llu's gGenerateUndoMoves.\n", p, p2);
									}
									HitAnyKeyToContinue();
								}
								FreeMoveList(mlist);
							}
							FreeTierList(list);
						}
					}
				} break;
			case 'c': case 'C':
				printf("\nEnter the TIER to change to, or non-number to go back:\n> ");
				p = GetMyPosition();
				if (p == kBadPosition) break;
				t = (TIER)p;
				printf("Calling gInitializeHashWindow on TIER: %d\n", t);
				gInitializeHashWindow(t, FALSE);
				printf("The hash window reported %d tiers in the hash window:\n", gNumTiersInHashWindow-1);
				int i;
				for (i = 1; i < gNumTiersInHashWindow; i++) {
					if (i == 1)
						printf("MAIN ");
					else printf("CHILD ");
					printf("TIER %d (%llu Positions)\n",
						gTierInHashWindow[i], gMaxPosOffset[i]-gMaxPosOffset[i-1]);
				}
				printf("The current (main) tier is reported to be %s\n", (gCurrentTierIsLoopy ? "LOOPY" : "NON-LOOPY"));
				printf("gNumberOfPositions has been set to: %llu\n", gNumberOfPositions);
				break;
			case 'b': case 'B':
				return;
			default:
				printf("Invalid option!\n");
		}
	}
}
*/


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
	fp = fopen("./b.txt","w");
	for (p = 0; p < gNumberOfPositions; p++)
		fprintf (fp, "%d\t%lld\t%d\n", GetValueOfPosition(p), p, Remoteness(p));
	fclose(fp);
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
			printf("Tier %d, with %llu...\n", i, gNumberOfTierPositionsFunPtr(i));
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
			printf("Tier %d, with %llu...\n", i, gNumberOfTierPositionsFunPtr(i));
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
void compareTwoFiles(char *mine, char *theirs) {
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
		if (c == '0') {
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
		printf("CONGRATULATIONS!!! SUCCESS!!!\n");
		printf("My Efficiency: %.1f", 100*(double)efficiency/(double)line);
	}
}

void skipToNewline(FILE* fp) {
	int c = getc(fp);
	while (c != '\n') c = getc(fp);
}



/* MOVED ALL OLD SOLVE STUFF DOWN HERE FOR NOW: */

/*
POSITIONLIST* childlist;
REMOTENESS maxUncorruptedWinRem;
REMOTENESS minTieRem;
BOOLEAN seenDraw;
*/

/************************************************************************
**
** ERROR HANDLING
**
************************************************************************

void HandleErrorAndExit() {
	printf("\nIt appears one more files have been corrupted.\n");
	printf("Please restart the game and solve from the start.\n");
	exit(1);
}

// Subtypes of HandleErrorAndExit for files, using filename.
void FileOpenError() {
	printf("\n--ERROR: Couldn't open file: %s\n", filename);
	HandleErrorAndExit();
}

void FileWriteError() {
	printf("\n--ERROR: Couldn't write to file: %s\n", filename);
	HandleErrorAndExit();
}

void FileSyntaxError() {
	printf("\n--ERROR: File in incorrect format: %s\n", filename);
	HandleErrorAndExit();
}

void FileCloseError() {
	printf("\n--ERROR: Couldn't close file: %s\n", filename);
	HandleErrorAndExit();
}
*/

/************************************************************************
**
** NAME:        SolveTier
**
** DESCRIPTION: The heart of the solver. This takes in a tier number,
**				reads in the appropriate tier file, and then iterates
**				through the tier, saving the children and such to file
**				if need be. This algorithm guarantees that what's left
**				in the end is ONLY Tier N values (where N is the tier
**				we're working with). If the tier is non-loopy (i.e.
**				all its children are lower tier), then it ends there.
**				Otherwise, it calls the loopy algorithm.
**				In the end, it marks unvisited children as draws.
**
************************************************************************

void SolveTier(int tier) {
	FILE *fileW, *fileR;
	int fR = 1;
	POSITION pos, child, numSolved = 0, numCorrupted = 0, sizeOfTier = tierSize[tier];
	MOVELIST *moves, *children;
	REMOTENESS remoteness, maxWinRem, minLoseRem;
	VALUE value;
	BOOLEAN seenLose;

	printf("\n--Solving Tier %d...\n",tier);
	printf("Size of tier: %lld\n",sizeOfTier);

	printf("Reading/Writing appropriate files...\n");

	sprintf(filename,"./retrograde/1.solve");
	if ((fileW = fopen(filename, "w")) == NULL) FileOpenError();
	sprintf(filename,"./retrograde/%s_%d_%d.tier",kDBName,variant,tier);
	if ((fileR = fopen(filename, "r")) == NULL) FileOpenError();

	printf("Doing an initial sweep, and creating child pointers (could take a WHILE)...\n");

	while((pos = readPos(fileR)) != kBadPosition) {
		value = Primitive(pos);
		if (value != undecided) { // check for primitive-ness
			StoreValueOfPosition(pos,value);
			SetRemoteness(pos,0);
			numSolved++; continue;
		}
		moves = children = GenerateMoves(pos);
		if (moves == NULL) { // no children, a LOSE
			StoreValueOfPosition(pos,lose);
			SetRemoteness(pos,0);
			numSolved++; continue;
		}
		minLoseRem = minTieRem = REMOTENESS_MAX; // otherwise, let's see what children we can get rid of.
		maxWinRem = 0;
		seenLose = seenDraw = FALSE;
		childlist = NULL;
		for (; children != NULL; children = children->next) {
			child = DoMove(pos,children->move);
			//if (TierValue(child) > tier) {//Too expensive right now, will be replaced with if(GetTier(child) > tier) for Slice API
			// printf("TierValue not correctly defined: position %lld in Tier %d
			//			has child %lld in Tier %d\n", pos, tier, child, GetTier(child));
			// exit(0); // and this will be handled more gracefully
			//} // and if(Visited(child)) down there will be replaced with if(GetTier(child) == tier)
			if (tier == 0) value = Primitive(child);
			else value = GetValueOfPosition(child);
			if (value != undecided) {
				remoteness = Remoteness(child);
				if (value == tie) { //this COULD be tie OR draw
					seenDraw = TRUE;
					if (remoteness < minTieRem)
						minTieRem = remoteness;
				} else if (value == lose) {
					seenLose = TRUE;
					if (Visited(child)) // NOT lower tier, but still corrupted tier N, gotta save
						childlist = StorePositionInList(child,childlist);
					if (remoteness < minLoseRem)
						minLoseRem = remoteness;
				} else if (value == win) {
					if (remoteness > maxWinRem)
						maxWinRem = remoteness;
				}
			} else childlist = StorePositionInList(child,childlist);
		}
		FreeMoveList(moves);
		if (childlist == NULL) { // ALL my children were lower tier/already solved
			if (seenLose) {
				StoreValueOfPosition(pos, win);
				SetRemoteness(pos,minLoseRem+1);
				numSolved++;
			} else if (seenDraw) {
				StoreValueOfPosition(pos, tie);
				if (minTieRem == REMOTENESS_MAX)
					SetRemoteness(pos,REMOTENESS_MAX);// a draw
				else SetRemoteness(pos,minTieRem+1);// else a tie
				numSolved++;
			} else {
				StoreValueOfPosition(pos, lose);
				SetRemoteness(pos,maxWinRem+1);
				numSolved++;
			}
		} else { //ah, too bad, remember values for later:
			if (seenLose) {
				StoreValueOfPosition(pos, win);// make this a "corrupted win"
				SetRemoteness(pos,minLoseRem+1);
				MarkAsVisited(pos);
				writeCorruptedWinToFile(fileW, pos, childlist);
				numCorrupted++;
			} else writeUnknownToFile(fileW, pos, childlist, maxWinRem, minTieRem, seenDraw);
		}
	}
	if (fclose(fileR) == EOF) FileCloseError();
	if (fclose(fileW) == EOF) {
		sprintf(filename,"./retrograde/1.solve");
		FileCloseError();
	}
	if (numSolved != sizeOfTier) {
		fR = SolveWithDelgadilloAlgorithm(sizeOfTier, numSolved, numCorrupted);
		printf("Setting undecided to DRAWs and correcting corruption...\n");
	}
	else printf("Tier is non-loopy! Setting undecided to DRAWs...\n");
	sprintf(filename,"./retrograde/%d.solve",fR);
	if ((fileR = fopen(filename, "r")) == NULL) FileOpenError();
	while((pos = readPos(fileR)) != kBadPosition) {
		if ((readSolveFile(fileR)) == 'u') { // non-corrupted position
			StoreValueOfPosition(pos, tie);
			if (seenDraw) { // a tie
				if (minTieRem == REMOTENESS_MAX)
					SetRemoteness(pos,REMOTENESS_MAX);// a draw
				else SetRemoteness(pos,minTieRem+1);// else a tie
			} else SetRemoteness(pos,REMOTENESS_MAX); // a draw
		} else UnMarkAsVisited(pos); // must've been corrupted, now TRUE
	}
	if (fclose(fileR) == EOF) FileCloseError();
	sprintf(filename,"./retrograde/1.solve");
	remove(filename);
	sprintf(filename,"./retrograde/2.solve");
	remove(filename);
	printf("Tier fully solved!...\n");
	positionsLeft -= sizeOfTier;
}
*/

/************************************************************************
**
** NAME:        SolveWithDelgadilloAlgorithm
**
** DESCRIPTION: Apply the "zero-memory/file hybrid" which I've
**				egotistically called the Delgadillo Algorithm for now.
**
** This is done in leu of a "loopy" solver. It does the zero-memory like thing.
** Only it's much much faster since it (a) only iterates through a tier,
** (b) already has all the child pointers in memory so doesn't need GenerateMoves
** (or DoMove), (c) those pointers only include other Tier N positions, and
** (d) thanks to the file representation we can ignore any positions and children
** that we already know and don't need to look at again.
**
** The key reason it looks like hell, though, is because since this is bottom-up,
** we can assign a value of WIN to something that has a LOSE child but not know
** its true remoteness (since unexplored values might be LOSES with less
** remoteness). So the idea is we label it a "corrupted" win. Namely, its
** label is correct (it IS a win), but its remoteness might be wrong. So,
** if you use a corrupted win to label yourself a lose, you become a "corrupted"
** lose. Etc.
** Things get really complicated from there, but essentially the key idea
** is that corruption means you OVERSHOT the initial WIN remoteness. That means
** that, for ANY corrupted position, whether WIN or LOSE:
** True remoteness <= Corrupted Remoteness.
** So for corrupted positions, we go and check if our corrupted child's remoteness
** got smaller so we can update ours.
** For MORE complicatedness (because, as the code shows, it's even MORE complicated),
** talk to me.
** Anyway, once there's no changes in the above loop, we exit.
**
** Any questions, talk to me.
** -Max
**
************************************************************************

int SolveWithDelgadilloAlgorithm(POSITION sizeOfTier, POSITION numSolved, POSITION numCorrupted) {
	FILE *fileW, *fileR;
	int numUnknownChildren, type, fW = 2, fR = 1;
	POSITION pos, child;
	POSITIONLIST *newchildlist, *copy;
	REMOTENESS remoteness, maxWinRem, minLoseRem, maxCorruptedWinRem;
	VALUE value;
	BOOLEAN change = TRUE, seenLose, corruptedWin, corruptedLose, update;

	//Invariant: ALL that's left to check are unknown Tier N positions.
	while (change) {
		change = FALSE;
		if (numSolved == sizeOfTier) continue; // if we're done, then stop the loop
		printf("%lld Positions still unsolved ", sizeOfTier-numSolved);
		printf("(%lld corrupted). Doing another sweep...\n", numCorrupted);

		sprintf(filename,"./retrograde/%d.solve",fW);
		if ((fileW = fopen(filename, "w")) == NULL) FileOpenError();
		sprintf(filename,"./retrograde/%d.solve",fR);
		if ((fileR = fopen(filename, "r")) == NULL) FileOpenError();

		while((pos = readPos(fileR)) != kBadPosition) {
			type = readSolveFile(fileR);
			copy = childlist;
			newchildlist = NULL;
			// Corrupted Lose: ALL children are KNOWN WIN values, but remoteness too high. Children are all corrupt.
			if (type == 'l') {
				while (TRUE) {
					update = FALSE;
					maxCorruptedWinRem = maxUncorruptedWinRem; // this is the SMALLEST the true remoteness can be.
					for (; childlist != NULL; childlist = childlist->next) {
						child = childlist->position;
						remoteness = Remoteness(child);
						if (remoteness < maxUncorruptedWinRem) // child is lower win, we dont care anymore
							continue;
						if (remoteness > maxCorruptedWinRem) {
							maxCorruptedWinRem = remoteness;
							if (!Visited(child)) {// if legal, change our minimum estimate
								maxUncorruptedWinRem = remoteness;
								update = TRUE;
								continue;
							}
						}
						if (!Visited(child)) // child became legal!
							continue;
						newchildlist = StorePositionInList(child,newchildlist);
					}
					if (update) {// loop through our children again, just in case
						childlist = copy;
						if (newchildlist != NULL)
							FreePositionList(newchildlist);
						newchildlist = NULL;
					} else break;
				}
				if (maxCorruptedWinRem+1 < Remoteness(pos)) {//if our high estimate was lowered
					SetRemoteness(pos, maxCorruptedWinRem+1);
					change = TRUE;
				}
				if (newchildlist == NULL) {// I just became uncorrupted!
					UnMarkAsVisited(pos);
					numSolved++;
					numCorrupted--;
					change = TRUE;
				} else writeCorruptedLoseToFile(fileW, pos, newchildlist, maxUncorruptedWinRem); // else, update new min
			// Corrupted Win: I have a KNOWN LOSE child, but not ALL my children are known,
			// so remoteness is wrong. Children are either unknown or corrupt.
			} else if (type == 'w') {
				for (; childlist != NULL; childlist = childlist->next) {
					child = childlist->position;
					value = GetValueOfPosition(child);
					if (value != undecided) {
						if (value == win || value == tie) //child must have become WIN or DRAW, good, ignore
							continue;
						remoteness = Remoteness(child); //else child's a LOSE, check remoteness
						if (remoteness+1 < Remoteness(pos)) {//if child changed
							SetRemoteness(pos, remoteness+1);
							change = TRUE;
						}
						if (!Visited(child)) // child became legal!
							continue;
					}
					newchildlist = StorePositionInList(child,newchildlist);
				}
				if (newchildlist == NULL) {// I just became uncorrupted!
					UnMarkAsVisited(pos);
					numSolved++;
					numCorrupted--;
					change = TRUE;
				} else writeCorruptedWinToFile(fileW, pos, newchildlist);
			// Turns out there's NO corruption, ALL you've seen is wins(perhaps corrupted),
			// and perhaps a draw. So some children are still unknown then!
			} else {
				numUnknownChildren = 0;
				minLoseRem = REMOTENESS_MAX;
				maxWinRem = maxCorruptedWinRem = maxUncorruptedWinRem;
				seenLose = corruptedWin = corruptedLose = FALSE;
				for (; childlist != NULL; childlist = childlist->next) {
					numUnknownChildren++;
					child = childlist->position;
					value = GetValueOfPosition(child);
					if (value != undecided) {
						numUnknownChildren--;
						remoteness = Remoteness(child);
						if (value == tie) {
							seenDraw = TRUE;
							if (remoteness < minTieRem)
								minTieRem = remoteness;
							continue;
						} if (value == lose) {
							seenLose = TRUE;
							if (remoteness < minLoseRem)
								minLoseRem = remoteness;
							if (Visited(child)) corruptedWin = TRUE;
							else continue;
						} else if (value == win) {
							if (Visited(child)) {
								if (remoteness > maxCorruptedWinRem)
									maxCorruptedWinRem = remoteness;
								corruptedLose = TRUE;
							} else {
								if (remoteness > maxWinRem)
									maxWinRem = remoteness;
								continue;
							}
						}
					} newchildlist = StorePositionInList(child,newchildlist);
				}
				if (numUnknownChildren == 0) { //no more unknown children!
					if (seenLose) {
						StoreValueOfPosition(pos, win);
						SetRemoteness(pos,minLoseRem+1);
						change = TRUE;
						if (!corruptedWin) numSolved++;
						else { //we ARE corrupted, make Corrupted WIN
							MarkAsVisited(pos);
							writeCorruptedWinToFile(fileW, pos, newchildlist);
							numCorrupted++;
						}
					} else if (seenDraw) { // we're a DRAW through and through
						StoreValueOfPosition(pos, tie);
						if (minTieRem == REMOTENESS_MAX)
							SetRemoteness(pos,REMOTENESS_MAX);// a draw
						else SetRemoteness(pos,minTieRem+1);// else a tie
						change = TRUE;
						numSolved++;
					} else {
						StoreValueOfPosition(pos, lose);
						change = TRUE;
						if (!corruptedLose || maxWinRem > maxCorruptedWinRem) {
							SetRemoteness(pos,maxWinRem+1);
							numSolved++;
						} else {//we ARE corrupted, make Corrupted LOSE
							SetRemoteness(pos,maxCorruptedWinRem+1);
							MarkAsVisited(pos);
							writeCorruptedLoseToFile(fileW, pos, newchildlist, maxWinRem);
							numCorrupted++;
						}
					}
				} else if (seenLose) {
					StoreValueOfPosition(pos, win);// make this a "corrupted win"
					SetRemoteness(pos,minLoseRem+1);
					MarkAsVisited(pos);
					writeCorruptedWinToFile(fileW, pos, newchildlist);
					numCorrupted++;
					change = TRUE;
				} else // STILL unknown, let's save if we're a future DRAW and our max remoteness so far
					writeUnknownToFile(fileW, pos, newchildlist, maxWinRem, minTieRem, seenDraw);
			}
			if (copy != NULL) // Should ALWAYS be the case
				FreePositionList(copy);
		}
		if (fclose(fileR) == EOF) FileCloseError();
		if (fclose(fileW) == EOF) {
			sprintf(filename,"./retrograde/%d.solve",fW);
			FileCloseError();
		}
		fR = (fR == 1 ? 2 : 1);
		fW = (fW == 1 ? 2 : 1);
	}
	return fR;
}
*/


/************************************************************************
**
** INITIALIZATION AND CLEANUP
**
************************************************************************

// This initializes the tier files
void initFiles() {
	int tier, lower, upper = 0;
	FILE **files;
	POSITION pos;

	positionsLeft = gNumberOfPositions;
	printf("We're going to make these many files: %d\n", tierMax+1);
	printf("C says we can open this many files at once: %d\n", FOPEN_MAX);
	printf("Thus, %d full iteration(s) will have to be done.\n", ((tierMax+1)/FOPEN_MAX)+1);
	mkdir("retrograde", 0755);
	// INITIALIZE ALL THE TIER FILES:
	files = (FILE**) SafeMalloc((tierMax+1) * sizeof(FILE*));
	while(upper != (tierMax+1)) {
		lower = upper;
		upper = ((tierMax+1) < upper+FOPEN_MAX ? tierMax+1 : upper+FOPEN_MAX);

		// Open all TierMax Files
		printf("  Opening all the files...\n");
		for (tier = lower; tier < upper; tier++) {
			sprintf(filename,"./retrograde/%s_%d_%d.tier",kDBName,variant,tier);
			if ((files[tier] = fopen(filename, "w")) == NULL) FileOpenError();
			tierSize[tier] = 0;
		}
		printf("  Writing all the files (could take FOREVER)...\n");
		for (pos = 0; pos < gNumberOfPositions; pos++) {
			tier = gRetrogradeTierValue(pos);
			if (tier == -1) {
				if (upper == (tierMax+1)) positionsLeft--;
				continue;
			}
			if (tier < 0 || tier > tierMax) {
				printf("\n--ERROR: gRetrogradeTierValue not correctly defined: returned %d for position %lld\n", tier, pos);
				printf("  Closing all the files...\n");
				for (tier = lower; tier < upper; tier++) {
					if (fclose(files[tier]) == EOF) {
						sprintf(filename,"./retrograde/%s_%d_%d.tier",kDBName,variant,tier);
						FileCloseError();
					}
				}
				HandleErrorAndExit();
			}
			if (tier >= upper || tier < lower) continue;
			//FOR SLICE API: SetTier(pos, tier);
			if (fprintf(files[tier], "%lld ", pos) < 0) {
				sprintf(filename,"./retrograde/%s_%d_%d.tier",kDBName,variant,tier);
				FileWriteError();
			}
			tierSize[tier] += 1;
		}
		// Close all the files
		printf("  Closing all the files...\n");
		for (tier = lower; tier < upper; tier++) {
			if (fclose(files[tier]) == EOF) {
				sprintf(filename,"./retrograde/%s_%d_%d.tier",kDBName,variant,tier);
				FileCloseError();
			}
		}
	}
	// Free the files pointer
	SafeFree(files);
}

// This removes ALL files created by the solver (ignores errors)
void removeFiles() {
	printf("  Deleting all the files...\n");
	int f;
	// Tier files
	for (f = 0; f <= tierMax; f++) {
		sprintf(filename,"./retrograde/%s_%d_%d.tier",kDBName,variant,f);
		remove(filename);
	}
	// Solve files
	sprintf(filename,"./retrograde/1.solve");
	remove(filename);
	sprintf(filename,"./retrograde/2.solve");
	remove(filename);
	// Save files
	sprintf(filename,"./retrograde/%s_%d_TIER.save",kDBName,variant);
	remove(filename);
	sprintf(filename,"./retrograde/%s_%d_DB.save",kDBName,variant);
	remove(filename);
	SafeFree(tierSize); // Frees the tierSize array
}
*/

/************************************************************************
**
** SOLVER PROGRESS FILES
**
************************************************************************

// This saves the current progress to a file.
void SaveProgressToFile(int tier) {
	FILE* fp;
	int f;

	printf("--Creating Save File...\n");
	sprintf(filename,"./retrograde/%s_%d_TIER.save",kDBName,variant);
	if ((fp = fopen(filename, "w")) == NULL) FileOpenError();
	if (fprintf(fp, "PROGRESS=%d \nPOSLEFT=%lld \nTIERSIZES=", tier, positionsLeft) < 0)
		FileWriteError();
	for (f = 0; f <= tierMax; f++)
		if (fprintf(fp, "%lld ", tierSize[f]) < 0)
			FileWriteError();
	SafeFree(tierSize);
	printf("--Save File created.\n");
	if (fclose(fp) == EOF) FileCloseError();
	SaveDBToFile();
	printf("--Progress properly saved.\n"
			"To guarantee correct solving, make sure that the game code is not\n"
			"modified before restarting the solving. Also make sure that the files\n"
			"in the 'retrograde' directory are not tampered with.\n");
}

// This loads the progress from a file, but also checks that every file
// that SHOULD be there is there. Then it deletes the file.
// Returns -1 if there is no save file..
int LoadProgressFromFile() {
	FILE* fp;
	int f, tier;

	sprintf(filename,"./retrograde/%s_%d_TIER.save",kDBName,variant);
	if ((fp = fopen(filename, "r")) == NULL) // no savefile
		return -1;
	printf("--Loading Save File...\n");
	if ((getc(fp)) != 'P' || (getc(fp)) != 'R' ||
		(getc(fp)) != 'O' || (getc(fp)) != 'G' || (getc(fp)) != 'R' ||
		(getc(fp)) != 'E' || (getc(fp)) != 'S' || (getc(fp)) != 'S' ||
		(getc(fp)) != '=')
		FileSyntaxError();
	if ((tier = readPos(fp)) == kBadPosition || tier < 0 || tier > tierMax)
		FileSyntaxError();
	if ((getc(fp)) != '\n' || (getc(fp)) != 'P' || (getc(fp)) != 'O' ||
		(getc(fp)) != 'S' || (getc(fp)) != 'L' || (getc(fp)) != 'E' ||
		(getc(fp)) != 'F' || (getc(fp)) != 'T' || (getc(fp)) != '=')
		FileSyntaxError();
	if ((positionsLeft = readPos(fp)) == kBadPosition || positionsLeft < 0
		|| positionsLeft > gNumberOfPositions)
		FileSyntaxError();
	if ((getc(fp)) != '\n' || (getc(fp)) != 'T' || (getc(fp)) != 'I' ||
		(getc(fp)) != 'E' || (getc(fp)) != 'R' || (getc(fp)) != 'S' ||
		(getc(fp)) != 'I' || (getc(fp)) != 'Z' || (getc(fp)) != 'E' ||
		(getc(fp)) != 'S' || (getc(fp)) != '=')
		FileSyntaxError();
	for (f = 0; f <= tierMax; f++) {
		if ((tierSize[f] = readPos(fp)) == kBadPosition || tierSize[f] < 0
			|| tierSize[f] > gNumberOfPositions)
			FileSyntaxError();
	}
	if ((getc(fp)) != EOF)
		FileSyntaxError();
	printf("--Save File Loaded\n");
	if (fclose(fp) == EOF) FileCloseError();
	printf("--Checking to make sure TIER files weren't deleted...\n");
	for (f = 0; f <= tierMax; f++) {
		sprintf(filename,"./retrograde/%s_%d_%d.tier",kDBName,variant,f);
		if ((fp = fopen(filename, "r")) == NULL) FileOpenError();
		if (fclose(fp) == EOF) FileCloseError();
	}
	LoadDBFromFile();
	sprintf(filename,"./retrograde/%s_%d_TIER.save",kDBName,variant);
	remove(filename);
	printf("--Save File Removed, everything properly loaded.\n");
	printf("To guarantee correct solving, make sure that the game code was not\n");
	printf("modified after saving the progress of the previous solve.\n");
	return tier;
}

// This loads a saved DB file, and deletes the file. Will be replaced
// by TierDB when it works.
void LoadDBFromFile() {
	FILE* fp;
	POSITION pos, c;
	VALUE value; REMOTENESS remoteness;

	printf("--Loading the Database from File...\n");
	sprintf(filename,"./data/%s_%d_DB.save",kDBName,variant);
	if ((fp = fopen(filename, "r")) == NULL) FileOpenError();
	while((pos = readPos(fp)) != kBadPosition) {
		if ((c = readPos(fp)) == kBadPosition || c < 0 || c > 3)
			FileSyntaxError();
		value = c;
		StoreValueOfPosition(pos, value);
		if ((c = readPos(fp)) == kBadPosition || c < 0 || c > REMOTENESS_MAX)
			FileSyntaxError();
		remoteness = c;
		SetRemoteness(pos, remoteness);
		if ((getc(fp)) != '\n')
			FileSyntaxError();
	}
	printf("  Database Loaded\n");
	if (fclose(fp) == EOF) FileCloseError();
	sprintf(filename,"./retrograde/%s_%d_DB.save",kDBName,variant);
	remove(filename);
	printf("  DB Save File Removed\n");
}
*/

/************************************************************************
**
** READING FILES
** (These are just here as precursors to Deepa Blue Stuff.
** They'll be deprecated in a bit.)
**
************************************************************************

// This reads in a POSITION from a file, delineated by whitespace.
// Instantly terminates if finds an EOF (tier files) or newline (solve files).
// Of course, also works with integers (and REMOTENESS and BOOLEAN).
POSITION readPos(FILE* fp) {
	int c, i = 0;
	int integer[80];
	while ((c = getc(fp)) != ' ') {
		if (c == EOF || c == '\n')
			return kBadPosition;
		if (c < '0' || c > '9') // SHOULDN'T be the case
			FileSyntaxError();
		integer[i] = c - 48;
		i++;
	}
	i--;
	POSITION pos = 0, place = 1;
	for (; i >= 0; i--) {
		pos += place*integer[i];
		place *= 10;
	}
	return pos;
}

// For solve files, this checks the "type" of the position just read
// (one of corrupted-(w)in, corrupted-(l)ose, or (u)nknown).
// Then it loads up the appropriate global variables.
int readSolveFile(FILE* fp) {
	int type;
	POSITION c, child;
	type = getc(fp);
	if (getc(fp) != ' ' && type != 'w' && type != 'l' && type != 'u')
		FileSyntaxError();
	switch(type) {
		case 'u':
			if ((c = readPos(fp)) == kBadPosition || c < 0 || c > REMOTENESS_MAX)
				FileSyntaxError();
			minTieRem = c;
			if ((c = readPos(fp)) == kBadPosition)
				FileSyntaxError();
			seenDraw = c;
		case 'l':
			if ((c = readPos(fp)) == kBadPosition || c < 0 || c > REMOTENESS_MAX)
				FileSyntaxError();
			maxUncorruptedWinRem = c;
	}
	childlist = NULL;
	while((child = readPos(fp)) != kBadPosition) {
		childlist = StorePositionInList(child,childlist);
	}
	return type;
}
*/

/************************************************************************
**
** WRITING FILES
**
************************************************************************

// This writes to file, in the TierFile sense
void writeChildrenToFile(FILE *fp, POSITIONLIST* children) {
	POSITIONLIST *copy = children;
	for (; children != NULL; children = children->next)
		if (fprintf(fp, "%lld ", children->position) < 0)
			FileWriteError();
	if (fprintf(fp, "\n") < 0) FileWriteError();
	if (copy != NULL) // Should ALWAYS be the case, but a safety check
		FreePositionList(copy);
}

void writeCorruptedWinToFile(FILE* fp, POSITION position, POSITIONLIST *children) {
	if (fprintf(fp, "%lld w ", position) < 0)
		FileWriteError();
	writeChildrenToFile(fp, children);
}

void writeCorruptedLoseToFile(FILE* fp, POSITION position, POSITIONLIST *children,
							REMOTENESS maxUncorruptedRem) {
	if (fprintf(fp, "%lld l %d ", position, maxUncorruptedRem) < 0)
		FileWriteError();
	writeChildrenToFile(fp, children);
}

void writeUnknownToFile(FILE* fp, POSITION position, POSITIONLIST *children,
						REMOTENESS maxUncorruptedRem, REMOTENESS minTieRem, BOOLEAN seenDraw) {
	if (fprintf(fp, "%lld u %d %d %d ", position, minTieRem, seenDraw, maxUncorruptedRem) < 0)
		FileWriteError();
	writeChildrenToFile(fp, children);
}
*/
