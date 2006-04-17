// $Id: solveretrograde.c,v 1.3 2006-04-17 07:36:38 max817 Exp $
// $Log: not supported by cvs2svn $
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

/*
FOR GAMESCRAFTERS: Here's EXACTLY what I changed in the core:
--globals.h: Added the line:

extern int (*gRetrogradeTierValue)(POSITION);

This is a function pointer, defined by individual game modules themselves,
which also serves to tell Gamesman to use this solver.
This means the decision to use this solver is decided at compile-time
rather than run-time (by the module, not by command-line args).

--globals.c: Added the line:

int (*gRetrogradeTierValue)(POSITION) = NULL;

This sets the pointer to NULL (and thus, the use of the retrograde
solver to FALSE) by default. It's up to the module to make this not NULL.

--main.c: Added line:

#include "solveretrograde.h"

And, in "SetSolver()", added this to the big if statement:
...
else if (gRetrogradeTierValue != NULL)
	gSolver = &DetermineRetrogradeValue;
...

This just make sure that if the function pointer is indeed implemented,
the correct solver is loaded.

--core/Makefile: (Obvious changes):

SOLVER_RETROGRADE = solveretrograde$(OBJSUFFIX)

SOLVERS= ... $(SOLVER_RETROGRADE)

--Other than that, added this file and solveretrograde.h to the core files.

Any questions, feel free to ask me.
This file in general is a bit unorganized for now, but I'll clean it all
up in a later update before Monday.

-Max
*/

/************************************************************************
**
** NAME:        RetrogradeTierValue (DOCUMENTATION)
**
** DESCRIPTION: The ONE function the solver requires modules to code.
**
**				RetrogradeTierValue(kBadPosition)
**					--returns tierMax (highest tier number, >= 0)
**
**				RetrogradeTierValue(any other POSITION >= 0, <= gNumberOfPositions)
**					--returns tier value of position (0 <= value <= tierMax)
**					--if value == -1, the solver skips it (use for illegal positions)
**
**				Remember, a "tier" value is defined such that for any position,
**				all of its children have a tier value less than or equal to its
**				own tier value. Currently the solver doesn't check that this holds,
**				but a future release will (using Eric's Slice API).
**				Thus there are MANY ways to define tiers (they are in no way unique).
**				Module writers then should do their best to define tiers such that
**				the total number of positions in each tier is minimized. If can,
**				define non-loopy tiers (such that for a tier, all of its positions
**				have children with tier values strictly less than the tier value),
**				as that greatly speeds up the solving.
**
**				If there is no way to define "tiers" in your game, or if you simply
**				don't WANT to exploit the retrograde solver's splitting of the data,
**				RetrogradeTierValue can simply ALWAYS RETURN 0 for any argument,
**				and the solver will correctly solve it (but will probably take forever).
**
**				For further details, talk to Max. I'll provide a more complete
**				documentation later.
**
**				As a side note, IF the tiers are defined correctly
**				(i.e. children's tier value is lower than or equal to parent)
**				then I think the solver is GUARANTEED to provide the correct solve,
**				regardless of how "bad" the tiers are defined. Must prove?
**
************************************************************************/

/* MAX'S SELF NOTES: What's next:
	-Once Scott's filedb is up and running, this is (mostly) zero-memory.
	-Fix up file usage:
		In initFiles: Deal with C's file open limit more gracefully.
		In SolveWithDAlg: Use BerkeleyDB instead of "piggy-back" files.
	-Add a "cache"-like thing that holds position values in memory rather than
		file, for a definite speed up
	-Progress bars for initFiles, SolveTier, and SolveWithDAlg!
	-Implement the loopy solver and compare run times with D Algorithm.
		If D Algorithm faster, make it even better.
*/

#include "gamesman.h"
#include "solveretrograde.h"

/* Global variables */
int tierMax, variant;
char filename[80];
POSITION positionsLeft, *tierSize;
POSITIONLIST* childlist;
REMOTENESS maxUncorruptedWinRem;
REMOTENESS minTieRem;
BOOLEAN seenDraw;

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
    int tier;
    char c;
	printf("\n\n=====Welcome to the Retrograde Solver!=====\n");
	printf("Currently solving game (%s) with variant (%d)\n", kGameName, variant);
    printf("Current Initial Position:  \n");
    PrintPosition(position, "Initial", 0);
    tierMax = gRetrogradeTierValue(kBadPosition);
	if (tierMax < 0) {
		printf("ERROR: gRetrogradeTierValue not correctly defined for kBadPosition argument.\n");
		exit(1);
	}
	printf("Solving up to Tier %d, with %lld total positions.\n", tierMax, gNumberOfPositions);
	tierSize = (POSITION*) SafeMalloc((tierMax+1) * sizeof(POSITION));
	tier = LoadProgressFromFile(); //sets positionsLeft
	if (tier == -1) { // no save, a mini-menu for initialization
		tier = 0;
		while(cont) {
			printf("\n\n===RETROGRADE SOLVER MENU for game: %s===\n", kGameName);
			printf("Before retrograde solving can begin, the tier values for all\n"
					"possible positions in the game must be calculated and stored\n"
					"into files. This could take quite some time.\n");
			printf("\tOptions:\n"
				   "\ts)\t(S)tart tier value calculations.\n"
				   "\ta)\t(A)utomate the entire solving procedure.\n"
				   "\te)\t(E)xit the solver\n"
				   "\th)\t(H)AXX Compare the two database files, a.txt and b.txt\n"
				   "\nSelect an option:  ");
			c = GetMyChar();
		   	switch(c) {
				case 's': case 'S':
					printf("-Begin the process?\n");
					if (!ConfirmAction('s')) break;
					initFiles(); //sets positionsLeft
					cont = FALSE;
					break;
				case 'a': case 'A':
					printf("-You chose to automate solving all the way to Tier %d.\n"
							"Keep in mind this process can take quite a long time and\n"
							"CANNOT be interrupted. Are you sure?\n", tierMax);
					if (!ConfirmAction('a')) break;
					initFiles();
					printf("\n Solving all the way to maximum Tier %d...\n\n",tierMax);
					for (; tier <= tierMax; tier++)
						SolveTier(tier);
					printf("\n%s is now fully solved!\n", kGameName);
					removeFiles();
					printf("Exiting Retrograde Solver...\n\n");
					return GetValueOfPosition(position);
				case 'e': case 'E':
					printf("-You chose to exit. Are you sure?\n");
					if (!ConfirmAction('e')) break;
					printf("Exiting Retrograde Solver (WITHOUT Solving)...\n");
					exit(0);
				case 'h': case 'H':
					compareTwoFiles("./retrograde/a.txt", "./retrograde/b.txt");
					break;
				default:
					printf("Invalid option!\n");
			}
		}
		cont = TRUE;
	} else HitAnyKeyToContinue(); // else tier is legal, continue solving.
    while(cont) {
        printf("\n\n===RETROGRADE SOLVER MENU for game: %s===\n", kGameName);
        printf("Ready to solve tier (%d), which contains (%lld) positions.\n", tier, tierSize[tier]);
        printf("There are %lld total positions (in %d tiers) still left to solve.\n", positionsLeft, tierMax-tier+1);
        printf("\tOptions:\n"
               "\ts)\t(S)olve the next tier.\n"
               "\ta)\t(A)utomate the solving for all the tiers left.\n"
               "\te)\tSave Progress and (E)xit\n"
               "\td)\t(D)iscard all Progress and Exit\n"
        	   "\th)\t(H)AXX Compare the two database files, a.txt and b.txt\n"
               "\nSelect an option:  ");
        c = GetMyChar();
        switch(c) {
			case 's': case 'S':
				printf("-Solve Tier %d?\n", tier);
				if (!ConfirmAction('s')) break;
	            SolveTier(tier);
	            if (tier == tierMax) {
	            	printf("\n%s is now fully solved!\n", kGameName);
                	removeFiles();
                	cont = FALSE;
                } else tier++;
                break;
            case 'a': case 'A':
            	printf("-You chose to automate solving from Tier %d to Tier %d.\n"
            			"Keep in mind this process can take quite a long time and\n"
            			"CANNOT be interrupted without losing all progress up to\n"
            			"this point. Are you sure?\n", tier, tierMax);
            	if (!ConfirmAction('a')) break;
            	printf("Solving from Tier %d to maximum Tier %d...\n\n",tier,tierMax);
            	for (; tier <= tierMax; tier++)
            		SolveTier(tier);
            	printf("\n%s is now fully solved!\n", kGameName);
                removeFiles();
                cont = FALSE;
                break;
            case 'e': case 'E':
            	printf("-You chose to save ALL progress and exit. Are you sure?\n");
           		if (!ConfirmAction('e')) break;
           		SaveProgressToFile(tier);
           		printf("Exiting Retrograde Solver (WITHOUT Solving)...\n");
                exit(0);
            case 'd': case 'D':
            	printf("-You chose to discard ALL progress and exit WITHOUT solving.\n"
            			"Keep in mind this will delete all files and you will lose\n"
            			"ALL progress completed up to this point. Are you sure?\n");
            	if (!ConfirmAction('d')) break;
            	removeFiles();
            	printf("Exiting Retrograde Solver (WITHOUT Solving)...\n");
            	exit(0);
			case 'h': case 'H':
				compareTwoFiles("./retrograde/a.txt", "./retrograde/b.txt");
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
	printf("\nIt appears one more files have been corrupted.\n"
			"For safety, deleting all files...\n");
	removeFiles();
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
************************************************************************/

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
		skipLineSolveFile(fileR);
		if (GetValueOfPosition(pos) == undecided) {
			StoreValueOfPosition(pos, tie);
			SetRemoteness(pos,REMOTENESS_MAX);
		} else if (Visited(pos)) UnMarkAsVisited(pos); // must've been corrupted, now TRUE
	}
	if (fclose(fileR) == EOF) FileCloseError();
	sprintf(filename,"./retrograde/1.solve");
	remove(filename);
	sprintf(filename,"./retrograde/2.solve");
	remove(filename);
	printf("Tier fully solved!...\n");
	positionsLeft -= sizeOfTier;
}

// LOOPY SOLVER!
// Parent pointers instead of child pointers (how to do?)
// Priority queues (just a file, one per REMOTENESS, since things
//   with same remoteness it actually doesn't matter the order.
// After this, pretty straightforward.

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
************************************************************************/

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

/************************************************************************
**
** READING FILES
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

// This just skips a line in the solve file
void skipLineSolveFile(FILE* fp) {
	int c;
	while ((c = getc(fp)) != '\n') {
		if (c == EOF) FileSyntaxError();
	}
}

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
** INITIALIZATION AND CLEANUP
**
************************************************************************/

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

/************************************************************************
**
** SOLVER PROGRESS FILES
**
************************************************************************/

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

// This creates a DB file to save the DB for later. Will be replaced
// by Scott's DB when it works.
void SaveDBToFile() {
	FILE* fp;
	POSITION pos;
	VALUE value;

	printf("--Saving the Database to File...\n");
	sprintf(filename,"./retrograde/%s_%d_DB.save",kDBName,variant);
	if ((fp = fopen(filename, "w")) == NULL) FileOpenError();
	for (pos = 0; pos < gNumberOfPositions; pos++) {
		value = GetValueOfPosition(pos);
		if (value != undecided)
			if (fprintf(fp, "%lld %d %d \n", pos, value, Remoteness(pos)) < 0)
				FileWriteError();
	}
	printf("  Database Saved\n");
	if (fclose(fp) == EOF) FileCloseError();
}

// This loads a saved DB file, and deletes the file. Will be replaced
// by Scott's DB when it works.
void LoadDBFromFile() {
	FILE* fp;
	POSITION pos, c;
	VALUE value; REMOTENESS remoteness;

	printf("--Loading the Database from File...\n");
	sprintf(filename,"./retrograde/%s_%d_DB.save",kDBName,variant);
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

// The following are just helpers I use to check the correctness
// of the solver, vs. the normal loopy solver.

/* HAXX that writes database to file, usually copy-pasted to a module */
void writeCurrentDBToFile() {
	FILE *fp; POSITION p;
	fp = fopen("./retrograde/db.txt","w");
	for (p = 0; p < gNumberOfPositions; p++)
		fprintf (fp, "%d\t%lld\t%d\n", GetValueOfPosition(p), p, Remoteness(p));
	fclose(fp);
}

// a helper that compares two database outfiles
void compareTwoFiles(char *mine, char *theirs) {
	FILE *my, *his;
	if ((my = fopen(mine, "r")) == NULL) {
		printf("Couldn't open %s\n", mine);
		return;
	}
	if ((his = fopen(theirs, "r")) == NULL) {
		printf("Couldn't open %s\n", theirs);
		return;
	}
	int c, line = 1;
	while ((c = getc(his)) != EOF) {
		if (c == '3') {
			skipToNewline(his);
			skipToNewline(my);
		} else {
			if (getc(my) != c) {
				printf("%d\n",line);
				skipToNewline(my);
				skipToNewline(his);
			} else {
				while (c != '\n') {
					if (getc(my) != (c = getc(his))) {
						printf("%d\n",line);
						skipToNewline(my);
						skipToNewline(his);
						break;
					}
				}
			}
		}
		line++;
	} if ((c = getc(my)) != EOF) printf("Files don't match up!\n");
}

void skipToNewline(FILE* fp) {
	int c = getc(fp);
	while (c != '\n') c = getc(fp);
}
