/************************************************************************
**
** NAME:        mctoi.c
**
** DESCRIPTION: Chung-Toi
**
** AUTHOR:      Dan Garcia  -  University of California at Berkeley
**              Copyright (C) Dan Garcia, 1995. All rights reserved.
**              Farzad H. Eskafi
**              Erwin A. Vedar
**
** DATE:        2001/10/29
**
** UPDATE HIST:
** 2003/02/18     :The new hash function was added. Proper documentation
**                 was done.  The file is ready to go.
**
** 2001/12/04     :We are done with the first phase!
**
** 2001/12/03     :Finished adding options.  Now there are misere,
**                 rotating in place, turning off one space hops, two
**                 space hops, landing in different orientations, and
**                 trapped is a win or lose options.
**
** 2001/11/26     :Added option to enable/disable rotating in place
**
** 2001/11/1      :Finished writing a modified version of Chung-Toi.
**
** 2001/10/29     :Scrapped old mctoi as mchung.
**                 Restarting, with 6-piece tic-tac-toe
**                 Note that tie is possible in this game
**                 Used a really wasteful gNumberOfPositions
**                 Did InitializeDatabases, DoMove,
**                 GetInitialPosition, PrintComputersMove,
**                 It does have some bugs in it, but it works.
**
**  10-10-01      :The coding part of the program is complete.
**                 We got rid of all the errors and buggs. But
**                 we are not able to run the program.
**
**  10-07-01      :The main modification started to change the
**                 Tic-Tac-Toi to Chung-Toi
*
**
**************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>





#include "gamesman.h"

/***********************************************************************************************************
 ********************************** FROM HERE, hascode stuff ***********************************************
 **********************************************************************************************************/

#include <stdio.h>
#include <math.h>

#define BOARDSIZE 9
/** Function Prototypes **/
int  fact(int );
int  nCr1(int, int );
int  hashC(char *, int );
int  hashCH(char *, int, int );
void unhashC(char *, int, int, int );
void printBoard(char *, int );
int  hashC2(char *);
void replace(char, char, char *);
void unhashC2(char *, int, int, int, int );
void replaceXS(char *, int, char *, int, char *, int );
int countOX(char *board);
STRING unhash(POSITION pos);

static int nCrTable[BOARDSIZE+1][BOARDSIZE+1] = {
	{ 1, 0,  0,  0,   0,   0,  0,  0, 0, 0 },
	{ 1, 1,  0,  0,   0,   0,  0,  0, 0, 0 },
	{ 1, 2,  1,  0,   0,   0,  0,  0, 0, 0 },
	{ 1, 3,  3,  1,   0,   0,  0,  0, 0, 0 },
	{ 1, 4,  6,  4,   1,   0,  0,  0, 0, 0 },
	{ 1, 5, 10, 10,   5,   1,  0,  0, 0, 0 },
	{ 1, 6, 15, 20,  15,   6,  1,  0, 0, 0 },
	{ 1, 7, 21, 35,  35,  21,  7,  1, 0, 0 },
	{ 1, 8, 28, 56,  70,  56, 28,  8, 1, 0 },
	{ 1, 9, 36, 84, 128, 128, 84, 36, 9, 1 }
};



/************************************************************************
**
** NAME:        nCr1
**
** DESCRIPTION: n choose r. Returns a certain value on the ncrTable
**
************************************************************************/
int nCr1 (int n, int r)
{
	return nCrTable[n][r];
}


/************************************************************************
**
** NAME:        hashC
**
** DESCRIPTION: Counts the number of o's on the board and calls hashCH
**
************************************************************************/
int hashC(char *board, int size)
{
	int count = 0;
	int i;
	/* Appearances */

	for (i = 0; i < size; i++)
	{
		if (board[i] == 'o')
		{
			count++;
		}
	}

	return hashCH(board, size, count);
}


/************************************************************************
**
** NAME:        hashCH
**
** DESCRIPTION: Hashes a ttt board with only one kind of piece
**
************************************************************************/
int hashCH(char *board, int sizeOfBoard, int count)
{
	if (sizeOfBoard == 0 || sizeOfBoard == count)
	{
		return 0;
	} else if (board[0] != 'b') {
		board++;

		return ((nCr1( (sizeOfBoard - 1), count)) + hashCH (board, (sizeOfBoard - 1), (count - 1)));
	} else {

		board++;
		return (hashCH (board, (sizeOfBoard - 1), count));
	}
}

/************************************************************************
**
** NAME:        unhashC
**
** DESCRIPTION: unhashes a ttt board with only one kind of piece
**
**
**
************************************************************************/
void unhashC(char *board, int sizeOfBoard, int count, int hashCode)
{
	if (sizeOfBoard == 0) {
		return;
	} else if ( sizeOfBoard == count) {
		board[0] = 'o';
		board++;
		unhashC(board, (sizeOfBoard - 1), (count -1), hashCode);
	} else if ( hashCode < (nCr1((sizeOfBoard - 1), count))) {
		board[0] = 'b';
		board++;
		unhashC(board, (sizeOfBoard -1), count, hashCode);
	} else {
		board[0] = 'o';
		board++;
		unhashC(board, (sizeOfBoard -1), (count -1), hashCode - nCr1 ((sizeOfBoard -1), count));
	}
}


/************************************************************************
**
** NAME:        printBoard
**
** DESCRIPTION: prints the elements in the board. Xs or Os
**
**
**
************************************************************************/
void printBoard(char *board, int size)
{
	int i;
	for (i = 0; i < size; i++) {
		printf("%c", board[i]);
	}
	printf("\n");
}


/************************************************************************
**
** NAME:        hashC2
**
** DESCRIPTION: hashes the ttt board  with two kinds of pieces
**
**
**
************************************************************************/
int  hashC2( char *board) {
	char bo[BOARDSIZE];
	int bon;
	int xBlankSize = countOX(board);
	char bxo[xBlankSize];
	int i, j = 0;
	int bxon;
	int cx;
	int numO = 0;


	memcpy(bo, board, BOARDSIZE);
	replace('x', 'o', bo);

	bon = hashC(bo, BOARDSIZE);

	for (i = 0; i < BOARDSIZE; i++) {
		if (board[i] == 'x') {
			bxo[j] = 'o';
			j++;
		} else if (board [i] == 'o') {
			bxo[j] = 'b';
			j++;
		}
	}



	bxon = hashC(bxo, xBlankSize);

	for (i = 0; i < xBlankSize; i++) {
		if (bxo[i] == 'o') {
			numO++;
		}
	}
	cx = nCr1(xBlankSize, numO );

	return((cx * bon) + bxon);


}

/************************************************************************
**
** NAME:        replace
**
** DESCRIPTION: replace elements in the board with new element.
**
************************************************************************/
void replace(char old, char new, char *board) {
	int i;

	for (i = 0; i <BOARDSIZE; i++) {
		if (board[i] == old) {
			board[i] = new;
		}
	}

}

/************************************************************************
**
** NAME:       countOX
**
** DESCRIPTION: count the number of non-empty cells in the board
**
**
**
************************************************************************/
int countOX(char *board) {
	int i;
	int count = 0;
	for (i = 0; i < BOARDSIZE; i++) {
		if (board[i] == 'x' || board[i] == 'o') {
			count++;
		}
	}
	return count;
}

/************************************************************************
**
** NAME:        unhashC2
**
** DESCRIPTION: unhashes the ttt board with two kinds of pieces
**
**
**
************************************************************************/
void unhashC2(char *board, int sizeOfBoard, int xCount, int oCount, int hashcode) {
	int mul;
	int fulli;
	int subi;
	char fullc[sizeOfBoard];
	char subc[xCount + oCount];

	mul = nCr1((xCount + oCount), xCount);

	fulli = hashcode / mul;

	subi = hashcode % mul;

	unhashC( fullc, sizeOfBoard, (xCount + oCount), fulli);

	unhashC( subc, (xCount + oCount), xCount, subi);

	replaceXS(board, sizeOfBoard, fullc, sizeOfBoard, subc, (xCount + oCount));
}

/************************************************************************
**
** NAME:        replaceXS
**
** DESCRIPTION: replaces Xs with 0s and 0s with blanks
**
**
**
************************************************************************/

void replaceXS(char *board, int bSize, char *full, int fSize, char *sub, int sSize) {
	if (fSize == 0) {
		return;
	} else if (full[0] == 'b') {
		board[0] = 'b';
		board++;
		full++;
		replaceXS(board, (bSize - 1), full, (fSize - 1), sub, sSize);
	} else if (sub[0] == 'b') {
		board[0] = 'o';
		board++;
		full++;
		sub++;
		replaceXS(board, (bSize -1), full, (fSize - 1), sub, (sSize -1));
	} else if (sub[0] == 'o') {
		board[0] = 'x';
		board++;
		full++;
		sub++;
		replaceXS(board, (bSize -1), full, (fSize -1), sub, (sSize - 1));
	} else {
		printf("ERROR: replaceXS: This is not supposed to happen!");
	}
}

/**********************************************************************************************************/
/********************************   END HERE, hascode stuff *****************BOOLEAN gAlignedX      = FALSE ;*/
/**********************************************************************************************************/


# define BOARDSIZE  9           /* 3x3 board */
# define NUMPIECES  6
# define OFFSETSIZE 7

/*
 * Global variables
 */
POSITION gNumberOfPositions  = 269779; /* the last valid number is 269778,
                                          including zero, it's 269779*/
POSITION gInitialPosition    =  0;           /* empty board                    */
POSITION kBadPosition        = -1;           /* This can never be the rep. of a
                                                position                       */
POSITION gMinimalPosition    = 0;

/*static char board[BOARDSIZE];*/
int gCTOffsets[OFFSETSIZE] = {1, 19, 307, 2323, 14419, 54739, 162259};

STRING kAuthorName          = "Dan Garcia, Farzad H. Eskafi, and Erwin A. Vedar";
STRING kGameName            = "Chung-Toi";
BOOLEAN kPartizan            = TRUE;
BOOLEAN kSupportsHeuristic   = TRUE;
BOOLEAN kSupportsSymmetries  = FALSE;
BOOLEAN kSupportsGraphics    = TRUE;
BOOLEAN kDebugMenu           = FALSE;
BOOLEAN kGameSpecificMenu    = TRUE;
BOOLEAN kTieIsPossible       = FALSE;
BOOLEAN kLoopy               = TRUE;         /* Rotating in place
                                              * and jumping back and
                                              * forth */
BOOLEAN kDebugDetermineValue = FALSE;
void*    gGameSpecificTclInit = NULL;

STRING kHelpGraphicInterface =
        "Help strings not initialized.";

STRING kHelpTextInterface    =
        "Help strings not initialized.";

STRING kHelpOnYourTurn =
        "Help strings not initialized.";

STRING kHelpStandardObjective =
        "Help strings not initialized.";

STRING kHelpReverseObjective =
        "Help strings not initialized.";

STRING kHelpTieOccursWhen =   /* Should follow 'A Tie occurs when... */
                            "Help strings not initialized.";

STRING kHelpExample =
        "Help strings not initialized.";

STRING MoveToString(MOVE);

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


typedef enum possibleBoardPieces {
	Blank, Rx, Wx, Rt, Wt
} BlankoxOX;

char *gBlankoxOXString[] = { "--", "Rx", "Wx", "R+", "W+" };

/* Powers of 5 - this is the way I encode the position, as an integer */
int g5Array[] =          { 1, 5, 25, 125, 625, 3125, 15625, 78125, 390625};

/* Option flags */
/* T F T T T F F F  */
BOOLEAN gRotateInPlace = TRUE;
BOOLEAN gStuckAWin     = FALSE;
BOOLEAN gHopOne        = TRUE;
BOOLEAN gHopTwo        = TRUE;
BOOLEAN gRotateOnHop   = TRUE;

/**************************************************************************
**                        prototypes
**************************************************************************/
int       MoveTo          ( MOVE              );
int       MoveFrom        ( MOVE              );
int       MoveOrientation ( MOVE              );
BOOLEAN   RedPiece        ( BlankoxOX         );
BOOLEAN   WhitePiece      ( BlankoxOX         );
BOOLEAN   BlankPiece      ( BlankoxOX         );
BlankoxOX WhoseTurn       ( POSITION          );
BOOLEAN   IsLegalJump     (int, int, BlankoxOX);
BOOLEAN   RedIsStuck      ( POSITION          );
BOOLEAN   WhiteIsStuck    ( POSITION          );

/**********************************/
/* Before converting to TTT */
void printoxOX               (BlankoxOX *              );
void ConvertCTtoTTT          (char *, BlankoxOX *      );
int CreateOrientationBitmask (BlankoxOX *              );
void blankoxOX_init          (BlankoxOX *              );
void printMask               (int                      );

/* After converting to TTT */
void board_init              (char *                   );
int printhash                (char *                   );
void printboard              (char *                   );
char *printunhash            (int, char *             );


/* After hashing */
int RemoveOffsets            (int, int                );
int GetPieceCount            (int                      );
BlankoxOX GetTurn            (int                      );
int GetOrientationBitmask    (int                      );
int GetRawHashcode           (int                      );
void ConvertTTTtoCT          (BlankoxOX *, char *      );
int ConvertToTTTHash         (int                      );
void FixOrientations         (BlankoxOX *, int         );

/* The whole thing */
int HashChungToi             (BlankoxOX *, BlankoxOX   );
void UnHashChungToi          (BlankoxOX *, int         );

void PositionToBlankoxOX     (POSITION, BlankoxOX*     );
POSITION BlankoxOXToPosition (BlankoxOX*, BlankoxOX    );

/* Changing Variants */
void InitializeHelpString    (                         );

/**********************************************/

void InitializeGame(){

	InitializeHelpString();

	gCustomUnhash = unhash;
	gMoveToStringFunPtr = &MoveToString;
}

void FreeGame() {

}

// Set up the help strings based on the current variant being played
void InitializeHelpString() {

	kHelpGraphicInterface =
	        "Farzad and Erwin: Once there's a graphic interface, fill this in.";

	kHelpTextInterface    =
	        "Chung-Toi has two phases to it.  First, pieces are placed on the board until\n\
both players have placed three pieces on the board each. In the second phase,\n\
the pieces may be moved to different slots, rotated or both.  In both phases,\n\
the legend denotes the slot numbering.  Pieces have either x or + orientation,\n\
indicating which direction the piece can hop.\n\n\
In the first phase, to place a piece, locate the slot number you want to move\n\
to and whether you want to place it in the x or + orientation.  Enter that slot\n\
number immediately followed by the orientation. \n\
Example: 7x would place a piece in the bottom left corner in the x orientation.\n\n\
In the second phase, enter a move as the slot you want to move from, immediately\n\
followed by the slot you with to move to, and the orientation. \n\
Example: 75+ would cause a piece at 7 to move to slot 5 and land in + orientation.\n\n\
An in-place rotation would be considered a move from a slot back to the same slot.\n\
Example: 77+ would cause the piece at 7 to rotate to the + orientation."                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          ;

	kHelpOnYourTurn =
	        "Place up to the three pieces on the board. The pieces can hop from \n\
slot to slot, or rotate in place."                                                                                        ;

	kHelpStandardObjective =
	        "To get three of your markers in a row, either horizontally, vertically, or \n\
diagonally. 3-in-a-row WINS."                                                                                                ;

	kHelpReverseObjective =
	        "To force your opponent into getting three of his markers \n\
in a row, either horizontally, vertically, or diagonally. 3-in-a-row LOSES."                                                                              ;

	kHelpTieOccursWhen = /* Should follow 'A Tie occurs when... */
	                     "Players have looped into a stalemate, unable to force a lose on the other player.";

	kHelpExample =
	        "        ( 1 2 3 )           : -- -- --\n\
LEGEND:  ( 4 5 6 )  TOTAL:   : -- -- -- \n\
         ( 7 8 9 )           : -- -- -- \n\n\
Computer's move              :  3x    \n\n\
         ( 1 2 3 )           : -- -- Rx \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : -- -- -- \n\
         ( 7 8 9 )           : -- -- -- \n\n\
     Dan's move [(u)ndo/1-9] : { 2+} \n\n\
         ( 1 2 3 )           : -- W+ Rx \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : -- -- -- \n\
         ( 7 8 9 )           : -- -- -- \n\n\
Computer's move              :  6+    \n\n\
         ( 1 2 3 )           : -- W+ Rx X \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : -- -- R+ \n\
         ( 7 8 9 )           : -- -- -- \n\n\
     Dan's move [(u)ndo/1-9] : { 9x} \n\n\
         ( 1 2 3 )           : -- W+ Rx \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : -- -- R+ \n\
         ( 7 8 9 )           : -- -- Wx \n\n\
Computer's move              :  5+    \n\n\
         ( 1 2 3 )           : -- W+ Rx \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : -- R+ R+ \n\
         ( 7 8 9 )           : -- -- Wx \n\n\
     Dan's move [(u)ndo/1-9] : { 7+ } \n\n\
         ( 1 2 3 )           : -- W+ Rx \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : -- R+ R+ \n\
         ( 7 8 9 )           : W+ -- Wx \n\n\
Computer's move              :  55x    \n\n\
         ( 1 2 3 )           : -- W+ Rx \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : -- Rx R+ \n\
         ( 7 8 9 )           : W+ -- Wx \n\n\
     Dan's move [(u)ndo/1-9] : { 91+ } \n\n\
         ( 1 2 3 )           : Wx W+ Rx \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : -- Rx R+ \n\
         ( 7 8 9 )           : W+ -- -- \n\n\
Computer's move              :  59x    \n\n\
         ( 1 2 3 )           : Wx W+ Rx \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : -- -- R+ \n\
         ( 7 8 9 )           : W+ -- Rx \n\n\
Computer wins. Nice try, Dan."                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          ;

}

/************************************************************************
**
** NAME:        DebugMenu
**
** DESCRIPTION: Menu used to debug internal problems. Does nothing if
**              kDebugMenu == FALSE
**
************************************************************************/

void DebugMenu()
{
	char GetMyChar();

	do {
		printf("\n\t----- Module DEBUGGER for %s -----\n\n", kGameName);

		printf("\tc)\tWrite PPM to s(C)reen\n");
		printf("\ti)\tWrite PPM to f(I)le\n");
		printf("\ts)\tWrite Postscript to (S)creen\n");
		printf("\tf)\tWrite Postscript to (F)ile\n");
		printf("\n\n\tb)\t(B)ack = Return to previous activity.\n");
		printf("\n\nSelect an option: ");

		switch(GetMyChar()) {
		case 'Q': case 'q':
			ExitStageRight();
			break;
		case 'B': case 'b':
			return;
		}
	} while(TRUE);

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
void GameSpecificMenu() {
	char GetMyChar();

	do {
		printf("\n\t----- Game-specific options for %s -----\n\n", kGameName);

		printf("\tS)\t(S)pinning a piece in place toggle from %s to %s\n",
		       gRotateInPlace ? "ON" : "OFF",
		       !gRotateInPlace ? "ON" : "OFF");
		printf("\tO)\t(O)ne space hops toggle from %s to %s\n",
		       gHopOne ? "ON" : "OFF",
		       !gHopOne ? "ON" : "OFF");
		printf("\tT)\t(T)wo space hops toggle from %s to %s\n",
		       gHopTwo ? "ON" : "OFF",
		       !gHopTwo ? "ON" : "OFF");
		printf("\tL)\tAllow (l)anding in different orientation toggle from %s to %s \n",
		       gRotateOnHop ? "ON" : "OFF",
		       !gRotateOnHop ? "ON" : "OFF");
		printf("\tP)\tTrapped (p)layer toggle from %s to %s\n",
		       gStuckAWin ? "WINNER" : "LOSER",
		       !gStuckAWin ? "WINNER" : "LOSER");

		printf("\n\n\tb)\t(B)ack = Return to previous activity.\n");
		printf("\n\nSelect an option: ");


		switch(GetMyChar()) {
		case 'Q': case 'q':
			ExitStageRight();
		case 'H': case 'h':
			HelpMenus();
			break;
		case 'S': case 's':
			gRotateInPlace = !gRotateInPlace;
			break;
		case 'O': case 'o': case '0':
			gHopOne = !gHopOne;
			break;
		case 'T': case 't':
			gHopTwo = !gHopTwo;
			break;
		case 'L': case 'l':
			gRotateOnHop = !gRotateOnHop;
			break;
		case 'P': case 'p':
			gStuckAWin = !gStuckAWin;
			break;
		case 'B': case 'b':
			return;
		default:
			printf("\nSorry, I don't know that option. Try another.\n");
			HitAnyKeyToContinue();
			break;
		}

	} while(TRUE);

}


/************************************************************************
**
** NAME:        SetTclCGameSpecificOptions
**
** DESCRIPTION: Set the C game-specific options (called from Tcl)
**              Ignore if you don't care about Tcl for now.
**
************************************************************************/

void SetTclCGameSpecificOptions(theOptions)
int theOptions[];
{
	/* This part has been added for the eight different options we have*/
	gRotateInPlace =  (BOOLEAN) theOptions[0];
	gHopOne        =  (BOOLEAN) theOptions[1];
	gHopTwo        =  (BOOLEAN) theOptions[2];
	gRotateOnHop   =  (BOOLEAN) theOptions[3];
	gStuckAWin     =  (BOOLEAN) theOptions[4];


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
** CALLS:       PositionToBlankoxOX(POSITION,*BlankoxOX)
**              BlankoxOX WhoseTurn(thePosition), RedPiece(BlankoxOX)
**
************************************************************************/
POSITION DoMove(thePosition, theMove)
POSITION thePosition;
MOVE theMove;
{
	/* local variables */
	int to;
	int from;
	int orientation;
	static BlankoxOX theBlankoxOX[BOARDSIZE];
	BlankoxOX turn, newTurn;


	/*
	 * function calls
	 * The set of function calls, calls PostionToBlankoxOX and
	 * it unhashed the theMove, the helper functions.
	 */
	PositionToBlankoxOX(thePosition,theBlankoxOX);
	to  = MoveTo(theMove);
	from  = MoveFrom(theMove);
	orientation = MoveOrientation(theMove);
	turn = WhoseTurn(thePosition);

	/* Rearranging theBlankoxOX */
	if (from ==  9) {          /* Still placing pieces        */
		if(turn == Rx) {   /* Red's turn                  */
			if(orientation) { /* t orientation               */
				theBlankoxOX[to] = Rt; /* place an Rt piece           */
			} else {   /* x orientation               */
				theBlankoxOX[to] = Rx; /* place an Rx piece           */
			}
		} else {           /* White's turn                */
			if(orientation) { /* t orientation               */
				theBlankoxOX[to] = Wt; /* place a Wt piece            */
			} else {   /* x orientation               */
				theBlankoxOX[to] = Wx; /* place a Wx piece            */
			}
		}
	} else {                   /* Jumping around              */
		if (turn == Rx) {  /* Red's turn                  */
			if (orientation) { /* t orientation               */
				theBlankoxOX[to] = Rt; /* Change piece to Rt          */
			} else {   /* x orientation               */
				theBlankoxOX[to] = Rx; /* Change piece to Rx          */
			}
		} else {           /* White's turn                */
			if (orientation) { /* t orientation               */
				theBlankoxOX[to] = Wt; /* Change piece to Wt          */
			} else {   /* x orientation               */
				theBlankoxOX[to] = Wx; /* Change piece to Wx          */
			}
		}
		if (from != to) {  /* Not rotating in place       */
			theBlankoxOX[from] = Blank; /* Blank out the spot you left */
		}
	}

	/*
	 * Determines the next peron's turn.  If currently, it's red's
	 * turn, nextTurn will be white's turn, otherwise, it's red's turn.
	 */
	newTurn = ((WhoseTurn(thePosition) == Rx) ? Wx : Rx);

	/*
	 * Returning the equivalent position of BlankoxOX
	 */
	return BlankoxOXToPosition(theBlankoxOX, newTurn);
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
POSITION GetInitialPosition()
{
	static BlankoxOX theBlankoxOX[BOARDSIZE];
	BlankoxOX whosTurn;
	signed char c;
	int i;

	blankoxOX_init(theBlankoxOX);
	printf("\n\n\t----- Get Initial Position -----\n");
	printf("\n\tPlease input the position to begin with.\n");
	printf("\tNote that it should be in the following format:\n\n");
	printf("Rx -- Wx\nW+ -- R+            <----- EXAMPLE \n-- -- Rx\n\n");

	i = 0;
	getchar();
	while(i < BOARDSIZE && (c = getchar()) != EOF) {
		if(c == 'r' || c == 'R') {       /* if the first char is R or r  */
			if ((c = getchar()) == 'x' || c == 'X') { /* if the second char is x or X */
				theBlankoxOX[i++] = Rx; /* put Rx into blankoxOX        */
			} else {                 /* otherwise                    */
				theBlankoxOX[i++] = Rt; /* put Rt into blankoxOX        */
			}
		} else if (c == 'w' || c == 'W') { /* if the first char is W or w  */
			if ((c = getchar()) == 'x' || c == 'X') { /* if the second char is x or X */
				theBlankoxOX[i++] = Wx; /* put Wx into blankoxOX        */
			} else {                 /* otherwise                    */
				theBlankoxOX[i++] = Wt; /* put Wt into blankoxOX        */
			}
		} else if (c == '-') {           /* if the first char is -       */
			getchar();               /* get the other dash, skip one */
			theBlankoxOX[i++] = Blank; /* input blank or -- in blanoxOX*/
		} else {                         /* otherwise                    */
			;                        /* do nothing                   */
		}
	}

	getchar();
	printf("\nNow, whose turn is it? [R/W] : ");
	scanf("%c",&c);
	if(c == 'r' || c == 'R') {               /* if it's red's turn           */

		whosTurn = Rx;                   /* assign Rx to whoseTurn       */
	}
	else {                                   /* otherwise                    */

		whosTurn = Wx;                   /* assign Wx to whoseTurn       */
	}

	/*
	 * Returning the equivalent POSITION to BlankoxOX
	 */
	return(BlankoxOXToPosition(theBlankoxOX,whosTurn));
}

/************************************************************************
**
** NAME:        PrintComputersMove
**
** DESCRIPTION: Nicely format the computers move.
**
** INPUTS:      MOVE   *computersMove : The computer's move.
**              STRING  computersName : The computer's name.
**
************************************************************************/

void PrintComputersMove(computersMove,computersName)
MOVE computersMove;
STRING computersName;
{
	int to = MoveTo(computersMove), from = MoveFrom(computersMove),
	    orientation = MoveOrientation(computersMove);
	if (from == 9) {
		printf("%8s's move       :%d%c\n",
		       computersName,to+1, (orientation ? '+' : 'x'));
	} else {
		printf("%8s's move       :%d%d%c",
		       computersName, from+1, to+1, (orientation ? '+' : 'x'));
	}

}

/************************************************************************
**
** NAME:        Primitive
**
** DESCRIPTION: Determines primitive value of a position. A primitive
**              lose is three in a row.  Depending on the options, a
**              stuck position can be a primitive lose or win.
**
** INPUTS:      POSITION position : The position to inspect.
**
** OUTPUTS:     (VALUE) an enum which is oneof: (win,lose,tie,undecided)
**
** CALLS:       BOOLEAN ThreeInARow()
**              BOOLEAN AllFilledIn()
**              PositionToBlankoxOX()
**              RedIsStuck()
**              WhiteIsStuck()
**
************************************************************************/
VALUE Primitive(position)
POSITION position;
{
	BOOLEAN ThreeInARow(), AllFilledIn();
	static BlankoxOX theBlankoxOX[BOARDSIZE];

	PositionToBlankoxOX(position,theBlankoxOX); /* function call      */

	if( ThreeInARow(theBlankoxOX,0,1,2) ||   /*     legend         */
	    ThreeInARow(theBlankoxOX,3,4,5) ||   /*  0  |  1  |  2     */
	    ThreeInARow(theBlankoxOX,6,7,8) ||   /* --------------     */
	    ThreeInARow(theBlankoxOX,0,3,6) ||   /*  3  |  4  |  5     */
	    ThreeInARow(theBlankoxOX,1,4,7) ||   /* --------------     */
	    ThreeInARow(theBlankoxOX,2,5,8) ||   /*  6  |  7  |  8     */
	    ThreeInARow(theBlankoxOX,0,4,8) ||
	    ThreeInARow(theBlankoxOX,2,4,6) ) {
		return(gStandardGame ? lose : win);
	} else if (AllFilledIn(theBlankoxOX)) {
		if (gRotateInPlace) {
			return (undecided); /* Can Always just spin
			                     * in place              */
		} else {
			if (!gHopOne && !gHopTwo) { /* no hopping allowed   */
				return (gStuckAWin ? win : lose); /* no moves possible    */
			} else {             /* At least one type of hop possible */
				if (WhoseTurn(position) == Rx) {
					if (RedIsStuck(position)) {
						return (gStuckAWin ? win : lose);
					} else {
						return (undecided);
					}
				} else {     /* White's turn */
					if (WhiteIsStuck(position)) {
						return (gStuckAWin ? win : lose);
					} else {
						return (undecided);
					}
				}
			}
		}
	} else { /* !AllFilledIn Still placing pieces down */
		return (undecided);
	}
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
** CALLS:       PositionToBlankoxOX()
**              GetValueOfPosition()
**              GetPrediction()
**
************************************************************************/

void PrintPosition(position,playerName,usersTurn)
POSITION position;
STRING playerName;
BOOLEAN usersTurn;
{
	static BlankoxOX theBlankoxOX[BOARDSIZE];

	PositionToBlankoxOX(position,theBlankoxOX);    /* function call */

	printf("\n         ( 1 2 3 )           : %s %s %s\n",
	       gBlankoxOXString[(int)theBlankoxOX[0]],
	       gBlankoxOXString[(int)theBlankoxOX[1]],
	       gBlankoxOXString[(int)theBlankoxOX[2]] );
	printf("LEGEND:  ( 4 5 6 )  TOTAL:   : %s %s %s\t\tTURN:  %s\n",
	       gBlankoxOXString[(int)theBlankoxOX[3]],
	       gBlankoxOXString[(int)theBlankoxOX[4]],
	       gBlankoxOXString[(int)theBlankoxOX[5]],
	       WhoseTurn(position) == Rx ? "RED" : "WHITE");
	printf("         ( 7 8 9 )           : %s %s %s %s\n\n",
	       gBlankoxOXString[(int)theBlankoxOX[6]],
	       gBlankoxOXString[(int)theBlankoxOX[7]],
	       gBlankoxOXString[(int)theBlankoxOX[8]],
	       GetPrediction(position,playerName,usersTurn));


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
** CALLS:       MOVELIST *CreateMovelistNode(MOVE,MOVELIST *)
**              WhoseTurn()
**              Primitive()
**              PositionToBlankoxOX()
**              IsLegalJump()
**
************************************************************************/
MOVELIST *GenerateMoves(position)
POSITION position;
{
	MOVELIST *CreateMovelistNode(), *head = NULL;
	VALUE Primitive();
	BlankoxOX WhoseTurn(), turn;
	int i, j, k, blankCount = 0;
	static int theBlanks[3];
	static BlankoxOX theBlankoxOX[BOARDSIZE];

	if (Primitive(position) == undecided) {

		turn = WhoseTurn(position);
		PositionToBlankoxOX(position, theBlankoxOX);
		for (i = 0; i < BOARDSIZE; i++ ) { /* look for blanks */
			if (theBlankoxOX[i] == Blank) {
				if (++blankCount <= 3) {
					theBlanks[blankCount - 1] = i;
				} else {
					break;     /* still placing
					            * pieces */
				}
			}
		}

		if (blankCount > 3) { /* Still placing pieces down */
			for (j = 0; j < BOARDSIZE; j++) {
				if (theBlankoxOX[j] == Blank) {
					head = CreateMovelistNode(900 + (10 * j), head);
					head = CreateMovelistNode(900 + (10 * j) + 1, head);
				}
			}
		} else { /* Hopping phase */
			if (turn == Rx) {
				for (j = 0; j < BOARDSIZE; j++) {
					if (theBlankoxOX[j] == Rt) {

						if (gRotateInPlace) {
							head = CreateMovelistNode ((100 * j) + (10 * j), head );
						}

						for (k = 0; k < 3; k++) { /* Loop through blanks */
							if (IsLegalJump(j, theBlanks[k], Rt)) {
								if (gRotateOnHop) {
									head = CreateMovelistNode((100 * j) + (10 * theBlanks[k]), head);
								}               head = CreateMovelistNode((100 * j) + (10 * theBlanks[k]) + 1, head);
							}
						}
						continue;

					} else if (theBlankoxOX[j] == Rx) {

						if (gRotateInPlace) {
							head = CreateMovelistNode ((100 * j) + (10 * j) + 1, head );
						}

						for (k = 0; k < 3; k++) { /* Loop through blanks */
							if (IsLegalJump(j, theBlanks[k], Rx)) {
								if (gRotateOnHop) {
									head = CreateMovelistNode((100 * j) + (10 * theBlanks[k]) + 1, head);
								}
								head = CreateMovelistNode((100 * j) + (10 * theBlanks[k]), head);
							}
						}
						continue;
					}
				}
			} else { /* White's Turn */
				for (j = 0; j < BOARDSIZE; j++) {
					if (theBlankoxOX[j] == Wt) {

						if (gRotateInPlace) {
							head = CreateMovelistNode ((100 * j) + (10 * j), head );
						}

						for (k = 0; k < 3; k++) { /* Loop through blanks */
							if (IsLegalJump(j, theBlanks[k], Wt)) {
								if (gRotateOnHop) {
									head = CreateMovelistNode((100 * j) + (10 * theBlanks[k]), head);
								}
								head = CreateMovelistNode((100 * j) + (10 * theBlanks[k]) + 1, head);
							}
						}
						continue;

					} else if (theBlankoxOX[j] == Wx) {

						if (gRotateInPlace) {
							head = CreateMovelistNode ((100 * j) + (10 * j) + 1, head );
						}

						for (k = 0; k < 3; k++) { /* Loop through blanks */
							if (IsLegalJump(j, theBlanks[k], Wx)) {
								if (gRotateOnHop) {
									head = CreateMovelistNode((100 * j) + (10 * theBlanks[k]) + 1, head);
								}
								head = CreateMovelistNode((100 * j) + (10 * theBlanks[k]), head);
							}
						}
						continue;
					}
				}
			}


		}
		return (head);

	} else { /* Primitive(position) != undecided */
		return (NULL);
	}
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

USERINPUT GetAndPrintPlayersMove(thePosition, theMove, playerName)
POSITION thePosition;
MOVE *theMove;
STRING playerName;
{
	/* local variables */
	BOOLEAN ValidMove(), AllFilledIn();
	USERINPUT ret, HandleDefaultTextInput();
	BlankoxOX theBlankoxOX[BOARDSIZE];

	PositionToBlankoxOX(thePosition, theBlankoxOX);
	do {
		if (AllFilledIn(theBlankoxOX)) {
			printf("%8s's move [(u)ndo/[(from) 1-9][(to) 1-9][x,+]] :  ", playerName);
		} else {
			printf("%8s's move [(u)ndo/[(to) 1-9][x,+]] :  ", playerName);
		}
		ret = HandleDefaultTextInput(thePosition, theMove, playerName);
		if(ret != Continue)
			return(ret);

	}
	while (TRUE);
	return(Continue); /* this is never reached, but link is now happy */
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
** OUTPUTS:     BOOLEAN : TRUE iff the input is a valid text input.
**
************************************************************************/

BOOLEAN ValidTextInput(input)
STRING input;
{
	/* local variable */
	int length;
	length = strlen (input);

	if ( length == 2 ) {
		return (
		        ( input[0] <= '9' && input[0] >= '1' ) &&
		        ( input[1] == 'x' || input[1] == 'X' ||
		          input[1] == '+' )
		        );
	}else if ( length == 3 ) {
		return (
		        ( input[0] <= '9' && input[0] >= '1' ) &&
		        ( input[1] <= '9' && input[1] >= '1' ) &&
		        ( input[2] == 'x' || input[2] == 'X' ||
		          input[2] == '+' )
		        );

	}else
		return FALSE;

}

/************************************************************************
**
** NAME:        ConvertTextInputToMove
**
** DESCRIPTION: Convert the string input to the internal move representation.
**
** INPUTS:      STRING input : The string input the user typed.
**
** OUTPUTS:     MOVE : The move corresponding to the user's input.
**
************************************************************************/

MOVE ConvertTextInputToMove(input)
STRING input;
{
	/* local variable */
	MOVE theMove;
	int length;

	length = strlen ( input );          /* caculating the length                */

	if ( length == 2 ) {                /* if still placing pieces on the board */
		theMove = 900 + ( 10 * (input[0] - '0' - 1));
		if ( input[1] == 'x' || input[1] == 'X' )      {
			;                   /* do nothing */
		}else if (input[1] ==  '+' )      {
			theMove += 1;
		}

		/* convert to int                       */
	}else if ( length == 3 ) {          /* if you want to move pieces around    */
		theMove = (100 * (input[0]- '0' - 1)) +
		          (10 *  (input[1]- '0' - 1));

		if ( input[2] == 'x' || input[2] == 'X' ) {
			;                   /* Do nothing */
		}else if (input[2] ==  '+' ) {
			theMove += 1;
		}
	}


	return theMove;                     /* return (int)theMove                  */
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
void PrintMove(theMove)
MOVE theMove;
{
	printf( "%s", MoveToString(theMove) );
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

	if (MoveFrom(theMove) == 9) { /* if placing pieces into the board     */
		sprintf(move, "%d%c", MoveTo(theMove) +1, (MoveOrientation(theMove) ? '+' : 'x'));
	} else {                   /* otherwise                            */
		                   /* The plus 1 is because the user
		                    * thinks it's 1-9, but MOVE is 0-8
		                    */
		sprintf(move, "%d%d%c", MoveFrom       (theMove) + 1,
		        MoveTo         (theMove) + 1,
		        (MoveOrientation(theMove) ? '+' : 'x')
		        );
	}

	return move;
}

/************************************************************************
*************************************************************************
**         EVERYTHING BELOW THESE LINES IS LOCAL TO THIS FILE
*************************************************************************
************************************************************************/

/************************************************************************
**
** NAME:        PositionToBlankoxOX
**
** DESCRIPTION: convert an internal position to that of a BlankoxOX.
**
** INPUTS:      POSITION thePos     : The position input.
**              BlankoxOX *theBlankoxOX:The converted BlankoxOX output array.
**
** CALLS:       BadElse()
**
************************************************************************/

void PositionToBlankoxOX(thePos,theBlankoxOX)
POSITION thePos;
BlankoxOX *theBlankoxOX;
{
	/* local variable */

	UnHashChungToi(theBlankoxOX, thePos); /* function call to */
	                                      /* UnHashChungToi,  */
	                                      /* to change the    */
	                                      /* position to      */
	                                      /* blankoxox        */
}

/************************************************************************
**
** NAME:        BlankoxOXToPosition
**
** DESCRIPTION: convert a BlankoxOX to that of an internal position.
**
** INPUTS:      BlankoxOX *theBlankOx : The converted BlankOX output array.
**              BlankoxOX turn
**
** OUTPUTS:     POSITION: The equivalent position given the BlankoxOX.
**
************************************************************************/
POSITION BlankoxOXToPosition(theBlankoxOX, turn)
BlankoxOX *theBlankoxOX, turn;
{
	/* local variables */
	POSITION position = 0;

	position = HashChungToi(theBlankoxOX, turn ); /*
	                                                 Function call to HashChungToi to
	                                                 change the blankoxOX to position
	                                               */

	return(position);
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
** CALLS:       none
**
************************************************************************/

POSITION GetCanonicalPosition(position)
POSITION position;
{
	POSITION theCanonicalPosition;

	theCanonicalPosition = position;
	return(theCanonicalPosition);
}

/************************************************************************
**
** NAME:        DecodeMove
**
** DESCRIPTION: If this is the move the canonical position would
**              have done, and this is the position we HAVE, how does
**              the move change for our position? We return that new
**              move that's relevant to our position.
**
** INPUTS:      POSITION thePosition : The new position
**              POSITION canPosition : The canonical position
**              MOVE     move        : The move to convert
**
** OUTPUTS:     MOVE                 : The equivalent move for thePosition.
**
** CALLS:       none
**
************************************************************************/

MOVE DecodeMove(thePosition, canPosition, move)
POSITION thePosition, canPosition;
MOVE move;
{
	return move;
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
** CALLS:       GENERIC_PTR SafeMalloc(int)
**
************************************************************************/

POSITION DoSymmetry(position, symmetry)
POSITION position;
int symmetry;
{
	return position;
}

/************************************************************************
**
** NAME:        ThreeInARow
**
** DESCRIPTION: Return TRUE iff there are three-in-a-row.
**
** INPUTS:      BlankoxOX theBlankoxOX[BOARDSIZE] : The BlankOX array.
**              int a,b,c                         : The 3 positions to check.
**
** OUTPUTS:     (BOOLEAN) TRUE iff there are three-in-a-row.
**
** CALLS:       BOOLEAN RedPiece  (theBlankoxOX )
**              BOOLEAN WhitePiece(theBlankoxOX )
************************************************************************/
BOOLEAN ThreeInARow(theBlankoxOX,a,b,c)
BlankoxOX theBlankoxOX[];
int a,b,c;
{
	/* returning true if it could find three pieces in a row, false otherwise */


	return((
	               ( RedPiece( theBlankoxOX[a] )) &&
	               ( RedPiece( theBlankoxOX[b] )) &&
	               ( RedPiece( theBlankoxOX[c] ))
	               )||
	       (
	               ( WhitePiece( theBlankoxOX[a] )) &&
	               ( WhitePiece( theBlankoxOX[b] )) &&
	               ( WhitePiece( theBlankoxOX[c] ))
	       ));
}



/**********************************************************************
**
** NAME:	RedPiece
**
** DESCRIPTION: Helper function checks if the piece is red or not
**
** INTPUTS:     BlankoxOX theBlankoxOX
**
** OUTPUT:      BOOLEAN TRUE || FALSE
**
** CALLS:       none
**
***********************************************************************/
BOOLEAN RedPiece (aBlankoxOX )
BlankoxOX aBlankoxOX;
{
	return ( aBlankoxOX == Rx || aBlankoxOX == Rt );

}

/**********************************************************************
**
** NAME:	WhitePiece
**
** DESCRIPTION: Helper function checks if the piece is white or not
**
** INTPUTS:     BlankoxOX theBlankoxOX
**
** OUTPUT:      BOOLEAN TRUE || FALSE
**
** CALLS:       none
**
***********************************************************************/
BOOLEAN WhitePiece (aBlankoxOX )
BlankoxOX aBlankoxOX;
{
	return ( aBlankoxOX == Wx || aBlankoxOX == Wt );

}


/**********************************************************************
**
** NAME:	BlankPiece
**
** DESCRIPTION: Helper function checks if the piece is blank or not
**
** INTPUTS:     BlankoxOX theBlankoxOX
**
** OUTPUT:      BOOLEAN TRUE || FALSE
**
** CALLS:       none
**
***********************************************************************/
BOOLEAN BlankPiece (aBlankoxOX )
BlankoxOX aBlankoxOX;
{
	return ( aBlankoxOX == Blank );

}



/************************************************************************
**
** NAME:        AllFilledIn
**
** DESCRIPTION: Return TRUE iff all the pieces are down
**
** INPUTS:      BlankOX theBlankOX[BOARDSIZE] : The BlankOX array.
**
** OUTPUTS:     (BOOLEAN) TRUE iff all the blanks are filled in.
**
************************************************************************/

BOOLEAN AllFilledIn(theBlankoxOX)
BlankoxOX theBlankoxOX[];
{
	int i, count = 0;

	for(i = 0; i < BOARDSIZE; i++) {

		if (BlankPiece(theBlankoxOX[i])) {

			count++;
		}


	}

	return(count <= 3); /*THE ONLY THING!*/
}

/************************************************************************
**
** NAME:        WhoseTurn
**
** DESCRIPTION: Helper function will detemine whose turn it is by looking at
**              the position of pieces. If thePosition is greater than
**              5^9, then it's white's turn, else, it's red's turn.
**
** INPUTS:      POSITION thePosition
**
** OUTPUTS:     (BlankoxOX) Rx or Wx
**
************************************************************************/
BlankoxOX WhoseTurn(thePosition)
POSITION thePosition;
{

	return GetTurn(thePosition);

}


/**********************************************************************
**
** NAME:        MoveFrom
**
** DESCRIPTION: Helper function, given a hashed move, gets the FROM slot in
**              theMove. (from,to,orientation)
**
** INPUT:       MOVE theMove
**
** OUTPUTS:     int from
**
** CALLS:       none
**
**********************************************************************/
int MoveFrom (theMove )
MOVE theMove;
{
	/* local variables */
	int from;

	from = theMove / 100;

	return from;
}

/***********************************************************************
**
** NAME:        MoveTo
**
** DESCRIPTION: Helper function, given a hashed move, gets the TO slot in
**              theMove. (from,to,orientation)
**
** INPUT:       MOVE theMove
**
** OUTPUTS:     int to
**
** CALLS:       int MoveFrom( theMove )
**
***********************************************************************/
int MoveTo(theMove )
MOVE theMove;
{
	/* local variables */
	int to;
	int from;

	/* function calls */
	from = MoveFrom( theMove );
	to   = (theMove - ( from * 100 )) / 10;

	return to;
}


/**********************************************************************
**
** NAME:        MoveOrientation
**
** DESCRIPTION: Helper function, given a hashed move, gets the ORIENTATION
**              slot in theMove. (from,to,orientation)
**
** INPUT:       MOVE theMove
**
** OUTPUTS:     int orientation
**
** CALLS:       int MoveFrom( theMove )
**              int MoveTo  ( theMove )
**
**********************************************************************/
int MoveOrientation (theMove )
MOVE theMove;
{
	/* local variables */
	int to;
	int from;
	int orientation;

	/* function calls */
	from        = MoveFrom( theMove );
	to          = MoveTo  ( theMove );
	orientation = theMove - ( (from * 100 ) + ( to * 10 ) );

	return orientation;
}


/*************************************************************************
**
** NAME :       IsLegalJump
**
** DESCRIPTION: Tells whether a jump is legal or not.  A jump is a
**              move from a space on the board onto a different space
**              on the board. ASSUMES AT LEAST ONE JUMP TYPE IS LEGAL.
**
** INPUTS:      int from; slot to move from
**              int to; empty slot to move to
**              int orientation; current orientation of the piece to move
**
** OUTPUTS:     Boolean { TRUE, FALSE }, whether the move is legal or not
**
** CALLS:       none
**
*************************************************************************/

BOOLEAN IsLegalJump (from, to, piece)
int from, to;
BlankoxOX piece;
{
	int orientation;
	if ((piece == Rx) || (piece == Wx)) {
		orientation = 0;
	} else {
		orientation = 1;
	}
	switch (from) { /* space to jump from */
	case 0:
		if (orientation) { /* + wise             */
			if (gHopOne && gHopTwo) {
				return (to == 1 || to == 3 || to == 6 || to ==2);
			} else if (gHopOne) {
				return (to == 1 || to == 3);
			} else { /* gHopTwo            */
				return (to == 2 || to == 6);
			}
		} else { /* x wise             */
			if (gHopOne && gHopTwo) {
				return (to == 4 || to == 8);
			} else if (gHopOne) {
				return (to == 4);
			} else { /* gHopTwo            */
				return (to == 8);
			}
		}
		break;
	case 1:
		if (orientation) { /* + wise             */
			if (gHopOne && gHopTwo) {
				return (to == 0 || to == 2 || to == 4 || to == 7);
			} else if (gHopOne) {
				return (to == 0 || to == 2 || to == 4);
			} else { /* gHopTwo            */
				return (to == 7);
			}
		} else { /* x wise             */
			if (gHopOne) {
				return (to == 3 || to == 5);
			} else { /* gHopTwo            */
				return FALSE;
			}
		}
		break;
	case 2:
		if (orientation) { /* + wise             */
			if (gHopOne && gHopTwo) {
				return (to == 0 || to == 1 || to == 5 || to == 8);
			} else if (gHopOne) {
				return (to == 1 || to == 5);
			} else { /* gHopTwo             */
				return (to == 0 || to == 8);
			}

		} else { /* x wise              */

			if (gHopOne && gHopTwo) {
				return (to == 4 || to == 6);
			} else if (gHopOne) {
				return (to == 4);
			} else { /* gHopTwo             */
				return (to == 6);
			}
		}
		break;
	case 3:
		if (orientation) { /* + wise             */
			if (gHopOne && gHopTwo) {
				return (to == 0 || to == 6 || to == 4 || to == 5);
			} else if (gHopOne) {
				return (to == 0 || to == 6 || to == 4);
			} else { /* gHopTwo            */
				return (to == 5);
			}
		} else { /* x wise             */
			if (gHopOne) {
				return (to == 1 || to == 7);
			} else { /* gHopTwo            */
				return FALSE;
			}
		}
		break;
	case 4:
		if (orientation) { /* + wise             */
			if (gHopOne) {
				return (to ==1 || to == 3 || to == 5 || to == 7);
			} else { /* gHopTwo            */
				return FALSE;
			}
		} else { /* x wise             */
			if (gHopOne) {
				return (to == 0 || to == 2 || to == 6 || to == 8);
			} else { /* gHopTwo            */
				return FALSE;
			}
		}
		break;
	case 5:
		if (orientation) { /* + wise              */
			if (gHopOne && gHopTwo) {
				return (to == 8 || to == 2 || to == 3 || to == 4);
			} else if (gHopOne) {
				return (to == 8 || to == 2 || to == 4);
			} else { /* gHopTwo             */
				return (to == 3);
			}
		} else { /* x wise              */
			if (gHopOne) {
				return (to == 1 || to == 7);
			} else { /* gHopTwo             */
				return FALSE;
			}
		}
		break;
	case 6:
		if (orientation) { /* + wise              */
			if (gHopOne && gHopTwo) {
				return (to == 0 || to == 3 || to == 7 || to == 8 );
			} else if (gHopOne) {
				return (to == 3 || to == 7);
			} else { /* gHopTwo             */
				return (to == 0 || to == 8);
			}
		} else { /* x wise              */
			if (gHopOne && gHopTwo) {
				return (to == 4 || to == 2);
			} else if (gHopOne) {
				return (to == 4);
			} else { /* gHopTwo             */
				return (to == 2);
			}
		}
		break;
	case 7:
		if (orientation) { /* + wise              */
			if (gHopOne && gHopTwo) {
				return (to == 6 || to == 8 || to == 1 || to == 4);
			} else if (gHopOne) {
				return (to == 6 || to == 8 || to == 4);
			} else { /* gHopTwo              */
				return (to == 1);
			}
		} else { /* x wise              */
			if (gHopOne) {
				return (to == 3 || to == 5);
			} else { /* gHopTwo             */
				return FALSE;
			}
		}
		break;
	case 8:
		if (orientation) { /* + wise              */
			if (gHopOne && gHopTwo) {
				return (to == 2 || to == 5 || to == 6 || to == 7);
			} else if (gHopOne) {
				return (to == 5 || to == 7);
			} else { /* gHopTwo             */
				return (to == 2 || to == 6);
			}
		} else { /* x wise              */
			if (gHopOne && gHopTwo) {
				return (to == 4 || to == 0);
			} else if (gHopOne) {
				return (to == 4);
			} else { /* gHopTwo              */
				return (to == 0);
			}
		}
		break;

	}

	//should never reach here
	return FALSE;
}




/**********************************************************************
**
** NAME:        RedIsStuck
**
** DESCRIPTION: Determines whether red can move or not.  Assumes
**              that at least one type of jump and rotations are
**              legal.
**
** INPUTS:      POSITION position
**
** OUTPUTS:     BOOLEAN {TRUE, FALSE}
**
** MODIFIES:    none
**
** CALLS:       PositionToBlankoxOX(), IsLegalJump()
**
**********************************************************************/
BOOLEAN RedIsStuck (position)
POSITION position;
{
	int i, j;
	static BlankoxOX theBlankoxOX[BOARDSIZE];

	PositionToBlankoxOX(position,theBlankoxOX);
	for (i = 0; i < BOARDSIZE; i++ ) {
		if (RedPiece(theBlankoxOX[i])) {
			for (j = 0; j < BOARDSIZE; j++) {
				if (theBlankoxOX[j] == Blank && IsLegalJump(i, j, theBlankoxOX[i]))
					return FALSE;
			}
		}
	}
	return TRUE;
}



/**********************************************************************
**
** NAME: WhiteIsStuck
**
** DESCRIPTION: Determines whether white can move or not.  Assumes
**              that at least one type of jump and rotations are
**              legal.
**
** INPUTS:      POSITION position
**
** OUTPUTS:     BOOLEAN {TRUE, FALSE}
**
** MODIFIES:    none
**
** CALLS:       PositionToBlankoxOX(), IsLegalJump()
**
**********************************************************************/
BOOLEAN WhiteIsStuck (position)
POSITION position;
{
	int i, j;
	static BlankoxOX theBlankoxOX[BOARDSIZE];

	PositionToBlankoxOX(position,theBlankoxOX);
	for (i = 0; i < BOARDSIZE; i++ ) {
		if (WhitePiece(theBlankoxOX[i])) {
			for (j = 0; j < BOARDSIZE; j++) {
				if (theBlankoxOX[j] == Blank && IsLegalJump(i, j, theBlankoxOX[i]))
					return FALSE;
			}
		}
	}
	return TRUE;
}




/*******************************************************************************/
/* Before converting to TTT */

/************************************************************************
**
** NAME:        ConvertCTtoTTT
**
** DESCRIPTION: Convert chung-toi to tic-tac-toe
**
************************************************************************/
void ConvertCTtoTTT (char *dest, BlankoxOX *board) {
	int i;
	for (i = 0; i < BOARDSIZE; i++) {
		if (board[i] == Rx || board[i] == Rt) {
			dest[i] = 'x';
		} else if (board[i] == Wx || board[i] == Wt) {
			dest[i] = 'o';
		} else if (board[i] == Blank) {
			dest[i] = 'b';
		} else {
			printf("Not supposed to get here in ConvertCTtoTTT");
		}
	}
}


/************************************************************************
**
** NAME:        ConvertTTTtoCT
**
** DESCRIPTION: Convert tic-tac-toe to chung-toi
**
************************************************************************/
void ConvertTTTtoCT (BlankoxOX *dest, char *board) {
	int i;
	for (i = 0; i < BOARDSIZE; i++) {
		if (board[i] == 'x') {
			dest[i] = Rx;
		} else if (board[i] == 'o') {
			dest[i] = Wx;
		} else if (board[i] == 'b') {
			dest[i] = Blank;
		} else {
			printf("You should never get here in ConvertTTTtoCT");
		}
	}
}


/************************************************************************
**
** NAME:        printoxOX
**
** DESCRIPTION: Prints the board and the elements in the board
**
************************************************************************/
void printoxOX(BlankoxOX *board) {
	int i;
	for (i = 0; i < BOARDSIZE; i++) {
		printf("%c%c ", gBlankoxOXString[(int) board[i]][0], gBlankoxOXString[(int) board[i]][1] );
	}
	printf("\n");
}


/************************************************************************
**
** NAME:        CreateOrientationBitmask
**
** DESCRIPTION: Gets the bit that represent the orientation (0/1)
**
************************************************************************/
int CreateOrientationBitmask(BlankoxOX *board) {
	int i;
	int count = NUMPIECES;
	int result = 0;
	for (i = 0; i <BOARDSIZE; i++) {
		if (board[i] == Rt || board[i] == Wt ) {
			result = (result << 1) | 0x1;
			count--;
		} else if (board [i] == Rx || board[i] == Wx ) {
			result = (result << 1);
			count--;
		}


	}

	/*result = result << count;*/
	return result;
}


/************************************************************************
**
** NAME:        GetPieceCount
**
** DESCRIPTION: Count the number of pieces on the board
**
************************************************************************/
int GetPieceCount(int hashcode) {
	int i;

	int count = 0;
	for ( i = 0; i < OFFSETSIZE; i++) {
		if (hashcode >= gCTOffsets[i]) {
			count++;
		} else {
			break;
		}
	}

	if (count > NUMPIECES) {
		return NUMPIECES;
	} else {
		return count;
	}
}


/************************************************************************
**
** NAME:        GetTurn
**
** DESCRIPTION: Decides who suppose place the next piece down (red/white)
**
************************************************************************/
BlankoxOX GetTurn(int hashcode) {
	int pieces = GetPieceCount(hashcode);
	int parity = pieces % 2;

	if (parity == 0) {
		if (pieces == NUMPIECES) {
			if (hashcode >= gCTOffsets[NUMPIECES]  ) {
				return Wx;
			} else {
				return Rx;
			}
		} else {
			return Rx;
		}
	} else if (parity == 1) {
		return Wx;
	} else {
		printf("You shouldn't get here in GetTurn");
	}

	//should never reach here
	return Wx;
}


/************************************************************************
**
** NAME:        GetOrientationBitmask
**
** DESCRIPTION: Gets the bitmask of the orientation
**
************************************************************************/
int GetOrientationBitmask(int hashcode) {
	int i;
	int mask = 0;
	int tempHash = 0;
	int count = GetPieceCount (hashcode);


	for (i = 0; i< count; i++) {
		mask = (mask << 1 ) | 0x1;
	}
	tempHash = RemoveOffsets(hashcode, count);

	return (tempHash & mask);
}


/************************************************************************
**
** NAME:        RemoveOffsets
**
** DESCRIPTION: Removes the  Offsets
**
************************************************************************/
int RemoveOffsets(int hashcode, int count) {
	int tempHash;
	if (count == 0) {
		return hashcode;
	} else if (count != NUMPIECES) {
		tempHash = hashcode - gCTOffsets[count - 1];
	} else {
		if (hashcode >= gCTOffsets[NUMPIECES]) {
			tempHash = hashcode - gCTOffsets[NUMPIECES];
		} else {
			tempHash = hashcode - gCTOffsets[count - 1];
		}
	}
	return tempHash;
}


/************************************************************************
**
** NAME:        GetRawHashCode
**
** DESCRIPTION: Calls GetPieceCount and removes the offsets
**
************************************************************************/
int GetRawHashcode(int hashcode) {
	int count = GetPieceCount (hashcode);

	hashcode = RemoveOffsets(hashcode, count);

	return (hashcode >> count);
}


/************************************************************************
**
** NAME:        ConvertToTTTHash
**
** DESCRIPTION:
**
************************************************************************/
int ConvertToTTTHash(int hashcode) {
	int hash;


	hash = GetRawHashcode(hashcode);


	return hash;
}


/************************************************************************
**
** NAME:        HashChungToi
**
** DESCRIPTION: Gets the board and turn, and hashes the board
**
************************************************************************/
int HashChungToi(BlankoxOX *board, BlankoxOX turn) {
	static char temp[BOARDSIZE];
	int hashcode;
	int bitmask;
	int count = 0;
	int i;
	int offsetHashcode = 0;

	board_init(temp);


	for ( i = 0; i < BOARDSIZE; i++) {
		if (board[i] != Blank ) {
			count++;
		}
	}
	ConvertCTtoTTT(temp, board);

	hashcode = hashC2(temp);
	bitmask = CreateOrientationBitmask(board);
	offsetHashcode = hashcode << count;

	offsetHashcode = offsetHashcode | bitmask;

	if ((turn == Wx || turn == Wt) && count == NUMPIECES) {
		offsetHashcode += gCTOffsets[NUMPIECES];
	} else if (count != 0) {
		offsetHashcode += gCTOffsets[count -1];
	}

	return offsetHashcode;
}


/************************************************************************
**
** NAME:         UnHashChungToi
**
** DESCRIPTION:  Gets a hashcode, and unhashes it
**
************************************************************************/
void UnHashChungToi(BlankoxOX *dest, int hashcode) {
	int mask, hash, pieces, Xs, Os;
	static char temp[BOARDSIZE];

	board_init(temp);
	blankoxOX_init(dest);

	mask = GetOrientationBitmask(hashcode);

	hash = ConvertToTTTHash(hashcode);
/*  printf("TTHash for board is %d", hash);
 */
	pieces = GetPieceCount(hashcode);
	if (pieces % 2 == 0) {
		Xs = pieces / 2;
	} else {
		Xs = pieces / 2 + 1;
	}
	Os = pieces - Xs;

	unhashC2(temp, BOARDSIZE, Xs, Os, hash);

	ConvertTTTtoCT(dest, temp);

	FixOrientations(dest, mask);


}


/************************************************************************
**
** NAME:        FixOrientations
**
** DESCRIPTION: By bitmasking and other means, the function fixed the
**              orientation
**
************************************************************************/
void FixOrientations(BlankoxOX *dest, int mask) {
	int current;
	int currentMask = mask;
	int i;

	for (i = BOARDSIZE - 1; i >= 0; i--) {
		if (currentMask == 0) {
			break;
		}
		current = currentMask & 0x1;
		switch (dest[i]) {
		case Rx:
		case Rt:
			if (current == 1) {
				dest[i] = Rt;
			} else {
				dest[i] = Rx;
			}
			currentMask = currentMask >> 1;
			break;
		case Wx:
		case Wt:
			if (current == 1) {
				dest[i] = Wt;
			} else {
				dest[i] = Wx;
			}
			currentMask = currentMask >> 1;
			break;
		case Blank:
			break;
		default:
			printf("You should never get here in FixOrientations");
		}
	}
}

/************************************************************************
**
** NAME:        blankoxOX_init
**
** DESCRIPTION: intialize the blankoxOX to all blank
**
************************************************************************/
void blankoxOX_init(BlankoxOX *board) {
	int i;
	for (i = 0; i < BOARDSIZE; i++) {
		board[i] = Blank;
	}
}


/************************************************************************
**
** NAME:        board_init
**
** DESCRIPTION: initialize the board to all blank
**
************************************************************************/
void board_init(char *board) {
	int i;
	for (i = 0; i < BOARDSIZE; i++) {
		board[i] = 'b';
	}
}


/************************************************************************
**
** NAME:        printMask
**
** DESCRIPTION: prints the bits that are masked
**
************************************************************************/
void printMask(int mask) {
	printf("Mask: %x \n", mask);

}


//// Changed starting here, by Sunil

/***    printf("\tS)\t(S)pinning a piece in place toggle from %s to %s\n",
           gRotateInPlace ? "ON" : "OFF",
           !gRotateInPlace ? "ON" : "OFF");
    printf("\tO)\t(O)ne space hops toggle from %s to %s\n",
           gHopOne ? "ON" : "OFF",
           !gHopOne ? "ON" : "OFF");
    printf("\tT)\t(T)wo space hops toggle from %s to %s\n",
           gHopTwo ? "ON" : "OFF",
           !gHopTwo ? "ON" : "OFF");
    printf("\tL)\tAllow (l)anding in different orientation toggle from %s to %s \n",
           gRotateOnHop ? "ON" : "OFF",
           !gRotateOnHop ? "ON" : "OFF");
    printf("\tP)\tTrapped (p)layer toggle from %s to %s\n",
           gStuckAWin ? "WINNER" : "LOSER",
           !gStuckAWin ? "WINNER" : "LOSER");
 ***/

STRING kDBName = "ctoi";

int NumberOfOptions()
{
	return 64;
}

int getOption()
{
	int ret;
	ret = gRotateInPlace;
	ret = ret << 1;
	ret = ret + gHopOne;
	ret = ret << 1;
	ret = ret + gHopTwo;
	ret = ret << 1;
	ret = ret + gRotateOnHop;
	ret = ret << 1;
	ret = ret + gStuckAWin;
	ret = ret << 1;
	ret = ret + gStandardGame;
	ret++;
	return ret;
}

void setOption(int option)
{
	option--;
	gStandardGame = option % 2;
	option /= 2;
	gStuckAWin = option % 2;
	option /= 2;
	gRotateOnHop = option % 2;
	option /= 2;
	gHopTwo = option % 2;
	option /= 2;
	gHopOne = option % 2;
	option /= 2;
	gRotateInPlace = option % 2;

	InitializeHelpString();
}

/* unhasing function specifically for use with the tcl version */
STRING unhash (POSITION pos) {
	int i;
	char* board = (char*) SafeMalloc(sizeof(char) * (BOARDSIZE + 1));
	static BlankoxOX theBlankoxOX[BOARDSIZE];
	PositionToBlankoxOX(pos, theBlankoxOX);
	for (i = 0; i < BOARDSIZE; i++) {
		if (theBlankoxOX[i] == Blank)
			board[i] = '-';
		else if (theBlankoxOX[i] == Rx)
			board[i] = 'X';
		else if (theBlankoxOX[i] == Wx)
			board[i] = 'x';
		else if (theBlankoxOX[i] == Rt)
			board[i] = 'T';
		else if (theBlankoxOX[i] == Wt)
			board[i] = 't';
	}
	board[BOARDSIZE] = '\0';
	return board;
}
POSITION StringToPosition(char* string) {
	// BlankoxOX* board = (char *) SafeMalloc(size(BlankoxOX) * (BOARDSIZE + 1));
	static BlankoxOX board[BOARDSIZE];
	int i;
	for(i = 0; i < BOARDSIZE; i++){
		if (string[i] == '-')
			board[i] = Blank;
		else if (string[i] == 'X')
			board[i] = Rx;
		else if (string[i] =='x')
			board[i] = Wx;
		else if (string[i] == 'T')
			board[i] = Rt;
		else if (string[i] == 't')
			board[i] = Wt;
	}
	int index = strcspn(string, "=");
	char turn = string[index + 1];

	if (turn == 'r')
		return HashChungToi(board, Rx);
	else
		return HashChungToi(board, Wx);
}


char* PositionToString(POSITION pos) {
	BlankoxOX turn = GetTurn(pos);
	if(turn == Rx)
		return MakeBoardString(unhash(pos), "turn" , StringDup("red"), "");
	else
		return MakeBoardString(unhash(pos), "turn" , StringDup("white"), "");
}

char * PositionToEndData(POSITION pos) {
	return NULL;
}
