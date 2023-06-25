/************************************************************************
**
** NAME:        mnotakto.c
**
** DESCRIPTION: Notakto
**
** AUTHOR:      Andrew Esteban
**
** DATE:        2023-03-17
**
************************************************************************/

#include <stdio.h>
#include "gamesman.h"

/* IMPORTANT GLOBAL VARIABLES */
CONST_STRING kAuthorName = "Andrew Esteban";
CONST_STRING kGameName = "Notakto"; //  use this spacing and case
CONST_STRING kDBName = "notakto"; // use this spacing and case
POSITION gNumberOfPositions; // TODO: Put your number of positions upper bound here.
POSITION gInitialPosition; // TODO: Put the hash value of the initial position.
BOOLEAN kPartizan = FALSE; // Is the game PARTIZAN i.e. given a board does each player have a different set of moves available to them?
BOOLEAN kTieIsPossible = FALSE; // Is a tie or draw possible?
BOOLEAN kLoopy = FALSE; // Is this game loopy?
BOOLEAN kSupportsSymmetries = FALSE; // TODO: Whether symmetries are supported (i.e. whether the GetCanonicalPosition is implemented)

/* Likely you do not have to change these. */
POSITION GetCanonicalPosition(POSITION);
STRING MoveToString(MOVE);
POSITION kBadPosition = -1;
BOOLEAN kDebugDetermineValue = FALSE;
void* gGameSpecificTclInit = NULL;

/* You do not have to change these for now. */
BOOLEAN kGameSpecificMenu = TRUE;
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

#define MIN_BOARDS 1
#define MAX_BOARDS 3

int numberOfBoards = 1;
int boardSize;


void GameSpecificMenu() {
  char inp;
	while (TRUE) {
		printf("\n\n\n");
		printf("        ----- Game-specific options for Game of Y -----\n\n");
		printf("        Enter a number of boards (1-3): ");
    char buff[4];
    fgets(buff, 4, stdin);
		inp = buff[0];
    int dim = ((int) inp) - 48;

    if (inp == 'b' || inp == 'B') {
      ;
    } else if (dim >= MIN_BOARDS && dim <= MAX_BOARDS) {
      numberOfBoards = dim;
    } else {
			printf("Invalid input.\n");
			continue;
    }
		break;
	}
}

/*********** BEGIN SOLVING FUNCIONS ***********/

/* Initialize any global variables or data structures needed before
solving or playing the game. */
void InitializeGame() {
  gCanonicalPosition = GetCanonicalPosition;
  gMoveToStringFunPtr = &MoveToString;

  boardSize = numberOfBoards * 9;

  // {char, min, max, char, min, max, ..., -1}
  int piecesArray[] = {'X', 0, 7 * numberOfBoards, ' ', 2 * numberOfBoards, boardSize, -1};
  gNumberOfPositions = generic_hash_init(boardSize, piecesArray, NULL, 1);

  char initialBoard[boardSize];

  for (int i = 0; i < boardSize; i++) {
    initialBoard[i] = ' ';
  }

  gInitialPosition = generic_hash_hash(initialBoard, 1);
}

/* Return the hash value of the initial position. */
POSITION GetInitialPosition() {
  return gInitialPosition;
}

BOOLEAN checkBoard(char board[], int boardNum) {
  int startIndex = boardNum * 9;

  if (board[startIndex] == 'X' && board[startIndex + 3] == 'X' && board[startIndex + 6] == 'X') {
    return TRUE;
  } else if (board[startIndex + 1] == 'X' && board[startIndex + 4] == 'X' && board[startIndex + 7] == 'X') {
    return TRUE;
  } else if (board[startIndex + 2] == 'X' && board[startIndex + 5] == 'X' && board[startIndex + 8] == 'X') {
    return TRUE;
  } else if (board[startIndex] == 'X' && board[startIndex + 1] == 'X' && board[startIndex + 2] == 'X') {
    return TRUE;
  } else if (board[startIndex + 3] == 'X' && board[startIndex + 4] == 'X' && board[startIndex + 5] == 'X') {
    return TRUE;
  } else if (board[startIndex + 6] == 'X' && board[startIndex + 7] == 'X' && board[startIndex + 8] == 'X') {
    return TRUE;
  } else if (board[startIndex] == 'X' && board[startIndex + 4] == 'X' && board[startIndex + 8] == 'X') {
    return TRUE;
  } else if (board[startIndex + 2] == 'X' && board[startIndex + 4] == 'X' && board[startIndex + 6] == 'X') {
    return TRUE;
  }

  return FALSE;
}

/* Return a linked list of moves. */
MOVELIST *GenerateMoves(POSITION position) {
  MOVELIST *moves = NULL;

  char board[boardSize];
  generic_hash_unhash(position, board);

  for (int i = 0; i < numberOfBoards; i++) {
    if (checkBoard(board, i) == TRUE) {
      continue;
    }

    for (int j = 0; j < 9; j++) {
      if (board[i * 9 + j] == ' ') {
        moves = CreateMovelistNode(i * 9 + j, moves);
      }
    }
  }

  return moves;
}

/* Return the position that results from making the 
input move on the input position. */
POSITION DoMove(POSITION position, MOVE move) {
  char board[boardSize];
  generic_hash_unhash(position, board);

  board[move] = 'X';

  return generic_hash_hash(board, 1);
}

/*****************************************************************
**  Determine whether the position is a primitive win,
**  primitive tie, primitive lose, or not primitive.
**  OUTPUTS: (VALUE) an enum; one of: (win, lose, tie, undecided)
**  See src/core/types.h for the value enum definition.
******************************************************************/
VALUE Primitive(POSITION position) {
  char board[boardSize];
  generic_hash_unhash(position, board);

  for (int i = 0; i < numberOfBoards; i++) {
    if (checkBoard(board, i) == FALSE) {
      return undecided;
    }
  }

  return win;
}

void rotateBoard(char* board, char* target) {
  /*
    0 1 2    6 3 0 
    3 4 5 => 7 4 1
    6 7 8    8 5 2
  */

  int indices[] = {6, 3, 0, 7, 4, 1, 8, 5, 2};

  for (int i = 0; i < 9; i++) {
    target[i] = board[indices[i]];
  }
}

void horizontalBoard(char* board, char* target) {
  /*
    0 1 2    6 7 8
    3 4 5 => 3 4 5
    6 7 8    0 1 2
  */

  int indices[] = {6, 7, 8, 3, 4, 5, 0, 1, 2};

  for (int i = 0; i < 9; i++) {
    target[i] = board[indices[i]];
  }
}

void verticalBoard(char* board, char* target) {
  /*
    0 1 2    2 1 0
    3 4 5 => 5 4 3
    6 7 8    8 7 6
  */

  int indices[] = {2, 1, 0, 5, 4, 3, 8, 7, 6};

  for (int i = 0; i < 9; i++) {
    target[i] = board[indices[i]];
  }
}

POSITION min(POSITION a, POSITION b) {
  return a < b ? a : b;
}

POSITION GetCanonicalPositionOne(POSITION position) {
  char options[12][9];
  generic_hash_unhash(position, options[0]);

  int player = generic_hash_turn(position);

  // 4 rotations
  for (int i = 1; i < 4; i++) {
    rotateBoard(options[i - 1], options[i]);
  }

  // 8 flips
  for (int i = 0; i < 4; i++) {
    horizontalBoard(options[i], options[i + 4]);
  }

  for (int i = 0; i < 4; i++) {
    verticalBoard(options[i], options[i + 8]);
  }

  POSITION best = generic_hash_hash(options[0], player);

  for (int i = 1; i < 12; i++) {
    best = min(best, generic_hash_hash(options[i], player));
  }

  return best;
}

POSITION GetCanonicalPositionTwo(POSITION position) {
  char options[2][18];
  char board[18];
  generic_hash_unhash(position, board);

  int player = generic_hash_turn(position);

  // (0, 1), (1, 0)
  int orders[6][3] = {{0, 1}, {1, 0}};

  char first[9];
  char second[9];

  memcpy(first, options[0], 9);
  memcpy(second, &options[0][9], 9);

  for (int i = 0; i < 2; i++) {
    memcpy(&options[i][0], &board[orders[i][0] * 9], 9);
    memcpy(&options[i][9], &board[orders[i][1] * 9], 9);
  }

  POSITION best = position;

  for (int i = 1; i < 6; i++) {
    best = min(best, generic_hash_hash(options[i], player));
  }

  return best;
}

POSITION GetCanonicalPositionThree(POSITION position) {
  char options[6][27];
  char board[27];
  generic_hash_unhash(position, board);

  int player = generic_hash_turn(position);

  // (0, 1, 2), (0, 2, 1), (1, 0, 2), (1, 2, 0), (2, 1, 0), (2, 0, 1)
  int orders[6][3] = {{0, 1, 2}, {0, 2, 1}, {1, 0, 2}, {1, 2, 0}, {2, 1, 0}, {2, 0, 1}};

  char first[9];
  char second[9];
  char third[9];

  memcpy(first, options[0], 9);
  memcpy(second, &options[0][9], 9);
  memcpy(third, &options[0][18], 9);

  for (int i = 0; i < 6; i++) {
    memcpy(&options[i][0], &board[orders[i][0] * 9], 9);
    memcpy(&options[i][9], &board[orders[i][1] * 9], 9);
    memcpy(&options[i][18], &board[orders[i][2] * 9], 9);
  }

  POSITION best = position;

  for (int i = 1; i < 6; i++) {
    best = min(best, generic_hash_hash(options[i], player));
  }

  return best;
}

/* Symmetry Handling: Return the canonical position. */
POSITION GetCanonicalPosition(POSITION position) {
  if (numberOfBoards == 1) {
    return GetCanonicalPositionOne(position);
  } else if (numberOfBoards == 2) {
    return GetCanonicalPositionTwo(position);
  } else if (numberOfBoards == 3) {
    return GetCanonicalPositionThree(position);
  }

  return position;
}

/*********** END SOLVING FUNCTIONS ***********/







/*********** BEGIN TEXTUI FUNCTIONS ***********/

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn) {
  (void)playerName;
  (void)usersTurn;
  char board[boardSize];
  generic_hash_unhash(position, board);

  /*
    X|X|X
    -+-+-
    X|X|X
    -+-+-
    X|X|X
  
  */

  printf("\n\t");

  for (int i = 0; i < numberOfBoards; i++) {
    printf("%c|%c|%c\t", board[(i * 9)], board[(i * 9) + 1], board[(i * 9) + 2]);
  }

  printf("\n\t");

  for (int i = 0; i < numberOfBoards; i++) {
    printf("-+-+-\t");
  }

  printf("\n\t");

  for (int i = 0; i < numberOfBoards; i++) {
    printf("%c|%c|%c\t", board[(i * 9) + 3], board[(i * 9) + 4], board[(i * 9) + 5]);
  }

  printf("\n\t");

  for (int i = 0; i < numberOfBoards; i++) {
    printf("-+-+-\t");
  }

  printf("\n\t");

  for (int i = 0; i < numberOfBoards; i++) {
    printf("%c|%c|%c\t", board[(i * 9) + 6], board[(i * 9) + 7], board[(i * 9) + 8]);
  }

  printf("\n\n");
}

void PrintComputersMove(MOVE computersMove, STRING computersName) {
  (void)computersName;
  PrintMove(computersMove);
}

USERINPUT GetAndPrintPlayersMove(POSITION position, MOVE *move, STRING playerName) {
  USERINPUT ret;
	do {
		printf("%8s's move [(u)ndo]/[<index>] :  ", playerName);
		ret = HandleDefaultTextInput(position, move, playerName);
		if (ret != Continue) {
			return ret;
    }
	} while (TRUE);
	return Continue;
}

BOOLEAN isNumeric(STRING input) {
   BOOLEAN result = TRUE;

   for (size_t i = 0; i < strlen(input); i++) {
      if (input[i] < 48 || input[i] > 48 + 9) {
        result = FALSE;
      }
   }

   return result;
}

/* Return whether the input text signifies a valid move. */
BOOLEAN ValidTextInput(STRING input) {
  int val = atoi(input);

  if (val < 0 || val >= boardSize || isNumeric(input) == FALSE) {
    return FALSE;
  }

  return TRUE;
}

/* Assume the text input signifies a valid move. Return
the move hash corresponding to the move. */
MOVE ConvertTextInputToMove(STRING input) {
  return atoi(input);
}

/* Return the string representation of the move. 
Ideally this matches with what the user is supposed to
type when they specify moves. */
STRING MoveToString(MOVE move) {
  char* res = (char*) SafeMalloc(3);

  sprintf(res, "%d", move);

  return res;
}

/* Basically just print the move. */
void PrintMove(MOVE move) {
  printf("[%d]", move);
}

/*********** END TEXTUI FUNCTIONS ***********/









/*********** BEGIN VARIANT FUNCTIONS ***********/

/* How many variants are you supporting? */
int NumberOfOptions() {
  return MAX_BOARDS - MIN_BOARDS + 1;
}

/* Return the current variant id. */
int getOption() {
  return numberOfBoards - 1;
}

/* The input is a variant id. This function sets any global variables
or data structures according to the variant specified by the variant id. */
void setOption(int option) {
  numberOfBoards = option + 1;
}

/*********** END VARIANT-RELATED FUNCTIONS ***********/







POSITION InteractStringToPosition(STRING board) {
  // Ignore the first 8 characters
  char realBoard[boardSize];

  int count = 0;

  for (int i = 0; i < boardSize; i++) {
    if (board[i + 8] == 'X') {
      realBoard[i] = 'X';
      count++;
    } else {
      realBoard[i] = ' ';
    }
  }

  return generic_hash_hash(realBoard, (count % 2) + 1);
}

STRING InteractPositionToString(POSITION position) {
  char board[boardSize];
  generic_hash_unhash(position, board);

  // R_A_0_0_TG-T-S--H
  STRING result = (STRING) SafeMalloc(9 + boardSize);
  result[0] = 'R';
  result[1] = '_';
  result[2] = 'C';
  result[3] = '_';
  result[4] = '0';
  result[5] = '_';
  result[6] = '0';
  result[7] = '_';

  for (int i = 0; i < boardSize; i++) {
    if (board[i] == 'X') {
      result[8 + i] = 'X';
    } else {
      result[8 + i] = '-';
    }
  }

  result[8 + boardSize] = '\0';

  return result;
}

STRING InteractMoveToString(POSITION position, MOVE move) {
  (void)position;
  STRING result = (STRING) SafeMalloc(8);

  sprintf(result, "A_-_%d", move);

  return result;
}