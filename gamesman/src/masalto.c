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

int      gNumberOfPositions  = 0;

POSITION gInitialPosition    = 0;
POSITION gMinimalPosition    = 0;
POSITION kBadPosition        = -1;

STRING   kGameName           = "Asalto - Fox and Geese";
STRING   kDBName             = "";
BOOLEAN  kPartizan           = ; 
BOOLEAN  kSupportsHeuristic  = ;
BOOLEAN  kSupportsSymmetries = ;
BOOLEAN  kSupportsGraphics   = ;
BOOLEAN  kDebugMenu          = ;
BOOLEAN  kGameSpecificMenu   = ;
BOOLEAN  kTieIsPossible      = ;
BOOLEAN  kLoopy               = ;
BOOLEAN  kDebugDetermineValue = ;

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
  char currentBoard[] = generic_unhash(position);
  int *boardPointer = currentBoard;
  printf("It's %s's turn",*playerName);
  
  /* Hard coded for now */

  printf("1         %c - %c - %c            \n",*(boardPointer +  0), *(boardPointer +  1), *(boardPointer +  2) );
  printf("          | \\   / |              \n");
  printf("2         %c - %c - %c            \n",*(boardPointer +  3), *(boardPointer +  4), *(boardPointer +  5) );
  printf("          | /   \\ |              \n");
  printf("3 %c - %c - %c - %c - %c - %c - %c\n",*(boardPointer +  6), *(boardPointer +  7), *(boardPointer +  8), *(boardPointer +  9), *(boardPointer + 10), *(boardPointer + 11), *(boardPointer + 12) );
  printf("  | \\   / | \\   / | \\   /    | \n");
  printf("4 %c - %c - %c - %c - %c - %c - %c\n",*(boardPointer + 13), *(boardPointer + 14), *(boardPointer + 15), *(boardPointer + 16), *(boardPointer + 17), *(boardPointer + 18), *(boardPointer + 19) );
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

