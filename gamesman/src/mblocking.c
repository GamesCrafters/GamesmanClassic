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
**              19 Mar, 04: Added parsing and printing code.
**              06 Apr, 04: Added the rest of the functions, fixed
**                          compiler errors.
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

POSITION gNumberOfPositions  = 0;

POSITION gInitialPosition    = 0;
POSITION gMinimalPosition    = 0;
POSITION kBadPosition        = -1;

STRING   kGameName           = "Blocking";
STRING   kDBName             = NULL;
BOOLEAN  kPartizan           = TRUE; 
BOOLEAN  kSupportsHeuristic  = FALSE;
BOOLEAN  kSupportsSymmetries = FALSE;
BOOLEAN  kSupportsGraphics   = FALSE;
BOOLEAN  kDebugMenu          = FALSE;
BOOLEAN  kGameSpecificMenu   = TRUE;
BOOLEAN  kTieIsPossible      = FALSE;
BOOLEAN  kLoopy               = TRUE;
BOOLEAN  kDebugDetermineValue = FALSE;

STRING kHelpGraphicInterface =
"Not written yet";

STRING   kHelpTextInterface    =
"Use the table of letter to number values to determine which numbers\
to choose.\n\
Enter the numbers of the node you wish to move your piece from and to\
, respectively.\n";

STRING   kHelpOnYourTurn =
"Move one of your pieces to an empty spot on the board\n";

STRING   kHelpStandardObjective =
"Move your pieces such that your opponent is trapped (ie, cannot move).\n";

STRING   kHelpReverseObjective =
"Move your pieces such that your opponent is forced to trap you.\n";

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
#define BLACK_PLAYER 1
#define WHITE_PLAYER 2
#define NEITHER_PLAYER 3
#define INVALID -2

#define CLASS_TABLE 0
#define NODES_TABLE 1

/* Each node has a class, and pieces in that node may
   only be restricted by type and may only move to
   nodes which both allow their type and are in the
   moveto list of the current node's class. */
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

/* Generic piece, but the current game limits pieces to black
   and white. Having more pieces complicates the 
   stringToBoard procedure and would require expansion
   of the parser grammar to provide additional rules for
   these pieces. */
struct Piece {
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
static FILE* g_file;
static char default_file[100] = "../grf/default.blk";
static BOOLEAN use_default = TRUE;
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
void moveUnHash(int move, int* to, int* from);

char* boardToString(char* s, nodes board, pieces black_pieces,
		    pieces white_pieces);
void stringToBoard(char*s, nodes board, pieces black_pieces,
		   pieces white_pieces);

void printBoard(nodes board, pieces black_pieces, 
		pieces white_pieces);
/* Prints just the structure, listing which node names
   correspond to which node numbers. */
void printEmptyBoard(nodes board);


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
  int generic_hash_init();

  int hash_array[10];

  nullInit(global_board, global_black, global_white, global_classes);

  if(use_default) {
    g_file = fopen(default_file, "r");
  }

  procFile(g_file, global_board, global_black, global_white, 
	   global_classes);
  hash_array[0] = global_black[0].pic;
  hash_array[1] = hash_array[2] = num_black;
  hash_array[3] = global_white[0].pic;
  hash_array[4] = hash_array[5] = num_white;
  hash_array[6] = '_';
  hash_array[7] = hash_array[8] = num_nodes - num_black - num_white;
  hash_array[9] = -1;

  strcpy(kDBName, g_name);

  gNumberOfPositions = generic_hash_init(num_nodes, hash_array, NULL);

  return;
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
  char file_name[100];
  FILE* fp;
  
  use_default = TRUE;

  while(use_default) {
    printf("\nSpecify one of the .blk files in the ../grf directory");
    printf("\n(But don't add the .blk at the end)\n\n");
    system("ls ../grf");
    printf("\nLoad Graph from : ");
    scanf("%s", file_name);
    (void) sprintf((char *)file_name, "../grf/%s.blk", file_name);


    if(!(fp = fopen(file_name, "r")))
      printf("Invalid file name\n");
    else
      use_default = FALSE;
  }

  InitializeGame();

  return;
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
**	        whoseMove ()
**
*************************************************************************/

POSITION DoMove (thePosition, theMove)
	POSITION thePosition;
	MOVE theMove;
{
  int generic_hash(), generic_unhash(), whoseMove();

  int to, from;
  int player;
  char string_board[MAX_NODES];

  generic_unhash(thePosition, string_board);
  moveUnHash(theMove, &to, &from);
  string_board[to] = string_board[from];
  string_board[from] = '-';

  if(whoseMove(thePosition) == BLACK_PLAYER)
    player = WHITE_PLAYER;
  else
    player = BLACK_PLAYER;

  return generic_hash(string_board, player);
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
  POSITION initialPosition;
  int current_node;
  int i;
  char tmp[100];
  char string_board[MAX_NODES];

  printEmptyBoard(global_board);
  printf("Enter the node numbers of black pieces, with spaces in between\n");
  scanf("%s", &tmp);
  
  i = 0;
  current_node = atoi(strtok(tmp, " "));
  while(current_node) {
    global_black[i].node = current_node;
    global_board[current_node].game_piece = &global_black[i];
    i++;
    current_node = atoi(strtok(NULL, " "));
  }

  printf("Enter the node numbers of white pieces, with spaces in between\n");
  scanf("%s", &tmp);
  
  i = 0;
  current_node = atoi(strtok(tmp, " "));
  while(current_node) {
    global_white[i].node = current_node;
    global_board[current_node].game_piece = &global_white[i];
    i++;
    current_node = atoi(strtok(NULL, " "));
  }

  boardToString(string_board, global_board, global_black, global_white);

  initialPosition = generic_hash(string_board, 1);

  return initialPosition;
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

  moveUnHash(computersMove, &to, &from);

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
** CALLS:       whoseMove ()
**              generic_unhash ()
**              stringToBoard ()
**              
**
************************************************************************/

VALUE Primitive (pos)
	POSITION pos;
{
  int whoseMove();

  char string_board[MAX_NODES];
  int player = whoseMove(pos);
  pieces* pieces_ptr;
  struct GraphNode* current_node;
  int num, i, j;

  generic_unhash(pos, string_board);
  stringToBoard(string_board, global_board, global_black, global_white);

  if(player == BLACK_PLAYER) { 
    pieces_ptr = &global_black;
    num = num_black;
  } else {
    pieces_ptr = &global_white;
    num = num_white;
  }

  for(i = 0; i < num; i++) {
    current_node = global_board[(*pieces_ptr)[i].node].adjacent[0];
    while(current_node) {
      if(!(current_node->game_piece))
	return undecided;
      j++;
      current_node = global_board[(*pieces_ptr)[i].node].adjacent[j];
    }
  }

  return (gStandardGame ? lose : win);
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
**              stringToBoard()
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

  printf("%s", GetPrediction(position, playerName, usersTurn));

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
  MOVELIST *CreateMovelistNode(), *head = NULL;
  int whoseMove();
  VALUE Primitive();

  char string_board[MAX_NODES];
  int player, i, j, max_pieces;
  pieces* user_pieces;

  if(Primitive(position) == undecided) {
    player = whoseMove(position);
    if(player == BLACK_PLAYER) {
      max_pieces = num_black;
      user_pieces = &global_black;
    } else {
      max_pieces = num_white;
      user_pieces = &global_white;
    }

    generic_unhash(position, string_board);
    stringToBoard(string_board, global_board, global_black, global_white);
    for(i = 0; i < max_pieces; i++) {
      for(j = 0; j < (num_nodes-1); j++)
	if(global_board[(*user_pieces)[i].node].adjacent[j] == NULL)
	  head = CreateMovelistNode(moveHash(j, i), head);
    }

    return (head);
  } else {
    return (NULL);
  }
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

  /* return (Continue); */
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
  char *node_string;
  int node;

  node_string = strtok(input, " ");
  node = atoi(node_string);
  if((node < 0) || (node > num_nodes))
    return FALSE;

  node_string = strtok(NULL, " ");
  node = atoi(node_string);
  if((node < 0) || (node > num_nodes))
    return FALSE;

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

MOVE ConvertTextInputToMove (input)
	STRING input;
{
  char *to_string, *from_string;
  int to, from;

  to_string = strtok(input, " ");
  to = atoi(to_string);

  from_string = strtok(NULL, " ");
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
  void moveUnHash();

  int to, from;
  moveUnHash(move, &to, &from);

  printf("[%d %d]", from, to);

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
  return 2;
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
  if(gStandardGame) return 1;
  return 2;
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
  if(option == 1)
    gStandardGame = TRUE;
  else
    gStandardGame = FALSE;
}


/************************************************************************
**
** NAME:        GameSpecificTclInit
**
** DESCRIPTION: Types changed to fix compilation...
**
************************************************************************/

int GameSpecificTclInit (interp, mainWindow) 
	void* interp;
	void* mainWindow;
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

int moveHash(int to, int from) {
  return (to << 16) & from;
}

void moveUnHash(int move, int* to, int* from) {
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
    if(s[i] == '_')
      board[i].game_piece = NULL;
    else if(s[i] == black_pieces[0].pic) {
      black_pieces[black_counter].node = i;
      board[i].game_piece = &black_pieces[black_counter];
      black_counter++;
    } else if(s[i] == white_pieces[0].pic) {
      white_pieces[white_counter].node = i;
      board[i].game_piece = &white_pieces[white_counter];
      white_counter++;
    }
  }

  return;
}

void printBoard(nodes board, pieces black_pieces, 
		pieces white_pieces) {
  int i;

  printf("\n");

  for(i = 0; i < num_nodes; i++) {
    if(board[i].game_piece)
      image[board[i].str_line][board[i].str_index] =
	board[i].game_piece->pic;
    else
      image[board[i].str_line][board[i].str_index] = 
	i;
  }

  i = 0;
  while((image[i][0] != EOF) && (i < 20))
    printf("%s\n", image[i++]);

  printf("\n");

  return;
}

void printEmptyBoard(nodes board) {
  int i;

  printf("\n");

  printf("Node names and corresponding numbers:\t");

  for(i = 0; i < num_nodes; i++) {
    printf("%c - %d ", board[i].name, i);
    image[board[i].str_line][board[i].str_index] =
      board[i].name;
  }

  printf("\n");

  i = 0;
  while((image[i][0] != EOF) && (i < 20))
    printf("%s\n", image[i++]);

  printf("\n");

  return;
}

/* Parser code: */

void nullInit(nodes board, pieces black_pieces, pieces white_pieces,
	      classes node_classes) {
  int i, j;

  for(i = 0; i < MAX_CLASSES; i++) {
    node_classes[i].player = INVALID;
  }

  for(i = 0; i < MAX_NODES; i++) {
    board[i].name = 0;
    board[i].game_piece = NULL;
    board[i].str_line = -1;
    board[i].str_index = -1;
    for(j = 0; j < MAX_NODES-1; j++) {
      board[i].adjacent[j] = NULL;
    }
  }

  for(i = 0; i < MAX_PIECES; i++) {
    black_pieces[i].node = white_pieces[i].node = -1;
    black_pieces[i].pic = white_pieces[i].pic = '*';
  }

  num_nodes = 0;
  num_black = 0;
  num_white = 0;
  version = 0;
}

int procFile(FILE* graph_file, nodes board, pieces black_pieces,
	      pieces white_pieces, classes node_classes) {
  char token[2000];
  char* arg1;
  int token_pos = 0;

  while(fileGetToken(graph_file, token)) {
    token_pos = 0;
    arg1 = stringGetToken(token, &token_pos);
    if(!strcmp(arg1, "graph") || !strcmp(arg1, "g"))
      handleGraph(token, &token_pos);
    else if(!strcmp(arg1, "node-class") || !strcmp(arg1, "nc"))
      handleNodeClass(token, &token_pos, board, node_classes);
    else if(!strcmp(arg1, "piece-class") || !strcmp(arg1, "pc"))
      handlePieceClass(token, &token_pos, board, black_pieces, 
		       white_pieces);
    else if(!strcmp(arg1, "node") || !strcmp(arg1, "n"))
      handleNodeDef(token, &token_pos, board);
    else if(!strcmp(arg1, "image") || !strcmp(arg1, "i"))
      handleImage(token, &token_pos, board);
    else
      return 0;
  }

  return 1;
}

void handleGraph(char* token, int* pos) {
  char* graph_name = stringGetToken(token, pos);
  version = atoi(stringGetToken(token, pos));
  strcpy(g_name, graph_name);

  return;
}

void handleNodeClass(char* token, int* pos,
		     nodes board, classes node_classes) {
  static int class_num = 0;
  char* players, * list, * list_token;
  int i, list_pos;

  strcpy(node_classes[class_num].name, stringGetToken(token, pos));

  addToHash(nameHash(node_classes[class_num].name, MAX_CLASSES), 
	    class_num, CLASS_TABLE);
  
  players = stringGetToken(token, pos);
  if(strcmp(players, "both")) {
    if(!strcmp(players, "black"))
      node_classes[class_num].player = BLACK_PLAYER;
    else if(!strcmp(players, "white"))
      node_classes[class_num].player = WHITE_PLAYER;
  } else
    node_classes[class_num].player = BOTH_PLAYERS;
  
  list = stringGetToken(token, pos);
  list_pos = 0;
  i = 0;
  while((list_token = stringGetToken(list, &list_pos))) {
    node_classes[class_num].moveto[i] = 
      &node_classes[hashLookup(list_token, node_classes, NULL)];
    i++;
  }

  list = stringGetToken(token, pos);
  list_pos = 0;
  while((list_token = stringGetToken(list, &list_pos))) {
    list_token[1] = '\0';
    addToHash(nameHash(list_token, MAX_NODES), num_nodes, NODES_TABLE);
    board[num_nodes].name = list_token[0];
    board[num_nodes].class = class_num;
    num_nodes++;
  }

  return;
}

void handlePieceClass(char* token, int* pos, nodes board,
		      pieces black_pieces, pieces white_pieces) {
  char* name, * pic_str, * list, * list_token;
  struct Piece** current_class;
  int* counter;
  int list_pos, current_node;

  name = stringGetToken(token, pos);
  if(strcmp(name, "black")) {
    current_class = &black_pieces;
    counter = &num_black;
  } else {
    current_class = &white_pieces;
    counter = &num_white;
  }

  pic_str = stringGetToken(token, pos);
  (*current_class)[*counter].pic = pic_str[0];

  list = stringGetToken(token, pos);
  list_pos = 0;
  while((list_token = stringGetToken(list, &list_pos))) {
    current_node = hashLookup(list_token, NULL, board);
    (*current_class)[*counter].pic = pic_str[0];
    (*current_class)[*counter].node = current_node;
    board[current_node].game_piece = &((*current_class)[*counter]);
    (*counter)++;
  }

  return;
}

void handleNodeDef(char* token, int* pos, nodes board) {
  char* name, * list, * list_token;
  int list_pos, node_num, current_neighbor;
  int i, j;

  name = stringGetToken(token, pos);
  node_num = hashLookup(name, NULL, board);

  /* Process the directed list. */
  list = stringGetToken(token, pos);
  list_pos = 0;
  i = 0;
  while((list_token = stringGetToken(list, &list_pos))) {
    current_neighbor = hashLookup(list_token, NULL, board);
    board[node_num].adjacent[i] = &board[current_neighbor];
    i++;
  }

  /* Process the connected list. */
  list = stringGetToken(token, pos);
  list_pos = 0;
  i = 0;
  while((list_token = stringGetToken(list, &list_pos))) {
    current_neighbor = hashLookup(list_token, NULL, board);
    board[node_num].adjacent[i] = &board[current_neighbor];

    j = 0;
    while(board[current_neighbor].adjacent[j])
      j++;
    board[current_neighbor].adjacent[j] = &board[node_num];

    i++;
  }

  return;
}

void handleImage(char* token, int* pos, nodes board) {
  char* c_line;
  char name[2];
  int i, j, hash_lookup;

  if(isspace(token[*pos]))
    (*pos)++;

  i = 0;
  c_line = stringGetLine(token + *pos);
  name[1] = '\0';
  while(c_line) {
    strcpy(image[i], c_line);
    j = 0;
    while(image[i][j] && (image[i][j] != '\n') 
	  && (image[i][j] != EOF)) {
      name[0] = image[i][j];
      hash_lookup = hashLookup(name, NULL, board);
      if(hash_lookup != -1) {
	board[hash_lookup].str_line = i;
	board[hash_lookup].str_index = j;
      }
      j++;
    }
    c_line = stringGetLine(NULL);
    i++;
  }

  if(i < 20)
    image[i][0] = -1;

  return;
}

int fileGetToken(FILE* graph_file, char* token) {
  char current;
  int i = 0;
  int unmatched_parens = 0;
  int is_plain_text = 0;

  /* Skip whitespace and any unprintable junk. */
  do 
    current = fgetc(graph_file);
  while(current && (current != EOF) && (current != '(') 
	&& !isgraph((int)current));

  if(feof(graph_file) || ferror(graph_file))
    return 0;

  /* Determine the type of token. 
     The plain text type is not currently used. */
  if(current == '(')
    unmatched_parens = 1;
  else if(current == ';') {
    while(current && (current != EOF) && (current != '\n'))
      current = fgetc(graph_file);
    return fileGetToken(graph_file, token);
  } else {
    token[0] = current;
    i++;
    is_plain_text = 1;
  }
 
  /* Increment through the file. */
  while(unmatched_parens || is_plain_text) {
    current = fgetc(graph_file);

    /* Skip any mid-token comments. */
    if(current == ';') {
      while(current == ';') {
	while(current && (current != EOF) && (current != '\n'))
	  current = fgetc(graph_file);
      }
    }

    if(is_plain_text && !isgraph((int)current))
      break;
    if(!is_plain_text) {
      if(current == '(')
	unmatched_parens++;
      else if(current == ')')
	unmatched_parens--;
      if(!unmatched_parens)
	break;
    }
    token[i] = current;
    i++;
  }

  token[i] = '\0';

  return 1;
}

char* stringGetToken(char* s, int* position) {
  char* start;
  int unmatched_parens = 0; 
  int is_plain_text = 0;

  
  /* Place the position at the first open parentheses or
     alphanumeric character. */
  while(s[*position] && (s[*position] != '(') && 
	!isalnum((int)s[*position]))
    (*position)++;

  if(!s[*position])
    return NULL;

  /* Determine the type of the current token. */
  start = s + *position;
  if(s[*position] == '(') {
    (*position)++;
    start++;
    unmatched_parens++;
  } else
    is_plain_text = 1;

  /* Increment through the string until the end of the token is
     reached (when either there are no more alphanumeric characters
     or '-', when a closed parentheses is found, or EOF). */
  if(is_plain_text) {
    do
      (*position)++;
    while(isalnum((int)s[*position]) || (s[*position] == '-'));
    if(s[(*position)]) {
      s[(*position)] = '\0';
      (*position)++;
    }
  } else { 
    while(unmatched_parens && s[*position]) {
      if(s[*position] == ')')
	  unmatched_parens--;
      else if(s[*position] == '(')
	  unmatched_parens++;      
      (*position)++;
    }
    if(s[*position - 1] == ')')
      s[*position - 1] = '\0';
  }

  return start;
}

char* stringGetLine(char* s) {
  static int position;
  static char* str;
  char* start;
 
  if(s) {
    position = 0;
    str = s;
  }
  
  start = str + position;

  if(!str[position] || (str[position] == EOF))
    return NULL;

  while(str[position] && (str[position] != '\n'))
    position++;
  
  if(str[position]) {
    str[position] = '\0';
    position++;
  }

  return start;
}

int nameHash(char* name, int hashsize) {
  int i;
  int hashed = 0;

  for(i = 0; i < 5; i++) {
    if(!name[i])
      break;
    hashed += name[i];
  }

  return hashed % hashsize;
}

#define CLASS_LOOKUP classes_lookup[hash_val][collision_index]
#define NODE_LOOKUP nodes_lookup[hash_val][collision_index]

void addToHash(int hash_val, int num_val, int table) {
  int collision_index = 0;

  if(table == CLASS_TABLE) {
    while(CLASS_LOOKUP && (collision_index < MAX_CLASSES))
      collision_index++;
    classes_lookup[hash_val][collision_index] = num_val + 1;
  }
  else {
    while(NODE_LOOKUP)
      collision_index++;
    nodes_lookup[hash_val][collision_index] = num_val + 1;
  }

  return;
}

int hashLookup(char* token, classes node_classes, nodes board) {
  int collision_index = 0;
  int hash_val;

  if(node_classes) {
    hash_val = nameHash(token, MAX_CLASSES);
    while(CLASS_LOOKUP &&
	  strcmp(node_classes[CLASS_LOOKUP-1].name, token) &&
	  (collision_index < MAX_CLASSES))
      collision_index++;
    if(collision_index >= MAX_CLASSES)
      return -1;
    return CLASS_LOOKUP-1;
  } else if(board) {
    hash_val = nameHash(token, MAX_NODES);
    while(NODE_LOOKUP && 
	  (board[NODE_LOOKUP-1].name != token[0]) &&
	  (collision_index < MAX_NODES))
      collision_index++;
    if(collision_index >= MAX_NODES)
      return -1;
    return NODE_LOOKUP-1;
  } else
    return -1;
}

/* End parser code. */
