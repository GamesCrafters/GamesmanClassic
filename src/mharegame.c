/************************************************************************
**
** NAME:        m<your-game-name-here>.c
**
** DESCRIPTION: <your-game-description>
**
** AUTHOR:      Firstname Lastname
**
** DATE:        YYYY-MM-DD
**
************************************************************************/

#include <stdio.h>
#include "gamesman.h"

POSITION gNumberOfPositions = 0; // TODO: Calculate a tight upper bound on the number of positions.
POSITION kBadPosition = -1;

POSITION gInitialPosition = 0; // TODO: You may need to change this based on how you hash the initial position. 

STRING kAuthorName = ""; // TODO: Your name here
STRING kGameName = "Hare Game";
STRING kDBName = "haregame";
BOOLEAN kPartizan = FALSE; // TODO: Set this to true if different sets of moves are available to each player given a position.
BOOLEAN kDebugMenu = FALSE;
BOOLEAN kGameSpecificMenu = FALSE; // TODO: Set this to true since we need a game-specific menu to choose the game variant.
BOOLEAN kTieIsPossible = FALSE; // TODO: Think... is a tie (or draw) possible in this game?
BOOLEAN kLoopy = FALSE; // TODO: Is this a loopy game? Can I loop back to a position I've seen before?
BOOLEAN kDebugDetermineValue = FALSE;
BOOLEAN kSupportsSymmetries = FALSE; // TODO: Whether we support symmetries. You will in this HomeFun.
void* gGameSpecificTclInit = NULL;

/* TODO: These are not required for the the game to solve and play 
properly but it is good to write down these explanations. */
STRING kHelpGraphicInterface = ""; // Ignore for now.
STRING kHelpTextInterface = ""; // Answer to "What do I do on MY TURN?"
STRING kHelpOnYourTurn = ""; // Answer to "How do I tell the computer WHICH MOVE I want?"
STRING kHelpStandardObjective = ""; // Answer to "What is the %s OBJECTIVE of Hare Game?"
STRING kHelpReverseObjective = ""; // Ignore for now. 
STRING kHelpTieOccursWhen = ""; // Should follow "A Tie occurs when..."
STRING kHelpExample = ""; // A string that shows an example game.

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
POSITION ActualNumberOfPositions(int variant);

/************************************************************************
**
** NAME: InitializeDatabases
**
** DESCRIPTION: Initialize the gDatabase, a global variable.
**
************************************************************************/

void InitializeGame() {
  gCanonicalPosition = GetCanonicalPosition;
  gMoveToStringFunPtr = &MoveToString;
	gActualNumberOfPositionsOptFunPtr = &ActualNumberOfPositions;
}

/************************************************************************
**
** NAME: DebugMenu
**
** DESCRIPTION: Menu used to debug internal problems. Does nothing if
** kDebugMenu == FALSE
**
************************************************************************/

void DebugMenu() {
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
** CALLS: BOOLEAN ThreeInARow()
** BOOLEAN AllFilledIn()
** PositionToBlankOX()
**
************************************************************************/

VALUE Primitive(POSITION position) {
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
  return NULL;
}

/**************************************************/
/*********** SYMMETRY HANDLING BEGIN **************/
/**************************************************/

/************************************************************************
**
** NAME: GetCanonicalPosition
**
** DESCRIPTION: Go through all of the positions that are symmetrically
** equivalent and return the SMALLEST, which will be used
** as the canonical element for the equivalence set.
**
** INPUTS: POSITION position : The position return the canonical elt. of.
**
** OUTPUTS: POSITION : The canonical element of the set.
**
************************************************************************/

POSITION GetCanonicalPosition(POSITION position) {
  return position;
}

/************************************************************************
**
** NAME: DoSymmetry
**
** DESCRIPTION: Perform the symmetry operation specified by the input
** on the position specified by the input and return the
** new position, even if it's the same as the input.
**
** INPUTS: POSITION position : The position to branch the symmetry from.
** int symmetry : The number of the symmetry operation.
**
** OUTPUTS: POSITION, The position after the symmetry operation.
**
************************************************************************/

POSITION DoSymmetry(POSITION position, int symmetry) {
  return position;
}

/**************************************************/
/*********** SYMMETRY HANDLING END ****************/
/**************************************************/

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
  return Continue;
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
  return 0;
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
  return NULL;
}

/************************************************************************
**
** NAME: NumberOfOptions
**
** DESCRIPTION: Return how many different variants are supported.
**
************************************************************************/
int NumberOfOptions() {
  return 1;
}

/************************************************************************
**
** NAME: getOption
**
** DESCRIPTION: Return the variant ID of the variant currently being played.
**
************************************************************************/
int getOption() {
  return 0;
}

/************************************************************************
**
** NAME: getOption
**
** DESCRIPTION: Change variables according to what variant we want to switch to.
** 
** INPUT: The variant ID
**
************************************************************************/
void setOption(int option) {
}


POSITION ActualNumberOfPositions(int variant) {
  return 0;
}

POSITION InteractStringToPosition(STRING board) {
  return 0;
}

STRING InteractPositionToString(POSITION position) {
  return NULL;
}

STRING InteractPositionToEndData(POSITION position) {
  return NULL;
}

STRING InteractMoveToString(POSITION position, MOVE move) {
  return MoveToString(move);
}
