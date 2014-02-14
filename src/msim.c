/************************************************************************
**
** NAME:        msim.c
**
** DESCRIPTION: SIM
**
** AUTHOR:      Dan Garcia & Sunil Ramesh et. al -  University of California at Berkeley
**              Copyright (C) Dan Garcia, 1995. All rights reserved.
**
** DATE:        2002-10-29
**
** UPDATE HIST:
**
** Decided to check out how much space was wasted with the array:
**
** Symmetries implemented, Ilya Landa
**************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gamesman.h"

extern STRING gValueString[];

POSITION gNumberOfPositions  = 14348907;  /* 3^15 */

POSITION gInitialPosition    =  0;
POSITION gMinimalPosition    = 0;

STRING kAuthorName          = "Dan Garcia and Sunil Ramesh et. al";
STRING kGameName            = "SIM";
BOOLEAN kPartizan            = TRUE;
BOOLEAN kDebugMenu           = TRUE;
BOOLEAN kGameSpecificMenu    = FALSE;
BOOLEAN kTieIsPossible       = TRUE;
BOOLEAN kLoopy               = FALSE;
BOOLEAN kDebugDetermineValue = FALSE;
POSITION kBadPosition           = -1;
void*    gGameSpecificTclInit = NULL;
STRING kHelpGraphicInterface =
        "";

STRING kHelpTextInterface    =
        "Place one of your pieces by selecting two endpoints of a line segment\n\
that does not yet have a piece on it."                                                                                  ;

STRING kHelpOnYourTurn =
        "You place one of your pieces on one of the empty board positions.";

STRING kHelpStandardObjective =
        "Force your opponent to form a triangle of three of your opponent's pieces.";

STRING kHelpReverseObjective =
        "Form a triangle of three of your pieces.";

STRING kHelpTieOccursWhen =   /* Should follow 'A Tie occurs when... */
                            "Neither player forms a triangle of three pieces of the same type.";

STRING kHelpExample =
        "\n\
  (Computer should Lose in 15)  \n\
Computer's move              : 36\n\
                                                            \n\
                                                            \n\
                                                            \n\
                5                         X6                \n\
                                         XX                 \n\
                                        XX                  \n\
                                       XX                   \n\
                                      XX                    \n\
                                     XX                     \n\
                                    X                       \n\
                                  XX                        \n\
                                 XX                         \n\
                                XX                          \n\
                               XX                           \n\
                              XX                            \n\
   4                         XX                          1  \n\
                           XX                               \n\
                          XX                                \n\
                         XX                                 \n\
                        XX                                  \n\
                       XX                                   \n\
                      XX                                    \n\
                     X                                      \n\
                   XX                                       \n\
                  XX                                        \n\
                 XX                                         \n\
                3X                         2                \n\
                                                            \n\
                                                            \n\
                                                            \n\
  (Player should Win in 14)  \n\
  Player's move [(u)ndo/[1-6][1-6]] :  {12} \n\
                                                            \n\
                                                            \n\
                                                            \n\
                5                         X6                \n\
                                         XX                 \n\
                                        XX                  \n\
                                       XX                   \n\
                                      XX                    \n\
                                     XX                     \n\
                                    X                       \n\
                                  XX                        \n\
                                 XX                         \n\
                                XX                          \n\
                               XX                           \n\
                              XX                            \n\
   4                         XX                        OO1  \n\
                           XX                         OO    \n\
                          XX                         OO     \n\
                         XX                         OO      \n\
                        XX                         OO       \n\
                       XX                         OO        \n\
                      XX                         OO         \n\
                     X                         OO           \n\
                   XX                         OO            \n\
                  XX                         OO             \n\
                 XX                         OO              \n\
                3X                         2O               \n\
                                                            \n\
                                                            \n\
                                                            \n\
  (Computer should Lose in 13)  \n\
Computer's move              : 46\n\
..."                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                ;

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

#define BOARDSIZE     15           /* # Edges = 15 */
#define VERTICES      6

typedef enum possibleBoardPieces { Blank, o, x } BlankOX;

char *gBlankOXString[] = { "-", "O", "X" };

/* Powers of 3 - this is the way I encode the position, as an integer */
int g3Array[] =          { 1, 3, 9, 27, 81, 243, 729, 2187, 6561, 19683, 59049, 177147, 531441, 1594323, 4782969 };

void PositionToBlankOX(POSITION thePos,BlankOX *theBlankOX);

STRING MoveToString( MOVE );
POSITION GetCanonical (POSITION p);
/************************************************************************
**
** NAME:        InitializeDatabases
**
** DESCRIPTION: Initialize the gDatabase, a global variable.
**
************************************************************************/

void InitializeGame()
{
	gMoveToStringFunPtr = &MoveToString;
	gCanonicalPosition = GetCanonical;
}

void FreeGame()
{
}

/************************************************************************
**
** NAME:        DebugMenu
**
** DESCRIPTION: Menu used to debub internal problems. Does nothing if
**              kDebugMenu == FALSE
**
************************************************************************/

void DebugMenu()
{
	char GetMyChar();

	do {
		printf("\n\t----- Module DEBUGGER for %s -----\n\n", kGameName);

		printf("\n\n\tb)\t(B)ack = Return to previous activity.\n");
		printf("\n\nSelect an option: ");

		switch(GetMyChar())
		{
		case 'Q': case 'q':
			ExitStageRight();
		case 'H': case 'h':
			HelpMenus();
			break;
		case 'B': case 'b':
			return;
		default:
			BadMenuChoice();
			HitAnyKeyToContinue();
			break;
		}
	} while(TRUE);

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
** CALLS:       PositionToBlankOX(POSITION,*BlankOX)
**              BlankOX WhosTurn(*BlankOX)
**
************************************************************************/

POSITION DoMove(thePosition, theMove)
POSITION thePosition;
MOVE theMove;
{
	BlankOX theBlankOX[BOARDSIZE], WhoseTurn();

	PositionToBlankOX(thePosition,theBlankOX);

	return(thePosition + (g3Array[theMove] * (int)WhoseTurn(theBlankOX)));
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
	POSITION BlankOXToPosition();
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
		else
			; /* do nothing */
	}

	return(BlankOXToPosition(theBlankOX,whosTurn));
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

int gInternalToUserMove[] =
{ 12, 13, 14, 15, 16, 23, 24, 25, 26, 34, 35, 36, 45, 46, 56 };

int gUserToInternalMove[] =
{
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1,  0,  1,  2,  3,  4, -1, -1, -1,
	-1,  0, -1,  5,  6,  7,  8, -1, -1, -1,
	-1,  1,  5, -1,  9, 10, 11, -1, -1, -1,
	-1,  2,  6,  9, -1, 12, 13, -1, -1, -1,
	-1,  3,  7, 10, 12, -1, 14, -1, -1, -1,
	-1,  4,  8, 11, 13, 14
};

void PrintComputersMove(computersMove,computersName)
MOVE computersMove;
STRING computersName;
{
	printf("%8s's move              : %2d\n", computersName, gInternalToUserMove[computersMove]);
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
	BOOLEAN ThreeInARow(), AllFilledIn(), Triangle();
	BlankOX theBlankOX[BOARDSIZE];

	PositionToBlankOX(position,theBlankOX);

	if( Triangle(theBlankOX,0,5,1)  ||
	    Triangle(theBlankOX,0,6,2)  ||
	    Triangle(theBlankOX,0,7,3)  ||
	    Triangle(theBlankOX,0,8,4)  ||
	    Triangle(theBlankOX,1,9,2)  ||
	    Triangle(theBlankOX,1,10,3) ||
	    Triangle(theBlankOX,1,11,4) ||
	    Triangle(theBlankOX,2,12,3) ||
	    Triangle(theBlankOX,2,13,4) ||
	    Triangle(theBlankOX,3,14,4) ||
	    Triangle(theBlankOX,5,9,6)  ||
	    Triangle(theBlankOX,5,10,7) ||
	    Triangle(theBlankOX,5,11,8) ||
	    Triangle(theBlankOX,6,12,7) ||
	    Triangle(theBlankOX,6,13,8) ||
	    Triangle(theBlankOX,7,14,8)||
	    Triangle(theBlankOX,9,12,10)||
	    Triangle(theBlankOX,9,13,11)||
	    Triangle(theBlankOX,10,14,11)||
	    Triangle(theBlankOX,12,14,13)
	    )
		return(gStandardGame ? win : lose);
//  else if(AllFilledIn(theBlankOX))
	// WILL I EVER COME HERE ?? ... NO
//    return(tie);
	else
		return(undecided);
}

/******************************************/
/*Crazy helper functions for printposition*/
/******************************************/


void cleararray( char** disp, int width, int height );
void printarray( char** disp, int width, int height );
void line( char** disp, int width, int height, float x1, float y1, float x2, float y2, char c );
void DrawSimBoard( char** disp, int width, int height );
void DrawMove( char** disp, int width, int height, int from, int to, char c );

#define PI (3.14159265)

#define caWidth 60
#define caHeight 30

#define LOOPY for( y=0; y<height; y++ )
#define LOOPX for( x=0; x<width; x++ )
#define LOOP  LOOPY LOOPX


void cleararray( char** disp, int width, int height )
{
	int x, y;
	LOOP disp[x][y] = ' ';
}

void printarray( char** disp, int width, int height )
{
	int x,y;
	LOOPY {
		LOOPX putchar( disp[x][y] );
		putchar( '\n' );
	}
}

void line( char** disp, int width, int height, float x1, float y1, float x2, float y2, char c )
{
	float t=0;
	for( t=0; t<=1; t+=(float)1/((float)width+(float)height) ) {
		float x,y;
		x = x1*(1-t)+x2*t;
		y = y1*(1-t)+y2*t;
		if( x>=0 && x<width && y>=0 && y<height )
			disp[(int)(x+.05)][(int)(y+.05)] = c;
	}
}

void DrawMove( char** disp, int width, int height, int from, int to, char c )
{
	float x1 = (.9)*(width/2)*cos(2*PI*(float)from/6) + width/2;
	float y1 = (.9)*(height/2)*sin(2*PI*(float)from/6) + height/2;
	float x2 = (.9)*(width/2)*cos(2*PI*(float)to/6) + width/2;
	float y2 = (.9)*(height/2)*sin(2*PI*(float)to/6) + height/2;

	from--;
	to--;
	line( disp, width, height, x1, y1, x2, y2, c );
	DrawSimBoard( disp, width, height );
}

void DrawSimBoard( char** disp, int width, int height )
{
	int i;
	float min = height;
	if( width<height ) min = width;

	for( i=0; i<6; i++ )
	{
		float x = (.9)*(width/2)*cos(2*PI*(float)i/6) + width/2;
		float y = (.9)*(height/2)*sin(2*PI*(float)i/6) + height/2;

		line( disp, width, height, x, y, x, y, '1'+i );
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

void PrintPosition(position,playerName,usersTurn)
POSITION position;
STRING playerName;
BOOLEAN usersTurn;
{
	int i;
	//  VALUE GetValueOfPosition();
	BlankOX theBlankOx[BOARDSIZE];
	char** disp;
	POSITION originalPosition = position;

	disp = (char**) malloc( sizeof(char*)*caWidth );
	for(i=0; i<caWidth; i++)
		disp[i] = (char*) malloc( sizeof(char)*caHeight );

	PositionToBlankOX(position,theBlankOx);



	cleararray( disp, caWidth, caHeight );

	for(i = 0; i < BOARDSIZE; i++)
	{
		if( position%3 ) DrawMove( disp, caWidth, caHeight,
			                   gInternalToUserMove[i]%10-1,
			                   gInternalToUserMove[i]/10-1,
			                   gBlankOXString[position%3][0] );
		position/=3;
	}

	printarray( disp, caWidth, caHeight );

	for(i=0; i<caWidth; i++)
		free( disp[i] );
	free(disp);

	printf("  %s\n", GetPrediction(originalPosition, playerName, usersTurn) );
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
	MOVELIST *CreateMovelistNode(), *head = NULL;
	VALUE Primitive();
	BlankOX theBlankOX[BOARDSIZE];
	int i;

	if (Primitive(position) == undecided) {
		PositionToBlankOX(position,theBlankOX);
		for(i = 0; i < BOARDSIZE; i++) {
			if(theBlankOX[i] == Blank)
				head = CreateMovelistNode(i,head);
		}
		return(head);
	} else {
		return(NULL);
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

USERINPUT GetAndPrintPlayersMove(thePosition, theMove, playerName)
POSITION thePosition;
MOVE *theMove;
STRING playerName;
{
	USERINPUT ret, HandleDefaultTextInput();

	do {
		printf("%8s's move [(u)ndo/[1-6][1-6]] :  ", playerName);

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
	return ((input[0] <= '6' && input[0] >= '1') && (input[1] <= '6' && input[1] >= '1') && (input[0] != input[1]));
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
	/* We get in "25", and return 7 ... etc.. */
	return gUserToInternalMove[(input[0]-'0')*10 + (input[1]-'0')];
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
	STRING m = (STRING) SafeMalloc( 3 );
	/* The plus 1 is because the user thinks it's 1-9, but MOVE is 0-8 */
	sprintf(m, "%d", gInternalToUserMove[theMove]);
	return m;
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

void PositionToBlankOX(thePos,theBlankOX)
POSITION thePos;
BlankOX *theBlankOX;
{
	int i;
	for(i = BOARDSIZE-1; i >= 0; i--) {
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

POSITION BlankOXToPosition(theBlankOX)
BlankOX *theBlankOX;
{
	int i;
	POSITION position = 0;

	for(i = 0; i < BOARDSIZE; i++)
		position += g3Array[i] * (int)theBlankOX[i]; /* was (int)position... */

	return(position);
}

/************************************************************************
**
** NAME:        Triangle
**
** DESCRIPTION: Return TRUE iff there is a triangle.
**
** INPUTS:      BlankOX theBlankOX[BOARDSIZE] : The BlankOX array.
**              int a,b,c                     : The 3 positions to check.
**
** OUTPUTS:     (BOOLEAN) TRUE iff there are three-in-a-row.
**
************************************************************************/

BOOLEAN Triangle(theBlankOX,a,b,c)
BlankOX theBlankOX[];
int a,b,c;
{
	return(       theBlankOX[a] == theBlankOX[b] &&
	              theBlankOX[b] == theBlankOX[c] &&
	              theBlankOX[c] != Blank );
}

/************************************************************************
**
** NAME:        AllFilledIn
**
** DESCRIPTION: Return TRUE iff all the blanks are filled in.
**
** INPUTS:      BlankOX theBlankOX[BOARDSIZE] : The BlankOX array.
**
** OUTPUTS:     (BOOLEAN) TRUE iff all the blanks are filled in.
**
************************************************************************/

BOOLEAN AllFilledIn(theBlankOX)
BlankOX theBlankOX[];
{
	BOOLEAN answer = TRUE;
	int i;

	for(i = 0; i < BOARDSIZE; i++)
		answer &= (theBlankOX[i] == o || theBlankOX[i] == x);

	return(answer);
}

/************************************************************************
**
** NAME:        WhoseTurn
**
** DESCRIPTION: Return whose turn it is - either x or o. Since x always
**              goes first, we know that if the board has an equal number
**              of x's and o's, that it's x's turn. Otherwise it's o's.
**
** INPUTS:      BlankOX theBlankOX : The input board
**
** OUTPUTS:     (BlankOX) : Either x or o, depending on whose turn it is
**
************************************************************************/

BlankOX WhoseTurn(theBlankOX)
BlankOX *theBlankOX;
{
	int i, xcount = 0, ocount = 0;

	for(i = 0; i < BOARDSIZE; i++)
		if(theBlankOX[i] == x)
			xcount++;
		else if(theBlankOX[i] == o)
			ocount++;
		else ;    /* don't count blanks */

	if(xcount == ocount)
		return(x);  /* in our TicTacToe, x always goes first */
	else
		return(o);
}


STRING kDBName = "sim";

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


/**********************************************************************
*                       SYMMERTY FUNCTIONS
* The following are the functions needed for the implementation
*   of symmetries.
*
* GetCanonical function is at the very bottom
**********************************************************************/

// This function returns an index of connection a-b in the
//   BlankOX board[] array
short connection_lookup(short a, short b){
	short temp, result = 0;
	if ((a == b) || (a < 1) || (a > 6) || (b < 1) || (b > 6)) {
		printf("Invalid arguments to connection_lookup: %d, %d\n", a, b);
		return 99;
	}
	if (a > b) {
		temp = a;
		a = b;
		b = temp;
	}
	for (temp = a; temp > 1; temp--)
		result += 7 - temp;
	result += b - a - 1;
	return result;
}

// This function determines whether <x> is present in
//   <array[]> prior to index <endInd>
char presBefore(short array[], short x, short endInd){
	short i;
	for (i = 0; i < endInd; i++)
		if (array[i] == x)
			return 1;
	return 0;
}

// This function fills all positions in <array[]> starting
//   at a position <sInd> with elements [1 ... VERTICES]
//   in an incrementing order without using elements already
//   present in <array[]> prior to position <sInd>
void fillEnd(short array[], short sInd){
	while (sInd < VERTICES) {
		array[sInd] = 1;
		while (presBefore(array, array[sInd], sInd))
			array[sInd]++;
		sInd++;
	}
	return;
}

// This function determines whether elements in <array[]>
//   are present in a decrementing order
// Returns 1 - Yes, 0 - No
char isDecr(short array[], short sInd){
	short cur = array[sInd], ind = sInd + 1;
	while (ind < VERTICES) {
		if (cur < array[ind])
			return 0;
		cur = array[ind];
		ind++;
	}
	return 1;
}

// This function takes <array[]> and changes it into its next
//   permutation.
// An array {1,2,3,4,5,6}, after 720 calls to this function
//   will be changed to {6,5,4,3,2,1}
void permute(short array[]){
	short end = VERTICES - 1;
	while (end >= 0) {
		if (array[end] == VERTICES)
			end--;
		if (!isDecr(array, end)) {
			do {
				array[end]++;
			} while (presBefore(array, array[end], end));
			fillEnd(array, end+1);
			return;
		}
		end--;
	}
	return;
}

// This function takes <old_board> and copies it into <new_boards>,
//   changing the order of vertices as indicated in <new_order>
void change_board(BlankOX* old_board, BlankOX* new_board, short new_order[]){
	short i = 0, a, b, x, y;
	for (x = 0; x < VERTICES - 1; x++)
		for (y = x+1; y < VERTICES; y++) {
			a = new_order[x];
			b = new_order[y];
			new_board[i++] = old_board[connection_lookup(a, b)];
		}
}

/************************************************************************
**
** NAME:        GetCanonical
**
** DESCRIPTION: This function tests all possible permutation of
**                vertices in a board derived from <p>.
**              The permutation with the smallest hash value is
**                returned as a canonical form of <p>
**
** INPUTS:      POSITION p - the original position
**
** OUTPUTS:     POSITION - the game position with the smallest num value
**
************************************************************************/
POSITION GetCanonical (POSITION p){
	POSITION testP, canonP = p;
	BlankOX posDecoded[BOARDSIZE];
	BlankOX testDecoded[BOARDSIZE];
	short vert_order[VERTICES] = {1,2,3,4,5,6};
	short perm = 6 * 5 * 4 * 3 * 2 - 1, i;
	PositionToBlankOX(p,posDecoded);
	for (i = 0; i < perm; i++) {
		permute(vert_order);
		change_board(posDecoded, testDecoded, vert_order);
		testP = BlankOXToPosition(testDecoded);
		if (testP < canonP)
			canonP = testP;
	}
	return canonP;
}

POSITION StringToPosition(char* board) {
  BlankOX theBlankOx[BOARDSIZE];
  int i;
  for(i = 0; i < BOARDSIZE; i++){
    if(board[i] == 'o')
      theBlankOx[i] = o;
    else if(board[i] == 'x')
      theBlankOx[i] = x;
  else if(board[i] == ' ')
      theBlankOx[i] = Blank;
  }
  return BlankOXToPosition(theBlankOx);
}


char* PositionToString(POSITION position) {
  int i;
  BlankOX theBlankOx[BOARDSIZE];
  PositionToBlankOX(position,theBlankOx);
  char * board = SafeMalloc(BOARDSIZE + 1);

  for(i = 0; i < BOARDSIZE; i++){
    if(theBlankOx[i] == o)
      board[i] = 'o';
    else if(theBlankOx[i] == x)
      board[i] = 'x';
    else if(theBlankOx[i] == Blank)
      board[i] = ' ';
  }
  board[BOARDSIZE] = '\0';

  return board;
}

char * PositionToEndData(POSITION pos) {
	return NULL;
}
