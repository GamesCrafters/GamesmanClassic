/************************************************************************
**
** NAME:        mttc.c
**
** DESCRIPTION: Tic-Tac-Chec
**
** AUTHOR:      Reman Child, Johnathon Tsai (is he still working on this????)
**              University of California at Berkeley
**              Copyright (C) 2004. All rights reserved.
**
** FILE HISTORY:
**
** 2004.3.30    First compilation of subroutines into mttc.c; this update
**              basically includes all functions below except for 
**              ttc_hash and ttc_unhash, which still needs to be written
**                                                                -- rc
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

POSITION gNumberOfPositions  = 2000000; /* Hmmm I made this number up */

POSITION gInitialPosition    = 0; 
POSITION gMinimalPosition    = 0; 
POSITION kBadPosition        = -1; /* need to change once create hash stuff */

STRING   kGameName           = "Tic-Tac-Chec"; 
STRING   kDBName             = "mttc";
BOOLEAN  kPartizan           = TRUE;
BOOLEAN  kDebugMenu          = FALSE;
BOOLEAN  kGameSpecificMenu   = FALSE;
BOOLEAN  kTieIsPossible      = FALSE;
BOOLEAN  kLoopy              = TRUE; 
BOOLEAN  kDebugDetermineValue = FALSE ;

/* 
   Help strings that are pretty self-explanatory 
*/

STRING kHelpGraphicInterface =
"Not written yet";

STRING   kHelpTextInterface    =
"Not written yet";

STRING   kHelpOnYourTurn =
"Not written yet";

STRING   kHelpStandardObjective =
"Not written yet";

STRING   kHelpReverseObjective =
"Not written yet";

STRING   kHelpTieOccursWhen = /* Should follow 'A Tie occurs when... */
"Not written yet";

STRING   kHelpExample =
"Not written yet";

/*************************************************************************
**
** Everything above here must be in every game file
**
**************************************************************************/

/*************************************************************************
**
** Game-specific DEFINES
**
*************************************************************************/

#define BOARD_LENGTH  16
#define BOARD_WIDTH   4
#define BOARD_HEIGHT  4
#define NUM_PIECES    8
#define MOVE_LENGTH   3
#define WIN_CONDITION 4

#define MOVE_PIECE    0
#define MOVE_ROW      1
#define MOVE_COL      2

#define N_LONG_LEG    2
#define N_SHORT_LEG   1

#define OFF           31 

#define max(a,b) ((a > b)? a : b)
#define min(a,b) ((a > b)? b : a)
 
/*************************************************************************
**
** Type Definitions 
**
*************************************************************************/

typedef int* BOARD;
typedef int PIECE;
typedef int PLAYER;
typedef int CELL;

/*************************************************************************
**
** Enumerations
**
*************************************************************************/

enum pieces { B_QN, B_RK, B_KN, B_PN, W_QN, W_RK, W_KN, W_PN, BLNK };
enum players {BLACK, WHITE};

/*************************************************************************
**
** Global Arrays
**
*************************************************************************/

char piece_names[] = {'Q','R','N','P','q','r','n','p','-'};
char row_names[] = {'a','b','c','d','e','f','g','h'};

/*************************************************************************
**
** Function Prototypes:
**
**************************************************************************/

/* External */
extern GENERIC_PTR	SafeMalloc ();
extern void		SafeFree ();

/* Internal */
int *ttc_unhash(POSITION pos);
POSITION ttc_hash(BOARD board, PLAYER player);


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
** CALLS:       ttc_hash ()
**              ttc_unhash ()
**	           
*************************************************************************/

POSITION DoMove (POSITION thePosition, MOVE theMove) {
  return 0;
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

POSITION GetInitialPosition() {
  char in_string[MOVE_LENGTH];
  POSITION pos = 0;
  while(TRUE) {
    PrintPosition(pos,"",0);
    printf("Input a valid move (? for help): ");
    scanf("%s",in_string);
    if (in_string[0] == 'e')
      break;
    if (in_string[0] == '?') {
      printf("\n\n\n\n");
      printf("##################################################\n");
      printf("- Move format: [Piece][Destination]\n");
      printf("   i.e. Qa3 to move the Black queen to a3\n");
      printf("\n - Special Moves: 'end' to finish, '?' for help\n");
      printf("##################################################\n");
      continue;
    }
    if (ValidTextInput(in_string)) {
      pos = DoMove(pos,ConvertTextInputToMove(in_string));
    } else {
      printf("Invalid move\n");
    }
  }
  return pos;
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

void PrintComputersMove(MOVE computersMove, STRING computersName) {
  char piece = piece_names[get_piece(computersMove)];
  char row = row_names[get_dest(computersMove) % BOARD_WIDTH];
  int col = get_dest(computersMove) / BOARD_WIDTH;
  printf("%s's move: %c%c%d\n", computersName, piece, row, col);
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

/** Do I need to code in whosemove to this function? **/
VALUE Primitive (POSITION pos) {
  BOARD board, get_board(POSITION);
  int i, num_in_row(int,BOARD);
  board = get_board(pos);
  for (i = 0; i < BOARD_LENGTH; i++) {
    if (num_in_row(i,board) >= WIN_CONDITION)
      return lose;
  }
  return undecided;
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

/* Very minimal at the moment; need to make prettier */
void PrintPosition (position, playerName, usersTurn)
	POSITION position;
	STRING playerName;
	BOOLEAN usersTurn;
{
  BOARD board, get_board(POSITION);
  board = get_board(position);
  int i;
  for(i = 0; i < BOARD_LENGTH; i++) {
    if(i % BOARD_WIDTH == 0)
      printf("\n");
    printf("%c | ", piece_names[board[i]]);
  }
  printf("\n");
  printf("\n\n {r,p,q,k} => white pieces  {R,P,Q,K} => black pieces \n\n");
  return;
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

MOVELIST *GenerateMoves(POSITION pos) {
  BOARD get_board(POSITION), board;
  MOVELIST *concat_ml(MOVELIST *first, MOVELIST *sec);
  BOOLEAN is_knight(PIECE), is_queen(PIECE), is_rook(PIECE);
  BOOLEAN is_bishop(PIECE), is_pawn(PIECE);
  MOVELIST *gen_pawn_moves(CELL cell, int *board, PLAYER player);
  MOVELIST *gen_knight_moves(CELL cell, int *board, PLAYER player);
  MOVELIST *gen_rook_moves(CELL cell, int *board, PLAYER player);
  MOVELIST *gen_bishop_moves(CELL cell, int *board, PLAYER player);
  MOVELIST *gen_queen_moves(CELL cell, int *board, PLAYER player);
  int player, piece_flags[NUM_PIECES], i, j;
  MOVELIST *head = NULL;
  player = get_player(pos);
  board = get_board(pos);
  for(i = 0; i < NUM_PIECES; i++)
    piece_flags[i] = OFF;
  for(i = 0; i < BOARD_LENGTH; i++)
    if (board[i] != BLNK)
      piece_flags[board[i]] = i;

  for(i = 0; i < NUM_PIECES; i++) {
    /* Moves to place pieces on the board */
    if (piece_flags[i] == OFF)
      for(j = 0; j < BOARD_LENGTH; j++) {
	if (board[j] == BLNK)
	  head = CreateMovelistNode(make_move(i,j), head);
      }
    /* Moves by pieces already on the board*/
    else {
      if (is_knight(i))
	head = concat_ml(gen_knight_moves(piece_flags[i],board, player),head);
      if (is_queen(i))
	head = concat_ml(gen_queen_moves(piece_flags[i], board, player), head);
      if (is_rook(i))
	head = concat_ml(gen_rook_moves(piece_flags[i], board, player), head);
      if (is_pawn(i))
	head = concat_ml(gen_pawn_moves(piece_flags[i], board, player), head);
      if (is_bishop(i))
	head = concat_ml(gen_bishop_moves(piece_flags[i],board, player), head);
    }
  }
  return head;
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

BOOLEAN ValidTextInput (STRING input) {
  BOOLEAN first = FALSE, second = FALSE, third = FALSE, end = FALSE;
  int i;
  for(i = 0; i < NUM_PIECES; i++) {
    if (input[MOVE_PIECE] != piece_names[i])
      continue;
    first = TRUE;
    break;
  }
  for(i = 0; i < BOARD_HEIGHT; i++) {
    if (input[MOVE_ROW] != row_names[i])
      continue;
    second = TRUE;
    break;
  }
  third = (input[MOVE_COL] - '0') > 0 && (input[2] - '0') <= BOARD_WIDTH;
  end = input[3] == '\0';
  return first && second && third && end;
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
**              i.e. 'Qa3'
**
** OUTPUTS:     MOVE : The move corresponding to the user's input.
**              i.e. piece << 4 + cell == 4 * 2^4 + 3
**
************************************************************************/

MOVE ConvertTextInputToMove (STRING input) {
  /* lowest 4 bits is the cell, next 3 bits is the piece */
  int cell = 0, i;
  for(i = 0; i < BOARD_HEIGHT; i++)
    if (row_names[i] == input[MOVE_ROW])
      cell = i * BOARD_WIDTH + input[MOVE_COL];
  return input[MOVE_PIECE] << 4 + cell;
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

void PrintMove (MOVE move) {
  char piece = piece_names[get_piece(move)];
  char row = row_names[get_dest(move) % BOARD_WIDTH];
  int col = get_dest(move) / BOARD_WIDTH;
  printf("%c%c%d", piece, row, col);
}

/************************************************************************
**
** NAME:        NumberOfOptions
**
** DESCRIPTION: Calculates and returns the number of option combinations
**		there are with all the game variations you program.
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
**		Should return a different number for each set of
**		variants.
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
**		Should take the input and set all the appropriate
**		variants.
**
** INPUT:     int : the number representation of the options.
**
************************************************************************/

void setOption(int option)
{
  return;
}


/************************************************************************
**
** NAME:        GameSpecificTclInit
**
** DESCRIPTION: 
**
************************************************************************/

int GameSpecificTclInit (interp, mainWindow) 
	Tcl_Interp* interp;
	Tk_Window mainWindow;
{
  return 0;
}


/************************************************************************
*************************************************************************
**         EVERYTHING BELOW THESE LINES IS LOCAL TO THIS FILE
*************************************************************************
************************************************************************/

/** Utility functions ****************************************************/

/* Relies on piece_names being accurate */
MOVE make_move(PIECE piece, CELL cell) {
  return piece << 4 + cell;
}

/* Concatenates two movelists into one larger movelist */
MOVELIST *concat_ml(MOVELIST *first, MOVELIST *sec) {
  if (first == NULL)
    return sec;
  if (sec == NULL) 
    return first;
  MOVELIST *temp;
  while(!(temp = first->next))
    ;
  temp->next = sec;
  return first;
}

BOOLEAN is_occupied(CELL cell, int *board) {
  return board[cell] != BLNK;
}

BOOLEAN ally_occupies(PLAYER player, CELL cell, int *board) {
  return board[cell] != BLNK && player == which_player(cell,board);
}

BOOLEAN enemy_occupies(PLAYER player, CELL cell, int *board) {
  return board[cell] != BLNK && player != which_player(cell,board);
}

/** Position Manipulation functions *************************************/

BOARD get_board(POSITION pos) {
  BOARD board;
  board = (int *) malloc(BOARD_LENGTH * sizeof(int));
  /* Have to make some call to ttc_unhash */
  return board;
}

PLAYER get_player(POSITION pos) {
  PLAYER player;
  /* make some call to ttc_unash */
  return player;
}

/** Identity Functions **************************************************/

/** requires first half of pieces are black, second half white  **/
BOOLEAN is_white(PIECE piece) {
  return piece >= NUM_PIECES/2 && piece < NUM_PIECES;
}

/** requires first half of pieces are black, second half white  **/
BOOLEAN is_black(PIECE piece) {
  return piece < NUM_PIECES/2;
}

/** Given two board locations and a board, tells if the two on are the 
    same color **/
BOOLEAN same_color(int first, int second, BOARD board) {
  return (is_white(board[first]) && is_white(board[second])) ||
    (is_black(board[first]) && is_black(board[second]));
}

/** Returns the opponent **/
PLAYER opponent(PLAYER player) {
  return (is_white(player))? BLACK : WHITE;
}

/* Given a board and cell, determines the player that occuppies the cell */
PLAYER which_player(CELL cell, int *board) {
  /* Assumes BLACK has first half of pieces, WHITE second half */
  return (board[cell] < NUM_PIECES/2)? BLACK : WHITE;
}

BOOLEAN is_knight(PIECE piece) {
  return (piece_names[piece] == 'N' || piece_names[piece] == 'n');
}

BOOLEAN is_queen(PIECE piece) {
  return (piece_names[piece] == 'Q' || piece_names[piece] == 'q');
}

BOOLEAN is_rook(PIECE piece) {
  return (piece_names[piece] == 'R' || piece_names[piece] == 'r');
}

BOOLEAN is_pawn(PIECE piece) {
  return (piece_names[piece] == 'P' || piece_names[piece] == 'p');
}

BOOLEAN is_bishop(PIECE piece) {
  return (piece_names[piece] == 'B' || piece_names[piece] == 'b');
}


/** PRIMITIVE helper functions ******************************************/

/** Returns max of num in row/column from the starting cell **/
int num_in_row(int start, BOARD board) {
  int num_up = 0, num_down = 0, num_left = 0, num_right = 0;
  int row = start / BOARD_WIDTH, col = start % BOARD_WIDTH;
  int i;
  BOOLEAN same_color(int, int, BOARD);
  if (board[start] == BLNK)
    return 0;
  for(i = 1; col + i < BOARD_WIDTH; i++) {
    if (!same_color(start, start+i, board))
      break;
    num_right++;
  }
  for(i = 1; col - i >= 0; i++) {
    if (!same_color(start, start-i, board))
      break;
    num_left++;
  }
  for(i = 1; row + i < BOARD_HEIGHT; i++) {
    if (!same_color(start, start + BOARD_WIDTH*i, board))
      break;
    num_up++;
  }
  for(i = 1; row - i >= 0; i++) {
    if (!same_color(start, start - BOARD_WIDTH*i, board))
      break;
    num_down++;
  }
  return 1+max(num_right+num_left,num_down+num_up);
}

/** DOMOVE helper functions *********************************************/

/** These assume move is represented by 4 lower bits->cell, 3 upper->piece **/

int get_piece(MOVE theMove) {
  return theMove >> 4;
}

int get_source(MOVE theMove, POSITION pos) {
  int piece, loc = OFF, i, *board, get_piece(MOVE);
  piece = get_piece(theMove);
  board = ttc_unhash(pos);
  for(i = 0; i < BOARD_LENGTH; i++) {
    if(board[i] == piece) {
      loc = i;
    }
  }
  return loc;
}

int get_dest(MOVE theMove) {
  return theMove & 0xF;
}

/** GENERATEMOVES helper functions ***************************************/

/** Some optimization might be in order **/

/* Three available moves (dependant on location of enemy pieces */
MOVELIST *gen_pawn_moves(CELL cell, int *board, PLAYER player) {
  MOVELIST *head = NULL;
  if (cell >= BOARD_LENGTH-BOARD_WIDTH) /* if on last row */
    return NULL;
  if (!is_occupied(cell + BOARD_WIDTH, board), board)
    head = CreateMovelistNode(make_move(board[cell],cell+BOARD_WIDTH), head);
  if (cell % BOARD_WIDTH != (BOARD_WIDTH-1) &&
      enemy_occupies(player, cell+BOARD_WIDTH+1, board))
    head = CreateMovelistNode(make_move(board[cell],cell+BOARD_WIDTH+1), head);
  if (cell % BOARD_WIDTH != 0 && 
      enemy_occupies(player, cell+BOARD_WIDTH-1, board))
    head = CreateMovelistNode(make_move(board[cell],cell+BOARD_WIDTH-1), head);
  return head;
}

/* There are eight moves that a knight can make */
MOVELIST *gen_knight_moves(CELL cell, int *board, PLAYER player) {
  MOVELIST *head = NULL;
  int space_up = BOARD_HEIGHT - cell/BOARD_WIDTH - 1;
  int space_down = cell/BOARD_WIDTH;
  int space_right = BOARD_WIDTH - cell % BOARD_WIDTH - 1;
  int space_left = cell % BOARD_WIDTH;
  int dest_cell = cell + N_LONG_LEG*BOARD_WIDTH + N_SHORT_LEG;
  if (space_up >= N_LONG_LEG && space_right >= N_SHORT_LEG &&
      !ally_occupies(player, dest_cell, board))
    head = CreateMovelistNode(make_move(board[cell],dest_cell), head);
  dest_cell = cell + N_LONG_LEG*BOARD_WIDTH - N_SHORT_LEG;
  if (space_up >= N_LONG_LEG && space_left >= N_SHORT_LEG &&
      !ally_occupies(player, dest_cell, board))
    head = CreateMovelistNode(make_move(board[cell],dest_cell), head);
  dest_cell = cell + N_SHORT_LEG*BOARD_WIDTH + N_LONG_LEG;
  if (space_up >= N_SHORT_LEG && space_right >= N_LONG_LEG &&
      !ally_occupies(player, dest_cell, board))
    head = CreateMovelistNode(make_move(board[cell],dest_cell), head);
  dest_cell = cell + N_SHORT_LEG*BOARD_WIDTH - N_LONG_LEG;
  if (space_up >= N_SHORT_LEG && space_left >= N_LONG_LEG &&
      !ally_occupies(player, dest_cell, board))
    head = CreateMovelistNode(make_move(board[cell],dest_cell), head);
  dest_cell = cell - N_SHORT_LEG*BOARD_WIDTH + N_LONG_LEG;
  if (space_down >= N_SHORT_LEG && space_right >= N_LONG_LEG &&
      !ally_occupies(player, dest_cell, board))
    head = CreateMovelistNode(make_move(board[cell],dest_cell), head);
  dest_cell = cell - N_SHORT_LEG*BOARD_WIDTH - N_LONG_LEG;
  if (space_down >= N_SHORT_LEG && space_left >= N_LONG_LEG &&
      !ally_occupies(player, dest_cell, board))
    head = CreateMovelistNode(make_move(board[cell],dest_cell), head);
  dest_cell = cell - N_LONG_LEG*BOARD_WIDTH + N_SHORT_LEG;
  if (space_down >= N_LONG_LEG && space_right >= N_SHORT_LEG &&
      !ally_occupies(player, dest_cell, board))
    head = CreateMovelistNode(make_move(board[cell],dest_cell), head);
  dest_cell = cell - N_LONG_LEG*BOARD_WIDTH - N_SHORT_LEG;
  if (space_down >= N_LONG_LEG && space_left >= N_SHORT_LEG &&
      !ally_occupies(player, dest_cell, board))
    head = CreateMovelistNode(make_move(board[cell],dest_cell), head);
  return head;
}

/* Four general directions in which the rook can move */
MOVELIST *gen_rook_moves(CELL cell, int *board, PLAYER player) {
  MOVELIST *head = NULL;
  int space_up = BOARD_HEIGHT - cell/BOARD_WIDTH - 1;
  int space_down = cell/BOARD_WIDTH;
  int space_right = BOARD_WIDTH - cell % BOARD_WIDTH - 1;
  int space_left = cell % BOARD_WIDTH;
  int dest_cell = cell + N_LONG_LEG*BOARD_WIDTH + N_SHORT_LEG;
  int start_row = cell / BOARD_WIDTH, start_col = cell % BOARD_WIDTH;
  int i;
  for(i = 1; i <= space_up; i++) {
    dest_cell = cell + i*BOARD_WIDTH;
    if (ally_occupies(player, dest_cell, board)) {
      break;
    } else if (enemy_occupies(player, dest_cell, board)) {
      head = CreateMovelistNode(make_move(board[cell],dest_cell), head);
      break;
    } else {
      head = CreateMovelistNode(make_move(board[cell],dest_cell), head);
    }
  }
  for(i = 1; i <= space_right; i++) {
    dest_cell = cell + i;
    if (ally_occupies(player, dest_cell, board)) {
      break;
    } else if (enemy_occupies(player, dest_cell, board)) {
      head = CreateMovelistNode(make_move(board[cell],dest_cell), head);
      break;
    } else {
      head = CreateMovelistNode(make_move(board[cell],dest_cell), head);
    }
  }
  for(i = 1; i <= space_left; i++) {
    dest_cell = cell - i;
    if (ally_occupies(player, dest_cell, board)) {
      break;
    } else if (enemy_occupies(player, dest_cell, board)) {
      head = CreateMovelistNode(make_move(board[cell],dest_cell), head);
      break;
    } else {
      head = CreateMovelistNode(make_move(board[cell],dest_cell), head);
    }
  }
  for(i = 1; i <= space_down; i++) {
    dest_cell = cell - i*BOARD_WIDTH;
    if (ally_occupies(player, dest_cell, board)) {
      break;
    } else if (enemy_occupies(player, dest_cell, board)) {
      head = CreateMovelistNode(make_move(board[cell],dest_cell), head);
      break;
    } else {
      head = CreateMovelistNode(make_move(board[cell],dest_cell), head);
    }
  }
  return head;
}

/* Like rook (four directions), but in diagonals */
MOVELIST *gen_bishop_moves(CELL cell, int *board, PLAYER player) {
  MOVELIST *head = NULL;
  int space_up = BOARD_HEIGHT - cell/BOARD_WIDTH - 1;
  int space_down = cell/BOARD_WIDTH;
  int space_right = BOARD_WIDTH - cell % BOARD_WIDTH - 1;
  int space_left = cell % BOARD_WIDTH;
  int dest_cell = cell + N_LONG_LEG*BOARD_WIDTH + N_SHORT_LEG;
  int start_row = cell / BOARD_WIDTH, start_col = cell % BOARD_WIDTH;
  int i;
  for(i = 1; i <= min(space_up, space_right); i++) {
    dest_cell = cell + BOARD_WIDTH*i + i;
    if (ally_occupies(player, dest_cell, board)) {
      break;
    } else if (enemy_occupies(player, dest_cell, board)) {
      head = CreateMovelistNode(make_move(board[cell],dest_cell), head);
      break;
    } else {
      head = CreateMovelistNode(make_move(board[cell],dest_cell), head);
    }
  }
  for(i = 1; i <= min(space_up, space_left); i++) {
    dest_cell = cell + BOARD_WIDTH*i - i;
    if (ally_occupies(player, dest_cell, board)) {
      break;
    } else if (enemy_occupies(player, dest_cell, board)) {
      head = CreateMovelistNode(make_move(board[cell],dest_cell), head);
      break;
    } else {
      head = CreateMovelistNode(make_move(board[cell],dest_cell), head);
    }
  }
  for(i = 1; i <= min(space_down, space_right); i++) {
    dest_cell = cell - BOARD_WIDTH*i + i;
    if (ally_occupies(player, dest_cell, board)) {
      break;
    } else if (enemy_occupies(player, dest_cell, board)) {
      head = CreateMovelistNode(make_move(board[cell],dest_cell), head);
      break;
    } else {
      head = CreateMovelistNode(make_move(board[cell],dest_cell), head);
    }
  }
  for(i = 1; i <= min(space_down, space_left); i++) {
    dest_cell = cell - BOARD_WIDTH*i - i;
    if (ally_occupies(player, dest_cell, board)) {
      break;
    } else if (enemy_occupies(player, dest_cell, board)) {
      head = CreateMovelistNode(make_move(board[cell],dest_cell), head);
      break;
    } else {
      head = CreateMovelistNode(make_move(board[cell],dest_cell), head);
    }
  }
  return head;
}

MOVELIST *gen_queen_moves(CELL cell, int *board, PLAYER player) {
  return concat_ml(gen_rook_moves(cell,board,player),
		   gen_bishop_moves(cell,board,player));
}


/** HASH Stuff ***********************************************************/

/** Placeholders for the real hash to come **/
int *ttc_unhash(POSITION pos) {
  return NULL;
}
POSITION ttc_hash(BOARD board, PLAYER player) {
  return 0;
}



/************************************************************************
*************************************************************************
**
**  TESTING FUNCTIONS
**
*************************************************************************
************************************************************************/

/** Generates a random board **/
BOARD generate_board() {
  int pieces_used[] = {FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE};
  int i = 0, j;
  BOARD board;
  board = (int *) malloc(BOARD_LENGTH*sizeof(int));
  srand(time(NULL));

  for(i = 0; i < BOARD_LENGTH; i++) {
    j = rand() % (2*NUM_PIECES);
    if (j < NUM_PIECES && pieces_used[j] != TRUE) {
      board[i] = j;
      pieces_used[j] = TRUE;
    } else {
      board[i] = BLNK;
    }
  }
  return board;
}

