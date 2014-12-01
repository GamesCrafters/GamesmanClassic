// $Id: mquickchess.c,v 1.52 2007-04-05 19:16:19 max817 Exp $

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
** 25 Jun 2006 Adam:  Finished debugging getCanonical() and finished writing
**                    MoveToString()
** 26 Jun 2006 Adam:  Made comment of finalized 3x4 starting board
** 27 Jun 2006 Adam:  Wrote much of generateUndoMoves and accompanying helper functions
** 28 Jun 2006 Adam:  Wrote more of generateUndoMoves: switched white to bottom and black to top
** 29 Jun 2006 Adam:  Finished writing generateUndoMoves
** 10 Jul 2006 Adam:  Finished Debugging gUndoMove and generateUndoMoves
** 13 Jul 2006 Adam:  Finished writing and debugging gPositionToTie
** 14 Jul 2006 Adam:  Finished writing and debugging gTierChildren
** 15 Jul 2006 Adam:  Finished writing and gInitializeHashWindow
** 16 Jul 2006 Adam:  Wrote gPositionToTierPosition by generating hash context everytime
** 17 Jul 2006 Adam:  Used array "contextArray" in order to cache contexts for the various tiers. This makes
**                    the function call to gPositionToTierPosition much faster. Also wrote the order of
**                    tierlist, the order in which the game is solved.
** 18-31 Jul 2006 Adam: Testing tierGamesman; changed inCheck to be more efficient
** 2 Aug 2006 Adam: Wrote IsLegal and began add a feature to the game specific
**                  menus in order to change the board initially.
** 4 Aug 2006 Adam: Debug some of the game specific menu. Wrote hash and unhash functions.
** 5 Aug 2006 Adam: A user can now enter a customizable board! Note: for now instructions must be
**                  followed closely. A simple mistake could make it error. This will be fixed soon so
**                  that there is a high error tolerance.
** 6-7 Aug 2006 Adam: Debugged Tier Gamesman. Can now solve all 3-piece game tiers.
** 8 Aug  2006 Adam: Wrote TierToString. Changed incheck to take in board instead of position. Debugging Tier
**                   Gamesman.Prevented memory leaks. Got game to solve up the 3 major pieces stuck on tier 14
** 15 Aug 2006 Adam: Debugged hash contexts
** 06 Dec 2006 Aaron: Implementing big changes from the semester.  Fixed gamespecific menu changes of original
**                    board and solving of tiers.  Changed default to 5x6 board.
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

STRING kGameName            = "Quick Chess";   /* The name of your game */
STRING kAuthorName          = "Aaron Levitan, Adam Abed, Glenn Kim";   /* Your name(s) */
STRING kDBName              = "quickchess";   /* The name to store the database under */

BOOLEAN kPartizan            = TRUE;   /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN kGameSpecificMenu    = TRUE;   /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN kTieIsPossible       = TRUE;   /* TRUE if a tie is possible. FALSE if it is impossible.*/
BOOLEAN kLoopy               = TRUE;   /* TRUE if the game tree will have cycles (a rearranger style game). FALSE if it does not.*/

BOOLEAN kDebugMenu           = TRUE;   /* TRUE only when debugging. FALSE when on release. */
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
        "The board is arranged like a standard chess board. \n\
The rows are specified by numeric values, while the \n\
columns are specified by letters. A square is referenced \n\
by the column and then the row, i.e. b4."                                                                                                                                                                                     ;
STRING kHelpOnYourTurn =
        "";

STRING kHelpStandardObjective =
        "Try to checkmate your opponent's king.";

STRING kHelpReverseObjective =
        "Try to get your king checkmated.";

STRING kHelpTieOccursWhen =
        "A tie occurs when a player is not in check and does not have any valid moves.";

STRING kHelpExample =
        "";


/*************************************************************************
**
** #defines and structs
**
**************************************************************************/
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
#define WHITE 0
#define BLACK 1
#define MAX_TIERS 22015
#define NUM_TIERS 256
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
int rows = 4;
int cols = 3;
char *theBoard = "rkq      QKR";
int theCurrentPlayer = WHITE_TURN;

/*VARIANTS*/
BOOLEAN normalVariant = TRUE;
BOOLEAN misereVariant = FALSE;

int Tier0Context;

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
void PrintPosition (POSITION position, STRING playersName, BOOLEAN usersTurn);
void setupPieces(char *Board);
BOOLEAN inCheck(char *bA, int currentPlayer);
BOOLEAN isDirectionCheck(char *Board, int i, int j, int currentPlayer, char currentPiece, char whitePiece, char blackPiece, int direction);
BOOLEAN isKingCaptureable(char *Board, int row, int col, int currentPlayer, char currentPiece, char whitePiece, char blackPiece);
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
int opposingPlayer(int player);
POSITION gUnDoMove(POSITION position, UNDOMOVE umove);
UNDOMOVE createUndoMove(int rowi, int coli, int rowf, int colf);
UNDOMOVE createCaptureUndoMove(int rowi, int coli, int rowf, int colf, char capturedPiece);
UNDOMOVE createReplaceCaptureUndoMove(int rowi, int coli, int rowf, int colf, char capturedPiece);
BOOLEAN testUndoMove(char *boardArray, int rowi, int rowf, int coli, int colf, int currentPlayer, TIER t);
BOOLEAN testCaptureUndoMove(char *boardArray, int rowi, int rowf, int coli, int colf, int currentPlayer, char capturedPiece, TIER t);
BOOLEAN testReplaceCaptureUndoMove(char *boardArray, int rowi, int rowf, int coli, int colf, int currentPlayer, char capturedPiece, TIER t);
BOOLEAN isSpecificPiece(char* boardArray, char piece);
int specificPawnCount(char* boardArray, int playerColor);
void generateCaptureUndoMoves(char* boardArray, UNDOMOVELIST **moves, int currentPlayer, int i, int j, int new_i, int new_j, int isReplacement, TIER t);
void generateUndoMovesDirection(char* boardArray,  UNDOMOVELIST **moves, int currentPlayer, int i, int j, int direction, TIER t);
void generateRookUndoMoves(char *boardArray,  UNDOMOVELIST **moves, int currentPlayer, int i, int j, TIER t);
void generateBishopUndoMoves(char *boardArray,  UNDOMOVELIST **moves, int currentPlayer, int i, int j, TIER t);
void generateQueenUndoMoves(char *boardArray,  UNDOMOVELIST **moves, int currentPlayer, int i, int j, TIER t);
void generateKnightUndoMoves(char *boardArray,  UNDOMOVELIST **moves, int currentPlayer, int i, int j, TIER t);
void generatePawnUndoMoves(char *boardArray,  UNDOMOVELIST **moves, int currentPlayer, int i, int j, TIER t);
void generateKingUndoMoves(char *boardArray, UNDOMOVELIST **moves, int currentPlayer, int i, int j, TIER t);
void generateReplacementUndoMoves(char *boardArray, UNDOMOVELIST **moves, int currentPlayer, int i, int j, char replacementPiece, TIER t);
UNDOMOVELIST *gGenerateUndoMovesToTier (POSITION position, TIER t);
void PrintUndoMove (UNDOMOVE umove);
void printUndoMoveList(UNDOMOVELIST *moves);
BOOLEAN isNotSameTeam(char piece, int currentPlayer);
TIER gPositionToTier(POSITION p);
int* gPositionToPiecesArray(POSITION position, int *piecesArray);
void printPiecesArray(int *piecesArray);
TIERLIST* gTierChildren(TIER t);
int* gTierToPiecesArray(TIER t, int *piecesArray);
TIER gPiecesArrayToTier(int *piecesArray);
void printTierList(TIERLIST* tl);
TIERPOSITION gPositionToTierPosition(POSITION p, TIER t);
//POSITION gInitializeHashWindow(TIER t, POSITION p);
int countBits(int i);
void printTierArray(TIER tierArray[NUM_TIERS]);
BOOLEAN IsLegal(POSITION position);
BOOLEAN areKingsAdjacent(char* boardArray);
char *getBoard();
int* gBoardToPiecesArray(char *boardArray, int *piecesArray);
BOOLEAN isLegalBoard(char *Board);
TIER BoardToTier(char *Board);
TIERPOSITION NumberOfTierPositions(TIER t);
BOOLEAN IsLegal(POSITION p);
POSITION hash(char* board, int turn);
char* unhash(POSITION position, char *board);
STRING TierToString(TIER);
int getNumPieces(int* piecesArray);
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
//int gSmallFlipLRNewPosition[] = { 2, 1, 0, 5, 4, 3, 8, 7, 6, 11, 10, 9 };

/* This is the array used for flipping along the  E-W axis  */
//int gSmallFlipUDNewPosition[] = { 9, 10 ,11, 6, 7, 8, 3, 4, 5, 0, 1, 2 };

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
	int i, zeroPiece = 0, onePiece = 1, twoPiece = 9, threePiece = 37, fourPiece = 93, fivePiece = 163, sixPiece = 219, sevenPiece = 247, eightPiece = 255, numBits;
	int  *piecesArray, minBlank, maxBlank;
	gMoveToStringFunPtr = &MoveToString;
	gCanonicalPosition = GetCanonicalPosition;
	gUnDoMoveFunPtr = &gUnDoMove;
	gGenerateUndoMovesToTierFunPtr = &gGenerateUndoMovesToTier;
	gTierChildrenFunPtr = &gTierChildren;
	gIsLegalFunPtr                                        = &IsLegal;
	gNumberOfTierPositionsFunPtr  = &NumberOfTierPositions;
	gTierToStringFunPtr                           = &TierToString;
	kSupportsTierGamesman = TRUE;
	/*
	   3x4 Initial Game
	   +---+---+---+
	   4 | R | K | Q |
	   +---+---+---+
	   3 | B | N | P |
	   +---+---+---+
	   2 | b | n | p |
	   +---+---+---+
	   1 | r | k | q |
	   +---+---+---+
	    a   b   c
	 */
	/* 140 = 46*/
	piecesArray = (int *) SafeMalloc(DISTINCT_PIECES * sizeof(int));
	for(i = 0; i < NUM_TIERS; i++) {

		piecesArray =  gTierToPiecesArray((TIER) i, piecesArray);

		minBlank = rows*cols - 2 - *(piecesArray) - *(piecesArray+1) - *(piecesArray+2) - *(piecesArray+3) - *(piecesArray + 4) - *(piecesArray+5) - *(piecesArray+6) - *(piecesArray+7) - *(piecesArray + DISTINCT_PIECES-2) - *(piecesArray + DISTINCT_PIECES-1);
		maxBlank = minBlank;
		int pieces_array[40] = {'p', *(piecesArray + DISTINCT_PIECES-1), *(piecesArray + DISTINCT_PIECES-1), 'b', *(piecesArray + 5), *(piecesArray + 5), 'r', *(piecesArray + 6), *(piecesArray + 6), 'n', *(piecesArray + 7), *(piecesArray + 7), 'q', *(piecesArray + 4), *(piecesArray + 4), 'k', 1, 1, 'P', *(piecesArray + DISTINCT_PIECES-2), *(piecesArray + DISTINCT_PIECES-2), 'B', *(piecesArray + 1), *(piecesArray + 1), 'R', *(piecesArray + 2), *(piecesArray + 2), 'N', *(piecesArray + 3), *(piecesArray + 3), 'Q', *(piecesArray), *(piecesArray), 'K', 1, 1, ' ', minBlank, maxBlank, -1};

		generic_hash_init(BOARDSIZE, pieces_array, NULL, 0);
		if (i == 0) // since we can't discard contexts, I use this:
			Tier0Context = generic_hash_cur_context();
	}


	TIER tierlist[NUM_TIERS];
	// Replace "NUM_TIERS" with how many total tiers your game has
	for(i = 0; i < NUM_TIERS; i++) {
		numBits = countBits(i);
		switch(numBits) {
		case 0:
			tierlist[zeroPiece] = i;
			zeroPiece++;
			break;
		case 1:
			tierlist[onePiece] = i;
			onePiece++;
			break;
		case 2:
			tierlist[twoPiece] = i;
			twoPiece++;
			break;
		case 3:
			tierlist[threePiece] = i;
			threePiece++;
			break;
		case 4:
			tierlist[fourPiece] = i;
			fourPiece++;
			break;
		case 5:
			tierlist[fivePiece] = i;
			fivePiece++;
			break;
		case 6:
			tierlist[sixPiece] = i;
			sixPiece++;
			break;
		case 7:
			tierlist[sevenPiece] = i;
			sevenPiece++;
			break;
		case 8:
			tierlist[eightPiece] = i;
			eightPiece++;
			break;

		}
	}

	TIERLIST* gTierSolveListPtr = NULL;

	// initial tier.  Can be changed by gamespecific menu.
	gInitialTier = BoardToTier(theBoard);
	generic_hash_context_switch(gInitialTier);
	gInitialTierPosition = generic_hash_hash(theBoard, theCurrentPlayer);

	for (i = NUM_TIERS-1; i >= 0; i--) {
		gTierSolveListPtr = CreateTierlistNode((TIER) tierlist[i], gTierSolveListPtr);
	}

	piecesArray =  gBoardToPiecesArray(theBoard, piecesArray);

	minBlank = rows*cols - 2 - *(piecesArray) - *(piecesArray+1) - *(piecesArray+2) - *(piecesArray+3) - *(piecesArray + 4) - *(piecesArray+5) - *(piecesArray+6) - *(piecesArray+7) - *(piecesArray + DISTINCT_PIECES-2) - *(piecesArray + DISTINCT_PIECES-1);
	maxBlank = rows*cols - 2;
	int pieces_array[40] = {'p', 0, 1, 'b', 0, 1, 'r', 0, 1, 'n', 0, 1, 'q', 0, 1, 'k', 1, 1, 'P', 0, 1, 'B', 0, 1, 'R', 0, 1, 'N', 0, 1, 'Q', 0, 1, 'K', 1, 1, ' ', 6, maxBlank, -1};
	gNumberOfPositions = generic_hash_init(rows*cols, pieces_array, NULL, 0);
	gInitialPosition = hash(theBoard,theCurrentPlayer);
	SafeFree(piecesArray);
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
	unhash(position, boardArray);
	currentPlayer = generic_hash_turn(position);
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

	unhash(position, boardArray);
	int currentPlayer = generic_hash_turn(position);
	tempPiece = boardArray[(rows-rowi)*cols + (coli - 10)];
	boardArray[(rows - rowi)*cols + (coli - 10)] = ' ';
	boardArray[(rows - rowf)*cols + (colf - 10)] = tempPiece;
	if(tempPiece == WHITE_PAWN) {
		if(rowf == 1) {
			boardArray[(rows - rowf)*cols + (colf - 10)] = replacementPiece;
		}
	} else if(tempPiece == BLACK_PAWN) {
		if(rowf == rows) {
			boardArray[(rows - rowf)*cols + (colf - 10)] = replacementPiece;
		}
	}

	POSITION out = hash(boardArray, opposingPlayer(currentPlayer));


	return out;
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
	int currentPlayer = generic_hash_turn(position);
	char bA[rows*cols];
	unhash(position, bA);
	if (inCheck(bA, currentPlayer) && moves == NULL) {
		// The king is checked and no moves can be made
		return (gStandardGame) ? lose : win;
	}
	else if (!inCheck(bA, currentPlayer) && moves == NULL) {
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
	unhash(position, boardArray);

	printf("\n");
	for(x = 0; x < rows; x++) {
		printf("    +");
		for(y = 0; y < cols; y++) {
			printf("---+");
		}
		printf("\n");
		printf("  %d |", rows - x);
		for(y = 0; y < cols; y++) {
			printf(" %c |", boardArray[x*cols + y]);
		}
		printf("\n");
	}
	printf("    +");
	for(y = 0; y < cols; y++) {
		printf("---+");
	}
	printf("\n");
	printf("     ");
	for(y = 0; y < cols; y++) {
		printf(" %c  ", 97+y);
	}
	printf("\n");
	printf("%s\n",GetPrediction(position,playersName,usersTurn));
	printf("It is %s's turn (%s).\n",playersName,(generic_hash_turn(position) == WHITE_TURN) ? "white/uppercase" : "black/lowercase");
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
	STRING str = MoveToString(move);
	printf( "%s", str );
	SafeFree( str );
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

	char *board = NULL;
	char c;
	printf("\n");
	printf("Tile Chess Game Specific Menu\n\n");
	printf("1) Normal QuickChess Play\n");
	printf("2) Misere Variant\n");
	printf("3) Customize Initial Board\n");
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
	case '3':
		printf("Each player MUST have one\n\
king and all other pieces may not exceed the number allowed in\n\
traditional quickchess. Only 5x6 and 3x4 boards allowed.Insert dashes for blanks.\n\
Press enter to start on a new row, and press enter twice when you are\n\
finished describing your last row.  Here is a sample board:\n\
k--\n\
---\n\
---\n\
KB-\n\
The valid pieces are:\n\
K = king, P = pawn, Q = queen, B = bishop, R = rook, N = knight\n\
Upper-case letters indicate the pieces that belong to white, and\n\
lower-case letters indicate the pieces that belong to black:\n\n");
		do {
			if (board != NULL) {
				printf("Illegal board, re-enter:\n");
			}
			printf("Please Choose Board Size, 1=3x4, 2=5x6: ");
			switch(GetMyChar()) {
			case '1':
				break;
			case '2':
				rows = 6;
				cols = 5;
				break;
			default:
				printf("Wrong Entry! Default will be 3x4 board\n\n");
				break;
			}
			printf("Enter Board Now\n\n");
			theBoard = getBoard();
			do {
				printf("Whose turn is it? (w/b): \n");
				c = GetMyChar();
			} while (c != 'w' && c != 'b');
			theCurrentPlayer = (c == 'w') ? WHITE_TURN : BLACK_TURN;
			gInitialTier = BoardToTier(theBoard);
			generic_hash_context_switch(gInitialTier);
			gInitialTierPosition = generic_hash_hash(theBoard, theCurrentPlayer);
		} while(theBoard == NULL);
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
	printf("theboard:%s\n", theBoard);
	TIER t = BoardToTier(theBoard);
	printf("%s\n", TierToString(t));
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
	   unhash(gInitialPosition, bA);

	   for(i = 0; i < BOARDSIZE; i++) {
	   printf("seeing whats on the board on space %d: %c\n", i, bA[i]);
	   }

	   unhash(flipLR(gInitialPosition), bA);

	   for(i = 0; i < BOARDSIZE; i++) {
	   printf("seeing whats on the board on space %d: %c\n", i, bA[i]);
	   }
	 */
	/*
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
	 */
	//int i;
	//printf("Islegal returns %d for this board\nThe number of positions in tier 3 is %d", IsLegal(gInitialPosition), NumberOfTierPositions((TIER)4));
	//for(i = 0; i < 40; i++) {
	//printf("tier %d has %d positions\n", i, NumberOfTierPositions((TIER)i));
	//}
	/*
	   UNDOMOVE m;
	   m = createReplaceCaptureUndoMove(0, 0, 1, 1, 0);

	   PrintPosition(gInitialPosition, "me", TRUE);
	   PrintPosition(gTUndoMove(gInitialPosition, m), "me", TRUE);
	 */
	// PrintPosition(gInitialPosition, "me", TRUE);
	//printf("yes\n");
	//printUndoMoveList(gGenerateUndoMovesToTier(gInitialPosition, 2));

	/* TIER t;

	   //int *piecesArray = (int *) malloc(DISTINCT_PIECES * sizeof(int));
	   t = gPositionToTier(gInitialPosition);
	   printf("%d\n", t);
	   PrintPosition(gInitialPosition, "me", TRUE);
	   //printf("%s %s", TierToString(19),  TierToString(3));
	   //printf("the tier value is %d in decimal and %x in hex\n", t, t);
	   //printTierList(gTierChildren(t));
	   //printPiecesArray(gTierToPiecesArray(257, piecesArray));
	   //free(piecesArray);
	   printf("The KkBR tier moves\n");
	   printUndoMoveList(gGenerateUndoMovesToTier (gInitialPosition, 9));
	   printf("The KkR tier moves\n");
	   printUndoMoveList(gGenerateUndoMovesToTier (gInitialPosition, 137));
	   printf("The KkB tier moves\n");
	 */
	//printUndoMoveList(gGenerateUndoMovesToTier (gInitialPosition, 2));
	/* int i , zeroPiece = 0, onePiece = 1, twoPiece = 9, threePiece = 37, fourPiece = 93, fivePiece = 163, sixPiece = 219, sevenPiece = 247, eightPiece = 255, numBits;
	   i = countBits(223);;
	   printf("numBits in i = %d\n", i);
	   TIER tierlist[NUM_TIERS];
	   // Replace "NUM_TIERS" with how many total tiers your game has
	   for(i = 0; i < NUM_TIERS; i++) {
	   numBits = countBits(i);
	   switch(numBits) {
	   case 0:
	    tierlist[zeroPiece] = i;
	    zeroPiece++;
	    break;
	   case 1:
	    tierlist[onePiece] = i;
	    onePiece++;
	    break;
	   case 2:
	    tierlist[twoPiece] = i;
	    twoPiece++;
	    break;
	   case 3:
	    tierlist[threePiece] = i;
	    threePiece++;
	    break;
	   case 4:
	    tierlist[fourPiece] = i;
	    fourPiece++;
	    break;
	   case 5:
	    tierlist[fivePiece] = i;
	    fivePiece++;
	    break;
	   case 6:
	    tierlist[sixPiece] = i;
	    sixPiece++;
	    break;
	   case 7:
	    tierlist[sevenPiece] = i;
	    sevenPiece++;
	    break;
	   case 8:
	    tierlist[eightPiece] = i;
	    eightPiece++;
	    break;

	   }
	   }
	   printTierArray(tierlist);
	 */
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
**
** NAME:        inCheck
**
** DESCRIPTION: inCheck determines whether the given player is Checked on the
**              current Board.
**
** INPUTS:      char* bA   : The current Board.
**              int player : The current player.
**
** OUTPUTS:     BOOLEAN    : Whether or not the current player is in check.
**
************************************************************************/

BOOLEAN inCheck(char* bA, int checkedPlayer) {
	int i, j;
	char piece;

	//currentPlayer = generic_hash_turn(N);
	for (i = 0; i < rows; i++) {
		for(j = 0; j < cols; j++) {
			piece = bA[i*cols +j];
			switch (piece) {
			case WHITE_QUEEN: case BLACK_QUEEN:
				if (queenCheck(bA, i, j, checkedPlayer,piece, WHITE_QUEEN, BLACK_QUEEN) == TRUE) {
					return TRUE;
				} else {
					break;
				}
			case WHITE_BISHOP: case BLACK_BISHOP:
				if (bishopCheck(bA, i,j, checkedPlayer, piece, WHITE_BISHOP, BLACK_BISHOP) == TRUE) {
					return TRUE;
				} else {
					break;
				}
			case WHITE_ROOK: case BLACK_ROOK:
				if (rookCheck(bA, i, j, checkedPlayer, piece, WHITE_ROOK, BLACK_ROOK) == TRUE) {
					return TRUE;
				} else {
					break;
				}
			case WHITE_KNIGHT: case BLACK_KNIGHT:
				if (knightCheck(bA, i, j, checkedPlayer, piece, WHITE_KNIGHT, BLACK_KNIGHT) == TRUE) {
					return TRUE;
				} else {
					break;
				}
			case WHITE_PAWN: case BLACK_PAWN:
				if (pawnCheck(bA, i,j, checkedPlayer, piece, WHITE_PAWN, BLACK_PAWN) == TRUE) {
					return TRUE;
				} else {
					break;
				}

			case WHITE_KING: case BLACK_KING:
				if (kingCheck(bA, i,j, checkedPlayer, piece, WHITE_KING, BLACK_KING) == TRUE) {
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



BOOLEAN isKingCaptureable(char *Board, int row, int col, int currentPlayer, char currentPiece, char whitePiece, char blackPiece) {
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
BOOLEAN isDirectionCheck(char *Board, int i, int j, int currentPlayer, char currentPiece, char whitePiece, char blackPiece, int direction) {
	int i_inc=0, j_inc=0,  new_i=i, new_j=j;
	char piece;
	switch (direction) {
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
	while(new_i < rows && new_i >= 0 && new_j < cols && new_j >= 0) {
		piece = Board[new_i*cols+new_j];
		if (piece != ' ') {
			return isKingCaptureable(Board, new_i, new_j, currentPlayer, currentPiece, whitePiece, blackPiece);
		}else {
			new_i += i_inc;
			new_j += j_inc;
		}

	}
	return FALSE;
}

BOOLEAN queenCheck(char *Board, int row, int col, int currentPlayer, char currentPiece, char whitePiece, char blackPiece) {
	return isDirectionCheck(Board, row, col, currentPlayer, currentPiece, whitePiece, blackPiece, UP) ||
	       isDirectionCheck(Board, row, col, currentPlayer, currentPiece, whitePiece, blackPiece, DOWN) ||
	       isDirectionCheck(Board, row, col, currentPlayer, currentPiece, whitePiece, blackPiece, LEFT) ||
	       isDirectionCheck(Board, row, col, currentPlayer, currentPiece, whitePiece, blackPiece, RIGHT) ||
	       isDirectionCheck(Board, row, col, currentPlayer, currentPiece, whitePiece, blackPiece, UR) ||
	       isDirectionCheck(Board, row, col, currentPlayer, currentPiece, whitePiece, blackPiece, UL) ||
	       isDirectionCheck(Board, row, col, currentPlayer, currentPiece, whitePiece, blackPiece, DR) ||
	       isDirectionCheck(Board, row, col, currentPlayer, currentPiece, whitePiece, blackPiece, DL);
}

BOOLEAN bishopCheck(char *Board, int row, int col, int currentPlayer, char currentPiece, char whitePiece, char blackPiece) {
	return isDirectionCheck(Board, row, col, currentPlayer, currentPiece, whitePiece, blackPiece, UR) ||
	       isDirectionCheck(Board, row, col, currentPlayer, currentPiece, whitePiece, blackPiece, UL) ||
	       isDirectionCheck(Board, row, col, currentPlayer, currentPiece, whitePiece, blackPiece, DR) ||
	       isDirectionCheck(Board, row, col, currentPlayer, currentPiece, whitePiece, blackPiece, DL);
}

BOOLEAN rookCheck(char *Board, int row, int col, int currentPlayer, char currentPiece, char whitePiece, char blackPiece) {
	return isDirectionCheck(Board, row, col, currentPlayer, currentPiece, whitePiece, blackPiece, UP) ||
	       isDirectionCheck(Board, row, col, currentPlayer, currentPiece, whitePiece, blackPiece, DOWN) ||
	       isDirectionCheck(Board, row, col, currentPlayer, currentPiece, whitePiece, blackPiece, LEFT) ||
	       isDirectionCheck(Board, row, col, currentPlayer, currentPiece, whitePiece, blackPiece, RIGHT);
}

BOOLEAN knightCheck(char *Board, int row, int col, int currentPlayer, char currentPiece, char whitePiece, char blackPiece) {
	int rowTemp, colTemp;

	rowTemp = row;
	colTemp = col;

	// up two left one
	if(row-2 >= 0 && col-1 >= 0) {
		if(isKingCaptureable(Board, row-2, col-1, currentPlayer, currentPiece, whitePiece, blackPiece)) {
			return TRUE;
		}
	}

	// up two right one
	if(row-2 >= 0 && col+1 < cols) {
		if(isKingCaptureable(Board, row-2, col+1, currentPlayer, currentPiece, whitePiece, blackPiece)) {
			return TRUE;
		}
	}

	// right two up one
	if(col+2 < cols && row-1 >= 0) {
		if(isKingCaptureable(Board, row-1, col+2, currentPlayer, currentPiece, whitePiece, blackPiece)) {
			return TRUE;
		}
	}

	// right two down one
	if(col+2 < cols && row+1 < rows) {
		if(isKingCaptureable(Board, row+1, col+2, currentPlayer, currentPiece, whitePiece, blackPiece)) {
			return TRUE;
		}
	}

	// down two left one
	if(row+2 < rows && col-1 >= 0) {
		if(isKingCaptureable(Board, row+2, col-1, currentPlayer, currentPiece, whitePiece, blackPiece)) {
			return TRUE;
		}
	}

	// down two right one
	if(row+2 < rows && col+1 < cols) {
		if(isKingCaptureable(Board, row+2, col+1, currentPlayer, currentPiece, whitePiece, blackPiece)) {
			return TRUE;
		}
	}

	// left two up one
	if(col-2 >= 0 && row-1 >= 0) {
		if(isKingCaptureable(Board, row-1, col-2, currentPlayer, currentPiece, whitePiece, blackPiece)) {
			return TRUE;
		}
	}

	// left two down one
	if(col-2 >= 0 && row+1 < rows) {
		if(isKingCaptureable(Board, row+1, col-2, currentPlayer, currentPiece, whitePiece, blackPiece)) {
			return TRUE;
		}
	}
	return FALSE;
}


BOOLEAN pawnCheck(char *Board, int row, int col, int currentPlayer, char currentPiece, char whitePiece, char blackPiece) {
	if(currentPlayer == BLACK_TURN) {


		if(col-1 >= 0 && row-1 >= 0) {
			if(isKingCaptureable(Board, row-1, col-1, currentPlayer, currentPiece, whitePiece, blackPiece)) {
				return TRUE;
			}
		}
		if(col+1 < cols && row-1 >= 0) {
			if(isKingCaptureable(Board, row-1, col+1, currentPlayer, currentPiece, whitePiece, blackPiece)) {
				return TRUE;
			}
		}
	} else {
		if(col-1 >= 0 && row+1 < rows) {
			if(isKingCaptureable(Board, row+1, col-1, currentPlayer, currentPiece, whitePiece, blackPiece)) {
				return TRUE;
			}
		}
		if(col+1 < cols && row+1 < rows) {
			if(isKingCaptureable(Board, row+1, col+1, currentPlayer, currentPiece, whitePiece, blackPiece)) {
				return TRUE;
			}
		}
	}
	return FALSE;
}

BOOLEAN kingCheck(char *Board, int row, int col, int currentPlayer, char currentPiece, char whitePiece, char blackPiece) {
	// Left
	if(col > 0) {
		if(isKingCaptureable(Board, row, col-1, currentPlayer, currentPiece, whitePiece, blackPiece)) {
			return TRUE;
		}
	}
	// Right
	if(col < cols-1) {
		if(isKingCaptureable(Board, row, col+1, currentPlayer, currentPiece, whitePiece, blackPiece)) {
			return TRUE;
		}
	}
	// Up
	if(row > 0) {
		if(isKingCaptureable(Board, row-1, col, currentPlayer, currentPiece, whitePiece, blackPiece)) {
			return TRUE;
		}
	}
	// Down
	if(row < rows-1) {
		if(isKingCaptureable(Board, row+1, col, currentPlayer, currentPiece, whitePiece, blackPiece)) {
			return TRUE;
		}
	}
	// Up-left
	if(row > 0 && col > 0) {
		if(isKingCaptureable(Board, row-1, col-1, currentPlayer, currentPiece, whitePiece, blackPiece)) {
			return TRUE;
		}
	}
	// Up-right
	if(row > 0 && col < cols-1) {
		if(isKingCaptureable(Board, row-1, col+1, currentPlayer, currentPiece, whitePiece, blackPiece)) {
			return TRUE;
		}
	}
	// Down-left
	if(row < rows-1 && col > 0) {
		if(isKingCaptureable(Board, row+1, col-1, currentPlayer, currentPiece, whitePiece, blackPiece)) {
			return TRUE;
		}
	}
	// Down-right
	if(row < rows-1 && col < cols-1) {
		if(isKingCaptureable(Board, row+1, col+1, currentPlayer, currentPiece, whitePiece, blackPiece)) {
			return TRUE;
		}
	}
	return FALSE;
}

/************************************************************************
**
** NAMES:       generate"Piece"Moves
**
** DESCRIPTION: Generates all the legal moves and puts them into the movelist
**              for the given board and player
**
** INPUTS:      char* bA   : The current Board.
**              int player : The current player.
**              int i, j   : coordinates of spot on board being examined
**
************************************************************************/

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
	if (i < rows-1 && j > 1 && !isSameTeam(boardArray[(i+1)*cols + j-2], currentPlayer)) {
		newMove = createMove(i, j, i+1, j-2 );
		if (testMove(boardArray, i,i+1,j,j-2, currentPlayer)) {
			*moves = CreateMovelistNode(newMove, *moves);
		}
	}
	//Right 2, Down 1
	if (i < rows-1 && j < cols-2 && !isSameTeam(boardArray[(i+1)*cols + j+2], currentPlayer)) {
		newMove = createMove(i, j, i+1, j+2 );
		if (testMove(boardArray, i,i+1,j,j+2, currentPlayer)) {
			*moves = CreateMovelistNode(newMove, *moves);
		}
	}
	//Down 2, Left 1
	if (i < rows-2 && j > 0 && !isSameTeam(boardArray[(i+2)*cols + j-1], currentPlayer)) {
		newMove = createMove(i, j, i+2, j-1);
		if (testMove(boardArray, i,i+2,j,j-1, currentPlayer)) {
			*moves = CreateMovelistNode(newMove, *moves);
		}
	}
	//Down 2, Right 1
	if (i < rows-2 && j < cols-1 && !isSameTeam(boardArray[(i+2)*cols + j+1], currentPlayer)) {
		newMove = createMove(i, j, i+2, j+1);
		if (testMove(boardArray, i,i+2,j,j+1, currentPlayer)) {
			*moves = CreateMovelistNode(newMove, *moves);
		}
	}
	//Left 2, Up 1
	if (i > 0 && j > 1 && !isSameTeam(boardArray[(i-1)*cols + j-2], currentPlayer)) {
		newMove = createMove(i, j, i-1, j-2 );
		if (testMove(boardArray, i,i-1,j,j-2, currentPlayer)) {
			*moves = CreateMovelistNode(newMove, *moves);
		}
	}
	//Right 2, Up 1
	if (i > 0 && j < cols-2 && !isSameTeam(boardArray[(i-1)*cols + j+2], currentPlayer)) {
		newMove = createMove(i, j, i-1, j+2 );
		if (testMove(boardArray, i,i-1,j,j+2, currentPlayer)) {
			*moves = CreateMovelistNode(newMove, *moves);
		}
	}
	//Up 2, Left 1
	if (i > 1 && j > 0 && !isSameTeam(boardArray[(i-2)*cols + j-1], currentPlayer)) {
		newMove = createMove(i, j, i-2, j-1 );
		if (testMove(boardArray, i,i-2,j,j-1, currentPlayer)) {
			*moves = CreateMovelistNode(newMove, *moves);
		}
	}
	//Up 2, Right 1
	if (i > 1 && j < cols-1 && !isSameTeam(boardArray[(i-2)*cols + j+1], currentPlayer)) {
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

	if (currentPlayer == BLACK_TURN) {
		//down 1.  Only a legal move if there is no piece there
		if (i != rows-1 && boardArray[(i+1)*cols + j] == ' ') {
			MOVE newMove = createMove(i, j, i+1, j);
			if (testMove(boardArray, i,i+1,j,j, currentPlayer)) {
				*moves = CreateMovelistNode(newMove, *moves);
				// check if pawn is at the end
				if(i + 1 == rows-1) {
					if(replacement(boardArray, BLACK_QUEEN)) {
						newMove = createPawnMove(i, j, i+1, j, BLACK_QUEEN);
						*moves = CreateMovelistNode(newMove, *moves);
					}
					if(replacement(boardArray, BLACK_ROOK)) {
						newMove = createPawnMove(i, j, i+1, j, BLACK_ROOK);
						*moves = CreateMovelistNode(newMove, *moves);
					}
					if(replacement(boardArray, BLACK_BISHOP)) {
						newMove = createPawnMove(i, j, i+1, j, BLACK_BISHOP);
						*moves = CreateMovelistNode(newMove, *moves);
					}
					if(replacement(boardArray, BLACK_KNIGHT)) {
						newMove = createPawnMove(i, j, i+1, j, BLACK_KNIGHT);
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
					if(replacement(boardArray, BLACK_QUEEN)) {
						newMove = createPawnMove(i, j, i+1, j, BLACK_QUEEN);
						*moves = CreateMovelistNode(newMove, *moves);
					}
					if(replacement(boardArray, BLACK_ROOK)) {
						newMove = createPawnMove(i, j, i+1, j, BLACK_ROOK);
						*moves = CreateMovelistNode(newMove, *moves);
					}
					if(replacement(boardArray, BLACK_BISHOP)) {
						newMove = createPawnMove(i, j, i+1, j, BLACK_BISHOP);
						*moves = CreateMovelistNode(newMove, *moves);
					}
					if(replacement(boardArray, BLACK_KNIGHT)) {
						newMove = createPawnMove(i, j, i+1, j, BLACK_KNIGHT);
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
					if(replacement(boardArray, BLACK_QUEEN)) {
						newMove = createPawnMove(i, j, i+1, j, BLACK_QUEEN);
						*moves = CreateMovelistNode(newMove, *moves);
					}
					if(replacement(boardArray, BLACK_ROOK)) {
						newMove = createPawnMove(i, j, i+1, j, BLACK_ROOK);
						*moves = CreateMovelistNode(newMove, *moves);
					}
					if(replacement(boardArray, BLACK_BISHOP)) {
						newMove = createPawnMove(i, j, i+1, j, BLACK_BISHOP);
						*moves = CreateMovelistNode(newMove, *moves);
					}
					if(replacement(boardArray, BLACK_KNIGHT)) {
						newMove = createPawnMove(i, j, i+1, j, BLACK_KNIGHT);
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
					if(replacement(boardArray, WHITE_QUEEN)) {
						newMove = createPawnMove(i, j, i+1, j, WHITE_QUEEN);
						*moves = CreateMovelistNode(newMove, *moves);
					}
					if(replacement(boardArray, WHITE_ROOK)) {
						newMove = createPawnMove(i, j, i+1, j, WHITE_ROOK);
						*moves = CreateMovelistNode(newMove, *moves);
					}
					if(replacement(boardArray, WHITE_BISHOP)) {
						newMove = createPawnMove(i, j, i+1, j, WHITE_BISHOP);
						*moves = CreateMovelistNode(newMove, *moves);
					}
					if(replacement(boardArray, WHITE_KNIGHT)) {
						newMove = createPawnMove(i, j, i+1, j, WHITE_KNIGHT);
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
					if(replacement(boardArray, WHITE_QUEEN)) {
						newMove = createPawnMove(i, j, i+1, j, WHITE_QUEEN);
						*moves = CreateMovelistNode(newMove, *moves);
					}
					if(replacement(boardArray, WHITE_ROOK)) {
						newMove = createPawnMove(i, j, i+1, j, WHITE_ROOK);
						*moves = CreateMovelistNode(newMove, *moves);
					}
					if(replacement(boardArray, WHITE_BISHOP)) {
						newMove = createPawnMove(i, j, i+1, j, WHITE_BISHOP);
						*moves = CreateMovelistNode(newMove, *moves);
					}
					if(replacement(boardArray, WHITE_KNIGHT)) {
						newMove = createPawnMove(i, j, i+1, j, WHITE_KNIGHT);
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
					if(replacement(boardArray, WHITE_QUEEN)) {
						newMove = createPawnMove(i, j, i+1, j, WHITE_QUEEN);
						*moves = CreateMovelistNode(newMove, *moves);
					}
					if(replacement(boardArray, WHITE_ROOK)) {
						newMove = createPawnMove(i, j, i+1, j, WHITE_ROOK);
						*moves = CreateMovelistNode(newMove, *moves);
					}
					if(replacement(boardArray, WHITE_BISHOP)) {
						newMove = createPawnMove(i, j, i+1, j, WHITE_BISHOP);
						*moves = CreateMovelistNode(newMove, *moves);
					}
					if(replacement(boardArray, WHITE_KNIGHT)) {
						newMove = createPawnMove(i, j, i+1, j, WHITE_KNIGHT);
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
	switch (direction) {
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
	while(new_i < rows && new_i >= 0 && new_j < cols && new_j >= 0) {
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
	boardInCheck = inCheck(boardArray, currentPlayer);
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

/* Returns TRUE if the given piece belongs to opposite of the currentPlayer,
   and FALSE otherwise. */
BOOLEAN isNotSameTeam(char piece, int currentPlayer) {
	if (piece >= 'a' && piece <= 'z' && currentPlayer == WHITE_TURN)
		return TRUE;
	else if (piece >= 'A' && piece <= 'Z' && currentPlayer == BLACK_TURN)
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
	for(x = 0; x < rows; x++) {
		printf("    +");
		for(y = 0; y < cols; y++) {
			printf("---+");
		}
		printf("\n");
		printf("  %d |", x);
		for(y = 0; y < cols; y++) {
			printf(" %c |", boardArray[x*cols + y]);
		}
		printf("\n");
	}
	printf("    +");
	for(y = 0; y < cols; y++) {
		printf("---+");
	}
	printf("\n");
	printf("     ");
	for(y = 0; y < cols; y++) {
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
		printf("\n");
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

	for(i = 1; i < NUMSYMMETRIES; i++) {
		if(!((isPawn(position) || isBishop(position)) && i > 1)) {
			newPosition = DoSymmetry(position, i); /* get new */
			if(newPosition < theCanonicalPosition) /* THIS is the one */
				theCanonicalPosition = newPosition; /* set it to the ans */
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
	char currentBoardArray[rows*cols], flippedBoardArray[rows*cols];
	int currentPlayer = generic_hash_turn(position);

	unhash(position, currentBoardArray);
	for(i = 0; i < rows; i++) {
		for(j = 0; j < cols; j++) {
			flippedBoardArray[(i * cols) + j] = currentBoardArray[(i * cols) + (cols - 1 - j)];
		}
	}

	return hash(flippedBoardArray, currentPlayer);
}
POSITION flipUD(position)
POSITION position;
{
	int i, j;
	char currentBoardArray[rows*cols], flippedBoardArray[rows*cols];
	int currentPlayer = generic_hash_turn(position);

	unhash(position, currentBoardArray);
	for(i = 0; i < rows; i++) {
		for(j = 0; j < cols; j++) {
			flippedBoardArray[(i * cols) + j] = currentBoardArray[((rows-1-i) * cols) + j];
		}
	}
	return hash(flippedBoardArray, currentPlayer);
}

BOOLEAN isPawn(position)
POSITION position;
{
	int i, j;
	char piece, boardArray[rows*cols];
	unhash(position, boardArray);
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
	unhash(position, boardArray);
	for(i = 0; i < rows; i++) {
		for(j = 0; j < cols; j++) {
			piece = boardArray[(i * cols) + j];
			if(piece == WHITE_BISHOP || piece == BLACK_BISHOP)
				return TRUE;
		}
	}
	return FALSE;
}

/************************************************************************
**
** TIER GAMESMAN API
**
************************************************************************/





/************************************************************************
**
** NAME:        NumberOfTierPositions
**
** DESCRIPTION: Given a tier, determine the maximum number of positions
**              that are exclusive to that tier.
**
** INPUTS:      TIER tier : the given tier
**
** OUTPUTS:     TIERPOSITION : The max number position in the set of positions
**                             of the given tier
**
************************************************************************/

TIERPOSITION NumberOfTierPositions(TIER tier) {
	generic_hash_context_switch(Tier0Context+tier);
	return generic_hash_max_pos();
}


/************************************************************************
**
** NAME:        gPositionToTier
**
** DESCRIPTION: Takes a given position and determines which tier it belongs to.
**
** INPUTS:      POSITION position : The given position
**
** OUTPUTS:     TIER              : The tier the position translates to.
**
************************************************************************/

TIER gPositionToTier(POSITION position) {
	int *piecesArray;
	TIER tier;
	piecesArray = (int *) SafeMalloc(DISTINCT_PIECES * sizeof(int));
	piecesArray =  gPositionToPiecesArray(position, piecesArray);
	//printPiecesArray(piecesArray);
	tier = gPiecesArrayToTier(piecesArray);
	SafeFree(piecesArray);
	return tier;

}

/************************************************************************
**
** NAME:        gTierChildren
**
** DESCRIPTION: Given a tier, this function determines all the tiers that can be
**              reached from this position by a move made, including the self tier and
**              returns all these tier in a tierlist
**
** INPUTS:      TIER t    : The given tier.
**
** OUTPUTS:     TIERLIST  : The children of the given tier
**
************************************************************************/


TIERLIST* gTierChildren(TIER t){
	TIERLIST* tiers = NULL;
	int *piecesArray;
	int i;
	TIER tempTier;
	piecesArray = (int *) SafeMalloc(DISTINCT_PIECES * sizeof(int));
	piecesArray =  gTierToPiecesArray(t, piecesArray);
	tiers = CreateTierlistNode(t, tiers);
	/* creating capture children tiers (NON-PAWNS) */
	for(i = 0; i < DISTINCT_PIECES - 2; i++) {
		if(*(piecesArray + i) == 1) {
			*(piecesArray + i) = 0;
			tempTier = gPiecesArrayToTier(piecesArray);
			tiers = CreateTierlistNode(tempTier, tiers);
			//printTierList(tiers);
			*(piecesArray + i) = 1;
		}
	}

	/* handling white pawns */

	/* are there any white pawns */
	if(*(piecesArray + DISTINCT_PIECES-2) > 0) {
		for(i = 0; i < 4; i++) {
			if(*(piecesArray + i) == 0) {
				*(piecesArray + i) = 1;
				*(piecesArray + DISTINCT_PIECES-2) -= 1;
				tempTier = gPiecesArrayToTier(piecesArray);
				tiers = CreateTierlistNode(tempTier, tiers);
				*(piecesArray + i) = 0;
				*(piecesArray + DISTINCT_PIECES-2) += 1;
			}
		}
		*(piecesArray + DISTINCT_PIECES-2) -= 1;
		tempTier = gPiecesArrayToTier(piecesArray);
		tiers = CreateTierlistNode(tempTier, tiers);
		*(piecesArray + DISTINCT_PIECES-2) += 1;
	}

	/* handling black pawns */
	if(*(piecesArray + DISTINCT_PIECES-1) > 0) {
		for(i = 4; i < DISTINCT_PIECES - 2; i++) {
			if(*(piecesArray + i) == 0) {
				*(piecesArray + i) = 1;
				*(piecesArray + DISTINCT_PIECES-1) -= 1;
				tempTier = gPiecesArrayToTier(piecesArray);
				tiers = CreateTierlistNode(tempTier, tiers);
				*(piecesArray + i) = 0;
				*(piecesArray + DISTINCT_PIECES-1) += 1;
			}
		}
		*(piecesArray + DISTINCT_PIECES-1) -= 1;
		tempTier = gPiecesArrayToTier(piecesArray);
		tiers = CreateTierlistNode(tempTier, tiers);
		*(piecesArray + DISTINCT_PIECES-1) += 1;
	}
	SafeFree(piecesArray);
	return tiers;
}

/************************************************************************
**
** NAME:        gUnDoMove
**
** DESCRIPTION: Given a position and an undomove, it does the undomove on the
**              position and returns the resulting position
**
**
** INPUTS:      UNDOMOVE umove    : The given undomove.
**              POSITION position : the given position
**
** OUTPUTS:     POSITION          : The resulting position from the undomove
**
************************************************************************/

POSITION gUnDoMove(POSITION position, UNDOMOVE umove) {
	char boardArray[rows*cols];
	char tempPiece, capturedPiece;
	int rowi, coli, rowf, colf, isReplacementPiece;
	rowf = umove & 15;
	colf = (umove >> 4) & 15;
	rowi = (umove >> 8) & 15;
	coli = (umove >> 12) & 15;
	capturedPiece = (char) (umove >> 16) & 255;
	isReplacementPiece = (char) (umove >> 24) & 1;

	unhash(position, boardArray);
	int currentPlayer = generic_hash_turn(position);
	tempPiece = boardArray[(rows-rowi)*cols + (coli - 10)];
	if(capturedPiece == 0 || capturedPiece == ' ')
		boardArray[(rows - rowi)*cols + (coli - 10)] = ' ';
	else boardArray[(rows - rowi)*cols + (coli - 10)] = capturedPiece;

	if(isReplacementPiece == 1) {
		if(currentPlayer == WHITE_TURN)
			boardArray[(rows - rowf)*cols + (colf - 10)] = BLACK_PAWN;
		else boardArray[(rows - rowf)*cols + (colf - 10)] = WHITE_PAWN;
	} else boardArray[(rows - rowf)*cols + (colf - 10)] = tempPiece;
	if(BoardToTier(boardArray) == 256)
		printArray(boardArray);
	return hash(boardArray, opposingPlayer(currentPlayer));

}



/************************************************************************
**
** NAME:        gGenerateUndoMovesToTier
**
** DESCRIPTION: Generates all the undomoves of the current position that lead into
**              the given tier and puts them into a list.
**
**
** INPUTS:      TIER t            : The given tier.
**              POSITION position : the given position
**
** OUTPUTS:     UNDOMOVELIST  : The list of undomoves that lead from the given position
**                              into the given tier.
**
************************************************************************/


UNDOMOVELIST *gGenerateUndoMovesToTier (POSITION position, TIER t)
{
	UNDOMOVELIST *moves = NULL;
	int currentPlayer, i,j;
	char piece;
	char boardArray[rows*cols];
	unhash(position, boardArray);
	currentPlayer = generic_hash_turn(position);
	for (i = 0; i < rows; i++) {
		for(j = 0; j < cols; j++) {

			piece = boardArray[i*cols + j];
			// check if piece belongs to the currentPlayer
			if (isNotSameTeam(piece, currentPlayer)) {
				//printf("move: i:%d, j:%d",i,j);
				switch (piece) {
				case WHITE_QUEEN: case BLACK_QUEEN:

					if (i == 0 && piece == WHITE_QUEEN) {
						generateReplacementUndoMoves(boardArray, &moves, currentPlayer, i, j, WHITE_QUEEN, t);
					} else if(i == rows-1 && piece == BLACK_QUEEN) {
						generateReplacementUndoMoves(boardArray, &moves, currentPlayer, i, j, BLACK_QUEEN, t);
					}

					generateQueenUndoMoves(boardArray, &moves, currentPlayer, i, j, t);
					break;
				case WHITE_BISHOP: case BLACK_BISHOP:

					if (i == 0 && piece == WHITE_QUEEN) {
						generateReplacementUndoMoves(boardArray, &moves, currentPlayer, i, j, WHITE_QUEEN, t);
					} else if(i == rows-1 && piece == BLACK_QUEEN) {
						generateReplacementUndoMoves(boardArray, &moves, currentPlayer, i, j, BLACK_QUEEN, t);
					}

					generateBishopUndoMoves(boardArray, &moves, currentPlayer, i, j, t);
					break;
				case WHITE_ROOK: case BLACK_ROOK:

					if (i == 0 && piece == WHITE_QUEEN) {
						generateReplacementUndoMoves(boardArray, &moves, currentPlayer, i, j, WHITE_QUEEN, t);
					} else if(i == rows-1 && piece == BLACK_QUEEN) {
						generateReplacementUndoMoves(boardArray, &moves, currentPlayer, i, j, BLACK_QUEEN, t);
					}

					generateRookUndoMoves(boardArray, &moves, currentPlayer, i, j, t);
					break;
				case WHITE_KNIGHT: case BLACK_KNIGHT:
					if (i == 0 && piece == WHITE_QUEEN) {
						generateReplacementUndoMoves(boardArray, &moves, currentPlayer, i, j, WHITE_QUEEN, t);
					} else if(i == rows-1 && piece == BLACK_QUEEN) {
						generateReplacementUndoMoves(boardArray, &moves, currentPlayer, i, j, BLACK_QUEEN, t);
					}
					generateKnightUndoMoves(boardArray, &moves, currentPlayer, i, j, t);
					break;
				case WHITE_PAWN: case BLACK_PAWN:
					generatePawnUndoMoves(boardArray, &moves, currentPlayer, i, j, t);
					break;
				case WHITE_KING: case BLACK_KING:
					generateKingUndoMoves(boardArray, &moves, currentPlayer, i, j, t);
					break;
				default:
					break;
				}
			}

		}
	}

	// Use CreateUndoMovelistNode(move, next) to 'cons' together a linked list

	return moves;
}

/************************************************************************
**
** NAME:        IsLegal
**
** DESCRIPTION: determines if the given position is reachable through gameplay
**
** INPUTS:      POSITION position : The given position.
**
** OUTPUTS:     BOOLEAN           : Whether or not the position is reachable.
**
************************************************************************/

BOOLEAN IsLegal(POSITION position) {
	char boardArray[rows*cols];
	unhash(position, boardArray);
	if(areKingsAdjacent(boardArray))
		return FALSE;
	else return TRUE;


}

/************************************************************************
**
** IsLegal Helper Functions
**
************************************************************************/

BOOLEAN areKingsAdjacent(char* boardArray) {
	char piece;
	int i, j;

	for(i = 0; i < rows; i++) {
		for(j = 0; j < cols; j++) {
			piece = boardArray[(i * cols) + j];
			if(piece == WHITE_KING) {
				// check down a row
				if(i+1 < rows) {
					if(boardArray[((i+1) * cols) + j] == BLACK_KING)
						return TRUE;
					if(j+1 < cols) {
						if(boardArray[((i+1) * cols) + j+1] == BLACK_KING)
							return TRUE;
					}
					if(j-1 >= 0) {
						if(boardArray[((i+1) * cols) + j-1] == BLACK_KING)
							return TRUE;
					}
				}
				// check up a row
				if(i-1 >= 0) {
					if(boardArray[((i-1) * cols) + j] == BLACK_KING)
						return TRUE;
					if(j+1 < cols) {
						if(boardArray[((i-1) * cols) + j+1] == BLACK_KING)
							return TRUE;
					}
					if(j-1 >= 0) {
						if(boardArray[((i-1) * cols) + j-1] == BLACK_KING)
							return TRUE;
					}
				}
				// check right
				if(j+1 < cols) {
					if(boardArray[(i * cols) + j+1] == BLACK_KING)
						return TRUE;
				}
				// check left
				if(j-1 >= 0) {
					if(boardArray[(i * cols) + j-1] == BLACK_KING)
						return TRUE;
				}
			}
		}
	}
	return FALSE;
}

/************************************************************************
**
** TIER GAMESMAN API Helper Functions
**
************************************************************************/

void printPiecesArray(int *piecesArray) {
	int i;
	printf("[ ");
	for(i = 0; i < DISTINCT_PIECES; i++) {
		printf("%d ", *(piecesArray + i));
	}
	printf("]\n");
}

/*
 * Takes a character board and returns an integer array with the number
 * of each type of piece.
 */
int* gBoardToPiecesArray(char *boardArray, int *piecesArray) {
	int i, j;

	char piece;
	for(i = 0; i < DISTINCT_PIECES; i++) {
		*(piecesArray + i) = 0;
	}

	for(i = 0; i < rows; i++) {
		for(j = 0; j < cols; j++) {
			piece = boardArray[(i * cols) + j];
			if(piece != ' ') {
				switch (piece) {
				case WHITE_QUEEN:
					*(piecesArray) += 1;
					break;
				case BLACK_QUEEN:
					*(piecesArray + 4) += 1;
					break;
				case WHITE_BISHOP:
					*(piecesArray + 1) += 1;
					break;
				case BLACK_BISHOP:
					*(piecesArray + 5) += 1;
					break;
				case WHITE_ROOK:
					*(piecesArray + 2) += 1;
					break;
				case BLACK_ROOK:
					*(piecesArray + 6) += 1;
					break;
				case WHITE_KNIGHT:
					*(piecesArray + 3) += 1;
					break;
				case BLACK_KNIGHT:
					*(piecesArray + 7) += 1;
					break;
				case WHITE_PAWN:
					*(piecesArray + 8) += 1;
					break;
				case BLACK_PAWN:
					*(piecesArray + 9) += 1;
					break;
				default:
					break;
				}
			}


		}
	}
	return piecesArray;
}

int* gPositionToPiecesArray(POSITION position, int *piecesArray){
	int i, j;

	char piece, boardArray[rows*cols];
	for(i = 0; i < DISTINCT_PIECES; i++) {
		*(piecesArray + i) = 0;
	}
	unhash(position, boardArray);
	for(i = 0; i < rows; i++) {
		for(j = 0; j < cols; j++) {
			piece = boardArray[(i * cols) + j];
			if(piece != ' ') {
				switch (piece) {
				case WHITE_QUEEN:
					*(piecesArray) += 1;
					break;
				case BLACK_QUEEN:
					*(piecesArray + 4) += 1;
					break;
				case WHITE_BISHOP:
					*(piecesArray + 1) += 1;
					break;
				case BLACK_BISHOP:
					*(piecesArray + 5) += 1;
					break;
				case WHITE_ROOK:
					*(piecesArray + 2) += 1;
					break;
				case BLACK_ROOK:
					*(piecesArray + 6) += 1;
					break;
				case WHITE_KNIGHT:
					*(piecesArray + 3) += 1;
					break;
				case BLACK_KNIGHT:
					*(piecesArray + 7) += 1;
					break;
				case WHITE_PAWN:
					*(piecesArray + 8) += 1;
					break;
				case BLACK_PAWN:
					*(piecesArray + 9) += 1;
					break;
				default:
					break;
				}
			}


		}
	}
	return piecesArray;
}

TIER BoardToTier(char *Board) {
	TIER t;
	int *piecesArray = (int *) SafeMalloc(DISTINCT_PIECES * sizeof(int));
	piecesArray =  gBoardToPiecesArray(Board, piecesArray);
	t = gPiecesArrayToTier(piecesArray);
	SafeFree(piecesArray);
	return t;
}



void printTierList(TIERLIST* tl) {
	printf("[ ");
	while(tl != NULL) {
		printf("%llu ", tl->tier);
		tl = tl->next;
	}
	printf("]");
}

void printTierArray(TIER tierArray[NUM_TIERS]) {
	int i;
	printf("[ ");
	for(i = 0; i < NUM_TIERS; i++) {
		printf("%llu ", tierArray[i]);
	}
	printf("]\n");
}

int* gTierToPiecesArray(TIER t, int *piecesArray) {
	int i;
	for(i = 0; i < DISTINCT_PIECES; i++) {
		*(piecesArray + i) = 0;
	}
	for(i = 0; i < DISTINCT_PIECES - 2; i++) {
		if(((t >> i) & 1) == 1) {
			*(piecesArray + i) = 1;

		}
	}
	*(piecesArray + DISTINCT_PIECES-2) = (t >> (DISTINCT_PIECES-2)) & 15;
	*(piecesArray + DISTINCT_PIECES-1) = (t >> (DISTINCT_PIECES+2)) & 15;

	return piecesArray;
}



TIER gPiecesArrayToTier(int *piecesArray) {
	TIER tier = 0;
	int i;
	for(i = 0; i < DISTINCT_PIECES - 2; i++) {
		tier = tier | ((*(piecesArray+i)) << i);
	}
	tier = tier | ((*(piecesArray+DISTINCT_PIECES-2)) << (DISTINCT_PIECES-2));
	tier = tier | ((*(piecesArray+DISTINCT_PIECES-1)) << (DISTINCT_PIECES+2));
	return tier;
}



STRING MoveToString(move)
MOVE move;
{
	char rowf, colf, rowi, coli, replacementPiece;
	STRING moveStr;
	rowf = (move & 15) + 48;
	colf = ((move >> 4) & 15) - 10 + 97;
	rowi = ((move >> 8) & 15) + 48;
	coli = ((move >> 12) & 15) - 10 + 97;
	replacementPiece =  move >> 16;
	if (replacementPiece == 0) {
		moveStr = (STRING) SafeMalloc(sizeof(char)*4 + 1);
		sprintf(moveStr, "%c%c%c%c", coli, rowi, colf, rowf);
	} else {
		moveStr = (STRING) SafeMalloc(sizeof(char)*6 + 1);
		sprintf(moveStr, "%c%c%c%c=%c", coli, rowi, colf, rowf, replacementPiece);
	}
	return moveStr;
}



void generateReplacementUndoMoves(char *boardArray, UNDOMOVELIST **moves, int currentPlayer, int i, int j, char replacementPiece, TIER t){
	UNDOMOVE newuMove;
	if(currentPlayer == WHITE_TURN) {
		// UP
		if(boardArray[(i-1)*cols + j] == ' ') {
			if(testReplaceCaptureUndoMove(boardArray, i, i-1, j, j, currentPlayer, 0, t)) {
				newuMove = createReplaceCaptureUndoMove(i, j, i-1, j, 0);
				*moves = CreateUndoMovelistNode(newuMove, *moves);
			}
		}
		// UP-LEFT
		if(j > 0) {
			if(boardArray[(i-1)*cols + j-1] == ' ') {
				generateCaptureUndoMoves(boardArray, moves, currentPlayer, i, j, i-1, j-1, 1, t);
			}
		}
		// UP-RIGHT
		if( j < cols-1) {
			if(boardArray[(i-1)*cols + j+1] == ' ') {
				generateCaptureUndoMoves(boardArray, moves, currentPlayer, i, j, i-1, j+1, 1, t);
			}
		}

	} else {
		// DOWN
		if(boardArray[(i+1)*cols + j] == ' ') {
			if(testReplaceCaptureUndoMove(boardArray, i, i+1, j, j, currentPlayer, 0, t)) {
				newuMove = createReplaceCaptureUndoMove(i, j, i+1, j, 0);
				*moves = CreateUndoMovelistNode(newuMove, *moves);
			}
		}
		// DOWN-LEFT
		if(j > 0) {
			if(boardArray[(i+1)*cols + j-1] == ' ') {
				generateCaptureUndoMoves(boardArray, moves, currentPlayer, i, j, i+1, j-1, 1, t);
			}
		}
		// DOWN-RIGHT
		if( j < cols-1) {
			if(boardArray[(i+1)*cols + j+1] == ' ') {
				generateCaptureUndoMoves(boardArray, moves, currentPlayer, i, j, i+1, j+1, 1, t);
			}
		}

	}

}




void generateKingUndoMoves(char *boardArray, UNDOMOVELIST **moves, int currentPlayer, int i, int j, TIER t){
	UNDOMOVE newuMove;
	//UP
	if (i > 0 && boardArray[(i-1)*cols + j] == ' ') {
		newuMove = createUndoMove(i, j, i-1, j);
		if (testUndoMove(boardArray, i, i-1, j, j, currentPlayer, t)) {
			*moves = CreateUndoMovelistNode(newuMove, *moves);
		}
		generateCaptureUndoMoves(boardArray, moves, currentPlayer, i, j, i-1, j, 0, t);
	}
	//Down
	if (i < rows-1 && boardArray[(i+1)*cols + j] == ' ') {
		newuMove = createUndoMove(i, j, i+1, j);
		if (testUndoMove(boardArray, i,i+1,j,j, currentPlayer, t)) {
			*moves = CreateUndoMovelistNode(newuMove, *moves);
		}
		generateCaptureUndoMoves(boardArray, moves, currentPlayer, i, j, i+1, j, 0, t);
	}
	//Left
	if (j > 0 && boardArray[i*cols + j-1] == ' ') {
		newuMove = createUndoMove(i, j, i, j-1 );
		if (testUndoMove(boardArray, i,i,j,j-1, currentPlayer, t)) {
			*moves = CreateUndoMovelistNode(newuMove, *moves);
		}
		generateCaptureUndoMoves(boardArray, moves, currentPlayer, i, j, i, j-1, 0, t);
	}
	//Right
	if (j < cols-1 && boardArray[i*cols + j+1] == ' ') {
		newuMove = createUndoMove(i, j, i, j+1 );
		if (testUndoMove(boardArray, i,i,j,j+1, currentPlayer, t)) {
			*moves = CreateUndoMovelistNode(newuMove, *moves);
		}
		generateCaptureUndoMoves(boardArray, moves, currentPlayer, i, j, i, j+1, 0, t);
	}
	//Up-left
	if (i > 0 && j > 0 && boardArray[(i-1)*cols + j-1] == ' ') {
		newuMove = createUndoMove(i, j, i-1, j-1 );
		if (testUndoMove(boardArray, i,i-1,j,j-1, currentPlayer, t)) {
			*moves = CreateUndoMovelistNode(newuMove, *moves);
		}
		generateCaptureUndoMoves(boardArray, moves, currentPlayer, i, j, i-1, j-1, 0, t);
	}
	//Up-right
	if (i > 0 && j < cols-1 && boardArray[(i-1)*cols + j+1] == ' ') {
		newuMove = createUndoMove(i, j, i-1, j+1 );
		if (testUndoMove(boardArray, i,i-1,j,j+1, currentPlayer, t)) {
			*moves = CreateUndoMovelistNode(newuMove, *moves);
		}
		generateCaptureUndoMoves(boardArray, moves, currentPlayer, i, j, i-1, j+1, 0, t);
	}
	//Down-left
	if (i < rows-1 && j > 0 && boardArray[(i+1)*cols + j-1] == ' ') {
		newuMove = createUndoMove(i, j, i+1, j-1 );
		if (testUndoMove(boardArray, i,i+1,j,j-1, currentPlayer, t)) {
			*moves = CreateUndoMovelistNode(newuMove, *moves);
		}
		generateCaptureUndoMoves(boardArray, moves, currentPlayer, i, j, i+1, j-1, 0, t);
	}
	//Down-right
	if (i < rows-1 && j < cols-1 && boardArray[(i+1)*cols + j+1] == ' ') {
		newuMove = createUndoMove(i, j, i+1, j+1 );
		if (testUndoMove(boardArray, i,i+1,j,j+1, currentPlayer, t)) {
			*moves = CreateUndoMovelistNode(newuMove, *moves);
		}
		generateCaptureUndoMoves(boardArray, moves, currentPlayer, i, j, i+1, j+1, 0, t);
	}
}


void generatePawnUndoMoves(char *boardArray,  UNDOMOVELIST **moves, int currentPlayer, int i, int j, TIER t){
	UNDOMOVE newuMove;
	if (currentPlayer == BLACK_TURN) {
		//down 1.  Only a legal move if there is no piece there
		if (i < rows-2 && boardArray[(i+1)*cols + j] == ' ') {
			newuMove = createUndoMove(i, j, i+1, j);
			if (testUndoMove(boardArray, i,i+1,j,j, currentPlayer, t)) {
				*moves = CreateUndoMovelistNode(newuMove, *moves);
			}
		}
		//down 1, left 1.  Only a legal move if captures an enemy piece
		if (i < rows-2 && j != 0 && boardArray[(i+1)*cols + j-1] != ' ') {
			generateCaptureUndoMoves(boardArray, moves, currentPlayer, i, j, i+1, j-1, 0, t);
		}
		//down 1, right 1.  Only a legal move if it captures an enemy piece
		if (i < rows-2 && j != cols-1 && boardArray[(i+1)*cols + j+1] != ' ') {
			generateCaptureUndoMoves(boardArray, moves, currentPlayer, i, j, i+1, j+1, 0, t);
		}
	} else {
		//up 1.  Only a legal move if there is no piece there
		if (i > 1 && boardArray[(i-1)*cols + j] == ' ') {
			newuMove = createMove(i, j, i-1, j);
			if (testUndoMove(boardArray, i, i-1, j, j, currentPlayer, t)) {
				*moves = CreateUndoMovelistNode(newuMove, *moves);
			}
		}
		//up 1, left 1.  Only a legal move if captures an enemy piece
		if (i > 1  && j != 0 && boardArray[(i-1)*cols + j-1] != ' ') {
			generateCaptureUndoMoves(boardArray, moves, currentPlayer, i, j, i-1, j-1, 0, t);
		}
		//up 1, right 1.  Only a legal move if captures an enemy piece
		if (i > 1 && j != cols-1 && boardArray[(i-1)*cols + j+1] != ' ') {
			generateCaptureUndoMoves(boardArray, moves, currentPlayer, i, j, i-1, j+1, 0, t);
		}
	}
}


void generateKnightUndoMoves(char *boardArray,  UNDOMOVELIST **moves, int currentPlayer, int i, int j, TIER t){
	UNDOMOVE newuMove;
	//Left 2, Down 1
	if (i < rows-1 && j > 1 && boardArray[(i+1)*cols + j-2] == ' ') {
		newuMove = createUndoMove(i, j, i+1, j-2 );
		if(testUndoMove(boardArray, i, i+1, j, j-2, currentPlayer, t)) {

			*moves = CreateUndoMovelistNode(newuMove, *moves);
		}
		generateCaptureUndoMoves(boardArray, moves, currentPlayer, i, j, i+1, j-2, 0, t);
	}
	//Right 2, Down 1
	if (i < rows-1 && j < cols-2 && boardArray[(i+1)*cols + j+2] == ' ') {
		newuMove = createUndoMove(i, j, i+1, j+2 );
		if(testUndoMove(boardArray, i, i+1, j, j+2, currentPlayer, t)) {
			*moves = CreateUndoMovelistNode(newuMove, *moves);
		}
		generateCaptureUndoMoves(boardArray, moves, currentPlayer, i, j, i+1, j+2, 0, t);
	}
	//Down 2, Left 1
	if (i < rows-2 && j > 0 && boardArray[(i+2)*cols + j-1] == ' ') {
		newuMove = createUndoMove(i, j, i+2, j-1);
		if(testUndoMove(boardArray, i, i+2, j, j-1, currentPlayer, t)) {
			*moves = CreateUndoMovelistNode(newuMove, *moves);
		}
		generateCaptureUndoMoves(boardArray, moves, currentPlayer, i, j, i+2, j-1, 0, t);
	}
	//Down 2, Right 1
	if (i < rows-2 && j < cols-1 && boardArray[(i+2)*cols + j+1] == ' ') {
		newuMove = createUndoMove(i, j, i+2, j+1);
		if(testUndoMove(boardArray, i, i+2, j, j+1, currentPlayer, t)) {
			*moves = CreateUndoMovelistNode(newuMove, *moves);
		}
		generateCaptureUndoMoves(boardArray, moves, currentPlayer, i, j, i+2, j+1, 0, t);
	}
	//Left 2, Up 1
	if (i > 0 && j > 1 && boardArray[(i-1)*cols + j-2] == ' ') {
		newuMove = createUndoMove(i, j, i-1, j-2 );
		if(testUndoMove(boardArray, i, i-1, j, j-2, currentPlayer, t)) {
			*moves = CreateUndoMovelistNode(newuMove, *moves);
		}
		generateCaptureUndoMoves(boardArray, moves, currentPlayer, i, j, i-1, j-2, 0, t);
	}
	//Right 2, Up 1
	if (i > 0 && j < cols-2 && boardArray[(i-1)*cols + j+2] == ' ') {
		newuMove = createUndoMove(i, j, i-1, j+2 );
		if(testUndoMove(boardArray, i, i-1, j, j+2, currentPlayer, t)) {
			*moves = CreateUndoMovelistNode(newuMove, *moves);
		}
		generateCaptureUndoMoves(boardArray, moves, currentPlayer, i, j, i-1, j+2, 0, t);
	}
	//Up 2, Left 1
	if (i > 1 && j > 0 && boardArray[(i-2)*cols + j-1] == ' ') {
		newuMove = createUndoMove(i, j, i-2, j-1 );
		if(testUndoMove(boardArray, i, i-2, j, j-1, currentPlayer, t)) {

			*moves = CreateUndoMovelistNode(newuMove, *moves);
		}
		generateCaptureUndoMoves(boardArray, moves, currentPlayer, i, j, i-2, j-1, 0, t);
	}
	//Up 2, Right 1
	if (i > 1 && j < cols-1 && boardArray[(i-2)*cols + j+1] == ' ') {
		newuMove = createUndoMove(i, j, i-2, j+1 );
		if(testUndoMove(boardArray, i, i-2, j, j+1, currentPlayer, t)) {
			*moves = CreateUndoMovelistNode(newuMove, *moves);
		}
		generateCaptureUndoMoves(boardArray, moves, currentPlayer, i, j, i-2, j+1, 0, t);
	}
}

/*
** Generates the moves for a king piece.  First checks if the move is off the board.  Then
** checks of the piece taken, if any, is of the same team.  Then checks if the move puts
** the player in check.  Generates moves in all directions until it hits a piece.
*/

void generateQueenUndoMoves(char *boardArray,  UNDOMOVELIST **moves, int currentPlayer, int i, int j, TIER t){
	generateUndoMovesDirection(boardArray, moves, currentPlayer, i, j, UP, t);
	generateUndoMovesDirection(boardArray, moves, currentPlayer, i, j, DOWN, t);
	generateUndoMovesDirection(boardArray, moves, currentPlayer, i, j, LEFT, t);
	generateUndoMovesDirection(boardArray, moves, currentPlayer, i, j, RIGHT, t);
	generateUndoMovesDirection(boardArray, moves, currentPlayer, i, j, UL, t);
	generateUndoMovesDirection(boardArray, moves, currentPlayer, i, j, UR, t);
	generateUndoMovesDirection(boardArray, moves, currentPlayer, i, j, DL, t);
	generateUndoMovesDirection(boardArray, moves, currentPlayer, i, j, DR, t);
}

/*
** Generates the moves for a king piece.  First checks if the move is off the board.  Then
** checks of the piece taken, if any, is of the same team.  Then checks if the move puts
** the player in check.  Generates moves in the four diagonal directions until it hits a piece.
*/

void generateBishopUndoMoves(char *boardArray,  UNDOMOVELIST **moves, int currentPlayer, int i, int j, TIER t){
	generateUndoMovesDirection(boardArray, moves, currentPlayer, i, j, UL, t);
	generateUndoMovesDirection(boardArray, moves, currentPlayer, i, j, UR, t);
	generateUndoMovesDirection(boardArray, moves, currentPlayer, i, j, DL, t);
	generateUndoMovesDirection(boardArray, moves, currentPlayer, i, j, DR, t);
}


/*
** Generates the moves for a king piece.  First checks if the move is off the board.  Then
** checks of the piece taken, if any, is of the same team.  Then checks if the move puts
** the player in check.  Generates moves in 4 compass directions until it hits a piece.
*/

void generateRookUndoMoves(char *boardArray,  UNDOMOVELIST **moves, int currentPlayer, int i, int j, TIER t){
	generateUndoMovesDirection(boardArray, moves, currentPlayer, i, j, UP, t);
	generateUndoMovesDirection(boardArray, moves, currentPlayer, i, j, DOWN, t);
	generateUndoMovesDirection(boardArray, moves, currentPlayer, i, j, LEFT, t);
	generateUndoMovesDirection(boardArray, moves, currentPlayer, i, j, RIGHT, t);
}


/*
   Given a board and a list of moves, this function will create all moves
   in that direction until it hits another piece.  This is for the queen, bishop,
   and rook.  These three pieces can move in directions any number of spaces until
   they hit another piece.  They can only take that piece if it is of the other team.
   POSTCONDITION: moves is updated with all of the legal moves.
 */

void generateUndoMovesDirection(char* boardArray,  UNDOMOVELIST **moves, int currentPlayer, int i, int j, int direction, TIER t) {
	int i_inc=0, j_inc=0,  new_i=i, new_j=j;
	char piece;
	switch (direction) {
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
	UNDOMOVE newuMove;
	while(new_i < rows && new_i >= 0 && new_j < cols && new_j >= 0) {
		piece = boardArray[new_i*cols+new_j];
		if (piece == ' ') {
			if(testUndoMove(boardArray, i, new_i, j, new_j, currentPlayer, t)) {
				newuMove = createUndoMove(i, j, new_i, new_j);
				*moves = CreateUndoMovelistNode(newuMove, *moves);
			}
			generateCaptureUndoMoves(boardArray, moves, currentPlayer, i, j, new_i, new_j, 0, t);
		} else return;

		new_i += i_inc;

		new_j += j_inc;
	}
}


void generateCaptureUndoMoves(char* boardArray, UNDOMOVELIST **moves, int currentPlayer, int i, int j, int new_i, int new_j, int isReplacement, TIER t){
	UNDOMOVE newuMove;
	if(currentPlayer == WHITE_TURN) {
		if(specificPawnCount(boardArray, WHITE) < 5 && new_i != rows-1 && new_i != 0) {
			if(isReplacement == 0) {
				if(testCaptureUndoMove(boardArray, i, new_i, j, new_j, currentPlayer, WHITE_PAWN, t)) {
					newuMove = createCaptureUndoMove(i, j, new_i, new_j, WHITE_PAWN);
					*moves = CreateUndoMovelistNode(newuMove, *moves);
				}
			} else  {
				if(testReplaceCaptureUndoMove(boardArray, i, new_i, j, new_j, currentPlayer, WHITE_PAWN, t)) {
					newuMove = createReplaceCaptureUndoMove(i, j, new_i, new_j, WHITE_PAWN);
					*moves = CreateUndoMovelistNode(newuMove, *moves);
				}
			}
		}
		if(!isSpecificPiece(boardArray, WHITE_ROOK)) {
			if(isReplacement == 0) {
				if(testCaptureUndoMove(boardArray, i, new_i, j, new_j, currentPlayer, WHITE_ROOK, t)) {
					newuMove = createCaptureUndoMove(i, j, new_i, new_j, WHITE_ROOK);
					*moves = CreateUndoMovelistNode(newuMove, *moves);
				}
			} else {
				if(testReplaceCaptureUndoMove(boardArray, i, new_i, j, new_j, currentPlayer, WHITE_ROOK, t)) {
					newuMove = createReplaceCaptureUndoMove(i, j, new_i, new_j, WHITE_ROOK);
					*moves = CreateUndoMovelistNode(newuMove, *moves);
				}
			}
		}
		if(!isSpecificPiece(boardArray, WHITE_BISHOP)) {
			if(isReplacement == 0) {
				if(testCaptureUndoMove(boardArray, i, new_i, j, new_j, currentPlayer, WHITE_BISHOP, t)) {
					newuMove = createCaptureUndoMove(i, j, new_i, new_j, WHITE_BISHOP);
					*moves = CreateUndoMovelistNode(newuMove, *moves);
				}
			} else {
				if(testReplaceCaptureUndoMove(boardArray, i, new_i, j, new_j, currentPlayer, WHITE_BISHOP, t)) {
					newuMove = createReplaceCaptureUndoMove(i, j, new_i, new_j, WHITE_BISHOP);
					*moves = CreateUndoMovelistNode(newuMove, *moves);
				}
			}
		}
		if(!isSpecificPiece(boardArray, WHITE_KNIGHT)) {
			if(isReplacement == 0) {
				if(testCaptureUndoMove(boardArray, i, new_i, j, new_j, currentPlayer, WHITE_KNIGHT, t)) {
					newuMove = createCaptureUndoMove(i, j, new_i, new_j, WHITE_KNIGHT);
					*moves = CreateUndoMovelistNode(newuMove, *moves);
				}
			} else {
				if(testReplaceCaptureUndoMove(boardArray, i, new_i, j, new_j, currentPlayer, WHITE_KNIGHT, t)) {
					newuMove = createReplaceCaptureUndoMove(i, j, new_i, new_j, WHITE_KNIGHT);
					*moves = CreateUndoMovelistNode(newuMove, *moves);
				}
			}
		}

		if(!isSpecificPiece(boardArray, WHITE_QUEEN)) {
			if(isReplacement == 0) {
				if(testCaptureUndoMove(boardArray, i, new_i, j, new_j, currentPlayer, WHITE_QUEEN, t)) {
					newuMove = createCaptureUndoMove(i, j, new_i, new_j, WHITE_QUEEN);
					*moves = CreateUndoMovelistNode(newuMove, *moves);
				}
			} else {
				if(testReplaceCaptureUndoMove(boardArray, i, new_i, j, new_j, currentPlayer, WHITE_QUEEN, t)) {
					newuMove = createReplaceCaptureUndoMove(i, j, new_i, new_j, WHITE_QUEEN);
					*moves = CreateUndoMovelistNode(newuMove, *moves);
				}
			}
		}
	} else {
		if(specificPawnCount(boardArray, BLACK) < 5 && new_i != rows-1 && new_i != 0 ) {
			if(isReplacement == 0) {
				if(testCaptureUndoMove(boardArray, i, new_i, j, new_j, currentPlayer, BLACK_PAWN, t)) {
					newuMove = createCaptureUndoMove(i, j, new_i, new_j, BLACK_PAWN);
					*moves = CreateUndoMovelistNode(newuMove, *moves);
				}
			} else {
				if(testReplaceCaptureUndoMove(boardArray, i, new_i, j, new_j, currentPlayer, BLACK_PAWN, t)) {
					newuMove = createReplaceCaptureUndoMove(i, j, new_i, new_j, BLACK_PAWN);
					*moves = CreateUndoMovelistNode(newuMove, *moves);
				}
			}
		}
		if(!isSpecificPiece(boardArray, BLACK_ROOK)) {
			if(isReplacement == 0) {
				if(testCaptureUndoMove(boardArray, i, new_i, j, new_j, currentPlayer, BLACK_ROOK, t)) {
					newuMove = createCaptureUndoMove(i, j, new_i, new_j, BLACK_ROOK);
					*moves = CreateUndoMovelistNode(newuMove, *moves);
				}
			} else {
				if(testReplaceCaptureUndoMove(boardArray, i, new_i, j, new_j, currentPlayer, BLACK_ROOK, t)) {
					newuMove = createReplaceCaptureUndoMove(i, j, new_i, new_j, BLACK_ROOK);
					*moves = CreateUndoMovelistNode(newuMove, *moves);
				}
			}
		}
		if(!isSpecificPiece(boardArray, BLACK_BISHOP)) {
			if(isReplacement == 0) {
				if(testCaptureUndoMove(boardArray, i, new_i, j, new_j, currentPlayer, BLACK_BISHOP, t)) {
					newuMove = createCaptureUndoMove(i, j, new_i, new_j, BLACK_BISHOP);
					*moves = CreateUndoMovelistNode(newuMove, *moves);
				}
			} else {
				if(testReplaceCaptureUndoMove(boardArray, i, new_i, j, new_j, currentPlayer, BLACK_BISHOP, t)) {
					newuMove = createReplaceCaptureUndoMove(i, j, new_i, new_j, BLACK_BISHOP);
					*moves = CreateUndoMovelistNode(newuMove, *moves);
				}
			}
		}
		if(!isSpecificPiece(boardArray, BLACK_KNIGHT)) {
			if(isReplacement == 0) {
				if(testCaptureUndoMove(boardArray, i, new_i, j, new_j, currentPlayer, BLACK_KNIGHT, t)) {
					newuMove = createCaptureUndoMove(i, j, new_i, new_j, BLACK_KNIGHT);
					*moves = CreateUndoMovelistNode(newuMove, *moves);
				}
			} else {
				if(testReplaceCaptureUndoMove(boardArray, i, new_i, j, new_j, currentPlayer, BLACK_KNIGHT, t)) {
					newuMove = createReplaceCaptureUndoMove(i, j, new_i, new_j, BLACK_KNIGHT);
					*moves = CreateUndoMovelistNode(newuMove, *moves);
				}
			}
		}
		if(!isSpecificPiece(boardArray, BLACK_QUEEN)) {
			if(isReplacement == 0) {
				if(testCaptureUndoMove(boardArray, i, new_i, j, new_j, currentPlayer, BLACK_QUEEN, t)) {
					newuMove = createCaptureUndoMove(i, j, new_i, new_j, BLACK_QUEEN);
					*moves = CreateUndoMovelistNode(newuMove, *moves);
				}
			} else {
				if(testReplaceCaptureUndoMove(boardArray, i, new_i, j, new_j, currentPlayer, BLACK_QUEEN, t)) {
					newuMove = createReplaceCaptureUndoMove(i, j, new_i, new_j, BLACK_QUEEN);
					*moves = CreateUndoMovelistNode(newuMove, *moves);
				}
			}
		}
	}
}


int specificPawnCount(char* boardArray, int playerColor){
	int i,j, pawnCount = 0;

	for(i = 0; i < rows; i++) {
		for(j = 0; j < cols; j++) {
			if(playerColor == WHITE && boardArray[i*cols + j] == WHITE_PAWN)
				pawnCount++;
			else if(playerColor == BLACK && boardArray[i*cols + j] == BLACK_PAWN)
				pawnCount++;
		}
	}
	return pawnCount;
}


BOOLEAN isSpecificPiece(char* boardArray, char piece){
	int i,j;

	for(i = 0; i < rows; i++) {
		for(j = 0; j < cols; j++) {
			if(boardArray[i*cols + j] == piece)
				return TRUE;
		}
	}
	return FALSE;
}


UNDOMOVE createUndoMove(int rowi, int coli, int rowf, int colf) {
	UNDOMOVE um;
	coli += 10;
	colf += 10;
	rowi = rows - rowi;
	rowf = rows - rowf;
	um = (coli << 12) | (rowi << 8) | (colf << 4) | rowf;
	return um;
}


UNDOMOVE createCaptureUndoMove(int rowi, int coli, int rowf, int colf, char capturedPiece) {
	UNDOMOVE um;
	coli += 10;
	colf += 10;
	rowi = rows - rowi;
	rowf = rows - rowf;
	um = (capturedPiece << 16) | (coli << 12) | (rowi << 8) | (colf << 4) | rowf;
	return um;
}


UNDOMOVE createReplaceCaptureUndoMove(int rowi, int coli, int rowf, int colf, char capturedPiece) {
	UNDOMOVE um;
	coli += 10;
	colf += 10;
	rowi = rows - rowi;
	rowf = rows - rowf;
	um = (1 << 24) | (capturedPiece << 16) | (coli << 12) | (rowi << 8) | (colf << 4) | rowf;
	return um;
}


BOOLEAN testUndoMove(char *boardArray, int rowi, int rowf, int coli, int colf, int currentPlayer, TIER t) {

	BOOLEAN boardInCheck;
	char piece = boardArray[rowi*cols + coli];
	boardArray[rowf*cols + colf] = piece;
	boardArray[rowi*cols + coli] = ' ';
	TIER thisTier =  BoardToTier(boardArray);
	boardInCheck = inCheck(boardArray, currentPlayer);
	boardArray[rowi*cols + coli] = piece;
	boardArray[rowf*cols + colf] = ' ';
	if (boardInCheck == FALSE && t == thisTier) {
		//printf("The move is ri=%d ci=%d to rf%d cf=%d, isInCheck = %d\n", rowi, coli, rowf, colf, boardInCheck);
		return TRUE;
	} else {
		return FALSE;
	}
}


BOOLEAN testCaptureUndoMove(char *boardArray, int rowi, int rowf, int coli, int colf, int currentPlayer, char capturedPiece, TIER t) {

	BOOLEAN boardInCheck;
	char piece = boardArray[rowi*cols + coli];
	boardArray[rowf*cols + colf] = piece;
	boardArray[rowi*cols + coli] = capturedPiece;
	TIER thisTier = BoardToTier(boardArray);
	boardInCheck = inCheck(boardArray, currentPlayer);
	boardArray[rowi*cols + coli] = piece;
	boardArray[rowf*cols + colf] = ' ';
	//printf("thisTier=%d, t=%d: The move is:%d%d%d%d=%c\n", thisTier, t,rowi,coli,rowf,colf,capturedPiece);
	if (boardInCheck == FALSE && t == thisTier) {
		return TRUE;
	} else {
		return FALSE;
	}
}


BOOLEAN testReplaceCaptureUndoMove(char *boardArray, int rowi, int rowf, int coli, int colf, int currentPlayer, char capturedPiece, TIER t) {
	BOOLEAN boardInCheck;
	char piece = boardArray[rowi*cols + coli];
	if(currentPlayer == WHITE_TURN)
		boardArray[rowf*cols + colf] = BLACK_PAWN;
	else boardArray[rowf*cols + colf] = WHITE_PAWN;

	if(capturedPiece != 0 && capturedPiece != ' ')
		boardArray[rowi*cols + coli] = capturedPiece;
	else boardArray[rowi*cols + coli] = ' ';
	TIER thisTier = BoardToTier(boardArray);
	boardInCheck = inCheck(boardArray, currentPlayer);
	boardArray[rowi*cols + coli] = piece;
	boardArray[rowf*cols + colf] = ' ';
	//printf("boardInCheck=%d, t=%d, thisTier=%d", boardInCheck, t, thisTier);
	if (boardInCheck == FALSE && t == thisTier) {
		return TRUE;
	} else {
		return FALSE;
	}
}


int opposingPlayer(int player) {
	if(player == WHITE_TURN)
		return BLACK_TURN;
	else return WHITE_TURN;
}

void PrintUndoMove (UNDOMOVE umove)
{
	char rowf, colf, rowi, coli, capturedPiece;
	int isReplace;
	rowf = (umove & 15) + 48;
	colf = ((umove >> 4) & 15) - 10 + 97;
	rowi = ((umove >> 8) & 15) + 48;
	coli = ((umove >> 12) & 15) - 10 + 97;
	capturedPiece =  ((umove >> 16) & 255);
	isReplace = ((umove >> 24) & 1);
	if (capturedPiece == 0) {
		if(isReplace == 0) {
			printf("%c%c%c%c", coli, rowi, colf, rowf);
		} else printf("%c%c%c%c=1", coli, rowi, colf, rowf);
	} else {
		if(isReplace == 0) {
			printf("%c%c%c%c=%c", coli, rowi, colf, rowf, capturedPiece);
		}
		else {
			printf("%c%c%c%c=%c=1", coli, rowi, colf, rowf, capturedPiece);
		}
	}
}

/* Used for testing.  This function will print a MOVELIST to show all the
   possible moves for the current Board and current player */
void printUndoMoveList(UNDOMOVELIST *moves) {
	while(moves != NULL) {
		PrintUndoMove(moves->undomove);
		printf("\n");
		moves = moves->next;
	}
}

int countBits(int i) {
	int j, k = 1, counter = 0;
	for (j = 0; j < 32; j++) {
		if((((k << j) & i) != 0))
			counter++;
	}
	return counter;
}


/* Customizing The Board */

char *getBoard() {
	char *boardArray = SafeMalloc(sizeof(char) * rows*cols);
	int i = 0;
	char c, cPrev;
	while ((c = getchar())) {
		if(c != '\n' && c != 0) {
			if(c == '-') {
				boardArray[i] = ' ';
			}
			else {
				boardArray[i] = c;
			}
			i++;
		} else if (c == '\n' && cPrev == '\n') {
			break;
		}
		cPrev = c;
	}
	return boardArray;
}

BOOLEAN isLegalBoard(char *Board){
	int i;
	char piece;
	for (i = 0; i < rows*cols; i++) {
		piece = Board[i];
	}
	return TRUE;
}

STRING TierToString(TIER tier) {

	int i = 2;
	int* piecesArray = (int *) SafeMalloc(DISTINCT_PIECES * sizeof(int));
	piecesArray =  gTierToPiecesArray(tier, piecesArray);
	int numPieces = getNumPieces(piecesArray);
	STRING tierStr = (STRING) SafeMalloc(sizeof(char)*(numPieces +1));

	tierStr[0] = WHITE_KING;
	tierStr[1] = BLACK_KING;
	if(*(piecesArray) == 1) {
		tierStr[i] = WHITE_QUEEN;
		i++;
	}
	if(*(piecesArray+1) == 1) {
		tierStr[i] = WHITE_BISHOP;
		i++;
	}
	if(*(piecesArray+2) == 1) {
		tierStr[i] = WHITE_ROOK;
		i++;
	}
	if(*(piecesArray+3) == 1) {
		tierStr[i] = WHITE_KNIGHT;
		i++;
	}
	if(*(piecesArray+4) == 1) {
		tierStr[i] = BLACK_QUEEN;
		i++;
	}
	if(*(piecesArray+5) == 1) {
		tierStr[i] = BLACK_BISHOP;
		i++;
	}
	if(*(piecesArray+6) == 1) {
		tierStr[i] = BLACK_ROOK;
		i++;
	}
	if(*(piecesArray+7) == 1) {
		tierStr[i] = BLACK_KNIGHT;
		i++;
	}
	tierStr[i] = '\0';
	SafeFree(piecesArray);
	return tierStr;
}

int getNumPieces(int* piecesArray) {
	int numPieces = 0, i;
	for(i = 0; i < DISTINCT_PIECES-2; i++) {
		if(*(piecesArray+i) == 1)
			numPieces++;
	}
	return numPieces;
}


/************************************************************************
**
**  Hashing and Unhashing
**
************************************************************************/


char* unhash(POSITION position, char* board)
{
	if (gHashWindowInitialized) {
		TIERPOSITION tierpos;
		TIER tier;
		gUnhashToTierPosition(position, &tierpos, &tier);
		generic_hash_context_switch(Tier0Context+tier);
		return (char *) generic_hash_unhash(tierpos, board);
	} else return (char *) generic_hash_unhash(position, board);
}


POSITION hash(char* board, int turn)
{
	POSITION position;

	if (gHashWindowInitialized) {
		TIER tier = BoardToTier(board);
		generic_hash_context_switch(Tier0Context+tier);
		TIERPOSITION tierpos = generic_hash_hash((char*)board, turn);
		if(tier == 256) {
			printf("tier 256 called\n");
			printArray(board);
		}
		position = gHashToWindowPosition(tierpos, tier);
	} else position = generic_hash_hash((char*)board, turn);
	return position;
}

// $Log: not supported by cvs2svn $
// Revision 1.51  2006/12/19 20:00:51  arabani
// Added Memwatch (memory debugging library) to gamesman. Use 'make memdebug' to compile with Memwatch
//
// Revision 1.50  2006/12/07 08:51:56  vert84
// *** empty log message ***
//
// Revision 1.49  2006/12/07 08:24:52  vert84
// *** empty log message ***
//
// Revision 1.48  2006/12/07 08:06:43  vert84
// *** empty log message ***
//
// Revision 1.47  2006/12/07 07:40:50  vert84
// *** empty log message ***
//
// Revision 1.46  2006/12/07 05:55:35  vert84
// *** empty log message ***
//
// Revision 1.45  2006/12/07 04:51:00  vert84
// *** empty log message ***
//
// Revision 1.44  2006/12/07 02:19:58  vert84
// *** empty log message ***
//
// Revision 1.43  2006/12/07 02:18:23  vert84
// *** empty log message ***
//
// Revision 1.42  2006/12/07 01:40:49  vert84
// *** empty log message ***
//
// Revision 1.41  2006/12/04 00:47:09  vert84
// *** empty log message ***
//
// Revision 1.40  2006/11/02 02:57:51  vert84
// *** empty log message ***
//
// Revision 1.39  2006/10/17 10:45:21  max817
// HUGE amount of changes to all generic_hash games, so that they call the
// new versions of the functions.
//
// Revision 1.38  2006/10/03 08:10:03  scarr2508
// added ActualNumberOfPositions
// -sean
//
// Revision 1.37  2006/09/27 07:00:08  vert84
// Reverting to version from May 2006
//
// Revision 1.36  2006/08/16 03:46:28  runner139
// *** empty log message ***
//
// Revision 1.35  2006/08/14 00:46:01  runner139
// *** empty log message ***
//
// Revision 1.34  2006/08/09 17:23:17  runner139
// *** empty log message ***
//
// Revision 1.33  2006/08/09 02:10:49  runner139
// *** empty log message ***
//
// Revision 1.30  2006/08/08 21:58:39  runner139
// *** empty log message ***
//
// Revision 1.29  2006/08/08 07:36:32  runner139
// *** empty log message ***
//
// Revision 1.28  2006/08/05 08:02:44  runner139
// *** empty log message ***
//
// Revision 1.24  2006/07/28 01:58:14  runner139
// *** empty log message ***
//
// Revision 1.23  2006/07/18 07:47:26  runner139
// *** empty log message ***
//
// Revision 1.22  2006/07/18 02:06:15  runner139
// *** empty log message ***
//
// Revision 1.21  2006/07/17 02:40:19  max817
// Just CVS'ing this file in for Adam, since his CVS doesn't work right now.
// Now the make is no longer broken! -Max
//
// Revision 1.20  2006/07/11 03:29:45  runner139
// *** empty log message ***
//
// Revision 1.19  2006/07/08 03:25:29  runner139
// *** empty log message ***
//
// Revision 1.18  2006/07/06 03:08:09  runner139
// *** empty log message ***
//
// Revision 1.17  2006/06/29 23:41:20  runner139
// *** empty log message ***
//
// Revision 1.16  2006/06/29 22:08:23  runner139
// *** empty log message ***
//
// Revision 1.15  2006/06/27 23:57:41  runner139
// *** empty log message ***
//
// Revision 1.14  2006/06/27 05:38:14  runner139
// *** empty log message ***
//
// Revision 1.13  2006/06/27 05:18:18  runner139
// *** empty log message ***
//
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
//           in their getOption/setOption hash.
// mttc.c: Edited to handle conflicting types.  Created a PLAYER type for
//         gamesman.  mttc.c had a PLAYER type already, so I changed it.
// analysis.c: Changed initialization of option variable in analyze() to -1.
// db.c: Changed check in the getter functions (GetValueOfPosition and
//       getRemoteness) to check if gMenuMode is Evaluated.
// gameplay.c: Removed PlayAgainstComputer and PlayAgainstHuman.  Wrote PlayGame
//             which is a generic version of the two that uses to PLAYER's.
// gameplay.h: Created the necessary structs and types to have PLAYER's, both
//          Human and Computer to be sent in to the PlayGame function.
// gamesman.h: Really don't think I changed anything....
// globals.h: Also don't think I changed anything....both these I just looked at
//            and possibly made some format changes.
// textui.c: Redid the portion of the menu that allows you to choose opponents
//        and then play a game.  Added computer vs. computer play.  Also,
//           changed the analysis part of the menu so that analysis should
//        work properly with symmetries (if it is in getOption/setOption hash).
//
// Revision 1.3  2005/03/10 02:06:47  ogren
// Capitalized CVS keywords, moved Log to the bottom of the file - Elmer
//

POSITION StringToPosition(char* board) {
	int turn = 0;
	POSITION pos;
	char * first_semicolon = strchr(board, ';');
	if ( GetValue(board, "turn", GetInt, &turn) ) {
		*first_semicolon = '\0';
		pos = hash(board, turn);
		*first_semicolon = ';';
		return pos;
	} else {
		printf("Error: StringToPosition could not determine turn from board \"%s\".", board);
		return INVALID_POSITION;
	}
}

char * position_to_string_storage = NULL;

char* PositionToString(POSITION pos) {
	int turn;
	char * turn_string = (char *) malloc( 2 * sizeof(char) );
	char * board_string = (char *) malloc(rows * cols + 1);
	char * formatted;
	TIERPOSITION tierpos;
	TIER tier;

	unhash(pos, board_string);

	board_string[rows*cols] = '\0';
	gUnhashToTierPosition(pos, &tierpos, &tier);

	turn = generic_hash_turn(tierpos);

	turn_string[1] = '\0';
	switch(turn) {
		case 1:
			turn_string[0] = '1';
			break;
		case 2:
			turn_string[0] = '2';
			break;
		default:
			printf("ERROR: turn = %d\n", turn);
	}

	formatted = MakeBoardString(board_string,
	                            "turn", turn_string,
	                            "tier", StrFromI(tier),
	                            "");
	free(board_string);
	return formatted;
}

char * PositionToEndData(POSITION pos) {
	return NULL;
}
