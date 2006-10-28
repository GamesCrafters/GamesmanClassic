/************************************************************************
**
** NAME:        mpentago.c
**
** DESCRIPTION: Pentago
**
** AUTHOR:      Jun Kang Chin
**              David Wu
**
** DATE:        Started: 2006-09-24
**
** UPDATE HIST: 
**
** LAST CHANGE: 2006/10/28 Tweaked around with Printing and Board representation
**              $Id$
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

STRING   kGameName            = "Pentago"; /* The name of your game */
STRING   kAuthorName          = "Jun Kang Chin and David Wu"; /* Your name(s) */
STRING   kDBName              = "pentago"; /* The name to store the database under */

BOOLEAN  kPartizan            = FALSE ; /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN  kGameSpecificMenu    = FALSE ; /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN  kTieIsPossible       = TRUE ; /* TRUE if a tie is possible. FALSE if it is impossible.*/
BOOLEAN  kLoopy               = FALSE ; /* TRUE if the game tree will have cycles (a rearranger style game). FALSE if it does not.*/

BOOLEAN  kDebugMenu           = TRUE ; /* TRUE only when debugging. FALSE when on release. */
BOOLEAN  kDebugDetermineValue = TRUE ; /* TRUE only when debugging. FALSE when on release. */

POSITION gNumberOfPositions   =  430467321; /* The number of total possible positions | If you are using our hash, this is given by the hash_init() function*/
POSITION gInitialPosition     =  0; /* The initial hashed position for your starting board */
POSITION kBadPosition         = -1; /* A position that will never be used */

void*	 gGameSpecificTclInit = NULL;

/**
 * Help strings that are pretty self-explanatory
 * Strings than span more than one line should have backslashes (\) at the end of the line.
 * These help strings should be updated and dynamically changed using 
 * InitializeHelpStrings() 
 **/

STRING   kHelpGraphicInterface =
"Help strings not initialized!";

STRING   kHelpTextInterface =
"Help strings not initialized!"; 

STRING   kHelpOnYourTurn =
"Help strings not initialized!";

STRING   kHelpStandardObjective =
"Help strings not initialized!";

STRING   kHelpReverseObjective =
"Help strings not initialized!";

STRING   kHelpTieOccursWhen =
"Help strings not initialized!";

STRING   kHelpExample =
"Help strings not initialized!";



/*************************************************************************
**
** #defines and structs
**
**************************************************************************/
#define DEBUG 1

typedef enum {
  EMPTY, O, X
}possibleBoardPieces;

/*************************************************************************
**
** Global Variables
**
*************************************************************************/
int ROWSIZE = 4; /* This should be an even number greater than 2 */
int NthInARow = 3; /*               1 < n < ROWSIZE                */

int BOARDSIZE;  /* ROWSIZE * ROWSIZE */

//int POSITIONSPLAYED = 0; /* This will be used to determine if there is a draw */

//psh, who needs a board when everything is unhashed!?
char* BOARD;  /* Board Representation is a 1D char array */


/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/

/* External */
extern GENERIC_PTR	SafeMalloc ();
extern void		SafeFree ();

STRING                  MoveToString(MOVE move);

/* Mine */
//BOOLEAN isOnBoard(POSITION position);
BOOLEAN nInARow(POSITION position);
void rotate();
BOOLEAN AllFilledIn();
void InitializeBoard();
int getRow(MOVE);
int getColumn(MOVE);
int getBoardToRotate(MOVE);
BOOLEAN isRotateClockwise(MOVE);

/************************************************************************
**
** NAME:        InitializeGame
**
** DESCRIPTION: Prepares the game for execution.
**              Initializes required variables.
** 
************************************************************************/

void InitializeGame ()
{
  if (DEBUG) {
    printf("InitializeGame Start \n");
  }
    //    InitializeHelpStrings(); 
  BOARDSIZE = ROWSIZE * ROWSIZE;
  BOARD = (char *) SafeMalloc(BOARDSIZE * sizeof(char));
  InitializeBoard();

  /*
  int i;
  for (i = 0; i < BOARDSIZE; i++) {
    printf(" %d ", BOARD[i]);
    if ((i + 1) % ROWSIZE == 0) {
      printf("\n");
    }
  }
  */

  if (DEBUG) {
    printf("InitializeGame End \n");
  }
}


/************************************************************************
**
** NAME:        InitializeHelpStrings
**
** DESCRIPTION: Sets up the help strings based on chosen game options.
**
** NOTES:       Should be called whenever the game options are changed.
**              (e.g., InitializeGame() and GameSpecificMenu())
**
************************************************************************/
void InitializeHelpStrings ()
{
  if (DEBUG) {
    printf("InitializingHelpStrings Start\n");
  }

kHelpGraphicInterface =
    "";

kHelpTextInterface =
   ""; 

kHelpOnYourTurn =
  "";

kHelpStandardObjective =
  "";

kHelpReverseObjective =
  ""; 

kHelpTieOccursWhen = 
  "A tie occurs when ...";

kHelpExample = 
  "";

    gMoveToStringFunPtr = &MoveToString;

  if (DEBUG) {
    printf("InitializingHelpStrings End\n");
  }
}


/************************************************************************
**
** NAME:        GenerateMoves
**
** DESCRIPTION: Creates a linked list of every move that can be reached
**              from this position. Returns a pointer to the head of the
**              linked list.
** 
** INPUTS:      POSITION position : Current position to generate moves
**
** OUTPUTS:     (MOVELIST *)      : A pointer to the first item of
**                                  the linked list of generated moves
**
** CALLS:       MOVELIST *CreateMovelistNode();
**
************************************************************************/

MOVELIST *GenerateMoves (POSITION position)
{
  if (DEBUG) {
    printf("GenerateMoves Start..\n");
  }

  MOVELIST *moves = NULL;
  generic_hash_unhash(position, BOARD);

  MOVE move = 0;
  int col, row, board, i;
  
  for (i = 0; i < BOARDSIZE; i++) {
    if (BOARD[i] == EMPTY) {
      col = (i % ROWSIZE) * 10;
      row = i / ROWSIZE;
      for (board = 0; board <= 4; board++) { //used to include all boards in possible rotations	  
	move = col + row + (100 * board);
	moves = CreateMovelistNode(move, moves);
	move *= -1;
	moves = CreateMovelistNode(move, moves);
      }
    }
  }
  /* Use CreateMovelistNode(move, next) to 'cons' together a linked list */    

  if (DEBUG) {
    printf("GenerateMoves End..\n");
  }
  return moves;
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
  if (DEBUG) {
    printf("DoMove Start..\n");
  }
  //  int MallocSize = BOARDSIZE + 1;
  //  char * UnhashedBoard = (char *) SafeMalloc(MallocSize * sizeof(char));
  generic_hash_unhash(position, BOARD);
  int turn = generic_hash_turn(position);
  int rotateCW;

  if (isRotateClockwise(move)) {
    rotateCW = TRUE;
    move *= -1;
  } else {
    rotateCW = FALSE;
  }
  int rotateBoard = getBoardToRotate(move);
  int row = getRow(move);
  int col = getColumn(move);
  
  //need to find a good algorithm for rotating that stupid board  

  //need to parse move

  int newMove = ((row * ROWSIZE) + col);
  //  BOARD[move] = turn ==PlayerX? 'x'='o';   //check to see if spot is free

  if (DEBUG) {
    printf("DoMove End..\n");
  }
  return generic_hash_hash(BOARD, turn = 'x' ? 'o' :'x');
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
  if (DEBUG) {
    printf("Primitive Start..\n");
  }
  //waiting, waiting..
    return undecided;
  if (DEBUG) {
    printf("Primitive End..\n");
  }
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
  if (DEBUG) {
    printf("PrintPosition Start..\n");
  }

  char * currBOARD = (char *) SafeMalloc(BOARDSIZE * sizeof(char));


  printf("hash start\n");
  generic_hash_unhash(position, currBOARD);
  printf("hash end\n");
  
  int i;
  int j;

 printf("+");
  for (i = 0; i < ROWSIZE; i++) {
    printf("---+");
  }
  printf("   ");
  printf("+");
  for (i = 0; i < ROWSIZE; i++) {
    printf("---+");
  }
  printf("\n");


  for (i = 0; i < BOARDSIZE; i++) {
    if ((i % ROWSIZE) == 0) {
      printf("|");
    }
    if ((i + 1) % ROWSIZE != 0 && (i % ROWSIZE) != ((ROWSIZE / 2) - 1)) {      
      printf(" %c  ", currBOARD[i]);
    } else {
      printf(" %c |", currBOARD[i]);
    }
  
  
    if ((i + 1) % ROWSIZE == 0) {
      printf("   ");
      int k;
      for (k = 0; k < ROWSIZE; k++) {
	if (k == 0) {
	  printf("|    ");
	} else if (k == ROWSIZE/4 || k == ROWSIZE-1){
	  printf("   |");
	} else {
	  printf("    ");
	}     
      }
      printf("\n");
      if (((i/ROWSIZE) != (ROWSIZE - 1)) && (i/ROWSIZE != (ROWSIZE/2 - 1))) {
	for (j = 0; j < (ROWSIZE / 2); j++) {
	  if (j == 0) {
	    printf("|       |");
	  } else {
	    printf("       |");
	    printf("   ");
	    if (i/ROWSIZE == 0) {
	      printf("|   1   |   2   |\n");
	    } else{
	      printf("|   3   |   4   |\n");
	    }
	  }
	}
      } else if (i/ROWSIZE == (ROWSIZE/2 - 1)) {
	for (j = 0; j < ROWSIZE; j++) {
	  if (j == 0) {
	    printf("+---+");
	  } else if (j != (ROWSIZE - 1)) {
	    printf("---+");
	  } else {
	    printf("---+   ");
	    for (k = 0; k < ROWSIZE; k++) {
	      if (k == 0) {
		printf("+---+");
	      } else {
		printf("---+");
	      }
	    }
	    printf("\n");
	  }
	}
      }
    }
  }

  printf("+");
  for (i = 0; i < ROWSIZE; i++) {
    printf("---+");
  }
  printf("   ");
  printf("+");
  for (i = 0; i < ROWSIZE; i++) {
    printf("---+");
  }
  printf("\n");
  
  if (DEBUG) {
    printf("PrintPosition End..\n");
  }
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
  printf("PrintComputersMove Start..\n");    

  printf("PrintComputersMove End..\n");    
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
  if (DEBUG) {
    printf("PrintMove Start..\n");    
  }

    STRING str = MoveToString( move );
    printf( "%s", str );
    SafeFree( str );

  if (DEBUG) {
    printf("PrintMove End..\n");    
  }
}


/************************************************************************
**
** NAME:        MoveToString
**
** DESCRIPTION: Returns the move as a STRING
** 
** INPUTS:      MOVE *move         : The move to put into a string.
**
************************************************************************/

STRING MoveToString (MOVE move)
{
  printf("MoveToString Start..\n");    
    return NULL;
  printf("MoveToString End..\n");    
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
	printf("%8s's move [(undo)/(MOVE FORMAT)] : ", playersName);
	
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
  if (DEBUG) {
    printf("ValidTextInput Start..\n");    
  }

  if (DEBUG) {
    printf("ValidTextInput End..\n");    
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

MOVE ConvertTextInputToMove (STRING input)
{
  if (DEBUG) {
    printf("ConvertTextInputToMove Start..\n");    
  }


  if (DEBUG) {
    printf("ConvertTextInputToMove End..\n");    
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
  InitializeHelpStrings();
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

/************************************************************************
**
** NAME:        InitializeBoard
**
** DESCRIPTION: Fill the global variable BOARD with EMPTY characters.
**            
**            
** 
** INPUTS:      void
**
** OUTPUTS:     void
**
************************************************************************/
void InitializeBoard() {
  if (DEBUG) {
    printf("InitializeBoard Start..\n");    
  }

  int i;
  for (i = 0; i < BOARDSIZE; i++) {
      BOARD[i] = X;
  }


  if (DEBUG) {
    printf("InitializeBoard End..\n");    
  }
}


/************************************************************************
**
** NAME:        isOnBoard
**
** DESCRIPTION: determines whether or not a position is on board
** 
** 
** 
** INPUTS:      POSITION position.  The position on BOARD being checked
**
** OUTPUTS:     BOOLEAN
**
************************************************************************/
/*
BOOLEAN isOnBoard(POSITION position) {
  if (position >= 0 && position < BOARDSIZE) {
    return TRUE;
  } else {
    return FALSE;
  }
}
*/

/*
void 1Dto2D(char ** to, char * from) {
  int i = 0;
  int j = 0;
  int k = 0;
  for (k = 0; k < BOARDSIZE ; k++) {
    if (i == ROWSIZE) {
      i = 0;
      j++;
    }
    to[i][j] = from[k];
  }
}
*/

/* Parses move statements
moves are represented as follows:
hundreds = rotating board
tens = column
ones = row
positive/negative = rotate clockwise, cc, respectively
*/

int getBoardToRotate(MOVE move) {
  return move/100;
}

int getColumn(MOVE move) {
  return (move % 100)/10;
}

int getRow(MOVE move) {
  return move % 10;
}

BOOLEAN isRotateClockwise(MOVE move) {
  if (move > 0) {   
    return FALSE;      
  } else {
    return TRUE;
  }
}





/************************************************************************
 ** Changelog
 **
 ** $Log$
 ** Revision 1.1  2006/10/23 07:15:54  davidcwu
 ** *** empty log message ***
 **
 ** Revision 1.10  2006/04/25 01:33:06  ogren
 ** Added InitialiseHelpStrings() as an additional function for new game modules to write.  This allows dynamic changing of the help strings for every game without adding more bookkeeping to the core.  -Elmer
 **
 ************************************************************************/

