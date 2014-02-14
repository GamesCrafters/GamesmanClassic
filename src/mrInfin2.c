
/********************************************************************
**
** NAME: mrInfin2.c
**
** DESCRIPTION: Rubik's Infinity
**
** AUTHOR: Edwin Mach and Chi Huynh (format and most code from Dan Garcia's GAMEMAN)
**
** DATE: November 22, 2001 (the beginning)
**
** UPDATES HISTORY:
**           - Apr 3, 2002  - Fixed bug for Special/normal piece.
**           - Mar 1-2, 2002   - Attempt REDUCT again (EM)
**           - Jan 2-4, 2002 - Make Gametree smaller by hard-coding possible move positions. The
**                           reduction will still use 60 megs. Changes are tagged "REDUCT" (EM)
**           - Dec 25-27, 2001 - Aesthetic fixes. :) (EM)
**           - Dec 9, 2001  - Bugs #1-19 FIXED (CH)
**                          - Still need to make it look nicer. :)
**           - Dec 6, 2001  - Bugs #1-16 FIXED (CH EM not chemistry)
**           - Dec 2, 2001  - Rewrote the Help menu and a mini-INTERNALS (EM)
**                          - Bugs #1-9 FIXED (EM)
**           - Dec 1, 2001  - attempt to fix the bugs.  (EM)
**                          - Fixed several crucial bugs.  (EM)
**                          - I have the hardcopy of bugs, btw. (EM)
**           - Nov 30, 2001 - ran debugger, no more seg fault! but need to fix
**                            some minor details about the game. Otherwise it works! (EM)
**           - Nov 22, 2001 - Compiled but when first playing the game, Seg fault! (EM)
**                          - Rewrote the ENTIRE game from scratch and developing new data struct.(EM)
**
**
** BUG REPORT: FIXES
**   1. "2" is actually "1" for human player. FIXED (EM)
**   2. The computer moved 3 but nothing happened. FIXED (EM)
**   3. "Special piece" printout skews table. FIXED (EM)
**   4. 2nd row is skewed by 2 spaces to the right. FIXED (EM)
**   5. Line space before showing the first row of board. FIXED (EM)
**   6. Can't take "1" as input from human player. FIXED (EM)
**   7. Can't see the computer's move except the Big "X". FIXED (EM)
**   8. Comp's move of 2 is a special piece "X". FIXED (EM)
**   9. Sometimes, I don't even see my own moves! FIXED (EM)
**  10. Init Score is 6 and Yellow wins !!! FIXED (CH)
**  11. Shifting up of some pieces were wrong.  FIXED (CH)
**  12. Numbers passed into ThreeInaRow method were off by one.  FIXED (CH)
**  13. Special pieces can now be inputted correctly.  Previously,
**      white special pieces could not be inputted properly because
**      it was checking the existence of the black special FIXED (CH)
**  14. Shifting of all pieces is now correct. FIXED (CH)
**  15. Score is never UPDATED!!!  ThreeInaRow always returns 0, but why? FIXED (EM)
**  16. Shifting of pieces II. They should all be fixed now. FIXED (EM)
**  18. Score update is delayed by 1 move. I just noticed this... (CM)
**  19. a. Computer wins, when the PLAYER actually wins. (CM)
**      b. The program outputs the correct winners when someone gets two matches.
**         however, when my score goes to 5, I lose, but i'm not supposed to.(CM)
**
**
** BUG REPORT II: NEED TO BE FIXED!!!!
**  20. REDUCT: The return value of the game is LOSE, instead of a
** TIE. The game SHOULD be a TIE.
**      - ThreeInARow works. Try looking at the Static Eval and
* Fuzzy methods...(EM)
**
**
**
** NOTE: When adding comments or whatnot, please follow the format. Thanks.
**
********************************************************************/

/*************************************************************************
**
** Un/Hashing,Data Structure Explanation:
** --------------------------------------
**   Written by Edwin Mach (Dec 2, 2001)
** --------------------------------------
**
** In our first set,there exists 13 buckets, each bucket telling us something
** about the current state of the board/game. There also exists another set
** of 13 buckets, rearranged, to satisfy Professor Dan Garcia's original format.
**
** Turn '0' == YELLOW's turn
** Turn 'X' == BLUE's turn
**
** I must remind you that this is a STRICT format that we must follow for this game to work.
** If you are modifying something and you have no idea what the heck you are doing, don't do it.
**
** Board:  | 1 2 3 |
**         | 4 5 6 |
**         | 7 8 9 |
**           ^ ^ ^ <-- push pieces up from here.
**
** Here is a breakdown of the first set. The THEPOSITION is an integer that we hashed.
**  b13 = thePosition % 2; // whoseturn
**  b12 = (thePosition / 2) % 3; // 9th position on board
**  b11 = (thePosition / (2*3)) % 3; //8 th
**  b10 = (thePosition / (2*3*3)) % 3; //7 th
**  b9 = (thePosition / (2*3*3*3)) % 3; //6
**  b8 = (thePosition / (2*3*3*3*3)) % 3; //5
**  b7 = (thePosition / (2*3*3*3*3*3)) % 3; // 4
**  b6 = (thePosition / (2*3*3*3*3*3*3)) % 3; // 3
**  b5 = (thePosition / (2*3*3*3*3*3*3*3)) % 3; // 2
**  b4 = (thePosition / (2*3*3*3*3*3*3*3*3)) % 3; // 1th
**  b3 = (thePosition / (2*3*3*3*3*3*3*3*3*3)) % 10; // white
**  b2 = (thePosition / (2*3*3*3*3*3*3*3*3*3*10)) % 10; // black
**  b1 = (thePosition / (2*3*3*3*3*3*3*3*3*3*10*10)) % 11; // score
**
**  In another words from b1-> b13, here are the max values plus one of each bucket:
**  { 11 10 10 3 3 3 3 3 3 3 3 3 2 } == { b1 b2 ... b13 }
**  So b13 could potentially be a 1 or a 0, thus 2 values.
**
** Here's theBlankOX representation:
**  theBlankOX[0] = b4; //1
**  theBlankOX[1] = b5; //2.
**  theBlankOX[2] = b6; //3.
**  theBlankOX[3] = b7; //4.
**  theBlankOX[4] = b8; //5.
**  theBlankOX[5] = b9; //6.
**  theBlankOX[6] = b10; //7
**  theBlankOX[7] = b11; //8
**  theBlankOX[8] = b12; //9   <-- when displaying the board, we call theBlankOX up to here, inclusive.
**  theBlankOX[9] = b1; // score
**  theBlankOX[10] = b2; // black
**  theBlankOX[11] = b3; //white
**  theBlankOX[12] = b13; //turn
**
**  This takes much space, but our game is very complex and not restricted.
**
************************************************************************/

/************************************************************************
**
** Hard-coding of all mrInfin's Positions (3 by 3 version)
** -------------------------------------------------------
** by Edwin Mach (Jan 2, 2001)
** ---------------------------
**
** There will be 15 possible positions for each column and all these positions
** will be hardcoded to save memory space. This reduction is significant
** because it reduces the memory usage to 1/7 the original amount of space!!!
** For this game to run on desktops, and many desktops DO NOT have
** 500 megs of RAM to play with, we saw it vital that we make this change
** and essential to the game being able to run on our OWN systems at home. :)
**
** IMPORTANT NOTES: From now on, these changes will have this keyword:
**                  "REDUCT"
**
** theBlankOX[ 7 is the max num]
**
** NEW HASHING INFO:
**  For Position integer:
**  b6 = thePosition % 2; // turn
**  b5 = (thePosition / 2) % 15; // 3rd column
**  b4 = (thePosition / (2*15)) % 15; //2nd column
**  b3 = (thePosition / (2*15*15)) % 15; //1st column
**  b2 = (thePosition / (2*15*15*15)) % 10; // white
**  b1 = (thePosition / (2*15*15*15*10)) % 10; //black
**  b0 = (thePosition / (2*15*15*15*10*10)) % 11; // score
**
**  theBlankOX:
**   theBlankOX[0] = b0; //score
**   theBlankOX[1] = b1; //black
**   theBlankOX[2] = b2; //white
**   theBlankOX[3] = b3; //1st column
**   theBlankOX[4] = b4; //2nd column
**   theBlankOX[5] = b5; //3rd column
**   theBlankOX[6] = b6; // turn
**
************************************************************************/


/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/

#include <string.h>
#include <stdio.h>
#include "gamesman.h"

POSITION gNumberOfPositions  = 7425000; // REDUCT: 11*10*10*15^3*2
                                        //43302600<--old num (11*100*3^9*2)

POSITION gInitialPosition    = 3375000; // REDUCT: score = 5, everythign else 0
// 19683000<-- old num; //
POSITION kBadPosition        = -1; /* This can never be the rep. of a position  */

STRING kAuthorName         = "Edwin Mach and Chi Huynh";
STRING kGameName           = "Rubik's Infinity";
STRING kDBName             = "rinfin2";
BOOLEAN kPartizan           = TRUE;
BOOLEAN kDebugMenu          = FALSE;
BOOLEAN kGameSpecificMenu   = FALSE;
BOOLEAN kTieIsPossible      = TRUE;
BOOLEAN kLoopy               = TRUE;
BOOLEAN kDebugDetermineValue = FALSE;
void*    gGameSpecificTclInit = NULL;

STRING kHelpGraphicInterface =
        "The LEFT button puts an X or O (depending on whether you went first\n\
or second) on the spot the cursor was on when you clicked. The MIDDLE\n\
button does nothing, and the RIGHT button is the same as UNDO, in that\n\
it reverts back to your your most recent position."                                                                                                                                                                                                                                   ;

STRING kHelpTextInterface    =
        "On your turn, use the LEGEND to determine which number to choose (between\n\
1 and 6, with 1 - 3 as inputting the normal pieces into columns 1-3 and\n\
humbers 4-6 as the optional pieces into columns 1-3 (Basically, subtract 3\n\
from your option 4-6.) Then hit return. If at any point you have made a mistake,\n\
you can type u and hit return and the system will\n\
revert back to your most recent position."                                                                                                                                                                                                                                                                                                                                                                                        ;

STRING kHelpOnYourTurn =
        "You place one of your pieces into any of the columns (1-3). For putting in\n\
special pieces use move Numbers (4-6)."                                                                                       ;

STRING kHelpStandardObjective =
        "To get three of your markers (either X or O) in a row, either\n\
horizontally, vertically, or diagonally. Two 3-in-a-row WINS the game\n\
automatically."                                                                                                                                                   ;

STRING kHelpReverseObjective =
        "To force your opponent into getting three of his markers (either X or\n\
O) in a row, either horizontally, vertically, or diagonally. 3-in-a-row\n\
LOSES."                                                                                                                                                             ;

STRING kHelpTieOccursWhen =   /* Should follow 'A Tie occurs when... */
                            "you just can't beat the computer.";

STRING kHelpExample =
        "         |:Rubik:|                    |:Infin:|\n\
         | _ _ _ |                    | - - - |\n\
LEGEND:  | _ _ _ |                    | - - - |\n\
         | _ _ _ |                    | - - - |\n\
           ^ ^ ^                        ^ ^ ^\n\
           1 2 3  <-- Normal pieces\n\
           4 5 6  <-- Special pieces\n\
\n\
Current Score: 0\n\
Game is currently even.\n\
(Dan should Tie in 0)\n\
\n\
It is yellow's turn.     Dan's move [(u)ndo/1-6] :  {2}\n\
\n\
         |:Rubik:|                    |:Infin:|\n\
         | _ _ _ |                    | - - - |\n\
LEGEND:  | _ _ _ |                    | - - - |\n\
         | _ _ _ |                    | - o - |\n\
           ^ ^ ^                        ^ ^ ^\n\
           1 2 3  <-- Normal pieces\n\
           4 5 6  <-- Special pieces\n\
\n\
Current Score: 0\n\
Game is currently even.\n\
(Computer should Tie in 0)\n\
\n\
It is blue's turn.Computer's move              :  1\n\
\n\
         |:Rubik:|                    |:Infin:|\n\
         | _ _ _ |                    | - - - |\n\
LEGEND:  | _ _ _ |                    | - - - |\n\
         | _ _ _ |                    | x o - |\n\
           ^ ^ ^                        ^ ^ ^\n\
           1 2 3  <-- Normal pieces\n\
           4 5 6  <-- Special pieces\n\
\n\
Current Score: 0\n\
Game is currently even.\n\
(Dan should Tie in 26)\n\
\n\
It is yellow's turn.     Dan's move [(u)ndo/1-6] :  {5}\n\
\n\
         |:Rubik:|                    |:Infin:|\n\
         | _ _ _ |                    | - - - |\n\
LEGEND:  | _ _ _ |                    | - o - |\n\
         | _ _ _ |                    | x O - |\n\
           ^ ^ ^                        ^ ^ ^\n\
           1 2 3  <-- Normal pieces\n\
           4 5 6  <-- Special pieces\n\
\n\
Current Score: 0\n\
Game is currently even.\n\
(Computer will Win in 25)\n\
\n\
It is blue's turn.Computer's move              :  2\n\
\n\
         |:Rubik:|                    |:Infin:|\n\
         | _ _ _ |                    | - o - |\n\
LEGEND:  | _ _ _ |                    | - O - |\n\
         | _ _ _ |                    | x x - |\n\
           ^ ^ ^                        ^ ^ ^\n\
           1 2 3  <-- Normal pieces\n\
           4 5 6  <-- Special pieces\n\
\n\
Current Score: 0\n\
Game is currently even.\n\
(Dan will Lose in 24)\n\
\n\
It is yellow's turn.     Dan's move [(u)ndo/1-6] :  {1}\n\
\n\
         |:Rubik:|                    |:Infin:|\n\
         | _ _ _ |                    | - o - |\n\
LEGEND:  | _ _ _ |                    | x O - |\n\
         | _ _ _ |                    | o x - |\n\
           ^ ^ ^                        ^ ^ ^\n\
           1 2 3  <-- Normal pieces\n\
           4 5 6  <-- Special pieces\n\
\n\
Current Score: 0\n\
Game is currently even.\n\
(Computer will Win in 23)\n\
\n\
It is blue's turn.\n\
Computer's move              :  6\n\
Computer placed a SPECIAL PIECE. Watch out! :)\n\
\n\
         |:Rubik:|                    |:Infin:|\n\
         | _ _ _ |                    | - o - |\n\
LEGEND:  | _ _ _ |                    | x O - |\n\
         | _ _ _ |                    | o x X |\n\
           ^ ^ ^                        ^ ^ ^\n\
           1 2 3  <-- Normal pieces\n\
           4 5 6  <-- Special pieces\n\
\n\
Current Score: 0\n\
Game is currently even.\n\
(Dan will Lose in 22)  "                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  ;


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


#define BOARDSIZE 13
// Yeah, 13... 9 is hardcoded in some places for the REAL board, for ex:. I use 9 if i want to print out the board using the the BlankOx
#define REALBOARDSIZE 9
// Replace all 9's with REALBOARDSIZE
#define REDUCTBOARDSIZE 7

typedef enum possibleBoardPieces {
	Blank, o, x,
} BlankOX;

char *gBlankOXString[] = { "-", "o", "x"}; // will change later to add o, x

// REDUCT
// hardcode of all possible positions
char gBlankOXPosition[15][4] = { "---", // 0
	                         "--o", //1
	                         "-ox", //2
	                         "-oo",
	                         "oxx", //4
	                         "oxo",
	                         "oox",
	                         "ooo", //7
	                         "--x", //8
	                         "-xo", //9
	                         "-xx",
	                         "xoo", //11
	                         "xox",
	                         "xxo",
	                         "xxx"  //14
}; // 15 possible positions

/** Function Prototypes **/
void PositionToBlankOX(POSITION thePos,BlankOX *theBlankOX);
int ThreeInARow(BlankOX *theBlankOX, int a, int b, int c);

STRING MoveToString( MOVE );

/************************************************************************
**
** NAME:        InitializeGame
**
** DESCRIPTION: Initialize the game.
**
************************************************************************/

void InitializeGame() {
	gMoveToStringFunPtr = &MoveToString;
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
	/* No need to have anything here, we have no extra options */
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
** CALLS:       PositionToBlankOX(POSITION,*BlankOX)
**              BlankOX WhosTurn(*BlankOX)
**
************************************************************************/

POSITION DoMove(thePosition, theMove)
POSITION thePosition;
MOVE theMove;
{
	//if theMove goes beyond 1, 2, 3, or 4, 5, 6 (for the special pieces)then throw error
	int currentScore, tempScore;
	int matchingYellow, matchingBlue, totalmatches;
	int c1, c2, c3, c4, c5, c6, c7, c8; // ints to find columns,rows,diags

	BlankOX theBlankOX[REDUCTBOARDSIZE], WhoseTurn();

	int columnNum, turn;
	int b0, b1, b2, b3,b4,b5,b6; //immediate retrieval of thePosition's values

	PositionToBlankOX(thePosition,theBlankOX);
	// EVERY VARIABLE INIT needs to be before this line!

	// Create and store the variables b*
	b6 = thePosition % 2; // turn
	b5 = (thePosition / 2) % 15; // 3rd column
	b4 = (thePosition / (2*15)) % 15; //2nd column
	b3 = (thePosition / (2*15*15)) % 15; //1st column
	b2 = (thePosition / (2*15*15*15)) % 10; // white
	b1 = (thePosition / (2*15*15*15*10)) % 10; //black
	b0 = (thePosition / (2*15*15*15*10*10)) % 11; // score

	//this tells us which bucket will be changed

	if (theMove > 3)
		columnNum = theMove - 3;
	else
		columnNum = theMove;

	//this gets the number position/number currently in the bucket that will be changed
	turn = b6;

	//printf("Inside DoMove!");

	if (turn == 0) { // white's turn
		//O's turn, meaning change it to X's turn
		b6 = 1;
		// bump special pieces up
		// 1. Find what column or movenumber = columnNum
		// 2. Find where the special piece exists from theBlankOX
		// 3. Find what special piece it is
		// 4. if special piece exists at the top, we reset it back to 0, because it's gone
		//if a special piece was just inserted, then assign it's position

		if (columnNum == 1) { // do Column1
			// check for special pieces

			// normal moves
			if (b3 == 0) {
				b3=1;
			}
			else if (b3 == 1) {
				b3=3;
			}
			else if (b3 == 2) {
				b3=5;
			}
			else if (b3 == 3) {
				b3=7;
			}
			else if (b3 == 4) {
				b3=13;
			}
			else if (b3 == 5) {
				b3=11;
			}
			else if (b3 == 6) {
				b3=5;
				// check if o was special
			}
			else if (b3 == 7) {
				b3=7; // same
			}
			else if (b3 == 8) {
				b3=9;
			}
			else if (b3 == 9) {
				b3=11;
			}
			else if (b3 == 10) {
				b3=13;
			}
			else if (b3 == 11) {
				b3=7;
			}
			else if (b3 == 12) {
				b3=5;
			}
			else if (b3 == 13) {
				b3=11;
			}
			else if (b3 == 14) {
				b3=13;
			}
			else {}

			// for all the special pieces
			// black
			if (b1 == 1) {
				b1 = 0;
			}
			if (b1 == 4) {
				b1 = 1;
			}
			if (b1 == 7) {
				b1 = 4;
			}
			// white
			if (b2 == 1) {
				b2 = 0;
			}
			if (b2 == 4) {
				b2 = 1;
			}
			if (b2 == 7) {
				b2 = 4;
			}
			if (theMove == 4) {
				b2=7; // white on pos7
			}

		}
		else if (columnNum == 2) {

			if (b4 == 0) {
				b4 =1;
			}
			else if (b4 == 1) {
				b4 = 3;
			}
			else if (b4 == 2) {
				b4 = 5;
			}
			else if (b4 == 3) {
				b4 = 7;
			}
			else if (b4 == 4) {
				b4 = 13;
			}
			else if (b4 == 5) {
				b4 = 11;
			}
			else if (b4 == 6) {
				b4 = 5;
			}
			else if ( b4 == 7) {
				b4 = 7;
			}
			else if (b4 == 8) {
				b4 = 9;
			}
			else if (b4 ==9) {
				b4 = 11;
			}
			else if (b4 == 10) {
				b4 = 13;
			}
			else if (b4 == 11) {
				b4 = 7;
			}
			else if (b4 == 12) {
				b4 = 5;
			}
			else if (b4 == 13) {
				b4 = 11;
			}
			else if (b4 == 14) {
				b4 = 13;
			}
			else {}

			// for all the special pieces
			// black
			if (b1 == 2) {
				b1 = 0;
			}
			if (b1 == 5) {
				b1 = 2;
			}
			if (b1 == 8) {
				b1 = 5;
			}
			// white
			if (b2 == 2) {
				b2 = 0;
			}
			if (b2 == 5) {
				b2 = 2;
			}
			if (b2 == 8) {
				b2 = 5;
			}
			if (theMove == 5) {
				b2 = 8;
			}


		}
		else if (columnNum == 3) {


			if (b5 == 0) {
				b5 =1;
			}
			else if (b5 == 1) {
				b5 = 3;
			}
			else if (b5 == 2) {
				b5 = 5;
			}
			else if (b5 == 3) {
				b5 = 7;
			}
			else if (b5 == 4) {
				b5 = 13;
			}
			else if (b5 == 5) {
				b5 = 11;
			}
			else if (b5 == 6) {
				b5 = 5;
			}
			else if ( b5 == 7) {
				b5 = 7;
			}
			else if (b5 == 8) {
				b5 = 9;
			}
			else if (b5 ==9) {
				b5 = 11;
			}
			else if (b5 == 10) {
				b5 = 13;
			}
			else if (b5 == 11) {
				b5 = 7;
			}
			else if (b5 == 12) {
				b5 = 5;
			}
			else if (b5 == 13) {
				b5 = 11;
			}
			else if (b5 == 14) {
				b5 = 13;
			}
			else {}

			// for all the special pieces
			// black
			if (b1 == 3) {
				b1 = 0;
			}
			if (b1 == 6) {
				b1 = 3;
			}
			if (b1 == 9) {
				b1 = 6;
			}
			// white
			if (b2 == 3) {
				b2 = 0;
			}
			if (b2 == 6) {
				b2 = 3;
			}
			if (b2 == 9) {
				b2 = 6;
			}
			if (theMove == 6) {
				b2 = 9;
			}
		}
		else { }

	}
	else {
		// X's turn,so change it to O's turn
		b6 = 0;

		if (columnNum == 1) {

			if (b3 == 0) {
				b3 =8;
			}
			else if (b3 == 1) {
				b3 = 2;
			}
			else if (b3 == 2) {
				b3 = 4;
			}
			else if (b3 == 3) {
				b3 = 6;
			}
			else if (b3 == 4) {
				b3 = 14;
			}
			else if (b3 == 5) {
				b3 = 12;
			}
			else if (b3 == 6) {
				b3 = 4;
			}
			else if ( b3 == 7) {
				b3 = 6;
			}
			else if (b3 == 8) {
				b3 = 10;
			}
			else if (b3 ==9) {
				b3 = 12;
			}
			else if (b3 == 10) {
				b3 = 14;
			}
			else if (b3 == 11) {
				b3 = 6;
			}
			else if (b3 == 12) {
				b3 = 4;
			}
			else if (b3 == 13) {
				b3 = 12;
			}
			else if (b3 == 14) {
				b3 = 14;
			}
			else {}

			// for all the special pieces
			// black
			if (b1 == 1) {
				b1 = 0;
			}
			if (b1 == 4) {
				b1 = 1;
			}
			if (b1 == 7) {
				b1 = 4;
			}
			if (theMove == 4) {
				b1 = 7;
			}
			// white
			if (b2 == 1) {
				b2 = 0;
			}
			if (b2 == 4) {
				b2 = 1;
			}
			if (b2 == 7) {
				b2 = 4;
			}
		}
		if (columnNum == 2) {


			if (b4 == 0) {
				b4 =8;
			}
			else if (b4 == 1) {
				b4 = 2;
			}
			else if (b4 == 2) {
				b4 = 4;
			}
			else if (b4 == 3) {
				b4 = 6;
			}
			else if (b4 == 4) {
				b4 = 14;
			}
			else if (b4 == 5) {
				b4 = 12;
			}
			else if (b4 == 6) {
				b4 = 4;
			}
			else if ( b4 == 7) {
				b4 = 6;
			}
			else if (b4 == 8) {
				b4 = 10;
			}
			else if (b4 == 9) {
				b4 = 12;
			}
			else if (b4 == 10) {
				b4 = 14;
			}
			else if (b4 == 11) {
				b4 = 6;
			}
			else if (b4 == 12) {
				b4 = 4;
			}
			else if (b4 == 13) {
				b4 = 12;
			}
			else if (b4 == 14) {
				b4 = 14;
			}
			else {}

			// for all the special pieces
			// black
			if (b1 == 2) {
				b1 = 0;
			}
			if (b1 == 5) {
				b1 = 2;
			}
			if (b1 == 8) {
				b1 = 5;
			}
			if (theMove == 5) {
				b1 = 8;
			}

			// white
			if (b2 == 2) {
				b2 = 0;
			}
			if (b2 == 5) {
				b2 = 2;
			}
			if (b2 == 8) {
				b2 = 5;
			}
		}

		if (columnNum == 3) {

			if (b5 == 0) {
				b5 =8;
			}
			else if (b5 == 1) {
				b5 = 2;
			}
			else if (b5 == 2) {
				b5 = 4;
			}
			else if (b5 == 3) {
				b5 = 6;
			}
			else if (b5 == 4) {
				b5 = 14;
			}
			else if (b5 == 5) {
				b5 = 12;
			}
			else if (b5 == 6) {
				b5 = 4;
			}
			else if ( b5 == 7) {
				b5 = 6;
			}
			else if (b5 == 8) {
				b5 = 10;
			}
			else if (b5 ==9) {
				b5 = 12;
			}
			else if (b5 == 10) {
				b5 = 14;
			}
			else if (b5 == 11) {
				b5 = 6;
			}
			else if (b5 == 12) {
				b5 = 4;
			}
			else if (b5 == 13) {
				b5 = 12;
			}
			else if (b5 == 14) {
				b5 = 14;
			}
			else {}

			// for all the special pieces
			// black
			if (b1 == 3) {
				b1 = 0;
			}
			if (b1 == 6) {
				b1 = 3;
			}
			if (b1 == 9) {
				b1 = 6;
			}
			if (theMove == 6) {
				b1 = 9;
			}

			// white
			if (b2 == 3) {
				b2 = 0;
			}
			if (b2 == 6) {
				b2 = 3;
			}
			if (b2 == 9) {
				b2 = 6;
			}
		}
		else { } // end columnNum

	} // end else

	//printf("%d", &b3);

	// Check for the score
	currentScore = b0;
	tempScore = 0;

	matchingYellow = 0;
	matchingBlue = 0;

	//update theBlankOX(just the piece locations) before sending it
	//to threeInaRow
	theBlankOX[0] = b0;
	theBlankOX[1] = b1;
	theBlankOX[2] = b2;
	theBlankOX[3] = b3;
	theBlankOX[4] = b4;
	theBlankOX[5] = b5;
	theBlankOX[6] = b6;

	// hardcode the score calculation
	// ThreeInARow will handle SPECIAL pieces
	c1 = ThreeInARow( theBlankOX, 8,7,6); //bot row
	c2 = ThreeInARow( theBlankOX, 5,4,3 ); // mid row
	c3 = ThreeInARow( theBlankOX,2,1,0); //top row
	c4 = ThreeInARow( theBlankOX,8,5,2); //right col
	c5 = ThreeInARow( theBlankOX,7,4,1); // mid col
	c6 = ThreeInARow( theBlankOX,6,3,0); //left col
	c7 = ThreeInARow( theBlankOX, 8,4,0); //diag
	c8 = ThreeInARow( theBlankOX,6,4,2); //diag

	//printf("%d %d %d %d %d %d %d %d<--3 in a row", c1, c2, c3, c4, c5, c6, c7, c8);

	// Count the numbers of rows matched for each player
	matchingYellow = 0; matchingBlue = 0; totalmatches = 0;
	if (c1 > 0)
		matchingYellow += 1;
	else if (c1 <0)
		matchingBlue += 1;

	if ( c2 > 0)
		matchingYellow += 1;
	else if (c2 < 0 )
		matchingBlue += 1;

	if ( c3 > 0)
		matchingYellow += 1;
	else if (c3 < 0 )
		matchingBlue += 1;

	if ( c4 > 0)
		matchingYellow += 1;
	else if (c4 < 0 )
		matchingBlue += 1;

	if ( c5 > 0)
		matchingYellow += 1;
	else if (c5 < 0 )
		matchingBlue += 1;

	if ( c6 > 0)
		matchingYellow += 1;
	else if (c6 < 0 )
		matchingBlue += 1;

	if ( c7 > 0)
		matchingYellow += 1;
	else if (c7 < 0 )
		matchingBlue += 1;

	if ( c8 > 0)
		matchingYellow += 1;
	else if (c8 < 0 )
		matchingBlue += 1;

	totalmatches = matchingYellow - matchingBlue;
	if (totalmatches >= 2) {
		// now, we know that yellow has 2 or more matches than blue, and thus constitutes a possible GAME WIN
		b0 = 10; // yellow increases, blue decreases
	}
	else if (totalmatches <= -2 ) {
		// now we know that BLUE has 2 or more matches than yellow, and thus constitutes a possible GAME WIN
		b0 = 0; // blue decreases to 0
	}
	else {
		// just add the score to b0
		tempScore = c1 + c2 + c3 + c4 + c5 + c6 + c7 + c8;
		currentScore += tempScore;
		b0 = currentScore;
		// incase the score goes over 10 or 0
		if ( b0 > 10)
			b0 = 10;
		else if (b0 < 0)
			b0 = 0;
		else {}
	}

	return b6 + (2*b5) + (2*15*b4) + (2*15*15*b3) +
	       (2*15*15*15*b2) + (2*15*15*15*10*b1) + (2*15*15*15*100*b0);

	/* return b13 + (2*b12) + (2*3*b11) + (2*9*b10) + (2*3*3*3*b9) +
	   (2*27*3*b8) + (2*27*9*b7) + (2*27*27*b6)+
	   (2*27*27*3*b5) + (2*27*27*9*b4) + (2*27*27*27*b3) +
	   (2*27*27*27*10*b2) + (2*27*27*27*100*b1); */
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
	POSITION BlankOXToPosition();
	BlankOX theBlankOX[REDUCTBOARDSIZE], whosTurn, temp;
	signed char c;
	int i;
	int c1,c2,c3,c4,c5,c6,c7,c8,c9; // REDUCT: These nums stores all the values first before actually storing them into theBlankOX var
	int column1, column2, column3;

	printf("\n\n\t----- Get Initial Position -----\n");
	printf("\n\tPlease input the position to begin with.\n");
	printf("\tNote that it should be in the following format:\n\n");
	printf("O - -\nO - -            <----- EXAMPLE \n- X X\n\n");
	printf("n such that n is a number between 1 and 9, inclusive, where 1 means that blue needs one more point to win and 9 means that yellow needs just one more point to win.");

	getchar();


	//makes it such that the initial positions of the special pieces are off the board. THIS METHOD IS STILL IN QUESTION.

	// REDUCT NEED TO CHANGE THIS CODE
	// Store all the 9 inputs before the decision is made
	for (i = 0; i < REALBOARDSIZE && (c = getchar()) != EOF; i += 1) {
		if(c == 'x' || c == 'X')
			temp = x;
		else if(c == 'o' || c == 'O' || c == '0')
			temp = o;
		else if(c == '-')
			temp = Blank;
		else
			; /* do nothing */
		if (i ==0) { c1 = temp; }
		else if ( i == 1) { c2 = temp; }
		else if ( i == 2) { c3 = temp; }
		else if ( i == 3) { c4 = temp; }
		else if ( i == 4) { c5 = temp; }
		else if ( i == 5) { c6 = temp; }
		else if ( i == 6) { c7 = temp; }
		else if ( i == 7) { c8 = temp; }
		else if ( i == 8) { c9 = temp; }
		else {}

	}

	// hardcode matching of gBlankOXPosition
	// Column1 first
	if (c1 == Blank && c4 == Blank && c7 == Blank) { column1 = 0; }
	else if (c1 == Blank && c4 == Blank && c7 == o) { column1 = 1; }
	else if (c1 == Blank && c4 == o && c7 == x) { column1 = 2; }
	else if (c1 == Blank && c4 == o && c7 == o) { column1 = 3; }
	else if (c1 == o && c4 == x && c7 == x) { column1 = 4; }
	else if (c1 == o && c4 == x && c7 == o) { column1 = 5; }
	else if (c1 == o && c4 == o && c7 == x) { column1 = 6; }
	else if (c1 == o && c4 == o && c7 == o) { column1 = 7; }
	else if (c1 == Blank && c4 == Blank && c7 == x) { column1 = 8; }
	else if (c1 == Blank && c4 == x && c7 == o) { column1 = 9; }
	else if (c1 == Blank && c4 == x && c7 == x) { column1 = 10; }
	else if (c1 == x && c4 == o && c7 == o) { column1 = 11; }
	else if (c1 == x && c4 == o && c7 == x) { column1 = 12; }
	else if (c1 == x && c4 == x && c7 == o) { column1 = 13; }
	else if (c1 == x && c4 == x && c7 == x) { column1 = 14; }
	else {}

	// col 2
	if (c2 == Blank && c5 == Blank && c8 == Blank) { column2 = 0; }
	else if (c2 == Blank && c5 == Blank && c8 == o) { column2 = 1; }
	else if (c2 == Blank && c5 == o && c8 == x) { column2 = 2; }
	else if (c2 == Blank && c5 == o && c8 == o) { column2 = 3; }
	else if (c2 == o && c5 == x && c8 == x) { column2 = 4; }
	else if (c2 == o && c5 == x && c8 == o) { column2 = 5; }
	else if (c2 == o && c5 == o && c8 == x) { column2 = 6; }
	else if (c2 == o && c5 == o && c8 == o) { column2 = 7; }
	else if (c2 == Blank && c5 == Blank && c8 == x) { column2 = 8; }
	else if (c2 == Blank && c5 == x && c8 == o) { column2 = 9; }
	else if (c2 == Blank && c5 == x && c8 == x) { column2 = 10; }
	else if (c2 == x && c5 == o && c8 == o) { column2 = 11; }
	else if (c2 == x && c5 == o && c8 == x) { column2 = 12; }
	else if (c2 == x && c5 == x && c8 == o) { column2 = 13; }
	else if (c2 == x && c5 == x && c8 == x) { column2 = 14; }
	else {}

	// col 3
	if (c3 == Blank && c6 == Blank && c9 == Blank) { column3 = 0; }
	else if (c3 == Blank && c6 == Blank && c9 == o) { column3 = 1; }
	else if (c3 == Blank && c6 == o && c9 == x) { column3 = 2; }
	else if (c3 == Blank && c6 == o && c9 == o) { column3 = 3; }
	else if (c3 == o && c6 == x && c9 == x) { column3 = 4; }
	else if (c3 == o && c6 == x && c9 == o) { column3 = 5; }
	else if (c3 == o && c6 == o && c9 == x) { column3 = 6; }
	else if (c3 == o && c6 == o && c9 == o) { column3 = 7; }
	else if (c3 == Blank && c6 == Blank && c9 == x) { column3 = 8; }
	else if (c3 == Blank && c6 == x && c9 == o) { column3 = 9; }
	else if (c3 == Blank && c6 == x && c9 == x) { column3 = 10; }
	else if (c3 == x && c6 == o && c9 == o) { column3 = 11; }
	else if (c3 == x && c6 == o && c9 == x) { column3 = 12; }
	else if (c3 == x && c6 == x && c9 == o) { column3 = 13; }
	else if (c3 == x && c6 == x && c9 == x) { column3 = 14; }
	else {}

	theBlankOX[3] = column1;
	theBlankOX[4] = column2;
	theBlankOX[5] = column3;


	// 9 becuase the first 9 indexes of theBlankOX are the pieces
	/*  while(i < REALBOARDSIZE && (c = getchar()) != EOF) { */
/*      if(c == 'x' || c == 'X') */
/*        theBlankOX[i++] = x; */
/*      else if(c == 'o' || c == 'O' || c == '0') */
/*        theBlankOX[i++] = o; */
/*      else if(c == '-') */
/*        theBlankOX[i++] = Blank; */
/*      else  */
/*        ;   do nothing  */
/*    }  */


	return(BlankOXToPosition(theBlankOX,whosTurn));
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
	if (computersMove <= 3 && computersMove >= 1) {
		printf("%8s's move              : %2d\n", computersName, computersMove);
	}
	else {
		printf("\n%8s's move              : %2d\n\n Computer placed a SPECIAL PIECE. Watch out! :)\n", computersName, computersMove );
	}

	// THIS METHOD IS STILL IN QUESTION.
}

/************************************************************************
**
** NAME:        Primitive
**
** DESCRIPTION: Return the value of a position if it fulfills certain
**              'primitive' constraints. Some examples of this is having
**              three-in-a-row with TicTacToe. TicTacToe has two
**              primitives it can immediately check for, when the board
**              is filled but nobody has one = primitive tie. Three in
**              a row is a primitive lose, because the player who faces
**              this board has just lost. I.e. the player before him
**              created the board and won. Otherwise undecided.
**
** INPUTS:      POSITION position : The position to inspect.
**
** OUTPUTS:     (VALUE) an enum which is oneof: (win,lose,tie,undecided)
**
** CALLS:       PositionToBlankOX()
**
************************************************************************/

VALUE Primitive(position)
POSITION position;
{
	int ThreeInARow();
	BlankOX theBlankOX[REDUCTBOARDSIZE];

	int turn;
	int b0,b6;
	int currentScore, tempScore;

	PositionToBlankOX(position,theBlankOX);
	// EVERY VARIABLE INIT needs to be before this line!

	b6 = position % 2; // turn
	b0 = (position / (2*15*15*15*10*10)) % 11; // score
	turn = b6;
	currentScore = b0; // attempt to keep the score current, not delayed by 1
	tempScore = 0;

	// NOTE: Something different than Dan's code is that we return WIN when Dan's code returns LOSE,
	// and vice-versa

	// First we have to check if we have a winner already!
	if (currentScore == 0) {
		if (gStandardGame) {
			if (turn == 0)
				return (lose);
			else
				return (win);
		}
		else {
			if (turn == 0)
				return (win);
			else
				return (lose);
		}
	}
	if (currentScore == 10) {
		if (gStandardGame) {
			if (turn == 0)
				return (win);
			else
				return (lose);
		}
		else {
			if (turn == 0)
				return (lose);
			else
				return (win);
		}
	}
	return (undecided);


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
** CALLS:       PositionToBlankOX()
**              GetValueOfPosition()
**              GetPrediction()
**
************************************************************************/

void PrintPosition(position,playerName,usersTurn)
POSITION position;
STRING playerName;
BOOLEAN usersTurn;
{
	/* All our variables before functions */
	BlankOX theBlankOX[REDUCTBOARDSIZE];
	char elt[REALBOARDSIZE];
	char *meters[] = { "----------@",
		           "---------@-",
		           "--------@--",
		           "-------@---",
		           "------@----",
		           "-----@-----",
		           "----@------",
		           "---@-------",
		           "--@--------",
		           "-@---------",
		           "@----------" };
	int white,black;
	int yellowLeads;
	int score;
	int tempScore;
	int b0, b1, b2, b3,b4,b5,b6; //immediate retrieval of thePosition's values
	PositionToBlankOX(position,theBlankOX);

	// Create and store the variables b*
	b6 = position % 2; // turn
	b5 = (position / 2) % 15; // 3rd column
	b4 = (position / (2*15)) % 15; //2nd column
	b3 = (position / (2*15*15)) % 15; //1st column
	b2 = (position / (2*15*15*15)) % 10; // white
	b1 = (position / (2*15*15*15*10)) % 10; //black
	b0 = (position / (2*15*15*15*10*10)) % 11; // score

	elt[8] = gBlankOXPosition[b5][2]; //9
	elt[7] = gBlankOXPosition[b4][2];
	elt[6] = gBlankOXPosition[b3][2];

	//printf("%d %d %d <-- b3 b4 b5\n", b3, b4, b5);

	elt[5] = gBlankOXPosition[b5][1]; // 6.
	elt[4] = gBlankOXPosition[b4][1];
	elt[3] = gBlankOXPosition[b3][1];

	elt[2] = gBlankOXPosition[b5][0];
	elt[1] = gBlankOXPosition[b4][0];
	elt[0] = gBlankOXPosition[b3][0]; //1.

	black = b1;
	if ( black != 0) {
		// black piece location
		elt[ black -1 ] = 'X';
	}
	white = b2;
	if (white != 0) {
		elt[white -1] = 'O';
	}

	// Calculate score BEFORE we print
	yellowLeads = 0;
	tempScore = b0 - 5;

	if (tempScore > 0) {
		yellowLeads = 1;
		score = tempScore;
	}
	else if (tempScore < 0) {
		yellowLeads = 2;
		score = tempScore;
	}
	else
		score = tempScore;


	// start printing out
	printf("\n");
	printf("                 |:Rubik:|       |:Infin:|       \n");
	printf("                 | _ _ _ |       | %c %c %c |       :Score:\n",
	       elt[0], elt[1], elt[2]);
	printf("      LEGEND:    | _ _ _ |       | %c %c %c |       :Meter:\n",
	       elt[3], elt[4], elt[5]);
	printf("                 | _ _ _ |       | %c %c %c |  \n",
	       elt[6], elt[7], elt[8]);
	printf("                   ^ ^ ^           ^ ^ ^\n");

	// 2nd to the last line
	printf("  Normal piece ->  1 2 3");
	// print player's piece
	if (gStandardGame) {
		printf("                    O ");
	} else {
		printf("                    X ");
	}
	printf("|%s| ", meters[score+5]);
	if (gStandardGame) {  printf("X\n"); }
	else { printf("O\n"); }

	// print the special piece line ONLY if the user can put a special piece
	// if it's your turn, and yourspeical piece is no on the board (EM)
	if (gStandardGame && white) {
		//don't print out anything
	} else {
		printf(" Special piece ->  4 5 6  \n");
	}
	printf("\n");


	//printf("Current Score: %d \n", score);
	/* if (yellowLeads == 1)
	   //printf ("Yellow is leading.\n");
	   else if (yellowLeads == 2)
	   //printf ("Blue is leading.\n");
	   else
	   //printf ("Game is currently even.\n");
	 */

	printf (" %s \n\n", GetPrediction(position,playerName,usersTurn));

	if (b6 == 0 && gStandardGame)
		printf("It is \"O\"'s turn.  ");
	else if (b6 == 1)
		printf ("It is \"X\"'s turn.  ");

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
**
************************************************************************/

MOVELIST *GenerateMoves(position)
POSITION position;
{
	MOVELIST *CreateMovelistNode(), *head = NULL;
	VALUE Primitive();
	BlankOX theBlankOX[REDUCTBOARDSIZE];

	int i;
	int turn = position % 2; //theBlankOX[6];
	int black = (position / (2*15*15*15*10)) % 10; // theBlankOX[1];
	int white = (position / (2*15*15*15)) % 10; //theBlankOX[2];

	//printf("Inside GenerateMoves %n", position);

	//depending on whose turn it is, we have to check to see if
	//their special piece has been used.

	if (Primitive(position) == undecided) {
		PositionToBlankOX(position,theBlankOX);
		//Black's turn
		if (turn == 1) {
			if (black == 0) {
				for (i = 1; i <= 6; i++) {
					head = CreateMovelistNode(i, head);
				}
			}
			else {
				for (i = 1; i <= 3; i++) {
					head = CreateMovelistNode(i, head);
				}
			}
		}
		//White's turn
		else if (turn == 0) {
			if (white == 0) {
				for (i = 1; i <= 6; i++) {
					head = CreateMovelistNode(i, head);
				}
			}
			else {
				for (i = 1; i <= 3; i++) {
					head = CreateMovelistNode(i,head);
				}
			}
		}

		return(head);
	}

	else {
		return(NULL);
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
**              STRING playerName     : The name of the player whose turn it
   is
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
	USERINPUT ret;

	do {
		printf("%8s's move [(u)ndo/1-6] :  ", playerName);

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
** OUTPUTS:     BOOLEAN : TRUE iff the input is a valid text input.
**
************************************************************************/

BOOLEAN ValidTextInput(input)
STRING input;
{
	return (input[0] <= '6' && input[0] >= '1');

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
	return((MOVE) input[0] -'0'); // hehe, i fixed this!! (EM)
	// our game takes in input from 1-6 ONLY.
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
	STRING m = MoveToString( theMove );
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
	STRING m = (STRING) SafeMalloc( 2 );

	// Our theMove is from 1-6, because of special pieces
	sprintf( m, "%d", theMove);
	return m;
}

/*** Database Functions ***/
int NumberOfOptions() {
	return 2;
}

int getOption() {
	int option = 1;

	option += (gStandardGame ? 0 : 1);

	return option;
}

void setOption(int option) {
	option--;

	gStandardGame = (option%2==0);
}


/************************************************************************
*************************************************************************
**         EVERYTHING BELOW THESE LINES IS LOCAL TO THIS FILE
*************************************************************************
************************************************************************/

/************************************************************************
**
** NAME:        PositionToBlankOX
**
** DESCRIPTION: convert an internal position to that of a BlankOX.
**
** INPUTS:      POSITION thePos     : The position input.
**              BlankOX *theBlankOx : The converted BlankOX output array.
**
** CALLS:       BadElse()
**
************************************************************************/

void PositionToBlankOX(thePos,theBlankOX)
POSITION thePos;
BlankOX *theBlankOX;
{
	int b0,b1, b2, b3, b4, b5, b6;

	b6 = thePos % 2; // turn
	b5 = (thePos / 2) % 15; // 3rd column
	b4 = (thePos / (2*15)) % 15; //2nd column
	b3 = (thePos / (2*15*15)) % 15; //1st column
	b2 = (thePos / (2*15*15*15)) % 10; // white
	b1 = (thePos / (2*15*15*15*10)) % 10; //black
	b0 = (thePos / (2*15*15*15*10*10)) % 11; // score

	theBlankOX[0] = b0; //score
	theBlankOX[1] = b1; //black
	theBlankOX[2] = b2; //white
	theBlankOX[3] = b3; //1st column
	theBlankOX[4] = b4; //2nd column
	theBlankOX[5] = b5; //3rd column
	theBlankOX[6] = b6; // turn

}


/************************************************************************
**
** NAME:        BlankOXToPosition
**
** DESCRIPTION: convert a BlankOX to that of an internal position.
**
** INPUTS:      BlankOX *theBlankOx : The converted BlankOX output array.
**
** OUTPUTS:     POSITION: The equivalent position given the BlankOX.
**
************************************************************************/

POSITION BlankOXToPosition(theBlankOX)
BlankOX *theBlankOX;
{
	POSITION position = 0;

	int turn = theBlankOX[6];
	int white = theBlankOX[2];
	int black = theBlankOX[1];
	int score = theBlankOX[0];
	int p1 = theBlankOX[3];
	int p2 = theBlankOX[4];
	int p3 = theBlankOX[5];

	position = turn + (2*p3) + (2*15*p2) + (2*15*15*p1) + (2*15*15*15*white) +
	           (2*15*15*15*10*black) + (2*15*15*15*100*score);

	/* position = turn + (2*p9) + (2*3*p8) + (2*3*3*p7) + (2*27*p6) +
	   (2*27*3*p5) + (2*27*9*p4) +
	   (2*27*27*p3) + (2*27*27*3*p2) + (2*27*27*9*p1) + (2*27*27*27*white) +
	   (2*27*27*27*10*black) +
	   (2*27*27*27*100*score); */

	return(position);
}


/************************************************************************
**
** NAME:        ThreeInARow
**
** DESCRIPTION: Return TRUE iff there are three-in-a-row.
**
** INPUTS:      BlankOX theBlankOX[BOARDSIZE] : The BlankOX array.
**              int a,b,c                     : The 3 positions to check.
**
** OUTPUTS:     (BOOLEAN) TRUE iff there are three-in-a-row.
**
************************************************************************/

int ThreeInARow(theBlankOX,a,b,c)
BlankOX theBlankOX[];
int a,b,c;
{
	// the arguments, A,B,C passed into this function is in decreasing order.

	int blackpiece = (int)theBlankOX[1];
	int whitepiece = (int)theBlankOX[2];
	int column1 = (int)theBlankOX[3];
	int column2 = (int)theBlankOX[4];
	int column3 = (int)theBlankOX[5];

	char piece1, piece2, piece3;

	// get the pieces
	if (a == 8) {
		piece1 =  gBlankOXPosition[column3][2];
		if (b == 7) { // horizontal bottom
			piece2 = gBlankOXPosition[column2][2];
			piece3 = gBlankOXPosition[column1][2];
		}
		if (b == 5) { // vert right
			piece2 = gBlankOXPosition[column3][1];
			piece3 = gBlankOXPosition[column3][0];
		}
		if (b== 4) { // diag top->bot
			piece2 = gBlankOXPosition[column2][1];
			piece3 = gBlankOXPosition[column1][0];
		}
	}
	else if (a == 7) { // vert mid
		piece1 =  gBlankOXPosition[column2][2];
		piece2 =  gBlankOXPosition[column2][1];
		piece3 =  gBlankOXPosition[column2][0];
	}
	else if (a == 6) {
		piece1 =  gBlankOXPosition[column1][2];
		if (b == 4) { // diag bot->top
			piece2 =  gBlankOXPosition[column2][1];
			piece3 =  gBlankOXPosition[column3][0];
		}
		if (b == 3) { // vert left
			piece2 = gBlankOXPosition[column1][1];
			piece3 =  gBlankOXPosition[column1][0];
		}
	}
	else if (a == 5) { //horizontal middle
		piece1 =  gBlankOXPosition[column3][1];
		piece2 =  gBlankOXPosition[column2][1];
		piece3 =  gBlankOXPosition[column1][1];
	}
	else if (a == 2 ) { // horizontal top
		piece1 =  gBlankOXPosition[column3][0];
		piece2 =  gBlankOXPosition[column2][0];
		piece3 =  gBlankOXPosition[column1][0];
	}
	else {}

	//printf("%c %c %c<--1,2,3", piece1, piece2, piece3);

	// for x
	if ('x' == piece1) {
		if (piece1 == piece2 && piece2 == piece3) {
			if (blackpiece == (a+1) || blackpiece == (b+1) || blackpiece == (c+1)
			    ) {
				return (-2);
			}
			else {
				return (-1);
			}
		}
		else {
			return (0);
		}
	}
	// if o matches
	else if ('o' == piece1) {
		if (piece1 == piece2 && piece2 == piece3) {
			if (whitepiece == (a+1) || whitepiece == (b+1) || whitepiece == (c+1)
			    ) {
				return (2);
			}
			else {
				return (1);
			}
		}
		else {
			return (0);
		}
	}
	else {
		return (0);
	}
}

/***********************************************************************
**
** NAME:        ConvertBackToInt
**
** DESCRIPTION: Returns the integer value associated with "x" "o" "-"
**
** INPUTS:      A char* "x" "-" or "o"
**
** OUTPUTS:     A int value associated with x, Blank, o
**
***********************************************************************/
int ConvertBackToInt(char* letter)
{
	printf("In ConvertBackToInt");

	if (strcmp("x", letter) == 0) {
		return x;
	}
	else if (strcmp("-", letter) == 0) {
		return Blank;
	}
	else if (strcmp("o", letter) == 0) {
		return o;
	}

	//never reaches here
	return Blank;
}

/************************************************************************
**
** NAME:        AllFilledIn
**
** DESCRIPTION: Return TRUE iff all the blanks are filled in.
**
** INPUTS:      BlankOX theBlankOX[BOARDSIZE] : The BlankOX array.
**
** OUTPUTS:     (BOOLEAN) TRUE iff all the blanks are filled in.
**
************************************************************************/
// WE NEVER NEED TO USE THIS FUNCTION
BOOLEAN AllFilledIn(theBlankOX)
BlankOX theBlankOX[];
{
	BOOLEAN answer = TRUE;
	int i;

	for(i = 0; i < REALBOARDSIZE; i++)
		answer &= (theBlankOX[i] == o || theBlankOX[i] == x);

	return (answer);
}

/************************************************************************
**
** NAME:        WhoseTurn
**
** DESCRIPTION: Return whose turn it is - either x or o. Since x always
**              goes first, we know that if the board has an equal number
**              of x's and o's, that it's x's turn. Otherwise it's o's.
**
** INPUTS:      BlankOX theBlankOX : The input board
**
** OUTPUTS:     (BlankOX) : Either x or o, depending on whose turn it is
**
************************************************************************/

BlankOX WhoseTurn(theBlankOX)
BlankOX *theBlankOX;
{
	//printf("Inside WhoseTurn");

	if (theBlankOX[6] == 1 ) {
		return (x); // this is the REAL LINE
	}
	else {
		return (o); // this is the REAL LINE
	}
}











/*
POSITION StringToPosition(char* board) {
	// FIXME: this is just a stub
	return atoi(board);
}


char* PositionToString(POSITION pos) {
	// FIXME: this is just a stub
	return "Implement Me";
}
*/

GM_DEFINE_BLANKOX_ENUM_BOARDSTRINGS()

char * PositionToEndData(POSITION pos) {
	return NULL;
}
