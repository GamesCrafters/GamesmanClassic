// $id$
// $log$

/*
 * The above lines will include the name and log of the last person
 * to commit this file to CVS
 */

/************************************************************************
**
** NAME:        mshogi.c
**
** DESCRIPTION: Hasami Shogi
**
** AUTHOR:      Chris Willmore, Ann Chen
**
** DATE:        February 1, 2005 / FINISH DATE
**
** UPDATE HIST:
** 02/02/2005 Updated some game-specific constants.
** 02/02/2005 Added PrintPosition().
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
#include <ctype.h>

/*************************************************************************
**
** Game-specific constants
**
**************************************************************************/

void*   gGameSpecificTclInit = NULL;
STRING   kGameName            = "Hasami Shogi"; /* The name of your game */
STRING   kAuthorName          = "Chris Willmore, Ann Chen"; /* Your name(s) */
STRING   kDBName              = "mhshogi"; /* The name to store the database under */

BOOLEAN  kPartizan            = TRUE ; /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN  kGameSpecificMenu    = FALSE ; /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN  kTieIsPossible       = FALSE ; /* TRUE if a tie is possible. FALSE if it is impossible.*/
BOOLEAN  kLoopy               = TRUE ; /* TRUE if the game tree will have cycles (a rearranger style game). FALSE if it does not.*/

BOOLEAN  kDebugMenu           = TRUE ; /* TRUE only when debugging. FALSE when on release. */
BOOLEAN  kDebugDetermineValue = TRUE ; /* TRUE only when debugging. FALSE when on release. */

POSITION gNumberOfPositions   =  729; /* The number of total possible positions | If you are using our hash, this is given by the hash_init() function*/
POSITION gInitialPosition     =  0; /* The initial hashed position for your starting board */
POSITION kBadPosition         = -1; /* A position that will never be used */

/* 
 * Help strings that are pretty self-explanatory
 * Strings than span more than one line should have backslashes (\) at the end of the line.
 */

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

STRING   kHelpTieOccursWhen =
"A tie occurs when ...";

STRING   kHelpExample =
"";


/*************************************************************************
**
** #defines and structs
**
**************************************************************************/

#define NUM_OF_ROWS 3
#define NUM_OF_COLS 2
#define BOARDSIZE NUM_OF_ROWS * NUM_OF_COLS
#define ROWS_OF_PIECES 1
#define NUM_OF_PIECES 2
#define Blank ' '
typedef char BlankOX;
/* Represents a complete shogi move */

typedef struct cleanMove {
	int fromX;
	int fromY;
	int toX;
	int toY;
} sMove;

/*************************************************************************
**
** Global Variables
**
*************************************************************************/

 
char *blankOXString[] = {" ", "o", "x"};

char sBlankOX[BOARDSIZE];

/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/

BOOLEAN inARow();
BlankOX oneOrNoPieces();
void PositionToBlankOX(POSITION thePos, BlankOX *theBlankOX);
POSITION BlankOXToPosition(BlankOX *theBlankOX, int turn);
BOOLEAN inARow(BlankOX theBlankOX[]);
BlankOX oneOrNoPieces(BlankOX theBlankOX[]);
int BoardPosToArrayPos(int x, int y);
MOVE hashMove(unsigned int fromX, unsigned int fromY, unsigned int toX, unsigned int toY);
sMove unhashMove(MOVE move);


/* External */
extern GENERIC_PTR	SafeMalloc ();
extern void		SafeFree ();


/*************************************************************************
**
** Global Database Declaration
**
**************************************************************************/

extern VALUE     *gDatabase;


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
  int i;
  /*int pieces[] = {' ', BOARDSIZE - 2*ROWS_OF_PIECES*NUM_OF_COLS, BOARDSIZE,
		   'x', 0, ROWS_OF_PIECES*NUM_OF_COLS,
		   'o', 0, ROWS_OF_PIECES*NUM_OF_COLS, -1};*/
  int pieces[] = {' ', 2, 6, 'x', 0, 2, 'o', 0, 2, -1};
  BlankOX  newBlankOX[BOARDSIZE];
  for (i = 0; i < NUM_OF_COLS; i++) {
    sBlankOX[i] = 'o';
  }
  for (; i < NUM_OF_COLS*(NUM_OF_ROWS - 1); i++) {
    sBlankOX[i] = ' ';
  }
  for (; i < BOARDSIZE; i++) {
    sBlankOX[i] = 'x';
  }
  for (i = 0; i < BOARDSIZE; i++) {
    printf("%c", sBlankOX[i]);
  }
  printf("\n");
  gNumberOfPositions = generic_hash_init(BOARDSIZE, pieces, NULL);
  gInitialPosition = generic_hash(sBlankOX, 1);
  generic_unhash(gInitialPosition, newBlankOX);
  for (i = 0; i < BOARDSIZE; i++) {
    printf("%c", sBlankOX[i]);
  }
  printf("\n");
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

MOVELIST *GenerateMoves(position)
     POSITION position;
{
  MOVELIST *head = NULL;
  
  char turn;  
  int x,y,moveToX,moveToY,i;

  generic_unhash(position,sBlankOX);
  //printf("in gernateMoves\n");
  if (whoseMove(position) == 1)
    turn = 'x';
  else
    turn = 'o';
	
  for(x = 0; x < NUM_OF_COLS; x++){
    for (y = 0; y < NUM_OF_ROWS; y++){
      int i = BoardPosToArrayPos(x, y);
      if (sBlankOX[i] == turn){
	int n; 
	//check for jumps
	//jump up
	if (((n = BoardPosToArrayPos(x,y-1)) > -1) 
	    && sBlankOX[n] != Blank
	    && (((n = BoardPosToArrayPos(x,y-2)) > -1))
	    && sBlankOX[n] == Blank) {
	  head = CreateMovelistNode(hashMove(x,y,x,y-2),head);
	    }
	//jump right
	if (((n = BoardPosToArrayPos(x+1,y)) > -1) 
	    && sBlankOX[n] != Blank
	    && (((n = BoardPosToArrayPos(x+2,y)) > -1))
	    && sBlankOX[n] == Blank) {
	  head = CreateMovelistNode(hashMove(x,y,x+2,y),head);
	    }
	//jump down
	if (((n = BoardPosToArrayPos(x,y+1)) > -1) 
	    && sBlankOX[n] != Blank
	    && (((n = BoardPosToArrayPos(x,y+2)) > -1))
	    && sBlankOX[n] == Blank) {
	  head = CreateMovelistNode(hashMove(x,y,x,y+2),head);
	    }
	//jump left
	if (((n = BoardPosToArrayPos(x-1,y)) > -1) 
	    && sBlankOX[n] != Blank
	    && (((n = BoardPosToArrayPos(x-2,y)) > -1))
	    && sBlankOX[n] == Blank) {
	  head = CreateMovelistNode(hashMove(x,y,x-2,y),head);
	    }
 
				
			
	//check for lateral move left
	moveToX = x-1;
	while((moveToX >= 0) && sBlankOX[BoardPosToArrayPos(moveToX,y)] == Blank){
	  head = CreateMovelistNode(hashMove(x,y,moveToX,y), head);
	  moveToX--;
	}
			
	//check for lateral move right
	moveToX = x+1;
	while((moveToX < NUM_OF_COLS) && sBlankOX[BoardPosToArrayPos(moveToX,y)] == Blank){
	  head = CreateMovelistNode(hashMove(x,y,moveToX,y), head);
	  moveToX++;
	}
	//check for vertical move up
	moveToY = y-1;
	while((moveToY >= 0) && sBlankOX[BoardPosToArrayPos(x,moveToY)] == Blank){
	  head = CreateMovelistNode(hashMove(x,y,x,moveToY), head);
	  moveToY--;
	}
	//check for vertical move down
	moveToY = y+1;
	while((moveToY <= NUM_OF_ROWS) && sBlankOX[BoardPosToArrayPos(x,moveToY)] == Blank){
	  head = CreateMovelistNode(hashMove(x,y,x,moveToY), head);
	  moveToY++;
	}
			


      }

    }
  }
  
  //printf("leave gernateMoves\n");
  return head;
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
  BlankOX piece;
  generic_unhash(position, sBlankOX);
  sMove theMove = unhashMove(move);
  int i;
int turn = whoseMove(position);
//printf("start do move\n"); 
//PrintMove(move);
//printf("\n");
piece = sBlankOX[(theMove.fromY) * NUM_OF_COLS + theMove.fromX];
//printf("current piece is %c\n", piece);  
//printf("turn is %d\n", turn);


if (turn == 1)
    turn = 2;
  else
    turn = 1;
 
  sBlankOX[(theMove.fromY) * NUM_OF_COLS + theMove.fromX] = Blank;
  sBlankOX[(theMove.toY) * NUM_OF_COLS + theMove.toX] = piece;

for (i = 0; i < BOARDSIZE; i++) {
  //printf("%c", sBlankOX[i]);
  }
//printf("\n");
 
  //printf("ok!\n");
  position = generic_hash(sBlankOX, turn);
  // printf("leave do move\n"); 
  return position;
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
** CALLS:       BOOLEAN inARow()
**              BlankOX oneOrNoPieces()
**              BlankOX PositionToBlankOX()          
**
************************************************************************/

VALUE Primitive (POSITION position)
{
  
  int turn = whoseMove(position);

  generic_unhash(position, sBlankOX);

  if(inARow(sBlankOX))
    return(gStandardGame ? lose : win);
  else if((oneOrNoPieces(sBlankOX) == 1 && turn == 'x') ||
	  (oneOrNoPieces(sBlankOX) == 2 && turn == 'o'))
    return lose;
  else
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
  int row, col, i;
  BlankOX  newOX[BOARDSIZE];
  generic_unhash(position, sBlankOX);
  for (i = 0; i < BOARDSIZE; i++) {
    printf("%c", sBlankOX[i]);
  }
  printf("\n");
 


  /*  printf("%s's turn\n  ", playersName);
  for (col = 0; col < NUM_OF_COLS; col++) {
    printf("_____");
  }
  for (row = 0; row < NUM_OF_ROWS; row++) {
    printf("\n  |");
    for (col = 0; col < NUM_OF_COLS; col++) {
      printf("   |");
    }
    printf("\n%d |", NUM_OF_ROWS - row);
    for (col = 0; col < NUM_OF_COLS; col++) {
      printf(" %c |", theBlankOX[row * NUM_OF_COLS + col]);
    }
    printf("\n  |");
    for (col = 0; col < NUM_OF_COLS; col++) {
      printf("   |");
    }
    printf("\n  ");
    for (col = 0; col < NUM_OF_COLS; col++) {
      printf("_____");
    }
  }
  printf("\n\n ");
  for (col = 0; col < NUM_OF_COLS; col++) {
    printf("   %c", 'a' + col);
  }
  printf("\n\n");*/
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
  printf("%s's move: ", computersName);
  PrintMove(computersMove);
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
  struct cleanMove x;
  x = unhashMove(move);
  /** printf("[(%d, ", 'a' + x.fromX);
  printf("%d) ", x.fromY);
  printf("(%d, ", 'a' + x.toX);
  printf("%d)]", x.toY);
  **/
   printf("[(%d, ", x.fromX);
  printf("%d) ", x.fromY);
  printf("(%d, ", x.toX);
  printf("%d)]", x.toY);
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
	printf("%8s's move [(u)ndo/a-%c 1-%d a-%c 1-%d] :  ",
	       playersName, 'a'+NUM_OF_COLS-1, NUM_OF_ROWS,
	       'a'+NUM_OF_COLS-1, NUM_OF_ROWS);
	input = HandleDefaultTextInput(position, move, playersName);
	// input[0] = tolower(input[0]);
	// input[2] = tolower(input[2]);

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
  /** if (input[0] < 'a' || input[0] >= 'a'+NUM_OF_COLS)
    return FALSE;
  if (input[1] < '1' || input[1] > '1'+NUM_OF_ROWS-1)
    return FALSE;
  if (input[2] < 'a' || input[2] >= 'a'+NUM_OF_COLS)
    return FALSE;
  if (input[3] < '1' || input[3] > '1'+NUM_OF_ROWS -1)
    return FALSE;
  return TRUE;
  **/

  printf("!!!!! %s\n", input);
   if (input[0] < '0' || input[0] >= '0'+NUM_OF_COLS)
    return FALSE;
  if (input[1] < '0' || input[1] > '0'+NUM_OF_ROWS-1)
    return FALSE;
  if (input[2] < '0' || input[2] >= '0'+NUM_OF_COLS)
    return FALSE;
  if (input[3] < '0' || input[3] > '0'+NUM_OF_ROWS -1)
    return FALSE;
  printf("FFFFFFFFFF\n");
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
** INPUTS:      STRING input : The VALID string input from the user.
**
** OUTPUTS:     MOVE         : Move converted from user input.
**
************************************************************************/

MOVE ConvertTextInputToMove (STRING input)
{
 
  printf("in convert!\n");
 return hashMove(input[0]-'0',input[1]-'0',input[2]-'0', input[3]-'0');
  
}


/************************************************************************
**
** Name:        GameSpecificMenu
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
  printf("hehehhehhe\n");
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
** NAME:        PositionToBlankOX
**
** DESCRIPTION: convert an internal position to that of a BlankOX.
** 
** INPUTS:      POSITION thePos     : The position input. 
**              BlankOX *theBlankOx : The converted BlankOX output array. 
**
** CALLS:       BadElse()
**
************************************************************************/

void PositionToBlankOX(POSITION thePos, BlankOX *theBlankOX)
{
  generic_unhash(thePos, theBlankOX);
}

/************************************************************************
**
** NAME:        BlankOXToPosition
**
** DESCRIPTION: convert a BlankOX to that of an internal position.
** 
** INPUTS:      BlankOX *theBlankOx : The converted BlankOX output array.
**
** OUTPUTS:     POSITION: The equivalent position given the BlankOX.
**
************************************************************************/

POSITION BlankOXToPosition(BlankOX *theBlankOX, int turn)
{
  return generic_hash(theBlankOX, turn);
}

/************************************************************************
**
** NAME:        inARow
**
** DESCRIPTION: Return TRUE iff there are a given number of pieces in
**              a row. None of the pieces may be in the player's original
**              starting row(s).
** 
** INPUTS:      BlankOX theBlankOX[BOARDSIZE] : The BlankOX array.
**
** OUTPUTS:     (BOOLEAN) TRUE iff there a given number of pieces in
**              a row. None of the pieces may be in the player's original
**              starting row(s).
**
************************************************************************/

BOOLEAN inARow(BlankOX theBlankOX[])
{
  int i, j, samePieces = 1;

  for (i = 0; i < BOARDSIZE; i++) {
    if (theBlankOX[i] == theBlankOX[i+1] &&
	((theBlankOX[i] == 'o' && i >= ROWS_OF_PIECES * NUM_OF_COLS) ||
	 (theBlankOX[i] == 'x' && i < (NUM_OF_ROWS - ROWS_OF_PIECES) * NUM_OF_COLS))) {
      if (i == 2)
	printf("zzzzzzzzzzzzz\n");
      samePieces++;
      if (samePieces == NUM_OF_PIECES)
	return TRUE;
      else
	samePieces = 0;
    }
  }

  samePieces = 0;
  for (i = 0; i < NUM_OF_COLS; i++) {
    for (j = 0; j < NUM_OF_ROWS; j++) {
      if (theBlankOX[j*NUM_OF_COLS+i] == theBlankOX[(j+1)*NUM_OF_COLS+i] &&
	  ((theBlankOX[i] == 'o' && i >= ROWS_OF_PIECES * NUM_OF_COLS) ||
	   (theBlankOX[i] == 'x' && i < (NUM_OF_ROWS - ROWS_OF_PIECES) * NUM_OF_COLS))) {
	samePieces++;
	if (samePieces == NUM_OF_PIECES)
	  return TRUE;
	else
	  samePieces = 0;
      }
    }
  }

  samePieces = 0;
  j = 0;
  for (i = 0; i < NUM_OF_COLS; i++) {
    if (j >= NUM_OF_ROWS)
      break;
    if (theBlankOX[i*NUM_OF_ROWS+j] == theBlankOX[(i+1)*NUM_OF_ROWS+j+1] &&
	((theBlankOX[i] == 'o' && i >= ROWS_OF_PIECES * NUM_OF_COLS) ||
	 (theBlankOX[i] == 'x' && i < (NUM_OF_ROWS - ROWS_OF_PIECES) * NUM_OF_COLS))) {
      samePieces++;
      if (samePieces == NUM_OF_PIECES)
	return TRUE;
      else
	samePieces = 0;
    }
    j++;
  }

  samePieces = 0;
  j = NUM_OF_ROWS - 1;
  for (i = 0; i < NUM_OF_COLS; i++) {
    if (j < 0)
      break;
    if (theBlankOX[i*NUM_OF_ROWS+j] == theBlankOX[(i+1)*NUM_OF_ROWS+j-1] &&
	((theBlankOX[i] == 'o' && i >= ROWS_OF_PIECES * NUM_OF_COLS) ||
	 (theBlankOX[i] == 'x' && i < (NUM_OF_ROWS - ROWS_OF_PIECES) * NUM_OF_COLS))) {
      samePieces++;
      if (samePieces == NUM_OF_PIECES)
	return TRUE;
      else
	samePieces = 0;
    }
    j--;
  }

  return FALSE;
}

/************************************************************************
**
** NAME:        oneOrNoPieces
**
** DESCRIPTION: Return TRUE iff one player has one or no pieces.
** 
** INPUTS:      BlankOX theBlankOX[BOARDSIZE] : The BlankOX array.
**
** OUTPUTS:     (BlankOX) x iff there is less than one x,
**                        o iff there is less than one o,
**                        Blank iff there are more than one x and more
**                        than one o.
**
************************************************************************/

BlankOX oneOrNoPieces(BlankOX theBlankOX[])
{
  int i, numOfx = 0, numOfo = 0;
	
  for(i = 0; i < BOARDSIZE; i++) {
    if (theBlankOX[i] == 'x') {
      numOfx++;
      if (numOfx > 1 && numOfo > 1)
	return Blank;
    }
    else if(theBlankOX[i] == 'o') {
      numOfo++;
      if (numOfx > 1 && numOfo > 1)
	return Blank;
    }
  }

  if(numOfx <= 1)
    return 'x';
  else
    return 'o';
}

int BoardPosToArrayPos(int x, int y){
	if (x < 0 || x >= NUM_OF_COLS)
		return -1;
	if (y < 0 || y >= NUM_OF_ROWS)
		return -1;
	return NUM_OF_COLS*y+x;
}

/******
 * Unhash a shogi move
 * 
 * input: MOVE
 *
 * output: sMove  
 ******/
sMove unhashMove(MOVE move){
	struct cleanMove m;
	m.fromX = (move >> 24) & 0xff;
	m.fromY = (move >> 16) & 0xff;
	m.toX = (move >> 8) & 0xff;
	m.toY = move & 0xff;
	
	return m;
}

/********
 * hash a shogi move
 * 
 * input: int fromX
 *        int fromY
 *        int toX
 *        int toY
 * return: void
 ********/
MOVE hashMove(unsigned int fromX, 
	unsigned int fromY, 
	unsigned int toX, 
	unsigned int toY){
	
	MOVE move = 0;
	move |= (fromX << 24);
	move |= (fromY << 16);
	move |= (toX << 8);
	move |= toY;
	return move;
}

