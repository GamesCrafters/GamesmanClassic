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
CONST_STRING kAuthorName = "Andy Kim Aj DeMarinis Benjamin Zimmerman";
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
#define NEXTPLAYER(i) (i == 1 ? 2 : 1)

/*********** BEGIN SOLVING FUNCIONS ***********/

void InitializeGame(void) {
    gCanonicalPosition = GetCanonicalPosition;
    int pieceList[] = {'w', 4, 4, 'b', 4, 4, '-', 8, 8, -1};
    gNumberOfPositions = generic_hash_init(16, pieceList, NULL, 0);
    gInitialPosition = generic_hash_hash("bwbw--------wbwb", 1);
    gPositionToStringFunPtr = &PositionToString;
}

MOVELIST *GenerateMoves(POSITION position) {
  MOVELIST *moves = NULL;
  char* board = (char*) SafeMalloc(16 * sizeof(char));
  board = generic_hash_unhash(position, board);
  char piece = (generic_hash_turn(position) == 1) ? WCHAR : BCHAR;
  for (int idx = 0; idx < 16; idx++) {
    if (board[idx] != piece) {
      continue;
    }
    if (idx < 12 && board[idx + 4] == ECHAR) { // up
      int new_move = (idx << 4) | ((idx + 4) & 0x0F);
      moves = CreateMovelistNode(new_move, moves);
    }
    if (idx > 3 && board[idx - 4] == ECHAR) { // down
      int new_move = (idx << 4) | ((idx - 4) & 0x0F);
      moves = CreateMovelistNode(new_move, moves);
    }
    if (idx < 15 && idx % 4 != 3 && board[idx + 1] == ECHAR) { // right
      int new_move = (idx << 4) | ((idx + 1) & 0x0F);
      moves = CreateMovelistNode(new_move, moves);
    }
    if (idx > 0 && idx % 4 != 0 && board[idx - 1] == ECHAR) { // left
      int new_move = (idx << 4) | ((idx - 1) & 0x0F);
      moves = CreateMovelistNode(new_move, moves);
    }
  }
  SafeFree(board);
  return moves;
}

POSITION DoMove(POSITION position, MOVE move) {
    char* board = (char*) SafeMalloc(16 * sizeof(char));
    board = generic_hash_unhash(position, board);
    int curr_player = generic_hash_turn(position);
    board[move & 0x0F] = board[(move >> 4) & 0x0F];
    board[(move >> 4) & 0x0F] = ECHAR;
    curr_player = NEXTPLAYER(curr_player);
    POSITION child_position = generic_hash_hash(board, curr_player);
    SafeFree(board);
    return child_position;
}

VALUE Primitive(POSITION position) {
    char* p_str = (char*)SafeMalloc(16 * sizeof(char));
    p_str = generic_hash_unhash(position, p_str);
    int deciding_idx = -1;
    for (int idx = 0; idx < 8; idx ++) {
        if (p_str[idx] == ECHAR) {
            continue;
        }
        if(p_str[idx] == p_str[idx + 4] && p_str[idx + 4] == p_str[idx + 8]) { // explore down
            deciding_idx = idx;
            break;
        }
        if (idx % 4 > 1) { // explore left
            if(p_str[idx] == p_str[idx - 1] && p_str[idx - 1] == p_str[idx - 2]) { // left
                deciding_idx = idx;
                break;
            }
            if(p_str[idx] == p_str[idx + 3] && p_str[idx + 3] == p_str[idx + 6]) { // left diagonal
                deciding_idx = idx;
                break;
            }
        }
        if (idx % 4 < 2) { // explore right 
            if(p_str[idx] == p_str[idx + 1] && p_str[idx + 1] == p_str[idx + 2]) { // right
                deciding_idx = idx;
                break;
            }
            if(p_str[idx] == p_str[idx + 5] && p_str[idx + 5] == p_str[idx + 10]) { // right diagonal
                deciding_idx = idx;
                break;
            }
        }
    }
    // check bottom 2 rows manually
    if (p_str[9] != ECHAR && (p_str[9] == p_str[10]) && (p_str[8] == p_str[9] || p_str[10] == p_str[11])) {
      deciding_idx = 9;
    }
    if (p_str[13] != ECHAR && (p_str[13] == p_str[14]) && (p_str[12] == p_str[13] || p_str[14] == p_str[15])) {
      deciding_idx = 13;
    }
    if (deciding_idx != -1) {
      SafeFree(p_str);
      return lose; 
    }
    SafeFree(p_str);
    return undecided;
}

POSITION GetCanonicalPosition(POSITION position) {
    return position;
}

/*********** END SOLVING FUNCTIONS ***********/
/*********** BEGIN TEXTUI FUNCTIONS ***********/

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn) {
    char* board = (char*)SafeMalloc(16 * sizeof(char));
    generic_hash_unhash(position, board);
    // int player = generic_hash_turn(position);
    char* prediction = GetPrediction(position, playerName, usersTurn);
    printf("\n         (  1  2  3  4 )           : %c %c %c %c\n",
	       board[0],
	       board[1],
	       board[2],
	       board[3]);
	printf("         (  5  6  7  8 )           : %c %c %c %c\n",
	       board[4],
	       board[5],
	       board[6],
	       board[7]);
	printf("LEGEND:  (  9 10 11 12 )   TOTAL:  : %c %c %c %c\n",
	       board[8],
	       board[9],
	       board[10],
	       board[11]);
	printf("         ( 13 14 15 16 )           : %c %c %c %c %s\n\n",
	       board[12],
	       board[13],
	       board[14],
           board[15],
	       prediction);

    SafeFree(board);
}

USERINPUT GetAndPrintPlayersMove(POSITION position, MOVE *move, STRING playerName) {
    USERINPUT ret;
    do {
        /* List of available moves */
        // Modify the player's move prompt as you wish
        printf("%8s's move: ", playerName);
        ret = HandleDefaultTextInput(position, move, playerName);
        if (ret != Continue) return ret;
    } while (TRUE);
    return (Continue); /* this is never reached, but lint is now happy */
}

BOOLEAN ValidTextInput(STRING input) {
    int start_pos, end_pos;
    char extra;

    if (sscanf(input, "%d %d %c", &start_pos, &end_pos, &extra) != 2) {
        return FALSE;  
    }

    if (start_pos <= 0 || start_pos > 16 || end_pos <= 0 || end_pos > 16) {
        return FALSE;
    }

    if (start_pos == end_pos) {
        return FALSE;
    }

    return TRUE;
}

MOVE ConvertTextInputToMove(STRING input) {
    if (ValidTextInput(input) == FALSE) {
        return 0;
    }
    int start_pos, end_pos;
    
    sscanf(input, "%d %d", &start_pos, &end_pos);
    
    MOVE encoded_move = (((start_pos - 1) & 0x0F) << 4) | ((end_pos - 1) & 0x0F);
    
    return encoded_move;
}

void MoveToString(MOVE move, char *moveStringBuffer) {
    int start_pos = ((move >> 4) & 0x0F) + 1; 
    int end_pos = (move & 0x0F) + 1;          
    
    snprintf(moveStringBuffer, MAX_MOVE_STRING_LENGTH, "%d %d", start_pos, end_pos);

    moveStringBuffer[MAX_MOVE_STRING_LENGTH - 1] = '\0';
}

void PrintComputersMove(MOVE computersMove, STRING computersName) {
    char moveStringBuffer[32];
    MoveToString(computersMove, moveStringBuffer);
    printf("%s's move: %s\n", computersName, moveStringBuffer);
}

void DebugMenu(void) {}

/*********** END TEXTUI FUNCTIONS ***********/
/*********** BEGIN VARIANT FUNCTIONS ***********/

int NumberOfOptions(void) {
    return 1;
}

int getOption(void) {
  return 0;
}

void setOption(int option) {
  (void) option;
}

void GameSpecificMenu(void) {}

/*********** END VARIANT-RELATED FUNCTIONS ***********/

void PositionToString(POSITION position, char *positionStringBuffer) {
  positionStringBuffer = generic_hash_unhash(position, positionStringBuffer);
}

POSITION StringToPosition(char *positionString) {
	int turn;
	char *board;
	if (ParseStandardOnelinePositionString(positionString, &turn, &board)) {
		return 0;
	}
	return NULL_POSITION;
}

void PositionToAutoGUIString(POSITION position, char *autoguiPositionStringBuffer) {
  char* board = (char*)SafeMalloc(17 * sizeof(char*));
  board = generic_hash_unhash(position, board);
  board[16] = '\0';
  int turn = generic_hash_turn(position);
  AutoGUIMakePositionString(turn, board, autoguiPositionStringBuffer);
}

void MoveToAutoGUIString(POSITION position, MOVE move, char *autoguiMoveStringBuffer) {
  (void) position; 
  AutoGUIMakeMoveButtonStringA('h', move, 'x', autoguiMoveStringBuffer);
}
