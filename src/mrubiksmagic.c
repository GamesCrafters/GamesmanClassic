/************************************************************************
**
** NAME:        mrubiksmagic.c
**
** DESCRIPTION: Rubik's Magic
**
** AUTHOR:      Jeffrey Chiang
**              Byron Ross
**              Cameron Cheung
**
************************************************************************/

#include "gamesman.h"

/* IMPORTANT GLOBAL VARIABLES */
CONST_STRING kAuthorName = "Jeffrey Chiang, Byron Ross, Cameron Cheung";
CONST_STRING kGameName = "Rubik's Magic";
CONST_STRING kDBName = "rubiksmagic";
POSITION gNumberOfPositions = 0;
POSITION gInitialPosition = 0;
BOOLEAN kPartizan = TRUE;
BOOLEAN kTieIsPossible = TRUE;
BOOLEAN kLoopy = FALSE;
BOOLEAN kSupportsSymmetries = TRUE;

POSITION GetCanonicalPosition(POSITION);
MULTIPARTEDGELIST* GenerateMultipartMoveEdges(POSITION, MOVELIST*, POSITIONLIST*);
POSITION kBadPosition = -1;
BOOLEAN kDebugDetermineValue = FALSE;
void* gGameSpecificTclInit = NULL;

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
CONST_STRING kHelpTieOccursWhen = "";
CONST_STRING kHelpExample = "";

// Each triplet is three indices that form a three-in-a-row
// There are 24 total three-in-a-rows
int lineChecks[72] = {
  0,1,2, 1,2,3, 4,5,6, 5,6,7, 8,9,10, 9,10,11, 12,13,14, 13,14,15,
  0,4,8, 1,5,9, 2,6,10, 3,7,11, 4,8,12, 5,9,13, 6,10,14, 7,11,15,
  0,5,10, 1,6,11, 2,5,8, 3,6,9, 4,9,14, 5,10,15, 6,9,12, 7,10,13
};

// Which indices must have a piece in them to lock the corresponding
// three-in-a-row, listed in the same order as in lineChecks
// Last element indicates how many such indices there are
int lockChecks[24][8] = {
  {  3,  4,  5,  6, -1, -1, -1, 4}, // i.e., 3,4,5,6 need to be filled in order to lock the 0,1,2 three-in-a-row
  {  0,  5,  6,  7, -1, -1, -1, 4}, // i.e., 0,5,6,7 locks the 1,2,3 three-in-a-row
  {  0,  1,  2,  7,  8,  9, 10, 7}, // i.e., 0,1,2,7,8,9,10 locks the 4,5,6 three-in-a-row
  {  1,  2,  3,  4,  9, 10, 11, 7},
  {  4,  5,  6, 11, 12, 13, 14, 7},
  {  5,  6,  7,  8, 13, 14, 15, 7},
  {  8,  9, 10, 15, -1, -1, -1, 4},
  {  9, 10, 11, 12, -1, -1, -1, 4},
  {  1,  5,  9, 12, -1, -1, -1, 4},
  {  0,  2,  4,  6,  8, 10, 13, 7},
  {  1,  3,  5,  7,  9, 11, 14, 7},
  {  2,  6, 10, 15, -1, -1, -1, 4},
  {  0,  5,  9, 13, -1, -1, -1, 4},
  {  1,  4,  6,  8, 10, 12, 14, 7},
  {  2,  5,  7,  9, 11, 13, 15, 7},
  {  3,  6, 10, 14, -1, -1, -1, 4},
  {  1,  4,  6,  9, 11, 14, -1, 6},
  {  0,  2,  5,  7, 10, 15, -1, 6},
  {  1,  3,  4,  6,  9, 12, -1, 6},
  {  2,  5,  7,  8, 10, 13, -1, 6},
  {  0,  5,  8, 10, 13, 15, -1, 6},
  {  1,  4,  6,  9, 11, 14, -1, 6},
  {  2,  5,  7,  8, 10, 13, -1, 6},
  {  3,  6,  9, 11, 12, 14, -1, 6}
};

// orthogonalAdjacencies[i] is the list of indices orthogonally adj to i
// The last element of each list is number of orthogonal adjacencies i has
int orthogonalAdjacencies[16][5] = {
  {  1,  4, -1, -1, 2},
  {  0,  2,  5, -1, 3},
  {  1,  3,  6, -1, 3},
  {  2,  7, -1, -1, 2},
  {  0,  5,  8, -1, 3},
  {  1,  4,  6,  9, 4},
  {  2,  5,  7, 10, 4},
  {  3,  6, 11, -1, 3},
  {  4,  9, 12, -1, 3},
  {  5,  8, 10, 13, 4},
  {  6,  9, 11, 14, 4},
  {  7, 10, 15, -1, 3},
  {  8, 13, -1, -1, 2},
  {  9, 12, 14, -1, 3},
  { 10, 13, 15, -1, 3},
  { 11, 14, -1, -1, 2}
};

int boardSize = 16;
#define DOWN 0       // 0b0
#define UP 1         // 0b1
#define U 7          // 0b111
#define D 6          // 0b110
#define u 5          // 0b101
#define d 4          // 0b100
#define BLANK 0      // 0b000
#define NONE 16       // 0b10000

int gSymmetryMatrix[8][16] = {
	{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},
	{0,4,8,12,1,5,9,13,2,6,10,14,3,7,11,15},
	{15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0},
	{15,11,7,3,14,10,6,2,13,9,5,1,12,8,4,0},
	{3,2,1,0,7,6,5,4,11,10,9,8,15,14,13,12},
	{12,8,4,0,13,9,5,1,14,10,6,2,15,11,7,3},
	{12,13,14,15,8,9,10,11,4,5,6,7,0,1,2,3},
	{3,7,11,15,2,6,10,14,1,5,9,13,0,4,8,12}
};

void DebugMenu(void) {}
void SetTclCGameSpecificOptions(int theOptions[]) {
  (void)theOptions;
}

/* Tier Functions for TierGamesman Support */
TIERLIST *getTierChildren(TIER tier);
TIERPOSITION numberOfTierPositions(TIER tier);


/*********** BEGIN BASIC SOLVING FUNCIONS ***********/

POSITION fact(int n) {
     if (n <= 1) return 1;
	 POSITION prod = 1;
	 for (int i = 2; i <= n; i++)
		 prod *= i;
     return prod;
}

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
unsigned long long combinations[17][9][9];
void combinationsInit(void) {
    for (int boardsize = 0; boardsize <= 16; boardsize++)
        for (int numx = 0; numx <= 8; numx++)
            for (int numo = 0; numo <= 8; numo++)
                combinations[boardsize][numx][numo] = getNumPos(boardsize, numx, numo);
}

POSITION hash(char *board) {
  TIER tier = 0;
  for (int i = 0; i < boardSize; i++) {
    if (board[i] != BLANK) {
      tier++;
    }
  }
  
  TIERPOSITION tierPosition = 0;
  int numX = (tier + 1) / 2;
  int numO = tier / 2;
  int i;
  for (i = boardSize - 1; i >= 0; i--) {
    switch (board[i]) {
      case U: case D:
        numX--;
        break;
      case u: case d:
        if (numX > 0) tierPosition += combinations[i][numX-1][numO];
        numO--;
        break;
      default:
        if (numX > 0) tierPosition += combinations[i][numX-1][numO];
        if (numO > 0) tierPosition += combinations[i][numX][numO-1];
        break;
    }
  }

  tierPosition <<= tier;
  TIERPOSITION one = 1;
  unsigned int c = 0;
  for (i = 0; i < boardSize && c < tier; i++) {
    if (board[i] == U || board[i] == u) {
      tierPosition |= (one << c);
      c++;
    } else if (board[i] != BLANK) {
      c++;
    }
  }
  return gHashToWindowPosition(tierPosition, tier);
}

void unhash(POSITION position, char *board, BOOLEAN *p1Turn) {
  TIER tier; TIERPOSITION tierposition;
  gUnhashToTierPosition(position, &tierposition, &tier);
  TIERPOSITION tierpositioncopy = tierposition;
  tierposition >>= tier;
  *p1Turn = (tier & 1) == 0;

  POSITION o1, o2;
  int numX = (tier + 1) / 2;
  int numO = tier / 2;
  int i;
  for (i = boardSize - 1; i >= 0; i--) {
    o1 = (numX > 0) ? combinations[i][numX-1][numO] : 0;
    o2 = o1 + ((numO > 0) ? combinations[i][numX][numO-1] : 0);
    if (tierposition >= o2) {
      board[i] = BLANK;
      tierposition -= o2;
    } else if (tierposition >= o1) {
      if (numO > 0) {
          board[i] = d;
          numO--;
      } else
          board[i] = BLANK;
      tierposition -= o1;
    } else {
      if (numX > 0) {
          board[i] = D;
          numX--;
      } else if (numO > 0) {
          board[i] = d;
          numO--;
      } else
          board[i] = BLANK;
    }
  }

  unsigned int c = 0;
  TIERPOSITION one = 1;
  for (i = 0; i < boardSize && c < tier; i++) {
    if (board[i] == d) {
      if (tierpositioncopy & (one << c)) {
        board[i] = u;
      }
      c++;
    } else if (board[i] == D) {
      if (tierpositioncopy & (one << c)) {
        board[i] = U;
      }
      c++;
    }
  }
}

MOVE hashMove(int moveFrom, int moveTo, int placeAt, int isUp) {
  return (moveFrom << 11) | (moveTo << 6)| (placeAt << 1) | isUp;
}

void unhashMove(MOVE move, int *moveFrom, int *moveTo, int *placeAt, int *isUp) {
  *moveFrom = (move >> 11) & 0x1F;
  *moveTo = (move >> 6) & 0x1F;
  *placeAt = (move >> 1) & 0x1F;
  *isUp = move & 1;
}

/* Initialize any global variables or data structures needed before
solving or playing the game. */
void InitializeGame(void) {
  /* FOR THE PURPOSES OF INTERACT. FEEL FREE TO CHANGE IF SOLVING. */
  if (gIsInteract) {
	gLoadTierdbArray = FALSE; // SET TO TRUE IF SOLVING
  } 
  /********************************/
  
  gGenerateMultipartMoveEdgesFunPtr = &GenerateMultipartMoveEdges;
  gCanonicalPosition = GetCanonicalPosition;

  /* Tier and UndoMove Variables and Function Pointers */
  gTierChildrenFunPtr = &getTierChildren;
  gNumberOfTierPositionsFunPtr = &numberOfTierPositions;
  gInitialTierPosition = gInitialPosition;
  kSupportsTierGamesman = TRUE;
  kExclusivelyTierGamesman = TRUE;
  gInitialTier = 0;
  gSymmetries = TRUE;

  combinationsInit();  
}

/* Return a linked list of moves. */
MOVELIST *GenerateMoves(POSITION position) {
  MOVELIST *moves = NULL;

  if (Primitive(position) == undecided) {
    char board[boardSize];
    BOOLEAN p1Turn;
    unhash(position, board, &p1Turn);
    int moveFrom, i, moveTo, placeAt, *moveTos;
    int oppU, oppD;
    if (p1Turn) {
      oppU = u;
      oppD = d;
    } else {
      oppU = U;
      oppD = D;
    }
    for (moveFrom = 0; moveFrom < boardSize; moveFrom++) {
      if (board[moveFrom] == oppU || board[moveFrom] == oppD) {
        moveTos = orthogonalAdjacencies[moveFrom];
        for (i = 0; i < moveTos[4]; i++) {
          moveTo = moveTos[i];
          if (board[moveTo] == BLANK) {
            for (placeAt = 0; placeAt < boardSize; placeAt++) {
              if (placeAt != moveTo && (placeAt == moveFrom || board[placeAt] == BLANK)) {
                moves = CreateMovelistNode(hashMove(moveFrom, moveTo, placeAt, DOWN), moves);
                moves = CreateMovelistNode(hashMove(moveFrom, moveTo, placeAt, UP), moves);
              }
            }
          }
        }
      }
    }
    // If sliding oppoonent's piece not possible, just generate placement moves
    if (moves == NULL) {
      for (placeAt = 0; placeAt < boardSize; placeAt++) {
        if (board[placeAt] == BLANK) {
          moves = CreateMovelistNode(hashMove(NONE, NONE, placeAt, DOWN), moves);
          moves = CreateMovelistNode(hashMove(NONE, NONE, placeAt, UP), moves);
        }
      }
    }
  }
  return moves;
}

/* Return the position that results from making the 
input move on the input position. */
POSITION DoMove(POSITION position, MOVE move) {
  char board[boardSize];
  BOOLEAN p1Turn;
  unhash(position, board, &p1Turn);
  int moveFrom, moveTo, placeAt, isUp;
  unhashMove(move, &moveFrom, &moveTo, &placeAt, &isUp);

  if (moveFrom != NONE) {
    board[moveTo] = board[moveFrom] ^ 1;
    board[moveFrom] = BLANK;
  }

  if (isUp) {
    if (p1Turn) {
      board[placeAt] = U;
    } else {
      board[placeAt] = u;
    }
  } else {
    if (p1Turn) {
      board[placeAt] = D;
    } else {
      board[placeAt] = d;
    }
  }
  return hash(board);
}

/*****************************************************************
**  Determine whether the position is a primitive win,
**  primitive tie, primitive lose, or not primitive.
**  OUTPUTS: (VALUE) an enum; one of: (win, lose, tie, undecided)
**  See src/core/types.h for the value enum definition.
******************************************************************/
VALUE Primitive(POSITION position) {
  BOOLEAN p1Turn;
  char brd[boardSize];
  unhash(position, brd, &p1Turn);
  int i, j, k, bi, *lockCheck, andResult;

  // Create inverted bitboard
  BOOLEAN filled = TRUE;
  char invBrd[boardSize];
  for (i = 0; i < boardSize; i++) {
    if (brd[i]) {
      invBrd[i] = brd[i] ^ 2; // This flips the turn bit of piece
    } else {
      invBrd[i] = BLANK;
      filled = FALSE;
    }
  }

  char *board, *invBoard;
  if (p1Turn) {
    board = brd;
    invBoard = invBrd;
  } else {
    board = invBrd;
    invBoard = brd;
  }

  // Check whether current player has a (not-necessarily-locked) three-in-a-row
  for (i = 0; i < 72; i += 3) {
    bi = board[lineChecks[i]];
    if (
      bi & 2 &&
      bi == board[lineChecks[i + 1]] &&
      bi == board[lineChecks[i + 2]]
    ) {
      return win;
    }
  }

  // Check whether opponent has locked three-in-a-row
  for (i = 0, j = 0; i < 72; i += 3, j++) {
    bi = invBoard[lineChecks[i]];
    if (
      bi & 2 &&
      bi == invBoard[lineChecks[i + 1]] &&
      bi == invBoard[lineChecks[i + 2]]
    ) {
      andResult = 4;
      lockCheck = lockChecks[j];
      for (k = 0; k < lockCheck[7]; k++) {
        andResult &= invBoard[lockCheck[k]];
      }
      if (andResult) { // if any index of the lockCheck is 0, andResult == 0 and this triplet is not locked
        return lose;
      }
    }
  }

  return filled ? tie : undecided;
}

/* Symmetry Handling: Return the canonical position. */
POSITION GetCanonicalPosition(POSITION position) {
  BOOLEAN p1Turn;
  char board[boardSize];
  char board2[boardSize];
  unhash(position, board, &p1Turn);
  POSITION symmetricPosition = 0;
  int *t;
  int i, j;
  for (i = 1; i < 8; i++) {
    t = gSymmetryMatrix[i];
    for (j = 0; j < boardSize; j++) {
      board2[j] = board[t[j]];
    }
    symmetricPosition = hash(board2);
    if (symmetricPosition < position) {
      position = symmetricPosition;
    }
  }
  
  for (i = 0; i < boardSize; i++) {
    if (board[i]) {
      board[i] ^= 1; // flip orientations
    }
  }

  for (i = 0; i < 8; i++) {
    t = gSymmetryMatrix[i];
    for (j = 0; j < boardSize; j++) {
      board2[j] = board[t[j]];
    }
    symmetricPosition = hash(board2);
    if (symmetricPosition < position) {
      position = symmetricPosition;
    }
  }
  return position;
}

/*********** END BASIC SOLVING FUNCTIONS ***********/





/*********** BEGIN TIER/UNDOMOVE FUNCTIONS ***********/

TIERLIST *getTierChildren(TIER tier) {
  return tier < ((unsigned int) boardSize) ? CreateTierlistNode(tier + 1, NULL) : NULL;
}

TIERPOSITION numberOfTierPositions(TIER tier) {
  return (combinations[16][(tier + 1) / 2][tier / 2]) << tier;
}

/*********** END TIER/UNDOMOVE FUNCTIONS ***********/


/*********** BEGIN TEXTUI FUNCTIONS ***********/

void encodingToLetters(char *board) {
  for (int i = 0; i < boardSize; i++) {
    switch (board[i]) {
      case U:
        board[i] = 'U';
        break;
      case D:
        board[i] = 'D';
        break;
      case u:
        board[i] = 'u';
        break;
      case d:
        board[i] = 'd';
        break;
      default:
        board[i] = '-';
        break;
    }
  }
}

void PrintPosition(POSITION position, STRING playersName, BOOLEAN usersTurn) {
  char board[boardSize];
  BOOLEAN p1Turn;
  unhash(position, board, &p1Turn);
  encodingToLetters(board);

  printf("\n  +---+---+---+---+\n");
  printf("1 | %c | %c | %c | %c |\n", board[0], board[1], board[2], board[3]);
  printf("  +---+---+---+---+\n");
  printf("2 | %c | %c | %c | %c |\n", board[4], board[5], board[6], board[7]);
  printf("  +---+---+---+---+      Turn: %c\n", p1Turn ? 'X' : 'O');
  printf("3 | %c | %c | %c | %c |\n", board[8], board[9], board[10], board[11]);
  printf("  +---+---+---+---+\n");
  printf("4 | %c | %c | %c | %c |\n", board[12], board[13], board[14], board[15]);
  printf("  +---+---+---+---+\n");
  printf("    a   b   c   d       %s\n", GetPrediction(position, playersName, usersTurn));
}

USERINPUT GetAndPrintPlayersMove(POSITION position, MOVE *move, STRING playerName) {
  USERINPUT input;
	do {
		printf("%8s's move [(u)ndo/[a-d][1-4][N/E/W/S][a-d][1-4][U/D]/[a-d][1-4][U/D]] : ", playerName);

		input = HandleDefaultTextInput(position, move, playerName);

		if (input != Continue)
			return input;
	} while (TRUE);

	/* NOT REACHED */
	return Continue;
}

/* Return whether the input text signifies a valid move. */
BOOLEAN ValidTextInput(STRING input) {
  if (strlen(input) == 3) {
    if (input[0] < 'a' || input[0] > 'd' || input[1] < '1' || input[1] > '4') {
      return FALSE;
    }
    return input[2] == 'U' || input[2] == 'D' || input[2] == 'u' || input[2] == 'd';
  } else if (strlen(input) == 6) {
    if (input[0] < 'a' || input[0] > 'd' || input[1] < '1' || input[1] > '4') {
      return FALSE;
    }
    if (input[2] == 'N' || input[2] == 'E' || input[2] == 'W' || input[2] == 'S' || input[2] == 'n' || input[2] == 'e' || input[2] == 'w' || input[2] == 's') {
      if (input[3] < 'a' || input[3] > 'd' || input[4] < '1' || input[4] > '4') {
        return FALSE;
      }
      return input[5] == 'U' || input[5] == 'D' || input[5] == 'u' || input[5] == 'd';
    } else {
      return FALSE;
    }
  } else {
    return FALSE;
  }
}

/* Assume the text input signifies a valid move. Return
the move hash corresponding to the move. */
MOVE ConvertTextInputToMove(STRING input) {
  if (input[3] == '\0') { // just placement
    int placeAt = (input[1] - '1') * 4 + (input[0] - 'a');
    int isUp = (input[2] == 'U' || input[2] == 'u') ? 1 : 0;
    return hashMove(NONE, NONE, placeAt, isUp);
  } else { // slide and place
    int moveFrom = (input[1] - '1') * 4 + (input[0] - 'a');
    int moveTo;
    if (input[2] == 'W' || input[2] == 'w') {
      moveTo = moveFrom - 1;
    } else if (input[2] == 'E' || input[2] == 'e') {
      moveTo = moveFrom + 1;
    } else if (input[2] == 'N' || input[2] == 'n') {
      moveTo = moveFrom - 4;
    } else {
      moveTo = moveFrom + 4;
    }
    int placeAt = (input[4] - '1') * 4 + (input[3] - 'a');
    int isUp = (input[5] == 'U' || input[5] == 'u') ? 1 : 0;
    return hashMove(moveFrom, moveTo, placeAt, isUp);
  }
}

/* Return the string representation of the move. 
Ideally this matches with what the user is supposed to
type when they specify moves. */
void MoveToString(MOVE move, char *moveString) {
  int moveFrom, moveTo, placeAt, isUp;
  BOOLEAN writeFullMoveString = TRUE;

  if (move & (1 << 18)) {
    // PartMove: selecting which piece to slide and where to slide it. Encoded as 1 << 18 | original mv
		moveFrom = (move >> 11) & 0x1F;
    moveTo = (move >> 6) & 0x1F;
    writeFullMoveString = FALSE;
  } else if (move & (1 << 19)) {
    unhashMove(move, &moveFrom, &moveTo, &placeAt, &isUp);
    moveString[0] = 'a' + (placeAt % 4);
    moveString[1] = '1' + (placeAt / 4);
    moveString[2] = isUp ? 'U' : 'D';
    moveString[3] = '\0';
    return;
  } else {
    // FullMove
    unhashMove(move, &moveFrom, &moveTo, &placeAt, &isUp);
  }

  int i = 0;
  if (moveFrom != NONE) {
    moveString[0] = 'a' + (moveFrom % 4);
    moveString[1] = '1' + (moveFrom / 4);
    if (moveFrom - 1 == moveTo) {
      moveString[2] = 'W';
    } else if (moveFrom + 1 == moveTo) {
      moveString[2] = 'E';
    } else if (moveFrom - 4 == moveTo) {
      moveString[2] = 'N';
    } else {
      moveString[2] = 'S';
    }
    i = 3;
  }

  if (writeFullMoveString) {
    moveString[i] = 'a' + (placeAt % 4);
    moveString[i + 1] = '1' + (placeAt / 4);
    moveString[i + 2] = isUp ? 'U' : 'D';
    moveString[i + 3] = '\0';
  } else {
    moveString[i] = '\0';
  }
}

void PrintMove(MOVE move) {
  char moveString[10];
  MoveToString(move, moveString);
  printf("%s", moveString);
}

void PrintComputersMove(MOVE computersMove, STRING computersName) {
  printf("%8s's move [[a-d][1-4][N/E/W/S][a-d][1-4][U/D]/[a-d][1-4][U/D]] : ", computersName);
	PrintMove(computersMove);
	printf("\n");
}

/*********** END TEXTUI FUNCTIONS ***********/







/*********** BEGIN VARIANT FUNCTIONS ***********/

void GameSpecificMenu (void) {}
int NumberOfOptions(void) { return 1; }
int getOption(void) { return 0; }
void setOption(int option) {
  (void) option;
}

/*********** END VARIANT-RELATED FUNCTIONS ***********/

POSITION encodeIntermediatePosition(POSITION realParentPosition, POSITION moveFrom, POSITION moveTo) {
  return (1LL << 63) | (moveFrom << 58) | (moveTo << 53) | realParentPosition;
}

POSITION StringToPosition(char *positionString) {
	int turn;
	char *str;
	if (ParseStandardOnelinePositionString(positionString, &turn, &str)) {
    char board[boardSize];
    TIER tier = boardSize;
    for (int i = 0; i < boardSize; i++) {
      switch (str[i]) {
        case 'U':
          board[i] = U;
          break;
        case 'D':
          board[i] = D;
          break;
        case 'u':
          board[i] = u;
          break;
        case 'd':
          board[i] = d;
          break;
        default:
          board[i] = BLANK;
          tier--;
          break;
      }
    }
    gInitializeHashWindow(tier, FALSE);
    return hash(board);
	}
	return NULL_POSITION;
}

void PositionToAutoGUIString(POSITION position, char *autoguiPositionStringBuffer) {
	int moveFrom = -1, moveTo = -1;
  if (position & (1LL << 63)) { // is intermediate
    moveFrom = (position >> 58) & 0x1F;
    moveTo = (position >> 53) & 0x1F;
    position &= 0xFFFFFFFFFFFF;
  }

  char board[boardSize + 1];
  BOOLEAN p1Turn;
  unhash(position, board, &p1Turn);

  if (moveFrom != -1) {
    board[moveTo] = board[moveFrom] ^ 1;
    board[moveFrom] = BLANK;
  }
  board[boardSize] = '\0';
  encodingToLetters(board);
  AutoGUIMakePositionString(p1Turn ? 1 : 2, board, autoguiPositionStringBuffer);
}

void MoveToAutoGUIString(POSITION position, MOVE move, char *autoguiMoveStringBuffer) {
  (void) position;
  int moveFrom = (move >> 11) & 0x1F;
	int moveTo = (move >> 6) & 0x1F;

  if (move & (1 << 18)) { // PartMove: selecting which piece to slide and where to slide it. Encoded as 1 << 18 | original mv
		AutoGUIMakeMoveButtonStringM(moveFrom, moveTo, 'y', autoguiMoveStringBuffer);
	} else if (moveFrom == NONE || move & (1 << 19)) { // Placing piece partmove or placing piece fullmove
    position &= 0xFFFFFFFFFFFF;
    char board[boardSize];
    BOOLEAN p1Turn;
    unhash(position, board, &p1Turn);

    int placeAt = (move >> 1) & 0x1F;
		char c; int at;
		if (move & 1) { // move & 1 == isUp; place with piece facing up
			c = p1Turn ? 'r' : 'p';
			at = placeAt;
		} else { // place with piece facing down
			c = p1Turn ? 's' : 'q';
			at = placeAt + 16;
		}
		AutoGUIMakeMoveButtonStringA(c, 16 + at, 'x', autoguiMoveStringBuffer);
	} else {
    AutoGUIWriteEmptyString(autoguiMoveStringBuffer);
  }
}

// CreateMultipartEdgeListNode(POSITION from, POSITION to, MOVE partMove, MOVE fullMove, BOOLEAN isTerminal, MULTIPARTEDGELIST *next)
MULTIPARTEDGELIST* GenerateMultipartMoveEdges(POSITION position, MOVELIST *moveList, POSITIONLIST *positionList) {
	MULTIPARTEDGELIST *mpel = NULL;
	int prevMoveFrom = -1, prevMoveTo = -1;
  int moveFrom, moveTo, placeAt, isUp;
  POSITION interPos;
	
	while (moveList != NULL) {
    unhashMove(moveList->move, &moveFrom, &moveTo, &placeAt, &isUp);

    if (moveFrom != NONE) {
      interPos = encodeIntermediatePosition(position, moveFrom, moveTo);
      if (prevMoveFrom != moveFrom || prevMoveTo != moveTo) {
        // sliding partmove
        mpel = CreateMultipartEdgeListNode(NULL_POSITION, interPos, (1 << 18) | moveList->move, 0, mpel);
        prevMoveFrom = moveFrom;
        prevMoveTo = moveTo;
      }

      // placing partmove
      mpel = CreateMultipartEdgeListNode(interPos, NULL_POSITION, (1 << 19) | moveList->move, moveList->move, mpel);
    }

		moveList = moveList->next;
		positionList = positionList->next;
	}
	return mpel;
}
