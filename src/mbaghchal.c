/************************************************************************
**
** NAME:        mbaghchal.c
**
** DESCRIPTION: Bagh Chal (Tigers Move)
**
** AUTHOR:      Damian Hites
**              Max Delgadillo
**              Deepa Mahajan
**              Cameron Cheung
**
** DATE:        2006.9.27
**
**************************************************************************/

#include <stdio.h>
#include "gamesman.h"

/*************************************************************************
**
** Game-specific constants
**
**************************************************************************/

CONST_STRING kGameName            = "Bagh-Chal (Tigers Move)";   /* The name of your game */
CONST_STRING kAuthorName          = "Damian Hites, Max Delgadillo, Deepa Mahajan, Cameron Cheung";   /* Your name(s) */
CONST_STRING kDBName              = "baghchal";   /* The name to store the database under */

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
void* gGameSpecificTclInit = NULL;


/*
 * Help strings that are pretty self-explanatory
 * Strings than span more than one line should have backslashes (\) at the end of the line.
 */

CONST_STRING kHelpGraphicInterface =
        "Not written yet";

CONST_STRING kHelpTextInterface    =
        "On your move, if you are placing a goat, you enter the\n"
        "coordinates of the space where you would like to place\n"
        "your goat (ex: a3).  If you are moving a goat or a tiger,\n"
        "you enter the coordinates of the piece you would like to\n"
        "move and the coordinates of the space you would like to\n"
        "move the piece to (ex: a3 b3).";

CONST_STRING kHelpOnYourTurn =
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

CONST_STRING kHelpStandardObjective =
        "The objective of the goats is to try and trap the tigers\n"
        "so that they are unable to move.  The tigers are trying to\n"
        "remove every goat from the board.";

CONST_STRING kHelpReverseObjective =
        "This is not implemented because it makes the game\n"
        "really really stupid.";

CONST_STRING kHelpTieOccursWhen =
        "A tie cannot occur.";

CONST_STRING kHelpExample =
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
#define GOAT        'G'
#define TIGER       'T'
#define SPACE       '+'

int nonDiagNumMoves3[9] = {2,3,2,3,4,3,2,3,2};
int nonDiagMoveMatrix3[9][2][8] = {
	{
		{1,3,-1,-1,-1,-1,-1,-1},
		{2,6,-1,-1,-1,-1,-1,-1}
	},
	{
		{0,2,4,-1,-1,-1,-1,-1},
		{-1,-1,7,-1,-1,-1,-1,-1}
	},
	{
		{1,5,-1,-1,-1,-1,-1,-1},
		{0,8,-1,-1,-1,-1,-1,-1}
	},
	{
		{0,4,6,-1,-1,-1,-1,-1},
		{-1,5,-1,-1,-1,-1,-1,-1}
	},
	{
		{1,3,5,7,-1,-1,-1,-1},
		{-1,-1,-1,-1,-1,-1,-1,-1}
	},
	{
		{2,4,8,-1,-1,-1,-1,-1},
		{-1,3,-1,-1,-1,-1,-1,-1}
	},
	{
		{3,7,-1,-1,-1,-1,-1,-1},
		{0,8,-1,-1,-1,-1,-1,-1}
	},
	{
		{4,6,8,-1,-1,-1,-1,-1},
		{1,-1,-1,-1,-1,-1,-1,-1}
	},
	{
		{5,7,-1,-1,-1,-1,-1,-1},
		{2,6,-1,-1,-1,-1,-1,-1}
	}
};

int diagNumMoves3[9] = {3,3,3,3,8,3,3,3,3};
int diagMoveMatrix3[9][2][8] = {
	{
		{1,3,4,-1,-1,-1,-1,-1},
		{2,6,8,-1,-1,-1,-1,-1}
	},
	{
		{0,2,4,-1,-1,-1,-1,-1},
		{-1,-1,7,-1,-1,-1,-1,-1}
	},
	{
		{1,4,5,-1,-1,-1,-1,-1},
		{0,6,8,-1,-1,-1,-1,-1}
	},
	{
		{0,4,6,-1,-1,-1,-1,-1},
		{-1,5,-1,-1,-1,-1,-1,-1}
	},
	{
		{0,1,2,3,5,6,7,8},
		{-1,-1,-1,-1,-1,-1,-1,-1}
	},
	{
		{2,4,8,-1,-1,-1,-1,-1},
		{-1,3,-1,-1,-1,-1,-1,-1}
	},
	{
		{3,4,7,-1,-1,-1,-1,-1},
		{0,2,8,-1,-1,-1,-1,-1}
	},
	{
		{4,6,8,-1,-1,-1,-1,-1},
		{1,-1,-1,-1,-1,-1,-1,-1}
	},
	{
		{4,5,7,-1,-1,-1,-1,-1},
		{0,2,6,-1,-1,-1,-1,-1}
	}
};

int nonDiagNumMoves4[16] = {2,3,3,2,3,4,4,3,3,4,4,3,2,3,3,2};
int nonDiagMoveMatrix4[16][2][8] = {
	{
		{1,4,-1,-1,-1,-1,-1,-1},
		{2,8,-1,-1,-1,-1,-1,-1}
	},
	{
		{0,2,5,-1,-1,-1,-1,-1},
		{-1,3,9,-1,-1,-1,-1,-1}
	},
	{
		{1,3,6,-1,-1,-1,-1,-1},
		{0,-1,10,-1,-1,-1,-1,-1}
	},
	{
		{2,7,-1,-1,-1,-1,-1,-1},
		{1,11,-1,-1,-1,-1,-1,-1}
	},
	{
		{0,5,8,-1,-1,-1,-1,-1},
		{-1,6,12,-1,-1,-1,-1,-1}
	},
	{
		{1,4,6,9,-1,-1,-1,-1},
		{-1,-1,7,13,-1,-1,-1,-1}
	},
	{
		{2,5,7,10,-1,-1,-1,-1},
		{-1,4,-1,14,-1,-1,-1,-1}
	},
	{
		{3,6,11,-1,-1,-1,-1,-1},
		{-1,5,15,-1,-1,-1,-1,-1}
	},
	{
		{4,9,12,-1,-1,-1,-1,-1},
		{0,10,-1,-1,-1,-1,-1,-1}
	},
	{
		{5,8,10,13,-1,-1,-1,-1},
		{1,-1,11,-1,-1,-1,-1,-1}
	},
	{
		{6,9,11,14,-1,-1,-1,-1},
		{2,8,-1,-1,-1,-1,-1,-1}
	},
	{
		{7,10,15,-1,-1,-1,-1,-1},
		{3,9,-1,-1,-1,-1,-1,-1}
	},
	{
		{8,13,-1,-1,-1,-1,-1,-1},
		{4,14,-1,-1,-1,-1,-1,-1}
	},
	{
		{9,12,14,-1,-1,-1,-1,-1},
		{5,-1,15,-1,-1,-1,-1,-1}
	},
	{
		{10,13,15,-1,-1,-1,-1,-1},
		{6,12,-1,-1,-1,-1,-1,-1}
	},
	{
		{11,14,-1,-1,-1,-1,-1,-1},
		{7,13,-1,-1,-1,-1,-1,-1}
	},
};

int diagNumMoves4[16] = {3,3,5,2,3,8,4,5,5,4,8,3,2,5,3,3};
int diagMoveMatrix4[16][2][8] = {
	{
		{1,4,5,-1,-1,-1,-1,-1},
		{2,8,10,-1,-1,-1,-1,-1}
	},
	{
		{0,2,5,-1,-1,-1,-1,-1},
		{-1,3,9,-1,-1,-1,-1,-1}
	},
	{
		{1,3,5,6,7,-1,-1,-1},
		{0,-1,8,10,-1,-1,-1,-1}
	},
	{
		{2,7,-1,-1,-1,-1,-1,-1},
		{1,11,-1,-1,-1,-1,-1,-1}
	},
	{
		{0,5,8,-1,-1,-1,-1,-1},
		{-1,6,12,-1,-1,-1,-1,-1}
	},
	{
		{0,1,2,4,6,8,9,10},
		{-1,-1,-1,-1,7,-1,13,15}
	},
	{
		{2,5,7,10,-1,-1,-1,-1},
		{-1,4,-1,14,-1,-1,-1,-1}
	},
	{
		{2,3,6,10,11,-1,-1,-1},
		{-1,-1,5,13,15,-1,-1,-1}
	},
	{
		{4,5,9,12,13,-1,-1,-1},
		{0,2,10,-1,-1,-1,-1,-1}
	},
	{
		{5,8,10,13,-1,-1,-1,-1},
		{1,-1,11,-1,-1,-1,-1,-1}
	},
	{
		{5,6,7,9,11,13,14,15},
		{0,2,-1,8,-1,-1,-1,-1}
	},
	{
		{7,10,15,-1,-1,-1,-1,-1},
		{3,9,-1,-1,-1,-1,-1,-1}
	},
	{
		{8,13,-1,-1,-1,-1,-1,-1},
		{4,14,-1,-1,-1,-1,-1,-1}
	},
	{
		{8,9,10,12,14,-1,-1,-1},
		{-1,5,7,-1,15,-1,-1,-1}
	},
	{
		{10,13,15,-1,-1,-1,-1,-1},
		{6,12,-1,-1,-1,-1,-1,-1}
	},
	{
		{10,11,14,-1,-1,-1,-1,-1},
		{5,7,13,-1,-1,-1,-1,-1}
	},
};

int nonDiagNumMoves5[25] = {2,3,3,3,2,3,4,4,4,3,3,4,4,4,3,3,4,4,4,3,2,3,3,3,2};
int nonDiagMoveMatrix5[25][2][8] = {
	{
		{1,5,-1,-1,-1,-1,-1,-1},
		{2,10,-1,-1,-1,-1,-1,-1}
	},
	{
		{0,2,6,-1,-1,-1,-1,-1},
		{-1,3,11,-1,-1,-1,-1,-1}
	},
	{
		{1,3,7,-1,-1,-1,-1,-1},
		{0,4,12,-1,-1,-1,-1,-1}
	},
	{
		{2,4,8,-1,-1,-1,-1,-1},
		{1,-1,13,-1,-1,-1,-1,-1}
	},
	{
		{3,9,-1,-1,-1,-1,-1,-1},
		{2,14,-1,-1,-1,-1,-1,-1}
	},
	{
		{0,6,10,-1,-1,-1,-1,-1},
		{-1,7,15,-1,-1,-1,-1,-1}
	},
	{
		{1,5,7,11,-1,-1,-1,-1},
		{-1,-1,8,16,-1,-1,-1,-1}
	},
	{
		{2,6,8,12,-1,-1,-1,-1},
		{-1,5,9,17,-1,-1,-1,-1}
	},
	{
		{3,7,9,13,-1,-1,-1,-1},
		{-1,6,-1,18,-1,-1,-1,-1}
	},
	{
		{4,8,14,-1,-1,-1,-1,-1},
		{-1,7,19,-1,-1,-1,-1,-1}
	},
	{
		{5,11,15,-1,-1,-1,-1,-1},
		{0,12,20,-1,-1,-1,-1,-1}
	},
	{
		{6,10,12,16,-1,-1,-1,-1},
		{1,-1,13,21,-1,-1,-1,-1}
	},
	{
		{7,11,13,17,-1,-1,-1,-1},
		{2,10,14,22,-1,-1,-1,-1}
	},
	{
		{8,12,14,18,-1,-1,-1,-1},
		{3,11,-1,23,-1,-1,-1,-1}
	},
	{
		{9,13,19,-1,-1,-1,-1,-1},
		{4,12,24,-1,-1,-1,-1,-1}
	},
	{
		{10,16,20,-1,-1,-1,-1,-1},
		{5,17,-1,-1,-1,-1,-1,-1}
	},
	{
		{11,15,17,21,-1,-1,-1,-1},
		{6,-1,18,-1,-1,-1,-1,-1}
	},
	{
		{12,16,18,22,-1,-1,-1,-1},
		{7,15,19,-1,-1,-1,-1,-1}
	},
	{
		{13,17,19,23,-1,-1,-1,-1},
		{8,16,-1,-1,-1,-1,-1,-1}
	},
	{
		{14,18,24,-1,-1,-1,-1,-1},
		{9,17,-1,-1,-1,-1,-1,-1}
	},
	{
		{15,21,-1,-1,-1,-1,-1,-1},
		{10,22,-1,-1,-1,-1,-1,-1}
	},
	{
		{16,20,22,-1,-1,-1,-1,-1},
		{11,-1,23,-1,-1,-1,-1,-1}
	},
	{
		{17,21,23,-1,-1,-1,-1,-1},
		{12,20,24,-1,-1,-1,-1,-1}
	},
	{
		{18,22,24,-1,-1,-1,-1,-1},
		{13,21,-1,-1,-1,-1,-1,-1}
	},
	{
		{19,23,-1,-1,-1,-1,-1,-1},
		{14,22,-1,-1,-1,-1,-1,-1}
	}
};

int diagNumMoves5[25] = {3,3,5,3,3,3,8,4,8,3,5,4,8,4,5,3,8,4,8,3,3,3,5,3,3};
int diagMoveMatrix5[25][2][8] = {
	{
		{1,5,6,-1,-1,-1,-1,-1},
		{2,10,12,-1,-1,-1,-1,-1}
	},
	{
		{0,2,6,-1,-1,-1,-1,-1},
		{-1,3,11,-1,-1,-1,-1,-1}
	},
	{
		{1,3,6,7,8,-1,-1,-1},
		{0,4,10,12,14,-1,-1,-1}
	},
	{
		{2,4,8,-1,-1,-1,-1,-1},
		{1,-1,13,-1,-1,-1,-1,-1}
	},
	{
		{3,8,9,-1,-1,-1,-1,-1},
		{2,12,14,-1,-1,-1,-1,-1}
	},
	{
		{0,6,10,-1,-1,-1,-1,-1},
		{-1,7,15,-1,-1,-1,-1,-1}
	},
	{
		{0,1,2,5,7,10,11,12},
		{-1,-1,-1,-1,8,-1,16,18}
	},
	{
		{2,6,8,12,-1,-1,-1,-1},
		{-1,5,9,17,-1,-1,-1,-1}
	},
	{
		{2,3,4,7,9,12,13,14},
		{-1,-1,-1,6,-1,16,18,-1}
	},
	{
		{4,8,14,-1,-1,-1,-1,-1},
		{-1,7,19,-1,-1,-1,-1,-1}
	},
	{
		{5,6,11,15,16,-1,-1,-1},
		{0,2,12,20,22,-1,-1,-1}
	},
	{
		{6,10,12,16,-1,-1,-1,-1},
		{1,-1,13,21,-1,-1,-1,-1}
	},
	{
		{6,7,8,11,13,16,17,18},
		{0,2,4,10,14,20,22,24}
	},
	{
		{8,12,14,18,-1,-1,-1,-1},
		{3,11,-1,23,-1,-1,-1,-1}
	},
	{
		{8,9,13,18,19,-1,-1,-1},
		{2,4,12,22,24,-1,-1,-1}
	},
	{
		{10,16,20,-1,-1,-1,-1,-1},
		{5,17,-1,-1,-1,-1,-1,-1}
	},
	{
		{10,11,12,15,17,20,21,22},
		{-1,6,8,-1,18,-1,-1,-1}
	},
	{
		{12,16,18,22,-1,-1,-1,-1},
		{7,15,19,-1,-1,-1,-1,-1}
	},
	{
		{12,13,14,17,19,22,23,24},
		{6,8,-1,16,-1,-1,-1,-1}
	},
	{
		{14,18,24,-1,-1,-1,-1,-1},
		{9,17,-1,-1,-1,-1,-1,-1}
	},
	{
		{15,16,21,-1,-1,-1,-1,-1},
		{10,12,22,-1,-1,-1,-1,-1}
	},
	{
		{16,20,22,-1,-1,-1,-1,-1},
		{11,-1,23,-1,-1,-1,-1,-1}
	},
	{
		{16,17,18,21,23,-1,-1,-1},
		{10,12,14,20,24,-1,-1,-1}
	},
	{
		{18,22,24,-1,-1,-1,-1,-1},
		{13,21,-1,-1,-1,-1,-1,-1}
	},
	{
		{18,19,23,-1,-1,-1,-1,-1},
		{12,14,22,-1,-1,-1,-1,-1}
	}
};

int gSymmetryMatrix3[8][25] = {
    {0,1,2,3,4,5,6,7,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {6,3,0,7,4,1,8,5,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {8,7,6,5,4,3,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {2,5,8,1,4,7,0,3,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {2,1,0,5,4,3,8,7,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,3,6,1,4,7,2,5,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {6,7,8,3,4,5,0,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {8,5,2,7,4,1,6,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

// TYPICAL DIHEDRAL GROUP D4 SYMMETRIES DO NOT WORK FOR 4x4 WITH DIAGONALS
// IT WORKS FOR 4x4 WITHOUT DIAGONALS
int gSymmetryMatrix4[8][25] = {
	{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0},
	{3,2,1,0,7,6,5,4,11,10,9,8,15,14,13,12,0,0,0,0,0,0,0,0,0},
	{12,8,4,0,13,9,5,1,14,10,6,2,15,11,7,3,0,0,0,0,0,0,0,0,0},
	{0,4,8,12,1,5,9,13,2,6,10,14,3,7,11,15,0,0,0,0,0,0,0,0,0},
	{15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,0,0,0,0,0,0,0,0,0},
	{12,13,14,15,8,9,10,11,4,5,6,7,0,1,2,3,0,0,0,0,0,0,0,0,0},
	{3,7,11,15,2,6,10,14,1,5,9,13,0,4,8,12,0,0,0,0,0,0,0,0,0},
	{15,11,7,3,14,10,6,2,13,9,5,1,12,8,4,0,0,0,0,0,0,0,0,0,0}
};

int gSymmetryMatrix5[8][25] = {
	{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24},
	{4,3,2,1,0,9,8,7,6,5,14,13,12,11,10,19,18,17,16,15,24,23,22,21,20},
	{20,15,10,5,0,21,16,11,6,1,22,17,12,7,2,23,18,13,8,3,24,19,14,9,4},
	{0,5,10,15,20,1,6,11,16,21,2,7,12,17,22,3,8,13,18,23,4,9,14,19,24},
	{24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0},
	{20,21,22,23,24,15,16,17,18,19,10,11,12,13,14,5,6,7,8,9,0,1,2,3,4},
	{4,9,14,19,24,3,8,13,18,23,2,7,12,17,22,1,6,11,16,21,0,5,10,15,20},
	{24,19,14,9,4,23,18,13,8,3,22,17,12,7,2,21,16,11,6,1,20,15,10,5,0}
};

POSITION combinations[26][5][21] = {{{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},{{1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},{{1,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},{{1,3,3,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{3,6,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},{{1,4,6,4,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{4,12,12,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{6,12,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},{{1,5,10,10,5,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{5,20,30,20,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{10,30,30,10,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{10,20,10,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},{{1,6,15,20,15,6,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{6,30,60,60,30,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{15,60,90,60,15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{20,60,60,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{15,30,15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},{{1,7,21,35,35,21,7,1,0,0,0,0,0,0,0,0,0,0,0,0,0},{7,42,105,140,105,42,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{21,105,210,210,105,21,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{35,140,210,140,35,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{35,105,105,35,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},{{1,8,28,56,70,56,28,8,1,0,0,0,0,0,0,0,0,0,0,0,0},{8,56,168,280,280,168,56,8,0,0,0,0,0,0,0,0,0,0,0,0,0},{28,168,420,560,420,168,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{56,280,560,560,280,56,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{70,280,420,280,70,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},{{1,9,36,84,126,126,84,36,9,1,0,0,0,0,0,0,0,0,0,0,0},{9,72,252,504,630,504,252,72,9,0,0,0,0,0,0,0,0,0,0,0,0},{36,252,756,1260,1260,756,252,36,0,0,0,0,0,0,0,0,0,0,0,0,0},{84,504,1260,1680,1260,504,84,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{126,630,1260,1260,630,126,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},{{1,10,45,120,210,252,210,120,45,10,1,0,0,0,0,0,0,0,0,0,0},{10,90,360,840,1260,1260,840,360,90,10,0,0,0,0,0,0,0,0,0,0,0},{45,360,1260,2520,3150,2520,1260,360,45,0,0,0,0,0,0,0,0,0,0,0,0},{120,840,2520,4200,4200,2520,840,120,0,0,0,0,0,0,0,0,0,0,0,0,0},{210,1260,3150,4200,3150,1260,210,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},{{1,11,55,165,330,462,462,330,165,55,11,1,0,0,0,0,0,0,0,0,0},{11,110,495,1320,2310,2772,2310,1320,495,110,11,0,0,0,0,0,0,0,0,0,0},{55,495,1980,4620,6930,6930,4620,1980,495,55,0,0,0,0,0,0,0,0,0,0,0},{165,1320,4620,9240,11550,9240,4620,1320,165,0,0,0,0,0,0,0,0,0,0,0,0},{330,2310,6930,11550,11550,6930,2310,330,0,0,0,0,0,0,0,0,0,0,0,0,0}},{{1,12,66,220,495,792,924,792,495,220,66,12,1,0,0,0,0,0,0,0,0},{12,132,660,1980,3960,5544,5544,3960,1980,660,132,12,0,0,0,0,0,0,0,0,0},{66,660,2970,7920,13860,16632,13860,7920,2970,660,66,0,0,0,0,0,0,0,0,0,0},{220,1980,7920,18480,27720,27720,18480,7920,1980,220,0,0,0,0,0,0,0,0,0,0,0},{495,3960,13860,27720,34650,27720,13860,3960,495,0,0,0,0,0,0,0,0,0,0,0,0}},{{1,13,78,286,715,1287,1716,1716,1287,715,286,78,13,1,0,0,0,0,0,0,0},{13,156,858,2860,6435,10296,12012,10296,6435,2860,858,156,13,0,0,0,0,0,0,0,0},{78,858,4290,12870,25740,36036,36036,25740,12870,4290,858,78,0,0,0,0,0,0,0,0,0},{286,2860,12870,34320,60060,72072,60060,34320,12870,2860,286,0,0,0,0,0,0,0,0,0,0},{715,6435,25740,60060,90090,90090,60060,25740,6435,715,0,0,0,0,0,0,0,0,0,0,0}},{{1,14,91,364,1001,2002,3003,3432,3003,2002,1001,364,91,14,1,0,0,0,0,0,0},{14,182,1092,4004,10010,18018,24024,24024,18018,10010,4004,1092,182,14,0,0,0,0,0,0,0},{91,1092,6006,20020,45045,72072,84084,72072,45045,20020,6006,1092,91,0,0,0,0,0,0,0,0},{364,4004,20020,60060,120120,168168,168168,120120,60060,20020,4004,364,0,0,0,0,0,0,0,0,0},{1001,10010,45045,120120,210210,252252,210210,120120,45045,10010,1001,0,0,0,0,0,0,0,0,0,0}},{{1,15,105,455,1365,3003,5005,6435,6435,5005,3003,1365,455,105,15,1,0,0,0,0,0},{15,210,1365,5460,15015,30030,45045,51480,45045,30030,15015,5460,1365,210,15,0,0,0,0,0,0},{105,1365,8190,30030,75075,135135,180180,180180,135135,75075,30030,8190,1365,105,0,0,0,0,0,0,0},{455,5460,30030,100100,225225,360360,420420,360360,225225,100100,30030,5460,455,0,0,0,0,0,0,0,0},{1365,15015,75075,225225,450450,630630,630630,450450,225225,75075,15015,1365,0,0,0,0,0,0,0,0,0}},{{1,16,120,560,1820,4368,8008,11440,12870,11440,8008,4368,1820,560,120,16,1,0,0,0,0},{16,240,1680,7280,21840,48048,80080,102960,102960,80080,48048,21840,7280,1680,240,16,0,0,0,0,0},{120,1680,10920,43680,120120,240240,360360,411840,360360,240240,120120,43680,10920,1680,120,0,0,0,0,0,0},{560,7280,43680,160160,400400,720720,960960,960960,720720,400400,160160,43680,7280,560,0,0,0,0,0,0,0},{1820,21840,120120,400400,900900,1441440,1681680,1441440,900900,400400,120120,21840,1820,0,0,0,0,0,0,0,0}},{{1,17,136,680,2380,6188,12376,19448,24310,24310,19448,12376,6188,2380,680,136,17,1,0,0,0},{17,272,2040,9520,30940,74256,136136,194480,218790,194480,136136,74256,30940,9520,2040,272,17,0,0,0,0},{136,2040,14280,61880,185640,408408,680680,875160,875160,680680,408408,185640,61880,14280,2040,136,0,0,0,0,0},{680,9520,61880,247520,680680,1361360,2042040,2333760,2042040,1361360,680680,247520,61880,9520,680,0,0,0,0,0,0},{2380,30940,185640,680680,1701700,3063060,4084080,4084080,3063060,1701700,680680,185640,30940,2380,0,0,0,0,0,0,0}},{{1,18,153,816,3060,8568,18564,31824,43758,48620,43758,31824,18564,8568,3060,816,153,18,1,0,0},{18,306,2448,12240,42840,111384,222768,350064,437580,437580,350064,222768,111384,42840,12240,2448,306,18,0,0,0},{153,2448,18360,85680,278460,668304,1225224,1750320,1969110,1750320,1225224,668304,278460,85680,18360,2448,153,0,0,0,0},{816,12240,85680,371280,1113840,2450448,4084080,5250960,5250960,4084080,2450448,1113840,371280,85680,12240,816,0,0,0,0,0},{3060,42840,278460,1113840,3063060,6126120,9189180,10501920,9189180,6126120,3063060,1113840,278460,42840,3060,0,0,0,0,0,0}},{{1,19,171,969,3876,11628,27132,50388,75582,92378,92378,75582,50388,27132,11628,3876,969,171,19,1,0},{19,342,2907,15504,58140,162792,352716,604656,831402,923780,831402,604656,352716,162792,58140,15504,2907,342,19,0,0},{171,2907,23256,116280,406980,1058148,2116296,3325608,4157010,4157010,3325608,2116296,1058148,406980,116280,23256,2907,171,0,0,0},{969,15504,116280,542640,1763580,4232592,7759752,11085360,12471030,11085360,7759752,4232592,1763580,542640,116280,15504,969,0,0,0,0},{3876,58140,406980,1763580,5290740,11639628,19399380,24942060,24942060,19399380,11639628,5290740,1763580,406980,58140,3876,0,0,0,0,0}},{{1,20,190,1140,4845,15504,38760,77520,125970,167960,184756,167960,125970,77520,38760,15504,4845,1140,190,20,1},{20,380,3420,19380,77520,232560,542640,1007760,1511640,1847560,1847560,1511640,1007760,542640,232560,77520,19380,3420,380,20,0},{190,3420,29070,155040,581400,1627920,3527160,6046560,8314020,9237800,8314020,6046560,3527160,1627920,581400,155040,29070,3420,190,0,0},{1140,19380,155040,775200,2713200,7054320,14108640,22170720,27713400,27713400,22170720,14108640,7054320,2713200,775200,155040,19380,1140,0,0,0},{4845,77520,581400,2713200,8817900,21162960,38798760,55426800,62355150,55426800,38798760,21162960,8817900,2713200,581400,77520,4845,0,0,0,0}},{{1,21,210,1330,5985,20349,54264,116280,203490,293930,352716,352716,293930,203490,116280,54264,20349,5985,1330,210,21},{21,420,3990,23940,101745,325584,813960,1627920,2645370,3527160,3879876,3527160,2645370,1627920,813960,325584,101745,23940,3990,420,21},{210,3990,35910,203490,813960,2441880,5697720,10581480,15872220,19399380,19399380,15872220,10581480,5697720,2441880,813960,203490,35910,3990,210,0},{1330,23940,203490,1085280,4069800,11395440,24690120,42325920,58198140,64664600,58198140,42325920,24690120,11395440,4069800,1085280,203490,23940,1330,0,0},{5985,101745,813960,4069800,14244300,37035180,74070360,116396280,145495350,145495350,116396280,74070360,37035180,14244300,4069800,813960,101745,5985,0,0,0}},{{1,22,231,1540,7315,26334,74613,170544,319770,497420,646646,705432,646646,497420,319770,170544,74613,26334,7315,1540,231},{22,462,4620,29260,131670,447678,1193808,2558160,4476780,6466460,7759752,7759752,6466460,4476780,2558160,1193808,447678,131670,29260,4620,462},{231,4620,43890,263340,1119195,3581424,8953560,17907120,29099070,38798760,42678636,38798760,29099070,17907120,8953560,3581424,1119195,263340,43890,4620,231},{1540,29260,263340,1492260,5969040,17907120,41783280,77597520,116396280,142262120,142262120,116396280,77597520,41783280,17907120,5969040,1492260,263340,29260,1540,0},{7315,131670,1119195,5969040,22383900,62674920,135795660,232792560,320089770,355655300,320089770,232792560,135795660,62674920,22383900,5969040,1119195,131670,7315,0,0}},{{1,23,253,1771,8855,33649,100947,245157,490314,817190,1144066,1352078,1352078,1144066,817190,490314,245157,100947,33649,8855,1771},{23,506,5313,35420,168245,605682,1716099,3922512,7354710,11440660,14872858,16224936,14872858,11440660,7354710,3922512,1716099,605682,168245,35420,5313},{253,5313,53130,336490,1514205,5148297,13728792,29418840,51482970,74364290,89237148,89237148,74364290,51482970,29418840,13728792,5148297,1514205,336490,53130,5313},{1771,35420,336490,2018940,8580495,27457584,68643960,137287920,223092870,297457160,327202876,297457160,223092870,137287920,68643960,27457584,8580495,2018940,336490,35420,1771},{8855,168245,1514205,8580495,34321980,102965940,240253860,446185740,669278610,818007190,818007190,669278610,446185740,240253860,102965940,34321980,8580495,1514205,168245,8855,0}},{{1,24,276,2024,10626,42504,134596,346104,735471,1307504,1961256,2496144,2704156,2496144,1961256,1307504,735471,346104,134596,42504,10626},{24,552,6072,42504,212520,807576,2422728,5883768,11767536,19612560,27457584,32449872,32449872,27457584,19612560,11767536,5883768,2422728,807576,212520,42504},{276,6072,63756,425040,2018940,7268184,20593188,47070144,88256520,137287920,178474296,194699232,178474296,137287920,88256520,47070144,20593188,7268184,2018940,425040,63756},{2024,42504,425040,2691920,12113640,41186376,109830336,235350720,411863760,594914320,713897184,713897184,594914320,411863760,235350720,109830336,41186376,12113640,2691920,425040,42504},{10626,212520,2018940,12113640,51482970,164745504,411863760,823727520,1338557220,1784742960,1963217256,1784742960,1338557220,823727520,411863760,164745504,51482970,12113640,2018940,212520,10626}},{{1,25,300,2300,12650,53130,177100,480700,1081575,2042975,3268760,4457400,5200300,5200300,4457400,3268760,2042975,1081575,480700,177100,53130},{25,600,6900,50600,265650,1062600,3364900,8652600,18386775,32687600,49031400,62403600,67603900,62403600,49031400,32687600,18386775,8652600,3364900,1062600,265650},{300,6900,75900,531300,2656500,10094700,30284100,73547100,147094200,245157000,343219800,405623400,405623400,343219800,245157000,147094200,73547100,30284100,10094700,2656500,531300},{2300,50600,531300,3542000,16824500,60568200,171609900,392251200,735471000,1144066000,1487285800,1622493600,1487285800,1144066000,735471000,392251200,171609900,60568200,16824500,3542000,531300},{12650,265650,2656500,16824500,75710250,257414850,686439600,1470942000,2574148500,3718214500,4461857400,4461857400,3718214500,2574148500,1470942000,686439600,257414850,75710250,16824500,2656500,265650}}};

char initialBaghchalInteractString3[] = "R_A_4_3_T-T---T-T0-0";
char initialBaghchalInteractString4[] = "R_A_5_4_T--T--------T--T00-0";
char initialBaghchalInteractString5[] = "R_A_6_5_T---T---------------T---T00-00";

int boardToStringIdxMapping[25] = {8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32};
int boardToGridIdxMapping[25] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24};

int goatsLeftTensIdx5 = 33;
int goatsLeftOnesIdx5 = 34;
int goatsCapturedTensIdx5 = 36;
int goatsCapturedOnesIdx5 = 37;

int goatsLeftTensIdx4 = 24;
int goatsLeftOnesIdx4 = 25;
int goatsCapturedTensIdx4 = 27;
int goatsCapturedOnesIdx4 = 27;

int goatsLeftTensIdx3 = 17;
int goatsLeftOnesIdx3 = 17;
int goatsCapturedTensIdx3 = 19;
int goatsCapturedOnesIdx3 = 19;

/*************************************************************************
**
** Global Variables
**
*************************************************************************/
int sideLength = 5;
int boardSize  = 25;
int tigers     = 4;
int goats      = 20;
int goatCaptureGoal = 5;
int (*symmetriesToUse)[25] = gSymmetryMatrix5;
int (*numMoves) = diagNumMoves5;
int (*moveMatrix)[2][8] = diagMoveMatrix5;

STRING initialBaghchalInteractString = initialBaghchalInteractString5;
int goatsLeftTensIdx = 45;
int goatsLeftOnesIdx = 46;
int goatsCapturedTensIdx = 48;
int goatsCapturedOnesIdx = 49;

BOOLEAN diagonals = TRUE;

POSITION genericHashMaxPos = 0; //saves a function call

/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/
int translate(int x, int y);
int get_x(int index);
int get_y(int index);
POSITION hash(char*, int, int, char);
char* TclUnhash(POSITION);
char* unhash(POSITION, int*, int*,char*);
void ChangeBoardSize();
STRING MoveToString(MOVE);
POSITION GetCanonicalPosition(POSITION position);
POSITION DoSymmetry(POSITION position, int symmetry);
POSITION GetInitialPosition();

//TIER GAMESMAN
void SetupTierStuff();
TIERLIST* TierChildren(TIER);
TIERPOSITION NumberOfTierPositions(TIER);
BOOLEAN IsLegal(POSITION);
UNDOMOVELIST* GenerateUndoMovesToTier(POSITION, TIER);
POSITION UnDoMove(POSITION, UNDOMOVE);
STRING TierToString(TIER);

void unhashTier(TIER, int*, int*, char*);
TIER hashTier(int, int, char);
void hashBoard(char *board, int goatsLeft, int goatsCaptured, char turn, TIER *tier, TIERPOSITION *tierposition);
char* unhashToBoard(TIER tier, TIERPOSITION tierposition, int *goatsLeft, int *goatsCaptured, char *turn, char *board);
void unhashMove(MOVE, int*, int*, int*);
MOVE hashMove(int, int, int);
void unhashCacheInit();
void hashCachePut(TIER tier, TIERPOSITION tierposition, char *board, char turn, int goatsLeft, int goatsCaptured);
BOOLEAN hashCacheGet(TIER tier, POSITION tierposition, char *board, char *turn, int *goatsLeft, int *goatsCaptured);

typedef struct {
	TIER tier;
	TIERPOSITION tierposition;
	char board[25];
	char turn;
	int goatsLeft;
	int goatsCaptured;
} UNHASH_RECORD;

#define NUM_UNHASH_RECORDS 0b100000000000000
#define UNHASH_MASK        0b011111111111111

UNHASH_RECORD unhashRecords[NUM_UNHASH_RECORDS];

int unhashCacheInited = 0;

void unhashCacheInit() {
	for (long i = 0; i < NUM_UNHASH_RECORDS; i++) {
		unhashRecords[i].tierposition = -1LL;
	}
	unhashCacheInited = 1;
}

void hashCachePut(TIER tier, TIERPOSITION tierposition, char *board, char turn, int goatsLeft, int goatsCaptured) {
	if (!unhashCacheInited) unhashCacheInit();

	long i = tierposition & UNHASH_MASK;
	if (unhashRecords[i].tier != tier ||
		unhashRecords[i].tierposition != tierposition) {
		unhashRecords[i].tier = tier;
		unhashRecords[i].tierposition = tierposition;
		memcpy(unhashRecords[i].board, board, boardSize);
		unhashRecords[i].turn = turn;
		unhashRecords[i].goatsLeft = goatsLeft;
		unhashRecords[i].goatsCaptured = goatsCaptured;
	}
}

// Returns TRUE if cache miss, otherwise FALSE
BOOLEAN hashCacheGet(TIER tier, POSITION tierposition, char *board, char *turn, int *goatsLeft, int *goatsCaptured) {
	if (!unhashCacheInited) unhashCacheInit();

	long i = tierposition & UNHASH_MASK;
	if (unhashRecords[i].tier == tier &&
		unhashRecords[i].tierposition == tierposition) {
		memcpy(board, unhashRecords[i].board, boardSize);
		(*turn) = unhashRecords[i].turn;
		(*goatsLeft) = unhashRecords[i].goatsLeft;
		(*goatsCaptured) = unhashRecords[i].goatsCaptured;
		return FALSE;
	}
	return TRUE;
}

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

STRING copy(STRING toCopy) {
	STRING toReturn = SafeMalloc((strlen(toCopy)+1) * sizeof(char));
	strcpy(toReturn, toCopy);
	return toReturn;
}

void *fakeUnhash(POSITION p) {
	return (void *) copy("balhblahblah");
}

void InitializeGame() {

	/* FOR THE PURPOSES OF INTERACT. FEEL FREE TO CHANGE IF SOLVING. */ 
	if (gIsInteract) {
		gLoadTierdbArray = FALSE; // SET TO TRUE IF SOLVING
	}
	/********************************/

	gSymmetries = TRUE;
	gCanonicalPosition = GetCanonicalPosition;
	gMoveToStringFunPtr = &MoveToString;

	kSupportsTierGamesman = TRUE;
	kExclusivelyTierGamesman = TRUE;

	gTierChildrenFunPtr = &TierChildren;
	gNumberOfTierPositionsFunPtr = &NumberOfTierPositions;
	gTierToStringFunPtr = &TierToString;

	gUnDoMoveFunPtr = &UnDoMove;
	gGenerateUndoMovesToTierFunPtr = &GenerateUndoMovesToTier;

	setOption(getOption());

	unhashCacheInit();

	gCustomUnhash = &TclUnhash;
	linearUnhash = &fakeUnhash;
}


/************************************************************************
**
** NAME:        GenerateMoves
**
************************************************************************/

MOVELIST *GenerateMoves(POSITION position) {
	int goatsLeft, goatsCaptured;
	char turn;
	char *board = unhash(position, &goatsLeft, &goatsCaptured, &turn);
	if (goatsCaptured >= goatCaptureGoal) {
		SafeFree(board);
		return NULL;
	}
	MOVELIST *moves = NULL;
	for (int i = 0; i < boardSize; i++) {
		if (turn == GOAT && goatsLeft && board[i] == SPACE) {
			moves = CreateMovelistNode(hashMove(i, i, i), moves);
		} else if (board[i] == turn && (turn == TIGER || !goatsLeft)) {
			for (int j = 0; j < numMoves[i]; j++) {
				int adjIndex = moveMatrix[i][0][j];
				int jumpIndex =  moveMatrix[i][1][j];
				if (board[adjIndex] == SPACE)
					moves = CreateMovelistNode(hashMove(i, adjIndex, adjIndex), moves);
				else if (turn == TIGER && jumpIndex != -1 && board[adjIndex] == GOAT && board[jumpIndex] == SPACE)
					moves = CreateMovelistNode(hashMove(i, jumpIndex, adjIndex), moves);
			}
		}
	}
	SafeFree(board);
	return moves;
}


/************************************************************************
**
** NAME:        DoMove
**
*************************************************************************/

POSITION DoMove(POSITION position, MOVE move) {
	int goatsLeft, goatsCaptured;
	char turn;
	char *board = unhash(position, &goatsLeft, &goatsCaptured, &turn);
	int from, to, remove;
	unhashMove(move, &from, &to, &remove);

	board[to] = turn;

	if (from == to) goatsLeft--;
	else board[from] = SPACE;
	
	if (remove != to) {
		board[remove] = SPACE;
		goatsCaptured++;
	}

	turn = (turn == GOAT) ? TIGER : GOAT;
	POSITION toReturn = hash(board, goatsLeft, goatsCaptured, turn);
	SafeFree(board);
	return toReturn;
}

/************************************************************************
**
** NAME:        Primitive
**
************************************************************************/

VALUE Primitive(POSITION position) {
	MOVELIST* moves = GenerateMoves(position);
	if (moves == NULL) return lose;
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

void PrintPosition(POSITION position, STRING playersName, BOOLEAN usersTurn) {
	int i, j;
	int goatsLeft, goatsCaptured;
	char turn;
	char* board = unhash(position, &goatsLeft, &goatsCaptured, &turn);
	printf("\t%s's Turn (%s):\n  ", playersName, (turn == GOAT ? "Goat" : "Tiger"));
	for (i = 1; i <= sideLength + 1; i++) { // print the rows one by one
		if (i <= sideLength) {
			printf("\t%d ", sideLength - i + 1); // first, print the row with the pieces
			for (j = 1; j <= sideLength; j++) {
				printf("%c", board[translate(i, j)]);
				if (j < sideLength)
					printf("-");
			}
			printf("\n\t  ");
			if (i < sideLength) { // then, print the row with the lines (diagonals and such)
				for (j = 1; j < sideLength; j++) {
					if (diagonals) {
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

					if (j == sideLength - 1)
						printf("|");
				}
			}
			if (i == 1) {
				if (goatsLeft != 0) printf("  <STAGE 1> Goats To Place: %d, Goats Captured: %d", goatsLeft, goatsCaptured);
				else printf("  <STAGE 2> Goats Captured: %d", goatsCaptured);
			} else if (i == 2)
				printf("    %s", GetPrediction(position, playersName, usersTurn));
			if(i<sideLength) printf("\n");
		} else if (i > sideLength) {
			for (j = 1; j <= sideLength; j++) // print the column numbers
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

void PrintComputersMove (MOVE computersMove, STRING computersName) {
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

void PrintMove (MOVE move) {
	STRING s = MoveToString(move);
	printf("%s", s);
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

USERINPUT GetAndPrintPlayersMove(POSITION position, MOVE *move, STRING playersName) {
	USERINPUT input;
	int goatsLeft, goatsCaptured;
	char turn;
	char *board = unhash(position, &goatsLeft, &goatsCaptured, &turn);
	SafeFree(board);

	for (;;) {
		/***********************************************************
		* CHANGE THE LINE BELOW TO MATCH YOUR MOVE FORMAT
		***********************************************************/
		if(turn == GOAT && goatsLeft > 0) //stage1
			printf("%8s's move [(undo)/([%c-%c][%d-%d])] : ", playersName, 'a', sideLength + 'a' - 1, 1, sideLength);
		else
			printf("%8s's move [(undo)/([%c-%c][%d-%d] [%c-%c][%d-%d])] : ", playersName, 'a', sideLength + 'a' - 1, 1, sideLength, 'a', sideLength+'a' - 1, 1, sideLength);
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
************************************************************************/

BOOLEAN ValidTextInput(STRING input) {
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
************************************************************************/

MOVE ConvertTextInputToMove(STRING input) {
	int y = input[0] - 'a' + 1;
	int x = sideLength - input[1] + '1';
	if (input[2] == '\0') {
		int idx = translate(x,y);
		return hashMove(idx, idx, idx);
	}
	int y1 = input[2] - 'a' + 1;
	int x1 = sideLength - input[3] + '1';
	
	int removeX, removeY;
	if (x1 > x)
		removeX = (x1 > x + 1) ? x1 - 1 : x1;
	else
		removeX = (x1 < x - 1) ? x1 + 1 : x1;
	
	if (y1 > y)
		removeY = (y1 > y + 1) ? y1 - 1 : y1;
	else
		removeY = (y1 < y - 1) ? y1 + 1 : y1;

	return hashMove(translate(x, y), translate(x1, y1), translate(removeX, removeY));
}


/************************************************************************
**
** NAME:        GameSpecificMenu
**
** DESCRIPTION: Prints, receives, and sets game-specific parameters.
**
**
************************************************************************/

void GameSpecificMenu() {
	BOOLEAN cont = TRUE;
	while (cont) {
		printf("\n\nCurrent %dx%d board with %d goats:  \n", sideLength, sideLength, goats);
		printf("\tGame Options:\n\n"
		       "\t1)\t(1) Load default 3x3 Game (4 Goats, Tigers Must Remove 1, Diagonals)\n"
		       "\t2)\t(2) Load default 4x4 Game (12 Goats, Tigers Must Remove 3. No Diagonals)\n"
		       "\t3)\t(3) Load default 5x5 Game (20 Goats, Tigers Must Remove 5, Diagonals)\n"
		       "\tc)\t(C)hange the board size (nxn), currently: %d\n"
		       "\td)\tTurn (D)iagonals %s\n"
		       //"\ti)\tSet the (I)nitial position (starting position)\n"
		       "\tb)\t(B)ack to the main menu\n"
		       "\nSelect an option:  ", sideLength, diagonals ? "off" : "on");
		switch (GetMyChar()) {
		case '1':
			setOption(1);
			break;
		case '2':
			setOption(2);
			break;
		case '3':
			setOption(5);
			break;
		case 'c': case 'C':
			ChangeBoardSize();
			break;
		case 'd': case 'D':
			setOption(getOption() ^ 1);
			break;
		/*
		case 'i': case 'I':
			SetInitialPosition();
			break;
		*/
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

void DebugMenu() {
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
************************************************************************/

POSITION GetInitialPosition() {
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

int NumberOfOptions() {
	return 6;
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

int getOption() {
	return 2 * (sideLength - 3) + ((diagonals) ? 1 : 0);
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

void setOption(int option) {
	diagonals = (option % 2 == 1);
	sideLength = (option / 2) + 3;
	boardSize = sideLength * sideLength;
	
	if (sideLength == 5) {
		goats = 20;
		goatCaptureGoal = 5;
		symmetriesToUse = gSymmetryMatrix5;
		numMoves = (diagonals) ? diagNumMoves5 : nonDiagNumMoves5;
		moveMatrix = (diagonals) ? diagMoveMatrix5 : nonDiagMoveMatrix5;

		initialBaghchalInteractString = initialBaghchalInteractString5;
		goatsLeftTensIdx = goatsLeftTensIdx5;
		goatsLeftOnesIdx = goatsLeftOnesIdx5;
		goatsCapturedTensIdx = goatsCapturedTensIdx5;
		goatsCapturedOnesIdx = goatsCapturedOnesIdx5;
		
		char board[25] = {
			TIGER,SPACE,SPACE,SPACE,TIGER,
			SPACE,SPACE,SPACE,SPACE,SPACE,
			SPACE,SPACE,SPACE,SPACE,SPACE,
			SPACE,SPACE,SPACE,SPACE,SPACE,
			TIGER,SPACE,SPACE,SPACE,TIGER
		};
		hashBoard(board, 20, 0, GOAT, &gInitialTier, &gInitialTierPosition);

	} else if (sideLength == 4) {
		goats = 12;
		goatCaptureGoal = 3;
		symmetriesToUse = gSymmetryMatrix4;
		numMoves = (diagonals) ? diagNumMoves4 : nonDiagNumMoves4;
		moveMatrix = (diagonals) ? diagMoveMatrix4 : nonDiagMoveMatrix4;

		initialBaghchalInteractString = initialBaghchalInteractString4;
		goatsLeftTensIdx = goatsLeftTensIdx4;
		goatsLeftOnesIdx = goatsLeftOnesIdx4;
		goatsCapturedTensIdx = goatsCapturedTensIdx4;
		goatsCapturedOnesIdx = goatsCapturedOnesIdx4;
		
		char board[16] = {
			TIGER,SPACE,SPACE,TIGER,
			SPACE,SPACE,SPACE,SPACE,
			SPACE,SPACE,SPACE,SPACE,
			TIGER,SPACE,SPACE,TIGER
		};
		hashBoard(board, 12, 0, GOAT, &gInitialTier, &gInitialTierPosition);

	} else {
		goats = 4;
		goatCaptureGoal = 1;
		symmetriesToUse = gSymmetryMatrix3;
		numMoves = (diagonals) ? diagNumMoves3 : nonDiagNumMoves3;
		moveMatrix = (diagonals) ? diagMoveMatrix3 : nonDiagMoveMatrix3;

		initialBaghchalInteractString = initialBaghchalInteractString3;
		goatsLeftTensIdx = goatsLeftTensIdx3;
		goatsLeftOnesIdx = goatsLeftOnesIdx3;
		goatsCapturedTensIdx = goatsCapturedTensIdx3;
		goatsCapturedOnesIdx = goatsCapturedOnesIdx3;

		char board[9] = {
			TIGER,SPACE,TIGER,
			SPACE,SPACE,SPACE,
			TIGER,SPACE,TIGER};
		hashBoard(board, 4, 0, GOAT, &gInitialTier, &gInitialTierPosition);
	}

	gInitialPosition = gInitialTierPosition;
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

int translate (int x, int y) {
	return (x-1)*sideLength+(y-1);
}

int get_x (int index) {
	return index / sideLength + 1;
}

int get_y (int index) {
	return index % sideLength + 1;
}

void hashBoard(char *board, int goatsLeft, int goatsCaptured, char turn, TIER *tier, TIERPOSITION *tierposition) {
	POSITION sum = 0;
	int numGoats = goats - goatsLeft - goatsCaptured;
	int numTigers = tigers;
    for (int i = boardSize - 1; i > 0; i--) { // no need to calculate i == 0
        switch (board[i]) {
            case GOAT:
                numGoats--;
                break;
            case TIGER:
                if (numGoats > 0) sum += combinations[i][numTigers][numGoats-1];
                numTigers--;
                break;
            case SPACE:
                if (numGoats > 0) sum += combinations[i][numTigers][numGoats-1];
                if (numTigers > 0) sum += combinations[i][numTigers-1][numGoats];
                break;
        }
    }
	(*tier) = hashTier(goatsLeft, goatsCaptured, turn);
	(*tierposition) = sum;
	if (goatsLeft == 0 && turn == TIGER) {
		(*tierposition) += (NumberOfTierPositions((*tier)) >> 1);
	}
}

POSITION hash(char* board, int goatsLeft, int goatsCaptured, char turn) {
	if (gHashWindowInitialized) {
		TIER tier; TIERPOSITION tierposition;
		hashBoard(board, goatsLeft, goatsCaptured, turn, &tier, &tierposition);
		POSITION position = gHashToWindowPosition(tierposition, tier);
		return position;
	} else { // Not supported.
		return 0;
	}
}

char* unhashToBoard(TIER tier, TIERPOSITION tierposition, int *goatsLeft, int *goatsCaptured, char *turn, char *board) {
	char fakeTurn;
	unhashTier(tier, goatsLeft, goatsCaptured, &fakeTurn);
	if ((*goatsLeft) > 0) {
		(*turn) = fakeTurn;
	} else {
		POSITION half = NumberOfTierPositions(tier) >> 1;
		(*turn) = GOAT;
		if (tierposition >= half) {
			(*turn) = TIGER;
			tierposition -= half;
		}
	}
	POSITION o1, o2;
	int numGoats = goats - (*goatsLeft) - (*goatsCaptured);
	int numTigers = tigers;
    for (int i = boardSize - 1; i >= 0; i--) {
        o1 = (numGoats > 0) ? combinations[i][numTigers][numGoats-1] : 0;
        o2 = o1 + ((numTigers > 0) ? combinations[i][numTigers-1][numGoats] : 0);
        if (tierposition >= o2) {
            board[i] = SPACE;
            tierposition -= o2;
        }
        else if (tierposition >= o1) {
            if (numTigers > 0) {
                board[i] = TIGER;
                numTigers--;
            } else
                board[i] = SPACE;
            tierposition -= o1;
        }
        else {
            if (numGoats > 0) {
                board[i] = GOAT;
                numGoats--;
            } else if (numTigers > 0) {
                board[i] = TIGER;
                numTigers--;
            } else
                board[i] = SPACE;
        }
    }
	return board;
}

char* unhash(POSITION position, int *goatsLeft, int *goatsCaptured, char *turn) {
	if (gHashWindowInitialized) {
		TIER tier; TIERPOSITION tierposition;
		gUnhashToTierPosition(position, &tierposition, &tier);

		char* board = (char*) SafeMalloc(boardSize * sizeof(char));
		BOOLEAN cache_miss = hashCacheGet(tier, tierposition, board, turn, goatsLeft, goatsCaptured);
		if (cache_miss) {
			unhashToBoard(tier, tierposition, goatsLeft, goatsCaptured, turn, board);
			hashCachePut(tier, tierposition, board, (*turn), (*goatsLeft), (*goatsCaptured));
		}
		return board;
	} else { // Not supported.
		return NULL;
	}
}

MOVE hashMove(int from, int to, int remove) {
	return (from << 10) | (to << 5) | remove;
}

void unhashMove(MOVE move, int *from, int *to, int *remove) {
	(*from) = move >> 10;
	(*to) = (move >> 5) & 0x1F;
	(*remove) = move & 0x1F;
}

char* TclUnhash(POSITION position) {
	/*
	char* TclBoard = (char*) SafeMalloc((boardSize+3) * sizeof(char));
	int goatsLeft, goatsCaptured;
	char turn;
	char* board = unhash(position, &goatsLeft, &goatsCaptured, &turn);

	TclBoard[0] = turn + 48;
	TclBoard[1] = ((goatsLeft /= 10) % 10) + 48;
	TclBoard[2] = (goatsLeft % 10) + 48;
	int i;
	for (i = 0; i < boardSize; i++)
		TclBoard[i+3] = board[i];
	if (board != NULL)
		SafeFree(board);
	return TclBoard;*/
	return NULL;
}

void ChangeBoardSize() {
	int change;
	BOOLEAN cont = TRUE;
	while (cont) {
		cont = FALSE;
		printf("\n\nCurrent board of size %d:\n\n", sideLength);
		PrintPosition(gInitialPosition, "", 0);
		printf("\n\nEnter the new board size (%d - %d):  ", 3, 5);
		change = GetMyChar() - 48;
		if (change > 5 || change < 3) {
			printf("\nInvalid base length!\n");
			cont = TRUE;
		} else {
			setOption(2 * (change - 3) + ((diagonals) ? 1 : 0));
		}
	}
}

STRING MoveToString(MOVE move) {
	int from, to, remove;
	unhashMove(move, &from, &to, &remove);
	STRING moveStr = (STRING) SafeMalloc(sizeof(char) * 4);
	int fromX = get_x(from);
	int fromY = get_y(from);
	int toX = get_x(to);
	int toY = get_y(to);
	if (from == to) { // Placement
		sprintf(moveStr, "%c%c%c%c", ' ', ' ', toY + 'a' - 1, '0' + sideLength - toX + 1);
	} else { // Movement
		sprintf(moveStr, "%c%c%c%c", fromY + 'a' - 1, '0' + sideLength - fromX + 1, toY + 'a' - 1, '0' + sideLength - toX + 1);
	}
	return moveStr;
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

POSITION GetCanonicalPosition(POSITION position) {
	char turn;
	int goatsLeft, goatsPlaced;
	char *originalBoard = unhash(position, &goatsLeft, &goatsPlaced, &turn);
	char canonBoard[25];
    POSITION canonPos = position;
    int bestSymmetryNum = 0;

    for (int symmetryNum = 1; symmetryNum < 8; symmetryNum++)
        for (int i = boardSize - 1; i >= 0; i--) {
            char pieceInSymmetry = originalBoard[symmetriesToUse[symmetryNum][i]];
            char pieceInBest = originalBoard[symmetriesToUse[bestSymmetryNum][i]];
            if (pieceInSymmetry != pieceInBest) {
                if (pieceInSymmetry > pieceInBest) // If new smallest hash.
                    bestSymmetryNum = symmetryNum;
                break;
            }
        };

    if (bestSymmetryNum == 0) {
		SafeFree(originalBoard);
        return position;
	}
    
    for (int i = 0; i < boardSize; i++) // Transform the rest of the board.
        canonBoard[i] = originalBoard[symmetriesToUse[bestSymmetryNum][i]];

    canonPos = hash(canonBoard, goatsLeft, goatsPlaced, turn);
	
	SafeFree(originalBoard);

    return canonPos;
}

/***************************
 ******  TIER API    *******
 ***************************/

void unhashTier(TIER tier, int *goatsLeft, int *goatsCaptured, char *turn) {
	(*turn) = (tier >> 10) ? TIGER : GOAT;
	(*goatsLeft) = tier >> 5 & 0x1F;
	(*goatsCaptured) = tier & 0x1F;
}

TIER hashTier(int goatsLeft, int goatsCaptured, char turn) {
	// Assumes goatsCaptured is fewer than 5 bits
	if (goatsLeft == 0) turn = TIGER;
	return (((turn == TIGER) ? 1 : 0) << 10) | (goatsLeft << 5) | goatsCaptured;
}

TIERLIST* TierChildren(TIER tier) {
	TIERLIST* list = NULL;
	int goatsLeft, goatsCaptured;
	char turn;
	unhashTier(tier, &goatsLeft, &goatsCaptured, &turn);
	if (goatsCaptured < goatCaptureGoal) {
		if (goatsLeft) { // STAGE 1
			if (turn == GOAT) {
				list = CreateTierlistNode(hashTier(goatsLeft - 1, goatsCaptured, TIGER), list);
			} else {
				list = CreateTierlistNode(hashTier(goatsLeft, goatsCaptured, GOAT), list);
				list = CreateTierlistNode(hashTier(goatsLeft, goatsCaptured + 1, GOAT), list);
			}
		} else { // STAGE 2
			list = CreateTierlistNode(tier, list);
			if (goatsLeft + goatsCaptured < goats) {
				list = CreateTierlistNode(hashTier(goatsLeft, goatsCaptured + 1, TIGER), list);
			}
		}
	}
	return list;
}

TIERPOSITION NumberOfTierPositions(TIER tier) {
	int goatsLeft, goatsCaptured;
	char turn;
	unhashTier(tier, &goatsLeft, &goatsCaptured, &turn);
	int goatsOnBoard = goats - goatsLeft - goatsCaptured;
	return ((goatsLeft == 0) ? 2 : 1) * combinations[boardSize][tigers][goatsOnBoard];
}

POSITION UnDoMove(POSITION position, UNDOMOVE undoMove) {
	int goatsLeft, goatsCaptured;
	char turn;
	char *board = unhash(position, &goatsLeft, &goatsCaptured, &turn);
	char oppTurn = (turn == GOAT) ? TIGER : GOAT;
	int from, to, remove;
	unhashMove(undoMove, &from, &to, &remove);

	board[to] = SPACE;
	if (to == from) {
		goatsLeft++;
	} else {
		board[from] = oppTurn;
	}
	if (to != remove) {
		board[remove] = GOAT;
		goatsCaptured--;
	}
	turn = oppTurn;

	POSITION toReturn = hash(board, goatsLeft, goatsCaptured, turn);
	SafeFree(board);
	return toReturn;
}

UNDOMOVELIST *GenerateUndoMovesToTier(POSITION position, TIER tier) {
	UNDOMOVELIST *undoMoves = NULL;
	int goatsLeft, goatsCaptured;
	char turn;
	char *board = unhash(position, &goatsLeft, &goatsCaptured, &turn);
	char oppTurn = (turn == GOAT) ? TIGER : GOAT;

	int toGoatsLeft, toGoatsCaptured;
	char toTurn;
	unhashTier(tier, &toGoatsLeft, &toGoatsCaptured, &toTurn);
	if (toGoatsLeft == goatsLeft && toGoatsCaptured == goatsCaptured && (oppTurn == TIGER || !goatsLeft)) { // Reverse a slide
		for (int i = 0; i < boardSize; i++) {
			if (board[i] == oppTurn) {
				for (int j = 0; j < numMoves[i]; j++) {
					int adjIndex = moveMatrix[i][0][j];
					if (board[adjIndex] == SPACE) {
						undoMoves = CreateUndoMovelistNode(hashMove(adjIndex, i, i), undoMoves);
					}
				}
			}
		}
	} else if (toGoatsLeft == goatsLeft + 1 && oppTurn == GOAT) { // Reverse a goat placement
		for (int i = 0; i < boardSize; i++) {
			if (board[i] == GOAT) {
				undoMoves = CreateUndoMovelistNode(hashMove(i, i, i), undoMoves);
			}
		}
	} else if (toGoatsCaptured == goatsCaptured - 1 && oppTurn == TIGER) { // Reverse a tiger capture
		for (int i = 0; i < boardSize; i++) {
			if (board[i] == TIGER) {
				for (int j = 0; j < numMoves[i]; j++) {
					int adjIndex = moveMatrix[i][0][j];
					int jumpIndex =  moveMatrix[i][1][j];
					if (board[adjIndex] == SPACE && jumpIndex != -1 && board[jumpIndex] == SPACE) {
						undoMoves = CreateUndoMovelistNode(hashMove(jumpIndex, i, adjIndex), undoMoves);
					}
				}
			}
		}
	}

	return undoMoves;
}

STRING TierToString(TIER tier) {
	STRING tierStr = (STRING) SafeMalloc(sizeof(char) * 64);
	int goatsLeft, goatsCaptured;
	char turn;
	unhashTier(tier, &goatsLeft, &goatsCaptured, &turn);
	int goatsOnBoard = goats - goatsLeft - goatsCaptured;

	if (goatsLeft)
		sprintf(tierStr, "Stage 1: %d Goats On Board, %d To Place, %d Goats Removed, %s's Turn", goatsOnBoard, goatsLeft, goatsCaptured, (turn == GOAT) ? "Goat" : "Tiger");
	else
		sprintf(tierStr, "Stage 2: %d Goats On Board, %d Goats Removed", goatsOnBoard, goatsCaptured);
	
	return tierStr;
}

STRING InteractPositionToString(POSITION pos) {
	char* finalBoard = calloc(51, sizeof(char));
	memcpy(finalBoard, initialBaghchalInteractString, 50);
	char turn;
	int goatsLeft, goatsCaptured;
	char *board = unhash(pos, &goatsLeft, &goatsCaptured, &turn);
	for (int i = 0; i < boardSize; i++) {
		finalBoard[boardToStringIdxMapping[i]] = (board[i] == SPACE) ? '-' : board[i];
	}
	finalBoard[2] = (turn == GOAT) ? 'A' : 'B';
	finalBoard[goatsLeftTensIdx] = (goatsLeft / 10) + '0';
	finalBoard[goatsLeftOnesIdx] = (goatsLeft % 10) + '0';
	finalBoard[goatsCapturedTensIdx] = (goatsCaptured / 10) + '0';
	finalBoard[goatsCapturedOnesIdx] = (goatsCaptured % 10) + '0';
	SafeFree(board);
	return finalBoard;
}

POSITION InteractStringToPosition(STRING string) {
	int goatsLeft, goatsCaptured;
	char turn = (string[2] == 'A') ? GOAT : TIGER;
	char fboard[boardSize];
	for (int i = 0; i < boardSize; i++) {
		char piece = string[boardToStringIdxMapping[i]];
		fboard[i] = (piece == '-') ? SPACE : piece;
	}

	if (sideLength == 3) {
		goatsLeft = string[goatsLeftOnesIdx] - '0';
	} else {
		goatsLeft = 10 * (string[goatsLeftTensIdx] - '0') + (string[goatsLeftOnesIdx] - '0');
	}

	if (sideLength == 5) {
		goatsCaptured = 10 * (string[goatsCapturedTensIdx] - '0') + (string[goatsCapturedOnesIdx] - '0');
	} else {
		goatsCaptured = (string[goatsCapturedOnesIdx] - '0');
	}

	TIER tier;
	TIERPOSITION tierposition;
	hashBoard(fboard, goatsLeft, goatsCaptured, turn, &tier, &tierposition);
	gInitializeHashWindow(tier, FALSE);
	return tierposition;
}


STRING InteractMoveToString(POSITION pos, MOVE mv) {
  int from, to, remove;
  unhashMove(mv, &from, &to, &remove);
  if (from == to) {
	  return UWAPI_Board_Regular2D_MakeAddString('-', boardToGridIdxMapping[to]);
  } else {
	  return UWAPI_Board_Regular2D_MakeMoveString(boardToGridIdxMapping[from], boardToGridIdxMapping[to]);
  }
}


/*
UPDATE HIST: -2004.10.21 = Original (Dom's) Version
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
*/
