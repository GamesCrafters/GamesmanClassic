// $id$
// $log$


/*
 * The above lines will include the name and log of the last person
 * to commit this file to CVS
 */

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
** UPDATE HIST: 2/19 - Cindy - Changed constants;
**			       Primitive and two helpers;
**			       PrintPosition
**              2/22 - Yuliya - DoMove and two Primitive helpers
**                              Added some constants
**                              Moved helpers to the bottom of file to the
**                                 space allocated for them
**                     NOTE: All functions are tentative until we come up
**                           with a final representation of position
**              2/25 - Cindy - Fixed PrintPosition
**			     - Fixed Primitive
**		       NOTE: Primitive will work with the variation of
**			     the game
**              2/26 - Yuliya - Changed game type to LOOPY
**                            - Adjusted constants a little
**                            - Did hash intialization in initializeGame()
**                            - Fixed DoMove to handle sliding more than
**                              one square at a time
**                            - Fixed GetSpace, GetPlayer and added one
**                              more helper
**                            - Wrote functions for hashing and unhashing
**                              moves
**		2/26 - Cindy - PrintPosition uses generic hash
**			     - Fixed DoMove minor errors
**			     - Add rule variation constant
**			     - Implemented GenerateMoves
**			     - Fixed and added helpers
**              2/27 - Yuliya - Added some more rule variation constants
**                              See comment
**                            - Finished implementing IntializeGame
**                            - Implemented PrintComputersMove, PrintMove
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
#include "hash.h"


/*************************************************************************
**
** Game-specific constants
**
**************************************************************************/

STRING kGameName            = "Lewthwaite's Game";   /* The name of your game */
STRING kAuthorName          = "Cindy Song, Yuliya Sarkisyan";   /* Your name(s) */
STRING kDBName              = "lewth";   /* The name to store the database under */

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

/* constants for board size */
int boardWidth                = 3;
int boardHeight               = 5;

int numXpieces                = 7;
int numOpieces                = 7;

/* other constants */

#define X      1
#define O      2
#define space -1

#define up     0
#define down   1
#define left   2
#define right  3

BOOLEAN multiplePieceMoves    = FALSE;
BOOLEAN gMisereGame           = FALSE;

BOOLEAN gCustomBoard          = FALSE;
int gFirstToMove              = X;
BOOLEAN positionInitialized   = FALSE;

BOOLEAN detailedDebug         = FALSE;

/*************************************************************************
**
** End of game-specific variables
**
**************************************************************************/

/*
 * Help strings that are pretty self-explanatory
 * Strings than span more than one line should have backslashes (\) at the end of the line.
 */

STRING kHelpGraphicInterface =
        "Not written yet";

// How to tell the computer WHICH MOVE I want?
STRING kHelpTextInterface    =
        "The text input of a move is composed of a direction (one of the \
following four letters: ijkl, corresponding \
to up, left, down, and right), and, if multiple piece moves are allowed, of \
the number of pieces to move. Please note, under regular rules, \
you can only move one piece at a time."                                                                                                                                                                                                                                                                         ;

// What do I do on MY TURN?
STRING kHelpOnYourTurn =
        "On your turn, you try to move one of your pieces \
into the available space. If you are playing under the regular rules, \
you can only move one piece at a time.  Therefore, choose \
one of your pieces on the left, right, top, or bottom \
of the available space to move into the space."                                                                                                                                                                                                                                                        ;

// std objective of lewth game
STRING kHelpStandardObjective =
        "The standard objective of Lewthwaite's game is to move your \
pieces so that your opponent cannot move any \
of his pieces on his turn."                                                                                                                      ;

//
STRING kHelpReverseObjective =
        "The reverse objective of Lewthwaite's game is to move your \
pieces so that after your opponent's turn, you cannot move any \
of your pieces."                                                                                                                                       ;

STRING kHelpTieOccursWhen =
        "A tie is not possible in this game.";

STRING kHelpExample = "                DIRECTION KEY\n\n\
                    (up)\n\
                     i\n\
           (left) j     l (right)\n\
                     k\n\
                   (down)\n\n\n\
                CURRENT BOARD\n\n\
                O X O\n\
                X - X\n\
                O X O\n\n\n\
                X TO MOVE\n\n\n\
Computer: (Computer should Lose in 8)\n\
Computer has moved l\n\n\n\
                DIRECTION KEY\n\n\
                    (up)\n\
                     i\n\
           (left) j     l (right)\n\
                     k\n\
                   (down)\n\n\n\
                CURRENT BOARD\n\n\
                O X O\n\
                - X X\n\
                O X O\n\n\n\
                O TO MOVE\n\n\n\
Player: (Player should Win in 7)\n\
Player's move [(u)ndo/(direction) : direction = i,j,k,l] : k\n\n\
                DIRECTION KEY\n\n\
                    (up)\n\
                     i\n\
           (left) j     l (right)\n\
                     k\n\
                   (down)\n\n\n\
                CURRENT BOARD\n\n\
                - X O\n\
                O X X\n\
                O X O\n\n\n\
                X TO MOVE\n\n\n\
Computer: (Computer should Lose in 6)\n\
Computer has moved j\n\n\n\
                DIRECTION KEY\n\n\
                    (up)\n\
                     i\n\
           (left) j     l (right)\n\
                     k\n\
                   (down)\n\n\n\
                CURRENT BOARD\n\n\
                X - O\n\
                O X X\n\
                O X O\n\n\n\
                O TO MOVE\n\n\n\
Player: (Player should Win in 5)\n\
Player's move [(u)ndo/(direction) : direction = i,j,k,l] : j\n\n\
                DIRECTION KEY\n\n\
                    (up)\n\
                     i\n\
           (left) j     l (right)\n\
                     k\n\
                   (down)\n\n\n\
                CURRENT BOARD\n\n\
                X O -\n\
                O X X\n\
                O X O\n\n\n\
                X TO MOVE\n\n\n\
Computer: (Computer should Lose in 4)\n\
Computer has moved i\n\n\n\
                DIRECTION KEY\n\n\
                    (up)\n\
                     i\n\
           (left) j     l (right)\n\
                     k\n\
                   (down)\n\n\n\
                CURRENT BOARD\n\n\
                X O X\n\
                O X -\n\
                O X O\n\n\n\
                O TO MOVE\n\n\n\
Player: (Player should Win in 3)\n\
Player's move [(u)ndo/(direction) : direction = i,j,k,l] : i\n\n\
                DIRECTION KEY\n\n\
                    (up)\n\
                     i\n\
           (left) j     l (right)\n\
                     k\n\
                   (down)\n\n\n\
                CURRENT BOARD\n\n\
                X O X\n\
                O X O\n\
                O X -\n\n\n\
                X TO MOVE\n\n\n\
Computer: (Computer should Lose in 2)\n\
Computer has moved l\n\n\n\
                DIRECTION KEY\n\n\
                    (up)\n\
                     i\n\
           (left) j     l (right)\n\
                     k\n\
                   (down)\n\n\n\
                CURRENT BOARD\n\n\
                X O X\n\
                O X O\n\
                O - X\n\n\n\
                O TO MOVE\n\n\n\
Player: (Player should Win in 1)\n\
Player's move [(u)ndo/(direction) : direction = i,j,k,l] : l\n\n\
                DIRECTION KEY\n\n\
                    (up)\n\
                     i\n\
           (left) j     l (right)\n\
                     k\n\
                   (down)\n\n\n\
                CURRENT BOARD\n\n\
                X O X\n\
                O X O\n\
                - O X\n\n\n\
                X TO MOVE\n\n\n\
Computer: (Computer should Lose in 0)\n\n\
Excellent! You won!\n";


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

/* Internal */
void setDefaultInitialPosition();
int GetSpace(char*);
int GetPlayer(char*, int, int, int);
int otherPlayer(int);
char getPiece(int);
int ModifyOffset(int, int);
int validCellToCheck(int, int, int);
void printBoard(char*);
MOVE hashMove(int, int);
int getDirection(MOVE);
int getNumPieces(MOVE);
MOVELIST* CreateMovelistNode(MOVE, MOVELIST*);

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
	int boardSize = boardWidth*boardHeight;
	int pieces[] = {'X', numXpieces, numXpieces,
		        'O', numOpieces, numOpieces,
		        '-', 1, 1,
		        -1};

	gNumberOfPositions =
	        generic_hash_init(boardSize, pieces, NULL, 0);

	if (boardHeight == boardWidth) {
		int reflections[4] = {0,45,90,135};
		int rotations[3] = {90,180,270};
		generic_hash_init_sym(0, boardHeight, boardWidth, reflections, 4, rotations, 3, 0);
	}
	else {
		int reflections[2] = {0,90};
		int rotations[1] = {180};
		generic_hash_init_sym(0, boardHeight, boardWidth, reflections, 2, rotations, 1, 0);
	}

	if (!positionInitialized) {
		int i;
		char gBoard[boardSize];

		for (i=0; i<boardSize; i++) {
			if (i%2 == 0) {
				gBoard[i] = 'O';
			} else {
				gBoard[i] = 'X';
			}
		}

		if (gBoard[boardSize/2] == 'O') {
			gBoard[boardSize/2] = '-';
		} else {
			gBoard[boardSize/2 - boardWidth] = '-';
		}

		gInitialPosition = generic_hash_hash(gBoard, gFirstToMove);
		positionInitialized = TRUE;
	}

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
	if (detailedDebug) printf("GenerateMoves start\n");
	MOVELIST *moves = NULL;
	int spaceIndex, dir, numToMove;
	int player = generic_hash_turn(position);
	char posArray[boardWidth*boardHeight];
	generic_hash_unhash(position, posArray);

	/* Use CreateMovelistNode(move, next) to 'cons' together a linked list */

	spaceIndex = GetSpace(posArray);

	for (dir=0; dir<4; dir++) {
		if (validCellToCheck(1,dir,spaceIndex)
		    && GetPlayer(posArray, spaceIndex, 1, dir) == player) {

			moves = CreateMovelistNode(hashMove(dir, 1), moves);

			if (multiplePieceMoves) {
				numToMove = 2;
				while(validCellToCheck(numToMove,dir,spaceIndex)) {

					if (GetPlayer(posArray,spaceIndex,numToMove,dir) == player) {
						moves = CreateMovelistNode(hashMove(dir, numToMove), moves);
					}

					numToMove = numToMove+1;
				}
			}
		}
	}
	if (detailedDebug) printf("GenerateMoves end\n");
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
	if (detailedDebug) printf("DoMove start\n");
	char posArray[boardWidth*boardHeight];
	generic_hash_unhash(position, posArray);
	int whosMove = generic_hash_turn(position);
	int empty_space = GetSpace(posArray);

	int direction = getDirection(move);
	int numPieces = getNumPieces(move);

	int i;
	if (direction == up) {
		for (i=0; i<numPieces; i++) {
			posArray[empty_space+boardWidth*i] = posArray[empty_space+boardWidth*(i+1)];
		}
		posArray[empty_space+boardWidth*i] = getPiece(space);
	} else if (direction == down) {
		for (i=0; i<numPieces; i++) {
			posArray[empty_space-boardWidth*i] = posArray[empty_space-boardWidth*(i+1)];
		}
		posArray[empty_space-boardWidth*numPieces] = getPiece(space);
	} else if (direction == left) {
		for (i=0; i<numPieces; i++) {
			posArray[empty_space+i] = posArray[empty_space+(i+1)];
		}
		posArray[empty_space+numPieces] = getPiece(space);
	} else if (direction == right) {
		for (i=0; i<numPieces; i++) {
			posArray[empty_space-i] = posArray[empty_space-(i+1)];
		}
		posArray[empty_space-numPieces] = getPiece(space);
	} else {
		//error("Invalid Move");
	}

	if (detailedDebug) printf("new position %s\n", posArray);
	if (detailedDebug) printf("DoMove end\n");
	return generic_hash_hash(posArray, otherPlayer(whosMove));

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
	int spaceIndex,dir;
	int player;
	char posArray[boardWidth*boardHeight];
	generic_hash_unhash(position, posArray);
	spaceIndex = GetSpace(posArray);
	player = generic_hash_turn(position);

	/* Current player loses when all 4 cells around the space
	 * belong to the opponent
	 */
	for (dir=0; dir<4; dir++) {
		if (validCellToCheck(1, dir,spaceIndex)) {
			if (GetPlayer(posArray, spaceIndex, 1, dir) == player) {
				return undecided;
			}
		}
	}
	return (gMisereGame ? win : lose);
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
	if (detailedDebug) printf("PrintPosition start\n");
	printf("\n");
	STRING name = "";

	char board[boardWidth*boardHeight];
	generic_hash_unhash(position, board);

	printf("\t\tDIRECTION KEY\n\n");

	printf("\t            (up)\n");
	printf("\t             i\n");
	printf("\t   (left) j     l (right)\n");
	printf("\t             k\n");
	printf("\t           (down)\n");

	printf("\n\n");

	printf("\t\tCURRENT BOARD\n\n");
	printBoard(board);

	printf("\n\n");

	printf("\t\t%c TO MOVE", getPiece(generic_hash_turn(position)));

	printf("\n\n");

	if (usersTurn) {
		name = playersName;
	} else {
		name = "Computer";
	}
	printf("\n%s: %s\n", name, GetPrediction(position, playersName, usersTurn));
	if (detailedDebug) printf("PrintPosition end\n");

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
	printf("%s has moved ", computersName);
	PrintMove(computersMove);
	printf("\n\n");
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
	STRING move = (STRING) SafeMalloc(4);

	if (detailedDebug) printf("MoveToString start\n");

	char dirCh;

	int numPieces = getNumPieces(theMove);
	int direction = getDirection(theMove);

	if (direction == up) {
		dirCh  = 'i';
	} else if (direction == down) {
		dirCh = 'k';
	} else if (direction == left) {
		dirCh = 'j';
	} else if (direction == right) {
		dirCh = 'l';
	}

	if (multiplePieceMoves) {
		sprintf( move, "%c%d", dirCh, numPieces );
	} else {
		sprintf( move, "%c", dirCh );
	}
	if (detailedDebug) printf("MoveToString end\n");
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
	if (detailedDebug) printf("GetAndPrintPlayersMove start\n");
	USERINPUT input;
	USERINPUT HandleDefaultTextInput();

	for (;; ) {
		/***********************************************************
		* CHANGE THE LINE BELOW TO MATCH YOUR MOVE FORMAT
		***********************************************************/
		if (multiplePieceMoves) {
			printf("%s's move [(u)ndo/(xy) : x = direction(i,j,k,l), y = # of pieces] : ",
			       playersName);
		} else {
			printf("%s's move [(u)ndo/(direction) : direction = i,j,k,l] : ",
			       playersName);
		}

		input = HandleDefaultTextInput(position, move, playersName);
		if (detailedDebug) if (input == Continue) printf("invalid move\n");

		if (input != Continue) {
			if (detailedDebug) printf("GetAndPrintPlayersMove end\n");
			return input;
		}
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
	if (detailedDebug) printf("ValidTextInput start\n");
	char c = *input;
	int direction = 0;
	int numPieces = 0;

	if (c == 'i') {
		direction = up;
	} else if (c == 'k') {
		direction = down;
	} else if (c == 'j') {
		direction = left;
	} else if (c == 'l') {
		direction = right;
	} else {
		return FALSE;
	}

	if (multiplePieceMoves) {
		input++;
		while ((c = *input) != '\0') {
			if (c < '0' || c > '9') {
				return FALSE;
			}
			numPieces = (numPieces * 10) + (c - '0');
			input++;
		}
	}
	if (detailedDebug) printf("ValidTextInput end\n");
	return TRUE; //////////// NOT IMPLEMENTED!!!!!!!!!!!!1

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
	if (detailedDebug) printf("ConvertTextInputToMove start\n");
	char c = *input;
	int direction = 0;
	int numPieces = 1;

	if (c == 'i') {
		direction = up;
	} else if (c == 'k') {
		direction = down;
	} else if (c == 'j') {
		direction = left;
	} else if (c == 'l') {
		direction = right;
	} else {
		return FALSE;
	}

	if (multiplePieceMoves) {
		numPieces = 0;
		input++;
		while ((c = *input) != '\0') {
			if (c < '0' || c > '9') {
				return FALSE;
			}
			numPieces = (numPieces * 10) + (c - '0');
			input++;
		}
	}

	if (detailedDebug) printf("ConvertTextInputToMove end\n");
	return hashMove(direction, numPieces);
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
	char initialBoard[boardWidth*boardHeight];
	while (TRUE) {
		printf("\n\t----- Game-specific options for %s -----\n", kGameName);
		printf("\n\tBoard Options:\n\n");
		printf("\td)\tChange board (D)imension (%d,%d)\n", boardWidth, boardHeight);
		printf("\ti)\tChange (i)nitial position.  Current position is:\n");
		printf("\n");
		generic_hash_unhash(gInitialPosition, initialBoard);
		printBoard(initialBoard);
		printf("\n\tRule Options:\n\n");
		printf("\tp)\tMultiple (p)iece moves (%s)\n", multiplePieceMoves ? "On" : "Off");
		printf("\n\n\tb)\t(B)ack = Return to previous activity.\n");
		printf("\n\nSelect an option: ");

		switch(GetMyChar()) {
		case 'Q': case 'q':
			ExitStageRight();

		case 'H': case 'h':
			HelpMenus();
			break;

		case 'D': case 'd':
			printf("\nPlease enter a new width (3 or 5): ");
			boardWidth = GetMyInt();
			while (boardWidth != 3 && boardWidth != 5) {
				printf("Please enter a new width (3 or 5): ");
				boardWidth = GetMyInt();
			}

			printf("Please enter a new height (3 or 5): ");
			boardHeight = GetMyInt();
			while (boardHeight != 3 && boardHeight != 5) {
				printf("Please enter a new height (3 or 5): ");
				boardHeight = GetMyInt();
			}

			numOpieces = boardHeight*boardWidth/2;
			numXpieces = numOpieces;
			positionInitialized = FALSE;
			InitializeGame();
			break;

		case 'P': case 'p':
			multiplePieceMoves = !multiplePieceMoves;
			break;

		case 'I': case 'i':
			gInitialPosition = GetInitialPosition();
			positionInitialized = TRUE;
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
	int i,j, length;
	int numO = 0;
	int numX = 0;
	int boardSize = boardWidth*boardHeight;
	char row[boardSize],board[boardSize],c;
	BOOLEAN spaceExists = FALSE;

	i = 0;
	printf("\n");
	while(i < boardHeight) {
		printf("Please enter row %d without any spaces: ", i+1);
		GetMyStr(row, boardSize);

		length = strlen(row);

		if (length < boardWidth) {
			printf("\nToo few pieces.  Try again.\n");
			continue;
		} else if (length > boardWidth) {
			printf("\nToo many pieces.  Try again.\n");
			continue;
		}
		for (j=0; j<boardWidth; j++) {
			c = row[j];

			if (c == 'O') {
				numO++;
				board[j+i*boardWidth] = c;

			} else if (c == 'X') {
				numX++;
				board[j+i*boardWidth] = c;

			} else if (c == '-') {
				if (spaceExists) {
					printf("\nThe board already contains a space.  Try again.\n");
					continue;
				}
				spaceExists = TRUE;
				board[j+i*boardWidth] = c;

			} else {
				printf("\nUnknown piece %c.  Try again.\n", c);
				i--;
				break;
			}

		}
		i++;
	}

	if (!spaceExists) {
		printf("\nThe board you entered does not contain a space.  Try again.\n");
		return GetInitialPosition();
	}

	numOpieces = numO;
	numXpieces = numX;
	InitializeGame();
	return generic_hash_hash(board, gFirstToMove);
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
	return 2 /* misere */
	       * 2 /* mulptiple peice moves */
	;
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
	int option = 0;
	if (gMisereGame) option += 1;
	if (multiplePieceMoves) option += 1*2;
	return option;
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
	gMisereGame = option%2==1;
	multiplePieceMoves = (option/2%2)==1;
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

/**********************************************************************
**
** NAME:                GetSpace
**
** DESCRIPTION: Return the index of the cell which nobody occupies.
**
** INPUTS:              POSITION position : The current board position
**
** CALLS:		None (Unhash?)
**
***********************************************************************/

int GetSpace(char* posArray) {
	int i;
	for (i=0; i<boardWidth*boardHeight; i++) {
		if (posArray[i] == '-') {
			return i;
		}
	}
	//error("Invalid position.");
	return -1;
}


/***********************************************************************
**
** NAME:		GetPlayer
**
** DESCRIPTION: Return which player is the owner of the given board cell
**
** INPUTS:              POSITION position: The current board position
**			INT index: The index of the cell to inspect.
**
** CALLS:               generic_hash_unhash()
**
***********************************************************************/

int GetPlayer(char* posArray, int spaceIndex, int numAway, int dir) {

	int index;
	if (dir == down) {
		index = spaceIndex-numAway*boardWidth;
	} else if (dir == up) {
		index = spaceIndex+numAway*boardWidth;
	} else if (dir == left) {
		index = spaceIndex+numAway;
	} else {
		index = spaceIndex-numAway;
	}

	if (posArray[index] == 'X') {
		return X;
	} else if (posArray[index] == 'O') {
		return O;
	} else if (posArray[index] == '-') {
		return space;
	} else {
		//error("Invalid position.");
	}
	return -1;
}


int otherPlayer(int player) {
	if (player == O) {
		return X;
	}
	return O;
}


/***********************************************************************
**
** NAME:		getPiece
**
** DESCRIPTION: Return character representation of one of: X, O,
**              space.
**
** INPUTS:              int piece_or_player: numerical representation of
**                                           peice or player
**
** CALLS:               None
**
***********************************************************************/

char getPiece(int piece_or_player) {
	if (piece_or_player == X) {
		return 'X';
	} else if (piece_or_player == O) {
		return 'O';
	} else if (piece_or_player == space) {
		return '-';
	} else {
		//error("Invalid piece or player.");
	}
	return -1;
}


int validCellToCheck(int numPieces, int dir, int indSpace){
	if (dir == down) {
		if (indSpace-numPieces*boardWidth < 0)
			return 0;
	} else if (dir == up) { // bottom of space
		if (indSpace+numPieces*boardWidth >= boardWidth*boardHeight)
			return 0;
	} else if (dir == right) { // left of space
		if ((indSpace-numPieces)/boardWidth != indSpace/boardWidth)
			return 0;
	} else if (dir == left) { // right of space
		if ((indSpace+numPieces)/boardWidth != indSpace/boardWidth)

			return 0;
	}

	return 1;
}

void printBoard(char* board) {
	int i;

	printf("\t\t");
	for (i=1; i<=boardHeight*boardWidth; i++) {
		printf("%c ", board[i-1]);
		if (i%boardWidth == 0) {
			printf("\n\t\t");
		}
	}
}



/*********************************************************************
**
**  START OF MOVE HASHING CODE
**
********************************************************************/


/*********************************************************************
**
** NAME:                hashMove
**
** DESCRIPTION: Hashes move to an int
**
** INPUTS:              int direction: direction of move
**              int numPieces: number of pieces to be moved
**
** CALLS:               None
**
*********************************************************************/

MOVE hashMove(int direction, int numPieces){
	int boardSize = boardWidth;
	if (boardSize < boardHeight) {
		boardSize = boardHeight;
	}
	return direction*(boardSize-1)+numPieces;
}

/*********************************************************************
**
** NAME:                getDirection
**
** DESCRIPTION: Get direction from hashed move
**
** INPUTS:              MOVE move
**
** CALLS:               None
**
*********************************************************************/

int getDirection(MOVE move){
	int boardSize = boardWidth;
	if (boardSize < boardHeight) {
		boardSize = boardHeight;
	}
	return move/(boardSize-1);
}


/*********************************************************************
**
** NAME:                getNumPieces
**
** DESCRIPTION: Get number of pieces to be moved from hashed move
**
** INPUTS:              MOVE move
**
** CALLS:               None
**
*********************************************************************/

int getNumPieces(MOVE move){
	int boardSize = boardWidth;
	if (boardSize < boardHeight) {
		boardSize = boardHeight;
	}
	return move%(boardSize-1);
}



/*********************************************************************
**
**  END OF MOVE HASHING CODE
**
********************************************************************/
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
