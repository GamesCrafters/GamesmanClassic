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

#include "gamesman.h"

/* IMPORTANT GLOBAL VARIABLES */
CONST_STRING kAuthorName = "Andrew Esteban";
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
CONST_STRING kGameName = "Game of Y";
CONST_STRING kDBName = "gameofy";
BOOLEAN kDebugDetermineValue = FALSE;
void* gGameSpecificTclInit = NULL;

/* You don't have to change these for now. */
BOOLEAN kGameSpecificMenu = TRUE;
BOOLEAN kDebugMenu = FALSE;

/* These variables are not needed for solving but if you have time 
after you're done solving the game you should initialize them 
with something helpful. */
CONST_STRING kHelpGraphicInterface = "";
CONST_STRING kHelpTextInterface = "";
CONST_STRING kHelpOnYourTurn = "On your turn, place the piece of your colour \
(either white or black) onto the board, using a letter (a, b, c,..) to signify \
the row and the number of spaces from the left for the column. For example, \
on the board \n\na\t   -\nb\t  - -\nc\t - - -\nd\t- - - -\n\ncalling a0 will \
place a piece on the top space like so: \n\na\t   w\nb\t  - -\nc\t - - -\nd\t- - - -\n";
CONST_STRING kHelpStandardObjective = "The goal of the game is to form a connected \
component within the graph using pieces from your colour that joins the three \
edges of the triangle together (corner spaces count as touching the two edges they join).\n";
CONST_STRING kHelpReverseObjective = "";
CONST_STRING kHelpTieOccursWhen = /* Should follow 'A Tie occurs when... */ "there \
are no more empty spaces and neither player has formed a Y joining all three edges.";
CONST_STRING kHelpExample = "";

/* You don't have to change this. */
void DebugMenu() {}
/* Ignore this function. */
void SetTclCGameSpecificOptions(int theOptions[]) {
  (void)theOptions;
}


#define MIN_DIMENSION 4
#define MAX_DIMENSION 7
#define DEFULT_DIMENSION 4

// Declare helper functions
int calculateboardSize(int dimensions);
int countUsed(char* board);

int vcfg(int* pieces);

char** convertBoardToMatrix(char* board);
void convertMatrixToBoard(char** matrix, char* target);
void freeMatrix(char** matrix);
int convertLocToIndex(int row, int col);

POSITION min(POSITION a, POSITION b);

void recurY(char** matrix, int row, int col, BOOLEAN* touches, BOOLEAN* visited, char startPiece);
BOOLEAN hasYStartLoc(char** matrix, int row, int col);
BOOLEAN hasY(char** matrix);

void rotateBoard(char* board, char* target);
void flipBoard(char* board, char* target);

int boardSize; // Declare now but calculate in InitializeGame()
int boardDimension = DEFULT_DIMENSION;
BOOLEAN misere = FALSE;

void GameSpecificMenu() {
  char inp;
	while (TRUE) {
		printf("\n\n\n");
		printf("        ----- Game-specific options for Game of Y -----\n\n");
		printf("        Enter a board dimension (4-7): ");
    char buff[4];
    fgets(buff, 4, stdin);
		inp = buff[0];
    int dim = ((int) inp) - 48;

    if (inp == 'b' || inp == 'B') {
      ;
    } else if (dim >= MIN_DIMENSION && dim <= MAX_DIMENSION) {
      boardDimension = dim;
    } else {
			printf("Invalid input.\n");
			continue;
    }
		break;
	}

	while (TRUE) {
		printf("        Play misere? (y/n): ");
		char buff[4];
    fgets(buff, 4, stdin);
		inp = buff[0];

    if (inp == 'b' || inp == 'B') {
      ;
    } else if (inp == 'y' || inp == 'Y') {
      misere = TRUE;
    } else if (inp == 'n' || inp == 'N') {
      misere = FALSE;
    } else {
			printf("Invalid input.\n");
			continue;
    }
		break;
	}

  fflush(stdin);
}

/*********** BEGIN SOLVING FUNCIONS ***********/

/* Initialize any global variables or data structures needed before
solving or playing the game. */
void InitializeGame() {
  gCanonicalPosition = GetCanonicalPosition;
  gMoveToStringFunPtr = &MoveToString;

  boardSize = calculateboardSize(boardDimension);

  // {char, min, max, char, min, max, ..., -1}
  if (boardSize % 2 == 0) {
    int piecesArray[] = {'w', 0, boardSize / 2, 'b', 0, boardSize / 2, '-', 0, boardSize, -1};
    gNumberOfPositions = generic_hash_init(boardSize, piecesArray, &vcfg, 1);    
  } else {
    int piecesArray[] = {'w', 0, (boardSize / 2) + 1, 'b', 0, boardSize / 2, '-', 0, boardSize, -1};
    gNumberOfPositions = generic_hash_init(boardSize, piecesArray, &vcfg, 1);
  }

  char* initialBoard = (char*) SafeMalloc(boardSize * sizeof(char));

  for (int i = 0; i < boardSize; i++) {
    initialBoard[i] = '-';
  }

  gInitialPosition = generic_hash_hash(initialBoard, 1);

  SafeFree(initialBoard);
}

/* Return the hash value of the initial position. */
POSITION GetInitialPosition() {
  return gInitialPosition;
}

/* Return a linked list of moves. */
MOVELIST *GenerateMoves(POSITION position) {
  MOVELIST *moves = NULL;

  char newBoard[boardSize];

  generic_hash_unhash(position, newBoard);

  for (int i = boardSize - 1; i >= 0; i--) {
    if (newBoard[i] == '-') {
      moves = CreateMovelistNode(i, moves);
    }
  }

  return moves;
}

/* Return the position that results from making the 
input move on the input position. */
POSITION DoMove(POSITION position, MOVE move) {
  char newBoard[boardSize];
  generic_hash_unhash(position, newBoard);

  int nonBlankCount = countUsed(newBoard);

  if (nonBlankCount % 2 == 0) {
    newBoard[move] = 'w';
    return generic_hash_hash(newBoard, 1);
  } else {
    newBoard[move] = 'b';
    return generic_hash_hash(newBoard, 1);
  }
}

/* Return lose, win, tie, or undecided. See src/core/types.h
for the value enum definition. */
VALUE Primitive(POSITION position) {
  char newBoard[boardSize];
  generic_hash_unhash(position, newBoard);

  char** matrix = convertBoardToMatrix(newBoard);

  BOOLEAN gameOver = hasY(matrix);

  freeMatrix(matrix);

  if (gameOver == TRUE) {
    return misere ? win : lose;
  }

  for (int i = 0; i < boardSize; i++) {
    if (newBoard[i] == '-') {
      return undecided;
    }
  }

  return tie;
}

/* Symmetry Handling: Return the canonical position. */
POSITION GetCanonicalPosition(POSITION position) {
  char cousinBoards[6][boardSize];

  generic_hash_unhash(position, cousinBoards[0]);

  for (int i = 0; i < 2; i++) {
    rotateBoard(cousinBoards[i], cousinBoards[i + 1]);
  }

  for (int i = 0; i < 3; i++) {
    flipBoard(cousinBoards[i], cousinBoards[i + 3]);
  }

  POSITION best = position;

  for (int i = 1; i < 6; i++) {
    POSITION option = generic_hash_hash(cousinBoards[i], 1);
    best = min(best, option);
  }

  return best;
}

/*********** END SOLVING FUNCTIONS ***********/

/*********** HELPER FUNCTIONS      ***********/

int vcfg(int pieces[]) {
  return pieces[0] == pieces[1] || pieces[0] - 1 == pieces[1];
}

int countUsed(char* board) {
  int nonBlankCount = 0;

  for (int i = 0; i < boardSize; i++) {
    if (board[i] != '-') {
      nonBlankCount++;
    }
  }

  return nonBlankCount;
}

void flipBoard(char* board, char* target) {
  char** matrix = convertBoardToMatrix(board);

  for (int i = 0; i < boardDimension; i++) {
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

char** convertBoardToMatrix(char* board) {
  char** matrix = (char**) SafeMalloc(boardDimension * sizeof(char*));

  for (int i = 0 ; i < boardDimension; i++) {
    matrix[i] = (char*) SafeMalloc((i + 1) * sizeof(char));
  }

  int boardIndex = 0;

  for (int i = 0; i < boardDimension; i++) {
    for (int j = 0; j < i + 1; j++) {
      matrix[i][j] = board[boardIndex++];
    }
  }

  return matrix;
}

void convertMatrixToBoard(char** matrix, char* target) {
  int boardIndex = 0;

  for (int i = 0; i < boardDimension; i++) {
    for (int j = 0; j < i + 1; j++) {
      target[boardIndex++] = matrix[i][j];
    }
  }
}

void freeMatrix(char** matrix) {
  for (int i = 0; i < boardDimension; i++) {
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
  if (row == boardDimension - 1) {
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

  if (col > 0 && row < boardDimension - 1) {
    recurY(matrix, row + 1, col, touches, visited, startPiece);
  }

  if (row < boardDimension - 1) {
    recurY(matrix, row + 1, col + 1, touches, visited, startPiece);
  }
}

BOOLEAN hasYStartLoc(char** matrix, int row, int col) {

  if (matrix[row][col] == '-') {
    return FALSE;
  }

  BOOLEAN visited[boardSize];

  for (int i = 0; i < boardSize; i++) {
    visited[i] = FALSE;
  }

  BOOLEAN touches[3] = {FALSE, FALSE, FALSE};

  recurY(matrix, row, col, touches, visited, matrix[row][col]);

  return touches[0] == TRUE && touches[1] == TRUE && touches[2] == TRUE;
}

BOOLEAN hasY(char** matrix) {
  for (int i = 0; i < boardDimension; i++) {
    if (matrix[i][0] != '-' && hasYStartLoc(matrix, i, 0)) {
      return TRUE;
    }
  }

  return FALSE;
}

POSITION min(POSITION a, POSITION b) {
  return a < b ? a : b;
}

void rotateBoard(char* board, char* target) {
  char** matrix = convertBoardToMatrix(board);
  char** rotated = convertBoardToMatrix(board);

  // First element of each row forms the new bottom row
  for (int i = 0; i < boardDimension; i++) {
    for (int j = i; j < boardDimension; j++) {
      rotated[boardDimension - 1 - i][j - i] = matrix[j][i];
    }
  }

  convertMatrixToBoard(rotated, target);

  freeMatrix(matrix);
  freeMatrix(rotated);
}

int calculateboardSize(int dimensions) {
  int size = 0;

  for (int i = 0; i < dimensions + 1; i++) {
    size += i;
  }

  return size;
}
/*********************************************/





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

  char board[boardSize];
  generic_hash_unhash(position, board);

  char** matrix = convertBoardToMatrix(board);

  for (int row = 0; row < boardDimension; row++) {
    printf("%c\t", (char) (row + 97));
    
    for (int i = 0; i < boardDimension - 1 - row; i++) {
      printf(" ");
    }

    for (int col = 0; col <= row; col++) {
      printf("%c ", matrix[row][col]);
    }

    if (row < boardDimension - 1) {
      printf("\n");
    }
  }

  
  printf("\t%s\n\n", GetPrediction(position,playerName,usersTurn));

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

  STRING res = (STRING) SafeMalloc( 3 );
  sprintf(res, "[%c%d]", (char) (row + 97), col);

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

/* How many variants are you supporting? */
int NumberOfOptions() {
  return 2 * (MAX_DIMENSION + 1 - MIN_DIMENSION);
}

/* Return the current variant id (which is 0 in this case since
for now you're only thinking about one variant). */
int getOption() {
  // 4, 5, 6, 7, 4m, 5m, 6m, 7m
  if (!misere) {
    return boardDimension - MIN_DIMENSION;
  } else {
    return boardDimension - MIN_DIMENSION + (MAX_DIMENSION + 1 - MIN_DIMENSION);
  }
}

/* The input is a variant id. This function sets any global variables
or data structures according to the variant specified by the variant id. */
void setOption(int option) {
  if (option < MAX_DIMENSION + 1 - MIN_DIMENSION) {
    boardDimension = option + MIN_DIMENSION;
    misere = FALSE;
  } else {
    boardDimension = option + MIN_DIMENSION - (MAX_DIMENSION + 1 - MIN_DIMENSION);
    misere = TRUE;
  }
}

/*********** END VARIANT-RELATED FUNCTIONS ***********/






/********* AUTOGUI FUNCTIONS **********/
POSITION InteractStringToPosition(STRING board) {
  // Ignore the first 8 characters
  char realBoard[boardSize];

  for (int i = 0; i < boardSize; i++) {
    if (board[i + 8] == 'W') {
      realBoard[i] = 'w';
    } else if (board[i + 8] == 'B') {
      realBoard[i] = 'b';
    } else {
      realBoard[i] = '-';
    }
  }

  return generic_hash_hash(realBoard, 1);
}

STRING InteractPositionToString(POSITION position) {
  /* YOUR CODE HERE LATER BUT NOT NOW */
  char board[boardSize];
  generic_hash_unhash(position, board);

  // R_A_0_0_TG-T-S--H
  STRING result = (STRING) SafeMalloc(9 + boardSize);
  result[0] = 'R';
  result[1] = '_';
  result[2] = countUsed(board) % 2 == 0 ? 'A' : 'B';
  result[3] = '_';
  result[4] = '0';
  result[5] = '_';
  result[6] = '0';
  result[7] = '_';

  for (int i = 0; i < boardSize; i++) {
    if (board[i] == 'w') {
      result[8 + i] = 'W';
    } else if (board[i] == 'b') {
      result[8 + i] = 'B';
    } else {
      result[8 + i] = '-';
    }
  }

  result[8 + boardSize] = '\0';

  return result;
}

STRING InteractMoveToString(POSITION position, MOVE move) {
  (void)position;
  STRING result = (STRING) SafeMalloc(10);

  sprintf(result, "A_-_%d_x", move);

  return result;
}