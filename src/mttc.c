/************************************************************************
**
** NAME:        mttc.c
**
** DESCRIPTION: Tic Tac Chec
**
** AUTHOR:      Reman Child
**              University of California at Berkeley
**              Copyright (C) 2004. All rights reserved.
**
** CREDITS:     Questions answered by Dam, JJ, and Scott
**              Minor debugging by Jonathon Tsai
**
** 2004.3.30    First compilation of subroutines into mttc.c; this update
**              basically includes all functions below except for
**              ttc_hash and ttc_unhash, which still needs to be written
**                                                                -- rc
** 2004.3.31    Added row nums and letters to print position. Shall we
**              consider changing the Queen to Bishop? That's what the
**              original game has. I'll add some variants later.
**                                                                -- jt
**
** 2004.4.6     Added in support for generic_hash; still have to add in
**              modified ruleset as per discussion with Dom.
**                                                                -- rc
**
** 2004.4.18    Fixed some stuff, found lots more to fix, etc. etc.
**                                                                -- rc
**
** 2004.4.25    Complete rewrite! This version makes all board options
**              configurable, notably: board width, board height, number
**              of pieces for each side, types of pieces for each side,
**              and number of pieces necessary to get in a row to win!
**              Still need to rewrite GenerateMoves() and
**              GameSpecificMenu().
**                                                                 --rc
**
** 2004.4.26    Added in GenerateMoves() for all pieces except for the
**              pawns (which present a problem because they have
**              direction associated with their move).  Fixed some
**              memory leaks.
**
**              It turns out the original ruleset specified that moving
**              pieces was only allowed after the third turn.  Since
**              this would require me to add in extra state to an already
**              jam packed position, I have no plans to implement this
**              rule at this time.
**
**              Added in GameSpecificMenu as well, which is really
**              cool because the knob can be turned on almost every
**              aspect of this game.
**
**              Hmmm everything seems to be fine but....segfaults.
**                                                                 --rc
**
** 2004.4.27    It solves!  And better yet, it solves correctly (or so
**              it appears).  This looks to be an interesting game,
**              especially with the modifiability of all the game
**              parameters.  I've only just now started testing the limits
**              of hash and memory....so I'll probably be modifying
**              initial starting positions in the near future (to find
**              one more interesting than the current)
**
**              Fixed primitive position, which was buggy.
**
**              Added documentation (help sections)
**                                                                 --rc
** 2004.5.4     Fixed bug in moving pieces (ala knight thing)
**              Fixed printMove bug
**              Changed moves to 'a2a4' as opposed to 'qa2a4' format
**
**              Need to add in options, i.e. misere and diagonals off
**                                                                --rc
**
** 2004.5.11    Fixed another bug in inputting text - hopefully that's it.
**
** 2005.3.3     Added missing generic hasher prototypes and include time.h
**              To suppress GCC 3.4 warnings
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
#include <time.h>

extern STRING gValueString[];

POSITION gNumberOfPositions  = 0;
POSITION gInitialPosition    = 0;
POSITION gMinimalPosition    = 0;
POSITION kBadPosition        = -1; /* Need to ask michel if this is correct */

STRING kAuthorName         = "Reman Child";
STRING kGameName           = "Tic-Tac-Chec";
STRING kDBName             = "ttc";
BOOLEAN kPartizan           = TRUE;
BOOLEAN kDebugMenu          = FALSE;
BOOLEAN kGameSpecificMenu   = TRUE;
BOOLEAN kTieIsPossible      = FALSE;
BOOLEAN kLoopy              = TRUE;
BOOLEAN kDebugDetermineValue = TRUE;

/*
   Help strings that are pretty self-explanatory
 */

STRING kHelpGraphicInterface =
        "Not written yet";

STRING kHelpTextInterface    =
        "On your turn, you must make a move corresponding to the piece you choose\n\
to move. If you want to *place* a piece from off the board, moves are\n\
specified by the piece followed by the destination cell.  If you want to\n\
move a piece already on the board, you must specify the source cell as well.\n\
Pieces are specified by one of {k,b,q,n,r,p} for white, depending on the play\n\
options, and {K,B,Q,N,R,P} for Black.\n\
\n\
Examples of PLACING pieces:\n\
  Qa3, Bb2, nc3\n\
\n\
Examples of MOVING pieces:\n\
  Rb2b4    -  move the rook FROM b2 TO b4\n\
  Ba1c3    -  move the bishop FROM a1 TO c3\n\
\n\
Capturing consists of the same conventions; no differentiation is made in\n\
move nomenclature."                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         ;

STRING kHelpOnYourTurn =
        "You input a valid move - either a capture, placement, or simple piece move";

STRING kHelpStandardObjective =
        "The objective of this game is to get N of your pieces in a row, where N \n\
denotes the win condition that the game is initialized with (default: ).\n\
This includes a row in either the horizontal, vertical, or diagonal \n\
directions."                                                                                                                                                                                                                                         ;

STRING kHelpReverseObjective =
        "The misere option of this game is essentially meaningless.  There is no\n\
forcing combination to make your opponent 'lose'."                                                                                    ;

STRING kHelpTieOccursWhen =   /* Should follow 'A Tie occurs when... */
                            "";

/* Added \n\ to handle multi-lined strings. --- Robert Liao */
STRING kHelpExample =
        "Ok, Player and Computer, let us begin. \n\
\n\
Type '?' if you need assistance...\n\
\n\
\n\
  Player               Win Condition: 3\n\
 -------------------------------------------------------\n\
\n\
     --- --- ---\n\
  3 | - | - | - |      WHITE's offboard pieces: [ r n b ]\n\
     --- --- ---\n\
  2 | - | - | - |\n\
     --- --- ---\n\
  1 | - | - | - |      BLACK's offboard pieces: [ R N B ]\n\
     --- --- ---\n\
      a   b   c\n\
\n\
 -------------------------------------------------------\n\
\n\
  Game Prediction:  (Player should Win in 13)\n\
\n\
  Player's move > ra3\n\
\n\
  Computer               Win Condition: 3\n\
 -------------------------------------------------------\n\
\n\
     --- --- ---\n\
  3 | r | - | - |      WHITE's offboard pieces: [ n b ]\n\
     --- --- ---\n\
  2 | - | - | - |\n\
     --- --- ---\n\
  1 | - | - | - |      BLACK's offboard pieces: [ R N B ]\n\
     --- --- ---\n\
      a   b   c\n\
\n\
 -------------------------------------------------------\n\
\n\
  Game Prediction:  (Computer should Lose in 12)\n\
\n\
  > Computer's move : Bb2\n\
\n\
  Player               Win Condition: 3\n\
 -------------------------------------------------------\n\
\n\
     --- --- ---\n\
  3 | r | - | - |      WHITE's offboard pieces: [ n b ]\n\
     --- --- ---\n\
  2 | - | B | - |\n\
     --- --- ---\n\
  1 | - | - | - |      BLACK's offboard pieces: [ R N ]\n\
     --- --- ---\n\
      a   b   c\n\
\n\
 -------------------------------------------------------\n\
\n\
  Game Prediction:  (Player should Win in 11)\n\
\n\
  Player's move > bc3\n\
\n\
  Computer               Win Condition: 3\n\
 -------------------------------------------------------\n\
\n\
     --- --- ---\n\
  3 | r | - | b |      WHITE's offboard pieces: [ n ]\n\
     --- --- ---\n\
  2 | - | B | - |\n\
     --- --- ---\n\
  1 | - | - | - |      BLACK's offboard pieces: [ R N ]\n\
     --- --- ---\n\
      a   b   c\n\
\n\
 -------------------------------------------------------\n\
\n\
  Game Prediction:  (Computer should Lose in 10)\n\
\n\
  > Computer's move : Rb3\n\
\n\
  Player               Win Condition: 3\n\
 -------------------------------------------------------\n\
\n\
     --- --- ---\n\
  3 | r | R | b |      WHITE's offboard pieces: [ n ]\n\
     --- --- ---\n\
  2 | - | B | - |\n\
     --- --- ---\n\
  1 | - | - | - |      BLACK's offboard pieces: [ N ]\n\
     --- --- ---\n\
      a   b   c\n\
\n\
 -------------------------------------------------------\n\
\n\
  Game Prediction:  (Player should Win in 9)\n\
\n\
  Player's move > c3b2\n\
\n\
  Computer               Win Condition: 3\n\
 -------------------------------------------------------\n\
\n\
     --- --- ---\n\
  3 | r | R | - |      WHITE's offboard pieces: [ n ]\n\
     --- --- ---\n\
  2 | - | b | - |\n\
     --- --- ---\n\
  1 | - | - | - |      BLACK's offboard pieces: [ N B ]\n\
     --- --- ---\n\
      a   b   c\n\
\n\
 -------------------------------------------------------\n\
\n\
  Game Prediction:  (Computer should Lose in 8)\n\
\n\
  > Computer's move : b3b2\n\
\n\
  Player               Win Condition: 3\n\
 -------------------------------------------------------\n\
\n\
     --- --- ---\n\
  3 | r | - | - |      WHITE's offboard pieces: [ n b ]\n\
     --- --- ---\n\
  2 | - | R | - |\n\
     --- --- ---\n\
  1 | - | - | - |      BLACK's offboard pieces: [ N B ]\n\
     --- --- ---\n\
      a   b   c\n\
\n\
 -------------------------------------------------------\n\
\n\
  Game Prediction:  (Player should Win in 7)\n\
\n\
  Player's move > ba1\n\
\n\
  Computer               Win Condition: 3\n\
 -------------------------------------------------------\n\
\n\
     --- --- ---\n\
  3 | r | - | - |      WHITE's offboard pieces: [ n ]\n\
     --- --- ---\n\
  2 | - | R | - |\n\
     --- --- ---\n\
  1 | b | - | - |      BLACK's offboard pieces: [ N B ]\n\
     --- --- ---\n\
      a   b   c\n\
\n\
 -------------------------------------------------------\n\
\n\
  Game Prediction:  (Computer should Lose in 6)\n\
\n\
  > Computer's move : Ba2\n\
\n\
  Player               Win Condition: 3\n\
 -------------------------------------------------------\n\
\n\
     --- --- ---\n\
  3 | r | - | - |      WHITE's offboard pieces: [ n ]\n\
     --- --- ---\n\
  2 | B | R | - |\n\
     --- --- ---\n\
  1 | b | - | - |      BLACK's offboard pieces: [ N ]\n\
     --- --- ---\n\
      a   b   c\n\
\n\
 -------------------------------------------------------\n\
\n\
  Game Prediction:  (Player should Win in 5)\n\
\n\
\n\
  Player's move > a1b2\n\
\n\
\n\
  Computer               Win Condition: 3\n\
 -------------------------------------------------------\n\
\n\
     --- --- ---\n\
  3 | r | - | - |      WHITE's offboard pieces: [ n ]\n\
     --- --- ---\n\
  2 | B | b | - |\n\
     --- --- ---\n\
  1 | - | - | - |      BLACK's offboard pieces: [ R N ]\n\
     --- --- ---\n\
      a   b   c\n\
\n\
 -------------------------------------------------------\n\
\n\
  Game Prediction:  (Computer should Lose in 4)\n\
\n\
  > Computer's move : Rc1\n\
\n\
  Player               Win Condition: 3\n\
 -------------------------------------------------------\n\
\n\
     --- --- ---\n\
  3 | r | - | - |      WHITE's offboard pieces: [ n ]\n\
     --- --- ---\n\
  2 | B | b | - |\n\
     --- --- ---\n\
  1 | - | - | R |      BLACK's offboard pieces: [ N ]\n\
     --- --- ---\n\
      a   b   c\n\
\n\
 -------------------------------------------------------\n\
\n\
  Game Prediction:  (Player should Win in 3)\n\
\n\
  Player's move > nc2\n\
\n\
  Computer               Win Condition: 3\n\
 -------------------------------------------------------\n\
\n\
     --- --- ---\n\
  3 | r | - | - |      WHITE's offboard pieces: [ ]\n\
     --- --- ---\n\
  2 | B | b | n |\n\
     --- --- ---\n\
  1 | - | - | R |      BLACK's offboard pieces: [ N ]\n\
     --- --- ---\n\
      a   b   c\n\
\n\
 -------------------------------------------------------\n\
\n\
  Game Prediction:  (Computer should Lose in 2)\n\
\n\
  > Computer's move : a2b3\n\
\n\
  Player               Win Condition: 3\n\
 -------------------------------------------------------\n\
\n\
     --- --- ---\n\
  3 | r | B | - |      WHITE's offboard pieces: [ ]\n\
     --- --- ---\n\
  2 | - | b | n |\n\
     --- --- ---\n\
  1 | - | - | R |      BLACK's offboard pieces: [ N ]\n\
     --- --- ---\n\
      a   b   c\n\
\n\
 -------------------------------------------------------\n\
\n\
  Game Prediction:  (Player should Win in 1)\n\
\n\
  Player's move > a3a2\n\
\n\
  Computer               Win Condition: 3\n\
 -------------------------------------------------------\n\
\n\
     --- --- ---\n\
  3 | - | B | - |      WHITE's offboard pieces: [ ]\n\
     --- --- ---\n\
  2 | r | b | n |\n\
     --- --- ---\n\
  1 | - | - | R |      BLACK's offboard pieces: [ N ]\n\
     --- --- ---\n\
      a   b   c\n\
\n\
 -------------------------------------------------------\n\
\n\
  Game Prediction:  (Computer should Lose in 0)\n\
\n\
\n\
Excellent! You won!\n"                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             ;

/*************************************************************************
**
** Everything above here must be in every game file
**
**************************************************************************/

/*************************************************************************
**
** Every variable declared here is only used in this file (game-specific)
**
**************************************************************************/

/*************************************************************************
**
** Below is where you put your #define's and your global variables, structs
**
*************************************************************************/

/** Structs *************************************************************/

/* Used mainly for setting up/changing the board */
struct pieceType {
	int id; // Piece ID
	int freq; // Number of occurrences
};


/** Type Definitions ******************************************************/

typedef char* BOARD;
typedef int PIECE;
typedef int MPLAYER;
typedef int CELL;

/** Global Variables *****************************************************/

int numRows = 0;
int numCols = 0;
int winCondition = 0;

/* defines how many of a piece are available; ends in a BLNK */
struct pieceType *initPieces = NULL;

enum pieceID { B_KG,B_QN,B_RK,B_BP,B_KN,B_PN,
	       W_KG,W_QN,W_RK,W_BP,W_KN,W_PN,BLNK };

enum players {BLACK, WHITE};

char piece_strings[13] = {'K','Q','R','B','N','P',
	                  'k','q','r','b','n','p','-'};

/** #defines **************************************************************/

#define INIT_NUMROWS 3
#define INIT_NUMCOLS 3
#define INIT_NUMPTYPES 3
#define INIT_WINCONDITION 3
#define NUM_PIECES 13
#define CELL_LENGTH 8
#define MOVE_LENGTH 5
#define COMMAND_LENGTH 2
#define MENU_OPTION 3
#define OFF 0xFF
#define N_LLEG 2 // length of knight's long leg
#define N_SLEG 1 // length of knight's short leg
#define K_LEG 1 // length of king's stride
#define MAX_PIECES 20

#define min(a,b) ((a < b) ? a : b)
#define max(a,b) ((a > b) ? a : b)

/*************************************************************************/

/* Function prototypes here. */

/* External */
#ifndef MEMWATCH
extern GENERIC_PTR      SafeMalloc ();
extern void             SafeFree ();
#endif
extern POSITION         generic_hash_init(int boardsize, int pieces_array[], int (*vcfg_function_ptr)(int* cfg), int player);
extern POSITION         generic_hash_hash(char *board, int player);
extern char            *generic_hash_unhash(POSITION hash_number, char *empty_board);
extern int              generic_hash_turn (POSITION hashed);
BOOLEAN offBoard(MOVE);
BOOLEAN isPlayer(PIECE,MPLAYER);


/* Local */
PIECE getPiece(MOVE move, BOARD board);
CELL getSource(MOVE move);
CELL getDest(MOVE move);
int getBoardSize();
int getRow(CELL cell);
int getCol(CELL cell);
MPLAYER getPlayer(POSITION pos);
int numOnBoard(PIECE piece, BOARD board);
int sizeOfPieceType(struct pieceType *pt);
MOVE makeMove(PIECE piece, CELL source, CELL dest);

STRING MoveToString( MOVE );

/************************************************************************
**
** NAME:        InitializeGame
**
** DESCRIPTION: Initialize the gDatabase, a global variable. and the other
**              local variables.
**
************************************************************************/

void InitializeGame () {
	int *getPieceArray(struct pieceType *, int), getBoardSize(),i;
	int *pieceArray;
	BOARD board;
	int sizeOfPieceType(struct pieceType *);
	/* Initialize Global Variables */
	if (numRows == 0)
		numRows = INIT_NUMROWS;
	if (numCols == 0)
		numCols = INIT_NUMCOLS;
	if (winCondition == 0)
		winCondition = INIT_WINCONDITION;
	if (initPieces == NULL) {
		initPieces = (struct pieceType *)SafeMalloc(MAX_PIECES*sizeof(struct pieceType));
		/* define initial pieces for board - bishop, queen, knight for each side; */

		initPieces[0].id = B_RK;
		initPieces[0].freq = 1;
		initPieces[1].id = B_KN;
		initPieces[1].freq = 1;
		initPieces[2].id = B_BP;
		initPieces[2].freq = 1;
		initPieces[3].id = W_RK;
		initPieces[3].freq = 1;
		initPieces[4].id = W_KN;
		initPieces[4].freq = 1;
		initPieces[5].id = W_BP;
		initPieces[5].freq = 1;


		//initPieces[0].id = B_QN;
		//initPieces[0].freq = 2;
		//initPieces[1].id = W_QN;
		//initPieces[1].freq = 2;
		initPieces[6].id = BLNK; // end condition
		initPieces[6].freq = getBoardSize();
	}
	if (gNumberOfPositions == 0) {
		pieceArray = getPieceArray(initPieces,sizeOfPieceType(initPieces)+1);
		gNumberOfPositions = generic_hash_init(getBoardSize(), pieceArray,NULL,0);
		////////////////////////////
		////printf("gnumpositions (init) : %u\n",gNumberOfPositions);
		SafeFree(pieceArray);
	}
	if (gInitialPosition == 0) {
		/* define initial board as being all blank */
		board = (BOARD)SafeMalloc(getBoardSize()*sizeof(char));
		for (i = 0; i < getBoardSize(); i++)
			board[i] = BLNK;
		gInitialPosition = generic_hash_hash(board,WHITE); // white goes first
		SafeFree(board);
	}

	gMoveToStringFunPtr = &MoveToString;

	return;
}


/************************************************************************
**
** NAME:        DebugMenu
**
** DESCRIPTION: Menu used to debub internal problems. Does nothing if
**              kDebugMenu == FALSE
**
************************************************************************/
// Doesn't do anything at the moment
void DebugMenu () {
	int option = 1;
	while (option != 0) {
		printf(" 1) Test Generate Moves\n");
		printf(" 0) Exit\n");
		printf("\n  DEBUG> ");
		scanf("%d",&option);
	}
	return;
}


/************************************************************************
**
** NAME:        GameSpecificMenu
**
** DESCRIPTION: Menu used to change game-specific parmeters, such as
**              the side of the board in an nxn Nim board, etc. Does
**              nothing if kGameSpecificMenu == FALSE
**
************************************************************************/

/* Notes: looks like I can reliably hash only up to a board area of 16
   spaces with 2 of 2
 */

void GameSpecificMenu () {
	void PrintPosition (POSITION, STRING, BOOLEAN);
	void resetBoard(), resetPieces(),addPieceToInit(PIECE);
	PIECE stringToPiece(char);
	int *input,i;
	char move[MOVE_LENGTH+1], piece;
	char option[2];
	BOOLEAN hadInitialPieces(MPLAYER);

	input = (int *) malloc(1*sizeof(int));
	while(TRUE) {
		PrintPosition(gInitialPosition,"GameMenu",FALSE);
		printf("  Game Specific Options:\n");
		printf("    Current number of positions: "POSITION_FORMAT "\n",gNumberOfPositions);
		printf("    *note* this should be <= ~2,000,000 positions\n\n");
		printf("\tr)\t (R)ows in board       -     [%d] rows\n",numRows);
		printf("\tc)\t (C)olumns in board    -     [%d] cols\n",numCols);
		printf("\tg)\t (G)ame win condition  -     [%d] in a row\n",winCondition);
		printf("\n");
		printf("\ti)\t (I)nitial board\n");
		printf("\th)\t W(h)ite pieceset\n");
		printf("\tb)\t B(l)ack pieceset\n");
		printf("\n");
		printf("\tb)\t (B)ack\n\n");
		printf("  Select an option: ");
		scanf("%s",option);
		if (!strcmp(option,"b")) {
			SafeFree(input);
			return;
		}
		if (!strcmp(option,"r")) {
			printf("  Input new board height > ");
			scanf("%d",input);
			if(input[0] > 0 && input[0] <= 4)
				numRows = input[0];
			else{
				printf("ERROR: row size must be between one to four\n");
				HitAnyKeyToContinue();
			}
			resetBoard();
		} else if (!strcmp(option,"c")) {
			printf("  Input new board width > ");
			scanf("%d",input);
			if(input[0] > 0 && input[0] <= 4)
				numCols = input[0];
			else{
				printf("ERROR: col size must be between one to four\n");
				HitAnyKeyToContinue();
			}
			resetBoard();
		} else if (!strcmp(option,"g")) {
			printf("  Input new win condition > ");
			scanf("%d",input);
			int maxRowCol = numCols;
			if(numRows > numCols)
				maxRowCol = numRows;
			if(input[0] > 0 && input[0] <= maxRowCol)
				winCondition = input[0];
			else{
				printf("ERROR: winningCondition must be between one to the maximum of rows and columns.\n");
				HitAnyKeyToContinue();
			}
		} else if (!strcmp(option,"i")) {
			while(TRUE) {
				printf("  Make a move (end with 'done') > ");
				scanf("%s",move);
				if (!strcmp(move,"done"))
					break;
				if (ValidTextInput(move)) {
					gInitialPosition = DoMove(gInitialPosition,ConvertTextInputToMove(move));
					PrintPosition(gInitialPosition,"Game Menu",FALSE);
				}
			}
		} else if (!strcmp(option,"h")) {
			printf("  Input a line of WHITE pieces (q,b,n,k,r)\n");
			printf("  i.e. '> q b q' would correspond to starting with two queens and a bishop\n\n");
			printf("  > ");
			i = 0;
			getchar(); // Account for the previous input from stdin
			// Hmmm will eventually have to check string to make sure something legal is added
			if (hadInitialPieces(WHITE))
				resetPieces();
			while((piece = getchar()) != '\n') {
				if (piece == 'k' || piece == 'q' || piece == 'b' ||
				    piece == 'p' || piece == 'n' || piece == 'r') {
					addPieceToInit(stringToPiece(piece));
				}
				resetBoard();
			}
		} else if (!strcmp(option,"l")) {
			printf("  Input a line of BLACK pieces (Q,B,N,K,R))\n");
			printf("  i.e. '> Q B Q' would correspond to starting with two queens and a bishop\n\n");
			printf("  > ");
			i = 0;
			getchar(); // Account for the previous input from stdin
			// Hmmm will eventually have to check string to make sure something legal is added
			if (hadInitialPieces(BLACK))
				resetPieces();
			while((piece = getchar()) != '\n') {
				if (piece == 'K' || piece == 'Q' || piece == 'B' ||
				    piece == 'P' || piece == 'N' || piece == 'R') {
					addPieceToInit(stringToPiece(piece));
				}
				resetBoard();
			}
		} else {
			printf("\n > INVALID OPTION, try to get it right this time please.\n");
		}
	}
	SafeFree(input);
	return;
}


/************************************************************************
**
** NAME:        SetTclCGameSpecificOptions
**
** DESCRIPTION: Set the C game-specific options (called from Tcl)
**              Ignore if you don't care about Tcl for now.
**
************************************************************************/

void SetTclCf (int options[]) {
	return;
}


/************************************************************************
**
** NAME:        DoMove
**
** DESCRIPTION: Apply the move to the position.
**
** INPUTS:      POSITION thePosition : The old position
**              MOVE     theMove     : The move to apply.
**
** OUTPUTS:     (POSITION) : The position that results after the move.
**
** CALLS:       Hash ()
**              Unhash ()
**	            LIST OTHER CALLS HERE
*************************************************************************/

POSITION DoMove (POSITION pos, MOVE move) {
	BOARD getBoard(POSITION), newBoard;
	MPLAYER getPlayer(POSITION), newPlayer;
	POSITION makePosition(BOARD, MPLAYER),newPos;
	newPlayer = (getPlayer(pos) == WHITE) ? BLACK : WHITE;
	newBoard = getBoard(pos);
	newBoard[getDest(move)] = getPiece(move, newBoard);
	if (!offBoard(move))
		newBoard[getSource(move)] = BLNK;
	newPos = makePosition(newBoard,newPlayer);
	SafeFree(newBoard);
	return newPos;
}


/************************************************************************
**
** NAME:        GetInitialPosition
**
** DESCRIPTION: Ask the user for an initial position for testing. Store
**              it in the space pointed to by initialPosition;
**
** OUTPUTS:     POSITION initialPosition : The position to fill.
**
************************************************************************/

POSITION GetInitialPosition () {
	char command[COMMAND_LENGTH+1];
	char inString[MOVE_LENGTH+1];
	MPLAYER player, getPlayer(POSITION);
	POSITION pos = gInitialPosition, switchPlayer(POSITION);
	while(TRUE) {
		player = getPlayer(pos);
		PrintPosition(pos,(player == WHITE) ? "White" : "Black",FALSE);
		printf(" DEBUG ('do ?' for help) > ");
		scanf("%s %s",command,inString);
		if (!strcmp(inString,"q"))
			break;
		if (!strcmp(inString,"?")) {
			printf("\n\n\n\n");
			printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
			printf("|\n");
			printf("| Commands : \n");
			printf("|   mv [MOVE]               -     Make a move\n");
			printf("|   pl [BLACK:WHITE]        -     Change the player\n");
			printf("|   do q                     -     Quits the menu\n");
			printf("|   do ?                     -     Prints this help box\n");
			printf("|\n");
			printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
			continue;
		}
		if (!strcmp(command,"mv")) {
			if (ValidTextInput(inString)) {
				pos = DoMove(pos,ConvertTextInputToMove(inString));
			} else {
				printf("\nINVALID MOVE\n");
			}
		}
		if (!strcmp(command,"pl")) {
			if (!strcmp(inString,"BLACK"))
				player = BLACK;
			if (!strcmp(inString,"WHITE"))
				player = WHITE;
			pos = switchPlayer(pos);
		}
	}
	return pos;
}


/************************************************************************
**
** NAME:        PrintComputersMove
**
** DESCRIPTION: Nicely format the computers move.
**
** INPUTS:      MOVE    computersMove : The computer's move.
**              STRING  computersName : The computer's name.
**
************************************************************************/

void PrintComputersMove (MOVE computersMove, STRING computersName) {
	void PrintMove();
	printf("  > %s's move : ",computersName);
	PrintMove(computersMove);
	printf("\n");
	return;
}


/************************************************************************
**
** NAME:        Primitive
**
** DESCRIPTION: Return the value of a position if it fulfills certain
**              'primitive' constraints. Some examples of this is having
**              three-in-a-row with Gobblet. Three in a row for the player
**              whose turn it is a win, otherwise its a loss.
**              Otherwise undecided.
**
** INPUTS:      POSITION position : The position to inspect.
**
** OUTPUTS:     (VALUE) an enum which is oneof: (win,lose,tie,undecided)
**
** CALLS:       LIST FUNCTION CALLS
**
**
************************************************************************/

VALUE Primitive (POSITION position) {
	BOARD board, getBoard(POSITION);
	MPLAYER player;
	int i, numInRow(CELL,BOARD);

	board = getBoard(position);
	for (i = 0; i < getBoardSize(); i++) {
		if (numInRow(i,board) >= winCondition && !isPlayer(board[i],player)) {
			SafeFree(board);
			return (gStandardGame ? lose : win);
		}
		if (numInRow(i,board) >= winCondition && isPlayer(board[i],player)) {
			SafeFree(board);
			return (gStandardGame ? win : lose);
		}
	}
	SafeFree(board);
	return undecided;
}


/************************************************************************
**
** NAME:        PrintPosition
**
** DESCRIPTION: Print the position in a pretty format, including the
**              prediction of the game's outcome.
**
** INPUTS:      POSITION position   : The position to pretty print.
**              STRING   playerName : The name of the player.
**              BOOLEAN  usersTurn  : TRUE <==> it's a user's turn.
**
** CALLS:       Unhash()
**              GetPrediction()
**              LIST OTHER CALLS HERE
**
************************************************************************/
void PrintPosition (POSITION position, STRING playerName, BOOLEAN usersTurn) {
	BOARD board, getBoard(POSITION);
	struct pieceType *piecesOffBoard(struct pieceType *, BOARD),*pt = NULL;
	board = getBoard(position);
	int row,col,i;
	BOOLEAN isWhite(PIECE), isBlack(PIECE);
	// Ascertains the pieces off the board
	//for (i = 0; i < sizeOfPieceType(initPieces); i++) {
	pt = piecesOffBoard(initPieces,board);
	//}
	printf("\n  %s",playerName);
	for (i = 0; i < (numCols*5); i++)
		printf(" ");
	printf("Win Condition: %d\n ",winCondition);
	for (i = 0; i < (numCols * 5) + 40; i++) {
		printf("-");
	}
	printf("\n\n     ");
	for (col = 0; col < numCols; col++)
		printf("--- ");
	printf("\n");
	for(row = 0; row < numRows; row++) {
		printf("  %d",numRows-row);
		for (col = 0; col < numCols; col++)
			printf(" | %c",piece_strings[board[row*numCols+col]-'0']);
		printf(" |      ");

		if (row == 0) {
			printf("WHITE's offboard pieces: [ ");
			for (i = 0; i < sizeOfPieceType(pt); i++) {
				if (pt[i].freq == 1 && isWhite(pt[i].id))
					printf("%c ",piece_strings[pt[i].id]);
				if  (pt[i].freq > 1 && isWhite(pt[i].id))
					printf("%cx%d ",piece_strings[pt[i].id],pt[i].freq);
			}
			printf("]");
		} else if (row == numRows-1) {
			printf("BLACK's offboard pieces: [ ");
			for (i = 0; i < sizeOfPieceType(pt); i++) {
				if (pt[i].freq == 1 && isBlack(pt[i].id))
					printf("%c ",piece_strings[pt[i].id]);
				if  (pt[i].freq > 1 && isBlack(pt[i].id))
					printf("%cx%d ",piece_strings[pt[i].id],pt[i].freq);
			}
			printf("]");
		}
		printf("\n     ");
		for (col = 0; col < numCols; col++)
			printf("--- ");
		printf("\n");
	}
	printf("     ");
	for(i = 0; i < numCols; i++) {
		printf(" %c  ",'a'+i);
	}
	printf("\n\n ");
	for (i = 0; i < (numCols * 5) + 40; i++) {
		printf("-");
	}
	if (strcmp(playerName,"GameMenu")) {
		printf("\n\n  Game Prediction:  %s\n\n",
		       GetPrediction(position,playerName,usersTurn));
	} else {
		printf("\n\n");
	}
	SafeFree(board);
	SafeFree(pt);
	return;
}

/************************************************************************
**
** NAME:        GenerateMoves
**
** DESCRIPTION: Create a linked list of every move that can be reached
**              from this position. Return a pointer to the head of the
**              linked list.
**
** INPUTS:      POSITION position : The position to branch off of.
**
** OUTPUTS:     (MOVELIST *), a pointer that points to the first item
**              in the linked list of moves that can be generated.
**
** CALLS:       GENERIC_PTR SafeSafeMalloc(int)
**              LIST OTHER CALLS HERE
**
************************************************************************/

MOVELIST *GenerateMoves (POSITION position) {
	BOARD getBoard(POSITION), board;
	MPLAYER player, getPlayer(POSITION);
	MOVELIST *genPlacingMoves(BOARD,MOVELIST *,MPLAYER);
	MOVELIST *genBishopMoves(BOARD,MPLAYER,CELL,MOVELIST *);
	MOVELIST *genQueenMoves(BOARD,MPLAYER,CELL,MOVELIST *);
	MOVELIST *genKnightMoves(BOARD,MPLAYER,CELL,MOVELIST *);
	MOVELIST *genRookMoves(BOARD,MPLAYER,CELL,MOVELIST *);
	MOVELIST *genPawnMoves(BOARD,MPLAYER,CELL,MOVELIST *);
	MOVELIST *genKingMoves(BOARD,MPLAYER,CELL,MOVELIST *);
	MOVELIST *head = NULL;

	int i;
	board = getBoard(position);
	player = getPlayer(position);

	/* Generate 'MOVE' moves */
	for (i = 0; i < getBoardSize(); i++) {
		if (isPlayer(board[i],player)) {
			switch(board[i]) {
			case (B_BP): case (W_BP):
				head = genBishopMoves(board,player,i,head);
				break;
			case (B_QN): case (W_QN):
				head = genQueenMoves(board,player,i,head);
				break;
			case (B_RK): case (W_RK):
				head = genRookMoves(board,player,i,head);
				break;
			case (B_PN): case (W_PN):
				head = genPawnMoves(board,player,i,head);
				break;
			case (B_KG): case (W_KG):
				head = genKingMoves(board,player,i,head);
				break;
			case (B_KN): case (W_KN):
				head = genKnightMoves(board,player,i,head);
				break;
			}
		}
	}

	/* Generate 'PLACING' moves */
	head = genPlacingMoves(board,head,player);
	SafeFree(board);
	return head;
}


/************************************************************************
**
** NAME:        GetAndPrintPlayersMove
**
** DESCRIPTION: This finds out if the player wanted an undo or abort or not.
**              If so, return Undo or Abort and don't change theMove.
**              Otherwise get the new theMove and fill the pointer up.
**
** INPUTS:      POSITION *thePosition : The position the user is at.
**              MOVE *theMove         : The move to fill with user's move.
**              STRING playerName     : The name of the player whose turn it is
**
** OUTPUTS:     USERINPUT             : Oneof( Undo, Abort, Continue )
**
** CALLS:       ValidMove(MOVE, POSITION)
**              BOOLEAN PrintPossibleMoves(POSITION) ...Always True!
**
************************************************************************/

USERINPUT GetAndPrintPlayersMove (POSITION thePosition, MOVE* theMove, STRING playerName) {
	BOOLEAN ValidMove();
	USERINPUT ret, HandleDefaultTextInput();
	do {
		printf("%8s's move > ",playerName);
		ret = HandleDefaultTextInput(thePosition, theMove, playerName);
		if(ret != Continue)
			return(ret);

	}
	while (TRUE);
	return(Continue); /* this is never reached, but lint is now happy */
}

/************************************************************************
**
** NAME:        ValidTextInput
**
** DESCRIPTION: Return TRUE iff the string input is of the right 'form'.
**              For example, if the user is allowed to select one slot
**              from the numbers 1-9, and the user chooses 0, it's not
**              valid, but anything from 1-9 IS, regardless if the slot
**              is filled or not. Whether the slot is filled is left up
**              to another routine.
**
** INPUTS:      STRING input : The string input the user typed.
**
** OUTPUTS:     BOOLEAN : TRUE if the input is a valid text input.
**
************************************************************************/

BOOLEAN ValidTextInput (STRING input) {
	BOOLEAN isPiece(char),isRow(char),isCol(char);
	// Can be either length 3 (placing piece) or 4 (moving piece)
	BOOLEAN valid = TRUE;

	// Case of placing: i.e. Qa4
	valid &= isPiece(input[0]);
	valid &= isRow(input[1]);
	valid &= isCol(input[2]);
	if (input[3] == '\0')
		return valid;

	// Case of moving: i.e. a2a4
	valid = TRUE;
	valid &= isRow(input[0]);
	valid &= isCol(input[1]);
	valid &= isRow(input[2]);
	valid &= isCol(input[3]);
	return valid && (input[4] == '\0');
}

/************************************************************************
**
** NAME:        ConvertTextInputToMove
**
** DESCRIPTION: Convert the string input to the internal move representation.
**              No checking if the input is valid is needed as it has
**              already been checked!
**
** INPUTS:      STRING input : The string input the user typed.
**
** OUTPUTS:     MOVE : The move corresponding to the user's input.
**
************************************************************************/

MOVE ConvertTextInputToMove (STRING input) {
	/* Move is represented by: lowest CELL_LENGTH bits is the dest,
	   next CELL_LENGTH bits is the source, then the rest corresponds
	   to the piece itself */
	CELL strToCell(STRING);
	int i;
	PIECE piece;
	for (i = 0; i < BLNK; i++)
		if (input[0] == piece_strings[i])
			piece = i;
	// if placing
	if (strlen(input) == 3)
		return makeMove(piece,OFF,strToCell(input+1));
	// else if moving
	return makeMove(OFF,strToCell(input),strToCell(input+2));
}

/************************************************************************
**
** NAME:        PrintMove
**
** DESCRIPTION: Print the move in a nice format.
**
** INPUTS:      MOVE *theMove         : The move to print.
**
************************************************************************/

void PrintMove (MOVE move) {
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
	STRING m = (STRING) SafeMalloc( 8 );
	STRING m2 = (STRING) SafeMalloc( 8 );

	//  sprintf( m, "" );
	//  sprintf( m2, "" );

	PIECE piece;
	piece = move >> 2*CELL_LENGTH;
	if (piece != OFF) {
		sprintf( m, "%c", piece_strings[piece]);
	} else {
		*m = '\0';
	}

	sprintf( m2, "%s", m );

	if (!offBoard(move)) {
		sprintf( m, "%s%c%d", m2, getCol(getSource(move))+'a',
		         numRows - getRow(getSource(move)));
	}

	sprintf( m2, "%s", m );

	sprintf( m, "%s%c%d", m2, getCol(getDest(move))+'a',
	         numRows - getRow(getDest(move)));


	SafeFree( m2 );
	return m;
}


/************************************************************************
**
** NAME:        NumberOfOptions
**
** DESCRIPTION: Calculates and returns the number of option combinations
**				there are with all the game variations you program.
**
** OUTPUTS:     int : the number of option combination there are.
**
************************************************************************/

/** HOW DOES THIS WORK IN OURS? INFINITE? **/

int NumberOfOptions () {
	return 8191; //new bounds are 4 by 4.  This is 2^13-1.
}


/************************************************************************
**
** NAME:        getOption
**
** DESCRIPTION: A hash function to keep track of all the game variants.
**				Should return a different number for each set of
**				variants.
**
** OUTPUTS:     int : the number representation of the options.
**
************************************************************************/

int getOption () {
	int option = gStandardGame;
	option = option << 4;
	option = option | winCondition;
	option = option << 4;
	option = option | numCols;
	option = option << 4;
	option = option | numRows;
	return option;
}


/************************************************************************
**
** NAME:        setOption
**
** DESCRIPTION: The corresponding unhash for the game variants.
**				Should take the input and set all the appropriate
**				variants.
**
** INPUT:     int : the number representation of the options.
**
************************************************************************/

void setOption (int option) {
	int rowIn = option & 0xf;
	int colIn = (option >> 4) & 0xf;
	int winCondIn = (option >> 8) & 0xf;
	int misereIn = (option >> 12) & 0x1;
	int maxRowCol;
	if(rowIn > colIn)
		maxRowCol = rowIn;
	else
		maxRowCol = colIn;
	if(winCondIn > maxRowCol) {
		winCondition = maxRowCol;
		printf("ERROR: Winning condition is impossible, changing to earlier variant.\n");
	}
	numRows = rowIn;
	numCols = colIn;
	winCondition = winCondIn;
	gStandardGame = misereIn;
}


/************************************************************************
*************************************************************************
**         EVERYTHING BELOW THESE LINES IS LOCAL TO THIS FILE
*************************************************************************
************************************************************************/

/** Utility Functions **************************************************/

/* Gets the board length */
int getBoardSize() {
	return numRows * numCols;
}

/* Gets the row from a cell number */
int getRow(CELL cell) {
	return cell / numCols;
}

/* Gets the col from a cell number */

int getCol(CELL cell) {
	return cell % numCols;
}

/** Primitive Helper Functions *****************************************/
BOOLEAN isBlack(PIECE piece) {
	return piece < B_PN + 1;
}

BOOLEAN isWhite(PIECE piece) {
	return piece > B_PN && piece < BLNK;
}

/* Identifies whether the pieces on the two cells are of the same color */
BOOLEAN sameColor(CELL first, CELL second, BOARD board) {
	return (isWhite(board[first]) && isWhite(board[second])) ||
	       (isBlack(board[first]) && isBlack(board[second]));
}

/** return the number of pieces consecutively adjacent including current piece
 *  vertically, horizonally, or diagonally
 */
int numInRow(CELL start, BOARD board) {
	int numUp=0, numDown=0, numLeft=0, numRight=0, numUL=0, numDR=0, numUR=0, numDL=0;
	int i=0, j=0;
	int row = (start / numCols) + 1; // rows start from 1
	int col = (start % numCols) + 1; // cols start from 1
	BOOLEAN sameColor(CELL,CELL,BOARD);
	if (board[start] == BLNK)
		return 0;
	else {
		for(i = 1; col + i <= numCols; i++) {
			if (sameColor(start, start+i, board))
				numRight++;
			else
				break;
		}
		for(i = 1; col - i > 0; i++) {
			if (sameColor(start, start-i, board))
				numLeft++;
			else
				break;
		}
		for(i = 1; row + i <= numRows; i++) {
			if (sameColor(start, start + numCols*i, board))
				numDown++;
			else
				break;
		}
		for(i = 1; row - i > 0; i++) {
			if (sameColor(start, start - numCols*i, board))
				numUp++;
			else
				break;
		}
		for (i=1; row+i <= numRows && col+i <= numCols; i++) {
			if (sameColor(start, start+numCols*i+i, board))
				numDR++;
			else
				break;
		}
		for (i=1; row-i > 0 && col-i > 0; i++) {
			if (sameColor(start, start-numCols*i-i, board))
				numUL++;
			else
				break;
		}
		for (i=1; row+i <= numRows && col-i > 0; i++) {
			if (sameColor(start, start+numCols*i-i, board))
				numDL++;
			else
				break;
		}
		for (i=1; row-i > 0 && col+i <= numCols; i++) {
			if (sameColor(start, start-numCols*i+i, board))
				numUR++;
			else
				break;
		}
		i = max (numRight+numLeft, numDown+numUp);
		j = max (numDR+numUL, numDL+numUR);
		return 1+max(i, j);
	}
	return 0;
}

/** GetInitialPosition helper functions ********************************/

/* Returns the same board with the player switched */
POSITION switchPlayer(POSITION pos) {
	POSITION makePosition(BOARD,MPLAYER),newPos;
	BOARD board, getBoard(POSITION);
	board = getBoard(pos);
	newPos = makePosition(board,(getPlayer(pos) == WHITE) ? BLACK : WHITE);
	SafeFree(board);
	return newPos;
}

/* Game Specific Menu helper functions *********************************/
void resetBoard() {
	int *pieceArray, *getPieceArray(struct pieceType *,int), getBoardSize(),i;
	int sizeOfPieceType(struct pieceType *);
	BOARD board;
	board = (BOARD)SafeMalloc(getBoardSize()*sizeof(char));
	for (i=0; i < getBoardSize(); i++)
		board[i] = BLNK;
	pieceArray = getPieceArray(initPieces,sizeOfPieceType(initPieces)+1);
	gNumberOfPositions = generic_hash_init(getBoardSize(),pieceArray,NULL,0);

	gInitialPosition = generic_hash_hash(board,WHITE);
	SafeFree(board);
}

/* Zeros out the initial starting pieces */
void resetPieces() {
	initPieces[0].id = BLNK;
	initPieces[0].freq = 0;
	return;
}

/* Adds another piece to the startup */
void addPieceToInit(PIECE piece) {
	int i;
	for (i = 0; i < sizeOfPieceType(initPieces); i++) {
		if (initPieces[i].id == piece) {
			initPieces[i].freq = initPieces[i].freq + 1;
			return;
		}
	}
	i = sizeOfPieceType(initPieces);
	initPieces[i].id = piece;
	initPieces[i].freq = 1;
	initPieces[i+1].id = BLNK;
	initPieces[i+1].freq = i;
	return;
}

/* Tests whether player has already specified an initial pieceset */
BOOLEAN hadInitialPieces(MPLAYER player) {
	int i;

	for (i = 0; i < sizeOfPieceType(initPieces); i++) {
		if (isPlayer(initPieces[i].id,player))
			return TRUE;
	}
	return FALSE;
}

PIECE stringToPiece(char piece) {
	int i;
	for (i = 0; i < BLNK; i++) {
		if (piece_strings[i] == piece)
			return i;
	}
	return BLNK;
}

/** PRINT POSITION helper functions ************************************/

struct pieceType *piecesOffBoard(struct pieceType *pieces, BOARD board) {
	int getBoardSize(), sizeOfPieceType(struct pieceType *),i;
	struct pieceType *newPT;
	newPT = (struct pieceType *)SafeMalloc((sizeOfPieceType(pieces)+1) * sizeof(struct pieceType));
	for (i = 0; i < sizeOfPieceType(pieces)+1; i++) {
		newPT[i].id = pieces[i].id;
		newPT[i].freq = pieces[i].freq - numOnBoard(pieces[i].id,board);
	}
	return newPT;
}

/* Counts the number of occurrences of specified piece on the board */
int numOnBoard(PIECE piece, BOARD board) {
	int i, count=0;
	for (i = 0; i < getBoardSize(); i++)
		if (board[i] == piece)
			count++;
	return count;
}

/* Returns the size of a struct pieceType array (assumes ends with a BLNK)*/
/* DOES NOT INCLUDE BLNK */
int sizeOfPieceType(struct pieceType *pt) {
	int i = 0;
	while(pt[i].id != BLNK) {
		i++;
	}
	return i;
}

/** Valid Text Input Helpers *******************************************/
BOOLEAN isPiece(char c) {
	int i;
	BOOLEAN valid = FALSE;
	for (i = 0; i < sizeOfPieceType(initPieces); i++)
		if (c == piece_strings[initPieces[i].id])
			valid = TRUE;
	return valid;
}

BOOLEAN isRow(char c) {
	return (c >= 'a' && c < 'a' + numRows);
}

BOOLEAN isCol(char c) {
	return (c >= '0' && c <= '9'); //places upper bound on numRows
}

/** ConvertTextInputToMove Helper Functions ****************************/

/* Converts a string of the form 'a5', 'b6',etc to cell on board */
CELL strToCell(STRING str) {
	return (str[0] - 'a') + numCols*(numRows-(str[1]-'0'));
}

/** GenerateMoves Helper Functions *************************************/

/* These set of functions calculate numbers to determine the boundaries */
// *NOTE* the BOARD is inverse from what is expected - as in it goes
//  0 -> end starting from **top left**
int getSpaceLeft(CELL cell) {
	return getCol(cell);
}
int getSpaceRight(CELL cell) {
	return numCols - (getCol(cell) + 1);
}
int getSpaceUp(CELL cell) {
	return getRow(cell);
}
int getSpaceDown(CELL cell) {
	return numRows - (getRow(cell) + 1);
}

/* Determines if specified piece is one of player's */
BOOLEAN isPlayer(PIECE piece, MPLAYER player) {
	return (isWhite(piece) && player == WHITE) ||
	       (isBlack(piece) && player == BLACK);
}

/* Tests to see if given cell in board has an opponent's piece */
BOOLEAN isOpponent(CELL cell, BOARD board, MPLAYER player) {
	return isPlayer(board[cell],(player == WHITE) ? BLACK : WHITE);
}

/* Tests to see whether the given board is occupied by player's piece */
BOOLEAN isAllied(CELL cell, BOARD board, MPLAYER player) {
	return isPlayer(board[cell],player);
}


/* Generates all available placing moves (as in, placing an offboard
   piece onto the board) */
MOVELIST *genPlacingMoves(BOARD board, MOVELIST *head,MPLAYER player) {
	int i,j;

	struct pieceType *pt;
	pt = piecesOffBoard(initPieces,board);
	for (i = 0; i < getBoardSize(); i++)
		if (board[i] == BLNK)
			for (j = 0; j < sizeOfPieceType(pt); j++) {
				if (isPlayer(pt[j].id,player) && pt[j].freq > 0)
					head = CreateMovelistNode(makeMove(pt[j].id,OFF,i),head);
			}
	SafeFree(pt);
	return head;
}

/* Generates all available bishop moves given a specific board and cell - 4 directions*/
MOVELIST *genBishopMoves(BOARD board, MPLAYER player, CELL cell, MOVELIST *head) {
	int i;
	int spaceLeft,spaceRight,spaceUp,spaceDown;
	PIECE piece;
	CELL test;
	spaceLeft = getSpaceLeft(cell);
	spaceRight = getSpaceRight(cell);
	spaceUp = getSpaceUp(cell);
	spaceDown = getSpaceDown(cell);
	piece = (player == BLACK) ? B_BP : W_BP;
	/* test northeasterly direction */
	for (i = 1; i <= spaceUp && i <= spaceRight; i++) {
		test = cell - i*numCols + i;
		if (isAllied(test,board,player))
			break;
		head = CreateMovelistNode(makeMove(piece,cell,test),head);
		if (isOpponent(test,board,player))
			break;
	}
	/* test southeasterly direction */
	for (i = 1; i <= spaceRight && i <= spaceDown; i++) {
		test = cell + i*numCols + i;
		if (isAllied(test,board,player))
			break;
		head = CreateMovelistNode(makeMove(piece,cell,test),head);
		if (isOpponent(test,board,player))
			break;
	}
	/* test southwesterly direction */
	for (i = 1; i <= spaceDown && i <= spaceLeft; i++) {
		test = cell + i*numCols - i;
		if (isAllied(test,board,player))
			break;
		head = CreateMovelistNode(makeMove(piece,cell,test),head);
		if (isOpponent(test,board,player))
			break;
	}
	/* test northwesterly direction */
	for (i = 1; i <= spaceUp && i <= spaceLeft; i++) {
		test = cell - i*numCols - i;
		if (isAllied(test,board,player))
			break;
		head = CreateMovelistNode(makeMove(piece,cell,test),head);
		if (isOpponent(test,board,player))
			break;
	}
	return head;
}

/* Generates the Knight moves - 8 max */
MOVELIST *genKnightMoves(BOARD board, MPLAYER player, CELL cell, MOVELIST *head) {
	int spaceLeft,spaceRight,spaceUp,spaceDown;
	PIECE piece;
	CELL test;
	spaceLeft = getSpaceLeft(cell);
	spaceRight = getSpaceRight(cell);
	spaceUp = getSpaceUp(cell);
	spaceDown = getSpaceDown(cell);

	//////////////////////////////

	//  printf("\nSPACE left: %d right: %d up: %d down: %d\n",spaceLeft,spaceRight,spaceUp,spaceDown);
	//printf("numCols: %d numRows: %d ROW: %d COL: %d\n",numCols,numRows,getRow(cell),getCol(cell));

	/////////////////////////////

	piece = (player == BLACK) ? B_KN : W_KN;
	test = cell - N_LLEG*numCols + N_SLEG;
	if (spaceUp >= N_LLEG && spaceRight >= N_SLEG && !isAllied(test,board,player))
		head = CreateMovelistNode(makeMove(piece,cell,test),head);
	test = cell - N_SLEG*numCols + N_LLEG;
	if (spaceUp >= N_SLEG && spaceRight >= N_LLEG && !isAllied(test,board,player))
		head = CreateMovelistNode(makeMove(piece,cell,test),head);
	test = cell + N_SLEG*numCols + N_LLEG;
	if (spaceDown >= N_SLEG && spaceRight >= N_LLEG && !isAllied(test,board,player))
		head = CreateMovelistNode(makeMove(piece,cell,test),head);
	test = cell + N_LLEG*numCols + N_SLEG;
	if (spaceDown >= N_LLEG && spaceRight >= N_SLEG && !isAllied(test,board,player))
		head = CreateMovelistNode(makeMove(piece,cell,test),head);
	test = cell + N_LLEG*numCols - N_SLEG;
	if (spaceDown >= N_LLEG && spaceLeft >= N_SLEG && !isAllied(test,board,player))
		head = CreateMovelistNode(makeMove(piece,cell,test),head);
	test = cell + N_SLEG*numCols - N_LLEG;
	if (spaceDown >= N_SLEG && spaceLeft >= N_LLEG && !isAllied(test,board,player))
		head = CreateMovelistNode(makeMove(piece,cell,test),head);
	test = cell - N_SLEG*numCols - N_LLEG;
	if (spaceUp >= N_SLEG && spaceLeft >= N_LLEG && !isAllied(test,board,player))
		head = CreateMovelistNode(makeMove(piece,cell,test),head);
	test = cell - N_LLEG*numCols - N_SLEG;
	if (spaceUp >= N_LLEG && spaceLeft >= N_SLEG && !isAllied(test,board,player))
		head = CreateMovelistNode(makeMove(piece,cell,test),head);
	return head;
}
/* Generates the King moves - 8 max */
MOVELIST *genKingMoves(BOARD board, MPLAYER player, CELL cell, MOVELIST *head) {
	int spaceLeft,spaceRight,spaceUp,spaceDown;
	PIECE piece;
	CELL test;
	spaceLeft = getSpaceLeft(cell);
	spaceRight = getSpaceRight(cell);
	spaceUp = getSpaceUp(cell);
	spaceDown = getSpaceDown(cell);
	piece = (player == BLACK) ? B_KG : W_KG;
	test = cell - numCols;
	if (spaceUp >= K_LEG && !isAllied(test,board,player))
		head = CreateMovelistNode(makeMove(piece,cell,test),head);
	test = cell-numCols+K_LEG;
	if (spaceUp >= K_LEG && spaceRight >= K_LEG && !isAllied(test,board,player))
		head = CreateMovelistNode(makeMove(piece,cell,test),head);
	test = cell + K_LEG;
	if (spaceRight >= K_LEG && !isAllied(test,board,player))
		head = CreateMovelistNode(makeMove(piece,cell,test),head);
	test = cell + numCols + K_LEG;
	if (spaceDown >= K_LEG && spaceRight >= K_LEG && !isAllied(test,board,player))
		head = CreateMovelistNode(makeMove(piece,cell,test),head);
	test = cell + numCols;
	if (spaceDown >= K_LEG && !isAllied(test,board,player))
		head = CreateMovelistNode(makeMove(piece,cell,test),head);
	test = cell + numCols - K_LEG;
	if (spaceDown >= K_LEG && spaceLeft >= K_LEG && !isAllied(test,board,player))
		head = CreateMovelistNode(makeMove(piece,cell,test),head);
	test = cell - K_LEG;
	if (spaceLeft >= K_LEG && !isAllied(test,board,player))
		head = CreateMovelistNode(makeMove(piece,cell,test),head);
	test = cell - numCols - K_LEG;
	if (spaceUp >= K_LEG && spaceLeft >= K_LEG && !isAllied(test,board,player))
		head = CreateMovelistNode(makeMove(piece,cell,test),head);
	//  printf("\nEXPECTED: 'K' RESULT: %c\n",piece_strings[getPiece(makeMove(piece,cell,test))]);
	return head;
}
/* Generates the Rook moves - 4 directions */
MOVELIST *genRookMoves(BOARD board, MPLAYER player, CELL cell, MOVELIST *head) {
	int spaceLeft,spaceRight,spaceUp,spaceDown;
	int i;
	PIECE piece;
	CELL test;
	spaceLeft = getSpaceLeft(cell);
	spaceRight = getSpaceRight(cell);
	spaceUp = getSpaceUp(cell);
	spaceDown = getSpaceDown(cell);
	piece = (player == BLACK) ? B_RK : W_RK;

	/* Test up */
	for (i = 1; i <= spaceUp; i++) {
		test = cell - i*numCols;
		if (isAllied(test,board,player))
			break;
		head = CreateMovelistNode(makeMove(piece,cell,test),head);
		if (isOpponent(test,board,player))
			break;
	}
	/* Test to the right */
	for (i = 1; i <= spaceRight; i++) {
		test = cell + i;
		if (isAllied(test,board,player))
			break;
		head = CreateMovelistNode(makeMove(piece,cell,test),head);
		if (isOpponent(test,board,player))
			break;
	}
	/* Test down */
	for (i = 1; i <= spaceDown; i++) {
		test = cell + i*numCols;
		if (isAllied(test,board,player))
			break;
		head = CreateMovelistNode(makeMove(piece,cell,test),head);
		if (isOpponent(test,board,player))
			break;
	}
	/* Test to the left */
	for (i = 1; i <= spaceLeft; i++) {
		test = cell - i;
		if (isAllied(test,board,player))
			break;
		head = CreateMovelistNode(makeMove(piece,cell,test),head);
		if (isOpponent(test,board,player))
			break;
	}
	return head;
}
/* Generates the Queen moves - basically the rook and bishop moves combined */
MOVELIST *genQueenMoves(BOARD board, MPLAYER player, CELL cell, MOVELIST *head) {
	int spaceLeft,spaceRight,spaceUp,spaceDown;
	int i;
	PIECE piece;
	CELL test;
	spaceLeft = getSpaceLeft(cell);
	spaceRight = getSpaceRight(cell);
	spaceUp = getSpaceUp(cell);
	spaceDown = getSpaceDown(cell);
	piece = (player == BLACK) ? B_QN : W_QN;
	for (i = 1; i <= spaceUp; i++) {
		test = cell - i*numCols;
		if (isAllied(test,board,player))
			break;
		head = CreateMovelistNode(makeMove(piece,cell,test),head);
		if (isOpponent(test,board,player))
			break;
	}
	/* Test to the right */
	for (i = 1; i <= spaceRight; i++) {
		test = cell + i;
		if (isAllied(test,board,player))
			break;
		head = CreateMovelistNode(makeMove(piece,cell,test),head);
		if (isOpponent(test,board,player))
			break;
	}
	/* Test down */
	for (i = 1; i <= spaceDown; i++) {
		test = cell + i*numCols;
		if (isAllied(test,board,player))
			break;
		head = CreateMovelistNode(makeMove(piece,cell,test),head);
		if (isOpponent(test,board,player))
			break;
	}
	/* Test to the left */
	for (i = 1; i <= spaceLeft; i++) {
		test = cell - i;
		if (isAllied(test,board,player))
			break;
		head = CreateMovelistNode(makeMove(piece,cell,test),head);
		if (isOpponent(test,board,player))
			break;
	}
	/* test northeasterly direction */
	for (i = 1; i <= spaceUp && i <= spaceRight; i++) {
		test = cell - i*numCols + i;
		if (isAllied(test,board,player))
			break;
		head = CreateMovelistNode(makeMove(piece,cell,test),head);
		if (isOpponent(test,board,player))
			break;
	}
	/* test southeasterly direction */
	for (i = 1; i <= spaceRight && i <= spaceDown; i++) {
		test = cell + i*numCols + i;
		if (isAllied(test,board,player))
			break;
		head = CreateMovelistNode(makeMove(piece,cell,test),head);
		if (isOpponent(test,board,player))
			break;
	}
	/* test southwesterly direction */
	for (i = 1; i <= spaceDown && i <= spaceLeft; i++) {
		test = cell + i*numCols - i;
		if (isAllied(test,board,player))
			break;
		head = CreateMovelistNode(makeMove(piece,cell,test),head);
		if (isOpponent(test,board,player))
			break;
	}
	/* test northwesterly direction */
	for (i = 1; i <= spaceUp && i <= spaceLeft; i++) {
		test = cell - i*numCols - i;
		if (isAllied(test,board,player))
			break;
		head = CreateMovelistNode(makeMove(piece,cell,test),head);
		if (isOpponent(test,board,player))
			break;
	}
	return head;
}
/* Generates the Pawn moves */
MOVELIST *genPawnMoves(BOARD board, MPLAYER player, CELL cell, MOVELIST *head) {
	return head;
}

/** Hash Interaction Functions *****************************************/

/* Gets the Board from the hash position */
BOARD getBoard(POSITION pos) {
	int getBoardSize();
	BOARD newBoard;
	newBoard = (BOARD) SafeMalloc(getBoardSize()*sizeof(char));
	newBoard = generic_hash_unhash(pos,newBoard);
	return newBoard;
}

/* Gets the Player from the hash position */

MPLAYER getPlayer(POSITION pos) { // Note: WHITE is player 1
	return (generic_hash_turn(pos) == 1) ? WHITE : BLACK;
}

POSITION makePosition(BOARD board, MPLAYER player) {
	return generic_hash_hash(board,(player == WHITE) ? 1 : 2);
}

/* Converts the pieceType array into one more palatable to the hash function
 * Inputs are the pieceType array itself and the size of the pieceType Array
 * REQUIRES: BLNK is the last piece in the pieceType array
 */
int *getPieceArray(struct pieceType *types, int size) {
	int *pieceArray, i, numPieces = 0, minblanks, getBoardSize();
	pieceArray = (int *) SafeMalloc ((3*size+1) * sizeof(int));
	for (i = 0; i < size; i++) {
		pieceArray[3*i] = types[i].id;
		if (types[i].id == BLNK) {
			minblanks = getBoardSize() - numPieces;
			pieceArray[3*i+1] = (minblanks > 0) ? minblanks : 0;
			pieceArray[3*i+2] = getBoardSize();
		} else {
			pieceArray[3*i+1] = 0;
			pieceArray[3*i+2] = types[i].freq;
		}
		numPieces += types[i].freq;
	}
	pieceArray[3*size] = -1;
	return pieceArray;
}

/** Move representation helper functions *********************************/

// A MOVE is an int whose lowest 8 bits represent the dest number,
// whose next 8 bits represent the source number, and whose next 4
// bits represent the piece being moved
// NOTE: 0xff for the source represents OFF the board

MOVE makeMove(PIECE piece, CELL source, CELL dest) {
	int move = dest;
	move += source << CELL_LENGTH;
	if (source == OFF) {
		move += piece << 2*CELL_LENGTH;
	} else {
		move += OFF << 2*CELL_LENGTH;
	}
	return move;
}

PIECE getPiece(MOVE move, BOARD board) {
	PIECE piece;
	piece = move >> 2*CELL_LENGTH;
	if (piece != OFF) {
		return piece;
	} else {
		return board[getSource(move)];
	}
}

CELL getSource(MOVE move) {
	return (move >> CELL_LENGTH) & 0xFF;
}

CELL getDest(MOVE move) {
	return move & 0xFF;
}

/* Returns TRUE if we're placing a piece from off the board, else FALSE */
BOOLEAN offBoard(MOVE move) {
	return (getSource(move) == 0xFF);
}

/*************************************************************************/

/** TESTING FUNCTIONS ***************************************************/

/* Generates a random board */

#define NULLPIECE 15 // we know that 15 doesn't correspond to a real piece
#define MAX 50
BOARD generateBoard(struct pieceType *pieces) {
	int i, j, k,pieceCount = 0, getBoardSize();
	char *pieceBank;
	BOARD newBoard;
	srand(time(NULL));
	for (i = 0;; i++) {
		if (pieces[i].id == BLNK) {
			break;
		} else {
			printf("id: %d, %d ",pieces[i].id,i);
		}
	}
	pieceBank = (char *) SafeMalloc(MAX*sizeof(char));
	for (i = 0,k=0;; i++) {
		if (pieces[i].id == BLNK) // loop terminates on BLNK
			break;
		pieceCount += pieces[i].freq;
		for (j = 0; j < pieces[i].freq; j++) {
			pieceBank[k++] =  pieces[i].id;
		}
	}
	printf("Number of pieces to possibly place: %d\n",pieceCount);
	newBoard = (BOARD) SafeMalloc (getBoardSize() * sizeof(char));
	for(i = 0; i < getBoardSize(); i++) {
		j = rand() % (2*pieceCount); // the probability of getting a blank is < 50%
		if (j < pieceCount && pieceBank[j] != NULLPIECE) {
			newBoard[i] = pieceBank[j];
			pieceBank[j] = NULLPIECE;
		} else {
			newBoard[i] = BLNK;
		}
	}
	return newBoard;
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
