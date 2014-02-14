/************************************************************************
**
** NAME:        mjoust.c
**
** DESCRIPTION: Joust for computers
**
** AUTHOR:      Dave and Isaac  -  University of California at Berkeley
**              Copyright (C) D & I productions, 1995. All rights reserved.
**
** DATE:        1995
**
** UPDATE HIST:
**
** 04/30/2008   Added Symmetries (works for any board size). Not a very
**              symmetry-frendly game the bigger the board is, because
**              initial positions are fixed:
**
** 4 x 4 board:
**
****With Symmetries
**
** Lose      =   287 out of 521 (  154 primitive)
** Win       =   234 out of 521 (    0 primitive)
** Tie       =     0 out of 521 (    0 primitive)
** Draw      =     0 out of 521
** Unknown   =     0 out of 521 (Sanity-check...should always be 0)
** TOTAL     =   521 out of 7864320 allocated (  154 primitive)
**
****With Symmetries:
**
** Lose      =    74 out of 133 (   39 primitive)
** Win       =    59 out of 133 (    0 primitive)
** Tie       =     0 out of 133 (    0 primitive)
** Draw      =     0 out of 133
** Unknown   =     0 out of 133 (Sanity-check...should always be 0)
** TOTAL     =   133 out of 7864320 allocated (   39 primitive)
**
****5 x 4:
****Without Symmetries
** TOTAL     = 16211 out of 199229440 allocated ( 4844 primitive)
****With Symmetries: Almost no difference?
** TOTAL     = 16096 out of 199229440 allocated ( 4788 primitive)
**************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/
#include <stdio.h>
#include "gamesman.h"
/* 10/12 Isaac: this is new */
#include <math.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
/* end new include */

POSITION gNumberOfPositions  = 3932160; //99614720;  /* (20 * 19 * 2^18) */ //203954088 from InitDatabases

POSITION gInitialPosition    = 16778208; //Blank with pieces in the corners for 4x4
POSITION gMinimalPosition    = 16778208;
POSITION kBadPosition        = -1; /* This can never be the rep. of a position */

STRING kAuthorName         = "Dave Le, Isaac Greenbride, and Mike Jurka";
STRING kGameName           = "Joust";
STRING kDBName             = "joust";
BOOLEAN kPartizan           = TRUE;
BOOLEAN kSupportsHeuristic  = TRUE;
BOOLEAN kSupportsSymmetries = TRUE;
BOOLEAN kSupportsGraphics   = TRUE;
BOOLEAN kDebugMenu          = TRUE;
BOOLEAN kGameSpecificMenu   = TRUE;  //We use this to set the board
BOOLEAN kTieIsPossible      = FALSE;  /* Isaac 10/24: changed to
                                       * FALSE */
BOOLEAN kLoopy               = FALSE;
BOOLEAN kDebugDetermineValue = FALSE;
void*    gGameSpecificTclInit = NULL;

STRING kHelpGraphicInterface =
        "";

STRING kHelpTextInterface    =
        "Two players control a single knight chess piece. Players alternate turns\n\
moving their piece in an L shaped movement on the board. Once the player\n\
has moved from space, the square that was previously occupied is considered\n\
to be burned. No piece may occupy the square again."                                                                                                                                                                                                                                                ;

STRING kHelpOnYourTurn =
        "Move your knight piece to any open spot that is not burned.";

STRING kHelpStandardObjective =
        "To force your opponent into a position where he cannot move.";

STRING kHelpReverseObjective =
        "To be in a spot where you cannot move your piece.";

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
#define MAXBOARDSIZE 20
unsigned int BOARDSIZE  =   16;          /* ROWSIZE x COLUMNSIZE board. */
unsigned int ROWSIZE    =   4;           /* Rows on the board. */
unsigned int COLUMNSIZE =   4;           /* Columns on the board. */
unsigned int BITSIZE    =   4;           /* The number of bits needed to
                                          * store the largest square number
                                          * on the board. EX: For a 20
                                          * square board, you need 4 bits
                                          * because 20 < 2^4 */
unsigned int BITSIZEMASK =  0xf;         /* Number to mask BITSIZE number
                                          * of bits */
BOOLEAN kPrintDebug = FALSE;
BOOLEAN kDoMoveDebug = FALSE;
typedef enum possibleBoardPieces { /* O are black pieces, X are white pieces */
	Blank, Burnt, o, x
} BlankBurntOX;

BOOLEAN gBurnMove = TRUE;
BOOLEAN doubleTrouble = FALSE;
int burnType = 0;                /* 0 is for the burnt bridges ver.,
                                  * 1 is for the arbitrary grenade
                                  * 2 is for piece-specific version
                                  * (Note: analysis of burn AFTER
                                  * move would be much larger) */
/* Prints an example based on the current Row and Column Sizes */
void PrintExample();

/* Checks if a player can move. */
BOOLEAN CanMove(POSITION);
/* helper functions for CanMove */
BOOLEAN CanKnightMove(BlankBurntOX*, int, int);
BOOLEAN CanRookMove(BlankBurntOX*, int, int);
BOOLEAN CanBishopMove(BlankBurntOX*, int, int);



/* pulls the current piece location from the position */
unsigned int GetPFromPosition(POSITION);
unsigned int GetXFromPosition(POSITION);
unsigned int GetOFromPosition(POSITION);

/* hash */
POSITION BlankBurntOXToPosition(BlankBurntOX[], BlankBurntOX);
void PositionToBlankBurntOX(POSITION, BlankBurntOX[]);
BlankBurntOX WhoseTurn(POSITION);
int ExtractMove(MOVE theMove);
int ExtractBurn(MOVE theMove);

/* For game specific options */
void PieceMessage(POSITION, BOOLEAN);
void ChangeBoardSize();
void ChangePieces();
void ChangePiece(POSITION);
void ChangeBurn();
int GenerateNewInitial();
void ChangeOrder();

/* Possible move and helpers */
BOOLEAN PossibleMove(int, POSITION, BlankBurntOX[]);
BOOLEAN PossibleRook(int, int, int, int, int, BlankBurntOX[]);
BOOLEAN PossibleKnight(int, int, int, int, int, BlankBurntOX[]);
BOOLEAN PossibleBishop(int, int, int, int, int, BlankBurntOX[]);
BOOLEAN PossibleKing(int, int, int, int, int, BlankBurntOX[]);
BOOLEAN PossibleQueen(int, int, int, int, int, BlankBurntOX[]);

/* Possible burn and helpers */
BOOLEAN PieceBurn(int, POSITION, BlankBurntOX[]);

STRING MoveToString(MOVE);

char *gBlankBurntOXString[] = { "-", "*", "O", "X" };
STRING kBBExplanation =
        "\nIn Burned Bridges, the spots you touch \
are burned after you leave them. \
There's no turning back!\n"                                                                                       ;
STRING kUGExplanation =
        "\nIn Ultimate Grenade, you have the ability \
to burn ANY spot on the board. Pretty good \
arm you've got there!\n"                                                                                                    ;
STRING kBMExplanation =
        "\nWith Piece-specific burning, you can \
only lob grenades to places your piece \
could move. Don't discount the knight \
too quickly: he can lob grenades OVER \
other burns...\n"                                                                                                                                                                           ;

/*Symmetries*/
POSITION GetCanonicalPosition(POSITION);
void ProcessSymmetries(int, int*, int*);
int gSymmetryMatrix[8][20];

/*************************************************************************
**
** Here we declare the global database variables
**
**************************************************************************/

typedef enum Pieces {knight, bishop, rook, queen, king} Piecetype;
Piecetype Xtype = knight;
Piecetype Otype = knight;
Piecetype XBurn = knight;
Piecetype OBurn = knight;

/************************************************************************
**
** NAME:        InitializeGame
**
** DESCRIPTION: Initialize all internal variables necessary to solve and
**              play the game, given the current game option
**
************************************************************************/

void InitializeGame()
{
	gInitialPosition = GenerateNewInitial();
	gMinimalPosition = gInitialPosition;
	gNumberOfPositions = BOARDSIZE*(BOARDSIZE-1)*pow(2,BOARDSIZE-2)*2; /* xpos * ypos * burned spaces * whoseturn */

	gMoveToStringFunPtr = &MoveToString;

	/*Symmetry code (Messy)*/
	gCanonicalPosition = GetCanonicalPosition;

	if (kSupportsSymmetries) {
		if (ROWSIZE != COLUMNSIZE) {
			int ordered_board[BOARDSIZE];
			int n;
			for (n = 0; n < BOARDSIZE; n++) {
				ordered_board[n] = n;
			}
			//flip along Y axis:
			int FlipNewPosition[BOARDSIZE];
			int Rotate180NewPosition[BOARDSIZE];
			int i, j;

			for (i = 0; i < ROWSIZE; i++) {
				for (j = 0; j < COLUMNSIZE; j++) {
					//flip along Y axis:
					FlipNewPosition[i*COLUMNSIZE+j] = ordered_board[(i+1)*COLUMNSIZE - j - 1];
					//Rotate 180 degrees:
					Rotate180NewPosition[i*COLUMNSIZE+j]= ordered_board[(ROWSIZE-i)*COLUMNSIZE-j-1];
				}
			}
			ProcessSymmetries(4, Rotate180NewPosition, FlipNewPosition);

		}
		else { //square
			if (ROWSIZE ==3) {
				/* This is the array used for flipping along the N-S axis */
				int FlipNewPosition[] = { 2, 1, 0, 5, 4, 3, 8, 7, 6 };

				/* This is the array used for rotating 90 degrees clockwise */
				int Rotate90CWNewPosition[] = { 6, 3, 0, 7, 4, 1, 8, 5, 2 };

				ProcessSymmetries(8, Rotate90CWNewPosition, FlipNewPosition);

			}
			else { //if (ROWSIZE == 4) {
				/*
				   0123   C840   3210
				   4567   D951   7654
				   89AB   EA62   BA98
				   CDEF   FB73   FEDC
				 */

				/* This is the array used for flipping along the N-S axis */
				int FlipNewPosition[] = { 3, 2, 1, 0, 7, 6, 5, 4, 11, 10, 9, 8, 15, 14, 13, 12 };

				/* This is the array used for rotating 90 degrees clockwise */
				int Rotate90CWNewPosition[] = { 12, 8, 4, 0, 13, 9, 5, 1, 14, 10, 6, 2, 15, 11, 7, 3};

				ProcessSymmetries(8, Rotate90CWNewPosition, FlipNewPosition);

			}

		}
	}

}
/************************************************************************
**
** NAME:        ProcessSymmetries
**
** DESCRIPTION: Takes in the number of symmetries we should have,a
**              rotation array, and a reflection array and stores
**              a lookup table of symmetries
**
************************************************************************/

void ProcessSymmetries(int numsymmetries, int* rotation, int* reflection)
{
	int i, j, temp;
	for(i = 0; i < BOARDSIZE; i++) {
		temp = i;
		for(j = 0; j < numsymmetries; j++) {
			if(j == numsymmetries/2)
				temp = reflection[i];
			if(j < numsymmetries/2)
				temp = gSymmetryMatrix[j][i] = rotation[temp];
			else
				temp = gSymmetryMatrix[j][i] = rotation[temp];
		}
	}
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
** DESCRIPTION: Menu used to change game-specific parmeters, such as
**              the side of the board in an nxn Nim board, etc. Does
**              nothing if kGameSpecificMenu == FALSE
**
************************************************************************/

void GameSpecificMenu() {
	printf("Welcome to Dave and Isaac's user-friendly variant chooser!\n");
	printf("type B to set up the board or,\n");
	printf("type O to change the order of moving/burning,\n");
	printf("type P to choose a new piece,\n");
	printf("type S to change the size of the Board, or\n");
	printf("type T Select a different style of burning\n");

	switch(GetMyChar()) {
	case 'B': case 'b':
		GetInitialPosition();
		break;
	case 'o': case 'O':
		ChangeOrder();
		break;
	case 'p': case 'P':
		ChangePieces();
		break;
	case 's': case 'S':
		ChangeBoardSize();
		break;
	case 't': case 'T':
		ChangeBurn();
		break;
	//our "hidden" debug command
	case 'd':
		kPrintDebug = !kPrintDebug;
		printf("Debugging on/off");
		if(kPrintDebug)
		{
			printf("Debug DoMove also? (0 or 1)");
			kDoMoveDebug = GetMyChar();
		}
		break;
	default:
		printf("Thank you, come again!");
		break;
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

/* 11/4 Isaac: Mask off the position's turn bit that has been preserved
 * up till now, then put in the opposite of the current turn bit */
POSITION ChangeTurn(POSITION thePos){
	return thePos ^ 1; /* xor */
}
/************************************************************************
**
** NAME:        GetCanonicalPosition
**
** DESCRIPTION: Go through all of the positions that are symmetrically
**              equivalent and return the SMALLEST, which will be used
**              as the canonical element for the equivalence set.
**
** INPUTS:      POSITION position : The position return the canonical elt. of.
**
** OUTPUTS:     POSITION          : The canonical element of the set.
**
************************************************************************/

POSITION GetCanonicalPosition(position)
POSITION position;
{
	POSITION newPosition, theCanonicalPosition, DoSymmetry();
	int i, NUMSYMMETRIES;

	theCanonicalPosition = position;

	if (ROWSIZE != COLUMNSIZE)
		NUMSYMMETRIES = 4;
	else
		NUMSYMMETRIES = 8;

	for(i = 0; i < NUMSYMMETRIES; i++) {

		newPosition = DoSymmetry(position, i); /* get new */
		if(newPosition < theCanonicalPosition) /* THIS is the one */
			theCanonicalPosition = newPosition; /* set it to the ans */
	}

	return(theCanonicalPosition);
}

/************************************************************************
**
** NAME:        DoSymmetry
**
** DESCRIPTION: Perform the symmetry operation specified by the input
**              on the position specified by the input and return the
**              new position, even if it's the same as the input.
**
** INPUTS:      POSITION position : The position to branch the symmetry from.
**              int      symmetry : The number of the symmetry operation.
**
** OUTPUTS:     POSITION, The position after the symmetry operation.
**
************************************************************************/

POSITION DoSymmetry(position, symmetry)
POSITION position;
int symmetry;
{
	int i;
	BlankBurntOX theBlankOx[BOARDSIZE], symmBlankOx[BOARDSIZE];
	POSITION BlankBurntOXToPosition();

	PositionToBlankBurntOX(position,theBlankOx);
	PositionToBlankBurntOX(position,symmBlankOx); /* Make copy */

	/* Copy from the symmetry matrix */

	for(i = 0; i < BOARDSIZE; i++)
		symmBlankOx[i] = theBlankOx[gSymmetryMatrix[symmetry][i]];

	return(BlankBurntOXToPosition(symmBlankOx, WhoseTurn(position)));
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
** CALLS:       PositionToBlankBurntOX(POSITION,*BlankBurntOX)
**              BlankBurntOX WhoseTurn(*BlankBurntOX)
**
************************************************************************/

/* Dave: 10/20.  theMove contains the move (1-20) in the last 5 bits,
 * and the square to be burnt in the first 5 bits. */

POSITION DoMove(thePosition, theMove)
POSITION thePosition;
MOVE theMove;
{
	/*Dave: Assumes piece move is in the last 5 bits of theMove,
	 * and the burnt spot is in the first 5 bits. */
	MOVE extractedMove = ExtractMove(theMove);     //Mask off all but 5 move bits
	MOVE extractedBurnt = ExtractBurn(theMove);    //Mask off all but 5 burn bits.
	MOVE oldSpot;                                  //Remembers the piece's old spot.
	BlankBurntOX whoseTurn;
	BlankBurntOX theBlankBurntOX[BOARDSIZE];
	POSITION newPos;

	PositionToBlankBurntOX(thePosition, theBlankBurntOX);

	whoseTurn = WhoseTurn(thePosition);

	// Set X/O's new position.
	oldSpot = GetPFromPosition(thePosition);

	if (burnType==0) {
		extractedBurnt = oldSpot;
	}

	if (doubleTrouble) {
		theBlankBurntOX[oldSpot] = Burnt;
	} else {
		theBlankBurntOX[oldSpot] = Blank;
	}
	theBlankBurntOX[extractedBurnt] = Burnt;
	theBlankBurntOX[extractedMove] = whoseTurn;

	newPos = BlankBurntOXToPosition(theBlankBurntOX, whoseTurn==x ? o : x);

	return(newPos);
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
	POSITION BlankBurntOXToPosition();
	BlankBurntOX theBlankBurntOX[BOARDSIZE], whosTurn;
	signed char c;
	int i, xCount = 0, oCount = 0;


	printf("\n\n\t----- Get Initial Position -----\n");
	printf("\n\tPlease input the position to begin with.\n");
	printf("\tNote that it should be in the following format:\n\n");
	PrintExample();
	/*printf("O - - - -\n- * - - -           <----- EXAMPLE \n- * - - X \n- - - - -\n\n");*/

	i = 0;
	getchar();
	while(i < BOARDSIZE && (c = getchar()) != EOF) {
		if((c == 'x' || c == 'X') && xCount < 1)
		{
			theBlankBurntOX[i++] = x;
			xCount = xCount + 1;
		}
		else if((c == 'o' || c == 'O' || c == '0') && oCount < 1)
		{
			theBlankBurntOX[i++] = o;
			oCount = oCount + 1;
		}
		else if(c == '-')
			theBlankBurntOX[i++] = Blank;
		else if(c == '*')
			theBlankBurntOX[i++] = Burnt;
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

	gInitialPosition = BlankBurntOXToPosition(theBlankBurntOX, whosTurn);
	return(gInitialPosition);
}


/************************************************************************
**
** NAME:        ExtractMove
**
** DESCRIPTION: Get a move from something in theMove format.
**
** INPUTS:      MOVE theMove
**
************************************************************************/
int ExtractMove(theMove)
MOVE theMove;
{
	return (theMove >> BITSIZE);
}

/************************************************************************
**
** NAME:        ExtractBurn
**
** DESCRIPTION: Get a burn from something in theMove format.
**
** INPUTS:      MOVE theMove
**
************************************************************************/
int ExtractBurn(theMove)
MOVE theMove;
{
	return (theMove & BITSIZEMASK);
}

/************************************************************************
**
** NAME:        EncodeTheMove
**
** DESCRIPTION: Put a move and a burn into theMove format.
**
** INPUTS:      int theMove, int theBurn
**
************************************************************************/
MOVE EncodeTheMove(theMove, theBurn)
MOVE theMove;
MOVE theBurn;
{
	return (theBurn | (theMove << BITSIZE));
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

	printf("%8s's move              : %2d  %2d\n", computersName, ExtractMove(computersMove) + 1, ExtractBurn(computersMove) + 1); //Dave: added 11114
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
**              PositionToBlankOX()
**
************************************************************************/

VALUE Primitive(position)
POSITION position;
{

	if (kPrintDebug)
		printf("\nPrimitive called"); // for debugging

	if(!CanMove(position))
		return(gStandardGame ? lose : win);
	//Isaac 11/30 need to check if you can burn as well as move
	else
		return(undecided);
}

/**********************************************
* PrintRows
* Whereas before rows were manually printed, we want to print a
* more flexible board.
* printf("         ( 11 12 13 14 15 )           : %s  %s  %s  %s\n",
* PrintRows only need to be passed the current board, it uses the
* global dimensions of the board to determine how it prints.
**********************************************/




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

void PrintPosition(position,playerName,usersTurn)
POSITION position;
STRING playerName;
BOOLEAN usersTurn;
{
	int i, j, spot;
	//  VALUE GetValueOfPosition();
	BlankBurntOX theBlankBurntOX[BOARDSIZE];
	BlankBurntOX WhoseTurn();

	PositionToBlankBurntOX(position, theBlankBurntOX);

	/* shiny, new, flexible way */
	for(i=0; i < ROWSIZE; i++)
	{
		printf("         (");
		for(j=0; j < COLUMNSIZE; j++)
		{
			spot = (j + i * COLUMNSIZE) + 1;
			if (spot < 10)
				printf("  %d", spot);
			else
				printf(" %d", spot);
		}

		if (i == 0)
		{ printf(" )   TOTAL   :"); }
		else
		{ printf(" )           :"); }

		for(j=0; j < COLUMNSIZE; j++)
		{
			printf(" %s", gBlankBurntOXString[(int)theBlankBurntOX[(i*COLUMNSIZE) + j]]);
		}
		if (i == (ROWSIZE - 1))
			printf(" %s", GetPrediction(position,playerName,usersTurn));
		printf("\n");
	}
	printf("%s's move  (as a ", gBlankBurntOXString[(int)WhoseTurn(position)]);
	PieceMessage(position, TRUE);
	printf(", burning");
	if(!gBurnMove || burnType == 0)
		printf(" AFTER moving");
	else
		printf(" BEFORE moving");
	printf(" as a ");
	PieceMessage(position, FALSE);
	printf(")");
	printf("\n");

	if (kPrintDebug)
		printf("Position = "POSITION_FORMAT "\n", position);
}

/************************************************************************
** NAME:        PossibleMove
**
** DESCRIPTION: This is the big one: PossibleMove checks the given
**              move in the array and decides whether this move is
**              blocked or not based on the current position. By
**              moving outward from the position it saves searches.
**
** INPUTS:      rp: the row of the position,
**              cp: the column of the Position,
**              rd: the row of the proposed move,
**              cd: the column of the proposed move.
**              2DBlankBurntOX: the current state of the board in a
**                              2D array
**
** OUTPUTS:  true or false
**
** CALLS:    PossibleRook, PossibleBishop, PossibleQueen, PossibleKnight,
**           PossibleKing
**
************************************************************************/
/*******************
* Isaac 10/18: For use with GenerateMoves
* *****************
*
*******************/

BOOLEAN PossibleMove(int theDest, POSITION thePos, BlankBurntOX theBlankBurntOX[])
{
	int playerPos;
	int rp;
	int cp;
	int rd;
	int cd;
	int i;
	int j;
	Piecetype thePiece;

	BlankBurntOX  WhoseTurn();


	playerPos = GetPFromPosition(thePos);

	/* Set the PieceType we'll be working
	   with */
	if (WhoseTurn(thePos) == x)
	{
		thePiece = Xtype;
	}
	else
	{
		thePiece = Otype;
	}

	for(i=0; i < ROWSIZE; i++) { //Get the player's location in 2D
		for(j=0; j < COLUMNSIZE; j++) {
			if((i*COLUMNSIZE) + j == playerPos) {
				rp = i;
				cp = j;
			}
		}
	}


	for(i=0; i < ROWSIZE; i++) { //Get theDest's location in 2D
		for(j=0; j < COLUMNSIZE; j++) {
			if((i*COLUMNSIZE) + j == theDest) {
				rd = i;
				cd = j;
			}
		}
	}

	if(rd > (ROWSIZE-1) || cd > (COLUMNSIZE-1) || rd < 0 || cd < 0)
	{
		printf("Off the board");
		return FALSE;
	}
	else
	{
		switch (thePiece) {
		case knight:
			return(PossibleKnight(rp, cp, rd, cd, thePos, theBlankBurntOX));
			break;
		case queen:
			return(PossibleQueen(rp, cp, rd, cd, thePos, theBlankBurntOX));
			break;
		case rook:
			return(PossibleRook(rp, cp, rd, cd, thePos, theBlankBurntOX));
			break;
		case bishop:
			return(PossibleBishop(rp, cp, rd, cd, thePos, theBlankBurntOX));
			break;
		case king:
			return(PossibleKing(rp, cp, rd, cd, thePos, theBlankBurntOX));
			break;
		default:
			BadElse("Error in PossibleMove\n");
			break;
		}
	}

	//should never reach there
	return FALSE;
}

/*So many possibilities...
    2     cp     1
        <-||->
    ^^^^<-||->^^^^
   rp----thePos-----
    vvvv<-||->vvvv
        <-||->
    3            4
 */
BOOLEAN PossibleBishop(int rp, int cp, int rd, int cd, int thePos, BlankBurntOX theBlankBurntOX[])
{

	if (kPrintDebug)
		printf(" PossibleBishop: rd=%d cd=%d|", rd, cd);

	/* is it on the board? */
	if ((rd < 0) || (rd > COLUMNSIZE) || (cd < 0) || (cd > ROWSIZE))
	{
		if (kPrintDebug)
			printf("OffBoard\n");
		return FALSE;
	}

/*a. theDest is to the left of the position */
	/*
	<-cd-> cp
	<-|
	<-|
	rp <-|thePos
	<-|
	<-|
	 */
	if (cp > cd )
	{
		/*1a the Dest is below the position */
		/*
		   <-cd-> cp


		   rp    thePos
		   ^     /
		   rd   /
		   v   /
		 */
		if (rp < rd)
		{
			/* I don't want to check (rp, cp) itself since it's not
			 * blank, therefore I look one ahead of my spot*/
			while(rp < rd)
			{
				rp++;
				cp--;
				if(theBlankBurntOX[rp*COLUMNSIZE + cp] != Blank)
				{
					if (kPrintDebug)
						printf("1a");
					return FALSE;
				}
			}
			/* the spots are all Blank --> you can move there if the
			 * destination actually is on that diagonal!*/
			if (kPrintDebug)
				printf("Yes!");
			return (cp == cd && rp == rd);
		}
		/*2a theDest is above the position */
		/*
		   <-cd-> cp
		   ^  \
		   rd  \
		   v    \
		   rp    thePos
		 */
		else if (rp > rd) {
			/* I don't want to check (rp, cp) itself since it's not
			 * blank, therefore I look one ahead of my spot*/
			while(rp > rd)
			{
				rp--;
				cp--;
				if(theBlankBurntOX[rp*COLUMNSIZE + cp] != Blank)
				{
					if (kPrintDebug)
						printf("2a");
					return FALSE;
				}
			}
			/* the spots are all Blank --> you can move there if the
			 * destination actually is on that diagonal! */
			if (kPrintDebug)
				printf("Yes!");
			return (cp == cd && rp == rd);
		}
		/*3a not a Bishop move */
		else
		{
			if (kPrintDebug)
				printf("3a");
			return FALSE;
		}
	}

/*b. theDest is to the right of the position */
	/*
	      cp <-cd->
	   |->
	   |->
	   rp    thePos->
	   |->
	   |->

	 */
	else if (cp < cd )
	{

		/*1b theDest is above the position */
		/*
		      cp <-cd->
		   ^             /
		   rd           /
		   v           /
		   rp    thePos
		 */
		if (rp > rd)
		{
			/* I don't want to check (rp, cp) itself since it's not
			 * blank, therefore I look one ahead of my spot*/
			while(rp > rd)
			{
				rp--;
				cp++;
				if(theBlankBurntOX[rp*COLUMNSIZE + cp] != Blank)
				{
					if (kPrintDebug)
						printf("1b");
					return FALSE;
				}
			}
			/* the spots are all Blank --> you can move there if the
			 * destination actually is on that diagonal!*/
			if (kPrintDebug)
				printf("Yes!");
			return (cp == cd && rp == rd);
		}
		/*2b theDest is below the position */
		/*
		   cp <-cd->


		   rp    thePos
		   ^          \
		   rd          \
		   v            \
		 */
		else if (rp < rd)
		{
			/* I don't want to check (rp, cp) itself since it's not
			 * blank, therefore I look one ahead of my spot*/
			while(rp < rd)
			{
				rp++;
				cp++;
				if(theBlankBurntOX[rp*COLUMNSIZE + cp] != Blank)
				{
					if (kPrintDebug)
						printf("2b");
					return FALSE;
				}
			}
			/* the spots are all Blank --> you can move thereif the
			 * destination actually is on that diagonal!*/
			if (kPrintDebug)
				printf("Yes!");
			return (cp == cd && rp == rd);
		}
		/*3b. not a Bishop move */
		else
		{
			if (kPrintDebug)
				printf("3b");
			return FALSE;
		}
	}
/*c. not a Bishop move */
	else
	{
		if (kPrintDebug)
			printf("c");
		return FALSE;
	}
}



/*The possibilities make my head hurt......
    2     cp     1
        <-||->
    ^^^^<-||->^^^^
   rp----thePos-----
    vvvv<-||->vvvv
        <-||->
    3            4
 */
BOOLEAN PossibleRook(int rp, int cp, int rd, int cd, int thePos, BlankBurntOX theBlankBurntOX[]){

	if (kPrintDebug)
		printf(" PossibleRook: rd=%d cd=%d|", rd, cd);

	/* is it on the board? */
	if ((rd < 0) || (rd > COLUMNSIZE) || (cd < 0) || (cd > ROWSIZE))
	{
		return FALSE;
	}
/* the rows are the same */
/*        cp

    --------------
   rp<---thePos--->
    --------------
 */
	if(rp == rd) {
		/* if cp is less than cd, we want to
		   check everything in the row AFTER cp but not AFTER cd */
		if(cp < cd)
		{
			while(cp < cd)
			{
				cp++;
				if(theBlankBurntOX[rp*COLUMNSIZE + cp] != Blank)
				{
					return FALSE;
				}
			}
			/* the spots are all Blank --> you can move there */
			if (kPrintDebug)
				printf("Yes!");
			return TRUE;
		}
/* check everything in the row BEFORE cp but not BEFORE cd */
		else if(cp > cd)
		{
			while(cp > cd)
			{
				cp--;
				if(theBlankBurntOX[rp*COLUMNSIZE + cp] != Blank)
				{return FALSE; }
			}
/* the spots are all Blank --> you can move there */
			if (kPrintDebug)
				printf("Yes!");
			return TRUE;
		}
		/* the move IS the position (not valid) */
		else if(cp == cd) {
			{
				return FALSE;
			}
		}
	}

/* the columns are the same */
/*
          cp
   |^|
 |||
   rp   thePos
 |||
 ||||v|
 */
	else if (cp == cd)
	{
/* if rp is less than rd, we want to
   check everything in the column AFTER rp but not AFTER rd */
		if(rp < rd)
		{
			while(rp < rd)
			{
				rp++;
				if(theBlankBurntOX[rp*COLUMNSIZE + cp] != Blank)
				{
					return FALSE;
				}
			}
/* the spots are all Blank --> you can move there */
			if (kPrintDebug)
				printf("Yes!");
			return TRUE;
		}
/* check everything in the column BEFORE rp but not BEFORE rd */
		else if(rp > rd)
		{
			while(rp > rd)
			{
				rp--;
				if(theBlankBurntOX[rp*COLUMNSIZE + cp] != Blank)
				{
					return FALSE;
				}
			}

/* the spots are all Blank --> you can move there */
			if (kPrintDebug)
				printf("Yes!");
			return TRUE;
		}
		/* the move IS the position (not valid) */
		else if(rp == rd)
		{
			return FALSE;
		}
	}
/* not a rook move */
	else
	{
		return FALSE;
	}

	//never reached
	return FALSE;
}



/**************************************************/
/*Ouch, the Possibilities
     c1 c2 c3 c4
   r1      a- -b
   r2   c -  |  - d
       thePos
   r3   e -  |  - f
   r4      g- -h

 */

/* Knight jumps over occupied squares so only the destination
 * matters, the King only moves one square, so the same is true */
BOOLEAN PossibleKnight(int rp, int cp, int rd, int cd, int thePos, BlankBurntOX theBlankBurntOX[]){
	int r1 = rp - 2;
	int r2 = rp - 1;
	int r3 = rp + 1;
	int r4 = rp + 2;
	int c1 = cp - 2;
	int c2 = cp - 1;
	int c3 = cp + 1;
	int c4 = cp + 2;

//  printf("PossibleKnight Called: rd%d cd%d, rp%d cp%d", rd, cd, rp, cp);
/* faster to simply check the destination and whether
 * the move is something a knight can do */
	if ((rd >= 0) && (rd < ROWSIZE) && (cd >= 0) && (cd < COLUMNSIZE) &&
	    (theBlankBurntOX[COLUMNSIZE*rd + cd] == Blank) &&
	    ((rd == r1 && (cd == c2 || cd == c3)) ||
	     (rd == r2 && (cd == c1 || cd == c4)) ||
	     (rd == r3 && (cd == c1 || cd == c4)) ||
	     (rd == r4 && (cd == c2 || cd == c3)))
	    )
	{
		if (kPrintDebug)
			printf("Yes!");
		return TRUE;
	}
	else
	{
//      printf("Location: %d\n", theBoard[5*rd + cd]);
		return FALSE;
	}
}

/**************************************************/
/*Yow, the Possibilities
     c1  cp  c3

   r1   a    b   c
   rp   d thePos e
   r3   f    g   h

 */
BOOLEAN PossibleKing(int rp, int cp, int rd, int cd, int thePos, BlankBurntOX theBlankBurntOX[]){

	int r1 = rp + 1;
	int r3 = rp - 1;
	int c1 = cp - 1;
	int c3 = cp + 1;

	if (kPrintDebug)
		printf(" PossibleKing: rd=%d cd=%d|", rd, cd);

	/* faster to simply check the destination and whether
	 * the move is something a king can do */
	if ((theBlankBurntOX[rd*COLUMNSIZE + cd] == Blank) &&
	    (rd == r1 || (rd == rp && cd != cp) || rd == r3) &&
	    (cd == c1 || (cd == cp && rd != rp) || cd == c3))
	{
		if (kPrintDebug)
			printf("Yes!");
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/* Oh no, not more Possibilities....
 * Haha, fooled ya! We can cheat and use old stuff here! Nyaaa Nyaaa.. */
BOOLEAN PossibleQueen(int rp, int cp, int rd, int cd, int thePos, BlankBurntOX theBlankBurntOX[])
{
	return (PossibleBishop(rp, cp, rd, cd, thePos, theBlankBurntOX) ||
	        PossibleRook(rp, cp, rd, cd, thePos, theBlankBurntOX));
	/* Notice that PossibleBishop and Rook return false when the move
	 * is not of their type */
}

/******************
* PossibleBurn
*
* Uses the burnType global to decide how to evaluate
* the given burn move.
* INPUTS: pPos, mPos, bPos, Pos = the current piece position, proposed move,
*         proposed burn, and board position respectively.
*
******************/
/* Note, burn AFTER move is implemented merely by passing Possible burn
* i (the proposed move), along with piecePos (its current position). */
BOOLEAN PossibleBurn(unsigned int pPos, unsigned int mPos, unsigned int bPos, POSITION Pos, BlankBurntOX theBlankBurntOX[]){
	int piecePos;
	MOVE aMove;

	piecePos = GetPFromPosition(Pos);

	switch(burnType) {

/* burn old position (it MUST be open) */
	case 0:
		return (pPos == bPos);
		break;

/* burn anywhere */
	case 1:
		if (doubleTrouble) {
			return (theBlankBurntOX[bPos] == Blank);
		}
		else {
			return (theBlankBurntOX[bPos] == Blank || pPos == bPos); /* burn old spot*/
		}

/* burn based on how your piece moves (you can't throw over other
 * burns) */
	case 2:
		/* Either */
		/* from your starting spot, then move. */
		/* move, then (from your new position) burn. */
		if(!gBurnMove)
		{
			aMove = EncodeTheMove(mPos, mPos);
			Pos = DoMove(Pos, aMove);
			Pos = ChangeTurn(Pos); /* revert turn */
			PositionToBlankBurntOX(Pos, theBlankBurntOX);
		}
		return (PieceBurn(bPos, Pos, theBlankBurntOX));
		break;

/* Not valid burnType == false */
	default:
		if (kPrintDebug)
			printf("\nBad burnType"); // for debugging
		return FALSE;
		break;
	};
}


BOOLEAN PieceBurn(int theDest, POSITION thePos, BlankBurntOX theBlankBurntOX[])
{
	int playerPos;
	int rp;
	int cp;
	int rd;
	int cd;
	int i;
	int j;
	Piecetype theBurn;

	BlankBurntOX  WhoseTurn();


	playerPos = GetPFromPosition(thePos);

	/* Set the PieceType we'll be working
	   with */
	if (WhoseTurn(thePos) == x)
	{
		theBurn = XBurn;
	}
	else
	{
		theBurn = OBurn;
	}

	for(i=0; i < ROWSIZE; i++) { //Get the player's location in 2D
		for(j=0; j < COLUMNSIZE; j++) {
			if((i*COLUMNSIZE) + j == playerPos) {
				rp = i;
				cp = j;
			}
		}
	}


	for(i=0; i < ROWSIZE; i++) { //Get theDest's location in 2D
		for(j=0; j < COLUMNSIZE; j++) {
			if((i*COLUMNSIZE) + j == theDest) {
				rd = i;
				cd = j;
			}
		}
	}

	if(rd > (ROWSIZE-1) || cd > (COLUMNSIZE-1) || rd < 0 || cd < 0)
	{
		printf("Off the board");
		return FALSE;
	}
	else
	{
		switch (theBurn) {
		case knight:
			return(PossibleKnight(rp, cp, rd, cd, thePos, theBlankBurntOX));
			break;
		case queen:
			return(PossibleQueen(rp, cp, rd, cd, thePos, theBlankBurntOX));
			break;
		case rook:
			return(PossibleRook(rp, cp, rd, cd, thePos, theBlankBurntOX));
			break;
		case bishop:
			return(PossibleBishop(rp, cp, rd, cd, thePos, theBlankBurntOX));
			break;
		case king:
			return(PossibleKing(rp, cp, rd, cd, thePos, theBlankBurntOX));
			break;
		default:
			BadElse("Error in PossibleMove\n");
			break;
		}
	}

	//never reached
	return FALSE;
}

/************************************************************************
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
**              BOOLEAN PossibleMove(MOVE, POSITION)
**
************************************************************************/

MOVELIST *GenerateMoves(position)
POSITION position;
{
	MOVELIST *head = NULL;

	int i;
	int j;
	int piecePos;
	MOVE aMove;
	BlankBurntOX theBlankBurntOX[BOARDSIZE];
	PositionToBlankBurntOX(position, theBlankBurntOX);

	piecePos = GetPFromPosition(position);
	if (kPrintDebug)
	{
		printf("Generate's WhoseTurn thinks: %d is up and in position %d\n",
		       WhoseTurn(position), piecePos); //for debugging
		printf("0=win, 1=lose, 2=tie, 3=undecided, 4=visited\n"); //for debugging
		printf("Generate's Primitive thinks the board is %d\n", Primitive(position)); //for debugging
	}


	for(i = 0; i < BOARDSIZE; i++)
	{
		if(PossibleMove(i, position, theBlankBurntOX))
		{
			if (burnType == 0) {
				aMove = EncodeTheMove(i,0);
				head = CreateMovelistNode(aMove, head);
			}
			else {
				/*IF it's a possible move, find the burn variations.*/
				for(j = 0; j < BOARDSIZE; j++)
				{
					if(PossibleBurn(piecePos, i, j, position, theBlankBurntOX))
					{
						/* IF it's a possible burn, and the burn isn't
						 * the same as the move, add the encoded move
						 * to the movelist.*/
						aMove = EncodeTheMove(i, j);
						head = CreateMovelistNode(aMove, head);
						if (kPrintDebug)
						{
							printf("Move: %d, burn: %d, together %d\n", i, j, aMove); //for debugging
						}
					}
				}
			}
		}
	}
	return (head);
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
/* 10/19 altered print string... we've got some small problems ahead
 * (2-part move..)
 */
USERINPUT GetAndPrintPlayersMove(thePosition, theMove, playerName)
POSITION thePosition;
MOVE *theMove;
STRING playerName;
{
	USERINPUT ret;

	do {
		if (burnType==0) {
			printf("%8s's move [(u)ndo/ 1-%d]: ", playerName, BOARDSIZE);
		}
		else {
			printf("%8s's move [(u)ndo/ 1-%d 1-%d]: ", playerName, BOARDSIZE, BOARDSIZE);
		}

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
**              from the numbers 1-20, and the user chooses 0, it's not
**              valid, but anything from 1-20 IS, regardless if the slot
**              is filled or not. Whether the slot is filled is left up
**              to another routine. The move must also have the burn move.
**
** INPUTS:      STRING input : The string input the user typed.
**
** OUTPUTS:     BOOLEAN : TRUE iff the input is a valid text input.
**
************************************************************************/

BOOLEAN ValidTextInput(input)
STRING input;
{
	int move=-1, burn=-1;

	if (burnType==0) {
		sscanf(input, "%d", &move);
		return (move>0 && move<=BOARDSIZE);
	}
	else {
		sscanf(input, "%d %d", &move, &burn);
		return (move>0 && move<=BOARDSIZE && burn>0 && burn<=BOARDSIZE);
	}
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
/* Isaac: 10/18 */
MOVE ConvertTextInputToMove(input)
STRING input;
{
	int move, burn;

	if (burnType==0) {
		sscanf(input, "%d", &move);
		burn = 1;
	}
	else {
		sscanf(input, "%d %d", &move, &burn);
	}

/* This version has the first move input as the first 5 bits. */
	return (EncodeTheMove(move-1, burn-1));
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
	STRING moveStr = (STRING) SafeMalloc(8);

	int burn;
	int move;
	burn = ExtractBurn(theMove);
	move = ExtractMove(theMove);

	/* The plus 1 is because the user thinks it's 1-9, but MOVE is 0-8 */
	if (burnType==0) {
		sprintf(moveStr, "[%d]", move + 1);
	}
	else {
		sprintf(moveStr, "[%d %d]", move + 1, burn + 1);
	}

	return moveStr;
}

int NumberOfOptions() {
	int fixedOptions = 2 * 5 * 5 * 5 * 5 * 7;
	int boardOptions = 0;
	int i,j;

	for (i=1; i<=MAXBOARDSIZE; i++) {
		for (j=1; j<=MAXBOARDSIZE; j++) {
			if (i*j<=MAXBOARDSIZE && i<=j)
				boardOptions++;
		}
	}

	return fixedOptions * boardOptions;
}

int getOption() {
	int option = 0;
	int burnOption;
	int boardOption = 0;
	BOOLEAN done = FALSE;
	int i,j;

	for (i=1; i<=MAXBOARDSIZE && !done; i++) {
		for (j=1; j<=MAXBOARDSIZE && !done; j++) {
			if (i==ROWSIZE && j==COLUMNSIZE)
				done = TRUE;
			else if (i*j<=MAXBOARDSIZE && i<=j)
				boardOption++;
		}
	}

	option += boardOption;
	option *= 7;

	if (burnType==0) {
		burnOption = 0;
	}
	else if (burnType==1) {
		burnOption = 1;
		if (doubleTrouble)
			burnOption++;
	}
	else if (burnType==2) {
		burnOption = 3;
		if (doubleTrouble)
			burnOption++;
		if (gBurnMove)
			burnOption += 2;
	}
	else {
		BadElse("NumberOfOptions");
	}

	option += burnOption;
	option *= 5;

	option += OBurn;
	option *= 5;

	option += Otype;
	option *= 5;

	option += XBurn;
	option *= 5;

	option += Xtype;
	option *= 2;

	if (!gStandardGame) { option++; }

	option++;

	return option;
}

void setOption(int option) {
	int burnOption;
	int i,j;
	BOOLEAN done = FALSE;

	option--;
	gStandardGame = option%2==0;
	option /= 2;

	Xtype = option%5;
	option /= 5;

	XBurn = option%5;
	option /= 5;

	Otype = option%5;
	option /= 5;

	OBurn = option%5;
	option /= 5;

	burnOption = option%7;

	if (burnOption == 0) burnType = 0;
	else if (burnOption <= 2) {
		burnType = 1;
		doubleTrouble = (burnOption-1 == 1);
	}
	else if (burnOption <= 6) {
		burnType = 2;
		doubleTrouble = ((burnOption-3)%2 == 1);
		gBurnMove = ((burnOption-3)/2 == 1);
	}
	else
		BadElse("setOption");

	option /= 7;

	for (i=1; i<=MAXBOARDSIZE && !done; i++) {
		for (j=1; j<=MAXBOARDSIZE && !done; j++) {
			if (i*j<=MAXBOARDSIZE && i<=j) {
				if (option==0) {
					ROWSIZE = i;
					COLUMNSIZE = j;
					done = TRUE;
				}
				else {
					option--;
				}
			}
		}
	}
}

/************************************************************************
*************************************************************************
**         EVERYTHING BELOW THESE LINES IS LOCAL TO THIS
* FILE
*************************************************************************
************************************************************************/

/************************************************************************
** NAME:       GetXFromPosition
**
** Desc:       Returns X's location from the Position.  The X is in
**             the 7-11 bits (after turn bit and 5 O bits).
**
** INPUT:      POSITION thePos    : the position input.
**
***********************************************************************/
unsigned int GetXFromPosition(thePos)
POSITION thePos;
{
	return (thePos >> (BOARDSIZE-2+1)) % BOARDSIZE;
}

/************************************************************************
** NAME:       GetOFromPosition
**
** Desc:       Returns O's location from the Position.  The O is in
**             the 2-6 bits (after turn bit).
**
** INPUT:      POSITION thePos    : the position input.
**
***********************************************************************/
unsigned int GetOFromPosition(thePos)
POSITION thePos;
{
	POSITION xPos, rawPos, rawOPos;
	rawPos = (thePos >> (BOARDSIZE-2+1));
	xPos = (rawPos % BOARDSIZE);
	rawOPos = (rawPos / BOARDSIZE);
	if (rawOPos >= xPos) {
		return rawOPos+1;
	} else {
		return rawOPos;
	}
}


/************************************************************************
** NAME:       GetPFromPosition
**
** Desc:       Returns current Player's location from the Position.  The X is in
**             the 7-11 bits (after turn bit and 5 O bits).
**
** INPUT:      POSITION thePos    : the position input.
**
***********************************************************************/
unsigned int GetPFromPosition (POSITION position){
	unsigned int piecePos;

	if(WhoseTurn(position) == x)
	{
		piecePos = GetXFromPosition(position);
	}
	else
	{
		piecePos = GetOFromPosition(position);
	}
	return (piecePos);
}

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

/* Dave: Added a complete rewrite of Hash function: PositionToBlankBurntOX.
 * It uses bitwise operations rather than a g3Array, storing data inside
 * an integer.  First bit is turn bit, next 5 bits is o's position
 * (0-20),
 * second 5 bits is x's position (0-20), last 20 bits are whether a
 * square is occupied or not
 * (0-1). */

void PositionToBlankBurntOX(thePos,theBlankBurntOX)
POSITION thePos;
BlankBurntOX *theBlankBurntOX;
{
	int i;
	int emptyCount = 0;

	int TheXSpot = (thePos >> (BOARDSIZE-2+1)) % (BOARDSIZE);
	int TheOSpot;

	TheOSpot = (thePos >> (BOARDSIZE-2+1)) / (BOARDSIZE);
	if (TheOSpot>=TheXSpot) {
		TheOSpot++;
	}

	theBlankBurntOX[TheXSpot] = x;
	theBlankBurntOX[TheOSpot] = o;

	for(i = 0; i < BOARDSIZE; i++) {
		if (i != TheXSpot && i != TheOSpot) {
			theBlankBurntOX[i] = (((thePos >> (1+emptyCount))&1)==1 ? Burnt : Blank);
			emptyCount++;
		}
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


/* Takes in a board configuration theBlankBurntOX, which
 * is in the form of an array with each element indicating
 * what is in each square.  From this, return a hashed number
 * unique to the board configuration stored in an int data type. */

POSITION BlankBurntOXToPosition(theBlankBurntOX, whoseTurn)
BlankBurntOX *theBlankBurntOX, whoseTurn;
{
	POSITION position = 0;
	unsigned int i;
	int emptycount = 0;
	BOOLEAN foundX = FALSE;
	// Set occupied spots, and x,o locations.
	for(i = 0; i < BOARDSIZE; i++)
	{
		if((int)theBlankBurntOX[i] == x)
		{
			position += (i << (BOARDSIZE-2+1));
			foundX = TRUE;
		}
		else if((int)theBlankBurntOX[i] == o)
		{
			if (foundX) {
				position += (((i-1)*BOARDSIZE) << (BOARDSIZE-2+1));
			} else {
				position += ((i*BOARDSIZE) << (BOARDSIZE-2+1));
			}
		}
		else if((int)theBlankBurntOX[i] == Burnt)
		{
			position |= (1 << (emptycount+1));
			emptycount++;
		}
		else if((int)theBlankBurntOX[i] == Blank)
		{
			emptycount++;
		}
		else
			BadElse("Not possible board piece.");
	}
	/* set turn bit.*/
	if (whoseTurn == o) {
		position |= 1;
	}
	else
	{} /* Do nothing (its already set to 0) */
	return(position);
}


/************************************************************************
**
** NAME:        WhoseTurn
**
** DESCRIPTION: Return whose turn it is - either x or o. Since x always
**              goes first, we know that if the board has an even number
**              of burnt squares, it's x
**
** INPUTS:      BlankOX theBlankOX : The input board
**
** OUTPUTS:     (BlankBurntOX) : Either x or o, depending on whose turn it is
**
************************************************************************/
// Dave: Modified 10/7
BlankBurntOX WhoseTurn(thePosition)
POSITION thePosition;
{
	if((thePosition & 1) == 0) {
		return(x);
	} /* In our game, x always goes first */
	else{
		return(o);
	}
}


/**************************************************
* Function: CanMove
*   Takes in a Position and determines if the
*   player who goes next has any valid moves for
*   for their piecetype.
*   For use with Primitive.
**************************************************/

BOOLEAN CanMove(POSITION position)
{
	int i;
	int j;
	int piecePos;
	BlankBurntOX theBlankBurntOX[BOARDSIZE];
	PositionToBlankBurntOX(position, theBlankBurntOX);

	piecePos = GetPFromPosition(position);

	for(i = 0; i < BOARDSIZE; i++)
	{
		if(PossibleMove(i, position, theBlankBurntOX))
		{
			if (burnType == 0) {
				return TRUE;
			}
			else {
				/*IF it's a possible move, find the burn variations.*/
				for(j = 0; j < BOARDSIZE; j++)
				{
					if(PossibleBurn(piecePos, i, j, position, theBlankBurntOX))
					{
						/* IF it's a possible burn, and the burn isn't
						 * the same as the move, add the encoded move
						 * to the movelist.*/
						return TRUE;
					}
				}
			}
		}
	}
	return FALSE;
}

/**************************************************/
/*
     c1 c2 c3 c4
   r1      a- -b
   r2   c -  |  - d
       thePos
   r3   e -  |  - f
   r4      g- -h

 */
BOOLEAN CanKnightMove(BlankBurntOX theBlankBurntOX[], int r, int c){
	int r1 = r - 2;
	int r2 = r - 1;
	int r3 = r + 1;
	int r4 = r + 2;
	int c1 = c - 2;
	int c2 = c - 1;
	int c3 = c + 1;
	int c4 = c + 2;

	if (kPrintDebug)
		printf("\nCanKnight wanted to move from %d %d\n", r, c);

	if(    (r1 < 0 || c2 <  0 || theBlankBurntOX[r1*COLUMNSIZE + c2] != Blank)
	       && (r1 < 0 || c3 >= COLUMNSIZE || theBlankBurntOX[r1*COLUMNSIZE + c3] != Blank)
	       && (r2 < 0 || c1 <  0 || theBlankBurntOX[r2*COLUMNSIZE + c1] != Blank)
	       && (r2 < 0 || c4 >= COLUMNSIZE || theBlankBurntOX[r2*COLUMNSIZE + c4 ] != Blank)
	       && (r3 >= ROWSIZE || c1 < 0 || theBlankBurntOX[r3*COLUMNSIZE + c1] != Blank)
	       && (r3 >= ROWSIZE || c4 >= COLUMNSIZE || theBlankBurntOX[r3*COLUMNSIZE + c4] != Blank)
	       && (r4 >= ROWSIZE || c3 >= COLUMNSIZE || theBlankBurntOX[r4*COLUMNSIZE + c3] != Blank)
	       && (r4 >= ROWSIZE || c2 < 0 || theBlankBurntOX[r4*COLUMNSIZE + c2] != Blank)
	       )
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

/**************************************************/
/*
    c1  c2  c3
   r1      a
   r2  b thePos c
   r3      d

 */
BOOLEAN CanRookMove(BlankBurntOX theBlankBurntOX[], int r, int c){
	int r1 = r - 1;
	int r3 = r + 1;
	int c1 = c - 1;
	int c3 = c + 1;

	if (kPrintDebug)
		printf("\nCanRook wanted to move from %d %d\n", r, c);

	if((theBlankBurntOX[r1*COLUMNSIZE + c ] == Blank && r1 >= 0)
	   ||
	   (theBlankBurntOX[r3*COLUMNSIZE + c ] == Blank && r3 <= ROWSIZE)
	   ||
	   (theBlankBurntOX[r*COLUMNSIZE + c1 ] == Blank && c1 >= 0)
	   ||
	   (theBlankBurntOX[r*COLUMNSIZE + c3 ] == Blank && c3 <= COLUMNSIZE))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/**************************************************/
/*
    c1  c2  c3
   r1   a      b
   r2    thePos
   r3   c      d

 */
BOOLEAN CanBishopMove(BlankBurntOX theBlankBurntOX[], int r, int c){
	int r1 = r - 1;
	int r3 = r + 1;
	int c1 = c - 1;
	int c3 = c + 1;

	if (kPrintDebug)
		printf("\nCanBishop wanted to move from %d %d\n", r, c);

	if((theBlankBurntOX[r1*COLUMNSIZE + c1] == Blank && r1 >= 0 && c1 >= 0)
	   ||
	   (theBlankBurntOX[r3*COLUMNSIZE + c3] == Blank && r3 <= ROWSIZE && c3 <= COLUMNSIZE)
	   ||
	   (theBlankBurntOX[r1*COLUMNSIZE + c3] == Blank && r1 >= 0 && c3 <= COLUMNSIZE)
	   ||
	   (theBlankBurntOX[r3*COLUMNSIZE + c1] == Blank && r3 <= ROWSIZE && c1 >= 0 ))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/************************************************************************
**
** NAME:        PieceMessage
**
** DESCRIPTION: Prints a message based on the Player's piece
**
** INPUTS:      POSITION gInitialPosition (uses this to know whose
**              piece to look at it)
**
** OUTPUTS:     Just changes the piece type of the player whose turn
**              it is.
**
************************************************************************/
void PieceMessage(POSITION thePos, BOOLEAN DisplayPieceType){

	Piecetype thePiece;

	if(WhoseTurn(thePos) == x)
	{
		if(DisplayPieceType)
			thePiece = Xtype;
		else
			thePiece = XBurn;
	}
	else
	{
		if(DisplayPieceType)
			thePiece = Otype;
		else
			thePiece = OBurn;
	}

	switch(thePiece) {
	case king:
		printf("King");
		break;
	case queen:
		printf("Queen");
		break;
	case rook:
		printf("Rook");
		break;
	case bishop:
		printf("Bishop");
		break;
	case knight:
		printf("Knight");
		break;
	default:
		printf("Mystery Piece");
		break;
	};
}

/* Got tired of rewriting the switch
 * this takes either case of K, Q, R, B, or N
 * and changes thePiece to match the choice.*/
void ChangeThePiece(char input){
	Piecetype thePiece;

	switch(input) {
	case 'k': case 'K':
		thePiece = king;
		break;
	case 'q': case 'Q':
		thePiece = queen;
		break;
	case 'r': case 'R':
		thePiece = rook;
		break;
	case 'b': case 'B':
		thePiece = bishop;
		break;
	case 'n': case 'N':
		thePiece = knight;
		break;
	default:
		thePiece = knight;
		break;
	};
}

/************************************************************************
**
** NAME:        ChangePiece
**
** DESCRIPTION: Set the Player's piece and method of throwing
**
** INPUTS:      POSITION thePos (uses this to know whose
**              piece to look at it)
**
** OUTPUTS:     Just changes the global piece type and burn type of the
**              player whose turn it is.
**
************************************************************************/

void ChangePiece(POSITION thePos){

	Piecetype thePiece, theBurn;

	if(WhoseTurn(thePos) == x)
	{
		printf("X ");
		thePiece = Xtype;
		theBurn = XBurn;
	}
	else
	{
		printf("O ");
		thePiece = Otype;
		theBurn = OBurn;
	}

	printf("moves like a ");
	PieceMessage(thePos, TRUE);
	printf(" and stings like a ");
	PieceMessage(thePos, FALSE);
/* change movement */
	printf(".\nChoose your piece: K, Q, R, B, N\n");
	ChangeThePiece(GetMyChar());
	if(WhoseTurn(thePos) == x)
	{
		Xtype = thePiece;
	}
	else
	{
		Otype = thePiece;
	}
/* change burning */
	printf("Choose your sting: K, Q, R, B, N\n");
	ChangeThePiece(GetMyChar());
	printf("\nNow ");
	if(WhoseTurn(thePos) == x)
	{
		XBurn = thePiece;
		printf("X ");
	}
	else
	{
		OBurn = thePiece;
		printf("O ");
	}

	printf("moves like a ");
	PieceMessage(thePos, TRUE);
	printf(" and stings like a ");
	PieceMessage(thePos, FALSE);
	printf(".\n");
}

void ChangePieces(){
	ChangePiece(gInitialPosition);
	ChangePiece(ChangeTurn(gInitialPosition));
}

void ChangeBurn()
{
	char doubleYes;
	printf("There are 3 burntypes:\n1. Burned Bridges: %s\n2. Ultimate Grenade: %s\n3. Piece-Specific Grenade: %s \n", kBBExplanation, kUGExplanation, kBMExplanation);
	printf("Choose 1, 2, or 3: ");
	switch(GetMyChar()) {
	case '1':
		burnType = 0;
		break;
	case '2':
		burnType = 1;
		break;
	case '3':
		burnType = 2;
		break;
	default:
		BadElse("Nice Try.");
		break;
	}
	printf("This last option is not for the faint of heart.\n");
	printf("\nWould you like to burn your previous spots\nAS WELL AS a spot of your choosing? (Y,N) ");
	doubleYes = GetMyChar();

	if(doubleYes == 'y' || doubleYes == 'Y')
	{
		doubleTrouble = TRUE;
		printf("\nNOW You're playing with Fire!");
	}
	else
	{ doubleTrouble = FALSE;
	  printf("\nThat's ok, not everyone likes to take risks\n"); }
}

/* for changing the Board size--> this creates and returns
 * a new initial position that is correct for the new size*/
int GenerateNewInitial(){
	BlankBurntOX NewInitial[BOARDSIZE];
	int i, j;
	for(i=0; i < ROWSIZE; i++)
	{
		for(j=0; j < COLUMNSIZE; j++)
		{
			NewInitial[i*ROWSIZE + j] = Blank;
		}
	}

	NewInitial[BOARDSIZE-1] = x;
	NewInitial[0] = o;
	return (BlankBurntOXToPosition(NewInitial, x));
}

/* Change the Order of Burning and Moving */
void ChangeOrder(){
	gBurnMove = !gBurnMove;
	if(gBurnMove && (burnType == 2))
		printf("Order: Burn based on current position, then move");
	else if(!gBurnMove && burnType == 2)
		printf("Order: Move, then burn based on the new position");
	else if(burnType == 0)
		printf("Order: Move, then burn the old position");
	else if(burnType == 1)
		printf("Order: Move, then burn anywhere you can.");
}

void PrintExample(){
	int i, j;

	for(i = 0; i < ROWSIZE; i++)
	{
		for(j = 0; j < COLUMNSIZE; j++)
		{
			if(i == 0 && j == 0)
				printf("0 ");
			else if(i == (ROWSIZE - 1) && j == (COLUMNSIZE - 1))
				printf("X ");
			else if(i == 1 && j == (COLUMNSIZE - 1))
				printf("-             <----- EXAMPLE\n");
			else if(j == (COLUMNSIZE - 1))
				printf("-\n");
			else
				printf("- ");
		}
	}
}

void ChangeBoardSize(){
	int i, j;

	do {
		printf("You may not play with more than 20 spaces\non the board!\n");
		printf("# of Rows? ");
		i = GetMyChar() - '0';
		printf("# of Columns? ");
		j = GetMyChar() - '0';
		if (i*j<1 || i*j>20)
			printf("Invalid board size!\n");
		printf("\n");
	} while (i*j<1 || i*j>20);

	COLUMNSIZE = j;
	ROWSIZE = i;

	BOARDSIZE = COLUMNSIZE * ROWSIZE;

	if(BOARDSIZE <= 16)
		BITSIZE = 4;
	else
		BITSIZE = 5;

	if(kPrintDebug)
		printf("ROWSIZE %d, COLUMNSIZE %d BITSIZE %d",
		       ROWSIZE, COLUMNSIZE, BITSIZE);
	printf("Your New Board has %d spaces", BOARDSIZE);

	InitializeGame();
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
