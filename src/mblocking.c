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
**              10 Apr, 04: Many bugfixes throughout the code, changes in
**                          output functions, and (untested) support for
**                          node classes.
**              11 Apr, 04: Changed handling of the image array to work
**                          with changing the graph file.
**                          Added error-catching to the parser and put in
**                          kHelpExample.
**              19 Apr, 04: Fixed more bugs, changed the output format.
**              23 Apr, 04: Minor changes in primitive and output,
**                          a little more error checking.
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
#include <dirent.h>
#include "hash.h"

extern STRING gValueString[];

POSITION gNumberOfPositions  = 0;

POSITION gInitialPosition    = 0;
POSITION gMinimalPosition    = 0;
POSITION kBadPosition        = -1;

STRING kAuthorName         = "Greg Bonin and Tanya Gordeeva";
STRING kGameName           = "Blocking";
STRING kDBName             = "blocking";
BOOLEAN kPartizan           = TRUE;
BOOLEAN kSupportsHeuristic  = FALSE;
BOOLEAN kSupportsSymmetries = FALSE;
BOOLEAN kSupportsGraphics   = FALSE;
BOOLEAN kDebugMenu          = FALSE;
BOOLEAN kGameSpecificMenu   = TRUE;
BOOLEAN kTieIsPossible      = FALSE;
BOOLEAN kLoopy               = TRUE;
BOOLEAN kDebugDetermineValue = FALSE;
void*    gGameSpecificTclInit = NULL;

STRING kHelpGraphicInterface =
        "Not written yet";

STRING kHelpTextInterface    =
        "Type in first the number of the node your piece is moving from and\n\
second the number of the node your piece is moving to.\n\
Ex: 1 2 to move a piece in node 1 to node 2.\n"                                                                                                                                         ;

STRING kHelpOnYourTurn =
        "Move one of your pieces to an empty spot on the board.\n\
Some graphs may have restrictions on what nodes your piece may visit,\n\
see the graph file for more details (the default has no such restrictions).\n"                                                                                                                                            ;

STRING kHelpStandardObjective =
        "Move your pieces such that your opponent is trapped (ie, cannot move).\n";

STRING kHelpReverseObjective =
        "Move your pieces such that your opponent is forced to trap you.\n";

STRING kHelpTieOccursWhen =   /* Should follow 'A Tie occurs when... */
                            "";

STRING kHelpExample =
        "Black (B)'s turn:\n\n\
BOARD                                   LEGEND\n\
B   B                                   1   2\n\
|\\ /|                                   |\\ /|\n\
| _ |                                   | 3 |\n\
|/ \\|                                   |/ \\|\n\
W---W                                   4---5\n\n\
(Player should draw)\n\
Player's move [1-5 1-5/u(undo)]: {1 3}\n\n\
White (W)'s turn:\n\n\
BOARD                                   LEGEND\n\
_   B                                   1   2\n\
|\\ /|                                   |\\ /|\n\
| B |                                   | 3 |\n\
|/ \\|                                   |/ \\|\n\
W---W                                   4---5\n\n\
(Computer should draw)\n\
Computer's move : 4 to 1\n\n\
Black (B)'s turn:\n\n\
BOARD                                   LEGEND\n\
W   B                                   1   2\n\
|\\ /|                                   |\\ /|\n\
| B |                                   | 3 |\n\
|/ \\|                                   |/ \\|\n\
_---W                                   4---5\n\n\
(Player should draw)\n\
Player's move [1-5 1-5/u(undo)]: {3 4}\n\n\
White (W)'s turn:\n\n\
BOARD                                   LEGEND\n\
W   B                                   1   2\n\
|\\ /|                                   |\\ /|\n\
| _ |                                   | 3 |\n\
|/ \\|                                   |/ \\|\n\
B---W                                   4---5\n\n\
(Computer should draw)\n\
Computer's move : 5 to 3\n\n\
Black (B)'s turn:\n\n\
BOARD                                   LEGEND\n\
W   B                                   1   2\n\
|\\ /|                                   |\\ /|\n\
| W |                                   | 3 |\n\
|/ \\|                                   |/ \\|\n\
B---_                                   4---5\n\n\
Black (B)'s turn:\n\n\
BOARD                                   LEGEND\n\
W   B                                   1   2\n\
|\\ /|                                   |\\ /|\n\
| W |                                   | 3 |\n\
|/ \\|                                   |/ \\|\n\
B---_                                   4---5\n\n\
(Player should draw)\n\
Player's move [1-5 1-5/u(undo)]: {4 5}\n\n\
White (W)'s turn:\n\n\
BOARD                                   LEGEND\n\
W   B                                   1   2\n\
|\\ /|                                   |\\ /|\n\
| W |                                   | 3 |\n\
|/ \\|                                   |/ \\|\n\
_---B                                   4---5\n\n\
(Computer will Win in 1)\n\
Computer's move : 1 to 4\n\n\
Black (B)'s turn:\n\n\
BOARD                                   LEGEND\n\
_   B                                   1   2\n\
|\\ /|                                   |\\ /|\n\
| W |                                   | 3 |\n\
|/ \\|                                   |/ \\|\n\
W---B                                   4---5\n\n\
(Player will Lose in 0)\n\n\
Computer wins. Nice try, Player.\n"                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            ;


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
	struct GraphClass* moveto[MAX_CLASSES];
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
static char default_file[100] = "../meta/default.blk";
static BOOLEAN use_default = TRUE;
static char image[20][35];

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

BOOLEAN checkNodeAndClass(struct GraphNode* to_node, struct GraphNode *from_node,
                          int player);

int moveHash(int to, int from);
void moveUnHash(int move, int* to, int* from);

char* boardToString(char* s, nodes board, pieces black_pieces,
                    pieces white_pieces);
void stringToBoard(char*s, nodes board, pieces black_pieces,
                   pieces white_pieces);

void printBoard(nodes board, BOOLEAN reset);


/* Starting here are the function prototypes for the parser: */

/* Initialize all the entries to 0. */
void nullInit(nodes board, pieces black_pieces, pieces white_pieces,
              classes node_classes);

/* Processes the file. */
int procFile(FILE* graph_file, nodes board, pieces black_pieces,
             pieces white_pieces, classes node_classes);

int handleGraph(char* token, int* pos);
int handleNodeClass(char* token, int* pos,
                    nodes board, classes node_classes, BOOLEAN reset);
int handlePieceClass(char* token, int* pos, nodes board,
                     pieces black_pieces, pieces white_pieces);
int handleNodeDef(char* token, int* pos, nodes board);
int handleImage(char* token, int* pos, nodes board);

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

/* Addition to list the contents of a directory. -JJ */
void    PrettyPrintDir (const char*, const char*);

/* External */
#ifndef MEMWATCH
extern GENERIC_PTR      SafeMalloc ();
extern void             SafeFree ();
#endif


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

	int hash_array[10];
	char string_board[MAX_NODES];

	if(use_default) {
		g_file = fopen(default_file, "r");
		if(!g_file) {
			printf("InitializeGame error: default file %s does not exist.\n",
			       default_file);
			ExitStageRight();
		}
		nullInit(global_board, global_black, global_white, global_classes);
		procFile(g_file, global_board, global_black, global_white,
		         global_classes);
	}

	boardToString(string_board, global_board, global_black, global_white);

	hash_array[0] = global_black[0].pic;
	hash_array[1] = hash_array[2] = num_black;
	hash_array[3] = global_white[0].pic;
	hash_array[4] = hash_array[5] = num_white;
	hash_array[6] = '_';
	hash_array[7] = hash_array[8] = num_nodes - num_black - num_white;
	hash_array[9] = -1;

	if (kDBName && strcmp(kDBName, "blocking") != 0)
		SafeFree(kDBName);

	kDBName = (STRING) SafeMalloc(sizeof(char)*100);
	sprintf(kDBName, "%.6s", g_name);

	gNumberOfPositions = generic_hash_init(num_nodes, hash_array, NULL, 0);
	gInitialPosition = generic_hash_hash(string_board, 1);

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
	char tmp[100];
	BOOLEAN no_errors;

	use_default = TRUE;

	while(use_default) {
		/*printf("\nSpecify one of the .blk files in the ../meta directory");
		   printf("\n(But don't add the .blk at the end)\n\n");
		   system("ls -C ../meta"); */
		printf("\nSpecify one of the following definition files:\n\n");
		PrettyPrintDir("../meta", ".blk");

		printf("\nLoad Graph from : ");
		scanf("%s", tmp);
		(void) sprintf((char *)file_name, "../meta/%s.blk", tmp);

		g_file = fopen(file_name, "r");

		if(!g_file) {
			printf("Invalid file name\n");
		} else {
			nullInit(global_board, global_black, global_white, global_classes);
			no_errors = procFile(g_file, global_board, global_black, global_white,
			                     global_classes);
			if(no_errors)
				use_default = FALSE;
		}

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
** CALLS:       POSITION generic_hash_hash ()
**              void generic_hash_unhash ()
**	        int generic_hash_turn ()
**
*************************************************************************/

POSITION DoMove (thePosition, theMove)
POSITION thePosition;
MOVE theMove;
{

	int to, from;
	int player;
	char string_board[MAX_NODES];

	generic_hash_unhash(thePosition, string_board);
	string_board[num_nodes] = '\0';
	moveUnHash(theMove, &to, &from);
	string_board[to] = string_board[from];
	string_board[from] = '_';

	if(generic_hash_turn(thePosition) == BLACK_PLAYER)
		player = WHITE_PLAYER;
	else
		player = BLACK_PLAYER;

	return generic_hash_hash(string_board, player);
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
	int current_node, i, player;
	char* string_board = (char*)SafeMalloc(sizeof(char)*MAX_NODES);

	for(i = 0; i < num_nodes; i++)
		string_board[i] = '_';

	printBoard(global_board, FALSE);
	printf("Enter the nodes of the black pieces, ending with -1:\n");

	scanf("%d", &current_node);
	while(current_node != -1) {
		string_board[current_node-1] = global_black[0].pic;
		scanf("%d", &current_node);
	}

	printf("Enter the nodes of the white pieces, ending with -1:\n");

	scanf("%d", &current_node);
	while(current_node != -1) {
		string_board[current_node-1] = global_white[0].pic;
		scanf("%d", &current_node);
	}

	printf("Enter the number of the player:\n");
	scanf("%d", &player);

	string_board[num_nodes] = '\0';

	initialPosition = generic_hash_hash(string_board, player);

	free(string_board);

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

	printf("%s's move\t: %d to %d\n", computersName, from+1, to+1);

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
** CALLS:       int generic_hash_turn ()
**              void generic_hash_unhash ()
**              stringToBoard ()
**
**
************************************************************************/

VALUE Primitive (pos)
POSITION pos;
{
	int generic_hash_turn();

	char string_board[MAX_NODES];
	int player = generic_hash_turn(pos);
	struct Piece* pieces_ptr;
	struct GraphNode* current_node;
	int num, i, j;

	generic_hash_unhash(pos, string_board);
	string_board[num_nodes] = '\0';
	stringToBoard(string_board, global_board, global_black, global_white);

	if(player == BLACK_PLAYER) {
		pieces_ptr = global_black;
		num = num_black;
	} else {
		pieces_ptr = global_white;
		num = num_white;
	}

	for(i = 0; i < num; i++) {
		current_node = global_board[pieces_ptr[i].node].adjacent[0];
		j = 0;
		while(current_node && (j < (num_nodes-1))) {
			if(checkNodeAndClass(current_node, &global_board[pieces_ptr[i].node],
			                     player))
				return undecided;
			j++;
			current_node = global_board[pieces_ptr[i].node].adjacent[j];
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
** CALLS:       void generic_hash_unhash()
**              STRING GetPrediction()
**              void stringToBoard()
**
************************************************************************/

void PrintPosition (position, playerName, usersTurn)
POSITION position;
STRING playerName;
BOOLEAN usersTurn;
{
	char string_board[MAX_NODES];

	generic_hash_unhash(position, string_board);
	string_board[num_nodes] = '\0';
	stringToBoard(string_board, global_board, global_black,
	              global_white);

	if(generic_hash_turn(position) == BLACK_PLAYER)
		printf("\nBlack (%c)'s turn:\n", global_black[0].pic);
	else
		printf("\nWhite (%c)'s turn:\n", global_white[0].pic);

	printBoard(global_board, FALSE);
	printf("%s\n", GetPrediction(position, playerName, usersTurn));

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
**              MOVELIST* CreateMovelistNode()
**              BOOLEAN checkNodeAndClass()
**
************************************************************************/

MOVELIST *GenerateMoves (position)
POSITION position;
{
	MOVELIST *CreateMovelistNode(), *head = NULL;
	int generic_hash_turn();
	VALUE Primitive();

	char string_board[MAX_NODES];
	int player, i, j, max_pieces;
	struct GraphNode* current_node;
	pieces* user_pieces;

	if(Primitive(position) == undecided) {
		player = generic_hash_turn(position);
		if(player == BLACK_PLAYER) {
			max_pieces = num_black;
			user_pieces = &global_black;
		} else {
			max_pieces = num_white;
			user_pieces = &global_white;
		}

		generic_hash_unhash(position, string_board);
		string_board[num_nodes] = '\0';
		stringToBoard(string_board, global_board, global_black, global_white);
		for(i = 0; i < max_pieces; i++) {
			for(j = 0; j < (num_nodes-1); j++) {
				current_node = global_board[(*user_pieces)[i].node].adjacent[j];
				if(checkNodeAndClass(current_node,
				                     &global_board[(*user_pieces)[i].node],
				                     player))
					head = CreateMovelistNode(
					        moveHash(
					                hashLookup(&current_node->name, NULL, global_board),
					                (*user_pieces)[i].node),
					        head);
			}
		}

		return (head);
	} else {
		return (NULL);
	}
}

BOOLEAN checkNodeAndClass(struct GraphNode* to_node, struct GraphNode* from_node,
                          int player)
{
	struct GraphClass* current_class;
	int i;
	BOOLEAN valid_move = FALSE;

	if(!to_node)
		return FALSE;

	if(to_node->game_piece)
		return FALSE;

	if((global_classes[to_node->class].player != BOTH_PLAYERS) &&
	   (global_classes[to_node->class].player != player))
		return FALSE;

	i = 0;
	current_class = global_classes[from_node->class].moveto[i];
	while(current_class &&
	      (current_class->player != INVALID) && (i < MAX_CLASSES)) {
		if((!strcmp(global_classes[to_node->class].name, current_class->name))
		   &&
		   ((current_class->player == global_classes[to_node->class].player) ||
		    (current_class->player == BOTH_PLAYERS)))
			return TRUE;
		i++;
		current_class = global_classes[from_node->class].moveto[i];
	}

	return valid_move;
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
		printf("%s's move [1-%d 1-%d/u(undo)]: ", playerName, num_nodes,
		       num_nodes);

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
	int from, to;

	sscanf(input, "%d %d", &from, &to);

	if((from < 1) || (to < 1) || (from > num_nodes) || (to > num_nodes))
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
	int to, from;

	sscanf(input, "%d %d", &from, &to);
	from--;
	to--;

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

	printf("[%d %d]", from+1, to+1);

	return;
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
	return (to << 16) | from;
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
	s[i] = '\0';

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

void printBoard(nodes board, BOOLEAN reset) {
	static char board_numbers[20][35];
	static BOOLEAN initialized = FALSE;
	char temp;
	int i, num_written;

	if(reset) {
		initialized = FALSE;
		return;
	}

	if(!initialized) {
		for(i = 0; i < 20; i++)
			strcpy(board_numbers[i], image[i]);
		for(i = 0; i < num_nodes; i++) {
			temp = board_numbers[board[i].str_line][board[i].str_index+1];
			num_written =
			        sprintf(&board_numbers[board[i].str_line][board[i].str_index],
			                "%d", i+1);
			board_numbers[board[i].str_line][board[i].str_index+num_written] =
			        temp;
		}
		initialized = TRUE;
	}

	printf("\nBOARD\t\t\t\t\tLEGEND\n");

	for(i = 0; i < num_nodes; i++) {
		if(board[i].game_piece)
			image[board[i].str_line][board[i].str_index] =
			        board[i].game_piece->pic;
		else
			image[board[i].str_line][board[i].str_index] = '_';
	}

	i = 0;
	while((image[i][0] != EOF) && (i < 20)) {
		printf("%s\t%s\n", image[i], board_numbers[i]);
		i++;
	}

	printf("\n");

	return;
}

/* Parser code: */

void nullInit(nodes board, pieces black_pieces, pieces white_pieces,
              classes node_classes) {
	int i, j;

	for(i = 0; i < MAX_CLASSES; i++) {
		node_classes[i].player = INVALID;
		for(j = 0; j < MAX_CLASSES; j++) {
			node_classes[i].moveto[j] = NULL;
		}
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

	for(i = 0; i < 20; i++) {
		image[i][0] = '\0';
	}

	printBoard(NULL, TRUE);

	for(i = 0; i < MAX_CLASSES; i++)
		for(j = 0; j < MAX_CLASSES; j++)
			classes_lookup[i][j] = 0;

	for(i = 0; i < MAX_NODES; i++)
		for(j = 0; j < MAX_NODES; j++)
			nodes_lookup[i][j] = 0;

	handleNodeClass(NULL, NULL, NULL, NULL, TRUE);

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
	int ret = 1;

	while(fileGetToken(graph_file, token)) {
		token_pos = 0;
		arg1 = stringGetToken(token, &token_pos);
		if(!arg1)
			break;
		if(!strcmp(arg1, "graph") || !strcmp(arg1, "g"))
			ret &= handleGraph(token, &token_pos);
		else if(!strcmp(arg1, "node-class") || !strcmp(arg1, "nc"))
			ret &= handleNodeClass(token, &token_pos, board, node_classes,
			                       FALSE);
		else if(!strcmp(arg1, "piece-class") || !strcmp(arg1, "pc"))
			ret &= handlePieceClass(token, &token_pos, board, black_pieces,
			                        white_pieces);
		else if(!strcmp(arg1, "node") || !strcmp(arg1, "n"))
			ret &= handleNodeDef(token, &token_pos, board);
		else if(!strcmp(arg1, "image") || !strcmp(arg1, "i"))
			ret &= handleImage(token, &token_pos, board);
		else {
			printf("procFile error: unknown token: %s\n", arg1);
			ret = 0;
		}
	}

	if(num_black == 0) {
		printf("procFile error: no black pieces declared.\n");
		ret = 0;
	}

	if(num_white == 0) {
		printf("procFile error: no white pieces declared.\n");
		ret = 0;
	}

	if(!ret) {
		printf("procFile encountered errors while interpreting the file.\n");
		printf("See the above error notices, if any.\n");
	}

	return ret;
}

int handleGraph(char* token, int* pos) {
	version = atoi(stringGetToken(token, pos));
	char* graph_name = stringGetToken(token, pos);

	if(graph_name)
		strcpy(g_name, graph_name);
	else {
		printf("handleGraph error: no graph name\n");
		return 0;
	}

	return 1;
}

int handleNodeClass(char* token, int* pos,
                    nodes board, classes node_classes,
                    BOOLEAN reset) {
	static int class_num = 0;
	char* players, * list, * list_token, * name_temp;
	int i, list_pos, ret, class_count, lookup;
	ret = 1;

	if(reset) {
		class_num = 0;
		return 1;
	}

	name_temp = stringGetToken(token, pos);
	if(!name_temp) {
		printf("handleNodeClass error: no node class name specified\n");
		return 0;
	}

	if(class_num >= MAX_CLASSES) {
		printf("handleNodeClass warning: found more classes than the max (%d)",
		       MAX_CLASSES);
		printf(", class %s has been ignored.\n", name_temp);
		return 1;
	}

	strcpy(node_classes[class_num].name, name_temp);

	addToHash(nameHash(name_temp, MAX_CLASSES),
	          class_num, CLASS_TABLE);

	players = stringGetToken(token, pos);

	if(!players) {
		printf("handleNodeClass error: no players specified\n");
		return 0;
	}

	if(!strcmp(players, "both"))
		node_classes[class_num].player = BOTH_PLAYERS;
	else if(!strcmp(players, "black"))
		node_classes[class_num].player = BLACK_PLAYER;
	else if(!strcmp(players, "white"))
		node_classes[class_num].player = WHITE_PLAYER;
	else {
		printf("handleNodeClass error: invalid players: %s\n", players);
		ret = 0;;
	}

	/* Process the moveto list. */
	list = stringGetToken(token, pos);

	if(!list) {
		printf("handleNodeClass error: no moveto list for class %s\n",
		       node_classes[class_num].name);
		return 0;
	}

	list_pos = 0;
	i = 0;
	while((list_token = stringGetToken(list, &list_pos))) {
		lookup = hashLookup(list_token, node_classes, NULL);
		if(lookup != -1) {
			node_classes[class_num].moveto[i] = &node_classes[lookup];
			i++;
		} else {
			printf("handleNodeClass error: unknown class in moveto list: %s\n",
			       list_token);
			ret = 0;
		}
	}

	/* Process the node declarations. */
	list = stringGetToken(token, pos);

	if(!list) {
		printf("handleNodeClass error: no nodelist in class %s\n",
		       node_classes[class_num].name);
		return 0;
	}

	class_count = num_nodes;
	list_pos = 0;
	while((list_token = stringGetToken(list, &list_pos))
	      && (num_nodes < MAX_NODES)) {
		list_token[1] = '\0';
		addToHash(nameHash(list_token, MAX_NODES), num_nodes, NODES_TABLE);
		board[num_nodes].name = list_token[0];
		board[num_nodes].class = class_num;
		num_nodes++;
	}

	if(num_nodes >= MAX_NODES) {
		printf("handleNodeClass warning: found more nodes than the max (%d)",
		       MAX_NODES);
		printf(", extra nodes have been ignored.\n");
	}

	if(class_count == num_nodes) {
		printf("handleNodeClass error: no nodes in class %s\n",
		       node_classes[class_num].name);
		return 0;
	}

	class_num++;

	return ret;
}

int handlePieceClass(char* token, int* pos, nodes board,
                     pieces black_pieces, pieces white_pieces) {
	char* name, * pic_str, * list, * list_token;
	struct Piece** current_class;
	int* counter;
	int list_pos, current_node, ret;
	ret = 1;

	name = stringGetToken(token, pos);

	if(!name) {
		printf("handlePieceClass error: no piece class name specified\n");
		return 0;
	}

	if(!strcmp(name, "black")) {
		current_class = &black_pieces;
		counter = &num_black;
	} else if(!strcmp(name, "white")) {
		current_class = &white_pieces;
		counter = &num_white;
	} else {
		printf("handlePieceClass error: unsupported piece class name: %s\n",
		       name);
		return 0;
	}

	pic_str = stringGetToken(token, pos);

	if(!pic_str) {
		printf("handlePieceClass error: no picture or node list for pieces");
		printf(" of type %s\n", name);
		return 0;
	}

	(*current_class)[*counter].pic = pic_str[0];

	/* Process the list of nodes with this piece. */
	list = stringGetToken(token, pos);

	if(!list) {
		printf("handlePieceClass error: no node list for pieces of type %s\n",
		       name);
		return 0;
	}

	list_pos = 0;
	while((list_token = stringGetToken(list, &list_pos))) {
		current_node = hashLookup(list_token, NULL, board);
		if(current_node < 0) {
			printf("handlePieceClass error: unknown node %c\n", list_token[0]);
			ret = 0;
		} else {
			(*current_class)[*counter].pic = pic_str[0];
			(*current_class)[*counter].node = current_node;
			board[current_node].game_piece = &((*current_class)[*counter]);
			(*counter)++;
		}
	}

	if((*counter) < 1) {
		printf("handlePieceClass error: no pieces of type %s declared\n",
		       name);
		return 0;
	}

	return ret;
}

int handleNodeDef(char* token, int* pos, nodes board) {
	char* node_name, * list, * list_token;
	int list_pos, node_num, current_neighbor;
	int i, j, ret;
	ret = 1;

	node_name = stringGetToken(token, pos);

	if(!node_name) {
		printf("handleNodeDef error: no node name\n");
		return 0;
	}

	node_num = hashLookup(node_name, NULL, board);

	if(node_num < 0) {
		printf("handleNodeDef error: unknown node %c\n",
		       node_name[0]);
		return 0;
	}

	/* Process the directed list. */
	list = stringGetToken(token, pos);
	list_pos = 0;
	i = 0;

	if(!list) {
		printf("handleNodeDef: no directed or connected list present");
		printf(" for node %c\n", node_name[0]);
		return 0;
	}

	while(board[node_num].adjacent[i])
		i++;

	while((list_token = stringGetToken(list, &list_pos))) {
		current_neighbor = hashLookup(list_token, NULL, board);
		if(current_neighbor < 0) {
			printf("handleNodeDef error: unknown node %c in the directed list\n",
			       list_token[0]);
			ret = 0;
		} else {
			board[node_num].adjacent[i] = &board[current_neighbor];
			i++;
		}
	}

	/* Process the connected list. */
	list = stringGetToken(token, pos);
	list_pos = 0;

	if(!list) {
		printf("handleNodeDef: no connected list present for node %c\n",
		       node_name[0]);
		return 0;
	}

	while((list_token = stringGetToken(list, &list_pos))) {
		current_neighbor = hashLookup(list_token, NULL, board);
		if(current_neighbor < 0) {
			printf("handleNodeDef error: unknown node %c in the connected list\n",
			       list_token[0]);
			ret = 0;
		} else {
			board[node_num].adjacent[i] = &board[current_neighbor];

			j = 0;
			while(board[current_neighbor].adjacent[j])
				j++;
			board[current_neighbor].adjacent[j] = &board[node_num];

			i++;
		}
	}

	return ret;
}

int handleImage(char* token, int* pos, nodes board) {
	char* c_line;
	char name[2];
	int i, j, hash_lookup, order_counter;

	if(isspace(token[*pos]))
		(*pos)++;

	if(token[(*pos)-1] == ' ')
		(*pos)--;

	i = 0;
	c_line = stringGetLine(token + *pos);

	order_counter = 0;
	name[1] = '\0';
	while(c_line && (i < 20)) {
		sprintf(image[i], "%.35s", c_line);
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
		while(j < 35)
			image[i][j++] = ' ';
		image[i][34] = '\0';
		c_line = stringGetLine(NULL);
		i++;
	}

	if(i < 20)
		image[i][0] = -1;

	return 1;
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
			if(!current || (current == EOF)) {
				printf("fileGetToken error: unmatched parentheses\n");
				return 0;
			}

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
		if(collision_index)
			collision_index--;
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


struct string_list {
	char*                 str;
	struct string_list*   next;
};

void PrettyPrintDir (const char* dir, const char* extension)
{
	DIR*                  d;
	struct dirent*        de;
	size_t ext_sz;
	size_t max_sz = 0;
	struct string_list*   head = NULL;
	struct string_list**  ptr = &head;
	char*                 tmp;
	int i;

	if (!(d = opendir(dir))) {
		printf("No such directory \"%s\"\n\n", dir);
		return;
	}

	ext_sz = strlen(extension);

	while ((de = readdir(d))) {
		size_t sz;
		char*       s;

		s = de->d_name;
		sz = strlen(s);
		if (sz >= ext_sz && !strcasecmp(&s[sz - ext_sz], extension)) {
			char*     cpy;

			cpy = strdup(s);
			cpy[sz - ext_sz] = 0;
			sz -= ext_sz;

			if (sz > max_sz)
				max_sz = sz;

			*ptr = (struct string_list*) SafeMalloc(sizeof(struct string_list));
			(*ptr)->str = cpy;
			(*ptr)->next = NULL;
			ptr = &(*ptr)->next;
		}
	}

	closedir(d);

	max_sz += 4; // How many spaces do you want?
	tmp = (char*) SafeMalloc(max_sz + 1);
	tmp[max_sz] = 0;

	i = 0;
	while (head) {
		struct string_list* next;

		memset(tmp, ' ', max_sz);
		memcpy(tmp, head->str, strlen(head->str));
		printf("%s", tmp);
		if ((i += max_sz) + max_sz > 80) {
			printf("\n");
			i = 0;
		}
		next = head->next;
		free(head->str);
		free(head);
		head = next;
	}

	if (i)
		printf("\n");

	SafeFree(tmp);
}
POSITION StringToPosition(char* board) {
	// FIXME: this is just a stub
	return atoi(board);
}


char* PositionToString(POSITION pos) {
	// FIXME: this is just a stub
	return "Implement Me";
}

STRING MoveToString(MOVE theMove) {
	return "Implement MoveToString";
}

char * PositionToEndData(POSITION pos) {
	return NULL;
}
