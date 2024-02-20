/************************************************************************
**
** NAME:        mlewthwaite.c
**
** DESCRIPTION: Lewthwaite's Game
**
** AUTHOR:      Cindy Song, Yuliya Sarkisyan
**
** DATE:        Feb 17, 2005
**
**************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/

#include "gamesman.h"

/*************************************************************************
**
** Game-specific constants
**
**************************************************************************/

CONST_STRING kGameName            = "Lewthwaite's Game";   /* The name of your game */
CONST_STRING kAuthorName          = "Cindy Song, Yuliya Sarkisyan";   /* Your name(s) */
CONST_STRING kDBName              = "lewthwaitesgame";   /* The name to store the database under */

BOOLEAN kPartizan            = TRUE;   /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN kGameSpecificMenu    = TRUE;   /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN kTieIsPossible       = FALSE;   /* TRUE if a tie is possible. FALSE if it is impossible.*/
BOOLEAN kLoopy               = TRUE;   /* TRUE if the game tree will have cycles (a rearranger style game). FALSE if it does not.*/

BOOLEAN kDebugMenu           = FALSE;   /* TRUE only when debugging. FALSE when on release. */
BOOLEAN kDebugDetermineValue = FALSE;   /* TRUE only when debugging. FALSE when on release. */

POSITION gNumberOfPositions   =  0; /* The number of total possible positions | If you are using our hash, this is given by the hash_init() function*/
POSITION gInitialPosition     =  0; /* The initial hashed position for your starting board */
POSITION kBadPosition         = -1; /* A position that will never be used */

/*************************************************************************
**
** Start of game-specific variables
**
**************************************************************************/

/*************************************************************************
**
** End of game-specific variables
**
**************************************************************************/

/*
 * Help strings that are pretty self-explanatory
 * Strings than span more than one line should have backslashes (\) at the end of the line.
 */

CONST_STRING kHelpGraphicInterface =
        "Not written yet";

// How to tell the computer WHICH MOVE I want?
CONST_STRING kHelpTextInterface    =
        "The text input of a move is composed of a direction (one of the \
following four letters: ijkl, corresponding \
to up, left, down, and right), and, if multiple piece moves are allowed, of \
the number of pieces to move. Please note, under regular rules, \
you can only move one piece at a time."                                                                                                                                                                                                                                                                         ;

// What do I do on MY TURN?
CONST_STRING kHelpOnYourTurn =
        "On your turn, you try to move one of your pieces \
into the available space. If you are playing under the regular rules, \
you can only move one piece at a time.  Therefore, choose \
one of your pieces on the left, right, top, or bottom \
of the available space to move into the space."                                                                                                                                                                                                                                                        ;

// std objective of lewth game
CONST_STRING kHelpStandardObjective =
        "The standard objective of Lewthwaite's game is to move your \
pieces so that your opponent cannot move any \
of his pieces on his turn."                                                                                                                      ;

//
CONST_STRING kHelpReverseObjective =
        "The reverse objective of Lewthwaite's game is to move your \
pieces so that after your opponent's turn, you cannot move any \
of your pieces."                                                                                                                                       ;

CONST_STRING kHelpTieOccursWhen =
        "A tie is not possible in this game.";

CONST_STRING kHelpExample = "";

/*************************************************************************
**
** #defines and structs
**
**************************************************************************/


/*************************************************************************
**
** Global Variables
**
*************************************************************************/
void* gGameSpecificTclInit = NULL;

/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/

POSITION GetCanonicalPosition(POSITION position);

int boardSize = 25;
int sideLength = 5;

BOOLEAN kSupportsSymmetries = TRUE;
int gSymmetryMatrix[8][25] = {
	{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24},
	{4,3,2,1,0,9,8,7,6,5,14,13,12,11,10,19,18,17,16,15,24,23,22,21,20},
	{20,15,10,5,0,21,16,11,6,1,22,17,12,7,2,23,18,13,8,3,24,19,14,9,4},
	{0,5,10,15,20,1,6,11,16,21,2,7,12,17,22,3,8,13,18,23,4,9,14,19,24},
	{24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0},
	{20,21,22,23,24,15,16,17,18,19,10,11,12,13,14,5,6,7,8,9,0,1,2,3,4},
	{4,9,14,19,24,3,8,13,18,23,2,7,12,17,22,1,6,11,16,21,0,5,10,15,20},
	{24,19,14,9,4,23,18,13,8,3,22,17,12,7,2,21,16,11,6,1,20,15,10,5,0}
};

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
	int pieces[] = {'W', 12, 12, 'B', 12, 12, '-', 1, 1, -1};
	gNumberOfPositions = generic_hash_init(boardSize, pieces, NULL, 0);
	gCanonicalPosition = GetCanonicalPosition;
	gInitialPosition = GetInitialPosition();
	gSymmetries = TRUE;
}

MOVELIST *GenerateMoves(POSITION position) {
	MOVELIST *moves = NULL;
	int player = generic_hash_turn(position);
	char piece = (player == 1) ? 'W' : 'B';
	char board[boardSize];
	generic_hash_unhash(position, board);

	for (int i = 0; i < boardSize; i++) {
		if (board[i] == '-') {
			if (i >= sideLength && board[i - sideLength] == piece) { //up
				moves = CreateMovelistNode(i - sideLength, moves);
			}
			if (i < boardSize - sideLength && board[i + sideLength] == piece) { //down
				moves = CreateMovelistNode(i + sideLength, moves);
			}
			if (i % sideLength != 0 && board[i - 1] == piece) { // left
				moves = CreateMovelistNode(i - 1, moves);
			}
			if (i % sideLength != sideLength - 1 && board[i + 1] == piece) { // right
				moves = CreateMovelistNode(i + 1, moves);
			}
		}
	}
	return moves;
}

POSITION DoMove(POSITION position, MOVE move) {
	char board[boardSize];
	generic_hash_unhash(position, board);
	int opponent = (generic_hash_turn(position) == 1) ? 2 : 1;
	for (int i = 0; i < boardSize; i++) {
		if (board[i] == '-') {
			board[i] = board[move];
			board[move] = '-';
			break;
		}
	}
	
	return generic_hash_hash(board, opponent);
}

POSITION GetCanonicalPosition(POSITION position) {
  // rotation/reflection/color symmetries
  char board[boardSize];
  char board2[boardSize];
  generic_hash_unhash(position, board);
  int player = generic_hash_turn(position);
  int opponent = (player == 1) ? 2 : 1;
  POSITION symmetricPosition = 0;
  int *t;
  int i, j;
  for (i = 1; i < 8; i++) {
    t = gSymmetryMatrix[i];
    for (j = 0; j < boardSize; j++) {
      board2[j] = board[t[j]];
    }
    symmetricPosition = generic_hash_hash(board2, player);
    if (symmetricPosition < position) {
      position = symmetricPosition;
    }
  }
  
  for (i = 0; i < boardSize; i++) {
    if (board[i] == 'W') {
      board[i] = 'B';
    } else if (board[i] == 'B') {
		board[i] = 'W';
	}
  }

  for (i = 0; i < 8; i++) {
    t = gSymmetryMatrix[i];
    for (j = 0; j < boardSize; j++) {
      board2[j] = board[t[j]];
    }
    symmetricPosition = generic_hash_hash(board2, opponent);
    if (symmetricPosition < position) {
      position = symmetricPosition;
    }
  }

  return position;
}

VALUE Primitive(POSITION position) {
	MOVELIST *moves = GenerateMoves(position);
	if (moves == NULL) {
		return gStandardGame ? lose : win;
	}
	SafeFree(moves);
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

void PrintPosition (POSITION position, STRING playersName, BOOLEAN usersTurn) {
	char board[boardSize];
	generic_hash_unhash(position, board);

	printf("\n         1  2  3  4  5     %c %c %c %c %c\n", board[0], board[1], board[2], board[3], board[4]);
	printf("         6  7  8  9 10     %c %c %c %c %c\n", board[5], board[6], board[7], board[8], board[9]);
	printf("LEGEND: 11 12 13 14 15     %c %c %c %c %c\n", board[10], board[11], board[12], board[13], board[14]);
	printf("        16 17 18 19 20     %c %c %c %c %c\n", board[15], board[16], board[17], board[18], board[19]);
	printf("        21 22 23 24 25     %c %c %c %c %c\n", board[20], board[21], board[22], board[23], board[24]);
	printf("%s\n", GetPrediction(position, playersName, usersTurn));
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

void PrintComputersMove (MOVE computersMove, STRING computersName) {
	printf("%s has moved %d\n", computersName, computersMove + 1);
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

void MoveToString(MOVE move, char *moveString) {
	move += 1; // one-index
	snprintf(moveString, 20, "%d", move);
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

USERINPUT GetAndPrintPlayersMove(POSITION position, MOVE *move, STRING playerName) {
	USERINPUT ret;
	do {
		printf("%8s's move [(u)ndo]/[<index>] :  ", playerName);
		ret = HandleDefaultTextInput(position, move, playerName);
		if (ret != Continue) {
			return ret;
		}
	} while (TRUE);
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
	return atoi(input) != 0;
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

MOVE ConvertTextInputToMove(STRING input) {
	return atoi(input) - 1;
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
		printf("\n\tp)\tMisere Play (%s)\n", gStandardGame ? "Off" : "On");
		printf("\n\n\tb)\t(B)ack = Return to previous activity.\n");
		printf("\n\nSelect an option: ");

		switch(GetMyChar()) {
		case 'M': case 'm':
			gStandardGame = !gStandardGame;
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

void SetTclCGameSpecificOptions (int options[]) {
	(void)options;
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

POSITION GetInitialPosition() {
	return generic_hash_hash("BWBWBWBWBWBW-WBWBWBWBWBWB", 1);
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

int NumberOfOptions() {
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

int getOption () {
	return gStandardGame ? 0 : 1;
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

void setOption(int option) {
	gStandardGame = (option == 0);
}

void DebugMenu() {}

POSITION StringToPosition(char *positionString) {
	int turn;
	char *board;
	if (ParseStandardOnelinePositionString(positionString, &turn, &board)) {
		return generic_hash_hash(board, turn);
	}
	return NULL_POSITION;
}

void PositionToAutoGUIString(POSITION position, char *autoguiPositionStringBuffer) {
	char board[boardSize + 1];
	generic_hash_unhash(position, board);
	board[boardSize] = '\0';
	AutoGUIMakePositionString(generic_hash_turn(position), board, autoguiPositionStringBuffer);
}

void MoveToAutoGUIString(POSITION position, MOVE move, char *autoguiMoveStringBuffer) {
  	char board[boardSize];
	generic_hash_unhash(position, board);
	for (int i = 0; i < boardSize; i++) {
		if (board[i] == '-') {
			AutoGUIMakeMoveButtonStringM(move, i, 'x', autoguiMoveStringBuffer);
			break;
		}
	}
}