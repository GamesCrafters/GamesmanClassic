/************************************************************************
**
** NAME:        moandc.c
**
** DESCRIPTION:  Order and chaos

** AUTHOR:      Dan Garcia and Samhith Kakarla
**
** DATE:        2023-08-21
**
************************************************************************/

#include "gamesman.h"

POSITION gNumberOfPositions  = 19683;  /* 3^9 */
POSITION kBadPosition        = -1;

POSITION gInitialPosition    =  0;
POSITION gMinimalPosition    =  0;

CONST_STRING kAuthorName         = "Dan Garcia and Samhith Kakarla";
CONST_STRING kGameName           = "Order and Chaos";
CONST_STRING kDBName = "oandc";
BOOLEAN kPartizan           = TRUE;
BOOLEAN kDebugMenu          = TRUE;
BOOLEAN kGameSpecificMenu   = FALSE;
BOOLEAN kTieIsPossible      = TRUE;
BOOLEAN kLoopy               = FALSE;
BOOLEAN kDebugDetermineValue = FALSE;
void*    gGameSpecificTclInit = NULL;

CONST_STRING kHelpGraphicInterface =""                                                                                                                                                                                                                                   ;
CONST_STRING kHelpTextInterface = "On your turn choose the position you want to place your piece in and wether u want it to be an x or an o by denoting 1 or 2 after the position number"                                                                                                                                                                                                                                            ;
CONST_STRING kHelpOnYourTurn = "You place a piece on one of the empty board positions.";
CONST_STRING kHelpStandardObjective = "Get 3 in a row of any peice";                                                                        
CONST_STRING kHelpReverseObjective = "Fill up the board without letting 3 of any peice end up in a row"                                                                                                                                                             ;
CONST_STRING kHelpTieOccursWhen =  "a tie never occurs";
CONST_STRING kHelpExample ="";                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          ;

// game spesific variables
#define BOARDSIZE     9           /* 3x3 board */
#define BOARDROWS     3
#define BOARDCOLS     3

typedef enum possibleBoardPieces {
	Blank, o, x
} BlankOX;

int moveCount = 0;

char *gBlankOXString[] = { " ", "o", "x" };

/* Powers of 3 - this is the way I encode the position, as an integer */
int g3Array[] = { 1, 3, 9, 27, 81, 243, 729, 2187, 6561 };

/* Global position solver variables.*/
struct {
	BlankOX board[BOARDSIZE];
	int piecesPlaced;
} gPosition;

/** Function Prototypes **/
BOOLEAN AllFilledIn(BlankOX theBlankOX[]);
POSITION BlankOXToPosition(BlankOX *theBlankOX);
POSITION GetCanonicalPosition(POSITION position);
void PositionToBlankOX(POSITION thePos,BlankOX *theBlankOX);
BOOLEAN ThreeInARow(BlankOX[], int, int, int);
void UndoMove(MOVE move);
BlankOX WhoseTurn(BlankOX *theBlankOX);

POSITION ActualNumberOfPositions(int variant);

BOOLEAN kSupportsSymmetries = TRUE; /* Whether we support symmetries */

#define NUMSYMMETRIES 8   /*  4 rotations, 4 flipped rotations */

int gSymmetryMatrix[NUMSYMMETRIES][BOARDSIZE];

/* This is the array used for flipping along the N-S axis */
int gFlipNewPosition[] = { 2, 1, 0, 5, 4, 3, 8, 7, 6 };

/* This is the array used for rotating 90 degrees clockwise */
int gRotate90CWNewPosition[] = { 6, 3, 0, 7, 4, 1, 8, 5, 2 };

/* Initialize any global variables or data structures needed before
solving or playing the game. */
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


	PositionToBlankOX(gInitialPosition, gPosition.board);
	gPosition.piecesPlaced = 0;
	gUndoMove = UndoMove;
	gActualNumberOfPositionsOptFunPtr = &ActualNumberOfPositions;
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

void GameSpecificMenu() {
}

// Anoto pen support - implemented in core/pen/pttt.c
// extern void gPenHandleTclMessage(int options[], char *filename, Tcl_Interp *tclInterp, int debug);

/************************************************************************
**
** NAME:        SetTclCGameSpecificOptions
**
** DESCRIPTION: Set the C game-specific options (called from Tcl)
**              Ignore if you don't care about Tcl for now.
**
************************************************************************/

#ifndef NO_GRAPHICS
void SetTclCGameSpecificOptions(int theOptions[])
{
	// // Anoto pen support
	// if ((gPenFile != NULL) && (gTclInterp != NULL)) {
	// 	gPenHandleTclMessage(theOptions, gPenFile, gTclInterp, gPenDebug);
	// }
}
#endif

/************************************************************************
**
** NAME:        DoMove
**
** DESCRIPTION: Apply the move to the position.
**
** INPUTS:      POSITION position : The old position
**              MOVE     move     : The move to apply.
**
** OUTPUTS:     (POSITION) : The position that results after the move.
**
** CALLS:       PositionToBlankOX(POSITION,*BlankOX)
**              BlankOX WhosTurn(*BlankOX)
**
************************************************************************/

POSITION DoMove(POSITION position, MOVE move)
{
  int pos = move/10;
  int mov = move % 10;

  if (gUseGPS) {
    if (mov == 1){
      gPosition.board[pos] = x;
    }else if(mov == 2){
      gPosition.board[pos] = o;
    }
		++gPosition.piecesPlaced;

		return BlankOXToPosition(gPosition.board);
	}
	else {
		BlankOX board[BOARDSIZE];

		PositionToBlankOX(position, board);

    if (mov == 1){
      return position + g3Array[pos] * (int) x;
    }else{
      return position + g3Array[pos] * (int) o;
    }

	}

  moveCount += 1;

}

void UndoMove(MOVE move)
{
	gPosition.board[move/10] = Blank;
	--gPosition.piecesPlaced;
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

	/*
	   getchar();
	   printf("\nNow, whose turn is it? [O/X] : ");
	   scanf("%c",&c);
	   if(c == 'x' || c == 'X')
	   whosTurn = x;
	   else
	   whosTurn = o;
	 */

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

void PrintComputersMove(MOVE computersMove, STRING computersName) {
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

VALUE Primitive(POSITION position) {
	if (!gUseGPS)
		PositionToBlankOX(position, gPosition.board); // Temporary storage.

	if (ThreeInARow(gPosition.board, 0, 1, 2) ||
	    ThreeInARow(gPosition.board, 3, 4, 5) ||
	    ThreeInARow(gPosition.board, 6, 7, 8) ||
	    ThreeInARow(gPosition.board, 0, 3, 6) ||
	    ThreeInARow(gPosition.board, 1, 4, 7) ||
	    ThreeInARow(gPosition.board, 2, 5, 8) ||
	    ThreeInARow(gPosition.board, 0, 4, 8) ||
	    ThreeInARow(gPosition.board, 2, 4, 6)){
        if (WhoseTurn(gPosition.board) == x){
          return win;
        }else{
          return lose;
        }
        // return win;

      }
	else if ((gUseGPS && (gPosition.piecesPlaced == BOARDSIZE)) ||
	         ((!gUseGPS) && AllFilledIn(gPosition.board))){
            printf("im actually here");
		 if (WhoseTurn(gPosition.board) == x){
          return lose;
        }else{
          return win;
        }
    // return lose;
           }
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
** CALLS:       PositionToBlankOX()
**              GetValueOfPosition()
**              GetPrediction()
**
************************************************************************/

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn) {
	BlankOX theBlankOx[BOARDSIZE];

	PositionToBlankOX(position,theBlankOx);

	printf("\n         ( 1 2 3 )           : %s %s %s\n",
	       gBlankOXString[(int)theBlankOx[0]],
	       gBlankOXString[(int)theBlankOx[1]],
	       gBlankOXString[(int)theBlankOx[2]] );
	printf("LEGEND:  ( 4 5 6 )  TOTAL:   : %s %s %s\n",
	       gBlankOXString[(int)theBlankOx[3]],
	       gBlankOXString[(int)theBlankOx[4]],
	       gBlankOXString[(int)theBlankOx[5]] );
	printf("         ( 7 8 9 )           : %s %s %s %s\n\n",
	       gBlankOXString[(int)theBlankOx[6]],
	       gBlankOXString[(int)theBlankOx[7]],
	       gBlankOXString[(int)theBlankOx[8]],
	       GetPrediction(position,playerName,usersTurn));
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

MOVELIST *GenerateMoves(POSITION position)
{
	int index;
	MOVELIST *moves = NULL;

	if (!gUseGPS)
		PositionToBlankOX(position, gPosition.board); // Temporary storage.

	for (index = 0; index < BOARDSIZE; ++index)
		if (gPosition.board[index] == Blank){
			moves = CreateMovelistNode(index*10 + 1, moves);
      		moves = CreateMovelistNode(index*10 + 2, moves);
		}
	return moves;
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

	PositionToBlankOX(position,theBlankOx);
	PositionToBlankOX(position,symmBlankOx); /* Make copy */

	/* Copy from the symmetry matrix */

	for(i = 0; i < BOARDSIZE; i++)
		symmBlankOx[i] = theBlankOx[gSymmetryMatrix[symmetry][i]];

	return(BlankOXToPosition(symmBlankOx));
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

		newPosition = DoSymmetry(position, i); /* get new */
		if(newPosition < theCanonicalPosition) /* THIS is the one */
			theCanonicalPosition = newPosition; /* set it to the ans */
	}

	return(theCanonicalPosition);
}

/**************************************************/
/**************** SYMMETRY FUN END ****************/
/**************************************************/

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
		printf("%8s's move [(u)ndo/11-92] :  ", playerName);

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
	// printf(input[0]);
	// printf((int)input[0] <= 92 && (int)input[0] >= 1);
	// return((int)input[0] <= 92 && (int)input[0] >= 1);
	(void) input;
	return TRUE;
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
	
	return((MOVE) atoi(input) - 10); 
}

/************************************************************************
**
** NAME:        MoveToString
**
** DESCRIPTION: Returns the move as a STRING
**
** INPUTS:      MOVE *move         : The move to put into a string.
**
************************************************************************/

void MoveToString(MOVE theMove, char *moveStringBuffer) {
	/* The plus 1 is because the user thinks it's 1-9, but MOVE is 0-8 */
	sprintf(moveStringBuffer, "%d", theMove + 10);
}

/************************************************************************
*************************************************************************
** BEGIN   FUZZY STATIC EVALUATION ROUTINES. DON'T WORRY ABOUT UNLESS
**         YOU'RE NOT GOING TO EXHAUSTIVELY SEARCH THIS GAME
*************************************************************************
************************************************************************/

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

void PositionToBlankOX(POSITION thePos, BlankOX *theBlankOX) {
	int i;
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

POSITION BlankOXToPosition(BlankOX *theBlankOX) {
	int i;
	POSITION position = 0;

	for(i = 0; i < BOARDSIZE; i++)
		position += g3Array[i] * (int)theBlankOX[i]; /* was (int)position... */

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
	return(       theBlankOX[a] == theBlankOX[b] &&
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

BlankOX WhoseTurn(BlankOX *theBlankOX) {
  int moveCount = 0;
  for(int i = 0; i < BOARDSIZE; i++)
		if(theBlankOX[i] == x || theBlankOX[i] == o)
			moveCount++;

	if(moveCount % 2 == 0){
    return (x);
  }else{
    return (o);
  }
}

int NumberOfOptions()
{
	return 2;
}

int getOption()
{
	int option = 0;
	option += gStandardGame;
	option *= 2;
	option += gSymmetries;
	return option+1;
}

void setOption(int option)
{
	option -= 1;
	gSymmetries = option % 2;
	option /= 2;
	gStandardGame = option;
}

POSITION ActualNumberOfPositions(int variant) {
	(void)variant;
	return 5478;
}

POSITION StringToPosition(char *positionString) {
	(void) positionString;
	return NULL_POSITION;
}

void PositionToAutoGUIString(POSITION position, char *autoguiPositionStringBuffer) {
	(void) position;
	(void) autoguiPositionStringBuffer;
}

void MoveToAutoGUIString(POSITION position, MOVE move, char *autoguiMoveStringBuffer) {
	(void) position;
	(void) move;
	(void) autoguiMoveStringBuffer;
}
