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
#include "hash.h"
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
#define NUM_OF_COLS 3
#define BOARDSIZE NUM_OF_ROWS * NUM_OF_COLS
#define ROWS_OF_PIECES 1
#define NUM_OF_PIECES 2
#define X_ORIGINAL_POS (NUM_OF_ROWS - ROWS_OF_PIECES) * NUM_OF_COLS
#define O_ORIGINAL_POS ROWS_OF_PIECES * NUM_OF_COLS
#define Blank ' '
#define VERSION_REGULAR 0
#define VERSION_WAR 1
//the number that need to be in a row
#define NUM_IN_ROW 3

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

//hard code version for now
int version = VERSION_REGULAR;

/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/

//BOOLEAN inARow();
BlankOX oneOrNoPieces();
void PositionToBlankOX(POSITION thePos, BlankOX *theBlankOX);
POSITION BlankOXToPosition(BlankOX *theBlankOX, int turn);
BOOLEAN inARow(BlankOX theBlankOX[], BlankOX piece);
BOOLEAN adjacent(BlankOX theBlankOX[], int current, int next);
BlankOX oneOrNoPieces(BlankOX theBlankOX[]);
int BoardPosToArrayPos(int x, int y);
MOVE hashMove(unsigned int fromX, unsigned int fromY, unsigned int toX, unsigned int toY);
sMove unhashMove(MOVE move);
void clearRow(int x1, int y1, int x2, int y2,  BlankOX theBlankOX[]);
void clearColumn(int x1, int y1, int x2, int y2, BlankOX theBlankOX[] );
void captureLine(int x, int y, BlankOX piece, BlankOX theBlankOX[]);

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
  int pieces[] = {' ', 2, BOARDSIZE, 'x', 0, NUM_OF_COLS, 'o', 0, NUM_OF_COLS, -1};
 
  printf("Initializing game\n");
   for (i = 0; i < NUM_OF_COLS; i++) {
    sBlankOX[i] = 'x';
    }
  
  for (; i < NUM_OF_COLS*(NUM_OF_ROWS - 1); i++) {
    sBlankOX[i] = ' ';
  }
  for (; i < BOARDSIZE; i++) {
    sBlankOX[i] = 'o';
    }
 
  gNumberOfPositions = generic_hash_init(BOARDSIZE, pieces, NULL);
  gInitialPosition = generic_hash(sBlankOX, 1);
  
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
  int x,y,moveToX,moveToY;

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
  int  
    fromLoc,
    toLoc,
    leftLoc,
    rightLoc,
    upLoc,
    downLoc,
    turn;
  //printf("in doMove!\n");

 generic_unhash(position, sBlankOX);
  sMove theMove = unhashMove(move);
  
  fromLoc = (theMove.fromY) * NUM_OF_COLS + theMove.fromX;
  toLoc =  (theMove.toY) * NUM_OF_COLS + theMove.toX;
  leftLoc = (theMove.fromY) * NUM_OF_COLS + theMove.fromX -1;
  rightLoc = (theMove.fromY) * NUM_OF_COLS + theMove.fromX +1;
  upLoc = (theMove.fromY + 1) * NUM_OF_COLS + theMove.fromX;
  downLoc = (theMove.fromY - 1) * NUM_OF_COLS + theMove.fromX;
  turn = whoseMove(position);
  
  //printf("fromLoc: %d, toLoc: %d, leftLoc: %d, rightLoc: %d, upLoc: %d, downLoc: %d\n", fromLoc, toLoc, leftLoc, rightLoc, upLoc, downLoc);

  piece = sBlankOX[(theMove.fromY) * NUM_OF_COLS + theMove.fromX];
 
  
  if (turn == 1)
    turn = 2;
  else
    turn = 1;

  //if jumping captures, test to see if it was a vertical jump
  if (version == VERSION_WAR){
    
    //up jump
    if ((fromLoc + (2* NUM_OF_COLS)) == toLoc){
      if (sBlankOX[fromLoc + NUM_OF_COLS] != piece)
	sBlankOX[fromLoc + NUM_OF_COLS] = Blank;
    }
    //down jump
    if ((fromLoc - (2* NUM_OF_COLS)) == toLoc){
     
      if (sBlankOX[fromLoc - NUM_OF_COLS] != piece)
	sBlankOX[fromLoc - NUM_OF_COLS] = Blank;
    }
    //left jump
    if ((fromLoc - 2) == toLoc){
      if (sBlankOX[fromLoc - 1] != piece)
	sBlankOX[fromLoc - 1] = Blank;
    }
    

    //right jump
    if ((fromLoc + 2) == toLoc){
      if (sBlankOX[fromLoc + 1] != piece)
	sBlankOX[fromLoc + 1] = Blank;
    }

  }

  

  sBlankOX[fromLoc] = Blank;
  sBlankOX[toLoc] = piece;
  

  //perform any line capture that has been made
  captureLine(theMove.toX, theMove.toY, piece, sBlankOX);
  

  position = generic_hash(sBlankOX, turn);
   
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
  BlankOX piece = (turn == 1 ? 'x' : 'o');
  BlankOX oppositePiece  = (turn == 1 ? 'o' : 'x');

  generic_unhash(position, sBlankOX);
 
  if(inARow(sBlankOX, oppositePiece))
    return(gStandardGame ? lose : win);

  if (version == VERSION_WAR){
    if((oneOrNoPieces(sBlankOX) == 'x' && piece == 'x') ||
	  (oneOrNoPieces(sBlankOX) == 'o' && piece == 'o'))
    return lose;
    
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
  int row,col;
  generic_unhash(position, sBlankOX);

  printf("%s's turn\n  ", playersName);
  for (row = NUM_OF_ROWS-1; row >= 0; row--) {
    printf("\n%d |", row+1);
    for (col = 0; col < NUM_OF_COLS; col++) {
      printf("%c|", sBlankOX[row*NUM_OF_COLS + col]);
    }
  }
  printf("\n  ");
  for (col = 0; col < NUM_OF_COLS; col++) {
    printf(" %c", col+'a');
  }
  printf("\n\n");
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
  printf("\n");
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
  
   printf("%c", x.fromX+'a');
  printf("%d", x.fromY+1);
  printf("%c", x.toX+'a');
  printf("%d", x.toY+1);
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
	printf("%8s's move [(u)ndo/0-%c 0-%d 0-%c 0-%d] :  ",
	       playersName, 'a'+NUM_OF_COLS-1, NUM_OF_ROWS,
	       'a'+NUM_OF_COLS-1, NUM_OF_ROWS);
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
 
  /******
   * Check for valid input based upon chess notation
   * example a1-b2 
   */
  int strlen;
  for (strlen = 0; input[strlen] != -1; strlen++){
  }

  //check to see if input is long enough
  if (strlen < 5){

    return FALSE;
  }
  if(input[0] < 'a' || input[0] >= 'a' + NUM_OF_COLS){
    return FALSE;
    //printf("bad 0");
  }
  if(input[1] < '1' || input[1] >= '1' + NUM_OF_ROWS){
    return FALSE;
    //printf("bad 1");
  }
   
  if(input[2] < 'a' || input[3] >= 'a' + NUM_OF_COLS){
    return FALSE;
    //printf("bad 3");

  }
  if(input[3] < '1' || input[4] >= '1' + NUM_OF_ROWS){
    return FALSE;
    //printf("bad 4");
  }
  //printf("good input!!\n");
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
 
  /*****
   * example conversions
   * a1-c3 = 00-22
   * b3-a5 = 12-04
   */
  //printf("in convert!\n");
  int fromX = input[0] - 'a';
 int fromY = input[1] - '1';
 int toX = input[2] - 'a';
 int toY = input[3] - '1';
 //printf("input is %d %d %d %d\n", fromX, fromY, toX, toY);

 return hashMove(fromX,fromY,toX, toY);
  
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
  BlankOX theBlankOX[BOARDSIZE];
  signed char c;
  int i, j, boardPosX, boardPosY;
  int whoseturn = 1; //what should this be?
  
  printf("\n\n\t----- Get Initial Position -----\n");
  printf("\n\tPlease input the position to begin with.\n");
  printf("\tNote that it should be in the following format:\n\n");
  printf("\n EXAMPLE INPUT\n");
  
  for (i = 0; i < NUM_OF_ROWS -1; i++){
    printf("O");
    for (j = 0; j < NUM_OF_COLS - 1; j++){
      printf(" -");
    }
    printf("\n");
  }
  for (i = 0; i < NUM_OF_COLS; i++){
    printf("X "); 
  }
  printf("\n\n");
  i = 0;
  boardPosX = 0;
  boardPosY = NUM_OF_ROWS-1;
  getchar();
  while(i < BOARDSIZE && (c = getchar()) != EOF) {
    if(c == 'x' || c == 'X'){
      theBlankOX[boardPosY*NUM_OF_COLS + boardPosX] = 'x';
      boardPosX++;
      if (boardPosX == NUM_OF_ROWS){
	boardPosX = 0;
	boardPosY--;
      }
      i++;
    }
    else if(c == 'o' || c == 'O' ){
      theBlankOX[boardPosY*NUM_OF_COLS + boardPosX] = 'o';
      boardPosX++;
      if (boardPosX == NUM_OF_ROWS){
	boardPosX = 0;
	boardPosY--;
      }
      i++;
    }
    else if(c == '-'){
      theBlankOX[boardPosY*NUM_OF_COLS + boardPosX] = Blank;
      boardPosX++;
      if (boardPosX == NUM_OF_ROWS){
	boardPosX = 0;
	boardPosY--;
      }
      i++;
    }
    else
      ;   /* do nothing */
  }
  
  return generic_hash(theBlankOX, whoseturn);
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
**              BlankOX piece: the piece we are determining if is in a row
**
** OUTPUTS:     (BOOLEAN) TRUE iff there a given number of pieces in
**              a row. None of the pieces may be in the player's original
**              starting row(s).
**
************************************************************************/

BOOLEAN inARow(BlankOX theBlankOX[], BlankOX piece)
{
  int x, y, loc, xN, yN;
  int bad_row; //row that the pieces cannot be in

  
  if (piece == 'x'){
    bad_row = 0;
  } else {
    bad_row = NUM_OF_ROWS-1;
  }
  

  for (y = 0; y < NUM_OF_ROWS; y++){
    if (y != bad_row){
      for (x = 0; x < NUM_OF_COLS; x++){
	loc = y*NUM_OF_COLS + x;
	if(theBlankOX[loc] == piece){
	  
	  //check lateral row 
	  //only need to check to the right because we are coming from the left
	  for (xN = 0; x+xN < NUM_OF_COLS; xN++){
	  
	    if(theBlankOX[loc+xN] != piece)
	      break;
	  }
	  if (xN == NUM_IN_ROW)
	    return TRUE;
	  
	  //check vertical
	  //we are checking upwards
	  for (yN = 0; (y+yN < NUM_OF_ROWS) && (y+yN != bad_row); yN++){
	  
	    if(theBlankOX[loc+yN*NUM_OF_COLS] != piece)
	      break;
	  }
	  if (yN == NUM_IN_ROW)
	    return TRUE;

	  //check diagonal left
	  for (yN = 0, xN = 0; (y+yN < NUM_OF_ROWS) && (x-xN > 0) && ( y+yN != bad_row); yN++, xN++){
	  
	    if(theBlankOX[(y+yN)*NUM_OF_COLS + x-xN] != piece)
	      break;
	  }
	  if (yN == NUM_IN_ROW)
	    return TRUE;

	  //check diagonal right
	  for (yN = 0, xN = 0; (y+yN < NUM_OF_ROWS) && (x+xN < NUM_OF_COLS) && (y+yN != bad_row); yN++, xN++){
	  
	    if(theBlankOX[(y+yN)*NUM_OF_COLS + x+xN] != piece)
	      break;
	  }
	  if (yN == NUM_IN_ROW)
	    return TRUE;




	}
      }
    }
  }

  return FALSE;
  

  /**
  for (i = 0; i < NUM_OF_ROWS; i++) {
    for (j = 0; j < NUM_OF_COLS - 1; j++) {
      current = i*NUM_OF_COLS+j;
      next = current + 1;
      if (adjacent(theBlankOX, current, next)) {
	samePieces++;
	if (samePieces == NUM_OF_PIECES)
	  return TRUE;
      }
      else
	samePieces = 0;
    }
  }

  samePieces = 0;
  for (i = 0; i < NUM_OF_COLS; i++) {
    for (j = 0; j < NUM_OF_ROWS; j++) {
      current = j*NUM_OF_COLS+i;
      next = (j+1)*NUM_OF_COLS+i;
      if (adjacent(theBlankOX, current, next)) {
	samePieces++;
	if (samePieces == NUM_OF_PIECES)
	  return TRUE;
      }
      else
	samePieces = 0;
    }
  }

  samePieces = 0;
  j = 0;
  for (i = 0; i < NUM_OF_COLS; i++) {
    if (j >= NUM_OF_ROWS)
      break;
    current = i*NUM_OF_ROWS+j;
    next = (i+1)*NUM_OF_ROWS+j+1;
    if (adjacent(theBlankOX, current, next)) {
      samePieces++;
      if (samePieces == NUM_OF_PIECES)
	return TRUE;
    }
    else
      samePieces = 0;
    j++;
  }

  samePieces = 0;
  j = NUM_OF_ROWS - 1;
  for (i = 0; i < NUM_OF_COLS; i++) {
    if (j < 0)
      break;
    current = i*NUM_OF_ROWS+j;
    next = (i+1)*NUM_OF_ROWS+j-1;
    if (adjacent(theBlankOX, current, next)) {
      samePieces++;
      if (samePieces == NUM_OF_PIECES)
	return TRUE;
    }
    else
      samePieces = 0;
    j--;
  }

  return FALSE;
  **/
}

BOOLEAN adjacent(BlankOX theBlankOX[], int current, int next) {
  return(theBlankOX[current] == theBlankOX[next] &&
	 ((theBlankOX[current] == 'o' && current >= O_ORIGINAL_POS) ||
	  (theBlankOX[current] == 'x' && current < X_ORIGINAL_POS)) &&
	 ((theBlankOX[next] == 'o' && current >= O_ORIGINAL_POS) ||
	  (theBlankOX[next] == 'x' && current < X_ORIGINAL_POS)));
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
  printf("in oneOrNoPieces\n");
  for(i = 0; i < BOARDSIZE; i++) {
    if (theBlankOX[i] == 'x') {
      numOfx++;
    }
    else if(theBlankOX[i] == 'o') {
      numOfo++;
    }
  }

  if(numOfx <= 1)
    return 'x';
  if (numOfo <= 1)
    return 'o';
  else
    return Blank;
}


/*
 * Given an x and y return corresponding array position
 * -1 is bad x and y
 */

int BoardPosToArrayPos(int x, int y){
  /**
   * The first spot in the array is really the top
   * left corner, the last spot is bottom right corner  
  */
  
  if (x < 0 || x >= NUM_OF_COLS)
		return -1;
  if (y < 0 || y >= NUM_OF_ROWS)
    return -1;
  return (y*NUM_OF_COLS)+x;
}


/**
 *
 * Checks to see if the move of this piece to this location has caused a capture
 * if so clear out the captured pieces
 **/
void captureLine(int x, int y, BlankOX piece, BlankOX theBlankOX[]){
 
  int xN, yN;
  //printf("check piece is: %c\n", piece);
  //check capture up
  for (yN = 1; y+yN < NUM_OF_ROWS; yN++){
    if (theBlankOX[(y+yN)*NUM_OF_COLS + x] == Blank)
      break;
    if (theBlankOX[(y+yN)*NUM_OF_COLS + x] == piece && yN > 1){
      clearColumn(x, y+1, x, y+yN -1, theBlankOX);
      //printf("clear a\n");
      break;
    }
    if (theBlankOX[(y+yN)*NUM_OF_COLS + x] == piece)
      break;
  }
  //check capture down
  for (yN = 1; y-yN >= 0; yN++){
    if (theBlankOX[(y-yN)*NUM_OF_COLS + x] == Blank)
      break;
    if (theBlankOX[(y-yN)*NUM_OF_COLS + x] == piece && yN > 1){
      clearColumn(x, y-1, x, y-yN +1, theBlankOX);
      //printf("clear b\n");
      break;
    }
    if (theBlankOX[(y-yN)*NUM_OF_COLS + x] == piece)
      break;
  }
  //check capture left
  for (xN = 1; x-xN >= 0; xN++){
    if (theBlankOX[y*NUM_OF_COLS + (x-xN)] == Blank)
      break;
    if (theBlankOX[y*NUM_OF_COLS + (x-xN)] == piece && xN > 1){
      clearRow(x-1, y, x-xN +1, y, theBlankOX);
      //printf("clear c\n");
      break;
    }
    if (theBlankOX[y*NUM_OF_COLS + (x-xN)] == piece)
      break;
  }
  //check capture right
   for (xN = 1; x+xN < NUM_OF_COLS; xN++){
    if (theBlankOX[y*NUM_OF_COLS + (x+xN)] == Blank)
      break;
    if (theBlankOX[y*NUM_OF_COLS + (x+xN)] == piece && xN > 1){
      clearRow(x+1, y, x+xN -1, y, theBlankOX);
      //printf("clear d\n");
      break;
    }
    if (theBlankOX[y*NUM_OF_COLS + (x+xN)] == piece)
      break;
  }
}


//clear a given row

void clearRow(int x1, int y1, int x2, int y2, BlankOX theBlankOX[]){
  //printf("CLEARING!\n");
  if (x1 <= x2){
    while(!(x1 > x2)){
      theBlankOX[y1*NUM_OF_COLS + x1] = Blank;
      x1++;
    }
  } else {
    
    while(!(x2 > x1)){
      theBlankOX[y1*NUM_OF_COLS + x2] = Blank;
      x2++;
    }
  }

}

//clear a given column
void clearColumn(int x1, int y1, int x2, int y2, BlankOX theBlankOX[]){
  if (y1 <= y2){
    while(!(y1 > y2)){
      theBlankOX[y1*NUM_OF_COLS + x1] = Blank;
      y1++;
    }
  } else {
    while(!(y2 > y1)){
      theBlankOX[y2*NUM_OF_COLS + x1] = Blank;
      y2++;
    } 
    
  }
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
	
  //printf("in hash move!\n");
  /*
   * 0 is at bottom left cornor for both axes
   */
	MOVE move = 0;
	//printf("x = %d, y=%d x=%d y=%d\n", fromX, fromY, toX, toY);
	move |= (fromX << 24);
	move |= (fromY << 16);
	move |= (toX << 8);
	move |= toY;
	return move;
}

