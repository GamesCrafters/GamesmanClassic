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
BOOLEAN  kGameSpecificMenu   = FALSE; /* TRUE if there is a game specific menu*/
BOOLEAN  kTieIsPossible      = FALSE; /* TRUE if a tie is possible */
BOOLEAN  kLoopy               = TRUE; /* TRUE if the game tree will have cycles (a rearranger style game) */
BOOLEAN  kDebugDetermineValue = TRUE; /* TRUE while debugging */


STRING kHelpGraphicInterface =
"Not written yet";

STRING   kHelpTextInterface    =
""; 

STRING   kHelpOnYourTurn =
"";

STRING   kHelpStandardObjective =
"";

STRING   kHelpReverseObjective =
"";

STRING   kHelpTieOccursWhen = /* Should follow 'A Tie occurs when... */
"";

STRING   kHelpExample =
"";

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

#define BOARDSIZE 33    /* 7x7 Crosscross Board */
#define GEESE_MAX 24
#define GEESE_MIN 9
#define FOX_MAX 2
#define FOX_MIN 2
#define GEESE_PLAYER 1
#define FOX_PLAYER 2

/*************************************************************************
**
** Below is where you put your #define's and your global variables, structs
**
*************************************************************************/

int freeGeeseLocations[24];


/*************************************************************************
**
** Above is where you put your #define's and your global variables, structs
**
*************************************************************************/

/*
** Function Prototypes:
*/

/* Function prototypes here. */
void printAscii (char start, char end, int spacing);
void printNumberBar(int start, int end, int spacing);
void stringToBoard(char board[], char returnBoard[7][7]);
void boardToString(char board[7][7], char returnString[33]);
int hashMove(int move[3]);
void unHashMove(int hashed_move, int move[3]);
void boardPieceStats(char boardString[33], int stats[2]);
int numFoxes(int stats[2]);
int numGeese(int stats[2]);
int numWinGeese(int stats[2]);
int coordToLocation(int coordinates[2]);
void locationToCoord(int location, int coordinates[2]);
int validCoord(int coord[2]);
void UserInputCoordinate(int coordinate[2]);
void addFreeGoose(int geeseLocation);
void updateFreeGoose(int geeseOrigin, int geeseDestination);
int freeGoose(int geeseLocation);

/* External */
extern GENERIC_PTR	SafeMalloc ();
extern void		SafeFree ();

/*************************************************************************
**
** Here we declare the global database variables
**
**************************************************************************/

extern VALUE     *gDatabase;


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
	int hash_data[] =  {' ', 0, BOARDSIZE,
		  	        'F', FOX_MIN, FOX_MAX,
				'G', GEESE_MIN, GEESE_MAX, -1};
	/* Initialize Hash Function */
	generic_hash_init(BOARDSIZE, hash_data, 0);
}

/************************************************************************
**
** NAME:        DebugMenu
**
** DESCRIPTION: Menu used to debub internal problems. Does nothing if
**              kDebugMenu == FALSE
** 
************************************************************************/

void DebugMenu ()
{
	char GetMyChar();
 
	printf("The Debug menu is not done.");
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
	printf("No Game Specific Menu Yet\n");
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

POSITION DoMove (thePosition, theMove)
	POSITION thePosition;
	MOVE theMove;
{
  int move[] = {0,0,0};
  char boardString[33];
  char origPiece = '*';
  int origin = -1;
  int destination = -1;
  int coord_origin[2];
  int coord_destination[2];
  int goAgain = 0;
  int next_player = 0;
  
  generic_unhash(thePosition, boardString);
  unHashMove(theMove, move);
  
  origin = move[0];
  destination = move[1];
  goAgain = move[2];
  origPiece = boardString[origin];
  
  /* Barebones move */
  boardString[origin] = ' ';
  boardString[destination] = origPiece;
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
	boardString[del_loc] = ' ';
  }
  if (goAgain == 1)
  {
  	next_player = whoseMove(thePosition);
  }
  else
  {
  	next_player = (whoseMove(thePosition) == GEESE_PLAYER) ? FOX_PLAYER : GEESE_PLAYER;
  }
  return generic_hash(boardString,next_player);
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
	char start_standard_board[]={      'G','G','G',
	                                   'G','G','G',
			           'G','G','G','G','G','G','G',
				   'G','G','G','G','G','G','G',
				   'G','G',' ',' ',' ','G','G',
				           ' ',' ',' ',
					   ' ',' ',' '};
        char *boardPointer=start_standard_board;
	int num_fox_placed=0;
	int coordinate[2];
	int location = -1;
	printf("Fox Player: Please place two foxes in the blank positions\n");
	do
	{
		printf("  A   B   C   D   E   F   G       \n");
  		printf("1         %c - %c - %c            \n",*(boardPointer +  0), *(boardPointer +  1), *(boardPointer +  2) );
		printf("          | \\   / |              \n");
		printf("2         %c - %c - %c            \n",*(boardPointer +  3), *(boardPointer +  4), *(boardPointer +  5) );
		printf("          | /   \\ |              \n");
		printf("3 %c - %c - %c - %c - %c - %c - %c    F = Fox \n",*(boardPointer +  6), *(boardPointer +  7), *(boardPointer +  8), *(boardPointer +  9), *(boardPointer + 10), *(boardPointer + 11), *(boardPointer + 12) );
		printf("  | \\   / | \\   / | \\   /    | \n");
		printf("4 %c - %c - %c - %c - %c - %c - %c    G = Geese\n",*(boardPointer + 13), *(boardPointer + 14), *(boardPointer + 15), *(boardPointer + 16), *(boardPointer + 17), *(boardPointer + 18), *(boardPointer + 19) );
		printf("  | /   \\ | /   \\ | /   \\    | \n");
		printf("5 %c - %c - %c - %c - %c - %c - %c\n",*(boardPointer + 20), *(boardPointer + 21), *(boardPointer + 22), *(boardPointer + 23), *(boardPointer + 24), *(boardPointer + 25), *(boardPointer + 26) );
		printf("          | \\   / |              \n");
		printf("6         %c - %c - %c            \n",*(boardPointer + 27), *(boardPointer + 28), *(boardPointer + 29));
		printf("          | /   \\ |              \n");
		printf("7         %c - %c - %c            \n",*(boardPointer + 30), *(boardPointer + 31), *(boardPointer + 32));
		printf("\n\n Please Enter in A Coordinate [Example C4]: ");
		UserInputCoordinate(coordinate);
		location = coordtoLocation(coordinate);
		if (start_standard_board[location] == ' ')
		{
			start_standard_board[location] = 'F';
			num_fox_placed++;
		}
		else
		{
			printf("There is a goose there. Please try again. \n");
		}
	}while(num_fox_placed < 2);
	return generic_hash(start_standard_board, GEESE_PLAYER);
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

void PrintComputersMove(computersMove, computersName)
	MOVE computersMove;
	STRING computersName;
{
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

VALUE Primitive (pos)
	POSITION pos;
{
  int boardStats[]={0,0,0};
  char boardString[33];
  int currentTurn = whoseTurn(pos);
  generic_unhash(pos, boardString);
  boardPieceStats(boardString, boardStats);
  if ( numGeese(boardStats) < 9 && currentTurn == GEESE_PLAYER) // Only will happen if the fox kill geese. It will be the geese's turn.
    {
        return (gStandardGame ? lose : win);
    }
  else if(numWinGeese(boardStats) == 9 && currentTurn == FOX_PLAYER) // Foxes View
    {
        return (gStandardGame ? lose : win);
    }
  else if(currentTurn == FOX_PLAYER)
    {
    	BOOLEAN CantMove();
	return CantMove(pos) ? lose : undecided;
    }
   else
    {
    	return undecided;
    }
  // Need to add Geese No Move Win for Fox.
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

void PrintPosition (position, playerName, usersTurn)
	POSITION position;
	STRING playerName;
	BOOLEAN usersTurn;
{
  char currentBoard[33];
  char *boardPointer = currentBoard;
  generic_unhash(position, currentBoard);
  
  /* Hard coded for now */
  printf(" It is %s's turn", playerName);
  printf("  A   B   C   D   E   F   G   H   \n");
  printf("1         %c - %c - %c            \n",*(boardPointer +  0), *(boardPointer +  1), *(boardPointer +  2) );
  printf("          | \\   / |              \n");
  printf("2         %c - %c - %c            \n",*(boardPointer +  3), *(boardPointer +  4), *(boardPointer +  5) );
  printf("          | /   \\ |              \n");
  printf("3 %c - %c - %c - %c - %c - %c - %c    F = Fox \n",*(boardPointer +  6), *(boardPointer +  7), *(boardPointer +  8), *(boardPointer +  9), *(boardPointer + 10), *(boardPointer + 11), *(boardPointer + 12) );
  printf("  | \\   / | \\   / | \\   /    | \n");
  printf("4 %c - %c - %c - %c - %c - %c - %c    G = Geese\n",*(boardPointer + 13), *(boardPointer + 14), *(boardPointer + 15), *(boardPointer + 16), *(boardPointer + 17), *(boardPointer + 18), *(boardPointer + 19) );
  printf("  | /   \\ | /   \\ | /   \\    | \n");
  printf("5 %c - %c - %c - %c - %c - %c - %c\n",*(boardPointer + 20), *(boardPointer + 21), *(boardPointer + 22), *(boardPointer + 23), *(boardPointer + 24), *(boardPointer + 25), *(boardPointer + 26) );
  printf("          | \\   / |              \n");
  printf("6         %c - %c - %c            \n",*(boardPointer + 27), *(boardPointer + 28), *(boardPointer + 29));
  printf("          | /   \\ |              \n");
  printf("7         %c - %c - %c            \n",*(boardPointer + 30), *(boardPointer + 31), *(boardPointer + 32));
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

MOVELIST *GenerateMoves (position)
         POSITION position;
{
  MOVELIST *CreateMovelistNode();
  MOVELIST *head = NULL;
  char boardString[33];
  int i=0;
  int scratch_coord[2];
  int candidate_destination_coord[2]={-1, -1};
  int candidate_destination_loc=-1;
  int candidate_move[3]={-1, -1, -1};
  int delta_row=-5;  
  int delta_col=-5;
  int currentTurn=whoseTurn(position);
  generic_unhash(position, boardString);

  /* Check Move */
	if(currentTurn = FOX_PLAYER)
	{
		/* Scan through all pieces for foxes */
		for (i=0; i < 33; i++)
		{
			/* Do we have a fox ?*/
			if (boardString[i] == 'F') /* Yup */
			{
				locationToCoord(i, scratch_coord);
				/* Scan all move directions */
				for (delta_row = -1; delta_row <= 1; delta_row ++)
				{
					for (delta_col = -1; delta_col <= 1; delta_col ++)
					{
						if (delta_row != 0 && delta_col != 0 && i%2 == 0) /* Gotta Move Somewhere. Can Move Diagonally*/
						{
							candidate_destination_coord[0] = scratch_coord[0] + delta_row;
							candidate_destination_coord[1] = scratch_coord[1] + delta_col;
							if (validCoord(candidate_destination_coord) == 1)
							{
								candidate_destination_loc = coordToLocation(candidate_destination_coord);
								if (boardString[candidate_destination_loc] == ' ')
								{
									candidate_move[0] = i;
									candidate_move[1] = candidate_destination_loc;
									candidate_move[2] = NoKillMoves(doMove(position,candidate_move));
									head = CreateMovelistNode(hashMove(candidate_move),head);
								}
								else if (boardString[candidate_destination_loc] == 'G') /* Maybe we can jump */
								{
									candidate_destination_coord[0] = scratch_coord[0] + delta_row * 2; /* Next Space Blank */
									candidate_destination_coord[1] = scratch_coord[0] + delta_col * 2; /* Next Space Blank */
									candidate_destination_loc = coordToLocation(candidate_destination_coord);
									if (validCoord(candidate_destination_coord) == 1 && boardString[candidate_destination_loc] == ' ') /* Yes, we can */
									{
										candidate_move[0] = i;
										candidate_move[1] = candidate_destination_loc;
										candidate_move[2] = 0;
										head = CreateMovelistNode(hashMove(candidate_move),head);
									}
								}
							}
						}
						else if(delta_row != 0 && delta_col != 0 && delta_row != delta_col) /* Gotta Move Somewhere. Can't Move Diagonally*/
						{
							candidate_destination_coord[0] = scratch_coord[0] + delta_row;
							candidate_destination_coord[1] = scratch_coord[1] + delta_col;
							if (validCoord(candidate_destination_coord) == 1)
							{
								candidate_destination_loc = coordToLocation(candidate_destination_coord);
								if (boardString[candidate_destination_loc] == ' ')
								{
									candidate_move[0] = i;
									candidate_move[1] = candidate_destination_loc;
									candidate_move[2] = NoKillMoves(doMove(position,candidate_move));
									head = CreateMovelistNode(hashMove(candidate_move),head);
								}
								else if (boardString[candidate_destination_loc] == 'G') /* Maybe we can jump */
								{
									candidate_destination_coord[0] = scratch_coord[0] + delta_row * 2; /* Next Space Blank */
									candidate_destination_coord[1] = scratch_coord[0] + delta_col * 2; /* Next Space Blank */
									candidate_destination_loc = coordToLocation(candidate_destination_coord);
									if (validCoord(candidate_destination_coord) == 1 && boardString[candidate_destination_loc] == ' ') /* Yes, we can */
									{
										candidate_move[0] = i;
										candidate_move[1] = candidate_destination_loc;
										candidate_move[2] = 0;
										head = CreateMovelistNode(hashMove(candidate_move),head);
									}
								}
							}
						}
					}					
				}
			}
		}
	}
	else if (currentTurn = GEESE_PLAYER)
	{
		/* Scan through all pieces for geese */
		for (i=0; i < 33; i++)
		{
			/* Do we have a goose?*/
			if (boardString[i] == 'G') /* Yup */
			{
				locationToCoord(i, scratch_coord);
				/* Free Goose? */
				if (freeGoose(i))
				{
					for (delta_row = -1; delta_row <= 1; delta_row ++)/* Scan all move directions */
					{
						for (delta_col = -1; delta_col <= 1; delta_col ++)
						{
							candidate_destination_coord[0] = scratch_coord[0] + delta_row;
							candidate_destination_coord[1] = scratch_coord[1] + delta_col;
							if (delta_row != 0 && delta_col != 0 && i%2 == 0) /* Gotta Move Somewhere. Can Move Diagonally*/
							{
								if (validCoord(candidate_destination_coord) == 1)
								{
									if (boardString[candidate_destination_loc] == ' ')
									{
										candidate_destination_loc = coordToLocation(candidate_destination_coord);
										candidate_move[0] = i;
										candidate_move[1] = candidate_destination_loc;
										candidate_move[2] = 0;
										head = CreateMovelistNode(hashMove(candidate_move),head);
									}
								}
							}
							else if(delta_row != 0 && delta_col != 0 && delta_row != delta_col) /* Gotta Move Somewhere. Can't Move Diagonally*/
							{
								if (validCoord(candidate_destination_coord) == 1)
								{
									if (boardString[candidate_destination_loc] == ' ')
									{
										candidate_destination_loc = coordToLocation(candidate_destination_coord);
										candidate_move[0] = i;
										candidate_move[1] = candidate_destination_loc;
										candidate_move[2] = 0;
										head = CreateMovelistNode(hashMove(candidate_move),head);
									}
								}
							}							
						}
					}
				}
				else
				{
					for (delta_row = 0; delta_row <= 1; delta_row ++)/* Scan all move directions */
					{
						for (delta_col = 0; delta_col <= 1; delta_col ++)
						{
							candidate_destination_coord[0] = scratch_coord[0] + delta_row;
							candidate_destination_coord[1] = scratch_coord[1] + delta_col;
							if (delta_row != 0 && delta_col != 0 && i%2 == 0) /* Gotta Move Somewhere. Can Move Diagonally*/
							{
								if (validCoord(candidate_destination_coord) == 1)
								{
									if (boardString[candidate_destination_loc] == ' ')
									{
										candidate_destination_loc = coordToLocation(candidate_destination_coord);
										candidate_move[0] = i;
										candidate_move[1] = candidate_destination_loc;
										candidate_move[2] = 0;
										head = CreateMovelistNode(hashMove(candidate_move),head);
									}
								}
							}
							else if(delta_row != 0 && delta_col != 0 && delta_row != delta_col) /* Gotta Move Somewhere. Can't Move Diagonally*/
							{
								if (validCoord(candidate_destination_coord) == 1)
								{
									if (boardString[candidate_destination_loc] == ' ')
									{
										candidate_destination_loc = coordToLocation(candidate_destination_coord);
										candidate_move[0] = i;
										candidate_move[1] = candidate_destination_loc;
										candidate_move[2] = 0;
										head = CreateMovelistNode(hashMove(candidate_move),head);
									}
								}
							}
						}
					}
				}
			}
		}
	}
  return head;
}

BOOLEAN CantMove(position)
     POSITION position;
{
  MOVELIST *ptr, *GenerateMoves();
  BOOLEAN cantMove;

  ptr = GenerateMoves(position);
  cantMove = (ptr == NULL);
  FreeMoveList(ptr);
  return(cantMove);
}

BOOLEAN NoKillMoves(position)
{
	MOVELIST *ptr, *GenerateKillMoves();
	BOOLEAN nokillMove;
	
	ptr = GenerateKillMoves(position);
	nokillMove = (ptr == NULL);
	FreeMoveList(ptr);
	return (nokillMove);
}

MOVELIST *GenerateKillMoves (position,location)
         POSITION position;
{
  MOVELIST *CreateMovelistNode();
  MOVELIST *head = NULL;
  char boardString[33];
  int i=location;
  int scratch_coord[2];
  int candidate_destination_coord[2]={-1, -1};
  int candidate_destination_loc=-1;
  int candidate_move[3]={-1, -1, -1};
  int delta_row=-5;  
  int delta_col=-5;
  int currentTurn=whoseTurn(position);
  generic_unhash(position, boardString);

  /* Check Move */
	if(currentTurn = FOX_PLAYER && boardString[i] == 'F')
	{
		locationToCoord(i, scratch_coord);
		/* Scan all move directions */
		for (delta_row = -1; delta_row <= 1; delta_row ++)
		{
			for (delta_col = -1; delta_col <= 1; delta_col ++)
			{
				if (delta_row != 0 && delta_col != 0 && i%2 == 0) /* Gotta Move Somewhere. Can Move Diagonally*/
				{
					candidate_destination_coord[0] = scratch_coord[0] + delta_row;
					candidate_destination_coord[1] = scratch_coord[1] + delta_col;
					candidate_destination_loc = coordToLocation(candidate_destination_coord);
					if (validCoord(candidate_destination_coord) == 1 && boardString[candidate_destination_loc] == 'G')
					{
						candidate_destination_coord[0] = scratch_coord[0] + delta_row * 2; /* Next Space Blank */
						candidate_destination_coord[1] = scratch_coord[1] + delta_col * 2; /* Next Space Blank */
						candidate_destination_loc = coordToLocation(candidate_destination_coord);
						if (validCoord(candidate_destination_coord) == 1 && boardString[candidate_destination_loc] == ' ') /* Yes, we can */
						{
							candidate_move[0] = i;
							candidate_move[1] = candidate_destination_loc;
							candidate_move[2] = -1;
							candidate_move[2] = NoKillMoves(doMove(position,candidate_move));
							head = CreateMovelistNode(hashMove(candidate_move),head);
						}
					}
				}
				else if(delta_row != 0 && delta_col != 0 && delta_row != delta_col) /* Gotta Move Somewhere. Can Move Diagonally*/
				{
					candidate_destination_coord[0] = scratch_coord[0] + delta_row;
					candidate_destination_coord[1] = scratch_coord[1] + delta_col;
					candidate_destination_loc = coordToLocation(candidate_destination_coord);
					if (validCoord(candidate_destination_coord) == 1 && boardString[candidate_destination_loc] == 'G')
					{
						candidate_destination_coord[0] = scratch_coord[0] + delta_row * 2; /* Next Space Blank */
						candidate_destination_coord[1] = scratch_coord[1] + delta_col * 2; /* Next Space Blank */
						candidate_destination_loc = coordToLocation(candidate_destination_coord);
						if (validCoord(candidate_destination_coord) == 1 && boardString[candidate_destination_loc] == ' ') /* Yes, we can */
						{
							candidate_move[0] = i;
							candidate_move[1] = candidate_destination_loc;
							candidate_move[2] = -1;
							candidate_move[2] = NoKillMoves(doMove(position,candidate_move));
							head = CreateMovelistNode(hashMove(candidate_move),head);
						}
					}
				}
			}
		}
		
	}
  return head;
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
** CALLS:       ValidMove(MOVE, POSITION)
**              BOOLEAN PrintPossibleMoves(POSITION) ...Always True!
**
************************************************************************/

USERINPUT GetAndPrintPlayersMove (thePosition, theMove, playerName)
	POSITION thePosition;
	MOVE *theMove;
	STRING playerName;
{
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

BOOLEAN ValidTextInput (input)
	STRING input;
{
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

MOVE ConvertTextInputToMove (input)
	STRING input;
{
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

void PrintMove (move)
	MOVE move;
{
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
	return 0;
}


/************************************************************************
**
** NAME:        getOption
**
** DESCRIPTION: A hash function to keep track of all the game variants.
**				Should return a different number for each set of
**				variants.
**
** OUTPUTS:     int : the number representation of the options.
**
************************************************************************/

int getOption()
{
	return 0;
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
}


/************************************************************************
**
** NAME:        GameSpecificTclInit
**
** DESCRIPTION: NO IDEA, BUT AS FAR AS I CAN TELL IS IN EVERY GAME
**
************************************************************************/

int GameSpecificTclInit (interp, mainWindow) 
	Tcl_Interp* interp;
	Tk_Window mainWindow;
{
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

/* Prints an ASCII Header Bar */
void printAscii (char start, char end, int spacing)
{
	for ( ; start <= end; start++)
	{
		printf("%c", start);
		int i;
		for(i=0; i < spacing; i++)
		{
			printf(" ");
		}
	}
	printf("\n Done \n");	
}

/* Prints a Number Bar */
void printNumberBar(int start, int end, int spacing)
{
    for ( ; start <= end ; start++)
    {
      printf("%d", start);
      int i;
      for(i=0; i < spacing; i++)
	{
	  printf(" ");
	}
    }
    printf("\n");
}

/* Board Abstraction under test. Probably won't use*/
void stringToBoard(char board[], char returnBoard[7][7])
{
  int i = 0;
  int j = 0;
  /* Initialize to blank */
  for (i=0; i < 7; i++)
    {
      for (j=0; j < 7; j++)
	{
	  returnBoard[i][j]='*';
	}
    }
  /* Top board 0-5 */
  for (i=0; i <= 1; i++)
    {
      for (j=2; j<= 4; j++)
	{
	  returnBoard[i][j]=board[3*i+(j-2)];
	}
    }
  /* Mid Board 6-26*/
  for (i=2; i <= 4; i++)
    {
      for (j=0; j<= 6; j++)
	{
	  returnBoard[i][j]=board[6+7*(i-2)+j];
	}
    }
  for (i=5; i <= 6; i++)
    {
      for (j=2; j<= 4; j++)
	{
	  returnBoard[i][j]=board[27+3*(i-5)+(j-2)];
	}
    }
}

/* BUG! returnString contains random characters at the end for some odd reason... */
void boardToString(char board[7][7], char returnString[33])
{
  int index=0;
  int i=0;
  int j=0;
  
  for (i=0; i < 7; i++)
    {
      for (j=0; j < 7; j++)
	{
	  if (board[i][j] != '*')
	    {
	      returnString[index] = board[i][j];
	      index++;
	    }
	}
    }
}

int hash_move(int move[3]) /* Revamped Bitshiftting Hash Function */
{
	int hashed=0;
	hashed = move[0] | move[1] << 6 | move[2] << 12;
	return hashed;
}
void unhash_move (int hashed_move, int move[3])
{
	move[0] = hashed_move & 0x3F; /* 0b111111*/
	move[1] = hashed_move >> 6 & 0x3F; /* 0b111111 */
	move[2] = hashed_move >> 12;
}

void boardPieceStats(char boardString[33], int stats[2])
{
  /* stats[0] = Number of Foxes , stats[1] = Number of Geese, stats[2] = Number of Geese in Winning Box */
  int i=0;
  stats[0] = 0; stats[1] = 0; stats[2] - 0;
  for (i=0; i < 33; i++)
    {
      if(boardString[i]== 'F')
	{
	  stats[0]++;
	}
      else if (boardString[i]== 'G')
	{
	  stats[1]++;
	  /* 22-24 and 27-32 Winning Zones */
	  if( (22 <= i && i <= 24) || (27 <= i && i <= 32))
	    {
	      stats[2]++;
	    }
	}
    }
}

int numFoxes(int stats[2])
{
  return stats[0];
}
int numGeese(int stats[2])
{
  return stats[1];
}
int numWinGeese(int stats[2])
{
  return stats[2];
}

int coordToLocation(int coordinates[2])
{
  int row = coordinates[0];
  int col = coordinates[1];
  if (0 <= row && row <= 1)
    {
      return 3 * row + (col - 2);
    }
  else if (2 <= row && row <= 4)
    {
      return 6 + (7 * (row - 2)) + col;
    }
  else if (5 <= row && row <= 6)
    {
      return 27 + 3 * (row - 5) + (col - 2);
    }
  else
    {
      return -1;
    }
}

void locationToCoord(int location, int coordinates[2])
{
  if (0 <= location && location <= 5)
    {
      coordinates[1] = 2 + location % 3;
      coordinates[0] = (location - location % 3) / 3;
    }
  else if (6 <= location && location <= 26)
    {
      /* Translate Two Down */
      location -= 6;
      coordinates[1] = location % 7;
      coordinates[0] = 2 + (location - coordinates[1]) / 7;
    }
  else if (27 <= location && location <= 32)
    {
      /* Translate 5 Down */
      location -= 27;
      coordinates[1] = 2 + location % 3;
      coordinates[0] = 5 + (location - location % 3) / 3;
    }
}

/* 1 for valid, 0 for invalid */
int validCoord(int coord[2])
{
  int row = coord[0];
  int col = coord[1];
  if (row < 0 || 6 < row)
    {
      return 0;
    }
  else if (0 <= row && row <= 1)
    {
      return (2 <= col && col <= 4) ? 1 : 0;
    }
  else if (2 <= row && row <= 4)
    {
      return (0 <= col && col <= 6) ? 1 : 0;
    }
  else if (5 <= row && row <= 6)
    {
      return (2 <= col && col <= 4) ? 1 : 0;
    }
  else
    {
      return 0;
    }
}

void GetCoordinate(int coordinate[2])
{
	char inString[80];
	int coord[2];
	int valid = 0;
	do{
		printf("Enter a coordinate [Example: D3]: "); scanf("%s",inString);
		inString[0]=toupper(inString[0]);
		coord[1] = (int) (inString[0] - 'A'); // Col
		coord[0] = (int) (inString[1] - '0') - 1; // Row
		if(validCoord(coord) == 1)
		{
			valid = 1;
		}
		else
		{
			printf("Invalid Coordinate. Please try again: \n");
		}
	}while(valid != 1);
	coordinate[0] = coord[0];
	coordinate[1] = coord[1];
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
