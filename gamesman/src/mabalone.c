/************************************************************************
**
** NAME:        mabalone.c
**
** DESCRIPTION: Abalone
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
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include "hash.h"
#include <string.h>

extern STRING gValueString[];
POSITION gNumberOfPositions  = 139675536;

POSITION gInitialPosition    = 8985892;
POSITION gMinimalPosition    = 0;
POSITION kBadPosition        = -1;

STRING   kGameName           = "";
STRING   kDBName             = "";
BOOLEAN  kPartizan           = TRUE; 
BOOLEAN  kSupportsHeuristic  = FALSE;
BOOLEAN  kSupportsSymmetries = FALSE;
BOOLEAN  kSupportsGraphics   = FALSE;
BOOLEAN  kDebugMenu          = FALSE;
BOOLEAN  kGameSpecificMenu   = FALSE;
BOOLEAN  kTieIsPossible      = FALSE;
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

#define BOARDSIZE 19
#define NULLSLOT 99
char gBoard[19];

char whosTurn='x';


/*************************************************************************
**
** Above is where you put your #define's and your global variables, structs
**
*************************************************************************/

/*
** Function Prototypes:
*/

//Function prototypes here.

// External
extern GENERIC_PTR	SafeMalloc ();
extern void		SafeFree ();

// Internal
int destination (int, int);
int move_hash (int, int, int);

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
int max;
int init;

void InitializeGame()
{
  gBoard[0]='x';
  gBoard[1]='-';
  gBoard[2]='x';
  gBoard[3]='x';
  gBoard[4]='x';
  gBoard[5]='x';
  gBoard[6]='x';
  gBoard[7]='-';
  gBoard[8]='-';
  gBoard[9]='-';
  gBoard[10]='-';
  gBoard[11]='-';
  gBoard[12]='o';
  gBoard[13]='o';
  gBoard[14]='o';
  gBoard[15]='o';
  gBoard[16]='o';
  gBoard[17]='-';
  gBoard[18]='o';
  
  char boardcopy[19];

  int init_array[] = {'o', 5, 6, 'x', 5, 6, '-', 7, 9, -1};
  int count;

  max = generic_hash_init(19,init_array,NULL);
  printf("%d  # of hash positions!\n",max);
  
  printf("1\n");
  init = generic_hash(gBoard);
  printf("1.5\n");
  generic_unhash(init,boardcopy);
  printf("2\n");
  printf("%d  is the initial position\n",init);

  for (count = 0; count < 19; count++) {
    printf("%d: %c\n", count, boardcopy[count]);
  }
  
  printf("%d is result of primitive on inital board", game_over(boardcopy));
  
  gDatabase = (VALUE *) SafeMalloc(gNumberOfPositions * sizeof(VALUE));
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
**	        destination ()
*************************************************************************/
POSITION DoMove(thePosition, theMove)
     POSITION thePosition;
     MOVE theMove;
{
  printf("DoMove\n");
  int destination(int,int);

  generic_unhash(thePosition, gBoard);
  int direction;
  int slot1;
  int slot2;
  int dest1;
  int dest2;
  int pushee_dest;
  BOOLEAN twopieces = TRUE;

  direction = theMove % 10;
  theMove = theMove/10;

  slot1 = theMove % 100;
  slot2 = theMove/100;
  
  if (slot2 == NULLSLOT) {
    twopieces = FALSE;
    }

  dest1 = destination(slot1, direction);
  if (twopieces) dest2 = destination(slot2, direction);

  if (gBoard[dest1] != '-') {
    pushee_dest = destination(dest1,direction);
    if (pushee_dest != NULLSLOT) 
      gBoard[pushee_dest] = gBoard[dest1];
  }
  gBoard[dest1] = gBoard[slot1];
  gBoard[slot1] = '-';

  if (twopieces) {
    gBoard[dest2] = gBoard[slot2];
    gBoard[slot2] = '-';
  }

  return (generic_hash(gBoard));
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
  BOOLEAN game_over(char[]);

  generic_unhash(h,gBoard);
  if (game_over(gBoard))
    return (lose);
  return (undecided);
}

BOOLEAN game_over(char theBoard[]){
  int count, x = 0, o = 0;

  for (count = 0; count < BOARDSIZE; count++) {
    if (theBoard[count] == 'x') {
      x++;
    }
    else if (theBoard[count] == 'o') {
      o++;
    }
    /*printf("position %d is a %c\n", count, theBoard[count]);*/
  }
  printf("I counted %d os and %d xs\n", o, x);
  return (o<6 || x<6);
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
  generic_unhash(position, gBoard);

  printf("    (%c)(%c)(%c) \n", gBoard[16], gBoard[17],gBoard[18]);
  printf("  (%c)(%c)(%c)(%c) \n", gBoard[12], gBoard[13], gBoard[14], gBoard[15]);
  printf("(%c)(%c)(%c)(%c)(%c)\n",gBoard[7], gBoard[8],gBoard[9],gBoard[10], gBoard[11]);
  printf("  (%c)(%c)(%c)(%c)\n",gBoard[3], gBoard[4],gBoard[5],gBoard[6]);
  printf("    (%c)(%c)(%c)\n", gBoard[0], gBoard[1],gBoard[2]);
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
**              Primitive
**
************************************************************************/

MOVELIST *GenerateMoves(position)
         POSITION position;
{
  MOVELIST *head = NULL;
  MOVELIST *CreateMovelistNode(); /* In gamesman.c */
  VALUE Primitive();
  int slot, dest1, dest2, dest3, direction, ssdir;

  generic_unhash(position,gBoard);
  if (Primitive(position) == undecided) {
    for (slot = 0; slot < 19 ; slot++) {
      if (gBoard[slot] == whosTurn) {
	for (direction = 1; direction <= 3; direction++) {
	  
	  dest1 = destination(slot, direction);
	  
	  /*Single Piece Moves*/
	  if (gBoard[dest1] == '-') {
	    head = CreateMovelistNode(move_hash(slot,NULLSLOT,direction), head);
	  }
	  
	  /*Double Piece Moves*/
	  else if (gBoard[dest1] == whosTurn) {
	    /*Test for pushes in both directions*/
	    /*the oppositve direction is represented by the negative of that direction*/
	    do {
	      direction = 0 - direction;
	      
	      dest2 = destination(dest1,direction);
	      
	      if (dest2 != NULLSLOT) {
		dest3 = destination(dest2,direction);
		if (gBoard[dest2] != whosTurn && (gBoard[dest3]== '-' || gBoard[dest3] == NULLSLOT)) {
		  head = CreateMovelistNode(move_hash(slot, dest1, direction), head);
		}
	      }
	    } while (direction < 0);
	    
	    /*Test for possible side steps*/
	    for (ssdir = -3; ssdir <= 3; ssdir++) {
	      if ((ssdir % direction) != 0) {/*skip over nonexistant zero direction as well as push direction*/
		dest2 = destination(slot,ssdir);
		dest3 = destination(dest1,ssdir);
		if ((dest2 != NULLSLOT && dest3 != NULLSLOT) && (gBoard[dest2] == gBoard[dest3] == '-')) {
		  head = CreateMovelistNode(move_hash(slot,dest1,ssdir), head);
		}
	      }
	    }
	  }
	}   
      }
    }
    return(head);
  }
  return(NULL);
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
  return (HandleDefaultTextInput(thePosition, theMove, playerName));
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
  return TRUE;
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
  return(atoi(input));
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
  return 0;
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
int destination(int slot, int direction) {
  int kill_lowleft[] = {5, 7, 3, 0, 1, 2};
  int kill_left[] = {5, 0, 3, 7, 12, 16};
   int kill_upleft[] = {5, 7, 12, 16, 17, 18};
  int kill_upright[] = {5, 16, 17, 18, 15, 11};
  int kill_right[] = {5, 18, 15, 11, 6, 2};
  int kill_lowright[] = {5, 11, 6, 2, 1, 0};
  int change;


  if ((direction == -3) || (direction == -2)) {
    if (slot<=6 || slot>=16) change = 3;
    else change = 4;
  }
  if (direction==2 || direction==3) {
    if (slot<=2 || slot>=12) change = 3;
    else change = 4;
  }
      


  switch(direction) {
  case -3:
    if (member(slot, kill_lowleft)) return 99;
    else return (slot - (change + 1));
  case -1:
    if (member(slot, kill_left)) return 99;
    else return (slot-1);
  case 2:
    if (member(slot, kill_upleft)) return 99;
    else return (slot + change);
case 3:
    if (member(slot, kill_lowleft)) return 99;
    else return (slot + change + 1);
case 1:
    if (member(slot, kill_lowleft)) return 99;
    else return (slot+1);
case -2:
    if (member(slot, kill_lowleft)) return 99;
    else return (slot - change);

  }
}

BOOLEAN member(int slot, int places[]) {
  int length = places[0];
  int i = 1;


  while (i<=length) {
    if (places[i] == slot) {
      return TRUE;
    }
  }
  return FALSE;
}
  
int move_hash(int slot1, int slot2, int direction) {
  int bigger, smaller;
  if (slot2 > slot1) {
    bigger = slot2;
    smaller = slot1;
  }
  else {
    bigger = slot1;
    smaller = slot2;
  }

  if (direction > 0) {
    return (direction + (1000 * bigger) + (100000 * smaller));
  }
  else {
    return (-1 * ((-1 * direction) + (1000 * smaller) + (100000 * bigger)));
  }
}
