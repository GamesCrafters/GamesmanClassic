/************************************************************************
**
** NAME:	solvebottomup.c
**
** DESCRIPTION:	Bottom Up solver.
**
** AUTHOR:	GamesCrafters Research Group, UC Berkeley
**		Supervised by Dan Garcia <ddgarcia@cs.berkeley.edu>
**
** DATE:	2005-11-25
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

#define MAX_FN_LEN 40

//DO NOT USE THIS WITH A LOOPY GAME, IT WILL SPIN IN AN INFINITE LOOP

int TotalStages;

FILE *StageFile;

void newStageFile()
{
	char filename[MAX_FN_LEN];
	int i;

	for (i = 0; i < MAX_FN_LEN; i++)
		filename[i] = ' ';
	sprintf(filename, "./stages/%s_stage%d.txt", kDBName, TotalStages);
	if ((StageFile = fopen(filename, "wb")) == NULL) {
		printf("Unable to create file for writing positions in a stage. Aborting.");
		ExitStageRight();
		exit(1);
	}
}

void openStageFile(FILE **filep, int stagenum)
{
	int i;
	char filen[MAX_FN_LEN];

	for (i = 0; i < MAX_FN_LEN; i++)
		filen[i]=' ';
	sprintf(filen, "./stages/%s_stage%d.txt", kDBName, stagenum);
	if (((*filep) = fopen(filen, "r")) == NULL) {
		printf("unable to create file for reading positions in a stage. Aborting.");
		ExitStageRight();
		exit(1);
	}
}

/* walk the game tree in a BFS (no graphs please) and generate files with the names
        "./stages/1210_stage1_positions.txt"
   to aid solving the stuff*/
void WalkGameTree()
{
	POSITIONQUEUE *head, *tail, *stagetail;
	BOOLEAN isPrimitive, beginNewStage;
	MOVELIST      *currentMoves, *currentMovesHead;
	POSITION currentPos, childPos;

	tail = NULL;
	AddPositionToQueue(gInitialPosition, &tail);
	head = tail;
	stagetail = tail;
	TotalStages = -1;

	mkdir("stages", 0755);

	printf("I am walking the game tree now.\n");

	beginNewStage = TRUE;

	UnMarkAllAsVisited();

	do {
		currentPos = head->position;

		/*write the position to the first file;*/
		if (beginNewStage) {
			TotalStages++;
			printf("walking stage %d\n", TotalStages);
			newStageFile();
			beginNewStage = FALSE;
		}

		fprintf(StageFile, POSITION_FORMAT "\n", currentPos);
		fflush(StageFile);

		currentMoves = currentMovesHead = GenerateMoves(currentPos);

		//this must hold true since we are always considering legal positions
		//they can only lead to valid positions
		//and even if primitives might have more moves ahead we stop already
		isPrimitive = (currentMoves == NULL || Primitive(currentPos) != undecided);

		if (!isPrimitive) {
			for(; currentMovesHead != NULL; currentMovesHead = currentMovesHead->next) {
				childPos = DoMove(currentPos, currentMovesHead->move);
				if (!Visited(childPos)) {
					AddPositionToQueue(childPos, &tail);
					MarkAsVisited(childPos);
				}
			}
		}

		if (head == stagetail) //we have finished this stage, rotating
		{
			fclose(StageFile);
			stagetail = tail;
			beginNewStage = TRUE;
		}

		FreeMoveList(currentMoves);

		RemovePositionFromQueue(&head);

	} while (head != NULL);
}

VALUE DetermineValueBU(POSITION position)
{
	if(kLoopy == TRUE) {
		printf("I am sorry, this solver only supports non-loopy games.");
		return(undecided);
	}

	int CurrentStage;

	MOVELIST        *mhead = NULL, *MoveList = NULL;

	FILE            *OutFile = NULL;

	BOOLEAN foundTie, foundLose, foundWin;
	VALUE currentValue, oldValue;
	REMOTENESS winRemoteness, loseRemoteness, tieRemoteness, childrmt;
	POSITION postosolve, child;

	//status
	//TODO: just put the results in a colldb for now....

	printf("\nSolving %s with the bottom up solver.\n", kGameName);

	WalkGameTree();

	CurrentStage = TotalStages;

	BOOLEAN foundnewvalue = TRUE;

	//UnMarkAllAsVisited();

	do {
		printf("I am starting to solve stage %d\n", CurrentStage);

		openStageFile(&OutFile, CurrentStage);

		//if(CurrentStage == TotalStages) {  //primitives

		//      while(!feof(OutFile)) {
		//            fscanf(OutFile, POSITION_FORMAT"\n", &postosolve);
		//          StoreValueOfPosition(postosolve, Primitive(postosolve));

		//}

		//} else {
		//TODO: put thisdb to lastdb and recreate thisdb

		foundnewvalue = TRUE;

		while(foundnewvalue) {

			//reset to start reading from the beginning
			fseek(OutFile, 0, SEEK_SET);

			foundnewvalue = FALSE;

			while(!feof(OutFile)) {

				foundTie = FALSE;
				foundLose = FALSE;
				foundWin = FALSE;
				currentValue = undecided;
				winRemoteness = tieRemoteness = REMOTENESS_MAX;
				loseRemoteness = 0;

				//read in the positions we need to solve with
				fscanf(OutFile, POSITION_FORMAT "\n", &postosolve);

				//            printf("read out position "POSITION_FORMAT"\n", postosolve);

				//if the position is not solved yet, or is depending on other positions in the stage
				//if (!Visited(postosolve) || GetValueOfPosition(postosolve) == undecided) {

				//MarkAsVisited(postosolve);
				oldValue = GetValueOfPosition(postosolve);

				if ((currentValue = Primitive(postosolve)) != undecided) {
					SetRemoteness(postosolve,0);
					StoreValueOfPosition(postosolve, currentValue);
				} else {
					//infer the value from children, but does not recurse
					mhead = MoveList = GenerateMoves(postosolve);
					for(; mhead != NULL; mhead = mhead->next) {
						child = DoMove(postosolve, mhead->move);
						currentValue = GetValueOfPosition(child);
						childrmt = Remoteness(child);

						if(currentValue == lose) {
							foundLose = TRUE;
							if (winRemoteness > childrmt)
								winRemoteness = childrmt;
						} else if(currentValue == tie) {
							foundTie = TRUE;
							if (tieRemoteness > childrmt)
								tieRemoteness = childrmt;
						} else if(currentValue == win) {
							foundWin = TRUE;
							if (loseRemoteness < childrmt)
								loseRemoteness = childrmt;
						}
						/*TODO:if I am winning I want to give you the shortest lose
						        if I am losing I want to give you the longest win
						        if we are tied I want to give you the longest tie
						        handle those here */
						//if (minrmt > childrmt)
						//        minrmt = childrmt;
						//if (maxrmt < childrmt)
						//        maxrmt = childrmt;
					}

					FreeMoveList(MoveList);

					if(foundLose) {
						SetRemoteness(postosolve, winRemoteness + 1);
						StoreValueOfPosition(postosolve, win);
					} else if(foundTie) {
						SetRemoteness(postosolve, tieRemoteness + 1);
						StoreValueOfPosition(postosolve, tie);
					} else if(foundWin) {
						SetRemoteness(postosolve, loseRemoteness + 1);
						StoreValueOfPosition(postosolve, lose);
					}
					//otherwise this position will probably have to wait.

					//                      printf("I am looking at "POSITION_FORMAT" and got a %s\n", postosolve, gValueString[GetValueOfPosition(postosolve)]);
				}

				foundnewvalue = (foundnewvalue || (oldValue != GetValueOfPosition(postosolve)));
				//}
			}  //while(!feof(OutFile)) - find value for one position
		} //while(foundnewvalue) - find value for all deducable positions in a stage

		fseek(OutFile, 0, SEEK_SET);

		//take care of the all the draws
		while (!feof(OutFile)) {
			fscanf(OutFile, POSITION_FORMAT "\n", &postosolve);
			if (GetValueOfPosition(postosolve) == undecided) {
				SetRemoteness(postosolve, REMOTENESS_MAX);
				StoreValueOfPosition(postosolve, tie);
				//this is what a draw is, a tie with remoteness REMOTENESS_MAX.
			}
		}

		//fprintf(OutFile, "%llu %c\n", postosolve, gValueString[currentValue]);
		//}

		//TODO: save db for this stage

		fclose(OutFile);

		printf("I have finished solving stage %d.\n", CurrentStage);
		// Do you want to continue solving the next stage? (y/n)", CurrentStage);

		//char response = getchar();
		//UserQuit = (response == 'y');

		CurrentStage--;

	} while (CurrentStage >= 0)
	;

	return (GetValueOfPosition(gInitialPosition));

}
