/************************************************************************
**
** NAME:        mtactix.c
**
** DESCRIPTION: The 2-D game of Tac Tix
**
** AUTHOR:      Dan Garcia  -  University of California at Berkeley
**              Copyright (C) Dan Garcia, 1995. All rights reserved.
**
** DATE:        09/18/91
**
** UPDATE HIST:
**
** 05-15-95 1.0   : Final release code for M.S.
**
** Decided to check out how much space was wasted with the array:
**
** Without checking for symmetries
**
** Evaluating the value of 3x3 TacTix...done in 0.266084 seconds!
** Undecided =     0 out of 512
** Lose      =    88 out of 512
** Win       =   424 out of 512
** Tie       =     0 out of 512
** Unk       =     0 out of 512
** TOTAL     =   512 out of 512
**
** This is the reverse game, where 3-in-a-row is a losing position.
** Undecided =     0 out of 512
** Lose      =    89 out of 512
** Win       =   423 out of 512
** Tie       =     0 out of 512
** Unk       =     0 out of 512
** TOTAL     =   512 out of 512
** While checking for symmetries and storing a canonical elt from them.
**
** Evaluating the value of Tic-Tac-Toe...done in 5.343184 seconds!
** Undecided = 18917 out of 19682
** Lose      =   224 out of 19682
** Win       =   390 out of 19682
** Tie       =   151 out of 19682
** Unk       =     0 out of 19682
** TOTAL     =   765 out of 19682
**
**     Time Loss : 3.723
** Space Savings : 6.279
**
**	8-21-06		changed to GetMyInt() dmchan
**************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/

#include <stdio.h>
#include <math.h>
#include "gamesman.h"

POSITION gInitialPosition  = 65535;
POSITION gMinimalPosition  = 65535;

STRING kAuthorName         = "Dan Garcia";
STRING kGameName           = "Tac Tix";
BOOLEAN kPartizan           = FALSE;
BOOLEAN kDebugMenu          = FALSE;
BOOLEAN kGameSpecificMenu   = TRUE;
BOOLEAN kTieIsPossible      = FALSE;
BOOLEAN kLoopy               = FALSE;
BOOLEAN kDebugDetermineValue = FALSE;
POSITION kBadPosition           = -1;
void*    gGameSpecificTclInit = NULL;

STRING kHelpGraphicInterface =
        "The LEFT button toggles the pieces under the cursor on and off for\n\
removal.  An 'X' is drawn in the center of the piece when it has been\n\
chosen for removal. The interface prevents you from selecting invalid\n\
pieces. When you are finished selecting, click the MIDDLE button. It\n\
doesn't matter where the cursor is when you click the MIDDLE button.\n\
The RIGHT button is the same as UNDO, in that it reverts back to\n\
your your most recent position."                                                                                                                                                                                                                                                                                                                                                                                                                                                     ;

STRING kHelpTextInterface    =
        "On your turn, use the LEGEND to determine the positions (between 1 and\n\
16, with 1 at the upper left and 9 at the lower right) of the pieces you\n\
wish to remove and hit return. To select more than one piece (which\n\
you'll have to do if you want to win...) just type the position numbers\n\
all at once and hit return. For example, to remove all the pieces from\n\
the top layer, type '1 2 3' and hit return. If at any point you have\n\
made a mistake, you can type u and hit return and the system will\n\
revert back to your most recent position."                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        ;

STRING kHelpOnYourTurn =
        "You remove as many disks as you want from any row or column (but NOT\n\
diagonal), as long as the disks you removed ARE contiguous (next to\n\
each other)."                                                                                                                                                        ;

STRING kHelpStandardObjective =
        "To remove the last disk. Your last move does not necessarily have to be\n\
one disk, however.  Example: If there are only two disks (in a row, and\n\
contiguous) on the board and it's your turn, you should take both of them\n\
for the win.  (The last disk was removed with your turn)."                                                                                                                                                                                                                                            ;

STRING kHelpReverseObjective =
        "To force your opponent to remove the last disk.";

STRING kHelpTieOccursWhen = "";   /* empty since kTieIsPossible == FALSE */

STRING kHelpExample =
        "         (  1  2  3  4 )           : O O O O\n\
         (  5  6  7  8 )           : O O O O\n\
LEGEND:  (  9 10 11 12 )  TOTAL:   : O O O O\n\
         ( 13 14 15 16 )           : O O O O \n\n\
Computer's move                    : [ 11 15 ]\n\n\
         (  1  2  3  4 )           : O O O O\n\
         (  5  6  7  8 )           : O O O O\n\
LEGEND:  (  9 10 11 12 )  TOTAL:   : O O - O\n\
         ( 13 14 15 16 )           : O O - O \n\n\
     Dan's move [(u)ndo/1-16]      : { 2 6 10 14 }\n\n\
         (  1  2  3  4 )           : O - O O\n\
         (  5  6  7  8 )           : O - O O\n\
LEGEND:  (  9 10 11 12 )  TOTAL:   : O - - O\n\
         ( 13 14 15 16 )           : O - - O \n\n\
Computer's move                    : [ 3 4 ]\n\n\
         (  1  2  3  4 )           : O - - -\n\
         (  5  6  7  8 )           : O - O O\n\
LEGEND:  (  9 10 11 12 )  TOTAL:   : O - - O\n\
         ( 13 14 15 16 )           : O - - O \n\n\
     Dan's move [(u)ndo/1-16]      : { 7 8 }\n\n\
         (  1  2  3  4 )           : O - - -\n\
         (  5  6  7  8 )           : O - - -\n\
LEGEND:  (  9 10 11 12 )  TOTAL:   : O - - O\n\
         ( 13 14 15 16 )           : O - - O \n\n\
Computer's move                    : [ 1 5 ]\n\n\
         (  1  2  3  4 )           : - - - -\n\
         (  5  6  7  8 )           : - - - -\n\
LEGEND:  (  9 10 11 12 )  TOTAL:   : O - - O\n\
         ( 13 14 15 16 )           : O - - O \n\n\
     Dan's move [(u)ndo/1-16]      : { 12 16 }\n\n\
         (  1  2  3  4 )           : - - - -\n\
         (  5  6  7  8 )           : - - - -\n\
LEGEND:  (  9 10 11 12 )  TOTAL:   : O - - -\n\
         ( 13 14 15 16 )           : O - - - \n\n\
Computer's move                    : [ 9 13 ]\n\n\
         (  1  2  3  4 )           : - - - -\n\
         (  5  6  7  8 )           : - - - -\n\
LEGEND:  (  9 10 11 12 )  TOTAL:   : - - - -\n\
         ( 13 14 15 16 )           : - - - - \n\n\
Computer wins. Nice try, Dan."                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          ;

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

#define k1InARow      16        /* 16 ways for there to be 1 in a row with a 4x4 board */
#define k2InARow      24        /* 24 ways for there to be 2 in a row with a 4x4 board */
#define k3InARow      16        /* 16 ways for there to be 3 in a row with a 4x4 board */
#define k4InARow      8         /*  8 ways for there to be 4 in a row with a 4x4 board */

int kNumberMoves       = k1InARow + k2InARow + k3InARow + k4InARow;
int kFull3x3Board      = 1911;  /* 1 + 2 + 4 + 16 + 32 + 64 + 256 + 512 + 1024 */

int gBoardColumns      = 4;     /* 3 columns on the board */
int gBoardRows         = 4;     /* 3 rows on the board */
int gBoardSize         = 16;    /* gBoardColumns x gBoardRows sized board */
POSITION gNumberOfPositions = 65536; /* 2^gBoardSize */

typedef enum possibleSquarePieces {
	Blank, o
} BlankO;

char *gBlankOString[] = { "-", "O" };

/* Powers of 2 - this is the way I encode the position, as an integer */
int g2Array[] =
{ 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536 };

int TwoInRow[k2InARow][2] = {   {0,1},{1, 2},{ 2, 3},{ 4, 5},{ 5, 6},{ 6, 7},
				{8,9},{9,10},{10,11},{12,13},{13,14},{14,15},
				{0,4},{4, 8},{ 8,12},{ 1, 5},{ 5, 9},{ 9,13},
				{2,6},{6,10},{10,14},{ 3, 7},{ 7,11},{11,15}};

int ThreeInRow[k3InARow][3] = {{0,1,2},{1,2,3},{4,5,6},{5,6,7},{8,9,10},{9,10,11},{12,13,14},{13,14,15},
			       {0,4,8},{4,8,12},{1,5,9},{5,9,13},{2,6,10},{6,10,14},{3,7,11},{7,11,15}};

int FourInRow[k4InARow][4] = {  {0,1,2,3},{4,5,6,7},{8,9,10,11},{12,13,14,15},
				{0,4,8,12},{1,5,9,13},{2,6,10,14},{3,7,11,15}};


/** Function Prototypes **/
void PositionToBlankO(POSITION thePos,BlankO *theBlankO);

STRING MoveToString( MOVE );

/*
   0 1 2
   3 4 5
   6 7 8

   0  1  2  3
   4  5  6  7
   8  9 10 11
   12 13 14 15
 */

/************************************************************************
**
** NAME:        InitializeDatabases
**
** DESCRIPTION: Initialize the gDatabase and any other
**              variables that might depend on the menu choices.
**
** CALLS:       GENERIC_PTR SafeMalloc(int)
**
************************************************************************/

void InitializeGame()
{
	gMoveToStringFunPtr= &MoveToString;
}

void FreeGame()
{
}

/************************************************************************
**
** NAME:        DebugMenu
**
** DESCRIPTION: Menu used to debug internal problems. Does nothing if
**              kDebugMenu == FALSE
**
************************************************************************/

void DebugMenu() {
}

/************************************************************************
**
** NAME:        GameSpecificMenu
**
** DESCRIPTION: Menu used to change game-specific parmeters, such as
**              the side of the board in an nxn Tactix board, etc. Does
**              nothing if kGameSpecificMenu == FALSE
**
************************************************************************/

void GameSpecificMenu()
{
	char GetMyChar();
	signed char c;
	int choice, i;

	do {
		if(gInitialPosition == (gNumberOfPositions - 1)) /* full 4x4 */
			choice = 2;
		else if(gInitialPosition == kFull3x3Board) /* full 3x3 */
			choice = 3;
		else
			choice = 1;

		printf("\n\t----- Game-specific options for %s -----\n\n", kGameName);

		printf("\tCurrent Initial Position:\n");
		PrintPosition(gInitialPosition, gPlayerName[kPlayerOneTurn], kHumansTurn);

		printf("\t1)\tChoose %s RANDOM initial position\n",
		       (choice == 2) ? "a" : "another");
		if(choice != 2)
			printf("\t2)\tChoose the DEFAULT, or FULL initial position\n");
		if(choice != 3)
			printf("\t3)\tChoose the old DEFAULT, a full 3 x 3 board\n");
		printf("\t4)\tChoose a USER-SPECIFIED board\n");

		printf("\n\n\tb)\t(B)ack = Return to previous activity.\n");
		printf("\n\nSelect an option: ");

		switch(GetMyChar()) {
		case 'Q': case 'q':
			ExitStageRight();
		case '1':
			gInitialPosition = GetRandomNumber(gNumberOfPositions); /* random board */
			break;
		case '2':
			if(choice == 2) {
				BadMenuChoice();
				HitAnyKeyToContinue();
			}
			else
				gInitialPosition = gNumberOfPositions - 1; /* 4 x 4 board */
			break;
		case '3':
			if(choice == 3) {
				BadMenuChoice();
				HitAnyKeyToContinue();
			}
			else
				gInitialPosition = kFull3x3Board; /* 3 x 3 board */
			break;
		case '4':
			printf("\n\n\t----- Get Initial Position -----\n");
			printf("\n\tPlease input the position to begin with.\n");
			printf("\tNote that it should be in the following format:\n\n");
			printf("O - - -\nO O - O           <----- EXAMPLE \n- O O -\nO O - O\n\n");

			gInitialPosition = 0;
			gBoardSize = gBoardColumns * gBoardRows;
			i = 0;
			getchar();
			while(i < gBoardSize && (c = getchar()) != EOF) {
				if(c == 'o' || c == 'O' || c == '0')
					gInitialPosition += (int)pow(2.0,(double)i++);
				else if(c == '-')
					i++; /* do nothing */
				else
					; /* do nothing */
			}

			break;
		case 'b': case 'B':
			gBoardSize = gBoardColumns * gBoardRows;
			gNumberOfPositions = (int)pow(2.0,(double)gBoardSize);
			return;
		default:
			printf("\nSorry, I don't know that option. Try another.\n");
			HitAnyKeyToContinue();
			break;
		}
	} while(TRUE);

}

// Anoto pen support - implemented in core/pen/ptactix.c
extern void gPenHandleTclMessage(int options[], char *filename, Tcl_Interp *tclInterp, int debug);

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
	// Anoto pen support
	if ((gPenFile != NULL) && (gTclInterp != NULL)) {
		gPenHandleTclMessage(theOptions, gPenFile, gTclInterp, gPenDebug);
	}
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
	int i, j = 1;

	for(i = 0, j = 1; i < gBoardSize; i++, j*=2)
		if(j & theMove) /* ith move */
			thePosition -= j;

	return(thePosition);
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
	int initialPosition;
	printf("Keeping in mind that X always goes first...\n");
	printf("Please input a 9-element string of either 0,1, or 2.\n");
	printf("Where blank = 0, O = 1, X = 2. Example: 100010001\n");
	/*scanf("%d",&initialPosition);*/
	initialPosition = GetMyInt();
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

void PrintComputersMove(computersMove,computersName)
MOVE computersMove;
STRING computersName;
{
	int i, j = 1;
	printf("%8s's move                    : [ ", computersName);
	for(i = 0, j = 1; i < gBoardSize; i++, j*=2)
		if(j & computersMove) /* ith move */
			printf("%d ", i+1);
	printf("]\n");
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
** CALLS:       BOOLEAN ThreeInARow()
**              BOOLEAN AllFilledIn()
**              PositionToBlankO()
**
************************************************************************/

VALUE Primitive(position)
POSITION position;
{
	if( position == 0 )
		return(gStandardGame ? lose : win);
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
** CALLS:       PositionToBlankO()
**              GetValueOfPosition()
**              GetPrediction()
**              GENERIC_PTR SafeMalloc(int)
**
************************************************************************/

void PrintPosition(position,playerName,usersTurn)
POSITION position;
STRING playerName;
BOOLEAN usersTurn;
{
	BlankO *theBlankO;
  #ifndef MEMWATCH
	GENERIC_PTR SafeMalloc();
  #endif
	theBlankO = (BlankO *) SafeMalloc (sizeof(BlankO) * gBoardSize);
	PositionToBlankO(position,theBlankO);

	printf("\n         (  1  2  3  4 )           : %s %s %s %s\n",
	       gBlankOString[(int)theBlankO[0]],
	       gBlankOString[(int)theBlankO[1]],
	       gBlankOString[(int)theBlankO[2]],
	       gBlankOString[(int)theBlankO[3]] );
	printf("         (  5  6  7  8 )           : %s %s %s %s\n",
	       gBlankOString[(int)theBlankO[4]],
	       gBlankOString[(int)theBlankO[5]],
	       gBlankOString[(int)theBlankO[6]],
	       gBlankOString[(int)theBlankO[7]] );
	printf("LEGEND:  (  9 10 11 12 )  TOTAL:   : %s %s %s %s\n",
	       gBlankOString[(int)theBlankO[8]],
	       gBlankOString[(int)theBlankO[9]],
	       gBlankOString[(int)theBlankO[10]],
	       gBlankOString[(int)theBlankO[11]] );
	printf("         ( 13 14 15 16 )           : %s %s %s %s %s\n\n",
	       gBlankOString[(int)theBlankO[12]],
	       gBlankOString[(int)theBlankO[13]],
	       gBlankOString[(int)theBlankO[14]],
	       gBlankOString[(int)theBlankO[15]],
	       GetPrediction(position,playerName,usersTurn));

	SafeFree((GENERIC_PTR)theBlankO);
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
**              BOOLEAN   Contiguous2(BlankO, int, int);
**              BOOLEAN   Contiguous3(BlankO, int, int, int);
**              BOOLEAN   Contiguous4(BlankO, int, int, int, int);
**
************************************************************************/

MOVELIST *GenerateMoves(position)
POSITION position;
{
	MOVE theMove;
	MOVELIST *head = NULL;
	MOVELIST *CreateMovelistNode();
	BOOLEAN Contiguous2(), Contiguous3(), Contiguous4();
	BlankO *theBlankO;
	int i, index1, index2, index3;
  #ifndef MEMWATCH
	GENERIC_PTR SafeMalloc();
  #endif

	theBlankO = (BlankO *) SafeMalloc (sizeof(BlankO) * gBoardSize);
	PositionToBlankO(position,theBlankO);

	index1 = k1InARow;
	index2 = index1 + k2InARow;
	index3 = index2 + k3InARow;

	for(i = kNumberMoves - 1; i >= 0; i--) {
		if( ((i < index1 && theBlankO[i] == o) ||
		     (i >= index1 && i < index2 && Contiguous2(theBlankO,
		                                               TwoInRow[i-index1][0],
		                                               TwoInRow[i-index1][1])) ||
		     (i >= index2 && i < index3 && Contiguous3(theBlankO,
		                                               ThreeInRow[i-index2][0],
		                                               ThreeInRow[i-index2][1],
		                                               ThreeInRow[i-index2][2])) ||
		     (i >= index3 && Contiguous4(theBlankO,
		                                 FourInRow[i-index3][0],
		                                 FourInRow[i-index3][1],
		                                 FourInRow[i-index3][2],
		                                 FourInRow[i-index3][3])))) {

			if(i >= index1 && i < index2)
				theMove = g2Array[TwoInRow[i-index1][0]] +
				          g2Array[TwoInRow[i-index1][1]];
			else if(i >= index2 && i < index3)
				theMove = g2Array[ThreeInRow[i-index2][0]] +
				          g2Array[ThreeInRow[i-index2][1]] +
				          g2Array[ThreeInRow[i-index2][2]];
			else if(i >= index3)
				theMove =       g2Array[FourInRow[i-index3][0]] +
				          g2Array[FourInRow[i-index3][1]] +
				          g2Array[FourInRow[i-index3][2]] +
				          g2Array[FourInRow[i-index3][3]];
			else if(i < index1)
				theMove = g2Array[i];
			else
				BadElse("");

			head = CreateMovelistNode(theMove,head);
		}
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
	BOOLEAN ValidMove();
	USERINPUT ret, HandleDefaultTextInput();

	do {
		*theMove = 0;
		printf("%8s's move [list of 1-%d]     : ", playerName, gBoardSize);

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
	BOOLEAN valid = TRUE;
	int slot[4], ret, i;
	ret = sscanf(input,"%d %d %d %d", &slot[0],&slot[1],&slot[2],&slot[3]);
	for(i=0; i<ret; i++)
		valid &= (slot[i] <= 16 && slot[i] >= 1);
	return(valid && ret);
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
	int slot[4], ret, i;
	MOVE tmpMove = 0;
	ret = sscanf(input,"%d %d %d %d", &slot[0],&slot[1],&slot[2],&slot[3]);
	for(i=0; i<ret; i++)
		tmpMove += g2Array[slot[i]-1];
	return(tmpMove);
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
	STRING m = MoveToString( theMove );
	printf( "%s", m );
	SafeFree( m );
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
	STRING m = (STRING) SafeMalloc( 20 );
	STRING temp = (STRING) SafeMalloc( 20 );

	/* The plus 1 is because the user thinks it's 1-9, but MOVE is 0-8 */
	int i;
	sprintf( temp, "[ " );
	sprintf( m, "%s", temp);

	for(i = 0; i < gBoardSize; i++) {
		if(theMove & g2Array[i]) {
			sprintf( temp, "%s%d ", m, i+1);
			sprintf( m, "%s", temp );
		}
	}

	sprintf( temp, "%s]", m);

	SafeFree( m );
	return temp;
}

/************************************************************************
*************************************************************************
** BEGIN   PROBABLY DON'T HAVE TO CHANGE THESE SUBROUTINES UNLESS YOU
**         FUNDAMENTALLY WANT TO CHANGE THE WAY YOUR GAME STORES ITS
**         POSITIONS IN THE TABLE FROM AN ARRAY TO SOMETHING ELSE
**         AND ALSO CHANGE THE DEFINITION OF A POSITION (NOW AN INT)
*************************************************************************
************************************************************************/

/************************************************************************
*************************************************************************
** END     PROBABLY DON'T HAVE TO CHANGE THESE SUBROUTINES UNLESS YOU
**         FUNDAMENTALLY WANT TO CHANGE THE WAY YOUR GAME STORES ITS
**         POSITIONS IN THE TABLE FROM AN ARRAY TO SOMETHING ELSE
**         AND ALSO CHANGE THE DEFINITION OF A POSITION (NOW AN INT)
*************************************************************************
************************************************************************/

/************************************************************************
*************************************************************************
**         EVERYTHING BELOW THESE LINES IS LOCAL TO THIS FILE
*************************************************************************
************************************************************************/

/************************************************************************
**
** NAME:        PositionToBlankO
**
** DESCRIPTION: convert an internal position to that of a BlankO.
**
** INPUTS:      POSITION thePos     : The position input.
**              BlankO *theBlankO : The converted BlankO output array.
**
************************************************************************/

void PositionToBlankO(thePos,theBlankO)
POSITION thePos;
BlankO *theBlankO;
{
	int i, j = 1;
	for(i = 0, j = 1; i < gBoardSize; i++, j*=2)
		if(j & thePos) /* ith move */
			theBlankO[i] = o;
		else
			theBlankO[i] = Blank;
}

/************************************************************************
**
** NAME:        BlankOToPosition
**
** DESCRIPTION: convert a BlankO to that of an internal position.
**
** INPUTS:      BlankO *theBlankO : The converted BlankO output array.
**
** OUTPUTS:     POSITION: The equivalent position given the BlankO.
**
************************************************************************/

POSITION BlankOToPosition(theBlankO)
BlankO *theBlankO;
{
	int i;
	POSITION position = 0;

	for(i = 0; i < gBoardSize; i++)
		position += g2Array[i] * (int)theBlankO[i];

	return(position);
}

/************************************************************************
**
** NAME:        Contiguous4
**
** DESCRIPTION: Return TRUE iff there are four contiguous pieces.
**
** INPUTS:      BlankO theBlankO[gBoardSize]   : The BlankO array.
**              int a,b,c,d                    : The 4 positions to check.
**
** OUTPUTS:     (BOOLEAN) TRUE iff there are four-in-a-row.
**
************************************************************************/

BOOLEAN Contiguous4(theBlankO,a,b,c,d)
BlankO theBlankO[];
int a,b,c,d;
{
	return(       theBlankO[a] == o &&
	              theBlankO[b] == o &&
	              theBlankO[c] == o &&
	              theBlankO[d] == o );
}

/************************************************************************
**
** NAME:        Contiguous3
**
** DESCRIPTION: Return TRUE iff there are three contiguous pieces.
**
** INPUTS:      BlankO theBlankO[gBoardSize] : The BlankO array.
**              int a,b,c                    : The 3 positions to check.
**
** OUTPUTS:     (BOOLEAN) TRUE iff there are three-in-a-row.
**
************************************************************************/

BOOLEAN Contiguous3(theBlankO,a,b,c)
BlankO theBlankO[];
int a,b,c;
{
	return(       theBlankO[a] == o &&
	              theBlankO[b] == o &&
	              theBlankO[c] == o );
}

/************************************************************************
**
** NAME:        Contiguous2
**
** DESCRIPTION: Return TRUE iff there are three contiguous pieces.
**
** INPUTS:      BlankO theBlankO[gBoardSize] : The BlankO array.
**              int a,b                     : The 3 positions to check.
**
** OUTPUTS:     (BOOLEAN) TRUE iff there are three-in-a-row.
**
************************************************************************/

BOOLEAN Contiguous2(theBlankO,a,b)
BlankO theBlankO[];
int a,b;
{
	return(theBlankO[a] == o && theBlankO[b] == o );
}

STRING kDBName = "tactix";

int NumberOfOptions()
{
	return 2;
}

int getOption()
{
	if(gStandardGame) return 1;
	return 2;
}

void setOption(int option)
{
	if(option == 1)
		gStandardGame = TRUE;
	else
		gStandardGame = FALSE;
}

POSITION StringToPosition(char* board) {
	// FIXME: this is just a stub
	return atoi(board);
}


char* PositionToString(POSITION pos) {
	// FIXME: this is just a stub
	return "Implement Me";
}

char * PositionToEndData(POSITION pos) {
	return NULL;
}
