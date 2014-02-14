/************************************************************************
**
** NAME:        mLgame.c
**
** DESCRIPTION: L-game
**
** AUTHORS:     Michael Savitzky & Alexander John Kozlowski
**
** DATE:        12/15/01
**
**************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/

#include <stdio.h>
#include "gamesman.h"

extern STRING gValueString[];

POSITION gNumberOfPositions  = 129022;  /* (6 + 7*7 + 23*7*8 + 47*7*8*24)*2 */

POSITION gInitialPosition    =  19388;
POSITION gMinimalPosition    =  19388;

STRING kAuthorName         = "Michael Savitzky and Alexander John Kozlowski";
STRING kGameName           = "L-game";
BOOLEAN kPartizan           = TRUE;
BOOLEAN kDebugMenu          = TRUE;
BOOLEAN kGameSpecificMenu   = TRUE;
BOOLEAN kTieIsPossible      = FALSE;
BOOLEAN kLoopy               = TRUE;
BOOLEAN kDebugDetermineValue = FALSE;
POSITION kBadPosition           = -1;
void*    gGameSpecificTclInit = NULL;


STRING kHelpGraphicInterface =
        "The LEFT button puts an X or O (depending on whether you went first\n\
or second) on the spot the cursor was on when you clicked. The MIDDLE\n\
button does nothing, and the RIGHT button is the same as UNDO, in that\n\
it reverts back to your your most recent position."                                                                                                                                                                                                                                   ;

STRING kHelpTextInterface    =
        "MOVING YOUR L-PIECE:\n\
First enter the orientation index number of your desired L-piece position.\n\
Then, after entering a space, enter the desired position for the corner square \n\
of your newly selected L-piece orientation.  If you do not wish to move a \n\
neutral piece, then hit enter to complete the move.\n\
MOVING A NEUTRAL PIECE:\n\
If you choose to move a neutral piece, enter a space, then the 'w' or 'g'\n\
character (depending on which neutral piece you want to move), and finally a\n\
space followed by the new position of this neutral piece."                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               ;

STRING kMove1or0HelpOnYourTurn =
        "Move your L-piece to an empty position and then one (or neither) of\n\
the neutral pieces to an empty square."                                                                                ;

STRING kMove1or0SpecHelpOnYourTurn =
        "Move your L-piece to an empty position and then your neutral piece (optional)\n\to an empty square.";

STRING kMove1HelpOnYourTurn =
        "Move your L-piece to an empty position and then one of\n\
the neutral pieces to an empty square."                                                                   ;

STRING kMove1SpecHelpOnYourTurn =
        "Move your L-piece to an empty position and then your neutral piece\n\
to an empty square."                                                                               ;

STRING kHelpOnYourTurn = "You move your L-piece to an empty position and then one (or neither) of\n\
the neutral pieces to an empty square.";

STRING kHelpStandardObjective =
        "To position your L-piece and the neutral pieces so that your opponent\n\
cannot move his L-piece."                                                                                  ;

STRING kHelpReverseObjective =
        "To not be able to move your L-piece.";

STRING kHelpTieOccursWhen =   /* Should follow 'A Tie occurs when... */
                            "you have been playing in a loop and you're sick of it, so you and your\n\
opponent (or just you if you're playing the computer) decide that you'd \n\
rather be doing something else."                                                                                                                                                                                   ;

STRING kHelpExample =
        "Ok, Dan and Computer, let us begin.\n\n\
Type '?' if you need assistance...\n\n\n\
   CURRENT               POSSIBLE ORIENTATIONS                 LEGEND\n\
+-----------+   +---+   +---+                               +-----------+\n\
|w  X  X    |   | +-+   +-+ |     +-----+         +-+       | 1  2  3  4|\n\
|           |   | | =1    | | =2  +---+ | =3  +---+ | =4    |           |\n\
|   O  X    |   | |       | |         +-+     +-----+       | 5  6  7  8|\n\
|           |   +-+       +-+                               |           |\n\
|   O  X    |     +-+     +-+                               | 9 10 11 12|\n\
|           |     | |     | |     +-+         +-----+       |           |\n\
|   O  O  g |     | | =5  | | =6  | +---+ =7  | +---+ =8    |13 14 15 16|\n\
+-----------+   +-+ |     | +-+   +-----+     +-+           +-----------+\n\
                +---+     +---+\n\
   X's turn      (Dan should Tie in 6) \n\n\
    Move format: ['Orientation' 'Pos of corner' 'N-Piece' 'Pos of N-Piece']\n\
     Dan's move: (u)ndo/[1-8 1-16]/[1-8 1-16 'w'or'g' 1-16] : 2 4 w 2\n\n\n\
   CURRENT               POSSIBLE ORIENTATIONS                 LEGEND\n\
+-----------+   +---+   +---+                               +-----------+\n\
|   w  X  X |   | +-+   +-+ |     +-----+         +-+       | 1  2  3  4|\n\
|           |   | | =1    | | =2  +---+ | =3  +---+ | =4    |           |\n\
|   O     X |   | |       | |         +-+     +-----+       | 5  6  7  8|\n\
|           |   +-+       +-+                               |           |\n\
|   O     X |     +-+     +-+                               | 9 10 11 12|\n\
|           |     | |     | |     +-+         +-----+       |           |\n\
|   O  O  g |     | | =5  | | =6  | +---+ =7  | +---+ =8    |13 14 15 16|\n\
+-----------+   +-+ |     | +-+   +-----+     +-+           +-----------+\n\
                +---+     +---+\n\
   O's turn      (Computer will Win in 1) \n\n\
     Computer's move                          : 26 g 05\n\n\n\
   CURRENT               POSSIBLE ORIENTATIONS                 LEGEND\n\
+-----------+   +---+   +---+                               +-----------+\n\
|   w  X  X |   | +-+   +-+ |     +-----+         +-+       | 1  2  3  4|\n\
|           |   | | =1    | | =2  +---+ | =3  +---+ | =4    |           |\n\
|g     O  X |   | |       | |         +-+     +-----+       | 5  6  7  8|\n\
|           |   +-+       +-+                               |           |\n\
|      O  X |     +-+     +-+                               | 9 10 11 12|\n\
|           |     | |     | |     +-+         +-----+       |           |\n\
|   O  O    |     | | =5  | | =6  | +---+ =7  | +---+ =8    |13 14 15 16|\n\
+-----------+   +-+ |     | +-+   +-----+     +-+           +-----------+\n\
                +---+     +---+\n\
   X's turn      (Dan will Lose in 0) \n\n\n\
Computer wins. Nice try, Dan."                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     ;

STRING MoveToString(MOVE);

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

int inPrimitive = 0; /* Turned on when entering Primitive; turned off when leaving */
int oneL = 0;  /* Turned on for the game option of moving only 1 specific neutral piece */
int white1 = 1;  /* If on, then white is player 1's piece (if oneL option is on) */
int mustMove = 0;  /* Turned on for the option of requiring a move of the neutral piece */

int valCor[9][6] = {{},  /* Valid corners (with blank board) for each orientation */
		    {1, 2, 3, 5, 6, 7},
		    {2, 3, 4, 6, 7, 8},
		    {3, 4, 7, 8, 11, 12},
		    {7, 8, 11, 12, 15, 16},
		    {10, 11, 12, 14, 15, 16},
		    {9, 10, 11, 13, 14, 15},
		    {5, 6, 9, 10, 13, 14},
		    {1, 2, 5, 6, 9, 10}};

int FOURSQUARES[49][4] = {{0, 0, 0, 0},  /*Gives all 4 squares each L-piece covers*/
			  {9, 5, 1, 2},
			  {10, 6, 2, 3},
			  {11, 7, 3, 4},
			  {13, 9, 5, 6},
			  {14, 10, 6, 7},
			  {15, 11, 7, 8},
			  {10, 6, 2, 1},
			  {11, 7, 3, 2},
			  {12, 8, 4, 3},
			  {14, 10, 6, 5},
			  {15, 11, 7, 6},
			  {16, 12, 8, 7},
			  {2, 3, 4, 8},
			  {6, 7, 8, 12},
			  {10, 11, 12, 16},
			  {1, 2, 3, 7},
			  {5, 6, 7, 11},
			  {9, 10, 11, 15},
			  {6, 7, 8, 4},
			  {10, 11, 12, 8},
			  {14, 15, 16, 12},
			  {5, 6, 7, 3},
			  {9, 10, 11, 7},
			  {13, 14, 15, 11},
			  {8, 12, 16, 15},
			  {7, 11, 15, 14},
			  {6, 10, 14, 13},
			  {4, 8, 12, 11},
			  {3, 7, 11, 10},
			  {2, 6, 10, 9},
			  {7, 11, 15, 16},
			  {6, 10, 14, 15},
			  {5, 9, 13, 14},
			  {3, 7, 11, 12},
			  {2, 6, 10, 11},
			  {1, 5, 9, 10},
			  {15, 14, 13, 9},
			  {11, 10, 9, 5},
			  {7, 6, 5, 1},
			  {16, 15, 14, 10},
			  {12, 11, 10, 6},
			  {8, 7, 6, 2},
			  {11, 10, 9, 13},
			  {7, 6, 5, 9},
			  {3, 2, 1, 5},
			  {12, 11, 10, 14},
			  {8, 7, 6, 10},
			  {4, 3, 2, 6}};

int LPIECESREMAINING[49][24] = {  /*Given that the rowth L-piece has been placed,
	                             this lists the posibilities for second L-piece.*/
	{0},
	{3, 5, 6, 9, 11, 12, 14, 15, 19, 20, 21, 24, 25, 26, 27, 28, 29, 31, 32, 34, 40, 41, 46, 47},
	{6, 12, 21, 24, 25, 26, 28, 31, 33, 37, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{1, 4, 7, 10, 21, 25, 27, 30, 32, 33, 36, 37, 40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{3, 6, 8, 9, 12, 13, 15, 16, 20, 21, 25, 26, 28, 29, 31, 34, 40, 46, 0, 0, 0, 0, 0, 0},
	{1, 9, 13, 25, 28, 45, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{1, 2, 4, 7, 10, 27, 30, 33, 36, 45, 48, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{3, 6, 9, 12, 21, 24, 25, 26, 28, 31, 33, 34, 37, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{4, 10, 21, 25, 27, 32, 33, 36, 37, 40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{1, 4, 5, 7, 10, 11, 17, 18, 23, 24, 26, 27, 30, 31, 32, 33, 35, 36, 37, 38, 39, 40, 43, 44},
	{3, 6, 8, 9, 12, 13, 16, 25, 28, 31, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{1, 9, 13, 33, 36, 45, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{1, 2, 4, 7, 10, 18, 24, 27, 30, 32, 33, 35, 36, 37, 38, 43, 45, 48, 0, 0, 0, 0, 0, 0},
	{4, 5, 10, 11, 15, 17, 18, 21, 23, 24, 26, 27, 31, 32, 33, 36, 37, 38, 39, 40, 41, 43, 44, 46},
	{1, 18, 24, 33, 36, 37, 38, 40, 43, 45, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{1, 4, 13, 16, 19, 22, 33, 37, 39, 42, 44, 45, 48, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{4, 10, 15, 18, 20, 21, 24, 25, 27, 28, 32, 33, 37, 38, 40, 41, 43, 46, 0, 0, 0, 0, 0, 0},
	{9, 13, 21, 25, 37, 40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{9, 12, 13, 14, 16, 19, 22, 39, 42, 45, 48, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{1, 15, 18, 21, 24, 33, 36, 37, 38, 40, 43, 45, 46, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{1, 4, 16, 22, 33, 37, 39, 44, 45, 48, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{1, 2, 3, 4, 7, 8, 13, 16, 17, 19, 22, 23, 29, 30, 35, 36, 38, 39, 42, 43, 44, 45, 47, 48},
	{15, 18, 20, 21, 24, 25, 28, 37, 40, 43, 46, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{9, 13, 21, 25, 45, 48, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{1, 2, 7, 9, 12, 13, 14, 16, 19, 22, 30, 36, 39, 42, 44, 45, 47, 48, 0, 0, 0, 0, 0, 0},
	{1, 2, 3, 4, 5, 7, 8, 10, 16, 17, 22, 23, 27, 29, 30, 33, 35, 36, 38, 39, 43, 44, 45, 48},
	{1, 2, 4, 7, 9, 13, 30, 36, 45, 48, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{1, 3, 6, 8, 9, 12, 13, 16, 25, 28, 31, 34, 45, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{1, 2, 4, 5, 7, 10, 16, 22, 27, 30, 32, 33, 36, 37, 39, 40, 44, 45, 0, 0, 0, 0, 0, 0},
	{1, 4, 21, 25, 33, 37, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{3, 6, 9, 12, 21, 24, 25, 26, 28, 31, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{1, 2, 4, 7, 9, 10, 13, 27, 30, 33, 36, 45, 48, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{1, 3, 8, 9, 12, 13, 16, 28, 34, 45, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{2, 3, 6, 7, 8, 9, 11, 12, 13, 14, 15, 16, 19, 20, 25, 28, 29, 31, 34, 35, 41, 42, 47, 48},
	{1, 4, 7, 10, 27, 30, 32, 33, 36, 37, 40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{9, 12, 21, 25, 33, 37, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{3, 6, 8, 9, 11, 12, 13, 14, 19, 21, 24, 25, 26, 28, 31, 34, 42, 48, 0, 0, 0, 0, 0, 0},
	{2, 3, 7, 8, 9, 12, 13, 14, 15, 16, 17, 19, 20, 22, 28, 29, 34, 35, 39, 41, 42, 45, 47, 48},
	{9, 12, 13, 14, 16, 19, 21, 25, 42, 48, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{9, 13, 15, 18, 20, 21, 24, 25, 28, 37, 40, 43, 46, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{1, 3, 4, 8, 9, 13, 14, 16, 17, 19, 22, 28, 34, 39, 42, 44, 45, 48, 0, 0, 0, 0, 0, 0},
	{1, 13, 16, 33, 37, 45, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{15, 18, 21, 24, 33, 36, 37, 38, 40, 43, 46, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{9, 12, 13, 14, 16, 19, 21, 22, 25, 39, 42, 45, 48, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{9, 13, 15, 20, 21, 24, 25, 28, 40, 46, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{5, 6, 11, 12, 14, 15, 18, 19, 20, 21, 23, 24, 25, 26, 27, 28, 31, 32, 37, 40, 41, 43, 46, 47},
	{1, 4, 13, 16, 19, 22, 39, 42, 44, 45, 48, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{1, 21, 24, 33, 37, 45, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{6, 12, 15, 18, 20, 21, 23, 24, 25, 26, 31, 33, 36, 37, 38, 40, 43, 46, 0, 0, 0, 0, 0, 0}
};

int transPairs[49][2] = { /*Gives orientation and corner position of each L-piece*/
	{0}, {1, 1}, {1, 2}, {1, 3}, {1, 5}, {1, 6}, {1, 7},
	{2, 2}, {2, 3}, {2, 4}, {2, 6}, {2, 7}, {2, 8},
	{3, 4}, {3, 8}, {3, 12}, {3, 3}, {3, 7}, {3, 10},
	{4, 8}, {4, 12}, {4, 16}, {4, 7}, {4, 11}, {4, 15},
	{5, 16}, {5, 15}, {5, 14}, {5, 12}, {5, 11}, {5, 10},
	{6, 15}, {6, 14}, {6, 13}, {6, 11}, {6, 10}, {6, 9},
	{7, 13}, {7, 9}, {7, 5}, {7, 14}, {7, 10}, {7, 6},
	{8, 9}, {8, 5}, {8, 1}, {8, 10}, {8, 6}, {8, 2}
};

int transArr[8][16] = { /*Gives the L-piece # for a given orienation and corner*/
	{1, 2, 3, 0, 4, 5, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 7, 8, 9, 0, 10, 11, 12, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 16, 13, 0, 0, 17, 14, 0, 0, 18, 15, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 22, 19, 0, 0, 23, 20, 0, 0, 24, 21},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 30, 29, 28, 0, 27, 26, 25},
	{0, 0, 0, 0, 0, 0, 0, 0, 36, 35, 34, 0, 33, 32, 31, 0},
	{0, 0, 0, 0, 39, 42, 0, 0, 38, 41, 0, 0, 37, 40, 0, 0},
	{45, 48, 0, 0, 44, 47, 0, 0, 43, 46, 0, 0, 0, 0, 0, 0}
};

/*static int gSymmetryMatrix[NUMSYMMETRIES][BOARDSIZE];*/

/* Proofs of correctness for the below arrays:
**
** FLIP						ROTATE
**
** 0 1 2	2 1 0		0 1 2		6 3 0		8 7 6		2 5 8
** 3 4 5  ->    5 4 3		3 4 5	->	7 4 1  ->	5 4 3	->	1 4 7
** 6 7 8	8 7 6		6 7 8		8 5 2		2 1 0		2 1 0
*/

/* This is the array used for flipping along the N-S axis */
int gFlipNewPosition[] = { 2, 1, 0, 5, 4, 3, 8, 7, 6 };

/* This is the array used for rotating 90 degrees clockwise */
int gRotate90CWNewPosition[] = { 6, 3, 0, 7, 4, 1, 8, 5, 2 };


/** Local Prototypes **/
int hash(int L1, int L2, int S1, int S2, int whosMove);
int unhashL1(int total);
int unhashL2(int total);
int unhashS1(int total);
int unhashS2(int total);
int unhashTurn(int total);
int hashMove(int L, int SPiece, int SValue);
int unhashMoveL(int total);
int unhashMoveSPiece(int total);
int unhashMoveSValue(int total);
int transform(int orient, int origL);
int untransformOrient(int L);
int untransformOrig(int L, int orient);
int printInit(int pieces, int L1, int L2, int S1, int S2);
int Make24(int L1, int L2);
int Make48(int L1, int L2);
int Make16to8(int L1, int L2, int S1as16);
int Make8to16(int L1, int L2, int S1as8);
int Make16to7(int L1, int L2, int S1, int S2as16);
int Make7to16(int L1, int L2, int S1, int S2as7);
int Get6Empties(int L1, int L2, int S1, int S2, int j);
int memberOf(int L, int square);
int clearS1(int L1, int L2, int S1);
int clearS2(int L1, int L2, int S1, int S2);
int checkCor(int Lo, int Lc);
int checkOrient(int Lo, int L1);

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

		switch(GetMyChar()) {
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

void GameSpecificMenu()
{
	char GetMyChar();
	STRING oneLSt;
	STRING oneLSt2;
	STRING color;
	STRING mustMoveSt;
	if (oneL) {
		oneLSt = "ONE";
		oneLSt2 = "BOTH";
	}
	else {
		oneLSt = "BOTH";
		oneLSt2 = "ONE";
	}
	if (white1)
		color = "WHITE";
	else
		color = "GRAY";
	if (mustMove)
		mustMoveSt = "ON";
	else
		mustMoveSt = "OFF";

	do {
		printf("\n\t----- Game Options for %s -----\n\n", kGameName);

		printf("\ta)\tToggle from having (A)ccess to %s neutral\n\t  \t piece(s)to having access to %s neutral piece(s).\n", oneLSt, oneLSt2);
		if (oneL)
			printf("\tn)\tChange the (N)eutral piece player X has access to.  Currently %s.\n", color);
		printf("\tm)\tRequire (M)ovement of a neutral piece each move.  Currently %s.\n", mustMoveSt);
		printf("\ti)\tChange the (I)nitial position.\n");
		printf("\n\n\tb)\t(B)ack = Return to previous activity.\n");
		printf("\n\nSelect an option: ");

		switch(GetMyChar()) {
		case 'Q': case 'q':
			ExitStageRight();
		case 'A': case 'a':
			if (oneL) {
				oneL = 0;
				oneLSt = "BOTH";
				oneLSt2 = "ONE";
				if (mustMove)
					kHelpOnYourTurn = kMove1HelpOnYourTurn;
				else kHelpOnYourTurn = kMove1or0HelpOnYourTurn;
			}
			else {
				oneL = 1;
				oneLSt = "ONE";
				oneLSt2 = "BOTH";
				if (mustMove)
					kHelpOnYourTurn = kMove1SpecHelpOnYourTurn;
				else kHelpOnYourTurn = kMove1or0SpecHelpOnYourTurn;
			}
			break;
		case 'N': case 'n':
			if (white1) {
				white1 = 0;
				color = "GRAY";
			}
			else {
				white1 = 1;
				color = "WHITE";
			}
			break;
		case 'M': case 'm':
			if (mustMove) {
				mustMove = 0;
				mustMoveSt = "OFF";
				if (oneL)
					kHelpOnYourTurn = kMove1or0SpecHelpOnYourTurn;
				else kHelpOnYourTurn = kMove1or0HelpOnYourTurn;
			}
			else {
				mustMove = 1;
				mustMoveSt = "ON";
				if (oneL)
					kHelpOnYourTurn = kMove1SpecHelpOnYourTurn;
				else kHelpOnYourTurn = kMove1HelpOnYourTurn;
			}
			break;
		case 'I': case 'i':
			gInitialPosition = GetInitialPosition();
			break;
		case 'B': case 'b':
			return;
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
** CALLS:       unhashL1(POSITION)
**              Make48(INT, INT)
**              unhashL2(POSITION)
**              Make8to16(INT, INT, INT)
**              unhashS1(POSITION)
**              Make7to16(INT, INT, INT, INT)
**              unhashS2(POSITION)
**              unhashTurn(POSITION)
**              unhashMoveL(MOVE)
**              unhashMoveSPiece(MOVE)
**              unhashMoveSValue(MOVE)
**              Make16to7(INT, INT, INT, INT)
**              Make16to8(INT, INT, INT)
**              Make24(INT, INT)
**              hash(INT, INT, INT, INT, INT)
**
************************************************************************/


POSITION DoMove(POSITION thePosition, MOVE theMove) {
	int L1, L2, S1, S2, whosMove, origL1, origL2, origS1, origS2, L, SPiece, SValue;
	origL1 = unhashL1(thePosition);
	origL2 = Make48(origL1, unhashL2(thePosition));
	origS1 = Make8to16(origL1, origL2, unhashS1(thePosition));
	origS2 = Make7to16(origL1, origL2, origS1, unhashS2(thePosition));
	whosMove = unhashTurn(thePosition);
	L = unhashMoveL(theMove);
	SPiece = unhashMoveSPiece(theMove);
	SValue = unhashMoveSValue(theMove);
	if (whosMove == 1) {
		L2 = origL2;
		L1 = L;
	}
	else {
		L1 = origL1;
		L2 = L;
	}
	if (SPiece == 0) {
		S1 = origS1;
		S2 = origS2;
	}
	else if (SPiece == 1) {
		S2 = origS2;
		S1 = SValue;
	}
	else {
		S1 = origS1;
		S2 = SValue;
	}
	S2 = Make16to7(L1, L2, S1, S2);
	S1 = Make16to8(L1, L2, S1);
	L2 = Make24(L1, L2);
	if (whosMove == 1)
		whosMove = 2;
	else whosMove = 1;
	return (POSITION)hash(L1, L2, S1, S2, whosMove);
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
** CALLS:       printInit(INT, INT, INT, INT, INT)
**              GetMyChar()
**              checkCor(INT, INT)
**              transform(INT, INT)
**              checkOrient(INT, INT)
**              clearS1(INT, INT, INT, INT)
**
************************************************************************/

POSITION GetInitialPosition() {
	int Lo, Lc, L1, L2, S1, S2, whosMove, i, temp2 = 0;
	char temp[2];
	printInit(0, 0, 0, 0, 0);
	printf("\n\n\t----- Get Initial Position -----\n");

	printf("\n\tInput the starting orientation of the first L-piece.\n");
	Lo = GetMyChar();
	while (1) {
		if (Lo <= '8' && Lo >= '1') {
			Lo = Lo - 48;
			break;
		}
		else {
			printf("Invalid starting orientation.  Please enter an integer between 1 and 8.\n");
			Lo = GetMyChar();
		}
	}

	printf("\n\tInput the starting location of the corner of the first L-piece.\n");
	while (1) {
		scanf("%s", temp);
		if (temp[1] != '\0' && temp[0] == '1' && temp[1] >= '0' && temp[1] <= '6' ) {
			Lc = 10 + temp[1] - 48;
			if (checkCor(Lo, Lc))
				break;
		}
		else if (temp[1] == '\0' && temp[0] <= '9' && temp[0] >= '1') {
			Lc = temp[0] - 48;
			if (checkCor(Lo, Lc))
				break;
		}
		printf("Invalid corner location for orientation %d.\nPlease enter one of the following valid corner locations: %d, %d, %d, %d, %d, %d.\n", Lo, valCor[Lo][0], valCor[Lo][1], valCor[Lo][2], valCor[Lo][3], valCor[Lo][4], valCor[Lo][5]);
	}
	L1 = transform(Lo, Lc);
	printInit(1, L1, 0, 0, 0);

	printf("\n\tInput the starting orientation of the second L-piece.\n");
	Lo = GetMyChar();
	while (1) {
		if (Lo <= '8' && Lo >= '1') {
			Lo = Lo - 48;
			if (checkOrient(Lo, L1))
				break;
		}
		printf("Invalid starting orientation.  Please try again.\n");
		Lo = GetMyChar();
	}

	printf("\n\tInput the starting location of the corner of the second L-piece.\n");
	while (1) {
		scanf("%s", temp);
		if (temp[1] != '\0' && temp[0] == '1' && temp[1] >= '0' && temp[1] <= '6' ) {
			Lc = 10 + temp[1] - 48;
			L2 = transform(Lo, Lc);
			for (i = 0; i<24; i++)
				if (L2 == LPIECESREMAINING[L1][i]) {
					temp2 = 1;
					break;
				}
		}
		else if (temp[1] == '\0' && temp[0] <= '9' && temp[0] >= '1') {
			Lc = temp[0] - 48;
			L2 = transform(Lo, Lc);
			for (i = 0; i<24; i++)
				if (L2 == LPIECESREMAINING[L1][i]) {
					temp2 = 1;
					break;
				}
		}
		if (temp2)
			break;
		printf("Invalid corner location for orientation %d.\nPlease try again.\n", Lo);
	}
	printInit(2, L1, L2, 0, 0);

	printf("\n\tInput the starting position of the first neutral piece.\n");
	while (1) {
		scanf("%s", temp);
		if (temp[1] != '\0' && temp[0] == '1' && temp[1] >= '0' && temp[1] <= '6') {
			S1 = 10 + temp[1] - 48;
			if (clearS1(L1, L2, S1))
				break;
		}
		else if (temp[1] == '\0' && temp[0] <= '9' && temp[0] >= '1') {
			S1 = temp[0] - 48;
			if (clearS1(L1, L2, S1))
				break;
		}
		printf("Invalid location for neutral piece.  Please choose an empty square.\n");
	}
	printInit(3, L1, L2, S1, 0);

	printf("\n\tInput the starting position of the second neutral piece.\n");
	while (1) {
		scanf("%s", temp);
		if (temp[1] != '\0' && temp[0] == '1' && temp[1] >= '0' && temp[1] <= '6') {
			S2 = 10 + temp[1] - 48;
			if (clearS2(L1, L2, S1, S2))
				break;
		}
		else if (temp[1] == '\0' && temp[0] <= '9' && temp[0] >= '1') {
			S2 = temp[0] - 48;
			if (clearS2(L1, L2, S1, S2))
				break;
		}
		printf("Invalid location for neutral piece.  Please choose an empty square.\n");
	}

	printInit(4, L1, L2, S1, S2);
	printf("\nNow, whose turn is it? [X/O] : ");
	whosMove = GetMyChar();
	while (1) {
		if (whosMove == 'x' || whosMove == 'X') {
			whosMove = 1;
			break;
		}
		else if (whosMove == 'o' || whosMove == 'O') {
			whosMove = 2;
			break;
		}
		else {
			printf("%c is invalid.  You must choose either X or O.\n", whosMove);
			whosMove = GetMyChar();
		}
	}

	return((POSITION)hash(L1, Make24(L1, L2), Make16to8(L1, L2, S1),
	                      Make16to7(L1, L2, S1, S2), whosMove));
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
**
** CALLS:       unhashMoveL(MOVE)
**              unhashMoveSValue(MOVE)
**              untransformOrient(INT)
**              untransformOrig(INT, INT)
**              unhashMoveSPiece(MOVE)
**
************************************************************************/

void PrintComputersMove(computersMove,computersName)
MOVE computersMove;
STRING computersName;
{
	int L = unhashMoveL(computersMove);
	int SValue = unhashMoveSValue(computersMove);
	char SPiece = 'w';
	int orient = untransformOrient(L);
	int orig = untransformOrig(L, orient);
	if (unhashMoveSPiece(computersMove) == 2)
		SPiece = 'g';
	printf("     %8s's move                          : %d %d %c %d\n\n", computersName, orient, orig, SPiece, SValue);
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
** CALLS:       GenerateMoves(POSITION)
**
************************************************************************/

VALUE Primitive(POSITION position) {
	MOVELIST *head = NULL, *ptr, *GenerateMoves();
	inPrimitive = 1;
	head = ptr = GenerateMoves(position);
	inPrimitive = 0;
	if (ptr == NULL)
		return(gStandardGame ? lose : win);
	else
		return(undecided);
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
** CALLS:       unhashL1(POSITION)
**              unhashL2(POSITION)
**              unhashS1(POSITION)
**              unhashS2(POSITION)
**              unhashTurn(POSITION)
**              Make48(INT, INT)
**              Make8to16(INT, INT, INT)
**              Make7to16(INT, INT, INT, INT)
**              GetPrediction()
**              unhashMoveL(MOVE)
**              unhashMoveSPiece(MOVE)
**              unhashMoveSValue(MOVE)
**              Make16to7(INT, INT, INT, INT)
**              Make16to8(INT, INT, INT)
**              Make24(INT, INT)
**              hash(INT, INT, INT, INT, INT)
**
************************************************************************/

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn) {
	int L1 = unhashL1(position);
	int L2 = unhashL2(position);
	int S1 = unhashS1(position);
	int S2 = unhashS2(position);
	int whosMove = unhashTurn(position);
	char* color;
	int i;
	char arr[17];
	L2 = Make48(L1, L2);
	S1 = Make8to16(L1, L2, S1);
	S2 = Make7to16(L1, L2, S1, S2);
	if (whosMove == 1)
		color = "X";
	else color = "O";
	for (i = 1; i<17; i++)
		arr[i] = ' ';
	for (i = 0; i<4; i++) {
		arr[FOURSQUARES[L1][i]] = 'X';
		arr[FOURSQUARES[L2][i]] = 'O';
	}
	arr[S1] = 'w';
	arr[S2] = 'g';

	printf("\n   CURRENT");
	printf("               POSSIBLE ORIENTATIONS");
	printf("                 LEGEND\n");
	printf("+-----------+   ");
	printf("+---+   +---+");
	printf("                               +-----------+\n");
	printf("|%c  %c  %c  %c |   ", arr[1], arr[2], arr[3], arr[4]);
	printf("| +-+   +-+ |     +-----+         +-+");
	printf("       |1  2  3  4 |\n");
	printf("|           |   ");
	printf("| | =1    | | =2  +---+ | =3  +---+ | =4");
	printf("    |           |\n");
	printf("|%c  %c  %c  %c |   ", arr[5], arr[6], arr[7], arr[8]);
	printf("| |       | |         +-+     +-----+");
	printf("       |5  6  7  8 |\n");
	printf("|           |   ");
	printf("+-+       +-+");
	printf("                               |           |\n");
	printf("|%c  %c  %c  %c |   ", arr[9], arr[10], arr[11], arr[12]);
	printf("  +-+     +-+");
	printf("                               |9  10 11 12|\n");
	printf("|           |   ");
	printf("  | |     | |     +-+         +-----+");
	printf("       |           |\n");
	printf("|%c  %c  %c  %c |   ", arr[13], arr[14], arr[15], arr[16]);
	printf("  | | =5  | | =6  | +---+ =7  | +---+ =8");
	printf("    |13 14 15 16|\n");
	printf("+-----------+   ");
	printf("+-+ |     | +-+   +-----+     +-+");
	printf("           +-----------+\n");
	printf("                +---+     +---+\n");
	printf("   %s's turn", color);
	printf("      %s\n\n",
	       GetPrediction(position,playerName,usersTurn));
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
** CALLS:       unhashL1(POSITION)
**              unhashL2(POSITION)
**              unhashS1(POSITION)
**              unhashS2(POSITION)
**              unhashTurn(POSITION)
**              Make48(INT, INT)
**              Make8to16(INT, INT, INT)
**              Make7to16(INT, INT, INT, INT)
**              memberOf(INT, INT)
**              hash(INT, INT, INT, INT, INT)
**              Make24(INT, INT)
**              Make16to8(INT, INT, INT)
**              Make16to7(INT, INT, INT, INT)
**              MOVELIST *CreateMovelistNode(MOVE,MOVELIST *)
**
************************************************************************/

MOVELIST *GenerateMoves(POSITION position) {
	int L1 = unhashL1(position);
	int L2 = unhashL2(position);
	int S1 = unhashS1(position);
	int S2 = unhashS2(position);
	int whosMove = unhashTurn(position);
	int newL2Piece = Make48(L1, L2);
	int newS1Piece = Make8to16(L1, Make48(L1, L2), S1);
	int newS2Piece = Make7to16(L1, Make48(L1, L2), newS1Piece, S2);
	MOVELIST *CreateMovelistNode(), *head = NULL;
	int i, j;

	if (whosMove == 1) {
		for (i = 1; i<49; i++) {
			if ((memberOf(i, FOURSQUARES[newL2Piece][0])==0) &&
			    (memberOf(i, FOURSQUARES[newL2Piece][1])==0) &&
			    (memberOf(i, FOURSQUARES[newL2Piece][2])==0) &&
			    (memberOf(i, FOURSQUARES[newL2Piece][3])==0) &&
			    (memberOf(i, newS1Piece)==0) && (memberOf(i, newS2Piece)==0) &&
			    hash(i, Make24(i, newL2Piece), Make16to8(i, newL2Piece, newS1Piece), Make16to7(i, newL2Piece, newS1Piece, newS2Piece), whosMove) != position) {
				if (!mustMove)
					head = CreateMovelistNode(hashMove(i, 0, 0), head);
				for (j = 0; j<6; j++) {
					if (white1) {
						head = CreateMovelistNode(hashMove(i, 1, Get6Empties(i, newL2Piece, newS1Piece, newS2Piece, j)), head);
						if (inPrimitive)
							return(head);
						if (!oneL)
							head = CreateMovelistNode(hashMove(i, 2, Get6Empties(i, newL2Piece, newS1Piece, newS2Piece, j)), head);
					}
					else {
						head = CreateMovelistNode(hashMove(i, 2, Get6Empties(i, newL2Piece, newS1Piece, newS2Piece, j)), head);
						if (inPrimitive)
							return(head);
						if (!oneL)
							head = CreateMovelistNode(hashMove(i, 1, Get6Empties(i, newL2Piece, newS1Piece, newS2Piece, j)), head);
					}
				}
			}
		}
	}

	else if (whosMove == 2) {
		for (i = 1; i<49; i++) {
			if ((memberOf(i, FOURSQUARES[L1][0])==0) &&
			    (memberOf(i, FOURSQUARES[L1][1])==0) &&
			    (memberOf(i, FOURSQUARES[L1][2])==0) &&
			    (memberOf(i, FOURSQUARES[L1][3])==0) &&
			    (memberOf(i, newS1Piece)==0) && (memberOf(i, newS2Piece)==0) &&
			    hash(L1, Make24(L1, i), Make16to8(L1, i, newS1Piece), Make16to7(L1, i, newS1Piece,newS2Piece), whosMove) != position && Make24(L1, i) != 0) {
				if (!mustMove)
					head = CreateMovelistNode(hashMove(i, 0, 0), head);
				for (j = 0; j<6; j++) {
					if (white1) {
						if (!oneL)
							head = CreateMovelistNode(hashMove(i, 1, Get6Empties(L1, i, newS1Piece, newS2Piece, j)), head);
						head = CreateMovelistNode(hashMove(i, 2, Get6Empties(L1, i, newS1Piece, newS2Piece, j)), head);
						if (inPrimitive)
							return(head);
					}
					else {
						if (!oneL)
							head = CreateMovelistNode(hashMove(i, 2, Get6Empties(L1, i, newS1Piece, newS2Piece, j)), head);
						head = CreateMovelistNode(hashMove(i, 1, Get6Empties(L1, i, newS1Piece, newS2Piece, j)), head);
						if (inPrimitive)
							return(head);
					}
				}
			}
		}
	}
	else printf("Invalid player number.");
	return(head);
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
** CALLS:       HandleDefaultTextInput(POSITION, MOVE, STRING)
**
************************************************************************/

USERINPUT GetAndPrintPlayersMove(thePosition, theMove, playerName)
POSITION thePosition;
MOVE *theMove;
STRING playerName;
{
	USERINPUT ret, HandleDefaultTextInput();

	do {
		printf("    Move format: ['Orientation' 'Pos of corner' 'N-Piece' 'Pos of N-Piece']\n");
		printf("%8s's move: (u)ndo/[1-8 1-16]/[1-8 1-16 'w'or'g' 1-16] : ", playerName);
		ret = HandleDefaultTextInput(thePosition, theMove, playerName);
		printf("\n");
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

BOOLEAN ValidTextInput(STRING input) {
	if (strlen(input) == 3) {
		return(input[0] >= '1' && input[0] <= '8' &&
		       input[1] == ' ' &&
		       input[2] >= '1' && input[2] <= '9');
	}
	else if (strlen(input) == 4)
		return(input[0] >= '1' && input[0] <= '8' &&
		       input[1] == ' ' &&
		       input[2] == '1' &&
		       input[3] >= '0' && input[3] <= '6');
	else if (strlen(input) == 7)
		return(input[0] >= '1' && input[0] <= '8' &&
		       input[1] == ' ' &&
		       input[2] >= '1' && input[2] <= '9' &&
		       input[3] == ' ' &&
		       (input[4] == 'w' || input[4] == 'W' ||
		        input[4] == 'g' || input[4] == 'G') &&
		       input[5] == ' ' &&
		       input[6] >= '1' && input[6] <= '9');
	else if (strlen(input) == 8 && input[3] == ' ')
		return(input[0] >= '1' && input[0] <= '8' &&
		       input[1] == ' ' &&
		       input[2] >= '1' && input[2] <= '9' &&
		       input[3] == ' ' &&
		       (input[4] == 'w' || input[4] == 'W' ||
		        input[4] == 'g' || input[4] == 'G') &&
		       input[5] == ' ' &&
		       input[6] == '1' &&
		       input[7] >= '0' && input[7] <= '6');
	else if (strlen(input) == 8)
		return(input[0] >= '1' && input[0] <= '8' &&
		       input[1] == ' ' &&
		       input[2] == '1' &&
		       input[3] >= '0' && input[3] <= '6' &&
		       input[4] == ' ' &&
		       (input[5] == 'w' || input[5] == 'W' ||
		        input[5] == 'g' || input[5] == 'G') &&
		       input[6] == ' ' &&
		       input[7] >= '1' && input[7] <= '9');
	else
		return(input[0] >= '1' && input[0] <= '8' &&
		       input[1] == ' ' &&
		       input[2] == '1' &&
		       input[3] >= '0' && input[3] <= '6' &&
		       input[4] == ' ' &&
		       (input[5] == 'w' || input[5] == 'W' ||
		        input[5] == 'g' || input[5] == 'G') &&
		       input[6] == ' ' &&
		       input[7] == '1' &&
		       input[8] >= '0' && input[8] <= '6');
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
** CALLS:       transform(INT, INT)
**              hashMove(INT, INT, INT)
**
************************************************************************/

MOVE ConvertTextInputToMove(STRING input) {
	int orient, L, SPiece, S;
	orient = input[0] - '0';
	if (strlen(input) == 3) {
		L = input[2] - '0';
		SPiece = 0;
		S = 0;
	}
	else if (strlen(input) == 4) {
		L = 10 + (input[3] - '0');
		SPiece = 0;
		S = 0;
	}
	else if (strlen(input) == 7) {
		L = input[2] - '0';
		SPiece = input[4];
		S = input[6] - '0';
	}
	else if (strlen(input) == 8) {
		if (input[3] == ' ') {
			L = input[2] - '0';
			SPiece = input[4];
			S = 10 + (input[7] - '0');
		}
		else {
			L = 10 + (input[3] - '0');
			SPiece = input[5];
			S = input[7] - '0';
		}
	}
	else {
		L = 10 + (input[3] - '0');
		SPiece = input[5];
		S = 10 + (input[8] - '0');
	}
	if (SPiece == 'w' || SPiece == 'W')
		SPiece = 1;
	else if (SPiece == 'g' || SPiece == 'G')
		SPiece = 2;
	L = transform(orient, L);
	return((MOVE) hashMove(L, SPiece, S));
}

/************************************************************************
**
** NAME:        PrintMove
**
** DESCRIPTION: Print the move in a nice format.
**
** INPUTS:      MOVE *theMove         : The move to print.
**
** CALLS:       unhashMoveL(MOVE)
**              unhashMoveSPiece(MOVE)
**              unhashMoveSValue(MOVE)
**
************************************************************************/

void PrintMove(MOVE theMove) {
	printf( "%s", MoveToString( theMove ) );
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
	STRING move = (STRING) SafeMalloc(13);

	int L = unhashMoveL(theMove);
	if (unhashMoveSPiece(theMove) == 0)
		sprintf( move, "[%d %d] ", transPairs[L][0], transPairs[L][1]);
	else if (unhashMoveSPiece(theMove) == 1)
		sprintf( move, "[%d %d %c %d] ", transPairs[L][0], transPairs[L][1], 'w', unhashMoveSValue(theMove));
	else
		sprintf( move, "[%d %d %c %d] ", transPairs[L][0], transPairs[L][1], 'g', unhashMoveSValue(theMove));

	return move;
}


/**************************************************************
** Lgame specific functions
**************************************************************/

/************************************************************************
**
** NAME:        hash
**
** DESCRIPTION: Find the hash value of two L-pieces, two neutral pieces,
**              and whose move it is.
**
** INPUTS:      INT L1       : The position of the first L-piece (1-48)
**              INT L2       : The position of the second L-piece,
                               relative to the first L-piece (1-24)
**              INT S1       : The position of the first neutral piece
**                             relative to the L-pieces (1-8)
**              INT S2       : The position of the second neutral piece,
**                             relative to all the other pieces (1-7).
**              INT whosmove : The player whose turn it is (1 or 2).
**
** OUTPUTS:     INT pos : The hash value of the board
**
** CALLS:       unhashMoveL(MOVE)
**              unhashMoveSPiece(MOVE)
**              unhashMoveSValue(MOVE)
**
************************************************************************/

int hash(int L1, int L2, int S1, int S2, int whosMove) {
	int pos;
	pos = (S2-1) + (S1-1)*7 + (L2-1)*7*8 + (L1-1)*7*8*24;
	pos = (pos << 1) | (whosMove - 1);
	return pos;
}

/************************************************************************
**
** NAME:        unhashL1
**
** DESCRIPTION: Find the position of the first L-piece given a hash
**              value.
**
** INPUTS:      INT total : The hash value of the current board
**
** OUTPUTS:     (no name) : The position of the first L-piece
**
************************************************************************/

int unhashL1(int total) {
	return (((total >> 1)/7/8/24) % 48)+1;
}

/************************************************************************
**
** NAME:        unhashLw
**
** DESCRIPTION: Find the position of the second L-piece given a hash
**              value.
**
** INPUTS:      INT total : The hash value of the current board
**
** OUTPUTS:     (no name) : The position of the second L-piece
**
************************************************************************/

int unhashL2(int total) {
	return (((total >> 1)/7/8) % 24)+1;
}

/************************************************************************
**
** NAME:        unhashS1
**
** DESCRIPTION: Find the position of the first neutral piece given a
**              hash value.
**
** INPUTS:      INT total : The hash value of the current board
**
** OUTPUTS:     (no name) : The position of the first L-piece
**
************************************************************************/

int unhashS1(int total) {
	return (((total >> 1)/7) % 8)+1;
}

/************************************************************************
**
** NAME:        unhashS2
**
** DESCRIPTION: Find the position of the second neutral piece given a
**              hash value.
**
** INPUTS:      INT total : The hash value of the current board
**
** OUTPUTS:     (no name) : The position of the first L-piece
**
************************************************************************/

int unhashS2(int total) {
	return ((total >> 1) % 7)+1;
}

/************************************************************************
**
** NAME:        unhashTurn
**
** DESCRIPTION: Find the number representing whose turn it is.
**
** INPUTS:      INT total : The hash value of the current board
**
** OUTPUTS:     (no name) : Whose turn it is.
**
************************************************************************/

int unhashTurn(int total) {
	return (total & 1) + 1;
}

/************************************************************************
**
** NAME:        hashMove
**
** DESCRIPTION: Find the hash value of a move.
**
** INPUTS:      INT L      : The position to which the L-piece is being
**                           moved (1-48)
**              INT SPiece : Which neutral piece is being moved (1 or 2)
**              INT SValue : The position to which the neutral piece is
**                           being moved (1-16).
**
** OUTPUTS:     (no name) : The hash value of the move
**
************************************************************************/

int hashMove(int L, int SPiece, int SValue) {
	return 1000*L+100*SPiece+SValue;
}

/************************************************************************
**
** NAME:        unhashMoveL
**
** DESCRIPTION: Find the new position of the L-piece.
**
** INPUTS:      INT total : The hash value of the move.
**
** OUTPUTS:     (no name) : The new position of the L-piece
**
************************************************************************/

int unhashMoveL(int total) {
	return total/1000;
}

/************************************************************************
**
** NAME:        unhashMoveSPiece
**
** DESCRIPTION: Find which neutral piece is being moved.
**
** INPUTS:      INT total : The hash value of the move.
**
** OUTPUTS:     (no name) : The neutral piece being moved (1-2).
**
************************************************************************/

int unhashMoveSPiece(int total) {
	total = total - (total/1000)*1000;
	return total/100;
}

/************************************************************************
**
** NAME:        unhashMoveSValue
**
** DESCRIPTION: Find the new position of the neutral piece.
**
** INPUTS:      INT total : The hash value of the move.
**
** OUTPUTS:     (no name) : The new position of the neutral piece.
**
************************************************************************/

int unhashMoveSValue(int total) {
	return total%100;
}

/************************************************************************
**
** NAME:        transform
**
** DESCRIPTION: Find the position of the L-piece represented by a
**              certain orientation and corner piece (origin).
**
** INPUTS:      INT orient : The orientation of the L-piece (1-8)
**              INT origL  : The location of the corner piece of the
**                           L-piece (1-16).
**
** OUTPUTS:     (no name) : The position of the L-piece.
**
************************************************************************/

int transform(int orient, int origL) {
	if (orient < 1 || orient > 8 || origL < 1 || origL > 16)
		return 0;
	else return transArr[orient-1][origL-1];
}

/************************************************************************
**
** NAME:        untransformOrient
**
** DESCRIPTION: Find the orientation of a given L-piece.
**
** INPUTS:      INT L : The L-piece being looked at.
**
** OUTPUTS:     (no name) : The orientation represented as an
**                                   integer (1-8).
**
************************************************************************/

int untransformOrient(int L) {
	int i, j;
	for (i = 0; i<8; i++) {
		for (j = 0; j<16; j++) {
			if (L == transArr[i][j])
				return (i+1);
		}
	}
	//should never reach here
	return 0;
}

/************************************************************************
**
** NAME:        untransformOrig
**
** DESCRIPTION: Find the location of a given L-piece's corner piece.
**
** INPUTS:      INT L      : The L-piece being looked at.
**              INT orient : The orientation of the L-piece.
**
** OUTPUTS:     (no name) : The corner piece location (1-16).
**
************************************************************************/

int untransformOrig(int L, int orient) {
	int i;
	for (i = 0; i<16; i++) {
		if (L == transArr[orient-1][i])
			return (i+1);
	}
	//should never be reached
	return 0;
}

/************************************************************************
**
** NAME:        printInit
**
** DESCRIPTION: Print a picture of each of the eight orienations
**              possible for L-pieces, a legend of the board and its
**              16 squares, and the current board setup, no matter how
**              many pieces have already been placed (when picking an
**              initial position).
**
** INPUTS:      INT pieces : The number of pieces already placed.
**              INT L1     : The position of the first L-piece.
**              INT L2     : The position of the second L-piece.
**              INT S1     : The position of the first neutral piece.
**              INT S2     : The position of the second neutral piece.
**
************************************************************************/

int printInit(int pieces, int L1, int L2, int S1, int S2) {
	char arr[17];
	int i;
	for (i = 1; i<17; i++)
		arr[i] = ' ';
	if (pieces > 0)
		for (i = 0; i<4; i++)
			arr[FOURSQUARES[L1][i]] = 'X';
	if (pieces > 1)
		for (i = 0; i<4; i++)
			arr[FOURSQUARES[L2][i]] = 'O';
	if (pieces > 2)
		arr[S1] = 'w';
	if (pieces > 3)
		arr[S2] = 'g';

	printf("\n         POSSIBLE ORIENTATIONS");
	printf("                 LEGEND");
	printf("         CURRENT\n");
	printf("+---+   +---+");
	printf("                               +-----------+");
	printf("  +-----------+\n");
	printf("| +-+   +-+ |     +-----+         +-+");
	printf("       |1  2  3  4 |");
	printf("  |%c  %c  %c  %c |\n", arr[1], arr[2], arr[3], arr[4]);
	printf("| | =1    | | =2  +---+ | =3  +---+ | =4");
	printf("    |           |");
	printf("  |           |\n");
	printf("| |       | |         +-+     +-----+");
	printf("       |5  6  7  8 |");
	printf("  |%c  %c  %c  %c |\n", arr[5], arr[6], arr[7], arr[8]);
	printf("+-+       +-+");
	printf("                               |           |");
	printf("  |           |\n");
	printf("   +-+    +-+");
	printf("                               |9  10 11 12|");
	printf("  |%c  %c  %c  %c |\n", arr[9], arr[10], arr[11], arr[12]);
	printf("   | |    | |     +-+         +-----+");
	printf("       |           |");
	printf("  |           |\n");
	printf("   | | =5 | | =6  | +---+ =7  | +---+ =8");
	printf("    |13 14 15 16|");
	printf("  |%c  %c  %c  %c |\n", arr[13], arr[14], arr[15], arr[16]);
	printf(" +-+ |    | +-+   +-----+     +-+");
	printf("           +-----------+");
	printf("  +-----------+\n");
	printf(" +---+    +---+\n\n");
	return 0;
}

/************************************************************************
**
** NAME:        Make24
**
** DESCRIPTION: Find the "base 24" position of the second L-piece;
**              that is, its position relative to the first L-piece.
**
** INPUTS:      INT L1 : The "base 48" position of the first L-piece.
**              INT L2 : The "base 48" position of the second L-piece.
**
** OUTPUTS:     newL2 : The "base 24" position of the second L-piece.
**
************************************************************************/

int Make24(int L1, int L2) {
	int newL2 = 0;
	int i;
	for (i = 0; i<24; i++)
		if (LPIECESREMAINING[L1][i] == L2)
			newL2 = i+1;
	return newL2;
}

/************************************************************************
**
** NAME:        Make48
**
** DESCRIPTION: Find the "base 48" position of the second L-piece;
**              that is, its position NOT relative to the first L-piece.
**
** INPUTS:      INT L1 : The "base 48" position of the first L-piece.
**              INT L2 : The "base 24" position of the second L-piece.
**
** OUTPUTS:     (no name) : The "base 48" position of the second L-piece.
**
************************************************************************/

int Make48(int L1, int L2) {
	return LPIECESREMAINING[L1][L2-1];
}

/************************************************************************
**
** NAME:        Make16to8
**
** DESCRIPTION: Find the "base 8" position of the first neutral piece;
**              that is, its position relative to the L-pieces.
**
** INPUTS:      INT L1     : The "base 48" position of the first L-piece.
**              INT L2     : The "base 48" position of the second
**                           L-piece.
**              INT S1as16 : The "base 16" location of the first neutral
**                           piece.
**
** OUTPUTS:     INT S1as8 : The "base 8" position of the first neutral
**                           piece.
**
** CALLS:       memberOf(INT, INT)
**
************************************************************************/

int Make16to8(int L1, int L2, int S1as16) {
	int S1as8 = 0;
	int i;
	int counter = 1;
	static int arr[17];
	for (i = 1; i<17; i++)
		arr[i] = 0;
	for (i = 0; i<4; i++) {
		arr[FOURSQUARES[L1][i]] = 1;
		arr[FOURSQUARES[L2][i]] = 1;
	}
	for (i = 1; i<17; i++) {
		if (i == S1as16)
			S1as8 = counter;
		else if (arr[i] == 0)
			counter++;
	}
	if (memberOf(L1, S1as16)==1 || memberOf(L2, S1as16)==1)
		S1as8 = 0;
	return S1as8;
}

/************************************************************************
**
** NAME:        Make8to16
**
** DESCRIPTION: Find the "base 16" position of the first neutral piece;
**              that is, its position NOT relative to the L-pieces.
**
** INPUTS:      INT L1    : The "base 48" position of the first L-piece.
**              INT L2    : The "base 48" position of the second L-piece.
**              INT S1as8 : The "base 8" location of the first neutral
**                          piece.
**
** OUTPUTS:     INT S1as16 : The "base 16" position of the first neutral
**                           piece.
**
************************************************************************/

int Make8to16(int L1, int L2, int S1as8) {
	int S1as16 = 0;
	int i;
	static int arr[17];
	for (i = 1; i<17; i++)
		arr[i] = 0;
	for (i = 0; i<4; i++) {
		arr[FOURSQUARES[L1][i]] = 1;
		arr[FOURSQUARES[L2][i]] = 1;
	}
	for (i = 1; i<17; i++)
		if (arr[i] == 0) {
			if (S1as8 == 1) {
				arr[i] = 2;
				S1as8 = S1as8-1;
			}
			else S1as8 = S1as8-1;
		}
	for (i = 1; i<17; i++)
		if (arr[i] == 2)
			S1as16 = i;
	return S1as16;
}

/************************************************************************
**
** NAME:        Make16to7
**
** DESCRIPTION: Find the "base 7" position of the second neutral piece;
**              that is, its position relative to the other pieces.
**
** INPUTS:      INT L1     : The "base 48" position of the first L-piece.
**              INT L2     : The "base 48" position of the second
**                           L-piece.
**              INT S1     : The "base 16" location of the first neutral
**                           piece.
**              INT S2as16 : The "base 16" location of the second neutral
**                           piece.
**
** OUTPUTS:     INT S2as8 : The "base 8" position of the second neutral
**                           piece.
**
** CALLS:       memberOf(INT, INT)
**
************************************************************************/

int Make16to7(int L1, int L2, int S1, int S2as16) {
	int S2as8;
	int i;
	int counter = 1;
	static int arr[17];
	for (i = 1; i<17; i++)
		arr[i] = 0;
	for (i = 0; i<4; i++) {
		arr[FOURSQUARES[L1][i]] = 1;
		arr[FOURSQUARES[L2][i]] = 1;
	}
	arr[S1] = 1;
	for (i = 1; i<17; i++) {
		if (i == S2as16)
			S2as8 = counter;
		else if (arr[i] == 0)
			counter++;
	}
	if (memberOf(L1, S2as16)==1 || memberOf(L2, S2as16)==1 || S1==S2as16)
		S2as8 = 0;
	return S2as8;
}

/************************************************************************
**
** NAME:        Make7to16
**
** DESCRIPTION: Find the "base 16" position of the second neutral piece;
**              that is, its position NOT relative to the other pieces.
**
** INPUTS:      INT L1    : The "base 48" position of the first L-piece.
**              INT L2    : The "base 48" position of the second L-piece.
**              INT S1    : The "base 16" location of the first neutral
**                          piece.
**              INT S2as7 : The "base 7" location of the second neutral
**                          piece.
**
** OUTPUTS:     INT S2as16 : The "base 16" position of the second neutral
**                           piece.
**
************************************************************************/

int Make7to16(int L1, int L2, int S1, int S2as7) {
	int S2as16;
	int i;
	static int arr[17];
	for (i = 1; i<17; i++)
		arr[i] = 0;
	for (i = 0; i<4; i++) {
		arr[FOURSQUARES[L1][i]] = 1;
		arr[FOURSQUARES[L2][i]] = 1;
	}
	arr[S1] = 1;
	for (i = 1; i<17; i++)
		if (arr[i] == 0) {
			if (S2as7 == 1) { //S2as7 is used as a counter
				arr[i] = 2;
				S2as7 = S2as7-1; //decrement counter to avoid error
			}
			else S2as7 = S2as7-1; //decrement counter
		}
	for (i = 1; i<17; i++)
		if (arr[i] == 2)
			S2as16 = i;
	return S2as16;
}

/************************************************************************
**
** NAME:        Get6Empties
**
** DESCRIPTION: Get one of the six empty spaces remaining on the board
**              with all the pieces in place.
**
** INPUTS:      INT L1 : The "base 48" position of the first L-piece.
**              INT L2 : The "base 48" position of the second L-piece.
**              INT S1 : The "base 16" location of the first neutral
**                       piece.
**              INT S2 : The "base 16" location of the second neutral
**                       piece.
**              INT j  : Which empty space is desired (1-6).
**
** OUTPUTS:     INT empties[j] : The location of the jth empty space.
**
************************************************************************/

int Get6Empties(int L1, int L2, int S1, int S2, int j) {
	static int empties[6];
	int counter = 0;
	int i;
	static int nonEmpties[17];
	for (i = 1; i<17; i++)
		nonEmpties[i] = 0;
	nonEmpties[0] = 1;
	nonEmpties[S1] = 1;
	nonEmpties[S2] = 1;
	for (i = 0; i<4; i++) {
		nonEmpties[FOURSQUARES[L1][i]] = 1;
		nonEmpties[FOURSQUARES[L2][i]] = 1;
	}
	for (i = 0; i<17; i++)
		if (nonEmpties[i] == 0) {
			empties[counter] = i;
			counter++;
		}
	return empties[j];
}

/************************************************************************
**
** NAME:        memberOf
**
** DESCRIPTION: See whether or not a certain square is a used by a
**              certain L-piece.
**
** INPUTS:      INT L      : The "base 48" position of the L-piece.
**              INT square : The square being tested.
**
** OUTPUTS:     ans : 1 if the square is used by the L-piece;
**                    otherwise, 0.
**
************************************************************************/

int memberOf(int L, int square) {
	int ans = 0;
	if (FOURSQUARES[L][0]==square || FOURSQUARES[L][1]==square ||
	    FOURSQUARES[L][2]==square || FOURSQUARES[L][3]==square)
		ans = 1;
	return ans;
}

/************************************************************************
**
** NAME:        clearS1
**
** DESCRIPTION: See whether or not a neutral piece can be placed in a
**              certain location (only the L-pieces have been placed.)
**
** INPUTS:      INT L1 : The "base 48" position of the first L-piece.
**              INT L2 : The "base 48" position of the second L-piece.
**              INT S1 : The "base 16" desired location of the neutral
**                       piece.
**
** OUTPUTS:     clear : 1 if the location is clear; otherwise, 0.
**
************************************************************************/

int clearS1(int L1, int L2, int S1) {
	int i, clear = 1;
	for (i = 0; i<4; i++) {
		if ((S1 == FOURSQUARES[L1][i]) || (S1 == FOURSQUARES[L2][i])) {
			clear = 0;
			break;
		}
	}
	return clear;
}

/************************************************************************
**
** NAME:        clearS2
**
** DESCRIPTION: See whether or not the second neutral piece can be placed
**              in a certain location (with all other pieces placed.)
**
** INPUTS:      INT L1 : The "base 48" position of the first L-piece.
**              INT L2 : The "base 48" position of the second L-piece.
**              INT S1 : The "base 16" location of the first neutral
**                       piece.
**              INT S2 : The "base 16" location of the second neutral
**                       piece.
**
** OUTPUTS:     clear : 1 if the location is clear; otherwise, 0.
**
************************************************************************/

int clearS2(int L1, int L2, int S1, int S2) {
	int i, clear = 1;
	for (i = 0; i<4; i++) {
		if ((S2 == FOURSQUARES[L1][i]) || (S2 == FOURSQUARES[L2][i])) {
			clear = 0;
			break;
		}
	}
	if (S1 == S2)
		clear = 0;
	return clear;
}

/************************************************************************
**
** NAME:        checkCor
**
** DESCRIPTION: See whether or not the location picked for the corner
**              piece of the L-piece is valid given its orientation.
**
** INPUTS:      INT Lo : The orienation of the L-piece.
**              INT Lc : The desired corner location of the L-piece.
**
** OUTPUTS:     b : 1 if the corner location is valid; otherwise, 0.
**
************************************************************************/

int checkCor(int Lo, int Lc) {
	int i, b = 0;
	for (i = 0; i < 6; i++)
		if (valCor[Lo][i] == Lc)
			b = 1;
	return b;
}

/************************************************************************
**
** NAME:        checkOrient
**
** DESCRIPTION: See whether or not the orientation picked for the second
**              L-piece is valid given the first L-piece.
**
** INPUTS:      INT Lo : The desired orientation of the second L-piece.
**              INT L1 : The "base 48" position of the first L-piece.
**
** OUTPUTS:     b : 1 if the orientation is valid; otherwise, 0.
**
** CALLS:       transform(INT, INT)
**
************************************************************************/

int checkOrient(int Lo, int L1) {
	int i, j, k, newL, b = 1;
	for (i = 0; i < 6; i++) {
		newL = transform(Lo, valCor[Lo][i]);
		b = 1;
		for (j = 0; j < 4; j++) {
			for (k = 0; k < 4; k++) {
				if (FOURSQUARES[newL][j] == FOURSQUARES[L1][k]) {
					b = 0;
				}
			}
		}
		if (b == 1)
			break;
	}
	return b;
}

STRING kDBName = "Lgame";

int NumberOfOptions()
{
	return 2*3*2;
}

int getOption()
{
	int option = 1;
	if(gStandardGame) option += 1;
	if(oneL) {
		if (white1) option += 1 * 2;
		else option += 2 * 2;
	}
	if (mustMove) option += 1 * (2*3);

	return option;
}

void setOption(int option)
{
	option -= 1;
	gStandardGame = option%2==1;
	oneL = option/2%3>0;
	if (oneL) {
		white1 = option/2%3==1;
	}
	mustMove = option/(2*3)%2==1;
}

POSITION StringToPosition(char* board) {
	// FIXME
	POSITION pos = 0;
	if ( GetValue(board, "pos", GetInt, &pos) ) {
		return pos;
	} else {
		return INVALID_POSITION;
	}
}


char* PositionToString(POSITION pos) {
	int len = sizeof(char) * 4 * 4 + 1;
	char * board_string = (char *) malloc(len + 1);
	int L1 = unhashL1(pos);
	int L2 = unhashL2(pos);
	int S1 = unhashS1(pos);
	int S2 = unhashS2(pos);
	int i;
	char* formatted;

	L2 = Make48(L1, L2);
	S1 = Make8to16(L1, L2, S1);
	S2 = Make7to16(L1, L2, S1, S2);

	for (i = 0; i<len; i++) {
		board_string[i] = ' ';
	}
	for (i = 0; i<4; i++) {
		board_string[FOURSQUARES[L1][i]] = 'x';
		board_string[FOURSQUARES[L2][i]] = 'o';
	}
	board_string[S1] = 'w';
	board_string[S2] = 'g';
	board_string[len] = '\0';

	formatted = MakeBoardString(board_string + 1,
	                            "turn", StrFromI(unhashTurn(pos)),
	                            "pos", StrFromI(pos),
	                            "");
	free(board_string);
	return formatted;
}

char * PositionToEndData(POSITION pos) {
	return NULL;
}
