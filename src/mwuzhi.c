// $id$
// $log$

/*
 * The above lines will include the name and log of the last person
 * to commit this file to CVS
 */

/************************************************************************
**
** NAME:        mwuzhi.c
**
** DESCRIPTION: Wu Zhi (5 piece game)
**
** AUTHOR:      Diana Fang, Dan Yan
**
** DATE:        2004-09-13
**
** UPDATE HIST:
**              2004-10-5  Added GenerateMoves with all its helpers
**              2004-10-12 Added Help Strings, Defines, Global Variables
**                         Started Initialize Game
**              2004-10-18 Added Print Position, Print Move, Print Computer's Move, Primitive,
**                         Added another helper to Moves: GetArrayNum given xycoords.
**              2004-10-19 Added DoMove to this template
**              2004-11-2  Set kPartizan to TRUE, modified help strings so they are not all on one line. Moves are now formatted as [1 d]
**                         rather than [1 down] Added a sample game to the help string.
**              2004-11-26 Added diagonal variation in generatemoves
**              2004-12-5  Finished implementation of diagonals and game specific menu
**              2004-12-19 Added in primitive that you lose if no moves left. Also tells the player what color they
**                         when printing the board.
**              2005-3-3   Added missing prototypes
**              2005-3-25  Added SetOptions and GetOptions to work with Diagonals
**		2006-8-21  changed to GetMyInt() dmchan
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
#include <ctype.h>


/*************************************************************************
**
** Game-specific constants
**
**************************************************************************/

STRING kGameName            = "Wu Zhi";   /* The name of your game */
STRING kAuthorName          = "Dan Yan and Diana Fang";   /* Your name(s) */
STRING kDBName              = "wuzhi";   /* The name to store the database under */

BOOLEAN kPartizan            = TRUE;   /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN kGameSpecificMenu    = TRUE;   /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN kTieIsPossible       = FALSE;   /* TRUE if a tie is possible. FALSE if it is impossible.*/
BOOLEAN kLoopy               = TRUE;   /* TRUE if the game tree will have cycles (a rearranger style game). FALSE if it does not.*/

BOOLEAN kDebugMenu           = FALSE;   /* TRUE only when debugging. FALSE when on release. */
BOOLEAN kDebugDetermineValue = FALSE;   /* TRUE only when debugging. FALSE when on release. */

POSITION gNumberOfPositions   =  0; /* The number of total possible positions | If you are using our hash, this is given by the hash_init() function*/
POSITION gInitialPosition     =  0; /* The initial hashed position for your starting board */
POSITION kBadPosition         = -1; /* A position that will never be used */
void*    gGameSpecificTclInit = NULL;

/*
 * Help strings that are pretty self-explanatory
 * Strings than span more than one line should have backslashes (\) at the end of the line.
 */

STRING kHelpGraphicInterface =
        "Not written yet";

STRING kHelpTextInterface    =
        "On your turn, pick a piece that you want to move and then move it.  You \n\
can only move up, down, left, or right.  Diagonals are only allowed if \n\
you set up that option."                                                                                                                                                                ;

STRING kHelpOnYourTurn =
        "On your turn, pick a piece that you want to move. Look at the key to the \n\
right of the board to determine the number of the position of the piece you \n\
want to move.  Then choose the direction you want to move your piece (up, \n\
down, left, right...you can also refer to the compass, shown above the board).\n\
A sample move would be '1 u'(which means 1 up). "                                                                                                                                                                                                                                                                                                                                      ;

STRING kHelpStandardObjective =
        "Capture all but one of your opponent's pieces to win. You can also win \n\
if your opponent does not have any more moves possible. Capturing takes \n\
place when you move your piece into a specific configuration.  The \n\
configuration is as follows: If two of your pieces are next to each other \n\
and if on that same line that your pieces are one, one of those pieces is \n\
adjacent to an opponent's piece, then that results in a capture.  However, \n\
no other pieces must be along that same line where your capture is taking \n\
place in order for it to be a legal capture.  Also, a capture only occurs \n\
if you move into a capture position.  A capture does not occur if your \n\
opponent moves into your capture position.\n\
Here are some examples of capture positions: \n\
\n\
Here white captures a black piece:\n\
( )--( )--( )--( )--( )\n\
 |    |    |    |    |   \n\
( )--(b)--(w)--(w)--( ) \n\
 |    |    |    |    |  \n\
( )--( )--( )--( )--( ) \n\
 |    |    |    |    | \n\
( )--( )--( )--( )--( ) \n\
 |    |    |    |    | \n\
( )--( )--( )--( )--( ) \n\
\n\
Here black captures a white piece: \n\
( )--( )--( )--( )--( )\n\
 |    |    |    |    |  \n\
( )--(b)--( )--( )--( ) \n\
 |    |    |    |    |   \n\
( )--(b)--( )--( )--( )\n\
 |    |    |    |    |\n\
( )--(w)--( )--( )--( )\n\
 |    |    |    |    |\n\
( )--( )--( )--( )--( ) \n\
\n\
Here white fails to capture a black piece because\n\
of the other black piece which interferes with the \n\
capturing position:\n\
( )--( )--( )--( )--( )          \n\
 |    |    |    |    |   \n\
( )--(b)--(w)--(w)--(b)  \n\
 |    |    |    |    |  \n\
( )--( )--( )--( )--( ) \n\
 |    |    |    |    |\n\
( )--( )--( )--( )--( )\n\
 |    |    |    |    |\n\
( )--( )--( )--( )--( )\n\
\n\
Here white fails to capture a black piece because \n\
the white piece is not adjacent to the black piece:\n\
( )--( )--( )--( )--( )          \n\
 |    |    |    |    |   \n\
(b)--( )--(w)--(w)--( ) \n\
 |    |    |    |    |  \n\
( )--( )--( )--( )--( )\n\
 |    |    |    |    |\n\
( )--( )--( )--( )--( )\n\
 |    |    |    |    |\n\
( )--( )--( )--( )--( )\n"                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          ;

STRING kHelpReverseObjective =
        "Try to lose all but one of your pieces.";

STRING kHelpTieOccursWhen =
        "A tie occurs when ...";

STRING kHelpExample =
        "Player Player 1's turn \n\
        (u)p            \n\
          |             \n\
(l)eft --( )-- (r)ight   \n\
          |             \n\
       (d)own           \n\
                     \n\
Board:          Key: \n\
(b)--(b)--(b)           ( 1)--( 2)--( 3)\n\
 |    |    |             |     |     |  \n\
( )--( )--( )           ( 4)--( 5)--( 6)\n\
 |    |    |             |     |     |   \n\
(w)--(w)--(w)           ( 7)--( 8)--( 9) \n\
\n\
\n\
Player 1's move [(undo)/(key dir [1 d])] : 8 u\n\
Player Player 2's turn\n\
        (u)p           \n\
          |             \n\
(l)eft --( )-- (r)ight   \n\
          |             \n\
       (d)own           \n\
                     \n\
\n\
Board:          Key: \n\
(b)--(b)--(b)           ( 1)--( 2)--( 3)\n\
 |    |    |             |     |     |\n\
( )--(w)--( )           ( 4)--( 5)--( 6)\n\
 |    |    |             |     |     |\n\
(w)--( )--(w)           ( 7)--( 8)--( 9)\n\
\n\
 \n\
Player 2's move [(undo)/(key dir [1 d])] : 1 d\n\
Player Player 1's turn\n\
        (u)p           \n\
          |             \n\
(l)eft --( )-- (r)ight   \n\
          |             \n\
       (d)own           \n\
                     \n\
Board:          Key: \n\
( )--(b)--(b)           ( 1)--( 2)--( 3)\n\
 |    |    |             |     |     |\n\
(b)--(w)--( )           ( 4)--( 5)--( 6)\n\
 |    |    |             |     |     |\n\
(w)--( )--(w)           ( 7)--( 8)--( 9)\n\
\n\
\n\
Player 1's move [(undo)/(key dir [1 d])] : 9 u\n\
Player Player 2's turn\n\
        (u)p            \n\
          |             \n\
(l)eft --( )-- (r)ight   \n\
          |             \n\
       (d)own           \n\
                     \n\
Board:          Key: \n\
( )--(b)--(b)           ( 1)--( 2)--( 3)\n\
 |    |    |             |     |     |\n\
( )--(w)--(w)           ( 4)--( 5)--( 6)\n\
 |    |    |             |     |     |\n\
(w)--( )--( )           ( 7)--( 8)--( 9)\n\
\n\
 \n\
Player 2's move [(undo)/(key dir [1 d])] : 2 l\n\
Player Player 1's turn\n\
        (u)p           \n\
          |             \n\
(l)eft --( )-- (r)ight   \n\
          |             \n\
       (d)own           \n\
\n\
Board:          Key:\n\
(b)--( )--(b)           ( 1)--( 2)--( 3)\n\
 |    |    |             |     |     |\n\
( )--(w)--(w)           ( 4)--( 5)--( 6)\n\
 |    |    |             |     |     |\n\
(w)--( )--( )           ( 7)--( 8)--( 9)\n\
\n\
 \n\
Player 1's move [(undo)/(key dir [1 d])] : 5 l \n\
Player Player 2's turn\n\
        (u)p           \n\
          |             \n\
(l)eft --( )-- (r)ight   \n\
          |             \n\
       (d)own           \n\
                     \n\
Board:          Key: \n\
( )--( )--(b)           ( 1)--( 2)--( 3)\n\
 |    |    |             |     |     |\n\
(w)--( )--(w)           ( 4)--( 5)--( 6)\n\
 |    |    |             |     |     |\n\
(w)--( )--( )           ( 7)--( 8)--( 9)\n\
\n\
Player 1 (player one) Wins!"                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          ;


/*************************************************************************
**
** #defines and structs
**
**************************************************************************/
#define BLANK ' '
#define WHITE 'w' /* player 1 */
#define BLACK 'b' /* player 2 */

#define UP 0
#define RIGHT 1
#define DOWN 2
#define LEFT 3
#define UPLEFT 4
#define UPRIGHT 5
#define DOWNRIGHT 6
#define DOWNLEFT 7



/*************************************************************************
**
** Global Variables
**
*************************************************************************/

/* default values */

int gBoardwidth = 3;
int gBoardlength;
char* gBoard;
BOOLEAN diagonals = FALSE;

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
extern POSITION         generic_hash_init(int boardsize, int pieces_array[], int (*vcfg_function_ptr)(int* cfg), int player);
extern POSITION         generic_hash_hash(char *board, int player);
extern char            *generic_hash_unhash(POSITION hash_number, char *empty_board);
extern int              generic_hash_turn (POSITION hashed);

/* not external */
MOVE EncodeMove(int dir, int x, int y);
int GetDirection (MOVE theMove);
int GetXCoord (MOVE theMove);
int GetYCoord (MOVE theMove);
int getArraynum(int xcoord, int ycoord);
int getColumn(int arraynum);
int getRow(int arraynum);
BOOLEAN canmoveup(int arraynum);
BOOLEAN canmovedown(int arraynum);
BOOLEAN canmoveleft(int arraynum);
BOOLEAN canmoveright(int arraynum);
BOOLEAN canmoveupleft(int arraynum);
BOOLEAN canmovedownright(int arraynum);
BOOLEAN canmovedownleft(int arraynum);
BOOLEAN canmoveupright(int arraynum);

STRING MoveToString( MOVE );

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
	int i, j, k;
	gBoardlength = gBoardwidth * gBoardwidth;
	int pieces_array[] = {WHITE, 1, gBoardwidth, BLACK, 1, gBoardwidth, BLANK, gBoardlength - (gBoardwidth * 2), gBoardlength - 2, -1 };
	gNumberOfPositions = generic_hash_init(gBoardlength, pieces_array, NULL, 0);

	int reflections[] = {0, 45, 90, 135};
	int rotations[] = {90, 180, 270};

	generic_hash_init_sym(0, gBoardwidth, gBoardwidth, reflections, 4, rotations, 3, 1);

	gBoard = (char*)SafeMalloc(sizeof(char) * (gBoardlength + 1));
	for(i = 0; i < gBoardwidth; i++) {
		gBoard[i] = BLACK;
	}
	for (j = gBoardlength - 1; j > gBoardlength - gBoardwidth - 1; j--) {
		gBoard[j] = WHITE;
	}
	for (k = gBoardwidth; k < gBoardlength - gBoardwidth; k++) {
		gBoard[k] = BLANK;
	}
	gBoard[gBoardlength] = '\0';
	gInitialPosition = generic_hash_hash(gBoard, 1);
	/* printf("This is the initialPosition:%d", gInitialPosition); */

	gMoveToStringFunPtr = &MoveToString;
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

MOVELIST *GenerateMoves (POSITION position)
{
	MOVELIST *moves = NULL;

	/* Use CreateMovelistNode(move, next) to 'cons' together a linked list */
	int player = generic_hash_turn(position);
	char* board = (char*)generic_hash_unhash (position, gBoard);
	char playerpiece = (player == 1 ? WHITE : BLACK);
	int i,c,r;
	for (i = 0; i < gBoardlength; i++) {
		if (board[i] == playerpiece) {
			c = getColumn (i);
			r = getRow (i);
			if (canmoveup(i)) {
				moves = CreateMovelistNode(EncodeMove(UP,c,r), moves);
			}
			if (canmovedown(i)) {
				moves = CreateMovelistNode(EncodeMove(DOWN,c,r), moves);
			}
			if (canmoveleft(i)) {
				moves = CreateMovelistNode(EncodeMove(LEFT,c,r), moves);
			}
			if (canmoveright(i)) {
				moves = CreateMovelistNode(EncodeMove(RIGHT,c,r), moves);
			}
			/* If Diagonal moves are allowed */
			if (diagonals) {
				if (canmoveupleft(i))
					moves = CreateMovelistNode(EncodeMove(UPLEFT, c, r), moves);
				if (canmoveupright(i)) {
					moves = CreateMovelistNode(EncodeMove(UPRIGHT, c, r), moves);
				}
				if (canmovedownleft(i))
					moves = CreateMovelistNode(EncodeMove(DOWNLEFT, c, r), moves);
				if (canmovedownright(i))
					moves = CreateMovelistNode(EncodeMove(DOWNRIGHT, c, r), moves);
			}
		}
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

	int dir = GetDirection(move);
	int x0 = GetXCoord(move);
	int y0 = GetYCoord(move);
	int x1, y1;

	char* board = (char*)generic_hash_unhash(position, gBoard);
	int player = generic_hash_turn(position);
	int nextPlayer;
	if (player == 1) {
		nextPlayer = 2;
	} else {
		nextPlayer = 1;
	}

	/* add new piece */

	/* store new piece coords in x1 y1 */
	switch(dir) {
	case UP:
		x1 = x0;
		y1 = y0 + 1;
		break;
	case RIGHT:
		x1 = x0 + 1;
		y1 = y0;
		break;
	case DOWN:
		x1 = x0;
		y1 = y0 - 1;
		break;
	case LEFT:
		x1 = x0 - 1;
		y1 = y0;
		break;
	case UPLEFT:
		x1 = x0 - 1;
		y1 = y0 + 1;
		break;
	case UPRIGHT:
		x1 = x0 + 1;
		y1 = y0 + 1;
		break;
	case DOWNRIGHT:
		x1 = x0 + 1;
		y1 = y0 - 1;
		break;
	case DOWNLEFT:
		x1 = x0 - 1;
		y1 = y0 - 1;
		break;
	default:
		printf("Error: bad switch in DoMove");
		break;
	}

	/* convert x1 y1 to position index */
	int pIndex1 = getArraynum(x1, y1);

	/* get type of piece */
	int pIndex0 = getArraynum(x0, y0);
	char pType = board[pIndex0];


	/* doing basic movement */
	board[pIndex0] = BLANK;
	board[pIndex1] = pType;

	/* capturing stuff code goes here:
	   break into check row and column
	   check 3 piece row/column
	   check for adjacent friend piece
	   check for adjacent enemy piece */


	/* checking row and column for 3 pieces */
	int tmpRow;
	int tmpCol;
	int numRowPieces = 0;
	int numColPieces = 0;
	int pIndex;
	/* check row */
	for (tmpRow = 0; tmpRow < gBoardwidth; tmpRow++) {
		pIndex = getArraynum(tmpRow, y1);
		if (board[pIndex] != BLANK) {
			numRowPieces++;
		}
	}
	/* check column */
	for (tmpCol = 0; tmpCol < gBoardwidth; tmpCol++) {
		pIndex = getArraynum(x1, tmpCol);
		if (board[pIndex] != BLANK) {
			numColPieces++;
		}
	}

	/* check for adjacent friend piece for row */
	char playerpiece = (player == 1 ? WHITE : BLACK);
	int ax1, ay1, ax2, ay2; /* adjacent coords 1, adjacent coords 2 */
	int a1exist = 0, a2exist = 0;
	if (numRowPieces == 3) {
		if (x1 != 0) {
			ax1 = x1 - 1;
			ay1 = y1;
			pIndex = getArraynum(ax1, ay1);
			if (board[pIndex] == playerpiece) {
				a1exist = 1;
			}
		}
		if (x1 < gBoardwidth-1) {
			ax2 = x1 + 1;
			ay2 = y1;
			pIndex = getArraynum(ax2, ay2);
			if (board[pIndex] == playerpiece) {
				a2exist = 1;
			}
		}
	}

	/* check for adjacent enemy piece for row, and remove from board if found */
	char enemypiece  = (player == 1 ? BLACK : WHITE);
	int ex, ey; /* enemy coords */
	if (a1exist) {
		if (ax1 != 0) {
			ex = ax1 - 1;
			ey = ay1;
			pIndex = getArraynum(ex, ey);
			if (board[pIndex] == enemypiece) {
				board[pIndex] = BLANK; /* removes enemy piece */
			}
		}
		if (x1 < gBoardwidth-1) {
			ex = x1 + 1;
			ey = y1;
			pIndex = getArraynum(ex, ey);
			if (board[pIndex] == enemypiece) {
				board[pIndex] = BLANK; /* removes enemy piece */
			}
		}
	}

	if (a2exist) {
		if (x1 != 0) {
			ex = x1 - 1;
			ey = y1;
			pIndex = getArraynum(ex, ey);
			if (board[pIndex] == enemypiece) {
				board[pIndex] = BLANK; /* removes enemy piece */
			}
		}
		if (ax2 < gBoardwidth-1) {
			ex = ax2 + 1;
			ey = ay2;
			pIndex = getArraynum(ex, ey);
			if (board[pIndex] == enemypiece) {
				board[pIndex] = BLANK; /* removes enemy piece */
			}
		}
	}



	/* check for adjacent friend piece for column */
	a1exist = 0; /* reset for column */
	a2exist = 0;
	if (numColPieces == 3) {
		if (y1 != 0) {
			ax1 = x1;
			ay1 = y1 - 1;
			pIndex = getArraynum(ax1, ay1);
			if (board[pIndex] == playerpiece) {
				a1exist = 1;
			}
		}
		if (y1 < gBoardwidth-1) {
			ax2 = x1;
			ay2 = y1 + 1;
			pIndex = getArraynum(ax2, ay2);
			if (board[pIndex] == playerpiece) {
				a2exist = 1;
			}
		}
	}

	/* check for adjacent enemy piece for column, and remove from board if found */
	if (a1exist) {
		if (ay1 != 0) {
			ex = ax1;
			ey = ay1 - 1;
			pIndex = getArraynum(ex, ey);
			if (board[pIndex] == enemypiece) {
				board[pIndex] = BLANK; /* removes enemy piece */
			}
		}
		if (y1 < gBoardwidth-1) {
			ex = x1;
			ey = y1 + 1;
			pIndex = getArraynum(ex, ey);
			if (board[pIndex] == enemypiece) {
				board[pIndex] = BLANK; /* removes enemy piece */
			}
		}
	}

	if (a2exist) {
		if (y1 != 0) {
			ex = x1;
			ey = y1 - 1;
			pIndex = getArraynum(ex, ey);
			if (board[pIndex] == enemypiece) {
				board[pIndex] = BLANK; /* removes enemy piece */
			}
		}
		if (ay2 < gBoardwidth-1) {
			ex = ax2;
			ey = ay2 + 1;
			pIndex = getArraynum(ex, ey);
			if (board[pIndex] == enemypiece) {
				board[pIndex] = BLANK; /* removes enemy piece */
			}
		}
	}

	return generic_hash_hash(board, nextPlayer);

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

VALUE Primitive (POSITION position)
{

	int player = generic_hash_turn(position);
	char* board = (char*)generic_hash_unhash (position, gBoard);
	char playerpiece = (player == 1 ? WHITE : BLACK);
	int numplayerpiece = 0;
	int i;
	for (i = 0; i < gBoardlength; i++) {
		if (board[i] == playerpiece)
			numplayerpiece++;
	}
	if (numplayerpiece <= 1)
		return gStandardGame ? lose : win;
	else if (GenerateMoves(position) == NULL) {
		return gStandardGame ? lose : win;
	}
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
	char* board;
	board = (char*)generic_hash_unhash(position, gBoard);
	int player = generic_hash_turn(position);
	STRING playercolor;
	if (player == 1)
		playercolor = "WHITE";
	else
		playercolor = "BLACK";
	printf("Player %s's turn (%s)\n", playersName,playercolor );
	/* counters */
	int h, i, j, k, c, c1;
	/* for diagonals */
	if (diagonals) {
		c = 1;
		printf("    (ul)      (ur)             \n");
		printf("       \\ (u)p/           \n");
		printf("        \\ | /           \n");
		printf("(l)eft --( )-- (r)ight     \n");
		printf("        / | \\           \n");
		printf("       /(d)own\\           \n");
		printf("    (dl)       (dr) \n");
		printf("                     \n");
		printf("Board: ");
		for (h = 0; h < gBoardwidth-1; h++) {
			printf("\t");
		}
		printf("Key: \n");
		for (i = 0; i < gBoardlength; ) {
			for (j = 0; j < gBoardwidth; j++) {
				printf ("(%c)", board [i]);
				i++;
				if (j < gBoardwidth - 1) {
					printf("-");
				}
			}
			printf("\t\t");
			for (c1 = 0; c1 < gBoardwidth; c1++) {
				if (c < 10) {
					printf ("( %d)", c);
					c++;
					if (c1 < gBoardwidth - 1) {
						printf("--");
					}
				}
				else {
					printf ("(%d)", c);
					c++;
					if (c1 < gBoardwidth - 1) {
						printf("--");
					}
				}
			}
			if (i < gBoardlength - gBoardwidth + 1) {
				printf("\n");
				for (k = 0; k < gBoardwidth - 1; k++) {
					printf(" | X");
				}
				printf(" |");
				printf("\t\t");
				for (k = 0; k < gBoardwidth - 1; k++) {
					printf(" |    ");
				}
				printf(" |");
				printf("\t\t");
			}
			printf("\n");
		}
	}
	/* for regular game */
	else {
		c = 1;
		printf("        (u)p            \n");
		printf("          |             \n");
		printf("(l)eft --( )-- (r)ight     \n");
		printf("          |             \n");
		printf("       (d)own           \n");
		printf("                     \n");
		printf("Board: ");
		for (h = 0; h < gBoardwidth-1; h++) {
			printf("\t");
		}
		printf("Key: \n");
		for (i = 0; i < gBoardlength; ) {
			for (j = 0; j < gBoardwidth; j++) {
				printf ("(%c)", board [i]);
				i++;
				if (j < gBoardwidth - 1) {
					printf("--");
				}
			}
			printf("\t\t");
			for (c1 = 0; c1 < gBoardwidth; c1++) {
				if (c < 10) {
					printf ("( %d)", c);
					c++;
					if (c1 < gBoardwidth - 1) {
						printf("--");
					}
				}
				else {
					printf ("(%d)", c);
					c++;
					if (c1 < gBoardwidth - 1) {
						printf("--");
					}
				}
			}
			if (i < gBoardlength - gBoardwidth + 1) {
				printf("\n");
				for (k = 0; k < gBoardwidth - 1; k++) {
					printf(" |   ");
				}
				printf(" |");
				printf("\t\t");
				for (k = 0; k < gBoardwidth - 1; k++) {
					printf(" |    ");
				}
				printf(" |");
				printf("\t\t");
			}
			printf("\n");
		}
	}
	printf("\n%s\n", GetPrediction(position, playersName, usersTurn));

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
	printf("%8s's move   : ", computersName);
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
	STRING m = MoveToString( move );
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
	STRING m = (STRING) SafeMalloc( 8 );
	int xcoord, ycoord, dir, Arraynum;
	STRING direction;
	xcoord = GetXCoord(theMove);
	ycoord = GetYCoord(theMove);
	dir = GetDirection(theMove);
	Arraynum = getArraynum(xcoord, ycoord);
	if (dir == UP)
		direction = "u";
	else if (dir == RIGHT)
		direction = "r";
	else if (dir == DOWN)
		direction = "d";
	else if (dir == LEFT)
		direction = "l";
	//for diagonals only
	else if (dir == UPLEFT)
		direction = "ul";
	else if (dir == UPRIGHT)
		direction = "ur";
	else if (dir == DOWNRIGHT)
		direction = "dr";
	else if (dir == DOWNLEFT)
		direction = "dl";
	sprintf( m, "[%d %s]", Arraynum + 1, direction);
	return m;
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
		printf("%8s's move [(undo)/(key dir [1 d])] : ", playersName);

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
	if (input[0] < '0' || input[0] > '9') {
		printf("badinput[0]");
		return FALSE;
	}
	char *spcptr = strchr(input, ' ');
	if (spcptr == NULL) {
		printf("badspcptr");
		return FALSE;
	}

	*spcptr = '\0';
	char *direction = ++spcptr;

	if (strcmp(direction,"u") && strcmp(direction,"r") &&
	    strcmp(direction,"d") && strcmp(direction,"l")&&
	    strcmp(direction,"ur") && strcmp(direction,"ul") &&
	    strcmp(direction,"dr") && strcmp(direction,"dl"))  {
		printf("badelse");
		return FALSE;
	}
	*(spcptr-1) = ' ';
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
	char *spcptr = strchr(input, ' ');
	*spcptr = '\0';
	char *direction = ++spcptr;
	int location = atoi(input);
	location--;

	int dir;
	if (0 == strcmp(direction,"u")) {
		dir = UP;
	} else if (0 == strcmp(direction,"r")) {
		dir = RIGHT;
	} else if (0 == strcmp(direction,"d")) {
		dir = DOWN;
	} else if (0 == strcmp(direction,"l")) {
		dir = LEFT;
	}
	//for diagonals
	else if (0 == strcmp(direction, "ul")) {
		dir = UPLEFT;
	}
	else if (0 == strcmp(direction, "ur")) {
		dir = UPRIGHT;
	}
	else if (0 == strcmp(direction, "dl")) {
		dir = DOWNLEFT;
	}
	else if (0 == strcmp(direction, "dr")) {
		dir = DOWNRIGHT;
	} else {
		printf("bad else in ConvertTextInputToMove");
	}


	int x, y;
	x = getColumn(location);
	y = getRow(location);
	return EncodeMove(dir, x, y);
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
	STRING D;
	char GetMyChar();
	int intWidth;

	if (diagonals) {
		D = "ON";
	}
	else
		D = "OFF";

	printf("\n");
	printf("Wuzhi Game Specific Menu:\n");
	printf("\tw) \tChange the Board (W)idth. Current Board Width: %d\n", gBoardwidth);
	printf("\td) \tToggle (D)IAGONAL movement. Currently Diagonals is: %s\n", D);
	printf("\tb) \t(B)ack to previous menu\n");
	printf("\n\n\tq) \t(Q)uit\n");
	printf("Select an option: ");

	switch(GetMyChar()) {
	case 'q':
	case 'Q':
		printf("\n");
		ExitStageRight();
		break;
	case 'w':
	case 'W':
		printf("Please input desired board width[3-5]: ");
		intWidth = GetMyUInt();
		gBoardwidth = intWidth;
		gBoardlength = gBoardwidth*gBoardwidth;
		InitializeGame();
		GameSpecificMenu();
		break;
	case 'd':
	case 'D':
		if (diagonals)
			diagonals = FALSE;
		else
			diagonals = TRUE;
		GameSpecificMenu();
		break;
	case 'b':
	case 'B':
		return;
	default:
		printf("Not a valid option.\n");
		GameSpecificMenu();
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
	printf ("Calling getInitialPosition in C");
	/* int i, j, k;
	   gBoardlength = gBoardwidth * gBoardwidth;
	   int pieces_array[] = {WHITE, 1, gBoardwidth, BLACK, 1, gBoardwidth, BLANK, gBoardlength - (gBoardwidth * 2), gBoardlength - 2, -1 };
	   gNumberOfPositions = generic_hash_init(gBoardlength, pieces_array, NULL, 0);
	   gBoard = (char*)malloc(sizeof(char) * (gBoardlength + 1));
	   for(i = 0; i < gBoardwidth; i++) {
	   gBoard[i] = BLACK;
	   }
	   for (j = gBoardlength - 1; j > gBoardlength - gBoardwidth - 1; j--) {
	   gBoard[j] = WHITE;
	   }
	   for (k = gBoardwidth; k < gBoardlength - gBoardwidth; k++) {
	   gBoard[k] = BLANK;
	   }
	   gBoard[gBoardlength] = '\0';
	   gInitialPosition = generic_hash_hash(gBoard,1);
	   //free(gBoard); */
	return gInitialPosition;

	// return gInitialPosition;
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
	return 2*2*3;
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
	int option = 1;
	if (gStandardGame)
		option = 1;
	else
		option = 0;
	option += (diagonals << 1);
	option += (gBoardwidth - 3) << 2;
	return option;

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
	gStandardGame = ((option & 1) == 1);
	diagonals = (option >> 1) & 1;
	gBoardwidth = (option >> 2) + 3;
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

/* MOVE stuff
   MOVES are hashed where first three bits is the dir, second three is x, last three is y */
MOVE EncodeMove(int dir, int x, int y)
{

	dir = dir << 6;
	x = x << 3;
	return (dir | x | y);
}

int GetDirection (MOVE theMove)
{
	return (theMove >> 6);
}

int GetXCoord (MOVE theMove)
{
	return (theMove >> 3) & 0x7;
}

int GetYCoord (MOVE theMove)
{
	return (theMove & 0x7);
}


/* Helper Functions for Generate Moves
   All columns and rows start with zero and start from the lower left */
int getArraynum(int xcoord, int ycoord) {
	return ((gBoardwidth *(gBoardwidth - ycoord)) - (gBoardwidth - xcoord));
}


int getColumn(int arraynum)
{
	return (arraynum%gBoardwidth);
}

int getRow(int arraynum)
{

	return gBoardwidth - 1 - (arraynum/gBoardwidth);
}

BOOLEAN canmoveup(int arraynum)
{
	if (arraynum >= gBoardlength || arraynum < 0)
		return FALSE;
	else
		return ((arraynum - gBoardwidth >= 0) && (gBoard[arraynum - gBoardwidth] == BLANK));
}

BOOLEAN canmovedown(int arraynum)
{
	if (arraynum >= gBoardlength || arraynum < 0)
		return FALSE;
	else
		return ((arraynum + gBoardwidth < gBoardlength) && (gBoard[arraynum + gBoardwidth] == BLANK));
}

BOOLEAN canmoveleft(int arraynum)
{
	if (arraynum >= gBoardlength || arraynum < 0)
		return FALSE;
	else
		return ((!(arraynum <= 0)) && arraynum%gBoardwidth != 0) &&
		       (gBoard[arraynum  - 1] == BLANK);
}

BOOLEAN canmoveright(int arraynum)
{
	if (arraynum >= gBoardlength || arraynum < 0)
		return FALSE;
	else
		return ((arraynum + 1)% gBoardwidth != 0 && (gBoard[arraynum + 1] == BLANK));
}

//for diagonals
BOOLEAN canmoveupleft(int arraynum)
{
	return (canmoveup(arraynum-1) && canmoveleft(arraynum-gBoardwidth));
}

BOOLEAN canmoveupright(int arraynum)
{

	return (canmoveup(arraynum + 1) && canmoveright(arraynum-gBoardwidth));
}

BOOLEAN canmovedownleft(int arraynum)
{
	return (canmovedown(arraynum -1) && canmoveleft(arraynum+gBoardwidth));
}

BOOLEAN canmovedownright(int arraynum)
{
	return (canmovedown(arraynum+1) && canmoveright(arraynum+gBoardwidth));
}



POSITION StringToPosition(char* board) {
	return -1;
}


char* PositionToString(POSITION pos) {
	// FIXME: this is just a stub
	return "Implement Me";
}

char * PositionToEndData(POSITION pos) {
	return NULL;
}
