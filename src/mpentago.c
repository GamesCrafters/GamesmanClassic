/************************************************************************
**
** NAME:        mpentago.c
**
** DESCRIPTION: Pentago
**
** AUTHOR:      Jun Kang Chin
**              David Wu
**
** DATE:        Started: 2006-09-24
**
** UPDATE HIST:
**
** LAST CHANGE: 2006/10/28 Tweaked around with Printing and Board representation
**              2006/11/25 Fixed Primitives and Rotate
**              $Id$
**
**************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/

#include <stdio.h>
#include "gamesman.h"
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include "hash.h"

/*************************************************************************
**
** Game-specific constants
**
**************************************************************************/

STRING kGameName            = "Pentago";   /* The name of your game */
STRING kAuthorName          = "Jun Kang Chin and David Wu";   /* Your name(s) */
STRING kDBName              = "pentago";   /* The name to store the database under */

BOOLEAN kPartizan            = TRUE;   /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN kGameSpecificMenu    = FALSE;   /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN kTieIsPossible       = TRUE;   /* TRUE if a tie is possible. FALSE if it is impossible.*/
BOOLEAN kLoopy               = FALSE;   /* TRUE if the game tree will have cycles (a rearranger style game). FALSE if it does not.*/

BOOLEAN kDebugMenu           = TRUE;   /* TRUE only when debugging. FALSE when on release. */
BOOLEAN kDebugDetermineValue = TRUE;   /* TRUE only when debugging. FALSE when on release. */

POSITION gNumberOfPositions   =  0; /* The number of total possible positions | If you are using our hash, this is given by the hash_init() function*/
POSITION gInitialPosition     =  0; /* The initial hashed position for your starting board */
POSITION kBadPosition         = -1; /* A position that will never be used */

void*    gGameSpecificTclInit = NULL;

/**
 * Help strings that are pretty self-explanatory
 * Strings than span more than one line should have backslashes (\) at the end of the line.
 * These help strings should be updated and dynamically changed using
 * InitializeHelpStrings()
 **/

STRING kHelpGraphicInterface =
        "Graphic Interface Unavailable";

STRING kHelpTextInterface =
        "Enter the coordinates of where you like to place your piece\n\
followed by quadrant letter and the direction you wish to rotate it.\n\
\n\
Coordinates: Columns are represented as letters & rows are represented \n\
             by numbers. \n\
Quadrants: Quadrants are labeled A - D. \n\
Rotation Direction: a for Anti-Clockwise 90 degree rotation.\n\
                    c for Clockwise 90 degree rotation.\n\
\n\
    +---+---+ +---+---+   +---+---+ +---+---+ \n\
  4 |   |   | |   |   |   |       | |       | \n\
    +---+---+ +---+---+   |   A   | |   B   | \n\
  3 |   |   | |   |   |   |       | |       | \n\
    +---+---+ +---+---+   +---+---+ +---+---+ \n\
    +---+---+ +---+---+   +---+---+ +---+---+ \n\
  2 |   |   | |   |   |   |       | |       | \n\
    +---+---+ +---+---+   |   C   | |   D   | \n\
  1 | x |   | |   |   |   |       | |       | \n\
    +---+---+ +---+---+   +---+---+ +---+---+ \n\
      a   b   c   d                            \n\
\n\
Example:  Place piece at A1 and rotate the 4th quadrant clockwise. \n\
          Input should look like a1dc \n\
"                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   ;

STRING kHelpOnYourTurn =
        "Place a piece into an empty coordiante and rotate a quadrant of your choice.\n\
Read help text interface for instructions on entering your move"                                                                                         ;

STRING kHelpStandardObjective =
        "To get the game winning number of your makers(either X or 0) in a row, \n\
either horizontally, vertically, or diagonally. For the original rules of \n\
 pentago, 5-in-a-row wins."                                                                                                                                                                  ;

STRING kHelpReverseObjective =
        "To force your opponent to get the game winning number of your makers\n\
(either X or 0) in a row, either horizontally, vertically, or diagonally.\n\
 For the original rules of pentago, 5-in-a-row wins."                                                                                                                                                              ;

STRING kHelpTieOccursWhen =
        "A tie occurs when the board is filled and  neither side is able to get \n\
a game winning number of markers in a row."                                                                                    ;

STRING kHelpExample =
        "4X4 Pentago Game: \n\
\n\
 Human Player move: a1ac \n\
   +---+---+ +---+---+   +---+---+ +---+---+\n\
 4 |   |   | |   |   |   |       | |       |\n\
   +---+---+ +---+---+   |   A   | |   B   |\n\
 3 |   |   | |   |   |   |       | |       |\n\
   +---+---+ +---+---+   +---+---+ +---+---+\n\
   +---+---+ +---+---+   +---+---+ +---+---+\n\
 2 |   |   | |   |   |   |       | |       |\n\
   +---+---+ +---+---+   |   C   | |   D   |\n\
 1 | x |   | |   |   |   |       | |       |\n\
   +---+---+ +---+---+   +---+---+ +---+---+\n\
     a   b     c   d \n\
\n\
 Computer Move: c1ca \n\
   +---+---+ +---+---+   +---+---+ +---+---+\n\
 4 |   |   | |   |   |   |       | |       |\n\
   +---+---+ +---+---+   |   A   | |   B   |\n\
 3 |   |   | |   |   |   |       | |       |\n\
   +---+---+ +---+---+   +---+---+ +---+---+\n\
   +---+---+ +---+---+   +---+---+ +---+---+\n\
 2 |   |   | |   |   |   |       | |       |\n\
   +---+---+ +---+---+   |   C   | |   D   |\n\
 1 |   | x | | o |   |   |       | |       |\n\
   +---+---+ +---+---+   +---+---+ +---+---+\n\
     a   b     c   d \n\
\n\
 Human Player Move: a2cc \n\
   +---+---+ +---+---+   +---+---+ +---+---+\n\
 4 |   |   | |   |   |   |       | |       |\n\
   +---+---+ +---+---+   |   A   | |   B   |\n\
 3 |   |   | |   |   |   |       | |       |\n\
   +---+---+ +---+---+   +---+---+ +---+---+\n\
   +---+---+ +---+---+   +---+---+ +---+---+\n\
 2 |   | x | |   |   |   |       | |       |\n\
   +---+---+ +---+---+   |   C   | |   D   |\n\
 1 | x |   | | o |   |   |       | |       |\n\
   +---+---+ +---+---+   +---+---+ +---+---+\n\
     a   b     c   d \n\
\n\
 Computer Move: d3bc \n\
   +---+---+ +---+---+   +---+---+ +---+---+\n\
 4 |   |   | |   |   |   |       | |       |\n\
   +---+---+ +---+---+   |   A   | |   B   |\n\
 3 |   |   | | o |   |   |       | |       |\n\
   +---+---+ +---+---+   +---+---+ +---+---+\n\
   +---+---+ +---+---+   +---+---+ +---+---+\n\
 2 |   | x | |   |   |   |       | |       |\n\
   +---+---+ +---+---+   |   C   | |   D   |\n\
 1 | x |   | | o |   |   |       | |       |\n\
   +---+---+ +---+---+   +---+---+ +---+---+\n\
     a   b     c   d \n\
\n\
Human Player Move: d3bc \n\
   +---+---+ +---+---+   +---+---+ +---+---+\n\
 4 |   |   | | o |   |   |       | |       |\n\
   +---+---+ +---+---+   |   A   | |   B   |\n\
 3 |   |   | | x |   |   |       | |       |\n\
   +---+---+ +---+---+   +---+---+ +---+---+\n\
   +---+---+ +---+---+   +---+---+ +---+---+\n\
 2 |   | x | |   |   |   |       | |       |\n\
   +---+---+ +---+---+   |   C   | |   D   |\n\
 1 | x |   | | o |   |   |       | |       |\n\
   +---+---+ +---+---+   +---+---+ +---+---+\n\
     a   b     c   d \n\
 Human Player wins!\n"                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        ;



/*************************************************************************
**
** #defines and structs
**
**************************************************************************/
#define DEBUG 0
#define printFormat 2  //available modes: 1, 2
#define moveFormat 1 //available modes: 1, 2

#define XPIECE 'x'
#define OPIECE 'o'
#define BLANKPIECE ' '
#define PLAYER1 1
#define PLAYER2 2


#define WIN 1
#define UNDECIDED 0

/*************************************************************************
**
** Global Variables
**
*************************************************************************/
int ROWSIZE = 4; /* This should be an even number greater than 2 */
int NthInARow = 3; /*               1 < n < ROWSIZE                */
int NthFromLocation;
int BOARDSIZE;  /* ROWSIZE * ROWSIZE */

//char* BOARD;  /* Board Representation is a 1D char array */


/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/

/* External */
#ifndef MEMWATCH
extern GENERIC_PTR      SafeMalloc ();
extern void             SafeFree ();
#endif

STRING                  MoveToString(MOVE move);

/* Mine */
//BOOLEAN isOnBoard(POSITION position);
BOOLEAN nInARow(POSITION position);
void rotate();
BOOLEAN AllFilledIn();
void InitializeBoard(char*);
int getRow(MOVE);
int getColumn(MOVE);
int getBoardToRotate(MOVE);
BOOLEAN isRotateClockwise(MOVE);
void printFormat1(char*);
void printFormat2(char*);
void rotateBoard(char*, int, BOOLEAN);
void printBoardBoardRow1(int);
void printBoardBoardRow2(int);
BOOLEAN canGoLeft(int);
BOOLEAN canGoRight(int);
BOOLEAN canGoUp(int);
BOOLEAN canGoDown(int);
int StepAndCheck(int, int, int, char*);
MOVE convertText1(STRING);
MOVE convertText2(STRING);

/************************************************************************
**
** NAME:        InitializeGame
**
** DESCRIPTION: Prepares the game for execution.
**              Initializes required variables.
**
************************************************************************/

void InitializeGame ()
{
	if (DEBUG) {
		printf("InitializeGame Start \n");
	}
	//    InitializeHelpStrings();
	BOARDSIZE = ROWSIZE * ROWSIZE;
	NthFromLocation = NthInARow - 1;
	char* initialBoard = (char *) SafeMalloc(BOARDSIZE * sizeof(char));

	/*
	   int pieces[] = {XPIECE, 0, BOARDSIZE/2,
	                OPIECE, 0, BOARDSIZE/2,
	                BLANKPIECE, 0, BOARDSIZE/2, -1};
	 */

	/*
	   int pieces[] = {BLANKPIECE, 0, BOARDSIZE,
	                XPIECE, 0, BOARDSIZE/2,
	                OPIECE, 0, BOARDSIZE/2, -1};
	 */

	/*******************************************
	   THE ORDER OF THE  PIECES ARRAY DOES NOT MATTER
	 **********************************************/




	int pieces[] = {BLANKPIECE, 0, BOARDSIZE,
		        OPIECE, 0, BOARDSIZE/2,
		        XPIECE, 0, BOARDSIZE/2, -1};


	gNumberOfPositions = (int) generic_hash_init(BOARDSIZE, pieces, NULL, 0);
	InitializeBoard(initialBoard);
	gInitialPosition = generic_hash_hash(initialBoard, PLAYER1);
	SafeFree(initialBoard);

	if (DEBUG) {
		printf("InitializeGame End \n");
	}
}


/************************************************************************
**
** NAME:        InitializeHelpStrings
**
** DESCRIPTION: Sets up the help strings based on chosen game options.
**
** NOTES:       Should be called whenever the game options are changed.
**              (e.g., InitializeGame() and GameSpecificMenu())
**
************************************************************************/
void InitializeHelpStrings ()
{
	if (DEBUG) {
		printf("InitializingHelpStrings Start\n");
	}

	kHelpGraphicInterface =
	        "";

	kHelpTextInterface =
	        "";

	kHelpOnYourTurn =
	        "";

	kHelpStandardObjective =
	        "";

	kHelpReverseObjective =
	        "";

	kHelpTieOccursWhen =
	        "A tie occurs when ...";

	kHelpExample =
	        "";

	gMoveToStringFunPtr = &MoveToString;


	if (DEBUG) {
		printf("InitializingHelpStrings End\n");
	}
}


/************************************************************************
**
** NAME:        GenerateMoves
**
** DESCRIPTION: Creates a linked list of every move that can be reached
**              from this position. Returns a pointer to the head of the
**              linked list.
**
** INPUTS:      POSITION position : Current position to generate moves
**
** OUTPUTS:     (MOVELIST *)      : A pointer to the first item of
**                                  the linked list of generated moves
**
** CALLS:       MOVELIST *CreateMovelistNode();
**
************************************************************************/

MOVELIST *GenerateMoves (POSITION position)
{
	if (DEBUG) {
		printf("GenerateMoves Start..\n");
	}

	MOVELIST *moves = NULL;

	char* currBOARD = (char*)SafeMalloc(BOARDSIZE * sizeof(char));

	generic_hash_unhash(position, currBOARD);

	MOVE move = 0;
	int col, row, board, i;

	for (i = 0; i < BOARDSIZE; i++) {
		if (currBOARD[i] == BLANKPIECE) {
			col = (i % ROWSIZE) * 10;
			row = i / ROWSIZE;
			for (board = 1; board <= 4; board++) { //used to include all boards in possible rotations
				move = col + row + (100 * board);
				moves = CreateMovelistNode(move, moves);
				move *= -1;
				moves = CreateMovelistNode(move, moves);
			}
		}
	}
	/* Use CreateMovelistNode(move, next) to 'cons' together a linked list */

	if (DEBUG) {
		printf("GenerateMoves End..\n");
	}
	return moves;
}

/************************************************************************
**
** NAME:        DoMove
**
** DESCRIPTION: Applies the move to the position.
**
** INPUTS:      POSITION position : The old position
**              MOVE     move     : The move to apply to the position
**
** OUTPUTS:     (POSITION)        : The position that results from move
**
** CALLS:       Some Board Hash Function
**              Some Board Unhash Function
**
*************************************************************************/

POSITION DoMove (POSITION position, MOVE move)
{
	if (DEBUG) {
		printf("DoMove Start..\n");
	}
	char * board = (char *) SafeMalloc(BOARDSIZE * sizeof(char));
	generic_hash_unhash(position, board);

	/* check this stuff out */
	int playerTurn = generic_hash_turn(position);
	BOOLEAN rotateCW;

	if (isRotateClockwise(move)) {
		rotateCW = TRUE;
	} else {
		rotateCW = FALSE;
		move *= -1;
	}

	int boardToRotate = getBoardToRotate(move);
	int row = getRow(move);
	int col = getColumn(move);


	/*player 1 is not necessarily the charactoral repr., but instead is just a 1 or 2 */
	/*
	   printf("col: %d, row: %d, boardToRotate: %d, Spot%d\n", col, row, boardToRotate, (row * ROWSIZE) + col);
	   printf("BOARD PRINT BEGIN\n");
	   printFormat1(board);
	   printf("playerTurn: %d, %c\n", playerTurn, playerTurn);
	   printf("BOARD PRINT END\n");
	 */


	/* CHECK THIS STUFF TOO */
	if (playerTurn == PLAYER1) {
		board[(row * ROWSIZE) + col] = XPIECE;
		//    printf("Row: %d, Col%d, Spot: %d\n", row, col, (row + ROWSIZE) + col);
	} else {
		board[(row * ROWSIZE) + col] = OPIECE;
	}

	rotateBoard(board, boardToRotate, rotateCW);

	//  int newMove = ((row * ROWSIZE) + col);
	//  BOARD[move] = turn ==PlayerX? 'x'='o';   //check to see if spot is free

	if (DEBUG) {
		printf("DoMove End..\n");
	}

	/* switching players */
	if (playerTurn == PLAYER1) {
		playerTurn = PLAYER2;
	} else {
		playerTurn = PLAYER1;
	}

	POSITION nextPosition = generic_hash_hash(board, playerTurn);
	SafeFree(board);
	return nextPosition;
}


/************************************************************************
**
** NAME:        Primitive
**
** DESCRIPTION: Returns the value of a position if it fulfills certain
**              'primitive' constraints.
**
**              Example: Tic-tac-toe - Last piece already placed
**
**              Case                                  Return Value
**              *********************************************************
**              Current player sees three in a row    lose
**              Entire board filled                   tie
**              All other cases                       undecided
**
** INPUTS:      POSITION position : The position to inspect.
**
** OUTPUTS:     (VALUE)           : one of
**                                  (win, lose, tie, undecided)
**
** CALLS:       None
**
************************************************************************/

/* YOU'LL HAVE TO FIX THIS GENERIC HASH TURN ISSUE WITH THE PLAYERS */




VALUE Primitive (POSITION position)
{
	if (DEBUG) {
		printf("Primitive Start..\n");
	}

	char* currBOARD = (char*)SafeMalloc(BOARDSIZE * sizeof(char));
	generic_hash_unhash(position, currBOARD);
	int PlayerNumber = generic_hash_turn(position);

	char currPlayer, oppPlayer;
	if (PlayerNumber == 1) {
		currPlayer = XPIECE;
		oppPlayer = OPIECE;
	} else {
		currPlayer = OPIECE;
		oppPlayer = XPIECE;
	}


	NthFromLocation = NthInARow - 1;
	int location, blanksOnBoard = 0;
	for (location = 0; location < BOARDSIZE; location++) {
		if (currBOARD[location] == BLANKPIECE) {
			blanksOnBoard++;
		} else {
			if (canGoLeft(location)) {
				if (StepAndCheck(location, -1, 0, currBOARD) == WIN) {
					if (currBOARD[location] == currPlayer) {
						if (DEBUG) {
							printf("%c wins going L\n", currPlayer);
						}
						return win;
					} else {
						if (DEBUG) {
							printf("%c wins going L\n", oppPlayer);
						}
						return lose;
					}
				}
			}
			if (canGoLeft(location) && canGoUp(location)) {
				if (StepAndCheck(location, -1, -1, currBOARD) == WIN) {
					if (currBOARD[location] == currPlayer) {
						if (DEBUG) {
							printf("%c wins going UL\n", currPlayer);
						}
						return win;
					} else {
						if (DEBUG) {
							printf("%c wins going UL\n", oppPlayer);
						}
						return lose;
					}
				}
			}
			if (canGoUp(location)) {
				if (StepAndCheck(location, 0, -1, currBOARD) == WIN) {
					if (currBOARD[location] == currPlayer) {
						if (DEBUG) {
							printf("%c wins going U\n", currPlayer);
						}
						return win;
					} else {
						if (DEBUG) {
							printf("%c wins going U\n", oppPlayer);
						}
						return lose;
					}
				}
			}
			if (canGoUp(location) & canGoRight(location)) {
				if (StepAndCheck(location, 1, -1, currBOARD) == WIN) {
					if (currBOARD[location] == currPlayer) {
						if (DEBUG) {
							printf("%c wins going UR\n", currPlayer);
						}
						return win;
					} else {
						if (DEBUG) {
							printf("%c wins going UR\n", oppPlayer);
						}
						return lose;
					}
				}
			}
			if (canGoRight(location)) {
				if (StepAndCheck(location, 1, 0, currBOARD) == WIN) {
					if (currBOARD[location] == currPlayer) {
						if (DEBUG) {
							printf("%c wins going R\n", currPlayer);
						}
						return win;
					} else {
						if (DEBUG) {
							printf("%c wins going R\n", oppPlayer);
						}
						return lose;
					}
				}
			}
			if (canGoRight(location) && canGoDown(location)) {
				if (StepAndCheck(location, 1, 1, currBOARD) == WIN) {
					if (currBOARD[location] == currPlayer) {
						if (DEBUG) {
							printf("%c wins going DR\n", currPlayer);
						}
						return win;
					} else {
						if (DEBUG) {
							printf("%c wins going DR\n", oppPlayer);
						}
						return lose;
					}
				}
			}
			if (canGoDown(location)) {
				if (StepAndCheck(location, 0, 1, currBOARD) == WIN) {
					if (currBOARD[location] == currPlayer) {
						if (DEBUG) {
							printf("%c wins going D\n", currPlayer);
						}
						return win;
					} else {
						if (DEBUG) {
							printf("%c wins going D\n", oppPlayer);
						}
						return lose;
					}
				}
			}
			if (canGoDown(location) && canGoLeft(location)) {
				if (StepAndCheck(location, -1, 1, currBOARD) == WIN) {
					if (currBOARD[location] == currPlayer) {
						if (DEBUG) {
							printf("%c wins going DL\n", currPlayer);
						}
						return win;
					} else {
						if (DEBUG) {
							printf("%c wins going DL\n", oppPlayer);
						}
						return lose;
					}
				}
			}
			//printf("\n");
		}
	}
	if (blanksOnBoard == 0) {
		if (DEBUG) {
			printf("TIE\n");
		}
		return tie;
	} else {
		if (DEBUG) {
			printf("Undecided");
		}
		return undecided;
	}

	if (DEBUG) {
		printf("Primitive End..\n");
	}
}


/************************************************************************
**
** NAME:        PrintPosition
**
** DESCRIPTION: Prints the position in a pretty format, including the
**              prediction of the game's outcome.
**
** INPUTS:      POSITION position    : The position to pretty print.
**              STRING   playersName : The name of the player.
**              BOOLEAN  usersTurn   : TRUE <==> it's a user's turn.
**
** CALLS:       Unhash()
**              GetPrediction()      : Returns the prediction of the game
**
************************************************************************/

void PrintPosition (POSITION position, STRING playersName, BOOLEAN usersTurn)
{
	if (DEBUG) {
		printf("PrintPosition Start..\n");
	}

	char * board = (char *) SafeMalloc(BOARDSIZE * sizeof(char));


	//  printf("hash start\n");
	generic_hash_unhash(position, board);
	//  printf("hash end\n");

	if (printFormat == 1) {
		printFormat1(board);
	} else {
		printFormat2(board);
	}


	if (DEBUG) {
		printf("PrintPosition End..\n");
	}
}


/************************************************************************
**
** NAME:        PrintComputersMove
**
** DESCRIPTION: Nicely formats the computers move.
**
** INPUTS:      MOVE    computersMove : The computer's move.
**              STRING  computersName : The computer's name.
**
************************************************************************/

void PrintComputersMove (MOVE computersMove, STRING computersName)
{
	if (DEBUG) {
		printf("PrintComputersMove Start..\n");
	}

	STRING str = MoveToString(computersMove);
	printf("%s's move: %s", computersName, str);
	SafeFree(str);


	if (DEBUG) {
		printf("PrintComputersMove End..\n");
	}
}


/************************************************************************
**
** NAME:        PrintMove
**
** DESCRIPTION: Prints the move in a nice format.
**
** INPUTS:      MOVE move         : The move to print.
**
************************************************************************/

void PrintMove (MOVE move)
{
	if (DEBUG) {
		printf("PrintMove Start..\n");
	}

	STRING str = MoveToString( move );
	printf( "%s", str );
	SafeFree( str );

	if (DEBUG) {
		printf("PrintMove End..\n");
	}
}


/************************************************************************
**
** NAME:        MoveToString
**
** DESCRIPTION: Returns the move as a STRING
**
** INPUTS:      MOVE *move         : The move to put into a string.
**
************************************************************************/

STRING MoveToString (MOVE move)
{
	if (DEBUG) {
		printf("MoveToString Start..\n");
	}

	int col, row, boardToRotate;
	BOOLEAN isRotateCW;


	isRotateCW = isRotateClockwise(move);
	if (!isRotateCW) {
		move *= -1;
	}
	col = getColumn(move);
	row = getRow(move);
	boardToRotate = getBoardToRotate(move);


	STRING stringMove = (STRING) SafeMalloc(27 * sizeof(char));

	//  printf("col: %d\n", col);

	if (isRotateCW) {
		if (moveFormat == 1) {
			sprintf(stringMove, "Col:%c, Row:%d, Rotate:%d cw\n", col + 'a', ROWSIZE - row, boardToRotate);
		} else if (moveFormat == 2) {
			sprintf(stringMove, "Col:%c, Row:%d, Rotate:%d - \n", col + 'a', ROWSIZE - row, boardToRotate);
		}
	} else {
		if (moveFormat == 1) {
			sprintf(stringMove, "Col:%c, Row:%d, Rotate:%d cc\n", col + 'a', ROWSIZE - row, boardToRotate);
		} else if (moveFormat == 2) {
			sprintf(stringMove, "Col:%c, Row:%d, Rotate:%d + \n", col + 'a', ROWSIZE - row, boardToRotate);
		}
	}

	return stringMove;


	if (DEBUG) {
		printf("MoveToString End..\n");
	}
}


/************************************************************************
**
** NAME:        GetAndPrintPlayersMove
**
** DESCRIPTION: Finds out if the player wishes to undo, abort, or use
**              some other gamesman option. The gamesman core does
**              most of the work here.
**
** INPUTS:      POSITION position    : Current position
**              MOVE     *move       : The move to fill with user's move.
**              STRING   playersName : Current Player's Name
**
** OUTPUTS:     USERINPUT          : One of
**                                   (Undo, Abort, Continue)
**
** CALLS:       USERINPUT HandleDefaultTextInput(POSITION, MOVE*, STRING)
**                                 : Gamesman Core Input Handling
**
************************************************************************/

USERINPUT GetAndPrintPlayersMove (POSITION position, MOVE *move, STRING playersName)
{
	USERINPUT input;
	USERINPUT HandleDefaultTextInput();

	for (;; ) {
		/***********************************************************
		* CHANGE THE LINE BELOW TO MATCH YOUR MOVE FORMAT
		***********************************************************/
		if (moveFormat == 1) {
			printf("%8s's move [(undo)/(Col Row Board Dir (ex: b21cw)] : ", playersName);
		} else if (moveFormat == 2) {
			printf("%8s's move [(undo)/(Col Row Board +/- (ex: b21+)] : ", playersName);
		}

		input = HandleDefaultTextInput(position, move, playersName);

		if (input != Continue)
			return input;
	}

	/* NOTREACHED */

	return Continue;
}


/************************************************************************
**
** NAME:        ValidTextInput
**
** DESCRIPTION: Rudimentary check to check if input is in the move form
**              you are expecting. Does not check if it is a valid move.
**              Only checks if it fits the move form.
**
**              Reserved Input Characters - DO NOT USE THESE ONE CHARACTER
**                                          COMMANDS IN YOUR GAME
**              ?, s, u, r, h, a, c, q
**                                          However, something like a3
**                                          is okay.
**
**              Example: Tic-tac-toe Move Format : Integer from 1 to 9
**                       Only integers between 1 to 9 are accepted
**                       regardless of board position.
**                       Moves will be checked by the core.
**
** INPUTS:      STRING input : The string input the user typed.
**
** OUTPUTS:     BOOLEAN      : TRUE if the input is a valid text input.
**
************************************************************************/

BOOLEAN ValidTextInput (STRING input)
{
	if (DEBUG) {
		printf("ValidTextInput Start..\n");
	}

	if (moveFormat == 1) {
		if ((input[0] >= 'a' && input[0] < 'a' + ROWSIZE) &&
		    (input[1] >= '1' && input[1] < '1' + ROWSIZE) &&
		    (input[2] >= '1' && input[1] <= '4') &&
		    (input[3] == 'c') &&
		    (input[4] == 'c' || input[4] == 'w')) {
			return TRUE;
		}
	} else if (moveFormat == 2) {
		if ((input[0] >= 'a' && input[0] < 'a' + ROWSIZE) &&
		    (input[1] >= '1' && input[1] < '1' + ROWSIZE) &&
		    (input[2] >= '1' && input[1] <= '4') &&
		    (input[3] == '+' || input[3] == '-')) {
			return TRUE;
		}
	}

	if (DEBUG) {
		printf("ValidTextInput End..\n");
	}

	return FALSE;
}


/************************************************************************
**
** NAME:        ConvertTextInputToMove
**
** DESCRIPTION: Converts the string input your internal move representation.
**              Gamesman already checked the move with ValidTextInput
**              and ValidMove.
**
** INPUTS:      STRING input : The VALID string input from the user.
**
** OUTPUTS:     MOVE         : Move converted from user input.
**
************************************************************************/

MOVE ConvertTextInputToMove (STRING input)
{
	if (DEBUG) {
		printf("ConvertTextInputToMove Start..\n");
	}

	MOVE move;

	if (moveFormat == 1) {
		move = convertText1(input);
	} else if (moveFormat == 2) {
		move = convertText2(input);
	}

	//  printf("TEST:%d,  %s\n", move, MoveToString(move));
	//  printf("col:%d, row:%d, board:%d\n", getColumn(move), getRow(move), getBoardToRotate(move));

	if (DEBUG) {
		printf("ConvertTextInputToMove End..\n");
	}
	return move;
}


/************************************************************************
**
** NAME:        GameSpecificMenu
**
** DESCRIPTION: Prints, receives, and sets game-specific parameters.
**
**              Examples
**              Board Size, Board Type
**
**              If kGameSpecificMenu == FALSE
**                   Gamesman will not enable GameSpecificMenu
**                   Gamesman will not call this function
**
**              Resets gNumberOfPositions if necessary
**
************************************************************************/

void GameSpecificMenu ()
{
	InitializeHelpStrings();
}


/************************************************************************
**
** NAME:        SetTclCGameSpecificOptions
**
** DESCRIPTION: Set the C game-specific options (called from Tcl)
**              Ignore if you don't care about Tcl for now.
**
************************************************************************/

void SetTclCGameSpecificOptions (int options[])
{

}


/************************************************************************
**
** NAME:        GetInitialPosition
**
** DESCRIPTION: Called when the user wishes to change the initial
**              position. Asks the user for an initial position.
**              Sets new user defined gInitialPosition and resets
**              gNumberOfPositions if necessary
**
** OUTPUTS:     POSITION : New Initial Position
**
************************************************************************/

POSITION GetInitialPosition ()
{
	//    return 0;
	return gInitialPosition; //haha what the?
}


/************************************************************************
**
** NAME:        NumberOfOptions
**
** DESCRIPTION: Calculates and returns the number of variants
**              your game supports.
**
** OUTPUTS:     int : Number of Game Variants
**
************************************************************************/

int NumberOfOptions ()
{
	return 0;
}


/************************************************************************
**
** NAME:        getOption
**
** DESCRIPTION: A hash function that returns a number corresponding
**              to the current variant of the game.
**              Each set of variants needs to have a different number.
**
** OUTPUTS:     int : the number representation of the options.
**
************************************************************************/

int getOption ()
{
	/* If you have implemented symmetries you should
	   include the boolean variable gSymmetries in your
	   hash */
	return 0;
}


/************************************************************************
**
** NAME:        setOption
**
** DESCRIPTION: The corresponding unhash function for game variants.
**              Unhashes option and sets the necessary variants.
**
** INPUT:       int option : the number representation of the options.
**
************************************************************************/

void setOption (int option)
{
	/* If you have implemented symmetries you should
	   include the boolean variable gSymmetries in your
	   hash */
}


/************************************************************************
**
** NAME:        DebugMenu
**
** DESCRIPTION: Game Specific Debug Menu (Gamesman comes with a default
**              debug menu). Menu used to debug internal problems.
**
**              If kDebugMenu == FALSE
**                   Gamesman will not display a debug menu option
**                   Gamesman will not call this function
**
************************************************************************/

void DebugMenu ()
{

}


/************************************************************************
**
** Everything specific to this module goes below these lines.
**
** Things you want down here:
** Move Hasher
** Move Unhasher
** Any other function you deem necessary to help the ones above.
**
************************************************************************/





/************************************************************************
**
** NAME:        InitializeBoard
**
** DESCRIPTION: Fill the global variable BOARD with EMPTY characters.
**
**
**
** INPUTS:      char*
**
** OUTPUTS:     void
**
************************************************************************/
void InitializeBoard(char* board) {
	if (DEBUG) {
		printf("InitializeBoard Start..\n");
	}

	int i;
	for (i = 0; i < BOARDSIZE; i++) {
		board[i] = BLANKPIECE;
	}


	if (DEBUG) {
		printf("InitializeBoard End..\n");
	}
}


/* MOVES */
/* moveFormat1
   col, row, board, cw/cc
   ex) a32cw            */
MOVE convertText1(STRING input) {
	int col, row, boardToRotate, isRotateCW;
	col = (input[0] - 'a');
	row = ROWSIZE - (input[1] - '0');
	boardToRotate = input[2] - '0';
	if (input[4] == 'w') {
		isRotateCW = 1;
	} else {
		isRotateCW = -1;
	}
	return (isRotateCW * (row + (col * 10) + (boardToRotate * 100)));
}

/* moveFormat 2
   col, row, board, +/-
   ex) b12+             */
MOVE convertText2(STRING input) {
	int col, row, boardToRotate, isRotateCW;
	col = (input[0] - 'a');
	row = ROWSIZE - (input[1] - '0');
	boardToRotate = input[2] - '0';
	if (input[3] == '-') {
		isRotateCW = 1;
	} else {
		isRotateCW = -1;
	}
	return (isRotateCW * (row + (col * 10) + (boardToRotate * 100)));
}



/* Parses move statements
   moves are represented as follows:
   hundreds = rotating board
   tens = column
   ones = row
   positive/negative = rotate clockwise, cc, respectively
 */

int getBoardToRotate(MOVE move) {
	//  printf("THIS IS THE MOVE: %d\n", move/100);
	return move/100;
}

int getColumn(MOVE move) {
	return (move % 100)/10;
}

int getRow(MOVE move) {
	return move % 10;
}

BOOLEAN isRotateClockwise(MOVE move) {
	if (move > 0) {
		return TRUE;
	} else {
		return FALSE;
	}
}


/* PrintFormat 1 */
/*********************************************************
  +---+---+---+---+---+---+   +---+---+---+---+---+---+
   6 | x   o   x | o   x   o |   |           |           |
|           |           |   |           |           |
   5 | x   o   x | o   x   o |   |     1     |     2     |
|           |           |   |           |           |
   4 | x   o   x | o   x   o |   |           |           |
|+---+---+---+---+---+---+   +---+---+---+---+---+---+
   3 | x   o   x | o   x   o |   |           |           |
|           |           |   |           |           |
   2 | x   o   x | o   x   o |   |     3     |     4     |
|           |           |   |           |           |
   1 | x   o   x | o   x   o |   |           |           |
|+---+---+---+---+---+---+   +---+---+---+---+---+---+
     a   b   c   d   e   f
*********************************************************/


void printFormat1(char* BOARD) {
	int i, j;
	int currRow = 1;
	int index = ROWSIZE;
	char letter = 'a';

	/* This is for printing the first row of +--+'s for the Piece board */
	for (i = 0; i < ROWSIZE; i++) {
		if (i == 0) {
			printf("   +---+");
		} else if (i == ROWSIZE - 1) {
			printf("---+");
		} else {
			printf("---+");
		}
	}

	/* leave a gap between the two boards */
	printf("   ");

	/* this is for printing the second row of +--+'s for the Board board */
	for (i = 0; i < ROWSIZE; i++) {
		if (i == 0) {
			printf("+---+");
		} else if (i == ROWSIZE - 1) {
			printf("---+\n");
		} else {
			printf("---+");
		}
	}

	/* This is for printing each successive entry */
	printf(" %d ", index--); /* print the number in chess-format */

	for (i = 0; i < BOARDSIZE; i++) {

		if (i % ROWSIZE == 0) {
			printf("| %c  ", BOARD[i]); /* print the start of each row */

		} else if ((i % ROWSIZE) + 1 == ROWSIZE / 2) {
			printf(" %c |", BOARD[i]);

		} else if ((i + 1) % ROWSIZE == 0) {
			printf(" %c |", BOARD[i]); /* print newline at the end of a row */

			printBoardBoardRow1(currRow++);

			printf("   "); /* print the number in chess-format */

			/* this is for printing the spaces in between rows */
			if (i != BOARDSIZE - 1) { /* exclude last row */
				if ((i / ROWSIZE) + 1 != ROWSIZE/2) {
					for (j = 0; j < ROWSIZE; j++) {
						if (j == 0) {
							printf("|   ");
						} else if (j == ROWSIZE - 1) {
							printf("    |");

							printBoardBoardRow1(currRow++);

							printf(" %d ", index--); /* print the number in chess-format */

						} else if ((j % ROWSIZE) == ROWSIZE / 2) {
							printf("|   ");
						} else {
							printf("    ");
						}
					}
				} else { /* This is for printing out the middle row */
					for (j = 0; j < ROWSIZE; j++) {
						if (j == 0) {
							printf("+---+");
						} else if (j == ROWSIZE - 1) {
							printf("---+");

							printBoardBoardRow1(currRow++);

							printf(" %d ", index--);

						} else {
							printf("---+");
						}
					}
				}
			}
		} else {
			printf(" %c  ", BOARD[i]);
		}
	}

	/* for printing out the last row */
	for (i = 0; i < ROWSIZE; i++) {
		if (i == 0) {
			printf("+---+");
		} else if (i == ROWSIZE - 1) {
			printf("---+");
		} else {
			printf("---+");
		}
	}

	/* this is for printing the second row of +--+'s for the Board board */
	for (i = 0; i < ROWSIZE; i++) {
		if (i == 0) {
			printf("   +---+");
		} else if (i == ROWSIZE - 1) {
			printf("---+\n");
		} else {
			printf("---+");
		}
	}

	/* for printing out the last set of letters */
	for (i = 0; i < ROWSIZE; i++) {
		if (i == 0) {
			printf("     %c ", letter++);
		} else if (i == ROWSIZE - 1) {
			printf("  %c\n", letter);
		} else {
			printf("  %c ", letter++);
		}
	}
}

void printBoardBoardRow1(int row) {
	int j = 0;
	printf("   ");
	if (row == ROWSIZE / 2) { /* for printing out the row with board numbers 1 and 2 */
		for (j = 0; j < ROWSIZE * 4; j++) {
			if (j == 0) {
				printf("| ");
			} else if (j == (ROWSIZE * 2) - 1 || j == (ROWSIZE * 4) - 1) {
				printf("|");
				if (j != ROWSIZE * 2 - 1) {
					printf("\n");
				}
			} else if (j == (ROWSIZE - 1)) {
				printf("1");
			} else if (j == (ROWSIZE * 3) - 1) {
				printf("2");
			} else {
				printf(" ");
			}
		}
	} else if (row == (ROWSIZE * 3 / 2)) { /* for printing out the row with board numbers 3 and 4 */
		for (j = 0; j < ROWSIZE * 4; j++) {
			if (j == 0) {
				printf("| ");
			} else if (j == (ROWSIZE * 2) - 1 || j == (ROWSIZE * 4) - 1) {
				printf("|");
				if (j != ROWSIZE * 2 - 1) {
					printf("\n");
				}
			} else if (j == (ROWSIZE - 1)) {
				printf("3");
			} else if (j == (ROWSIZE * 3) - 1) {
				printf("4");
			} else {
				printf(" ");
			}
		}

	} else if (row == ROWSIZE) { /* for the middle row */
		for (j = 0; j < ROWSIZE; j++) {
			if (j == 0) {
				printf("+---+");
			} else if (j == ROWSIZE-1) {
				printf("---+\n");
			} else {
				printf("---+");
			}
		}


	} else { /* for printing out the blank rows */
		for (j = 0; j < ROWSIZE * 4; j++) {
			if (j == 0) {
				printf("| ");
			} else if (j == (ROWSIZE * 2 - 1) || j == (ROWSIZE * 4) - 1) {
				printf("|");
				if (j != ROWSIZE * 2 - 1) {
					printf("\n");
				}
			} else {
				printf(" ");
			}
		}
	}
}

/* END OF PrintFormat1  */





/* PrintFormat2 */
/************************************************************
   +---+---+---+ +---+---+---+   +---+---+---+ +---+---+---+
   6 | x | o | x | | o | x | o |   |           | |           |
   +---+---+---+ +---+---+---+   |           | |           |
   5 | x | o | x | | o | x | o |   |     1     | |     2     |
   +---+---+---+ +---+---+---+   |           | |           |
   4 | x | o | x | | o | x | o |   |           | |           |
   +---+---+---+ +---+---+---+   +---+---+---+ +---+---+---+
   +---+---+---+ +---+---+---+   +---+---+---+ +---+---+---+
   3 | x | o | x | | o | x | o |   |           | |           |
   +---+---+---+ +---+---+---+   |           | |           |
   2 | x | o | x | | o | x | o |   |     3     | |     4     |
   +---+---+---+ +---+---+---+   |           | |           |
   1 | x | o | x | | o | x | o |   |           | |           |
   +---+---+---+ +---+---+---+   +---+---+---+ +---+---+---+
     a   b   c     d   e   f
 *************************************************************/
void printFormat2(char* BOARD) {
	int i, j;
	int currRow = 1;
	int index = ROWSIZE;
	int quadrant = ROWSIZE / 2;
	char letter = 'a';

	printf("\n");

	/* This is for printing the first row of +--+'s for the Piece board */
	for (i = 0; i < ROWSIZE; i++) {
		if (i == 0) {
			printf("   +---+");
		} else if (i == ROWSIZE - 1) {
			printf("---+");
		} else if (i == quadrant -1) {
			printf("---+ ");
		} else if (i == quadrant) {
			printf("+---+");
		} else {
			printf("---+");
		}
	}

	/* leave a gap between the two boards */
	printf("   ");

	/* this is for printing the second row of +--+'s for the Board board */
	for (i = 0; i < ROWSIZE; i++) {
		if (i == 0) {
			printf("+---+");
		} else if (i == ROWSIZE - 1) {
			printf("---+\n");
		} else if (i == quadrant -1) {
			printf("---+ ");
		} else if (i == quadrant) {
			printf("+---+");
		} else {
			printf("---+");
		}
	}

	/* This is for printing each successive entry */
	printf(" %d ", index--); /* print the number in chess-format */

	for (i = 0; i < BOARDSIZE; i++) {

		if (i % ROWSIZE == 0) {
			printf("| %c |", BOARD[i]); /* print the start of each row */

			/*
			   } else if ((i % ROWSIZE) + 1 == ROWSIZE / 2) {
			   printf(" %c |", BOARD[i]);
			 */
		} else if (i % quadrant == 0) { /* prints for the start of each quadrant */
			printf(" | %c |", BOARD[i]);
		} else if ((i + 1) % ROWSIZE == 0) {
			printf(" %c |", BOARD[i]); /* print newline at the end of a row */

			printBoardBoardRow2(currRow++);

			printf("   "); /* print the number in chess-format */

			/* this is for printing the spaces in between rows */
			if (i != BOARDSIZE - 1) { /* exclude last row */
				if ((i / ROWSIZE) + 1 != ROWSIZE/2) {
					for (j = 0; j < ROWSIZE; j++) {
						if (j == 0) {
							printf("+---+");
						} else if (j == ROWSIZE - 1) {
							printf("---+");
							printBoardBoardRow2(currRow++);
							printf(" %d ", index--); // print the number in chess-format
						} else if (j == quadrant -1) {
							printf("---+ ");
						} else if (j == quadrant) {
							printf("+---+");
						} else {
							printf("---+");
						}
					}
				} else { /* This is for printing out the middle row */
					for (j = 0; j < ROWSIZE; j++) {
						if (j == 0) {
							printf("+---+");
						} else if (j == quadrant) {
							printf(" +---+");
						} else if (j == ROWSIZE - 1) {
							printf("---+");

							printBoardBoardRow2(currRow);

							// printf(" %d ", index--);

						} else {
							printf("---+");
						}
					}
					//printf("\n");

					for (j = 0; j < ROWSIZE; j++) {
						if (j == 0) {
							printf("   +---+");
						} else if (j == quadrant) {
							printf(" +---+");
						} else if (j == ROWSIZE - 1) {
							printf("---+");

							printBoardBoardRow2(currRow++);

							printf(" %d ", index--);

						} else {
							printf("---+");
						}
					}
				}
			}
		} else {
			printf(" %c |", BOARD[i]);
		}
	}

	/* for printing out the last row */
	for (i = 0; i < ROWSIZE; i++) {
		if (i == 0) {
			printf("+---+");
		} else if (i == ROWSIZE - 1) {
			printf("---+");
		} else if (i == quadrant) {
			printf(" +---+");
		}  else {
			printf("---+");
		}
	}

	/* this is for printing the second row of +--+'s for the Board board */
	for (i = 0; i < ROWSIZE; i++) {
		if (i == 0) {
			printf("   +---+");
		} else if (i == quadrant) {
			printf(" +---+");
		} else if (i == ROWSIZE - 1) {
			printf("---+\n");
		} else {
			printf("---+");
		}
	}

	/* for printing out the last set of letters */
	for (i = 0; i < ROWSIZE; i++) {
		if (i == 0) {
			printf("     %c ", letter++);
		} else if (i == ROWSIZE - 1) {
			printf("  %c\n", letter);
		} else if (i == quadrant) {
			printf("    %c ", letter++);
		} else {
			printf("  %c ", letter++);
		}
	}
}

void printBoardBoardRow2(int row) {
	int quadrant = ROWSIZE / 2;
	int j = 0;
	printf("   ");
	if (row == ROWSIZE / 2) { /* for printing out the row with board numbers 1 and 2 */
		for (j = 0; j < ROWSIZE * 4; j++) {
			if (j == 0) {
				printf("| ");
			} else if (j == (ROWSIZE * 2 - 1)) {
				printf("| |");
			} else if (j == (ROWSIZE * 4) - 1) {
				printf("|");
				printf("\n");
			} else if (j == (ROWSIZE - 1)) {
				printf("1"); // 1
			} else if (j == (ROWSIZE * 3) - 1) {
				printf("2"); // 2
			} else {
				printf(" ");
			}
		}
	} else if (row == (ROWSIZE * 3 / 2)) { /* for printing out the row with board numbers 3 and 4 */
		for (j = 0; j < ROWSIZE * 4; j++) {
			if (j == 0) {
				printf("| ");
			} else if (j == (ROWSIZE * 2 - 1)) {
				printf("| |");
			} else if (j == (ROWSIZE * 4) - 1) {
				printf("|");
				printf("\n");
			} else if (j == (ROWSIZE - 1)) {
				printf("3"); // 3
			} else if (j == (ROWSIZE * 3) - 1) {
				printf("4"); // 4
			} else {
				printf(" ");
			}
		}

	} else if (row == ROWSIZE) { /* for the middle row */
		for (j = 0; j < ROWSIZE; j++) {
			if (j == 0) {
				printf("+---+");
			} else if (j == ROWSIZE-1) {
				printf("---+\n");
			} else if (j == quadrant) {
				printf(" +---+");
			} else {
				printf("---+");
			}
		}

	} else { /* for printing out the blank rows */
		for (j = 0; j < ROWSIZE * 4; j++) {
			if (j == 0) {
				printf("| ");
			} else if (j == (ROWSIZE * 2 - 1)) {
				printf("| |");
			} else if (j == (ROWSIZE * 4) - 1) {
				printf("|");
				printf("\n");
			} else {
				printf(" ");
			}
		}
	}
}

/* END OF PrintFormat 2 */


/* board should be the unhashed board, boardToRotate is the subboard to be
   rotated, and BOOLEAN CW determines whether the board is going to be
   rotated clockwise or counter-clockwise                               */
void rotateBoard(char* currBOARD, int boardToRotate, BOOLEAN CW) {
	int distance = (ROWSIZE / 2) - 1;
	int numOfSteps = distance;
	int ULcorner, URcorner, DRcorner, DLcorner;
	/* this is for determining the corners used to rotate the subboards */
	if (boardToRotate == 1) {
		ULcorner = 0;
		URcorner = distance;
	} else if (boardToRotate == 2) {
		ULcorner = ROWSIZE/2;
		URcorner = ULcorner + distance;
	} else if (boardToRotate == 3) {
		ULcorner = ROWSIZE * ROWSIZE / 2;
		URcorner = ULcorner + distance;
	} else if (boardToRotate == 4) {
		ULcorner = (ROWSIZE / 2) * (ROWSIZE + 1);
		URcorner = ULcorner + distance;
	} else {
		printf("NOT A BOARD NUMBER!!\n");
		if (DEBUG) {
			printf("boardToRotate: %d\n", boardToRotate);
		}
	}
	DRcorner = URcorner + (ROWSIZE * distance);
	DLcorner = ULcorner + (ROWSIZE * distance);
	/* end of corner assignment */
	if (CW) {
		while (URcorner > ULcorner) {
			int counter = 0;
			int oldUL = ULcorner, oldUR = URcorner, oldDR = DRcorner, oldDL = DLcorner;
			while (counter++ != numOfSteps) {
				char temp1, temp2;
				temp1 = currBOARD[URcorner];
				temp2 = currBOARD[DLcorner];
				currBOARD[URcorner] = currBOARD[ULcorner];
				currBOARD[DLcorner] = currBOARD[DRcorner];
				currBOARD[ULcorner] = temp2;
				currBOARD[DRcorner] = temp1;
				ULcorner++;
				URcorner += ROWSIZE;
				DRcorner--;
				DLcorner -= ROWSIZE;
			}
			/* resetting the values of the corner indexes */
			DLcorner = oldDL;
			DRcorner = oldDR;
			URcorner = oldUR;
			ULcorner = oldUL;
			/* changing the corner indexes for the next interation */
			ULcorner = ULcorner + 1 + ROWSIZE;
			URcorner = URcorner - 1 + ROWSIZE;
			DLcorner = DLcorner + 1 - ROWSIZE;
			DRcorner = DRcorner - 1 - ROWSIZE;
			numOfSteps -= 2;
		}
	} else {
		while (URcorner > ULcorner) {
			int counter = 0;
			int oldUL = ULcorner, oldUR = URcorner, oldDR = DRcorner, oldDL = DLcorner;
			while (counter++ != numOfSteps) {
				char temp1, temp2;
				temp1 = currBOARD[ULcorner];
				temp2 = currBOARD[DRcorner];
				currBOARD[ULcorner] = currBOARD[URcorner];
				currBOARD[DRcorner] = currBOARD[DLcorner];
				currBOARD[URcorner] = temp2;
				currBOARD[DLcorner] = temp1;
				ULcorner += ROWSIZE;
				URcorner--;
				DRcorner -= ROWSIZE;
				DLcorner++;
			}
			/* resetting the values of the corner indexes */
			DLcorner = oldDL;
			DRcorner = oldDR;
			URcorner = oldUR;
			ULcorner = oldUL;
			/* changing the corner indexes for the next interation */
			ULcorner = ULcorner + 1 + ROWSIZE;
			URcorner = URcorner - 1 + ROWSIZE;
			DLcorner = DLcorner + 1 - ROWSIZE;
			DRcorner = DRcorner - 1 - ROWSIZE;
			numOfSteps -= 2;
		}
	}
}










/* FOR PRIMITIVES */
int StepAndCheck(int location, int horizontalStep, int verticalStep, char* currBOARD) {
	char currPiece = currBOARD[location];
	int N = 0; /* used to see if a win has occured */
	while (currBOARD[location] == currPiece && N <= NthFromLocation) {
		//    printf("location: %d\n", location);
		N++;
		location += horizontalStep; /* for going left or right */
		location = location + (verticalStep * ROWSIZE); /* for going up or down */
	}
	if (N == NthInARow) {
		return WIN;
	} else {
		return UNDECIDED;
	}
}


BOOLEAN canGoLeft(int location) {
	if ((location - NthFromLocation) % ROWSIZE < (location % ROWSIZE) && (location - NthFromLocation) >= 0) {
		return TRUE;
	} else {
		return FALSE;
	}
}

BOOLEAN canGoRight(int location) {
	if ((location + NthFromLocation) % ROWSIZE > (location % ROWSIZE)) {
		return TRUE;
	} else {
		return FALSE;
	}
}

BOOLEAN canGoUp(int location) {
	if ((location - (NthFromLocation * ROWSIZE)) >= 0) {
		return TRUE;
	} else {
		return FALSE;
	}
}

BOOLEAN canGoDown(int location) {
	if ((location + (NthFromLocation * ROWSIZE)) < BOARDSIZE) {
		return TRUE;
	} else {
		return FALSE;
	}
}
/* END OF PRIMITIVE FUNCTIONS */


/************************************************************************
** Changelog
**
** $Log$
** Revision 1.3  2006/12/07 02:14:35  davidcwu
** C
**
** Revision 1.2  2006/10/28 09:24:37  davidcwu
** *** empty log message ***
**
** Revision 1.1  2006/10/23 07:15:54  davidcwu
** *** empty log message ***
**
** Revision 1.10  2006/04/25 01:33:06  ogren
** Added InitialiseHelpStrings() as an additional function for new game modules to write.  This allows dynamic changing of the help strings for every game without adding more bookkeeping to the core.  -Elmer
**
************************************************************************/

POSITION StringToPosition(char* board) {
	// FIXME: this is just a stub
	return atoi(board);
}


char* PositionToString(POSITION pos) {
	// FIXME: this is just a stub
	return "Implement Me";
}
