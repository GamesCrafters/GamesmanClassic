// $Id: mataxx.c,v 1.15 2007-12-07 19:56:04 max817 Exp $

/************************************************************************
**
** NAME:        mattax.c
**
** DESCRIPTION: Attax (a.k.a. Infection/Spot)
**
** AUTHOR:      Max Delgadillo
**
** DATE:        2006-10-04
**
** UPDATE HIST: -2006.10.4 = First version, includes most functionality.
**				Some things not implemented yet, and all functions not
**				fully tested.
**				-2006.10.4 = Added a quick implementation of Tiers for
**				the demo. Basic functions only.
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

STRING kGameName            = "Ataxx";   /* The name of your game */
STRING kAuthorName          = "Max Delgadillo";   /* Your name(s) */
STRING kDBName              = "ataxx";   /* The name to store the database under */

BOOLEAN kPartizan            = TRUE;   /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN kGameSpecificMenu    = TRUE;   /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN kTieIsPossible       = TRUE;   /* TRUE if a tie is possible. FALSE if it is impossible.*/
BOOLEAN kLoopy               = TRUE;  /* TRUE if the game tree will have cycles (a rearranger style game). FALSE if it does not.*/

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
** #defines and structs
**
**************************************************************************/
#define WIDTH_MAX       9
#define LENGTH_MAX      9
#define WIDTH_MIN       3
#define LENGTH_MIN      3

#define RED                     'X'
#define BLUE            'O'
#define SPACE           ' '
#define PLAYER_ONE  1
#define PLAYER_TWO  2

/*************************************************************************
**
** Global Variables
**
*************************************************************************/
int width               = 3;
int length              = 3;
int boardsize   = 9;

/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/
//void InitializeHelpStrings();
void SetupGame();
POSITION hash (char*, int);
char* unhash (POSITION, int*);
int toIndex (int, int);
int legalCoords (int, int);
void countPieces (char*, int*, int*);
void ChangeBoardSize();

WINBY computeWinBy (POSITION);

TIER BoardToTier(char* board);
void SetupTierStuff();
STRING TierToString(TIER tier);
TIERLIST* TierChildren(TIER tier);
TIERPOSITION NumberOfTierPositions(TIER tier);
/* External */
#ifndef MEMWATCH
extern GENERIC_PTR SafeMalloc ();
extern void     SafeFree ();
#endif

STRING MoveToString(MOVE move);
int GenerateMovesEfficient (POSITION);

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
	//InitializeHelpStrings();
	SetupGame();

	//gPutWinBy = &computeWinBy;

	gGenerateMovesEfficientFunPtr = &GenerateMovesEfficient;

	gMoveToStringFunPtr = &MoveToString;
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
/*void InitializeHelpStrings ()
   {

   kHelpGraphicInterface =
    "";

   kHelpTextInterface =
   "";

   kHelpOnYourTurn =
   "";

   kHelpStandardObjective =
   "";

   kHelpReverseObjective =
   "";

   kHelpTieOccursWhen =
   "A tie occurs when ...";

   kHelpExample =
   "";

    gMoveToStringFunPtr = &MoveToString;

   }*/


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
	MOVELIST* moves = NULL;
	int x, y, i, j;
	int turn;
	char* board = unhash(position, &turn);
	for (y = 1; y <= length; y++) { // look through all the rows, bottom-up
		for (x = 1; x <= width; x++) { // look through the columns, left-to-right
			if (board[toIndex(x,y)] != (turn==PLAYER_ONE ? RED : BLUE))
				continue; //not our piece, we can't move it
			/* check the 5x5 grid around us, using i to modify x and j for y.
			   moves-wise, jumps are handled the same as slides
			   note that this DOES check its own square, but always fails
			   the test since it's not a SPACE. */
			for (j = -2; j <= 2; j++) // rows, bottom-up
				for (i = -2; i <= 2; i++) // columns, left-right
					if (legalCoords(x+i,y+j) && board[toIndex(x+i,y+j)] == SPACE)
						moves = CreateMovelistNode((x*1000) + (y*100) + ((x+i)*10) + (y+j), moves);
		}
	}
	if (board != NULL)
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

POSITION DoMove (POSITION position, MOVE move)
{
	/* MOVE = four digits, are just coords. So a4b3 is 1423. */
	int x1, y1, x2, y2, i, j;
	//unhash the move
	x1 = (move/1000) % 10;
	y1 = (move/100) % 10;
	x2 = (move/10) % 10;
	y2 = move % 10;
	// find out if this is a jump
	BOOLEAN jump = (abs(x1-x2) > 1 || abs(y1-y2) > 1);
	// now make the board
	int turn;
	char* board = unhash(position, &turn);
	char myPiece = (turn==PLAYER_ONE ? RED : BLUE),
	     oppPiece = (turn==PLAYER_ONE ? BLUE : RED);
	if (jump) { // if there is a jump, remove old piece and place new piece
		board[toIndex(x1,y1)] = SPACE;
		board[toIndex(x2,y2)] = myPiece;
	} else // no jump, leave the old piece alone and place new piece
		board[toIndex(x2,y2)] = myPiece;
	// now replace all pieces around you with this for loop
	for (j = -1; j <= 1; j++) // rows, bottom-up
		for (i = -1; i <= 1; i++) // columns, left-right
			if (legalCoords(x2+i,y2+j) && board[toIndex(x2+i,y2+j)] == oppPiece)
				board[toIndex(x2+i,y2+j)] = myPiece;
	turn = (turn==PLAYER_ONE ? PLAYER_TWO : PLAYER_ONE);
	return hash(board, turn);
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
	int reds, blues;
	BOOLEAN redWon;
	int turn;
	char* board = unhash(position, &turn);

	countPieces(board, &reds, &blues);

	if (board != NULL)
		SafeFree(board);

	if (reds == 0) // no reds, blue has won
		redWon = FALSE;
	else if (blues == 0) // no blues, red has won
		redWon = TRUE;
	else if (reds+blues == boardsize) { // else if board is filled
		if (reds > blues) //red won
			redWon = TRUE;
		else if (blues > reds) //blue won
			redWon = FALSE;
		else return tie; // tie
	} else {
		// now, we check to see if there's legal moves:
		MOVELIST* moves = GenerateMoves(position);
		if (moves == NULL) // a lose for current player!
			redWon = (turn==PLAYER_ONE ? TRUE : FALSE);
		else {
			FreeMoveList(moves);
			return undecided;
		}
	}

	VALUE value;
	if (redWon) //for cleaner code
		value = (turn==PLAYER_ONE ? win : lose);
	else value = (turn==PLAYER_ONE ? lose : win);

	if (value == win)
		return (gStandardGame ? win : lose);
	else return (gStandardGame ? lose : win);
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
	int x, y, reds, blues;
	int turn;
	char* board = unhash(position, &turn);
	countPieces(board, &reds, &blues);
	printf("\t%s's Turn (%s):\n  ",playersName,(turn==PLAYER_ONE ? "X" : "O"));
	printf("%s\n", GetPrediction(position, playersName, usersTurn));
	printf("X = %d\tO = %d\n  ", reds, blues);
	for (y = length; y >= 1; y--) { // for all the rows
		for (x = 1; x <= width; x++) // for all the columns
			printf("+-");
		printf("+\n%d ", y);
		for (x = 1; x <= width; x++)
			printf("|%c",board[toIndex(x,y)]);
		printf("|\n  ");
	}
	for (x = 1; x <= width; x++) // for all the columns
		printf("+-");
	printf("+\n  ");
	for (x = 1; x <= width; x++) { // for all the columns
		printf(" %c",'a'-1+x);
	}
	printf("\n\n");
	if (board != NULL)
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
	printf("%8s's move : ", computersName);
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
	STRING str = MoveToString(move);
	printf("%s", str);
	SafeFree(str);
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
	STRING moveStr = (STRING) SafeMalloc(sizeof(char)*5);
	moveStr[0] = ((move/1000) % 10)+'a'-1;
	moveStr[1] = ((move/100) % 10)+'0';
	moveStr[2] = ((move/10) % 10)+'a'-1;
	moveStr[3] = (move % 10)+'0';
	moveStr[4] = '\0';
	return moveStr;
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
		printf("%8s's move [(undo)/([%c-%c][%d-%d][%c-%c][%d-%d])] : ", playersName,
		       'a', width+'a'-1, 1, length, 'a', width+'a', 1, length);

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
	if(strlen(input) != 4)
		return FALSE;
	if (!isalpha(input[0]) || !isdigit(input[1]) ||
	    !isalpha(input[2]) || !isdigit(input[3]))
		return FALSE;
	return TRUE;
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
	return (1000*(input[0]-'a'+1) + 100*(input[1]-'0')
	        + 10*(input[2]-'a'+1) + (input[3]-'0'));
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
	char c;
	BOOLEAN cont = TRUE;
	while(cont) {
		printf("\n\nCurrent %dx%d board:  \n", width, length);
		PrintPosition(gInitialPosition, "Gamesman", 0);
		printf("\tGame Options:\n\n"
		       "\tc)\t(C)hange the board size (nxn), currently: %dx%d\n"
		       "\ti)\tSet the (I)nitial position\n"
		       "\tb)\t(B)ack to the main menu\n"
		       "\nSelect an option:  ", width, length);
		c = GetMyChar();
		switch(c) {
		case 'c': case 'C':
			ChangeBoardSize();
			break;
		case 'i': case 'I':
			GetInitialPosition();
			break;
		case 'b': case 'B':
			cont = FALSE;
			break;
		default:
			printf("Invalid option!\n");
		}
	}
	//InitializeHelpStrings();
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
	if (gStandardGame)
		return 1;
	else return 2;
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
	if (option == 1)
		gStandardGame = TRUE;
	else gStandardGame = FALSE;
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

// This counts the number of pieces on the board.
void countPieces (char* board, int* reds, int* blues) {
	int i, red = 0, blue = 0;
	for (i = 0; i < boardsize; i++) { //count the pieces on the board
		if (board[i] == RED) red++;
		else if (board[i] == BLUE) blue++;
	}
	(*reds) = red;
	(*blues) = blue;
}


// This is for Win-By
WINBY computeWinBy (POSITION position)
{
	int reds, blues;
	int turn;
	char* board = unhash(position, &turn);
	countPieces(board, &reds, &blues);
	if (board != NULL)
		SafeFree(board);
	return reds-blues;
}

/*
   The BOARD the player sees is as follows:
   3 + + + +
   2 + + + +
   1 + + + +
   a b c d

   length = 3, width = 4, boardsize = 12.

   So on the array, it's like this:
   { a3, b3, c3, d3, a2, b2, c2, d2, a1, b1, c1, d1 }
   0   1   2   3   4   5   6   7   8   9   10  11

   For the game, letters are translated to numbers (a = 1, b = 2, etc.)
   So (1,1) is the bottom left, (4,3) is the top right.

   toIndex takes in the coord and produces its index.
   So toIndex(1,1) = 8, toIndex(4,3) = 3.
 */

//a precursor to toIndex = checks if x and y are legal coords
int legalCoords (int x, int y) {
	return ((x > 0 && x <= width) && (y > 0 && y <= length));
}

//goes from 2D coords to 1D array
int toIndex (int x, int y) {
	return ((length-y)*width) + (x-1);
}

void ChangeBoardSize ()
{
	int newWidth, newLength;
	while (TRUE) {
		printf("\n\nCurrent board of size %dx%d:\n\n", width, length);
		PrintPosition(gInitialPosition, "Gamesman", 0);
		printf("\n\nEnter the new width (%d - %d):  ", WIDTH_MIN, WIDTH_MAX);
		newWidth = GetMyChar()-48;
		if(newWidth > WIDTH_MAX || newWidth < WIDTH_MIN) {
			printf("\nInvalid width!\n");
			continue;
		}
		printf("\n\nEnter the new length (%d - %d):  ", LENGTH_MIN, LENGTH_MAX);
		newLength = GetMyChar()-48;
		if(newLength > LENGTH_MAX || newLength < LENGTH_MIN) {
			printf("\nInvalid length!\n");
			continue;
		}
		width = newWidth;
		length = newLength;
		boardsize = width*length;
		SetupGame();
		break;
	}
}

void SetupGame() {
	generic_hash_destroy();
	generic_hash_custom_context_mode(TRUE);
	SetupTierStuff();
	if (width <= 5 && length <= 5) {
		int pieces_array[10] = {RED, 0, boardsize, BLUE, 0, boardsize, SPACE, 0, boardsize-4, -1};
		gNumberOfPositions = generic_hash_init(boardsize, pieces_array, NULL, 0);
		// initial position
		char* board = (char*) SafeMalloc(boardsize * sizeof(char));
		int i;
		for(i = 0; i < boardsize; i++)
			board[i] = SPACE;
		board[toIndex(1, length)] = RED;
		board[toIndex(width, length)] = BLUE;
		board[toIndex(1, 1)] = BLUE;
		board[toIndex(width, 1)] = RED;
		int turn = 1;
		gInitialPosition = hash(board, turn);
	}
}

char* unhash (POSITION position, int* turn)
{
	char* board = (char*) SafeMalloc(boardsize * sizeof(char));
	if (gHashWindowInitialized) {
		TIERPOSITION tierPos; TIER tier;
		gUnhashToTierPosition(position, &tierPos, &tier);
		generic_hash_context_switch(tier);
		(*turn) = generic_hash_turn(tierPos);
		generic_hash_unhash(tierPos, board); //sets board
	} else {
		(*turn) = generic_hash_turn(position);
		generic_hash_unhash(position, board); //sets board
	}
	return board;
}

POSITION hash (char* board, int turn)
{
	POSITION position;
	if (gHashWindowInitialized) {
		TIER tier = BoardToTier(board);
		generic_hash_context_switch(tier);
		TIERPOSITION tierPos = generic_hash_hash(board, turn);
		position = gHashToWindowPosition(tierPos, tier);
	} else position = generic_hash_hash(board, turn);
	if (board != NULL)
		SafeFree(board);
	return position;
}

/* TIERS: Number of pieces left to place. Alternately, number of spaces
   on the board.
   That gives tiers solvable from 0 to boardsize.
   Pieces on board = boardsize-tier
 */

TIER BoardToTier(char* board) {
	int reds, blues;
	countPieces(board, &reds, &blues);
	int totalPieces = reds+blues;
	return boardsize-totalPieces;
}

void SetupTierStuff() {
	// kSupportsTierGamesman
	kSupportsTierGamesman = TRUE;
	// function pointers
	gTierChildrenFunPtr = &TierChildren;
	gNumberOfTierPositionsFunPtr = &NumberOfTierPositions;
	gTierToStringFunPtr = &TierToString;
	// hashes
	// Tier-Specific Hashes
	int piecesArray[10] = { RED, 0, 0, BLUE, 0, 0, SPACE, 0, 0, -1 };
	int piecesOnBoard, tier;
	for (tier = 0; tier <= boardsize; tier++) {
		piecesOnBoard = boardsize - tier;
		// Reds AND Blues = from 0 to piecesOnBoard
		piecesArray[1] = 0;
		piecesArray[2] = piecesOnBoard;
		piecesArray[4] = 0;
		piecesArray[5] = piecesOnBoard;
		// Blanks = tier
		piecesArray[7] = piecesArray[8] = tier;
		// make the hashes
		generic_hash_init(boardsize, piecesArray, NULL, 0);
		generic_hash_set_context(tier);
	}
	// Initial
	char* board = (char*) SafeMalloc(boardsize * sizeof(char));
	int i;
	for(i = 0; i < boardsize; i++)
		board[i] = SPACE;
	board[toIndex(1, length)] = RED;
	board[toIndex(width, length)] = BLUE;
	board[toIndex(1, 1)] = BLUE;
	board[toIndex(width, 1)] = RED;
	// Initial Tier = boardsize-4 (so there's boardsize-4 spaces)
	gInitialTier = boardsize-4;
	generic_hash_context_switch(gInitialTier);
	int turn = 1;
	gInitialTierPosition = hash(board, turn);
}

// children = always me and one below
TIERLIST* TierChildren(TIER tier) {
	TIERLIST* list = NULL;
	list = CreateTierlistNode(tier, list);
	if (tier != 0)
		list = CreateTierlistNode(tier-1, list);
	return list;
}

TIERPOSITION NumberOfTierPositions(TIER tier) {
	generic_hash_context_switch(tier);
	return generic_hash_max_pos();
}

// Tier = Number of pieces left to place.
STRING TierToString(TIER tier) {
	STRING tierStr = (STRING) SafeMalloc(sizeof(char)*16);
	sprintf(tierStr, "%d Pieces Placed", boardsize-((int)tier));
	return tierStr;
}


// An experimental GenerateMoves that should be much faster
int GenerateMovesEfficient (POSITION position)
{
	int x, y, i, j, index = 0;
	int turn;
	char* board = unhash(position, &turn);
	for (y = 1; y <= length; y++) { // look through all the rows, bottom-up
		for (x = 1; x <= width; x++) { // look through the columns, left-to-right
			if (board[toIndex(x,y)] != (turn==PLAYER_ONE ? RED : BLUE))
				continue; //not our piece, we can't move it
			/* check the 5x5 grid around us, using i to modify x and j for y.
			   moves-wise, jumps are handled the same as slides
			   note that this DOES check its own square, but always fails
			   the test since it's not a SPACE. */
			for (j = -2; j <= 2; j++) // rows, bottom-up
				for (i = -2; i <= 2; i++) // columns, left-right
					if (legalCoords(x+i,y+j) && board[toIndex(x+i,y+j)] == SPACE)
						gGenerateMovesArray[index++] = (x*1000) + (y*100) + ((x+i)*10) + (y+j);
		}
	}
	if (board != NULL)
		SafeFree(board);
	return index;
}


// $Log: not supported by cvs2svn $
// Revision 1.14  2007/05/08 22:14:00  max817
// Fixed a bug with initializing the game
//
// Revision 1.13  2007/04/05 19:16:19  max817
// Changed TIER from an unsigned int to an unsigned long long, and fixed any warnings associated with the change.
//
// Revision 1.12  2007/03/06 02:16:40  max817
// Fixed Generic Hash custom contexts mode. Tested with mataxx.c. -Max
//
// Revision 1.11  2007/02/27 02:15:00  max817
// Fixed a bug with the global board inits. -Max
//
// Revision 1.10  2007/02/27 02:08:18  max817
// Fixed a bug in Primitive, and implemented a tester for GenerateMovesEfficient.
//
// Revision 1.9  2007/02/27 01:29:45  max817
// Added Win-By for fun, even though it doesn't work thanks to tiers. -Max
//
// Revision 1.8  2007/02/27 01:02:39  max817
// Made code more efficient with globals. -Max
//
// Revision 1.7  2006/12/19 20:00:50  arabani
// Added Memwatch (memory debugging library) to gamesman. Use 'make memdebug' to compile with Memwatch
//
// Revision 1.6  2006/12/07 02:52:50  max817
// TUI Changes to Ataxx.
//
// Revision 1.5  2006/11/30 10:30:25  max817
// Ataxx now correctly handles misere.
//
// Revision 1.4  2006/10/17 10:45:20  max817
// HUGE amount of changes to all generic_hash games, so that they call the
// new versions of the functions.
//
// Revision 1.3  2006/10/11 06:59:02  max817
// A quick modification of the Tier Gamesman games to include the new changes.
//
// Revision 1.2  2006/10/04 23:55:40  max817
// Added quick implementation of Tiers for the demo tonight.
//
// Revision 1.1  2006/10/04 13:14:13  max817
// Added in Ataxx in mataxx.c, and changed Makefile.in to include it.
//
// Revision 1.10  2006/04/25 01:33:06  ogren
// Added InitialiseHelpStrings() as an additional function for new game modules to write.  This allows dynamic changing of the help strings for every game without adding more bookkeeping to the core.  -Elmer
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
