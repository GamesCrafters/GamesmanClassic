/************************************************************************
**
** NAME:        madugo.c
**
** DESCRIPTION: Adugo
**
** AUTHOR:      Haonan Huang
**
** DATE:        2023-04-24
**
************************************************************************/

#include "gamesman.h"

/* IMPORTANT GLOBAL VARIABLES */
CONST_STRING kAuthorName = "Haonan Huang";
CONST_STRING kGameName = "Adugo"; //  use this spacing and case
CONST_STRING kDBName = "adugo"; // use this spacing and case
POSITION gNumberOfPositions; // Put your number of positions upper bound here.
POSITION gInitialPosition; // Put the hash value of the initial position.
BOOLEAN kPartizan = TRUE; // TODO: Is the game PARTIZAN i.e. given a board does each player have a different set of moves available to them?
BOOLEAN kTieIsPossible = TRUE; // TODO: Is a tie or draw possible?
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
void SetTclCGameSpecificOptions(int theOptions[]) { (void) theOptions; }
/* Do not worry about this yet because you will only be supporting 1 variant for now. */
void GameSpecificMenu() {}

int gSymmetryMatrix3x3[8][25] = {
  {0,1,2,3,4,5,6,7,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {6,3,0,7,4,1,8,5,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {8,7,6,5,4,3,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {2,5,8,1,4,7,0,3,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {2,1,0,5,4,3,8,7,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,3,6,1,4,7,2,5,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {6,7,8,3,4,5,0,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {8,5,2,7,4,1,6,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

int gSymmetryMatrix5x5[8][25] = {
	{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24},
	{4,3,2,1,0,9,8,7,6,5,14,13,12,11,10,19,18,17,16,15,24,23,22,21,20},
	{20,15,10,5,0,21,16,11,6,1,22,17,12,7,2,23,18,13,8,3,24,19,14,9,4},
	{0,5,10,15,20,1,6,11,16,21,2,7,12,17,22,3,8,13,18,23,4,9,14,19,24},
	{24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0},
	{20,21,22,23,24,15,16,17,18,19,10,11,12,13,14,5,6,7,8,9,0,1,2,3,4},
	{4,9,14,19,24,3,8,13,18,23,2,7,12,17,22,1,6,11,16,21,0,5,10,15,20},
	{24,19,14,9,4,23,18,13,8,3,22,17,12,7,2,21,16,11,6,1,20,15,10,5,0}
};

#define BLANK '.'
#define X 'X' // dog
#define O 'O' // jaguar
#define PLAYER_ONE 1
#define PLAYER_TWO 2

int BOARDDIMENSIONX = 3; //default
int BOARDDIMENSIONY = 3; //default
int BOARDSIZE = 9; //default
int MAXSYMMETRY = 8; //default
int PIECENUM = 5;
int INITIALOPOSITION = 4;
int LOSINGPIECES = 2;
int (*symmetriesToUse)[25] = gSymmetryMatrix3x3; //default
const int DX[8] = {-1, 0, 1, 0, -1, 1, 1, -1};
const int DY[8] = {0, 1, 0, -1, 1, 1, -1, -1};
int moveOffset = 9;
int boardOffset = 9;
char initialInteractString[] = "R_A_0_0_----------------------------";


// HASH/UNHASH
TIER BoardToTier(char*);
POSITION BoardToPosition(char*, int);
void PositionToBoard(POSITION, char*, int*);

//TIER GAMESMAN
void SetupTierStuff();
TIERLIST* TierChildren(TIER);
TIERPOSITION NumberOfTierPositions(TIER);
STRING TierToString(TIER);

// SOLVING FUNCTION
BOOLEAN InBoard(int, int);
MOVE MoveEncode(int, int, int, int);
void MoveDecode(MOVE, int*, int*, int*, int*);
int boardStatus(POSITION);
void InitializeGame();
POSITION GetInitialPosition();
MOVELIST *GenerateMoves(POSITION);
POSITION DoMove(POSITION, MOVE);
VALUE Primitive(POSITION);
POSITION GetCanonicalPosition(POSITION);


// TEXTUI FUNCTION
void PrintBoard(char*);
void PrintPosition(POSITION, STRING, BOOLEAN);
void PrintComputersMove(MOVE, STRING);
USERINPUT GetAndPrintPlayersMove(POSITION, MOVE *, STRING);
BOOLEAN ValidTextInput(STRING);
MOVE ConvertTextInputToMove(STRING);
STRING MoveToString(MOVE);
void PrintMove(MOVE);

// Interact functions
POSITION InteractStringToPosition(STRING);
STRING InteractPositionToString(POSITION);
STRING InteractPositionToEndData(POSITION);
STRING InteractMoveToString(POSITION, MOVE);

/*********** BEGIN TIER FUNCIONS ***********/

TIER SetTier(int numx, int intermediate){
  return numx*2+intermediate;
}

void GetFromTier(TIER tier, int* numx, int* intermediate){
  *numx = tier / 2;
  *intermediate = tier % 2;
}

POSITION fact(int n) {
  if (n <= 1) return 1;
  POSITION prod = 1;
  for (int i = 2; i <= n; i++)
    prod *= i;
  return prod;
}

// calculates the number of combinations
unsigned long long getNumPos(int boardsize, int numx, int numo) {
  long long temp = 1;
  int numb = boardsize - numx - numo;
  // Do fact(boardsize) / fact(numx) / fact(numo) / fact(numb) without overflowing
  // fact(boardsize) is too big to store in a unsigned long long
  // so this for loop represents temp = fact(boardsize) / fact(numb)
	if (numb >= numx && numb >= numo) {
		for (int i = numb + 1; i <= boardsize; i++)
      temp *= i;
    return temp / fact(numx) / fact(numo);
	} else if (numx >= numb && numx >= numo) {
		for (int i = numx + 1; i <= boardsize; i++)
      temp *= i;
    return temp / fact(numb) / fact(numo);
	} else {
		for (int i = numo + 1; i <= boardsize; i++)
      temp *= i;
    return temp / fact(numb) / fact(numx);
	}
}

unsigned long long combinations[26][15][2];
void combinationsInit() {
  for (int boardsize=0; boardsize<=BOARDSIZE; boardsize++)
    for (int numx=0; numx<=PIECENUM; numx++)
      for (int numo=0; numo<=1; numo++)
        combinations[boardsize][numx][numo] = getNumPos(boardsize, numx, numo);
}
typedef struct {
	int tier;
	POSITION position;
	char board[25];
} HASH_RECORD;

#define HASH_RECORDS 100000

HASH_RECORD hashRecords[HASH_RECORDS];

int hashCacheInited = 0;

void hashCacheInit() {
	for (long i=0; i<HASH_RECORDS; i++) {
		hashRecords[i].position = -1LL;
	}
	hashCacheInited = 1;
}

void hashCachePut(int tier, POSITION position, char *board) {
	if (!hashCacheInited) hashCacheInit();

	long i = position % HASH_RECORDS;
	if (hashRecords[i].tier != tier ||
		hashRecords[i].position != position) {
		hashRecords[i].tier = tier;
		hashRecords[i].position = position;
		memcpy(hashRecords[i].board, board, BOARDSIZE);
	}
}

// Returns TRUE if cache miss, otherwise FALSE
BOOLEAN hashCacheGet(int tier, POSITION position, char *board) {
	if (!hashCacheInited) hashCacheInit();

	long i = position % HASH_RECORDS;
	if (hashRecords[i].tier == tier &&
		hashRecords[i].position == position) {
		memcpy(board, hashRecords[i].board, BOARDSIZE);
		return FALSE;
	}
	return TRUE;
}

unsigned long long hashIt(int boardsize, int numx, char *board) {
  int numo = 1;
  unsigned long long sum=0;
  for (int i=boardsize-1; i>0; i--) { // no need to calculate i == 0
    switch (board[i]) {
      case X:
        numx--;
        break;
      case O:
        if (numx > 0) sum += combinations[i][numx-1][numo];
        numo--;
        break;
      case BLANK:
        if (numx > 0) sum += combinations[i][numx-1][numo];
        if (numo > 0) sum += combinations[i][numx][numo-1];
        break;
    }
  }
  return sum;
}

void unhashIt(int boardsize, int numx, POSITION pos, char *board) {
  int numo = 1;
  POSITION o1, o2;
  for (int i=boardsize-1; i>=0; i--) {
    o1 = (numx > 0) ? combinations[i][numx-1][numo] : 0;
    o2 = o1 + ((numo > 0) ? combinations[i][numx][numo-1] : 0);
    if (pos >= o2) {
      board[i] = BLANK;
      pos -= o2;
    }
    else if (pos >= o1) {
      if (numo > 0) {
        board[i] = O;
        numo--;
      }
      else board[i] = BLANK;
      pos -= o1;
    }
    else {
      if (numx > 0) {
        board[i] = X;
        numx--;
      }
      else if (numo > 0) {
        board[i] = O;
        numo--;
      }
      else board[i] = BLANK;
    }
  }
}

POSITION hash(char* board, char turn, int intermediate) {
	POSITION pos = 0;
	if (gHashWindowInitialized) {
    int numx = 0; int numo = 1;
    for (int i = 0; i < BOARDSIZE; i++){
      if (board[i] == X){
        numx++;
      }
    }
		TIER tier = SetTier(numx, intermediate);
		generic_hash_context_switch(tier);
		TIERPOSITION tierposition = hashIt(BOARDSIZE, numx, board);

		if (turn == X) {
			tierposition += combinations[BOARDSIZE][numx][numo];
		}
		pos = gHashToWindowPosition(tierposition, tier);
	}
	return pos;
}

char* unhash(POSITION pos, char* turn, int* numx, int* intermediate) {
	//piecesLeft = total pieces left during stage 1 (x + o)
	char* board = (char*)SafeMalloc(BOARDSIZE * sizeof(char));
	if (gHashWindowInitialized) {
		TIER tier; TIERPOSITION tierposition;
		gUnhashToTierPosition(pos, &tierposition, &tier);
		generic_hash_context_switch(tier);
		(*turn) = (generic_hash_turn(tierposition) == PLAYER_ONE ? O : X);

    GetFromTier(tier, numx, intermediate);
    int numo = 1;

		BOOLEAN cache_miss = hashCacheGet(tier, tierposition, board);
		if (cache_miss) {
			unhashIt(BOARDSIZE, *numx, tierposition % combinations[BOARDSIZE][*numx][numo], board);
			hashCachePut(tier, tierposition, board);
		}
	}
	return board;
}

void SetupTierStuff() {
	generic_hash_destroy();
	char* board = (char*) SafeMalloc(BOARDSIZE * sizeof(char));
	kSupportsTierGamesman = TRUE;
	gTierChildrenFunPtr = &TierChildren;
	gNumberOfTierPositionsFunPtr = &NumberOfTierPositions;
	gTierToStringFunPtr = &TierToString;
	int tier, numx, intermediate, i;
	generic_hash_custom_context_mode(TRUE);
	int pieces_array[] = {X, 0, 0, O, 1, 1, BLANK, 0, 0, -1};
	kExclusivelyTierGamesman = TRUE;

  for (numx=0; numx<=PIECENUM; numx++) {
    for (intermediate = 0; intermediate <= 1; intermediate++){
      tier = SetTier(numx, intermediate);
      pieces_array[1]=pieces_array[2]=numx;
      pieces_array[7]=pieces_array[8]=BOARDSIZE-numx-1;

      generic_hash_init(BOARDSIZE, pieces_array, NULL, 0);
      generic_hash_set_context(tier);
    }
  }
  
  tier = SetTier(PIECENUM, 0);
	gInitialTier = tier;    //initial pieces on board
	gCurrentTier = tier;
	generic_hash_context_switch(tier);

	for(i = 0; i < PIECENUM+1; i++) {
		board[i] = X;
	}
  for (i = PIECENUM+1; i < BOARDSIZE; i++){
    board[i] = BLANK;
  }
  board[INITIALOPOSITION] = O;
	gInitialTierPosition = generic_hash_hash(board, PLAYER_ONE);
}

TIERLIST* TierChildren(TIER tier) {
  int numx, intermediate;
  GetFromTier(tier, &numx, &intermediate);

	TIERLIST* list = NULL;
  list = CreateTierlistNode(tier, list);

  if (gIsInteract){
    if (intermediate == 1){
      list = CreateTierlistNode(tier-1, list);
      if (numx > 0){
        list = CreateTierlistNode(tier-2, list);
        list = CreateTierlistNode(tier-3, list);
      }
    } else {
      if (numx > 0){
        list = CreateTierlistNode(tier-1, list);
        list = CreateTierlistNode(tier-2, list);
      }
    }
  } else {
    if (intermediate == 1){
      for (int i = 0; i <= PIECENUM; i++)
        if (tier>= (POSITION) 2*i+1)
          list = CreateTierlistNode(tier-(2*i+1), list);
    } else {
      if (numx > 0){
        // just for solving, the game will never go to that tier
        list = CreateTierlistNode(tier-1, list);
      }
      for (int i = 1; i <= PIECENUM; i++){
        if (tier>= (POSITION)2*i)
          list = CreateTierlistNode(tier-2*i, list);
      }
    }
  }
  
	return list;
}

TIERPOSITION NumberOfTierPositions(TIER tier) {
	generic_hash_context_switch(tier);
	return generic_hash_max_pos();
}

STRING TierToString(TIER tier) {
  int numx, intermediate;
  GetFromTier(tier, &numx, &intermediate);
	STRING str = (STRING) SafeMalloc (sizeof (char) * 40);
	sprintf(str, "numx: %d intermediate: %d", numx, intermediate);
	return str;
}

/*********** END TIER FUNCIONS ***********/



/*********** BEGIN SOLVING FUNCIONS ***********/

/* Initialize any global variables or data structures needed before
solving or playing the game. */
void InitializeGame() {
  /* FOR THE PURPOSES OF INTERACT. FEEL FREE TO CHANGE IF SOLVING. */ 
	if (gIsInteract) {
		gLoadTierdbArray = FALSE; // SET TO TRUE IF SOLVING
	}
	/********************************/
  BOARDSIZE = BOARDDIMENSIONX * BOARDDIMENSIONY;

  /* FOR THE PURPOSES OF INTERACT. FEEL FREE TO CHANGE IF SOLVING. */ 
	if (gIsInteract) {
		gLoadTierdbArray = FALSE; // SET TO TRUE IF SOLVING
	}
	/********************************/

  gCanonicalPosition = GetCanonicalPosition;
  gMoveToStringFunPtr = &MoveToString;
 	gSymmetries = TRUE;

	combinationsInit();
	hashCacheInit();
	// SYMMETRY
	gCanonicalPosition = GetCanonicalPosition;
	char* board = (char*) SafeMalloc(BOARDSIZE * sizeof(char));
  int i;

	int pminmax[] = {X, 0, PIECENUM, O, 0, 1, BLANK, BOARDSIZE-PIECENUM-1, BOARDSIZE, -1};

	// gUnDoMoveFunPtr = &UndoMove;
	// gGenerateUndoMovesToTierFunPtr = &GenerateUndoMovesToTier;
	// gCustomUnhash = &customUnhash;
	// gReturnTurn = &returnTurn;
	SetupTierStuff();

	for(i = 0; i < PIECENUM+1; i++) {
		board[i] = X;
	}
  for (i = PIECENUM+1; i < BOARDSIZE; i++){
    board[i] = BLANK;
  }
  board[INITIALOPOSITION] = O;

	gNumberOfPositions = generic_hash_init(BOARDSIZE, pminmax, NULL, 0);
	gInitialPosition = hash(board, O, 0);

	// InitializeHelpStrings();
}
/* Return the hash value of the initial position. */
POSITION GetInitialPosition() {
  return gInitialPosition;
}

MOVELIST *dfs(char* board, MOVE moveValue, MOVELIST* moves, int ox, int oy, int depth, int intermediate){
  BOOLEAN canjump = FALSE;
  for (int d = 0; d < ((ox+oy)%2==0 ? 8 : 4); d++){
    //dfs
    int nx = ox + DX[d];
    int ny = oy + DY[d];
    int mx = ox + DX[d]*2;
    int my = oy + DY[d]*2;
    if (InBoard(nx, ny) && InBoard(mx, my) && board[nx*BOARDDIMENSIONY+ny] == X && board[mx*BOARDDIMENSIONY+my] == BLANK){
      canjump = TRUE;
      if ((!gIsInteract || depth == 0) && depth <= LOSINGPIECES){
        board[ox*BOARDDIMENSIONY+oy] = board[nx*BOARDDIMENSIONY+ny] = BLANK;
        board[mx*BOARDDIMENSIONY+my] = O;

        moves = dfs(board, moveValue*moveOffset+(d+1), moves, mx, my, depth+1, intermediate);

        board[ox*BOARDDIMENSIONY+oy] = O;
        board[nx*BOARDDIMENSIONY+ny] = X;
        board[mx*BOARDDIMENSIONY+my] = BLANK;
      }
    }
  }
  if (gIsInteract){
    if (canjump && moveValue != 0)
      moves = CreateMovelistNode(moveValue*3+2, moves);
    if ((moveValue == 0 && intermediate != 0) || (!canjump && moveValue != 0))
      moves = CreateMovelistNode(moveValue*3+1, moves);
  } else {
    if (!canjump && moveValue != 0){
      moves = CreateMovelistNode(moveValue*3+1, moves);
    }
  }
  return moves;
}

/* Return a linked list of moves. */
MOVELIST *GenerateMoves(POSITION position) {
  char turn;
  int numx, intermediate;
  char *board = unhash(position, &turn, &numx, &intermediate);
  MOVELIST *moves = NULL;
  int ox, oy;
  // move
  if (intermediate == 0){
    for (int i = 0; i < BOARDDIMENSIONX; i++){
      for (int j = 0; j < BOARDDIMENSIONY; j++){
        if (board[i*BOARDDIMENSIONY+j] == turn){
          for (int d = 0; d < ((i+j)%2==0 ? 8 : 4); d++){
            int nx = i + DX[d];
            int ny = j + DY[d];
            if (InBoard(nx, ny) && board[nx*BOARDDIMENSIONY+ny] == BLANK){
              int from = i*BOARDDIMENSIONY+j;
              int to = nx*BOARDDIMENSIONY+ny;
              int moveValue = (from*BOARDSIZE+to)*3;
              moves = CreateMovelistNode(moveValue, moves);
            }
          }
        }
      }
    }
  }
  // jump
  if (turn == O){
    for (int i = 0; i < BOARDDIMENSIONX; i++){
      for (int j = 0; j < BOARDDIMENSIONY; j++){
        if (board[i*BOARDDIMENSIONY+j] == O){
          ox = i; oy = j;
          break;
        }
      }
    }
    moves = dfs(board, 0, moves, ox, oy, 0, intermediate);
  }
	SafeFree(board);
  return moves;
}

/* Return the position that results from making the 
input move on the input position. */
POSITION DoMove(POSITION position, MOVE move) {
  char turn;
  int numx, intermediate;
  char *board = unhash(position, &turn, &numx, &intermediate);
  BOOLEAN isIntermediate = move % 3 == 2;
  // move
  if (move % 3 == 0){
    move /= 3;
    int from = move / BOARDSIZE;
    int to = move % BOARDSIZE;
    board[to] = board[from];
    board[from] = BLANK;
  }
  // jump
  else {
    int ox, oy;
    for (int i = 0; i < BOARDDIMENSIONX; i++){
      for (int j = 0; j < BOARDDIMENSIONY; j++){
        if (board[i*BOARDDIMENSIONY+j] == O){
          ox = i; 
          oy = j;
          break;
        }
      }
    }
    // extract moves
    int stack[6];
    int top = -1;
    move /= 3;
    while (move > 0){
      top++;
      stack[top] = move % moveOffset;
      move /= moveOffset;
    }
    // do move
    board[ox*BOARDDIMENSIONY+oy] = BLANK;
    for (; top >= 0; top--){
      int nx = ox + DX[stack[top]-1];
      int ny = oy + DY[stack[top]-1];
      board[nx*BOARDDIMENSIONY+ny] = BLANK;
      ox += DX[stack[top]-1]*2;
      oy += DY[stack[top]-1]*2;
    }
    board[ox*BOARDDIMENSIONY+oy] = O;
  }

  turn = (isIntermediate || turn == X) ? O : X;
	TIER tier; TIERPOSITION tierposition;
	gUnhashToTierPosition(position, &tierposition, &tier);
	gCurrentTier = tier;

  POSITION pos = hash(board, turn, isIntermediate ? 1 : 0);
  SafeFree(board);
  return pos;
}

/*****************************************************************
**  Determine whether the position is a primitive win,
**  primitive tie, primitive lose, or not primitive.
**  OUTPUTS: (VALUE) an enum; one of: (win, lose, tie, undecided)
**  See src/core/types.h for the value enum definition.
******************************************************************/
VALUE Primitive(POSITION position) {
  char turn;
  int numx, intermediate;
  unhash(position, &turn, &numx, &intermediate);
  if (numx <= PIECENUM - LOSINGPIECES)
    return lose;

  MOVELIST* moves = GenerateMoves(position);
  if (NULL == moves)
    return lose;
  FreeMoveList(moves);

  return undecided;
}

/* Symmetry Handling: Return the canonical position. */
POSITION GetCanonicalPosition(POSITION position) {
  char turn;
  int numx, intermediate;
  char *originalBoard = unhash(position, &turn, &numx, &intermediate);
	char canonBoard[BOARDSIZE];
  POSITION canonPos = position;
  int bestSymmetryNum = 0;

  for (int symmetryNum = 1; symmetryNum < MAXSYMMETRY; symmetryNum++){
    for (int i = BOARDSIZE - 1; i >= 0; i--) {
      char pieceInSymmetry = originalBoard[symmetriesToUse[symmetryNum][i]];
      char pieceInBest = originalBoard[symmetriesToUse[bestSymmetryNum][i]];
      if (pieceInSymmetry != pieceInBest) {
        if (pieceInSymmetry > pieceInBest) // If new smallest hash.
          bestSymmetryNum = symmetryNum;
        break;
      }
    }
  }

  if (bestSymmetryNum == 0) {
    SafeFree(originalBoard);
    return position;
  }
  
  for (int i = 0; i < BOARDSIZE; i++) // Transform the rest of the board.
      canonBoard[i] = originalBoard[symmetriesToUse[bestSymmetryNum][i]];

  canonPos = hash(canonBoard, turn, intermediate);
	SafeFree(originalBoard);
  return canonPos;
}

/*********** END SOLVING FUNCTIONS ***********/

BOOLEAN InBoard(int x, int y){
  return x >= 0 && y >= 0 && x < BOARDDIMENSIONX && y < BOARDDIMENSIONY;
}





/*********** BEGIN TEXTUI FUNCTIONS ***********/

void PrintBoard(char* board){
  if (BOARDSIZE == 9)
    printf("\n%c-%c-%c\n|\\|/|\n%c-%c-%c\n|/|\\|\n%c-%c-%c\n",
      board[0],board[1],board[2],board[3],board[4],board[5],board[6],board[7],board[8]);
  else if (BOARDSIZE == 25)
    printf("\n%c-%c-%c-%c-%c\n|\\|/|\\|/|\n%c-%c-%c-%c-%c\n|/|\\|/|\\|\n%c-%c-%c-%c-%c\n|\\|/|\\|/|\n%c-%c-%c-%c-%c\n|/|\\|/|\\|\n%c-%c-%c-%c-%c\n",
      board[0],board[1],board[2],board[3],board[4],board[5],board[6],board[7],board[8],board[9],
      board[10],board[11],board[12],board[13],board[14],board[15],board[16],board[17],board[18],board[19],
      board[20],board[21],board[22],board[23],board[24]);
}

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn) {
  (void) playerName;
  (void) usersTurn;
  char turn;
  int numx, intermediate;
  char *board = unhash(position, &turn, &numx, &intermediate);
  PrintBoard(board);
  printf("turn: %c intermediate: %d\n", turn, intermediate);
}


void PrintComputersMove(MOVE computersMove, STRING computersName) {
  (void) computersName;
    printf("%s", "computer's move: ");
    PrintMove(computersMove);
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
/* Return whether the input text signifies a valid move. */
BOOLEAN ValidTextInput(STRING input) {
	int i = 0;
	while (input[i] != '\0') {
		if (!((input[i] >= 48 && input[i] <= 57) || 
            input[i] == 'm' || input[i] == 'j' || input[i] == ' ')) 
			return FALSE;
		i++;
	}
	return TRUE;
}

/* Assume the text input signifies a valid move. Return
the move hash corresponding to the move. */
MOVE ConvertTextInputToMove(STRING input) {
  MOVE move;
  int i = 2, phase = 0;
  int arg[6] = {0,0,0,0,0,0};
  while (input[i] != '\0'){
		if (input[i] == ' ') {
			phase++;
			i++;
			continue;
		}
    arg[phase] = arg[phase] * 10 + (input[i] - '0');
		i++;
	}
  if (input[0] == 'm'){
    move = (arg[0]*BOARDSIZE+arg[1])*3;
  } else {
    move = arg[0];
    for (int j = 1; j <= phase; j++){
      move *= moveOffset;
      move += arg[j];
    }
    move = move * 3 + 1;
  }
  return move;
}

/* Return the string representation of the move. 
Ideally this matches with what the user is supposed to
type when they specify moves. */
STRING MoveToString(MOVE move) {
  STRING movestring = (STRING) SafeMalloc(40);
  char temp[40];
  if (move % 3 == 0){
    move /= 3;
    int from = move / BOARDSIZE;
    int to = move % BOARDSIZE;
    sprintf(movestring, "m %d %d", from, to);
  } else {
    int stack[6];
    int top = -1;
    move /= 3;
    while (move > 0){
      top++;
      stack[top] = move % moveOffset;
      move /= moveOffset;
    }
    if (top == -1){
      sprintf(movestring, "%s", "s");
    } else {
      sprintf(movestring, "%s", "j");
      for (; top >= 0; top--){
        sprintf(temp, "%s", movestring);
        sprintf(movestring, "%s %d", temp, stack[top]);
      }
    }
  }

  return movestring;
}

/* Basically just print the move. */
void PrintMove(MOVE move) {
  printf("(%s)", MoveToString(move));
}

/*********** END TEXTUI FUNCTIONS ***********/









/*********** BEGIN VARIANT FUNCTIONS ***********/

/* How many variants are you supporting? */
int NumberOfOptions() {
  /* YOUR CODE HERE */
  return 2;
}

/* Return the current variant id. */
int getOption() {
  /* YOUR CODE HERE */
  if (BOARDSIZE == 9) return 0;
  if (BOARDSIZE == 25) return 1;
  return -1;
}

/* The input is a variant id. This function sets any global variables
or data structures according to the variant specified by the variant id. */
void setOption(int option) {
  if (option == 0){
    BOARDDIMENSIONX = BOARDDIMENSIONY = 3;
    PIECENUM = 5;
    INITIALOPOSITION = 4;
    symmetriesToUse = gSymmetryMatrix3x3; 
    LOSINGPIECES = 2;
  } else if (option == 1){
    BOARDDIMENSIONX = BOARDDIMENSIONY = 5;
    PIECENUM = 14;
    INITIALOPOSITION = 12;
    symmetriesToUse = gSymmetryMatrix5x5;
    LOSINGPIECES = 5;
  }
}

/*********** END VARIANT-RELATED FUNCTIONS ***********/







/* Don't worry about these Interact functions below yet.
They are used for the AutoGUI which eventually we would
want to implement, but they are not needed for solving. */
POSITION InteractStringToPosition(STRING board) {
	char realBoard[BOARDSIZE];
	char turn = (board[2] == 'A') ? O : X;
  int intermediate = board[8] == 'I' ? 1 : 0;
  int numx = 0;
  for (int i = 0; i < BOARDSIZE; i++) {
    if (board[i+boardOffset] == '-'){
      realBoard[i] = BLANK;
    } else if (board[i+boardOffset] == 'W'){
      realBoard[i] = X;
      numx++;
    } else {
      realBoard[i] = O;
    }
	}
	gInitializeHashWindow(SetTier(numx, intermediate), FALSE);
	return hash(realBoard, turn, intermediate);
}

STRING InteractPositionToString(POSITION position) {  
  char* result = calloc(boardOffset + BOARDSIZE + 1, sizeof(char));
  memcpy(result, initialInteractString, boardOffset+BOARDSIZE);
  char turn;
  int numx, intermediate;
  char *board = unhash(position, &turn, &numx, &intermediate);
  result[2] = (turn == O) ? 'A' : 'B';
  result[8] = (intermediate == 1) ? 'I' : 'N';

  for (int i = 0; i < BOARDSIZE; i++) {
    if (board[i] != BLANK) {
      result[i+boardOffset] = (board[i] == X) ? 'W' : 'B';
    }
	}
	SafeFree(board);

  result[boardOffset + BOARDSIZE] = '\0';
	return result;
}

STRING InteractMoveToString(POSITION position, MOVE move) {
  if (move % 3 == 0){
    move /= 3;
    int from = move / BOARDSIZE;
    int to = move % BOARDSIZE;
    return UWAPI_Board_Regular2D_MakeMoveString(from+1, to+1);
  } else {
    char turn;
    int numx, intermediate;
    int ox, oy;
    char *board = unhash(position, &turn, &numx, &intermediate);
    for(int i = 0; i < BOARDDIMENSIONX; i++){
      for (int j = 0; j < BOARDDIMENSIONY; j++){
        if (board[i*BOARDDIMENSIONY+j] == O){
          ox = i; oy = j;
          break;
        }
      }
    }
    if (move == 1){
      return UWAPI_Board_Regular2D_MakeAddString('-', ox*BOARDDIMENSIONY+oy+1);
    }
    move /= 3;
    int mx, my;
    mx = ox + DX[move-1]*2;
    my = oy + DY[move-1]*2;
    return UWAPI_Board_Regular2D_MakeMoveString(ox*BOARDDIMENSIONY+oy+1, mx*BOARDDIMENSIONY+my+1);
  }
}