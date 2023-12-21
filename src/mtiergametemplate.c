/************************************************************************
**
** NAME:        m<your game name>.c
**
** DESCRIPTION: <Your Game Name> (use this spacing and case)
**
** AUTHOR:      Firstname Lastname
**
** DATE:        YYYY-MM-DD
**
************************************************************************/

#include "gamesman.h"

/* IMPORTANT GLOBAL VARIABLES */
CONST_STRING kAuthorName = "Firstname Lastname";
CONST_STRING kGameName = "Your Game Name";  //  use this spacing and case
CONST_STRING kDBName = "yourgamename";      // use this spacing and case
POSITION gNumberOfPositions =
    0;  // TODO: Put your number of positions upper bound here.
POSITION gInitialPosition =
    0;  // TODO: Put the hash value of the initial position.
BOOLEAN kPartizan =
    FALSE;  // TODO: Is the game PARTIZAN i.e. given a board does each player
            // have a different set of moves available to them?
BOOLEAN kTieIsPossible = FALSE;  // TODO: Is a tie or draw possible?
BOOLEAN kLoopy = FALSE;          // TODO: Is this game loopy?
BOOLEAN kSupportsSymmetries =
    FALSE;  // TODO: Whether symmetries are supported (i.e. whether the
            // GetCanonicalPosition is implemented)

/* Likely you do not have to change these. */
POSITION GetCanonicalPosition(POSITION);
STRING MoveToString(MOVE);
POSITION kBadPosition = -1;
BOOLEAN kDebugDetermineValue = FALSE;
void *gGameSpecificTclInit = NULL;

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
void SetTclCGameSpecificOptions(int theOptions[]) { (void)theOptions; }
/* Do not worry about this yet because you will only be supporting 1 variant for
 * now. */
void GameSpecificMenu() {}

/* Tier Functions for TierGamesman Support */
TIERLIST *getTierChildren(TIER tier);
TIERPOSITION numberOfTierPositions(TIER tier);
UNDOMOVELIST *GenerateUndoMovesToTier(
    POSITION position, TIER tier);  // Not needed if not using undomove
POSITION UndoMove(POSITION position,
                  UNDOMOVE undoMove);  // Not needed if not using undomove

/*********** BEGIN BASIC SOLVING FUNCIONS ***********/

/* TODO: Add a hashing function and unhashing function, if needed. */

/* Initialize any global variables or data structures needed before
solving or playing the game. */
void InitializeGame() {
    gCanonicalPosition = GetCanonicalPosition;
    gMoveToStringFunPtr = &MoveToString;

    /* Tier and UndoMove Variables and Function Pointers */
    gTierChildrenFunPtr = &getTierChildren;
    gNumberOfTierPositionsFunPtr = &numberOfTierPositions;
    gInitialTierPosition = gInitialPosition;  // May need to modify this
    kSupportsTierGamesman = TRUE;
    kExclusivelyTierGamesman = TRUE;  // Set this to false if you wish
    gInitialTier = 0;                 // May need to modify this
    gUnDoMoveFunPtr =
        &UndoMove;  // If you decide not to use UndoMove, then remove this
    gGenerateUndoMovesToTierFunPtr =
        &GenerateUndoMovesToTier;  // If you decide not to use UndoMove, remove
                                   // this

    /* YOUR CODE HERE */
}

/* Return the hash value of the initial position within the initial tier. */
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

/*********** END BASIC SOLVING FUNCTIONS ***********/

/*********** BEGIN TIER/UNDOMOVE FUNCTIONS ***********/

TIERLIST *getTierChildren(TIER tier) {
    /*
      Return a linked list of child tiers of this tier.
      If making a move from some position in this tier can
      lead to another position in this tier, then include
      this tier in the linked list as well.

      You can use CreateTierlistNode(tier, <tierchild linked list>) to append to
      linked list.
    */

    /* YOUR CODE HERE */
    return NULL;
}

/* How many positions are in the input tier? You may return
an upper bound on the number of positions (if perhaps
the way you encode positions includes unreachable positions).
Try to make that upper bound as small as you can. */
TIERPOSITION numberOfTierPositions(TIER tier) {
    /* YOUR CODE HERE */
    return 1;
}

/* If not using undomove, you can remove this function.
If using undomove, return a linked list of all possible moves from parent
positions in the input tier that could have been made in order to arrive
at the input position. */
UNDOMOVELIST *GenerateUndoMovesToTier(POSITION position, TIER tier) {
    /*
      You can use CreateUndoMovelistNode() to append to a linked list
      of undomoves.
    */

    /* YOUR CODE HERE */
    return NULL;
}

/* If not using undomove, you can remove this function.
Return the parent position given the undoMove. */
POSITION UndoMove(POSITION position, UNDOMOVE undoMove) {
    /* YOUR CODE HERE */
    return 0;
}

/*********** END TIER/UNDOMOVE FUNCTIONS ***********/

/*********** BEGIN TEXTUI FUNCTIONS ***********/

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn) {
    /* THIS ONE IS MOST IMPORTANT FOR YOUR DEBUGGING */
    /* YOUR CODE HERE */
}

void PrintComputersMove(MOVE computersMove, STRING computersName) {
    STRING moveString = MoveToString(computersMove);
    // Modify the computer's move prompt as you wish
    // If anything is malloced, make sure to free it!
    printf("%s's move: %s\n", computersName, moveString);
    SafeFree(moveString);
}

USERINPUT GetAndPrintPlayersMove(POSITION position, MOVE *move,
                                 STRING playerName) {
    USERINPUT ret;
    do {
        /* List of available moves */
        // Modify the player's move prompt as you wish
        printf("%8s's move: ", playerName);

        ret = HandleDefaultTextInput(position, move, playerName);
        if (ret != Continue) return (ret);

    } while (TRUE);
    return (Continue); /* this is never reached, but lint is now happy */
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

/* Return the string representation of the move in a
buffer in heap space.
Ideally this matches with what the user is supposed to
type when they specify moves. */
STRING MoveToString(MOVE move) {
    /* YOUR CODE HERE */
    return NULL;
}

/* Basically just print the move. */
void PrintMove(MOVE move) {
    /* YOUR CODE HERE */
    STRING moveString = MoveToString(move);
    printf("%s", moveString);
    SafeFree(moveString);
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
void setOption(int option) { /* YOUR CODE HERE  */ }

/*********** END VARIANT-RELATED FUNCTIONS ***********/

/* Don't worry about these Interact functions below yet.
They are used for the AutoGUI which eventually we would
want to implement, but they are not needed for solving. */
POSITION InteractStringToPosition(STRING str) {
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