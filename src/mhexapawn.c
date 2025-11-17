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
CONST_STRING kGameName = "Hexapawn";
CONST_STRING kDBName = "hexapawn";
POSITION gNumberOfPositions;
POSITION gInitialPosition;
BOOLEAN kPartizan = TRUE;
BOOLEAN kTieIsPossible = FALSE;
BOOLEAN kLoopy = FALSE;
BOOLEAN kSupportsSymmetries = FALSE;
POSITION GetCanonicalPosition(POSITION);
POSITION kBadPosition = -1;
BOOLEAN kDebugDetermineValue = FALSE;
void* gGameSpecificTclInit = NULL;
BOOLEAN kGameSpecificMenu = TRUE;
BOOLEAN kDebugMenu = FALSE;
CONST_STRING kHelpGraphicInterface = "";

CONST_STRING kHelpTextInterface = "MOVING YOUR PAWN: \n First enter the column and then row of the piece (of your color) that you wish to move.\nThen, without entering a space, enter the desired column and row of where you want to place your pawn.";

CONST_STRING kHelpOnYourTurn = "Move one of your pawns to an empty position 'ahead' of you or diagonally to capture a pawn of the opposing color.";

CONST_STRING kHelpStandardObjective = "Advance a pawn to the far rank or block opponent so they cannot move.";
CONST_STRING kHelpReverseObjective = "";
CONST_STRING kHelpTieOccursWhen = "";
CONST_STRING kHelpExample = "";


void DebugMenu() {}
void SetTclCGameSpecificOptions(int theOptions[]) {
  (void)theOptions;
}

/*********** BEGIN SOLVING FUNCIONS ***********/
int variant_length = 0;
int ROWS = 3;
int COLS = 0;
int CELLS = 0;
int MISERE = 0; // 0 = normal, 1 = misère
int ID = 0;

static inline int in_bounds(int i) {
    return (i >= 0 && i < CELLS);
}

void InitializeGame() {
  if (variant_length <= 0) {
    setOption(3);
  }

  gCanonicalPosition = GetCanonicalPosition;
  int piecesArray[] = {'B', 0, variant_length, 'W', 0, variant_length, '-', 0, CELLS, -1};
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

        if (sideToMove == 1) {
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

        } else {
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
*/
  char board[CELLS];
  generic_hash_unhash(position, board);
  int turn = generic_hash_turn(position);
  // Zero Pawn Check
  int whiteCount = 0, blackCount = 0;
  for (int i = 0; i < CELLS; i++) {
    if (board[i] == 'W')
        whiteCount++;
    else if (board[i] == 'B')
        blackCount++;
  }



    #define FLIP(value) ((MISERE) ? ((value == win) ? lose : win) : (value))

    // Zero Pawn Check
    if (whiteCount == 0) return FLIP((turn == 1) ? lose : win);
    if (blackCount == 0) return FLIP((turn == 2) ? lose : win);

    // Pawn Reaches opposite side
    for (int c = 0; c < COLS; c++) {
        if (board[c] == 'W') return FLIP((turn == 1) ? win : lose);
        if (board[(ROWS-1)*COLS + c] == 'B') return FLIP((turn == 2) ? win : lose);
    }

    // No legal moves
    if (!HasAnyMove(board, turn)) return FLIP(lose);

    return undecided;
}

POSITION GetCanonicalPosition(POSITION position) {
  return position;
}

/*********** END SOLVING FUNCTIONS ***********/

/*********** BEGIN TEXTUI FUNCTIONS ***********/

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
 for (int r = ROWS - 1; r >= 0; r--) {
    if(r == 1) printf("\tBASE:   ");
    else printf("\t\t");
   printf(" %d  ", r + 1);

   for (int c = 0; c < COLS; c++) {
     putchar(start[r * COLS + c]);
     if (c + 1 < COLS) putchar(' ');
   }
   printf("\t");
    if(r == 1) printf("\tGAMEPLAY:   \t");
    else printf("\t\t\t");
   printf("%d  ", r + 1);
   for (int c = 0; c < COLS; c++) {
     putchar(board[r * COLS + c]);
     if (c + 1 < COLS) putchar(' ');
   }
   putchar('\n');
 }
 printf("\t\t    ");
 for (int c = 0; c < COLS; c++) printf("%c ", 'z' - variant_length + c + 1);
 if (variant_length == 6) printf("\t\t\t   ");
 else printf("\t\t\t\t   ");
 for (int c = 0; c < COLS; c++) printf("%c ", 'z' - variant_length + c + 1);
 printf("\n");
 printf("\n\t%s to move\n\n", (turn == 1) ? "White (W)" : "Black (B)");
 printf("PREDICTION: %s\n", GetPrediction(position, playerName, usersTurn));
 printf("==== EXAMPLE MOVE: x3x2 ====\n\n");
 free(start);
}

USERINPUT GetAndPrintPlayersMove(POSITION position, MOVE *move, STRING playerName) {
  USERINPUT ret;
  do {
    printf("%8s's move [(u)ndo]/[<origin><target>] :  ", playerName);
    ret = HandleDefaultTextInput(position, move, playerName);
    if (ret != Continue) return ret;
  } while (TRUE);
  return Continue;
}

BOOLEAN ValidTextInput(STRING input) {
  if (strlen(input) != 4) return FALSE;
  if (input[0] < ('x' - variant_length + 1) || input[0] > 'z') return FALSE;
  if (input[2] < ('x' - variant_length + 1) || input[2] > 'z') return FALSE;
  if (input[1] < '1' || input[1] > '3') return FALSE;
  if (input[3] < '1' || input[3] > '3') return FALSE;
  return TRUE;
}

MOVE ConvertTextInputToMove(STRING input) {
  int originRow = input[1] - '1';
  int originCol = input[0] - ('z' - variant_length + 1);
  int targetRow = input[3] - '1';
  int targetCol = input[2] - ('z' - variant_length + 1);
  int origin = originRow * variant_length + originCol;
  int target = targetRow * variant_length + targetCol;
  return origin * 100 + target;
}

void MoveToString(MOVE move, char *moveStringBuffer) {
  int origin = move / 100;
  int target = move % 100;
  int originRow = origin / variant_length;
  int originCol = origin % variant_length;
  int targetRow = target / variant_length;
  int targetCol = target % variant_length;
  snprintf(moveStringBuffer, 6, "%c%d%c%d",
           'z' - variant_length + originCol + 1, originRow + 1,
           'z' - variant_length + targetCol + 1, targetRow + 1);
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
  return 8;
}

/* Return the current variant id. */
int getOption() {
  return ID;
}

/* The input is a variant id. This function sets any global variables
or data structures according to the variant specified by the variant id. */
void setOption(int option) {
  variant_length = option % 4 + 3;
  ID = option;
  ROWS = 3;
  COLS = variant_length;
  CELLS = ROWS * COLS;
  MISERE = option > 3;
}

/**
* @brief Interactive menu used to change the variant, i.e., change
* game-specific parameters, such as the side-length of a tic-tac-toe
* board, for example. Does nothing if kGameSpecificMenu == FALSE.
*/
void GameSpecificMenu(void) {
    int option = 0;
    while (1) { // keep asking until valid input
        printf("\n========== Hexapawn Variant Menu ==========\n");
        printf("VARIANTS:\n");
        printf("0: LENGTH 3, REGULAR\n");
        printf("1: LENGTH 4, REGULAR\n");
        printf("2: LENGTH 5, REGULAR\n");
        printf("3: LENGTH 6, REGULAR\n");
        printf("4: LENGTH 3, MISERE\n");
        printf("5: LENGTH 4, MISERE\n");
        printf("6: LENGTH 5, MISERE\n");
        printf("7: LENGTH 6, MISERE\n");
        printf("===========================================\n");
        printf("SELECT VARAINT: ");

        if (scanf("%d", &option) != 1) {
            printf("Invalid input. Try again.\n");
            while (getchar() != '\n');
            continue;
        }
        while (getchar() != '\n');

        if (option <= 7) {
            setOption(option);
            printf("Variant set to %dx3 board.\n", variant_length);
            break;
        } else {
            printf("Invalid option. Please enter a number between [0, 7].\n");
        }
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
