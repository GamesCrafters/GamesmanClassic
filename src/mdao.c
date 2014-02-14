// $Id: mdao.c,v 1.10 2008-05-08 05:13:44 l156steven Exp $

/*
 * The above lines will include the name and log of the last person
 * to commit this file to CVS
 */

/************************************************************************
**
** NAME:        mdao.c
**
** DESCRIPTION: The game of Dao
**
** AUTHOR:      GamesCrafters2005Fa
**
** DATE:        2005-10-05
**
** UPDATE HIST: 1.0 Release
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

STRING kGameName            = "Dao";   /* The name of your game */
STRING kAuthorName          = "GamesCrafters2005Fa";   /* Your name(s) */
STRING kDBName              = "Dao";   /* The name to store the database under */

BOOLEAN kPartizan            = TRUE;   /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN kGameSpecificMenu    = FALSE;   /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN kTieIsPossible       = FALSE;   /* TRUE if a tie is possible. FALSE if it is impossible.*/
BOOLEAN kLoopy               = TRUE;   /* TRUE if the game tree will have cycles (a rearranger style game). FALSE if it does not.*/

BOOLEAN kDebugMenu           = TRUE;   /* TRUE only when debugging. FALSE when on release. */
BOOLEAN kDebugDetermineValue = FALSE;   /* TRUE only when debugging. FALSE when on release. */

POSITION gNumberOfPositions   =  900900; /* The number of total possible positions | If you are using our hash, this is given by the hash_init() function*/
POSITION gInitialPosition     =  189388; /* x--o -xo- -ox- o--x */ /* The initial hashed position for your starting board */
POSITION kBadPosition         = -1; /* A position that will never be used */

void*    gGameSpecificTclInit = NULL;

/*
 * Help strings that are pretty self-explanatory
 * Strings than span more than one line should have backslashes (\) at the end of the line.
 */

STRING kHelpGraphicInterface =
        "Not written yet";

STRING kHelpTextInterface    =
        "";

STRING kHelpOnYourTurn =
        "";

STRING kHelpStandardObjective =
        "";

STRING kHelpReverseObjective =
        "";

STRING kHelpTieOccursWhen =
        "A tie occurs when ...";

STRING kHelpExample =
        "";


/*************************************************************************
**
** #defines and structs
**
**************************************************************************/

#define PLAYER1_PIECE       'X'
#define PLAYER2_PIECE       'O'
#define EMPTY_PIECE         ' '
#define PLAYER1_TURN        1
#define PLAYER2_TURN        2
#define INPUT_PARAM_COUNT1  3
#define INPUT_PARAM_COUNT2  4
#define BOARD_SIZE          (BOARD_ROWS*BOARD_COLS)
#define PLAYER_PIECES       4
#define EMPTY_PIECES        8
#define NUM_OF_DIRS         8
#define BOARD_DIM_MAX       10
#define BOARD_DIM_MIN       3
#define NUM_TO_WIN_MAX      8
#define NUM_TO_WIN_MIN      3
#define ROW_START        'a'

/*************************************************************************
**
** Global Variables
**
*************************************************************************/

int BOARD_ROWS          = 4;
int BOARD_COLS          = 4;
int NUM_TO_WIN          = 4;
BOOLEAN CAN_MOVE_DIAGONALLY = TRUE;
BOOLEAN MISERE              = FALSE;

/*the corresponding direction for output*/
STRING directions[NUM_OF_DIRS] = {
	"sw", "s", "se",
	"w",        "e",
	"nw", "n", "ne"
};

/*char* alpha = "abcdefghij";  used to print row letter */

/*the increments to the row and column numbers of the piece *
 * THESE ARE ACCORIDNG TO THE KEYPAD ARRANGEMENTS
   1  2  3
   4  5  6
   7  8  9
 */
int dir_increments[NUM_OF_DIRS][2] = {
	{ -1,  1 }, { 0, 1 }, { 1, 1 },
	{ -1,  0 },             { 1, 0 },
	{ -1, -1 }, { 0, -1 }, { 1,-1 }
};

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
extern POSITION         generic_hash_init(int boardsize, int pieces_array[], int (*vcfg_function_ptr)(int* cfg), int player);
extern POSITION         generic_hash_hash(char *board, int player);
extern char            *generic_hash_unhash(POSITION hash_number, char *empty_board);
extern int              generic_hash_turn (POSITION hashed);
/*internal*/
void                    InitializeGame();
MOVELIST               *GenerateMoves(POSITION position);
POSITION                DoMove (POSITION position, MOVE move);
VALUE                   Primitive (POSITION position);
void                    PrintPosition(POSITION position, STRING playersName, BOOLEAN usersTurn);
void                    printBoard(char board[]);
void                    PrintComputersMove(MOVE computersMove, STRING computersName);
void                    PrintMove(MOVE move);
STRING                  MoveToString(MOVE);
USERINPUT               GetAndPrintPlayersMove (POSITION position, MOVE *move, STRING playersName);
BOOLEAN                 ValidTextInput(STRING input);
MOVE                    ConvertTextInputToMove(STRING input);
void                    GameSpecificMenu();
void                    SetTclCGameSpecificOptions(int options[]);
POSITION                GetInitialPosition();
BOOLEAN                 GetInitPosHelper (char *board, char playerchar);
int                     NumberOfOptions();
int                     getOption();
void                    setOption(int option);
void                    DebugMenu();
/* helpers */
int                     Index(int row, int col);
int                     Row(int Pos);
int                     Column(int Pos);
MOVE                    Hasher(int row, int col, int dir);
int                     Unhasher_Index(MOVE move);
int                     Unhasher_Direction(MOVE move);
void                    setGameParameters(int row, int col, int num_of_dirs, BOOLEAN diag, BOOLEAN misere);
void                    initializePiecesArray(int p_a[]);
void                    initializeBoard(char board[]);
POSITION                getCanonicalPosition(POSITION p);

POSITION                ActualNumberOfPositions(int variant);

/************************************************************************
**
** NAME:        InitializeGame
**
** DESCRIPTION: Prepares the game for execution.
**              Initializes required variables.
**              Sets up gDatabase (if necessary).
**
************************************************************************/

void InitializeGame ()
{
	int init_pieces[10];
	char board[] = "X  O XO  OX O  X";

	initializePiecesArray(init_pieces);
	gNumberOfPositions = generic_hash_init (BOARD_SIZE, init_pieces, NULL, 0);

	int reflections[] = {0,45,90,135};
	int rotations[] = {90, 180, 270};
	generic_hash_init_sym(0, BOARD_ROWS, BOARD_COLS, reflections, 4, rotations, 3, 1);
	gInitialPosition = generic_hash_hash(board, PLAYER1_TURN);

	gMoveToStringFunPtr = &MoveToString;
	gActualNumberOfPositionsOptFunPtr = &ActualNumberOfPositions;
}


/************************************************************************
**
** NAME:        GenerateMoves
**
** DESCRIPTION: Creates a linked list of every move that can be reached
**              from this position. Returns a pointer to the head of the
**              linked list.
**
** INPUTS:      POSITION position : Current position for move
**                                  generation.
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
	int i, j, k, di, dj;
	char board[BOARD_SIZE+1];
	int player = generic_hash_turn(position);
	BOOLEAN canProcessDir = FALSE;

	generic_hash_unhash (position, board);
	/* Use CreateMovelistNode(move, next) to 'cons' together a linked list */
	for (i = 0; i < BOARD_ROWS; i++) {
		for (j = 0; j < BOARD_COLS; j++) {
			if (((player==1) && (board[Index(i,j)] == PLAYER1_PIECE)) ||
			    ((player==2) && (board[Index(i,j)] == PLAYER2_PIECE)))
			{
				for (k = 0; k < NUM_OF_DIRS; k++) {
					// for every possible movement as specified in dir_increments
					// except 5 and the diagonals when !CAN_MOVE_DIAGONALLY
					// 0 1 2
					// 3   4
					// 5 6 7
					if ((k == 0) || (k == 2) || (k == 5) || (k == 7))
						canProcessDir = CAN_MOVE_DIAGONALLY;
					else
						canProcessDir = TRUE;
					if (canProcessDir) {
						di = i + dir_increments[k][0];
						dj = j + dir_increments[k][1];
						if ((di >= 0) && (dj >= 0) &&
						    (di < BOARD_ROWS) && (dj < BOARD_COLS) &&
						    (board[Index(di,dj)] == EMPTY_PIECE)) {
							moves = CreateMovelistNode(Hasher(i,j,k),moves);
						}
					}
				}
			}
		}
	}
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
	char board[BOARD_SIZE];
	int current_player = generic_hash_turn(position);
	int move_position = Unhasher_Index(move);
	int row = Row (move_position);
	int col = Column (move_position);
	int direction = Unhasher_Direction(move);
	int new_position;

	generic_hash_unhash(position, board);
	do {
		row += dir_increments[direction][0];
		col += dir_increments[direction][1];
	} while ( (row >= 0) && (col >= 0) &&
	          (row < BOARD_ROWS) && (col < BOARD_COLS) &&
	          (board[Index(row,col)] == EMPTY_PIECE));

	new_position = Index(row - dir_increments[direction][0],
	                     col - dir_increments[direction][1]);

	generic_hash_unhash (position, board);
	board[new_position] = board[move_position];
	board[move_position] = (char) EMPTY_PIECE;
	return generic_hash_hash(board, (current_player == PLAYER1_TURN ? PLAYER2_TURN : PLAYER1_TURN));
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
	/* if it's your turn but there's already a win, then you must have lost!
	 * can think about it this way because players are only allowed to move their
	 * own pieces.
	 */
	char board[BOARD_SIZE];
	int r,c,d;

	generic_hash_unhash(position, board);

	for (r = 0; r < BOARD_ROWS; r++) /*horizontal*/
		for (c = 0; c < BOARD_COLS+1-NUM_TO_WIN; c++)
			if (board[Index(r,c)] != EMPTY_PIECE) {
				for (d = 1; d < NUM_TO_WIN; d++)
					if (board[Index(r,c+d-1)] != board[Index(r,c+d)])
						break;
				if (d == NUM_TO_WIN) return (gStandardGame ? lose : win);
			}

	for (r = 0; r < BOARD_ROWS+1-NUM_TO_WIN; r++) /*vertical*/
		for (c = 0; c < BOARD_COLS; c++)
			if (board[Index(r,c)] != EMPTY_PIECE) {
				for (d = 1; d < NUM_TO_WIN; d++)
					if (board[Index(r+d-1,c)] != board[Index(r+d,c)])
						break;
				if (d == NUM_TO_WIN) return (gStandardGame ? lose : win);
			}

	for (r = 0; r < BOARD_ROWS-1; r++) /*square*/
		for (c = 0; c < BOARD_COLS-1; c++)
			if (board[Index(r,c)] != EMPTY_PIECE) {
				if ( (board[Index(r,c)] == board[Index(r,c+1)]) &&
				     (board[Index(r,c)] == board[Index(r+1,c)]) &&
				     (board[Index(r,c)] == board[Index(r+1,c+1)]) )
					return (gStandardGame ? lose : win);
			}

	if ( (board[Index(0,0)] != EMPTY_PIECE) && /* four corners */
	     (board[Index(0,0)] == board[Index(0,BOARD_COLS-1)]) &&
	     (board[Index(0,0)] == board[Index(BOARD_ROWS-1,0)]) &&
	     (board[Index(0,0)] == board[Index(BOARD_ROWS-1,BOARD_COLS-1)]) )
		return (gStandardGame ? lose : win);

	if ( ( (board[Index(0,0)] != EMPTY_PIECE) && /* trapping */
	       (board[Index(1,0)] != EMPTY_PIECE) &&
	       (board[Index(0,1)] != EMPTY_PIECE) &&
	       (board[Index(1,1)] != EMPTY_PIECE) &&
	       (board[Index(0,0)] != board[Index(0,1)]) &&
	       (board[Index(0,0)] != board[Index(1,0)]) &&
	       (board[Index(0,0)] != board[Index(1,1)])) ||

	     ( (board[Index(0,BOARD_COLS-1)] != EMPTY_PIECE) &&
	       (board[Index(1,BOARD_COLS-1)] != EMPTY_PIECE) &&
	       (board[Index(0,BOARD_COLS-2)] != EMPTY_PIECE) &&
	       (board[Index(1,BOARD_COLS-2)] != EMPTY_PIECE) &&
	       (board[Index(0,BOARD_COLS-1)] != board[Index(0,BOARD_COLS-2)]) &&
	       (board[Index(0,BOARD_COLS-1)] != board[Index(1,BOARD_COLS-1)]) &&
	       (board[Index(0,BOARD_COLS-1)] != board[Index(1,BOARD_COLS-2)])) ||

	     ( (board[Index(BOARD_ROWS-1,BOARD_COLS-1)] != EMPTY_PIECE) &&
	       (board[Index(BOARD_ROWS-2,BOARD_COLS-1)] != EMPTY_PIECE) &&
	       (board[Index(BOARD_ROWS-1,BOARD_COLS-2)] != EMPTY_PIECE) &&
	       (board[Index(BOARD_ROWS-2,BOARD_COLS-2)] != EMPTY_PIECE) &&
	       (board[Index(BOARD_ROWS-1,BOARD_COLS-1)] != board[Index(BOARD_ROWS-1,BOARD_COLS-2)]) &&
	       (board[Index(BOARD_ROWS-1,BOARD_COLS-1)] != board[Index(BOARD_ROWS-2,BOARD_COLS-1)]) &&
	       (board[Index(BOARD_ROWS-1,BOARD_COLS-1)] != board[Index(BOARD_ROWS-2,BOARD_COLS-2)])) ||

	     ( (board[Index(BOARD_ROWS-1,0)] != EMPTY_PIECE) &&
	       (board[Index(BOARD_ROWS-2,0)] != EMPTY_PIECE) &&
	       (board[Index(BOARD_ROWS-1,1)] != EMPTY_PIECE) &&
	       (board[Index(BOARD_ROWS-2,1)] != EMPTY_PIECE) &&
	       (board[Index(BOARD_ROWS-1,0)] != board[Index(BOARD_ROWS-1,1)]) &&
	       (board[Index(BOARD_ROWS-1,0)] != board[Index(BOARD_ROWS-2,0)]) &&
	       (board[Index(BOARD_ROWS-1,0)] != board[Index(BOARD_ROWS-2,1)])) )
		return (gStandardGame ? win : lose);

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
	char board[BOARD_SIZE];

	generic_hash_unhash (position, board);
	printf (" The game board as %s sees it:\n", playersName);
	printBoard (board);
}

void printBoard (char *board) {
	int i,j;

	for (i = 0; i < BOARD_ROWS; i++) {
		printf ("  +");
		for (j = 0; j < BOARD_COLS; j++)
			printf ("-+");
		printf ( "\n%d ", BOARD_ROWS-i );
		for (j = 0; j < BOARD_COLS; j++)
			printf ("|%c", board[Index(i,j)]);
		printf("|\n");
	}
	printf("  +");
	for (j = 0; j < BOARD_COLS; j++)
		printf("-+");
	printf("\n   ");
	for (j = 0; j < BOARD_COLS; j++)
		printf("%c ", j+ROW_START); /*start count from 1*/
	printf("\n");
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
	int position = Unhasher_Index(computersMove);
	int direction = Unhasher_Direction(computersMove);

	printf ("%s moves the piece %c%d%s\n", computersName, \
	        Column(position)+ROW_START, BOARD_ROWS-Row (position), \
	        directions[direction]);
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
	printf( "%s", MoveToString(move) );
}

/************************************************************************
**
** NAME:        MoveToString
**
** DESCRIPTION: Returns the move as a STRING
**
** INPUTS:      MOVE *theMove         : The move to put into a string.
**
************************************************************************/

STRING MoveToString (theMove)
MOVE theMove;
{
	STRING move = (STRING) SafeMalloc(5);

	int position = Unhasher_Index(theMove);
	int direction = Unhasher_Direction(theMove);

	sprintf (move, "%c%d%s", Column (position)+ROW_START, BOARD_ROWS-Row (position),
	         directions[direction]);

	return move;
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
	char player_char = (generic_hash_turn(position) == PLAYER1_TURN) ? PLAYER1_PIECE : PLAYER2_PIECE;

	for (;; ) {
		/***********************************************************
		* CHANGE THE LINE BELOW TO MATCH YOUR MOVE FORMAT
		***********************************************************/
		printf("%8s's (%c) move [(undo)/<row><col><dir>] : ", playersName, player_char);

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
	char *dirstr;
	int i;
	if ((strlen(input) != 3) && (strlen(input) != 4))
		return FALSE;
	if ((input[0] < ROW_START) || (input[0] > (ROW_START+BOARD_COLS)) ||
	    (input[1] < '1') || (input[1] > '1'+BOARD_ROWS))
		return FALSE;
	dirstr = input+2; /*dir points at the first character of the direction*/
	for (i = 0; i < NUM_OF_DIRS; i++)
		if (strcmp(dirstr, directions[i]) == 0)
			return TRUE;
	return FALSE;
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
	int row, col, dir;
	char *dirstr;

	col = input[0] - 'a';
	row = BOARD_ROWS - (input[1] - '1') - 1;
	dirstr = input+2;
	for (dir = 0; strcmp(directions[dir], dirstr) != 0; dir++)
		;
	return Hasher (row, col, dir);
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
	return 1;
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
	return 0;
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
void initializePiecesArray(int p_a[]) {
	/* {'X', 4, 4, 'O', 4, 4, '-', 8, 8, -1} */
	/*   0   1  2   3   4  5   6   7  8  9  */
	p_a[0] = PLAYER1_PIECE;
	p_a[3] = PLAYER2_PIECE;
	p_a[6] = EMPTY_PIECE;
	p_a[9] = -1;
	p_a[1] = p_a[2] = p_a[4] = p_a[5] = PLAYER_PIECES;
	p_a[7] = p_a[8] = EMPTY_PIECES;
}

void initializeBoard (char board[]) {
	int x,y;
	char piece;

	for (x = 0; x < BOARD_COLS; x++) {
		for (y = 0; y < BOARD_ROWS; y++) {
			if (y == 0) {
				piece = (x % 2) ? PLAYER2_PIECE : PLAYER1_PIECE;
			} else if (y == BOARD_ROWS-1) {
				piece = (x % 2) ? PLAYER1_PIECE : PLAYER2_PIECE;
			} else {
				piece = EMPTY_PIECE;
			}
			board[Index(y, x)] = (char) piece;
		}
	}
}

int Index(int row, int col) {
	return  (row*BOARD_COLS+col);
}

int Row(int Pos) {
	return (Pos/BOARD_COLS);
}

int Column(int Pos) {
	return (Pos%BOARD_COLS);
}

MOVE Hasher(int row, int col, int dir) {
	return (MOVE) ( ( Index(row, col) )* NUM_OF_DIRS + dir);
}

int Unhasher_Index(MOVE hashed_move) {
	return (hashed_move / NUM_OF_DIRS);
}

int Unhasher_Direction(MOVE hashed_move) {
	return (hashed_move % NUM_OF_DIRS);
}

POSITION ActualNumberOfPositions(int variant) {
	return 1768108;
}

POSITION StringToPosition(char* board) {
	int i, turn;
	char board_char[BOARD_SIZE];
	for(i = 0; i < BOARD_SIZE; i++){
		board_char[i] = board[i];
	}

	int success = GetValue(board, "turn", GetInt, &turn);
    if(success){
    	return generic_hash_hash(board_char, turn);
    } else {
    	return INVALID_POSITION;
    }
}

char* PositionToString(POSITION pos) {
	char board[BOARD_SIZE];
	int current_player = generic_hash_turn(pos);
	generic_hash_unhash (pos, board);
	char * str = SafeMalloc(sizeof(char) * (BOARD_SIZE + 1));
	int i = 0;
	for(i; i < BOARD_SIZE; i++){
		str[i] = board[i];
	}
	str[BOARD_SIZE] = '\0'; 
	return MakeBoardString(str, "turn",  StrFromI(current_player), "");
}

char * PositionToEndData(POSITION pos) {
	return NULL;
}
