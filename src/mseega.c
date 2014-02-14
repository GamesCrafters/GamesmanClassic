
// $id$
// $log$

/*
   To compile, use
   gcc -fPIC -O -DSUNOS5  -I/usr/sww/pkg/tcltk-8.4.4/include -I/usr/openwin/include -fPIC -lz -lnsl -lsocket -lm -shared -static-libgcc -L/usr/sww/pkg/tcltk-8.4.4/lib -ltk8.4 -L/usr/sww/pkg/tcltk-8.4.4/lib -ltcl8.4 -L/usr/sww/pkg/tcltk-8.4.4/lib -ltcl8.4 mseega.c
 */

//Please write down updates!!
/*
 * Above is will include the name and the log of the last
 * person to commit this file to gamesman.
 */

/************************************************************************
**
** NAME:        mseega.c
**
** DESCRIPTION: Seega
**
** AUTHORS:     Emad Salman <emadsalman@berkeley.edu>,
**              Yonathan Randolph <yonran@berkeley.edu>,
**              Peter Wu <peterwu@berkeley.edu>
**
** DATE:        Began 2004-09-29; Finished 2004-10-...
**
** UPDATE HIST: 2004-10-04 YR: Put it into CVS.
**              2004-10-11 Peter: I wrote this earlier, but I didn't know
**               know that checking-in a file would lead to a vi interface
**              2004-10-11 Added more defines, GamesSpecificMenu, PrintPosition
**	        2004-10-12 Peter, on behalf of Emad: legalMove, boardcopy, *GenerateMoves
**              **PLEASE PUT UPDATES HERE**
**
**		2006-8-21	change to GetMyChar()/GetMyInt() dmchan
**     2008-2-05   Implemented getOption(), setOption(int option), and NumberOfOption().
**     2008-2-09   Added checks for misere mode and modified options to handle the new variants introcuted by misere mode.
**
**************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/

#include <stdio.h>
#include "gamesman.h"
#include "hash.h"
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <math.h>
#define DEBUGGING 0
#define DEBUGGING2 0
#define DEBUGGING3 0 //Peter's debugging flag

extern STRING gValueString[];

POSITION gNumberOfPositions  = 27962; /* # total possible positions -> given by the hash_init() function*/
POSITION gInitialPosition    =  147972; /* The initial position (starting board) */
//POSITION gMinimalPosition    = 0 ;
POSITION kBadPosition        = -1; /* A position that will never be used */

STRING kAuthorName          = "Emad Salman, Yonathan Randolph, and Peter Wu";
STRING kGameName           = "Seega";   /* The name of your game */
STRING kDBName             = "seega";   /* The name to store the database under */
//TODO Peter: check this - this could be a partisan game
BOOLEAN kPartizan           = FALSE;  /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
//BOOLEAN  kSupportsHeuristic  = ;
//BOOLEAN  kSupportsSymmetries = ;
BOOLEAN kSupportsGraphics   = FALSE;
BOOLEAN kDebugMenu          = TRUE;  /* TRUE while debugging */
BOOLEAN kGameSpecificMenu   = TRUE;  /* TRUE if there is a game specific menu*/
BOOLEAN kTieIsPossible      = FALSE;  /* TRUE if a tie is possible */
BOOLEAN kLoopy              = TRUE;  /* TRUE if the game tree will have cycles (a rearranger style game) */
BOOLEAN kDebugDetermineValue = FALSE;  /* TRUE while debugging */
void*    gGameSpecificTclInit = NULL;


/*
   Help strings that are pretty self-explanatory
 */

STRING kHelpGraphicInterface =
        "Not written yet";

STRING kHelpTextInterface    =
        "There are two types of turns in Seega: place moves, and slide moves.\n\
***Part 1: PLACE MOVES:***\n\
First, you and your opponent will take turns placing TWO moves at a time.\n\
Use the BOARD to locate the empty slots where you can insert your 2 pieces,\n\
and type the 2 numbers corresponding to the two empty slots (i.e. [1 2])\n\
Note: Some spots, called forbidden spots, are marked as unplayable.\n\
***Part 2: SLIDE MOVES:***\n\
Next, once the board is filled up with the 2 players' pieces, we'll go into\n\
the slide moves phase. Once again, you'll type two numbers, but the first\n\
number will represent the piece to move, and the second number would be the\n\
space you would like to slide it\n\
Note: You may only do slide moves to adjacent squares connected to the\n\
current square. If at any point you have made a mistake, you can type u and\n\
hit return and the system will revert back to your most recent position."                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    ;

STRING kHelpOnYourTurn =
        "There are two types of turns in Seega: place moves, and slide moves.\n\
***Part 1: PLACE MOVES:***\n\
Each player alternates taking turns putting down pieces on the board,\n\
two at a time.\n\
***Part 2: SLIDE MOVES:***\n\
Each player alternates moving pieces to an unoccupied, adjacent squre in\n\
the horizontal or vertical directions in an attempt to capture your\n\
opponent's pieces. A capture is made when the current player makes a move\n\
such that an opponent's piece is directly sandwiched between two of the\n\
current player's pieces as a result of the current player's move. Example:\n\
x-ox                -x-x\n\
-o--   =======>     -o--\n\
-o--                -o--\n\
-x-o                -x-o\n\
x's turn to move    x moves right and captures the left o piece.\n\
Multiple captures are possible:\n\
----                ----\n\
x-ox   =======>     -x-x\n\
-o--                ----\n\
-x-o                -x-o\n\
x's turn to move    x moves right and captures two o pieces."                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        ;

STRING kHelpStandardObjective =
        "To capture the opponent's counters until the opponent only has one\n"
        "counter left.";

STRING kHelpReverseObjective =
        "To force your opponent to make moves that will eat of your pieces until\n"
        "you are left with one piece.";

STRING kHelpTieOccursWhen =   /* Should follow 'A Tie occurs when... */
                            "NOT possible in this game.";

//TODO
STRING kHelpExample =
        "  LEGEND:            TOTAL (forbidden spots don't show up):\n\
  ( 0  1  2 )        :- - -\n\
  ( 3  4  5 )        :-   -\n\
  ( 6  7  8 )        :- - -\n\
  Player 1's turn to move\n\
\n\
Player 1's move [(undo)/<number> <number>] : 0 8\n\n\
  LEGEND:            TOTAL (forbidden spots don't show up):\n\
  ( 0  1  2 )        :x - -\n\
  ( 3  4  5 )        :-   -\n\
  ( 6  7  8 )        :- - x\n\
  Player 2's turn to move\n\
\n\
Player 2's move [(undo)/<number> <number>] : 2 6\n\n\
  LEGEND:            TOTAL (forbidden spots don't show up):\n\
  ( 0  1  2 )        :x - o\n\
  ( 3  4  5 )        :-   -\n\
  ( 6  7  8 )        :o - x\n\
  Player 1's turn to move\n\
\n\
Player 1's move [(undo)/<number> <number>] : 1 5\n\n\
  LEGEND:            TOTAL (forbidden spots don't show up):\n\
  ( 0  1  2 )        :x x o\n\
  ( 3  4  5 )        :-   x\n\
  ( 6  7  8 )        :o - x\n\
  Player 2's turn to move\n\
\n\
Player 2's move [(undo)/<number> <number>] : 3 7\n\n\
  LEGEND:            TOTAL:\n\
  ( 0  1  2 )        :x x o\n\
  ( 3  4  5 )        :o - x\n\
  ( 6  7  8 )        :o o x\n\
  Player 1's turn to move\n\
\n\
Player 1's move [(undo)/<number> <number>] : 5 4\n\n\
  LEGEND:            TOTAL:\n\
  ( 0  1  2 )        :x x o\n\
  ( 3  4  5 )        :o x -\n\
  ( 6  7  8 )        :o o x\n\
  Player 2's turn to move\n\
\n\
Player 2's move [(undo)/<number> <number>] : 2 5\n\n\
  LEGEND:            TOTAL:\n\
  ( 0  1  2 )        :x x -\n\
  ( 3  4  5 )        :o - o\n\
  ( 6  7  8 )        :o o x\n\
  Player 1's turn to move\n\
\n\
Player 1's move [(undo)/<number> <number>] : 1 2\n\n\
  LEGEND:            TOTAL:\n\
  ( 0  1  2 )        :x - x\n\
  ( 3  4  5 )        :o - -\n\
  ( 6  7  8 )        :o o x\n\
  Player 2's turn to move\n\
\n\
Player 2's move [(undo)/<number> <number>] :"                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           ;

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

//Peter's note: having more than 9 rows would mess up the printf, unless we want to swap the letter and column convention
#define MAXROWS 9
#define MAXCOLS 9
#define DEFAULTROWS 3
#define DEFAULTCOLS 3
#define DEFAULTBOARDSIZE 9

//Peter: we should add a 'g' in front of global variables for good coding style
int width=DEFAULTROWS, height=DEFAULTCOLS;
int BOARDSIZE = DEFAULTBOARDSIZE;
int BOARDARRAYSIZE;
char P1='x';
char P2='o';
char blank='-'; //TODO changed Peter: I think blank should be '-'
//int BOARDSIZE = 11;
/* width*height, plus one for whose move it is and whether we are in
   placing mode.*/

typedef char* Board;
typedef int* SMove;
//array for keeping the forbidden spots - not used yet
//int forbiddenSpots[30]; //MAX 30 forbidden spots - arbitrary num - not used
int changedForbidden = -1;
int defaultForbidden = TRUE;

//FIXME remove line
//typedef enum blank_o_x {Blank, x, o} BlankOX;

int forbiddenSpot(int r);

char whoseBoard(Board b);
char getpce(Board b, int r);
char getPiece(Board b, int x, int y);
char otherPlayer(char c);

int fromWhere(SMove m); // applies only if !placingBoard(b)
int toWhere(SMove m);
int toWhere2(SMove m); // applies only if placingBoard(b)

int whoToInt(char c); // converts 'x' or 'o' to 1 or 2 for generic_hash.

// Are we in the opening phase of the game?
BOOLEAN placingBoard(Board b);
BOOLEAN fullBoard(Board b);

void setWhoseBoard(Board b, char t);
void setpce(Board b, int r, char c);
void setMove2(SMove m,int val);
void setMove(SMove m, char who, int rfrom, int rto);
void setPlacingBoard(Board b, BOOLEAN t);

void makeRandomBoard(Board b);

POSITION hash(Board b);
void unhash(Board b, POSITION p);


/* A few helper functions for GenerateMoves. */
int nextOpenSpot(Board b, int lowerBound);
int nextOpenInitSpot(Board b, int lowerBound);
int nextSpotOfType(Board b, int lowerBound, int whoseTurn);
MOVELIST *GeneratePlacingMoves(Board b);
MOVELIST *GenerateMovingMoves(Board b);


/*************************************************************************
**
** Above is where you put your #define's and your global variables, structs
**
*************************************************************************/

/*
** Function Prototypes:
*/

/* Function prototypes here. */

/* External */
#ifndef MEMWATCH
extern GENERIC_PTR      SafeMalloc ();
extern void             SafeFree ();
#endif

STRING                  MoveToString(MOVE);

/************************************************************************
**
** NAME:        InitializeGame
**
** DESCRIPTION: Initialize the gDatabase, a global variable. and the other
**              local variables.
**
************************************************************************/

void InitializeGame () {
	BOARDSIZE = width * height; //global variables
	BOARDARRAYSIZE = width*height + 2;
	int boardspec[] = {P1, 0, floor(BOARDSIZE/2)+2,
		           P2, 0, floor(BOARDSIZE/2)+1,
		           blank, 1, BOARDSIZE+1,
		           -1};
	gNumberOfPositions = generic_hash_init(BOARDARRAYSIZE, boardspec, NULL, 0);

	do { // I have this block to create new stack frame
		// TODO: find a one-line way to say the following:
		char tempname[BOARDARRAYSIZE];
		Board b = tempname;
		int r;
		for (r=0; r<width*height; r++) {
			setpce(b,r,blank);
		}
		setWhoseBoard(b, 'x');
		setPlacingBoard(b,TRUE);
		gInitialPosition = hash(b);
		//printf ("init------------- %d",gInitialPosition);
	} while (FALSE);

	gMoveToStringFunPtr = &MoveToString;
}


/************************************************************************
  **x
** NAME:        DebugMenu
**
** DESCRIPTION: Menu used to debub internal problems. Does nothing if
**              kDebugMenu == FALSE
**
************************************************************************/

void DebugMenu () {

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

void changeBoard();
void changeForbiddenSpots();

void GameSpecificMenu()
{
	char GetMyChar();

	printf("\n");
	printf("Seega Game Specific Menu\n\n");
	printf("1) Change the board size (currently at = %d x %d) \n", height, width); //rows x cols ?
	//printf("2) Change or remove the forbidden spot \n");
	printf("b) Back to previous menu\n\n");
	printf("Select an option: ");

	switch(GetMyChar()) {
	case 'Q': case 'q':
		printf("\n");
		ExitStageRight();
		break;
	case '1':
		changeBoard();
		GameSpecificMenu();
		break;
	/*
	   case '2':
	   changeForbiddenSpots();
	   GameSpecificMenu();
	   break;
	 */

	case 'b': case 'B':
		return;
	default:
		printf("\nSorry, I don't know that option. Try another.\n");
		HitAnyKeyToContinue();
		GameSpecificMenu();
		break;
	}
}

//cheesy error: floating point input doesn't work
void changeBoard()
{
	int n_rows, n_cols, valid_cols, valid_rows;
	valid_cols = 0; //a flag
	valid_rows = 0; //another flag - not used
	printf("Enter the new number of rows (3-%d):  ", MAXROWS);
	n_rows = GetMyUInt();
	if ((n_rows < 3) || (n_rows > MAXROWS)) {
		printf("Number of rows must be between to 3 and %d\n", MAXROWS);
		changeBoard(); //optional - change to better style
	} else {
		printf("Changing number of rows to %d ...\n", n_rows);
		height = n_rows;
	}
	printf("Enter the new number of columns (3-%d):  ", MAXCOLS);
	while (valid_cols == 0) {
		n_cols = GetMyUInt();
		if ((n_cols < 3) || (n_cols > MAXCOLS)) {
			printf("Number of columns must be between to 3 and %d\n", MAXCOLS);
		} else {
			printf("Changing number of columns to %d ...\n", n_cols);
			width = n_cols;
			//BOARDSIZE=width*height;
			valid_cols = 1;
		}
	}
	InitializeGame();
	//displayBoard(); //this is temporary!!
	//printf("done! \n");
	//GetMyChar();
}

void changeForbiddenSpots()
{
	int MAXNUM = 8; //TEMP
	printf("Enter the new forbidden spot (between -1 and %u),\n", MAXNUM);
	printf("-1 indicates that there are no forbidden spots:   ");
	changedForbidden = GetMyUInt();
	if ((changedForbidden < -1) || (changedForbidden > MAXNUM)) {
		printf("not a valid forbidden spot\n");
		changedForbidden = -1;
		InitializeGame();
		return;
	}
	defaultForbidden = FALSE;
	if (changedForbidden == -1) {
		printf("removing the forbidden spot\n");
	} else {
		printf("Changed the new forbidden spot to %d ...\n", changedForbidden);
	}
	InitializeGame();
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

char* getBoard(POSITION pos);
/*
   //TODO doesn't actually do any moves yet =(.. just switches players
   POSITION DoMove (POSITION thePosition, MOVE theMove) {
   int whoseTurn, nextPlayer;
   char *board;
   char ownpiece, opponentpiece;
   whoseTurn = whoseBoard(b);
   //currboard = gboard;
   if (DEBUGGING) {
   printf("Starting Do Move with input: %d\n", theMove);
   }
   if(whoseTurn == 1) {
   nextPlayer = 2;
   ownpiece = P1;
   opponentpiece = P2;
   } else {
   nextPlayer = 1;
   ownpiece = P2;
   opponentpiece = P1;
   }
   board = getBoard(thePosition); //Peter: using gboard or getBoard, I'm confused?
   return generic_hash_hash(board, nextPlayer);
   }
 */
//this will do the actual moves
POSITION DoMove (POSITION position, MOVE m) {
	// TODO: find a one-line way to say the following:
	char tempname[BOARDARRAYSIZE];
	Board b = tempname;
	unhash(b,position);
	char c = whoseBoard(b), d=otherPlayer(c);
	if (DEBUGGING) {
		int i;
		printf("Starting DoMove...\n");
		printf("Board is a %s; %c's turn (next: %c's turn). ",
		       placingBoard(b) ? "placing board" : "moving board",
		       c,d);
		for (i=0; i<BOARDSIZE; i++) printf("%c",getpce(b,i));
		printf("\n");
	}
	if (placingBoard(b)) {
		setpce(b, toWhere(&m), c);
		//setpce(b, toWhere2(&m), c);
		setWhoseBoard(b, d);
		if (fullBoard(b)) setPlacingBoard(b,FALSE);
	}
	else {
		int r = toWhere(&m);
		setpce(b, r, c);
		setpce(b, fromWhere(&m), '-');
		if (r-2*width>=0 &&
		    getpce(b, r-2*width)==c &&
		    getpce(b, r-width)==d)
			setpce(b, r-width, '-');
		if (r+2*width<width*height &&
		    getpce(b, r+2*width)==c &&
		    getpce(b, r+width)==d)
			setpce(b, r+width, '-');
		if (r%width+2<width &&
		    getpce(b, r+2)==c &&
		    getpce(b, r+1)==d)
			setpce(b, r+1, '-');
		if (r%width-2>=0 &&
		    getpce(b, r-2)==c &&
		    getpce(b, r-1)==d)
			setpce(b, r-1, '-');
		setWhoseBoard(b, d);
	}
	if (DEBUGGING) { for (c=0; c<BOARDARRAYSIZE; c++) printf("%c",getpce(b,c));
		         unhash(b,hash(b));
		         for (c=0; c<BOARDARRAYSIZE; c++) printf("%c",getpce(b,c)); }
	return hash(b);
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

//TODO change - default for now
POSITION GetInitialPosition ()
{
	// TODO: find a one-line way to say the following:
	//char tempname[BOARDARRAYSIZE];
	//Board b = tempname;

	//printf("\n\n\t----- Get Initial Position -----\n");
	//printf("\n\tPlease input the position to begin with.\n");
	//printf("\nUse x for left player, o for right player, and _ for blank spaces\n");
	//printf("Example:\n");

	//printf("\nTEMP: On second thought let me just give you a random board.\n");
	//printf("TODO: allow you to enter something.\n");
	//setWhoseBoard(b, 'x');
	//makeRandomBoard(b);
	//InitializeGame();
	//printf("d",);
	//gInitialPosition;
	return gInitialPosition;
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

//TODO not tested
VALUE Primitive (POSITION position) {
	int r, c, flag=0;
	int num_pieces = 0;
	// TODO: find a one-line way to say the following:
	char tempname[BOARDARRAYSIZE];
	Board b = tempname;
	unhash(b,position);
	c=whoseBoard(b);
	//c=generic_hash_turn();
	//c==P1?d=P2:d=P1;
	MOVELIST* moves = GenerateMoves(position);
	FreeMoveList(moves); //what does this do?
	for (r=0; r<width*height; r++) {
		if(getpce(b,r)==c) {
			num_pieces++;
		}
	}
	if ((num_pieces == 1) && (!placingBoard(b)))  {
		return (gStandardGame ? lose : win);
	}

	if (moves==NULL) {
		for (r=0; r<width*height; r++) {
			if(getpce(b,r)==c) {
				flag=1;
				break;
			}
		}
		if (flag != 1) { //THIS WILL NEVER BE REACHED
			//FreeMoveList(moves);
			return (gStandardGame ? lose : win);
		}
		//printf("TODO: implement do again\n");
		//printf("the current player is trapped, with numPieces %d\n", num_pieces);
		return (gStandardGame ? win : lose);
	} else {
		for (r=0; r<width*height; r++) {
			if(getpce(b,r)==c) {
				num_pieces++;
			}
		}
		/*
		   if ((num_pieces == 1) && (!placingBoard(b)))  {
		   return lose;
		   }
		 */
	}
	//else FreeMoveList(moves);
	// A board is primitive if there are no more things to move.
	//for (r=0;r<width*height; r++)
	//if (getpce(b,r)!=c)
	return undecided;
	//return lose;
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

//this will be our print position function - prints for a general sized board
//now we only have to grab the current values of the board!
//TODO: take into account the arguments
//TODO: get predicition here
void PrintPosition (POSITION position, STRING playerName, BOOLEAN usersTurn)
{
	int index=0;
	int currRow;
	char currCol;
	char temp[BOARDARRAYSIZE];
	Board b=temp;
	int moveCounter;
	moveCounter = 0;
	unhash(b,position);
	//generic_hash_unhash(position,b);
	printf("\n");
	printf("  LEGEND:");
	for (currCol = 0; currCol < width; currCol++) {
		printf("   ");
	}
	if (placingBoard(b)) {
		printf("   TOTAL (forbidden spots don't show up):\n");
	} else {
		printf("   TOTAL:\n");
	}
	for (currRow = height; currRow>0; currRow--) {
		//printf("    %d ( ", currRow);
		printf("  (");
		for (currCol = 0; currCol < width; currCol++) {
			//printf("%c%d ", alphabet[currCol], currRow);
			if (moveCounter >= 10) {
				printf(" %d", moveCounter);
			} else {
				printf(" %d ", moveCounter);
			}
			moveCounter++;
		}
		printf(")        :");
		for (currCol = 0; currCol < width; currCol++) {
			if (forbiddenSpot(index) && placingBoard(b)) {
				printf("  ");
			} else {
				printf("%c ", getpce(b,index)); //get piece
			}
			index++;
		}
		printf("\n");
	}
	/*
	   printf("        ");
	   for (currCol = 0; currCol < width; currCol++) {
	   printf("%c  ", alphabet[currCol]);
	   }
	 */
	printf("  %s's turn to move\n\n", playerName);
	//TODO notify square as unplacable
}

/*
   void PrintPosition (POSITION position, STRING playerName, BOOLEAN usersTurn)
   {
   int index=0;
   int currRow;
   char currCol;
   char alphabet[]="abcdefghijklmnopqrstuvwxyz";
   char temp[BOARDARRAYSIZE];
   Board b=temp;
   int moveCounter;
   moveCounter = 0;
   unhash(b,p);
   printf("\n");
   printf("         LEGEND:");
   for (currCol = 0; currCol < width; currCol++) {
   printf("   ");
   }
   printf("       TOTAL:\n");
   for (currRow = height; currRow>0; currRow--) {
   printf("    %d ( ", currRow);
   for (currCol = 0; currCol < width; currCol++) {
   printf("%c%d ", alphabet[currCol], currRow);
   }
   printf(")          :");
   for (currCol = 0; currCol < width; currCol++) {
   printf("%c ", getpce(b,index));
   index++;
   }
   printf("\n");
   }
   printf("        ");
   for (currCol = 0; currCol < width; currCol++) {
   printf("%c  ", alphabet[currCol]);
   }
   printf("\n\n");
   }
 */


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
** CALLS:       GENERIC_PTR SafeMalloc(int)
**              LIST OTHER CALLS HERE
**
************************************************************************/
MOVELIST *GenerateMoves (POSITION position)
{
	char gBoard[BOARDARRAYSIZE];
	/* Use CreateMovelistNode(move, next) to 'cons' together a linked list */
	char mover;
	int player,i;
	// void boardcopy();
	int legalMove();
	int isoddbrd;
	MOVE m;
	MOVELIST *head = NULL;
	MOVELIST *CreateMovelistNode();
	unhash(gBoard,position);
	player=generic_hash_turn(position);
	if(player == 1)
		mover=P1;
	else
	if(player == 2)
		mover=P2;

	if (placingBoard(gBoard)) {
		isoddbrd=BOARDSIZE%2;
		for (i=0; i<BOARDSIZE; i++) {
			//   if (!isoddbrd && i<4)
			//continue;
			//else
			if(i==BOARDSIZE/2)
				continue;
			if(gBoard[i]==blank) {
				setMove2(&m,i);
				head=CreateMovelistNode(m,head);
			}
		}
	}


	else{

		for (i=0; i<BOARDSIZE; i++) {
			if(gBoard[i]==mover) {
				if (legalMove(i,i+1) && gBoard[i+1]== blank) {
					setMove(&m,mover,i,i+1);
					head=CreateMovelistNode(m,head);
				}
				if (legalMove(i,i-1) && gBoard[i-1]== blank) {
					setMove(&m,mover,i,i-1);
					head=CreateMovelistNode(m,head);
				}
				if (legalMove(i,i+width) && gBoard[i+width]== blank) {
					setMove(&m,mover,i,i+width);
					head=CreateMovelistNode(m,head);
				}
				if (legalMove(i,i-width) && gBoard[i-width]== blank) {
					setMove(&m,mover,i,i-width);
					head=CreateMovelistNode(m,head);
				}
			}
		}
	}
	return head;
}





/*MOVELIST *GenerateMoves (POSITION position) {
   // TODO: say this in one line instead of 2:
   char tempname[BOARDARRAYSIZE];
   Board b = tempname;
   generic_hash_unhash(position, b);
   if (placingBoard(b))
   return GeneratePlacingMoves(b);
   else
   return GenerateMovingMoves(b);
   }*/


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

USERINPUT GetAndPrintPlayersMove (POSITION position, MOVE *move, STRING playersName)
{
	USERINPUT input;
	USERINPUT HandleDefaultTextInput();

	BOOLEAN ValidMove();

	do {
		printf("%8s's move [(undo)/<number> <number>] : ", playersName);

		input = HandleDefaultTextInput(position, move, playersName);
		if(input != Continue)
			return(input);

	}
	while (TRUE);

	/*
	   for (;;) {
	   printf("%8s's move [(undo)/<number> <number>] : ", playersName);

	   input = HandleDefaultTextInput(position, move, playersName);

	   if (input != Continue)
	    return input;
	   }
	 */

	/* NOTREACHED */
	return Continue;
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

	return(input[0] <= '9' && input[0] >= '0');
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

// some parts from mabalone.c
MOVE ConvertTextInputToMove (STRING input) {
	if (DEBUGGING2)
		printf("Starting conversion\n");
	int n=0, p1=0, p2=0;
	MOVE m;

	// skip whitespace etc.
	for (; input[n]!='\0' && (input[n]<'0' || input[n]>'9'); n++) ;
	for (; input[n]!='\0' && input[n] >= '0' && input[n] <= '9'; n++)
		p1 = p1*10 + input[n] - '0';
	for (; input[n]!='\0' && (input[n]<'0' || input[n]>'9'); n++) ;
	for (; input[n]!='\0' && input[n] >= '0' && input[n] <= '9'; n++)
		p2 = p2*10 + input[n] - '0';
	if (p2==0)
		setMove2(&m,p1);
	else
		setMove(&m, 'x', p1, p2);
	if (DEBUGGING2) {printf("Conversion finds "); PrintMove(m); printf(".\n"); }
	return m;
	//get first piece
	//formula: result = letter + number
	//letters: a = 100, b = 200, c = 300, etc.
	/*
	   if ((input[n] >= 'a') && (input[n] <= 'z')) {
	   //calculate offset!
	   p1 = input[n] - '0';
	   n++;
	   if ((input[n] >= '0') && (input[n] <= '9')) {
	   p1 = p1 + (input[n] - '0'); //adds value
	   n++;
	   }
	   p1--; //don't ask me what this is
	   }
	 */

	/*
	   if ((input[n] >= '0') && (input[n] <= '9')) {
	   p1 = input[n] - '0';
	   n++;
	   if ((input[n] >= '0') && (input[n] <= '9')) {
	   p1 = p1 * 10 + (input[n] - '0');
	   n++;
	   }
	   p1--;
	   }
	 */


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

/* Converts the number to the specified base and puts this into the
   array. Returns the number of digits that it used. */
/*
   void numberInBase(char* out, unsigned int n, int b, char* lookuptable) {
   int i,j;
   char swap;
   out[0]='\0';
   for (i=1; n!=0; i++) {
   out[i]=lookuptable[n%b];
   n = n/b;
   }
   if (i==1) out[i++]=lookuptable[0]; // if the number was 0, print 0.
   i--;
   for (j=0;i>j;i--,j++) {
   swap=out[i];
   out[i]=out[j];
   out[j]=swap;
   }
   }
   void PrintMove (MOVE move) {
   char alphabet[]="abcdefghijklmnopqrstuvwxyz";
   char digits[]="0123456789";
   // TODO: change this to log(maxnum)/log(base), so that we are
   //   not restricted to boards of size 10^4 * 10^4
   char row1[5],col1[5]; // numbers from 0 to base
   char row2[5],col2[5];
   numberInBase(row1, fromWhere(&move)/width, 26, alphabet);
   numberInBase(col1, fromWhere(&move)%width, 10, digits);
   numberInBase(row2, toWhere(&move)/width, 26, alphabet);
   numberInBase(col2, toWhere(&move)%width, 10, digits);
   printf("Move with coordinates %s%s and %s%s.",row1,col1,row2,col2);
   printf("%d",move);
   }
 */
void PrintMove(MOVE move) {
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

STRING MoveToString (move)
MOVE move;
{
	STRING m = (STRING) SafeMalloc( 5 );

	sprintf(m, "[%d]", toWhere(&move));

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

int NumberOfOptions () {
	return 98;
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
	return (gStandardGame ? (7*(height-3)+(width-2)) : (49 + (7*(height-3)+(width-2))));
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
	int row;
	int col;
	if(option >= 50) {
		gStandardGame = 0;
		option -= 49;
	}
	row = ceil(option/7.0)+2;
	if(!(option%7)) {
		col = 9;
	}else{
		col = option%7+2;
	}
	height = row;
	width = col;
	return;
}


/************************************************************************
*************************************************************************
**         EVERYTHING BELOW THESE LINES IS LOCAL TO THIS FILE
*************************************************************************
************************************************************************/

/************************************************************************
*************************************************************************
**         EVERYTHING BELOW THESE LINES IS LOCAL TO THIS FILE
*************************************************************************
************************************************************************/

char whoseBoard(Board b) {
	return b[width*height+1];
}
char getpce(Board b, int r) {
	return b[r];
}
char getPiece(Board b, int x, int y) {
	return getpce(b,x*width+y);
}
char otherPlayer(char c) {
	return c=='x' ? 'o' : 'x';
}

POSITION hash(Board b) {
	return generic_hash_hash(b,whoToInt(whoseBoard(b)));
}
void unhash(Board b, POSITION p) {
	generic_hash_unhash(p, b);
	setWhoseBoard(b, generic_hash_turn(p)==1 ? 'x' : 'o');
}

int fromWhere(SMove m) { // applies only if !placingBoard(b)
	return (*m>>(8*sizeof(MOVE)/2)) & ((1<<8*sizeof(MOVE)/2)-1);
}
int toWhere(SMove m) {
	return *m & ((1<<8*sizeof(MOVE)/2)-1);
}
int toWhere2(SMove m) { // applies only if placingBoard(b)
	return (*m>>(8*sizeof(MOVE)/2)) & ((1<<8*sizeof(MOVE)/2)-1);
}

int whoToInt(char c) {
	// converts 'x' or 'o' to 1 or 2 for generic_hash.
	if (c!='x' && c!='o')
		BadElse("whoToInt got something not 'x' or 'o'.");
	return c=='x' ? 1 : 2;
}

BOOLEAN placingBoard(Board b) {
	return b[width*height]==P1;
}

void setWhoseBoard(Board b, char t) {
	b[width*height+1]=t;
}
void setpce(Board b, int r, char c) {
	b[r]=c;
}
void setMove2(SMove m, int val){
	*m=val;
}
void setMove(SMove m, char who, int rfrom, int rto) {
	/* TODO: figure out how we're supposed to encode invariants.
	   if ((unsigned int)rfrom >= 1<<8*sizeof(MOVE)/2 ||
	   (unsigned int)rto >= 1<<8*sizeof(MOVE)/2)
	   error("Moves are too big to store.");
	 */
	*m = rfrom<<8*sizeof(MOVE)/2 | rto;
}
void setPlacingBoard(Board b, BOOLEAN t) {
	b[width*height]=t ? P1 : blank;
}



BOOLEAN fullBoard(Board b) {
	int r;
	for (r=0; r<width*height; r++)
		if (!forbiddenSpot(r) && getpce(b,r)=='-')
			return FALSE;
	return TRUE;
}

/* YR: These are the functions I made before. They don't work yet
   probably.  */
int nextOpenSpot(Board b, int lowerBound) {
	for (; lowerBound<width*height; lowerBound++)
		if (!getpce(b, lowerBound))
			return lowerBound;
	return -1;
}
int nextOpenInitSpot(Board b, int lowerBound) {
	for (; lowerBound<width*height; lowerBound++)
		if (!forbiddenSpot(lowerBound) && !getpce(b, lowerBound))
			return lowerBound;
	return -1;
}
int nextSpotOfType(Board b, int lowerBound, int whoseTurn) {
	for (; lowerBound<width*height; lowerBound++)
		if (getpce(b, lowerBound)==whoseTurn)
			return lowerBound;
	return -1;
}
MOVELIST *GeneratePlacingMoves(Board b) {
	MOVELIST *CreateMovelistNode(), *head = NULL;
	char c = whoseBoard(b);
	int i,move=0;
	for (i=nextOpenInitSpot(b,0);
	     i!=-1;
	     i=nextOpenInitSpot(b, i+1))
		if (i-width>=0 &&
		    getpce(b, i-width)=='-') {
			setMove(&move, c, i, i-width);
			head = CreateMovelistNode(move, head);
		}
	if (i+width<width*height &&
	    getpce(b, i-width)=='-') {
		setMove(&move, c, i, i+width);
		head = CreateMovelistNode(move, head);
	}
	if (i%width+1<width &&
	    getpce(b, i+1)=='-') {
		setMove(&move, c, i, i+1);
		head = CreateMovelistNode(move, head);
	}
	if (i%width-1>=0 &&
	    getpce(b, i-1)=='-') {
		setMove(&move, c, i, i-1);
		head = CreateMovelistNode(move, head);
	}
	return head;
}
MOVELIST *GenerateMovingMoves(Board b) {
	MOVELIST *CreateMovelistNode(), *head = NULL;
	char c = whoseBoard(b);
	int i,j,move=0;
	for (i=nextSpotOfType(b,0,c);
	     i!=-1;
	     i=nextSpotOfType(b, i+1,c)) {
		for (j=nextOpenSpot(b, 0);
		     j!=-1;
		     j=nextOpenSpot(b, j+1)) {
			setMove(&move, c, i, j);
			head = CreateMovelistNode(move, head);
		}
	}
	return head;
}

void makeRandomBoard(Board b) {
	int r, r2;
	char c = 'x';
	for (r=0; r<width*height; r++)
		if (!forbiddenSpot(r)) {
			setpce(b,r,c);
			c = otherPlayer(c);
		}
		else setpce(b,r,'-');
	for (r=0; r<width*height; r++) {
		if (forbiddenSpot(r)) continue;
		do { // find a random nonforbidden spot to swap with
			r2= r + (int)((double)rand()/(RAND_MAX+1.0)*(width*height-r));
			/*TODO: find out how to do error checking.
			   if (r2<0 || r2>=width*height)
			   error("I didn't set up my random generation right.");
			 */
		} while (forbiddenSpot(r2));
		c=getpce(b,r);
		setpce(b,r,getpce(b,r2));
		setpce(b,r2,c);
	}
	setPlacingBoard(b,FALSE);
	setWhoseBoard(b,'x');
}


/************************************************************************
** This is where you can put any helper functions, including your
** hash and unhash functions if you are not using one of the existing
** ones.
************************************************************************/
int legalMove(int from, int to)
{
	if (to>=0 && to<BOARDSIZE) {
		if ( abs(to-from)== 1 && floor (to/width)!= floor(from/width))
			return 0;
		return 1;
	}
	else
		return 0;
}

void boardcopy(char *from,char*to){
	int i;
	for (i=0; i<BOARDSIZE; i++) {
		to[i]=from[i];
	}
}

//FORBIDDEN SPOTS
/* tells whether r is a forbidden spot for placing pieces (i.e. the
   center) */
int forbiddenSpot(int r) {
	//printf("FORBIDDEN spot check r[%u] height[%u] width[%u]\n", r, height, width);
	if (defaultForbidden) {
		return height/2 == r/width && width/2 == r%width;
	} else {
		/*
		   for (i = 0;  forbiddenSpots[i] != 0; i++) {
		   if(r == forbiddenSpots[i]) {
		   return TRUE;
		   }
		   }
		 */
		if (r==changedForbidden) {
			return TRUE;
		} else {
			return FALSE;
		}
	}
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
