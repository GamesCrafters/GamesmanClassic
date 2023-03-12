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




/* GLOBAL VARIABLES */

/* The person who implemented this game */
STRING kAuthorName = "Andrew Lee";

/* Full name of the game */
STRING kGameName = "Five-Field Kono";

/* Name of the game for databases */
STRING kDBName = "fivefieldkono";

/* Tightest upper bound for number of positions */
POSITION gNumberOfPositions = 1189188000;

/* The hash value of the initial position of the board */
POSITION gInitialPosition = 0;

/* The hash value of any invalid position */
POSITION kBadPosition = -1;

/* There can be different moves available to each player */
BOOLEAN kPartizan = TRUE;

/* It is possible to tie or draw */
BOOLEAN kTieIsPossible = TRUE;

/* The game is loopy */
BOOLEAN kLoopy = TRUE;

/* GetCanonicalPosition will be implemented */
BOOLEAN kSupportsSymmetries = TRUE;

/* TODO: No clue what this does */
BOOLEAN kDebugDetermineValue = FALSE;

/* For initializing the game in Tcl non-generically */
void* gGameSpecificTclInit = NULL;

/* Useful when there are variants available */
BOOLEAN kGameSpecificMenu = FALSE;

/* Enables debug menu for... debugging */
BOOLEAN kDebugMenu = FALSE;

/* Help strings for human players */
STRING kHelpGraphicInterface = "";
STRING kHelpTextInterface = "";
STRING kHelpOnYourTurn = "";
STRING kHelpStandardObjective = "";
STRING kHelpReverseObjective = "";
STRING kHelpTieOccursWhen = "";
STRING kHelpExample = "";




/* FUNCTIONAL DECLARATION */

/* Solving functions. */
void InitializeGame();
POSITION GetInitialPosition();
MOVELIST *GenerateMoves(POSITION hash);
POSITION DoMove(POSITION hash, MOVE move);
VALUE Primitive(POSITION position);

/* Solving helper functions. */
void evaluateEven(int currPos, int newPos, BOOLEAN turn, MOVELIST **moves, char *even_component);
void evaluateOdd(int currPos, int newPos, BOOLEAN turn, MOVELIST **moves, char *odd_component);
int convertChar(char char_component);
char convertInt(char int_component);

/* Transformation functions. */
void transform(FFK_Board* board, int flips, int rotations);
void rotate(FFK_Board* board);
void flip(FFK_Board* board);

/* Board hashing functions. */
POSITION hash(FFK_Board *board);
FFK_Board* unhash(POSITION hash);

/* Board hashing helper functions. */
POSITION withTurn(POSITION pos, BOOLEAN turn);
BOOLEAN getTurn(POSITION hash);

/* Board value functions. */
BOOLEAN isWin(FFK_Board* board);
BOOLEAN isLose(FFK_Board* board);
BOOLEAN isTie(FFK_Board* board);

/* Move hashing functions. */
MOVE hashMove(int oldPos, int newPos, BOOLEAN turn);
void unhashMove(MOVE mv, int *oldPos, int *newPos, BOOLEAN *turn);




/* BOARD DEFINITION */

/* A Five-Field Kono board consists of 25 spots, but has two connected 
components (such that pieces that start out in one cannot possibly move
into the other). Since clever solving might be possible due to this, we
store them separately. */
typedef struct {
  char *even_component; // 12 characters + 1 null byte
  char *odd_component; // 13 characters + 1 null byte
  BOOLEAN turn; // TRUE = your turn (piece x), FALSE = opponent turn (piece o)
} FFK_Board;




/* BOARD TRANSFORMATION ARRAYS */

/* Describes a board transformation of a 90 degree turn clockwise, 
where the piece at original[i] ends up at destination[array[i]],
where 'array' is one of the arrays below. */
int even_turn_pos[12] = {4, 9, 1, 6, 11, 3, 8, 0, 5, 10, 2, 7};
int odd_turn_pos[13] = {2, 7, 12, 4, 9, 1, 6, 11, 3, 8, 0, 5, 10};

/* Describes a board transformation of a horizontal flip (a reflection
across the y-axis, so to say). The piece at original[i] should end up 
at destination[array[i]], where 'array' is one of the arrays below. */
int even_flip_pos[12] = {1, 0, 4, 3, 2, 6, 5, 9, 8, 7, 11, 10};
int odd_flip_pos[13] = {2, 1, 0, 4, 3, 7, 6, 5, 9, 8, 12, 11, 10};




/* SOLVING FUNCIONS */

/* Initialize any global variables or data structures needed. */
void InitializeGame() {
  gCanonicalPosition = GetCanonicalPosition();
  gMoveToStringFunPtr = &MoveToString();
  gInitialPosition = GetInitialPosition();
}

/* Return the hash value of the initial position. */
POSITION GetInitialPosition() {
  FFK_Board *initial_board = (FFK_Board *) malloc(sizeof(FFK_Board));
  initial_board->even_component = malloc(sizeof(char)*13);
  initial_board->odd_component = malloc(sizeof(char)*14);
  strcpy(initial_board->even_component, "ooo-o--x-xxx");
  strcpy(initial_board->odd_component, "ooo-------xxx");
  initial_board->turn = TRUE;
  return hash(initial_board);
}

void evaluateEven(int currPos, int newPos, BOOLEAN turn, MOVELIST **moves, char *even_component) {
  if (newPos < 0 || newPos >= 12) {
    return;
  }
  int currElem = convertChar(even_component[currPos]);
  int newElem = convertChar(even_component[newPos]);
  int match = turn ? 2 : 1;
  if (currElem > 0 && newElem == 0 && currElem == match) {
    *moves = CreateMovelistNode(hashMove((12 - 1) - currPos, (12 - 1) - newPos), *moves);
  }
}

void evaluateOdd(int currPos, int newPos, BOOLEAN turn, MOVELIST **moves, char *odd_component) {
  if (newPos < 0 || newPos >= 13) {
    return;
  }
  int currElem = convertChar(odd_component[currPos]);
  int newElem = convertChar(odd_component[newPos]);
  int match = turn ? 2 : 1;
  if (currElem > 0 && newElem == 0 && currElem == match) {
    *moves = CreateMovelistNode(hashMove((25 - 1) - currPos, (25 - 1) - newPos), *moves);
  }
}

/* Return a linked list of possible moves. */
MOVELIST *GenerateMoves(POSITION hash) {
  FFK_Board *newboard = unhash(hash);
  MOVELIST *moves = NULL;

  /* - = 0; o = 1; x = 2; total = 0b<turn_bit, base_3_odd_component, base_3_even_component))> */
  int even_len = 12;
  for (int i = 0; i < even_len; i++) {
    if (i != 4 || i != 9) {
      evaluateEven(hash, i, i - 2, &moves, newboard->even_component);
      evaluateEven(hash, i, i + 3, &moves, newboard->even_component);
    }
    if (i != 2 || i != 7) {
      evaluateEven(hash, i, i + 2, &moves, newboard->even_component);
      evaluateEven(hash, i, i - 3, &moves, newboard->even_component);
    }
  }

  int odd_len = 13;
  for (int j = 0; j < odd_len; j++) {
    if (i != 0 || i != 5 || i != 10) {
      evaluateOdd(hash, j, j + 2, &moves, newboard->odd_component);
      evaluateOdd(hash, j, j - 3, &moves, newboard->odd_component);
    }
    if (i != 2 || i != 7 || i != 12) {
      evaluateOdd(hash, j, j - 2, &moves, newboard->odd_component);
      evaluateOdd(hash, j, j + 3, &moves, newboard->odd_component);
    }
  }

  return moves;
}

/* Return the resulting position from making 'move' on 'position'. */
POSITION DoMove(POSITION hash, MOVE move) {
  int oldPos, newPos;
  unhashMove(move, &oldPos, &newPos);
  int piece_type = turn ? 2: 1; // o == 1 and x == 2 in base 3
  POSITION original = piece_type * pow(3, oldPos);
  POSITION new = piece_type * pow(3, newPos);
  BOOLEAN turn = getTurn(hash);
  return withTurn(hash - original + new, (turn + 1) % 2);
}

/* Symmetry Handling: Return the canonical position. */
POSITION GetCanonicalPosition(POSITION position) {
  // TODO
  return position;
}

/* Return lose, win, tie, or undecided. See src/core/types.h
for the value enum definition. */
VALUE Primitive(POSITION position) {
  FFK_Board *board = unhash(position);
  BOOLEAN is_win = isWin(board);
  BOOLEAN is_lose = isLose(board);
  BOOLEAN is_tie = isTie(board);
  free(board);
  if (is_win) {
    return win;
  }
  if (is_lose) {
    return lose;
  }
  if (is_tie) {
    return tie;
  }
  return undecided;
}




/* SOLVING HELPER FUNCTIONS */

/* TODO */
void evaluateEven(int currPos, int newPos, BOOLEAN turn, MOVELIST **moves, char *even_component) {
  if (newPos < 0 || newPos >= 12) {
    return;
  }
  int currElem = convertChar(even_component[currPos]);
  int newElem = convertChar(even_component[newPos]);
  int match = turn ? 2 : 1;
  if (currElem > 0 && newElem == 0 && currElem == match) {
    *moves = CreateMovelistNode(hashMove((12 - 1) - currPos, (12 - 1) - newPos, turn), *moves);
  }
}

/* TODO */
void evaluateOdd(int currPos, int newPos, BOOLEAN turn, MOVELIST **moves, char *odd_component) {
  if (newPos < 0 || newPos >= 13) {
    return;
  }
  int currElem = convertChar(odd_component[currPos]);
  int newElem = convertChar(odd_component[newPos]);
  int match = turn ? 2 : 1;
  if (currElem > 0 && newElem == 0 && currElem == match) {
    *moves = CreateMovelistNode(hashMove((25 - 1) - currPos, (25 - 1) - newPos, turn), *moves);
  }
}

/* Converts a character to its ternary integer equivalent
for hash calculations. */
int convertChar(char char_component) {
  if (char_component == '-') {
    return 0;
  } else if (char_component == 'o') {
    return 1;
  } else if (char_component == 'x') {
    return 2;
  }
  return -1;
}

/* Converts a ternary integer to its character equivalent 
for hash calculations. */
char convertInt(char int_component) {
  if (int_component == 0) {
    return '-';
  } else if (int_component == 1) {
    return 'o';
  } else if (int_component == 2) {
    return 'x';
  }
  return '\0';
}




/* TRANSFORMATION FUNCTIONS */

/* Transforms the board by rotating it 90 degrees clockwise. */
void rotate(FFK_Board* board) {

  /* POSSIBLE OPTIMIZATION: Do this in-place (without allocating another board)
  by moving each value to its new place, and moving the new place's old value
  next. Note that this would de-parallelize the operation (the compiler should
  perform the for loop cycles asynchronously). */

  /* Rotate odd component clockwise. */
  char *new_even_arr = (char *) malloc(sizeof(board->even_component));
  int even_len = 12;
  for (int i = 0; i < even_len; i++) {
    new_even_arr[i] = board->even_component[even_turn_pos[i]];
  }
  new_even_arr[even_len] = '\0';

  /* Rotate even component clockwise. */
  char *new_odd_arr = (char *) malloc(sizeof(board->odd_component));
  int odd_len = 13;
  for (int j = 0; j < odd_len; j++) {
    new_odd_arr[j] = board->odd_component[odd_turn_pos[j]];
  }
  new_odd_arr[odd_len] = '\0';

  /* Free old board and assign transformed one. */
  free(board->even_component);
  free(board->odd_component);
  board->even_component = new_even_arr;
  board->odd_component = new_odd_arr;
}

/* Transforms the board by flipping it horizontally (a reflection 
about the y-axis). */
void flip(FFK_Board* board) {

  /* POSSIBLE OPTIMIZATION: Same as rotate(). */

  /* Flip odd component about y-axis. */
  char *new_even_arr = (char *) malloc(sizeof(board->even_component));
  int even_len = 12;
  for (int i = 0; i < even_len; i++) {
    new_even_arr[i] = board->even_component[even_flip_pos[i]];
  }
  new_even_arr[even_len] = '\0';

  /* Flip even component about y-axis. */
  char *new_odd_arr = (char *) malloc(sizeof(board->odd_component));
  int odd_len = 13;
  for (int j = 0; j < odd_len; j++) {
    new_odd_arr[j] = board->odd_component[odd_flip_pos[j]];
  }
  new_odd_arr[odd_len] = '\0';

  /* Free old board and assign transformed one. */
  free(board->even_component);
  free(board->odd_component);
  board->even_component = new_even_arr;
  board->odd_component = new_odd_arr;
}

/* Transforms the board by flipping and rotating it a specified
amount of times, helping to get all of its 8 symmetries. */
void transform(FFK_Board* board, int flips, int rotations) {
  for (int i = 0; i < flips; i++) flip(board);
  for (int j = 0; j < rotations; j++) rotate(board);
}




/* BOARD HASHING FUNCTIONS */

/* Calculates the unique base-3 integer that corresponds to the boar. */
POSITION hash(FFK_Board *board) {
  POSITION total = 0;
  int even_len = 12;
  for (int i = 0; i < even_len; i++) {
    total += convertChar(board->even_component[(even_len - 1) - i]) * pow(3, i);
  }
  int odd_len = 13;
  for (int j = 0; j < odd_len, j++) {
    total += convertChar(board->odd_component[(odd_len - 1) - j]) * pow(3, 12 + j)
  }
  // Returns base_3(concatenate(odd_component, even_component))
  // with the MSB flipped to board->turn
  return withTurn(total, board->turn);
}

/* Unhash function for the Board. */
FFK_Board* unhash(POSITION hash) {
  FFK_Board* newBoard = (FFK_Board *) malloc(sizeof(struct FFK_Board));
  newBoard->even_component = (char *) malloc(13 * sizeof(char));
  newBoard->odd_component = (char *) malloc(14 * sizeof(char));
  newBoard->turn = getTurn(hash);

  int remain = -1;
  int even_len = 12;
  newBoard->even_component[even_len] = '\0';
  for (int i = 0; i < even_len; i++) {
    remain = hash % 3;
    hash = floor(hash/3);
    board->even_component[(even_len - 1) - i] = convertInt(remain);
  }
  
  int odd_len = 13;
  newBoard->odd_component[odd_len] = '\0';
  for (int j = 0; j < odd_len; j++) {
    remain = hash % 3;
    hash = floor(hash/3);
    board->odd_component[(odd_len - 1) - j] = convertInt(remain);
  }
  return newBoard;
}




/* BOARD HASHING HELPER FUNCTIONS */

/* Returns a position with the MSB of POS set to TURN. */
POSITION withTurn(POSITION pos, BOOLEAN turn) {
  return turn ? (pos|0x8000000000000000) : pos;
}

/* Returns whose turn it is according to a position HASH. */
BOOLEAN getTurn(POSITION hash) {
  return !((hash&0x8000000000000000) == 0);
}




/* BOARD VALUE FUNCTIONS */

/* X wins when all of the spots originally populated by O's are 
filled with X's. */
BOOLEAN isWin(FFK_Board* board) {
  return board->even_component[0] == 'x'
  && board->even_component[1] == 'x'
  && board->even_component[2] == 'x'
  && board->even_component[4] == 'x'
  && board->odd_component[0] == 'x'
  && board->odd_component[1] == 'x'
  && board->odd_component[2] == 'x';
}

/* O wins when all of the spots originally populated by X's are 
filled with O's. */
BOOLEAN isLose(FFK_Board* board) {
  return board->even_component[12] == 'o'
  && board->even_component[11] == 'o'
  && board->even_component[10] == 'o'
  && board->even_component[8] == 'o'
  && board->odd_component[13] == 'o'
  && board->odd_component[12] == 'o'
  && board->odd_component[11] == 'o';
}

/* If there are no moves left to be made, then the game is a tie. */
BOOLEAN isTie(FFK_Board* board) {
  MOVELIST possible_moves = GenerateMoves(hash(board));
  if (possible_moves == NULL) {
    return TRUE;
  }
  return FALSE;
}




/* MOVE HASHING FUNCTIONS */

/* Uses the start and destination index of a piece in the connected
component of the board it belongs to and whose turn it is to generate
a unique hash for a game state graph edge. */
MOVE hashMove(int oldPos, int newPos) {
  // TODO: RETURN - 0b<base 25 newPos, base 25 oldPos>
  return 25*newPos + oldPos;
}

/* Obtains the start and destination index of a piece in the connected
component of the board it belongs to and whose turn it is based on
a unique hash for a game state graph edge. */
void unhashMove(MOVE mv, int *oldPos, int *newPos) {
  // TODO: unhashMove 
  // 0b<base 25 newPos, base 25 oldPos>
  *oldPos = mv % 25;
  mv = floor(mv/25);
  *newPos = mv % 25;
}




/* TEXTUI FUNCTIONS */

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn) {
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




/* VARIANT FUNCTIONS */

/* Amount of variants supported. */
int NumberOfOptions() { return 1; }

/* Return the current variant ID (0 in this case). */
int getOption() { return 0; }

/* The input is a variant id. This function sets any global variables
or data structures according to the variant specified by the variant id. 
But for now you have one variant so don't worry about this. */
void setOption(int option) { }




/* INTERACT FUNCTIONS */

POSITION InteractStringToPosition(STRING board) { return 0; }

STRING InteractPositionToString(POSITION position) { return NULL; }

STRING InteractPositionToEndData(POSITION position) { return NULL; }

STRING InteractMoveToString(POSITION position, MOVE move) { return MoveToString(move); }




/* UNUSED INTERFACE IMPLEMENTATIONS */

/* Debug menu for... debugging */
void DebugMenu() {}

/* Should be configured when Tcl is initialized non-generically */
void SetTclCGameSpecificOptions(int theOptions[]) {}

/* For implementing more than one variant */
void GameSpecificMenu() {}
