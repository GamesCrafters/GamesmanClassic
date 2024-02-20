/************************************************************************
**
** NAME:        mdodgem.c
**
** DESCRIPTION: Dodgem
**
** AUTHOR:      Dan Garcia  -  University of California at Berkeley
**              Copyright (C) Dan Garcia, 1995. All rights reserved.
**
** DATE:        04/29/92
**
**************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/

#include "gamesman.h"

POSITION gNumberOfPositions  = 39366;  /* 3^9 times 2 */

POSITION gInitialPosition    = 17524;
POSITION gMinimalPosition    = 17524;

CONST_STRING kAuthorName         = "Dan Garcia";
CONST_STRING kGameName           = "Dodgem";
CONST_STRING kDBName = "dodgem";
BOOLEAN kPartizan           = TRUE;
BOOLEAN kDebugMenu          = FALSE;
BOOLEAN kGameSpecificMenu   = TRUE;
BOOLEAN kTieIsPossible      = FALSE;
BOOLEAN kLoopy               = TRUE;
BOOLEAN kDebugDetermineValue = FALSE;
POSITION kBadPosition           = -1;
void*    gGameSpecificTclInit = NULL;

CONST_STRING kHelpGraphicInterface =
        "The LEFT button puts a small circle over your piece. This selects\n\
the FROM slot. The MIDDLE button then selects the TO slot. If you\n\
wish to remove a piece from the board, click the MIDDLE button on\n\
the same place as the FROM slot. The RIGHT button is the same as UNDO,\n\
in that it reverts back to your most recent position."                                                                                                                                                                                                                                                                                                  ;

CONST_STRING kHelpTextInterface    =
        "On your turn, use the LEGEND to determine which numbers to choose (between\n\
1 and 9, with 1 at the upper left and 9 at the lower right) to correspond\n\
to the location of your piece and the empty orthogonally-adjacent position\n\
you wish to move that piece to. If you wish to move a piece off of the board,\n\
choose 0 as your destination. Example: '20' moves the piece on location\n\
2 off of the board. '52' moves your piece from position 5 to position 2."                                                                                                                                                                                                                                                                                                                                                                                                              ;

CONST_STRING kHelpOnYourTurn =
        "The moves on your turn are different for different players. Here is a summary:\n\
\n\
O player:      The O player may move his pieces UP, DOWN and to the RIGHT.\n\
               The objective is to be the first to move both of your pieces\n\
 ^             off of the board. You may only move one of your pieces to an\n\
 |             adjacent empty spot on your turn. Moving off of the board\n\
 O->           means moving one of your pieces past the RIGHT-HAND-SIDE\n\
 |             of the board.\n\
 v\n\n\
X player:      The X player may move his pieces LEFT, UP, and to the RIGHT.\n\
               The objective is to be the first to move both of your pieces\n\
    ^          off of the board. You may only move one of your pieces to an\n\
    |          adjacent empty spot on your turn. Moving off of the board\n\
 <- X ->       means moving one of your pieces past the UPPER-SIDE of the board.\n\n\
Note: The circle always goes first."                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         ;

CONST_STRING kHelpStandardObjective =
        "To be the FIRST player to move both your pieces off of the board OR be prevented from moving by your opponent's pieces. ";

CONST_STRING kHelpReverseObjective =
        "To be the LAST player to move your pieces off of the board OR prevent \n\
your opponent from moving. "                                                                                   ;

CONST_STRING kHelpTieOccursWhen =   /* Should follow 'A Tie occurs when... */
                            "";

CONST_STRING kHelpExample =
        "         ( 1 2 3 )           : O - -     PLAYER O's turn\n\
LEGEND:  ( 4 5 6 )  TOTAL:   : O - -                     \n\
         ( 7 8 9 )           : - X X                     \n\n\
     Dan's move [(u)ndo/1-9] : 12                       \n\n\
         ( 1 2 3 )           : - O -     PLAYER X's turn \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : O - -                     \n\
         ( 7 8 9 )           : - X X                     \n\n\
Computer's move              : 96                       \n\n\
         ( 1 2 3 )           : - O -     PLAYER O's turn \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : O - X                     \n\
         ( 7 8 9 )           : - X -                     \n\n\
     Dan's move [(u)ndo/1-9] : 23                       \n\n\
         ( 1 2 3 )           : - - O     PLAYER X's turn \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : O - X                     \n\
         ( 7 8 9 )           : - X -                     \n\n\
Computer's move              : 87                       \n\n\
         ( 1 2 3 )           : - - O     PLAYER O's turn \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : O - X                     \n\
         ( 7 8 9 )           : X - -                     \n\n\
     Dan's move [(u)ndo/1-9] : 45                       \n\n\
         ( 1 2 3 )           : - - O     PLAYER X's turn \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - O X                     \n\
         ( 7 8 9 )           : X - -                     \n\n\
Computer's move              : 78                       \n\n\
         ( 1 2 3 )           : - - O     PLAYER O's turn \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - O X                     \n\
         ( 7 8 9 )           : - X -                     \n\n\
     Dan's move [(u)ndo/1-9] : 52                       \n\n\
         ( 1 2 3 )           : - O O     PLAYER X's turn \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - - X                     \n\
         ( 7 8 9 )           : - X -                     \n\n\
Computer's move              : 85                       \n\n\
         ( 1 2 3 )           : - O O     PLAYER O's turn \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - X X                     \n\
         ( 7 8 9 )           : - - -                     \n\n\
     Dan's move [(u)ndo/1-9] : 30                       \n\n\
         ( 1 2 3 )           : - O -     PLAYER X's turn \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - X X                     \n\
         ( 7 8 9 )           : - - -                     \n\n\
Computer's move              : 63                       \n\n\
         ( 1 2 3 )           : - O X     PLAYER O's turn \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - X -                     \n\
         ( 7 8 9 )           : - - -                     \n\n\n\
Computer wins. Nice try, Dan."                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         ;


/*************************************************************************
**
** Everything above here must be in every game file
**
**************************************************************************/

/*************************************************************************
**
** Every variable declared here is only used in this file (game-specific)
**
**************************************************************************/

#define BOARDSIZE        9           /* 3x3 board */
#define POSITION_OFFSET  19683       /* 3^9 */
#define OFFTHEBOARD      9           /* Removing that piece from the board */
#define BADSLOT         -2           /* You've moved off the board in a bad way */
#define INTERACT_ROWS	 4
#define INTERACT_COLS    4
#define INTERACT_BOARDSIZE 16

typedef enum possibleBoardPieces {
	Blank, o, x
} BlankOX;

typedef int SLOT;     /* A slot is the place where a piece moves from or to */

char *gBlankOXString[] = { "-", "O", "X" };

/* Powers of 3 - this is the way I encode the position, as an integer */
int g3Array[] =          { 1, 3, 9, 27, 81, 243, 729, 2187, 6561 };

BOOLEAN gToTrapIsToWin = FALSE;  /* Being stuck is when you can't move. */

/** Function Prototypes */
void PositionToBlankOX(POSITION thePos, BlankOX *theBlankOX, BlankOX *whosTurn);
void MoveToSlots(MOVE theMove, SLOT *fromSlot, SLOT *toSlot);
MOVE SlotsToMove (SLOT fromSlot, SLOT toSlot);
SLOT GetToSlot(SLOT fromSlot, int direction,BlankOX whosTurn);
BOOLEAN OkMove(BlankOX *theBlankOX, BlankOX whosTurn, SLOT fromSlot, int direction);
BlankOX OnlyPlayerLeft(BlankOX *theBlankOX);
BOOLEAN CantMove(POSITION position);
POSITION BlankOXToPosition(BlankOX *theBlankOX, BlankOX whosTurn);

STRING MToS (MOVE);

void InitializeGame() {
}

void FreeGame() {}

/************************************************************************
**
** NAME:        DebugMenu
**
** DESCRIPTION: Menu used to debub internal problems. Does nothing if
**              kDebugMenu == FALSE
**
************************************************************************/

void DebugMenu() {}

/************************************************************************
**
** NAME:        GameSpecificMenu
**
** DESCRIPTION: Menu used to change game-specific parmeters, such as
**              the side of the board in an nxn Nim board, etc. Does
**              nothing if kGameSpecificMenu == FALSE
**
************************************************************************/

void GameSpecificMenu() {
	char GetMyChar();

	do {
		printf("\n\t----- Game-specific options for %s -----\n\n", kGameName);

		printf("\tCurrent Initial Position:\n");
		PrintPosition(gInitialPosition, gPlayerName[kPlayerOneTurn], kHumansTurn);

		printf("\tI)\tChoose the (I)nitial position\n");
		printf("\tT)\t(T)rapping opponent toggle from %s to %s\n",
		       gToTrapIsToWin ? "GOOD (WINNING)" : "BAD (LOSING)",
		       !gToTrapIsToWin ? "GOOD (WINNING)" : "BAD (LOSING)");

		printf("\n\n\tb)\t(B)ack = Return to previous activity.\n");
		printf("\n\nSelect an option: ");

		switch(GetMyChar()) {
		case 'B': case 'b':
			return;
		case 'Q': case 'q':
			ExitStageRight();
			break;
		case 'H': case 'h':
			HelpMenus();
			break;
		case '1':
			gInitialPosition = GetInitialPosition();
			break;
		case 'T': case 't':
			gToTrapIsToWin = !gToTrapIsToWin;
			break;
		default:
			printf("\nSorry, I don't know that option. Try another.\n");
			HitAnyKeyToContinue();
			break;
		}
	} while(TRUE);
}

/************************************************************************
**
** NAME:        SetTclCGameSpecificOptions
**
** DESCRIPTION: Set the C game-specific options (called from Tcl)
**              Ignore if you don't care about Tcl for now.
**
************************************************************************/

void SetTclCGameSpecificOptions(int *theOptions) {
	gToTrapIsToWin = (BOOLEAN) theOptions[0];
}

/************************************************************************
**
** NAME:        DoMove
**
** DESCRIPTION: Apply the move to the position.
**
** INPUTS:      POSITION thePosition : The old position
**              MOVE     theMove     : The move to apply.
**
** OUTPUTS:     (POSITION) : The position that results after the move.
**
** CALLS:       MoveToSlots(MOVE,*SLOT,*SLOT)
**
************************************************************************/

POSITION DoMove(POSITION thePosition, MOVE theMove) {
	SLOT fromSlot, toSlot;
	BlankOX theBlankOX[BOARDSIZE], whosTurn;

	PositionToBlankOX(thePosition,theBlankOX,&whosTurn);
	MoveToSlots(theMove, &fromSlot, &toSlot);

	if(toSlot == OFFTHEBOARD) /* removed from board */
		return(thePosition
		       + (whosTurn == o ? POSITION_OFFSET : -POSITION_OFFSET)
		       - (g3Array[fromSlot] * (int)whosTurn)); /* take from slot */
	else
		return(thePosition
		       + (whosTurn == o ? POSITION_OFFSET : -POSITION_OFFSET)
		       - (g3Array[fromSlot] * (int)whosTurn) /* take from slot */
		       + (g3Array[toSlot] * (int)whosTurn)); /* put in to slot */
}

/************************************************************************
**
** NAME:        GetInitialPosition
**
** DESCRIPTION: Ask the user for an initial position for testing. Store
**              it in the space pointed to by initialPosition;
**
** OUTPUTS:     POSITION initialPosition : The position returned
**
************************************************************************/

POSITION GetInitialPosition() /* UNWRITTEN */
{
	BlankOX theBlankOX[BOARDSIZE], whosTurn;
	signed char c;
	int i;


	printf("\n\n\t----- Get Initial Position -----\n");
	printf("\n\tPlease input the position to begin with.\n");
	printf("\tNote that it should be in the following format:\n\n");
	printf("O - -\nO - -            <----- EXAMPLE \n- X X\n\n");

	i = 0;
	getchar();
	while(i < BOARDSIZE && (c = getchar()) != EOF) {
		if(c == 'x' || c == 'X')
			theBlankOX[i++] = x;
		else if(c == 'o' || c == 'O' || c == '0')
			theBlankOX[i++] = o;
		else if(c == '-')
			theBlankOX[i++] = Blank;
		/* else do nothing */
	}

	getchar();
	printf("\nNow, whose turn is it? [O/X] : ");
	scanf("%c",&c);
	if(c == 'x' || c == 'X')
		whosTurn = x;
	else
		whosTurn = o;

	return(BlankOXToPosition(theBlankOX,whosTurn));
}

/************************************************************************
**
** NAME:        PrintComputersMove
**
** DESCRIPTION: Nicely format the computers move.
**
** INPUTS:      MOVE    computersMove : The computer's move.
**              STRING  computersName : The computer's name.
**
************************************************************************/

void PrintComputersMove(MOVE computersMove, STRING computersName) {
	SLOT fromSlot,toSlot;
	MoveToSlots(computersMove,&fromSlot,&toSlot);
	printf("%8s's move              : %d%d\n", computersName,
	       fromSlot+1,toSlot==OFFTHEBOARD ? 0 : toSlot+1);
}

/************************************************************************
**
** NAME:        Primitive
**
** DESCRIPTION: Return the value of a position if it fulfills certain
**              'primitive' constraints. Some examples of this is having
**              three-in-a-row with TicTacToe. TicTacToe has two
**              primitives it can immediately check for, when the board
**              is filled but nobody has one = primitive tie. Three in
**              a row is a primitive lose, because the player who faces
**              this board has just lost. I.e. the player before him
**              created the board and won. Otherwise undecided.
**
** INPUTS:      POSITION position : The position to inspect.
**
** OUTPUTS:     (VALUE) an enum which is oneof: (win,lose,tie,undecided)
**
** CALLS:       BOOLEAN CantMove()
**              BOOLEAN OkMove
**              PositionToBlankOX()
**              BlankOX OnlyPlayerLeft(*BlankOX)
**
************************************************************************/

VALUE Primitive(POSITION position) {
	BlankOX theBlankOX[BOARDSIZE],whosTurn;

	PositionToBlankOX(position,theBlankOX,&whosTurn);

	if (OnlyPlayerLeft(theBlankOX) == whosTurn)
		return(gStandardGame ? lose : win); /* cause you're the only one left */
	else if(CantMove(position)) /* the other player just won */
		return(gToTrapIsToWin ? lose : win); /* !gStandardGame */
	else
		return(undecided);        /* no one has won yet */
}

BOOLEAN CantMove(POSITION position) {
	MOVELIST *ptr;
	BOOLEAN cantMove;

	ptr = GenerateMoves(position);
	cantMove = (ptr == NULL);
	FreeMoveList(ptr);
	return(cantMove);
}


BlankOX OnlyPlayerLeft(BlankOX *theBlankOX) {
	int i;
	BOOLEAN sawO = FALSE, sawX = FALSE;
	for(i = 0; i < BOARDSIZE; i++) {
		sawO |= (theBlankOX[i] == o);
		sawX |= (theBlankOX[i] == x);
	}
	if(sawX && !sawO)
		return(x);
	else if (sawO && !sawX)
		return(o);
	else if (sawO && sawX)
		return(Blank);
	else {
		printf("Error in OnlyPlayerLeft! the board is blank!!!\n");
		return(Blank);
	}
}

/************************************************************************
**
** NAME:        PrintPosition
**
** DESCRIPTION: Print the position in a pretty format, including the
**              prediction of the game's outcome.
**
** INPUTS:      POSITION position   : The position to pretty print.
**              STRING   playerName : The name of the player.
**              BOOLEAN  usersTurn  : TRUE <==> it's a user's turn.
**
** CALLS:       PositionToBlankOX()
**              GetValueOfPosition()
**              GetPrediction()
**
************************************************************************/

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn) {
	//  VALUE GetValueOfPosition();
	BlankOX theBlankOx[BOARDSIZE], whosTurn;

	PositionToBlankOX(position,theBlankOx,&whosTurn);

	printf("\n         ( 1 2 3 )           : %s %s %s     PLAYER %s's turn\n",
	       gBlankOXString[(int)theBlankOx[0]],
	       gBlankOXString[(int)theBlankOx[1]],
	       gBlankOXString[(int)theBlankOx[2]],
	       gBlankOXString[(int)whosTurn]);
	printf("LEGEND:  ( 4 5 6 )  TOTAL:   : %s %s %s\n",
	       gBlankOXString[(int)theBlankOx[3]],
	       gBlankOXString[(int)theBlankOx[4]],
	       gBlankOXString[(int)theBlankOx[5]] );
	printf("         ( 7 8 9 )           : %s %s %s %s\n\n",
	       gBlankOXString[(int)theBlankOx[6]],
	       gBlankOXString[(int)theBlankOx[7]],
	       gBlankOXString[(int)theBlankOx[8]],
	       GetPrediction(position,playerName,usersTurn));

}

/************************************************************************
**
** NAME:        GenerateMoves
**
** DESCRIPTION: Create a linked list of every move that can be reached
**              from this position. Return a pointer to the head of the
**              linked list.
**
** INPUTS:      POSITION position : The position to branch off of.
**
** OUTPUTS:     (MOVELIST *), a pointer that points to the first item
**              in the linked list of moves that can be generated.
**
** CALLS:       GENERIC_PTR SafeMalloc(int)
**
************************************************************************/

MOVELIST *GenerateMoves(POSITION position) {
	MOVELIST *head = NULL;
	BlankOX theBlankOX[BOARDSIZE], whosTurn;
	int i,j; /* Values for J: 0=left,1=straight,2=right */

	PositionToBlankOX(position,theBlankOX,&whosTurn);

	for(i = 0; i < BOARDSIZE; i++) { /* enumerate over all FROM slots */
		for(j = 0; j < 3; j++) { /* enumerate over all directions */
			if(OkMove(theBlankOX,whosTurn,(SLOT)i,j))
				head = CreateMovelistNode(SlotsToMove((SLOT)i,GetToSlot(i,j,whosTurn)),head);
		}
	}
	return(head);
}

BOOLEAN OkMove(BlankOX *theBlankOX, BlankOX whosTurn, SLOT fromSlot, int direction) {
	SLOT toSlot;
	toSlot = GetToSlot(fromSlot,direction,whosTurn);
	return((theBlankOX[fromSlot] == whosTurn) &&
	       (toSlot != BADSLOT) &&
	       (theBlankOX[toSlot] == Blank || toSlot == OFFTHEBOARD));
}

SLOT GetToSlot(SLOT fromSlot, int direction, BlankOX whosTurn) {
	if((fromSlot < 3 && whosTurn == x && direction == 1) ||
	   (((fromSlot % 3) == 2) && whosTurn == o && direction == 1))
		return(OFFTHEBOARD);
	else if (((whosTurn == x) &&
	          ((direction == 2 && ((fromSlot % 3) == 2)) ||
	           (direction == 0 && ((fromSlot % 3) == 0)))) ||
	         ((whosTurn == o) &&
	          ((direction == 2 && (fromSlot > 5)) ||
	     (direction == 0 && (fromSlot < 3)))))
		return(BADSLOT);

	/** I used this diagram to reference the following.
	**
	** 0 1 2   0            1
	** 3 4 5  <O> 1      0 <X> 2
	** 6 7 8   2
	*/

	else if ((whosTurn == o && direction == 1) ||
	         (whosTurn == x && direction == 2)) /* east */
		return (fromSlot + 1);
	else if ((whosTurn == o && direction == 0) ||
	         (whosTurn == x && direction == 1)) /* north */
		return (fromSlot - 3);
	else if (whosTurn == o && direction == 2)   /* south */
		return (fromSlot + 3);
	else if (whosTurn == x && direction == 0)   /* west */
		return (fromSlot - 1);
	else {
		BadElse("GetToSlot");
		return(BADSLOT); /* Control never gets here, but lint is happy */
	}
}

/************************************************************************
**
** NAME:        GetAndPrintPlayersMove
**
** DESCRIPTION: This finds out if the player wanted an undo or abort or not.
**              If so, return Undo or Abort and don't change theMove.
**              Otherwise get the new theMove and fill the pointer up.
**
** INPUTS:      POSITION *thePosition : The position the user is at.
**              MOVE *theMove         : The move to fill with user's move.
**              STRING playerName     : The name of the player whose turn it is
**
** OUTPUTS:     USERINPUT             : Oneof( Undo, Abort, Continue )
**
** CALLS:       ValidMove(MOVE, POSITION)
**              BOOLEAN PrintPossibleMoves(POSITION) ...Always True!
**
************************************************************************/

USERINPUT GetAndPrintPlayersMove(POSITION thePosition, MOVE *theMove, STRING playerName) {
	USERINPUT ret;

	do {
		printf("%8s's move [(u)ndo/1-9] : ", playerName);

		ret = HandleDefaultTextInput(thePosition, theMove, playerName);
		if(ret != Continue)
			return(ret);
	}
	while (TRUE);
	return(Continue); /* this is never reached, but lint is now happy */
}

/************************************************************************
**
** NAME:        ValidTextInput
**
** DESCRIPTION: Return TRUE iff the string input is of the right 'form'.
**              For example, if the user is allowed to select one slot
**              from the numbers 1-9, and the user chooses 0, it's not
**              valid, but anything from 1-9 IS, regardless if the slot
**              is filled or not. Whether the slot is filled is left up
**              to another routine.
**
** INPUTS:      STRING input : The string input the user typed.
**
** OUTPUTS:     BOOLEAN : TRUE iff the input is a valid text input.
**
************************************************************************/

BOOLEAN ValidTextInput(STRING input) {
	char fromChar, toChar;
	SLOT fromSlot, toSlot;
	int ret;
	//   ret = sscanf(input,"%d %d", &fromSlot, &toSlot);
	ret = sscanf(input,"%c%c", &fromChar, &toChar);
	fromSlot = fromChar - '0';
	toSlot   = toChar   - '0';
	return(ret == 2 &&
	       fromSlot <= 9 && fromSlot >= 1 && toSlot <= 9 && toSlot >= 0);
}

/************************************************************************
**
** NAME:        ConvertTextInputToMove
**
** DESCRIPTION: Convert the string input to the internal move representation.
**              No checking if the input is valid is needed as it has
**              already been checked!
**
** INPUTS:      STRING input : The string input the user typed.
**
** OUTPUTS:     MOVE : The move corresponding to the user's input.
**
************************************************************************/

MOVE ConvertTextInputToMove(STRING input) {
	SLOT fromSlot, toSlot;
	char fromChar, toChar;
	(void)sscanf(input,"%c%c", &fromChar, &toChar);
	fromSlot = fromChar - '0';
	toSlot   = toChar   - '0';

	fromSlot--;         /* user input is 1-9, our rep. is 0-8 */
	if(toSlot == 0)
		toSlot = OFFTHEBOARD; /* '0' = offtheboard, convert to OFFTHEBOARD */
	else
		toSlot--; /* user input is 1-9, our rep. is 0-8 */

	return(SlotsToMove(fromSlot,toSlot));
}

void MoveToString(MOVE theMove, char *moveStringBuffer) {
	SLOT fromSlot, toSlot;
	MoveToSlots(theMove,&fromSlot,&toSlot);
	snprintf(moveStringBuffer, 10, "%d%d", fromSlot + 1, toSlot==9 ? 0 : toSlot + 1);
}

/************************************************************************
*************************************************************************
**         EVERYTHING BELOW THESE LINES IS LOCAL TO THIS FILE
*************************************************************************
************************************************************************/

/************************************************************************
**
** NAME:        PositionToBlankOX
**
** DESCRIPTION: convert an internal position to that of a BlankOX.
**
** INPUTS:      POSITION thePos     : The position input.
**              BlankOX *theBlankOx : The converted BlankOX output array.
**
** CALLS:       BadElse()
**
************************************************************************/

void PositionToBlankOX(POSITION thePos, BlankOX *theBlankOX, BlankOX *whosTurn) {
	int i;

	if (thePos >= POSITION_OFFSET) { /* X moves first <==> pos >= offset */
		*whosTurn = x;
		thePos -= POSITION_OFFSET;
	}
	else
		*whosTurn = o;

	for(i = 8; i >= 0; i--) {
		if(thePos >= (POSITION)(x * g3Array[i])) {
			theBlankOX[i] = x;
			thePos -= x * g3Array[i];
		}
		else if(thePos >= (POSITION)(o * g3Array[i])) {
			theBlankOX[i] = o;
			thePos -= o * g3Array[i];
		}
		else if(thePos >= (POSITION)(Blank * g3Array[i])) {
			theBlankOX[i] = Blank;
			thePos -= Blank * g3Array[i];
		}
		else
			BadElse("PositionToBlankOX");
	}
}

/************************************************************************
**
** NAME:        MoveToSlots
**
** DESCRIPTION: convert an internal move to that of two slots
**
** INPUTS:      MOVE theMove    : The move input.
**              SLOT *fromSlot  : The slot the piece moves from (output)
**              SLOT *toSlot    : The slot the piece moves to   (output)
**
************************************************************************/

void MoveToSlots(MOVE theMove, SLOT *fromSlot, SLOT *toSlot) {
	*fromSlot = theMove % (BOARDSIZE+1);
	*toSlot   = theMove / (BOARDSIZE+1);
}

/************************************************************************
**
** NAME:        SlotsToMove
**
** DESCRIPTION: convert two slots (from and to) to an encoded MOVE
**
** INPUT:       SLOT fromSlot   : The slot the piece moves from (0->BOARDSIZE)
**              SLOT toSlot     : The slot the piece moves to   (0->BOARDSIZE)
**
** OUTPUT:      MOVE            : The move corresponding to from->to
**
************************************************************************/

MOVE SlotsToMove(SLOT fromSlot, SLOT toSlot) {
	return ((MOVE) toSlot*(BOARDSIZE+1) + fromSlot);
}

/************************************************************************
**
** NAME:        BlankOXToPosition
**
** DESCRIPTION: convert a BlankOX to that of an internal position.
**
** INPUTS:      BlankOX *theBlankOx : The converted BlankOX output array.
**
** OUTPUTS:     POSITION: The equivalent position given the BlankOX.
**
************************************************************************/

POSITION BlankOXToPosition(BlankOX *theBlankOX, BlankOX whosTurn) {
	int i;
	POSITION position = 0;

	for(i = 0; i < BOARDSIZE; i++)
		position += g3Array[i] * (int)theBlankOX[i];

	if(whosTurn == x)
		position += POSITION_OFFSET; /* account for whose turn it is */

	return(position);
}

int NumberOfOptions() {
	return 4;
}

int getOption() {
	if(gStandardGame)
	{
		if(gToTrapIsToWin)
			return 1;
		else
			return 2;
	}
	else
	{
		if(gToTrapIsToWin)
			return 3;
		else
			return 4;
	}
}

void setOption(int option) {
	if(option == 1) {
		gStandardGame = TRUE;
		gToTrapIsToWin = TRUE;
	}
	else if(option == 2) {
		gStandardGame = TRUE;
		gToTrapIsToWin = FALSE;
	}
	else if(option == 3) {
		gStandardGame = FALSE;
		gToTrapIsToWin = TRUE;
	}
	else {
		gStandardGame = FALSE;
		gToTrapIsToWin = FALSE;
	}
}

POSITION StringToPosition(char *positionString) {
	int turn;
	char *board;
	if (ParseStandardOnelinePositionString(positionString, &turn, &board)) {
		POSITION position = 0;
		for (int i = 0; i < BOARDSIZE; i += 1) {
			int offset = i / 3 + 4;
			if (board[i + offset] == 'x') {
				position += g3Array[i] * (int) x;
			} else if (board[i + offset] == 'o') {
				position += g3Array[i] * (int) o;
			} else {
				position += g3Array[i] * (int) Blank;
			}
		}
		// Player X in Dodgem is Player 2
		if (turn == 2) {
			position += POSITION_OFFSET;
		}
		return position;
	}
	return NULL_POSITION;
}

void PositionToAutoGUIString(POSITION position, char *autoguiPositionStringBuffer) {
	char oxboard[INTERACT_BOARDSIZE + 1];
	int turn;
	if (position >= POSITION_OFFSET) {
		turn = 2;
		position -= POSITION_OFFSET;
	} else {
		turn = 1;
	}

	for (int i = BOARDSIZE - 1; i >= 0; i -= 1) {
		// 12	13	 14	  15
		// 8    9    10   11
		// (6)  (7)  (8)      <-- Indices in a 3x3 board
		// 4    5    6    7
		// (3)  (4)  (5)      <-- Indices in a 3x3 board
		// 0    1    2    3 
		// Therefore, we need to add offset of i/4 when putting it onto a 4x4 board
		int offset = i / 3 + 4;
		if (position >= (POSITION)(x * g3Array[i])) {
			oxboard[i + offset] = 'x';
			position -= x * g3Array[i];
		} else if (position >= (POSITION)(o * g3Array[i])) {
			oxboard[i + offset] = 'o';
			position -= o * g3Array[i];
		} else if (position >= (POSITION)(Blank * g3Array[i])) {
			oxboard[i + offset] = '-';
			position -= Blank * g3Array[i];
		} else {
			BadElse("PositionToAutoGUIString");
		}
	}
	for (int j = 0; j < INTERACT_BOARDSIZE; j += 1) {
		if (j % 4 == 3 || j / 4 == 0) {
			oxboard[j] = '-';
		}
	}
	oxboard[INTERACT_BOARDSIZE] = '\0';
	AutoGUIMakePositionString(turn, oxboard, autoguiPositionStringBuffer);
}

void MoveToAutoGUIString(POSITION position, MOVE move, char *autoguiMoveStringBuffer) {
	(void) position;
	int turn = 0;
	if (position >= POSITION_OFFSET) { // Player X i.e. Player 2
		turn = 2;
	} else { // Player O i.e. Player 1
		turn = 1;
	}

	SLOT fromSlot, toSlot;
	MoveToSlots(move, &fromSlot, &toSlot);

	fromSlot = fromSlot / 3 + 4 + fromSlot;
	if (toSlot == OFFTHEBOARD) {
		if (fromSlot / 4 == 1 || fromSlot % 4 == 2) {
			// If the piece is moving off the board from the top row
			if (turn == 1) {
				toSlot = fromSlot + 1;
			} else {
				toSlot = fromSlot - 4;
			}
		}
	} else {
		toSlot = toSlot / 3 + 4 + toSlot;
	}
	AutoGUIMakeMoveButtonStringM(fromSlot, toSlot, 'x', autoguiMoveStringBuffer);
}