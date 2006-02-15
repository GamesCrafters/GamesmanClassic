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

//DO NOT USE THIS WITH A LOOPY GAME, IT WILL EVENTUALLY EXPLODE IN MEMORY USAGE

int   totalstages;

char  filename[MAX_FN_LEN];
FILE *stagefile;

void newStageFile()
{
        int i;
        for (i = 0; i < MAX_FN_LEN; i++)
                filename[i] = ' ';
        sprintf(filename, "./stages/%s_stage%d.txt", kDBName, totalstages);
        if ((stagefile = (fopen(filename, "wb"))) == NULL) {
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
        sprintf(filename, "./stages/%s_stage%d.txt", kDBName, stagenum);
        if (((*filep) = fopen(filen, "r")) == NULL) {
                printf("unable to create file for reading positions in a stage. Aborting.");
                ExitStageRight();
                exit(1);
        }
}

/* walk the game tree (no graphs please) and generate files with the names
	"./stages/1210_stage1_positions.txt"
   to aid solving the stuff*/
void WalkGameTree()
{
        /*
          initialize head, tail, stagetail, and open the first file, totalstages = 0;
        */

        POSITIONQUEUE *head, *tail, *stagetail;
        BOOLEAN        isPrimitive, beginNewStage;
        MOVE           currentMove;
        MOVELIST      *currentMoves, *currentMovesHead;
        POSITION       currentPos, childPos;

        tail = NULL;
        AddPositionToQueue(gInitialPosition, &tail);
        head = tail;
        stagetail = tail;
        totalstages = 0;

        mkdir("stages", 0755);

        printf("I am walking the game tree now.\n");

        beginNewStage = TRUE;

        do {
                currentPos = head->position;

                /*write the position to the first file;*/
                if (beginNewStage) {
                        printf("walking stage %d\n", totalstages);
                        newStageFile();
                        totalstages++;
                        beginNewStage = FALSE;
                }

                fprintf(stagefile, POSITION_FORMAT"\n", currentPos);

                currentMoves = currentMovesHead = GenerateMoves(currentPos);

                //this must hold true since we are always considering legal positions
                //they can only lead to valid positions
                //and even if primitives might have more moves ahead we stop already
                isPrimitive = (currentMoves == NULL || Primitive(currentPos) != undecided);

                if (!isPrimitive) {
                        for(; currentMovesHead != NULL; currentMovesHead = currentMovesHead->next) {
                                currentMove = currentMovesHead->move;
                                childPos = DoMove(currentPos, currentMove);
                                AddPositionToQueue(childPos, &tail);
                        }

                        if (head == stagetail) //we have finished this stage, rotating
                        {
                                fclose(stagefile);
                                stagetail = tail;
                                beginNewStage = TRUE;
                        }
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

        BOOLEAN UserQuit = FALSE;

        int	CurrentStage;

        POSITION  solvingpos;

        //this is what we get from enumerating all positions in the current Stage.
        POSITIONLIST	*phead = NULL, *PosList = NULL;

        MOVELIST	*mhead = NULL, *MoveList = NULL;

        FILE   		*OutFile = NULL;

        //status
        //TODO: just put the results in a colldb for now....

        printf("\nSolving %s with the bottom up solver.\n", kGameName);

        WalkGameTree();

        CurrentStage = totalstages;

        do {
                printf("I am starting to solve stage %d\n", CurrentStage);

                if(CurrentStage == totalstages) {  //primitives
                        openStageFile(&OutFile, CurrentStage);
                        while(!feof(OutFile)) {
                                fscanf(OutFile, POSITION_FORMAT"\n", &solvingpos);
                                StoreValueOfPosition(solvingpos, Primitive(solvingpos));
                        }
                        //TODO: save thisdb

                } else {
                        //TODO: dump the lastdb if there is one, put thisdb to lastdb
                        // and recreate thisdb

                        openStageFile(&OutFile, CurrentStage);

                        BOOLEAN foundnewvalue = TRUE;

                        while(foundnewvalue) {

                                //reset to start reading from the beginning
                                fseek(OutFile, 0, SEEK_SET);

                                while(!feof(OutFile)) {

                                        BOOLEAN  foundTie = FALSE, foundLose = FALSE, foundWin = FALSE;
                                        VALUE currentValue = undecided;
                                        POSITION postosolve, child;

                                        //read in the positions we need to solve with
                                        fscanf(OutFile, POSITION_FORMAT"\n", &postosolve);

                                        //solve like the non-loopy solver, but does not recurse
                                        mhead = MoveList = GenerateMoves(postosolve);
                                        for( ; mhead != NULL; mhead = mhead->next) {
                                                child = DoMove(postosolve, mhead->move);
                                                currentValue = GetValueOfPosition(child);
                                                if(currentValue == lose) {
                                                        foundLose = TRUE;
                                                } else if(currentValue == tie) {
                                                        foundTie = TRUE;
                                                } else if(currentValue == win) {
                                                        foundWin = TRUE;
                                                }
                                        }

                                        FreeMoveList(MoveList);

                                        if(foundLose)
                                                StoreValueOfPosition(postosolve, win);
                                        else if(foundTie)
                                                StoreValueOfPosition(postosolve, tie);
                                        else if(foundWin)
                                                StoreValueOfPosition(postosolve, lose);
                                        //otherwise this position will probably have to wait.

                                        foundnewvalue = (foundnewvalue || foundTie || foundLose || foundWin);
                                }  //find value for one position
                        } //find value for all deducable positions

                        //TODO: need to reread the whole file to get draws in. Save thisdb

                        //fprintf(OutFile, "%llu %c\n", postosolve, gValueString[currentValue]);
                        fclose(OutFile);
                }

                printf("I have finished solving stage %d. Do you want to continue solving the next stage? (y/n)", CurrentStage);

                char response = getchar();
                UserQuit = (response == 'y');

                CurrentStage--;

        } while (!UserQuit && CurrentStage >= 0);

        return (GetValueOfPosition(gInitialPosition));

}
