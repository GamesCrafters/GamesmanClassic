// $id$
// $log$

/*
 * The above lines will include the name and log of the last person
 * to commit this file to CVS
 */

/************************************************************************
**
** NAME:        mwuzhi.c
**
** DESCRIPTION: Wu Zhi (5 piece game)
**
** AUTHOR:      Diana Fang, Dan Yan
**
** DATE:        2004-09-13
**
** UPDATE HIST: 2004-10-5  Added GenerateMoves with all its helpers
**              2004-10-12 Added Help Strings, Defines, Global Variables(i think there are more...but we need to talk about these)
**                         Started Initialize Game (What else do we need there?)
**              2004-10-18 Added Print Position, Print Move, Print Computer's Move, Primitive,
**                         Added another helper to Moves: GetArrayNum given xycoords.    
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


/*************************************************************************
**
** Game-specific constants
**
**************************************************************************/

STRING   kGameName            = "Wu Zhi"; /* The name of your game */
STRING   kAuthorName          = ""; /* Your name(s) */
STRING   kDBName              = "mwuzhi"; /* The name to store the database under */

BOOLEAN  kPartizan            = FALSE ; /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN  kGameSpecificMenu    = FALSE ; /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN  kTieIsPossible       = FALSE ; /* TRUE if a tie is possible. FALSE if it is impossible.*/
BOOLEAN  kLoopy               = TRUE ; /* TRUE if the game tree will have cycles (a rearranger style game). FALSE if it does not.*/

BOOLEAN  kDebugMenu           = TRUE ; /* TRUE only when debugging. FALSE when on release. */
BOOLEAN  kDebugDetermineValue = TRUE ; /* TRUE only when debugging. FALSE when on release. */

POSITION gNumberOfPositions   =  0; /* The number of total possible positions | If you are using our hash, this is given by the hash_init() function*/
POSITION gInitialPosition     =  0; /* The initial hashed position for your starting board */
POSITION kBadPosition         = -1; /* A position that will never be used */

/* 
 * Help strings that are pretty self-explanatory
 * Strings than span more than one line should have backslashes (\) at the end of the line.
 */

STRING kHelpGraphicInterface =
"Not written yet";

STRING   kHelpTextInterface    =
"On your turn, pick a piece that you want to move and then move it.  You can only move up, down, left, or right.  Diagonals are only allowed if you set up that option."; 

STRING   kHelpOnYourTurn =
"On your turn, pick a piece that you want to move. Look at the key to the right of the board to determine the number of the position of the piece you want to move.  Then choose the direction you want to move your piece(up, down, left, right...you can also refer to the compass, shown above the board). A sample move would be '1 up' ";

STRING   kHelpStandardObjective =
"Capture all but one of your opponent's pieces to win.  Capturing takes place when you move your piece into a specific configuration.  The configuration is as follows: If two of your pieces are next to each other and if on that same line that your pieces are one, one of those pieces is adjacent to an opponent's piece, then that results in a capture.  However, no other pieces must be along that same line where your capture is taking place in order for it to be a legal capture.  Also, a capture only occurs if you move into a capture position.  A capture does not occur if your opponent moves into your capture position.";

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
#define BLANK ' ' /* player 2 */
#define WHITE 'w' /* player 1 */
#define BLACK 'b'

#define UP 0
#define RIGHT 1
#define DOWN 2
#define LEFT 3

 


/*************************************************************************
**
** Global Variables
**
*************************************************************************/

/* default values */
int boardlength = 25;
int boardwidth = 5;
char* gBoard[boardlength + 1];


/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/

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
  int pieces_array[] = {WHITE, 1, boardwidth, BLACK, 1, boardwidth, BLANK, boardlength - (boardwidth * 2), boardlength - 2, -1 };
  gNumberOfPositions = generic_hash_init(boardlength, pieces_array, NULL);
    

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
    MOVELIST *moves = NULL;
    
    /* Use CreateMovelistNode(move, next) to 'cons' together a linked list */
    int player = whoseMove(position);
    char* board = generic_unhash (position, gBoard);
    char playerpiece = (player == 1 ? WHITE : BLACK);
    int i,c,r;
    for (i = 0; i < boardlength; i++) {
      if (board[i] = playerpiece) {
	c = getColumn (i, boardwidth);
	r = getRow (i,boardwidth);
	if (canmoveup(i, position, boardwidth)) {
	    moves = CreateMovelistNode(EncodeMove(0,c,r), moves);
	}
	if (canmovedown(i, position, boardwidth)) {
	  moves = CreateMovelistNode(EncodeMove(2,c,r), moves);
	}
	if (canmoveleft(i, position, boardwidth)) {
	  moves = CreateMovelistNode(EncodeMove(3,c,r), moves);
	}
	if (canmoveright(i, position, boardwidth)) {
	  moves = CreateMovelistNode(EncodeMove(1,c,r), moves);
	}
      }
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
    return 0;
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

  int player = whoseMove(position);
  char* board = generic_unhash (position, gBoard);
  char playerpiece = (player == 1 ? WHITE : BLACK);
  int numplayerpiece = 0;
  for (i = 0; i < boardlength; i++) {
    if (board[i] = playerpiece)
      numplayerpiece++;
  }
  if (numplayerpiece <= 1) 
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
  char* board;
  board = generic_unhash(position, EmptyBoard);
  printf("Player %s's turn\n", playersName);
  /* counters */
  int i, j, c, c1, c2;
  c = 1;
  printf("        up            \n");
  printf("        |             \n");
  printf("left --( )-- right     \n");
  printf("        |             \n");
  printf("       down           \n");
  printf("                     \n");
  for (i = 0; i < boardlength;) {
    for (j = 0 ; j < boardwidth; j ++) {
      printf ("(%c)", board [j]);
      i++;
      if (j < boardwidth - 1) {
	printf("--");
      }
    }
    printf ("\t");
    for (c1 = 0; c1 < boardwidth; c1++) {
      printf ("(%d)", c);
      c++;
      if (c1 < boardwidth - 1) {
	printf("--");
      }
    }
    printf("\n");
    for (c2 = 0; c2 < 2; c2++) { 
      for (k = 0; k < boardwidth - 1; k++) {
	printf(" |   ");
      }
      printf(" |");
      printf("\t");
    }
    printf("\n");
  }
  printf("\n%s", GetPrediction(position, playersName, usersTurn));
   
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
 printf("%8s's move   : ", computersName);
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
  int xcoord, ycoord, dir, Arraynum;
  String direction;
  xcoord = GetXCoord(move);
  ycoord = GetYCoord(move);
  dir = GetDir(move);
  Arraynum = GetArraynum(xcoord, ycoord); 
  if (dir == 0) 
    direction = "up";
  else if (dir == 1) 
    direction = "right";
  else if (dir == 2)
    direction = "down";
  else if (dir == 3) 
    direction = "left";
  printf("[%d %s]", Arraynum + 1, direction);
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

/* MOVE stuff 
MOVES are hashed where first three bits is the dir, second three is x, last three is y */
MOVE EncodeMove(int dir, int x, int y) 
{
  dir << 6;
  x << 3;
  return (dir | x | y);
}

int GetDirection (MOVE theMove) 
{
  return (theMove >> 6);
}

int GetXCoord (MOVE theMove) 
{
  return (theMove >> 3) & 0x7;
}

int GetYCoord (MOVE theMove) 
{
  return (theMove & 0x7);
}


/* Helper Functions for Generate Moves
All columns and rows start with zero and start from the lower left */
int getArraynum(int xcoord, int ycoord) {
  return ((boadrdwidth*(boardwidth - ycoord)) - (boardwidth - xcoord));
}


int getColumn(int arraynum, int width) 
{
  return boardwidth - 1 - (arraynum%width);
}

int getRow(int arraynum, int width) 
{
  if ((arraynum + 1)%boardwidth == 0)
    return boardwidth - 1;
  else 
    return boardwidth - 1 - (arraynum/boardwidth);
}

BOOLEAN canmoveup(int arraynum, POSITION position, int width) 
{
  char* board = generic_unhash(position, EmptyBoard);
  return ((arraynum - width >= 0) & (board[arraynum - width] == BLANK));
}

BOOLEAN canmovedown(int arraynum, POSITION position, int width) 
{
  char* board = generic_unhash(position, EmptyBoard);
  return ((arraynum + width < boardlength) & (board[arraynum + width] == BLANK));
}

BOOLEAN canmoveleft(int arraynum, POSITION position, int width) 
{
  char* board = generic_unhash(position, EmptyBoard);
  return (arraynum != 0 || arraynum%width != 0) & 
    (board[arraynum  - 1] == BLANK);
}

BOOLEAN canmoveright(int arraynum, POSITION position, int width) 
{
  char* board = generic_unhash(position, EmptyBoard);
  return ((arraynum + 1)% width != 0 & (board[arraynum + 1] == BLANK));
}

