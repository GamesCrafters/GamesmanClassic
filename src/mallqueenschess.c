/************************************************************************
**
** NAME:        mallqueenschess.c
**
** DESCRIPTION: All Queens Chess
**
** AUTHOR:      Andrew Esteban
**
** DATE:        2023-04-08
**
************************************************************************/

#include <stdio.h>
#include "gamesman.h"

/* IMPORTANT GLOBAL VARIABLES */
CONST_STRING kAuthorName = "Andrew Esteban";
CONST_STRING kGameName = "All Queens Chess"; //  use this spacing and case
CONST_STRING kDBName = "allqueenschess"; // use this spacing and case
POSITION gNumberOfPositions; // Put your number of positions upper bound here.
POSITION gInitialPosition; // Put the hash value of the initial position.
BOOLEAN kPartizan = TRUE; // Is the game PARTIZAN i.e. given a board does each player have a different set of moves available to them?
BOOLEAN kTieIsPossible = FALSE; // Is a tie or draw possible?
BOOLEAN kLoopy = TRUE; // Is this game loopy?
BOOLEAN kSupportsSymmetries = TRUE; // Whether symmetries are supported (i.e. whether the GetCanonicalPosition is implemented)

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
void SetTclCGameSpecificOptions(int theOptions[]) {}
/* Do not worry about this yet because you will only be supporting 1 variant for now. */
void GameSpecificMenu() {}


UNDOMOVELIST *GenerateUndoMovesToTier(POSITION position, TIER tier);
POSITION UndoMove(POSITION position, UNDOMOVE undoMove);



/*********** BEGIN SOLVING FUNCIONS ***********/

#define BOARD_SIZE 25
#define BOARD_DIMS 5
#define WIN_COUNT 4
POSITION numBoardArrangements = 4805077200;


/* Initialize any global variables or data structures needed before
solving or playing the game. */

TIERLIST* getTierChildren(TIER tier) {
  return CreateTierlistNode(0, NULL);
}

POSITION numberOfTierPositions(TIER tier) {
  return gNumberOfPositions;
}

POSITION fact(int n) {
     if (n <= 1) return 1;
	 POSITION prod = 1;
	 for (int i = 2; i <= n; i++)
		 prod *= i;
     return prod;
}

// calculates the number of combinations
unsigned long long getNumPos(int boardsize, int numX, int numO) {
    long long temp = 1;
    int numB = boardsize - numX - numO;

    // Do fact(boardsize) / fact(numx) / fact(numo) / fact(numb) without overflowing
    // fact(boardsize) is too big to store in a unsigned long long
    // so this for loop represents temp = fact(boardsize) / fact(numb)
	if (numB >= numX && numB >= numO) {
		for (int i = numB + 1; i <= boardsize; i++)
        	temp *= i;
    	return temp / fact(numX) / fact(numO);
	} else if (numX >= numB && numX >= numO) {
		for (int i = numX + 1; i <= boardsize; i++)
        	temp *= i;
    	return temp / fact(numB) / fact(numO);
	} else {
		for (int i = numO + 1; i <= boardsize; i++)
        	temp *= i;
    	return temp / fact(numB) / fact(numX);
	}
}

// Pre-calculate the number of combinations and store in combinations array
// Combinations can be retrieved using combinations[boardsize][numx][numo]
unsigned long long combinations[26][7][7]; // 2400*8bytes
void combinationsInit() {
    for (int boardsize=0; boardsize<=25; boardsize++)
        for (int numx=0; numx<=6; numx++)
            for (int numo=0; numo<=6; numo++)
                combinations[boardsize][numx][numo] = getNumPos(boardsize, numx, numo);
}

unsigned long long hashIt(char *board, char turn) {
    int numx = 6, numo = 6;
    unsigned long long sum = 0;
    for (int i = BOARD_SIZE - 1; i > 0; i--) { // no need to calculate i == 0
        switch (board[i]) {
            case 'W':
                numx--;
                break;
            case 'B':
                if (numx > 0) sum += combinations[i][numx-1][numo];
                numo--;
                break;
            case '-':
                if (numx > 0) sum += combinations[i][numx-1][numo];
                if (numo > 0) sum += combinations[i][numx][numo-1];
                break;
        }
    }
    if (turn == 'B') {
        return sum + numBoardArrangements;
    }
    return sum;
}

void unhashIt(POSITION pos, char *board, char *turn) {
    if (pos < numBoardArrangements) {
        (*turn) = 'W';
    } else {
        (*turn) = 'B';
        pos -= numBoardArrangements;
    }
    int numx = 6, numo = 6;
    POSITION o1, o2;
    for (int i = BOARD_SIZE - 1; i >= 0; i--) {
        o1 = (numx > 0) ? combinations[i][numx-1][numo] : 0;
        o2 = o1 + ((numo > 0) ? combinations[i][numx][numo-1] : 0);
        if (pos >= o2) {
            board[i] = '-';
            pos -= o2;
        }
        else if (pos >= o1) {
            if (numo > 0) {
                board[i] = 'B';
                numo--;
            }
            else
                board[i] = '-';
            pos -= o1;
        }
        else {
            if (numx > 0) {
                board[i] = 'W';
                numx--;
            }
            else if (numo > 0) {
                board[i] = 'B';
                numo--;
            }
            else
                board[i] = '-';
        }
    }
}

void InitializeGame() {
  combinationsInit();
  gCanonicalPosition = GetCanonicalPosition;
  gMoveToStringFunPtr = &MoveToString;
  gNumberOfPositions = 9610154400;

  char initialBoard[BOARD_SIZE + 1] = "WBWBW-----B---W-----BWBWB\0";

  gInitialPosition = hashIt(initialBoard, 'W');

  gTierChildrenFunPtr = &getTierChildren; // returns a linked list of child tiers of a given tier
  gNumberOfTierPositionsFunPtr = &numberOfTierPositions; // this returns the number of tierpositions in a given tier
  gInitialTierPosition = gInitialPosition; 
  kSupportsTierGamesman = TRUE;
  kExclusivelyTierGamesman = TRUE;
  gInitialTier = 0;

  gUnDoMoveFunPtr = &UndoMove;
  gGenerateUndoMovesToTierFunPtr = &GenerateUndoMovesToTier;
}

/* Return the hash value of the initial position. */
POSITION GetInitialPosition() {
  return gInitialPosition;
}

/* Return a linked list of moves. */
MOVELIST *GenerateMoves(POSITION position) {
  MOVELIST *moves = NULL;

  char board[BOARD_SIZE];
  char turn;
  unhashIt(position, board, &turn);

  for (int i = 0; i < BOARD_SIZE; i++) {
    if ((turn == 'W' && board[i] == 'W') || (turn == 'B' && board[i] == 'B')) {
      int originRow = i / BOARD_DIMS;
      int originCol = i % BOARD_DIMS;
      int origin = i;

      // Left
      for (int col = originCol - 1; col >= 0; col--) {
        if (board[originRow * BOARD_DIMS + col] == '-') {
          int targetRow = originRow;
          int targetCol = col;

          int target = targetRow * BOARD_DIMS + targetCol;

          moves = CreateMovelistNode(origin * 100 + target, moves);
        } else {
          break;
        }
      }

      // Right
      for (int col = originCol + 1; col < BOARD_DIMS; col++) {
        if (board[originRow * BOARD_DIMS + col] == '-') {
          int targetRow = originRow;
          int targetCol = col;

          int target = targetRow * BOARD_DIMS + targetCol;

          moves = CreateMovelistNode(origin * 100 + target, moves);
        } else {
          break;
        }
      }

      // Up
      for (int row = originRow - 1; row >= 0; row--) {
        if (board[row * BOARD_DIMS + originCol] == '-') {
          int targetRow = row;
          int targetCol = originCol;

          int target = targetRow * BOARD_DIMS + targetCol;

          moves = CreateMovelistNode(origin * 100 + target, moves);
        } else {
          break;
        }
      }

      // Down
      for (int row = originRow + 1; row < BOARD_DIMS; row++) {
        if (board[row * BOARD_DIMS + originCol] == '-') {
          int targetRow = row;
          int targetCol = originCol;

          int target = targetRow * BOARD_DIMS + targetCol;

          moves = CreateMovelistNode(origin * 100 + target, moves);
        } else {
          break;
        }
      }

      // Left-Up
      if (originRow > 0 && originCol > 0) {
        int row = originRow - 1;
        int col = originCol - 1;

        while (row >= 0 && col >= 0) {
          if (board[row * BOARD_DIMS + col] == '-') {
            int target = row * BOARD_DIMS + col;
            moves = CreateMovelistNode(origin * 100 + target, moves);

            row--;
            col--;
          } else {
            break;
          }
        }
      }

      // Left-Down
      if (originRow < BOARD_DIMS - 1 && originCol > 0) {
        int row = originRow + 1;
        int col = originCol - 1;

        while (row < BOARD_DIMS && col >= 0) {
          if (board[row * BOARD_DIMS + col] == '-') {
            int target = row * BOARD_DIMS + col;
            moves = CreateMovelistNode(origin * 100 + target, moves);

            row++;
            col--;
          } else {
            break;
          }
        }
      }

      // Right-Up
      if (originRow > 0 && originCol < BOARD_DIMS) {
        int row = originRow - 1;
        int col = originCol + 1;

        while (row >= 0 && col < BOARD_DIMS) {
          if (board[row * BOARD_DIMS + col] == '-') {
            int target = row * BOARD_DIMS + col;
            moves = CreateMovelistNode(origin * 100 + target, moves);

            row--;
            col++;
          } else {
            break;
          }
        }
      }

      // Right-Down
      if (originRow < BOARD_DIMS && originCol < BOARD_DIMS) {
        int row = originRow + 1;
        int col = originCol + 1;

        while (row < BOARD_DIMS && col < BOARD_DIMS) {
          if (board[row * BOARD_DIMS + col] == '-') {
            int target = row * BOARD_DIMS + col;
            moves = CreateMovelistNode(origin * 100 + target, moves);

            row++;
            col++;
          } else {
            break;
          }
        }
      }
    }
  }

  return moves;
}

/* Return the position that results from making the 
input move on the input position. */
POSITION DoMove(POSITION position, MOVE move) {
  char board[BOARD_SIZE];
  char turn;

  unhashIt(position, board, &turn);

  int origin = move / 100;
  int target = move % 100;

  board[target] = board[origin];
  board[origin] = '-';

  return hashIt(board, (turn == 'W') ? 'B' : 'W');
}

/*****************************************************************
**  Determine whether the position is a primitive win,
**  primitive tie, primitive lose, or not primitive.
**  OUTPUTS: (VALUE) an enum; one of: (win, lose, tie, undecided)
**  See src/core/types.h for the value enum definition.
******************************************************************/
VALUE Primitive(POSITION position) {
  char board[BOARD_SIZE];
  char turn;

  unhashIt(position, board, &turn);

  char piece;

  // Vertical
  int i = 0;
  for (i = 10; i < 15; i++) {
    piece = board[i];
    if (piece != '-') {
      if (board[i - 5] == piece && board[i + 5] == piece) {
        if (board[i - 10] == piece || board[i + 10] == piece) {
          return lose;
        }
      }
    }
  }

  // Horizontal
  for (i = 2; i < 25; i += 5) {
    piece = board[i];
    if (piece != '-') {
      if (board[i - 1] == piece && board[i + 1] == piece) {
        if (board[i - 2] == piece || board[i + 2] == piece) {
          return lose;
        }
      }
    }
  }

  piece = board[12]; 
  if (piece != '-') {
    // Antidiagonal
    if (board[6] == piece && board[18] == piece) {
      if (board[0] == piece || board[24] == piece) {
        return lose;
      }
    }
    // Maindiagonal
    if (board[8] == piece && board[16] == piece) {
      if (board[4] == piece || board[20] == piece) {
        return lose;
      }
    }
  }

  piece = board[1];
  if (piece != '-' && board[7] == piece && board[13] == piece && board[19] == piece) {
    return lose;
  }

  piece = board[5];
  if (piece != '-' && board[11] == piece && board[17] == piece && board[23] == piece) {
    return lose;
  }

  piece = board[3];
  if (piece != '-' && board[7] == piece && board[11] == piece && board[15] == piece) {
    return lose;
  }

  piece = board[9];
  if (piece != '-' && board[13] == piece && board[17] == piece && board[21] == piece) {
    return lose;
  }

  return undecided;
}

void rotate(char board[BOARD_SIZE], char target[BOARD_SIZE]) {
  /*
    00 01 02 03 04
    05 06 07 08 09
    10 11 12 13 14
    15 16 17 18 19
    20 21 22 23 24

    20 15 10 05 00
    21 16 11 06 01
    22 17 12 07 02
    23 18 13 08 03
    24 19 14 09 04
  */

  int targetIndex = 0;

  for (int i = 0; i < BOARD_DIMS; i++) {
    for (int j = BOARD_DIMS - 1; j >= 0; j--) {
      target[targetIndex++] = board[i * BOARD_DIMS + j];
    }
  }
}

void horizontal(char board[BOARD_SIZE], char target[BOARD_SIZE]) {
  for (int i = 0; i < BOARD_DIMS; i++) {
    for (int j = 0; j < BOARD_DIMS; j++) {
      target[i * BOARD_DIMS + (BOARD_DIMS - 1- j)] = board[i * BOARD_DIMS + j];
    }
  }
}

void swapColour(char board[BOARD_SIZE], char target[BOARD_SIZE]) {
  for (int i = 0; i < BOARD_SIZE; i++) {
    if (board[i] == 'W') {
      target[i] = 'B';
    } else if (board[i] == 'B') {
      target[i] = 'W';
    } else {
      target[i] = '-';
    }
  }
}

POSITION min(POSITION a, POSITION b) {
  return a < b ? a : b;
}

/* Symmetry Handling: Return the canonical position. */
POSITION GetCanonicalPosition(POSITION position) {
  char boards[8][BOARD_SIZE];
  char turn;
  unhashIt(position, boards[0], &turn);
  char oppTurn = (turn == 'W') ? 'B' : 'W';

  horizontal(boards[0], boards[4]);

  for (int i = 1; i < 4; i++) {
    rotate(boards[i - 1], boards[i]);
    rotate(boards[i + 3], boards[i + 4]);
  }

  char swapped[8][BOARD_SIZE];

  for (int i = 0; i < 8; i++) {
    swapColour(boards[i], swapped[i]);
  }

  POSITION best = position;

  for (int i = 1; i < 8; i++) {
    POSITION alt = hashIt(boards[i], turn);
    best = min(best, alt);
  }

  for (int i = 0; i < 8; i++) {
    POSITION alt = hashIt(swapped[i], oppTurn);
    best = min(best, alt);
  }

  return best;
}

/*********** END SOLVING FUNCTIONS ***********/







/*********** BEGIN TEXTUI FUNCTIONS ***********/

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn) {
  char board[BOARD_SIZE];
  char turn;

  unhashIt(position, board, &turn);

  printf("\n");

  for (int i = 0; i < BOARD_DIMS; i++) {
    printf("%d ", (i + 1));

    for (int j = 0; j < BOARD_DIMS; j++) {
      printf("%c", board[i * BOARD_DIMS + j]);
    }

    printf("\n");
  }

  printf("  abcde          TURN: %c\n\n", turn);
}

void PrintComputersMove(MOVE computersMove, STRING computersName) {
  int origin = computersMove / 100;
  int target = computersMove % 100;

  int originRow = origin / BOARD_DIMS;
  int originCol = origin % BOARD_DIMS;

  int targetRow = target / BOARD_DIMS;
  int targetCol = target % BOARD_DIMS;

  printf("Computer's move: [%d%c %d%c]\n", originRow + 1, (char) (originCol + 97), targetRow + 1, (char) (targetCol + 97));
}

USERINPUT GetAndPrintPlayersMove(POSITION position, MOVE *move, STRING playerName) {
  USERINPUT ret;
	do {
		printf("%8s's move [(u)ndo]/[<origin> <target>] :  ", playerName);
		ret = HandleDefaultTextInput(position, move, playerName);
		if (ret != Continue) {
			return ret;
        }
	} while (TRUE);
	return Continue;
}

/* Return whether the input text signifies a valid move. */
BOOLEAN ValidTextInput(STRING input) {
  if (strlen(input) < 5) {
    return FALSE;
  } else if (input[0] < 49 || input[0] > 53) {
    return FALSE;
  } else if (input[1] < 97 || input[1] > 101) {
    return FALSE;
  } else if (input[3] < 49 || input[3] > 53) {
    return FALSE;
  } else if (input[4] < 97 || input[4] > 101) {
    return FALSE;
  }

  return TRUE;
}

/* Assume the text input signifies a valid move. Return
the move hash corresponding to the move. */
MOVE ConvertTextInputToMove(STRING input) {
  int originRow = input[0] - 49;
  int originCol = input[1] - 97;

  int origin = originRow * BOARD_DIMS + originCol;
  
  int targetRow = input[3] - 49;
  int targetCol = input[4] - 97;

  int target = targetRow * BOARD_DIMS + targetCol;

  return origin * 100 + target;
}

/* Return the string representation of the move. 
Ideally this matches with what the user is supposed to
type when they specify moves. */
STRING MoveToString(MOVE move) {
  int origin = move / 100;
  int target = move % 100;

  int originRow = origin / BOARD_DIMS;
  int originCol = origin % BOARD_DIMS;

  int targetRow = target / BOARD_DIMS;
  int targetCol = target % BOARD_DIMS;

  STRING result = (STRING) SafeMalloc(6);

  sprintf(result, "%d%c %d%c", originRow + 1, (char) (originCol + 97), targetRow + 1, (char) (targetCol + 97));

  return result;
}

/* Basically just print the move. */
void PrintMove(MOVE move) {
  STRING str = MoveToString(move);
	printf("[%s]", str);
	SafeFree(str);
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


/***************** TIER FUNCTIONS ********************/

POSITION swapTurn(POSITION position) {
  if (position < numBoardArrangements) {
    return position + numBoardArrangements;
  } else {
    return position - numBoardArrangements;
  }
}

UNDOMOVELIST *GenerateUndoMovesToTier(POSITION position, TIER tier) {
  MOVELIST *moves = GenerateMoves(swapTurn(position));
  MOVELIST *head = moves;
  UNDOMOVELIST *undoMoves = NULL;
  while (moves != NULL) {
    if (Primitive(UndoMove(position, moves->move)) == undecided) {
      undoMoves = CreateUndoMovelistNode(moves->move, undoMoves);
    }
    moves = moves->next;
  }
  FreeMoveList(head);
  return undoMoves;
}

POSITION UndoMove(POSITION position, UNDOMOVE undoMove) {
  return swapTurn(DoMove(swapTurn(position), undoMove));
}


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

/* Optional. */
STRING InteractPositionToEndData(POSITION position) {
  return NULL;
}

STRING InteractMoveToString(POSITION position, MOVE move) {
  /* YOUR CODE HERE */
  return MoveToString(move);
}