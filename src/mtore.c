// $id$
// $log$

/*
 * The above lines will include the name and log of the last person
 * to commit this file to CVS
 */

/************************************************************************
**
** NAME:        mtore.c
**
** DESCRIPTION: Mu Torere
**
** AUTHOR:      Joe Jing, Jeff Chou
**
** DATE:        2004-9-28 Started Module.
**
** UPDATE HIST: 2006-5-22	 Fixed getoption() and setoption()
**				2005-3-3     Added missing prototypes
**              2004-10-26   Changed PrintPosition to include layout of board
**                           Wrote GetinitialPosition
**              2004-10-22   Error still in ValidTextInput
**              2004-10-21   Fixed GenerateMoves, Primitive
**              2004-10-10   Wrote ConvertTextInputToMove
**                           Wrote PrintMove, and ValidTextInput
**              2004-10-4    Wrote GenerateMoves, and Primative
**              2004-9-28    Started Module. Print Position, Do Move, Hash Stuff.
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

STRING kGameName            = "Mu Torere";   /* The name of your game */
STRING kAuthorName          = "Joe Jing and Jeff Chou";   /* Your name(s) */
STRING kDBName              = "tore";   /* The name to store the database under */

BOOLEAN kPartizan            = TRUE;   /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN kGameSpecificMenu    = FALSE;   /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN kTieIsPossible       = FALSE;   /* TRUE if a tie is possible. FALSE if it is impossible.*/
BOOLEAN kLoopy               = TRUE;   /* TRUE if the game tree will have cycles (a rearranger style game). FALSE if it does not.*/

BOOLEAN kDebugMenu           = FALSE;   /* TRUE only when debugging. FALSE when on release. */
BOOLEAN kDebugDetermineValue = FALSE;   /* TRUE only when debugging. FALSE when on release. */

POSITION gNumberOfPositions   =  0; /* The number of total possible positions | If you are using our hash, this is given by the hash_init() function*/
POSITION gInitialPosition     =  0; /* The initial hashed position for your starting board */
POSITION kBadPosition         = -1; /* A position that will never be used */
void* gGameSpecificTclInit    = NULL;


/*
 * Help strings that are pretty self-explanatory
 * Strings than span more than one line should have backslashes (\) at the end of the line.
 */

STRING kHelpGraphicInterface =
        "Not written yet";

STRING kHelpTextInterface =
        "Type in from what position you want to move.  For example, if you want to move to 3, type in '3'.";

STRING kHelpOnYourTurn =
        "Any piece may move into the middle as long as the middle space is empty and the moving piece is adjacent to an opponent's piece.  Any piece may move to any free adjacent side position.";

STRING kHelpStandardObjective =
        "The objective is to trap your opponent so he has no possible moves left.";

STRING kHelpReverseObjective =
        "Objective is to obtain a position in which you have no more moves";

STRING kHelpTieOccursWhen =
        "A tie will never occur";


STRING kHelpExample =
        "   Board         Key \n\
  o  |  o      2  |  4\n\
   \\ | /        \\ | /\n\
 o-- _ --x    1-- 0 --5\n\
   / | \\        / | \\\n\
  x  |  x      8  |  6\n\
     x            7\n\
\n\
Player's Turn(x)\n\
(Player should draw)\n\
  Player's move [(undo)/(## FromTo)] : s\n\
\n\
Here are the values of all possible moves:\n\
                Move    Remoteness\n\
Winning Moves:  \n\
Tieing Moves:   \n\
                5       Draw\n\
                8       Draw\n\
Losing Moves:   \n\
\n\
  Player's move [(undo)/(## FromTo)] : 8\n\
   Board         Key\n\
     o            3\n\
  o  |  o      2  |  4\n\
   \\ | /        \\ | /\n\
 o-- x --x    1-- 0 --5\n\
   / | \\        / | \\\n\
  _  |  x      8  |  6\n\
     x            7\n\
\n\
Computer's Turn(o)\n\
(Computer should draw)\n\
Computer's move [(undo)/(## FromTo)] : s\n\
\n\
Here are the values of all possible moves:\n\
                Move    Remoteness\n\
Winning Moves:  \n\
Tieing Moves:   \n\
                1      Draw\n\
Losing Moves:   \n\
\n\
Computer's move [(undo)/(## FromTo)] : 1\n\
   Board         Key\n\
     o            3\n\
  o  |  o      2  |  4\n\
   \\ | /        \\ | /\n\
 _-- x --x    1-- 0 --5\n\
   / | \\        / | \\\n\
  o  |  x      8  |  6\n\
     x            7\n\
\n\
Player's Turn(x)\n\
(Player should draw)\n\
  Player's move [(undo)/(## FromTo)] : s\n\
\n\
Here are the values of all possible moves:\n\
                Move    Remoteness\n\
Winning Moves:  \n\
Tieing Moves:   \n\
                1       Draw\n\
Losing Moves:   \n\
\n\
  Player's move [(undo)/(## FromTo)] : 1 \n\
   Board         Key\n\
     o            3\n\
  o  |  o      2  |  4\n\
   \\ | /        \\ | /\n\
 x-- _ --x    1-- 0 --5\n\
   / | \\        / | \\\n\
  o  |  x      8  |  6\n\
     x            7\n\
\n\
Computer's Turn(o)\n\
(Computer should draw)\n\
Computer's move [(undo)/(## FromTo)] : s\n\
\n\
Here are the values of all possible moves: \n\
                Move    Remoteness\n\
Winning Moves:  \n\
Tieing Moves:   \n\
                2       Draw\n\
                4       Draw\n\
Losing Moves:   \n\
                8       5\n\
\n\
Computer's move [(undo)/(## FromTo)] : 8\n\
   Board         Key\n\
     o            3\n\
  o  |  o      2  |  4\n\
   \\ | /        \\ | /\n\
 x-- o --x    1-- 0 --5\n\
   / | \\        / | \\\n\
  _  |  x      8  |  6\n\
     x            7\n\
\n\
Player's Turn(x)\n\
(Player should Win in 5)\n\
  Player's move [(undo)/(## FromTo)] : s\n\
\n\
Here are the values of all possible moves: \n\
                Move    Remoteness\n\
Winning Moves:  \n\
                7      4\n\
Tieing Moves:   \n\
Losing Moves:   \n\
                1      1\n\
\n\
  Player's move [(undo)/(## FromTo)] : 7\n\
   Board         Key\n\
     o            3\n\
  o  |  o      2  |  4\n\
   \\ | /        \\ | /\n\
 x-- o --x    1-- 0 --5\n\
   / | \\        / | \\\n\
  x  |  x      8  |  6\n\
     _            7\n\
\n\
Computer's Turn(o)\n\
(Computer should Lose in 4)\n\
Computer's move [(undo)/(## FromTo)] : s\n\
\n\
Here are the values of all possible moves: \n\
                Move    Remoteness\n\
Winning Moves:  \n\
Tieing Moves:   \n\
Losing Moves:   \n\
                7       3\n\
\n\
Computer's move [(undo)/(## FromTo)] : 7\n\
   Board         Key\n\
     o            3\n\
  o  |  o      2  |  4\n\
   \\ | /        \\ | /\n\
 x-- _ --x    1-- 0 --5\n\
   / | \\        / | \\\n\
  x  |  x      8  |  6\n\
     o            7\n\
\n\
Player's Turn(x)\n\
(Player should Win in 3)\n\
  Player's move [(undo)/(## FromTo)] : s\n\
\n\
Here are the values of all possible moves: \n\
                Move    Remoteness\n\
Winning Moves:  \n\
                6       2\n\
                8       2\n\
Tieing Moves:   \n\
                1       Draw\n\
                5       Draw\n\
Losing Moves:   \n\
\n\
  Player's move [(undo)/(## FromTo)] : 6\n\
   Board         Key\n\
     o            3\n\
  o  |  o      2  |  4\n\
   \\ | /        \\ | /\n\
 x-- x --x    1-- 0 --5\n\
   / | \\        / | \\ \n\
  x  |  _      8  |  6\n\
     o            7\n\
\n\
Computer's Turn(o)\n\
(Computer should Lose in 2)\n\
Computer's move [(undo)/(## FromTo)] : s\n\
\n\
   Board         Key\n\
     o            3\n\
  o  |  o      2  |  4 \n\
   \\ | /        \\ | /  \n\
 x-- x --x    1-- 0 --5 \n\
   / | \\        / | \\ \n\
  x  |  o      8  |  6  \n\
     _            7     \n\
\n\
Player's Turn(x)\n\
(Player should Win in 1)\n\
  Player's move [(undo)/(## FromTo)] : s\n\
\n\
   Board         Key\n\
     o            3\n\
  o  |  o      2  |  4\n\
   \\ | /        \\ | /\n\
 x-- x --x    1-- 0 --5\n\
   / | \\        / | \\ \n\
  _  |  o      8  |  6\n\
     x            7\n\
\n\
Computer's Turn(o)\n\
(Computer should Lose in 0)\n\
\n\
Player (player one) Wins!\n"                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  ;



/*************************************************************************
**
** #defines and structs
**
**************************************************************************/
#define move_from(x) (x / 9)
#define move_to(x) (x % 9)

#define move_make(x,y) ((x * 9) + y)
/*************************************************************************
**
** Global Variables
**
*************************************************************************/

char gBoard[9];

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

POSITION getCanonicalPosition(POSITION position);

STRING MoveToString( MOVE );

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
	int init_array[10];
	init_array[0] = '_';
	init_array[1] = 1;
	init_array[2] = 1;
	init_array[3] = 'x';
	init_array[4] = 4;
	init_array[5] = 4;
	init_array[6] = 'o';
	init_array[7] = 4;
	init_array[8] = 4;
	init_array[9] = -1;


	gNumberOfPositions = generic_hash_init(9, init_array, NULL, 0);

	gBoard[1] = gBoard[2] = gBoard[3] = gBoard[4] = 'o';
	gBoard[5] = gBoard[6] = gBoard[7] = gBoard[8] = 'x';
	gBoard[0] = '_';

	gInitialPosition = generic_hash_hash(gBoard, 1);

	gCanonicalPosition = getCanonicalPosition;

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
	char turn;
	char opp;
	int i;
	int x;
	generic_hash_unhash(position, gBoard);
	if (generic_hash_turn(position) == 1) {
		turn = 'x';
		opp = 'o';
	} else {
		turn = 'o';
		opp = 'x';
	}
	MOVELIST *moves = NULL;
	for (i = 0; i < 9; i++) {
		if (gBoard[i] == turn) {
			if (i == 0) {
				for (x = 1; x < 9; x++) {
					if (gBoard[x] == '_') {
						moves = CreateMovelistNode(i, moves);
					}
				}
			} else if (i == 1) {
				if (gBoard[8] == '_') {
					moves = CreateMovelistNode(i, moves);
				} else if (gBoard[8] == opp && gBoard[0] == '_') {
					moves = CreateMovelistNode(i, moves);
				} else if (gBoard[i+1] == '_') {
					moves = CreateMovelistNode(i, moves);
				} else if (gBoard[i+1] == opp && gBoard[0] == '_') {
					moves = CreateMovelistNode(i, moves);
				}
			} else if (i == 8) {
				if (gBoard[1] == '_') {
					moves = CreateMovelistNode(i, moves);
				} else if (gBoard[1] == opp && gBoard[0] == '_') {
					moves = CreateMovelistNode(i, moves);
				} else if (gBoard[i-1] == '_') {
					moves = CreateMovelistNode(i, moves);
				} else if (gBoard[i-1] == opp && gBoard[0] == '_') {
					moves = CreateMovelistNode(i, moves);
				}
			} else {
				if (gBoard[i-1] == '_') {
					moves = CreateMovelistNode(i, moves);
				} else if (gBoard[i+1] == '_') {
					moves = CreateMovelistNode(i, moves);
				} else if (gBoard[i-1] == opp && gBoard[0] == '_') {
					moves = CreateMovelistNode(i, moves);
				} else if (gBoard[i+1] == opp && gBoard[0] == '_') {
					moves = CreateMovelistNode(i, moves);
				}
			}
		}
	}




	/* Use CreateMovelistNode(move, next) to 'cons' together a linked list */

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
	int i, from, to;
	char oldc, turn;
	generic_hash_unhash(position,gBoard);
	for (i = 0; i < 9; i++) {
		if (gBoard[i] == '_') {
			to = i;
		}
	}
	from = move;
	oldc = gBoard[from];
	if (generic_hash_turn(position) == 1) {
		turn = 'x';
	} else {
		turn = 'o';
	}
	gBoard[to] = oldc;
	gBoard[from] = '_';
	if (generic_hash_turn(position) == 1)
		return generic_hash_hash(gBoard,2);
	else
		return generic_hash_hash(gBoard, 1);
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
*********************************************
***************************/

VALUE Primitive (POSITION position)
{
	if (GenerateMoves(position) != NULL) {
		return undecided;
	} else {
		if(gStandardGame)
			return lose;
		else
			return win;
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

void PrintPosition (POSITION position, STRING playersName, BOOLEAN usersTurn)
{
	generic_hash_unhash(position, gBoard);
	printf("   Board         Key\n");
	printf("     %c            3\n",gBoard[3]);
	printf("  %c  |  %c      2  |  4\n", gBoard[2], gBoard[4]);
	printf("   \\ | /        \\ | /\n");
	printf(" %c-- %c --%c    1-- 0 --5\n", gBoard[1], gBoard[0], gBoard[5]);
	printf("   / | \\        / | \\\n");
	printf("  %c  |  %c      8  |  6\n", gBoard[8], gBoard[6]);
	printf("     %c            7\n", gBoard[7]);
	printf("\n%s's Turn(%c)\n", playersName, (generic_hash_turn (position)) == 1 ? 'x' : 'o');
	printf("%s\n", GetPrediction(position,playersName,usersTurn));

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
	printf("%8s's move: %d\n", computersName, computersMove);


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
	STRING m = (STRING) SafeMalloc( 5 );

	sprintf( m, "%d", move);

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
		printf("%8s's move [(undo)/(## FromTo)] : ", playersName);

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
	return ((input[0] <= '8' ) && (input[0] >= '0'));
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
	int theInput = atoi(input);
	int from = theInput/10;
	int to = theInput%10;
	return ((MOVE) move_make(from, to));
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

/*  Copied from mttt.c
 */
POSITION GetInitialPosition ()
{
	// POSITION BlankOXToPosition();
	//BlankOX theBlankOX[BOARDSIZE], whosTurn;
	//signed char c;
	//int i, goodInputs = 0;


	//printf("\n\n\t----- Get Initial Position -----\n");
	//printf("\n\tPlease input the position to begin with.\n");
	//printf("\tNote that it should be in the following format:\n\n");
	//printf("O - -\nO - -            <----- EXAMPLE \n- X X\n\n");

	//i = 0;
	//getchar();
	//while(i < BOARDSIZE && (c = getchar()) != EOF) {
	//if(c == 'x' || c == 'X')
	//  theBlankOX[i++] = x;
	//else if(c == 'o' || c == 'O' || c == '0')
	//  theBlankOX[i++] = o;
	//else if(c == '-')
	//  theBlankOX[i++] = Blank;
	//else
	//  ;   /* do nothing */
	//}

	/*
	   getchar();
	   printf("\nNow, whose turn is it? [O/X] : ");
	   scanf("%c",&c);
	   if(c == 'x' || c == 'X')
	   whosTurn = x;
	   else
	   whosTurn = o;
	 */

	//return(BlankOXToPosition(theBlankOX,whosTurn));
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
	if (gStandardGame) {
		return 2;
	} else {
		return 1;
	}
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
	if (option == 1) {
		gStandardGame = 0;
	} else if (option == 2) {
		gStandardGame = 1;
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

POSITION getCanonicalPosition (POSITION position) {

	char board[9], invertedBoard[9], reversedSideBoard[9], temp;
	POSITION minPosHash = position, newHash = 0;
	int currentTurn = generic_hash_turn(position), reverseTurn = (currentTurn == 1 ? 2 : 1);
	int i, j;

	generic_hash_unhash(position, board);

	/*inverted board, where the order of pieces go from clockwise to counter-clockwise*/
	invertedBoard[0] = board[0];
	invertedBoard[1] = board[1];
	for (i = 2; i < 9; i++)
		invertedBoard[i] = board[10-i];
	//    printf("invertedBoard: %s", invertedBoard);
	newHash = generic_hash_hash(invertedBoard, currentTurn);
	if (newHash < minPosHash) minPosHash = newHash;

	/*rotate it*/
	for (i = 0; i < 8; i++) {
		temp = invertedBoard[1];
		for (j = 1; j < 8; j++)
			invertedBoard[j] = invertedBoard[j+1];
		invertedBoard[8] = temp;
		//	printf("invertedBoard rotated: %s", invertedBoard);
		newHash = generic_hash_hash(invertedBoard, currentTurn);
		if (newHash < minPosHash) minPosHash = newHash;
	}

	/*reversed board, with the pieces changed to one of its opposing side,
	 * and the turn changed to the other player*/
	for (i = 0; i < 9; i++) {
		if (board[i] != '_')
			reversedSideBoard[i] = (board[i] == 'x' ? 'o' : 'x');
		else
			reversedSideBoard[i] = '_';
	}
	//    printf("reversedSideBoard: %s", invertedBoard);
	newHash = generic_hash_hash(reversedSideBoard, reverseTurn);
	if (newHash < minPosHash) minPosHash = newHash;

	/*turn this board too because they lead to the same value*/
	/*rotate it*/
	for (i = 0; i < 8; i++) {
		temp = reversedSideBoard[1];
		for (j = 1; j < 8; j++)
			reversedSideBoard[j] = reversedSideBoard[j+1];
		reversedSideBoard[8] = temp;
		//printf("reversedSideBoard rotated: %s", invertedBoard);
		newHash = generic_hash_hash(reversedSideBoard, reverseTurn);
		if (newHash < minPosHash) minPosHash = newHash;
	}

	/*rotate current board*/
	for (i = 0; i < 8; i++) {
		temp = board[1];
		for (j = 1; j < 8; j++)
			board[j] = board[j+1];
		board[8] = temp;
		//printf("board rotated: %s", invertedBoard);
		newHash = generic_hash_hash(board, currentTurn);
		if (newHash < minPosHash) minPosHash = newHash;
	}

	return minPosHash;
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
