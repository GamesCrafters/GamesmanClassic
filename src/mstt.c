/************************************************************************
**
** NAME:        mstt.c
**
** DESCRIPTION: Shift-Tac-Toe
**
** AUTHOR:      David Chen & Ling Xiao  -  University of California at Berkeley
**
**
** DATE:        03/06/01
**
**************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/

#include <stdio.h>
#include "gamesman.h"

extern STRING gValueString[];

POSITION gNumberOfPositions  = 14348907;  /* 3^15 */

POSITION gInitialPosition    =  (59049 + 177147 + 531441) + 1;
POSITION gMinimalPosition    =  (59049 + 177147 + 531441) + 1;

STRING kAuthorName         = "David Chen and Ling Xiao";
STRING kGameName           = "Shift-Tac-Toe";
BOOLEAN kPartizan           = TRUE;
BOOLEAN kDebugMenu          = FALSE;
BOOLEAN kGameSpecificMenu   = TRUE;
BOOLEAN kTieIsPossible      = FALSE;
BOOLEAN kLoopy               = TRUE;
BOOLEAN kDebugDetermineValue = FALSE;
POSITION kBadPosition           = -1;
void*    gGameSpecificTclInit = NULL;

STRING kHelpGraphicInterface =
        "The LEFT button puts an X or O (depending on whetherxinn you went first\n\
or second) in the slot the arrow which you clicked is pointing to (if\n\
you clicked on one of the three arrows at the top.)  Or it shifts the\n\
row left or right depending on which arrow you clicked (if you clicked\n\
on one of the six arrows on the sides.) The MIDDLE button does nothing,\n\
and the RIGHT button is the same as UNDO, in that it reverts back to your\n\
your most recent position."                                                                                                                                                                                                                                                                                                                                                                                                                                                                        ;

STRING kHelpTextInterface    =
        "On your turn, use the LEGEND to determine which number to choose (between\n\
1 and 9, 1 through 3 puts a piece in the chose slot and 4 through 9 shifts\n\
the chosen row left or right) to correspond to the empty board position or\n\
the shifting you desire and hit return.  If at any point you have made a\n\
mistake, you can type u and hit return and the system will revert back to\n\
your most recent position."                                                                                                                                                                                                                                                                                                                                                                                                           ;

STRING kHelpOnYourTurn =
        "You place one of your pieces on one of the empty board positions, or you\n\
shift one of the rows."                                                                                     ;

STRING kHelpStandardObjective =
        "To get three of your markers (either X or O) in a row, either\n\
horizontally, vertically, or diagonally. 3-in-a-row WINS."                                                                          ;

STRING kHelpReverseObjective =
        "To force your opponent into getting three of his markers (either X or\n\
O) in a row, either horizontally, vertically, or diagonally. 3-in-a-row\n\
LOSES."                                                                                                                                                             ;

STRING kHelpTieOccursWhen =   /* Should follow 'A Tie occurs when... */
                            "the board fills up without either player getting three-in-a-row.";

STRING kHelpExample = "stuff";
/*
   "moves:
         0 1 2
      3 (0 0 0) 6
      4 (0 0 0) 7
      5 (0 0 0) 8
   ";
 */

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

#define BOARDSIZE     15           /* 3x3 board */
/*#define NUMSYMMETRIES 8            4 rotations, 4 flipped rotations */

typedef enum possibleBoardPieces {
	Blank, o, x
} BlankOX;

/* function prototypes */
POSITION BlankOXToPosition(BlankOX* theBlankOX);
void PositionToBlankOX(POSITION thePos,BlankOX  *theBlankOX);

STRING MoveToString( MOVE );

/* The position contains a last digit that records the turn, 1 - o's, 2 - x's*/
int g3Array[] = { 1, 3, 9, 27, 81, 243, 729, 2187, 6561, 19683, 59049, 177147, 531441, 1594323, 4782969, 14348907};

char *gBlankOXString[] = { "-", "O", "X" };

/* Powers of 3 - this is the way I encode the position, as an integer */


BOOLEAN gFlatMode = FALSE; /*The board is oriented vertically w/ gravity by default*/

BOOLEAN gQueuePiece = FALSE;  /* you can't have a piece waiting if the slot is full*/
BOOLEAN gSwivel = FALSE; /* the rows don't have the option to rotate */
BOOLEAN gWrapAround = FALSE; /* the rows don't wrap around */
BOOLEAN gExtraSlider = FALSE; /*only the horizontal sliders by default*/



/*int gSymmetryMatrix[NUMSYMMETRIES][BOARDSIZE];*/

/* Proofs of correctness for the below arrays:
**
** FLIP						ROTATE
**
** 0 1 2	2 1 0		0 1 2		6 3 0		8 7 6		2 5 8
** 3 4 5  ->    5 4 3		3 4 5	->	7 4 1  ->	5 4 3	->	1 4 7
** 6 7 8	8 7 6		6 7 8		8 5 2		2 1 0		2 1 0
*/

/* This is the array used for flipping along the N-S axis */
/*int gFlipNewPosition[] = { 2, 1, 0, 5, 4, 3, 8, 7, 6 };*/

/* This is the array used for rotating 90 degrees clockwise */
/*int gRotate90CWNewPosition[] = { 6, 3, 0, 7, 4, 1, 8, 5, 2 };*/

/************************************************************************
**
** NAME:        InitializeDatabases
**
** DESCRIPTION: Initialize the gDatabase, a global variable.
**
************************************************************************/

void InitializeGame()
{
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

void DebugMenu() {
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
	POSITION GetInitialPosition();

	do {
		printf("\n\t----- Game-specific options for %s -----\n\n", kGameName);

		printf("\tCurrent Initial Position:\n");
		PrintPosition(gInitialPosition, gPlayerName[kPlayerOneTurn], kHumansTurn);

		printf("\tI)\tChoose the (I)nitial position\n\n");

		printf("\tO)\t(O)rientation of the board toggles from %s to %s\n",
		       gFlatMode ? "FLAT" : "VERTICAL",
		       !gFlatMode ? "FLAT" : "VERTICAL");
		printf("\t  \t(different options are avavilabe in each orientation)\n\n");

		printf("\tA)\t(A)bility to rotate rows toggles from %s to %s\n",
		       gSwivel ? "ENABLED" : "DISABLED",
		       !gSwivel ? "ENABLED" : "DISABLED");

		printf("\tR)\t(R)ows wrapping toggles from %s to %s\n",
		       gWrapAround ? "WRAP AROUND" : "NO WRAPPING",
		       !gWrapAround ? "WRAP AROUND" : "NO WRAPPING");

		printf("\n\tOptions specific to the orientation:\n");

		if(gFlatMode) {
			printf("\tE)\t(E)xtra sliders toggles from %s to %s\n",
			       gExtraSlider ? "AVAILABLE" : "NOT AVAILABLE",
			       !gExtraSlider ? "AVAILABLE" : "NOT AVAILABLE");
		}
		else {
			printf("\tP)\t(P)ieces can be queued at top toggles from %s to %s\n",
			       gQueuePiece ? "TRUE" : "FALSE",
			       !gQueuePiece ? "TRUE" : "FALSE");
		}

		printf("\n\n\tb)\t(B)ack = Return to previous activity.\n");
		printf("\n\nSelect an option: ");

		switch(GetMyChar()) {
		case 'Q': case 'q':
			ExitStageRight();
		case 'H': case 'h':
			HelpMenus();
			break;
		case 'i': case 'I':
			gInitialPosition = GetInitialPosition();
			break;
		case 'o': case 'O':
			gFlatMode = !gFlatMode;
			if(gFlatMode) {
				gQueuePiece = FALSE;
			}
			else {
				gExtraSlider = FALSE;
			}
			break;
		case 'a': case 'A':
			gSwivel = !gSwivel;
			break;
		case 'r': case 'R':
			gWrapAround = !gWrapAround;
			break;
		case 'b': case 'B':
			return;
		case 'e': case 'E':
			if (gFlatMode) {
				gExtraSlider = !gExtraSlider;
			}
			else {
				printf("\nSorry, I don't know that option. Try another.\n");
				HitAnyKeyToContinue();
			}
			break;
		case 'p': case 'P':
			if (!gFlatMode) {
				gQueuePiece = !gQueuePiece;
			}
			else {
				printf("\nSorry, I don't know that option. Try another.\n");
				HitAnyKeyToContinue();
			}
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

POSITION DoMove(POSITION thePosition, MOVE theMove)
{

	BlankOX theBoard[BOARDSIZE], WhoseTurn();
	POSITION thePos;
	int empty = 0;
	void printBoard(), PerformGravity(), ShiftRows();
	int colEmptyPos();

	PositionToBlankOX(thePosition,theBoard);


	if (gFlatMode) {
		switch(theMove)
		{
		case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8:
			if (theBoard[theMove] == Blank) {
				theBoard[theMove] = WhoseTurn(thePosition);
			}
			else {
				printf("ERROR: ILLEGAL MOVE");
			}
			break;
		case 9: case 10: case 11:
			ShiftRows(theBoard,theMove,0);
			break;
		case 12: case 13: case 14:
			ShiftRows(theBoard,theMove-3,1);
			break;
		case 15: case 16: case 17:
			ShiftRows(theBoard,theMove-3,0);
			break;
		case 18: case 19: case 20:
			ShiftRows(theBoard,theMove-6,1);
			break;
		default:
			printf("Invalid Move %d", theMove);

			break;
		}
	}

	else {

		switch(theMove)
		{
		case 0:
		case 1:
		case 2:
			empty = colEmptyPos(theBoard,(int)theMove);
			if (empty == 3) {
				printf("illegal move, the slot is full");
				return thePosition;
			}
			theBoard[theMove*3 + empty] = WhoseTurn(thePosition);
			break;

		case 3:
		case 4:
		case 5:
			ShiftRows(theBoard,theMove+6,0);
			break;

		case 6:
		case 7:
		case 8:
			ShiftRows(theBoard,theMove+3,1);
			break;
		}

		PerformGravity(theBoard);
	}

	thePos = BlankOXToPosition(theBoard);
	thePos = thePos + (WhoseTurn(thePosition)%2+1);

	if(kDebugDetermineValue) {
		printf("move: %d \n",theMove);
		printBoard(thePos);
	}
	return(thePos);
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
	BlankOX theBlankOX[BOARDSIZE];
	signed char c;
	int i;


	printf("\n\n\t----- Get Initial Position -----\n");
	printf("\n\tPlease input the position to begin with.\n");
	printf("\tNote that it should be in the following format:\n\n");
	printf("O - -\nO - -            <----- EXAMPLE \n- X X\n\n");

	i = 0;
	for(i = 0; i < BOARDSIZE-3; i++)
		theBlankOX[i] = Blank;

	for(i = 9; i < BOARDSIZE; i++)
		theBlankOX[i] = (BlankOX) 1;

	getchar();
	while(i < (BOARDSIZE-3) && (c = getchar()) != EOF) {
		if(c == 'x' || c == 'X')
			theBlankOX[i++] = x;
		else if(c == 'o' || c == 'O' || c == '0')
			theBlankOX[i++] = o;
		else if(c == '-')
			theBlankOX[i++] = Blank;
		else
			; /* do nothing */
	}

	printf("Get Initial Move Called");
	return(BlankOXToPosition(theBlankOX) + x);
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
	printf("%8s's move              : %2d\n", computersName, computersMove+1);
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
	BlankOX WhoseTurn();
	BlankOX NotWhoseTurn = WhoseTurn(position)%2 + 1;
	BlankOX WhichTurn = WhoseTurn(position);

	PositionToBlankOX(position,theBlankOX);

	if( (ThreeInARow(theBlankOX,0,1,2) == NotWhoseTurn) ||
	    (ThreeInARow(theBlankOX,3,4,5) == NotWhoseTurn) ||
	    (ThreeInARow(theBlankOX,6,7,8) == NotWhoseTurn) ||
	    (ThreeInARow(theBlankOX,0,3,6) == NotWhoseTurn) ||
	    (ThreeInARow(theBlankOX,1,4,7) == NotWhoseTurn) ||
	    (ThreeInARow(theBlankOX,2,5,8) == NotWhoseTurn) ||
	    (ThreeInARow(theBlankOX,0,4,8) == NotWhoseTurn) ||
	    (ThreeInARow(theBlankOX,2,4,6) == NotWhoseTurn)) {
		return(gStandardGame ? lose : win);
	}
	else if( (ThreeInARow(theBlankOX,0,1,2) == WhichTurn) ||
	         (ThreeInARow(theBlankOX,3,4,5) == WhichTurn) ||
	         (ThreeInARow(theBlankOX,6,7,8) == WhichTurn) ||
	         (ThreeInARow(theBlankOX,0,3,6) == WhichTurn) ||
	         (ThreeInARow(theBlankOX,1,4,7) == WhichTurn) ||
	         (ThreeInARow(theBlankOX,2,5,8) == WhichTurn) ||
	         (ThreeInARow(theBlankOX,0,4,8) == WhichTurn) ||
	         (ThreeInARow(theBlankOX,2,4,6) == WhichTurn)) {
		return(gStandardGame ? win : lose);
	}
	/*  el
	   else if(AllFilledIn(theBlankOX))
	   return(tie);*/
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
	int i;
	//  VALUE GetValueOfPosition();
	BlankOX theBlankOx[BOARDSIZE];

	PositionToBlankOX(position,theBlankOx);

	if (gFlatMode) {

		/*   printf("\n                   1 2 3 \n");*/


		for(i = 2; i >= 0; i--) {
			printf("  %d > ", 12-i);
			switch((int)theBlankOx[11-i]) {
			case 0:
				printf("- - | ");
				break;
			case 1:
				printf("  - | ");
				break;
			case 2:
				printf("    | ");
				break;
			default:
				printf("ERROR: corrupt data in shifter position!");
			}

			printf("%d %d %d ", i+1, i+4, i+7);
			/*     gBlankOXString[(int)theBlankOx[i]],
			       gBlankOXString[(int)theBlankOx[i+3]],
			       gBlankOXString[(int)theBlankOx[i+6]]); */

			switch((int)theBlankOx[11-i]) {
			case 0:
				printf("|     ");
				break;
			case 1:
				printf("| -   ");
				break;
			case 2:
				printf("| - - ");
				break;
			default:
				printf("ERROR: corrupt data in shifter position!");
			}
			printf("< %d         ", 15-i);


			printf("         %d > ", 12-i);
			switch((int)theBlankOx[11-i]) {
			case 0:
				printf("- - | ");
				break;
			case 1:
				printf("  - | ");
				break;
			case 2:
				printf("    | ");
				break;
			default:
				printf("ERROR: corrupt data in shifter position!");
			}

			printf("%s %s %s ",
			       gBlankOXString[(int)theBlankOx[i]],
			       gBlankOXString[(int)theBlankOx[i+3]],
			       gBlankOXString[(int)theBlankOx[i+6]]);

			switch((int)theBlankOx[11-i]) {
			case 0:
				printf("|     ");
				break;
			case 1:
				printf("| -   ");
				break;
			case 2:
				printf("| - - ");
				break;
			default:
				printf("ERROR: corrupt data in shifter position!");
			}
			printf("< %d \n\n", 15-i);
		}

		printf("          %s\n\n",  GetPrediction(position,playerName,usersTurn));
	}

	else {


		printf("\n                   1 2 3 \n");



		for(i = 2; i >= 0; i--) {
			printf("         %d > ", 6-i);
			switch((int)theBlankOx[11-i]) {
			case 0:
				printf("- - | ");
				break;
			case 1:
				printf("  - | ");
				break;
			case 2:
				printf("    | ");
				break;
			default:
				printf("ERROR: corrupt data in shifter position!");
			}

			printf("%s %s %s ",
			       gBlankOXString[(int)theBlankOx[i]],
			       gBlankOXString[(int)theBlankOx[i+3]],
			       gBlankOXString[(int)theBlankOx[i+6]]);

			switch((int)theBlankOx[11-i]) {
			case 0:
				printf("|     ");
				break;
			case 1:
				printf("| -   ");
				break;
			case 2:
				printf("| - - ");
				break;
			default:
				printf("ERROR: corrupt data in shifter position!");
			}
			printf("< %d \n\n", 9-i);
		}

		printf("          %s\n\n",  GetPrediction(position,playerName,usersTurn));
	}
	/**  printf("         5 > %s %s %s < 8          :  > %d %s  \n\n",
	** gBlankOXString[(int)theBlankOx[0]],
	** gBlankOXString[(int)theBlankOx[3]],
	** gBlankOXString[(int)theBlankOx[6]],
	** (int)theBlankOx[11],
	** GetPrediction(position,playerName,usersTurn));
	**/
	/*printBoard(position);*/
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
	BlankOX theBlankOX[BOARDSIZE];
	int i;

	PositionToBlankOX(position,theBlankOX);

	if (Primitive(position) == undecided) {
		if (gFlatMode) {
			for(i = 0; i < 9; i++) {
				if(theBlankOX[i] == Blank)
					head = CreateMovelistNode(i,head);
			}
			for(i = 9; i < 12; i++) {
				if (gWrapAround) {
					head = CreateMovelistNode(i, head);
					head = CreateMovelistNode(i+3, head);
				}
				else if(theBlankOX[i] == 0) {
					head = CreateMovelistNode(i, head);
				}
				else if (theBlankOX[i] == 1) {
					head = CreateMovelistNode(i+3, head);
					head = CreateMovelistNode(i, head);
				}
				else if (theBlankOX[i] == 2) {
					head = CreateMovelistNode(i+3, head);
				}
			}

			if (gExtraSlider) {
				for(i = 12; i < 15; i++) {
					if (gWrapAround) {
						head = CreateMovelistNode(i+3, head);
						head = CreateMovelistNode(i+6, head);
					}
					else if (theBlankOX[i] == 0) {
						head = CreateMovelistNode(i+3, head);
					}
					else if (theBlankOX[i] == 1) {
						head = CreateMovelistNode(i+6, head);
						head = CreateMovelistNode(i+3, head);
					}
					else if (theBlankOX[i] == 2) {
						head = CreateMovelistNode(i+6, head);
					}
					else {
						printf("ERROR in GenerateMove(), BlankOX for extra slider has invalid value");
					}
				}
			}
		}

		else {
			for(i = 0; i < 9; i += 3) {
				if(theBlankOX[i+2] == Blank) {
					head = CreateMovelistNode(i/3,head);
				}
			}
			for (i = 9; i < 12; i++) {
				if (gWrapAround) {
					head = CreateMovelistNode(i-6, head);
					head = CreateMovelistNode(i-3, head);
				}
				else if(theBlankOX[i] == 0) {
					head = CreateMovelistNode(i-6, head);
				}
				else if (theBlankOX[i] == 1) {
					head = CreateMovelistNode(i-6, head);
					head = CreateMovelistNode(i-3, head);
				}
				else if (theBlankOX[i] == 2) {
					head = CreateMovelistNode(i-3, head);
				}
			}
		}

		return(head);

	} else {
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
		printf("%8s's move [(u)ndo/0-8] :  ", playerName);

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
	int inputNum = atoi(input);
	return (inputNum <= 27 && inputNum >= 1);
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
	return((MOVE) (atoi(input) - 1));
	/*  int inputNum = atoi(input);
	    return((MOVE) input[0] - '0'); */
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
	STRING m = (STRING) SafeMalloc( 3 );
	sprintf( m, "%d", theMove+1);
	return m;
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
	int i;
	thePos = (thePos - (thePos%3))/3; /*take out last turn bit*/

	for(i = BOARDSIZE-1; i >= 0; i--) {
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
		else {
			BadElse("PositionToBlankOX");
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

	for(i = 0; i < BOARDSIZE; i++)
		position += g3Array[i] * (int)theBlankOX[i]; /* was (int)position... */

	return(position*3);
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
** CALLS:       POSITION DoSymmetry(POSITION, int)
**
************************************************************************/

POSITION GetCanonicalPosition(position)
POSITION position;
{
	/*  POSITION newPosition, theCanonicalPosition, DoSymmetry();
	    int i;

	    theCanonicalPosition = position;

	    for(i = 0 ; i < NUMSYMMETRIES ; i++) {

	    newPosition = DoSymmetry(position, i);    / get new /
	    if(newPosition < theCanonicalPosition)    / THIS is the one /
	    theCanonicalPosition = newPosition;   / set it to the ans /
	    }

	    return(theCanonicalPosition);
	 */
	return position;
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
** CALLS:       POSITION DoSymmetry(POSITION, int)
**
************************************************************************/

MOVE DecodeMove(thePosition, canPosition, move)
POSITION thePosition, canPosition;
MOVE move;
{
	/*  int i;*/
	/*for(i = 0 ; i < NUMSYMMETRIES ; i++)*/
	/*if(canPosition == DoSymmetry(thePosition, i))*/    /* THIS is the one */
	/*return((MOVE) gSymmetryMatrix[i][move]); */
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
/*
   POSITION DoSymmetry(position, symmetry)
     POSITION position;
     int symmetry;
   {  int i;
   BlankOX theBlankOx[BOARDSIZE], symmBlankOx[BOARDSIZE];

   PositionToBlankOX(position,theBlankOx);
   PositionToBlankOX(position,symmBlankOx); / Make copy /

   / Copy from the symmetry matrix /

   for(i = 0 ; i < BOARDSIZE ; i++)
    symmBlankOx[i] = theBlankOx[gSymmetryMatrix[symmetry][i]];

   return(BlankOXToPosition(symmBlankOx));
   }
 */
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
	if (theBlankOX[a] == theBlankOX[b] &&
	    theBlankOX[b] == theBlankOX[c] &&
	    theBlankOX[c] != Blank ) {
		return theBlankOX[a];
	}
	else
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
/*
   BOOLEAN AllFilledIn(theBlankOX)
     BlankOX theBlankOX[];
   {
   BOOLEAN answer = TRUE;
   int i;

   for(i = 0; i < BOARDSIZE; i++)
    answer &= (theBlankOX[i] == o || theBlankOX[i] == x);

   return(answer);
   }
 */
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

BlankOX WhoseTurn(int thePos)
{
	/*BlankOX turn = (BlankOX)((int)thePos % 3);*/
	return((BlankOX)((int)thePos % 3));
	/* in our TicTacToe, x always goes first */
}

void printBoard(POSITION thePos)
{
	BlankOX tempBoard[BOARDSIZE];

	PositionToBlankOX(thePos,tempBoard);

	printf("| %d | %d | %d | > %d \n", tempBoard[2], tempBoard[5], tempBoard[8], tempBoard[9]);
	printf("| %d | %d | %d | > %d \n", tempBoard[1], tempBoard[4], tempBoard[7], tempBoard[10]);
	printf("| %d | %d | %d | > %d \n", tempBoard[0], tempBoard[3], tempBoard[6], tempBoard[11]);
	printf("_____________________ \n");
}


void PerformGravity(BlankOX* theBoard) {
	int i = 0;
	for (i = 0; i < 9; i += 3) {
		if (theBoard[i] == 0 && theBoard[i+1] != 0) {
			theBoard[i] = theBoard[i+1];
			theBoard[i+1] = 0;
		}
		if (theBoard[i+1] == 0 && theBoard[i+2] != 0) {
			theBoard[i+1] = theBoard[i+2];
			theBoard[i+2] = 0;
		}

		if (theBoard[i] == 0 && theBoard[i+1] != 0) {
			theBoard[i] = theBoard[i+1];
			theBoard[i+1] = 0;
		}
	}
	return;
}


void ShiftRows(BlankOX* theBoard, int theRow, int theDirection) {
	int temp;

	switch(theRow) {
	case 9: case 10: case 11:
		if (theDirection == 0) {

			if (!gWrapAround && theBoard[theRow] == 2) {
				printf("ILLEGAL MOVE: shifter already all the way to the right");
				return;
			}

			temp = theBoard[17-theRow];

			theBoard[17-theRow] = theBoard[14-theRow];
			theBoard[14-theRow] = theBoard[11-theRow];
			if (gWrapAround) {
				theBoard[11-theRow]= temp;
			}
			else {
				theBoard[11-theRow] = 0;
				theBoard[theRow]++;
			}
		}

		else if (theDirection == 1) {

			if (!gWrapAround && theBoard[theRow] == 0) {
				printf("ILLEGAL MOVE: shifter already all the way to the left");
				return;
			}

			temp = theBoard[11-theRow];

			theBoard[11-theRow] = theBoard[14-theRow];
			theBoard[14-theRow] = theBoard[17-theRow];
			if (gWrapAround) {
				theBoard[17-theRow]= temp;
			}
			else {
				theBoard[17-theRow] = 0;
				theBoard[theRow]--;
			}
		}

		else {
			printf("UNKNOWN DIRECTION IN ShiftRow");
		}
		break;


	case 12: case 13: case 14:
		if (gExtraSlider) {
			if (theDirection == 0) {

				if (!gWrapAround && theBoard[theRow] == 2) {
					printf("ILLEGAL MOVE: shifter already all the way to the bottom");
					return;
				}

				temp = theBoard[(theRow%12)*3];

				theBoard[(theRow%12)*3] = theBoard[(theRow%12)*3+1];
				theBoard[(theRow%12)*3+1] = theBoard[(theRow%12)*3+2];
				if (gWrapAround) {
					theBoard[(theRow%12)*3+2]= temp;
				}
				else {
					theBoard[(theRow%12)*3+2] = 0;
					theBoard[theRow]++;
				}
			}

			else if (theDirection == 1) {

				if (!gWrapAround && theBoard[theRow] == 0) {
					printf("ILLEGAL MOVE: shifter already all the way to the top");
					return;
				}

				temp = theBoard[(theRow%12)*3+2];

				theBoard[(theRow%12)*3+2] = theBoard[(theRow%12)*3+1];
				theBoard[(theRow%12)*3+1] = theBoard[(theRow%12)*3];
				if (gWrapAround) {
					theBoard[(theRow%12)*3]= temp;
				}
				else {
					theBoard[(theRow%12)*3] = 0;
					theBoard[theRow]--;
				}
			}

			else {
				printf("UNKNOWN DIRECTION IN ShiftRow");
			}
		}
		else {
			printf("ILLEGAL ROW TO SHIFT in ShiftRows, no Extra Sliders");
		}
		break;

	default:
		printf("ILLEGAL ROW TO SHIFT in ShiftRows");
		break;
	}

	return;
}

int colEmptyPos(BlankOX* theBoard, int col)
{
	int empty = 3;
	int i;

	for(i = (col*3); i < ((col + 1) * 3); i++)
	{
		if(theBoard[i] == 0) {
			empty = i - (col*3);
			break;
		}
	}
	return empty;
}



STRING kDBName = "stt";

int NumberOfOptions()
{
	return 2*2*2*4;
}

int getOption()
{
	int option = 1;
	if (gStandardGame) option += 1;
	if (gSwivel) option += 1 *2;
	if (gWrapAround) option += 1 *2*2;
	if (gFlatMode) {
		option += 2* 2*2*2;
		if (gExtraSlider) option += 1* 2*2*2;
	}
	else {
		if (gQueuePiece) option += 1* 2*2*2;
	}

	return option;
}

void setOption(int option)
{
	option--;
	gStandardGame = option%2==1;
	gSwivel = option/2%2==1;
	gWrapAround = option/(2*2)%2==1;
	gFlatMode = option/(2*2*2)%4>=2;
	if (gFlatMode) {
		gExtraSlider = option/(2*2*2)%4==3;
	}
	else {
		gQueuePiece = option/(2*2*2)%4==1;
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
