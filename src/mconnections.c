/************************************************************************
**
** NAME:        mgameofy.c
**
** DESCRIPTION: Game Of Y
**
** AUTHOR:      Andrew Esteban
**
** DATE:        2023-03-02
**
************************************************************************/

#include <stdio.h>
#include "gamesman.h"

/* IMPORTANT GLOBAL VARIABLES */
STRING kAuthorName = "Andrew Esteban";
POSITION gNumberOfPositions; // Set in InitializeGame().
POSITION gInitialPosition; // Put the hash value of the initial position.
BOOLEAN kPartizan = TRUE; // Is the game PARTIZAN i.e. given a board does each player have a different set of moves available to them?
BOOLEAN kTieIsPossible = TRUE; // Is a tie or draw possible?
BOOLEAN kLoopy = FALSE; // Is this game loopy?
BOOLEAN kSupportsSymmetries = TRUE; // Whether symmetries are supported (i.e. whether the GetCanonicalPosition is implemented)

/* Do not change these. */
POSITION GetCanonicalPosition(POSITION);
STRING MoveToString(MOVE);
POSITION kBadPosition = -1;
STRING kGameName = "Game Of Y";
STRING kDBName = "gameofy";
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
STRING kHelpOnYourTurn = "On your turn, place the piece of your colour \
(either white or black) onto the board, using a letter (a, b, c,..) to signify \
the row and the number of spaces from the left for the column. For example, \
on the board \n\na\t   -\nb\t  - -\nc\t - - -\nd\t- - - -\n\ncalling a0 will \
place a piece on the top space like so: \n\na\t   w\nb\t  - -\nc\t - - -\nd\t- - - -\n";
STRING kHelpStandardObjective = "The goal of the game is to form a connected \
component within the graph using pieces from your colour that joins the three \
edges of the triangle together (corner spaces count as touching the two edges they join).\n";
STRING kHelpReverseObjective = "";
STRING kHelpTieOccursWhen = /* Should follow 'A Tie occurs when... */ "there \
are no more empty spaces and neither player has formed a Y joining all three edges.";
STRING kHelpExample = "";

/* You don't have to change this. */
void DebugMenu() {}
/* Ignore this function. */
void SetTclCGameSpecificOptions(int theOptions[]) {}
/* Do not worry about this yet because you will only be supporting 1 variant for now. */
void GameSpecificMenu() {}




#define DEFAULT_DIMENSION 4
#define MAX_BOARD_SIZE 128

int calculateBoardSize(int dimensions);
int BOARD_SIZE; // Declare now but calculate in InitializeGame()
int BOARD_DIMENSION;


/*********** BEGIN SOLVING FUNCIONS ***********/

/* Add a hashing function and unhashing function, if needed. */

int vcfg(int pieces[]) {
  return pieces[0] == pieces[1] || pieces[0] - 1 == pieces[1];
}

/* Initialize any global variables or data structures needed before
solving or playing the game. */
void InitializeGame() {
  gCanonicalPosition = GetCanonicalPosition;
  gMoveToStringFunPtr = &MoveToString;

  BOARD_DIMENSION = DEFAULT_DIMENSION;
  BOARD_SIZE = calculateBoardSize(BOARD_DIMENSION);

  // {char, min, max, char, min, max, ..., -1}
  int piecesArray[] = {'w', 0, BOARD_SIZE / 2, 'b', 0, BOARD_SIZE / 2, '-', 0, BOARD_SIZE, -1};
  gNumberOfPositions = generic_hash_init(BOARD_SIZE, piecesArray, &vcfg, 0);

  char* initialBoard = (char*) SafeMalloc(BOARD_SIZE * sizeof(char));

  for (int i = 0; i < BOARD_SIZE; i++) {
    initialBoard[i] = '-';
  }

  gInitialPosition = generic_hash_hash(initialBoard, 1);
}

/* Return the hash value of the initial position. */
POSITION GetInitialPosition() {
  return gInitialPosition;
}

/* Return a linked list of moves. */
MOVELIST *GenerateMoves(POSITION position) {
  MOVELIST *moves = NULL;

  char newBoard[BOARD_SIZE];

  generic_hash_unhash(position, newBoard);

  for (int i = BOARD_SIZE - 1; i >= 0; i--) {
    if (newBoard[i] == '-') {
      moves = CreateMovelistNode(i, moves);
    }
  }

  return moves;
}

/* Return the position that results from making the 
input move on the input position. */
POSITION DoMove(POSITION position, MOVE move) {
  char newBoard[BOARD_SIZE];
  generic_hash_unhash(position, newBoard);

  int nonBlankCount = 0;

  for (int i = 0; i < BOARD_SIZE; i++) {
    if (newBoard[i] != '-') {
      nonBlankCount++;
    }
  }

  if (nonBlankCount % 2 == 0) {
    newBoard[move] = 'w';
    return generic_hash_hash(newBoard, 2);
  } else {
    newBoard[move] = 'b';
    return generic_hash_hash(newBoard, 1);
  }
}

char** convertBoardToMatrix(char* board) {
  char** matrix = (char**) SafeMalloc(BOARD_DIMENSION * sizeof(char*));

  for (int i = 0 ; i < BOARD_DIMENSION; i++) {
    matrix[i] = (char*) SafeMalloc((i + 1) * sizeof(char));
  }

  int boardIndex = 0;

  for (int i = 0; i < BOARD_DIMENSION; i++) {
    for (int j = 0; j < i + 1; j++) {
      matrix[i][j] = board[boardIndex++];
    }
  }

  return matrix;
}

void convertMatrixToBoard(char** matrix, char* target) {
  int boardIndex = 0;

  for (int i = 0; i < BOARD_DIMENSION; i++) {
    for (int j = 0; j < i + 1; j++) {
      target[boardIndex++] = matrix[i][j];
    }
  }
}

void freeMatrix(char** matrix) {
  for (int i = 0; i < BOARD_DIMENSION; i++) {
    SafeFree(matrix[i]);
  }

  SafeFree(matrix);
}

int convertLocToIndex(int row, int col) {
  int index = 0;

  for (int i = 0; i <= row; i++) {
    index += i;
  }

  return index + col;
}

void recurY(char** matrix, int row, int col, BOOLEAN* touches, BOOLEAN* visited, char startPiece) {
  // touches: {left edge, right edge, bottom edge}

  // Check if visited already
  if (visited[convertLocToIndex(row, col)] == TRUE) {
    return;
  }

  // Update visited
  visited[convertLocToIndex(row, col)] = TRUE;

  // Check if piece matches colour
  if (matrix[row][col] != startPiece) {
    return;
  }

  // Update touches
  if (col == 0) {
    touches[0] = TRUE;
  } 
  if (col == row) {
    touches[1] = TRUE;
  }
  if (row == BOARD_DIMENSION - 1) {
    touches[2] = TRUE;
  }

  // Go to neighbours if same colour
  if (col > 0 && row > 0) {
    recurY(matrix, row - 1, col - 1, touches, visited, startPiece);
  }

  if (col < row && row > 0) {
    recurY(matrix, row - 1, col, touches, visited, startPiece);
  }

  if (col > 0) {
    recurY(matrix, row, col - 1, touches, visited, startPiece);
  }

  if (col < row) {
    recurY(matrix, row, col + 1, touches, visited, startPiece);
  }

  if (col > 0 && row < BOARD_DIMENSION - 1) {
    recurY(matrix, row + 1, col, touches, visited, startPiece);
  }

  if (row < BOARD_DIMENSION - 1) {
    recurY(matrix, row + 1, col + 1, touches, visited, startPiece);
  }
}

BOOLEAN hasYStartLoc(char** matrix, int row, int col) {

  if (matrix[row][col] == '-') {
    return FALSE;
  }

  BOOLEAN visited[BOARD_SIZE];

  for (int i = 0; i < BOARD_SIZE; i++) {
    visited[i] = FALSE;
  }

  BOOLEAN touches[3] = {FALSE, FALSE, FALSE};

  recurY(matrix, row, col, touches, visited, matrix[row][col]);

  return touches[0] == TRUE && touches[1] == TRUE && touches[2] == TRUE;
}

BOOLEAN hasY(char** matrix) {
  for (int i = 0; i < BOARD_DIMENSION; i++) {
    if (hasYStartLoc(matrix, i, 0)) {
      return TRUE;
    }
  }

  return FALSE;
}

/* Return lose, win, tie, or undecided. See src/core/types.h
for the value enum definition. */
VALUE Primitive(POSITION position) {
  char newBoard[BOARD_SIZE];
  generic_hash_unhash(position, newBoard);

  char** matrix = convertBoardToMatrix(newBoard);

  BOOLEAN gameOver = hasY(matrix);

  freeMatrix(matrix);

  if (gameOver == TRUE) {
    return lose;
  }

  for (int i = 0; i < BOARD_SIZE; i++) {
    if (newBoard[i] == '-') {
      return undecided;
    }
  }

  return tie;
}

POSITION min(POSITION a, POSITION b) {
  if (a < b) {
    return a;
  } else {
    return b;
  }
}

void rotateBoard(char* board, char* target) {
  char** matrix = convertBoardToMatrix(board);
  char** rotated = convertBoardToMatrix(board);

  // First element of each row forms the new bottom row
  for (int i = 0; i < BOARD_DIMENSION; i++) {
    for (int j = i; j < BOARD_DIMENSION; j++) {
      rotated[BOARD_DIMENSION - 1 - i][j - i] = matrix[j][i];
    }
  }

  convertMatrixToBoard(rotated, target);

  freeMatrix(matrix);
  freeMatrix(rotated);
}

void flipBoard(char* board, char* target) {
  char** matrix = convertBoardToMatrix(board);

  for (int i = 0; i < BOARD_DIMENSION; i++) {
    for (int j = 0; j <= i / 2; j++) {
      char temp = matrix[i][j];
      int reflectIndex = i - j;
      matrix[i][j] = matrix[i][reflectIndex];
      matrix[i][reflectIndex] = temp;
    }
  }

  convertMatrixToBoard(matrix, target);

  freeMatrix(matrix);
}

/* Symmetry Handling: Return the canonical position. */
POSITION GetCanonicalPosition(POSITION position) {
  char cousinBoards[6][BOARD_SIZE];

  generic_hash_unhash(position, cousinBoards[0]);

  for (int i = 0; i < 2; i++) {
    rotateBoard(cousinBoards[i], cousinBoards[i + 1]);
  }

  for (int i = 0; i < 3; i++) {
    flipBoard(cousinBoards[i], cousinBoards[i + 3]);
  }

  // Figure out player
  int count = 0;

  for (int i = 0; i < BOARD_SIZE; i++) {
    if (cousinBoards[0][i] != '-') {
      count++;
    }
  }

  int player = count % 2 == 0 ? 0 : 1;

  POSITION best = position;

  for (int i = 1; i < 6; i++) {
    POSITION option = generic_hash_hash(cousinBoards[i], player);
    best = min(best, option);
  }

  return best;
}

/*********** END SOLVING FUNCTIONS ***********/







/*********** BEGIN TEXTUI FUNCTIONS ***********/

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn) {
  /* THIS ONE IS MOST IMPORTANT FOR YOUR DEBUGGING */
  /* YOUR CODE HERE */

  // TODO: Consider this
  /*
        Ʌ
       | |
       Ʌ Ʌ
      | | |
      Ʌ Ʌ Ʌ
     | | | |
     Ʌ Ʌ Ʌ Ʌ
    | | | | |
     V V V V 
  */

  char board[BOARD_SIZE];
  generic_hash_unhash(position, board);

  char** matrix = convertBoardToMatrix(board);

  for (int row = 0; row < BOARD_DIMENSION; row++) {
    printf("%c\t", (char) (row + 97));
    
    for (int i = 0; i < BOARD_DIMENSION - 1 - row; i++) {
      printf(" ");
    }

    for (int col = 0; col <= row; col++) {
      printf("%c ", matrix[row][col]);
    }
    printf("\n");
  }

  
  printf("%s\n\n", GetPrediction(position,playerName,usersTurn));

  freeMatrix(matrix);
}

void PrintComputersMove(MOVE computersMove, STRING computersName) {
  // TODO: Make function for turning move into row + col
  int row = 0;
  int col = 0;
  int current = 0;

  while (current != computersMove) {
    if (col >= row) {
      col = 0;
      row++;
    } else {
      col++;
    }

    current++;
  }

  printf("%s's move: [%c%d]\n", computersName, (char) (row + 97), col);
}

USERINPUT GetAndPrintPlayersMove(POSITION position, MOVE *move, STRING playerName) {
  USERINPUT ret;
	do {
		printf("%8s's move [(u)ndo]/[<row><col>] :  ", playerName);
		ret = HandleDefaultTextInput(position, move, playerName);
		if (ret != Continue) {
			return ret;
        }
	} while (TRUE);
	return Continue;
}

/* Return whether the input text signifies a valid move. */
BOOLEAN ValidTextInput(STRING input) {
  if (input[0] == '\0' || input[1] == '\0') {
    return FALSE;
  }

  // Ensure first letter is within row bounds
  if ((int) input[0] < 97 || (int) input[0] > 122) {
    return FALSE;
  }

  // Ensure second digit is within column bounds
  if ((int) input[1] < 48 || (int) input[1] > 57) {
    return FALSE;
  }

  return TRUE;
}

/* Assume the text input signifies a valid move. Return
the move hash corresponding to the move. */
MOVE ConvertTextInputToMove(STRING input) {

  int row = ((int) input[0]) - 97;
  int col = ((int) input[1]) - 48;

  MOVE index = convertLocToIndex(row, col);

  return index;
}

/* Return the string representation of the move. 
Ideally this matches with what the user is supposed to
type in. */
STRING MoveToString(MOVE move) {
  int row = 0;
  int col = 0;
  int current = 0;

  while (current != move) {
    if (col >= row) {
      col = 0;
      row++;
    } else {
      col++;
    }

    current++;
  }

  char *res;

  asprintf(&res, "[%c%d]", (char) (row + 97), col);

  return res;
  
}

/* Basically just print the move. */
void PrintMove(MOVE move) {
  STRING str = MoveToString(move);
	printf("%s", str);
	SafeFree(str);
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

/*
Added functions
*/
int fact(int n) {
  if (n <= 1) {
    return 1;
  } else {
    return n * fact(n - 1);
  }
}

int choose(int n, int r) {
  return fact(n) / (fact(n - r) * fact(r));
}


int calculateBoardSize(int dimensions) {
  int size = 0;

  for (int i = 0; i < dimensions + 1; i++) {
    size += i;
  }

  return size;
}