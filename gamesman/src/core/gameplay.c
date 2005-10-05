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

#define qq printf("%d\n", __LINE__)

typedef struct moveList {
  MOVE move;
  POSITION position;
  struct moveList* next;
  struct moveList* prev;
}moveList;

moveList* mList;
int maxR;
int maxTR;

/*
** Local function prototypes
*/

static	MOVE		RandomLargestRemotenessMove	(MOVELIST*, REMOTENESSLIST*);
static	MOVE		RandomSmallestRemotenessMove	(MOVELIST*, REMOTENESSLIST*);
static	VALUE_MOVES*	SortMoves			(POSITION, MOVE, VALUE_MOVES*);
static	VALUE_MOVES*	StoreMoveInList			(MOVE, REMOTENESS, VALUE_MOVES*, int);
static  void            printLine                       (char*, char*, int, POSITION, int, int, int, int);
static  char*           addMove                         (char*, int, char*, int);
static  char*           addRemoteness                   (char*, int, int, int);
static  char*           addDraw                         (char*, int);
static  char*           addSpacePadding                 (char*, int);
static  char*           digitToString                   (char*, int);
static  moveList*       moveListHandleUndo              (moveList*);
static  moveList*       moveListHandleNewMove           (POSITION, MOVE, moveList*);
static  void            moveListHandleGameOver             (moveList*);

OPPONENT gOpponent;

void PlayGame(PLAYER playerOne, PLAYER playerTwo)
{
    char c;
    POSITION position;
    MOVE move;
    UNDO *undo;
    VALUE value;
    BOOLEAN error, player_draw;
    BOOLEAN playing = TRUE;
    BOOLEAN aborted = FALSE;
    BOOLEAN menu    = TRUE;
    USERINPUT userInput = Continue; /* default added to satify compiler */
    PLAYER player = playerOne;
    int oldRemainingGivebacks;
    moveList* mlist = 0;
    int i;
    maxR = 14;
    maxTR = 14;
    int r;
    for(i = 0; i < gNumberOfPositions; i++){
      r = Remoteness(i);
      if (GetValueOfPosition(i) != tie) { if (r > maxR) maxR = r; }
      else { if (r > maxTR) maxTR = r; }
    }
    position = gInitialPosition;
    undo = InitializeUndo();
    
#ifndef X
    printf("Type '?' if you need assistance...\n\n");
#endif
    
    while(playing) {

	while((value = Primitive(position)) == undecided) {
	    oldRemainingGivebacks = remainingGivebacks;


	    PrintPosition(position, player->name, (player->type==Human?TRUE:FALSE));
	    if((userInput = player->GetMove(position,&move,player->name)) == Undo) {
	        undo = HandleUndoRequest(&position,undo,&error);
	        if(!error && gOpponent == AgainstHuman) {
		    player = (player->turn ? playerOne : playerTwo);
	        }
	        if (!error) {
		    mlist = moveListHandleUndo(mlist);
		}
	    } else if(userInput == Abort) {
		break;
	    } else {
	        position = DoMove(position,move);
		mlist = moveListHandleNewMove(position, move, mlist);
		undo = UpdateUndo(position,undo,&player_draw);
		undo->givebackUsed = oldRemainingGivebacks>remainingGivebacks;
		if(!gGoAgain(position,move)) {
		    player = (player->turn ? playerOne : playerTwo);
		}
		if(player_draw)
		    break;
	    }
	}
	PrintPosition(position,player->name,(player->type==Human?TRUE:FALSE));
	switch(value) {
	case tie:
	    printf("The match ends in a draw. Excellent strategies, %s and %s\n",playerOne->name,playerTwo->name);
	    break;
	case lose:
	    printf("\n%s (player %s) Wins!\n\n",player->turn?playerOne->name:playerTwo->name,player->turn?"one":"two");
	    break;
	case win:
	    printf("\n%s (player %s) Wins!\n\n",player->turn?playerTwo->name:playerOne->name,player->turn?"two":"one");
	    break;
	default:
	    if(userInput == Abort) {
		printf("Your abort command has been received and successfully processed!\n");
		moveListHandleGameOver(mlist);
		aborted = TRUE;
		playing = FALSE;
	    }
	    else if(player_draw)
		printf("The match ends in a draw.  Excellent strategy, %s and %s. \n\n",playerOne->name,playerTwo->name);
	    else
		BadElse("PlayGame");
	}
	if(!aborted) {
	    while(menu) {
		printf("%s%s%s%s%s",
		       "What would you like to do?\n",
		       (gOpponent != ComputerComputer) ? 
		       "\tu)\t(U)ndo the last move\n" : "",
		       "\tb)\t(B)ack to the menu\n",
		       "\tq)\t(Q)uit\n\n",
		       "Select an option:  ");
		c = GetMyChar();
		switch(c) {
		case 'u': case 'U':
		    if(gOpponent != ComputerComputer) {
			undo = HandleUndoRequest(&position,undo,&error);
			moveListHandleUndo(mlist);
			playing = TRUE;
			menu = FALSE;
		    }
		    else
			BadMenuChoice();
		    break;
		case 'b': case 'B':
		    moveListHandleGameOver(mlist);
		    playing = FALSE;
		    menu = FALSE;
		    break;
		case 'q': case 'Q':
		    ExitStageRight();
		    exit(0);
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
moveList* moveListHandleUndo(moveList* lastEntry) {
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
    lastEntry = 0;
  }
  return lastEntry;
}

moveList* moveListHandleNewMove(POSITION position, MOVE move, 
				moveList* lastEntry) {
  moveList* newmlist;
  newmlist = (moveList*) SafeMalloc(sizeof(moveList));
  newmlist->move = move;
  newmlist->position = position;
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

void moveListHandleGameOver(moveList* lastEntry) {
  moveList* temp = lastEntry;
  while(lastEntry != 0) {
    lastEntry = lastEntry->prev;
    SafeFree(temp);
    temp = lastEntry;
  }
  mList = 0;
}

void PrintVisualValueHistory(POSITION position)
{
  moveList* mlist = mList;
  int maxN = 10;
  int maxL = maxN+1+maxR+1+maxTR+1+maxTR+1+maxR+1+maxN;
  int i;
  int whoseTurn;

  char* digit = "";
  char line[maxL];

  if (gOpponent == AgainstComputer && !gHumanGoesFirst)
    whoseTurn = kPlayerOneTurn;
  else whoseTurn = kPlayerTwoTurn;

  printf("\n");
  strcpy(line, "*");
  for (i = 0; i < maxL-1; i++) strcat(line, "*");
  printf("%s\n", line);
  printf("%s Visual Value History (aka \"who messed up when?!)\n", 
	 kGameName);
  printf("%s\n", line);
  PrintPosition(gInitialPosition, gPlayerName[whoseTurn], whoseTurn);
  printf("\n");

  addSpacePadding(strcpy(line,"LEFT"),maxN-strlen("LEFT"));
  addSpacePadding(strcat(line,"|<-WIN"),maxR-strlen("<-WIN"));
  addSpacePadding(strcat(line,"|<-TIE"),maxTR-strlen("<-TIEDR"));
  addSpacePadding(strcat(line,"DRAW!"),maxTR-strlen("W!TIE->"));
  addSpacePadding(strcat(line,"TIE->|"),maxR-strlen("WIN->"));
  addSpacePadding(strcat(line,"WIN->|"),maxN-strlen("RIGHT"));
  strcat(line, "RIGHT");
  printf("%s\n", line);
  
  addSpacePadding(strcpy(line, "MOVES"), maxN - strlen("MOVES"));
  for (i = 0; i <= maxR; i++) strcat(line, digitToString(digit,i%10));
  for (i = 0; i <= maxTR; i++) strcat(line, digitToString(digit,i%10));
  strcat(line, "D");
  for (i = maxTR; i >= 0; i--) strcat(line, digitToString(digit,i%10));
  for (i = maxR; i >= 0; i--) strcat(line, digitToString(digit,i%10));
  addSpacePadding(line, maxN - strlen("MOVES"));
  strcat(line, "MOVES");
  printf("%s\n", line);
  
  printLine(gPlayerName[kPlayerOneTurn], gPlayerName[kPlayerTwoTurn], 
	      whoseTurn, gInitialPosition, maxN, maxR,  maxTR, maxL);

  while(mlist != 0) {
    if (whoseTurn == kPlayerTwoTurn) {
      printLine("p1", "", whoseTurn, mlist->position, maxN, maxR,maxTR,maxL);
      whoseTurn = kPlayerOneTurn;
    } else {
      printLine("", "p2", whoseTurn, mlist->position, maxN, maxR,maxTR,maxL);
      whoseTurn = kPlayerTwoTurn;
    }
    mlist = mlist->next;
  }

  if (whoseTurn == kPlayerTwoTurn) whoseTurn = kPlayerOneTurn;
  else whoseTurn = kPlayerTwoTurn;

  PrintPosition(position, gPlayerName[whoseTurn], whoseTurn);
  strcpy(line, "*");
  for (i = 0; i < maxL-1; i++) strcat(line, "*");
  printf("%s\n", line);
}


void printLine(char* mPlayer1, char* mPlayer2, int whoseTurn, 
	       POSITION position, int maxN, int maxR, int maxTR, int maxL) {
  char line[maxL];
  VALUE value = GetValueOfPosition(position);
  int remoteness = Remoteness(position);
  
  /*
  printf("value %d, player %d, win %d, loss %d, firstP %d, secondP %d\n", 
	 value, whoseTurn, win, lose, kPlayerOneTurn, kPlayerTwoTurn);
  */
 
  int winForLeft = ((value == win && whoseTurn == kPlayerOneTurn) 
		    || (value == lose && whoseTurn == kPlayerTwoTurn));
  int winForRight = !winForLeft && value!= tie;
 
  addMove(line, TRUE, mPlayer1, maxN);
  winForLeft ? addRemoteness(line, TRUE, remoteness, maxR) :
    addRemoteness(line, TRUE, -1, maxR);
  value == tie ? addRemoteness(line, 1, remoteness, maxTR) :
    addRemoteness(line, TRUE, -1, maxTR);
  addDraw(line, value == tie);
  value == tie ? addRemoteness(line, 0, remoteness, maxTR) :
    addRemoteness(line, FALSE, -1, maxTR);
  winForRight ? addRemoteness(line, FALSE, remoteness, maxR) :
    addRemoteness(line, FALSE, -1, maxR);
  addMove(line, FALSE, mPlayer2, maxN);
  printf("%s\n", line);
}
char* addMove(char* line, int left, char* move, int maxL){
  if (left) {
    strcpy(line, move);
    addSpacePadding(line, maxL - strlen(move));
  } else {
    addSpacePadding(line, maxL - strlen(move));
    strcat(line, move);
  }
  return line;
}
char* addRemoteness(char* line, int left, int remoteness, int maxR){
  int i;
  if (left) {
    remoteness == 0 ? strcat(line, "*") : strcat(line, "|");
    for(i = 1; i <= maxR; i++) 
      remoteness == i ? strcat(line, "*") : strcat(line, " ");
  } else {
    for(i = maxR; i > 0; i--) 
      remoteness == i ? strcat(line, "*") : strcat(line, " ");
    remoteness == 0 ? strcat(line, "*") : strcat(line, "|");
  }
  return line;
}
char* addDraw(char* line, int draw){
  return draw ? strcat(line, "*") : strcat(line, "|");
} 
char* addSpacePadding(char* s, int n) {
  while (n > 0){
    n--;
    strcat(s, " ");
  }
  return s;
}

char* digitToString(char* s, int d) {
  s = "?";
  if (d == 0) s = "0";
  else if (d == 1) s = "1";
  else if (d == 2) s = "2";
  else if (d == 3) s = "3";
  else if (d == 4) s = "4";
  else if (d == 5) s = "5";
  else if (d == 6) s = "6";
  else if (d == 7) s = "7";
  else if (d == 8) s = "8";
  else if (d == 9) s = "9";
  return s;
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
		  (gOpponent == AgainstComputer && (undo->next->next == NULL))))) {
	
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
    
    if(gOpponent == AgainstComputer) {
	tmp = undo;
	undo = undo->next;
	SafeFree((GENERIC_PTR)tmp);
	*thePosition = undo->position;
    }
    
    return(undo);
}

UNDO *UpdateUndo(POSITION thePosition, UNDO* undo, BOOLEAN* abort)
{
    UNDO *tmp, *index = undo;
    BOOLEAN inList = FALSE;

    while(index != NULL) {
	if(index->position == thePosition) {
	    undo = Stalemate(undo,thePosition,abort);
	    inList = TRUE;
	}
	index = index->next;
    }
    if(!inList) {
        tmp = undo; 
        undo = (UNDO *) SafeMalloc (sizeof(UNDO));
        undo->position = thePosition;
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
    }
    else {
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
    }
    else if(gValue == win || gValue == lose) {
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
    }
    else
        BadElse("PrintComputerValueExplanation");
}

UNDO *Stalemate(UNDO* undo, POSITION stalematePosition, BOOLEAN* abort)
{
    UNDO *tmp;
    
    printf("\nWe have reached a position we have already encountered. We have\n");
    printf("achieved a STALEMATE. Now, we could go on forever playing like this\n");
    printf("or we could just stop now. Should we continue (y/n) ? ");
    
    if(GetMyChar() != 'y') {      /* quit */
        *abort = TRUE;
    }
    else {
        while(undo->next != NULL && undo->position != stalematePosition) {
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
    printf("\t\tMove \tRemoteness\n");
    printf("Winning Moves: \t");
    PrintMoves(ptr->moveList[WINMOVE], ptr->remotenessList[WINMOVE]);
    printf("Tieing Moves: \t");
    PrintMoves(ptr->moveList[TIEMOVE], ptr->remotenessList[TIEMOVE]);
    printf("Losing Moves: \t");
    PrintMoves(ptr->moveList[LOSEMOVE], ptr->remotenessList[LOSEMOVE]);
    printf("\n");
    
    FreeValueMoves(ptr);
}

STRING GetPrediction(POSITION position, STRING playerName, BOOLEAN usersTurn)
{
    static char prediction[80];
    char mexString[20];
    VALUE value;
    
    if(gPrintPredictions && (gMenuMode == Evaluated)) {
        MexFormat(position,mexString);
        value = GetValueOfPosition(position);
	
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
                sprintf(prediction, "(%s %s %s in %d) %s",
			playerName,
			((value == lose && usersTurn && gOpponent == AgainstComputer) ||
			 (value == win && !usersTurn && gOpponent == AgainstComputer)) ?
			"will" : "should",
			gValueString[(int)value],
			Remoteness(position),
			mexString);
            }
        }
    }
    else
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
        }
        else if (remotenessList->remoteness == maxRemoteness) {
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

/* Jiong */
VALUE_MOVES* SortMoves (POSITION thePosition, MOVE move, VALUE_MOVES* valueMoves) 
{
    POSITION child;
    VALUE childValue;
    
    child = DoMove(thePosition, move);
    childValue = GetValueOfPosition(child);
    if (gGoAgain(thePosition, move)) {
        switch(childValue) {
	case win: childValue = lose; break;
	case lose: childValue = win; break;
	default: childValue = childValue;
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
            // WINMOVE: Win as quickly as possible (smallest remoteness best)
            theMove = RandomSmallestRemotenessMove(moves->moveList[moveType], moves->remotenessList[moveType]);
        } else if (moveType == TIEMOVE) {
            // TIEMOVE: Tie as quickly as possible when smart???
            theMove = RandomSmallestRemotenessMove(moves->moveList[moveType], moves->remotenessList[moveType]);
        } else {
            // LOSEMOVE: Prolong the game as much as possible (largest remoteness is best).
            theMove = RandomLargestRemotenessMove(moves->moveList[moveType], moves->remotenessList[moveType]);
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
            printf("%s could equivalently choose [ ", gPlayerName[kComputersTurn]);      }
	
        for (head = NULL, i = 0; i <= LOSEMOVE; i++) {
            ptr = moves -> moveList[i];
            while (ptr) {
                if (gHints) {
                    PrintMove(ptr -> move);
                    printf(" ");
                }
                head = CreateMovelistNode(ptr -> move, head);
                ptr = ptr -> next;
                numberMoves++;
            }
        }
	
        if(gHints)
            printf("]\n\n");
	
        randomMove = GetRandomNumber(numberMoves);
        for (ptr = head; randomMove > 0; --randomMove)
            ptr = ptr -> next;
	
        theMove = ptr -> move;
	
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
        }
        else if (moveType == WINMOVE || moveType == TIEMOVE) {
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
        while(ptr != NULL) {                    /* otherwise  (theValue = (win|tie) */
            valueMoves = SortMoves(thePosition, ptr->move, valueMoves);
            ptr = ptr->next;
        }
        FreeMoveList(head);
    }
    return(valueMoves);
}


/* Jiong */
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
            }
            else {
                prevMoveList->next = newMove;
                prevRemotenessList->next = newRemoteness;
            }
            return (valueMoves);
        }
        else {
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

USERINPUT ComputerMove(POSITION position, MOVE* move, STRING name)
{
    *move = GetComputersMove(position);
    PrintComputersMove(*move,name);
    return Continue;
}
