/************************************************************************
**
** NAME:	gameplay.c
**
** DESCRIPTION:	Textual interface and computer intelligence code that is
**		active when a game is being played.
**
** AUTHOR:	GamesCrafters Research Group, UC Berkeley
**		Supervised by Dan Garcia <ddgarcia@cs.berkeley.edu>
**
** DATE:	2005-01-11
**
** LAST CHANGE: $Id: gameplay.c,v 1.56 2007-11-14 23:21:16 alb_shau Exp $
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
#include "httpclient.h"
#include "openPositions.h"
#include "globals.h"
#include "seval.h"

#define leftJustified 1
#define rightJustified 2

typedef struct moveList
{
	MOVE move;
	POSITION position;
	TIERPOSITION tierposition; // Added for
	TIER tier; // Tier-Gamesman moves
	MOVELIST* generatedMoves;
	struct moveList* next;
	struct moveList* prev;
}
moveList;

static moveList* mList;
static int tiesExist = 0;
static int drawsExist = 0;
static int playerOne = 0;
static int playerTwo = 1;

static int maxN = 10;
static int maxPossibleMoveLength = 30;
static int maxPossibleLineLength = 256;

BOOLEAN usingLookupTierDB = FALSE;

/*
** Local function prototypes
*/

static VALUE_MOVES*    SortMoves                       (POSITION, MOVE, VALUE_MOVES*);
static VALUE_MOVES*    StoreMoveInList                 (MOVE, REMOTENESS, VALUE_MOVES*, int);
static moveList*       moveListHandleUndo              (moveList*);
static moveList*       moveListHandleNewMove           (POSITION, MOVE, moveList*, MOVELIST*);
static void            moveListHandleGameOver             (moveList*);

extern MOVE             RandomLargestRemotenessMove     (MOVELIST*, REMOTENESSLIST*);
extern MOVE             RandomSmallestRemotenessMove    (MOVELIST*, REMOTENESSLIST*);

extern MOVE             GetWinByMove                    (POSITION, MOVELIST*);


void             PrintMoveHistory                (POSITION);
void             PrintVisualValueHistory         (POSITION, int);

/**
 * Prototypes for delta-Remoteness
 **/
REMOTENESS findMaxRemoteness (REMOTENESSLIST*);
REMOTENESS findMinRemoteness(REMOTENESSLIST*);
REMOTENESS FindDelta(REMOTENESS, REMOTENESSLIST*, VALUE);
REMOTENESS FindWinningDelta(REMOTENESS, REMOTENESSLIST*);
REMOTENESS FindLosingDelta(REMOTENESS, REMOTENESSLIST*);
REMOTENESS FindTieingDelta(REMOTENESS, REMOTENESSLIST*);
BOOLEAN DEBUG = FALSE;

OPPONENT gOpponent;

void PlayGame(PLAYER playerOne, PLAYER playerTwo)
{
	char c;
	POSITION position;
	MOVE move;
	UNDO *undo;
	VALUE value;
	BOOLEAN error;
	BOOLEAN player_draw = FALSE;
	gPlaying = TRUE;
	BOOLEAN aborted = FALSE;
	BOOLEAN menu    = TRUE;
	USERINPUT userInput = Continue; /* default added to satify compiler */
	PLAYER player = playerOne, swapPlayer; // swapPlayer is just a temporary for use during player switching
	int oldRemainingGivebacks;

	moveList* mlist = 0;
	MOVELIST* generatedMoves = 0;

	position = gInitialPosition;
	if (gHashWindowInitialized) { // TIER GAMESMAN
		usingLookupTierDB = ReinitializeTierDB();
		gInitializeHashWindow(gInitialTier, !usingLookupTierDB);
		position = gHashToWindowPosition(gInitialTierPosition, gInitialTier);
	}
	undo = InitializeUndo();

#ifndef X
	printf("Type '?' if you need assistance...\n\n");
#endif

	while(gPlaying) {

		while(!aborted &&
		      !player_draw &&
		      ((value = Primitive(position)) == undecided ||
		       userInput==Configure ||
		       userInput==Switch)) {
			oldRemainingGivebacks = remainingGivebacks;

			PrintPosition(position, player->name, (player->type==Human ? TRUE : FALSE));
			userInput = player->GetMove(position,&move,player->name);
			// (SP07) hfwang - rewrote game loop
			/*end of players' setup*/
			switch (userInput) {
			case Switch:
				if (gOpponent == AgainstComputer) {
					// swap the two players
					swapPlayer = playerOne;
					playerOne = playerTwo;
					playerTwo = swapPlayer;
					// need to update player->turn
					playerOne->turn = 0;
					playerTwo->turn = 1;
				}
				break;
			case Undo:
				undo = HandleUndoRequest(&position,undo,&error);
				if(!error && gOpponent == AgainstHuman) {
					player = (player->turn ? playerOne : playerTwo);         // swap player
				}
				if (!error) {
					mlist = moveListHandleUndo(mlist);
				}
				break;
			case Abort:
				aborted = TRUE;
				break;
			case Configure:
				break;
			default:
				// UserInput is a move
				generatedMoves = GenerateMoves(position);
				mlist = moveListHandleNewMove(position, move, mlist, generatedMoves);
				position = DoMove(position,move);
				if (gHashWindowInitialized)         // TIER GAMESMAN
					gInitializeHashWindowToPosition(&position, !usingLookupTierDB);
				undo = UpdateUndo(position,undo,&player_draw);
				undo->givebackUsed = oldRemainingGivebacks>remainingGivebacks;

				// swap players as needed
				if(!gGoAgain(position,move)) player = (player->turn ? playerOne : playerTwo);

				//game over!
				if (player_draw) {
					gPlaying = FALSE;
					break;
				}

			}
		}

		PrintPosition(position,player->name,(player->type==Human ? TRUE : FALSE));
		switch(value) {
		case tie:
			printf("\nThe match ends in a tie. Excellent strategies, %s and %s!\n",playerOne->name,playerTwo->name);
			break;
		case lose:
			printf("\n%s (player %s) Wins!\n\n",player->turn ? playerOne->name : playerTwo->name,player->turn ? "one" : "two");
			break;
		case win:
			printf("\n%s (player %s) Wins!\n\n",player->turn ? playerTwo->name : playerOne->name,player->turn ? "two" : "one");
			break;
		default:
			if(aborted) {
				printf("Your abort command has been received and successfully processed!\n");
				moveListHandleGameOver(mlist);
				gPlaying = FALSE;
			} else if(player_draw) {
				printf("\nThe match ends in a draw. Excellent strategies, %s and %s! \n\n",playerOne->name,playerTwo->name);
			} else {
				BadElse("PlayGame");
			}
		}
		if(!aborted) {
			while(menu) {

				printf("\n\t----- Postgame Menu for %s -----\n\n", kGameName);

				printf("%s",
				       (gOpponent != ComputerComputer) ?
				       "\tu)\t(U)ndo the last move\n" : "");
				if (!gUnsolved) {
					printf("%s",
					       "\tv)\tPrint (V)isual Value History\n");
					printf("%s", "\tw)\tPrint Visual Value History (W)ith All Possible Moves\n");
				}

				printf("%s",
				       "\tm)\tScript of (M)ove History\n");
				printf("%s%s%s",
				       "\tb)\t(B)ack to the Play Menu\n",
				       "\tq)\t(Q)uit\n\n",

				       "Select an option:  ");
				c = GetMyChar();
				switch(c) {
				case 'u':
				case 'U':
					if(gOpponent != ComputerComputer) {
						undo = HandleUndoRequest(&position,undo,&error);

						if(!error && gOpponent == AgainstHuman) {
							player = (player->turn ? playerOne : playerTwo);
						}
						if (!error) {
							mlist = moveListHandleUndo(mlist);
						}
						gPlaying = TRUE;

						menu = FALSE;
					} else
						BadMenuChoice();
					break;
				case 'b':
				case 'B':
					moveListHandleGameOver(mlist);
					gPlaying = FALSE;
					menu = FALSE;
					break;
				case 'q':
				case 'Q':
					ExitStageRight();
					exit(0);

				case 'm':
				case 'M':
					PrintMoveHistory(-1);
					break;
				case 'v':
				case 'V':
					if (gUnsolved) {
						BadMenuChoice();
						HitAnyKeyToContinue();
						break;
					}
					PrintVisualValueHistory(-1, 0);
					break;
				case 'w':
				case 'W':
					if (gUnsolved) {
						BadMenuChoice();
						HitAnyKeyToContinue();
						break;
					}
					PrintVisualValueHistory(-1, 1);
					break;
				default:
					BadMenuChoice();
				}
			}
			menu = TRUE;
		}
	}
	ResetUndoList(undo);
}

/* Jiong */
moveList* moveListHandleUndo(moveList* lastEntry)
{
	moveList* temp = lastEntry;
	if (gOpponent == AgainstHuman) {
		lastEntry = lastEntry->prev;
	} else {
		lastEntry = lastEntry->prev->prev;
		SafeFree(temp->prev);
	}
	SafeFree(temp);
	if (lastEntry != 0) {
		lastEntry->next = 0;
	} else {
		mList = 0;
	}
	return lastEntry;
}


moveList* moveListHandleNewMove(POSITION position, MOVE move,
                                moveList* lastEntry, MOVELIST* generatedMoves)
{
	moveList* newmlist;
	newmlist = (moveList*) SafeMalloc(sizeof(moveList));
	newmlist->move = move;
	newmlist->generatedMoves = generatedMoves;
	newmlist->position = position;
	if (gHashWindowInitialized) { // Tier-Gamesman
		gUnhashToTierPosition(position, &newmlist->tierposition,
		                      &newmlist->tier);
	}
	newmlist->next = 0;
	newmlist->prev = lastEntry;
	if (lastEntry == 0) {
		lastEntry = newmlist;
		mList = newmlist;
	} else {
		lastEntry->next = newmlist;
		lastEntry = newmlist;
	}
	return lastEntry;
}

void moveListHandleGameOver(moveList* lastEntry)
{
	moveList* temp = lastEntry;
	while(lastEntry != 0) {
		lastEntry = lastEntry->prev;
		SafeFree(temp);
		temp = lastEntry;
	}
	mList = 0;
}

void PrintMoveHistory(POSITION position)
{
	int whoseTurn = kPlayerOneTurn;
	int ct = 1;
	moveList* mlist = mList;
	printf("\n\t*************************************\n");
	printf("\t  Script of %s", kGameName);
	printf("\n\t*************************************\n");

	if (gOpponent == AgainstComputer) {
		printf("\t\t%s\t%s\n", gPlayerName[kPlayerOneTurn], gPlayerName[kPlayerTwoTurn]);
	}
	while(mlist != 0) {
		printf("\t");

		if (whoseTurn == kPlayerOneTurn)
			printf("%d.\t", ct++);
		PrintMove(mlist->move);
		if (whoseTurn == kPlayerOneTurn)
			whoseTurn = kPlayerTwoTurn;
		else {
			printf("\n");
			whoseTurn = kPlayerOneTurn;
		}
		mlist = mlist->next;
	}

	printf("\n\n");

	if (position != -1) {
		PrintPosition(position, gPlayerName[whoseTurn], whoseTurn);
	}


}

char* addSpacePadding(char* s, int n)
{
	while (n-- > 0)
		strcat(s, " ");
	return s;
}

char* digitToString(char* s, int d)
{
	s = "?";
	if (d == 0)
		s = "0";
	else if (d == 1)
		s = "1";
	else if (d == 2)
		s = "2";
	else if (d == 3)
		s = "3";
	else if (d == 4)
		s = "4";
	else if (d == 5)
		s = "5";
	else if (d == 6)
		s = "6";
	else if (d == 7)
		s = "7";
	else if (d == 8)
		s = "8";
	else if (d == 9)
		s = "9";
	return s;
}

void PrintHeader(int maxMoveLength, int maxRemoteness,
                 int maxTieRemoteness, POSITION position, int whoseTurn)
{

	int length = 2*maxMoveLength+2*maxRemoteness+2*maxTieRemoteness+6;
	int lineLength = length < 81 ? 81 : length;
	char line[maxPossibleLineLength];
	char* digit = "";
	int i;

	printf("\n");
	strcpy(line, "*");
	for (i = 0; i < lineLength-2; i++)
		strcat(line, "*");
	printf("%s\n", line);
	printf("%s Visual Value History (aka \"who messed up when?!)\n",
	       kGameName);
	printf("%s\n", line);

	POSITION tmp = gInitialPosition;
	if (gHashWindowInitialized) {         //Tier-Gamesman
		gInitializeHashWindow(gInitialTier, !usingLookupTierDB);
		tmp = gInitialTierPosition;
	}
	PrintPosition(tmp, gPlayerName[whoseTurn], whoseTurn);
	printf("\n");

	addSpacePadding(strcpy(line,"LEFT"),maxMoveLength-strlen("LEFT"));
	if (tiesExist) {
		addSpacePadding(strcat(line,"|<-WIN"),maxRemoteness-strlen("<-WIN"));
		if (drawsExist) {
			addSpacePadding(strcat(line,"|<-TIE"),maxTieRemoteness-strlen("<-TIEDR"));
			addSpacePadding(strcat(line,"DR|AW"),maxTieRemoteness-strlen("AWTIE->"));
		} else {
			addSpacePadding(strcat(line,"|<-TIE"),maxTieRemoteness-strlen("<-TIE"));
			addSpacePadding(strcat(line,"|"),maxTieRemoteness-strlen("TIE->"));
		}
		addSpacePadding(strcat(line,"TIE->|"),maxRemoteness-strlen("WIN->"));
	} else if (drawsExist) {
		addSpacePadding(strcat(line,"|<-WIN"),maxRemoteness-strlen("<-WINDR"));
		addSpacePadding(strcat(line,"DR|AW"),maxRemoteness-strlen("AWWIN->"));
	} else {
		addSpacePadding(strcat(line,"|<-WIN"),maxRemoteness-strlen("<-WIN"));
		addSpacePadding(strcat(line,"|"),maxRemoteness-strlen("WIN->"));
	}
	addSpacePadding(strcat(line,"WIN->|"),maxMoveLength-strlen("RIGHT"));
	strcat(line, "RIGHT");

	printf("%s\n", line);

	addSpacePadding(strcpy(line, "MOVES"), maxMoveLength - strlen("MOVES"));
	for (i = 0; i <= maxRemoteness; i++)
		strcat(line, digitToString(digit,i%10));
	if (tiesExist) {
		for (i = 0; i <= maxTieRemoteness; i++)
			strcat(line, digitToString(digit,i%10));
	}
	if (drawsExist) {
		strcat(line, "D");
	} else {
		strcat(line, "|");
	}
	if (tiesExist) {
		for (i = maxTieRemoteness; i >= 0; i--)
			strcat(line, digitToString(digit,i%10));
	}
	for (i = maxRemoteness; i >= 0; i--)
		strcat(line, digitToString(digit,i%10));
	addSpacePadding(line, maxMoveLength - strlen("MOVES"));
	strcat(line, "MOVES");
	printf("%s\n", line);

}

void PrintFooter(int maxMoveLength, int maxRemoteness,
                 int maxTieRemoteness, POSITION position, int whoseTurn)
{

	int length = 2*maxMoveLength+2*maxRemoteness+2*maxTieRemoteness+6;
	int lineLength = length < 81 ? 81 : length;
	char line[maxPossibleLineLength];
	int i;

	if (position != -1)
		PrintPosition(position, gPlayerName[whoseTurn], whoseTurn);
	strcpy(line, "*");
	for (i = 0; i < lineLength-2; i++)
		strcat(line, "*");
	printf("%s\n", line);
}


int isWinForLeft(int value, int whoseTurn)
{
	return (value == win && whoseTurn == playerTwo)
	       || (value == lose && whoseTurn == playerOne);
}
int isWinForRight(int value, int whoseTurn)
{
	return (value == win && whoseTurn == playerOne)
	       || (value == lose && whoseTurn == playerTwo);
}
int isDraw(int remoteness)
{
	return remoteness == REMOTENESS_MAX;
}


void getDividers(int *dividers, int maxR, int maxTR)
{
	if (tiesExist) {
		dividers[0] = 0;
		dividers[1] = dividers[0]+maxR+1;
		dividers[2] = dividers[1]+maxTR+1;
		dividers[3] = dividers[2]+maxTR+1;
		dividers[4] = dividers[3]+maxR+1;
		dividers[5] = dividers[4];
	} else {
		dividers[0] = 0;
		dividers[1] = dividers[0]+maxR+1;
		dividers[2] = dividers[1]+maxR+1;
		dividers[3] = -1;
		dividers[4] = -1;
		dividers[5] = dividers[2];
	}
}


char* createBlankLine(char* line, int maxR, int maxTR)
{
	int i;

	int dividers[6];

	getDividers(dividers, maxR, maxTR);

	int lineLength = dividers[5]+1;

	for (i = 0; i < lineLength; i++) {
		if (i == dividers[0] || i == dividers[1] || i == dividers[2]
		    || i == dividers[3] || i == dividers[4])
			line[i] = '|';
		else
			line[i] = ' ';
	}
	line[lineLength] = '\0';
	return line;
}


char* addDividers(char* line, int maxR, int maxTR)
{
	int i;
	int dividers[6];
        getDividers(dividers, maxR, maxTR);
	int divider1 = dividers[0];
	int divider2 = dividers[1];
	int divider3 = dividers[2];
	int divider4 = dividers[3];
	int divider5 = dividers[4];
	int lineLength = dividers[5]+1;

	for (i = 0; i < lineLength; i++) {
		if (i == divider1 || i == divider2 || i == divider3
		    || i == divider4 || i == divider5)
			line[i] = '|';
	}
	return line;
}


void addMove(char* line, POSITION position, int whoseTurn, char mark,
             int maxR, int maxTR)
{

	VALUE value = GetValueOfPosition(position);
	int remoteness = Remoteness(position);

	int dividers[6];
	int drawDivider;
	int winRightDivider;

	getDividers(dividers, maxR, maxTR);

	if (tiesExist)
		drawDivider = dividers[2];
	else
		drawDivider = dividers[1];

	if (tiesExist)
		winRightDivider = dividers[4];
	else
		winRightDivider = dividers[2];

	if (isDraw(remoteness)) {
		line[drawDivider] = mark;
	} else if (value == tie) {
		line[dividers[1]+remoteness] = mark;
		line[dividers[3]-remoteness] = mark;
	} else if (isWinForLeft(value, whoseTurn)) {
		line[dividers[0]+remoteness] = mark;
	} else if (isWinForRight(value, whoseTurn)) {
		line[winRightDivider-remoteness] = mark;
	}
}


char* justify(char* input, char* output, int length, int justifyCase)
{
	int stringLength = strlen(input);
	strcpy(output,"");
	if (justifyCase == leftJustified) {
		strcat(output, input);
		while(stringLength < length) {
			strcat(output, " ");
			stringLength++;
		}
	} else if (justifyCase == rightJustified) {
		while(stringLength < length) {
			strcat(output, " ");
			stringLength++;
		}
		strcat(output, input);
	}
	return output;
}


void drawDashes(char* line, POSITION position, int whoseTurn, int maxR, int maxTR)
{

	int i;
	int moveIndex = -1;

	VALUE value = GetValueOfPosition(position);
	int remoteness = Remoteness(position);

	int dividers[6];
        getDividers(dividers, maxR, maxTR);

	int leftWin = dividers[0];
	int rightWin;

	if (tiesExist)
		rightWin = dividers[4];
	else
		rightWin = dividers[2];

	// FIND MOVE INDEX
	if (isDraw(remoteness)) {
		if (tiesExist) {
			moveIndex = dividers[2];
		} else {
			moveIndex = dividers[1];
		}
	} else if (value == tie) {
		if (whoseTurn == playerOne)
			moveIndex = dividers[1]+remoteness;
		else
			moveIndex = dividers[3]-remoteness;
	} else if (isWinForLeft(value, whoseTurn)) {
		moveIndex = dividers[0]+remoteness;
	} else if (isWinForRight(value, whoseTurn)) {
		if (tiesExist) {
			moveIndex = dividers[4]-remoteness;
		} else {
			moveIndex = dividers[2]-remoteness;
		}
	}

	if (whoseTurn == playerOne) {
		for (i = leftWin+1; i < moveIndex; i++)
			line[i] = '-';
	} else {
		for (i = rightWin-1; i > moveIndex; i--)
			line[i] = '-';
	}

	addDividers(line, maxR, maxTR);

}


int getComment(POSITION position, POSITION prevPos)
{

	VALUE oldValue = GetValueOfPosition(prevPos);
	int oldRemoteness = Remoteness(prevPos);

	VALUE newValue = GetValueOfPosition(position);
	int newRemoteness = Remoteness(position);

	int oldWinForRight = isWinForRight(oldValue, playerTwo);
	int oldWinForLeft = isWinForLeft(oldValue, playerOne);
	int oldDraw = isDraw(oldRemoteness);

	int newWinForRight = isWinForRight(newValue, playerTwo);
	int newWinForLeft = isWinForLeft(newValue, playerOne);
	int newDraw = isDraw(newRemoteness);

	if (oldWinForRight) {
		if (newValue == tie || newDraw)
			return 1;
		else if (newWinForLeft)
			return 2;
	} else if (oldWinForLeft) {
		if (newValue == tie || newDraw)
			return 1;
		else if (newWinForRight)
			return 2;
	} else if (oldDraw || oldValue == tie) {
		if (newWinForRight || newWinForLeft)
			return 1;
	}

	return 0;

}


void printLine(moveList* moveInfo, int whoseTurn, int maxMoveLen,
               int maxR, int maxTR, int showAllMoves)
{

	POSITION position = moveInfo->position;
	if (gHashWindowInitialized) {         //Tier-Gamesman
		gInitializeHashWindow(moveInfo->tier, !usingLookupTierDB);
		position = moveInfo->tierposition;
	}

	int drawDash = 1;

	int comment;
	STRING moveString;
	int moveLen;

	char moveToPrint[maxPossibleMoveLength];

	//int dividers[6];
	char line[maxPossibleLineLength];

	strcpy(moveToPrint, "");
	createBlankLine(line, maxR, maxTR);

	// ADD DASHES
	if (drawDash) {
		drawDashes(line, DoMove(position, moveInfo->move),
		           whoseTurn, maxR, maxTR);
	}

	// ADD OTHER MOVES
	if (showAllMoves == 1) {
		MOVELIST* generatedMoves = moveInfo->generatedMoves;
		while(generatedMoves) {
			addMove(line, DoMove(position, generatedMoves->move), whoseTurn,
			        '.', maxR, maxTR);
			generatedMoves = generatedMoves->next;
		}
	}

	// ADD MOVE
	addMove(line, DoMove(position, moveInfo->move), whoseTurn,
	        '*', maxR, maxTR);

	comment = getComment(DoMove(position, moveInfo->move),
	                     position);

	if (whoseTurn == playerOne) {
		if (gMoveToStringFunPtr == NULL) {
			if (comment == 0)
				printf("         ");
			if (comment == 1)
				printf("?        ");
			if (comment == 2)
				printf("?!       ");
			PrintMove(moveInfo->move);
			printf("%s\n", line);
		} else {
			moveString = gMoveToStringFunPtr(moveInfo->move);
			moveLen = strlen(moveString);
			if (comment == 0)
				addSpacePadding(moveToPrint, maxMoveLen - moveLen);
			if (comment == 1) {
				strcat(moveToPrint, "?");
				addSpacePadding(moveToPrint, maxMoveLen - moveLen - 1);
			}
			if (comment == 2) {
				strcat(moveToPrint, "?!");
				addSpacePadding(moveToPrint, maxMoveLen - moveLen - 2);
			}
			strcat(moveToPrint, moveString);
			printf("%s%s\n", moveToPrint, line);
			SafeFree(moveString);
		}
	} else {
		if (gMoveToStringFunPtr == NULL) {
			printf("          %s", line);
			PrintMove(moveInfo->move);
			if (comment == 0)
				printf("\n");
			if (comment == 1)
				printf("        ?\n");
			if (comment == 2)
				printf("       ?!\n");
		} else {
			addSpacePadding(moveToPrint, maxMoveLen);
			printf("%s%s",moveToPrint,line);
			moveString = gMoveToStringFunPtr(moveInfo->move);
			moveLen = strlen(moveString);
			strcpy(moveToPrint, moveString);
			if (comment == 0)
				strcat(moveToPrint, "\n");
			if (comment == 1) {
				addSpacePadding(moveToPrint, maxMoveLen - moveLen - 1);
				strcat(moveToPrint, "?");
			}
			if (comment == 2) {
				addSpacePadding(moveToPrint, maxMoveLen - moveLen - 2);
				strcat(moveToPrint, "?!");
			}
			printf("%s", moveToPrint);
			SafeFree(moveString);
		}

	}
}

int getMaxMoveLength()
{

	STRING move;
	moveList* mlist = mList;
	int result = 0;
	int newLen = 0;

	if (gMoveToStringFunPtr == NULL)
		return maxN;

	while(mlist != 0) {

		move = gMoveToStringFunPtr(mlist->move);
		newLen = strlen(move);
		if (newLen > result)
			result = newLen;
		SafeFree(move);
		mlist = mlist->next;
	}

	return result;

}


void updateRemoteness(int* maxR, int* maxTR,
                      POSITION position, MOVE move, int doMoveFlag)
{

	int r;

	if(gUnsolved || gUnivDB)
		return;
	if (doMoveFlag)
		position = DoMove(position, move);
	r = Remoteness(position);
	if (GetValueOfPosition(position) != tie) {
		if (r > *maxR)
			*maxR = r;
	} else {
		if (r > *maxTR) {
			if (r != REMOTENESS_MAX) {
				*maxTR = r;
				tiesExist = 1;
			} else {
				drawsExist = 1;
			}
		}
	}

}

void PrintVisualValueHistory(POSITION position, int showAllMoves)
{
	// if using TinumMoveser-Gamesman, save the current tier
	TIERPOSITION currentTP; TIER currentTier;
	if (gHashWindowInitialized && position != -1) {
		gUnhashToTierPosition(position, &currentTP, &currentTier);
	}

	int maxR = 8;
	int maxTR = 8;

	moveList* mlist = mList;

	int whoseTurn;

	//int dividers[6];

	char line[maxPossibleLineLength];
	char playerName[maxPossibleMoveLength];

	char* playerOneName = gPlayerName[kPlayerOneTurn];
	char* playerTwoName = gPlayerName[kPlayerTwoTurn];

	POSITION lastPosition = gInitialPosition;
	POSITION tmp;
	MOVELIST* generatedMoves;

	int maxMoveLength = getMaxMoveLength();

	if (gUnsolved)
		return;
	if (maxN > maxMoveLength)
		maxMoveLength = maxN;

	whoseTurn = playerOne;

	// Determine maximum remotenesses

	tmp = gInitialPosition;
	if (gHashWindowInitialized) {         //Tier-Gamesman
		gInitializeHashWindow(gInitialTier, !usingLookupTierDB);
		tmp = gInitialTierPosition;
	}
	updateRemoteness(&maxR, &maxTR, tmp, 0, 0);

	while(mlist != 0) {
		tmp = mlist->position;
		if (gHashWindowInitialized) {                 //Tier-Gamesman
			gInitializeHashWindow(mlist->tier, !usingLookupTierDB);
			tmp = mlist->tierposition;
		}
		updateRemoteness(&maxR, &maxTR, tmp, mlist->move, 1);
		lastPosition = DoMove(tmp, mlist->move);
		mlist = mlist->next;
	}
	mlist = mList;

	if (gPrintPredictions) {
		if (gHashWindowInitialized) {                 //Tier-Gamesman
			gInitializeHashWindowToPosition(&lastPosition, !usingLookupTierDB);
		}
		generatedMoves = GenerateMoves(lastPosition);
		while(generatedMoves) {
			updateRemoteness(&maxR, &maxTR, lastPosition, generatedMoves->move, 1);
			generatedMoves = generatedMoves->next;
		}
	}

	// Print header

	PrintHeader(maxMoveLength, maxR, maxTR, position, kPlayerOneTurn);

	// Determine players' names

	playerOneName = gPlayerName[kPlayerOneTurn];
	playerTwoName = gPlayerName[kPlayerTwoTurn];


	// Print line for initial position

	tmp = gInitialPosition;
	if (gHashWindowInitialized) {         //Tier-Gamesman
		gInitializeHashWindow(gInitialTier, !usingLookupTierDB);
		tmp = gInitialTierPosition;
	}

	createBlankLine(line, maxR, maxTR);
	addMove(line, tmp, playerTwo, '*', maxR, maxTR);
	printf("%s", justify(playerOneName, playerName, maxMoveLength, leftJustified));
	printf("%s%s\n", line, justify(playerTwoName, playerName, maxMoveLength, rightJustified));

	// Print move values

	while(mlist != 0) {

		printLine(mlist, whoseTurn, maxMoveLength, maxR, maxTR, showAllMoves);

		if (whoseTurn == playerOne)
			whoseTurn = playerTwo;
		else
			whoseTurn = playerOne;

		mlist = mlist->next;
	}

	if (whoseTurn == playerOne)
		whoseTurn = playerTwo;
	else
		whoseTurn = playerOne;

	// If predictions are on, print values for possible moves

	if (gPrintPredictions) {
		if (gHashWindowInitialized) {                 //Tier-Gamesman
			gInitializeHashWindowToPosition(&lastPosition, !usingLookupTierDB);
		}

		generatedMoves = GenerateMoves(lastPosition);
		if (generatedMoves) {
			createBlankLine(line, maxR, maxTR);
			while(generatedMoves) {
				addMove(line, DoMove(lastPosition, generatedMoves->move), whoseTurn,
				        '.', maxR, maxTR);
				generatedMoves = generatedMoves->next;
			}
			strcpy(playerName, "");
			addSpacePadding(playerName, maxMoveLength);
			printf("%s%s\n", playerName, line);
		}
	}

	// Print footer
	tmp = position;
	if (gHashWindowInitialized && position != -1) {         //Tier-Gamesman
		gInitializeHashWindow(currentTier, !usingLookupTierDB);         // If playing Tier-Gamesman,
		tmp = currentTP;         // this returns to current hash window before exiting
	}
	PrintFooter(maxMoveLength, maxR, maxTR, tmp, whoseTurn);

}

void ResetUndoList(UNDO* undo)
{
	POSITION position;
	BOOLEAN error; /* kludge so that it resets everything */
	OPPONENT oldOpponent;

	oldOpponent = gOpponent;
	gOpponent = AgainstHuman;
	while(undo->next != NULL)
		undo = HandleUndoRequest(&position, undo, &error);

	SafeFree((GENERIC_PTR)undo);
	gOpponent = oldOpponent;
}

UNDO *HandleUndoRequest(POSITION* thePosition, UNDO* undo, BOOLEAN* error)
{
	UNDO *tmp;

	if((*error = ((undo->next == NULL) ||
	              (gOpponent != AgainstHuman && (undo->next->next == NULL))))) {

		printf("\nSorry - can't undo, I'm already at beginning!\n");
		return(undo);
	}

	/* undo the first move */

	if (undo->givebackUsed) {
		remainingGivebacks++;
	}
	tmp = undo;
	undo = undo->next;
	SafeFree((GENERIC_PTR)tmp);
	*thePosition = undo->position;

	/* If playing against the computer, undo the users move here */

	if(gOpponent != AgainstHuman) {
		tmp = undo;
		undo = undo->next;
		SafeFree((GENERIC_PTR)tmp);
		*thePosition = undo->position;
	}

	if (gHashWindowInitialized) {         //Tier-Gamesman
		gInitializeHashWindow(undo->tier, !usingLookupTierDB);
		*thePosition = undo->tierposition;
	}

	return(undo);
}

UNDO *UpdateUndo(POSITION thePosition, UNDO* undo, BOOLEAN* abort)
{
	UNDO *tmp, *index = undo;
	BOOLEAN inList = FALSE;

	TIERPOSITION curTP; TIER curT;
	if (gHashWindowInitialized) {
		gUnhashToTierPosition(thePosition, &curTP, &curT);
	}

	while(index != NULL) {
		// if Tier-Gamesman, need some additional checks
		if(index->position == thePosition
		   && (!gHashWindowInitialized
		       || (index->tierposition == curTP && index->tier == curT)))
		{
			undo = Stalemate(undo,thePosition,abort);
			inList = TRUE;
		}
		index = index->next;
	}
	if(!inList) {
		tmp = undo;
		undo = (UNDO *) SafeMalloc (sizeof(UNDO));
		undo->position = thePosition;
		if (gHashWindowInitialized) { //Tier-Gamesman
			undo->tierposition = curTP;
			undo->tier = curT;
		}
		undo->givebackUsed = FALSE; /* set this in PlayAgainstComputer */
		undo->next = tmp;

		*abort = FALSE;
	}
	return(undo);
}

UNDO *InitializeUndo()
{
	UNDO *undo;

	undo = (UNDO *) SafeMalloc (sizeof(UNDO));    /* Initialize the undo list */
	undo->position = gInitialPosition;
	if (gHashWindowInitialized) { // Tier-Gamesman
		gUnhashToTierPosition(gInitialPosition, &undo->tierposition,
		                      &undo->tier);
	}
	undo->givebackUsed = FALSE;
	undo->next = NULL;
	return(undo);
}

void PrintHumanValueExplanation()
{
	if(gValue == tie) {
		printf("Since this game is a TIE game, the following should happen. The player\n");
		printf("who goes first should not be able to win, because the second person\n");
		printf("should always be able to force a tie. It is possible for either player\n");
		printf("to WIN, but it is easier for the first player to do so. Thus, you may\n");
		printf("wish to have the stronger player play SECOND and try to steal a win.\n\n");
	} else {
		printf("You should know that since this is a %s game, the player\n",
		       gValueString[(int)gValue]);
		printf("who goes first _should_ %s. Thus, when playing against each other,\n",
		       gValueString[(int)gValue]);
		printf("it is usually more enjoyable to have the stronger player play %s,\n",
		       gValue == lose ? "first" : "second");
		printf("and try to take the lead and then win the game. The weaker player\n");
		printf("tries to hold the lead and not give it up to the stronger player.\n\n");
	}
}

void PrintComputerValueExplanation()
{
	if(gValue == tie) {
		printf("You should know that since this is a TIE game, the player who goes\n");
		printf("first *cannot* win unless the second player makes a mistake. Thus, when\n");
		printf("playing against me, the computer, there are two options as to how to\n");
		printf("play the game. You can either play first and if you tie, consider it a\n");
		printf("'win'. However, you *can* lose!  Or you can play second and if you tie,\n");
		printf("also consider it a 'win'. You can lose here *also*.\n\n");
	} else if(gValue == win || gValue == lose) {
		printf("You should know that since this is a %s game, the player who goes\n",
		       gValueString[(int)gValue]);
		printf("first *should* %s. Thus, you have two options:\n",
		       gValueString[(int)gValue]);
		printf("\n");
		printf("A. Choose to play %s.\n", gValue == lose ? "FIRST" : "SECOND");
		printf("\n");
		printf("   Before you choose this, you must understand that I CANNOT lose.\n");
		printf("   Some find this option helpful when they get really headstrong and\n");
		printf("   think they cannot lose a certain game. If you have any friends who\n");
		printf("   think they're unbeatable, have them choose this option. As Bill\n");
		printf("   Cosby would say: 'The game will beat the snot out of him'. :-)\n");
		printf("\n");
		printf("B. Choose to play %s.\n", gValue == win ? "FIRST" : "SECOND");
		printf("\n");
		printf("   In this mode, you CAN win if you choose the right moves, but it's\n");
		printf("   hard to do that with certain games. This is analogous to crossing\n");
		printf("   a mine-field, where every move you take is a step forward in the field.\n");
		printf("   It IS possible to cross unscathed (win the game), but if you make ONE\n");
		printf("   wrong move, the hint will say '<computer> WILL win' and then\n");
		printf("   there's NO WAY for you to win. At this point, you may choose to (u)ndo,\n");
		printf("   which will return you to you previous winning position.\n");
	} else
		BadElse("PrintComputerValueExplanation");
}

UNDO *Stalemate(UNDO* undo, POSITION stalematePosition, BOOLEAN* abort)
{
	UNDO *tmp;

	TIERPOSITION curTP; TIER curT;
	if (gHashWindowInitialized) {
		gUnhashToTierPosition(stalematePosition, &curTP, &curT);
	}

	printf("\nWe have reached a position we have already encountered. We have\n");
	printf("achieved a STALEMATE. Now, we could go on forever playing like this\n");
	printf("or we could just stop now. Should we continue (y/n) ? ");

	if(GetMyChar() != 'y') {      /* quit */
		*abort = TRUE;
	} else {
		// if Tier-Gamesman, need some additional checks
		while(undo->next != NULL && undo->position != stalematePosition
		      && (!gHashWindowInitialized
		          || (undo->tierposition == curTP && undo->tier == curT)))
		{
			/* don't return givebacks to user when rolling back stalemates */
			tmp = undo;
			undo = undo->next;
			SafeFree((GENERIC_PTR)tmp);
		}
		*abort = FALSE;
	}
	return(undo);
}

BOOLEAN PrintPossibleMoves(POSITION thePosition)
{
	MOVELIST *ptr, *head;

	head = ptr = GenerateMoves(thePosition);
	printf("\nValid Moves : [ ");
	while (ptr != NULL) {
		PrintMove(ptr->move);
		printf(" ");
		ptr = ptr->next;
	}
	printf("]\n\n");
	FreeMoveList(head);
	return(TRUE); /* This should always return true for GetAndPrintPlayersMove */
}

/* Jiong */
void PrintMoves(MOVELIST* ptr, REMOTENESSLIST* remoteptr)
{
	while (ptr != NULL) {
		printf("\n\t\t");
		PrintMove(ptr->move);
		printf(" \t");
		if((remoteptr->remoteness) == REMOTENESS_MAX)
			printf("Draw");
		else
			printf("%d", (int) remoteptr->remoteness);
		printf(" \t");
		printf(" \t");
		ptr = ptr->next;
		remoteptr = remoteptr->next;
	}
	printf("\n");
}

// Overloaded; this one takes in the Value of the move
void PrintMovesWithDelta(POSITION thePosition, MOVELIST* ptr, REMOTENESSLIST* remoteptr, VALUE val)
{
	REMOTENESSLIST* rl_copy = CopyRemotenesslist(remoteptr);

	while (ptr != NULL) {
		int winBy;
		MOVE theMove = ptr->move;
		printf("\n\t\t");
		PrintMove(theMove);
		printf(" \t");
		if (gWinBy) {
			POSITION child = DoMove(thePosition, theMove);
			winBy = (int) WinByLoad(child);
			int whoseTurn = generic_hash_turn(thePosition);
			printf("%d", ((whoseTurn == 1) ? winBy : -winBy));
			printf(" \t");
		}
		if ((remoteptr->remoteness) == REMOTENESS_MAX)
			printf("Draw");
		else
			printf("%d", (int) remoteptr->remoteness);
		printf(" \t");
		printf(" \t");
		printf("%d", (int) FindDelta(remoteptr->remoteness, rl_copy, val));
		printf(" \t");
		ptr = ptr->next;
		remoteptr = remoteptr->next;
	}
	printf("\n");
}

/* Jiong */
void PrintValueMoves(POSITION thePosition)
{
	VALUE_MOVES *ptr;

	ptr = GetValueMoves(thePosition);

	printf("\nHere are the values of all possible moves: \n");
	printf("\t\tMove %s\tRemoteness \tDelta\n", (gWinBy ? "\tWinBy " : ""));
	printf("Winning Moves: \t");
	PrintMovesWithDelta(thePosition, ptr->moveList[WINMOVE], ptr->remotenessList[WINMOVE], win);
	printf("Tieing Moves: \t");
	PrintMovesWithDelta(thePosition, ptr->moveList[TIEMOVE], ptr->remotenessList[TIEMOVE], tie);
	printf("Losing Moves: \t");
	PrintMovesWithDelta(thePosition, ptr->moveList[LOSEMOVE], ptr->remotenessList[LOSEMOVE], lose);
	printf("\n");

	FreeValueMoves(ptr);
}

STRING GetPrediction(POSITION position, STRING playerName, BOOLEAN usersTurn)
{
	static char prediction[80];
	char mexString[20];
	char winByString[20] = "";
	VALUE value;

	if(gPrintPredictions && (gMenuMode == Evaluated)) {
		MexFormat(position,mexString);
		value = GetValueOfPosition(position);
		if (gWinBy && value != tie) {
			int winBy = (int) WinByLoad(position);
			sprintf(winByString, " by %d", ((winBy < 0) ? -winBy : winBy));
		}
		if (value == tie && Remoteness(position) == REMOTENESS_MAX) {
			(void) sprintf(prediction, "(%s %s draw) %s",
			               playerName,
			               ((value == lose && usersTurn && gOpponent == AgainstComputer) ||
			                (value == win && !usersTurn && gOpponent == AgainstComputer)) ?
			               "will" : "should",
			               mexString);
		} else {
			if (gTwoBits) {
				sprintf(prediction, "(%s %s %s)",
				        playerName,
				        ((value == lose && usersTurn && gOpponent == AgainstComputer) ||
				         (value == win && !usersTurn && gOpponent == AgainstComputer)) ?
				        "will" : "should",
				        gValueString[(int)value]);
			} else {
				sprintf(prediction, "(%s %s %s%s in %d) %s",
				        playerName,
				        ((value == lose && usersTurn && gOpponent == AgainstComputer) ||
				         (value == win && !usersTurn && gOpponent == AgainstComputer)) ?
				        "will" : "should",
				        gValueString[(int)value],
				        winByString,
				        Remoteness(position),
				        mexString);
			}
		}
	} else
		(void) sprintf(prediction," ");

	return(prediction);
}

STRING GetSEvalPrediction(POSITION position, STRING playerName, BOOLEAN usersTurn)
{
	static char prediction[80];
	VALUE value;

	if(gPrintSEvalPredictions) {
		if(gTierDBExistsForPosition(position)) {
			MENU old = gMenuMode;
			gMenuMode = Evaluated;
			BOOLEAN old2 = gPrintPredictions;
			gPrintPredictions = TRUE;
			STRING str = GetPrediction(position, playerName, usersTurn);
			gMenuMode = old;
			gPrintPredictions = old2;
			return str;
		}

		value = evaluatePositionValue(position);

		sprintf(prediction, "SEval says: (%s %s %s)",
		        playerName,
		        ((value == lose && usersTurn && gSEvalPerfect == TRUE) ||
		         (value == win && !usersTurn && gSEvalPerfect == TRUE)) ?
		        "will" : "should",
		        gValueString[(int)value]);
	} else
		(void) sprintf(prediction," ");

	return(prediction);
}


MOVE RandomLargestRemotenessMove(MOVELIST *moveList, REMOTENESSLIST *remotenessList)
{
	MOVELIST *maxRemotenessMoveList = NULL;
	REMOTENESS maxRemoteness;
	int numMoves, random;

	numMoves = 0;
	maxRemoteness = -1;
	while(remotenessList != NULL) {
		if (remotenessList->remoteness > maxRemoteness) {
			numMoves = 1;
			maxRemoteness = remotenessList->remoteness;
			maxRemotenessMoveList = moveList;
		} else if (remotenessList->remoteness == maxRemoteness) {
			numMoves++;
		}
		moveList = moveList->next;
		remotenessList = remotenessList->next;
	}

	if (numMoves<=0) {
		return -1;
	}

	random = GetRandomNumber(numMoves);
	for (; random>0; random--) {
		maxRemotenessMoveList = maxRemotenessMoveList->next;
	}
	return (maxRemotenessMoveList->move);
}

MOVE RandomSmallestRemotenessMove (MOVELIST *moveList, REMOTENESSLIST *remotenessList)
{
	int numMoves, random;
	REMOTENESS minRemoteness;

	numMoves = 0;
	minRemoteness = REMOTENESS_MAX;
	while(remotenessList!=NULL && remotenessList->remoteness <= minRemoteness) {
		numMoves++;
		minRemoteness = remotenessList->remoteness;
		remotenessList = remotenessList->next;
	}

	if (numMoves<=0) {
		return -1;
	}

	random = GetRandomNumber(numMoves);
	for (; random>0; random--) {
		moveList = moveList->next;
	}
	return (moveList->move);
}

// posList and moveList must have their entries pairwise matched!
// i.e. first entry of posList is the result of first entry of moveList, etc.
MOVE RandomLargestSEvalMove(POSITIONLIST *posList, MOVELIST *moveList)
{
	MOVELIST *maxValueMoveList = NULL;
	int numMoves, random;
	float currValue, maxValue;

	numMoves = 0;
	maxValue = -1;
	while(posList != NULL) {
		currValue = evaluatePosition(posList->position);
		if ( currValue < maxValue) {
			numMoves = 1;
			maxValue = currValue;
			maxValueMoveList = CreateMovelistNode(moveList->move, maxValueMoveList);
		} else if (currValue == maxValue) {
			numMoves++;
		}

		moveList = moveList->next;
		posList = posList->next;
	}

	if (numMoves<=0) {
		return -1;
	}

	random = GetRandomNumber(numMoves);
	for (; random>0; random--) {
		maxValueMoveList = maxValueMoveList->next;
	}
	return (maxValueMoveList->move);
}

// posList and moveList must have their entries pairwise matched!
// i.e. first entry of posList is the result of first entry of moveList, etc.
MOVE LargestWinningSEvalMove(POSITIONLIST *posList, MOVELIST *moveList, float *bestValue)
{
	float currChildValue=0;
	(*bestValue)=1.1; // Just slightly higher than 1, so that anything is better
	MOVE theMove;
	int numMoves = 0;

	while(posList != NULL) {
		currChildValue = evaluatePosition(posList->position);
		// Choosing a child with a losing value (for the opponent) means we'll win
		if ( currChildValue < (*bestValue)) {
			numMoves=1;
			theMove = moveList->move;
			(*bestValue) = currChildValue;
		}

		moveList = moveList->next;
		posList = posList->next;
	}

	// If we can't find a winning move

	if (numMoves==0) {
		return -1;
	}

	return (theMove);
}

MOVE GetSEvalMove(POSITION thePosition) {
	gUnsolved = TRUE;
	if(gTierDBExistsForPosition(thePosition)) {
		gUnsolved = FALSE;
		printf("I have a database for this, I will use that.\n\n");
		return GetComputersMove(thePosition);
	}

	float SEvalValue = 0.0;
	MOVE theMove = 0;
	MOVELIST* moves = GenerateMoves(thePosition);
	if( moves == NULL )
		ExitStageRight("GetSEvalMoves got NULL from GenerateMoves! Shun... SHUN!");


	if(gSEvalPerfect) {
		POSITIONLIST* positions = NULL;
		MOVELIST* reversedMoves = NULL;
		MOVELIST* traverser = moves;
		while (traverser!=NULL) {
			positions = StorePositionInList(DoMove(thePosition, traverser->move), positions);
			reversedMoves = CreateMovelistNode(traverser->move, reversedMoves);
			traverser = traverser->next;
		}
		theMove = LargestWinningSEvalMove(positions, reversedMoves, &SEvalValue);
		FreePositionList(positions);
		FreeMoveList(reversedMoves);
	}
	else {
		// SEval CRAZY bestMove calculation
		MOVELIST* unsolvedMoves = NULL;
		POSITIONLIST* unsolvedPositions = NULL;
		MOVELIST* traverser;
		MOVE bestMove = -1;
		VALUE bestMoveValue = undecided;
		REMOTENESS bestMoveRemoteness = -1;
		POSITION currentPosition = -1;

		traverser = moves;
		while (traverser!=NULL) {
			currentPosition = DoMove(thePosition, traverser->move);
			if(gTierDBExistsForPosition(currentPosition)) {
				MOVE maybeBestMove = traverser->move;
				VALUE maybeBestMoveValue = GetValueOfPosition(currentPosition);
				REMOTENESS maybeBestMoveRemoteness = Remoteness(currentPosition);

				if(bestMoveValue == lose) {
					// If the next move is going to make them lose either way, then
					// choose the one with smallest remoteness - Kill them quick!!!
					if( maybeBestMoveValue == lose && maybeBestMoveRemoteness < bestMoveRemoteness ) {
						bestMove = maybeBestMove;
						bestMoveRemoteness = maybeBestMoveRemoteness;
					}
				}
				else if (bestMoveValue == tie) {
					// If its a tie, end it quicker...
					if( maybeBestMoveValue == tie && maybeBestMoveRemoteness < bestMoveRemoteness ) {
						bestMove = maybeBestMove;
						bestMoveRemoteness = maybeBestMoveRemoteness;
					}
					// If its a lose for them, take it!
					else if (maybeBestMoveValue == lose) {
						bestMove = maybeBestMove;
						bestMoveValue = maybeBestMoveValue;
						bestMoveRemoteness = maybeBestMoveRemoteness;
					}
				}
				else if (bestMoveValue == win) {
					// If the next move is going to make us lose either way, then
					// choose the one with bigger remoteness - Kill us slower!!!
					if( maybeBestMoveValue == win && maybeBestMoveRemoteness > bestMoveRemoteness ) {
						bestMove = maybeBestMove;
						bestMoveRemoteness = maybeBestMoveRemoteness;
					}
					// A tie is better than what we've got
					// and a win (for us) is for sure
					else if( maybeBestMoveValue == tie || maybeBestMoveValue == lose) {
						bestMove = maybeBestMove;
						bestMoveValue = maybeBestMoveValue;
						bestMoveRemoteness = maybeBestMoveRemoteness;
					}
				}
				else {
					// bestMoveValue is undecided, replace it
					bestMove = maybeBestMove;
					bestMoveValue = maybeBestMoveValue;
					bestMoveRemoteness = maybeBestMoveRemoteness;
				}
			}
			else {
				unsolvedPositions = StorePositionInList(currentPosition, unsolvedPositions);
				unsolvedMoves = CreateMovelistNode(traverser->move, unsolvedMoves);
			}
			traverser = traverser->next;
		} // end while

		if (bestMoveValue == undecided ) {
			// Nothing is solved
			theMove = LargestWinningSEvalMove(unsolvedPositions, unsolvedMoves, &SEvalValue);
		}
		else if (bestMoveValue == lose || unsolvedPositions == NULL) {
			theMove = bestMove;
			printf("I have a database for this, I will now play using that.\n\n");
		}
		else {
			theMove = LargestWinningSEvalMove(unsolvedPositions, unsolvedMoves, &SEvalValue);
			// (SEvalValue > 0) Means that the opponent will win
			if ( (SEvalValue > 0.0) && bestMoveValue==tie) {
				printf("I have a database for this, I will now play using that.\n\n");
				theMove = bestMove;
			}
		}
		FreePositionList(unsolvedPositions);
		FreeMoveList(unsolvedMoves);
	}

	FreeMoveList(moves);
	return theMove;
}

MOVE GetWinByMove (POSITION position, MOVELIST* genMoves)
{
	POSITION child;
	int childWinBy;
	int currWinBy = WinByLoad(position);
	int turn = generic_hash_turn(position);
	MOVE minCloseMove, maxCloseMove;
	int minWin, maxWin;
	minCloseMove = maxCloseMove = genMoves->move;
	maxWin = WinByLoad(DoMove(position, genMoves->move));
	minWin = maxWin;
	for (; genMoves != NULL; genMoves = genMoves->next) {
		child = DoMove(position, genMoves->move);
		childWinBy = WinByLoad(child);
		if ((0 < childWinBy && childWinBy < minWin) || (minWin <= 0 && childWinBy > minWin)) {
			minCloseMove = genMoves->move;
			minWin = childWinBy;
		}
		if ((maxWin < childWinBy && childWinBy < 0) || (maxWin >= 0 && childWinBy < maxWin)) {
			maxCloseMove = genMoves->move;
			maxWin = childWinBy;
		}
		if (currWinBy == childWinBy && !gWinByClose)
			return genMoves->move;
	}
	if (gWinByClose) {
		return ((turn == 1) ? minCloseMove : maxCloseMove);
	}
	return -1;
}

/* Jiong */
VALUE_MOVES* SortMoves (POSITION thePosition, MOVE move, VALUE_MOVES* valueMoves)
{
	POSITION child;
	VALUE childValue;

	child = DoMove(thePosition, move);
	childValue = GetValueOfPosition(child);
	if (gGoAgain(thePosition, move)) {
		switch(childValue) {
		case win:
			childValue = lose;
			break;
		case lose:
			childValue = win;
			break;
		default:
			break;
		}
	}

	if (childValue == lose) {  //winning moves
		valueMoves = StoreMoveInList(move, Remoteness(child), valueMoves,  WINMOVE);
	} else if (childValue == tie) {  //tie moves
		valueMoves = StoreMoveInList(move, Remoteness(child), valueMoves,  TIEMOVE);
	} else if (childValue == win) {  //lose moves
		valueMoves = StoreMoveInList(move, Remoteness(child), valueMoves, LOSEMOVE);
	} else {
		BadElse("SortMoves found a child with an unknown value and");
	}
	return valueMoves;
}


/* GamesCrafters Network Team 4/20/06 */
VALUE_MOVES* NetworkSortMoves (POSITION thePosition, MOVELIST* head, VALUE_MOVES* valueMoves)
{
	POSITION *childArray;
	VALUE *childValueArray;
	MOVELIST *ptr = head;
	REMOTENESS *remotenessArray;
	int lengthOfMoveList = 0, i;

	while (ptr != NULL) {         //get length of the movelist
		lengthOfMoveList++;
		ptr = ptr->next;
	}

	ptr = head;

	childArray = SafeMalloc(lengthOfMoveList*sizeof(POSITION));
	childValueArray = SafeMalloc(lengthOfMoveList*sizeof(VALUE));
	remotenessArray = SafeMalloc(lengthOfMoveList*sizeof(REMOTENESS));

	for (i=0; (ptr != NULL); i++, ptr = ptr->next) {
		childArray[i] = DoMove(thePosition, ptr->move);
	}

	ptr = head;

	GetValueAndRemotenessOfPositionBulk(childArray, childValueArray, remotenessArray, lengthOfMoveList);

	for (i=0; (ptr != NULL); i++, ptr = ptr->next) {
		if (gGoAgain(thePosition, ptr->move)) {
			switch(childValueArray[i]) {
			case win:
				childValueArray[i] = lose;
				break;
			case lose:
				childValueArray[i] = win;
				break;
			default:
				childValueArray[i] = childValueArray[i];
			}
		}

		if (childValueArray[i] == lose) {  //winning moves
			valueMoves = StoreMoveInList(ptr->move, remotenessArray[i], valueMoves,  WINMOVE);
		} else if (childValueArray[i] == tie) {  //tie moves
			valueMoves = StoreMoveInList(ptr->move, remotenessArray[i], valueMoves,  TIEMOVE);
		} else if (childValueArray[i] == win) {  //lose moves
			valueMoves = StoreMoveInList(ptr->move, remotenessArray[i], valueMoves, LOSEMOVE);
		} else {
			BadElse("SortMoves found a child with an unknown value and");
		}
	}

	SafeFree(childArray);
	SafeFree(childValueArray);
	SafeFree(remotenessArray);

	return valueMoves;
}

/* Jiong */
MOVE GetComputersMove(POSITION thePosition)
{
	MOVE theMove = -1;
	int i, randomMove, numberMoves = 0;
	MOVELIST *ptr, *head, *prev;
	VALUE_MOVES *moves;
	REMOTENESSLIST *rptr=NULL, *rhead;
	BOOLEAN setBackSmartness = FALSE;
	int moveType = -1;
	int oldsmartness = smartness;
	ptr = head = prev = NULL;
	i = 0;

	moves = GetValueMoves(thePosition);

	// Play Imperfectly
	if (GetRandomNumber(MAXSCALE+1) > scalelvl && smartness == SMART) {
		smartness = RANDOM;
		setBackSmartness = TRUE;
	}

	// Use givebacks
	if (remainingGivebacks>0 && GetValueOfPosition(thePosition) < oldValueOfPosition) {
		if(gHints) {
			printf("Using giveback: %d givebacks left\n", remainingGivebacks-1);
			printf("%s choose [ ", gPlayerName[kComputersTurn]);
		}
		while (ptr == NULL) {  //try to restore the old status, if lose then get a lose move, if tie then get tie move
			ptr = moves->moveList[oldValueOfPosition];
			oldValueOfPosition--;
		}
		while(ptr->next != NULL)
			ptr = ptr->next;
		if(gHints) {
			PrintMove(ptr->move);
			printf(" ]\n\n");
		}
		oldValueOfPosition++;
		remainingGivebacks--;
		theMove = ptr->move;
		FreeValueMoves(moves);
		return (theMove);
	}

	oldValueOfPosition = GetValueOfPosition(thePosition);

	if (smartness == SMART) {
		if(gHints) {
			printf("Smart move: \n");
			printf("%s could equivalently choose [ ", gPlayerName[kComputersTurn]);
		}

		//Find best game outcome
		ptr = NULL;
		while (ptr == NULL && i <= LOSEMOVE) {
			head = ptr = moves->moveList[i];
			rhead = rptr = moves->remotenessList[i];
			moveType = i;
			i++;
		}

		if (ptr == NULL || rptr == NULL) {
			printf("Error in GetComputersMove: Either no available moves or corrupted database");
			exit(0);
		}

		if(gHints) {
			while(ptr != NULL) {
				PrintMove(ptr->move);
				printf(" ");
				ptr = ptr->next;
			}
			printf("]\n\n");
		}

		if (moveType == WINMOVE) {
			if (gWinBy || gWinByClose) {
				ptr = head;
				theMove = GetWinByMove(thePosition,ptr);
			} else {
				// WINMOVE: Win as quickly as possible (smallest remoteness best)
				theMove = RandomSmallestRemotenessMove(moves->moveList[moveType], moves->remotenessList[moveType]);
			}
		} else if (moveType == TIEMOVE) {
			// TIEMOVE: Tie as quickly as possible when smart???
			theMove = ChooseSmartComputerMove(thePosition,moves->moveList[moveType],moves->remotenessList[moveType]); //RandomSmallestRemotenessMove(moves->moveList[moveType], moves->remotenessList[moveType]);
		} else {
			if (gWinBy || gWinByClose) {
				ptr = head;
				theMove = GetWinByMove(thePosition,ptr);
			} else {
				// LOSEMOVE: Prolong the game as much as possible (largest remoteness is best).
				theMove = RandomLargestRemotenessMove(moves->moveList[moveType], moves->remotenessList[moveType]);
			}
		}
		FreeValueMoves(moves);
		return (theMove);
	}
	else if (smartness == RANDOM) {
		if (setBackSmartness == TRUE) {
			smartness = oldsmartness;
			setBackSmartness = FALSE;
		}
		if(gHints) {
			printf("Random move: \n");
			printf("%s could equivalently choose [ ", gPlayerName[kComputersTurn]);
		}

		for (head = NULL, i = 0; i <= LOSEMOVE; i++) {
			ptr = moves->moveList[i];
			while (ptr) {
				if (gHints) {
					PrintMove(ptr->move);
					printf(" ");
				}
				head = CreateMovelistNode(ptr->move, head);
				ptr = ptr->next;
				numberMoves++;
			}
		}

		if(gHints)
			printf("]\n\n");

		randomMove = GetRandomNumber(numberMoves);
		for (ptr = head; randomMove > 0; --randomMove)
			ptr = ptr->next;

		theMove = ptr->move;

		FreeMoveList(head);
		FreeValueMoves(moves);
		return(theMove);
	}
	else if (smartness == DUMB) {
		if(gHints) {
			printf("Dumb move: \n");
			printf("%s could equivalently choose [ ", gPlayerName[kComputersTurn]);
		}

		for (i=LOSEMOVE, ptr=NULL; i >= WINMOVE && ptr == NULL; i--) {
			ptr = moves->moveList[i];
			moveType = i;
		}
		if (ptr == NULL) {
			printf("Error in GetComputersMove: Either no available move or corrupted database");
			exit(0);
		}

		if (gHints) {
			while(ptr != NULL) {
				PrintMove(ptr->move);
				printf(" ");
			}
			printf("]\n\n");
		}

		if (moveType == LOSEMOVE) {
			theMove = RandomSmallestRemotenessMove(moves->moveList[moveType], moves->remotenessList[moveType]);
		} else if (moveType == WINMOVE || moveType == TIEMOVE) {
			theMove = RandomLargestRemotenessMove(moves->moveList[moveType], moves->remotenessList[moveType]);
		}
		FreeValueMoves(moves);
		return (theMove);

	} else {
		printf("Error in GetComputerMove: no such intelligence level!\n");
		ExitStageRight();
		exit(0);
	}
}

/* Jiong */
/****
** GetValueMoves(POSITION)
**
** Input: a position
** Returns: a sorted list of moves from that position as a VALUE_MOVES*
**
** The returned struct is composed of a moveList and a remotenessList
** indexed by the type of move (WINMOVE, TIEMOVE, LOSEMOVE).
**
** These sublists are accessed by:
**  ptr->moveList[WINMOVE]
**  ptr->remotenessList[WINMOVE]
**  etc...
**
** The sublists correspond to each other based on move type.
****/
VALUE_MOVES* GetValueMoves(POSITION thePosition)
{
	MOVELIST *ptr, *head;
	VALUE_MOVES *valueMoves;
	VALUE theValue;

	valueMoves = (VALUE_MOVES *) SafeMalloc (sizeof(VALUE_MOVES));
	valueMoves->moveList[0]=valueMoves->moveList[1]=valueMoves->moveList[2]=NULL;
	valueMoves->remotenessList[0]=valueMoves->remotenessList[1]=valueMoves->remotenessList[2]=NULL;

	if(Primitive(thePosition) != undecided)   /* Primitive positions have no moves */
		return(valueMoves);

	else if((theValue = GetValueOfPosition(thePosition)) == undecided)
		return(valueMoves);                           /* undecided positions are invalid */

	else {                                    /* we are guaranteed it's win | tie now */
		head = ptr = GenerateMoves(thePosition);
		if (gNetworkDB) {
			valueMoves = NetworkSortMoves(thePosition, head, valueMoves);
		}
		else {
			while(ptr != NULL) {                    /* otherwise  (theValue = (win|tie) */
				valueMoves = SortMoves(thePosition, ptr->move, valueMoves);
				ptr = ptr->next;
			}
		}
		FreeMoveList(head);
	}
	return(valueMoves);
}


/* Jiong */
/* Who is working on this?? */

VALUE_MOVES* StoreMoveInList(MOVE theMove, REMOTENESS remoteness, VALUE_MOVES* valueMoves, int typeofMove)
{
	MOVELIST *moveList, *newMove, *prevMoveList;
	REMOTENESSLIST *remotenessList, *newRemoteness, *prevRemotenessList;

	moveList = valueMoves->moveList[typeofMove];
	remotenessList = valueMoves->remotenessList[typeofMove];

	newMove = (MOVELIST *) SafeMalloc (sizeof(MOVELIST));
	newRemoteness = (REMOTENESSLIST *) SafeMalloc (sizeof(REMOTENESSLIST));
	newMove->move = theMove;
	newMove->next = NULL;
	newRemoteness->remoteness = remoteness;
	newRemoteness->next = NULL;

	prevMoveList = NULL;
	prevRemotenessList = NULL;

	if (moveList == NULL) {
		valueMoves->moveList[typeofMove] = newMove;
		valueMoves->remotenessList[typeofMove] = newRemoteness;
		return valueMoves;
	}

	while(moveList != NULL) {
		if (remotenessList->remoteness >= remoteness) {
			newMove->next = moveList;
			newRemoteness->next = remotenessList;

			if (prevMoveList==NULL) {
				valueMoves->moveList[typeofMove] = newMove;
				valueMoves->remotenessList[typeofMove] = newRemoteness;
			} else {
				prevMoveList->next = newMove;
				prevRemotenessList->next = newRemoteness;
			}
			return (valueMoves);
		} else {
			prevMoveList = moveList;
			moveList = moveList->next;
			prevRemotenessList = remotenessList;
			remotenessList = remotenessList->next;
		}
	}

	prevMoveList->next = newMove;
	prevRemotenessList->next = newRemoteness;
	return (valueMoves);
}

PLAYER NewHumanPlayer(STRING name, int turn)
{
	PLAYER new = (PLAYER)SafeMalloc(sizeof(struct Player));
	new->name = name;
	new->turn = turn;
	new->type = Human;
	new->GetMove = GetAndPrintPlayersMove;
	return new;
}

PLAYER NewComputerPlayer(STRING name, int turn)
{
	PLAYER new = (PLAYER)SafeMalloc(sizeof(struct Player));
	new->name = name;
	new->turn = turn;
	new->type = Computer;
	new->GetMove = ComputerMove;
	return new;
}

PLAYER NewSEvalPlayer(STRING name, int turn)
{
	PLAYER new = (PLAYER)SafeMalloc(sizeof(struct Player));
	new->name = name;
	new->turn = turn;
	new->type = Evaluator;
	new->GetMove = SEvalMove;
	return new;
}

USERINPUT LocalPlayersMove(POSITION position, MOVE* move, STRING name) {
	void sendLocalMove();
	USERINPUT result = GetAndPrintPlayersMove(position, move, name);
	sendLocalMove(move, name, gTurnNumber);
	gTurnNumber++;
	return result;
}

USERINPUT RemoteMove(POSITION position, MOVE* move, STRING name) {
	int getRemoteMove();
	*move = getRemoteMove(name, gTurnNumber);
	PrintComputersMove(*move, name);
	gTurnNumber++;
	return Continue;
}

int SetupNetworkGame(STRING gameName) {
	httpreq *req;
	httpres *res;
	char *url = malloc(256); //"127.0.0.1:3000/game/request_game_url"; //gMPServerAddress
	char *body = malloc(256);
	char *tmp;
	sprintf(body, "game_name=%s", gameName);

	gRemoteGameURL = malloc(256);

	printf("Initiating game.\n");
	printf("This may take awhile (if no one else wants to play this game)\n");

	while (TRUE) {
		strncpy(url, gMPServerAddress, 255);
		if (newrequest(url, &req, &tmp) != 0)
			exit(1);
		addheader(req, "Content-Type", "application/x-www-form-urlencoded");
		post(req, body, strlen(body), &res, &tmp);

		if (strncmp(res->status, "HTTP/1.1 201", 12) == 0) {
			// we have a game url (that uniquely represents a game)
			// and we are player two
			printf("\nYou have been matched with an opponent... ");

			// lets figure out what turn we are
			strncpy(gRemoteGameURL, res->body, 255);
			getheader(res, "Your-Turn", &tmp);
			if (strncmp(tmp, "0", 1))  {
				freeresponse(res);
				return 0;
			} else {
				freeresponse(res);
				return 1;
			}
		} else if (strncmp(res->status, "HTTP/1.1 202", 12) == 0) {
			// Server: accepted game id, will process later.
			// Client: okay, will wait for opponent (save the token given by server to poll)
			sprintf(body, "game_id=%s&game_name=%s", res->body, gameName);
			freeresponse(res);
			sleep(5); //poll every 5 seconds
			//return SetupWait(body);
		} else {
			freeresponse(res);
			printf("Server is talking gibberish...\n");
			exit(1);
		}
	}
}

void sendLocalMove(MOVE* move, STRING name, int turnNumber) {
	httpreq *req;
	httpres *res;
	char *body = malloc(256);
	char *url = malloc(256);
	char *tmp;

	sprintf(body, "opponent=%s&turn_number=%d&value=%d", name, turnNumber, *move);

	while (1) {
		strncpy(url, gRemoteGameURL, 255); //"192.168.0.104:3000/game/<game_id>/get_last_move"
		if (newrequest(url, &req, &tmp) != 0)
			exit(1);
		addheader(req, "Content-Type", "application/x-www-form-urlencoded");
		post(req, body, strlen(body), &res, &tmp);

		if (strncmp(res->status, "HTTP/1.1 204", 12) == 0) {
			freeresponse(res);
			free(body);
			free(url);
			return; // we are done here
		} else {
			sleep(5); //poll every 5 seconds

			// reset the body, lets not submit the same data over and over eh?
			//sprintf(body, "opponent=%s&turn_number=%d", name, turnNumber);
			freeresponse(res);
		}
	}
}

int getRemoteMove(STRING name, int turnNumber) {
	httpreq *req;
	httpres *res;
	char *body = malloc(256);
	char *url = malloc(256);
	char *tmp;
	int result;

	sprintf(body, "opponent=%s&turn_number=%d", name, turnNumber);
	printf("Waiting for opponent to move...\n");

	while (1) {
		strncpy(url, gRemoteGameURL, 255); //"192.168.0.104:3000/game/<game_id>/get_last_move"
		if (newrequest(url, &req, &tmp) != 0)
			exit(1);
		addheader(req, "Content-Type", "application/x-www-form-urlencoded");
		post(req, body, strlen(body), &res, &tmp);

		if (strncmp(res->status, "HTTP/1.1 200", 12) == 0) {
			result = atoi(res->body);
			freeresponse(res);
			break;
		} else { // reply was a 304 (no modifications)
			sleep(5); //poll every 5 seconds
			freeresponse(res);
		}
	}
	free(body);
	free(url);
	return result;
}

void reportGameEnd(int code) {
	printf("Game ended!\n");
}

/* END Jiong
   Who is working on this?? */

void IncrementTurnNumber() {
	gTurnNumber++;
}

int getTurnNumber() {
	return gTurnNumber;
}

USERINPUT ComputerMove(POSITION position, MOVE* move, STRING name)
{
	*move = GetComputersMove(position);
	PrintComputersMove(*move,name);
	return Continue;
}

USERINPUT SEvalMove(POSITION position, MOVE* move, STRING name)
{
	*move = GetSEvalMove(position);
	PrintComputersMove(*move,name);
	return Continue;
}

/**
 * findDelta(REMOTENESS, REMOTENESSLIST, VALUE)
 *
 * Given the REMOTENESS, the relevant REMOTENESSLIST, VALUE of the MOVE
 *
 * Return the Delta-Remoteness of the MOVE
 *
 * The best move of any type will increase the Remoteness by 1
 * (e.g., 1 closer to a win, 1 closer to a lose, 1 closer to a tie)
 * or 0 (e.g., still a draw).
 *
 * The Delta-Remoteness of the best move will be 0, and increasing
 * Delta-Remoteness will indicate progressively worse moves.
 * (i.e., farther from a win or closer to a lose)
 *
 * In the case of a Tieing Move, we want to reach the Tie ASAP since
 * we must assume the opponent is playing perfectly.
 **/
REMOTENESS FindDelta(REMOTENESS remote, REMOTENESSLIST* remoteptr, VALUE val)
{

	if (DEBUG) {
		printf("\nFindDelta called with REMOTENESS:%d\n", (int) remote);
		printf("FindDelta called with VALUE:%d\n", (int) val);
	}

	switch (val) {
	case win:
		return FindWinningDelta(remote, remoteptr);
	case lose:
		return FindLosingDelta(remote, remoteptr);
	case tie:
		return FindTieingDelta(remote, remoteptr);
	default:
		/* BadElse("FindDelta Given Unrecognized Move Value\n"); */
		/* hack s.t. this is not displayed during GUI use */
		return kBadRemoteness;
	}

	/* reaching this is a bad, bad thing */
	return -1;

}

/* FindWinningDelta - return the delta remoteness of a Winning Move */
REMOTENESS FindWinningDelta(REMOTENESS remote, REMOTENESSLIST* remoteptr)
{
	REMOTENESS min_remoteness = findMinRemoteness(remoteptr);
	REMOTENESS delta;

	if (DEBUG) {
		printf("FindWinningDelta called with REMOTENESS:%d\n", (int) remote);
	}

	delta = remote - min_remoteness;

	if (DEBUG) {
		printf("returning delta:%d\n", (int) delta);
	}

	return delta;
}

/* FindLosingDelta - return the delta remoteness of a Losing Move */
REMOTENESS FindLosingDelta(REMOTENESS remote, REMOTENESSLIST* remoteptr)
{
	REMOTENESS max_remoteness = findMaxRemoteness(remoteptr);
	REMOTENESS delta;

	if (DEBUG) {
		printf("FindLosingDelta called with REMOTENESS:%d\n", (int) remote);
	}

	delta = max_remoteness - remote;

	if (DEBUG) {
		printf("returning delta:%d\n", (int) delta);
	}

	return delta;
}

/* FindTieingDelta - return the delta remoteness of a Tieing Move */
REMOTENESS FindTieingDelta(REMOTENESS remote, REMOTENESSLIST* remoteptr)
{
	REMOTENESS min_remoteness = findMinRemoteness(remoteptr);
	REMOTENESS delta;

	if (DEBUG) {
		printf("FindTieingDelta called with REMOTENESS:%d\n", (int) remote);
	}

	delta = remote - min_remoteness;

	if (DEBUG) {
		printf("returning delta:%d\n", (int) delta);
	}

	return delta;
}


/**
 * findMaxRemoteness(REMOTENESSLIST)
 *
 * Given a REMOTENESSLIST
 *
 * Return the maximum REMOTENESS within the list
 **/
REMOTENESS findMaxRemoteness(REMOTENESSLIST* remoteptr)
{
	REMOTENESS max = 0;
	REMOTENESS temp = 0;

	if (DEBUG) {
		printf("\nfindMaxRemoteness called.\n");
	}

	while (remoteptr != NULL) {
		temp = remoteptr->remoteness;
		if (DEBUG) {
			printf("temp: %d\n", temp);
		}
		if (temp > max) {
			max = temp;
			if (DEBUG) {
				printf("new max remoteness found: %d\n", max);
			}
		}
		remoteptr = remoteptr->next;
	}

	return max;
}

/**
 * findMinRemoteness(REMOTENESSLIST)
 *
 * Given a REMOTENESSLIST
 *
 * Return the minimum REMOTENESS within the list
 **/
REMOTENESS findMinRemoteness(REMOTENESSLIST* remoteptr)
{
	REMOTENESS min = REMOTENESS_MAX;
	REMOTENESS temp = REMOTENESS_MAX;

	if (DEBUG) {
		printf("\nfindMinRemoteness called.\n");
	}

	while (remoteptr != NULL) {
		temp = remoteptr->remoteness;
		if (DEBUG) {
			printf("temp: %d\n", temp);
		}
		if (temp < min) {
			min = temp;
			if (DEBUG) {
				printf("new min remoteness found: %d\n", min);
			}
		}
		remoteptr = remoteptr->next;
	}

	return min;
}

/* Matt Jacobsen: ONLINE GAMING FUNCTIONS */

int LoginUser(STRING username, STRING password, char** sessionId, char** errMsg)
{
	httpreq *req;
	httpres *res;
	int errCode;
	char * url = malloc(strlen(ServerAddress)+1);
	memcpy(url,ServerAddress,strlen(ServerAddress)+1);

	if ((errCode = newrequest(url, &req, errMsg)) != 0)
		return errCode;
	settype(req, HD_LOGON_USER);
	addheader(req, HD_USERNAME, username);
	addheader(req, HD_PASSWORD, password);
	if ((errCode = post(req,NULL,0,&res,errMsg)) != 0)
		return errCode;

	if ((errCode = responseerrorcheck(res, errMsg)) == 0)
	{
		// Read the session/secret key
		getheader(res,HD_SECRET_KEY, sessionId);
	}
	freeresponse(res);
	return errCode;
}

int LogoutUser(STRING username, STRING password, STRING sessionId, char** errMsg)
{
	httpreq *req;
	httpres *res;
	int errCode;
	char * url = malloc(strlen(ServerAddress)+1);
	memcpy(url,ServerAddress,strlen(ServerAddress)+1);

	if ((errCode = newrequest(url, &req, errMsg)) != 0)
		return errCode;
	settype(req, HD_LOGOFF_USER);
	addheader(req, HD_USERNAME, username);
	addheader(req, HD_PASSWORD, password);
	addheader(req, HD_SECRET_KEY, sessionId);
	if ((errCode = post(req,NULL,0,&res,errMsg)) != 0)
		return errCode;

	errCode = responseerrorcheck(res, errMsg);
	freeresponse(res);
	return errCode;
}

int RegisterUser(STRING username, STRING password, char** errMsg)
{
	httpreq *req;
	httpres *res;
	int errCode;
	char * url = malloc(strlen(ServerAddress)+1);
	memcpy(url,ServerAddress,strlen(ServerAddress)+1);

	if ((errCode = newrequest(url, &req, errMsg)) != 0)
		return errCode;
	settype(req, HD_CREATE_USER);
	addheader(req, HD_USERNAME, username);
	addheader(req, HD_PASSWORD, password);
	if ((errCode = post(req,NULL,0,&res,errMsg)) != 0)
		return errCode;

	errCode = responseerrorcheck(res, errMsg);
	freeresponse(res);
	return errCode;
}

int GetUsers(STRING username, STRING password, STRING sessionId, char** users, char** errMsg)
{
	httpreq *req;
	httpres *res;
	int errCode;
	char * url = malloc(strlen(ServerAddress)+1);
	memcpy(url,ServerAddress,strlen(ServerAddress)+1);

	if ((errCode = newrequest(url, &req, errMsg)) != 0)
		return errCode;
	settype(req, HD_GET_USERS);
	addheader(req, HD_USERNAME, username);
	addheader(req, HD_PASSWORD, password);
	addheader(req, HD_SECRET_KEY, sessionId);
	if ((errCode = post(req,NULL,0,&res,errMsg)) != 0)
		return errCode;

	if ((errCode = responseerrorcheck(res, errMsg)) == 0)
	{
		if (res->bodyLength > 0)
		{
			// Read the body that should contain user/game records. Each one terminated by
			// "\n". Where the fields in each record are delimited by ":".
			if ((*users = malloc(res->bodyLength+1)) == NULL)
			{
				fprintf(stderr,"ERROR, could not allocate memory for reading response body\n");
				return 1;
			}
			strcpy(*users, res->body);
		}
		else
		{
			*users = NULL;
		}
	}
	freeresponse(res);
	return errCode;
}

int RegisterGame(STRING username, STRING password, STRING sessionId, STRING gamename, STRING gamevariant, STRING gamedesc, STRING iMoveFirst, char** gameId, char** errMsg)
{
	httpreq *req;
	httpres *res;
	int errCode;
	char * url = malloc(strlen(ServerAddress)+1);
	memcpy(url,ServerAddress,strlen(ServerAddress)+1);

	if ((errCode = newrequest(url, &req, errMsg)) != 0)
		return errCode;
	settype(req, HD_REGISTER_GAME);
	addheader(req, HD_USERNAME, username);
	addheader(req, HD_PASSWORD, password);
	addheader(req, HD_SECRET_KEY, sessionId);
	addheader(req, HD_GAME_NAME, gamename);
	addheader(req, HD_GAME_VARIANT, gamevariant);
	addheader(req, HD_GAME_DESCRIPTION, gamedesc);
	addheader(req, HD_GAME_HOST_MOVES_FIRST, iMoveFirst);
	if ((errCode = post(req,NULL,0,&res,errMsg)) != 0)
		return errCode;

	if ((errCode = responseerrorcheck(res, errMsg)) == 0)
	{
		// Read the gameId
		getheader(res,HD_GAME_ID, gameId);
	}
	freeresponse(res);
	return errCode;
}

int JoinGame(STRING username, STRING password, STRING sessionId, STRING gameId, char** errMsg)
{
	httpreq *req;
	httpres *res;
	int errCode;
	char * url = malloc(strlen(ServerAddress)+1);
	memcpy(url,ServerAddress,strlen(ServerAddress)+1);

	if ((errCode = newrequest(url, &req, errMsg)) != 0)
		return errCode;
	settype(req, HD_JOIN_GAME);
	addheader(req, HD_USERNAME, username);
	addheader(req, HD_PASSWORD, password);
	addheader(req, HD_SECRET_KEY, sessionId);
	addheader(req, HD_GAME_ID, gameId);
	if ((errCode = post(req,NULL,0,&res,errMsg)) != 0)
		return errCode;

	errCode = responseerrorcheck(res, errMsg);
	freeresponse(res);
	return errCode;
}

int ReceivedChallenge(STRING username, STRING password, STRING sessionId, STRING gameId, char** status, char** errMsg)
{
	httpreq *req;
	httpres *res;
	int errCode;
	char * url = malloc(strlen(ServerAddress)+1);
	memcpy(url,ServerAddress,strlen(ServerAddress)+1);

	if ((errCode = newrequest(url, &req, errMsg)) != 0)
		return errCode;
	settype(req, HD_RECEIVED_CHALLENGE);
	addheader(req, HD_USERNAME, username);
	addheader(req, HD_PASSWORD, password);
	addheader(req, HD_SECRET_KEY, sessionId);
	addheader(req, HD_GAME_ID, gameId);
	if ((errCode = post(req,NULL,0,&res,errMsg)) != 0)
		return errCode;

	if ((errCode = responseerrorcheck(res, errMsg)) == 0)
	{
		// Read the status
		getheader(res,HD_STATUS, status);
	}
	freeresponse(res);
	return errCode;
}

int AcceptChallenge(STRING username, STRING password, STRING sessionId, STRING gameId, STRING accept, char** moveFirst, char** errMsg)
{
	httpreq *req;
	httpres *res;
	int errCode;
	char * url = malloc(strlen(ServerAddress)+1);
	memcpy(url,ServerAddress,strlen(ServerAddress)+1);

	if ((errCode = newrequest(url, &req, errMsg)) != 0)
		return errCode;
	settype(req, HD_ACCEPT_CHALLENGE);
	addheader(req, HD_USERNAME, username);
	addheader(req, HD_PASSWORD, password);
	addheader(req, HD_SECRET_KEY, sessionId);
	addheader(req, HD_GAME_ID, gameId);
	addheader(req, HD_ACCEPT, accept);
	if ((errCode = post(req,NULL,0,&res,errMsg)) != 0)
		return errCode;

	if ((errCode = responseerrorcheck(res, errMsg)) == 0)
	{
		// Read the move first results
		getheader(res,HD_MOVE_FIRST, moveFirst);
	}
	freeresponse(res);
	return errCode;
}

int GetGameStatus(STRING username, STRING password, STRING sessionId, STRING gameId, char** users, char** errMsg)
{
	httpreq *req;
	httpres *res;
	int errCode;
	char * url = malloc(strlen(ServerAddress)+1);
	memcpy(url,ServerAddress,strlen(ServerAddress)+1);

	if ((errCode = newrequest(url, &req, errMsg)) != 0)
		return errCode;
	settype(req, HD_GET_GAME_STATUS);
	addheader(req, HD_USERNAME, username);
	addheader(req, HD_PASSWORD, password);
	addheader(req, HD_SECRET_KEY, sessionId);
	addheader(req, HD_GAME_ID, gameId);
	if ((errCode = post(req,NULL,0,&res,errMsg)) != 0)
		return errCode;

	if ((errCode = responseerrorcheck(res, errMsg)) == 0)
	{
		if (res->bodyLength > 0)
		{
			// Read the body that should contain user records. Each one terminated by
			// "\n". Where the fields in each record are delimited by ":".
			if ((*users = malloc(res->bodyLength+1)) == NULL)
			{
				fprintf(stderr,"ERROR, could not allocate memory for reading response body\n");
				return 1;
			}
			strcpy(*users, res->body);
		}
		else
		{
			*users = NULL;
		}
	}
	freeresponse(res);
	return errCode;
}

int AcceptedChallenge(STRING username, STRING password, STRING sessionId, STRING gameId, char** status, char** moveFirst, char** errMsg)
{
	httpreq *req;
	httpres *res;
	int errCode;
	char * url = malloc(strlen(ServerAddress)+1);
	memcpy(url,ServerAddress,strlen(ServerAddress)+1);

	if ((errCode = newrequest(url, &req, errMsg)) != 0)
		return errCode;
	settype(req, HD_ACCEPTED_CHALLENGE);
	addheader(req, HD_USERNAME, username);
	addheader(req, HD_PASSWORD, password);
	addheader(req, HD_SECRET_KEY, sessionId);
	addheader(req, HD_GAME_ID, gameId);
	if ((errCode = post(req,NULL,0,&res,errMsg)) != 0)
		return errCode;

	if ((errCode = responseerrorcheck(res, errMsg)) == 0)
	{
		// Read the status & the moveFirst header (if available)
		getheader(res,HD_STATUS, status);
		getheader(res,HD_MOVE_FIRST, moveFirst);
	}
	freeresponse(res);
	return errCode;
}

int UnregisterGame(STRING username, STRING password, STRING sessionId, STRING gameId, char** challenger, char** errMsg)
{
	httpreq *req;
	httpres *res;
	int errCode;
	char * url = malloc(strlen(ServerAddress)+1);
	memcpy(url,ServerAddress,strlen(ServerAddress)+1);

	if ((errCode = newrequest(url, &req, errMsg)) != 0)
		return errCode;
	settype(req, HD_UNREGISTER_GAME);
	addheader(req, HD_USERNAME, username);
	addheader(req, HD_PASSWORD, password);
	addheader(req, HD_SECRET_KEY, sessionId);
	addheader(req, HD_GAME_ID, gameId);
	if ((errCode = post(req,NULL,0,&res,errMsg)) != 0)
		return errCode;

	if ((errCode = responseerrorcheck(res, errMsg)) == 312)
	{
		// Read the challenger username (if available)
		getheader(res,HD_CHALLENGER_USERNAME, challenger);
	}
	freeresponse(res);
	return errCode;
}

int DeselectChallenger(STRING username, STRING password, STRING sessionId, STRING gameId, char** challenger, char** errMsg)
{
	httpreq *req;
	httpres *res;
	int errCode;
	char * url = malloc(strlen(ServerAddress)+1);
	memcpy(url,ServerAddress,strlen(ServerAddress)+1);

	if ((errCode = newrequest(url, &req, errMsg)) != 0)
		return errCode;
	settype(req, HD_DESELECT_CHALLENGER);
	addheader(req, HD_USERNAME, username);
	addheader(req, HD_PASSWORD, password);
	addheader(req, HD_SECRET_KEY, sessionId);
	addheader(req, HD_GAME_ID, gameId);
	if ((errCode = post(req,NULL,0,&res,errMsg)) != 0)
		return errCode;

	if ((errCode = responseerrorcheck(res, errMsg)) == 312)
	{
		// Read the challenger username (if available)
		getheader(res,HD_CHALLENGER_USERNAME, challenger);
	}
	freeresponse(res);
	return errCode;
}

int SelectChallenger(STRING username, STRING password, STRING sessionId, STRING gameId, STRING selChallenger, char** challenger, char** errMsg)
{
	httpreq *req;
	httpres *res;
	int errCode;
	char * url = malloc(strlen(ServerAddress)+1);
	memcpy(url,ServerAddress,strlen(ServerAddress)+1);

	if ((errCode = newrequest(url, &req, errMsg)) != 0)
		return errCode;
	settype(req, HD_SELECT_CHALLENGER);
	addheader(req, HD_USERNAME, username);
	addheader(req, HD_PASSWORD, password);
	addheader(req, HD_SECRET_KEY, sessionId);
	addheader(req, HD_GAME_ID, gameId);
	addheader(req, HD_CHALLENGER_USERNAME, selChallenger);
	if ((errCode = post(req,NULL,0,&res,errMsg)) != 0)
		return errCode;

	if ((errCode = responseerrorcheck(res, errMsg)) == 312)
	{
		// Read the challenger username (if available)
		getheader(res,HD_CHALLENGER_USERNAME, challenger);
	}
	freeresponse(res);
	return errCode;
}

int GetLastOnlineMove(STRING username, STRING password, STRING sessionId, STRING gameId, char** move, char** errMsg)
{
	httpreq *req;
	httpres *res;
	int errCode;
	char * url = malloc(strlen(ServerAddress)+1);
	memcpy(url,ServerAddress,strlen(ServerAddress)+1);

	if ((errCode = newrequest(url, &req, errMsg)) != 0)
		return errCode;
	settype(req, HD_GET_LAST_MOVE);
	addheader(req, HD_USERNAME, username);
	addheader(req, HD_PASSWORD, password);
	addheader(req, HD_SECRET_KEY, sessionId);
	addheader(req, HD_GAME_ID, gameId);
	if ((errCode = post(req,NULL,0,&res,errMsg)) != 0)
		return errCode;

	if ((errCode = responseerrorcheck(res, errMsg)) == 0)
	{
		// Read the last move (if available)
		getheader(res,HD_LAST_MOVE, move);
	}
	freeresponse(res);
	return errCode;
}

int SendNewOnlineMove(STRING username, STRING password, STRING sessionId, STRING gameId, STRING move, char** errMsg)
{
	httpreq *req;
	httpres *res;
	int errCode;
	char * url = malloc(strlen(ServerAddress)+1);
	memcpy(url,ServerAddress,strlen(ServerAddress)+1);

	if ((errCode = newrequest(url, &req, errMsg)) != 0)
		return errCode;
	settype(req, HD_SEND_MOVE);
	addheader(req, HD_USERNAME, username);
	addheader(req, HD_PASSWORD, password);
	addheader(req, HD_SECRET_KEY, sessionId);
	addheader(req, HD_GAME_ID, gameId);
	addheader(req, HD_MOVE, move);
	if ((errCode = post(req,NULL,0,&res,errMsg)) != 0)
		return errCode;

	errCode = responseerrorcheck(res, errMsg);
	freeresponse(res);
	return errCode;
}

int SendGameOver(STRING username, STRING password, STRING sessionId, STRING gameId, char** errMsg)
{
	httpreq *req;
	httpres *res;
	int errCode;
	char * url = malloc(strlen(ServerAddress)+1);
	memcpy(url,ServerAddress,strlen(ServerAddress)+1);

	if ((errCode = newrequest(url, &req, errMsg)) != 0)
		return errCode;
	settype(req, HD_GAME_OVER);
	addheader(req, HD_USERNAME, username);
	addheader(req, HD_PASSWORD, password);
	addheader(req, HD_SECRET_KEY, sessionId);
	addheader(req, HD_GAME_ID, gameId);
	if ((errCode = post(req,NULL,0,&res,errMsg)) != 0)
		return errCode;

	errCode = responseerrorcheck(res, errMsg);
	freeresponse(res);
	return errCode;
}

int SendResign(STRING username, STRING password, STRING sessionId, STRING gameId, char** errMsg)
{
	httpreq *req;
	httpres *res;
	int errCode;
	char * url = malloc(strlen(ServerAddress)+1);
	memcpy(url,ServerAddress,strlen(ServerAddress)+1);

	if ((errCode = newrequest(url, &req, errMsg)) != 0)
		return errCode;
	settype(req, HD_RESIGN);
	addheader(req, HD_USERNAME, username);
	addheader(req, HD_PASSWORD, password);
	addheader(req, HD_SECRET_KEY, sessionId);
	addheader(req, HD_GAME_ID, gameId);
	if ((errCode = post(req,NULL,0,&res,errMsg)) != 0)
		return errCode;

	errCode = responseerrorcheck(res, errMsg);
	freeresponse(res);
	return errCode;
}
