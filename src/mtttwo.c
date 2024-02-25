#include <stdio.h>
#include "gamesman.h"

POSITION gNumberOfPositions = 0;
POSITION kBadPosition = -1;

POSITION gInitialPosition = 0;
POSITION gMinimalPosition = 0;

CONST_STRING kAuthorName = "Stella Wan, Nala Chen, and Cameron Cheung";
CONST_STRING kGameName = "Tic-Tac-Two";
CONST_STRING kDBName = "tttwo";
BOOLEAN kPartizan = TRUE;
BOOLEAN kDebugMenu = TRUE;
BOOLEAN kGameSpecificMenu = TRUE;
BOOLEAN kTieIsPossible = TRUE;
BOOLEAN kLoopy = TRUE;
BOOLEAN kDebugDetermineValue = FALSE;
void* gGameSpecificTclInit = NULL;

CONST_STRING kHelpGraphicInterface = "";
CONST_STRING kHelpTextInterface = "";
CONST_STRING kHelpOnYourTurn = "";
CONST_STRING kHelpStandardObjective = "";
CONST_STRING kHelpReverseObjective = "";
CONST_STRING kHelpTieOccursWhen = /* Should follow 'A Tie occurs when... */ "";
CONST_STRING kHelpExample = "";

/*************************************************************************
**
** Everything above here must be in every game file
**
**************************************************************************/

void unhashCacheInit();
void hashCachePut(TIER tier, TIERPOSITION tierposition, char *board, char turn, int xPlaced, int oPlaced, int gridPos);
BOOLEAN hashCacheGet(TIER tier, POSITION tierposition, char *board, char *turn, int *xPlaced, int *oPlaced, int *gridPos);
void InitializeGame();
void DebugMenu();
void hashBoard(char *board, int xPlaced, int oPlaced, int gridPos, char turn, TIER *tier, TIERPOSITION *tierposition);
POSITION hash(char *board, int xPlaced, int oPlaced, int gridPos, char turn);
char* unhashToBoard(TIER tier, TIERPOSITION tierposition, int *xPlaced, int *oPlaced, int *gridPos, char *turn, char *board);
char *unhash(POSITION position, int *xPlaced, int *oPlaced, int *gridPos, char *turn);
MOVE hashMove(BOOLEAN isGridMove, int from, int to);
void unhashMove(MOVE move, BOOLEAN *isGridMove, int *from, int *to);
void GameSpecificMenu();
POSITION DoMove(POSITION position, MOVE move);
void PrintComputersMove(MOVE computersMove, STRING computersName);
VALUE Primitive(POSITION position);
void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn);
MOVELIST *GenerateMoves(POSITION position);
POSITION GetCanonicalPosition(POSITION position);
void unhashTier(TIER tier, int *xPlaced, int *oPlaced, char *turn);
TIER hashTier(int xPlaced, int oPlaced, char turn);
TIERLIST* TierChildren(TIER tier);
TIERPOSITION NumberOfTierPositions(TIER tier);
STRING TierToString(TIER tier);
POSITION UnDoMove(POSITION position, UNDOMOVE undoMove);
UNDOMOVELIST *GenerateUndoMovesToTier(POSITION position, TIER tier);
BOOLEAN ValidTextInput(STRING input);
MOVE ConvertTextInputToMove(STRING input);
void PrintMove(MOVE theMove);
int NumberOfOptions();
int getOption();
void setOption(int option);
MULTIPARTEDGELIST* GenerateMultipartMoveEdges(POSITION position, MOVELIST *moveList, POSITIONLIST *positionList);

POSITION GetCanonicalPositionTest(POSITION position, POSITION *symmetricTo);

/*************************************************************************
**
** Every variable declared here is only used in this file (game-specific)
**
**************************************************************************/

#define X 'X'
#define O 'O'
#define BLANK '-'
int sideLength = 5;
int boardSize = 25;
int numPiecesPerPlayer = 4; // Number of pieces each player has.
int numGridPlacements = 9; // Number of ways you can place the grid.

int (*gridSlots)[9];
int (*centerMapping);
int (*revCenterMapping);
int (*allTheRows)[8][3];
int (*symmetriesToUse)[25];
int (*numGridAdjacencies);
int (*gridAdjacencies)[8];

int centerMapping4[16] = {-1,-1,-1,-1,-1,0,1,-1,-1,2,3,-1,-1,-1,-1,-1};
int revCenterMapping4[4] = {5,6,9,10};
int gridSlots4[4][9] = {
  {0,1,2,4,5,6,8,9,10},
  {1,2,3,5,6,7,9,10,11},
  {4,5,6,8,9,10,12,13,14},
  {5,6,7,9,10,11,13,14,15}
};
int allTheRows4[4][8][3] = {
  {
    {0,1,2},
    {4,5,6},
    {8,9,10},
    {0,4,8},
    {1,5,9},
    {2,6,10},
    {0,5,10},
    {2,5,8}
  },
  {
    {1,2,3},
    {5,6,7},
    {9,10,11},
    {1,5,9},
    {2,6,10},
    {3,7,11},
    {1,6,11},
    {3,6,9}
  },
  {
    {4,5,6},
    {8,9,10},
    {12,13,14},
    {4,8,12},
    {5,9,13},
    {6,10,14},
    {4,9,14},
    {6,9,12}
  },
  {
    {5,6,7},
    {9,10,11},
    {13,14,15},
    {5,9,13},
    {6,10,14},
    {7,11,15},
    {5,10,15},
    {7,10,13}
  }
};
int numGridAdjacencies4[4] = {3,3,3,3};
int gridAdjacencies4[4][8] = {
  {6,9,10,-1,-1,-1,-1,-1},
  {5,9,10,-1,-1,-1,-1,-1},
  {5,6,10,-1,-1,-1,-1,-1},
  {5,6,9,-1,-1,-1,-1,-1},
};

int centerMapping5[25] = {-1,-1,-1,-1,-1,-1,0,1,2,-1,-1,3,4,5,-1,-1,6,7,8,-1,-1,-1,-1,-1,-1};
int revCenterMapping5[9] = {6,7,8,11,12,13,16,17,18};
int gridSlots5[9][9] = {
  {0,1,2,5,6,7,10,11,12},
  {1,2,3,6,7,8,11,12,13},
  {2,3,4,7,8,9,12,13,14},
  {5,6,7,10,11,12,15,16,17},
  {6,7,8,11,12,13,16,17,18},
  {7,8,9,12,13,14,17,18,19},
  {10,11,12,15,16,17,20,21,22},
  {11,12,13,16,17,18,21,22,23},
  {12,13,14,17,18,19,22,23,24}
};
int allTheRows5[9][8][3] = {
  {
    {0,1,2},
    {5,6,7},
    {10,11,12},
    {0,5,10},
    {1,6,11},
    {2,7,12},
    {0,6,12},
    {2,6,10}
  },
  {
    {1,2,3},
    {6,7,8},
    {11,12,13},
    {1,6,11},
    {2,7,12},
    {3,8,13},
    {1,7,13},
    {3,7,11}
  },
  {
    {2,3,4},
    {7,8,9},
    {12,13,14},
    {2,7,12},
    {3,8,13},
    {4,9,14},
    {2,8,14},
    {4,8,12}
  },
  {
    {5,6,7},
    {10,11,12},
    {15,16,17},
    {5,10,15},
    {6,11,16},
    {7,12,17},
    {5,11,17},
    {7,11,15}
  },
  {
    {6,7,8},
    {11,12,13},
    {16,17,18},
    {6,11,16},
    {7,12,17},
    {8,13,18},
    {6,12,18},
    {8,12,16}
  },
  {
    {7,8,9},
    {12,13,14},
    {17,18,19},
    {7,12,17},
    {8,13,18},
    {9,14,19},
    {7,13,19},
    {9,13,17}
  },
  {
    {10,11,12},
    {15,16,17},
    {20,21,22},
    {10,15,20},
    {11,16,21},
    {12,17,22},
    {10,16,22},
    {12,16,20}
  },
  {
    {11,12,13},
    {16,17,18},
    {21,22,23},
    {11,16,21},
    {12,17,22},
    {13,18,23},
    {11,17,23},
    {13,17,21}
  },
  {
    {12,13,14},
    {17,18,19},
    {22,23,24},
    {12,17,22},
    {13,18,23},
    {14,19,24},
    {12,18,24},
    {14,18,22}
  }
};
int numGridAdjacencies5[9] = {3,5,3,5,8,5,3,5,3};
int gridAdjacencies5[9][8] = {
  {7,11,12,-1,-1,-1,-1,-1},
  {6,8,11,12,13,-1,-1,-1},
  {7,12,13,-1,-1,-1,-1,-1},
  {6,7,12,16,17,-1,-1,-1},
  {6,7,8,11,13,16,17,18},
  {7,8,12,17,18,-1,-1,-1},
  {11,12,17,-1,-1,-1,-1,-1},
  {11,12,13,16,18,-1,-1,-1},
  {12,13,17,-1,-1,-1,-1,-1}
};

int gSymmetryMatrix4[8][25] = {
	{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0},
	{3,2,1,0,7,6,5,4,11,10,9,8,15,14,13,12,0,0,0,0,0,0,0,0,0},
	{12,8,4,0,13,9,5,1,14,10,6,2,15,11,7,3,0,0,0,0,0,0,0,0,0},
	{0,4,8,12,1,5,9,13,2,6,10,14,3,7,11,15,0,0,0,0,0,0,0,0,0},
	{15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,0,0,0,0,0,0,0,0,0},
	{12,13,14,15,8,9,10,11,4,5,6,7,0,1,2,3,0,0,0,0,0,0,0,0,0},
	{3,7,11,15,2,6,10,14,1,5,9,13,0,4,8,12,0,0,0,0,0,0,0,0,0},
	{15,11,7,3,14,10,6,2,13,9,5,1,12,8,4,0,0,0,0,0,0,0,0,0,0}
};

int gSymmetryMatrix5[8][25] = {
	{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24},
	{4,3,2,1,0,9,8,7,6,5,14,13,12,11,10,19,18,17,16,15,24,23,22,21,20},
	{20,15,10,5,0,21,16,11,6,1,22,17,12,7,2,23,18,13,8,3,24,19,14,9,4},
	{0,5,10,15,20,1,6,11,16,21,2,7,12,17,22,3,8,13,18,23,4,9,14,19,24},
	{24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0},
	{20,21,22,23,24,15,16,17,18,19,10,11,12,13,14,5,6,7,8,9,0,1,2,3,4},
	{4,9,14,19,24,3,8,13,18,23,2,7,12,17,22,1,6,11,16,21,0,5,10,15,20},
	{24,19,14,9,4,23,18,13,8,3,22,17,12,7,2,21,16,11,6,1,20,15,10,5,0}
};

POSITION combinations[26][5][5] = {{{1,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0}},{{1,1,0,0,0},{1,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0}},{{1,2,1,0,0},{2,2,0,0,0},{1,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0}},{{1,3,3,1,0},{3,6,3,0,0},{3,3,0,0,0},{1,0,0,0,0},{0,0,0,0,0}},{{1,4,6,4,1},{4,12,12,4,0},{6,12,6,0,0},{4,4,0,0,0},{1,0,0,0,0}},{{1,5,10,10,5},{5,20,30,20,5},{10,30,30,10,0},{10,20,10,0,0},{5,5,0,0,0}},{{1,6,15,20,15},{6,30,60,60,30},{15,60,90,60,15},{20,60,60,20,0},{15,30,15,0,0}},{{1,7,21,35,35},{7,42,105,140,105},{21,105,210,210,105},{35,140,210,140,35},{35,105,105,35,0}},{{1,8,28,56,70},{8,56,168,280,280},{28,168,420,560,420},{56,280,560,560,280},{70,280,420,280,70}},{{1,9,36,84,126},{9,72,252,504,630},{36,252,756,1260,1260},{84,504,1260,1680,1260},{126,630,1260,1260,630}},{{1,10,45,120,210},{10,90,360,840,1260},{45,360,1260,2520,3150},{120,840,2520,4200,4200},{210,1260,3150,4200,3150}},{{1,11,55,165,330},{11,110,495,1320,2310},{55,495,1980,4620,6930},{165,1320,4620,9240,11550},{330,2310,6930,11550,11550}},{{1,12,66,220,495},{12,132,660,1980,3960},{66,660,2970,7920,13860},{220,1980,7920,18480,27720},{495,3960,13860,27720,34650}},{{1,13,78,286,715},{13,156,858,2860,6435},{78,858,4290,12870,25740},{286,2860,12870,34320,60060},{715,6435,25740,60060,90090}},{{1,14,91,364,1001},{14,182,1092,4004,10010},{91,1092,6006,20020,45045},{364,4004,20020,60060,120120},{1001,10010,45045,120120,210210}},{{1,15,105,455,1365},{15,210,1365,5460,15015},{105,1365,8190,30030,75075},{455,5460,30030,100100,225225},{1365,15015,75075,225225,450450}},{{1,16,120,560,1820},{16,240,1680,7280,21840},{120,1680,10920,43680,120120},{560,7280,43680,160160,400400},{1820,21840,120120,400400,900900}},{{1,17,136,680,2380},{17,272,2040,9520,30940},{136,2040,14280,61880,185640},{680,9520,61880,247520,680680},{2380,30940,185640,680680,1701700}},{{1,18,153,816,3060},{18,306,2448,12240,42840},{153,2448,18360,85680,278460},{816,12240,85680,371280,1113840},{3060,42840,278460,1113840,3063060}},{{1,19,171,969,3876},{19,342,2907,15504,58140},{171,2907,23256,116280,406980},{969,15504,116280,542640,1763580},{3876,58140,406980,1763580,5290740}},{{1,20,190,1140,4845},{20,380,3420,19380,77520},{190,3420,29070,155040,581400},{1140,19380,155040,775200,2713200},{4845,77520,581400,2713200,8817900}},{{1,21,210,1330,5985},{21,420,3990,23940,101745},{210,3990,35910,203490,813960},{1330,23940,203490,1085280,4069800},{5985,101745,813960,4069800,14244300}},{{1,22,231,1540,7315},{22,462,4620,29260,131670},{231,4620,43890,263340,1119195},{1540,29260,263340,1492260,5969040},{7315,131670,1119195,5969040,22383900}},{{1,23,253,1771,8855},{23,506,5313,35420,168245},{253,5313,53130,336490,1514205},{1771,35420,336490,2018940,8580495},{8855,168245,1514205,8580495,34321980}},{{1,24,276,2024,10626},{24,552,6072,42504,212520},{276,6072,63756,425040,2018940},{2024,42504,425040,2691920,12113640},{10626,212520,2018940,12113640,51482970}},{{1,25,300,2300,12650},{25,600,6900,50600,265650},{300,6900,75900,531300,2656500},{2300,50600,531300,3542000,16824500},{12650,265650,2656500,16824500,75710250}}};

BOOLEAN kSupportsSymmetries = TRUE; /* Whether we support symmetries */

typedef struct {
	TIER tier;
	TIERPOSITION tierposition;
	char board[25];
	char turn;
	int xPlaced;
	int oPlaced;
  int gridPos;
} UNHASH_RECORD;

#define NUM_UNHASH_RECORDS 0b100000000000000
#define UNHASH_MASK        0b011111111111111

UNHASH_RECORD unhashRecords[NUM_UNHASH_RECORDS];

int unhashCacheInited = 0;

void unhashCacheInit() {
	for (long i = 0; i < NUM_UNHASH_RECORDS; i++) {
		unhashRecords[i].tierposition = -1LL;
	}
	unhashCacheInited = 1;
}

void hashCachePut(TIER tier, TIERPOSITION tierposition, char *board, char turn, int xPlaced, int oPlaced, int gridPos) {
	if (!unhashCacheInited) unhashCacheInit();

	long i = tierposition & UNHASH_MASK;
	if (unhashRecords[i].tier != tier ||
		unhashRecords[i].tierposition != tierposition) {
		unhashRecords[i].tier = tier;
		unhashRecords[i].tierposition = tierposition;
		memcpy(unhashRecords[i].board, board, boardSize);
		unhashRecords[i].turn = turn;
		unhashRecords[i].xPlaced = xPlaced;
		unhashRecords[i].oPlaced = oPlaced;
    unhashRecords[i].gridPos = gridPos;
	}
}

// Returns TRUE if cache miss, otherwise FALSE
BOOLEAN hashCacheGet(TIER tier, POSITION tierposition, char *board, char *turn, int *xPlaced, int *oPlaced, int *gridPos) {
	if (!unhashCacheInited) unhashCacheInit();

	long i = tierposition & UNHASH_MASK;
	if (unhashRecords[i].tier == tier &&
		unhashRecords[i].tierposition == tierposition) {
		memcpy(board, unhashRecords[i].board, boardSize);
		(*turn) = unhashRecords[i].turn;
		(*xPlaced) = unhashRecords[i].xPlaced;
		(*oPlaced) = unhashRecords[i].oPlaced;
    (*gridPos) = unhashRecords[i].gridPos;
		return FALSE;
	}
	return TRUE;
}

/************************************************************************
**
** NAME: InitializeDatabases
**
** DESCRIPTION: Initialize the gDatabase, a global variable.
**
************************************************************************/

void InitializeGame() {

  /* FOR THE PURPOSES OF INTERACT. FEEL FREE TO CHANGE IF SOLVING. */ 
	if (gIsInteract) {
		gLoadTierdbArray = FALSE; // SET TO TRUE IF SOLVING
	}
	/********************************/

	gCanonicalPosition = GetCanonicalPosition;

	kSupportsTierGamesman = TRUE;
	kExclusivelyTierGamesman = TRUE;

	gTierChildrenFunPtr = &TierChildren;
	gNumberOfTierPositionsFunPtr = &NumberOfTierPositions;
	gTierToStringFunPtr = &TierToString;

  gUnDoMoveFunPtr = &UnDoMove;
	gGenerateUndoMovesToTierFunPtr = &GenerateUndoMovesToTier;
  gGenerateMultipartMoveEdgesFunPtr = &GenerateMultipartMoveEdges;

	setOption(getOption());

	unhashCacheInit();
}

/************************************************************************
**
** NAME: DebugMenu
**
** DESCRIPTION: Menu used to debug internal problems. Does nothing if
** kDebugMenu == FALSE
**
************************************************************************/

void DebugMenu() {}

void hashBoard(char *board, int xPlaced, int oPlaced, int gridPos, char turn, TIER *tier, TIERPOSITION *tierposition) {
	POSITION sum = 0;
	int numX = xPlaced;
	int numO = oPlaced;
    for (int i = boardSize - 1; i > 0; i--) { // no need to calculate i == 0
        switch (board[i]) {
            case X:
                numX--;
                break;
            case O:
                if (numX > 0) sum += combinations[i][numO][numX-1];
                numO--;
                break;
            case BLANK:
                if (numX > 0) sum += combinations[i][numO][numX-1];
                if (numO > 0) sum += combinations[i][numO-1][numX];
                break;
        }
    }
	(*tier) = hashTier(xPlaced, oPlaced, turn);
	(*tierposition) = sum * numGridPlacements + centerMapping[gridPos];
	if (oPlaced >= 2 && turn == O) {
		(*tierposition) += (NumberOfTierPositions((*tier)) >> 1);
	}
}

POSITION hash(char *board, int xPlaced, int oPlaced, int gridPos, char turn) {
  if (gHashWindowInitialized) {
		TIER tier; TIERPOSITION tierposition;
		hashBoard(board, xPlaced, oPlaced, gridPos, turn, &tier, &tierposition);
		POSITION position = gHashToWindowPosition(tierposition, tier);
		return position;
	} else { // Not supported.
		return 0;
	}
}

char* unhashToBoard(TIER tier, TIERPOSITION tierposition, int *xPlaced, int *oPlaced, int *gridPos, char *turn, char *board) {
  char fakeTurn;
  unhashTier(tier, xPlaced, oPlaced, &fakeTurn);

  if ((*oPlaced) >= 2) {
		POSITION half = NumberOfTierPositions(tier) >> 1;
		(*turn) = X;
		if (tierposition >= half) {
			(*turn) = O;
			tierposition -= half;
		}
	} else {
    (*turn) = fakeTurn;
  }

  (*gridPos) = revCenterMapping[tierposition % numGridPlacements];
  tierposition /= numGridPlacements;

	POSITION o1, o2;
  int numX = (*xPlaced);
	int numO = (*oPlaced);
    for (int i = boardSize - 1; i >= 0; i--) {
        o1 = (numX > 0) ? combinations[i][numO][numX-1] : 0;
        o2 = o1 + ((numO > 0) ? combinations[i][numO-1][numX] : 0);
        if (tierposition >= o2) {
            board[i] = BLANK;
            tierposition -= o2;
        }
        else if (tierposition >= o1) {
            if (numO > 0) {
                board[i] = O;
                numO--;
            } else
                board[i] = BLANK;
            tierposition -= o1;
        }
        else {
            if (numX > 0) {
                board[i] = X;
                numX--;
            } else if (numO > 0) {
                board[i] = O;
                numO--;
            } else
                board[i] = BLANK;
        }
    }
	return board;
}

char *unhash(POSITION position, int *xPlaced, int *oPlaced, int *gridPos, char *turn) {
  if (gHashWindowInitialized) {
		TIER tier; TIERPOSITION tierposition;
		gUnhashToTierPosition(position, &tierposition, &tier);

		char* board = (char*) SafeMalloc(boardSize * sizeof(char));
		BOOLEAN cache_miss = hashCacheGet(tier, tierposition, board, turn, xPlaced, oPlaced, gridPos);
		if (cache_miss) {
			unhashToBoard(tier, tierposition, xPlaced, oPlaced, gridPos, turn, board);
			hashCachePut(tier, tierposition, board, (*turn), (*xPlaced), (*oPlaced), (*gridPos));
		}
		return board;
	} else { // Not supported.
		return NULL;
	}
}

MOVE hashMove(BOOLEAN isGridMove, int from, int to) {
  return (((isGridMove) ? 1 : 0) << 10) | (from << 5) | (to);
}

void unhashMove(MOVE move, BOOLEAN *isGridMove, int *from, int *to) {
  (*isGridMove) = ((move >> 10) > 0) ? TRUE : FALSE;
  (*from) = (move >> 5) & 0x1F;
  (*to) = move & 0x1F;
}

/************************************************************************
**
** NAME: GameSpecificMenu
**
** DESCRIPTION: Menu used to change game-specific parmeters, such as
** the side of the board in an nxn Nim board, etc. Does
** nothing if kGameSpecificMenu == FALSE
**
************************************************************************/

void GameSpecificMenu() {
  char c;
	BOOLEAN cont = TRUE;
	while (cont) {
    printf("Currently using default %dx%d game settings.\n\n", sideLength, sideLength);
		printf("\tGame Options:\n\n"
		       "\t4)\tLoad default 4x4 Game\n"
		       "\t5)\tLoad default 5x5 Game\n"
		       "\tb)\t(B)ack to the main menu\n"
		       "\nSelect an option:  ");
		c = GetMyChar();
		switch (c) {
		case '4':
			setOption(1);
			break;
		case '5':
			setOption(0);
			break;
		case 'b': case 'B':
			cont = FALSE;
			break;
		default:
			printf("Invalid option!\n");
		}
	}
}

/************************************************************************
**
** NAME: SetTclCGameSpecificOptions
**
** DESCRIPTION: Set the C game-specific options (called from Tcl)
** Ignore if you don't care about Tcl for now.
**
************************************************************************/

void SetTclCGameSpecificOptions(int theOptions[]) {
  (void)theOptions;
}

/************************************************************************
**
** NAME: DoMove
**
** DESCRIPTION: Apply the move to the position.
**
** INPUTS: POSITION position : The old position
** MOVE move : The move to apply.
**
** OUTPUTS: (POSITION) : The position that results after the move.
**
** CALLS: PositionToBlankOX(POSITION,*BlankOX)
** BlankOX WhosTurn(*BlankOX)
**
************************************************************************/

POSITION DoMove(POSITION position, MOVE move) {
  char turn;
  int xPlaced, oPlaced, gridPos;
  char *board = unhash(position, &xPlaced, &oPlaced, &gridPos, &turn);
  BOOLEAN isGridMove;
  int from, to;
  unhashMove(move, &isGridMove, &from, &to);

  if (isGridMove) {
    gridPos = to;
  } else if (from == to) { // Placement
    if (turn == X)
      xPlaced++;
    else
      oPlaced++;
    board[to] = turn;
  } else { // Sliding
    board[from] = BLANK;
    board[to] = turn;
  }
  turn = (turn == X) ? O : X;

  POSITION toReturn = hash(board, xPlaced, oPlaced, gridPos, turn);
  SafeFree(board);
  return toReturn;
}

/************************************************************************
**
** NAME: PrintComputersMove
**
** DESCRIPTION: Nicely format the computers move.
**
** INPUTS: MOVE *computersMove : The computer's move.
** STRING computersName : The computer's name.
**
************************************************************************/

void PrintComputersMove(MOVE computersMove, STRING computersName) {
	printf("%8s's move : ", computersName);
	PrintMove(computersMove);
	printf("\n\n");
}

/************************************************************************
**
** NAME: Primitive
**
** DESCRIPTION: If there is a three in a row in the grid, then that is a
** losing position, else the position is undecided.
**
** INPUTS: POSITION position : The position to inspect.
**
** OUTPUTS: (VALUE) an enum which is oneof: (win,lose,tie,undecided)
**
** CALLS: BOOLEAN ThreeInARow()
** BOOLEAN PositionToGameBoard()
**
************************************************************************/

VALUE Primitive(POSITION position) {
  char turn;
  int xPlaced, oPlaced, gridPos;
  char *board = unhash(position, &xPlaced, &oPlaced, &gridPos, &turn);

  BOOLEAN x3inARow = FALSE;
  BOOLEAN o3inARow = FALSE;
  int cmIdx = centerMapping[gridPos];
  for (int i = 0; i < 8 && !(x3inARow && o3inARow); i++) {
    char first = board[allTheRows[cmIdx][i][0]];
    if (first == board[allTheRows[cmIdx][i][1]] && first == board[allTheRows[cmIdx][i][2]]) {
      if (first == X)
        x3inARow = TRUE;
      else if (first == O)
        o3inARow = TRUE;
    }
  }
  SafeFree(board);
  if (x3inARow && o3inARow) return tie;
  else if (x3inARow) return (turn == X) ? win : lose;
  else if (o3inARow) return (turn == O) ? win : lose;
  return undecided;
}

/************************************************************************
**
** NAME: PrintPosition
**
** DESCRIPTION: Print the position in a pretty format, including the
** prediction of the game's outcome.
**
** INPUTS: POSITION position : The position to pretty print.
** STRING playerName : The name of the player.
** BOOLEAN usersTurn : TRUE <==> it's a user's turn.
**
** CALLS: PositionToBlankOX()
** GetValueOfPosition()
** GetPrediction()
**
************************************************************************/

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn) {
  int xPlaced, oPlaced, gridPos;
  char turn;
  char *board = unhash(position, &xPlaced, &oPlaced, &gridPos, &turn);
  /*printf("INFO: %llu %d %d %d %c\n", position, xPlaced, oPlaced, gridPos, turn);
  for (int m = 0; m < boardSize; m++) {
    printf("%c ", board[m]);
  }
  printf("\n");*/
  // gridx and gridy tell you where the index of the center of the grid is
  int gridx = gridPos % sideLength;
  int gridy = gridPos / sideLength;
  printf("\n");
  for (int i = 0; i < sideLength; i++) {
    // Print two rows for each row in the original board to encode for 
    // pieces (row 1) and inner grid (row 2) 
    printf("  |");
    for (int j = 0; j < sideLength; j++) {
      printf("  "); 
      int index = j + i * sideLength;
      if (board[index] == X) {
        printf("X |");
      } else if (board[index] == O) {
        printf("O |");
      } else {
        printf("  |"); 
      }
    }
    if (i == 1) {
      printf("   %d X Left to Place", numPiecesPerPlayer - xPlaced);
    } else if (i == 2) {
      printf("   Turn: %c", turn);
    } else if (i == 3) {
      printf("   %s", GetPrediction(position, playerName, usersTurn));
    }
    printf("\n");
    printf("  |");
    for (int j = 0; j < sideLength; j++) {
      printf("__"); 
      if ((i >= gridy - 1 && i <= gridy + 1) && (j >= gridx - 1 && j <= gridx + 1)) {
        printf("#_|");
      } else {
        printf("__|");
      }
    }
    if (i == 1) {
      printf("   %d O Left to Place", numPiecesPerPlayer - oPlaced);
    }
    printf("\n");
  }
  printf("\n");
  SafeFree(board);
}

/************************************************************************
**
** NAME: GenerateMoves
**
** DESCRIPTION: Create a linked list of every move that can be reached
** from this position. Return a pointer to the head of the
** linked list.
**
** INPUTS: POSITION position : The position to branch off of.
**
** OUTPUTS: (MOVELIST *), a pointer that points to the first item
** in the linked list of moves that can be generated.
**
** CALLS: MOVELIST *CreateMovelistNode(MOVE,MOVELIST *)
**
************************************************************************/

// Generate Moves:
// If there has been fewer than 4 pieces:
// We can 1) place pieces in empty locations within the grid
// If there has been at least 4 pieces:
// We can 1) place pieces in empty locations within the grid
//        2) move the grid
//        3) place new pieces on empty locations within the grid

MOVELIST *GenerateMoves(POSITION position) {
  MOVELIST *moves = NULL;
  char turn;
  int xPlaced, oPlaced, gridPos;
  char *board = unhash(position, &xPlaced, &oPlaced, &gridPos, &turn);
  int cmIdx = centerMapping[gridPos];
  
  if ((turn == X && (numPiecesPerPlayer - xPlaced)) || (turn == O && (numPiecesPerPlayer - oPlaced))) {
    for (int i = 0; i < 9; i++) { // Placement moves
      int to = gridSlots[cmIdx][i];
      if (board[to] == BLANK) {
        moves = CreateMovelistNode(hashMove(FALSE, to, to), moves);
      }
    }
  }

  if (oPlaced >= 2) { // Grid and sliding moves
    for (int from = 0; from < boardSize; from++) {
      if (board[from] == turn) {
        for (int j = 0; j < 9; j++) { // Sliding Moves
          int to = gridSlots[cmIdx][j];
          if (board[to] == BLANK)
            moves = CreateMovelistNode(hashMove(FALSE, from, to), moves);
        }
      }
    }
    for (int i = 0; i < numGridAdjacencies[cmIdx]; i++) { // Grid Moves
      moves = CreateMovelistNode(hashMove(TRUE, gridPos, gridAdjacencies[cmIdx][i]), moves);
    }
  }

  SafeFree(board);
  return moves;
}

/**************************************************/
/**************** SYMMETRY FUN BEGIN **************/
/**************************************************/

/************************************************************************
**
** NAME: GetCanonicalPosition
**
** DESCRIPTION: Go through all of the positions that are symmetrically
** equivalent and return the SMALLEST, which will be used
** as the canonical element for the equivalence set.
**
** INPUTS: POSITION position : The position return the canonical elt. of.
**
** OUTPUTS: POSITION : The canonical element of the set.
**
************************************************************************/

POSITION DoSymmetry(int symmetry, char *originalBoard, int xPlaced, int oPlaced, int gridPos, char turn) {

	char symBoard[boardSize];
  int symGridPos;

  for (int i = 0; i < boardSize; i++)
    symBoard[symmetriesToUse[symmetry][i]] = originalBoard[i];
  symGridPos = symmetriesToUse[symmetry][gridPos];
  //printf("%d %d %d\n", gridPos, symGridPos, symmetry);

  return hash(symBoard, xPlaced, oPlaced, symGridPos, turn);
}

POSITION GetCanonicalPositionTest(POSITION position, POSITION *symmetricTo) {
  char turn;
	int xPlaced, oPlaced, gridPos;
	char *originalBoard = unhash(position, &xPlaced, &oPlaced, &gridPos, &turn);
  POSITION canonPos = position;
  for (int i = 0; i < 8; i++) {
    POSITION symPos = DoSymmetry(i, originalBoard, xPlaced, oPlaced, gridPos, turn);
    symmetricTo[i] = symPos;
    if (symPos < canonPos) canonPos = symPos;
  }
  if (xPlaced >= 2 && xPlaced == oPlaced) {
    for (int i = 0; i < 25; i++) {
      if (originalBoard[i] != BLANK) {
        originalBoard[i] = (originalBoard[i] == X) ? O : X;
      }
    }
    turn = (turn == X) ? O : X;
    for (int i = 0; i < 8; i++) {
      POSITION symPos = DoSymmetry(i, originalBoard, xPlaced, oPlaced, gridPos, turn);
      symmetricTo[i + 8] = symPos;
      if (symPos < canonPos) canonPos = symPos;
    }
  }
  SafeFree(originalBoard);
  return canonPos;
}

POSITION GetCanonicalPosition(POSITION position) {
  char turn;
	int xPlaced, oPlaced, gridPos;
	char *originalBoard = unhash(position, &xPlaced, &oPlaced, &gridPos, &turn);
  POSITION canonPos = position;
  for (int i = 0; i < 8; i++) {
    POSITION symPos = DoSymmetry(i, originalBoard, xPlaced, oPlaced, gridPos, turn);
    if (symPos < canonPos) canonPos = symPos;
  }
  if (xPlaced >= 2 && xPlaced == oPlaced) {
    for (int i = 0; i < 25; i++) {
      if (originalBoard[i] != BLANK) {
        originalBoard[i] = (originalBoard[i] == X) ? O : X;
      }
    }
    turn = (turn == X) ? O : X;
    for (int i = 0; i < 8; i++) {
      POSITION symPos = DoSymmetry(i, originalBoard, xPlaced, oPlaced, gridPos, turn);
      if (symPos < canonPos) canonPos = symPos;
    }
  }
  SafeFree(originalBoard);
  return canonPos;
}

/**************************************************/
/**************** SYMMETRY FUN END ****************/
/**************************************************/

void unhashTier(TIER tier, int *xPlaced, int *oPlaced, char *turn) {
	(*xPlaced) = (tier / 10) % 10;
	(*oPlaced) = tier % 10;
  (*turn) = X;
  if ((*xPlaced) <= 2 && (*oPlaced) == (*xPlaced) - 1) {
    (*turn) = O;
  }
}

TIER hashTier(int xPlaced, int oPlaced, char turn) {
  (void)turn;
	return xPlaced * 10 + oPlaced;
}

TIERLIST* TierChildren(TIER tier) {
	TIERLIST* list = NULL;

  switch (tier) {
    case 0:
      list = CreateTierlistNode(10, list);
    break;
    case 10:
      list = CreateTierlistNode(11, list);
    break;
    case 11:
      list = CreateTierlistNode(21, list);
    break;
    case 21:
      list = CreateTierlistNode(22, list);
    break;
    case 22:
      list = CreateTierlistNode(22, list);
      list = CreateTierlistNode(23, list);
      list = CreateTierlistNode(32, list);
    break;
    case 23:
      list = CreateTierlistNode(23, list);
      list = CreateTierlistNode(24, list);
      list = CreateTierlistNode(33, list);
    break;
    case 24:
      list = CreateTierlistNode(24, list);
      list = CreateTierlistNode(34, list);
    break;
    case 32:
      list = CreateTierlistNode(32, list);
      list = CreateTierlistNode(33, list);
      list = CreateTierlistNode(42, list);
    break;
    case 33:
      list = CreateTierlistNode(33, list);
      list = CreateTierlistNode(34, list);
      list = CreateTierlistNode(43, list);
    break;
    case 34:
      list = CreateTierlistNode(34, list);
      list = CreateTierlistNode(44, list);
    break;
    case 42:
      list = CreateTierlistNode(42, list);
      list = CreateTierlistNode(43, list);
    break;
    case 43:
      list = CreateTierlistNode(43, list);
      list = CreateTierlistNode(44, list);
    break;
    case 44:
      list = CreateTierlistNode(44, list);
    break;
    default:
    break;
  }

  return list;
}

TIERPOSITION NumberOfTierPositions(TIER tier) {
	int xPlaced, oPlaced;
  char turn;
	unhashTier(tier, &xPlaced, &oPlaced, &turn);
	return ((oPlaced >= 2) ? 2 : 1) * numGridPlacements * combinations[boardSize][oPlaced][xPlaced];
}

STRING TierToString(TIER tier) {
	STRING tierStr = (STRING) SafeMalloc(sizeof(char) * 40);
	int xPlaced, oPlaced;
	char turn;
	unhashTier(tier, &xPlaced, &oPlaced, &turn);

  if (oPlaced >= 2) {
	  sprintf(tierStr, "%d X on board, %d O on board", xPlaced, oPlaced);
  } else {
    sprintf(tierStr, "%d X on board, %d O on board, %c's Turn", xPlaced, oPlaced, turn);
  }
	return tierStr;
}

POSITION UnDoMove(POSITION position, UNDOMOVE undoMove) {
	int xPlaced, oPlaced, gridPos;
	char turn;
	char *board = unhash(position, &xPlaced, &oPlaced, &gridPos, &turn);
	char oppTurn = (turn == X) ? O : X;
	int from, to;
  BOOLEAN isGridMove;
	unhashMove((MOVE) undoMove, &isGridMove, &from, &to);

	if (isGridMove) {
    gridPos = from;
  } else {
    board[to] = BLANK;
    if (from == to) {
      if (oppTurn == X) xPlaced--;
      else oPlaced--;
    } else {
      board[from] = oppTurn;
    }
  }
  turn = oppTurn;

	POSITION toReturn = hash(board, xPlaced, oPlaced, gridPos, turn);
	SafeFree(board);
	return toReturn;
}

UNDOMOVELIST *GenerateUndoMovesToTier(POSITION position, TIER tier) {
	UNDOMOVELIST *undoMoves = NULL;
	int xPlaced, oPlaced, gridPos;
	char turn;
	char *board = unhash(position, &xPlaced, &oPlaced, &gridPos, &turn);
	char oppTurn = (turn == X) ? O : X;
  int cmIdx = centerMapping[gridPos];

	int toXPlaced, toOPlaced;
	char toTurn;
	unhashTier(tier, &toXPlaced, &toOPlaced, &toTurn);

  if (xPlaced == toXPlaced && oPlaced == toOPlaced) { // Slide or gridmove
    for (int from = 0; from < boardSize; from++) {
      if (board[from] == BLANK) {
        for (int j = 0; j < 9; j++) { // Sliding Moves
          int to = gridSlots[cmIdx][j];
          if (board[to] == oppTurn)
            undoMoves = CreateUndoMovelistNode(hashMove(FALSE, from, to), undoMoves);
        }
      }
    }
    for (int i = 0; i < numGridAdjacencies[cmIdx]; i++) { // Grid Moves
      undoMoves = CreateUndoMovelistNode(hashMove(TRUE, gridAdjacencies[cmIdx][i], gridPos), undoMoves);
    }
  } else if ((toXPlaced == xPlaced - 1 && oppTurn == X) || (toOPlaced == oPlaced - 1 && oppTurn == O)) { // xPlaced
    for (int i = 0; i < 9; i++) { // Placement moves
      int to = gridSlots[cmIdx][i];
      if (board[to] == oppTurn)
        undoMoves = CreateUndoMovelistNode(hashMove(FALSE, to, to), undoMoves);
    }
  }

  SafeFree(board);

	return undoMoves;
}

/************************************************************************
**
** NAME: GetAndPrintPlayersMove
**
** DESCRIPTION: This finds out if the player wanted an undo or abort or not.
** If so, return Undo or Abort and don't change theMove.
** Otherwise get the new theMove and fill the pointer up.
**
** INPUTS: POSITION *thePosition : The position the user is at.
** MOVE *theMove : The move to fill with user's move.
** STRING playerName : The name of the player whose turn it is
**
** OUTPUTS: USERINPUT : Oneof( Undo, Abort, Continue )
**
** CALLS: ValidMove(MOVE, POSITION)
** BOOLEAN PrintPossibleMoves(POSITION) ...Always True!
**
************************************************************************/

USERINPUT GetAndPrintPlayersMove(POSITION thePosition, MOVE *theMove, STRING playerName) {
  USERINPUT input;
	for (;;) {
		printf("%8s's move:  ", playerName);
		input = HandleDefaultTextInput(thePosition, theMove, playerName);
		if (input != Continue) return input;
	}

  /* NOTREACHED */
  return Continue;
}

/************************************************************************
**
** NAME: ValidTextInput
**
** DESCRIPTION: Return TRUE iff the string input is of the right 'form'.
** For example, if the user is allowed to select one slot
** from the numbers 1-9, and the user chooses 0, it's not
** valid, but anything from 1-9 IS, regardless if the slot
** is filled or not. Whether the slot is filled is left up
** to another routine.
**
** INPUTS: STRING input : The string input the user typed.
**
** OUTPUTS: BOOLEAN : TRUE iff the input is a valid text input.
**
************************************************************************/

BOOLEAN ValidTextInput(STRING input) {
  if (input[0] != 'A' && input[0] != 'G' && input[0] != 'M') {
    return FALSE;
  }
  if (input[1] != '-') {
    return FALSE;
  }
  return TRUE;
}

/************************************************************************
**
** NAME: ConvertTextInputToMove
**
** DESCRIPTION: Convert the string input to the internal move representation.
**
** INPUTS: STRING input : The string input the user typed.
**
** OUTPUTS: MOVE : The move corresponding to the user's input.
**
************************************************************************/

MOVE ConvertTextInputToMove(STRING input) {
  char type = input[0]; 
  int source; 
  int dest; 
  MOVE ret = 0;
  if (type == 'A') {
    if (input[3] != 0) {
      dest = (input[2] - '0') * 10 + (input[3] - '0');
    } else {
      dest = input[2] - '0'; 
    }
    ret = hashMove(FALSE, dest - 1, dest - 1);
  } else if (type == 'M' || type == 'G') {
    // Get source
    if (input[3] != '-') {
      source = (input[2] - '0') * 10 + (input[3] - '0');
      // Get dest 
      if (input[6] != 0) {
        dest = (input[5] - '0') * 10 + (input[6] - '0');
      } else {
        dest = input[5] - '0'; 
      }
    } else {
      source = input[2] - '0'; 
      // Get dest 
      if (input[5] != 0) {
        dest = (input[4] - '0') * 10 + (input[5] - '0');
      } else {
        dest = input[4] - '0'; 
      }
    }
    if (type == 'M') {
      ret = hashMove(FALSE, source - 1, dest - 1);
    } else {
      ret = hashMove(TRUE, source - 1, dest - 1);
    }
  } else {
    printf("You should not be here. Something went wrong."); 
  }
  return ret; 
}

/************************************************************************
**
** NAME: MoveToString
**
** DESCRIPTION: Returns the move as a STRING
**
** INPUTS: MOVE *move : The move to put into a string.
**
** A-DEST, M-SRC-DEST, G-SRC-DEST
************************************************************************/
void MoveToString(MOVE move, char *moveStringBuffer) {
  BOOLEAN isGridMove;
  int from, to;
  unhashMove(move % 100000, &isGridMove, &from, &to);
  // 0: placing new pieces
  // message: "A-DEST"
  if (move >= 500000) { // Move is "choose to move grid"; 500000 + move
    sprintf(moveStringBuffer, "G");
  } else if (move >= 300000) { // Move is "select piece to move"; 300000 + move
    sprintf(moveStringBuffer, "M-%d", from);
  } else {
    if (from == to) {
      sprintf(moveStringBuffer, "A-%d", to + 1);
    }
    // 1: moving existing pieces
    // message: "M-SRC-DEST"
    else if (!isGridMove) {
      sprintf(moveStringBuffer, "M-%d-%d", from + 1, to + 1);
    }
    // 2: moving the grid
    // message: "G-SRC-DEST"
    else {
      sprintf(moveStringBuffer, "G-%d-%d", from + 1, to + 1); 
    }
  }
}

/************************************************************************
**
** NAME: PrintMove
**
** DESCRIPTION: Print the move in a nice format.
**
** INPUTS: MOVE *theMove : The move to print.
**
************************************************************************/

void PrintMove(MOVE theMove) {
  char moveStringBuffer[10];
  MoveToString(theMove, moveStringBuffer);
	printf("%s", moveStringBuffer);
}

int NumberOfOptions() {
  return 2;
}

int getOption() {
  return (boardSize == 25) ? 0 : 1;
}

void setOption(int option) {
  if (option == 0) {
    boardSize = 25;
    sideLength = 5;
    numGridPlacements = 9;
    symmetriesToUse = gSymmetryMatrix5;
    allTheRows = allTheRows5;
    centerMapping = centerMapping5;
    revCenterMapping = revCenterMapping5;
    gridSlots = gridSlots5;
    numGridAdjacencies = numGridAdjacencies5;
    gridAdjacencies = gridAdjacencies5;

    char board[25] = {
			BLANK,BLANK,BLANK,BLANK,BLANK,
			BLANK,BLANK,BLANK,BLANK,BLANK,
			BLANK,BLANK,BLANK,BLANK,BLANK,
			BLANK,BLANK,BLANK,BLANK,BLANK,
			BLANK,BLANK,BLANK,BLANK,BLANK
		};
		hashBoard(board, 0, 0, 12, X, &gInitialTier, &gInitialTierPosition);
  } else {
    boardSize = 16;
    sideLength = 4;
    numGridPlacements = 4;
    symmetriesToUse = gSymmetryMatrix4;
    allTheRows = allTheRows4;
    centerMapping = centerMapping4;
    revCenterMapping = revCenterMapping4;
    gridSlots = gridSlots4;
    numGridAdjacencies = numGridAdjacencies4;
    gridAdjacencies = gridAdjacencies4;

    char board[16] = {
			BLANK,BLANK,BLANK,BLANK,
			BLANK,BLANK,BLANK,BLANK,
			BLANK,BLANK,BLANK,BLANK,
			BLANK,BLANK,BLANK,BLANK
		};
		hashBoard(board, 0, 0, 5, X, &gInitialTier, &gInitialTierPosition);
  }

  gInitialPosition = gInitialTierPosition;
}

POSITION encodeIntermediatePosition(POSITION position, BOOLEAN isGridMove, int from) {
	// 0b1 1 00000 0; intermediate marker (1), isGridMove (1), from (5)
	return position | (1LL << 63) | (((isGridMove) ? 1LL : 0LL) << 62) | (((POSITION) from) << 57); 
}

BOOLEAN decodeIntermediatePosition(POSITION interPos, POSITION *origPos, BOOLEAN *isGridMove, int *from) {
	(*origPos) = interPos & 0xFFFFFFFFFFFFFF;
	(*isGridMove) = ((interPos >> 62) & 1) ? TRUE : FALSE;
	(*from) = (interPos >> 57) & 0x1F;
	return (interPos >> 63) ? TRUE : FALSE;
}

POSITION StringToPosition(char *positionString) {
	int turnInt;
	char *board;
	if (ParseStandardOnelinePositionString(positionString, &turnInt, &board)) {
		char turn = (turnInt == 1) ? X : O;
    int xPlaced = 0, oPlaced = 0;
    int i;
    for (i = 0; i < boardSize; i++) {
      switch (board[i]) {
        case 'X':
          xPlaced++;
          break;
        case 'O':
          oPlaced++;
          break;
        default:
          break;
      }
    }
    int gridPos = 12;
    for (; i < boardSize + numGridPlacements; i++) {
      if (board[i] == 'G') {
        gridPos = revCenterMapping[i - boardSize];
        break;
      }
    }
    TIER tier;
    TIERPOSITION tierposition;
    hashBoard(board, xPlaced, oPlaced, gridPos, turn, &tier, &tierposition);
    gInitializeHashWindow(tier, FALSE);
    return tierposition;
	}
	return NULL_POSITION;
}

/* boardSize (pieces) + numGridPlacements (where grid is) + 1 ("select grid center" sign + also for multipart) + 2 (multipart) */
void PositionToAutoGUIString(POSITION position, char *autoguiPositionStringBuffer) {
	POSITION pos;
  BOOLEAN isGridMove;
  int from;
  BOOLEAN isIntermediate = decodeIntermediatePosition(position, &pos, &isGridMove, &from);

  int xPlaced, oPlaced, gridPos;
  char turn;
  char *board = unhash(pos, &xPlaced, &oPlaced, &gridPos, &turn);

  int turnInt = (turn == X) ? 1 : 2;

  int entityStringSize = boardSize + numGridPlacements + 2;
  char finalBoard[entityStringSize];
  memset(finalBoard, '-', entityStringSize * sizeof(char));
  memcpy(finalBoard, board, boardSize * sizeof(char));

  int cmIdx = centerMapping[gridPos];
  finalBoard[boardSize + cmIdx] = 'G';

  if (isIntermediate) {
    if (isGridMove) {
      finalBoard[entityStringSize - 2] = 'T';
    } else {
      finalBoard[entityStringSize - 2] = 'a' + from;
    }
  }
  finalBoard[entityStringSize - 1] = '\0';
  SafeFree(board);
  AutoGUIMakePositionString(turnInt, finalBoard, autoguiPositionStringBuffer);
}

void MoveToAutoGUIString(POSITION position, MOVE move, char *autoguiMoveStringBuffer) {
  (void) position;
  int isGridMove, from, to;
  unhashMove(move % 100000, &isGridMove, &from, &to);
  if (move >= 500000) { // Move is "choose to move grid"; 500000 + move
    AutoGUIMakeMoveButtonStringA('g', boardSize + numGridPlacements, 'x', autoguiMoveStringBuffer);
  } else if (move >= 400000) { // Move is "choose where to move grid"; 400000 + move
    AutoGUIMakeMoveButtonStringA('h', to, 'z', autoguiMoveStringBuffer);
  } else if (move >= 300000) { // Move is "select piece to move"; 300000 + move
    AutoGUIMakeMoveButtonStringA('h', from, 'y', autoguiMoveStringBuffer);
  } else if (move >= 200000) { // Move is "select where to move piece"; 200000 + move
    AutoGUIMakeMoveButtonStringM(from, to, 'z', autoguiMoveStringBuffer);
  } else {
    if (from == to) { // Single-part move
      AutoGUIMakeMoveButtonStringA('h', to, 'x', autoguiMoveStringBuffer);
    } else { // A full-move that is multipart
      AutoGUIWriteEmptyString(autoguiMoveStringBuffer);
    }
  }
}

// CreateMultipartEdgeListNode(POSITION from, POSITION to, MOVE partMove, MOVE fullMove, BOOLEAN isTerminal, MULTIPARTEDGELIST *next)
MULTIPARTEDGELIST* GenerateMultipartMoveEdges(POSITION position, MOVELIST *moveList, POSITIONLIST *positionList) {
	MULTIPARTEDGELIST *mpel = NULL;
  int edgeFromAdded = 0;
  BOOLEAN gridMoveAdded = FALSE;
  POSITION gridMoveInterPos = encodeIntermediatePosition(position, TRUE, 0);
	while (moveList != NULL) {
    BOOLEAN isGridMove;
    int from, to;
    unhashMove(moveList->move, &isGridMove, &from, &to);

    if (isGridMove) {
      if (!gridMoveAdded) {
        // Add "choose to move grid" partMove
        mpel = CreateMultipartEdgeListNode(NULL_POSITION, gridMoveInterPos, 500000 + moveList->move, 0, mpel);
        gridMoveAdded = TRUE;
      }
      // Add "choose where to move grid" partMove
      mpel = CreateMultipartEdgeListNode(gridMoveInterPos, NULL_POSITION, 400000 + moveList->move, moveList->move, mpel);
    } else if (from != to) {
      POSITION slideMoveInterPos = encodeIntermediatePosition(position, FALSE, from);
      if (!(edgeFromAdded & (1 << from))) {
        // Add "select piece to move" partMove
        mpel = CreateMultipartEdgeListNode(NULL_POSITION, slideMoveInterPos, 300000 + moveList->move, 0, mpel);
        edgeFromAdded ^= (1 << from);
      }
      mpel = CreateMultipartEdgeListNode(slideMoveInterPos, NULL_POSITION, 200000 + moveList->move, moveList->move, mpel);
    }
    // Ignore placement moves, they're single-part

		moveList = moveList->next;
		positionList = positionList->next;
	}
	return mpel;
}