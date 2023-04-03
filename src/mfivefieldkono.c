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

/* How big our POSITION hash can get -- note that this isn't the same
as the upper bound on the number of positions of the game for unefficient 
(< 100% efficient) hashing methods */
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

/* 12!/(4!4!4!) = 34650 */ 
POSITION max_even_hash = 34650;

/* 13!/(3!3!7!) = 34320 */
POSITION max_odd_hash = 34320;

/* optimized factorial lookup: 0 to 25 */
int fact_array[26];




/* BOARD DEFINITION */

/* A Five-Field Kono board consists of 25 spots, but has two connected 
components (such that pieces that start out in one cannot possibly move
into the other). Since clever solving might be possible due to this, we
store them separately. */
typedef struct {
  char even_component[12];
  char odd_component[13];
} FFK_Board;




/* FUNCTIONAL DECLARATION */

/* Solving functions. */
void InitializeGame();
POSITION GetInitialPosition();
MOVELIST *GenerateMoves(POSITION hash);
POSITION GetCanonicalPosition(POSITION position);
POSITION DoMove(POSITION hash, MOVE move);
VALUE Primitive(POSITION position);

/* Solving helper functions. */
void evaluateEven(int currPos, int newPos, MOVELIST **moves, char *even_component);
void evaluateOdd(int currPos, int newPos, MOVELIST **moves, char *odd_component);
int convertChar(char char_component);
char convertInt(char int_component);

/* Transformation functions. */
void permute(char* target, int* map, int size);
void rotate(FFK_Board* board);
void flip(FFK_Board* board);

/* Board hashing functions. */
POSITION Hash(FFK_Board* board);
POSITION compute_hash(char* board_component, int slots, int num_x, int num_o);
FFK_Board* Unhash(POSITION in);
void compute_unhash(char* board_component, POSITION in, int slots, int num_x, int num_o);
POSITION rearrangements(int slots, int x, int o);
void precompute_fact(int* fact_array, int limit);
int factorial(int n);

/* Board value functions. */
BOOLEAN isWin(FFK_Board* board);
BOOLEAN isLose(FFK_Board* board);
BOOLEAN isTie(FFK_Board* board);

/* Move hashing functions. */
MOVE hashMove(int oldPos, int newPos);
void unhashMove(MOVE mv, int *oldPos, int *newPos);

/* Move hashing helper functions. */
char opposite(char piece);

/* Other stuff we don't care about for now. */
STRING MoveToString(MOVE move);




/* INITIAL BOARD DESCRIPTION */

/* Amount of 'o' pieces initially in each component. */
int initial_even_num_o = 4;
int initial_odd_num_o = 3;

/* Amount of 'x' pieces initially in each component. */
int initial_even_num_x = 4;
int initial_odd_num_x = 3;

/* Amount of pieces in the even component of the baord. */
int even_comp_size = 12;

/* Amount of pieces in the odd component of the baord. */
int odd_comp_size = 13;

/* Describes the initial piece positions in the even part of the board. */
char initial_even_component[12] =
{'o', 'o', 'o', '-', 'o', '-', '-', 'x', '-', 'x', 'x', 'x'};

/* Describes the initial piece positions in the even part of the board. */
char initial_odd_component[13] =
{'o', 'o', 'o', '-', '-', '-', '-', '-', '-', '-', 'x', 'x', 'x'};




/* BOARD TRANSFORMATION ARRAYS */

/* Describes a board transformation of keeping things the same. This is only
for convenience and clarity (and dramatic value). */
int even_identity_pos[12] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
int odd_identity_pos[13] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

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
  precompute_fact(fact_array, 25);
  gMoveToStringFunPtr = &MoveToString;
  gInitialPosition = GetInitialPosition();
  gCanonicalPosition = GetCanonicalPosition;
}

/* Return the hash value of the initial position. */
POSITION GetInitialPosition() {
  FFK_Board *initial_board = malloc(sizeof(FFK_Board));
  for (int i = 0; i < even_comp_size; i++) 
  initial_board->even_component[i] = initial_even_component[i];
  for (int i = 0; i < odd_comp_size; i++) 
  initial_board->odd_component[i] = initial_odd_component[i];
  return Hash(initial_board);
}

// FIXME: evaluate funcs don't need 'turn' anymore
/* Return a linked list of possible moves. */
MOVELIST *GenerateMoves(POSITION hash) {
  FFK_Board *newboard = Unhash(hash);
  MOVELIST *moves = NULL;

  /* - = 0; o = 1; x = 2; */
  for (int i = 0; i < even_comp_size; i++) {
    if (i != 4 && i != 9) {
      evaluateEven(i, i - 2, &moves, newboard->even_component);
      evaluateEven(i, i + 3, &moves, newboard->even_component);
    }
    if (i != 2 && i != 7) {
      evaluateEven(i, i + 2, &moves, newboard->even_component);
      evaluateEven(i, i - 3, &moves, newboard->even_component);
    }
  }

  for (int j = 0; j < odd_comp_size; j++) {
    if (j != 0 && j != 5 && j != 10) {
      evaluateOdd(j, j + 2, &moves, newboard->odd_component);
      evaluateOdd(j, j - 3, &moves, newboard->odd_component);
    }
    if (j != 2 && j != 7 && j != 12) {
      evaluateOdd(j, j - 2, &moves, newboard->odd_component);
      evaluateOdd(j, j + 3, &moves, newboard->odd_component);
    }
  }

  return moves;
}

/* Return the resulting position from making 'move' on 'position'. */
POSITION DoMove(POSITION hash, MOVE move) {
  // Get current board
  FFK_Board* board = Unhash(hash);

  // Get move information (from, to = indices in board[25])
  int from, to;
  unhashMove(move, &from, &to);

  if (from < 12) {
    // Piece to be moved is in board->even_component[12] which means
    // that 'to' is as well; mutate board accordingly
    char piece = board->even_component[from];
    board->even_component[from] = '-';
    board->even_component[to] = piece;
  } else {
    // Piece to be moved is in board->odd_component[13] which means
    // that 'to' is as well
    from = from - 12;
    to = to - 12;

    // Mutate board accordingly
    char piece = board->odd_component[from];
    board->odd_component[from] = '-';
    board->odd_component[to] = piece;
  }

  // Compute hash for post-move
  POSITION result = Hash(board);
  free(board);
  return result;
}

/* Symmetry Handling: Return the canonical position. */
POSITION GetCanonicalPosition(POSITION position) {
  POSITION* symmetries = malloc(sizeof(POSITION)*8);
  FFK_Board* board = Unhash(position);
  POSITION canonical = 0;

  symmetries[0] = position; // identity
  rotate(board);
  symmetries[1] = Hash(board); // r
  rotate(board);
  symmetries[2] = Hash(board); // r^2
  rotate(board);
  symmetries[3] = Hash(board); // r^3
  rotate(board);
  flip(board);
  symmetries[4] = Hash(board); // f
  rotate(board);
  symmetries[5] = Hash(board); // fr (frfr ong deadass)
  rotate(board);
  symmetries[6] = Hash(board); // fr^2
  rotate(board);
  symmetries[7] = Hash(board); // fr^3
  
  for (int i = 0; i < 8; i++) {
    if (symmetries[i] > canonical) {
      canonical = symmetries[i];
    }
  }

  free(symmetries);
  free(board);

  return canonical;
}

/* Return lose, win, tie, or undecided. See src/core/types.h
for the value enum definition. */
VALUE Primitive(POSITION position) {
  FFK_Board *board = Unhash(position);
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

// FIXME: These shouldn't care about whose turn it is -- they should assume it is
//        always 'x's turn

void evaluateEven(int currPos, int newPos, MOVELIST **moves, char *even_component) {
  if (newPos < 0 || newPos >= even_comp_size) {
    return;
  }
  int currElem = convertChar(even_component[currPos]);
  int newElem = convertChar(even_component[newPos]);
  if (currElem == 2 && newElem == 0) {
    *moves = CreateMovelistNode(hashMove((12 - 1) - currPos, (12 - 1) - newPos), *moves);
  }
}

void evaluateOdd(int currPos, int newPos, MOVELIST **moves, char *odd_component) {
  if (newPos < 0 || newPos >= odd_comp_size) {
    return;
  }
  int currElem = convertChar(odd_component[currPos]);
  int newElem = convertChar(odd_component[newPos]);
  if (currElem == 2 && newElem == 0) {
    *moves = CreateMovelistNode(hashMove((25 - 1) - currPos, (25 - 1) - newPos), *moves);
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
  permute(board->even_component, even_turn_pos, even_comp_size);
  permute(board->odd_component, odd_turn_pos, odd_comp_size);
}

/* Transforms the board by flipping it horizontally (a reflection 
about the y-axis). */
void flip(FFK_Board* board) {
  permute(board->even_component, even_flip_pos, even_comp_size);
  permute(board->odd_component, odd_flip_pos, odd_comp_size);
}

/* Performs an in-place rearrangement of the contents of TARGET as outlined
by a transformation array MAP, assuming they are both the same SIZE. Does not 
allocate memory and is linear in the size of the array being permuted. */
void permute(char* target, int* map, int size) {
  register int count = 0;
  register int fromIndex = 0;
  register int toIndex = map[fromIndex];
  char temp;
  char displacedChar = target[fromIndex];
  while (count < size) {
    temp = displacedChar;
    displacedChar = target[toIndex];
    target[toIndex] = temp;
    fromIndex = toIndex;
    toIndex = map[toIndex];
    count += 1;
  }
}




/* BOARD HASHING FUNCTIONS */

/* Returns the index that IN would have in the alphabetical ordering of all
possible strings composed of the same kinds and amounts of characters. */
POSITION Hash(FFK_Board* board) {
  // Collect the number of slots <-- 12 + 13 = 25 slots
  POSITION even_hash = compute_hash(board->even_component, even_comp_size, initial_even_num_x, initial_even_num_o);
  POSITION odd_hash = compute_hash(board->odd_component, odd_comp_size, initial_odd_num_x, initial_odd_num_o);
  
  // odd_hash * max_even_hash + even_hash <= 1.2 billion positions
  return odd_hash * max_even_hash + even_hash;
}

POSITION compute_hash(char* board_component, int slots, int num_x, int num_o) {
  POSITION total = 0;
  for (int i = 0; i < slots; i++) {
    int t1 = rearrangements(slots - 1, num_x, num_o);
    int t2 = t1 + rearrangements(slots - 1, num_x, num_o - 1);
    if (board_component[i] == 'o') {
      total += t1;
      num_o -= 1;
    } else if (board_component[i] == 'x') {
      total += t2;
      num_x -= 1;
    }
    slots -= 1;
  }
  return total;
}

/* The inverse process of hash. */
FFK_Board* Unhash(POSITION in) {
  FFK_Board* newBoard = (FFK_Board *) malloc(sizeof(FFK_Board));
  POSITION even_hash = in % max_even_hash;
  POSITION odd_hash = floor(in/max_even_hash);

  compute_unhash(newBoard->even_component, even_hash, even_comp_size, initial_even_num_x, initial_even_num_o);
  compute_unhash(newBoard->odd_component, odd_hash, odd_comp_size, initial_odd_num_x, initial_odd_num_o);

  return newBoard;
}

void compute_unhash(char* board_component, POSITION in, int slots, int num_x, int num_o) {
  int idx = 0;
  while (slots > 0) {
    int t1 = rearrangements(slots - 1, num_x, num_o);
    int t2 = t1 + rearrangements(slots - 1, num_x, num_o - 1);
    if (in < t1) {
      board_component[idx] = '-';
    } else if (in < t2) {
      board_component[idx] = 'o';
      in -= t1;
      num_o -= 1;
    } else {
      board_component[idx] = 'x';
      in -= t2;
      num_x -= 1;
    }
    slots -= 1;
  }
}

/* Returns the amount of ways to put X x's and O o's into SLOTS slots. */
POSITION rearrangements(int slots, int x, int o) {
  if ((slots < 0) || (x < 0) || (o < 0) || (slots < (o + x))) return 0;
  // Essentially returns the number of ways to place the -'s, x's, and o's in the slots
  return fact_array[slots]/(fact_array[x]*fact_array[o]*fact_array[slots - x - o]);
}

void precompute_fact(int fact_array[], int limit) {
  fact_array[0] = 1;
  for (int i = 1; i <= limit; i++) {
    fact_array[i] = i * fact_array[i-1];
  }
}

/* Returns the factorial of N, which is N(N-1)(N-2)...(2)(1). */
int factorial(int n) {
  int total = 1;
  for (int k = 1; k <= n; k++) total *= k;
  return total;
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
  return board->even_component[11] == 'o'
  && board->even_component[10] == 'o'
  && board->even_component[9] == 'o'
  && board->even_component[7] == 'o'
  && board->odd_component[12] == 'o'
  && board->odd_component[11] == 'o'
  && board->odd_component[10] == 'o';
}

/* If there are no moves left to be made, then the game is a tie. */
BOOLEAN isTie(FFK_Board* board) {
  MOVELIST *possible_moves = GenerateMoves(Hash(board));
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
  // 0b<base 25 newPos, base 25 oldPos>
  return 25*newPos + oldPos;
}

/* Obtains the start and destination index of a piece in the connected
component of the board it belongs to and whose turn it is based on
a unique hash for a game state graph edge. */
void unhashMove(MOVE mv, int *oldPos, int *newPos) {
  // 0b<base 25 newPos, base 25 oldPos>
  *oldPos = mv % 25;
  mv = floor(mv/25);
  *newPos = mv % 25;
}

/* Returns the opposite of a piece (i.e. swaps 'x' <-> 'o'). */
char opposite(char piece) {
  if (piece == 'x') {
    return 'o';
  } else if (piece == 'o') {
    return 'x';
  }
  return piece;
}



/* TEXTUI FUNCTIONS */

/* This will print the board in the following format:

(o) (o) (o) (o) (o)
  \ / \ / \ / \ /  
  / \ / \ / \ / \  
(o) ( ) ( ) ( ) (o)
  \ / \ / \ / \ /  
  / \ / \ / \ / \  
( ) ( ) ( ) ( ) ( )
  \ / \ / \ / \ /  
  / \ / \ / \ / \  
(x) ( ) ( ) ( ) (x)
  \ / \ / \ / \ /  
  / \ / \ / \ / \  
(x) (x) (x) (x) (x)

*/
void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn) {
  FFK_Board* board = Unhash(position);
  char* fb = malloc(sizeof(char)*25);
  for (int i = 1; i < 26; i++) {
    if (i % 2 == 0) {
      char curr = board->even_component[(i/2)-1];
      fb[i-1] = (curr == '-') ? ' ' : curr;
    } else {
      char curr = board->odd_component[(i-1)/2];
      fb[i-1] = (curr == '-') ? ' ' : curr;
    }
  }
  if (usersTurn) printf("%s's move.\n", playerName);
  // *shamelessly puts on sunglasses
  for (int j = 0; j < 4; j++) printf("(%c) (%c) (%c) (%c) (%c) \n  \\ / \\ / \\ / \\ /   \n  / \\ / \\ / \\ / \\   \n", fb[5*j], fb[(5*j)+1], fb[(5*j)+2], fb[(5*j)+3], fb[(5*j)+4]);
  printf("(%c) (%c) (%c) (%c) (%c) \n", fb[20], fb[21], fb[22], fb[23], fb[24]);
  free(fb);
  free(board);
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
