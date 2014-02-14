
/************************************************************************
**
** NAME:        mchange.c
**
** DESCRIPTION: Change!
**
** AUTHOR:      Alice Chang & Judy Tuan  -  University of California at Berkeley
**              Copyright (C) Alice Chang & Judy Tuan, 2002. All rights reserved.
**
** DATE:        2002-10-08
**
** UPDATE HIST: 2004-04-14 Upgraded to work with Gamesman3
**
**
**
**************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/

#include <stdio.h>
#include "gamesman.h"

/* The beginning position is:
**
**     O   O   O          1   2   3      which corresponds to 1*(3^0) +
**    / \ / \ / \        / \ / \ / \     1*(3^1) + 1*(3^2) + 2*(3^11) +
**   -   -   -   -      4   5   6   7    2*(3^12) + 2*(3^13) = 4605835
**  / \ / \ / \ /      / \ / \ / \ /
** -   -   -   -      8   9   10  11
**  \ / \ / \ /        \ / \ / \ /
**   X   X   X          12  13  14
**
*/

POSITION gNumberOfPositions  = 9565938;  /* 3^14 times 2 */

POSITION gInitialPosition    = 9388804; /* x goes first. 4605835 + POSITION_OFFSET */
POSITION kBadPosition        = -1; /* This can never be the rep. of a position */

STRING kAuthorName         = "Alice Chang and Judy Tuan";
STRING kGameName           = "Change!";
STRING kDBName             = "change";
BOOLEAN kPartizan           = TRUE;
BOOLEAN kDebugMenu          = TRUE;
BOOLEAN kGameSpecificMenu   = TRUE;
BOOLEAN kTieIsPossible      = FALSE;
BOOLEAN kLoopy               = FALSE;
BOOLEAN kDebugDetermineValue = FALSE;
void*    gGameSpecificTclInit = NULL;

STRING kHelpGraphicInterface =
        "The LEFT button puts an X or O (depending on whether you went first\n\
or second) on the spot the cursor was on when you clicked. The MIDDLE\n\
button does nothing, and the RIGHT button is the same as UNDO, in that\n\
it reverts back to your most recent position."                                                                                                                                                                                                                                   ;

STRING kHelpTextInterface    =
        "Players alternate turns moving one of their pieces along the lines. First\n\
select the piece by typing in the number the piece is in. Then select where\n\
you want to move the piece by typing in the corresponding space. Pieces may\n\
not move backward, jump pieces or turn corners "                                                                                                                                                                                                                                                    ;

STRING kHelpOnYourTurn =
        "Slide your piece to an empty space by first selecting your piece number\n\
the space number."                                                                                    ;

STRING kHelpStandardObjective =
        "To be the first player to occupy your opponent's spaces.";

STRING kHelpReverseObjective =
        "To force your opponent into getting three of his markers (either X or\n\
O) in a row, either horizontally, vertically, or diagonally. 3-in-a-row\n\
LOSES."                                                                                                                                                             ;

STRING kHelpTieOccursWhen =   /* Should follow 'A Tie occurs when... */
                            "the board fills up without either player getting three-in-a-row.";

STRING kHelpExample =
        "         ( 1 2 3 )           : - - -\n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - - - \n\
         ( 7 8 9 )           : - - - \n\n\
Computer's move              :  3    \n\n\
         ( 1 2 3 )           : - - X \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - - - \n\
         ( 7 8 9 )           : - - - \n\n\
     Dan's move [(u)ndo/1-9] : { 2 } \n\n\
         ( 1 2 3 )           : - O X \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - - - \n\
         ( 7 8 9 )           : - - - \n\n\
Computer's move              :  6    \n\n\
         ( 1 2 3 )           : - O X \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - - X \n\
         ( 7 8 9 )           : - - - \n\n\
     Dan's move [(u)ndo/1-9] : { 9 } \n\n\
         ( 1 2 3 )           : - O X \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - - X \n\
         ( 7 8 9 )           : - - O \n\n\
Computer's move              :  5    \n\n\
         ( 1 2 3 )           : - O X \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - X X \n\
         ( 7 8 9 )           : - - O \n\n\
     Dan's move [(u)ndo/1-9] : { 7 } \n\n\
         ( 1 2 3 )           : - O X \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - X X \n\
         ( 7 8 9 )           : O - O \n\n\
Computer's move              :  4    \n\n\
         ( 1 2 3 )           : - O X \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : X X X \n\
         ( 7 8 9 )           : O - O \n\n\
Computer wins. Nice try, Dan."                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             ;

STRING MoveToString(MOVE);

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

#define BOARDSIZE     14          /* 2x3 staggered-diamonds-shaped board */
#define POSITION_OFFSET 4782969   /* 3^14 */
#define BADSLOT -2                /* slot does not exist */

typedef enum possibleBoardPieces {
	Blank, o, x
} BlankOX;

typedef int SLOT;    /* A slot is the place where a piece moves from or to*/

char *gBlankOXString[] = { "-", "O", "X" };

/* Powers of 3 - this is the way I encode the position, as an integer */
int g3Array[] = { 1, 3, 9, 27, 81, 243, 729, 2187, 6561, 19683, 59049,
	          177147, 531441, 1594323 };

BOOLEAN gToTrapIsToWin = TRUE;  /* Being stuck means you lose. */

/* local function prototypes */
void PositionToBlankOX(POSITION, BlankOX*, BlankOX*);
POSITION BlankOXToPosition(BlankOX*, BlankOX);
BOOLEAN OkMove(BlankOX*, BlankOX, SLOT, int, int, SLOT*);
BOOLEAN FullSlots(BlankOX[]);
BOOLEAN Trapped(POSITION);
void MoveToSlots(MOVE, SLOT*, SLOT*);
SLOT GetToSlot(BlankOX, SLOT, int, int);
MOVE SlotsToMove(SLOT fromSlot, SLOT toSlot);


/************************************************************************
**
** NAME:        InitializeGame
**
** DESCRIPTION: Initializes the internal game variables
**
************************************************************************/

void InitializeGame() {
	gMoveToStringFunPtr = &MoveToString;
}


/************************************************************************
**
** NAME:        DebugMenu
**
** DESCRIPTION: Menu used to debug internal problems. Does nothing if
**              kDebugMenu == FALSE
**
************************************************************************/

void DebugMenu()
{
}

/************************************************************************
**
** NAME:        GameSpecificMenu
**
** DESCRIPTION: Menu used to change game-specific parameters, such as
**              the side of the board in an nxn Nim board, etc. Does
**              nothing if kGameSpecificMenu == FALSE
**
************************************************************************/

void GameSpecificMenu() {
	do {
		printf("\n\t----- Game-specific options for %s -----\n\n", kGameName);

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
		case 'H': case 'h':
			HelpMenus();
			break;
		case 'I': case 'i':
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

void SetTclCGameSpecificOptions(int theOptions[])
{
	/* No need to have anything here, we have no extra options */
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
** CALLS:       PositionToBlankOX(POSITION,*BlankOX)
**              BlankOX WhosTurn(*BlankOX)
**
************************************************************************/

POSITION DoMove(POSITION thePosition, MOVE theMove)
{
	SLOT fromSlot, toSlot;
	BlankOX theBlankOX[BOARDSIZE], whosTurn;

	PositionToBlankOX(thePosition,theBlankOX,&whosTurn);
	MoveToSlots(theMove, &fromSlot, &toSlot);

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
** OUTPUTS:     POSITION initialPosition : The position to fill.
**
************************************************************************/

POSITION GetInitialPosition()
{
	BlankOX theBlankOX[BOARDSIZE], whosTurn;
	signed char c;
	int i;


	printf("\n\n\t----- Get Initial Position -----\n");
	printf("\n\tPlease input the position to begin with.\n");
	printf("\tNote that it should be in the following format:\n\n");
	printf("    O   O   O\n");
	printf("   / \\ / \\ / \\\n");
	printf("  -   -   -   -\n");
	printf(" / \\ / \\ / \\ /         <---- EXAMPLE \n");
	printf("-   -   -   -\n");
	printf(" \\ / \\ / \\ /\n");
	printf("  X   X   X\n");
	printf("Type this: \nO O O\n- - - -\n- - - -\nX X X\n");

	i = 0;
	// getchar();
	while(i < BOARDSIZE && (c = getchar()) != EOF) {
		if(c == 'x' || c == 'X')
			theBlankOX[i++] = x;
		else if(c == 'o' || c == 'O' || c == '0')
			theBlankOX[i++] = o;
		else if(c == '-')
			theBlankOX[i++] = Blank;
		else
			; /* do nothing */
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

void PrintComputersMove(MOVE computersMove,STRING computersName)
{
	SLOT fromSlot,toSlot;
	MoveToSlots(computersMove,&fromSlot,&toSlot);
	printf("%8s's move              : %d %d\n",computersName,
	       fromSlot+1,toSlot+1);
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
** CALLS:       PositionToBlankOX()
**
************************************************************************/

VALUE Primitive(POSITION position)
{
	BlankOX theBlankOX[BOARDSIZE], whosTurn;

	PositionToBlankOX(position,theBlankOX,&whosTurn);

	if(FullSlots(theBlankOX))
		return(gStandardGame ? lose : win);
	else if(Trapped(position))
		return(gToTrapIsToWin ? lose : win);
	else
		return(undecided);
}

BOOLEAN FullSlots(theBlankOX)
BlankOX theBlankOX[];
{
	return((theBlankOX[0] == theBlankOX[1] &&
	        theBlankOX[1] == theBlankOX[2] &&
	        theBlankOX[2] == x) ||
	       (theBlankOX[11] == theBlankOX[12] &&
	        theBlankOX[12] == theBlankOX[13] &&
	        theBlankOX[13] == o));
}

BOOLEAN Trapped(POSITION position)
{
	MOVELIST *ptr;
	BOOLEAN trapped;

	ptr = GenerateMoves(position);
	trapped = (ptr == NULL);
	FreeMoveList(ptr);
	return(trapped);
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

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn)
{
	BlankOX theBlankOx[BOARDSIZE], whosTurn;

	PositionToBlankOX(position,theBlankOx,&whosTurn);

	printf("\n             1   2   3                  %s   %s   %s\n",
	       gBlankOXString[(int)theBlankOx[0]],
	       gBlankOXString[(int)theBlankOx[1]],
	       gBlankOXString[(int)theBlankOx[2]] );
	printf("            / \\ / \\ / \\                / \\ / \\ / \\\n");
	printf("           4   5   6   7              %s   %s   %s   %s    Player %s's turn\n",
	       gBlankOXString[(int)theBlankOx[3]],
	       gBlankOXString[(int)theBlankOx[4]],
	       gBlankOXString[(int)theBlankOx[5]],
	       gBlankOXString[(int)theBlankOx[6]],
	       gBlankOXString[(int)whosTurn]);
	printf("          / \\ / \\ / \\ /              / \\ / \\ / \\ /\n");
	printf("LEGEND:  8   9   10  11     TOTAL:  %s   %s   %s   %s\n",
	       gBlankOXString[(int)theBlankOx[7]],
	       gBlankOXString[(int)theBlankOx[8]],
	       gBlankOXString[(int)theBlankOx[9]],
	       gBlankOXString[(int)theBlankOx[10]] );
	printf("          \\ / \\ / \\ /                \\ / \\ / \\ /\n");
	printf("           12  13  14                 %s   %s   %s        %s\n\n",
	       gBlankOXString[(int)theBlankOx[11]],
	       gBlankOXString[(int)theBlankOx[12]],
	       gBlankOXString[(int)theBlankOx[13]],
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
** CALLS:       MOVELIST *CreateMovelistNode(MOVE,MOVELIST)
**
*************************************************************************/


MOVELIST *GenerateMoves(position)
POSITION position;
{
	MOVELIST *head = NULL;
	BlankOX theBlankOX[BOARDSIZE], whosTurn;
	SLOT i, toSlot;

	int j,k; /* i is which FROM slot to start from
	          * j is direction
	          * k is number of slots to slide
	          * values for j: 0="left", 1="right"
	          * 0   1     O
	          *  \ /     / \
	          *   X     1   0
	          */

	PositionToBlankOX(position, theBlankOX, &whosTurn);

	for(i = 0; i < BOARDSIZE; i++) { /*enumerate over all FROM slots*/
		for(j = 0; j < 2; j++) { /* enumerate over all directions */
			k = 1;
			while((k < 4) && OkMove(theBlankOX, whosTurn, i, j, k, &toSlot)) {
				head = CreateMovelistNode(SlotsToMove(i, toSlot),head);
				k++;
			}
		}
	}
	return(head);
}


BOOLEAN OkMove(theBlankOx, whosTurn, fromSlot, direction, slide, toSlot)
BlankOX *theBlankOx, whosTurn;
SLOT fromSlot;
int direction, slide;
SLOT *toSlot;
{
	*toSlot = GetToSlot(whosTurn, fromSlot, direction, slide);
	return((theBlankOx[fromSlot] == whosTurn) &&
	       (*toSlot != BADSLOT) &&
	       (theBlankOx[*toSlot] == Blank));
}

SLOT GetToSlot(BlankOX whosTurn, SLOT fromSlot, int direction, int slide)
{
//here, a bunch of else ifs where you return the toSlot slot
	if(whosTurn == o) {
		if((fromSlot == 6 && direction == 0) ||
		   (fromSlot == 7 && direction == 1) ||
		   (fromSlot == 10 && direction == 0) ||
		   (fromSlot > 10)) //if fromSlot is in farthest row
			return(BADSLOT);
		if((slide == 2) &&
		   ((fromSlot > 6) || //if you can't slide 2 (or 3)
		    (fromSlot == 2 && direction == 0) ||
		    (fromSlot == 3 && direction == 1) ||
		    (fromSlot == 5 && direction == 0)))
			return(BADSLOT);
		if((slide == 3) &&
		   ((fromSlot > 2) ||
		    (fromSlot == 0 && direction == 1) ||
		    (fromSlot == 1 && direction == 0)))
			return(BADSLOT);
		if(direction == 0) {
			if(slide == 3)
				return(fromSlot + 13);
			if(slide == 2)
				return(fromSlot + 9);

			if(slide == 1) {
				if((fromSlot > 2) && (fromSlot < 7))
					return(fromSlot + 5);
				else return(fromSlot + 4);
			}

		}
		//else, the direction = 1. (which means "right")
		if(slide == 1) {
			if((fromSlot > 2) && (fromSlot < 7))
				return(fromSlot + 4);
			else return(fromSlot + 3);
		}
		if(slide == 2)
			return(fromSlot + 7);
		if(slide == 3)
			return(fromSlot + 10);
	} //ends if(whosTurn == o) !!!!!

	else if(whosTurn == x) {
		if((fromSlot == 7 && direction == 0) ||
		   (fromSlot == 6 && direction == 1) ||
		   (fromSlot == 3 && direction == 0) ||
		   (fromSlot < 3)) //if fromSlot is in farthest row
			return(BADSLOT);
		if((slide == 2) &&
		   ((fromSlot < 7) || //can't slide two (or three, for that matter)
		    (fromSlot == 10 && direction == 1) ||
		    (fromSlot == 11 && direction == 0) ||
		    (fromSlot == 8 && direction == 0)))
			return(BADSLOT);
		if((slide == 3) &&
		   ((fromSlot < 11) || //can't slide three
		    (fromSlot == 12 && direction == 0) ||
		    (fromSlot == 13 && direction == 1)))
			return(BADSLOT);
		if(direction == 0) { //"left"
			if(slide == 1) {
				if(fromSlot < 11 && fromSlot > 6)
					return(fromSlot - 5);
				else
					return(fromSlot - 4);
			}
			if(slide == 2)
				return(fromSlot - 9);
			if(slide == 3)
				return(fromSlot - 13);
		}
		//else, the direction is 1 ("right")
		if(slide == 1) {
			if(fromSlot < 11 && fromSlot > 6)
				return(fromSlot - 4);
			else
				return(fromSlot - 3);
		}
		if(slide == 2)
			return(fromSlot - 7);
		if(slide == 3)
			return(fromSlot - 10);
	} //ends if(whosTurn == x) !!!!!!!!!!!!!!!
	BadElse("GetToSlot"); //if it doesn't return anything, there's a PROBLEM

	//should never reach here
	return 0;
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

USERINPUT GetAndPrintPlayersMove(thePosition, theMove, playerName)
POSITION thePosition;
MOVE *theMove;
STRING playerName;
{
	USERINPUT ret;

	do {
		printf("%8s's move [(u)ndo/1-14  1-14] :  ", playerName);

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

BOOLEAN ValidTextInput(input)
STRING input;
{
	SLOT fromSlot, toSlot;
	int text;
	text = sscanf(input, "%d %d", &fromSlot, &toSlot);
	return(text == 2 && fromSlot <= 14 && fromSlot >= 1 && toSlot <= 14 &&
	       toSlot >= 1);
}

/************************************************************************
**
** NAME:        ConvertTextInputToMove
**
** DESCRIPTION: Convert the string input to the internal move representation.
**
** INPUTS:      STRING input : The string input the user typed.
**
** OUTPUTS:     MOVE : The move corresponding to the user's input.
**
************************************************************************/

MOVE ConvertTextInputToMove(input)
STRING input;
{
	SLOT fromSlot, toSlot;
	int text;
	text = sscanf(input, "%d %d", &fromSlot, &toSlot);

	fromSlot--;
	toSlot--;

	return(SlotsToMove(fromSlot,toSlot));
}

/************************************************************************
**
** NAME:        PrintMove
**
** DESCRIPTION: Print the move in a nice format.
**
** INPUTS:      MOVE *theMove         : The move to print.
**
************************************************************************/

void PrintMove(theMove)
MOVE theMove;
{
	printf( "%s", MoveToString(theMove) );
}

/************************************************************************
**
** NAME:        MoveToString
**
** DESCRIPTION: Returns the move as a STRING
**
** INPUTS:      MOVE *theMove         : The move to put into a string.
**
************************************************************************/

STRING MoveToString (theMove)
MOVE theMove;
{
	STRING move = (STRING) SafeMalloc(6);

	SLOT fromSlot, toSlot;
	MoveToSlots(theMove,&fromSlot,&toSlot);
/* The plus 1 is because the user thinks it's 1-9, but MOVE is 0-8 */
	sprintf(move, "[%d %d]", fromSlot+1, toSlot+1);

	return move;
}


int NumberOfOptions() {
	return 2*2;
}


int getOption() {
	int option = 1;
	option += gStandardGame ? 0 : 1;
	option += 2*(gToTrapIsToWin ? 1 : 0);
	return option;
}


void setOption(int option) {
	option--;
	gStandardGame = (option%2==0);
	gToTrapIsToWin = (option/2%2==1);
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

void PositionToBlankOX(thePos,theBlankOX,whosTurn)
POSITION thePos;
BlankOX *theBlankOX, *whosTurn;
{
	int i;

	if(thePos >= POSITION_OFFSET) {
		*whosTurn = x;
		thePos -= POSITION_OFFSET;
	}
	else *whosTurn = o;

	for(i = (BOARDSIZE - 1); i >= 0; i--) {
		if(thePos >= ((int)x * g3Array[i])) {
			theBlankOX[i] = x;
			thePos -= (int)x * g3Array[i];
		}
		else if(thePos >= ((int)o * g3Array[i])) {
			theBlankOX[i] = o;
			thePos -= (int)o * g3Array[i];
		}
		else if(thePos >= ((int)Blank * g3Array[i])) {
			theBlankOX[i] = Blank;
			thePos -= (int)Blank * g3Array[i];
		}
		else
			BadElse("PositionToBlankOX");
	}
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

POSITION BlankOXToPosition(theBlankOX, whosTurn)
BlankOX *theBlankOX, whosTurn;
{
	int i;
	POSITION position = 0;

	for(i = 0; i < BOARDSIZE; i++)
		position += g3Array[i] * (int)theBlankOX[i];

	if(whosTurn == x)
		position += POSITION_OFFSET;

	return(position);
}

void MoveToSlots(theMove, fromSlot, toSlot)
MOVE theMove;
SLOT *fromSlot, *toSlot;
{
	*fromSlot = theMove % (BOARDSIZE+1);
	*toSlot = theMove / (BOARDSIZE+1);
}

MOVE SlotsToMove(fromSlot, toSlot)
SLOT fromSlot, toSlot;
{
	return((MOVE)toSlot*(BOARDSIZE+1) + fromSlot);
}







POSITION StringToPosition(char* board) {
	// FIXME: this is just a stub
	return atoi(board);
}


char* PositionToString(POSITION pos) {
	// FIXME: this is just a stub
	return "Implement Me";
}

//GM_DEFINE_BLANKOX_ENUM_BOARDSTRINGS()

char * PositionToEndData(POSITION pos) {
	return NULL;
}
