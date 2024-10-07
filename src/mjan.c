/************************************************************************
**
** NAME:        mjan.c
**
** DESCRIPTION: Jan 4x4 (A Simple Game)
**
** AUTHORS:      Andrew Jacob DeMarinis, Andy Tae Kyu Kim, Benjamin Riley Zimmerman
**
** DATE:        10/23/2024
**
************************************************************************/

#include "gamesman.h" 

CONST_STRING kAuthorName = "Aj DeMarinis Andy Kim Benjamin Zimmerman";
CONST_STRING kGameName = "Jan4x4";
CONST_STRING kDBName = "jan4x4";

enum variant {
  STANDARD,
  MISERE
};

enum variant gVariant = STANDARD;

POSITION gNumberOfPositions = 0;
POSITION gInitialPosition = 0;
BOOLEAN kPartizan = FALSE;
BOOLEAN kTieIsPossible = FALSE;
BOOLEAN kLoopy = TRUE;
BOOLEAN kSupportsSymmetries = TRUE;
POSITION kBadPosition = -1;
BOOLEAN kDebugDetermineValue = FALSE;
POSITION GetCanonicalPosition(POSITION);
void PositionToString(POSITION, char*);
BOOLEAN kGameSpecificMenu = TRUE;
BOOLEAN kDebugMenu = FALSE;
CONST_STRING kHelpGraphicInterface = "";
CONST_STRING kHelpTextInterface = "";
CONST_STRING kHelpOnYourTurn = "";
CONST_STRING kHelpStandardObjective = "";
CONST_STRING kHelpReverseObjective = "";
CONST_STRING kHelpTieOccursWhen = /* Should follow 'A Tie occurs when... */ "";
CONST_STRING kHelpExample = "";
void *gGameSpecificTclInit = NULL;
void SetTclCGameSpecificOptions(int theOptions[]) { (void)theOptions; }

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

  char piece = (generic_hash_turn(position) == 1) ? 'w' : 'b';
  for (int idx = 0; idx < 16; idx++) {
    if (board[idx] != piece) {
      continue;
    }
    if (idx < 12 && board[idx + 4] == '-') {
      int new_move = (idx << 4) | ((idx + 4) & 0x0F);
      moves = CreateMovelistNode(new_move, moves);
    }
    if (idx > 3 && board[idx - 4] == '-') {
      int new_move = (idx << 4) | ((idx - 4) & 0x0F);
      moves = CreateMovelistNode(new_move, moves);
    }
    if (idx < 15 && idx % 4 != 3 && board[idx + 1] == '-') {
      int new_move = (idx << 4) | ((idx + 1) & 0x0F);
      moves = CreateMovelistNode(new_move, moves);
    }
    if (idx > 0 && idx % 4 != 0 && board[idx - 1] == '-') {
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
    board[(move >> 4) & 0x0F] = '-';
    curr_player = (curr_player + 1) % 3;
    POSITION child_position = generic_hash_hash(board, curr_player > 0 ? curr_player : 1);
    SafeFree(board);
    return child_position;
}

VALUE Primitive(POSITION position) {
    char* p_str = (char*)SafeMalloc(16 * sizeof(char));
    p_str = generic_hash_unhash(position, p_str);
    int player = generic_hash_turn(position);
    int deciding_idx = -1;
    for (int idx = 0; idx < 8; idx ++) {
        if (p_str[idx] == '-') {
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
    enum value_enum result = undecided; 
    if (deciding_idx != -1) {
      if ((p_str[deciding_idx] == 'w' && player == 1) || (p_str[deciding_idx] == 'b' && player == 2)) {
        result = gVariant == MISERE ? lose : win;
      } else {
        result = gVariant == MISERE ? win : lose;
      }
    }
    SafeFree(p_str);
    return result;
}

POSITION GetCanonicalPosition(POSITION position) {
    return position;
}

/*********** END SOLVING FUNCTIONS ***********/
/*********** BEGIN TEXTUI FUNCTIONS ***********/

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn) {
  char* board = (char*)SafeMalloc(16 * sizeof(char));
  generic_hash_unhash(position, board);
  char* prediction = GetPrediction(position, playerName, usersTurn);
  printf("%s's turn\n", playerName);
  for (int idx = 0; idx < 16; idx += 4) {
    printf("%c%c%c%c\n", board[idx], board[idx + 1], board[idx + 2], board[idx + 3]);
  }
  if (prediction) {
    printf("Prediction: %s\n\n", prediction);
  }
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

    if (sscanf(input, "%d->%d %c", &start_pos, &end_pos, &extra) != 2) {
        return FALSE;  
    }

    if (start_pos < 0 || start_pos > 15 || end_pos < 0 || end_pos > 15) {
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
    
    sscanf(input, "%d->%d", &start_pos, &end_pos);
    
    MOVE encoded_move = ((start_pos & 0x0F) << 4) | (end_pos & 0x0F);
    
    return encoded_move;
}

void MoveToString(MOVE move, char *moveStringBuffer) {
    int start_pos = (move >> 4) & 0x0F; 
    int end_pos = move & 0x0F;          
    
    snprintf(moveStringBuffer, MAX_MOVE_STRING_LENGTH, "%d->%d", start_pos, end_pos);

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
  switch (gVariant) {
    case STANDARD:
      return 0;
    case MISERE:
      return 1;
    default:
      return 0;
  }
}

void setOption(int option) {
  switch (option) {
    case 1:
      gVariant = MISERE;
      break;
    default:
      gVariant = STANDARD;
  }
}

void GameSpecificMenu(void) {
	while (TRUE) {
		printf("\n\nGame Options:\n\n"
		      "\tt)\t(T)oggle from ");
		if(getOption() == 0)
			printf("STANDARD to MISERE\n");
		else
			printf("MISERE to STANDARD\n");
		printf("\tb)\t(B)ack to the main menu\n"
		       "\nSelect an option:  ");
		switch (GetMyChar()) {
		case 't':
			setOption((getOption() + 1) % 2);
			if(getOption() == 0)
				printf("STANDARD\n");
			else
				printf("MISERE\n");
			break;
		case 'b':
			return;
		default:
			printf("Invalid option!\n");
		}
	}
}

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
