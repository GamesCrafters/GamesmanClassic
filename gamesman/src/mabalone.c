/************************************************************************
**
** NAME:        mabalone.c
**
** DESCRIPTION: Abalone
**
** AUTHOR:      Michael Mottmann & Melinda Franco
**
** DATE:        4/6/04 - Working and all nice and pretty and stuff
**              5/3/04 - Even better than before!
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

STRING   kAuthorName         = "";
STRING   kGameName           = "abalone";
STRING   kDBName             = "Abalone";
BOOLEAN  kPartizan           = TRUE; 
BOOLEAN  kSupportsHeuristic  = FALSE;
BOOLEAN  kSupportsSymmetries = FALSE;
BOOLEAN  kSupportsGraphics   = FALSE;
BOOLEAN  kDebugMenu          = FALSE;
BOOLEAN  kGameSpecificMenu   = TRUE;
BOOLEAN  kTieIsPossible      = FALSE;
BOOLEAN  kLoopy               = TRUE;
BOOLEAN  kDebugDetermineValue = FALSE;
void*	 gGameSpecificTclInit = NULL;

STRING kHelpGraphicInterface =
"Not written yet";

STRING   kHelpTextInterface    =
"ON YOUR TURN, use the legend to determine which pieces to move.\n\
In a move, multiple pieces can be moved, but any one piece\n\
can only shift one space away from where it started.\n\
There are two types of moves, pushes and side-steps.\n\n\
ONE PIECE can be pushed in any direction, as long as the connecting\n\
space is vacant.  TO PUSH TWO OR THREE PIECES, the direction of movement\n\
must be along the axis on which the pieces lie.  Multiple piece pushes\n\
can move into a vacant space, or into a space with an opponent's piece\n\
provided your pieces outnumber your opponent's (3 pieces in a row can\n\
push 1 or 2 pieces, 2 pieces can only push 1 piece).  Only through a push\n\
off the edge can you eliminate your opponent's pieces.\n\n\
IN A SIDE-STEP, two or three pieces that are in a line can all move\n\
in any direction in unison, as long as their desination spaces are empty.\n\n\
TO PERFORM A PUSH, enter the first piece that pushes all the rest, followed\n\
by the direction of the push.  TO PERFORM A SIDE-STEP, enter all the pieces to\n\
be moved, followed by the direction of the side-step.\n";

STRING   kHelpOnYourTurn =
"Choose which piece(s) of yours to move, and in what direction.";

STRING   kHelpStandardObjective =
"To push as many of your opponent's pieces off the board as it takes to win";

STRING   kHelpReverseObjective =
"To force your opponent to push as many of your pieces off the board as it takes to win.";

STRING   kHelpTieOccursWhen = /* Should follow 'A Tie occurs when... */
"There is no tie, only do";

STRING   kHelpExample =
"          BOARD                 LEGEND        DIRECTIONS\n\
\n\
       -----------        \n\
      / --------- \\       |\n\
     / /         \\ \\      |\n\
    / /  (x)-(x)  \\ \\     |     (1)-(2)        NW   NE\n\
   / /   / \\ / \\   \\ \\    |     / \\ / \\          \\ /\n\
  | |  ( )-( )-( )  | |   |   (3)-(4)-(5)      W -*- E\n\
   \\ \\   \\ / \\ /   / /    |     \\ / \\ /          / \\\n\
    \\ \\  (o)-(o)  / /     |     (6)-(7)        SW   SE\n\
     \\ \\         / /      |\n\
      \\ --------- /       |\n\
       -----------\n\
\n\
\n\
  Player's move :  6 7 ne\n\
\n\
          BOARD                 LEGEND        DIRECTIONS\n\
\n\
       -----------        \n\
      / --------- \\       |\n\
     / /         \\ \\      |\n\
    / /  (x)-(x)  \\ \\     |     (1)-(2)        NW   NE\n\
   / /   / \\ / \\   \\ \\    |     / \\ / \\          \\ /\n\
  | |  ( )-(o)-(o)  | |   |   (3)-(4)-(5)      W -*- E\n\
   \\ \\   \\ / \\ /   / /    |     \\ / \\ /          / \\\n\
    \\ \\  ( )-( )  / /     |     (6)-(7)        SW   SE\n\
     \\ \\         / /      |\n\
      \\ --------- /       |\n\
       -----------\n\
\n\
\n\
Computer's move   : [1 SW]\n\
\n\
          BOARD                 LEGEND        DIRECTIONS\n\
\n\
       -----------        \n\
      / --------- \\       |\n\
     / /         \\ \\      |\n\
    / /  ( )-(x)  \\ \\     |     (1)-(2)        NW   NE\n\
   / /   / \\ / \\   \\ \\    |     / \\ / \\          \\ /\n\
  | |  (x)-(o)-(o)  | |   |   (3)-(4)-(5)      W -*- E\n\
   \\ \\   \\ / \\ /   / /    |     \\ / \\ /          / \\\n\
    \\ \\  ( )-( )  / /     |     (6)-(7)        SW   SE\n\
     \\ \\         / /      |\n\
      \\ --------- /       |\n\
       -----------\n\
\n\
\n\
Player's move :  5 w\n\
\n\
          BOARD                 LEGEND        DIRECTIONS\n\
\n\
       -----------        \n\
      / --------- \\       |\n\
     / /         \\ \\      |\n\
    / /  ( )-(x)  \\ \\     |     (1)-(2)        NW   NE\n\
   / /   / \\ / \\   \\ \\    |     / \\ / \\          \\ /\n\
  | |  (o)-(o)-( )  | |   |   (3)-(4)-(5)      W -*- E\n\
   \\ \\   \\ / \\ /   / /    |     \\ / \\ /          / \\\n\
    \\ \\  ( )-( )  / /     |     (6)-(7)        SW   SE\n\
     \\ \\         / /      |\n\
      \\ --------- /       |\n\
       -----------\n\
\n\
Excellent! You won!";

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
#define MAXN 3 /*the maximum n that the solver will solve for*/
int N = 2;
int MISERE = 0;
int NSS = 0;
int XHITKILLS = 1;
int PIECES = 2;

int primcount = 1;

BOOLEAN DEBUGGING = FALSE;
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
int move_hash (int, int, int, int);
struct row * makerow (int, int);
int m_generic_hash(char *);
void m_generic_unhash(int, char *);
void printrow (int, int);
void printlines (int, int);
void changeBoard ();
int b_size (int);
int def_start_pieces (int);
void changePieces();
void changeKills();
int maxPieces(int);
int sum(int, int);
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

  int count, x_pieces_left = PIECES, o_pieces_left = PIECES, start, size, stop;
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
     
  
  /*printf("testing...\n");
  for (count = 0; count < BOARDSIZE; count++) {
    printf("%d: %c\n", count, gBoard[count]);
    }*/
  
  int init_array[10];
  init_array[0] = 'o';
  init_array[3] = 'x';
  init_array[6] = '*';
  init_array[9] = -1;

  init_array[1] = PIECES - XHITKILLS;
  init_array[2] = PIECES;
  init_array[4] = PIECES - XHITKILLS;
  init_array[5] = PIECES;
  init_array[7] = BOARDSIZE - 2 * init_array[2];
  init_array[8] = BOARDSIZE - 2 * init_array[1];

  /*
  printf("init array:\n");
  for (count = 0; count < 10; count++) {
    printf("%d: %d\n", count, init_array[count]);
    }*/

  max = generic_hash_init(BOARDSIZE,init_array,NULL);
  /*printf("%d  # of hash positions!\n",max);*/
  
  
  /*printf("The Board before unhashing is:\n");
  for (count = 0; count < BOARDSIZE; count++) {
    printf("%d: %c\n", count, gBoard[count]);
    }*/

  init = generic_hash(gBoard, 1);
  /*printf("%d  is the initial position\n",init);*/
  
  /*
  generic_unhash(init,gBoard);
  printf("The Board after unhashing is:\n");
    for (count = 0; count < BOARDSIZE; count++) {
    printf("%d: %c\n", count, gBoard[count]);
    }*/
  
  gNumberOfPositions  = max;
  gInitialPosition    = init;
  gMinimalPosition    = init;

  /*
  printf("mallocing %d positions\n", gNumberOfPositions);
  gDatabase = (VALUE *) SafeMalloc(gNumberOfPositions * sizeof(VALUE));*/
 
  /*int hash;
  for (hash = 1; hash <= 32; hash++) {
    setOption(hash);
    printf("hash = %d:  N = %d, PIECES = %d, KILLS = %d, NSS = %d, MISERE = %d\n", hash, N, PIECES, XHITKILLS, NSS, MISERE);
  }
  
  printf("the number of options is %d\n", NumberOfOptions());
  printf("current option is %d\n", getOption()); */


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
  fflush(stdin);
  char selection;
  printf("\n\tn)\t Change the value of (N), the edge size of the hexagon board -- currently %d\n", N);
  if (MISERE == 0)
    printf("\tm)\t Toggle from Standard to (M)isere\n");
  else
    printf("\tm)\t Toggle from (M)isere to Standard\n");
  if (NSS == 0)
    printf("\ts)\t Toggle from (S)ide Steps Allowed to No Side Steps\n");
  else
    printf("\ts)\t Toggle from No (S)ide Steps to Side Steps Allowed\n");
  printf("\tp)\t Change initial number of (p)ieces -- there are currently %d, max allowed is %d\n", PIECES, maxPieces(N));
  printf("\tc)\t Change the number of pieces that must be (c)aptured to win -- currently %d\n\n", XHITKILLS);
  printf("\tb)\t (B)ack to the previous menu\n\nSelect Option:  ");


  /*fflush(stdin);
    (void) scanf("%c", &selection);*/

  selection = (char) getchar();
  selection = (char) getchar();

  if (selection == 'n' || selection == 'N') {
    changeBoard();
    printf("\n");
    GameSpecificMenu();
  } else if (selection == 'm' || selection == 'M') {
    if (MISERE == 0)
      MISERE = 1;
    else
      MISERE = 0;
    
    SafeFree(rows);
    SafeFree(gBoard);
    InitializeGame();
    printf("\n");
    GameSpecificMenu();
  } else if (selection == 's' || selection == 'S') {
    if (NSS == 0)
      NSS = 1;
    else
      NSS = 0;
    
    SafeFree(rows);
    SafeFree(gBoard);
    InitializeGame();
    printf("\n");
    GameSpecificMenu();
  } 
  else if (selection == 'p' || selection == 'P') {
    changePieces();
    printf("\n");
    GameSpecificMenu();
  } 
  else if (selection == 'c' || selection == 'C') {
    changeKills();
    printf("\n");
    GameSpecificMenu();
  }
  else if ((selection == 'b') || (selection == 'B'))
    return;
  else {
    printf("\n\n\n Please select a valid option...\n\n");
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
    MISERE = 0;
    NSS = 0;
    XHITKILLS = 1;
    PIECES = def_start_pieces(N);
    SafeFree(rows);
    SafeFree(gBoard);
    InitializeGame();
  }
}

void changeKills()
{
  int kills;
  printf("Enter the new number of pieces to capture:   ");
  (void) scanf("%u", &kills);
  /*kills = (int) getchar();
    kills = (int) getchar();*/

  if (PIECES - kills < 0) {
    printf("A player can only lose as many pieces as the game starts with\n");
    changeKills();
  }
  else if (kills <= 0) {
    printf("There must be at least one piece captured\n");
    changeKills();
  }
  else{
    XHITKILLS = kills;
    SafeFree(rows);
    SafeFree(gBoard);
    InitializeGame();
  }
}

void changePieces()
{
  int num;
  printf("Enter the new number of pieces:  ");
  fflush(stdin);
  (void) scanf("%u", &num);
  /*num = getchar();
    num = getchar();*/

  if ((2 * num + (*rows[N-1]).size) > BOARDSIZE) {
    printf("Too many pieces for board\n");
    changePieces();
  }
  else if (num < 2) {
    printf("There must be at least two pieces\n");
    changePieces();
  }
  else {
    PIECES = num;
    XHITKILLS = 1;
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
  if (DEBUGGING) 
    printf("Starting Do Move with input: %d\n", theMove);
  int destination(int,int);
  int whoseTurn;
  generic_unhash(thePosition, gBoard);
  int direction;
  int slot1, slot2, slot3, dest1, dest2, dest3, pushee1, pushee2, pushdest1, pushdest2, doubpushdest;
  BOOLEAN twopieces = FALSE, threepieces = FALSE;
  int dir_shift = 10;
  int piece_shift = 100;

  direction = theMove % dir_shift;
  theMove = theMove/dir_shift;

  if (theMove < 0) {
    theMove = 0 - theMove;
  }

  slot1 = theMove % piece_shift;
  theMove = theMove/piece_shift;
  slot2 = theMove % piece_shift;
  slot3 = theMove/piece_shift;
  
  /*one piece scenarios*/
  if ((slot1 == NULLSLOT) && (slot2 == NULLSLOT)) {
    slot1 = slot3;
  }
  else if ((slot2 == NULLSLOT) && (slot3 == NULLSLOT)) {
    /*do nothing*/
  }
  else if (slot1 == NULLSLOT) {
    twopieces = TRUE;
    slot1 = slot2;
    slot2 = slot3;
  }
  else if (slot3 == NULLSLOT) {
    twopieces = TRUE;
  }
  else {
    threepieces = TRUE;
  }
  
  dest1 = destination(slot1, direction);
  dest2 = destination(slot2, direction);
  dest3 = destination(slot3, direction);
  pushee1 = dest3;
  doubpushdest = destination(dest2, direction);
  pushee2 = destination(pushee1, direction);
  pushdest1 = pushee2;
  pushdest2 = destination(pushee2, direction);
  
  /* one piece move */
  if ((twopieces == FALSE) && (threepieces == FALSE)) {
    gBoard[dest1] = gBoard[slot1];
    gBoard[slot1] = '*';
  }

  /* double piece move */
  if (twopieces) {
    /*printf("slot1 = %d, slot2 = %d, in direction %d, dest1 is %d, dest2 is %d, doubpushdest is %d\n", slot1, slot2, direction, dest1, dest2, doubpushdest);*/
    if (gBoard[dest2] != '*') {
      /*push a piece*/
      if (doubpushdest != NULLSLOT) {
	gBoard[doubpushdest] = gBoard[dest2];
      }
    }

    /*printf("dest1 is %d:%c, dest2 is %d:%c\n", dest1, gBoard[dest1], dest2, gBoard[dest2]);*/
    
    gBoard[dest2] = gBoard[slot2];
    gBoard[slot2] = '*';
    gBoard[dest1] = gBoard[slot1];
    gBoard[slot1] = '*';
  
    /*printf("dest1 is %d:%c, dest2 is %d:%c\n", dest1, gBoard[dest1], dest2, gBoard[dest2]);*/
  }

  /* triple piece move */
  if (threepieces) {
    /*three push two*/
    if ((pushee2 != NULLSLOT) && (gBoard[pushee1] != '*') && (gBoard[pushee2] != '*')) {
      if (pushdest2 != NULLSLOT) {
	gBoard[pushdest2] = gBoard[pushee2];
      }
      gBoard[pushdest1] = gBoard[pushee1];
    }
    else if (gBoard[pushee1] != '*') {
      /*three push one*/
      if (pushdest1 != NULLSLOT) {
	gBoard[pushdest1] = gBoard[pushee1];
      }
    }

    /*main move*/
    gBoard[dest3] = gBoard[slot3];
    gBoard[slot3] = '*';
    gBoard[dest2] = gBoard[slot2];
    gBoard[slot2] = '*';
    gBoard[dest1] = gBoard[slot1];
    gBoard[slot1] = '*';
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
  int count, player, xs = 0, os = 0, option = -1;
  char selection;
  
  while ((option < 0) || (option > maxPieces(N))){
    printf("Enter the maximum pieces per side: ");
    fflush(stdin);
    (void) scanf("%d", &option);
    if ((option < 0) || (option > maxPieces(N)))
      printf("Please enter a valid number\n\n");
  }
  PIECES = option;
  option = -1;

  while ((option < 1) || (option > PIECES)) {
    printf("Enter the number of pieces to capture for victory: ");
    fflush(stdin);
    (void) scanf("%d", &option);
    if ((option < 1) || (option > PIECES))
      printf("The number of eliminated pieces must be at least 1\nand no more than the total number of pieces\n\n");
  }
  XHITKILLS = option;

  printf("Enter the board as you would like it (enter a 'b' for blanks)\n\n");

  for (count = 0; count < BOARDSIZE; count++) {
    printf("Enter the piece at position %d: ", count + 1);
    fflush(stdin);
    (void) scanf("%c", &selection);
    if ((selection != 'x') && (selection != 'o') && (selection != 'b') && (selection != 'X') && (selection != 'O') && (selection != 'B')) {
      printf("\n\nPlease enter a valid piece\n\n");
      count--;
    }
    else if (selection == 'x') {
      if (xs < PIECES) {
	gBoard[count] = selection;
	xs++;
      }
      else {
	printf("\n\nThis board already has the maximum number of x pieces allowed\n\n");
	count--;
      }
    }
    else if (selection == 'o') {
      if (os < PIECES) {
	gBoard[count] = selection;
	os++;
      }
      else {
	printf("\n\nThis board already has the maximum number of o pieces allowed\n\n");
	count--;
      }
    }
    else {
      gBoard[count] = '*';
    }
  }  

  player = 0;
  while (player == 0) {
    printf("\n\n Whose Turn is it?  ");
    fflush(stdin);
    (void) scanf("%c", &selection);
    if ((selection == 'x') || (selection == 'X'))
      player = 2;
    else if ((selection == 'o') || (selection == 'O'))
      player = 1;
    else
      printf ("\n\n Please enter x or o\n\n");
  }

  return generic_hash(gBoard,player);  
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

  /*  printf("analyzing position %d\n", primcount);*/
  primcount++;

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
  
  lose = PIECES - XHITKILLS;

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
  char piece;
  if (whoseMove(position) == 2)
    piece = 'x';
  else
    piece = 'y';


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
	printf("     NW   NE");
      else if (r == N - 1)
	printf("     W -*- E");
      else if (r == N)
	printf("     SW   SE");
      else
	printf("               ");
      if (r != 2 * N - 2) {
	printf("\n ");
	printlines(r, 0);
	printf("   |   ");
	printlines(r, 1);
	printf("  ");
	
	if (r == N - 1)
	  printf("     / \\");
	else if (r == N - 2)
	  printf("     \\ /");
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
  
  for (r = 0; r < N - 2; r++) {
    printf("  ");
  }
  printf("       ");
  printf("%s\n\n",GetPrediction(position,playerName,usersTurn));
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
  if (DEBUGGING)
    printf("generate\n");
  MOVELIST *head = NULL;
  MOVELIST *CreateMovelistNode(); /* In gamesman.c */
  VALUE Primitive();
  int slot, direction, ssdir;
  int pusher2, pusher3, pushee1, pushee2, pushee3;
  char whoseTurn, opponent;
  
  if (whoseMove(position) == 2) {
    whoseTurn = 'x';
    opponent = 'o';
  }
  else {
    whoseTurn = 'o';
    opponent = 'x';
  }
  generic_unhash(position,gBoard);

  /*printf("the hash is %d\n", position);
    for (slot = 0; slot < BOARDSIZE; slot++)
    printf("%d :%c\n", slot, gBoard[slot]);*/

  if (Primitive(position) == undecided) {
    for (slot = 0; slot < BOARDSIZE ; slot++) {
      if (gBoard[slot] == whoseTurn) {
	for (direction = 1; direction <= 3; direction++) {

	  /*Single Piece Moves in all directions*/
	  pushee1 = destination(slot, direction);
	  pushee2 = destination(slot, (0 - direction));

	  if (gBoard[pushee1] == '*') {
	    head = CreateMovelistNode(move_hash(slot,NULLSLOT, NULLSLOT, direction), head);
	  }
	  if (gBoard[pushee2] == '*') {
	    head = CreateMovelistNode(move_hash(slot,NULLSLOT, NULLSLOT, (0 - direction)), head);
	  }
	  
	  /*Multiple Piece Moves in positive directions*/
	  pusher2 = destination(slot, direction);
	  if ((pusher2 != NULLSLOT) && (gBoard[pusher2] == whoseTurn)) {
	    
	    /*Double Piece Moves in positive directions*/
	    pushee1 = destination(pusher2,direction);
	    pushee2 = destination(pushee1,direction);
	    if ((pushee1 != NULLSLOT) &&
		((gBoard[pushee1] == '*') ||
		 ((gBoard[pushee1] == opponent) && 
		  ((pushee2 == NULLSLOT) || (gBoard[pushee2] == '*'))))) {
	      head = CreateMovelistNode(move_hash(slot, pusher2, NULLSLOT, direction), head);
	    }

	    /*Triple Piece Push in positive direction*/
	    pusher3 = destination(pusher2, direction);
	    pushee1 = destination(pusher3, direction);
	    pushee2 = destination(pushee1, direction);
	    pushee3 = destination(pushee2, direction);

	    if (((pusher3 != NULLSLOT) && (gBoard[pusher3] == whoseTurn)) &&
		(/*no pieces pushed*/
		 ((pushee1 != NULLSLOT) && (gBoard[pushee1] == '*')) ||
		 /*pieces pushed*/
		 (((pushee1 != NULLSLOT) && (gBoard[pushee1] == opponent)) &&
		  (/*one piece pushed*/
		   ((pushee2 == NULLSLOT) || (gBoard[pushee2] == '*')) ||
		   /*two pieces pushed*/
		   (((pushee2 != NULLSLOT) && (gBoard[pushee2] == opponent)) && ((pushee3 == NULLSLOT) || (gBoard[pushee3] == '*'))))))) {
	      head = CreateMovelistNode(move_hash(slot, pusher2, pusher3, direction), head);
	    }
	  }

	  direction = 0 - direction; 

	  /*Multiple Piece Moves in negative directions*/
	  pusher2 = destination(slot, direction);
	  if ((pusher2 != NULLSLOT) && (gBoard[pusher2] == whoseTurn)) {
	    
	    /*Double Piece Moves in negative direction*/
	    pushee1 = destination(pusher2,direction);
	    pushee2 = destination(pushee1,direction);
	    if ((pushee1 != NULLSLOT) &&
		((gBoard[pushee1] == '*') ||
		 ((gBoard[pushee1] == opponent) && ((pushee2 == NULLSLOT) || (gBoard[pushee2] == '*')))))
	      head = CreateMovelistNode(move_hash(slot, pusher2, NULLSLOT, direction), head);
	    

	    /*Triple Piece Push in negative direction*/
	    pusher3 = destination(pusher2, direction);
	    pushee1 = destination(pusher3, direction);
	    pushee2 = destination(pushee1, direction);
	    pushee3 = destination(pushee2, direction);

	    if (((pusher3 != NULLSLOT) && (gBoard[pusher3] == whoseTurn)) &&
		(/*no pieces pushed*/
		 ((pushee1 != NULLSLOT) && (gBoard[pushee1] == '*')) ||
		 /*pieces pushed*/
		 (((pushee1 != NULLSLOT) && (gBoard[pushee1] == opponent)) &&
		  (/*one piece pushed*/
		   ((pushee2 == NULLSLOT) || (gBoard[pushee2] == '*')) ||
		   /*two pieces pushed*/
		   (((pushee2 != NULLSLOT) && (gBoard[pushee2] == opponent)) && ((pushee3 == NULLSLOT) || (gBoard[pushee3] == '*'))))))) {
	      head = CreateMovelistNode(move_hash(slot, pusher2, pusher3, direction), head);
	    }
	  }

	  direction = 0 - direction;
	      
	  if (NSS == 0) { 
	    /*Test for possible side steps*/
	    for (ssdir = -3; ssdir <= 3; ssdir++) {
	      if ((ssdir != 0) && (ssdir != direction) && (ssdir != 0 - direction)) {
		/*skip over nonexistant zero direction as well as push direction*/
		pusher2 = destination(slot,direction);
		pusher3 = destination(pusher2, direction);
		pushee1 = destination(slot,ssdir);
		pushee2 = destination(pusher2, ssdir);
		pushee3 = destination(pusher3, ssdir);
		
		
		if ((pusher2 != NULLSLOT) && (pushee1 != NULLSLOT) && (pushee2 != NULLSLOT) && 
		    (gBoard[pusher2] == whoseTurn) && (gBoard[pushee1] == '*') && (gBoard[pushee2] == '*')) {
		  
		  /*two piece sidestep*/
		  head = CreateMovelistNode(move_hash(slot,pusher2, NULLSLOT, ssdir), head);
		  
		  /*three piece sidestep*/
		  if ((pusher3 != NULLSLOT) && (pushee3 != NULLSLOT) &&
		      (gBoard[pusher3] == whoseTurn) && (gBoard[pushee3] == '*')) {
		    head = CreateMovelistNode(move_hash(slot,pusher2,pusher3,ssdir), head);
		  }
		}
	      }
	    }
	    
	    
	  }
	} 
      }
    }
    if (DEBUGGING)
      printf("end gen\n");
    return(head);
  }
  if (DEBUGGING)
    printf("end gen - NULL\n");
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
  char whoseTurn;
  if (whoseMove(thePosition) == 2) {
    whoseTurn = 'x';
  }
  else {
    whoseTurn = 'o';
  }


  do {
    printf("for a list of valid moves, press ?\n\n");
    printf("%8s's move (%c):  ", playerName, whoseTurn);
  
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
  int n=0, p1, p2, p3;

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

  /*skip whitespace and commas*/
  while ((input[n] == ' ') || (input[n] == ',') || (input[n] == ']')) {
    n++;
  }

  /*get third piece, if it exists*/
  if ((input[n] >= '0') && (input[n] <= '9')) {
    p3 = input[n] - '0';
    n++;
    if ((input[n] >= '0') && (input[n] <= '9')) {
      p3 = p3 * 10 + (input[n] - '0');
    }
    n++;
    p3--;
    if (p3 >= BOARDSIZE || p3 < 0) {
      return FALSE;
    }
  }

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
  else {
    return FALSE;
  }
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
  if (DEBUGGING)
  printf("Starting conversion\n");
  int n=0, dir, p1, p2, p3, pushee;

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
    p3 = NULLSLOT;
  }
    

  /*skip whitespace and commas*/
  while ((input[n] == ' ') || (input[n] == ',') || (input[n] == ']')) {
    n++;
  }

  /*get third piece, if it exists*/
  if ((input[n] >= '0') && (input[n] <= '9')) {
    p3 = input[n] - '0';
    n++;
    if ((input[n] >= '0') && (input[n] <= '9')) {
      p3 = p3 * 10 + (input[n] - '0');
    }
    n++;
    p3--;
  } else {
    p3 = NULLSLOT;
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
  else if ((input[n] == 'S') || (input[n] == 's')) {
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

  /*fill in implicit push moves*/
  if ((p2 == NULLSLOT) && (p3 == NULLSLOT)) {
    pushee = destination (p1, dir);
    if ((pushee != NULLSLOT) && (gBoard[pushee] == gBoard[p1])) {
      p2 = pushee;
      
      pushee = destination (p2, dir);
      if ((pushee != NULLSLOT) && (gBoard[pushee] == gBoard[p2]))
	p3 = pushee;
    }
  }   

  /*printf("p1 = %d, p2 = %d, p3 = %d, dir = %d\n", p1, p2, p3, dir);*/
  int move = move_hash (p1, p2, p3, dir);
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
  if (DEBUGGING) 
    printf("starting print w/move = %d\n", theMove);
  int direction, slot1, slot2, slot3;
  int dir_shift = 10;
  int piece_shift = 100;
  STRING dir;
 
  direction = theMove % dir_shift;
  theMove = theMove/dir_shift;


  if (theMove < 0) {
    theMove = 0 - theMove;
  }

  slot1 = theMove % piece_shift;
  theMove = theMove/piece_shift;
  slot2 = theMove % piece_shift;
  slot3 = theMove/piece_shift;

  /*match up internal numbers (starting at 0) w/external (starting at 1) */
  if (slot1 != NULLSLOT)
    slot1++;
  if (slot2 != NULLSLOT)
    slot2++;
  if (slot3 != NULLSLOT)
    slot3++;

  if (direction == 1) {
    dir = "E";
  }
  else if (direction == -1) {
    dir = "W"; 
  }
  else if (direction == -3){
    dir = "NE";
  }
  else if (direction == -2) {
    dir = "NW"; 
  }
  else if (direction == 3) {
    dir = "SW";
  }
  else {
    dir = "SE";
  }

  /*printf("slot1 = %d, slot2 = %d, slot3 = %d, direction = %d\n", slot1, slot2, slot3, direction);*/

  if ((slot1 == NULLSLOT) && (slot2 == NULLSLOT)) {
    printf("[%d %s]", slot3, dir);
  }
  else if ((slot3 == NULLSLOT) && (slot2 == NULLSLOT)) {
    printf("[%d %s]", slot1, dir);
  }
  else if (slot1 == NULLSLOT) {
    if ((slot3 - 1 ) == destination ((slot2 - 1), direction))
      printf("[%d %s]", slot2, dir);
    else
      printf("[%d %d %s]",slot2, slot3, dir);
  }
  else if (slot3 == NULLSLOT) {
    if ((slot2 - 1) == destination ((slot1 - 1), direction))
      printf("[%d %s]", slot1, dir);
    else
      printf("[%d %d %s]",slot1, slot2, dir);
  }
  else if ((slot2 - 1) == destination((slot1 - 1), direction)) { 
     printf("[%d %s]",slot1, dir);
  }
  else {
    printf("[%d %d %d %s]",slot1,slot2,slot3, dir);
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
  int options = 0, n, p;
  for (n = 2; n <= MAXN; n++) {
    for (p = 2; p <= maxPieces(n); p++) {
      options += 4 * p;
    }
  }
  return options;
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
  int option = 1, n, p;
  for (n = 2; n < N; n++) {
    for (p = 2; p <= maxPieces(n); p++) {
      option += 4 * p;
    }
  }
  for (p = 2; p < PIECES; p++) {
    option += 4 * p;
  }

  if (NSS == 1)
    option += 2;
  if (MISERE == 1)
    option += 1;

  return option;
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
  printf("Set Option has been called!\n");
  option--;
  int n = 2, pieces = 2, kills = 0, hash, mod;
  
  for (hash = 0; hash <= option; hash += 4) {
    if ((pieces == maxPieces(n)) && (pieces == kills)) {
      pieces = 2;
      kills = 1;
      n++;
    }
    else if (kills == pieces) {
      pieces++;
      kills = 1;
    }
    else
      kills++;
  }


  N = n;
  PIECES = pieces;
  XHITKILLS = kills;

  mod = fmod(hash - option, 4);

  switch (mod) {
  case 0:
    NSS = 0;
    MISERE = 0;
    break;
  case 3:
    NSS = 0;
    MISERE = 1;
    break;
  case 2:
    NSS = 1;
    MISERE = 0;
    break;
  case 1:
    NSS = 1;
    MISERE = 1;
  }
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
  if (DEBUGGING)
    printf("starting destination\n");
  /*garbage in, garbage out*/
  if (slot == NULLSLOT)
    return NULLSLOT;

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
  
int move_hash(int slot1, int slot2, int slot3, int direction) {
  int bigger, smaller, middle;
  int small_shift = 10;
  int mid_shift = 1000;
  int big_shift = 100000;

  if (slot2 > slot1) {/* 2 > 1*/
    if (slot3 > slot1) { /* 3 > 1*/
      smaller = slot1;
      if (slot3 > slot2) { /* 3 > 2*/
	bigger = slot3;
	middle = slot2;
      }
      else {                /* 2 > 3*/
	bigger = slot2;
	middle = slot3;
      }
    }
    else {               /* 1 > 3*/
      smaller = slot3;
      middle = slot1;
      bigger = slot2;
    }
  }
  else {              /* 1 > 2*/
    if (slot3 > slot2) { /* 3 > 2*/
      smaller = slot2;
      if (slot1 > slot3) {  /* 1 > 3*/
	bigger = slot1;
	middle = slot3;
      }
      else {                /* 3 > 1*/
	bigger = slot3;
	middle = slot1;
      }
    }
    else {               /* 2 > 3*/
      smaller = slot3;
      middle = slot2;
      bigger = slot1;
    }
  }

  /* so do move can simply shift the pieces over in the order it gets them*/
  if (direction > 0) {
    return (direction + (small_shift * smaller) + (mid_shift * middle) + (big_shift * bigger));
  }
  else {
    return (-1 * ((-1 * direction) + (small_shift * bigger) + (mid_shift * middle) +  (big_shift * smaller)));
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

int maxPieces (int n) {
  if (n == 2)
    return 2;
  return n + 2 * (n - 2) + maxPieces(n - 1);
}

int def_start_pieces (int n) {
  if (n == 2)
    return 2;
  else if (n == 3)
    return 6;
  else
    return 5 + ((n - 2)*(n - 2));
}

int sum(int start, int stop) {
  if (stop == start)
    return start;
  return stop + sum(start, stop - 1);
}

int getInitialPosition() {
  return ((int)gInitialPosition);
}
