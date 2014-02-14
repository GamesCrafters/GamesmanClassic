/************************************************************************
**
** NAME:        mswans.c
**
** DESCRIPTION: Dragons & Swans
**
** AUTHOR:      Anh Thai
**              Sandra Tang
**
** DATE:        12/8/02
**
**	8-21-06		change to GetMyInt();
**************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/

#include <stdio.h>
#include "gamesman.h"
#include "hash.h"

/* With 0-4 dragons and 0-12 swans, we get the incredible
* 14593151 * 64 => 933,961,664 positions!!! (too many) */

/* We're going to try to hardcode 1 dragon only, yielding 522,352 positions
 * times 64 ways of changing the extra information (turn, phase, swans-left)
 * = 33,430,528 */

POSITION gNumberOfPositions  = 33430528;
/* position shifted over by 6 bits: 1 for whosTurn, 1 for phase, 4 for numSwans */
/* POSITION gInitialPosition    = (2285 << 6) + 12;  */  /* for full 4-dragon bd */
POSITION gInitialPosition;
POSITION kBadPosition        = -1;
STRING kAuthorName         = "Anh Thai and Sandra Tang";
STRING kGameName           = "Dragons & Swans";
BOOLEAN kPartizan           = TRUE;
BOOLEAN kDebugMenu          = TRUE;
BOOLEAN kGameSpecificMenu   = TRUE;
BOOLEAN kTieIsPossible      = FALSE;
BOOLEAN kLoopy               = TRUE;
BOOLEAN kDebugDetermineValue = FALSE;
void*    gGameSpecificTclInit = NULL;

STRING kHelpGraphicInterface =
        "";

STRING kHelpTextInterface    =
        "On the dragon's turn, use the LEGEND to determine which numbers to choose (between 1 and 16, with 1 at the upper left and 16 at the lower right) to correspond to the location of your dragon and the empty orthogonally-adjacent position you wish to move that dragon to. On the swan's turn, if the swan still has pieces remaining from the initial pile, choose a number between 1 and 16 to correspond to the location you want to place your swan. If the swan has no remaining swans from the inital pile, choose numbers between 1 and 16 to correspond to the location of your swan and the emtpy orthogonally-adjacent position you wish to move that swan to.";

STRING kHelpOnYourTurn =
        "If it is the swan's turn and there are swans remaining from the intial pile place a swan on one of the empty board positions. If it is the swan's turn and there are no swans remaining in the initial pile, or it is the dragon's turn, move one of your pieces to an orthogonally-adjacent empty board position.";

STRING kHelpStandardObjective =
        "Swans will try to TRAP all the dragons and dragons will try to EAT all the swans";

STRING kHelpReverseObjective =
        "Dragons will try to be TRAPPED and swans will try to be EATEN.";

STRING kHelpTieOccursWhen =   /* Should follow 'A Tie occurs when... */
                            "";

STRING kHelpExample =
"         ( 1  2  3  4 )           : X - - X     PLAYER O's turn\n\
LEGEND:  ( 5  6  7  8 )  TOTAL:   : - - - -\n\
         ( 9 10 11 12 )           : - - - -\n\
         (13 14 15 16 )           : X - - X\n\n\
You have 12 swans left.\n\
     Dan's move [(u)ndo/1-16] : 6\n\n\
         ( 1  2  3  4 )           : X - - X     PLAYER X's turn\n\
LEGEND:  ( 5  6  7  8 )  TOTAL:   : - O - -\n\
         ( 9 10 11 12 )           : - - - -\n\
         (13 14 15 16 )           : X - - X\n\n\
Computer's move              : 13 14\n\n\
         ( 1  2  3  4 )           : X - - X     PLAYER O's turn\n\
LEGEND:  ( 5  6  7  8 )  TOTAL:   : - O - -\n\
         ( 9 10 11 12 )           : - - - -\n\
         (13 14 15 16 )           : - X - X\n\n\
You have 11 swans left.\n\
     Dan's move [(u)ndo/1-16] : 13\n\n\
         ( 1  2  3  4 )           : X - - X     PLAYER X's turn\n\
LEGEND:  ( 5  6  7  8 )  TOTAL:   : - O - -\n\
         ( 9 10 11 12 )           : - - - -\n\
         (13 14 15 16 )           : O X - X\n\n\
Computer's move              : 16 15\n\n\
         ( 1  2  3  4 )           : X - - X     PLAYER O's turn\n\
LEGEND:  ( 5  6  7  8 )  TOTAL:   : - O - -\n\
         ( 9 10 11 12 )           : - - - -\n\
         (13 14 15 16 )           : O X X -\n\n\
You have 10 swans left.\n\
     Dan's move [(u)ndo/1-16] : 16\n\n\
         ( 1  2  3  4 )           : X - - X     PLAYER X's turn\n\
LEGEND:  ( 5  6  7  8 )  TOTAL:   : - O - -\n\
         ( 9 10 11 12 )           : - - - -\n\
         (13 14 15 16 )           : O X X O\n\n\
Computer's move              : 15 11\n\n\
         ( 1  2  3  4 )           : X - - X     PLAYER O's turn\n\
LEGEND:  ( 5  6  7  8 )  TOTAL:   : - O - -\n\
         ( 9 10 11 12 )           : - - X -\n\
         (13 14 15 16 )           : O X - O\n\n\
You have 9 swans left.\n\
     Dan's move [(u)ndo/1-16] : 15\n\n\
         ( 1  2  3  4 )           : X - - X     PLAYER X's turn\n\
LEGEND:  ( 5  6  7  8 )  TOTAL:   : - O - -\n\
         ( 9 10 11 12 )           : - - X -\n\
         (13 14 15 16 )           : O X O O\n\n\
Computer's move              : 11 12\n\
         ( 1  2  3  4 )           : X - - X     PLAYER O's turn\n\
LEGEND:  ( 5  6  7  8 )  TOTAL:   : - O - -\n\
         ( 9 10 11 12 )           : - - - X\n\
         (13 14 15 16 )           : O X O O\n\n\
You have 8 swans left.\n\
     Dan's move [(u)ndo/1-16] : 10\n\n\
         ( 1  2  3  4 )           : X - - X     PLAYER X's turn\n\
LEGEND:  ( 5  6  7  8 )  TOTAL:   : - O - -\n\
         ( 9 10 11 12 )           : - O - X\n\
         (13 14 15 16 )           : O X O O\n\n\
Computer's move              : 1 5\n\n\
         ( 1  2  3  4 )           : - - - X     PLAYER O's turn\n\
LEGEND:  ( 5  6  7  8 )  TOTAL:   : X O - -\n\
         ( 9 10 11 12 )           : - O - X\n\
         (13 14 15 16 )           : O X O O\n\n\
You have 7 swans left.\n\
     Dan's move [(u)ndo/1-16] : 7\n\n\
         ( 1  2  3  4 )           : - - - X     PLAYER X's turn\n\
LEGEND:  ( 5  6  7  8 )  TOTAL:   : X O O -\n\
         ( 9 10 11 12 )           : - O - X\n\
         (13 14 15 16 )           : O X O O\n\n\
Computer's move              : 4 8\n\n\
         ( 1  2  3  4 )           : - - - -     PLAYER O's turn\n\
LEGEND:  ( 5  6  7  8 )  TOTAL:   : X O O X\n\
         ( 9 10 11 12 )           : - O - X\n\
         (13 14 15 16 )           : O X O O\n\n\
You have 6 swans left.\n\
     Dan's move [(u)ndo/1-16] : 4\n\n\
         ( 1  2  3  4 )           : - - - O     PLAYER X's turn\n\
LEGEND:  ( 5  6  7  8 )  TOTAL:   : X O O X\n\
         ( 9 10 11 12 )           : - O - X\n\
         (13 14 15 16 )           : O X O O\n\n\
Computer's move              : 5 1\n\n\
         ( 1  2  3  4 )           : X - - O     PLAYER O's turn\n\
LEGEND:  ( 5  6  7  8 )  TOTAL:   : - O O X\n\
         ( 9 10 11 12 )           : - O - X\n\
         (13 14 15 16 )           : O X O O\n\n\
You have 5 swans left.\n\
     Dan's move [(u)ndo/1-16] : 11\n\n\
         ( 1  2  3  4 )           : X - - O     PLAYER X's turn\n\
LEGEND:  ( 5  6  7  8 )  TOTAL:   : - O O X\n\
         ( 9 10 11 12 )           : - O O X\n\
         (13 14 15 16 )           : O X O O\n\n\
Computer's move              : 1 5\n\n\
         ( 1  2  3  4 )           : - - - O     PLAYER O's turn\n\
LEGEND:  ( 5  6  7  8 )  TOTAL:   : X O O X\n\
         ( 9 10 11 12 )           : - O O X\n\
         (13 14 15 16 )           : O X O O\n\n\
You have 4 swans left.\n\
     Dan's move [(u)ndo/1-16] : 9\n\n\
         ( 1  2  3  4 )           : - - - O     PLAYER X's turn\n\
LEGEND:  ( 5  6  7  8 )  TOTAL:   : X O O X\n\
         ( 9 10 11 12 )           : O O O X\n\
         (13 14 15 16 )           : O X O O\n\n\
Computer's move              : 5 1\n\n\
         ( 1  2  3  4 )           : X - - O     PLAYER O's turn\n\
LEGEND:  ( 5  6  7  8 )  TOTAL:   : - O O X\n\
         ( 9 10 11 12 )           : O O O X\n\
         (13 14 15 16 )           : O X O O\n\n\
You have 3 swans left.\n\
     Dan's move [(u)ndo/1-16] : 5\n\n\
         ( 1  2  3  4 )           : X - - O     PLAYER X's turn\n\
LEGEND:  ( 5  6  7  8 )  TOTAL:   : O O O X\n\
         ( 9 10 11 12 )           : O O O X\n\
         (13 14 15 16 )           : O X O O\n\n\
Computer's move              : 1 2\n\n\
         ( 1  2  3  4 )           : - X - O     PLAYER O's turn\n\
LEGEND:  ( 5  6  7  8 )  TOTAL:   : O O O X\n\
         ( 9 10 11 12 )           : O O O X\n\
         (13 14 15 16 )           : O X O O\n\n\
You have 2 swans left.\n\
     Dan's move [(u)ndo/1-16] : 1\n\n\
         ( 1  2  3  4 )           : O X - O     PLAYER X's turn\n\
LEGEND:  ( 5  6  7  8 )  TOTAL:   : O O O X\n\
         ( 9 10 11 12 )           : O O O X\n\
         (13 14 15 16 )           : O X O O\n\n\
Computer's move              : 2 3\n\n\
         ( 1  2  3  4 )           : O - X O     PLAYER O's turn\n\
LEGEND:  ( 5  6  7  8 )  TOTAL:   : O O O X\n\
         ( 9 10 11 12 )           : O O O X\n\
         (13 14 15 16 )           : O X O O\n\n\
You have 1 swans left.\n\
     Dan's move [(u)ndo/1-16] : 2\n\n\
         ( 1  2  3  4 )           : O O X O     PLAYER X's turn\n\
LEGEND:  ( 5  6  7  8 )  TOTAL:   : O O O X\n\
         ( 9 10 11 12 )           : O O O X\n\
         (13 14 15 16 )           : O X O O\n\n\n\
Excellent! You won!"                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             ;


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

#define BOARDSIZE        16           /* 4x4 board */
#define BADSLOT         -2           /* You've moved off the board in a bad way */

typedef int SLOT;     /* A slot is the place where a piece moves from or to */

char gBoard[] = { 'b', 'b', 'b', 'b', 'b', 'b', 'b', 'b', 'b', 'b', 'b', 'b', 'b', 'b', 'b', 'b'};

BOOLEAN gToTrapIsToWin = TRUE;  /* Being stuck is when you can't move. */

#define MIN_DRAGONS 1
#define MAX_DRAGONS 4

int gNumDragons = MIN_DRAGONS;

/* local prototypes */
void generic_hash_unhash2(int, char*, char*, int*, int*);
POSITION generic_hash_hash2(char*, char, int, int);
BOOLEAN OkMove(char[], char, SLOT, int);
BOOLEAN CantMove(POSITION);
void MoveToSlots(MOVE theMove,SLOT *fromSlot, SLOT *toSlot);
MOVE SlotsToMove (SLOT fromSlot, SLOT toSlot);
SLOT GetToSlot(char *theBoard, SLOT fromSlot, int direction, char whosTurn);

STRING MoveToString( MOVE );

void InitializeGame()
{
	int numSwans = 12;
	int numDragons = gNumDragons;	

	int pieces_array[] = { 'o', 0, numSwans, 'x', numDragons, numDragons, 'b', 0, BOARDSIZE, -1};
	gNumberOfPositions = generic_hash_init(BOARDSIZE, pieces_array, NULL, 0)<<1<<4;
	char initialBoard[] = {'x', 'b', 'b', 'b',
		               'b', 'b', 'b', 'b',
		               'b', 'b', 'b', 'b',
		               'b', 'b', 'b', 'b'};
	if (numDragons>1) initialBoard[15] = 'x';
	if (numDragons>2) initialBoard[3] = 'x';
	if (numDragons>3) initialBoard[12] = 'x';

	gInitialPosition = generic_hash_hash2(initialBoard, 'o', 1, numSwans);

	gMoveToStringFunPtr = &MoveToString;
}

void FreeGame()
{
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
	do {
		printf("\n\t----- Game-specific options for %s -----\n\n", kGameName);

		printf("\tCurrent Initial Position:\n");
		PrintPosition(gInitialPosition, gPlayerName[kPlayerOneTurn], kHumansTurn);

		printf("\tI)\tChoose the (I)nitial position\n");
		printf("\tT)\t(T)rapping opponent toggle from %s to %s\n",
		       gToTrapIsToWin ? "GOOD (WINNING)" : "BAD (LOSING)",
		       !gToTrapIsToWin ? "GOOD (WINNING)" : "BAD (LOSING)");
		printf("\tD)\tChange the number of (D)ragons (Currently %d)\n",
		       gNumDragons);

		printf("\n\n\tb)\t(B)ack = Return to previous activity.\n");
		printf("\n\nSelect an option: ");

		switch(GetMyChar()) {
		case 'Q': case 'q':
			ExitStageRight();
		case 'H': case 'h':
			HelpMenus();
			break;
		case 'I': case 'i':
			gInitialPosition = GetInitialPosition();
			break;
		case 'T': case 't':
			gToTrapIsToWin = !gToTrapIsToWin;
			break;
		case 'D': case 'd':
			printf("How many dragons [%d-%d]? ", MIN_DRAGONS, MAX_DRAGONS);
			/*scanf("%d", &gNumDragons);*/
			gNumDragons = GetMyInt();
			while (gNumDragons > MAX_DRAGONS || gNumDragons < MIN_DRAGONS) {
				printf("Invalid entry. Please try again\n");
				printf("How many dragons [%d-%d]? ", MIN_DRAGONS, MAX_DRAGONS);
				/*scanf("%d", &gNumDragons);*/
				gNumDragons = GetMyInt();
			}
			InitializeGame();
			break;
		case 'b': case 'B':
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
	gToTrapIsToWin = (BOOLEAN) theOptions[0];
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
** CALLS:       MoveToSlots(MOVE,*SLOT,*SLOT)
**
************************************************************************/

POSITION DoMove(thePosition, theMove)
POSITION thePosition;
MOVE theMove;
{
	SLOT fromSlot, toSlot;
	char whosTurn;
	int phase, numSwans;
	
	generic_hash_unhash2(thePosition, gBoard, &whosTurn, &phase, &numSwans);
	theMove = theMove >> 1; /* shift phase bit off of theMove */
	if ((whosTurn == 'o') && (numSwans != 0)) { /* placing swans on the board */
		numSwans--;
		if (numSwans == 0)
			phase = 2;
		gBoard[theMove] = 'o';
		return((POSITION) generic_hash_hash2(gBoard, 'x', phase, numSwans));
	}
	else if ((whosTurn == 'x') || (numSwans == 0)) { /* sliding or jumping pieces */
		MoveToSlots(theMove, &fromSlot, &toSlot);
		if ((fromSlot-toSlot) == 2)
			gBoard[(fromSlot-1)] = 'b';
		else if ((fromSlot-toSlot) == -2)
			gBoard[(fromSlot+1)] = 'b';
		else if ((fromSlot-toSlot) == 8)
			gBoard[(fromSlot-4)] = 'b';
		else if ((fromSlot-toSlot) == -8)
			gBoard[(fromSlot+4)] = 'b';
		gBoard[toSlot] = gBoard[fromSlot];
		gBoard[fromSlot]='b';
		return((POSITION) generic_hash_hash2(gBoard,(whosTurn == 'o' ? 'x' : 'o'),phase, numSwans));
	}
	else
		return(Continue);

}

/************************************************************************
**
** NAME:        GetInitialPosition
**
** DESCRIPTION: Ask the user for an initial position for testing. Store
**              it in the space pointed to by initialPosition;
**
** OUTPUTS:     POSITION initialPosition : The position returned
**
************************************************************************/

POSITION GetInitialPosition()
{
	char whosTurn;
	signed char c;
	int numX, numO;
	int i, phase, numSwans;


	printf("\n\n\t----- Get Initial Position -----\n");
	printf("\n\tPlease input the position to begin with.\n");
	printf("\tNote that it should be in the following format:\n\n");
	printf("o - - - \no - - -            <----- EXAMPLE \n- - x x\n- - x x\n\n");

	getchar();
	do {
		numX = numO = 0;
		i = 0;
		while(i < BOARDSIZE && (c = getchar()) != EOF) {
			if(c == 'x' || c == 'X') {
				numX++;
				gBoard[i++] = 'x';
			}
			else if(c == 'o' || c == 'O' || c == '0') {
				numO++;
				gBoard[i++] = 'o';
			}
			else if(c == '-')
				gBoard[i++] = 'b';
			else
				; /* do nothing */
		}

		getchar();
		printf("\nNow, whose turn is it? [O/X] : ");
		/*scanf("%c",&c);*/
		c = GetMyChar();
		if(c == 'x' || c == 'X')
			whosTurn = 'x';
		else
			whosTurn = 'o';
		getchar();
		printf("\nHow many swans do you want to start with? [0-%d] : ", 12-numO);
		/*scanf("%d", &numSwans);*/
		numSwans = GetMyInt();

		if (numSwans < 0 || numSwans+numO > 12 || numX < MIN_DRAGONS || numX > MAX_DRAGONS)
			printf("\n\nInvalid board. Please try again\n\n");
	} while (numSwans < 0 || numSwans+numO > 12 || numX < MIN_DRAGONS || numX > MAX_DRAGONS);

	if(numSwans != 0)
		phase = 1;
	else
		phase = 2;
	return((POSITION) generic_hash_hash2(gBoard, whosTurn, phase, numSwans));
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

void PrintComputersMove(computersMove,computersName)
MOVE computersMove;
STRING computersName;
{
	SLOT fromSlot,toSlot;
	int phase;
	phase = computersMove & 1; /* phase 1 = 0, phase 2 = 1 */
	computersMove = computersMove >> 1;
	if(phase == 0)
		printf("%8s's move              : %2d\n", computersName, computersMove+1);
	else {
		MoveToSlots(computersMove,&fromSlot,&toSlot);
		printf("%8s's move              : %d %d\n", computersName,
		       fromSlot+1,toSlot+1);
	}
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
** CALLS:       BOOLEAN AllFilledIn()
**              generic_hash_unhash2()
**              char OnlyPlayerLeft(theBoard)
**
************************************************************************/

VALUE Primitive(position)
POSITION position;
{
	BOOLEAN AllFilledIn();
	char OnlyPlayerLeft();
	char whosTurn;
	char theBoard[16];
	int phase, numSwans;

	generic_hash_unhash2(position, theBoard, &whosTurn, &phase, &numSwans);

	if ((OnlyPlayerLeft(theBoard) == 'x') && (numSwans == 0) && (whosTurn == 'x'))
		return(gStandardGame ? win : lose); /* cause you're the only one left */

	if(AllFilledIn(theBoard))
		return(lose);
	else if(CantMove(position) && (whosTurn == 'x'))
		return(gToTrapIsToWin ? lose : win);
	else if(CantMove(position) && (numSwans == 0)) /* the other player just won */
		return(gToTrapIsToWin ? lose : win); /* !gStandardGame */
	else
		return(undecided);        /* no one has won yet */
}

BOOLEAN CantMove(position)
POSITION position;
{
	MOVELIST *ptr, *GenerateMoves();
	BOOLEAN cantMove;
	ptr = GenerateMoves(position);
	cantMove = (ptr == NULL);
	FreeMoveList(ptr);
	return(cantMove);
}


char OnlyPlayerLeft(theBoard)
char theBoard[16];
{
	int i;
	BOOLEAN sawO = FALSE, sawX = FALSE;
	for(i = 0; i < BOARDSIZE; i++) {
		sawO |= (theBoard[i] == 'o');
		sawX |= (theBoard[i] == 'x');
	}
	if(sawX && !sawO)
		return('x');
	else if (sawO && !sawX)
		return('o');
	else if (sawO && sawX)
		return('b');
	else {
		printf("Error in OnlyPlayerLeft! the board is blank!!!\n");
		return('b');
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
** CALLS:       generic_hash_unhash2()
**              GetValueOfPosition()
**              GetPrediction()
**
************************************************************************/

void PrintPosition(position,playerName,usersTurn)
POSITION position;
STRING playerName;
BOOLEAN usersTurn;
{
	int i;
	char whosTurn;
	int phase, numSwans;
	char theBoard[16];

	generic_hash_unhash2(position,theBoard,&whosTurn, &phase, &numSwans);

	printf("\n         ( 1  2  3  4 )           : ");
	for(i=0; i<4; i++) {
		if(theBoard[i] == 'b')
			printf("- ");
		else
			printf("%c ", theBoard[i]);
	}
	printf("    PLAYER %c's turn\n", whosTurn);
	printf("LEGEND:  ( 5  6  7  8 )  TOTAL:   : ");
	for(i=4; i<8; i++) {
		if(theBoard[i] == 'b')
			printf("- ");
		else
			printf("%c ", theBoard[i]);
	}
	printf("\n         ( 9 10 11 12 )           : ");
	for(i=8; i<12; i++) {
		if(theBoard[i] == 'b')
			printf("- ");
		else
			printf("%c ", theBoard[i]);
	}
	printf("\n         (13 14 15 16 )           : ");
	for(i=12; i<16; i++) {
		if(theBoard[i] == 'b')
			printf("- ");
		else
			printf("%c ", theBoard[i]);
	}
	printf("%s\n", GetPrediction(position,playerName,usersTurn));
	printf("                              Phase: %d Swans Remaining: %d\n\n", phase, numSwans);
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
** CALLS:       *CreateMovelistNode()
**              Primitive()
**              generic_hash_unhash2()
**
************************************************************************/

MOVELIST *GenerateMoves(position)
POSITION position;
{
	MOVELIST *head = NULL;
	MOVELIST *CreateMovelistNode();
	MOVE tempMove;
	int i,j,k, l, phase, numSwans;
	char theBoard[16];
	char whosTurn;
	generic_hash_unhash2(position, theBoard, &whosTurn, &phase, &numSwans);
	if((whosTurn == 'o') && (numSwans != 0)) {
		for(k = 0; k < BOARDSIZE; k++) {
			if(theBoard[k] == 'b') {
				/* shift move over and add the phase bit,
				   bit is off for phase 1, on for phase 2 */
				l = k << 1;
				head = CreateMovelistNode(l,head);
			}
		}
	}
	else{
		for(i = 0; i < BOARDSIZE; i++) { /* enumerate over all FROM slots */
			for(j = 0; j < 8; j++) { /* enumerate over all directions */
				if(OkMove(theBoard,whosTurn,(SLOT)i,j)) {
					tempMove = SlotsToMove((SLOT)i, GetToSlot(theBoard, i, j, whosTurn));
					/* shift tempMove over and add the phase bit */
					tempMove = tempMove << 1;
					tempMove++; /* phase bit is 1 when phase is 2 */
					head = CreateMovelistNode(tempMove,head);
				}
			}
		}
	}
	return(head);
}

BOOLEAN OkMove(theBoard,whosTurn,fromSlot,direction)
char theBoard[16];
char whosTurn;
SLOT fromSlot;
int direction;
{
	SLOT toSlot;
	toSlot = GetToSlot(theBoard,fromSlot,direction,whosTurn);
	return((theBoard[fromSlot] == whosTurn) &&
	       (toSlot != BADSLOT) &&
	       (theBoard[toSlot] == 'b'));
}

SLOT GetToSlot(theBoard,fromSlot,direction,whosTurn)
char theBoard[16];
SLOT fromSlot;
int direction;
char whosTurn;
{
	/* direction:
	       5

	       1                   0

	   4  0  'x'  2  6        3  '0'  1

	       3                   2

	       7
	 */
	if (((whosTurn == 'x') &&
	     ((direction == 2 && ((fromSlot % 4) == 3)) || (direction == 1 && (fromSlot < 4)) ||
	      (direction == 3 && (fromSlot > 11)) || (direction == 4 && (((fromSlot % 4) <= 1) ||
	                                                                 (theBoard[(fromSlot-1)] == 'b') || (theBoard[(fromSlot-1)] == 'x'))) ||
	      (direction == 5 && ((fromSlot < 8) || (theBoard[(fromSlot - 4)] == 'b') ||
	                          (theBoard[(fromSlot-4)] == 'x'))) || (direction == 6 && (((fromSlot % 4) >= 2) ||
	                                                                                   (theBoard[(fromSlot+1)] == 'b') || (theBoard[(fromSlot+1)] == 'x'))) ||
	      (direction == 7 && ((fromSlot > 7) || (theBoard[(fromSlot + 4)] == 'b') ||
	                          (theBoard[(fromSlot+4)] == 'x'))) || (direction == 0 && ((fromSlot % 4) == 0)))) ||
	    ((whosTurn == 'o') && ((direction == 3 && (fromSlot > 11)) ||
	                           (direction == 4) || (direction == 5) || (direction == 6) || (direction == 7) ||
	                           (direction == 2 && ((fromSlot % 4) == 3)) || (direction == 0 && ((fromSlot % 4) == 0)) ||
	                           (direction == 1 && (fromSlot < 4)))))
		return(BADSLOT);

	else if (whosTurn == 'x' && direction == 4) /* far west */
		return (fromSlot - 2);
	else if (whosTurn == 'x' && direction == 5) /* far north */
		return (fromSlot - 8);
	else if (whosTurn == 'x' && direction == 6) /* far east */
		return (fromSlot + 2);
	else if (whosTurn == 'x' && direction == 7) /* far south */
		return (fromSlot + 8);
	else if (direction == 0)        /* west */
		return (fromSlot - 1);
	else if (direction == 1)        /* north */
		return (fromSlot - 4);
	else if (direction == 2)        /* east */
		return (fromSlot + 1);
	else if (direction == 3)        /* south */
		return(fromSlot + 4);
	else
		BadElse("GetToSlot");

	//never reaches here
	return 0;
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
	int phase, numSwans;
	char whosTurn;
	generic_hash_unhash2(thePosition, gBoard, &whosTurn, &phase, &numSwans);

	do {
		if((whosTurn == 'o') && (numSwans != 0)) {
			printf("%8s's move [(u)ndo/1-16] : ", playerName);
			ret = HandleDefaultTextInput(thePosition, theMove, playerName);
			if(ret != Continue)
				return(ret);
		}
		else if((whosTurn == 'x') || (numSwans == 0)) {
			printf("%8s's move [(u)ndo/1-16 1-16] : ", playerName);
			ret = HandleDefaultTextInput(thePosition, theMove, playerName);
			if(ret != Continue)
				return(ret);
		}
	} while (TRUE);
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
	SLOT fromSlot, toSlot;
	int ret;
	if(input[1] == '\0') /* one digit input */
		return((input[0] >= '1') && (input[0] <= '9'));
	else if(input[2] == '\0') { /* two digit input */
		return((input[0] == '1') && (input[1] >= '0') && (input[1] <= '6'));
	}
	else {
		ret = sscanf(input,"%d %d", &fromSlot, &toSlot);
		return((fromSlot <= 16) && (fromSlot >= 1) && (toSlot <= 16) && (toSlot >= 1));
	}
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
	MOVE SlotsToMove();
	SLOT fromSlot, toSlot;
	int ret;
	MOVE tempMove;
	if((input[1] == '\0') || (input[2] == '\0')) { /* single number input */
		ret = (((atoi(input)) - 1) * 2);
		return((MOVE) ret);
	}
	else {
		ret = sscanf(input,"%d %d", &fromSlot, &toSlot);
		fromSlot--;     /* user input is 1-16, our rep. is 0-15 */
		toSlot--;   /* user input is 1-16, our rep. is 0-15 */
		tempMove = SlotsToMove(fromSlot, toSlot);
		tempMove *= 2;
		tempMove++;
		return(tempMove);
	}
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
	STRING m = (STRING) SafeMalloc( 10 );
	SLOT fromSlot, toSlot;
	int phase;
	phase = theMove & 1;
	theMove = theMove >> 1;
	if(phase == 0)
		sprintf( m, "%d", theMove + 1);
	else {
		MoveToSlots(theMove,&fromSlot,&toSlot);
		/* The plus 1 is because the user thinks it's 1-16, but MOVE is 0-15 */
		sprintf( m, "[ %d %d ] ", fromSlot + 1, toSlot + 1);
	}

	return m;
}

/************************************************************************
*************************************************************************
**         EVERYTHING BELOW THESE LINES IS LOCAL TO THIS FILE
*************************************************************************
************************************************************************/

/************************************************************************
**
** NAME:        generic_hash_unhash2
**
** DESCRIPTION: convert an internal position to that of a BlankOX.
**
** INPUTS:      POSITION thePos     : The position input.
**              BlankOX *theBlankOx : The converted BlankOX output array.
**
** CALLS:       BadElse()
**
************************************************************************/

void generic_hash_unhash2(int hashed, char* dest, char* whosTurn, int* phase, int* numSwans)
{
	int whoseTurnTemp, phaseTemp;

	// isolate numswans
	*numSwans = (hashed & 0xF);
	hashed = hashed >> 4;

	// isolate phase
	phaseTemp = (hashed & 1);
	if(phaseTemp == 0)
		*phase = 1;
	else
		*phase = 2;
	hashed = hashed >> 1;

	// isolate generic_hash_turn
	whoseTurnTemp = generic_hash_turn(hashed);
	if(whoseTurnTemp == 1)
		*whosTurn = 'o';
	else
		*whosTurn = 'x';

	generic_hash_unhash(hashed, dest);
}

/************************************************************************
**
** NAME:        MoveToSlots
**
** DESCRIPTION: convert an internal move to that of two slots
**
** INPUTS:      MOVE theMove    : The move input.
**              SLOT *fromSlot  : The slot the piece moves from (output)
**              SLOT *toSlot    : The slot the piece moves to   (output)
**
************************************************************************/

void MoveToSlots(theMove, fromSlot, toSlot)
MOVE theMove;
SLOT *fromSlot, *toSlot;
{
	*fromSlot = theMove % (BOARDSIZE+1);
	*toSlot   = theMove / (BOARDSIZE+1);
}

/************************************************************************
**
** NAME:        SlotsToMove
**
** DESCRIPTION: convert two slots (from and to) to an encoded MOVE
**
** INPUT:       SLOT fromSlot   : The slot the piece moves from (0->BOARDSIZE)
**              SLOT toSlot     : The slot the piece moves to   (0->BOARDSIZE)
**
** OUTPUT:      MOVE            : The move corresponding to from->to
**
************************************************************************/

MOVE SlotsToMove (fromSlot, toSlot)
SLOT fromSlot, toSlot;
{
	return ((MOVE) toSlot* (BOARDSIZE+1) + fromSlot);
}

/************************************************************************
**
** NAME:        generic_hash_hash2
**
** DESCRIPTION: convert a BlankOX to that of an internal position.
**
** INPUTS:      BlankOX *theBlankOx : The converted BlankOX output array.
**
** OUTPUTS:     POSITION: The equivalent position given the BlankOX.
**
************************************************************************/

POSITION generic_hash_hash2(char* board, char whosTurn, int phase, int numSwans)
{
	POSITION temp;
	int whoseTurnTemp;

	if (whosTurn == 'o')
		whoseTurnTemp = 1;
	else
		whoseTurnTemp = 2;

	temp = generic_hash_hash(board, whoseTurnTemp);

	// encode phase
	temp = temp << 1;
	if(phase == 2)
		temp++;

	// encode numswans
	temp = temp << 4;
	temp += numSwans;

	return temp;
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

BOOLEAN AllFilledIn(theBoard)
char theBoard[16];
{
	BOOLEAN answer = TRUE;
	int i;

	for(i = 0; i < BOARDSIZE; i++)
		answer &= (theBoard[i] == 'o' || theBoard[i] == 'x');
	return(answer);
}


STRING kDBName = "swans";

int NumberOfOptions()
{
	return 4*(MAX_DRAGONS-MIN_DRAGONS+1);
}

int getOption()
{
	int option = 1;
	option += (gStandardGame ? 0 : 1);
	option += 2*(gToTrapIsToWin ? 0 : 1);
	option += 4*(gNumDragons-MIN_DRAGONS);
	return option;
}

void setOption(int option)
{
	option--;
	gStandardGame = (option%2==0);
	gToTrapIsToWin = (option/2%2==0);
	gNumDragons = (option/4)+MIN_DRAGONS;
}

POSITION StringToPosition(char* string) {
	char * board = (char *) SafeMalloc(sizeof(char) * (BOARDSIZE + 1));
	int i;
	for (i = 0; i < BOARDSIZE; i++) {
	  char toBeInserted = string[i];
	  if (toBeInserted == ' ') {
	    toBeInserted = 'b';
	  }
	  sprintf(board + i, "%c", toBeInserted);
	}
	board[i] = '\0';

    int phase, numSwans;
    char whosTurn;

    BOOLEAN success = GetValue(string, "phase", GetInt, &phase) &&
                        GetValue(string, "numSwans", GetInt, &numSwans) &&
                        GetValue(string, "whosTurn", GetChar, &whosTurn);
    if(success == FALSE){
        printf("ERROR: something went wrong in getting the values out of the string \n");
        return INVALID_POSITION;
    }

    POSITION p = generic_hash_hash2(board, whosTurn, phase, numSwans);
	
	if (board != NULL) {
	  SafeFree(board);
	}	

	return p;
}


char* PositionToString(POSITION pos) {
	int phase, numSwans;
	char whosTurn;
	char * board = (char *) SafeMalloc(sizeof(char) * (BOARDSIZE + 1)); //+1 for the null character
	generic_hash_unhash2(pos, board, &whosTurn, &phase, &numSwans);
	
    int i;
	for (i = 0; i < BOARDSIZE; i++) {
	  if (board[i] == 'b') {
        board[i] = ' ';
	  }
	}
    board[BOARDSIZE] = '\0';

    char* phaseValue = (char*) SafeMalloc(11);
    sprintf(phaseValue, "%d", phase);

    char* numSwansValue = (char*) SafeMalloc(11);
    sprintf(numSwansValue, "%d", numSwans);

    char* whosTurnValue = (char*) SafeMalloc(2);
    sprintf(whosTurnValue, "%c", whosTurn);

    char* retString = MakeBoardString(board, "phase", phaseValue, "numSwans", numSwansValue, 
        "whosTurn", whosTurnValue, "");

	if (board != NULL) {
	  SafeFree(board);
	}

	return retString;		
}

char * PositionToEndData(POSITION pos) {
	return NULL;
}
