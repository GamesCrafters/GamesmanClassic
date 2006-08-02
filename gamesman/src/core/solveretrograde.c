// $Id: solveretrograde.c,v 1.10 2006-08-02 07:12:11 max817 Exp $

/************************************************************************
**
** NAME:	solveretrograde.c
**
** DESCRIPTION:	The Retrograde Solver.
**
** AUTHOR:	Max Delgadillo
**		(along with much help from Eric Siroker)
**		GamesCrafters Research Group, UC Berkeley
**		Supervised by Dan Garcia <ddgarcia@cs.berkeley.edu>
**
** DATE:	2006-04-16
**
** UPDATE HIST: -2006.4.11 = Just added fully functional solver to Gamesman.
**				-2006.4.16 = Most of the main features are now implemented.
**					(i.e. working with files (zero-memory), saving progress).
**					This also should work with ANY game, not just Bagh Chal.
**				-2006.5.24 = Fixed a small bug that incorrectly labeled ties
**					as draws.
**				-2006.7.11 = First step in the MASSIVE amount of changes to
**					follow in transitioning this solver into "Tier-Gamesman".
**					For now, the solver is DISABLED, replaced with a rough-draft
**					version of the Tier-Gamesman API debugger that will come
**					for free with the solver, usable before solving.
**				-2006.7.29 = Plenty of big changes. It now makes makeshift
**					tierdb files, which are really largely huge and inefficient
**					versions of the final tierdb. But at least we'll HAVE the
**					files to be translated later. For this reason, it now reads
**					and accepts gPositionToTierPosition. Also ChildCounter is
**					much more memory-efficient, albeit less time-efficient.
**					We're looking at gInitHashWindow use in the next update.
**				-2006.8.01 = WAY too many changes to count... After my dramatic
**					epiphany, the solver's algorithm now uses the NEW API
**					functions, including the generic Hash Window initializing.
**					Also has a non-loopy version, which solves much faster.
**					But biggest of all is that this now calls memdb to make
**					ACTUAL MEMDB TIER DATABASES! No more makeshift files!
**					As far as I know, it still solves 100% correctly.
**					There's still many more upgrades to come, espeicially with
**					the debugger...
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

/*
MAX'S NOTES TO SELF: STUFF TO DO:
-Add symmetries too
-Move Frontier/Childcounts to file
-Fix up debug menu
-Implement "Deepa Blue"'s solve files and "partial" solving
*/

/* Global variables */
int variant;
char filename[80];
TIER tier;
POSITION tierSize;
BOOLEAN sanityCheck;

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
	BOOLEAN cont = TRUE;
	TIERLIST* tierSolveList;
	TIER tiersToSolve;
    char c;
    sanityCheck = TRUE;

	printf("\n\n=====Welcome to the Retrograde Solver!=====\n");
	printf("Currently solving game (%s) with variant (%d)\n", kGameName, variant);
    //printf("Current Initial Position:  \n");
    //PrintPosition(position, "Initial", 0);

	printf("Checking to see how much of the API is given:\n\n");
	if (gTierSolveListPtr == NULL) {
		printf("gTierSolveListPtr NOT GIVEN\n");
		cont = FALSE;
	} else {
		printf("-Tier Solve Order: ");
		tierSolveList = gTierSolveListPtr;
		tiersToSolve = 0;
		for (; tierSolveList != NULL; tierSolveList = tierSolveList->next) {
			printf("%d ", tierSolveList->tier);
			tiersToSolve++;
		}
		printf("\n   %d Tiers are confirmed to be solved.\n", tiersToSolve);
	} if (gTierChildrenFunPtr == NULL) {
		printf("gTierChildrenFunPtr NOT GIVEN\n"); cont = FALSE;
	} if (gNumberOfTierPositionsFunPtr == NULL) {
		printf("gNumberOfTierPositionsFunPtr NOT GIVEN\n"); cont = FALSE;
	} if (gIsLegalFunPtr == NULL) {
		printf("gIsLegalFunPtr NOT GIVEN\n"); cont = FALSE;
	} if (gGenerateUndoMovesToTierFunPtr == NULL) {
		printf("gGenerateUndoMovesToTierFunPtr NOT GIVEN\n"); cont = FALSE;
	} if (gUnDoMoveFunPtr == NULL) {
		printf("gUnDoMoveFunPtr NOT GIVEN\n"); cont = FALSE;
	}

	if (cont == FALSE) {
		printf("\nOne or more parts of the API not given...");
		printf("\nAutomatically redirecting to the Debug Menu...\n");
		debugMenu();
		printf("Exiting Retrograde Solver (WITHOUT Solving)...\n");
		exit(0);
	} else printf("API Functions Confirmed.\n");
	tierSolveList = gTierSolveListPtr;
	tier = tierSolveList->tier;
	gInitializeHashWindow(tier);
	tierSize = gMaxPosOffset[1];
    while(cont) {
        printf("\n\n===RETROGRADE SOLVER MENU for game: %s===\n", kGameName);
        printf("Ready to solve %sLOOPY tier (%d), which contains (%lld) positions.\n",
        		(gCurrentTierIsLoopy ? "" : "NON-"), tier, tierSize);
        printf("\tOptions:\n"
        	   "\td)\t(D)ebug API Functions!\n"
               "\tc)\t(C)hange Sanity-Check option. Currently: %s.\n"
               "\ts)\t(S)olve the next tier.\n"
               "\ta)\t(A)utomate the solving for all the tiers left.\n"
               "\te)\t(E)xit the Retrograde Solver.\n"
        	   "\th)\t(H)AXX Compare the two database files, a.txt and b.txt\n"
               "\nSelect an option:  ", (sanityCheck ? "TRUE" : "FALSE"));
        c = GetMyChar();
        switch(c) {
			case 'd': case 'D':
				debugMenu();
				break;
			case 'c': case 'C':
				if (sanityCheck) sanityCheck = FALSE;
				else sanityCheck = TRUE;
				break;
			case 's': case 'S':
				printf("-Solve Tier %d?\n", tier);
				if (!ConfirmAction('s')) break;
	            SolveLoopyTier();
	            tierSolveList = tierSolveList->next;
	            if (tierSolveList == NULL) {
	            	printf("\n%s is now fully solved!\n", kGameName);
                	cont = FALSE;
                } else {
					tier = tierSolveList->tier;
					gInitializeHashWindow(tier);
					tierSize = gMaxPosOffset[1];
				}
                break;
            case 'a': case 'A':
            	printf("-You chose to automate solving starting from Tier %d.\n"
            			"Keep in mind this process can take quite a long time and\n"
            			"CANNOT be interrupted. Are you sure?\n", tier);
            	if (!ConfirmAction('a')) break;
            	printf("Fully Solving starting from Tier %d...\n\n",tier);
            	for (; tierSolveList != NULL; tierSolveList = tierSolveList->next) {
            		tier = tierSolveList->tier;
					gInitializeHashWindow(tier);
					tierSize = gMaxPosOffset[1];
            		SolveLoopyTier();
				}
            	printf("\n%s is now fully solved!\n", kGameName);
                cont = FALSE;
                break;
            case 'e': case 'E':
            	printf("Exit WITHOUT solving the whole game?\n"
            			"Keep in mind that next time you start the solve, you will\n"
            			"continue from this exact point. Are you sure?\n");
            	if (!ConfirmAction('e')) break;
            	printf("Exiting Retrograde Solver (WITHOUT Fully Solving)...\n");
            	exit(0);
			case 'h': case 'H':
				compareTwoFiles("./a.txt", "./b.txt");
				break;
            default:
                printf("Invalid option!\n");
        }
    }
    printf("Exiting Retrograde Solver...\n\n");
    return GetValueOfPosition(position);
}


/************************************************************************
**
** ERROR HANDLING
**
************************************************************************/

BOOLEAN ConfirmAction(char c) {
	printf("Enter '%c' to confirm, or anything else to cancel\n > ", c);
	return (GetMyChar() == c);
}

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

/* Rather than a Frontier Queue, this uses a sort of hashtable,
with a POSITIONLIST for every REMOTENESS from 0 to REMOTENESS_MAX-1.
It's constant time insert and remove, so it works just fine. */
FRnode**	rWinFR = NULL;	// The FRontier Win Hashtable
FRnode**	rLoseFR = NULL;	// The FRontier Lose Hashtable
FRnode**	rTieFR = NULL;	// The FRontier Tie Hashtable
REMOTENESS  currentWinList, currentLoseList, currentTieList;

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
	//gNumOfPoss bytes = (gNumOfPos / 1,028) KB = (gNumOfPos / 1,056,784) GB
	childCounts = (CHILDCOUNT*) SafeMalloc (tierSize * sizeof(CHILDCOUNT));
	// 255 * 4 bytes = 1,020 bytes = ~1 KB
	rWinFR = (FRnode**) SafeMalloc (REMOTENESS_MAX * sizeof(FRnode*));
	rLoseFR = (FRnode**) SafeMalloc (REMOTENESS_MAX * sizeof(FRnode*)); // ~1 KB
	rTieFR = (FRnode**) SafeMalloc (REMOTENESS_MAX * sizeof(FRnode*)); // ~1 KB
	int r;
	for (r = 0; r < REMOTENESS_MAX; r++)
		rWinFR[r] = rLoseFR[r] = rTieFR[r] = NULL;
	currentWinList = currentLoseList = currentTieList = -1;
}

void rFreeFRStuff() {
	if (childCounts != NULL) SafeFree(childCounts);
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

POSITIONLIST* rRemoveFRList(VALUE value) {
	if (value == win) {
		currentWinList++;
		while (currentWinList < REMOTENESS_MAX) {
			if (rWinFR[currentWinList] != NULL)
				return rWinFR[currentWinList];
			else currentWinList++;
		}
	} else if (value == lose) {
		currentLoseList++;
		while (currentLoseList < REMOTENESS_MAX) {
			if (rLoseFR[currentLoseList] != NULL)
				return rLoseFR[currentLoseList];
			else currentLoseList++;
		}
	} else if (value == tie) {
		currentTieList++;
		while (currentTieList < REMOTENESS_MAX) {
			if (rTieFR[currentTieList] != NULL)
				return rTieFR[currentTieList];
			else currentTieList++;
		}
	}
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
** NAME:        SolveLoopyTier
**
** DESCRIPTION: The heart of the solver. This takes in a tier number,
**				iterates through the current window to generate the
**				frontier, and then calls the "loopy solver" algorithm
**				to solve the current tier.
**
************************************************************************/

POSITION numSolved, sizeOfTier;

void SolveLoopyTier() {
	numSolved = 0; sizeOfTier = 0;
	POSITION pos;
	MOVELIST *moves, *movesptr;
	VALUE value;
	REMOTENESS remoteness;
	TIER t;

	printf("\n--Solving Tier %d...\n",tier);

	printf("Size of tier hash: %llu\n",tierSize);
	printf("Size of hash window: %llu\n",gNumberOfPositions);
	printf("Setting up Child Counters and Frontier Hashtables...\n");
	rInitFRStuff();
	printf("Setting up Tier-Specific DB...\n");
	initDB();
	printf("Doing an sweep of the tier, and setting up the frontier...\n");
	for (pos = 0; pos < tierSize; pos++) { // SET UP PARENTS
		childCounts[pos] = 0;
		if (sanityCheck && !gIsLegalFunPtr(pos)) continue; //skip
		//if (gSymmetries) {
		//	canonPos = gCanonicalPosition(pos);
		//	if (pos != canonPos) continue; // skip, since we'll do canon one later
		//}
		sizeOfTier++;
		// should check in tierdb that this is all empty (undecided)
		value = Primitive(pos);
		if (value != undecided) { // check for primitive-ness
			SetRemoteness(pos,0);
			StoreValueOfPosition(pos,value);
			numSolved++;
			if (gCurrentTierIsLoopy) // if not loopy, no reason to put in frontier
				rInsertFR(value, pos, 0);
		} else {
			moves = movesptr = GenerateMoves(pos);
			if (moves == NULL) { // no chillins
				SetRemoteness(pos,0);
				StoreValueOfPosition(pos,lose);
				numSolved++;
				if (gCurrentTierIsLoopy)
					rInsertFR(lose, pos, 0);
			} else {// make a Child Counter for it
				for (; movesptr != NULL; movesptr = movesptr->next)
					childCounts[pos]++;
				FreeMoveList(moves);
			}
		}
	}
	printf("True size of tier: %lld\n",sizeOfTier);
	printf("Amount of those already solved (primitives): %lld\n",numSolved);
	if (numSolved != sizeOfTier) { // SET UP FRONTIER! & ERROR CHECKING
		printf("Doing an sweep of child tiers, and setting up the frontier...\n");
		// go through all tiers, add them in one by one
		// if a global hash, we have to make sure we only check the right tiers
		int index;
		for (index = 2; index < gNumTiersInHashWindow; index++) {
			t = gTierInHashWindow[index];
			for (pos = gMaxPosOffset[index-1]; pos < gMaxPosOffset[index]; pos++) {
				if (sanityCheck && !gIsLegalFunPtr(pos)) continue; //skip
				//if (gSymmetries) {
				//	canonPos = gCanonicalPosition(pos);
				//	if (pos != canonPos) continue; // skip, since we'll do canon one later
				//}
				value = GetValueOfPosition(pos);
				remoteness = Remoteness(pos);
				if (!((value == tie && remoteness == REMOTENESS_MAX)
						|| value == undecided))
					rInsertFR(value, pos, remoteness);
			}
		}
		SolveWithLoopyAlgorithm(); // LOOPY SOLVER!
	} else printf("Tier is all primitives! No loopy algorithm needed!\n");
	rFreeFRStuff();
	printf("Tier fully solved! Saving to database...\n");
	SaveTierDBToFile();
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
	printf("Beginning the loopy algorithm...\n");
	POSITIONLIST *FRptr;
	// start with lose/win frontiers
	printf("Processing Lose Frontier!\n");
	while ((FRptr = rRemoveFRList(lose)) != NULL)
		LoopyParentsHelper(FRptr, win, currentLoseList);

	printf("Processing Win/Lose Frontiers!\n");
	while ((FRptr = rRemoveFRList(win)) != NULL)
		LoopyParentsHelper(FRptr, lose, currentWinList);

	printf("Amount now solved: %lld\n",numSolved);
	if (numSolved != sizeOfTier) { // We must process ties!
		printf("Processing Tie Frontier!\n");
		while((FRptr = rRemoveFRList(tie)) != NULL)
			LoopyParentsHelper(FRptr, tie, currentTieList);

		printf("Amount now solved: %lld\n",numSolved);
		if (numSolved != sizeOfTier) { // We have undecideds... must make them DRAWs
			printf("Setting undecided to DRAWs...\n");
			POSITION pos;
			for(pos = 0; pos < tierSize; pos++) {
				if (childCounts[pos] > 0) { // no lose/tie children, no/some wins = draw
					SetRemoteness(pos,REMOTENESS_MAX); // a draw
					StoreValueOfPosition(pos, tie);
					numSolved++;
				}
			}
			assert(numSolved == sizeOfTier);
		}
	}
}

void LoopyParentsHelper(POSITIONLIST* list, VALUE valueParents, REMOTENESS remotenessChild) {
	POSITION child, parent;
	FRnode *miniLoseFR = NULL;
	UNDOMOVELIST *parents, *parentsPtr;
	for (; list != NULL; list = list->next) {
		child = list->position;
		parents = parentsPtr = gGenerateUndoMovesToTierFunPtr(child, tier);
		// If no parents, just go on to the next:
		//if (parents == NULL) continue;
		for (; parentsPtr != NULL; parentsPtr = parentsPtr->next) {
			parent = gUnDoMoveFunPtr(child, parentsPtr->undomove);
			if (parent < 0 || parent >= tierSize) {
				printf("ERROR: %llu generated an undo-parent not in current tier!\n", child);
				exit(1);
			}
			// if childCounts is already 0, we don't mess with this parent
			// (already dealt with OR illegal)
			if (childCounts[parent] != 0) {
				// With losing children, every parent is winning, so we just go through
				// all the parents and declare them winning.
				// Same with tie children.
				if (valueParents == win || valueParents == tie) {
					childCounts[parent] = 0; // reset child counter
					if (gCurrentTierIsLoopy || parent >= tierSize) // parent's tier != tier
						rInsertFR(valueParents, parent, remotenessChild+1);
				// With winning children, first decrement the child counter by one. If
				// child counter reaches 0, put the parent not in the FR but in the miniFR.
				} else if (valueParents == lose) {
					childCounts[parent] -= 1;
					if (childCounts[parent] != 0) continue;
					if (gCurrentTierIsLoopy || parent >= tierSize)
						miniLoseFR = StorePositionInList(parent, miniLoseFR);
				}
				SetRemoteness(parent, remotenessChild+1);
				StoreValueOfPosition(parent, valueParents);
				numSolved++;
			}
		}
		FreeUndoMoveList(parents);
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
** READING FILES
** (These are just here as precursors to Deepa Blue Stuff.
** They'll be deprecated in a bit.)
**
************************************************************************/

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
/*
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
************************************************************************/

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

/************************************************************************
**
** DB SAVE FILES
**
************************************************************************/

void initDB() {
	CreateDatabases();
	InitializeDatabases();
	if(LoadDatabase())
		printf("Databases loaded successfully!\n");
	else {
		printf("Couldn't load tierDBs!\n");
		exit(1);
	}
}

// This creates a DB file to save the DB for later. Will be replaced
// by TierDB when it works.
void SaveTierDBToFile() {
	printf("--Saving the Tier Database to File...\n");
	if (SaveDatabase())
		printf ("  Database successfully saved!\n");
	else {
		printf("Couldn't save tierDB!\n");
		exit(1);
	}
	FILE *fp; POSITION p;
	sprintf(filename, "./a%d.txt", tier);
	fp = fopen(filename,"w");
	for (p = 0; p < tierSize; p++)
		fprintf (fp, "%d\t%lld\t%d\n", GetValueOfPosition(p), p, Remoteness(p));
	fclose(fp);
}


/************************************************************************
**
** DEBUG (TO HELP MODULE WRITERS)
**
************************************************************************/

POSITION GetMyPosition() {
    char inString[MAXINPUTLENGTH];
    scanf("%s",inString);
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

void debugMenu() {
	TIER hashWindowTier = kBadTier;
	TIER TiersToSolve = 0;

	printf(	"\n\n=====Welcome to the Debug Menu!=====\n");
	BOOLEAN TC, NOTP, IL, GUMTT, UDM;
	if (gTierSolveListPtr != NULL) {
		TIERLIST* tierSolveList = gTierSolveListPtr;
		for (; tierSolveList != NULL; tierSolveList = tierSolveList->next)
			TiersToSolve++;
	}
	TIER tierArray[TiersToSolve];

	TC = !(gTierChildrenFunPtr == NULL);
	NOTP = !(gNumberOfTierPositionsFunPtr == NULL);
	IL = !(gIsLegalFunPtr == NULL);
	GUMTT = !(gGenerateUndoMovesToTierFunPtr == NULL);
	UDM = !(gUnDoMoveFunPtr == NULL);

	char c; POSITION p, p2, p3; TIER t, t2; BOOLEAN check, check2;
	TIERLIST *ptr, *list, *listptr; int index, i; TIERPOSITION tp;
	UNDOMOVELIST *ulist, *ulistptr; MOVELIST *mlist, *mlistptr;
	while(TRUE) {
		printf("\n\t-----Debug Options:-----\n"
			"\t(Current Hash Window: %d)\n"
			"\tp)\tGo to (P)OSITION-DEBUG screen (for current hash window)\n"
			"\tg)\tTest g(G)enerateUndoMovesToTier (for current hash window)\n"
			"\tu)\tTest g(U)nDoMove (for current hash window)\n"
			"\tc)\t(C)hange current HASH WINDOW (gInitializeHashWindow)\n"
			"\td)\t(D)raw and check Tier \"Tree\" (gTierSolveList, gTierChildren)\n"
			"\tb)\t(B)ack to main solver menu\n"
			"\nSelect an option:  ", hashWindowTier);
		c = GetMyChar();
		switch(c) {
			/*
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
				*/
			case 'g': case 'G':
				if (!GUMTT) {
					printf("gGenerateUndoMovesToTier isn't written, so...\n");
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
								/*if (PTT) {
									printf("Calling gPositionToTier to ensure correctness.\n");
									t2 = gPositionToTierFunPtr(p2);
									if (t == t2) printf("Returned tier confirms that this is in TIER: %d\n", t);
									else printf("---ERROR FOUND: Returned TIER: %d, not %d!", t2, t);
								}*/
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
			case 'c': case 'C':
				printf("\nEnter the TIER to change to, or non-number to go back:\n> ");
				p = GetMyPosition();
				if (p == kBadPosition) break;
				t = (TIER)p;
				printf("Calling gInitializeHashWindow on TIER: %d\n", t);
				gInitializeHashWindow(t);
				hashWindowTier = t;
				break;
			case 'd': case 'D':
				if (!TC || gTierSolveListPtr == NULL) {
					printf("gTierSolveList and/or gTierChildren isn't written, so...\n");
					break;
				}
				printf("\nNow checking all Tiers for correctness. This could take a while.\n");
				if (ConfirmAction('c')) {
					check = TRUE;
					ptr = gTierSolveListPtr;
					index = 0;
					for (; ptr != NULL; ptr = ptr->next) {
						t = ptr->tier;
						for (i = 0; i < index; i++) {
							if (t == tierArray[i]) {
								printf("---ERROR FOUND: Duplicate Tiers in List! (Indexes %d and %d)\n", i, index);
								check = FALSE;
							}
						}
						list = listptr = gTierChildrenFunPtr(t);
						for (; listptr != NULL; listptr = listptr->next) {
							t2 = listptr->tier;
							if (t == t2) continue;
							check2 = FALSE;
							for (i = 0; i < index; i++) {
								if (t2 == tierArray[i]) {
									check2 = TRUE;
									break;
								}
							} if (!check2) {
								printf("---ERROR FOUND: Tier %d's child, Tier %d, is not solved before it!\n", t, t2);
								check = FALSE;
							}
						}
						FreeTierList(list);
						tierArray[index] = t;
						index++;
					}
					if (check) printf("No Errors! Congratulations!\n");
					else printf("\nError(s) Found! Be sure to fix them!\n");
				} break;
			case 'b': case 'B':
				return;
			default:
				printf("Invalid option!\n");
		}
	}
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
	fp = fopen("./db.txt","w");
	for (p = 0; p < gNumberOfPositions; p++)
		fprintf (fp, "%d\t%lld\t%d\n", GetValueOfPosition(p), p, Remoteness(p));
	fclose(fp);
}

/* // wrote this for Tic Tac Tier - should work for most games
void writeCurrentDBToTierFiles() {
	FILE *fp; POSITION p, p2;
	int i, GlobalContext = generic_hash_cur_context();
	for (i = 0; i <= 9; i++) {
		char filename[80], *board, turn;
		sprintf(filename,"./b%d.txt", i);
		fp = fopen(filename,"w");
		printf("Tier %d, with %llu...\n", i, gNumberOfTierPositionsFunPtr(i));
		for (p = 0; p < gNumberOfTierPositionsFunPtr(i); p++) {
			generic_hash_context_switch(Tier0Context+i);
			board = PositionToBlankOX(p);
			turn = WhoseTurn(p);
			generic_hash_context_switch(GlobalContext);
			p2 = BlankOXToPosition(board, turn);
			fprintf (fp, "%d\t%lld\t%d\n", GetValueOfPosition(p2), p, Remoteness(p2));
		}
		fclose(fp);
	}
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
		printf("My Efficiency: %f", (float)efficiency/(float)line);
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

// $Log: not supported by cvs2svn $
// Revision 1.9  2006/07/30 20:00:26  max817
// Just fixed a small file-writing error.
//
// Revision 1.8  2006/07/29 08:35:33  max817
// gPositionToTierPosition abilities added. Now the solver writes makeshift
// tierdb files (to have the info to translate to ACTUAL db files later),
// and Tic-Tac-Tier now correctly implements both gPosToTierPos and
// gInitHashWindow. Also fixed plenty of bugs in both solver and game.
//
// Revision 1.7  2006/07/26 03:17:28  max817
// The Retrograde Solver now handles the Tier-Gamesman API! It's very rough
// draft for now, doesn't use tier-windows (gInitHashWindow) or tier-specific
// databases (gPositionToTierPosition) yet, but it will soon. As far as I
// know it IS solving games correctly, at least compared to the regular
// loopy solver. More updates coming soon.
//
// Revision 1.6  2006/07/11 08:53:34  max817
// Just a few last-minute additions to the debugger.
//
// Revision 1.5  2006/07/11 07:58:34  max817
// The first of many changes to implement the new "Tier-Gamesman".
// Added tiergamesman.txt to the repository, which contains a wealth
// of information on the new system.
// Changed 7 core files to implement the new API that Tier-Gamesman modules
// must fulfill, and added a rough debugger to the Retrograde Solver.
// See doc/tiergamesman.txt for the full documentation.
//
// Revision 1.4  2006/05/25 04:22:51  max817
// Just an update to fix a small bug in the solver's algorithm that incorrectly labeled some ties as draws. Also added the TRUE code for RetrogradeTierValue to baghchal.c, though it's commented for now (since the solver as it stands can't handle 421 tiers too well).
//
// Revision 1.3  2006/04/17 07:36:38  max817
// mbaghchal.c now has no solver debug code left (i.e. it's independent of anything in solveretrograde.c) and has variants implemented correctly. Aside from the POSITION size stuff, it should be near its final version.
// As for the solver, all of the main features I wanted to implement are now implemented: it is almost zero-memory (aside from memdb usage), and it's possible to stop and save progress in the middle of solving. Also, ANY game can use the solver now, not just Bagh Chal. All in all, it's close to the final version (for this semester). -Max
//
// Revision 1.2  2006/04/13 21:40:56  max817
// A few changes to mbaghchal and the solver here and then, but mostly
// submitting to fix a small bug with solveretrograde.h that didn't allow
// some compilers/linkers to "make" properly. -Max
//
// Revision 1.1  2006/04/12 03:02:12  max817
// This is the big update that moves the Retrograde Solver from mbaghchal.c
// to its own set of new files, solveretrograde.c and solveretrograde.h.
// Exact details on the exact changes I made to the core files can be found
// in a comment on solveretrograde.c. -Max
//
