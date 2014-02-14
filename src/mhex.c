/************************************************************************
**
** NAME:        mhex.c
**
** DESCRIPTION: Hex
**
** AUTHOR:      Shah Bawany and Jacob Andreas
**
** DATE:        2006-9-18 / 2006-11-29
**
** LAST CHANGE: $Id$
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


/*************************************************************************
**
** Game-specific constants
**
**************************************************************************/

STRING kGameName            = "Hex";
STRING kAuthorName          = "Shah Bawany and Jacob Andreas";
STRING kDBName              = "Hex";

BOOLEAN kPartizan            = TRUE;
BOOLEAN kGameSpecificMenu    = TRUE;
BOOLEAN kTieIsPossible       = FALSE;
BOOLEAN kLoopy               = FALSE;

BOOLEAN kDebugMenu           = FALSE;
BOOLEAN kDebugDetermineValue = FALSE;

POSITION gNumberOfPositions   =  0; /* set in initializeGame */
POSITION gInitialPosition     =  0; /* set in initializeGame */
POSITION kBadPosition         = -1;

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
** #defines and structs
**
**************************************************************************/

#define DEFAULT_BOARDROWS 3
#define DEFAULT_BOARDCOLS 3
#define DEFAULT_BOARDSIZE DEFAULT_BOARDROWS * DEFAULT_BOARDCOLS

#define MAX_BOARDROWS 5
#define MAX_BOARDCOLS 5
#define MAX_BOARDSIZE MAX_BOARDROWS * MAX_BOARDCOLS

// Nash's reccommended board size is 14 (Hein suggests 11). The hash
// currently wraps for all square boards larger than 5x5. On the bright
// side, your grandchildren will be dead before your computer finishes
// solving a 14x14 board, so it's not such a big loss.

#define SWAP_NONE 0
#define SWAP_COLOR 1
#define SWAP_DIRECTION 2
#define SWAP_BOTH 3

#define MAX_SWAPMODE SWAP_BOTH+1
#define MAX_SWAPTURN 9

#define BLACKCHAR 'X'
#define WHITECHAR 'O'
#define BLANKCHAR ' '

#define SWAPMOVE -3

#define WHITEPLAYER 1
#define BLACKPLAYER 2

#define FIRSTPLAYER WHITEPLAYER
#define DEFAULT_HORIZPLAYER WHITEPLAYER

typedef struct INode {

	int index;
	struct INode* next;

} lnode;

/*************************************************************************
**
** Global Variables
**************************************************************************/

int swapmode = SWAP_NONE;
int swapturn = 1;

int boardrows = DEFAULT_BOARDROWS;
int boardcols = DEFAULT_BOARDCOLS;
int boardsize = DEFAULT_BOARDSIZE;
int possize;

/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/

/* External */
#ifndef MEMWATCH
extern GENERIC_PTR      SafeMalloc ();
extern void             SafeFree ();
#endif

//int                   vcfg(int *this_cfg);
void                    InitializeHelpStrings();
MOVELIST*               getValidMoves(STRING board, int index, int* count);
STRING                  MoveToString(MOVE move);
void                    PrintPosition (POSITION position, STRING playersName, BOOLEAN usersTurn);
void                    playSwapMove(char* board);
void                    push(lnode** stack, int index);
int                     pop(lnode** stack);
void                    stackfree(lnode* stack);
BOOLEAN                 inbounds(int r, int c);
void                    modifyBoardSize();
void                    configureSwapRule();
void                    getIntVal(int* target);

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

	if(swapmode == SWAP_DIRECTION || swapmode == SWAP_BOTH)
		possize = boardsize + 1;
	else
		possize = boardsize;

	int i;
	char* initialBoard;
	int pieces[] = {BLANKCHAR, 0, possize, BLACKCHAR, 0, possize, WHITECHAR, 0, possize, -1};

	InitializeHelpStrings();

	gNumberOfPositions = generic_hash_init(possize, pieces, NULL, 0); // ***

	initialBoard = (char*)SafeMalloc((possize)*sizeof(char));

	for(i = 0; i < boardsize; i++)
		initialBoard[i] = BLANKCHAR;

	if(swapmode == SWAP_DIRECTION || swapmode == SWAP_BOTH)
		initialBoard[possize-1] = (DEFAULT_HORIZPLAYER == WHITEPLAYER ? WHITECHAR : BLACKCHAR);

	gInitialPosition = generic_hash_hash(initialBoard, FIRSTPLAYER);

}


/************************************************************************
**
** NAME:        InitializeHelpStrings
**
** DESCRIPTION: Sets up the help strings based on chosen game options.
**
** NOTES:       Should be called whenever the game options are changed.
**              (e.g., InitializeGame()p and GameSpecificMenu())
**
************************************************************************/
void InitializeHelpStrings ()
{

	kHelpGraphicInterface =
	        "";

	kHelpTextInterface =
	        "";

	kHelpOnYourTurn =
	        "Enter a move in the format [letter][number], where [letter] is the column of the cell you want to move into and [number] is its row.";

	kHelpStandardObjective =
	        "To connect your two parallel sides of the board before the other player connects his or her sides.";

	kHelpReverseObjective =  "To force your opponent to connect his or her sides of the board first";

	kHelpTieOccursWhen =
	        "A tie is not possible in the game of Hex.";

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
** CALLS:       MOVELIST getNextBlank
**
************************************************************************/

MOVELIST *GenerateMoves (POSITION position)
{

	MOVELIST* validMoves;
	char* board;
	int blankcount = 0;

	board = (char*)SafeMalloc((possize)*sizeof(char));

	generic_hash_unhash(position, board);

	validMoves = getValidMoves(board, 0, &blankcount);

	if(swapmode != SWAP_NONE && generic_hash_turn(position) != FIRSTPLAYER && blankcount == boardsize-swapturn)
		validMoves = CreateMovelistNode(SWAPMOVE, validMoves);

	SafeFree(board);

	return validMoves;
}


/************************************************************************
**
** NAME:        DoMove
**
** DESCRIPTION: Applies the move to the position.
**
** INPUTS:      POSITION position
   : The old position
**              MOVE     move     : The move to apply to the position
**
** OUTPUTS:     (POSITION)        : The position that results from move
**
** CALLS:       generic_hash_hash, generic_hash_unhash
**
*************************************************************************/

POSITION DoMove (POSITION position, MOVE move)
{

	char* board;
	POSITION newPosition;
	int turn = generic_hash_turn(position);

	board = (char*)SafeMalloc((boardsize)*sizeof(char));
	generic_hash_unhash(position, board);

	if (move == SWAPMOVE) {
		if(swapmode == SWAP_COLOR || swapmode == SWAP_BOTH)
			playSwapMove(board);
		if(swapmode == SWAP_DIRECTION || swapmode == SWAP_BOTH) {
			board[possize-1] = (board[possize-1] == WHITECHAR ? BLACKCHAR : WHITECHAR);
		}

	} else {
		board[move] = ((turn == 1) ? WHITECHAR : BLACKCHAR);
	}

	newPosition = generic_hash_hash(board, (turn % 2)+1);

	SafeFree(board);

	return newPosition;

}


/************************************************************************
**
** NAME:        Primitive
**
** DESCRIPTION: Returns the value of a position if it fulfills certain
**              'primitive' constraints.
*
**              Case                                  Return Value
**              *********************************************************
**              Current player sees a path across board      sees lose
**              Doubling back case (temporary fix)           tie
**              All other cases                              undecided
**
** INPUTS:      POSITION position : The position to inspect.
**
** OUTPUTS:     (VALUE)           : one of
**                                  (win, lose, tie, undecided)
**
** CALLS:       RowMaskBoard, ColMaskBoard
**
************************************************************************/

VALUE Primitive (POSITION position)
{
	char* board;
	char searchchar, horizchar;
	BOOLEAN* visited;
	BOOLEAN search_horiz;
	int i, current, r, c;
	lnode* stack = NULL;

	board = (char*)SafeMalloc((possize)*sizeof(char));
	visited = (BOOLEAN*)SafeMalloc((boardsize)*sizeof(BOOLEAN));

	for(i = 0; i < boardsize; i++) visited[i] = FALSE;

	generic_hash_unhash(position, board);

	if(swapmode == SWAP_DIRECTION || swapmode == SWAP_BOTH)
		horizchar = board[possize-1];
	else
		horizchar = DEFAULT_HORIZPLAYER == WHITEPLAYER ? WHITECHAR : BLACKCHAR;

	if(generic_hash_turn(position) == WHITEPLAYER)
		searchchar = BLACKCHAR;
	else
		searchchar = WHITECHAR;


	if(horizchar == searchchar) {

		search_horiz = TRUE;

		for(i = 0; i < boardrows; i++) {
			if(board[boardcols*i] == searchchar) {
				push(&stack, boardcols*i);
				visited[boardcols*i] = TRUE;
			}
		}

	} else {

		search_horiz = FALSE;

		for(i = 0; i < boardcols; i++) {
			if(board[i] == searchchar) {
				push(&stack, i);
				visited[i] = TRUE;
			}
		}

	}

	while(stack != NULL) {
		current = pop(&stack);
		if((search_horiz && current % boardcols == boardcols - 1 && board[current] == searchchar) ||
		   (!search_horiz && current / boardcols == boardrows - 1 && board[current] == searchchar)) {

			stackfree(stack);
			SafeFree(visited);
			SafeFree(board);
			return gStandardGame ? lose : win;

		} else {

			r = (int)(current / boardcols);
			c = (int)(current % boardcols);


			if(inbounds(r-1, c) && !(visited[c+(r-1)*boardcols]) && board[c+(r-1)*boardcols] == searchchar) {
				push(&stack, c+(r-1)*boardcols);
				visited[c+(r-1)*boardcols] = TRUE;
			}

			if(inbounds(r-1, c+1) && !(visited[(c+1)+(r-1)*boardcols]) && board[(c+1)+(r-1)*boardcols] == searchchar) {
				push(&stack, (c+1) + (r-1)*boardcols);
				visited[(c+1)+(r-1)*boardcols] = TRUE;
			}

			if(inbounds(r, c-1) && !(visited[(c-1) + r*boardcols]) && board[(c-1) + r*boardcols] == searchchar) {
				push(&stack, (c-1) + r*boardcols);
				visited[(c-1) + r*boardcols] = TRUE;
			}

			if(inbounds(r, c+1) && !(visited[(c+1) + r*boardcols]) && board[(c+1) + r*boardcols] == searchchar) {
				push(&stack, (c+1) + r*boardcols);
				visited[(c+1) + r*boardcols] = TRUE;
			}

			if(inbounds(r+1, c-1) && !(visited[(c-1) + (r+1)*boardcols]) && board[(c-1) + (r+1)*boardcols] == searchchar) {
				push(&stack, (c-1) + (r+1)*boardcols);
				visited[(c-1) + (r+1)*boardcols] = TRUE;
			}

			if(inbounds(r+1, c) && !(visited[c+(r+1)*boardcols]) && board[c+(r+1)*boardcols] == searchchar) {
				push(&stack, c + (r+1)*boardcols);
				visited[(c + (r+1)*boardcols)] = TRUE;
			}

		}


	}


	stackfree(stack);
	SafeFree(visited);
	SafeFree(board);

	return undecided;

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

	int n, row, i, col;
	char* board;
	char horizchar;

	board = (char*)SafeMalloc((possize)*sizeof(char));

	generic_hash_unhash(position, board);

	if(swapmode == SWAP_DIRECTION || swapmode == SWAP_BOTH)
		horizchar = board[possize - 1];
	else
		horizchar = (DEFAULT_HORIZPLAYER == WHITEPLAYER ? WHITECHAR : BLACKCHAR);

	printf("\n\n\n\n");

	printf("  %s's turn (%c):\n\n", playersName, generic_hash_turn(position) == 1 ? WHITECHAR : BLACKCHAR);

	printf("  %c: horizontal\n", horizchar);
	printf("  %c: vertical\n\n", horizchar == WHITECHAR ? BLACKCHAR : WHITECHAR);

	printf("    ");

	for(i = 0; i < boardcols; i++) {
		printf("/ \\ ");
	}
	printf("\n");

	for(row = 0; row < boardrows; row++) {
		for(i = 0; i < row; i++)
			printf("  ");
		printf(" %c |", '0'+row);
		for(col = 0; col < boardcols; col++)
			printf(" %c |", board[row*boardcols+col]);
		printf("\n    ");
		for(i = 0; i < row; i++) {
			printf("  ");
		}
		for(i = 0; i < boardcols; i++)
			printf("\\ / ");
		if(row != boardrows-1) printf("\\");
		printf("\n");
	}
	printf("   ");
	for(col = 0; col < boardrows; col++)
		printf("  ");

	printf(" ");
	for(n = 0; n < boardcols; n++)
		printf("%c   ", 'a'+n);

	printf("\n\n");

	printf(GetPrediction(position, playersName, usersTurn));
	printf("\n\n");

	SafeFree(board);

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
	printf("%s's move: %s", computersName, MoveToString(computersMove));
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
	STRING str = (char*)SafeMalloc(20*sizeof(char));
	str = MoveToString( move );

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
	STRING movestring = (char*)SafeMalloc(20*sizeof(char));

	if(move == SWAPMOVE) {

		movestring [0] = 'w';
		movestring [1] = '\0';

	} else {

		movestring[0] = 'a' + (int)(move % boardcols);
		movestring[1] = '0' + (int)(move / boardcols);

		movestring[2] = '\0';

	}

	return movestring;
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

		printf("%8s's move [ `undo' | {column}{row} ] : ", playersName);

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

BOOLEAN ValidTextInput (STRING input) {

	if(strlen(input) < 1) {
		return FALSE;
	} else if(swapmode != SWAP_NONE && input[0] == 'w') {
		return TRUE;
	} else if((input[0] < 'a') || (input[0] > boardrows+'a')) {
		return FALSE;
	} else if((input[1] < '0') || (input[1] > boardrows+'0')) {
		return FALSE;
	} else {
		return TRUE;
	}
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
	int output;

	if(input[0] == 'w')
		output = SWAPMOVE;
	else
		output = (int)(input[0]-'a')+(int)(input[1]-'0')*boardcols;


	return output;

}


/************************************************************************
**
** NAME:        GameSpecificMenu
**
** DESCRIPTION: Prints, receives, and sets game-specific parameters.
***
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
	char command = 'Z';

	do {

		printf("\n\t----- Game-specific options for Hex -----\n\n");
		printf("\td)\tChange board (D)imensions (currently %dx%d)\n", boardrows, boardcols);
		printf("\ts)\tConfigure (S)waprule\n\n");
		printf("\tb)\t(B)ack to previous screen\n\n");
		printf("Please select an option: ");

		command = toupper(GetMyChar());

		switch(command) {
		case 'D':
			modifyBoardSize();
			command = 'Z';
			break;

		case 'S':
			configureSwapRule();
			command = 'Z';
			break;

		case 'B':
			return;

		default:
			printf("\nI don't understand %c - please choose another option.", command);
			command = 'Z';

		}

	} while (command != 'B');

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
	return MAX_BOARDROWS * MAX_BOARDCOLS * MAX_SWAPMODE * MAX_SWAPTURN * 2;
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

	return (((boardrows * MAX_BOARDCOLS + boardcols) * MAX_SWAPMODE + swapmode) * MAX_SWAPTURN + swapturn) * 2 + gStandardGame;
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

	gStandardGame = option % 2;
	option /= 2;

	swapturn = option % MAX_SWAPTURN;
	option /= MAX_SWAPTURN;

	swapmode = option % MAX_SWAPMODE;
	option /= MAX_SWAPMODE;

	boardcols = option % MAX_BOARDCOLS;
	boardrows = option / MAX_BOARDCOLS;

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


int vcfg(int *this_cfg) {
	return this_cfg[1] == this_cfg[2] || this_cfg[1] == this_cfg[2] + 1;
}



MOVELIST* getValidMoves(STRING board, int index, int* count) {
	MOVELIST* validMoves;

	while((board[index] != BLANKCHAR) && (index < boardsize)) {
		index++;
	}
	if(index==boardsize) {
		validMoves = NULL;
	} else {
		*count = *count + 1;
		validMoves = CreateMovelistNode(index, getValidMoves(board, index+1, count));
	}
	return validMoves;
}


void playSwapMove(char* board) {

	int i;

	for(i = 0; i < boardsize; i++) {

		if(board[i] != BLANKCHAR)
			board[i] = (board[i] == WHITECHAR ? BLACKCHAR : WHITECHAR);

	}

}


void push(lnode** stack, int nindex) {

	lnode* tmp = SafeMalloc(sizeof(*tmp));
	tmp->index = nindex;
	tmp->next = *stack;
	*stack = tmp;

}

int pop(lnode** stack) {

	int reti = (*stack)->index;
	lnode* next = (*stack)->next;
	SafeFree(*stack);
	*stack = next;

	return reti;

}

void stackfree(lnode* stack) {

	lnode* tmp;

	while(stack != NULL) {
		tmp = stack->next;
		SafeFree(stack);
		stack=tmp;
	}
}

BOOLEAN inbounds(int row, int col) {
	return row < boardrows && row >= 0 && col < boardcols && col >= 0;
}

void modifyBoardSize() {
	char command = 'Z';
	do {
		printf("\n\t----- Editing board -----\n\n");
		printf("\tr)\tChange (R)ows (currently %d)\n", boardrows);
		printf("\tc)\tChange (C)olumns (currently %d)\n\n", boardcols);
		printf("\tb)\tGo (B)ack to previous screen\n\n");
		printf("Please select an option: ");

		command = toupper(GetMyChar());

		switch (command) {

		case 'R':
			getIntVal(&boardrows);
			command = 'Z';
			break;

		case 'C':
			getIntVal(&boardcols);
			command = 'Z';
			break;

		case 'B':
			return;

		default:
			printf("\nI don't understand %c - please choose another option.\n", command);
			command = 'Z';

		}

	} while (1);

}

char* swapmodeToString(int mode) {
	if(mode == SWAP_BOTH) return "direction and color";
	else if(mode == SWAP_DIRECTION) return "direction";
	else if(mode == SWAP_COLOR) return "color";
	else if(mode == SWAP_NONE) return "never";
	return "?";
}

void configureSwapRule() {
	char command = 'Z';
	do {
		printf("\n\t----- Configuring Swaprule -----\n\n");

		printf("\tSwapmode is currently \"Swap %s\"\n\n", swapmodeToString(swapmode));

		printf("\tt)\tChange (T)urn when swap is offered (currently %d)\n\n", swapturn+1);

		if(swapmode != SWAP_BOTH) printf("\ta)\tChange mode to \"Swap direction (A)nd color\"\n");
		if(swapmode != SWAP_DIRECTION) printf("\td)\tChange mode to \"Swap (D)irection only\"\n");
		if(swapmode != SWAP_COLOR) printf("\tc)\tChange mode to \"Swap (C)olor only\"\n");
		if(swapmode != SWAP_NONE) printf("\tn)\tChange mode to \"No swaprule\"\n\n");

		printf("\tb)\tGo (B)ack to previous screen\n\n");

		printf("Please select an option: ");

		command = toupper(GetMyChar());

		switch(command) {

		case 'T':
			getIntVal(&swapturn);
			swapturn--;
			command = 'Z';
			break;

		case 'A':
			swapmode = SWAP_BOTH;
			command = 'Z';
			break;

		case 'D':
			swapmode = SWAP_DIRECTION;
			command = 'Z';
			break;

		case 'C':
			swapmode = SWAP_COLOR;
			command = 'Z';
			break;

		case 'B':
			return;

		default:
			printf("\nI don't understand %c - please choose another option.\n", command);
			command = 'Z';
			break;

		}

	} while(1);

}


void getIntVal(int* target) {
	printf("\nEnter a new value: ");
	*target = GetMyInt();
	boardsize = boardrows * boardcols;

}




/************************************************************************
** Changelog
**
** $Log$
** Revision 1.8  2007/03/25 15:30:55  and-qso
** Removed buggy bitwise primitive, replaced it with DFS that should work for all boards.
** Swapmove implemented, but incorrectly: will be fixed in a later version.
**
** Revision 1.7  2007/02/28 23:13:40  and-qso
** Fixed player direction representation from visual to text-based.
**
** Revision 1.1.2.9  2007/02/18 08:32:41  hevanm
** Changes from mainline.
**
** Revision 1.6  2006/12/19 20:00:50  arabani
** Added Memwatch (memory debugging library) to gamesman. Use 'make memdebug' to compile with Memwatch
**
** Revision 1.5  2006/12/07 02:25:07  shahbawany
**
** Working version - boardcols = 3
**
** Can solve 3x3, can play player v. player 4x4
**
** Revision 1.4  2006/11/28 00:47:04  and-qso
**
**
** Changed vcfg pointer to NULL in initializeGame;
**
** Revision 1.3  2006/11/15 20:49:36  and-qso
**
**
** Fixed character reading, cleaned up move handling a bit.
** Generic hash appears to be unhashing incorrectly.
**
** Revision 1.2  2006/11/02 02:46:32  brianzimmer
** Fixed seg fault related to generic_hash_init
**
** Revision 1.1  2006/10/22 22:26:45  and-qso
** *** empty log message ***
**
** Revision 1.10  2006/04/25 01:33:06  ogren
** Added InitialiseHelpStrings() as an additional function for new game modules to write.  This allows dynamic changing of the help strings for every game without adding more bookkeeping to the core.  -Elmer
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
