/************************************************************************
**
** NAME:        NAME OF FILE
**
** DESCRIPTION: Lines of Action
**
** AUTHOR:      Albert Shau
**
** DATE:        Started 9/18/2006
**
** UPDATE HIST: 2008.05.05 Fixed getOption to start at 1 instead of 0
**              2006.09.18 Coded InitializeGame and PrintPosition
**                         without hashes
**              2006.09.19 Fixed PrintPosition
**              2006.09.25 Fixed InitializeGame and PrintPosition to work
**                         with generic hash
**              2006.09.26 Coded Primitive and created isConnected to help
**                         Coded GenerateMoves and its helper functions
**              2006.10.07 Coded DoMove and debugged all previous functions
**                         fixed piecesInLineOfAction, got rid of board
**                         within functions in favor of gBoard everywhere
**                         to get rid of seg faults, fixed Primitive so that
**                         result is not always undecided
**              2006.10.12 Debugged Primitive and changed the way the board
**                         is printed so it's like a chessboard.
**              2006.10.13 Fixed Primitive to check for a tie. Fixed
**                         MoveToString (forgot to add '\0' to the end of
**                         strings)
**              2006.10.16 Fixed GenerateMoves to check that the destination
**                         square is blank before adding to list of moves.
**              2006.10.17 Fixed GenerateMoves to check that the destination
**                         square does not have a piece of your own color.
**                         Fixed moveHash hardcoding 4 instead of SIDELENGTH
**                         Wrote my own hash, unhash and turn functions since
**                         generic_hash stuff was acting up and didn't let me
**                         go over a 4x4 board. Fixed Primitive so it doesn't
**                         think win means Black won.  Instead, win means the
**                         player whose turn it is has won.
**              2006.11.28 Added game specific menu, allowing you to resize
**                         the board.  Added help strings and misere to
**                         Primitive.
**              2006.12.13 Changed PrintPosition to also display info on
**                         whose turn it is and what piece that player is using.
**                         Also changed to 1-based game variants.
**              2007.2.16  Fixed primitive so it didn't try and free a null pointer
**              2007.2.19  Added symmetries using generic symmetries
**
**
**
** LAST CHANGE: $Id: mloa.c,v 1.18 2008-05-08 06:09:42 l156steven Exp $
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


/*************************************************************************
**
** Game-specific constants
**
**************************************************************************/

STRING kGameName            = "Lines of Action";
STRING kAuthorName          = "Albert Shau";
STRING kDBName              = "loa";   /* The name to store the database under */

BOOLEAN kPartizan            = TRUE;
BOOLEAN kGameSpecificMenu    = TRUE;   /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN kTieIsPossible       = TRUE;   /* TRUE if a tie is possible. FALSE if it is impossible.*/
BOOLEAN kLoopy               = TRUE;

BOOLEAN kDebugMenu           = FALSE;   /* TRUE only when debugging. FALSE when on release. */
BOOLEAN kDebugDetermineValue = FALSE;   /* TRUE only when debugging. FALSE when on release. */

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
        "No graphic interface yet.";

STRING kHelpTextInterface =
        "The board is a square with columns labeled by letter and rows labeled by number. \n\
A position is referenced with a letter first and then a number.  A move consists \n\
of two positions: the starting position followed by the ending position.  \n\
For example, the bottom left square of the board is a1.  If you want to move a piece \n\
from a1 to a3, the format of the move is a1a3."                                                                                                                                                                                                                                                                                                                                                          ;

STRING kHelpOnYourTurn =
        "On your turn, pick a piece of your color that you wish to move.  You may move \n\
in a straight line in any direction.  This line is called a line of action. \n\
To find the number of spaces you may move in a given line of action, count all \n\
pieces, both your own and your opponents', along that line.  You may move that many \n\
squares in that line.  You may jump over your own pieces, but not your opponents'. \n\
You may also capture the opponents' pieces if your piece lands directly on it.  You \n\
may not capture your own pieces."                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     ;

STRING kHelpStandardObjective =
        "The objective of the game is to move all your pieces into a single connected \n\
block.  A piece is connected to another piece if it occupies one of the eight \n\
squares directly surrounding the other peice.  If a move causes both players' \n\
pieces to be connected in one block, the player who just moved wins the game.  \n\
If a player only has one piece remaining, that player has won.  The board below \n\
illustrates a game in which X has won.\n\
     +---+---+---+---+\n\
   4 |   |   | X |   |\n\
     +---+---+---+---+\n\
   3 |   | X | o | X |\n\
     +---+---+---+---+\n\
   2 | o | X |   | o |\n\
     +---+---+---+---+\n\
   1 |   |   |   |   |\n\
     +---+---+---+---+\n\
       a   b   c   d "                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         ;

STRING kHelpReverseObjective =
        "The misere (reverse) objective of the game to get your opponents pieces into \n\
a single connected block.  If the opponent only has one piece left, that counts \n\
as a single connected block.  If a move results in both players having single \n\
connected blocks of their own pieces, the player that just moved loses."                                                                                                                                                                                                                                                                ;

// A tie occurs when is already displayed above this
STRING kHelpTieOccursWhen =
        "a player cannot move on his/her turn.";

STRING kHelpExample =
        "The following is an example game played on a 4x4 board. Player1 is X and Player2 is o. \n\
\n\
Starting Position:\n\
\n\
     +---+---+---+---+\n\
   4 |   | X | X |   |\n\
     +---+---+---+---+\n\
   3 | o |   |   | o |\n\
     +---+---+---+---+\n\
   2 | o |   |   | o |\n\
     +---+---+---+---+\n\
   1 |   | X | X |   |\n\
     +---+---+---+---+\n\
       a   b   c   d \n\
\n\
Player1 moves: c1a3.\n\
\n\
     +---+---+---+---+\n\
   4 |   | X | X |   |\n\
     +---+---+---+---+\n\
   3 | X |   |   | o |\n\
     +---+---+---+---+\n\
   2 | o |   |   | o |\n\
     +---+---+---+---+\n\
   1 |   | X |   |   |\n\
     +---+---+---+---+\n\
       a   b   c   d \n\
\n\
Player2 moves: d2b4.\n\
\n\
     +---+---+---+---+\n\
   4 |   | o | X |   |\n\
     +---+---+---+---+\n\
   3 | X |   |   | o |\n\
     +---+---+---+---+\n\
   2 | o |   |   |   |\n\
     +---+---+---+---+\n\
   1 |   | X |   |   |\n\
     +---+---+---+---+\n\
       a   b   c   d \n\
\n\
Player1 moves: b1b3.\n\
\n\
Player 1 wins!"                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           ;



/*************************************************************************
**
** #defines and structs
****ccbb
**************************************************************************/
#define PLAYERBLACK 1
#define PLAYERWHITE 2

#define BLANK ' '
#define BLACK 'X'
#define WHITE 'o'

typedef enum possibleDirections {
	UP, UPRIGHT, RIGHT, DOWNRIGHT, DOWN, DOWNLEFT, LEFT, UPLEFT
} Direction;


/*************************************************************************
**
** Global Variables
**
*************************************************************************/
// length is left to right, height is up to down
int gBoardLength = 4;
int gBoardHeight = 4;
int gBoardSize = 16;
char* gBoard;


/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/
void addPieceMoves(int boardSquare, int PlayerTurn, MOVELIST **moves);
void addMovesInDirection(Direction direction, int boardSquare, int PlayerTurn, MOVELIST **moves);
int piecesInLineOfAction(Direction direction, int boardSquare);
BOOLEAN onEdge(Direction direction, int boardSquare);
BOOLEAN isConnected(int boardSquare1, int boardSquare2);
BOOLEAN pieceIsolated(int boardSquare);
Direction oppositeDirection(Direction direction);
POSITION setInitialPosition();
int numPiecesLeft(char color);
STRING moveUnhash(MOVE move);
MOVE moveHash(STRING input);
char* boardUnhash(POSITION pos, char* board);
POSITION boardHash(char* board, int player);
POSITION power(POSITION base, int exponent);
int boardHash_turn(POSITION pos);
int goInDirection(Direction direction);
void InitializeHelpStrings();


/* External */
#ifndef MEMWATCH
extern GENERIC_PTR      SafeMalloc ();
extern void             SafeFree ();
#endif

STRING                  MoveToString(MOVE move);

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
	InitializeHelpStrings();

	int pieces[] = {BLANK, gBoardSize - 2*(gBoardLength-2) - 2*(gBoardHeight-2), gBoardSize-1,
		        BLACK, 1, 2*(gBoardLength-2), WHITE, 1, 2*(gBoardHeight-2), -1};

	gNumberOfPositions = generic_hash_init(gBoardSize, pieces, NULL, 0);
	//gNumberOfPositions = power(3, gBoardSize+1);
	gBoard = (char*)SafeMalloc(sizeof(char) * (gBoardSize+1));
	gInitialPosition = setInitialPosition();

	int rotations[1] = {90};
	int reflections[1] = {90};
	if (gBoardHeight == gBoardLength)
		generic_hash_init_sym(0, gBoardHeight, gBoardLength, reflections, 1, rotations, 1, 0);
	else
		generic_hash_init_sym(0, gBoardHeight, gBoardLength, reflections, 1, NULL, 0, 0);

	//printf("gInitialPosition = " POSITION_FORMAT "\n",gInitialPosition);
	//printf("gNumberOfPositions = " POSITION_FORMAT "\n",gNumberOfPositions);
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

	kHelpGraphicInterface =
	        "No graphic interface yet.";

	kHelpTextInterface =
	        "The board is a square with columns labeled by letter and rows labeled by number. \n\
A position is referenced with a letter first and then a number.  A move consists \n\
of two positions: the starting position followed by the ending position.  \n\
For example, the bottom left square of the board is a1.  If you want to move a piece \n\
from a1 to a3, the format of the move is a1a3."                                                                                                                                                                                                                                                                                                                                                                  ;

	kHelpOnYourTurn =
	        "On your turn, pick a piece of your color that you wish to move.  You may move \n\
in a straight line in any direction.  This line is called a line of action. \n\
To find the number of spaces you may move in a given line of action, count all \n\
pieces, both your own and your opponents', along that line.  You may move that many \n\
squares in that line.  You may jump over your own pieces, but not your opponents'. \n\
You may also capture the opponents' pieces if your piece lands directly on it.  You \n\
may not capture your own pieces."                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             ;

	kHelpStandardObjective =
	        "The objective of the game is to move all your pieces into a single connected \n\
block.  A piece is connected to another piece if it occupies one of the eight \n\
squares directly surrounding the other peice.  If a move causes both players' \n\
pieces to be connected in one block, the player who just moved wins the game.  \n\
If a player only has one piece remaining, that player has won.  The board below \n\
illustrates a game in which X has won.\n\
     +---+---+---+---+\n\
   4 |   |   | X |   |\n\
     +---+---+---+---+\n\
   3 |   | X | O | X |\n\
     +---+---+---+---+\n\
   2 | O | X |   | O |\n\
     +---+---+---+---+\n\
   1 |   |   |   |   |\n\
     +---+---+---+---+\n\
       a   b   c   d "                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 ;

	kHelpReverseObjective =
	        "The misere (reverse) objective of the game to get your opponents pieces into \n\
a single connected block.  If the opponent only has one piece left, that counts \n\
as a single connected block.  If a move results in both players having single \n\
connected blocks of their own pieces, the player that just moved loses."                                                                                                                                                                                                                                                                        ;

	// A tie occurs when is already displayed above this
	kHelpTieOccursWhen =
	        "a player cannot move on his/her turn.";

	kHelpExample =
	        "The following is an example game played on a 4x4 board. Player1 is X and Player2 is O. \n\
\n\
Starting Position:\n\
\n\
     +---+---+---+---+\n\
   4 |   | X | X |   |\n\
     +---+---+---+---+\n\
   3 | O |   |   | O |\n\
     +---+---+---+---+\n\
   2 | O |   |   | O |\n\
     +---+---+---+---+\n\
   1 |   | X | X |   |\n\
     +---+---+---+---+\n\
       a   b   c   d \n\
\n\
Player1 moves: c1a3.\n\
\n\
     +---+---+---+---+\n\
   4 |   | X | X |   |\n\
     +---+---+---+---+\n\
   3 | X |   |   | O |\n\
     +---+---+---+---+\n\
   2 | O |   |   | O |\n\
     +---+---+---+---+\n\
   1 |   | X |   |   |\n\
     +---+---+---+---+\n\
       a   b   c   d \n\
\n\
Player2 moves: d2b4.\n\
\n\
     +---+---+---+---+\n\
   4 |   | O | X |   |\n\
     +---+---+---+---+\n\
   3 | X |   |   | O |\n\
     +---+---+---+---+\n\
   2 | O |   |   |   |\n\
     +---+---+---+---+\n\
   1 |   | X |   |   |\n\
     +---+---+---+---+\n\
       a   b   c   d \n\
\n\
Player1 moves: b1b3.\n\
\n\
Player 1 wins!"                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   ;
/*
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
 */
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
	int i;
	int playerTurn = generic_hash_turn(position);
	//int playerTurn = boardHash_turn(position);
	char playerColor = (playerTurn == PLAYERBLACK ? BLACK : WHITE);
	MOVELIST *moves = NULL;
	generic_hash_unhash(position, gBoard);
	//boardUnhash(position, gBoard);

	for (i = 0; i < gBoardSize; i++) {
		if (gBoard[i] == playerColor)
			addPieceMoves(i, playerTurn, &moves);
	}

	//printf("GenerateMoves ending ... \n");
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
	//printf("doing move... \n");
	int start, end;
	int playerTurn = generic_hash_turn(position);
	//int playerTurn = boardHash_turn(position);
	POSITION nextPosition;
	generic_hash_unhash(position, gBoard);
	//boardUnhash(position, gBoard);

	/* MOVE is just an int. 4 digits, the first two are the starting position
	   and the second two are the ending position.  For example, 1234 means
	   the piece at board[12] moves to board[34]. Something like 321 means
	   board[3] moves to board[21].
	 */
	end = move % 100;
	start = (move - end)/100;

	/* Move the piece */
	gBoard[end] = gBoard[start];
	gBoard[start] = BLANK;

	if (playerTurn == PLAYERBLACK)
		playerTurn = PLAYERWHITE;
	else
		playerTurn = PLAYERBLACK;

	nextPosition = generic_hash_hash(gBoard, playerTurn);
	//nextPosition = boardHash(gBoard, playerTurn);

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

VALUE Primitive (POSITION position)
{
	int i, j, blackChainLength, whiteChainLength, blackPiecesLeft, whitePiecesLeft;
	VALUE result;
	BOOLEAN allWhiteConnected, allBlackConnected, doneChecking;
	int playerTurn = generic_hash_turn(position);
	//int playerTurn = boardHash_turn(position);
	char* board = generic_hash_unhash(position, gBoard);
	//char* board = boardUnhash(position, gBoard);

	blackPiecesLeft = numPiecesLeft(BLACK);
	whitePiecesLeft = numPiecesLeft(WHITE);
	int chainOfBlacks[blackPiecesLeft+1], chainOfWhites[whitePiecesLeft];

	//printf("primitive starting...\n");
	// chains keep track of the boardSquare of every piece connected to that
	// chain of a certain color.  Initialize so that it's full of -1's except
	// for chain[0], which is the first piece we see on the board
	i = 0;
	//printf("initializing chains ... blacksleft = %d, whitesleft = %d \n", blackPiecesLeft, whitePiecesLeft);
	while (board[i] != BLACK) { i++; }
	chainOfBlacks[0] = i;
	board[i] = BLANK;
	for (i = 1; i < blackPiecesLeft; i++) {
		chainOfBlacks[i] = -1;
	}
	blackChainLength = 1;
	i = 0;
	while (board[i] != WHITE) { i++; }
	chainOfWhites[0] = i;
	board[i] = BLANK;
	for (i = 1; i < whitePiecesLeft; i++) {
		chainOfWhites[i] = -1;
	}
	whiteChainLength = 1;
	//printf("chains initialized \n");

	/******************************************************************************
	** goes through the gameboard and adds pieces to their corresponding chains.
	** If it goes through the whole gameboard without adding a piece to the chain,
	** then we know we're done. Chain length variables point to the place in the
	** array that new pieces should go.
	*****************************************************************************/
	doneChecking = FALSE;
	while (!doneChecking) {
		doneChecking = TRUE;
		for (i = 0; i < gBoardSize; i++) {
			if (board[i] == BLACK && blackPiecesLeft > 1) {
				//printf("checking to see if square %d is connected to chain\n", i);
				for (j = 0; chainOfBlacks[j] != -1; j++) {
					if (isConnected(i, chainOfBlacks[j])) {
						//printf("connected to chain! square %d is connected to %d\n", i, chainOfBlacks[j]);
						chainOfBlacks[blackChainLength] = i;
						blackChainLength++;
						doneChecking = FALSE;
						board[i] = BLANK; // so we don't check it again
						break;
					}
				}
			}
			else if (board[i] == WHITE && whitePiecesLeft > 1) {
				j = 0;
				do {
					if (isConnected(i, chainOfWhites[j])) {
						chainOfWhites[whiteChainLength] = i;
						whiteChainLength++;
						doneChecking = FALSE;
						board[i] = BLANK; // so we don't check it again
						break;
					}
					j++;
				} while (chainOfWhites[j] != -1);
			}
		}
	}

	/*
	   printf("printing chainOfBlacks ... \n");
	   for (i = 0; i < blackPiecesLeft; i++) {
	   printf("| %d ", chainOfBlacks[i]);
	   }
	   printf("\n");
	   printf("printing chainOfWhites ... \n");
	   for (i = 0; i < whitePiecesLeft; i++) {
	   printf("| %d ", chainOfWhites[i]);
	   }
	   printf("\n");
	 */

	allWhiteConnected = TRUE;
	allBlackConnected = TRUE;
	for (i = 0; i < blackPiecesLeft; i++) {
		if (chainOfBlacks[i] == -1) {
			allBlackConnected = FALSE;
			break;
		}
	}
	for (i = 0; i < whitePiecesLeft; i++) {
		if (chainOfWhites[i] == -1) {
			allWhiteConnected = FALSE;
			break;
		}
	}

	//MOVELIST* moves = GenerateMoves(position);
	//if (moves == NULL)
	//  result = tie;
	if (allBlackConnected && !allWhiteConnected)
	{
		if (playerTurn == PLAYERBLACK)
			result = gStandardGame ? win : lose;
		else
			result = gStandardGame ? lose : win;
	}
	else if (allWhiteConnected && !allBlackConnected)
	{
		if (playerTurn == PLAYERBLACK)
			result = gStandardGame ? lose : win;
		else
			result = gStandardGame ? win : lose;
	}
	else if (allWhiteConnected && allBlackConnected)
	{
		if (playerTurn == PLAYERBLACK)
			result = gStandardGame ? lose : win;
		else
			result = gStandardGame ? win : lose;
	}
	else
		result = undecided;

	//if (moves != NULL)
	//SafeFree(moves);
	return result;
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
	int i;
	int j;
	generic_hash_unhash(position, gBoard);
	int playerTurn = generic_hash_turn(position);
	//boardUnhash(position, gBoard);


	printf("\n\n     ");
	for (i = 0; i < gBoardLength; i++) {
		printf("+---");
	}
	printf("+\n");

	for (i = 0; i < gBoardHeight; i++) {
		printf("   %d |", gBoardHeight-i);
		for (j = 0; j < gBoardLength; j++) {
			printf(" %c |", gBoard[gBoardLength*i+j]);
		}
		printf("\n");
		printf("     ");
		for (j = 0; j < gBoardLength; j++) {
			printf("+---");
		}
		printf("+");
		printf("\n");
	}
	printf("      ");
	for (i = 0; i < gBoardLength; i++) {
		printf(" %c  ", 'a'+i);
	}
	printf("\n\n");

	// This part stolen from Asalto.
	printf("   It is %s's turn\n", playersName);
	if (playerTurn == 1)
		printf("   %s is playing X\n", playersName);
	else
		printf("   %s is playing o\n\n", playersName);

	printf("%s\n\n", GetPrediction(position,playersName,usersTurn));

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
	STRING str = MoveToString(computersMove);
	printf("%s's move: %s", computersName, str);
	SafeFree(str);
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
	STRING str = MoveToString(move);
	printf("%s", str);
	SafeFree(str);
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
	STRING moveString = moveUnhash(move);
	return moveString;
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
		printf("%8s's move [(undo)/(<coli><rowi><colf><rowf>)] : ", playersName);

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
	/* input should be like a2c2 */

	if ((input[0] >= 'a' && input[0] < 'a' + gBoardLength) &&
	    (input[1] > '0' && input[1] < '1' + gBoardHeight) &&
	    (input[2] >= 'a' && input[0] < 'a' + gBoardLength) &&
	    (input[3] > '0' && input[3] < '1' + gBoardHeight))
		return TRUE;


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
	return moveHash(input);
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
	int temp;

	while(TRUE) {
		printf("\n");
		printf("\t----- Game Specific Options for Lines of Action -----\n\n");
		printf("\tr) \t(R)esize board. Current board is %dx%d.\n", gBoardLength, gBoardHeight);
		printf("\tb) \t(B)ack to previous menu\n");
		printf("\n\tq) \t(Q)uit\n\n");
		printf("Select an option: ");

		switch(GetMyChar()) {
		case 'r':
		case 'R':
			printf("Input desired board length[3-6]: ");
			temp = GetMyUInt();
			while(temp < 3 || temp > 6) {
				printf("Invalid length.  Please enter a length from 3 to 6.\n");
				temp = GetMyUInt();
			}
			gBoardLength = temp;
			printf("Input desired board height[3-6]: ");
			temp = GetMyUInt();
			while(temp < 3 || temp > 6) {
				printf("Invalid length.  Please enter a length from 3 to 6.\n");
				temp = GetMyUInt();
			}
			gBoardHeight = temp;
			gBoardSize = gBoardLength*gBoardHeight;
			SafeFree(gBoard);
			break;
		case 'q':
		case 'Q':
			printf("\n");
			ExitStageRight();
			break;
		case 'b':
		case 'B':
			return;
		default:
			printf("Sorry, I don't know that option.  Try another.\n");
			break;
		}
	}

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
	return 2;
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

	return (gStandardGame) ? 1 : 2;
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

	if (option == 1)
		gStandardGame = TRUE;
	else if (option == 2)
		gStandardGame = FALSE;
	else
		printf(" Sorry I don't know that option\n");
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
** returns TRUE if boardSquare2 is connected to boardSquare1.  Diagonals
** count as connected. FALSE if they're the same. Called by Primitive
***********************************************************************/
BOOLEAN isConnected(int boardSquare1, int boardSquare2)
{
	Direction d;

	for (d = UP; d <= UPLEFT; d++) {
		if ((!onEdge(d, boardSquare1)) &&
		    (boardSquare1 + goInDirection(d) == boardSquare2))
			return TRUE;
	}

	return FALSE;
}

/************************************************************************
** this function returns TRUE is the piece specified has no other pieces
** of the same color touching it in any direction. diagonals included
***********************************************************************/
BOOLEAN pieceIsolated(int boardSquare)
{
	Direction d;

	for (d = UP; d <= UPLEFT; d++) {
		if ((!onEdge(d, boardSquare)) && (gBoard[boardSquare + goInDirection(d)] == gBoard[boardSquare])) {
			return FALSE;
		}
	}
	return TRUE;
}

/************************************************************************
** adds moves to the movelist given a square on the board that has a
** piece on it. Called by GenerateMoves so assume position has been
** unhashed and is stored correctly in gPosition.
***********************************************************************/
void addPieceMoves(int boardSquare, int playerTurn, MOVELIST **moves)
{
	//printf("starting addPieceMoves...\n");
	Direction d;

	for (d = UP; d <= UPLEFT; d++) {
		//printf(" checking to add move in direction = %d from boardSquare = %d\n", d, boardSquare);
		addMovesInDirection(d, boardSquare, playerTurn, moves);
	}
}

/************************************************************************
** Adds moves to the movelist when given the square that a piece is on
** and the direction you're looking in. Called by addPieceMoves
***********************************************************************/
void addMovesInDirection(Direction direction, int boardSquare, int playerTurn, MOVELIST **moves)
{
	int moveLength, startSquare, endSquare, i;
	char playerColor = (playerTurn == PLAYERBLACK ? BLACK : WHITE);
	char otherPlayerColor = (playerTurn == PLAYERBLACK ? WHITE : BLACK);
	//printf("starting addMovesInDirection...\n");


	moveLength = piecesInLineOfAction(direction, boardSquare);
	startSquare = boardSquare;
	endSquare = boardSquare + moveLength*goInDirection(direction);
	i = 0;

	/* go in that direction one step at a time.  If you hit an edge or a
	   piece of the opposite color, you cannot move in this direction */
	while (i < moveLength && !onEdge(direction, boardSquare))
	{
		//printf("   i = % d, moveLength = %d, \n", i, moveLength);
		boardSquare += goInDirection(direction);
		if (gBoard[boardSquare] != otherPlayerColor)
			i++;
		else
			break;
	}
	if ((boardSquare == endSquare) && (gBoard[endSquare] != playerColor))
	{
		//printf("   adding a piece to moves... direction = %d, startSquare = %d, moveLength = %d \n", direction, startSquare, moveLength);
		*moves = CreateMovelistNode(startSquare*100 + endSquare, *moves);
	}
}

/************************************************************************
** Returns the number of pieces in a specific line of action, which is
** specified by the direction.  Note that it's line of action and not
** direction.  This means this function will return the same thing
** if the direction is RIGHT or LEFT, Up or DOWN, UpLEFT or DOWNRIGHT,
** UPRIGHT or DOWNLEFT.
***********************************************************************/
int piecesInLineOfAction(Direction direction, int boardSquare)
{
	int piecesSoFar = 1;
	int tempBoardSquare = boardSquare;

	/* Go opposite direction until you hit an edge */
	while (!onEdge(oppositeDirection(direction), tempBoardSquare)) {
		tempBoardSquare += goInDirection(oppositeDirection(direction));
		if (gBoard[tempBoardSquare] != BLANK)
			piecesSoFar++;
	}
	/* Go in direction, counting all pieces as you go */
	while (!onEdge(direction, boardSquare)) {
		boardSquare += goInDirection(direction);
		if (gBoard[boardSquare] != BLANK)
			piecesSoFar++;
	}


	return piecesSoFar;
}

/*************************************************************************
** I'm defining an Edge a bit differently than normal.  An edge depends
** on which direction you're looking at.  For example, if you're looking
** right and you're on the top most row, you're still not on an edge.
** You're only on an edge if you cannot go any further in the specified
** direction.  If you're going right, you're only on an edge if you're
** on the rightmost column of the board.
************************************************************************/
BOOLEAN onEdge(Direction direction, int boardSquare)
{
	switch (direction)
	{
	case UP:
		if (boardSquare >= 0 && boardSquare < gBoardLength)
			return TRUE;
		else
			return FALSE;
	case UPRIGHT:
		if ((boardSquare >= 0 && boardSquare < gBoardLength) ||
		    (boardSquare % gBoardLength == gBoardLength-1))
			return TRUE;
		else
			return FALSE;
	case RIGHT:
		if (boardSquare % gBoardLength == gBoardLength-1)
			return TRUE;
		else
			return FALSE;
	case DOWNRIGHT:
		if ((boardSquare % gBoardLength == gBoardLength-1) ||
		    (boardSquare >= gBoardSize-gBoardLength && boardSquare < gBoardSize))
			return TRUE;
		else
			return FALSE;
	case DOWN:
		if (boardSquare >= gBoardSize-gBoardLength && boardSquare < gBoardSize)
			return TRUE;
		else
			return FALSE;
	case DOWNLEFT:
		if ((boardSquare >= gBoardSize-gBoardLength && boardSquare < gBoardSize) ||
		    (boardSquare % gBoardLength == 0))
			return TRUE;
		else
			return FALSE;
	case LEFT:
		if (boardSquare % gBoardLength == 0)
			return TRUE;
		else
			return FALSE;
	case UPLEFT:
		if ((boardSquare >= 0 && boardSquare < gBoardLength) ||
		    (boardSquare % gBoardLength == 0))
			return TRUE;
		else
			return FALSE;
	}

	/* This return should never happen but I put this in to get rid of a warning */
	return FALSE;
}

Direction oppositeDirection(Direction direction)
{
	if (direction == UP)
		return DOWN;
	else if (direction == DOWN)
		return UP;
	else if (direction == RIGHT)
		return LEFT;
	else if (direction == LEFT)
		return RIGHT;
	else if (direction == UPRIGHT)
		return DOWNLEFT;
	else if (direction == DOWNLEFT)
		return UPRIGHT;
	else if (direction == UPLEFT)
		return DOWNRIGHT;
	else //(direction == DOWNRIGHT)
		return UPLEFT;
}


/**************************************************************************
** returns initial position using gBoardSize and gBoardLength.  Called from
** InitializeGame().
*************************************************************************/
POSITION setInitialPosition()
{
	int i;
	POSITION initialPos;
	//char* board = (char*)SafeMalloc(sizeof(char) * (gBoardSize+1));

	for (i = 0; i < gBoardSize; i++) {
		if ((i > 0 && i < gBoardLength-1) || (i < gBoardSize-1 && i > gBoardSize-gBoardLength))
			gBoard[i] = BLACK;
		else if ((i % gBoardLength == 0) || (i % gBoardLength == gBoardLength-1))
			gBoard[i] = WHITE;
		else
			gBoard[i] = BLANK;
	}
	gBoard[0] = BLANK;
	gBoard[gBoardLength-1] = BLANK;
	gBoard[gBoardSize-gBoardLength] = BLANK;
	gBoard[gBoardSize-1] = BLANK;
	gBoard[gBoardSize] = '\0';

	initialPos = generic_hash_hash(gBoard, PLAYERBLACK);
	//initialPos = boardHash(gBoard, PLAYERBLACK);

	//SafeFree(board);
	return initialPos;
}

/***************************************************************************
** Given a position and a color, returns the number of pieces of that same
** color on the board. Only called by Primitive so assume gBoard is correct
**************************************************************************/
int numPiecesLeft(char color)
{
	int i, piecesSoFar;
	piecesSoFar = 0;

	for (i = 0; i < gBoardSize; i++) {
		if (gBoard[i] == color)
			piecesSoFar++;
	}

	return piecesSoFar;
}

/*****************************************************************************
** Takes in a direction and returns an integer that gets added to the
** numerical representation of a square to get a new square which corresponds
** to moving that piece in that direction
****************************************************************************/
int goInDirection(Direction direction)
{

	switch(direction) {
	case UP: return -gBoardLength;
	case UPRIGHT: return -gBoardLength+1;
	case RIGHT: return 1;
	case DOWNRIGHT: return gBoardLength+1;
	case DOWN: return gBoardLength;
	case DOWNLEFT: return gBoardLength-1;
	case LEFT: return -1;
	case UPLEFT: return -gBoardLength-1;
	default: return 0;
	}
}

MOVE moveHash(STRING input)
{
	MOVE move;
	int startSquare, endSquare;

	// input[0] is the letter and input[1] is the number
	startSquare = input[0] - 'a' + (gBoardHeight - (input[1] - '0'))*gBoardLength;
	endSquare = input[2] - 'a' + (gBoardHeight - (input[3] - '0'))*gBoardLength;
	move = endSquare + startSquare*100;
	//printf("startsquare = %d, endSquare = %d\n", startSquare, endSquare);
	return move;
}

STRING moveUnhash(MOVE move)
{
	STRING moveString = (STRING) SafeMalloc(5);
	int startSquare, endSquare;
	char letter, number;

	endSquare = move % 100;
	startSquare = move/100;
	number = '0' + (gBoardHeight - startSquare / gBoardLength);
	letter = (startSquare % gBoardLength) + 'a';
	moveString[0] = letter;
	moveString[1] = number;
	number = '0' + (gBoardHeight - endSquare / gBoardLength);
	letter = (endSquare % gBoardLength) + 'a';
	moveString[2] = letter;
	moveString[3] = number;
	moveString[4] = '\0';

	return moveString;
}


char* boardUnhash(POSITION pos, char* board)
{
	int i;
	pos = (pos - (pos % 3))/3;

	for (i = 0; i < gBoardSize; i++) {
		pos = (pos - (pos % 3))/3;

		if (pos % 3 == 0)
			board[i] = BLANK;
		else if (pos % 3 == 1)
			board[i] = BLACK;
		else if (pos % 3 == 2)
			board[i] = WHITE;
	}
	return board;
}

POSITION boardHash(char* board, int player)
{
	int i;
	POSITION result;

	result = player;

	for (i = 0; i < gBoardSize; i++) {
		if (gBoard[i] == BLACK)
			result += power(3, i+2);
		else if (gBoard[i] == WHITE)
			result += 2*power(3, i+2);
	}
	return result;
}

int boardHash_turn(POSITION pos)
{
	return (int) (pos % 3);
}

// really stupidly done but whatever
POSITION power(POSITION base, int exponent)
{
	int i;
	POSITION result = 1;

	for (i = 0; i < exponent; i++) {
		result *= base;
	}
	return result;
}

/************************************************************************
** Changelog
**
** $Log: not supported by cvs2svn $
** Revision 1.17  2008/05/06 02:59:16  zumbooruk
** mloa.c
**
** Revision 1.16  2008/02/13 02:18:25  alb_shau
** Fixed a bug that would make weird rectangular boards.
**
** BUGZID:
**
** Revision 1.15  2007/11/28 10:47:28  alb_shau
** Fixed things related to option.  Symmetries should not be a separate option.
**
** Revision 1.14  2007/03/01 03:40:27  alb_shau
** Added symmetry stuff via generic symmetries and removed ties
**
** Revision 1.13  2007/02/08 05:23:46  dmchan
** fixed unsigned int call
**
** Revision 1.12  2006/12/19 20:00:51  arabani
** Added Memwatch (memory debugging library) to gamesman. Use 'make memdebug' to compile with Memwatch
**
** Revision 1.11  2006/12/14 06:23:55  alb_shau
** fixed a memory leak
**
** Revision 1.10  2006/12/14 05:35:51  alb_shau
** Made PrintPosition pretty
** changed variants to be 1 based
**
** Revision 1.9  2006/12/04 22:02:00  alb_shau
** last version was not done correctly. Moves became wrong.  Updated the way
** user input is translated into a move to incorporate the board height
**
** Revision 1.8  2006/12/04 19:26:41  alb_shau
** allowed resizing board to a rectangular board.  The "bug" about the computer
** going first was not a bug.  That is what Gamesman does to give people a chance
** to win.  Also, previous version (s) had the wrong date ... about a month
** ahead.
**
** Revision 1.7  2006/11/29 06:43:06  alb_shau
** Added a game specific menu, added ability to resize the board, added help
** strings and misere support
**
** Revision 1.6  2006/10/18 09:09:39  alb_shau
** Fixed some bugs with Primitive, GenerateMoves, and moveHash.  Going to demo
** soon and generic hash wasn't letting me use boards bigger than 4x4 so I wrote
** my own hash and unhash functions.  Unfortunately, when I try to solve, I get
** error messages saying SafeMalloc can't allocate x number of bytes.
**
** Revision 1.5  2006/10/17 10:45:21  max817
** HUGE amount of changes to all generic_hash games, so that they call the
** new versions of the functions.
**
** Revision 1.4  2006/10/16 20:58:42  alb_shau
** Added half a line to make it ready to demo 2006.10.16.  GenerateMoves now
** checks to make sure the move destination is blank before adding that move
** to the list of moves
**
** Revision 1.3  2006/10/14 00:15:33  alb_shau
** finished coding all essential functions.  Debugged Primitive and MoveToString.
**
** Can play human vs. human and solve but solving adds problems.  Basicaly the
** computer moves first but Primitive still acts as if the computer moves second.
**
** Revision 1.2  2006/10/08 05:14:59  alb_shau
** 2006.10.07
** Finished coding almost all of the essential functions.  Still some bugs.
** For example, computer goes first for some reason.
**
** Revision 1.1  2006/09/26 23:01:28  alb_shau
** *** empty log message ***
**
**
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

char * PositionToEndData(POSITION pos) {
	return NULL;
}
