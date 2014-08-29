// $Id: machi.c,v 1.34 2008-05-08 05:12:56 l156steven Exp $
/************************************************************************
**
** NAME:        machi.c
**
** DESCRIPTION: Achi
**
** AUTHOR:      Jeffrey Chiang
**              Jennifer Lee
**	                 Jesse Phillips
**
** DATE:        02/11/2003
**
** UPDATE HIST:
**
** 2/11/2003 - wrote printPosition, doMove, generateMoves excluding diagonal moves
**	2/13/2003 - wrote get input functions, print functions, fixed errors
**	2/20/2003 - wrote hash/unhash, whosemove, fixed domove, representation of our move
**	2/27/2003 - wrote help strings, added nodiag, alldiag variations
** 3/06/2003 - updated print position
** 4/02/2006 - added GetVarString to return English description of option hash
** ?/??/2008 - added symmetries (getCanonicalPosition, DoSymmetry)
**************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/

#include <stdio.h>
#include "gamesman.h"

POSITION gNumberOfPositions  = 39366; /*19683;*/  /* 3^9 */

POSITION gInitialPosition    =  0;
POSITION gMinimalPosition    =  0;
POSITION kBadPosition        = -1; /* This can never be the rep. of a position */

STRING kAuthorName         = "Jeffrey Chiang, Jennifer Lee, and Jesse Phillips";
STRING kGameName           = "Achi";

BOOLEAN kPartizan           = TRUE;
BOOLEAN kSupportsHeuristic  = TRUE;
BOOLEAN kSupportsSymmetries = TRUE;
BOOLEAN kSupportsGraphics   = TRUE;
BOOLEAN kDebugMenu          = FALSE;
BOOLEAN kGameSpecificMenu   = TRUE;
BOOLEAN kTieIsPossible      = FALSE;
BOOLEAN kLoopy               = TRUE;
BOOLEAN kDebugDetermineValue = FALSE;
void*    gGameSpecificTclInit = NULL;

// Help strings are placeholders to be filled in after InitializeGame
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

#define BOARDSIZE     9           /* 3x3 board */
#define NUMSYMMETRIES 8           /* 4 rotations, 4 flipped rotations */
#define WIDTH 3
#define POSITION_OFFSET  19683       /* 3^9 */

typedef enum possibleBoardPieces {
	Blank, o, x
} BlankOX;

char *gBlankOXString[] = { "·", "#", "$" };

/* Powers of 3 - this is the way I encode the position, as an integer */
int g3Array[] =          { 1, 3, 9, 27, 81, 243, 729, 2187, 6561, 19683 };

/* function prototypes */
POSITION BlankOXToPosition(BlankOX* theBlankOX, BlankOX whosTurn);
void PositionToBlankOX(POSITION thePos,BlankOX *theBlankOX,BlankOX *whosTurn);
STRING GetVarString();
STRING MoveToString (MOVE);
STRING _PositionToString(POSITION);
POSITION ActualNumberOfPositions(int variant);
POSITION GetCanonicalPosition(POSITION position);

/* Variants */
BOOLEAN allDiag = FALSE;
BOOLEAN noDiag = FALSE;

void InitializeHelpStrings();

int gSymmetryMatrix[NUMSYMMETRIES][BOARDSIZE];

/* Proofs of correctness for the below arrays:
**
** FLIP						ROTATE
**
** 0 1 2	2 1 0		0 1 2		6 3 0		8 7 6		2 5 8
** 3 4 5  ->    5 4 3		3 4 5	->	7 4 1  ->	5 4 3	->	1 4 7
** 6 7 8	8 7 6		6 7 8		8 5 2		2 1 0		0 3 6
*/

/* This is the array used for flipping along the N-S axis */
int gFlipNewPosition[] = { 2, 1, 0, 5, 4, 3, 8, 7, 6 };

/* This is the array used for rotating 90 degrees clockwise */
int gRotate90CWNewPosition[] = { 6, 3, 0, 7, 4, 1, 8, 5, 2 };



void InitializeGame()
{
	//Symmetry code (directly from mttt.c)
	gCanonicalPosition = GetCanonicalPosition;

	int i, j, temp; /* temp is used for debugging */

	if(kSupportsSymmetries) { /* Initialize gSymmetryMatrix[][] */
		for(i = 0; i < BOARDSIZE; i++) {
			temp = i;
			for(j = 0; j < NUMSYMMETRIES; j++) {
				if(j == NUMSYMMETRIES/2)
					temp = gFlipNewPosition[i];
				if(j < NUMSYMMETRIES/2)
					temp = gSymmetryMatrix[j][i] = gRotate90CWNewPosition[temp];
				else
					temp = gSymmetryMatrix[j][i] = gRotate90CWNewPosition[temp];
			}
		}
	}

	gMoveToStringFunPtr = &MoveToString;
	gCustomUnhash = &_PositionToString;

	gGetVarStringPtr = &GetVarString;
	gActualNumberOfPositionsOptFunPtr = &ActualNumberOfPositions;
	InitializeHelpStrings();
}

void FreeGame()
{
}

/*****
** void InitializeHelpStrings()
**
** Set up the help strings based on the variant being played
**
** Variants:
** BOOLEAN allDiag
** BOOLEAN noDiag
*****/
void InitializeHelpStrings() {

	//printf("InitializeHelpStrings called.");

	kHelpGraphicInterface =
	        "There is currently no graphic interface\n";

	kHelpTextInterface    =
	        "There are two types of moves in Achi: place moves and slide moves. \n\
PLACE MOVES: \n\
The game starts out with place moves, when players alternates turns placing \n\
each of their three piece on the board. During this turn players attempt \n\
to get three in a row, either diagonally, horizontally, or vertically. If \n\
a player is able to get three in a row, then he or she wins the game. If \n\
both players are unable to get three in a row after placing all their pieces \n\
then players start sliding their pieces. \n\
SLIDE MOVES: \n\
In this phase, players alternate turns sliding their pieces along the line \n\
to any empty adjacent spot. In the standard version of the game, players \n\
may move up, down, or diagonally to any unoccupied, connected spot. \n\
FIRST: Choose a piece to move and type the number (1-9) of the space it is on. \n\
SECOND: Type the number (1-9) of the space you would like to slide to. \n\
i.e. 23, 45, 69 etc.\n"                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  ;

	if (noDiag) {
		kHelpOnYourTurn =
		        "Players alternate turns placing each of their three pieces on one of the nine \n\
spaces. During this placing phase, players attempt to connect any of \n\
their three pieces in a row. If no one succeeds, then the first player slides \n\
any of his three pieces to any open adjacent spot. The pieces can only be moved \n\
to a spot that is up or down from the previous spaces. Diagonal sliding is not\nallowed."                                                                                                                                                                                                                                                                                                                                                          ;
	} else {
		kHelpOnYourTurn =
		        "For the first six turns, each player puts a piece on an empty board position. \n\
After all six pieces are on the board, move any one of your pieces along a \n\
line to an open spot on the board."                                                                                                                                                                                          ;
	}

	kHelpStandardObjective =
	        "To get three pieces in a row either diagonally, vertically, or horizontally.";

	kHelpReverseObjective =
	        "To force your opponent into getting three of his or her pieces in a row, \n\
 either horizontally, vertically, or diagonally."                                                                                              ;

	kHelpTieOccursWhen = /* Should follow 'A Tie occurs when... */
	                     "NEVER!!!";

	kHelpExample =
	        "         ( 1 2 3 )           : - - - \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - - - \n\
         ( 7 8 9 )           : - - - \n\n\
Computer's move              :  3    \n\n\
         ( 1 2 3 )           : - - X \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - - - \n\
         ( 7 8 9 )           : - - - \n\n\
     Dan's move  : { 2 } \n\n\
         ( 1 2 3 )           : - O X \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - - - \n\
         ( 7 8 9 )           : - - - \n\n\
Computer's move              :  6    \n\n\
         ( 1 2 3 )           : - O X \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - - X \n\
         ( 7 8 9 )           : - - - \n\n\
     Dan's move  : { 9 } \n\n\
         ( 1 2 3 )           : - O X \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - - X \n\
         ( 7 8 9 )           : - - O \n\n\
Computer's move              :  5    \n\n\
         ( 1 2 3 )           : - O X \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - X X \n\
         ( 7 8 9 )           : - - O \n\n\
     Dan's move  : { 7 } \n\n\
         ( 1 2 3 )           : - O X \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - X X \n\
         ( 7 8 9 )           : O - O \n\n\
Computer's move              :  54   \n\n\
         ( 1 2 3 )           : - O X \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : X - X \n\
         ( 7 8 9 )           : O - O \n\n\
     Dan's move  : { 98 } \n\n\
         ( 1 2 3 )           : - O X \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : X - X \n\
         ( 7 8 9 )           : O O - \n\n\
Computer's move              :  35    \n\n\
         ( 1 2 3 )           : - O - \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : X X X \n\
         ( 7 8 9 )           : O - O \n\n\
Computer wins. Nice try, Dan."                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        ;
}


/************************************************************************
**
** NAME:        DebugMenu
**
** DESCRIPTION: Menu used to debug internal problems. Does nothing if
**              kDebugMenu == FALSE
**
**
**        DebugMenu adapted from Dan Garcia's mttt.c
************************************************************************/

void DebugMenu()
{
	char GetMyChar();
	void tttppm();

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
		case 'H': case 'h':
			HelpMenus();
			break;
		case 'C': case 'c': /* Write PPM to s(C)reen */
			tttppm(0,0);
			break;
		case 'I': case 'i': /* Write PPM to f(I)le */
			tttppm(0,1);
			break;
		case 'S': case 's': /* Write Postscript to (S)creen */
			tttppm(1,0);
			break;
		case 'F': case 'f': /* Write Postscript to (F)ile */
			tttppm(1,1);
			break;
		case 'B': case 'b':
			return;
		default:
			BadMenuChoice();
			HitAnyKeyToContinue();
			break;
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

void GameSpecificMenu()
{
	char GetMyChar();

	printf("\n");
	printf("Achi Game Specific Menu\n\n");
	printf("1) No diagonal moves \n");
	printf("2) All possible diagonal moves\n");
	printf("3) Standard diagonal moves (default)\n");
	printf("b) Back to previous menu\n\n");

	printf("Current option:   %s\n", allDiag ? "All diagonal moves" : noDiag ? "No diagonal moves" : "Standard diagonal moves");
	printf("Select an option: ");

	switch(GetMyChar()) {
	case 'Q': case 'q':
		ExitStageRight();
	case '1':
		noDiag = TRUE;
		allDiag = FALSE;
		break;
	case '2':
		noDiag = FALSE;
		allDiag = TRUE;
		break;
	case '3':
		noDiag = FALSE;
		allDiag = FALSE;
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

	BlankOX theBlankOX[BOARDSIZE];
	int from, to;
	BlankOX whosTurn;
	BOOLEAN phase1();


	PositionToBlankOX(thePosition,theBlankOX, &whosTurn);
	from = 0;
	to = 0;

	if(phase1(theBlankOX)) {
		theBlankOX[theMove] = whosTurn; // phase1 moves are 0-8
	}
	else {

		from = theMove/10;
		to = theMove%10;
		theBlankOX[to-1] = theBlankOX[from-1];
		theBlankOX[from-1] = Blank;
	}
	if(whosTurn == o)
		whosTurn = x;
	else
		whosTurn = o;

	return BlankOXToPosition(theBlankOX,whosTurn);
}

/**************************************************/
/**************** SYMMETRY FUN BEGIN **************/
/**************************************************/

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
	POSITION newPosition, theCanonicalPosition, DoSymmetry();
	int i;

	theCanonicalPosition = position;

	for(i = 0; i < NUMSYMMETRIES; i++) {

		newPosition = DoSymmetry(position, i);
		if(newPosition < theCanonicalPosition) /* THIS is the one */
			theCanonicalPosition = newPosition; /* set it to the ans */
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
	int i;
	BlankOX theBlankOx[BOARDSIZE], symmBlankOx[BOARDSIZE];
	POSITION BlankOXToPosition();

	BlankOX whosTurn; //has to be same person's move
	BlankOX whosTurnSym;

	PositionToBlankOX(position,theBlankOx, &whosTurn);
	PositionToBlankOX(position,symmBlankOx, &whosTurnSym); /* Make copy */

	/* Copy from the symmetry matrix */

	for(i = 0; i < BOARDSIZE; i++)
		symmBlankOx[i] = theBlankOx[gSymmetryMatrix[symmetry][i]];


	return(BlankOXToPosition(symmBlankOx, whosTurnSym));
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

/*  we haven't changed this, but we probably should */

POSITION GetInitialPosition()
{
	BlankOX theBlankOX[BOARDSIZE], whosTurn;
	signed char c;
	int i;


	printf("\n\n\t----- Get Initial Position -----\n");
	printf("\n\tPlease input the position to begin with.\n");
	printf("\tNote that it should be in the following format:\n\n");
	printf("O - -\nO - -            <----- EXAMPLE \n- X X\n\n");

	i = 0;
	getchar();
	while(i < BOARDSIZE && (c = getchar()) != EOF) {
		if(c == 'x' || c == 'X')
			theBlankOX[i++] = x;
		else if(c == 'o' || c == 'O' || c == '0')
			theBlankOX[i++] = o;
		else if(c == '-')
			theBlankOX[i++] = Blank;
		else
			; /* do nothing */
	}

	/*
	   getchar();
	   printf("\nNow, whose turn is it? [O/X] : ");
	   scanf("%c",&c);
	   if(c == 'x' || c == 'X')
	   whosTurn = x;
	   else
	   whosTurn = o;
	 */

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
	BOOLEAN phase1;
	if(computersMove < 9) {
		computersMove++; // internally moves are 0-8
		phase1 = TRUE;
	}
	else
		phase1 = FALSE;
	printf("%8s's move              : %2d\n", computersName, phase1 ? computersMove : computersMove );
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
** CALLS:       BOOLEAN ThreeInARow()
**              BOOLEAN AllFilledIn()
**              PositionToBlankOX()
**
************************************************************************/

VALUE Primitive(position)
POSITION position;
{
	BOOLEAN ThreeInARow(), AllFilledIn();
	BlankOX theBlankOX[BOARDSIZE];
	BlankOX whosTurn;

	PositionToBlankOX(position,theBlankOX, &whosTurn);

	if( ThreeInARow(theBlankOX,0,1,2) ||
	    ThreeInARow(theBlankOX,3,4,5) ||
	    ThreeInARow(theBlankOX,6,7,8) ||
	    ThreeInARow(theBlankOX,0,3,6) ||
	    ThreeInARow(theBlankOX,1,4,7) ||
	    ThreeInARow(theBlankOX,2,5,8) ||
	    ThreeInARow(theBlankOX,0,4,8) ||
	    ThreeInARow(theBlankOX,2,4,6) )
		return(gStandardGame ? lose : win);
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

void PrintPosition(position,playerName,usersTurn)
POSITION position;
STRING playerName;
BOOLEAN usersTurn;
{
	BlankOX theBlankOx[BOARDSIZE];
	BlankOX whosTurn;

	PositionToBlankOX(position,theBlankOx, &whosTurn);
	if(noDiag) {
		printf("\n         ( 1 2 3 )           : %s - %s - %s\n",
		       gBlankOXString[(int)theBlankOx[0]],
		       gBlankOXString[(int)theBlankOx[1]],
		       gBlankOXString[(int)theBlankOx[2]] );
		printf("                               |   |   |\n");
		printf("LEGEND:  ( 4 5 6 )  TOTAL:   : %s - %s - %s\n",
		       gBlankOXString[(int)theBlankOx[3]],
		       gBlankOXString[(int)theBlankOx[4]],
		       gBlankOXString[(int)theBlankOx[5]] );
		printf("                               |   |   |\n");
		printf("         ( 7 8 9 )           : %s - %s - %s %s\n\n",
		       gBlankOXString[(int)theBlankOx[6]],
		       gBlankOXString[(int)theBlankOx[7]],
		       gBlankOXString[(int)theBlankOx[8]],
		       GetPrediction(position,playerName,usersTurn));
	}
	else if(allDiag) {
		printf("\n         ( 1 2 3 )           : %s - %s - %s\n",
		       gBlankOXString[(int)theBlankOx[0]],
		       gBlankOXString[(int)theBlankOx[1]],
		       gBlankOXString[(int)theBlankOx[2]] );
		printf("                               | X | X |\n");
		printf("LEGEND:  ( 4 5 6 )  TOTAL:   : %s - %s - %s\n",
		       gBlankOXString[(int)theBlankOx[3]],
		       gBlankOXString[(int)theBlankOx[4]],
		       gBlankOXString[(int)theBlankOx[5]] );
		printf("                               | X | X |\n");
		printf("         ( 7 8 9 )           : %s - %s - %s %s\n\n",
		       gBlankOXString[(int)theBlankOx[6]],
		       gBlankOXString[(int)theBlankOx[7]],
		       gBlankOXString[(int)theBlankOx[8]],
		       GetPrediction(position,playerName,usersTurn));
	}


	else {
		printf("\n         ( 1 2 3 )           : %s - %s - %s\n",
		       gBlankOXString[(int)theBlankOx[0]],
		       gBlankOXString[(int)theBlankOx[1]],
		       gBlankOXString[(int)theBlankOx[2]] );
		printf("                               | \\ | / |\n");
		printf("LEGEND:  ( 4 5 6 )  TOTAL:   : %s - %s - %s\n",
		       gBlankOXString[(int)theBlankOx[3]],
		       gBlankOXString[(int)theBlankOx[4]],
		       gBlankOXString[(int)theBlankOx[5]] );
		printf("                               | / | \\ |\n");
		printf("         ( 7 8 9 )           : %s - %s - %s %s\n\n",
		       gBlankOXString[(int)theBlankOx[6]],
		       gBlankOXString[(int)theBlankOx[7]],
		       gBlankOXString[(int)theBlankOx[8]],
		       GetPrediction(position,playerName,usersTurn));
	}
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
	BlankOX player;
	BlankOX theBlankOX[BOARDSIZE];
	BOOLEAN phase1();
	BlankOX whosTurn;
	MOVE aMove;
	int i;

	PositionToBlankOX(position, theBlankOX, &whosTurn);
	player = whosTurn;
	if (Primitive(position) == undecided) {

		if(phase1(theBlankOX)) {
			for(i = 0; i < BOARDSIZE; i++) {
				if(theBlankOX[i] == Blank)
					head = CreateMovelistNode(i,head); // moves are 0-index (0-8)
			}
		}
		else {
			for (i=0; i<BOARDSIZE; i++) {
				if (theBlankOX[i]==player) {

					//move left
					if ((i%WIDTH)-1>=0 && theBlankOX[i-1]==Blank) {
						aMove = i*10 + i - 1 + 11;
						head = CreateMovelistNode(aMove, head);
					}

					//move right
					if ((i%WIDTH)+1<=WIDTH-1 && theBlankOX[i+1]==Blank) {
						aMove = i*10 + i + 1 + 11;
						head = CreateMovelistNode(aMove, head);
					}

					//move up
					if (i-WIDTH>=0 && theBlankOX[i-WIDTH]==Blank) {
						aMove = i*10 + i - WIDTH + 11;
						head = CreateMovelistNode(aMove, head);
					}

					//move down
					if (i+WIDTH<=BOARDSIZE-1 && theBlankOX[i+WIDTH]==Blank) {
						aMove = i*10 + i + WIDTH + 11;
						head = CreateMovelistNode(aMove, head);
					}

				}
			}


			if(!noDiag) {
				for(i=8; i>=0; i-=2) {
					if(i!=4) {
						if(theBlankOX[i] == whosTurn && theBlankOX[4] == Blank) {
							aMove = i*10 + 4 + 11;
							head = CreateMovelistNode(aMove, head);
						}
					}
				}

				if(theBlankOX[4] == whosTurn) {
					for(i=8; i>=0; i-=2) {
						if(i!=4) {
							if(theBlankOX[i] == Blank && theBlankOX[4] == whosTurn) {
								aMove = 40 + i + 11;
								head = CreateMovelistNode(aMove, head);
							}
						}
					}
				}



				if(allDiag) {

					for(i = 7; i>=1; i-=2) {
						if(theBlankOX[i] == whosTurn && (i == 1 || i == 7)) {
							if(theBlankOX[3] == Blank) {
								aMove = (i*10) + 11 + 3;
								head = CreateMovelistNode(aMove, head);
							}
							if(theBlankOX[5] == Blank) {
								aMove = (i*10) + 11 + 5;
								head = CreateMovelistNode(aMove, head);
							}
						}
						if(theBlankOX[i] == whosTurn && (i == 3 || i == 5)) {
							if(theBlankOX[1] == Blank) {
								aMove = (i*10) + 11 + 1;
								head = CreateMovelistNode(aMove, head);
							}
							if(theBlankOX[7] == Blank) {
								aMove = (i*10) + 11 + 7;
								head = CreateMovelistNode(aMove, head);
							}
						}
					}
				}
			}
		}

		return (head);
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
	BOOLEAN ValidMove();
	USERINPUT ret, HandleDefaultTextInput();

	do {
		printf("%8s's move :  ", playerName);

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
	if (!input[1])
		return(input[0] <= '9' && input[0] >= '1');
	return(input[0] <= '9' && input[0] >= '1' && input[1] <= '9' && input[1] >= '1');
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
	MOVE theMove = 0;
	if (!input[1])
		return((MOVE) input[0] - '1'); /* user input is 1-9, our rep. is 0-8 */
	else {
		theMove = ((MOVE) input[0] - '0') * 10;
		theMove = theMove + ((MOVE) input[1] - '0');
	}
	return theMove;
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
	/* The plus 1 is because the user thinks it's 1-9, but MOVE is 0-8 */
	if(theMove < 9) {
		printf("%d", theMove + 1);
	} else {
		/* else
		   printf("%d", theMove + 11); */
		printf("%d", theMove);
	}
}


STRING MoveToString(theMove)
MOVE theMove;
{
	/* The plus 1 is because the user thinks it's 1-9, but MOVE is 0-8 */
	if(theMove < 9) {
		STRING move = (STRING) SafeMalloc(2);
		sprintf(move, "%d", theMove + 1);
		return move;
	} else {
		STRING move = (STRING) SafeMalloc(3);
		sprintf(move, "%d", theMove);
		return move;
	}
}

STRING _PositionToString( thePos )
POSITION thePos;
{
	BlankOX* turn = SafeMalloc( sizeof(BlankOX) );
	BlankOX* board = SafeMalloc( sizeof(BlankOX)*BOARDSIZE );

	char* str = (STRING) SafeMalloc(BOARDSIZE+1);

	PositionToBlankOX( thePos, board, turn );

	int i;
	for( i = 0; i < BOARDSIZE; ++i ) {
		if( board[i] == Blank ) {
			str[i] = ' ';
		} else if( board[i] == x ) {
			str[i] = 'X';
		} else if( board[i] == o ) {
			str[i] = 'O';
		}
	}
	str[BOARDSIZE] = '\0';

	SafeFree( turn );
	SafeFree( board );
	return str;
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
**              BlankOX *whosTurn     The converted turn of the position
**
** CALLS:       BadElse()
**
************************************************************************/

void PositionToBlankOX(thePos,theBlankOX,whosTurn)
POSITION thePos;
BlankOX *theBlankOX, *whosTurn;
{
	int i;

	if(thePos >= POSITION_OFFSET) {
		*whosTurn = x; /* if the last character in the array is an x */
		thePos -= POSITION_OFFSET;
	}
	else
		*whosTurn = o;
	for(i = 8; i >= 0; i--) {
		if(thePos >= ((int)x * g3Array[i])) {
			theBlankOX[i] = x;
			thePos -= (int)x * g3Array[i];
		}
		else if(thePos >= ((int)o * g3Array[i])) {
			theBlankOX[i] = o;
			thePos -= (int)o * g3Array[i];
		}
		else if(thePos >= ((int)Blank * g3Array[i])) {
			theBlankOX[i] = Blank;
			thePos -= (int)Blank * g3Array[i];
		}
		else
			BadElse("PositionToBlankOX");
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


POSITION BlankOXToPosition(theBlankOX, whosTurn)
BlankOX *theBlankOX,whosTurn;

{
	int i;
	POSITION position = 0;
	
	for(i = 0; i < BOARDSIZE; i++){
		position += g3Array[i] * (int)theBlankOX[i]; /* was (int)position... */
	}

	if(whosTurn == x)
		position += POSITION_OFFSET; /* account for whose turn it is */

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

BOOLEAN ThreeInARow(theBlankOX,a,b,c)
BlankOX theBlankOX[];
int a,b,c;
{
	return(     theBlankOX[a] == theBlankOX[b] &&
	            theBlankOX[b] == theBlankOX[c] &&
	            theBlankOX[c] != Blank );
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
	BOOLEAN answer = TRUE;
	int i;

	for(i = 0; i < BOARDSIZE; i++)
		answer &= (theBlankOX[i] == o || theBlankOX[i] == x);

	return(answer);
}



BOOLEAN phase1(BlankOX *theBlankOX)
{
	int count = 0;
	int i = 0;
	while(i < BOARDSIZE)
	{
		if(theBlankOX[i] != Blank)
			count++;
		if(count >= 6)
			return FALSE;
		i++;
	}
	return TRUE;
}



STRING kDBName = "achi";

int NumberOfOptions()
{
	return 2*3;
}

int getOption()
{
	int option = 1;
	if(gStandardGame) option += 1;
	if(allDiag) option += 1 *2;
	if(noDiag) option += 2 *2;

	return option;
}

STRING GetVarString() {
	switch (getOption())
	{
	case 1:
		return "Misère game with standard diagonal moves";
		break;
	case 2:
		return "Standard game with standard diagonal moves";
		break;
	case 3:
		return "Misère game with all possible diagonal moves";
		break;
	case 4:
		return "Standard game with all possible diagonal moves";
		break;
	case 5:
		return "Misère game with no diagonal moves";
		break;
	case 6:
		return "Standard game with no diagonal moves";
		break;
	default:
		BadElse("GetVarString");
		break;
	}
	return "String not set for this option";
}


void setOption(int option)
{
	option -= 1;
	gStandardGame = option%2==1;
	allDiag = option/2%3==1;
	noDiag = option/2%3==2;

	InitializeHelpStrings();

}

POSITION ActualNumberOfPositions(int variant) {
	return 5390;
}


// $Log: not supported by cvs2svn $
// Revision 1.33  2006/10/03 08:10:02  scarr2508
// added ActualNumberOfPositions
// -sean
//
// Revision 1.32  2006/04/11 02:10:52  kmowery
//
//
// Added PositionToString and set gCustomUnhash (used in the java gui)
//
// Revision 1.31  2006/04/05 11:42:00  arabani
// Fixed XML file writing.
//
// Revision 1.30  2006/02/26 08:31:15  kmowery
//
// Changed MToS to MoveToString
//
// Revision 1.29  2006/02/12 02:30:58  kmowery
//
// Changed MoveToString to be gMoveToStringFunPtr.  Updated already existing MoveToString implementations (Achi, Dodgem, SquareDance, and Othello)
//
// Revision 1.28  2006/02/03 06:08:39  hevanm
// fixed warnings. I will leave the real bugs to retro hehehehe.
//
// Revision 1.27  2006/01/03 00:19:34  hevanm
// Added types.h. Cleaned stuff up a little. Bye bye gDatabase.
//
// Revision 1.26  2005/12/28 18:34:03  ddgarcia
// Fixed some trailing \n\ in the text strings...
//
// Revision 1.25  2005/12/27 10:57:50  hevanm
// almost eliminated the existance of gDatabase in all files, with some declarations commented earlier that need to be hunt down and deleted from the source file.
//
// Revision 1.24  2005/12/27 05:25:19  ciokita
// *** empty log message ***
//
// Revision 1.23  2005/12/21 00:54:22  hevanm
// database updates and various commenting through out the modules where getValueOfPosition is declared outside of core code.... bad bad bad
//
// Revision 1.22  2005/12/08 04:00:22  ogren
// removed debug string. -Elmer
//
// Revision 1.21  2005/12/08 03:56:29  ogren
// achi has holder text for HelpOnYourTurn with noDiag. -Elmer
//
// Revision 1.20  2005/12/08 03:24:33  ogren
// created InitializeHelpStrings() to prepare for dynamic help strings.  -Elmer
//
// Revision 1.19  2005/12/08 01:26:18  yulikjan
// Added MoveToString.
//
// Revision 1.18  2005/05/06 07:24:55  nizebulous
// Finally fixed ALL the function prototypes so that there are NO warnings
// when gamesman compiles.  YAY!
//      -Dom
//
// Revision 1.17  2005/05/05 02:19:03  ogren
// local prototyped void tttppm() in DebugMenu() -Elmer
//
// Revision 1.16  2005/05/04 22:47:47  ciokita
// fixed help strings
//
// Revision 1.15  2005/04/27 21:22:03  ogren
// fixed CVS tags, turned off kDebugMenu.  GameTree printer still in DebugMenu, however. -Elmer
//

POSITION StringToPosition(char* board) {
	// FIXME
	POSITION pos = 0;
	if (GetValue(board, "pos", GetUnsignedLongLong, &pos)) {
		return pos;
	} else {
		return INVALID_POSITION;
	}
}

char* PositionToString(POSITION pos) {
	BlankOX board[BOARDSIZE];
	BlankOX whoseMove;
	PositionToBlankOX(pos, board, &whoseMove);
	char* boardStr = malloc(sizeof(char) * BOARDSIZE+1);
	int i;
	for (i = 0; i < BOARDSIZE; i++) {
		if (board[i] == o) {
			boardStr[i] = 'o';
		}
		else if (board[i] == x) {
			boardStr[i] = 'x';
		}
		else if (board[i] == Blank) {
			boardStr[i] = ' ';
		}
	}
	boardStr[BOARDSIZE] = '\0';
	return MakeBoardString(boardStr, "pos", StrFromI(pos), "");
}

//GM_DEFINE_BLANKOX_ENUM_BOARDSTRINGS()

char * PositionToEndData(POSITION pos) {
	return NULL;
}
