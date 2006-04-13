// $Id: solveretrograde.c,v 1.2 2006-04-13 21:40:56 max817 Exp $
// $Log: not supported by cvs2svn $
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
** DATE:	2006-04-11
**
** UPDATE HIST: -2006.4.11 = Just added fully functional solver to Gamesman.
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
** NAME:        RetrogradeTierValue
**
** DESCRIPTION: The ONE function the solver requires modules to code.
**				I'll add documentation here... later.
**
**				RetrogradeTierValue(kBadPosition)
**					--returns tierMax (highest tier number, >= 0)
**
**				RetrogradeTierValue(any other POSITION >= 0, <= gNumberOfPositions)
**					--returns tier value of position (0 <= value <= tierMax)
**					--if value == -1, the solver skips it (use for illegal positions)
**
**				As a side note, IF defined with the property that my
**				tier number is less than or equal to ALL of my parent's
**				tier values, then I think the solver is GUARANTEED to provide
**				the correct tree, regardless of how "bad" the tiers are
**				defined. Must prove?
**				In the future (with Slice API) the solver will check this
**				for you anyway.
**
************************************************************************/

/* Global variables */
int tierMax;
char filename[80];
POSITION positionsLeft;
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

// HANDLE ERRORS BETTER!!
// Make HandleErrorAndExit() take in a STRING error arg.
// Add "ERROR!" to all string error args.
// Add HandleErrorAndExit()s to fprintfs (if < 0, error).

// LOOPY SOLVER!
// Parent pointers instead of child pointers (how to do?)
// Priority queues (just a file, one per REMOTENESS, since things
//   with same remoteness it actually doesn't matter the order.
// After this, pretty straightforward.

VALUE DetermineRetrogradeValue(POSITION position) {
	BOOLEAN cont = TRUE;
    int tier;
    char c;
	printf("\n\nWelcome to the Retrograde Solver!\n");
	printf("Currently solving game (%s) with variant (%d)\n", kGameName, getOption());
    printf("Current Initial Position:  \n");
    PrintPosition(position, "Initial", 0);
    tierMax = gRetrogradeTierValue(kBadPosition);
	if (tierMax < 0) {
		printf("ERROR: gRetrogradeTierValue not correctly defined for kBadPosition argument.\n");
		exit(1);
	}
	printf("Solving up to Tier %d, with %lld total positions.\n\n", tierMax, gNumberOfPositions);
	tier = LoadProgressFromFile(); //sets positionsLeft
	if (tier == -1) {
		printf("Before retrograde solving can begin, the tier values for all\n"
				"possible positions in the game must be calculated and stored\n"
				"into files. This could take quite some time.\n"
				"Press Enter to begin the process, or CTRL-C to quit without solving.\n");
		HitAnyKeyToContinue();
		initFiles(); //sets positionsLeft
		tier = 0;
	} // else tier is legal, continue solving.
    while(cont) {
        printf("\n\nRETROGRADE SOLVER MENU for game: %s\n", kGameName);
        printf("Ready to solve tier: %d\n", tier);
        printf("There are %lld positions (in %d tiers) still left to solve.\n", positionsLeft, tierMax-tier+1);
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
				printf("Solve Tier %d?\n", tier);
				if (!ConfirmAction('s')) break;
	            SolveTier(tier);
	            if (tier == tierMax) {
	            	printf("%s is now fully solved!\n", kGameName);
                	removeFiles();
                	cont = FALSE;
                } else tier++;
                break;
            case 'a': case 'A':
            	printf("You chose to automate solving from Tier %d to Tier %d.\n"
            			"Keep in mind this process can take quite a long time and\n"
            			"CANNOT be interrupted without losing all progress up to\n"
            			"this point. Are you sure?\n", tier, tierMax);
            	if (!ConfirmAction('a')) break;
            	printf("Solving from Tier %d to maximum Tier %d...\n\n",tier,tierMax);
            	for (; tier <= tierMax; tier++)
            		SolveTier(tier);
            	printf("%s is now fully solved!\n", kGameName);
                removeFiles();
                cont = FALSE;
                break;
            case 'e': case 'E':
            	printf("You chose to save ALL progress and exit. Are you sure?\n");
           		if (!ConfirmAction('e')) break;
           		SaveProgressToFile(tier);
           		printf("Exiting Retrograde Solver (WITHOUT Solving)...\n\n");
                exit(0);
            case 'd': case 'D':
            	printf("You chose to discard ALL progress and exit WITHOUT solving.\n"
            			"Keep in mind this will delete all files and you will lose\n"
            			"ALL progress completed up to this point. Are you sure?\n");
            	if (!ConfirmAction('d')) break;
            	removeFiles();
            	printf("Exiting Retrograde Solver (WITHOUT Solving)...\n\n");
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

void HandleErrorAndExit() {
	printf("\nIt appears one more files have been corrupted.\n"
			"For safety, deleting all files...\n");
	removeFiles();
	printf("Please restart the game and solve from the start.\n");
	exit(1);
}

BOOLEAN ConfirmAction(char c) {
	printf("Enter '%c' to confirm, or anything else to cancel\n > ", c);
	return (GetMyChar() == c);
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
**				In the end, it marks unvisited children as draws.
**
************************************************************************/

void SolveTier(int tier) {
	FILE *fileW, *fileR;
	int fR = 1;
	POSITION pos, child, tierSize = 0, numSolved = 0, numCorrupted = 0;
	MOVELIST *moves, *children;
	REMOTENESS remoteness, maxWinRem, minLoseRem;
	VALUE value;
	BOOLEAN seenLose;

	printf("--Solving Tier %d...\n",tier);

	printf("Reading/Writing appropriate files...\n");

	sprintf(filename,"./retrograde/%s_%d.tier",kDBName,tier);
	if ((fileR = fopen(filename, "r")) == NULL) {
		printf("Couldn't open TIER file %s\n", filename);
		HandleErrorAndExit();
	}
	sprintf(filename,"./retrograde/%s_1.solve",kDBName);
	if ((fileW = fopen(filename, "w")) == NULL) {
		printf("Couldn't open SOLVE file %s\n", filename);
		HandleErrorAndExit();
	}

	printf("Doing an initial sweep, and creating child pointers (could take a WHILE)...\n");

	while((pos = readPos(fileR)) != kBadPosition) {
		tierSize++;
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
	fclose(fileR);
	fclose(fileW);
	printf("Size of tier: %lld\n",tierSize);
	if (numSolved != tierSize) {
		fR = SolveWithDelgadilloAlgorithm(tierSize, numSolved, numCorrupted);
		printf("Setting undecided to DRAWs and correcting corruption...\n");
	}
	else printf("Tier is non-loopy! Setting undecided to DRAWs...\n");
	sprintf(filename,"./retrograde/%s_%d.solve",kDBName,fR);
	if ((fileR = fopen(filename, "r")) == NULL) {
		printf("Couldn't open SOLVE file %s\n", filename);
		HandleErrorAndExit();
	}
	while((pos = readPos(fileR)) != kBadPosition) {
		skipLineSolveFile(fileR);
		if (GetValueOfPosition(pos) == undecided) {
			StoreValueOfPosition(pos, tie);
			SetRemoteness(pos,REMOTENESS_MAX);
		} else if (Visited(pos)) UnMarkAsVisited(pos); // must've been corrupted, now TRUE
	}
	fclose(fileR);
	sprintf(filename,"./retrograde/%s_1.solve",kDBName);
	remove(filename);
	sprintf(filename,"./retrograde/%s_2.solve",kDBName);
	remove(filename);
	printf("Tier fully solved!...\n");
	positionsLeft -= tierSize;
}

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

int SolveWithDelgadilloAlgorithm(POSITION tierSize, POSITION numSolved, POSITION numCorrupted) {
	FILE *fileW, *fileR;
	int numUnknownChildren, type, fW = 2, fR = 1;
	POSITION pos, child;
	POSITIONLIST *newchildlist, *copy;
	REMOTENESS remoteness, maxWinRem, minLoseRem, maxCorruptedWinRem;
	VALUE value;
	BOOLEAN change = TRUE, seenLose, corruptedWin, corruptedLose, update;

	//Invariant: ALL that's left to check are corrupted(wins) and unknown Tier N positions.
	while (change) {
		change = FALSE;
		if (numSolved == tierSize) continue; // if we're done, then stop the loop
		printf("%lld Positions still unsolved ", tierSize-numSolved);
		printf("(%lld corrupted). Doing another sweep...\n", numCorrupted);
		sprintf(filename,"./retrograde/%s_%d.solve",kDBName, fR);
		if ((fileR = fopen(filename, "r")) == NULL) {
			printf("Couldn't open SOLVE file %s\n", filename);
			HandleErrorAndExit();
		}
		sprintf(filename,"./retrograde/%s_%d.solve",kDBName, fW);
		if ((fileW = fopen(filename, "w")) == NULL) {
			printf("Couldn't open SOLVE file %s\n", filename);
			HandleErrorAndExit();
		}
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
		fclose(fileR);
		fclose(fileW);
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
		if (c < '0' || c > '9') { // SHOULDN'T be the case
			printf("ERROR: file in incorrect format!\n");
			HandleErrorAndExit();
		}
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
	if (getc(fp) != ' ' && type != 'w' && type != 'l' && type != 'u') {
		printf("ERROR: file in incorrect format!\n");
		HandleErrorAndExit();
	}
	switch(type) {
		case 'u':
			if ((c = readPos(fp)) == kBadPosition || c < 0 || c > REMOTENESS_MAX) {
				printf("ERROR: file in incorrect format!\n");
				HandleErrorAndExit();
			} minTieRem = c;
			if ((c = readPos(fp)) == kBadPosition) {
				printf("ERROR: file in incorrect format!\n");
				HandleErrorAndExit();
			} seenDraw = c;
		case 'l':
			if ((c = readPos(fp)) == kBadPosition || c < 0 || c > REMOTENESS_MAX) {
				printf("ERROR: file in incorrect format!\n");
				HandleErrorAndExit();
			} maxUncorruptedWinRem = c;
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
		if (c == EOF) {
			printf("ERROR: file in incorrect format!\n");
			HandleErrorAndExit();
		}
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
		fprintf(fp, "%lld ", children->position);
	fprintf(fp, "\n");
	if (copy != NULL) // Should ALWAYS be the case, but a safety check
		FreePositionList(copy);
}

void writeCorruptedWinToFile(FILE* fp, POSITION position, POSITIONLIST *children) {
	fprintf(fp, "%lld w ", position);
	writeChildrenToFile(fp, children);
}

void writeCorruptedLoseToFile(FILE* fp, POSITION position, POSITIONLIST *children,
							REMOTENESS maxUncorruptedRem) {
	fprintf(fp, "%lld l %d ", position, maxUncorruptedRem);
	writeChildrenToFile(fp, children);
}

void writeUnknownToFile(FILE* fp, POSITION position, POSITIONLIST *children,
						REMOTENESS maxUncorruptedRem, REMOTENESS minTieRem, BOOLEAN seenDraw) {
	fprintf(fp, "%lld u %d %d %d ", position, minTieRem, seenDraw, maxUncorruptedRem);
	writeChildrenToFile(fp, children);
}

/************************************************************************
**
** INITIALIZATION AND CLEANUP
**
************************************************************************/

// This initializes the tier files
void initFiles() {
	int f, tier;
	FILE **files;
	POSITION pos;

	positionsLeft = gNumberOfPositions;
	printf("We're going to make these many files: %d\n", tierMax+1);
	printf("C says we can open this many files at once: %d\n", FOPEN_MAX);
	if (tierMax+1 > FOPEN_MAX) {
		printf("It looks like we can't open that many files! I'll deal with this later...\n");
		exit(2);
	}
	mkdir("retrograde", 0755);
	// INITIALIZE ALL THE TIER FILES:
	files = (FILE**) SafeMalloc(tierMax * sizeof(FILE*));
	// Open all TierMax Files
	printf("  Opening all the files...\n");
	for (f = 0; f <= tierMax; f++) {
		sprintf(filename,"./retrograde/%s_%d.tier",kDBName,f);
		if ((files[f] = fopen(filename, "w")) == NULL) {
			printf("ERROR: Couldn't open file %s\n", filename);
			HandleErrorAndExit();
		}
	}
	printf("  Writing all the files (could take FOREVER)...\n");
	for (pos = 0; pos < gNumberOfPositions; pos++) {
		tier = gRetrogradeTierValue(pos);
		if (tier == -1) {
			positionsLeft--;
			continue;
		} if (tier < 0 || tier > tierMax) {
			printf("ERROR: gRetrogradeTierValue not correctly defined: returned %d for position %lld\n", tier, pos);
			HandleErrorAndExit();
		}
		//FOR SLICE API: SetTier(pos, tier);
		fprintf(files[tier], "%lld ", pos);
	}
	// Close all the files
	printf("  Closing all the files...\n");
	for (f = 0; f <= tierMax; f++)
		fclose(files[f]);
	// Free the files pointer
	//SafeFree(files); This is commented out because it causes a glitch!
}

// This removes ALL files created by the solver (ignores errors)
void removeFiles() {
	printf("  Deleting all the files...\n");
	int f;
	// Tier files
	for (f = 0; f <= tierMax; f++) {
		sprintf(filename,"./retrograde/%s_%d.tier",kDBName,f);
		remove(filename);
	}
	// Solve files
	sprintf(filename,"./retrograde/%s_1.solve",kDBName);
	remove(filename);
	sprintf(filename,"./retrograde/%s_2.solve",kDBName);
	remove(filename);
	// Save files
	sprintf(filename,"./retrograde/%s_TIER.save",kDBName);
	remove(filename);
	sprintf(filename,"./retrograde/%s_DB.save",kDBName);
	remove(filename);
}

/************************************************************************
**
** SOLVER PROGRESS FILES
**
************************************************************************/

// This saves the current progress to a file.
void SaveProgressToFile(int tier) {
	FILE* fp;
	printf(" -Creating Save File...\n");
	sprintf(filename,"./retrograde/%s_TIER.save",kDBName);
	if ((fp = fopen(filename, "w")) == NULL) {
		printf("ERROR: Couldn't open file %s\n", filename);
		HandleErrorAndExit();
	}
	fprintf(fp, "VARIANT=%d \nPROGRESS=%d \nPOSLEFT=%lld \n", getOption(), tier, positionsLeft);
	printf(" -Save File created.\n");
	fclose(fp);
	SaveDBToFile();
	printf(" Progress properly saved.\n"
			"To guarantee correct solving, make sure that the game code is not\n"
			"modified before restarting the solving. Also make sure that the files\n"
			"in the 'retrograde' directory are not tampered with.\n");
}

// This loads the progress from a file, but also checks that every file
// that SHOULD be there is there. Then it deletes the file.
// Returns -1 if there is no save file..
int LoadProgressFromFile() {
	FILE* fp;
	int f, variant, tier;

	sprintf(filename,"./retrograde/%s_TIER.save",kDBName);
	if ((fp = fopen(filename, "r")) == NULL) // no savefile
		return -1;
	printf(" -Loading Save File...\n");
	if ((getc(fp)) != 'V' || (getc(fp)) != 'A' || (getc(fp)) != 'R' ||
		(getc(fp)) != 'I' || (getc(fp)) != 'A' || (getc(fp)) != 'N' ||
		(getc(fp)) != 'T' || (getc(fp)) != '=') {
		printf("Save File corrupted!\n");
		HandleErrorAndExit();
	}
	if ((variant = readPos(fp)) == kBadPosition || variant != getOption()) {
		printf("ERROR: Current variant: %d, but previously solving variant: %d\n", getOption(), variant);
		printf("Please restart the game and solve with the correct variant selected.\n");
		exit(1);
	}
	if ((getc(fp)) != '\n' || (getc(fp)) != 'P' || (getc(fp)) != 'R' ||
		(getc(fp)) != 'O' || (getc(fp)) != 'G' || (getc(fp)) != 'R' ||
		(getc(fp)) != 'E' || (getc(fp)) != 'S' || (getc(fp)) != 'S' ||
		(getc(fp)) != '=') {
		printf("Save File corrupted!\n");
		HandleErrorAndExit();
	}
	if ((tier = readPos(fp)) == kBadPosition || tier < 0 || tier > tierMax) {
		printf("Save File corrupted!\n");
		HandleErrorAndExit();
	}
	if ((getc(fp)) != '\n' || (getc(fp)) != 'P' || (getc(fp)) != 'O' ||
		(getc(fp)) != 'S' || (getc(fp)) != 'L' || (getc(fp)) != 'E' ||
		(getc(fp)) != 'F' || (getc(fp)) != 'T' || (getc(fp)) != '=') {
		printf("Save File corrupted!\n");
		HandleErrorAndExit();
	}
	if ((positionsLeft = readPos(fp)) == kBadPosition || positionsLeft < 0
		|| positionsLeft > gNumberOfPositions) {
		printf("Save File corrupted!\n");
		HandleErrorAndExit();
	}
	if ((getc(fp)) != '\n' || (getc(fp)) != EOF) {
		printf("Save File corrupted!\n");
		HandleErrorAndExit();
	}
	printf(" Save File Loaded\n");
	fclose(fp);
	printf(" -Checking to make sure TIER files weren't deleted...\n");
	for (f = 0; f <= tierMax; f++) {
		sprintf(filename,"./retrograde/%s_%d.tier",kDBName,f);
		if ((fp = fopen(filename, "r")) == NULL) {
			printf("Couldn't open file %s\n", filename);
			HandleErrorAndExit();
		}
		fclose(fp);
	}
	LoadDBFromFile();
	sprintf(filename,"./retrograde/%s_TIER.save",kDBName);
	remove(filename);
	printf(" Save File Removed, everything properly loaded.\n");
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

	printf(" -Saving the Database to File...\n");
	sprintf(filename,"./retrograde/%s_DB.save",kDBName);
	if ((fp = fopen(filename, "w")) == NULL) {
		printf("ERROR: Couldn't open file %s\n", filename);
		HandleErrorAndExit();
	}
	for (pos = 0; pos < gNumberOfPositions; pos++) {
		value = GetValueOfPosition(pos);
		if (value != undecided)
			fprintf(fp, "%lld %d %d \n", pos, value, Remoteness(pos));
	}
	printf(" Database Saved\n");
	fclose(fp);
}

// This loads a saved DB file, and deletes the file. Will be replaced
// by Scott's DB when it works.
void LoadDBFromFile() {
	FILE* fp;
	POSITION pos, c;
	VALUE value; REMOTENESS remoteness;

	printf(" -Loading the Database from File...\n");
	sprintf(filename,"./retrograde/%s_DB.save",kDBName);
	if ((fp = fopen(filename, "r")) == NULL) {
		printf("Couldn't open file %s\n", filename);
		HandleErrorAndExit();
	}
	while((pos = readPos(fp)) != kBadPosition) {
		if ((c = readPos(fp)) == kBadPosition || c < 0 || c > 3) {
			printf("ERROR!! DB Save file in incorrect format!\n");
			HandleErrorAndExit();
		}
		value = c;
		StoreValueOfPosition(pos, value);
		if ((c = readPos(fp)) == kBadPosition || c < 0 || c > REMOTENESS_MAX) {
			printf("ERROR!! DB Save file in incorrect format!\n");
			HandleErrorAndExit();
		}
		remoteness = c;
		SetRemoteness(pos, remoteness);
		if ((getc(fp)) != '\n') {
			printf("ERROR!! DB Save file in incorrect format!\n");
			HandleErrorAndExit();
		}
	}
	printf(" Database Loaded\n");
	fclose(fp);
	sprintf(filename,"./retrograde/%s_DB.save",kDBName);
	remove(filename);
	printf(" DB Save File Removed\n");
}

/* HAXX that writes database to file */
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
