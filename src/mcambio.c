// $Id: mcambio.c,v 1.36 2007-01-26 19:13:33 simontaotw Exp $

/*
 * The above lines will include the name and log of the last person
 * to commit this file to CVS
 */

/************************************************************************
**
** NAME:        mcambio.c
**
** DESCRIPTION: Cambio
** AUTHOR:      Albert Chae and Simon Tao
**
** DATE:        Begin: 2/20/2006 End:
**
** UPDATE HIST: 2/20/2006 - Updated game-specific constants.
**              2/21/2006 - Updated defines and structs, global variables, and InitializeGame(). Corrected CVS log.
**              2/26/2006 - Updated PrintPosition() (Modified PrintPosition() from mtopitop.c).
**              3/02/2006 - Fixed various errors.
**              3/04/2006 - Fixed compile errors.
**              3/05/2006 - Updated Primitive() and added FiveInARow(). Updated tie possible.
**              3/06/2006 - Updated GetInitialPosition().
**	        3/12/2006 - Fixed PrintPosition() seg fault. Fixed GetInitialPosition() polling loop.
**                          Removed blanks from the game.
**              3/14/2006 - Reducing game to 4x4 to see if that fixes a hash problem.
**              3/16/2006 - Changed board printout. Made GetInitialPosition check inputs.
**              3/18/2006 - Added GenerateMove() and PrintMove() and ValidTextInput() and ConvertTextInputToMove().
**                          Also added DoMove(). Some bugs left to work out with involving the alternation of moves etc.
**              3/19/2006 - Changed the Legend so it looks less cramped.
**	        4/16/2006 - Trying 3x3 board to see if it solves
**              4/28/2006 - Added new input format with 3x3 board.
**              5/03/2006 - Fix small bug in DoMove, changed GetAndPrintPlayersMove.
**              5/03/2006 - Changed winning condition in DoMove.
**              5/08/2006 - Fixed some bugs. Game solves.
**              5/09/2006 - Code cleanup.
**              5/09/2006 - Added FourInARow, FiveInARow. Modified PrintPosition, ValidTextInput, ConvertTextInputToMove.
**              5/10/2006 - Added NInARow. Modified PrintPosition, GenerateMove, Primitive, PrintMove, GetAndPrintPlayersMove.
**              5/22/2006 - Added shiftXX functions to accommodate DoMove. Modified DoMove and GameSpecificMenu.
**              5/24/2006 - Modified some hash calls. 5x5 does not work because 3^25 is too big.
**              10/05/2006 - Deleted unnecessary functions. 5x5 player vs. player does not work... too big for hash...
**              12/19/2006 - Added/Debugged Tierfication functions. 3x3 and 4x4 solve with Tierfication. Working on 5x5 and invariants.
**              1/5/2007 - Added variable number of placement variant.
**              1/7/2007 - Fixed original gamesman; changed input so it does not conflict with system inputs.
**		1/26/2007 - Fixed setOption
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

STRING kGameName            = "Cambio";   /* The name of your game */
STRING kAuthorName          = "Albert Chae and Simon Tao";   /* Your name(s) */
STRING kDBName              = "cambio";   /* The name to store the database under */

BOOLEAN kPartizan            = TRUE;   /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN kGameSpecificMenu    = TRUE;   /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN kTieIsPossible       = TRUE;   /* TRUE if a tie is possible. FALSE if it is impossible.*/
BOOLEAN kLoopy               = TRUE;   /* TRUE if the game tree will have cycles (a rearranger style game). FALSE if it does not.*/

BOOLEAN kDebugMenu           = FALSE;   /* TRUE only when debugging. FALSE when on release. */
BOOLEAN kDebugDetermineValue = FALSE;   /* TRUE only when debugging. FALSE when on release. */

POSITION gNumberOfPositions   = 0; /* The number of total possible positions | If you are using our hash, this is given by the hash_init() function*/

POSITION gInitialPosition     = 0; /* The initial hashed position for your starting board */
POSITION kBadPosition         = -1; /* A position that will never be used */

void*    gGameSpecificTclInit = NULL;

/*
 * Help strings that are pretty self-explanatory
 * Strings than span more than one line should have backslashes (\) at the end of the line.
 */

STRING kHelpGraphicInterface =
        "Not written yet";

STRING kHelpTextInterface    =
        "Input the letter or number you wish to move to then press enter.\n\
You may only push off neutral cubes, or cubes of your own symbol.\n\
As the game progresses, you should have more and more of your own\n\
cubes in play.\n"                                                                                                                                                                                                                          ;

STRING kHelpOnYourTurn =
        "BOARD SETUP:\n\
1. The players each select a symbol to be their own, and decide\n\
   who will begin the game.\n\
2. Fill the four corners with neutral symbols.\n\
3. Alternatively, Player A places player B's symbol on the board,\n\
   then player B places player A's symbol on the board.\n\
   The number of initial placement depends on the board size \n\
   (columns - 1) or can be specified in the game options menu.\n\
   These cubes may be placed in any of the positions that a cube\n\
   may occupy when the tray is full (except the four corners).\n\
4. Fill in the rest of the positions with neutral symbols.\n\n\
PLAY:\n\
Each player in turn places a spare cube with his/her own symbol, at\n\
the beginning of any row or column; frees the end cube by lifting\n\
it from the board; then pushes all the pieces in that line along\n\
one place. A player MAY NOT push cubes with your opponents symbol\n\
showing OFF the board.\n"                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    ;

STRING kHelpStandardObjective =
        "To make a line composed of all 3, 4, or 5 (depending on the board size)\n\
of your own symbol - horizontally, vertically or diagonally.\n"                                                                                     ;

STRING kHelpReverseObjective =
        "";

STRING kHelpTieOccursWhen =
        "a player's move causes two 3 in a rows simultaneously.\n";

STRING kHelpExample =
        "";


/*************************************************************************
**
** #defines and structs
**
**************************************************************************/

#define NEUTRAL '+';
#define A_PIECE 'X';
#define B_PIECE 'O';
#define UNKNOWN '?';

typedef enum player {
	playerA = 1, playerB = 2
} Player;


/*************************************************************************
**
** Global Variables
**
*************************************************************************/

int rowcount = 3;
int colcount = 3;
int boardSize;
int initialPlacement = 2;
char neutral = NEUTRAL;
char aPiece = A_PIECE;
char bPiece = B_PIECE;
char unknown = UNKNOWN;

// regular hash pieceArray
int piecesArray3[] = {'+', 0, 9, 'X', 0, 9, 'O', 0, 9, -1 };
int piecesArray4[] = {'+', 0, 16, 'X', 0, 12, 'O', 0, 12, -1 };
int piecesArray5[] = {'+', 0, 25, 'X', 0, 20, 'O', 0, 20, -1 };

/* to convert, use alphaArray[i]
   3x3 example:
   {  0,    1,    2,
   3,    4,    5,
   6,    7,    8 }; */

STRING alphaArray3[] =
{ "a1", "a2", "a3",
  "b1", "b2", "b3",
  "c1", "c2", "c3" };

STRING alphaArray4[] =
{ "a1", "a2", "a3", "a4",
  "b1", "b2", "b3", "b4",
  "c1", "c2", "c3", "c4",
  "d1", "d2", "d3", "d4" };

STRING alphaArray5[] =
{ "a1", "a2", "a3", "a4", "a5",
  "b1", "b2", "b3", "b4", "b5",
  "c1", "c2", "c3", "c4", "c5",
  "d1", "d2", "d3", "d4", "d5",
  "e1", "e2", "e3", "e4", "e5" };

/* to convert, use shiftArray[i-rowcount*colcount]
   3x3 example:
   {   9,  10,  11,
   12,  13,  14,
   15,  16,  17,
   18,  19,  20 }; */

STRING shiftArray3[] =
{ "i", "j", "k",
  "3", "4", "5",
  "v", "w", "x",
  "0", "1", "2" };

STRING shiftArray4[] =
{ "i", "j", "k", "l",
  "4", "5", "6", "7",
  "v", "x", "w", "y",
  "0", "1", "2", "3" };

STRING shiftArray5[] =
{ "i", "j", "k", "l", "m",
  "5", "6", "7", "8", "9",
  "v", "w", "x", "y", "z",
  "0", "1", "2", "3", "4" };

char *gBoard;

/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/

/* External */
extern GENERIC_PTR      SafeSafeMalloc ();
extern void             SafeSafeFree ();
extern POSITION         generic_hash_init(int boardsize, int pieces_array[], int (*vcfg_function_ptr)(int* cfg), int player);
extern POSITION         generic_hash_hash(char *board, int player);
extern char            *generic_hash_unhash(POSITION hash_number, char *empty_board);
extern int              generic_hash_turn (POSITION hashed);
extern void             generic_hash_custom_context_mode(BOOLEAN on);
extern void             generic_hash_set_context(int context);
extern void             generic_hash_context_switch(int context);
/* Internal */
void                    InitializeGame();
MOVELIST               *GenerateMoves(POSITION position);
POSITION                DoMove (POSITION position, MOVE move);
VALUE                   Primitive (POSITION position);
void                    PrintPosition(POSITION position, STRING playersName, BOOLEAN usersTurn);
void                    PrintComputersMove(MOVE computersMove, STRING computersName);
void                    PrintMove(MOVE move);
USERINPUT               GetAndPrintPlayersMove (POSITION position, MOVE *move, STRING playersName);
BOOLEAN                 ValidTextInput(STRING input);
MOVE                    ConvertTextInputToMove(STRING input);
void                    GameSpecificMenu();
void                    SetTclCGameSpecificOptions(int options[]);
POSITION                GetInitialPosition();
int                     NumberOfOptions();
int                     getOption();
void                    setOption(int option);
void                    DebugMenu();
/* Game-specific */
void                    shiftColDown(char *board, char symbol, int pushon, int pushoff);
void                    shiftColUp(char *board, char symbol, int pushon, int pushoff);
void                    shiftRowRight(char *board, char symbol, int pushon, int pushoff);
void                    shiftRowLeft(char *board, char symbol, int pushon, int pushoff);
BOOLEAN                 NInARow(char *board, char symbol, int size);
/* TIER HELPER FUNCTIONS */
void                    SetupTier();
TIER                    BoardToTier(char *board);
POSITION                BoardToPosition(char *board, int turn);
char                   *PositionToBoard(POSITION position);
TIERPOSITION            NumberOfTierPositions(TIER tier);
TIERLIST               *TierChildren(TIER tier);


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
	// clean up
	generic_hash_destroy();

	int i;

	// set boardSize
	boardSize = rowcount*colcount;

	gBoard = (char *) SafeMalloc(boardSize*sizeof(char));

	// switch to custom context
	generic_hash_custom_context_mode(TRUE);

	if(colcount == 3) {
		gNumberOfPositions = generic_hash_init(boardSize, piecesArray3, NULL, 0);
	}
	else if(colcount == 4) {
		gNumberOfPositions = generic_hash_init(boardSize, piecesArray4, NULL, 0);
	}
	else {
		gNumberOfPositions = generic_hash_init(boardSize, piecesArray5, NULL, 0);
	}

	// boardSize^3 to avoid conflict with Tier
	generic_hash_set_context(boardSize*boardSize*boardSize);

	/* filling up the board; with Neutrals */
	for (i = 0; i < boardSize; i++)
		gBoard[i] = neutral;

	gInitialPosition = generic_hash_hash(gBoard, 2);

	SafeFree(gBoard);

	// TIER CODE
	SetupTier();

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
/* must check all the math used for general case, such as 5x5 and on */
{
	MOVELIST *moves = NULL;
	MOVELIST *CreateMovelistNode();

	char opposymbol;
	int turn, i;
	int countA = 0, countB = 0;
	int lefttop = 0, righttop = (colcount-1), leftbottom = (boardSize-colcount), rightbottom = (boardSize-1);

	gBoard = (char *) SafeMalloc(boardSize*sizeof(char));

	gBoard = PositionToBoard(position);

	//generic_hash_unhash(position, gBoard);

	turn = generic_hash_turn(position);

	/* count the number of pieces for each player */
	for(i = 0; i < boardSize; i++)
		if(gBoard[i] == aPiece) countA++;
		else if(gBoard[i] == bPiece) countB++;

	/* Assign pieces for each player */
	if(turn == playerB) {
		opposymbol = aPiece;
	}
	else {
		opposymbol = bPiece;
	}


	/* Phase 1: Less than colcount-1 of PlayerB's and colcount-1 of PlayerA's pieces on the board. */
	if(countB < initialPlacement || countA < initialPlacement)
	{
		//printf("phase 1\n");
		for(i = 0; i < boardSize; i++)
		{
			if((gBoard[i] != aPiece) && (gBoard[i] != bPiece) &&
			   (i != lefttop) && (i != righttop) &&
			   (i != leftbottom) && (i != rightbottom))
			{
				moves = CreateMovelistNode(i, moves);
			}
		}
	}
	/* Phase 2:The main phase of the game. Place your piece at the end of one row and push the piece at the other side off */
	else
	{
		//printf("phase 2\n");
		/* top row moves */
		for(i = 0; i < colcount; i++)
		{
			if(gBoard[i + (boardSize-colcount)] != opposymbol)
				moves = CreateMovelistNode(i + boardSize, moves);
		}
		/* right column moves */
		for(i = 0; i < rowcount; i++)
		{
			if(gBoard[((i+1)*colcount) - colcount] != opposymbol)
				moves = CreateMovelistNode(i + boardSize + colcount, moves);
		}
		/* bottom row moves */
		for(i = 0; i < colcount; i++)
		{
			if(gBoard[(colcount - i - 1)] != opposymbol)
				moves = CreateMovelistNode(i + boardSize + colcount + rowcount, moves);
		}
		/* left column moves */
		for(i = 0; i < rowcount; i++)
		{
			if(gBoard[((colcount - i)*colcount) - 1] != opposymbol)
				moves = CreateMovelistNode(i + boardSize + colcount + rowcount + colcount, moves);
		}
	}

	SafeFree(gBoard);

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
	char symbol, opposymbol;
	int turn;
	int pushon, pushoff;
	POSITION positionAfterMove;

	gBoard = (char *) SafeMalloc(boardSize*sizeof(char));

	gBoard = PositionToBoard(position);

	//generic_hash_unhash(position, gBoard);

	turn = generic_hash_turn(position);

	/* Assign pieces for each player */
	if(turn == playerA) {
		symbol = aPiece;
		opposymbol = bPiece;
	}
	else {
		symbol = bPiece;
		opposymbol = aPiece;
	}


	if(move < boardSize)
	{
		gBoard[move] = opposymbol;
	}
	else
	{
		if (move < boardSize + colcount) //push col down
		{
			pushon = (move - boardSize);
			pushoff = (move - colcount);

			shiftColDown(gBoard, symbol, pushon, pushoff);
		}
		else if (move < boardSize + colcount + rowcount) //push row left
		{
			pushon = (move - boardSize - colcount + 1)*colcount - 1;
			pushoff = (move - boardSize - colcount)*colcount;

			shiftRowLeft(gBoard, symbol, pushon, pushoff);
		}
		else if (move < boardSize + colcount + rowcount + colcount) //push col up
		{
			pushon = boardSize - (move - boardSize - colcount - rowcount + 1);
			pushoff = colcount - (move - boardSize - colcount - rowcount) - 1;

			shiftColUp(gBoard, symbol, pushon, pushoff);
		}
		else if (move < boardSize + colcount + rowcount + colcount + rowcount) //push row right
		{
			pushon = boardSize - (move - boardSize - colcount*2 - rowcount + 1)*colcount;
			pushoff = boardSize - (move - boardSize - colcount*2 - rowcount + 1)*colcount + colcount - 1;

			shiftRowRight(gBoard, symbol, pushon, pushoff);
		}
	}

	if(turn == playerA)
		turn = playerB;
	else
		turn = playerA;

	positionAfterMove = BoardToPosition(gBoard, turn); // generic_hash_hash(gBoard, turn);

	SafeFree(gBoard);

	return positionAfterMove;
}


/************************************************************************
**
** NAME:        Primitive
**
** DESCRIPTION: Returns the value of a position if it fulfills certain
**              'primitive' constraints.
**
**              Example: Tic-tac-toe - Last piece already placed
****asciiascii
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
	int turn = generic_hash_turn(position);
	char symbol, opposymbol;
	BOOLEAN playerWin, oppoWin;

	gBoard = (char *) SafeMalloc(boardSize*sizeof(char));

	gBoard = PositionToBoard(position);

	//generic_hash_unhash(position, gBoard);

	if(turn == playerB) {
		symbol = bPiece;
		opposymbol = aPiece;
	}
	else {
		symbol = aPiece;
		opposymbol = bPiece;
	}

	playerWin = NInARow(gBoard, symbol, colcount);
	oppoWin = NInARow(gBoard, opposymbol, colcount);

	SafeFree(gBoard);

	if (playerWin && oppoWin)
		return tie;
	else if (oppoWin)
		return gStandardGame ? lose : win;
	else if (playerWin)
		return gStandardGame ? win : lose;
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

	char *gBoard = (char *) SafeMalloc(boardSize*sizeof(char));
	int countA = 0, countB = 0, i = 0;

	gBoard = PositionToBoard(position);

	//generic_hash_unhash(position, gBoard);

	/* count the number of pieces for each player */
	for(i = 0; i < boardSize; i++)
		if(gBoard[i] == aPiece) countA++;
		else if(gBoard[i] == bPiece) countB++;


	/* Phase 1: Less than colcount-1 of PlayerB's pieces or less than colcount-1 of PlayerA's pieces on the board. */
	if(countB < initialPlacement || countA < initialPlacement)
	{
		printf("\n");
		if(colcount == 3) {
			printf("One's Symbol = %c       1 2 3 \n", bPiece);
			printf("Two's Symbol = %c      -------\n", aPiece);
		}
		else if(colcount == 4) {
			printf("One's Symbol = %c       1 2 3 4 \n", bPiece);
			printf("Two's Symbol = %c      ---------\n", aPiece);
		}
		else if(colcount == 5) {
			printf("One's Symbol = %c       1 2 3 4 5 \n", bPiece);
			printf("Two's Symbol = %c      -----------\n", aPiece);
		}
		else {
			printf("Error in: PrintPosition\n");
		}

		for(i = 0; i < boardSize; i++)
		{
			//indentation
			if(i == 0)
			{
				printf("     Neutral = %c    %c ", neutral, (i/rowcount)+97);
			}
			if((i != 0) && ((i % rowcount) == 0))
			{
				printf("                    %c ", (i/rowcount)+97);
			}

			if(gBoard[i] == aPiece)
			{
				printf("|%c", aPiece);
			}
			else if(gBoard[i] == bPiece)
			{
				printf("|%c", bPiece);
			}
			else if((gBoard[i] == neutral) &&
			        ((i == 0) || (i == colcount-1) || (i == (rowcount*colcount)-colcount) ||(i == (rowcount*colcount)-1)) )
			{
				printf("|%c", neutral);
			}
			else if(gBoard[i] == neutral)
			{
				printf("| ");
			}
			else
			{
				printf("Error in: PrintPosition Phase 1 Loop\n");
			}

			if((i != 0) && (((i+1) % rowcount) == 0))
			{
				printf("| %c\n", (i/rowcount)+97);
			}
		}

		if(colcount == 3) {
			printf("                      -------\n");
			printf("                       1 2 3 \n");
		}
		else if(colcount == 4) {
			printf("                      ---------\n");
			printf("                       1 2 3 4 \n");
		}
		else if(colcount == 5) {
			printf("                      -----------\n");
			printf("                       1 2 3 4 5 \n");
		}
		else {
			printf("Error in: PrintPosition\n");
		}
		printf("\n");
	}
	/* Phase 2: The main phase of the game. Place your piece at the end of one row and push the piece at the other side off */
	else if(countB >= initialPlacement && countA >= initialPlacement)
	{
		printf("\n");
		if(colcount == 3) {
			printf("One's Symbol = %c       i j k \n", bPiece);
			printf("Two's Symbol = %c      -------\n", aPiece);
		}
		else if(colcount == 4) {
			printf("One's Symbol = %c       i j k l \n", bPiece);
			printf("Two's Symbol = %c      ---------\n", aPiece);
		}
		else if(colcount == 5) {
			printf("One's Symbol = %c       i j k l m \n", bPiece);
			printf("Two's Symbol = %c      -----------\n", aPiece);
		}
		else {
			printf("Error in: PrintPosition\n");
		}

		for(i = 0; i < boardSize; i++)
		{
			//indentation
			if(i == 0)
			{
				printf("     Neutral = %c    %d ",neutral, rowcount-(i/rowcount)-1);
			}
			if((i != 0) && ((i % rowcount) == 0))
			{
				printf("                    %d ", rowcount-(i/rowcount)-1);
			}

			if(gBoard[i] == aPiece)
			{
				printf("|%c", aPiece);
			}
			else if(gBoard[i] == bPiece)
			{
				printf("|%c", bPiece);
			}
			else if(gBoard[i] == neutral)
			{
				printf("|%c", neutral);
			}
			else
			{
				printf("Error in: PrintPosition Phase 2 Loop\n");
			}

			if((i != 0) && (((i+1) % rowcount) == 0))
			{
				printf("| %d\n", (i/rowcount)+rowcount);
			}
		}

		if(colcount == 3) {
			printf("                      -------\n");
			printf("                       x w v \n");
		}
		else if(colcount == 4) {
			printf("                      ---------\n");
			printf("                       y x w v \n");
		}
		else if(colcount == 5) {
			printf("                      -----------\n");
			printf("                       z y x w v \n");
		}
		else {
			printf("Error in: PrintPosition\n");
		}
		printf("\n");
	}
	/* bad else */
	else
	{
		printf("Error in: PrintPosition Phase Conditional\n");
	}


	SafeFree(gBoard);
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
	printf("\t%s's move: ", computersName);
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
/* must check all the math used for general case, such as 5x5 and on */
{
	if(colcount == 3) {
		if(move < boardSize)
			printf("%s", alphaArray3[move]);
		else if (move < boardSize + colcount)
			printf("%s", shiftArray3[move-rowcount*colcount]);
		else if (move < boardSize + colcount + rowcount)
			printf("%s", shiftArray3[move-rowcount*colcount]);
		else if (move < boardSize + colcount + rowcount + colcount)
			printf("%s", shiftArray3[move-rowcount*colcount]);
		else if (move < boardSize + colcount + rowcount + colcount + rowcount)
			printf("%s", shiftArray3[move-rowcount*colcount]);
	}
	else if(colcount == 4) {
		if(move < boardSize)
			printf("%s", alphaArray4[move]);
		else if (move < boardSize + colcount)
			printf("%s", shiftArray4[move-rowcount*colcount]);
		else if (move < boardSize + colcount + rowcount)
			printf("%s", shiftArray4[move-rowcount*colcount]);
		else if (move < boardSize + colcount + rowcount + colcount)
			printf("%s", shiftArray4[move-rowcount*colcount]);
		else if (move < boardSize + colcount + rowcount + colcount + rowcount)
			printf("%s", shiftArray4[move-rowcount*colcount]);
	}
	else if(colcount == 5) {
		if(move < boardSize)
			printf("%s", alphaArray5[move]);
		else if (move < boardSize + colcount)
			printf("%s", shiftArray5[move-rowcount*colcount]);
		else if (move < boardSize + colcount + rowcount)
			printf("%s", shiftArray5[move-rowcount*colcount]);
		else if (move < boardSize + colcount + rowcount + colcount)
			printf("%s", shiftArray5[move-rowcount*colcount]);
		else if (move < boardSize + colcount + rowcount + colcount + rowcount)
			printf("%s", shiftArray5[move-rowcount*colcount]);
	}
	else {
		printf("Error in: PrintMove\n");
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

	char *gBoard = (char *) SafeMalloc(boardSize*sizeof(char));

	gBoard = PositionToBoard(position);

	//generic_hash_unhash(position, gBoard);

	int countA = 0, countB = 0, i = 0, turn;

	turn = generic_hash_turn(position);

	/* count the number of pieces for each player */
	for(i = 0; i < boardSize; i++)
		if(gBoard[i] == aPiece) countA++;
		else if(gBoard[i] == bPiece) countB++;


	/* Phase 1: Less than colcount-1 of PlayerB's or colcount-1 of PlayerA's pieces on the board*/
	if(countB < initialPlacement || countA < initialPlacement)
	{
		for (;; ) {
			/***********************************************************
			* CHANGE THE LINE BELOW TO MATCH YOUR MOVE FORMAT
			***********************************************************/
			printf("%8s's move [(u)ndo/([a-%c][1-%d])] : ", playersName, 'a'+(colcount-1), colcount);

			input = HandleDefaultTextInput(position, move, playersName);

			if (input != Continue)
				return input;
		}
	}
	/* Phase 2: The main phase of the game. Place your piece at the end of one row and push the piece at the other side off */
	else if(countB >= initialPlacement && countA >= initialPlacement)
	{
		for (;; ) {
			/***********************************************************
			* CHANGE THE LINE BELOW TO MATCH YOUR MOVE FORMAT
			***********************************************************/
			printf("%8s's move [(u)ndo/([i-%c or v-%c or 1-%d])] : ", playersName, 'i'+(colcount-1), 'v'+(colcount-1), (colcount*2-1));

			input = HandleDefaultTextInput(position, move, playersName);

			if (input != Continue)
				return input;
		}
	}
	else
	{
		printf("Error in: GetAndPrintPlayersMove\n");
		return Continue;
	}

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
	BOOLEAN flag = TRUE;

	if(strlen(input) > 3)
		flag = FALSE;

	if(!(((input[0] == 'a') && (input[1] == '1')) ||
	     ((input[0] == 'a') && (input[1] == '2')) ||
	     ((input[0] == 'a') && (input[1] == '3')) ||
	     ((input[0] == 'a') && (input[1] == '4')) ||
	     ((input[0] == 'a') && (input[1] == '5')) ||
	     ((input[0] == 'b') && (input[1] == '1')) ||
	     ((input[0] == 'b') && (input[1] == '2')) ||
	     ((input[0] == 'b') && (input[1] == '3')) ||
	     ((input[0] == 'b') && (input[1] == '4')) ||
	     ((input[0] == 'b') && (input[1] == '5')) ||
	     ((input[0] == 'c') && (input[1] == '1')) ||
	     ((input[0] == 'c') && (input[1] == '2')) ||
	     ((input[0] == 'c') && (input[1] == '3')) ||
	     ((input[0] == 'c') && (input[1] == '4')) ||
	     ((input[0] == 'c') && (input[1] == '5')) ||
	     ((input[0] == 'd') && (input[1] == '1')) ||
	     ((input[0] == 'd') && (input[1] == '2')) ||
	     ((input[0] == 'd') && (input[1] == '3')) ||
	     ((input[0] == 'd') && (input[1] == '4')) ||
	     ((input[0] == 'd') && (input[1] == '5')) ||
	     ((input[0] == 'e') && (input[1] == '1')) ||
	     ((input[0] == 'e') && (input[1] == '2')) ||
	     ((input[0] == 'e') && (input[1] == '3')) ||
	     ((input[0] == 'e') && (input[1] == '4')) ||
	     ((input[0] == 'e') && (input[1] == '5')) ||
	     (input[0] == 'i') ||
	     (input[0] == 'j') ||
	     (input[0] == 'k') ||
	     (input[0] == 'l') ||
	     (input[0] == 'm') ||
	     (input[0] == 'v') ||
	     (input[0] == 'w') ||
	     (input[0] == 'x') ||
	     (input[0] == 'y') ||
	     (input[0] == 'z') ||
	     (input[0] == '0') ||
	     (input[0] == '1') ||
	     (input[0] == '2') ||
	     (input[0] == '3') ||
	     (input[0] == '4') ||
	     (input[0] == '5') ||
	     (input[0] == '6') ||
	     (input[0] == '7') ||
	     (input[0] == '8') ||
	     (input[0] == '9') ))
		flag = FALSE;


	return flag;
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
	int move1 = (int)input[0];
	int move2 = (int)input[1];
	int move;

	// placement
	if(move1 == 'a' && move2 >= '1' && move2 <= ('1'+colcount-1))
		move = move2 - '1';
	else if(move1 == 'b' && move2 >= '1' && move2 <= ('1'+colcount-1))
		move = move2 - ('1'-colcount);
	else if(move1 == 'c' && move2 >= '1' && move2 <= ('1'+colcount-1))
		move = move2 - ('1'-colcount*2);
	else if(move1 == 'd' && move2 >= '1' && move2 <= ('1'+colcount-1))
		move = move2 - ('1'-colcount*3);
	else if(move1 == 'e' && move2 >= '1' && move2 <= ('1'+colcount-1))
		move = move2 - ('1'-colcount*4);

	// shifting
	else if(move1 >= 'i' && move1 <= ('i'+colcount-1))
		move = (colcount*rowcount) + (move1-'i');
	else if(move1 >= ('0'+rowcount) && move1 <= (('0'+rowcount)+rowcount-1))
		move = (colcount*rowcount) + (colcount) + (move1-('0'+rowcount));
	else if(move1 >= 'v' && move1 <= ('v'+colcount-1))
		move = (colcount*rowcount) + (colcount+rowcount) + (move1-'v');
	else if(move1 >= '0' && move1 <= ('0'+rowcount-1))
		move = (colcount*rowcount) + (colcount+rowcount+colcount) + (move1-'0');
	else
		printf("Error in: ConvertTextInputToMove");

	return (MOVE) move;
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
	STRING misere;
	do {
		if(gStandardGame) misere = "Off";
		else misere = "On";

		printf("\n\t----- Game-specific options for %s -----\n\n", kGameName);
		printf("\ts)\tChoose the board (s)ize. Currently: %dx%d\n", rowcount, colcount);
		printf("\t\t3x3, 4x4, and 5x5 solve (4x4 and 5x5 take a\n");
		printf("\t\tconsiderable amount of time)\n\n");

		printf("\tp)\tChange the number of (p)ieces placed in the\n");
		printf("\t\tinitial placement phase. Currently: %d\n\n", initialPlacement);

		printf("\tm)\tPlay (m)isere. Currently: %s.\n\n", misere);

		printf("\tb)\t(B)ack = Return to previous activity.\n");
		printf("\n\nSelect an option: ");

		switch(GetMyChar()) {
		case 'q': case 'Q':
			ExitStageRight();
			break;
		case 's': case 'S':
			printf("Enter a size: ");
			temp = GetMyChar(); //get as a char; must convert to int
			rowcount = temp - 48;
			colcount = temp - 48;
			boardSize = rowcount*colcount;
			break;
		case 'p': case 'P':
			do {
				printf("Enter a number: ");
				temp = GetMyChar(); //get as a char; must convert to int
				temp = temp - 48;
				if(temp*2 > boardSize-4) {
					printf("\nThe number you have entered will exceed the\n");
					printf("board size. Please try again.\n\n");
				}
			} while(temp*2 > boardSize-4);
			initialPlacement = temp;
			break;
		case 'm': case 'M':
			printf("Play misere? (y/n): ");
			temp = GetMyChar();
			if(temp == 'y' || temp == 'Y')
				gStandardGame = 0;
			else
				gStandardGame = 1;
			break;
		case 'b': case 'B':
			return;
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
	int turn, i;
	char piece, turnString;

	gBoard = (char*) SafeMalloc(boardSize*sizeof(char));

	getchar(); // for the enter after picking option 1 on the debug menu

	for(i = 0; i < boardSize; i++) {
		do {
			printf("Input the character at cell %i: ", i);
			piece = (char) getchar();
			getchar();
		} while((piece != neutral) && (piece != aPiece) && (piece != bPiece));
		gBoard[i] = piece;
	}


	do {
		printf("Input whose turn it is (1 for player A, 2 for player B): ");
		turnString = (char) getchar();
		getchar();
	} while((turnString != '1') && (turnString != '2'));
	turn = atoi(&turnString);

	if(colcount == 3) {
		gNumberOfPositions = generic_hash_init(boardSize, piecesArray3, NULL, 0);
	}
	else if(colcount == 4) {
		gNumberOfPositions = generic_hash_init(boardSize, piecesArray4, NULL, 0);
	}
	else {
		gNumberOfPositions = generic_hash_init(boardSize, piecesArray5, NULL, 0);
	}

	// boardSize^3 to avoid conflict with Tier
	generic_hash_set_context(boardSize*boardSize*boardSize);

	gInitialPosition = generic_hash_hash(gBoard, turn);

	SafeFree(gBoard);

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
	//boardSize: 3,4,5
	//initial placement: 0,1,2 ; 0,1,2,3,4,5,6 ; 0,1,2,3,4,5,6,7,8,9,10
	//misere: 1,2
	//3*2 + 7*2 + 11*2 = 42
	return 42;
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
	//boardSize^3*(initialPlacement+1)*gStandardGame
	return boardSize*boardSize*boardSize*(initialPlacement+1)*gStandardGame;
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
void shiftColDown(char *board, char symbol, int pushon, int pushoff) {
	int i;
	char temp1, temp2;

	for(i = pushon; i <= pushoff; i+=colcount) {
		if(i == pushon) {
			temp2 = board[i];
			board[i] = symbol;
		}
		else {
			temp1 = board[i];
			board[i] = temp2;
			temp2 = temp1;
		}
	}

}

void shiftColUp(char *board, char symbol, int pushon, int pushoff) {
	int i;
	char temp1, temp2;

	for(i = pushon; i >= pushoff; i-=colcount) {
		if(i == pushon) {
			temp2 = board[i];
			board[i] = symbol;
		}
		else {
			temp1 = board[i];
			board[i] = temp2;
			temp2 = temp1;
		}
	}

}

void shiftRowRight(char *board, char symbol, int pushon, int pushoff) {
	int i;
	char temp1, temp2;

	for(i = pushon; i <= pushoff; i++) {
		if(i == pushon) {
			temp2 = board[i];
			board[i] = symbol;
		}
		else {
			temp1 = board[i];
			board[i] = temp2;
			temp2 = temp1;
		}
	}

}

void shiftRowLeft(char *board, char symbol, int pushon, int pushoff) {
	int i;
	char temp1, temp2;

	for(i = pushon; i >= pushoff; i--) {
		if(i == pushon) {
			temp2 = board[i];
			board[i] = symbol;
		}
		else {
			temp1 = board[i];
			board[i] = temp2;
			temp2 = temp1;
		}
	}

}

BOOLEAN NInARow(char *board, char symbol, int size) {
	BOOLEAN nInARow = FALSE, flag = TRUE;
	int i, j;

	/* horizontal */
	for(i = 0; i < size && !nInARow; i++) {
		for(j = 0; j < size; j++) {
			if(board[i*size+j] != symbol) {
				flag = FALSE;
			}
		}

		if(flag) {
			nInARow = TRUE;
		}

		flag = TRUE;
	}

	/* vertical */
	for(i = 0; i < size && !nInARow; i++) {
		for(j = 0; j < size; j++) {
			if(board[j*size+i] != symbol) {
				flag = FALSE;
			}
		}

		if(flag) {
			nInARow = TRUE;
		}

		flag = TRUE;
	}

	/* diagonal */
	for(i = 0; i < size; i++) {
		if(board[i*size+i] != symbol) {
			flag = FALSE;
		}
	}

	if(flag) {
		nInARow = TRUE;
	}

	// reinitialize flag
	flag = TRUE;

	for(i = 0; i < size; i++) {
		if(board[(size-1)*(i+1)] != symbol) {
			flag = FALSE;
		}
	}

	if(flag) {
		nInARow = TRUE;
	}

	/* result */
	return nInARow;
}

/*************************
* TIER HELPER FUNCTIONS *
*************************/

/* initialization helper*/
void SetupTier() {
	// mark support tier
	kSupportsTierGamesman = TRUE;

	gTierChildrenFunPtr = &TierChildren;
	gNumberOfTierPositionsFunPtr  = &NumberOfTierPositions;

	// get the initial board
	int piecesArrayTier[10] = { aPiece, 0, 0, bPiece, 0, 0, neutral, 0, 0, -1 };
	int i;
	int countA, countB;

	// Tier-Specific Hashes
	// Phase 1
	for(i = 0; i < (rowcount - 1)*2; i++) {
		// X placement
		if(i % 2 == 1) {
			countA = i/2 + 1;
			countB = i/2;
			// Xs
			piecesArrayTier[1] = piecesArrayTier[2] = countA;
			// Os
			piecesArrayTier[4] = piecesArrayTier[5] = countB;
			// Neutrals
			piecesArrayTier[7] = piecesArrayTier[8] = boardSize - countA - countB;
			// initialize hash
			generic_hash_init(boardSize, piecesArrayTier, NULL, 1);
			// set context; tier = # of X's + # of O's*boardSize^2
			generic_hash_set_context(countA + countB*boardSize*boardSize);
		}
		// O placement (includes initial board)
		else {
			countA = i/2;
			countB = i/2;
			// Xs
			piecesArrayTier[1] = piecesArrayTier[2] = countA;
			// Os
			piecesArrayTier[4] = piecesArrayTier[5] = countB;
			// Neutrals
			piecesArrayTier[7] = piecesArrayTier[8] = boardSize - countA - countB;
			// initialize hash
			generic_hash_init(boardSize, piecesArrayTier, NULL, 2);
			// set context; tier = # of X's + # of O's*boardSize^2
			generic_hash_set_context(countA + countB*boardSize*boardSize);
		}
	}

	// Phase 2
	for(countA = (rowcount - 1); countA <= boardSize; countA++) {
		for(countB = (rowcount - 1); countB <= boardSize - countA; countB++) {
			// Xs
			piecesArrayTier[1] = piecesArrayTier[2] = countA;
			// Os
			piecesArrayTier[4] = piecesArrayTier[5] = countB;
			// Neutrals
			piecesArrayTier[7] = piecesArrayTier[8] = boardSize - countA - countB;
			// initialize hash
			generic_hash_init(boardSize, piecesArrayTier, NULL, 0);
			// set context; tier = # of X's + # of O's*boardSize^2
			generic_hash_set_context(countA + countB*boardSize*boardSize);
		}
	}


	// setup for initial board
	gBoard = (char *) SafeMalloc(boardSize*sizeof(char));

	// initialize the initial board
	for(i = 0; i < boardSize; i++)
		gBoard[i] = neutral;

	// set the initial tier
	gInitialTier = BoardToTier(gBoard);

	// set the initial tier position
	gInitialTierPosition = BoardToPosition(gBoard, 1);

	SafeFree(gBoard);
}

/* find tier number of the board */
TIER BoardToTier(char *board) {
	int i;
	int countA = 0, countB = 0;

	// counting the pieces
	for(i = 0; i < boardSize; i++) {
		if(board[i] == aPiece)
			countA++;
		if(board[i] == bPiece)
			countB++;
	}

	// Tier = # of X's + # of O's*boardSize^2
	return countA + countB*boardSize*boardSize;
}

TIERLIST *TierChildren(TIER tier) {
	TIERLIST *tierlist = NULL;
	int i;
	int countA, countB;

	if(tier < (rowcount - 1) + (rowcount - 1)*boardSize*boardSize) {
		for(i = 0; i < (rowcount - 1)*2; i++) {
			// initial tier
			if(i == 0) {
				if(tier == 0) {
					tierlist = CreateTierlistNode(1, tierlist);
					return tierlist;
				}
			}
			// X's
			else if(i % 2 == 1) {
				countA = i/2 + 1;
				countB = i/2;

				if(tier == countA + countB*boardSize*boardSize) {
					tierlist = CreateTierlistNode(tier+boardSize*boardSize, tierlist);
					return tierlist;
				}
			}
			// O's
			else {
				countA = i/2;
				countB = i/2;

				if(tier == countA + countB*boardSize*boardSize) {
					tierlist = CreateTierlistNode(tier+1, tierlist);
					return tierlist;
				}
			}
		}
	}
	// each Tier has three children: itself, one with extra X, and one with extra O
	else {
		for(countA = (rowcount - 1); countA <= boardSize; countA++) {
			for(countB = (rowcount - 1); countB <= boardSize - countA; countB++) {
				if(tier + 1 == countA + countB*boardSize*boardSize ||
				   tier + boardSize*boardSize == countA + countB*boardSize*boardSize) {
					// self loop
					tierlist = CreateTierlistNode(tier, tierlist);
					// putting an X down
					tierlist = CreateTierlistNode(tier+1, tierlist);
					// putting an O down
					tierlist = CreateTierlistNode(tier+boardSize*boardSize, tierlist);
				}
				else if(tier == countA + countB*boardSize*boardSize) {
					// self loop
					tierlist = CreateTierlistNode(tier, tierlist);
				}
			}
		}
	}

	return tierlist;
}

TIERPOSITION NumberOfTierPositions(TIER tier) {
	generic_hash_context_switch(tier);
	return generic_hash_max_pos();
}

/* find position from board */
POSITION BoardToPosition(char *board, int turn) {
	// if we use tier
	if(gHashWindowInitialized) {
		TIER tier = BoardToTier(board);
		generic_hash_context_switch(tier);
		TIERPOSITION tierpos = generic_hash_hash(board, turn);
		return gHashToWindowPosition(tierpos, tier);
	}
	// if we use generic hash
	else {
		generic_hash_context_switch(boardSize*boardSize*boardSize);
		return generic_hash_hash(board, turn);
	}
}

/* find board from position */
char *PositionToBoard(POSITION position) {
	gBoard = (char *) SafeMalloc(boardSize * sizeof(char));

	// if we use tier
	if(gHashWindowInitialized) {
		TIERPOSITION tierpos;
		TIER tier;
		gUnhashToTierPosition(position, &tierpos, &tier);
		generic_hash_context_switch(tier);
		generic_hash_unhash(tierpos, gBoard);
		return (char *) generic_hash_unhash(tierpos, gBoard);
	}
	// if we use generic hash
	else {
		generic_hash_context_switch(boardSize*boardSize*boardSize);
		return (char *) generic_hash_unhash(position, gBoard);
	}
}

// $Log: not supported by cvs2svn $
// Revision 1.35  2007/01/08 02:53:03  simontaotw
// Fixed original gamesman; changed game inputs so they do not conflict with system inputs.
//
// Revision 1.33  2006/12/19 20:23:48  simontaotw
// Added/Debugged Tierfication functions. 3x3 and 4x4 solve with Tierfication. Working on 5x5 and invariants.
//
// Revision 1.31  2006/10/17 10:45:20  max817
// HUGE amount of changes to all generic_hash games, so that they call the
// new versions of the functions.
//
// Revision 1.30  2006/10/06 05:47:13  simontaotw
//
// Deleted unnecessary functions. 5x5 player vs. player does not work...
// too big for hash...
//
// Revision 1.29  2006/05/08 19:09:49  simontaotw
// Code cleanup.
//
// Revision 1.28  2006/05/08 07:15:41  simontaotw
// Fixed some bugs. Game solves.
//
// Revision 1.27  2006/05/04 04:59:29  simontaotw
// Changed winning condition in DoMove.
//
// Revision 1.26  2006/05/03 20:15:50  simontaotw
// Fix small bug in DoMove, changed GetAndPrintPlayersMove.
//
// Revision 1.25  2006/04/29 01:40:56  simontaotw
// Added new input format with 3x3 board.
//
// Revision 1.19  2006/03/19 20:39:15  simontaotw
// Changed the legend so it looks less cramped.
//
// Revision 1.18  2006/03/19 06:21:27  albertchae
// DoMove() works somewhat. Able to play human v human except for a bug
// with alternating player's move and challenger's move.
//
// Revision 1.17  2006/03/18 10:36:35  albertchae
// Wrote the code for GenerateMoves and PrintMoves. Tried to keep
// the logic general in case we have variant board sizes. Still needs some
// testing.
//
// Revision 1.16  2006/03/14 12:38:29  albertchae
// Minor edit. Changed help strings to reflect 4x4. Added some ideas for
// generate move.
//
// Revision 1.15  2006/03/14 12:06:05  albertchae
// Changed game to 4x4 variant. Added test case to Primitive().
//
// Revision 1.14  2006/03/13 06:15:12  albertchae
// Removed blanks as a possible piece. By rewriting the instructions, we can play using only neutrals.
//
// Revision 1.13  2006/03/12 22:16:55  albertchae
// I fixed the PrintPosition() seg fault. generic_hash requires an empty board which we did not have allocated before.
// I also fixed the loop in GetInitialPosition() that asks for input. There is something wrong with hashing.
//
// Revision 1.12  2006/03/12 22:13:02  albertchae
// *** empty log message ***
//
// Revision 1.11  2006/03/08 06:25:41  simontaotw
// Updated GetInitialPosition().
//
// Revision 1.10  2006/03/07 07:59:11  albertchae
// Minor change to the help strings regarding ties.
//
// Revision 1.9  2006/03/06 06:07:10  simontaotw
// Updated tie possible.
//
// Revision 1.8  2006/03/06 01:57:32  simontaotw
// Updated Primitive() and added FiveInARow().
//
// Revision 1.7  2006/03/05 03:28:15  yanpeichen
// Yanpei Chen changing mcambio.c
//
// deleted two stray characters that caused a compiler error.
//
// Revision 1.6  2006/03/04 20:00:54  simontaotw
// Fixed compile errors.
//
// Revision 1.5  2006/03/03 05:19:49  simontaotw
// Fixed various errors.
//
// Revision 1.4  2006/02/26 20:36:53  simontaotw
// Updated PrintPosition() (Modified PrintPosition() in mtopitop.c).
//
// Revision 1.3  2006/02/22 02:54:48  simontaotw
// Updated defines and structs, global variables, and InitializeGame(). Corrected CVS log.
//
// Revision 1.1  2006/02/21 03:17:00  simontaotw
// Updated game-specific constants
//
// Revision 1.7  2006/01/29 09:59:47  ddgarcia
// Removed "gDatabase" reference from comment in InitializeGame
//
// Revision 1.6  2005/12/27 10:57:50  hevanm
// almost eliminated the existance of gDatabase in all files, with some declarations commented earlier that need to be hunt down and deleted from the source file.
//
// Revision 1.5  2005/10/06 03:06:11  hevanm
// Changed kDebugDetermineValue to be FALSE.
//
// Revision 1.4  2005/05/02 17:33:01  nizebulous
// mtemplate.c: Added a comment letting people know to include gSymmetries
//           in their getOption/setOption hash.
// mttc.c: Edited to handle conflicting types.  Created a PLAYER type for
//         gamesman.  mttc.c had a PLAYER type already, so I changed it.
// analysis.c: Changed initialization of option variable in analyze() to -1.
// db.c: Changed check in the getter functions (GetValueOfPosition and
//       getRemoteness) to check if gMenuMode is Evaluated.
// gameplay.c: Removed PlayAgainstComputer and PlayAgainstHuman.  Wrote PlayGame
//             which is a generic version of the two that uses to PLAYER's.
// gameplay.h: Created the necessary structs and types to have PLAYER's, both
//          Human and Computer to be sent in to the PlayGame function.
// gamesman.h: Really don't think I changed anything....
// globals.h: Also don't think I changed anything....both these I just looked at
//            and possibly made some format changes.
// textui.c: Redid the portion of the menu that allows you to choose opponents
//        and then play a game.  Added computer vs. computer play.  Also,
//           changed the analysis part of the menu so that analysis should
//        work properly with symmetries (if it is in getOption/setOption hash).
//
// Revision 1.3  2005/03/10 02:06:47  ogren
// Capitalized CVS keywords, moved Log to the bottom of the file - Elmer
//
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
