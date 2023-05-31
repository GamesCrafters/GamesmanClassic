/************************************************************************
**
** NAME:        m1dchess.c
**
** DESCRIPTION: 1D Chess
**
** AUTHOR:      Andrew Esteban
**
** DATE:        2023-03-20
**
************************************************************************/

#include <stdio.h>
#include "gamesman.h"

/* IMPORTANT GLOBAL VARIABLES */
STRING kAuthorName = "Andrew Esteban";
STRING kGameName = "1D Chess"; //  Use this spacing and case
STRING kDBName = "onedchess"; // Use this spacing and case
POSITION gNumberOfPositions; // Put your number of positions upper bound here.
POSITION gInitialPosition; // Put the hash value of the initial position.
BOOLEAN kPartizan = FALSE; // Is the game PARTIZAN i.e. given a board does each player have a different set of moves available to them?
BOOLEAN kTieIsPossible = TRUE; // Is a tie or draw possible?
BOOLEAN kLoopy = TRUE; // Is this game loopy?
BOOLEAN kSupportsSymmetries = FALSE; // Whether symmetries are supported (i.e. whether the GetCanonicalPosition is implemented)

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
STRING kHelpGraphicInterface = "";
STRING kHelpTextInterface = "";
STRING kHelpOnYourTurn = "";
STRING kHelpStandardObjective = "";
STRING kHelpReverseObjective = "";
STRING kHelpTieOccursWhen = /* Should follow 'A Tie occurs when... */ "";
STRING kHelpExample = "";

/* You don't have to change this. */
void DebugMenu() {}
/* Ignore this function. */
void SetTclCGameSpecificOptions(int theOptions[]) {}
/* Do not worry about this yet because you will only be supporting 1 variant for now. */
void GameSpecificMenu() {}




/*********** BEGIN SOLVING FUNCIONS ***********/

int boardSize = 8;

int vcfg(int pieces[]) {
  return pieces[0] == pieces[3];
}

/* Initialize any global variables or data structures needed before
solving or playing the game. */
void InitializeGame() {
  gCanonicalPosition = GetCanonicalPosition;
  gMoveToStringFunPtr = &MoveToString;

  // {char, min, max, char, min, max, ..., -1}
  int piecesArray[] = {'t', 1, 1, 'k', 0, 1, 'r', 0, 1, 'T', 1, 1, 'K', 0, 1, 'R', 0, 1, '-', 2, 6, -1};
  gNumberOfPositions = generic_hash_init(boardSize, piecesArray, &vcfg, 0);

  char initial[8] = {'t', 'k', 'r', '-', '-', 'R', 'K', 'T'};
  gInitialPosition = generic_hash_hash(initial, 1);
}

/* Return the hash value of the initial position. */
POSITION GetInitialPosition() {
  return gInitialPosition;
}

BOOLEAN CheckCheck(POSITION position, int player) {
  char board[boardSize];
  generic_hash_unhash(position, board);

  // Find the king
  int kingLoc = 0;

  for (int i = 0; i < boardSize; i++) {
    if ((player == 1 && board[i] == 't') || (player == 2 && board[i] == 'T')) {
      kingLoc = i;
      break;
    }
  }

  // Check opposing king
  if (kingLoc > 0 && (board[kingLoc - 1] == 'T' || board[kingLoc - 1] == 't')) {
    return TRUE;
  } else if (kingLoc < boardSize - 1 && (board[kingLoc + 1] == 'T' || board[kingLoc + 1] == 't')) {
    return TRUE;
  }

  // Check opposing knight
  if (kingLoc > 1 && ((board[kingLoc - 2] == 'K' && player == 1) || (board[kingLoc - 2] == 'k' && player == 2))) {
    return TRUE;
  } else if (kingLoc < boardSize - 2 && ((board[kingLoc + 2] == 'K' && player == 1) || (board[kingLoc + 2] == 'k' && player == 2))) {
    return TRUE;
  }

  // Rook
  for (int i = kingLoc + 1; i < boardSize; i++) {
    if (board[i] == '-') {
      continue;
    } else if ((player == 1 && board[i] == 'R') || (player == 2 && board[i] == 'r')) {
      return TRUE;
    } else {
      break;
    }
  }

  for (int i = kingLoc - 1; i >= 0; i--) {
    if (board[i] == '-') {
      continue;
    } else if ((player == 1 && board[i] == 'R') || (player == 2 && board[i] == 'r')) {
      return TRUE;
    } else {
      break;
    }
  }

  return FALSE;
}

MOVELIST* MoveChecker(POSITION position, MOVE newMove, int player, MOVELIST* moves) {
  int newPosition = DoMove(position, newMove);
  if ((player == 1 && CheckCheck(newPosition, 1) == FALSE) || (player == 2 && CheckCheck(newPosition, 2) == FALSE)) {
    moves = CreateMovelistNode(newMove, moves);
  }
  return moves;
}

/* Return a linked list of moves. */
MOVELIST *GenerateMoves(POSITION position) {
  MOVELIST *moves = NULL;

  char board[8];
  generic_hash_unhash(position, board);

  int player = generic_hash_turn(position);

  for (int i = 0; i < 8; i++) {
    if ((board[i] == 't' && player == 1) || (board[i] == 'T' && player == 2)) {
      if (i > 0) {
        if (board[i - 1] == '-') {
          moves = MoveChecker(position, i * 100 + (i - 1), player, moves);
        } else if (player == 1 && (board[i - 1] == 'K' || board[i - 1] == 'R')) {
          moves = MoveChecker(position, i * 100 + (i - 1), player, moves);
        } else if (player == 2 && (board[i - 1] == 'k' || board[i - 1] == 'r')) {
          moves = MoveChecker(position, i * 100 + (i - 1), player, moves);
        }
      }

      if (i < boardSize - 1) {
        if (board[i + 1] == '-') {
          moves = MoveChecker(position, i * 100 + (i + 1), player, moves);
        } else if (player == 1 && (board[i + 1] == 'K' || board[i + 1] == 'R')) {
          moves = MoveChecker(position, i * 100 + (i + 1), player, moves);
        } else if (player == 2 && (board[i + 1] == 'k' || board[i + 1] == 'r')) {
          moves = MoveChecker(position, i * 100 + (i + 1), player, moves);
        }
      }      

    } else if ((board[i] == 'k' && player == 1) || (board[i] == 'K' && player == 2)) {

      if (i > 1) {
        if (board[i - 2] == '-') {
          moves = MoveChecker(position, i * 100 + (i - 2), player, moves);
        } else if (player == 1 && (board[i - 2] == 'K' || board[i - 2] == 'R')) {
          moves = MoveChecker(position, i * 100 + (i - 2), player, moves);
        } else if (player == 2 && (board[i - 2] == 'k' || board[i - 2] == 'r')) {
          moves = MoveChecker(position, i * 100 + (i - 2), player, moves);
        }
      }

      if (i < boardSize - 2) {
        if (board[i + 2] == '-') {
          moves = MoveChecker(position, i * 100 + (i + 2), player, moves);
        } else if (player == 1 && (board[i + 2] == 'K' || board[i + 2] == 'R')) {
          moves = MoveChecker(position, i * 100 + (i + 2), player, moves);
        } else if (player == 2 && (board[i + 2] == 'k' || board[i + 2] == 'r')) {
          moves = MoveChecker(position, i * 100 + (i + 2), player, moves);
        }
      }    
      
    } else if ((board[i] == 'r' && player == 1) || (board[i] == 'R' && player == 2)) {
      int target = i - 1;

      while (target >= 0) {
        if (board[target] == '-') {
          moves = MoveChecker(position, i * 100 + target, player, moves);
        } else if (player == 1 && (board[target] == 'K' || board[target] == 'R')) {
          moves = MoveChecker(position, i * 100 + target, player, moves);
          break;
        } else if (player == 2 && (board[target] == 'k' || board[target] == 'r')) {
          moves = MoveChecker(position, i * 100 + target, player, moves);
          break;
        } else {
          break;
        }

        target--;
      }

      target = i + 1;

      while (target < boardSize) {
        if (board[target] == '-') {
          moves = MoveChecker(position, i * 100 + target, player, moves);
        } else if (player == 1 && (board[target] == 'K' || board[target] == 'R')) {
          moves = MoveChecker(position, i * 100 + target, player, moves);
          break;
        } else if (player == 2 && (board[target] == 'k' || board[target] == 'r')) {
          moves = MoveChecker(position, i * 100 + target, player, moves);
          break;
        } else {
          break;
        }

        target++;
      }
    }
  }
  
  return moves;
}

/* Return the position that results from making the 
input move on the input position. */
POSITION DoMove(POSITION position, MOVE move) {
  char board[8];
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
  Win by checkmating the enemy king. This occurs when the enemy king is in check 
  (under attack by one of your pieces) and there are no legal moves for the opponent 
  to get their king out of check.

  Careful! A draw can occur if:
  + A player is not in check and there are no legal moves for them to play (Stalemate)
  + The same board position is repeated 3 times in a game. (3 Fold Repetition)
  + There are only kings left on the board, thus it is impossible to checkmate the 
    opponent (Insufficient Material)
  */

  char board[8];
  generic_hash_unhash(position, board);

  int player = generic_hash_turn(position);

  // Check if just kings remain
  BOOLEAN others = FALSE;

  for (int i = 0; i < boardSize; i++) {
    if (board[i] == 'r' || board[i] == 'R' || board[i] == 'k' || board[i] == 'K') {
      others = TRUE;
    }
  }

  if (others == FALSE) {
    return tie;
  }

  // Check if checkmate (in check and no moves out of check)
  if (player == 1 && CheckCheck(position, player) == TRUE) {
    if (GenerateMoves(position) == NULL) {
      return lose;
    }

    MOVELIST* moves = GenerateMoves(position);

    BOOLEAN nonCheck = FALSE;

    while (moves != NULL) {
      MOVE move = moves->move;
      POSITION newPosition = DoMove(position, move);

      if (CheckCheck(newPosition, 1) == FALSE) {
        nonCheck = TRUE;
        break;
      }

      moves = moves->next;
    }

    if (nonCheck == FALSE) {
      return lose;
    }
  } else if (player == 2 && CheckCheck(position, player) == TRUE) {
    if (GenerateMoves(position) == NULL) {
      return lose;
    }

    MOVELIST* moves = GenerateMoves(position);

    BOOLEAN nonCheck = FALSE;

    while (moves != NULL) {
      MOVE move = moves->move;
      POSITION newPosition = DoMove(position, move);

      if (CheckCheck(newPosition, 2) == FALSE) {
        nonCheck = TRUE;
        break;
      }

      moves = moves->next;
    }

    if (nonCheck == FALSE) {
      return lose;
    }
  }

  if (GenerateMoves(position) == NULL) {
    return tie;
  }

  return undecided;
}

/* Symmetry Handling: Return the canonical position. */
POSITION GetCanonicalPosition(POSITION position) {
  // There are no symmetries as the kings are unable to pass each other
  return position;
}

/*********** END SOLVING FUNCTIONS ***********/







/*********** BEGIN TEXTUI FUNCTIONS ***********/

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn) {
  char board[8];
  generic_hash_unhash(position, board);

  printf("\n\t");

  for (int i = 0; i < 8; i++) {
    printf("%c", board[i]);
  }

  printf("\n\t01234567\n\n");
}

void PrintComputersMove(MOVE computersMove, STRING computersName) {
  printf("%s's move: [%s]\n", computersName, MoveToString(computersMove));
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
  int origin = ((int) input[0] - 48) * 10;
  origin += ((int) input[1] - 48);

  int target = ((int) input[3] - 48) * 10;
  target += ((int) input[4] - 48);

  if (origin < 0 || origin >= boardSize) {
    return FALSE;
  } else if (target < 0 || target >= boardSize) {
    return FALSE;
  }

  return TRUE;
}

/* Assume the text input signifies a valid move. Return
the move hash corresponding to the move. */
MOVE ConvertTextInputToMove(STRING input) {
  int origin = ((int) input[0] - 48) * 10;
  origin += ((int) input[1] - 48);

  int target = ((int) input[3] - 48) * 10;
  target += ((int) input[4] - 48);

  return origin * 100 + target;
}

/* Return the string representation of the move. 
Ideally this matches with what the user is supposed to
type when they specify moves. */
STRING MoveToString(MOVE move) {
  int origin = move / 100;
  int target = move % 100;

  STRING res = (STRING) SafeMalloc(6);

  sprintf(res, "%02d %02d", origin, target);

  return res;
}

/* Basically just print the move. */
void PrintMove(MOVE move) {
  printf("[%s]", MoveToString(move));
}

/*********** END TEXTUI FUNCTIONS ***********/









/*********** BEGIN VARIANT FUNCTIONS ***********/

/* How many variants are you supporting? */
int NumberOfOptions() {
  return 1;
}

/* Return the current variant id. */
int getOption() {
  return 0;
}

/* The input is a variant id. This function sets any global variables
or data structures according to the variant specified by the variant id. */
void setOption(int option) {
}

/*********** END VARIANT-RELATED FUNCTIONS ***********/

POSITION InteractStringToPosition(STRING board) {
  char realBoard[boardSize];

  int player = board[2] == 'A' ? 1 : 2;

  for (int i = 0; i < boardSize; i++) {
    realBoard[i] = board[i + 8];
  }

  return generic_hash_hash(realBoard, player);
}

STRING InteractPositionToString(POSITION position) {
  char board[boardSize];
  generic_hash_unhash(position, board);

  STRING result = (STRING) SafeMalloc(9 + boardSize);
  result[0] = 'R';
  result[1] = '_';
  result[2] = generic_hash_turn(position) == 1 ? 'A' : 'B';
  result[3] = '_';
  result[4] = '0';
  result[5] = '_';
  result[6] = '0';
  result[7] = '_';

  for (int i = 0; i < boardSize; i++) {
    result[i + 8] = board[i];
  }

  result[8 + boardSize] = '\0';

  return result;
}

STRING InteractMoveToString(POSITION position, MOVE move) {
  STRING result = (STRING) SafeMalloc(8);

  int origin = move / 100;
  int target = move % 100;

  sprintf(result, "M_%d_%d", origin, target);

  return result;
}