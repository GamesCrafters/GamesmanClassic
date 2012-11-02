// $Id: mparadux.c,v 1.24 2006-12-19 20:00:51 arabani Exp $

/*
 * The above lines will include the name and log of the last person
 * to commit this file to CVS
 */

/************************************************************************
**
** NAME:        mparadux.c
**
** DESCRIPTION: Paradux
**
** AUTHOR:      David Chen   chendx@berkeley.edu
**              Yanpei Chen  ychen@berkeley.edu
**
** DATE:        09/13/03
**
** UPDATE HIST:
**
** 09/13/2005 David  - First Revision
** 09/14/2005 Yanpei - Fixed some typo in InitializeGame().
**                     Proposed alternative board indexing.
**                     Drew initial board position.
** 09/26/2005 Yanpei - Re-drew board position and coordinates.
** 09/27/2005 Yanpei - Some support functions and data structs
** 09/27/2005 David  - Merged some changes from my version (CVS doesn't
**                     work for me?)
** 10/04/2005 David  - Added a bunch of simple functions--without testing
**                     and which are incomplete; just wanted to get
**                     something out before the meeting.
** 10/18/2005 Yanpei - Tidied up davidPrintPos(), wrote dyPrintPos() using
**                     more tidy code. Proof read hashMove() unhashMove()
** 10/26/2005 Yanpei - PrintPos and initializeGame for odd boards debugged.
** 11/09/2005 Yanpei - DoMove proof read, reasonably confident,
**                     need to write getNeighbor,
**                     yanpeiTestNeighboringDir() written but yet to run.
**                     Primitive proof read, reasonably confident
**                     Wrote PrintMove + PrintComputersMove
**                     Still to do: GenerateMoves + test UI functions
** 11/10/2005 Yanpei - GenerateMoves proof read, reasonably confident
**                     Need to write AddMovesPerPair to make it work
**                     Need to check hash to see whether need to keep
**                     track of turns.
** 11/18/2005 David  - Got it compiling and running. Looks like there's a
**                     memory leak somewhere when solving. Also, solving
**                     is very slow. Anyways, this update plays. Next will
**                     have optimizations, maybe GPS.
** 11/19/2005 David  - Added caching of all lookups. Solved in 18000 secs
**                     as a draw. However, there are still some small
**                     issues that need to be resolved so that solution
**                     might not be correct.
** 11/21/2005 David  - Added help text (except the example play, because
**                     solving the game takes too long). Cleaned up the
**                     code a lot.
** 11/22/2005 David  - Added functionality to support the rule that a
**                     player cannot undo the move that his opponent has
**                     just made. Unfortunately, this brings the number of
**                     positions to ~37 billion. Will decide proper course
**                     of action tomorrow.
** 11/22/2005 David  - Solved Paradux without the undo stuff last night.
**                     At most, a player can have 34 moves (experimentally
**                     discovered). Therefore, I simply enumerate the
**                     moves and the hash actually fits under 2^31.
**                     Unfortunately, memdb calls malloc on
**                     gNumberOfPositions * sizeof(VALUE)--which is about
**                     2 GB--which my computer can't handle. Using 2-bit
**                     solver thrases my hard drive. This is about as
**                     compact as we can get... Time to commit and let
**                     Yanpei think about this one.
** 12/07/2005 Yanpei - Added prevMoveAllowed as game option
**                     Coded brief game options menu
**                     Fixed some memory leaks
** 12/19/2005 David  - Fixed first player always wins
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

STRING kGameName            = "Paradux";   /* The name of your game */
STRING kAuthorName          = "David Chen, Yanpei Chen";   /* Your name(s) */
STRING kDBName              = "Paradux";   /* The name to store the database under */

BOOLEAN kPartizan            = FALSE;   /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
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
        "ON YOUR TURN, use the legend to determine which pieces to move.\n\
Every legal move requires two pieces, one of each color.\n\
These two pieces can then be moved together in any direction (northwest,\n\
northeast, west, east, southwest, or southeast) so long as the slots in\n\
that direction are empty. In addition, the pieces may be swapped.\n\n\
To perform a move, use the legend to find the slots associated with your\n\
pieces. Enter those (in either order) and then enter the direction using\n\
the movement key, each separated by spaces."                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              ;

STRING kHelpOnYourTurn =
        "Choose one of your pieces and one of your opponent's pieces.\n\
Then choose a direction to move in or SWAP to swap the pieces."                                                                         ;

STRING kHelpStandardObjective =
        "On a board with side length N, to line N of your pieces in a row in any direction.";

STRING kHelpReverseObjective =
        "On a board with side length N, to line N of your opponent's pieces in a row in\n\
any direction."                                                                                           ;

STRING kHelpTieOccursWhen =
        "A tie can never occur";

STRING kHelpExample =
        "";

/*************************************************************************
**
** Game-specific variables
**
**************************************************************************/

#define NUM_OF_OPTIONS 1

/* The actual board */
char *board;

/* Mapping from values to characters */
char valToChar[] = { '-', 'X', 'O', '*' };

/* Just for kicks, let the user choose his name.. and the computer's */
STRING playersName = "Humanoid";
STRING computerName = "Robbie";

/*************************************************************************
**
** #defines and structs
**
**************************************************************************/

/* Board Coordinates

          row,col format          el(ement) format

          0,0  0,1  0,2              00  01  02

       1,0  1,1  1,2  1,3          03  04  05  06

     2,0  2,1  2,2  2,3  2,4     07  08  09  10  11

       3,0  3,1  3,2  3,3          12  13  14  15

         4,0  4,1  4,2               16  17  18


 */

/* Initial board -

         Paradux mini                  Paradux regular

          X   O   X                     X   O   X   O

        O   -   -   -                 O   -   -   -   X

      X   -   -   -   0             X   -   -   X   -   O

        -   -   -   x             O   -   -   -   -   -   X

          0   x   0                 X   -   O   -   -   O

                                      O   -   -   -   X

                                        X   O   X   O
 */

/* macros */
#define nextPlayer(x) ((x) % 2 + 1)

/* pieces */
#define WHITE    1
#define BLACK    2
#define BLANK    0
#define X        1
#define O        2
#define INVALID -1

#define NW       0
#define NE       1
#define E        2
#define W        3
#define SW       4
#define SE       5
#define SWAP     6
#define INVALID -1

#define MAX_DIR        5
#define NUM_DIRS       6
#define NUM_MOVE_TYPES 7

// must be used on arithmetic expressions and comparables
// "x" and "y" should not contain side effects
#define max(x,y) ((x)>(y) ? (x) : (y))
#define min(x,y) ((x)<(y) ? (x) : (y))

// must be used on ints or POSITION or the like
#define abs(x) (0<(x) ? (x) : -(x))

#define HASH_MOVE(type, slot, nDir) ((type) * boardSize * NUM_DIRS + (slot) * NUM_DIRS + (nDir))
#define UNHASH_MOVE_TYPE(move) ((move) / (boardSize * NUM_DIRS))
#define UNHASH_MOVE_SLOT(move) ((move) % (boardSize * NUM_DIRS) / NUM_DIRS)
#define UNHASH_MOVE_NDIR(move) ((move) % (boardSize * NUM_DIRS) % NUM_DIRS)

/*************************************************************************
**
** Global Variables
**
*************************************************************************/

/* On the hexagonal board, only one side needs to be specified */
int boardSide = 3;

/* Other options */
int firstGo = X;
int prevMoveAllowed = TRUE;

/* Magically generated (in InitializeGame) */
int maxDim;
int boardSize;
int numX;
int numO;
int numBlank;
int numGenericPositions;

/* The maximum number of moves in one turn; this is experimentally derived. */
int maxNumMoves;

int slotStrLength;

/* Whether or not the caches have been initialized already */
BOOLEAN cachesInited = FALSE;

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

/* Support Functions */

void                    initOddBoard();
void                    initEvenBoard();
void                    initCaches();
void                    freeCaches();

void                    AddMovesPerPair(MOVELIST **moves, char *board, int slot1, int slot2, MOVE lastMoveHash);
BOOLEAN                 isOpen(char* board, int direction, int slot1, int slot2);

MOVE                    hashMove(int type, int slot1, int slot2);
void                    unhashMove(MOVE move, int* type, int* slot1, int* slot2);

POSITION                paraduxHashInit(int boardSize, int* pieces_array, int (*fn)(int *));
POSITION                paraduxHash(char* board, MOVE lastMoveHash, int player);
char*                   paraduxUnhash(POSITION hashed, char* dest, MOVE* lastMoveHash);

char readchar();


// returns TRUE if square u,v is next to square x,y
// where u,v and x,y are the row,column coordinates
// returns FALSE for invalid coordinates
BOOLEAN                 neighbor(int u, int v, int x, int y);

// if there is no neigher in the specified direction, return INVALID
// returns a board index b/n 0 and boardSize
int                     getNeighbor(int slot, int direction);
int*                    getNeighborCache;
#define                 GET_NEIGHBOR(slot, dir) getNeighborCache[(slot) * NUM_DIRS + (dir)]

/* Direction in which the neighbor neighbors slot:
   returns one of
   0 NW
   1 NE
   2 E
   3 SE
   4 SW
   5 W
   -1 Not neighboring
 */
int                     neighboringDirection(int slot, int neighbor);
int*                    neighboringDirectionCache;
#define                 NEIGHBORING_DIRECTION(slot, neighbor) neighboringDirectionCache[(slot) * boardSize + (neighbor)]

// returns slot number when given row and column coordinates
// returns -1 when given invalid coordinates
int                     rcToSlot(int r, int c);
int*                    rcToSlotCache;
#define                 RC_TO_SLOT(r, c) rcToSlotCache[(r) * maxDim + (c)]

// returns row and column coordinate when given slot number
// returns -1 when given invalid slot number
void                    slotToRC(int slot, int* pRow, int* pCol);
int*                    slotToRCCache;
#define                 SLOT_TO_ROW(slot) slotToRCCache[2 * (slot)]
#define                 SLOT_TO_COL(slot) slotToRCCache[2 * (slot) + 1]

/* Test Functions */

void                    runTests();

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
	int pieces[10];

	if (boardSide < 3) {
		printf("ERROR: boards with side length < 3 cannot be represented or are trivial");
		exit(1);
	}

	maxDim = boardSide * 2 - 1;

	/* 3(n-1)n + 1 */
	boardSize = 3 * (boardSide - 1) * boardSide + 1;

	int slotStrL = 1, temp = boardSize;
	while ((temp = temp / 10) >= 10) {
		slotStrL++;
	}
	slotStrLength = slotStrL;

	switch(boardSide) {
	case 3:
		maxNumMoves = 34;
		break;
	default:
		printf("Warning: maxNumMoves has not yet been generated for a board of this size.\n\n");
		maxNumMoves = boardSize * NUM_DIRS * NUM_DIRS;
	}

	// Add one for the first move
	maxNumMoves++;

	if (boardSide % 2) {
		// odd-sided board
		numX = numO = 2*boardSide - 1;
	} else {
		// even-sided board
		numX = numO = (boardSide - 1) * 3 + 1;
	}
	numBlank = boardSize - numX - numO;

	pieces[0] = 'X';
	pieces[1] = numX;
	pieces[2] = numX;
	pieces[3] = 'O';
	pieces[4] = numO;
	pieces[5] = numO;
	pieces[6] = '-';
	pieces[7] = numBlank;
	pieces[8] = numBlank;
	pieces[9] = -1;

	//  gNumberOfPositions = generic_hash_init(boardSize, pieces, NULL, 0);
	gNumberOfPositions = paraduxHashInit(boardSize, pieces, NULL);

	// Due to horrible performance, we cache all of the position lookup code
	initCaches();

	if (boardSide % 2) {
		initOddBoard();
	} else {
		initEvenBoard();
	}
}

/* Initialize odd-sided board */
void initOddBoard() {
	int row, col, el = 0, maxCol = boardSide - 1, maxRow = boardSide * 2 - 2;

	char* board = (char *) SafeMalloc (sizeof(char) * boardSize);

	// This is an impossible move used as a dummy for paraduxhashMove
	MOVE lastMoveHash = hashMove(NW, 0, 1);

	for (row = 0; row <= maxRow; row++) {
		if (row < boardSide) {
			maxCol++;
		} else {
			maxCol--;
		}

		for (col = 0; col < maxCol; col++, el++) {
			if (row == 0) {
				board[el] = valToChar[col % 2 + 1];
			} else if (row == maxRow) {
				board[el] = valToChar[(col + 1) % 2 + 1];
			} else if (row <= boardSide - 2) {
				if (col == 0) {
					board[el] = valToChar[row % 2 + 1];
				} else {
					board[el] = valToChar[BLANK];
				}
			} else if (row >= boardSide) {
				if (col == maxCol-1) {
					board[el] = valToChar[(row + 1) % 2 + 1];
				} else {
					board[el] = valToChar[BLANK];
				}
			} else {
				if (col == 0) {
					board[el] = valToChar[row % 2 + 1];
				} else if (col == maxCol-1) {
					board[el] = valToChar[(row+1) % 2 + 1];
				} else {
					board[el] = valToChar[BLANK];
				}
			}
		}
	}

	//  gInitialPosition = generic_hash_hash(board, firstGo);
	gInitialPosition = paraduxHash(board, lastMoveHash, firstGo);
	SafeFree(board);
}

/* Initialize even-sided board */
void initEvenBoard() {
	int row, col, el = 0, maxCol = boardSide - 1, maxRow = boardSide * 2 - 2;

	char* board = (char *) SafeMalloc (sizeof(char) * boardSize);

	// This is an impossible move used as a dummy for paraduxhashMove
	MOVE lastMoveHash = hashMove(NW, 0, 1);

	for (row = 0; row <= maxRow; row++) {
		if (row < boardSide) {
			maxCol++;
		} else {
			maxCol--;
		}

		for (col = 0; col < maxCol; col++, el++) {
			if (row == 0 || row == maxRow) {
				board[el] = valToChar[col % 2 + 1];
			} else if (row == boardSide - 2) {
				if (col == 0 || col == boardSide - 1) {
					board[el] = valToChar[X];
				} else if (col == maxCol-1) {
					board[el] = valToChar[O];
				} else {
					board[el] = valToChar[BLANK];
				}
			} else if (row == boardSide) {
				if (col == 0) {
					board[el] = valToChar[X];
				} else if (col == maxCol-1 || col == boardSide - 2) {
					board[el] = valToChar[O];
				} else {
					board[el] = valToChar[BLANK];
				}
			} else {
				if (col == 0) {
					board[el] = valToChar[row % 2 + 1];
				} else if (col == maxCol-1) {
					board[el] = valToChar[(row+1) % 2 + 1];
				} else {
					board[el] = valToChar[BLANK];
				}
			}
		}
	}

	//  gInitialPosition = generic_hash_hash(board, firstGo);
	gInitialPosition = paraduxHash(board, lastMoveHash, firstGo);
	SafeFree(board);
}

void initCaches ()
{
	int slot, row, col, neighbor, dir;

	// Make sure to free the caches if they've already been initialized
	if (cachesInited) {
		freeCaches();
		cachesInited = FALSE;
	}

	// First, the slotToRCCache
	slotToRCCache = (int *) SafeMalloc (sizeof(int) * boardSize * 2);

	for (slot = 0; slot < boardSize; slot++) {
		slotToRC(slot, &row, &col);

		SLOT_TO_ROW(slot) = row;
		SLOT_TO_COL(slot) = col;
	}

	for (slot = 0; slot < boardSize; slot++) {
		slotToRC(slot, &row, &col);
	}

	// Next, the rcToSlotCache
	rcToSlotCache = (int *) SafeMalloc (sizeof(int) * maxDim * maxDim);

	for (row = 0; row < maxDim; row++) {
		for (col = 0; col < maxDim; col++) {
			RC_TO_SLOT(row, col) = rcToSlot(row, col);
		}
	}

	// ...neighboringDirectionCache
	neighboringDirectionCache = (int *) SafeMalloc (sizeof(int) * boardSize * boardSize);

	for (slot = 0; slot < boardSize; slot++) {
		for (neighbor = 0; neighbor < boardSize; neighbor++) {
			NEIGHBORING_DIRECTION(slot, neighbor) = neighboringDirection(slot, neighbor);
		}
	}

	// And, finally, getNeighborCache
	getNeighborCache = (int *) SafeMalloc (sizeof(int) * boardSize * NUM_DIRS);

	for (slot = 0; slot < boardSize; slot++) {
		for (dir = 0; dir < NUM_DIRS; dir++) {
			GET_NEIGHBOR(slot, dir) = getNeighbor(slot, dir);
		}
	}

	cachesInited = TRUE;
}

void freeCaches ()
{
	SafeFree(slotToRCCache);
	SafeFree(rcToSlotCache);
	SafeFree(neighboringDirectionCache);
	SafeFree(getNeighborCache);
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

MOVELIST *GenerateMovesFromBoard (char *board, MOVE lastMoveHash)
{
	MOVELIST *moves = NULL;
	int i;

	int slot, nSlot;
	char piece;

	for (i = 0; i < boardSize; i++) {
		slot = i;

		piece = board[i];

		char otherPiece = (piece == valToChar[X] ? valToChar[O] : valToChar[X]);

		if (piece != '-') {
			// Check east
			nSlot = GET_NEIGHBOR(i, E);
			if (nSlot >= 0 && board[nSlot] == otherPiece) {
				AddMovesPerPair(&moves, board, slot, nSlot, lastMoveHash);
			}

			// Check southwest
			nSlot = GET_NEIGHBOR(i, SW);
			if (nSlot >= 0 && board[nSlot] == otherPiece) {
				AddMovesPerPair(&moves, board, slot, nSlot, lastMoveHash);
			}

			// Check southeast
			nSlot = GET_NEIGHBOR(i, SE);
			if (nSlot >= 0 && board[nSlot] == otherPiece) {
				AddMovesPerPair(&moves, board, slot, nSlot, lastMoveHash);
			}

			// No need to check W NE or NW because we start from slot = 0 forwards
			// So any W NE or NW should have been found by E SW or SE earlier.
		}
	}

	return moves;
}

MOVELIST *GenerateMoves (POSITION position)
{
	char *board = SafeMalloc(sizeof(char) * boardSize);

	MOVE lastMoveHash;

	//  generic_hash_unhash(position, board);
	paraduxUnhash(position, board, &lastMoveHash);
	// if prevMoveAllowed then lastMoveHash = INVALID = -1

	MOVELIST *moves = GenerateMovesFromBoard(board, lastMoveHash);

	SafeFree(board);

	return moves;
}

void AddMovesPerPair(MOVELIST **moves, char *board, int slot1, int slot2, MOVE lastMoveHash) {
	// Note that we will have to take care of not being able to undo a move here

	//  int lastMoveHash = -1;

	int curHash;

	// Swapping pieces requires little logic
	curHash = hashMove(SWAP, slot1, slot2);
	if (curHash != lastMoveHash) {
		*moves = CreateMovelistNode(curHash, *moves);
	}

	int i;

	// Loop over each of the directions
	for (i = 0; i < NUM_DIRS; i++) {
		if (isOpen(board, i, slot1, slot2)) {
			curHash = hashMove(i, slot1, slot2);

			if (curHash != lastMoveHash) {
				*moves = CreateMovelistNode(curHash, *moves);
			}
		}
	}
}

// Returns true if the two pieces can move in the given direction
BOOLEAN isOpen(char* board, int direction, int slot1, int slot2) {
	int p1Top2 = NEIGHBORING_DIRECTION(slot1, slot2);
	int p2Top1 = MAX_DIR - p1Top2;

	int p1Neighbor = GET_NEIGHBOR(slot1, direction),
	    p2Neighbor = getNeighbor(slot2, direction);

	if (direction == p1Top2) {
		return p2Neighbor != INVALID && board[p2Neighbor] == '-';
	} else if (direction == p2Top1) {
		return p1Neighbor != INVALID && board[p1Neighbor] == '-';
	} else {
		return p1Neighbor != INVALID && board[p1Neighbor] == '-' && p2Neighbor != INVALID && board[p2Neighbor] == '-';
	}

	// should never fall through to here
	BadElse("isOpen");
	return FALSE;
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
	int type, slot1, slot2, target1, target2, temp1, temp2, hashVal;

	// This is so that a user cannot undo his opponent's move
	// (which would make the game a trivial draw)
	MOVE oppositeMove;

	char *board = SafeMalloc(sizeof(char) * boardSize);

	unhashMove(move, &type, &slot1, &slot2);
	//  generic_hash_unhash(position, board);
	paraduxUnhash(position, board, &oppositeMove);
	// if prevMoveAllowed then oppositeMove = INVALID = -1

	if (type == SWAP) {
		temp1 = board[slot1];
		board[slot1] = board[slot2];
		board[slot2] = temp1;

		if (!prevMoveAllowed) oppositeMove = hashMove(SWAP, slot1, slot2);
	} else {
		target1 = GET_NEIGHBOR(slot1, type);
		target2 = GET_NEIGHBOR(slot2, type);

		temp1 = board[slot1];
		temp2 = board[slot2];
		board[slot1] = valToChar[BLANK];
		board[slot2] = valToChar[BLANK];
		board[target1] = temp1;
		board[target2] = temp2;

		if (!prevMoveAllowed) oppositeMove = hashMove(MAX_DIR - type, target1, target2);
	}

	//  int hashVal = generic_hash_hash(board, nextPlayer(generic_hash_turn(position)));
	hashVal = paraduxHash(board, oppositeMove, nextPlayer(generic_hash_turn(position)));

	SafeFree(board);

	return hashVal;
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
	char *board = SafeMalloc(sizeof(char) * boardSize);
	int slot, curSlot;
	char piece, curPiece, playersPiece = valToChar[generic_hash_turn(position % numGenericPositions)];
	int i, j;

	MOVE lastMoveHash;

	//  generic_hash_unhash(position, board);
	paraduxUnhash(position, board, &lastMoveHash);

	for (i = 0; i < boardSize; i++) {
		slot = i;

		piece = board[i];

		if (piece != '-') {

			// Check east
			curSlot = slot;

			for (j = 0; j < boardSide - 1; j++) {
				if ((curSlot = GET_NEIGHBOR(curSlot, E)) == INVALID)
					break;

				curPiece = board[curSlot];

				if (piece != curPiece)
					break;

				if (j == boardSide - 2) {
					SafeFree(board);

					printf("primitive at board location %d, playersPiece = %c\n", i, playersPiece);

					printf("this is a %s position\n", (playersPiece == piece ? "winning" : "losing"));

					return playersPiece == piece ? win : lose;
				}
			}

			// Check southeast
			curSlot = slot;

			for (j = 0; j < boardSide - 1; j++) {
				if ((curSlot = GET_NEIGHBOR(curSlot, SE)) == INVALID)
					break;

				curPiece = board[curSlot];

				if (piece != curPiece)
					break;


				if (j == boardSide - 2) {
					SafeFree(board);

					printf("primitive at board location %d, playersPiece = %c\n", i, playersPiece);

					printf("this is a %s position\n", (playersPiece == piece ? "winning" : "losing"));

					return playersPiece == piece ? win : lose;
				}
			}

			// Check southwest
			curSlot = slot;

			for (j = 0; j < boardSide - 1; j++) {
				if ((curSlot = GET_NEIGHBOR(curSlot, SW)) == INVALID)
					break;

				curPiece = board[curSlot];

				if (piece != curPiece)
					break;

				if (j == boardSide - 2) {
					SafeFree(board);

					printf("primitive at board location %d, playersPiece = %c\n", i, playersPiece);

					printf("this is a %s position\n", (playersPiece == piece ? "winning" : "losing"));

					return playersPiece == piece ? win : lose;
				}
			}

			// No need to check W NE or NW because we search from slot = 0 forwards
			// So any W NE or NW should have been found by E SW or SE earlier.
		}
	}

	SafeFree(board);
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
	printf("It is %s's turn.\n\n", (usersTurn ? playersName : computerName));

	int el = 0, initEl;
	int row, col, i, totalCols = boardSide;
	char* board = (char*) SafeMalloc(sizeof(char) * boardSize);

	MOVE lastMoveHash;

	//  generic_hash_unhash(position, board);
	paraduxUnhash(position, board, &lastMoveHash);

	if (!prevMoveAllowed) {
		if (lastMoveHash > 0) {
			printf("NOTE: You may not make the move ");
			PrintMove(lastMoveHash);
			printf(" as it would undo the last move.\n\n");
		}
	}

	for (row = 0; row < boardSide * 2 - 1; row++, (row<boardSide ? totalCols++ : totalCols--)) {
		initEl = el;

		// Leading spaces
		printf("     ");

		for (i = 0; i < abs(boardSide - row - 1); i++) {
			printf("  ");
		}
		// Columns
		for (col = 0; col < totalCols; col++, el++) {
			printf("%c   ", board[el]);
		}
		// Spaces between board and divider
		for (i = 0; i < abs(boardSide - row - 1); i++) {
			printf("  ");
		}

		printf("         ");
		el = initEl;

		// Spaces between divider and legend; and legend
		if (boardSide < 6) {
			// less than 100 slots
			for (i = 0; i < abs(boardSide - row - 1); i++) {
				printf("  ");
			}
			for (col = 0; col < totalCols; col++, el++) {
				printf("%2d  ", el);
			}
		} else {
			// more than 100 slots
			for (i = 0; i < abs(boardSide - row); i++) {
				printf("   ");
			}
			for (col = 0; col < totalCols; col++, el++) {
				printf("%3d  ", el);
			}
		} // end lengend

		printf("\n\n");
	} // end for (row = 0 ...)

	printf("+---------------------------+------------------------------+\n");
	printf("|        MOVEMENT KEY       |          PREDICTION          |\n");
	printf("|                           |                              |\n");
	printf("|   NW     NE               |                              |\n");
	printf("|     \\   /                 |                              |\n");
	printf("|   W - + - E   or   SWAP   |  %-26s  |\n", GetPrediction(position,playersName,usersTurn));
	printf("|     /   \\                 |                              |\n");
	printf("|   SW     SE               |                              |\n");
	printf("|                           |                              |\n");
	printf("+---------------------------+------------------------------+\n\n");

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
	int type, slot1, slot2;
	unhashMove(computersMove, &type, &slot1, &slot2);

	switch (type) {
	case NW:
		printf("Computer %s moves %d %d NW",computersName,slot1,slot2);
	case NE:
		printf("Computer %s moves %d %d NE",computersName,slot1,slot2);
	case E:
		printf("Computer %s moves %d %d E", computersName,slot1,slot2);
	case SE:
		printf("Computer %s moves %d %d SE",computersName,slot1,slot2);
	case SW:
		printf("Computer %s moves %d %d SW",computersName,slot1,slot2);
	case W:
		printf("Computer %s moves %d %d W", computersName,slot1,slot2);
	case SWAP:
		printf("Computer %s SWAP %d %d",    computersName,slot1,slot2);
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
	int type, slot1, slot2;
	unhashMove(move, &type, &slot1, &slot2);

	switch (type) {
	case NW:
		printf("[%d %d NW]",slot1,slot2);
		break;
	case NE:
		printf("[%d %d NE]",slot1,slot2);
		break;
	case E:
		printf("[%d %d E]", slot1,slot2);
		break;
	case SE:
		printf("[%d %d SE]",slot1,slot2);
		break;
	case SW:
		printf("[%d %d SW]",slot1,slot2);
		break;
	case W:
		printf("[%d %d W]", slot1,slot2);
		break;
	case SWAP:
		printf("[%d %d]", slot1,slot2);
		break;
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
		printf("%8s's move [(undo)/slot1 slot2 [type]] : ", playersName);

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
**  POSITION
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

	int i = 0, slot1, slot2;
	char *slot2s, *moveTypes = NULL;
	BOOLEAN toReturn = TRUE;

	// slot1 within range
	toReturn = toReturn && (slot1 = atoi(input)) < boardSize && slot1 >= 0;

	while (input[i]!= ' ') {
		i++;
	}
	i++;
	slot2s = input + i;

	// slot2 within range
	toReturn = toReturn && (slot2 = atoi(slot2s)) < boardSize && slot2 >= 0;

	while (input[i] != ' ' && input[i] != '\0') {
		i++;
	}

	// slot1 and slot2 are neighbors
	toReturn = toReturn && NEIGHBORING_DIRECTION(slot1, slot2) != -1;

	moveTypes = input + i;

	// lowercase everything
	while (input[i] != '\0') {
		if (input[i] >= 'A' && input[i] <= 'Z') {
			input[i] -= 'A';
		}
		i++;
	}

	toReturn = toReturn &&
	           //    (strstr(moveTypes, "swap") ||
	           (moveTypes[0] == '\0' ||
	            strstr(moveTypes, "sw")   || strstr(moveTypes, "se") ||
	            strstr(moveTypes, "nw")   || strstr(moveTypes, "ne") ||
	            strstr(moveTypes, "w")    || strstr(moveTypes, "e"));

	return toReturn;
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

	int i = 0, slot1, slot2, moveType = INVALID, temp;
	char *slot2s, *moveTypes = NULL;

	// slot1
	slot1 = atoi(input);

	while (input[i]!= ' ') {
		i++;
	}
	i++;
	slot2s = input + i;

	// slot2
	slot2 = atoi(slot2s);

	// Swap the slots so that the lower-valued one comes first
	if (slot2 < slot1) {
		temp = slot1;
		slot1 = slot2;
		slot2 = temp;
	}

	while (input[i] != ' ' && input[i] != '\0') {
		i++;
	}

	// type
	moveTypes = input + i;

	while (input[i] != '\0') {
		if (input[i] >= 'A' && input[i] <= 'Z') {
			input[i] -= 'A';
		}
		i++;
	}

	//  if (strstr(moveTypes, "swap")) {
	if (moveTypes[0] == '\0') {
		moveType = SWAP;
	} else if (strstr(moveTypes, "sw")) {
		moveType = SW;
	} else if (strstr(moveTypes, "se")) {
		moveType = SE;
	} else if (strstr(moveTypes, "nw")) {
		moveType = NW;
	} else if (strstr(moveTypes, "ne")) {
		moveType = NE;
	} else if (strstr(moveTypes, "w")) {
		moveType = W;
	} else if (strstr(moveTypes, "e")) {
		moveType = E;
	} else {
		// Should not fall through to here
		// BadElse("ConvertTextInputToMove");
		return INVALID;
	}

	return hashMove(moveType, slot1, slot2);
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
	char choice;
	BOOLEAN validInput = FALSE;

	while (!validInput) {
		printf("\t---- mparadux Option Menu ---- \n\n");
		printf("\tU)\tToggle Allow (U)ndoing Previous Move: Currently %s\n\n", prevMoveAllowed ? "ALLOWED" : "DISALLOWED");

		printf("Select an option: ");
		//fflush(stdin); // This should only work on windows, flushing input is an ambiguous concept
		//flush();
		choice = readchar();
		choice = toupper(choice);
		switch(choice) {
		case 'U':
			printf("\nUndoing the previous move is now %s [Press ENTER]: \n", prevMoveAllowed ? "DISALLOWED" : "ALLOWED");
			readchar();
			prevMoveAllowed = !prevMoveAllowed;
			InitializeGame( );
			validInput = TRUE;
			break;
		default:
			printf("Not a valid option.\n");
			break;
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
	return 0;
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
	return NUM_OF_OPTIONS;
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

// Have to rewrite un/hashMove so that it doesn't take up so much space
MOVE hashMove (int type, int slot1, int slot2)
{
	int nDirection = NEIGHBORING_DIRECTION(slot1, slot2);

	return HASH_MOVE(type, slot1, nDirection);
}

void unhashMove (MOVE move, int* type, int* slot1, int* slot2) {
	*type = UNHASH_MOVE_TYPE(move);
	*slot1 = UNHASH_MOVE_SLOT(move);
	*slot2 = GET_NEIGHBOR(*slot1, UNHASH_MOVE_NDIR(move));
}

POSITION paraduxHashInit(int boardSize, int* pieces_array, int (*fn)(int *)) {
	numGenericPositions = generic_hash_init(boardSize, pieces_array, fn, 0);

	if (prevMoveAllowed) {
		return numGenericPositions;
	} else {
		return maxNumMoves * numGenericPositions;
	}

	//  return boardSize * NUM_DIRS * NUM_DIRS * numGenericPositions;
}

POSITION paraduxHash(char* board, MOVE lastMoveHash, int player) {
	// First, get the generic hash
	POSITION genericPart = generic_hash_hash(board, player);

	if (!prevMoveAllowed) {
		// Then, enumerate the moves
		MOVELIST *moves = GenerateMovesFromBoard(board, INVALID);
		MOVELIST *movesCopy = moves;

		// Find the index of the move that undoes the last move
		int index = 0;
		for (; moves != NULL && moves->move != lastMoveHash; moves = moves->next, index++) ;

		if (moves == NULL) {
			index = maxNumMoves - 1;
		}

		//  printf("paraduxHash: genericPart = %d, index = %d\n", (int)genericPart, index);
		SafeFree(movesCopy);

		return index * numGenericPositions + genericPart;
		//  return genericPart;
		//  return lastMoveHash * numGenericPositions + genericPart;

	} else { // prevMoveAllowed
		return genericPart;
	}
}

char* paraduxUnhash(POSITION hashed, char* dest, MOVE* lastMoveHash) {
	// First, get the index
	int index = hashed / numGenericPositions;

	// Get the board that corresponds to the generic part of the position
	generic_hash_unhash(hashed % numGenericPositions, dest);

	if (!prevMoveAllowed) {

		// Then, enumerate the moves from the board
		MOVELIST *moves = GenerateMovesFromBoard(dest, INVALID);
		MOVELIST *movesCopy = moves;

		// Finally, find the lastMoveHash using the index
		int i = 0;
		for (; i < index && moves != NULL; moves = moves->next, i++) ;

		//  printf("paraduxUnhash: genericPart = %d, index = %d\n", (int)(hashed % numGenericPositions), index);

		if (moves == NULL) {
			*lastMoveHash = INVALID;
		} else {
			*lastMoveHash = moves->move;
		}

		SafeFree(movesCopy);
		//  *lastMoveHash = hashed / numGenericPositions;

		//  generic_hash_unhash(hashed % numGenericPositions, dest);

		//  *lastMoveHash = -1;
		//  generic_hash_unhash(hashed, dest);

	} else { // prevMoveAllowed
		*lastMoveHash = INVALID;
	}

	return dest;
}

/************************************************************************
**
**  SUPPORT FUNCTIONS
**
************************************************************************/

char readchar( ) {

	while( getchar( ) != '\n' ) ;
	return getchar( );

}


// if there is no neigher in the specified direction, return INVALID
// returns a board index b/n 0 and boardSize
int getNeighbor(int slot, int direction) {
	int pRow, pCol;

	slotToRC(slot, &pRow, &pCol);

	if (pRow < boardSide - 1) {
		// if the slot in the top half of the board

		switch (direction) {
		case NW:
			return rcToSlot(pRow - 1, pCol - 1);
		case NE:
			return rcToSlot(pRow - 1, pCol);
		case SE:
			return rcToSlot(pRow + 1, pCol + 1);
		case SW:
			return rcToSlot(pRow + 1, pCol);
		case W:
			return rcToSlot(pRow, pCol - 1);
		case E:
			return rcToSlot(pRow, pCol + 1);
		default:
			return INVALID;
		}
	} else if (pRow > boardSide - 1) {
		// if the slot in the bottom half of the board

		switch (direction) {
		case NW:
			return rcToSlot(pRow - 1, pCol);
		case NE:
			return rcToSlot(pRow - 1, pCol + 1);
		case SE:
			return rcToSlot(pRow + 1, pCol);
		case SW:
			return rcToSlot(pRow + 1, pCol - 1);
		case W:
			return rcToSlot(pRow, pCol - 1);
		case E:
			return rcToSlot(pRow, pCol + 1);
		default:
			return INVALID;
		}
	} else {
		// if the slot in the middle row of the board

		switch (direction) {
		case NW:
			return rcToSlot(pRow - 1, pCol - 1);
		case NE:
			return rcToSlot(pRow - 1, pCol);
		case SE:
			return rcToSlot(pRow + 1, pCol);
		case SW:
			return rcToSlot(pRow + 1, pCol - 1);
		case W:
			return rcToSlot(pRow, pCol - 1);
		case E:
			return rcToSlot(pRow, pCol + 1);
		default:
			return INVALID;
		}
	}

	// should never fall through here
	BadElse("getNeighbor");

	return INVALID;
}

/* Direction in which the neighbor neighbors slot:
   returns one of
   0 NW
   1 NE
   2 E
   3 SE
   4 SW
   5 W
   -1 Not neighboring
 */
int neighboringDirection(int slot, int neighbor) {
	int pCol, pRow;
	int nCol, nRow;

	slotToRC(slot, &pRow, &pCol);
	slotToRC(neighbor, &nRow, &nCol);

	int dCol = nCol - pCol,
	    dRow = nRow - pRow;

	if (pRow < boardSide - 1) {
		// if the first slot is in the top half of the board

		switch (dRow) {
		case -1:
			switch (dCol) {
			case -1:
				return NW;
			case 0:
				return NE;
			default:
				return INVALID;
			}
			break;
		case 1:
			switch (dCol) {
			case 1:
				return SE;
			case 0:
				return SW;
			default:
				return INVALID;
			}
			break;
		case 0:
			switch (dCol) {
			case -1:
				return W;
			case 1:
				return E;
			default:
				return INVALID;
			}
			break;
		default:
			return INVALID;
		}
	} else if (pRow > boardSide - 1) {
		// if the first slot is in the bottom half of the board

		switch (dRow) {
		case -1:
			switch (dCol) {
			case 1:
				return NE;
			case 0:
				return NW;
			default:
				return INVALID;
			}
			break;
		case 1:
			switch (dCol) {
			case -1:
				return SW;
			case 0:
				return SE;
			default:
				return INVALID;
			}
			break;
		case 0:
			switch (dCol) {
			case -1:
				return W;
			case 1:
				return E;
			default:
				return INVALID;
			}
			break;
		default:
			return INVALID;
		}
	} else {
		// if the first slot is in the middle row of the board

		switch (dRow) {
		case -1:
			switch (dCol) {
			case -1:
				return NW;
			case 0:
				return NE;
			default:
				return INVALID;
			}
			break;
		case 1:
			switch (dCol) {
			case -1:
				return SW;
			case 0:
				return SE;
			default:
				return INVALID;
			}
			break;
		case 0:
			switch (dCol) {
			case -1:
				return W;
			case 1:
				return E;
			default:
				return INVALID;
			}
			break;
		default:
			return INVALID;
		}
	}
}

// returns TRUE if square u,v is next to square x,y
// where u,v and x,y are the row,column coordinates
// returns FALSE for invalid coordinates
BOOLEAN neighbor(int u, int v, int x, int y) {

	// Hell, I don't even know what this means...
	//   - David
	return
	        // valid rows
	        (u>=0 && u<(2*boardSide-1) && x>=0 && x<(2*boardSide-1)) &&
	        // valid columns
	        (v>=0 && v<(u<boardSide ? boardSide+u : 2*boardSide - u%boardSide - 2)) &&
	        (y>=0 && y<(x<boardSide ? boardSide+x : 2*boardSide - x%boardSide - 2)) &&
	        // neighbor same row, two possible neighbors, two cases
	        (((u==x) && ((v==y-1) || (v==y+1))) ||
	         // neighbor different rows, four possible neighbors, four cases
	         (((u==x-1) || (u==x+1)) &&
      ((v==y) ||
       (x<(boardSide-1) ? ((u==x-1) && (v==y-1)) : ((u==x+1) && (v==y+1))) ||
       (x>(boardSide-1) ? ((u==x-1) && (v==y+1)) : ((u==x+1) && (v==y-1))) ||
       (x==(boardSide-1) && (v==y-1))
      )
	         ) // end neighbor different rows
	        ) // end all neighbors
	; // end return

}

// returns slot number when given row and column coordinates
// returns -1 when given invalid coordinates
int rcToSlot(int r, int c) {
	int x, y = boardSide, toReturn = 0;

	if (
	        // valid r
	        (r>=0 && r<(2*boardSide-1)) &&
	        // valid columns
	        (c>=0 && c<(r<boardSide ? boardSide+r : 2*boardSide - r%boardSide - 2))
	        ) {

		// for rows before r w/ increasing length
		for (x=0; x<min(boardSide-1,r); x++, toReturn += y++) ;
		// for rows before r w/ decreasing length
		for (; x<r; x++, toReturn += y--) ;
		// for columns in row = r
		return toReturn += c;

	} else {
		// invalid r,c
		return INVALID;
	}
}

void slotToRC(int slot, int* pRow, int* pCol) {
	int row = 0, colSize = boardSide, numEls = 0;

	if (slot >= boardSide) {
		*pRow = -1;
		*pCol = -1;
	}

	for (; slot >= numEls + colSize; row++, numEls += colSize, colSize += (row > boardSide - 1 ? -1 : 1)) ;

	*pRow = row;

	*pCol = slot - numEls;
}

int generic_hash_turnF(POSITION p) {
	return p;
}

////////////////////////////
//
//  TEST CODE
//
////////////////////////////

BOOLEAN tested = FALSE;

void runTests() {
	if (tested)
		return;

	printf("Running a series of tests on Paradux code...\n");

	printf(" + Testing slotToRC and rcToSlot...\n");
	int i, row, col;

	for (i = 0; i < boardSize; i++) {
		slotToRC(i, &row, &col);
		printf("   - slotToRC(%d) returns (%d, %d)\n", i, row, col);
		printf("   - (SLOT_TO_ROW(%d), SLOT_TO_COL(%d)) returns (%d, %d)\n", i, i, SLOT_TO_ROW(i), SLOT_TO_COL(i));

		int slot = rcToSlot(row, col);

		printf("   - rcToSlot(%d, %d) returns %d\n", row, col, slot);
		printf("   - RC_TO_SLOT(%d, %d) returns %d\n", row, col, RC_TO_SLOT(row, col));

		if (slot != i) {
			printf("   ! ERROR: slotToRC and rcToSlot do not have matching output!!\n");
		}
	}

	printf("\n\n + Testing getNeighbor...\n");
	for (i = 0; i < boardSize; i++) {
		int j;
		for (j = 0; j <= MAX_DIR; j++) {
			int nSlot = getNeighbor(i, j);

			printf("   - getNeighbor(%d, %d) returns %d\n", i, j, nSlot);
			printf("   - GET_NEIGHBOR(%d, %d) returns %d\n", i, j, GET_NEIGHBOR(i, j));

			if (nSlot >= 0) {
				int dir = neighboringDirection(i, nSlot);

				printf("   - neighboringDirection(%d, %d) returns %d\n", i, nSlot, dir);
				printf("   - NEIGHBORING_DIRECTION(%d, %d) returns %d\n", i, nSlot, NEIGHBORING_DIRECTION(i, nSlot));

				if (dir != j) {
					printf("   ! ERROR: getNeighbor and neighboringDirection do not have matching output!!\n");
				}
			}
		}
	}

	tested = TRUE;
}

// $Log: not supported by cvs2svn $
// Revision 1.23  2006/10/17 10:45:21  max817
// HUGE amount of changes to all generic_hash games, so that they call the
// new versions of the functions.
//
// Revision 1.22  2006/01/03 00:19:35  hevanm
// Added types.h. Cleaned stuff up a little. Bye bye gDatabase.
//
// Revision 1.21  2005/12/19 10:06:23  trikeizo
// Fixed first player always wins.
//
// Revision 1.20  2005/12/07 10:50:19  yanpeichen
// ** 12/07/2005 Yanpei - Added prevMoveAllowed as game option
// **                     Coded brief game options menu
// **                     Fixed some memory leaks
// **                     Changed SWAP move input to "", i.e. no need to type "swap"
// **                     All changes untested.
//
// Revision 1.19  2005/11/23 03:29:33  trikeizo
// Now, everything fits 2^31 but the solver still tries to malloc about 2 gigs. My computer can't handle that. On the bright side, you now cannot undo the last move :-). See the file for more changes.
//
// Revision 1.18  2005/11/22 09:32:44  trikeizo
// Added support for the rule that a player cannot undo his opponent's move. This brings the number of positions to ~37 billion. Boo.
//
// Revision 1.17  2005/11/22 03:45:01  trikeizo
// Added help text and cleaned up the code.
//
// Revision 1.16  2005/11/19 09:24:57  trikeizo
// Compiling and solving--but still some issues to resolve with the code.
//
// Revision 1.15  2005/11/18 08:35:35  trikeizo
// Everything compiles. Runs without solving; crashes when solving.
//
// Revision 1.14  2005/11/13 07:38:14  yanpeichen
// *** empty log message ***
//
// Revision 1.13  2005/11/12 07:39:33  yanpeichen
// *** empty log message ***
//
// Revision 1.12  2005/11/11 08:52:20  yanpeichen
// *** empty log message ***
//
// Revision 1.11  2005/11/09 10:10:12  yanpeichen
// brief fixes to prev commit
//
// Revision 1.10  2005/11/09 10:01:11  yanpeichen
// ** 11/09/2005 Yanpei - DoMove proof read, reasonably confident,
// **                     need to write getNeighbor,
// **                     yanpeiTestBeighboringDir() written but yet to run.
// **                     Primitive proof read, reasonably confident
// **                     Wrote PrintMove + PrintComputersMove
// **                     Still to do: GenerateMoves + test UI functions
//
// Revision 1.9  2005/10/26 09:37:54  yanpeichen
// yanpei: debugged printPos and initGamme for odd boards
//
// Revision 1.8  2005/10/18 08:34:01  yanpeichen
// ** 10/18/2005 Yanpei - Tidied up davidPrintPos(), wrote dyPrintPos() using
// **                     more tidy code. Proof read hashMove() unhashMove()
// **final fantasy ix
//
// Revision 1.7  2005/10/05 03:23:55  trikeizo
// Added a bunch of small functions, untested.
//
// Revision 1.6  2005/09/28 06:17:18  yanpeichen
// *** empty log message ***
//
// Revision 1.5  2005/09/28 05:27:24  trikeizo
// See change log.
//
// Revision 1.4  2005/09/28 04:54:03  yanpeichen
// 09/27/2005 Yanpei - Some support functions and data structs
//
// Revision 1.3  2005/09/28 04:24:24  yanpeichen
// *** empty log message ***
//
// Revision 1.2  2005/09/26 08:09:43  yanpeichen
// yanpei: additional comments
//
// Revision 1.1  2005/09/14 19:57:44  yanpeichen
// David Chen, Yanpei Chen modifying mparadux.c
//
// First revision
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
