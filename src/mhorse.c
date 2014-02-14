/************************************************************************
**
** NAME:        horse.c
**
** DESCRIPTION: Horseshoe
**
** AUTHOR:      Dave Wong, Venu Kolavennu
**
** DATE:        2002-11-23
**
** UPDATE HIST:
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


POSITION gNumberOfPositions  = 39366;  /* 3^9*2  */

POSITION gInitialPosition    = 116;
POSITION kBadPosition        = -1; /* This can never be the rep. of a position */

STRING kAuthorName         = "Dave Wong and Venu Kolavennu";
STRING kGameName           = "Horseshoe";
STRING kDBName             = "horse";
BOOLEAN kPartizan           = TRUE;
BOOLEAN kSupportsHeuristic  = FALSE;
BOOLEAN kSupportsSymmetries = FALSE;
BOOLEAN kSupportsGraphics   = TRUE;
BOOLEAN kDebugMenu          = TRUE;
BOOLEAN kGameSpecificMenu   = TRUE;
BOOLEAN kTieIsPossible      = FALSE;
BOOLEAN kLoopy               = TRUE;
BOOLEAN kDebugDetermineValue = FALSE;
BOOLEAN kUseCustomBoard      = FALSE;
void*    gGameSpecificTclInit = NULL;


STRING kHelpGraphicInterface =
        "The LEFT button puts a small circle over your piece. This selects\n\
the FROM slot. The MIDDLE button then selects the TO slot. If you\n\
wish to remove a piece from the board, click the MIDDLE button on\n\
the same place as the FROM slot. The RIGHT button is the same as UNDO,\n\
in that it reverts back to your most recent position."                                                                                                                                                                                                                                                                                                  ;

STRING kHelpTextInterface    =
        "On your turn, use the LEGEND to determine which numbers to choose to\n\
correspond to the location of your piece and an empty connected-adjacent\n\
slot you wish to move that piece to. Example: '5 2' moves your piece from\n\
slot 5 to slot 2, assuming the two slots are connected."                                                                                                                                                                                                                                          ;

STRING kHelpOnYourTurn =
        "Note: The circle always goes first.  Move one of your pieces to an empty\n\
slot.  To do so, type in two numbers: the fromSlot and the toSlot, like\n\
so: 2 3 and then hit <RETURN>"                                                                                                                                                                ;

STRING kHelpStandardObjective =
        "To be the FIRST player to prevent your opponent from moving.";

STRING kHelpReverseObjective =
        "To be prevented from moving by your opponent's pieces.";

STRING kHelpTieOccursWhen =   /* Should follow 'A Tie occurs when... */
                            "";

STRING kHelpExample =
        "";


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

int BOARDSIZE = 5;                                      /* Default board */
int gVertices = 5;                                      /* Default number of vertices */
int POSITION_OFFSET = 19683;                    /* 3^9 */
#define MAXBOARDSIZE     9
#define OFFTHEBOARD      243         /* Removing that piece from the board */
#define BADSLOT         -2           /* You've moved off the board in a bad way */


int gCustomBoard[MAXBOARDSIZE][MAXBOARDSIZE];
int gDefaultBoard[5][5] =  {
	{2,3,-1,-1,-1},
	{2,4,-1,-1,-1},
	{0,1, 3, 4,-1},
	{0,2, 4,-1,-1},
	{1,2, 3,-1,-1}
};


typedef enum possibleBoardPieces {
	Blank, o, x
} BlankOX;

typedef int SLOT;     /* A slot is the place where a piece moves from or to */

char *gBlankOXString[] = { "+", "O", "X" };

/* Powers of 3 - this is the way I encode the position, as an integer */
int g3Array[] =          { 1, 3, 9, 27, 81, 243, 729, 2187, 6561 };

/* local function prototypes */
BOOLEAN CantMove(POSITION);
BOOLEAN OkMove(BlankOX*, BlankOX, SLOT, int);
void PositionToBlankOX(POSITION thePos, BlankOX *theBlankOX, BlankOX *whosTurn);
void MoveToSlots(MOVE theMove, SLOT *fromSlot, SLOT *toSlot);
MOVE SlotsToMove (SLOT fromSlot, SLOT toSlot);
SLOT GetToSlot(SLOT fromSlot,int neighbor,BlankOX whosturn);

STRING MoveToString(MOVE);

/************************************************************************
**
** NAME:        InitializeGame
**
** DESCRIPTION: Initialize the game
**
************************************************************************/

void InitializeGame()
{
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

void DebugMenu()
{
}

/************************************************************************
**
** NAME:        gCreateCustomBoard
**
** DESCRIPTION: Menu used to let user creat a custom board
**
************************************************************************/

void gCreateCustomBoard()
{
	int i, j, k;
	int c;

	for (i = 0; i < MAXBOARDSIZE; i++) {
		for (j = 0; j < MAXBOARDSIZE; j++) {
			gCustomBoard[i][j] = -1;
		}
	}

	printf("How many vertices? (4 - %d): ", MAXBOARDSIZE);
	scanf(" %d", &gVertices);
	if (gVertices < 4 || gVertices > MAXBOARDSIZE) {
		printf("Unacceptable.  Switching back to default game board.\n");
		kUseCustomBoard = FALSE;
	}

	BOARDSIZE = gVertices;

	printf("\nGreat.  The vertices will be numbered from 0 to %d.\n\n", gVertices - 1);
	printf("Next, I need you to tell me how the vertices are connected.\n");

	for (i = 0; i < gVertices; i++) /*vertices*/ {
		printf("\n");
		for (j = 0; j < gVertices; j++) /*neighbors*/ {
			printf("Vertex %d is connected to which vertex (-1 if no more): ",i);
			scanf(" %d", &c);
			if (c < 0) {
				j = gVertices;
			}
			else if (c == i) {
				printf("Sorry, that's the same vertex.\n");
				j--;
			}
			else if (c >= gVertices) {
				printf("That's too big.  Try again.\n");
				j--;
			}
			else {
				k=0;
				//gCustomBoard[i][j] = c;
				while (gCustomBoard[i][j] != -1) { j++; }
				gCustomBoard[i][j] = c;
				while (gCustomBoard[c][k] != -1) { k++; }
				gCustomBoard[c][k] = i;
			}
		}
	}

	for (i = 0; i < gVertices; i++) {
		printf("\n");
		for (j = 0; j < gVertices; j++) {
			printf("gCustomBoard[%d][%d] = %d\n", i, j, gCustomBoard[i][j]);
		}
	}

	printf("\n");

	printf("Custom board created.\n");
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
		printf("\n\t----- Game-specific options for %s -----\n\n", kGameName);

		printf("\tCurrent Initial Position:\n");
		PrintPosition(gInitialPosition, gPlayerName[kPlayerOneTurn], kHumansTurn);

		printf("\n\tI)\tChoose the (I)nitial position\n");
		printf("\tC)\tCreate a (C)ustom board\n");
		printf("\tD)\tUse (D)efault board\n");

		printf("\n\n\tB)\t(B)ack = Return to previous activity.\n");
		printf("\n\nSelect an option: ");

		switch(GetMyChar()) {
		case 'Q': case 'q':
			ExitStageRight();
		case 'H': case 'h':
			HelpMenus();
			break;
		case '1': case 'I': case 'i':
			gInitialPosition = GetInitialPosition();
			break;
		case 'c': case 'C':
			kUseCustomBoard = TRUE;
			gCreateCustomBoard();
			break;
		case 'd': case 'D':
			kUseCustomBoard = FALSE;
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
	BlankOX theBlankOX[BOARDSIZE], whosTurn;

	PositionToBlankOX(thePosition,theBlankOX,&whosTurn);
	MoveToSlots(theMove, &fromSlot, &toSlot);

	return(thePosition
	       + (whosTurn == o ? POSITION_OFFSET : -POSITION_OFFSET)
	       - (g3Array[fromSlot] * (int)whosTurn) /* take from slot */
	       + (g3Array[toSlot] * (int)whosTurn)); /* put in to slot */
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

POSITION GetInitialPosition() /* UNWRITTEN */
{
	POSITION BlankOXToPosition();
	BlankOX theBlankOX[BOARDSIZE], whosTurn;
	signed char c;
	int i;


	printf("\n\n\t----- Get Initial Position -----\n");
	printf("\n\tPlease input the position to begin with.\n");
	printf("\tNote that it should be in the following format:\n\n");
	for (i = 0; i < gVertices; i++) {
		printf("- ");
	}
	printf("        <----- EXAMPLE \n- x x - o ..etc\n\n");

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
** INPUTS:      MOVE    computersMove : The computer's move.
**              STRING  computersName : The computer's name.
**
************************************************************************/

void PrintComputersMove(computersMove,computersName)
MOVE computersMove;
STRING computersName;
{
	SLOT fromSlot,toSlot;
	MoveToSlots(computersMove,&fromSlot,&toSlot);
	printf("%8s's move              : %d %d\n", computersName,
	       fromSlot,toSlot);
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
** CALLS:
**              PositionToBlankOX()
**              BlankOX OnlyPlayerLeft(*BlankOX)
**
************************************************************************/

VALUE Primitive(position)
POSITION position;
{

	BlankOX theBlankOX[BOARDSIZE],whosTurn;

	PositionToBlankOX(position,theBlankOX,&whosTurn);


	if(CantMove(position)) /* the other player just won */
		return(gStandardGame ? lose : win);
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

/*

   +---+---+     0       2
 |\ /|\ /|     |\     /|
 | x | x |     | \   / |
 ||/ \|/ \|     |  \ /  |
 |+---+---+     |   4   |
 |\ /|\ /|     |  / \  |
 | x | x |     | /   \ |
 ||/ \|/ \|     |/     \|
 |+---+---+     6-------9

 +
   /|\
 ++- -+
 +|\|/|
 | + |
 ||/|\|
 |+- -+
 ||\|/|
 | + |
 ||/ \|
 |+---+

   0-1
 \
    4

   1-0
 |\
   4 3
 */
void PrintPosition(position,playerName,usersTurn)


POSITION position;
STRING playerName;
BOOLEAN usersTurn;

{

	BlankOX theBlankOx[BOARDSIZE], whosTurn;

	PositionToBlankOX(position,theBlankOx,&whosTurn);

	printf("     0           1      %s           %s\n",
	       gBlankOXString[(int)theBlankOx[0]],
	       gBlankOXString[(int)theBlankOx[1]]);
	printf("     | -       - |      | -       - |\n");

	printf("     |   -   -   |      |   -   -   |\n");

	printf("     |     2     |      |     %s     |\n",
	       gBlankOXString[(int)theBlankOx[2]]);
	printf("     |   -   -   |      |   -   -   |\n");
	printf("     | -       - |      | -       - |\n");
	printf("     3 - - - - - 4      %s - - - - - %s\n\n",
	       gBlankOXString[(int)theBlankOx[3]],
	       gBlankOXString[(int)theBlankOx[4]]);


	/*
	   for (i = 0; i < gVertices; i++) {
	        printf("%d\t%s\n", i, gBlankOXString[(int)theBlankOx[i]]);
	   }
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
** CALLS:       GENERIC_PTR SafeMalloc(int)
**
************************************************************************/


MOVELIST *GenerateMoves(position)
POSITION position;
{
	MOVELIST *head = NULL;
	MOVELIST *CreateMovelistNode();
	BlankOX theBlankOX[BOARDSIZE], whosTurn;
	int i,j; /* Values for J: 0=left,1=straight,2=right */

	//  if (kDebugMenu) printf("GenerateMoves called.\n");

	PositionToBlankOX(position,theBlankOX,&whosTurn);

	for(i = 0; i < BOARDSIZE; i++) { /* enumerate over all FROM slots */
		for(j = 0; j < gVertices; j++) { /* enumerate over all directions */
			if(OkMove(theBlankOX,whosTurn,(SLOT)i,j))
				head = CreateMovelistNode(SlotsToMove((SLOT)i,GetToSlot(i,j,whosTurn)),head);
		}
	}
	return(head);
}

BOOLEAN OkMove(theBlankOX,whosTurn,fromSlot,neighbor)
BlankOX *theBlankOX, whosTurn;
SLOT fromSlot;
int neighbor;
{
	SLOT toSlot;
	//  if (kDebugMenu) printf("OkMove called.\n");
	toSlot = GetToSlot(fromSlot,neighbor,whosTurn);
	return((theBlankOX[fromSlot] == whosTurn) &&
	       (toSlot != BADSLOT) &&
	       (theBlankOX[toSlot] == Blank));
}

SLOT GetToSlot(fromSlot,neighbor,whosturn)
SLOT fromSlot;
int neighbor;
BlankOX whosturn;
{
	SLOT toSlot;
	if (!kUseCustomBoard) {
		toSlot = gDefaultBoard[fromSlot][neighbor];
	}
	else {
		toSlot = gCustomBoard[fromSlot][neighbor];
	}

	if (toSlot < 0) {
		toSlot = BADSLOT;
	}
	return toSlot;

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
	USERINPUT ret, HandleDefaultTextInput();
	BlankOX theBlankOX[BOARDSIZE], whosTurn;
	PositionToBlankOX(thePosition,theBlankOX,&whosTurn);

	do {
		printf("%8s's move [(u)ndo/ Num1 Num2] : ", playerName);

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
	SLOT fromSlot, toSlot;
	int ret;
	ret = sscanf(input,"%d %d", &fromSlot, &toSlot);
	return(ret == 2 &&
	       fromSlot < BOARDSIZE && fromSlot >= 0 && toSlot < BOARDSIZE && toSlot >= 0);
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
	ret = sscanf(input,"%d %d", &fromSlot, &toSlot);


	return(SlotsToMove(fromSlot,toSlot));
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
	STRING move = (STRING) SafeMalloc(11);

	SLOT fromSlot, toSlot;
	MoveToSlots(theMove,&fromSlot,&toSlot);

	sprintf(move, "[ %d %d ] ", fromSlot, toSlot);
	return move;
}


int NumberOfOptions() {
	return 2;
}


int getOption() {
	int option = 1;
	option += (gStandardGame ? 0 : 1);
	return option;
}


void setOption(int option) {
	option--;

	gStandardGame = (option%2==0);
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

void PositionToBlankOX(thePos,theBlankOX,whosTurn)
POSITION thePos;
BlankOX *theBlankOX, *whosTurn;
{
	int i;

	if (thePos >= POSITION_OFFSET) { /* X moves first <==> pos >= offset */
		*whosTurn = x;
		thePos -= POSITION_OFFSET;
	}
	else
		*whosTurn = o;

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
		else
			BadElse("PositionToBlankOX");
	}
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
	return ((MOVE) toSlot*(BOARDSIZE+1) + fromSlot);
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

POSITION BlankOXToPosition(theBlankOX,whosTurn)
BlankOX *theBlankOX,whosTurn;
{
	int i;
	POSITION position = 0;

	for(i = 0; i < BOARDSIZE; i++)
		position += g3Array[i] * (int)theBlankOX[i];

	if(whosTurn == x)
		position += POSITION_OFFSET; /* account for whose turn it is */

	return(position);
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
