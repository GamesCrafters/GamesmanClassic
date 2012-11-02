// $id$
// $log$

/*
 * The above lines will include the name and log of the last person
 * to commit this file to CVS
 */

/************************************************************************
**
** NAME:        mshogi.c
**
** DESCRIPTION: Hasami Shogi
**
** AUTHOR:      Chris Willmore, Ann Chen
**
** DATE:        February 1, 2005 / FINISH DATE
**
** UPDATE HIST:
** 02/02/2005 Updated some game-specific constants.
** 02/02/2005 Added PrintPosition().
**
**************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/

#include <stdio.h>
#include "gamesman.h"
#include "hash.h"
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <ctype.h>

/*************************************************************************
**
** Game-specific constants
**
**************************************************************************/

void*   gGameSpecificTclInit = NULL;
STRING kGameName            = "Hasami Shogi";   /* The name of your game */
STRING kAuthorName          = "Chris Willmore, Ann Chen";   /* Your name(s) */
STRING kDBName              = "mhshogi";   /* The name to store the database under */

BOOLEAN kPartizan            = TRUE;   /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN kGameSpecificMenu    = TRUE;   /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN kTieIsPossible       = FALSE;   /* TRUE if a tie is possible. FALSE if it is impossible.*/
BOOLEAN kLoopy               = TRUE;   /* TRUE if the game tree will have cycles (a rearranger style game). FALSE if it does not.*/

BOOLEAN kDebugMenu           = FALSE;   /* TRUE only when debugging. FALSE when on release. */
BOOLEAN kDebugDetermineValue = FALSE;   /* TRUE only when debugging. FALSE when on release. */

POSITION gNumberOfPositions   =  0; /* The number of total possible positions | If you are using our hash, this is given by the hash_init() function*/
POSITION gInitialPosition     =  0; /* The initial hashed position for your starting board */
POSITION kBadPosition         = -1; /* A position that will never be used */

/*
 * Help strings that are pretty self-explanatory
 * Strings than span more than one line should have backslashes (\) at the end of the line.
 */

STRING kHelpGraphicInterface =
        "Not written yet";

STRING kHelpTextInterface    =
        "On your turn, lookup the horizontal and vertical position of the piece\n\
you want to move and the destination of the move.  The position\n\
notation is the same as chess notation, and you can find the positions\n\
labeled on the sides of the board. Then type in the position of the\n\
piece immediately followed by the destination without any spaces in\n\
between (e.g. a1a2). If at any point you have made a mistake, you can\n\
type u and hit return and the system will revert back to your most\n\
recent position."                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             ;

STRING kHelpOnYourTurn =
        "On your turn, move one or more spaces in any direction--forward,\n\
backward, left, right--everything except diagonal. All the spaces\n\
passed over and the one landed upon must be empty. You can jump over\n\
no more than a single piece, and it has to be adjacent to the jumper's\n\
starting spot. The jumped-over piece can belong to either player, and\n\
it is not removed afterwards. When you jump, you must land on the\n\
space adjacent (just beyond) to the jumped piece. That space has to be\n\
empty, and the jumper has to stop there. Just like moving, jumping can\n\
go in any direction excepty diagonal.  Capturing is done by making\n\
\"capture sandwiches.\" If you move or jump so that your piece becomes\n\
adjacent to one or more enemy pieces that are in a straight line\n\
(vertical or horizontal, it doesn't matter), and you also have another\n\
piece at the other end of the line, those in-between enemy piece(s) are\n\
captured and removed from the board. Note that there must be no empty\n\
spaces in the captured sandwich, which can be any length. It matters who\n\
completes the sandwich.  You may safely move a piece into a line\n\
between enemy pieces--forming a sandwich--so long as you build the\n\
sandwich, not your opponent.\n\
The War Version: An enemy piece located in a corner, or a connected\n\
line of pieces--of which one is in a corner--can be captured by your\n\
pieces on both ends of the line. This just means you can make right-\n\
angle sandwiches."                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     ;

STRING kHelpStandardObjective =
        "To make a chain of 5 connected pieces of your markers (either x or o)\n\
in a straight line. The chain can be in any direction--vertical,\n\
horizontal, or diagonal. None of the pieces may be in your original\n\
starting rows.\n\
The War Version: To capture all of the enemy pieces, or reduce the\n\
enemy to one or no pieces."                                                                                                                                                                                                                                                                                                                     ;

STRING kHelpReverseObjective =
        "To force your enemy into making a chain of 5 connected pieces of\n\
his/her markers (either x or o) in a straight line. The chain can be in\n\
any direction--vertical, horizontal, or diagonal. None of the pieces\n\
may be in his/her original starting rows.\n\
The War Version: To force your enemy into capturing all of your pieces,\n\
or reduce you to one or no pieces."                                                                                                                                                                                                                                                                                                                                                        ;

STRING kHelpTieOccursWhen =
        "both players have less than 5 pieces.\n\
The War Version: A tie never occurs."                                                  ;

STRING kHelpExample =
        "Player's turn\n\
  -----\n\
3 |o|o|\n\
2 | | |\n\
1 |x|x|\n\
  -----\n\
   a b\n\n\
Player's move [(u)ndo/a-b 0-3 a-b 0-3] :  a1a2\n\
Computer's turn\n\
  -----\n\
3 |o|o|\n\
2 |x| |\n\
1 | |x|\n\
  -----\n\
   a b\n\n\
Computer's move: b3b2\n\
Player's turn\n\
  -----\n\
3 |o| |\n\
2 |x|o|\n\
1 | |x|\n\
  -----\n\
   a b\n\n\
Player's move [(u)ndo/a-b 0-3 a-b 0-3] :  b1a1\n\
Computer's turn\n\
  -----\n\
3 |o| |\n\
2 |x|o|\n\
1 |x| |\n\
  -----\n\
   a b\n\n\
Computer's move: b2b1\n\
Player's turn\n\
  -----\n\
3 |o| |\n\
2 | | |\n\
1 | |o|\n\
  -----\n\
   a b\n\n\
Computer wins. Nice try, Player."                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       ;


/*************************************************************************
**
** #defines and structs
**
**************************************************************************/

#define Blank ' '
#define X 'x'
#define O 'o'
//#define VERSION_REGULAR 0
//#define VERSION_WAR 1
#define VERSION_LINE 0
#define VERSION_CAPTURE 1
#define VERSION_BOTH 2
#define VERSION_NO_CORNER 0
#define VERSION_CORNER 1

typedef char BlankOX;
/* Represents a complete shogi move */

typedef struct cleanMove {
	int fromX;
	int fromY;
	int toX;
	int toY;
} sMove;

/*************************************************************************
**
** Global Variables
**
*************************************************************************/

int numOfRows = 3;
int numOfCols = 3;
int rowsOfPieces = 1;
int numInRow = 3;
int winVersion = VERSION_LINE;
int captureVersion = VERSION_NO_CORNER;

int boardSize = 9;

/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/

BOOLEAN inARow(BlankOX theBlankOX[]);
BOOLEAN adjacent(BlankOX theBlankOX[], int current, int next);
void captureLine(int x, int y, BlankOX piece, BlankOX theBlankOX[]);
void captureCorner(int x, int y, BlankOX piece, BlankOX theBlankOX[]);
void clearRow(int x1, int x2, int y,  BlankOX theBlankOX[]);
void clearColumn(int y1, int y2, int x, BlankOX theBlankOX[] );
BlankOX oneOrNoPieces(BlankOX theBlankOX[]);
int BoardPosToArrayPos(int x, int y);
MOVE hashMove(unsigned int fromX, unsigned int fromY, unsigned int toX, unsigned int toY);
sMove unhashMove(MOVE move);


/* External */
#ifndef MEMWATCH
extern GENERIC_PTR      SafeMalloc ();
extern void             SafeFree ();
#endif


/************************************************************************
**
** NAME:        InitializeGame
**
** DESCRIPTION: Prepares the game for execution.
**              Initializes required variables.
**              Sets up gDatabase (if necessary).
**
************************************************************************/

void InitializeGame ()
{
	BlankOX theBlankOX[boardSize];
	int i;
	int pieces[] = {Blank, boardSize - 2*rowsOfPieces*numOfCols, boardSize,
		        X, 0, rowsOfPieces*numOfCols,
		        O, 0, rowsOfPieces*numOfCols, -1};

	for (i = 0; i < rowsOfPieces * numOfCols; i++) {
		theBlankOX[i] = X;
	}
	for (; i < numOfCols * (numOfRows - rowsOfPieces); i++) {
		theBlankOX[i] = Blank;
	}
	for (; i < boardSize; i++) {
		theBlankOX[i] = O;
	}

	gNumberOfPositions = generic_hash_init(boardSize, pieces, NULL, 0);
	gInitialPosition = generic_hash_hash(theBlankOX, 1);
}

/************************************************************************
**
** NAME:        GenerateMoves
**
** DESCRIPTION: Creates a linked list of every move that can be reached
**              from this position. Returns a pointer to the head of the
**              linked list.
**
** INPUTS:      POSITION position : Current position for move
**                                  generation.
**
** OUTPUTS:     (MOVELIST *)      : A pointer to the first item of
**                                  the linked list of generated moves
**
** CALLS:       MOVELIST *CreateMovelistNode();
**
************************************************************************/

MOVELIST *GenerateMoves(position)
POSITION position;
{
	BlankOX theBlankOX[boardSize];
	MOVELIST *head = NULL;

	char turn;
	int i, n, x, y, moveToX, moveToY;

	generic_hash_unhash(position, theBlankOX);

	if (generic_hash_turn(position) == 1)
		turn = X;
	else
		turn = O;

	for(x = 0; x < numOfCols; x++) {
		for (y = 0; y < numOfRows; y++) {
			i = BoardPosToArrayPos(x, y);
			if (theBlankOX[i] == turn) {
				//check for jumps
				//jump up
				if (((n = BoardPosToArrayPos(x, y-1)) > -1)
				    && theBlankOX[n] != Blank
				    && ((n = BoardPosToArrayPos(x, y-2)) > -1)
				    && theBlankOX[n] == Blank) {
					head = CreateMovelistNode(hashMove(x, y, x, y-2), head);
				}
				//jump right
				if (((n = BoardPosToArrayPos(x+1, y)) > -1)
				    && theBlankOX[n] != Blank
				    && ((n = BoardPosToArrayPos(x+2, y)) > -1)
				    && theBlankOX[n] == Blank) {
					head = CreateMovelistNode(hashMove(x, y, x+2, y), head);
				}
				//jump down
				if (((n = BoardPosToArrayPos(x, y+1)) > -1)
				    && theBlankOX[n] != Blank
				    && ((n = BoardPosToArrayPos(x, y+2)) > -1)
				    && theBlankOX[n] == Blank) {
					head = CreateMovelistNode(hashMove(x, y, x, y+2), head);
				}
				//jump left
				if (((n = BoardPosToArrayPos(x-1, y)) > -1)
				    && theBlankOX[n] != Blank
				    && ((n = BoardPosToArrayPos(x-2, y)) > -1)
				    && theBlankOX[n] == Blank) {
					head = CreateMovelistNode(hashMove(x, y, x-2, y), head);
				}



				//check for lateral move left
				moveToX = x-1;
				while((moveToX >= 0) &&
				      theBlankOX[BoardPosToArrayPos(moveToX, y)] == Blank) {
					head = CreateMovelistNode(hashMove(x, y, moveToX, y), head);
					moveToX--;
				}

				//check for lateral move right
				moveToX = x+1;
				while((moveToX < numOfCols) &&
				      theBlankOX[BoardPosToArrayPos(moveToX, y)] == Blank) {
					head = CreateMovelistNode(hashMove(x, y, moveToX, y), head);
					moveToX++;
				}
				//check for vertical move up
				moveToY = y-1;
				while((moveToY >= 0) &&
				      theBlankOX[BoardPosToArrayPos(x, moveToY)] == Blank) {
					head = CreateMovelistNode(hashMove(x, y, x, moveToY), head);
					moveToY--;
				}
				//check for vertical move down
				moveToY = y+1;
				while((moveToY <= numOfRows) &&
				      theBlankOX[BoardPosToArrayPos(x, moveToY)] == Blank) {
					head = CreateMovelistNode(hashMove(x, y, x, moveToY), head);
					moveToY++;
				}
			}
		}
	}
	return head;
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
	BlankOX theBlankOX[boardSize];
	BlankOX piece;
	sMove theMove = unhashMove(move);
	int fromLoc, toLoc, turn = generic_hash_turn(position);

	generic_hash_unhash(position, theBlankOX);
	fromLoc = (theMove.fromY) * numOfCols + theMove.fromX;
	toLoc = (theMove.toY) * numOfCols + theMove.toX;
	piece = theBlankOX[fromLoc];

	if (turn == 1)
		turn = 2;
	else
		turn = 1;

	theBlankOX[fromLoc] = Blank;
	theBlankOX[toLoc] = piece;

	captureLine(theMove.toX, theMove.toY, piece, theBlankOX);
	if(captureVersion == VERSION_CORNER)
		captureCorner(theMove.toX, theMove.toY, piece, theBlankOX);

	position = generic_hash_hash(theBlankOX, turn);
	return position;
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
** CALLS:       BOOLEAN inARow()
**              BlankOX oneOrNoPieces()
**
************************************************************************/

VALUE Primitive (POSITION position)
{
	BlankOX theBlankOX[boardSize];
	int turn = generic_hash_turn(position);

	generic_hash_unhash(position, theBlankOX);

	if(inARow(theBlankOX))
		return(gStandardGame ? lose : win);
	else if((winVersion == VERSION_CAPTURE || winVersion == VERSION_BOTH) &&
	        ((oneOrNoPieces(theBlankOX) == X && turn == 1) ||
	         (oneOrNoPieces(theBlankOX) == O && turn == 2)))
		return lose;
	else
		return undecided;
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
	BlankOX theBlankOX[boardSize];
	int row, col;
	generic_hash_unhash(position, theBlankOX);

	printf("%s's turn\n  ", playersName);
	for (col = 0; col < numOfCols; col++) {
		printf("--");
	}
	printf("-");
	for (row = numOfRows; row > 0; row--) {
		printf("\n%d |", row);
		for (col = 0; col < numOfCols; col++) {
			printf("%c|", theBlankOX[(row - 1) * numOfCols + col]);
		}
	}
	printf("\n  ");
	for (col = 0; col < numOfCols; col++) {
		printf("--");
	}
	printf("-\n  ");
	for (col = 0; col < numOfCols; col++) {
		printf(" %c", col + 'a');
	}
	printf("\n\n");
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
	printf("%s's move: ", computersName);
	PrintMove(computersMove);
	printf("\n");
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
	struct cleanMove x;
	x = unhashMove(move);

	printf("%c", x.fromX + 'a');
	printf("%d", x.fromY + 1);
	printf("%c", x.toX + 'a');
	printf("%d", x.toY + 1);
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
		printf("%s's move [(u)ndo/a-%c0-%da-%c0-%d] :  ",
		       playersName, numOfCols-1+'a', numOfRows,
		       numOfCols-1+'a', numOfRows);
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
	/******
	 * Check for valid input based upon chess notation
	 * example a1b2
	 */
	int strlen;
	for (strlen = 0; input[strlen] != -1; strlen++) {
	}

	//check to see if input is long enough
	if (strlen < 4) {
		return FALSE;
	}
	if(input[0] < 'a' || input[0] >= 'a' + numOfCols)
		return FALSE;
	if(input[1] < '1' || input[1] >= '1' + numOfRows)
		return FALSE;
	if(input[2] < 'a' || input[2] >= 'a' + numOfCols)
		return FALSE;
	if(input[3] < '1' || input[3] >= '1' + numOfRows)
		return FALSE;
	return TRUE;
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

	/*****
	 * Example conversions:
	 * a1c3 = 0022
	 * b3a5 = 1204
	 */
	int fromX = input[0] - 'a';
	int fromY = input[1] - '1';
	int toX = input[2] - 'a';
	int toY = input[3] - '1';

	return hashMove(fromX, fromY, toX, toY);

}


/************************************************************************
**
** Name:        GameSpecificMenu
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
	/*int *input;*/
	char option;
	STRING versionName;
	/*input = (int *) malloc(1*sizeof(int));?!?!?!?!?!!*/

	while(TRUE) {
		versionName = (captureVersion == VERSION_NO_CORNER ?
		               "corner" : "no corner");
		printf("\n  Game Specific Options:\n\n");
		printf("\tr)\t (R)ows in the board         -   [%d] rows\n",numOfRows);
		printf("\tc)\t (C)olumns in the board      -   [%d] cols\n",numOfCols);
		printf("\tp)\t (P)layer's rows of pieces   -   [%d] rows\n",rowsOfPieces);
		printf("\n");
		printf("\tv)\t Capturing (V)ersion         -   [%s] captures allowed\n",versionName);
		if (winVersion == VERSION_LINE)
			printf("\tw)\t (W)inning condition         -   [%d] in a row\n",numInRow);
		else if (winVersion == VERSION_CAPTURE)
			printf("\tw)\t (W)inning condition         -   reduce enemy to 1 or no pieces\n");
		else {
			printf("\tw)\t (W)inning conditions        -   [%d] in a row, or\n",numInRow);
			printf("\t  \t                                 reduce enemy to 1 or no pieces\n");
		}
		printf("\n");
		printf("\tb)\t (B)ack\n\n");
		printf("  Select an option: ");
		option = tolower(GetMyChar());
		if ('b' == option) {
			/*SafeFree(input);*/
			return;
		}
		if ('r' == option) {
			printf("Input the number of rows in the board: ");
			/*scanf("%d",input);*/
			numOfRows = GetMyInt(); /*input[0];*/
			boardSize = numOfRows * numOfCols;
			InitializeGame();
		} else if (!strcmp(option,"c")) {
			printf("Input the number of columns in the board: ");
			/*scanf("%d",input);*/
			numOfCols = GetMyInt(); /*input[0];*/
			boardSize = numOfRows * numOfCols;
			InitializeGame();
		} else if ('p' == option) {
			printf("Input the number of rows of pieces of one player: ");
			/*scanf("%d",input);*/
			rowsOfPieces = GetMyInt(); /*input[0];*/
			InitializeGame();
		} else if ('v' == option) {
			captureVersion = (captureVersion == VERSION_NO_CORNER ?
			                  VERSION_CAPTURE : VERSION_NO_CORNER);
		} else if ('w' == option) {
			printf("\n  Winning conditions:\n");
			printf("\tl)\t (L)ine up your pieces - [%d] in a row\n", numInRow);
			printf("\tr)\t (R)educe enemy to 1 or no pieces\n");
			printf("\tb)\t (B)oth\n\n");
			printf("  Select the winning conditions: ");
			option = tolower(GetMyChar());
			if ('l' == option) {
				winVersion = VERSION_LINE;
				printf("Input the number of pieces in a row to win: ");
				/*scanf("%d", input);*/
				numInRow = GetMyInt(); /*input[0];*/
			} else if ('r' == option) {
				winVersion = VERSION_CAPTURE;
			} else if ('b' == option) {
				winVersion = VERSION_BOTH;
				printf("Input the number of pieces in a row to win: ");
				/*scanf("%d", input);*/
				numInRow = GetMyInt(); /*input[0];*/
			} else {
				printf("\nSorry, I don't know that option.\n");
			}
		} else {
			printf("\nSorry, I don't know that option. Try another.\n");
		}
	}
	/*SafeFree(input);*/
	return;
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
	return gInitialPosition;
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
** NAME:        inARow
**
** DESCRIPTION: Return TRUE iff there are a given number of pieces in
**              a row. None of the pieces may be in the player's original
**              starting row(s).
**
** INPUTS:      BlankOX theBlankOX[boardSize] : The BlankOX array.
**
** OUTPUTS:     (BOOLEAN) TRUE iff there a given number of pieces in
**              a row. None of the pieces may be in the player's original
**              starting row(s).
**
************************************************************************/

BOOLEAN inARow(BlankOX theBlankOX[])
{
	int i, j, k, samePieces = 1, current, next;
	for (i = 0; i < numOfRows; i++) {
		for (j = 0; j < numOfCols - 1; j++) {
			current = i*numOfCols+j;
			next = current + 1;
			if (adjacent(theBlankOX, current, next)) {
				samePieces++;
				if (samePieces == numInRow)
					return TRUE;
			}
			else
				samePieces = 1;
		}
	}

	samePieces = 1;
	for (i = 0; i < numOfCols; i++) {
		for (j = 0; j < numOfRows - 1; j++) {
			current = j*numOfCols+i;
			next = (j+1)*numOfCols+i;
			if (adjacent(theBlankOX, current, next)) {
				samePieces++;
				if (samePieces == numInRow)
					return TRUE;
			}
			else
				samePieces = 1;
		}
	}

	samePieces = 1;
	for (k = 1; k < numOfRows - 1; k++) {
		j = k;
		for (i = 0; i < numOfCols - 1; i++) {
			if (j >= numOfRows - 1)
				break;
			current = j*numOfCols+i;
			next = (j+1)*numOfCols+i+1;
			if (adjacent(theBlankOX, current, next)) {
				samePieces++;
				if (samePieces == numInRow)
					return TRUE;
			}
			else
				samePieces = 1;
			j++;
		}
	}

	samePieces = 1;
	for (k = 0; k < numOfCols - 1; k++) {
		j = 0;
		for (i = k; i < numOfCols - 1; i++) {
			if (j >= numOfRows - 1)
				break;
			current = j*numOfCols+i;
			next = (j+1)*numOfCols+i+1;
			if (adjacent(theBlankOX, current, next)) {
				samePieces++;
				if (samePieces == numInRow)
					return TRUE;
			}
			else
				samePieces = 1;
			j++;
		}
	}

	samePieces = 1;
	for (k = 1; k < numOfRows - 1; k++) {
		j = k;
		for (i = 0; i < numOfCols - 1; i++) {
			if (j < 1)
				break;
			current = j*numOfCols+i;
			next = (j-1)*numOfCols+i+1;
			if (adjacent(theBlankOX, current, next)) {
				samePieces++;
				if (samePieces == numInRow)
					return TRUE;
			}
			else
				samePieces = 1;
			j--;
		}
	}

	samePieces = 1;
	for (k = 0; k < numOfCols - 1; k++) {
		j = numOfRows - 1;
		for (i = 0; i < numOfCols - 1; i++) {
			if (j < 1)
				break;
			current = j*numOfCols+i;
			next = (j-1)*numOfCols+i+1;
			if (adjacent(theBlankOX, current, next)) {
				samePieces++;
				if (samePieces == numInRow)
					return TRUE;
			}
			else
				samePieces = 1;
			j--;
		}
	}

	return FALSE;
}

BOOLEAN adjacent(BlankOX theBlankOX[], int current, int next) {
	int x_original_pos = numOfCols - 1;
	int o_original_pos = (numOfRows - rowsOfPieces) * numOfCols;
	return(theBlankOX[current] == theBlankOX[next] &&
	       ((theBlankOX[current] == O && current < o_original_pos) ||
	        (theBlankOX[current] == X && current > x_original_pos)) &&
	       ((theBlankOX[next] == O && next < o_original_pos) ||
	        (theBlankOX[next] == X && next > x_original_pos)));
}

void captureLine(int x, int y, BlankOX piece, BlankOX theBlankOX[]) {

	int xN, yN;

	// Check capture left
	if (theBlankOX[y*numOfCols + x - 1] != piece) {
		for (xN = 1; x-xN >= 0; xN++) {
			if (theBlankOX[y*numOfCols + (x-xN)] == Blank)
				break;
			if (theBlankOX[y*numOfCols + (x-xN)] == piece && xN > 1) {
				clearRow(x-xN + 1, x - 1, y, theBlankOX);
				break;
			}
		}
	}

	// Check capture right
	if (theBlankOX[y*numOfCols + x + 1] != piece) {
		for (xN = 1; x+xN < numOfCols; xN++) {
			if (theBlankOX[y*numOfCols + (x+xN)] == Blank)
				break;
			if (theBlankOX[y*numOfCols + (x+xN)] == piece && xN > 1) {
				clearRow(x + 1, x+xN - 1, y, theBlankOX);
				break;
			}
		}
	}

	// Check capture up
	if (theBlankOX[(y+1)*numOfCols + x] != piece) {
		for (yN = 1; y+yN < numOfRows; yN++) {
			if (theBlankOX[(y+yN)*numOfCols + x] == Blank)
				break;
			if (theBlankOX[(y+yN)*numOfCols + x] == piece && yN > 1) {
				clearColumn(y + 1, y+yN - 1, x, theBlankOX);
				break;
			}
		}
	}

	// Check capture down
	if (theBlankOX[(y-1)*numOfCols + x] != piece) {
		for (yN = 1; y-yN >= 0; yN++) {
			if (theBlankOX[(y-yN)*numOfCols + x] == Blank)
				break;
			if (theBlankOX[(y-yN)*numOfCols + x] == piece && yN > 1) {
				clearColumn(y-yN + 1, y - 1, x, theBlankOX);
				break;
			}
		}
	}
}

void captureCorner(int x, int y, BlankOX piece, BlankOX theBlankOX[]) {

	BlankOX oppositePiece = (piece == X ? O : X);
	int xN, yN;

	if (y == numOfRows - 1) {
		// Check capture left down
		for (xN = 1; x-xN >= 0; xN++) {
			if (theBlankOX[y*numOfCols + (x-xN)] != oppositePiece)
				break;
		}
		if (x-xN == -1) {
			for (yN = 1; y-yN >= 0; yN++) {
				if (theBlankOX[(y-yN)*numOfCols + 0] == Blank)
					break;
				if (theBlankOX[(y-yN)*numOfCols + 0] == piece) {
					clearRow(0, x - 1, numOfRows - 1, theBlankOX);
					clearColumn(y-yN + 1, numOfRows - 2, 0, theBlankOX);
					break;
				}
			}
		}

		// Check capture right down
		for (xN = 1; x+xN < numOfCols; xN++) {
			if (theBlankOX[y*numOfCols + (x+xN)] != oppositePiece)
				break;
		}
		if (x+xN == numOfCols) {
			for (yN = 1; y-yN >= 0; yN++) {
				if (theBlankOX[(y-yN)*numOfCols + numOfCols - 1] == Blank)
					break;
				if (theBlankOX[(y-yN)*numOfCols + numOfCols - 1] == piece) {
					clearRow(x + 1, numOfCols - 1, numOfRows - 1, theBlankOX);
					clearColumn(y-yN + 1,  numOfRows - 2, numOfCols - 1, theBlankOX);
					break;
				}
			}
		}
	}

	if (y == 0) {
		// Check capture left up
		for (xN = 1; x-xN >= 0; xN++) {
			if (theBlankOX[y*numOfCols + (x-xN)] != oppositePiece)
				break;
		}
		if (x-xN == -1) {
			for (yN = 1; y+yN < numOfRows; yN++) {
				if (theBlankOX[(y+yN)*numOfCols + 0] == Blank)
					break;
				if (theBlankOX[(y+yN)*numOfCols + 0] == piece) {
					clearRow(0, x - 1, 0, theBlankOX);
					clearColumn(1, y+yN - 1, 0, theBlankOX);
					break;
				}
			}
		}

		// Check capture right up
		for (xN = 1; x+xN < numOfCols; xN++) {
			if (theBlankOX[y*numOfCols + (x+xN)] != oppositePiece)
				break;
		}
		if (x+xN == numOfCols) {
			for (yN = 1; y+yN < numOfRows; yN++) {
				if (theBlankOX[(y+yN)*numOfCols + numOfCols - 1] == Blank)
					break;
				if (theBlankOX[(y+yN)*numOfCols + numOfCols - 1] == piece) {
					clearRow(x + 1, numOfCols - 1, 0, theBlankOX);
					clearColumn(1, y+yN - 1, numOfCols - 1, theBlankOX);
					break;
				}
			}
		}
	}

	if (x == 0) {
		// Check capture up right
		for (yN = 1; y+yN < numOfRows; yN++) {
			if (theBlankOX[(y+yN)*numOfCols + x] != oppositePiece)
				break;
		}
		if (y+yN == numOfRows) {
			for (xN = 1; x+xN < numOfCols; xN++) {
				if (theBlankOX[(numOfRows - 1)*numOfCols + (x+xN)] == Blank)
					break;
				if (theBlankOX[(numOfRows - 1)*numOfCols + (x+xN)] == piece) {
					clearColumn(y + 1, numOfRows - 1, 0, theBlankOX);
					clearRow(1, x+xN - 1, numOfRows - 1, theBlankOX);
					break;
				}
			}
		}

		// Check capture down right
		for (yN = 1; y-yN >= 0; yN++) {
			if (theBlankOX[(y-yN)*numOfCols + x] != oppositePiece)
				break;
		}
		if (y-yN == -1) {
			for (xN = 1; x+xN < numOfCols; xN++) {
				if (theBlankOX[x+xN] == Blank)
					break;
				if (theBlankOX[x+xN] == piece) { // [0*numOfCols + x+xN]
					clearColumn(0, y - 1, 0, theBlankOX);
					clearRow(1, x+xN - 1, 0, theBlankOX);
					break;
				}
			}
		}

		// Check capture up left
		for (yN = 1; y+yN < numOfRows; yN++) {
			if (theBlankOX[(y+yN)*numOfCols + x] != oppositePiece)
				break;
		}
		if (y+yN == numOfRows) {
			for (xN = 1; x-xN >= 0; xN++) {
				if (theBlankOX[(numOfRows - 1)*numOfCols + (x-xN)] == Blank)
					break;
				if (theBlankOX[(numOfRows - 1)*numOfCols + (x-xN)] == piece) {
					clearColumn(y + 1, numOfRows - 1, numOfCols - 1, theBlankOX);
					clearRow(x-xN + 1, numOfCols - 2, numOfRows - 1, theBlankOX);
					break;
				}
			}
		}

		// Check capture down left
		for (yN = 1; y-yN >= 0; yN++) {
			if (theBlankOX[(y-yN)*numOfCols + x] != oppositePiece)
				break;
		}
		if (y-yN == -1) {
			for (xN = 1; x-xN >= 0; xN++) {
				if (theBlankOX[x-xN] == Blank)
					break;
				if (theBlankOX[x-xN] == piece) { // [0*numOfCols + (x-xN)]
					clearColumn(0, y - 1, numOfCols - 1, theBlankOX);
					clearRow(x-xN + 1, numOfCols - 2, 0, theBlankOX);
					break;
				}
			}
		}
	}
}

//clear a given row

void clearRow(int x1, int x2, int y, BlankOX theBlankOX[]){
	while(x1 <= x2) {
		theBlankOX[y*numOfCols + x1] = Blank;
		x1++;
	}
}

//clear a given column
void clearColumn(int y1, int y2, int x, BlankOX theBlankOX[]){
	while(y1 <= y2) {
		theBlankOX[y1*numOfCols + x] = Blank;
		y1++;
	}
}

/************************************************************************
**
** NAME:        oneOrNoPieces
**
** DESCRIPTION: Return TRUE iff one player has one or no pieces.
**
** INPUTS:      BlankOX theBlankOX[boardSize] : The BlankOX array.
**
** OUTPUTS:     (BlankOX) x iff there is less than one x,
**                        o iff there is less than one o,
**                        Blank iff there are more than one x and more
**                        than one o.
**
************************************************************************/

BlankOX oneOrNoPieces(BlankOX theBlankOX[])
{
	int i, numOfX = 0, numOfO = 0;

	for(i = 0; i < boardSize; i++) {
		if (theBlankOX[i] == X) {
			numOfX++;
			if (numOfX > 1 && numOfO > 1)
				return Blank;
		}
		else if(theBlankOX[i] == O) {
			numOfO++;
			if (numOfX > 1 && numOfO > 1)
				return Blank;
		}
	}

	if(numOfX <= 1)
		return X;
	else
		return O;
}

/*
 * Given an x and y return corresponding array position
 * -1 is bad x and y
 */

int BoardPosToArrayPos(int x, int y){
	/**
	 * The first spot in the array is really the top
	 * left corner, the last spot is bottom right corner
	 */

	if (x < 0 || x >= numOfCols)
		return -1;
	if (y < 0 || y >= numOfRows)
		return -1;
	return (y*numOfCols)+x;
}

/******
* Unhash a shogi move
*
* input: MOVE
*
* output: sMove
******/
sMove unhashMove(MOVE move){
	struct cleanMove m;
	m.fromX = (move >> 24) & 0xff;
	m.fromY = (move >> 16) & 0xff;
	m.toX = (move >> 8) & 0xff;
	m.toY = move & 0xff;

	return m;
}

/********
* hash a shogi move
*
* input: int fromX
*        int fromY
*        int toX
*        int toY
* return: void
********/
MOVE hashMove(unsigned int fromX,
              unsigned int fromY,
              unsigned int toX,
              unsigned int toY){

	MOVE move = 0;
	move |= (fromX << 24);
	move |= (fromY << 16);
	move |= (toX << 8);
	move |= toY;
	return move;
}

POSITION StringToPosition(char* board) {
	// FIXME: this is just a stub
	return atoi(board);
}


char* PositionToString(POSITION pos) {
	// FIXME: this is just a stub
	return "Implement Me";
}
