// $id$
// $log$

/*
 * The above lines will include the name and log of the last person
 * to commit this file to CVS
 */

/************************************************************************
**
** NAME:        mkono.c
**
** DESCRIPTION: (Four Field) Kono
**
** AUTHOR:      Greg Bonin, Nathan Spindel
**
** DATE:        09/29/04
**
** UPDATE HIST: 09/29/04 - Initial Commit: DoMove, PrintPosition, Primitive,
**                         helper functions, board representation.
**
**              10/07/04 - Fixed up everything. Game runs!
**                         To-do: ValidTextInput and game specific options.
**
**              10/08/04 - Changed it so the user input is 1-16 instead of 0-15.
**                         Added help strings. Added LEGEND and BOARD to printPosition.
**
**              10/11/04 - Fixed i/o formatting a bit, added predictions to PrintPosition.
**
**              12/09/04 - Fixed misere to use gStandard instead of our own bool.
**                         Added [x, y] to PrintComputersMove.
**                         Fixed little bug in GenerateMoves that was generating bogus moves.
**                         Fixed bug in numberOfPieces so that it now works.
**                         Tweaked the hash to be slightly more efficient.
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

STRING kGameName            = "(Four Field) Kono";   /* The name of your game */
STRING kAuthorName          = "Greg Bonin and Nathan Spindel";     /* Your name(s) */
STRING kDBName              = "kono";   /* The name to store the database under */

BOOLEAN kPartizan            = TRUE;   /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN kGameSpecificMenu    = TRUE;   /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN kTieIsPossible       = FALSE;   /* TRUE if a tie is possible. FALSE if it is impossible.*/
BOOLEAN kLoopy               = TRUE;   /* TRUE if the game tree will have cycles (a rearranger style game). FALSE if it does not.*/

BOOLEAN kDebugMenu           = FALSE;   /* TRUE only when debugging. FALSE when on release. */
BOOLEAN kDebugDetermineValue = FALSE;   /* TRUE only when debugging. FALSE when on release. */

POSITION gNumberOfPositions   =  0; /* The number of total possible positions | If you are using our hash, this is given by the hash_init() function*/
POSITION gInitialPosition     =  0; /* The initial hashed position for your starting board */
POSITION kBadPosition         = -1; /* A position that will never be used */
void*    gGameSpecificTclInit = NULL;

/*
 * Help strings that are pretty self-explanatory
 * Strings than span more than one line should have backslashes (\) at the end of the line.
 */

STRING kHelpGraphicInterface =
        "Not written yet";

STRING kHelpTextInterface    =
        "On your turn, use the LEGEND to determine which number to choose to move your\n\
piece from, a space character, and a second number to where you want the piece\n\
to move to, and hit return. If at any point you have made a mistake, you can\n\
 type u and hit return and the system will revert back to your most recent position."                                                                                                                                                                                                                                                            ;

STRING kHelpOnYourTurn =
        "If a capture is available, you must make a capture move by moving one of your\n\
pieces (up/down/left/right) over another one of your pieces and capturing the\n\
opponent piece two spaces away. If a capture move is not available, you move\n\
one of your pieces to an adjacent space."                                                                                                                                                                                                                                                           ;

STRING kHelpStandardObjective =
        "To capture all but one of your opponent's pieces, or to put them into a position\n\
where they cannot move."                                                                                             ;

STRING kHelpReverseObjective =
        "To have all but one of your pieces captured first, or to be put in a position where\n\
you cannot move."                                                                                                ;

STRING kHelpTieOccursWhen =
        "Both players have exactly one piece each, or both players cannot move.";

STRING kHelpExample =
        "";


/*************************************************************************
**
** #defines and structs
**
**************************************************************************/

#define EMPTY  0
#define BLACK_PLAYER 1
#define WHITE_PLAYER 2

/*************************************************************************
**
** Global Variables
**
*************************************************************************/

int WIDTH = 4, HEIGHT = 4;
int BOARDSIZE;

BOOLEAN DEFAULT = TRUE;
BOOLEAN gAllowVWrap = FALSE, gAllowHWrap = FALSE;
BOOLEAN gMustCapture = TRUE;

char *gBoard;

/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/

/* Internal */
int exponent(int base, int exponent);
void printRowOfBars();
int numberOfPieces(POSITION board, int player);
int oppositePlayer(int player);
int getSourceFromMove(MOVE move);
int getDestFromMove(MOVE move);
MOVE makeMove(int source, int dest);
int neighbors(int x, int y);

STRING MoveToString(MOVE);

/* External */
#ifndef MEMWATCH
extern GENERIC_PTR      SafeMalloc ();
extern void             SafeFree ();
#endif

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
	int i;
	BOARDSIZE = WIDTH*HEIGHT;

	gBoard = (char *) SafeMalloc (BOARDSIZE * sizeof(char));

	int piece_array[] = {'x', 1, BOARDSIZE/2,
		             'o', 1, BOARDSIZE/2,
		             ' ', 0, BOARDSIZE-3,
		             -1};

	gNumberOfPositions = generic_hash_init(BOARDSIZE, piece_array, NULL, 0);

	for (i = 0; i < BOARDSIZE/2; i++)
		gBoard[i] = 'x';

	if (BOARDSIZE % 2 != 0) {
		gBoard[i] = ' ';
		i++;
	}

	for (; i < BOARDSIZE; i++) {
		gBoard[i] = 'o';
	}

	gInitialPosition = generic_hash_hash(gBoard, 1);

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
** INPUTS:      POSITION position : Current position for move
**                                  generation.
**
** OUTPUTS:     (MOVELIST *)      : A pointer to the first item of
**                                  the linked list of generated moves
**
** CALLS:       MOVELIST *CreateMovelistNode();
**
************************************************************************/

#define upOne(x, y)    (((y + HEIGHT - 1) % HEIGHT) * WIDTH + x)
#define upTwo(x, y)    (((y + HEIGHT - 2) % HEIGHT) * WIDTH + x)
#define downOne(x, y)  (((y + HEIGHT + 1) % HEIGHT) * WIDTH + x)
#define downTwo(x, y)  (((y + HEIGHT + 2) % HEIGHT) * WIDTH + x)

#define leftOne(x, y)  ((y * WIDTH) + ((x + WIDTH - 1) % WIDTH))
#define leftTwo(x, y)  ((y * WIDTH) + ((x + WIDTH - 2) % WIDTH))
#define rightOne(x, y) ((y * WIDTH) + ((x + WIDTH + 1) % WIDTH))
#define rightTwo(x, y) ((y * WIDTH) + ((x + WIDTH + 2) % WIDTH))

MOVELIST *GenerateMoves (POSITION position)
{
	MOVELIST *moves = NULL, *captures = NULL;
	int player = generic_hash_turn (position);
	int i, j, pos;
	char currentPlayer, oppPlayer;

	generic_hash_unhash(position, gBoard);

	if (player == 1) {
		currentPlayer = 'x';
		oppPlayer = 'o';
	} else {
		currentPlayer = 'o';
		oppPlayer = 'x';
	}

	for (i = 0; i < WIDTH; i++) {
		for (j = 0; j < HEIGHT; j++) {

			pos = j*WIDTH+i;

			if (gBoard[pos] == currentPlayer) {

				/* check up one */
				if (pos >= WIDTH || gAllowVWrap) // if pos is not first row OR if vertical wrap is on
					if (gBoard[upOne(i, j)] == ' ') // if up one is empty
						moves = CreateMovelistNode(makeMove(pos, upOne(i, j)), moves);

				/* check up two */
				if (pos >= WIDTH*2 || gAllowVWrap)
					if (gBoard[upOne(i, j)] == currentPlayer && gBoard[upTwo(i, j)] == oppPlayer) {
						if (gMustCapture)
							captures = CreateMovelistNode(makeMove(pos, upTwo(i, j)), captures);
						else moves = CreateMovelistNode(makeMove(pos, upTwo(i, j)), moves);
					}
				/* check down one */
				if (pos < WIDTH*(HEIGHT-1) || gAllowVWrap)
					if (gBoard[downOne(i, j)] == ' ')
						moves = CreateMovelistNode(makeMove(pos, downOne(i, j)), moves);

				/* check down two */
				if (pos < WIDTH*(HEIGHT-2) || gAllowVWrap)
					if (gBoard[downOne(i, j)] == currentPlayer && gBoard[downTwo(i, j)] == oppPlayer) {
						if (gMustCapture)
							captures = CreateMovelistNode(makeMove(pos, downTwo(i, j)), captures);
						else moves = CreateMovelistNode(makeMove(pos, downTwo(i, j)), moves);
					}
				/* check left one */
				if (pos % WIDTH != 0 || gAllowHWrap)
					if (gBoard[leftOne(i, j)] == ' ')
						moves = CreateMovelistNode(makeMove(pos, leftOne(i, j)), moves);

				/* check left two */
				if (pos % WIDTH > 1 || gAllowHWrap)
					if (gBoard[leftOne(i, j)] == currentPlayer && gBoard[leftTwo(i, j)] == oppPlayer) {
						if (gMustCapture)
							captures = CreateMovelistNode(makeMove(pos, leftTwo(i, j)), captures);
						else moves = CreateMovelistNode(makeMove(pos, leftTwo(i, j)), moves);
					}
				/* check right one */
				if (pos % WIDTH != WIDTH-1 || gAllowHWrap)
					if (gBoard[rightOne(i, j)] == ' ')
						moves = CreateMovelistNode(makeMove(pos, rightOne(i, j)), moves);

				/* check right two */
				if (pos % WIDTH < WIDTH-2 || gAllowHWrap)
					if (gBoard[rightOne(i, j)] == currentPlayer && gBoard[rightTwo(i, j)] == oppPlayer) {
						if (gMustCapture)
							captures = CreateMovelistNode(makeMove(pos, rightTwo(i, j)), captures);
						else moves = CreateMovelistNode(makeMove(pos, rightTwo(i, j)), moves);
					}
			}
		}
	}

	/* if there are capture moves, return those. otherwise, return regular moves */
	if (captures == NULL)
		return moves;
	else
		return captures;
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
	int player = generic_hash_turn(position);

	generic_hash_unhash(position, gBoard);

	gBoard[getSourceFromMove(move)] = ' ';

	gBoard[getDestFromMove(move)] = (player == BLACK_PLAYER) ? 'x' : 'o';

	return generic_hash_hash(gBoard, oppositePlayer(player));
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
	int player = generic_hash_turn(position);

	MOVELIST * head = GenerateMoves(position);
	BOOLEAN noMoves = (head == NULL);
	free(head);

	if ((numberOfPieces(position, player) == 1) || noMoves)
		return (gStandardGame ? lose : win);
	else
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
	int i, j, k;

	generic_hash_unhash(position, gBoard);

	/* top row */
	printf("\n\t ");
	for (k = 0; k < WIDTH/2; k++) printf(" ");
	printf("LEGEND");
	for (k = 2; k < WIDTH; k++) printf("   ");
	for (k = 0; k < WIDTH*1.5; k++) printf(" ");
	printf("\tBOARD\n");
	printRowOfBars();


	for (i = 0; i < HEIGHT; i++) {
		/* print the legend */
		printf("\t");

		for (k = 1; k < WIDTH+1; k++) {
			if (WIDTH*i+k < 10) printf(" ");
			printf("%d ", WIDTH*i+k);
		}

		printf("\t| ");

		/* print a piece and a -- */
		for (j = 0; j < WIDTH-1; j++)
			printf(" %c --", gBoard[i*WIDTH + j]);

		/* print last piece in the  row and a | and a newline */
		printf(" %c  |\n", gBoard[i*WIDTH + j]);

		/* print the | rows in between */
		if (i != HEIGHT-1) {
			printf("\t");
			for (k = 0; k < WIDTH; k++) printf("   ");
			printf("\t|");
			for (k = 0; k < WIDTH; k++) printf("  |  ");
			printf("|\n");
		}
	}

	/* last two rows */
	printf("\t");
	for (k = 0; k < WIDTH; k++) printf("   ");
	printf("\t|");
	for (k = 0; k < WIDTH; k++) printf("     ");
	printf("|\n");
	printRowOfBars();

	printf("\n\tIt is %s's (%s) turn.\n", playersName, (generic_hash_turn(position) == BLACK_PLAYER) ? "x" : "o");
	printf("\t%s\n\n", GetPrediction(position,playersName,usersTurn));
}

void printRowOfBars() {
	int i;

	printf("\t");
	for (i = 0; i < WIDTH; i++) printf("   ");
	printf("\t-");
	for (i = 0; i < WIDTH; i++) printf("-----");
	printf("-\n");
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
	int dest = getDestFromMove(computersMove)+1;
	int source = getSourceFromMove(computersMove)+1;

	if (neighbors(source-1, dest-1))
		printf("[%d %d] : %s moved the piece at %d to %d.\n\n",
		       source, dest, computersName, source, dest);
	else
		printf("[%d %d] : %s captured the piece at %d with the piece from %d.\n\n",
		       source, dest, computersName, dest, source);
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
	STRING m = MoveToString( move );
	printf( "%s", m );
	SafeFree( m );
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
	STRING move = (STRING) SafeMalloc(8);
	sprintf(move, "[%d %d]", getSourceFromMove(theMove)+1, getDestFromMove(theMove)+1);
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

	for (;; ) {
		printf("%8s's move [(u)ndo/1-%d 1-%d] : ", playersName, BOARDSIZE, BOARDSIZE);

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
	/* move format: "xx yy" where xx is source and yy is dest (board is 10 to 99 squares)
	 *              "xxx yyy" is board has 100 to 999 squares */

	int i, spaceCount = 0;

	for (i = 0; i < strlen(input); i++) {
		if (input[i] == ' ')
			spaceCount++;
	}

	return ((spaceCount == 1) ? TRUE : FALSE);
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
	int i, j = 0, k = 0, stringPos, source = 0, dest = 0;

	for (i = 0; input[i] != ' '; i++) {}

	for (j = i-1; j >= 0; j--)
		source += ((input[j]-'0') * exponent(10, k++));

	i++;
	k = 0;
	stringPos = i;

	for (; i < strlen(input); i++) {}

	for (j = i-1; j >= stringPos; j--)
		dest += ((input[j]-'0') * exponent(10, k++));

	return makeMove(source-1, dest-1);
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

	while (TRUE) {
		printf("\n\t----- Game-specific options for %s -----\n", kGameName);
		printf("\n\tBoard Options:\n\n");
		printf("\td)\tChange board (D)imension (%d,%d)\n", WIDTH, HEIGHT);
		printf("\n\tRule Options:\n\n");
		printf("\tc)\t%s (C)apture\n", gMustCapture ? "Forced" : "Optional");
		printf("\tv)\t(V)ertical Wrapping: %s\n", gAllowVWrap ? "On" : "Off");
		printf("\to)\tH(o)rizontal Wrapping: %s\n", gAllowHWrap ? "On" : "Off");
		printf("\n\n\tb)\t(B)ack = Return to previous activity.\n");
		printf("\n\nSelect an option: ");

		switch(GetMyChar()) {
		case 'Q': case 'q':
			ExitStageRight();
		case 'H': case 'h':
			HelpMenus();
			break;
		case 'D': case 'd':
			printf("\nPlease enter a new width (between 1 and 6, inclusive): ");
			scanf("%d", &WIDTH);
			while (WIDTH < 1 || WIDTH > 6) {
				printf("Please enter a new width (between 1 and 6, inclusive): ");
				scanf("%d", &WIDTH);
			}

			printf("Please enter a new height (between 1 and 6, inclusive): ");
			scanf("%d", &HEIGHT);
			while (HEIGHT < 1 || HEIGHT > 6) {
				printf("Please enter a new height (between 1 and 6, inclusive): ");
				scanf("%d", &HEIGHT);
			}

			BOARDSIZE = WIDTH*HEIGHT;
			break;
		case 'V': case 'v':
			gAllowVWrap = !gAllowVWrap;
			break;
		case 'O': case 'o':
			gAllowHWrap = !gAllowHWrap;
			break;
		case 'C': case 'c':
			gMustCapture = !gMustCapture;
			break;
		case 'B': case 'b':
			return;
		default:
			BadMenuChoice();
			HitAnyKeyToContinue();
		}
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
	return 6 /* max width */
	       * 6 /* max height */
	       * 2 /* gStandardGame */
	       * 2 /* gAllowVWrap */
	       * 2 /* gAllowHWrap */
	       * 2; /* gMustCapture */
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
	return WIDTH
	       + (6 + HEIGHT)
	       + (12 + (gStandardGame ? 2 : 1))
	       + (14 + (gAllowVWrap ? 2 : 1))
	       + (16 + (gAllowHWrap ? 2 : 1))
	       + (18 + (gMustCapture ? 2 : 1));
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
	gMustCapture = (option % 18 == 2);
	option -= 2;

	gAllowHWrap = (option % 16 == 2);
	option -= 2;

	gAllowVWrap = (option % 14 == 2);
	option -= 2;

	gStandardGame = (option % 12 == 2);
	option -= 2;

	HEIGHT = option % 5;
	option -= 6;

	WIDTH = option;
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

int oppositePlayer(int player) {
	return ((player == BLACK_PLAYER) ? WHITE_PLAYER : BLACK_PLAYER);
}

int numberOfPieces(POSITION position, int player) {
	int pieceCount = 0, i;
	char currentPlayer;

	generic_hash_unhash(position, gBoard);

	currentPlayer = (player == 1) ? 'x' : 'o';

	for (i = 0; i < BOARDSIZE; i++)
		if (gBoard[i] == currentPlayer)
			pieceCount++;

	return pieceCount;
}

int exponent(int base, int exponent) {
	int x;

	for (x = 1; exponent > 0; exponent--)
		x *= base;

	return x;
}

int getSourceFromMove(MOVE move) {
	return move / 1000;
}

int getDestFromMove(MOVE move) {
	return move % 1000;
}

MOVE makeMove(int source, int dest) {
	return (source*1000)+dest;
}

int neighbors(int x, int y) {
	if (x - 1 == y || x + 1 == y || x - WIDTH == y || x + WIDTH == y)
		return 1;
	else
		return 0;
}
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
