/************************************************************************
**
** NAME:         miceblocks.c
**
** DESCRIPTION:  Ice Blocks
**
** AUTHOR:       Kevin Duncan, Neil Trotter
**
** DATE:         2004.05.01
**
** UPDATE HIST:  a lot of shit happened.
**
**************************************************************************/

#include <stdio.h>
#include "gamesman.h"
#include "hash.h"
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

#define X               99999
#define O               9999
#define MAX_BOARD_SIZE  5

/* External Globals */
extern GENERIC_PTR SafeMalloc ();
extern void SafeFree ();
extern VALUE *gDatabase;
extern STRING gValueString[];
/* External Globals */

/* Globals setup for use by outside functions */
POSITION gNumberOfPositions   = 0;
POSITION gInitialPosition     = 0;
POSITION gMinimalPosition     = 0;
POSITION kBadPosition         = -1;
STRING   kGameName            = "Ice Blocks";
STRING   kDBName              = "iceblocks";
BOOLEAN  kPartizan            = TRUE;
BOOLEAN  kDebugMenu           = TRUE;
BOOLEAN  kGameSpecificMenu    = TRUE;
BOOLEAN  kTieIsPossible       = TRUE;
BOOLEAN  kLoopy               = FALSE;
BOOLEAN  kDebugDetermineValue = TRUE;
void*	 gGameSpecificTclInit = NULL;
/* Globals setup for use by outside functions */

/* Help strings for this game */
STRING kHelpGraphicInterface =
"Not written yet";
STRING kHelpTextInterface =
""; 
STRING kHelpOnYourTurn =
"";
STRING kHelpStandardObjective =
"";
STRING kHelpReverseObjective =
"";
STRING kHelpTieOccursWhen =
"";
STRING kHelpExample =
"";
/* Help strings for this game */

/* Board representation for this game */
typedef enum win_condition {
  standard, tallyblocks, tallythrees
} CONDITION;
typedef int TURN;
typedef struct boardRep *BOARD;
struct boardRep{
  int **spaces;
  TURN turn;
};
int base = 4;
CONDITION WinningCondition = standard;
/* Board representation for this game */

BOOLEAN gMenu = FALSE;

/* Function declarations */
int sumto (int);
int vcfg (int *);
BOARD arraytoboard (POSITION);
POSITION boardtoarray (BOARD);
void ChangeBoardSize();
void SetWinningCondition();
/* Function declarations */


/*************************** GAME FUNCTIONS ****************************/


/************************************************************************
**
** NAME:        DebugMenu
**
** DESCRIPTION: Menu used to debub internal problems. Does nothing if
**              kDebugMenu == FALSE
** 
************************************************************************/

void DebugMenu () {
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

void GameSpecificMenu () {
  char c;
  BOOLEAN cont = TRUE;
  while (cont) {
    fflush(stdin);
    printf("\tc)\t(C)hange the size of the board (change the base length)\n"
	   "\tm)\t(M)odify the winning conditions of the game\n"
	   "\tb)\t(B)ack to the main menu\n");
    c = tolower(getc(stdin));
    switch (c) {
    case 'c':
      ChangeBoardSize();
      break;
    case 'm':
      SetWinningCondition();
      break;
    case 'b':
      cont = FALSE;
    default:
      printf("Invalid option!\n");
    }
  }
}

/************************************************************************
**
** NAME:        InitializeGame
**
** DESCRIPTION: Initialize the gDatabase, a global variable. and the other
**              local variables.
** 
************************************************************************/

void InitializeGame () {
  int i, sum = sumto(base);
  char board[sum];
  int theBoard[10] = {'x', 0, ((sum / 2) + (sum % 2)), 'o', 0, (sum / 2), '-', 0, sum, -1};
  gNumberOfPositions = generic_hash_init(sum, theBoard, vcfg);
  for(i = 0; i < sum; i++)
    board[i] = '-';
  gInitialPosition = generic_hash(board, 1);
  gMinimalPosition = gInitialPosition;
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

MOVELIST *GenerateMoves (POSITION position) {
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

POSITION DoMove (POSITION thePosition, MOVE theMove) {
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

VALUE Primitive (POSITION pos) {
  BOARD board = arraytoboard(pos);
  int i, j, countX = 0, countO = 0, color, k, m, count;
  int pointsX = 0, pointsO = 0, threesX = 0, threesO = 0;
  int dlvisited[base][base], drvisited[base][base], hvisited[base][base];
  for(i = 0; i < base; i++) {
    for(j = 0; j < base; j++) {
      dlvisited[i][j] = 0;
      drvisited[i][j] = 0;
      hvisited[i][j] = 0;
    }
  }
  for(i = 0; i < base; i++) {
    for(j = 0; j < (base - i); j++) {
      if(board->spaces[i][j] < 999)
	return undecided;
      color = board->spaces[i][j];
      for(k = i, m = j, count = 0; k < base && m >= 0 && 
	    ((WinningCondition == tallythrees) ? TRUE : !dlvisited[k][m]) &&
	    board->spaces[k][m] == color; k++, m--) {
	count++; 
	dlvisited[k][m] = 1; 
      }
      if((count > 1) && (color == X))
	countX += count;
      else if ((count > 1) && (color == O))
	countO += count;
      if((count > 2) && (color == X)) {
	pointsX += 3 + (count - 3) * 2;
	threesX++;
      }
      else if((count > 2) && (color == O)) {
	pointsO += 3 + (count - 3) * 2;
	threesO++;
      }
      for (k = i, m = j, count = 0; k < base && m < (base - k) &&
	     ((WinningCondition == tallythrees) ? TRUE : !drvisited[k][m]) && 
	     board->spaces[k][m] == color; k++) {
	count++;
	drvisited[k][m] = 1;
      }
      if ((count > 1) && (color == X))
	countX += count;
      else if ((count > 1) && (color == O))
	countO += count;
      if((count > 2) && (color == X)) {
	pointsX += 3 + (count - 3) * 2;
	threesX++;
      }
      else if((count > 2) && (color == O)) {
	pointsO += 3 + (count - 3) * 2;
	threesO++;
      }
      for (k = i, m = j, count = 0; m < (base - k) && 
	     ((WinningCondition == tallythrees) ? TRUE : !hvisited[k][m]) && 
	     board->spaces[k][m] == color; m++) { 
	count++;
	hvisited[k][m] = 1;
      }
      if((count > 1) && (color == X))
	countX += count;
      else if ((count > 1) && (color == O))
	countO += count;
      if((count > 2) && (color == X)) {
	pointsX += 3 + (count - 3) * 2;
	threesX++;
      }
      else if((count > 2) && (color == O)) {
	pointsO += 3 + (count - 3) * 2;
	threesO++;
      }
    }
  }
  switch (WinningCondition) {
  case standard:
    if(pointsX > pointsO) {
      if(board->turn == X) 
	return (gStandardGame ? win : lose);
      else
	return (gStandardGame ? lose : win);
    }
    else if(pointsO > pointsX) {
      if(board->turn == O)
	return (gStandardGame ? win : lose);
      else 
	return (gStandardGame ? lose : win);
    }
    else
      return tie;
  case tallyblocks:
    if(countX > countO) {
      if(board->turn == X) 
	return (gStandardGame ? win : lose);
      else
	return (gStandardGame ? lose : win);
    }
    else if(countO > countX) {
      if(board->turn == O) 
	(gStandardGame ? win : lose);
      else
	(gStandardGame ? lose : win);
    }
    else
      return tie;
 case tallythrees:
   if(threesX > threesO) {
      if(board->turn == X) 
	return (gStandardGame ? win : lose);
      else
	return (gStandardGame ? lose : win);
    }
    else if(threesO > threesX) {
      if(board->turn == O) 
	(gStandardGame ? win : lose);
      else
	(gStandardGame ? lose : win);
    }
    else
      return tie;
  }
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
  char input[sumto(base)];
  printf("type in a %d character string of X's, O's and -'s\nNote that X always goes first: ", sumto(base));
  scanf("%s", input);
  return generic_hash(input, 1);
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

void PrintPosition (POSITION position, STRING playerName, BOOLEAN usersTurn) {
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
  if(!gMenu)
    printf("\n  %s\n\n", GetPrediction(position, playerName, usersTurn));
}

/************************************************************************
**
** NAME:        GetAndPrintPlayersMove
**
** DESCRIPTION: This finds out if the player wanted an undo or abort or not.
**              If so, return Undo or Abort and don't change theMove.
**              Otherwise get the new theMove and fill the pointer up.
** 
** INPUTS:      POSITION thePosition : The position the user is at. 
**              MOVE theMove         : The move to fill with user's move. 
**              STRING playerName     : The name of the player whose turn it is
**
** OUTPUTS:     USERINPUT             : Oneof( Undo, Abort, Continue )
**
** CALLS:       ValidMove(MOVE, POSITION)
**              BOOLEAN PrintPossibleMoves(POSITION) ...Always True!
**
************************************************************************/

USERINPUT GetAndPrintPlayersMove (POSITION thePosition, MOVE *theMove, STRING playerName) {
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
** NAME:        PrintComputersMove
**
** DESCRIPTION: Nicely format the computers move.
** 
** INPUTS:      MOVE    computersMove : The computer's move. 
**              STRING  computersName : The computer's name. 
**
************************************************************************/

void PrintComputersMove(MOVE computersMove, STRING computersName) {
  printf("%8s's move              : %d\n\n", computersName, computersMove);
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
  printf("%d", move);
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

MOVE ConvertTextInputToMove (STRING input) {
  return ((MOVE) atoi(input));
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

int NumberOfOptions () {
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

int getOption() {
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

void setOption(int option) {
}

/************************************************************************
**
** NAME:        SetTclCGameSpecificOptions
**
** DESCRIPTION: Set the C game-specific options (called from Tcl)
**              Ignore if you don't care about Tcl for now.
** 
************************************************************************/

void SetTclCGameSpecificOptions (int options[]) {
}


/***********************************************************************
 ***********************************************************************
 ** Helper functions local to this file...
 ***********************************************************************
 **********************************************************************/

/***********************************************************************
**
** NAME:         sumto
**
** DESCRIPTION:  Returns a cumulative sum from 1 to i.
**
** INPUT:        i, the integer to sum to.
**
** OUTPUT:       the cumulative sum.
**
***********************************************************************/
int sumto (int i) {
  int ans = 0;
  while(i > 0) {
    ans += i;
    i--;
  }
  return ans;
}

/* Required function by Michel's hash for dartboard behavior from hash */
int vcfg (int *this_cfg) {
  return ((this_cfg[0] == this_cfg[1]) || (this_cfg[0] == (this_cfg[1] + 1))); 
}

/************************************************************************
 **
 ** NAME:         arraytoboard
 **
 ** DESCRIPTION:  Intermediate function between our board representation
 **               and Michel's hash.  Takes in the POSITION, unhashes it
 **               and then turns the resulting array into our board
 **               representation.
 **
 ** INPUT:        position, the hashed POSITION of the board.
 **
 ** OUPTUT:       A BOARD, our board representation.
 **
 ***********************************************************************/

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



/************************************************************************
 **
 ** NAME:         boardtoarray
 **
 ** DESCRIPTION:  Intermediate function between our board representation
 **               and Michel's hash.  Takes in the BOARD, converts it
 **               into an the array appropriate for Michel's hash and
 **               then calls Michel's hash to get the hashed POSITION of
 **               the board.
 **
 ** INPUT:        BOARD, the representation of that board.
 **
 ** OUPTUT:       A POSITION, the hashed value of that BOARD.
 **
 ***********************************************************************/

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
 ** NAME:         ChangeBoardSize
 **
 ** DESCRIPTION:  Handles menu option to change the board size.
 **
 ***********************************************************************/

void ChangeBoardSize () {
  int change;
  BOOLEAN cont = TRUE;
  while (cont) {
    fflush(stdin);
    cont = FALSE;
    printf("Current board with base %d:\n", base);
    gMenu = TRUE;
    PrintPosition(gInitialPosition, "Fred", 0);
    gMenu = FALSE;
    printf("Enter the new base length (2 - %d):  ", MAX_BOARD_SIZE);
    scanf("%d", &change);
    if(change > MAX_BOARD_SIZE || change < 2) {
      printf("Invalid base length!\n");
      cont = TRUE;
    }
    else
      base = change;
  }
}

/************************************************************************
 **
 ** NAME:         SetWinningCondition
 **
 ** DESCRIPTION:  Handles menu option to change the winning condition.
 **
 ***********************************************************************/

void SetWinningCondition () {
  char c;
  BOOLEAN cont = TRUE;
  while(cont) {
    fflush(stdin);
    cont = FALSE;
    printf("Options for the winning condition:\n"
	   "\ts)\t(S)tandard point tally\n"
	   "\tm)\t(M)ost number of blocks in a row\n"
	   "\tn)\tMost (n)umber of three's in a row\n"
	   "Enter your choice for the winning condition:  ");
    c = tolower(getc(stdin));
    switch (c) {
    case 's':
      WinningCondition = standard;
      break;
    case 'm':
      WinningCondition = tallyblocks;
      break;
    case 'n':
      WinningCondition = tallythrees;
      break;
    default:
      cont = TRUE;
    }
  }
}

/* end of file. */
