/************************************************************************
**
** NAME:        mstickaround.c
**
** DESCRIPTION: Stick Around  (use this spacing and case)
**
** AUTHOR:      Amy Chakladar
**
** DATE:        2023-08-08
**
************************************************************************/

#include "gamesman.h"

/* IMPORTANT GLOBAL VARIABLES */
CONST_STRING kAuthorName = "Amy Chakladar";
CONST_STRING kGameName = "Stick Around";
CONST_STRING kDBName = "stickaround";
POSITION gNumberOfPositions = 69;
POSITION gInitialPosition = 0; // TODO: Put the hash value of the initial position.
BOOLEAN kPartizan = TRUE;
BOOLEAN kTieIsPossible = FALSE; 
BOOLEAN kLoopy = FALSE;
BOOLEAN kSupportsSymmetries = TRUE; 

/* Likely you do not have to change these. */
POSITION GetCanonicalPosition(POSITION);
STRING MoveToString(MOVE);
POSITION kBadPosition = -1;
BOOLEAN kDebugDetermineValue = FALSE;
void* gGameSpecificTclInit = NULL;

/* You do not have to change these for now. */
BOOLEAN kGameSpecificMenu = FALSE;
BOOLEAN kDebugMenu = FALSE;

/* These variables are not needed for solving but if you have time 
after you're done solving the game you should initialize them 
with something helpful. */
CONST_STRING kHelpGraphicInterface = "";
CONST_STRING kHelpTextInterface = "";
CONST_STRING kHelpOnYourTurn = "";
CONST_STRING kHelpStandardObjective = "";
CONST_STRING kHelpReverseObjective = "";
CONST_STRING kHelpTieOccursWhen = /* Should follow 'A Tie occurs when... */ "";
CONST_STRING kHelpExample = "";

/* You don't have to change this. */
void DebugMenu() {}
/* Ignore this function. */
void SetTclCGameSpecificOptions(int theOptions[]) {
  (void)theOptions;
}
/* Do not worry about this yet because you will only be supporting 1 variant for now. */
void GameSpecificMenu() {}






/*********** BEGIN SOLVING FUNCIONS ***********/

/* TODO: Add a hashing function and unhashing function, if needed. */

/* Initialize any global variables or data structures needed before
solving or playing the game. */
void InitializeGame() {
  gCanonicalPosition = GetCanonicalPosition;
  gMoveToStringFunPtr = &MoveToString;

  /* YOUR CODE HERE */
  
}

/* Return the hash value of the initial position. */
POSITION GetInitialPosition() {
  /* YOUR CODE HERE */
  return 0;
}

/* Return a linked list of moves. */
MOVELIST *GenerateMoves(POSITION position) {
  MOVELIST *moves = NULL;
  /* YOUR CODE HERE 
     
     To add to the linked list, do 
     moves = CreateMovelistNode(<the move you're adding>, moves);
     See the function CreateMovelistNode in src/core/misc.c
  */
  return moves;
}

/* Return the position that results from making the 
input move on the input position. */
POSITION DoMove(POSITION position, MOVE move) {
  /* YOUR CODE HERE */
  return 0;
}

/*****************************************************************
**  Determine whether the position is a primitive win,
**  primitive tie, primitive lose, or not primitive.
**  OUTPUTS: (VALUE) an enum; one of: (win, lose, tie, undecided)
**  See src/core/types.h for the value enum definition.
******************************************************************/
VALUE Primitive(POSITION position) {
  /* YOUR CODE HERE */
  return undecided;
}

/* Symmetry Handling: Return the canonical position. */
POSITION GetCanonicalPosition(POSITION position) {
  /* YOUR CODE HERE */
  return position;
}

/*********** END SOLVING FUNCTIONS ***********/







/*********** BEGIN TEXTUI FUNCTIONS ***********/

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn) {
  /* THIS ONE IS MOST IMPORTANT FOR YOUR DEBUGGING */
  /* YOUR CODE HERE */
}

void PrintComputersMove(MOVE computersMove, STRING computersName) {
  /* YOUR CODE HERE */
}

USERINPUT GetAndPrintPlayersMove(POSITION position, MOVE *move, STRING playerName) {
  USERINPUT ret;
	do {
    /* List of available moves */
    // Modify the player's move prompt as you wish
		printf("%8s's move: ", playerName);

		ret = HandleDefaultTextInput(position, move, playerName);
		if(ret != Continue)
			return(ret);

	}
	while (TRUE);
	return(Continue); /* this is never reached, but lint is now happy */
}

/* Return whether the input text signifies a valid move. */
BOOLEAN ValidTextInput(STRING input) {
  /* YOUR CODE HERE */
  return TRUE;
}

/* Assume the text input signifies a valid move. Return
the move hash corresponding to the move. */
MOVE ConvertTextInputToMove(STRING input) {
  /* YOUR CODE HERE */
  return 0;
}

/* Return the string representation of the move. 
Ideally this matches with what the user is supposed to
type when they specify moves. */
STRING MoveToString(MOVE move) {
  /* YOUR CODE HERE */
  return NULL;
}

/* Basically just print the move. */
void PrintMove(MOVE move) {
  /* YOUR CODE HERE */
}

/*********** END TEXTUI FUNCTIONS ***********/









/*********** BEGIN VARIANT FUNCTIONS ***********/

/* How many variants are you supporting? */
int NumberOfOptions() {
  /* YOUR CODE HERE */
  return 1;
}

/* Return the current variant id. */
int getOption() {
  /* YOUR CODE HERE */
  return 0;
}

/* The input is a variant id. This function sets any global variables
or data structures according to the variant specified by the variant id. */
void setOption(int option) {
  /* YOUR CODE HERE  */
}

/*********** END VARIANT-RELATED FUNCTIONS ***********/







/* Don't worry about these Interact functions below yet.
They are used for the AutoGUI which eventually we would
want to implement, but they are not needed for solving. */
POSITION InteractStringToPosition(STRING board) {
  /* YOUR CODE HERE */
  return 0;
}

STRING InteractPositionToString(POSITION position) {
  /* YOUR CODE HERE */
  return NULL;
}

STRING InteractMoveToString(POSITION position, MOVE move) {
  /* YOUR CODE HERE */
  return MoveToString(move);
}