/************************************************************************
**
** NAME:        machi.c
**
** DESCRIPTION: Achi
**
** AUTHOR:      Jeffrey Chiang
**              Jennifer Lee
**	            Jesse Phillips
**
** DATE:        02/11/2003
**
**************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/

#include "gamesman.h"

POSITION gNumberOfPositions  = 39366; /*19683;*/  /* 3^9 */

POSITION gInitialPosition    =  0;
POSITION gMinimalPosition    =  0;
POSITION kBadPosition        = -1; /* This can never be the rep. of a position */

CONST_STRING kAuthorName         = "Jeffrey Chiang, Jennifer Lee, and Jesse Phillips";
CONST_STRING kGameName           = "Achi";
CONST_STRING kDBName = "achi";

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
CONST_STRING kHelpGraphicInterface =
        "Help strings not initialized.";

CONST_STRING kHelpTextInterface    =
        "Help strings not initialized.";

CONST_STRING kHelpOnYourTurn =
        "Help strings not initialized.";

CONST_STRING kHelpStandardObjective =
        "Help strings not initialized.";

CONST_STRING kHelpReverseObjective =
        "Help strings not initialized.";

CONST_STRING kHelpTieOccursWhen =   /* Should follow 'A Tie occurs when... */
                            "Help strings not initialized.";

CONST_STRING kHelpExample =
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

char *gBlankOXString[] = { "�", "#", "$" };

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
BOOLEAN AllFilledIn(BlankOX *theBlankOX);
BOOLEAN ThreeInARow(BlankOX *theBlankOX, int a, int b, int c);
BOOLEAN phase1(BlankOX *theBlankOX);

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

void FreeGame() {}

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

void DebugMenu() {}

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
		break;
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

void SetTclCGameSpecificOptions(int theOptions[]) {
	/* No need to have anything here, we have no extra options */
	(void)theOptions;
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

POSITION DoMove(POSITION thePosition, MOVE theMove) {

	BlankOX theBlankOX[BOARDSIZE];
	int from, to;
	BlankOX whosTurn;

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

POSITION DoSymmetry(POSITION position, int symmetry) {
	int i;
	BlankOX theBlankOx[BOARDSIZE], symmBlankOx[BOARDSIZE];
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


POSITION GetCanonicalPosition(POSITION position) {
	POSITION newPosition, theCanonicalPosition;
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
** NAME:        GetInitialPosition
**
** DESCRIPTION: Ask the user for an initial position for testing. Store
**              it in the space pointed to by initialPosition;
**
** OUTPUTS:     POSITION initialPosition : The position to fill.
**
************************************************************************/

/*  we haven't changed this, but we probably should */

POSITION GetInitialPosition() {
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
		/* else do nothing */
	}

	whosTurn = o;
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

void PrintComputersMove(MOVE computersMove, STRING computersName) {
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

VALUE Primitive(POSITION position) {
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

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn) {
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

MOVELIST *GenerateMoves(POSITION position) {
	MOVELIST *head = NULL;
	BlankOX player;
	BlankOX theBlankOX[BOARDSIZE];
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

USERINPUT GetAndPrintPlayersMove(POSITION thePosition, MOVE *theMove, STRING playerName) {
	USERINPUT ret;

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

BOOLEAN ValidTextInput(STRING input) {
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

MOVE ConvertTextInputToMove(STRING input) {
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

void PrintMove(MOVE theMove) {
	/* The plus 1 is because the user thinks it's 1-9, but MOVE is 0-8 */
	if(theMove < 9) {
		printf("%d", theMove + 1);
	} else {
		/* else
		   printf("%d", theMove + 11); */
		printf("%d", theMove);
	}
}


STRING MoveToString(MOVE theMove) {
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

STRING _PositionToString(POSITION thePos) {
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

void PositionToBlankOX(POSITION thePos, BlankOX *theBlankOX, BlankOX *whosTurn) {
	int i;

	if(thePos >= POSITION_OFFSET) {
		*whosTurn = x; /* if the last character in the array is an x */
		thePos -= POSITION_OFFSET;
	}
	else
		*whosTurn = o;
	for(i = 8; i >= 0; i--) {
		if(thePos >= (POSITION)(x * g3Array[i])) {
			theBlankOX[i] = x;
			thePos -= x * g3Array[i];
		}
		else if(thePos >= (POSITION)(o * g3Array[i])) {
			theBlankOX[i] = o;
			thePos -= o * g3Array[i];
		}
		else if(thePos >= (POSITION)(Blank * g3Array[i])) {
			theBlankOX[i] = Blank;
			thePos -= Blank * g3Array[i];
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


POSITION BlankOXToPosition(BlankOX *theBlankOX, BlankOX whosTurn) {
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

BOOLEAN ThreeInARow(BlankOX *theBlankOX, int a, int b, int c) {
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

BOOLEAN AllFilledIn(BlankOX *theBlankOX) {
	BOOLEAN answer = TRUE;
	int i;

	for(i = 0; i < BOARDSIZE; i++)
		answer &= (theBlankOX[i] == o || theBlankOX[i] == x);

	return(answer);
}



BOOLEAN phase1(BlankOX *theBlankOX) {
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


int NumberOfOptions() {
	return 2*3;
}

int getOption() {
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
		return "Mis�re game with standard diagonal moves";
		break;
	case 2:
		return "Standard game with standard diagonal moves";
		break;
	case 3:
		return "Mis�re game with all possible diagonal moves";
		break;
	case 4:
		return "Standard game with all possible diagonal moves";
		break;
	case 5:
		return "Mis�re game with no diagonal moves";
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


void setOption(int option) {
	option -= 1;
	gStandardGame = option%2==1;
	allDiag = option/2%3==1;
	noDiag = option/2%3==2;

	InitializeHelpStrings();

}

POSITION ActualNumberOfPositions(int variant) {
	(void)variant;
	return 5390;
}

POSITION InteractStringToPosition(STRING str) {
	// Parse UWAPI standard position string & get UWAPI standard board string
	enum UWAPI_Turn turn;
	unsigned int num_rows, num_columns;
	STRING board;
	if (!UWAPI_Board_Regular2D_ParsePositionString(str, &turn, &num_rows, &num_columns, &board)) {
		// Failed to parse string
		return INVALID_POSITION;
	}

	// Validate parsed board size
	if (num_rows != 3 || num_columns != 3) {
		SafeFreeString(board); // Free the string!
		return INVALID_POSITION;
	}

	// Convert UWAPI standard board string to internal board representation
	BlankOX oxboard[BOARDSIZE];
	int i;
	for (i = 0; i < BOARDSIZE; i++) {
		if (board[i] == 'o') {
			oxboard[i] = o;
		} else if (board[i] == 'x') {
			oxboard[i] = x;
		} else if (board[i] == '-') {
			oxboard[i] = Blank;
		} else {
			SafeFreeString(board); // Free the string!
			return INVALID_POSITION;
		}
	}

	// Convert internal board representation to internal position
	BlankOX whosTurn = (turn == UWAPI_TURN_A) ? o : x;
	POSITION position = BlankOXToPosition(oxboard, whosTurn);

	// Return internal position
	SafeFreeString(board); // Free the string!
	return position;
}

STRING InteractPositionToString(POSITION pos) {
	// Convert internal position to internal board representation
	BlankOX oxboard[BOARDSIZE];
	BlankOX whosTurn;
	PositionToBlankOX(pos, oxboard, &whosTurn);

	// Convert internal board representation to UWAPI standard board string
	char board[BOARDSIZE + 1];
	int i;
	for (i = 0; i < BOARDSIZE; i++) {
		if (oxboard[i] == o) {
			board[i] = 'o';
		} else if (oxboard[i] == x) {
			board[i] = 'x';
		} else if (oxboard[i] == Blank) {
			board[i] = '-';
		}
	}
	board[BOARDSIZE] = '\0';

	// Return formatted UWAPI position string
	enum UWAPI_Turn turn = (whosTurn == o) ? UWAPI_TURN_A : UWAPI_TURN_B;
	return UWAPI_Board_Regular2D_MakePositionString(turn, 3, 3, board);
}

STRING InteractMoveToString(POSITION pos, MOVE mv) {
	BlankOX oxboard[BOARDSIZE];
	BlankOX whosTurn;
	PositionToBlankOX(pos, oxboard, &whosTurn);

	if (mv < 9) {
		// Add piece
		return UWAPI_Board_Regular2D_MakeAddStringWithSound('-', mv, 'x');
	} else {
		// Move piece
		return UWAPI_Board_Regular2D_MakeMoveStringWithSound(mv / 10 - 1, mv % 10 - 1, 'y');
	}
}
