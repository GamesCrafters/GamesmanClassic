/*
 * Pylos
 * Eric Siroker
 * February 11, 2004
 */

#include "gamesman.h"
#include "hash.h"

#ifndef WORD_BIT
# if INT_MAX > 32767
#  define WORD_BIT 32
# else
#  if INT_MAX > 2147483647
#   define WORD_BIT 64
#  endif
# endif
#endif

#define BLANK_PIECE_LABEL '-'
#define BOARD_DIMENSION   3
#define BOARD_SPACE       " )          : "
#define BOARD_TITLE       " )  BOARD:  : "
#define DARK_PIECE_LABEL  'O'
#define LEGEND_SPACE      "         ( "
#define LEGEND_TITLE      "LEGEND:  ( "
#define LIGHT_PIECE_LABEL 'X'
#define MOVE_PROMPT       "%s's move : "
#define MOVES_IN_GROUP    WORD_BIT / CHAR_BIT
#define PIECES_LEFT_TITLE "PIECES LEFT: "

POSITION gInitialPosition       = 0;
POSITION gMinimalPosition       = 0;
POSITION gNumberOfPositions     = 0;
POSITION kBadPosition           = -1;
STRING   kDBName                = "pylos";
BOOLEAN  kDebugDetermineValue   = FALSE;
BOOLEAN  kDebugMenu             = FALSE;
STRING   kGameName              = "Pylos";
BOOLEAN  kGameSpecificMenu      = TRUE;
STRING   kHelpExample           = "\
         (  1   2   3 )          : - - -\n\
         (    4   5   )          :  - -\n\
LEGEND:  (  6   7   8 )  BOARD:  : - - -\n\
         (    9  10   )          :  - -\n\
         ( 11  12  13 )          : - - - (Player will Lose in 14)\n\n\
PIECES LEFT: XXXXXXX (7)\n\n\
                   Player's move : { 1 }\n\n\
         (  1   2   3 )          : X - -\n\
         (    4   5   )          :  - -\n\
LEGEND:  (  6   7   8 )  BOARD:  : - - -\n\
         (    9  10   )          :  - -\n\
         ( 11  12  13 )          : - - - (Computer will Win in 13)\n\n\
PIECES LEFT: OOOOOOO (7)\n\n\
                 Computer's move : 8\n\n\
         (  1   2   3 )          : X - -\n\
         (    4   5   )          :  - -\n\
LEGEND:  (  6   7   8 )  BOARD:  : - - O\n\
         (    9  10   )          :  - -\n\
         ( 11  12  13 )          : - - - (Player will Lose in 12)\n\n\
PIECES LEFT: XXXXXX (6)\n\n\
                   Player's move : { 2 }\n\n\
         (  1   2   3 )          : X X -\n\
         (    4   5   )          :  - -\n\
LEGEND:  (  6   7   8 )  BOARD:  : - - O\n\
         (    9  10   )          :  - -\n\
         ( 11  12  13 )          : - - - (Computer will Win in 11)\n\n\
PIECES LEFT: OOOOOO (6)\n\n\
                 Computer's move : 12\n\n\
         (  1   2   3 )          : X X -\n\
         (    4   5   )          :  - -\n\
LEGEND:  (  6   7   8 )  BOARD:  : - - O\n\
         (    9  10   )          :  - -\n\
         ( 11  12  13 )          : - O - (Player should Win in 11)\n\n\
PIECES LEFT: XXXXX (5)\n\n\
                   Player's move : { 6 }\n\n\
         (  1   2   3 )          : X X -\n\
         (    4   5   )          :  - -\n\
LEGEND:  (  6   7   8 )  BOARD:  : X - O\n\
         (    9  10   )          :  - -\n\
         ( 11  12  13 )          : - O - (Computer will Win in 9)\n\n\
PIECES LEFT: OOOOO (5)\n\n\
                 Computer's move : 3\n\n\
         (  1   2   3 )          : X X O\n\
         (    4   5   )          :  - -\n\
LEGEND:  (  6   7   8 )  BOARD:  : X - O\n\
         (    9  10   )          :  - -\n\
         ( 11  12  13 )          : - O - (Player should Win in 9)\n\n\
PIECES LEFT: XXXX (4)\n\n\
                   Player's move : { 7 1 }\n\n\
         (  1   2   3 )          : - X O\n\
         (    4   5   )          :  - -\n\
LEGEND:  (  6   7   8 )  BOARD:  : X X O\n\
         (    9  10   )          :  - -\n\
         ( 11  12  13 )          : - O - (Computer should Lose in 8)\n\n\
PIECES LEFT: OOOO (4)\n\n\
                 Computer's move : 1\n\n\
         (  1   2   3 )          : O X O\n\
         (    4   5   )          :  - -\n\
LEGEND:  (  6   7   8 )  BOARD:  : X X O\n\
         (    9  10   )          :  - -\n\
         ( 11  12  13 )          : - O - (Player should Win in 7)\n\n\
PIECES LEFT: XXXX (4)\n\n\
                   Player's move : { 4 }\n\n\
         (  1   2   3 )          : O X O\n\
         (    4   5   )          :  X -\n\
LEGEND:  (  6   7   8 )  BOARD:  : X X O\n\
         (    9  10   )          :  - -\n\
         ( 11  12  13 )          : - O - (Computer should Lose in 6)\n\n\
PIECES LEFT: OOO (3)\n\n\
                 Computer's move : 13\n\n\
         (  1   2   3 )          : O X O\n\
         (    4   5   )          :  X -\n\
LEGEND:  (  6   7   8 )  BOARD:  : X X O\n\
         (    9  10   )          :  - -\n\
         ( 11  12  13 )          : - O O (Player should Win in 5)\n\n\
PIECES LEFT: XXX (3)\n\n\
                   Player's move : { 5 }\n\n\
         (  1   2   3 )          : O X O\n\
         (    4   5   )          :  X X\n\
LEGEND:  (  6   7   8 )  BOARD:  : X X O\n\
         (    9  10   )          :  - -\n\
         ( 11  12  13 )          : - O O (Computer should Lose in 4)\n\n\
PIECES LEFT: OO (2)\n\n\
                 Computer's move : 10\n\n\
         (  1   2   3 )          : O X O\n\
         (    4   5   )          :  X X\n\
LEGEND:  (  6   7   8 )  BOARD:  : X X O\n\
         (    9  10   )          :  - O\n\
         ( 11  12  13 )          : - O O (Player should Win in 3)\n\n\
PIECES LEFT: XX (2)\n\n\
                   Player's move : { 11 }\n\n\
         (  1   2   3 )          : O X O\n\
         (    4   5   )          :  X X\n\
LEGEND:  (  6   7   8 )  BOARD:  : X X O\n\
         (    9  10   )          :  - O\n\
         ( 11  12  13 )          : X O O (Computer should Lose in 2)\n\n\
PIECES LEFT: O (1)\n\n\
                 Computer's move : 9\n\n\
         (  1   2   3 )          : O X O\n\
         (    4   5   )          :  X X\n\
LEGEND:  (  6   7   8 )  BOARD:  : X - O\n\
         (    9  10   )          :  O O\n\
         ( 11  12  13 )          : X O O (Player should Win in 1)\n\n\
PIECES LEFT: X (1)\n\n\
                   Player's move : { 7 }\n\n\
         (  1   2   3 )          : O X O\n\
         (    4   5   )          :  X X\n\
LEGEND:  (  6   7   8 )  BOARD:  : X X O\n\
         (    9  10   )          :  O O\n\
         ( 11  12  13 )          : X O O (Computer should Lose in 0)\n\n\
PIECES LEFT: (0)\n\n\n\
Excellent! You won!";
STRING   kHelpGraphicInterface  = "";
STRING   kHelpOnYourTurn        = "\
Place a piece on a blank spot. If a two-by-two square is formed, you must\n\
take back one of your pieces. You may not take back a piece that is being\n\
used to support another piece.";
STRING   kHelpReverseObjective  = "\
To use all of your pieces before your opponent does.";
STRING   kHelpStandardObjective = "\
To make your opponent use all of his or her pieces before you do.";
STRING   kHelpTextInterface     = "\
The board represents a bird's eye view of a pyramid. Pieces are placed\n\
building this pyramid. A piece can only be placed on a higher level of the\n\
pyramid if all four of its bases have piece on them. The legend indicates\n\
which vertical column action will be taken on. Typically the action will be\n\
adding a piece, but, if a two-by-two square is formed, a move can consist of\n\
two actions: an addition and a removal.";
STRING   kHelpTieOccursWhen     = "";
BOOLEAN  kLoopy                 = TRUE;
BOOLEAN  kPartizan              = TRUE;
BOOLEAN  kTieIsPossible         = FALSE;

typedef enum Direction Direction;
typedef struct MoveGroup MoveGroup;
typedef struct Options Options;
typedef enum Piece Piece;
typedef struct Pyramid Pyramid;

enum Direction {
  INVALID_DIRECTION = -1,
  LOWER_LEFT_DIRECTION,
  LOWER_RIGHT_DIRECTION,
  UPPER_LEFT_DIRECTION,
  UPPER_RIGHT_DIRECTION,
  NUMBER_OF_DIRECTIONS
};

struct MoveGroup {
  char moves[MOVES_IN_GROUP];
};

struct Options {
  BOOLEAN isStandardGame : 1;
  int boardDimension : WORD_BIT - 1;
};

enum Piece {
  BLANK_PIECE,
  LIGHT_PIECE,
  DARK_PIECE
};

struct Pyramid {
  char pieceLabel;
  Pyramid *bases[NUMBER_OF_DIRECTIONS], *parents[NUMBER_OF_DIRECTIONS];
};

int gBoardAddresses;
int gBoardDimension = BOARD_DIMENSION;
int gBoardPieces;
int gBoardSize;

char gPieceLabels[] = {
  BLANK_PIECE_LABEL,
  LIGHT_PIECE_LABEL,
  DARK_PIECE_LABEL
};

int CalculateDigits(int integer);
int CalculatePower(int base, int exponent);
int ConvertAddressToIndex(int address);
int ConvertPyramidIndex(int index, Direction direction);
int CountPieces(char board[], Piece piece);
Pyramid *CreatePyramid(Pyramid pyramids[], int dimension, int baseIndex,
                       Direction parentDirection, int parentIndex);
void CreatePyramidHashTable(Pyramid pyramids[], char board[]);
Direction FlipDirection(Direction direction);
BOOLEAN FormsSquare(Pyramid *pyramid, Piece piece);
BOOLEAN IsBlankPyramid(Pyramid *pyramid);
BOOLEAN IsSupportedPyramid(Pyramid *pyramid);
BOOLEAN IsSupportingPyramid(Pyramid *pyramid);
void MakeAddressable(char board[]);
void PrintBinary(unsigned int integer);
void PrintMoveGroup(MOVE move);
void PrintMovePrompt(char *name);

MOVE ConvertTextInputToMove(STRING input) {
  int index;
  char *move;
  MoveGroup moveGroup;

  move = strtok(input, " ");

  for (index = 0; index < MOVES_IN_GROUP; index++) {
    moveGroup.moves[index] = move == NULL ? 0 : atoi(move);
    move = strtok(NULL, " ");
  }

  return *(MOVE*)&moveGroup;
}

void DebugMenu() {}

POSITION DoMove(POSITION position, MOVE move) {
  char board[gBoardSize];
  MoveGroup moveGroup = *(MoveGroup*)&move;
  int index = ConvertAddressToIndex(moveGroup.moves[0]);
  Piece piece = whoseMove(position) == LIGHT_PIECE ? DARK_PIECE : LIGHT_PIECE;

  generic_unhash(position, board);

  /* Hardcoded */
  if (gBoardDimension == 3 && index == 4 &&
      board[9] != gPieceLabels[BLANK_PIECE] &&
      board[10] != gPieceLabels[BLANK_PIECE] &&
      board[11] != gPieceLabels[BLANK_PIECE] &&
      board[12] != gPieceLabels[BLANK_PIECE])
    index = 13;

  board[index] = gPieceLabels[whoseMove(position)];

  for (index = 1; index < MOVES_IN_GROUP; index++) {
    move = moveGroup.moves[index];

    if (move > 0)
      board[ConvertAddressToIndex(move)] = gPieceLabels[BLANK_PIECE];
  }

  return generic_hash(board, piece);
}

void GameSpecificMenu() {
  int dimension;

  while (TRUE) {
    printf("\n\t----- Game-specific options for %s -----\n\n", kGameName);
    printf("\td)\tChange board (D)imension (%d)\n", gBoardDimension);
    printf("\n\n\tb)\t(B)ack = Return to previous activity.\n");
    printf("\n\nSelect an option: ");

    switch(GetMyChar()) {
      case 'Q': case 'q':
        ExitStageRight();
      case 'H': case 'h':
        HelpMenus();
        break;
      case 'D': case 'd':
        printf("\nBoard dimension (%d): ", gBoardDimension);
        scanf("%d", &dimension);

        if (dimension > 0)
          gBoardDimension = dimension;

        break;
      case 'B': case 'b':
        return;
      default:
        BadMenuChoice();
        HitAnyKeyToContinue();
    }
  }
}

MOVELIST *GenerateMoves(POSITION position) {
  char board[gBoardSize];
  char center; /* Used for hardcoding */
  int index;
  MOVE move;
  MOVELIST* moveList = NULL;
  MoveGroup moveGroup;
  Pyramid *pyramid, pyramids[gBoardAddresses];

  generic_unhash(position, board);

  if (gBoardDimension == 3)
    center = board[4]; /* Used for hardcoding */

  MakeAddressable(board);
  CreatePyramidHashTable(pyramids, board);

  for (index = 0; index < MOVES_IN_GROUP; index++)
    moveGroup.moves[index] = 0;

  for (moveGroup.moves[0] = gBoardAddresses; moveGroup.moves[0] > 0;
       moveGroup.moves[0]--) {
    pyramid = &pyramids[moveGroup.moves[0] - 1];

    if (IsBlankPyramid(pyramid) && IsSupportedPyramid(pyramid) ||
        gBoardDimension == 3 && moveGroup.moves[0] == 7 && /* Hardcoded */
        board[4] == gPieceLabels[BLANK_PIECE]) {
      if (!(gBoardDimension == 3 && moveGroup.moves[0] == 7 && /* Hardcoded */
            center != gPieceLabels[BLANK_PIECE]) &&
          FormsSquare(pyramid, whoseMove(position))) {
        for (moveGroup.moves[1] = gBoardAddresses; moveGroup.moves[1] > 0;
             moveGroup.moves[1]--) {
          pyramid = &pyramids[moveGroup.moves[1] - 1];

          if (moveGroup.moves[1] == moveGroup.moves[0] ||
              pyramid->pieceLabel == gPieceLabels[whoseMove(position)] &&
              !IsSupportingPyramid(pyramid))
            moveList = CreateMovelistNode(*(MOVE*)&moveGroup, moveList);
        }
      }
      else
        moveList = CreateMovelistNode(*(MOVE*)&moveGroup, moveList);
    }
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
  return gInitialPosition;
}

int getOption() {
  Options options;

  options.boardDimension = gBoardDimension;
  options.isStandardGame = gStandardGame;

  return *(int*)&options;
}

void InitializeGame() {
  int dimension;

  gBoardAddresses = gBoardDimension * gBoardDimension + (gBoardDimension - 1) *
                    (gBoardDimension - 1);

  for (dimension = gBoardDimension, gBoardSize = 0; dimension > 0; dimension--)
    gBoardSize += dimension * dimension;

  gBoardPieces = gBoardSize / 2 + (gBoardSize % 2 == 1);

  freeAll();
  int piecesArray[] = {gPieceLabels[BLANK_PIECE], 0, gBoardSize,
                       gPieceLabels[LIGHT_PIECE], 0, gBoardPieces,
                       gPieceLabels[DARK_PIECE], 0, gBoardPieces, -1};

  gNumberOfPositions = generic_hash_init(gBoardSize, piecesArray, NULL);
}

int NumberOfOptions() {
  return INT_MAX;
}

VALUE Primitive(POSITION position) {
  char board[gBoardSize];
  Piece piece = whoseMove(position) == LIGHT_PIECE ? DARK_PIECE : LIGHT_PIECE;

  generic_unhash(position, board);

  if (board[gBoardSize - 1] == gPieceLabels[piece] ||
      CountPieces(board, whoseMove(position)) == gBoardPieces)
    return gStandardGame ? lose : win;
  else
    return undecided;
}

void PrintComputersMove(MOVE move, STRING name) {
  PrintMovePrompt(name);
  PrintMoveGroup(move);
  putchar('\n');
}

void PrintMove(MOVE move) {
  putchar('(');
  PrintMoveGroup(move);
  putchar(')');
}

void PrintPosition(POSITION position, STRING name, BOOLEAN isUsersTurn) {
  int address, column, count, digits = CalculateDigits(gBoardSize), offset;
  int pieces, row, rows = gBoardDimension * 2 - 1;
  int columns = strlen(LEGEND_SPACE) + rows * digits +
                strlen(BOARD_SPACE) + rows + 1; /* + 1 for '\n' */
  char board[gBoardSize], buffer[rows][columns], number[digits];
  char format[CalculateDigits(digits) + 2], *string; /* + 2 for %d */
  char pieceLabel = gPieceLabels[whoseMove(position)];

  generic_unhash(position, board);
  pieces = gBoardPieces - CountPieces(board, whoseMove(position));
  MakeAddressable(board);

  for (address = 1, count = 1, row = 0; row < rows; row++) {
    for (column = 0; column < columns; column++)
      buffer[row][column] = ' ';

    string = row != rows / 2 ? LEGEND_SPACE : LEGEND_TITLE;
    strncpy(buffer[row], string, strlen(string));
    offset = strlen(LEGEND_SPACE);
    sprintf(format, "%%%dd", digits);

    for (column = 0; column < rows; column++) {
      if (column % 2 == row % 2) {
        sprintf(number, format, count++);
        strncpy(buffer[row] + offset + column * digits, number, strlen(number));
      }
    }

    offset += rows * digits;
    string = row != rows / 2 ? BOARD_SPACE : BOARD_TITLE;
    strncpy(buffer[row] + offset, string, strlen(string));
    offset += strlen(BOARD_SPACE);

    for (column = 0; column < rows; column++)
      if (column % 2 == row % 2)
        buffer[row][offset + column] = board[ConvertAddressToIndex(address++)];

    buffer[row][columns - 1] = '\n';
  }

  buffer[rows - 1][columns - 1] = '\0';

  printf("\n%s %s\n\n%s", buffer, GetPrediction(position, name, isUsersTurn),
         PIECES_LEFT_TITLE);

  for (count = 0; count < pieces; count++)
    putchar(pieceLabel);

  if (pieces > 0)
    putchar(' ');

  printf("(%d)\n\n", pieces);
}

void setOption(int option) {
  Options options = *(Options*)&option;

  gBoardDimension = options.boardDimension;
  gStandardGame = options.isStandardGame;
}

BOOLEAN ValidTextInput(STRING input) {
  int index;
  char *move, inputBackup[strlen(input)];

  strcpy(inputBackup, input);
  move = strtok(inputBackup, " ");

  for (index = 0; index < MOVES_IN_GROUP && move != NULL; index++) {
    if (atoi(move) < 1)
      return FALSE;

    move = strtok(NULL, " ");
  }

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

int ConvertAddressToIndex(int address) {
  int column, dimension, row;

  address--;
  dimension = gBoardDimension * 2 - 1;
  column = address % dimension;
  row = address / dimension;

  return column < gBoardDimension ? row * gBoardDimension + column :
         gBoardDimension * gBoardDimension + row * (gBoardDimension - 1) +
         column - gBoardDimension;
}

int ConvertPyramidIndex(int index, Direction direction) {
  switch (direction) {
    case LOWER_LEFT_DIRECTION:
      return index + gBoardDimension - 1;
    case LOWER_RIGHT_DIRECTION:
      return index + gBoardDimension;
    case UPPER_LEFT_DIRECTION:
      return index - gBoardDimension;
    case UPPER_RIGHT_DIRECTION:
      return index - gBoardDimension + 1;
    default:
      return index;
  }
}

int CountPieces(char board[], Piece piece) {
  char pieceLabel = gPieceLabels[piece];
  int index, pieces;

  for (index = 0, pieces = 0; index < gBoardSize; index++)
    if (board[index] == pieceLabel)
      pieces++;

  return pieces;
}

Pyramid *CreatePyramid(Pyramid pyramids[], int dimension, int baseIndex,
                       Direction parentDirection, int parentIndex) {
  Direction direction;
  Pyramid *pyramid = NULL;

  if (dimension > 0) {
    pyramid = &pyramids[baseIndex];

    for (direction = 0; direction < NUMBER_OF_DIRECTIONS; direction++)
      if (pyramid->bases[direction] == NULL)
        pyramid->bases[direction] =
            CreatePyramid(pyramids, dimension - 1,
                          ConvertPyramidIndex(baseIndex, direction),
                          FlipDirection(direction), baseIndex);

    if (parentDirection != INVALID_DIRECTION)
      pyramid->parents[parentDirection] = &pyramids[parentIndex];
  }

  return pyramid;
}

void CreatePyramidHashTable(Pyramid pyramids[], char board[]) {
  Direction direction;
  int index;

  for (index = 0; index < gBoardAddresses; index++) {
    for (direction = 0; direction < NUMBER_OF_DIRECTIONS; direction++) {
      pyramids[index].bases[direction] = NULL;
      pyramids[index].parents[direction] = NULL;
    }
  }

  CreatePyramid(pyramids, gBoardDimension, gBoardAddresses / 2,
                INVALID_DIRECTION, gBoardAddresses / 2);

  for (index = 0; index < gBoardAddresses; index++)
    pyramids[index].pieceLabel = board[ConvertAddressToIndex(index + 1)];
}

Direction FlipDirection(Direction direction) {
  switch (direction) {
    case LOWER_LEFT_DIRECTION:
      return UPPER_RIGHT_DIRECTION;
    case LOWER_RIGHT_DIRECTION:
      return UPPER_LEFT_DIRECTION;
    case UPPER_LEFT_DIRECTION:
      return LOWER_RIGHT_DIRECTION;
    case UPPER_RIGHT_DIRECTION:
      return LOWER_LEFT_DIRECTION;
    default:
      return direction;
  }
}

BOOLEAN FormsSquare(Pyramid *pyramid, Piece piece) {
  Direction baseDirection, parentDirection;
  Pyramid *basePyramid, *parentPyramid;

  for (parentDirection = 0; parentDirection < NUMBER_OF_DIRECTIONS;
       parentDirection++) {
    parentPyramid = pyramid->parents[parentDirection];

    if (parentPyramid != NULL) {
      for (baseDirection = 0; baseDirection < NUMBER_OF_DIRECTIONS;
           baseDirection++) {
        basePyramid = parentPyramid->bases[baseDirection];

        if (basePyramid != pyramid &&
            basePyramid->pieceLabel != gPieceLabels[piece])
          break;

        if (baseDirection == NUMBER_OF_DIRECTIONS - 1)
          return TRUE;
      }
    }
  }

  return FALSE;
}

BOOLEAN IsBlankPyramid(Pyramid *pyramid) {
  return pyramid != NULL && pyramid->pieceLabel == gPieceLabels[BLANK_PIECE];
}

BOOLEAN IsSupportedPyramid(Pyramid *pyramid) {
  Direction direction;

  for (direction = 0; direction < NUMBER_OF_DIRECTIONS; direction++)
    if (IsBlankPyramid(pyramid->bases[direction]))
      return FALSE;

  return TRUE;
}

BOOLEAN IsSupportingPyramid(Pyramid *pyramid) {
  Direction direction;

  for (direction = 0; direction < NUMBER_OF_DIRECTIONS; direction++)
    if (pyramid->parents[direction] != NULL &&
        pyramid->parents[direction]->pieceLabel != gPieceLabels[BLANK_PIECE])
      return TRUE;

  return FALSE;
}

void MakeAddressable(char board[]) {
  /* Hardcoded */
  if (gBoardDimension == 3 &&
      board[9] != gPieceLabels[BLANK_PIECE] &&
      board[10] != gPieceLabels[BLANK_PIECE] &&
      board[11] != gPieceLabels[BLANK_PIECE] &&
      board[12] != gPieceLabels[BLANK_PIECE])
    board[4] = board[13];
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

void PrintMoveGroup(MOVE move) {
  int index;
  MoveGroup moveGroup = *(MoveGroup*)&move;

  for (index = 0; index < MOVES_IN_GROUP; index++) {
    if (moveGroup.moves[index] != 0) {
      if (index > 0)
        putchar(' ');

      printf("%d", moveGroup.moves[index]);
    }
  }
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
