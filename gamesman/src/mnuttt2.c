// Guy Boo
// grenrock

/* the above two lines contains the name and login of the last person to edit
 * the file
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
** UPDATE HIST: legend: [+]: feature/additions/revisions,
**                      [-]: old stuff removed/deprecated,
**                      [*]: comments
**
**            02/05/05: [+] initial commit, some game-specific consts,
**                          global variables for directions, IntitializeGame(),
**                          PrintPosition(), DoMove(), primitive(),
**                          printComputersMove (), Hash/unhash for moves,
**                          and other helpers.
**            02/07/05: [*] guy's typical shortsighted and egocentric complete
**                          revision of the entire file. (yeah, yeah...)
**                      [+] basically, redefined all the necessary functions
**                          and constants except GetInitialPosition() and
**                          GetAndPrintPlayersMove() which still require their
**                          initial definitions.
**                      [-] replaced all of the original helper functions,
**                          partially because they were designed to run on the
**                          preliminary format which didn't have any support
**                          for undoing moves, partially because they weren't
**                          sufficiently general enough for my tastes, and
**                          partially because i'm more comfortable with the
**                          operation of the algorithms this way. Despite this,
**                          i'm very sure that i must have forgotten somewhere
**                          to pass them a pointer and not a variable.
**            02/08/05: [*] Committed this version into CVS.
**                      [+] All necessary functions have been implemented.
**                          Work out the quirks and we are ready for prime time
**            02/09/05: [*] Corrected some indexing and pointer problems.
**                      [+] Implemented GetAndPrintPlayersMove correctly and
**                          hotwired GetInitialPosition
**
**************************************************************************/


/* TODO:
**    implement the rest of the functions.
**        ayup. preferably by next wednesday. - I thought we were done.
**    some more stuff to put here....
**        >> evan <<
**        Please double check the converstions between (x,y) and the index
**        in the vector. I think we should not think of them as x, y
**        corrdinates but uniformly as row number and column number.
**        so index = row*number of columns + col.
**        Some unnecessary checks here and there, but doesn't hurt.
**        if we get this version to work faster, I will take down my
**        version and make this ours.
**        >> guy <<
**        aren't the two ways of thinking about indexing them equivalent?
**        (1)y iterates over all of the rows, hence it is the row number.
**        (2)x iterates over all of the columns, hence... etc.
**        what do you mean by "unnecessary checks"?
**        what do you mean "work faster"?
*/

/* FORMATS:
 * POSITIONs that come out of here must encode the entire game state, including
 * the board layout and whose turn it is.  so if i move my x one space down, i
 * return a position representing that and the fact that it's o's turn.
 * MOVEs need to encode anything having to do with a player's move, but note
 * that they do not (necessarily) encode who performs it.
 *
 * from studying mjoust.c, it appears that both formats are arbitrary and left
 * entirely up to us. note, though, that the definition implies that
 * maintaining state is futile - we're only defining the rulepacks.
 * redefinition of MOVE:
 *     fx,fy,tx,ty - ie use different base combination as discussed at last
 *                   meeting. this is for undo compatibility, and to not
 *                   restrict boardsize or move types.
 * redefinition of POSITION:
 *     (blank=0, x=1, o=2) * 3^(row+column*numRows)+(player=x?1:0)<<31;
 *     don't know if there's a better way to do this... so far this takes all
 *     32 bits for a 20 square board.  on the plus side it doesn't take any
 *     more space if we decide to allow captures.
 * EVAN - this is definition of POSITION used anywhere in the code? cuz I don't see
 *        it. I thought we were gonna use generic_hash...
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

STRING   kGameName            = "Nu Tic-Tac-Toe";
STRING   kAuthorName          = "Guy Boo and Ming (Evan) Huang";
STRING   kDBName              = "nuttt"; /* The name of the stored database */

BOOLEAN  kPartizan            = TRUE ; /* i'm not completely sure about this */
BOOLEAN  kGameSpecificMenu    = FALSE ;
BOOLEAN  kTieIsPossible       = FALSE ;
BOOLEAN  kLoopy               = TRUE ;
/* the following two vars should be TRUE only during debugging. */
BOOLEAN  kDebugMenu           = TRUE ;
BOOLEAN  kDebugDetermineValue = TRUE ;

/* as we discussed, the total number of positions is 20!/(12!*4!*4!), which
 * equals 8,817,900.  this will be fine for generic hash.  If we ever wish to
 * include a variant that allows for capturing, then that number will get much
 * larger, but we don't have to worry about that for now.
 */
POSITION gNumberOfPositions   =  0; /* The number of total possible positions | If you are using our hash, this is given by the generic_hash_init() function*/

POSITION gInitialPosition     =  0; /* The initial hashed position for your starting board */
POSITION kBadPosition         = -1; /* A position that will never be used */

/* 
 * Help strings that are pretty self-explanatory
 * Strings that span more than one line should have backslashes (\) at the end of the line.
 */

STRING kHelpGraphicInterface = ""; /* kSupportsGraphics == FALSE */
/* should probably be changed to north/south interface if ever generalize
 * move types
 */
STRING   kHelpTextInterface =
  "On your turn, enter the xy coordinates of the piece you'd like to move\n\
and the direction you wish to move it. Enter your move in the format\n\
<column> <row> <direction> where the direction is any one of 'up', 'down',\n\
'left', or 'right'.  For example, to move the piece in column 3 and row 2 up\n\
one square, you would enter '3 2 up'.";

STRING   kHelpOnYourTurn =
"You should type: <row number> <column number> <direction>, ie. one of \n\
\"up\" \"down\" \"left\" \"right\"";

STRING   kHelpStandardObjective =
  "Get any three of your pieces to form a horizontal, diagonal, or vertical\n\
line.";

STRING   kHelpReverseObjective =
  "Force your opponent to arrange three pieces in a horizontal, diagonal,\n\
or vertical line.";

STRING   kHelpTieOccursWhen = ""; /* kTieIsPossible == FALSE */

STRING   kHelpExample = "coming soon! buy viagra in the meantime!";


/*************************************************************************
**
** #defines and structs
**
**************************************************************************/
#define PLAYER1_PIECE 'X';
#define PLAYER2_PIECE 'O';
#define EMPTY_PIECE ' ';
int BOARD_ROWS = 5;
int BOARD_COLS = 4;
int PLAYER_PIECES = 4; /* BOARD_COLS */
/* should enforce this stronger by hardcoding BOARD_COLS? not an issue unless
 * consider allowing debugging to alter this.
 */
int NUM_TO_WIN = 3; /* how many pieces must be in the line */
int BOARD_SIZE = 20; /* BOARD_ROWS*BOARD_COLS */

/*************************************************************************
**
** Global Variables
**
*************************************************************************/
/* want to be able to change these so they are more universal, but that's for
 * later.
 */

/* the user input that corresponds to the direction in DIR_INCREMENTS */
/* changed this from strings to chars to deal with difficulties of character
 * input.
 */
const STRING directions[] = {"up", "right", "down", "left"};

/* 0 = up, 1 = right, 2 = down, 3 = left */
const int dir_increments[][] = {{0,1} , {1,0} , {0,-1} , {-1,0}};

/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/

/* External */
extern GENERIC_PTR	SafeMalloc ();
extern void		SafeFree ();
extern int generic_hash_init();
extern int generic_hash();
extern int whoseMove();
extern char* generic_unhash();
/* Internal */
void InitializeGame();
MOVELIST *GenerateMoves(POSITION position);
POSITION DoMove (POSITION position, MOVE move);
VALUE Primitive (POSITION position);
void PrintPosition(POSITION position, STRING playersName, BOOLEAN usersTurn);
void printBoard(char board[]);
void PrintComputersMove(MOVE computersMove, STRING computersName);
void PrintMove(MOVE move);
USERINPUT GetAndPrintPlayersMove (POSITION position, MOVE *move, STRING playersName);
BOOLEAN ValidTextInput(STRING input);
MOVE ConvertTextInputToMove(STRING input);
void GameSpecificMenu();
void SetTclCGameSpecificOptions(int options[]);
POSITION GetInitialPosition();
BOOLEAN GetInitPosHelper (char playerchar);
int NumberOfOptions();
int getOption();
void setOption(int option);
void DebugMenu();
/* helper */
int CoordsToIndex(int x, int y);
int* IndexToCoords(int index, int[] coords);
MOVE CoordsToMove(int from_x, int from_y, int to_x, int to_y);
int * MoveToCoords(MOVE hashed_move, int[] coords);
int *MoveToIndex(MOVE move, int[] index);
MOVE IndexToMove (int from_index, int to_index);
int *initializePiecesArray(int p_a[]);
int* initializeBoard(int board[]);

/*************************************************************************
**
** Global Database Declaration
**
**************************************************************************/

extern VALUE *gDatabase;


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
  int x,y;
  char piece;
  char board[BOARD_ROWS*BOARD_COLS];
  int player = 1;
  int p_a[10];
  BOARD_SIZE = BOARD_ROWS*BOARD_COLS;
  PLAYER_PIECES = BOARD_COLS;
  /* so far can't imagine a circumstance in which i'd want to initialize the
   * board externally, but if i ever do, the methods are at the very bottom.
  initializePiecesArray(pieces_array);
  initializeBoard(board);

  int pieces_array[] = {PLAYER1_PIECE, PLAYER_PIECES, PLAYER_PIECES,
			PLAYER2_PIECE, PLAYER_PIECES, PLAYER_PIECES,
			EMPTY_PIECE,   BOARD_SIZE-2*PLAYER_PIECES,
			BOARD_SIZE-2*PLAYER_PIECES, -1};
  */
  p_a[0] = PLAYER1_PIECE;
  p_a[3] = PLAYER2_PIECE;
  p_a[6] = EMPTY_PIECE;
  p_a[1] = p_a[2] = p_a[4] = p_a[5] = PLAYER_PIECES;
  p_a[7] = p_a[8] = BOARD_SIZE-2*PLAYER_PIECES;
  p_a[9] = -1;

  for (x = 0; x < BOARD_COLS; x++) {
    for (y = 0; y < BOARD_ROWS; y++) {
      if (y == 0) {
	if ((x%2) == 0) {
	  piece = PLAYER1_PIECE;
	} else {
	  piece = PLAYER2_PIECE;
	} 
      } else if (y == BOARD_ROWS-1) {
	if ((x%2) == 0) {
	  piece = PLAYER2_PIECE;
	} else {
	  piece = PLAYER1_PIECE;
	}
      } else {
	piece = EMPTY_PIECE;
      }
      board[CoordsToIndex(x,y)] = piece;
    }
  }
  gNumberOfPositions = generic_hash_init(BOARD_SIZE, p_a, NULL);
  gInitialPosition = generic_hash(board, player);
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
  extern const int dir_increments[][];
  MOVELIST *moves = NULL;
  int x, y, i, dx, dy;
  char board[BOARD_SIZE];
  int player = whoseMove(position);
  generic_unhash (position, board);

  /* Use CreateMovelistNode(move, next) to 'cons' together a linked list */
  for (x = 0; x < BOARD_COLS; x++) {
    for (y = 0; y < BOARD_ROWS; y++) {
      if ((player==1) && (board[CoordsToIndex(x,y)] == PLAYER1_PIECE)) {
	for (i = 0; i < 4; i++) {
	  // for every possible movement as specified in dir_increments
	  dx = x + dir_increments[i][0];
	  dy = y + dir_increments[i][1];
	  if ((dx >= 0) && (dy >= 0) &&	\
	      (dy < BOARD_ROWS) && (dx < BOARD_COLS) && \
	      (board[CoordsToIndex(dx,dy)] == EMPTY_PIECE)) {
	    moves = CreateMovelistNode(CoordsToMove(x,y,dx,dy),moves);
	  }
	}
      } else if ((player==2) && (board[CoordsToIndex(x,y)] == PLAYER2_PIECE)) {
	for (i = 0; i < 4; i++) {
	  // for every possible movement as specified in dir_increments
	  dx = x + dir_increments[i][0];
	  dy = y + dir_increments[i][1];
	  if ((dx >= 0) && (dy >= 0) && \
	      (dy < BOARD_ROWS) && (dx < BOARD_COLS) && \
	      (board[CoordsToIndex(dx,dy)] == EMPTY_PIECE)) {
	    moves = CreateMovelistNode(CoordsToMove(x,y,dx,dy),moves);
	  }
	}
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
  char board[BOARD_SIZE];
  int index[2];
  int player = whoseMove(position);
  generic_unhash(position, board);
  MoveToCoords(move, index);
  board[index[0]] = EMPTY_PIECE;
  if (player == 1) {
    board[index[1]] = PLAYER1_PIECE;
    return generic_hash(board, 2);
  } else {
    board[index[1]] = PLAYER2_PIECE;
    return generic_hash(board, 1);
  }
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
  /* if it's your turn but there's already a win, then you must have lost!
   * can think about it this way because players are only allowed to move their
   * own pieces.
   */
  char board[BOARD_SIZE];
  int x,y,d;
  generic_unhash (position, board);
  for (y = 0; y < BOARD_ROWS+1-NUM_TO_WIN; y++) {
    for (x = 0; x < BOARD_COLS+1-NUM_TO_WIN; x++) {
      if (board[CoordsToIndex(x,y)] != EMPTY_PIECE) {
	for (d = 1; d < NUM_TO_WIN; d++) {
	  if (board[CoordsToIndex(x+d-1,y)] != board[CoordsToIndex(x+d,y)]) {
	    break;
	  }
	}
	if (d == NUM_TO_WIN) return (gStandardGame ? lose : win);
	for (d = 1; d < NUM_TO_WIN; d++) {
	  if (board[CoordsToIndex(x,y+d-1)] != board[CoordsToIndex(x,y+d)]) {
	    break;
	  }
	}
	if (d == NUM_TO_WIN) return (gStandardGame ? lose : win);
	for (d = 1; d < NUM_TO_WIN; d++) {
	  if (board[CoordsToIndex(x+d-1,y+d-1)] != \
	      board[CoordsToIndex(x+d,y+d)]) {
	    break;
	  }
	}
	if (d == NUM_TO_WIN) return (gStandardGame ? lose : win);
      }
      if (board[CoordsToIndex(x,y+NUM_TO_WIN-1)] != EMPTY_PIECE) {
	for (d = 1; d < NUM_TO_WIN; d++) {
	  if (board[CoordsToIndex(x+d-1,y+NUM_TO_WIN-d)] != \
	      board[CoordsToIndex(x+d,y+NUM_TO_WIN-d-1)]) {
	    break;
	  }
	}
	if (d == NUM_TO_WIN) return (gStandardGame ? lose : win);
      }
    }
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
  char board[BOARD_SIZE];
  generic_unhash (position, board);
  printf (" The game board as %s sees it:\n  +", playersName);
  printBoard(board);
}

void printBoard(char board[]) {
  /* this is not the most efficient implementation.  better to structure the
   * board so that traverse columns left to right and rows top to bottom.
   * that way can just have one for loop.  so far x is fine, but y must be
   * inverted.  best part is entire program relies on CoordsToIndex, so only
   * need to change the helper functions.
   */
  int x,y;
  printf("  +");
  for (x = 0; x < BOARD_COLS; x++) printf("-+");
  for (y = BOARD_ROWS-1; y >= 0; y--) {
    /* printing the board top-down */
    printf ( "\n%d |" , y );
    for (x = 0; x < BOARD_COLS; x++) printf ("%c|", board[CoordsToIndex(x,y)]);
    printf ("\n  +");
    for (x = 0; x < BOARD_COLS; x++) printf("-+");
  }
  printf("\n  ");
  for (x = 0; x < BOARD_COLS; x++)
    printf(" %d",x);
  printf("\n");
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
    int coords[4];
    MoveToCoords(computersMove, coords);
    printf("%8s moved (%2d,%2d) to (%2d,%2d)\n\n ",computersName, coords[0], \
	   coords[1], coords[2], coords[3]);
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
  int xy[4];
  MoveToCoords(move, xy);
  printf("from (%2d,%2d) to (%2d,%2d)", xy[0], xy[1], xy[2], xy[3]);
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
    
    while (TRUE) {
        /***********************************************************
         * CHANGE THE LINE BELOW TO MATCH YOUR MOVE FORMAT
         ***********************************************************/
	printf("%8s's move [(undo)/([0-%d][0-%d] [up,down,left,right])] : ", \
	       playersName, BOARD_COLS-1, BOARD_ROWS-1);
	
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

/* if ever expand the possible moves, may want to change this to NWSE version.
 */
BOOLEAN ValidTextInput (STRING input)
{
  extern const STRING directions[];
  int x,y,i;
  char dir[strlen(input)];
  sscanf(input, "%d %d %s",&x,&y,dir);
  if ((x >= 0) && (x < BOARD_COLS) && (y >= 0) && (y < BOARD_ROWS)) {
    for (i = 0; i < 4; i++) {
      // for every string in directions
      if (strcmp(directions[i],dir) == 0) return TRUE;
    }
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
  extern const STRING directions[];
  extern const int dir_increments[][];
  int x,y,i;
  char dir[strlen(input)];
  sscanf(input, "%d %d %s",&x,&y,dir);
  for (i = 0; i < 4; i++) {
    // for every string in directions
    if (strcmp(directions[i], dir) == 0) {
      return CoordsToMove(x,y,x+dir_increments[i][0],y+dir_increments[i][1]);
    }
  }
  printf("ERROR - CONVERT TEXT INPUT TO MOVE CAUGHT BAD MOVE");
  return kBadPosition;
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
  char board[BOARD_SIZE];
  int i;
  const int player = 1;
  char input[80];
  BOOLEAN setboard = TRUE;
  BOOLEAN tryagain = TRUE;

  while (tryagain) {
    for (i = 0; i < BOARD_SIZE; i++) {
      board[i] = EMPTY_PIECE;
    }
    printf("please enter board locations for the pieces in 'x y' format.\n");
    printf("for example, to place a piece at (0,1) you'd enter '0 1'.\n");
    printf("enter 'quit' or 'exit' to leave this function and return to\n");
    printf("the default initial board.\n");
    if (!(setboard = tryagain = GetInitPosHelper(board, PLAYER1_PIECE))) break;
    if (!(setboard = tryagain = GetInitPosHelper(board, PLAYER2_PIECE))) break;
    while (TRUE) {
      printf("is this the board you want? (y/n)");
      printBoard(board);
      gets(input);
      if ((strcmp(input,"y") == 0) || (strcmp(input,"yes") == 0)) {
	setboard = TRUE;
	tryagain = FALSE;
	break;
      } else if ((strcmp(input,"n") == 0) || (strcmp(input,"no") == 0)) {
	setboard = FALSE;
	tryagain = TRUE;
	break;
      } else {
	printf("error. enter 'y', 'yes', 'n', or 'no'.\n");
      }
    }
  }
  if (setboard) gInitialPosition = generic_hash(board, player);
  return gInitialPosition;
}

BOOLEAN GetInitPosHelper (char board[], char playerchar) {
  int i,x,y;
  char input[80];
  for (i = 0; i < PLAYER_PIECES; i++) {
    printf("enter the location of %c piece %d: ",playerchar,i+1);
    gets(input);
    if ((strcmp(input,"quit") == 0) || (strcmp(input,"exit") == 0)) {
      return FALSE;
    } else {
      sscanf(input,"%d %d",x,y);
      if ((x < 0) || (y < 0) || (x >= BOARD_COLS) || (y >= BOARD_ROWS)) {
	printf("error - off board.");
	i--;
	continue;
      } else if (board[CoordsToIndex(x,y)] != EMPTY_PIECE) {
	printf("error - already occupied.");
	i--;
	continue;
      } else {
	board[CoordsToIndex(x,y)] = playerchar;
      }
    }
  }
  return TRUE;
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
    return 2;
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
/* possible variants:
 *   misere (2 options) - implemented
 *   push one/push many (3 options)
 *   move one/move many (2 options)
 *   move style: rook/bishop (3 options)
 *   36 options total.
 */
int getOption ()
{
  if (gStandardGame) return 1;
    return 2;
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
  gStandardGame = (option == 1) ? TRUE : FALSE;
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

/* EVAN: once these functions are in place, feel free to modify them any
 * way you want. (split IndexToCoords into an x and y variant for example)
 * just make sure they fulfill their basic contract to answer meaningfully.
 */

int CoordsToIndex (int x, int y) {
  return  (x*BOARD_ROWS+y);
}

/* remember to call with a pointer! */
int* IndexToCoords (int index, int coords[]) {
  coords[0] = index/BOARD_ROWS;
  coords[1] = index%BOARD_ROWS;
  return coords;
}

/* depracated
MOVE EncodeMove (int from_x, int from_y, int to_x, int to_y) {
*/
MOVE CoordsToMove (int from_x, int from_y, int to_x, int to_y) {
  return (MOVE) ( ( (from_x*BOARD_ROWS+from_y) *BOARD_COLS+to_x)
		  *BOARD_ROWS + to_y);
}

/* why do i use board rows and columns directly?  why don't i use
 * CoordsToIndex and board size?
 */
/* depracated
int* DecodeMove(MOVE hashed_move, int[] coords) {
*/
int* MoveToCoords (MOVE hashed_move, int coords[]) {
  coords[0] = hashed_move/(BOARD_ROWS*BOARD_COLS*BOARD_ROWS); /* fromx */
  coords[1] = (hashed_move/(BOARD_ROWS*BOARD_COLS))%BOARD_ROWS; /* fromy */
  coords[2] = (hashed_move/BOARD_ROWS)%BOARD_COLS; /* tox */
  coords[3] = hashed_move % BOARD_ROWS; /* toy */
  return coords;
}

/* based on how i'm using it, it looks like it'd be more useful and less buggy
 * to have functions that transfer from coords to indices and back.  as in,
 * don't have Encode and Decode Move.
 */

int* MoveToIndex (MOVE move, int index[]) {
  index[0] = move/BOARD_SIZE;
  index[1] = move%BOARD_SIZE;
  return index;
}

MOVE IndexToMove (int from_index, int to_index) {
  return from_index*BOARD_SIZE+to_index;
}

/* not used.  yet.
 */
int* initializePiecesArray(int p_a[]) {
  p_a[0] = PLAYER1_PIECE;
  p_a[3] = PLAYER2_PIECE;
  p_a[6] = EMPTY_PIECE;
  p_a[9] = -1;
  p_a[1] = p_a[2] = p_a[4] = p_a[5] = PLAYER_PIECES;
  p_a[7] = p_a[8] = BOARD_SIZE-2*PLAYER_PIECES;
  return p_a;
}

int* initializeBoard (int board[]) {
  int x,y;
  char piece;
  for (x = 0; x < BOARD_COLS; x++) {
    for (y = 0; y < BOARD_ROWS; y++) {
      if (y == 0) {
	if ((x%2) == 0) {
	  piece = PLAYER1_PIECE;
	} else {
	  piece = PLAYER2_PIECE;
	} 
      } else if (y == BOARD_ROWS-1) {
	if ((x%2) == 0) {
	  piece = PLAYER2_PIECE;
	} else {
	  piece = PLAYER1_PIECE;
	}
      } else {
	piece = EMPTY_PIECE;
      }
      board[CoordsToIndex(x,y)] = piece;
    }
  }
  return board;
}
