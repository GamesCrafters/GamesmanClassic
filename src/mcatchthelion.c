/************************************************************************
**
** NAME:        mcatchthelion.c
**
** DESCRIPTION: <Dōbutsu shōgi> (Catch The Lion)
**x
** AUTHOR:      Nayna Siddharth, Maansi Singh
**
** DATE:        YYYY-MM-DD
**
************************************************************************/

#include "gamesman.h"

/* IMPORTANT GLOBAL VARIABLES */
CONST_STRING kAuthorName = "Nayna Siddharth, Maansi Singh";
CONST_STRING kGameName = "Catch The Lion"; //  use this spacing and case
CONST_STRING kDBName = "catchthelion"; // use this spacing and case
POSITION gNumberOfPositions = 11800; // TODO: Put your number of positions upper bound here.
POSITION gInitialPosition = 0; // TODO: Put the hash value of the initial position.
BOOLEAN kPartizan = TRUE; // TODO: Is the game PARTIZAN i.e. given a board does each player have a different set of moves available to them?
BOOLEAN kTieIsPossible = FALSE; // TODO: Is a tie or draw possible?
BOOLEAN kLoopy = TRUE; // TODO: Is this game loopy?
BOOLEAN kSupportsSymmetries = TRUE; // TODO: Whether symmetries are supported (i.e. whether the GetCanonicalPosition is implemented)

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
void SetTclCGameSpecificOptions(int theOptions[]) {
  (void)theOptions;
}
/* Do not worry about this yet because you will only be supporting 1 variant for now. */
void GameSpecificMenu() {}

#define BOARD_SIZE 4

typeDef enum{
  CHICK = 1,
  ELEPHANT = 2,
  GIRAFFE = 3,
  LION = 4,
  EMPTY = 0
} PieceType;

typedef enum {
  PLAYER_1,
  PLAYER_2
} player;

/*********** BEGIN SOLVING FUNCIONS ***********/

/* TODO: Add a hashing function and unhashing function, if needed. */
//board init
Piece board[BOARD_SIZE][BOARD_SIZE] = {
  { GIRAFFE, CHICK, EMPTY, LION },
  { EMPTY, ELEPHANT, EMPTY, EMPTY },
  { EMPTY, CHICK, GIRAFFE, EMPTY },
  { LION, EMPTY, CHICK, GIRAFFE }
};

//positions as strings
char* encode_position() {
  char* position = malloc(BOARD_SIZE * BOARD_SIZE * sizeof(char));
  for (int i = 0; i < BOARD_SIZE; i++) {
    for (int j = 0; j < BOARD_SIZE; j++) {
      switch (board[i][j]) {
        case CHICK:
          position[i*BOARD_SIZE + j] = 'c';
          break;
        case GIRAFFE:
          position[i*BOARD_SIZE + j] = 'g';
          break;
        case ELEPHANT:
          position[i*BOARD_SIZE + j] = 'e';
          break;
        case LION:
          position[i*BOARD_SIZE + j] = 'l';
          break;
        case EMPTY:
          position[i*BOARD_SIZE + j] = '-';
          break;
      }
    }
  }
  position[BOARD_SIZE * BOARD_SIZE] = '\0';
  return position;
}


/* Initialize any global variables or data structures needed before
solving or playing the game. */
void InitializeGame() {
  gCanonicalPosition = GetCanonicalPosition;
  gMoveToStringFunPtr = &MoveToString;
  int rows = 4;
  int cols = 3;
  /* YOUR CODE HERE */
  
}

/* Return the hash value of the initial position. */
POSITION GetInitialPosition() {
  /* YOUR CODE HERE */
}

/* Return a linked list of moves. */
MOVELIST *GenerateMoves(POSITION position) {
  MOVELIST *moves = NULL;
  /* YOUR CODE HERE 
     To add to the linked list, do 
     moves = CreateMovelistNode(<the move you're adding>, moves);
     See the function CreateMovelistNode in src/core/misc.c
  */
 *num_moves = 0;
  for (int i = 0; i < BOARD_SIZE; i++) {
    for (int j = 0; j < BOARD_SIZE; j++) {
      if (player == PLAYER_1 && board[i][j] != EMPTY && board[i][j] < LION) {
        // Player 1 can move any piece that isn't the lion
        if (i > 0 && (board[i-1][j] == EMPTY || (board[i-1][j] >= LION && board[i][j] < board[i-1][j]))) {
          // Can move up
          char* move = malloc(5 * sizeof(char));
          move[0] = i + '0';
          move[1] = j + '0';
          move[2] = i-1 + '0';
          move[3] = j + '0';
          move[4] = '\0';
          if (is_valid_move(position, move)) {
            moves = CreateMovelistNode(move, moves);
            (*num_moves)++;
          } else {
            free(move);
          }
        }
      } else if (player == PLAYER_2 && board[i][j] != EMPTY && board[i][j] >= LION) {
        // Player 2 can move any piece that isn't the lion
        if (i < BOARD_SIZE-1 && (board[i+1][j] == EMPTY || (board[i+1][j] < LION && board[i][j] < board[i+1][j]))) {
          // Can move down
          char* move = malloc(5 * sizeof(char));
          move[0] = i + '0';
          move[1] = j + '0';
          move[2] = i+1 + '0';
          move[3] = j + '0';
          move[4] = '\0';
          if (is_valid_move(position, move)) {
            moves[*num_moves] = move;
            (*num_moves)++;
          } else {
            free(move);
          }
        }
      }
    }
  }
}

bool is_valid_move(char* position, Player player, char* move) {
  // Convert move to row and column values
  int from_row = move[0] - '0';
  int from_col = move[1] - '0';
  int to_row = move[2] - '0';
  int to_col = move[3] - '0';

  // Check if move is within board boundaries
  if (from_row < 0 || from_row >= BOARD_SIZE ||
      from_col < 0 || from_col >= BOARD_SIZE ||
      to_row < 0 || to_row >= BOARD_SIZE ||
      to_col < 0 || to_col >= BOARD_SIZE) {
    return false;
  }

  // Check if moving player's own piece
  int piece = position[from_row][from_col];
  if (player == PLAYER_1 && piece < EMPTY || piece > ELEPHANT_P2) {
    return false;
  }
  if (player == PLAYER_2 && piece < LION_P1 || piece > ELEPHANT_P1) {
    return false;
  }

  // Check if destination is empty or contains opponent's piece
  int dest = position[to_row][to_col];
  if (player == PLAYER_1 && dest >= LION_P1 && dest <= ELEPHANT_P1) {
    return false;
  }
  if (player == PLAYER_2 && dest >= EMPTY && dest <= ELEPHANT_P2) {
    return false;
  }

  // Check if move is valid for piece type
  switch (piece) {
    case LION_P1:
    case LION_P2:
      if (abs(from_row - to_row) > 1 || abs(from_col - to_col) > 1) {
        return false;
      }
      break;
    case ELEPHANT_P1:
    case ELEPHANT_P2:
      if ((abs(from_row - to_row) > 2 || abs(from_col - to_col) > 2) ||
          (abs(from_row - to_row) == 1 && abs(from_col - to_col) == 1)) {
        return false;
      }
      break;
    case GIRA_P1:
    case GIRA_P2:
      if ((from_row == to_row && abs(from_col - to_col) > 1) ||
          (from_col == to_col && abs(from_row - to_row) > 1) ||
          (abs(from_row - to_row) == 1 && abs(from_col - to_col) == 1)) {
        return false;
      }
      break;
    case KIRI_P1:
    case KIRI_P2:
      if ((from_row == to_row && abs(from_col - to_col) > 1) ||
          (from_col == to_col && abs(from_row - to_row) > 1) ||
          (abs(from_row - to_row) == abs(from_col - to_col))) {
        return false;
      }
      break;
    case ZOU_P1:
    case ZOU_P2:
      if ((abs(from_row - to_row) != 1 || abs(from_col - to_col) != 1) &&
          (abs(from_row - to_row) != 2 || abs(from_col - to_col) != 0) &&
          (abs(from_row - to_row) != 0 || abs(from_col - to_col) != 2)) {
        return false;
      }
      break;
    default:
      return false;
  }

  return true;
}

/* Return the position that results from making the 
input move on the input position. */
POSITION DoMove(POSITION position, MOVE move) {
  /* YOUR CODE HERE */

  char board[BOARD_SIZE][BOARD_SIZE];
  
  int start_row = move[0] - '0';
  int start_col = move[1] - '0';
  int end_row = move[2] - '0';
  int end_col = move[3] - '0';

  char piece = board[start_row][start_col];
  char captured_piece = board[end_row][end_col];

  if (!is_valid_move(board, start_row, start_col, end_row, end_col, piece)) {
    return INVALID_MOVE;
  }

  board[end_row][end_col] = piece;
  board[start_row][start_col] = EMPTY;

  char new_position[POSITION_SIZE];
  format_position(board, new_position);

  if (captured_piece == ELEPHANT) {
    return PLAYER_1_WINS;
  } else if (captured_piece == LION) {
    return PLAYER_2_WINS;
  } else {
    return OK;
  }
}


/*****************************************************************
**  Determine whether the position is a primitive win,
**  primitive tie, primitive lose, or not primitive.
**  OUTPUTS: (VALUE) an enum; one of: (win, lose, tie, undecided)
**  See src/core/types.h for the value enum definition.
******************************************************************/
VALUE Primitive(POSITION position) {
  /* YOUR CODE HERE */
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
type when they specify moves. */
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

STRING InteractMoveToString(POSITION position, MOVE move) {
  /* YOUR CODE HERE */
  return MoveToString(move);
}