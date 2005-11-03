/************************************************************************
**
** NAME:	solvebottomup.c
**
** DESCRIPTION:	Bottom Up solver.
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

#include "gamesman.h"
#include <stdio.h>

int TotalStage = 16;

//helper data structures
typedef struct PosValuePair_t{
    POSITION 	position;
    VALUE	value;
    struct PosValuePair_t *next;
} POSVALUEPAIR;


//the initial position is ignored!
VALUE DetermineValueBU(POSITION position)
{
    if(!strcmp(kGameName,"Connect-4")) {
	printf("I am sorry, this solver only supports connect 4 for now.");
	return(undecided);
    }

    BOOLEAN 		UserQuit = FALSE;

    int     		CurrentStage = 16; //4*4 board, will figure out how to do this dynamically later

    //this is what we get from enumerating all positions in the current Stage.
    POSITIONLIST	*head;
    POSITIONLIST 	*PosList = NULL;

    int			ValueListLen = 100;
    POSVALUEPAIR	*tail, *current;
    //This is the value list from last stage.
    POSVALUEPAIR    	*ValueList = SafeMalloc(sizeof(VALUE)*ValueListLen);

    FILE   		*DataFile = NULL;
    char 		DataFileName[7] = "stage  ";
    FILE		*OutFile = NULL;
    char		OutFileName[7] = "stage  ";

    //status
    printf("Solving %s with the bottom up solver.", kGameName);
    do {

	if(CurrentStage != TotalStage) {//we are not solving the lowest level, we need the values from last stage
	    DataFileName[1] = '0' + (CurrentStage/10);
	    DataFileName[0] = '0' + (CurrentStage%10);
	    if (!(DataFile = fopen(DataFileName,'r')))
		ExitStageRightErrorString("cannot find the file for the values from last stage, please make sure data file is present");
	    tail = 100, current = 0;
	    while(!feof(DataFile)){

		//read in the values into StageDB

	    }
	}

	head = PosList = gEnumerateWithinStage(CurrentStage);

	OutFileName[1] = '0' + (CurrentStage-1)/10;
	OutFileName[0] = '0' + (CurrentStage-1)%10;
	OutFile = fopen(OutFileName,'w');

	while(head != NULL) {

	    BOOLEAN  foundTie = FALSE, foundLose = FALSE, foundWin = FALSE;
	    VALUE currentvalue;
	    POSITION postosolve = head->position;

	    //solve
	    currentvalue = Primitive(postosolve);
	    if (currentvalue == undecided) { //we better infer its value from the last stage
		
	    }
	    
	    fprintf(OutFile, "%llu %c\n", head->position, currentvalue);
	
	    head = head->next;
	}
	
	SafeFree(PosList);
	fclose(DataFile);
	fclose(OutFile);

	printf("I have finished solving stage %d. Do you want to continue solving the next stage? (y/n)", CurrentStage);

	char response = getchar();
	UserQuit = (response == 'y');

    }while (!UserQuit && CurrentStage != 0);

    return (undecided);

}
