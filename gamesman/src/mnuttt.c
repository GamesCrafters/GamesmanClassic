// $id$
// $log$

/*
 * The above lines will include the name and log of the last person
 * to commit this file to CVS
 */

/************************************************************************
**
** NAME:        mnuttt.c
**
** DESCRIPTION: Nu Tic-Tac-Toe
**
** AUTHOR:      Guy Boo, Ming (Evan) Huang
**
** DATE:        Initial editing started on 2/5/05.
**              Ending date: unknown yet
**
** UPDATE HIST: legend: [+]: feature/new stuff, [-]: old stuff removed/deprecated, [*]comments
**
**              2/5/05: [+] initial commit, some game-specific consts,
**                          global variables for directions,
**                          IntitializeGame(), PrintPosition(), DoMove(), primitive(),
**                          printComputersMove (), Hash/unhash for moves, and other helpers.
**              2/6/05: [+] fixed some hardcoded constants, Position(), Row(), Column()
**                          now operates on an arbitrarily sized board.
**                          Easy job for getAndPrintUserInput().
**
**************************************************************************/


/* Some notes to Guy regarding how we are going to do this.
** It seems like the generic has function converts between POSITION (unsigned long)
** and a char array (char *). Why don't we use that and represent our board as a one dimentional
** array of chars (relevant defines are given already, see that section for details), and we will
** hash it into POSITION or vice versa when necessary.
** we will make helper functions to convert the piece's position in the board string to its row
** and column numbers using row() and column() functions. ALL INDICES START FROM 0.
** as for moves, I have written a hash function that takes in the row, column, and direction,
** and gives back a MOVE (int). The unhash function is more complicated. We have two of them
** for the originating position and the direction of the move. see the end of this file for details.
*/

/* TODO: find out how to keep track of current_player.
**       implement the rest of the functions.
**       some more stuff to put here....
*/


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

STRING   kGameName            = "Nu Tic-Tac-Toe"; /* The name of your game */
STRING   kAuthorName          = "Guy Boo and Ming (Evan) Huang"; /* Your name(s) */
STRING   kDBName              = "nuttt"; /* The name to store the database under */

BOOLEAN  kPartizan            = TRUE ; /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN  kGameSpecificMenu    = FALSE ; /* TRUE if there is a game specific menu. FALSE if there is not one. */

/*is there a tie in this?? I don't think so*/
BOOLEAN  kTieIsPossible       = FALSE ; /* TRUE if a tie is possible. FALSE if it is impossible.*/

BOOLEAN  kLoopy               = TRUE ; /* TRUE if the game tree will have cycles (a rearranger style game). FALSE if it does not.*/

BOOLEAN  kDebugMenu           = TRUE ; /* TRUE only when debugging. FALSE when on release. */
BOOLEAN  kDebugDetermineValue = TRUE ; /* TRUE only when debugging. FALSE when on release. */

/* each position consists choosing 8 boxes out of 20. that is 20*19*18*17...*13/8!=5079110400
** each choice has "8 choose 4" = 70 possible placements of X and O's.
** a combined 8817900 boards. This is not considering symmetry so the end result by init() may be less
*/
POSITION gNumberOfPositions   =  8817900; /* The number of total possible positions | If you are using our hash, this is given by the hash_init() function*/


POSITION gInitialPosition     =  0; /* The initial hashed position for your starting board */
POSITION kBadPosition         = -1; /* A position that will never be used */

/* 
 * Help strings that are pretty self-explanatory
 * Strings than span more than one line should have backslashes (\) at the end of the line.
 */

STRING kHelpGraphicInterface =
"Not written yet";

STRING   kHelpTextInterface    =
"On your turn, please enter three things: <row number> <column number> <direction>\n\
direction can be one of these things: up, down, left, right."; 

STRING   kHelpOnYourTurn =
"You should type: <row number> <column number> <direction, ie. one of \"up\" \"down\" \"left\" \"right\"";

STRING   kHelpStandardObjective =
"Get three of your pieces to form a straight line in any direction, including diagonal ones.";

STRING   kHelpReverseObjective =
"Prevent your opponent from lining up his/her pieces in a straight line, not even in the diagonal direction.";

STRING   kHelpTieOccursWhen =
"A tie occurs when the sun happens to rise from the west.";

STRING   kHelpExample =
"";


/*************************************************************************
**
** #defines and structs
**
**************************************************************************/
#define PLAYER1_PIECE 'X';
#define PLAYER2_PIECE 'O';
#define EMPTY_PIECE ' ';
/*this means that it is a BOARD_ROWS by BOARD_COLS gameboard.*/
#define BOARD_ROWS 5;
#define BOARD_COLS BOARD_ROWS-1;
#define PLAYER_PIECES BOARD_ROWS-1;
#define BOARD_SIZE BOARD_ROWS*BOARD_COLS;
#define EMPTY_PIECES BOARD_SIZE-PLAYER_PIECES;
#define INIT_PIECE_ARRAY {PLAYER1_PIECE, PLAYER_PIECES, PLAYER_PIECES, PLAYER2_PIECE, PLAYER_PIECES, PLAYER_PIECES, EMPTY_PIECE, EMPTY_PIECES, EMPTY_PIECES}

/*************************************************************************
**
** Global Variables
**
*************************************************************************/
/*the user input that corresponds to the direction in DIR_INCREMENTS*/
STRING directions[] {
  "up", "right", "down", "left"
}

/*the increments to the row and column numbers of the piece *
**0 = up, 1 = right, 2 = down, 3 = left*/
int dir_increments[][] {
  { -1, 0 } , { 0 , 1 } , { 1 , 0 } , { 0 , -1 }
}

char initial_board[BOARD_SIZE];

/*POSITION current_position;*/

/* what's the intialization value here? -- Evan*/
int current_player = PLAYER1_TURN;

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
    int i,j;
    gNumberOfPositions = generic_hash_init (BOARDSIZE, INIT_PIECE_ARRAY, NULL);
    /*initialize initial position*/
    for ( j = 0 ; j < BOARD_COLS ; j++)
      initial_board [Position (0, j)] = j%2 ? PLAYER1_PIECE : PLAYER2_PIECE;
    for ( i = 1 ; i < ( BOARD_ROWS - 1 ), i++ )
      for (j = 0; j < BOARD_COLS ; j++ )
	initial_board [Position (i,j)] = EMPTY_PIECE;
    for ( j = 0 ; j < BOARD_COLS ; j++ )
      initial_board [Position (BOARD_ROWS-1 ,j)] = j%2 ? PLAYER2_PIECE : PLAYER1_PIECE;
    gInitialPosition = generic_hash(initial_board, current_player);
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
       char* board = generic_unhash (position);
       int move_position = Unhasher_Position (move);
       int row = Row (move_position);
       int col = Column (move_position);
       int direction = Unhasher_Direction (move);
       int new_position = Postition (row + directions[direction], col + directions[direction]);
       board[new_position] = board[position];
       board[position] = EMPTY_PIECE;
       return generic_hash(board, current_player);
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

VALUE Primitive (POSITION position) /*pretty stupid but works*/
{
    char *board = generic_unhash (position);
    int i,j;
    /*check for horizontal ones*/
    for (i = 0; i < BOARD_ROWS; i++)
      for (j = 0; j < (BOARD_COLS-2); j++)
	if ((board[Position (i,j)] == board[Position (i,j+1)]) &&
	    (board[Position (i,j+1)] == board[Position (i,j+2)]))
	  return lose;
    /*check for vertical ones*/
    for (i = 0; i < (BOARD_ROWS-2); i++)
      for (j = 0; j < BOARD_COLS; j++)
	if ((board[Position (i,j)] == board[Position (i+1,j)]) &&
	    (board[Position (i+1,j)] == board[Position (i+2,j)]))
	  return lose;
    /*check for diagonal ones*/
    for (i = 0; i < (BOARD_ROWS-2); i++)
      for (j = 0; j < (BOARD_COLS-2); j++)
	if (
	    ((board[Position (i,j)] == board[Position (i+1,j+1)]) &&
	     (board[Position (i+1,j+1)] == board[Position (i+2,j+2)])) ||
	    ((board[Position (i, j+2)] == board[Position (i+1,j+1)]) &&
	     (board[Position (i+1, j+1)] == board[Position (i+2, j)]))
	   )
	  return lose;
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
    char *board = generic_unhash (position);
    int i,j;
    printf (" The game board as %s sees it:\n", playersName);
    printf ("   0 1 2 3\n");
    printf ("  +-+-+-+-+\n");
    for (i = 0; i <= BOARD_ROWS; i++) {
      printf ( "%d " , i );
      for (j = 0; j <= BOARD_COLS; j++) 
	printf ("|%c", board[Position(i,j)]);
      printf("|\n");
      printf ("  +-+-+-+-+\n");
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
    int position = Unhasher_Position (computersMove);
    int direction = Unhasher_Direction (computersMove);
    printf ("The computer picks up the piece in (%d,%d) and moves it %s\n", Row (position), Column (position), directions[direction]);
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
/*is this the users move?????*/
void PrintMove (MOVE move)
{

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
	printf("%8s's move [(undo)/<row> <col> <dir>] : ", playersName);
	
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

int Position (int row, int col) {
  return  (row*BOARD_COLS+col);
}

int Row (int Pos) {
  return (Pos/BOARD_ROWS);
}

int Column (int Pos) {
  return (Pos%BOARD_COLS);
}

MOVE Hasher (int row, int col, int dir) {
  return (MOVE) ( ( Position (row, col) )* 4 + dir);
}

int Unhasher_Position (MOVE hashed_move) {
  return (hashed_move/4);
}

int Unhasher_Direction (MOVE hashed_move) {
  return (hashed_move%4);
}
