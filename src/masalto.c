/************************************************************************
**
** NAME:        Asalto
**
** DESCRIPTION: Fox and Geese
**
** AUTHOR:      Robert Liao
**              Michael Chen
**
** DATE:        24 Feb 2004 - Added Initial Code
**
** UPDATE HIST: 24 Feb 2004 - Initial Setup
**              31 Mar 2004 - More Stuff Added. Almost Done.
**              18 Apr 2004 - Revamped Generate Moves. Now shows up in Gamesman
**              23 May 2004 - Began Reduction of Board Size from 33 positions to 21
**              Dates After - Refer to CVS Logs
**
** CHECKLIST:
**            Fix Bugs (GoAgain)
**            Unflag GoAgain
**            PROOFREAD
**            DOUBLE CHECK DEFAULT POSITION!
**
** DONE       Reverse Board
**            Make User Lowercase
**            'D' DONE instead of Pass
**            Hash Go Again
**            Make more Friendly
**            Make EVERYTHING Pretty. Mimic Gamesman
**            Add Sample Game Help
**            Fix Menus. B) Categorically Go Back
**            Add Piece on PrintPosition
**            Redo Example Game
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

extern STRING gValueString[];

POSITION gNumberOfPositions  = 0; /* The number of total possible positions | If you are using our hash, this is given by the hash_init() function*/

POSITION gInitialPosition    = 812760; /* The initial position (starting board) */
POSITION gMinimalPosition    = 0; /* */
POSITION kBadPosition        = -1; /* A position that will never be used */

STRING kGameName           = "Asalto";   /* The name of your game */
STRING kDBName             = "Asalto";   /* The name to store the database under */
STRING kAuthorName          = "Robert Liao and Michael Chen";
BOOLEAN kPartizan           = TRUE;  /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN kDebugMenu          = FALSE;  /* TRUE while debugging */
BOOLEAN kGameSpecificMenu   = TRUE;  /* TRUE if there is a game specific menu*/
BOOLEAN kTieIsPossible      = FALSE;  /* TRUE if a tie is possible */
BOOLEAN kLoopy               = TRUE;  /* TRUE if the game tree will have cycles (a rearranger style game) */
BOOLEAN kDebugDetermineValue = FALSE;  /* TRUE while debugging */
void*    gGameSpecificTclInit = NULL;

STRING kHelpGraphicInterface = "init_game_help not run!";

STRING kHelpTextInterface  = "init_game_help not run!";

STRING kHelpOnYourTurn     = "init_game_help not run!";

STRING kHelpStandardObjective = "init_game_help not run!";

STRING kHelpReverseObjective  = "init_game_help not run!";

STRING kHelpTieOccursWhen =  "init_game_help not run!";
/* Should follow 'A Tie occurs when... */

STRING kHelpExample = "init_game_help not run!";

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

int BOARDSIZE = 21;    /* 5X5 Crosscross Board */
int GEESE_MAX = 3;
int GEESE_MIN = 3;
int GEESE_HASH_MIN = 2; /* MUST BE ONE LESS THAN GEESE_MIN */
int FOX_MAX = 2;
int FOX_MIN = 2;
int WHITESPACE = 0; /* There is a bug in the generic hash function dealing with whitespace */

#define GEESE_PLAYER 1
#define FOX_PLAYER 2


#define INIT_DEBUG 0
#define HASH_TEST 0

#define DOMOVE_DEBUG 0
#define DOMOVE_TEST 0

#define GAMESMAN_GOAGAIN_DEBUG 0

#define PRIMITIVE_DEBUG 0

#define GENERATEMOVES_DEBUG 0

#define GETANDPRINT_DEBUG 0

#define VALIDTEXT_DEBUG 0

#define PRINTPOSITION_DEBUG 0

#define FOXGENERATEMOVES_DEBUG 0

#define GEESEGENERATEMOVES_DEBUG 0

#define GENERATESHIFTMOVES_DEBUG 0

#define GENERATEKILLMOVES_DEBUG 0

#define CONVERTEXTINPUTTOMOVE_DEBUG 0

#define GETOPTION_DEBUG 0

#define SETOPTION_DEBUG 0

char start_standard_board[]={                'G','G','G',
	                                     ' ',' ',' ',' ',' ',
	                                     ' ',' ',' ',' ',' ',
	                                     ' ',' ',' ',' ',' ',
	                                     'F',' ','F'};


/*************************************************************************
**
** Below is where you put your #define's and your global variables, structs
**
*************************************************************************/

int freeGeeseLocations[24];

int variant_goAgain = 1;
int variant_diagonals = 1;
int variant_geeseMoveBackwards = 0;

int NUM_GOAGAIN_BITS = -1;

/*************************************************************************
**
** Above is where you put your #define's and your global variables, structs
**
*************************************************************************/

/*
** Function Prototypes:
*/

void init_board_hash();
void init_game_help();

void PrintBoard(char board[]);

int diagonalConnect(int location);

BOOLEAN GoAgain (POSITION, MOVE);

MOVELIST *FoxGenerateMoves(const char board[BOARDSIZE], MOVELIST *moves);
MOVELIST *GeeseGenerateMoves(const char board[BOARDSIZE], MOVELIST *moves, int location);
MOVELIST *GenerateShiftMoves(const char board[BOARDSIZE], MOVELIST *moves, int location, int player);
MOVELIST *GenerateKillMoves(const char board[BOARDSIZE], MOVELIST *moves, int location, int player);
int MoreKillMoves(const char board[BOARDSIZE], int location, int player);
int validMove(const char board[BOARDSIZE], int move[2],int player);
int CantMove(const char board[BOARDSIZE], int player);

int hashMove(int move[2]);
void unHashMove(int hashed_move, int move[2]);

void boardPieceStats(char board[BOARDSIZE], int stats[3]);
int numFoxes(int stats[3]);
int numGeese(int stats[3]);
int numWinGeese(int stats[3]);

int coordToLocation(int coordinates[2]);
void locationToCoord(int location, int coordinates[2]);
int validCoord(int coord[2]);

void UserInput(char input[]);
void UserInputCoordinate(int coordinate[2]);

void AddRemoveFoxes(char *boardPointer);
void AddRemoveGeese(char *boardPointer);

void coordToGridCoordinate(int coordinate[2], char human[2]);
void gridCoordinatetoCoord(char human[2], int coord[2]);

int mergePositionGoAgain(int position, int goAgainPos);
void mergePositionSetRequiredBits(int numFoxes);
int getPosition(int mergedPos);
int getGoAgainFoxNum(int mergedPos);
int GoAgainPos(int mergedPos);
int intpow(int base, int exp);

int getFoxNumber(const char board[BOARDSIZE], int loc);
int getFoxPos(const char board[BOARDSIZE], int foxnum);

void PrintSpaces(int spaces);

STRING MoveToString( MOVE );


/* External */
#ifndef MEMWATCH
extern GENERIC_PTR      SafeMalloc ();
extern void             SafeFree ();
#endif

/************************************************************************
**
** NAME:        InitializeGame
**
** DESCRIPTION: Initialize the gDatabase, a global variable. and the other
**              local variables.
**
************************************************************************/

void InitializeGame ()
{
	if (INIT_DEBUG) { printf("mASALTO - InitializeGame() Running...\n"); }
	/* Initialize Hash Function */

	init_board_hash();

	/* GoAgain assignment moved here. -JJ */
	gGoAgain = GoAgain;

	gMoveToStringFunPtr = &MoveToString;

	if (INIT_DEBUG) { printf("mASALTO - InitializeGame() Done\n"); }
}

void init_board_hash()
{
	int hash_data[] =  {' ', WHITESPACE, BOARDSIZE,
		            'F', FOX_MIN, FOX_MAX,
		            'G', GEESE_HASH_MIN, GEESE_MAX, -1};
	int max;
	int init;
	int boardStats[3];
	boardPieceStats(start_standard_board,boardStats);
	hash_data[5] = numFoxes(boardStats);
	hash_data[8] = numGeese(boardStats);
	hash_data[7] = GEESE_MIN - 1;

	if (INIT_DEBUG) { printf("mASALTO - init_board_hash() --> generic_hash_init\n"); }
	max = generic_hash_init(BOARDSIZE, hash_data, NULL, 0);
	if (INIT_DEBUG) { printf("mASALTO - init_board_hash() <-- generic_hash_init: %d\n",max); }

	if (INIT_DEBUG) { printf("mASALTO - init_board_hash() --> generic_hash\n"); }

	mergePositionSetRequiredBits(numFoxes(boardStats));

	init = mergePositionGoAgain(generic_hash_hash(start_standard_board,GEESE_PLAYER), 0);

	if (INIT_DEBUG) { printf("mASALTO - init_board_hash() <-- generic_hash: %d\n",init); }

	gInitialPosition = init;
	gNumberOfPositions = max * intpow(2,NUM_GOAGAIN_BITS);

	if (HASH_TEST)
	{
		char test_board[BOARDSIZE];
		printf("INIT CURRENT BOARD\n"); PrintBoard(start_standard_board);
		printf("Hash Test. Unhashed Board. Hash Value %d\n", init);
		generic_hash_unhash(getPosition(init), test_board);
		PrintBoard(test_board);
	}
	init_game_help();
}

void init_game_help()
{
	if (variant_goAgain)
	{
		kHelpTextInterface    =
		        "Your move consists of the location of the piece you want to move\n\
and the location of where you want to move the piece to.\n\
\nFor example, the move\n\
\t[c4 c5]  (brackets are not necessary)\n\
will move the piece located at c4 and move it to c5.\n\
\n\
Note: You can only move your own pieces.\n\
\n\
JUMPING\n\
If you're the fox and need to jump, do the exact same thing.\n\
A fox must jump when given the opportunity.\n\
Enter in the coordinates of the piece you want to move and\n\
the coordinates you want to jump to.\n\
The jump will kill the geese and remove the piece from the board.\n\
Sometimes the fox has the option of jumping multiple times,\n\
Enter in the move of your first jump. You will then be given the\n\
chance to enter the moves of your second jump. When you don't want\n\
to jump any longer, enter a 'd' (Done) to end your turn."                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         ;
	}
	else
	{
		kHelpTextInterface    =
		        "Your move consists of the location of the piece you want to move\n\
and the location of where you want to move the piece to.\n\
\nFor example, the move\n\
\t[c4 c5]  (brackets are not necessary)\n\
will move the piece located at c4 and move it to c5.\n\
\n\
Note: You can only move your own pieces.\n\
\n\
JUMPING\n\
If you're the fox and need to jump, do the exact same thing,\n\
A fox must jump when given the opportunity.\n\
entering in the coordinates of the piece you want to move and\n\
the coordinates you want to jump to.\n\
The jump will kill the geese and remove the piece from the board.\n\
You have disabled the opportunity to perform multiple jumps."                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   ;
	}
	if (variant_geeseMoveBackwards)
	{
		kHelpOnYourTurn =
		        "Both the fox and the geese can move to any adjacent empty square that\n\
is connected to a piece by a line. Note that you can only move to a\n\
few spots using a diagonal route."                                                                                                                                                                         ;
	}
	else
	{
		kHelpOnYourTurn =
		        "Both the fox and the geese can move to any adjacent empty square that\n\
is connected to a piece by a line. Note that you can only move to a\n\
few spots using a diagonal route. The geese cannot move away from the\n\
'castle' area."                                                                                                                                                                                                                                                  ;
	}

	kHelpStandardObjective =
	        "If you're the fox...\n\
\n\
Kill all the geese before they occupy the 'castle' area.\n\
In general, this means kill as many as you can.\n\
\n\
If you're the geese...\n\
\n\
Occupy the 'castle' area with a user-defined number of geese.\n\
Alternately, the geese can win if they trap the foxes so they\n\
cannot move.\n\
\n\
The castle is defined to be the locations of these coordinates:\n\
b1, c1, d2, b2, c2, d2."                                                                                                                                                                                                                                                                                                                                                                                                                       ;

	kHelpReverseObjective =
	        "If you're the fox...\n\
\n\
Try to let the geese take over the 'castle' area, or \n\
let them trap you.\n\
\n\
If you're the geese...\n\
\n\
Try to let the fox kill you.\n\
\n\
The castle is defined to be the locations of these coordinates:\n\
b1, c1, d2, b2, c2, d2."                                                                                                                                                                                                                                                                     ;

	kHelpTieOccursWhen = /* Should follow 'A Tie occurs when... */
	                     "A tie can never occur in Asalto.";

	kHelpGraphicInterface =
	        "No Graphic Interface with Asalto Right Now";

	kHelpExample = "Type '?' if you need assistance... \n \
\n \
\n \
        +-------------------------------------------+\n \
        | GAMESMAN Asalto                           |\n \
        +---------------------------+---------------+\n \
        |                           |               |\n \
        |  5      G - G - G         |               |\n \
        |         | / | \\ |         |               |\n \
        |  4    -   -   -   -       |               |\n \
        |     | / | \\ | / | \\ |     |   F = Foxes   |\n \
        |  3    -   -   -   -       |   G = Geese   |\n \
        |     | \\ | / | \\ | / |     |               |\n \
        |  2    -   -   -   -       |   3 Geese     |\n \
        |         | \\ | / |         |     Needed    |\n \
        |  1      F -   - F         |     To Win    |\n \
        |     a   b   c   d   e     |               |\n \
        |                           |               |\n \
        +---------------------------+---------------+\n \
        | It is Geese's turn (G Piece).             |\n \
        | (Geese should Win in 8)                   |\n \
        +-------------------------------------------+\n \
\n \
   Geese's move [(u)ndo/([A-E][1-5])] :  c5 c4\n \
\n \
        +-------------------------------------------+\n \
        | GAMESMAN Asalto                           |\n \
        +---------------------------+---------------+\n \
        |                           |               |\n \
        |  5      G -   - G         |               |\n \
        |         | / | \\ |         |               |\n \
        |  4    -   - G -   -       |               |\n \
        |     | / | \\ | / | \\ |     |   F = Foxes   |\n \
        |  3    -   -   -   -       |   G = Geese   |\n \
        |     | \\ | / | \\ | / |     |               |\n \
        |  2    -   -   -   -       |   3 Geese     |\n \
        |         | \\ | / |         |     Needed    |\n \
        |  1      F -   - F         |     To Win    |\n \
        |     a   b   c   d   e     |               |\n \
        |                           |               |\n \
        +---------------------------+---------------+\n \
        | It is Foxes's turn (F Piece).             |\n \
        | (Foxes should Lose in 7)                  |\n \
        +-------------------------------------------+\n \
\n \
\n \
   Foxes's move [(u)ndo/([A-E][1-5])] :  D1 D2\n \
\n \
        +-------------------------------------------+\n \
        | GAMESMAN Asalto                           |\n \
        +---------------------------+---------------+\n \
        |                           |               |\n \
        |  5      G -   - G         |               |\n \
        |         | / | \\ |         |               |\n \
        |  4    -   - G -   -       |               |\n \
        |     | / | \\ | / | \\ |     |   F = Foxes   |\n \
        |  3    -   -   -   -       |   G = Geese   |\n \
        |     | \\ | / | \\ | / |     |               |\n \
        |  2    -   -   - F -       |   3 Geese     |\n \
        |         | \\ | / |         |     Needed    |\n \
        |  1      F -   -           |     To Win    |\n \
        |     a   b   c   d   e     |               |\n \
        |                           |               |\n \
        +---------------------------+---------------+\n \
        | It is Geese's turn (G Piece).             |\n \
        | (Geese should Win in 6)                   |\n \
        +-------------------------------------------+\n \
\n \
   Geese's move [(u)ndo/([A-E][1-5])] :  c4 c3\n \
\n \
        +-------------------------------------------+\n \
        | GAMESMAN Asalto                           |\n \
        +---------------------------+---------------+\n \
        |                           |               |\n \
        |  5      G -   - G         |               |\n \
        |         | / | \\ |         |               |\n \
        |  4    -   -   -   -       |               |\n \
        |     | / | \\ | / | \\ |     |   F = Foxes   |\n \
        |  3    -   - G -   -       |   G = Geese   |\n \
        |     | \\ | / | \\ | / |     |               |\n \
        |  2    -   -   - F -       |   3 Geese     |\n \
        |         | \\ | / |         |     Needed    |\n \
        |  1      F -   -           |     To Win    |\n \
        |     a   b   c   d   e     |               |\n \
        |                           |          TRUE     |\n \
        +---------------------------+---------------+\n \
        | It is Foxes's turn (F Piece).             |\n \
        | (Foxes should Win in 1)                   |\n \
        +-------------------------------------------+\n \
\n \
   Foxes's move [(u)ndo/([A-E][1-5])] :  D2 B4\n \
\n \
        +-------------------------------------------+\n \
        | GAMESMAN Asalto                           |\n \
        +---------------------------+---------------+\n \
        |                           |               |\n \
        |  5      G -   - G         |               |\n \
        |         | / | \\ |         |               |\n \
        |  4    - F -   -   -       |               |\n \
        |     | / | \\ | / | \\ |     |   F = Foxes   |\n \
        |  3    -   -   -   -       |   G = Geese   |\n \
        |     | \\ | / | \\ | / |     |               |\n \
        |  2    -   -   -   -       |   3 Geese     |\n \
        |         | \\ | / |         |     Needed    |\n \
        |  1      F -   -           |     To Win    |\n \
        |     a   b   c   d   e     |               |\n \
        |                           |               |\n \
        +---------------------------+---------------+\n \
        | It is Geese's turn (G Piece).             |\n \
        | (Geese should Lose in 0)                  |\n \
        +-------------------------------------------+\n \
\n \
\n \
Foxes (player two) Wins!\n";

}

/************************************************************************
**
** NAME:        DebugMenu
**
** DESCRIPTION: Menu used to debug internal problems. Does nothing if
**              kDebugMenu == FALSE
**
************************************************************************/

void DebugMenu ()
{

}


/************************************************************************
**
** NAME:        GameSpecificMenu
**
** DESCRIPTION: Menu used to change game-specific parmeters, such as
**              the side of the board in an nxn Nim board, etc. Does
**              nothing if kGameSpecificMenu == FALSE
**
*******************************origin*****************************************/

void GameSpecificMenu ()
{
	char selection = 'Z';
	POSITION GetInitialPosition();
	do
	{
		printf("\n\t----- Game Specific Options for Asalto ----- \n\n");
		printf("\tCurrent Number of Maximum Positions: " POSITION_FORMAT, gNumberOfPositions);
		printf("\n\n");
		printf("\tm)\t(M)odify Board\n");
		printf("\tg)\tToggle (G)o Again from ");
		if(variant_goAgain)
		{
			printf("ENABLED to DISABLED.\n");
		}
		else
		{
			printf("DISABLED to ENABLED.\n");
		}
		printf("\td)\tToggle (D)iagonals from ");
		if(variant_diagonals)
		{
			printf("ENABLED to DISABLED.\n");
		}
		else
		{
			printf("DISABLED to ENABLED.\n");
		}
		printf("\ta)\tToggle (A)llow Geese To Move Backwards ");
		if(variant_geeseMoveBackwards)
		{
			printf("ENABLED to DISABLED.\n");
		}
		else
		{
			printf("DISABLED to ENABLED.\n");
		} if (INIT_DEBUG) { printf("mASALTO - InitializeGame() --> generic_hash_init\n");  }
		printf("\tn)\tChange (N)umber of Geese in Castle To Win (Currently %d)\n",GEESE_MIN);

		printf("\tb)\t(B)ack to previous menu\n\n");
		printf("Select an option: ");
		selection = toupper(GetMyChar());
		switch (selection)
		{
		case 'M':
			gInitialPosition = GetInitialPosition();
			break;
		case 'G':
			variant_goAgain = (variant_goAgain) ? 0 : 1;
			selection = 'Z';
			break;
		case 'D':
			variant_diagonals = (variant_diagonals) ? 0 : 1;
			selection = 'Z';
			break;
		case 'A':
			variant_geeseMoveBackwards = (variant_geeseMoveBackwards) ? 0 : 1;
			selection = 'Z';
			break;
		case 'N':
		{
			int OLD_GEESE_MIN = GEESE_MIN;
			do
			{
				char input[80];

				printf("How many geese are needed to win [(C)ancel]? "); GetMyStr(input, 80);
				input[0] = toupper(input[0]);
				if (input[0] == 'C')
				{
					GEESE_MIN = OLD_GEESE_MIN;
				}
				else
				{
					GEESE_MIN = input[0] - '0';
					if ((GEESE_MIN < 0 || 6 < GEESE_MIN))
					{
						printf("At least 1 goose and at most 6 geese can be set as a winning condition for Asalto.\n\n");
					}
					else if (GEESE_MIN > GEESE_MAX)
					{
						printf("You can only require at most %d geese because you only have %d geese on the board!\n\n",GEESE_MAX,GEESE_MAX);
					}
				}
			} while (GEESE_MIN < 0 || 6 < GEESE_MIN || GEESE_MIN > GEESE_MAX);
			selection = 'Z';
		}
		break;
		case 'B':
			return;
		default:
			printf("Invalid Option.\n");
			break;
		}
		selection = '0';
		init_board_hash();
	} while (1);
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
** NAME:        DoMove
**
** DESCRIPTION: Apply the move to the position.
**
** INPUTS:      POSITION thePosition : The old position
**              MOVE     theMove     : The move to apply.
**
** OUTPUTS:     (POSITION) : The position that results after the move.
**
** CALLS:       Hash ()
**              Unhash ()
**	            LIST OTHER CALLS HERE
*************************************************************************/

POSITION DoMove (POSITION thePosition, MOVE theMove)
{
	int move[] = {0,0};
	char board[BOARDSIZE];
	char origPiece = '*';
	int origin = -1;
	int destination = -1;
	int coord_origin[2];
	int coord_destination[2];
	int next_player = 0;
	int goAgainFoxNum = 0;

	if (DOMOVE_DEBUG) { printf("mASALTO - DoMove() Running...\n"); }
	generic_hash_unhash(getPosition(thePosition), board);
	if (DOMOVE_TEST) { printf("CURRENT BOARD: %d\n",theMove); PrintBoard(board); }

	unHashMove(theMove, move);

	origin = move[0];
	destination = move[1];
	origPiece = board[origin];

	if (theMove == -1)
	{
		next_player = (generic_hash_turn(getPosition(thePosition)) == GEESE_PLAYER) ? FOX_PLAYER : GEESE_PLAYER;
		return mergePositionGoAgain(generic_hash_hash(board,next_player), 0);
	}

	/* Barebones move */
	board[origin] = ' ';
	board[destination] = origPiece;
	locationToCoord(origin,coord_origin);
	locationToCoord(destination,coord_destination);
	/* Now check if we need to remove pieces */
	if(origPiece == 'F' &&
	   (abs(coord_origin[0] - coord_destination[0]) == 2 || abs(coord_origin[1] - coord_destination[1]) == 2))
	{
		int del_coord[2];
		int del_loc=-1;
		del_coord[0] = (coord_origin[0] + coord_destination[0]) / 2;
		del_coord[1] = (coord_origin[1] + coord_destination[1]) / 2;
		del_loc = coordToLocation(del_coord);
		board[del_loc] = ' ';
		if (variant_goAgain && MoreKillMoves(board, destination, FOX_PLAYER)) /* Go Again */
		{
			goAgainFoxNum = getFoxNumber(board, destination);
			next_player = generic_hash_turn(getPosition(thePosition));
		}
		else
		{
			next_player = (generic_hash_turn(getPosition(thePosition)) == GEESE_PLAYER) ? FOX_PLAYER : GEESE_PLAYER;
		}
	}
	else
	{
		next_player = (generic_hash_turn(getPosition(thePosition)) == GEESE_PLAYER) ? FOX_PLAYER : GEESE_PLAYER;
	}

	if (DOMOVE_TEST)
	{
		printf("MOVE FOR NEXT BOARD: "); PrintMove(theMove);
		printf("\n");
		printf("NEXT PLAYER: ");
		if (next_player == FOX_PLAYER)
		{
			printf("%d Fox",next_player);
		}
		else if (next_player == GEESE_PLAYER)
		{
			printf("%d Geese",next_player);
		}
		printf("  goAgainFoxNum = %d \n",goAgainFoxNum);
		PrintBoard(board);
	}
	if (DOMOVE_DEBUG) { printf("mASALTO - DoMove() Done\n"); }
	return mergePositionGoAgain(generic_hash_hash(board,next_player),goAgainFoxNum);
}

BOOLEAN GoAgain(POSITION pos, MOVE move)
{
	int player = generic_hash_turn(getPosition(pos));
	if(GAMESMAN_GOAGAIN_DEBUG)
	{
		printf("Move: "); PrintMove(move); printf("    ");
		printf("player == %d ",player);
	}
	if (variant_goAgain && player == FOX_PLAYER)
	{
		int moveArray[2];
		int coord_origin[2];
		int coord_destination[2];
		unHashMove(move,moveArray);
		locationToCoord(moveArray[0],coord_origin);
		locationToCoord(moveArray[1],coord_destination);
		if(GAMESMAN_GOAGAIN_DEBUG)
		{
			printf(" FOX PLAYER DETECT ");
			printf(" init = %d   dest = %d  ", moveArray[0],moveArray[1]);
			printf(" deltax = %d   deltay = %d ",abs(coord_origin[0] - coord_destination[0]),abs(coord_origin[1] - coord_destination[1]));
		}
		if (abs(coord_origin[0] - coord_destination[0]) == 2 || abs(coord_origin[1] - coord_destination[1]) == 2)
		{
			char board[BOARDSIZE];
			generic_hash_unhash(getPosition(pos), board);
			unHashMove(move,moveArray);
			if(GAMESMAN_GOAGAIN_DEBUG) {printf("GO AGAIN %d \n", MoreKillMoves(board, moveArray[1], FOX_PLAYER)); }
			return MoreKillMoves(board, moveArray[1], FOX_PLAYER);
		}
		else
		{
			if(GAMESMAN_GOAGAIN_DEBUG) { printf("NO GO AGAIN\n"); }
			return 0;
		}
	}
	else
	{
		if(GAMESMAN_GOAGAIN_DEBUG) {printf("NO GO AGAIN\n"); }
		return 0;
	}
	return 0;
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
************************************************************************/

POSITION GetInitialPosition()
{
	int boardStats[2];
	char selection = 'Z';

	do
	{
		boardPieceStats(start_standard_board, boardStats);
		printf("\n\t----- Asalto Initial Position Setup -----\n\n");
		printf("\tCurrent Number of Maximum Positions: " POSITION_FORMAT, gNumberOfPositions);
		printf("\n\n");
		printf("\tCurrent Board\n");

		PrintBoard(start_standard_board);

		printf("\tg)\tAdd/Remove (G)eese\n");

		if (numFoxes(boardStats) < 2)
		{
			printf("\tf)\tAdd/Remove (F)oxes [REQUIRED]\n");
			printf("\t  \tYou must place two foxes to play the game.\n");
		}
		else
		{
			printf("\tf)\tAdd/Remove (F)oxes\n");
			printf("\tb)\t(B)ack to previous menu\n\n");
		}
		printf("Select an option: ");
		selection = toupper(GetMyChar());
		switch (selection)
		{
		case 'G':
			AddRemoveGeese(start_standard_board);
			selection = 'Z';
			break;
		case 'F':
			AddRemoveFoxes(start_standard_board);
			selection= 'Z';
			break;
		case 'B':
			break;
		default:
			printf("Invalid option. Try again\n");
			selection = -1;
		}
		boardPieceStats(start_standard_board, boardStats);
		GEESE_MAX = numGeese(boardStats);
		FOX_MAX = numFoxes(boardStats);

		init_board_hash();
	} while (selection != 'B');


	return mergePositionGoAgain(generic_hash_hash(start_standard_board, GEESE_PLAYER),0);
}

/************************************************************************
**
** NAME:        PrintComputersMove
**
** DESCRIPTION: Nicely format the computers move.
**
** INPUTS:      MOVE    computersMove : The computer's move.
**              STRING  computersName : The computer's name.
**
************************************************************************/

void PrintComputersMove(MOVE computersMove, STRING computersName)
{
	int origin_coord[2];
	int destination_coord[2];
	int move[2];
	char origin_grid[2];
	char destination_grid[2];

	unHashMove(computersMove,move);
	locationToCoord(move[0],origin_coord);
	locationToCoord(move[1],destination_coord);

	coordToGridCoordinate(origin_coord,origin_grid);
	coordToGridCoordinate(destination_coord,destination_grid);

	printf("\n%s moved from %c%c to %c%c\n",computersName,origin_grid[0],origin_grid[1],destination_grid[0],destination_grid[1]);
}


/************************************************************************
**
** NAME:        Primitive
**
** DESCRIPTION: Return the value of a position if it fulfills certain
**              'primitive' constraints. Some examples of this is having
**              three-in-a-row with Gobblet. Three in a row for the player
**              whose turn it is a win, otherwise its a loss.
**              Otherwise undecided.
**
** INPUTS:      POSITION position : The position to inspect.
**
** OUTPUTS:     (VALUE) an enum which is oneof: (win,lose,tie,undecided)
**
** CALLS:       LIST FUNCTION CALLS
**
**
************************************************************************/

VALUE Primitive (POSITION pos)
{
	int boardStats[]={0,0,0};
	char board[BOARDSIZE];
	int player = generic_hash_turn(getPosition(pos));


	if (PRIMITIVE_DEBUG) { printf("mASALTO - Primitive() Running...\n"); }
	generic_hash_unhash(getPosition(pos), board);
	boardPieceStats(board, boardStats);
	if (PRIMITIVE_DEBUG)
	{
		printf("mASALTO - Primitive() Returning...\n");
		if (player == GEESE_PLAYER)
		{
			printf("Goose Player    ");
		}
		else
		{
			printf("Fox Player      ");
		}
		printf("NumGeese = %d    NumWinGeese = %d  ",numGeese(boardStats), numWinGeese(boardStats));
	}
	if (numGeese(boardStats) < GEESE_MIN)
	{
		if (player == GEESE_PLAYER)
		{
			if (PRIMITIVE_DEBUG)
			{
				if (gStandardGame)
				{
					printf("Lose\n");
				}
				else
				{
					printf("Win\n");
				}
			}
			return (gStandardGame ? lose : win);
		}
		else if (player == FOX_PLAYER)
		{
			if (PRIMITIVE_DEBUG)
			{
				if (gStandardGame)
				{
					printf("Win\n");
				}
				else
				{
					printf("Lose\n");
				}
			}
			return (gStandardGame ? win : lose);
		}

	}
	else if(numWinGeese(boardStats) == GEESE_MIN)
	{
		if (player == GEESE_PLAYER)
		{
			if (PRIMITIVE_DEBUG)
			{
				if (gStandardGame)
				{
					printf("Win\n");
				}
				else
				{
					printf("Lose\n");
				}
			}
			return (gStandardGame ? win : lose);
		}
		else if (player == FOX_PLAYER)
		{
			if (PRIMITIVE_DEBUG)
			{
				if (gStandardGame)
				{
					printf("Lose\n");
				}
				else
				{
					printf("Win\n");
				}
			}
			return (gStandardGame ? lose : win);
		}
	}
	else if(player == FOX_PLAYER && CantMove(board,player))
	{
		if (PRIMITIVE_DEBUG)
		{
			if (gStandardGame)
			{
				printf("Lose\n");
			}
			else
			{
				printf("Win\n");
			}
		}
		return (gStandardGame ? lose : win);
	}
	else
	{
		if (PRIMITIVE_DEBUG) { printf("Undecided\n"); }
		return undecided;
	}
	return undecided; /* Ummm... We shouldn't be here */
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
** CALLS:       Unhash()
**              GetPrediction()
**              LIST OTHER CALLS HERE
**
************************************************************************/

void PrintPosition (POSITION position, STRING playerName, BOOLEAN usersTurn)
{
	char currentBoard[BOARDSIZE];
	char turnString[80];
	char prediction[80];
	char piece;
	int width = strlen("+-------------------------------------------+");

	generic_hash_unhash(getPosition(position), currentBoard);

	if (generic_hash_turn(getPosition(position)) ==  FOX_PLAYER)
	{
		piece = 'F';
	}
	else if (generic_hash_turn(getPosition(position)) ==  GEESE_PLAYER)
	{
		piece = 'G';
	}
	else
	{
		piece = 'Z';
	}

	if (PRINTPOSITION_DEBUG) { printf("Position Hash " POSITION_FORMAT "\n",position); }

	printf("\n");

	PrintBoard(currentBoard);

	sprintf(turnString,"| It is %s's turn (%c Piece).", playerName,piece);
	printf("\t%s",turnString); PrintSpaces(width - strlen(turnString) - 1); printf("|\n");

	sprintf(prediction,"| %s",GetPrediction(position,playerName,usersTurn));
	if (prediction[2] == '(')
	{
		printf("\t%s",prediction); PrintSpaces(width - strlen(prediction) - 1); printf("|\n");
	}
	printf("\t+-------------------------------------------+\n\n");
}

void PrintSpaces(int spaces)
{
	for (; spaces > 0; spaces--)
	{
		printf(" ");
	}
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
** CALLS:       GENERIC_PTR SafeMalloc(int)
**              LIST OTHER CALLS HERE
**
************************************************************************/

MOVELIST *GenerateMoves (POSITION position)
{
	MOVELIST *CreateMovelistNode();
	MOVELIST *moves = NULL;
	char board[BOARDSIZE];
	int i=0;
	int player = generic_hash_turn(getPosition(position));
	int positionGoAgainFoxNum = getGoAgainFoxNum(position);

	if (GENERATEMOVES_DEBUG)
	{
		printf("mASALTO - GenerateMoves() Running...\n");
		printf("mASALTO - GenerateMoves() --> pos: " POSITION_FORMAT "  player: %d\n",position,player);
	}

	generic_hash_unhash(getPosition(position), board);

	if (player == FOX_PLAYER && positionGoAgainFoxNum)
	{
		if (GENERATEMOVES_DEBUG) { printf("mASALTO - GenerateMoves() --> GenerateKillMoves()\n"); }
		moves = GenerateKillMoves(board,moves,getFoxPos(board,positionGoAgainFoxNum),FOX_PLAYER);
		moves = CreateMovelistNode(-1,moves);
	}
	else if(player == FOX_PLAYER)
	{
		if (GENERATEMOVES_DEBUG) { printf("mASALTO - GenerateMoves() --> FoxGenerateMoves()\n"); }
		moves = FoxGenerateMoves(board,moves);
	}
	else if (player == GEESE_PLAYER)
	{
		if (GENERATEMOVES_DEBUG) { printf("mASALTO - GenerateMoves() --> GeeseGenerateMoves()\n"); }
		for(i = 0; i < BOARDSIZE; i++) /* Scan For Geese */
		{
			if(board[i] =='G')
			{
				moves = GeeseGenerateMoves(board,moves,i);
			}
		}
	}
	if (GENERATEMOVES_DEBUG) { printf("mASALTO - GenerateMoves() Done.\n"); }
	return(moves);
}

MOVELIST *FoxGenerateMoves(const char board[BOARDSIZE], MOVELIST *moves)
{
	int location = 0;
	int fox_kill_move = 0;
	for(location = 0; location < BOARDSIZE; location++)
	{
		if(board[location] == 'F' && MoreKillMoves(board,location,FOX_PLAYER))
		{
			fox_kill_move++;
		}
	}
	for(location = 0; location < BOARDSIZE; location++) /* Scan For Foxes */
	{
		if(board[location] == 'F')
		{
			if (FOXGENERATEMOVES_DEBUG) { printf("mASALTO - FoxGenerateMoves() Running.\n"); }
			if (fox_kill_move > 0)
			{
				/* Killing Moves */
				moves = GenerateKillMoves(board,moves,location,FOX_PLAYER);
			}
			else if (fox_kill_move == 0)
			{
				moves = GenerateShiftMoves(board,moves,location,FOX_PLAYER);
			}
			else
			{
				printf("FoxGenerateMoves Error\n");
			}
		}
	}

	if (FOXGENERATEMOVES_DEBUG) { printf("mASALTO - FoxGenerateMoves() Done.\n"); }
	return(moves);
}

MOVELIST *GeeseGenerateMoves(const char board[BOARDSIZE], MOVELIST *moves, int location)
{
	/* Standard Shifting Moves for Geese*/
	if (GEESEGENERATEMOVES_DEBUG) { printf("mASALTO - GeeseGenerateMoves() Running.\n"); }
	moves = GenerateShiftMoves(board,moves,location,GEESE_PLAYER);
	if (GEESEGENERATEMOVES_DEBUG) { printf("mASALTO - GeeseGenerateMoves() Done.\n"); }

	return(moves);
}

MOVELIST *GenerateShiftMoves(const char board[BOARDSIZE], MOVELIST *moves, int location, int player)
{
	int delta_row = 0;
	int delta_col = 0;
	int origin_coord[2];
	int destination_coord[2];
	int candidate_move[2] = {location, -1};

	if (GENERATESHIFTMOVES_DEBUG) { printf("mASALTO - GenerateShiftMoves() Running.\n"); }

	locationToCoord(location,origin_coord);

	for(delta_row = -1; delta_row <= 1; delta_row++)
	{
		for(delta_col = -1; delta_col <= 1; delta_col++)
		{
			destination_coord[0] = origin_coord[0] + delta_row;
			destination_coord[1] = origin_coord[1] + delta_col;
			candidate_move[1] = coordToLocation(destination_coord);
			if(validCoord(destination_coord) && validMove(board, candidate_move, player))
			{
				moves = CreateMovelistNode((hashMove(candidate_move)),moves);
			}
		}
	}
	if (GENERATESHIFTMOVES_DEBUG) { printf("mASALTO - GenerateShiftMoves() Done.\n"); }
	return(moves);
}

MOVELIST *GenerateKillMoves(const char board[BOARDSIZE], MOVELIST *moves, int location, int player)
{
	int neighbor_location = 0;
	int neighbor_coord[2] = {-1, -1};

	int neighbor_location_one_beyond = 0;
	int neighbor_coord_one_beyond[2] = {-1, -1};

	int delta_row = 0;
	int delta_col = 0;

	int origin_coord[2];

	int candidate_move[2] = {location, -1};

	if (GENERATEKILLMOVES_DEBUG) { printf("mASALTO - GenerateKillMoves() Running.\n"); }

	locationToCoord(location,origin_coord);

	if (player == FOX_PLAYER)
	{
		for(delta_row = -1; delta_row <= 1; delta_row++)
		{
			for(delta_col = -1; delta_col <= 1; delta_col++)
			{
				neighbor_coord[0] = origin_coord[0] + delta_row;
				neighbor_coord[1] = origin_coord[1] + delta_col;
				neighbor_location = coordToLocation(neighbor_coord);

				neighbor_coord_one_beyond[0] = origin_coord[0] + (2 * delta_row);
				neighbor_coord_one_beyond[1] = origin_coord[1] + (2 * delta_col);
				neighbor_location_one_beyond = coordToLocation(neighbor_coord_one_beyond);

				candidate_move[1] = neighbor_location_one_beyond;

				if(validCoord(neighbor_coord) && validCoord(neighbor_coord_one_beyond) && board[neighbor_location] == 'G' && validMove(board, candidate_move, player))
				{
					moves = CreateMovelistNode((hashMove(candidate_move)),moves);
				}
			}
		}
	}
	if (GENERATEKILLMOVES_DEBUG) { printf("mASALTO - GenerateKillMoves() Done.\n"); }
	return(moves);
}

int MoreKillMoves(const char board[BOARDSIZE], int location, int player)
{
	int neighbor_location = 0;
	int neighbor_coord[2] = {-1, -1};

	int neighbor_location_one_beyond = 0;
	int neighbor_coord_one_beyond[2] = {-1, -1};

	int delta_row = 0;
	int delta_col = 0;

	int origin_coord[2];

	int candidate_move[2] = {location, -1};

	locationToCoord(location,origin_coord);

	if (player == FOX_PLAYER)
	{
		for(delta_row = -1; delta_row <= 1; delta_row++)
		{
			for(delta_col = -1; delta_col <= 1; delta_col++)
			{
				neighbor_coord[0] = origin_coord[0] + delta_row;
				neighbor_coord[1] = origin_coord[1] + delta_col;
				neighbor_location = coordToLocation(neighbor_coord);

				neighbor_coord_one_beyond[0] = origin_coord[0] + (2 * delta_row);
				neighbor_coord_one_beyond[1] = origin_coord[1] + (2 * delta_col);
				neighbor_location_one_beyond = coordToLocation(neighbor_coord_one_beyond);

				candidate_move[1] = neighbor_location_one_beyond;

				if(validCoord(neighbor_coord) && validCoord(neighbor_coord_one_beyond) && board[neighbor_location] == 'G' && validMove(board, candidate_move, player))
				{
					return 1;
				}
			}
		}
		return 0;
	}
	return 0;
}

int CantMove(const char board[BOARDSIZE], int player)
{
	int delta_row = 0;
	int delta_col = 0;
	int origin_coord[2];
	int destination_coord[2];
	int candidate_move[2] = {-1, -1};
	int location = 0;

	for (location = 0; location <= 33; location++)
	{
		candidate_move[0] = location;
		locationToCoord(location,origin_coord);
		for(delta_row = -1; delta_row <= 1; delta_row++)
		{
			for(delta_col = -1; delta_col <= 1; delta_col++)
			{
				destination_coord[0] = origin_coord[0] + delta_row;
				destination_coord[1] = origin_coord[1] + delta_col;
				candidate_move[1] = coordToLocation(destination_coord);
				if(validCoord(destination_coord) && validMove(board, candidate_move, player))
				{
					return 0;
				}
				else if (MoreKillMoves(board, location, player))
				{
					return 0;
				}
			}
		}
	}
	return 1;
}

int validMove(const char board[BOARDSIZE], int move[2],int player)
{
	int origin_coord[2];
	int destination_coord[2];
	int delta_row = 0;
	int delta_col = 0;
	int diagonal = diagonalConnect(move[0]);

	locationToCoord(move[0],origin_coord);
	locationToCoord(move[1],destination_coord);

	delta_row = destination_coord[0] - origin_coord[0];
	delta_col = destination_coord[1] - origin_coord[1];

	if (move[0] == move[1])
	{
		return 0;
	}
	if (board[move[1]] != ' ')
	{
		return 0;
	}
	if (player == FOX_PLAYER)
	{
		if (!validCoord(destination_coord))
		{
			return 0;
		}
		if (variant_diagonals && diagonal)
		{
			return (board[move[1]] == ' ' && validCoord(destination_coord)) ? 1 : 0;
		}
		else
		{
			return (board[move[1]] == ' ' && validCoord(destination_coord) && abs(delta_row) != abs(delta_col)) ? 1 : 0;
		}
	}
	else if (player == GEESE_PLAYER)
	{
		if (!validCoord(destination_coord))
		{
			return 0;
		}
		if (variant_geeseMoveBackwards)
		{
			switch(origin_coord[1])
			{
			case 0:
			case 1:
				if (variant_diagonals && diagonal)
				{
					return (board[move[1]]) ? 1 : 0;
				}
				else
				{
					return (board[move[1]] && abs(delta_row) != abs(delta_col)) ? 1 : 0;
				}
				break;
			case 2:
				if (variant_diagonals && diagonal)
				{
					return (board[move[1]]) ? 1 : 0;
				}
				else
				{
					return (board[move[1]] && abs(delta_row) != abs(delta_col)) ? 1 : 0;
				}
				break;
			case 3:
			case 4:
				if (variant_diagonals && diagonal)
				{
					return (board[move[1]]) ? 1 : 0;
				}
				else
				{
					return (board[move[1]] && abs(delta_row) != abs(delta_col)) ? 1 : 0;
				}
				break;

			}
		}
		else
		{
			switch(origin_coord[1])
			{
			case 0:
			case 1:
				if (variant_diagonals && diagonal)
				{
					return (board[move[1]] && delta_row >= 0 && delta_col >= 0) ? 1 : 0;
				}
				else
				{
					return (board[move[1]] && delta_row >= 0 && delta_col >= 0 && abs(delta_row) != abs(delta_col)) ? 1 : 0;
				}
				break;
			case 2:
				if (variant_diagonals && diagonal)
				{
					return (board[move[1]] && delta_row >= 0) ? 1 : 0;
				}
				else
				{
					return (board[move[1]] && delta_row >= 0 && abs(delta_row) != abs(delta_col)) ? 1 : 0;
				}
				break;
			case 3:
			case 4:
				if (variant_diagonals && diagonal)
				{
					return (board[move[1]] && delta_row >= 0 && delta_col <= 0) ? 1 : 0;
				}
				else
				{
					return (board[move[1]] && delta_row >= 0 && delta_col <= 0 && abs(delta_row) != abs(delta_col)) ? 1 : 0;
				}
				break;

			}
		}
	}
	else
	{
		return 0;
	}
	return 0; /* We shouldn't be here */
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
** CALLS:       validMove(MOVE, POSITION)
**              BOOLEAN PrintPossibleMoves(POSITION) ...Always True!
**
************************************************************************/

USERINPUT GetAndPrintPlayersMove (POSITION thePosition, MOVE *theMove, STRING playerName)
{
	BOOLEAN ValidMove();
	USERINPUT ret, HandleDefaultTextInput();

	if (GETANDPRINT_DEBUG) {printf("mASALTO - GetAndPrintPlayersMove() Start\n"); }

	do
	{
		printf("%8s's move [(u)ndo/([A-E][1-5])] :  ", playerName);

		ret = HandleDefaultTextInput(thePosition, theMove, playerName);
		if (GETANDPRINT_DEBUG) {printf("mASALTO - GetAndPrintPlayersMove() Returning\n"); }
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
** OUTPUTS:     BOOLEAN : TRUE if the input is a valid text input.
**
************************************************************************/

BOOLEAN ValidTextInput (STRING input)
{
	if (VALIDTEXT_DEBUG) {printf("mASALTO - ValidTextInput() Start\n");
		              printf("mASALTO - ValidTextInput() <-- %s\n",input); }

	if(strlen(input) == 1)
	{
		input[0] = toupper(input[0]);
		return input[0] == 'D';
	}
	else if(strlen(input) == 5)
	{
		input[0] = toupper(input[0]);
		input[3] = toupper(input[3]);
		if (VALIDTEXT_DEBUG)
		{
			printf("String Length 5\n");
			printf("Result: %d\n",('A' <= input[0] && input[0] <= 'E')
			       &&
			       ('1' <= input[1] && input[1] <= '5')
			       &&
			       ('A' <= input[3] && input[3] <= 'E')
			       &&
			       ('1' <= input[4] && input[4] <= '5'));
		}

		return ('A' <= input[0] && input[0] <= 'E')
		       &&
		       ('1' <= input[1] && input[1] <= '5')
		       &&
		       ('A' <= input[3] && input[3] <= 'E')
		       &&
		       ('1' <= input[4] && input[4] <= '5');
	}
	else if (strlen(input) == 7)
	{
		input[0] = toupper(input[0]);
		input[3] = toupper(input[3]);
		input[6] = toupper(input[6]);
		if (VALIDTEXT_DEBUG)
		{
			printf("String Length 7\n");
			printf("Result: %d\n",
			       ('A' <= input[0] && input[0] <= 'E')
			       &&
			       ('1' <= input[1] && input[1] <= '5')
			       &&
			       ('A' <= input[3] && input[3] <= 'E')
			       &&
			       ('1' <= input[4] && input[4] <= '5')
			       &&
			       input[6] == 'G'
			       );
		}

		return ('A' <= input[0] && input[0] <= 'E')
		       &&
		       ('1' <= input[1] && input[1] <= '5')
		       &&
		       ('A' <= input[3] && input[3] <= 'E')
		       &&
		       ('1' <= input[4] && input[4] <= '5')
		       &&
		       input[6] == 'G'
		;
	}
	else
	{
		return FALSE;
	}
	return FALSE;
}


/************************************************************************
**
** NAME:        ConvertTextInputToMove
**
** DESCRIPTION: Convert the string input to the internal move representation.
**              No checking if the input is valid is needed as it has
**              already been checked!
**
** INPUTS:      STRING input : The string input the user typed.
**
** OUTPUTS:     MOVE : The move corresponding to the user's input.
**
************************************************************************/

MOVE ConvertTextInputToMove (STRING input)
{
	char origin_grid[2];
	char destination_grid[2];
	int origin_coord[2];
	int destination_coord[2];
	int move[2];

	if (CONVERTEXTINPUTTOMOVE_DEBUG) {printf("mASALTO - ConvertTextInputToMove () Start\n"); }

	if (strlen(input) == 1)
	{
		return -1;
	}

	origin_grid[0] = input[0];
	origin_grid[1] = input[1];

	destination_grid[0] = input[3];
	destination_grid[1] = input[4];

	gridCoordinatetoCoord(origin_grid,origin_coord);
	gridCoordinatetoCoord(destination_grid,destination_coord);

	move[0] = coordToLocation(origin_coord);
	move[1] = coordToLocation(destination_coord);

	if (CONVERTEXTINPUTTOMOVE_DEBUG)
	{
		printf("mASALTO - ConvertTextInputToMove() <-- String: %s\n",input);
		printf("mASALTO - ConvertTextInputToMove() <-- Origin Grid: %c%c\n",origin_grid[0],origin_grid[1]);
		printf("mASALTO - ConvertTextInputToMove() <-- Origin Coord: %d %d\n",origin_coord[0],origin_coord[0]);
		printf("mASALTO - ConvertTextInputToMove() <-- Origin Location %d\n",coordToLocation(origin_coord));
		printf("mASALTO - ConvertTextInputToMove() <-- Destination Grid: %c%c\n",destination_grid[0],destination_grid[1]);
		printf("mASALTO - ConvertTextInputToMove() <-- Destination Coord: %d %d\n",destination_coord[0],destination_coord[1]);
		printf("mASALTO - ConvertTextInputToMove() <-- Destination Location %d\n",coordToLocation(destination_coord));
	}

	return hashMove(move);
}


/************************************************************************
**
** NAME:        PrintMove
**
** DESCRIPTION: Print the move in a nice format.
**
** INPUTS:      MOVE *theMove         : Fixed m3spot case errorThe move to print.
**
************************************************************************/

void PrintMove (MOVE move)
{
	printf( "%s", MoveToString( move ) );
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
	STRING move = (STRING) SafeMalloc(9);

	int moveArray[2];
	char origin_grid[2];
	char destination_grid[2];
	int origin_coord[2];
	int destination_coord[2];

	unHashMove(theMove,moveArray);

	locationToCoord(moveArray[0], origin_coord);
	locationToCoord(moveArray[1], destination_coord);

	coordToGridCoordinate(origin_coord, origin_grid);
	coordToGridCoordinate(destination_coord, destination_grid);

	sprintf(move, "[%c%c %c%c]",origin_grid[0],origin_grid[1],destination_grid[0],destination_grid[1]);

	return move;
}


/************************************************************************
**
** NAME:        NumberOfOptions
**
** DESCRIPTION: Calculates and returns the number of option combinations
**				there are with all the game variations you program.
**
** OUTPUTS:     int : the number of option combination there are.
**
************************************************************************/

int NumberOfOptions ()
{
	return 2*2*2*6;
}


/************************************************************************
**
** NAME:        getOption
**
** DESCRIPTION: A hash function to keep track of all the game variants.
**		Should return a different number for each set of
**		variants.
**
** OUTPUTS:     int : the number representation of the options.
**
************************************************************************/

int getOption()
{
	/* Bit Shifting Hasher */
	int hashed = 0;
	hashed = variant_goAgain | variant_diagonals << 1 | variant_geeseMoveBackwards << 2 | GEESE_MIN << 3;
	if (SETOPTION_DEBUG)
	{
		printf("\nBEFORE   GoAgain %d   Diagonals %d   MoveBackwards %d  GeeseMin %d\n",
		       variant_goAgain, variant_diagonals, variant_geeseMoveBackwards, GEESE_MIN);
		setOption(hashed);
		printf("HASH %d   GoAgain %d   Diagonals %d   MoveBackwards %d  GeeseMin %d\n",
		       hashed, variant_goAgain, variant_diagonals, variant_geeseMoveBackwards, GEESE_MIN);
	}
	return hashed;
}


/************************************************************************
**
** NAME:        setOption
**
** DESCRIPTION: The corresponding unhash for the game variants.
**				Should take the input and set all the appropriate
**				variants.
**
** INPUT:     int : the number representation of the options.
**
************************************************************************/

void setOption(int option)
{
	variant_goAgain = option & 0x1;            /* 0b001 */
	variant_diagonals = option >> 1 & 0x1;
	variant_geeseMoveBackwards = option >> 2 & 0x1;
	GEESE_MIN = option >> 3;
	GEESE_HASH_MIN = GEESE_MIN - 1;
	init_board_hash();
	if (SETOPTION_DEBUG)
	{
		printf("\nGoAgain %d   Diagonals %d   MoveBackwards %d  GeeseMin %d\n",
		       variant_goAgain, variant_diagonals, variant_geeseMoveBackwards, GEESE_MIN);
	}
}


/************************************************************************
*************************************************************************
**         EVERYTHING BELOW THESE LINES IS LOCAL TO THIS FILE
*************************************************************************
************************************************************************/


/************************************************************************
** This is where you can put any helper functions, including your
** hash and unhash functions if you are not using one of the existing
** ones.
************************************************************************/

void PrintBoard (char board[])
{
	if (variant_diagonals)
	{

		printf("\t+-------------------------------------------+\n");
		printf("\t| GAMESMAN Asalto                           |\n");
		printf("\t+---------------------------+---------------+\n");
		printf("\t|                           |               |\n");
		printf("\t|  5      %c - %c - %c         |               |\n",board[0],board[1],board[2]);
		printf("\t|         | / | \\ |         |               |\n");
		printf("\t|  4  %c - %c - %c - %c - %c     |               |\n",board[3],board[4],board[5],board[6],board[7]);
		printf("\t|     | / | \\ | / | \\ |     |   F = Foxes   |\n");
		printf("\t|  3  %c - %c - %c - %c - %c     |   G = Geese   |\n",board[8],board[9],board[10],board[11],board[12]);
		printf("\t|     | \\ | / | \\ | / |     |               |\n");
		printf("\t|  2  %c - %c - %c - %c - %c     |   %d Geese     |\n",board[13],board[14],board[15],board[16],board[17],GEESE_MIN);
		printf("\t|         | \\ | / |         |     Needed    |\n");
		printf("\t|  1      %c - %c - %c         |     To Win    |\n",board[18],board[19],board[20]);
		printf("\t|     a   b   c   d   e     |               |\n");
		printf("\t|                           |               |\n");
		printf("\t+---------------------------+---------------+\n");
	}
	else
	{
		printf("\t+-------------------------------------------+\n");
		printf("\t| GAMESMAN Asalto                           |\n");
		printf("\t+---------------------------+---------------+\n");
		printf("\t|                           |               |\n");
		printf("\t|  5      %c - %c - %c         |               |\n",board[0],board[1],board[2]);
		printf("\t|         |   |   |         |               |\n");
		printf("\t|  4  %c - %c - %c - %c - %c     |               |\n",board[3],board[4],board[5],board[6],board[7]);
		printf("\t|     |   |   |   |   |     |   F = Foxes   |\n");
		printf("\t|  3  %c - %c - %c - %c - %c     |   G = Geese   |\n",board[8],board[9],board[10],board[11],board[12]);
		printf("\t|     |   |   |   |   |     |               |\n");
		printf("\t|  2  %c - %c - %c - %c - %c     |   %d Geese     |\n",board[13],board[14],board[15],board[16],board[17],GEESE_MIN);
		printf("\t|         |   |   |         |     Needed    |\n");
		printf("\t|  1      %c - %c - %c         |     To Win    |\n",board[18],board[19],board[20]);
		printf("\t|     a   b   c   d   e     |               |\n");
		printf("\t|                           |               |\n");
		printf("\t+---------------------------+---------------+\n");
	}
}

int diagonalConnect(int location)
{
	return
	        (location == 1)
	        ||
	        ( (4 <= location && location <= 17) && location%2 == 0)
	        ||
	        (location == 19);
}

void AddRemoveFoxes(char board[])
{
	char selection = 'Z';
	int location = -1;
	int coordinate[2] = {-1,-1};
	int boardStats[3];
	int validCoord=0;

	do
	{
		boardPieceStats(board, boardStats);
		printf("\n\t----- Asalto Fox Placement -----\n\n");
		printf("\tCurrent Number of Maximum Positions: " POSITION_FORMAT, gNumberOfPositions);
		printf("\n\n");
		printf("\tCurrent Board\n");

		PrintBoard(start_standard_board);
		printf("\ta)\t(A)dd a fox\n");
		printf("\tr)\t(R)emove a fox\n");
		if (numFoxes(boardStats) < 2)
		{
			printf("\t  \tYou must place two foxes to play the game\n");
		}
		else
		{
			printf("\tB)\t(B)ack to Previous Menu\n\n");
		}
		printf("Select an option: ");
		selection = toupper(GetMyChar());
		switch (selection)
		{
		case 'A':
			printf("Where do you wish to add a fox [(S)top]?\n");
			validCoord = 0;
			do
			{
				UserInputCoordinate(coordinate);
				if(coordinate[0] < 0)
				{
					break;
				}
				location = coordToLocation(coordinate);
				if (board[location] == ' ')
				{
					board[location] = 'F';
					validCoord = 1;
				}
				else
				{
					printf("A piece is already there.\n");
					validCoord = 0;
				}
			} while(!validCoord);
			selection = 'Z';
			break;
		case 'R':
			printf("Where do you wish to remove a fox?\n");
			validCoord = 0;
			do
			{
				UserInputCoordinate(coordinate);
				if(coordinate[0] < 0)
				{
					break;
				}
				location = coordToLocation(coordinate);
				if (board[location] == 'F')
				{
					board[location] = ' ';
					validCoord = 1;
				}
				else
				{
					printf("That's not a fox.");
					validCoord = 0;
				} selection = 'Z';
			} while(!validCoord);
			selection = 'Z';
			break;
		case 'B':
			if (numFoxes(boardStats) < 2)
			{
				printf("You must have two foxes on the board.\n");
				selection = 'Z';
			}
			else
			{
				break;
			}
		default:
			printf("Invalid option. Try again\n");
			selection = 'Z';
		}
		boardPieceStats(start_standard_board, boardStats);
		GEESE_MAX = numGeese(boardStats);
		FOX_MAX = numFoxes(boardStats);

		init_board_hash();
	} while (selection != 'B');
}

void AddRemoveGeese(char board[])
{
	char selection = 'Z';
	int location = -1;
	int coordinate[2] = {-1,-1};
	int boardStats[2];
	int validCoord=0;

	do
	{
		boardPieceStats(board, boardStats);
		printf("\n\t----- Asalto Geese Placement-----\n\n");
		printf("\tCurrent Number of Maximum Positions: " POSITION_FORMAT, gNumberOfPositions);
		printf("\n\n");
		printf("\tCurrent Board\n");

		PrintBoard(start_standard_board);

		printf("\ta)\t(A)dd a Goose\n");
		printf("\tr)\t(R)emove a Goose\n");
		printf("\tb)\t(B)ack To Previous Menu\n\n");
		printf("Select an option: ");
		selection = toupper(GetMyChar());
		switch (selection)
		{
		case 'A':
			printf("Where do you wish to add a goose [(S)top]?\n");
			validCoord = 0;
			do
			{
				UserInputCoordinate(coordinate);
				if(coordinate[0] < 0)
				{
					break;
				}
				location = coordToLocation(coordinate);
				if (board[location] == ' ')
				{
					board[location] = 'G';
					validCoord = 1;
				}
				else
				{
					printf("A piece is already there.\n");
					validCoord = 0;
				}
			} while(!validCoord);
			selection = 'Z';
			break;
		case 'R':
			printf("Where do you wish to remove a goose [(S)top]?\n");
			validCoord = 0;
			do
			{
				UserInputCoordinate(coordinate);
				if(coordinate[0] < 0)
				{
					break;
				}
				location = coordToLocation(coordinate);
				if (board[location] == 'G')
				{
					board[location] = ' ';
					validCoord = 1;
				}
				else
				{
					printf("That's not a goose.");
					validCoord = 0;
				}
			} while(!validCoord);
			selection = 'Z';
			break;
		case 'B':
			return;
		default:
			printf("Invalid option. Try again\n");
			selection = 'Z';
		}
		boardPieceStats(start_standard_board, boardStats);
		GEESE_MAX = numGeese(boardStats);
		FOX_MAX = numFoxes(boardStats);

		init_board_hash();
	} while (selection != 'B');
}

int hashMove(int move[2]) /* Revamped Bitshiftting Hash Function */
{
	return move[0] | move[1] << 6;
}
void unHashMove (int hashed_move, int move[2])
{
	move[0] = hashed_move & 0x3F; /* 0b111111*/
	move[1] = hashed_move >> 6;
}

void boardPieceStats(char board[BOARDSIZE], int stats[3])
{
	/* stats[0] = Number of Foxes , stats[1] = Number of Geese, stats[2] = Number of Geese in Winning Box */
	int i=0;
	stats[0] = 0; stats[1] = 0; stats[2] = 0;
	for (i=0; i < BOARDSIZE; i++)
	{
		if(board[i]== 'F')
		{
			stats[0]++;
		}
		else if (board[i]== 'G')
		{
			stats[1]++;
			/* 14-16 or 18-20 Winning Zones */
			if ((14 <= i && i <= 16) || (18 <= i && i <= 20))
			{
				stats[2]++;
			}
		}
	}
}

int numFoxes(int stats[3])
{
	return stats[0];
}
int numGeese(int stats[3])
{
	return stats[1];
}
int numWinGeese(int stats[3])
{
	return stats[2];
}

int coordToLocation(int coordinates[2])
{
	int row = coordinates[0];
	int col = coordinates[1];
	if (0 <= row && row <= 4 && 0 <= col && col <=4)
	{
		if (0 == row && 1 <= col && col <= 3)
		{
			return col - 1;
		}
		else if (1 <= row && row <= 3)
		{
			return 3 + (5 * (row - 1)) + col;
		}
		else if (4 == row && 1 <= col && col <= 3)
		{
			return 18 + (col - 1);
		}
		else
		{
			return -1;
		}
	}
	else
	{
		return -1;
	}
}

void locationToCoord(int location, int coordinates[2])
{
	if (0 <= location && location <= 2)
	{
		coordinates[1] = location + 1;
		coordinates[0] = 0;
	}
	else if (3 <= location && location <= 17)
	{
		/* Translate 1 Row */
		location -= 3;
		coordinates[1] = location % 5;
		coordinates[0] = 1 + (location - coordinates[1]) / 5;
	}
	else if (18 <= location && location <= 20)
	{
		/* Translate 5 Down */
		location -= 18;
		coordinates[1] = location + 1;
		coordinates[0] = 4;
	}
}

void coordToGridCoordinate(int coordinate[2], char human[2])
{
	human[0] = (char) (coordinate[1] + 'a');
	human[1] = (char) ((5 - coordinate[0]) + '0');
}
void gridCoordinatetoCoord(char human[2], int coord[2])
{
	human[0] = tolower(human[0]);
	coord[1] = (int) (human[0] - 'a'); // Column Switch
	coord[0] = 5 - (int) (human[1] - '0'); // Row Switch
}


/* 1 for valid, 0 for invalid */
int validCoord(int coord[2])
{
	return (coordToLocation(coord) != -1);
}

/*
        unsafe....

 */

void UserInput(char input[])
{
	scanf("%s",input);
}
void UserInputCoordinate(int coordinate[2])
{
	char input[80];
	int coord[2]={-1, -1};
	char human[2];
	do
	{
		printf("Enter a Coordinate: "); GetMyStr(input, 80);
		input[0] = toupper(input[0]);
		if (input[0] == 'S')
		{
			coordinate[0]=-1;
			coordinate[1]=-1;
			return;
		}
		human[0] = input[0];
		human[1] = input[1];
		gridCoordinatetoCoord(human, coord);
		if(validCoord(coord))
		{
			coordinate[0] = coord[0];
			coordinate[1] = coord[1];
			return;
		}
		else
		{
			printf("Invalid coordinate. Use coordinates of this style: C3\n");
		}

	} while(validCoord(coord) != 1);
}

int mergePositionGoAgain(int position, int goAgainPos)
{
	return goAgainPos | position << NUM_GOAGAIN_BITS;
}

void mergePositionSetRequiredBits(int numFoxes)
{
	numFoxes += 1;
	if (1 < numFoxes && numFoxes <= 3)
	{
		NUM_GOAGAIN_BITS=2;
	}
	else if (3 < numFoxes && numFoxes <= 7)
	{
		NUM_GOAGAIN_BITS=3;
	}
	else if (7 < numFoxes && numFoxes <= 15)
	{
		NUM_GOAGAIN_BITS=4;
	}
	else if (15 < numFoxes && numFoxes <= 31)
	{
		NUM_GOAGAIN_BITS=5;
	}

}

int getPosition(int mergedPos)
{
	return mergedPos >> NUM_GOAGAIN_BITS;
}

int getGoAgainFoxNum(int mergedPos)
{
	return mergedPos & (intpow(2,NUM_GOAGAIN_BITS) - 1);
}
int GoAgainPos(int mergedPos)
{

	return (variant_goAgain && (getGoAgainFoxNum(mergedPos) > 0));
}

int intpow (int base, int exp)
{
	int product = 1;
	for (; exp > 0; exp--)
	{
		product *= base;
	}
	return product;
}

int getFoxNumber(const char board[BOARDSIZE], int loc)
{
	int foxnum = 0;
	int i = 0;
	for (i = 0; i < loc; i++)
	{
		if (board[i] == 'F')
		{
			foxnum++;
		}
	}
	return foxnum + 1;
}

int getFoxPos(const char board[BOARDSIZE], int foxnum)
{
	int i = 0;
	for (i = 0; i < BOARDSIZE; i++)
	{
		if (board[i] == 'F' && foxnum == 1)
		{
			return i;
		}
		else if (board[i] == 'F')
		{
			foxnum--;
		}
	}
	return -1;
}

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
