// $id$
// $log$

/* Above is will include the name and the log of the last
 * person to commit this file to gamesman.
 */

/************************************************************************
**
** NAME:        miceblocks.c
**
** DESCRIPTION: Ice Blocks
**
** AUTHOR:      YOUR NAMES HERE
**
** DATE:        WHEN YOU START/FINISH
**
** UPDATE HIST: RECORD CHANGES YOU HAVE MADE SO THAT TEAMMATES KNOW
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
#include "hash.h"
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

extern STRING gValueString[];

POSITION gNumberOfPositions  = 0;
POSITION gInitialPosition    = 0;
POSITION gMinimalPosition    = 0;
POSITION kBadPosition        = -1;
STRING   kGameName           = "Ice Blocks";
STRING   kDBName             = "iceblocks";
BOOLEAN  kPartizan           = TRUE;
BOOLEAN  kDebugMenu          = TRUE;
BOOLEAN  kGameSpecificMenu   = FALSE;
BOOLEAN  kTieIsPossible      = TRUE;
BOOLEAN  kLoopy               = FALSE;
BOOLEAN  kDebugDetermineValue = TRUE;
void*	 gGameSpecificTclInit = NULL;

/* 
   Help strings that are pretty self-explanatory 
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

#define X   99999
#define O   9999

int base = 4;

typedef int TURN;
typedef struct boardRep *BOARD;

struct boardRep{
  int **spaces;
  TURN turn;
};


/* External */
extern GENERIC_PTR	SafeMalloc ();
extern void		SafeFree ();


/*************************************************************************
**
** Here we declare the global database variables
**
**************************************************************************/

extern VALUE     *gDatabase;

int sumto (int i) {
  int ans = 0;
  while(i > 0) {
    ans += i;
    i--;
  }
  return ans;
}

int vcfg (int *this_cfg) {
  return ((this_cfg[0] == this_cfg[1]) || (this_cfg[0] == (this_cfg[1] + 1))); 
}

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
  int i, sum = sumto(base);
  char board[sum];
  int theBoard[10] = {'x', 0, ((sum / 2) + (sum % 2)), 'o', 0, (sum / 2), '-', 0, sum, -1};
  gNumberOfPositions = generic_hash_init(sum, theBoard, vcfg);
  for(i = 0; i < sum; i++)
    board[i] = '-';
  gInitialPosition = generic_hash(board, 1);
  gMinimalPosition = gInitialPosition;
}

BOARD arraytoboard (POSITION position) {
  char *theBoard = (char *) malloc(sumto(base) * sizeof(char));
  int i, j, k;
  BOARD board = (BOARD) SafeMalloc(sizeof(struct boardRep));
  board->spaces = (int **) SafeMalloc(base * sizeof(int *));
  for(i = 0; i < base; i++)
    board->spaces[i] = (int *) SafeMalloc((base - i) * sizeof(int));
  theBoard = generic_unhash(position, theBoard);
  for(i = 0, k = 0; i < base; i++) {
    for(j = 0; j < (base - i); j++, k++) {
      if(theBoard[k] == '-')
	board->spaces[i][j] = i * base - sumto(i - 1) + j + 1;
      else if(theBoard[k] == 'x')
	board->spaces[i][j] = X;
      else if(theBoard[k] == 'o')
	board->spaces[i][j] = O;
    }
  }
  if(whoseMove(position) - 1)
    board->turn = O;
  else
    board->turn = X;
  return board;
}

POSITION boardtoarray (BOARD board) {
  char theBoard[sumto(base)];
  int i, j, k;
  TURN whoseTurn;
  for(i = 0, k = 0; i < base; i++) {
    for(j = 0; j < base - i; j++, k++) {
      if(board->spaces[i][j] < 999)
	theBoard[k] = '-';
      else if(board->spaces[i][j] == X)
	theBoard[k] = 'x';
      else if(board->spaces[i][j] == O)
	theBoard[k] = 'o';
    }
  }
  if(board->turn == X)
    whoseTurn = 1;
  else
    whoseTurn = 2;
  for(i = 0; i < base; i++)
    SafeFree(board->spaces[i]);
  SafeFree(board->spaces);
  SafeFree(board);
  return generic_hash(theBoard, whoseTurn);
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
** CALLS:       Hash ()
**              Unhash ()
**	            LIST OTHER CALLS HERE
*************************************************************************/

POSITION DoMove (thePosition, theMove)
	POSITION thePosition;
	MOVE theMove;
{
  BOARD board = arraytoboard(thePosition);
  int i; 
  for (i = 0; theMove - (base - i) > 0; theMove -= (base - i), i++);
  board->spaces[i][theMove - 1] = board->turn;
  if(board->turn == X)
    board->turn = O;
  else
    board->turn = X;
  return boardtoarray(board);
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
  char input[sumto(base)];
  printf("type in a %d character string of X's, O's and -'s\nNote that X always goes first: ", sumto(base));
  scanf("%s", input);
  return generic_hash(input, 1);
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
  printf("%8s's move              : %d\n\n", computersName, computersMove);
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

VALUE Primitive (pos)
	POSITION pos;
{
  BOARD board = arraytoboard(pos);
  int i, j, countX = 0, countO = 0, color, k, m, count;
  int dlvisited[base][base], drvisited[base][base], hvisited[base][base];
  for(i = 0; i < base; i++) {
    for(j = 0; j < base; j++) {
      dlvisited[i][j] = 0;
      drvisited[i][j] = 0;
      hvisited[i][j] = 0;
    }
  }
  for (i = 0; i < base; i++) {
    for (j = 0; j < (base - i); j++) {
      if(board->spaces[i][j] < 999)
	return undecided;
      color = board->spaces[i][j];
      for(k = i, m = j, count = 0; k < base && m >= 0 && !dlvisited[k][m] &&
	    board->spaces[k][m] == color; k++, m--) {
	count++; 
	dlvisited[k][m] = 1; 
      }
      if ((count > 1) && (color == X))
	countX += count;
      else if ((count > 1) && (color == O))
	countO += count;
      for (k = i, m = j, count = 0; k < base && m < (base - k) &&
	     !drvisited[k][m] && board->spaces[k][m] == color; k++) {
	count++;
	drvisited[k][m] = 1;
      }
      if ((count > 1) && (color == X))
	countX += count;
      else if ((count > 1) && (color == O))
	countO += count;
      for (k = i, m = j, count = 0; m < (base - k) && !hvisited[k][m] && 
	     board->spaces[k][m] == color; m++) { 
	count++;
	hvisited[k][m] = 1;
      }
      if((count > 1) && (color == X))
	countX += count;
      else if ((count > 1) && (color == O))
	countO += count;
    }
  }
  if(countX > countO)
    return ((board->turn == X) ? win : lose);
  else if(countO > countX)
    return ((board->turn == O) ? win : lose);
  else
    return tie;
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

void PrintPosition (position, playerName, usersTurn)
	POSITION position;
	STRING playerName;
	BOOLEAN usersTurn;
{
  BOARD board = arraytoboard(position);
  int i, j;
  for(i = base - 1; i >= 0; i--) {
    printf("  ");
    for(j = 0; j < i; j++)
      printf("  ");
    for(j = 0; j < (base - i); j++) {
      printf("[");
      if(board->spaces[i][j] == X)
	printf("XX]");
      else if(board->spaces[i][j] == O)
	printf("OO]");
      else {
	if(board->spaces[i][j] < 10)
	  printf("0");
	printf("%d]", board->spaces[i][j]);
      }
    }
    printf("\n");
  }
  printf("\n  %s\n\n", GetPrediction(position, playerName, usersTurn));
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

MOVELIST *GenerateMoves (position)
         POSITION position;
{
  int i, j;
  MOVELIST *head = NULL;
  MOVELIST *CreateMovelistNode();
  BOARD board = arraytoboard(position);
  for (i = 0; i < (base - 1); i++) {
    for (j = 0; j < (base - i); j++) {
      if ((i == 0) && (board->spaces[i][j] < 999))
	head = CreateMovelistNode(j + 1, head);
      else if((board->spaces[i][j] >= 999) && 
	      ((j + 1) < (base - i)) && 
	      (board->spaces[i][j+1] >= 999) && 
	      ((i+1) < base) && 
	      (board->spaces[i + 1][j] < 999))
	head = CreateMovelistNode(((i + 1) * base - sumto(i) + j + 1), head);
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
  BOOLEAN ValidMove();
  USERINPUT ret, HandleDefaultTextInput();  
  do {
    printf("%8s's move [(u)ndo/(1-%d)] :  ", playerName, sumto(base));
    ret = HandleDefaultTextInput(thePosition, theMove, playerName);
    if(ret != Continue)
      return(ret);
  }
  while (TRUE);
  return(Continue); /* this is never reached, but lint is now happy */
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

BOOLEAN ValidTextInput (input)
	STRING input;
{
  int i;
  i = atoi(input);
  return ((i > 0) && (i <= sumto(base)));
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

MOVE ConvertTextInputToMove (input)
	STRING input;
{
  return ((MOVE) atoi(input));
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

void PrintMove (move)
	MOVE move;
{
  printf("%d", move);
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

int NumberOfOptions ()
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
	return 1;
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
*************************************************************************
**         EVERYTHING BELOW THESE LINES IS LOCAL TO THIS FILE
*************************************************************************
************************************************************************/


/************************************************************************
** This is where you can put any helper functions, including your
** hash and unhash functions if you are not using one of the existing
** ones.
************************************************************************/
