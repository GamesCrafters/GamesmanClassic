/************************************************************************
**
** NAME:        mfivefieldkono.c
**
** DESCRIPTION: Five-Field Kono
**
** AUTHOR:      Andrew Lee
**
** DATE:        2023-02-24
**
************************************************************************/

#include <stdio.h>
#include <math.h>
#include "gamesman.h"

// SORRY
// /* Factorial Helper Function */
// int fact(n) {
//   res = 1;
//   for (int i = 0; i < n; i++) res = i * res;
//   return res;
// }

// /* Board Struct */
// typedef struct {
//   char even_component[13]; // 12 characters + 1 null byte
//   char odd_component[14]; // 13 characters + 1 null byte
//   char outcome; // win = w, lose = l, tie = t
// } FFK_Board;

// /* Char to Ternary Converter */
// int convertChar(char char_component) {
//   if (char_component == '-') {
//     return 0;
//   } else if (char_component == 'o') {
//     return 1;
//   } else if (char_component == 'x') {
//     return 2;
//   }
//   return -1;
// }

// /* Int to Char Converter */
// char convertInt(char int_component) {
//   if (int_component == 0) {
//     return '-';
//   } else if (int_component == 1) {
//     return 'o';
//   } else if (int_component == 2) {
//     return 'x';
//   }
//   return '\0';
// }

// /* Hash Function for the Board*/
// int hash(FFK_Board *board) {
//   /* Base 3 Hash */
//   int total = 0;
//   int even_len = 12;
//   for (int i = 0; i < even_len; i++) {
//     total += convertChar(board->even_component[(even_len - 1) - i]) * pow(3, i);
//   }
//   int odd_len = 13;
//   for (int j = 0; j < odd_len, j++) {
//     total += convertChar(board->odd_component[(odd_len - 1) - j]) * pow(3, 12 + j)
//   }
//   return total;
// }

// /* Unhash function for the Board */
// FFK_Board unhash(int hash) {
//   FFK_Board *newBoard = malloc(sizeof(struct FFK_Board));
//   newBoard.even_component[12] = unhash_char;
//   newBoard.odd_component[13] = unhash_char;
//   int remain = -1;
//   int even_len = 12;
//   for (int i = 0; i < even_len; i++) {
//     remain = hash % 3;
//     hash = floor(hash/3);
//     board->even_component[(even_len - 1) - i] = convertInt(remain);
//   }
//   int odd_len = 13;
//   for (int j = 0; j < odd_len; j++) {
//     remain = hash % 3;
//     hash = floor(hash/3);
//     board->even_component[(odd_len - 1) - j] = convertInt(remain);
//   }
// }

/* IMPORTANT GLOBAL VARIABLES */
STRING kAuthorName = "Andrew Lee";
POSITION gNumberOfPositions = 1189188000; // TODO: Put your number of positions upper bound here.
POSITION gInitialPosition = 0; // TODO: Put the hash value of the initial position.
BOOLEAN kPartizan = TRUE; // TODO: Is the game PARTIZAN i.e. given a board does each player have a different set of moves available to them?
BOOLEAN kTieIsPossible = TRUE; // TODO: Is a tie or draw possible?
BOOLEAN kLoopy = TRUE; // TODO: Is this game loopy?
BOOLEAN kSupportsSymmetries = TRUE; // TODO: Whether symmetries are supported (i.e. whether the GetCanonicalPosition is implemented)

/* Do not change these. */
POSITION GetCanonicalPosition(POSITION);
STRING MoveToString(MOVE);
POSITION kBadPosition = -1;
STRING kGameName = "Five-Field Kono";
STRING kDBName = "fivefieldkono";
BOOLEAN kDebugDetermineValue = FALSE;
void* gGameSpecificTclInit = NULL;

/* You don't have to change these for now. */
BOOLEAN kGameSpecificMenu = FALSE;
BOOLEAN kDebugMenu = FALSE;

/* These variables are not needed for solving but if you have time 
after you're done solving the game you should initialize them 
with something helpful. */
STRING kHelpGraphicInterface = "";
STRING kHelpTextInterface = "";
STRING kHelpOnYourTurn = "";
STRING kHelpStandardObjective = "";
STRING kHelpReverseObjective = "";
STRING kHelpTieOccursWhen = /* Should follow 'A Tie occurs when... */ "";
STRING kHelpExample = "";

/* You don't have to change this. */
void DebugMenu() {}
/* Ignore this function. */
void SetTclCGameSpecificOptions(int theOptions[]) {}
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
  // SORRY
  // struct FFK_Board* initial_board = malloc(sizeof(struct FFK_Board));
  // initial_board->even_component = "ooo-o--x-xxx";
  // initial_board->odd_component = "ooo-------xxx";
  // initial_board->outcome = 'I';
  // return hash(initial_board);
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

/* Return lose, win, tie, or undecided. See src/core/types.h
for the value enum definition. */
VALUE Primitive(POSITION position) {
  // SORRY
  // bool x_wins;
  // FFK_Board* board = unhash(position);
  // TODO
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
  /* YOUR CODE HERE */
  return Continue;
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
type in. */
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

/* How many variants are you supporting? For now, just 1.
Maybe in the future you want to support more variants. */
int NumberOfOptions() {
  /* YOUR CODE HERE MAYBE LATER BUT NOT NOW */
  return 1;
}

/* Return the current variant id (which is 0 in this case since
for now you're only thinking about one variant). */
int getOption() {
  /* YOUR CODE HERE MAYBE LATER BUT NOT NOW */
  return 0;
}

/* The input is a variant id. This function sets any global variables
or data structures according to the variant specified by the variant id. 
But for now you have one variant so don't worry about this. */
void setOption(int option) {
  /* YOUR CODE HERE MAYBE LATER BUT NOT NOW */
}

/*********** END VARIANT-RELATED FUNCTIONS ***********/







/* Don't worry about these Interact functions below yet.
They are used for the AutoGUI which eventually we would
want to implement, but they are not needed for solving. */
POSITION InteractStringToPosition(STRING board) {
  /* YOUR CODE HERE LATER BUT NOT NOW */
  return 0;
}

STRING InteractPositionToString(POSITION position) {
  /* YOUR CODE HERE LATER BUT NOT NOW */
  return NULL;
}

/* Ignore this function. */
STRING InteractPositionToEndData(POSITION position) {
  return NULL;
}

STRING InteractMoveToString(POSITION position, MOVE move) {
  /* YOUR CODE HERE LATER BUT NOT NOW */
  return MoveToString(move);
}