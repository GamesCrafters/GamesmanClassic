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
**
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
#include <stdlib.h>
#include <unistd.h>	
#include <limits.h>
#include "hash.h"

extern STRING gValueString[];

POSITION gNumberOfPositions  = 0; /* The number of total possible positions | If you are using our hash, this is given by the hash_init() function*/

POSITION gInitialPosition    = 0; /* The initial position (starting board) */
POSITION gMinimalPosition    = 0; /* */
POSITION kBadPosition        = -1; /* A position that will never be used */

STRING   kGameName           = "Asalto"; /* The name of your game */
STRING   kDBName             = "Asalto"; /* The name to store the database under */
BOOLEAN  kPartizan           = TRUE; /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN  kDebugMenu          = TRUE; /* TRUE while debugging */
BOOLEAN  kGameSpecificMenu   = TRUE; /* TRUE if there is a game specific menu*/
BOOLEAN  kTieIsPossible      = FALSE; /* TRUE if a tie is possible */
BOOLEAN  kLoopy               = TRUE; /* TRUE if the game tree will have cycles (a rearranger style game) */
BOOLEAN  kDebugDetermineValue = FALSE; /* TRUE while debugging */
void*	 gGameSpecificTclInit = NULL;

STRING kHelpGraphicInterface =
"No Graphic Interface with Asalto Right Now";

STRING   kHelpTextInterface    =
"Your move consists of the location of the piece you wantto move\n \
and the location of where you want to move the piece to.\n \
\nFor example, the move\n \
    [c4 c5]  (brackets are not necessary)\n \
will move the piece located at c4 and move it to c5.\n\n \
Note: You can only move your own pieces.\n\n \
If you're the fox and need to jump, do the exact same thing.\n \
If you're the fox and wish to jump multiple times, enter a 'G'\n \
\nExample: [B4 B6 G] makes the fox jump over a geese at B5 and the fox\n \
indicates that he wishes to make another jump immediately afterward.\n \
The removal of the geese is factored in automatically.\n \
If you're the fox and have already previously jumped, you can\n \
enter 'p' to pass instead of jumping once again.";

STRING   kHelpOnYourTurn =
"Both the fox and the geese can move to any adjacent empty square that\n \
is connected to a piece by a line. Note that you can only move to a\n \
few spots using a diagonol route"; 

STRING   kHelpStandardObjective =
"If you're the fox...\n\n \
Kill all the geese before they occupy the entire 'castle' area.\n \
In general, this means kill as many as you can.\n\n \
If you're the geese...\n\n \
Occupy the 'castle' area with a user-defined number of geese.\n \
Alternately, the geese can win if they trap the foxes so they\n \
cannot move.";

STRING   kHelpReverseObjective =
"If you're the fox...\n\n \
Try to let the geese take over the 'castle' area, or let\n \
them trap you.\n\n \
If you're the geese...\n\n \
Try to let the fox kill you as much as possible.";

STRING   kHelpTieOccursWhen = /* Should follow 'A Tie occurs when... */
"A tie can never occur in Asalto.";

STRING   kHelpExample =
"Help Example On The Way!";

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
int GEESE_MAX = 9;
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

char start_standard_board[]={                'G','G','G',
			                 ' ',' ',' ',' ',' ',    
				         ' ',' ',' ',' ',' ',    
				         ' ',' ','G',' ',' ',    
				             'F','F','F'};
					                     

/*************************************************************************
**
** Below is where you put your #define's and your global variables, structs
**
*************************************************************************/

int freeGeeseLocations[24];

int variant_goAgain = 1;
int variant_diagonals = 1;
int variant_geeseMoveBackwards = 0;

int positionGoAgain = 0;
int positionGoAgainPiece = -1; /* Stores New Location of Piece */

/*************************************************************************
**
** Above is where you put your #define's and your global variables, structs
**
*************************************************************************/

/*
** Function Prototypes:
*/

void init_board_hash();

void PrintBoard(char board[]);

int diagonalConnect(int location);

BOOLEAN GoAgain (POSITION, MOVE);

MOVELIST *FoxGenerateMoves(const char board[BOARDSIZE], MOVELIST *moves);
MOVELIST *GeeseGenerateMoves(const char board[BOARDSIZE], MOVELIST *moves, int location);
MOVELIST *GenerateShiftMoves(const char board[BOARDSIZE], MOVELIST *moves, int location, int player);
MOVELIST *GenerateKillMoves(const char board[BOARDSIZE], MOVELIST *moves, int location, int player);
int MoreKillMoves(const char board[BOARDSIZE], int location, int player);
int validMove(const char board[BOARDSIZE], int move[3],int player);
int CantMove(const char board[BOARDSIZE], int player);

int hashMove(int move[3]);
void unHashMove(int hashed_move, int move[3]);

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

void initFreeGoose();
void addFreeGoose(int geeseLocation);
void updateFreeGoose(int geeseOrigin, int geeseDestination);
int freeGoose(int geeseLocation);

void coordToGridCoordinate(int coordinate[2], char human[2]);
void gridCoordinatetoCoord(char human[2], int coord[2]);

/* External */
extern GENERIC_PTR	SafeMalloc ();
extern void		SafeFree ();

/*************************************************************************
**
** Here we declare the global database variables
**
**************************************************************************/

extern VALUE     *gDatabase;
extern BOOLEAN  (*gGoAgain)(POSITION, MOVE);


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
	int hash_data[] =  {' ', WHITESPACE, BOARDSIZE,
		  	    'F', FOX_MIN, FOX_MAX,
		            'G', GEESE_HASH_MIN, GEESE_MAX, -1};
	int max;
	int init;
	
	if (INIT_DEBUG) { printf("mASALTO - InitializeGame() Running...\n"); }
	/* Initialize Hash Function */
	
	if (INIT_DEBUG) { printf("mASALTO - InitializeGame() --> generic_hash_init\n"); 
	max = generic_hash_init(BOARDSIZE, hash_data, NULL);
	}
		
	if (INIT_DEBUG) { printf("mASALTO - InitializeGame() <-- generic_hash_init: %d\n",max); }
	
	if (INIT_DEBUG) { printf("mASALTO - InitializeGame() --> generic_hash\n"); init = generic_hash(start_standard_board,GEESE_PLAYER);}
	
	if (HASH_TEST) {printf("INIT CURRENT BOARD\n"); PrintBoard(start_standard_board);}
			
	if (INIT_DEBUG) { printf("mASALTO - InitializeGame() <-- generic_hash: %d\n",init); }

	init_board_hash();
	
	if (HASH_TEST)
	{
		char test_board[BOARDSIZE];
		printf("Hash Test. Unhashed Board. Hash Value %d\n", init);
		generic_unhash(init, test_board);
		PrintBoard(test_board);
	}
	
	if (INIT_DEBUG) { printf("mASALTO - InitializeGame() Done\n"); }
	
	/* GoAgain assignment moved here. -JJ */
	gGoAgain = GoAgain;
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
	char selection_command[80];
	char selection = 'Z';
	POSITION GetInitialPosition();
	do
	{
		printf("\n\tGamesman Module Asalto Options\n");
		printf("\t==============================\n");
		printf("\t(1) Modify Board\n");
		if(variant_goAgain)
		{
			printf("\t(2) Toggle GoAgain from (Enabled) to Disabled.\n");
		}
		else
		{
			printf("\t(2) Toggle GoAgain from (Disabled) to Enabled.\n");
		}
		if(variant_diagonals)
		{
			printf("\t(3) Toggle Diagonals from (Enabled) to Disabled.\n");
		}
		else
		{
			printf("\t(3) Toggle Diagonals from (Disabled) to Enabled.\n");
		}
		if(variant_geeseMoveBackwards)
		{
			printf("\t(4) Toggle Allow Geese To Move Backwards (Enabled) to Disabled.\n");
		}
		else
		{
			printf("\t(4) Toggle Allow Geese To Move Backwards (Disabled) to Enabled.\n");
		}
		printf("\t(5) Change Number of Geese in Castle To Win (Currently %d)\n",GEESE_MIN);
		
		printf("\t(6) Exit Options\n");
		printf("Selection: "); scanf("%s", selection_command);
		selection = selection_command[0];
		switch (selection)
		{
			case '1':
				gInitialPosition = GetInitialPosition();
				selection = 'Z';
				break;
			case '2':
				variant_goAgain = (variant_goAgain) ? 0 : 1;
				selection = 'Z';
				break;
			case '3':
				variant_diagonals = (variant_diagonals) ? 0 : 1;
				selection = 'Z';
				break;
			case '4':
				variant_geeseMoveBackwards = (variant_geeseMoveBackwards) ? 0 : 1;
				selection = 'Z';
				break;
			case '5':
				do
				{
					char input[80];
					printf("How many geese are needed to win? "); scanf("%s",input);
					GEESE_MIN = input[0] - '0';
				}while (GEESE_MIN < 0 || 6 <= GEESE_MIN);
				init_board_hash();
				selection = 'Z';
				break;
			case '6':
				return;
			default:
				printf("Invalid Option.\n");
				selection = 'Z';
				break;
			
		}
	} while (selection != 6);
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
	int move[] = {0,0,0};
	char board[BOARDSIZE];
	char origPiece = '*';
	int origin = -1;
	int destination = -1;
	int coord_origin[2];
	int coord_destination[2];
	int next_player = 0;
	int goAgain = 0;
	
	if (DOMOVE_DEBUG) { printf("mASALTO - DoMove() Running...\n"); }
	generic_unhash(thePosition, board);
	if (DOMOVE_TEST) { printf("CURRENT BOARD: %d\n",theMove); PrintBoard(board);}
	
	unHashMove(theMove, move);
	
	origin = move[0];
	destination = move[1];
	goAgain = move[2];
	origPiece = board[origin];
	
	if (theMove == -1)
	{
		next_player = (whoseMove(thePosition) == GEESE_PLAYER) ? FOX_PLAYER : GEESE_PLAYER;
		positionGoAgain = 0;
		positionGoAgainPiece = -1;
		return generic_hash(board,next_player);
	}
	
	if (goAgain)
	{
		next_player = whoseMove(thePosition);
		positionGoAgain = 1;
		positionGoAgainPiece = move[1];
	}
	else
	{
		next_player = (whoseMove(thePosition) == GEESE_PLAYER) ? FOX_PLAYER : GEESE_PLAYER;
	}
	
	/* Barebones move */
	board[origin] = ' ';
	board[destination] = origPiece;
	locationToCoord(origin,coord_origin);
	locationToCoord(destination,coord_destination);
	/* Now check if we need to remove pieces */
	if(origPiece = 'F' && 
	  (abs(coord_origin[0] - coord_destination[0]) == 2 || abs(coord_origin[1] - coord_destination[1]) == 2))
	{
		int del_coord[2];
		int del_loc=-1;
		del_coord[0] = (coord_origin[0] + coord_destination[0]) / 2;
		del_coord[1] = (coord_origin[1] + coord_destination[1]) / 2;
		del_loc = coordToLocation(del_coord);
		board[del_loc] = ' ';
	}
	
	if (DOMOVE_TEST)
	{
		printf("MOVE FOR NEXT BOARD: "); PrintMove(theMove);
		printf("\n");
		printf("NEXT PLAYER: ");
		if (next_player == FOX_PLAYER)
		{
			printf("%d Fox\n",next_player);
		}
		else if (next_player == GEESE_PLAYER,next_player)
		{
			printf("%d Geese\n");
		} 
		PrintBoard(board);
	}
	if (DOMOVE_DEBUG) { printf("mASALTO - DoMove() Done\n"); }
	return generic_hash(board,next_player);
}

BOOLEAN GoAgain(POSITION pos, MOVE move)
{
	int unHashedMove[3];
	unHashMove(move, unHashedMove);
	return unHashedMove[2]==1;
}
/*
** Line moved up to InitializeGame. -JJ
** BOOLEAN (*gGoAgain)(POSITION,MOVE) = GoAgain;
**/

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
	int coordinate[2];
	int location = -1;
	char selection_command[80];
	char selection = 'Z';
	
	do
	{
		boardPieceStats(start_standard_board, boardStats);
		printf("\n\n\n\n\n\nAsalto Initial Position Setup\n");
		printf("=============================\n");
		printf("Current Board\n");
		PrintBoard(start_standard_board);
		printf("(1) Add/Remove Geese\n");
		if (numFoxes(boardStats) < 2)
		{
			printf("(2) Add/Remove Foxes [REQUIRED]\n");
			printf("You must place two foxes to play the game.\n");
		}
		else
		{
			printf("(2) Add/Remove Foxes\n");
			printf("(3) Exit Initial Setup\n");
		}
		printf("Selection: "); scanf("%s", selection_command);
		selection = selection_command[0];
		switch (selection)
		{
			case '1':
				AddRemoveGeese(start_standard_board);
				selection = 'Z';
				break;
			case '2':
				AddRemoveFoxes(start_standard_board);
				selection= 'Z';
				break;
			case '3':
				break;
			default:
				printf("Invalid option. Try again\n");
				selection = -1;
		}
		
	} while (selection != '3');
	
	boardPieceStats(start_standard_board, boardStats);
	GEESE_MAX = numGeese(boardStats);
	FOX_MAX = numFoxes(boardStats);
	
	init_board_hash();
	
	return(generic_hash(start_standard_board, GEESE_PLAYER));
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
	max = generic_hash_init(BOARDSIZE, hash_data, NULL);
	init = generic_hash(start_standard_board,GEESE_PLAYER);
	gInitialPosition = init;		
	gNumberOfPositions = max;
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
	int move[3];
	char origin_grid[2];
	char destination_grid[2];
	
	unHashMove(computersMove,move);
	locationToCoord(move[0],origin_coord);
	locationToCoord(move[1],destination_coord);
	
	coordToGridCoordinate(origin_coord,origin_grid);
	coordToGridCoordinate(destination_coord,destination_grid);
	
	printf("%s moved from %c%c to %c%c\n",computersName,origin_grid[0],origin_grid[1],destination_grid[0],destination_grid[1]);
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
	int player = whoseMove(pos);
	
	
	if (PRIMITIVE_DEBUG) { printf("mASALTO - Primitive() Running...\n"); }
	generic_unhash(pos, board);
	boardPieceStats(board, boardStats);
	if (PRIMITIVE_DEBUG)
	{
		printf("mASALTO - Prbin/masimitive() Returning...\n");
		if (player == GEESE_PLAYER)
		{
			printf("Goose Player    ");
		}
		else
		{
			printf("Fox Player      ");
		}
		printf("NumGeese = %d    NumWinGeese %d  ",numGeese(boardStats), numWinGeese(boardStats));
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
	generic_unhash(position, currentBoard);
	 
	printf("It is %s's turn.\n", playerName);
	printf("%s\n",GetPrediction(position,playerName,usersTurn));
	
	if (PRINTPOSITION_DEBUG) { printf("Position Hash %d\n",position);}
	
	PrintBoard(currentBoard);
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
	int player = whoseMove(position);
	
	if (GENERATEMOVES_DEBUG)
	{
		printf("mASALTO - GenerateMoves() Running...\n"); 
		printf("mASALTO - GenerateMoves() --> pos: %d  player: %d\n",position,player);
	}
	generic_unhash(position, board);
	
	if (player == FOX_PLAYER && positionGoAgain)
	{
		if (GENERATEMOVES_DEBUG) { printf("mASALTO - GenerateMoves() --> GenerateKillMoves()\n"); }
		moves = GenerateKillMoves(board,moves,positionGoAgainPiece,FOX_PLAYER);
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
	int candidate_move[3] = {location, -1, 0};
	
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
	
	int candidate_move[3] = {location, -1, 0};
	
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
					if (variant_goAgain && MoreKillMoves(board, candidate_move[1], player)) /* Go Again */
					{
						candidate_move[2] = 1;
					        moves = CreateMovelistNode((hashMove(candidate_move)),moves);
					}
					else
					{
						candidate_move[2] = 0;
						moves = CreateMovelistNode((hashMove(candidate_move)),moves); /* No Go Again */
					}
					if (GENERATEKILLMOVES_DEBUG) { printf("mASALTO - GenerateKillMoves() Move Found!\n"); }
					
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
	
	int candidate_move[3] = {location, -1, 0};
	
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
	int candidate_move[3] = {-1, -1, 0};
	int location = 0;
	
	for (location = 0; location <= 33; location ++)
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

int validMove(const char board[BOARDSIZE], int move[3],int player)
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
		printf("%8s's move [(u)ndo/([A-G][1-7] [G])] :  ", playerName);
		
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
			      printf("mASALTO - ValidTextInput() <-- %s\n",input);}
	
	if(strlen(input) == 4)
	{
		input[0] = toupper(input[0]);
		input[1] = toupper(input[1]);
		input[2] = toupper(input[2]);
		input[3] = toupper(input[3]);
		return input[0] == 'P' && input[1] == 'A' && input[2] == 'S' && input[3] == 'S';
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
		
		return  ('A' <= input[0] && input[0] <= 'E')
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
		
		return  ('A' <= input[0] && input[0] <= 'E')
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
	int move[3];

	if (CONVERTEXTINPUTTOMOVE_DEBUG) {printf("mASALTO - ConvertTextInputToMove () Start\n"); }
	
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
		printf("mASALTO - ConvertTextInputToMove() <-- GoAgain %d\n",move[2]);
	}
	
	if(strlen(input) == 5)
	{
		move[2] = 0;	
	}
	else if(strlen(input) == 7)
	{
		move[2] = 1;
	}
	else if (strlen(input) == 4)
	{
		return -1;
	}
	
	return hashMove(move);
}


/************************************************************************
**
** NAME:        PrintMove
**
** DESCRIPTION: Print the move in a nice format.
** 
** INPUTS:      MOVE *theMove         : The move to print. 
**
************************************************************************/

void PrintMove (MOVE move)
{
	int moveArray[3];
	char origin_grid[2];
	char destination_grid[2];
	int origin_coord[2];
	int destination_coord[2];
	
	unHashMove(move,moveArray);
	
	locationToCoord(moveArray[0], origin_coord);
	locationToCoord(moveArray[1], destination_coord);
	
	coordToGridCoordinate(origin_coord, origin_grid);
	coordToGridCoordinate(destination_coord, destination_grid);
	
	if (move == -1)
	{
		printf("[Pass]");
	}
	else if (moveArray[2] == 1)
	{
		printf("[%c%c %c%c G]",origin_grid[0],origin_grid[1],destination_grid[0],destination_grid[1]);
	}
	else
	{
		printf("[%c%c %c%c]",origin_grid[0],origin_grid[1],destination_grid[0],destination_grid[1]);
	}
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
	           variant_goAgain = option & 0x1; /* 0b001 */
	         variant_diagonals = option >> 1 & 0x1;             
	variant_geeseMoveBackwards = option >> 2 & 0x1;             
	                 GEESE_MIN = option >> 3;
		    GEESE_HASH_MIN = GEESE_MIN - 1;
		    init_board_hash();
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
		printf("  A   B   C   D   E \n");
		printf("1     %c - %c - %c     \n",board[0],board[1],board[2]);
		printf("      | / | \\ |     \n");
		printf("2 %c - %c - %c - %c - %c   F = Fox\n",board[3],board[4],board[5],board[6],board[7]);
		printf("  | / | \\ | / | \\ | \n");
		printf("3 %c - %c - %c - %c - %c   G = Geese\n",board[8],board[9],board[10],board[11],board[12]);
		printf("  | \\ | / | \\ | / |\n");
		printf("4 %c - %c - %c - %c - %c\n",board[13],board[14],board[15],board[16],board[17]);
		printf("      | \\ | / |    \n");
		printf("5     %c - %c - %c    \n",board[18],board[19],board[20]);
	}
	else
	{
		printf("  A   B   C   D   E \n");
		printf("1     %c - %c - %c     \n",board[0],board[1],board[2]);
		printf("      |   |   |     \n");
		printf("2 %c - %c - %c - %c - %c   F = Fox\n",board[3],board[4],board[5],board[6],board[7]);
		printf("  |   |   |   |   | \n");
		printf("3 %c - %c - %c - %c - %c   G = Geese\n",board[8],board[9],board[10],board[11],board[12]);
		printf("  |   |   |   |   |\n");
		printf("4 %c - %c - %c - %c - %c\n",board[13],board[14],board[15],board[16],board[17]);
		printf("      |   |   |    \n");
		printf("5     %c - %c - %c    \n",board[18],board[19],board[20]);
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
	char selection_command[80];
	char selection = 'Z';
	int location = -1;
	int coordinate[2] = {-1,-1};
	int boardStats[2];
	int num_fox_placed = 0;
	int validCoord=0;
		
	do
	{
		boardPieceStats(board, boardStats);
		printf("\n\n\n\n\n\nAsalto Fox Placement\n");
		printf("====================\n");
		printf("(1) Add a fox\n");
		printf("(2) Remove a fox\n");
		printf("(3) Show Board\n");
		if (numFoxes(boardStats) < 2)
		{
			printf("You must place two foxes to play the game\n");
		}
		else
		{
			printf("(4) Exit Fox Placement\n");
		}
		printf("Selection: "); scanf("%s", selection_command);
		selection = selection_command[0];
		switch (selection)
		{
			case '1':
				printf("Where do you wish to add a fox?\n");
				validCoord = 0;
				do
				{
					UserInputCoordinate(coordinate);
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
				}while(!validCoord);
				selection = 'Z';
				break;
			case '2':
				printf("Where do you wish to remove a fox?\n");
				validCoord = 0;
				do
				{
					UserInputCoordinate(coordinate);
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
					}
				}while(!validCoord);
				selection = 'Z';
				break;
			case '3':
				PrintBoard(board);
				break;
			case '4':
				return;
			default:
				printf("Invalid option. Try again\n");
				selection = 'Z';
		}
		
	} while (selection != '4');
}

void AddRemoveGeese(char board[])
{
	char selection_command[80];
	char selection = 'Z';
	int location = -1;
	int coordinate[2] = {-1,-1};
	int boardStats[2];
	int validCoord=0;
		
	do
	{
		boardPieceStats(board, boardStats);
		printf("\n\n\n\n\n\nAsalto Geese Placement\n");
		printf("======================\n");
		printf("(1) Add a Goose\n");
		printf("(2) Remove a Goose\n");
		printf("(3) Show Board\n");
		printf("(4) Exit Geese Placement\n");
		printf("Selection: "); scanf("%s",selection_command);
		selection = selection_command[0];
		switch (selection)
		{
			case '1':
				printf("Where do you wish to add a goose?\n");
				validCoord = 0;
				do
				{
					UserInputCoordinate(coordinate);
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
				}while(!validCoord);
				selection = 'Z';
				break;
			case '2':
				printf("Where do you wish to remove a goose?\n");
				validCoord = 0;
				do
				{
					UserInputCoordinate(coordinate);
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
				}while(!validCoord);
				selection = 'Z';
				break;
			case '3':
				PrintBoard(board);
				break;
			case '4':
				return;
			default:
				printf("Invalid option. Try again\n");
				selection = 'Z';
		}
		
	} while (selection != '4');
}

int hashMove(int move[3]) /* Revamped Bitshiftting Hash Function */
{
	int hashed=0;
	hashed = move[0] | move[1] << 6 | move[2] << 12;
	return hashed;
}
void unHashMove (int hashed_move, int move[3])
{
	move[0] = hashed_move & 0x3F; /* 0b111111*/
	move[1] = hashed_move >> 6 & 0x3F; /* 0b111111 */
	move[2] = hashed_move >> 12;
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

/* 1 for valid, 0 for invalid */
int validCoord(int coord[2])
{
	return (coordToLocation(coord) != -1);
}

void UserInput(char input[])
{
	scanf("%s",input);
}
void UserInputCoordinate(int coordinate[2])
{
	char input[80];
	int coord[2]={-1, -1};
	
	do
	{
		printf("Enter a Coordinate: "); UserInput(input);
		input[0] = toupper(input[0]);
		coord[1] = (int) (input[0] - 'A'); // Column Switch
		coord[0] = (int) (input[1] - '0') - 1; // Row Switch
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
		
	}while(validCoord(coord) != 1);
}
void coordToGridCoordinate(int coordinate[2], char human[2])
{
	human[0] = (char) (coordinate[1] + 'A');
	human[1] = (char) (coordinate[0] + '0' + 1);
}
void gridCoordinatetoCoord(char human[2], int coord[2])
{
	human[0] = toupper(human[0]);
	coord[1] = (int) (human[0] - 'A'); // Column Switch
	coord[0] = (int) (human[1] - '0') - 1; // Row Switch
}


void initFreeGoose()
{
	int i=0;
	for(i=0; i < 24; i++)
	{
		freeGeeseLocations[i] = -1;
	}
}
void addFreeGoose(int geeseLocation)
{
	int i=0;
	for(i=0; i < 24; i++)
	{
		if(freeGeeseLocations[i] == -1)
		{
			freeGeeseLocations[i]=geeseLocation;
			geeseLocation = -1;
			return;
		}
	}
}
void updateFreeGoose(int geeseOrigin, int geeseDestination)
{
	int i=0;
	for(i=0; i < 24; i++)
	{
		if(freeGeeseLocations[i] == geeseOrigin)
		{
			freeGeeseLocations[i]=geeseDestination;
			geeseOrigin = -1;
			geeseDestination = -1;
			return;
		}
	}
}
int freeGoose(int geeseLocation)
{
	int i=0;
	for(i=0; i < 24; i++)
	{
		if(freeGeeseLocations[i] == geeseLocation)
		{
			return 1;
		}
	}
	return 0;
}
