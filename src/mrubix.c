/************************************************************************
**
** NAME:        mrubix.c
**
** DESCRIPTION: Rubik's Magic (NxM Version)
**
** AUTHOR:      Jeffrey Chiang & Bryon Ross
**
** DATE:        2003-10-11
**
** UPDATE HIST:
**
** 0.0.0 (02-09-06) Dan Garcia's Tic-Tac-Toe minus the game-dependent code.
** 0.0.1 (10-06) PrintPosition, PositionToBlankOX, BlankOXToPosition,
**	        and possibleBoardPieces updated
** 0.0.2 (10-14) GetAndPrintPlayersMoves, ValidTextInput,
**              ConvertTextInputToMove, and PrintMove updated
** 0.0.3 (10-15) Removed Symmetry Code
**              Updated PrintComputersMove
** 0.1.0 (10-26) First solvable version
** 0.1.1 (11-20) Updated GenerateMoves, Text UI & X moves first
** 0.1.2 (03-02-22) Updated ValidTextInput, GetAndPrintPlayersMove,
**              ConvertTextInputToMove, & PrintPosition to handle
**              large board sizes.
** 0.2.0 (04-19) Now meets the GAMESMAN3 standard.
** 0.2.1 (05-10) Dynamically resizable game board size
** 0.2.2 (08-26) Fixed bugs relating to setting board size
** 0.2.3 (09-09) Updated hash function to use combinatorics
** 0.2.4 (10-03) Fixed an initialization bug in CombinationInit
** 0.2.5 (10-11) Added sanity check in GetInitialPosition
**
**	(8-21)	Changed to GetMyInt()/GetMyChar()
**************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/

#include <stdio.h>
#include "gamesman.h"

POSITION gNumberOfPositions  = 0;
POSITION gMinimalPosition    = 0;

POSITION gInitialPosition    =  0;
POSITION kBadPosition        = -1; /* This can never be the rep. of a position */

STRING kAuthorName         = "Jeffrey Chiang and Bryon Ross";
STRING kGameName           = "Rubik's Magic";
BOOLEAN kPartizan           = TRUE;
BOOLEAN kSupportsHeuristic  = FALSE;
BOOLEAN kSupportsSymmetries = FALSE;
BOOLEAN kSupportsGraphics   = TRUE;
BOOLEAN kDebugMenu          = TRUE;
BOOLEAN kGameSpecificMenu   = TRUE;
BOOLEAN kTieIsPossible      = TRUE;
BOOLEAN kLoopy               = FALSE;
BOOLEAN kDebugDetermineValue = FALSE;

STRING kHelpGraphicInterface =
        "There is currently no graphic interface.";
/*STRING   kHelpGraphicInterface =
   "The LEFT button puts an X or O (depending on whether you went first\n\
   or second) on the spot the cursor was on when you clicked. The MIDDLE\n\
   button does nothing, and the RIGHT button is the same as UNDO, in that\n\
   it reverts back to your your most recent position.";*/

STRING kHelpTextInterface    =
        "Your turn consists of up to two parts:\n\n\
1) Using the LEGEND, determine the number corresponding to\n\
the opponent's piece to flip and the direction to flip it.\n\
Directions: (U)p (D)own (L)eft (R)ight\n\n\
2) Using the LEGEND, determine the number corresponding to\n\
the blank space in which to place your piece and the orientation\n\
of that piece.\n\
Orientations: x X (1st Player);  o O (2nd Player)\n\n\
The input should take the form:\n\
[initialLocation][direction] [newLocation][orientation]\n\n\
Example input: 4U 2X"                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        ;

STRING kHelpOnYourTurn =
        "Flip an opponent's piece into an adjacent, but not diagonal space.\n\
(This will change the size [example: X -> x] of your opponent's piece.)\n\
Then place one of your pieces on any empty space (including the one you\n\
flipped your opponent's piece from), either side facing up."                                                                                                                                                                                                                                     ;

STRING kHelpStandardObjective =
        "To complete either a vertical, horizontal, or diagonal line with three\n\
of your pieces, all of them of the same size. If your opponent cannot\n\
break this three piece line in the next move, you have won the game."                                                                                                                                                            ;

STRING kHelpReverseObjective =
        "To prevent a vertical, horizontal, or diagonal line with three\n\
of your pieces, each of them of the same size. If your opponent cannot\n\
break this three piece line in the next move, you have lost the game."                                                                                                                                                     ;

STRING kHelpTieOccursWhen =   /* Should follow 'A Tie occurs when... */
                            "the board becomes entirely filled without either player\n\
getting three-in-a-row."                                                                                        ;

STRING kHelpExample =
        "Ok, Dan Garcia and Computer, let us begin.\n\
\n\
Type '?' if you need assistance...\n\
\n\
\n\
         (  1  2  3 )           : - - -\n\
         (  4  5  6 )           : - - -\n\
         (  7  8  9 )           : - - -   (Dan Garcia should Tie in 9)\n\
Dan Garcia's move:\n\
        (u)ndo or {[1-9][x/X]}:  {5X}\n\
\n\
         (  1  2  3 )           : - - -\n\
         (  4  5  6 )           : - X -\n\
         (  7  8  9 )           : - - -   (Computer should Tie in 8)\n\
Computer's move: [5D 7o]\n\
\n\
         (  1  2  3 )           : - - -\n\
         (  4  5  6 )           : - - -\n\
         (  7  8  9 )           : o x -   (Dan Garcia should Tie in 7)\n\
Dan Garcia's move:\n\
        (u)ndo or {[1-9][(U)p/(D)own/(L)eft/(R)ight] [1-9][x/X]} :  {7U 2x}\n\
\n\
         (  1  2  3 )           : - x -\n\
         (  4  5  6 )           : O - -\n\
         (  7  8  9 )           : - x -   (Computer will Win in 6)\n\
Computer's move: [8U 1o]\n\
\n\
         (  1  2  3 )           : o x -\n\
         (  4  5  6 )           : O X -\n\
         (  7  8  9 )           : - - -   (Dan Garcia will Lose in 5)\n\
Dan Garcia's move:\n\
        (u)ndo or {[1-9][(U)p/(D)own/(L)eft/(R)ight] [1-9][x/X]} :  {4D 3x}\n\
\n\
         (  1  2  3 )           : o x x\n\
         (  4  5  6 )           : - X -\n\
         (  7  8  9 )           : o - -   (Computer will Win in 3)\n\
Computer's move: [5D 4O]\n\
\n\
         (  1  2  3 )           : o x x\n\
         (  4  5  6 )           : O - -\n\
         (  7  8  9 )           : o x -   (Dan Garcia will Lose in 2)\n\
Dan Garcia's move:\n\
        (u)ndo or {[1-9][(U)p/(D)own/(L)eft/(R)ight] [1-9][x/X]} :  {4R 6x}\n\
\n\
         (  1  2  3 )           : o x x\n\
         (  4  5  6 )           : - o x\n\
         (  7  8  9 )           : o x -   (Computer will Win in 1)\n\
Computer's move: [6D 4o]\n\
\n\
         (  1  2  3 )           : o x x\n\
         (  4  5  6 )           : o o -\n\
         (  7  8  9 )           : o x X   (Dan Garcia will Lose in 0)\n\
\n\
Computer wins. Nice try, Dan Garcia."                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          ;

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

int BOARDSIZE =    9;
int WIDTH =        3;
#define MAX_BOARDSIZE 12
#define MIN_LENGTH 3
#define MIN_WIDTH MIN_LENGTH
#define MAX_WIDTH (MAX_BOARDSIZE/MIN_LENGTH)
#define DIRECTIONS    4 /* Up Down Left Right */
#define ORIENTATIONS  5
BOOLEAN twoInARow = FALSE;

struct _PrimitiveSequence {
	int numSpaces;
	int spaces[3];
	struct _PrimitiveSequence* next;
};

typedef struct _PrimitiveSequence PrimitiveSequence;

PrimitiveSequence* primitiveSequence = NULL;

typedef enum possibleBoardPieces {
	Blank, o, O, x, X
} BlankOX;

enum directions { U, D, L, R };

char *gBlankOXString[] = { "-", "o", "O", "x", "X"};

/* Powers of 5 - this is the way I encode the position, as an integer */
/* 9 spots on the board */
int g5Array[] = { 1, 5, 25, 125, 625, 3125, 15625, 78125, 390625};

STRING MoveToString( MOVE );

/***************************************************************************/


#ifndef NO_GRAPHICS

/*#pragma weak Tcl_CreateCommand*/

static int HashCmd(ClientData dummy, Tcl_Interp *interp,
                   int argc, char **argv);
static int UnhashCmd(ClientData dummy, Tcl_Interp *interp,
                     int argc, char **argv);

int RubixTclInit(Tcl_Interp* interp,Tk_Window mainWindow) {
	Tcl_CreateCommand(interp, "C_Hash", (Tcl_CmdProc*) HashCmd, (ClientData) mainWindow, (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "C_Unhash", (Tcl_CmdProc*) UnhashCmd, (ClientData) mainWindow, (Tcl_CmdDeleteProc*) NULL);
	return TCL_OK;
}

void* gGameSpecificTclInit = (void*) RubixTclInit;

static int HashCmd(dummy, interp, argc, argv)
ClientData dummy;                       /* Not used. */
Tcl_Interp *interp;                     /* Current interpreter. */
int argc;                               /* Number of arguments. */
char **argv;                            /* Argument strings. */
{
	BlankOX board[BOARDSIZE];
	POSITION BlankOXToPosition();
	int i, j;
	POSITION position;

	if (argc != 2) {
		Tcl_SetResult(interp, "wrong # args: Hash (STRING)board", TCL_STATIC);
		return TCL_ERROR;
	}
	else {
		for (i=0; i<BOARDSIZE; i++) {
			for (j=0; j<ORIENTATIONS; j++) {
				if (argv[1][i] == gBlankOXString[j][0]) {
					board[i] = j;
					break;
				}
			}
		}
		position = BlankOXToPosition(board);
		Tcl_SetResult(interp, StrFromI(position), SafeFreeString);
		return TCL_OK;
	}
}

static int UnhashCmd(dummy, interp, argc, argv)
ClientData dummy;                       /* Not used. */
Tcl_Interp *interp;                     /* Current interpreter. */
int argc;                               /* Number of arguments. */
char **argv;                            /* Argument strings. */
{
	POSITION position;
	void PositionToBlankOX();
	BlankOX board[BOARDSIZE];
	char boardString[BOARDSIZE+1];
	int i;

	if (argc != 2) {
		Tcl_SetResult(interp, "wrong # args: Unhash (POSITION)position", TCL_STATIC);
		return TCL_ERROR;
	}
	else {
		if (sscanf(argv[1], POSITION_FORMAT, &position) == EOF)
			return TCL_ERROR;
		PositionToBlankOX(position, board);
		for (i=0; i<BOARDSIZE; i++) {
			boardString[i] = gBlankOXString[board[i]][0];
		}
		boardString[BOARDSIZE] = '\0';

		Tcl_SetResult(interp, StringDup(boardString), SafeFreeString);
		return TCL_OK;
	}
}
#else
void* gGameSpecificTclInit = NULL;
#endif


static POSITION **CArray;
static POSITION *hashSizes;
static POSITION *hashBasePosition;

void CombinationInit(int boardsize)
{
  #ifndef MEMWATCH
	GENERIC_PTR SafeMalloc();
  #endif
	int i, j;
	if (CArray != NULL) {
		SafeFree((GENERIC_PTR) CArray);
	}
	CArray = (POSITION**) SafeMalloc(sizeof(POSITION*) * (boardsize + 1));
	for(i=0; i<=boardsize; i++) {
		CArray[i] = (POSITION*) SafeMalloc(sizeof(POSITION) * (boardsize + 1));
		for (j=0; j<=boardsize; j++) {
			if (j>i) CArray[i][j] = 0;
			else if (j==0 || j==i) CArray[i][j] = 1;
			else CArray[i][j] = CArray[i-1][j-1] + CArray[i-1][j];
		}
	}
}

int C(int n, int r) {
	if (n<0 || r<0) return 0;
	return CArray[n][r];
}

void InitializeGame()
{
	PrimitiveSequence* temp;
	int i, j;
  #ifndef MEMWATCH
	GENERIC_PTR SafeMalloc();
  #endif
	CombinationInit(BOARDSIZE);

	if (hashSizes != NULL) {
		SafeFree(hashSizes);
	}
	hashSizes = (POSITION*) SafeMalloc((BOARDSIZE+1) * sizeof(POSITION));

	if (hashBasePosition != NULL) {
		SafeFree(hashBasePosition);
	}
	hashBasePosition = (POSITION*) SafeMalloc((BOARDSIZE+2) * sizeof(POSITION));

	for (i=0; i<BOARDSIZE+1; i++) {
		hashSizes[i] = ((C(BOARDSIZE, i) * C(i, i/2)) << i);
	}

	hashBasePosition[0] = 0;
	for (i=1; i<=BOARDSIZE+1; i++) {
		hashBasePosition[i] = hashBasePosition[i-1] + hashSizes[i-1];
		/* printf("Hash Base position %d: %d\n", i, hashBasePosition[i]); */
	}

	gNumberOfPositions = hashBasePosition[BOARDSIZE+1];

	/* clear out primitive sequences */
	while (primitiveSequence != NULL) {
		temp = primitiveSequence;
		primitiveSequence = primitiveSequence->next;
		free(temp);
	}

	/* compute new primitive sequence */
	for (i=0; i<BOARDSIZE; i++) {
		/* 3 in a row horizontally */
		if ((i+2)%WIDTH > i%WIDTH) {
			temp = (PrimitiveSequence*) malloc(sizeof(PrimitiveSequence));
			temp->numSpaces = 3;
			temp->spaces[0] = i;
			temp->spaces[1] = i+1;
			temp->spaces[2] = i+2;

			temp->next = primitiveSequence;
			primitiveSequence = temp;
		}

		/* 3 in a row diagonally & vertically */
		for (j=-1; j<=1; j++) {
			if (i+2*(WIDTH+j) < BOARDSIZE && i/WIDTH +1 == (i+WIDTH+j)/WIDTH && i/WIDTH +2 == (i+2*(WIDTH+j))/WIDTH) {
				temp = (PrimitiveSequence*) malloc(sizeof(PrimitiveSequence));
				temp->numSpaces = 3;
				temp->spaces[0] = i;
				temp->spaces[1] = i+WIDTH+j;
				temp->spaces[2] = i+2*(WIDTH+j);

				temp->next = primitiveSequence;
				primitiveSequence = temp;
			}
		}

		gMoveToStringFunPtr = &MoveToString;
	}


	if (twoInARow) {
		for (i=0; i<BOARDSIZE; i++) {
			/* 2 in a row horizontally */
			if ((i+1)%WIDTH > i%WIDTH) {
				temp = (PrimitiveSequence*) malloc(sizeof(PrimitiveSequence));
				temp->numSpaces = 2;
				temp->spaces[0] = i;
				temp->spaces[1] = i+1;

				temp->next = primitiveSequence;
				primitiveSequence = temp;
			}

			/* 2 in a row diagonally & vertically */
			for (j=-1; j<=1; j++) {
				if (i+1*(WIDTH+j) < BOARDSIZE && i/WIDTH + 1 == (i+WIDTH+j)/WIDTH) {
					temp = (PrimitiveSequence*) malloc(sizeof(PrimitiveSequence));
					temp->numSpaces = 2;
					temp->spaces[0] = i;
					temp->spaces[1] = i+WIDTH+j;

					temp->next = primitiveSequence;
					primitiveSequence = temp;
				}
			}
		}
	}
	/*
	   temp = primitiveSequence;
	   while(temp!=NULL) {
	   printf("Sequence:");
	   for (i=0; i<temp->numSpaces; i++) {
	    printf(" %d",temp->spaces[i]);
	   }
	   printf("\n");
	   temp= temp->next;
	   }
	 */
}

void FreeGame()
{
	PrimitiveSequence* temp;

	if (CArray != NULL) {
		SafeFree((GENERIC_PTR) CArray);
	}
	if (hashSizes != NULL) {
		SafeFree(hashSizes);
	}
	if (hashBasePosition != NULL) {
		SafeFree(hashBasePosition);
	}
	while (primitiveSequence != NULL) {
		temp = primitiveSequence;
		primitiveSequence = primitiveSequence->next;
		free(temp);
	}
}


/************************************************************************
**
** NAME:        DebugMenu
**
** DESCRIPTION: Menu used to debub internal problems. Does nothing if
**              kDebugMenu == FALSE
**
************************************************************************/

void DebugMenu() {
}

/************************************************************************
**
** NAME:        GetIntegerValue
**
** DESCRIPTION: Asks the user for an integer value
**
** INPUTS:      inputMsg: The string with which to query the user
**              errorMsg: The string with which to notify the user of an
**                        invalid entry
**              lowerBound: the lower bound of the returned input
**              upperBound: the upper bound of the returned input
**
** OUTPUTS:     The integer entered by the user
**
************************************************************************/
int GetIntegerValue(char* valueName, int lowerBound,
                    int upperBound) {
	int value = 0;
	BOOLEAN done = FALSE;
	int result;
	/*char dummy;*/

	while (!done) {
		printf("Enter a %s (%d-%d): ", valueName, lowerBound, upperBound);
		result = GetMyInt(); /*scanf("%d", &value);*/
		if (result==1) {
			if (value < lowerBound || value > upperBound) {
				printf("The %s must be between %d and %d.\n", valueName, lowerBound, upperBound);
			}
			else {
				done = TRUE;
			}
		}
		else {
			printf("The %s must be between %d and %d.\n", valueName, lowerBound, upperBound);
		}

		/*do {
		   result = scanf("%c",&dummy);
		   } while (result!=EOF && dummy!='\n');*/
	}

	return value;
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
	int length;

	POSITION GetInitialPosition();
	void ExitStageRight();
	void HitAnyKeyToContinue();

	do {
		printf("\n\t----- Game-specific options for %s -----\n\n", kGameName);

		printf("\tCurrent Initial Position:\n");
		PrintPosition(gInitialPosition, gPlayerName[kPlayerOneTurn], kHumansTurn);

		printf("\n");
		printf("\tW)\tChoose the board (W)idth:  Currently %d\n", WIDTH);
		printf("\tL)\tChoose the board (L)ength: Currently %d\n", BOARDSIZE/WIDTH);
		printf("\tI)\tChoose the (I)nitial position\n");
		printf("\tT)\tToggle from Two in a Row %s to Three in a Row to\n\t\tTwo in a Row %s to Three in a Row\n",twoInARow ? "EQUIVALENT" : "NOT EQUIVALENT", twoInARow ? "NOT EQUIVALENT" : "EQUIVALENT");
		printf("\n\tB)\t(B)ack = Return to previous activity.\n");
		printf("\n\nSelect an option: ");

		switch(GetMyChar()) {
		case 'B': case 'b':
			return;
		case 'Q': case 'q':
			ExitStageRight();
		case 'I': case 'i':
			gInitialPosition = GetInitialPosition();
			break;
		case 'W': case 'w':
			BOARDSIZE /= WIDTH;
			WIDTH = GetIntegerValue("board width", MIN_WIDTH, MAX_WIDTH);
			if (BOARDSIZE > WIDTH)
				BOARDSIZE = WIDTH;
			BOARDSIZE *= WIDTH;
			gInitialPosition = 0;
			InitializeGame();
			break;
		case 'L': case 'l':
			length = GetIntegerValue("board length", MIN_WIDTH, WIDTH<(MAX_BOARDSIZE/WIDTH) ? WIDTH : (MAX_BOARDSIZE/WIDTH));
			BOARDSIZE = length * WIDTH;
			gInitialPosition = 0;
			InitializeGame();
			break;
		case 'T': case 't':
			twoInARow = !twoInARow;
			InitializeGame();
			break;
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

void SetTclCGameSpecificOptions(int theOptions[])
{
	//defaults
	WIDTH = 3;
	BOARDSIZE = 9;

	twoInARow = !theOptions[0];

	if (theOptions[1]==0) {
		WIDTH=3;
		BOARDSIZE=9;
	}
	else if (theOptions[1]==1) {
		WIDTH=4;
		BOARDSIZE=12;
	}
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
	int initloc, newloc;
	BlankOX newpiece, pieceafterflip;
	BlankOX theBlankOX[BOARDSIZE];
	POSITION BlankOXToPosition(), newPosition;
	void PositionToBlankOX();

	PositionToBlankOX(thePosition,theBlankOX);

	/* MOVE REPRESENTATION:
	   For all moves:
	   WhereToPlace (0-BOARDSIZE-1)      *BOARDSIZE*DIRECTIONS
	   Orientation (x, X, o, O)          *BOARDSIZE*DIRECTIONS*BOARDSIZE

	   For moves where the player CAN'T flip a piece:
	   CantFlip Flag = 1            *BOARDSIZE*BOARDSIZE*DIRECTIONS*ORIENTATIONS

	   For moves where the player CAN flip a piece:
	   PieceToMove (0-BOARDSIZE-1)       *1
	   DirectionToMove (U, D, L, R)  *BOARDSIZE
	 */

	//can flip
	if (theMove/(BOARDSIZE*DIRECTIONS*BOARDSIZE*ORIENTATIONS) == 0) {

		initloc = (theMove%BOARDSIZE);

		switch (theBlankOX[initloc]) {
		case x: pieceafterflip = X; break;
		case X: pieceafterflip = x; break;
		case o: pieceafterflip = O; break;
		case O: pieceafterflip = o; break;
		default: pieceafterflip = Blank; //should never happen
		}

		theBlankOX[initloc] = Blank; //flip from old spot

		//to new spot
		switch ((theMove/BOARDSIZE)%DIRECTIONS) {
		case 0: theBlankOX[initloc-WIDTH] = pieceafterflip; break; //up
		case 1: theBlankOX[initloc+WIDTH] = pieceafterflip; break; //down
		case 2: theBlankOX[initloc-1] = pieceafterflip; break; //left
		case 3: theBlankOX[initloc+1] = pieceafterflip; break; //right
		}
	}

	newloc = ((theMove/(BOARDSIZE*DIRECTIONS))%BOARDSIZE);
	newpiece = theMove/(BOARDSIZE*DIRECTIONS*BOARDSIZE)%ORIENTATIONS;
	theBlankOX[newloc] = newpiece;
	newPosition = BlankOXToPosition(theBlankOX);

	/*
	   printf("newloc is %d\n", newloc);
	   printf("newpiece is %d\n", newpiece);
	   PrintPosition(thePosition, "bob", TRUE);
	   PrintPosition(newPosition, "bob", TRUE);
	 */

	return newPosition;
}

typedef struct {
	int initloc;
	int newloc;
	BlankOX orientation;
	int direction;
	BOOLEAN canflip;
} MOVEREP;

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
	BlankOX theBlankOX[BOARDSIZE];
	signed char c;
	int i;
	BOOLEAN done = FALSE;
	int xCount, oCount;

	while(!done) {
		printf("\n\n\t----- Get Initial Position -----\n");
		printf("\n\tPlease input the position to begin with.\n");
		printf("\tNote that it should be in the following format:\n\n");
		printf("O - -\nO x -            <----- EXAMPLE \no X X\n\n");

		xCount = oCount = 0;
		i = 0;
		getchar();
		while(i < BOARDSIZE && (c = getchar()) != EOF) {
			if(c == 'x') {
				theBlankOX[i++] = x;
				xCount++;
			}
			else if(c == 'X') {
				theBlankOX[i++] = X;
				xCount++;
			}
			else if(c == 'o') {
				theBlankOX[i++] = o;
				oCount++;
			}
			else if(c == 'O') {
				theBlankOX[i++] = O;
				oCount++;
			}
			else if(c == '-')
				theBlankOX[i++] = Blank;
		}

		if (xCount==oCount || xCount==oCount+1) {
			done = TRUE;
		}
		else {
			printf("\nIllegal position. Please try again.");
		}

	}
	return(BlankOXToPosition(theBlankOX));
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

void PrintComputersMove(MOVE computersMove, STRING computersName)
{
	void PrintMove();

	printf("%8s's move: ",computersName);
	PrintMove(computersMove);
	printf("\n");
}

/************************************************************************
**
** NAME:        Primitive
**
** DESCRIPTION: Return the value of a position if it fulfills certain
**              'primitive' constraints. Some examples of this is having
**              three-in-a-row with TicTacToe. TicTacToe has two
**              primitives it can immediately check for, when the board
**              is filled but nobody has won = primitive tie. Three in
**              a row is a primitive lose, because the player who faces
**              this board has just lost. I.e. the player before him
**              created the board and won. Otherwise undecided.
**
** INPUTS:      POSITION position : The position to inspect.
**
** OUTPUTS:     (VALUE) an enum which is oneof: (win,lose,tie,undecided)
**
** CALLS:       BOOLEAN ThreeInARow()
**              BOOLEAN AllFilledIn()
**              PositionToBlankOX()
**
************************************************************************/

VALUE Primitive(position)
POSITION position;
{
	BOOLEAN ThreeInARow(), AllFilledIn(), CannotShiftPieces();
	void PositionToBlankOX();
	BlankOX theBlankOX[BOARDSIZE], WhoseTurn();
	BlankOX player, playerBig, otherPlayer, otherPlayerBig;
	BlankOX firstPiece;

	PrimitiveSequence* tempSequence;
	BOOLEAN inARow = FALSE;
	BOOLEAN playerThreeInARow = FALSE;
	BOOLEAN unbreakableOpponentThreeInARow = FALSE;

	/*printf("Called primitive\n");*/

	PositionToBlankOX(position,theBlankOX);
	player = WhoseTurn(theBlankOX);
	playerBig = ((player==x) ? X : O);
	otherPlayer = ((player==x) ? o : x);
	otherPlayerBig = ((otherPlayer==x) ? X : O);

	tempSequence = primitiveSequence;

	while(tempSequence!=NULL) {
		/*printf("Started while loop\n");*/
		firstPiece = theBlankOX[tempSequence->spaces[0]];

		if (tempSequence->numSpaces == 3) {
			inARow = firstPiece==theBlankOX[tempSequence->spaces[1]] && firstPiece==theBlankOX[tempSequence->spaces[2]];
		}
		else if (tempSequence->numSpaces == 2) {
			inARow = firstPiece==theBlankOX[tempSequence->spaces[1]];
		}

		/*printf("Got to inARow\n");*/
		if (inARow) {
			playerThreeInARow |= (firstPiece == player || firstPiece == playerBig);
			if (firstPiece == otherPlayer || firstPiece == otherPlayerBig) {
				/*printf("Calling  CannotShiftPieces\n");*/
				unbreakableOpponentThreeInARow |= CannotShiftPieces(theBlankOX, tempSequence->numSpaces, tempSequence->spaces[0], tempSequence->spaces[1], tempSequence->spaces[2]);
				/*printf("Returned  CannotShiftPieces\n");*/
			}
		}
		/*printf("Got to incrementer");*/
		tempSequence = tempSequence->next;
		/*printf("Finished while loop\n");*/
	}
	/*printf("Exited primitive\n");*/

	if (playerThreeInARow)
		return (gStandardGame ? win : lose);
	else if (unbreakableOpponentThreeInARow)
		return (gStandardGame ? lose : win);
	else if(AllFilledIn(theBlankOX))
		return(tie);
	else
		return(undecided);
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

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn)
{
	int i, j;
	//  VALUE GetValueOfPosition();
	void PositionToBlankOX();
	BlankOX theBlankOX[BOARDSIZE], WhoseTurn();
	STRING playerString;

	PositionToBlankOX(position,theBlankOX);
	playerString = gBlankOXString[(int)WhoseTurn(theBlankOX)];

	printf("\n");
	for (i=0; i<BOARDSIZE/WIDTH; i++) {
		if (BOARDSIZE/WIDTH==2*i && BOARDSIZE%i==0) {
			printf("LEGEND:  (");
		}
		else {
			printf("         (");
		}

		for (j=0; j<WIDTH; j++) {
			printf(" %2d",i*WIDTH+j+1);
		}

		if (BOARDSIZE/WIDTH==2*i && BOARDSIZE%i==0) {
			printf(" )  TOTAL:   :");
		}
		else {
			printf(" )           :");
		}

		for (j=0; j<WIDTH; j++) {
			printf(" %s",gBlankOXString[(int)theBlankOX[i*WIDTH+j]]);
		}

		if (BOARDSIZE/WIDTH==2*i && BOARDSIZE%i==0) {
			printf("   %s's Turn", playerString);
		}
		else if (i==BOARDSIZE/WIDTH-1) {
			printf("   %s", GetPrediction(position,playerName,usersTurn));
		}

		printf("\n");
	}
	/*
	   printf("\n         ( 1 2 3 )           : %s %s %s\n",
	     gBlankOXString[(int)theBlankOX[0]],
	     gBlankOXString[(int)theBlankOX[1]],
	     gBlankOXString[(int)theBlankOX[2]] );
	   printf("LEGEND:  ( 4 5 6 )  TOTAL:   : %s %s %s   %s's Turn\n",
	     gBlankOXString[(int)theBlankOX[3]],
	     gBlankOXString[(int)theBlankOX[4]],
	     gBlankOXString[(int)theBlankOX[5]],
	     playerString);
	   printf("         ( 7 8 9 )           : %s %s %s   %s\n",
	     gBlankOXString[(int)theBlankOX[6]],
	     gBlankOXString[(int)theBlankOX[7]],
	     gBlankOXString[(int)theBlankOX[8]],
	     GetPrediction(position,playerName,usersTurn));
	 */
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
	BlankOX theBlankOX[BOARDSIZE];
	BlankOX player, playerBig, opposingPlayer, opposingPlayerBig;
	BlankOX WhoseTurn();
	void PositionToBlankOX();
	int baseMove;
	MOVE aMove;
	int i, j;
	BOOLEAN canFlip;

	int blanks[BOARDSIZE];
	int blankCounter;

	int orientationSmall, orientationBig;

	/*printf("Called GenerateMoves\n");*/

	/* MOVE REPRESENTATION:
	   For all moves:
	   WhereToPlace (0-8)          *BOARDSIZE*DIRECTIONS
	   Orientation (x, X, o, O)          *BOARDSIZE*DIRECTIONS*BOARDSIZE

	   For moves where the player CAN'T flip a piece:
	   CantFlip Flag = 1            *BOARDSIZE*DIRECTIONS*BOARDSIZE*ORIENTATIONS

	   For moves where the player CAN flip a piece:
	   PieceToMove (0-8)                     *1
	   DirectionToMove (U, D, L, R)  *BOARDSIZE
	 */

	canFlip = FALSE;

	PositionToBlankOX(position, theBlankOX);
	player = WhoseTurn(theBlankOX);
	playerBig = player==x ? X : O;
	opposingPlayer = player==x ? o : x;
	opposingPlayerBig = opposingPlayer==x ? X : O;

	orientationSmall = player*BOARDSIZE*DIRECTIONS*BOARDSIZE;
	orientationBig = playerBig*BOARDSIZE*DIRECTIONS*BOARDSIZE;

	blankCounter = 0;
	for (i=0; i<BOARDSIZE; i++) {
		if (theBlankOX[i]==Blank) {
			blanks[blankCounter++] = i;
		}
	}

	for (i=0; i<BOARDSIZE; i++) {
		if (theBlankOX[i]==opposingPlayer ||
		    theBlankOX[i]==opposingPlayerBig) {

			//flip left
			if ((i%WIDTH)-1>=0 && theBlankOX[i-1]==Blank) {
				canFlip = TRUE;
				baseMove = i + L*BOARDSIZE;
				for (j=0; j<blankCounter; j++) {
					if (blanks[j] != i-1) {
						aMove = baseMove+blanks[j]*BOARDSIZE*DIRECTIONS;
						head = CreateMovelistNode(aMove+orientationSmall, head);
						head = CreateMovelistNode(aMove+orientationBig, head);
					}
				}
				aMove = baseMove+i*BOARDSIZE*DIRECTIONS;
				head = CreateMovelistNode(aMove+orientationSmall, head);
				head = CreateMovelistNode(aMove+orientationBig, head);
			}

			//flip right
			if ((i%WIDTH)+1<=WIDTH-1 && theBlankOX[i+1]==Blank) {
				canFlip = TRUE;
				baseMove = i + R*BOARDSIZE;
				for (j=0; j<blankCounter; j++) {
					if (blanks[j]!=i+1) {
						aMove = baseMove+blanks[j]*BOARDSIZE*DIRECTIONS;
						head = CreateMovelistNode(aMove+orientationSmall, head);
						head = CreateMovelistNode(aMove+orientationBig, head);
					}
				}
				aMove = baseMove+i*BOARDSIZE*DIRECTIONS;
				head = CreateMovelistNode(aMove+orientationSmall, head);
				head = CreateMovelistNode(aMove+orientationBig, head);
			}

			//flip up
			if (i-WIDTH>=0 && theBlankOX[i-WIDTH]==Blank) {
				canFlip = TRUE;
				baseMove = i + U*BOARDSIZE;
				for (j=0; j<blankCounter; j++) {
					if (blanks[j]!=i-WIDTH) {
						aMove = baseMove+blanks[j]*BOARDSIZE*DIRECTIONS;
						head = CreateMovelistNode(aMove+orientationSmall, head);
						head = CreateMovelistNode(aMove+orientationBig, head);
					}
				}
				aMove = baseMove+i*BOARDSIZE*DIRECTIONS;
				head = CreateMovelistNode(aMove+orientationSmall, head);
				head = CreateMovelistNode(aMove+orientationBig, head);
			}

			//flip down
			if (i+WIDTH<=BOARDSIZE-1 && theBlankOX[i+WIDTH]==Blank) {
				canFlip = TRUE;
				baseMove = i + D*BOARDSIZE;
				for (j=0; j<blankCounter; j++) {
					if (blanks[j] != i+WIDTH) {
						aMove = baseMove+blanks[j]*BOARDSIZE*DIRECTIONS;
						head = CreateMovelistNode(aMove+orientationSmall, head);
						head = CreateMovelistNode(aMove+orientationBig, head);
					}
				}
				aMove = baseMove+i*BOARDSIZE*DIRECTIONS;
				head = CreateMovelistNode(aMove+orientationSmall, head);
				head = CreateMovelistNode(aMove+orientationBig, head);
			}
		}
	}

	if (canFlip == FALSE) {
		for (i=0; i<blankCounter; i++) {
			aMove = blanks[i]*BOARDSIZE*DIRECTIONS+
			        1*BOARDSIZE*DIRECTIONS*BOARDSIZE*ORIENTATIONS;
			head = CreateMovelistNode(aMove+orientationSmall, head);
			head = CreateMovelistNode(aMove+orientationBig, head);
		}
	}

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

USERINPUT GetAndPrintPlayersMove(POSITION thePosition, MOVE* theMove,
                                 STRING playerName)
{
	BOOLEAN ValidMove(), CantFlipAnyPiece();
	USERINPUT ret, HandleDefaultTextInput();
	void PositionToBlankOX();
	BlankOX theBlankOX[BOARDSIZE];
	BlankOX WhoseTurn(), player;
	STRING playerString, playerBigString;

	PositionToBlankOX(thePosition, theBlankOX);
	player = WhoseTurn(theBlankOX);
	playerString = gBlankOXString[player];
	playerBigString = gBlankOXString[player==x ? X : O];

	do {
		//can't flip
		if (CantFlipAnyPiece(theBlankOX))
			printf("%8s's move:\n\t(u)ndo or {[1-%d][%s/%s]}:  ", playerName,BOARDSIZE,playerString, playerBigString );

		//can flip
		else
			printf("%8s's move:\n\t(u)ndo or {[1-%d][(U)p/(D)own/(L)eft/(R)ight] [1-9][%s/%s]} :  ", playerName,BOARDSIZE,playerString,playerBigString);

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

BOOLEAN ValidTextInput(STRING input)
{
	int initloc, newloc;
	char direction, orientation;

	sscanf(input, "%d%c %d%c",&initloc, &direction, &newloc, &orientation);
	if (initloc<1 || initloc>BOARDSIZE) {
		return FALSE;
	}

	if (direction=='o' || direction=='O' || direction=='x' || direction=='X')
		return TRUE;

	if ((direction=='U' || direction=='u' || direction=='D' || direction=='d' ||
	     direction=='L' || direction=='l' || direction=='R' || direction=='r') &&
	    (newloc>=1 && newloc<=BOARDSIZE) &&
	    (orientation=='x' || orientation=='X' ||
	     orientation=='o' || orientation=='O'))
		return TRUE;

	else
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

MOVE ConvertTextInputToMove(STRING input)
{
	char directionChar, orientationChar;
	int initloc, direction=-1, newloc, orientation=-1;
	BOOLEAN cantflip=FALSE;

	sscanf(input, "%d%c %d%c",&initloc,&directionChar,&newloc,&orientationChar);

	switch (directionChar) {
	case 'U': case 'u': direction = U; cantflip=FALSE; break;
	case 'D': case 'd': direction = D; cantflip=FALSE; break;
	case 'L': case 'l': direction = L; cantflip=FALSE; break;
	case 'R': case 'r': direction = R; cantflip=FALSE; break;
	case 'O': orientation = O; cantflip=TRUE; break;
	case 'X': orientation = X; cantflip=TRUE; break;
	case 'o': orientation = o; cantflip=TRUE; break;
	case 'x': orientation = x; cantflip=TRUE; break;
	}

	if (cantflip) {
		newloc = initloc-1;
		initloc = 0;
		direction = 0;
	}

	else {
		initloc--; // input is 1+internalRepresentation
		newloc--;
		switch (orientationChar) {
		case 'o': orientation = o; break;
		case 'x': orientation = x; break;
		case 'O': orientation = O; break;
		case 'X': orientation = X; break;
		}
	}
	return (initloc + BOARDSIZE*direction + BOARDSIZE*DIRECTIONS*newloc
	        + BOARDSIZE*DIRECTIONS*BOARDSIZE*orientation +
	        BOARDSIZE*DIRECTIONS*BOARDSIZE*ORIENTATIONS*cantflip);

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
	int initloc, newloc;
	int direction, orientation;

	newloc = ((theMove/(BOARDSIZE*DIRECTIONS))%BOARDSIZE)+1;

	orientation = *gBlankOXString[(theMove/(BOARDSIZE*DIRECTIONS*BOARDSIZE))%ORIENTATIONS];

	if (theMove/(BOARDSIZE*DIRECTIONS*BOARDSIZE*ORIENTATIONS)) { //can't flip
		sprintf(m, "[%d%c]",newloc,orientation);
	} else { //can flip

		initloc = (theMove%BOARDSIZE)+1;

		switch ((theMove/BOARDSIZE)%DIRECTIONS) {
		case U: direction = 'U'; break;
		case D: direction = 'D'; break;
		case L: direction = 'L'; break;
		case R: direction = 'R'; break;
		default: direction = 5;
		}

		sprintf(m, "[%d%c %d%c]",initloc,direction,newloc,orientation);
	}

	return m;
}

/************************************************************************
** FOR USE IN DATABASE *************************************************
***********************************************************************/

STRING kDBName = "rubix";

int NumberOfOptions()
{
	int standardGameOptions = 2;
	int twoInARowOptions = 2;
	int boardsizeOptions = 0;
	int width, length;

	for (width=MIN_WIDTH; width<=MAX_WIDTH; width++) {
		for (length=MIN_LENGTH; length<=width && length*width<=MAX_BOARDSIZE; length++) {
			boardsizeOptions++;
		}
	}

	return standardGameOptions * twoInARowOptions * boardsizeOptions;
}

int getOption()
{
	int option = 1;
	int boardsizeOption = 0;
	int i, j;
	BOOLEAN done = FALSE;

	if (gStandardGame) option += 1;
	if (twoInARow) option += 1<<1;

	/* BOARDSIZE=WIDTH*LENGTH where WIDTH>=LENGTH and min LENGTH=3*/
	for (i=MIN_WIDTH; i<WIDTH && !done; i++) {
		for (j=MIN_LENGTH; j<=i; j++) {
			boardsizeOption++;
		}
	}
	for (i=MIN_LENGTH; i<(BOARDSIZE/WIDTH); i++) {
		boardsizeOption++;
	}

	option += boardsizeOption * 2*2;

	return option;
}

void setOption(int option)
{
	int width=MIN_WIDTH, length=MIN_LENGTH;
	int boardsizeOption;
	option -= 1;

	gStandardGame = option & 1;
	twoInARow = (option >> 1) & 1;

	for(boardsizeOption=option>>2; boardsizeOption>0; boardsizeOption--) {
		length++;
		if(length>width) {
			width++;
			length = MIN_LENGTH;
		}
	}
	WIDTH = width;
	BOARDSIZE = width*length;
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
	int i, numPieces, numO, numX;
	POSITION parityMask, parityPos, xPos, oPos;
	BlankOX piece;
	BOOLEAN foundNumPieces = FALSE;

	for (i=1; i<=BOARDSIZE+1 && !foundNumPieces; i++) {
		if (thePos < hashBasePosition[i]) {
			numPieces = i-1;
			foundNumPieces = TRUE;
		}
	}
	numO = numPieces/2;
	numX = numPieces - numO;

	thePos -= hashBasePosition[numPieces];
	parityMask = (1<<numPieces)-1;
	parityPos = thePos & parityMask;
	thePos >>= numPieces;
	xPos = thePos % C(BOARDSIZE,numX);
	oPos = thePos / C(BOARDSIZE,numX);

	for (i=BOARDSIZE-1; i>=0; i--) {
		if (xPos >= C(i,numX)) {
			theBlankOX[i] = x;
			xPos -= C(i,numX);
			numX--;
		}
		else if (oPos >= C(i-numX, numO)) {
			theBlankOX[i] = o;
			oPos -= C(i-numX, numO);
			numO--;
		}
		else
			theBlankOX[i] = Blank;
	}

	for (i=0; i<BOARDSIZE; i++) {
		piece = theBlankOX[i];
		if (piece==x) {
			if ((parityPos&1) == 1) {
				theBlankOX[i] = X;
			}
			parityPos >>= 1;
		}
		else if (piece==o) {
			if ((parityPos&1) == 1) {
				theBlankOX[i] = O;
			}
			parityPos >>= 1;
		}
	}
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
	int i;
	POSITION position = 0;
	POSITION xPos=0, oPos=0, parityPos=0;
	BlankOX piece;
	int numX = 0, numO = 0, numPieces;

	for(i = 0; i < BOARDSIZE; i++) {
		piece = theBlankOX[i];
		if (piece==x || piece==X) {
			numX++;
		}
		else if (piece==o || piece==O) {
			numO++;
		}
	}
	numPieces = numX + numO;

	position = hashBasePosition[numPieces];

	for (i=BOARDSIZE-1; i>=0; i--) {
		piece = theBlankOX[i];
		if (piece==x || piece==X) {
			xPos += C(i,numX);
			parityPos <<= 1;
			if (piece==X) {
				parityPos |= 1;
			}
			numX--;
		}
		else if (piece==o || piece==O) {
			oPos += C(i-numX, numO);
			parityPos <<= 1;
			if (piece==O) {
				parityPos |= 1;
			}
			numO--;
		}
	}
	position += ((xPos + oPos * C(BOARDSIZE, numPieces-numPieces/2)) << numPieces) | parityPos;

	return(position);
}

/************************************************************************
**
** NAME:        CannotShiftPieces
**
** DESCRIPTION: Return TRUE iff none of the given pieces can be flipped
**
** INPUTS:      BlankOX theBlankOX[BOARDSIZE] : The BlankOX array.
**              int a,b,c                     : The 3 positions to check.
**
************************************************************************/

BOOLEAN CannotShiftPieces(theBlankOX, num, a, b, c)
BlankOX theBlankOX[];
int a,b,c,num;
{
	int count, spot;

	/*printf("Starting CannotShiftPieces\n");*/
	for (count=0; count<num; count++) {
		if (count==0) spot=a;
		else if (count==1) spot=b;
		else spot=c;

		/*printf("spot: %d\n", spot);*/

		//if a piece isn't surrounded
		if (((spot%WIDTH)-1>=0 && theBlankOX[spot-1]==Blank) ||
		    ((spot%WIDTH)+1<WIDTH && theBlankOX[spot+1]==Blank) ||
		    (spot-WIDTH>=0 && theBlankOX[spot-WIDTH]==Blank) ||
		    (spot+WIDTH<BOARDSIZE && theBlankOX[spot+WIDTH]==Blank)) {
			/*printf("Finishing cannot shift pieces\n");*/
			return FALSE;
		}
	}
	/*printf("Finishing cannot shift pieces true\n");*/
	return TRUE;
}


/************************************************************************
**
** NAME:        AllBlank
**
** DESCRIPTION: Return TRUE iff all spaces are blank.
**
** INPUTS:      BlankOX theBlankOX[BOARDSIZE] : The BlankOX array.
**
** OUTPUTS:     (BOOLEAN) TRUE iff all the spaces are blank.
**
************************************************************************/

BOOLEAN AllBlank(theBlankOX)
BlankOX theBlankOX[];
{
	int i;

	for(i = 0; i < BOARDSIZE; i++)
		if (theBlankOX[i] != Blank) return FALSE;

	return TRUE;
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

BOOLEAN AllFilledIn(theBlankOX)
BlankOX theBlankOX[];
{
	int i;

	for(i = 0; i < BOARDSIZE; i++)
		if (theBlankOX[i] == Blank) return FALSE;

	return TRUE;
}

/************************************************************************
**
** NAME:        CantFlipAnyPiece
**
** DESCRIPTION: Return TRUE iff no piece of the opposing player can be
**              flipped.
**
** INPUTS:      BlankOX theBlankOX[BOARDSIZE] : The BlankOX array.
**
** OUTPUTS:     (BOOLEAN) TRUE iff no piece of the opposing player can be
**              flipped.
**
************************************************************************/

BOOLEAN CantFlipAnyPiece(theBlankOX)
BlankOX theBlankOX[];
{
	int i;
	BlankOX opposingPlayer, opposingPlayerBig;
	BlankOX WhoseTurn();

	if (AllFilledIn(theBlankOX)) return TRUE;

	//determine the opposing player
	opposingPlayer = WhoseTurn(theBlankOX)==x ? o : x;
	opposingPlayerBig = opposingPlayer==x ? X : O;

	for(i = 0; i < BOARDSIZE; i++) {
		if (theBlankOX[i] == opposingPlayer ||
		    theBlankOX[i] == opposingPlayerBig) {
			if (((i%WIDTH)-1>=0 && theBlankOX[i-1]==Blank) ||
			    ((i%WIDTH)+1<=WIDTH-1 && theBlankOX[i+1]==Blank) ||
			    (i-WIDTH>=0 && theBlankOX[i-WIDTH]==Blank) ||
			    (i+WIDTH<=BOARDSIZE-1 && theBlankOX[i+WIDTH]==Blank)) {
				return FALSE;
			}
		}
	}

	return TRUE;
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
	int i, xcount = 0, ocount = 0;

	for(i = 0; i < BOARDSIZE; i++) {
		if(theBlankOX[i] == x || theBlankOX[i] == X)
			xcount++;
		else if(theBlankOX[i] == o || theBlankOX[i] == O)
			ocount++;
	}

	if(xcount == ocount)
		return(x); /* X always goes first */
	else
		return(o);
}

POSITION StringToPosition(char* board) {
	// FIXME: this is just a stub
	return atoi(board);
}


char* PositionToString(POSITION pos) {
	// FIXME: this is just a stub
	return "Implement Me";
}

//GM_DEFINE_BLANKOX_ENUM_BOARDSTRINGS()


char * PositionToEndData(POSITION pos) {
	return NULL;
}
