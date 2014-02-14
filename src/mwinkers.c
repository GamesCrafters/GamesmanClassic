// $id$
// $log$

/* Above is will include the name and the log of the last
 * person to commit this file to gamesman.
 */

/************************************************************************
**
** NAME:        mwinkers.c
**
** DESCRIPTION: Winkers
**
** AUTHORS:     Newton Le, Edward Li - University of California Berkeley
**
**
** DATE:        02/26/2004
**
** UPDATE HIST:
**      Highest height 1 game solved: width 4
**      Highest height 2 game solved: width 1
**	8-21-06 fflush(stdin) should no longer be needed - dmchan
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

extern STRING gValueString[];

POSITION gNumberOfPositions; /* The number of total possible positions | If you are using our hash, this is given by the hash_init() function*/

POSITION gInitialPosition; /* The initial position (starting board) */
POSITION gMinimalPosition; /* */
POSITION kBadPosition        = -1; /* A position that will never be used */

STRING kGameName           = "Winkers";   /* The name of your game */
STRING kDBName             = "winkers";   /* The name to store the database under */
STRING kAuthorName         = "Newton Le and Edward Li";
BOOLEAN kPartizan           = TRUE;  /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN kDebugMenu          = FALSE;  /* TRUE while debugging */
BOOLEAN kGameSpecificMenu   = TRUE;  /* TRUE if there is a game specific menu*/
BOOLEAN kTieIsPossible      = TRUE;  /* TRUE if a tie is possible */
BOOLEAN kLoopy               = FALSE;  /* TRUE if the game tree will have cycles (a rearranger style game) */
BOOLEAN kDebugDetermineValue = FALSE;  /* TRUE while debugging */
void*    gGameSpecificTclInit = NULL;

/*
   Help strings that are pretty self-explanatory
 */

STRING kHelpGraphicInterface =
        "Not written yet";

STRING kHelpTextInterface    =
        "On your turn, use the LEGEND to determine which number you want to make\n\
a move on. You can choose any number that doesn't have a winker on it. A\n\
blank position will mean you place a checker, and a checker position will\n\
mean you place your color winker."                                                                                                                                                                                                                                             ;

STRING kHelpOnYourTurn =
        "You place either a checker onto an empty position, or a winker onto\n\
a checker as long as you have a piece of that type. If you can't make a\n\
move with the pieces you have, you will pass."                                                                                                                                                           ;

STRING kHelpStandardObjective =
        "To get three of your winkers in a row, horizontally or diagonally.\n\
Three-in-a-row WINS."                                                                               ;

STRING kHelpReverseObjective =
        "To force your opponent into getting three winker in a row of their color\n\
horizontally or diagonally. Three-in-a-row LOSES."                                                                                     ;

STRING kHelpTieOccursWhen =   /* Should follow 'A Tie occurs when... */
                            "the entire board has been filled with winkers without either player\n\
getting three-in-a-row."                                                                                                    ;

STRING kHelpExample =
        "            1   2   3     :  - - - \n\
 LEGEND:  4   5   6   7   : - - - - \n\
            8   9  10     :  - - - \n\
\n\
Player pieces left (O: 5, R: 5)\n\
\n\
\n\
  Player's move [1-10] : \n\
            1   2   3     :  O - - \n\
 LEGEND:  4   5   6   7   : - - - - \n\
            8   9  10     :  - - - \n\
\n\
Computer pieces left (O: 5, B: 5)\n\
\n\
\n\
Computer's move : 10\n\
            1   2   3     :  O - - \n\
 LEGEND:  4   5   6   7   : - - - - \n\
            8   9  10     :  - - O \n\
\n\
Player pieces left (O: 4, R: 5)\n\
\n\
\n\
  Player's move [1-10] : \n\
            1   2   3     :  O - - \n\
 LEGEND:  4   5   6   7   : O - - - \n\
            8   9  10     :  - - O \n\
\n\
Computer pieces left (O: 4, B: 5)\n\
\n\
\n\
Computer's move : 6\n\
            1   2   3     :  O - - \n\
 LEGEND:  4   5   6   7   : O - O - \n\
            8   9  10     :  - - O \n\
\n\
Player pieces left (O: 3, R: 5)\n\
\n\
\n\
  Player's move [1-10] : \n\
            1   2   3     :  O - - \n\
 LEGEND:  4   5   6   7   : O - O - \n\
            8   9  10     :  O - O \n\
\n\
Computer pieces left (O: 3, B: 5)\n\
\n\
\n\
Computer's move : 5\n\
            1   2   3     :  O - - \n\
 LEGEND:  4   5   6   7   : O O O - \n\
            8   9  10     :  O - O \n\
\n\
Player pieces left (O: 2, R: 5)\n\
\n\
\n\
  Player's move [1-10] : \n\
            1   2   3     :  R - - \n\
 LEGEND:  4   5   6   7   : O O O - \n\
            8   9  10     :  O - O \n\
\n\
Computer pieces left (O: 2, B: 5)\n\
\n\
\n\
Computer's move : 6\n\
            1   2   3     :  R - - \n\
 LEGEND:  4   5   6   7   : O O B - \n\
            8   9  10     :  O - O \n\
\n\
Player pieces left (O: 2, R: 4)\n\
\n\
\n\
  Player's move [1-10] : \n\
            1   2   3     :  R - - \n\
 LEGEND:  4   5   6   7   : O O B - \n\
            8   9  10     :  O O O \n\
\n\
Computer pieces left (O: 2, B: 4)\n\
\n\
\n\
Computer's move : 5\n\
            1   2   3     :  R - - \n\
 LEGEND:  4   5   6   7   : O B B - \n\
            8   9  10     :  O O O \n\
\n\
Player pieces left (O: 1, R: 4)\n\
\n\
\n\
  Player's move [1-10] : 2\n\
            1   2   3     :  R O - \n\
 LEGEND:  4   5   6   7   : O B B - \n\
            8   9  10     :  O O O \n\
\n\
Computer pieces left (O: 2, B: 3)\n\
\n\
\n\
Computer's move : 4\n\
            1   2   3     :  R O - \n\
 LEGEND:  4   5   6   7   : B B B - \n\
            8   9  10     :  O O O \n\
\n\
Player pieces left (O: 0, R: 4)\n\
\n\
\n\
Computer wins.  Nice try player.\n"                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                ;

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

#define PASSMOVE 0
#define MAXHEIGHT 3
#define MAXWIDTH 5
#define EMPTYSPACE '-'

int BOARDWIDTH = 3;
int BOARDHEIGHT = 1;
int BOARDSIZE = 11;
BOOLEAN DEFAULT = TRUE;

typedef enum possibleBoardPieces {
	Blank, O, R, B
} BlankORB;

char *gBlankORBString[] = { "-", "O", "R", "B" };

char *gBoard;
// char *LegendKey;

int *CP, *RW, *RN;

/*typedef struct moveValuesStruct {
 *  BlankORB piece;
 *  int moveIndex;
 *  int dummy;
 *} moveValues;
 *
 * typedef moveValues *moveValuesPtr;
 */

/*************************************************************************
**
** Below is where you put your #define's and your global variables, structs
**
*************************************************************************/

/*************************************************************************
**
** Above is where you put your #define's and your global variables, structs
**
*************************************************************************/

/*
** Function Prototypes:
*/

/* Function prototypes here. */

int moveUnhash_index(MOVE);
int moveUnhash_dummy(MOVE);
BlankORB moveUnhash_piece(MOVE);
int ConvertToNumber(char*);
char MoveToCharacter(MOVE);
void PrintSpaces(int n);
int Legend (int n);
BOOLEAN passMoveOnly (POSITION, MOVE);

MOVE moveHash (int, int, BlankORB);
int RowNumber(int i);
int ColPosition(int i);
int RowWidth(int i);

STRING MoveToString( MOVE );

/* External */
#ifndef MEMWATCH
extern GENERIC_PTR      SafeMalloc ();
extern void             SafeFree ();
#endif
extern BOOLEAN (*gGoAgain)(POSITION, MOVE);

/************************************************************************
**
** NAME:        InitializeGame
**
** DESCRIPTION: Initialize the gDatabase, a global variable. and the other
**              local variables.
**
************************************************************************/

void InitializeGame ()
{
	gGoAgain = passMoveOnly;

	int i;
	BOARDSIZE = BOARDHEIGHT * (2 * BOARDWIDTH + BOARDHEIGHT) + BOARDWIDTH;

	gBoard = (char *) SafeMalloc (BOARDSIZE * sizeof(char));
	//  LegendKey = (char *) SafeMalloc (BOARDSIZE * sizeof(char));
	RN = (int *) SafeMalloc (BOARDSIZE * sizeof(int));
	RW = (int *) SafeMalloc (BOARDSIZE * sizeof(int));
	CP = (int *) SafeMalloc (BOARDSIZE * sizeof(int));

	//  int x;
	//  for (x = 0; x < BOARDSIZE; x++) {
	//   LegendKey[x] = Legend(x+1);
	// }

	int half = (BOARDSIZE + 1) / 2;

	int piece_array[] = {'B', 0, half,
		             'R', 0, half,
		             'O', 0, BOARDSIZE,
		             EMPTYSPACE, 0, BOARDSIZE,
		             -1};

	gNumberOfPositions = generic_hash_init(BOARDSIZE, piece_array, 0, 0);

	if (DEFAULT == TRUE) {
		for (i=0; i<BOARDSIZE; i++)
			gBoard[i] = EMPTYSPACE;

		gInitialPosition = generic_hash_hash(gBoard, 1);
	}

	gMinimalPosition = gInitialPosition;

	//  printf("Number of Boards: %d", gNumberOfPositions);

	for (i = 0; i < BOARDSIZE; i++) {
		RN[i] = RowNumber(i);
		CP[i] = ColPosition(i);
		RW[i] = RowWidth(i);
	}

	gMoveToStringFunPtr = &MoveToString;
}


/************************************************************************
**
** NAME:        DebugMenu
**
** DESCRIPTION: Menu used to debub internal problems. Does nothing if
**              kDebugMenu == FALSE
**
************************************************************************/

void DebugMenu ()
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

void GameSpecificMenu ()
{
	char GetMyChar();
	void GetDimensions();

	do {
		/*fflush(stdin);*/

		printf("\n\t----- Game-specific options for %s -----\n\n", kGameName);

		printf("\td)\t Change the (D)imensions of the board.  Currently: (%d,%d)\n", BOARDWIDTH, BOARDHEIGHT+1);

		/*
		   if (MISERE == 0)
		   printf("\tm)\t Toggle from Standard to (M)isere\n");
		   else
		   printf("\tm)\t Toggle from (M)isere to Standard\n");
		 */
		printf("\ti)\t Change the (I)nitial Position.\n");
		printf("\tb)\t (B)ack to the previous menu\n\nSelect an option:  ");


		switch(GetMyChar()) {
		case 'Q': case 'q':
			ExitStageRight();
		case 'H': case 'h':
			HelpMenus();
			break;
		case 'I': case 'i':
			gInitialPosition = GetInitialPosition();
			break;
		case 'B': case 'b':
			return;
		case 'D': case 'd':
			GetDimensions();
			break;
		default:
			BadMenuChoice();
			HitAnyKeyToContinue();
			break;
		}
	} while(TRUE);
}


void GetDimensions() {

	char c;
	char GetMyChar();
	void printBoard(char*);
	int height, width;

	do {
		/*fflush(stdin);*/
		printf("\nEnter the horizontal dimension or get (h)elp:  ");

		if ((c = GetMyChar()) == 'h' || c == 'H') {
			printf("\nThe horizontal dimension is the number of pieces on the first row.  For example,");
			printf("\nthe current board has a horizontal dimension of %d: \n", BOARDWIDTH);
			printf("The maximum horizontal dimension is %d\n", MAXWIDTH);
			printBoard(NULL);
		} else {
			char maximumWChar = MAXWIDTH + '0' + 1;
			if (c >= '1' && c <= maximumWChar) {
				width = c - '0';
				break;
			} else {
				printf("\nInvalid input.");
			}
		}
	} while(TRUE);

	do {
		/*fflush(stdin);*/
		printf("\nEnter the vertical dimension or get (h)elp:  ");
		if ((c = GetMyChar()) == 'h' || c == 'H') {
			printf("\nThe vertical dimension is the number of pieces from the top row to the middle");
			printf("\nrow.  For example, the current board has a vertical dimension of %d: \n", BOARDHEIGHT+1);
			printf("The maximum vertical dimension is %d\n", MAXHEIGHT + 1);
			printBoard(NULL);
		} else {
			char maximumHChar = MAXHEIGHT + '0' + 1;
			if (c > '0' && c <= maximumHChar) {
				height = c - '0' - 1;
				break;
			} else {
				printf("\nInvalid input.");
			}
		}
	} while(TRUE);

	BOARDHEIGHT = height;
	BOARDWIDTH = width;
	DEFAULT = TRUE;
	InitializeGame();

	printf("\n The current board is now: \n");
	printBoard(NULL);
}


// =======================80 CHARS=================================================

void printBoard (char * board) {
	int i, j, m = 0;

	printf("\n");

	for (i = 0; i < 2*BOARDHEIGHT+1; i++) {
		PrintSpaces (abs(BOARDHEIGHT - i));

		for (j = 0; j < BOARDWIDTH + BOARDHEIGHT - abs(BOARDHEIGHT - i); j++)
			if (board == NULL)
				printf("O ");
			else
				printf("%c ", board[m++]);

		printf("\n");
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

void SetTclCGameSpecificOptions (options)
int options[];
{
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
POSITION DoMove (thePosition, theMove)
POSITION thePosition;
MOVE theMove;
{
	int player = generic_hash_turn(thePosition);

	generic_hash_unhash(thePosition, gBoard);

	if (theMove != PASSMOVE) {
		if (gBoard[theMove-1] == EMPTYSPACE)
			gBoard[theMove-1] = 'O';
		else if (player == 1)
			gBoard[theMove-1] = 'R';
		else
			gBoard[theMove-1] = 'B';
	}

	POSITION next = generic_hash_hash(gBoard,(player==1) ? 2 : 1);

	if (GenerateMoves(next) == NULL)
		return generic_hash_hash(gBoard, player);
	else
		return next;
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
	/*  BlankORB theBlankORB[BOARDSIZE];*/
	void printBoard();
	signed char c;
	int i;

	printf("\n\n\t----- Get Initial Position -----\n");
	printf("\n\tPlease input the position to begin with.\n");
	printf("\tNote that it should be in the following format:\n\n");
	printf("Example:\n");

	for (i = 0; i < BOARDSIZE; i++) {
		if ((i%4) == 0)
			gBoard[i] = '-';
		else if ((i%4) == 1)
			gBoard[i] = 'O';
		else if ((i%4) == 2)
			gBoard[i] = 'R';
		else
			gBoard[i] = 'B';
	}

	printBoard(gBoard);
	printf("\n");

	// printf("  - -\n O - -\n- R - -         <----- EXAMPLE \n - R O\n  - -\n\n");

	i = 0;
	getchar();
	while(i < BOARDSIZE && (c = getchar()) != EOF) {
		if(c == 'r' || c == 'R')
			gBoard[i++] = 'R';
		else if(c == 'o' || c == 'O' || c == '0')
			gBoard[i++] = 'O';
		else if(c == 'b' || c == 'B')
			gBoard[i++] = 'B';
		else if(c == '-')
			gBoard[i++] = EMPTYSPACE;
		else
			; /* do nothing */
	}
	DEFAULT = FALSE;
	return(generic_hash_hash(gBoard, 1));
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

void PrintComputersMove(computersMove, computersName)
MOVE computersMove;
STRING computersName;
{
	printf("%8s's move : ", computersName);
	if (computersMove == PASSMOVE)
		printf("pass");
	else
		printf("%d", computersMove);
	//    printf("%c", LegendKey[computersMove-1]);
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

VALUE Primitive (pos)
POSITION pos;
{
	BOOLEAN AllFilledIn();
	char ThreeInARow();
	/*  BlankORB theBlankORB[BOARDSIZE];*/
	VALUE EndGame(char, int);
	generic_hash_unhash(pos, gBoard);
	char current;

	int i;
	for (i = 0; i < BOARDSIZE; i++) {
		if (CP[i] < RW[i] - 2) {
			current = ThreeInARow(gBoard, i, i+1, i+2);
			if (current != EMPTYSPACE)
				return EndGame(current, generic_hash_turn(pos));
		}

		if (RN[i] < BOARDHEIGHT - 1) {
			current = ThreeInARow(gBoard, i, i + RW[i], i + 2*RW[i] + 1);
			if (current != EMPTYSPACE)
				return EndGame(current, generic_hash_turn(pos));
		} else if (RN[i] == BOARDHEIGHT - 1 && CP[i] != 0) {
			current = ThreeInARow(gBoard, i, i + RW[i], i + 2*RW[i]);
			if (current != EMPTYSPACE)
				return EndGame(current, generic_hash_turn(pos));
		} else if (RN[i] >= BOARDHEIGHT && RN[i] <= 2*BOARDHEIGHT - 2 && CP[i] >=2) {
			current = ThreeInARow(gBoard, i, i + RW[i]-1, i + 2*RW[i] - 3);
			if (current != EMPTYSPACE)
				return EndGame(current, generic_hash_turn(pos));
		}

		if (RN[i] < BOARDHEIGHT - 1) {
			current = ThreeInARow(gBoard, i, i + RW[i] + 1, i + 2*RW[i] + 3);
			if (current != EMPTYSPACE)
				return EndGame(current, generic_hash_turn(pos));
		} else if (RN[i] == BOARDHEIGHT - 1 && CP[i] != RW[i]-1) {
			current = ThreeInARow(gBoard, i, i + RW[i]+1, i + 2*RW[i] + 2);
			if (current != EMPTYSPACE)
				return EndGame(current, generic_hash_turn(pos));
		} else if (RN[i] >= BOARDHEIGHT && RN[i] <= 2*BOARDHEIGHT - 2 && CP[i] < RW[i] -2) {
			current = ThreeInARow(gBoard, i, i + RW[i], i + 2*RW[i] - 1);
			if (current != EMPTYSPACE)
				return EndGame(current, generic_hash_turn(pos));
		}
	}
	if (AllFilledIn(gBoard))
		return tie;
	else
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

void PrintPosition (position, playerName, usersTurn)
POSITION position;
STRING playerName;
BOOLEAN usersTurn;
{
	int i, j, m = 0; /*, n = 0;*/
	generic_hash_unhash(position, gBoard);

	char wink, opWink;
	int numCheckers = 0;
	int numWinks = 0;
	int numOpWinks = 0;

	/*  printf("NumOfOptions: %d\n", NumberOfOptions());
	   for (j = 1; j <= NumberOfOptions(); j++) {
	   setOption(j);
	   printf("Option: %d - W: %d, H: %d, ", j, BOARDWIDTH, BOARDHEIGHT);
	   if (gStandardGame)
	    printf("Standard\n");
	   else
	    printf("Misere\n");
	    } */



	//  if (Primitive(position)) {
	generic_hash_unhash(position, gBoard);

	if (generic_hash_turn(position) == 1) {
		wink = 'R';
		opWink = 'B';
	} else {
		wink = 'B';
		opWink = 'R';
	}

	//Count pieces on board
	for (i = 0; i < BOARDSIZE; i++)
		if (gBoard[i] == opWink)
			numOpWinks++;
		else if (gBoard[i] == wink)
			numWinks++;
		else if (gBoard[i] == 'O')
			numCheckers++;

	printf("\n");

	int z = 1;
	for (i = 0; i < 2*BOARDHEIGHT+1; i++) {
		if (i == BOARDHEIGHT)
			printf (" LEGEND: ");
		else
			printf ("         ");

		PrintSpaces (2*abs(BOARDHEIGHT - i));

		for (j = 0; j < BOARDWIDTH + BOARDHEIGHT - abs(BOARDHEIGHT - i); j++) {
			printf("%2d  ", z);
			z++;
		}
		//      printf("%c ", LegendKey[n++]);

		PrintSpaces (2*abs(BOARDHEIGHT - i));
		printf(" : ");
		PrintSpaces (abs(BOARDHEIGHT - i));

		for (j = 0; j < BOARDWIDTH + BOARDHEIGHT - abs(BOARDHEIGHT - i); j++)
			printf("%c ", gBoard[m++]);

		if (i == BOARDHEIGHT)
			printf("%s", GetPrediction(position,playerName,usersTurn));

		printf("\n");
	}

	int half = (BOARDSIZE+1)/2;

	printf("\n%s pieces left (O: %d, %c: %d)", playerName, half - numOpWinks - (numCheckers/2), wink, half - numWinks);
	printf("\n\n");

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
** CALLS:       GENERIC_PTR SafeMalloc(int)
**              LIST OTHER CALLS HERE
**
************************************************************************/
MOVELIST *GenerateMoves (position)
POSITION position;
{
	MOVELIST *CreateMovelistNode(), *head = NULL;
	BOOLEAN AllFilledIn(char*);
	VALUE Primitive();
	int player = generic_hash_turn (position);
	char wink, opWink;
	int numCheckers = 0;
	int numWinks = 0;
	int numOpWinks = 0;
	int i;

	//  if (Primitive(position)) {
	generic_hash_unhash(position, gBoard);

	if (player == 1) {
		wink = 'R';
		opWink = 'B';
	} else {
		wink = 'B';
		opWink = 'R';
	}

	//Count pieces on board
	for (i = 0; i < BOARDSIZE; i++)
		if (gBoard[i] == opWink)
			numOpWinks++;
		else if (gBoard[i] == wink)
			numWinks++;
		else if (gBoard[i] == 'O')
			numCheckers++;

	/*    //Generate checker moves
	   if (((BOARDSIZE+1)/2 - numOpWinks - numCheckers/2) > 0)
	   for (i = 0; i < BOARDSIZE; i--)
	    if (gBoard[i] == EMPTYSPACE)
	      head = CreateMovelistNode(moveHash(0, i , O), head);

	   //Generate winker moves
	   if (numWinks > 0)
	   for (i = 0; i < BOARDSIZE; i--)
	    if (gBoard[i] == O)
	      head = CreateMovelistNode(moveHash(0, i , wink), head);
	 */

	for (i = BOARDSIZE - 1; i >= 0; i--) {
		if (gBoard[i] == 'O' && numWinks < (BOARDSIZE+1)/2)
			head = CreateMovelistNode(i+1, head);
		else if (gBoard[i] == EMPTYSPACE && ((BOARDSIZE+1)/2 - numOpWinks - numCheckers/2) > 0)
			head = CreateMovelistNode(i+1, head);
	}

	/* Commented out to support goAgain
	   if (head == NULL && AllFilledIn(gBoard) == FALSE)
	   head = CreateMovelistNode(PASSMOVE, head);
	 */

	return head;
	// }
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

USERINPUT GetAndPrintPlayersMove (thePosition, theMove, playerName)
POSITION thePosition;
MOVE *theMove;
STRING playerName;
{
	USERINPUT ret, HandleDefaultTextInput();
	do {
		printf("%8s's move [1-%d] : ", playerName, BOARDSIZE);
		/*    if (BOARDSIZE == 0)
		      printf("0] : ");
		      else if (BOARDSIZE < 10)
		      printf("1-%c] : ", LegendKey[BOARDSIZE-1]);
		      else if (BOARDSIZE == 10)
		      printf("1-9/A] : ");
		      else if (BOARDSIZE < 36)
		      printf("1-9/A-%c] : ", LegendKey[BOARDSIZE-1]);
		      else if (BOARDSIZE == 36)
		      printf("1-9/A-Z/a] : ");
		      else
		      printf("1-9/A-Z/a-%c] : ", LegendKey[BOARDSIZE-1]);
		 */

		ret = HandleDefaultTextInput(thePosition, theMove, playerName);
		if (ret != Continue)
			return(ret);

	} while(TRUE);

	return (Continue);
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

BOOLEAN ValidTextInput (input)
STRING input;
{
	if (strlen(input) != 1 && strlen(input)!= 2)
		return FALSE;
	int a;
	if ((a = ConvertToNumber(input)) < 0)
		return FALSE;
	return TRUE;
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

MOVE ConvertTextInputToMove (input)
STRING input;
{
	return ConvertToNumber(input);
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

void PrintMove (move)
MOVE move;
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
	STRING m = (STRING) SafeMalloc( 5 );

	if (theMove == PASSMOVE)
		sprintf( m, "pass");
	else
		sprintf( m, "%d", theMove);
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

int NumberOfOptions()
{
	return (2 * (MAXWIDTH * (MAXHEIGHT + 1)));
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

int getOption()
{
	int opt = 0;

	int i;
	for (i = 2; i <= BOARDWIDTH; i++) {
		opt += (2 * (MAXHEIGHT+1));
	}

	opt += (2 * BOARDHEIGHT);

	if (!gStandardGame)
		opt += 1;

	return opt+1;
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

void setOption(int option)
{
	option -= 1;

	BOARDWIDTH = 1;
	for (; option >= (2 * (MAXHEIGHT+1)); ) {
		BOARDWIDTH++;
		option -= (2 * (MAXHEIGHT+1));
	}

	BOARDHEIGHT = 0;
	for (; option >= 2; ) {
		BOARDHEIGHT++;
		option -= 2;
	}

	if (option == 1)
		gStandardGame = FALSE;
	else
		gStandardGame = TRUE;

}


/************************************************************************
*************************************************************************
**         EVERYTHING BELOW THESE LINES IS LOCAL TO THIS FILE
*************************************************************************
************************************************************************/


/************************************************************************
** This is where you can put any helper functions, including your
** hash and unhash functions if you are not using one of the existing
** ones.
************************************************************************/

int ConvertToNumber(input)
STRING input;
{
	int x;

	if (strlen(input) == 1) {
		char a = input[0];

		x = a - '0';
	} else if (strlen(input) == 2) {
		int a = input[0] - '0';
		int b = input[1] - '0';

		if (a < 1 || b < 0 || a > 9 || b > 9)
			return -1;

		x = 10*a + b;
	}

	if (x < 0 || x > BOARDSIZE)
		return -1;
	/*  int i;
	    for (i=0; i< BOARDSIZE; i++)
	    if (a == LegendKey[i])
	    return i+1;
	 */
	return x;
}

char MoveToCharacter(move)
MOVE move;
{
	if (move < 10)
		return (move + '0');
	else if (move < 36)
		return (move + 'a' - 10);
	else
		return (move + 'A' - 36);
}

/* PrintSpaces(n) outputs n spaces
 */
void PrintSpaces (n)
int n;
{
	int i;
	for (i = 0; i < n; i++)
		printf(" ");
}

/* Legend will calculate the correct symbol to display on board.
 *
 * n - the position in numberical form
 *
 * returns 0-9,a-z,A-Z
 */
int Legend (n)
int n;
{
	if (n > 35)
		n += ('a' - 36);
	else if (n > 9)
		n += ('A' - 10);
	else
		n += '0';
	return n;
}

VALUE EndGame(char x, int player) {
	VALUE EndGame2(VALUE);

	if (x == 'R') {
		if (player == 1)
			return (gStandardGame ? win : lose);
		else
			return (gStandardGame ? lose : win);
	} else if (x == 'B') {
		if (player == 1)
			return (gStandardGame ? lose : win);
		else
			return (gStandardGame ? win : lose);
	} else
		return undecided;
}

char ThreeInARow(theBlankORB, a, b, c)
char theBlankORB[];
int a, b, c;
{
	if (theBlankORB[a] == theBlankORB[b] &&
	    theBlankORB[b] == theBlankORB[c] &&
	    (theBlankORB[c] != EMPTYSPACE || theBlankORB[c] != 'O'))
		return theBlankORB[a];
	else
		return EMPTYSPACE;
}

BOOLEAN AllFilledIn(theBlankORB)
char theBlankORB[];
{
	int i;

	for (i = 0; i < BOARDSIZE; i++) {
		if (theBlankORB[i]== EMPTYSPACE || theBlankORB[i]=='O')
			return FALSE;
	}
	return TRUE;
}

/*** RowNumber() returns the row number of the ith piece.
 *
 * Rows are numbered starting with 0.
 *
 */

int RowNumber(i)
int i;
{
	int CurrentRow = 0, CurrentWidth = BOARDWIDTH;

	while(TRUE) {
		if (i < CurrentWidth)
			return CurrentRow;
		i -= CurrentWidth;
		if (CurrentRow < BOARDHEIGHT)
			CurrentWidth += 1;
		else
			CurrentWidth -= 1;
		CurrentRow += 1;
	}
	return 0;
}

/*** RowWidth() determines the width of the row i is on.
 *
 */

int RowWidth(i)
int i;
{
	int CurrentRow = 0, CurrentWidth = BOARDWIDTH;

	while(TRUE) {
		if (i < CurrentWidth)
			return CurrentWidth;
		i -= CurrentWidth;
		if (CurrentRow < BOARDHEIGHT)
			CurrentWidth += 1;
		else
			CurrentWidth -= 1;
		CurrentRow += 1;
	}
	return 0;
}

/*** ColPosition() determines where in the Row a piece is.
 *
 * The first piece in the row is 0.
 * The last piece is (width - 1).
 *
 */

int ColPosition(i)
int i;
{
	int CurrentRow = 0, CurrentWidth = BOARDWIDTH;

	while(TRUE) {
		if (i < CurrentWidth)
			return i;
		i -= CurrentWidth;
		if (CurrentRow < BOARDHEIGHT)
			CurrentWidth += 1;
		else
			CurrentWidth -= 1;
		CurrentRow += 1;
	}
	return 0;
}

BOOLEAN passMoveOnly (POSITION pos, MOVE move) {

	int player1 = generic_hash_turn(pos);

	POSITION next = DoMove(pos, move);

	return (generic_hash_turn(next) == player1);
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
