/************************************************************************
**
** NAME:	analysis.c
**
** DESCRIPTION:	Contains the majority of game analysis functionality.
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


/*
** Globals
*/

int   gHashEfficiency = 0;
float gAverageFanout = +0.0f;
long  gTotalPositions = 0;
long  gTotalMoves = 0;
long  gWinCount = 0, gLoseCount = 0, gTieCount = 0, gUnknownCount = 0;
long  gPrimitiveWins = 0, gPrimitiveLoses = 0, gPrimitiveTies = 0;


/*
** Code
*/

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
    } else
        fp = stdout;
    
    fprintf(fp,"Position/Value list for %s\n\n", kGameName);
    fprintf(fp,"POS  | VISITED-FLAG VALUE in REMOTENESS\n");
    
    for(i=0 ; i<gNumberOfPositions ; i++)
        if((value = GetValueOfPosition((POSITION)i)) != undecided)
            fprintf(fp,POSITION_FORMAT " | %c %4s in %d\n",
		    i,
		    Visited((POSITION)i) ? 'V' : '-',
		    gValueString[value],
		    Remoteness((POSITION)i));
    
    if(toFile)
        fclose(fp);
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
        for(j = 0 ; ptr != NULL && j < maxPositions ;j++) {
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
        for(j = 0 ; ((thePosition = GetNextPosition()) != kBadPosition) && j < maxPositions ;) {
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
    POSITION thePosition;
    VALUE theValue;
    int j;
    char yesOrNo;
    
    j = 0;
    continueSearching = TRUE;
    do {
        for(j = 0 ; ((thePosition = GetNextPosition()) != kBadPosition) && j < maxPositions ;) {
            theValue = GetValueOfPosition(thePosition);
            if((theValue == win  && (c == 'w' || c == 'W')) || 
	       (theValue == lose && (c == 'l' || c == 'L')) ||
	       (theValue == tie  && (c == 't' || c == 'T'))) {
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
            printf("\nThere are no more %s positions to list...\n",
		   c == 'w' || c == 'W' ? "winning" : c == 'l' || c == 'L' ? "losing" : "tieing");
    } while (continueSearching && (thePosition != kBadPosition));
    HitAnyKeyToContinue();
}



void PrintGameValueSummary()
{
    
    
    printf("\n\n\t----- Summary of Game values -----\n\n");
    
    printf("\tValue       Number       Total\n");
    printf("\t------------------------------\n");
    printf("\tLose      = %5lu out of %lu\n",gLoseCount,gTotalPositions);	
    printf("\tWin       = %5lu out of %lu\n",gWinCount,gTotalPositions);	
    printf("\tTie       = %5lu out of %lu\n",gTieCount,gTotalPositions);	
    printf("\tUnknown   = %5lu out of %lu\n",gUnknownCount,gTotalPositions);	
    printf("\tTOTAL     = %5lu out of %lu allocated\n",
	   gTotalPositions,
	   gNumberOfPositions);
    
    printf("\tHash Efficiency                   = %6d\%%        \n",gHashEfficiency);
    printf("\tTotal Moves                       = %5lu\n",gTotalMoves);
    printf("\tAvg. number of moves per position = %2f           \n", gAverageFanout);
    printf("\tTotal Primitive Wins              = %5lu\n", gPrimitiveWins);
    printf("\tTotal Primitive Loses             = %5lu\n", gPrimitiveLoses);
    printf("\tTotal Primitive Ties              = %5lu\n", gPrimitiveTies);
    
    
    return;
    
}


/** Analysis **/

void analyze()
{
  static int analyzed = 0;
  static int previousOption = 0;
  
  if (analyzed == 0 || previousOption != getOption())
    {
      analyzer();
      analyzed = 1;
    }
  
}

void analyzer()
{    
    POSITION thePosition;
    VALUE theValue;
    long winCount, loseCount, tieCount, unknownCount;
    long primitiveWins, primitiveLoses, primitiveTies;
    long reachablePositions;
    long totalPositions;
    int  hashEfficiency;
    float averageFanout;
    
    totalPositions = winCount = loseCount = tieCount = unknownCount = 0;
    primitiveWins = primitiveLoses = primitiveTies = 0;
    reachablePositions = 0;
    hashEfficiency = 0;
    averageFanout = 0;
    for(thePosition = 0 ; thePosition < gNumberOfPositions ; thePosition++) 
	{
	    theValue = GetValueOfPosition(thePosition);
	    if (theValue != undecided) {
		totalPositions++;
		if(theValue == win)  {
		    winCount++;
		    reachablePositions++;
		    if (Remoteness(thePosition) == 0) primitiveWins++;
		} else if(theValue == lose) {
		    loseCount++;
		    reachablePositions++;
		    if (Remoteness(thePosition) == 0) primitiveLoses++;
		} else if(theValue == tie) {
		    tieCount++;
		    reachablePositions++;
		    if (Remoteness(thePosition) == 0) primitiveTies++;
		} else {
		    unknownCount++;
		}
	    }
	}
    hashEfficiency = (int)((((float)reachablePositions ) / (float)gNumberOfPositions) * 100.0); 
    averageFanout = (float)((float)gTotalMoves/(float)reachablePositions);
    
    gHashEfficiency = hashEfficiency;
    gAverageFanout = averageFanout;
    gTotalPositions = totalPositions;
    gWinCount = winCount;
    gLoseCount = loseCount;
    gTieCount = tieCount;
    gUnknownCount = unknownCount;
    gPrimitiveWins = primitiveWins;
    gPrimitiveLoses = primitiveLoses;
    gPrimitiveTies = primitiveTies;
    
}



// Write variant statistic
void writeVarStat(char * statName, char * text, FILE *rowp)
{
    FILE * filep;
    //FILE * rawfilep ;
    char outFileName[256];
    
    sprintf(outFileName, "analysis/%s/var%d/%s", kDBName,getOption(),statName) ;
    
    filep = fopen(outFileName, "w");
    
    
    fprintf(filep,"<!-- AUTO CREATED, do //not modify-->\n");
    fprintf(filep,text);
    fprintf(filep,"\n");
    
    
    fprintf(rowp,"<td ALIGN = ""center""><!--#include virtual=\"%s\"--></td>\n",statName);
    
    
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
    mkdir(varDirName, 0755) ;
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
    fprintf(tablep,"<table align=""ABSCENTER"" BORDER =""1"" CELLSPACING=""0"" CELLPADDING=""5"">\n");
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
    
    sprintf(rowFileName, "analysis/%s/var%d/row.shtml", kDBName,getOption()) ;
    
    rowp = fopen(rowFileName, "w");
    
    /***********************************
    Variant Specific
    ************************************/
    
    
    fprintf(rowp,"<!-- AUTO CREATED, do not modify-->\n");
    
    sprintf(text, "%d",getOption());
    writeVarStat("option",text,rowp);
    
    writeVarStat("value", gValueString[(int)gValue], rowp);
    
    sprintf(text, "%5lu", gWinCount);
    writeVarStat("WinCount", text, rowp);
    
    sprintf(text, "%5lu", gLoseCount);
    writeVarStat("LoseCount", text, rowp);
    
    sprintf(text, "%5lu", gTieCount);
    writeVarStat("TieCount", text, rowp);
    
    sprintf(text, "%5lu", gPrimitiveWins);
    writeVarStat("Prim.WinCount", text, rowp);
    
    sprintf(text, "%5lu", gPrimitiveLoses);
    writeVarStat("Prim.LoseCount", text, rowp);
    
    sprintf(text, "%5lu", gPrimitiveTies);
    writeVarStat("Prim.TieCount", text, rowp);
    
    
    sprintf(text, "%5lu", gTotalPositions);
    writeVarStat("totalPositions", text , rowp);
    
    sprintf(text, "%5lu", gNumberOfPositions);
    writeVarStat("NumberOfPositions", text, rowp);
    
    sprintf(text, "%d", gHashEfficiency);
    writeVarStat("hashEfficiency", text, rowp);
    
    
    sprintf(text, "%2f", gAverageFanout);
    writeVarStat("AverageFanout", text, rowp);
    
    sprintf(text, "%d", gTimer);
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
    for(position=0 ; position<gNumberOfPositions ; position++) { /* for all positions */
        parentIsWin = FALSE;
        foundLosingChild = FALSE;
        parentIsTie = FALSE;
        foundTieingChild = FALSE;
        if((parentValue = GetValueOfPosition(position)) != undecided) { /* if valid position */
            if(Primitive(position) == undecided) { /* Not Primitive, children */
                head = ptr = GenerateMoves(position);
                while (ptr != NULL) {
                    child = DoMove(position,ptr->move);  /* Create the child */
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
                    ptr = ptr->next;                     /* Go to the next child */
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
** Percentage
**
** TODO: I'll let you actually implement this one, Robert. -JJ
** Also, please make it Pascal Case (PercentDone), like the rest of
** gamesman's naming convention.
*/

float percentDone (STATICMESSAGE msg)
{
	return 0.0f;
}
