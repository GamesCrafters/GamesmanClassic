/************************************************************************
**
** NAME:        mhex.c
**
** DESCRIPTION: Hex
**
** AUTHOR:      Shah Bawany and Jacob Andreas
**
** DATE:        18 September 2006 / ...
**
** UPDATE HIST: RECORD CHANGES YOU HAVE MADE SO THAT TEAMMATES KNOW
**
** LAST CHANGE: $Id$
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

STRING   kGameName            = "Hex"; /* The name of your game */
STRING   kAuthorName          = "Shah Bawany and Jacob Andreas"; /* Your name(s) */
STRING   kDBName              = "Hex"; /* The name to store the database under */

BOOLEAN  kPartizan            = TRUE ;  /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN  kGameSpecificMenu    = FALSE ; /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN  kTieIsPossible       = FALSE ; /* TRUE if a tie is possible. FALSE if it is impossible.*/
BOOLEAN  kLoopy               = FALSE ; /* TRUE if the game tree will have cycles (a rearranger style game). FALSE if it does not.*/

BOOLEAN  kDebugMenu           = TRUE ;  /* TRUE only when debugging. FALSE when on release. */
BOOLEAN  kDebugDetermineValue = TRUE ;  /* TRUE only when debugging. FALSE when on release. */

POSITION gNumberOfPositions   =  0; /* The number of total possible positions | If you are using our hash, this is given by the hash_init() function*/
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

#define BOARDROWS 4
#define BOARDCOLS BOARDROWS
#define BOARDSIZE BOARDROWS * BOARDCOLS

#define BLACKCHAR '#'
#define WHITECHAR '0'
#define BLANKCHAR ' '

#define WHITEPLAYER 1
#define BLACKPLAYER 2

#define FIRSTPLAYER WHITEPLAYER

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

int                     vcfg(int *this_cfg);
void                    InitializeHelpStrings();
MOVELIST*               getNextBlank(STRING board, int index);
STRING                  MoveToString(MOVE move);
int                     ColMaskBoard(char* board, int col, char piece);
int                     RowMaskBoard(char* board, int row, char piece);

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
  int i;
  char* initialBoard;
  int pieces[] = {BLANKCHAR, 0, BOARDSIZE, BLACKCHAR, 0, BOARDSIZE/2, WHITECHAR, 0, (BOARDSIZE/2), -1};

  InitializeHelpStrings();
  
  gNumberOfPositions = generic_hash_init(BOARDSIZE, pieces, vcfg, 0);  // ***
  
  initialBoard = (char*)SafeMalloc((BOARDSIZE+1)*sizeof(char));
  
  for(i = 0; i < BOARDSIZE; i++)
	initialBoard[i] = BLANKCHAR;
  initialBoard[BOARDSIZE] = '\0';
	
  
  gInitialPosition = generic_hash_hash(initialBoard, FIRSTPLAYER);
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
    MOVELIST* validMoves;
    char* board;	

    board = (char*)SafeMalloc((BOARDSIZE+1)*sizeof(char));

    generic_hash_unhash(position, board);	
    
    validMoves = getNextBlank(board, 0);	
    SafeFree(board);
    return validMoves;
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
  char* board;
  POSITION newPosition;
  
  board = (char*)SafeMalloc((BOARDSIZE+1)*sizeof(char));
  generic_hash_unhash(position, board);
  
  board[move] = (generic_hash_turn(position) == 1) ? WHITECHAR : BLACKCHAR;
  
  newPosition = generic_hash_hash(board, generic_hash_turn(position));
  SafeFree(board);
  
  return newPosition;  
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
    char* board;
    int mask, nextmask, i;

    
    board = (char*)SafeMalloc(BOARDSIZE*sizeof(char));

    generic_hash_unhash(position, board);

    mask = RowMaskBoard(board, 0, BLACKCHAR);
    for(i = 1; i < BOARDROWS; i++) {
        nextmask = 
	(RowMaskBoard(board, i, BLACKCHAR)|(RowMaskBoard(board, i, BLACKCHAR) << 1));
	mask = mask & nextmask;
    }
    if(mask != 0) {
      return win;     // BLACK WINS
    }

    mask = RowMaskBoard(board, 0, WHITECHAR);
    for(i = 1; i < BOARDROWS; i++) {
        nextmask = 
	(RowMaskBoard(board, i, WHITECHAR)|(RowMaskBoard(board, i, WHITECHAR) << 1));
	mask = mask & nextmask;
    }
    if(mask != 0) {
      return lose;  // WHITE WINS
    }
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

  int n, row, i, col;
  char* board;

  board = (char*)SafeMalloc((BOARDSIZE+1)*sizeof(char));
  
  generic_hash_unhash(position, board);



  
  // printf("Prediction: %s", getPrediction());
  printf("%s's turn:\n", playersName);
  
  printf("  ");

  printf("\n");
  
  for(row = 0; row < BOARDROWS; row++) {
    for(i = 0; i < row; i++)
	  printf("   ");
    printf(" %c ", '0'+row);
    for(col = 0; col < BOARDCOLS; col++)
	  printf("[  %c ]", board[row*BOARDROWS+col]);
	printf("\n\n");
  }

  for(col = 0; col < BOARDCOLS; col++)
    printf("    ");

  for(n = 0; n < BOARDCOLS; n++)
    printf("%c     ", 'a'+n);

  printf("\n");
  
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
  STRING str = (char*)SafeMalloc(20*sizeof(char)); 
  str = MoveToString( move );

  printf( "%s", str );
  SafeFree( str );
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
  STRING movestring = (char*)SafeMalloc(20*sizeof(char));
  
  movestring[0] = 'a' + (int)(move % BOARDCOLS);
  movestring[1] = '0' + (int)(move / BOARDROWS);
  movestring[2] = '\0';
  return movestring;
  SafeFree(movestring);
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
	printf("%8s's move [(undo)/#l] : ", playersName);
	
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

BOOLEAN ValidTextInput (STRING input) {

      if(strlen(input) < 2) {
           return FALSE;
      } else if((input[0] < 'a') || (input[0] > BOARDROWS+'a')) {
           return FALSE;
      } else if((input[1] < '0') || (input[1] > BOARDROWS+'0')) { 
           return FALSE;
      } else { 
           return TRUE; 
      } 
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
    
    return((input[0]-'a')+input[1]*BOARDROWS);
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


int vcfg(int *this_cfg) {
    return this_cfg[1] == this_cfg[2] || this_cfg[1] == this_cfg[2] + 1;
}



MOVELIST* getNextBlank(STRING board, int index) {
    MOVELIST* validMoves;

    while((board[index] != BLANKCHAR) && (index < BOARDSIZE)) {
	index++;
    }
    if(index==BOARDSIZE) {
    	validMoves = NULL;
    } else {
        validMoves = CreateMovelistNode(index, getNextBlank(board, index+1));
    }
    return validMoves;
}

int RowMaskBoard(char* board, int row, char piece) {
	int i, mask;
	mask = 0;

	if(row > BOARDROWS) {
		return -1;
	}
	for(i = 0; i < BOARDROWS; i++) {
		mask <<= 1;
		mask += (board[row*BOARDROWS+i] == piece) ? 1 : 0;
	}
	return mask;
}

int ColMaskBoard(char* board, int col, char piece) {
	int i, mask;
	mask = 0;

	if(col > BOARDROWS) {
		return -1;
	}
	for(i = 0; i < BOARDROWS; i++) {
		mask <<= 1;
		mask += (board[i*BOARDROWS+col] == piece) ? 1 : 0;
	}
	return mask;
}






/************************************************************************
 ** Changelog
 **
 ** $Log$
 ** Revision 1.1  2006/10/22 22:26:45  and-qso
 ** *** empty log message ***
 **
 ** Revision 1.10  2006/04/25 01:33:06  ogren
 ** Added InitialiseHelpStrings() as an additional function for new game modules to write.  This allows dynamic changing of the help strings for every game without adding more bookkeeping to the core.  -Elmer
initialBoard = (char*)SafeMalloc((BOARDSIZE+1)*sizeof(char));
  
  for(i = 0; i < BOARDSIZE; i++)
	initialBoard[i] = BLANKCHAR;
  initialBoard[BOARDSIZE] = '\0';
	
  
  gInitialPosition = generic_hash_hash(initialBoard, FIRSTPLAYER); **
 ************************************************************************/

