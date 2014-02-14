// Alex Wallisch
// $log$

/*
 * The above lines will include the name and log of the last person
 * to commit this file to CVS
 */

/************************************************************************
**
** NAME:        mqland.c
**
** DESCRIPTION: Queensland
**
** AUTHOR:      Steven Kusalo, Alex Wallisch
**
** DATE:        2004-09-13
**
** UPDATE HIST: 2004-10-30	Finished GetInitialPosition
                2004-10-26	Finished helpstrings, finalized PrintPosition
                2004-10-25      Fixed the last bug in GameSpecificMenu
**				Wrote ValidTextInput
**              2004-10-08      Partially wrote GameSpecificMenu
**		2004-10-03      Wrote Primitive
**				Wrote ConvertTextInputToMove
**                              Wrote PrintComputersMove
**                              Wrote PrintMove
**              2004-10-02:	Wrote GetMoveList
**		2004-09-27:     Wrote vcfg
**				Wrote DoMove
**		2004-09-26:	Wrote InitializeBoard
**		2004-09-25:	Wrote PrintPosition
**
**************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/

#include <stdio.h>
#include "gamesman.h"
#include "hash.h"
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <ctype.h>


/*************************************************************************
**
** Game-specific constants
**
**************************************************************************/

STRING kGameName            = "Queensland";   /* The name of your game */
STRING kAuthorName          = "Steven Kusalo and Alex Wallisch";   /* Your name(s) */
STRING kDBName              = "qland";   /* The name to store the database under */

BOOLEAN kPartizan            = TRUE;   /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN kGameSpecificMenu    = TRUE;   /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN kTieIsPossible       = TRUE;   /* TRUE if a tie is possible. FALSE if it is impossible.*/
BOOLEAN kLoopy               = FALSE;   /* TRUE if the game tree will have cycles (a rearranger style game). FALSE if it does not.*/

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
        "On your turn, enter the coordinates of a piece you want to move and then the coordinates of where you want to move it to.  Then type the coordinates of an empty position on the board where you'd like to place a piece.  If you don't want to move a piece, you may just type the coordinates of where you want to place a new piece and ignore the first part.  For example, these are both legal moves: [a1 a3 b2] [b2]";

STRING kHelpOnYourTurn =
        "If you want to move a piece, type the position of the piece and the position you want to move it to (e.g. \"a1 a3\").  Ignore this if you don't want to move.  Then, type the number of an empty position where you want to place a new piece (e.g. \"b2\").  A complete move will look something like this: \"a1 a3 b2\", or alternatively, if you don't want to move the piece from a1 to a3, you can just type \"b2\"";

STRING kHelpStandardObjective =
        "When all pieces are on the board, the game ends.  Any two pieces of your color that are connected by a straight (horizontal, vertical or diagonal), unbroken line score one point for each empty space they cover.  You win if you score MORE points than your opponent.";

STRING kHelpReverseObjective =
        "When all pieces are on the board, the game ends.  Any two pieces of your color that are connected by a straight (horizontal, vertical, or diagonal), unbroken line score one point for each empty space they cover.  You win if you score FEWER points than your opponent.";

STRING kHelpTieOccursWhen =
        "A tie occurs when each player has played all their pieces and have the same number of points.";

STRING kHelpExample =
        "   Queensland!\n\
/===============\\\n\
|   XXXX OOOO   |\n\
|  /---------\\ X|\n\
| 4| . . . . | 0|\n\
| 3| . . . . |  |\n\
| 2| . . . . |  |\n\
| 1| . . . . | 0|\n\
|  \\---------/ O|\n\
|    a b c d    |\n\
\\===============/\n\
\n\
Player 1's move [(undo)/(SOURCE DESTINATION PLACE)/(PLACE)] : a1\n\
   Queensland!\n\
/===============\\\n\
|   XXX. OOOO   |\n\
|  /---------\\ X|\n\
| 4| . . . . | 0|\n\
| 3| . . . . |  |\n\
| 2| . . . . |  |\n\
| 1| X . . . | 0|\n\
|  \\---------/ O|\n\
|    a b c d    |\n\
\\===============/\n\
\n\
Player 2's move [(undo)/(SOURCE DESTINATION PLACE)/(PLACE)] : d3\n\
   Queensland!\n\
/===============\\\n\
|   XXX. .OOO   |\n\
|  /---------\\ X|\n\
| 4| . . . . | 0|\n\
| 3| . . . O |  |\n\
| 2| . . . . |  |\n\
| 1| X . . . | 0|\n\
|  \\---------/ O|\n\
|    a b c d    |\n\
\\===============/\n\
\n\
Player 1's move [(undo)/(SOURCE DESTINATION PLACE)/(PLACE)] : d4\n\
   Queensland!\n\
/===============\\\n\
|   XX.. .OOO   |\n\
|  /---------\\ X|\n\
| 4| . . . X | 2|\n\
| 3| . . . O |  |\n\
| 2| . . . . |  |\n\
| 1| X . . . | 0|\n\
|  \\---------/ O|\n\
|    a b c d    |\n\
\\===============/\n\
\n\
Player 2's move [(undo)/(SOURCE DESTINATION PLACE)/(PLACE)] : d3 c3 c1\n\
   Queensland!\n\
/===============\\\n\
|   XX.. ..OO   |\n\
|  /---------\\ X|\n\
| 4| . . . X | 0|\n\
| 3| . . O . |  |\n\
| 2| . . . . |  |\n\
| 1| X . O . | 1|\n\
|  \\---------/ O|\n\
|    a b c d    |\n\
\\===============/\n\
\n\
Player 1's move [(undo)/(SOURCE DESTINATION PLACE)/(PLACE)] : a1 a2 d2\n\
   Queensland!\n\
/===============\\\n\
|   X... ..OO   |\n\
|  /---------\\ X|\n\
| 4| . . . X | 3|\n\
| 3| . . O . |  |\n\
| 2| X . . X |  |\n\
| 1| . . O . | 1|\n\
|  \\---------/ O|\n\
|    a b c d    |\n\
\\===============/\n\
\n\
Player 2's move [(undo)/(SOURCE DESTINATION PLACE)/(PLACE)] : c3 b2 b4\n\
   Queensland!\n\
/===============\\\n\
|   X... ...O   |\n\
|  /---------\\ X|\n\
| 4| . O . X | 1|\n\
| 3| . . . . |  |\n\
| 2| X O . X |  |\n\
| 1| . . O . | 1|\n\
|  \\---------/ O|\n\
|    a b c d    |\n\
\\===============/\n\
\n\
Player 1's move [(undo)/(SOURCE DESTINATION PLACE)/(PLACE)] : d2 d1 a4\n\
   Queensland!\n\
/===============\\\n\
|   .... ...O   |\n\
|  /---------\\ X|\n\
| 4| X O . X | 5|\n\
| 3| . . . . |  |\n\
| 2| X O . . |  |\n\
| 1| . . O X | 1|\n\
|  \\---------/ O|\n\
|    a b c d    |\n\
\\===============/\n\
\n\
Player 2's move [(undo)/(SOURCE DESTINATION PLACE)/(PLACE)] : c1 d2 c2\n\
   Queensland!\n\
/===============\\\n\
|   .... ....   |\n\
|  /---------\\ X|\n\
| 4| X O . X | 1|\n\
| 3| . . . . |  |\n\
| 2| X O O O |  |\n\
| 1| . . . X | 2|\n\
|  \\---------/ O|\n\
|    a b c d    |\n\
\\===============/\n\
\n\
\n\
Player 2 (player two) Wins!\n"                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       ;

/*************************************************************************
**
** #defines and structs
**
**************************************************************************/
#define BLANK '.'
#define WHITE 'X'
#define BLACK 'O'

/* The min width and height are set to
 * 3 since the game isn't any fun otherwise.
 * The max is set to 9 so that we can
 * assume a move is of the form letterdigit,
 * e.g. a9. Also when allowing the
 * user to change the board size we assume
 * that the newwidth and height are one
 * character.
 */
#define MAX_HEIGHT 9
#define MIN_HEIGHT 3
#define MAX_WIDTH 9
#define MIN_WIDTH 3
#define MIN_PIECES 2
#define MAX_PIECES width

#define pieceat(B, x, y) ((B)[(y) * width + (x)])
#define get_location(x, y) ((y) * width + (x))
#define get_x_coord(location) ((location) % width)
#define get_y_coord(location) ((location) / width)

/* This represents a move as being source*b^2 + dest*b +place
 * where b is the size of the board, i.e. width*height.
 */
#define get_move_source(move) ((move) / (width*width*height*height))
#define get_move_dest(move) (((move) % (width*width*height*height)) / (width*height))
#define get_move_place(move) ((move) % (width*height))
#define set_move_source(move, source) ((move) += ((source)*width*width*height*height))
#define set_move_dest(move, dest) ((move)+= ((dest)*width*height))
#define set_move_place(move, place) ((move) += (place))

/*************************************************************************
**
** Global Variables
**
*************************************************************************/

/* Default values, can be changed in GameSpecific Menu */
int height = 4;
int width = 4;
int numpieces = 4;
enum rules_for_sliding {MUST_SLIDE, MAY_SLIDE, NO_SLIDE} slide_rules = MAY_SLIDE;
BOOLEAN scoreDiagonal = TRUE;
BOOLEAN scoreStraight = TRUE;
BOOLEAN moveDiagonal = TRUE;
BOOLEAN moveStraight = TRUE;

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

int vcfg(int* this_cfg);
int next_player(POSITION position);

int XYToNumber(char* xy);
int countPieces(char *board, char piece);
int scoreBoard(char *board, char player);
void ChangeBoardSize();
void ChangeNumPieces();
MOVELIST* add_all_place_moves(int source_pos, int dest_pos, char* board, MOVELIST* moves);
BOOLEAN valid_move(int source_pos, int dest_pos, char* board);


STRING MoveToString(MOVE);


/************************************************************************
**
** NAME:        InitializeGame
**
** DESCRIPTION: Prepares the game for execution.
**              Initializes required variables.
**              Sets up gDatabase (if necessary).
**
************************************************************************/

void InitializeGame () {
	int i, j;
	int pieces_array[10] = {BLANK, 0, width * height, WHITE, 0, numpieces, BLACK, 0, numpieces, -1 };
	char* board = (char*)malloc(sizeof(char) * width * height);

	gNumberOfPositions = generic_hash_init(width * height, pieces_array, vcfg, 0);
	for (j = 0; j < height; j++) {
		for (i = 0; i < width; i++) {
			pieceat(board, i, j) = BLANK;
		}
	}
	gInitialPosition = generic_hash_hash(board, 1);
	getOption();

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

MOVELIST *GenerateMoves (POSITION position)
{
	MOVELIST *moves = NULL;

	/* Use CreateMovelistNode(move, next) to 'cons' together a linked list */
	int player;
	int s, d; /* source, dest */
	char* board = (char*) SafeMalloc(sizeof(char) * width * height);
	char players_piece;

	player = next_player(position);
	players_piece = (player == 1 ? WHITE : BLACK);
	board = generic_hash_unhash(position, board);


	if (slide_rules != NO_SLIDE) {
		for (s = width * height - 1; s >= 0; s--) {
			if (pieceat(board, get_x_coord(s), get_y_coord(s)) == players_piece) {
				for (d = width * height - 1; d >= 0; d--) {
					if (valid_move(s,d, board)) {
						moves = add_all_place_moves(s, d, board, moves);
					}
				}
			}
		}
	}

	/**
	** Check moves that don't slide a piece from SOURCE to DEST
	** This is checked after moves that have a slide component are generated.  If no slide moves
	** are available, this section will ignore the MUST_SLIDE rule.
	*/
	if ((slide_rules != MUST_SLIDE) || moves == NULL) {
		moves = add_all_place_moves(0, 0, board, moves);
	}

	SafeFree(board);
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

POSITION DoMove (POSITION position, MOVE move) {
	/* This function does ZERO ZILCH ABSOLUTELY-NONE-AT-ALL error checking.  move had better be valid */

	int player = next_player(position);
	char players_piece = player == 1 ? WHITE : BLACK;
	POSITION new;
	char* board = (char*) SafeMalloc(sizeof(char) * width * height);
	int source = get_move_source(move);
	int dest = get_move_dest(move);
	int place = get_move_place(move);

	board = generic_hash_unhash(position, board);

	if (source != dest) {
		/* Place a new piece at the destination of the SLIDE move */
		pieceat(board, get_x_coord(dest), get_y_coord(dest)) = players_piece;

		/* Erase the piece from the source of the SLIDE move */
		pieceat(board, get_x_coord(source), get_y_coord(source)) = BLANK;
	}

	/* Place a new piece at the location of the PLACE move */
	pieceat(board, get_x_coord(place), get_y_coord(place)) = players_piece;

	new = generic_hash_hash(board, player);

	SafeFree(board);
	return new;
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

VALUE Primitive (POSITION position) {
	char* board = (char*) SafeMalloc(sizeof(char) * width * height);
	board = generic_hash_unhash(position, board);
	if (countPieces(board,WHITE) != numpieces || countPieces(board, BLACK) != numpieces) {
		SafeFree(board);
		return undecided;
	} else {
		int blackscore = scoreBoard(board, BLACK);
		int whitescore = scoreBoard(board, WHITE);
		SafeFree(board);
		if (whitescore == blackscore) {
			return tie;
		} else if (whitescore > blackscore) {
			return gStandardGame ? win : lose;
		} else {
			return gStandardGame ? lose : win;
		}
	}
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

void PrintPosition (POSITION position, STRING playersName, BOOLEAN usersTurn) {
	char *board = (char*) SafeMalloc(sizeof(char) * width * height);
	board= generic_hash_unhash(position, board);
	int i, j;

	if (width < 4) {
		printf("  Queensland!\n");
	}
	else {
		for (i = 0; i < width - 1; i++)
		{
			printf(" ");
		}
		printf("Queensland!\n");
	}

	printf("/");                                            /* Top row */
	for (i = 0; i < (2 * width + 7); i++) {                 /* /===============\ */
		printf("=");
	}
	printf("\\\n");

	printf("|   ");
	for (i = 0; i < width - numpieces; i++) {               /* Second row */
		printf(" ");                                    /* |   XX-- -OOO   | */
	}
	for (i = 0; i < numpieces; i++) {
		if (numpieces - countPieces(board, WHITE) > i) {
			printf("%c", WHITE);
		} else {
			printf("%c", BLANK);
		}
	}
	printf(" ");
	for (i = 0; i < numpieces; i++) {
		if (countPieces(board, BLACK) <= i) {
			printf("%c", BLACK);
		} else {
			printf("%c", BLANK);
		}
	}
	for (i = 0; i < width - numpieces; i++) {
		printf(" ");
	}
	printf("   |\n");
	/* Third row */
	printf("|  /");                                         /* |  /---------\ X| */
	for (i = 0; i < (2*width+1); i++) {
		printf("-");
	}
	printf("\\ %c|\n", WHITE);


	for (j = 0; j < height; j++) {                          /* Body of board */

		printf("| %d", height-j);

		printf("| ");
		for (i = 0; i < width; i++) {
			switch(pieceat(board, i, j)) {
			case BLANK:
				printf("%c ", BLANK);
				break;
			case WHITE:
				printf("%c ", WHITE);
				break;
			case BLACK:
				printf("%c ", BLACK);
				break;
			default:
				BadElse("PrintPosition");
			}
		}
		printf("|");
		if (j == 0) {
			printf("%2d", scoreBoard(board, WHITE));
		} else if (j == height - 1) {
			printf("%2d", scoreBoard(board, BLACK));
		} else printf("  ");
		printf("|\n");
	}

	printf("|  \\");                                        /* Third-from-bottom row */
	for (i = 0; i < (2*width+1); i++) {                     /* |  \---------/ O| */
		printf("-");
	}
	printf("/ %c|\n", BLACK);

	printf("|    ");
	for (i = 'a'; i < width + 'a'; i++) {                   /* |    a b c d    | */
		printf("%c ", i);
	}
	printf("   |\n");

	printf("\\");                                           /* Bottom row */
	for (i = 0; i < (2 * width + 7); i++) {                 /* \===============/ */
		printf("=");
	}
	printf("/\n");
	printf("%s\n", GetPrediction(position, playersName, usersTurn));
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
	printf("%8s's move: ", computersName);
	PrintMove(computersMove);
	printf("\n");
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

STRING MoveToString (move)
MOVE move;
{
	STRING m = (STRING) SafeMalloc( 14 );
	if (get_move_source(move) == 0 && get_move_dest(move) == 0) {
		sprintf( m,
		         "[%c%d]",
		         get_x_coord(get_move_place(move)) + 'a',
		         height - get_y_coord(get_move_place(move)));
	}
	else {
		sprintf( m,
		         "[%c%d %c%d %c%d]",
		         get_x_coord(get_move_source(move)) + 'a',
		         height - get_y_coord(get_move_source(move)),
		         get_x_coord(get_move_dest(move)) + 'a',
		         height - get_y_coord(get_move_dest(move)),
		         get_x_coord(get_move_place(move)) + 'a',
		         height - get_y_coord(get_move_place(move)));
	}

	return m;
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
		printf("%8s's move [(undo)/(SOURCE DESTINATION PLACE)/(PLACE)] : ", playersName);

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

	int pos = 0;

	/* Eat whitespace */
	while (isspace(input[pos])) pos++;

	/* Next 2 chars should be letterdigit, e.g. a9
	 * and they must be within range */
	if (input[pos] == '\0' || input[pos] < 'a' || input[pos] > 'a' + (width-1)) {
		return FALSE;
	}
	pos++;
	if (input[pos] == '\0' || input[pos] < '1' || input[pos] > '1' + (height-1)) {
		return FALSE;
	}
	pos++;

	/* Eat whitespace */
	while (isspace(input[pos])) pos++;

	/* The input was a valid move without a slide */
	if (input[pos] == '\0') {
		return TRUE;
	}

	/* Next 2 chars should be letterdigit */
	if (input[pos] < 'a' || input[pos] > 'a' + (width-1)) {
		return FALSE;
	}
	pos++;
	if (input[pos] == '\0' || input[pos] < '1' || input[pos] > '1' + (height-1)) {
		return FALSE;
	}
	pos++;

	/* Eat whitespace */
	while (isspace(input[pos])) pos++;

	/* Next 2 chars should be letterdigit */
	if (input[pos] == '\0' || input[pos] < 'a' || input[pos] > 'a' + (width-1)) {
		return FALSE;
	}
	pos++;
	if (input[pos] == '\0' || input[pos] < '1' || input[pos] > '1' + (height-1)) {
		return FALSE;
	}
	pos++;

	/* Eat whitespace */
	while (isspace(input[pos])) pos++;

	/* The input was a valid move with a slide */
	if (input[pos] == '\0') {
		return TRUE;
	}

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

MOVE ConvertTextInputToMove (STRING input) {

	int first, second, third;
	char* curr = input;
	MOVE move = 0;

	/* Eat leading whitespace */
	while (isspace((int)*curr)) curr++;

	/* Turn the next two chars into an position
	 * on the board */
	first = XYToNumber(curr);
	curr +=2;

	/* Eat more whitespace */
	while (isspace((int)*curr)) curr++;

	/* Check if we are at the end of the string. If
	 * so then this is a place only.*/
	if (!(*curr)) {
		set_move_source(move, 0);
		set_move_dest(move, 0);
		set_move_place(move, first);
	} else {
		/* Turn the next two chars into an position
		 * on the board */
		second = XYToNumber(curr);
		curr +=2;

		/* Eat more whitespace */
		while (isspace((int)*curr)) curr++;

		/* Turn the next two chars into an position
		 * on the board */
		third = XYToNumber(curr);
		curr +=2;

		set_move_source(move, first);
		set_move_dest(move, second);
		set_move_place(move, third);
	}

	return move;
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

void GameSpecificMenu () {
	char c;
	while (TRUE) {
		printf("\n\nGame Options:\n\n");
		printf("\ts)\tchange the (S)ize of the board (currently %d by %d)\n", width, height);
		printf("\tp)\tchange the number of (P)ieces (currently %d)\n", numpieces);
		printf("\tt)\ttoggle the (T)urn structure (currently a player %s move a piece before placing)\n",
		       slide_rules == MAY_SLIDE ? "*MAY*/must/cannot" :
		       (slide_rules == MUST_SLIDE ? "may/*MUST*/cannot" : "may/must/*CANNOT*"));
		printf("\tm)\ttoggle the way a piece (M)oves (currently pieces move like %s)\n",
		       moveDiagonal ? (moveStraight ? "*QUEENS*/bishops/rooks" :
		                       "queens/*BISHOPS*/rooks") : "queens/bishops/*ROOKS*");
		printf("\tc)\ttoggle the s(C)oring system (currently %s lines are scored)\n",
		       scoreStraight ? (scoreDiagonal ? "*ALL*/straight/diagonal" :
		                        "all/*STRAIGHT*/diagonal") : "all/straight/*DIAGONAL*");
		printf("\tb)\tgo (B)ack to the previous menu\n");
		printf("\nSelect an option:  ");

		while (!isalpha(c = getc(stdin))) ;
		c = tolower(c);
		switch (c) {
		case 's':
			ChangeBoardSize();
			break;
		case 'p':
			ChangeNumPieces();
			break;
		case 't':
			if (slide_rules == MAY_SLIDE) {
				slide_rules = MUST_SLIDE;
			} else if (slide_rules == MUST_SLIDE) {
				slide_rules = NO_SLIDE;
			} else {
				slide_rules = MAY_SLIDE;
			}
			break;
		case 'm':
			if (moveDiagonal && moveStraight) {
				moveDiagonal = TRUE;
				moveStraight = FALSE;
			} else if (moveStraight) {
				moveDiagonal = TRUE;
				moveStraight = TRUE;
			} else {
				moveDiagonal = FALSE;
				moveStraight = TRUE;
			}
			break;
		case 'c':
			if (scoreDiagonal && scoreStraight) {
				scoreStraight = TRUE;
				scoreDiagonal = FALSE;
			} else if (scoreStraight) {
				scoreStraight = FALSE;
				scoreDiagonal = TRUE;
			} else {
				scoreStraight = TRUE;
				scoreStraight = TRUE;
			}
			break;
		case 'b':
			return;
		default:
			printf("Invalid option. Please try again.\n");
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

POSITION GetInitialPosition () {
	int i;
	char c;
	char* board = (char*)malloc(width * height * sizeof(char));

	/** WARNING: This results of this function are undefined if the user specifies a board
	** that should never be reached through normal play.
	**/

	printf("\n\n\t----- Get Initial Position -----\n");
	printf("\n\tPlease input the position to begin with.\n");
	printf("\tNote that it should be in the following format:\n\n");
	printf("%c %c %c %c\n%c %c %c %c            <----- EXAMPLE \n%c %c %c %c\n%c %c %c %c\n\n", WHITE, BLANK, BLANK, BLANK, BLANK, BLACK, BLANK, WHITE, BLANK, BLANK, BLANK, BLANK, BLANK, BLACK, BLANK, WHITE);

	i = 0;
	getchar();
	while (i < width * height && (c = getchar()) != EOF) {
		if (c == BLANK || c == WHITE || c == BLACK) {
			board[i++] = c;
		}
		/** Currently, BLACK is the capital letter 'O'.  It is NOT the number zero.
		** On some terminals, however, 'O' and '0' are indistinguishable, or at least not
		** easily distinguishable.  To make things easier on the player, this function will
		** accept either the letter 'O' or the number zero to represent BLACK.  Delete this
		** next statement if BLACK changes to something other than the letter 'O'.
		**/
		if (c == '0') {
			board[i++] = BLACK;
		}
	}
	return generic_hash_hash(board, countPieces(board, BLACK) < countPieces(board, WHITE) ? 2 : 1);
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
	/*
	 * 2 win conditions (standard or misere)
	 * 8 styles of moves
	 * 245 valid board size/num pieces configurations
	 * 3 styles of scoring
	 */
	return 2 * 8 * 245 * 3;
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
	/* The options which make a variant of our game are (copied from above):
	 * int height = 4;
	 * int width = 4;
	 * int numpieces = 4;
	 * enum rules_for_sliding {MUST_SLIDE, MAY_SLIDE, NO_SLIDE} slide_rules = MAY_SLIDE;
	 * BOOLEAN scoreDiagonal = TRUE;
	 * BOOLEAN scoreStraight = TRUE;
	 * BOOLEAN moveDiagonal = TRUE;
	 * BOOLEAN moveStraight = TRUE;
	 *
	 * There is also the question of whether this is standard or misere.
	 */

	int winConditionVal;
	int moveStyleVal;
	int boardSizeVal;
	int scoreVal;
	int i, j, k, l;

	/* determine winConditionVal (0 to 1) */
	if (gStandardGame) {
		winConditionVal = 0;
	} else {
		winConditionVal = 1;
	}

	/* determine moveStyleVal (0 to 7) */
	if (slide_rules == NO_SLIDE) {
		moveStyleVal = 0;
	} else if (slide_rules == MAY_SLIDE) {
		moveStyleVal = 1;
	} else if (slide_rules == MUST_SLIDE) {
		moveStyleVal = 2;
	} else {
		BadElse("getOption");
	}
	if (slide_rules != NO_SLIDE) {
		if (moveDiagonal && moveStraight) {
			moveStyleVal += (2 * 0);
		} else if (moveDiagonal) {
			moveStyleVal += (2 * 1);
		} else if (moveStraight) {
			moveStyleVal += (2 * 2);
		} else {
			BadElse("getOption");
		}
	}

	/* determine boardSizeVal (0 to 244) */
	boardSizeVal = 0;
	l = 0;
	for (i = MIN_WIDTH; i <= MAX_WIDTH; i++) {
		for (j = MIN_PIECES; j <= i; j++) {
			for (k = MIN_HEIGHT; k <= MAX_HEIGHT; k++) {
				if (i ==  width && j == numpieces && k == height) {
					boardSizeVal = l;
				}
				l++;
			}
		}
	}

	/* determine scoreVal (0 to 2) */
	if (scoreDiagonal && scoreStraight) {
		scoreVal = 0;
	} else if (scoreDiagonal) {
		scoreVal = 1;
	} else if (scoreStraight) {
		scoreVal = 2;
	} else {
		BadElse("getOption");
	}

	return 245*8*2*scoreVal + 8*2*boardSizeVal + 2*moveStyleVal + winConditionVal + 1;
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
	option = option - 1;

	int winConditionVal = option % 2;
	int moveStyleVal = (option / 2) % 8;
	int boardSizeVal = (option / (2 * 8)) % 245;
	int scoreVal = (option / (2 * 8 * 245)) % 3;

	int slide_op;
	int move_op;

	int i, j, k, l;

	/* set the options indicated by winConditionVal */
	if (winConditionVal == 0) {
		gStandardGame = TRUE;
	} else {
		gStandardGame = FALSE;
	}

	/* set the options indicated by moveStyleVal */
	if (moveStyleVal == 0) {
		slide_rules = NO_SLIDE;
	} else {
		moveStyleVal = moveStyleVal - 1;
		slide_op = moveStyleVal % 2;
		move_op = (moveStyleVal / 2) % 3;
		if (slide_op == 0) {
			slide_rules = MAY_SLIDE;
		} else if (slide_op == 1) {
			slide_rules = MUST_SLIDE;
		} else {
			BadElse("setOption");
		}
		if (move_op == 0) {
			moveDiagonal = TRUE;
			moveStraight = TRUE;
		} else if (move_op == 1) {
			moveDiagonal = TRUE;
			moveStraight = FALSE;
		} else if (move_op == 2) {
			moveDiagonal = FALSE;
			moveStraight = TRUE;
		} else {
			BadElse("setOption");
		}
	}

	/* set the options indicated by boardSizeVal */
	l = 0;
	for (i = MIN_WIDTH; i <= MAX_WIDTH; i++) {
		for (j = MIN_PIECES; j <= i; j++) {
			for (k = MIN_HEIGHT; k <= MAX_HEIGHT; k++) {
				if (l == boardSizeVal) {
					width = i;
					numpieces = j;
					height = k;
				}
				l++;
			}
		}
	}

	/* set the options indicated by scoreVal */
	if (scoreVal == 0) {
		scoreDiagonal = TRUE;
		scoreStraight = TRUE;
	} else if (scoreVal == 1) {
		scoreDiagonal = TRUE;
		scoreStraight = FALSE;
	} else if (scoreVal == 2) {
		scoreDiagonal = FALSE;
		scoreStraight = TRUE;
	} else {
		BadElse("setOption");
	}
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
	/* If number of BLACKs is equal to or one less than number of WHITEs then this configuration is valid. */
	return this_cfg[2] == this_cfg[1] || this_cfg[2] + 1 == this_cfg[1];
}

int next_player(POSITION position) {
	char* board = (char*)SafeMalloc(sizeof(char) * width * height);
	int numWhite, numBlack;

	board = generic_hash_unhash(position, board);
	numBlack = countPieces(board, BLACK);
	numWhite = countPieces(board, WHITE);
	SafeFree (board);
	return (numWhite > numBlack ? 2 : 1);
}

/*
 * This function tallys and returns how many points
 * the given player (the argument should either be
 * BLACK or WHITE) gets for this board.
 */
int scoreBoard(char *board, char player) {
	int x;
	int y;
	int score = 0;

	for (x = 0; x < width; x++) {
		for (y = 0; y < height; y++) {
			if(pieceat(board, x, y) == player) {
				int i; /* used when x varies */
				int j; /* used when y varies */

				if (scoreStraight) {

					/* count any horizontal lines going to the right */
					for (i = x+1; i < width && pieceat(board, i, y) == BLANK; i++) { }
					if (i < width && pieceat(board, i, y) == player) {
						score += (i-x-1);
					}

					/* count any vertical lines going down */
					for (j = y+1; j < height && pieceat(board, x, j) == BLANK; j++) { }
					if (j < height && pieceat(board, x, j) == player) {
						score += (j-y-1);
					}
				}

				if (scoreDiagonal) {

					/* count any diagonal lines going up and to the right */
					for (i = x+1, j = y-1; i < width && j >= 0 && pieceat(board, i, j) == BLANK; i++, j--) { }
					if (i < width && j >= 0 && pieceat(board, i, j) == player) {
						score += (i-x-1);
					}

					/* count any diagonal lines going down and to the right */
					for (i = x+1, j = y+1; i < width && j < height && pieceat(board, i, j) == BLANK; i++, j++) { }
					if (i < width && j < height && pieceat(board, i, j) == player) {
						score += (i-x-1);
					}
				}
			}
		}
	}
	return score;
}

/* Counts the number of pieces of the given type
 * that are on the board.
 */
int countPieces(char *board, char piece) {
	int x;
	int y;
	int count = 0;
	for (x = 0; x < width; x++) {
		for (y = 0; y < height; y++) {
			if (board[get_location(x,y)] == piece) {
				count++;
			}
		}
	}
	return count;
}

/* Allows the user to change the width and height
 * of the board.
 */
void ChangeBoardSize() {
	int newWidth, newHeight;
	char c;
	while(TRUE) {
		printf("\n\nEnter a new width for the board (between %d and %d): ", MIN_WIDTH, MAX_WIDTH);
		while (isspace(c = getc(stdin))) ;
		newWidth = c - '0';
		printf("Enter a new height for the board (between %d and %d): ", MIN_HEIGHT, MAX_HEIGHT);
		while (isspace(c = getc(stdin))) ;
		newHeight = c - '0';;
		if (newWidth <= MAX_WIDTH && newHeight <= MAX_HEIGHT && newWidth >= MIN_WIDTH && newHeight >= MIN_HEIGHT) {
			height = newHeight;
			width = newWidth;
			printf("\nThe board is now %d by %d.\n", width, height);
			return;
		}
		printf("Board must be between %d by %d and %d by %d.  Please try again.\n",
		       MIN_WIDTH, MIN_HEIGHT,
		       MAX_WIDTH, MAX_HEIGHT);
	}
}

/* Allows the user to change the number of pieces
 * each player starts with.
 */
void ChangeNumPieces() {
	int newAmount;
	char c;
	while(TRUE) {
		printf("\n\nEnter the number of pieces each team starts with (between %d and %d): ", MIN_PIECES, MAX_PIECES);
		while (isspace(c = getc(stdin))) ;
		newAmount = c - '0';
		if (newAmount <= MAX_PIECES && newAmount >= MIN_PIECES) {
			numpieces = newAmount;
			printf("\nNumber of pieces changed to %d\n", numpieces);
			return;
		}
		printf("Number must be between %d and %d.  Please try again.\n", MIN_PIECES, MAX_PIECES);
	}
}

/* Converts a string of two characters to a board position */
int XYToNumber(char* xy) {
	int x, y;

	x = tolower(xy[0]) - 'a';
	y = height - (xy[1] - '0');
	return get_location(x, y);
}


MOVELIST* add_all_place_moves(int source_pos, int dest_pos, char* board, MOVELIST* moves) {
	int px, py;
	MOVE move;
	for (px = width - 1; px >= 0; px--) {
		for (py = height - 1; py >= 0; py--) {
			//if (px,py) is BLANK and either
			//1) there is no slide move; or
			//2) the destination of the slide move is not (px,py)
			//then (px,py) is a valid place move
			if (pieceat(board, px, py) == BLANK && ((source_pos == 0 && dest_pos == 0) || !(get_x_coord(dest_pos) == px && get_y_coord(dest_pos) == py))) {
				move = 0;
				set_move_source(move, source_pos);
				set_move_dest(move, dest_pos);
				set_move_place(move, get_location(px, py));
				moves = CreateMovelistNode(move, moves);
			}
		}
	}
	return moves;
}

BOOLEAN valid_move(int source_pos, int dest_pos, char* board) {
	int sx = get_x_coord(source_pos);
	int sy = get_y_coord(source_pos);
	int dx = get_x_coord(dest_pos);
	int dy = get_y_coord(dest_pos);
	int i, j;

	if (pieceat(board, dx, dy) != BLANK) {
		return FALSE;
	}
	else if (sx == dx && sy == dy) {
		return FALSE;
	}
	else if ((sx == dx || sy == dy) && !moveStraight) {
		return FALSE;
	}
	else if (sx == dx) {
		for (i = (sy < dy ? sy + 1 : sy - 1); i != dy; (sy < dy ? i++ : i--)) {
			if (pieceat(board, sx, i) != BLANK) {
				return FALSE;
			}
		}
	}
	else if (sy == dy) {
		for (i = (sx < dx ? sx + 1 : sx - 1); i != dx; (sx < dx ? i++ : i--)) {
			if (pieceat(board, i, sy) != BLANK) {
				return FALSE;
			}
		}
	}
	else if ((abs(sx - dx) == abs (sy - dy)) && !moveDiagonal) {
		return FALSE;
	}
	else if (abs(sx - dx) == abs(sy - dy)) { /* Check if (sx, sy) and (dx, dy) are on the same diagonal line */
		for (i = (sx < dx ? sx + 1 : sx - 1), j = (sy < dy ? sy + 1 : sy - 1); i != dx; (sx < dx ? i++ : i--), (sy < dy ? j++ : j--)) {
			if (pieceat(board, i, j) != BLANK) {
				return FALSE;
			}
		}
	}
	else return FALSE;
	return TRUE;
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
