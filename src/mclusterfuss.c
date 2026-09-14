/************************************************************************
**
** NAME:        mclusterfuss.c
**
** DESCRIPTION: Clusterfuss
**
** AUTHOR:      Kai Chen, Matthew Lee
**
** DATE:        Started 09/14/2026
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

CONST_STRING kGameName            = "Clusterfuss";
CONST_STRING kAuthorName          = "Kai Chen, Matthew Lee";
CONST_STRING kDBName              = "clusterfuss";   /* The name to store the database under */

BOOLEAN kPartizan            = TRUE;
BOOLEAN kGameSpecificMenu    = TRUE;   /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN kTieIsPossible       = TRUE;   /* TODO: the rule sheet doesn't spell out a tie condition explicitly
                                           (only that a player with no legal move is skipped). Revisit once
                                           GenerateMoves()/Primitive() are implemented. */
BOOLEAN kLoopy               = FALSE;  /* Every legal move is a capture (MOVES / ENEMY-ONLY GROUP REMOVAL),
                                           so the number of checkers on the board strictly decreases each
                                           turn and no position can ever repeat. Revisit if that turns out
                                           to be wrong. */

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

CONST_STRING kHelpGraphicInterface =
        "No graphic interface yet.";

CONST_STRING kHelpTextInterface =
        "The board is a square with columns labeled by letter and rows labeled by number. \n\
A position is referenced with a letter first and then a number.  A move consists \n\
of two positions: the starting position followed by the ending position.  \n\
For example, the bottom left square of the board is a1.  If you want to move a piece \n\
from a1 to a2, the format of the move is a1a2."                                                                                                                                                                                                                                                                                                                                                          ;

CONST_STRING kHelpOnYourTurn =
        "On your turn, pick one of your own checkers and move it onto an orthogonally \n\
adjacent square (up, down, left, or right) that is occupied by a checker, capturing \n\
it.  You cannot move onto an empty square or move diagonally.  You may capture a \n\
friendly checker or an enemy checker, but after your move your own checkers must \n\
form exactly one connected group (checkers are connected if they touch horizontally \n\
or vertically; diagonal touches don't count).  If your move detaches one or more \n\
groups made up entirely of enemy checkers, those groups are immediately removed from \n\
the board.  If you have no legal move, your turn is skipped."                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          ;

CONST_STRING kHelpStandardObjective =
        "The objective of the game is to remove all of your opponent's checkers from \n\
the board."                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 ;

CONST_STRING kHelpReverseObjective =
        "TODO: Clusterfuss has no misere variant defined in the original rules.";

// A tie occurs when is already displayed above this
CONST_STRING kHelpTieOccursWhen =
        "TODO: tie conditions have not been determined yet.";

CONST_STRING kHelpExample =
        "TODO: add a worked example once GenerateMoves, DoMove, and Primitive are implemented.";



/*************************************************************************
**
** #defines and structs
**
**************************************************************************/
#define PLAYERRED  1
#define PLAYERBLUE 2

#define BLANK ' '
#define RED   'R'
#define BLUE  'B'

/* Clusterfuss moves are orthogonal only (MOVES: "immediately above, below,
   to the right, or to the left") -- no diagonals. */
typedef enum possibleDirections {
	UP, RIGHT, DOWN, LEFT
} Direction;


/*************************************************************************
**
** Global Variables
**
*************************************************************************/
// length is left to right, height is up to down
// NOTE: Clusterfuss is played on a SQUARE board ("of any size"), so keep
// gBoardLength == gBoardHeight.
int gBoardLength = 4;
int gBoardHeight = 4;
int gBoardSize = 16;
char* gBoard;


/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/
void addPieceMoves(int boardSquare, int playerTurn, MOVELIST **moves);
void addMovesInDirection(Direction direction, int boardSquare, int playerTurn, MOVELIST **moves);
BOOLEAN onEdge(Direction direction, int boardSquare);
BOOLEAN isConnected(int boardSquare1, int boardSquare2);
Direction oppositeDirection(Direction direction);
POSITION setInitialPosition();
int numPiecesLeft(char color);
void moveUnhash(MOVE move, char *moveString);
MOVE moveHash(STRING input);
int goInDirection(Direction direction);
void InitializeHelpStrings();

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

	/* TODO: tighten these bounds once setInitialPosition() encodes the
	   exact starting layout from Figure 1 of the rule sheet. For now every
	   square may independently be BLANK, RED, or BLUE, since captures can
	   remove any number of checkers of either color as the game goes on. */
	int pieces[] = {BLANK, 0, gBoardSize,
		        RED,   0, gBoardSize,
		        BLUE,  0, gBoardSize, -1};

	gNumberOfPositions = generic_hash_init(gBoardSize, pieces, NULL, 0);
	gBoard = (char*)SafeMalloc(sizeof(char) * (gBoardSize+1));
	gInitialPosition = setInitialPosition();

	int rotations[1] = {90};
	int reflections[1] = {90};
	if (gBoardHeight == gBoardLength)
		generic_hash_init_sym(0, gBoardHeight, gBoardLength, reflections, 1, rotations, 1, 0);
	else
		generic_hash_init_sym(0, gBoardHeight, gBoardLength, reflections, 1, NULL, 0, 0);
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
void InitializeHelpStrings ()
{

	kHelpGraphicInterface =
	        "No graphic interface yet.";

	kHelpTextInterface =
	        "The board is a square with columns labeled by letter and rows labeled by number. \n\
A position is referenced with a letter first and then a number.  A move consists \n\
of two positions: the starting position followed by the ending position.  \n\
For example, the bottom left square of the board is a1.  If you want to move a piece \n\
from a1 to a2, the format of the move is a1a2."                                                                                                                                                                                                                                                                                                                                                                  ;

	kHelpOnYourTurn =
	        "On your turn, pick one of your own checkers and move it onto an orthogonally \n\
adjacent square (up, down, left, or right) that is occupied by a checker, capturing \n\
it.  You cannot move onto an empty square or move diagonally.  You may capture a \n\
friendly checker or an enemy checker, but after your move your own checkers must \n\
form exactly one connected group (checkers are connected if they touch horizontally \n\
or vertically; diagonal touches don't count).  If your move detaches one or more \n\
groups made up entirely of enemy checkers, those groups are immediately removed from \n\
the board.  If you have no legal move, your turn is skipped."                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  ;

	kHelpStandardObjective =
	        "The objective of the game is to remove all of your opponent's checkers from \n\
the board."                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         ;

	kHelpReverseObjective =
	        "TODO: Clusterfuss has no misere variant defined in the original rules.";

	// A tie occurs when is already displayed above this
	kHelpTieOccursWhen =
	        "TODO: tie conditions have not been determined yet.";

	kHelpExample =
	        "TODO: add a worked example once GenerateMoves, DoMove, and Primitive are implemented.";
}


/************************************************************************
**
** NAME:        GenerateMoves
**
** DESCRIPTION: Creates a linked list of every move that can be reached
**              from this position. Returns a pointer to the head of the
**              linked list.
**
**              TODO: a move is only legal if it (1) is an orthogonal
**              capture onto an adjacent occupied square (MOVES), and (2)
**              leaves the moving player's checkers in exactly one
**              connected group afterward (MOVE RESTRICTIONS). Neither
**              rule is enforced yet -- see addMovesInDirection().
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
	int i;
	int playerTurn = generic_hash_turn(position);
	char playerColor = (playerTurn == PLAYERRED ? RED : BLUE);
	MOVELIST *moves = NULL;
	generic_hash_unhash(position, gBoard);

	for (i = 0; i < gBoardSize; i++) {
		if (gBoard[i] == playerColor)
			addPieceMoves(i, playerTurn, &moves);
	}

	return moves;
}


/************************************************************************
**
** NAME:        DoMove
**
** DESCRIPTION: Applies the move to the position.
**
**              TODO: not yet implemented. `move` decodes to start*100+end
**              (see moveHash/moveUnhash). Still needs to:
**                1. Move the checker from `start` onto `end`, capturing
**                   whatever was there (friendly or enemy) per MOVES.
**                2. Per ENEMY-ONLY GROUP REMOVAL, find every connected
**                   group left on the board (orthogonal adjacency only --
**                   see isConnected()) and remove any group that is made
**                   up entirely of enemy checkers and was just detached
**                   by this move.
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
	int playerTurn = generic_hash_turn(position);
	generic_hash_unhash(position, gBoard);

	(void) move; /* TODO: see function description above */

	playerTurn = (playerTurn == PLAYERRED) ? PLAYERBLUE : PLAYERRED;

	return generic_hash_hash(gBoard, playerTurn);
}


/************************************************************************
**
** NAME:        Primitive
**
** DESCRIPTION: Returns the value of a position if it fulfills certain
**              'primitive' constraints.
**
**              TODO: OBJECT is "remove all enemy checkers from the
**              board", so this likely wants something like:
**                generic_hash_unhash(position, gBoard);
**                int playerTurn = generic_hash_turn(position);
**                if (playerTurn == PLAYERRED  && numPiecesLeft(BLUE) == 0) return lose/win;
**                if (playerTurn == PLAYERBLUE && numPiecesLeft(RED)  == 0) return lose/win;
**              Also still needs to decide how "no legal move" is handled --
**              the rules treat it as a skipped turn, not a loss, so
**              GenerateMoves() returning NULL may need to become a forced
**              pass move rather than a primitive here.
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
	(void) position; /* TODO: see function description above */

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
	int j;
	generic_hash_unhash(position, gBoard);
	int playerTurn = generic_hash_turn(position);


	printf("\n\n     ");
	for (i = 0; i < gBoardLength; i++) {
		printf("+---");
	}
	printf("+\n");

	for (i = 0; i < gBoardHeight; i++) {
		printf("   %d |", gBoardHeight-i);
		for (j = 0; j < gBoardLength; j++) {
			printf(" %c |", gBoard[gBoardLength*i+j]);
		}
		printf("\n");
		printf("     ");
		for (j = 0; j < gBoardLength; j++) {
			printf("+---");
		}
		printf("+");
		printf("\n");
	}
	printf("      ");
	for (i = 0; i < gBoardLength; i++) {
		printf(" %c  ", 'a'+i);
	}
	printf("\n\n");

	printf("   It is %s's turn\n", playersName);
	if (playerTurn == PLAYERRED)
		printf("   %s is playing Red (%c)\n", playersName, RED);
	else
		printf("   %s is playing Blue (%c)\n\n", playersName, BLUE);

	printf("%s\n\n", GetPrediction(position,playersName,usersTurn));

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

void MoveToString (MOVE move, char *moveStringBuffer) {
	moveUnhash(move, moveStringBuffer);
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
	char str[20];
	MoveToString(move, str);
	printf("%s", str);
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
	char str[20];
	MoveToString(computersMove, str);
	printf("%s's move: %s", computersName, str);
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

	for (;; ) {
		/***********************************************************
		* CHANGE THE LINE BELOW TO MATCH YOUR MOVE FORMAT
		***********************************************************/
		printf("%8s's move [(undo)/(<coli><rowi><colf><rowf>)] : ", playersName);

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
	/* input should be like a2c2 */

	if ((input[0] >= 'a' && input[0] < 'a' + gBoardLength) &&
	    (input[1] > '0' && input[1] < '1' + gBoardHeight) &&
	    (input[2] >= 'a' && input[0] < 'a' + gBoardLength) &&
	    (input[3] > '0' && input[3] < '1' + gBoardHeight))
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
	return moveHash(input);
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
	int temp;

	while(TRUE) {
		printf("\n");
		printf("\t----- Game Specific Options for Clusterfuss -----\n\n");
		printf("\tr) \t(R)esize board. Current board is %dx%d.\n", gBoardLength, gBoardHeight);
		printf("\tb) \t(B)ack to previous menu\n");
		printf("\n\tq) \t(Q)uit\n\n");
		printf("Select an option: ");

		switch(GetMyChar()) {
		case 'r':
		case 'R':
			/* Clusterfuss is played on a square board, so length == height. */
			printf("Input desired board size[3-6]: ");
			temp = GetMyUInt();
			while(temp < 3 || temp > 6) {
				printf("Invalid size.  Please enter a size from 3 to 6.\n");
				temp = GetMyUInt();
			}
			gBoardLength = temp;
			gBoardHeight = temp;
			gBoardSize = gBoardLength*gBoardHeight;
			SafeFree(gBoard);
			break;
		case 'q':
		case 'Q':
			printf("\n");
			ExitStageRight();
			break;
		case 'b':
		case 'B':
			return;
		default:
			printf("Sorry, I don't know that option.  Try another.\n");
			break;
		}
	}

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
	(void)options;
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
	/* TODO: Clusterfuss (as specified in the rule sheet) has no rule
	   variants beyond board size, which is already handled by
	   GameSpecificMenu()/gBoardLength rather than this option hash.
	   Revisit if a real variant is added later. */
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

	return 1;
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

	(void) option; /* TODO: no variants defined yet, see NumberOfOptions() */
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

/************************************************************************
** GROUPS: "checkers interconnected horizontally or vertically, or both.
** Diagonal adjacencies are irrelevant in Clusterfuss." Returns TRUE if
** boardSquare2 is orthogonally adjacent to boardSquare1. Does not look at
** what's actually on either square -- callers are expected to check that.
***********************************************************************/
BOOLEAN isConnected(int boardSquare1, int boardSquare2)
{
	Direction d;

	for (d = UP; d <= LEFT; d++) {
		if ((!onEdge(d, boardSquare1)) &&
		    (boardSquare1 + goInDirection(d) == boardSquare2))
			return TRUE;
	}

	return FALSE;
}

/************************************************************************
** adds moves to the movelist given a square on the board that has a
** piece on it. Called by GenerateMoves so assume position has been
** unhashed and is stored correctly in gBoard.
***********************************************************************/
void addPieceMoves(int boardSquare, int playerTurn, MOVELIST **moves)
{
	Direction d;

	for (d = UP; d <= LEFT; d++) {
		addMovesInDirection(d, boardSquare, playerTurn, moves);
	}
}

/************************************************************************
** Adds a move to the movelist if capturing in the given direction from
** boardSquare is legal. Called by addPieceMoves.
**
** TODO: not yet implemented. Per MOVES / GROUPS / MOVE RESTRICTIONS /
** ENEMY-ONLY GROUP REMOVAL, this needs to:
**   1. Bail out if onEdge(direction, boardSquare) or the adjacent square
**      is BLANK (a capture requires a checker to land on).
**   2. Simulate the capture (mover's checker replaces whatever was on
**      the target square).
**   3. Reject the move unless, afterward, the mover's own-colored
**      checkers form exactly one connected group (use isConnected() with
**      a flood fill/union-find over orthogonal neighbors).
**   4. If legal, push it:
**        *moves = CreateMovelistNode(boardSquare*100 + targetSquare, *moves);
***********************************************************************/
void addMovesInDirection(Direction direction, int boardSquare, int playerTurn, MOVELIST **moves)
{
	(void) direction;
	(void) boardSquare;
	(void) playerTurn;
	(void) moves;
}

/*************************************************************************
** I'm defining an Edge a bit differently than normal.  An edge depends
** on which direction you're looking at.  For example, if you're looking
** right and you're on the top most row, you're still not on an edge.
** You're only on an edge if you cannot go any further in the specified
** direction.  If you're going right, you're only on an edge if you're
** on the rightmost column of the board.
************************************************************************/
BOOLEAN onEdge(Direction direction, int boardSquare)
{
	switch (direction)
	{
	case UP:
		if (boardSquare >= 0 && boardSquare < gBoardLength)
			return TRUE;
		else
			return FALSE;
	case RIGHT:
		if (boardSquare % gBoardLength == gBoardLength-1)
			return TRUE;
		else
			return FALSE;
	case DOWN:
		if (boardSquare >= gBoardSize-gBoardLength && boardSquare < gBoardSize)
			return TRUE;
		else
			return FALSE;
	case LEFT:
		if (boardSquare % gBoardLength == 0)
			return TRUE;
		else
			return FALSE;
	}

	/* This return should never happen but I put this in to get rid of a warning */
	return FALSE;
}

Direction oppositeDirection(Direction direction)
{
	if (direction == UP)
		return DOWN;
	else if (direction == DOWN)
		return UP;
	else if (direction == RIGHT)
		return LEFT;
	else //(direction == LEFT)
		return RIGHT;
}


/**************************************************************************
** returns initial position using gBoardSize and gBoardLength.  Called from
** InitializeGame().
**
** TODO: not yet implemented. Per the INTRODUCTION, Clusterfuss starts with
** the board COMPLETELY full (no BLANK squares at all) with Red and Blue
** checkers interspersed as shown in Figure 1 of the rule sheet -- go check
** Clusterfuss_rules.pdf for the exact layout before filling this in.
** Placeholder below just clears the board so the file compiles/links.
*************************************************************************/
POSITION setInitialPosition()
{
	int i;

	for (i = 0; i < gBoardSize; i++) {
		gBoard[i] = BLANK;
	}
	gBoard[gBoardSize] = '\0';

	return generic_hash_hash(gBoard, PLAYERRED);
}

/***************************************************************************
** Given a color, returns the number of pieces of that color currently on
** gBoard. Assumes gBoard has already been populated (e.g. via
** generic_hash_unhash).
**************************************************************************/
int numPiecesLeft(char color)
{
	int i, piecesSoFar;
	piecesSoFar = 0;

	for (i = 0; i < gBoardSize; i++) {
		if (gBoard[i] == color)
			piecesSoFar++;
	}

	return piecesSoFar;
}

/*****************************************************************************
** Takes in a direction and returns an integer that gets added to the
** numerical representation of a square to get a new square which corresponds
** to moving that piece in that direction
****************************************************************************/
int goInDirection(Direction direction)
{

	switch(direction) {
	case UP: return -gBoardLength;
	case RIGHT: return 1;
	case DOWN: return gBoardLength;
	case LEFT: return -1;
	default: return 0;
	}
}

MOVE moveHash(STRING input)
{
	MOVE move;
	int startSquare, endSquare;

	// input[0] is the letter and input[1] is the number
	startSquare = input[0] - 'a' + (gBoardHeight - (input[1] - '0'))*gBoardLength;
	endSquare = input[2] - 'a' + (gBoardHeight - (input[3] - '0'))*gBoardLength;
	move = endSquare + startSquare*100;
	return move;
}

void moveUnhash(MOVE move, char *moveString)
{
	int startSquare, endSquare;
	char letter, number;

	endSquare = move % 100;
	startSquare = move/100;
	number = '0' + (gBoardHeight - startSquare / gBoardLength);
	letter = (startSquare % gBoardLength) + 'a';
	moveString[0] = letter;
	moveString[1] = number;
	number = '0' + (gBoardHeight - endSquare / gBoardLength);
	letter = (endSquare % gBoardLength) + 'a';
	moveString[2] = letter;
	moveString[3] = number;
	moveString[4] = '\0';
}


/************************************************************************
** Changelog
**
** (No history yet -- this file was adapted from mloa.c (Lines of Action,
** by Albert Shau) as a starting template for Clusterfuss on 09/14/2026.)
**
************************************************************************/

POSITION StringToPosition(char *positionString) {
	(void) positionString;
	return NULL_POSITION;
}

void PositionToAutoGUIString(POSITION position, char *autoguiPositionStringBuffer) {
	(void) position;
	(void) autoguiPositionStringBuffer;
}

void MoveToAutoGUIString(POSITION position, MOVE move, char *autoguiMoveStringBuffer) {
	(void) position;
	(void) move;
	(void) autoguiMoveStringBuffer;
}
