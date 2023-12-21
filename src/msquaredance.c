/************************************************************************
**
** NAME:        msquaredance.c
**
** DESCRIPTION: Square Dance
**
** AUTHOR:      Yu-TE (Jack) Hsu
**              Joey Corless
**              Cameron Cheung
**
************************************************************************/

#include "gamesman.h"

/* IMPORTANT GLOBAL VARIABLES */
CONST_STRING kAuthorName = "Yu-TE (Jack Hsu), Joey Corless, Cameron Cheung";
CONST_STRING kGameName = "Square Dance";
CONST_STRING kDBName = "squaredance";
POSITION gNumberOfPositions = 0;
POSITION gInitialPosition = 0;
BOOLEAN kPartizan = TRUE;
BOOLEAN kTieIsPossible = TRUE;
BOOLEAN kLoopy = FALSE;
BOOLEAN kSupportsSymmetries = TRUE;

POSITION GetCanonicalPosition(POSITION);
STRING MoveToString(MOVE);
POSITION kBadPosition = -1;
BOOLEAN kDebugDetermineValue = FALSE;
void* gGameSpecificTclInit = NULL;

BOOLEAN kGameSpecificMenu = TRUE;
BOOLEAN kDebugMenu = FALSE;

/* These variables are not needed for solving but if you have time 
after you're done solving the game you should initialize them 
with something helpful. */
CONST_STRING kHelpGraphicInterface = "";
CONST_STRING kHelpTextInterface = "You denote the column of your move with a letter (starting with a on the \n\
  left and increasing), and the row with a number (starting with 1 on the bottom \n\
  and increasing).  You then specify which way you want to place it (u for up, d \n\
  for down).  All three characters should be entered on one line with no spaces \n\
  in between.\n";
CONST_STRING kHelpOnYourTurn = "You can place your piece on the blank spot.\nFor example: a1u means put a piece at (a,1) with position up";
CONST_STRING kHelpStandardObjective = "Your goal is to form a square with four pieces sharing one of the attributes\n\
Attributes include:\n\
  O or X (player's color), U or D (up or down)\n\
Each player can only play one color (O or X) once the game has started\n\
He or she may choose to place the piece up or down\n\
In the following examples, assume Player X goes first and Player O placed just placed a piece:\n\
Example 1  \n\
   ----------  \n\
 3 |OU|  |XU|  \n\
   |--+--+--|  \n\
 2 |  |  |  |  \n\
   |--+--+--|  \n\
 1 |OU|  |XU|  \n\
   ----------  \n\
     a  b  c  \n\
 O placed the piece U at (0,0).  \n\
 O wins!\n\
 Example 2  \n\
   ----------  \n\
 3 |OD|XD|  |  \n\
   |--+--+--|  \n\
 2 |OD|XD|  |  \n\
   |--+--+--|  \n\
 1 |  |  |  |  \n\
   ----------  \n\
     a  b  c  \n\
 O placed the piece U at (0,0).  \n\
 O wins!\n\
Example 3  \n\
   ----------  \n\
 3 |  |OU|  |  \n\
   |--+--+--|  \n\
 2 |OU|  |XU|  \n\
   |--+--+--|  \n\
 1 |  |XU|  |  \n\
   ----------  \n\
     a  b  c  \n\
 O placed the piece U at (0,0).  \n\
O wins!\n";
CONST_STRING kHelpReverseObjective = "Force your opponent to make a square before you do.";
CONST_STRING kHelpTieOccursWhen = "the board is full and no squares are formed.";
CONST_STRING kHelpExample = "              \n\
   ----------  \n\
 3 |  |OU|  |  \n\
   |--+--+--|  \n\
 2 |  |  |  |  \n\
   |--+--+--|  \n\
 1 |  |  |  |  \n\
   ----------  \n\
     a  b  c  \n\
 O placed the piece U at (b3).  \n\
               \n\
   ----------  \n\
 3 |  |OU|  |  \n\
   |--+--+--|  \n\
 2 |  |XD|  |  \n\
   |--+--+--|  \n\
 1 |  |  |  |  \n\
   ----------  \n\
     a  b  c   \n\
 X placed the piece D at b2.  \n\
               \n\
   ----------  \n\
 3 |  |OU|  |  \n\
   |--+--+--|  \n\
 2 |  |XD|OD|  \n\
   |--+--+--|  \n\
 1 |  |  |  |  \n\
   ----------  \n\
     a  b  c   \n\
 O placed the piece D at c2.  \n\
               \n\
   ----------  \n\
 3 |  |OU|  |  \n\
   |--+--+--|  \n\
 2 |  |XD|OD|  \n\
   |--+--+--|  \n\
 1 |XD|  |  |  \n\
   ----------  \n\
     a  b  c  \n\
 X placed the piece D at a1.  \n\
               \n\
   ----------  \n\
 3 |  |OU|  |  \n\
   |--+--+--|  \n\
 2 |  |XD|OD|  \n\
   |--+--+--|  \n\
 1 |XD|OD|  |  \n\
   ----------  \n\
     a  b  c  \n\
 O placed the piece D at b1.  \n\
               \n\
   ----------  \n\
 3 |  |OU|  |  \n\
   |--+--+--|  \n\
 2 |  |XD|OD|  \n\
   |--+--+--|  \n\
 1 |XD|OD|XD|  \n\
   ----------  \n\
     a  b  c  \n\
 X placed the piece D at c1.  \n\
X wins!\n";

int cornerChecks[20][4] = {
  {0, 1, 4, 5},
  {1, 2, 5, 6},
  {2, 3, 6, 7},
  {4, 5, 8, 9},
  {5,6, 9,10},
  {6,7,10,11},
  {8,9,12,13},
  {9,10,13,14},
  {10,11,14,15},
  {0,2,8,10},
  {1,3,9,11},
  {4,6,12,14},
  {5,7,13,15},
  {0,3,12,15},
  {1,4,6,9},
  {2,5,7,10},
  {5,8,10,13},
  {6,9,11,14},
  {1,7,8,14},
  {2,4,11,13}
};

int primitiveMask = 0b11;
int boardSize = 16;

#define U 0b111
#define D 0b110
#define u 0b101
#define d 0b100
#define BLANK 0b000

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

void DebugMenu() {}
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
void combinationsInit() {
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
  int c = 0;
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

  int c = 0;
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

MOVE hashMove(BOOLEAN p1Turn, int to, int isUp) {
  return (p1Turn ? 32 : 0) | (to << 1) | isUp;
}

void unhashMove(MOVE move, BOOLEAN *p1Turn, int *to, int *isUp) {
  *p1Turn = (move >> 5) ? TRUE : FALSE;
  *to = (move >> 1) & 0xF;
  *isUp = move & 1;
}



/* Initialize any global variables or data structures needed before
solving or playing the game. */
void InitializeGame() {
  /* FOR THE PURPOSES OF INTERACT. FEEL FREE TO CHANGE IF SOLVING. */ 
	if (gIsInteract) {
		gLoadTierdbArray = FALSE; // SET TO TRUE IF SOLVING
	}
	/********************************/
  
  gCanonicalPosition = GetCanonicalPosition;
  gMoveToStringFunPtr = &MoveToString;

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

POSITION GetInitialPosition() {
  return 0;
}

/* Return a linked list of moves. */
MOVELIST *GenerateMoves(POSITION position) {
  MOVELIST *moves = NULL;

  if (Primitive(position) == undecided) {
    char board[boardSize];
    BOOLEAN p1Turn;
    unhash(position, board, &p1Turn);
    for (int i = 0; i < boardSize; i++) {
      if (board[i] == BLANK) {
        moves = CreateMovelistNode(hashMove(p1Turn, i, 0), moves);
        moves = CreateMovelistNode(hashMove(p1Turn, i, 1), moves);
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
  int to, isUp;
  unhashMove(move, &p1Turn, &to, &isUp);
  if (isUp) {
    if (p1Turn) {
      board[to] = U;
    } else {
      board[to] = u;
    }
  } else {
    if (p1Turn) {
      board[to] = D;
    } else {
      board[to] = d;
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
  char board[boardSize];
  unhash(position, board, &p1Turn);
  int *z;
  int i;
  for (i = 0; i < 20; i++) {
    z = cornerChecks[i];
    if (board[z[0]] & board[z[1]] & board[z[2]] & board[z[3]] & primitiveMask) {
      return gStandardGame ? lose : win;
    }
  }
  // invert board
  BOOLEAN filled = TRUE;
  for (i = 0; i < boardSize; i++) {
    if (board[i]) {
      board[i] ^= primitiveMask;
    } else {
      filled = FALSE;
    }
  }
  for (i = 0; i < 20; i++) {
    z = cornerChecks[i];
    if (board[z[0]] & board[z[1]] & board[z[2]] & board[z[3]] & primitiveMask) {
      return gStandardGame ? lose : win;
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
      board[i] ^= 0b1; // flip orientations
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
  if (tier != boardSize) {
    return CreateTierlistNode(tier + 1, NULL);
  } else {
    return NULL;
  }
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
  printf("4 | %c | %c | %c | %c |\n", board[12], board[13], board[14], board[15]);
  printf("  +---+---+---+---+\n");
  printf("3 | %c | %c | %c | %c |\n", board[8], board[9], board[10], board[11]);
  printf("  +---+---+---+---+      Turn: %c\n", p1Turn ? 'X' : 'O');
  printf("2 | %c | %c | %c | %c |\n", board[4], board[5], board[6], board[7]);
  printf("  +---+---+---+---+\n");
  printf("1 | %c | %c | %c | %c |\n", board[0], board[1], board[2], board[3]);
  printf("  +---+---+---+---+\n");
  printf("    a   b   c   d       %s\n", GetPrediction(position, playersName, usersTurn));
}

void PrintComputersMove(MOVE computersMove, STRING computersName) {
  printf("%8s's move [([a-d][1-4][u/d])] : ", computersName);
	PrintMove(computersMove);
	printf("\n");
}

USERINPUT GetAndPrintPlayersMove(POSITION position, MOVE *move, STRING playerName) {
  USERINPUT input;
	do {
		printf("%8s's move: (u)ndo", playerName);
		printf(": ");

		input = HandleDefaultTextInput(position, move, playerName);

		if (input != Continue)
			return input;
	} while (TRUE);

	/* NOT REACHED */
	return Continue;
}

/* Return whether the input text signifies a valid move. */
BOOLEAN ValidTextInput(STRING input) {
  if (strlen(input) != 3)
		return FALSE;
	if (input[0] < 'a' || input[0] > 'd' || input[1] < '1' || input[1] > '4')
		return FALSE;
  return (input[2] == 'u' || input[2] == 'd' || input[2] == 'D' || input[2] == 'U');
}

/* Assume the text input signifies a valid move. Return
the move hash corresponding to the move. */
MOVE ConvertTextInputToMove(STRING input) {
  int to = (input[1] - '1') * 4 + (input[0] - 'a');
  switch (input[2]) {
    case 'U':
      return hashMove(TRUE, to, 1);
    case 'D':
      return hashMove(TRUE, to, 0);
    case 'u':
      return hashMove(FALSE, to, 1);
    case 'd':
      return hashMove(FALSE, to, 0);
    default:
      return 0;
  }
}

/* Return the string representation of the move. 
Ideally this matches with what the user is supposed to
type when they specify moves. */
STRING MoveToString(MOVE move) {
  STRING moveString = SafeMalloc(4);
  BOOLEAN p1Turn;
  int to, isUp;
  unhashMove(move, &p1Turn, &to, &isUp);
  moveString[0] = 'a' + (to % 4);
  moveString[1] = '1' + (to / 4);
  if (isUp) {
    if (p1Turn) {
      moveString[2] = 'U';
    } else {
      moveString[2] = 'u';
    }
  } else {
    if (p1Turn) {
      moveString[2] = 'D';
    } else {
      moveString[2] = 'd';
    }
  }
  moveString[3] = '\0';

  return moveString;
}

void PrintMove(MOVE move) {
  STRING moveString = MoveToString(move);
  printf("%s", moveString);
  SafeFree(moveString);
}

/*********** END TEXTUI FUNCTIONS ***********/







/*********** BEGIN VARIANT FUNCTIONS ***********/

void GameSpecificMenu () {
	char selection = 'z';
	do {
		printf("\n\t----- Game Specific Options for Squaredance ----- \n\n");
		printf("\tc)\tToggle win by (C)olor.  [Currently: ");
		if(primitiveMask & 0b10) {
			printf("ON]\n");
    } else {
			printf("OFF]\n");
    }
		printf("\to)\tToggle win by (O)rientation.  [Currently: ");
		if(primitiveMask & 0b01) {
			printf("ON]\n");
    } else {
			printf("OFF]\n");
    }
    printf("\tm)\tToggle Misere Gameplay.  [Currently: ");
		if(!gStandardGame) {
			printf("ON]\n");
    } else {
			printf("OFF]\n");
    }
		printf("\tb)\t(B)ack to previous screen\n\n");
		printf("\tPlease select an option: ");
    selection = GetMyChar();
		switch (selection) {
      case 'c':
        primitiveMask ^= 0b10;
        selection = 'z';
        break;
      case 'o':
        primitiveMask ^= 0b01;
        selection = 'z';
        break;
      case 'm':
        gStandardGame = !gStandardGame;
        selection = 'z';
        break;
      case 'b':
        return;
      default:
        printf("Invalid Option.\n");
        selection = 'z';
        break;
		}
	} while (selection != 'b');
}

/* How many variants are you supporting? */
int NumberOfOptions() {
  /* 3 Binary Options: Misere, WinByUD, WinByColor */
  return 8;
}

/* Return the current variant id. */
int getOption() {
  return (gStandardGame ? 4 : 0) | primitiveMask;
}

/* The input is a variant id. This function sets any global variables
or data structures according to the variant specified by the variant id. */
void setOption(int option) {
  gStandardGame = (option >> 2) ? TRUE : FALSE;
  primitiveMask = option & 0b11;
}

/*********** END VARIANT-RELATED FUNCTIONS ***********/







POSITION InteractStringToPosition(STRING str) {
  str += 8;
  char board[boardSize];
  TIER tier = boardSize;
  for (int i = 0; i < boardSize; i++) {
    switch (str[i]) {
      case 'U': case 'W':
        board[i] = U;
        break;
      case 'D': case 'X':
        board[i] = D;
        break;
      case 'u': case 'Y':
        board[i] = u;
        break;
      case 'd': case 'Z':
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

BOOLEAN highlightSquare(char *board, int **slots) {
  int *z;
  int i;
  for (i = 0; i < 20; i++) {
    z = cornerChecks[i];
    if (board[z[0]] & board[z[1]] & board[z[2]] & board[z[3]] & primitiveMask) {
      (*slots) = z;
      return TRUE;
    }
  }
  // invert board
  int board2[boardSize];
  for (i = 0; i < boardSize; i++) {
    if (board[i]) {
      board2[i] = board[i] ^ primitiveMask;
    } else {
      board2[i] = BLANK;
    }
  }
  for (i = 0; i < 20; i++) {
    z = cornerChecks[i];
    if (board2[z[0]] & board2[z[1]] & board2[z[2]] & board2[z[3]] & primitiveMask) {
      (*slots) = z;
      return TRUE;
    }
  }
  return FALSE;
}

STRING InteractPositionToString(POSITION position) {
  char board[boardSize + 1];
  BOOLEAN p1Turn;
  unhash(position, board, &p1Turn);
  int *slots = NULL;
  BOOLEAN isPrimitive = highlightSquare(board, &slots);
  board[boardSize] = '\0';
  encodingToLetters(board);

  if (isPrimitive) {
    // This is for the sake of highlighting the relevant
    // squares if the position is primitive. WXYZ are highlighted
    // versions of UDud.
    for (int i = 0; i < 4; i++) {
      switch (board[slots[i]]) {
        case 'U':
          board[slots[i]] = 'W';
          break;
        case 'D':
          board[slots[i]] = 'X';
          break;
        case 'u':
          board[slots[i]] = 'Y';
          break;
        case 'd':
          board[slots[i]] = 'Z';
          break;
        default:
          break;
      }
    }
  }

  enum UWAPI_Turn turn = p1Turn ? UWAPI_TURN_A : UWAPI_TURN_B;
  return UWAPI_Board_Regular2D_MakeBoardString(turn, boardSize + 1, board);
}

STRING InteractMoveToString(POSITION position, MOVE move) {
  (void) position;
  BOOLEAN p1Turn;
  int to, isUp;
  unhashMove(move, &p1Turn, &to, &isUp);
  char moveToken;
  int offs = 16;
  if (isUp) {
    if (p1Turn) {
      moveToken = 'q';
    } else {
      moveToken = 'r';
    }
  } else {
    offs = 32;
    if (p1Turn) {
      moveToken = 's';
    } else {
      moveToken = 't';
    }
  }
  return UWAPI_Board_Regular2D_MakeAddStringWithSound(moveToken, offs + to, 'x');
}