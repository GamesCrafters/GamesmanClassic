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
#include <stdio.h>

//the initial position is ignored!
//this will always return undecided, just enough to signal it completed
//no go agains, connect 4 only.
VALUE DetermineValueBU(POSITION position)
{
	if(!strcmp(kGameName,"Connect-4")) {
		printf("I am sorry, this solver only supports connect 4 for now.");
		return(undecided);
	}

	BOOLEAN 		UserQuit = FALSE;

	int			TotalStage = 16;

	int     		CurrentStage = 16; //4*4 board, will figure out how to do this dynamically later

	//this is what we get from enumerating all positions in the current Stage.
	POSITIONLIST	*phead = NULL;
	POSITIONLIST 	*PosList = NULL;

	MOVELIST	*mhead = NULL;
	MOVELIST	*MoveList = NULL;

	FILE   		*DataFile = NULL;
	char 		DataFileName[11] = "stage  .sdb";
	FILE		*OutFile = NULL;
	char		OutFileName[11] = "stage  .sdb";

	//status
	printf("Solving %s with the bottom up solver.", kGameName);
	do {
		printf("I am starting to solve stage %d", CurrentStage);
		
		if(CurrentStage != TotalStage) {//we are not solving the lowest level, we need the values from last stage from the file
			DataFileName[1] = '0' + (CurrentStage/10);
			DataFileName[0] = '0' + (CurrentStage%10);
			if (!(DataFile = fopen(DataFileName,"r")))
				ExitStageRightErrorString("cannot find the file for the values from last stage, please make sure data file is present");
			//tail = 100, current = 0;
			//while(!feof(DataFile)){

			//read in the values into StageDB
			//fscanf(DataFileName, "%llu %c\n", 

			//}
		}

		phead = PosList = gEnumerateWithinStage(CurrentStage);

		OutFileName[1] = '0' + (CurrentStage-1)/10;
		OutFileName[0] = '0' + (CurrentStage-1)%10;
		OutFile = fopen(OutFileName,"w");

		while(phead != NULL) {

			BOOLEAN  foundTie = FALSE, foundLose = FALSE, foundWin = FALSE;
			VALUE currentValue = undecided;
			POSITION postosolve = phead->position, child;

			//solve like the non-loopy solver, but does not recurse
			if(CurrentStage != TotalStage) { //we are solving non-primitives
				mhead = MoveList = GenerateMoves(postosolve);
				for( ; mhead != NULL; mhead = mhead->next) {
					child = DoMove(postosolve, mhead->move);
					currentValue = GetValueOfPosition(child);
					if(currentValue == lose) {
						foundLose = TRUE;
					}
					else if(currentValue == tie) {
						foundTie = TRUE;
					}
					else if(currentValue == win) {
						foundWin = TRUE;
					}
					else printf("undecided position encountered in %llu", child);
				}
				FreeMoveList(MoveList);
				if(foundLose)
					StoreValueOfPosition(postosolve, win);
				else if(foundTie)
					StoreValueOfPosition(postosolve, tie);
				else if(foundWin)
					StoreValueOfPosition(postosolve, lose);
				else //the position is wierd.
					printf("encountered unsolvable value for node %llu. WATCH OUT!!", postosolve);
			}
			else { //we are solving primitives
				currentValue = Primitive(postosolve);
				if (currentValue == undecided) {
					printf("encountered an undecided value for a primitive %llu. Skipping it.\n", postosolve);
				} else {
					StoreValueOfPosition(position,currentValue);
				}
			}
			
			fprintf(OutFile, "%llu %c\n", phead->position, currentValue);
	
			phead = phead->next;
		}
	
		FreePositionList(PosList);
		fclose(DataFile);
		fclose(OutFile);

		CurrentStage--;

		printf("I have finished solving stage %d. Do you want to continue solving the next stage? (y/n)", CurrentStage);

		char response = getchar();
		UserQuit = (response == 'y');

	}while (!UserQuit && CurrentStage != 0);

	return (undecided);

}
