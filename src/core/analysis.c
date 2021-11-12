/************************************************************************
**
** NAME:	analysis.c
**
** DESCRIPTION:	Contains the majority of game analysis functionality.
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

#include "gamesman.h"
#include "analysis.h"
#include "db.h"
#include "openPositions.h"
#include "textui.h"
#include "hashwindow.h"
#include <math.h>
#include <stdint.h>

/*
** Globals
*/

long gTotalMoves = 0;
ANALYSIS gAnalysis = {};

/* Variable to allow hooking into slot based databases. This will hold the
        slot number of the VALUE slot, which is where the position values are
        stored. */
UINT32 gValueSlot = 0;

/*
** Local variables
*/
POSITION thePosition;
VALUE theValue;
POSITION winCount, loseCount, tieCount, unknownCount = 0;       // CHANGED from UINT (change back if no worky)
POSITION primitiveWins, primitiveLoses, primitiveTies = 0;
POSITION reachablePositions = 0;
POSITION totalPositions;
int hashEfficiency = 0;
float averageFanout = 0;
REMOTENESS theRemoteness;       /*	For extended analysis including remoteness */
REMOTENESS theLargestRemoteness = 0;  // for keeping track of largest found remoteness.  For efficiency and a good statistic.
FILE *xmlFile = 0;
FILE *xmlVarFile = 0;
char xmlDir[256];

/*
** Code
*/

void InitializeAnalysis() {
	theLargestRemoteness = 0;
	winCount = loseCount = tieCount = unknownCount = 0;     // CHANGED from UINT (change back if no worky)
	primitiveWins = primitiveLoses = primitiveTies = 0;
	reachablePositions = 0;
	totalPositions = 0;
	hashEfficiency = 0;
	averageFanout = 0;
	memset(&gAnalysis, 0, sizeof(gAnalysis));
	gAnalysisLoaded = FALSE;
}

void PrintRawGameValues(BOOLEAN toFile)
{
	FILE *fp;
	char filename[80];
	POSITION i;
	VALUE value;

	if(toFile) {
		printf("File to save to: ");
		scanf("%s",filename);

		if((fp = fopen(filename, "w")) == NULL) {
			ExitStageRightErrorString("Couldn't open file, sorry.");
			exit(0);
		}
		printf("Writing to %s...", filename);
		fflush(stdout);
	} else
		fp = stdout;

	if (!toFile) printf("\n");
	fprintf(fp,"%s\n", kGameName);
	fprintf(fp,"Position,Value,Remoteness%s\n",
			(!kPartizan && !gTwoBits) ? ",MexValue" : "");

	for(i=0; i<gNumberOfPositions; i++)
		if((value = GetValueOfPosition((POSITION)i)) != undecided) {
			fprintf(fp,POSITION_FORMAT ",%c,%d", i,
			gValueLetter[value], Remoteness((POSITION)i));
			if(!kPartizan && !gTwoBits)
				fprintf(fp,",%d\n",MexLoad((POSITION)i));
			else
				fprintf(fp,"\n");
		}

	if(toFile) {
		fclose(fp);
		printf("done\n");
	}
}

void PrintBinaryGameValuesToFile(char * filename)
{
	FILE *fp;
	char filename_array[80];
	POSITION i = 0;
	VALUE value = 0;
	REMOTENESS remoteness = 0;
	MEX mex = 0;
	BOOLEAN toFile = 0;
	uint64_t output = 0;
	MOVELIST *all_next_moves = NULL;
	MOVELIST *current_move = NULL;
	uint64_t max_move_choices = 0;
	uint64_t possible_move_choices = 0;
	POSITION choice = 0;
	int j = 0;
	POSITION max_position = gNumberOfPositions;
	size_t count = 1;
	int last_printed = 0;

	if (!filename) {
		printf("File to save to: ");
		scanf("%s",filename_array);
		filename = filename_array;
	}

	if ((fp = fopen(filename, "wb")) == NULL) {
		ExitStageRightErrorString("Couldn't open file, sorry.");
		exit(0);
	}

	printf("Writing to %s\n", filename);
	fflush(stdout);

	/* Header */
	output = sizeof(VALUE);
	count = (count == 1) && (fwrite(&output, sizeof(uint64_t), 1, fp) == 1); 
	if(!kPartizan && !gTwoBits) {
		output = sizeof(MEX);
	} else {
		output = 0;
	}
	count = (count == 1) && (fwrite(&output, sizeof(uint64_t), 1, fp) == 1);
	output = sizeof(POSITION);
	count = (count == 1) && (fwrite(&output, sizeof(uint64_t), 1, fp) == 1);

	printf("Finding maximum number of move counts:\n");
	printf("    Progress: [%3d%%]", 0);

	/* move_counts */
	for(i=0; i < max_position; i++) {
		if (last_printed != ((100 * i) / max_position)) {
			last_printed = ((100 * i) / max_position);
			printf("\r    Progress: [%3d%%]", last_printed);
			fflush(stdout);
		}
		all_next_moves = GenerateMoves(i);
		possible_move_choices = MoveListLength(all_next_moves);
		if (possible_move_choices > max_move_choices) {
			max_move_choices = possible_move_choices;
		}
		FreeMoveList(all_next_moves);
	}

	printf("\r    Progress: [%3d%%]\n", 100);

	printf("Maximum move choices: %llu\n", max_move_choices);

	output = max_move_choices;
	count = (count == 1) && (fwrite(&output, sizeof(uint64_t), 1, fp) == 1);

	output = gInitialPosition;
	count = (count == 1) && (fwrite(&output, sizeof(uint64_t), 1, fp) == 1);

	printf("Final export pass:\n");

	printf("    Progress: [%3d%%]", 0);
	for(i=0; i < gNumberOfPositions; i++) {
		if (last_printed != ((100 * i) / max_position)) {
			last_printed = ((100 * i) / max_position);
			printf("\r    Progress: [%3d%%]", last_printed);
			fflush(stdout);
		}
		if (kSupportsTierGamesman && gTierGamesman) {
			gInitializeHashWindowToPosition(&i, TRUE);
		}
		value = GetValueOfPosition(i);
		count = (count == 1) && (fwrite(&gValueLetter[value], sizeof(char), 1, fp) == 1);
		remoteness = Remoteness(i);
		count = (count == 1) && (fwrite(&remoteness, sizeof(REMOTENESS), 1, fp) == 1);
		if(!kPartizan && !gTwoBits) {
			mex = MexLoad(i);
			count = (count == 1) && (fwrite(&mex, sizeof(MEX), 1, fp) == 1);
		}
		current_move = all_next_moves = GenerateMoves(i);
		for (j = 0; j < max_move_choices; ++j) {
			if (current_move) {
				choice = DoMove(i, current_move->move);
				current_move = current_move->next;
			} else {
				/* choice = kBadPosition; */
				choice = -1;
			}
			count = (count == 1) && (fwrite(&choice, sizeof(POSITION), 1, fp) == 1);
		}
		FreeMoveList(all_next_moves);
	}
	printf("\r    Progress: [%3d%%]\n", 100);

	if (count != 1) {
		printf("EXPORT FAILURE: an error occured in writing the file.\n");
	}

	if(toFile) {
		fclose(fp);
		printf("done\n");
	}
}

void PrintBadPositions(char c,int maxPositions, POSITIONLIST* badWinPositions, POSITIONLIST* badTiePositions, POSITIONLIST* badLosePositions)
{
	POSITIONLIST *ptr = NULL;
	BOOLEAN continueSearching = TRUE;
	POSITION thePosition;
	int j;
	char yesOrNo;

	if     (c == '1') ptr = badWinPositions;
	else if(c == '2') ptr = badTiePositions;
	else if(c == '3') ptr = badLosePositions;
	else BadElse("PrintBadPositions");

	j = 0;
	continueSearching = TRUE;
	do {
		for(j = 0; ptr != NULL && j < maxPositions; j++) {
			thePosition = ptr->position;
			PrintPosition(thePosition, "Nobody", TRUE);
			ptr = ptr->next;
		}
		if(ptr != NULL) {
			printf("\nDo you want more? [Y/N] : ");
			scanf("%c",&yesOrNo);
			scanf("%c",&yesOrNo);
			continueSearching = (yesOrNo == 'y' || yesOrNo == 'Y');
		}
		else
			printf("\nThere are no more %s positions to list...\n",
			       c == '1' ? "winning" : c == '3' ? "losing" : "tieing");
	} while (continueSearching && ((ptr = ptr->next) != NULL));
}

void PrintMexValues(MEX mexValue, int maxPositions)
{
	BOOLEAN continueSearching = TRUE;
	POSITION thePosition;
	int j;
	char yesOrNo;

	j = 0;
	continueSearching = TRUE;
	do {
		for(j = 0; ((thePosition = GetNextPosition()) != kBadPosition) && j < maxPositions; ) {
			if (MexLoad(thePosition) == mexValue) {
				PrintPosition(thePosition, "Nobody", TRUE);
				j++;
			}
		}
		if(thePosition != kBadPosition) {
			printf("\nDo you want more? [Y/N] : ");
			scanf("%c",&yesOrNo);
			scanf("%c",&yesOrNo);
			continueSearching = (yesOrNo == 'y' || yesOrNo == 'Y');
		}
		else
			printf("\nThere are no more positions with Mex value %d to list...\n",
			       (int)mexValue);
	} while (continueSearching && (thePosition != kBadPosition));
	HitAnyKeyToContinue();
}

void PrintValuePositions(char c, int maxPositions)
{
	BOOLEAN continueSearching = TRUE;
	POSITION thePosition, theCanonicalPosition;
	VALUE theValue;
	int j;
	char yesOrNo;

	j = 0;
	thePosition = 0;
	continueSearching = TRUE;
	do {
		for(; ((j < maxPositions) && (thePosition < gNumberOfPositions)); thePosition++) {

			if (gSymmetries) {
				theCanonicalPosition = gCanonicalPosition(thePosition);
				theValue = GetValueOfPosition(theCanonicalPosition);
			} else
				theValue = GetValueOfPosition(thePosition);

			if ((c == 'r' || c == 'R') ||
			    (thePosition != theCanonicalPosition && (c == 'y' || c == 'Y') && (theValue != undecided)) ||
			    (theValue == win  && (c == 'w' || c == 'W')) ||
			    (theValue == lose && (c == 'l' || c == 'L')) ||
			    (theValue == tie  && (c == 't' || c == 'T')) ||
			    (theValue == undecided && (c == 'u' || c == 'U'))) {
				if (gSymmetries && (theCanonicalPosition != thePosition))
					printf("\nRaw position %llu is symmetric to position %llu, and has value: %s\n", thePosition, theCanonicalPosition,
					       theValue == undecided ? "Undecided" : theValue == win ? "Win" : theValue == lose ? "Lose" : "Tie");
				else
					printf("\nRaw position %llu has value: %s\n", thePosition,
					       theValue == undecided ? "Undecided" : theValue == win ? "Win" : theValue == lose ? "Lose" : "Tie");

				if (gSymmetries && (thePosition != theCanonicalPosition) && (c == 'y' || c == 'Y')) {
					printf("Position %llu:\n", thePosition);
					PrintPosition(thePosition, "Nobody", TRUE);

					printf("Position %llu:\n", theCanonicalPosition);
					PrintPosition(theCanonicalPosition, "Nobody", TRUE);
				} else
					PrintPosition(thePosition, "Nobody", TRUE);

				j++;
			}
		}
		if(thePosition != gNumberOfPositions) {
			printf("\nDo you want more? [Y/N] : ");
			yesOrNo = GetMyChar();

			while ((yesOrNo != 'y') && (yesOrNo != 'Y') && (yesOrNo != 'n') && (yesOrNo != 'N')) {
				printf("\nDo you want more? [Y/N] : ");
				yesOrNo = GetMyChar();
			}

			if ((yesOrNo == 'y') || (yesOrNo == 'Y')) {
				j = 0;
				continueSearching = TRUE;
			}
			else
				continueSearching = FALSE;
		}
		else
			printf("\nThere are no more%s positions to list...\n",
			       c == 'r' || c == 'R' ? "" : c == 'u' || c == 'U' ? " undecided" : c == 'w' || c == 'W' ? " winning" : c == 'l' || c == 'L' ? " losing" : " tieing");
	} while (continueSearching && (thePosition != gNumberOfPositions));
	HitAnyKeyToContinue();
}

void PrintDetailedGameValueSummary()
{
	char *initialPositionValue = "";
	REMOTENESS currentRemoteness;

	switch(gAnalysis.InitialPositionValue)
	{
	case win:
		initialPositionValue = "Win";
		break;
	case lose:
		initialPositionValue = "Lose";
		break;
	case tie:
		initialPositionValue = "Tie";
		break;
	default:
		BadElse("PrintGameValueSummary [InitialPositionValue]");
	}
	printf("\n\n\t----- Detailed Summary of Game values -----\n\n");

	if (!gCheckPure) {
		PrintDrawAnalysis();
	}

	printf("\tRemoteness          Win         Lose          Tie         Draw        Total\n");
	printf("\t------------------------------------------------------------------------------\n");
	printf("\t       Inf   %10llu   %10llu   %10llu   %10llu   %10llu\n", gAnalysis.DetailedPositionSummary[REMOTENESS_MAX][0],
	       gAnalysis.DetailedPositionSummary[REMOTENESS_MAX][1], gAnalysis.DetailedPositionSummary[REMOTENESS_MAX][2],
	       gAnalysis.Draws,
	       gAnalysis.Draws);
	for(currentRemoteness = gAnalysis.LargestFoundRemoteness; currentRemoteness >= 0; currentRemoteness-=1) {
		if(gAnalysis.DetailedPositionSummary[currentRemoteness][0] == 0 && gAnalysis.DetailedPositionSummary[currentRemoteness][1] == 0
		   && gAnalysis.DetailedPositionSummary[currentRemoteness][2] == 0) continue;

		printf("\t%10d   %10llu   %10llu   %10llu   %10d   %10llu\n", currentRemoteness, gAnalysis.DetailedPositionSummary[currentRemoteness][0],
		       gAnalysis.DetailedPositionSummary[currentRemoteness][1], gAnalysis.DetailedPositionSummary[currentRemoteness][2],
		       0,
		       gAnalysis.DetailedPositionSummary[currentRemoteness][2]+
		       gAnalysis.DetailedPositionSummary[currentRemoteness][1]+
		       gAnalysis.DetailedPositionSummary[currentRemoteness][0]);
	}

	printf("\t------------------------------------------------------------------------------\n");
	printf("\tTotals       %10llu   %10llu   %10llu   %10llu   %10llu\n", gAnalysis.WinCount, gAnalysis.LoseCount, gAnalysis.TieCount,
	       gAnalysis.Draws, gAnalysis.TotalPositions);
	if (gAnalysis.Draws != 0) {
		printf("\tDraws = %llu\n\n", gAnalysis.Draws);
		printf("\tFringe1 Nodes = %llu\n\tAverage Win/Draw Child Ratio = %f\n\tAvg No. Winning Children = %f\n",gAnalysis.F0NodeCount, ((float) gAnalysis.F0EdgeCount) / ((float) gAnalysis.F0DrawEdgeCount), ((float) gAnalysis.F0EdgeCount) / ((float) gAnalysis.F0NodeCount));
	}
	printf("\n\tTotal Positions Visited: %llu\n", gAnalysis.TotalPositions);

	if(gUseOpen && kLoopy)
		PrintDetailedOpenSummary(); //ALAN
	return;
}

void PrintDetailedOpenSummary()
{
	//if ganalysis.draws
	REMOTENESS currentFRemoteness;
	REMOTENESS currentCorruption;
	REMOTENESS currentLevel;

	printf("\n\n\t                             ----- Detailed Summary of Open Position values -----\n\n");
	printf("\tLevel     Corruption    FRemoteness        DrawWin       DrawLose        DrawTie       DrawDraw          Total\n");
	printf("\t----------------------------------------------------------------------------------------------------------------\n");

	printf("\t  Inf          undef            Inf              0              0              0     %10llu     %10llu\n\n",
	       gAnalysis.DrawDraws,
	       gAnalysis.DrawDraws);


	// have another array in ganalysis to save corruption/fremoteness max values at each level?
	int printedstuff = 0;

	for(currentLevel = gAnalysis.LargestFoundLevel; currentLevel >= 1; currentLevel-=1) {
		// set largest corruption
		for(currentCorruption = 10; currentCorruption >= 0; currentCorruption-=1) {
			// set largest found fremoteness with for loop
			for(currentFRemoteness = 10; currentFRemoteness >= 0; currentFRemoteness-=1) {
				if(gAnalysis.DetailedOpenSummary[currentLevel][currentCorruption][currentFRemoteness][win] == 0 &&
				   gAnalysis.DetailedOpenSummary[currentLevel][currentCorruption][currentFRemoteness][lose] == 0) continue;

				printedstuff = 1;

				printf("\t%5d     %10d     %10d     %10llu     %10llu     %10llu     %10llu     %10llu\n",
				       currentLevel,
				       currentCorruption,
				       currentFRemoteness,
				       gAnalysis.DetailedOpenSummary[currentLevel][currentCorruption][currentFRemoteness][win],
				       gAnalysis.DetailedOpenSummary[currentLevel][currentCorruption][currentFRemoteness][lose],
				       gAnalysis.DetailedOpenSummary[currentLevel][currentCorruption][currentFRemoteness][tie],
				       gAnalysis.DetailedOpenSummary[currentLevel][currentCorruption][currentFRemoteness][undecided],

				       gAnalysis.DetailedOpenSummary[currentLevel][currentCorruption][currentFRemoteness][win] +
				       gAnalysis.DetailedOpenSummary[currentLevel][currentCorruption][currentFRemoteness][lose] +
				       gAnalysis.DetailedOpenSummary[currentLevel][currentCorruption][currentFRemoteness][tie] +
				       gAnalysis.DetailedOpenSummary[currentLevel][currentCorruption][currentFRemoteness][undecided]);
			}
			if (printedstuff && currentCorruption != 0) printf("\n");
		}
		if (printedstuff && currentLevel != 1) printf("\n");
		printedstuff = 0;
	}

	printf("\t----------------------------------------------------------------------------------------------------------------\n");
	printf("\tTotals                                  %10llu     %10llu     %10llu     %10llu     %10llu\n",
	       /* gAnalysis.DrawWinCount,
	          gAnalysis.DrawLoseCount,
	          gAnalysis.DrawTieCount,
	          gAnalysis.DrawDraws,
	          gAnalysis.TotalOpenPositions); */
	       gAnalysis.OpenSummary[win],
	       gAnalysis.OpenSummary[lose],
	       gAnalysis.OpenSummary[tie],
	       gAnalysis.OpenSummary[undecided],
	       gAnalysis.OpenSummary[win]+gAnalysis.OpenSummary[lose]+gAnalysis.OpenSummary[tie]+gAnalysis.OpenSummary[undecided]);


	printf("\tDraws = %llu\n\n", gAnalysis.Draws);
	printf("\tFringe1 Nodes = %llu\n\tAverage Win/Draw Child Ratio = %f\n\tAvg No. Winning Children = %f\n",gAnalysis.F0NodeCount, ((float) gAnalysis.F0EdgeCount) / ((float) gAnalysis.F0DrawEdgeCount), ((float) gAnalysis.F0EdgeCount) / ((float) gAnalysis.F0NodeCount));
	printf("\n\tLargestFoundLevel: %d\n", gAnalysis.LargestFoundLevel);
	printf("\tLargestFoundCorruption: %d\n", gAnalysis.LargestFoundCorruption);
	printf("\n\tTotal Draw Wins: %llu\n", gAnalysis.OpenSummary[win]);
	printf("\tTotal Draw Loses: %llu\n", gAnalysis.OpenSummary[lose]);
	printf("\tTotal Draw Draws: %llu\n", gAnalysis.DrawDraws);
	//printf("\tTotal Open Positions: %llu\n", gAnalysis.TotalOpenPositions);

	printf("\n\tTotal Positions Visited: %llu\n", gAnalysis.TotalPositions);


	return;
}


void PrintGameValueSummary()
{
	char *initialPositionValue = "";
	switch(gAnalysis.InitialPositionValue)
	{
	case win:
		initialPositionValue = "Win";
		break;
	case lose:
		initialPositionValue = "Lose";
		break;
	case tie:
		initialPositionValue = "Tie";
		break;
	default:
		BadElse("PrintGameValueSummary [InitialPositionValue]");
	}
	printf("\n\n\t----- Summary of Game values -----\n\n");

	printf("\tValue       Number       Total\n");
	printf("\t------------------------------\n");
	printf("\tLose      = %5llu out of %llu (%5llu primitive)\n",gAnalysis.LoseCount,gAnalysis.TotalPositions,gAnalysis.PrimitiveLoses);
	printf("\tWin       = %5llu out of %llu (%5llu primitive)\n",gAnalysis.WinCount,gAnalysis.TotalPositions, gAnalysis.PrimitiveWins);
	printf("\tTie       = %5llu out of %llu (%5llu primitive)\n",gAnalysis.TieCount,gAnalysis.TotalPositions,gAnalysis.PrimitiveTies);
	printf("\tDraw      = %5llu out of %llu\n",
	       gAnalysis.Draws,   // ADDED to count draws
	       gAnalysis.TotalPositions);
	printf("\tUnknown   = %5llu out of %llu (Sanity-check...should always be 0)\n",gAnalysis.UnknownCount,gAnalysis.TotalPositions);
	printf("\tTOTAL     = %5llu out of %llu allocated (%5llu primitive)\n",
	       gAnalysis.TotalPositions,
	       gNumberOfPositions,
	       gAnalysis.TotalPrimitives);

	printf("\tHash Efficiency                   = %6d\%%\n",gAnalysis.HashEfficiency);
	printf("\tTotal Moves                       = %5llu\n",gAnalysis.TotalMoves);
	printf("\tAvg. number of moves per position = %2f\n", gAnalysis.AverageFanout);
	printf("\tProbability of maintaining a %-5s= %2f\n", initialPositionValue,gAnalysis.InitialPositionProbability);

	return;
}



/** Analysis **/

void analyze() {
	if(!gAnalysisLoaded) {
		CreateDatabases();
		InitializeDatabases();
		if (gUseOpen) InitializeOpenPositions(gNumberOfPositions);
		if (gPrintDatabaseInfo)
			printf("\nEvaluating the value of %s...", kGameName); \
		gSolver(gInitialPosition);
		showStatus(Clean);
		AnalysisCollation();

		if(gSaveDatabase) {
			SaveDatabase();
			SaveAnalysis();
		}

		gAnalysisLoaded = TRUE;
	}
}

void DetermineInterestingness(POSITION position) {
	int debugme = 0;
	int count_max = 0;
	float max_seen = 0.0;
	POSITION i;
	POSITION most_interesting = 0;


	gAnalysis.Interestingness = (float *) SafeMalloc(gNumberOfPositions * sizeof(float));

	for (i=0; i < gNumberOfPositions; i++) {
		gAnalysis.Interestingness[i] = 0.0;
	}

	printf("\nDetermining Interestingness...");

	DetermineInterestingnessDFS(position);

	// set most mostinteresting
	for (i=0; i < gNumberOfPositions; i++) {
		if (debugme && gAnalysis.Interestingness[i]) {
			printf("%d\tNONZERO INTERESTINGNESS IN LOOP at %llu (%f)\n",GetValueOfPosition(i),i,gAnalysis.Interestingness[i]);
		}
		if (GetValueOfPosition(i) == win && gAnalysis.Interestingness[i] > max_seen) {
			count_max = 0;
			max_seen = gAnalysis.Interestingness[i];
			most_interesting = i;
		} else if (gAnalysis.Interestingness[i] == max_seen) {
			count_max++;
		}
	}

	gAnalysis.MostInteresting = most_interesting;
	gAnalysis.MaxInterestingness = max_seen = 100 * max_seen;

	SafeFree(gAnalysis.Interestingness);
	gAnalysis.Interestingness = NULL;

	printf("%f\n",max_seen);

	printf("Re-saving analysis DB...");
	SaveAnalysis();

}

void DetermineInterestingnessDFS(POSITION position) {
	int debugme = 0;
	// for interestingness calculations
	float interestingness, immediate_interestingness;
	int wincount, losecount, tiecount, count;
	// for generate moves
	POSITION child;
	MOVELIST *ptr, *head;
	MOVE move;
	// misc
	VALUE value, childvalue;

	if (Visited(position)) {
		if (debugme) {
			printf("\tVISITED, returning\n");
		}
		// this position's interestingness has been determined
		// gAnalysis.Interestingness[position] != 0
		return;
	}
	if ((value = GetValueOfPosition(position)) == undecided) {
		if(debugme) {
			printf("\tUNDECIDED or TIE, returning\n");
		}

		// interestingness defined as zero here
		// gAnalysis.Interestingness[position] == 0
		return;
	}
	// first time we've seen this position
	if (debugme) {
		printf("UNSEEN POSITION\n");
	}
	MarkAsVisited(position);

	if (Primitive(position) != undecided) {
		// if this is primitive, we assign a default interestingness value
		gAnalysis.Interestingness[position] = PRIMITIVE_INTERESTINGNESS;
		if(debugme) {
			printf("\tPRIMITIVE, set to %f\n",gAnalysis.Interestingness[position]);
		}

		return;
	} else {
		if(debugme) {
			printf("\tNOT PRIMITIVE, recursing on children\n");
		}
		// recurse on children
		wincount = losecount = tiecount = 0;
		head = ptr = GenerateMoves(position);
		while(ptr != NULL) {
			move = ptr->move;

			child = DoMove(position,move);
			if ((childvalue = GetValueOfPosition(child)) != undecided) {
				DetermineInterestingnessDFS(child);
				switch(childvalue) {
				case win:
					// only non trivial wins are 'hard to see'
					if (Remoteness(child) > 1) {
						wincount++;
					}
					break;
				case lose:
					losecount++;
					break;
				case tie:
					tiecount++;
					break;
				default:
					BadElse("DetermineInterestingness");
				}

			}

			ptr = ptr->next;
		}
		// set interestingness of this position by children values, immediate interestingness
		switch(value) {
		case win:
			if (debugme) {
				printf("\tTHIS IS A WIN\n");
			}
			// immediate interestingness
			// higher W+T/W+L+T is better
			immediate_interestingness = ((float) wincount + tiecount) / ((float) wincount + losecount + tiecount);
			immediate_interestingness = (immediate_interestingness > PRIMITIVE_INTERESTINGNESS) ? immediate_interestingness : PRIMITIVE_INTERESTINGNESS;
			// accumulate on loses
			// form 1 - \prod_{lose_children} (1 - Interestingness[child])
			interestingness = 0;
			count = 0;
			ptr = head;
			while(ptr != NULL) {
				move = ptr->move;

				child = DoMove(position,move);
				// only include losing children
				if ((childvalue = GetValueOfPosition(child)) == lose) {
					interestingness += gAnalysis.Interestingness[child];
					// interestingness *= 1.0 - gAnalysis.Interestingness[child];
				}

				ptr = ptr->next;
			}
			// interestingness -= 1.0;
			// gAnalysis.Interestingness[position] = 1.0 - sqrt((interestingness + (1.0 - immediate_interestingness))/2);
			interestingness /= losecount;
			gAnalysis.Interestingness[position] = 1.0 - sqrt((1.0 - interestingness)*(1.0 - immediate_interestingness));
			if (debugme) {
				printf("\tIMMEDIATE %f, RECURSIVE %f\n",immediate_interestingness,interestingness);
			}
			break;
		case lose:
			if (debugme) {
				printf("\tTHIS IS A LOSE\n");
			}
			interestingness = 0;
			// interestingness = 1;
			count = 0;
			ptr = head;
			while(ptr != NULL) {
				move = ptr->move;

				child = DoMove(position,move);
				// all children should be win values
				interestingness += gAnalysis.Interestingness[child];
				count++;
				// interestingness *= 1.0 - gAnalysis.Interestingness[child];

				ptr = ptr->next;
			}
			gAnalysis.Interestingness[position] = interestingness /= count;
			// gAnalysis.Interestingness[position] = 1.0 - interestingness;

			break;
		case tie:
			// this interestingness is zero
			// assert gAnalysis.Interestingness[position] = 0;
			break;
		default:
			// should not get here
			BadElse("DetermineInterestingness");
		}
		// clean up from GenerateMoves
		FreeMoveList(head);
	}
	if(debugme) {
		printf("Set interestingness of %f for %d\n",gAnalysis.Interestingness[position],value);
	}
	// assert gAnalysis.Interestingness[position] != 0
}

VALUE AnalyzePosition(POSITION thePosition, VALUE theValue)
{
	if (theValue != undecided) {
		totalPositions++;
		if(theValue == win)  {
			winCount++;
			reachablePositions++;
			if ((theRemoteness = Remoteness(thePosition)) == 0) primitiveWins++;    // Stores remoteness on each call, saves data to array
			gAnalysis.DetailedPositionSummary[theRemoteness][0] += 1;
			if (theRemoteness > theLargestRemoteness) theLargestRemoteness = theRemoteness; // Keeps track of the largest seen remoteness
		} else if(theValue == lose) {
			loseCount++;
			reachablePositions++;
			if ((theRemoteness = Remoteness(thePosition)) == 0) primitiveLoses++;
			gAnalysis.DetailedPositionSummary[theRemoteness][1] += 1;
			if (theRemoteness > theLargestRemoteness) theLargestRemoteness = theRemoteness;
		} else if(theValue == tie) {
			if ((theRemoteness = Remoteness(thePosition)) < REMOTENESS_MAX)
			{
				tieCount++;
				gAnalysis.DetailedPositionSummary[theRemoteness][2] += 1;
				if (theRemoteness > theLargestRemoteness) theLargestRemoteness = theRemoteness;
				if (theRemoteness == 0) primitiveTies++;
			}
			reachablePositions++;
		} else {
			unknownCount++;
		}
	}
	else printf("Undecided!");

	return(theValue);
}

void AnalysisCollation()
{
	hashEfficiency = (int)((((float)reachablePositions ) / (float)gNumberOfPositions) * 100.0);
	averageFanout = (float)((float)gAnalysis.TotalMoves/(float)(reachablePositions - primitiveWins - primitiveLoses - primitiveTies));

	gAnalysis.InitialPositionValue = GetValueOfPosition(gInitialPosition);

	//gAnalysis.InitialPositionProbability = DetermineProbability(gInitialPosition,gAnalysis.InitialPositionValue);

	gAnalysis.HashEfficiency    = hashEfficiency;
	gAnalysis.AverageFanout     = averageFanout;
	gAnalysis.TotalPositions    = totalPositions;
	gAnalysis.WinCount          = winCount;
	gAnalysis.LoseCount         = loseCount;
	gAnalysis.TieCount          = tieCount;
	gAnalysis.Draws             = gAnalysis.TotalPositions - gAnalysis.WinCount - gAnalysis.LoseCount - gAnalysis.TieCount;
	gAnalysis.UnknownCount      = unknownCount;
	gAnalysis.PrimitiveWins     = primitiveWins;
	gAnalysis.PrimitiveLoses    = primitiveLoses;
	gAnalysis.PrimitiveTies     = primitiveTies;
	gAnalysis.NumberOfPositions = gNumberOfPositions;
	gAnalysis.TotalPrimitives   = gAnalysis.PrimitiveWins+gAnalysis.PrimitiveLoses+gAnalysis.PrimitiveTies;
	gAnalysis.LargestFoundRemoteness = theLargestRemoteness;  //ADDED

	gAnalysisLoaded = TRUE;
}

/* Determines the chance that you'll maintain your value if you *
* randomly select a move given a position.                     */
float DetermineProbability(POSITION position, VALUE value)
{
	MOVELIST *ptr, *head;
	VALUE opposite_value=lose;
	VALUE primitive = Primitive(position);
	POSITION child;
	POSITION numChildren = 0;
	float probabilitySum = 0.0;

	switch(value)
	{
	case win:
		opposite_value = lose;
		break;
	case lose:
		opposite_value = win;
		break;
	case tie:
		opposite_value = tie;
		break;
	default:
		BadElse("DetermineProbability [next_level_value]");
	}


	if(primitive == value)
	{
		return 1.000;
	}
	else if (primitive == opposite_value)
	{
		return 0.0;
	}
	else if(Visited(position))
	{
		return 0.0;
	}
	else
	{
		MarkAsVisited(position);

		head = ptr = GenerateMoves(position);
		if(ptr == NULL) {return 0.0; }
		while(ptr != NULL)
		{
			child = DoMove(position, ptr->move);

			probabilitySum += DetermineProbability(child, opposite_value);

			numChildren++;
			ptr = ptr->next;
		}
		FreeMoveList(head);

		return (float)((float) probabilitySum / (float)numChildren);


	}

}


// Write variant statistic
void writeVarStat(char * statName, char * text, FILE *rowp)
{
	FILE * filep;
	//FILE * rawfilep ;
	char outFileName[256];

	sprintf(outFileName, "analysis/%s/var%d/%s", kDBName,getOption(),statName);

	filep = fopen(outFileName, "w");


	fprintf(filep,"<!-- AUTO CREATED, do //not modify-->\n");
	fprintf(filep,"%s",text);
	fprintf(filep,"\n");


	fprintf(rowp,"<td ALIGN = " "center" "><!--#include virtual=\"%s\"--></td>\n",statName);


	fclose(filep);


}

void createAnalysisGameDir()
{
	char gameDirName[256];
	sprintf(gameDirName, "analysis/%s", kDBName);

	mkdir("analysis", 0755);
	mkdir(gameDirName, 0755);

}

void createAnalysisVarDir()
{
	char varDirName[256];
	sprintf(varDirName, "analysis/%s/var%d", kDBName,getOption());
	mkdir(varDirName, 0755);
}

BOOLEAN LoadAnalysis() {
	char gameFileName[256];
	//int currentRemoteness;
	char version;
	FILE *fp;

	Stopwatch();
	createAnalysisGameDir();
	sprintf(gameFileName, "analysis/%s/m%s_%d_analysis.dat", kDBName, kDBName, getOption());

	if(!gAnalyzing)
		printf("\nLoading Analysis DB for %s...", kGameName);

	if(gAnalysisLoaded) {
		if(!gAnalyzing)
			printf("done in %u seconds!\n", Stopwatch());
		return TRUE;
	}

	/* Open file for reading */
	if((fp = fopen(gameFileName, "rb")) == NULL) {
		if(!gAnalyzing)
			printf("Failed!");
		Stopwatch();
		return FALSE;
	}

	/* Check file version */
	if((fread(&version, sizeof(char), 1, fp) != 1) || (version != ANALYSIS_FILE_VER)) {
		if(!gAnalyzing)
			printf("Failed!");
		Stopwatch();
		return FALSE;
	}

	/* Read misc. info */
	/*
	    if((fread(&(gAnalysis.NumberOfPositions), sizeof(POSITION), 1, fp) != 1) ||
	            (fread(&(gAnalysis.TotalPositions), sizeof(POSITION), 1, fp) != 1) ||
	            (fread(&(gAnalysis.TotalMoves), sizeof(POSITION), 1, fp) != 1) ||
	            (fread(&(gAnalysis.TotalPrimitives), sizeof(POSITION), 1, fp) != 1) ||
	            (fread(&(gAnalysis.WinCount), sizeof(POSITION), 1, fp) != 1) ||
	            (fread(&(gAnalysis.LoseCount), sizeof(POSITION), 1, fp) != 1) ||
	            (fread(&(gAnalysis.TieCount), sizeof(POSITION), 1, fp) != 1) ||
	            (fread(&(gAnalysis.UnknownCount), sizeof(POSITION), 1, fp) != 1) ||
	            (fread(&(gAnalysis.Draws), sizeof(POSITION), 1, fp) != 1) ||
	            (fread(&(gAnalysis.PrimitiveWins), sizeof(POSITION), 1, fp) != 1) ||
	            (fread(&(gAnalysis.PrimitiveLoses), sizeof(POSITION), 1, fp) != 1) ||
	            (fread(&(gAnalysis.PrimitiveTies), sizeof(POSITION), 1, fp) != 1) ||
	            (fread(&(gAnalysis.F0EdgeCount), sizeof(POSITION), 1, fp) != 1) ||
	            (fread(&(gAnalysis.F0NodeCount), sizeof(POSITION), 1, fp) != 1) ||
	            (fread(&(gAnalysis.F0DrawEdgeCount), sizeof(POSITION), 1, fp) != 1) ||
	            (fread(&(gAnalysis.InitialPositionValue), sizeof(VALUE), 1, fp) != 1) ||
	            (fread(&(gAnalysis.LargestFoundRemoteness), sizeof(REMOTENESS), 1, fp) != 1) ||
	            (fread(&(gAnalysis.TimeToSolve), sizeof(unsigned int), 1, fp) != 1) ||
	            (fread(&(gAnalysis.HashEfficiency), sizeof(int), 1, fp) != 1) ||
	            (fread(&(gAnalysis.AverageFanout), sizeof(float), 1, fp) != 1) ||
	            (fread(&(gAnalysis.InitialPositionProbability), sizeof(float), 1, fp) != 1))
	    {
	            printf("Failed!");
	            return FALSE;
	    }

	   for(currentRemoteness = 0; currentRemoteness <= gAnalysis.LargestFoundRemoteness; currentRemoteness++)
	   {
	    if((fread(&(gAnalysis.DetailedPositionSummary[currentRemoteness][0]), sizeof(POSITION), 1, fp) != 1) ||
	            (fread(&(gAnalysis.DetailedPositionSummary[currentRemoteness][1]), sizeof(POSITION), 1, fp) != 1) ||
	            (fread(&(gAnalysis.DetailedPositionSummary[currentRemoteness][0]), sizeof(POSITION), 1, fp) != 1))
	    {
	            printf("Failed!");
	            return FALSE;
	    }
	   }
	 */

	if(fread(&gAnalysis, sizeof(ANALYSIS), 1, fp) != 1) {
		if(!gAnalyzing)
			printf("Failed!");
		Stopwatch();
		return FALSE;
	}

	fclose(fp);
	gAnalysisLoaded = TRUE;
	if(!gAnalyzing)
		printf("done in %u seconds!", Stopwatch());
	return TRUE;
}

/**
 *  Analysis database format:
 *  Header:
 *      First byte is version number (database format version 2 in this case)
 *      Second byte is '/n' to delimit header from data
 *  Other data:
 *      Misc. data will be on one line, delimited by commas. This includes
 *      total wins, loses, ties, etc.
 *  Remoteness Data:
 *      Each remoteness is on its own line. Data is stored in following order
 *      per line: win, lose, tie, with values delimited by a comma.
 */

void SaveAnalysis() {
	char gameFileName[256];
	//int currentRemoteness;
	char version = ANALYSIS_FILE_VER;
	FILE *fp;

	Stopwatch();
	createAnalysisGameDir();

	sprintf(gameFileName, "analysis/%s/m%s_%d_analysis.dat", kDBName, kDBName, getOption());
	printf("\nSaving Analysis DB with %s...", kGameName);

	/* Open file for reading */
	if((fp = fopen(gameFileName, "wb")) == NULL) {
		printf("Failed!");
		Stopwatch();
		return;
	}

	/* Write file version */
	if(fwrite(&version, sizeof(char), 1, fp) != 1) {
		printf("Failed!");
		Stopwatch();
		return;
	}

	/* Write misc. info */
	/*
	    if((fwrite(&(gAnalysis.NumberOfPositions), sizeof(POSITION), 1, fp) != 1) ||
	            (fwrite(&(gAnalysis.TotalPositions), sizeof(POSITION), 1, fp) != 1) ||
	            (fwrite(&(gAnalysis.TotalMoves), sizeof(POSITION), 1, fp) != 1) ||
	            (fwrite(&(gAnalysis.TotalPrimitives), sizeof(POSITION), 1, fp) != 1) ||
	            (fwrite(&(gAnalysis.WinCount), sizeof(POSITION), 1, fp) != 1) ||
	            (fwrite(&(gAnalysis.LoseCount), sizeof(POSITION), 1, fp) != 1) ||
	            (fwrite(&(gAnalysis.TieCount), sizeof(POSITION), 1, fp) != 1) ||
	            (fwrite(&(gAnalysis.UnknownCount), sizeof(POSITION), 1, fp) != 1) ||
	            (fwrite(&(gAnalysis.Draws), sizeof(POSITION), 1, fp) != 1) ||
	            (fwrite(&(gAnalysis.PrimitiveWins), sizeof(POSITION), 1, fp) != 1) ||
	            (fwrite(&(gAnalysis.PrimitiveLoses), sizeof(POSITION), 1, fp) != 1) ||
	            (fwrite(&(gAnalysis.PrimitiveTies), sizeof(POSITION), 1, fp) != 1) ||
	            (fwrite(&(gAnalysis.F0EdgeCount), sizeof(POSITION), 1, fp) != 1) ||
	            (fwrite(&(gAnalysis.F0NodeCount), sizeof(POSITION), 1, fp) != 1) ||
	            (fwrite(&(gAnalysis.F0DrawEdgeCount), sizeof(POSITION), 1, fp) != 1) ||
	            (fwrite(&(gAnalysis.InitialPositionValue), sizeof(VALUE), 1, fp) != 1) ||
	            (fwrite(&(gAnalysis.LargestFoundRemoteness), sizeof(REMOTENESS), 1, fp) != 1) ||
	            (fwrite(&(gAnalysis.TimeToSolve), sizeof(unsigned int), 1, fp) != 1) ||
	            (fwrite(&(gAnalysis.HashEfficiency), sizeof(int), 1, fp) != 1) ||
	            (fwrite(&(gAnalysis.AverageFanout), sizeof(float), 1, fp) != 1) ||
	            (fwrite(&(gAnalysis.InitialPositionProbability), sizeof(float), 1, fp) != 1))
	    {
	            printf("Failed!");
	            return;
	    }


	   for(currentRemoteness = 0; currentRemoteness <= gAnalysis.LargestFoundRemoteness; currentRemoteness++)
	   {
	    if((fwrite(&(gAnalysis.DetailedPositionSummary[currentRemoteness][0]), sizeof(POSITION), 1, fp) != 1) ||
	            (fwrite(&(gAnalysis.DetailedPositionSummary[currentRemoteness][1]), sizeof(POSITION), 1, fp) != 1) ||
	            (fwrite(&(gAnalysis.DetailedPositionSummary[currentRemoteness][0]), sizeof(POSITION), 1, fp) != 1))
	    {
	            printf("Failed!");
	            return;
	    }
	   }
	 */
	if(fwrite(&gAnalysis, sizeof(ANALYSIS), 1, fp) != 1) {
		printf("Failed!");
		Stopwatch();
		return;
	}

	fclose(fp);
	gAnalysisLoaded = TRUE;
	printf("done in %u seconds!\n", Stopwatch());
	return;
}

void writeGameHTML()
{
	char gameFileName[256];
	FILE *gamep;


	STRING bgColor = "#000066";
	STRING fontColor = "#FFFFFF";
	STRING fontFace = "verdana";

	sprintf(gameFileName, "analysis/%s/%s.shtml", kDBName,kDBName);
	gamep = fopen(gameFileName, "w");

	fprintf(gamep, "<html><head>\n");
	fprintf(gamep, "<style>a:link, a:visited {color: %s\ntext-decoration: none;}\n\n", bgColor);
	fprintf(gamep, "a:hover, a:active {color: %s; text-decoration: none;}\ntd {color: %s}\n</style>\n", bgColor, fontColor);

	fprintf(gamep, "</head>\n");
	fprintf(gamep, "<body bgcolor=\"%s\">\n",bgColor);
	fprintf(gamep, "<font color = \"%s\" face = %s size = 2>", fontColor, fontFace);

	// a picture of the game
	fprintf(gamep, "<center>\n");
	fprintf(gamep, "<img src=\"../images/%s.gif\" width = 100 height = 100>", kDBName);
	fprintf(gamep, "</br></br>\n");
	fprintf(gamep, "</center>\n");

	// Game name, gamescrafter
	fprintf(gamep, "<center>");
	fprintf(gamep, "<h1><b>\n");
	fprintf(gamep, "%s\n", kGameName);
	fprintf(gamep, "</h1></b>\n");
	fprintf(gamep, "<h2><b>\n");
	fprintf(gamep, "Crafted by: %s", kAuthorName);
	fprintf(gamep, "</h2></b>\n");
	fprintf(gamep, "</center>");

	fprintf(gamep, "<!--#include virtual=\"%s_table.shtml\"-->\n", kDBName);

	fprintf(gamep, "</body>");
	fprintf(gamep, "</html>\n");

	fclose(gamep);
}


void createVarTable ()
{
	char tableFileName[256];
	FILE * tablep;
	int i;

	sprintf(tableFileName, "analysis/%s/%s_table.shtml", kDBName, kDBName);
	tablep = fopen(tableFileName, "w");

	fprintf(tablep,"<!-- AUTO CREATED, do not modify-->\n");
	fprintf(tablep,"<table align=" "ABSCENTER" " BORDER =" "1" " CELLSPACING=" "0" " CELLPADDING=" "5" ">\n");
	fprintf(tablep,"<tr>\n");

	fprintf(tablep,"<td><b>Variant</b></td>\n");
	fprintf(tablep,"<td><b>Value</b></td>\n");
	fprintf(tablep,"<td><b>Wins</b></td>\n");
	fprintf(tablep,"<td><b>Loses</b></td>\n");
	fprintf(tablep,"<td><b>Ties</b></td>\n");
	fprintf(tablep,"<td><b>Primitive Wins</b></td>\n");
	fprintf(tablep,"<td><b>Primitive Loses</b></td>\n");
	fprintf(tablep,"<td><b>Primitive Ties</b></td>\n");
	fprintf(tablep,"<td><b>Reachable Positions</b></td>\n");
	fprintf(tablep,"<td><b>Total Positions</b></td>\n");
	fprintf(tablep,"<td><b>Hash Efficiency (%%)</b></td>\n");
	fprintf(tablep,"<td><b>Avg. Fanout</b></td>\n");
	fprintf(tablep,"<td><b>Timer(s)</b></td>\n");


	fprintf(tablep,"</tr>\n");

	for (i = 1; i <= NumberOfOptions(); i++) {
		fprintf(tablep,"<tr>\n");
		fprintf(tablep,"<!--#include virtual=\"var%d/row.shtml\"-->\n", i);
		fprintf(tablep,"</tr>\n");
	}

	fprintf(tablep,"</table>\n");
	fclose (tablep);

}

void writeVarHTML ()
{

	char text[256];
	FILE * rowp;
	char rowFileName[256];

	sprintf(rowFileName, "analysis/%s/var%d/row.shtml", kDBName,getOption());

	rowp = fopen(rowFileName, "w");

	/***********************************
	   Variant Specific
	 ************************************/


	fprintf(rowp,"<!-- AUTO CREATED, do not modify-->\n");

	sprintf(text, "%d",getOption());
	writeVarStat("option",text,rowp);

	writeVarStat("value", gValueString[(int)gValue], rowp);

	sprintf(text, "%5llu", gAnalysis.WinCount);
	writeVarStat("WinCount", text, rowp);

	sprintf(text, "%5llu", gAnalysis.LoseCount);
	writeVarStat("LoseCount", text, rowp);

	sprintf(text, "%5llu", gAnalysis.TieCount);
	writeVarStat("TieCount", text, rowp);

	sprintf(text, "%5llu", gAnalysis.PrimitiveWins);
	writeVarStat("Prim.WinCount", text, rowp);

	sprintf(text, "%5llu", gAnalysis.PrimitiveLoses);
	writeVarStat("Prim.LoseCount", text, rowp);

	sprintf(text, "%5llu", gAnalysis.PrimitiveTies);
	writeVarStat("Prim.TieCount", text, rowp);


	sprintf(text, "%5llu", gAnalysis.TotalPositions);
	writeVarStat("totalPositions", text, rowp);

	sprintf(text, "%5llu", gNumberOfPositions);
	writeVarStat("NumberOfPositions", text, rowp);

	sprintf(text, "%d", gAnalysis.HashEfficiency);
	writeVarStat("hashEfficiency", text, rowp);


	sprintf(text, "%2f", gAnalysis.AverageFanout);
	writeVarStat("AverageFanout", text, rowp);

	sprintf(text, "%d", gAnalysis.TimeToSolve);
	writeVarStat("TimeToSolve", text, rowp);



	fclose(rowp);

}


BOOLEAN CorruptedValuesP()
{
	MOVELIST *ptr, *head;
	VALUE parentValue, childValue;
	POSITION position, child;
	BOOLEAN parentIsWin, foundLosingChild, parentIsTie, foundTieingChild, corrupted;

	corrupted = FALSE;
	for(position=0; position<gNumberOfPositions; position++) { /* for all positions */
		parentIsWin = FALSE;
		foundLosingChild = FALSE;
		parentIsTie = FALSE;
		foundTieingChild = FALSE;
		if((parentValue = GetValueOfPosition(position)) != undecided) { /* if valid position */
			if(Primitive(position) == undecided) { /* Not Primitive, children */
				head = ptr = GenerateMoves(position);
				while (ptr != NULL) {
					child = DoMove(position,ptr->move); /* Create the child */
					childValue = GetValueOfPosition(child); /* Get its value */

					if (gGoAgain(position, ptr->move)) {
						switch(childValue) {
						case win: childValue = lose; break;
						case lose: childValue = win; break;
						default: break;
						}
					}

					if(parentValue == lose) {
						if(childValue != win) {
							corrupted = TRUE;
							printf("Corruption: Losing Parent " POSITION_FORMAT " has %s child " POSITION_FORMAT ", shouldn't be losing\n",position,gValueString[childValue],child);
						}
					} else if (parentValue == win) {
						parentIsWin = TRUE;
						if(childValue == lose)
							foundLosingChild = TRUE;
					} else if (parentValue == tie) {
						parentIsTie = TRUE;
						if(childValue == lose) {
							corrupted = TRUE;
							printf("Corruption: Tieing Parent " POSITION_FORMAT " has Lose child " POSITION_FORMAT ", should be win\n",position,child);
						} else if (childValue == tie)
							foundTieingChild = TRUE;
					} else
						BadElse("CorruptedValuesP");
					ptr = ptr->next; /* Go to the next child */
				} /* while ptr != NULL (for all children) */
				FreeMoveList(head);
				if(parentIsWin && !foundLosingChild) {
					corrupted = TRUE;
					printf("Corruption: Winning Parent " POSITION_FORMAT " has no losing children, shouldn't be win\n",position);
				}
				if(parentIsTie && !foundTieingChild) {
					corrupted = TRUE;
					printf("Corruption: Tieing Parent " POSITION_FORMAT " has no tieing children, should be a lose\n",position);
				}
			} /* if not primitive */
		} /* if valid position */
	} /* for all positions */
	return(corrupted);
}

/*
** Analysis XML Output
*/

void writeXML(STATICMESSAGE msg)
{
	char xmlPath[512];

	switch(msg)
	{
	case Init:
		prepareXMLFile();
		sprintf(xmlPath, "%s/m%s.xml", xmlDir, kDBName);
		xmlFile = fopen(xmlPath, "a+");
		break;
	case InitVar:
		prepareXMLVarFile();
		sprintf(xmlPath, "%s/m%s_%d.xml", xmlDir, kDBName, getOption());
		xmlVarFile = fopen(xmlPath, "a+");
		break;
	case Save:
		if(xmlFile != NULL)
		{
			writeXMLData();
			//printf("\nSaving XML data for option %d", getOption());
		} else {
			printf("\nCouldn't write XML data for option %d", getOption());
		}
		break;
	case SaveVar:
		if(xmlVarFile != NULL)
		{
			writeXMLVarData();
			//printf("\nSaving XML Var data for option %d", getOption());
		} else {
			printf("\nCouldn't write XML Var data for option %d", getOption());
		}
		break;
	case Clean:
		if(xmlFile != NULL)
		{
			closeXMLFile();
			xmlFile = NULL;
		}
		break;
	case CleanVar:
		if(xmlVarFile != NULL)
		{
			closeXMLVarFile();
			xmlVarFile = NULL;
		}
		break;
	default:
		BadElse("writeXML");
		break;
	}
}

void prepareXMLFile()
{
	char xmlPath[512];

	sprintf(xmlDir, "analysis/%s/xml", kDBName);
	mkdir(xmlDir,0755);

	sprintf(xmlPath, "%s/m%s.xml", xmlDir, kDBName);
	xmlFile = fopen(xmlPath,"w+");
	fprintf(xmlFile,"<?xml version=\"1.0\"?>\n");
	fprintf(xmlFile,"<game name=\"%s\" author=\"%s\" shortname=\"%s\">\n", kGameName,kAuthorName,kDBName);
	fclose(xmlFile);
}

void prepareXMLVarFile()
{
	char xmlPath[512];

	sprintf(xmlDir, "analysis/%s/xml", kDBName);
	mkdir(xmlDir,0755);

	sprintf(xmlPath, "%s/m%s_%d.xml", xmlDir, kDBName, getOption());
	xmlVarFile = fopen(xmlPath, "w+");
	fprintf(xmlVarFile,"<?xml version=\"1.0\"?>\n");
	fprintf(xmlVarFile,"<game name=\"%s\" author=\"%s\" shortname=\"%s\">\n", kGameName,kAuthorName,kDBName);
	fclose(xmlVarFile);
}


void closeXMLFile()
{
	fprintf(xmlFile,"</game>\n");
	fclose(xmlFile);
	//printf("\nxmlFile closed for option %d\n", getOption());
}

void closeXMLVarFile()
{
	fprintf(xmlVarFile,"</game>\n");
	fclose(xmlVarFile);
	//printf("\nxmlVarFile closed for option %d\n", getOption());
}

void writeXMLData()
{
	fprintf(xmlFile,"\t<variant hashcode=\"%d\" string=\"%s\">\n",getOption(),gGetVarStringPtr());
	fprintf(xmlFile,"\t\t<value>%s</value>\n",gValueString[(int)gValue]);
	fprintf(xmlFile,"\t\t<count>\n");
	fprintf(xmlFile,"\t\t\t<win>%llu</win>\n",gAnalysis.WinCount);
	fprintf(xmlFile,"\t\t\t<lose>%llu</lose>\n",gAnalysis.LoseCount);
	fprintf(xmlFile,"\t\t\t<tie>%llu</tie>\n",gAnalysis.TieCount);
	fprintf(xmlFile,"\t\t\t<draw>%llu</draw>\n",gAnalysis.Draws);
	fprintf(xmlFile,"\t\t</count>\n");
	fprintf(xmlFile,"\t\t<interestingness>\n");
	fprintf(xmlFile,"\t\t\t<mostInterestingPos>%llu</mostInterestingPos>\n",gAnalysis.MostInteresting);
	fprintf(xmlFile,"\t\t\t<maxInterestingness>%f</maxInterestingness>\n",gAnalysis.MaxInterestingness);
	fprintf(xmlFile,"\t\t</interestingness>\n");
	fprintf(xmlFile,"\t\t<primitive>\n");
	fprintf(xmlFile,"\t\t\t<win>%llu</win>\n",gAnalysis.PrimitiveWins);
	fprintf(xmlFile,"\t\t\t<lose>%llu</lose>\n",gAnalysis.PrimitiveLoses);
	fprintf(xmlFile,"\t\t\t<tie>%llu</tie>\n",gAnalysis.PrimitiveTies);
	fprintf(xmlFile,"\t\t</primitive>\n");
	fprintf(xmlFile,"\t\t<positionstats>\n");
	fprintf(xmlFile,"\t\t\t<total>%llu</total>\n",gAnalysis.TotalPositions);
	fprintf(xmlFile,"\t\t\t<hashtotal>%llu</hashtotal>\n",gNumberOfPositions);
	fprintf(xmlFile,"\t\t\t<hashefficiency>%d</hashefficiency>\n",gAnalysis.HashEfficiency);
	fprintf(xmlFile,"\t\t\t<fanout>%2f</fanout>\n",gAnalysis.AverageFanout);
	fprintf(xmlFile,"\t\t</positionstats>\n");
	fprintf(xmlFile,"\t\t<time>%d</time>\n",gAnalysis.TimeToSolve);
	fprintf(xmlFile,"\t\t<maxremoteness>%d</maxremoteness>\n",gAnalysis.LargestFoundRemoteness);
	fprintf(xmlFile,"\t</variant>\n");
	fflush(xmlFile);
}

void writeXMLVarData()
{
	REMOTENESS currentRemoteness;
	fprintf(xmlVarFile,"\t<variant hashcode=\"%d\" string=\"%s\">\n",getOption(),gGetVarStringPtr());
	fprintf(xmlVarFile,"\t\t<value>%s</value>\n",gValueString[(int)gValue]);
	fprintf(xmlVarFile,"\t\t<count>\n");
	fprintf(xmlVarFile,"\t\t\t<win>%llu</win>\n",gAnalysis.WinCount);
	fprintf(xmlVarFile,"\t\t\t<lose>%llu</lose>\n",gAnalysis.LoseCount);
	fprintf(xmlVarFile,"\t\t\t<tie>%llu</tie>\n",gAnalysis.TieCount);
	fprintf(xmlVarFile,"\t\t\t<draw>%llu</draw>\n",gAnalysis.Draws);
	fprintf(xmlVarFile,"\t\t</count>\n");
	fprintf(xmlVarFile,"\t\t<interestingness>\n");
	fprintf(xmlVarFile,"\t\t\t<mostInterestingPos>%llu</mostInterestingPos>\n",gAnalysis.MostInteresting);
	fprintf(xmlVarFile,"\t\t\t<maxInterestingness>%f</maxInterestingness>\n",gAnalysis.MaxInterestingness);
	fprintf(xmlVarFile,"\t\t</interestingness>\n");
	fprintf(xmlVarFile,"\t\t<positionstats>\n");
	fprintf(xmlVarFile,"\t\t\t<total>%llu</total>\n",gAnalysis.TotalPositions);
	fprintf(xmlVarFile,"\t\t</positionstats>\n");
	fprintf(xmlVarFile,"\t</variant>\n");

	/* write array of remoteness stats */
	for(currentRemoteness = gAnalysis.LargestFoundRemoteness; currentRemoteness >= 0; currentRemoteness--) {
		fprintf(xmlVarFile,"\t<remoteness level=\"%d\">\n", currentRemoteness);
		fprintf(xmlVarFile,"\t\t<win>%llu</win>\n", gAnalysis.DetailedPositionSummary[currentRemoteness][0]);
		fprintf(xmlVarFile,"\t\t<lose>%llu</lose>\n", gAnalysis.DetailedPositionSummary[currentRemoteness][1]);
		fprintf(xmlVarFile,"\t\t<tie>%llu</tie>\n", gAnalysis.DetailedPositionSummary[currentRemoteness][2]);
		fprintf(xmlVarFile,"\t\t<draw>0</draw>\n");
		fprintf(xmlVarFile,"\t\t<total>%llu</total>\n", gAnalysis.DetailedPositionSummary[currentRemoteness][0]+
		        gAnalysis.DetailedPositionSummary[currentRemoteness][1]+
		        gAnalysis.DetailedPositionSummary[currentRemoteness][2]);
		fprintf(xmlVarFile,"\t</remoteness>\n");
	}
	fflush(xmlVarFile);
}

/*
** Percentage
*/



float PercentDone (STATICMESSAGE msg)
{
	static POSITION num_pos_seen = 0;
	float percent = 0;
	int total_positions = gNumberOfPositions;
	BOOLEAN useTcl = TRUE;
	if (gHashWindowInitialized) { // Tier-Gamesman Retrograde Solver!
		total_positions = gCurrentTierSize;
		useTcl = FALSE;
		if (msg == AdvanceTier && gTclInterp != NULL && gTotalTiers != 0) {
			percent = 1/(float)gTotalTiers*100;
			char str[100];
			sprintf(str, "advanceProgressBar %f", percent);
			Tcl_Eval(gTclInterp, str);
		}
	} else if (gActualNumberOfPositionsOptFunPtr != NULL) {
		total_positions = gActualNumberOfPositionsOptFunPtr(getOption());
		if (total_positions < 0)
			total_positions = gNumberOfPositions;
	}
	switch (msg)
	{
	case Update:
		num_pos_seen++;
		if (useTcl && gTclInterp != NULL && total_positions >= 1000 && 0 == (num_pos_seen % (total_positions / 1000)))
			Tcl_Eval(gTclInterp, "advanceProgressBar 0.1");
		break;
	case Clean:
		num_pos_seen = 0;
		if (useTcl && gTclInterp != NULL)
			Tcl_Eval(gTclInterp, "advanceProgressBar 0");
		break;
	default:
		break;
	}
	percent = (float)num_pos_seen/(float)total_positions * 100.0;

	return percent;
}

float PercentLoaded (STATICMESSAGE msg)
{
	static POSITION num_pos_loaded = 0;
	int total_positions = gNumberOfPositions;
	BOOLEAN useTcl = TRUE;
	if (gHashWindowInitialized) { // Tier-Gamesman Retrograde Solver!
		total_positions = gCurrentTierSize;
		useTcl = FALSE;
		if (msg == AdvanceTier && gTclInterp != NULL && gTotalTiers != 0) {
			float percent = 1/(float)gTotalTiers;
			char str[100];
			sprintf(str, "advanceLoadingProgressBar %f", percent);
			Tcl_Eval(gTclInterp, str);
		}
	}
	switch (msg)
	{
	case Update:
		num_pos_loaded++;
		if (useTcl && gTclInterp != NULL && total_positions >= 1000 && 0 == (num_pos_loaded % (total_positions / 1000)))
			Tcl_Eval(gTclInterp, "advanceLoadingProgressBar 0.1");
		break;
	case Clean:
		num_pos_loaded = 0;
		if (useTcl && gTclInterp != NULL)
			Tcl_Eval(gTclInterp, "advanceLoadingProgressBar 0");
		break;
	default:
		break;
	}
	return 100.0 * (float)num_pos_loaded / (float)total_positions;
}



/************************************************************************
**
** NAME:        DatabaseCombVisualization
**
** DESCRIPTION: Print to stdout the Comb Visualization (described below)
**              which is essentially all the positions and holes of the DB,
**              encoded as positive and negative numbers respectively.
**
** INPUTS:      none
**
************************************************************************/

void DatabaseCombVisualization()
{
	POSITION thePosition;
	BOOLEAN lastUndecided, thisUndecided;
	long streak = 0, longestUndecided = 0, longestDecided = 0, switches = 0;

	printf("\nThis is called a \"Database Comb Visualization\"\n");
	printf("because we go through the database and find the streaks of\n");
	printf("visited (i.e., known) positions and those the hash reserved\n");
	printf("space for but never used. Every known streak is represented as\n");
	printf("a POSITIVE number and every unknown streak is represented as\n");
	printf("a NEGATIVE number. Thus, a full database of size K will\n");
	printf("simply be printed as the positive number K. Likewise, a half-full database\n");
	printf("with every other position known and unknown will be a sequence of\n");
	printf("positive and negative ones: -1, 1, -1, 1, etc.\n");
	printf("---------------------------------------------------------------------------\n");

	lastUndecided = (GetValueOfPosition(0) == undecided); /* Handles 1st case */

	/* Can you say DAV? We should write an enumerator someday... */
	for(thePosition = 0; thePosition < gNumberOfPositions; thePosition++) {

		thisUndecided = (GetValueOfPosition(thePosition) == undecided);
		if (lastUndecided == thisUndecided) {
			streak++;
			if ( thisUndecided && streak > longestUndecided) longestUndecided = streak;
			if (!thisUndecided && streak > longestDecided  ) longestDecided   = streak;
		}
		else {
			/* Streak of Undecideds prints as a negative # */
			/* Streak of Knowns     prints as a positive # */
			printf("%s%lu\n", (lastUndecided ? "-" : ""), streak);
			streak = 1; /* A new streak of 1 of a different parity */
			switches++;
		}
		lastUndecided = thisUndecided;
	}

	/* Must flush the last bookend one too */
	printf("%s%lu\n", (lastUndecided ? "-" : ""), streak);

	/* Print some stats */
	printf("\n\nLongest   Visited (positive #s) streak: %lu\n", longestDecided);
	printf("Longest UnVisited (negative #s) streak: %lu\n", longestUndecided);
	printf("Total switches we have (# of changes) : %lu\n", switches);

	HitAnyKeyToContinue();
}
