/************************************************************************
**
** NAME:        mbeeline.c
**
** DESCRIPTION: Beeline
**
** AUTHOR:      Andrew Esteban
**
** DATE:        2023-03-11
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
BOOLEAN kLoopy = TRUE; // Is this game loopy?
BOOLEAN kSupportsSymmetries = TRUE; // Whether symmetries are supported (i.e. whether the GetCanonicalPosition is implemented)

/* Do not change these. */
POSITION GetCanonicalPosition(POSITION);
STRING MoveToString(MOVE);
POSITION kBadPosition = -1;
STRING kGameName = "Beeline";
STRING kDBName = "beeline";
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

/*************************************************************************
**
** Everything above here must be in every game file
**
**************************************************************************/

/*************************************************************************
**
** Every variable declared here is only used in this file (game-specific)
**
**************************************************************************/

const int boardDimension = 4;
const int boardSize = boardDimension * boardDimension;

POSITION GetCanonicalPosition(POSITION);
STRING MoveToString(MOVE);
POSITION ActualNumberOfPositions(int variant);

/************************************************************************
**
** NAME: InitializeDatabases
**
** DESCRIPTION: Initialize the gDatabase, a global variable.
**
************************************************************************/

int vcfg(int pieces[]) {
  return pieces[0] == pieces[1];
}

void InitializeGame() {
  gCanonicalPosition = GetCanonicalPosition;
  gMoveToStringFunPtr = &MoveToString;
  gActualNumberOfPositionsOptFunPtr = &ActualNumberOfPositions;

  // {char, min, max, char, min, max, ..., -1}
  int piecesArray[] = {'w', 3, 3, 'b', 3, 3, ' ', 10, 10, -1};
  gNumberOfPositions = generic_hash_init(boardSize, piecesArray, &vcfg, 0);

  char initialBoard[boardSize];

  for (int i = 0; i < boardSize; i++) {
    initialBoard[i] = ' ';
  }

  initialBoard[0] = 'w';
  initialBoard[1] = 'w';
  initialBoard[4] = 'w';
  initialBoard[11] = 'b';
  initialBoard[14] = 'b';
  initialBoard[15] = 'b';

  gInitialPosition = generic_hash_hash(initialBoard, 1);
}

/************************************************************************
**
** NAME: DebugMenu
**
** DESCRIPTION: Menu used to debug internal problems. Does nothing if
** kDebugMenu == FALSE
**
************************************************************************/

void DebugMenu() {
}

/************************************************************************
**
** NAME: GameSpecificMenu
**
** DESCRIPTION: Menu used to change game-specific parmeters, such as
** the side of the board in an nxn Nim board, etc. Does
** nothing if kGameSpecificMenu == FALSE
**
************************************************************************/

void GameSpecificMenu() {
}

/************************************************************************
**
** NAME: SetTclCGameSpecificOptions
**
** DESCRIPTION: Set the C game-specific options (called from Tcl)
** Ignore if you don't care about Tcl for now.
**
************************************************************************/

void SetTclCGameSpecificOptions(int theOptions[]) {
}

/************************************************************************
**
** NAME: DoMove
**
** DESCRIPTION: Apply the move to the position.
**
** INPUTS: POSITION position : The old position
** MOVE move : The move to apply.
**
** OUTPUTS: (POSITION) : The position that results after the move.
**
** CALLS: PositionToBlankOX(POSITION,*BlankOX)
** BlankOX WhosTurn(*BlankOX)
**
************************************************************************/

POSITION DoMove(POSITION position, MOVE move) {
  char board[boardSize];
  generic_hash_unhash(position, board);

  // First 2 digits of move represent the index of the piece
  // Last 2 digits represent the index of the target location

  int player = generic_hash_turn(position);

  int target = move % 100;
  int origin = move / 100;

  board[target] = board[origin];
  board[origin] = ' ';

  return generic_hash_hash(board, (player % 2) + 1);
}

/************************************************************************
**
** NAME: GetInitialPosition
**
** DESCRIPTION: Ask the user for an initial position for testing. Store
** it in the space pointed to by initialPosition;
**
** OUTPUTS: POSITION initialPosition : The position to fill.
**
************************************************************************/

POSITION GetInitialPosition() {
  return gInitialPosition;
}

/************************************************************************
**
** NAME: PrintComputersMove
**
** DESCRIPTION: Nicely format the computers move.
**
** INPUTS: MOVE *computersMove : The computer's move.
** STRING computersName : The computer's name.
**
************************************************************************/

void PrintComputersMove(MOVE computersMove, STRING computersName) {
  PrintMove(computersMove);
}

/************************************************************************
**
** NAME: Primitive
**
** DESCRIPTION: Return the value of a position if it fulfills certain
** 'primitive' constraints. Some examples of this is having
** three-in-a-row with TicTacToe. TicTacToe has two
** primitives it can immediately check for, when the board
** is filled but nobody has one = primitive tie. Three in
** a row is a primitive lose, because the player who faces
** this board has just lost. I.e. the player before him
** created the board and won. Otherwise undecided.
**
** INPUTS: POSITION position : The position to inspect.
**
** OUTPUTS: (VALUE) an enum which is oneof: (win,lose,tie,undecided)
**
** CALLS: BOOLEAN ThreeInARow()
** BOOLEAN AllFilledIn()
** PositionToBlankOX()
**
************************************************************************/

VALUE Primitive(POSITION position) {
  char board[boardSize];

  generic_hash_unhash(position, board);

  char matrix[boardDimension][boardDimension];

  for (int i = 0; i < boardSize; i++) {
    matrix[i / boardDimension][i % boardDimension] = board[i];
  }

  int player = generic_hash_turn(position);

  for (int i = 0; i < boardDimension; i++) {
    for (int j = 0; j < boardDimension; j++) {
      if (matrix[i][j] == ' ') {
        continue;
      }

      char other = matrix[i][j] == 'w' ? 'b' : 'w';
      int count = 0;

      // Up
      if (i > 0 && j != boardDimension - 1 && matrix[i - 1][j + 1] == other) {
        count++;
      }

      // Right up
      if (j < boardDimension - 1 && matrix[i][j + 1] == other) {
        count++;
      }

      // Right down
      if (i < boardDimension - 1 && matrix[i + 1][j] == other) {
        count++;
      }

      // Down
      if (i < boardDimension - 1 && j != 0 && matrix[i + 1][j - 1] == other) {
        count++;
      }

      // Left down
      if (j > 0 && matrix[i][j - 1] == other) {
        count++;
      }

      // Left up
      if (i > 0 && matrix[i - 1][j] == other) {
        count++;
      }

      if (count == 3) {
        if ((player == 1 && matrix[i][j] == 'w') || (player == 2 && matrix[i][j] == 'b')) {
          return lose;
        } else {
          return win;
        }
      }
    }
  }

  return undecided;
}

/************************************************************************
**
** NAME: PrintPosition
**
** DESCRIPTION: Print the position in a pretty format, including the
** prediction of the game's outcome.
**
** INPUTS: POSITION position : The position to pretty print.
** STRING playerName : The name of the player.
** BOOLEAN usersTurn : TRUE <==> it's a user's turn.
**
** CALLS: PositionToBlankOX()
** GetValueOfPosition()
** GetPrediction()
**
************************************************************************/

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn) {

  /*
                           _ _
                         /     \
                    _ _ /       \ _ _ 
                  /     \       /     \
             _ _ /       \ _ _ /       \ _ _
           /     \       /     \       /     \
      _ _ /       \ _ _ /       \ _ _ /       \ _ _
    /     \       /     \       /     \       /     \
   /       \ _ _ /       \ _ _ /       \ _ _ /       \
   \       /     \       /     \       /     \       /
    \ _ _ /       \ _ _ /       \ _ _ /       \ _ _ /
          \       /     \       /     \       /
           \ _ _ /       \ _ _ /       \ _ _ /
                 \       /     \       /
                  \ _ _ /       \ _ _ /
                        \       / 
                         \ _ _ /    
  */

  char board[boardSize];

  generic_hash_unhash(position, board);

  printf("\t                         _ _\n");
  printf("\t                       /     \\\n");
  printf("\t                  _ _ /   %c   \\ _ _\n", board[3]);
  printf("\t                /     \\       /     \\\n");
  printf("\t           _ _ /   %c   \\ _ _ /   %c   \\ _ _\n", board[2], board[7]);
  printf("\t         /     \\       /     \\       /     \\\n");
  printf("\t    _ _ /   %c   \\ _ _ /   %c   \\ _ _ /   %c   \\ _ _\n", board[1], board[6], board[11]);
  printf("\t  /     \\       /     \\       /     \\       /     \\\n");
  printf("\t /   %c   \\ _ _ /   %c   \\ _ _ /   %c   \\ _ _ /   %c   \\\n", board[0], board[5], board[10], board[15]);
  printf("\t \\       /     \\       /     \\       /     \\       /\n");
  printf("\t  \\ _ _ /   %c   \\ _ _ /   %c   \\ _ _ /   %c   \\ _ _ /\n", board[4], board[9], board[14]);
  printf("\t        \\       /     \\       /     \\       /\n");
  printf("\t         \\ _ _ /   %c   \\ _ _ /   %c   \\ _ _ /\n", board[8], board[13]);
  printf("\t               \\       /     \\       /\n");
  printf("\t                \\ _ _ /   %c   \\ _ _ /\n", board[12]);
  printf("\t                      \\       /\n");
  printf("\t                       \\ _ _ /\n");

  printf("\n\t%s\n\n", GetPrediction(position,playerName,usersTurn));

}

/************************************************************************
**
** NAME: GenerateMoves
**
** DESCRIPTION: Create a linked list of every move that can be reached
** from this position. Return a pointer to the head of the
** linked list.
**
** INPUTS: POSITION position : The position to branch off of.
**
** OUTPUTS: (MOVELIST *), a pointer that points to the first item
** in the linked list of moves that can be generated.
**
** CALLS: MOVELIST *CreateMovelistNode(MOVE,MOVELIST *)
**
************************************************************************/

MOVELIST *GenerateMoves(POSITION position) {
  char board[boardSize];

  generic_hash_unhash(position, board);

  char matrix[boardDimension][boardDimension];

  for (int i = 0; i < boardSize; i++) {
    matrix[i / boardDimension][i % boardDimension] = board[i];
  }

  int player = generic_hash_turn(position);

  MOVELIST *moves = NULL;

  for (int i = 0; i < boardDimension; i++) {
    for (int j = 0; j < boardDimension; j++) {
      if ((player == 1 && matrix[i][j] == 'w') || (player == 2 && matrix[i][j] == 'b')) {
        // Move left-down
        if (j > 0 && matrix[i][j - 1] == ' ') {
          int k = j - 1;

          while (k - 1 >= 0 && matrix[i][k - 1] == ' ') {
            k--;
          }

          int moveValue = (i * boardDimension + j) * 100 + (i * boardDimension + k);

          moves = CreateMovelistNode(moveValue, moves);
        }

        // Move left-up
        if (i > 0 && matrix[i - 1][j] == ' ') {
          int k = i - 1;

          while (k - 1 >= 0 && matrix[k - 1][j] == ' ') {
            k--;
          }

          int moveValue = (i * boardDimension + j) * 100 + (k * boardDimension + j);

          moves = CreateMovelistNode(moveValue, moves);
        }

        // Move right-up
        if (j < boardDimension - 1 && matrix[i][j + 1] == ' ') {
          int k = j + 1;

          while (k + 1 < boardDimension && matrix[i][k + 1] == ' ') {
            k++;
          }

          int moveValue = (i * boardDimension + j) * 100 + (i * boardDimension + k);

          moves = CreateMovelistNode(moveValue, moves);
        }

        // Move right-down
        if (i < boardDimension - 1 && matrix[i + 1][j] == ' ') {
          int k = i + 1;

          while (k + 1 < boardDimension && matrix[k + 1][j] == ' ') {
            k++;
          }

          int moveValue = (i * boardDimension + j) * 100 + (k * boardDimension + j);

          moves = CreateMovelistNode(moveValue, moves);
        }
      }
    }
  }

  for (int i = 0; i < boardSize; i++) {
    if ((player == 1 && board[i] == 'w') || (player == 2 && board[i] == 'b')) {
      // Move down
      if (i % boardDimension != 0 && i < boardSize - boardDimension && \
          i + boardDimension - 1 < boardSize && board[i + boardDimension - 1] == ' ') {
        int k = i + boardDimension - 1;

        if (board[k] == ' ') {
          while (k % boardDimension != 0 && \
                 k + boardDimension - 1 < boardSize && board[k + boardDimension - 1] == ' ') {
            k += boardDimension - 1;
          }

          moves = CreateMovelistNode(i * 100 + k, moves);
        }
      }

      // Move up
      if (i >= boardDimension && i % boardDimension != boardDimension - 1 && \
          i - boardDimension + 1 >= 0 && board[i - boardDimension + 1] == ' ') {
        int k = i - boardDimension + 1;

        if (board[k] == ' ') {
          while (k % boardDimension != boardDimension - 1 && k - boardDimension + 1 >= 0 && board[k - boardDimension + 1] == ' ') {
            k -= (boardDimension - 1);
          }

          moves = CreateMovelistNode(i * 100 + k, moves);
        }
      }
    }
  }

  return moves;
}

/**************************************************/
/*********** SYMMETRY HANDLING BEGIN **************/
/**************************************************/

/************************************************************************
**
** NAME: GetCanonicalPosition
**
** DESCRIPTION: Go through all of the positions that are symmetrically
** equivalent and return the SMALLEST, which will be used
** as the canonical element for the equivalence set.
**
** INPUTS: POSITION position : The position return the canonical elt. of.
**
** OUTPUTS: POSITION : The canonical element of the set.
**
************************************************************************/

void toMatrix(char* board, char matrix[boardDimension][boardDimension]) {
  for (int i = 0; i < boardSize; i++) {
    matrix[i / boardDimension][i % boardDimension] = board[i];
  }
}

void toBoard(char matrix[boardDimension][boardDimension], char* board) {
  for (int i = 0; i < boardDimension; i++) {
    for (int j = 0; j < boardDimension; j++) {
      board[i * boardDimension + j] = matrix[i][j];
    }
  }
}

void verticalFlip(char* board, char* target) {
  char matrix[boardDimension][boardDimension];
  toMatrix(board, matrix);

  for (int i = 0; i < boardDimension; i++) {
    for (int j = 0; j < boardDimension / 2; j++) {
      char temp = matrix[i][j];
      matrix[i][j] = matrix[i][boardDimension - 1 - j];
      matrix[i][boardDimension - 1 - j] = temp;
    }
  }

  toBoard(matrix, target);
}

void horizontalFlip(char* board, char* target) {
  char matrix[boardDimension][boardDimension];
  toMatrix(board, matrix);

  for (int i = 0; i < boardDimension / 2; i++) {
    for (int j = 0; j < boardDimension; j++) {
      char temp = matrix[i][j];
      matrix[i][j] = matrix[boardDimension - 1 - i][j];
      matrix[boardDimension - 1 - i][j] = temp;
    }
  }

  toBoard(matrix, target);
}

void diagonalTopLeftFlip(char* board, char* target) {
  char matrix[boardDimension][boardDimension];
  toMatrix(board, matrix);

  for (int i = 0; i < boardDimension; i++) {
    for (int j = i + 1; j < boardDimension; j++) {
      char temp = matrix[i][j];
      matrix[i][j] = matrix[j][i];
      matrix[j][i] = temp;
    }
  }

  toBoard(matrix, target);
}

void diagonalTopRightFlip(char* board, char* target) {
  char matrix[boardDimension][boardDimension];
  toMatrix(board, matrix);

  for (int i = 0; i < boardDimension / 2; i++) {
    for (int j = 0; j < boardDimension - i; j++) {
      char temp = matrix[i][j];
      matrix[i][j] = matrix[boardDimension - 1 - i][ boardDimension - 1 - j];
      matrix[boardDimension - 1 - i][ boardDimension - 1 - j] = temp;
    }
  }

  toBoard(matrix, target);
}

void rotateBoard(char* board, char* target) {
  char buffer[boardSize];
  diagonalTopLeftFlip(board, buffer);
  verticalFlip(buffer, target);
}

POSITION min(POSITION a, POSITION b) {
  return a < b ? a : b;
}

POSITION GetCanonicalPosition(POSITION position) {
  char cousins[20][boardDimension];
  generic_hash_unhash(position, cousins[0]);

  int player = generic_hash_turn(position);
  int newPlayer = (player % 2) + 1;

  // 4 rotations
  rotateBoard(cousins[0], cousins[1]);
  rotateBoard(cousins[1], cousins[2]);
  rotateBoard(cousins[2], cousins[3]);

  // 4 reflections
  verticalFlip(cousins[0], cousins[4]);
  verticalFlip(cousins[1], cousins[5]);
  verticalFlip(cousins[2], cousins[6]);
  verticalFlip(cousins[3], cousins[7]);

  horizontalFlip(cousins[0], cousins[8]);
  horizontalFlip(cousins[1], cousins[9]);
  horizontalFlip(cousins[2], cousins[10]);
  horizontalFlip(cousins[3], cousins[11]);

  diagonalTopLeftFlip(cousins[0], cousins[12]);
  diagonalTopLeftFlip(cousins[1], cousins[13]);
  diagonalTopLeftFlip(cousins[2], cousins[14]);
  diagonalTopLeftFlip(cousins[3], cousins[15]);

  diagonalTopRightFlip(cousins[0], cousins[16]);
  diagonalTopRightFlip(cousins[1], cousins[17]);
  diagonalTopRightFlip(cousins[2], cousins[18]);
  diagonalTopRightFlip(cousins[3], cousins[19]);

  POSITION smallest = generic_hash_hash(cousins[0], newPlayer);

  for (int i = 1; i < 20; i++) {
    POSITION alt = generic_hash_hash(cousins[1], newPlayer);
    smallest = min(smallest, alt);
  }

  return position;
}

/**************************************************/
/*********** SYMMETRY HANDLING END ****************/
/**************************************************/

/************************************************************************
**
** NAME: GetAndPrintPlayersMove
**
** DESCRIPTION: This finds out if the player wanted an undo or abort or not.
** If so, return Undo or Abort and don't change theMove.
** Otherwise get the new theMove and fill the pointer up.
**
** INPUTS: POSITION *thePosition : The position the user is at.
** MOVE *theMove : The move to fill with user's move.
** STRING playerName : The name of the player whose turn it is
**
** OUTPUTS: USERINPUT : Oneof( Undo, Abort, Continue )
**
** CALLS: ValidMove(MOVE, POSITION)
** BOOLEAN PrintPossibleMoves(POSITION) ...Always True!
**
************************************************************************/

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

/************************************************************************
**
** NAME: ValidTextInput
**
** DESCRIPTION: Return TRUE iff the string input is of the right 'form'.
** For example, if the user is allowed to select one slot
** from the numbers 1-9, and the user chooses 0, it's not
** valid, but anything from 1-9 IS, regardless if the slot
** is filled or not. Whether the slot is filled is left up
** to another routine.
**
** INPUTS: STRING input : The string input the user typed.
**
** OUTPUTS: BOOLEAN : TRUE iff the input is a valid text input.
**
************************************************************************/

BOOLEAN ValidTextInput(STRING input) {
  // Length needs to be 5
  if (strlen(input) != 5) {
    return FALSE;
  }

  int origin = ((int) input[0]) - 48;
  origin *= 10;
  origin += ((int) input[1]) - 48;

  if (origin < 0 || origin >= boardSize) {
    return FALSE;
  }

  int target = ((int) input[3]) - 48;
  target *= 10;
  target += ((int) input[4]) - 48;

  if (target < 0 || target >= boardSize) {
    return FALSE;
  }

  return TRUE;
}

/************************************************************************
**
** NAME: ConvertTextInputToMove
**
** DESCRIPTION: Convert the string input to the internal move representation.
**
** INPUTS: STRING input : The string input the user typed.
**
** OUTPUTS: MOVE : The move corresponding to the user's input.
**
************************************************************************/

MOVE ConvertTextInputToMove(STRING input) {
  // Input: xx yy
  // xx is the origin index
  // yy is the target index
  int move = 0;

  move += ((int) input[0]) - 48;
  move *= 10;

  move += ((int) input[1]) - 48;
  move *= 10;

  move += ((int) input[3]) - 48;
  move *= 10;

  move += ((int) input[4]) - 48;

  return move;
}

/************************************************************************
**
** NAME: PrintMove
**
** DESCRIPTION: Print the move in a nice format.
**
** INPUTS: MOVE *theMove : The move to print.
**
************************************************************************/

void PrintMove(MOVE move) {
  int origin = move / 100;
  int target = move % 100;

  printf("[%02d %02d]", origin, target);
}

/************************************************************************
**
** NAME: MoveToString
**
** DESCRIPTION: Returns the move as a STRING
**
** INPUTS: MOVE *move : The move to put into a string.
**
************************************************************************/

STRING MoveToString(MOVE move) {
  char* result = (char*) SafeMalloc(5);

  int origin = move / 100;
  int target = move % 100;

  sprintf(result, "%2d %2d", origin, target);

  return result;
}

int NumberOfOptions() {
  return 1;
}

int getOption() {
  return 0;
}

void setOption(int option) {
}

POSITION ActualNumberOfPositions(int variant) {
  return 0;
}

POSITION InteractStringToPosition(STRING board) {
  return 0;
}

STRING InteractPositionToString(POSITION position) {
  return NULL;
}

STRING InteractPositionToEndData(POSITION position) {
  return NULL;
}

STRING InteractMoveToString(POSITION position, MOVE move) {
  return MoveToString(move);
}
