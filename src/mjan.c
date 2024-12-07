/************************************************************************
**
** NAME:        mjan.c
**
** DESCRIPTION: Jan 4x4 (A Simple Game)
**
** AUTHOR:      Andy Tae Kyu Kim, Andrew Jacob DeMarinis,
**              Benjamin Riley Zimmerman
**
** DATE:        12/4/2024
**
************************************************************************/

#include "gamesman.h"

/************************************************************************
**
** Important system metadata
**
************************************************************************/

CONST_STRING kDBName     = "jan4x4";
CONST_STRING kGameName   = "Jan4x4";
CONST_STRING kAuthorName = "Andy Kim, Aj DeMarinis, and Benjamin Zimmerman";


/************************************************************************
**
** Positional data for the solver
**
************************************************************************/

POSITION kBadPosition       = -1;
POSITION gInitialPosition   = 0; // set later
POSITION gNumberOfPositions = 0; // set later


/************************************************************************
**
** Jan4x4 game metadata
**
************************************************************************/

BOOLEAN kLoopy         = TRUE;
BOOLEAN kPartizan      = FALSE;
BOOLEAN kTieIsPossible = FALSE;


/************************************************************************
**
** Feature gates
**
************************************************************************/

BOOLEAN kDebugMenu           = FALSE;
BOOLEAN kDebugDetermineValue = FALSE;
BOOLEAN kGameSpecificMenu    = TRUE;
BOOLEAN kSupportsSymmetries  = FALSE;
void*   gGameSpecificTclInit = NULL;


/************************************************************************
**
** Help menu content
**
************************************************************************/

CONST_STRING kHelpExample =
  "TODO";

CONST_STRING kHelpGraphicInterface =
  "TODO";

CONST_STRING kHelpOnYourTurn =
  "Move one of your pieces horizontally or vertically into an open space.\n\
  If diagonal moves are enabled, you can make diagonal moves accordingly.";

CONST_STRING kHelpReverseObjective =
  "Avoid aligning three of your pieces in any direction";

CONST_STRING kHelpStandardObjective =
  "Align three of your pieces in any direction.";

CONST_STRING kHelpTextInterface =
  "TODO";

CONST_STRING kHelpTieOccursWhen =
  "There are no ties in Jan4x4 or its variants, \n\
  but misere has draws!";


/************************************************************************
**
** Function prototypes, helpful definitions, and variants
**
************************************************************************/

void     PositionToString     (POSITION, char *);
POSITION GetCanonicalPosition (POSITION);
VALUE    FourByFourPrimitive  (POSITION);


#define WHITE 'w'
#define BLACK 'b'
#define EMPTY '-'

#define ROWS       4
#define COLS       4
#define BOARDSIZE 16

#define NEXTPLAYER(player)   (player == 1 ? 2 : 1)

#define ENCODEMOVE(from, to) ((from << 4) | (to & 0x0F))
#define DECODEMOVEFROM(move) ((move >> 4) & 0x0F)
#define DECODEMOVETO(move)   (move & 0x0F)

// works when ROWS = COLS
#define ISMAINDIAG(idx)      (idx / ROWS == idx % COLS || idx / ROWS + idx % COLS == COLS - 1)


// variants
BOOLEAN gAllDiag  = FALSE;
BOOLEAN gMainDiag = FALSE;


/************************************************************************
**
** NAME:        IntitializeGame
**
** DESCRIPTION: Sets up generic hash.
**
************************************************************************/

void InitializeGame(void) {
  int pieceList[]    = {'w', 4, 4, 'b', 4, 4, '-', 8, 8, -1};
  gNumberOfPositions = generic_hash_init(16, pieceList, NULL, 0);

  gInitialPosition   = generic_hash_hash("bwbw--------wbwb", 1);
  gCanonicalPosition = GetCanonicalPosition;

  gPositionToStringFunPtr = NULL;
}


/************************************************************************
**
** NAME:        GenerateMoves
**
** DESCRIPTION: Create a linked list of every move that can be reached
**              from this position. Return a pointer to the head of the
**              linked list.
**
** INPUTS:      POSITION position : The position to branch off of.
**
** OUTPUTS:     (MOVELIST *), a pointer that points to the first item
**              in the linked list of moves that can be generated.
**
************************************************************************/

MOVELIST *GenerateMoves(POSITION position) {
  MOVELIST *moves = NULL;

  char board[BOARDSIZE];
  generic_hash_unhash(position, board);

  char piece = (generic_hash_turn(position) == 1) ? WHITE : BLACK;

  int diffs[8] = {4, -4, 1, -1, 5, -5, 3, -3};

  for (int idx = 0; idx < 16; idx++) {
    if (board[idx] != piece)
      continue;

    for (int cnt = 0; cnt < 8; cnt++) {
      int diff = diffs[cnt];
      int jdx = idx + diff;

      // Check if target square is empty
      if (board[jdx] != EMPTY)
        continue;

      // check bounds
      if (jdx < 0 || jdx >= BOARDSIZE)
        continue;

      // check if move stays within the same row for horizontal moves
      if ((diff == 1 || diff == -1) && jdx / 4 != idx / 4)
        continue;

      // check diagonal rules
      if ((diff == 5 || diff == -5 || diff == 3 || diff == -3) &&
         ((!gAllDiag && !gMainDiag)                            ||
         (abs(idx % 4 - jdx % 4) > 1)                          ||
         (gMainDiag && (!ISMAINDIAG(idx) || !ISMAINDIAG(jdx)))))
        continue;

      moves = CreateMovelistNode(ENCODEMOVE(idx, jdx), moves);
    }
  }
  return moves;
}


/************************************************************************
**
** NAME:        DoMove
**
** DESCRIPTION: Apply the move to the position.
**
** INPUTS:      POSITION position : The old position
**              MOVE     move     : The move to apply.
**
** OUTPUTS:     (POSITION) : The position that results after the move.
**
************************************************************************/

POSITION DoMove(POSITION position, MOVE move) {
  char board[BOARDSIZE];
  generic_hash_unhash(position, board);

  board[DECODEMOVETO(move)] = board[DECODEMOVEFROM(move)];
  board[DECODEMOVEFROM(move)] = EMPTY;

  return generic_hash_hash(board, NEXTPLAYER(generic_hash_turn(position)));
}


/************************************************************************
**
** NAME:        Primitive
**
** DESCRIPTION: Return the value of a position if it fulfills certain
**              'primitive' constraints, which are dependant on variant.
**
** INPUTS:      POSITION position : The position to inspect
**
** OUTPUTS:     (VALUE) an enum which is oneof: (win,lose,tie,undecided)
**
************************************************************************/

VALUE Primitive(POSITION position) {
  VALUE result = FourByFourPrimitive(position);
  if (result == lose)
    return gStandardGame ? lose : win;
  else
    return undecided;
}


/************************************************************************
**
** NAME:        FourByFourPrimitive
**
** DESCRIPTION: Return the value of a position if it fulfills certain
**              'primitive' constraints. For Jan4x4, this is consituted
**              as a three in a row win condition.
**
** INPUTS:      POSITION position : The position to inspect
**
** OUTPUTS:     (VALUE) an enum which is oneof: (win,lose,tie,undecided)
**
************************************************************************/

VALUE FourByFourPrimitive(POSITION position) {
  char board[BOARDSIZE];
  generic_hash_unhash(position, board);

  // checks all vertical and diagonals
  for (int idx = 0; idx < BOARDSIZE / 2; idx++) {
    if (board[idx] == EMPTY)
      continue;

    if (board[idx] == board[idx + 4] && board[idx + 4] == board[idx + 8]) // down
      return lose;

    if (idx % 4 > 1 &&
        board[idx] == board[idx - 1] && board[idx - 1] == board[idx - 2]) // left
      return lose;

    if (idx % 4 > 1 &&
        board[idx] == board[idx + 3] && board[idx + 3] == board[idx + 6]) // left diagonal
      return lose;

    if (idx % 4 < 2 &&
        board[idx] == board[idx + 1] && board[idx + 1] == board[idx + 2]) // right
      return lose;

    if (idx % 4 < 2 &&
        board[idx] == board[idx + 5] && board[idx + 5] == board[idx + 10]) // right diagonal
      return lose;
  }

  // check bottom 2 rows manually
  if (board[9] != EMPTY && (board[9] == board[10]) &&
      (board[8] == board[9] || board[10] == board[11]))
    return lose;

  if (board[13] != EMPTY && (board[13] == board[14]) &&
      (board[12] == board[13] || board[14] == board[15]))
    return lose;

  return undecided;
}


/************************************************************************
**
** NAME:        GetCanonicalPosition
**
** DESCRIPTION: Effectively blank, as symmetries have not been implemented.
**
** INPUTS:      POSITION position : The position return the canonical elt. of.
**
** OUTPUTS:     POSITION          : The canonical element of the set.
**
************************************************************************/

POSITION GetCanonicalPosition(POSITION position) { return position; }


/************************************************************************
**
** NAME:        PrintPosition
**
** DESCRIPTION: Print the position in a pretty format, including the
**              prediction of the game's outcome.
**
** INPUTS:      POSITION position   : The position to pretty print.
**              STRING   playerName : The name of the player.
**              BOOLEAN  usersTurn  : TRUE <==> it's a user's turn.
**
************************************************************************/

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn) {
  char board[BOARDSIZE];
  generic_hash_unhash(position, board);

  printf("\n         (  1  2  3  4 )           : %c %c %c %c\n", // row one
         board[0], board[1], board[2], board[3]);

  printf("         (  5  6  7  8 )           : %c %c %c %c\n", // row two
         board[4], board[5], board[6], board[7]);

  printf("LEGEND:  (  9 10 11 12 )   TOTAL:  : %c %c %c %c\n", // row three
         board[8], board[9], board[10], board[11]);

  printf("         ( 13 14 15 16 )           : %c %c %c %c %s\n\n", // row four
         board[12], board[13], board[14], board[15],
         GetPrediction(position, playerName, usersTurn));
}


/************************************************************************
**
** NAME:        PrintComputersMove
**
** DESCRIPTION: Nicely format the computers move.
**
** INPUTS:      MOVE   *computersMove : The computer's move.
**              STRING  computersName : The computer's name.
**
************************************************************************/

USERINPUT GetAndPrintPlayersMove(POSITION position, MOVE *move, STRING playerName) {
  USERINPUT ret;
  do {
    printf("%8s's move: ", playerName);
    ret = HandleDefaultTextInput(position, move, playerName);
    if (ret != Continue)
      return ret;
  } while (TRUE);
  return (Continue); // for linter
}


/************************************************************************
**
** NAME:        ValidTextInput
**
** DESCRIPTION: Return TRUE iff the string input is of the right 'form'.
**              In this case, correct form is two integers within the
**              range [1, BOARDSIZE] that denote a from and to idx,
**              seperated by a space.
**
** INPUTS:      STRING input : The string input the user typed.
**
** OUTPUTS:     BOOLEAN : TRUE iff the input is a valid text input.
**
************************************************************************/

BOOLEAN ValidTextInput(STRING input) {
  int idx, jdx;
  char ext;

  if (sscanf(input, "%d %d %c", &idx, &jdx, &ext) != 2)
    return FALSE;

  if (idx <= 0 || idx > BOARDSIZE || jdx <= 0 || jdx > BOARDSIZE)
    return FALSE;

  return TRUE;
}


/************************************************************************
**
** NAME:        ConvertTextInputToMove
**
** DESCRIPTION: Convert the string input to the internal move representation.
**
** INPUTS:      STRING input : The string input the user typed.
**
** OUTPUTS:     MOVE : The move corresponding to the user's input.
**
************************************************************************/

MOVE ConvertTextInputToMove(STRING input) {
  if (!ValidTextInput(input))
    return 0;

  int idx, jdx;
  sscanf(input, "%d %d", &idx, &jdx);

  return ENCODEMOVE((idx - 1), (jdx - 1));
}


/************************************************************************
**
** NAME:        MoveToString
**
** DESCRIPTION: Writes string representation of move to moveStringBuffer
**
** INPUTS:      MOVE *move         : The move to put into a string.
**              char *moveStringBuffer : Buffer to write movestring to
**
************************************************************************/

void MoveToString(MOVE move, char *moveStringBuffer) {
  sprintf(moveStringBuffer, "%d %d", DECODEMOVEFROM(move) + 1, DECODEMOVETO(move) + 1);
}


/************************************************************************
**
** NAME:        PrintComputersMove
**
** DESCRIPTION: Nicely format the computers move.
**
** INPUTS:      MOVE   *computersMove : The computer's move.
**              STRING  computersName : The computer's name.
**
************************************************************************/

void PrintComputersMove(MOVE computersMove, STRING computersName) {
  char moveStringBuffer[32];
  MoveToString(computersMove, moveStringBuffer);
  printf("%s's move: %s\n", computersName, moveStringBuffer);
}


/************************************************************************
**
** NAME:        DebugMenu
**
** DESCRIPTION: Menu used to debub internal problems. Does nothing if
**              kDebugMenu == FALSE
**
************************************************************************/

void DebugMenu(void) {}


/************************************************************************
**
** NAME:        NumberOfOptions
**
** DESCRIPTION: Returns the amount of variants currently supported.
**              As of this version, we support
**                 * standard
**                 * misere
**              with
**                 * no
**                 * main
**                 * all
**              diagonal moves.
**
** OUTPUTS:     int : Number of variants.
**
************************************************************************/

int NumberOfOptions() { return 6; }


/************************************************************************
**
** NAME:        getOption
**
** DESCRIPTION: Returns the integer representation of the current variant.
**              As of this version, we map
**                 1 -> * standard no diagonals
**                 2 -> * standard all diagonals
**                 3 -> * standard main diagonals
**                 4 -> * misere   no diagonals
**                 5 -> * misere   all diagonals
**                 6 -> * misere   main diagonals
**
** OUTPUTS:     int : Current variant as an integer.
**
************************************************************************/

int getOption() {
  int option = 1;
  if (gAllDiag)
    option = 2;
  if (gMainDiag)
    option = 3;
  return gStandardGame ? option : option + (NumberOfOptions() / 2);
}


/************************************************************************
**
** NAME:        GetVarString
**
** DESCRIPTION: Returns a string specific to the current variant.
**
** OUTPUTS:     STRING : Variant selection descriptor.
**
************************************************************************/

STRING GetVarString() {
  switch (getOption()) {
  case 1:
    return "{[Standard]/Misere} Win Condition {[None]/All/Main} Diagonal "
           "Moves";
  case 2:
    return "{[Standard]/Misere} Win Condition {None/[All]/Main} Diagonal "
           "Moves";
  case 3:
    return "{[Standard]/Misere} Win Condition {None/All/[Main]} Diagonal "
           "Moves";
  case 4:
    return "{Standard/[Misere]} Win Condition {[None]/All/Main} Diagonal "
           "Moves";
  case 5:
    return "{Standard/[Misere]} Win Condition {None/[All]/Main} Diagonal "
           "Moves";
  case 6:
    return "{Standard/[Misere]} Win Condition {None/All/[Main]} Diagonal "
           "Moves";
  default:
    BadElse("GetVarString");
    return "String not set for this option";
  }
}


/************************************************************************
**
** NAME:        setOption
**
** DESCRIPTION: Sets variant based on pased integer.
**              As of this version, we map
**                 1 -> * standard no diagonals
**                 2 -> * standard all diagonals
**                 3 -> * standard main diagonals
**                 4 -> * misere   no diagonals
**                 5 -> * misere   all diagonals
**                 6 -> * misere   main diagonals
**
** INPUTS:      int : Desired variant as integer.
**
************************************************************************/

void setOption(int option) {
  gStandardGame = option <= 3 ? TRUE : FALSE;
  switch (option % 3) {
  case 1:
    gAllDiag = gMainDiag = FALSE;
    break;
  case 2:
    gAllDiag = TRUE;
    gMainDiag = FALSE;
    break;
  case 0:
    gAllDiag = FALSE;
    gMainDiag = TRUE;
    break;
  default:
    BadElse("SetOption");
    break;
  }
}


/************************************************************************
**
** NAME:        GameSpecificMenu
**
** DESCRIPTION: Menu used to change game-specific parmeters, such as
**              the side of the board in an nxn Nim board, etc. Does
**              nothing if kGameSpecificMenu == FALSE
**
************************************************************************/

void GameSpecificMenu() {
  while (TRUE) {
    char GetMyChar();
    printf("\n");
    printf("        Jan 4x4 Game Toggle Menu\n");
    printf("        n)   [n]o diagonal moves (default)\n");
    printf("        a)   [a]ll diagonal moves\n");
    printf("        m)   [m]ain diagonal moves\n");
    printf("        b)   [b]ack to previous menu\n\n");

    printf("        Current option:   %s\n", GetVarString());
    printf("        Select an option: ");

    switch (GetMyChar()) {
    case 'n':
      gStandardGame ? setOption(1) : setOption(4);
      break;
    case 'a':
      gStandardGame ? setOption(2) : setOption(5);
      break;
    case 'm':
      gStandardGame ? setOption(3) : setOption(6);
      break;
    case 'B':
    case 'b':
      return;
    default:
      printf("\n        Sorry, I don't know that option. Try another.\n");
      break;
    }
  }
}

/************************************************************************
**
** NAME:        StringToPosition
**
** DESCRIPTION: Converts formal position strings to position hashes.
**
** INPUTS:      char*    : Position in string format.
**
** OUTPUTS:     POSITION : Position in integer format.
**
************************************************************************/

POSITION StringToPosition(char *positionString) {
  int turn;
  char *board;

  if (ParseStandardOnelinePositionString(positionString, &turn, &board)) {
    return generic_hash_hash(board, turn);
  }

  return NULL_POSITION;
}


/************************************************************************
**
** NAME:        StringToPosition
**
** DESCRIPTION: Converts position hashes to AutoGUI position strings.
**
** INPUTS:      POSITION : Position, in integer form, to convert.
**              char*    : Auto GUI string buffer for storage.
**
************************************************************************/

void PositionToAutoGUIString(POSITION position, char *autoguiPositionStringBuffer) {
  char board[BOARDSIZE + 1];
  generic_hash_unhash(position, board);
  board[BOARDSIZE] = '\0';

  AutoGUIMakePositionString(generic_hash_turn(position), board, autoguiPositionStringBuffer);
}


/************************************************************************
**
** NAME:        StringToPosition
**
** DESCRIPTION: Converts internal representations of moves to
**              AutoGUI move strings.
**
** INPUTS:      MOVE  : Move to convert.
**              char* : Auto GUI string buffer for storage.
**
************************************************************************/

void MoveToAutoGUIString(POSITION position, MOVE move, char *autoguiMoveStringBuffer) {
  (void)position;

  AutoGUIMakeMoveButtonStringM(DECODEMOVEFROM(move), DECODEMOVETO(move), 's', autoguiMoveStringBuffer);
}
