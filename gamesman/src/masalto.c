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

extern STRING gValueString[];

POSITION gNumberOfPositions  = 0; /* The number of total possible positions | If you are using our hash, this is given by the hash_init() function*/

POSITION gInitialPosition    = 0; /* The initial position (starting board) */
POSITION gMinimalPosition    = 0; /* */
POSITION kBadPosition        = -1; /* A position that will never be used */

STRING   kGameName           = "Asalto"; /* The name of your game */
STRING   kDBName             = ""; /* The name to store the database under */
BOOLEAN  kPartizan           = ; /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN  kDebugMenu          = ; /* TRUE while debugging */
BOOLEAN  kGameSpecificMenu   = ; /* TRUE if there is a game specific menu*/
BOOLEAN  kTieIsPossible      = ; /* TRUE if a tie is possible */
BOOLEAN  kLoopy               = ; /* TRUE if the game tree will have cycles (a rearranger style game) */
BOOLEAN  kDebugDetermineValue = ; /* TRUE while debugging */


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
#define FOX_MAX 24
#define FOX_MIN 9
#define GEESE_MAX 2
#define GEESE_MIN 2

typedef enum possibleBoardPieces
  {Blank,Fox,Geese} BoardPieces;

char *gBoardPiecesString[] = { "*", "F", "G");

/*************************************************************************
**
** Below is where you put your #define's and your global variables, structs
**
*************************************************************************/

#include "hash.h"

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
char[][] stringToBoard(char[] board);
char[] boardToString(char[][] board);
int hashMove(char[] move);
char unHashMove(int hashed_move);

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
  /* Initialize Hash Function */
  generic_hash_init(BOARDSIZE, FOX_MIN, FOX_MAX, GEESE_MIN, GEESE_MAX);
  /* CHANGE ABOVE LINE. Args are now an array. */
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
 
  do {
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
  int move[] = {0,0};
  char boardString[33];
  char origPiece = '*';
  int origin = -1;
  int destination = -1;
  
  generic_unhash(pos, boardString);
  unHashMove(theMove, move);
  
  origin = move[0];
  destination = move[1];
  origPiece = boardString[origin];
  
  /* Barebones move */
  boardString[origin] = ' ';
  boardString[destination] = origPiece;

  /* Now check if we need to remove pieces */
  
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
  /* Run Generic Hash Here */
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
  generic_unhash(pos, boardString);
  boardPieceStats(boardString, boardStats);
  if ( numGeese(boardStats) < 9) // Only will happen if the fox kill geese. It will be the geese's turn.
    {
      return (gStandardGame ? lose : win);
    }
  else if( numWinGeese(boardStats) == 9) // Foxes View
    {
      return (gStandardGame ? lose : win);
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

/* Board Abstraction under test */
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

int hashMove(int move[])
{
  return (move[0] * 32) + move[1];
}

void unHashMove(int hashed_move, int move[2])
{
  move[1] = hashed_move % 32;
  move[0] = (hashed_move - move[1]) / 32;
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
