/************************************************************************
**
** NAME:        mabalone.c
**
** DESCRIPTION: Abalone
**
** AUTHOR:      Michael Mottmann & Melinda Franco
**
** DATE:        4/6/04 - Working and all nice and pretty and stuff
**
** UPDATE HIST: WHAT ONCE WAS BROKEN NOW IS FIXED
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
#include <math.h>

extern STRING gValueString[];
POSITION gNumberOfPositions  = 924;

POSITION gInitialPosition    = 9;
POSITION gMinimalPosition    = 0;
POSITION kBadPosition        = -1;

STRING   kGameName           = "abalone";
STRING   kDBName             = "Abalone";
BOOLEAN  kPartizan           = TRUE; 
BOOLEAN  kSupportsHeuristic  = FALSE;
BOOLEAN  kSupportsSymmetries = FALSE;
BOOLEAN  kSupportsGraphics   = FALSE;
BOOLEAN  kDebugMenu          = TRUE;
BOOLEAN  kGameSpecificMenu   = TRUE;
BOOLEAN  kTieIsPossible      = FALSE;
BOOLEAN  kLoopy               = TRUE;
BOOLEAN  kDebugDetermineValue = FALSE;
void*	 gGameSpecificTclInit = NULL;

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

#define NULLSLOT 99 
int N = 2;
int MISERE = 0;
int SS = 0;
int XHITKILLS = 1;
int PIECEMOD = 0;

BOOLEAN DEBUGGING = FALSE;

int maxPieces;
int BOARDSIZE;
char *gBoard;

struct row {
  int size;
  int start_slot;
};

struct row **rows;

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
struct row * makerow (int, int);
int m_generic_hash(char *);
void m_generic_unhash(int, char *);
void printrow (int, int);
void printlines (int, int);
void changeBoard ();
int b_size (int);
int def_start_pieces (int);
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
  if (DEBUGGING) printf("start initialize game\n");

  rows = (struct row **) SafeMalloc ((2*N - 1) * sizeof(struct row *));
  
  int rowsize = N, rownum, slot = 0, bsize = 0;
  for (rownum = 0; rownum < N - 1; rownum++) {
    rows[rownum] = makerow(rowsize,slot);
    slot += rowsize;
    rowsize++;
  }
  for (rownum = N - 1; rownum <= 2*N - 2; rownum++) {
    rows[rownum] = makerow(rowsize,slot);
    slot += rowsize;
    rowsize--;
  }
  
  BOARDSIZE = b_size(N);

 
  /* long nasty thing to initialize n-sized board */

  gBoard = (char *) SafeMalloc (BOARDSIZE * sizeof(char));

  maxPieces = def_start_pieces(N) + PIECEMOD;
  int count, x_pieces_left = maxPieces, o_pieces_left = maxPieces, start, size, stop;
  double space;

  for (rownum = 0; rownum < 2 * N - 1; rownum++) {
    start = (*rows[rownum]).start_slot;
    size = (*rows[rownum]).size;

    if (x_pieces_left >= size) {
      for (count = start; count < start + size; count++)
	{
	  gBoard[count] = 'x';
	  x_pieces_left--;
	}
    }
    else if (x_pieces_left > 0) {
      space = (size - x_pieces_left)/2;
  
      for (count = start; count < start + (int)floor(space); count++) {
	gBoard[count] = '*';
      }
      stop = start + (int)floor(space) + x_pieces_left;
      for (; count < stop; count++) {
	gBoard[count] = 'x';
	x_pieces_left--;
      }
      for (;count < start + size; count++) {
	gBoard[count] = '*';
      }
    }
    else if (o_pieces_left <= (BOARDSIZE - start - size)) {
      for (count = start; count < start + size; count++) {
	gBoard[count] = '*';
      }
    }
    else if (o_pieces_left > BOARDSIZE - start - size) {
      space = ((double)(size - (o_pieces_left - (BOARDSIZE - start - size))))/2;
      for (count = start; count < start + (int)ceil(space); count++) {
	gBoard[count] = '*';
      }
      stop = start + (int)ceil(space) + (o_pieces_left - (BOARDSIZE - start - size));
      for (;count < stop; count++) {
	gBoard[count] = 'o';
	o_pieces_left--;
      }
      for (;count < start + size; count++) {
	gBoard[count] = '*';
      }
    }
    else {
      for (count = start; count < start + size; count++) {
	gBoard[count] = 'o';
	o_pieces_left--;
      }
    }
  }
     
  
  /* printf("testing...\n");
  for (count = 0; count < BOARDSIZE; count++) {
    printf("%d: %c\n", count, gBoard[count]);
    }*/
  
  int init_array[10];
  init_array[0] = 'o';
  init_array[3] = 'x';
  init_array[6] = '*';
  init_array[9] = -1;

  init_array[1] = maxPieces - XHITKILLS;
  init_array[2] = maxPieces;
  init_array[4] = maxPieces - XHITKILLS;
  init_array[5] = maxPieces;
  init_array[7] = BOARDSIZE - 2 * init_array[2];
  init_array[8] = BOARDSIZE - 2 * init_array[1];

  /* printf("init array:\n");
  for (count = 0; count < 10; count++) {
    printf("%d: %d\n", count, init_array[count]);
    }*/

  max = generic_hash_init(BOARDSIZE,init_array,NULL);
  /*  printf("%d  # of hash positions!\n",max);*/
  
  init = generic_hash(gBoard, 1);
  /*  printf("%d  is the initial position\n",init);*/
  
  /*
  generic_unhash(init,gBoard);
  printf("testing...\n");
    for (count = 0; count < BOARDSIZE; count++) {
    printf("%d: %c\n", count, gBoard[count]);
  }*/
  
  /*gDatabase = (VALUE *) SafeMalloc(gNumberOfPositions * sizeof(VALUE));*/
  
  gNumberOfPositions  = max;
  gInitialPosition    = init;
  gMinimalPosition    = init;
 
  if (DEBUGGING) printf("end initializegame\n");
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
  int selection;
  printf("1.\t Change the value of N, the edge size of the hexagon board\n");
  if (MISERE == 0)
    printf("2.\t Toggle from Standard to Misere\n");
  else
    printf("2.\t Toggle from Misere to Standard\n");
  if (SS == 0)
    printf("3.\t Toggle from Side Steps Allowed to No Side Steps\n");
  else
    printf("3.\t Toggle from No Side Steps to Side Steps Allowed\n");
  printf("4.\t Return to the previous menu\n\nSelect Option:  ");
  (void) scanf("%d", &selection);
  
  if (selection == 1) {
    changeBoard();
  } else if (selection == 2) {
    if (MISERE == 0)
      MISERE = 1;
    else
      MISERE = 0;
    
    SafeFree(rows);
    SafeFree(gBoard);
    InitializeGame();
    GameSpecificMenu();
  } else if (selection == 3) {
    if (SS == 0)
      SS = 1;
    else
      SS = 0;
    
    SafeFree(rows);
    SafeFree(gBoard);
    InitializeGame();
    GameSpecificMenu();

  } else if (selection == 4)
    return;
  else {
    printf("\n\n\n Please select a valid option...\n");
    GameSpecificMenu();
  }
}

void changeBoard()
{
  int size;
  printf("Enter the new N:  ");
  (void) scanf("%u", &size);
  if (size < 2) {
    printf("N must be at least 2\n");
    changeBoard();
  }
  else {
    printf("Changing N to %d ...\n", size);
    N = size;
    SafeFree(rows);
    SafeFree(gBoard);
    InitializeGame();
  }
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
  if (DEBUGGING) printf("Starting Do Move with input: %d\n", theMove);
  int destination(int,int);
  int whoseTurn;
  generic_unhash(thePosition, gBoard);
  int direction;
  int slot1;
  int slot2;
  int dest1;
  int dest2;
  int pushee_dest;
  BOOLEAN twopieces = TRUE;
  int dir_shift = 10;
  int piece_shift = 100;

  direction = theMove % dir_shift;
  theMove = theMove/dir_shift;

  if (theMove < 0) {
    theMove = 0 - theMove;
  }

  slot1 = theMove % piece_shift;
  slot2 = theMove/piece_shift;
  
  if (slot2 == NULLSLOT) {
    twopieces = FALSE;
  }
  
  if (slot1 == NULLSLOT) {
    twopieces = FALSE;
    slot1 = slot2;
  }
  

  dest1 = destination(slot1, direction);
  if (twopieces) dest2 = destination(slot2, direction);

  /*printf("dest1 = %d, direction = %d, gBoard[dest1] = %c\n", dest1, direction, gBoard[dest1]);*/

  if (gBoard[dest1] != '*') {
    pushee_dest = destination(dest1,direction);
    if (pushee_dest != NULLSLOT) 
      gBoard[pushee_dest] = gBoard[dest1];
  }

 
  gBoard[dest1] = gBoard[slot1];
  gBoard[slot1] = '*';

  if (twopieces) {
    gBoard[dest2] = gBoard[slot2];
    gBoard[slot2] = '*';
  }

  if (whoseMove(thePosition) == 1) {
    whoseTurn = 2;
  }
  else {
    whoseTurn = 1;
  }

  if (DEBUGGING)  printf("finished do move\n");
  return (generic_hash(gBoard,whoseTurn));
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
** NAME:        PrintComputersMove*
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
  printf("%8s's move   : ", computersName);
  PrintMove(computersMove);
  printf("\n");
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
  if (DEBUGGING) printf("prim\n");
  BOOLEAN game_over(char[]);

  generic_unhash(h,gBoard);
  
  if (game_over(gBoard)) {
    if (MISERE == 1) {
      if (DEBUGGING) printf("end prim\n");
      return (win);
    }
    else {
      if (DEBUGGING) printf("end prim\n");
      return (lose);
    }
  }
  if (DEBUGGING) printf("end prim\n");
  return (undecided);
}

BOOLEAN game_over(char theBoard[]){
  int count, x = 0, o = 0, lose;
  
  lose = maxPieces - XHITKILLS;

  for (count = 0; count < BOARDSIZE; count++) {
    if (theBoard[count] == 'x') {
      x++;
    }
    else if (theBoard[count] == 'o') {
      o++;
    }
    /*printf("position %d is a %c\n", count, theBoard[count]);*/
  }
  /*printf("I counted %d os and %d xs\n", o, x);*/
  return (o<=lose || x<=lose);
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
  if (N < 4) {
    generic_unhash(position, gBoard);
  }
  int r, spacing;


  if (N < 4) {
    /* messy centering spacing issues for sizeable first line*/
    printf("\n");
    spacing = (4 * (*rows[N - 1]).size) + 8;
    for (r = 0; r < spacing/2; r++)
      printf (" ");
    printf("BOARD");
    for (r = 0; r < spacing/2; r++)
      printf (" ");
    if (spacing % 2 == 1)
      printf (" ");
    

    if (N == 2)
      spacing = (4 * (*rows[N - 1]).size) + 2;
    else
      spacing = (5 * (*rows[N - 1]).size) + 2;
    
    for (r = 0; r < spacing/2; r++)
      printf (" "); 
    printf("LEGEND");
    for (r = 0; r < spacing/2; r++)
      printf (" ");
    if (spacing % 2 == 1)
      printf (" ");
    
    printf(" DIRECTIONS\n\n");
  }
    

  /* edge of hex board */
  /*top edge*/
  printf("       ");
  for (r = 0; r < N - 2; r++) {
    printf("  ");
  }
  printf("---");
  for (r = 0; r < N; r++){
    printf("----");
  }
  printf("        ");
  printf("\n");

  /*second edge*/
  printf("      ");
  for (r = 0; r < N - 2; r++) {
    printf("  ");
  }
  printf("/ ");
  for (r = 0; r < N; r++){
    printf("----");
  }
  printf("- \\");
  if (N < 4) {
    printf("      ");
    for (r = 0; r < N - 2; r++) {
      printf("  ");
    }
    printf(" |\n");
  }
  else {
    printf("\n");
  }
  
  /*spacer*/
  printf("     ");
  for (r = 0; r < N - 2; r++) {
    printf("  ");
  }
  printf("/ /");
  for (r = 0; r < N; r++) {
    printf("    ");
  }
  printf(" \\ \\");
  if (N < 4) {
    printf("     ");
    for (r = 0; r < N - 2; r++) {
      printf("  ");
    }
    printf(" |\n");
  }
  else {
    printf("\n");
  }

    
  /*main board printing here*/
  for (r = 0; r < (2 * N) - 1; r++) {
    if (N < 4){
      printf("  ");
      printrow(r, 0);
      printf("   |   ");
      printrow(r, 1);
      
      if (r == N - 2)
	printf("     NW   NE       To move one piece:");
      else if (r == N - 1)
	printf("     W -*- E      To move two pieces:");
      else if (r == N)
	printf("     SW   SE          (order of pieces doesn't matter)");
      else
	printf("               ");
      if (r != 2 * N - 2) {
	printf("\n ");
	printlines(r, 0);
	printf("   |   ");
	printlines(r, 1);
	printf("  ");
	
	if (r == N - 1)
	  printf("     / \\            direction, piece1, piece2");
	else if (r == N - 2)
	  printf("     \\ /            direction, piece1");
	else
	  printf("                 ");
      }
      printf("\n");
    }
    else {
      printf("  ");
      printrow(r, 0);
      printf("\n");
      if (r != 2 * N - 2) {
	printf(" ");
	printlines(r, 0);
	printf("\n");
      }
    }
  }

    
/* edge of hex board */
  /*spacer*/
  printf("     ");
  for (r = 0; r < N - 2; r++) {
    printf("  ");
  }
  printf("\\ \\");
  for (r = 0; r < N; r++) {
    printf("    ");
  }
  printf(" / /");
  if (N < 4) {
    printf("     ");
    for (r = 0; r < N - 2; r++) {
      printf("  ");
    }
    printf(" |\n");
  }
  else {
    printf("\n");
  }

  /*first edge*/
  printf("      ");
  for (r = 0; r < N - 2; r++) {
    printf("  ");
  }
  printf("\\ ");
  for (r = 0; r < N; r++){
    printf("----");
  }
  printf("- /");
  if (N < 4) {
    printf("      ");
    for (r = 0; r < N - 2; r++) {
      printf("  ");
    }
    printf(" |\n");
  }
  else {
    printf ("\n");
  }

  /*bottom edge*/
  printf("       ");
  for (r = 0; r < N - 2; r++) {
    printf("  ");
  }
  printf("---");
  for (r = 0; r < N; r++){
    printf("----");
  }
  printf("\n\n\n");

    
  

  /*
printf("\\-------------------------------");
  for (r = 0; r < (*rows[N - 1]).size; r++) {
    if (N == 2)
      printf("--------");
    else
      printf("---------");
  }
  printf("/\n");*/
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
  if (DEBUGGING) printf("generate\n");
  MOVELIST *head = NULL;
  MOVELIST *CreateMovelistNode(); /* In gamesman.c */
  VALUE Primitive();
  int slot, slot2, dest0, dest1, dest2, direction, ssdir;
  char whoseTurn;

  if (whoseMove(position) == 1) {
    whoseTurn = 'x';
  }
  else {
    whoseTurn = 'o';
  }

  generic_unhash(position,gBoard);
  
  if (Primitive(position) == undecided) {
    for (slot = 0; slot < BOARDSIZE ; slot++) {
      if (gBoard[slot] == whoseTurn) {
	for (direction = 1; direction <= 3; direction++) {
	  
	  slot2 = destination(slot, direction);
	  
	  /*Single Piece Moves*/
	  dest0 = destination(slot, (0 - direction));
	  if (gBoard[slot2] == '*') {
	    head = CreateMovelistNode(move_hash(slot,NULLSLOT,direction), head);
	  }
	  if (gBoard[dest0] == '*') {
	    head = CreateMovelistNode(move_hash(slot,NULLSLOT,(0 - direction)), head);
	  }
	  
	  /*Double Piece Moves*/
	  if (gBoard[slot2] == whoseTurn) {
	    /*Test for pushes in both directions*/
	    /*the oppositve direction is represented by the negative of that direction*/
	    
	    dest1 = destination(slot2,direction);
	    
	    if (dest1 != NULLSLOT) {
	      dest2 = destination(dest1,direction);
	      if ((gBoard[dest1] != whoseTurn) && ((gBoard[dest2]== '*') ||(dest2 == NULLSLOT)||(gBoard[dest1] == '*'))) {
		head = CreateMovelistNode(move_hash(slot, slot2, direction), head);
	      }
	    }
	    direction = 0 - direction; 
	    dest1 = destination(slot,direction);
	    
	    if (dest1 != NULLSLOT) {
	      dest2 = destination(dest1,direction);
	      if ((gBoard[dest1] != whoseTurn) && ((gBoard[dest2]== '*') || (dest2 == NULLSLOT)||(gBoard[dest1] == '*'))) {
		head = CreateMovelistNode(move_hash(slot, slot2, direction), head);
	      }
	    }
	    direction = 0 - direction;
	      
	    
	    if (SS == 0) { 
	      /*Test for possible side steps*/
	      for (ssdir = -3; ssdir <= 3; ssdir++) {
		if ((ssdir != 0) && (ssdir != direction) && (ssdir != 0 - direction)) {/*skip over nonexistant zero direction as well as push direction*/
		  dest1 = destination(slot,ssdir);
		  dest2 = destination(slot2,ssdir);
		  
		  if ((dest1 != NULLSLOT) && (dest2 != NULLSLOT) && (gBoard[dest1] == '*') && (gBoard[dest2] == '*')) {
		    head = CreateMovelistNode(move_hash(slot,slot2,ssdir), head);
		  }
		  
		  
		  if (slot == 0) {
		    /*printf("slot is 0, slot2 is %d, dest1 is %d, gBoard[dest1] is %c, dest2 is %d, gBoard[dest2] is %c, in direction %d\n", slot2, dest1, gBoard[dest1], dest2, gBoard[dest2], ssdir);*/
		    
		  }
		}
	      }
	    }
	  }
	}   
      }
    }
    if (DEBUGGING) printf("end gen\n");
    return(head);
  }
  if (DEBUGGING) printf("end gen\n");
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
  USERINPUT ret;
  do {
    printf("for a list of valid moves, press ?\n\n");
    printf("%8s's move :  ", playerName);
  
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

BOOLEAN ValidTextInput(input)
     STRING input;
{
  int n=0, p1, p2;

  /*skip whitespace*/
  while ((input[n] == ' ') || (input[n] == '[')) {
    n++;
  }

  /*get first piece*/
  if ((input[n] >= '0') && (input[n] <= '9')) {
    p1 = input[n] - '0';
    n++;
    if ((input[n] >= '0') && (input[n] <= '9')) {
      p1 = p1 * 10 + (input[n] - '0');
      n++;
    }
    p1--;
    if (p1 >= BOARDSIZE || p1 < 0) {
      printf("why am I here?\n");
      return FALSE;
    }
  }
  else {
    return FALSE;
  }

  /*skip whitespace and commas*/
  while ((input[n] == ' ') || (input[n] == ',') || (input[n] == ']')) {
    n++;
  }

  /*get second piece, if it exists*/
  if ((input[n] >= '0') && (input[n] <= '9')) {
    p2 = input[n] - '0';
    n++;
    if ((input[n] >= '0') && (input[n] <= '9')) {
      p2 = p2 * 10 + (input[n] - '0');
    }
    n++;
    p2--;
    if (p2 >= BOARDSIZE || p2 < 0) {
      return FALSE;
    }
  }
  
  printf("or here?\n");

  /*skip whitespace and commas*/
  while ((input[n] == ' ') || (input[n] == ',') || (input[n] == ']')) {
    n++;
  }

  /*get direction*/
  if ((input[n] == 'N') || (input[n] == 'n')) {
    n++;
    if ((input [n] == 'W') || (input[n] == 'w')) {
      return TRUE;
    }
    else if ((input[n] == 'E') ||(input [n] == 'e')) {
     return TRUE;
    }
    else {
      return FALSE;
    }
  }
  else if (input[n] == 'S' || input[n] == 's') {
    n++;
    if (input [n] == 'W' || input[n] == 'w') {
      return TRUE;
    }
    else if (input[n] == 'E' || input [n] == 'e') {
      return TRUE;
    }
    else {
      return FALSE;
    }
  }
  else if (input[n] == 'W' || input[n] == 'w') {
    return TRUE;
  }
  else if (input[n] == 'E' || input[n] == 'e') {
    return TRUE;
  }
  else
    return FALSE;
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
  if (DEBUGGING) printf("Starting conversion\n");
  int n=0, dir, p1, p2;

  /*skip whitespace*/
  while ((input[n] == ' ') || (input[n] == '[')) {
    n++;
  }
  
  /*get first piece*/
  if ((input[n] >= '0') && (input[n] <= '9')) {
    p1 = input[n] - '0';
    n++;
    if ((input[n] >= '0') && (input[n] <= '9')) {
      p1 = p1 * 10 + (input[n] - '0');
      n++;
    }
    p1--;
  }
 
  /*skip whitespace and commas*/
  while ((input[n] == ' ') || (input[n] == ',') || (input[n] == ']')) {
    n++;
  }

  /*get second piece, if it exists*/
  if ((input[n] >= '0') && (input[n] <= '9')) {
    p2 = input[n] - '0';
    n++;
    if ((input[n] >= '0') && (input[n] <= '9')) {
      p2 = p2 * 10 + (input[n] - '0');
    }
    n++;
    p2--;
  } else {
    p2 = NULLSLOT;
  }
    
  /*skip whitespace and commas*/
  while ((input[n] == ' ') || (input[n] == ',') || (input[n] == ']')) {
    n++;
  }

  /*get direction*/
  if ((input[n] == 'N') || (input[n] == 'n')) {
    n++;
    if ((input [n] == 'W') || (input[n] == 'w')) {
      dir = -2;
    }
    else if ((input[n] == 'E') ||(input [n] == 'e')) {
      dir = -3;
    }
  }
  else if (input[n] == 'S' || input[n] == 's') {
    n++;
    if (input [n] == 'W' || input[n] == 'w') {
      dir = 3;
    }
    else if (input[n] == 'E' || input [n] == 'e') {
      dir = 2;
    }
  }
  else if (input[n] == 'E' || input [n] == 'e') {
    dir = 1;
  }
  else if (input[n] == 'W' || input [n] == 'w') {
    dir = -1;
  }

  int pushee = destination(p1, dir);

  if (gBoard[pushee] == gBoard[p1])
    p2 = pushee;

  int move = move_hash (p1, p2, dir);
  if (DEBUGGING) printf("finished conversion: move is %d\n", move);
  return move;
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
  if (DEBUGGING) printf("starting print w/move = %d\n", theMove);
  int direction, slot1, slot2;
  STRING dir;
 
  direction = theMove % 10;
  theMove = theMove/10;


  if (theMove < 0) {
    theMove = 0 - theMove;
  }
 

  slot1 = theMove % 100;
  slot2 = theMove/100;


  if (slot1 != NULLSLOT)
    slot1++;
  if (slot2 != NULLSLOT)
    slot2++;

  if (direction == 1)
    dir = "E"; 
  else if (direction == -1)
    dir = "W"; 
  else if (direction == -3) 
    dir = "NE";
  else if (direction == -2) 
    dir = "NW"; 
  else if (direction == 3) 
    dir = "SW";
  else
    dir = "SE";
 

  if (slot1 == NULLSLOT) {
    printf("[%d,%s]",slot2, dir);
  }
  else if (slot2 == NULLSLOT) {  
    printf("[%d,%s]",slot1, dir);
  }
  else if ((slot1 - 1) == destination((slot2 - 1), direction)) { 
     printf("[%d,%s]",slot2, dir);
  }
  else {
    printf("[%d,%d,%s]",slot1,slot2, dir);
  }
  if (DEBUGGING) printf("finished printmove\n");
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
  int option = N * 100;
  option += MISERE;
  option += 10 * SS;
}

/************************************************************************
**
** NAME:        setOption
**
** DESCRIPTION: The correspondithang unhash for the game variants.
**				Should take the input and set all the appropriate
**				variants.
**
** INPUT:     int : the number representation of the options.
**
************************************************************************/
void setOption(int option)
{
  int m, ss;
  m = option % 10;
  option = option /10;
  ss = option % 10;
  option = option / 10;

  N = option;
  SS = ss;
  MISERE = m;
  
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
  /*find the row*/
  int r, start, size;
  for (r = 0; r <= 2*N - 2; r++) {
    if (slot < (*rows[r]).start_slot + (*rows[r]).size) {
      break;
    }
  }
  start = (*rows[r]).start_slot;
  size = (*rows[r]).size;

  /*test for nullslot*/

  if ((direction == 1 && slot == start + size - 1) ||
      (direction == 2 && ((r == 2*N -2) || ((slot == start + size -1) && (*rows[r+1]).size < size))) ||
      (direction == 3 && ((r == 2*N -2) || ((slot == start) && (*rows[r+1]).size < size))) ||
      (direction == -1 && slot == start) ||
      (direction == -2 && ((r == 0) || ((slot == start) && (*rows[r-1]).size < size))) ||
      (direction == -3 && ((r == 0) || ((slot == start + size -1) && (*rows[r-1]).size < size)))) {
    return NULLSLOT;
  }

  if (direction == 1 || direction == -1) {
    return (slot + direction);
  }

  if (direction == 2) {
    if (r < N - 1) {
      return ((slot - start + 1) + (*rows[r+1]).start_slot);
    }
    return ((slot - start) + (*rows[r+1]).start_slot);
  }

  if (direction == 3) {
    if (r < N - 1) {
      return ((slot - start) + (*rows[r+1]).start_slot);
    }
    return ((slot - start - 1) + (*rows[r+1]).start_slot);
  }

  if (direction == -2) {
    if (r > N - 1) {
      return ((slot - start) + (*rows[r-1]).start_slot);
    }
    return ((slot - start - 1) + (*rows[r-1]).start_slot);
  }

  if (direction == -3) {
    if (r > N - 1) {
      return ((slot - start + 1) + (*rows[r-1]).start_slot);
    }
    return ((slot - start) + (*rows[r-1]).start_slot);
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
  int small_shift = 10;
  int big_shift = 1000;

  if (slot2 > slot1) {
    bigger = slot2;
    smaller = slot1;
  }
  else {
    bigger = slot1;
    smaller = slot2;
  }

  if (direction > 0) {
    return (direction + (small_shift * bigger) + (big_shift * smaller));
  }
  else {
    return (-1 * ((-1 * direction) + (small_shift * smaller) + (big_shift * bigger)));
  }
}

struct row * makerow (int size, int start) {
  int i;

  struct row *new;
  new = (struct row *) SafeMalloc(1 * sizeof(struct row));
  (*new).size = size;
  (*new).start_slot = start;
  return new;
}
 
int m_generic_hash(char* board) {
  int size = 8;
  int val;
  int i, hashval= 0;
  for (i=0; i<size;i++) {
    if (board[i] == '*') 
      val = 0;
    if (board[i] == 'x')
      val = 1;
    if (board[i] == 'o')
      val = 2;
    hashval = hashval + val*pow(10,i);
  }
  return hashval;
}

void m_generic_unhash(int val, char* board) {
  int i;
  int size = 8;
  int temp;
  for (i = 0; i<size;i++) {
    temp = val %10;
    if (temp == 0) board[i] = '*';
    if (temp == 1) board[i] = 'x';
    if (temp == 2) board[i] = 'o';
    val = val/10;
  }
}

void printrow (int line, int flag) {
  int s, size, start;

  size = (*rows[line]).size;
  start = (*rows[line]).start_slot;

  for (s = 0; s < abs((N - 1) - line); s++) {
    printf ("  ");
  }
  if (flag == 0) {
    if (line > N - 1) 
      printf("\\ \\  ");
    else if (line == N - 1)
      printf("| |  ");
    else
      printf("/ /  ");
  }

  for (s = 0; s < size; s++) {
    printf ("(");
    
    if (flag == 0)
      if (gBoard[start + s] == '*')
	printf(" ");
      else
	printf ("%c", gBoard[start + s]);
    if (flag == 1) {
      if (N == 2) {
	printf("%d", start + s + 1);
      }
      else {
	if (start + s < 9)
	  printf("0");
	printf("%d",start + s + 1);
      }
    }
    
    if (s != size - 1)
      printf(")-");
    else
      printf(") ");
  }
  if (flag == 0) {
    if (line > N - 1) 
      printf(" / /");
    else if (line == N - 1)
      printf(" | |");
    else
      printf(" \\ \\");
  }

  
  for (s = 0; s < abs((N - 1) - line); s++) {
    if (flag == 0)
      printf ("  ");
    if (flag == 1)
      if (N == 2)
	printf("  ");
      else
	printf ("   ");
  }
}
    


void printlines (int line, int flag) {
  int s, max;
 
  for (s = 0; s < abs((N - 1) - line); s++) {
    printf ("  ");
  }
  if (flag == 0) {
    if (line < N - 1) 
      printf("/ /   ");
    else
      printf("  \\ \\ ");
  }
 
  if (line < N - 1) {
    s = 0;
  }
  else {
    s = 1;
    printf("  ");
  }
  for (; s < (*rows[line]).size; s++) {
    if (line < N - 1) {
      if (flag == 0)
	printf("/ \\ ");
      else
	if (N == 2)
	  printf("/ \\ ");
	else
	  printf("/  \\ ");
    }
    else {
      if (flag == 0)
	printf("\\ / ");
      else
	if (N == 2)
	  printf("\\ / ");
	else
	  printf("\\  / ");
    }
  }

  if (flag == 0) {
    if (line < N -1) 
      printf("  \\ \\");
    else
      printf("  / /");
  }

 
  if (line < N - 1)
    s = 0;
  else
    s = -1;

  if (flag == 0) {
    if (line < N - 1) {
      max = 2 * abs((N - 1) - line) - 1;
    }
    else {
      max = 2 * abs((N - 1) - line);
    }
  }
  else
    max = abs((N - 1) - line);
 
  for (; s < max; s++) {
    if (flag == 0)
      printf (" ");
    if (flag == 1)
      if (N == 2)
	printf ("  ");
    else
      printf ("   ");
  }
}

int b_size (int n) {
  if (n == 1)
    return 1;
  
  return ((n - 1) * 6) + b_size (n - 1);
}

int def_start_pieces (int n) {
  if (n == 2)
    return 2;
  else if (n == 3)
    return 6;
  else
    return 5 + ((n - 2)*(n - 2));
}

