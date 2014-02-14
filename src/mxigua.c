// $id$
// $log$

/*
 * The above lines will include the name and log of the last person
 * to commit this file to CVS
 */

/************************************************************************
**
** NAME:        xigua.c
**
** DESCRIPTION: XiGua
**
** AUTHOR:      Joshua Kocher, Daniel Honegger, Gerardo Snyder
**
** DATE:        1.27.05 - ????
**
** UPDATE HIST: RECORD CHANGES YOU HAVE MADE SO THAT TEAMMATES KNOW
**              -- 1.27.05 -- Created file and started adding in game info. -JK
**		-- 1.30.05 -- Added to displayasciitable.
**                            Filled in most of PrintPosition.
**                            Still need code to unhash.  -JK
**		-- 2.9.05 -- Filled in code for the following functions:
**                           - NumberOfOptions()
**			     - getOption()
**                           - setOption()
**              -- 2.23.05 -- Fixed Initialize game (set gNumberOfPositions and gInitialPosition)
**                            Filled in DoMove and GenerateMoves();
**                            Actually allocated space for the board in PrintPosition
**              -- 3.01.05 -- Added in wrapped hash functions
**			      Wrote Primitive()
**                            Changed functions to reflect the new hash functions
**			      Get a floating point exception now though when trying to go into the debug
**			      menu.
**			      Also cannot represent more than 6 empty spaces...maybe something to do with
**		              my new hash functions. Need the symmetries.
**              -- 3.01.05 -- Switched to SafeMalloc and SafeFree
**                            Made #define NUM_HASHED_WRAPPER_BITS to hold '5' for use in hash wrapper
**                            Filled in GenerateMoves to be more selective (use isValidMove()) (incomplete)
**                             - uses adjacency information implemented for DoMove
**                             - uses isSurrounded
**                            DoMove now captures stones; requires helpers
**                             - isSurrounded
**                             - removeStones
**                             - adjacency information initialized in InitializeGame
**                            Note: solving algorithm does not account for "pass" ability - plays out all outcomes to unquestionable end (unlike game of Go between humans) to enable this, use Chinese rules for Go territory counting
**	        -- 3.06.05 -- Fixed compile time errors.
**	                      Created a game specific menu, (currently only changes boardsize)
**			      Created smaller board sizes (5 board, have functions for 9, 13, 17)
**              -- 3.07.05 -- Created more boards (9, 13, and 17)
**			      Wrote ValidTextInput() and ConvertTextInputToMove()
**			            PrintComputersMove(), PrintMove(), getmovechar(),
**				    GetAndPrintPlayersMove()
**
**              -- 3.08.05 -- Filled in adjacency info for new boards
**                            Fixed Primitive() to 1) comply with win condition
**                                                 2) count "territory" not actually filled with stones
**                            Filled in isValidMove() (still incomplete)
**	        -- 3.08.05 -- Fixed compile time errors.
**			      Wrote GetInitialPosition()
**              -- 3.09.05 -- Finally completed isValidMove()
**		-- 3.9.05 -- Fixed problem with ocount and scount not being initialized in isValidMove()
**		-- 4.5.05 -- fixed issues with the prediction not being displayed
**			     made the debug menu look like the other menus
**			     fixed a floating point exception bug in the hash piece array (introduced by me).
**            -- 04.14.05 -- Added symmetries
**                           changed most function declarations to old C-style
**                           few minuteia such as SafeMallocs, other debugging
**
**		-- 8.19.06	commented out fflush and added in GetMyInt()
**				seg fault in intialize game, someone fix
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

/* didn't see hash.h included in gamesman.h */
#include "hash.h"

/*************************************************************************
**
** Game-specific constants
**
**************************************************************************/

char EMPTYSPACE ='-';
char PLAYER1 = 'X';
char PLAYER2 = 'O';

STRING kGameName            = "XiGua";   /* The name of your game */
STRING kAuthorName          = "Joshua Kocher, Daniel Honegger, Gerardo Snyder";   /* Your name(s) */
STRING kDBName              = "xigua";   /* The name to store the database under */

BOOLEAN kPartizan            = TRUE;   /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN kGameSpecificMenu    = TRUE;   /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN kTieIsPossible       = TRUE;   /* TRUE if a tie is possible. FALSE if it is impossible.*/
BOOLEAN kLoopy               = FALSE;   /* TRUE if the game tree will have cycles (a rearranger style game). FALSE if it does not.*/

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
        "Selecting a move is done by picking a letter corresponding with a square on the board. The board displayed will list the key to base your selection from (Note: not all spots on the board will be possible at any given time).";

STRING kHelpOnYourTurn =
        "Pick a spot listed in the list of valid moves. Your piece will be placed there and either that will be it or a capture move will be made. A capture move is made when a player has blocked all exit points for a piece. Exit moves are denoted by an edge in the board.";

STRING kHelpStandardObjective =
        "The objective of the game is to be the player with the most pieces on the board when the last piece is placed.";

STRING kHelpReverseObjective =
        "The objective of the game is to be the player with the least number of pieces of the board when the last piece is placed.";

STRING kHelpTieOccursWhen =
        "A tie occurs when both players have the same number of pieces on the board when the last piece is placed.";

STRING kHelpExample =
        "Legend:    1         Current:      �\n          /|\\        Player1: X   /|\\\n         2-3-4       Player2: *  �-�-�\n          \\|/                     \\|/\n           5                       �\n Prediction: (Computer should Lose in 8)\n\nComputer's move         : 5\nLegend:    1         Current:      �\n          /|\\        Player1: X   /|\\\n         2-3-4       Player2: *  �-�-�\n          \\|/                     \\|/\n           5                       X\n Prediction: (Player should Win in 7)\n\n  Player's move [(undo)/(4,3,2,1)] : {3}\n";


/*************************************************************************
**
** #defines and structs
**
**************************************************************************/


/*************************************************************************
**
** Global Variables
**
*************************************************************************/

#define NUM_HASH_WRAPPER_BITS 5

/* Game options */
int boardsize; /* 0-4 Board size is 5 + 4 * boardsize */
int numpieces=-1; /* default is maxsize-1 */
int rulesvariant; /* 0/1 normal rules / inverted rules */
int handicapping; /* 0/1 Handicapped or not */
int towin; /* 0-2  0 - counting territory, 1 - captured pieces, 2 - both */

/* max board size */
int maxsize;

char *board;
char *symmboard;
BOOLEAN *checked;

/*struct with adjacency information - built in Initialize Game */
typedef struct adjacency_info {
	int numAdjacent;
	MOVE *adj;
} adjacency;

adjacency *adjacent;

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

void* gGameSpecificTclInit = NULL;

/* Internal */
void displayasciiboard(char *, char *);
int NumberOfOptions();
int getOption();
void setOption(int);
char *emptyboard(char *);
POSITION hash(char *,int, int);
char *unhashboard(POSITION,char *);
int hash_init(int, int*, int (*vcfg_function_ptr)(int* cfg));
int getplayer(POSITION);
int getturnnumber(POSITION);
int countboard(char *,char);
char *getprediction(char *);
BOOLEAN isValidMove(char *, MOVE, char);      /* helper for GenerateMoves */
VALUE countWinner(POSITION);              /* helper for Primitive */
BOOLEAN isSurrounded(char *, MOVE, char, BOOLEAN *);  /* helper for DoMove, isValidMove */
void zeroChecked(); /* helper for InitializeGame, isSurrounded, isTerritory */
BOOLEAN isTerritory(char *, MOVE, char, BOOLEAN *);  /* helper for Primitive */
void removeStones(char *, MOVE, char, BOOLEAN *); /* helper for DoMove */
char getmovechar(MOVE);

STRING MoveToString( MOVE );

/**************************************************/
/**************** SYMMETRY FUN BEGIN **************/
/**************************************************/

BOOLEAN kSupportsSymmetries = FALSE; /* Whether we support symmetries - false until full boardsize selected */

#define NUMSYMMETRIES 8   /*  4 rotations, 4 flipped rotations */

int **gSymmetryMatrix;
/* This is the array used for flipping along the N-S axis */
int * gFlipNewPosition;

/* This is the array used for rotating 90 degrees clockwise */
int *gRotate90CWNewPosition;

POSITION GetCanonicalPosition(POSITION position);

/**************************************************/
/**************** SYMMETRY FUN END ****************/
/**************************************************/

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
	/* need to change this to reflect the board size */
	maxsize=5+4*boardsize;
	//if(numpieces==-1) numpieces=maxsize-1;
	numpieces=maxsize-1;
	int piecesarray[]={PLAYER1,0,numpieces,PLAYER2,0,numpieces,EMPTYSPACE,0,maxsize,-1};

	/* manually setting options right now */
	gNumberOfPositions=hash_init(maxsize, piecesarray, NULL);    /* initialize the hash */
	board = emptyboard(board);
	symmboard = emptyboard(board);
	gInitialPosition=hash(board,1,0);


	/**************************************************/
	/**************** SYMMETRY FUN BEGIN **************/
	/**************************************************/

	if(boardsize == 4) {
		kSupportsSymmetries = TRUE; /* Whether we support symmetries - false until full boardsize selected */

		gSymmetryMatrix = SafeMalloc(sizeof(MOVE) * NUMSYMMETRIES * maxsize);

		/* This is the array used for flipping along the N-S axis */
		gFlipNewPosition = SafeMalloc(sizeof(MOVE) * maxsize);
		*gFlipNewPosition = 2;
		*(++gFlipNewPosition) = 1;
		*(++gFlipNewPosition) = 0;
		*(++gFlipNewPosition) = 3;
		*(++gFlipNewPosition) = 6;
		*(++gFlipNewPosition) = 5;
		*(++gFlipNewPosition) = 4;
		*(++gFlipNewPosition) = 13;
		*(++gFlipNewPosition) = 12;
		*(++gFlipNewPosition) = 11;
		*(++gFlipNewPosition) = 10;
		*(++gFlipNewPosition) = 9;
		*(++gFlipNewPosition) = 8;
		*(++gFlipNewPosition) = 7;
		*(++gFlipNewPosition) = 16;
		*(++gFlipNewPosition) = 15;
		*(++gFlipNewPosition) = 14;
		*(++gFlipNewPosition) = 17;
		*(++gFlipNewPosition) = 20;
		*(++gFlipNewPosition) = 19;
		*(++gFlipNewPosition) = 18;

		/* This is the array used for rotating 90 degrees clockwise */
		gRotate90CWNewPosition = SafeMalloc(sizeof(MOVE) * maxsize);
		*gRotate90CWNewPosition = 14;
		*(++gRotate90CWNewPosition) = 7;
		*(++gRotate90CWNewPosition) = 4;
		*(++gRotate90CWNewPosition) = 8;
		*(++gRotate90CWNewPosition) = 15;
		*(++gRotate90CWNewPosition) = 9;
		*(++gRotate90CWNewPosition) = 0;
		*(++gRotate90CWNewPosition) = 19;
		*(++gRotate90CWNewPosition) = 17;
		*(++gRotate90CWNewPosition) = 15;
		*(++gRotate90CWNewPosition) = 10;
		*(++gRotate90CWNewPosition) = 5;
		*(++gRotate90CWNewPosition) = 3;
		*(++gRotate90CWNewPosition) = 1;
		*(++gRotate90CWNewPosition) = 20;
		*(++gRotate90CWNewPosition) = 11;
		*(++gRotate90CWNewPosition) = 2;
		*(++gRotate90CWNewPosition) = 12;
		*(++gRotate90CWNewPosition) = 16;
		*(++gRotate90CWNewPosition) = 13;
		*(++gRotate90CWNewPosition) = 6;

		gCanonicalPosition = GetCanonicalPosition;

		int i, j, temp; /* temp is used for debugging */

		/* the following loop seg faults on first iteration for boardsize
		        choice of 21, someone fix*/
		/* Initialize gSymmetryMatrix[][] */
		for(i = 0; i < maxsize; i++) {
			temp = i;
			for(j = 0; j < NUMSYMMETRIES; j++) {
				if(j == NUMSYMMETRIES/2)
					temp = gFlipNewPosition[i];
				if(j < NUMSYMMETRIES/2)
					temp = gSymmetryMatrix[j][i] = gRotate90CWNewPosition[temp];
				else
					temp = gSymmetryMatrix[j][i] = gRotate90CWNewPosition[temp];
			}
		}
	}
	/**************************************************/
	/**************** SYMMETRY FUN END ****************/
	/**************************************************/


	adjacent = (adjacency *) SafeMalloc(sizeof(adjacency) * maxsize);

	checked = (BOOLEAN *)SafeMalloc(sizeof(BOOLEAN)*maxsize);
	zeroChecked();

	switch(boardsize) {
	case 0: adjacent[0].numAdjacent = adjacent[1].numAdjacent = adjacent[3].numAdjacent = adjacent[4].numAdjacent = 3;
		adjacent[2].numAdjacent = 4;
		adjacent[0].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[0].numAdjacent);
		adjacent[0].adj[0] = 1;
		adjacent[0].adj[1] = 2;
		adjacent[0].adj[2] = 3;
		adjacent[1].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[1].numAdjacent);
		adjacent[1].adj[0] = 0;
		adjacent[1].adj[1] = 2;
		adjacent[1].adj[2] = 4;
		adjacent[3].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[3].numAdjacent);
		adjacent[3].adj[0] = 0;
		adjacent[3].adj[1] = 2;
		adjacent[3].adj[2] = 4;
		adjacent[4].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[4].numAdjacent);
		adjacent[4].adj[0] = 1;
		adjacent[4].adj[1] = 2;
		adjacent[4].adj[2] = 3;
		adjacent[2].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[0].numAdjacent);
		adjacent[2].adj[0] = 0;
		adjacent[2].adj[1] = 1;
		adjacent[2].adj[2] = 3;
		adjacent[2].adj[3] = 4;
		break;
	case 1: adjacent[1].numAdjacent = adjacent[8].numAdjacent = 2;
		adjacent[0].numAdjacent = adjacent[2].numAdjacent = adjacent[6].numAdjacent = adjacent[7].numAdjacent = 3;
		adjacent[3].numAdjacent = adjacent[4].numAdjacent = adjacent[5].numAdjacent = 4;
		adjacent[1].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[1].numAdjacent);
		adjacent[1].adj[0] = 5;
		adjacent[1].adj[1] = 6;
		adjacent[8].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[8].numAdjacent);
		adjacent[8].adj[0] = 5;
		adjacent[8].adj[1] = 6;
		adjacent[0].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[0].numAdjacent);
		adjacent[0].adj[0] = 2;
		adjacent[0].adj[1] = 3;
		adjacent[0].adj[2] = 4;
		adjacent[2].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[2].numAdjacent);
		adjacent[2].adj[0] = 0;
		adjacent[2].adj[1] = 3;
		adjacent[2].adj[2] = 7;
		adjacent[6].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[6].numAdjacent);
		adjacent[6].adj[0] = 1;
		adjacent[6].adj[1] = 5;
		adjacent[6].adj[2] = 8;
		adjacent[7].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[7].numAdjacent);
		adjacent[7].adj[0] = 2;
		adjacent[7].adj[1] = 3;
		adjacent[7].adj[2] = 4;
		adjacent[3].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[3].numAdjacent);
		adjacent[3].adj[0] = 0;
		adjacent[3].adj[1] = 2;
		adjacent[3].adj[2] = 4;
		adjacent[3].adj[3] = 7;
		adjacent[4].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[4].numAdjacent);
		adjacent[4].adj[0] = 0;
		adjacent[4].adj[1] = 3;
		adjacent[4].adj[2] = 5;
		adjacent[4].adj[3] = 7;
		adjacent[5].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[5].numAdjacent);
		adjacent[5].adj[0] = 1;
		adjacent[5].adj[1] = 4;
		adjacent[5].adj[2] = 6;
		adjacent[5].adj[3] = 8;
		break;
	case 2: adjacent[0].numAdjacent = adjacent[2].numAdjacent = adjacent[10].numAdjacent = adjacent[12].numAdjacent = 2;
		adjacent[1].numAdjacent = adjacent[3].numAdjacent = adjacent[9].numAdjacent = adjacent[11].numAdjacent = 3;
		adjacent[4].numAdjacent = adjacent[5].numAdjacent = adjacent[6].numAdjacent = adjacent[7].numAdjacent = adjacent[8].numAdjacent = 4;
		adjacent[0].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[0].numAdjacent);
		adjacent[0].adj[0] = 3;
		adjacent[0].adj[1] = 4;
		adjacent[2].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[2].numAdjacent);
		adjacent[2].adj[0] = 8;
		adjacent[2].adj[1] = 9;
		adjacent[10].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[10].numAdjacent);
		adjacent[10].adj[0] = 3;
		adjacent[10].adj[1] = 4;
		adjacent[12].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[12].numAdjacent);
		adjacent[12].adj[0] = 8;
		adjacent[12].adj[1] = 9;
		adjacent[1].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[1].numAdjacent);
		adjacent[1].adj[0] = 5;
		adjacent[1].adj[1] = 6;
		adjacent[1].adj[2] = 7;
		adjacent[3].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[3].numAdjacent);
		adjacent[3].adj[0] = 0;
		adjacent[3].adj[1] = 4;
		adjacent[3].adj[2] = 10;
		adjacent[9].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[9].numAdjacent);
		adjacent[9].adj[0] = 2;
		adjacent[9].adj[1] = 8;
		adjacent[9].adj[2] = 12;
		adjacent[11].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[11].numAdjacent);
		adjacent[11].adj[0] = 5;
		adjacent[11].adj[1] = 6;
		adjacent[11].adj[2] = 7;
		adjacent[4].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[4].numAdjacent);
		adjacent[4].adj[0] = 0;
		adjacent[4].adj[1] = 3;
		adjacent[4].adj[2] = 5;
		adjacent[4].adj[3] = 10;
		adjacent[5].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[5].numAdjacent);
		adjacent[5].adj[0] = 1;
		adjacent[5].adj[1] = 4;
		adjacent[5].adj[2] = 6;
		adjacent[5].adj[3] = 11;
		adjacent[6].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[6].numAdjacent);
		adjacent[6].adj[0] = 1;
		adjacent[6].adj[1] = 5;
		adjacent[6].adj[2] = 7;
		adjacent[6].adj[3] = 11;
		adjacent[7].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[7].numAdjacent);
		adjacent[7].adj[0] = 1;
		adjacent[7].adj[1] = 6;
		adjacent[7].adj[2] = 8;
		adjacent[7].adj[3] = 11;
		adjacent[8].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[8].numAdjacent);
		adjacent[8].adj[0] = 2;
		adjacent[8].adj[1] = 7;
		adjacent[8].adj[2] = 9;
		adjacent[8].adj[3] = 12;
		break;
	case 3: adjacent[14].numAdjacent = adjacent[16].numAdjacent = 2;
		adjacent[0].numAdjacent = adjacent[1].numAdjacent = adjacent[2].numAdjacent = adjacent[4].numAdjacent = adjacent[6].numAdjacent = adjacent[7].numAdjacent = adjacent[13].numAdjacent = adjacent[15].numAdjacent = 3;
		adjacent[3].numAdjacent = adjacent[5].numAdjacent = adjacent[8].numAdjacent = adjacent[9].numAdjacent = adjacent[10].numAdjacent = adjacent[11].numAdjacent = adjacent[12].numAdjacent = 4;
		adjacent[14].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[14].numAdjacent);
		adjacent[14].adj[0] = 7;
		adjacent[14].adj[1] = 8;
		adjacent[16].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[16].numAdjacent);
		adjacent[16].adj[0] = 12;
		adjacent[16].adj[1] = 13;
		adjacent[0].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[0].numAdjacent);
		adjacent[0].adj[0] = 1;
		adjacent[0].adj[1] = 3;
		adjacent[0].adj[2] = 4;
		adjacent[1].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[1].numAdjacent);
		adjacent[1].adj[0] = 0;
		adjacent[1].adj[1] = 2;
		adjacent[1].adj[2] = 3;
		adjacent[2].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[2].numAdjacent);
		adjacent[2].adj[0] = 1;
		adjacent[2].adj[1] = 3;
		adjacent[2].adj[2] = 6;
		adjacent[4].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[4].numAdjacent);
		adjacent[4].adj[0] = 0;
		adjacent[4].adj[1] = 7;
		adjacent[4].adj[2] = 8;
		adjacent[6].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[6].numAdjacent);
		adjacent[6].adj[0] = 2;
		adjacent[6].adj[1] = 12;
		adjacent[6].adj[2] = 13;
		adjacent[7].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[7].numAdjacent);
		adjacent[7].adj[0] = 4;
		adjacent[7].adj[1] = 8;
		adjacent[7].adj[2] = 14;
		adjacent[13].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[13].numAdjacent);
		adjacent[13].adj[0] = 6;
		adjacent[13].adj[1] = 12;
		adjacent[13].adj[2] = 16;
		adjacent[15].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[15].numAdjacent);
		adjacent[15].adj[0] = 9;
		adjacent[15].adj[1] = 10;
		adjacent[15].adj[2] = 11;
		adjacent[3].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[3].numAdjacent);
		adjacent[3].adj[0] = 0;
		adjacent[3].adj[1] = 1;
		adjacent[3].adj[2] = 2;
		adjacent[3].adj[3] = 5;
		adjacent[5].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[5].numAdjacent);
		adjacent[5].adj[0] = 3;
		adjacent[5].adj[1] = 9;
		adjacent[5].adj[2] = 10;
		adjacent[5].adj[3] = 11;
		adjacent[8].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[8].numAdjacent);
		adjacent[8].adj[0] = 4;
		adjacent[8].adj[1] = 7;
		adjacent[8].adj[2] = 9;
		adjacent[8].adj[3] = 14;
		adjacent[9].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[9].numAdjacent);
		adjacent[9].adj[0] = 5;
		adjacent[9].adj[1] = 8;
		adjacent[9].adj[2] = 10;
		adjacent[9].adj[3] = 15;
		adjacent[10].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[10].numAdjacent);
		adjacent[10].adj[0] = 5;
		adjacent[10].adj[1] = 9;
		adjacent[10].adj[2] = 11;
		adjacent[10].adj[3] = 15;
		adjacent[11].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[11].numAdjacent);
		adjacent[11].adj[0] = 5;
		adjacent[11].adj[1] = 10;
		adjacent[11].adj[2] = 12;
		adjacent[11].adj[3] = 15;
		adjacent[12].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[12].numAdjacent);
		adjacent[12].adj[0] = 6;
		adjacent[12].adj[1] = 11;
		adjacent[12].adj[2] = 13;
		adjacent[12].adj[3] = 16;
		break;
	case 4: adjacent[0].numAdjacent = adjacent[1].numAdjacent = adjacent[2].numAdjacent = \
		                                                            adjacent[4].numAdjacent = adjacent[6].numAdjacent = adjacent[7].numAdjacent = \
		                                                                                                                        adjacent[13].numAdjacent = adjacent[14].numAdjacent = adjacent[16].numAdjacent = \
		                                                                                                                                                                                      adjacent[18].numAdjacent = adjacent[19].numAdjacent = adjacent[20].numAdjacent = 3;
		adjacent[3].numAdjacent = adjacent[5].numAdjacent = adjacent[8].numAdjacent = \
		                                                            adjacent[9].numAdjacent = adjacent[10].numAdjacent = adjacent[11].numAdjacent = \
		                                                                                                                         adjacent[12].numAdjacent = adjacent[15].numAdjacent = adjacent[17].numAdjacent = 4;
		adjacent[0].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[0].numAdjacent);
		adjacent[0].adj[0] = 1;
		adjacent[0].adj[1] = 3;
		adjacent[0].adj[2] = 4;
		adjacent[1].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[1].numAdjacent);
		adjacent[1].adj[0] = 0;
		adjacent[1].adj[1] = 2;
		adjacent[1].adj[2] = 3;
		adjacent[2].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[2].numAdjacent);
		adjacent[2].adj[0] = 1;
		adjacent[2].adj[1] = 3;
		adjacent[2].adj[2] = 6;
		adjacent[4].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[4].numAdjacent);
		adjacent[4].adj[0] = 0;
		adjacent[4].adj[1] = 7;
		adjacent[4].adj[2] = 8;
		adjacent[6].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[6].numAdjacent);
		adjacent[6].adj[0] = 2;
		adjacent[6].adj[1] = 12;
		adjacent[6].adj[2] = 13;
		adjacent[7].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[7].numAdjacent);
		adjacent[7].adj[0] = 4;
		adjacent[7].adj[1] = 8;
		adjacent[7].adj[2] = 14;
		adjacent[13].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[13].numAdjacent);
		adjacent[13].adj[0] = 6;
		adjacent[13].adj[1] = 12;
		adjacent[13].adj[2] = 16;
		adjacent[14].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[14].numAdjacent);
		adjacent[14].adj[0] = 7;
		adjacent[14].adj[1] = 8;
		adjacent[14].adj[2] = 18;
		adjacent[16].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[16].numAdjacent);
		adjacent[16].adj[0] = 12;
		adjacent[16].adj[1] = 13;
		adjacent[16].adj[2] = 20;
		adjacent[18].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[18].numAdjacent);
		adjacent[18].adj[0] = 14;
		adjacent[18].adj[1] = 17;
		adjacent[18].adj[2] = 19;
		adjacent[19].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[19].numAdjacent);
		adjacent[19].adj[0] = 17;
		adjacent[19].adj[1] = 18;
		adjacent[19].adj[2] = 20;
		adjacent[20].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[20].numAdjacent);
		adjacent[20].adj[0] = 16;
		adjacent[20].adj[1] = 17;
		adjacent[20].adj[2] = 19;
		adjacent[3].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[3].numAdjacent);
		adjacent[3].adj[0] = 0;
		adjacent[3].adj[1] = 1;
		adjacent[3].adj[2] = 2;
		adjacent[3].adj[3] = 5;
		adjacent[5].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[5].numAdjacent);
		adjacent[5].adj[0] = 3;
		adjacent[5].adj[1] = 9;
		adjacent[5].adj[2] = 10;
		adjacent[5].adj[3] = 11;
		adjacent[8].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[8].numAdjacent);
		adjacent[8].adj[0] = 4;
		adjacent[8].adj[1] = 7;
		adjacent[8].adj[2] = 9;
		adjacent[8].adj[3] = 14;
		adjacent[9].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[9].numAdjacent);
		adjacent[9].adj[0] = 5;
		adjacent[9].adj[1] = 8;
		adjacent[9].adj[2] = 10;
		adjacent[9].adj[3] = 15;
		adjacent[10].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[10].numAdjacent);
		adjacent[10].adj[0] = 5;
		adjacent[10].adj[1] = 9;
		adjacent[10].adj[2] = 11;
		adjacent[10].adj[3] = 15;
		adjacent[11].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[11].numAdjacent);
		adjacent[11].adj[0] = 5;
		adjacent[11].adj[1] = 10;
		adjacent[11].adj[2] = 12;
		adjacent[11].adj[3] = 15;
		adjacent[12].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[12].numAdjacent);
		adjacent[12].adj[0] = 6;
		adjacent[12].adj[1] = 11;
		adjacent[12].adj[2] = 13;
		adjacent[12].adj[3] = 16;
		adjacent[15].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[15].numAdjacent);
		adjacent[15].adj[0] = 9;
		adjacent[15].adj[1] = 10;
		adjacent[15].adj[2] = 11;
		adjacent[15].adj[3] = 15;
		adjacent[17].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[17].numAdjacent);
		adjacent[17].adj[0] = 15;
		adjacent[17].adj[1] = 18;
		adjacent[17].adj[2] = 19;
		adjacent[17].adj[3] = 20;
		break; /*NOTE: need to free all this she-bop */
	default:
		printf("We should never get here inside InitializeGame while creating the adjacency table information\n");
		break;
	}

	gMoveToStringFunPtr = &MoveToString;
}

char * emptyboard(board)
char *board;
{
	int i;

	board = (char *) SafeMalloc(sizeof(char) * maxsize);
	for(i=0; i<maxsize; i++) board[i]=EMPTYSPACE;

	return board;
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
	MOVELIST *moves = NULL; /* has fields move and next */
	int i;
	char piece;
	int player;

	player=getplayer(position);

	if(player==1) {
		piece=PLAYER1;
	} else if (player==2) {
		piece=PLAYER2;
	} else {
		fprintf(stderr,"Bad else: GenerateMovse (Piece selection)\n");
		exit(1);
	}

	board=emptyboard(board);
	/* Use CreateMovelistNode(move, next) to 'cons' together a linked list */
	/* wow, talk about making something foolproof for having students coding games */
	board=unhashboard(position,board);
	for(i=0; i<maxsize; i++) {
		if(isValidMove(board, (MOVE) i, piece)) {
			moves = CreateMovelistNode((MOVE)i,moves);
		}
	}

	return moves;
}

/* Generate Moves helper function isValidMove */
BOOLEAN isValidMove(char *bd, MOVE mv, char p) {
	if(board[mv] != EMPTYSPACE)
		return FALSE;

	int i, scount=0, ocount=0;
	char oPiece = ((p == PLAYER1) ? PLAYER2 : PLAYER1);

	for(i = 0; i < adjacent[mv].numAdjacent; i++)
		if(board[adjacent[mv].adj[i]] == p)
			scount++;
		else if(board[adjacent[mv].adj[i]] == oPiece)
			ocount++;

	if(scount == adjacent[mv].numAdjacent) {
		board[mv] = p;

		if(isSurrounded(board, mv, p, checked)) {
			board[mv] = EMPTYSPACE;
			zeroChecked();
			return FALSE;
		}

		board[mv] = EMPTYSPACE;
		zeroChecked();
	}
	else if(ocount == adjacent[mv].numAdjacent) {
		board[mv] = p;
		BOOLEAN canCapture = FALSE;

		for(i = 0; i < adjacent[mv].numAdjacent && !canCapture; i++) {
			canCapture = (canCapture || isSurrounded(board, adjacent[mv].adj[i], oPiece, checked));
			zeroChecked();
		}

		board[mv] = EMPTYSPACE;

		return canCapture;
	}

	return TRUE;
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
	char piece, oPiece;
	int player, turn, i;

	board=unhashboard(position,board);
	player=getplayer(position);
	turn=getturnnumber(position);
	if(player==1) {
		piece=PLAYER1;
		oPiece = PLAYER2;
	} else if (player==2) {
		piece=PLAYER2;
		oPiece = PLAYER1;
		turn++; /* sneak incrementing the turn, because next it will be player 1's turn */
	} else {
		fprintf(stderr,"Bad else: Do Move (Piece selection)\n");
		exit(1);
	}

	board[move]=piece;


	/* check for capture */

	for(i = 0; i < adjacent[move].numAdjacent; i++) {
		if(board[adjacent[move].adj[i]] == oPiece && !checked[adjacent[move].adj[i]]) {
			checked[adjacent[move].adj[i]] = TRUE;
			if(isSurrounded(board, adjacent[move].adj[i], oPiece, checked)) {
				zeroChecked();
				removeStones(board, adjacent[move].adj[i], oPiece, checked);
			}
			zeroChecked();
		}
	}

	player^=3; /* slickness, 1 becomes 2 becomes 1.... */
	position = hash(board,player,turn);

	return position;
}

void zeroChecked() {
	int i;
	for(i = 0; i < maxsize; i++)
		checked[i] = FALSE;
}

BOOLEAN isSurrounded(board, move, p, check)
char *board;
MOVE move;
char p;
BOOLEAN *check;
{
	if(board[move] != p)
		return (board[move] == EMPTYSPACE) ? FALSE : TRUE;
	else {
		int i;
		BOOLEAN ret = TRUE;

		for(i = 0; i < adjacent[move].numAdjacent && ret; i++)
			if(!check[adjacent[move].adj[i]]) {
				check[adjacent[move].adj[i]] = TRUE;
				ret = (ret && isSurrounded(board, adjacent[move].adj[i], p, check));
			}

		return ret;
	}
}

void removeStones(board, move, p, check)
char *board;
MOVE move;
char p;
BOOLEAN *check;
{
	int i;
	if(board[move] == p) {
		check[move] = TRUE;
		board[move] = EMPTYSPACE;


		for(i = 0; i < adjacent[move].numAdjacent; i++)
			if(!check[adjacent[move].adj[i]])
				removeStones(board, adjacent[move].adj[i], p, check);
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
************************************************************************/

VALUE Primitive (position)
POSITION position;
{
	int player,turn;
	int p1c, p2c;
	int i;
	VALUE ret;
	char piece;
	BOOLEAN noValidMoves = TRUE;

	turn=getturnnumber(position);
	player=getplayer(position);
	board=unhashboard(position,board);

	if(player == 1)
		piece = PLAYER1;
	else if(player == 2)
		piece = PLAYER2;
	else {
		fprintf(stderr,"Bad else: GenerateMovse (Piece selection)\n");
		exit(1);
	}

	for(i = 0; i < maxsize && noValidMoves; i++)
		if(isValidMove(board, (MOVE) i, piece))
			noValidMoves = FALSE;

	/* if we've ran out of pieces or there are no more valid spaces on the board */
	if(turn==numpieces || noValidMoves) {
		p1c=countboard(board,PLAYER1);
		p2c=countboard(board,PLAYER2);

		for(i = 0; i < maxsize; i++) {
			if(board[i] == EMPTYSPACE) {
				checked[i] = TRUE;

				if(isTerritory(board, (MOVE) i, PLAYER1, checked))
					p1c++;
				else {
					zeroChecked();
					if(isTerritory(board, (MOVE) i, PLAYER2, checked))
						p2c++;
					zeroChecked();
				}
			}

			zeroChecked();
		}

		/* if its player 2's turn and he's ahead, this spots a win */
		if(p1c<p2c && player==2) {
			ret=win;
			/* if its player 1's turn and he's ahead, this spots a win */
		} else if(p1c>p2c && player==1) {
			ret=win;
			/* if they are equal, this spots a tie */
		} else if(p1c==p2c) {
			ret=tie;
			/* changed this so that it would solve for now */
			// ret=win;
			/* otherwise it must be a lose */
		} else {
			ret=lose;
		}
	} else {
		ret=undecided;
	}

	return ret;
}

/* helper function to count empty indices surrounded by one player as territory for that player */
BOOLEAN isTerritory(board, move, p, check)
char *board;
MOVE move;
char p;
BOOLEAN *check;
{
	if(board[move] != EMPTYSPACE)
		return (board[move] == p);
	else {
		int i;
		BOOLEAN ret = TRUE;

		for(i = 0; i < adjacent[move].numAdjacent && ret; i++)
			if(!check[adjacent[move].adj[i]]) {
				check[adjacent[move].adj[i]] = TRUE;
				ret = (ret && isTerritory(board, adjacent[move].adj[i], p, check));
			}

		return ret;
	}
}

/************************************************************************
** displayasciiboard() -- displays the board in a nice to view manner
**
** Takes in a char array that represents each of the positions and a character
** array with the prediction.
**
** IMPORTANT: Prediction needs to be 15 characters, 15 is the number of characters
** for 16 is too much and 14 is too few. 15 is the number of characters needed
** for the output to look proper  -JK
**
** Returns nothing.
**
** Still need to add the prediction into the drawing. -JK
**
*************************************************************************/

void display5board(pos, prediction)
char *pos;
char *prediction;
{
	printf("Legend:    1         Current:      %c\n",pos[0]);
	printf("          /|\\        Player1: X   /|\\\n");
	printf("         2-3-4       Player2: *  %c-%c-%c\n",pos[1],pos[2],pos[3]);
	printf("          \\|/                     \\|/\n");
	printf("           5                       %c\n",pos[4]);
	if(prediction) printf(" Prediction: %s\n\n",prediction);
}
void display9board(pos, prediction)
char *pos;
char *prediction;
{
	printf("Legend:   1      2        Current:      %c      %c\n",pos[0],pos[1]);
	printf("         /|\\    /|        Player1: X   /|\\    /|\n");
	printf("        3-4-5--6-7        Player2: *  %c-%c-%c--%c-%c\n",pos[2],pos[3],pos[4],pos[5],pos[6]);
	printf("         \\|/    \\|                     \\|/    \\|\n");
	printf("          8      9                      %c      %c\n",pos[7],pos[8]);
	if(prediction) printf(" Prediction: %s\n\n",prediction);
}
void display13board(pos, prediction)
char *pos;
char *prediction;
{
	printf("Legend:   1      2      3   Current:    %c      %c      %c\n",pos[0],pos[1],pos[2]);
	printf("          |\\    /|\\    /|   Player1: X  |\\    /|\\    /|\n");
	printf("          4-5--6-7-8--9-B   Player2: *  %c-%c--%c-%c-%c--%c-%c\n",pos[3],pos[4],pos[5],pos[6],pos[7],pos[8],pos[9]);
	printf("          |/    \\|/    \\|               |/    \\|/    \\|\n");
	printf("          D      E      F               %c      %c      %c\n",pos[10],pos[11],pos[12]);
	if(prediction) printf(" Prediction: %s\n\n",prediction);
}

void display17board(pos, prediction)
char *pos;
char *prediction;
{
	printf("Legend:    /1-2-3\\     Current:        /%c-%c-%c\\\n",pos[0],pos[1],pos[2]);
	printf("          /  \\|/  \\    Player1: X     /  \\|/  \\\n");
	printf("         /    4    \\   Player2: *    /    %c    \\\n",pos[3]);
	printf("        /     |     \\               /     |     \\\n");
	printf("       5      6      7             %c      %c      %c\n",pos[4],pos[5],pos[6]);
	printf("       |\\    /|\\    /|             |\\    /|\\    /|\n");
	printf("       8-9--B-D-E--F-G             %c-%c--%c-%c-%c--%c-%c\n",pos[7],pos[8],pos[9],pos[10],pos[11],pos[12],pos[13]);
	printf("       |/    \\|/    \\|             |/    \\|/    \\|\n");
	printf("       I      J      K             %c      %c      %c\n",pos[14],pos[15],pos[16]);
	if(prediction) printf(" Prediction: %s\n\n",prediction);
}

void display21board(pos, prediction)
char *pos;
char *prediction;
{
	printf("Legend:    /1-2-3\\     Current:        /%c-%c-%c\\\n",pos[0],pos[1],pos[2]);
	printf("          /  \\|/  \\    Player1: X     /  \\|/  \\\n");
	printf("         /    4    \\   Player2: *    /    %c    \\\n",pos[3]);
	printf("        /     |     \\               /     |     \\\n");
	printf("       5      6      7             %c      %c      %c\n",pos[4],pos[5],pos[6]);
	printf("       |\\    /|\\    /|             |\\    /|\\    /|\n");
	printf("       8-9--B-D-E--F-G             %c-%c--%c-%c-%c--%c-%c\n",pos[7],pos[8],pos[9],pos[10],pos[11],pos[12],pos[13]);
	printf("       |/    \\|/    \\|             |/    \\|/    \\|\n");
	printf("       I      J      K             %c      %c      %c\n",pos[14],pos[15],pos[16]);
	printf("        \\     |     /               \\     |     /\n");
	printf("         \\    L    /                 \\    %c    /\n",pos[17]);
	printf("          \\  /|\\  /                   \\  /|\\  /\n");
	printf("           \\M-N-P/        \\%c-%c-%c/\n",pos[18],pos[19],pos[20]);
	if(prediction) printf(" Prediction: %s\n\n",prediction);

}

char *getprediction(pred)
char *pred;
{
	int i=0;
	/* code to initialize it to blank values until a later time */
	for(; i<16; i++) pred[i]=(char)32; /* fill with spaces */
	pred[i]='\0';
	return pred;
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

void PrintPosition (position, playersName, usersTurn)
POSITION position;
STRING playersName;
BOOLEAN usersTurn;
{
	/* just a doodle of the gameboard
	 * - Josh
	 *              /0-0-0\
	 *             /..\|/..\
	 *	      /....0....\
	 *           /.....|.....\
	 *          0......0......0
	 *          |\..../|\..../|
	 *          0-0--0-0-0--0-0
	 *          |/....\|/....\|
	 *          0......0......0
	 *           \.....|...../
	 *            \....0..../
	 *             \../|\../
	 *              \0-0-0/
	 *
	 * all thanks to boring lectures =)
	 */
	char *toprint, *prediction;
	toprint=(char *)SafeMalloc(sizeof(char)*(5+4*boardsize));
	/*prediction=(char *)SafeMalloc(sizeof(char)*17); */

	/* need to get prediction, till then... */
	prediction=GetPrediction(position,playersName,usersTurn);
	if(strcmp(prediction," ")==0) prediction=NULL;
	toprint = unhashboard(position,toprint);
	switch(boardsize) {
	case 0:
		display5board(toprint,prediction);
		break;
	case 1:
		display9board(toprint,prediction);
		break;
	case 2:
		display13board(toprint,prediction);
		break;
	case 3:
		display17board(toprint,prediction);
		break;
	case 4:
		display21board(toprint,prediction);
		break;
	default:
		printf("Invalid boardsize in printPosition\n");
		exit(1);
		break;
	}
	SafeFree(toprint);
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

void PrintComputersMove (computersMove, computersName)
MOVE computersMove;
STRING computersName;
{
	char move=0;
	if(computersMove>-1 && computersMove < 9) {
		move=49+computersMove;
	} else {
		switch(computersMove) {
		case 9:
			move='B';
			break;
		case 10:
			move='D';
			break;
		case 11:
			move='E';
			break;
		case 12:
			move='F';
			break;
		case 13:
			move='G';
			break;
		case 14:
			move='I';
			break;
		case 15:
			move='J';
			break;
		case 16:
			move='K';
			break;
		case 17:
			move='L';
			break;
		case 18:
			move='M';
			break;
		case 19:
			move='N';
			break;
		case 20:
			move='P';
			break;
		}
	}
	printf("%s's move		: %c\n",computersName,move);
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


void PrintMove (move)
MOVE move;
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
	STRING m = (STRING) SafeMalloc( 2 );
	char movechar=getmovechar(theMove);

	sprintf( m, "%c",movechar);
	return m;
}

char getmovechar(move)
MOVE move;
{
	char movechar=0;
	if(move>-1 && move < 9) {
		movechar=49+move;
	} else {
		switch(move) {
		case 9:
			movechar='B';
			break;
		case 10:
			movechar='D';
			break;
		case 11:
			movechar='E';
			break;
		case 12:
			movechar='F';
			break;
		case 13:
			movechar='G';
			break;
		case 14:
			movechar='I';
			break;
		case 15:
			movechar='J';
			break;
		case 16:
			movechar='K';
			break;
		case 17:
			movechar='L';
			break;
		case 18:
			movechar='M';
			break;
		case 19:
			movechar='N';
			break;
		case 20:
			movechar='P';
			break;
		}
	}
	return movechar;
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

USERINPUT GetAndPrintPlayersMove (position, move, playersName)
POSITION position;
MOVE *move;
STRING playersName;
{
	USERINPUT input;
	USERINPUT HandleDefaultTextInput();

	MOVELIST *moves;
	MOVE thisMove;
	char *validmoves;
	int cur=0;

	validmoves=(char *)SafeMalloc(sizeof(char)*2*maxsize+1);

	moves=GenerateMoves(position);
	while(moves) {
		thisMove=moves->move;
		validmoves[cur]=getmovechar(thisMove);
		validmoves[cur+1]=',';
		cur=cur+2;
		moves=moves->next;
	}
	if(cur!=0) cur--;
	validmoves[cur]='\0';
	for (;; ) {
		/***********************************************************
		* CHANGE THE LINE BELOW TO MATCH YOUR MOVE FORMAT
		***********************************************************/
		printf("%8s's move [(undo)/(%s)] : ", playersName,validmoves);

		input = HandleDefaultTextInput(position, move, playersName);

		if (input != Continue)
			return input;
	}
	SafeFree(validmoves);
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

BOOLEAN ValidTextInput (input)
STRING input;
{
	switch(toupper(input[0])) {
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'B':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'P':
		return TRUE;
	}
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

MOVE ConvertTextInputToMove (input)
STRING input;
{
	switch(toupper(input[0])) {
	case 'B':
		return (MOVE)9;
	case 'D':
		return (MOVE)10;
	case 'E':
		return (MOVE)11;
	case 'F':
		return (MOVE)12;
	case 'G':
		return (MOVE)13;
	case 'I':
		return (MOVE)14;
	case 'J':
		return (MOVE)15;
	case 'K':
		return (MOVE)16;
	case 'L':
		return (MOVE)17;
	case 'M':
		return (MOVE)18;
	case 'N':
		return (MOVE)19;
	case 'P':
		return (MOVE)20;
	default:
		input[1]='\0';
		return (MOVE)atoi(input)-1;
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
	char choice;
	int boardsizechoice;

	while(1) {
		/*fflush(stdin);		fflush is undefined for input streams
		   choice=getc(stdin);  dummy call to get the carrage return in the buffer */
		printf("\n\t----- mxigua Option Menu -----\n\n");
		printf("\td)\tChange Boar(d) Size (Currently: %d)\n",maxsize);
		printf("\tn)\tChange (N)umber of Pieces (Currently: %d)\n",numpieces);
		printf("\n\n");
		printf("\tb)\t(B)ack to main menu\n\n\n");
		/* printf("\tr)\tChange (R)ule Type (Currently: )\n"); */
		/* printf("h - Handicapping (ON/OFF)\n"); */
		/* printf("w - Win Style (Territory/Captures/Both)\n"); */

		printf("Please enter a selection: ");
		/*choice=getc(stdin);*/
		choice = GetMyChar();
		choice=toupper(choice);
		switch(choice) {
		case 'D':
			/*fflush(stdin);
			   choice=getc(stdin);*/
			printf("Please enter a boardsize from the following [5, 9, 13, 17, 21]:");
			/*scanf("%d",&boardsizechoice);*/
			boardsizechoice = GetMyInt();
			if(boardsizechoice==5||boardsizechoice==9||boardsizechoice==13||boardsizechoice==17||boardsizechoice==21) {
				boardsize=(boardsizechoice-5)/4;
				// uncomment this to auto-reset numpieces
				//numpieces=-1;
				InitializeGame();
			} else {
				printf("Incorrect size.\n");
			}
			break;
		case 'N':
			/*fflush(stdin);
			   choice=getc(stdin);*/
			printf("Please enter the number of pieces you want to be in play [1-%d]",maxsize);
			/*scanf("%d",&boardsizechoice);*/
			boardsizechoice = GetMyInt();
			if(boardsizechoice>=1 && boardsizechoice <= maxsize) {
				numpieces=boardsizechoice;
				InitializeGame();
			} else {
				printf("Incorrect number of pieces.\n");
			}
			break;
		case 'R':
			printf("Not this time, sorry.\n");
			break;
		case 'H':
			printf("Not this time, sorry.\n");
			break;
		case 'W':
			printf("Not this time, sorry.\n");
			break;
		case 'B':
			return;
		default:
			printf("You did not select a valid option.\n");
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

void SetTclCGameSpecificOptions (options)
int options[];
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
	char *moves;
	char in;
	char *prediction="";
	int count=0,pnum,turn;
	switch(boardsize) {
	case 0:
		display5board(board,prediction);
		moves="1 2 3 4 5";
		break;
	case 1:
		display9board(board,prediction);
		moves="1 2 3 4 5 6 7 8 9";
		break;
	case 2:
		display13board(board,prediction);
		moves="1 2 3 4 5 6 7 8 9 B D E F";
		break;
	case 3:
		display17board(board,prediction);
		moves="1 2 3 4 5 6 7 8 9 B D E F G I J K";
		break;
	case 4:
		display21board(board,prediction);
		moves="1 2 3 4 5 6 7 8 9 B D E F G I J K L M N P";
		break;
	default:
		printf("Bad default in GetInitialPosition().");
		exit(1);
	}
	printf("Please enter the pieces [X,*,O]\nin each position [%s] with one space between each position.\n",moves);
	/*fflush(stdin);
	   getc(stdin);	 kill the remaining \n in the buffer
	   in=getc(stdin);*/
	in = GetMyChar();
	while(in!='\n') {
		if(count < maxsize) {
			switch(toupper(in)) {
			//case PLAYER1:
			case 'X':
				board[count]=PLAYER1;
				break;
			//case PLAYER2:
			case 'O':
				board[count]=PLAYER2;
				break;
			default:
				board[count]=EMPTYSPACE;
				break;
			}
			/*in=getc(stdin);*/
			in = GetMyChar();
		}
		/*if(in!='\n') in=getc(stdin);*/
		count++;
	}
	printf("Please enter the player whose turn it is [1,2]: ");
	/*scanf("%d",&pnum);*/
	pnum = GetMyInt();
	printf("Please enter the number of turns that have passed [0-%d]:",(numpieces));
	/*scanf("%d",&turn);*/
	turn = GetMyInt();
	if(pnum<1 || pnum>2) pnum=1;
	if(turn<0||turn>(numpieces)) turn=numpieces;

	return hash(board,pnum,turn);
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
	int curoptions=getOption();
	int numberOfOptions;
	/* set all the options at max */
	boardsize=4;
	rulesvariant=1;
	handicapping=1;
	towin=2;
	numberOfOptions=getOption();
	/* return the options to their previous values */
	setOption(curoptions);
	return numberOfOptions;
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
	/**
	** Options:
	** 1/0 Inverse/Normal Rules
	** 1/0 Handicapped/Not
	** 0-2 Counting Territory, captures, or both
	** 0-4 Boardsize
	**/
	return 30*rulesvariant+15*handicapping+5*towin+boardsize;
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

void setOption (option)
int option;
{
	rulesvariant=option/30;
	option-=(rulesvariant*30);
	handicapping=option/15;
	option-=(handicapping*15);
	towin=option/5;
	option-=(towin*5);
	boardsize=option;
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
	printf("DEBUG MENU\n\n\n");
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
** int hash(char *board,int player,int turn)
** takes in a board, player number, and turn number (starting from 0)
** returns a hash of that
**
** uses the generic_hash() to hash the board and position
** shifts that left 5 bits and encodes the turn number
**
**************************************************************************/
POSITION hash(board, player, turn)
char *board;
int player;
int turn;
{
	POSITION ret;

	/* do the generic hash */
	ret=generic_hash_hash(board,player);

	ret=ret<<NUM_HASH_WRAPPER_BITS;
	ret|=turn;

	return ret;
}

/*************************************************************************
**
** char *unhashboard(int hashed)
** returns a board from a hashed board
**
***************************************************************************/

char *unhashboard(hashed, board)
POSITION hashed;
char *board;
{
	hashed=hashed>>NUM_HASH_WRAPPER_BITS; /* get rid of the turn encoding */
	/* better have a non null board */
	return generic_hash_unhash(hashed, board);
}

/*************************************************************************
**
** int hash_init(int, int, int..)
** initializes generic hash function
**
***************************************************************************/

int hash_init(int boardsize, int pieces_array[], int (*vcfg_function_ptr)(int* cfg)) {
	return (generic_hash_init(boardsize, pieces_array, NULL, 0) << NUM_HASH_WRAPPER_BITS);    /* initialize the hash */
}

/***************************************************************************
**
** int getplayer(int hashed)
** returns a player number (whose turn it is currently)
** takes in a hashed board
**
*****************************************************************************/

int getplayer(hashed)
POSITION hashed;
{
	hashed = hashed>>NUM_HASH_WRAPPER_BITS;
	return generic_hash_turn(hashed);
}

/****************************************************************************
**
** int getturnnumber(int hashed)
** returns the turn number of the current player
**
*****************************************************************************/

int getturnnumber(hashed)
POSITION hashed;
{
	return hashed&31;
}

/*********
**
** int countboard(char *board, char tocount)
** returns the number of occurances of a character in a board
**
***********/

int countboard(board, tocount)
char *board;
char tocount;
{
	int counter=0,i=0;
	for(; i<maxsize; i++)
		if(board[i]==tocount)
			counter++;
	return counter;
}


/**************************************************/
/**************** SYMMETRY FUN BEGIN **************/
/**************************************************/

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
	POSITION DoSymmetry();
	POSITION newPosition, theCanonicalPosition;
	int i;

	theCanonicalPosition = position;

	for(i = 0; i < NUMSYMMETRIES; i++) {

		newPosition = DoSymmetry(position, i); /* get new */
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
** INPUTS:      POSITION position : The position to branch the symmetry from.
**              int      symmetry : The number of the symmetry operation.
**
** OUTPUTS:     POSITION, The position after the symmetry operation.
**
************************************************************************/

POSITION DoSymmetry(position, symmetry)
POSITION position;
int symmetry;
{
	int i;
	int player = getplayer(position);
	int turn = getturnnumber(position);

	board = unhashboard(position,board);
	symmboard = unhashboard(position,symmboard); /* Make copy */

	/* Copy from the symmetry matrix */

	for(i = 0; i < maxsize; i++)
		symmboard[i] = board[gSymmetryMatrix[symmetry][i]];

	return(hash(symmboard, player, turn));
}

/**************************************************/
/**************** SYMMETRY FUN END ****************/
/**************************************************/
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
