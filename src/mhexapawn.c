/************************************************************************
**
** NAME:        mhexapawn.c
**
** DESCRIPTION: Hexapawn
**
** AUTHOR:      Madeline Agusalim & Sareena Mann
**
** DATE:        2025-10-05
**
************************************************************************/


#include "gamesman.h"


/* IMPORTANT GLOBAL VARIABLES */
CONST_STRING kAuthorName = "Madeline Agusalim & Sareena Mann";
CONST_STRING kGameName = "Hexapawn"; //  Use this spacing and case
CONST_STRING kDBName = "hexapawn"; // Use this spacing and case
POSITION gNumberOfPositions; // Put your number of positions upper bound here.
POSITION gInitialPosition; // Put the hash value of the initial position.
BOOLEAN kPartizan = TRUE; // Is the game PARTIZAN i.e. given a board does each player have a different set of moves available to them?
BOOLEAN kTieIsPossible = FALSE; // Is a tie or draw possible?
BOOLEAN kLoopy = FALSE; // Is this game loopy?
BOOLEAN kSupportsSymmetries = FALSE; // Whether symmetries are supported (i.e. whether the GetCanonicalPosition is implemented)


/* Likely you do not have to change these. */
POSITION GetCanonicalPosition(POSITION);
POSITION kBadPosition = -1;
BOOLEAN kDebugDetermineValue = FALSE;
void* gGameSpecificTclInit = NULL;


/* You do not have to change these for now. */
BOOLEAN kGameSpecificMenu = TRUE; // True for Variants
BOOLEAN kDebugMenu = FALSE;


/* These variables are not needed for solving but if you have time
after you're done solving the game you should initialize them
with something helpful. */
CONST_STRING kHelpGraphicInterface = "";
CONST_STRING kHelpTextInterface = "";
CONST_STRING kHelpOnYourTurn = "";
CONST_STRING kHelpStandardObjective = "Advance a pawn to the far rank or block opponent so they cannot move.";
CONST_STRING kHelpReverseObjective = "";
CONST_STRING kHelpTieOccursWhen = /* Should follow 'A Tie occurs when... */ "";
CONST_STRING kHelpExample = "";


/* You don't have to change this. */
void DebugMenu() {}
/* Ignore this function. */
void SetTclCGameSpecificOptions(int theOptions[]) {
  (void)theOptions;
}


/*********** BEGIN SOLVING FUNCIONS ***********/
// static const int rows = 3;
// static const int cols = 3;
// static const int CELLS = rows * cols;


int variant_length;
int ROWS;
int COLS;
int CELLS;
int boardSize;


// Helper Functions
static inline int in_bounds(int i) {
    return (i >= 0 && i < CELLS);
}


// int vcfg(int pieces[]) {
//   // return pieces[0] > 1 || pieces[1] > 1;
//   return 0;
// }


/* Initialize any global variables or data structures needed before
solving or playing the game. */
void InitializeGame() {
  if (variant_length <= 0) {
    setOption(3);
  }


  gCanonicalPosition = GetCanonicalPosition;


  // {char, min, max, char, min, max, ..., -1}
  int piecesArray[] = {'B', 0, CELLS, 'W', 0, CELLS, '-', 0, CELLS, -1};


  gNumberOfPositions = generic_hash_init(CELLS, piecesArray, NULL, 0);


  char *initial = malloc(CELLS * sizeof(char));

  for (int r = 0; r < ROWS; r++) {
    for (int c = 0; c < COLS; c++) {
        int idx = r * COLS + c;
        if (r == 0)
            initial[idx] = 'B';
        else if (r == ROWS - 1)
            initial[idx] = 'W';
        else
            initial[idx] = '-';
    }
  }


  gInitialPosition = generic_hash_hash(initial, 1);
  free(initial);
}


BOOLEAN HasAnyMove(const char board[CELLS], int sideToMove){
    for (int index = 0; index < CELLS; index++) {
        int row = index / COLS;
        int col = index % COLS;


        if (sideToMove == 1) { // White moves up (-3)
            if (board[index] != 'W') continue;
            if (row - 1 >= 0) {
                int fwd_idx = index - COLS; // up
                int left_capture_idx = fwd_idx - 1; // up diagonal left
                int right_capture_idx = fwd_idx + 1; // up diagonal right
                if (board[fwd_idx] == '-')
                    return TRUE;
                if (col > 0 && in_bounds(left_capture_idx) && board[left_capture_idx]  == 'B')
                    return TRUE;
                if (col < COLS - 1 && in_bounds(right_capture_idx) && board[right_capture_idx] == 'B')
                    return TRUE;
            }


        } else { // Black moves down (+3)
            if (board[index] != 'B') continue;
            if (row + 1 < ROWS) {
                int fwd_idx = index + COLS; // down
                int left_capture_idx = fwd_idx - 1; // down diagonal left
                int right_capture_idx = fwd_idx + 1; // down diagonal right
                if (board[fwd_idx] == '-')
                    return TRUE;
                if (col > 0 && in_bounds(left_capture_idx) && board[left_capture_idx] == 'W')
                    return TRUE;
                if (col < COLS - 1 && in_bounds(right_capture_idx) && board[right_capture_idx] == 'W')
                    return TRUE;
            }
        }
    }
    return FALSE;
}


/* Return a linked list of moves. */
/*
  For the side to move:
    - Add a forward move (−3 for White, +3 for Black) if the square is empty.
    - Add up-left / up-right (White) or down-left / down-right (Black)
      if the target square contains an opponent pawn and stays within the 3×3 bounds.
*/
MOVELIST *GenerateMoves(POSITION position) {
  MOVELIST *moves = NULL;


  char board[CELLS];
  generic_hash_unhash(position, board);
  int turn = generic_hash_turn(position);


  for (int index = 0; index < CELLS; index++) {
    int row = index / COLS;
    int col = index % COLS;


    if (turn == 1) {
        if (board[index] != 'W') continue;
        if (row - 1 >= 0 ) {
            int fwd_idx = index - COLS;
            int left_capture_idx = fwd_idx - 1;
            int right_capture_idx = fwd_idx +1;


            if (board[fwd_idx] == '-')
                moves = CreateMovelistNode(index*100 + fwd_idx, moves);


            if(col > 0 && in_bounds(left_capture_idx) && board[left_capture_idx] == 'B')
                moves = CreateMovelistNode(index*100 + left_capture_idx, moves);


            if (col < COLS - 1 && in_bounds(right_capture_idx) && board[right_capture_idx] == 'B')
                moves = CreateMovelistNode(index*100 + right_capture_idx, moves);
        }


    } else {
        if (board[index] != 'B') continue;
        if (row + 1 < ROWS) {
            int fwd_idx = index + COLS;
            int left_capture_idx = fwd_idx - 1;
            int right_capture_idx = fwd_idx + 1;


            if (board[fwd_idx] == '-')
                moves = CreateMovelistNode(index*100 + fwd_idx, moves);


            if (col > 0 && in_bounds(left_capture_idx) && board[left_capture_idx] == 'W')
                moves = CreateMovelistNode(index*100 + left_capture_idx, moves);


            if (col < COLS - 1 && in_bounds(right_capture_idx) && board[right_capture_idx] == 'W')
                moves = CreateMovelistNode(index*100 + right_capture_idx, moves);
        }
    }
  }

  return moves;
}


/* Return the position that results from making the
input move on the input position. */
POSITION DoMove(POSITION position, MOVE move) {
  char board[CELLS];
  generic_hash_unhash(position, board);


  int origin = move / 100;
  int target = move % 100;


  board[target] = board[origin];
  board[origin] = '-';


  int player = generic_hash_turn(position);

  return generic_hash_hash(board, (player % 2) + 1);
}


/*****************************************************************
**  Determine whether the position is a primitive win,
**  primitive tie, primitive lose, or not primitive.
**  OUTPUTS: (VALUE) an enum; one of: (win, lose, tie, undecided)
**  See src/core/types.h for the value enum definition.
******************************************************************/
VALUE Primitive(POSITION position) {
/*
  Win conditions (for the player who just moved):
    - A pawn reaches the far rank (White reaches row 0; Black reaches row 2).
    - The opponent has no pawns left.
    - The opponent has no legal moves on their turn.


  Lose condition:
    - If it’s your turn and you have no legal moves, you lose.


  Notes:
    - No draws/ties
    - Pawns move straight forward one square if empty;
      capture one square diagonally forward.
*/


  char board[CELLS];
  generic_hash_unhash(position, board);


  int turn = generic_hash_turn(position); // 1 = White to move; 2 = Black to move


  // Zero Pawn Check
  int whiteCount = 0, blackCount = 0;
  for (int i = 0; i < CELLS; i++) {
    if (board[i] == 'W')
        whiteCount++;
    else if (board[i] == 'B')
        blackCount++;
  }
  if (whiteCount == 0)
    return (turn == 1) ? lose : win;
  if (blackCount == 0)
    return (turn == 2) ? lose : win;


  // Pawn Reaches opposite side
  if (board[0] == 'W' || board[1] == 'W' || board[2] == 'W')
    return (turn == 1) ? win : lose;
  if (board[6] == 'B' || board[7] == 'B' || board[8] == 'B')
    return (turn == 2) ? win : lose;


  // no moves left -> that side loses
  if (!HasAnyMove(board, turn)) return lose;
  return undecided;
}


/* Symmetry Handling: Return the canonical position. */
POSITION GetCanonicalPosition(POSITION position) {
  // There are no symmetries
  return position;
}


/*********** END SOLVING FUNCTIONS ***********/














/*********** BEGIN TEXTUI FUNCTIONS ***********/


// helper functions : x y z -> cols 0 1 2
static int col_to_index(char colLabel) {
  if (colLabel == 'x') return 0;
  if (colLabel == 'y') return 1;
  if (colLabel == 'z') return 2;
  return -1;
}


static int row_to_index(int rowNumber) {
  if (rowNumber < 1 || rowNumber > ROWS) return -1;
  return rowNumber - 1;
}


static int coord_to_index(char colLabel, int rowNumber) {
  int c = col_to_index(colLabel);
  int r = row_to_index(rowNumber);
  if (c < 0 || r < 0) return -1;
  return r * COLS + c;
}


/* index (0-based) -> "x3" written into out */
static void index_to_coord(int idx, char *out) {
  const char colsLbl[3] = { 'x', 'y', 'z' };
  int r = idx / COLS;
  int c = idx % COLS;
  snprintf(out, 16, "%c%d", colsLbl[c], r + 1);
}


void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn) {
  (void)playerName;
  (void)usersTurn;
  char board[CELLS];
  generic_hash_unhash(position, board);
  int turn = generic_hash_turn(position);


  char *start = malloc(CELLS * sizeof(char));
  for (int r = 0; r < ROWS; r++) {
    for (int c = 0; c < COLS; c++) {
      int idx = r * COLS + c;
      if (r == 0)
        start[idx] = 'B';
      else if (r == ROWS - 1)
        start[idx] = 'W';
      else
        start[idx] = '-';
    }
  }


  printf("\n STARTING POSITION           CURRENT POSITION\n");
  printf(" -----------------           ----------------\n");

  for (int r = ROWS - 1; r >= 0; r--) {
    printf(" %d  ", r + 1);


    // left (starting board)
    for (int c = 0; c < COLS; c++) {
      putchar(start[r * COLS + c]);
      if (c + 1 < COLS) putchar(' ');
    }


    printf("                    ");


    // right (current board)
    printf("%d  ", r + 1);
    for (int c = 0; c < COLS; c++) {
      putchar(board[r * COLS + c]);
      if (c + 1 < COLS) putchar(' ');
    }
    putchar('\n');
  }


  printf("    ");
  for (int c = 0; c < COLS; c++) printf("%c ", 'r' + c);
  //printf("          ");
  printf("                      ");
  for (int c = 0; c < COLS; c++) printf("%c ", 'r' + c);
  printf("\n");


  printf("\n\t%s to move\n\n", (turn == 1) ? "White (W)" : "Black (B)");
  printf("Prediction: %s\n", GetPrediction(position, playerName, usersTurn));
  printf("\tEnter moves as:  x3 x2   (from to)\n");
  printf("\tLowercase x/y/z only. Example for White: x3 x2\n\n");
  free(start);

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
/* Accept "x3 x2" (lowercase x/y/z; rows 1..rows) */
BOOLEAN ValidTextInput(STRING input) {
  /* pattern: [x|y|z][1-rows] ' ' [x|y|z][1-rows] */
  if (!input || !input[0] || !input[1] || !input[2] || !input[3] || !input[4])
    return FALSE;


  char c1 = input[0];
  char r1 = input[1];
  char sp = input[2];
  char c2 = input[3];
  char r2 = input[4];


  if (sp != ' ') return FALSE;


  if (!(c1 == 'x' || c1 == 'y' || c1 == 'z')) return FALSE;
  if (!(c2 == 'x' || c2 == 'y' || c2 == 'z')) return FALSE;


  if (r1 < '1' || r1 > ('0' + ROWS)) return FALSE;
  if (r2 < '1' || r2 > ('0' + ROWS)) return FALSE;
  if (input[5] != '\0' && input[5] != '\n') return FALSE;


  return TRUE;
}


/* Assume the text input -> valid move
Return move hash -> move */
/* Convert "x3 x2" to internal MOVE hash = from*100 + to */
MOVE ConvertTextInputToMove(STRING input) {
  char fromCol = input[0];
  int  fromRow = input[1] - '0';
  char toCol   = input[3];
  int  toRow   = input[4] - '0';


  int fromIdx = coord_to_index(fromCol, fromRow);
  int toIdx   = coord_to_index(toCol, toRow);


  if (fromIdx < 0 || toIdx < 0) return 0;
  return fromIdx * 100 + toIdx;
}


/* Return string representation of the move */
void MoveToString(MOVE move, char *moveStringBuffer) {
  int fromIdx = move / 100;
  int toIdx   = move % 100;


  char fromBuf[8], toBuf[8];
  index_to_coord(fromIdx, fromBuf);
  index_to_coord(toIdx, toBuf);


  snprintf(moveStringBuffer, 32, "%s %s", fromBuf, toBuf);
}


void PrintMove(MOVE move) {
  char moveStringBuffer[32];
  MoveToString(move, moveStringBuffer);
  printf("%s", moveStringBuffer);
}


void PrintComputersMove(MOVE computersMove, STRING computersName) {
  printf("%s's move: ", computersName);
  PrintMove(computersMove);
  printf("\n");
}


/*********** END TEXTUI FUNCTIONS ***********/


















/*********** BEGIN VARIANT FUNCTIONS ***********/


/* How many variants are you supporting? */
int NumberOfOptions() {
  return 3;
}


/* Return the current variant id. */
int getOption() {
  return variant_length;
}


/* The input is a variant id. This function sets any global variables
or data structures according to the variant specified by the variant id. */
void setOption(int option) {
  variant_length = option;
  ROWS = 3; // always 3 rows for Hexapawn
  COLS = variant_length;
  CELLS = ROWS * COLS;
  boardSize = CELLS;


}


/**
* @brief Interactive menu used to change the variant, i.e., change
* game-specific parameters, such as the side-length of a tic-tac-toe
* board, for example. Does nothing if kGameSpecificMenu == FALSE.
*/
void GameSpecificMenu(void) {
  int option = 0;
  printf("\n========== Hexapawn Variant Menu ==========\n");
  printf("1. Standard 3x3 Hexapawn\n");
  printf("2. Extended Nx3 Hexapawn ( N < 10\n");
  printf("===========================================\n");
  printf("Select a variant [1-2]: ");




  if (scanf("%d", &option) != 1) {
      printf("Invalid input. Keeping current variant (%dx3).\n", variant_length);
      while (getchar() != '\n'); // clear input buffer
      return;
  }
  switch (option) {
      case 1:
          setOption(3);
          printf("Variant set to Standard 3x3.\n");
          break;


      case 2: {
          int n = 0;
          printf("Enter board length N (3 ≤ N < 10): ");
          if (scanf("%d", &n) != 1 || n < 3 || n >= 10) {
              printf("Invalid N. Keeping current variant (%dx3).\n", variant_length);
              while (getchar() != '\n'); // clear input buffer
              return;
          }
          setOption(n);
          printf("Variant set to %dx3 Hexapawn.\n", n);
          break;
      }


      default:
          printf("Invalid choice. Keeping current variant (%dx3).\n", variant_length);
          break;
  }
  InitializeGame();
}


/*********** END VARIANT-RELATED FUNCTIONS ***********/


POSITION StringToPosition(char *positionString) {
  int turn;
  char *board;
  if (ParseStandardOnelinePositionString(positionString, &turn, &board)) {
    return generic_hash_hash(board, turn);
  }
  return NULL_POSITION;
}


void PositionToAutoGUIString(POSITION position, char *autoguiPositionStringBuffer) {
  char board[CELLS + 1];
  generic_hash_unhash(position, board);
  board[CELLS] = '\0';
  AutoGUIMakePositionString(generic_hash_turn(position), board, autoguiPositionStringBuffer);
}


void MoveToAutoGUIString(POSITION position, MOVE move, char *autoguiMoveStringBuffer) {
  (void) position;
  char board[CELLS];
  generic_hash_unhash(position, board);
  int origin = move / 100;
  int target = move % 100;
  char sound = (board[target] == '-') ? 'x' : 'y';
  AutoGUIMakeMoveButtonStringM(origin, target, sound, autoguiMoveStringBuffer);
}

