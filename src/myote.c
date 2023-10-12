/************************************************************************
**
** NAME:        myote.c
**
** DESCRIPTION: Yote
**
** AUTHOR:      Haonan Huang
**
** DATE:        2022-04-18
**
************************************************************************/

#include "gamesman.h"

/* IMPORTANT GLOBAL VARIABLES */
CONST_STRING kAuthorName = "Haonan Huang";
CONST_STRING kGameName = "Yote"; //  use this spacing and case
CONST_STRING kDBName = "yote"; // use this spacing and case
POSITION gNumberOfPositions; // Put your number of positions upper bound here.
POSITION gInitialPosition; // Put the hash value of the initial position.
BOOLEAN kPartizan = TRUE; // Is the game PARTIZAN i.e. given a board does each player have a different set of moves available to them?
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

int gSymmetryMatrix3x3[8][20] = {
  {0,1,2,3,4,5,6,7,8,0,0,0,0,0,0,0,0,0,0,0},
  {6,3,0,7,4,1,8,5,2,0,0,0,0,0,0,0,0,0,0,0},
  {8,7,6,5,4,3,2,1,0,0,0,0,0,0,0,0,0,0,0,0},
  {2,5,8,1,4,7,0,3,6,0,0,0,0,0,0,0,0,0,0,0},
  {2,1,0,5,4,3,8,7,6,0,0,0,0,0,0,0,0,0,0,0},
  {0,3,6,1,4,7,2,5,8,0,0,0,0,0,0,0,0,0,0,0},
  {6,7,8,3,4,5,0,1,2,0,0,0,0,0,0,0,0,0,0,0},
  {8,5,2,7,4,1,6,3,0,0,0,0,0,0,0,0,0,0,0,0}
};
int gSymmetryMatrix3x4[4][20] = {
	{0,1,2,3,4,5,6,7,8,9,10,11,0,0,0,0,0,0,0,0},
  {3,2,1,0,7,6,5,4,11,10,9,8,0,0,0,0,0,0,0,0},
  {11,10,9,8,7,6,5,4,3,2,1,0,0,0,0,0,0,0,0,0},
  {8,9,10,11,4,5,6,7,0,1,2,3,0,0,0,0,0,0,0,0}
};

int gSymmetryMatrix4x4[8][20] = {
	{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,0,0,0},
	{3,2,1,0,7,6,5,4,11,10,9,8,15,14,13,12,0,0,0,0},
	{12,8,4,0,13,9,5,1,14,10,6,2,15,11,7,3,0,0,0,0},
	{0,4,8,12,1,5,9,13,2,6,10,14,3,7,11,15,0,0,0,0},
	{15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,0,0,0},
	{12,13,14,15,8,9,10,11,4,5,6,7,0,1,2,3,0,0,0,0},
	{3,7,11,15,2,6,10,14,1,5,9,13,0,4,8,12,0,0,0,0},
	{15,11,7,3,14,10,6,2,13,9,5,1,12,8,4,0,0,0,0}
};

int gSymmetryMatrix4x5[4][20] = {
	{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19},
  {4,3,2,1,0,9,8,7,6,5,14,13,12,11,10,19,18,17,16,15},
  {19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0},
  {15,16,17,18,19,10,11,12,13,14,5,6,7,8,9,0,1,2,3,4}
};

int BOARDDIMENSIONX = 3; //default
int BOARDDIMENSIONY = 3; //default
int BOARDSIZE = 9; //default
int PIECENUM = 3; //default
int MAXSYMMETRY = 8; //default
int (*symmetriesToUse)[20] = gSymmetryMatrix3x3; //default
const int DX[4] = {0, 1, 0, -1};
const int DY[4] = {1, 0, -1, 0};
const int INVALID = 25;
const int leftxIndex = 8;
const int leftoIndex = 9;
const int numxIndex = 10;
const int numoIndex = 11;
const int multipartFromIndex = 12;
const int multipartPassIndex = 13;
const int multipartToIndex = 14;
const int boardOffset = 15;
char initialInteractString[] = "R_A_0_0_--------------------------------------------------------";


#define BLANK '.'
#define X 'X'
#define O 'O'
#define PLAYER_ONE 1
#define PLAYER_TWO 2

// HASH/UNHASH
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
MULTIPARTEDGELIST* GenerateMultipartMoveEdges(POSITION position, MOVELIST *moveList, POSITIONLIST *positionList);


/*********** BEGIN HASHING FUNCIONS ***********/

TIER SetTier(int leftx, int lefto, int numx, int numo){
  return ((leftx*10+lefto)*10+numx)*10+numo;
}

void GetFromTier(TIER tier, int* leftx, int* lefto, int* numx, int* numo){
  *leftx = tier / 1000;
  *lefto = (tier / 100) % 10;
  *numx = (tier / 10) % 10;
  *numo = tier % 10;
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

unsigned long long combinations[21][9][9];
void combinationsInit() {
  for (int boardsize=0; boardsize<=BOARDSIZE; boardsize++)
    for (int numx=0; numx<=PIECENUM; numx++)
      for (int numo=0; numo<=PIECENUM; numo++)
        combinations[boardsize][numx][numo] = getNumPos(boardsize, numx, numo);
}

typedef struct {
	int tier;
	POSITION position;
	char board[20];
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

// (Perfect) hash the board
unsigned long long hashIt(int boardsize, int numx, int numo, char *board) {
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

void unhashIt(int boardsize, int numx, int numo, POSITION pos, char *board) {
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

POSITION hash(char* board, char turn, int leftx, int lefto, int numx, int numo) {
	POSITION pos = 0;
	if (gHashWindowInitialized) {
		TIER tier = SetTier(leftx, lefto, numx, numo);
		generic_hash_context_switch(tier);
		TIERPOSITION tierposition = hashIt(BOARDSIZE, numx, numo, board);

		if (turn == O) {
			tierposition += combinations[BOARDSIZE][numx][numo];
		}
		pos = gHashToWindowPosition(tierposition, tier);
	}
	return pos;
}

char* unhash(POSITION pos, char* turn, int* leftx, int* lefto, int* numx, int* numo) {
	//piecesLeft = total pieces left during stage 1 (x + o)
	char* board = (char*)SafeMalloc(BOARDSIZE * sizeof(char));
	if (gHashWindowInitialized) {
		TIER tier; TIERPOSITION tierposition;
		gUnhashToTierPosition(pos, &tierposition, &tier);
		generic_hash_context_switch(tier);
		(*turn) = (generic_hash_turn(tierposition) == PLAYER_ONE ? X : O);

    GetFromTier(tier, leftx, lefto, numx, numo);

		BOOLEAN cache_miss = hashCacheGet(tier, tierposition, board);
		if (cache_miss) {
			unhashIt(BOARDSIZE, *numx, *numo, tierposition % combinations[BOARDSIZE][*numx][*numo], board);
			hashCachePut(tier, tierposition, board);
		}
	}
	return board;
}

/*********** END HASHING FUNCIONS ***********/

/*********** BEGIN TIER FUNCIONS ***********/

/* Initialize any global variables or data structures needed before
solving or playing the game. */

void SetupTierStuff() {
	generic_hash_destroy();
	char* board = (char*) SafeMalloc(BOARDSIZE * sizeof(char));
	kSupportsTierGamesman = TRUE;
	gTierChildrenFunPtr = &TierChildren;
	gNumberOfTierPositionsFunPtr = &NumberOfTierPositions;
	gTierToStringFunPtr = &TierToString;
	int tier, i, leftx, lefto, numx, numo;
	generic_hash_custom_context_mode(TRUE);
	int pieces_array[] = {X, 0, 0, O, 0, 0, BLANK, 0, 0, -1};
	kExclusivelyTierGamesman = TRUE;

  for (leftx=0; leftx<=PIECENUM; leftx++) {
    for (lefto=0; lefto<=PIECENUM; lefto++) {
      for (numx=0; numx+leftx<=PIECENUM; numx++) {
        for (numo=0; numo+lefto<=PIECENUM; numo++) {
          tier = SetTier(leftx, lefto, numx, numo);
          pieces_array[1]=pieces_array[2]=numx;
          pieces_array[4]=pieces_array[5]=numo;
          pieces_array[7]=pieces_array[8]=BOARDSIZE-numx-numo;

          generic_hash_init(BOARDSIZE, pieces_array, NULL, 0);
          generic_hash_set_context(tier);
          
        }
      }
    }
  }
  
  tier = SetTier(PIECENUM, PIECENUM, 0, 0);
	gInitialTier = tier;    //initial pieces on board
	gCurrentTier = tier;
	generic_hash_context_switch(tier);

	for(i = 0; i < BOARDSIZE; i++) {
		board[i] = BLANK;
	}
	gInitialTierPosition = generic_hash_hash(board, PLAYER_ONE);
}

TIERLIST* TierChildren(TIER tier) {
  int leftx, lefto, numx, numo;
  GetFromTier(tier, &leftx, &lefto, &numx, &numo);

	TIERLIST* list = NULL;
  // move
	list = CreateTierlistNode(tier, list);
  
  // put
  if (leftx > 0){
    list = CreateTierlistNode(tier-1000+10, list);
  }
  if (lefto > 0){
    list = CreateTierlistNode(tier-100+1, list);
  }
  // jump
  if (numx > 0 && numo > 0){
    if (numx > 1) {
      list = CreateTierlistNode(tier-20, list);
    } else {
      list = CreateTierlistNode(tier-10, list);
    }
    if (numo > 1){
      list = CreateTierlistNode(tier-2, list);
    } else {
      list = CreateTierlistNode(tier-1, list);
    }
  }
  
	return list;
}

TIERPOSITION NumberOfTierPositions(TIER tier) {
	generic_hash_context_switch(tier);
	return generic_hash_max_pos();
}

STRING TierToString(TIER tier) {
  int leftx, lefto, numx, numo;
  GetFromTier(tier, &leftx, &lefto, &numx, &numo);
	STRING str = (STRING) SafeMalloc (sizeof (char) * 60);
	sprintf(str, "leftx: %d lefto: %d numx: %d numo: %d", leftx, lefto, numx, numo);
	return str;
}

/*********** END TIER FUNCIONS ***********/


/*********** BEGIN SOLVING FUNCTIONS ***********/
void InitializeGame() {

  /* FOR THE PURPOSES OF INTERACT. FEEL FREE TO CHANGE IF SOLVING. */ 
  if (gIsInteract) {
    gLoadTierdbArray = FALSE; // SET TO TRUE IF SOLVING
  }
  /********************************/

  BOARDSIZE = BOARDDIMENSIONX * BOARDDIMENSIONY;

  gCanonicalPosition = GetCanonicalPosition;
  gMoveToStringFunPtr = &MoveToString;
 	gSymmetries = TRUE;

	combinationsInit();
	hashCacheInit();
	// SYMMETRY
	gCanonicalPosition = GetCanonicalPosition;
	char* board = (char*) SafeMalloc(BOARDSIZE * sizeof(char));

	int pminmax[] = {X, 0, PIECENUM, O, 0, PIECENUM, BLANK, BOARDSIZE-PIECENUM*2, BOARDSIZE, -1};

	// gUnDoMoveFunPtr = &UndoMove;
	// gGenerateUndoMovesToTierFunPtr = &GenerateUndoMovesToTier;
	// gCustomUnhash = &customUnhash;
	// gReturnTurn = &returnTurn;
	SetupTierStuff();

	for(int i = 0; i < BOARDSIZE; i++)
		board[i] = BLANK;

	gNumberOfPositions = generic_hash_init(BOARDSIZE, pminmax, NULL, 0);
	gInitialPosition = hash(board, X, PIECENUM, PIECENUM, 0, 0);

	gGenerateMultipartMoveEdgesFunPtr = &GenerateMultipartMoveEdges;

	// InitializeHelpStrings();
}
/* Return the hash value of the initial position. */
POSITION GetInitialPosition() {
  return 0;
}

/* Return a linked list of moves. */
MOVELIST *GenerateMoves(POSITION position) {
  MOVELIST *moves = NULL;
	char turn, opponent;
  int leftx, lefto, numx, numo;
  char *board = unhash(position, &turn, &leftx, &lefto, &numx, &numo);

  opponent = turn == X ? O : X;
  // put
  if ((leftx > 0 && turn == X) || (lefto > 0 && turn == O)){
    for (int i = 0; i < BOARDSIZE; i++){
      if (board[i] == BLANK){
        moves = CreateMovelistNode(MoveEncode(INVALID, i, INVALID, INVALID), moves);
      }
    }
  }
  // move
  if ((numx > 0 && turn == X) || (numo > 0 && turn == O)){
    for (int i = 0; i < BOARDDIMENSIONX; i++){
      for (int j = 0; j < BOARDDIMENSIONY; j++){
        if (board[i*BOARDDIMENSIONY+j] == turn){
          for (int d = 0; d < 4; d++){
            int nx = i + DX[d];
            int ny = j + DY[d];
            if (InBoard(nx, ny) && board[nx*BOARDDIMENSIONY+ny] == BLANK){
              int from = i*BOARDDIMENSIONY+j;
              int to = nx*BOARDDIMENSIONY+ny;
              moves = CreateMovelistNode(MoveEncode(from, to, INVALID, INVALID), moves);
            }
          }
        }
      }
    }
  }
  // jump
  if (numx > 0 && numo > 0){
    for (int i = 0; i < BOARDDIMENSIONX; i++){
      for (int j = 0; j < BOARDDIMENSIONY; j++){
        if (board[i*BOARDDIMENSIONY+j] == turn){
          for (int d = 0; d < 4; d++){
            int nx = i + DX[d];
            int ny = j + DY[d];
            int mx = i + DX[d]*2;
            int my = j + DY[d]*2;
            if (InBoard(nx, ny) && InBoard(mx, my) && board[nx*BOARDDIMENSIONY+ny] == opponent && board[mx*BOARDDIMENSIONY+my] == BLANK){
              int from = i*BOARDDIMENSIONY+j;
              int to = mx*BOARDDIMENSIONY+my;
              int pass = nx*BOARDDIMENSIONY+ny;
              // cannot remove
              if ((opponent == X && numx == 1) || (opponent == O && numo == 1)){
                moves = CreateMovelistNode(MoveEncode(from, to, pass, INVALID), moves);
              } else {
                for (int k = 0; k < BOARDSIZE; k++){
                  if (k != pass && board[k] == opponent){
                    moves = CreateMovelistNode(MoveEncode(from, to, pass, k), moves);
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  

	SafeFree(board);
  return moves;
}

/* Return the position that results from making the 
input move on the input position. */
POSITION DoMove(POSITION position, MOVE move) {
  char turn;
  int leftx, lefto, numx, numo;
  char *board = unhash(position, &turn, &leftx, &lefto, &numx, &numo);
  int from, to, pass, remove;
  MoveDecode(move, &from, &to, &pass, &remove);

  // put
  if (from == INVALID){
    board[to] = turn;
    if (turn == X) {
      numx++; leftx--;
    } else {
      numo++; lefto--;
    }
  }
  // move
  else if (pass == INVALID){
    board[from] = BLANK;
    board[to] = turn;
  }
  // jump
  else {
    if (remove != INVALID){
      board[remove] = BLANK;
    }
    board[from] = board[pass] = BLANK;
    board[to] = turn;
    if (turn == O){
      if (remove != INVALID)
        numx--;
      numx--;
    } else {
      if (remove != INVALID)
        numo--;
      numo--;
    }
  }
  turn = turn == X ? O : X;
	TIER tier; TIERPOSITION tierposition;
	gUnhashToTierPosition(position, &tierposition, &tier);
	gCurrentTier = tier;
  
	POSITION pos = hash(board, turn, leftx, lefto, numx, numo);
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
  int leftx, lefto, numx, numo;
  unhash(position, &turn, &leftx, &lefto, &numx, &numo);
  if (leftx == 0 || lefto == 0){
    if (numx == 0 || numo == 0)
      return lose;
		MOVELIST* moves = GenerateMoves(position);
		if (NULL == moves)
			return lose;
    FreeMoveList(moves);
  }

  return undecided;
}

/* Symmetry Handling: Return the canonical position. */
POSITION GetCanonicalPosition(POSITION position) {
  char turn;
  int leftx, lefto, numx, numo;
  char *originalBoard = unhash(position, &turn, &leftx, &lefto, &numx, &numo);
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

  canonPos = hash(canonBoard, turn, leftx, lefto, numx, numo);
	SafeFree(originalBoard);
  return canonPos;
}

/*********** END SOLVING FUNCTIONS ***********/


/*********** BEGIN AUXILIARY FUNCTIONS ***********/
BOOLEAN InBoard(int x, int y){
  return x >= 0 && y >= 0 && x < BOARDDIMENSIONX && y < BOARDDIMENSIONY;
}

MOVE MoveEncode(int from, int to, int pass, int remove) {
  return((from << 15) | (to << 10) | (pass << 5) | remove);
}

void MoveDecode(MOVE move, int* from, int* to, int* pass, int* remove){
  *from = move >> 15;
  *to = (move >> 10) & 0x1F;
  *pass = (move >> 5) & 0x1F;
  *remove = move & 0x1F;
}

/*********** END AUXILIARY FUNCTIONS ***********/




/*********** BEGIN TEXTUI FUNCTIONS ***********/

void PrintBoard(char* board){
  printf("\n");
  for (int i = 0; i < BOARDDIMENSIONX; i++){
    for (int j = 0; j < BOARDDIMENSIONY; j++){
      printf("%c", board[i*BOARDDIMENSIONY+j]);
    }
    printf("\n");
  }
}

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn) {
  (void) playerName;
  (void) usersTurn;
  char turn;
  int leftx, lefto, numx, numo;
  char *board = unhash(position, &turn, &leftx, &lefto, &numx, &numo);
  PrintBoard(board);
  printf("X left: %d, O left: %d, turn: %c\n", leftx, lefto, turn);
  // printf("%s", InteractPositionToString(position));  
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
BOOLEAN ValidTextInput(STRING input) {
	int i = 0;
	while (input[i] != '\0') {
		if (!((input[i] >= 48 && input[i] <= 57) || input[i] == '-' || input[i] == 'r' || 
            input[i] == 'p' || input[i] == 'm' || input[i] == 'j' || input[i] == ' ')) 
			return FALSE;
		i++;
	}
	return TRUE;
}

/* Assume the text input signifies a valid move. Return
the move hash corresponding to the move. */
MOVE ConvertTextInputToMove(STRING input) {
  int from = INVALID, to = INVALID, pass = INVALID, remove = INVALID;

  int i = 2, phase = 0;
  int arg[4] = {0,0,0,0};
	while (input[i] != '\0') {
		if (input[i] == ' ') {
			phase++;
			i++;
			continue;
		}
    arg[phase] = arg[phase] * 10 + (input[i] - '0');
		i++;
	}
  if (input[0] == 'p'){
    to = arg[0];
  } else if (input[0] == 'm'){
    from = arg[0];
    to = arg[1];
  } else if (input[0] == 'j'){
    from = arg[0];
    to = arg[2];
    pass = arg[1];
    remove = arg[3];
  }

  MOVE move = MoveEncode(from, to, pass, remove);

  PrintMove(move);
  return move;
}

/* Return the string representation of the move. 
Ideally this matches with what the user is supposed to
type when they specify moves. */
STRING MoveToString(MOVE move) {
	STRING movestring = (STRING) SafeMalloc(30);
  int from, to, pass, remove;
  MoveDecode(move, &from, &to, &pass, &remove);

  if (from == INVALID){
    sprintf(movestring, "(p %d)", to);
  }
  else if (pass == INVALID){
    sprintf(movestring, "(m %d %d)", from, to);
  }
  else {
    sprintf(movestring, "(j %d %d %d %d)", from, pass, to, remove);
  }
  return movestring;
}

/* Basically just print the move. */
void PrintMove(MOVE move) {
  printf("%s", MoveToString(move));
}

/*********** END TEXTUI FUNCTIONS ***********/









/*********** BEGIN VARIANT FUNCTIONS ***********/

/* How many variants are you supporting? */
int NumberOfOptions() {
  return 4;
}

/* Return the current variant id. */
int getOption() {
  return BOARDDIMENSIONX+BOARDDIMENSIONY-6;
}

/* The input is a variant id. This function sets any global variables
or data structures according to the variant specified by the variant id. */
void setOption(int option) {
  if (option == 0){
    BOARDDIMENSIONX = BOARDDIMENSIONY = 3;
    symmetriesToUse = gSymmetryMatrix3x3;
    PIECENUM = 3;
    MAXSYMMETRY = 8;
  } else if (option == 1){
    BOARDDIMENSIONX = 3;
    BOARDDIMENSIONY = 4;
    symmetriesToUse = gSymmetryMatrix3x4;
    PIECENUM = 4;
    MAXSYMMETRY = 4;
  } else if (option == 2){
    BOARDDIMENSIONX = BOARDDIMENSIONY = 4;
    symmetriesToUse = gSymmetryMatrix4x4; 
    PIECENUM = 6;
    MAXSYMMETRY = 8;
  } else if (option == 3){
    BOARDDIMENSIONX = 4;
    BOARDDIMENSIONY = 5;
    symmetriesToUse = gSymmetryMatrix4x5; 
    PIECENUM = 8;
    MAXSYMMETRY = 4;
  }
}

/*********** END VARIANT-RELATED FUNCTIONS ***********/







/* Don't worry about these Interact functions below yet.
They are used for the AutoGUI which eventually we would
want to implement, but they are not needed for solving. */
POSITION InteractStringToPosition(STRING board) {
	char realBoard[BOARDSIZE];
	char turn = (board[2] == 'A') ? X : O;
	int numx = board[numxIndex] - '0';
	int numo = board[numoIndex] - '0';
  int leftx = board[leftxIndex] - '0';
  int lefto = board[leftoIndex] - '0';

	for (int i = 0; i < BOARDSIZE; i++) {
    if (board[i+boardOffset] == '-'){
      realBoard[i] = BLANK;
    } else {
      realBoard[i] = (board[i+boardOffset] == 'W') ? X : O;
    }
	}

	// Conversion from intermediate to real
  if (board[multipartFromIndex] != '-'){
    int from = board[multipartFromIndex] - 'A';
    int to = board[multipartToIndex] - 'A';
    int pass = board[multipartPassIndex] -'A';
    if (realBoard[to] == X){
      realBoard[pass] = O;
      numo++;
    } else {
      realBoard[pass] = X;
      numx++;
    }
    realBoard[from] = realBoard[to];
    realBoard[to] = BLANK;
  }
	// End Conversion from intermediate to real
	gInitializeHashWindow(SetTier(leftx, lefto, numx, numo), FALSE);
	return hash(realBoard, turn, leftx, lefto, numx, numo);
}

STRING InteractPositionToString(POSITION position) {
  char* result = calloc(boardOffset + BOARDSIZE + 1, sizeof(char));
  memcpy(result, initialInteractString, boardOffset+BOARDSIZE);
	int origFrom = INVALID, origTo = INVALID, origPass = INVALID;
  // printf("%llx", position);
	if (position >> 63) {
		origFrom = (position >> 58) & 0x1F;
		origPass = (position >> 53) & 0x1F;
		origTo = (position >> 48) & 0x1F;
		position &= 0xFFFFFFFFFF;
    // printf("%llx", position);
	}
  // printf("\n");

  char turn;
  int leftx, lefto, numx, numo;
  char *board = unhash(position, &turn, &leftx, &lefto, &numx, &numo);

  result[2] = (turn == X) ? 'A' : 'B';

	for (int i = 0; i < BOARDSIZE; i++) {
    if (board[i] != BLANK) {
      result[i+boardOffset] = (board[i] == X) ? 'W' : 'B';
    }
	}
	SafeFree(board);

	if (origTo != INVALID) {
		result[origTo+boardOffset] = (turn == X) ? 'W' : 'B';
    if (turn == X){
      numo--;
    } else {
      numx--;
    }
		result[multipartToIndex] = origTo + 'A';
    result[origFrom+boardOffset] = '-';
    result[multipartFromIndex] = origFrom + 'A';
    result[origPass+boardOffset] = '-';
    result[multipartPassIndex] = origPass + 'A';
	}

  result[numxIndex] = numx + '0';
	result[numoIndex] = numo + '0';
	result[leftxIndex] = leftx + '0';
	result[leftoIndex] = lefto + '0';

  result[boardOffset + BOARDSIZE] = '\0';
	return result;
}

STRING InteractMoveToString(POSITION position, MOVE move) {
  int from, to, pass, remove;
  MoveDecode(move, &from, &to, &pass, &remove);
  
  char turn;
  int leftx, lefto, numx, numo;
  char *board = unhash(position, &turn, &leftx, &lefto, &numx, &numo);
	SafeFree(board);

	if (pass == INVALID && remove == INVALID) { // Fullmove
    if (from != INVALID) { // move or jump but cannot remove
			return UWAPI_Board_Regular2D_MakeMoveString(7+from, 7+to);
    } else { // put
			return UWAPI_Board_Regular2D_MakeAddString('-', to+7);
    }
	} else {
		if (from != INVALID && to != INVALID) { // jumping piece partmove
			return UWAPI_Board_Regular2D_MakeMoveString(7+from, 7+to);
		} else { // Removing opponent piece partmove
			return UWAPI_Board_Regular2D_MakeAddString('-', remove+7);
		}
	}
}

// CreateMultipartEdgeListNode(POSITION from, POSITION to, MOVE partMove, MOVE fullMove, BOOLEAN isTerminal, MULTIPARTEDGELIST *next)
MULTIPARTEDGELIST* GenerateMultipartMoveEdges(POSITION position, MOVELIST *moveList, POSITIONLIST *positionList) {
	// Assumes moveList/positionList is same ordering as generated in GenerateMoves
	MULTIPARTEDGELIST *mpel = NULL;
  char turn;
  int leftx, lefto, numx, numo;
  char *board = unhash(position, &turn, &leftx, &lefto, &numx, &numo);
	POSITION currIntermediatePosition = 0;

	while (moveList != NULL) {
		MOVE move = moveList->move;

    int from, to, pass, remove;
    MoveDecode(move, &from, &to, &pass, &remove);

		if (remove != INVALID) {
			if (to != INVALID) {
				currIntermediatePosition = (1LL << 63) | ((POSITION)from << 58) | ((POSITION)pass << 53) | ((POSITION)to << 48) | position;
				mpel = CreateMultipartEdgeListNode(position, currIntermediatePosition, MoveEncode(from, to, pass, INVALID), 0, FALSE, mpel);
			}
			
			mpel = CreateMultipartEdgeListNode(currIntermediatePosition, positionList->position, MoveEncode(INVALID, INVALID, INVALID, remove), move, TRUE, mpel);
		}

		moveList = moveList->next;
		positionList = positionList->next;
	}

	SafeFree(board);
	return mpel;
}