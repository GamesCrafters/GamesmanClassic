// $Id: mtilechess.c,v 1.34 2008-05-01 00:27:18 phase_ac Exp $
//
/**
 * The above lines will include the name and log of the last person
 * to commit this file to CVS
 */

/*****************%*******************************************************
**
** NAME:        mtilechess.c
**
** DESCRIPTION: Tile Chess
**
** AUTHOR:      Brian Zimmer; Alan Roytman
**
** DATE:        2005-10-03 / 2005-12-15
**
** UPDATE HIST: Version 1.6 - Final verson for Fall 2005
**
**************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**
*************************************************************************/

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

STRING kGameName            = "Tile Chess";   /* The name of your game */
STRING kAuthorName          = "Alan Roytman, Brian Zimmer";   /* Your name(s) */
STRING kDBName              = "tilechess";   /* The name to store the database under */

BOOLEAN kPartizan            = TRUE;   /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN kGameSpecificMenu    = TRUE;   /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN kTieIsPossible       = TRUE;   /* TRUE if a tie is possible. FALSE if it is impossible.*/
BOOLEAN kLoopy               = TRUE;   /* TRUE if the game tree will have cycles (a rearranger style game). FALSE if it does not.*/

BOOLEAN kDebugMenu           = TRUE;   /* TRUE only when debugging. FALSE when on release. */
BOOLEAN kDebugDetermineValue = TRUE;   /* TRUE only when debugging. FALSE when on release. */

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
        "The board is arranged like a standard chess board, with the exception that it can change size. The rows are specified by numeric values, while the rows are specified by letters. A square is referenced by the column and then the row, i.e. a1.";

STRING kHelpOnYourTurn =
        "Enter your move in a style similar to algebraic notation for chess. Specify the beginning square with the rank and file (a letter then a number), and the destination square. All of the pieces move in the same way as regular chess, with a few exceptions. You can jump over your own pieces with pieces other than the knight, and pawns can move, as well as capture, forwards and backwards.\nEx: If you want to move a king from square b2 to b3, type b2b3.";

STRING kHelpStandardObjective =
        "Try to checkmate your opponent's king.";

STRING kHelpReverseObjective =
        "Try to get your king checkmated.";

STRING kHelpTieOccursWhen =
        "the current player is not in check and cannot move his/her king.";

STRING kHelpExample =
        "    +---+---+---+---+---+\n\
  3 |   |   |   |   |   |\n\
    +---+---+---+---+---+\n\
  2 |   | K | B | k |   |\n\
    +---+---+---+---+---+\n\
  1 |   |   |   |   |   |\n\
    +---+---+---+---+---+\n\
      a   b   c   d   e\n\
It is Player's turn (white/uppercase).\n\
  Player's move [(undo)/<ranki><filei><rankf><filef>] : b2b3\n\
\n\
    +---+---+---+---+---+\n\
  4 |   |   |   |   |   |\n\
    +---+---+---+---+---+\n\
  3 |   | K |   |   |   |\n\
    +---+---+---+---+---+\n\
  2 |   |   | B | k |   |\n\
    +---+---+---+---+---+\n\
  1 |   |   |   |   |   |\n\
    +---+---+---+---+---+\n\
      a   b   c   d   e\n\
It is Computer's turn (black/lowercase).\n\
\n\
    +---+---+---+---+\n\
  5 |   |   |   |   |\n\
    +---+---+---+---+\n\
  4 |   | K |   |   |\n\
    +---+---+---+---+\n\
  3 |   |   | B |   |\n\
    +---+---+---+---+\n\
  2 |   |   | k |   |\n\
    +---+---+---+---+\n\
  1 |   |   |   |   |\n\
    +---+---+---+---+\n\
      a   b   c   d\n\
It is Player's turn (white/uppercase).\n\
  Player's move [(undo)/<ranki><filei><rankf><filef>] : b4c4\n\
\n\
    +---+---+---+\n\
  5 |   |   |   |\n\
    +---+---+---+\n\
  4 |   | K |   |\n\
    +---+---+---+\n\
  3 |   | B |   |\n\
    +---+---+---+\n\
  2 |   | k |   |\n\
    +---+---+---+\n\
  1 |   |   |   |\n\
    +---+---+---+\n\
      a   b   c\n\
It is Computer's turn (black/lowercase).\n\
The match ends in a draw. Excellent strategies, Player and Computer\n"                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           ;


/*************************************************************************
**
** #defines and structs
**
**************************************************************************/
#define PLAYER1_TURN 1
#define PLAYER2_TURN 2

// Constants specifying directions to "look" on the board
#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3
#define UL 4
#define UR 5
#define DL 6
#define DR 7

typedef struct contextList {
	int hashedPieces;
	int context;
	int offset;
	int numPieces;
	struct contextList* next;
} contextList;

/*************************************************************************
**
** Global Variables
**
*************************************************************************/
int *indexToBoard; // PreProcessed array of legal placements
int *boardToIndex; // The reverse of the previous array (placement to an index)
contextList *cList;
char *theBoard = "                KBk      ";
int theCurrentPlayer = PLAYER1_TURN;
int BMAX = 0;

/*VARIANTS*/
BOOLEAN bishopVariant = TRUE; // Default is the Bishop Variant
BOOLEAN rookVariant = FALSE;

/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/
STRING MToS(MOVE theMove);
void reverse(char s[]);
void itoa(long long unsigned n, char s[], int base);
long long unsigned power(int base, int exp);
long long unsigned atobi(char s[], int base);
BOOLEAN isEqualString(char s[], char t[]);
int hashPieces(char *pieces);
int isLegalPlacement(long long unsigned place, int sideLength, int numPieces, BOOLEAN isolation);
void PreProcess(char *pieces);
POSITION hashBoard(char bA[], int currentPlayer);
POSITION hashBoardWithoutTiers(char bA[], int currentPlayer);
int getCurrTurn(POSITION position);
char *unhashBoard(POSITION position);
char *FillBoardArray(char *pieceArray, char *placeArray);
void generateKingMoves(char *bA, MOVELIST **moves, int place, int currentPlayer);
BOOLEAN isSameTeam(char piece, int currentPlayer);
MOVE createMove(char *bA, int init, int final, int sidelength);
BOOLEAN testMove(char *bA, int newspot, int origspot, int currentPlayer);
BOOLEAN inCheck(char *bA, int currentPlayer);
BOOLEAN kingCheck(char *bA, int place, int currentPlayer);
BOOLEAN isLegalBoard(char *bA, BOOLEAN isolation);
BOOLEAN isLegalFormat(char *board);
BOOLEAN isIsolation(char **board);
char *setBoard(char *board);
char *getBoard();
void generateBishopMoves(char *bA, MOVELIST **moves, int place, int currentPlayer);
void generateRookMoves(char *bA, MOVELIST **moves, int place, int currentPlayer);
void generateQueenMoves(char *bA, MOVELIST **moves, int place, int currentPlayer);
void generatePawnMoves(char *bA, MOVELIST **moves, int place, int currentPlayer);
void generateKnightMoves(char *bA, MOVELIST **moves, int place, int currentPlayer);
BOOLEAN rookCheck(char *bA, int place, int opKingPlace, int currentPlayer);
BOOLEAN bishopCheck(char *bA, int place, int opKingPlace, int currentPlayer);
BOOLEAN queenCheck(char *bA, int place, int opKingPlace, int currentPlayer);
BOOLEAN pawnCheck(char *bA, int place, int currentPlayer);
BOOLEAN knightCheck(char *bA, int place, int opKingPlace, int currentPlayer);
BOOLEAN canMove(char *bA, int currentPlayer);
void generateMovesDirection(char *bA, MOVELIST **moves, int place, int direction, int currentPlayer);
void quickSort(char *pieces, int l, int r);
int partition(char *pieces, int l, int r);
void piecesCombinations(char *pieces);
void addContextListNode(int context, int hashed, int numPieces);
BOOLEAN newContext(int hashval);
contextList *getContextNodeFromOffset(int offset);
contextList *getContextNodeFromHashPieces(int hashedPieces);
void initializePiecesArray(int *init_pieces, char *pieces);
int pieceValue(char piece);
int indexOf(char piece, int *init_pieces, int scanBoundary);
BOOLEAN legalCapture(int kingSpot, int pieceSpot, char *bA);
BOOLEAN opponentInCheck(POSITION position);
BOOLEAN opponentCanMove(POSITION position);
void fillFuturePieces(int *fP, char *pA, BOOLEAN *visited, int index, int sideLength, int *fpCounter);
BOOLEAN inFuturePieces(int *fP, int piecePlace, int *fpCounter);
int bitCount(long long unsigned n);
BOOLEAN isDirectionCheck(char *boardArray, int place, int direction, int opponentKingPlace, int currentPlayer);
void fillMove(MOVE move, char *moveStr);
MOVE getMove(STRING input);
void SetupTierStuff();
TIER TierPieceValue(char, int);
TIER getTier(char*);
TIER getInitialTier();
int alignPieceToTier(char, TIER, int);
TIERPOSITION getTierPosition(char*, int);
TIERPOSITION getInitialTierPosition();
TIERLIST* TierChildren(TIER);
char* tierToBoard(TIER, TIERPOSITION);
TIERPOSITION NumberOfTierPositions(TIER);
char PieceTierValue(TIER);
void unhashToTierPosition(POSITION pos, TIERPOSITION* tierpos, TIER *tier);
char *unhashBoardWithoutTiers(POSITION position);
char *TierToStringFunPtr(TIER);
BOOLEAN isLegalPos(POSITION);
int pieces (char*);
void printProperBoard(char*);
char* flushBoard(char*);
char* switchBoardSize(char*);

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
	int i, counter = 0, boardSize = (int)(pow((int)sqrt(strlen(theBoard))-2,2)), totalPieces = (int)sqrt(boardSize);
	long long unsigned largestBoard = 0;
	char *pieces = SafeMalloc(totalPieces+1);
	for (i = 0; i < strlen(theBoard); i++) {
		if (theBoard[i] != ' ') {
			pieces[counter++] = theBoard[i];
		}
	}
	pieces[counter] = '\0';
	gMoveToStringFunPtr = &MToS;
	for (i = 0; i < boardSize; i += (totalPieces+1)) {
		largestBoard = largestBoard | (long long unsigned)pow(2,i);
	}
	boardToIndex = (int*)SafeMalloc(sizeof(int)*pow(2,totalPieces*totalPieces));
	indexToBoard = (int*)SafeMalloc(sizeof(int)*pow(2,totalPieces*totalPieces));
	PreProcess(pieces); // Generate the legal placements for pieces
	/*gNumberOfPositions = (((MaxPlacementIndex*MaxPiecePermutationValue) +
	   MaxPiecePermutationValue-1) << 1) + MaxPlayerTurnValue; */
	gNumberOfPositions = (boardToIndex[largestBoard]*BMAX) + BMAX;
	/*
	   gInitialPosition = (boardToIndex[7]*BMAX) + generic_hash_hash(pieces,theCurrentPlayer)+0;//+ 0 for offset
	 */
	gInitialPosition = hashBoard(theBoard,theCurrentPlayer);
	SafeFree(pieces);
	SetupTierStuff();
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
	int currentPlayer, i, length;
	char piece;
	char *boardArray;
	boardArray = unhashBoard(position);
	currentPlayer = getCurrTurn(position);
	length = strlen(boardArray);
	for (i = 0; i < length; i++) {
		piece = boardArray[i];
		if (isSameTeam(piece,currentPlayer)) {
			// The piece we're look at is the current player's piece
			if (piece >= 'a' && piece <= 'z')
				piece = piece - 'a' + 'A'; // Capitalize piece for switch statement
			switch (piece) {
			case 'K':
				generateKingMoves(boardArray,&moves,i,currentPlayer);
				break;
			case 'B':
				generateBishopMoves(boardArray,&moves,i,currentPlayer);
				break;
			case 'R':
				generateRookMoves(boardArray,&moves,i,currentPlayer);
				break;
			case 'Q':
				generateQueenMoves(boardArray,&moves,i,currentPlayer);
				break;
			case 'P':
				generatePawnMoves(boardArray,&moves,i,currentPlayer);
				break;
			case 'N':
				generateKnightMoves(boardArray,&moves,i,currentPlayer);
				break;
			default:
				break;
			}
		}
	}
	SafeFree(boardArray);
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
	char *boardArray = unhashBoard(position);
	int filef;
	int rankf;
	int filei;
	int ranki;
	char pieceMoved;
	int sideLength = (int)sqrt(strlen(boardArray));
	int i, length = strlen(boardArray), minx = sideLength, x;
	//ANDing by 2^(6)-1 will give the last 6 bits
	for (i = 0; i < length; i++) {
		if (boardArray[i] != ' ') {
			x = i % sideLength;
			if (x < minx) {
				minx = x;
			}
		}
	}
	filef = move & 63;
	rankf = ((move >> 6) & 63) + minx - 1;
	filei = (move >> 12) & 63;
	ranki = ((move >> 18) & 63) + minx - 1;
	pieceMoved = boardArray[(sideLength-filei)*sideLength+ranki];
	boardArray[(sideLength-filei)*sideLength+ranki] = ' ';
	boardArray[(sideLength-filef)*sideLength+rankf] = pieceMoved;
	//printf("In DoMove: \n");
	//printProperBoard(boardArray);
	boardArray = flushBoard(boardArray);
	//printf("After flushing the board back: \n");
	//printProperBoard(boardArray);
	boardArray = switchBoardSize(boardArray);
	return hashBoard(boardArray,(getCurrTurn(position) == PLAYER1_TURN) ? PLAYER2_TURN : PLAYER1_TURN);
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
	char *boardArray = unhashBoard(position);
	int currentPlayer = getCurrTurn(position);
	if (inCheck(boardArray,currentPlayer) && !canMove(boardArray,currentPlayer)) {
		SafeFree(boardArray);
		// The king is checked and can't move
		return (gStandardGame) ? lose : win;
	}
	else if (!inCheck(boardArray,currentPlayer) && !canMove(boardArray,currentPlayer)) {
		// King is not in check and can't move - Stalemate
		SafeFree(boardArray);
		return tie;
	} else {
		SafeFree(boardArray);
		return undecided;
	}
}


/************************************************************************
**
** NAME:   maxInt     PrintPosition
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
	char *boardArray;
	int r, c, sideLength, x, y;
	boardArray = unhashBoard(position);
	sideLength = sqrt(strlen(boardArray));
	int bi=0, iterations = 0;
	int minx=sideLength, maxx=0, miny=sideLength, maxy=0;

	/* Determine the rectangle the pieces reside in.
	   This is done so as not to print unnecessary empty
	   rows and columns. */
	for(bi=0; bi<strlen(boardArray); bi++) {
		if(boardArray[bi]!=' ') {
			x = bi%sideLength;
			y = bi/sideLength;
			if(x<minx) minx=x;
			if(x>maxx) maxx=x;
			if(y<miny) miny=y;
			if(y>maxy) maxy=y;
		}
	}
	printf("\n");
	for(r = miny-1; r <= maxy+1; r++) {
		printf("    +");
		for(c = minx-1; c <= maxx+1; c++) {
			printf("---+");
		}
		printf("\n");
		printf("  %d |", sideLength - r);
		for(c = minx-1; c <= maxx+1; c++) {
			printf(" %c |", boardArray[r*sideLength+c]);
		}
		printf("\n");
	}
	printf("    +");
	for(c = minx-1; c <= maxx+1; c++) {
		printf("---+");
	}
	printf("\n");
	printf("     ");
	iterations = (maxx + 1) - (minx - 1) + 1;
	for(c = 0; c < iterations; c++) {
		printf(" %c  ", 97+c);
	}
	printf("\n");
	printf("%s\n",GetPrediction(position,playersName,usersTurn));
	printf("It is %s's turn (%s).\n",playersName,(usersTurn) ? "white/uppercase" : "black/lowercase");
	SafeFree(boardArray);
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
	char moveStr[7];
	fillMove(move,moveStr);
	printf("%s",moveStr); //base 63 num
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
		printf("%8s's move [(undo)/<ranki><filei><rankf><filef>] : ", playersName);

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
	if(strlen(input)!=4)
		return FALSE;
	else{
		char c = input[0];
		/* Make sure the rank and file are within the bounds
		   set by the current board size. */
		if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';
		if(c < 'A' || c > 'Z') return FALSE;
		c = input[2];
		if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';
		if(c < 'A' || c > 'Z') return FALSE;
		c = input[1];
		if(c < '1' || c > '9') return FALSE;
		c = input[3];
		if(c < '1' || c > '9') return FALSE;
	}
	return TRUE;
}


/************************************************************************
**
** NAME:        ConvertTextInputToMove
**
** DESCRIPTION: Converts the string input your internal move representation.
**         maxInt     Gamesman already checked the move with ValidTextInput
**              and ValidMove.
**
** INPUTS:      STRING input : The VALID string input from the user.
**
** OUTPUTS:     MOVE         : Move converted from user input.
**
************************************************************************/

MOVE ConvertTextInputToMove (STRING input)
{
	return getMove(input);
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
	int i = 0, numpieces = 0;
	char *board = NULL;
	char c;
	printf("\n");
	printf("Tile Chess Game Specific Menu\n\n");
	printf("1) Setup Board\n");
	printf("b) Back to previous menu\n\n");
	printf("Select an option: ");

	switch(GetMyChar()) {
	case 'Q': case 'q':
		ExitStageRight();
	case '1':
		printf("Limit on total pieces is 5. Tier solver will only work for boards\n\
with 3 pieces.  Each player MUST have one king and all other\n\
pieces may not exceed the number allowed in traditional chess\n\
(e.g. 2 knights). In addition, the pieces must all be connected\n\
You can describe the board starting at the uppermost row that\n\
contains at least one piece, and each row need only be described\n\
until the last piece on that row has been placed. Insert dashes for\n\
blanks. Press enter to start on a new row, and press enter twice\n\
when you are finished describing your last row.\n\
Here is a sample board:\n\
KB\n\
--Ppk\n\
The valid pieces are:\n\
K = king, P = pawn, Q = queen, B = bishop, R = rook, N = knight\n\
Upper-case letters indicate the pieces that belong to white, and\n\
lower-case letters indicate the pieces that belong to black:\n");
		do {
			if (board != NULL) {
				printf("Illegal board, re-enter:\n");
			}
			//getchar();
			board = getBoard();
			do {
				printf("Whose turn is it? (w/b): \n");
				c = GetMyChar();
			} while (c != 'w' && c != 'b');
			theCurrentPlayer = (c == 'w') ? PLAYER1_TURN : PLAYER2_TURN;
			if (isLegalFormat(board) && isIsolation(&board)) {
				for (i = 0; i < strlen(board); i++) {
					if (board[i] != ' ')
						numpieces++;
				}
				if (numpieces >= 7 && kSupportsTierGamesman &&
				    !kExclusivelyTierGamesman && gTierGamesman) {
					printf("Greater than 6 pieces on the board! Turning the tier solver OFF.\n");
					gTierGamesman = !gTierGamesman;
				}
				theBoard = setBoard(board);
			} else {
				theBoard = NULL;
			}
		} while(theBoard == NULL || inCheck(theBoard,(theCurrentPlayer == PLAYER1_TURN) ? PLAYER2_TURN : PLAYER1_TURN));
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
	return 1527;
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
	int numPieces = (int)sqrt(strlen(theBoard))-2, i, j = 0, offset;
	int temp = 0;
	char *pieces = SafeMalloc(numPieces+1);
	pieces[numPieces] = 0;
	for (i = 0; i < strlen(theBoard); i++) {
		if (theBoard[i] != ' ') {
			pieces[j++] = theBoard[i];
		}
	}
	quickSort(pieces,0,numPieces-1);
	if (numPieces == 3) {
		offset = hashPieces("KkQ") - 1;
	} else if (numPieces == 4) {
		offset = hashPieces("KkQq") - 24;
	} else {
		offset = hashPieces("KkQqR") - 288;
	}
	temp = hashPieces(pieces) - offset;
	SafeFree(pieces);

	return temp;
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
		bishopVariant = TRUE;
		rookVariant = FALSE;
	} else if (option == 2) {
		bishopVariant = FALSE;
		rookVariant = TRUE;
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

/* Returns a string representation of theMove */
STRING MToS(MOVE theMove) {
	STRING move = (STRING) SafeMalloc(7);
	fillMove(theMove,move);
	return move;
}

void fillMove(MOVE move, char *moveStr) {
	char filei = ((move >> 18) & 63) + 'a';
	int ranki = (move >> 12) & 63;
	char filef = ((move >> 6) & 63) + 'a';
	int rankf = move & 63;
	int counter = 0;
	moveStr[counter++] = filei;
	if (ranki > 9) {
		moveStr[counter++] = ranki/10 + '0';
		moveStr[counter++] = ranki%10 + '0';
	} else {
		moveStr[counter++] = ranki + '0';
	}
	moveStr[counter++] = filef;
	if (rankf > 9) {
		moveStr[counter++] = rankf/10 + '0';
		moveStr[counter++] = rankf%10 + '0';
	} else {
		moveStr[counter++] = rankf + '0';
	}
	moveStr[counter] = '\0';
}

/* Reverses the order of a string
   PRECONDITION: s must end with a null character
   POSTCONDITION: the characters in s are reversed */
void reverse(char s[]) {
	int c, i, j;
	for (i = 0, j = strlen(s)-1; i < j; i++, j--) {
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}

long long unsigned power (int base, int exp) {
	long long unsigned retval = 1;
	while (exp > 0) {
		retval *= base;
		exp--;
	}
	return retval;
}

MOVE getMove(STRING input) {
	int filei, filef, ranki, rankf, counter = 0;
	char c = input[counter++];
	MOVE m;
	filei = c - 'a';
	c = input[counter+1];
	if (isdigit(c)) {
		ranki = 10*(input[counter] - '0') + (input[counter+1] - '0');
		counter += 2;
	} else {
		ranki = input[counter] - '0';
		counter += 1;
	}
	c = input[counter++];
	filef = c - 'a';
	c = input[counter+1];
	if (isdigit(c)) {
		rankf = 10*(input[counter] - '0') + (input[counter+1] - '0');
		counter += 2;
	} else {
		rankf = input[counter] - '0';
		counter += 1;
	}
	m = (filei << 18) | (ranki << 12) | (filef << 6) | rankf;
	return m;
}

int bitCount(long long unsigned n) {
	int counter = 0;
	while (n != 0) {
		if ((n % 2) == 1) {
			counter++;
		}
		n = n >> 1;
	}
	return counter;
}

/* Returns the string of base 'base' as an integer
   PRECONDITION: s must end in a null character, and contain only the digits
   0-9, and the letters a-f (hexadecimal) */
long long unsigned atobi(char s[], int base) {
	int i;
	long long unsigned total = 0;
	char c;
	for (i = 0; s[i] != '\0'; i++) {
		c = s[i];
		total = total*base + ((isalpha(c)) ? toupper(c)-'A'+10 : c-'0');
	}
	return total;
}

/* Sets s[] to be a string representation of a
   base 'base' number n
   POSTCONDITION: s ends in a null character contains
   the digits of n*/
void itoa(long long unsigned n, char s[], int base){
	int i, nmodbase;
	long long unsigned sign;
	if ((sign = n) <0)
		n = -n;
	i=0;
	do {
		if ((nmodbase = n%base) > 9)
			s[i++] = nmodbase-10 + 'a';
		else
			s[i++] = n%base + '0';
	} while((n = n / base)>0);
	if(sign<0)
		s[i++] = '-';
	s[i] = '\0';
	reverse(s);
}

/* Fills the global arrays indexToBoard and boardToIndex
   with the values of valid placements.  Also sets up piece
   hashing/unhashing based on what the variant is set to. */
void PreProcess(char *pieces) {
	int world, j, totalPieces = strlen(pieces);
	long long unsigned legalcounter = 0, i = 0, largestBoard = 0;
	int init_pieces[3*totalPieces+1];
	char *tempPieces = (char *)SafeMalloc(sizeof(char)*totalPieces+1);
	for (world = 3; world <= totalPieces; world++) {
		largestBoard = 0;
		for (j = 0; j < world*world; j += (world+1)) {
			largestBoard = largestBoard | (long long unsigned)pow(2,j);
		}
		for (i = 0; i < largestBoard+1; i++) {
			if (bitCount(i) == world && isLegalPlacement(i,world,world,FALSE)) {
				indexToBoard[legalcounter] = i;
				boardToIndex[i] = legalcounter;
				legalcounter++;
			}
		}
	}
	strcpy(tempPieces,pieces);
	initializePiecesArray(init_pieces,tempPieces);
	quickSort(tempPieces,0,totalPieces-1);
	if (BMAX == 0) {
		BMAX = generic_hash_init(totalPieces,init_pieces,NULL,0);
		cList = (contextList *)SafeMalloc(sizeof(contextList));
		cList->context = 0;
		cList->next = NULL;
		cList->offset = 0;
		cList->numPieces = totalPieces;
		cList->hashedPieces = hashPieces(tempPieces);
	} else {
		int currContext = generic_hash_context_init();
		int init_pieces[3*(totalPieces-1)+1];
		generic_hash_context_switch(currContext);
		initializePiecesArray(init_pieces, tempPieces);
		addContextListNode(currContext+1,hashPieces(tempPieces),totalPieces);
		BMAX += generic_hash_init(totalPieces, init_pieces, NULL,0);
	}
	piecesCombinations(tempPieces);
	generic_hash_context_switch(cList->context);
	SafeFree(tempPieces);
}

/* Returns TRUE if the strings s and t are equal, and FALSE
   otherwise.
   PRECONDITION: s and t both end in a null character */
BOOLEAN isEqualString(char s[], char t[]) {
	int i;
	for (i = 0; s[i] != '\0' && t[i] != '\0'; i++) {
		if (s[i] != t[i]) {
			return FALSE;
		}
	}
	return TRUE;
}

int hashPieces(char *pieces) {
	int length = strlen(pieces), i, retval = 0;
	for(i = length-1; i >= 0; i--) {
		retval += pieceValue(pieces[i]) * pow(12,length-i-1);
	}
	return retval;
}

/* Checks to see if any piece is separated from the rest of the pieces.
   For each piece (1) that it finds, it makes sure that there is at least
   1 other piece around it.  If it is a legal board, this function returns
   1.  Otherwise, it returns 0. */
int isLegalPlacement(long long unsigned place, int sideLength, int numPieces, BOOLEAN isolation) {
	char *pA;
	BOOLEAN visitedPieces[1156]; //34x34 board
	int futurePieces[100];
	int i, pieces, fpCounter, boardSize = (int) pow(sideLength,2);
	memset(visitedPieces,FALSE,sizeof(BOOLEAN)*1156); //34x34 board
	pieces = fpCounter = 0;
	pA = (char *)SafeMalloc((boardSize+2) * sizeof(char));
	place = place | (long long unsigned)pow(2,boardSize);
	itoa(place,pA,2);
	pA++;
	i = 0;
	while (pA[i] == '0') {
		i++;
	}
	pieces++;
	fillFuturePieces(futurePieces,pA,visitedPieces,i,sideLength,&fpCounter);
	while (fpCounter > 0) {
		pieces++;
		visitedPieces[i] = TRUE;
		i = futurePieces[fpCounter-1];
		fpCounter--;
		fillFuturePieces(futurePieces,pA,visitedPieces,i,sideLength,&fpCounter);
	}
	pA--;
	SafeFree(pA);
	if (pieces  == numPieces) {
		return 1;
	} else {
		return 0;
	}
}

void fillFuturePieces(int *fP, char *pA, BOOLEAN *visited, int index, int sideLength, int *fpCounter) {
	int boardSize = (int) pow(sideLength,2);
	int piecePlace;
	piecePlace = index - sideLength;
	if (index-sideLength >= 0 && pA[piecePlace] == '1' && !visited[piecePlace] && !inFuturePieces(fP,piecePlace,fpCounter)) {
		fP[*fpCounter] = piecePlace;
		(*fpCounter)++;
	}
	piecePlace = index + sideLength;
	if (index+sideLength < boardSize && pA[index+sideLength] == '1' && !visited[piecePlace] && !inFuturePieces(fP,piecePlace,fpCounter)) {
		fP[*fpCounter] = piecePlace;
		(*fpCounter)++;
	}
	piecePlace = index + 1;
	if ((index+1)%sideLength != 0 && pA[index+1] == '1' && !visited[piecePlace] && !inFuturePieces(fP,piecePlace,fpCounter)) {
		fP[*fpCounter] = piecePlace;
		(*fpCounter)++;
	}
	piecePlace = index - 1;
	if (index%sideLength != 0 && pA[index-1] == '1' && !visited[piecePlace] && !inFuturePieces(fP,piecePlace,fpCounter)) {
		fP[*fpCounter] = piecePlace;
		(*fpCounter)++;
	}
	piecePlace = index - sideLength + 1;
	if (index-sideLength >= 0 && (index+1)%sideLength != 0 && pA[index-sideLength+1] == '1' && !visited[piecePlace] && !inFuturePieces(fP,piecePlace,fpCounter)) {
		fP[*fpCounter] = piecePlace;
		(*fpCounter)++;
	}
	piecePlace = index - sideLength - 1;
	if (index-sideLength >= 0 && index%sideLength != 0 && pA[index-sideLength-1] == '1' && !visited[piecePlace] && !inFuturePieces(fP,piecePlace,fpCounter)) {
		fP[*fpCounter] = piecePlace;
		(*fpCounter)++;
	}
	piecePlace = index + sideLength + 1;
	if (index+sideLength < boardSize && (index+1)%sideLength != 0 && pA[index+sideLength+1] == '1' && !visited[piecePlace] && !inFuturePieces(fP,piecePlace,fpCounter)) {
		fP[*fpCounter] = piecePlace;
		(*fpCounter)++;
	}
	piecePlace = index + sideLength - 1;
	if (index+sideLength < boardSize && index%sideLength != 0 && pA[index+sideLength-1] == '1' && !visited[piecePlace] && !inFuturePieces(fP,piecePlace,fpCounter)) {
		fP[*fpCounter] = piecePlace;
		(*fpCounter)++;
	}
}

BOOLEAN inFuturePieces(int *fP, int piecePlace, int *fpCounter) {
	int i, limit = *fpCounter;
	for (i = 0; i < limit; i++) {
		if (fP[i] == piecePlace) {
			return TRUE;
		}
	}
	return FALSE;
}








//TIER GAMESMAN FUNCTION
POSITION hashBoard(char boardArray[], int currentPlayer) {

	TIER tier;
	TIERPOSITION tierpos;
	POSITION pos;

	if (gHashWindowInitialized) {
		//printProperBoard(boardArray);
		tier = getTier(boardArray);
		//printf("hashing with tiers\n");
		tierpos = getTierPosition(boardArray, currentPlayer);
		//printf("The current tier is: %lld, and the current tier position is: %lld\n", tier, tierpos);
		if (tier == 142606337) {
			//printProperBoard(boardArray);
			//printf("Size of board: %d", strlen(boardArray));
			//printf("\n");
		}
		pos = gHashToWindowPosition(tierpos, tier);
		//printf("Corresponds to a position of: %llu", pos);
		//printf(" and a board of:\n");
		//printProperBoard(tierToBoard(tier, tierpos));
		//printf("\n\n");


	} else {
		pos = hashBoardWithoutTiers(boardArray, currentPlayer);
	}

	return pos;

}

/* Given a string which represents a board and the currentPlayer,
   this function returns a POSITION, which is a number representing the
   hashed value of the given board
   EXAMPLE:
   +---+---+---+---+---+
   3 |   |   |   |   |   |
   +---+---+---+---+---+
   2 |   | K | B | k |   |
   +---+---+---+---+---+
   1 |   |   |   |   |   |
   +---+---+---+---+---+
      a   b   c   d   e
      This board would be hashed as follows:
      (((boardToIndex[7]*BMAX) + hashBishopPieces("KBk")) << 1) + currentPlayer
      The number 7 comes  from a binary representation of a 3x3 board:
      000
      000
      111  ------------> 000000111 -------> 7 */
POSITION hashBoardWithoutTiers(char boardArray[], int currentPlayer) {
	POSITION N;
	int newPlacement = 0;
	int B = 0, hashedPlacement, boardLength = strlen(boardArray), sideLength = (int)sqrt(boardLength);
	int boardSize = (int)pow(sideLength-2,2), limit;
	char *snewPieces = (char *)SafeMalloc(((sideLength-2)+1) * sizeof(char));
	char *tempPieces = (char *)SafeMalloc(((sideLength-2)+1) * sizeof(char));
	contextList *tempNode;
	int bi=0,pi=0,x,y;
	int minx=sideLength, maxx=0, miny=sideLength, maxy=0;
	int hp;
	// Find the rectangle of the board that the pieces occupy
	for(bi=0; bi<boardLength; bi++) {
		if(boardArray[bi]!=' ') {
			x = bi%sideLength;
			y = bi/sideLength;
			if(x<minx) minx=x;
			if(x>maxx) maxx=x;
			if(y<miny) miny=y;
			if(y>maxy) maxy=y;
			snewPieces[pi++] = boardArray[bi];
		}
	}
	snewPieces[pi] = '\0';
	strcpy(tempPieces,snewPieces);
	quickSort(tempPieces,0,strlen(tempPieces)-1);
	hp = hashPieces(tempPieces);
	tempNode = getContextNodeFromHashPieces(hashPieces(tempPieces));
	// Squarize the rectangle
	limit = strlen(snewPieces);
	while ((maxx-minx) < limit-1) {
		if(minx > 0)
			minx--;
		else {
			int i;
			for (i = boardLength-1; i > 0; i--) {
				if (i%sideLength != 0) {
					boardArray[i] = boardArray[i - 1];
				}
			}
			maxx++;
		}
	}
	while ((maxy-miny) < limit-1) {
		if(miny>0)
			miny--;
		else {
			int i;
			for (i = boardLength-1; i >= sideLength; i--) {
				boardArray[i] = boardArray[i - sideLength];
			}
			maxy++;
		}
	}

	if (limit < sideLength - 2) { //if there was a capture
		boardSize = (int)pow(limit,2);
	}
	/* Go through the spaces on the board inside the square just obtained.
	   If a piece is found, put a 1 in the placement number corresponding to
	   the place it was found. Then add that piece to the pieces string.*/
	for(y=miny; y<=maxy; y++) {
		for(x=minx; x<=maxx; x++) {
			bi=y*sideLength+x;
			if(boardArray[bi]!=' ') {
				newPlacement = newPlacement | (1<<(boardSize-((y-miny)*(maxy-miny+1)+(x-minx))-1));
			}
		}
	}
	hashedPlacement = boardToIndex[newPlacement];
	generic_hash_context_switch(tempNode->context);
	B = generic_hash_hash(snewPieces,currentPlayer) + tempNode->offset;
	N = hashedPlacement*BMAX+B;
	SafeFree(snewPieces);
	SafeFree(tempPieces);
	return N;
}

/* UnhashBoard must be called first to set context
 * Returns whose turn it is */
int getCurrTurn(POSITION position) {
	int B = position%BMAX;
	if (gTierGamesman) {
		return position%2 ? position & 1 : PLAYER2_TURN;
	}
	else {
		contextList *temp = getContextNodeFromOffset(B);
		generic_hash_context_switch(temp->context);
		return (generic_hash_turn(B - temp->offset));
	}
}



/* Given a hashed value, this function returns a string
   which represents a board */
char *unhashBoardWithoutTiers(POSITION position) {
	int A, B = position%BMAX;
	contextList *temp = getContextNodeFromOffset(B);
	char pieces[(temp->numPieces)+1];
	int boardLength = (int)pow(temp->numPieces,2);
	char placement[boardLength+2], *bA;
	generic_hash_context_switch(temp->context);
	generic_hash_unhash(B - temp->offset,pieces);
	A = indexToBoard[position/BMAX] | (int)pow(2,boardLength);
	itoa(A,placement,2);
	bA = FillBoardArray(pieces,placement);
	return bA;
}

/* Given a placement array (string of 1's and 0's) and a string of pieces, will
   return a board with the corresponding pieces placed according to the placeArray. */
char *FillBoardArray(char *pieceArray, char *placeArray) {
	int c, r, sideLength, displayBoardSize, boardLength = strlen(placeArray)-1; // Subtract 1 for placeholder
	char *bA;
	sideLength = (int)sqrt(boardLength)+2; // +2 is for border
	displayBoardSize = (int)pow(sideLength,2);
	bA = (char *)SafeMalloc((displayBoardSize + 1) * sizeof(char));
	placeArray++; //Ignore bit that indicates number of significant bits
	for(c = 0; c < sideLength; c++) {
		bA[c] = ' '; // Top border
	}
	for (r = 1; r < sideLength-1; r++) {
		bA[r*sideLength] = ' '; // Left border
		for(c = 1; c < sideLength-1; c++) {
			if (*placeArray == '1') {
				bA[r*sideLength+c] = *pieceArray;
				pieceArray++;
			}
			else {
				bA[r*sideLength+c] = ' '; // Right border
			}
			placeArray++;
		}
		bA[r*sideLength+c] = ' ';
	}
	for(c = 0; c < sideLength; c++) {
		bA[displayBoardSize-sideLength+c] = ' '; // Bottom border
	}
	bA[displayBoardSize] = '\0';
	return bA;
}

/* Given a position N, a MOVELIST, and the placement of the king,
   this function "creates" all of the available moves for the king.
   POSTCONDITION: moves is updated with all the legal moves. */
void generateKingMoves(char *boardArray, MOVELIST **moves, int place, int currentPlayer) {
	int direction, sidelength;
	sidelength = sqrt(strlen(boardArray));
	/*MOVE UP*/
	direction = place-sidelength;
	if (testMove(boardArray,direction,place,currentPlayer)) {
		*moves = CreateMovelistNode(createMove(boardArray,place,direction,sidelength), *moves);
	}
	/*MOVE DOWN*/
	direction = place+sidelength;
	if (testMove(boardArray,direction,place,currentPlayer)) {
		*moves = CreateMovelistNode(createMove(boardArray,place,direction,sidelength), *moves);
	}
	/*MOVE RIGHT*/
	direction = place+1;
	if (testMove(boardArray,direction,place,currentPlayer)) {
		*moves = CreateMovelistNode(createMove(boardArray,place,direction,sidelength), *moves);
	}
	/*MOVE LEFT*/
	direction = place-1;
	if (testMove(boardArray,direction,place,currentPlayer)) {
		*moves = CreateMovelistNode(createMove(boardArray,place,direction,sidelength), *moves);
	}
	/*MOVE UPPER-RIGHT*/
	direction = place-sidelength+1;
	if (testMove(boardArray,direction,place,currentPlayer)) {
		*moves = CreateMovelistNode(createMove(boardArray,place,direction,sidelength), *moves);
	}
	/*MOVE UPPER-LEFT*/
	direction = place-sidelength-1;
	if (testMove(boardArray,direction,place,currentPlayer)) {
		*moves = CreateMovelistNode(createMove(boardArray,place,direction,sidelength), *moves);
	}
	/*MOVE LOWER-RIGHT*/
	direction = place+sidelength+1;
	if (testMove(boardArray,direction,place,currentPlayer)) {
		*moves = CreateMovelistNode(createMove(boardArray,place,direction,sidelength), *moves);
	}
	/*MOVE LOWER-LEFT*/
	direction = place+sidelength-1;
	if (testMove(boardArray,direction,place,currentPlayer)) {
		*moves = CreateMovelistNode(createMove(boardArray,place,direction,sidelength), *moves);
	}
}

/* Given a position N, a MOVELIST, and the placement of the bishop,
   this function "creates" all of the available moves for the bishop.
   POSTCONDITION: moves is updated with all the legal moves. */
void generateBishopMoves(char *bA, MOVELIST **moves, int place, int currentPlayer) {
	generateMovesDirection(bA,moves,place,UL,currentPlayer);
	generateMovesDirection(bA,moves,place,UR,currentPlayer);
	generateMovesDirection(bA,moves,place,DL,currentPlayer);
	generateMovesDirection(bA,moves,place,DR,currentPlayer);
}

/* Given a position N, a MOVELIST, and the placement of the rook,
   this function "creates" all of the available moves for the rook.
   POSTCONDITION: moves is updated with all the legal moves. */
void generateRookMoves(char *bA, MOVELIST **moves, int place, int currentPlayer) {
	generateMovesDirection(bA,moves,place,UP,currentPlayer);
	generateMovesDirection(bA,moves,place,DOWN,currentPlayer);
	generateMovesDirection(bA,moves,place,LEFT,currentPlayer);
	generateMovesDirection(bA,moves,place,RIGHT,currentPlayer);
}

void generateQueenMoves(char *bA, MOVELIST **moves, int place, int currentPlayer) {
	generateMovesDirection(bA,moves,place,UL,currentPlayer);
	generateMovesDirection(bA,moves,place,UR,currentPlayer);
	generateMovesDirection(bA,moves,place,DL,currentPlayer);
	generateMovesDirection(bA,moves,place,DR,currentPlayer);
	generateMovesDirection(bA,moves,place,UP,currentPlayer);
	generateMovesDirection(bA,moves,place,DOWN,currentPlayer);
	generateMovesDirection(bA,moves,place,LEFT,currentPlayer);
	generateMovesDirection(bA,moves,place,RIGHT,currentPlayer);
}

void generatePawnMoves(char *boardArray, MOVELIST **moves, int place, int currentPlayer) {
	int direction, sidelength;
	sidelength = sqrt(strlen(boardArray));
	/*MOVE UP*/
	direction = place-sidelength;
	if ((boardArray[direction] == ' ') && testMove(boardArray,direction,place,currentPlayer)) {
		*moves = CreateMovelistNode(createMove(boardArray,place,direction,sidelength), *moves);
	}
	/*MOVE DOWN*/
	direction = place+sidelength;
	if ((boardArray[direction] == ' ') && testMove(boardArray,direction,place,currentPlayer)) {
		*moves = CreateMovelistNode(createMove(boardArray,place,direction,sidelength), *moves);
	}
	/*MOVE RIGHT*/
	direction = place+1;
	if ((boardArray[direction] == ' ') && testMove(boardArray,direction,place,currentPlayer)) {
		*moves = CreateMovelistNode(createMove(boardArray,place,direction,sidelength), *moves);
	}
	/*MOVE LEFT*/
	direction = place-1;
	if ((boardArray[direction] == ' ') && testMove(boardArray,direction,place,currentPlayer)) {
		*moves = CreateMovelistNode(createMove(boardArray,place,direction,sidelength), *moves);
	}
	/*MOVE UPPER-RIGHT*/
	direction = place-sidelength+1;
	if ((boardArray[direction] != ' ') && testMove(boardArray,direction,place,currentPlayer)) {
		*moves = CreateMovelistNode(createMove(boardArray,place,direction,sidelength), *moves);
	}
	/*MOVE UPPER-LEFT*/
	direction = place-sidelength-1;
	if ((boardArray[direction] != ' ') && testMove(boardArray,direction,place,currentPlayer)) {
		*moves = CreateMovelistNode(createMove(boardArray,place,direction,sidelength), *moves);
	}
	/*MOVE LOWER-RIGHT*/
	direction = place+sidelength+1;
	if ((boardArray[direction] != ' ') && testMove(boardArray,direction,place,currentPlayer)) {
		*moves = CreateMovelistNode(createMove(boardArray,place,direction,sidelength), *moves);
	}
	/*MOVE LOWER-LEFT*/
	direction = place+sidelength-1;
	if ((boardArray[direction] != ' ') && testMove(boardArray,direction,place,currentPlayer)) {
		*moves = CreateMovelistNode(createMove(boardArray,place,direction,sidelength), *moves);
	}
}

void generateKnightMoves(char *boardArray, MOVELIST **moves, int place, int currentPlayer) {
	int boardSize = strlen(boardArray), sideLength = sqrt(boardSize);
	int col = place % sideLength, row = place / sideLength, destination;
	/*MOVE UP TWICE, LEFT ONCE*/
	destination = (row-2)*sideLength + (col-1);
	if ((destination > 0) && testMove(boardArray,destination,place,currentPlayer)) {
		*moves = CreateMovelistNode(createMove(boardArray,place,destination,sideLength), *moves);
	}
	/*MOVE UP TWICE, RIGHT ONCE*/
	destination = (row-2)*sideLength + (col+1);
	if ((destination > 0) && testMove(boardArray,destination,place,currentPlayer)) {
		*moves = CreateMovelistNode(createMove(boardArray,place,destination,sideLength), *moves);
	}
	/*MOVE UP ONCE, LEFT TWICE*/
	destination = (row-1)*sideLength + (col-2);
	if (((col - 2) >= 0) && testMove(boardArray,destination,place,currentPlayer)) {
		*moves = CreateMovelistNode(createMove(boardArray,place,destination,sideLength), *moves);
	}
	/*MOVE UP ONCE, RIGHT TWICE*/
	destination = (row-1)*sideLength + (col+2);
	if (((col + 2) < sideLength) && testMove(boardArray,destination,place,currentPlayer)) {
		*moves = CreateMovelistNode(createMove(boardArray,place,destination,sideLength), *moves);
	}
	/*MOVE DOWN TWICE, LEFT ONCE*/
	destination = (row+2)*sideLength + (col-1);
	if ((destination < boardSize) && testMove(boardArray,destination,place,currentPlayer)) {
		*moves = CreateMovelistNode(createMove(boardArray,place,destination,sideLength), *moves);
	}
	/*MOVE DOWN TWICE, RIGHT ONCE*/
	destination = (row+2)*sideLength + (col+1);
	if ((destination < boardSize) && testMove(boardArray,destination,place,currentPlayer)) {
		*moves = CreateMovelistNode(createMove(boardArray,place,destination,sideLength), *moves);
	}
	/*MOVE DOWN ONCE, LEFT TWICE*/
	destination = (row+1)*sideLength + (col-2);
	if (((col - 2) >= 0) && testMove(boardArray,destination,place,currentPlayer)) {
		*moves = CreateMovelistNode(createMove(boardArray,place,destination,sideLength), *moves);
	}
	/*MOVE DOWN ONCE, RIGHT TWICE*/
	destination = (row+1)*sideLength + (col+2);
	if (((col + 2) < sideLength) && testMove(boardArray,destination,place,currentPlayer)) {
		*moves = CreateMovelistNode(createMove(boardArray,place,destination,sideLength), *moves);
	}
}

/* Given a position N, a MOVELIST, the placement of a piece, and
   a direction, this function will "create" all of the available/legal
   moves of the piece at place in the given direction.
   POSTCONDITION: moves is updated with all of the legal moves. */
void generateMovesDirection(char *boardArray, MOVELIST **moves, int place, int direction, int currentPlayer) {
	int i, iterations=0, sideLength, x, y, x_inc=0, y_inc=0;
	char piece;
	sideLength = (int) sqrt(strlen(boardArray));
	x = place % sideLength;
	y = place / sideLength;
	switch (direction) {
	case UP:
		y_inc = -1;
		iterations = y;
		break;
	case DOWN:
		y_inc = 1;
		iterations = sideLength - 1 - y;
		break;
	case LEFT:
		x_inc = -1;
		iterations = x;
		break;
	case RIGHT:
		x_inc = 1;
		iterations = sideLength - 1 - x;
		break;
	case UL:
		y_inc = -1;
		x_inc = -1;
		iterations = ((x<y) ? x : y);
		break;
	case UR:
		y_inc = -1;
		x_inc = 1;
		iterations = ((y<(sideLength - 1 - x)) ? y : sideLength - 1 - x);
		break;
	case DL:
		y_inc = 1;
		x_inc = -1;
		iterations = ((x<(sideLength - 1 - y)) ? x : sideLength - 1 - y);
		break;
	case DR:
		y_inc = 1;
		x_inc = 1;
		iterations = (x>y) ? (sideLength - 1 - x) : (sideLength - 1 - y);
		break;
	}
	for(i=0; i<iterations; i++) {
		x += x_inc;
		y += y_inc;
		piece = boardArray[y*sideLength+x];
		if (piece == ' ') {
			if (testMove(boardArray,y*sideLength+x,place,currentPlayer)) {
				*moves = CreateMovelistNode(createMove(boardArray,place,y*sideLength+x,sideLength), *moves);
			}
		} else if (!isSameTeam(piece,currentPlayer)) {
			if (testMove(boardArray,y*sideLength+x,place,currentPlayer)) {
				*moves = CreateMovelistNode(createMove(boardArray,place,y*sideLength+x,sideLength), *moves);
			}
			return;
		} else {
			return;
		}
	}
}

BOOLEAN isDirectionCheck(char *boardArray, int place, int direction, int opponentKingPlace, int currentPlayer) {
	int i, iterations=0, sideLength, x, y, x_inc=0, y_inc=0, newplace;
	int difference = (place>opponentKingPlace) ? place-opponentKingPlace : opponentKingPlace-place;
	char piece, opKing = boardArray[opponentKingPlace];
	sideLength = (int) sqrt(strlen(boardArray));
	x = place % sideLength;
	y = place / sideLength;
	switch (direction) {
	case UP:
		if (difference % sideLength) {
			return FALSE;
		}
		y_inc = -1;
		iterations = y;
		break;
	case DOWN:
		if (difference % sideLength) {
			return FALSE;
		}
		y_inc = 1;
		iterations = sideLength - 1 - y;
		break;
	case LEFT:
		if ((place / sideLength) != (opponentKingPlace / sideLength)) {
			return FALSE;
		}
		x_inc = -1;
		iterations = x;
		break;
	case RIGHT:
		if ((place / sideLength) != (opponentKingPlace / sideLength)) {
			return FALSE;
		}
		x_inc = 1;
		iterations = sideLength - 1 - x;
		break;
	case UL:
		if ((difference-(difference/sideLength)) % sideLength) {
			return FALSE;
		}
		y_inc = -1;
		x_inc = -1;
		iterations = ((x<y) ? x : y);
		break;
	case UR:
		if ((difference+(difference/sideLength)+1) % sideLength) {
			return FALSE;
		}
		y_inc = -1;
		x_inc = 1;
		iterations = ((y<(sideLength - 1 - x)) ? y : sideLength - 1 - x);
		break;
	case DL:
		if ((difference+(difference/sideLength)+1) % sideLength) {
			return FALSE;
		}
		y_inc = 1;
		x_inc = -1;
		iterations = ((x<(sideLength - 1 - y)) ? x : sideLength - 1 - y);
		break;
	case DR:
		if ((difference-(difference/sideLength)) % sideLength) {
			return FALSE;
		}
		y_inc = 1;
		x_inc = 1;
		iterations = (x>y) ? (sideLength - 1 - x) : (sideLength - 1 - y);
		break;
	}
	for(i=0; i<iterations; i++) {
		x += x_inc;
		y += y_inc;
		newplace = y*sideLength+x;
		piece = boardArray[newplace];
		if (piece == opKing) {
			return TRUE;
		} else if (piece != ' ') {
			return FALSE;
		}
	}
	return FALSE;
}


/* Returns TRUE if the given piece belongs to currentPlayer,
   and FALSE otherwise. */
BOOLEAN isSameTeam(char piece, int currentPlayer) {
	/*if (gHashWindowInitialized) {
	   if (piece >= 'a' && piece <= 'z' && !currentPlayer)
	    return TRUE;
	   else if (piece >= 'A' && piece <= 'Z' && currentPlayer == PLAYER1_TURN)
	    return TRUE;
	   else
	    return FALSE;
	    }*/
	//else {
	if (piece >= 'a' && piece <= 'z' && currentPlayer == PLAYER2_TURN)
		return TRUE;
	else if (piece >= 'A' && piece <= 'Z' && currentPlayer == PLAYER1_TURN)
		return TRUE;
	else
		return FALSE;
	//}
}

/* Given the starting and ending indeces, will return
   a MOVE representing this move.
   PRECONDITION: The move is legal */
MOVE createMove(char *bA, int init, int final, int sidelength) {
	MOVE m;
	int filei, ranki, filef, rankf, i, minx = sidelength, length = strlen(bA), x;
	for (i = 0; i < length; i++) {
		if (bA[i] != ' ') {
			x = i%sidelength;
			if (x < minx) {
				minx = x;
			}
		}
	}
	filei = init % sidelength - minx + 1;
	filef = final % sidelength - minx + 1;
	ranki = sidelength-(init/sidelength);
	rankf = sidelength-(final/sidelength);
	m = (filei << 18) | (ranki << 12) | (filef << 6) | rankf;
	return m;
}

/* Checks to see whether the move will create an illegal board -
   Checks to see if it will result in an illegal placement,
   or if the move will cause the player to be in check.  Returns
   TRUE if the move is legal, and FALSE otherwise. */
BOOLEAN testMove(char *bA, int newspot, int origspot, int currentPlayer) {
	char newpiece;
	newpiece = bA[newspot];
	if (!isSameTeam(newpiece,currentPlayer)) {
		bA[newspot] = bA[origspot];
		bA[origspot] = ' ';
		if (isLegalBoard(bA,(newpiece == ' ') ? FALSE : TRUE) && !inCheck(bA,currentPlayer)) {
			bA[origspot] = bA[newspot];
			bA[newspot] = newpiece;
			return TRUE;
		} else {
			bA[origspot] = bA[newspot];
			bA[newspot] = newpiece;
		}
	}
	return FALSE;
}

/* Determines if the player is currently in check based on
   the given position.  Returns TRUE if the player is in check,
   and FALSE otherwise. */
BOOLEAN inCheck(char *bA, int currentPlayer) {
	int i, kingPlace, length = strlen(bA);
	char piece, king = (currentPlayer == PLAYER1_TURN) ? 'K' : 'k';
	for(i = 0; i < length; i++) {
		if (bA[i] == king) {
			kingPlace = i;
			break;
		}
	}
	for (i = 0; i < length; i++) {
		piece = bA[i];
		if (!isSameTeam(piece,currentPlayer)) {
			if (piece >= 'a' && piece <= 'z') {
				piece = piece - 'a' + 'A';
			}
			switch (piece) {
			case 'K':
				if (kingCheck(bA,i,currentPlayer) && legalCapture(kingPlace,i,bA)) {
					return TRUE;
				} else {
					break;
				}
			case 'B':
				if (bishopCheck(bA,i,kingPlace,currentPlayer) && legalCapture(kingPlace,i,bA)) {
					return TRUE;
				} else {
					break;
				}
			case 'R':
				if (rookCheck(bA,i,kingPlace,currentPlayer) && legalCapture(kingPlace,i,bA)) {
					return TRUE;
				} else {
					break;
				}
			case 'Q':
				if (queenCheck(bA,i,kingPlace,currentPlayer) && legalCapture(kingPlace,i,bA)) {
					return TRUE;
				} else {
					break;
				}
			case 'P':
				if (pawnCheck(bA,i,currentPlayer) && legalCapture(kingPlace,i,bA)) {
					return TRUE;
				} else {
					break;
				}
			case 'N':
				if (knightCheck(bA,i,kingPlace,currentPlayer) && legalCapture(kingPlace,i,bA)) {
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

BOOLEAN legalCapture(int kingSpot, int pieceSpot, char *bA) {
	char king = bA[kingSpot];
	BOOLEAN retval;
	bA[kingSpot] = bA[pieceSpot];
	bA[pieceSpot] = ' ';
	retval = isLegalBoard(bA,TRUE);
	bA[pieceSpot] = bA[kingSpot];
	bA[kingSpot] = king;
	return retval;
}

/* Checks if the king is checking the opposing king based on the given
   board, the placement of the king that might be checking, and the currentPlayer. */
BOOLEAN kingCheck(char *bA, int place, int currentPlayer) {
	char king;
	int sidelength;
	king = (currentPlayer == PLAYER2_TURN) ? 'k' : 'K';
	sidelength = sqrt(strlen(bA));
	return ((bA[place-sidelength]==king) ||
	        (bA[place+sidelength]==king) ||
	        (bA[place+1]==king) ||
	        (bA[place-1]==king) ||
	        (bA[place-sidelength+1]==king) ||
	        (bA[place-sidelength-1]==king) ||
	        (bA[place+sidelength+1]==king) ||
	        (bA[place+sidelength-1]==king)) ? TRUE : FALSE;
}

/* Checks if the bishop is checking the opposing king based on the given
   board, the placement of the bishop, and the currentPlayer. */
BOOLEAN bishopCheck(char *bA, int place, int opKingPlace, int currentPlayer) {
	if (place < opKingPlace) {
		return (isDirectionCheck(bA,place,DR,opKingPlace,currentPlayer) ||
		        isDirectionCheck(bA,place,DL,opKingPlace,currentPlayer));
	} else {
		return (isDirectionCheck(bA,place,UL,opKingPlace,currentPlayer) ||
		        isDirectionCheck(bA,place,UR,opKingPlace,currentPlayer));
	}
}

/* Checks if the rook is checking the opposing king based on the given
   board, the placement of the rook, and the currentPlayer. */
BOOLEAN rookCheck(char *bA, int place, int opKingPlace, int currentPlayer) {
	if (place < opKingPlace) {
		return (isDirectionCheck(bA,place,RIGHT,opKingPlace,currentPlayer) ||
		        isDirectionCheck(bA,place,DOWN,opKingPlace,currentPlayer));
	} else {
		return (isDirectionCheck(bA,place,LEFT,opKingPlace,currentPlayer) ||
		        isDirectionCheck(bA,place,UP,opKingPlace,currentPlayer));
	}
}

BOOLEAN queenCheck(char *bA, int place, int opKingPlace, int currentPlayer) {
	if (place < opKingPlace) {
		return (isDirectionCheck(bA,place,RIGHT,opKingPlace,currentPlayer) ||
		        isDirectionCheck(bA,place,DOWN,opKingPlace,currentPlayer) ||
		        isDirectionCheck(bA,place,DR,opKingPlace,currentPlayer) ||
		        isDirectionCheck(bA,place,DL,opKingPlace,currentPlayer));
	} else {
		return (isDirectionCheck(bA,place,UL,opKingPlace,currentPlayer) ||
		        isDirectionCheck(bA,place,UR,opKingPlace,currentPlayer) ||
		        isDirectionCheck(bA,place,LEFT,opKingPlace,currentPlayer) ||
		        isDirectionCheck(bA,place,UP,opKingPlace,currentPlayer));
	}
}

BOOLEAN pawnCheck(char *bA, int place, int currentPlayer) {
	char king;
	int sidelength;
	king = (currentPlayer == PLAYER2_TURN) ? 'k' : 'K';
	sidelength = sqrt(strlen(bA));
	return ((bA[place-sidelength+1]==king) ||
	        (bA[place-sidelength-1]==king) ||
	        (bA[place+sidelength+1]==king) ||
	        (bA[place+sidelength-1]==king)) ? TRUE : FALSE;
}

BOOLEAN knightCheck(char *bA, int place, int opKingPlace, int currentPlayer) {
	char king;
	int sidelength, boardSize, destination, row, col;
	int knightMoves[4];
	king = (currentPlayer == PLAYER2_TURN) ? 'k' : 'K';
	boardSize = strlen(bA);
	sidelength = sqrt(boardSize);
	row = place / sidelength;
	col = place % sidelength;
	if (place < opKingPlace) {
		/* DOWN ONCE, RIGHT TWICE */
		destination = (row + 1)*sidelength + (col + 2);
		knightMoves[0] = ((col + 2) < sidelength) ? destination : -1;
		/* DOWN TWICE, RIGHT ONCE */
		destination = (row + 2)*sidelength + (col + 1);
		knightMoves[1] = (destination < boardSize) ? destination : -1;
		/* DOWN TWICE, LEFT ONCE */
		destination = (row + 2)*sidelength + (col - 1);
		knightMoves[2] = (destination < boardSize) ? destination : -1;
		/* DOWN ONCE, LEFT TWICE */
		destination = (row + 1)*sidelength + (col - 2);
		knightMoves[3] = ((col - 2) >= 0) ? destination : -1;
		return ((knightMoves[0] != -1 && bA[knightMoves[0]] == king) ||
		        (knightMoves[1] != -1 && bA[knightMoves[1]] == king) ||
		        (knightMoves[2] != -1 && bA[knightMoves[2]] == king) ||
		        (knightMoves[3] != -1 && bA[knightMoves[3]] == king));
	} else {
		/* UP ONCE, RIGHT TWICE */
		destination = (row - 1)*sidelength + (col + 2);
		knightMoves[0] = ((col + 2) < sidelength) ? destination : -1;
		/* UP TWICE, RIGHT ONCE */
		destination = (row - 2)*sidelength + (col + 1);
		knightMoves[1] = (destination >= 0) ? destination : -1;
		/* UP TWICE, LEFT ONCE */
		destination = (row - 2)*sidelength + (col - 1);
		knightMoves[2] = (destination >= 0) ? destination : -1;
		/* UP ONCE, LEFT TWICE */
		destination = (row - 1)*sidelength + (col - 2);
		knightMoves[3] = ((col - 2) >= 0) ? destination : -1;
		return ((knightMoves[0] != -1 && bA[knightMoves[0]] == king) ||
		        (knightMoves[1] != -1 && bA[knightMoves[1]] == king) ||
		        (knightMoves[2] != -1 && bA[knightMoves[2]] == king) ||
		        (knightMoves[3] != -1 && bA[knightMoves[3]] == king));
	}
}

/* Checks if the king has any legal moves based on the given
   position.  Returns TRUE if the king has a legal move,
   and FALSE otherwise. */
BOOLEAN canMove(char *boardArray, int currentPlayer) {
	MOVELIST *moves = NULL;
	int length = strlen(boardArray);
	int i;
	char piece;
	for (i = 0; i < length; i++) {
		piece = boardArray[i];
		if (isSameTeam(piece,currentPlayer)) {
			// The piece we're look at is the current player's piece
			if (piece >= 'a' && piece <= 'z')
				piece = piece - 'a' + 'A'; // Capitalize piece for switch statement
			switch (piece) {
			case 'K':
				generateKingMoves(boardArray,&moves,i,currentPlayer);
				break;
			case 'B':
				generateBishopMoves(boardArray,&moves,i,currentPlayer);
				break;
			case 'R':
				generateRookMoves(boardArray,&moves,i,currentPlayer);
				break;
			case 'Q':
				generateQueenMoves(boardArray,&moves,i,currentPlayer);
				break;
			case 'P':
				generatePawnMoves(boardArray,&moves,i,currentPlayer);
				break;
			case 'N':
				generateKnightMoves(boardArray,&moves,i,currentPlayer);
				break;
			default:
				break;
			}
		}
	}
	return !(moves == NULL);
}

/* Temporary work around so that we do not need to hash a board that is
   not of size 9 - Our preprocessed arrays only deal with this size.
   Same as isLegalPlacement, but takes a board instead of a placement. */
BOOLEAN isLegalBoard(char *bA, BOOLEAN isolation) {
	int i, sideLength, length = strlen(bA), numPieces = 0;
	char temp[length+1];
	long long unsigned place;
	for (i = 0; bA[i] != '\0'; i++) {
		if (bA[i] == ' ') {
			temp[i] = '0';
		} else {
			temp[i] = '1';
			numPieces++;
		}
	}
	temp[i] = '\0';
	sideLength = (int) sqrt(length);
	place = atobi(temp,2);
	return isLegalPlacement(place,sideLength,numPieces,isolation); //ignore border regarding sideLength
}

BOOLEAN isIsolation(char **board) {
	char *newBoard;
	int i, j, numPieces = 0, numRows = 0, length = strlen(*board), rowsToAdd, colsToAdd;
	for (i = 0; i < length; i++) {
		if ((*board)[i] == '\n') {
			numRows++;
		} else if ((*board)[i] != '-') {
			numPieces++;
		}
	}
	newBoard = SafeMalloc(sizeof(char)*numPieces*numPieces+1);
	rowsToAdd = numPieces - numRows;
	for (i = 0; i < rowsToAdd; i++) {
		for (j = 0; j < numPieces; j++) {
			newBoard[numPieces*i + j] = ' ';
		}
	}
	j = 0;
	colsToAdd = numPieces;
	for (i = rowsToAdd*numPieces; i < numPieces*numPieces; i++) {
		if ((*board)[j] == '\n') {
			while (colsToAdd > 0) {
				newBoard[i++] = ' ';
				colsToAdd--;
			}
			colsToAdd = numPieces;
			j++;
		}
		newBoard[i] = ((*board)[j] == '-') ? ' ' : (*board)[j];
		colsToAdd--;
		j++;
	}
	newBoard[i] = '\0';
	SafeFree(*board);
	*board = newBoard;
	return isLegalBoard(newBoard,FALSE);
}

char *setBoard(char *board) {
	int i, j, row, col, numPieces = (int)sqrt(strlen(board)), boardSize = (int)pow(numPieces+2,2);
	char *newBoard = SafeMalloc(boardSize+1);
	BOOLEAN flushedDown = FALSE, flushedRight = FALSE;
	for (row = numPieces-1; row > 0; row--) {
		for (i = numPieces*(numPieces-1); i < numPieces*numPieces; i++) {
			if (board[i] != ' ') {
				flushedDown = TRUE;
			}
		}
		if (flushedDown) {
			break;
		} else {
			for (j = numPieces*numPieces-1; j >= numPieces; j--) {
				board[j] = board[j - numPieces];
				board[j - numPieces] = ' ';
			}
		}
	}
	for (col = numPieces-1; col > 0; col--) {
		for (i = numPieces-1; i < numPieces*numPieces; i += numPieces) {
			if (board[i] != ' ') {
				flushedRight = TRUE;
			}
		}
		if (flushedRight) {
			break;
		} else {
			for (j = numPieces*numPieces-1; j > 0; j--) {
				if (j%numPieces != 0) {
					board[j] = board[j - 1];
					board[j - 1] = ' ';
				}
			}
		}
	}
	memset(newBoard,' ',boardSize);
	newBoard[boardSize] = '\0';
	i = 0;
	for (row = 1; row < numPieces+1; row++) {
		for (col = 1; col < numPieces+1; col++) {
			newBoard[row*(numPieces+2)+col] = board[i++];
		}
	}
	SafeFree(board);
	return newBoard;
}

BOOLEAN isLegalFormat(char *board) {
	int i, row = 0, col = 0, numPieces = 0, length = strlen(board);
	char c;
	int pieceArray[12]; //6 pieces for white + 6 pieces for black
	memset(pieceArray,0,sizeof(int)*12);
	if (!(length >= 5)) { //smallest legal input is 3 pieces followed by two newlines
		return FALSE;
	}
	for (i = 0; (c = board[i]); i++) {
		if (c != '-' && c != '\n' && pieceValue(c) == -1) {
			return FALSE;
		} else if (c != '-' && c != '\n') {
			numPieces++;
			pieceArray[pieceValue(c)]++;
		}
	}
	if (numPieces <= 2) {
		return FALSE;
	}
	if (board[length-2] != '\n') {
		return FALSE;
	}
	for (i = 0; i < length-2; i++) {
		if (board[i] != '\n') {
			col++;
		} else {
			if (col > numPieces) {
				return FALSE;
			}
			col = 0;
			row++;
		}
	}
	if (row > numPieces) {
		return FALSE;
	}
	board[length-1] = '\0';
	return (pieceArray[pieceValue('K')] == 1 && pieceArray[pieceValue('k')] == 1 &&
	        pieceArray[pieceValue('Q')] <= 1 && pieceArray[pieceValue('q')] <= 1 &&
	        pieceArray[pieceValue('R')] <= 2 && pieceArray[pieceValue('r')] <= 2 &&
	        pieceArray[pieceValue('B')] <= 2 && pieceArray[pieceValue('b')] <= 2 &&
	        pieceArray[pieceValue('N')] <= 2 && pieceArray[pieceValue('n')] <= 2 &&
	        pieceArray[pieceValue('P')] <= 8 && pieceArray[pieceValue('p')] <= 8);
}

char *getBoard() {
	char *boardArray = SafeMalloc(1000*sizeof(char));
	int i = 0;
	char c;
	while ((c = getchar())) {
		boardArray[i] = c;
		if (c == '\n' && i != 0 && boardArray[i-1] == '\n') {
			boardArray[i+1] = '\0';
			break;
		}
		i++;
	}
	return boardArray;
}

void quickSort(char *pieces, int l, int r)
{
	int j;

	if( l < r )
	{
		j = partition(pieces, l, r);
		quickSort(pieces, l, j-1);
		quickSort(pieces, j+1, r);
	}

}

int partition(char *pieces, int l, int r) {
	int i, j, t;
	char pivot = pieces[l];
	i = l; j = r+1;
	while(1)
	{
		do ++i; while(pieceValue(pieces[i]) <= pieceValue(pivot) && i <= r);
		do --j; while(pieceValue(pieces[j]) > pieceValue(pivot));
		if(i >= j) break;
		t = pieces[i]; pieces[i] = pieces[j]; pieces[j] = t;
	}
	t = pieces[l]; pieces[l] = pieces[j]; pieces[j] = t;
	return j;
}

int pieceValue(char piece){
	int value = -1;
	switch(piece) {
	case 'K': value=0; break;
	case 'k': value=1; break;
	case 'Q': value=2; break;
	case 'q': value=3; break;
	case 'R': value=4; break;
	case 'r': value=5; break;
	case 'B': value=6; break;
	case 'b': value=7; break;
	case 'N': value=8; break;
	case 'n': value=9; break;
	case 'P': value=10; break;
	case 'p': value=11; break;
	}
	return value;
}

/*
 * Takes an alphabetized list as an argument.
 *
 */
void piecesCombinations(char *pieces) {
	int i,j,k,currContext = 0;
	int piecesLength = strlen(pieces);
	int hashval;
	char tempPieces[piecesLength]; // Holds string in length 1 less than pieces
	// Add number of postions from children to num positions of this context
	for(i=2; i<piecesLength; i++) {
		for(j=0; j<piecesLength; j++) {
			if(j!=i) {
				k=j;
				if(j>i)
					k--;
				tempPieces[k] = pieces[j];
			}
		}
		tempPieces[j-1] = '\0';
		hashval = hashPieces(tempPieces);
		if ((piecesLength - 1) > 2 && newContext(hashval)) {
			int init_pieces[3*(piecesLength-1)+1];
			currContext = generic_hash_context_init();
			generic_hash_context_switch(currContext);
			initializePiecesArray(init_pieces, tempPieces);
			addContextListNode(currContext+1,hashval,piecesLength-1);
			BMAX += generic_hash_init(piecesLength-1, init_pieces, NULL,0);
			piecesCombinations(tempPieces);
		}
	}
}

BOOLEAN newContext(int hashval){
	// Go through context list looking for pieces
	BOOLEAN notFound = TRUE;
	contextList *temp = cList;
	while (temp) {
		if (temp->hashedPieces == hashval) {
			notFound = FALSE;
			break;
		}
		temp = temp->next;
	}
	return notFound;
}

void initializePiecesArray(int *init_pieces, char *pieces){
	int i, uniquePieces = 0, totalPieces = strlen(pieces);
	int scanBoundary = 0, index;
	for(i=0; i<totalPieces; i++) {
		scanBoundary = 3*uniquePieces;
		if ((index = indexOf(pieces[i],init_pieces,scanBoundary)) == -1) {
			init_pieces[3*uniquePieces] = pieces[i];
			init_pieces[3*uniquePieces+1] = 1;
			init_pieces[3*uniquePieces+2] = 1;
			uniquePieces++;
		} else {
			(init_pieces[index+1])++;
			(init_pieces[index+2])++;
		}
	}
	init_pieces[3*uniquePieces] = -1;
}

int indexOf(char piece, int *init_pieces, int scanBoundary) {
	int i;
	for(i = 0; i < scanBoundary; i += 3) {
		if (piece == init_pieces[i]) {
			return i;
		}
	}
	return -1;
}

void addContextListNode(int newContext, int hashed, int numPieces){
	contextList *temp = cList;
	while (temp->next) {
		temp = temp->next;
	}
	temp->next = (contextList*)SafeMalloc(sizeof(contextList));
	temp->next->context = newContext;
	temp->next->hashedPieces = hashed;
	temp->next->offset = BMAX;
	temp->next->numPieces = numPieces;
	temp->next->next = NULL;
}

contextList *getContextNodeFromHashPieces(int hashedPieces) {
	contextList *temp = cList;
	while (hashedPieces != temp->hashedPieces) {
		temp = temp->next;
	}
	return temp;
}

contextList *getContextNodeFromOffset(int offset) {
	contextList *temp = cList;
	while (temp->next && (offset >= (temp->next->offset))) {
		temp = temp->next;
	}
	return temp;
}


/*****
      Check the borders for pieces, and if there are pieces there,
       move the whole board in a direction so that everything is
       located in the box of valid pieces.
 ****/
char* flushBoard(char* bA) {
	int i = 0;
	int boardLength = strlen(bA);
	int sideLength = (int) sqrt(boardLength);
	char* temp = (void*) SafeMalloc(boardLength * (sizeof(char)) + 1);
	int j = 0;
	int reverseRow = 0;
	int col = 0;

	strcpy(temp, bA);
	temp[boardLength] = 0;

	//first shift the board to the bottom left, and then shift the board
	//  up one and right one.

	for (i = 0; i < boardLength; i++) {
		bA[i] = ' ';
	}

	//scan by rows (reverse order)
	for (i = boardLength - 1; i >= 0; i--) {
		if (temp[i] != ' ') {
			reverseRow = sideLength - (int)(i / sideLength) - 1;

			//added for efficiency
			if (reverseRow == 1) {
				strcpy(bA, temp);
				break;
			}

			for (j = 0; j < boardLength; j++) {

				if (j >= (reverseRow - 1) * sideLength &&
				    j < boardLength - (1 - reverseRow) * sideLength)
					bA[j] = temp[j + (1 - reverseRow) * sideLength];
			}
			break;
		}
	}

	strcpy(temp, bA);
	for (i = 0; i < boardLength; i++) {
		bA[i] = ' ';
	}

	//scan by columns
	for (i = 0; i < boardLength + sideLength; i = i + sideLength) {
		//update the column counter (kind of a pain)
		if (i >= boardLength) {
			i = i % boardLength;
			i++;
		}

		if (temp[i] != ' ') {
			col = i % sideLength;

			//again for efficiency
			if (col == 1) {
				strcpy(bA, temp);
				break;
			}

			for (j = 0; j < boardLength; j++) {

				if (j % sideLength >= 1 - col &&
				    j % sideLength < sideLength + (1 - col))
					bA[j] = temp[j - 1 + col];
			}
			break;
		}
	}

	SafeFree(temp);
	return bA;
}


//Tier Gamesman stuff below
//NTS: put in the function prototypes
void SetupTierStuff() {
	TIERPOSITION maxtierpos;
	kSupportsTierGamesman = TRUE;
	gInitialTier = getInitialTier();
	gInitialTierPosition = getInitialTierPosition();
	maxtierpos = NumberOfTierPositions(gInitialTier);
	//printf("Initial tier: %llu\n", gInitialTier);
	//printf("Initial tier position: %llu\n", gInitialTierPosition);
	//printf("Max tier number: %llu\n", maxtierpos);
	//printf("The board converted back: %s\n", tierToBoard(gInitialTier, gInitialTierPosition));

	gTierChildrenFunPtr = &TierChildren;
	gNumberOfTierPositionsFunPtr = &NumberOfTierPositions;
	gTierToStringFunPtr = &TierToStringFunPtr;
	gIsLegalFunPtr = &isLegalPos;
}

//gives string representation of tier
char* TierToStringFunPtr(TIER tier) {

	//the value to be returned
	char* retval = (char*) SafeMalloc(45 * sizeof(char));

	TIER newtier;
	int i = 0;
	int index=0;
	int mask = 0;

	retval[45] = '\0';

	retval[0] = 'T';
	retval[1] = 'I';
	retval[2] = 'E';
	retval[3] = 'R';
	retval[4] = ':';
	retval[5] = ' ';
	index=6;

	for (; i<28; i++) {
		newtier = (unsigned long long) 1<<i;
		mask = (int) newtier & tier;

		if (mask) {
			retval[index] = PieceTierValue(mask);
			++index;
			retval[index] = ',';
			++index;
			retval[index] = ' ';
			++index;
		}

	}

	retval[index] = '\n';
	++index;
	retval[index] = '\0';
	return retval;
}


char *unhashBoard(POSITION position) {

	if (gHashWindowInitialized) {
		//printf("unhashing with tiers\n");
		TIER tier; TIERPOSITION tierposition;
		gUnhashToTierPosition(position, &tierposition, &tier);
		//printf("unhashing %lld with tier position: %lld\n", tier, tierposition);

		return (char*) tierToBoard(tier, tierposition);
	}
	else {
		return unhashBoardWithoutTiers(position);
	}
}

void unhashToTierPosition(POSITION pos, TIERPOSITION* tierpos, TIER *tier) {
	char* board = unhashBoardWithoutTiers(pos);
	int currentPlayer = pos % 2;
	*tier = getTier(board);
	*tierpos = getTierPosition(board, currentPlayer);
}

TIER TierPieceValue(char piece, int reset) {
	static int Rcount = 0;
	static int Bcount = 0;
	static int Ncount = 0;
	static int Pcount = 0;
	static int rcount = 0;
	static int bcount = 0;
	static int ncount = 0;
	static int pcount = 0;
	TIER value = (unsigned long long) 0;

	if (reset) {
		Rcount = Bcount = Ncount = Pcount = rcount = bcount = ncount = pcount = 0;
		return 0;
	}

	if (piece == 'K') {
		value = 1 << 0;
	} else if (piece == 'Q') {
		value = 1 << 1;
	} else if (piece == 'R') {
		value = 1 << (2 + Rcount);
		Rcount++;
	} else if (piece == 'B') {
		value = 1 << (4 + Bcount);
		Bcount++;
	} else if (piece == 'N') {
		value = 1 << (6 + Ncount);
		Ncount++;
	} else if (piece == 'P') {
		value = 1 << (8 + Pcount);
		Pcount++;
	} else if (piece == 'k') {
		value = (unsigned long long) 1 << 27;
	} else if (piece == 'q') {
		value = (unsigned long long) 1 << (27 - 1);
	} else if (piece == 'r') {
		value = (unsigned long long) 1 << (27 - 2 - rcount);
		rcount++;
	} else if (piece == 'b') {
		value = (unsigned long long) 1 << (27 - 4 - bcount);
		bcount++;
	} else if (piece == 'n') {
		value = (unsigned long long) 1 << (27 - 6 - ncount);
		ncount++;
	} else if (piece == 'p') {
		value = (unsigned long long) 1 << (27 - 8 - pcount);
		pcount++;
	}
	else value = 0;
	return value;
}

// converts a board into a tier
TIER getTier(char* board) {


	TIER retval = 0;
	int length = strlen(board);
	int i = 0;
	TierPieceValue(0,1); //reset the counters used in TierPieceValue
	for (i = 0; i < length; i++) {
		retval += TierPieceValue(board[i], 0);
	}

	return retval;
}

TIER getInitialTier() {
	//  TIER initialTier;
	//  int length = strlen(theBoard);
	//  for (int i = 0; i < length; i++) {
	//    initialTier += TierPieceValue(theBoard[i]);
	//  }
	//  return intialTier;
	return getTier(theBoard);
}

//Given a piece and a tier, it determines which position that piece appears in,
//  starting from the least significant bit (white's king)
int alignPieceToTier(char piece, TIER tempTier, int reset) {
	int total = 0; //keeps track of the nth piece in the tier
	int i = 0;
	TIER tempPieceValue = TierPieceValue(piece, 0);
	if (reset) {
		TierPieceValue(0, 1); //reset the counters in this function
		return -1;
	}

	for (i = 0; i < sizeof(tempTier) * 8; i++) {
		//if (TierPieceValue(piece, 0) & tempTier) {
		//  return total;
		//}

		//updates the nth piece in the tier because we've found a piece
		if (1 << i & tempTier) {
			if (1 << i == tempPieceValue)
				return total;
			else
				total++;
		}
	}
	return -1;
}




// converts a board into a tierposition
TIERPOSITION getTierPosition(char* board, int currentPlayer) {
	TIER thisTier = getTier(board);
	TIERPOSITION retval = 0;
	int length = strlen(board);
	int sideLength = (int)sqrt(length);
	int row = 0;
	int col = 0;
	int i;
	int numPieces = sideLength - 2;
	alignPieceToTier(0,0,1);
	//loop through the board starting from the rectangle that consists of the board
	// find the pieces and their row/col information
	for (i = sideLength; i < length - sideLength; i++) {
		//ignore the 0th and last spaces on the board
		if (!(i%sideLength == 0) && !(i%sideLength == sideLength - 1)) {
			//found a piece so keep track of its row/col info
			if (board[i] != ' ') {
				//retval += ((row<<2) + col) << (4 * alignPieceToTier(board[i], thisTier, 0));
				retval += ((row*numPieces) + col) *
				          power(numPieces*numPieces, alignPieceToTier(board[i], thisTier, 0));
			}
			col++;
		}
		if (i%sideLength == sideLength - 1) {
			row++;
			col = 0;
		}
	}
	return (retval<<1) + (currentPlayer & 1);
}

TIERPOSITION getInitialTierPosition() {
/*   TIER initialTier = getInitialTier(); */
/*   TIERPOSITION retval = 0; */
/*   int length = strlen(theBoard); */
/*   int sideLength = (int)sqrt(length); */
/*   int row = 0; */
/*   int col = 0; */
/*   //loop through the board starting from the rectangle that consists of the board */
/*   // find the pieces and their row/col information */
/*   for (int i = sideLength; i < length - sideLength; i++) { */
/*     //ignore the 0th and last spaces on the board */
/*     if (!(i%sideLength == 0) && !(i%sideLength == sideLength - 1)) { */
/*       //found a piece so keep track of its row/col info */
/*       if (theBoard[i] != ' ') { */
/*      retval += (row<<3 + col) << (6 * alignPieceToTier(theBoard[i])); */
/*       } */
/*       col++; */
/*     } */
/*     if (i%sideLength == sideLength - 1) { */
/*       row++; */
/*       col = 0; */
/*     } */
/*   } */
/*   return retval; */
	return getTierPosition(theBoard, theCurrentPlayer);
}

//given a tier, output char
char PieceTierValue(TIER tier) {

	//the index at which the '1' is
	int index=0;
	int mask = 0;

	tier = tier & 0xfffffff;

	char piece=' ';

	//go through loop until find '1'
	for (; index<28; index++) {

		mask = 1<<index;
		if (mask & tier)
			break;
	}

	if (index == 0) {
		piece = 'K';
	}
	else if (index == 1) {
		piece = 'Q';
	}
	else if (index == 2 || index == 3) {
		piece = 'R';
	}
	else if (index == 4 || index == 5) {
		piece = 'B';
	}
	else if (index == 6 || index == 7) {
		piece = 'N';
	}
	else if (index > 7 && index <= 13) {
		piece = 'P';
	}
	else if (index == 27) {
		piece = 'k';
	}
	else if (index == 26) {
		piece = 'q';
	}
	else if (index == 25 || index == 24) {
		piece = 'r';
	}
	else if (index == 23 || index == 22) {
		piece = 'b';
	}
	else if (index == 21 || index == 20) {
		piece = 'n';
	}
	else if (index >= 14 && index < 20) {
		piece = 'p';
	}

	return piece;
}





//GETING WRONG INPUT FOR TIERPOS

//given a tier and tierposition, this will return a char-array that
// represents the board
char* tierToBoard(TIER tier, TIERPOSITION tierpos) {

	//for the size of the board
	int i = 0;
	TIER temp = 0;
	int rootsize = 0;
	int fullsize = 0;
	int x = 0;
	int y = 0;
	int numPieces = 0;


	//the board
	char* board;

	//variables to keep track of iteration through tierpos and tier
	int tierpos_index=0;

	TIERPOSITION row;
	TIERPOSITION col;

	//1). get size of the board
	for (; i <= 63; i++) {
		temp = (unsigned long long) tier << i;
		temp = (unsigned long long) temp >> 63;
		rootsize += temp;
	}

	//to get rid of the player's turn
	tierpos = (unsigned long long) tierpos >> 1;

	numPieces = rootsize;
	//to account for blank border around board
	rootsize +=2;

	fullsize = rootsize * rootsize;

	//malloc space for board
	board = (char*) SafeMalloc((fullsize+1) * sizeof(char));

	//initialize board to be all spaces
	for (x = 0; x < fullsize; x++) {
		board[x] = ' ';
	}
	//null terminator just in case
	board[fullsize] = '\0';

	//3 fill in using tierpos and tier
	//look at first tier item, find the char, then go to tier pos
	for (y = 0; y < 28; y++) {
		temp = ((unsigned long long) (1 << y)) & tier;
		//printf("Tier: %llu, 1<<y: %d, Index: %d, PiecePresent?: %llu \n", tier, (1<<y), y, temp);

		if (temp) {
			//col = (tierpos >> (tierpos_index * 4)) & 3;
			col = (tierpos / power(numPieces*numPieces, tierpos_index)) % numPieces;
			//row = (tierpos >> (tierpos_index * 4 + 2)) & 3;
			row = (tierpos / (power(numPieces*numPieces, tierpos_index) * numPieces)) % numPieces;

			//printf("TierPosition: %llu, ", tierpos);
			//printf("Row: %llu, Col: %llu\n", row, col);

			board[(int) ((row+1)*rootsize + col + 1)] = PieceTierValue((unsigned long long) 1 << y);
			tierpos_index++;
		}

	}
	//printf("rootsize: %d\n", rootsize);
	//printProperBoard(board);

	return board;
}

void printProperBoard(char* ba)
{
	int i = 0;
	int fullsize = strlen(ba);
	int rootsize = (int) sqrt(fullsize);
	for (i = 0; i < fullsize; i++) {
		printf("%c", ba[i]);
		//printf("%d", i);
		if (((i+1) % ( rootsize )) == 0 )
			printf("\n");
	}
}


//return the factorial of that number
TIER factorial(TIER num) {
	if (num > 1) {
		return (num* factorial(num-1));
	} else {
		return 1;
	}
}



//returns number of tier positions
TIERPOSITION NumberOfTierPositions(TIER tier) {

	int numOfPieces=  0;
	int i = 0;
	TIER temp = (unsigned long long) 0;
	tier = tier & 0xfffffff;
	TIERPOSITION numOfPositions = 0;

	//shift until get that each individual bit
	for (i = 0; i < 28; i++) {
		temp = tier & (1 << i);
		if (temp) {
			numOfPieces++;
		}
	}

	//printf("number of pieces: %d \n", numOfPieces);

	for (i = 0; i < numOfPieces; i++) {
		//printf("numOfPositions: %llu \n", numOfPositions);
		//numOfPositions += ((((numOfPieces - 1) << 2) + i) << 4 * i);
		numOfPositions+=((((numOfPieces - 1) * numOfPieces) + i) *
		                 power(numOfPieces*numOfPieces,i));
	}

	//printf("numOfPositions: %llu \n", ((TIERPOSITION) (numOfPositions << 1)) + 2);
	return (numOfPositions << 1) + 2; // last shift + 1 is due to the current player
}



//returns a list of the children tiers
TIERLIST* TierChildren(TIER tier) {

	TIERLIST* children = NULL;
	TIER newtier = 0;
	TIER mask = 0;
	TIER tierbit = 0;
	int i = 0;

	//go through each bit in tier
	//if that bit is equal to 1, then turn it to 0 and
	//add that new tier to the tier list
	for (; i <= 63; i++) {

		tierbit = (unsigned long long) tier << i;
		tierbit = (unsigned long long) tierbit >> 63;
		mask = (unsigned long long) 1 << (63-i);

		//then create new tier and append it to the tierlist
		if (tierbit == 1) {
			//create new tier and add to list
			newtier = tier - mask;

			//if newtier is not just two kings or less
			if (newtier & 1 && newtier & 1<<27 && newtier != 1+(1<<27)) {
				//printf("\nMask: %llu\n", mask);
				//printf("CurrentTier: %llu\n", tier);
				//printf("NewTier: %llu\n", newtier);
				children = CreateTierlistNode(newtier, children);
			}
		}
		tierbit = 0;
	}

	children = CreateTierlistNode(tier, children);

	return children;
}

int pieces (char* ba)
{
	int pieces = 0;
	char cur = 0;
	int i = 0;
	do {
		cur = ba[i];
		if (cur != ' ' && cur != 0)
			pieces++;
		i++;
	} while (cur != 0);
	return pieces;
}

//If a piece is captured, the board shrinks in size. This function
//   factors that in and generates a new board with fewer spots.
char* switchBoardSize(char* ba)
{
	int bdlength = strlen(ba);
	int sidelength = (int) sqrt(bdlength);
	char* tempBoard = SafeMalloc((sidelength-1)*(sidelength-1)*(sizeof(char))+1);
	int piecesCounter = 0;
	int i;
	int y = 0;

	tempBoard[(sidelength-1)*(sidelength-1)] = 0;

	for (i = 0; i < bdlength; i++)
		if (ba[i] != ' ')
			piecesCounter++;

	if (piecesCounter == sidelength - 2) { //no pieces were taken
		SafeFree(tempBoard);
		//printf("Original board: \n");
		//printProperBoard(ba);
		return ba;
	}
	else { //new board is the old board minus the top and right edges
		//printf("piecesCounter: %d\n", piecesCounter);
		for (i = sidelength; i < bdlength; i++) {
			if (!(i % sidelength == sidelength - 1)) {
				//not at the right edge of the board. transfer these spots to new board
				tempBoard[y] = ba[i];
				y++;
			}
			tempBoard[y] = 0; //last character has to be a zero
		}
		SafeFree(ba);
		//printf("Smaller board: \n");
		//printProperBoard(tempBoard);
		return tempBoard;
	}
}

BOOLEAN isLegalPos(POSITION pos)
{
	//printf("The board: %s\n", unhashBoard(pos));
	//printf("It has %d pieces\n", pieces(unhashBoard(pos)));
	char* boardArray = unhashBoard(pos);
	int bdlength = strlen(boardArray);
	int sideLength = (int) sqrt(bdlength);
	int numPieces = sideLength - 2;
	int piecesCounter = 0;
	int i;
	BOOLEAN temp;
	int kingChecker = 0;
	char* flushedBoard = (void*) SafeMalloc(bdlength * (sizeof(char)) + 1);

	for (i = 0; i < bdlength; i++) {
		if (i < sideLength || !(i % sideLength) ||
		    (i % sideLength == sideLength - 1) || (i > bdlength - sideLength)) {
			if (boardArray[i] != ' ') {
				SafeFree(flushedBoard);
				SafeFree(boardArray);
				return FALSE;
			}
		}
		else
		if (boardArray[i] != ' ')
			piecesCounter++;
	}

	//printProperBoard(boardArray);
	//printf("\nPiecesCounter %d, NumPieces %d \n", piecesCounter, numPieces);

	if (piecesCounter < numPieces) {
		SafeFree(flushedBoard);
		SafeFree(boardArray);
		return FALSE;
	}

	for (i = 0; i < bdlength; i++) {
		if (boardArray[i] == 'k' || boardArray[i] == 'K')
			kingChecker++;
	}

	if (inCheck(boardArray, (getCurrTurn(pos) == PLAYER1_TURN ? PLAYER2_TURN : PLAYER1_TURN))) {
		SafeFree(flushedBoard);
		SafeFree(boardArray);
		return FALSE;
	}

	strcpy(flushedBoard, boardArray);
	flushedBoard = flushBoard(flushedBoard);
	for(i = 0; i < bdlength; i++) {
		if(flushedBoard[i] != boardArray[i]) {
			SafeFree(flushedBoard);
			SafeFree(boardArray);
			return FALSE;
		}
	}

	if (kingChecker != 2) {
		SafeFree(flushedBoard);
		SafeFree(boardArray);
		return FALSE;
	}
	temp = isLegalBoard(boardArray, TRUE);
	temp = temp && (pieces(boardArray) > 2);

	SafeFree(flushedBoard);
	SafeFree(boardArray);
	return temp;
}






// $Log: not supported by cvs2svn $
// Revision 1.32  2008/03/06 01:40:07  phase_ac
// 4-piece variant sort-of working (as long as you don't take any pieces... promptly crashes if you do)
//
// Revision 1.31  2007/12/29 05:14:31  phase_ac
// Tierized with 3-pieces. Debug statements removed. 4-piece games default to solving without TierGamesman.
//
// Revision 1.30  2007/12/06 04:33:26  phase_ac
// IT WORKS! (i think) for 3 pieces. Untested for four, primarily because it'll
// 2^6 times longer than it currently does (2 minutes currently). Lots of debug
// statements littered throughout the game.
//
// Revision 1.29  2007/12/04 08:03:18  phase_ac
// Fixed a bug in the isLegalFunPtr.
//
// Revision 1.28  2007/12/03 11:00:28  phase_ac
// Got the first tier solved! But database is corrupted, meaning isLegal is prolly not doing its job correctly. Uploading to let partner to do some debugging/fixing.
//
// Revision 1.27  2007/11/29 05:44:55  tjlee0909
// updating for partner
//
// Revision 1.26  2007/11/29 03:43:16  tjlee0909
// Fixed warnings
//
// Revision 1.24  2007/11/28 04:46:44  phase_ac
// Uploading to let partner edit (his was too buggy).
//
// Revision 1.23  2007/11/27 04:02:14  tjlee0909
// BUGZID:Taejun Lee: changed tierToBoard
//
// Revision 1.22  2007/11/27 03:27:52  phase_ac
// Fixed getTierPosition (and its dependent functions, like alignPieceToTier).
//
// Revision 1.21  2007/11/27 01:43:59  phase_ac
// Fixed random bugs, still crashes somewhere. Uploading mainly to allow partner to edit it as well.
//
// Revision 1.20  2007/11/26 07:17:23  tjlee0909
// BUGZID:
//
// Revision 1.19  2007/11/26 00:08:37  phase_ac
// Discovered/kind of fixed a casting problem when using 64bit integers. Still really buggy.
//
// Revision 1.18  2007/11/14 07:44:40  phase_ac
// Anthony - added the SetupTierStuff to initializeGame to commence testing (with crossed fingers)
//
// Revision 1.17  2007/11/14 07:20:55  tjlee0909
// Taejun Lee
// Wrote hashBoard and renamed the old function hashBoard to hashBoardWithoutTiers.
//
// Revision 1.16  2007/11/14 07:06:08  phase_ac
// Anthony - added the prototypes I forgot to add in the last commit, and fixed a warning.
//
// Revision 1.15  2007/11/14 07:00:39  phase_ac
// Anthony - updated getTierPosition to match spec
//
// Revision 1.14  2007/11/14 06:22:58  phase_ac
// Added some tiergamesman functions (not working with tiergamesman yet!)
//
// Revision 1.13  2007/11/12 23:19:49  phase_ac
// BUGZID:
//
//
// Fixed board input bug in the game specifc menu.
//
// Revision 1.12  2006/12/19 20:00:51  arabani
// Added Memwatch (memory debugging library) to gamesman. Use 'make memdebug' to compile with Memwatch
//
// Revision 1.11  2006/10/17 10:45:21  max817
// HUGE amount of changes to all generic_hash games, so that they call the
// new versions of the functions.
//
// Revision 1.10  2006/05/22 00:44:07  zwizeguy
// Implemented GameSpecific Menu, can play any game with 5 pieces.
//
// Revision 1.9  2006/04/17 09:26:33  zwizeguy
// Implemented with 5 pieces, can choose from all types: K, Q, R, B, N, P
//
// Revision 1.8  2006/02/12 02:30:57  kmowery
//
// Changed MoveToString to be gMoveToStringFunPtr.  Updated already existing MoveToString implementations (Achi, Dodgem, SquareDance, and Othello)
//
// Revision 1.7  2005/12/27 10:57:50  hevanm
// almost eliminated the existance of gDatabase in all files, with some declarations commented earlier that need to be hunt down and deleted from the source file.
//
// Revision 1.6  2005/12/23 01:34:12  brianzimmer
// Final version of Tile Chess that adheres to the 10 point checklist for the Fall 2005 semester.
//
// Revision 1.5  2005/12/19 11:11:04  brianzimmer
// Cleaned up and commented final version of Tile Chess for the Fall 2005 semester.
//
// Revision 1.4  2005/12/17 04:31:55  zwizeguy
// Implemented rook/bishop variants, and misere option.
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
	POSITION pos = INVALID_POSITION;
	if ( GetValue ( board, "pos", GetUnsignedLongLong, &pos ) ) {
		printf ( "pos = %lld\n", pos );
		return pos;
	} else {
		printf("Error: StringToPosition could not determine pos from board \"%s\".", board);
		return INVALID_POSITION;
	}
}


char* PositionToString(POSITION pos) {
	char * board = unhashBoard(pos);
	if (gHashWindowInitialized) {
		char * formatted = MakeBoardString(board,
			"turn", StrFromI(getCurrTurn(pos)),
			"pos", StrFromI(pos),
			"tier", TierstringFromPosition(pos),
			"");
		free ( board );
		return formatted;
	}
	else {
		char * formatted = MakeBoardString(board,
			"turn", StrFromI(getCurrTurn(pos)),
			"pos", StrFromI(pos),
			"");
		free ( board );
		return formatted;
	}

}

STRING MoveToString(MOVE theMove) {
	return StringDup("");
}

char * PositionToEndData(POSITION pos) {
	return NULL;
}
