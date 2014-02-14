/************************************************************************
**
** NAME:        m1ton.c
**
** DESCRIPTION: The 1,2,...N game
**
** AUTHOR:      Dan Garcia  -  University of California at Berkeley
**              Copyright (C) Dan Garcia, 1995. All rights reserved.
**
** DATE:        08/25/91
**
** UPDATE HIST:
**
**  9-04-91 1.0a1 : Added <crs> to PrintPosition and added call
**                  to ValidMove() and PrintPossibleMoves()
**  9-06-91 1.0a2 : Added the two extra arguments to PrintPosition
**                  Recoded the way to do "visited" - bitmask
**  9-16-91 1.0a6 : Replaced redundant with GetRawValueFromDatabase
**  5-12-92 1.0a7 : Added Static Evaluation - it's perfect!
** 05-15-95 1.0   : Final release code for M.S.
**
**************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/

#include <stdio.h>
#include "gamesman.h"

#define MAXN     30
#define MAXM     3

unsigned int N = 10;
unsigned int gTurn = 2;

extern STRING gValueString[];

POSITION gNumberOfPositions = 11;       /* Initized to 11 */
POSITION kBadPosition = -1;

POSITION gInitialPosition    = 0;

POSITION gMinimalPosition = 0;

STRING kAuthorName          = "Dan Garcia";
STRING kGameName            = "1 TO N";
BOOLEAN kPartizan            = FALSE;
BOOLEAN kDebugMenu           = FALSE;
BOOLEAN kGameSpecificMenu    = TRUE;
BOOLEAN kTieIsPossible       = FALSE;
BOOLEAN kLoopy               = FALSE;
BOOLEAN kDebugDetermineValue = FALSE;
void*    gGameSpecificTclInit = NULL;

STRING kHelpGraphicInterface = "";    /* empty since kSupportsGraphics == FALSE */

STRING kHelpTextInterface    =
        "On your turn, select the number 1 or 2 to raise the total sum. After\n\
selecting your move, the total is displayed. The play then alternates to the\n\
second player, who also has the choice of raising the sum by 1 or 2 points.\n\
The winneris the first person to raise the total sum to N. If at any point\n\
you have made a mistake, type u to revert back to your previous position."                                                                                                                                                                                                                                                                                                                              ;

STRING kHelpOnYourTurn =
        "Select the number 1 or 2 to raise the total sum. You can also revert to the\n\
previous position by typing u."                                                                                        ;

STRING kHelpStandardObjective =
        "To be the player to raise the total to N.";

STRING kHelpReverseObjective =
        "To force the other player to raise the total to N.";

STRING kHelpTieOccursWhen = "";   /* empty since kTieIsPossible == FALSE */

STRING kHelpExample =
        "TOTAL                        :  0   \n\n\
     Dan's move [(u)ndo/1/2] : { 2 } \n\n\
TOTAL                        :  2    \n\n\
Computer's move              :  2    \n\n\
TOTAL                        :  4    \n\n\
     Dan's move [(u)ndo/1/2] : { 1 } \n\n\
TOTAL                        :  5    \n\n\
Computer's move              :  2    \n\n\
TOTAL                        :  7    \n\n\
     Dan's move [(u)ndo/1/2] : { 2 } \n\n\
TOTAL                        :  9    \n\n\
Computer's move              :  2    \n\n\
TOTAL                        : 11    \n\n\
Computer wins. Nice try, Dan."                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       ;

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

STRING gGameSpecificMenu = "1.\tSet the value of N\n  \t(the target ending sum.\n\n2.\tSet the highest number you can add\n  \tto the running sum.\n\n3.\tReturn to previous menu.\n\nSelect option: ";

void InitializeGame()
{
	gNumberOfPositions = N + 1;
	gMoveToStringFunPtr = &MoveToString;
}

/************************************************************************
**
** NAME:        DebugMenu
**
** DESCRIPTION: Menu used to debub internal problems. Does nothing if
**              kDebugMenu == FALSE
**
************************************************************************/

void DebugMenu() {
}

void changeBoard()
{
	unsigned int boardSize;
	printf("Enter the new N:  ");
	boardSize = GetMyUInt();
	N = boardSize;
	gNumberOfPositions = N + 1;
}

void changeMove()
{
	unsigned int turnSize;
	printf("Enter the new turn size:  ");
	turnSize = GetMyUInt();
	gTurn = turnSize;
}

/************************************************************************
**
** NAME:        GameSpecificMenu
**
** DESCRIPTION: Menu used to change game-specific parmeters, such as
**              the side of the board in an nxn Nim board, etc. Does
**              nothing if kGameSpecificMenu == FALSE
**
************************************************************************/

void GameSpecificMenu()
{
	int selection;
	printf("%s", gGameSpecificMenu);
	selection = GetMyInt();
	if(selection == 1)
		changeBoard();
	else if(selection == 2)
		changeMove();
	else if(selection == 3)
		return;
	else {
		printf("\n\n\nInvalid Entry, please try again...");
		GameSpecificMenu();
	}
}

/************************************************************************
**
** NAME:        SetTclCGameSpecificOptions
**
** DESCRIPTION: Set the C game-specific options (called from Tcl)
**              Ignore if you don't care about Tcl for now.
**
************************************************************************/

void SetTclCGameSpecificOptions(theOptions)
int theOptions[];
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
************************************************************************/

POSITION DoMove(thePosition, theMove)
POSITION thePosition;
MOVE theMove;
{
	return(thePosition + theMove);
}

/************************************************************************
**
** NAME:        GetInitialPosition
**
** DESCRIPTION: Ask the user for an initial position for testing. Store
**              it in the space pointed to by initialPosition;
**
** INPUTS:      POSITION initialPosition : The position to fill.
**
************************************************************************/

POSITION GetInitialPosition()
{
	POSITION initialPosition;
	printf("Please input the starting value [0 - N] : ");
	scanf(POSITION_FORMAT,&initialPosition);
	return initialPosition;
}

/************************************************************************
**
** NAME:        PrintComputersMove
**
** DESCRIPTION: Nicely format the computers move.
**
** INPUTS:      MOVE   *computersMove : The computer's move.
**              STRING  computersName : The computer's name.
**
************************************************************************/

void PrintComputersMove(computersMove, computersName)
MOVE computersMove;
STRING computersName;
{
	printf("%8s's move              : %2d\n", computersName, computersMove);
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
************************************************************************/

VALUE Primitive(pos)
POSITION pos;
{
	if(pos >= N) /* once they get to N, the game is over*/
		return(gStandardGame ? lose : win); /* gStandardGame  lose win*/
	else
		return(undecided);
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

void PrintPosition(position, playerName, usersTurn)
POSITION position;
STRING playerName;
BOOLEAN usersTurn;
{
	printf("\nTOTAL                        : "POSITION_FORMAT " %s \n\n",position,
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
** CALLS:       MOVELIST *CreateMovelistNode(MOVE,MOVELIST *)
**
************************************************************************/

MOVELIST *GenerateMoves(position)
POSITION position;
{
	unsigned int count = 1;
	MOVELIST *head = NULL;
	MOVELIST *CreateMovelistNode();
	while((count <= gTurn) && (count + position <= N)) {
		head = CreateMovelistNode(count, head);
		count++;
	}
	return(head);
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
	USERINPUT ret, HandleDefaultTextInput();
	BOOLEAN ValidMove();
	char input[2];

	input[0] = '3';

	do {
		printf("%8s's move [(u)ndo/1/2] :  ", playerName);

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
	unsigned int theInput = atoi(input);
	return(theInput <= gTurn && theInput >= 1);
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
	return((MOVE) atoi(input)); /* user input is 1-9, our rep. is 0-8 */
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

void PrintMove(MOVE theMove)
{
	printf("%d", theMove);
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
	STRING move = (STRING) SafeMalloc(3);
	sprintf( move, "%d", theMove );
	return move;
}



STRING kDBName = "1TON";

int NumberOfOptions()
{
	return (MAXN * MAXM);
}

int getOption()
{
	if(gStandardGame)
		return ((N * MAXM + gTurn - 1) * 2);
	else
		return ((N * MAXM + gTurn - 1) * 2 + 1);
}

void setOption(int option)
{
	int set;
	set = option % 2;
	option /= 2;
	if(!set) {
		gStandardGame = TRUE;
		gTurn = ((option % MAXM) + 1);
		N = (option / MAXM);
	}
	else {
		gStandardGame = FALSE;
		gTurn = ((option % MAXM) + 1);
		N = (option / MAXM);
	}
}


POSITION StringToPosition(char* board) {
	return atoi(board);
}

char* PositionToString(POSITION pos) {
	char buffer[32];
	snprintf(buffer, 32, "%lld",pos);
	char* ret = malloc(sizeof(char)*(strlen(buffer)+1));
	strncpy(ret, buffer, (strlen(buffer)+1));
	return ret;
}

char * PositionToEndData(POSITION pos) {
	return NULL;
}
