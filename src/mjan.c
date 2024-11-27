/************************************************************************
**
** NAME:        mjan.c
**
** DESCRIPTION: Jan 4x4 (A Simple Game)
**
** AUTHOR:      Andy Tae Kyu Kim, Andrew Jacob DeMarinis, Benjamin Riley Zimmerman
**
** DATE:        WIP
**
************************************************************************/

#include "gamesman.h" 

// system metadata
CONST_STRING kAuthorName = "Andy Kim, Aj DeMarinis, and Benjamin Zimmerman";
CONST_STRING kDBName     = "jan4x4";
CONST_STRING kGameName   = "Jan4x4";

// position data for solver
POSITION kBadPosition       = -1;
POSITION gInitialPosition   = 0;
POSITION gNumberOfPositions = 0;

// game metadata
BOOLEAN kDebugDetermineValue = FALSE;
BOOLEAN kDebugMenu           = FALSE;
BOOLEAN kGameSpecificMenu    = FALSE;
BOOLEAN kLoopy               = TRUE;
BOOLEAN kPartizan            = FALSE;
BOOLEAN kSupportsSymmetries  = TRUE;
BOOLEAN kTieIsPossible       = FALSE;

// game functions for solver
POSITION GetCanonicalPosition(POSITION);
void PositionToString(POSITION, char*);

// helper menu text
CONST_STRING kHelpExample           = "TODO";
CONST_STRING kHelpGraphicInterface  = "TODO";
CONST_STRING kHelpOnYourTurn        = "Move one of your pieces horizontally or vertically into an open space.";
CONST_STRING kHelpReverseObjective  = "Avoid aligning three of your pieces in any direction";
CONST_STRING kHelpStandardObjective = "Align three of your pieces in any direction.";
CONST_STRING kHelpTextInterface     = "TODO";
CONST_STRING kHelpTieOccursWhen     = "There are no ties in Jan4x4.";

// tcl frontend setup
void *gGameSpecificTclInit = NULL;
void SetTclCGameSpecificOptions(int theOptions[]) { (void)theOptions; }

// helpful definitions
#define WCHAR 'w'
#define BCHAR 'b'
#define ECHAR '-'
#define BOARDSIZE 16
#define ENCODEMOVE(idx, diff) ((idx << 4) | ((idx + diff) & 0x0F))
#define DECODEMOVEIDX(move) ((move >> 4) & 0x0F)
#define DECODEMOVEJDX(move) (move & 0x0F)
#define NEXTPLAYER(player) (player == 1 ? 2 : 1)

// intializes neccessary game variables.
// 
// there are always eight pieces on the board,
// four white pieces and four black pieces
//
// example jan4x4 boards:
//
// "bwbw--------wbwb" -> bwbw
//                       ----
//                       ----
//                       wbwb
//
// "-wbwb-------wbwb" -> -wbw
//                       b---
//                       ----
//                       wbwb
//
// "---wbwb---w-wb-b" -> ---w
//                       bwb-
//                       --w-
//                       wb-b
void InitializeGame(void) {
  int pieceList[] = {'w', 4, 4, 'b', 4, 4, '-', 8, 8, -1};
  gNumberOfPositions = generic_hash_init(16, pieceList, NULL, 0);

  gInitialPosition = generic_hash_hash("bwbw--------wbwb", 1);
  gCanonicalPosition = GetCanonicalPosition;
  gPositionToStringFunPtr = NULL;
}

// generates all possible moves for the current player.
//
// example movelists:
// 
// player: b
//   bwbw      bwbw
//   ----  ->  *-*-
//   ----  ->  -*-*
//   wbwb      wbwb
//
// player: w
//   -wbw      *wbw
//   b---  ->  b*-*
//   ----  ->  *-*-
//   wbwb      wbwb
//
// player: b 
//   ---w      *-*w
//   bwb-  ->  bwb*
//   --w-  ->  **w-
//   wb-b      wb*b
MOVELIST *GenerateMoves(POSITION position) {
  MOVELIST *moves = NULL;

  char board[BOARDSIZE];
  generic_hash_unhash(position, board);

  char piece = (generic_hash_turn(position) == 1) ? WCHAR : BCHAR;
  int diffs[4] = {4, -4, 1, -1}; // up down right left

  for (int idx = 0; idx < 16; idx ++) {
    if (board[idx] != piece)
      continue;

    for (int cnt = 0; cnt < 4; cnt ++) {
      int jdx = idx + diffs[cnt];
      if (jdx >= 0 && jdx < BOARDSIZE && board[jdx] == ECHAR && ((diffs[cnt] == -4 || diffs[cnt] == 4) || jdx / 4 == idx / 4))
        moves = CreateMovelistNode(ENCODEMOVE(idx, diffs[cnt]), moves);
    }
  }

  return moves;
}

// enacts a move on a board, returning the resulting position.
POSITION DoMove(POSITION position, MOVE move) {
  char board[BOARDSIZE];
  generic_hash_unhash(position, board);

  board[DECODEMOVEJDX(move)] = board[DECODEMOVEIDX(move)];
  board[DECODEMOVEIDX(move)] = ECHAR;

  return generic_hash_hash(board, NEXTPLAYER(generic_hash_turn(position)));
} 

// parses a position and returns if the position is a loss, or undecided.
//
// example results:
// 
//  bwbw
//  ----  -> undecided
//  ----  ->
//  wbwb
//
//  --bw      --bw
//  bw--  ->  *w--  -> loss
//  -bw-  ->  -*w-  ->
//  w-b-      w-*-
//
//  -w-b      -*-b
//  bwb-  ->  b*b-  -> loss
//  -w--  ->  -*--  ->
//  wb--      wb--
VALUE Primitive(POSITION position) {
  char board[BOARDSIZE];
  generic_hash_unhash(position, board);

  // checks all diags and vertical loss conditions + horizontal loss conditions for first two rows
  for (int idx = 0; idx < BOARDSIZE / 2; idx ++) {
    if (board[idx] == ECHAR)
      continue;

    if (board[idx] == board[idx + 4] && board[idx + 4] == board[idx + 8]) // down
      return lose;

    if (idx % 4 > 1 && board[idx] == board[idx - 1] && board[idx - 1] == board[idx - 2]) // left
      return lose;

    if (idx % 4 > 1 && board[idx] == board[idx + 3] && board[idx + 3] == board[idx + 6]) // left diagonal
      return lose;

    if (idx % 4 < 2 && board[idx] == board[idx + 1] && board[idx + 1] == board[idx + 2]) // right
      return lose;

    if (idx % 4 < 2 && board[idx] == board[idx + 5] && board[idx + 5] == board[idx + 10]) // right diagonal
      return lose;
  }

  // check bottom 2 rows manually
  if (board[9] != ECHAR && (board[9] == board[10]) && (board[8] == board[9] || board[10] == board[11]))
    return lose;

  if (board[13] != ECHAR && (board[13] == board[14]) && (board[12] == board[13] || board[14] == board[15]))
    return lose;

  return undecided;
}

// for symmetries, effectively blank
POSITION GetCanonicalPosition(POSITION position) {
  return position;
}

/*********** END SOLVING FUNCTIONS ***********/
/*********** BEGIN TEXTUI FUNCTIONS ***********/

// prints a visually digestable position representation
// with legend and prediction.
void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn) {
  char board[BOARDSIZE];
  generic_hash_unhash(position, board);

  printf("\n         (  1  2  3  4 )           : %c %c %c %c\n", // row one
	  board[0],
	  board[1],
	  board[2],
	  board[3]
  );
	
  printf("         (  5  6  7  8 )           : %c %c %c %c\n", // row two
	  board[4],
	  board[5],
	  board[6],
	  board[7]
  );
	
  printf("LEGEND:  (  9 10 11 12 )   TOTAL:  : %c %c %c %c\n", // row three
	  board[8],
	  board[9],
	  board[10],
	  board[11]
  );

  printf("         ( 13 14 15 16 )           : %c %c %c %c %s\n\n", // row four
	  board[12],
	  board[13],
	  board[14],
    board[15],
	  GetPrediction(position, playerName, usersTurn)
  );
}

// prints the players move in a digestable format.
USERINPUT GetAndPrintPlayersMove(POSITION position, MOVE *move, STRING playerName) {
  USERINPUT ret;
  do {
    printf("%8s's move: ", playerName);
    ret = HandleDefaultTextInput(position, move, playerName);
    if (ret != Continue) return ret;
  } while (TRUE);
  return (Continue); // for linter
}

// checks if an input is in a valid format to be considered a move.
BOOLEAN ValidTextInput(STRING input) {
  int idx, jdx, diff;
  char ext;
  
  if (sscanf(input, "%d %d %c", &idx, &jdx, &ext) != 2)
    return FALSE;

  if (idx <= 0 || idx > BOARDSIZE || jdx <= 0 || jdx > BOARDSIZE)
    return FALSE;

  diff = jdx - idx;
  if (diff != 4 && diff != -4 && diff != 1 && diff != -1)
    return FALSE;

  return TRUE;
}

// converts a valid input into a move.
MOVE ConvertTextInputToMove(STRING input) {
  if (!ValidTextInput(input))
    return 0;

  int idx, jdx;
  sscanf(input, "%d %d", &idx, &jdx); 

  return ENCODEMOVE((idx - 1), (jdx - idx));
}

// converts a move into string output.
void MoveToString(MOVE move, char *moveStringBuffer) {    
  snprintf(moveStringBuffer, MAX_MOVE_STRING_LENGTH, "%d %d", DECODEMOVEIDX(move) + 1, DECODEMOVEJDX(move) + 1);
  moveStringBuffer[MAX_MOVE_STRING_LENGTH - 1] = '\0';
}

// prints computers move
void PrintComputersMove(MOVE computersMove, STRING computersName) {
  char moveStringBuffer[32];
  MoveToString(computersMove, moveStringBuffer);
  printf("%s's move: %s\n", computersName, moveStringBuffer);
}

// kDebugMenu = FALSE -> no purpose
void DebugMenu(void) {}

/*********** END TEXTUI FUNCTIONS ***********/
/*********** BEGIN VARIANT FUNCTIONS ***********/

// as of this version, only standard exists
int NumberOfOptions(void) {
    return 1;
}
int getOption(void) {
  return 0;
}
void setOption(int option) {
  (void) option;
}

// kGameSpecificMenu = FALSE -> no purpose
void GameSpecificMenu(void) {}

/*********** END VARIANT-RELATED FUNCTIONS ***********/
/***********    BEGIN AUTOGUI FUNCTIONS    ***********/

// converts formal position strings to position hashes
POSITION StringToPosition(char *positionString) {
	int turn;
	char *board;

	if (ParseStandardOnelinePositionString(positionString, &turn, &board)) {
		return generic_hash_hash(board, turn);
	}

	return NULL_POSITION;
}

// converts position hashes to AutoGUI position strings
void PositionToAutoGUIString(POSITION position, char *autoguiPositionStringBuffer) {
  char board[BOARDSIZE + 1];
  generic_hash_unhash(position, board);
  board[BOARDSIZE] = '\0';
  
  AutoGUIMakePositionString(generic_hash_turn(position), board, autoguiPositionStringBuffer);
}

// converts internal representations of moves to AutoGUI move strings
void MoveToAutoGUIString(POSITION position, MOVE move, char *autoguiMoveStringBuffer) {
  (void) position;

  AutoGUIMakeMoveButtonStringM(DECODEMOVEIDX(move), DECODEMOVEJDX(move), 's', autoguiMoveStringBuffer);
}
