
/*
 * The above lines will include the name and log of the last person
 * to commit this file to CVS
 */

/************************************************************************
**
** NAME:        MOOE.C
**
** DESCRIPTION: ODD OR EVEN
**
** AUTHOR:      PAI-HSIEN (PETER) YU
**
** DATE:        WHEN YOU START/FINISH
**
** UPDATE HIST: RECORD CHANGES YOU HAVE MADE SO THAT TEAMMATES KNOW
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

STRING kGameName            = "Odd or Even";   /* The name of your game */
STRING kAuthorName          = "Peter Yu";   /* Your name(s) */
STRING kDBName              = "ooe";   /* The name to store the database under */

void* gGameSpecificTclInit = NULL;

BOOLEAN kPartizan            = FALSE;   /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN kGameSpecificMenu    = FALSE;   /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN kTieIsPossible       = FALSE;   /* TRUE if a tie is possible. FALSE if it is impossible.*/
BOOLEAN kLoopy               = FALSE;   /* TRUE if the game tree will have cycles (a rearranger style game). FALSE if it does not.*/

BOOLEAN kDebugMenu           = TRUE;   /* TRUE only when debugging. FALSE when on release. */
BOOLEAN kDebugDetermineValue = TRUE;   /* TRUE only when debugging. FALSE when on release. */

POSITION gNumberOfPositions   =  3000000; /* The number of total possible positions | If you are using our hash, this is given by the hash_init() function*/
/* Don't know */
POSITION gInitialPosition     =  1150000; /* The initial hashed position for your starting board */
POSITION kBadPosition         = -1; /* A position that will never be used */

/*
 * Help strings that are pretty self-explanatory
 * Strings than span more than one line should have backslashes (\) at the end of the line.
 */

STRING kHelpGraphicInterface =
        "Not written yet";

STRING kHelpTextInterface    =
        "On your turn, type in the number 1, 2, or 3 and hit return. If at any point\n\
you have made a mistake, you can type u and hit return and the system will\n\
revert back to your most recent position."                                                                                                                                                                        ;

STRING kHelpOnYourTurn =
        "You can enter 1, 2, or 3 to indicate how many match(es) you want take off board.\
A running total of how many matches you and your opponents have will be kept.\
Keep in mind though that the objective is not to get the last match; instead, you would have to have\
an even number of matches to win"                                                                                                                                                                                                                                                                                   ;

STRING kHelpStandardObjective =
        "At the end game, to have an even number of matches.";

STRING kHelpReverseObjective =
        "At the end game, to have an odd number of matches. (i.e. to force your opponent to take a total of even number\
of matches."                                                                                                                          ;

STRING kHelpTieOccursWhen =
        "There cannot be a tie";

STRING kHelpExample =
        "";


/*************************************************************************
**
** #defines and structs
**
**************************************************************************/

#define FIRST_TURN 1
#define SECOND_TURN 2

/*************************************************************************
**
** Global Variables
**
*************************************************************************/

const int MULTIPLE = 100;
int MAXMOVE = 3; /* Maximum matches that is allowed to be taken off the board */
int currentTurn = FIRST_TURN; /* current turn for player */

/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/

int playersTurn(POSITION position);
BOOLEAN gameOver(POSITION position);
int numberOfMatches(POSITION position);
int firstPlayerMatches(POSITION position);
int secondPlayerMatches(POSITION position);
int power(int number, int pow);

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

/* Move is in form of 103, where 1 is the player's turn and 3 is the matches taken off */
MOVELIST *GenerateMoves (POSITION position)
{
	MOVELIST *moves = NULL;
	int matchCount = numberOfMatches(position);
	int whoseTurn = playersTurn(position);
	int moveCount = 1; /* Minimum move is 1 */
	if(matchCount == 0) {
		moves = CreateMovelistNode(whoseTurn*MULTIPLE, moves);
	}
	else
	if(whoseTurn == FIRST_TURN) {
		while(matchCount > 0 && moveCount <= MAXMOVE) {
			moves = CreateMovelistNode(FIRST_TURN*MULTIPLE + moveCount, moves);
			--matchCount;
			++moveCount;
		}
	}
	else {
		while(matchCount > 0 && moveCount <= MAXMOVE) {
			moves = CreateMovelistNode(SECOND_TURN*MULTIPLE + moveCount, moves);
			--matchCount;
			++moveCount;
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
	int whoseTurn = move / MULTIPLE;
	int matches = move % MULTIPLE;
	int addition;
	if(whoseTurn == FIRST_TURN) {
		addition = matches * MULTIPLE;
		matches = matches * power(MULTIPLE, 2);
		/*if(matchesLeft - matches <= 0)
		        currentTurn = FIRST_TURN;
		   else*/
		currentTurn = SECOND_TURN;
		return (position + power(MULTIPLE, 3) - matches + addition);
		/* Add 1000000 to get second player turn*/
	}
	else {
		/*FOR DEBUG ONLY */

		/*FOR DEBUG ONLY */
		addition = matches;
		matches = matches * power(MULTIPLE, 2);
		currentTurn = FIRST_TURN; /* What about undo? */
		return (position - power(MULTIPLE, 3) - matches + addition);
		/* Subtract 1000000 to get first player turn*/
	}
	printf("Current Turn: %d", currentTurn);
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

/* Actually there is a tie... If there are even matches, even though it doesn't make sense */
VALUE Primitive (POSITION position)
{

	if(gameOver(position)) {
		int whoseTurn = playersTurn(position);
		int firstMatches = firstPlayerMatches(position);
		int secondMatches = secondPlayerMatches(position);
		if(whoseTurn == FIRST_TURN) {
			if((firstMatches % 2) != 0)
				return lose;
			else
				return win;
		}
		else {
			if((secondMatches % 2) != 0)
				return lose;
			else
				return win;
		}
	}
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
	int i;
	int currentMatches = numberOfMatches(position);
	printf("\n");
	printf("\n");
	printf(" ");
	for(i = 0; i < currentMatches; i++) {
		printf("O ");
	}
	printf("\n");
	for(i = 0; i < currentMatches; i++) {
		printf("/ ");
	}
	printf("\n");
	printf("\n");
	printf("The number of matches currently on the board: %d \n", currentMatches);
	printf("The number of matches first player has is: %d \n", firstPlayerMatches(position));
	printf("The number of matches second player has is: %d \n", secondPlayerMatches(position));
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
	printf("%s just removed %d matches from the table.\n", computersName, computersMove % MULTIPLE);
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
	STRING move = (STRING) SafeMalloc(5);
	/* Not quitesure whether this is only for player's move, but... */
	sprintf(move, "[%d]", theMove % 100);
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
		/***********************************************************
		* CHANGE THE LINE BELOW TO MATCH YOUR MOVE FORMAT
		***********************************************************/
		printf("%8s's move [(u)ndo/1/2/3] : ", playersName);

		input = HandleDefaultTextInput(position, move, playersName);

		if (input != Continue)
			return input;
	}

	/* NOTREACHED */
	return Continue;
}


/************************************************************************
**
** NAME:        ValidTex\tInput
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

	return (input[0] <= (MAXMOVE + '0') && input[0] >= '1');
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
	return (currentTurn*MULTIPLE + (input[0] - '0'));
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
	int totalMatches; /* first and second Matches probably should not be changed */
	printf("Please input the total number of matches: ");
	scanf("%d",&totalMatches);
	printf("Total matches is: %d \n", totalMatches);
	currentTurn = FIRST_TURN;
	return FIRST_TURN*power(MULTIPLE, 3) + totalMatches*power(MULTIPLE, 2);

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

	return 1; /* Currently only have the option of changing the number of matches */
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
	return 1; /* ???? */
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
	int maxMoves;
	printf("Please enter the number of matches you allow to be taken off board each time:");
	scanf("%d", &maxMoves);
	MAXMOVE = maxMoves;

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
	/* Check undo, because it doesn't work right now*/
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

/* Note: the number of representation we are temporarily supporting is in the form of 1234567,
   where 1 is the player's turn, 23 is the number of matches left, 45 is the matches first player has, and
   67 is the number of matches second player has. */

/* Might have to move the above definition variables up, depending on whether we need it in formal functions. */

/* Function player's turn accepts a current position and determine whose turn is it right now.
   Arguments: position number */
int playersTurn(POSITION position) {
	return (position / power(MULTIPLE, 3));
}

BOOLEAN gameOver(POSITION position) {
	return (numberOfMatches(position) == 0);
}

int numberOfMatches(POSITION position) {
	return (position / power(MULTIPLE, 2)) % MULTIPLE;
}

int firstPlayerMatches(POSITION position) {
	return (position / MULTIPLE) % MULTIPLE;
}

int secondPlayerMatches(POSITION position) {
	return position % MULTIPLE;
}

int power(int number, int pow) {
	int i;
	int answer = 1;
	for(i = 0; i < pow; i++) {
		answer = answer*number;
	}

	return answer;
}
POSITION StringToPosition(char* string) {
	int matches, first_player_matches, second_player_matches, position;
	int success = GetValue(string, "matches", GetInt, &matches) &&
                  GetValue(string, "first_player_matches", GetInt, &first_player_matches) &&
                  GetValue(string, "second_player_matches", GetInt, &second_player_matches) &&
                  GetValue(string, "position", GetInt, &position);
    if(success){
    	return position;
    } else {
    	return INVALID_POSITION;
    }    
}


char* PositionToString(POSITION position) {
	// char* emptyBoard = (char *) SafeMalloc(1);
	int matches = numberOfMatches(position);
	int first_player_matches = firstPlayerMatches(position);
	int second_player_matches = secondPlayerMatches(position);
	return MakeBoardString("", "matches", StrFromI(matches), "first_player_matches", StrFromI(first_player_matches), 
		"second_player_matches", StrFromI(second_player_matches), "position", StrFromI(position), "");
}

char * PositionToEndData(POSITION pos) {
	return NULL;
}
