/************************************************************************
**
** NAME:        msquaredance.c
**
** DESCRIPTION: Square Dance
**
** AUTHOR:      Yu-TE (Jack) Hsu
**				Joey Corless
**
** DATE:        WHEN YOU START/FINISH
**
** UPDATE HIST: RECORD CHANGES YOU HAVE MADE SO THAT TEAMMATES KNOW
**
** 2005-11-12
**   Joey - Fixed the problem in initializeGame().  The game now solves and plays
**          for 3x3.  Memory allocation error for 4x3, still possibly a problem
**          with my initializeGame() or maybe something else.
**
**   Joey - Major changes: Added a call to freeBoard before returns in
*/
/*************************************************************************
**
** NAME:        msquaredance.c
**
** DESCRIPTION: Square Dance
**
** AUTHOR:      Yu-TE (Jack) Hsu
**				Joey Corless
**
** DATE:        WHEN YOU START/FINISH
**
** UPDATE HIST: RECORD CHANGES YOU HAVE MADE SO THAT TEAMMATES KNOW
**
** 2005-12-18
**   Jack - added MoveToString
**
** 2005-11-12
**   Joey - Fixed the problem in initializeGame().  The game now solves and plays
**          for 3x3.  Memory allocation error for 4x3, still possibly a problem
**          with my initializeGame() or maybe something else.
**
**   Joey - Major changes: Added a call to freeBoard before returns i Primitive
**    and GenerateMoves.  Changed the array indexing in Primitive to utilize boardIndex
**    for ease of reading.  Fixed ValidTextInput to check for valid numbers.  Made
**    initializeGame fill in gInitialPosition and gNumberofPositions. Changed the
**    representation of a square from short to int, for uniformity (such a small difference
**    in memory seems insignificant when there will only be one board representation at a
**    time...  can change it back if necessary; it was just to test for possible problems.)
**    Changed the malloc in unhashBoard to allocate sizeof(int)*BOARD_WIDTH*BOARD_HEIGHT,
**    previously was sizeof(BOARD_WIDTH*BOARD_HEIGHT).
**
**    Some testing done: Inserted a printf into hashBoard. It is called, of course, when
**    it solves for 2x3 and 3x2, but not for 3x3.  So the problem can't be in the hash.  The above
**    changes seemed to make a difference: the game now plays without solving at 4x3 (and 4x4 and 5x5),
**    and goes into solving, but encounters a "bad position" which isn't really a bad position...
**    probably a problem with my initializegame.

**    Various minor changes: Changed FIRST_TURN and SECOND_TURN to YELLOW and BLUE for ease of
**    understanding.  Removed isEmpty prototype since the procedure was previously removed.
**
**

**
** 2005/11/11
**   Jack - add more #ifdef and #ifndef for selftest main(), we can remove the line:
**              #define SQUAREDANCEDEBUG
**          to compile this file alone without importing
**   Joey - Fixed various bugs, most notably fixing Primitive to return properly
**          for standard game.  Also changed size to 3x2, since 3x3 and above fails.
**   Joey - Incomplete version of squaredance. Compiled and runs, but crashes
**          upon starting game with or without solve.
**************************************************************************/

/* Note:
 * comment out this line for Gamesman compiling
 * keep this when run the main() test on its own
 */
//#define SQUAREDANCEDEBUG

#ifdef SQUAREDANCEDEBUG
	#define NULL 0
	#define FALSE 0
	#define TRUE 1
typedef char* STRING;
typedef int BOOLEAN;
typedef long int POSITION;
typedef int MOVE;
BOOLEAN gStandardGame = TRUE;
typedef enum {
	win, lose, draw, tie, undecided
} VALUE;
void PrintMove(MOVE move);
#endif


/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/
#ifndef SQUAREDANCEDEBUG

#include <stdio.h>
#include "gamesman.h"
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

#endif

STRING kGameName            = "Square Dance";   /* The name of your game */
STRING kAuthorName          = "Jack Hsu, Joey Corless";   /* Your name(s) */
STRING kDBName              = "squaredance";   /* The name to store the database under */

void* gGameSpecificTclInit = NULL;

BOOLEAN kPartizan            = TRUE;   /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN kGameSpecificMenu    = TRUE;   /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN kTieIsPossible       = TRUE;   /* TRUE if a tie is possible. FALSE if it is impossible.*/
BOOLEAN kLoopy               = FALSE;   /* TRUE if the game tree will have cycles (a rearranger style game). FALSE if it does not.*/

BOOLEAN kDebugMenu           = TRUE;   /* TRUE only when debugging. FALSE when on release. */
BOOLEAN kDebugDetermineValue = TRUE;   /* TRUE only when debugging. FALSE when on release. */

POSITION gNumberOfPositions   =  1; /* The number of total possible positions | If you are using our hash, this is given by the hash_init() function*/
/* Don't know */
POSITION gInitialPosition     =  0; /* The initial hashed position for your starting board: 0 in this game */
POSITION kBadPosition         = -1; /* A position that will never be used */

/*
 * Help strings that are pretty self-explanatory
 * Strings than span more than one line should have backslashes (\) at the end of the line.
 */

STRING kHelpGraphicInterface =
        "No Graphic Interface with Square Dance Right Now";

STRING kHelpTextInterface=
        "You denote the column of your move with a letter (starting with a on the \n\
left and increasing), and the row with a number (starting with 1 on the bottom \n\
and increasing).  You then specify which way you want to place it (u for up, d \n\
for down).  All three characters should be entered on one line with no spaces \n\
in between.\n"                                                                                                                                                                                                                                                                                                                                              ;

STRING kHelpOnYourTurn =
        "You can place your piece on the blank spot.\n\
For example: a1u means put a piece at (a,1) with position up"                                                        ;

STRING kHelpStandardObjective =
        "Your goal is to form a square with four pieces sharing one of the attributes\n\
Attributes include:\n\
  O or X (player's color), U or D (up or down)\n\
Each player can only play one color (O or X) once the game has started\n\
He or she may choose to place the piece up or down\n\
In the following examples, assume Player X goes first and Player O placed just placed a piece:\n\
Example 1  \n\
   ----------  \n\
 3 |OU|  |XU|  \n\
   |--+--+--|  \n\
 2 |  |  |  |  \n\
   |--+--+--|  \n\
 1 |OU|  |XU|  \n\
   ----------  \n\
     a  b  c  \n\
 O placed the piece U at (0,0).  \n\
 O wins!\n\
 Example 2  \n\
   ----------  \n\
 3 |OD|XD|  |  \n\
   |--+--+--|  \n\
 2 |OD|XD|  |  \n\
   |--+--+--|  \n\
 1 |  |  |  |  \n\
   ----------  \n\
     a  b  c  \n\
 O placed the piece U at (0,0).  \n\
 O wins!\n\
Example 3  \n\
   ----------  \n\
 3 |  |OU|  |  \n\
   |--+--+--|  \n\
 2 |OU|  |XU|  \n\
   |--+--+--|  \n\
 1 |  |XU|  |  \n\
   ----------  \n\
     a  b  c  \n\
 O placed the piece U at (0,0).  \n\
O wins!\n"                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              ;


STRING kHelpReverseObjective ="Force your opponent to make a square before you do.";

STRING kHelpTieOccursWhen ="the board is full and no squares are formed.";

STRING kHelpExample =
        "              \n\
   ----------  \n\
 3 |  |OU|  |  \n\
   |--+--+--|  \n\
 2 |  |  |  |  \n\
   |--+--+--|  \n\
 1 |  |  |  |  \n\
   ----------  \n\
     a  b  c  \n\
 O placed the piece U at (b3).  \n\
               \n\
   ----------  \n\
 3 |  |OU|  |  \n\
   |--+--+--|  \n\
 2 |  |XD|  |  \n\
   |--+--+--|  \n\
 1 |  |  |  |  \n\
   ----------  \n\
     a  b  c   \n\
 X placed the piece D at b2.  \n\
               \n\
   ----------  \n\
 3 |  |OU|  |  \n\
   |--+--+--|  \n\
 2 |  |XD|OD|  \n\
   |--+--+--|  \n\
 1 |  |  |  |  \n\
   ----------  \n\
     a  b  c   \n\
 O placed the piece D at c2.  \n\
               \n\
   ----------  \n\
 3 |  |OU|  |  \n\
   |--+--+--|  \n\
 2 |  |XD|OD|  \n\
   |--+--+--|  \n\
 1 |XD|  |  |  \n\
   ----------  \n\
     a  b  c  \n\
 X placed the piece D at a1.  \n\
               \n\
   ----------  \n\
 3 |  |OU|  |  \n\
   |--+--+--|  \n\
 2 |  |XD|OD|  \n\
   |--+--+--|  \n\
 1 |XD|OD|  |  \n\
   ----------  \n\
     a  b  c  \n\
 O placed the piece D at b1.  \n\
               \n\
   ----------  \n\
 3 |  |OU|  |  \n\
   |--+--+--|  \n\
 2 |  |XD|OD|  \n\
   |--+--+--|  \n\
 1 |XD|OD|XD|  \n\
   ----------  \n\
     a  b  c  \n\
 X placed the piece D at c1.  \n\
X wins!\n"                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    ;


#ifndef SQUAREDANCEDEBUG

/* External */
#ifndef MEMWATCH
extern GENERIC_PTR      SafeMalloc ();
extern void             SafeFree ();
#endif

#endif

extern BOOLEAN (*gGoAgain)(POSITION, MOVE);


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

#define YELLOW 0
#define BLUE 1
#define DOWN 0
#define UP 1
#define EMPTY 0
#define POSSIBLE_SQUARE_VALUES 5  // 5 possible values for a square, should not be changed

//#define EMPTY 0
#define YELLOW_DOWN 1
#define YELLOW_UP 2
#define BLUE_DOWN 3
#define BLUE_UP 4


/* Note: these are to be used as offsets for parsing input, so should only be changed if
 * changing the input method (e.g. changing from 0-4x0-4 to a-dx0-4).
 */
#define ROW_START '1'
#define COL_START 'a'


/* Note: DOES NOT support size of 4 x 4 or greater
 */

#define DEFAULT_BOARD_WIDTH  3
#define DEFAULT_BOARD_HEIGHT 3
#define MAX_BOARD_WIDTH 4
#define MAX_BOARD_HEIGHT 4

/*************************************************************************
**
** #defines and structs
**
**************************************************************************/

#define FIRST_TURN YELLOW
#define SECOND_TURN BLUE

/*************************************************************************
**
** Global Variables
**
*************************************************************************/

typedef struct {
	//char *squares;
	int *squares;
	int squaresOccupied;
	int currentTurn;
} SDBoard, *SDBPtr;

/* variants */
int BOARD_WIDTH = DEFAULT_BOARD_WIDTH, BOARD_HEIGHT = DEFAULT_BOARD_HEIGHT;
int BOARD_ROWS          = DEFAULT_BOARD_HEIGHT;
int BOARD_COLS          = DEFAULT_BOARD_WIDTH;
BOOLEAN gCanWinByColor = TRUE;
BOOLEAN gCanWinByUD = TRUE;

//For GPS:
SDBPtr gBoard;

char charColorYellow = 'X';
char charColorBlue = 'O';
char charPositionUp = 'u';
char charPositionDown = 'd';


/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/

STRING MToS(MOVE theMove);

//BOOLEAN isSquareWin(char slot1, char slot2, char slot3, char slot4);
BOOLEAN isSquareWin(int slot1, int slot2, int slot3, int slot4);
int boardIndex(int x, int y);

/* Square */
int hashSquare(int ud, int color);
int getSquare(SDBPtr board, int x, int y);
BOOLEAN isSquareEmpty(SDBPtr board, int x, int y);
int getSquareColor(SDBPtr board, int x, int y);
int getSquareUD(SDBPtr board, int x, int y);
void setSquare(SDBPtr board, int x, int y, int value);
void setSquareEmpty(SDBPtr board, int x, int y);

/* Move */
MOVE hashMove(int x, int y, int ud);
int unhashMoveToX(MOVE move);
int unhashMoveToY(MOVE move);
int unhashMoveToUD(MOVE move);
/*** Had to remove color from the move hash in order to use it in ConvertTextInputToMove.
   int unhashMoveToColor(MOVE move);
   Note: Delete this once we're certain we won't need color in the move hash.
 */

/* Board */
POSITION hashBoard(SDBPtr board);
SDBPtr unhashBoard(POSITION position);
// unhashBoard will create a board, need to call freeBoard in the end
SDBPtr newBoard();
void freeBoard(SDBPtr board); // need to call this as soon as we dont need the board


/* Assorted Helpers */
int prompt_board_width();
int prompt_board_height();
int getCurrentTurn(POSITION position);


/*needed for generic hash*/
int vcfg(int *this_cfg);

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
	int row, col;
	SDBPtr board = newBoard();
	int totalPositions = 1;

	for(row=0; row<BOARD_ROWS; row++)
	{
		for(col=0; col<BOARD_COLS; col++)
		{
			setSquareEmpty(board,col,row);
			totalPositions*=5;
		}
	}

	gNumberOfPositions = totalPositions << 1; // last bit for storing whose turn it is

	gNumberOfPositions <<=1;
	board->squaresOccupied=0;
	board->currentTurn=FIRST_TURN;


	gInitialPosition = hashBoard(board);

	if(gUseGPS)
		gBoard = board;
	else
		freeBoard(board);

	gMoveToStringFunPtr = &MToS;

}

int vcfg(int *this_cfg)
{
	return ((this_cfg[0] + this_cfg[1])  == (this_cfg[2] + this_cfg[3])) || ((this_cfg[0] + this_cfg[1])  == (this_cfg[2] + this_cfg[3] + 1));

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
/*------------------- implementation ------------------------*/
/* MOVE format: move_x, move_y, move_ud, move_color */
#ifndef SQUAREDANCEDEBUG
MOVELIST *GenerateMoves (POSITION position)
{
	int x, y, valid_color;
	MOVELIST *moves = NULL;
	SDBPtr board = unhashBoard(position);
	valid_color = getCurrentTurn(position);

	for(x=0; x<BOARD_WIDTH; x++) {
		for(y=0; y<BOARD_HEIGHT; y++) {
			if(isSquareEmpty(board,x,y)) { // for each empty slots
				moves = CreateMovelistNode(hashMove(x,y,UP),moves);
				moves = CreateMovelistNode(hashMove(x,y,DOWN),moves);
			}
		}
	}
	freeBoard(board);
	return moves;
}
#endif

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

POSITION DoMove(POSITION position, MOVE move ) {
	/* unhash move */
	int move_x = unhashMoveToX(move);
	int move_y = unhashMoveToY(move);
	int move_ud = unhashMoveToUD(move);
	int move_color = getCurrentTurn(position);
	/* unhash board */
	if(!gUseGPS)
	{
		SDBPtr board = unhashBoard(position);

		board->squares[boardIndex(move_x, move_y)] = (move_color << 1) + move_ud + 1;
		board->squaresOccupied++;
		board->currentTurn = !board->currentTurn;

		POSITION hash = hashBoard(board);
		freeBoard(board);
		return hash;
	}
	else
	{
		gBoard->squares[boardIndex(move_x, move_y)] = (move_color << 1) + move_ud + 1;
		gBoard->squaresOccupied++;
		gBoard->currentTurn = !gBoard->currentTurn;
		return 0;
	}
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
***********************************************************************/

/* determine the result of the game */
VALUE Primitive (POSITION position) { //(POSITION position) {
	SDBPtr board;
	if(gUseGPS)
		board = gBoard;
	else
		board = unhashBoard(position);
	int x1, y1, x2, y2, w, slot1 = EMPTY, slot2 = EMPTY, slot3 = EMPTY, slot4 = EMPTY;
	for(x1=0; x1<BOARD_WIDTH-1; x1++) {
		for(y1=0; y1<BOARD_HEIGHT-1; y1++) {
			slot1 = board->squares[y1*BOARD_WIDTH+x1];
			for(w=1; x1+w<BOARD_WIDTH && y1+w<BOARD_HEIGHT; w++) {
				x2 = x1+w;
				y2 = y1+w;
				slot2 = board->squares[boardIndex(x2,y1)];
				slot3 = board->squares[boardIndex(x1,y2)];
				slot4 = board->squares[boardIndex(x2,y2)];
				if(isSquareWin(slot1,slot2,slot3,slot4))
				{
					freeBoard(board);
					return gStandardGame ? lose : win;
				}
			}
		}
	}

	//check for squares rotated 90 degree
	int xc, yc;
	for(xc=1; xc<BOARD_WIDTH-1; xc++) {
		for(yc=1; yc<BOARD_HEIGHT-1; yc++) {
			for(w=1; (x1=xc-w)>=0 && (x2=xc+w)<=BOARD_WIDTH-1 && (y1=yc-w)>=0 && (y2=yc+w)<=BOARD_HEIGHT-1; w++) {
				slot1 = board->squares[boardIndex(xc,y1)];
				slot2 = board->squares[boardIndex(xc,y2)];
				slot3 = board->squares[boardIndex(x1,yc)];
				slot4 = board->squares[boardIndex(x2,yc)];
			}
			if(isSquareWin(slot1,slot2,slot3,slot4)) {
				if(!gUseGPS)
					freeBoard(board);
				return gStandardGame ? lose : win;
			}
		}
	}

	/* hard code for 4x4 board
	 * for this version, we only support 3 x 3
	 * in the future, we will support up to 4 x 4
	 */
	if(BOARD_WIDTH==4 && BOARD_HEIGHT==4) {
		slot1 = board->squares[boardIndex(1,0)];
		slot2 = board->squares[boardIndex(3,1)];
		slot3 = board->squares[boardIndex(2,3)];
		slot4 = board->squares[boardIndex(0,2)];
		if(isSquareWin(slot1,slot2,slot3,slot4)) return gStandardGame ? lose : win;
		slot1 = board->squares[boardIndex(2,0)];
		slot2 = board->squares[boardIndex(0,1)];
		slot3 = board->squares[boardIndex(3,2)];
		slot4 = board->squares[boardIndex(1,3)];
		if(isSquareWin(slot1,slot2,slot3,slot4)) return gStandardGame ? lose : win;
	}

	if(board->squaresOccupied==BOARD_WIDTH*BOARD_HEIGHT)
	{
		if(!gUseGPS)
			freeBoard(board);
		return tie;
	}
	else
	{
		if(!gUseGPS)
			freeBoard(board);
		return undecided;
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
void PrintPosition (POSITION position, STRING playerName, BOOLEAN usersTurn )
{
	int x, y, hyphens;
	SDBPtr board = unhashBoard(position);

	int whoseturn = board->currentTurn;
	char turnString1[80], turnString2[80], prediction[80];

	char ownColor = (whoseturn==YELLOW) ? charColorYellow : charColorBlue;
	int spaceNeededEndOfRow = 15;

// top
	printf("\n\t+--------------");
	for(x=0; x<BOARD_WIDTH; x++)
		printf("---");
	printf("---------+\n");
	printf("\t| Square Dance ");
	for(x=0; x<BOARD_WIDTH; x++)
		printf("   ");
	printf("         |\n");

	printf("\t+--------------");
	for(x=0; x<BOARD_WIDTH; x++)
		printf("---");
	printf("---------+\n");

	// top of the game board
	printf("\t|       ");
	for(x=0; x<BOARD_WIDTH; x++)
		printf("---");
	printf("-");
	for(hyphens = 0; (int) hyphens < spaceNeededEndOfRow; hyphens++)
		printf(" ");
	printf("|\n");


	for(y=BOARD_HEIGHT-1; y>=0; y--) {

		// odd rows (the ones that matter)
		printf("\t|     %c ", ROW_START + y);
		for(x=0; x<BOARD_WIDTH; x++) {
			printf("|");
			if(isSquareEmpty(board,x,y))
				printf("  ");
			else {
				char colorToPrint = (getSquareColor(board,x,y) == YELLOW) ? charColorYellow : charColorBlue;
				char udToPrint = (getSquareUD(board,x,y) == UP) ? charPositionUp : charPositionDown;
				printf("%c%c",colorToPrint,udToPrint);
			}
		}
		printf("|");


		for(hyphens = 0; (int) hyphens < spaceNeededEndOfRow; hyphens++)
			printf(" ");
		printf("|\n");

		// even rows (hyphen-lines)
		if(y!=0) {
			printf("\t|       |");
			for(x=0; x<BOARD_WIDTH-1; x++) {
				printf("--+");
			}
			printf("--|");
		} else {
			printf("\t|       ");
			for(x=0; x<BOARD_WIDTH; x++)
				printf("---");
			printf("-");
		}

		// end of the row
		for(hyphens = 0; (int) hyphens < spaceNeededEndOfRow; hyphens++)
			printf(" ");
		printf("|\n");
	}

	printf("\t|       ");
	for(x = 0; x < BOARD_WIDTH; x++)
		printf("  %c", COL_START + x);

	for(hyphens = 0; (int) hyphens < spaceNeededEndOfRow; hyphens++)
		printf(" ");
	printf(" |\n");

	//Player Name. Stolen directly from Othello
	sprintf(turnString1,"| It is %s's turn.", playerName);
	printf("\t%s",turnString1);
	if((int) strlen(turnString1) < (3 * BOARD_WIDTH) + 24)
		for(hyphens = 0; (int) hyphens < (int) ((3 * BOARD_WIDTH) + 24 - strlen(turnString1)); hyphens++)
			printf(" ");
	printf("|\n");

	sprintf(turnString2,"| %s is playing %c", playerName, ownColor);
	printf("\t%s",turnString2);
	if((int) strlen(turnString2) < (int) ((3 * BOARD_WIDTH) + 24))
		for(hyphens = 0; (int) hyphens < (int) ((3 * BOARD_WIDTH) + 24 - strlen(turnString2)); hyphens++)
			printf(" ");
	printf("|\n");

	#ifndef SQUAREDANCEDEBUG
	sprintf(prediction,"| %s",GetPrediction(position,playerName,usersTurn));
	if (prediction[2] == '(')
	{
		printf("\t%s", prediction);
		if((int) strlen(prediction) < (3 * BOARD_WIDTH) + 24)
			for(hyphens = 0; (int) hyphens < (int) ((3 * BOARD_WIDTH) + 24 - strlen(prediction)); hyphens++)
				printf(" ");
		printf("|\n");
	}
	#endif

// bottom
	printf("\t+--------------");
	for(x=0; x<BOARD_WIDTH; x++)
		printf("---");
	printf("---------+\n\n");

	freeBoard(board);

}

/************* Other helper functions ******************/



/************************************************************************
** Utility functions Implementation. Private to this file.
*************************************************************************/

/* Square */
int boardIndex(int x, int y) {
	return y*BOARD_WIDTH+x;
}
int getSquare(SDBPtr board, int x, int y) {
	return board->squares[boardIndex(x,y)];
}
BOOLEAN isSquareEmpty(SDBPtr board, int x, int y) {
	return getSquare(board,x,y)==0;
}
int getSquareColor(SDBPtr board, int x, int y) {
	return getSquare(board,x,y)>=3;
}
int getSquareUD(SDBPtr board, int x, int y) {
	return (getSquare(board,x,y)&1)==0;
}                                                                                   //Joey made a change
void setSquare(SDBPtr board, int x, int y, int value) {
	board->squares[boardIndex(x,y)] = value;
}
void setSquareEmpty(SDBPtr board, int x, int y) {
	setSquare(board,x,y,EMPTY);
}


/* Move */
MOVE hashMove(int x, int y, int ud) {
	return ((x << 24) | (y << 16) | (ud << 8));
}
int unhashMoveToX(MOVE move)  {
	return move >> 24;
}
int unhashMoveToY(MOVE move)  {
	return (move >> 16) & 0xff;
}
int unhashMoveToUD(MOVE move) {
	return (move >> 8) &  0xff;
}

/* Board */

/*************************************************
** hashBoard
** note: use the general hash function for now
**       but limited to 3x4, 4x3
***************************************************/
POSITION hashBoard(SDBPtr board) {
	POSITION hash = 0;
	int base = POSSIBLE_SQUARE_VALUES;
	int x, y;
	for(y=0; y<BOARD_HEIGHT; y++)
		for(x=0; x<BOARD_WIDTH; x++)
			hash = hash * base + getSquare(board,x,y);

	hash = (hash << 1) + board->currentTurn; // last bit stores the current turn
	return hash;

	//return generic_hash(board->squares, board->currentTurn);

}

/*************************************************
** unhashBoard
** note: use the general hash function for now
**       but limited to 3x4, 4x3
***************************************************/
SDBPtr unhashBoard(POSITION position) {

	SDBPtr board = newBoard();

	int x, y, squareValue;
	int base = POSSIBLE_SQUARE_VALUES;

	board->squaresOccupied = 0;
	board->currentTurn = position & 1; // last bit

	position = position >> 1;

	for(y=BOARD_HEIGHT-1; y>=0; y--) {
		for(x=BOARD_WIDTH-1; x>=0; x--) {
			squareValue = position % base;
			setSquare(board,x,y,squareValue);
			if(squareValue!=EMPTY)
				board->squaresOccupied++;
			position /= base;
		}
	}

	return board;

	/*  SDBPtr board = newBoard();
	   generic_unhash(position, board->squares);
	   int col, row;
	   for(row=0;row<BOARD_ROWS;row++)
	   {
	   for(col=0;col<BOARD_COLS;col++)
	   {
	   if(board->squares[boardIndex(col,row)]!=EMPTY)
	    {
	      board->squaresOccupied++;
	    }
	   }
	   }
	   board->currentTurn=whoseMove(position);
	   return board;*/

}

/* return the color of the current turn */
int getCurrentTurn(POSITION position) {
	//return whoseMove(position);
	return position & 1;
}

/* free the memory space for the board
   called when board is not needed anymore */
void freeBoard(SDBPtr board) {
	SafeFree(board->squares);
	SafeFree(board);
}

/* allocate space for the board */
SDBPtr newBoard() {
	SDBPtr board = (SDBPtr) SafeMalloc(sizeof(SDBoard));
	board->squares = (int *) SafeMalloc(sizeof(int)*BOARD_WIDTH*BOARD_HEIGHT);
	return board;
}

/********** helper function for Primitive **************/
//BOOLEAN isSquareWin(char slot1, char slot2, char slot3, char slot4) {
BOOLEAN isSquareWin(int slot1, int slot2, int slot3, int slot4) {
	int slots[4]={slot1, slot2, slot3, slot4}, colors[4], uds[4], colorMatch=1, udMatch=1;
	int i;

	if(slot1==0  || slot2==0 || slot3==0 || slot4==0) {
		return FALSE;
	}

	for(i=0; i<4; i++) {
		colors[i] = (slots[i]-1) >> 1;
		uds[i]=(slots[i]-1) & 1;
	}

	for(i=1; i<4; i++) {
		if(colors[i]==colors[0]) colorMatch++;
		if(uds[i]==uds[0]) udMatch++;
	}
	return ((gCanWinByColor && colorMatch==4) || (gCanWinByUD && udMatch==4));

}



/******************* MAIN ***********************/
#ifdef SQUAREDANCEDEBUG
int main() { //int argc, char [] argv) {
	int i;
	POSITION position=gInitialPosition;
	BOOLEAN turn = TRUE; // user first

	STRING playerName = "Dan";
	STRING computerName = "Game Crafters";

	//position = 3;
	//PrintPosition(position, playerName, turn);
	//position = 3752;
	//PrintPosition(position, playerName, turn);
	//PrintMove(50397184);
	for(i=0; i<4; i++) {
		switch(i) {
		case 0: position = DoMove(position, hashMove(0,0, UP));  break;
		case 1: position = DoMove(position, hashMove(0,BOARD_HEIGHT-1, UP)); break;
		case 2: position = DoMove(position, hashMove(BOARD_WIDTH-1,0, UP)); break;
		case 3: position = DoMove(position, hashMove(BOARD_WIDTH-1,BOARD_HEIGHT-1, DOWN)); break;
			// default:
		}
		turn = !turn;
		PrintPosition(position, (turn ? playerName : computerName), turn);
	}
//  printf("%d",position);
	//printf("Primitive = %d\n",Primitive(board));
	getchar();
	return 0;

}
#endif


/************************************************************************
**
** NAME:        PrintMove
**
** DESCRIPTION: Prints the move in a nice format.
**
** INPUTS:      MOVE move         : The move to print.
**
************************************************************************/


void PrintMove(MOVE move)
{
	printf("%s",gMoveToStringFunPtr(move));
}


/************************************************************************
**
** NAME:        MToS()
**
** DESCRIPTION: return the string of the move
**
** INPUTS:      MOVE move         : The move
**
** OUTPUTS:     String : the string form of the move
**
************************************************************************/
STRING MToS(MOVE theMove) {
	STRING move = (STRING) SafeMalloc(4);
	int col = unhashMoveToX(theMove);
	int row = unhashMoveToY(theMove);
	char UD = (unhashMoveToUD(theMove)==UP) ? charPositionUp : charPositionDown;
	sprintf(move,"%c%c%c", col + COL_START, row + ROW_START, UD);
	return move;
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
	return (((BOARD_ROWS-1) << 5) + ((BOARD_COLS-1) << 3) + (gCanWinByColor << 2) + (gCanWinByUD << 1) + (gStandardGame));
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
	return MAX_BOARD_WIDTH*MAX_BOARD_HEIGHT*8;
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
	char selection = 'Z';
	do
	{
		printf("\n\t----- Game Specific Options for Squaredance ----- \n\n");
		printf("\tc)\tToggle win by (C)olor.  [Currently: ");
		if(gCanWinByColor)
			printf("ON]\n");
		else
			printf("OFF]\n");
		printf("\to)\tToggle win by (O)rientation.  [Currently: ");
		if(gCanWinByUD)
			printf("ON]\n");
		else
			printf("OFF]\n");
		printf("\ts)\tChange Board (S)ize [Currently: %dx%d]\n", BOARD_COLS, BOARD_ROWS);
		printf("\tb)\t(B)ack to previous screen\n\n");
		printf("\tPlease select an option: ");
		selection = toupper(GetMyChar());
		switch (selection)
		{
		case 'C':
			gCanWinByColor = !gCanWinByColor;
			selection = 'Z';
			break;
		case 'O':
			gCanWinByUD = !gCanWinByUD;
			selection = 'Z';
			break;
		case 'S':
			BOARD_WIDTH = BOARD_COLS = prompt_board_width();
			BOARD_HEIGHT = BOARD_ROWS = prompt_board_height();
			selection = 'Z';
			break;
		case 'B':
			return;
		default:
			printf("Invalid Option.\n");
			selection = 'Z';
			break;

		}
	} while (selection != 'B');


}

int prompt_board_width()
{
	char selection = 'Z';
	while(1) {
		printf("Enter board width (From 1 to %d): ", MAX_BOARD_WIDTH);
		selection = toupper(GetMyChar());
		if(selection < '1' || selection > ('0' + MAX_BOARD_WIDTH))
		{
			printf("Invalid width.\n");
		}
		else
		{
			return (int) (selection - '0');
		}
	}

}


int prompt_board_height()
{
	char selection = 'Z';
	while(1) {
		printf("Enter board height (From 1 to %d): ", MAX_BOARD_HEIGHT);
		selection = toupper(GetMyChar());
		if(selection < '1' || selection > '0' + MAX_BOARD_HEIGHT)
		{
			printf("Invalid width.\n");
		}
		else
		{
			return (int) (selection - '0');
		}
	}
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
	BOARD_ROWS = (option >> 5)+1;
	BOARD_COLS = ((option >> 3) & 3)+1;
	gCanWinByColor = (option >> 2) & 1;
	gCanWinByUD = (option >> 1) & 1;
	gCanWinByUD = option & 1;
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
	if (strlen(input) != 3)
		return FALSE;
	if ((input[0] < COL_START) || (input[0] > (COL_START+BOARD_ROWS)) ||
	    (input[1] < ROW_START) || (input[1] > ROW_START+BOARD_COLS))
		return FALSE;
	if ((input[2] != charPositionUp) && (input[2] != charPositionDown))
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
	int row, col, updown;

	col = input[0] - COL_START;
	row = input[1] - ROW_START;
	if(input[2]==charPositionUp)
	{
		updown = UP;
	}
	else
	{
		updown = DOWN;
	}
	return hashMove (col, row, updown);
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
#ifndef SQUAREDANCEDEBUG
USERINPUT GetAndPrintPlayersMove (POSITION position, MOVE *move, STRING playersName)
{
	USERINPUT input;
	USERINPUT HandleDefaultTextInput();
	char player_char = (getCurrentTurn(position) == YELLOW) ? charColorYellow : charColorBlue;

	for (;; ) {
		/***********************************************************
		* CHANGE THE LINE BELOW TO MATCH YOUR MOVE FORMAT
		***********************************************************/
		printf("%8s's (%c) move [(u)ndo/([%c-%c][%c-%c][%c/%c])] : ", playersName, player_char,
		       COL_START, COL_START+BOARD_WIDTH-1, ROW_START, ROW_START+BOARD_HEIGHT-1, charPositionUp, charPositionDown );

		input = HandleDefaultTextInput(position, move, playersName);

		if (input != Continue)
			return input;
	}

	/* NOTREACHED */
	return Continue;
}
#endif

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
	printf("%8s's move [([%c-%c][%c-%c][%c/%c])] : ", computersName,
	       COL_START, COL_START+BOARD_WIDTH-1, ROW_START, ROW_START+BOARD_WIDTH-1, charPositionUp, charPositionDown );
	PrintMove(computersMove);
	printf("\n");
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
POSITION StringToPosition(char* board) {
	// FIXME: this is just a stub
	return atoi(board);
}


char* PositionToString(POSITION pos) {
	// FIXME: this is just a stub
	return "Implement Me";
}

STRING MoveToString(MOVE theMove) {
	return "Implement MoveToString";
}

char * PositionToEndData(POSITION pos) {
	return NULL;
}
