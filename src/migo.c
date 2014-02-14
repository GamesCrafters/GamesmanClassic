/************************************************************************
**
** NAME:        migo.c
**
** DESCRIPTION: Go and its variants
**
** AUTHOR:      Alexander D'Archangel
**
** DATE:        Nov 29, 2006
**
** UPDATE HIST: 2006.11.29:  Imported
**
** LAST CHANGE: $Id$
**              2/9/08 Implemented the three option functions.
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


/*************************************************************************
**
** Game-specific constants
**
**************************************************************************/

/**
 * externs
 */
STRING kAuthorName         = "Alexander D'Archangel";
STRING kGameName           = "Go";
STRING kDBName                         = "igo";

BOOLEAN kPartizan            = TRUE;
BOOLEAN kGameSpecificMenu    = FALSE;   /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN kTieIsPossible       = FALSE;   /* TRUE if a tie is possible. FALSE if it is impossible.*/
BOOLEAN kLoopy               = FALSE;   /* TRUE if the game tree will have cycles (a rearranger style game). FALSE if it does not.*/

BOOLEAN kDebugMenu           = FALSE;   /* TRUE only when debugging. FALSE when on release. */
BOOLEAN kDebugDetermineValue = FALSE;   /* TRUE only when debugging. FALSE when on release. */

POSITION gNumberOfPositions   =  0; /* The number of total possible positions | If you are using our hash, this is given by the hash_init() function*/
POSITION gInitialPosition     =  0; /* The initial hashed position for your starting board */
POSITION kBadPosition         = -1; /* A position that will never be used */

void*    gGameSpecificTclInit = NULL;

/**
 * Help strings that are pretty self-explanatory
 * Strings than span more than one line should have backslashes (\) at the end of the line.
 * These help strings should be updated and dynamically changed using
 * InitializeHelpStrings()
 **/

STRING kHelpGraphicInterface =
        "Help strings not initialized!";

STRING kHelpTextInterface =
        "Help strings not initialized!";

STRING kHelpOnYourTurn =
        "Help strings not initialized!";

STRING kHelpStandardObjective =
        "Help strings not initialized!";

STRING kHelpReverseObjective =
        "Help strings not initialized!";

STRING kHelpTieOccursWhen =
        "Help strings not initialized!";

STRING kHelpExample =
        "Help strings not initialized!";

/*************************************************************************
**
** Forward declarations
**
**************************************************************************/
typedef enum {
	STONE_BLACK,
	STONE_WHITE
} StoneColor;

struct GoIntersection;
struct GoIntersectionList;
struct GoBoard;
struct GoPosition;
struct GoStone; /* FIXME:  Make better */

/*************************************************************************
**
** Abstractions
**
**************************************************************************/

/* Stone abstraction */
typedef struct GoStone {
	StoneColor color;
}* GoStone;

static GoStone
newGoStone(StoneColor color);

static void
delGoStone(GoStone which);

/* Game abstraction */

typedef struct GoGame {
	struct GoPosition* position;
	/* FIXME:  More:  History, player names, other metadata */
}* GoGame;

static GoGame
newGoGame(size_t boardsize);

static void
delGoGame(GoGame which);

/* Position abstraction */
typedef struct GoPosition {
	struct GoBoard* board;
	StoneColor turn;
	GoStone lastMoveCapture;
	BOOLEAN passedOnce;
	BOOLEAN passedTwice;
}* GoPosition;

static GoPosition
newGoPosition(size_t boardsize);

static GoPosition
copyGoPosition(const GoPosition const pos);

static void
delGoPosition(GoPosition which);

#ifdef DONE
static void
addCaptives(GoPosition pos, StoneColor color, unsigned int num);
#endif

static char*
stringifyGoPosition(GoPosition pos);

static POSITION
hashPosition(GoPosition pos);

static GoPosition
unhashPosition(POSITION pos);

/* GoIntersection abstraction */

typedef struct GoIntersection {
	struct GoStone* stone;
	struct GoIntersectionList* neighbors;
}* GoIntersection;

static GoIntersection
newGoIntersectionDefault(void);

#ifdef DONE
static void
delGoIntersection(GoIntersection which);
#endif

static void
playStone(GoIntersection where, StoneColor which);

static void
insertNeighbor(GoIntersection target, GoIntersection neighbor);

/* GoIntersectionNode abstraction */
typedef struct GoIntersectionNode {
	GoIntersection data;
	struct GoIntersectionNode* next;
}* GoIntersectionNode;

/* GoIntersectionList abstraction */

typedef struct GoIntersectionList {
	GoIntersectionNode head;
}* GoIntersectionList;

static GoIntersectionList
newGoIntersectionListDefault(void);

#ifdef DONE
static void
delGoIntersectionList(GoIntersectionList which);
#endif

static void
insertGoIntersection(GoIntersectionList list, GoIntersection element);

static void
removeGoIntersection(GoIntersectionList list, GoIntersection element);

static int
findGoIntersection(GoIntersectionList list, GoIntersection element);
/* Board abstraction */

typedef struct GoBoard {
	size_t size;
	GoIntersection* matrix;
}* GoBoard;

static GoBoard
newGoBoard(size_t boardsize);

static GoBoard
copyGoBoard(const GoBoard const RHS);

static void
delGoBoard(GoBoard const gbDestructee);

static GoIntersection
getGoIntersection(const GoBoard const board, size_t const x, size_t const y);

/* Move abstraction */

typedef struct GoMove {
	unsigned short x;
	unsigned short y;
} *GoMove;

static GoMove
newGoMove(size_t x, size_t y);

static void
delGoMove(GoMove which);

static MOVE
hashMove(GoMove move);

static GoMove
unhashMove(MOVE move);

static BOOLEAN
isValidMove(GoMove move, GoPosition pos);

/* StoneString abstraction */

typedef GoIntersectionList StoneString;

static StoneString
getStoneString(const GoIntersection const start);

static unsigned int
countLiberties(const StoneString const which);

#ifdef DONE
static unsigned int
countStones(const StoneString const which);
#endif

static void
removeString(StoneString const which);

/*************************************************************************
**
** Support functions
**
*************************************************************************/

/* FIXME:  Need to refactor more of the gamesman functions and turn them
 * into variant-specific _imps. */
static GoPosition
doMove_imp(GoPosition pos, GoMove move);

void InitializeHelpStrings (void);

/*************************************************************************
**
** Global Variables
**
*************************************************************************/

/* File-scope globals */
static GoGame g_game = NULL;

/* File-scope global constants */
#define DEFAULT_BOARD_SIZE_M 3
static const size_t DEFAULT_BOARD_SIZE = DEFAULT_BOARD_SIZE_M;
static const int DEFAULT_PIECES_ARRAY[] =
#ifdef ATARIGO_ONLY
#else
{
	'-', 0, DEFAULT_BOARD_SIZE_M * DEFAULT_BOARD_SIZE_M,
	'X', 0, DEFAULT_BOARD_SIZE_M * DEFAULT_BOARD_SIZE_M,
	'O', 0, DEFAULT_BOARD_SIZE_M * DEFAULT_BOARD_SIZE_M,
	-1
};
#endif

/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/

/* External */
#ifndef MEMWATCH
        extern GENERIC_PTR      SafeMalloc ();
extern void                     SafeFree ();
#endif

        STRING                          MoveToString(MOVE move);

/*************************************************************************
**
** IMPLEMENTATION
**
*************************************************************************/

/**
 * Gamesman API functions
 * These are largely wrappers around the guts of this module.
 */

/************************************************************************
**
** NAME:        InitializeGame
**
** DESCRIPTION: Prepares the game for execution.
**              Initializes required variables.
**
************************************************************************/

void InitializeGame ()
{
	InitializeHelpStrings();
	if (g_game)
		delGoGame(g_game);
	g_game = newGoGame(DEFAULT_BOARD_SIZE);
	gNumberOfPositions = 4 * generic_hash_init(
	        DEFAULT_BOARD_SIZE * DEFAULT_BOARD_SIZE,
	        (int *) DEFAULT_PIECES_ARRAY, NULL, 0);
	gInitialPosition = hashPosition(g_game->position);
	return;
}


/************************************************************************
**
** NAME:        InitializeHelpStrings
**
** DESCRIPTION: Sets up the help strings based on chosen game options.
**
** NOTES:       Should be called whenever the game options are changed.
**              (e.g., InitializeGame() and GameSpecificMenu())
**
************************************************************************/
void InitializeHelpStrings (void)
{

	kHelpGraphicInterface =
	        "Not implemented.";

	kHelpTextInterface =
	        "";

	kHelpOnYourTurn =
	        "On each of your turns, you choose an empty intersection on the board to play\n"
	        "on.  If doing so surrounds one of the opponent's pieces, you capture that\n"
	        "piece and win the game.\n";

	kHelpStandardObjective =
	        "The normal Atari-Go objective is to surround one or more of the opponent's\n"
	        "pieces with your own and capture it.  Every empty intersection adjacent to\n"
	        "a stone is one of its liberties.  Adjacent stones of the same color share\n"
	        "liberties.  When a string of stones is entirely out of liberties -- i.e.,\n"
	        "when it is entirely surrounded by enemy stones -- it is captured and taken\n"
	        "off the board.  The capturing player wins.";

	kHelpReverseObjective =
	        "When playing backwards Atari-Go, your objective is to force the opponent\n"
	        "to capture at least one of your pieces.";

	kHelpTieOccursWhen =
#ifdef ATARIGO_ONLY
#else
	        "A tie occurs when both players pass before a stone is captured.";
#endif

	        kHelpExample =
	                "";

	gMoveToStringFunPtr = &MoveToString;

}


/************************************************************************
**
** NAME:        GenerateMoves
**
** DESCRIPTION: Creates a linked list of every move that can be reached
**              from this position. Returns a pointer to the head of the
**              linked list.
**
** INPUTS:      POSITION position : Current position to generate moves
**
** OUTPUTS:     (MOVELIST *)      : A pointer to the first item of
**                                  the linked list of generated moves
**
** CALLS:       MOVELIST *CreateMovelistNode();
**
************************************************************************/

MOVELIST *GenerateMoves (POSITION position)
{
	MOVELIST *moves = NULL;
	GoPosition pos = unhashPosition(position);

/* FIXME:  Invert */
	int i, j;
	for (i = 0; i < DEFAULT_BOARD_SIZE; ++i) {
		for (j = 0; j < DEFAULT_BOARD_SIZE; ++j) {
			GoMove move = newGoMove(i + 1, j + 1);
			if (isValidMove(move, pos)) {
				moves = CreateMovelistNode(hashMove(move), moves);
			}
			delGoMove(move);
		}
	}
	delGoPosition(pos);
	return moves;
}


/************************************************************************
**
** NAME:        DoMove
**
** DESCRIPTION: Applies the move to the position.
**
** INPUTS:      POSITION position : The old position
**              MOVE     move     : The move to apply to the position
**
** OUTPUTS:     (POSITION)        : The position that results from move
**
** CALLS:       Some Board Hash Function
**              Some Board Unhash Function
**
*************************************************************************/

POSITION DoMove (POSITION position, MOVE move)
{
	GoPosition pos = unhashPosition(position);
	POSITION result = hashPosition(doMove_imp(pos, unhashMove(move)));
	delGoPosition(pos);
	return result;
}


/************************************************************************
**
** NAME:        Primitive
**
** DESCRIPTION: Returns the value of a position if it fulfills certain
**              'primitive' constraints.
**
**              Example: Tic-tac-toe - Last piece already placed
**
**              Case                                  Return Value
**              *********************************************************
**              Current player sees three in a row    lose
**              Entire board filled                   tie
**              All other cases                       undecided
**
** INPUTS:      POSITION position : The position to inspect.
**
** OUTPUTS:     (VALUE)           : one of
**                                  (win, lose, tie, undecided)
**
** CALLS:       None
**
************************************************************************/

VALUE Primitive (POSITION position)
{
	GoPosition pos = unhashPosition(position);
	/* FIXME:  Move to evaluatePosition() */
	/* FIXME:  This is Atarigo-specific code! */
	VALUE result;
	MOVELIST *moves = NULL;
	if (pos->lastMoveCapture) {
#if DONE
		/* FIXME:  This code doesn't work; the else clause works for atarigo.
		 */
		result = pos->lastMoveCapture->color == pos->turn
		         ? (gStandardGame ? win : lose)
			 : (gStandardGame ? lose : win);
#else
		result = gStandardGame ? lose : win;
#endif
	} else if (pos->passedOnce && pos->passedTwice) {
		result = tie;
	} else if ((moves = GenerateMoves(position))) {
		MOVELIST* tmp;
		while (moves) {
			tmp = moves;
			moves = moves->next;
			SafeFree(tmp);
		}
		result = undecided;
	} else {
		result = lose;
	}

	delGoPosition(pos);
	return result;
}


/************************************************************************
**
** NAME:        PrintPosition
**
** DESCRIPTION: Prints the position in a pretty format, including the
**              prediction of the game's outcome.
**
** INPUTS:      POSITION position    : The position to pretty print.
**              STRING   playersName : The name of the player.
**              BOOLEAN  usersTurn   : TRUE <==> it's a user's turn.
**
** CALLS:       Unhash()
**              GetPrediction()      : Returns the prediction of the game
**
************************************************************************/

void PrintPosition (POSITION position, STRING playersName, BOOLEAN usersTurn)
{
	printf("%s\n", stringifyGoPosition(unhashPosition(position)));
	printf("  %s\n", GetPrediction(position,playersName,usersTurn));
}

/************************************************************************
**
** NAME:        PrintComputersMove
**
** DESCRIPTION: Nicely formats the computers move.
**
** INPUTS:      MOVE    computersMove : The computer's move.
**              STRING  computersName : The computer's name.
**
************************************************************************/

void PrintComputersMove (MOVE computersMove, STRING computersName)
{

}


/************************************************************************
**
** NAME:        PrintMove
**
** DESCRIPTION: Prints the move in a nice format.
**
** INPUTS:      MOVE move         : The move to print.
**
************************************************************************/

void PrintMove (MOVE move)
{
	STRING str = MoveToString( move );
	printf( "%s", str );
	SafeFree( str );
}


/************************************************************************
**
** NAME:        MoveToString
**
** DESCRIPTION: Returns the move as a STRING
**
** INPUTS:      MOVE *move         : The move to put into a string.
**
************************************************************************/

STRING MoveToString (MOVE move)
{
	char* buf = SafeMalloc(3 * sizeof(char));

	GoMove my_move = unhashMove(move);
	buf[0] = my_move->x + 'A';
	buf[1] = my_move->y + '1';
	buf[2] = '\0';
	delGoMove(my_move);

	return buf;
}


/************************************************************************
**
** NAME:        GetAndPrintPlayersMove
**
** DESCRIPTION: Finds out if the player wishes to undo, abort, or use
**              some other gamesman option. The gamesman core does
**              most of the work here.
**
** INPUTS:      POSITION position    : Current position
**              MOVE     *move       : The move to fill with user's move.
**              STRING   playersName : Current Player's Name
**
** OUTPUTS:     USERINPUT          : One of
**                                   (Undo, Abort, Continue)
**
** CALLS:       USERINPUT HandleDefaultTextInput(POSITION, MOVE*, STRING)
**                                 : Gamesman Core Input Handling
**
************************************************************************/

USERINPUT GetAndPrintPlayersMove (POSITION position, MOVE *move, STRING playersName)
{
	USERINPUT input;
	USERINPUT HandleDefaultTextInput();

	for (;; ) {
		/***********************************************************
		* CHANGE THE LINE BELOW TO MATCH YOUR MOVE FORMAT
		***********************************************************/
		/* FIXME:  Adjust for variable-size board */
		printf("%8s's move [a-c][1-3]: ", playersName);

		input = HandleDefaultTextInput(position, move, playersName);

		if (input != Continue)
			return input;
	}

	/* NOTREACHED */
	return Continue;
}


/************************************************************************
**
** NAME:        ValidTextInput
**
** DESCRIPTION: Rudimentary check to check if input is in the move form
**              you are expecting. Does not check if it is a valid move.
**              Only checks if it fits the move form.
**
**              Reserved Input Characters - DO NOT USE THESE ONE CHARACTER
**                                          COMMANDS IN YOUR GAME
**              ?, s, u, r, h, a, c, q
**                                          However, something like a3
**                                          is okay.
**
**              Example: Tic-tac-toe Move Format : Integer from 1 to 9
**                       Only integers between 1 to 9 are accepted
**                       regardless of board position.
**                       Moves will be checked by the core.
**
** INPUTS:      STRING input : The string input the user typed.
**
** OUTPUTS:     BOOLEAN      : TRUE if the input is a valid text input.
**
************************************************************************/

BOOLEAN ValidTextInput (STRING input)
{
#if 0
	return (tolower(input[0]) >= 'a' && tolower(input[0]) <= 'c'
	        && input[1] >= 1 && input[1] <= 3);
#else
	return TRUE;
#endif
}


/************************************************************************
**
** NAME:        ConvertTextInputToMove
**
** DESCRIPTION: Converts the string input your internal move representation.
**              Gamesman already checked the move with ValidTextInput
**              and ValidMove.
**
** INPUTS:      STRING input : The VALID string input from the user.
**
** OUTPUTS:     MOVE         : Move converted from user input.
**
************************************************************************/

MOVE ConvertTextInputToMove (STRING input)
{
	/* FIXME:  Make smarter */
	GoMove my_move = newGoMove(tolower(input[0]) - 'a' + 1, input[1] - '1' + 1);
	MOVE result = hashMove(my_move);
	delGoMove(my_move);
	return result;
}


/************************************************************************
**
** NAME:        GameSpecificMenu
**
** DESCRIPTION: Prints, receives, and sets game-specific parameters.
**
**              Examples
**              Board Size, Board Type
**
**              If kGameSpecificMenu == FALSE
**                   Gamesman will not enable GameSpecificMenu
**                   Gamesman will not call this function
**
**              Resets gNumberOfPositions if necessary
**
************************************************************************/

void GameSpecificMenu ()
{
	InitializeHelpStrings();
}


/************************************************************************
**
** NAME:        SetTclCGameSpecificOptions
**
** DESCRIPTION: Set the C game-specific options (called from Tcl)
**              Ignore if you don't care about Tcl for now.
**
************************************************************************/

void SetTclCGameSpecificOptions (int options[])
{

}


/************************************************************************
**
** NAME:        GetInitialPosition
**
** DESCRIPTION: Called when the user wishes to change the initial
**              position. Asks the user for an initial position.
**              Sets new user defined gInitialPosition and resets
**              gNumberOfPositions if necessary
**
** OUTPUTS:     POSITION : New Initial Position
**
************************************************************************/

POSITION GetInitialPosition ()
{
	return 0;
}


/************************************************************************
**
** NAME:        NumberOfOptions
**
** DESCRIPTION: Calculates and returns the number of variants
**              your game supports.
**
** OUTPUTS:     int : Number of Game Variants
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
** DESCRIPTION: A hash function that returns a number corresponding
**              to the current variant of the game.
**              Each set of variants needs to have a different number.
**
** OUTPUTS:     int : the number representation of the options.
**
************************************************************************/

int getOption ()
{
	/* If you have implemented symmetries you should
	   include the boolean variable gSymmetries in your
	   hash */
	return (gStandardGame ? 1 : 2);
}


/************************************************************************
**
** NAME:        setOption
**
** DESCRIPTION: The corresponding unhash function for game variants.
**              Unhashes option and sets the necessary variants.
**
** INPUT:       int option : the number representation of the options.
**
************************************************************************/

void setOption (int option)
{
	/* If you have implemented symmetries you should
	   include the boolean variable gSymmetries in your
	   hash */
	if(option == 1)
		gStandardGame = TRUE;
	else
		gStandardGame = FALSE;
}


/************************************************************************
**
** NAME:        DebugMenu
**
** DESCRIPTION: Game Specific Debug Menu (Gamesman comes with a default
**              debug menu). Menu used to debug internal problems.
**
**              If kDebugMenu == FALSE
**                   Gamesman will not display a debug menu option
**                   Gamesman will not call this function
**
************************************************************************/

void DebugMenu ()
{

}


/************************************************************************
**
** Everything specific to this module goes below these lines.
**
** Things you want down here:
** Move Hasher
** Move Unhasher
** Any other function you deem necessary to help the ones above.
**
************************************************************************/

/*
 * Support functions
 * All of these have internal linkage; these functions manipulate the
 * high-level abstractions.
 */

static POSITION
hashPosition(GoPosition pos) {
	char pos_string[DEFAULT_BOARD_SIZE * DEFAULT_BOARD_SIZE];
	POSITION result;
	int i;
	for (i = 0; i < DEFAULT_BOARD_SIZE * DEFAULT_BOARD_SIZE; ++i)
		pos_string[i] = pos->board->matrix[i]->stone
		                ? (pos->board->matrix[i]->stone->color == STONE_BLACK ? 'X' : 'O')
				: '-';
	result = generic_hash_hash(pos_string, pos->turn == STONE_BLACK ? 1 : 2);
	result <<= 2;
	if (pos->lastMoveCapture) {
		result |= 0x2;
		result |= (pos->lastMoveCapture->color == STONE_WHITE);
	}
	return result;
}

static GoPosition
unhashPosition(POSITION pos) {
	/* FIXME:  This currently leaks memory!  Figure out how to manage it
	 * better... currently, clients call delGoPosition().  There may not
	 * be a better way. */
	GoPosition result = newGoPosition(DEFAULT_BOARD_SIZE);
	char pos_string[DEFAULT_BOARD_SIZE * DEFAULT_BOARD_SIZE];
	if (pos & 0x2) {
		result->lastMoveCapture = newGoStone(pos & 0x1 ? STONE_WHITE : STONE_BLACK);
	}
	pos >>= 2;
	generic_hash_unhash(pos, pos_string);
	int i;
	for (i = 0; i < DEFAULT_BOARD_SIZE * DEFAULT_BOARD_SIZE; ++i)
		result->board->matrix[i]->stone =
		        pos_string[i] == '-'
		        ? NULL
			: (pos_string[i] == 'X'
		           ? newGoStone(STONE_BLACK)
			   : newGoStone(STONE_WHITE));
	result->turn = generic_hash_turn(pos) == 1 ? STONE_BLACK : STONE_WHITE;
	return result;
}

static MOVE
hashMove(GoMove move) {
	return move->x + move->y * 5;
}

static GoMove
unhashMove(MOVE move) {
	size_t y = move / 5 + 1;
	size_t x = move % 5 + 1;

	return newGoMove(x, y);
}

static GoPosition
doMove_imp(GoPosition pos, GoMove move) {
	/* PRE:  move is valid */
	GoIntersection intersection = getGoIntersection(pos->board, move->x, move->y);
	playStone(intersection, pos->turn);
	GoIntersectionNode neighbor;
	for (neighbor = intersection->neighbors->head; neighbor; neighbor = neighbor->next) {
		if (neighbor->data->stone && neighbor->data->stone->color != pos->turn) {       /* FIXME:  More abstraction */
			StoneString string = getStoneString(neighbor->data);
			if (string && countLiberties(string) == 0) {
#ifdef DONE
				int captives = countStones(string);
				addCaptives(pos, pos->turn, captives);
#endif
				pos->lastMoveCapture = string->head->data->stone;
				removeString(string);
			}
		}
	}
	pos->turn = pos->turn == STONE_BLACK ? STONE_WHITE : STONE_BLACK;
	return pos;
}

/**
 * GoGame
 */

/** Creates a new GoGame object.
 * \param boardsize	the size of the board
 * \return			the newly created GoGame
 */
static GoGame
newGoGame(size_t boardsize) {
	GoGame game = SafeMalloc(sizeof(struct GoGame));
	game->position = newGoPosition(boardsize);
	return game;
}

/** Deletes a GoGame object.
 * \param which		the GoGame to delete
 */
static void
delGoGame(GoGame which) {
	delGoPosition(which->position);
	SafeFree(which);
	return;
}

/**
 * GoPosition
 */

/** Creates a new GoPosition object.
 * \param boardsize	the size of the board
 * \return			the newly created GoPosition
 */
static GoPosition
newGoPosition(size_t boardsize) {
	GoPosition pos = SafeMalloc(sizeof(struct GoPosition));
	pos->board = newGoBoard(boardsize);
	pos->turn = STONE_BLACK;
	pos->lastMoveCapture = NULL;
	pos->passedOnce = pos->passedTwice = FALSE;
	return pos;
}

static GoPosition
copyGoPosition(const GoPosition const RHS) {
	GoPosition pos = SafeMalloc(sizeof(struct GoPosition));
	pos->board = copyGoBoard(RHS->board);
	pos->turn = RHS->turn;
	pos->lastMoveCapture = RHS->lastMoveCapture;
	pos->passedOnce = RHS->passedOnce;
	pos->passedTwice = RHS->passedTwice;
	return pos;
}


/** Deletes a GoPosition object.
 * \param which		the GoPosition to delete.
 */
static void
delGoPosition(GoPosition which) {
	delGoBoard(which->board);
	SafeFree(which);
	return;
}

/** Produces a string that represents the position.
 * \param pos		the position to stringify
 * \return			a newly allocated string (caller must free())
 */
static char*
stringifyGoPosition(GoPosition pos) {
#if 0
	STRING str = SafeMalloc(sizeof(char) * 1024);   /* FIXME:  BADBADBAD! */
#else
	static char str[800];
#endif
	int i, j;
	sprintf(str, "         ");
	for (i = 0; i < pos->board->size; ++i)
		sprintf(str + strlen(str), " %c", 'A' + i);
	strcat(str, "\n      ");
	for (j = pos->board->size; j > 0; --j) {
		sprintf(str + strlen(str), " %2d ", j);
		for (i = 0; i < pos->board->size; ++i) {
			GoStone stone = getGoIntersection(pos->board, i, j - 1)->stone;
			strcat(str, stone ? (stone->color == STONE_WHITE ? "O " : "X ") : ". ");
		}
		sprintf(str + strlen(str), "%2d\n      ", j);
	}
	strcat(str, "   ");
	for (i = 0; i < pos->board->size; ++i)
		sprintf(str + strlen(str), " %c", 'A' + i);
	strcat(str, "\n");
	return str;
}

/*
 * GoBoard
 ** Testing.
 */

static GoBoard
newGoBoard(size_t boardsize) {
	GoBoard new_board = SafeMalloc(sizeof(struct GoBoard));
	new_board->size = boardsize;
	new_board->matrix = SafeMalloc(new_board->size * new_board->size * sizeof(GoIntersection));
	/* construct empty intersections */
	int i, j;
	for (j = 0; j < new_board->size; ++j)           /* rows */
		for (i = 0; i < new_board->size; ++i)   /* cols */
			*(new_board->matrix + (j * new_board->size) + i) = newGoIntersectionDefault();
	/* assign neighbors */
	for (j = 0; j < new_board->size; ++j) {
		for (i = 0; i < new_board->size; ++i) {
			if (i > 0)
				insertNeighbor(getGoIntersection(new_board, i, j), getGoIntersection(new_board, i - 1, j));
			if (j > 0)
				insertNeighbor(getGoIntersection(new_board, i, j), getGoIntersection(new_board, i, j - 1));
			if (i < (new_board->size - 1))
				insertNeighbor(getGoIntersection(new_board, i, j), getGoIntersection(new_board, i + 1, j));
			if (j < (new_board->size - 1))
				insertNeighbor(getGoIntersection(new_board, i, j), getGoIntersection(new_board, i, j + 1));
		}
	}
	/* FIXME:  Implement more initializations*/
	return new_board;
}

static GoBoard
copyGoBoard(const GoBoard const RHS) {
	GoBoard new_board = SafeMalloc(sizeof(struct GoBoard));
	new_board->size = RHS->size;
	new_board->matrix = SafeMalloc(new_board->size * new_board->size * sizeof(GoIntersection));
	/* construct empty intersections */
	int i, j;
	for (j = 0; j < new_board->size; ++j)           /* rows */
		for (i = 0; i < new_board->size; ++i)   /* cols */
			*(new_board->matrix + (j * new_board->size) + i) = newGoIntersectionDefault();
	/* assign neighbors */
	for (j = 0; j < new_board->size; ++j) {
		for (i = 0; i < new_board->size; ++i) {
			if (i > 0)
				insertNeighbor(getGoIntersection(new_board, i, j), getGoIntersection(new_board, i - 1, j));
			if (j > 0)
				insertNeighbor(getGoIntersection(new_board, i, j), getGoIntersection(new_board, i, j - 1));
			if (i < (new_board->size - 1))
				insertNeighbor(getGoIntersection(new_board, i, j), getGoIntersection(new_board, i + 1, j));
			if (j < (new_board->size - 1))
				insertNeighbor(getGoIntersection(new_board, i, j), getGoIntersection(new_board, i, j + 1));
		}
	}
	/* fill with stones */
	for (i = 0; i < new_board->size * new_board->size; ++i) {
		GoStone stone = NULL;
		if ((stone = RHS->matrix[i]->stone))
			new_board->matrix[i]->stone = newGoStone(stone->color);
	}
	return new_board;
}

static void
delGoBoard(GoBoard const old_board) {
	SafeFree(old_board->matrix);
	SafeFree(old_board);
	/* FIXME:  Implement */
	return;
}

static GoIntersection
getGoIntersection(const GoBoard const board, size_t const x, size_t const y) {
	GoIntersection result;
	if (x < board->size && y < board->size) {
		result = *(board->matrix + y * board->size + x);
	} else {
		result = NULL;
	}
	return result;
}

static GoMove
newGoMove(size_t x, size_t y) {
	GoMove move = SafeMalloc(sizeof(struct GoMove));
	move->x = x - 1;
	move->y = y - 1;
	return move;
}

static void
delGoMove(GoMove which) {
	SafeFree(which);
	return;
}

static BOOLEAN
isValidMove(GoMove move, GoPosition pos) {
	BOOLEAN result;
	GoIntersection intersection = getGoIntersection(pos->board, move->x, move->y);
	if (!intersection->stone) {
		GoPosition tmp = copyGoPosition(pos);
		tmp = doMove_imp(tmp, move);
		intersection = getGoIntersection(tmp->board, move->x, move->y);
		StoneString string = getStoneString(intersection);
		if (string && countLiberties(string) != 0) {
			result = TRUE;
		} else {
			result = FALSE;
		}
		delGoPosition(tmp);
	} else {
		result = FALSE;
	}
	return result;
}

/**
 * GoGame
 */

/** Creates a new GoGame object.
 * \param boardsize	the size of the board
 * \return			the newly created GoGame
 */
/*
 * GoIntersection
 */

static GoIntersection
newGoIntersectionDefault(void) {
	GoIntersection result = SafeMalloc(sizeof(struct GoIntersection));
	result->stone = NULL;
	result->neighbors = newGoIntersectionListDefault();
	return result;
}

#ifdef DONE
static void
delGoIntersection(GoIntersection which) {
	if (which->neighbors)
		delGoIntersectionList(which->neighbors);
	if (which->stone)
		delGoStone(which->stone);
	SafeFree(which);
	return;
}
#endif

static void
playStone(GoIntersection where, StoneColor color) {
	where->stone = newGoStone(color);
	return;
}

static void
insertNeighbor(GoIntersection target, GoIntersection neighbor) {
	insertGoIntersection(target->neighbors, neighbor);
	return;
}

/* GoIntersectionList */

static GoIntersectionList
newGoIntersectionListDefault(void) {
	GoIntersectionList result = SafeMalloc(sizeof(struct GoIntersectionList));
	result->head = NULL;
	return result;
}

#ifdef DONE
static void
delGoIntersectionList(GoIntersectionList which) {
	GoIntersectionNode node = which->head;
	while (node) {
		GoIntersectionNode tmp = node;
		node = node->next;
		SafeFree(tmp);
	}
	return;
}
#endif

static void
insertGoIntersection(GoIntersectionList list, GoIntersection element) {
	GoIntersectionNode node = SafeMalloc(sizeof(struct GoIntersectionNode));
	node->data = element;
	node->next = list->head;
	list->head = node;
	return;
}

static void
removeGoIntersection(GoIntersectionList list, GoIntersection element) {
/* FIXME:  Reorganize! */
	GoIntersectionNode node = list->head;
	GoIntersectionNode* link = &(list->head);
	while (node) {
		GoIntersectionNode tmp = node;
		node = node->next;
		if (tmp->data == element) {
			*link = tmp->next;
			link = &(tmp->next);
			SafeFree(tmp);
		} else {
			link = &(tmp->next);
		}
	}

	return;
}

static int
findGoIntersection(GoIntersectionList list, GoIntersection element) {
	GoIntersectionNode node;
	int result = 0;
	for (node = list->head; node; node = node->next)
		if (node->data == element)
			result = 1;
	return result;
}

/* StoneString */

static StoneString
getStoneString(const GoIntersection const start) {
	/* FIXME:  Refactor! */
	StoneString result = newGoIntersectionListDefault();
	GoIntersectionList visited = newGoIntersectionListDefault();
	GoIntersectionList check = newGoIntersectionListDefault();

	StoneColor color = start->stone->color;
	insertGoIntersection(check, start);
	GoIntersectionNode node;
	GoIntersectionNode neighbor;

	while ((node = check->head)) {
		if (!findGoIntersection(visited, node->data)) {
			insertGoIntersection(visited, node->data);
			if (node->data->stone && node->data->stone->color == color) {
				insertGoIntersection(result, node->data);
				for (neighbor = node->data->neighbors->head;
				     neighbor; neighbor = neighbor->next) {
					if (!findGoIntersection(visited, neighbor->data) && !findGoIntersection(check, neighbor->data)) {
						insertGoIntersection(check, neighbor->data);
					}
				}
			}
		}
		removeGoIntersection(check, node->data);
	}
	/* FIXME:  Clean up */
	return result;
}

static unsigned int
countLiberties(const StoneString const which) {
	unsigned int liberties = 0;
	GoIntersectionNode node, neighborNode;
	GoIntersectionList counted = newGoIntersectionListDefault();
	for (node = which->head; node; node = node->next) {
		for (neighborNode = node->data->neighbors->head; neighborNode; neighborNode = neighborNode->next) {
			if (!findGoIntersection(counted, neighborNode->data)) {
				insertGoIntersection(counted, neighborNode->data);
				if (neighborNode->data->stone == NULL) {
					++liberties;
				}
			}
		}
	}
	return liberties;
}

static void
removeString(StoneString const which) {
	GoIntersectionNode node;
	for (node = which->head; node; node = node->next) {
		if (node->data->stone) {
			delGoStone(node->data->stone);
			node->data->stone = NULL;
		}
	}
	return;
}

static GoStone
newGoStone(StoneColor color) {
	GoStone result = SafeMalloc(sizeof(struct GoStone));
	result->color = color;
	return result;
}

static void
delGoStone(GoStone which) {
	SafeFree(which);
	return;
}


/************************************************************************
** Changelog
**
** $Log$
** Revision 1.5  2008/02/20 06:13:23  ungu1d3d_s0ul
** changed main.c in core and Tootnotto
**
** Revision 1.4  2006/12/19 20:00:50  arabani
** Added Memwatch (memory debugging library) to gamesman. Use 'make memdebug' to compile with Memwatch
**
** Revision 1.3  2006/12/07 03:20:52  darch
** ValidTestInput was broken; fixed by breaking to a more usefully broken state.
**
** Revision 1.10  2006/04/25 01:33:06  ogren
** Added InitialiseHelpStrings() as an additional function for new game modules to write.  This allows dynamic changing of the help strings for every game without adding more bookkeeping to the core.  -Elmer
**
************************************************************************/

POSITION StringToPosition(char* board) {
	// FIXME: this is just a stub
	return atoi(board);
}


char* PositionToString(POSITION pos) {
	// FIXME: this is just a stub
	return "Implement Me";
}

char * PositionToEndData(POSITION pos) {
	return NULL;
}
