/************************************************************************
**
** NAME:        mlite3.c
**
** DESCRIPTION: Lite-3
**
** AUTHORS:     Alex Perelman
**              Babak Hamadani
**              Kehan Chen
**
** DATE:        11/16/22
**
**************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/

#include "gamesman.h"

POSITION gNumberOfPositions  = 2097152;  /* 2^21 */

POSITION gInitialPosition    =  0;
POSITION gMinimalPosition    =  0;
POSITION kBadPosition           = -1;

CONST_STRING kAuthorName         = "Alex Perelmen and Babak Hamadani";
CONST_STRING kGameName           = "Lite-3";
CONST_STRING kDBName = "lite3";
BOOLEAN kPartizan           = TRUE;
BOOLEAN kDebugMenu          = FALSE;
BOOLEAN kGameSpecificMenu   = TRUE;
BOOLEAN kTieIsPossible      = FALSE;
BOOLEAN kLoopy               = TRUE;
BOOLEAN kDebugDetermineValue = FALSE;
void*    gGameSpecificTclInit = NULL;

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

#define BOARDSIZE     10           /* 3x3 board, element 0 is whoseTurn */
#define NUMSYMMETRIES 8           /* 4 rotations, 4 flipped rotations */

typedef int SLOT;     /* A slot is the place where a piece moves from or to */

typedef enum players {
	x, o
} PLAYER_TURN;

typedef enum possibleBoardPieces {
	Blank, o1, o2, o3, x1, x2, x3
} BlankOOOXXX;

BOOLEAN gShowMoveSuccession = FALSE;

/** Variants **/

typedef enum possibleGameObjectives {
	THREE_IN_A_ROW, SURROUND, BOTH
} GameObjective;

GameObjective gGameObjective = THREE_IN_A_ROW; //by default

char *gBlankOOOXXXString[] = { "-", "O", "O", "O", "X", "X", "X"};
char *gBlankOOOXXXStringWithShow[] = { "-", "a", "b", "c", "1", "2", "3"};

STRING kHandleTextInputHelp =
        "\n\
Text Input Commands:\n\
-------------------\n\
?           : Brings up this list of Text Input Commands available\n\
s (or S)    : (S)how the values of all possible moves\n\
u (or U)    : (U)ndo last move (not possible at beginning position)\n\
r (or R)    : (R)eprint the position\n\
m (or M)    : Toggle Show (M)ove Succession\n\
h (or H)    : (H)elp\n\
a (or A)    : (A)bort the game\n\
q (or Q)    : (Q)uit"                                                                                                                                                                                                                                                                                                                                                                                                                   ;

/*static int gSymmetryMatrix[NUMSYMMETRIES][BOARDSIZE];*/

/* Proofs of correctness for the below arrays:
**
** FLIP						ROTATE
**
** 0 1 2	2 1 0		0 1 2		6 3 0		8 7 6		2 5 8
** 3 4 5  ->    5 4 3		3 4 5	->	7 4 1  ->	5 4 3	->	1 4 7
** 6 7 8	8 7 6		6 7 8		8 5 2		2 1 0		2 1 0
*/

/* This is the array used for flipping along the N-S axis */
int gFlipNewPosition[] = { 2, 1, 0, 5, 4, 3, 8, 7, 6 };

/* This is the array used for rotating 90 degrees clockwise */
int gRotate90CWNewPosition[] = { 6, 3, 0, 7, 4, 1, 8, 5, 2 };


/** Function Prototypes **/
void PositionToBlankOOOXXX(POSITION thePos, BlankOOOXXX *theBlankOOOXXX);
POSITION BlankOOOXXXToPosition(BlankOOOXXX *theBlankOOOXXX);
BOOLEAN ThreeInARow(BlankOOOXXX *theBlankOOOXXX, int a, int b, int c);
VALUE Surround(BlankOOOXXX *theBlankOOOXXX, PLAYER_TURN whoseTurn);

/** Changing Variants **/
void InitializeHelpStrings();

/************************************************************************
**
** NAME:        InitializeDatabases
**
** DESCRIPTION: Initialize the gDatabase, a global variable.
**
************************************************************************/

void InitializeGame() {
	InitializeHelpStrings();
}

void FreeGame() {}

// Set up the help strings based on the variant being played
// gGameObjective = THREE_IN_A_ROW, SURROUND, BOTH
void InitializeHelpStrings() {

	kHelpGraphicInterface =
	        "The LEFT button puts an X or O (depending on whether you went first\n\
or second) on the spot the cursor was on when you clicked. The MIDDLE\n\
button does nothing, and the RIGHT button is the same as UNDO, in that\n\
it reverts back to your your most recent position."                                                                                                                                                                                                                                           ;

	kHelpTextInterface    =
	        "On your turn, use the LEGEND to determine which number to choose (between\n\
1 and 9, with 1 at the upper left and 9 at the lower right) to correspond\n\
to the empty board position you desire and hit return. If at any point\n\
you have made a mistake, you can type u and hit return and the system will\n\
revert back to your most recent position."                                                                                                                                                                                                                                                                                                                                   ;

	kHelpOnYourTurn =
	        "You place one of your pieces on one of the empty board positions.";

	if (gGameObjective == SURROUND) {
		kHelpStandardObjective = "Surround your opponent�s piece with three of your\n\
pieces to win.";
	}
	if (gGameObjective == THREE_IN_A_ROW) {
		kHelpStandardObjective =
		        "To get three of your markers (either X or O) in a row, either\n\
horizontally, vertically, or diagonally. 3-in-a-row WINS."                                                                                          ;
	}
	if (gGameObjective == BOTH) {
		kHelpStandardObjective =
		        "Either get three of your pieces in a row either vertically, horizontally\n\
 or diagonally or surround your opponent to win."                                                                                                     ;
	}

	kHelpReverseObjective =
	        "To force your opponent into getting three of his markers (either X or\n\
O) in a row, either horizontally, vertically, or diagonally. 3-in-a-row\n\
LOSES."                                                                                                                                                                     ;

	kHelpTieOccursWhen = /* Should follow 'A Tie occurs when... */
	                     "the board fills up without either player getting three-in-a-row.";

	kHelpExample =
	        "         ( 1 2 3 )           : - - -\n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - - - \n\
         ( 7 8 9 )           : - - - \n\n\
Computer's move              :  3    \n\n\
         ( 1 2 3 )           : - - X \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - - - \n\
         ( 7 8 9 )           : - - - \n\n\
     Dan's move [(u)ndo/1-9] : { 2 } \n\n\
         ( 1 2 3 )           : - O X \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - - - \n\
         ( 7 8 9 )           : - - - \n\n\
Computer's move              :  6    \n\n\
         ( 1 2 3 )           : - O X \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - - X \n\
         ( 7 8 9 )           : - - - \n\n\
     Dan's move [(u)ndo/1-9] : { 9 } \n\n\
         ( 1 2 3 )           : - O X \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - - X \n\
         ( 7 8 9 )           : - - O \n\n\
Computer's move              :  5    \n\n\
         ( 1 2 3 )           : - O X \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - X X \n\
         ( 7 8 9 )           : - - O \n\n\
     Dan's move [(u)ndo/1-9] : { 7 } \n\n\
         ( 1 2 3 )           : - O X \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - X X \n\
         ( 7 8 9 )           : O - O \n\n\
Computer's move              :  4    \n\n\
         ( 1 2 3 )           : - O X \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : X X X \n\
         ( 7 8 9 )           : O - O \n\n\
Computer wins. Nice try, Dan."                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     ;

}

/************************************************************************
**
** NAME:        DebugMenu
**
** DESCRIPTION: Menu used to debub internal problems. Does nothing if
**              kDebugMenu == FALSE
**
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
	do {
		printf("\n\t----- Game-specific options for %s -----\n\n", kGameName);

		printf("\t Current Game Objective: ");

		if(gGameObjective == THREE_IN_A_ROW)
			printf("Three-In-A-Row Wins\n\n");
		else if(gGameObjective == SURROUND)
			printf("Surround Wins\n\n");
		else if(gGameObjective == BOTH)
			printf("Three-In-A-Row and Surround BOTH win.\n\n");

		printf("\t1)\tToggle Game Objective to Three-In-A-Row Wins\n");
		printf("\t2)\tToggle Game Objective to Surround Wins\n");
		printf("\t3)\tToggle Game Objective to Three-In-A-Row and Surround BOTH win\n");


		printf("\n\n\tb)\t(B)ack = Return to previous activity.\n");
		printf("\n\nSelect an option: ");

		switch(GetMyChar()) {
		case 'Q': case 'q':
			ExitStageRight();
			break;

		case '1':
			gGameObjective = THREE_IN_A_ROW;
			kHelpStandardObjective = "Be the first player to get three of your pieces in a row (horizontally, vertically or diagonally)";
			kHelpReverseObjective = "Force your opponent into getting three of his pieces in a row (horizontally, vertically or diagonally) first";
			break;
		case '2':
			gGameObjective = SURROUND;
			kHelpStandardObjective = "Be the first player to trap your opponent's piece, i.e. completely surround it with your three pieces";
			kHelpReverseObjective = "Force your opponent into trapping one of your pieces, i.e. completely surrounding it with his three pieces";
			break;
		case '3':
			gGameObjective = BOTH;
			kHelpStandardObjective = "Be the first player to get three of your pieces in a row or trap your opponent's piece";
			kHelpReverseObjective = "Force your opponent into getting three of his pieces in a row or trapping one of your piece";
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

void SetTclCGameSpecificOptions(int *theOptions) {
	if(theOptions[0] == 0)
		gGameObjective = THREE_IN_A_ROW;
	else if(theOptions[0] == 1)
		gGameObjective = SURROUND;
	else if(theOptions[0] == 2)
		gGameObjective = BOTH;
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
** CALLS:       PositionToBlankOOOXXX(POSITION,*BlankOOOXXX)
**              BlankOOOXXX WhosTurn(*BlankOOOXXX)
**
************************************************************************/

POSITION DoMove(POSITION thePosition, MOVE theMove) {
	int pieces = thePosition >> 1;

	int piece_o3 = pieces % 10;
	int piece_o2 = (pieces % 100) / 10;
	int piece_o1 = (pieces % 1000) / 100;

	int piece_x3 = (pieces % 10000) / 1000;
	int piece_x2 = (pieces % 100000) / 10000;
	int piece_x1 = (pieces % 1000000) / 100000;

	int whoseTurn = (thePosition & 1);

	if(theMove == 0)
	{
		gShowMoveSuccession = TRUE;
		return thePosition;
	}


	if(whoseTurn == x) //x's turn
	{
		if(piece_x1 == 0) //first piece not placed yet
		{
			piece_x1 = theMove;
		}
		else if(piece_x2 == 0) //second not placed yet
		{
			piece_x2 = theMove;
		}
		else if(piece_x3 == 0) //third not placed yet
		{
			piece_x3 = theMove;
		}
		else //all three in place, shift x2->x1, x3->x2, x3 = theMove or x4 (depending on 4piece game or not)
		{
			piece_x1 = piece_x2;
			piece_x2 = piece_x3;
			piece_x3 = theMove;
		}
	}
	else //o's turn
	{
		if(piece_o1 == 0) //first piece not placed yet
		{
			piece_o1 = theMove;
		}
		else if(piece_o2 == 0) //second not placed yet
		{
			piece_o2 = theMove;
		}
		else if(piece_o3 == 0) //third not placed yet
		{
			piece_o3 = theMove;
		}
		else //all three in place, shift o2->o1, o3->o2, o3 = theMove
		{
			piece_o1 = piece_o2;
			piece_o2 = piece_o3;
			piece_o3 = theMove;
		}
	}

	pieces = 0; //reset hashed position
	pieces += piece_x1 * 100000;
	pieces += piece_x2 * 10000;
	pieces += piece_x3 * 1000;
	pieces += piece_o1 * 100;
	pieces += piece_o2 * 10;
	pieces += piece_o3;

	thePosition = pieces << 1;

	whoseTurn = whoseTurn ? 0 : 1;
	thePosition = thePosition | whoseTurn;

	return(thePosition);
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

POSITION GetInitialPosition() {
	BlankOOOXXX theBlankOOOXXX[BOARDSIZE];
	signed char c;
	int i;


	printf("\n\n\t----- Get Initial Position -----\n");
	printf("\n\tPlease input the position to begin with.\n");
	printf("\tNote that it should be in the following format:\n\n");
	printf("O - -\nO - -            <----- EXAMPLE \n- X X\n\n");

	theBlankOOOXXX[0] = (BlankOOOXXX)x;
	i = 1;
	getchar();
	while(i < BOARDSIZE && (c = getchar()) != EOF) {
		if(c == 'x' || c == 'X')
			theBlankOOOXXX[i++] = (BlankOOOXXX)x;
		else if(c == 'o' || c == 'O' || c == '0')
			theBlankOOOXXX[i++] = (BlankOOOXXX)o;
		else if(c == '-')
			theBlankOOOXXX[i++] = Blank;
	}

	return(BlankOOOXXXToPosition(theBlankOOOXXX));
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
	printf("%8s's move              : %2d\n", computersName, computersMove);
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
**              PositionToBlankOOOXXX()
**
************************************************************************/

VALUE Primitive(POSITION position) {
	BlankOOOXXX theBlankOOOXXX[BOARDSIZE];
	PLAYER_TURN whoseTurn = position & 1;

	PositionToBlankOOOXXX(position,theBlankOOOXXX);
	

	if( gGameObjective == THREE_IN_A_ROW &&
	    (ThreeInARow(theBlankOOOXXX,1,2,3) ||
	     ThreeInARow(theBlankOOOXXX,4,5,6) ||
	     ThreeInARow(theBlankOOOXXX,7,8,9) ||
	     ThreeInARow(theBlankOOOXXX,1,4,7) ||
	     ThreeInARow(theBlankOOOXXX,2,5,8) ||
	     ThreeInARow(theBlankOOOXXX,3,6,9) ||
	     ThreeInARow(theBlankOOOXXX,1,5,9) ||
	     ThreeInARow(theBlankOOOXXX,3,5,7)) ) {
		return(gStandardGame ? lose : win);
	} else if( gGameObjective == SURROUND ) {
	         return Surround(theBlankOOOXXX, whoseTurn);
	} else if( gGameObjective == BOTH ) {
		if (ThreeInARow(theBlankOOOXXX,1,2,3) ||
	          ThreeInARow(theBlankOOOXXX,4,5,6) ||
	          ThreeInARow(theBlankOOOXXX,7,8,9) ||
	          ThreeInARow(theBlankOOOXXX,1,4,7) ||
	          ThreeInARow(theBlankOOOXXX,2,5,8) ||
	          ThreeInARow(theBlankOOOXXX,3,6,9) ||
	          ThreeInARow(theBlankOOOXXX,1,5,9) ||
	          ThreeInARow(theBlankOOOXXX,3,5,7)) {
			return gStandardGame ? lose : win;
		}
	    return Surround(theBlankOOOXXX, whoseTurn);
	} else {
		return(undecided);
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
** CALLS:       PositionToBlankOOOXXX()
**              GetValueOfPosition()
**              GetPrediction()
**
************************************************************************/

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn) {
	BlankOOOXXX theBlankOOOXXX[BOARDSIZE];
	PositionToBlankOOOXXX(position,theBlankOOOXXX);

	if(gShowMoveSuccession)
	{
		printf("\n         ( 1 2 3 )           : %s %s %s\n",
		       gBlankOOOXXXStringWithShow[(int)theBlankOOOXXX[1]],
		       gBlankOOOXXXStringWithShow[(int)theBlankOOOXXX[2]],
		       gBlankOOOXXXStringWithShow[(int)theBlankOOOXXX[3]] );
		printf("LEGEND:  ( 4 5 6 )  TOTAL:   : %s %s %s\n",
		       gBlankOOOXXXStringWithShow[(int)theBlankOOOXXX[4]],
		       gBlankOOOXXXStringWithShow[(int)theBlankOOOXXX[5]],
		       gBlankOOOXXXStringWithShow[(int)theBlankOOOXXX[6]] );
		printf("         ( 7 8 9 )           : %s %s %s %s\n\n",
		       gBlankOOOXXXStringWithShow[(int)theBlankOOOXXX[7]],
		       gBlankOOOXXXStringWithShow[(int)theBlankOOOXXX[8]],
		       gBlankOOOXXXStringWithShow[(int)theBlankOOOXXX[9]],
		       GetPrediction(position,playerName,usersTurn));
		printf("X's: 1-3 \t O's: a-c \n1 and a are next pieces to be moved\n\n");
	}
	else
	{
		printf("\n         ( 1 2 3 )           : %s %s %s\n",
		       gBlankOOOXXXString[(int)theBlankOOOXXX[1]],
		       gBlankOOOXXXString[(int)theBlankOOOXXX[2]],
		       gBlankOOOXXXString[(int)theBlankOOOXXX[3]] );
		printf("LEGEND:  ( 4 5 6 )  TOTAL:   : %s %s %s\n",
		       gBlankOOOXXXString[(int)theBlankOOOXXX[4]],
		       gBlankOOOXXXString[(int)theBlankOOOXXX[5]],
		       gBlankOOOXXXString[(int)theBlankOOOXXX[6]] );
		printf("         ( 7 8 9 )           : %s %s %s %s\n\n",
		       gBlankOOOXXXString[(int)theBlankOOOXXX[7]],
		       gBlankOOOXXXString[(int)theBlankOOOXXX[8]],
		       gBlankOOOXXXString[(int)theBlankOOOXXX[9]],
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
	BlankOOOXXX theBlankOOOXXX[BOARDSIZE];
	int i;

	if (Primitive(position) == undecided) {
		PositionToBlankOOOXXX(position,theBlankOOOXXX);
		for(i = 1; i < BOARDSIZE; i++) {
			if(theBlankOOOXXX[i] == Blank)
				head = CreateMovelistNode(i,head);
		}

		return(head);
	} else {
		return(NULL);
	}
}

/************************************************************************
**
** NAME:        HandleTextInput
**
** DESCRIPTION: Dispatch on the command the user types.
**
** INPUTS:      POSITION *thePosition : The position the user is at.
**              MOVE *theMove         : The move to fill with user's move.
**              STRING playerName     : The name of the player whose turn it is
**
** OUTPUTS:     USERINPUT : Oneof( Undo, Abort, Continue )
**
************************************************************************/

USERINPUT HandleTextInput(POSITION thePosition, MOVE *theMove, STRING playerName) {
	MOVE tmpMove;
	char tmpAns[2], input[MAXINPUTLENGTH];

	GetMyStr(input,MAXINPUTLENGTH);

	if(input[0] == '\0')
		PrintPossibleMoves(thePosition);

	else if (ValidTextInput(input))
		if(ValidMove(thePosition,tmpMove = ConvertTextInputToMove(input))) {
			*theMove = tmpMove;
			return(Move);
		}
		else
			PrintPossibleMoves(thePosition);

	else
		switch(input[0]) {
		case 'Q': case 'q':
			ExitStageRight();
			break;
		case 'u': case 'U':
			return(Undo);
		case 'a': case 'A':
			printf("\nSure you want to Abort? [no] :  ");
			GetMyStr(tmpAns,2);
			printf("\n");
			if(tmpAns[0] == 'y' || tmpAns[0] == 'Y')
				return(Abort);
			else
				return(Continue);
		case 'H': case 'h':
			HelpMenus();
			printf("");
			PrintPosition(thePosition, playerName, TRUE);
			break;
		case 'r': case 'R':
			PrintPosition(thePosition, playerName, TRUE);
			break;
		case 'm': case 'M':
			gShowMoveSuccession = (gShowMoveSuccession == TRUE) ? FALSE : TRUE;
			PrintPosition(thePosition, playerName, TRUE);
			break;
		case 's': case 'S':
			PrintValueMoves(thePosition);
			break;
		case '?':
			printf("%s",kHandleTextInputHelp);
			PrintPossibleMoves(thePosition);
			break;
		default:
			BadMenuChoice();
			printf("%s",kHandleTextInputHelp);
			PrintPossibleMoves(thePosition);
			break;
		}

	return(Continue); /* The default action is to return Continue */
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
		printf("%8s's move [(u)ndo/1-9] :  ", playerName);

		ret = HandleTextInput(thePosition, theMove, playerName);
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
	return((input[0] <= '9' && input[0] >= '1'));
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
	return((MOVE) input[0] - '0'); /* user input is 1-9, our rep. is 0-8 */
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

void MoveToString(MOVE theMove, char *moveStringBuffer) {
	/* The plus 1 is because the user thinks it's 1-9, but MOVE is 0-8 */
	snprintf(moveStringBuffer, 5, "%d", theMove);
}

/************************************************************************
*************************************************************************
**         EVERYTHING BELOW THESE LINES IS LOCAL TO THIS FILE
*************************************************************************
************************************************************************/

/************************************************************************
**
** NAME:        PositionToBlankOOOXXX
**
** DESCRIPTION: convert an internal position to that of a BlankOOOXXX.
**
** INPUTS:      POSITION thePos     : The position input.
**              BlankOOOXXX *theBlankOOOXXX : The converted BlankOOOXXX output array.
**
** CALLS:       BadElse()
**
************************************************************************/

void PositionToBlankOOOXXX(POSITION thePos, BlankOOOXXX *theBlankOOOXXX) {
	int i;
	int pieces = thePos >> 1;

	int whoseTurn = (thePos & 1);

	int piece_o3 = pieces % 10;
	int piece_o2 = (pieces % 100) / 10;
	int piece_o1 = (pieces % 1000) / 100;
	int piece_x3 = (pieces % 10000) / 1000;
	int piece_x2 = (pieces % 100000) / 10000;
	int piece_x1 = (pieces % 1000000) / 100000;

	for(i=0; i < BOARDSIZE; i++)
	{
		theBlankOOOXXX[i] = 0;
	}

	theBlankOOOXXX[piece_o3] = o3;
	theBlankOOOXXX[piece_o2] = o2;
	theBlankOOOXXX[piece_o1] = o1;

	theBlankOOOXXX[piece_x3] = x3;
	theBlankOOOXXX[piece_x2] = x2;
	theBlankOOOXXX[piece_x1] = x1;

	theBlankOOOXXX[0] = whoseTurn;
}


/************************************************************************
**
** NAME:        BlankOOOXXXToPosition
**
** DESCRIPTION: convert a BlankOOOXXX to that of an internal position.
**
** INPUTS:      BlankOOOXXX *theBlankOOOXXX : The converted BlankOOOXXX output array.
**
** OUTPUTS:     POSITION: The equivalent position given the BlankOOOXXX.
**
************************************************************************/

POSITION BlankOOOXXXToPosition(BlankOOOXXX *theBlankOOOXXX) {
	int i, result, whoseTurn;
	int powTen[] = {0, 100, 10, 1, 100000, 10000, 1000};
	POSITION position = 0;

	whoseTurn = theBlankOOOXXX[0];

	for(i = 1, result = 0; i < BOARDSIZE; i++)
		result += i * powTen[theBlankOOOXXX[i]];

	position = (result << 1) | whoseTurn;

	return(position);
}


/************************************************************************
**
** NAME:        ThreeInARow
**
** DESCRIPTION: Return TRUE iff there are three-in-a-row.
**
** INPUTS:      BlankOOOXXX theBlankOOOXXX[BOARDSIZE] : The BlankOOOXXX array.
**              int a,b,c                     : The 3 positions to check.
**
** OUTPUTS:     (BOOLEAN) TRUE iff there are three-in-a-row.
**
************************************************************************/

BOOLEAN ThreeInARow(BlankOOOXXX *theBlankOOOXXX, int a, int b, int c) {
	return (theBlankOOOXXX[a] >= x1 &&
	        theBlankOOOXXX[b] >= x1 &&
	        theBlankOOOXXX[c] >= x1)     ||
	       ((theBlankOOOXXX[a] > Blank && theBlankOOOXXX[a] <= o3) &&
	        (theBlankOOOXXX[b] > Blank && theBlankOOOXXX[b] <= o3) &&
	        (theBlankOOOXXX[c] > Blank && theBlankOOOXXX[c] <= o3)); //for 4piece <= o4
}

/************************************************************************
**
** NAME:        Surround
**
** DESCRIPTION: Return TRUE iff a player surrounded another's piece.
**
** EXAMPLE BOARDS: X - O    O X -
**                 0 X O    X X -    X wins both boards
**                 X - -    - O O
**
** INPUTS:      BlankOOOXXX theBlankOOOXXX[BOARDSIZE] : The BlankOOOXXX array.
**
** OUTPUTS:     (BOOLEAN) TRUE iff a player surrounded another's piece.
**
************************************************************************/

VALUE Surround(BlankOOOXXX *theBlankOOOXXX, PLAYER_TURN whoseTurn) {
	char* opponentPiece;
	char* middlePiece = gBlankOOOXXXString[theBlankOOOXXX[5]]; //is middle piece Blank, X or O?

	opponentPiece = (strcmp(middlePiece, "X") == 0) ? "O" : "X";

	if (strcmp(middlePiece, "-") != 0)
	{
		if( (gBlankOOOXXXString[theBlankOOOXXX[1]] == opponentPiece &&
		     gBlankOOOXXXString[theBlankOOOXXX[2]] == middlePiece &&
		     gBlankOOOXXXString[theBlankOOOXXX[4]] == middlePiece) ||
		    (gBlankOOOXXXString[theBlankOOOXXX[2]] == opponentPiece &&
		     gBlankOOOXXXString[theBlankOOOXXX[1]] == middlePiece &&
		     gBlankOOOXXXString[theBlankOOOXXX[3]] == middlePiece) ||
		    (gBlankOOOXXXString[theBlankOOOXXX[3]] == opponentPiece &&
		     gBlankOOOXXXString[theBlankOOOXXX[2]] == middlePiece &&
		     gBlankOOOXXXString[theBlankOOOXXX[6]] == middlePiece) ||
		    (gBlankOOOXXXString[theBlankOOOXXX[4]] == opponentPiece &&
		     gBlankOOOXXXString[theBlankOOOXXX[1]] == middlePiece &&
		     gBlankOOOXXXString[theBlankOOOXXX[7]] == middlePiece) ||
		    (gBlankOOOXXXString[theBlankOOOXXX[6]] == opponentPiece &&
		     gBlankOOOXXXString[theBlankOOOXXX[3]] == middlePiece &&
		     gBlankOOOXXXString[theBlankOOOXXX[9]] == middlePiece) ||
		    (gBlankOOOXXXString[theBlankOOOXXX[7]] == opponentPiece &&
		     gBlankOOOXXXString[theBlankOOOXXX[4]] == middlePiece &&
		     gBlankOOOXXXString[theBlankOOOXXX[8]] == middlePiece) ||
		    (gBlankOOOXXXString[theBlankOOOXXX[8]] == opponentPiece &&
		     gBlankOOOXXXString[theBlankOOOXXX[7]] == middlePiece &&
		     gBlankOOOXXXString[theBlankOOOXXX[9]] == middlePiece) ||
		    (gBlankOOOXXXString[theBlankOOOXXX[9]] == opponentPiece &&
		     gBlankOOOXXXString[theBlankOOOXXX[8]] == middlePiece &&
		     gBlankOOOXXXString[theBlankOOOXXX[6]] == middlePiece)
		    ) {
				if (strcmp(opponentPiece, "X") == 0) {
					if (whoseTurn == x) {
						return gStandardGame ? lose : win;
					} else {
						return gStandardGame ? win : lose;
					}
				} else if (strcmp(opponentPiece, "O") == 0) {
					if (whoseTurn == o) {
						return gStandardGame ? lose : win;
					} else {
						return gStandardGame ? win : lose;
					}
				}
			}
	}

	return undecided;
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

void MoveToSlots(MOVE theMove, SLOT *toSlot) {
	*toSlot   = theMove % (BOARDSIZE+1);
}

/************************************************************************
**
** NAME:        WhoseTurn
**
** DESCRIPTION: Return 0 if x's turn, 1 if o's turn
**
** INPUTS:      thePosition
**
** OUTPUTS:     0 for x's turn, 1 for o's turn
**
************************************************************************/

int WhoseTurn(POSITION thePosition) {
	return (thePosition & 1);
}

int NumberOfOptions() {
	return 6;
}

int getOption() {
	int ret;

	if(gGameObjective == THREE_IN_A_ROW) ret = 1;
	else if(gGameObjective == SURROUND) ret = 2;
	else ret = 3;
	ret += 3 * (!gStandardGame);
	return ret;
}

void setOption(int option) {
	gStandardGame = (option < 4);
	option %= 3;
	if(option == 0) gGameObjective = BOTH;
	else if(option == 1) gGameObjective = THREE_IN_A_ROW;
	else gGameObjective = SURROUND;

	InitializeHelpStrings();
}

POSITION StringToPosition(char *positionString) {
	int turn;
	char *board;
	if (ParseStandardOnelinePositionString(positionString, &turn, &board)) {
		BlankOOOXXX theBlankOOOXXX[BOARDSIZE];
		theBlankOOOXXX[0] = (BlankOOOXXX)(turn == 2 ? o : x);
		for (int i = 1; i < BOARDSIZE; i++) {
			switch (board[i-1]) {
				default:
					fprintf(stderr, "Error: Unexpected char in position\n");
					break;
				case '-':
					theBlankOOOXXX[i] = Blank;
					break;
				case 'a':
					theBlankOOOXXX[i] = o1;
					break;
				case 'b':
					theBlankOOOXXX[i] = o2;
					break;
				case 'c':
					theBlankOOOXXX[i] = o3;
					break;
				case '1':
					theBlankOOOXXX[i] = x1;
					break;
				case '2':
					theBlankOOOXXX[i] = x2;
					break;
				case '3':
					theBlankOOOXXX[i] = x3;
					break;
			}
		}
		return BlankOOOXXXToPosition(theBlankOOOXXX);
	}
	return NULL_POSITION;
}

void PositionToAutoGUIString(POSITION position, char *autoguiPositionStringBuffer) {
	BlankOOOXXX theBlankOOOXXX[BOARDSIZE];
	PositionToBlankOOOXXX(position, theBlankOOOXXX);
	int turn = (!WhoseTurn(position)) ? 1 : 2;

	char board[BOARDSIZE];
	for (int i = 0; i < BOARDSIZE-1; i++) {
		switch (theBlankOOOXXX[i+1]) {
			default:
				fprintf(stderr, "Error: Unexpected position\n");
				break;
			case Blank:
				board[i] = '-';
				break;
			case o1:
				board[i] = 'a';
				break;
			case o2:
				board[i] = 'b';
				break;
			case o3:
				board[i] = 'c';
				break;
			case x1:
				board[i] = '1';
				break;
			case x2:
				board[i] = '2';
				break;
			case x3:
				board[i] = '3';
				break;
		}
	}
	board[BOARDSIZE-1] = '\0'; // Make sure to null-terminate your board.
	AutoGUIMakePositionString(turn, board, autoguiPositionStringBuffer);
}

void MoveToAutoGUIString(POSITION position, MOVE move, char *autoguiMoveStringBuffer) {
  	(void) position;
 	SLOT toSlot;
	MoveToSlots(move, &toSlot);
	AutoGUIMakeMoveButtonStringA('h', toSlot - 1, 'x', autoguiMoveStringBuffer);
}
