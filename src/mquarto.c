/************************************************************************
**
** NAME:        mquarto.c
**
** DESCRIPTION: Quarto
**
** AUTHOR:      Cameron Cheung
**
** DATE:        2022-10-23
**
** NOTE: THIS DOES NOT WORK YET LOCALLY ON CLASSIC. ONLY GamesmanUni VERSION SHOULD WORK FOR NOW
**
************************************************************************/

#include "gamesman.h"

POSITION gNumberOfPositions = 0;
POSITION kBadPosition = -1;

POSITION gInitialPosition = 0;
POSITION gMinimalPosition = 0;

CONST_STRING kAuthorName = "Cameron Cheung";
CONST_STRING kGameName = "mquarto";
CONST_STRING kDBName = "quarto"; // should not be used
BOOLEAN kPartizan = TRUE;
BOOLEAN kDebugMenu = FALSE;
BOOLEAN kGameSpecificMenu = FALSE;
BOOLEAN kTieIsPossible = TRUE;
BOOLEAN kLoopy = FALSE;
BOOLEAN kDebugDetermineValue = FALSE;
BOOLEAN kSupportsSymmetries = FALSE; /* Whether we support symmetries */
void* gGameSpecificTclInit = NULL;

CONST_STRING kHelpGraphicInterface = "";

CONST_STRING kHelpTextInterface = "";

CONST_STRING kHelpOnYourTurn = "";

CONST_STRING kHelpStandardObjective = "";

CONST_STRING kHelpReverseObjective = "";

CONST_STRING kHelpTieOccursWhen = /* Should follow 'A Tie occurs when... */ "";

CONST_STRING kHelpExample = "";

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

POSITION GetCanonicalPosition(POSITION);
STRING MoveToString(MOVE);

POSITION GetCanonicalPosition(POSITION position) {
  return position;
}

void DebugMenu() {
}

void SetTclCGameSpecificOptions(int theOptions[]) {
  (void)theOptions;
}


/************************************************************************
**
** NAME: InitializeDatabases
**
** DESCRIPTION: Initialize the gDatabase, a global variable.
**
************************************************************************/

void InitializeGame() {
  kUsesQuartoGamesman = TRUE;
  gInitialPosition = 0;
}

/************************************************************************
**
** NAME: GameSpecificMenu
**
** DESCRIPTION: Menu used to change game-specific parmeters, such as
** the side of the board in an nxn Nim board, etc. Does
** nothing if kGameSpecificMenu == FALSE
**
************************************************************************/

void GameSpecificMenu() {
}

/************************************************************************
**
** NAME: DoMove
**
** DESCRIPTION: Apply the move to the position.
**
** INPUTS: POSITION position : The old position
** MOVE move : The move to apply.
**
** OUTPUTS: (POSITION) : The position that results after the move.
**
** CALLS: PositionToBlankOX(POSITION,*BlankOX)
** BlankOX WhosTurn(*BlankOX)
**
************************************************************************/

POSITION DoMove(POSITION position, MOVE move) {
  (void)position;
  (void)move;
  return 0;
}

/************************************************************************
**
** NAME: GetInitialPosition
**
** DESCRIPTION: Ask the user for an initial position for testing. Store
** it in the space pointed to by initialPosition;
**
** OUTPUTS: POSITION initialPosition : The position to fill.
**
************************************************************************/

POSITION GetInitialPosition() {
  return 0;
}

/************************************************************************
**
** NAME: PrintComputersMove
**
** DESCRIPTION: Nicely format the computers move.
**
** INPUTS: MOVE *computersMove : The computer's move.
** STRING computersName : The computer's name.
**
************************************************************************/

void PrintComputersMove(MOVE computersMove, STRING computersName) {
  (void)computersMove;
  (void)computersName;
  printf("the computer moved\n");
}

/************************************************************************
**
** NAME: Primitive
**
** DESCRIPTION: Return the value of a position if it fulfills certain
** 'primitive' constraints. Some examples of this is having
** three-in-a-row with TicTacToe. TicTacToe has two
** primitives it can immediately check for, when the board
** is filled but nobody has one = primitive tie. Three in
** a row is a primitive lose, because the player who faces
** this board has just lost. I.e. the player before him
** created the board and won. Otherwise undecided.
**
** INPUTS: POSITION position : The position to inspect.
**
** OUTPUTS: (VALUE) an enum which is oneof: (win,lose,tie,undecided)
**
************************************************************************/

VALUE Primitive(POSITION position) {
  (void)position;
  return undecided;
}

/************************************************************************
**
** NAME: PrintPosition
**
** DESCRIPTION: Print the position in a pretty format, including the
** prediction of the game's outcome.
**
** INPUTS: POSITION position : The position to pretty print.
** STRING playerName : The name of the player.
** BOOLEAN usersTurn : TRUE <==> it's a user's turn.
**
** CALLS: PositionToBlankOX()
** GetValueOfPosition()
** GetPrediction()
**
************************************************************************/

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn) {
    (void)position;
    (void)playerName;
    (void)usersTurn;
}

/************************************************************************
**
** NAME: GenerateMoves
**
** DESCRIPTION: Create a linked list of every move that can be reached
** from this position. Return a pointer to the head of the
** linked list.
**
** INPUTS: POSITION position : The position to branch off of.
**
** OUTPUTS: (MOVELIST *), a pointer that points to the first item
** in the linked list of moves that can be generated.
**
** CALLS: MOVELIST *CreateMovelistNode(MOVE,MOVELIST *)
**
************************************************************************/

MOVELIST *GenerateMoves(POSITION position) {
  (void)position;
  return NULL;
}

/************************************************************************
**
** NAME: GetAndPrintPlayersMove
**
** DESCRIPTION: This finds out if the player wanted an undo or abort or not.
** If so, return Undo or Abort and don't change theMove.
** Otherwise get the new theMove and fill the pointer up.
**
** INPUTS: POSITION *thePosition : The position the user is at.
** MOVE *theMove : The move to fill with user's move.
** STRING playerName : The name of the player whose turn it is
**
** OUTPUTS: USERINPUT : Oneof( Undo, Abort, Continue )
**
** CALLS: ValidMove(MOVE, POSITION)
** BOOLEAN PrintPossibleMoves(POSITION) ...Always True!
**
************************************************************************/

USERINPUT GetAndPrintPlayersMove(POSITION position, MOVE *move, STRING playerName) {
  /* local variables */
	USERINPUT ret, HandleDefaultTextInput();
	do {
		printf("Enter your move here: ");
		ret = HandleDefaultTextInput(position, move, playerName);
		if(ret != Continue) return(ret);
	}
	while (TRUE);
	return(Continue); /* this is never reached, but link is now happy */
}

/************************************************************************
**
** NAME: ValidTextInput
**
** DESCRIPTION: Return TRUE iff the string input is of the right 'form'.
** For example, if the user is allowed to select one slot
** from the numbers 1-9, and the user chooses 0, it's not
** valid, but anything from 1-9 IS, regardless if the slot
** is filled or not. Whether the slot is filled is left up
** to another routine.
**
** INPUTS: STRING input : The string input the user typed.
**
** OUTPUTS: BOOLEAN : TRUE iff the input is a valid text input.
**
************************************************************************/

BOOLEAN ValidTextInput(STRING input) {
  (void)input;
  return TRUE;
}

/************************************************************************
**
** NAME: ConvertTextInputToMove
**
** DESCRIPTION: Convert the string input to the internal move representation.
**
** INPUTS: STRING input : The string input the user typed.
**
** OUTPUTS: MOVE : The move corresponding to the user's input.
**
************************************************************************/

MOVE ConvertTextInputToMove(STRING input) {
  if (input[1]) {
    return (input[0] - '0') * 10 + (input[1] - '0');
  } else {
    return input[0] - '0';
  }
}

/************************************************************************
**
** NAME: PrintMove
**
** DESCRIPTION: Print the move in a nice format.
**
** INPUTS: MOVE *theMove : The move to print.
**
************************************************************************/

void PrintMove(MOVE move) {
    printf("%s", MoveToString(move));
}

/************************************************************************
**
** NAME: MoveToString
**
** DESCRIPTION: Returns the move as a STRING
**
** INPUTS: MOVE *move : The move to put into a string.
**
************************************************************************/

STRING MoveToString(MOVE move) {
  (void)move;
  return NULL;
}

int NumberOfOptions() {
  return 1;
}

int getOption() {
  return 0;
}

void setOption(int option) {
  (void)option;
  gInitialPosition = 0;
}

POSITION InteractStringToPosition(STRING str) {
  (void)str;
 return 0;
}

STRING InteractPositionToString(POSITION position) {
  (void)position;
  return UWAPI_Board_Regular2D_MakePositionString(UWAPI_TURN_A, 17, 1, "-----------------");
}

STRING InteractMoveToString(POSITION position, MOVE move) {
  (void)position;
  (void)move;
  return "not implemented";
}