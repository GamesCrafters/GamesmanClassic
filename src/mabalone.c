// $Id: mabalone.c,v 1.46 2007-10-17 02:37:50 dmchan Exp $
/************************************************************************
**
** NAME:        mabalone.c
**
** DESCRIPTION: Abalone
**
** AUTHOR:      Daniel Wei & Jerry Hong & Michael Mottmann & Melinda Franco
**
** DATE:        10/2/06 - Attempted to tierify
**              4/6/04 - Working and all nice and pretty and stuff
**              5/3/04 - Even better than before!
**
** UPDATE HIST: WHAT ONCE WAS BROKEN NOW IS FIXED
**		8-21-06	commented out fflush and changed to use GetMyChar()/GetMyInt();
**			dmchan
**
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
#include <string.h>
#include <math.h>

extern STRING gValueString[];
POSITION gNumberOfPositions  = 924;

POSITION gInitialPosition    = 9;
POSITION gMinimalPosition    = 0;
POSITION kBadPosition        = -1;

STRING kAuthorName         = "Jerry Hong, Daniel Wei, Michael Mottmann and Melinda Franco";
STRING kGameName           = "Abalone";
STRING kDBName             = "Abalone";
BOOLEAN kPartizan           = TRUE;
BOOLEAN kSupportsHeuristic  = FALSE;
BOOLEAN kSupportsSymmetries = FALSE;
BOOLEAN kSupportsGraphics   = FALSE;
BOOLEAN kDebugMenu          = FALSE;
BOOLEAN kGameSpecificMenu   = TRUE;
BOOLEAN kTieIsPossible      = FALSE;
BOOLEAN kLoopy               = TRUE;
BOOLEAN kDebugDetermineValue = FALSE;
void*    gGameSpecificTclInit = NULL;

STRING kHelpGraphicInterface =
        "Not written yet";

STRING kHelpTextInterface    =
        "ON YOUR TURN, use the legend to determine which pieces to move.\n\
In a move, multiple pieces can be moved, but any one piece\n\
can only shift one space away from where it started.\n\
There are two types of moves, pushes and side-steps.\n\n\
ONE PIECE can be pushed in any direction, as long as the connecting\n\
space is vacant.  TO PUSH TWO OR THREE PIECES, the direction of movement\n\
must be along the axis on which the pieces lie.  Multiple piece pushes\n\
can move into a vacant space, or into a space with an opponent's piece\n\
provided your pieces outnumber your opponent's (3 pieces in a row can\n\
push 1 or 2 pieces, 2 pieces can only push 1 piece).  Only through a push\n\
off the edge can you eliminate your opponent's pieces.\n\n\
IN A SIDE-STEP, two or three pieces that are in a line can all move\n\
in any direction in unison, as long as their desination spaces are empty.\n\n\
TO PERFORM A PUSH, enter the first piece that pushes all the rest, followed\n\
by the direction of the push.  TO PERFORM A SIDE-STEP, enter all the pieces to\n\
be moved, followed by the direction of the side-step.\n"                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            ;

STRING kHelpOnYourTurn =
        "Choose which piece(s) of yours to move, and in what direction.";

STRING kHelpStandardObjective =
        "To push as many of your opponent's pieces off the board as it takes to win";

STRING kHelpReverseObjective =
        "To force your opponent to push as many of your pieces off the board as it takes to win.";

STRING kHelpTieOccursWhen =   /* Should follow 'A Tie occurs when... */
                            "There is no tie, only do";

STRING kHelpExample =
        "          BOARD                 LEGEND        DIRECTIONS\n\
\n\
       -----------        \n\
      / --------- \\       |\n\
     / /         \\ \\      |\n\
    / /  (x)-(x)  \\ \\     |     (1)-(2)        NW   NE\n\
   / /   / \\ / \\   \\ \\  |      / \\ / \\          \\ /\n\
  | |  ( )-( )-( )  | |   |   (3)-(4)-(5)      W -*- E\n\
   \\ \\   \\ / \\ /   / /  |     \\ / \\ /          / \\\n\
    \\ \\  (o)-(o)  / /     |     (6)-(7)        SW   SE\n\
     \\ \\         / /      |\n\
      \\ --------- /       |\n\
       -----------\n\
\n\
\n\
  Player's move :  6 7 ne\n\
\n\
          BOARD                 LEGEND        DIRECTIONS\n\
\n\
       -----------        \n\
      / --------- \\       |\n\
     / /         \\ \\      |\n\
    / /  (x)-(x)  \\ \\     |     (1)-(2)        NW   NE\n\
   / /   / \\ / \\   \\ \\  |      / \\ / \\          \\ /\n\
  | |  ( )-(o)-(o)  | |   |   (3)-(4)-(5)      W -*- E\n\
   \\ \\   \\ / \\ /   / /  |      \\ / \\ /          / \\\n\
    \\ \\  ( )-( )  / /     |     (6)-(7)        SW   SE\n\
     \\ \\         / /      |\n\
      \\ --------- /       |\n\
       -----------\n\
\n\
\n\
Computer's move   : [1 SW]\n\
\n\
          BOARD                 LEGEND        DIRECTIONS\n\
\n\
       -----------        \n\
      / --------- \\       |\n\
     / /         \\ \\      |\n\
    / /  ( )-(x)  \\ \\     |     (1)-(2)        NW   NE\n\
   / /   / \\ / \\   \\ \\  |      / \\ / \\          \\ /\n\
  | |  (x)-(o)-(o)  | |   |   (3)-(4)-(5)      W -*- E\n\
   \\ \\   \\ / \\ /   / /  |      \\ / \\ /          / \\\n\
    \\ \\  ( )-( )  / /     |     (6)-(7)        SW   SE\n\
     \\ \\         / /      |\n\
      \\ --------- /       |\n\
       -----------\n\
\n\
\n\
Player's move :  5 w\n\
\n\
          BOARD                 LEGEND        DIRECTIONS\n\
\n\
       -----------        \n\
      / --------- \\       |\n\
     / /         \\ \\      |\n\
    / /  ( )-(x)  \\ \\     |     (1)-(2)        NW   NE\n\
   / /   / \\ / \\   \\ \\  |      / \\ / \\          \\ /\n\
  | |  (o)-(o)-( )  | |   |   (3)-(4)-(5)      W -*- E\n\
   \\ \\   \\ / \\ /   / /  |      \\ / \\ /          / \\\n\
    \\ \\  ( )-( )  / /     |     (6)-(7)        SW   SE\n\
     \\ \\         / /      |\n\
      \\ --------- /       |\n\
       -----------\n\
\n\
Excellent! You won!"                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   ;

STRING MoveToString( MOVE );

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

#define NULLSLOT 99
#define MAXN 3 /*the maximum n that the solver will solve for*/
int N = 2;
int MISERE = 0;
int NSS = 0;
int XHITKILLS = 1;
int PIECES = 2;

int primcount = 1;

BOOLEAN DEBUGGING = FALSE;
int BOARDSIZE;
char *gBoard;

struct row {
	int size;
	int start_slot;
};

struct row **rows;

/*************************************************************************
**
** Above is where you put your #define's and your global variables, structs
**
*************************************************************************/

//Function prototypes here.
void SetupGame();
TIER BoardToTier(char* board);
TIER PiecesToTier(int, int);
void TierToPieces(TIER, int *, int *);
void SetupTierStuff();
STRING TierToString(TIER tier);
TIERLIST* TierChildren(TIER tier);
TIERPOSITION NumberOfTierPositions(TIER tier);
POSITION hash(int);
void unhash(POSITION, int*);
void FreeHelper(struct row**);

// External
#ifndef MEMWATCH
extern GENERIC_PTR      SafeMalloc ();
extern void             SafeFree ();
#endif

// Internal
int destination (int, int);
int move_hash (int, int, int, int);
struct row * makerow (int, int);
void printrow (int, int);
void printlines (int, int);
void changeBoard ();
int b_size (int);
int def_start_pieces (int);
void changePieces();
void changeKills();
int maxPieces(int);
int sum(int, int);
BOOLEAN ValidTextInputHelper(STRING, int);
BOOLEAN validCoordinate(int, int);
int intToCoordinateX(int);
int intToCoordinateY(int);
int coordinateToInt(int, int);


/************************************************************************
**
** NAME:        InitializeGame
**
** DESCRIPTION: Initialize the gDatabase, a global variable. and the other
**              local variables.
**
************************************************************************/
int gMax;
int init;

void InitializeGame()
{
	if (DEBUGGING) printf("start initialize game\n");

	SetupGame();

	gMoveToStringFunPtr = &MoveToString;

	if (DEBUGGING) printf("end initializegame\n");
}

/************************************************************************
**
** NAME:        DebugMenu
**
** DESCRIPTION: Menu used to debub internal problems. Does nothing if
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

void GameSpecificMenu()
{
	/*fflush(stdin);	no longer needed*/
	char selection;
	printf("\n\tn)\t Change the value of (N), the edge size of the hexagon board -- currently %d\n", N);
	if (MISERE == 0)
		printf("\tm)\t Toggle from Standard to (M)isere\n");
	else
		printf("\tm)\t Toggle from (M)isere to Standard\n");
	if (NSS == 0)
		printf("\ts)\t Toggle from (S)ide Steps Allowed to No Side Steps\n");
	else
		printf("\ts)\t Toggle from No (S)ide Steps to Side Steps Allowed\n");
	printf("\tp)\t Change initial number of (p)ieces -- there are currently %d, max allowed is %d\n", PIECES, maxPieces(N));
	printf("\tc)\t Change the number of pieces that must be (c)aptured to win -- currently %d\n\n", XHITKILLS);
	printf("\tb)\t (B)ack to the previous menu\n\nSelect Option:  ");


	/*fflush(stdin);
	   (void) scanf("%c", &selection);*/

	selection = GetMyChar();
	/*selection = (char) getchar(); doesn't follow abstraction?*/

	if (selection == 'n' || selection == 'N') {
		changeBoard();
		printf("\n");
		GameSpecificMenu();
	} else if (selection == 'm' || selection == 'M') {
		if (MISERE == 0)
			MISERE = 1;
		else
			MISERE = 0;

		//SafeFree(rows);
		//printf("calling safefree0\n");
		FreeHelper(rows);
		SafeFree(gBoard);
		InitializeGame();
		printf("\n");
		GameSpecificMenu();
	} else if (selection == 's' || selection == 'S') {
		if (NSS == 0)
			NSS = 1;
		else
			NSS = 0;

		//SafeFree(rows);
		//printf("calling safefree1\n");
		FreeHelper(rows);
		SafeFree(gBoard);
		InitializeGame();
		printf("\n");
		GameSpecificMenu();
	}
	else if (selection == 'p' || selection == 'P') {
		changePieces();
		printf("\n");
		GameSpecificMenu();
	}
	else if (selection == 'c' || selection == 'C') {
		changeKills();
		printf("\n");
		GameSpecificMenu();
	}
	else if ((selection == 'b') || (selection == 'B'))
		return;
	else {
		printf("\n\n\n Please select a valid option...\n\n");
		GameSpecificMenu();
	}
}

void changeBoard()
{
	unsigned int size;
	printf("Enter the new N:  ");
	size = GetMyUInt();
	if (size < 2) {
		printf("N must be at least 2\n");
		changeBoard();
	}
	else {
		printf("Changing N to %d ...\n", size);
		FreeHelper(rows);
		SafeFree(gBoard);
		N = size;
		MISERE = 0;
		NSS = 0;
		XHITKILLS = 1;
		PIECES = def_start_pieces(N);
		//SafeFree(rows);
		//printf("calling safefree2\n");
		InitializeGame();
	}
}

void changeKills()
{
	unsigned int kills;
	printf("Enter the new number of pieces to capture:   ");
	kills = GetMyUInt();
	if (PIECES - kills < 0) {
		printf("A player can only lose as many pieces as the game starts with\n");
		changeKills();
	}
	else if (kills <= 0) {
		printf("There must be at least one piece captured\n");
		changeKills();
	}
	else{
		XHITKILLS = kills;
		//SafeFree(rows);
		//printf("calling safefree3\n");
		FreeHelper(rows);
		SafeFree(gBoard);
		InitializeGame();
	}
}

void changePieces()
{
	int num;
	printf("Enter the new number of pieces:  ");
	num = GetMyUInt();
	if ((2 * num + (*rows[N-1]).size) > BOARDSIZE) {
		printf("Too many pieces for board\n");
		changePieces();
	}
	else if (num < 2) {
		printf("There must be at least two pieces\n");
		changePieces();
	}
	else {
		PIECES = num;
		XHITKILLS = 1;
		//SafeFree(rows);
		//printf("calling safefree4\n");
		FreeHelper(rows);
		SafeFree(gBoard);
		InitializeGame();
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

void SetTclCGameSpecificOptions(theOptions)
int theOptions[];
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
**	        destination ()
*************************************************************************/
POSITION DoMove(thePosition, theMove)
POSITION thePosition;
MOVE theMove;
{
	if (DEBUGGING)
		printf("Starting Do Move with input: %d\n", theMove);
	int destination(int,int);
	int whoseTurn;
	unhash(thePosition, &whoseTurn);
	int direction;
	int slot1, slot2, slot3, dest1, dest2, dest3, pushee1, pushee2, pushdest1, pushdest2, doubpushdest;
	BOOLEAN twopieces = FALSE, threepieces = FALSE;
	int dir_shift = 10;
	int piece_shift = 100;

	direction = theMove % dir_shift;
	theMove = theMove/dir_shift;

	if (theMove < 0) {
		theMove = 0 - theMove;
	}

	slot1 = theMove % piece_shift;
	theMove = theMove/piece_shift;
	slot2 = theMove % piece_shift;
	slot3 = theMove/piece_shift;

	/*one piece scenarios*/
	if ((slot1 == NULLSLOT) && (slot2 == NULLSLOT)) {
		slot1 = slot3;
	}
	else if ((slot2 == NULLSLOT) && (slot3 == NULLSLOT)) {
		/*do nothing*/
	}
	else if (slot1 == NULLSLOT) {
		twopieces = TRUE;
		slot1 = slot2;
		slot2 = slot3;
	}
	else if (slot3 == NULLSLOT) {
		twopieces = TRUE;
	}
	else {
		threepieces = TRUE;
	}

	dest1 = destination(slot1, direction);
	dest2 = destination(slot2, direction);
	dest3 = destination(slot3, direction);
	pushee1 = dest3;
	doubpushdest = destination(dest2, direction);
	pushee2 = destination(pushee1, direction);
	pushdest1 = pushee2;
	pushdest2 = destination(pushee2, direction);

	/* one piece move */
	if ((twopieces == FALSE) && (threepieces == FALSE)) {
		gBoard[dest1] = gBoard[slot1];
		gBoard[slot1] = '*';
	}

	/* double piece move */
	if (twopieces) {
		/*printf("slot1 = %d, slot2 = %d, in direction %d, dest1 is %d, dest2 is %d, doubpushdest is %d\n", slot1, slot2, direction, dest1, dest2, doubpushdest);*/
		if (gBoard[dest2] != '*') {
			/*push a piece*/
			if (doubpushdest != NULLSLOT) {
				gBoard[doubpushdest] = gBoard[dest2];
			}
		}

		/*printf("dest1 is %d:%c, dest2 is %d:%c\n", dest1, gBoard[dest1], dest2, gBoard[dest2]);*/

		gBoard[dest2] = gBoard[slot2];
		gBoard[slot2] = '*';
		gBoard[dest1] = gBoard[slot1];
		gBoard[slot1] = '*';

		/*printf("dest1 is %d:%c, dest2 is %d:%c\n", dest1, gBoard[dest1], dest2, gBoard[dest2]);*/
	}

	/* triple piece move */
	if (threepieces) {
		/*three push two*/
		if ((pushee2 != NULLSLOT) && (gBoard[pushee1] != '*') && (gBoard[pushee2] != '*')) {
			if (pushdest2 != NULLSLOT) {
				gBoard[pushdest2] = gBoard[pushee2];
			}
			gBoard[pushdest1] = gBoard[pushee1];
		}
		else if (gBoard[pushee1] != '*') {
			/*three push one*/
			if (pushdest1 != NULLSLOT) {
				gBoard[pushdest1] = gBoard[pushee1];
			}
		}

		/*main move*/
		gBoard[dest3] = gBoard[slot3];
		gBoard[slot3] = '*';
		gBoard[dest2] = gBoard[slot2];
		gBoard[slot2] = '*';
		gBoard[dest1] = gBoard[slot1];
		gBoard[slot1] = '*';
	}

	if (DEBUGGING) printf("finished do move\n");
	return (hash((whoseTurn == 1 ? 2 : 1)));
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
	int count, player, xs = 0, os = 0, option = -1;
	char selection;

	while ((option < 0) || (option > maxPieces(N))) {
		printf("Enter the maximum pieces per side: ");
		/*fflush(stdin);			no longer needed
		   (void) scanf("%d", &option);*/
		option = GetMyInt();
		if ((option < 0) || (option > maxPieces(N)))
			printf("Please enter a valid number\n\n");
	}
	PIECES = option;
	option = -1;

	while ((option < 1) || (option > PIECES)) {
		printf("Enter the number of pieces to capture for victory: ");
		/*fflush(stdin);			no longer needed
		   (void) scanf("%d", &option);*/
		option = GetMyInt();
		if ((option < 1) || (option > PIECES))
			printf("The number of eliminated pieces must be at least 1\nand no more than the total number of pieces\n\n");
	}
	XHITKILLS = option;

	printf("Enter the board as you would like it (enter a 'b' for blanks)\n\n");

	for (count = 0; count < BOARDSIZE; count++) {
		printf("Enter the piece at position %d: ", count + 1);
		/*fflush(stdin);			no longer needed
		   (void) scanf("%c", &selection);*/
		selection = GetMyChar();
		if ((selection != 'x') && (selection != 'o') && (selection != 'b') && (selection != 'X') && (selection != 'O') && (selection != 'B')) {
			printf("\n\nPlease enter a valid piece\n\n");
			count--;
		}
		else if (selection == 'x') {
			if (xs < PIECES) {
				gBoard[count] = selection;
				xs++;
			}
			else {
				printf("\n\nThis board already has the maximum number of x pieces allowed\n\n");
				count--;
			}
		}
		else if (selection == 'o') {
			if (os < PIECES) {
				gBoard[count] = selection;
				os++;
			}
			else {
				printf("\n\nThis board already has the maximum number of o pieces allowed\n\n");
				count--;
			}
		}
		else {
			gBoard[count] = '*';
		}
	}

	player = 0;
	while (player == 0) {
		printf("\n\n Whose Turn is it?  ");
		/*fflush(stdin);			no longer needed
		   (void) scanf("%c", &selection);*/
		selection = GetMyChar();
		if ((selection == 'x') || (selection == 'X'))
			player = 2;
		else if ((selection == 'o') || (selection == 'O'))
			player = 1;
		else
			printf ("\n\n Please enter x or o\n\n");
	}

	return hash(player);
}


/************************************************************************
**
** NAME:        PrintComputersMove*
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
	printf("%8s's move   : ", computersName);
	PrintMove(computersMove);
	//SafeFree(computersMove);
	printf("\n");
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

VALUE Primitive ( POSITION h )
{
	if (DEBUGGING) printf("prim\n");
	BOOLEAN game_over(char[]);

	int whoseTurn;
	unhash(h,&whoseTurn);

	/*
	   int i;
	   for(i=0; i<BOARDSIZE; i++)
	   {
	   printf("%c", gBoard[i]);
	   }
	   printf("\n");
	 */




	/*  printf("analyzing position %d\n", primcount);*/
	primcount++;

	if (game_over(gBoard)) {
		if (MISERE == 1) {
			if (DEBUGGING) printf("end prim\n");
			return (win);
		}
		else {
			if (DEBUGGING) printf("end prim\n");
			return (lose);
		}
	}
	if (DEBUGGING) printf("end prim\n");
	return (undecided);
}

BOOLEAN game_over(char theBoard[]){
	int count, x = 0, o = 0, lose;

	lose = PIECES - XHITKILLS;

	for (count = 0; count < BOARDSIZE; count++) {
		if (theBoard[count] == 'x') {
			x++;
		}
		else if (theBoard[count] == 'o') {
			o++;
		}
		/*printf("position %d is a %c\n", count, theBoard[count]);*/
	}
	/*printf("I counted %d os and %d xs\n", o, x);*/
	return (o<=lose || x<=lose);
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

void PrintPosition(position, playerName, usersTurn)
POSITION position;
STRING playerName;
BOOLEAN usersTurn;
{
	int whoseMove;
	unhash(position, &whoseMove);
	int r, spacing;

	char piece;
	if (whoseMove == 2)
		piece = 'x';
	else
		piece = 'y';

	//printf("trying to print in PrintPosition");
	if (N < 4) {
		/* messy centering spacing issues for sizeable first line*/
		printf("\n");
		spacing = (4 * (*rows[N - 1]).size) + 8;
		for (r = 0; r < spacing/2; r++)
			printf (" ");
		printf("BOARD");
		for (r = 0; r < spacing/2; r++)
			printf (" ");
		if (spacing % 2 == 1)
			printf (" ");


		if (N == 2)
			spacing = (4 * (*rows[N - 1]).size) + 2;
		else
			spacing = (5 * (*rows[N - 1]).size) + 2;

		for (r = 0; r < spacing/2; r++)
			printf (" ");
		printf("  LEGEND");
		for (r = 0; r < spacing/2; r++)
			printf (" ");
		if (spacing % 2 == 1)
			printf (" ");

		printf("   DIRECTIONS\n\n");
	}


	/* edge of hex board */
	/*top edge*/
	printf("       ");
	for (r = 0; r < N - 2; r++) {
		printf("  ");
	}
	printf("---");
	for (r = 0; r < N; r++) {
		printf("----");
	}
	printf("        ");
	printf("\n");

	/*second edge*/
	printf("      ");
	for (r = 0; r < N - 2; r++) {
		printf("  ");
	}
	printf("/ ");
	for (r = 0; r < N; r++) {
		printf("----");
	}
	printf("- \\");
	if (N < 4) {
		printf("      ");
		for (r = 0; r < N - 2; r++) {
			printf("  ");
		}
		printf(" |\n"); //2nd row of |
	}
	else {
		printf("\n");
	}

	/*spacer*/
	printf("     ");
	for (r = 0; r < N - 2; r++) {
		printf("  ");
	}
	printf("/ /");  //First row with (//                   \\)
	for (r = 0; r < N; r++) {
		printf("    ");
	}
	printf(" \\ \\");
	if (N < 4) {
		printf("     ");
		for (r = 0; r < N - 2; r++) {
			printf("  ");
		}
		printf(" |\n"); //3rd row of |
	}
	else {
		printf("\n");
	}


	/*main board printing here*/

	for (r = 0; r < (2 * N) - 1; r++) {
		if (N < 4) {
			printf("  ");
			printrow(r, 0);
			printf("   |   "); // this is for the rows with letters
			printrow(r, 1);

			if ( (r == N - 2) && (N==2))
				printf("      NW   NE");
			else if ((r == N - 1) && (N==2))
				printf("     W -*- E");
			else if ((r == N) && (N==2))
				printf("      SW   SE");

			if ( (r == N - 2) && (N==3))
				printf("         NW   NE");
			else if ((r == N - 1) && (N==3))
				printf("        W -*- E");
			else if ((r == N) && (N==3))
				printf("         SW   SE");

			else
				printf("%s", "");
			if (r != 2 * N - 2)
			{
				if(N==2)
				{
					printf("\n ");
					printlines(r, 0);
					printf("|   "); //Print the | for the top and the bottom rows with /\ /\ /\ /\ with boardsize of 3
					printlines(r, 1);
					printf("  ");
				}

				if(N==3)
				{
					printf("\n ");
					printlines(r, 0);
					printf("|   "); //Print the | for the top and the bottom rows with /\ /\ /\ /\ with boardsize of 3
					printlines(r, 1);
					printf("  ");   //This controls the space for the directions
				}

				if ((r == N - 1) && (N==2))
					printf("    / \\"); //Direction's (/\)
				else if ((r == N - 2) && (N==2))
					printf("    \\ /"); //Direction's(\/)

				if ((r == N - 1) && (N==3))
					printf("            / \\"); //Direction's (/\)
				else if ((r == N - 2) && (N==3))
					printf("            \\ /"); //Direction's(\/)
			}

			printf("\n");
		}


		else {
			printf("  ");
			printrow(r, 0);
			printf("\n");
			if (r != 2 * N - 2) {
				printf(" ");
				printlines(r, 0);
				printf("\n");
			}
		}


	}

/* edge of hex board */
	/*spacer*/
	printf("     ");
	for (r = 0; r < N - 2; r++) {
		printf("  ");
	}
	printf("\\ \\");
	for (r = 0; r < N; r++) {
		printf("    ");
	}
	printf(" / /");         //Last rows with  (\\             //)
	if (N < 4) {
		printf("     ");
		for (r = 0; r < N - 2; r++) {
			printf("  ");
		}
		printf(" |\n"); // 2nd to last row with
	}
	else {
		printf("\n");
	}

	/*first edge*/
	printf("      ");
	for (r = 0; r < N - 2; r++) {
		printf("  ");
	}
	printf("\\ ");
	for (r = 0; r < N; r++) {
		printf("----");
	}
	printf("- /");
	if (N < 4) {
		printf("      ");
		for (r = 0; r < N - 2; r++) {
			printf("  ");
		}
		printf(" |\n"); //used for making the last line ----------
	}
	else {
		printf ("\n");
	}

	/*bottom edge*/
	printf("       ");
	for (r = 0; r < N - 2; r++) {
		printf("  ");
	}
	printf("---");
	for (r = 0; r < N; r++) {
		printf("----");
	}

	for (r = 0; r < N - 2; r++) {
		printf("  ");
	}
	printf("       ");
	printf("%s\n\n",GetPrediction(position,playerName,usersTurn));
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
**              Primitive
**
************************************************************************/

MOVELIST *GenerateMoves(position)
POSITION position;
{
	if (DEBUGGING)
		printf("generate\n");
	MOVELIST *head = NULL;
	MOVELIST *CreateMovelistNode(); /* In gamesman.c */
	VALUE Primitive();
	int slot, direction, ssdir;
	int pusher2, pusher3, pushee1, pushee2, pushee3;
	char whoseTurn, opponent;
	int whoTurn;

	unhash(position,&whoTurn);

	if (whoTurn == 2) {
		whoseTurn = 'x';
		opponent = 'o';
	}
	else {
		whoseTurn = 'o';
		opponent = 'x';
	}


	/*printf("the hash is %d\n", position);
	   for (slot = 0; slot < BOARDSIZE; slot++)
	   printf("%d :%c\n", slot, gBoard[slot]);*/

	if (Primitive(position) == undecided) {
		for (slot = 0; slot < BOARDSIZE; slot++) {
			if (gBoard[slot] == whoseTurn) {
				for (direction = 1; direction <= 3; direction++) {

					/*Single Piece Moves in all directions*/
					pushee1 = destination(slot, direction);
					pushee2 = destination(slot, (0 - direction));

					if (gBoard[pushee1] == '*') {
						head = CreateMovelistNode(move_hash(slot,NULLSLOT, NULLSLOT, direction), head);
					}
					if (gBoard[pushee2] == '*') {
						head = CreateMovelistNode(move_hash(slot,NULLSLOT, NULLSLOT, (0 - direction)), head);
					}

					/*Multiple Piece Moves in positive directions*/
					pusher2 = destination(slot, direction);
					if ((pusher2 != NULLSLOT) && (gBoard[pusher2] == whoseTurn)) {

						/*Double Piece Moves in positive directions*/
						pushee1 = destination(pusher2,direction);
						pushee2 = destination(pushee1,direction);
						if ((pushee1 != NULLSLOT) &&
						    ((gBoard[pushee1] == '*') ||
						     ((gBoard[pushee1] == opponent) &&
		  ((pushee2 == NULLSLOT) || (gBoard[pushee2] == '*'))))) {
							head = CreateMovelistNode(move_hash(slot, pusher2, NULLSLOT, direction), head);
						}

						/*Triple Piece Push in positive direction*/
						pusher3 = destination(pusher2, direction);
						pushee1 = destination(pusher3, direction);
						pushee2 = destination(pushee1, direction);
						pushee3 = destination(pushee2, direction);

						if (((pusher3 != NULLSLOT) && (gBoard[pusher3] == whoseTurn)) &&
						    ( /*no pieces pushed*/
						            ((pushee1 != NULLSLOT) && (gBoard[pushee1] == '*')) ||
						            /*pieces pushed*/
						            (((pushee1 != NULLSLOT) && (gBoard[pushee1] == opponent)) &&
						             ( /*one piece pushed*/
						                     ((pushee2 == NULLSLOT) || (gBoard[pushee2] == '*')) ||
						                     /*two pieces pushed*/
						                     (((pushee2 != NULLSLOT) && (gBoard[pushee2] == opponent)) && ((pushee3 == NULLSLOT) || (gBoard[pushee3] == '*'))))))) {
							head = CreateMovelistNode(move_hash(slot, pusher2, pusher3, direction), head);
						}
					}

					direction = 0 - direction;

					/*Multiple Piece Moves in negative directions*/
					pusher2 = destination(slot, direction);
					if ((pusher2 != NULLSLOT) && (gBoard[pusher2] == whoseTurn)) {

						/*Double Piece Moves in negative direction*/
						pushee1 = destination(pusher2,direction);
						pushee2 = destination(pushee1,direction);
						if ((pushee1 != NULLSLOT) &&
						    ((gBoard[pushee1] == '*') ||
						     ((gBoard[pushee1] == opponent) && ((pushee2 == NULLSLOT) || (gBoard[pushee2] == '*')))))
							head = CreateMovelistNode(move_hash(slot, pusher2, NULLSLOT, direction), head);


						/*Triple Piece Push in negative direction*/
						pusher3 = destination(pusher2, direction);
						pushee1 = destination(pusher3, direction);
						pushee2 = destination(pushee1, direction);
						pushee3 = destination(pushee2, direction);

						if (((pusher3 != NULLSLOT) && (gBoard[pusher3] == whoseTurn)) &&
						    ( /*no pieces pushed*/
						            ((pushee1 != NULLSLOT) && (gBoard[pushee1] == '*')) ||
						            /*pieces pushed*/
						            (((pushee1 != NULLSLOT) && (gBoard[pushee1] == opponent)) &&
						             ( /*one piece pushed*/
						                     ((pushee2 == NULLSLOT) || (gBoard[pushee2] == '*')) ||
						                     /*two pieces pushed*/
						                     (((pushee2 != NULLSLOT) && (gBoard[pushee2] == opponent)) && ((pushee3 == NULLSLOT) || (gBoard[pushee3] == '*'))))))) {
							head = CreateMovelistNode(move_hash(slot, pusher2, pusher3, direction), head);
						}
					}

					direction = 0 - direction;

					if (NSS == 0) {
						/*Test for possible side steps*/
						for (ssdir = -3; ssdir <= 3; ssdir++) {
							if ((ssdir != 0) && (ssdir != direction) && (ssdir != 0 - direction)) {
								/*skip over nonexistant zero direction as well as push direction*/
								pusher2 = destination(slot,direction);
								pusher3 = destination(pusher2, direction);
								pushee1 = destination(slot,ssdir);
								pushee2 = destination(pusher2, ssdir);
								pushee3 = destination(pusher3, ssdir);


								if ((pusher2 != NULLSLOT) && (pushee1 != NULLSLOT) && (pushee2 != NULLSLOT) &&
								    (gBoard[pusher2] == whoseTurn) && (gBoard[pushee1] == '*') && (gBoard[pushee2] == '*')) {

									/*two piece sidestep*/
									head = CreateMovelistNode(move_hash(slot,pusher2, NULLSLOT, ssdir), head);

									/*three piece sidestep*/
									if ((pusher3 != NULLSLOT) && (pushee3 != NULLSLOT) &&
									    (gBoard[pusher3] == whoseTurn) && (gBoard[pushee3] == '*')) {
										head = CreateMovelistNode(move_hash(slot,pusher2,pusher3,ssdir), head);
									}
								}
							}
						}


					}
				}
			}
		}
		if (DEBUGGING)
			printf("end gen\n");
		return(head);
	}
	if (DEBUGGING)
		printf("end gen - NULL\n");
	return(NULL);
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
	USERINPUT ret;
	char whoseTurn;
	if (generic_hash_turn(thePosition) == 2) {
		whoseTurn = 'x';
	}
	else {
		whoseTurn = 'o';
	}


	do {
		printf("for a list of valid moves, press ?\n\n");
		printf("%8s's move (%c):  ", playerName, whoseTurn);

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

BOOLEAN ValidTextInput (STRING input)
{
	int length = strlen(input), i = 0;

	if((length == 4) || (length == 5)) /* one piece case */
	{
		if(!ValidTextInputHelper(input, i))
			return FALSE;

		i += 3;
	}
	else if((length == 7) || (length == 8)) /* two piece case */
	{
		if(!(ValidTextInputHelper(input, i) && ValidTextInputHelper(input, i+3)))
			return FALSE;

		i += 6;
	}
	else if((length == 10) || (length == 11)) /* three piece case */
	{
		if(!(ValidTextInputHelper(input, i)  && ValidTextInputHelper(input, i+3)
		     && ValidTextInputHelper(input, i+6)))
			return FALSE;

		i += 9;
	}
	else
		return FALSE;

	/* check direction */

	if((((input[i] == 'W') || (input[i] == 'w')) && (input[i+1] == '\0')) ||
	   (((input[i] == 'E') || (input[i] == 'e')) && (input[i+1] == '\0')) ||
	   (((input[i] == 'N') || (input[i] == 'n')) && ((input[i+1] == 'W') || (input[i+1] == 'w'))) ||
	   (((input[i] == 'N') || (input[i] == 'n')) && ((input[i+1] == 'E') || (input[i+1] == 'e'))) ||
	   (((input[i] == 'S') || (input[i] == 's')) && ((input[i+1] == 'W') || (input[i+1] == 'w'))) ||
	   (((input[i] == 'S') || (input[i] == 's')) && ((input[i+1] == 'E') || (input[i+1] == 'e'))))
		return TRUE;
	else
		return FALSE;
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

MOVE ConvertTextInputToMove(input)
STRING input;
{
	if (DEBUGGING)
		printf("Starting conversion\n");
	int dir, p1, p2, p3, pushee;

	int length = strlen(input), i = 0;

	if((length == 4) || (length == 5)) /* one piece case */
	{
		p1 = coordinateToInt(input[0], input[1]) - 1;
		p2 = NULLSLOT;
		p3 = NULLSLOT;
		i = 3;
		//printf("p1 is %d", p1);
	}
	else if((length == 7) || (length == 8)) /* two piece case */
	{
		p1 = coordinateToInt(input[0], input[1]) - 1;
		p2 = coordinateToInt(input[3], input[4]) - 1;
		p3 = NULLSLOT;
		i = 6;
	}
	else if((length == 10) || (length == 11)) /* three piece case */
	{
		p1 = coordinateToInt(input[0], input[1]) - 1;
		p2 = coordinateToInt(input[3], input[4]) - 1;
		p3 = coordinateToInt(input[6], input[7]) - 1;
		i = 9;
	}

	/* check direction */
	//printf("input 1 and 2 is %c %c", input[i], input[i+1]);
	if((input[i] == 'W') || (input[i] == 'w'))
	{
		dir = -1;
	}
	else if((input[i] == 'E') || (input[i] == 'e'))
	{
		dir = 1;
	}
	else if(((input[i] == 'N') || (input[i] == 'n')) && ((input[i+1] == 'W') || (input[i+1] == 'w')))
	{
		dir = -2;
	}
	else if(((input[i] == 'N') || (input[i] == 'n')) && ((input[i+1] == 'E') || (input[i+1] == 'e')))
	{
		dir = -3;
		printf("NE");
	}
	else if(((input[i] == 'S') || (input[i] == 's')) && ((input[i+1] == 'W') || (input[i+1] == 'w')))
	{
		dir = 3;
	}
	else if(((input[i] == 'S') || (input[i] == 's')) && ((input[i+1] == 'E') || (input[i+1] == 'e')))
	{
		dir = 2;
	}

	/*fill in implicit push moves*/
	if ((p2 == NULLSLOT) && (p3 == NULLSLOT)) {
		pushee = destination (p1, dir);
		if ((pushee != NULLSLOT) && (gBoard[pushee] == gBoard[p1])) {
			p2 = pushee;

			pushee = destination (p2, dir);
			if ((pushee != NULLSLOT) && (gBoard[pushee] == gBoard[p2]))
				p3 = pushee;
		}
	}

	//printf("p1 = %d, p2 = %d, p3 = %d, dir = %d\n", p1, p2, p3, dir);
	int move = move_hash (p1, p2, p3, dir);
	if (DEBUGGING) printf("finished conversion: move is %d\n", move);
	return move;
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

STRING MoveToString( theMove )
MOVE theMove;
{
	STRING move = (STRING) SafeMalloc(12);

	if (DEBUGGING)
		printf("starting MoveToString w/move = %d\n", theMove);
	int direction, slot1, slot2, slot3;
	int dir_shift = 10;
	int piece_shift = 100;
	STRING dir;

	direction = theMove % dir_shift;
	theMove = theMove/dir_shift;


	if (theMove < 0) {
		theMove = 0 - theMove;
	}

	slot1 = theMove % piece_shift;
	theMove = theMove/piece_shift;
	slot2 = theMove % piece_shift;
	slot3 = theMove/piece_shift;

	/*match up internal numbers (starting at 0) w/external (starting at 1) */
	if (slot1 != NULLSLOT)
		slot1++;
	if (slot2 != NULLSLOT)
		slot2++;
	if (slot3 != NULLSLOT)
		slot3++;

	if (direction == 1) {
		dir = "E";
	}
	else if (direction == -1) {
		dir = "W";
	}
	else if (direction == -3) {
		dir = "NE";
	}
	else if (direction == -2) {
		dir = "NW";
	}
	else if (direction == 3) {
		dir = "SW";
	}
	else {
		dir = "SE";
	}

	/*printf("slot1 = %d, slot2 = %d, slot3 = %d, direction = %d\n", slot1, slot2, slot3, direction);*/

	if ((slot1 == NULLSLOT) && (slot2 == NULLSLOT)) {
		sprintf(move, "[%c%c %s]", intToCoordinateX(slot3), intToCoordinateY(slot3), dir);
	}
	else if ((slot3 == NULLSLOT) && (slot2 == NULLSLOT)) {
		sprintf(move, "[%c%c %s]", intToCoordinateX(slot1), intToCoordinateY(slot1), dir);
	}
	else if (slot1 == NULLSLOT) {
		if ((slot3 - 1 ) == destination ((slot2 - 1), direction))
			sprintf(move, "[%c%c %s]", intToCoordinateX(slot2), intToCoordinateY(slot2), dir);
		else
			sprintf(move, "[%c%c %c%c %s]",intToCoordinateX(slot2), intToCoordinateY(slot2),
			        intToCoordinateX(slot3), intToCoordinateY(slot3), dir);
	}
	else if (slot3 == NULLSLOT) {
		if ((slot2 - 1) == destination ((slot1 - 1), direction))
			sprintf(move, "[%c%c %s]", intToCoordinateX(slot1), intToCoordinateY(slot1), dir);
		else
			sprintf(move, "[%c%c %c%c %s]",intToCoordinateX(slot1), intToCoordinateY(slot1),
			        intToCoordinateX(slot2), intToCoordinateY(slot2), dir);
	}
	else if ((slot2 - 1) == destination((slot1 - 1), direction)) {
		sprintf(move, "[%c%c %s]",intToCoordinateX(slot1), intToCoordinateY(slot1), dir);
	}
	else {
		sprintf(move, "[%c%c %c%c %c%c %s]",intToCoordinateX(slot1), intToCoordinateY(slot1),
		        intToCoordinateX(slot2), intToCoordinateY(slot2), intToCoordinateX(slot3),
		        intToCoordinateY(slot3), dir);
	}
	if (DEBUGGING) printf("finished MoveToString\n");

	return move;
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
	int options = 0, n, p;
	for (n = 2; n <= MAXN; n++) {
		for (p = 2; p <= maxPieces(n); p++) {
			options += 4 * p;
		}
	}
	return options;
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
	int option = 1, n, p;
	for (n = 2; n < N; n++) {
		for (p = 2; p <= maxPieces(n); p++) {
			option += 4 * p;
		}
	}
	for (p = 2; p < PIECES; p++) {
		option += 4 * p;
	}

	if (NSS == 1)
		option += 2;
	if (MISERE == 1)
		option += 1;

	return option;
}

/************************************************************************
**
** NAME:        setOption
**
** DESCRIPTION: The correspondithang unhash for the game variants.
**				Should take the input and set all the appropriate
**				variants.
**
** INPUT:     int : the number representation of the options.
**
************************************************************************/
void setOption(int option)
{
	printf("Set Option has been called!\n");
	option--;
	int n = 2, pieces = 2, kills = 0, hash, mod;

	for (hash = 0; hash <= option; hash += 4) {
		if ((pieces == maxPieces(n)) && (pieces == kills)) {
			pieces = 2;
			kills = 1;
			n++;
		}
		else if (kills == pieces) {
			pieces++;
			kills = 1;
		}
		else
			kills++;
	}


	N = n;
	PIECES = pieces;
	XHITKILLS = kills;

	mod = fmod(hash - option, 4);

	switch (mod) {
	case 0:
		NSS = 0;
		MISERE = 0;
		break;
	case 3:
		NSS = 0;
		MISERE = 1;
		break;
	case 2:
		NSS = 1;
		MISERE = 0;
		break;
	case 1:
		NSS = 1;
		MISERE = 1;
	}
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
int destination(int slot, int direction) {
	if (DEBUGGING)
		printf("starting destination\n");
	/*garbage in, garbage out*/
	if (slot == NULLSLOT)
		return NULLSLOT;

	/*find the row*/
	int r, start, size;
	for (r = 0; r <= 2*N - 2; r++) {
		if (slot < (*rows[r]).start_slot + (*rows[r]).size) {
			break;
		}
	}
	start = (*rows[r]).start_slot;
	size = (*rows[r]).size;

	/*test for nullslot*/

	if ((direction == 1 && slot == start + size - 1) ||
	    (direction == 2 && ((r == 2*N -2) || ((slot == start + size -1) && (*rows[r+1]).size < size))) ||
	    (direction == 3 && ((r == 2*N -2) || ((slot == start) && (*rows[r+1]).size < size))) ||
	    (direction == -1 && slot == start) ||
	    (direction == -2 && ((r == 0) || ((slot == start) && (*rows[r-1]).size < size))) ||
	    (direction == -3 && ((r == 0) || ((slot == start + size -1) && (*rows[r-1]).size < size)))) {
		return NULLSLOT;
	}

	if (direction == 1 || direction == -1) {
		return (slot + direction);
	}

	if (direction == 2) {
		if (r < N - 1) {
			return ((slot - start + 1) + (*rows[r+1]).start_slot);
		}
		return ((slot - start) + (*rows[r+1]).start_slot);
	}

	if (direction == 3) {
		if (r < N - 1) {
			return ((slot - start) + (*rows[r+1]).start_slot);
		}
		return ((slot - start - 1) + (*rows[r+1]).start_slot);
	}

	if (direction == -2) {
		if (r > N - 1) {
			return ((slot - start) + (*rows[r-1]).start_slot);
		}
		return ((slot - start - 1) + (*rows[r-1]).start_slot);
	}

	if (direction == -3) {
		if (r > N - 1) {
			return ((slot - start + 1) + (*rows[r-1]).start_slot);
		}
		return ((slot - start) + (*rows[r-1]).start_slot);
	}
	//should never reach here
	return 0;
}


BOOLEAN member(int slot, int places[]) {
	int length = places[0];
	int i = 1;


	while (i<=length) {
		if (places[i] == slot) {
			return TRUE;
		}
	}
	return FALSE;
}

int move_hash(int slot1, int slot2, int slot3, int direction) {
	int bigger, smaller, middle;
	int small_shift = 10;
	int mid_shift = 1000;
	int big_shift = 100000;

	if (slot2 > slot1) { /* 2 > 1*/
		if (slot3 > slot1) { /* 3 > 1*/
			smaller = slot1;
			if (slot3 > slot2) { /* 3 > 2*/
				bigger = slot3;
				middle = slot2;
			}
			else { /* 2 > 3*/
				bigger = slot2;
				middle = slot3;
			}
		}
		else {   /* 1 > 3*/
			smaller = slot3;
			middle = slot1;
			bigger = slot2;
		}
	}
	else {        /* 1 > 2*/
		if (slot3 > slot2) { /* 3 > 2*/
			smaller = slot2;
			if (slot1 > slot3) { /* 1 > 3*/
				bigger = slot1;
				middle = slot3;
			}
			else { /* 3 > 1*/
				bigger = slot3;
				middle = slot1;
			}
		}
		else {   /* 2 > 3*/
			smaller = slot3;
			middle = slot2;
			bigger = slot1;
		}
	}

	/* so do move can simply shift the pieces over in the order it gets them*/
	if (direction > 0) {
		return (direction + (small_shift * smaller) + (mid_shift * middle) + (big_shift * bigger));
	}
	else {
		return (-1 * ((-1 * direction) + (small_shift * bigger) + (mid_shift * middle) +  (big_shift * smaller)));
	}
}

struct row * makerow (int size, int start)
{
	struct row *new;
	new = (struct row *) SafeMalloc(1 * sizeof(struct row));
	(*new).size = size;
	(*new).start_slot = start;
	return new;
}

void printrow (int line, int flag) {
	int s, size, start;

	size = (*rows[line]).size;
	start = (*rows[line]).start_slot;

	for (s = 0; s < abs((N - 1) - line); s++) {
		printf ("  ");
	}
	if (flag == 0) {
		if (line > N - 1)
			printf("\\ \\  "); //Controls the beginning of (\ \  A(o)-(o) ....
		else if (line == N - 1)
			printf("| |  ");        //Controls the beginning of Middle line with B
		else
			printf("/ /  ");
	}

	//printf("%c", 2*N - 2 - line + 'A');

	for (s = 0; s < size; s++) {
		printf ("(");

		if (flag == 0) {
			if (gBoard[start + s] == '*')
				printf(" ");
			else
				printf ("%c", gBoard[start + s]);
		}
		if (flag == 1) {
			printf("%c%c", intToCoordinateX(start + s + 1), intToCoordinateY(start + s + 1));
		}

		if (s != size - 1)
			printf(")-");
		else
			printf(")  ");
	}
	if (flag == 0) {
		if (line > N - 1)
			printf("/ /"); //Controls the end of the bottom row with A(o) -(o)
		else if (line == N - 1)
			printf("| |"); //Controls the end of the Middle row with B() -()-()
		else
			printf("\\ \\");
	}


	for (s = 0; s < abs((N - 1) - line); s++) {
		if (flag == 0)
			printf ("  ");  //Controls the spaces between (\\   |)
		if (flag == 1) {
			if (N == 2)
				printf("  ");
			else
				printf ("  ");
		}
	}
}

void printlines (int line, int flag) {
	int s, line_max, temp;

	for (s = 0; s < abs((N - 1) - line); s++)
	{
		printf ("  ");
	}

	if (flag == 0)
	{
		if (line < N - 1)
			printf("/ /   ");
		else
			printf("  \\ \\ ");
	}

	if (line < N - 1)
	{
		s = 0;
	}
	else
	{
		s = 1;
		printf("  ");
	}

	for (; s < (*rows[line]).size; s++) {
		if (line < N - 1) {
			if (flag == 0)
				printf("/ \\ ");
			else
			if (N == 2)
				printf(" / \\ ");
			else
				printf("/  \\ ");
		}
		else {
			if (flag == 0)
				printf("\\ / ");
			else
			if (N == 2)
				printf(" \\ / ");
			else
				printf("\\  / ");
		}
	}

	if (flag == 0) {
		if (line < N -1)
			printf("  \\ \\");
		else
			printf("  / /");
	}


	if (line < N - 1)
		s = 0;
	else
		s = -1;

	if (flag == 0) {
		if (line < N - 1) {
			line_max = 2 * abs((N - 1) - line) - 1;
		}
		else {
			line_max = 2 * abs((N - 1) - line);
		}
	}
	else
		line_max = abs((N - 1) - line);

	temp = 1;
	for (; s < line_max; s++)
	{
		if (flag == 0)
			printf (" ");
		if (flag == 1)
		{
			if (N == 2)
				printf ("  ");
		}

		if(N == 2)
			printf ("   ");
		else if((N==3) && (flag==0) && (s==-1 || s==0))
		{
			if((N==3) && (flag==0) && (s==-1) && (temp==1))
			{
				printf("   ");
				temp++;
				s++;
			}
			else
				printf("   ");

			if(line>2)
			{
				printf(" ");
			}
		}

	}
}

int b_size (int n) {
	if (n == 1)
		return 1;

	return ((n - 1) * 6) + b_size (n - 1);
}

int maxPieces (int n) {
	if (n == 2)
		return 2;
	return n + 2 * (n - 2) + maxPieces(n - 1);
}

int def_start_pieces (int n) {
	if (n == 2)
		return 2;
	else if (n == 3)
		return 6;
	else
		return 5 + ((n - 2)*(n - 2));
}

int sum(int start, int stop) {
	if (stop == start)
		return start;
	return stop + sum(start, stop - 1);
}

int getInitialPosition() {
	return ((int)gInitialPosition);
}

/************************************************************************
**
** NAME:        ValidTextInputHelper
**
** DESCRIPTION: helps with ValidTextInput
**
************************************************************************/
BOOLEAN ValidTextInputHelper (STRING input, int i)
{
	if((input[i] >= 'a' && input[i] <= 'z') || (input[i] >= 'A' && input[i] <= 'Z'))
	{
		i++;

		if(input[i] >= '1' && input[i] <= '9')
		{
			if(validCoordinate(input[i-1], input[i]))
				return TRUE;
			else
				return FALSE;
		}
		else
			return FALSE;
	}
	else
		return FALSE;
}

/************************************************************************
**
** NAME:        coordinateToInt
**
** DESCRIPTION: Changes a move representation from coordinates (eg. A1) to an integer (eg. 11)
**
************************************************************************/
int coordinateToInt(int x, int y)
{
	int i, position = 0;

	if((x - 'a') >= 0)
		x = x - 'a' + 1;
	else
		x = x - 'A' + 1;

	y = y - '0';

	for(i = 0; i < (2*N-x-1); i++) /* count full rows from top down */
	{
		if(i < N)
		{
			position = position + N + i;
			//printf("if pos is %d i is %d\n", position, i);
		}
		else
		{
			position = position + 3*N - 2 - i;
			//printf("else pos is %d i is %d\n", position, i);
		}
	}

	//printf("donepos is %d i is %d\n", position, i);
	/* count the last partial row */
	if(i < N-1)
	{
		position = position + y - N + 1 + i;
		//printf("i is %d", i);
	}
	else
		position = position + y;

	return position;
}

/************************************************************************
**
** NAME:        intToCoordinateX
**
** DESCRIPTION: Changes a move representation from integers (eg. 11) to its X coordinate (A)
**
************************************************************************/
int intToCoordinateX(int position)
{
	int i;

	for(i = 0; i < N; i++)
	{
		if(position - (N+i) > 0)
		{
			position = position - (N+i);
			//printf("i is %d, position is %d\n", i, position);
		}
		else
			return 2*N -1 - i - 1 + 'A';
	}

	for(; i < (2*N - 1); i++)
	{
		if((position - (3*N - i - 2)) > 0)
		{
			position = position - (3*N - i - 2);
			//printf("second loop i is %d, position is %d\n", i, position);
		}
		else
			return 2*N -1 - i -1 + 'A';
	}

	return -1;
}

/************************************************************************
**
** NAME:        intToCoordinateY
**
** DESCRIPTION: Changes a move representation from integers (eg. 11) to coordinates (eg. A1)
**
************************************************************************/
int intToCoordinateY(int position)
{
	int i;

	for(i = 0; i < N; i++)
	{
		if(position - (N+i) > 0)
		{
			position = position - (N+i);
			//printf("yi is %d, position is %d\n", i, position);
		}
		else
			return position + N - i - 1 + '0';
	}

	for(; i < (2*N - 1); i++)
	{
		if((position - (3*N - i - 2)) > 0)
		{
			position = position - (3*N - i - 2);
			//printf("ysecond loop i is %d, position is %d\n", i, position);
		}
		else
			return position + '0';
	}

	return -1;
}


/************************************************************************
**
** NAME:        validCoordinate
**
** DESCRIPTION: Determines if a given coordinate is valid
**
************************************************************************/
BOOLEAN validCoordinate(int x, int y)
{
	if((x - 'a') >= 0)
		x = x - 'a' + 1;
	else
		x = x - 'A' + 1;

	y = y - '0';

	if(y <= (x-N))
	{
		//printf("invalid topleft coordinate %d,%d\n", x, y);
		return FALSE;
	}
	else if(x <= (y-N))
	{
		//printf("invalid bottomright coordinate %d,%d\n", x, y);
		return FALSE;
	}
	else if((x > (2*N - 1)) || (y > (2*N - 1)))
	{
		//printf("invalid topright coordinate %d,%d\n", x, y);
		return FALSE;
	}
	else if((x < 1) || (y < 1))
	{
		//printf("invalid bottomleft coordinate %d,%d\n", x, y);
		return FALSE;
	}
	else
		return TRUE;
}

//Time to terify

void SetupGame() {
	generic_hash_destroy();


	rows = (struct row **) SafeMalloc ((2*N - 1) * sizeof(struct row *));

	int rowsize = N, rownum, slot = 0;
	for (rownum = 0; rownum < N - 1; rownum++) {
		rows[rownum] = makerow(rowsize,slot);
		slot += rowsize;
		rowsize++;
	}
	for (rownum = N - 1; rownum <= 2*N - 2; rownum++) {
		rows[rownum] = makerow(rowsize,slot);
		slot += rowsize;
		rowsize--;
	}

	BOARDSIZE = b_size(N);


	/* long nasty thing to initialize n-sized board */

	gBoard = (char *) SafeMalloc (BOARDSIZE * sizeof(char));
	//printf("gBoard is created.\n");

	int count, x_pieces_left = PIECES, o_pieces_left = PIECES, start, size, stop;
	double space;

	for (rownum = 0; rownum < 2 * N - 1; rownum++) {
		start = (*rows[rownum]).start_slot;
		size = (*rows[rownum]).size;

		if (x_pieces_left >= size) {
			for (count = start; count < start + size; count++)
			{
				gBoard[count] = 'x';
				x_pieces_left--;
			}
		}
		else if (x_pieces_left > 0) {
			space = (size - x_pieces_left)/2;

			for (count = start; count < start + (int)floor(space); count++) {
				gBoard[count] = '*';
			}
			stop = start + (int)floor(space) + x_pieces_left;
			for (; count < stop; count++) {
				gBoard[count] = 'x';
				x_pieces_left--;
			}
			for (; count < start + size; count++) {
				gBoard[count] = '*';
			}
		}
		else if (o_pieces_left <= (BOARDSIZE - start - size)) {
			for (count = start; count < start + size; count++) {
				gBoard[count] = '*';
			}
		}
		else if (o_pieces_left > BOARDSIZE - start - size) {
			space = ((double)(size - (o_pieces_left - (BOARDSIZE - start - size))))/2;
			for (count = start; count < start + (int)ceil(space); count++) {
				gBoard[count] = '*';
			}
			stop = start + (int)ceil(space) + (o_pieces_left - (BOARDSIZE - start - size));
			for (; count < stop; count++) {
				gBoard[count] = 'o';
				o_pieces_left--;
			}
			for (; count < start + size; count++) {
				gBoard[count] = '*';
			}
		}
		else {
			for (count = start; count < start + size; count++) {
				gBoard[count] = 'o';
				o_pieces_left--;
			}
		}
	}

	SetupTierStuff();

	int init_array[10];
	init_array[0] = 'o';
	init_array[3] = 'x';
	init_array[6] = '*';
	init_array[9] = -1;

	init_array[1] = PIECES - XHITKILLS;
	init_array[2] = PIECES;
	init_array[4] = PIECES - XHITKILLS;
	init_array[5] = PIECES;
	init_array[7] = BOARDSIZE - 2 * init_array[2];
	init_array[8] = BOARDSIZE - 2 * init_array[1];

	gMax = generic_hash_init(BOARDSIZE,init_array,NULL,0);
	generic_hash_set_context( PiecesToTier(PIECES+1, PIECES+1) );

	init = hash( 1);

	gNumberOfPositions  = gMax;
	gInitialPosition    = init;
	gMinimalPosition    = init;
}

/* TIERS: x*(PIECES+1) + o = T
          T % (PIECES+1)  = 0
            / (PIECES+1) = x
 */

void FreeHelper(struct row ** rows)
{
	int i;
	//printf("in Freehelper");

	for(i=0; i < (2*N - 1); i++)
	{
		SafeFree(rows[i]);
		//printf("Freeing i = %d", i);
	}
	// printf("end");
	//SafeFree(rows);
}

TIER BoardToTier(char* board) {
	int x = 0, o = 0, count;

	for (count = 0; count < BOARDSIZE; count++) {
		if (board[count] == 'x')
		{
			x++;
		}
		else if (board[count] == 'o')
		{
			o++;
		}
	}
	return PiecesToTier(x, o);
}


void TierToPieces(TIER tierNum, int *xCount, int *oCount)
{
	int x, o;

	o = tierNum % (PIECES + 1);
	x = tierNum / (PIECES + 1);

	*(xCount) = x;
	*(oCount) = o;
}


TIER PiecesToTier( int xCount, int oCount)
{
	return xCount*(PIECES+1)+oCount;
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
	int piecesArray[10] = { 'x', 0, 0, 'o', 0, 0, '*', 0, 0, -1 };
	int x,o;

	generic_hash_custom_context_mode(TRUE);
	for (x =0; x <= PIECES; x++)
	{
		for(o=0; o<=PIECES; o++)
		{

			piecesArray[1] = x;
			piecesArray[2] = x;
			piecesArray[4] = o;
			piecesArray[5] = o;
			// Blanks = tier
			piecesArray[7] = piecesArray[8] = BOARDSIZE - (x+o);
			// make the hashes
			generic_hash_init(BOARDSIZE, piecesArray, NULL, 0);
			generic_hash_set_context ( PiecesToTier(x, o) );
		}
	}

	// Initial
	gInitialTier = BoardToTier(gBoard);
	generic_hash_context_switch(gInitialTier);
	gInitialTierPosition = hash(1);
}

// children = always me and one below
TIERLIST* TierChildren(TIER tier) {
	TIERLIST* list = NULL;
	int x, o;
	TierToPieces(tier, &x, &o);

	list = CreateTierlistNode(tier, list);
	if((x > (PIECES - XHITKILLS)) && !(o == (PIECES - XHITKILLS)))
		list = CreateTierlistNode(PiecesToTier(x-1, o), list);
	if((o > (PIECES - XHITKILLS)) && !(x == (PIECES - XHITKILLS)))
		list = CreateTierlistNode(PiecesToTier(x, o-1), list);

	return list;
}

TIERPOSITION NumberOfTierPositions(TIER tier) {
	generic_hash_context_switch(tier);
	return generic_hash_max_pos();
}


void unhash (POSITION position, int* turn)
{



	if (gHashWindowInitialized) {
		TIERPOSITION tierPos; TIER tier;
		gUnhashToTierPosition(position, &tierPos, &tier);
		generic_hash_context_switch(tier);
		(*turn) = generic_hash_turn(tierPos);
		generic_hash_unhash(tierPos, gBoard);
	} else {
		(*turn) = generic_hash_turn(position);
		generic_hash_unhash(position, gBoard);
	}

}

POSITION hash (int turn)
{
	if (gHashWindowInitialized) {
		TIER tier = BoardToTier(gBoard);
		generic_hash_context_switch(tier);
		TIERPOSITION tierPos = generic_hash_hash(gBoard, turn);
		return gHashToWindowPosition(tierPos, tier);
	} else return generic_hash_hash(gBoard, turn);
}


// Tier = Number of pieces left to place.
STRING TierToString(TIER tier) {
	int x, o;
	TierToPieces(tier, &x, &o);
	STRING tierStr = (STRING) SafeMalloc(sizeof(char)*16);
	sprintf(tierStr, "x = %d; o = %d", x, o);
	return tierStr;
}


// $Log: not supported by cvs2svn $
// Revision 1.45  2007/02/08 05:23:43  dmchan
// fixed unsigned int call
//
// Revision 1.44  2006/12/19 20:00:50  arabani
// Added Memwatch (memory debugging library) to gamesman. Use 'make memdebug' to compile with Memwatch
//
// Revision 1.43  2006/12/19 09:03:17  koolswim88
// *** empty log message ***
//
// Revision 1.42  2006/12/07 04:25:57  koolswim88
// *** empty log message ***
//
// Revision 1.40  2006/12/07 02:10:30  koolswim88
// *** empty log message ***
//
// Revision 1.39  2006/11/21 03:01:17  jerricality
// Fix some bugs on tierification
//
// -Now we no longer solve for the last tier that we can never get to.
//
// Revision 1.38  2006/11/19 00:22:24  jerricality
// Tierization is complete - Jerry
//
// Revision 1.37  2006/11/02 02:18:25  koolswim88
// *** empty log message ***
//
// Revision 1.36  2006/10/17 10:45:19  max817
// HUGE amount of changes to all generic_hash games, so that they call the
// new versions of the functions.
//
// Revision 1.35  2006/10/03 08:17:20  jerricality
// *** empty log message ***
//
// Revision 1.34  2006/10/03 06:41:52  jerricality
// *** empty log message ***
//
// Revision 1.33  2006/08/21 23:52:47  dmchan
// commented out calls to fflush(stdin) and changed to GetMyInt() and GetMyChar() where appropriate
//
// Revision 1.32  2006/02/26 08:31:15  kmowery
//
// Changed MToS to MoveToString
//
// Revision 1.31  2006/02/13 08:34:47  kmowery
//
// Added MoveToString and set gMoveToStringFunPtr (required for visual value history).
//
// Revision 1.30  2006/02/03 06:08:39  hevanm
// fixed warnings. I will leave the real bugs to retro hehehehe.
//
// Revision 1.29  2006/01/03 00:19:34  hevanm
// Added types.h. Cleaned stuff up a little. Bye bye gDatabase.
//
// Revision 1.28  2005/12/27 10:57:50  hevanm
// almost eliminated the existance of gDatabase in all files, with some declarations commented earlier that need to be hunt down and deleted from the source file.
//
// Revision 1.27  2005/12/08 07:26:32  esiroker
// Made compilable on Mac.
//
// Revision 1.26  2005/12/08 07:16:56  ogren
// changed second definition of max to line_max.  dont really know if the name is appropriate. -Elmer
//
// Revision 1.25  2005/09/15 03:56:08  ogren
// added : $, : $, changed kGameName = Abalone
//
POSITION StringToPosition(char* board) {
	POSITION pos = INVALID_POSITION;
	GetValue(board, "pos", GetUnsignedLongLong, &pos);
	return pos;
}

char* PositionToString(POSITION pos) {
	int whoseMove;
	unhash(pos, &whoseMove);
	return MakeBoardString(gBoard,
			       "turn", StrFromI(whoseMove),
			       "pos", StrFromI(pos),
	                       "");
}

char * PositionToEndData(POSITION pos) {
	return NULL;
}
