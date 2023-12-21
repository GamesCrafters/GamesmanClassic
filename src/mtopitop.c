/************************************************************************
**
** NAME:        mtopitop.c
**
** DESCRIPTION: Topitop
**
** AUTHOR:      (Version 1) Mike Hamada and Alex Choy
**              (Version 2) Matthew Yu and Cameron Cheung
**
** DATE:        February 2022
**
**************************************************************************/

#include "gamesman.h"

/* Defines */
#define BLUE 'B'
#define RED 'R'

#define BLUEBUCKETPIECE 'B'
#define REDBUCKETPIECE 'R'
#define SMALLPIECE 'S'
#define LARGEPIECE 'L'
#define BLUESMALLPIECE 'X'
#define REDSMALLPIECE 'O'
#define CASTLEPIECE 'C'
#define BLUECASTLEPIECE 'P'
#define REDCASTLEPIECE 'Q'
#define BLANKPIECE '-'

#define BLUEDISPLAY "  B  "
#define REDDISPLAY "  R  "
#define SMALLDISPLAY " /_\\ "
#define LARGEDISPLAY "/___\\"
#define BLANKDISPLAY "     "

#define NULLMOVE 0b1111111111
/***/

POSITION gNumberOfPositions = 0;
POSITION kBadPosition = -1;

POSITION gInitialPosition = 0;
POSITION gMinimalPosition = 0;

int gSymmetryMatrix[8][9] = {
    {0,1,2,3,4,5,6,7,8},
    {6,3,0,7,4,1,8,5,2},
    {8,7,6,5,4,3,2,1,0},
    {2,5,8,1,4,7,0,3,6},
    {2,1,0,5,4,3,8,7,6},
    {0,3,6,1,4,7,2,5,8},
    {6,7,8,3,4,5,0,1,2},
    {8,5,2,7,4,1,6,3,0}
};

int numAdjacencies[9] = {3, 5, 3, 5, 8, 5, 3, 5, 3};

int adjacencyMatrix[9][8] = {
	{1, 3, 4, 9, 9, 9, 9, 9},
	{0, 2, 3, 4, 5, 9, 9, 9},
	{1, 4, 5, 9, 9, 9, 9, 9},
	{0, 1, 4, 6, 7, 9, 9, 9},
	{0, 1, 2, 3, 5, 6, 7, 8},
	{1, 2, 4, 7, 8, 9, 9, 9},
	{3, 4, 7, 9, 9, 9, 9, 9},
	{3, 4, 5, 6, 8, 9, 9, 9},
	{4, 5, 7, 9, 9, 9, 9, 9}
};

int movesToIds[9][9] = {
	{0, 1, 100, 2, 3, 100, 100, 100, 100},
	{4, 0, 5, 6, 7, 8, 100, 100, 100},
	{100, 9, 0, 100, 10, 11, 100, 100, 100},
	{12, 13, 100, 0, 14, 100, 15, 16, 100},
	{17, 18, 19, 20, 0, 21, 22, 23, 24},
	{100, 25, 26, 100, 27, 0, 100, 28, 29},
	{100, 100, 100, 30, 31, 100, 0, 32, 100},
	{100, 100, 100, 33, 34, 35, 36, 0, 37},
	{100, 100, 100, 100, 38, 39, 100, 40, 0}
};

int idsToMoves[2][41] = {
	{0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 6, 6, 6, 7, 7, 7, 7, 7, 8, 8, 8},
	{0, 1, 3, 4, 0, 2, 3, 4, 5, 1, 4, 5, 0, 1, 4, 6, 7, 0, 1, 2, 3, 5, 6, 7, 8, 1, 2, 4, 7, 8, 3, 4, 7, 3, 4, 5, 6, 8, 4, 5, 7}
};

CONST_STRING kAuthorName = "(V1) Mike Hamada, Alex Choy; (V2) Matthew Yu, Cameron Cheung";
CONST_STRING kGameName = "Topitop";
CONST_STRING kDBName = "topitop";   /* The name to store the database under */
BOOLEAN kPartizan = TRUE;
BOOLEAN kDebugMenu = FALSE;
BOOLEAN kGameSpecificMenu = TRUE;
BOOLEAN kTieIsPossible = FALSE;
BOOLEAN kLoopy = TRUE;
BOOLEAN kDebugDetermineValue = FALSE;
BOOLEAN kSupportsSymmetries = TRUE; /* Whether we support symmetries */
void* gGameSpecificTclInit = NULL;

BOOLEAN tyingRule = FALSE;
VALUE ifNoLegalMoves = undecided;
BOOLEAN isMisere = FALSE;

CONST_STRING kHelpGraphicInterface = "";

CONST_STRING kHelpTextInterface = "";

CONST_STRING kHelpOnYourTurn =
"Each player takes turns making one valid move, also noting\n\
that player CANNOT reverse an opponent's move that was just made.\n\
Use the BOARD to determine which numbers to choose to\n\
correspond to either a piece to place and a board slot to place it\n\
OR to the location of your piece or a neutral piece and the empty\n\
adjacent position you wish to move that piece to.\n\
\n\
VALID MOVES:\n\
1.) Place 1 of your Buckets or 1 of the Small or Large Sand Piles\n\
    on any free space of the board.\n\
2.) Move 1 of your Buckets or any Small or Large Sand Pile already\n\
    placed on the board from one square to another (one space at\n\
    a time in any direction).\n\
       a.) Any piece already on the board can be moved to an \n\
           adjacent free space.\n\
       b.) A Bucket can go on top of a Small Sand Pile.\n\
       c.) A Small Sand Pile can go on top of a Large Sand Pile.\n\
       d.) A Small Sand Pile with a Bucket on its top can go on\n\
           top of a Large Sand Pile.\n\
       e.) A Bucket can go on top of a Sand Castle.\n\
    NOTE: You CANNOT place a piece on top of another piece.\n\
          Pieces must be MOVED on top of other pieces.\n\
3.) Move any combination of Sand Piles with your Buckets on top,\n\
    or any Sand Castle, to any free space.";

CONST_STRING kHelpStandardObjective =
"Be the first to have your 2 Buckets on top of 2 Sand Castles,\n\
where a Sand Castle consists of 1 Small Sand Pile put on top\n\
of 1 Large Sand Pile.";

CONST_STRING kHelpReverseObjective = "";

CONST_STRING kHelpTieOccursWhen = "A tie never occurs.";

CONST_STRING kHelpExample = "";

/*************************************************************************
**
** Everything above here must be in every game file
**
**************************************************************************/

void InitializeGame();
void DebugMenu();
char* unhashPosition(POSITION position, char *turn, int *disallowedMove, int *blueLeft, int *redLeft, int *smallLeft, int *largeLeft);
POSITION hashPosition(char* board, char turn, int disallowedMove);
void unhashMove(MOVE move, char *piece, int *from, int *to, BOOLEAN *p2Turn);
MOVE hashMove(char piece, int from, int to, BOOLEAN p2Turn);
void GameSpecificMenu();
void SetTclCGameSpecificOptions(int theOptions[]);
POSITION DoMove(POSITION position, MOVE move);
POSITION GetInitialPosition();
void PrintComputersMove(MOVE computersMove, STRING computersName);
char* PrintHelper(char piece, int level);
void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn);
MOVELIST *GenerateMoves(POSITION position);
POSITION GetCanonicalPosition(POSITION position);
USERINPUT GetAndPrintPlayersMove(POSITION thePosition, MOVE *theMove, STRING playerName);
BOOLEAN ValidTextInput(STRING input);
MOVE ConvertTextInputToMove(STRING input);
void PrintMove(MOVE theMove);
STRING DisallowedMoveToString(int disallowedMove);
STRING MoveToString(MOVE move);
int NumberOfOptions();
int getOption();
void setOption(int option);
POSITION InteractStringToPosition(STRING board);
STRING InteractPositionToString(POSITION pos);
STRING InteractMoveToString(POSITION pos, MOVE mv);

void countPiecesOnBoard(char *board, int *bb, int *rb, int *bs, int *rs, int *bc, int *rc, int *s, int *l, int *c);

void unhashCacheInit();
void hashCachePut(TIER tier, TIERPOSITION tierposition, char *board, char turn, int disallowedMove, int blueLeft, int redLeft, int smallLeft, int largeLeft);
BOOLEAN hashCacheGet(TIER tier, POSITION tierposition, char *board, char *turn, int *disallowedMove, int *blueLeft, int *redLeft, int *smallLeft, int *largeLeft);


void unhashTier(TIER tier, int *bb, int *rb, int *bs, int *rs, int *bc, int *rc, int *s, int *l, int *c);
TIER hashTier(TIER bb, TIER rb, TIER bs, TIER rs, TIER bc, TIER rc, TIER s, TIER l, TIER c);
TIERLIST* gTierChildren(TIER tier);
TIERPOSITION gNumberOfTierPositions(TIER tier);
STRING TierToString(TIER tier);

POSITION fact(int n);
char* unhashToBoard(TIER tier, TIERPOSITION tierposition, char *turn, int *disallowedMove, int *blueLeft, int *redLeft, int *smallLeft, int *largeLeft);
TIERPOSITION numRearrangements(int length, int numBlanks, int bb, int rb, int bs, int rs, int bc, int rc, int s, int l, int c);
void hashBoard(char *board, char turn, int disallowedMove, TIER *tier, TIERPOSITION *tierposition);
BOOLEAN isLegal(POSITION position);

int (*symmetriesToUse)[9];

void InitializeGame() {

	/* FOR THE PURPOSES OF INTERACT. FEEL FREE TO CHANGE IF SOLVING. */ 
	if (gIsInteract) {
		gLoadTierdbArray = FALSE; // SET TO TRUE IF SOLVING
	}
	/********************************/
	
	gCanonicalPosition = GetCanonicalPosition;
	gMoveToStringFunPtr = &MoveToString;

	kSupportsTierGamesman = TRUE;
	kExclusivelyTierGamesman = TRUE;
	gInitialTier = 0;
	gInitialTierPosition = 0;
	gTierChildrenFunPtr = &gTierChildren;
	gNumberOfTierPositionsFunPtr = &gNumberOfTierPositions;
	gTierToStringFunPtr = &TierToString;
	gIsLegalFunPtr = &isLegal;

	symmetriesToUse = gSymmetryMatrix;
	unhashCacheInit();
}

/////

typedef struct {
	TIER tier;
	TIERPOSITION tierposition;
	char board[9];
	char turn;
	int disallowedMove;
	int blueLeft;
	int redLeft;
	int smallLeft;
	int largeLeft;
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

void hashCachePut(TIER tier, TIERPOSITION tierposition, char *board, char turn, int disallowedMove, int blueLeft, int redLeft, int smallLeft, int largeLeft) {
	if (!unhashCacheInited) unhashCacheInit();

	long i = tierposition & UNHASH_MASK;
	if (unhashRecords[i].tier != tier ||
		unhashRecords[i].tierposition != tierposition) {
		unhashRecords[i].tier = tier;
		unhashRecords[i].tierposition = tierposition;
		memcpy(unhashRecords[i].board, board, 9);
		unhashRecords[i].turn = turn;
		unhashRecords[i].disallowedMove = disallowedMove;
		unhashRecords[i].blueLeft = blueLeft;
		unhashRecords[i].redLeft = redLeft;
		unhashRecords[i].smallLeft = smallLeft;
		unhashRecords[i].largeLeft = largeLeft;
	}
}

// Returns TRUE if cache miss, otherwise FALSE
BOOLEAN hashCacheGet(TIER tier, POSITION tierposition, char *board, char *turn, int *disallowedMove, int *blueLeft, int *redLeft, int *smallLeft, int *largeLeft) {
	if (!unhashCacheInited) unhashCacheInit();

	long i = tierposition & UNHASH_MASK;
	if (unhashRecords[i].tier == tier &&
		unhashRecords[i].tierposition == tierposition) {
		memcpy(board, unhashRecords[i].board, 9);
		(*turn) = unhashRecords[i].turn;
		(*disallowedMove) = unhashRecords[i].disallowedMove;
		(*blueLeft) = unhashRecords[i].blueLeft;
		(*redLeft) = unhashRecords[i].redLeft;
		(*smallLeft) = unhashRecords[i].smallLeft;
		(*largeLeft) = unhashRecords[i].largeLeft;
		return FALSE;
	}
	return TRUE;
}

/////

void DebugMenu() {

}

POSITION fact(int n) {
    if (n <= 1) return 1;
	POSITION prod = 1;
	for (int i = 2; i <= n; i++)
		prod *= i;
    return prod;
}

char* unhashToBoard(TIER tier, TIERPOSITION tierposition, char *turn, int *disallowedMove, int *blueLeft, int *redLeft, int *smallLeft, int *largeLeft) {
	char *board = (char*) SafeMalloc(sizeof(char) * 9);
	int bb, rb, bs, rs, bc, rc, s, l, c;
	unhashTier(tier, &bb, &rb, &bs, &rs, &bc, &rc, &s, &l, &c);
	int numBlanks = 9 - bb - rb - bs - rs - bc - rc - s - l - c;

	(*disallowedMove) = tierposition % 41;
	TIERPOSITION half = gNumberOfTierPositions(tier) >> 1;
	(*turn) = (tierposition >= half) ? RED : BLUE;
	(*blueLeft) = 2 - bb - bs - bc;
	(*redLeft) = 2 - rb - rs - rc;
	(*smallLeft) = 4 - bs - rs - bc - rc - s - c;
	(*largeLeft) = 4 - bc - rc - l - c;

	tierposition -= (tierposition % 41);
	if (tierposition >= half) {
		tierposition -= half;
	}
	tierposition /= 41;

	TIERPOSITION idxOfFirstBLUEBUCKETPIECE, idxOfFirstREDBUCKETPIECE, idxOfFirstSMALLPIECE, idxOfFirstLARGEPIECE, idxOfFirstBLUESMALLPIECE, idxOfFirstREDSMALLPIECE, idxOfFirstCASTLEPIECE, idxOfFirstBLUECASTLEPIECE, idxOfFirstREDCASTLEPIECE;
	for (int i = 0; i < 9; i++) {
		TIERPOSITION lastIdx = 0;
		
		if (numBlanks > 0) {
			lastIdx += numRearrangements(8 - i, numBlanks - 1, bb, rb, bs, rs, bc, rc, s, l, c);
		}

		if (bb > 0) {
			idxOfFirstBLUEBUCKETPIECE = lastIdx;
			lastIdx += numRearrangements(8 - i, numBlanks, bb - 1, rb, bs, rs, bc, rc, s, l, c);
		} else {
			idxOfFirstBLUEBUCKETPIECE = -1;
		}

		if (rb > 0) {
			idxOfFirstREDBUCKETPIECE = lastIdx;
			lastIdx += numRearrangements(8 - i, numBlanks, bb, rb - 1, bs, rs, bc, rc, s, l, c);
		} else {
			idxOfFirstREDBUCKETPIECE = -1;
		}

		if (s > 0) {
			idxOfFirstSMALLPIECE = lastIdx;
			lastIdx += numRearrangements(8 - i, numBlanks, bb, rb, bs, rs, bc, rc, s - 1, l, c);
		} else {
			idxOfFirstSMALLPIECE = -1;
		}

		if (l > 0) {
			idxOfFirstLARGEPIECE = lastIdx;
			lastIdx += numRearrangements(8 - i, numBlanks, bb, rb, bs, rs, bc, rc, s, l - 1, c);
		} else {
			idxOfFirstLARGEPIECE = -1;
		}

		if (bs > 0) {
			idxOfFirstBLUESMALLPIECE = lastIdx;
			lastIdx += numRearrangements(8 - i, numBlanks, bb, rb, bs - 1, rs, bc, rc, s, l, c);
		} else {
			idxOfFirstBLUESMALLPIECE = -1;
		}

		if (rs > 0) {
			idxOfFirstREDSMALLPIECE = lastIdx;
			lastIdx += numRearrangements(8 - i, numBlanks, bb, rb, bs, rs - 1, bc, rc, s, l, c);
		} else {
			idxOfFirstREDSMALLPIECE = -1;
		}

		if (c > 0) {
			idxOfFirstCASTLEPIECE = lastIdx;
			lastIdx += numRearrangements(8 - i, numBlanks, bb, rb, bs, rs, bc, rc, s, l, c - 1);
		} else {
			idxOfFirstCASTLEPIECE = -1;
		}

		if (bc > 0) {
			idxOfFirstBLUECASTLEPIECE = lastIdx;
			lastIdx += numRearrangements(8 - i, numBlanks, bb, rb, bs, rs, bc - 1, rc, s, l, c);
		} else {
			idxOfFirstBLUECASTLEPIECE = -1;
		}

		if (rc > 0) {
			idxOfFirstREDCASTLEPIECE = lastIdx;
			lastIdx += numRearrangements(8 - i, numBlanks, bb, rb, bs, rs, bc, rc - 1, s, l, c);
		} else {
			idxOfFirstREDCASTLEPIECE = -1;
		}

		if (tierposition >= idxOfFirstREDCASTLEPIECE) {
			board[i] = REDCASTLEPIECE;
			rc--;
			tierposition -= idxOfFirstREDCASTLEPIECE;
		} else if (tierposition >= idxOfFirstBLUECASTLEPIECE) {
			board[i] = BLUECASTLEPIECE;
			bc--;
			tierposition -= idxOfFirstBLUECASTLEPIECE;
		} else if (tierposition >= idxOfFirstCASTLEPIECE) {
			board[i] = CASTLEPIECE;
			c--;
			tierposition -= idxOfFirstCASTLEPIECE;
		} else if (tierposition >= idxOfFirstREDSMALLPIECE) {
			board[i] = REDSMALLPIECE;
			rs--;
			tierposition -= idxOfFirstREDSMALLPIECE;
		} else if (tierposition >= idxOfFirstBLUESMALLPIECE) {
			board[i] = BLUESMALLPIECE;
			bs--;
			tierposition -= idxOfFirstBLUESMALLPIECE;
		} else if (tierposition >= idxOfFirstLARGEPIECE) {
			board[i] = LARGEPIECE;
			l--;
			tierposition -= idxOfFirstLARGEPIECE;
		} else if (tierposition >= idxOfFirstSMALLPIECE) {
			board[i] = SMALLPIECE;
			s--;
			tierposition -= idxOfFirstSMALLPIECE;
		} else if (tierposition >= idxOfFirstREDBUCKETPIECE) {
			board[i] = REDBUCKETPIECE;
			rb--;
			tierposition -= idxOfFirstREDBUCKETPIECE;
		} else if (tierposition >= idxOfFirstBLUEBUCKETPIECE) {
			board[i] = BLUEBUCKETPIECE;
			bb--;
			tierposition -= idxOfFirstBLUEBUCKETPIECE;
		} else {
			board[i] = BLANKPIECE;
			numBlanks--;
		}
	}

	return board;
}

TIERPOSITION numRearrangements(int length, int numBlanks, int bb, int rb, int bs, int rs, int bc, int rc, int s, int l, int c) {
	if (numBlanks < 0 || bb < 0 || rb < 0 || bs < 0 || rs < 0 || bc < 0 || rc < 0 || s < 0 || l < 0 || c < 0) return 0;
	if (length <= 1) return 1;
	return fact(length) / (fact(numBlanks) * fact(bb) * fact(rb) * fact(bs) * fact(rs) * fact(bc) * fact(rc) * fact(s) * fact(l) * fact(c));
}

void hashBoard(char *board, char turn, int disallowedMove, TIER *tier, TIERPOSITION *tierposition) {
	int bb, rb, bs, rs, bc, rc, s, l, c;
	countPiecesOnBoard(board, &bb, &rb, &bs, &rs, &bc, &rc, &s, &l, &c);
	(*tier) = hashTier(bb, rb, bs, rs, bc, rc, s, l, c);
	(*tierposition) = 0;
	int numBlanks = 9 - bb - rb - bs - rs - bc - rc - s - l - c;
	for (int i = 0; i < 9; i++) {
		switch(board[i]) {
			case REDCASTLEPIECE:
				(*tierposition) += numRearrangements(8 - i, numBlanks, bb, rb, bs, rs, bc - 1, rc, s, l, c);
  				// fall through
			case BLUECASTLEPIECE:
				(*tierposition) += numRearrangements(8 - i, numBlanks, bb, rb, bs, rs, bc, rc, s, l, c - 1);
  				// fall through
			case CASTLEPIECE:
				(*tierposition) += numRearrangements(8 - i, numBlanks, bb, rb, bs, rs - 1, bc, rc, s, l, c);
  				// fall through
			case REDSMALLPIECE:
				(*tierposition) += numRearrangements(8 - i, numBlanks, bb, rb, bs - 1, rs, bc, rc, s, l, c);
  				// fall through
			case BLUESMALLPIECE:
				(*tierposition) += numRearrangements(8 - i, numBlanks, bb, rb, bs, rs, bc, rc, s, l - 1, c);
  				// fall through
			case LARGEPIECE:
				(*tierposition) += numRearrangements(8 - i, numBlanks, bb, rb, bs, rs, bc, rc, s - 1, l, c);
  				// fall through
			case SMALLPIECE:
				(*tierposition) += numRearrangements(8 - i, numBlanks, bb, rb - 1, bs, rs, bc, rc, s, l, c);
  				// fall through
			case REDBUCKETPIECE:
				(*tierposition) += numRearrangements(8 - i, numBlanks, bb - 1, rb, bs, rs, bc, rc, s, l, c);
  				// fall through
			case BLUEBUCKETPIECE:
				(*tierposition) += numRearrangements(8 - i, numBlanks - 1, bb, rb, bs, rs, bc, rc, s, l, c);
  				// fall through
			default:
				break; 
		}
		switch(board[i]) {
			case REDCASTLEPIECE:
				rc--;
				break;
			case BLUECASTLEPIECE:
				bc--;
				break;
			case CASTLEPIECE:
				c--;
				break;
			case REDSMALLPIECE:
				rs--;
				break;
			case BLUESMALLPIECE:
				bs--;
				break;
			case LARGEPIECE:
				l--;
				break;
			case SMALLPIECE:
				s--;
				break;
			case REDBUCKETPIECE:
				rb--;
				break;
			case BLUEBUCKETPIECE:
				bb--;
				break;
			default:
				numBlanks--;
				break; 
		}
	}

	(*tierposition) *= 41;
	(*tierposition) += ((turn == RED) ? (gNumberOfTierPositions((*tier)) >> 1) : 0);
	(*tierposition) += disallowedMove;
}

char* unhashPosition(POSITION position, char *turn, int *disallowedMove, int *blueLeft, int *redLeft, int *smallLeft, int *largeLeft) {
	if (gHashWindowInitialized) {
		TIER tier; TIERPOSITION tierposition;
		gUnhashToTierPosition(position, &tierposition, &tier);
		//generic_hash_context_switch(tier);
		char *board = unhashToBoard(tier, tierposition, turn, disallowedMove, blueLeft, redLeft, smallLeft, largeLeft);
		return board;
	} else { // Not supported.
		return NULL;
	}
}

POSITION hashPosition(char* board, char turn, int disallowedMove) {
	if (gHashWindowInitialized) {
		TIER tier; TIERPOSITION tierposition;
		hashBoard(board, turn, disallowedMove, &tier, &tierposition);
		//generic_hash_context_switch(tier);
		POSITION position = gHashToWindowPosition(tierposition, tier);
		return position;
	} else { // Not supported.
		return 0;
	}
}

void countPiecesOnBoard(char *board, int *bb, int *rb, int *bs, int *rs, int *bc, int *rc, int *s, int *l, int *c) {
	*bb = *rb = *bs = *rs = *bc = *rc = *s = *l = *c = 0;
	for (int i = 0; i < 9; i++) {
		switch(board[i]) {
			case BLUEBUCKETPIECE:
				(*bb)++;
				break;
			case REDBUCKETPIECE:
				(*rb)++;
				break;
			case BLUESMALLPIECE:
				(*bs)++;
				break;
			case REDSMALLPIECE:
				(*rs)++;
				break;
			case BLUECASTLEPIECE:
				(*bc)++;
				break;
			case REDCASTLEPIECE:
				(*rc)++;
				break;
			case SMALLPIECE:
				(*s)++;
				break;
			case LARGEPIECE:
				(*l)++;
				break;
			case CASTLEPIECE:
				(*c)++;
				break;
			default:
				break;
		}
	}
}

void unhashTier(TIER tier, int *bb, int *rb, int *bs, int *rs, int *bc, int *rc, int *s, int *l, int *c) {
	int mask = 0b1111;
	*bb = tier & mask;
	tier >>= 4;
	*rb = tier & mask;
	tier >>= 4;
	*bs = tier & mask;
	tier >>= 4;
	*rs = tier & mask;
	tier >>= 4;
	*bc = tier & mask;
	tier >>= 4;
	*rc = tier & mask;
	tier >>= 4;
	*s = tier & mask;
	tier >>= 4;
	*l = tier & mask;
	tier >>= 4;
	*c = tier & mask;
}

TIER hashTier(TIER bb, TIER rb, TIER bs, TIER rs, TIER bc, TIER rc, TIER s, TIER l, TIER c) {
	return bb | (rb << 4) | (bs << 8) | (rs << 12) | (bc << 16) | (rc << 20) | (s << 24) | (l << 28) | (c << 32);
}

TIERLIST* gTierChildren(TIER tier) {
	int bb, rb, bs, rs, bc, rc, s, l, c;
	unhashTier(tier, &bb, &rb, &bs, &rs, &bc, &rc, &s, &l, &c);
	int total = bb + rb + bs + rs + bc + rc + s + l + c;

	TIERLIST* list = NULL;
	if (bc == 2 || rc == 2) return list; // Primitive tiers contain two castles of the same color
	if (tier != gInitialTier) list = CreateTierlistNode(tier, list);
	
	// Placement
	if (total < 9) {
		if (bb + bs + bc < 2) list = CreateTierlistNode(hashTier(bb + 1, rb, bs, rs, bc, rc, s, l, c), list);
		if (rb + rs + rc < 2) list = CreateTierlistNode(hashTier(bb, rb + 1, bs, rs, bc, rc, s, l, c), list); // If there are red buckets to place
		if (s + c + rc + bc + rs + bs < 4) list = CreateTierlistNode(hashTier(bb, rb, bs, rs, bc, rc, s + 1, l, c), list); // If there are smalls to place
		if (l + c + rc + bc < 4) list = CreateTierlistNode(hashTier(bb, rb, bs, rs, bc, rc, s, l + 1, c), list); // If there are larges to place
	}

	// Sliding
	if (bb > 0 && s > 0) list = CreateTierlistNode(hashTier(bb - 1, rb, bs + 1, rs, bc, rc, s - 1, l, c), list); // Blue + Small = BlueSmall
	if (rb > 0 && s > 0) list = CreateTierlistNode(hashTier(bb, rb - 1, bs, rs + 1, bc, rc, s - 1, l, c), list); // Red + Small = RedSmall
	if (bs > 0 && l > 0) list = CreateTierlistNode(hashTier(bb, rb, bs - 1, rs, bc + 1, rc, s, l - 1, c), list); // BlueSmall + Large = BlueCastle
	if (rs > 0 && l > 0) list = CreateTierlistNode(hashTier(bb, rb, bs, rs - 1, bc, rc + 1, s, l - 1, c), list); // RedSmall + Large = RedCastle
	if (s > 0 && l > 0) list = CreateTierlistNode(hashTier(bb, rb, bs, rs, bc, rc, s - 1, l - 1, c + 1), list); // Small + Large = Castle
	if (bb > 0 && c > 0) list = CreateTierlistNode(hashTier(bb - 1, rb, bs, rs, bc + 1, rc, s, l, c - 1), list); // Blue + Castle = BlueCastle
	if (rb > 0 && c > 0) list = CreateTierlistNode(hashTier(bb, rb - 1, bs, rs, bc, rc + 1, s, l, c - 1), list); // Red + Castle = RedCastle
	return list;
}

BOOLEAN isLegal(POSITION position) {
	char turn;
	int disallowedMove, blueLeft, redLeft, smallLeft, largeLeft;
	char *board = unhashPosition(position, &turn, &disallowedMove, &blueLeft, &redLeft, &smallLeft, &largeLeft);

	if (disallowedMove == 0) {
		SafeFree(board);
		return TRUE;
	}

	int prevTo = idsToMoves[0][disallowedMove]; // They are switched
	int prevFrom = idsToMoves[1][disallowedMove];

	if (board[prevFrom] != BLANKPIECE || board[prevTo] == BLANKPIECE) {
		SafeFree(board);
		return FALSE;
	}
 
	if (turn == BLUE && (board[prevTo] == BLUEBUCKETPIECE || board[prevTo] == BLUESMALLPIECE || board[prevTo] == BLUECASTLEPIECE)) {
		SafeFree(board);
		return FALSE;
	} else if (turn == RED && (board[prevTo] == REDBUCKETPIECE || board[prevTo] == REDSMALLPIECE || board[prevTo] == REDCASTLEPIECE)) {
		SafeFree(board);
		return FALSE;
	}

	SafeFree(board);
	return TRUE;
}

TIERPOSITION gNumberOfTierPositions(TIER tier) {
	int bb, rb, bs, rs, bc, rc, s, l, c;
	unhashTier(tier, &bb, &rb, &bs, &rs, &bc, &rc, &s, &l, &c);
	int numBlanks = 9 - bb - rb - bs - rs - bc - rc - s - l - c;
	// 9! * 41 (Upper bound on # of possible disallowedMoves) * 2 (to account for turn) = 29756160 
	return 29756160 / (fact(numBlanks) * fact(bb) * fact(rb) * fact(bs) * fact(rs) * fact(bc) * fact(rc) * fact(s) * fact(l) * fact(c));
}

STRING TierToString(TIER tier) {
	int bb, rb, bs, rs, bc, rc, s, l, c;
	unhashTier(tier, &bb, &rb, &bs, &rs, &bc, &rc, &s, &l, &c);
	STRING str = (STRING) SafeMalloc(sizeof(char) * 100);
	sprintf(str, "BB: %d, RB: %d, BS: %d, RS: %d, BC: %d, RC: %d, S: %d, L: %d, C: %d", bb, rb, bs, rs, bc, rc, s, l, c);
	return str;
}

void unhashMove(MOVE move, char *piece, int *from, int *to, BOOLEAN *p2Turn) {
	*to = move & 0b1111;
	move >>= 4;
	*from = move & 0b1111;
	move >>= 4;
	switch (move & 0b1111) {
		case 1:
			*piece = REDBUCKETPIECE;
			break;
		case 2:
			*piece = SMALLPIECE;
			break;
		case 3:
			*piece = LARGEPIECE;
			break;
		default:
			*piece = BLUEBUCKETPIECE;
			break;
	}
	move >>= 4;
	*p2Turn = move ? TRUE : FALSE;
}

MOVE hashMove(char piece, int from, int to, BOOLEAN p2Turn) {
	MOVE theHash = 0;
	theHash |= to;
	theHash |= from << 4;
	switch (piece) {
		case REDBUCKETPIECE:
			theHash |= 1 << 8;
			break;
		case SMALLPIECE:
			theHash |= 2 << 8;
			break;
		case LARGEPIECE:
			theHash |= 3 << 8;
			break;
		default:
			break;
	}
	if (p2Turn) {
		theHash |= 1 << 12;
	}
	return theHash;
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
	char GetMyChar();

	do {
		printf("\n\t----- Game-specific options for %s -----\n\n\n", kGameName);

		if (tyingRule == FALSE && ifNoLegalMoves == undecided && isMisere == FALSE) {
			printf("\tCurrently using default (Variant 0) rules.\n");
		} else {
			printf("\tCurrently using rules for Variant %d.\n", getOption());
		}

		printf("\n\ts)\tIf no legal moves: change from %s to %s.\n", (ifNoLegalMoves == undecided) ? "PASS TURN" : "IMMEDIATE LOSS", (ifNoLegalMoves == undecided) ? "IMMEDIATE LOSS" : "PASS TURN");
		printf("\n\tt)\tTwo-tower tying rule: change from %s to %s.\n", (tyingRule) ? "ON" : "OFF", (tyingRule) ? "OFF" : "ON");
		printf("\n\tm)\tChange from %sMISERE to %sMISERE.\n", (isMisere) ? "" : "NON-", (isMisere) ? "NON-" : "");
		printf("\n\tb)\tBack = Return to previous activity.\n");
		printf("\n\nSelect an option: ");

		switch(GetMyChar()) {
		case 'S': case 's':
			ifNoLegalMoves = (ifNoLegalMoves == undecided) ? lose : undecided;
			break;
		case 'T': case 't':
			tyingRule = !tyingRule;
			break;
		case 'M': case 'm':
			isMisere = !isMisere;
			break;
		case 'B': case 'b': case 'Q': case 'q':
			return;
		default:
			printf("\nSorry, I don't know that option. Try another.\n");
			HitAnyKeyToContinue();
			break;
		}
	} while(TRUE);
}

/************************************************************************
**
** NAME:        NumberOfOptions
**
** DESCRIPTION: Calculates and returns the number of variants
**              your game supports.
**
** OUTPUTS:     int : Number of Game Variants
**
************************************************************************/

int NumberOfOptions() {
	return 8;
}


/************************************************************************
**
** NAME:        getOption
**
** DESCRIPTION: A hash function that returns a number corresponding
**              to the current variant of the game.
**              Each set of variants needs to have a different number.
**
** OUTPUTS:     int : the number representation of the options.
**
************************************************************************/

int getOption() {
	return (isMisere << 2) | (tyingRule << 1) | (ifNoLegalMoves == lose);
}

/************************************************************************
**
** NAME:        setOption
**
** DESCRIPTION: The corresponding unhash function for game variants.
**              Unhashes option and sets the necessary variants.
**
** INPUT:       int option : the number representation of the options.
**
************************************************************************/

void setOption(int option) {
	ifNoLegalMoves = (option & 0b1) ? lose : undecided;
	tyingRule = (option & 0b10) ? TRUE : FALSE;
	isMisere = (option & 0b100) ? TRUE : FALSE;
}

/************************************************************************
**
** NAME: SetTclCGameSpecificOptions
**
** DESCRIPTION: Set the C game-specific options (called from Tcl)
** Ignore if you don't care about Tcl for now.
**
************************************************************************/
void SetTclCGameSpecificOptions(int theOptions[])
{
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
	int disallowedMove, blueLeft, redLeft, smallLeft, largeLeft;
	char *board = unhashPosition(position, &turn, &disallowedMove, &blueLeft, &redLeft, &smallLeft, &largeLeft);

	if (move == NULLMOVE) {
		position = hashPosition(board, (turn == BLUE) ? RED : BLUE, 0);
		SafeFree(board);
		return position;
	}
	
	char piece;
	int from, to;
	BOOLEAN p2Turn;
	unhashMove(move, &piece, &from, &to, &p2Turn);
	if (from == to) { // Placement
		board[to] = piece;
		switch (piece) {
			case BLUEBUCKETPIECE:
				blueLeft--;
				break;
			case REDBUCKETPIECE:
				redLeft--;
				break;
			case SMALLPIECE:
				smallLeft--;
				break;
			case LARGEPIECE:
				largeLeft--;
				break;
			default:
				break;
		}
		disallowedMove = 0;
	} else { // SLIDING
		switch (board[to]) {
			case BLANKPIECE:
				if (board[from] == SMALLPIECE || board[from] == LARGEPIECE || board[from] == CASTLEPIECE) {
					disallowedMove = movesToIds[to][from];  // A reversal of this slide is forbidden.
				} else {
					disallowedMove = 0;
				}
				board[to] = board[from];
				break;
			case SMALLPIECE:
				if (board[from] == BLUEBUCKETPIECE) {
					board[to] = BLUESMALLPIECE;
				} else {
					board[to] = REDSMALLPIECE;
				}
				disallowedMove = 0;
				break;
			case LARGEPIECE:
				if (board[from] == SMALLPIECE) {
					board[to] = CASTLEPIECE;
				} else if (board[from] == BLUESMALLPIECE) {
					board[to] = BLUECASTLEPIECE;
				} else {
					board[to] = REDCASTLEPIECE;
				}
				disallowedMove = 0;
				break;
			case CASTLEPIECE:
				if (board[from] == BLUEBUCKETPIECE) {
					board[to] = BLUECASTLEPIECE;
				} else {
					board[to] = REDCASTLEPIECE;
				}
				disallowedMove = 0;
				break;
			default:
				break;
		}
		board[from] = BLANKPIECE;
	}

	position = hashPosition(board, (turn == BLUE) ? RED : BLUE, disallowedMove);

	SafeFree(board);

	return position;
}


/************************************************************************
**
** NAME: GetInitialPosition
**
** DESCRIPTION: Ask the user for an initial position for testing. Store
** it in the space pointed to by initialPosition;
**
** OUTPUTS: POSITION initialPosition : The position to fill.
**
************************************************************************/

POSITION GetInitialPosition() {
	char board[9] = {BLANKPIECE, BLANKPIECE, BLANKPIECE, BLANKPIECE, BLANKPIECE, BLANKPIECE, BLANKPIECE, BLANKPIECE, BLANKPIECE};
	return hashPosition(board, BLUE, 0);
}

/************************************************************************
**
** NAME: PrintComputersMove
**
** DESCRIPTION: Nicely format the computers move.
**
** INPUTS: MOVE computersMove : The computer's move.
** STRING computersName : The computer's name.
**
************************************************************************/

void PrintComputersMove(MOVE computersMove, STRING computersName) {
	STRING str = MoveToString(computersMove);
	printf("%8s's move: %s\n", computersName, str);
	SafeFree(str);
}

VALUE Primitive(POSITION position) {
	char turn;
	int disallowedMove, blueLeft, redLeft, smallLeft, largeLeft;
	char *board = unhashPosition(position, &turn, &disallowedMove, &blueLeft, &redLeft, &smallLeft, &largeLeft);
	int bb, rb, bs, rs, bc, rc, s, l, c;
	countPiecesOnBoard(board, &bb, &rb, &bs, &rs, &bc, &rc, &s, &l, &c);
	
	if (bc == 2 || rc == 2) {
		if (!tyingRule) {
			SafeFree(board);
			return (isMisere) ? win : lose;
		} else {
			if (bc == 2 && rc == 1) {
				for (int i = 0; i < 9; i++) {
					if (board[i] == REDBUCKETPIECE) { // Check if red bucket piece is next to castle piece.
						for (int j = 0; j < numAdjacencies[i]; j++) {
							if (board[adjacencyMatrix[i][j]] == CASTLEPIECE) {
								SafeFree(board);
								return tie;
							}
						}
					} else if (board[i] == REDSMALLPIECE) { // Check if red small piece is next to large piece.
						for (int j = 0; j < numAdjacencies[i]; j++) {
							if (board[adjacencyMatrix[i][j]] == LARGEPIECE) {
								SafeFree(board);
								return tie;
							}
						}
					}
				}
				SafeFree(board);
				return (isMisere) ? win : lose;

			}
			SafeFree(board);
			return (isMisere) ? win : lose;
		}
	}
	SafeFree(board);
	if (ifNoLegalMoves == lose) {
		MOVELIST *moveList = GenerateMoves(position);
		if (moveList->move == NULLMOVE) {
			FreeMoveList(moveList);
			return (isMisere) ? win : lose;
		}
		FreeMoveList(moveList);
	}
	return undecided;
}

/************************************************************************
**
** NAME: PrintPosition
**
**     *-------*-------*-------*
**     |   B   |       |       |  Turn: Blue
**     |  /_\  |       |       |  Disallowed Move: [4-3]
**     | /___\ |  /_\  |   R   |
**     *-------+-------+-------*  Remaining Pieces to Place:
**     |       |       |       |  Blue Bucket (0)
**     |       |       |       |  Red Bucket (0)
**     |       | /___\ | /___\ |  Small Sand Pile (0)
**     *-------+-------+-------*  Large Sand Pile (0)
**     |       |       |       |
**     |  /_\  |   R   |       |  <Prediction>
**     | /___\ |  /_\  |   B   |
**     *-------*-------*-------*
**
************************************************************************/

char* PrintHelper(char piece, int level) {
	if (level == 0) {
		switch (piece) {
			case BLUECASTLEPIECE:
			case REDCASTLEPIECE:
			case CASTLEPIECE:
			case LARGEPIECE:
				return LARGEDISPLAY;
			case BLUESMALLPIECE:
			case REDSMALLPIECE:
			case SMALLPIECE:
				return SMALLDISPLAY;
			case BLUEBUCKETPIECE:
				return BLUEDISPLAY;
			case REDBUCKETPIECE:
				return REDDISPLAY;
			default:
				return BLANKDISPLAY;
		}
	} else if (level == 1) {
		switch (piece) {
			case BLUECASTLEPIECE:
			case REDCASTLEPIECE:
			case CASTLEPIECE:
				return SMALLDISPLAY;
			case BLUESMALLPIECE:
				return BLUEDISPLAY;
			case REDSMALLPIECE:
				return REDDISPLAY;
			default:
				return BLANKDISPLAY;
		}
	} else {
		if (piece == BLUECASTLEPIECE) return BLUEDISPLAY;
		if (piece == REDCASTLEPIECE) return REDDISPLAY;
		return BLANKDISPLAY;
	}
}

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn) {
	char turn;
	int disallowedMove, blueLeft, redLeft, smallLeft, largeLeft;
	char *board = unhashPosition(position, &turn, &disallowedMove, &blueLeft, &redLeft, &smallLeft, &largeLeft);
	char *strDisallowedMove = DisallowedMoveToString(disallowedMove);

	printf("\n");
	printf("        *-------*-------*-------*\n");
	printf("        | %s | %s | %s |  Turn: %s\n", PrintHelper(board[0], 2), PrintHelper(board[1], 2), PrintHelper(board[2], 2), (turn == BLUE) ? "Blue" : "Red");
	printf("        | %s | %s | %s |  Disallowed Move: %s\n", PrintHelper(board[0], 1), PrintHelper(board[1], 1), PrintHelper(board[2], 1), strDisallowedMove);
	printf("        | %s | %s | %s |\n", PrintHelper(board[0], 0), PrintHelper(board[1], 0), PrintHelper(board[2], 0));
	printf("        *-------*-------*-------*  Remaining Pieces to Place:\n");
	printf("        | %s | %s | %s |  Blue Bucket (%d)\n", PrintHelper(board[3], 2), PrintHelper(board[4], 2), PrintHelper(board[5], 2), blueLeft);
	printf("        | %s | %s | %s |  Red Bucket (%d)\n", PrintHelper(board[3], 1), PrintHelper(board[4], 1), PrintHelper(board[5], 1), redLeft);
	printf("        | %s | %s | %s |  Small Sand Pile (%d)\n", PrintHelper(board[3], 0), PrintHelper(board[4], 0), PrintHelper(board[5], 0), smallLeft);
	printf("        *-------*-------*-------*  Large Sand Pile (%d)\n", largeLeft);
	printf("        | %s | %s | %s |\n", PrintHelper(board[6], 2), PrintHelper(board[7], 2), PrintHelper(board[8], 2));
	printf("        | %s | %s | %s |  %s\n", PrintHelper(board[6], 1), PrintHelper(board[7], 1), PrintHelper(board[8], 1), GetPrediction(position, playerName, usersTurn));
	printf("        | %s | %s | %s |\n", PrintHelper(board[6], 0), PrintHelper(board[7], 0), PrintHelper(board[8], 0));
	printf("        *-------*-------*-------*\n\n");

	SafeFree(board);
	SafeFree(strDisallowedMove);
}

MOVELIST *GenerateMoves(POSITION position) {
	MOVELIST *moveList = NULL;

	char turn;
	int disallowedMove, blueLeft, redLeft, smallLeft, largeLeft;
	char *board = unhashPosition(position, &turn, &disallowedMove, &blueLeft, &redLeft, &smallLeft, &largeLeft);

	for (int to = 0; to < 9; to++) {
		if (board[to] == BLANKPIECE) {
	
			// PLACEMENT //
			if (blueLeft > 0 && turn == BLUE)
				moveList = CreateMovelistNode(hashMove(BLUEBUCKETPIECE, to, to, turn == RED), moveList);
			else if (redLeft > 0 && turn == RED)
				moveList = CreateMovelistNode(hashMove(REDBUCKETPIECE, to, to, turn == RED), moveList);

			if (smallLeft > 0) moveList = CreateMovelistNode(hashMove(SMALLPIECE, to, to, FALSE), moveList);
      		if (largeLeft > 0) moveList = CreateMovelistNode(hashMove(LARGEPIECE, to, to, FALSE), moveList);

			// SLIDE TO BLANK //
			for (int j = 0; j < numAdjacencies[to]; j++) {
				int from = adjacencyMatrix[to][j];
				if (!(from == idsToMoves[0][disallowedMove] && to == idsToMoves[1][disallowedMove])) { // Don't undo previous person's slide
					if ((turn == BLUE && board[from] != BLANKPIECE && board[from] != REDBUCKETPIECE && board[from] != REDSMALLPIECE && board[from] != REDCASTLEPIECE) || (turn == RED && board[from] != BLANKPIECE && board[from] != BLUEBUCKETPIECE && board[from] != BLUESMALLPIECE && board[from] != BLUECASTLEPIECE))
						moveList = CreateMovelistNode(hashMove(BLUEBUCKETPIECE, from, to, FALSE), moveList);
				}
			}
		} else if (board[to] == SMALLPIECE || board[to] == CASTLEPIECE) { // SLIDE ONTO SMALL PIECE OR CASTLE PIECE //
			for (int j = 0; j < numAdjacencies[to]; j++) {
				int from = adjacencyMatrix[to][j];
				if ((turn == BLUE && board[from] == BLUEBUCKETPIECE) || (turn == RED && board[from] == REDBUCKETPIECE))
					moveList = CreateMovelistNode(hashMove(BLUEBUCKETPIECE, from, to, FALSE), moveList);
			}
		} else if (board[to] == LARGEPIECE) { // SLIDE ONTO LARGE PIECE //
			for (int j = 0; j < numAdjacencies[to]; j++) {
				int from = adjacencyMatrix[to][j];
				if (board[from] == SMALLPIECE || (turn == BLUE && board[from] == BLUESMALLPIECE) || (turn == RED && board[from] == REDSMALLPIECE))
					moveList = CreateMovelistNode(hashMove(BLUEBUCKETPIECE, from, to, FALSE), moveList);
			}
		}
	}

	SafeFree(board);

	// Add passing move if no legal moves.
	if (moveList == NULL) {
		moveList = CreateMovelistNode(NULLMOVE, moveList);
	}

	return moveList;
}

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

POSITION GetCanonicalPosition(POSITION position) {
	char turn;
	int disallowedMove, blueLeft, redLeft, smallLeft, largeLeft;
	char *originalBoard = unhashPosition(position, &turn, &disallowedMove, &blueLeft, &redLeft, &smallLeft, &largeLeft);

	char canonBoard[9];
	int canonDisallowedMove = disallowedMove;
    POSITION canonPos = position;
    int bestSymmetryNum = 0;

    for (int symmetryNum = 1; symmetryNum < 8; symmetryNum++)
        for (int i = 0; i < 9; i++) {
            char pieceInSymmetry = originalBoard[symmetriesToUse[symmetryNum][i]];
            char pieceInBest = originalBoard[symmetriesToUse[bestSymmetryNum][i]];
            if (pieceInSymmetry > pieceInBest) {
                bestSymmetryNum = symmetryNum;
                break;
            }
        };

    if (bestSymmetryNum == 0) {
		SafeFree(originalBoard);
        return position;
	}
    
    for (int i = 0; i < 9; i++) { // Transform the rest of the board.
        canonBoard[i] = originalBoard[symmetriesToUse[bestSymmetryNum][i]];
	}
	
	if (disallowedMove != 0) { // Transform disallowed move.
		canonDisallowedMove = movesToIds[symmetriesToUse[bestSymmetryNum][idsToMoves[0][disallowedMove]]][symmetriesToUse[bestSymmetryNum][idsToMoves[1][disallowedMove]]];
	}

    canonPos = hashPosition(canonBoard, turn, canonDisallowedMove);
	
	SafeFree(originalBoard);

    return canonPos;
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
** MOVE theMove : The move to fill with user's move.
** STRING playerName : The name of the player whose turn it is
**
** OUTPUTS: USERINPUT : Oneof( Undo, Abort, Continue )
**
** CALLS: ValidMove(MOVE, POSITION)
** BOOLEAN PrintPossibleMoves(POSITION) ...Always True!
**
************************************************************************/

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
	return (input[0] >= 49 && input[0] <= 57 && input[1] == '-' && input[2] >= 49 && input[2] <= 57 && input[3] == '\0') || // Movement
			((input[0] == 'B' || input[0] == 'R' || input[0] == 'S' || input[0] == 'L' || input[0] == 'b' || input[0] == 'r' || input[0] == 's' || input[0] == 'l') && input[1] >= 49 && input[1] <= 57 && input[2] == '\0') || // Placement
			strcmp(input, "None") == 0; // Pass Turn
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
	char piece = BLUEBUCKETPIECE;
	if (input[0] >= 49 && input[0] <= 57) {// Movement
		return hashMove(piece, input[0] - '1', input[2] - '1', FALSE);
	} else if (input[0] == 'N' || input[0] == 'n') {
		return NULLMOVE;
	} else {
		switch(input[0]) {
			case 'B': case 'b':
				piece = BLUEBUCKETPIECE;
				break;
			case 'R': case 'r':
				piece = REDBUCKETPIECE;
				break;
			case 'S': case 's':
				piece = SMALLPIECE;
				break;
			case 'L': case 'l':
				piece = LARGEPIECE;
				break;
			default:
				break;
		}
		return hashMove(piece, input[1] - '1', input[1] - '1', FALSE);
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

void PrintMove(MOVE move) {
	STRING str = MoveToString(move);
	printf("%s", str);
	SafeFree(str);
}

STRING DisallowedMoveToString(int disallowedMove) {
	STRING moveString = (STRING) SafeMalloc(7);
	if (disallowedMove == 0) {
		sprintf(moveString, "None");
	} else {
		sprintf(moveString, "%d-%d", idsToMoves[0][disallowedMove] + 1, idsToMoves[1][disallowedMove] + 1);
	}
	return moveString;
}

/************************************************************************
**
** NAME: MoveToString
**
** DESCRIPTION: Returns the move as a STRING
**
** INPUTS: MOVE *move : The move to put into a string.
**
************************************************************************/

STRING MoveToString(MOVE move) {
	char piece;
	int from, to;
	BOOLEAN p2Turn;
	if (move == NULLMOVE) {
		STRING moveString = (STRING) SafeMalloc(7);
		sprintf(moveString, "None");
		return moveString;
	}
	unhashMove(move, &piece, &from, &to, &p2Turn);
	if (from == to) { // Placement
		STRING moveString = (STRING) SafeMalloc(5);
		sprintf(moveString, "%c%d", piece, from + 1);
		return moveString;
	} else { // Sliding
		STRING moveString = (STRING) SafeMalloc(6);
		sprintf(moveString, "%d-%d", from  +1, to + 1);
		return moveString;
	}
}

// 27 bucket centers
// 18 small piece centers
// 9 large piece centers
// 1 disallowedFromIdx [54]
// 1 disallowedToIdx [55]
// 20*2 arrow centers [56-95]
// 27 placepiece move button centers [96-122]
// 1 pass turn center [123]

// BUCKET (3) top mid bottom SMALL mid bottom (2) LARGE bottom (1) 
POSITION InteractStringToPosition(STRING str) {
	char turn = (str[2] == 'A') ? BLUE : RED;
	char *entityString = str + 8;
	char board[9];
	int i, j;
	for (i = 0, j = 0; i < 9; i++, j += 6) {
		if (entityString[j] == BLUEBUCKETPIECE) {
			board[i] = BLUECASTLEPIECE;
		} else if (entityString[j] == REDBUCKETPIECE) {
			board[i] = REDCASTLEPIECE;
		} else if (entityString[j + 1] == BLUEBUCKETPIECE) {
			board[i] = BLUESMALLPIECE;
		} else if (entityString[j + 1] == REDBUCKETPIECE) {
			board[i] = REDSMALLPIECE;
		} else if (entityString[j + 2] == BLUEBUCKETPIECE) {
			board[i] = BLUEBUCKETPIECE;
		} else if (entityString[j + 2] == REDBUCKETPIECE) {
			board[i] = REDBUCKETPIECE;
		} else if (entityString[j + 3] == SMALLPIECE) {
			board[i] = CASTLEPIECE;
		} else if (entityString[j + 4] == SMALLPIECE) {
			board[i] = SMALLPIECE;
		} else if (entityString[j + 5] == LARGEPIECE) {
			board[i] = LARGEPIECE;
		} else {
			board[i] = BLANKPIECE;
		}
	}
	int disallowedMove = (entityString[54] == '-') ? 0 : movesToIds[entityString[54] - '1'][entityString[55] - '1'];

	TIER tier;
	TIERPOSITION tierposition;
	hashBoard(board, turn, disallowedMove, &tier, &tierposition);
	gInitializeHashWindow(tier, FALSE);
	return tierposition;
}

STRING InteractPositionToString(POSITION position) {
	char entityString[57];
	memset(entityString, '-', 57 * sizeof(char));

	char turn;
	int disallowedMove, blueLeft, redLeft, smallLeft, largeLeft;
	char *board = unhashPosition(position, &turn, &disallowedMove, &blueLeft, &redLeft, &smallLeft, &largeLeft);
	int i, j;

	for (i = 0, j = 0; i < 9; i++, j += 6) {
		if (board[i] == BLUECASTLEPIECE) {
			entityString[j] = BLUEBUCKETPIECE;
			entityString[j + 3] = SMALLPIECE;
			entityString[j + 5] = LARGEPIECE;
		} else if (board[i] == REDCASTLEPIECE) {
			entityString[j] = REDBUCKETPIECE;
			entityString[j + 3] = SMALLPIECE;
			entityString[j + 5] = LARGEPIECE;
		} else if (board[i] == BLUESMALLPIECE) {
			entityString[j + 1] = BLUEBUCKETPIECE;
			entityString[j + 4] = SMALLPIECE;
		} else if (board[i] == REDSMALLPIECE) {
			entityString[j + 1] = REDBUCKETPIECE;
			entityString[j + 4] = SMALLPIECE;
		} else if (board[i] == BLUEBUCKETPIECE) {
			entityString[j + 2] = BLUEBUCKETPIECE;
		} else if (board[i] == REDBUCKETPIECE) {
			entityString[j + 2] = REDBUCKETPIECE;
		} else if (board[i] == CASTLEPIECE) {
			entityString[j + 3] = SMALLPIECE;
			entityString[j + 5] = LARGEPIECE;
		} else if (board[i] == SMALLPIECE) {
			entityString[j + 4] = SMALLPIECE;
		} else if (board[i] == LARGEPIECE) {
			entityString[j + 5] = LARGEPIECE;
		}
	}

	SafeFree(board);

	enum UWAPI_Turn uwapiTurn = (turn == BLUE) ? UWAPI_TURN_A : UWAPI_TURN_B;
	entityString[54] = (disallowedMove == 0) ? '-' : idsToMoves[0][disallowedMove] + '1';
	entityString[55] = (disallowedMove == 0) ? '-' : idsToMoves[1][disallowedMove] + '1';
	entityString[56] = '\0';
	return UWAPI_Board_Regular2D_MakeBoardString(uwapiTurn, 57, entityString);
}

int uwapiArrowCoords[8][9] = {
	{ 9,  0,  9,  2,  4,  9,  9,  9,  9},
	{ 9,  9,  6,  8, 10, 12,  9,  9,  9},
	{ 9,  9,  9,  9, 14, 16,  9,  9,  9},
	{ 9,  9,  9,  9, 18,  9, 20, 22,  9},
	{ 9,  9,  9,  9,  9, 24, 26, 28, 30},
	{ 9,  9,  9,  9,  9,  9,  9, 32, 34},
	{ 9,  9,  9,  9,  9,  9,  9, 36,  9},
	{ 9,  9,  9,  9,  9,  9,  9,  9, 38}
};

STRING InteractMoveToString(POSITION pos, MOVE move) {
	(void)pos;
	if (move == NULLMOVE) {
		return UWAPI_Board_Regular2D_MakeAddStringWithSound('P', 123, 'v');
	}
	
	char piece;
	int from, to;
	BOOLEAN p2Turn;
	unhashMove(move, &piece, &from, &to, &p2Turn);

	if (from == to) {// Placement
		if (piece == SMALLPIECE) {
			return UWAPI_Board_Regular2D_MakeAddStringWithSound('v', 105 + to, 'w');
		} else if (piece == LARGEPIECE) {
			return UWAPI_Board_Regular2D_MakeAddStringWithSound('w', 114 + to, 'x');
		} else {
			return UWAPI_Board_Regular2D_MakeAddStringWithSound(p2Turn ? 'u' : 't', 96 + to, 'y');
		}
	} else {
		if (to > from) {
			int d = uwapiArrowCoords[from][to] + 56;
			return UWAPI_Board_Regular2D_MakeMoveStringWithSound(d, d + 1, 'z');
		} else {
			int d = uwapiArrowCoords[to][from] + 56;
			return UWAPI_Board_Regular2D_MakeMoveStringWithSound(d + 1, d, 'z');
		}
	}
}