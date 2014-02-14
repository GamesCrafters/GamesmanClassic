// $Id: mbaghchal.c,v 1.39 2007-12-07 19:56:04 max817 Exp $

/************************************************************************
**
** NAME:        mbaghchal.c
**
** DESCRIPTION: Bagh Chal (Tigers Move)
**
** AUTHOR:      Damian Hites
**              Max Delgadillo
**              Deepa Mahajan
**
** DATE:        2006.9.27
**
** UPDATE HIST: -2004.10.21 = Original (Dom's) Version
**              -2006.3.2 = Updates + Fixes by Max and Deepa
**                  (Nearly) functional Standard version of game
**              -2006.3.20 = Includes hack version of the Retrograde Solver
**					(FAR from complete, but at least it's a working version)
**					Also a few changes to the game, particular the board display.
**				-2006.4.11 = Retrograde Solver now moved to its own file.
**					Diagonals and Variants now implemented correctly.
**					Fixed ALL the warnings.
**					Fixed a few more little bugs, non 5x5 game is fully complete.
**					Added RetrogradeTierValue function pointer implementation.
**					Added CheckLegality code, but will implement in next update.
**				-2006.4.16 = Slight bug fixes, like reading double-digit input.
**					Variant number calculations fixed.
**					Changed GOATS_MAX to 24 and upper-limit on goats from
**					  "boardsize-tigers-1" to "boardsize-tigers".
**					Now 4x4, No Diagonals, 12 Goats is "Dragons and Swans".
**					Also lowered TIGERS_MAX to 9 to keep it single-digit.
**					Got rid of any final debugging artifacts for the solver.
**				-2006.8.4 = Complete clean-up of the game's code. Now doesn't
**					have awkward semi-state variables like phase1 and NumGoats.
**					Hash and unhash is done a lot more cleanly and intuitively.
**					Tier Gamesman additions are in progress, so they are
**					disabled for now, but will be working by the next update.
**				-2006.8.7 = Tier Gamesman API functions coded! Unfortunately,
**					the UndoMove functions don't seem to work... Hopefully,
**					they'll be fixed soon!
**				-2006.8.8 = UndoMove functions work perfectly! Now the Tier
**					Gamesman API is fully implemented for Bagh Chal. It looks
**					like symmetries are the only thing left to fix.
**				-2006.9.10 = Fixed an unhash bug which returned the wrong
**					person's turn in Stage 2 boards. Now it SHOULD be perfect,
**					excluding IsLegal and Undomove functions of course.
**					The Undomove functions will be fixed once the solver's
**					debugger finally works and can help debug the problem.
**				-2006.9.27 = It turns out Undomove functions were actually
**					fully correct! The solver's debugger will confirm for sure
**					once it's finished. Also, got rid of "Tier0Context" since
**					hash destruction now works.
**
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
#include <string.h>

/*************************************************************************
**
** Game-specific constants
**
**************************************************************************/

STRING kGameName            = "Bagh Chal (Tigers Move)";   /* The name of your game */
STRING kAuthorName          = "Damian Hites, Max Delgadillo, and Deepa Mahajan";   /* Your name(s) */
STRING kDBName              = "baghchal";   /* The name to store the database under */

BOOLEAN kPartizan            = TRUE;  /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN kGameSpecificMenu    = TRUE;  /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN kTieIsPossible       = FALSE;  /* TRUE if a tie is possible. FALSE if it is impossible.*/
BOOLEAN kLoopy               = TRUE;  /* TRUE if the game tree will have cycles (a rearranger style game). FALSE if it does not.*/

BOOLEAN kDebugMenu           = FALSE;  /* TRUE only when debugging. FALSE when on release. */
BOOLEAN kDebugDetermineValue = FALSE;  /* TRUE only when debugging. FALSE when on release. */

POSITION gNumberOfPositions   =  0; /* The number of total possible positions | If you are using our hash, this is given by the hash_init() function*/
POSITION gInitialPosition     =  0; /* The initial hashed position for your starting board */
POSITION kBadPosition         = -1; /* A position that will never be used */


BOOLEAN kSupportsSymmetries = TRUE; /* Whether we support symmetries */
void*    gGameSpecificTclInit = NULL;


/*
 * Help strings that are pretty self-explanatory
 * Strings than span more than one line should have backslashes (\) at the end of the line.
 */

STRING kHelpGraphicInterface =
        "Not written yet";

STRING kHelpTextInterface    =
        "On your move, if you are placing a goat, you enter the\n"
        "coordinates of the space where you would like to place\n"
        "your goat (ex: a3).  If you are moving a goat or a tiger,\n"
        "you enter the coordinates of the piece you would like to\n"
        "move and the coordinates of the space you would like to\n"
        "move the piece to (ex: a3 b3).";

STRING kHelpOnYourTurn =
        "FOR TIGER:\n"
        "Move one tiger one space in any of the directions\n"
        "indicated by the lines.  You may also jump one goat\n"
        "if it is in a space next to you and the space behind\n"
        "it is empty.  Jumping a goat removes it from the board."
        "FOR GOAT, PHASE 1:\n"
        "Place a goat in a space anywhere on the board that is\n"
        "not already occupied."
        "FOR GOAT, PHASE 2:\n"
        "Move one goat one space in any of the directions\n"
        "indicated by the lines.";

STRING kHelpStandardObjective =
        "The objective of the goats is to try and trap the tigers\n"
        "so that they are unable to move.  The tigers are trying to\n"
        "remove every goat from the board.";

STRING kHelpReverseObjective =
        "This is not implemented because it makes the game\n"
        "really really stupid.";

STRING kHelpTieOccursWhen =
        "A tie cannot occur.";

STRING kHelpExample =
        "  1 2 3 4 5\n"
        "a T-+-+-+-T\n"
        "  |\\|/|\\|/|\n"
        "b +-+-+-+-+\n"
        "  |/|\\|/|\\|\n"
        "c +-+-+-+-+\n"
        "  |\\|/|\\|/|\n"
        "d +-+-+-+-+\n"
        "  |/|\\|/|\\|\n"
        "e T-+-+-+-T\n"
        "Dan's Move: c3\n\n"
        "  1 2 3 4 5\n"
        "a T-+-+-+-T\n"
        "  |\\|/|\\|/|\\n"
        "b +-+-+-+-+\n"
        "  |/|\\|/|\\|\n"
        "c +-+-G-+-+\n"
        "  |\\|/|\\|/|\n"
        "d +-+-+-+-+\n"
        "  |/|\\|/|\\|\n"
        "e T-+-+-+-T\n"
        "Computer's Move: e1 d1\n\n"
        "  1 2 3 4 5\n"
        "a T-+-+-+-T\n"
        "  |\\|/|\\|/|\n"
        "b +-+-+-+-+\n"
        "  |/|\\|/|\\|\n"
        "c +-+-G-+-+\n"
        "  |\\|/|\\|/|\n"
        "d T-+-+-+-+\n"
        "  |/|\\|/|\\|\n"
        "e +-+-+-+-T\n"
        "Dan's Move: c1\n\n"
        "  1 2 3 4 5\n"
        "a T-+-+-+-T\n"
        "  |\\|/|\\|/|\n"
        "b +-+-+-+-+\n"
        "  |/|\\|/|\\|\n"
        "c G-+-G-+-+\n"
        "  |\\|/|\\|/|\n"
        "d T-+-+-+-+\n"
        "  |/|\\|/|\\|\n"
        "e +-+-+-+-T\n"
        "Computer's Move: d1 b1\n\n"
        "  1 2 3 4 5\n"
        "a T-+-+-+-T\n"
        "  |\\|/|\\|/|\n"
        "b T-+-+-+-+\n"
        "  |/|\\|/|\\|\n"
        "c +-+-G-+-+\n"
        "  |\\|/|\\|/|\n"
        "d +-+-+-+-+\n"
        "  |/|\\|/|\\|\n"
        "e +-+-+-+-T\n"
        "Dan's Move: d3\n\n"
        "etc...";


/*************************************************************************
**
** #defines and structs
**
**************************************************************************/
#define WIDTH_MAX   5
#define LENGTH_MAX  5
#define WIDTH_MIN   3
#define LENGTH_MIN  3
#define GOATS_MAX   24
#define TIGERS_MAX  9

#define GOAT        'G'
#define TIGER       'T'
#define SPACE       '+'
#define PLAYER_ONE  1
#define PLAYER_TWO  2
#define UP          0
#define DOWN        1
#define UP_RIGHT    2
#define UP_LEFT     3
#define DOWN_RIGHT  4
#define DOWN_LEFT   5
#define RIGHT       6
#define LEFT        7
#define NUMSYMMETRIES 8

/*************************************************************************
**
** Global Variables
**
*************************************************************************/
int width      = 3;
int length     = 3;
int boardSize  = 9;
int tigers     = 4;
int goats      = 4;

BOOLEAN diagonals = TRUE;

POSITION genericHashMaxPos = 0; //saves a function call

int gSymmetryMatrix5[NUMSYMMETRIES][25];
/* This is the array used for flipping along the N-S axis */
int gFlipNewPosition5[] = {4, 3, 2, 1, 0, 9, 8, 7, 6, 5, 14, 13, 12, 11, 10, 19, 18, 17, 16, 15, 24, 23, 22, 21, 20};

/* This is the array used for rotating 90 degrees clockwise */
int gRotate90CWNewPosition5[] = { 20, 15, 10, 5, 0, 21, 16, 11, 6, 1, 22, 17, 12, 7, 2, 23, 18, 13, 8, 3, 24, 19, 14, 9, 4};


int gSymmetryMatrix4[NUMSYMMETRIES][16];
/* This is the array used for flipping along the N-S axis */
int gFlipNewPosition4[] = { 3, 2, 1, 0, 7, 6, 5, 4, 11, 10, 9, 8, 15, 14, 13, 12};

/* This is the array used for rotating 90 degrees clockwise */
int gRotate90CWNewPosition4[] = { 12, 8, 4, 0, 13, 9, 5, 1, 14, 10, 6, 2, 15, 11, 7, 3};


int gSymmetryMatrix3[NUMSYMMETRIES][9];
/* This is the array used for flipping along the N-S axis */
int gFlipNewPosition3[] = {2, 1, 0, 5, 4, 3, 8, 7, 6};

/* This is the array used for rotating 90 degrees clockwise */
int gRotate90CWNewPosition3[] = {6, 3, 0, 7, 4, 1, 8, 5, 2 };

/* Proofs of correctness for the below arrays:
**
** FLIP
**
** 1  2  3  4  5        5  4  3  2  1
** 6  7  8  9  10  ->   10 9  8  7  6
** 11 12 13 14 15       15 14 13 12 11
** 16 17 18 19 20       20 19 18 17 16
** 21 22 23 24 25       25 24 23 22 21
**
** ROTATE
**
** 1  2  3  4  5        21 16 11 6  1
** 6  7  8  9  10  ->   22 17 12 7  2
** 11 12 13 14 15       23 18 13 8  3
** 16 17 18 19 20       24 19 14 9  4
** 21 22 23 24 25       25 20 15 10 5
*/

/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/
int translate (int x, int y);
int get_x (int index);
int get_y (int index);
POSITION hash (char*, int, int);
char* TclUnhash(POSITION);
char* unhash (POSITION, int*, int*);
void ChangeBoardSize ();
void SetupGame ();
//BOOLEAN CheckLegality (POSITION position);
STRING MoveToString(MOVE);
POSITION GetCanonicalPosition(POSITION position);
POSITION DoSymmetry(POSITION position, int symmetry);

//TIER GAMESMAN
void SetupTierStuff();
TIERLIST* TierChildren(TIER);
TIERPOSITION NumberOfTierPositions(TIER);
BOOLEAN IsLegal(POSITION);
UNDOMOVELIST* GenerateUndoMovesToTier(POSITION, TIER);
POSITION UnDoMove(POSITION, UNDOMOVE);
STRING TierToString(TIER);

void unhashTier(TIER, int*, int*, int*);
TIER hashTier(int, int, int);
int numGoatsOnBoard (char*);
int s1GoatOffset, s1TigerOffset;
// Actual functions are at the end of this file

/* External */
#ifndef MEMWATCH
extern GENERIC_PTR  SafeMalloc ();
extern void     SafeFree ();
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

int vcfg_board(int* configuration) {
	return 1;
}

STRING copy(STRING toCopy){
	STRING toReturn = SafeMalloc((strlen(toCopy)+1) * sizeof(char));
	strcpy(toReturn, toCopy);
	return toReturn;
}

void *fakeUnhash(POSITION p){
	return (void *) copy("balhblahblah");
}

void InitializeGame ()
{
	gCanonicalPosition = GetCanonicalPosition;

	gMoveToStringFunPtr = &MoveToString;
	gCustomUnhash = &TclUnhash;
	linearUnhash = &fakeUnhash;
	SetupGame();
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

MOVELIST *GenerateMoves    (POSITION position)
{ /*      UP
	          ^
	LEFT<---|--> RIGHT
	          v
	         DOWN
	(1,1) IS top left corner and (row,col)=(length, width)= bottom right corner
	 */
	int turn, goatsLeft;
	char* board = unhash(position, &turn, &goatsLeft);
	char animal;
	int row = length, col = width;
	MOVELIST *moves = NULL;
	int i, j;
	if(goatsLeft != 0 && turn == PLAYER_ONE)    {
		// Generates the list of all possible drop locations for Phase 1
		for(i = 1; i <= row; i++) {
			for(j = 1; j <= col; j++)    {
				if(board[translate(i, j)] == SPACE)
					moves = CreateMovelistNode(translate(i, j), moves);
			}
		}
	}
	else {
		// Generates the list of all possible jump/ move locations for Phase 2
		if(PLAYER_ONE == turn)
			animal = GOAT;
		else
			animal = TIGER;

		for(i = 1; i <= row; i++) {
			for(j = 1; j <= col; j++)    {
				if(board[translate(i, j)] == animal) {
					// Move Up
					if((i-1 > 0) && (board[translate(i-1, j)] == SPACE))
						moves = CreateMovelistNode((translate(i, j)*8+UP)*2 + boardSize, moves);
					// Move Right
					if((j+1 <= col) && (board[translate(i, j+1)] == SPACE))
						moves = CreateMovelistNode((translate(i, j)*8+RIGHT)*2 + boardSize, moves);
					// Move Down
					if((i+1 <= row) && (board[translate(i+1, j)] == SPACE))
						moves = CreateMovelistNode((translate(i, j)*8+DOWN)*2 + boardSize, moves);
					// Move Left
					if((j-1 > 0) && (board[translate(i, j-1)] == SPACE))
						moves = CreateMovelistNode((translate(i, j)*8+LEFT)*2 + boardSize, moves);
					//DIAGONAL MOVES
					if(diagonals) { // Move NW
						if((i-1 > 0) && (j-1 > 0) && (((i + j) % 2) == 0)    &&
						   (board[translate(i-1, j-1)] == SPACE))
							moves = CreateMovelistNode((translate(i, j)*8+UP_LEFT)*2 + boardSize, moves);
						// Move NE
						if((i-1 > 0) && (j+1 <= col) && (((i + j) % 2) == 0) &&
						   (board[translate(i-1, j+1)] == SPACE))
							moves = CreateMovelistNode((translate(i, j)*8+UP_RIGHT)*2 + boardSize,moves);
						// Move SE
						if((i+1 <= row) && (j+1 <= col) && (((i + j) % 2) == 0) &&
						   (board[translate(i+1, j+1)] == SPACE))
							moves = CreateMovelistNode((translate(i, j)*8+DOWN_RIGHT)*2 + boardSize, moves);
						// Move SW
						if((i+1 <= row) && (j-1 > 0) && (((i + j) % 2) == 0) &&
						   (board[translate(i+1, j-1)] == SPACE))
							moves = CreateMovelistNode((translate(i, j)*8+DOWN_LEFT)*2 + boardSize, moves);
					}
					if(animal == TIGER)    {
						// tigers can jump
						// Jump Up
						if((i-1 > 1) && (board[translate(i-1, j)] == GOAT) &&
						   (board[translate(i-2, j)] == SPACE))
							moves = CreateMovelistNode((translate(i, j)*8+UP)*2+1 + boardSize, moves);
						// Jump Right
						if((j+1 < width) && (board[translate(i, j+1)] == GOAT)    &&
						   (board[translate(i, j+2)] == SPACE))
							moves = CreateMovelistNode((translate(i, j)*8+RIGHT)*2+1 + boardSize, moves);
						// Jump Down
						if((i+1 < row) && (board[translate(i+1, j)] == GOAT) &&
						   (board[translate(i+2, j)] == SPACE))
							moves = CreateMovelistNode((translate(i, j)*8+DOWN)*2+1 + boardSize, moves);
						// Jump Left
						if((j-1 > 1) && (board[translate(i, j-1)] == GOAT) &&
						   (j-2 > 0) && (board[translate(i, j-2)] == SPACE))
							moves = CreateMovelistNode((translate(i, j)*8+LEFT)*2+1 + boardSize, moves);
						if(diagonals) { // Jump SW
							if((i+1 < length) && (j-1 > 1)    &&
							   (board[translate(i+1, j-1)] == GOAT)    &&
							   (((i + j) % 2) == 0)    &&
							   (board[translate(i+2, j-2)] == SPACE))
								moves = CreateMovelistNode((translate(i, j)*8+DOWN_LEFT)*2+1 + boardSize, moves);
							// Jump SE
							if((i+1 < length) && (j+1 < width) &&
							   (board[translate(i+1, j+1)] == GOAT)    &&
							   (((i + j) % 2) == 0)    &&
							   (board[translate(i+2, j+2)] == SPACE))
								moves = CreateMovelistNode((translate(i, j)*8+DOWN_RIGHT)*2+1 + boardSize, moves);
							// Jump NE
							if((i-1 > 1) && (j+1 < width) &&
							   (board[translate(i-1, j+1)] == GOAT)&& (((i+j) %2) == 0) &&
							   (board[translate(i-2, j+2)] == SPACE))
								moves = CreateMovelistNode((translate(i, j)*8+UP_RIGHT)*2+1 + boardSize, moves);
							// Jump NW
							if((i-1 > 1) && (j-1 > 1) &&
							   (board[translate(i-1, j-1)] == GOAT) &&
							   (i-2 >0) && (j-2 >0) && (((i + j) % 2) == 0) && (board[translate(i-2, j-2)] ==  SPACE))
								moves = CreateMovelistNode((translate(i, j)*8+UP_LEFT)*2+1 + boardSize, moves);
						}
					}
				}
			}
		}
	}
	if (board != NULL)
		SafeFree(board);
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
	int turn, goatsLeft;
	char* board = unhash(position, &turn, &goatsLeft);
	char piece;
	int jump, direction, i, j, jumpI, jumpJ;
	if(move < boardSize) { // It's only a Goat being placed
		board[move] = GOAT;
		turn = PLAYER_TWO;
		goatsLeft -= 1;
		return hash(board, turn, goatsLeft);
	}
	move -= boardSize;
	jump = move % 2;
	move /= 2;
	direction = move % 8;
	move /= 8;
	i = jumpI = get_x(move);
	j = jumpJ = get_y(move);
	piece = board[move];
	board[move] = SPACE; // clear the current location of the piece
	switch(direction) { // find the appropriate modifiers to i and j
	case UP: if (jump) { i -= 2; jumpI -= 1; }
		else i -= 1; break;
	case DOWN: if (jump) { i += 2; jumpI += 1; }
		else i += 1; break;
	case RIGHT: if (jump) { j += 2; jumpJ += 1; }
		else j += 1; break;
	case LEFT: if (jump) { j -= 2; jumpJ -= 1; }
		else j -= 1; break;
		if (diagonals) {
		case UP_RIGHT: if (jump) { i -= 2; j += 2; jumpI -= 1; jumpJ += 1; }
			else { i -= 1; j += 1; } break;
		case UP_LEFT: if (jump) { i -= 2; j -= 2; jumpI -= 1; jumpJ -= 1; }
			else { i -= 1; j -= 1; } break;
		case DOWN_RIGHT: if (jump) { i += 2; j += 2; jumpI += 1; jumpJ += 1; }
			else { i += 1; j += 1; } break;
		case DOWN_LEFT: if (jump) { i += 2; j -= 2; jumpI += 1; jumpJ -= 1; }
			else { i += 1; j -= 1; } break;
		}
	}
	board[translate(i, j)] = piece; // place the piece in its new location
	board[translate(jumpI, jumpJ)] = SPACE; // erase the piece jumped over
	turn = (turn == PLAYER_ONE ? PLAYER_TWO : PLAYER_ONE);
	//goatsLeft stays the same
	return hash(board, turn, goatsLeft);
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
	MOVELIST* moves = GenerateMoves(position);
	if(NULL == moves) // no legal moves left == lose
		return lose;
	FreeMoveList(moves);
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
	int i, j;
	int turn, goatsLeft;
	char* board = unhash(position, &turn, &goatsLeft);
	printf("\t%s's Turn (%s):\n  ",playersName,(turn==PLAYER_ONE ? "Goat" : "Tiger"));
	for(i = 1; i <= length+1; i++) { // print the rows one by one
		if(i <= length) {
			printf("\t%d ", length-i+1); // first, print the row with the pieces
			for(j = 1; j <= width; j++) {
				printf("%c", board[translate(i, j)]);
				if(j < width)
					printf("-");
			}
			printf("\n\t  ");
			if(i < length) { // then, print the row with the lines (diagonals and such)
				for(j = 1; j < width; j++) {
					if(diagonals) {
						if(j % 2 && i % 2)
							printf("|\\");
						else if(j % 2 && ((i % 2) == 0))
							printf("|/");
						else if(i % 2)
							printf("|/");
						else
							printf("|\\");
					}
					else printf("| ");

					if(j == width - 1)
						printf("|");
				}
			}
			if(i == 1) {
				if(goatsLeft != 0) printf("  <STAGE 1> Goats left to be placed: %d", goatsLeft);
				else printf("  <STAGE 2>");
			} else if (i == 2)
				printf("    %s", GetPrediction(position, playersName, usersTurn));
			if(i<length) printf("\n");
		}
		else if(i > length) {
			for(j = 1; j <= width; j++) // print the column numbers
				printf("%c ", j+'a'-1);
		}
	}
	printf("\n\n");
	if (board != NULL)
		SafeFree(board);
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
	printf("%8s's move : ", computersName);
	PrintMove(computersMove);
	printf("\n\n");
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
	STRING s = MoveToString( move );
	printf( "%s", s );
	SafeFree(s);
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
		if(position >= genericHashMaxPos) //stage1
			printf("%8s's move [(undo)/([%c-%c][%d-%d])] : ", playersName, 'a', length+'a'-1, 1, width);
		else
			printf("%8s's move [(undo)/([%c-%c][%d-%d] [%c-%c][%d-%d])] : ", playersName, 'a', length+'a'-1, 1, width, 'a', length+'a', 1, width);
		input = HandleDefaultTextInput(position, move, playersName);

		if (input != Continue)
			return input;
	}

	/* NOTREACHED */
	return Continue;
}


/***v*********************************************************************
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
****h
** OUTPUTS:     BOOLEAN      : TRUE if the input is a valid text input.
**
************************************************************************/

BOOLEAN ValidTextInput (STRING input)
{
	int size = strlen(input);
	if(size != 2 && size != 4)
		return FALSE;
	if (!isalpha(input[0]) || !isdigit(input[1]) ||
	    (size == 4 && (!isalpha(input[2]) || !isdigit(input[3]))))
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
	int x, y, x1, y1;
	MOVE move;
	y = input[0] - 'a' + 1;
	x = length - input[1] + '1';
	if(input[2] == '\0') {
		move = translate(x, y);
		return move;
	}
	y1 = input[2] - 'a' + 1;
	x1 = length - input[3] + '1';
	if(x1 == x+2) {
		if(y1 == y+2 && diagonals)
			return (MOVE) (translate(x, y)*8+DOWN_RIGHT)*2+1 + boardSize;
		else if(y1 == y)
			return (MOVE) (translate(x, y)*8+DOWN)*2+1 + boardSize;
		else if(diagonals)
			return (MOVE) (translate(x, y)*8+DOWN_LEFT)*2+1 + boardSize;
		else return 0;
	} else if(x1 == x+1) {
		if(y1 == y+1 && diagonals)
			return (MOVE) (translate(x, y)*8+DOWN_RIGHT)*2 + boardSize;
		else if(y1 == y)
			return (MOVE) (translate(x, y)*8+DOWN)*2 + boardSize;
		else if(diagonals)
			return (MOVE) (translate(x, y)*8+DOWN_LEFT)*2 + boardSize;
		else
			return 0;
	} else if(x1 == x) {
		if(y1 == y+2)
			return (MOVE) (translate(x, y)*8+RIGHT)*2+1 + boardSize;
		else if(y1 == y+1)
			return (MOVE) (translate(x, y)*8+RIGHT)*2 + boardSize;
		else if(y1 == y-1)
			return (MOVE) (translate(x, y)*8+LEFT)*2 + boardSize;
		else
			return (MOVE) (translate(x, y)*8+LEFT)*2+1 + boardSize;
	} else if(x1 == x-1) {
		if(y1 == y+1 && diagonals)
			return (MOVE) (translate(x, y)*8+UP_RIGHT)*2 + boardSize;
		else if(y1 == y)
			return (MOVE) (translate(x, y)*8+UP)*2 + boardSize;
		else if (diagonals)
			return (MOVE) (translate(x, y)*8+UP_LEFT)*2 + boardSize;
		else return 0;
	} else {
		if(y1 == y+2 && diagonals)
			return (MOVE) (translate(x, y)*8+UP_RIGHT)*2+1 + boardSize;
		else if(y1 == y)
			return (MOVE) (translate(x, y)*8+UP)*2+1 + boardSize;
		else if(diagonals)
			return (MOVE) (translate(x, y)*8+UP_LEFT)*2+1 + boardSize;
		else return 0;
	}
	return 0;
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
	char c;
	BOOLEAN cont = TRUE;
	while(cont) {
		printf("\n\nCurrent %dx%d board with %d goats:  \n", width, length, goats);
		PrintPosition(gInitialPosition, "Gamesman", 0);
		printf("\tGame Options:\n\n"
		       "\t1)\t(1) Load default 3x3 Game (4 Goats, Diagonals)\n"
		       "\t2)\t(2) Load default 4x4 Game (12 Goats. No Diagonals)\n"
		       "\t3)\t(3) Load default 5x5 Game (20 Goats, Diagonals)\n"
		       "\tc)\t(C)hange the board size (nxn), currently: %d\n"
		       "\td)\tTurn (D)iagonals %s\n"
		       "\ti)\tSet the (I)nitial position (starting position)\n"
		       "\tb)\t(B)ack to the main menu\n"
		       "\nSelect an option:  ", width, diagonals ? "off" : "on");
		c = GetMyChar();
		switch(c) {
		case '1':
			width = length = 3; boardSize  = width*length;
			tigers = 4; goats = 4; diagonals = TRUE;
			SetupGame();
			break;
		case '2':
			width = length = 4; boardSize  = width*length;
			tigers = 4; goats = 12; diagonals = FALSE;
			SetupGame();
			break;
		case '3':
			width = length = 5; boardSize  = width*length;
			tigers = 4; goats = 20; diagonals = TRUE;
			SetupGame();
			break;
		case 'c': case 'C':
			ChangeBoardSize();
			break;
		case 'd': case 'D':
			diagonals= !diagonals;
			break;
		case 'i': case 'I':
			GetInitialPosition();
			break;
		case 'b': case 'B':
			cont = FALSE;
			break;
		default:
			printf("Invalid option!\n");
		}
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

void SetTclCGameSpecificOptions (int options[]) {
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
	int i, j;
	int turn, goatsLeft;
	char* board = (char*) SafeMalloc(boardSize * sizeof(char));
	char line[width], in[2], first;
	for(i = 0; i < width; i++)
		line[i] = SPACE;
	BOOLEAN valid = FALSE;
	while(!valid) {
		valid = TRUE;
		printf("\nCurrent Position: \n");
		PrintPosition(gInitialPosition, "Fred", 0);
		printf("Enter a character string to represent the position you want\n"
		       "%c for tiger, %c for goat, %c for a space.  Example:\n"
		       "For the initial position of a 5X5 board, the string is:\n"
		       "%c%c%c%c%c\n"
		       "%c%c%c%c%c\n"
		       "%c%c%c%c%c\n"
		       "%c%c%c%c%c\n"
		       "%c%c%c%c%c\n\n"
		       "Enter the character string: \n",TIGER,GOAT,SPACE,TIGER,SPACE,SPACE,SPACE,TIGER,
		       SPACE,SPACE,SPACE,SPACE,SPACE,SPACE,SPACE,SPACE,SPACE,SPACE,SPACE,SPACE,SPACE,SPACE,SPACE,
		       TIGER,SPACE,SPACE,SPACE,TIGER);
		for(i = 1; i <= length; i++) {
			printf(">");
			scanf("%s", line);
			for(j = 1; j <= width; j++) {
				if(line[j-1] != SPACE && line[j-1] != GOAT && line[j-1] != TIGER)
					valid = FALSE;
				board[translate(i,j)] = line[j-1];
			}
		}
		printf("Enter how many goats are left to place: ");
		scanf("%s", in);
		if (strlen(in) == 1) {
			goatsLeft = in[0]-48;
		} else goatsLeft = ((in[0]-48)*10)+(in[1]-48);
		printf("Enter who you would like to go first: (g)oats or (t)igers: ");
		first = GetMyChar();
		if(first == 'g' || first == 'G')
			turn = PLAYER_ONE;
		else if(first == 't' || first == 'G')
			turn = PLAYER_TWO;
		else valid = FALSE;
		goats = 0;
		tigers = 0;
		for(i = 0; i < boardSize; i++) {
			if(board[i] == GOAT)
				goats++;
			else if(board[i] == TIGER)
				tigers++;
		}
		if (goats > GOATS_MAX || tigers > TIGERS_MAX || goats+goatsLeft > boardSize-tigers)
			valid = FALSE;
		goats += goatsLeft;
		if(!valid)
			printf("\n\nInvalid board!!!\n\n");
	}
	SetupGame();
	gInitialPosition = hash(board, turn, goatsLeft);
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
	return 2591; //1 + 2( 1 + (2 * ( 2*9*24 + 8*24 +23)))
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
	int option = 0;
	option += ((length-3)*TIGERS_MAX*GOATS_MAX + (tigers-1)*GOATS_MAX + (goats-1)) *2;
	if (diagonals == FALSE)
	{
		option +=1;
	}
	option *= 2;
	option += gSymmetries;
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
	gSymmetries = option %2;
	option /= 2;
	if(option%2==1) {
		diagonals = FALSE;
		option-=1;
	}
	else
		diagonals = TRUE;
	option /= 2;
	width = length = (option / (TIGERS_MAX * GOATS_MAX))+3;
	boardSize = width*length;
	tigers = ((option % (TIGERS_MAX * GOATS_MAX)) / GOATS_MAX)+1;
	goats = ((option % (TIGERS_MAX * GOATS_MAX)) % GOATS_MAX)+1;
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

void DebugMenu () {
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

int translate (int x, int y)
{
	return (x-1)*width+(y-1);
}

int get_x (int index)
{
	return index / length + 1;
}

int get_y (int index)
{
	return index % length + 1;
}

POSITION hash (char* board, int turn, int goatsLeft)
{
	POSITION position;
	if (gHashWindowInitialized) {
		int goatsOnBoard = numGoatsOnBoard(board);
		TIER tier = hashTier(goatsOnBoard, goatsLeft, turn); // find this board's tier
		generic_hash_context_switch(goatsOnBoard); // switch to that context
		TIERPOSITION tierpos;
		if (tier < s1GoatOffset) //stage 2
			tierpos = generic_hash_hash(board, turn); //hash normally
		else tierpos = generic_hash_hash(board, 1); //else hash only one player
		position = gHashToWindowPosition(tierpos, tier); //gets TIERPOS, find POS
	} else {
		position = generic_hash_hash(board, turn);
		position += (genericHashMaxPos * goatsLeft);
	}
	if (board != NULL)
		SafeFree(board);
	return position;
}

char* TclUnhash(POSITION position) {
	char* TclBoard = (char*) SafeMalloc((boardSize+3) * sizeof(char));
	int turn, goatsLeft;
	char* board = unhash(position, &turn, &goatsLeft);

	TclBoard[0] = turn + 48;
	TclBoard[1] = ((goatsLeft /= 10) % 10) + 48;
	TclBoard[2] = (goatsLeft % 10) + 48;
	int i;
	for (i = 0; i < boardSize; i++)
		TclBoard[i+3] = board[i];
	if (board != NULL)
		SafeFree(board);
	return TclBoard;
}

char* unhash (POSITION position, int* turn, int* goatsLeft)
{
	char* board = (char*) SafeMalloc(boardSize * sizeof(char));
	if(gHashWindowInitialized) {
		TIERPOSITION tierpos; TIER tier;
		gUnhashToTierPosition(position, &tierpos, &tier); // get tierpos
		int goatsOnBoard;
		unhashTier(tier, &goatsOnBoard, goatsLeft, turn); // this sets goatsLeft and turn
		generic_hash_context_switch(goatsOnBoard); // switch to that tier's context
		if (tier < s1GoatOffset) // stage 2
			(*turn) = generic_hash_turn(tierpos); // hash tells the turn
		generic_hash_unhash(tierpos, board); // unhash in that tier
	} else {
		(*goatsLeft) = position / genericHashMaxPos;
		position %= genericHashMaxPos;
		(*turn) = generic_hash_turn(position);
		generic_hash_unhash(position, board);
	}
	return board;
}

void ChangeBoardSize ()
{
	int change;
	BOOLEAN cont = TRUE;
	while (cont) {
		cont = FALSE;
		printf("\n\nCurrent board of size %d:\n\n", width);
		PrintPosition(gInitialPosition, "Fred", 0);
		printf("\n\nEnter the new board size (%d - %d):  ", WIDTH_MIN, WIDTH_MAX);
		change = GetMyChar()-48;
		if(change > WIDTH_MAX || change < WIDTH_MIN) {
			printf("\nInvalid base length!\n");
			cont = TRUE;
		}
		else {
			width = length = change;
			boardSize = width*length;
			tigers = 4;
			goats = boardSize-tigers-1;
			SetupGame();
		}
	}
}


void SetupGame ()
{
	// destroy current hash
	generic_hash_destroy();
	// setup the tier hashes
	SetupTierStuff();
	// The GLOBAL Hash:
	int game[10] = {TIGER, tigers, tigers, GOAT, 0, goats, SPACE, boardSize - tigers - goats, boardSize - tigers, -1};
	genericHashMaxPos = generic_hash_init(boardSize, game, vcfg_board, 0);
	gNumberOfPositions = genericHashMaxPos * (goats + 1);
	// Set Initial Position
	char* board = (char*) SafeMalloc(boardSize * sizeof(char));
	char* board2 = (char*) SafeMalloc(boardSize * sizeof(char));
	int i;
	for(i = 0; i < boardSize; i++)
		board[i] = SPACE;
	board[translate(1, 1)] = TIGER;
	board[translate(1, width)] = TIGER;
	board[translate(length, 1)] = TIGER;
	board[translate(length, width)] = TIGER;
	for (i = 0; i < boardSize; i++)
		board2[i] = board[i];
	int turn = 1;
	int goatsLeft = goats;
	gInitialPosition = hash(board, turn, goatsLeft);
	// for gInitialTierPosition
	int globalHashContext = generic_hash_cur_context();
	generic_hash_context_switch(0); //switch to hash with 0 goats
	goatsLeft = 0;
	gInitialTierPosition = hash(board2, turn, goatsLeft);
	//gInitialTier set in SetupTierStuff();
	generic_hash_context_switch(globalHashContext);
}

/*
   BOOLEAN CheckLegality (POSITION position) {
        char* board = unhash(position);
        int goatsOnBoard = 0, i, j;
        UNDOMOVELIST* moves;
        //determines the number of goats on the board
        for (i = 1; i <= length; i++){
                for (j = 1; j <= width; j++){
                        if (board[translate(i,j)] == GOAT) goatsOnBoard++;
                }
        }
        if (phase1 == TRUE && (goatsOnBoard + NumGoats) != goats)
                return FALSE;
        moves = GenerateUndoMovesToTier(position, 0);
        if (moves == NULL)
                return FALSE;
        else return TRUE;
   }
 */

STRING MoveToString(move)
MOVE move;
{
	int jump, direction, i, j, x, y;
	STRING moveStr = (STRING) SafeMalloc(sizeof(char)*4);
	if(move < boardSize) { // This is just a goat being placed
	  i = get_x(move);
		j = get_y(move);
		sprintf(moveStr, "%c%c%c%c", ' ',' ',j + 'a' - 1, '0' +length - i + 1);
		return moveStr;
	}
	else {
		move -= boardSize;
		jump = move % 2;
		move /= 2;
		direction = move % 8;
		move /= 8;
		i = x = get_x(move);
		j = y = get_y(move);
		switch(direction) {
		case UP: if(jump) x -= 2;
			else x -= 1; break;
		case LEFT: if(jump) y -= 2;
			else y -= 1; break;
		case RIGHT: if(jump) y += 2;
			else y += 1; break;
		case DOWN: if(jump) x += 2;
			else x += 1; break;
			if(diagonals) {

			case UP_LEFT: if(jump) { y -= 2; x -= 2; }
				else { y -= 1; x -= 1; } break;
			case UP_RIGHT: if(jump) { y += 2; x -= 2;  }
				else { y += 1; x -= 1; } break;
			case DOWN_LEFT: if(jump) { y -= 2; x += 2; }
				else { y -= 1; x += 1; } break;
			case DOWN_RIGHT: if(jump) { y += 2; x += 2; }
				else { y += 1; x += 1; } break;
			}
		}
		sprintf(moveStr, "%c%c%c%c", j + 'a' - 1, '0'+ length - i + 1, y + 'a' - 1, '0'+length - x + 1);
		return moveStr;
	}
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
	int i;

	theCanonicalPosition = position;
	//PrintPosition(position, "bob", 0);
	for(i = 0; i < NUMSYMMETRIES; i++) {
		newPosition = DoSymmetry(position, i); /* get new */
		//PrintPosition(newPosition, "bob", 0);
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
** INPUTS:      POSITION position : The position to branch the symmetry
   from.
**              int      symmetry : The number of the symmetry operation.
**
** OUTPUTS:     POSITION, The position after the symmetry operation.
**
************************************************************************/

POSITION DoSymmetry(position, symmetry)
POSITION position;
int symmetry;
{
	int turn, goatsLeft;
	char* board = unhash(position, &turn, &goatsLeft);
	char* unflipped = (char*) SafeMalloc(boardSize * sizeof(char));
	int i;
	for(i = 0; i < boardSize; i++)
		unflipped[i] = board[i];

	/* Copy from the symmetry matrix */

	if(width == 5)
	{
		for(i = 0; i < boardSize; i++)
			board[i] = unflipped[gSymmetryMatrix5[symmetry][i]];
	}
	if(width == 4)
	{
		for(i = 0; i < boardSize; i++)
			board[i] = unflipped[gSymmetryMatrix4[symmetry][i]];
	}
	if(width == 3)
	{
		for(i = 0; i < boardSize; i++)
			board[i] = unflipped[gSymmetryMatrix3[symmetry][i]];
	}
	if (unflipped != NULL)
		SafeFree(unflipped);
	return(hash(board, turn, goatsLeft));
}

// I assumed that turn is always one bit
char* PositionToString(POSITION pos) {
  int i, j;
  int turn, goatsLeft;
  char* board = unhash(pos, &turn, &goatsLeft);
  int total_length = boardSize;
  char* string = (char*) SafeMalloc(total_length * sizeof(char));
  char piece;
  for(i = 1; i <= length; i++) { // print the rows one by one
    for(j = 1; j <= width; j++) {
      piece = board[translate(i, j)];
      if (piece == '+'){
        *string = ' ';
      } else if(piece == 'T') {
        *string = 'O';
      } else {
        *string = 'X';
      }
      string++;
    }
  }
	*string = '\0';
  string -= total_length;
  char* turnValue = (char*) SafeMalloc(2);
  sprintf(turnValue, "%d", turn);
  char* goatValue = (char*) SafeMalloc(11);
  sprintf(goatValue, "%d", goatsLeft);
  char* retString = MakeBoardString(string, "turn", turnValue, "goatsLeft", goatValue, "");
  if (board != NULL)
    SafeFree(board);
  SafeFree(string);
  return retString;
}

POSITION StringToPosition(char* string){
  int total_length = boardSize;
  char* board = (char*) SafeMalloc(total_length * sizeof(char));
  int i = 0;
  while (i < total_length){
    if (*string == ' '){
      *board = '+';
    } else if(*string == 'O' || *string == 'o') {
      *board = 'T';
    } else if (*string == 'X' || *string == 'x'){
      *board = 'G';
    }
    string++;
    board++;
    i++;
  }
  board = board - total_length;
  int goats;
  int turn;
  int success = GetValue(string, "turn", GetInt, &turn) &&
                GetValue(string, "goatsLeft", GetInt, &goats);
  if (success == 0){
    printf("ERROR: something went wrong in getting the values out of the string \n");
    return INVALID_POSITION;
  }
  return hash(board, turn, goats);
}

/***************************
 ******  TIER API    ********
 ****************************/

/* HOW TIERS WORK:
   The tiers are defined as follows:
   FIRST TIER: goats*(goats+1)
   this is simplified to (goats+1)^2 for easier offsets.

   0 - goats (0 - 20): Stage 2 boards. Tier number = number of Goats on the board.
   Nice and straightforward like before.

   (goats+1) - (goats+1)^2 (21 - 441): Stage 1, Goat's turn boards. The math is:
   Take a Stage 2 board. Add to it the number of goats left to place, multiplied
   by (goats+1).
   That means that:
   TIER / (goats+1) = goats left to place
   TIER % (goats+1) = goats on the board (a.k.a. Stage 2 Tier)

   ((goats+1)^2)+1 - MAX+_TIER (442 - 8380): Stage 1, Tiger's turn boards. The main is:
   Take a Stage 2 board. Add to it the number of goats left to place, multiple
   by (goats+1)^2.
   So:
   TIER / (goats+1)^2 = goats left to place
   TIER % (goats+1)^2 = goats on board (a.k.a. Stage 2 Tier)

   Not the most tightly-packed system... but it works. :)

   HOW HASHES WORK:
   We COULD waste the space and define 3 full copies of each of the Stage 2 boards:
   one with both players (for Stage 2), and 2 with only one player (for the two
   Stage 1 types). Instead we only define one: the both player version; so that's
   goats+1 hashes total, one for each of the Stage 2 boards.
   Then for the Stage 1's, we just work with offsets: we know that the Goat's turn
   boards are from 0 through (max_pos/2)-1, and Tiger's turn are max_pos/2 through
   max_pos-1.
 */

// Hash/unhash helpers for tiers
void unhashTier(TIER tier, int* goatsOnBoard, int* goatsLeft, int* turn) {
	if (tier < s1GoatOffset) { //stage 2
		(*goatsOnBoard) = tier;
		(*goatsLeft) = 0;
		(*turn) = 0; // actually undefined since it's BOTH turns, so ignored
	} else if (tier < s1TigerOffset) { //stage 1, goat
		(*goatsOnBoard) = tier % s1GoatOffset;
		(*goatsLeft) = tier / s1GoatOffset;
		(*turn) = PLAYER_ONE;
	} else { //stage 1, tiger
		(*goatsOnBoard) = tier % s1TigerOffset;
		(*goatsLeft) = tier / s1TigerOffset;
		(*turn) = PLAYER_TWO;
	}
}

TIER hashTier(int goatsOnBoard, int goatsLeft, int turn) {
	if (goatsLeft == 0) //stage 2
		return goatsOnBoard;
	else if (turn == PLAYER_ONE) //stage 1, goat
		return goatsOnBoard + (goatsLeft * s1GoatOffset);
	else // stage 2, tiger
		return goatsOnBoard + (goatsLeft * s1TigerOffset);
}

// A helper that comes in handy:
int numGoatsOnBoard (char* board) {
	int goat = 0, i;
	for (i = 0; i < boardSize; i++)
		if (board[i] == GOAT)
			goat++;
	return goat;
}

void SetupTierStuff() {
	// gUsingTierGamesman
	kSupportsTierGamesman = TRUE;

	// offsets for tiers
	s1GoatOffset = goats+1;
	s1TigerOffset = (goats+1)*(goats+1);

	// All other function pointers
	gTierChildrenFunPtr                             = &TierChildren;
	gNumberOfTierPositionsFunPtr    = &NumberOfTierPositions;
	//gIsLegalFunPtr				= &IsLegal;
	gGenerateUndoMovesToTierFunPtr  = &GenerateUndoMovesToTier;
	gUnDoMoveFunPtr                                 = &UnDoMove;
	gTierToStringFunPtr                             = &TierToString;
	// Tier-Specific Hashes (1 per Stage 2 board)
	int tier, piecesArray[10]= {TIGER, tigers, tigers, GOAT, 0, 0, SPACE, 0, 0, -1};
	for (tier = 0; tier <= goats; tier++) {
		// Goats = tier
		piecesArray[4] = piecesArray[5] = tier;
		// Blanks = boardSize - tigers - goats(tier)
		piecesArray[7] = piecesArray[8] = boardSize - tigers - tier;
		// make the hashes
		generic_hash_init(boardSize, piecesArray, NULL, 0);
	}
	gInitialTier = goats*(goats+1);
}

// If Stage 2, children are tier and tier-1 (except 0, it's just 0)
// If Stage 1 goat, one child: the tiger below it with goatsOnBoard+1 and goatsLeft-1
// If Stage 2 tiger, two children: the goats below, both with same goatsLeft, but
//                                                      one with same goatsOnBoard and other with goatsOnBoard-1
TIERLIST* TierChildren(TIER tier)
{
	TIERLIST* list = NULL;
	int goatsOnBoard, goatsLeft, turn;
	unhashTier(tier, &goatsOnBoard, &goatsLeft, &turn);
	if(tier < s1GoatOffset) { //stage2
		list = CreateTierlistNode(tier, list);
		if (tier != 0)
			list = CreateTierlistNode(tier-1, list);
	} else if (tier < s1TigerOffset) { //stage 1, goat
		list = CreateTierlistNode(hashTier(goatsOnBoard+1, goatsLeft-1, PLAYER_TWO), list);
	} else { //stage 1, tiger
		list = CreateTierlistNode(hashTier(goatsOnBoard, goatsLeft, PLAYER_ONE), list);
		list = CreateTierlistNode(hashTier(goatsOnBoard-1, goatsLeft, PLAYER_ONE), list);
	}
	return list;
}

TIERPOSITION NumberOfTierPositions(TIER tier) {
	int goatsOnBoard, goatsLeft, turn;
	unhashTier(tier, &goatsOnBoard, &goatsLeft, &turn);
	generic_hash_context_switch(goatsOnBoard);
	if (tier < s1GoatOffset) // stage 2
		return generic_hash_max_pos();
	else // stage 1, report only half the boards
		return generic_hash_max_pos()/2;
}

BOOLEAN IsLegal(POSITION position) {
	return TRUE;
}

POSITION UnDoMove (POSITION position, UNDOMOVE undoMove)
{
	int turn, goatsLeft;
	char* board = unhash(position, &turn, &goatsLeft);
	char piece;
	int jump, direction, i, j, jumpI, jumpJ;
	if(undoMove < boardSize) { // It's only a Goat being removed
		board[undoMove] = SPACE;
		turn = PLAYER_ONE;
		goatsLeft = goatsLeft+1;
		return hash(board, turn, goatsLeft);
	}
	undoMove -= boardSize;
	jump = undoMove % 2;
	undoMove /= 2;
	direction = undoMove % 8;
	undoMove /= 8;
	i = jumpI = get_x(undoMove);
	j = jumpJ = get_y(undoMove);
	piece = board[undoMove];
	board[undoMove] = SPACE; // clear the current location of the piece
	switch(direction) { // find the appropriate modifiers to i and j
	case UP: if (jump) { i -= 2; jumpI -= 1; }
		else i -= 1; break;
	case DOWN: if (jump) { i += 2; jumpI += 1; }
		else i += 1; break;
	case RIGHT: if (jump) { j += 2; jumpJ += 1; }
		else j += 1; break;
	case LEFT: if (jump) { j -= 2; jumpJ -= 1; }
		else j -= 1; break;
		if (diagonals) {
		case UP_RIGHT: if (jump) { i -= 2; j += 2; jumpI -= 1; jumpJ += 1; }
			else { i -= 1; j += 1; } break;
		case UP_LEFT: if (jump) { i -= 2; j -= 2; jumpI -= 1; jumpJ -= 1; }
			else { i -= 1; j -= 1; } break;
		case DOWN_RIGHT: if (jump) { i += 2; j += 2; jumpI += 1; jumpJ += 1; }
			else { i += 1; j += 1; } break;
		case DOWN_LEFT: if (jump) { i += 2; j -= 2; jumpI += 1; jumpJ -= 1; }
			else { i += 1; j -= 1; } break;
		}
	}
	board[translate(i, j)] = piece; // place the piece in its new location
	if(jump) board[translate(jumpI, jumpJ)] = GOAT; // place the piece jumped over
	turn = (turn == PLAYER_ONE ? PLAYER_TWO : PLAYER_ONE);
	return hash(board, turn, goatsLeft);
}

UNDOMOVELIST* GenerateUndoMovesToTier(POSITION position, TIER tier){
	/*     UP
	        ^
	   LEFT<---|--> RIGHT
	        v
	      DOWN
	   (1,1) IS top left corner and (row,col)=(length, width)= bottom right corner
	 */
	int turn, goatsLeft;
	char* board = unhash(position, &turn, &goatsLeft);
	char animal;
	int row = length, col = width;
	UNDOMOVELIST *moves = NULL;
	int i, j;

	if(PLAYER_ONE == turn)
		animal = TIGER; //opposite of what it should be, means player 2, tiger, just moved
	else
		animal = GOAT; // means player 1 = goat just moved

	// If tigers turn, the goat was just placed
	// check if this is the correct parent tier
	if(animal == GOAT && tier == hashTier(numGoatsOnBoard(board)-1, goatsLeft+1, PLAYER_ONE)) {
		// Generates the list of all possible undo-drop locations for Phase 1
		for(i = 1; i <= row; i++) {
			for(j = 1; j <= col; j++) {
				if(board[translate(i, j)] == GOAT) {
					moves = CreateUndoMovelistNode(translate(i, j), moves);
				}
			}
		}
	}
	// For both players, generate the list of all possible move locations for Phases 1 and 2
	// check if this is the correct parent tier
	if ((animal == TIGER || goatsLeft == 0) &&
	    tier == hashTier(numGoatsOnBoard(board), goatsLeft, (turn == PLAYER_ONE ? PLAYER_TWO : PLAYER_ONE))) {
		for(i = 1; i <= row; i++) {
			for(j = 1; j <= col; j++)    {
				if(board[translate(i, j)] == animal) {
					// Move Up
					if((i-1 > 0) && (board[translate(i-1, j)] == SPACE))
						moves = CreateUndoMovelistNode((translate(i, j)*8+UP)*2 + boardSize, moves);
					// Move Right
					if((j+1 <= col) && (board[translate(i, j+1)] == SPACE))
						moves = CreateUndoMovelistNode((translate(i, j)*8+RIGHT)*2 + boardSize, moves);
					// Move Down
					if((i+1 <= row) && (board[translate(i+1, j)] == SPACE))
						moves = CreateUndoMovelistNode((translate(i, j)*8+DOWN)*2 + boardSize, moves);
					// Move Left
					if((j-1 > 0) && (board[translate(i, j-1)] == SPACE))
						moves = CreateUndoMovelistNode((translate(i, j)*8+LEFT)*2 + boardSize, moves);
					//DIAGONAL MOVES
					if(diagonals) { // Move NW
						if((i-1 > 0) && (j-1 > 0) && (((i + j) % 2) == 0)    &&
						   (board[translate(i-1, j-1)] == SPACE))
							moves = CreateUndoMovelistNode((translate(i, j)*8+UP_LEFT)*2 + boardSize, moves);
						// Move NE
						if((i-1 > 0) && (j+1 <= col) && (((i + j) % 2) == 0) &&
						   (board[translate(i-1, j+1)] == SPACE))
							moves = CreateUndoMovelistNode((translate(i, j)*8+UP_RIGHT)*2 + boardSize,moves);
						// Move SE
						if((i+1 <= row) && (j+1 <= col) && (((i + j) % 2) == 0) &&
						   (board[translate(i+1, j+1)] == SPACE))
							moves = CreateUndoMovelistNode((translate(i, j)*8+DOWN_RIGHT)*2 + boardSize, moves);
						// Move SW
						if((i+1 <= row) && (j-1 > 0) && (((i + j) % 2) == 0) &&
						   (board[translate(i+1, j-1)] == SPACE))
							moves = CreateUndoMovelistNode((translate(i, j)*8+DOWN_LEFT)*2 + boardSize, moves);
					}
				}
			}
		}
	}
	// tigers can jump
	// check if this is the correct parent tier
	if(animal == TIGER && tier == hashTier(numGoatsOnBoard(board)+1, goatsLeft, PLAYER_ONE)) {
		for(i = 1; i <= row; i++) {
			for(j = 1; j <= col; j++)    {
				if(board[translate(i, j)] == animal) {
					// Jump Up
					if((i-1 > 1) && (board[translate(i-1, j)] == SPACE) &&
					   (board[translate(i-2, j)] == SPACE))
						moves = CreateUndoMovelistNode((translate(i, j)*8+UP)*2+1 + boardSize, moves);
					// Jump Right
					if((j+1 < width) && (board[translate(i, j+1)] == SPACE)    &&
					   (board[translate(i, j+2)] == SPACE))
						moves = CreateUndoMovelistNode((translate(i, j)*8+RIGHT)*2+1 + boardSize, moves);
					// Jump Down
					if((i+1 < row) && (board[translate(i+1, j)] == SPACE) &&
					   (board[translate(i+2, j)] == SPACE))
						moves = CreateUndoMovelistNode((translate(i, j)*8+DOWN)*2+1 + boardSize, moves);
					// Jump Left
					if((j-1 > 1) && (board[translate(i, j-1)] == SPACE) &&
					   (j-2 > 0) && (board[translate(i, j-2)] == SPACE))
						moves = CreateUndoMovelistNode((translate(i, j)*8+LEFT)*2+1 + boardSize, moves);
					if(diagonals) { // Jump SW
						if((i+1 < length) && (j-1 > 1)    &&
						   (board[translate(i+1, j-1)] == SPACE)    &&
						   (((i + j) % 2) == 0)    &&
						   (board[translate(i+2, j-2)] == SPACE))
							moves = CreateUndoMovelistNode((translate(i, j)*8+DOWN_LEFT)*2+1 + boardSize, moves);
						// Jump SE
						if((i+1 < length) && (j+1 < width) &&
						   (board[translate(i+1, j+1)] == SPACE)    &&
						   (((i + j) % 2) == 0)    &&
						   (board[translate(i+2, j+2)] == SPACE))
							moves = CreateUndoMovelistNode((translate(i, j)*8+DOWN_RIGHT)*2+1 + boardSize, moves);
						// Jump NE
						if((i-1 > 1) && (j+1 < width) &&
						   (board[translate(i-1, j+1)] == SPACE)&& (((i+j) %2) == 0) &&
						   (board[translate(i-2, j+2)] == SPACE))
							moves = CreateUndoMovelistNode((translate(i, j)*8+UP_RIGHT)*2+1 + boardSize, moves);
						// Jump NW
						if((i-1 > 1) && (j-1 > 1) &&
						   (board[translate(i-1, j-1)] == SPACE) &&
						   (i-2 >0) && (j-2 >0) && (((i + j) % 2) == 0) && (board[translate(i-2, j-2)] ==  SPACE))
							moves = CreateUndoMovelistNode((translate(i, j)*8+UP_LEFT)*2+1 + boardSize, moves);
					}
				}
			}
		}
	}
	if (board != NULL)
		SafeFree(board);
	return moves;
}

// Tier = Number of pieces left to place.
STRING TierToString(TIER tier) {
	STRING tierStr = (STRING) SafeMalloc(sizeof(char)*45);
	int goatsOnBoard, goatsLeft, turn;
	unhashTier(tier, &goatsOnBoard, &goatsLeft, &turn);
	if (tier < s1GoatOffset)
		sprintf(tierStr, "Stage 2, %d Goats", goatsOnBoard);
	else if (tier < s1TigerOffset)
		sprintf(tierStr, "Stage 1 (GOAT's Turn), %d Goats, %d To Place",
		        goatsOnBoard, goatsLeft);
	else
		sprintf(tierStr, "Stage 1 (TIGER's Turn), %d Goats, %d To Place",
		        goatsOnBoard, goatsLeft);
	return tierStr;
}

// $Log: not supported by cvs2svn $
// Revision 1.38  2007/05/11 01:38:18  max817
// Fixed a bug
//
// Revision 1.37  2007/05/08 22:14:00  max817
// Fixed a bug with initializing the game
//
// Revision 1.36  2007/05/07 22:12:04  max817
// 3x3
//
// Revision 1.35  2007/05/07 07:04:42  brianzimmer
//
//
// An attempt to fix a bug that was being experienced.
//
// Revision 1.34  2007/04/19 04:43:32  max817
// More support for Tcl. -Max
//
// Revision 1.33  2007/04/19 04:40:16  max817
// Support for Tcl. -Max
//
// Revision 1.32  2007/04/03 01:29:05  eudean
// Added Tcl pointer so a GUI can be made.
//
// Revision 1.31  2007/02/27 02:15:00  max817
// Fixed a bug with the global board inits. -Max
//
// Revision 1.30  2007/02/27 01:24:33  max817
// Made more efficient by adding globals. -Max
//
// Revision 1.29  2006/12/19 20:00:50  arabani
// Added Memwatch (memory debugging library) to gamesman. Use 'make memdebug' to compile with Memwatch
//
// Revision 1.28  2006/10/17 10:45:20  max817
// HUGE amount of changes to all generic_hash games, so that they call the
// new versions of the functions.
//
// Revision 1.27  2006/10/11 06:59:02  max817
// A quick modification of the Tier Gamesman games to include the new changes.
//
// Revision 1.26  2006/09/27 11:28:58  max817
// Removed "Tier0Context" usage from both Tier-Gamesman games, now that hash
// destruction works.
//
// Revision 1.25  2006/09/11 05:20:36  max817
// Fixed the bug with Tier-Gamesman unhash. Now, with the exception of the
// undomove functions and the (lack of) IsLegal, the Tier-Gamesman version
// of Bagh Chal is (hopefully) 100% correct, working, and ready to solve.
//
// Revision 1.24  2006/08/23 03:28:40  max817
// CVS'ing in Deepa's symmetries changes.
//
// Revision 1.23  2006/08/09 02:19:04  max817
// Tier Gamesman API fully implemented! UndoMove fuctions now work.
//
// Revision 1.22  2006/08/08 01:57:21  max817
// Added the parent pointers version of the loopy solver for the Retrograde
// Solver. Also added most of the API for Bagh Chal so that it uses this
// version. The UndoMove functions are yet to be implemented correctly,
// however.
//
// Revision 1.21  2006/08/07 01:05:27  max817
// *** empty log message ***
//
// Revision 1.20  2006/08/05 19:03:36  deepamahajan
// changes set/getOption to include symmetries
//
// Revision 1.19  2006/08/04 20:50:47  max817
// Cleaned up the code, completely revamped hashing and unhashing, and got
// rid of ugly semi-state variables like phase1 and NumGoats. Now the
// regular game is probably as close to final version as can be. Tier
// Gamesman stuff is still in progress so it's disabled for now, but should
// be finished by next update.
//
// Revision 1.18  2006/07/31 04:26:46  max817
// Fixed a few (mostly hash-related) bugs in the Tier-Gamesman API functions.
// More fixes to come.
//
// Revision 1.17  2006/07/30 23:30:06  deepamahajan
// Tier API added!
//
// Revision 1.15  2006/05/08 08:13:07  deepamahajan
// *** empty log message ***
//
// Revision 1.15  2006/05/08 01:10:00 deepamahajan
// Added MoveToString and changed board to follow standard chess conventions.
// Revision 1.14  2006/05/03 06:13:24  ddgarcia
// Removed the space (and the []s) from printmove and other corresponding
// functions, so that the user's move is now "a1b1" instead of "a1 b1", which
// means that the moves can now be printed [ a1b1 a2b2 ... ] instead of
// [ [a1 b2] [a2 b2] ... ] (i.e., more compact) -dan 2006-05-02
//
// Revision 1.13  2006/04/17 07:36:38  max817
// mbaghchal.c now has no solver debug code left (i.e. it's independent of anything in solveretrograde.c) and has variants implemented correctly. Aside from the POSITION size stuff, it should be near its final version.
// As for the solver, all of the main features I wanted to implement are now implemented: it is almost zero-memory (aside from memdb usage), and it's possible to stop and save progress in the middle of solving. Also, ANY game can use the solver now, not just Bagh Chal. All in all, it's close to the final version (for this semester). -Max
//
// Revision 1.12  2006/04/12 03:02:12  max817
// This is the big update that moves the Retrograde Solver from mbaghchal.c
// to its own set of new files, solveretrograde.c and solveretrograde.h.
// Exact details on the exact changes I made to the core files can be found
// in a comment on solveretrograde.c. -Max
//

char * PositionToEndData(POSITION pos) {
	return NULL;
}
