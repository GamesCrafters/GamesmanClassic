/*
 * Pylos
 * Eric Siroker
 * February 11, 2004
 */

#include "gamesman.h"
#include "hash.h"

#define BLANK_PIECE_LABEL '-'
#define BOARD_DIMENSION   2
#define BOARD_SIZE        BOARD_DIMENSION * BOARD_DIMENSION +\
                          (BOARD_DIMENSION - 1) * (BOARD_DIMENSION - 1)
#define BOARD_SPACE       " )          : "
#define BOARD_TITLE       " )  BOARD:  : "
#define DARK_PIECE_LABEL  'O'
#define LEGEND_SPACE      "         ( "
#define LEGEND_TITLE      "LEGEND:  ( "
#define LIGHT_PIECE_LABEL 'X'
#define MOVE_PROMPT       "%s's move : "
#define MOVE_SIZE         8
#define MOVES_IN_GROUP    WORD_BIT / MOVE_SIZE

POSITION gInitialPosition       = 0;
POSITION gMinimalPosition       = 0;
POSITION gNumberOfPositions     = 0;
POSITION kBadPosition           = -1;
STRING   kDBName                = "pylos";
BOOLEAN  kDebugDetermineValue   = FALSE;
BOOLEAN  kDebugMenu             = FALSE;
STRING   kGameName              = "Pylos";
BOOLEAN  kGameSpecificMenu      = TRUE;
STRING   kHelpExample           = "Not written yet.";
STRING   kHelpGraphicInterface  = "Not written yet.";
STRING   kHelpOnYourTurn        = "Not written yet.";
STRING   kHelpReverseObjective  = "Not written yet.";
STRING   kHelpStandardObjective = "Not written yet.";
STRING   kHelpTextInterface     = "Not written yet.";
STRING   kHelpTieOccursWhen     = "Not written yet.";
BOOLEAN  kLoopy                 = TRUE;
BOOLEAN  kPartizan              = TRUE;
BOOLEAN  kTieIsPossible         = FALSE;

typedef struct MoveGroup MoveGroup;
typedef enum Piece Piece;
typedef struct Pyramid Pyramid;

struct MoveGroup {
  MOVE moves[MOVES_IN_GROUP];
};

enum Piece {
  BLANK_PIECE,
  LIGHT_PIECE,
  DARK_PIECE
};

struct Pyramid {
  char piece;
  Pyramid *lowerLeft, *lowerRight, *upperLeft, *upperRight;
};

int gBoardDimension = BOARD_DIMENSION;
int gBoardSize = BOARD_SIZE;

char gPieceLabels[] = {
  BLANK_PIECE_LABEL,
  LIGHT_PIECE_LABEL,
  DARK_PIECE_LABEL
};

int CalculateDigits(int integer);
int CalculatePower(int base, int exponent);
Pyramid *CreatePyramid(Pyramid pyramids[], int dimension, int index);
void CreatePyramidHashTable(Pyramid pyramids[], char board[]);
BOOLEAN IsBlankPyramid(Pyramid *pyramid);
void PrintBinary(unsigned int integer);
void PrintMovePrompt(char *name);

MOVE ConvertTextInputToMove(STRING input) {
  MoveGroup moveGroup;

  moveGroup.moves[0] = atoi(input);

  return *(MOVE*)&moveGroup;
}

void DebugMenu() {}

POSITION DoMove(POSITION position, MOVE move) {
  char board[gBoardSize];
  MoveGroup moveGroup = *(MoveGroup*)&move;
  int index;

  generic_unhash(position, board);
  board[moveGroup.moves[0] - 1] = gPieceLabels[whoseMove(position)];

  return generic_hash(board, whoseMove(position) == 1 ? 2 : 1);
}

void GameSpecificMenu() {}

MOVELIST *GenerateMoves(POSITION position) {
  char board[gBoardSize];
  MOVE move;
  MOVELIST* moveList = NULL;
  MoveGroup moveGroup;

  generic_unhash(position, board);

  for (move = gBoardSize; move > 0; move--) {
    moveGroup.moves[0] = move;
    moveList = CreateMovelistNode(*(MOVE*)&moveGroup, moveList);
  }

  return moveList;
}

USERINPUT GetAndPrintPlayersMove(POSITION position, MOVE *move, STRING name) {
  USERINPUT userInput = Continue;

  while (userInput == Continue) {
    PrintMovePrompt(name);
    userInput = HandleDefaultTextInput(position, move, name);
  }

  return userInput;
}

POSITION GetInitialPosition() {
  return 0;
}

int getOption() {
  return gBoardDimension;
}

void InitializeGame() {
  int dimension, pieces;

  for (dimension = 1, pieces = 0; dimension <= gBoardDimension; dimension++)
    pieces += dimension * dimension;

  pieces /= 2;

  int piecesArray[] = {gPieceLabels[BLANK_PIECE], 0, gBoardSize,
                       gPieceLabels[LIGHT_PIECE], 0, pieces,
                       gPieceLabels[DARK_PIECE], 0, pieces, -1};

  freeAll();
  gNumberOfPositions = generic_hash_init(gBoardSize, piecesArray, NULL);
}

int NumberOfOptions() {
  return INT_MAX;
}

VALUE Primitive(POSITION position) {
  char board[gBoardSize];
  Piece piece = whoseMove(position) == LIGHT_PIECE ? DARK_PIECE : LIGHT_PIECE;

  generic_unhash(position, board);

  if (board[gBoardSize / 2] == gPieceLabels[piece])
    return gStandardGame ? lose : win;
  else
    return undecided;
}

void PrintComputersMove(MOVE move, STRING name) {
  PrintMovePrompt(name);
  PrintMove(move);
  putchar('\n');
}

void PrintMove(MOVE move) {
  MoveGroup moveGroup = *(MoveGroup*)&move;

  printf("%d", moveGroup.moves[0]);
}

void PrintPosition(POSITION position, STRING name, BOOLEAN isUsersTurn) {
  int address, column, digits = CalculateDigits(gBoardSize), index, offset;
  int row, rows = gBoardDimension * 2 - 1;
  int columns = strlen(LEGEND_SPACE) + rows * digits +
                strlen(BOARD_SPACE) + rows + 1; /* + 1 for '\n' */
  char board[gBoardSize], buffer[rows][columns], number[digits];
  char format[CalculateDigits(digits) + 2], *string; /* + 2 for %d */

  generic_unhash(position, board);

  for (address = 1, index = 0, row = 0; row < rows; row++) {
    for (column = 0; column < columns; column++)
      buffer[row][column] = ' ';

    string = row != rows / 2 ? LEGEND_SPACE : LEGEND_TITLE;
    strncpy(buffer[row], string, strlen(string));
    offset = strlen(LEGEND_SPACE);
    sprintf(format, "%%%dd", digits);

    for (column = 0; column < rows; column++) {
      if (column % 2 == row % 2) {
        sprintf(number, format, address++);
        strncpy(buffer[row] + offset + column * digits, number, strlen(number));
      }
    }

    offset += rows * digits;
    string = row != rows / 2 ? BOARD_SPACE : BOARD_TITLE;
    strncpy(buffer[row] + offset, string, strlen(string));
    offset += strlen(BOARD_SPACE);

    for (column = 0; column < rows; column++)
      if (column % 2 == row % 2)
        buffer[row][offset + column] = board[index++];

    buffer[row][columns - 1] = '\n';
  }

  buffer[rows - 1][columns - 1] = '\0';

  printf("\n%s %s\n\n", buffer, GetPrediction(position, name, isUsersTurn));
}

void setOption(int option) {
  gBoardDimension = option;
}

BOOLEAN ValidTextInput(STRING input) {
  return TRUE;
}

int CalculateDigits(int integer) {
  int digits;

  for (digits = 1; (integer /= 10) > 0; digits++);

  return digits;
}

int CalculatePower(int base, int exponent) {
  int power;

  for (power = 1; exponent > 0; exponent--)
    power *= base;

  return power;
}

Pyramid *CreatePyramid(Pyramid pyramids[], int dimension, int index) {
  Pyramid *pyramid = NULL;

  if (dimension > 0) {
    pyramid = &pyramids[index];
    pyramid->upperLeft = CreatePyramid(pyramids, dimension - 1,
                                       index - gBoardDimension);
    pyramid->upperRight = CreatePyramid(pyramids, dimension - 1,
                                        index - gBoardDimension + 1);
    pyramid->lowerLeft = CreatePyramid(pyramids, dimension - 1,
                                       index + gBoardDimension - 1);
    pyramid->lowerRight = CreatePyramid(pyramids, dimension - 1,
                                        index + gBoardDimension);
  }

  return pyramid;
}

void CreatePyramidHashTable(Pyramid pyramids[], char board[]) {
  int index;

  CreatePyramid(pyramids, gBoardDimension, gBoardSize / 2);

  for (index = 0; index < gBoardSize; index++)
    pyramids[index].piece = board[index];
}

BOOLEAN IsBlankPyramid(Pyramid *pyramid) {
  return pyramid != NULL && pyramid->piece == gPieceLabels[BLANK_PIECE];
}

void PrintBinary(unsigned int integer) {
  int power;

  for (power = WORD_BIT - 1; power >= 0; power--) {
    putchar(integer & CalculatePower(2, power) ? '1' : '0');

    if (power % CHAR_BIT == 0)
      putchar(' ');
  }

  putchar('\n');
}

void PrintMovePrompt(char *name) {
  int promptLength = strlen(LEGEND_SPACE) + (gBoardDimension * 2 - 1) *
                     CalculateDigits(gBoardSize) + strlen(BOARD_SPACE);
  char format[CalculateDigits(promptLength) + 2]; /* + 2 for %d */
  char prompt[promptLength];

  sprintf(format, "%%%ds", promptLength);
  sprintf(prompt, MOVE_PROMPT, name);

  printf(format, prompt);
}

void *gGameSpecificTclInit = NULL;
