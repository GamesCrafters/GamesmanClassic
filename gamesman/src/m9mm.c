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

POSITION gNumberOfPositions  = 0;

POSITION gInitialPosition    = 11137;
POSITION gMinimalPosition    = 0;
POSITION kBadPosition        = -1;

STRING   kGameName           = "Nine Men's Morris";
STRING   kDBName             = "9mm";
BOOLEAN  kPartizan           = TRUE; 
BOOLEAN  kSupportsHeuristic  = FALSE;
BOOLEAN  kSupportsSymmetries = FALSE;
BOOLEAN  kSupportsGraphics   = FALSE;
BOOLEAN  kDebugMenu          = TRUE;
BOOLEAN  kGameSpecificMenu   = FALSE;
BOOLEAN  kTieIsPossible      = FALSE;
BOOLEAN  kLoopy               = TRUE;
BOOLEAN  kDebugDetermineValue = FALSE;

STRING kHelpGraphicInterface =
"Not written yet";

STRING   kHelpTextInterface    =
"No help for you!"; 

STRING   kHelpOnYourTurn =
"Make a move";

STRING   kHelpStandardObjective =
"Win";

STRING   kHelpReverseObjective =
"Lose";

STRING   kHelpTieOccursWhen = /* Should follow 'A Tie occurs when... */
"A Tie occurs when....never!";

STRING   kHelpExample =
"See example.";

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

BOOLEAN debug = TRUE;

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
char gblankoxChar[] = { 'b', 'x', 'o'};
//POSITION gHashNumberOfPos = 0;

/*************************************************************************
**
** Above is where you put your #define's and your global variables, structs
**
*************************************************************************/

/*
** Function Prototypes:
*/

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn); // not sure why..
POSITION hash(blankox *board, blankox turn);
blankox *unhash(int hash_val, blankox *dest);
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
blankox parse_char(char c);
BOOLEAN closes_mill_move(MOVE the_move);

// Debugging
void debugBoard(blankox *bboard, char *cboard);

// External
GENERIC_PTR	SafeMalloc (size_t size);
void		SafeFree ();

/*************************************************************************
**
** Here we declare the global database variables
**
**************************************************************************/

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
  
  
  int b_size = BOARDSIZE;
  int pminmax[] = {'o', mino, maxo, 'x', minx, maxx, 'b', b_size-maxo-maxx, b_size-minx-mino, -1};

  generic_hash_init(b_size, pminmax, NULL);

  
  
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
	char GetMyChar();
  POSITION GetInitialPosition();
  
  do {
    printf("?\n\t----- Game-specific options for %s -----\n\n", kGameName);
    
    printf("\tCurrent Initial Position:\n");
    PrintPosition(gInitialPosition, gPlayerName[kPlayerOneTurn], kHumansTurn);
    
    printf("\tI)\tChoose the (I)nitial position\n");
    	    
    printf("\n\n\tb)\t(B)ack = Return to previous activity.\n");
    printf("\n\nSelect an option: ");
    
    switch(GetMyChar()) {
    case 'Q': case 'q':
      ExitStageRight();
    case 'H': case 'h':
      HelpMenus();
      break;
    case 'I': case 'i':
      gInitialPosition = GetInitialPosition();
      break;
    case 'B': case 'b':
      return;
    default:
      printf("\nSorry, I don't know that option. Try another.\n");
      HitAnyKeyToContinue();
      break;
    }
  } while(TRUE);
	  
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
 
  //debug
  char cboard[BOARDSIZE];

  blankox board [BOARDSIZE];  
  int from_slot = from(theMove);
  int to_slot = to(theMove);
  int remove_slot = remove_piece(theMove);

  
  //debug
  if (debug) {
    printf("The move doMove get is: %d\n", theMove);
    printf("In doMove, the from, to, remove are: %d %d %d\n", from_slot, to_slot, remove_slot);
  }
 
  unhash(thePosition, board);
  //debug
  if (debug) {
    printf("thePosition is: %d\n", thePosition);
    printf("It is this person's turn: %d\n", whoseMove(thePosition));
  }

  board[to_slot] = board[from_slot];
  board[from_slot] = blank;
  board[remove_slot] = blank; // if no piece is removed, remove = from

  //debug
  if (debug) {
    unparse_board(board, cboard);
    printf("The board after the move is: ");
    debugBoard(board,cboard);
  }
  
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
  int xOnBoard, oOnBoard, bOnBoard, i;
  signed char c;
  blankox board[BOARDSIZE];
  blankox turn;
  
  i = xOnBoard = oOnBoard = bOnBoard = 0;
  
  
  printf("\n\n\t----- Get Initial Position -----\n");
  printf("\n\tPlease input the position to begin with.\n");
  printf("\nUse x for left player, o for right player, and _ for blank spaces\n");
  printf("Example:\n");
  printf("        _-----------o-----------_\n");
  printf("        |           |           |\n");
  printf("        |           |           |\n");
  printf("        |   x-------_-------o   |\n");
  printf("        |   |       |       |   |\n");
  printf("        |   |   _---x---_   |   |\n");
  printf("        |   |   |       |   |   |\n");
  printf("        _---_---_       _---_---_\n");
  printf("        |   |   |       |   |   |\n");
  printf("        |   |   _---x---_   |   |\n");
  printf("        |   |       |       |   |\n");
  printf("        |   _-------_-------o   |\n");
  printf("        |           |           |\n");
  printf("        |           |           |\n");
  printf("        _-----------_-----------_\n");
  printf("Is input as \n\t_ o _\n\tx _ o\n _ x _ \n\t_ _ _ _ _ _\n\t_ x _ \n\t_ _ o\n\t_ _ _");

  getchar(); // dump a char

  while (i < BOARDSIZE && (c = getchar()) != EOF) {
    board[i++] = parse_char(c);
    if (c == 'x' || c == 'X')
      xOnBoard++;
    else if (c == 'o' || c == 'O' || c == '0')
      oOnBoard++;
    else if (c == 'b' || c == 'B')
      bOnBoard++;
    else {
      printf("\n Illegal Board Position Please Re-Enter\n");
      return GetInitialPosition();
    }
  }

  // hard coded sanity check for 3 vs 3
  // should add a hash/unhash sanity check
  if (xOnBoard != 3 || oOnBoard != 3 || bOnBoard != 18) {
    printf("\n Illegal Board Position Please Re-Enter\n");
    return GetInitialPosition();
  }

  // now get the turn
  getchar(); // dump another character
  printf("\nWhose turn is it? [x/o]");
  scanf("%c",&c);

  turn = parse_char(c);
  
  return hash(board, turn);

  
  
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
  printf("%8s's move              : from %d  to %d\n", computersName, from(computersMove), to(computersMove));
  if (closes_mill_move(computersMove))
      printf("\tand removes %d", remove_piece(computersMove));
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
  blankox dest[BOARDSIZE];
  //blankox *dest = (blankox *) malloc (BOARDSIZE * sizeof (blankox));
  int numXs = 0;
  int numOs = 0;
  int i;
  blankox turn = whose_turn(h);
  
  unhash(h, dest);

  for (i = 0; i < BOARDSIZE; i++) {
    if (dest[i] == x) {
      numXs++;
    }
    else if (dest[i] == o) {
      numOs++;
    }
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
  blankox board[BOARDSIZE];
  blankox turn;
  char c_board[BOARDSIZE];
  
  unhash(position, board);
  unparse_board(board, c_board);
  turn = whose_turn(position);
  
  
  printf("\n");
  printf("        0-----------1-----------2       %c-----------%c-----------%c\n", c_board[0], c_board[1], c_board[2] );
  printf("        |           |           |       |           |           |\n");
  printf("        |           |           |       |           |           |\n");
  printf("        |   3-------4-------5   |       |   %c-------%c-------%c   |\n", c_board[3], c_board[4], c_board[5] );
  printf("        |   |       |       |   |       |   |       |       |   |\n");
  printf("        |   |   6---7---8   |   |       |   |   %c---%c---%c   |   |\n", c_board[6], c_board[7], c_board[8] );
  printf("        |   |   |       |   |   |       |   |   |       |   |   |\n");
  printf("LEGEND: 9---10--11      12--13--14      %c---%c---%c       %c---%c---%c    Turn:%c\n", c_board[9], c_board[10], c_board[11], c_board[12], c_board[13], c_board[14], turn );
  printf("        |   |   |       |   |   |       |   |   |       |   |   |\n");
  printf("        |   |   15--16--17  |   |       |   |   %c---%c---%c   |   |\n", c_board[15], c_board[16], c_board[17] );
  printf("        |   |       |       |   |       |   |       |       |   |\n");
  printf("        |   18------19------20  |       |   %c-------%c-------%c   |\n", c_board[18], c_board[19], c_board[20] );
  printf("        |           |           |       |           |           |\n");
  printf("        |           |           |       |           |           |\n");
  printf("        21----------22----------23      %c-----------%c-----------%c\n", c_board[21], c_board[22], c_board[23] );
  GetPrediction(position,playerName,usersTurn);
    /*
  0-----------1-----------2
  |           |           |
  |           |           |
  |   3-------4-------5   |
  |   |       |       |   |
  |   |   6---7---8   |   |
  |   |   |       |   |   |
  9---10--11      12--13--14
  |   |   |       |   |   |
  |   |   15--16--17  |   |
  |   |       |       |   |
  |   18------19------20  |
  |           |           |
  |           |           |
  21----------22----------23
  */
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
	      (blanks[j] * BOARDSIZE) + player_pieces[i];

	    //debug
	    if (debug) {
	      printf ("the raw_move is: %d\n", raw_move);
	    }

	    if (closes_mill(position, raw_move))
	      {
		for (k = 0; k < opponent_count; k++)
		  if (can_be_taken(position, opponent_pieces[k]))
		    head = CreateMovelistNode(raw_move += (opponent_pieces[k]-player_pieces[i]) , head);
	      }
	    else
	      head = CreateMovelistNode(raw_move, head);
	    
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
  BOOLEAN ValidMove();
  USERINPUT ret, HandleDefaultTextInput();
  
  do {
    printf("%8s's move [(u)ndo/[0-23 0-23](,[0-23])?] :  ", playerName);

    
    ret = HandleDefaultTextInput(thePosition, theMove, playerName);
    if(ret != Continue)
      return(ret);
  }
  while (TRUE);
  return (Continue);
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
  // we could bulletproof this a lot more
  // currently checks for a number, a space followed by a number, 
  //and an optional comma with a number
  
  int i;

  BOOLEAN hasSpace, hasComma;
  STRING afterSpace;
  STRING afterComma;
  
  i = atoi(input);

  hasSpace = index(input, ' ') != NULL;
  hasComma = index(input, ',') != NULL;
  
  if (hasSpace)
    afterSpace = index(input, ' ');
  else {
    return FALSE;
  }
    
  if (hasComma)
    afterComma = index(input, ',');
  else
    return FALSE;

  i = atoi(input);
  if (!(i < BOARDSIZE) && (i >= 0))
  	return FALSE;
  
  i = atoi(afterSpace);
  if (!(i < BOARDSIZE) && (i >= 0))
  	return FALSE;
  
  if (hasComma)
  i = atoi(afterComma);
  if (!(i < BOARDSIZE) && (i >= 0))
  	return FALSE;
  else 
    return TRUE;
  
  return FALSE; // should never be reached
  
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
  int from, to, remove;
  STRING afterSpace;
  STRING afterComma;
  BOOLEAN hasSpace, hasComma;

  hasSpace = (index(input, ' ')) != NULL;
  hasComma = (index(input, ',')) != NULL;
  
  from = atoi(input);

  
  if (hasSpace) {
    afterSpace = index(input, ' ');
  to = atoi(afterSpace);
  } else {
    return 0; // Should be a bad else
  }
    
  if (hasComma) {
    afterComma = index(input, ',');
    remove = atoi(afterComma);
  } else
    remove = from;
  
  //debug
  if (debug) {
    printf ("in InputHandler, the from, to, remove is: %d %d %d\n", from, to, remove);
  }

  
  return hash_move(from, to, remove);
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
      printf("\"%d %d", from(theMove), to(theMove));
      if (closes_mill_move(theMove))
	printf(" %d", remove_piece(theMove));
      printf("\"");
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
  return 1;
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
  int ret;
  ret = gStandardGame;
  
  return ++ret;
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
  option--;
  gStandardGame = option %2;
  

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
**         Everything BELOW THESE LINES IS LOCAL TO THIS FILE
*************************************************************************
************************************************************************/


/************************************************************************
** This is where you can put any helper functions, including your
** hash and unhash functions if you are not using one of the existing
** ones.
************************************************************************/

POSITION hash(blankox *b_board, blankox turn)
{

  char c_board[BOARDSIZE];
  int raw_hash;
  int generic_hash();
  int player;

  if (turn == x) 
    player = 1;
  else 
    player = 2;
  unparse_board(b_board, c_board);
  
  return generic_hash(c_board, player);

  
}

blankox *unhash(int hash_val, blankox *b_board)
{
  char c_board [BOARDSIZE];
  
  generic_unhash(hash_val, c_board);
  
  //debug
  if (debug) {
    printf("The hash value is: %d\n", hash_val);
    debugBoard(b_board, c_board);
  }
  
  parse_board(c_board, b_board);
  
  //debug
  if (debug) {
    debugBoard(b_board,c_board);
  }

  return b_board;
  
}

// char to blankox conversion
void parse_board(char *c_board, blankox *b_board)
{
  int i;
  for (i = 0; i < BOARDSIZE; i++) 
    { 
      if (c_board[i] == 'o' || c_board[i] == 'O')
	b_board[i] = o;
      else if (c_board[i] == 'x' || c_board[i] == 'X')
	b_board[i] = x;
      else if (c_board[i] == 'b' || c_board[i] == 'B')
	b_board[i] = blank;
    }
}

blankox parse_char(char c) {
  if (c == 'x' || c == 'X')
    return x;
  else if (c == 'o' || c == '0' || c == 'O')
    return o;
  else if (c == 'b' || c == 'B')
    return blank;
  else
    return x; // fix this so that it's a badelse
  
}

//blankox to char conversion
void unparse_board(blankox *b_board, char *c_board)
{
  int i;
  for (i = 0; i < BOARDSIZE; i++)
    {
      c_board[i] = gblankoxChar[b_board[i]];
    }
}




blankox whose_turn(int hash_val)
{
  if (whoseMove (hash_val) == 1) 
    return x;
  else return o;
}

// if there is no removal, then from == remove
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
  return (the_move % BOARDSIZE);
}

blankox opponent (blankox player)
{
  return (player == o ? x : o);
}

BOOLEAN can_be_taken(POSITION position, int slot)
{
  blankox board[BOARDSIZE];
  unhash(position, board);
  return !check_mill(board, slot);
}

BOOLEAN closes_mill_move(MOVE the_move) {
  return from(the_move) == remove_piece(the_move);
}

BOOLEAN closes_mill(POSITION position, int raw_move)
{
  blankox board[BOARDSIZE];
  unhash(DoMove(position, raw_move), board);
  return check_mill (board, to(raw_move));
}

BOOLEAN check_mill(blankox *board, int slot)
{
  return three_in_a_row(board, 0, 1,  2, slot) || // horizontals
    three_in_a_row(board, 3, 4,  5, slot) ||
    three_in_a_row(board, 6, 7,  8, slot) ||
    three_in_a_row(board, 9, 10,  11, slot) ||
    three_in_a_row(board, 12, 13, 14,slot) ||
    three_in_a_row(board, 15, 16,  17, slot) ||
    three_in_a_row(board, 18, 19, 20,slot) ||
    three_in_a_row(board, 21, 22, 23,slot) ||
    three_in_a_row(board, 0, 9,  21,slot) || // verticals
    three_in_a_row(board, 3, 10, 18,slot) ||
    three_in_a_row(board, 1, 4, 7,slot) ||
    three_in_a_row(board, 16, 19,22,slot) ||
    three_in_a_row(board, 8, 12,17,slot) ||
    three_in_a_row(board, 6, 11,15,slot) ||
    three_in_a_row(board, 5, 13,20,slot) ||
    three_in_a_row(board, 2, 14,23,slot);
}

BOOLEAN three_in_a_row(blankox *board, int slot1, int slot2, int slot3, int slot)
{
  return board[slot] == board[slot1] &&
    board[slot] == board[slot2] &&
    board[slot] == board[slot3] &&
    (slot == slot1 || slot == slot2 || slot == slot3);
}

/************************************************************************
 ** Some Debugging Functions
************************************************************************/

void debugBoard(blankox *bboard, char *cboard)
{
  int i;
  if (debug) {
    for (i = 0; i < BOARDSIZE; i++)
      printf("%d", bboard[i]);
    printf("\n");
    for (i = 0; i < BOARDSIZE; i++)
      printf("%c", cboard[i]);
    printf("\n");
  }
}



//$Log: not supported by cvs2svn $
//Revision 1.25  2004/04/05 01:58:40  weitu
//m9mm fixed no segfaults, need to take care of input
//
//Revision 1.24  2004/04/04 19:08:11  weitu
//hash doesn't work? see debug messages
//
//Revision 1.22  2004/03/17 11:39:04  bryonr
//Changed type of gNumberOfPositions from int to POSITION. -Bryon
//
//Revision 1.21  2004/03/16 01:58:39  weitu
//Fixed to fit hash.c, now runs but wrong init position.
//
//Revision 1.20  2004/03/16 01:46:18  ogren
//set kdebugmenu  TRUE - Elmer
//
//Revision 1.19  2004/03/15 23:36:18  bryonr
//Changed some return types to match function prototypes in gamesman.h. Made minor fixes where necessary.
//
//Revision 1.18  2004/03/14 22:28:13  jjjordan
//Prototype fix. -JJ
//
//Revision 1.17  2004/03/11 02:25:27  evedar
//9mm compiles (2 warnings), still need to fix things mentioned in last entry.  9mm now included in makefile
//
//Revision 1.16  2004/03/11 01:42:48  evedar
//Finished m9mm functions.  Still needs: fixes from last checkin, help strings, etc.  Need to compile this latest version.  Specifically: filled in get option, set option, game-specific menu.
//
//Revision 1.15  2004/03/10 23:47:07  evedar
//Filled in almost all user i/o functions.  Need to fix: int/MOVE abstraction, badElse's.  Had to hard code in get initial position.  Should add a position hash/unhash sanity check to that function.  Commented 9mm out of Makefile while getting this stuff to compile.
//
//Revision 1.14  2004/03/07 20:49:09  evedar
//Added printposition, changed internal rep of board locations
//
//Revision 1.13  2004/03/07 19:28:31  evedar
//Parse errors fixed
//
//Revision 1.12  2004/03/07 19:05:03  weitu
//1.12 added hash.h and changed to fit new hash function.
//

//Revision 1.11  2004/03/07 18:59:04  ogren
//commet changes -Elmer
//
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
