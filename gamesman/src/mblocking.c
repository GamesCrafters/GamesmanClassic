// $ id $
// $ log $

/************************************************************************
**
** NAME:        mblocking.c
**
** DESCRIPTION: Generic Blocking
**
** AUTHOR:      Greg Bonin and Tanya Gordeeva
**
** DATE:        Start: ~ Jan 28, 2004
**
** UPDATE HIST: 
**              16 Mar, 04: Tentatively added structs, defines, globals,
**                          DoMove, PrintComputersMove, 
**                          PrintPosition (depends on parser),
**                          GetAndPrintPlayersMove, ConvertTextInputToMove,
**                          and several auxillary functions.
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

STRING   kGameName           = "";
STRING   kDBName             = "";
BOOLEAN  kPartizan           = ; 
BOOLEAN  kSupportsHeuristic  = ;
BOOLEAN  kSupportsSymmetries = ;
BOOLEAN  kSupportsGraphics   = ;
BOOLEAN  kDebugMenu          = ;
BOOLEAN  kGameSpecificMenu   = ;
BOOLEAN  kTieIsPossible      = ;
BOOLEAN  kLoopy               = ;
BOOLEAN  kDebugDetermineValue = ;

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

#define MAX_CLASSES 5
#define MAX_NODES 15
#define MAX_PIECES 30

#define BOTH_PLAYERS -1
#define BLACK_PLAYER 0
#define WHITE_PLAYER 1
#define NEITHER_PLAYER 2
#define INVALID -2

#define EMPTY_NODE -1
#define BLACK_NODE 0
#define WHITE_NODE 1

#define CLASS_TABLE 0
#define NODES_TABLE 1

struct GraphClass {
  char name[10];
  int player;
  struct GraphClass* moveto[MAX_CLASSES-1];
};

struct GraphNode {
  struct Piece* game_piece;
  /* Class of the node. */
  int class;
  /* Line in the image. */
  int str_line;
  /* Place in the line. */
  int str_index;
  char name;
  struct GraphNode* adjacent[MAX_NODES-1];
};

struct Piece {
  int color;
  char pic;
  int node;
};

typedef struct GraphClass classes[MAX_CLASSES];
typedef struct GraphNode nodes[MAX_NODES];
typedef struct Piece pieces[MAX_PIECES];

static int version;
static int num_nodes;
static int num_black;
static int num_white;
static char g_name[100];
static char image[20][500];

static classes global_classes;
static nodes global_board;
static pieces global_black;
static pieces global_white;

/* Hash tables for class and node names (for use by the parser). */
static int classes_lookup[MAX_CLASSES][MAX_CLASSES];
static int nodes_lookup[MAX_NODES][MAX_NODES];

/*************************************************************************
**
** Above is where you put your #define's and your global variables, structs
**
*************************************************************************/

/* Function prototypes here. */

int moveHash(int to, int from);
void moveUnhash(int move, int* to, int* from);

char* boardToString(char* s, nodes board, pieces black_pieces,
		    pieces white_pieces);
void stringToBoard(char*s, nodes board, pieces black_pieces,
		   pieces white_pieces);


/* Starting here are the function prototypes for the parser: */

/* Initialize all the entries to 0. */
void nullInit(nodes board, pieces black_pieces, pieces white_pieces, 
	      classes node_classes);

/* Processes the file. */
int procFile(FILE* graph_file, nodes board, pieces black_pieces,
	     pieces white_pieces, classes node_classes);

void handleGraph(char* token, int* pos);
void handleNodeClass(char* token, int* pos,
		     nodes board, classes node_classes);
void handlePieceClass(char* token, int* pos, nodes board,
		      pieces black_pieces, pieces white_pieces);
void handleNodeDef(char* token, int* pos, nodes board);
void handleImage(char* token, int* pos, nodes board);

/* Retrieves one token, of the form (...), a number, or 
   plain text (all other cases).
   The number of left facing and right facing parentheses must 
   be equal.
   Returns 0 if any errors occur, 1 otherwise. */
int fileGetToken(FILE* graph_file, char* token);

/* Similar to the above, but is more similar to strtok internally.
   However, it does not use static variables (two levels of parsing
   are needed for some expressions).
   The input string is modified, with all tokenizing characters
   (anything that isn't '(', ')', or an alphanumeric character)
   replaced with '\0' as the function steps through the string. */
char* stringGetToken(char* s, int* position);

/* Returns one line of a string - for use in constructing the image
   array. */
char* stringGetLine(char* s);

/* Returns a hash number for class or node names. */
int nameHash(char* name, int hashsize);

/* Adds a value to one of the hash tables. 
   0 for classes, 1 for nodes. */
void addToHash(int val, int num_val, int table);
int hashLookup(char* token, classes node_classes, nodes board);

void printBoard(nodes board, pieces black_pieces, 
		pieces white_pieces);

/* Parser prototypes end here. */

/* External */
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
** CALLS:       Hash ()
**              Unhash ()
**	            LIST OTHER CALLS HERE
*************************************************************************/

POSITION DoMove (thePosition, theMove)
	POSITION thePosition;
	MOVE theMove;
{
  int to, from;
  char string_board[MAX_NODES];

  generic_unhash(thePosition, string_board);
  moveUnhash(theMove, &to, &from);
  string_board[to] = string_board[from];
  string_board[from] = '-';

  return generic_hash(thePosition);
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
  int to, from;

  moveUnhash(computersMove, &to, &from);

  printf("%s's move\t: %d to %d\n", computersName, to, from);

  return;
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
  char string_board[MAX_NODES];

  generic_unhash(position, string_board);
  stringToBoard(string_board, global_board, global_black,
		global_white);

  printBoard(global_board, global_black, global_white);

  printf("%s should %s in %d moves.", playerName, 
	 GetPrediction(position), 0);

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

MOVELIST *GenerateMoves (position)
         POSITION position;
{
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
  char input[MAX_NODES*2 + 1];

  do {
    printf("%s's move [1-%d 1-%d/u(undo)]: ", playerName);
    
    ret = HandleDefaultTextInput(thePosition, theMove, playerName);
    if(ret != Continue)
      return (ret);
  }
  while(TRUE);
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

BOOLEAN ValidTextInput (input)
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

MOVE ConvertTextInputToMove (input)
	STRING input;
{
  char *to_string, *from_string;
  int to, from;

  to_string = strtok(input, " ");
  to = atoi(to_string);

  from_string = strtok(input, " ");
  from = atoi(from_string);

  return moveHash(to, from);
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

int GameSpecificTclInit (interp, mainWindow) 
	Tcl_Interp* interp;
	Tk_Window mainWindow;
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

int moveHash(int to, int from) {
  return (to << 16) & from;
}

void moveUnhash(int move, int* to, int* from) {
  *to = (move >> 16) & 0xFFFF;
  *from = move & 0xFFFF;
  return;
}

char* boardToString(char* s, nodes board, pieces black_pieces, 
		    pieces white_pieces) {
  int i;

  for(i = 0; i < num_nodes; i++) {
    s[i] = (board[i].game_piece ? board[i].game_piece->pic :
	    '_');
  }

  return s;
}

void stringToBoard(char* s, nodes board, pieces black_pieces,
		   pieces white_pieces) {
  int i, black_counter, white_counter;

  black_counter = white_counter = 0;

  for(i = 0; i < num_nodes; i++) {
    switch(s[i]) {
    case '_':
      board[i].game_piece = NULL;
      break;
    case black_pieces[0].pic:
      black_pieces[black_counter].node = i;
      board[i].game_piece = &black_pieces[black_counter];
      black_counter++;
      break;
    case white_pieces[0].pic:
      white_pieces[white_counter].node = i;
      board[i].game_piece = &white_pieces[white_counter];
      break;
    default:
      break;
    }
  }

  return;
}
