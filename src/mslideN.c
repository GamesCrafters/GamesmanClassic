

/************************************************************************
**
** NAME:        mslideN.c
**
** DESCRIPTION: Slide-N
**
** AUTHORS:     Rach Liu, Bryon Ross, Jiong Shen & Tom Wang
**
** DATE:        2003-03-12
**
** UPDATE HIST:
** 2003-02-13 v0.9 :  Solves Slide-1 through Slide-3
** 2003-02-21 v1.0 :  Solves Slide-1 through Slide-4
** 2003-02-22 v1.1 :  X goes first & PrintPosition displays whoseTurn
** 2003-03-05 v1.2 :  Help example added
** 2003-03-08 v1.3 :  Primitive updated to handle vertical and
**                    horizontal N-in-a-row
**                    Variation added to Game Specific Options
** 2003-03-12 v1.31:  Fixed a bug in PositionToBlankOX
** 2003-05-09 v1.32:  Modified game menu
**
**************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/

#include <stdio.h>
#include "gamesman.h"

POSITION gNumberOfPositions = 0;  /* variable size loopy game */

POSITION gInitialPosition    =  0; /* hashed value */
POSITION gMinimalPosition    =  0;

STRING kAuthorName         = "Rach Liu, Bryon Ross, Jiong Shen and Tom Wang";
STRING kGameName           = "SlideN";
BOOLEAN kPartizan           = TRUE;
BOOLEAN kDebugMenu          = FALSE;
BOOLEAN kGameSpecificMenu   = TRUE;
BOOLEAN kTieIsPossible      = FALSE;
BOOLEAN kLoopy               = TRUE;
BOOLEAN kDebugDetermineValue = FALSE;
POSITION kBadPosition           = -1;
void*    gGameSpecificTclInit = NULL;

STRING kHelpGraphicInterface =
        "There is no graphical interface.";

STRING kHelpTextInterface    =
        "On your turn, insert a piece from one of the insertion points along\n\
the top of the board.  Every board has 2*N insertion points labeled\n\
1 through 2*N (where 1 is the left-most entry point and 2*N is the\n\
right-most entry point). If the space into which you are inserting\n\
your piece is occupied, the existing piece will be pushed in the direction\n\
from which your new piece was inserted. If the space into which the\n\
pushed piece is being moved is occupied, the pushed piece will\n\
push that piece in the same direction as the original push.\n\
This process will continue until either a piece is pushed into\n\
an empty space, or a piece is pushed off the bottom of the board.\n\
Pieces pushed off the bottom of the board are discarded."                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                ;

STRING kHelpOnYourTurn =
        "You insert one of your pieces from one of the insertion points along \n\
the top of the board."                                                                                  ;

STRING kHelpStandardObjective =
        "To get N of your pieces in a row (where N is the length of one side \n\
of the board). N-in-a-row can be made vertically, horizontally, or \n\
diagonally. N-in-a-row WINS."                                                                                                                                                        ;

STRING kHelpReverseObjective =
        "To force your opponent into getting N of his pieces in a row \n"
        "(where N is the length of one side of the board). \n"
        "N-in-a-row can be made vertically, horizontally, or diagonally. \n"
        "N-in-a-row LOSES.";

STRING kHelpTieOccursWhen =
        "both players get N in a row at the same time";

STRING kHelpExample =
        "========= How moves and pieces work, how to input them ==============  \n\n\
    3 4               ** This is the initial board set up.             \n\
  2  -  5                The numbers along the top indicate where      \n\
1  -   -  6              pieces can be inserted.                       \n\
 -   -   -               Pieces can only be inserted at the spots      \n\
   -   -                 directly adjacent to the numbers.             \n\
     -   \n\
X's Turn        (Dan should Tie in 0)      Dan's move [(u)ndo/1-6] : 3 \n\n\
** the player inputs 3, the third position ** \n\n\n\
    3 4               ** The player moved, and so the player's piece  \n\
  2  X  5                appears at the spot marked by the number 3.  \n\
1  -   -  6              Now the opponent moves...   \n\
 -   -   -               \n\
   -   -                 \n\
     -                   \n\
O's Turn        (Computer should Tie in 6)      Computer's move [(u)ndo/1-6] : 3 \n\n\
** the computer inputs 3, the fifth position **  \n\
\n\
    3 4               ** The opponent placed move at position 3 also.  \n\
  2  O  5                The piece occupying position 3 is now pushed  \n\
1  -   X  6              along the direction of position 3 -- in this  \n\
 -   -   -               case, it's pushed diagonally parallel to the  \n\
   -   -                 line along position 3. \n\
     -   \n\
X's Turn        (Dan should Tie in 6)      Dan's move [(u)ndo/1-6] :  5 \n\n\
** the player inputs 5, the fifth position ** \n\n\n\
    3 4               ** Since the player chose position 5, an X is   \n\
  2  O  5                inserted at position 5, pushing the previous \n\
1  -   X  6              piece occupying that position (an X, in this \n\
 -   X   -               case) diagonally parallel to the line formed \n\
   -   -                 along position 5.  \n\
     -   \n\
O's Turn        (Computer will Win in 5) Computer's move :  5   \n\n\
** Computer inputs 5, the fifth position **   \n\n\
=========================== Winning Situations =======================  \n\n\
    3 4   \n\
  2  O  5        ** X pieces are lined up diagonally along position 5.  \n\
1  -   X  6      \n\
 -   X   -       \n\
   X   O         \n\
     _           \n\n\n\
    3 4          \n\
  2  X  5        ** O pieces are lined up diagonally along position 1. \n\
1  X   X  6      \n\
 O   O   -       \n\
   O   X         \n\
     O           \n\n\n\
    3 4          \n\
  2  X  5        ** X pieces are lined up vertically.  \n\
1  O   _  6      \n\
 _   X   -       \n\
   _   _         \n\
     X           \n\n\n\
    3 4          \n\
  2  X  5        ** O pieces are lined up horizontally. \n\
1  X   X  6      \n\
 O   O   O       \n\
   X   X         \n\
     _           \n\n"                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 ;

/**
 * Position numbers:
 *     1
 *   4   2
 * 7   5   3
 *   8   6
 *     9
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

int DefaultN = 3;
static int N;
static int BoardSize;

typedef enum possibleBoardPieces {
	Blank, x, o
} BlankOX;

char *gBlankOXString[] = { "-", "X", "O" };
BOOLEAN gTieLoses = TRUE;
BOOLEAN gDiagonalsWin = TRUE;
BOOLEAN gDoubleGravity = FALSE;
BOOLEAN gDiagonalLeftGravity = FALSE;
BOOLEAN gDiagonalRightGravity = FALSE;
BOOLEAN firstPass = TRUE;

static int* g3Array;

STRING MoveToString( MOVE );

void InitializeGame()
{
	void InitializeGameVariables();
	if(firstPass) {
		InitializeGameVariables(DefaultN);
		firstPass = FALSE;
	}

	gMoveToStringFunPtr = &MoveToString;
}

void FreeGame()
{
}


/************************************************************************
**
** NAME:        InitializeGameVariables
**
** DESCRIPTION: Initializes game-specific variables
**
** INPUT:       int length: the length of a side of the board
**
************************************************************************/

void InitializeGameVariables(int length) {
	int i;

	N = length;
	BoardSize = N*N;
	gNumberOfPositions = 1;
	for (i = 0; i < BoardSize; i++) {
		gNumberOfPositions *= 3;
	}
	gNumberOfPositions *= 2;

	//printf("positions: %lu\n",(long) gNumberOfPositions);
	fflush(NULL);

	// calculate g3Array here
	if (g3Array != NULL) SafeFree(g3Array);
	g3Array = (int*) SafeMalloc ((BoardSize+1) * sizeof(int));
	g3Array[0] = 1;
	for (i = 1; i <= BoardSize; i++) {
		g3Array[i] = 3 * g3Array[i-1];
	}
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
	char GetMyChar();
	POSITION GetInitialPosition();
	void SetInitialBoardSize();
	VALUE Primitive();

	do {
		printf("\n\t----- Game-specific options for %s -----\n\n", kGameName);

		printf("\ts)\tSet the initial board (s)ize\n");
		printf("\ti)\tChoose the (i)nitial position\n");
		printf("\tt)\tToggle (t)ieing from %s to %s\n",
		       gTieLoses ? "BAD (LOSING)" : "GOOD (WINNING)",
		       gTieLoses ? "GOOD (WINNING)" : "BAD (LOSING)");
		printf("\td)\tToggle diagonal victory (horizontal or vertical rows) from %s to %s\n",
		       gDiagonalsWin ? "ON" : "OFF", gDiagonalsWin ? "OFF" : "ON");
		printf("\tp)\tDetermine if the initial position is (P)rimitive\n");
		printf("\tl)\tToggle diagonal down-left gravity from %s to %s\n",
		       gDiagonalLeftGravity ? "ON" : "OFF", gDiagonalLeftGravity ? "OFF" : "ON");
		printf("\tr)\tToggle diagonal down-right gravity from %s to %s \n",
		       gDiagonalRightGravity ? "ON" : "OFF", gDiagonalRightGravity ? "OFF" : "ON");
		printf("\n\tb)\t(B)ack = Return to previous activity.\n");
		printf("\nSelect an option: ");

		switch(GetMyChar()) {
		case 'Q': case 'q':
			ExitStageRight();
		case 'H': case 'h':
			HelpMenus();
			break;
		case 'S': case 's':
			SetInitialBoardSize();
			break;
		case 'I': case 'i':
			gInitialPosition = GetInitialPosition();
			break;
		case 'T': case 't':
			gTieLoses = !gTieLoses;
			break;
		case 'D': case 'd':
			gDiagonalsWin = !gDiagonalsWin;
			break;
		case 'P': case 'p':
			if (gNumberOfPositions == 0) {
				printf("Set the initial position before determining primitive values.");
				break;
			}
			PrintPosition(gInitialPosition, "Nobody", TRUE);
			printf("Primitive: %d (0=WIN, 1=LOSE, 3=UNDECIDED)\n",Primitive(gInitialPosition));
			break;
		case 'L': case 'l':
			gDiagonalLeftGravity = !gDiagonalLeftGravity;
			break;
		case 'R': case 'r':
			gDiagonalRightGravity = !gDiagonalRightGravity;
			break;
		case 'b': case 'B':
			if (gDiagonalLeftGravity && gDiagonalRightGravity)
				gDoubleGravity = TRUE;
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
	int i;
	int myMove;

	void LeftGravity(), RightGravity();
	POSITION BlankOXToPosition();
	BlankOX PositionToBlankOX();
	BlankOX theBlankOX[BoardSize];
	BlankOX WhoseTurn, temp, temp2;

	WhoseTurn = PositionToBlankOX(thePosition,theBlankOX);
	temp2 = WhoseTurn;

	// Original code for no gravity
	if (theMove > N) {
		myMove = theMove-N-1;

		for (i = theMove-N-1; i < N*N; i += N) {
			if (theBlankOX[i] != Blank) {
				temp = theBlankOX[i];
				theBlankOX[i] = temp2;
				temp2 = temp;
			} else {
				theBlankOX[i] = temp2;
				break;
			}
		}
	}
	else {
		myMove = N * (N - theMove);

		for (i = N * (N - theMove); i < N * (N - theMove + 1); i++) {
			if (theBlankOX[i] != Blank) {
				temp = theBlankOX[i];
				theBlankOX[i] = temp2;
				temp2 = temp;
			} else {
				theBlankOX[i] = temp2;
				break;
			}
		}
	}

	if (gDoubleGravity) {
		if (theMove > N) {
			LeftGravity(&theBlankOX, &i);
			RightGravity(&theBlankOX, &i);
		}
		else {
			RightGravity(&theBlankOX, &i);
			LeftGravity(&theBlankOX, &i);
		}
	}
	else if (gDiagonalLeftGravity) {
		LeftGravity(&theBlankOX, &i);
	}
	else if (gDiagonalRightGravity) {
		RightGravity(&theBlankOX, &i);
	}

	WhoseTurn = (WhoseTurn==x) ? o : x;

	thePosition = BlankOXToPosition(theBlankOX, WhoseTurn);

	return(thePosition);
}

void Gravity(BlankOX *theBlankOX, int *pieceToGravitate, int bound, int increment) {
	int i;
	BlankOX temp;

	for (i = *pieceToGravitate + increment; i < bound; i += increment) {
		if (theBlankOX[i] != Blank) {
			break;
		}
	}

	temp = theBlankOX[*pieceToGravitate];

	theBlankOX[*pieceToGravitate] = Blank;
	theBlankOX[i - increment] = temp;

	*pieceToGravitate = i - increment;
}

void LeftGravity(BlankOX *theBlankOX, int *pieceToGravitate)
{
	Gravity(theBlankOX, pieceToGravitate, N * N, N);
}

void RightGravity(BlankOX *theBlankOX, int *pieceToGravitate)
{
	Gravity(theBlankOX, pieceToGravitate, (*pieceToGravitate / N + 1) * N, 1);
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

void SetInitialBoardSize()
{
	printf("\n\n\t----- Set Initial Board Size -----\n");
	printf("Size of the board? (1-1X1, 2-2X2, 3-3X3, 4-4X4...): \n");
	scanf("%d", &N);
	InitializeGameVariables(N);
	gInitialPosition = 0;
}

POSITION GetInitialPosition()
{
	POSITION BlankOXToPosition();
	BlankOX theBlankOX[BoardSize], whosTurn;
	char c;
	int i;

	if (gNumberOfPositions == 0) {
		printf("\n\n**Set the initial board size before selecting an initial position.**");
		return 0;
	}

	printf("\n\n\t----- Get Initial Position -----\n");
	printf("\n\tPlease input the position to begin with.\n");
	printf("\tNote that it should be in the following format:\n\n");
	printf("O - -\nO - -            <----- EXAMPLE \n- X X\n\n");

	i = 0;

	while(i < BoardSize && (c = getchar()) != EOF) {
		if(c == 'x' || c == 'X')
			theBlankOX[i++] = x;
		else if(c == 'o' || c == 'O' || c == '0')
			theBlankOX[i++] = o;
		else if(c == '-')
			theBlankOX[i++] = Blank;
	}

	getchar();
	printf("\nNow, whose turn is it? [O/X] : ");
	scanf("%c",&c);
	if(c == 'x' || c == 'X')
		whosTurn = x;
	else
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

void PrintComputersMove(computersMove,computersName)
MOVE computersMove;
STRING computersName;
{
	printf("%8s's move : %2d\n", computersName, computersMove);
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
	BlankOX NinARow(), PositionToBlankOX();
	BlankOX theBlankOX[BoardSize];
	BlankOX temp = Blank;
	int i, val;
	BlankOX whoseturn;

	whoseturn = PositionToBlankOX(position,theBlankOX);

	for (i = 1; i <= 2*N; i++) {
		temp |= NinARow(theBlankOX, i);
	}

	if (gDiagonalsWin) {
		//vertical check
		val = Blank;
		for (i = 1; i <= N; i++) {
			if (theBlankOX[i*(N-1)] == Blank) {
				val = Blank;
				break;
			} else {
				if (val == Blank) {
					val = theBlankOX[i*(N-1)];
				}
				else if (val != theBlankOX[i*(N-1)]) {
					val = Blank;
					break;
				}
			}
		}
		temp |= val;

		//horizontal check
		val = Blank;
		for (i = 0; i < N; i++) {
			if (theBlankOX[i*(N+1)] == Blank) {
				val = Blank;
				break;
			} else {
				if (val == Blank) {
					val = theBlankOX[i*(N+1)];
				}
				else if (val != theBlankOX[i*(N+1)]) {
					val = Blank;
					break;
				}
			}
		}
		temp |= val;
	}

	if (temp == (o|x))
		return(gTieLoses ? win : lose);
	else if (temp == whoseturn)
		return(gStandardGame ? win : lose);
	else if (temp != Blank)
		return(gStandardGame ? lose : win);
	else
		return(undecided);
}

/**********************************************************************
**
** NAME:        NinARow
**
** DESCRIPTION: Return x if X has N pieces in a row.
**              Return o if O has N pieces in a row.
**              Return (o|x) if both O and X have N pieces in a row.
**
** INPUTS:      BlankOX *theBlankOX: the current game board
**              int i: the row being examined
**
** OUTPUTS:     BlankOX Blank, x, o, or (x|o)
**
***********************************************************************/

BlankOX NinARow(theBlankOX, i)
BlankOX *theBlankOX;
int i;
{
	BlankOX val = Blank;
	int j;

	if (i > N) {
		for (j = i-N-1; j < N*N; j += N) {
			if (theBlankOX[j] == Blank) {
				return Blank;
			} else {
				if (val == Blank) {
					val = theBlankOX[j];
				}
				else if (val != theBlankOX[j])
					return Blank;
			}
		}
		return val;
	} else {
		for (j = (N-i)*N; j < (N-i+1)*N; j++) {
			if (theBlankOX[j] == Blank) {
				return Blank;
			} else {
				if (val == Blank) {
					val = theBlankOX[j];
				} else if (val != theBlankOX[j])
					return Blank;
			}
		}
		return val;
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
** CALLS:       PositionToBlankOX()
**              GetValueOfPosition()
**              GetPrediction()
**
************************************************************************/

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn) {
	BlankOX PositionToBlankOX();
	int i,j;
	BlankOX theBlankOX[BoardSize];
	BlankOX whoseTurn;

	whoseTurn = PositionToBlankOX(position,theBlankOX);
	printf("\n");
	for (i=0; i<N-1; i++) {
		printf("  ");
	}
	printf("%d %d\n",N,N+1);
	for (i=0; i<N-1; i++) {
		for (j=0; j<N-i-2; j++) {
			printf("  ");
		}
		printf("%d  ", N-i-1);
		for (j=0; j<=i; j++) {
			printf("%s  ",gBlankOXString[theBlankOX[N*(i-j)+j]]);
			if (j!=i) { printf(" "); }
		}
		printf("%d\n", N+i+2);
	}

	for (i=0; i<N; i++) {
		printf(" ");
		for (j=0; j<i; j++) {
			printf("  ");
		}
		for (j=0; j<N-i; j++) {
			printf("%s   ",gBlankOXString[theBlankOX[N*(N-1)+i-j*(N-1)]]);
		}
		printf("\n");
	}
	printf("%s's Turn\t%s",gBlankOXString[whoseTurn],GetPrediction(position,playerName,usersTurn));
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
	BlankOX PositionToBlankOX();
	MOVELIST *CreateMovelistNode(), *head = NULL;
	BlankOX theBlankOX[BoardSize];
	int i;

	PositionToBlankOX(position,theBlankOX);

	//moves 1 to 2N is valid at all time
	for(i = 1; i <= N*2; i++) {
		head = CreateMovelistNode(i,head);
	}
	return(head);
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
		printf("%8s's move [(u)ndo/1-%d] :  ", playerName, 2*N);

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
	return(atoi(input)<=(2*N) && atoi(input)>0);
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
	return((MOVE) atoi(input)); /* start with 1 */
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
	sprintf(m, "%d", theMove);
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

BlankOX PositionToBlankOX(thePos,theBlankOX)
POSITION thePos;
BlankOX *theBlankOX;
{
	int i;
	BlankOX whosturn;

	whosturn = (thePos / g3Array[BoardSize]) + 1;
	thePos = thePos % g3Array[BoardSize];

	for(i = BoardSize-1; i >= 0; i--) {
		if(thePos >= ((int)o * g3Array[i])) {
			theBlankOX[i] = o;
			thePos -= (int)o * g3Array[i];
		}
		else if(thePos >= ((int)x * g3Array[i])) {
			theBlankOX[i] = x;
			thePos -= (int)x * g3Array[i];
		}
		else if(thePos >= ((int)Blank * g3Array[i])) {
			theBlankOX[i] = Blank;
			thePos -= (int)Blank * g3Array[i];
		}
		else
			BadElse("PositionToBlankOX");
	}
	return whosturn;
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

POSITION BlankOXToPosition(theBlankOX, whosturn)
BlankOX *theBlankOX, whosturn;
{
	int i;
	POSITION position = 0;

	for(i = 0; i < BoardSize; i++)
		position += g3Array[i] * (int)theBlankOX[i];

	position += g3Array[BoardSize] * (whosturn-1);

	return(position);
}

STRING kDBName = "slide-N";

int NumberOfOptions()
{
	return 2*2*2*2*2;
}

int getOption()
{
	int option = 1;
	if(gStandardGame) option += 1;
	if(gTieLoses) option += 1 *2;
	if(gDiagonalsWin) option += 1 *2*2;
	if(gDiagonalLeftGravity) option += 1 *2*2*2;
	if(gDiagonalRightGravity) option += 1 *2*2*2*2;
	return option;
}

void setOption(int option)
{
	option--;
	gStandardGame = option%2==1;
	gTieLoses = option/2%2==1;
	gDiagonalsWin = option/(2*2)%2==1;
	gDiagonalLeftGravity = option/(2*2*2)%2==1;
	gDiagonalRightGravity = option/(2*2*2*2)%2==1;
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
