// $Id: mquickchess.c,v 1.13 2006-06-27 05:18:18 runner139 Exp $

/*
* The above lines will include the name and log of the last person
 * to commit this file to CVS
 */

/************************************************************************
**
** NAME:        mquickchess.c
**
** DESCRIPTION: Quick Chess
**
** AUTHOR:      Adam Abed, Aaron Levitan
**
** DATE:        Start Spring 2006
**
** UPDATE HIST:
** 11 Mar 2006 Aaron: updated string fields with game information
** 17 Mar 2006 Adam:  finished all of primitive
** 01 Apr 2006 Aaron: added generateMoves code, with helper functions:
**                    getCurrTurn - returns whose turn it is
**                    isSameTeam - checks if the piece is the current player's
**                    defined player1 and player2 turn constants
** 07 Apr 2006 Adam:  reverted back to using a single array for board
** 09 Apr 2006 Aaron: Changed move to be a 4 byte character array of numbers
**                    in the format old i, old j, new i, new j
** 10 Apr 2006 Aaron: converted generate moves function and helper functions
**                    to use moves instead of helper struct from testing file
**                    Remove BOARDSIZE constant and replaced with rows*cols
**                    Fixed DoMove's use of generic hash by changing WHITE
**                    and BLACK players constants to use 1,2 instead of 0,1
** 15 Apr 2006 Aaron: Got rid of global currentPlayer to use generic hash player
**                    Modified substitutePawn and other functions to use generic
**                    hash player variable.
**                    Fixed ConvertTextInputToMove
**                    Fixed ValidTextInput by removing = sign in third char check
**                    Added kingCheck function and case in inCheck
** 18 Apr 2006 Aaron: Removed return char in PrintMove
**                    Moved setupPieces code into InitializeGame()
**                    Downsized board to make it solve
** 25 Apr 2006 Adam:  Added substitute pawn functionality
** 02 May 2006 Aaron: Added code to print and move functions to make work with
**                    substitute pawn.  Substitute pawn only working with queens.
**                    Changed way to display whose turn.  now based on WhoseMove instead of
**                    is users turn.
**                    Removed substitutePawn, isWhiteReplacementValid, and isBlackReplacementValid
** 19 May 2006 Adam:  Added replacement function, needed for pawn promotion
**                    Worked on implementing pawn promotion
** 6 Jun 2006 Adam:   Finished pawn replacement
** 11 Jun 2006 Adam:  Worked on getCanoncial() for symmetries
** 12 Jun 2006 Adam:  Worked on getCanoncial() for symmetries
** 15 Jun 2006 Adam:  Worked on getCanoncial() for symmetries
** 20 Jun 2006 Adam:  Finished function getCanonical() still need to debug
** 23 Jun 2006 Adam:  Begin writing gTierValue 
** 24 Jun 2006 Adam:  Wrote gTierValue for tiers with 2 and 3 pieces, now need 4-20 YIKES! 
**                    Also finished writing gUndoMove
** 25 Jun 2006 Adam: Finished debugging getCanonical() and finished writing 
**                   MoveToString()


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

STRING   kGameName            = "Quick Chess"; /* The name of your game */
STRING   kAuthorName          = "Aaron Levitan, Adam Abed"; /* Your name(s) */
STRING   kDBName              = "quickchess"; /* The name to store the database under */

BOOLEAN  kPartizan            = TRUE ; /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN  kGameSpecificMenu    = TRUE ; /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN  kTieIsPossible       = TRUE ; /* TRUE if a tie is possible. FALSE if it is impossible.*/
BOOLEAN  kLoopy               = TRUE ; /* TRUE if the game tree will have cycles (a rearranger style game). FALSE if it does not.*/

BOOLEAN  kDebugMenu           = TRUE ; /* TRUE only when debugging. FALSE when on release. */
BOOLEAN  kDebugDetermineValue = FALSE ; /* TRUE only when debugging. FALSE when on release. */

POSITION gNumberOfPositions   =  0; /* The number of total possible positions | If you are using our hash, this is given by the hash_init() function*/
POSITION gInitialPosition     =  0; /* The initial hashed position for your starting board */
POSITION kBadPosition         = -1; /* A position that will never be used */

void*	 gGameSpecificTclInit = NULL;

/* 
* Help strings that are pretty self-explanatory
* Strings than span more than one line should have backslashes (\) at the end of the line.
*/

STRING kHelpGraphicInterface =
"Not written yet";

STRING   kHelpTextInterface    =
"The board is arranged like a standard chess board. \n\
The rows are specified by numeric values, while the \n\
columns are specified by letters. A square is referenced \n\
by the column and then the row, i.e. b4.";
STRING   kHelpOnYourTurn =
"";

STRING   kHelpStandardObjective =
"Try to checkmate your opponent's king.";

STRING   kHelpReverseObjective =
"Try to get your king checkmated.";

STRING   kHelpTieOccursWhen =
"A tie occurs when a player is not in check and does not have any valid moves.";

STRING   kHelpExample =
"";


/*************************************************************************
**
** #defines and structs
**
**************************************************************************/
#define rows 4
#define cols 3
#define BOARDSIZE rows*cols
#define WHITE_TURN 2 
#define BLACK_TURN 1 
#define BLACK_PAWN 'p'
#define BLACK_BISHOP 'b'
#define BLACK_ROOK 'r'
#define BLACK_KNIGHT 'n'
#define BLACK_QUEEN 'q'
#define BLACK_KING 'k'
#define WHITE_PAWN 'P'
#define WHITE_BISHOP 'B'
#define WHITE_ROOK 'R'
#define WHITE_KNIGHT 'N'
#define WHITE_KING 'K'
#define WHITE_QUEEN 'Q'
#define BLANK_PIECE ' '
#define DISTINCT_PIECES 10

// Constants specifying directions to "look" on the board 
#define UP 0 
#define DOWN 1 
#define LEFT 2 
#define RIGHT 3 
#define UL 4 
#define UR 5 
#define DL 6 
#define DR 7 
/*************************************************************************
**
** Global Variables
**
*************************************************************************/

/*VARIANTS*/
BOOLEAN normalVariant = TRUE;
BOOLEAN misereVariant = FALSE;

/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/

/* External */
extern GENERIC_PTR	SafeMalloc ();
extern void		SafeFree ();
void PrintPosition (POSITION position, STRING playersName, BOOLEAN usersTurn);
void setupPieces(char *Board);
BOOLEAN inCheck(POSITION N, int currentPlayer);
BOOLEAN isKingCaptureWithBreak(int *breaks, char *Board, int row, int col, int currentPlayer, char currentPiece, char whitePiece, char blackPiece);
BOOLEAN isKingCaptureWithoutBreak(char *Board, int row, int col, int currentPlayer, char currentPiece, char whitePiece, char blackPiece);
BOOLEAN queenCheck(char *Board, int row, int col, int currentPlayer, char currentPiece, char whitePiece, char blackPiece);
BOOLEAN bishopCheck(char *Board, int row, int col, int currentPlayer, char currentPiece, char whitePiece, char blackPiece);
BOOLEAN rookCheck(char *Board, int row, int col, int currentPlayer, char currentPiece, char whitePiece, char blackPiece);
BOOLEAN knightCheck(char *Board, int row, int col, int currentPlayer, char currentPiece, char whitePiece, char blackPiece);
BOOLEAN pawnCheck(char *Board, int row, int col, int currentPlayer, char currentPiece, char whitePiece, char blackPiece);
BOOLEAN kingCheck(char *Board, int row, int col, int currentPlayer, char currentPiece, char whitePiece, char blackPiece);
void substitutePawn(char *boardArray, int currentPlayer, int x, int y);
BOOLEAN isWhiteReplacementValid(char piece, char *bA);
BOOLEAN isBlackReplacementValid(char piece, char *bA);
int atobi(char s[], int base);
BOOLEAN isSameTeam(char piece, int currentPlayer); 
BOOLEAN testMove(char *boardArray, int rowi, int rowf, int coli, int colf, int currentPlayer);
void UndoMove(char *boardArray,int rowi, int rowf, int coli, int colf, int currentPlayer);
void testDoMove(char *boardArray, int rowi, int rowf, int coli, int colf, int currentPlayer);
MOVE createMove(int rowi, int coli, int rowf, int colf);
MOVE createPawnMove(int rowi, int coli, int rowf, int colf, char replacementPiece);
void generateMovesDirection(char* boardArray,  MOVELIST **moves, int currentPlayer, int i, int j, int direction);
void generatePawnMoves(char *boardArray,  MOVELIST **moves, int currentPlayer, int i, int j);
void generateKnightMoves(char *boardArray,  MOVELIST **moves, int currentPlayer, int i, int j);
void generateRookMoves(char *boardArray,  MOVELIST **moves, int currentPlayer, int i, int j);
void generateBishopMoves(char *boardArray,  MOVELIST **moves, int currentPlayer, int i, int j);
void generateQueenMoves(char *boardArray,  MOVELIST **moves, int currentPlayer, int i, int j);
void generateKingMoves(char *boardArray,  MOVELIST **moves, int currentPlayer, int i, int j);
void printArray (char* boardArray);
void printMoveList(MOVELIST *moves);
BOOLEAN replacement(char *boardArray, char replacementPiece);
POSITION GetCanonicalPosition(POSITION position);
POSITION DoSymmetry(POSITION position, int symmetry);
POSITION flipLR(POSITION position);
POSITION flipUD(POSITION position);
BOOLEAN isPawn(POSITION position);
BOOLEAN isBishop(POSITION position);
STRING MoveToString(MOVE move);

/**************************************************/
/**************** SYMMETRY FUN BEGIN **************/
/**************************************************/

BOOLEAN kSupportsSymmetries = TRUE; /* Whether we support symmetries */

#define NUMSYMMETRIES 4   /*  4 flippings */ 

// int gSymmetryMatrix[NUMSYMMETRIES][BOARDSIZE];


/* Proofs of correctness for the below arrays:
**
** FLIP	LR				FLIP UD
**
** 0  1  2	2  1  0		0  1  2	       9  10 11
** 3  4  5  ->  5  4  3		3  4  5	  ->   6  7  8  
** 6  7  8	8  7  6		6  7  8	       3  4  5
** 9  10 11	11 10 9         9  10 11       0  1  2
*/

/* This is the array used for flipping along the N-S axis */
int gSmallFlipLRNewPosition[] = { 2, 1, 0, 5, 4, 3, 8, 7, 6, 11, 10, 9 };

/* This is the array used for flipping along the  E-W axis  */
int gSmallFlipUDNewPosition[] = { 9, 10 ,11, 6, 7, 8, 3, 4, 5, 0, 1, 2 };

/* This is the array used for flipping along the N-S axis */
// int gLargeFlipLRNewPosition[] = { 4, 3, 2, 1, 0, 9, 8, 7, 6, 5, 14, 13, 12, 11, 10, 19,18,17,16,15,24,23,22,21,20,29,28,27,26,25 };

/* This is the array used for flipping along the  E-W axis  */
// int int gLargeFlipUDNewPosition[] = { 9, 10 ,11, 6, 7, 8, 3, 4, 5, 0, 1, 2 };


/**************************************************/
/**************** SYMMETRY FUN END ****************/
/**************************************************/
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
  
  gMoveToStringFunPtr = &MoveToString;
  gCanonicalPosition = GetCanonicalPosition;

  //int pieces_array[40] = {'p', 0, 1, 'b', 0, 1, 'r', 0, 1, 'n', 0, 1, 'q', 0, 1, 'k', 1, 1, 'P', 0, 1, 'B', 0, 1, 'R', 0, 1, 'N', 0, 1, 'Q', 0, 1, 'K', 1, 1, ' ',10, 28, -1};
  //     int pieces_array[22] = {'R', 0, 1, 'K', 0, 1, 'P', 0, 1, 'r', 0, 1, 'k', 0, 1, 'p', 0, 1, ' ', 7, 13, -1};
  // int pieces_array[28] = {'B', 0, 1, 'R', 0, 1, 'K', 1, 1, 'P', 0, 3, 'r', 0, 1, 'k', 1, 1, 'p', 0, 3, 'b', 0, 1, ' ', 3, 13, -1};
  int pieces_array[22] = {'K', 1, 1, 'R', 0, 1, 'B', 0, 1, 'k', 1, 1, 'r', 0, 1, 'b', 0, 1, ' ', 6, 10, -1};
	char gameBoard[rows*cols];
	setupPieces(gameBoard);
	gNumberOfPositions = generic_hash_init(rows*cols, pieces_array, NULL);
	gInitialPosition = generic_hash(gameBoard, WHITE_TURN);
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
	int currentPlayer, i,j; 
	char piece; 
	char boardArray[rows*cols]; 
	generic_unhash(position, boardArray); 
	currentPlayer = whoseMove(position);
	for (i = 0; i < rows; i++) {
		for(j = 0; j < cols; j++) {
			//printf("move: i:%d, j:%d",i,j);
			piece = boardArray[i*cols + j]; 
			// check if piece belongs to the currentPlayer
			if (isSameTeam(piece, currentPlayer)) {
				switch (piece) { 
					case WHITE_QUEEN: case BLACK_QUEEN:  
						generateQueenMoves(boardArray, &moves, currentPlayer, i, j);
						break;
					case WHITE_BISHOP: case BLACK_BISHOP: 
						generateBishopMoves(boardArray, &moves, currentPlayer, i, j);
						break;
					case WHITE_ROOK: case BLACK_ROOK:
						generateRookMoves(boardArray, &moves, currentPlayer, i, j);
						break;
					case WHITE_KNIGHT: case BLACK_KNIGHT:
						generateKnightMoves(boardArray, &moves, currentPlayer, i, j);
						break;
					case WHITE_PAWN: case BLACK_PAWN:
						generatePawnMoves(boardArray, &moves, currentPlayer, i, j);
						break;
					case WHITE_KING: case BLACK_KING:
						generateKingMoves(boardArray, &moves, currentPlayer, i, j);
						break;
					default:  
						break; 
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
**** INPUTS:      POSITION position : The old position
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
	char boardArray[rows*cols];
	char tempPiece, replacementPiece;
	int rowi, coli, rowf, colf;
	rowf = move & 15; 
	colf = (move >> 4) & 15; 
	rowi = (move >> 8) & 15; 
	coli = (move >> 12) & 15; 
	replacementPiece = (char) (move >> 16) & 255;
	
	generic_unhash(position, boardArray);
	int currentPlayer = whoseMove(position);
	tempPiece = boardArray[(rows-rowi)*cols + (coli - 10)];
	boardArray[(rows - rowi)*cols + (coli - 10)] = ' ';
	boardArray[(rows - rowf)*cols + (colf - 10)] = tempPiece;
	if(tempPiece == WHITE_PAWN) {
		if(rowf == 1){
		  boardArray[(rows - rowf)*cols + (colf - 10)] = replacementPiece;
		} 
	} else if(tempPiece == BLACK_PAWN) {
		if(rowf == rows) {
		  boardArray[(rows - rowf)*cols + (colf - 10)] = replacementPiece;
		}
	}
	if(currentPlayer == WHITE_TURN) {
		currentPlayer = BLACK_TURN;
	} else currentPlayer = WHITE_TURN;
	return generic_hash(boardArray, currentPlayer);	
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
	//printf("primitive");
	MOVELIST *moves = NULL;
	moves = GenerateMoves(position);
	int currentPlayer = whoseMove(position);
	if (inCheck(position, currentPlayer) && moves == NULL) { 
		// The king is checked and no moves can be made 
		return (gStandardGame) ? lose : win; 
	} 
	else if (!inCheck(position, currentPlayer) && moves == NULL) { 
		// King is not in check and no moves can be made - Stalemate 
		return tie; 
	} else { 
		return undecided; 
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
	int x, y;
	char boardArray[rows*cols];
	generic_unhash(position, boardArray);
	
	printf("\n"); 
	for(x = 0; x < rows; x++){ 
		printf("    +"); 
		for(y = 0; y < cols; y++){ 
			printf("---+"); 
		} 
		printf("\n"); 
		printf("  %d |", rows - x); 
		for(y = 0; y < cols; y++){ 
			printf(" %c |", boardArray[x*cols + y]); 
		} 
		printf("\n"); 
	} 
	printf("    +"); 
	for(y = 0; y < cols; y++){ 
		printf("---+"); 
	} 
	printf("\n"); 
	printf("     "); 
	for(y = 0; y < cols; y++){ 
		printf(" %c  ", 97+y); 
	} 
	printf("\n"); 
	printf("%s\n",GetPrediction(position,playersName,usersTurn)); 
	 printf("It is %s's turn (%s).\n",playersName,(usersTurn) ? "white/uppercase":"black/lowercase"); 
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
	char rowf, colf, rowi, coli, replacementPiece;
	rowf = (move & 15) + 48; 
	colf = ((move >> 4) & 15) - 10 + 97; 
	rowi = ((move >> 8) & 15) + 48; 
	coli = ((move >> 12) & 15) - 10 + 97; 
	replacementPiece =  move >> 16;
	if (replacementPiece == 0) {
	  printf("%c%c%c%c", coli, rowi, colf, rowf);
	} else {
	  printf("%c%c%c%c=%c", coli, rowi, colf, rowf, replacementPiece);		
	}

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
    
    for (;;) {
        /***********************************************************
		* CHANGE THE LINE BELOW TO MATCH YOUR MOVE FORMAT
		***********************************************************/
		printf("%8s's move [(undo)/<coli><rowi><colf><rowf><replacementPiece>] : ", playersName);
		
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
**************************************************************************/

BOOLEAN ValidTextInput (STRING input)
{
char c;
	int length = strlen(input);
	if(length!=4 && length!=6 ) 
		return FALSE; 
	else{ 
		/* Make sure the row and column are within the bounds 
		set by the current board size. */  
		c = input[0];
		if (c < 'a' || c >= 'a' + cols) return FALSE; 
		c = input[2]; 
		if (c < 'a' || c >= 'a' + cols) return FALSE; 
		c = input[1]; 
		if(c > rows + '0' || c < '1') return FALSE; 
		c = input[3]; 
		if(c > rows + '0' || c < '1') return FALSE; 
		if (length == 6) {  // pawn replacement move
			c = input[4];
			if (c != '=') return FALSE;
			c = input [5];
			if (c != 'Q' && c != 'q' && c != 'R' && c != 'r'
				&& c != 'B' && c != 'b') return FALSE;
		}
	} 
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
  MOVE m;
  int rowi, coli, rowf, colf;
  coli = input[0] - 97;
  rowi = input[1] - 48;
  colf = input[2] - 97;
  rowf = input[3] - 48;
  if (strlen(input) == 6) {
    char replacement = input[5];
    m = createPawnMove(rows - rowi, coli, rows - rowf, colf, replacement);
  } else {
    m = createMove(rows - rowi, coli, rows - rowf, colf);
  }
  return m;
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
	printf("\n"); 
	printf("Tile Chess Game Specific Menu\n\n"); 
	printf("1) Normal QuickChess Play\n"); 
	printf("2) Misere Variant\n"); 
	printf("b) Back to previous menu\n\n"); 
	
	printf("Select an option: "); 
	
	switch(GetMyChar()) { 
		case 'Q': case 'q': 
			ExitStageRight(); 
		case '1': 
			normalVariant = TRUE; 
			misereVariant = FALSE; 
			break; 
		case '2': 
			misereVariant = TRUE; 
			normalVariant = FALSE; 
			break; 
		case 'b': case 'B': 
			return; 
		default: 
			printf("\nSorry, I don't know that option. Try another.\n"); 
			HitAnyKeyToContinue(); 
			GameSpecificMenu(); 
			break; 
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
    return gInitialPosition;
	
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
    /* If you have implemented symmetries you should
	include the boolean variable gSymmetries in your
	hash */
	if (normalVariant) 
		return 1; 
	else if (misereVariant) 
		return 2; 
	else 
		BadElse("getOption"); 
	
	return -1; 
	
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
	if (option == 1) { 
		normalVariant = TRUE; 
		misereVariant = FALSE; 
	} else if (option == 2) { 
		normalVariant = FALSE; 
		misereVariant = TRUE; 
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
  //MOVE m;
	/*
	int coli, rowi, colf, rowf;
	 coli = 10;
	 colf = 10;
	 rowi = 4;
	 rowf = 3;
	 m = (coli << 12) | (rowi << 8) | (colf << 4) | rowf;
	 */
	//	m = createMove(0,0,1,0);
	/*
	printf("input move is %s\n", "a4a1");
	m = ConvertTextInputToMove("a4a1");
	PrintMove(m);
	*/
	//PrintPosition(gInitialPosition, "me", TRUE);
	/*POSITION newPos = DoMove(gInitialPosition,
	PrintPosition(newPos, "me", TRUE);
	m = ConvertTextInputToMove("b1a2");
	newPos = DoMove(newPos, m);
	PrintPosition(newPos, "me", TRUE);
	*/
	/*
       	m = ConvertTextInputToMove("b1c1");
	PrintPosition(gInitialPosition, "me", TRUE);
	POSITION newPos = DoMove(gInitialPosition,m); 
	PrintPosition(newPos, "me", TRUE);
	if(inCheck(newPos, 1)) {
	  printf("player 1 in check\n");
	}
	if(inCheck(gInitialPosition, 2)) {
	  printf("player 2 in check\n");
	}
	*/
  /*
  int i;
  char bA[rows*cols];
  printf("the number of spaces is: %d\n", rows*cols);	
  generic_unhash(gInitialPosition, bA);
  
  for(i = 0; i < BOARDSIZE; i++) {
    printf("seeing whats on the board on space %d: %c\n", i, bA[i]);
  }
  
  generic_unhash(flipLR(gInitialPosition), bA);
  
  for(i = 0; i < BOARDSIZE; i++) {
    printf("seeing whats on the board on space %d: %c\n", i, bA[i]);
  }
  */
  MOVE m1, m2;
  STRING s1, s2;
  m1 = createMove(1, 2, 2, 3);
  m2 = createPawnMove(1, 3, 4, 5, WHITE_PAWN);
  s1 = MoveToString(m1);
  s2 = MoveToString(m2);

  printf("%s\n", s1);
  printf("%s\n", s2);
  SafeFree(s1);
  SafeFree(s2);
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


void setupPieces(char *Board) {
	int x, y;
	// setup empty spaces 
	for(x = 0; x < rows; x++ ){
		for(y = 0; y < cols; y++) {
			Board[x*cols + y] = ' ';
		}
	}
	/*
	Board[0] = 'R';
	Board[1] = 'K';
	Board[2] = 'P';
	Board[9] = 'r';
	Board[10] = 'k';
	Board[11] = 'p';
	*/
	/*	
	  // setup pawns 
	 for(y = 0; y < cols; y++ ){
	   Board[1*cols + y] = WHITE_PAWN;
	   Board[(rows-2)*cols + y] = BLACK_PAWN;
	 }
	 // setup black major pieces 
	 Board[(rows-1)*cols] = BLACK_ROOK;
	 Board[(rows-1)*cols + 1] = BLACK_BISHOP;
	 Board[(rows-1)*cols + 2] = BLACK_KING;
	 Board[(rows-1)*cols + 3] = BLACK_QUEEN;
	 Board[(rows-1)*cols + 4] = BLACK_KNIGHT;
	 // setup white major pieces 
	 Board[0] = WHITE_ROOK;
	 Board[1] = WHITE_BISHOP;
	 Board[2] = WHITE_KING;
	 Board[3] = WHITE_QUEEN;
	 Board[4] = WHITE_KNIGHT;
	*/

	  // setup pawns 
	/*
	Board[4] = WHITE_PAWN;
	Board[10] = BLACK_PAWN;
	*/
	/*
	for(y = 0; y < cols; y++ ){
	  Board[1*cols + y] = WHITE_PAWN;
	  Board[(rows-2)*cols + y] = BLACK_PAWN;
	}
	Board[0] = WHITE_BISHOP;
	Board[1] = WHITE_KING;
	Board[2] = WHITE_ROOK;
	
	Board[(rows-1)*cols] = BLACK_BISHOP;
	Board[(rows-1)*cols + 1] = BLACK_KING;
	Board[(rows-1)*cols + 2] = BLACK_ROOK;
	*/
	Board[0] = WHITE_BISHOP;
	Board[1] = WHITE_KING;
	Board[2] = WHITE_ROOK;
	//	Board[3] = WHITE_PAWN;
     
	//     Board[5] = WHITE_PAWN;

	//	Board[9] = BLACK_PAWN;
	//	Board[11] = BLACK_PAWN;
	Board[9] = BLACK_ROOK;
	Board[10] = BLACK_KING;
	Board[11] = BLACK_BISHOP;
}


/*  This function checks if the board is in check.
*/
BOOLEAN inCheck(POSITION N, int currentPlayer) { 
	int i, j; 
	char piece;  
	char bA[rows*cols];
	
	generic_unhash(N,bA);
	for (i = 0; i < rows; i++) {
		for(j = 0; j < cols; j++) {
			piece = bA[i*cols +j]; 
			switch (piece) { 
				case WHITE_QUEEN: case BLACK_QUEEN:  
					if (queenCheck(bA, i, j, currentPlayer,piece, WHITE_QUEEN , BLACK_QUEEN) == TRUE) { 
						return TRUE; 
					} else { 
						break;    				 
					} 
				case WHITE_BISHOP: case BLACK_BISHOP: 
					if (bishopCheck(bA, i,j, currentPlayer, piece, WHITE_BISHOP , BLACK_BISHOP) == TRUE) { 
						return TRUE; 
					} else { 
						break; 
					} 
				case WHITE_ROOK: case BLACK_ROOK:
					if (rookCheck(bA, i, j, currentPlayer, piece, WHITE_ROOK , BLACK_ROOK) == TRUE) { 
						return TRUE; 
					} else { 
						break; 
					} 
				case WHITE_KNIGHT: case BLACK_KNIGHT:
					if (knightCheck(bA, i, j, currentPlayer, piece, WHITE_KNIGHT , BLACK_KNIGHT) == TRUE) { 
						return TRUE; 
					} else { 
						break; 
					} 
				case WHITE_PAWN: case BLACK_PAWN:
					if (pawnCheck(bA, i,j, currentPlayer, piece, WHITE_PAWN , BLACK_PAWN) == TRUE) { 
						return TRUE; 
					} else { 
						break; 
					} 
				case WHITE_KING: case BLACK_KING:
					if (kingCheck(bA, i,j, currentPlayer, piece, WHITE_KING , BLACK_KING) == TRUE) { 
						return TRUE; 
					} else { 
						break; 
					} 
				default:  
					break; 
			} 
		} 
    } 
    return FALSE; 
} 

BOOLEAN isKingCaptureWithBreak(int *breaks, char *Board, int row, int col, int currentPlayer, char currentPiece, char whitePiece, char blackPiece) {
	if(currentPlayer == WHITE_TURN) {
		if(currentPiece == blackPiece) {
			if(Board[row*cols + col] == WHITE_KING) {
				return TRUE;
			} else if(Board[row*cols + col] != ' ') {
				*breaks = 1;
			}
		}
    } else {
		if(currentPiece == whitePiece) {
			if(Board[row*cols + col] == BLACK_KING) {
				return TRUE;
			} else if(Board[row*cols + col] != ' ') {
				*breaks = 1;
			}
		}
    }
	return FALSE;
}

BOOLEAN isKingCaptureWithoutBreak(char *Board, int row, int col, int currentPlayer, char currentPiece, char whitePiece, char blackPiece) {
	if(currentPlayer == WHITE_TURN) {
		if(currentPiece == blackPiece) {
			if(Board[row*cols + col] == WHITE_KING) {
				return TRUE;
			} 
		}
    } else {
		if(currentPiece == whitePiece) {
			if(Board[row*cols + col] == BLACK_KING) {
				return TRUE;
			} 
		}
    }
	return FALSE;
}
BOOLEAN queenCheck(char *Board, int row, int col, int currentPlayer, char currentPiece, char whitePiece, char blackPiece) {
	return (bishopCheck(Board, row, col, currentPlayer, currentPiece, whitePiece, blackPiece) || rookCheck(Board, row, col, currentPlayer, currentPiece, whitePiece, blackPiece));
}

BOOLEAN bishopCheck(char *Board, int row, int col, int currentPlayer, char currentPiece, char whitePiece, char blackPiece) {
	int rowTemp, colTemp;
	int *breaks = (int *) malloc(sizeof(int));
	
	*breaks = 0;
	rowTemp = row;
	colTemp = col;
	// up and left
	while(row-1 >= 0 && col-1 >= 0) {
		row--;
		col--;
		if(isKingCaptureWithBreak(breaks, Board, row, col, currentPlayer, currentPiece, whitePiece, blackPiece)){
			return TRUE;
		}
		if(*breaks == 1) break;
	}
	
	*breaks = 0;
	row = rowTemp;
	col = colTemp; 
	// up and right
	while(row-1 >= 0 && col+1 < cols) {
		row--;
		col++;
		if(isKingCaptureWithBreak(breaks, Board, row, col, currentPlayer, currentPiece, whitePiece, blackPiece)){
			return TRUE;
		}
		if(*breaks == 1) break;
	}
	
	*breaks = 0;
	row = rowTemp;
	col = colTemp;
	// down and left
	while(row+1 < rows && col-1 >= 0) {
		row++;
		col--;
		if(isKingCaptureWithBreak(breaks, Board, row, col, currentPlayer, currentPiece, whitePiece, blackPiece)) {
			return TRUE;
		}
		if(*breaks == 1) break;
	}
	
	*breaks = 0;
	row = rowTemp;
	col = colTemp;
	// down and right
	while(row+1 < rows && col+1 < cols) {
		row++;
		col++;
		if(isKingCaptureWithBreak(breaks, Board, row, col, currentPlayer, currentPiece, whitePiece, blackPiece)){
			return TRUE;
		}
		if(*breaks == 1) break;
		
	}
	return FALSE;
}

BOOLEAN knightCheck(char *Board, int row, int col, int currentPlayer, char currentPiece, char whitePiece, char blackPiece) {
	int rowTemp, colTemp;
	
	rowTemp = row;
	colTemp = col;
	
	// up two left one
	if(row-2 >= 0 && col-1 >= 0) {
		if(isKingCaptureWithoutBreak(Board, row-2, col-1, currentPlayer, currentPiece, whitePiece, blackPiece)){
			return TRUE;
		}
	}
	
	// up two right one
	if(row-2 >= 0 && col+1 < cols) {
		if(isKingCaptureWithoutBreak(Board, row-2, col+1, currentPlayer, currentPiece, whitePiece, blackPiece)){
			return TRUE;
		}
	}
	
	// right two up one
	if(col+2 < cols && row-1 >= 0) {
		if(isKingCaptureWithoutBreak(Board, row-1, col+2, currentPlayer, currentPiece, whitePiece, blackPiece)){
			return TRUE;
		}
	}
	
	// right two down one
	if(col+2 < cols && row+1 < rows) {
		if(isKingCaptureWithoutBreak(Board, row+1, col+2, currentPlayer, currentPiece, whitePiece, blackPiece)){
			return TRUE;
		}
	}
	
	// down two left one
	if(row+2 < rows && col-1 >= 0) {
		if(isKingCaptureWithoutBreak(Board, row+2, col-1, currentPlayer, currentPiece, whitePiece, blackPiece)){
			return TRUE;
		}
	}
	
	// down two right one
	if(row+2 < rows && col+1 < cols) {
		if(isKingCaptureWithoutBreak(Board, row+2, col+1, currentPlayer, currentPiece, whitePiece, blackPiece)){
			return TRUE;
		}
	}
	
	// left two up one
	if(col-2 >= 0 && row-1 >= 0) {
		if(isKingCaptureWithoutBreak(Board, row-1, col-2, currentPlayer, currentPiece, whitePiece, blackPiece)){
			return TRUE;
		}
	}
	
	// left two down one
	if(col-2 >= 0 && row+1 < rows) {
		if(isKingCaptureWithoutBreak(Board, row+1, col-2, currentPlayer, currentPiece, whitePiece, blackPiece)){
			return TRUE;
		}
	}
	return FALSE;
}

BOOLEAN rookCheck(char *Board, int row, int col, int currentPlayer, char currentPiece, char whitePiece, char blackPiece) {
	int rowTemp, colTemp;
	int *breaks = (int *) malloc(sizeof(int));
	
	*breaks = 0;
	rowTemp = row;
	colTemp = col;
	// up 
	while(row-1 >= 0) {
		row--;
		
		if(isKingCaptureWithBreak(breaks, Board, row, col, currentPlayer, currentPiece, whitePiece, blackPiece)){
			return TRUE;
		}
		if(*breaks == 1) break;
	}
	
	*breaks = 0;
	row = rowTemp;
	col = colTemp;
	// right
	while(col+1 < cols) {
		col++;
		if(isKingCaptureWithBreak(breaks, Board, row, col, currentPlayer, currentPiece, whitePiece, blackPiece)){
			return TRUE;
		}
		if(*breaks == 1) break;
	}
	
	*breaks = 0;
	col = colTemp;
	row = rowTemp;
	// left
	while(col-1 >= 0) {
		col--;
		if(isKingCaptureWithBreak(breaks, Board, row, col, currentPlayer, currentPiece, whitePiece, blackPiece)) {
			return TRUE;
		}
		if(*breaks == 1) break;
	}
	
	*breaks = 0;
	col = colTemp;
	row = rowTemp;
	// down 
	while(row+1 < rows) {
		row++;
		if(isKingCaptureWithBreak(breaks, Board, row, col, currentPlayer, currentPiece, whitePiece, blackPiece)) {
			return TRUE;
		}
		if(*breaks == 1) break;
	}
	return FALSE;
}
BOOLEAN pawnCheck(char *Board, int row, int col, int currentPlayer, char currentPiece, char whitePiece, char blackPiece) { 
	if(currentPlayer == WHITE_TURN) { 
		if(col-1 >= 0) {
			if(isKingCaptureWithoutBreak(Board, row-1, col-1, currentPlayer, currentPiece, whitePiece, blackPiece)) {
				return TRUE;
			}
		}
		if(col+1 < cols) {
			if(isKingCaptureWithoutBreak(Board, row-1, col+1, currentPlayer, currentPiece, whitePiece, blackPiece)) {
				return TRUE;
			}
		}
	} else {
		if(col-1 >= 0) {
			if(isKingCaptureWithoutBreak(Board, row+1, col-1, currentPlayer, currentPiece, whitePiece, blackPiece)) {
				return TRUE;
			}
		}
		if(col+1 < cols) {
			if(isKingCaptureWithoutBreak(Board, row+1, col+1, currentPlayer, currentPiece, whitePiece, blackPiece)) {
				return TRUE;
			}
		}
	}
	return FALSE;
}

BOOLEAN kingCheck(char *Board, int row, int col, int currentPlayer, char currentPiece, char whitePiece, char blackPiece) { 
	// Left
	if(col > 0) {
		if(isKingCaptureWithoutBreak(Board, row, col-1, currentPlayer, currentPiece, whitePiece, blackPiece)) {
			return TRUE;
		}
	}
	// Right
	if(col < cols-1) {
		if(isKingCaptureWithoutBreak(Board, row, col+1, currentPlayer, currentPiece, whitePiece, blackPiece)) {
			return TRUE;
		}
	}
	// Up
	if(row > 0) {
		if(isKingCaptureWithoutBreak(Board, row-1, col, currentPlayer, currentPiece, whitePiece, blackPiece)) {
			return TRUE;
		}
	}
	// Down
	if(row < rows-1) {
		if(isKingCaptureWithoutBreak(Board, row+1, col, currentPlayer, currentPiece, whitePiece, blackPiece)) {
			return TRUE;
		}
	}
	// Up-left
	if(row > 0 && col > 0) {
		if(isKingCaptureWithoutBreak(Board, row-1, col-1, currentPlayer, currentPiece, whitePiece, blackPiece)) {
			return TRUE;
		}
	}	
	// Up-right
	if(row > 0 && col < cols-1) {
		if(isKingCaptureWithoutBreak(Board, row-1, col+1, currentPlayer, currentPiece, whitePiece, blackPiece)) {
			return TRUE;
		}
	}	
	// Down-left
	if(row < rows-1 && col > 0) {
		if(isKingCaptureWithoutBreak(Board, row+1, col-1, currentPlayer, currentPiece, whitePiece, blackPiece)) {
			return TRUE;
		}
	}
	// Down-right
	if(row < rows-1 && col < cols-1) {
		if(isKingCaptureWithoutBreak(Board, row+1, col+1, currentPlayer, currentPiece, whitePiece, blackPiece)) {
			return TRUE;
		}
	}	
	return FALSE;
}

/* Returns the string of base 'base' as an integer 
PRECONDITION: s must end in a null character, and contain only the digits 
0-9, and the letters a-f (hexadecimal) */ 
int atobi(char s[], int base) { 
	int i; 
	int total = 0; 
	char c; 
	for (i = 0; s[i] != '\0'; i++) { 
		c = s[i]; 
		total = total*base + ((isalpha(c)) ? toupper(c)-'A'+10:c-'0'); 
	} 
	return total; 
} 

void generateKingMoves(char *boardArray, MOVELIST **moves, int currentPlayer, int i, int j){
	MOVE newMove;
	//UP
	if (i != 0 && !isSameTeam(boardArray[(i-1)*cols + j], currentPlayer)) {
		newMove = createMove(i, j, i-1, j);
		if (testMove(boardArray, i,i-1,j,j, currentPlayer)) {
			*moves = CreateMovelistNode(newMove, *moves);	
		}
	}
	//Down
	if (i != rows-1 && !isSameTeam(boardArray[(i+1)*cols + j], currentPlayer)) {
		newMove = createMove(i, j, i+1, j);
		if (testMove(boardArray, i,i+1,j,j, currentPlayer)) {
			*moves = CreateMovelistNode(newMove, *moves);	
		}
	}
	//Left
	if (j != 0 && !isSameTeam(boardArray[i*cols + j-1], currentPlayer)) {
		newMove = createMove(i, j, i, j-1 );
		if (testMove(boardArray, i,i,j,j-1, currentPlayer)) {
			*moves = CreateMovelistNode(newMove, *moves);	
		}
	}
	//Right
	if (j != cols-1 && !isSameTeam(boardArray[i*cols + j+1], currentPlayer)) {
		newMove = createMove(i, j, i, j+1 );
		if (testMove(boardArray, i,i,j,j+1, currentPlayer)) {
			*moves = CreateMovelistNode(newMove, *moves);		
		}
	}
	//Up-left
	if (i != 0 && j != 0 && !isSameTeam(boardArray[(i-1)*cols + j-1], currentPlayer)) {
		newMove = createMove(i, j, i-1, j-1 );
		if (testMove(boardArray, i,i-1,j,j-1, currentPlayer)) {
			*moves = CreateMovelistNode(newMove, *moves);	
		}
	}
	//Up-right
	if (i != 0 && j != cols-1 && !isSameTeam(boardArray[(i-1)*cols + j+1], currentPlayer)) {
		newMove = createMove(i, j, i-1, j+1 );
		if (testMove(boardArray, i,i-1,j,j+1, currentPlayer)) {
			*moves = CreateMovelistNode(newMove, *moves);	
		}
	}
	//Down-left
	if (i != rows-1 && j != 0 && !isSameTeam(boardArray[(i+1)*cols + j-1], currentPlayer)) {
		newMove = createMove(i, j, i+1, j-1 );
		if (testMove(boardArray, i,i+1,j,j-1, currentPlayer)) {
			*moves = CreateMovelistNode(newMove, *moves);	
		}
	}	
	//Down-right
	if (i != rows-1 && j != cols-1 && !isSameTeam(boardArray[(i+1)*cols + j+1], currentPlayer)) {
		newMove = createMove(i, j, i+1, j+1 );
		if (testMove(boardArray, i,i+1,j,j+1, currentPlayer)) {
			*moves = CreateMovelistNode(newMove, *moves);	
		}
	}
}

/*
** Generates the moves for a king piece.  First checks if the move is off the board.  Then
 ** checks of the piece taken, if any, is of the same team.  Then checks if the move puts
 ** the player in check.  Generates moves in all directions until it hits a piece.
 */
void generateQueenMoves(char *boardArray,  MOVELIST **moves, int currentPlayer, int i, int j){
	generateMovesDirection(boardArray, moves, currentPlayer, i, j, UP);
	generateMovesDirection(boardArray, moves, currentPlayer, i, j, DOWN);
	generateMovesDirection(boardArray, moves, currentPlayer, i, j, LEFT);
	generateMovesDirection(boardArray, moves, currentPlayer, i, j, RIGHT);
	generateMovesDirection(boardArray, moves, currentPlayer, i, j, UL);
	generateMovesDirection(boardArray, moves, currentPlayer, i, j, UR);
	generateMovesDirection(boardArray, moves, currentPlayer, i, j, DL);
	generateMovesDirection(boardArray, moves, currentPlayer, i, j, DR);
}

/*
** Generates the moves for a king piece.  First checks if the move is off the board.  Then
 ** checks of the piece taken, if any, is of the same team.  Then checks if the move puts
 ** the player in check.  Generates moves in the four diagonal directions until it hits a piece.
 */
void generateBishopMoves(char *boardArray,  MOVELIST **moves, int currentPlayer, int i, int j){
	generateMovesDirection(boardArray, moves, currentPlayer, i, j, UL);
	generateMovesDirection(boardArray, moves, currentPlayer, i, j, UR);
	generateMovesDirection(boardArray, moves, currentPlayer, i, j, DL);
	generateMovesDirection(boardArray, moves, currentPlayer, i, j, DR);
}

/*
** Generates the moves for a king piece.  First checks if the move is off the board.  Then
 ** checks of the piece taken, if any, is of the same team.  Then checks if the move puts
 ** the player in check.  Generates moves in 4 compass directions until it hits a piece.
 */
void generateRookMoves(char *boardArray,  MOVELIST **moves, int currentPlayer, int i, int j){
	generateMovesDirection(boardArray, moves, currentPlayer, i, j, UP);
	generateMovesDirection(boardArray, moves, currentPlayer, i, j, DOWN);
	generateMovesDirection(boardArray, moves, currentPlayer, i, j, LEFT);
	generateMovesDirection(boardArray, moves, currentPlayer, i, j, RIGHT);
}

/*
** Generates the moves for a king piece.  First checks if the move is off the board.  Then
 ** checks of the piece taken, if any, is of the same team.  Then checks if the move puts
 ** the player in check.  There are 8 possible moves for a knight.  The first direction indicates
 ** 2 blocks of move, the second direction is one block.
 */
void generateKnightMoves(char *boardArray,  MOVELIST **moves, int currentPlayer, int i, int j){
	MOVE newMove;
	//Left 2, Down 1
	if (i != rows-1 && j > 1 && !isSameTeam(boardArray[(i+1)*cols + j-2], currentPlayer)) {
		newMove = createMove(i, j, i+1, j-2 );
		if (testMove(boardArray, i,i+1,j,j-2, currentPlayer)) {
			*moves = CreateMovelistNode(newMove, *moves);	
		}
	}	
	//Right 2, Down 1
	if (i != rows-1 && j < cols-1 && !isSameTeam(boardArray[(i+1)*cols + j+2], currentPlayer)) {
		newMove = createMove(i, j, i+1, j+2 );
		if (testMove(boardArray, i,i+1,j,j+2, currentPlayer)) {
			*moves = CreateMovelistNode(newMove, *moves);
		}
	}	
	//Down 2, Left 1
	if (i < rows-1 && j != 0 && !isSameTeam(boardArray[(i+2)*cols + j-1], currentPlayer)) {
		newMove = createMove(i, j, i+2, j-1);
		if (testMove(boardArray, i,i+2,j,j-1, currentPlayer)) {
			*moves = CreateMovelistNode(newMove, *moves);
		}
	}
	//Down 2, Right 1
	if (i < rows-1 && j != cols-1 && !isSameTeam(boardArray[(i+2)*cols + j+1], currentPlayer)) {
		newMove = createMove(i, j, i+2, j+1);
		if (testMove(boardArray, i,i+2,j,j+1, currentPlayer)) {
			*moves = CreateMovelistNode(newMove, *moves);
		}
	}	
	//Left 2, Up 1
	if (i != 0 && j > 1 && !isSameTeam(boardArray[(i-1)*cols + j-2], currentPlayer)) {
		newMove = createMove(i, j, i-1, j-2 );
		if (testMove(boardArray, i,i-1,j,j-2, currentPlayer)) {
			*moves = CreateMovelistNode(newMove, *moves);
		}
	}
	//Right 2, Up 1
	if (i != 0 && j < cols-1 && !isSameTeam(boardArray[(i-1)*cols + j+2], currentPlayer)) {
		newMove = createMove(i, j, i-1, j+2 );
		if (testMove(boardArray, i,i-1,j,j+2, currentPlayer)) {
			*moves = CreateMovelistNode(newMove, *moves);
		}
	}	
	//Up 2, Left 1
	if (i > 1 && j != 0 && !isSameTeam(boardArray[(i-2)*cols + j-1], currentPlayer)) {
		newMove = createMove(i, j, i-2, j-1 );
		if (testMove(boardArray, i,i-2,j,j-1, currentPlayer)) {
			*moves = CreateMovelistNode(newMove, *moves);
		}
	}	
	//Up 2, Right 1
	if (i > 1 && j != cols-1 && !isSameTeam(boardArray[(i-2)*cols + j+1], currentPlayer)) {
		newMove = createMove(i, j, i-2, j+1 );
		if (testMove(boardArray, i, i-2, j,j+1, currentPlayer)) {
			*moves = CreateMovelistNode(newMove, *moves);
		}
	}		
}

/*
** Generates the moves for a king piece.  First checks if the move is off the board.  Then
 ** checks of the piece taken, if any, is of the same team.  Then checks if the move puts
 ** the player in check.  Calculates moves forward one piece if there is no piece there
 ** or moves diagonal forward if there is an opposing player piece there.
 */
void generatePawnMoves(char *boardArray,  MOVELIST **moves, int currentPlayer, int i, int j){
	
	if (currentPlayer == WHITE_TURN){
		//down 1.  Only a legal move if there is no piece there
		if (i != rows-1 && boardArray[(i+1)*cols + j] == ' ') {
			MOVE newMove = createMove(i, j, i+1, j);
			if (testMove(boardArray, i,i+1,j,j, currentPlayer)) {
				*moves = CreateMovelistNode(newMove, *moves);
				// check if pawn is at the end
				if(i + 1 == rows-1) {
				  if(replacement(boardArray, WHITE_QUEEN)) {
				    newMove = createPawnMove(i, j, i+1, j, WHITE_QUEEN);
				    *moves = CreateMovelistNode(newMove, *moves);
				  }
				  if(replacement(boardArray, WHITE_ROOK)){
				    newMove = createPawnMove(i, j, i+1, j, WHITE_ROOK);
				    *moves = CreateMovelistNode(newMove, *moves);
				  }
				  if(replacement(boardArray, WHITE_BISHOP)) {
				    newMove = createPawnMove(i, j, i+1, j, WHITE_BISHOP);
				    *moves = CreateMovelistNode(newMove, *moves);
				  }
				  if(replacement(boardArray, WHITE_KNIGHT)){
				    newMove = createPawnMove(i, j, i+1, j, WHITE_KNIGHT);
				    *moves = CreateMovelistNode(newMove, *moves);
				  }
				
				
				}
			}
		}
		//down 1, left 1.  Only a legal move if captures an enemy piece
		if (i != rows-1 && j != 0 && boardArray[(i+1)*cols + j-1] != ' ' && 
			!isSameTeam(boardArray[(i+1)*cols + j-1], currentPlayer)) {
			MOVE newMove = createMove(i, j, i+1, j-1);
			if (testMove(boardArray, i,i+1,j,j-1, currentPlayer)) {
				*moves = CreateMovelistNode(newMove, *moves);
				// check if pawn is at the end
				if(i + 1 == rows-1) {
				  if(replacement(boardArray, WHITE_QUEEN)) {
				    newMove = createPawnMove(i, j, i+1, j, WHITE_QUEEN);
				    *moves = CreateMovelistNode(newMove, *moves);
				  }
				  if(replacement(boardArray, WHITE_ROOK)){
				    newMove = createPawnMove(i, j, i+1, j, WHITE_ROOK);
				    *moves = CreateMovelistNode(newMove, *moves);
				  }
				  if(replacement(boardArray, WHITE_BISHOP)) {
				    newMove = createPawnMove(i, j, i+1, j, WHITE_BISHOP);
				    *moves = CreateMovelistNode(newMove, *moves);
				  }
				  if(replacement(boardArray, WHITE_KNIGHT)){
				    newMove = createPawnMove(i, j, i+1, j, WHITE_KNIGHT);
				    *moves = CreateMovelistNode(newMove, *moves);
				  }
				}
			}	
		}
		//down 1, right 1.  Only a legal move if it captures an enemy piece
		if (i != rows-1 && j != cols-1 && boardArray[(i+1)*cols + j+1] != ' ' && 
			!isSameTeam(boardArray[(i+1)*cols + j+1], currentPlayer)) {
			MOVE newMove = createMove(i, j, i+1, j+1);
			if (testMove(boardArray, i, i+1, j, j+1, currentPlayer)) {
				*moves = CreateMovelistNode(newMove, *moves);
				// check if pawn is at the end
				if(i + 1 == rows-1) {
				  if(replacement(boardArray, WHITE_QUEEN)) {
				    newMove = createPawnMove(i, j, i+1, j, WHITE_QUEEN);
				    *moves = CreateMovelistNode(newMove, *moves);
				  }
				  if(replacement(boardArray, WHITE_ROOK)){
				    newMove = createPawnMove(i, j, i+1, j, WHITE_ROOK);
				    *moves = CreateMovelistNode(newMove, *moves);
				  }
				  if(replacement(boardArray, WHITE_BISHOP)) {
				    newMove = createPawnMove(i, j, i+1, j, WHITE_BISHOP);
				    *moves = CreateMovelistNode(newMove, *moves);
				  }
				  if(replacement(boardArray, WHITE_KNIGHT)){
				    newMove = createPawnMove(i, j, i+1, j, WHITE_KNIGHT);
				    *moves = CreateMovelistNode(newMove, *moves);
				  }
				} 
			}
		}
	} else {
		//up 1.  Only a legal move if there is no piece there
		if (i != 0 && boardArray[(i-1)*cols + j] == ' ') {
			MOVE newMove = createMove(i, j, i-1, j);
			if (testMove(boardArray, i, i-1, j, j, currentPlayer)) {
				*moves = CreateMovelistNode(newMove, *moves);
				// check if pawn is at the end
				if(i-1 == 0) {
				  if(replacement(boardArray, BLACK_QUEEN)) {
				    newMove = createPawnMove(i, j, i+1, j, BLACK_QUEEN);
				    *moves = CreateMovelistNode(newMove, *moves);
				  }
				  if(replacement(boardArray, BLACK_ROOK)){
				    newMove = createPawnMove(i, j, i+1, j, BLACK_ROOK);
				    *moves = CreateMovelistNode(newMove, *moves);
				  }
				  if(replacement(boardArray, BLACK_BISHOP)) {
				    newMove = createPawnMove(i, j, i+1, j, BLACK_BISHOP);
				    *moves = CreateMovelistNode(newMove, *moves);
				  }
				  if(replacement(boardArray, BLACK_KNIGHT)){
				    newMove = createPawnMove(i, j, i+1, j, BLACK_KNIGHT);
				    *moves = CreateMovelistNode(newMove, *moves);
				  }
				} 
			}
		}
		//up 1, left 1.  Only a legal move if captures an enemy piece
		if (i != 0 && j != 0 && boardArray[(i-1)*cols + j-1] != ' ' && 
			!isSameTeam(boardArray[(i-1)*cols + j-1], currentPlayer)) {
			MOVE newMove = createMove(i, j, i-1, j-1);
			if (testMove(boardArray, i, i-1,j,j-1, currentPlayer)) {
				*moves = CreateMovelistNode(newMove, *moves);
				// check if pawn is at the end
				if(i-1 == 0) {
				  if(replacement(boardArray, BLACK_QUEEN)) {
				    newMove = createPawnMove(i, j, i+1, j, BLACK_QUEEN);
				    *moves = CreateMovelistNode(newMove, *moves);
				  }
				  if(replacement(boardArray, BLACK_ROOK)){
				    newMove = createPawnMove(i, j, i+1, j, BLACK_ROOK);
				    *moves = CreateMovelistNode(newMove, *moves);
				  }
				  if(replacement(boardArray, BLACK_BISHOP)) {
				    newMove = createPawnMove(i, j, i+1, j, BLACK_BISHOP);
				    *moves = CreateMovelistNode(newMove, *moves);
				  }
				  if(replacement(boardArray, BLACK_KNIGHT)){
				    newMove = createPawnMove(i, j, i+1, j, BLACK_KNIGHT);
				    *moves = CreateMovelistNode(newMove, *moves);
				  }
				} 
			}
		}
		//up 1, right 1.  Only a legal move if captures an enemy piece
		if (i != 0 && j != cols-1 && boardArray[(i-1)*cols + j+1] != ' ' && 
			!isSameTeam(boardArray[(i-1)*cols + j+1], currentPlayer)) {
			MOVE newMove = createMove(i, j, i-1, j+1);
			if (testMove(boardArray, i, i-1, j, j+1, currentPlayer)) {
				*moves = CreateMovelistNode(newMove, *moves);
				// check if pawn is at the end
				if(i-1 == 0) {
				  if(replacement(boardArray, BLACK_QUEEN)) {
				    newMove = createPawnMove(i, j, i+1, j, BLACK_QUEEN);
				    *moves = CreateMovelistNode(newMove, *moves);
				  }
				  if(replacement(boardArray, BLACK_ROOK)){
				    newMove = createPawnMove(i, j, i+1, j, BLACK_ROOK);
				    *moves = CreateMovelistNode(newMove, *moves);
				  }
				  if(replacement(boardArray, BLACK_BISHOP)) {
				    newMove = createPawnMove(i, j, i+1, j, BLACK_BISHOP);
				    *moves = CreateMovelistNode(newMove, *moves);
				  }
				  if(replacement(boardArray, BLACK_KNIGHT)){
				    newMove = createPawnMove(i, j, i+1, j, BLACK_KNIGHT);
				    *moves = CreateMovelistNode(newMove, *moves);
				  }
				} 
			}
		}
	}
}




/*
Given a board and a list of moves, this function will create all moves
 in that direction until it hits another piece.  This is for the queen, bishop,
 and rook.  These three pieces can move in directions any number of spaces until
 they hit another piece.  They can only take that piece if it is of the other team.
 POSTCONDITION: moves is updated with all of the legal moves.
 */
void generateMovesDirection(char* boardArray,  MOVELIST **moves, int currentPlayer, int i, int j, int direction) { 
	int i_inc=0, j_inc=0,  new_i=i, new_j=j;
	char piece; 
	switch (direction){ 
		case UP:  
			i_inc = -1; 
			break; 
		case DOWN:  
			i_inc = 1; 
			break; 
		case LEFT:  
			j_inc = -1; 
			break; 
		case RIGHT:  
			j_inc = 1; 
			break; 
		case UL:  
			i_inc = -1; 
			j_inc = -1; 
			break; 
		case UR:  
			i_inc = -1; 
			j_inc = 1;  
			break; 
		case DL:  
			i_inc = 1; 
			j_inc = -1; 
			break; 
		case DR:  
			i_inc = 1; 
			j_inc = 1; 
			break; 
	} 
	new_i += i_inc; 
	new_j += j_inc; 
	MOVE newMove;
	while(new_i < rows && new_i >= 0 && new_j < cols && new_j >= 0){ 
		piece = boardArray[new_i*cols+new_j]; 
		if (!isSameTeam(piece, currentPlayer)) {
			newMove = createMove(i, j, new_i, new_j);
			if (testMove(boardArray, i, new_i, j, new_j, currentPlayer)) {
				*moves = CreateMovelistNode(newMove, *moves);
			}
		}
		if (piece != ' ') {
			return;
		}
		new_i += i_inc; 
		new_j += j_inc; 
	} 
} 

MOVE createMove(int rowi, int coli, int rowf, int colf){
	MOVE m;
	coli += 10;
	colf += 10;
	rowi = rows - rowi;
	rowf = rows - rowf;
	m = (coli << 12) | (rowi << 8) | (colf << 4) | rowf;
	return m;
}

MOVE createPawnMove(int rowi, int coli, int rowf, int colf, char replacementPiece){
	MOVE m;
	coli += 10;
	colf += 10;
	rowi = rows - rowi;
	rowf = rows - rowf;
	m = (replacementPiece << 16) | (coli << 12) | (rowi << 8) | (colf << 4) | rowf;
	return m;
}

/* 
** Tests to see if a move is valid by checking to see if the new board is in check
** Returns false if the move puts the player in check.
*/
BOOLEAN testMove(char *boardArray, int rowi, int rowf, int coli, int colf, int currentPlayer){
	BOOLEAN boardInCheck;
	char overwrittenPiece = boardArray[rowf*cols + colf];
	testDoMove(boardArray, rowi, rowf, coli, colf, currentPlayer);
	POSITION bP = generic_hash(boardArray, currentPlayer);
	boardInCheck = inCheck(bP, currentPlayer);
	UndoMove(boardArray, rowi, rowf, coli, colf, currentPlayer);
	boardArray[rowf*cols + colf] = overwrittenPiece;
	if (boardInCheck == FALSE) {
		return TRUE;
	} else {
		return FALSE;
	}
}

void testDoMove(char *boardArray, int rowi, int rowf, int coli, int colf, int currentPlayer){
	char tempPiece;
	tempPiece = boardArray[rowi*cols + coli];
	boardArray[rowi*cols + coli] = ' ';
	boardArray[rowf*cols + colf] = tempPiece;
	if(currentPlayer == WHITE_TURN) {
		currentPlayer = BLACK_TURN;
	} else currentPlayer = WHITE_TURN;
}
void UndoMove (char *boardArray,int rowi, int rowf, int coli, int colf, int currentPlayer)
{
	char tempPiece;
	tempPiece = boardArray[rowf*cols + colf];
	boardArray[rowf*cols + colf] = ' ';
	boardArray[rowi*cols + coli] = tempPiece;
	
	
	if(currentPlayer == WHITE_TURN) {
		currentPlayer = BLACK_TURN;
	} else currentPlayer = WHITE_TURN;
}

/* Returns TRUE if the given piece belongs to currentPlayer, 
and FALSE otherwise. */ 
BOOLEAN isSameTeam(char piece, int currentPlayer) { 
	if (piece >= 'a' && piece <= 'z' && currentPlayer == BLACK_TURN) 
		return TRUE; 
	else if (piece >= 'A' && piece <= 'Z' && currentPlayer == WHITE_TURN) 
		return TRUE; 
	else  
		return FALSE; 
} 

/* Used for testing.  this function will print an array, not a hash position
like PrintPosition(); */
void printArray (char* boardArray)
{	
	int x, y;	
	printf("\n"); 
	for(x = 0; x < rows; x++){ 
		printf("    +"); 
		for(y = 0; y < cols; y++){ 
			printf("---+"); 
		} 
		printf("\n"); 
		printf("  %d |", x); 
		for(y = 0; y < cols; y++){ 
			printf(" %c |", boardArray[x*cols + y]); 
		} 
		printf("\n"); 
	} 
	printf("    +"); 
	for(y = 0; y < cols; y++){ 
		printf("---+"); 
	} 
	printf("\n"); 
	printf("     "); 
	for(y = 0; y < cols; y++){ 
		printf(" %d  ", y); 
	}
	printf("\n"); 
	printf("\n");
	
}

/* Used for testing.  This function will print a MOVELIST to show all the
possible moves for the current Board and current player */
void printMoveList(MOVELIST *moves) {
	while(moves != NULL) {
		PrintMove(moves->move);
		moves = moves->next;
	}
}

BOOLEAN replacement(char *boardArray, char replacementPiece) {
  int i, j;

  for (i = 0; i < rows; i++) {
    for (j = 0; j < cols; j++) {
      if(boardArray[i*cols + j] == replacementPiece)
	return FALSE;
    }
  }
  return TRUE;
}

/************************************************************************
**
** NAME:        GetCanonicalPosition
**
** DESCRIPTION: Go through all of the positions that are symmetrically
**              equivalent and return the SMALLEST, which will be used
**              as the canonical element for the equivalence set.
** 
** INPUTS:      POSITION position : The position return the canonical elt. of.
**
** OUTPUTS:     POSITION          : The canonical element of the set.
**
************************************************************************/

POSITION GetCanonicalPosition(position)
     POSITION position;
{
  POSITION newPosition, theCanonicalPosition;
  int i;
  
  theCanonicalPosition = position;
  
  for(i = 1 ; i < NUMSYMMETRIES ; i++) {
    if(!((isPawn(position) || isBishop(position)) && i > 1)) {
    newPosition = DoSymmetry(position, i);    /* get new */
    if(newPosition < theCanonicalPosition)    /* THIS is the one */
      theCanonicalPosition = newPosition;     /* set it to the ans */
    }
  }
  
  return(theCanonicalPosition);
}

/************************************************************************
**
** NAME:        DoSymmetry
**
** DESCRIPTION: Perform the symmetry operation specified by the input
**              on the position specified by the input and return the
**              new position, even if it's the same as the input.
** 
** INPUTS:      POSITION position : The position to branch the symmetry from.
**              int      symmetry : The number of the symmetry operation.
**
** OUTPUTS:     POSITION, The position after the symmetry operation.
**
************************************************************************/

POSITION DoSymmetry(position, symmetry)
     POSITION position;
     int symmetry;
{  
  POSITION newPosition;
switch (symmetry) { 
		case 1:  
		  newPosition = flipLR(position);
		  break;
		case 2:  
		  newPosition = flipUD(position);
		  break;
		case 3:  
		  newPosition = flipUD(flipLR(position));
		  break;
	
	}
 return newPosition; 
}

POSITION flipLR(position)
  POSITION position;
  {
    int i, j;
    char currentBoardArray[rows*cols], flippedBoardArray[rows*cols] ;
    int currentPlayer = whoseMove(position);

    generic_unhash(position, currentBoardArray);
    for(i = 0; i < rows; i++) {
      for(j = 0; j < cols; j++) {
	flippedBoardArray[(i * cols) + j] = currentBoardArray[(i * cols) + (cols - 1 - j)];
      }
    }
    
    return generic_hash(flippedBoardArray, currentPlayer);	
  }
POSITION flipUD(position)
  POSITION position;
  {
    int i, j;
    char currentBoardArray[rows*cols], flippedBoardArray[rows*cols];
    int currentPlayer = whoseMove(position);

    generic_unhash(position, currentBoardArray);
    for(i = 0; i < rows; i++) {
      for(j = 0; j < cols; j++) {
	flippedBoardArray[(i * cols) + j] = currentBoardArray[((rows-1-i) * cols) + j];
      }
    }
    return generic_hash(flippedBoardArray, currentPlayer);	
  }

BOOLEAN isPawn(position)
  POSITION position;
 {
   int i, j;
   char piece, boardArray[rows*cols];
   generic_unhash(position, boardArray);
   for(i = 0; i < rows; i++) {
     for(j = 0; j < cols; j++) {
       piece = boardArray[(i * cols) + j];
       if(piece == WHITE_PAWN || piece == BLACK_PAWN )
	 return TRUE;
     }
   }
   return FALSE;	
 }

BOOLEAN isBishop(position)
  POSITION position;
 {
   int i, j;
   char piece, boardArray[rows*cols];
   generic_unhash(position, boardArray);
   for(i = 0; i < rows; i++) {
     for(j = 0; j < cols; j++) {
       piece = boardArray[(i * cols) + j];
       if(piece == WHITE_BISHOP || piece == BLACK_BISHOP)
	 return TRUE;
     }
   }
   return FALSE;	
 }
/*
TIER gTierValue(position)
     POSITION position;
{
  int i,j, numPieces, piecesArray[DISTINCT_PIECES];

  piecesArray = gPiecesArray(position);
  for(i = 0; i < DISTINCT_PIECES; i++) {
    numPieces += piecesArray[i];
  }
  switch (numPieces) { 
   case 2:  
   return 0;
    break;
  case 3:  
    for(i = 1; i <= DISTINCT_PIECES; i++) {
      if(piecesArray[i - 1] > 0) {
	return i;
      }
		  }
    break;
  case 4:
    for(i = 0; i < DISTINCT_PIECES - 2; i++) {
      if(piecesArray[i] > 0) {
	for(j = i + 1; i < DISTINCT_PIECES - 2; i++) {
	  if(piecesArray[j] > 0) 
	    return 11;
	}
      }
    }
    break;
  }
}

int[] gPiecesArray(position)
     POSITION position;
{
  int i, j, piecesArray[DISTINCT_PIECES];
  char piece, boardArray[rows*cols];
  for(i = 0; i < DISTINCT_PIECES; i++){
    piecesArray[i] = 0;
  }
  generic_unhash(position, boardArray);
  for(i = 0; i < rows; i++) {
    for(j = 0; j < cols; j++) {
      piece = boardArray[(i * cols) + j];
      if(piece != ' ') {
	switch (piece) { 
	case WHITE_QUEEN: 
	  piecesArray[0] += 1;
	  break;
	case BLACK_QUEEN:  
	  piecesArray[4] += 1;
	  break;
	case WHITE_BISHOP: 
	  piecesArray[1] += 1;
	  break;
	case BLACK_BISHOP:
	  piecesArray[5] += 1;
	  break;
	case WHITE_ROOK:
	  piecesArray[2] += 1;
	  break;
	case BLACK_ROOK:
	  piecesArray[6] += 1;
	  break;
	case WHITE_KNIGHT: 
	  piecesArray[3] += 1;
	  break;
	case BLACK_KNIGHT:
	  piecesArray[7] += 1;
	  break;
	case WHITE_PAWN: 
	  piecesArray[8] += 1;
	  break;
	case BLACK_PAWN:
	  piecesArray[9] += 1;
	  break;
	default:  
	  break; 
	} 
      }
	 
    }
  }
  return piecesArray;
}
*/
STRING MoveToString(move)
MOVE move;
{
  char rowf, colf, rowi, coli, replacementPiece;
  STRING moveStr = (STRING) SafeMalloc(sizeof(char)*4);
  rowf = (move & 15) + 48; 
  colf = ((move >> 4) & 15) - 10 + 97; 
  rowi = ((move >> 8) & 15) + 48; 
  coli = ((move >> 12) & 15) - 10 + 97; 
  replacementPiece =  move >> 16;
  if (replacementPiece == 0) {
    sprintf(moveStr, "%c%c%c%c", coli, rowi, colf, rowf);
  } else {
    printf(moveStr, "%c%c%c%c=%c", coli, rowi, colf, rowf, replacementPiece);		
  }
  return moveStr;
}
/*
POSITION gUndoMove(POSITION position, UNDOMOVE umove) {
  	char boardArray[rows*cols];
	char tempPiece, replacementPiece, capturedPiece;
	int rowi, coli, rowf, colf;
	rowf = umove & 15; 
	colf = (umove >> 4) & 15; 
	rowi = (umove >> 8) & 15; 
	coli = (umove >> 12) & 15; 
	replacementPiece = (char) (umove >> 16) & 255;
	capturedPiece = (char) (umove >> 24) & 255;

	generic_unhash(position, boardArray);
	int currentPlayer = whoseMove(position);
	tempPiece = boardArray[(rows-rowf)*cols + (colf - 10)];
	boardArray[(rows - rowf)*cols + (colf - 10)] = capturedPiece;
	if(replacementPiece != WHITE_QUEEN || replacementPiece != BLACK_QUEEN || 
	   replacementPiece != WHITE_BISHOP || replacementPiece != BLACK_BISHOP || 
	   replacementPiece != WHITE_ROOK || replacementPiece != BLACK_ROOK || 
	   replacementPiece != WHITE_KNIGHT || replacementPiece != BLACK_KNIGHT) { 
	  if(currentPlayer == WHITE_TURN) 
	    boardArray[(rows - rowi)*cols + (coli - 10)] = WHITE_PAWN;
	  else boardArray[(rows - rowi)*cols + (coli - 10)] = BLACK_PAWN;
	} else boardArray[(rows - rowi)*cols + (coli - 10)] = tempPiece;
	
	if(currentPlayer == WHITE_TURN) {
	  currentPlayer = BLACK_TURN;
	} else currentPlayer = WHITE_TURN;
	return generic_hash(boardArray, currentPlayer);	
	
}
*/
/*
void substitutePawn(char *boardArray, int currentPlayer, int x, int y){

  char piece;
  printf("Choose a piece to replace your pawn with from your already captured pieces, (i.e.Q, B, R, etc.NO SPACES!):");
	piece = getchar();
	if(currentPlayer == WHITE_TURN) {
		if(isWhiteReplacementValid(piece, boardArray)) {
			boardArray[x*cols + y] = piece;
		}
	} else {
		if(isBlackReplacementValid(piece, boardArray)) {
			boardArray[x*cols + y] = piece;
		}
	}
}



BOOLEAN isWhiteReplacementValid(char piece, char *bA) {
	int x, y;
	for(x=0; x<rows; x++) {
		for(y=0; y<cols; y++) {
			if(bA[x*cols + y] == piece)
				return TRUE;
		}
	}
	return FALSE;
}


BOOLEAN isBlackReplacementValid(char piece, char *bA) {
	int x, y;
	for(x=0; x<rows; x++) {
		for(y=0; y<cols; y++) {
			if(bA[x*cols + y] == piece)
				return TRUE;
		}
	}
	return FALSE;
}
*/


// $Log: not supported by cvs2svn $
// Revision 1.8  2006/04/16 10:34:37  vert84
// *** empty log message ***
//
// Revision 1.7  2006/04/11 04:19:45  vert84
// *** empty log message ***
//
// Revision 1.6  2006/04/11 01:38:35  vert84
// *** empty log message ***
//
// Revision 1.5  2006/04/11 00:39:47  runner139
// *** empty log message ***
//
// Revision 1.4  2006/03/18 04:16:10  runner139
// *** empty log message ***
//
// Revision 1.3  2006/03/12 03:56:42  vert84
// Updated description fields.
//
// Revision 1.2  2006/03/05 03:32:12  yanpeichen
// Yanpei Chen changing mquickchess.c
//
// Deleted a stray character that caused a file inclusion error.
//
// Revision 1.1  2006/03/01 01:49:21  vert84
// Added quickchess to cvs
//
// Revision 1.7  2006/01/29 09:59:47  ddgarcia
// Removed "gDatabase" reference from comment in InitializeGame
//
// Revision 1.6  2005/12/27 10:57:50  hevanm
// almost eliminated the existance of gDatabase in all files, with some declarations commented earlier that need to be hunt down and deleted from the source file.
//
// Revision 1.5  2005/10/06 03:06:11  hevanm
// Changed kDebugDetermineValue to be FALSE.
//
// Revision 1.4  2005/05/02 17:33:01  nizebulous
// mtemplate.c: Added a comment letting people know to include gSymmetries
// 	     in their getOption/setOption hash.
// mttc.c: Edited to handle conflicting types.  Created a PLAYER type for
//         gamesman.  mttc.c had a PLAYER type already, so I changed it.
// analysis.c: Changed initialization of option variable in analyze() to -1.
// db.c: Changed check in the getter functions (GetValueOfPosition and
//       getRemoteness) to check if gMenuMode is Evaluated.
// gameplay.c: Removed PlayAgainstComputer and PlayAgainstHuman.  Wrote PlayGame
//             which is a generic version of the two that uses to PLAYER's.
// gameplay.h: Created the necessary structs and types to have PLAYER's, both
// 	    Human and Computer to be sent in to the PlayGame function.
// gamesman.h: Really don't think I changed anything....
// globals.h: Also don't think I changed anything....both these I just looked at
//            and possibly made some format changes.
// textui.c: Redid the portion of the menu that allows you to choose opponents
// 	  and then play a game.  Added computer vs. computer play.  Also,
//           changed the analysis part of the menu so that analysis should
// 	  work properly with symmetries (if it is in getOption/setOption hash).
//
// Revision 1.3  2005/03/10 02:06:47  ogren
// Capitalized CVS keywords, moved Log to the bottom of the file - Elmer
//

