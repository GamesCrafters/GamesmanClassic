//$Id: m9mm.c,v 1.11 2004-03-07 18:59:04 ogren Exp $
/************************************************************************
**
** NAME:        m9mm.c
**
** DESCRIPTION: Nine Men's Morris
**
** AUTHOR:      Erwin A. Vedar, Wei Tu, Elmer Lee
**
** DATE:        Start: 10am 2004.2.22
**              Finish: Never
**
** UPDATE HIST: 2004.02.22: Begin coding module, implemented functions: 
**                          int hash
**                          POSITION unhash
**                          void parse_board
**                          void unparse_board
**                          blankox whose_turn
**                          MOVE hash_move
**                          int from
**                          int to
**                          int remove_piece
**                          blankox opponent
**                          BOOLEAN can_be_taken
**                          BOOLEAN closes_mill
**                          BOOLEAN check_mill
**                          BOOLEAN three_in_a_row
**                          void InitializeGame
**                          POSITION DoMove
**                          VALUE Primitive
**                          MOVELIST *GenerateMoves(POSITION position)
**
**              2004.03.07: DoMove, GenerateMoves  now compiles
** 
**
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

STRING   kGameName           = "Nine Men's Morris";
STRING   kDBName             = "9mm";
BOOLEAN  kPartizan           = TRUE; 
BOOLEAN  kSupportsHeuristic  = FALSE;
BOOLEAN  kSupportsSymmetries = FALSE;
BOOLEAN  kSupportsGraphics   = FALSE;
BOOLEAN  kDebugMenu          = FALSE;
BOOLEAN  kGameSpecificMenu   = FALSE;
BOOLEAN  kTieIsPossible      = TRUE;
BOOLEAN  kLoopy               = TRUE;
BOOLEAN  kDebugDetermineValue = FALSE;

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


/*************************************************************************
**
** Below is where you put your #define's and your global variables, structs
**
*************************************************************************/

#define BOARDSIZE 24
#define minx  2
#define maxx  3
#define mino  2
#define maxo  3

typedef enum Pieces {
  blank, x, o
} blankox;
char *gblankoxString[] = { " ", "x", "o"};


/*************************************************************************
**
** Above is where you put your #define's and your global variables, structs
**
*************************************************************************/

/*
** Function Prototypes:
*/

int hash(blankox *board, blankox turn);
blankox* unhash(int hash_val, blankox *dest);
void parse_board(char *c_board, blankox *b_board);
void unparse_board(blankox *b_board, char *c_board);
blankox whose_turn(int hash_val);
MOVE hash_move(int from, int to, int remove);
int from(MOVE the_move);
int to(MOVE the_move);
int remove_piece(MOVE the_move);
blankox opponent(blankox player);
BOOLEAN can_be_taken(POSITION the_position, int slot);
BOOLEAN closes_mill(POSITION position, int raw_move);
BOOLEAN check_mill(blankox *board, int slot);
BOOLEAN three_in_a_row(blankox *board, int slot1, int slot2, int slot3, int slot);

// External
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

void InitializeGame()
{
  //generic_hash_init(BOARDSIZE, mino, maxo, minx, maxx);
  int b_size = BOARDSIZE;
  int right_min = mino;
  int right_max = maxo;
  int left_min = minx;
  int left_max = maxx;
  generic_hash_init(b_size, right_min, right_max, left_min, left_max);
}

/************************************************************************
**
** NAME:        DebugMenu
**
** DESCRIPTION: Menu used to debub internal problems. Does nothing if
**              kDebugMenu == FALSE
** 
************************************************************************/

void DebugMenu()
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

void GameSpecificMenu() 
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

void SetTclCGameSpecificOptions(theOptions)
     int theOptions[];
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
POSITION DoMove(thePosition, theMove)
     POSITION thePosition;
     MOVE theMove;
{
  blankox board [24];  
  int from_slot = from(theMove);
  int to_slot = to(theMove);
  int remove_slot = remove_piece(theMove);

  
  unhash(thePosition, board);

  board[to_slot] = board[from_slot];
  board[from_slot] = blank;
  board[remove_slot] = blank; // if no piece is removed, remove = from

  return hash(board, whose_turn(thePosition) == x ? o : x);
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

VALUE Primitive ( POSITION h )
{
  blankox[BOARDSIZE] dest;
  int numXs = 0;
  int numOs = 0;
  int i;
  blankox turn = whose_turn(h);
  
  unhash(h, dest);

  for (i = 0; i < BOARDSIZE; i++) {
    if (dest[i] == x)
      numXs++;
    else if (dest[i] == o)
      numOs++;
    
  }


  // doesn't check getting stuck
  if (turn == o && numOs == mino)
    return (gStandardGame ? lose : win );
  else if (turn == x && numXs == minx)
    return (gStandardGame ? lose : win );
  else
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

void PrintPosition(position, playerName, usersTurn)
     POSITION position;
     STRING playerName;
     BOOLEAN usersTurn;
{
  
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

MOVELIST *GenerateMoves(POSITION position)         
{
  int i, j, k, x_count, o_count, blank_count;
  int player_count, opponent_count, raw_move;
  MOVELIST *CreateMovelistNode(), *head = NULL;
  MOVELIST *temp_head = NULL;
  blankox dest[BOARDSIZE];
  blankox x_pieces[maxx];
  blankox o_pieces[maxo];
  blankox *player_pieces;
  blankox *opponent_pieces;
  blankox blanks[BOARDSIZE];
  blankox turn = whose_turn(position);
  x_count = o_count = blank_count = 0;
    
  if(Primitive(position) == undecided) {
    unhash(position, dest);
    
    
    for (i = 0; i < BOARDSIZE; i++)
      {
	if (dest[i] == x)
	  x_pieces[x_count++] = i;
	else if (dest[i] == o)
	  o_pieces[o_count++] = i;
	else
	  blanks[blank_count++] = i;
      }

    if (turn == x)
      {
	player_pieces = x_pieces;
	player_count = x_count;
	opponent_count = o_count;
	opponent_pieces = o_pieces;
      }
    else
      {
	player_pieces = o_pieces;
	player_count = o_count;
	opponent_count = x_count;
	opponent_pieces = x_pieces;
      }
    
    for (i = 0; i < player_count; i++)
      {
	for (j = 0; j < blank_count; j++)
	  {
	    raw_move = (player_pieces[i] * BOARDSIZE * BOARDSIZE) +
	      (blanks[j] * BOARDSIZE);
	    if (closes_Mill(position, raw_move))
	      {
		for (k = 0; k < opponent_count; k++)
		  if (can_be_taken(position, opponent_pieces[k]))
		    head = CreateMovelistNode(raw_move += opponent_pieces[k], head);
	      }
	    else
	      head = CreateMovelistNode(raw_move += player_pieces[i], head);
	    
	  }
      }
    return head;
  }
  else
    return NULL;
  
    
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

USERINPUT GetAndPrintPlayersMove(thePosition, theMove, playerName)
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

BOOLEAN ValidTextInput(input)
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

MOVE ConvertTextInputToMove(input)
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

void PrintMove(theMove)
     MOVE theMove;
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

int NumberOfOptions()
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
int GameSpecificTclInit(Tcl_Interp* interp,Tk_Window mainWindow) 
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

int hash(blankox *b_board, blankox turn)
{

  int raw_hash;
  char[BOARDSIZE] c_board; 
  unparse_board(b_board, c_board);
  raw_hash = generic_hash(c_board);
  return (turn == B ? raw_hash : raw_hash + gHashNumberOfPos);
}

blankox *unhash(int hash_val, blankox *b_board)
{
  char[BOARDSIZE] c_board;
  
  
  (hash_val > gHashNumberOfPos ? generic_unhash(hash_val - gHashNumberOfPos, c_board)
						       : generic_unhash(hash_val, c_board));

  parse_board(c_board, b_board);

  return b_board;
  
}

void parse_board(char *c_board, blankox *b_board)
{
  int i;
  for (i = 0; i < BOARDSIZE; i++) 
    { 
      if (c_board[i] == 'o')
	b_board[i] = o;
      else if (c_board[i] == 'x')
	b_board[i] = x;
      else if (c_board[i] == 'b')
	b_board[i] = blank;
    }
}

void unparse_board(blankox *b_board, char *c_board)
{
  int i;
  for (i = 0; i < BOARDSIZE; i++)
    {
      if (b_board[i] == o)
	c_board[i] = 'o';
      else if (b_board[i] == x)
	c_board[i] = 'x';
      else if (b_board[i] == blank)
	c_board[i] = 'b';
    }
}


blankox whose_turn(int hash_val)
{
  return (hash_val > gHashNumerOfPos ? x : o);
}

MOVE hash_move(int from, int to, int remove)
{
  return ((from * BOARDSIZE * BOARDSIZE) + (to * BOARDSIZE) + remove);
}

int from(MOVE move)
{
  return (move / (BOARDSIZE * BOARDSIZE));
}

int to(MOVE move)
{
  return ((move % (BOARDSIZE * BOARDSIZE)) / (BOARDSIZE));
}

int remove_piece(MOVE the_move)
{
  return (move % BOARDSIZE);
}

blankox opponent (blankox player)
{
  return (player == o ? x : o);
}

BOOLEAN can_be_taken(POSITION position, int slot)
{
  blankox[BOARDSIZE] board;
  unhash(position, slot);
  return !check_mill(board, slot);
}

BOOLEAN closes_mill(POSITION position, int raw_move)
{
  blankox board[BOARDSIZE];
  unhash(do_move(position, raw_move), board);
  return check_mill(board, to(raw_move));
}

BOOLEAN check_mill(blankox *board, int slot)
{
  return three_in_a_row(board, 0, 1,  2, slot) ||
    three_in_a_row(board, 0, 6,  7, slot) ||
    three_in_a_row(board, 1, 7,  8, slot) ||
    three_in_a_row(board, 2, 3,  4, slot) ||
    three_in_a_row(board, 3, 11, 19,slot) ||
    three_in_a_row(board, 4, 5,  6, slot) ||
    three_in_a_row(board, 5, 13, 21,slot) ||
    three_in_a_row(board, 7, 15, 23,slot) ||
    three_in_a_row(board, 8, 9,  10,slot) ||
    three_in_a_row(board, 8, 13, 14,slot) ||
    three_in_a_row(board, 10,11, 12,slot) ||
    three_in_a_row(board, 12, 13,14,slot) ||
    three_in_a_row(board, 16, 17,18,slot) ||
    three_in_a_row(board, 16, 22,23,slot) ||
    three_in_a_row(board, 18, 19,20,slot) ||
    three_in_a_row(board, 20, 21,22,slot);
}

BOOLEAN three_in_a_row(blankox *board, int slot1, int slot2, int slot3, int slot)
{
  return board[slot] == board[slot1] &&
    board[slot] == board[slot2] &&
    board[slot] == board[slot3] &&
    (slot == slot1 || slot == slot2 || slot == slot3);
}


//$Log: not supported by cvs2svn $
//Revision 1.10  2004/03/07 18:56:29  ogren
//misc. helper changes -Elmer
//
//Revision 1.9  2004/03/07 18:54:06  ogren
//Fixed some small syntax errors -Elmer
//
//Revision 1.8  2004/03/07 18:41:41  ogren
//removed semicolons from #define - Elmer
//
//Revision 1.7  2004/03/07 18:12:12  ogren
//DoMove compiles, changed piece enumeration from "b,o,x" to "blank,o,x" - Elmer
//
//Revision 1.6  2004/02/29 04:41:02  evedar
//Fixed some compile bugs.  Apparently boolean should be BOOLEAN.  Seems to be some problem with #define's
//
//Revision 1.5  2004/02/26 02:34:36  ogren
//Added CVS Id, Log tags to code
//
