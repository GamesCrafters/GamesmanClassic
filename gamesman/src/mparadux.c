// $Id: mparadux.c,v 1.16 2005-11-19 09:24:57 trikeizo Exp $

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
**
**************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**NUM_DIRECTIONS
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

STRING   kGameName            = "Paradux"; /* The name of your game */
STRING   kAuthorName          = "David Chen, Yanpei Chen"; /* Your name(s) */
STRING   kDBName              = "Paradux"; /* The name to store the database under */

BOOLEAN  kPartizan            = FALSE ; /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN  kGameSpecificMenu    = FALSE ; /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN  kTieIsPossible       = FALSE ; /* TRUE if a tie is possible. FALSE if it is impossible.*/
BOOLEAN  kLoopy               = TRUE ; /* TRUE if the game tree will have cycles (a rearranger style game). FALSE if it does not.*/

BOOLEAN  kDebugMenu           = TRUE ; /* TRUE only when debugging. FALSE when on release. */
BOOLEAN  kDebugDetermineValue = TRUE ; /* TRUE only when debugging. FALSE when on release. */

POSITION gNumberOfPositions   =  0; /* The number of total possible positions | If you are using our hash, this is given by the hash_init() function*/
POSITION gInitialPosition     =  0; /* The initial hashed position for your starting board */
POSITION kBadPosition         = -1; /* A position that will never be used */

void*	 gGameSpecificTclInit = NULL;

/* 
 * Help strings that are pretty self-explanatory
 * Strings than span more than one line should have backslashes (\) at the end of the line.
 */

STRING kHelpGraphicInterface =
"Not written yet";

STRING   kHelpTextInterface    =
"";

STRING   kHelpOnYourTurn =
"Choose one of your pieces and one of your opponent's pieces. Then choose a direction to move in or SWAP to swap the pieces."; 

STRING   kHelpStandardObjective =
"On a board with side length N, to line N of your pieces in a row in any direction.";

STRING   kHelpReverseObjective =
"On a board with side length N, to line N of your opponent's pieces in a row in any direction.";

STRING   kHelpTieOccursWhen =
"A tie can never occur";

STRING   kHelpExample =
"";

/* macros */
//#define nextPlayer(x) (((x) - 1) % 2 + 1)

#define nextPlayer(x) ((x) % 2 + 1)

/* pieces */
#define WHITE 1
#define BLACK 2
#define BLANK 0
#define X 1
#define O 2
#define INVALID -1 
// INVALID used by getNeighbor(),
// so must not be valid return value of getNeighbor()

/* directions */
/* #define NW   0 */
/* #define NE   1 */
/* #define E    2 */
/* #define SE   3 */
/* #define SW   4 */
/* #define W    5 */
/* #define SWAP 6 */
/* #define INVALID -1 */

// new directions so you can do something like MAX_DIR - DIR_A to get the opposite direction

#define NW       0
#define NE       1
#define E        2
#define W        3
#define SW       4
#define SE       5
#define SWAP     6
#define INVALID -1

#define MAX_DIR 5
#define NUM_DIRS 6
#define NUM_MOVE_TYPES 7

/*************************************************************************
**
** Game-specific variables
**
**************************************************************************/


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

/* MOVE encoding: (position << 6) | (direction << 3) | type */

/* On the hexagonal board, only one side needs to be specified */
int boardSide = 3;

#define maxDim (boardSide * 2 - 1)
//int maxDim = boardSide * 2 - 1;

/* Magically generated (in InitializeGame) */
int boardSize;
int numX;
int numO;
int numBlank;

int posStrLength;

/* Other options */
int firstGo = X;

typedef struct board_item {

  short *slots;
  short turn;

} PARABOARD;

typedef PARABOARD* PBPtr;

// must be used on arithmetic expressions and comparables
// "x" and "y" should not contain side effects
#define max(x,y) ((x)>(y) ? (x) : (y))
#define min(x,y) ((x)<(y) ? (x) : (y))

// must be used on ints or POSITION or the like
#define abs(x) (0<(x) ? (x) : -(x))

#define HASH_MOVE(type, pos, nDir) ((type) * boardSize * NUM_DIRS + (pos) * NUM_DIRS + (nDir))
#define UNHASH_MOVE_TYPE(move) ((move) / (boardSize * NUM_DIRS))
#define UNHASH_MOVE_POS(move) ((move) % (boardSize * NUM_DIRS) / NUM_DIRS)
#define UNHASH_MOVE_NDIR(move) ((move) % (boardSize * NUM_DIRS) % NUM_DIRS)

/*************************************************************************
**
** Global Variables
**
*************************************************************************/


/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/

/* External */
extern GENERIC_PTR	SafeMalloc ();
extern void		SafeFree ();

/* Multiple Implementations */

void                    davidInitGame ();
void                    dyPrintPos(POSITION position, STRING playersName, BOOLEAN usersTurn);

void                    (*initGame)( ) = &davidInitGame;
void                    (*printPos)(POSITION position, STRING playersName, BOOLEAN usersTurn) = &dyPrintPos;

/* Support Functions */

PBPtr                   MallocBoard();
void                    FreeBoard(PBPtr);
MOVE                    hashMove(int type, int pos1, int pos2);
void                    unhashMove(MOVE move, int* type, int* pos1, int* pos2);

// returns TRUE if square u,v is next to square x,y
// where u,v and x,y are the row,column coordinates
// returns FALSE for invalid coordinates
BOOLEAN                 neighbor(int u, int v, int x, int y);

// if there is no neigher in the specified direction, return INVALID
// returns a board index b/n 0 and boardSize
int                     getNeighbor(int pos, int direction);
int* getNeighborCache;
#define GET_NEIGHBOR(pos, dir) getNeighborCache[(pos) * NUM_DIRS + (dir)]

/* Direction in which the neighbor neighbors pos:
  returns one of 
  0 NW
  1 NE
  2 E
  3 SE
  4 SW
  5 W
 -1 Not neighboring
*/

int                     neighboringDirection(int pos, int neighbor);
int* neighboringDirectionCache;
#define NEIGHBORING_DIRECTION(pos, neighbor) neighboringDirectionCache[(pos) * boardSize + (neighbor)]

// returns slot number when given row and column coordinates
// returns -1 when given invalid coordinates
int                     rcToSlot(int r, int c);
int* rcToSlotCache;
#define RC_TO_SLOT(r, c) rcToSlotCache[(r) * maxDim + (c)]

// returns row and column coordinate when given slot number
// returns -1 when given invalid slot number
// return data format = r*100+c
int                     slotToRC(int s);

void                    getColRow(int pos, int* pCol, int* pRow);

// exact same thing as getColRow but in the more standard row, col format
void                    getRowCol(int pos, int* pRow, int* pCol);
int* getRowColCache;
#define GET_ROW(pos) getRowColCache[2 * (pos)]
#define GET_COL(pos) getRowColCache[2 * (pos) + 1]

int                     whoseMoveF(POSITION p);

/* Test Functions */

void                    yanpeiTestNeighboringDir();

/*************************************************************************
**
** Global Database Declaration
**
**************************************************************************/

extern VALUE     *gDatabase;


/************************************************************************
**
** NAME:        InitializeGamedavidInitGame
**
** DESCRIPTION: Prepares the game for execution.
**              Initializes required variables.
**              Sets up gDatabase (if necessary).
** 
************************************************************************/

int outstandingMallocs = 0;

GENERIC_PTR debugMalloc(size_t amt) {
  outstandingMallocs++;

  if (outstandingMallocs > 2) {
    printf("*** Warning: More than 2 unfreed mallocs exist: %d\n\n", outstandingMallocs);
  }

  return SafeMalloc(amt);
}

void debugFree(GENERIC_PTR ptr) {
  outstandingMallocs--;

  SafeFree(ptr);
}

BOOLEAN tested = FALSE;

void runTests() {
  if (tested)
    return;

  printf("Running a series of tests on Paradux code...\n");

  printf(" + Testing getRowCol and rcToSlot...\n");
  int i, row, col;

  for (i = 0; i < boardSize; i++) {
    getRowCol(i, &row, &col);
    printf("   - getRowCol(%d) returns (%d, %d)\n", i, row, col);
    //    GET_ROW(i) = row;
    //    GET_COL(i) = col;
    printf("   - (GET_ROW(%d), GET_COL(%d)) returns (%d, %d)\n", i, i, GET_ROW(i), GET_COL(i));

    int slot = rcToSlot(row, col);

    printf("   - rcToSlot(%d, %d) returns %d\n", row, col, slot);
    printf("   - RC_TO_SLOT(%d, %d) returns %d\n", row, col, RC_TO_SLOT(row, col));

    if (slot != i) {
      printf("   ! ERROR: getRowCol and rcToSlot do not have matching output!!\n");
    }
  }

  printf("\n\n + Testing getNeighbor...\n");
  for (i = 0; i < boardSize; i++) {
    int j;
    for (j = 0; j <= MAX_DIR; j++) {
      int nPos = getNeighbor(i, j);

      printf("   - getNeighbor(%d, %d) returns %d\n", i, j, nPos);
      printf("   - GET_NEIGHBOR(%d, %d) returns %d\n", i, j, GET_NEIGHBOR(i, j));

      if (nPos >= 0) {
	int dir = neighboringDirection(i, nPos);

	printf("   - neighboringDirection(%d, %d) returns %d\n", i, nPos, dir);
	printf("   - NEIGHBORING_DIRECTION(%d, %d) returns %d\n", i, nPos, NEIGHBORING_DIRECTION(i, nPos));

	if (dir != j) {
	  printf("   ! ERROR: getNeighbor and neighboringDirection do not have matching output!!\n");
	}
      }
    }
  }

  tested = TRUE;
}

void InitializeGame ()
{
  //printf("testpoint1\n");
  initGame();
  //printf("testpoint2\n");

  runTests();
}

/* Initialize odd-sided board */
void initOddBoard() {
  int row, col, el = 0, maxCol = boardSide - 1, maxRow = boardSide * 2 - 2;

  char* board = (char *) SafeMalloc (sizeof(char) * boardSize);


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
      } else { // (row == boardSide-1)
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
  /*
  for (i=0; i<boardSize; i++) {
    printf("%c",board[i]);
  }
  printf("\n");
  */

  gInitialPosition = generic_hash(board, firstGo);
  SafeFree(board);
}

/* Initialize even-sided board */
void initEvenBoard() {
  int row, col, el = 0, maxCol = boardSide - 1, maxRow = boardSide * 2 - 2;

  char* board = (char *) SafeMalloc (sizeof(char) * boardSize);

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
	} else {    //    getRowColCache[2 * pos] = row;
    //    getRowColCache[2 * pos + 1] = col;

    //    printf("   - initCaches: getRowCol(%d) returns (%d, %d)\n", pos, row, col);
    //    GET_ROW(i) = row;
    //    GET_COL(i) = col;
    //    printf("   - initCaches: (GET_ROW(%d), GET_COL(%d)) returns (%d, %d)\n", pos, pos, GET_ROW(pos), GET_COL(pos));

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

  gInitialPosition = generic_hash(board, firstGo);
  SafeFree(board);
}

void initCaches ()
{
  int pos, row, col, neighbor, dir;

  // First, the getRowColCache
  getRowColCache = (int *) SafeMalloc (sizeof(int) * boardSize * 2);

  for (pos = 0; pos < boardSize; pos++) {
    getRowCol(pos, &row, &col);

    GET_ROW(pos) = row;
    GET_COL(pos) = col;
  }

  for (pos = 0; pos < boardSize; pos++) {
    getRowCol(pos, &row, &col);
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

  for (pos = 0; pos < boardSize; pos++) {
    for (neighbor = 0; neighbor < boardSize; neighbor++) {
      NEIGHBORING_DIRECTION(pos, neighbor) = neighboringDirection(pos, neighbor);
    }
  }

  // And, finally, getNeighborCache
  getNeighborCache = (int *) SafeMalloc (sizeof(int) * boardSize * NUM_DIRS);

  for (pos = 0; pos < boardSize; pos++) {
    for (dir = 0; dir < NUM_DIRS; dir++) {
      GET_NEIGHBOR(pos, dir) = getNeighbor(pos, dir);
    }
  }
}

void freeCaches ()
{
  SafeFree(getRowColCache);
  SafeFree(rcToSlotCache);
  SafeFree(neighboringDirectionCache);
  SafeFree(getNeighborCache);
}

void davidInitGame ()
{
  int pieces[10];

  if (boardSide < 3) {
    printf("ERROR: boards with side length < 3 cannot be represented or are trivial");
    exit(1);
  }

  /* 3(n-1)n + 1... I think */
  boardSize = 3 * (boardSide - 1) * boardSide + 1;

  int posStrL = 1, temp = boardSize;
  while ((temp = temp / 10) >= 10) {
    posStrL++;
  }
  posStrLength = posStrL;

  //printf("%d\n", boardSize);

  /* boards with side length 1 don't follow this but we'll ignore that */
  if (boardSide % 2) { // odd board
    numX = numO = 2*boardSide - 1;
  } else { // even board
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

  //printf("testpoint4 %d %d\n",boardSize,numX);
  /*
  int i;
  for (i=0; i<10; i++)
    printf("%d ",pieces[i]);
  printf("\n");
  */

  gNumberOfPositions = generic_hash_init(boardSize, pieces, NULL);

  printf("gNumberOfPositions = %d\n", (int)gNumberOfPositions);

  //printf("testpoint3\n");

  if (boardSide % 2) {
    initOddBoard();
  } else {
    initEvenBoard();
  }

  // Due to horrible performance, we cache all of the position lookup code
  initCaches();

  //printf("%d\n", gInitialPosition);

  /*
  printf("%d %s\n",atoi("01"),"01");
  printf("%d %s\n",atoi(" 10")," 10");
  printf("%d %s\n",atoi("10 10"),"10 10");
  printf("%d %s\n",atoi("10s"),"10s");
  printf("%d %s\n",atoi("s"),"s");
  */

  //PrintPosition(gInitialPosition, playersName, TRUE);
  //yanpeiTestNeighboringDir();
  //yanpeiTestRcToSlot();
  

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

// Returns true if the two pieces can move in the given direction
BOOLEAN isOpen(char* board, int direction, int pos1, int pos2) {
  //  int p1Top2 = neighboringDirection(pos1, pos2);
  int p1Top2 = NEIGHBORING_DIRECTION(pos1, pos2);
  int p2Top1 = MAX_DIR - p1Top2;

  //  int p1Neighbor = getNeighbor(pos1, direction),
  //    p2Neighbor = getNeighbor(pos2, direction);

  int p1Neighbor = GET_NEIGHBOR(pos1, direction),
    p2Neighbor = getNeighbor(pos2, direction);

  if (direction == p1Top2) {
    return p2Neighbor != INVALID && board[p2Neighbor] == '-';
  } else if (direction == p2Top1) {
    return p1Neighbor != INVALID && board[p1Neighbor] == '-';
  } else {
    return p1Neighbor != INVALID && board[p1Neighbor] == '-' && p2Neighbor != INVALID && board[p2Neighbor] == '-';
  }

  // should never fall through to here
  return FALSE;
}

void AddMovesPerPair(MOVELIST **moves, char *board, int pos1, int pos2) {
  //  int r1 = 0, c1 = 0,
  //    r2 = 0, c2 = 0;

  //  printf("  - Adding moves for pair %d and %d\n", pos1, pos2);

  //  getRowCol(pos1, &r1, &c1);

  //  getRowCol(pos2, &r2, &c2);

  //  printf("Adding moves for pair (%d, %d) and (%d, %d)\n", r1, c1, r2, c2);
  // Note that we will have to take care of not being able to undo a move here

  int lastMoveHash = -1;

  int curHash;

  // Swapping pieces requires little logic
  curHash = hashMove(SWAP, pos1, pos2);
  if (curHash != lastMoveHash) {
    //    printf("     * AddMovesPerPair - Adding swap move\n");
    *moves = CreateMovelistNode(curHash, *moves);
  }

  int i;

  // Loop over each of the directions
  for (i = 0; i < NUM_DIRS; i++) {
    if (isOpen(board, i, pos1, pos2)) {
      curHash = hashMove(i, pos1, pos2);

      if (curHash != lastMoveHash) {
	//	printf("     * AddMovesPerPair - Adding move in direction %d\n", i);
	*moves = CreateMovelistNode(curHash, *moves);
      }
    }
  }
}
  /* Yanpei's AddMovesPerPair
MOVELIST *AddMovesPerPair(char *board, int pos1, int pos2, MOVELIST *moveL) {
  // can assume GenerateMoves is correct, and pos1,pos2 are neighboring

  int target1, target2;

  // can always SWAP
  moveL = CreateMovelistNode(hashMove(SWAP,pos1,pos2), moveL);

  // check NW
  if (((target1 = getNeighbor(pos1,NW)) != INVALID) &&
      ((target2 = getNeighbor(pos2,NW)) != INVALID) &&
      (board[target1] == '-' || target1 == pos2) && 
      (board[target2] == '-' || target2 == pos1)) {
    moveL = CreateMovelistNode(hashMove(NW,pos1,pos2), moveL);
  }

  // check NE
  if (((target1 = getNeighbor(pos1,NE)) != INVALID) &&
      ((target2 = getNeighbor(pos2,NE)) != INVALID) &&
      (board[target1] == '-' || target1 == pos2) && 
      (board[target2] == '-' || target2 == pos1)) {
    moveL = CreateMovelistNode(hashMove(NE,pos1,pos2), moveL);
  }

  // check E
  if (((target1 = getNeighbor(pos1,E)) != INVALID) &&
      ((target2 = getNeighbor(pos2,E)) != INVALID) &&
      (board[target1] == '-' || target1 == pos2) && 
      (board[target2] == '-' || target2 == pos1)) {
    moveL = CreateMovelistNode(hashMove(E,pos1,pos2), moveL);
  }

  // check SE
  if (((target1 = getNeighbor(pos1,SE)) != INVALID) &&
      ((target2 = getNeighbor(pos2,SE)) != INVALID) &&
      (board[target1] == '-' || target1 == pos2) && 
      (board[target2] == '-' || target2 == pos1)) {
    moveL = CreateMovelistNode(hashMove(SE,pos1,pos2), moveL);
  }

  // check SW
  if (((target1 = getNeighbor(pos1,SW)) != INVALID) &&
      ((target2 = getNeighbor(pos2,SW)) != INVALID) &&
      (board[target1] == '-' || target1 == pos2) && 
      (board[target2] == '-' || target2 == pos1)) {
    moveL = CreateMovelistNode(hashMove(SW,pos1,pos2), moveL);
  }

  // check W
  if (((target1 = getNeighbor(pos1,W)) != INVALID) &&
      ((target2 = getNeighbor(pos2,W)) != INVALID) &&
      (board[target1] == '-' || target1 == pos2) && 
      (board[target2] == '-' || target2 == pos1)) {
    moveL = CreateMovelistNode(hashMove(W,pos1,pos2), moveL);
  }

  */

MOVELIST *GenerateMoves (POSITION position)
{
  MOVELIST *moves = NULL;
    
  int i;
  char *board = SafeMalloc(sizeof(char) * boardSize);
  int pos, nPos;//, whoseMove = whoseMoveF(position);
  //char pieceA;
  char piece;
  //char player = valToChar[whoseMove], otherPlayer = valToChar[nextPlayer(whoseMove)];

  /* Use CreateMovelistNode(move, next) to 'cons' together a linked list */
  /* have AddMovesPerPair() call CreateMoveListNode() */

  //  printf("testpoint GenerateMoves");

  generic_unhash(position, board);

  for (i = 0; i < boardSize; i++) {
    pos = i;

    piece = board[i];

    char otherPiece = (piece == valToChar[X] ? valToChar[O] : valToChar[X]);

    if (piece != '-') {
      //      printf("Checking piece at location %d...\n", i);

      // Check east
      //      nPos = getNeighbor(i, E);
      nPos = GET_NEIGHBOR(i, E);
      if (nPos >= 0 && board[nPos] == otherPiece) {
	AddMovesPerPair(&moves, board, pos, nPos);
      }

      // Check southwest
      //      nPos = getNeighbor(i, SW);
      nPos = GET_NEIGHBOR(i, SW);
      if (nPos >= 0 && board[nPos] == otherPiece) {
	AddMovesPerPair(&moves, board, pos, nPos);
      }

      // Check southeast
      //      nPos = getNeighbor(i, SE);
      nPos = GET_NEIGHBOR(i, SE);
      if (nPos >= 0 && board[nPos] == otherPiece) {
	AddMovesPerPair(&moves, board, pos, nPos);
      }

      // No need to check W NE or NW because we start from pos = 0 forwards
      // So any W NE or NW should have been found by E SW or SE earlier. 

    }
  }

  SafeFree(board);

  return moves;

  //  return NULL;
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
  int type, pos1, pos2, target1, target2, temp1, temp2;

  char *board = SafeMalloc(sizeof(char) * boardSize);

  unhashMove(move, &type, &pos1, &pos2);
  generic_unhash(position, board);

  if (type == SWAP) {
    temp1 = board[pos1];
    board[pos1] = board[pos2];
    board[pos2] = temp1;
  } else {
    //    target1 = getNeighbor(pos1, type);
    //    target2 = getNeighbor(pos2, type);

    target1 = GET_NEIGHBOR(pos1, type);
    target2 = GET_NEIGHBOR(pos2, type);

    temp1 = board[pos1];
    temp2 = board[pos2];
    board[pos1] = valToChar[BLANK];
    board[pos2] = valToChar[BLANK];
    board[target1] = temp1;
    board[target2] = temp2;
  }

  int hashVal = generic_hash(board, nextPlayer(whoseMove(position)));
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
  int pos, curPos;
  char piece, curPiece, playersPiece = valToChar[whoseMove(position)];
  int i,j;

  generic_unhash(position, board);

  for (i = 0; i < boardSize; i++) {
    pos = i;

    piece = board[i];

    if (piece != '-') {

      // Check east
      curPos = pos;

      for (j = 0; j < boardSide - 1; j++) {
	//	if ((curPos = getNeighbor(curPos, E)) == INVALID)
	if ((curPos = GET_NEIGHBOR(curPos, E)) == INVALID)
	  break;

	curPiece = board[curPos];

	if (piece != curPiece)
	  break;

	if (j == boardSide - 2) {
	  printf("for player %c, %s on east\n", playersPiece, playersPiece == piece ? "win" : "lose");
	  SafeFree(board);
	  return playersPiece == piece ? win : lose;
	}
      }

      // Check southeast
      curPos = pos;

      for (j = 0; j < boardSide - 1; j++) {
	//	if ((curPos = getNeighbor(curPos, SE)) == INVALID)
	if ((curPos = GET_NEIGHBOR(curPos, SE)) == INVALID)
	  break;

	curPiece = board[curPos];

	if (piece != curPiece)
	  break;


	if (j == boardSide - 2) {
	  //	  printf("lose on se\n");
	  printf("for player %c, %s on southeast\n", playersPiece, playersPiece == piece ? "win" : "lose");
	  SafeFree(board);
	  //	  return lose;
	  return playersPiece == piece ? win : lose;
	}
      }
      
      // Check southwest
      curPos = pos;

      for (j = 0; j < boardSide - 1; j++) {
	//	if ((curPos = getNeighbor(curPos, SW)) == INVALID)
	if ((curPos = GET_NEIGHBOR(curPos, SW)) == INVALID)
	  break;

	curPiece = board[curPos];

	if (piece != curPiece)
	  break;

	if (j == boardSide - 2) {
	  //	  printf("lose on sw");
	  printf("for player %c, %s on southwest\n", playersPiece, playersPiece == piece ? "win" : "lose");
	  SafeFree(board);
	  //	  return lose;
	  return playersPiece == piece ? win : lose;
	}
      }

      // No need to check W NE or NW because we search from pos = 0 forwards
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

  printPos(position, playersName, usersTurn);

}

void davidPrintPos(POSITION position, STRING playersName, BOOLEAN usersTurn) 
{

  printf("It is %s's turn.\n\n", (usersTurn ? playersName : computerName));

  int initEl, el = 0;
  int totalCols = boardSide;
  int row, col, i;

  char* board = SafeMalloc(sizeof(char) * boardSize);

  generic_unhash(position, board);

  for (row = 0; row < boardSide * 2 - 1; row++) {
    initEl = el;

    // Leading spaces
    for (i = 0; i < boardSide - row; i++) {
      printf(" ");
    }

    for (col = 0; col < totalCols; col++, el++) {
      printf("%c ", board[el]);
    }
    // Spaces between board and divider
    for (i = 0; i < boardSide - row - 1; i++) {
      printf("------");
    }

    el = initEl;

    // Spaces between divider and legend; and legend
    if (boardSide < 6) {
      for (i = 0; i < boardSide - row; i++) {
	printf("     ");
      }

      for (col = 0; col < totalCols; col++, el++) {
	printf("%2d", el);
      }
    } else {
      for (i = 0; i < boardSide - row; i++) {
	printf("  ");
      }

      for (col = 0; col < totalCols; col++, el++) {
	printf("%3d", el);
      }
    }

    printf("\n\n");
  }

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

void dyPrintPos(POSITION position, STRING playersName, BOOLEAN usersTurn) 
{

  printf("It is %s's turn.\n\n", (usersTurn ? playersName : computerName));

  //printf("%d\n", position);

  int el = 0, initEl;
  int row, col, i, totalCols = boardSide;
  char* board = (char*) SafeMalloc(sizeof(char) * boardSize);
  generic_unhash(position, board);

  /*
  //board[0] = 'a';
  //board[1] = 'a';
  printf ("testpoint8\n");
  for (i=0; i<boardSize; i++) {
    //printf("testpoint9 %d\n", i);
    printf("%c",board[i]);
  }
  printf("\n");
  */
  //printf("%d %d %d\n",abs(-1+1),abs(-2+1),abs(-1+2));

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
  int type, pos1, pos2;
  unhashMove(computersMove, &type, &pos1, &pos2);

  switch (type) {
  case NW: 
    printf("Computer %s moves %d %d NW",computersName,pos1,pos2);
  case NE:
    printf("Computer %s moves %d %d NE",computersName,pos1,pos2);
  case E:
    printf("Computer %s moves %d %d E", computersName,pos1,pos2);
  case SE:
    printf("Computer %s moves %d %d SE",computersName,pos1,pos2);
  case SW:
    printf("Computer %s moves %d %d SW",computersName,pos1,pos2);
  case W:
    printf("Computer %s moves %d %d W", computersName,pos1,pos2);
  case SWAP:
    printf("Computer %s SWAP %d %d",    computersName,pos1,pos2); 
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
  int type, pos1, pos2;
  unhashMove(move, &type, &pos1, &pos2);

  switch (type) {
  case NW: 
    printf("[%d %d NW]",pos1,pos2);
    break;
  case NE:
    printf("[%d %d NE]",pos1,pos2);
    break;
  case E:
    printf("[%d %d E]", pos1,pos2);
    break;
  case SE:
    printf("[%d %d SE]",pos1,pos2);
    break;
  case SW:
    printf("[%d %d SW]",pos1,pos2);
    break;
  case W:
    printf("[%d %d W]", pos1,pos2);
    break;
  case SWAP:
    printf("[%d %d SWAP]", pos1,pos2);
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
    
    for (;;) {
        /***********************************************************
         * CHANGE THE LINE BELOW TO MATCH YOUR MOVE FORMAT
         ***********************************************************/
	printf("%8s's move [(undo)/pos1 pos2 type] : ", playersName);
	
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
  
  int i = 0,pos1,pos2,moveType;
  char *pos2s, *moveTypes;
  BOOLEAN toReturn = TRUE;

  // pos1 within range
  toReturn = toReturn && (pos1 = atoi(input)) < boardSize && pos1 >= 0;

  while (input[i]!= ' ') {
    i++;
  }
  i++;
  pos2s = input + i;

  // pos2 within range
  toReturn = toReturn && (pos2 = atoi(pos2s)) < boardSize && pos2 >= 0;

  while (input[i]!= ' ') {
    i++;
  }

  // pos1 and pos2 are neighbors
  toReturn = toReturn && NEIGHBORING_DIRECTION(pos1, pos2) != -1;

  //  while (input[i] == ' ' || 

  moveTypes = input + i;
  //  moveType = atoi(moveTypes);

  // lowercase everything
  while (input[i] != '\0') {
    if (input[i] >= 'A' && input[i] <= 'Z') {
      input[i] -= 'A';
    }
    i++;
  }

  toReturn = toReturn && 
    (strstr(moveTypes, "swap") ||
     strstr(moveTypes, "sw")   || strstr(moveTypes, "se") ||
     strstr(moveTypes, "nw")   || strstr(moveTypes, "ne") ||
     strstr(moveTypes, "w")    || strstr(moveTypes, "e"));
    //             (moveType==NW || moveType==NE || moveType==E ||
    //	      moveType==SE || moveType==SW || moveType==W ||
    //	      moveType=SWAP);

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
  
  int i = 0,pos1,pos2,moveType, temp;
  char *pos2s, *moveTypes;

  // pos1
  pos1 = atoi(input);

  while (input[i]!= ' ') {
    i++;
  }
  i++;
  pos2s = input + i;

  // pos2
  pos2 = atoi(pos2s);

  // Swap the positions so that the lower-valued one comes first
  if (pos2 < pos1) {
    temp = pos1;
    pos1 = pos2;
    pos2 = temp;
  }

  while (input[i]!= ' ') {
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

  if (strstr(moveTypes, "swap")) {
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
  }

  //  moveType = atoi(moveTypes);

  return hashMove(moveType,pos1,pos2);

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

/* MOVE hashMove (int type, int pos1, int pos2) */
/* { */
/*   int nDirection = neighboringDirection(pos1, pos2); */

/*   return (pos1 << 6) | (nDirection << 3) | type; */
/* } */

/* void unhashMove (MOVE move, int* type, int* pos1, int* pos2) { */
/*   *pos1 = move >> 6; */
/*   *pos2 = getNeighbor(*pos1, (move >> 3) & 7); */

/*   *type = move & 7; */
/* } */

// Have to rewrite un/hashMove so that it doesn't take up so much space
MOVE hashMove (int type, int pos1, int pos2)
{
  int nDirection = NEIGHBORING_DIRECTION(pos1, pos2);

  return HASH_MOVE(type, pos1, nDirection);
}

void unhashMove (MOVE move, int* type, int* pos1, int* pos2) {
  *type = UNHASH_MOVE_TYPE(move);
  *pos1 = UNHASH_MOVE_POS(move);
  *pos2 = GET_NEIGHBOR(*pos1, UNHASH_MOVE_NDIR(move));
}

/************************************************************************
**
**  SUPPORT FUNCTIONS
**
************************************************************************/

PBPtr MallocBoard() {
    PBPtr toReturn = (PBPtr) SafeMalloc(sizeof(PARABOARD));
    toReturn->slots = (short *) SafeMalloc(boardSize);
    return toReturn;
}

void FreeBoard(PBPtr b) {
    SafeFree(b->slots);
    SafeFree(b);
}

// Return whether or not a position, given by a row and a column, is valid
// on the current board size
// NOTE: no longer in use because rcToSlot returns invalid on fail
BOOLEAN validPosition(int row, int col) {
  printf("validPosition called on %d, %d\n", row, col);

  return TRUE;
}

// if there is no neigher in the specified direction, return INVALID
// returns a board index b/n 0 and boardSize
int getNeighbor(int pos, int direction) {
  int pRow, pCol;

  //  getColRow(pos, &pCol, &pRow);

  getRowCol(pos, &pRow, &pCol);

  //  printf("     * getNeighbor: getRowCol(%d) returns (%d, %d)\n", pos, pRow, pCol);

  if (pRow < boardSide - 1) {
    // if the position in the top half of the board
    //    printf("     * getNeighbor: position %d is in top half of board\n", pos);

    switch (direction) {
    case NW:
      return rcToSlot(pRow - 1, pCol - 1);
      //      return (validPosition(pRow - 1, pCol - 1) ? rcToSlot(pRow - 1, pCol - 1) : INVALID);
    case NE:
      return rcToSlot(pRow - 1, pCol);
      //      return (validPosition(pRow, pCol - 1) ? rcToSlot(pRow, pCol - 1) : INVALID);
    case SE:
      return rcToSlot(pRow + 1, pCol + 1);
      //      return (validPosition(pRow + 1, pCol + 1) ? rcToSlot(pRow + 1, pCol + 1) : INVALID);
    case SW:
      return rcToSlot(pRow + 1, pCol);
      //      return (validPosition(pRow, pCol + 1) ? rcToSlot(pRow, pCol) : INVALID);
    case W:
      return rcToSlot(pRow, pCol - 1);
      //      return (validPosition(pRow - 1, pCol) ? rcToSlot(pRow - 1, pCol) : INVALID);
    case E:
      return rcToSlot(pRow, pCol + 1);
      //      return (validPosition(pRow + 1, pCol) ? rcToSlot(pRow + 1, pCol) : INVALID);
    default:
      return INVALID;
    }
  } else if (pRow > boardSide - 1) {
    // if the position in the bottom half of the board

    switch (direction) {
    case NW:
      return rcToSlot(pRow - 1, pCol);
      //      return (validPosition(pRow, pCol - 1) ? rcToSlot(pRow, pCol - 1) : INVALID);
    case NE:
      return rcToSlot(pRow - 1, pCol + 1);
      //      return (validPosition(pRow + 1, pCol - 1) ? rcToSlot(pRow + 1, pCol - 1) : INVALID);
    case SE:
      return rcToSlot(pRow + 1, pCol);
      //      return (validPosition(pRow, pCol + 1) ? rcToSlot(pRow, pCol + 1) : INVALID);
    case SW:
      return rcToSlot(pRow + 1, pCol - 1);
      //      return (validPosition(pRow - 1, pCol + 1) ? rcToSlot(pRow - 1, pCol) : INVALID);
    case W:
      return rcToSlot(pRow, pCol - 1);
      //      return (validPosition(pRow - 1, pCol) ? rcToSlot(pRow - 1, pCol) : INVALID);
    case E:
      return rcToSlot(pRow, pCol + 1);
      //      return (validPosition(pRow + 1, pCol) ? rcToSlot(pRow + 1, pCol) : INVALID);
    default:
      return INVALID;
    }
  } else {
    // if the position in the middle row of the board

    switch (direction) {
    case NW:
      return rcToSlot(pRow - 1, pCol - 1);
      //      return (validPosition(pRow - 1, pCol - 1) ? rcToSlot(pRow - 1, pCol - 1) : INVALID);
    case NE:
      return rcToSlot(pRow - 1, pCol);
      //      return (validPosition(pRow, pCol - 1) ? rcToSlot(pRow, pCol - 1) : INVALID);
    case SE:
      return rcToSlot(pRow + 1, pCol);
      //      return (validPosition(pRow, pCol + 1) ? rcToSlot(pRow, pCol + 1) : INVALID);
    case SW:
      return rcToSlot(pRow + 1, pCol - 1);
      //      return (validPosition(pRow - 1, pCol + 1) ? rcToSlot(pRow - 1, pCol) : INVALID);
    case W:
      return rcToSlot(pRow, pCol - 1);
      //      return (validPosition(pRow - 1, pCol) ? rcToSlot(pRow - 1, pCol) : INVALID);
    case E:
      return rcToSlot(pRow, pCol + 1);
      //      return (validPosition(pRow + 1, pCol) ? rcToSlot(pRow + 1, pCol) : INVALID);
    default:
      return INVALID;
    }
  }

  // should never fall through here
  return INVALID;
}

/* Direction in which the neighbor neighbors pos:
  returns one of 
  0 NW
  1 NE
  2 E
  3 SE
  4 SW
  5 W
 -1 Not neighboring
*/

int neighboringDirection(int pos, int neighbor) {
  int pCol, pRow;
  int nCol, nRow;

  //  getColRow(pos, &pCol, &pRow);
  //  getColRow(pos, &nCol, &nRow);

  getRowCol(pos, &pRow, &pCol);
  getRowCol(neighbor, &nRow, &nCol);

  int dCol = nCol - pCol,
      dRow = nRow - pRow;

  if (pRow < boardSide - 1) {
    // if the first position is in the top half of the board

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
	return  SE;
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
    // if the first position is in the bottom half of the board

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
    // if the first position is in the middle row of the board

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

  int x,y=boardSide,toReturn=0;
  int row, col, totalCols = boardSide;

  if (    
      // valid r
      (r>=0 && r<(2*boardSide-1)) &&
      // valid columns
      (c>=0 && c<(r<boardSide ? boardSide+r : 2*boardSide - r%boardSide - 2))
      ) {
    
    // for rows before r w/ increasing length
    for (x=0; x<min(boardSide-1,r); x++, toReturn += y++);
    // for rows before r w/ decreasing length
    for (; x<r; x++, toReturn += y--);
    // for columns in row = r
    return toReturn += c;

  } else {
    // invalid r,c
    //    printf("**** Error -- rcToSlot: Invalid r,c: %d, %d\n", r, c);
    return INVALID;
  }
}

// returns row and column coordinate when given slot number
// returns -1 when given invalid slot number
// return data format = r*100+c
int slotToRC(int s) {

  int x=0,y=boardSide,z=-1;
  int r,c;

  while (z<s && x<boardSide) {
    z += y++;
    x++;
  }
  if (x==(boardSide-1)) y--;
  while (z<s && x<(2*boardSide-1)) {
    z += y--;
    x++;
  }

  r = x-1;
  if (r<boardSide-1) {
    c = (s-(z-y+1)-1)%(y-1);
  } else if (r==boardSize-1) {
    c = (s--);
  } else {
    c = (s-(z-y+1)+1)%(y+1);
  }

  return r * 100 + c;

  //  return 0;

}

void getColRow(int pos, int* pCol, int* pRow) {
  int col = 0, row = 0, rowSize = boardSide, numEls = 0;

  for (; pos >= numEls + rowSize; col++, numEls += rowSize, rowSize += (col > boardSide - 1 ? -1 : 1));

  //  for (; pos < (numEls - rowSize); col++, rowSize += (col > boardSize - 1 ? -1 : 1), numEls += rowSize);

  *pCol = col;
  //  *pRow = pos - numEls + rowSize;

  *pRow = pos - numEls;
}

/* void getRowCol(int pos, int* pRow, int* pCol) { */
/*   int col = 0, row = 0, rowSize = boardSide, numEls = 0; */

/*   for (; pos >= numEls + rowSize; col++, numEls += rowSize, rowSize += (col > boardSide - 1 ? -1 : 1)); */

/*   *pCol = col; */

/*   *pRow = pos - numEls; */
/* } */

void getRowCol(int pos, int* pRow, int* pCol) {
  int col = 0, row = 0, colSize = boardSide, numEls = 0;

  for (; pos >= numEls + colSize; row++, numEls += colSize, colSize += (row > boardSide - 1 ? -1 : 1));

  *pRow = row;

  *pCol = pos - numEls;
}

int whoseMoveF(POSITION p) {
  return p;
}

////////////////////////////
//
//  TEST CODE
//
////////////////////////////

void yanpeiTestNeighboringDir() {

  int i, j;

  //  printf("\nTESTING neighboringDirection()\n");
  for (i=0; i<boardSize; i++) {
    for (j=0; j<boardSize; j++) {
      if (i!=j) {
	//	printf("pos %d neighbor%d direction%d\n",
	//	       i,j,neighboringDirection(i,j));
      }
    }
  }
}

void yanpeiTestRcToSlot() {

  int row, col, totalCols = boardSide;

  printf("\nTESTING rcToSlot()\n");

  for (row = 0; row < boardSide * 2 - 1; row++, (row<boardSide ? totalCols++ : totalCols--)) {
    for (col = 0; col < totalCols; col++) {
      printf("r c %d %d slot %d\n", row, col, rcToSlot(row, col));
    }
  }
  printf ("\n");

}

// $Log: not supported by cvs2svn $
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
// 	     in their getOption/setOption hash.
// mttc.c: Edited to handle conflicting types.  Created a PLAYER type for
//         gamesman.  mttc.c had a PLAYER type already, so I changed it.
// analysis.c: Changed initialization of option variable in analyze() to -1.
// db.c: Changed check in the getter functions (GetValueOfPosition and
//       getRemoteness) to check if gMenuMode is Evaluated.
// gameplay.c: Removed PlayAgainstComputer and PlayAgainstHuman.  Wrote PlayGame
//             which is a generic version of the two that uses to PLAYER's.
// gameplay.h: Created the necessary structs and types to have PLAYER's, both
// 	    Human and Computer to be sent in to the PlayGame function.
// gamesman.h: Really don't think I changed anything....
// globals.h: Also don't think I changed anything....both these I just looked at
//            and possibly made some format changes.
// textui.c: Redid the portion of the menu that allows you to choose opponents
// 	  and then play a game.  Added computer vs. computer play.  Also,
//           changed the analysis part of the menu so that analysis should
// 	  work properly with symmetries (if it is in getOption/setOption hash).
//
// Revision 1.3  2005/03/10 02:06:47  ogren
// Capitalized CVS keywords, moved Log to the bottom of the file - Elmer
//

