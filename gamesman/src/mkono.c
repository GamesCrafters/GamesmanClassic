// $id$
// $log$

/*
 * The above lines will include the name and log of the last person
 * to commit this file to CVS
 */

/************************************************************************
**
** NAME:        mkono.c
**
** DESCRIPTION: (Four Field) Kono
**
** AUTHOR:      Greg Bonin, Nathan Spindel
**
** DATE:        09/29/04
**
** UPDATE HIST: 09/29/04 - Initial Commit: DoMove, PrintPosition, Primitive,
**                         helper functions, board representation.
**
**              10/07/04 - Fixed up everything. Game runs!
**                         To-do: ValidTextInput and game specific options.
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


/*************************************************************************
**
** Game-specific constants
**
**************************************************************************/

STRING   kGameName            = "(Four Field) Kono"; /* The name of your game */
STRING   kAuthorName          = "Greg Bonin, Nathan Spindel";   /* Your name(s) */
STRING   kDBName              = ""; /* The name to store the database under */

BOOLEAN  kPartizan            = TRUE ; /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
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

#define EMPTY  0
#define BLACK_PLAYER 1
#define WHITE_PLAYER 2

/*************************************************************************
**
** Global Variables
**
*************************************************************************/

int WIDTH = 2, HEIGHT = 5;
char *gBoard;

/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/

/* Internal */
int exponent(int base, int exponent);
void printRowOfStars();
int numberOfPieces(POSITION board, int player);
int oppositePlayer(int player);
int getSourceFromMove(MOVE move);
int getDestFromMove(MOVE move);
MOVE makeMove(int source, int dest);
int neighbors(int x, int y);

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
  int BOARDSIZE = WIDTH*HEIGHT;

  gBoard = (char *) SafeMalloc (BOARDSIZE * sizeof(char));
  
  int piece_array[] = {'x', 0, BOARDSIZE/2,
		       'o' , 0, BOARDSIZE/2,
		       ' ', 0, BOARDSIZE-1, 
		       -1};
  
  gNumberOfPositions = generic_hash_init(BOARDSIZE, piece_array, 0);

  for (i = 0; i < BOARDSIZE/2; i++)
    gBoard[i] = 'x';

  if (BOARDSIZE % 2 != 0) {
    gBoard[i] = ' ';
    i++;
  }

  for (; i < BOARDSIZE; i++) {
    gBoard[i] = 'o';
  }

  gInitialPosition = generic_hash(gBoard, 1);

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
  /*  MOVELIST *CreateMovelistNode(); */
  MOVELIST *moves = NULL, *captures = NULL;
  int player = whoseMove (position);
  int i;
  char currentPlayer, oppPlayer;

  generic_unhash(position, gBoard);

  if (player == 1) {
    currentPlayer = 'x';
    oppPlayer = 'o';
  } else {
    currentPlayer = 'o';
    oppPlayer = 'x';
  }
  
  /* Use CreateMovelistNode(move, next) to 'cons' together a linked list */
  
  for (i = WIDTH*HEIGHT; i >= 0; i--) {
    if (gBoard[i] == currentPlayer) {

      /* check up one */
      if (i > WIDTH)
	if (gBoard[i-WIDTH] == ' ')
	  moves = CreateMovelistNode(makeMove(i, i-WIDTH), moves);

      /* check up two */
      if (i > WIDTH*2)
	if (gBoard[i-WIDTH] == currentPlayer && gBoard[i-WIDTH*2] == oppPlayer)
	  captures = CreateMovelistNode(makeMove(i, i-WIDTH*2), captures);

      /* check down one */
      if (i < WIDTH*(HEIGHT-1))
	if (gBoard[i+WIDTH] == ' ')
	  moves = CreateMovelistNode(makeMove(i, i+WIDTH), moves);

      /* check down two */
      if (i < WIDTH*(HEIGHT-2))
	if (gBoard[i+WIDTH] == currentPlayer && gBoard[i+WIDTH*2] == oppPlayer)
	  captures = CreateMovelistNode(makeMove(i, i+WIDTH*2), captures);

      /* check left one */
      if (i % WIDTH != 0)
	if (gBoard[i-1] == ' ')
	  moves = CreateMovelistNode(makeMove(i, i-1), moves);

      /* check left two */
      if (i % WIDTH > 1)
	if (gBoard[i-1] == currentPlayer && gBoard[i-2] == oppPlayer)
	  captures = CreateMovelistNode(makeMove(i, i-2), captures);

      /* check right one */
      if (i % WIDTH < WIDTH-1)
	if (gBoard[i+1] == ' ')
	  moves = CreateMovelistNode(makeMove(i, i+1), moves);

      /* check right two */
      if (i % WIDTH < WIDTH-2)
	if (gBoard[i+1] == currentPlayer && gBoard[i+2] == oppPlayer)
	  captures = CreateMovelistNode(makeMove(i, i+2), captures);
    }
  }
  
  /* if there are capture moves, return those. otherwise, return regular moves */
  if (captures == NULL)
    return moves;
  else
    return captures;
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
  int player = whoseMove(position);

  generic_unhash(position, gBoard);

  gBoard[getSourceFromMove(move)] = ' ';

  gBoard[getDestFromMove(move)] = (player == BLACK_PLAYER) ? 'x' : 'o';

  return generic_hash(gBoard, oppositePlayer(player));
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

  if (numberOfPieces(position, player) == 1)
    return lose;
  else if (numberOfPieces(position, oppositePlayer(player)) == 1)
    return win;
  else if (GenerateMoves(position) == NULL)
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
  int i, j, k;
  
  generic_unhash(position, gBoard);

  /* top row */
  printRowOfStars();

  /* other rows */
  for (i = 0; i < HEIGHT; i++) {
    printf("\t");
    /* print the legend */
    for (k = 0; k < WIDTH; k++) {
      if (WIDTH*i+k < 10) printf(" ");
      printf("%d ", WIDTH*i+k);
    }

    printf("\t| ");

    for (j = 0; j < WIDTH-1; j++)
      printf(" %c --", gBoard[i*WIDTH+j]);

    /* print last piece in each row and a | and a tab */
    printf(" %c  |\t", gBoard[i*WIDTH +j]);

    printf("\n");

    /* print the | rows in between */
    if (i != HEIGHT-1) {
      printf("\t");
      for (k = 0; k < WIDTH; k++) printf("   ");
      printf("\t|");
      for (k = 0; k < WIDTH; k++) printf("  |  ");
      printf("|\n");
    }
  }

  printf("\t");
  for (k = 0; k < WIDTH; k++) printf("   ");
  printf("\t|");
  for (k = 0; k < WIDTH; k++) printf("     ");
  printf("|\n");

  /* bottom row */
  printRowOfStars();

  printf("\n\t\tIt is %s turn.\n\n", (whoseMove(position) == BLACK_PLAYER) ? "x's" : "o's");
}

void printRowOfStars() {
  int i;

  printf("\t");
  for (i = 0; i < WIDTH; i++) printf("   ");
  printf("\t-");
  for (i = 0; i < WIDTH; i++) printf("-----");
  printf("-\n");
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
  if (neighbors(getSourceFromMove(computersMove), getDestFromMove(computersMove)))
    printf("%s moved the piece at %d to %d.\n\n", computersName,
	   getSourceFromMove(computersMove), getDestFromMove(computersMove));
  else printf("%s captured the piece at %d with the piece from %d.\n\n",
	      computersName, getDestFromMove(computersMove), getSourceFromMove(computersMove));
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
  printf("[%d %d]", getSourceFromMove(move), getDestFromMove(move));
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
  /* move format: "xx yy" where xx is source and yy is dest (board is 10 to 99 squares)
   *              "xxx yyy" is board has 100 to 999 squares */

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
  int i, j = 0, k = 0, stringPos, source = 0, dest = 0;

  for (i = 0; input[i] != ' '; i++) {} 

  for (j = i-1; j >= 0; j--) {
    source += ((input[j]-'0') * exponent(10, k++));
  }

  i++;
  k = 0;
  stringPos = i;

  for (; i < strlen(input); i++) {}

  for (j = i-1; j >= stringPos; j--)
    dest += ((input[j]-'0') * exponent(10, k++));

  return makeMove(source, dest);
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

int oppositePlayer(int player) {
  if (player == BLACK_PLAYER) return WHITE_PLAYER;
  else return BLACK_PLAYER;
}

int numberOfPieces(POSITION position, int player) {
  int pieceCount, i;
  char currentPlayer;

  generic_unhash(position, gBoard);

  if (player == 1) {
    currentPlayer = 'x';
  } else {
    currentPlayer = 'o';
  }

  for (i = 0; i < WIDTH*HEIGHT; i++)
    if (gBoard[i] = currentPlayer) pieceCount++;

  return pieceCount;
}

int exponent(int base, int exponent) {
  int x;
  
  for (x = 1; exponent > 0; exponent--)
    x *= base;
  
  return x;
}

int getSourceFromMove(MOVE move) {
  return move / 1000;
}

int getDestFromMove(MOVE move) {
  return move % 1000;
}

MOVE makeMove(int source, int dest) {
  return (source*1000)+dest;
}

int neighbors(int x, int y) {
  if (x - 1 == y || x + 1 == y || x - WIDTH == y || x + WIDTH == y)
    return 1;
  else
    return 0;
}
