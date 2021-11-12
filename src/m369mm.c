/************************************************************************
**
** NAME:        m369mm.c
**
** DESCRIPTION: THREE/SIX/NINE MEN'S MORRIS
**
** AUTHOR:      Patricia Fong & Kevin Liu & Erwin A. Vedar, Wei Tu, Elmer Lee
**
** DATE:        too long to remmeber
**
** UPDATE HIST: RECORD CHANGES YOU HAVE MADE SO THAT TEAMMATES KNOW
**
** LAST CHANGE: $Id: m369mm.c,v 1.4 2008-09-29 07:33:40 noafroboy Exp $
**
**************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/

#include <stdio.h>
#include "gamesman.h"
#include "core/db.h"
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>

/*************************************************************************
**
** Game-specific constants
**
**************************************************************************/

STRING kGameName            = "369 Men's Morris";   /* The name of your game */
STRING kAuthorName          = "Patricia Fong, Kevin Liu, Erwin A. Vedar, Elmer Lee";   /* Your name(s) */
STRING kDBName              = "369mm";   /* The name to store the database under */

BOOLEAN kPartizan            = TRUE;   /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN kGameSpecificMenu    = TRUE;   /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN kTieIsPossible       = FALSE;   /* TRUE if a tie is possible. FALSE if it is impossible.*/
BOOLEAN kLoopy               = TRUE;   /* TRUE if the game tree will have cycles (a rearranger style game). FALSE if it does not.*/

BOOLEAN kDebugMenu           = TRUE;   /* TRUE only when debugging. FALSE when on release. */
BOOLEAN kDebugDetermineValue = FALSE;   /* TRUE only when debugging. FALSE when on release. */

POSITION gNumberOfPositions   =  0; /* The number of total possible positions | If you are using our hash, this is given by the hash_init() function*/
POSITION gInitialPosition     =  0; /* The initial hashed position for your starting board */
POSITION kBadPosition         = -1; /* A position that will never be used */
BOOLEAN isInteractive = FALSE;

void*    gGameSpecificTclInit = NULL;

STRING initial9mmInteractString = "R_A_8_7_9-----9s--s--s-s-s-s---sss--sss-sss--sss---s-s-s-s--s--s";
int indexMap9mmInteractString[24] = {15,18,21,23,25,27,31,32,33,36,37,38,40,41,42,45,46,47,51,53,55,57,60,63};
int turnIndex9mmInteractString = 2;
int remainingXIndex9mmInteractString = 8;
int remainingOIndex9mmInteractString = 14;
VALUE (*oldvaluegetter)(POSITION) = NULL;
REMOTENESS (*oldremotenessgetter)(POSITION) = NULL;
char validTiers[1800];

void logMsg(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    printf(fmt, args);
    va_end(args);
}

/**
 * Help strings that are pretty self-explanatory
 * Strings than span more than one line should have backslashes (\) at the end of the line.
 * These help strings should be updated and dynamically changed using
 * InitializeHelpStrings()
 **/

STRING kHelpGraphicInterface =
        "369 Men's Morris does not currently support a Graphical User Interface\n(other than beloved ASCII).";

STRING kHelpTextInterface =
        "The LEGEND shows numbers corresponding to positions on the board.  On your\nturn, use the LEGEND to enter the position your piece currently is, the position\nyour piece is moving to, and (if your move creates a mill) the position of the\npiece you wish to remove from play.  Seperate each number entered with a space\nand hit return to commit your move.  If you ever make a mistake when choosing\nyour move, you can type \"u\" and hit return to revert back to your most recent\nposition.";

STRING kHelpOnYourTurn =
        "Help strings not initialized!";

STRING kHelpStandardObjective =
        "Help strings not initialized!";

STRING kHelpReverseObjective =
        "Help strings not initialized!";

STRING kHelpTieOccursWhen =
        "Help strings not initialized!";

STRING kHelpExample =
        "Help strings not initialized!";

/*************************************************************************
**
** Variants
**
**************************************************************************/
BOOLEAN gFlying = TRUE;
int gameType = 9; // 3,6,9 men's morris
int millType = 0; // 0: can remove piece not from mill unless if only mills left. 1: can remove any piece. 2: can not remove pieces from any mill ever

/*************************************************************************
**
** #defines and structs
**
**************************************************************************/

#define MOVE_ENCODE(from, to, remove) ((from << 10) | (to << 5) | remove)
#define THREE_IN_A_ROW(board, slot1, slot2, turn) (board[slot1] == turn && board[slot2] == turn)

/*************************************************************************
**
** Global Variables
**
*************************************************************************/
int BOARDSIZE; //6mm 16   9mm 24
int minx = 2;
int maxx;  //6mm 6   9mm 9
int mino = 2;
int maxo;  //6mm 6   9mm 9
int minb;
int maxb;
int (*adjacent)[5]; // Pre-Computed Adjacency Arrays

#define BLANK '.'
#define X 'X'
#define O 'O'
#define PLAYER_ONE 1
#define PLAYER_TWO 2
int NUMX=0;
int NUMO=0;
int totalPieces = 18; //Remove when tiering

/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/

/* External */
#ifndef MEMWATCH
extern GENERIC_PTR      SafeMalloc ();
extern void             SafeFree ();
#endif

/*function prototypes*/
void InitializeGame ();
void SetupTierStuff();
TIERLIST* gTierChildren(TIER tier);
TIERPOSITION gNumberOfTierPositions (TIER tier);
STRING TierToString(TIER tier);
void InitializeHelpStrings ();

MOVELIST *GenerateMoves (POSITION position);
POSITION DoMove (POSITION position, MOVE move);

VALUE Primitive (POSITION position);
void PrintPosition (POSITION position, STRING playersName, BOOLEAN usersTurn);
void PrintComputersMove (MOVE computersMove, STRING computersName);
void PrintMove (MOVE move);
STRING MoveToString(MOVE move);
USERINPUT GetAndPrintPlayersMove (POSITION position, MOVE *move, STRING playersName);
BOOLEAN ValidTextInput (STRING input);
MOVE ConvertTextInputToMove (STRING input);
void GameSpecificMenu ();
void SetTclCGameSpecificOptions (int options[]);
POSITION GetInitialPosition ();
int NumberOfOptions ();
int getOption ();
void setOption (int option);
void DebugMenu ();
char* unhash(POSITION pos, char* turn, int* piecesLeft, int* numx, int* numo);
POSITION hash(char* board, char turn, int piecesLeft, int numx, int numo);
POSITION updatepieces(char* board,char turn,int piecesLeft,int numx, int numo, MOVE move, POSITION position);
BOOLEAN can_be_taken(POSITION position, int slot);
BOOLEAN all_mills(char *board, int slot, char turn);
int find_pieces(char *board, char piece, int *pieces);
BOOLEAN closes_mill(POSITION position, int raw_move);
BOOLEAN checkMill(char *board, int slot, char turn);
//BOOLEAN THREE_IN_A_ROW(char *board, int slot1, int slot2, char turn);
char returnTurn(POSITION pos);
char* customUnhash(POSITION pos);
void changeToSix();
void changeToNine();
void changeToThree();
POSITION GetCanonicalPosition(POSITION);
POSITION smallHash(char*, char);
char* smallUnhash(POSITION, char*);

BOOLEAN closesMillNew(char *board, char turn, int fromIdx, int toIdx);
int findLegalRemoves(char *board, char turn, int *legalRemoves);
int findAdjacentNew(char *board, int slot, int *slots);
STRING MoveToStringOld (MOVE move);
UNDOMOVELIST *GenerateUndoMovesToTier(POSITION position, TIER tier);
POSITION UndoMove(POSITION position, UNDOMOVE undoMove);
int findLegalRemovesUndo(char *board, char turn, int *legalRemoves);
STRING UndoMoveToString(UNDOMOVE move);
BOOLEAN allMillsNew(char *board, int slot, char turn);
POSITION DoMoveSafe(POSITION position, MOVE move);

STRING InteractPositionToString(POSITION pos);
POSITION InteractStringToPosition(STRING s);
MOVELIST *InteractGenerateMoves(POSITION pos);
POSITION InteractDoMove(POSITION pos, MOVE move);
VALUE InteractPrimitive(POSITION pos);
BOOLEAN isIntermediate(POSITION pos);
VALUE InteractGetValue(POSITION pos);
REMOTENESS InteractGetRemoteness(POSITION pos);	

int gSymmetryMatrix3MM[8][24] = {
    {0,1,2,3,4,5,6,7,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {6,3,0,7,4,1,8,5,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {8,7,6,5,4,3,2,1,0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {2,5,8,1,4,7,0,3,6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {2,1,0,5,4,3,8,7,6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {0,3,6,1,4,7,2,5,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {6,7,8,3,4,5,0,1,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {8,5,2,7,4,1,6,3,0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}
};

int gSymmetryMatrix6MM[16][24] = {
	{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1},
	{13,6,0,10,7,3,14,11,4,1,12,8,5,15,9,2,-1,-1,-1,-1,-1,-1,-1,-1},
	{15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,-1,-1,-1,-1,-1,-1,-1,-1},
	{2,9,15,5,8,12,1,4,11,14,3,7,10,0,6,13,-1,-1,-1,-1,-1,-1,-1,-1},
	{3,4,5,0,1,2,7,6,9,8,13,14,15,10,11,12,-1,-1,-1,-1,-1,-1,-1,-1},
	{10,7,3,13,6,0,11,14,1,4,15,9,2,12,8,5,-1,-1,-1,-1,-1,-1,-1,-1},
	{12,11,10,15,14,13,8,9,6,7,2,1,0,5,4,3,-1,-1,-1,-1,-1,-1,-1,-1},
	{5,8,12,2,9,15,4,1,14,11,0,6,13,3,7,10,-1,-1,-1,-1,-1,-1,-1,-1},
	{2,1,0,5,4,3,9,8,7,6,12,11,10,15,14,13,-1,-1,-1,-1,-1,-1,-1,-1},
	{0,6,13,3,7,10,1,4,11,14,5,8,12,2,9,15,-1,-1,-1,-1,-1,-1,-1,-1},
	{13,14,15,10,11,12,6,7,8,9,3,4,5,0,1,2,-1,-1,-1,-1,-1,-1,-1,-1},
	{15,9,2,12,8,5,14,11,4,1,10,7,3,13,6,0,-1,-1,-1,-1,-1,-1,-1,-1},
	{5,4,3,2,1,0,8,9,6,7,15,14,13,12,11,10,-1,-1,-1,-1,-1,-1,-1,-1},
	{3,7,10,0,6,13,4,1,14,11,2,9,15,5,8,12,-1,-1,-1,-1,-1,-1,-1,-1},
	{10,11,12,13,14,15,7,6,9,8,0,1,2,3,4,5,-1,-1,-1,-1,-1,-1,-1,-1},
	{12,8,5,15,9,2,11,14,1,4,13,6,0,10,7,3,-1,-1,-1,-1,-1,-1,-1,-1}
};

int gSymmetryMatrix9MM[16][24] = { 
	{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23},
	{6,7,8,3,4,5,0,1,2,11,10,9,14,13,12,21,22,23,18,19,20,15,16,17},
	{2,1,0,5,4,3,8,7,6,14,13,12,11,10,9,17,16,15,20,19,18,23,22,21},
	{8,7,6,5,4,3,2,1,0,12,13,14,9,10,11,23,22,21,20,19,18,17,16,15},
	{21,22,23,18,19,20,15,16,17,9,10,11,12,13,14,6,7,8,3,4,5,0,1,2},
	{15,16,17,18,19,20,21,22,23,11,10,9,14,13,12,0,1,2,3,4,5,6,7,8},
	{23,14,2,20,13,5,17,12,8,22,19,16,7,4,1,15,11,6,18,10,3,21,9,0},
	{17,12,8,20,13,5,23,14,2,16,19,22,1,4,7,21,9,0,18,10,3,15,11,6},
	{0,9,21,3,10,18,6,11,15,1,4,7,16,19,22,8,12,17,5,13,20,2,14,23},
	{6,11,15,3,10,18,0,9,21,7,4,1,22,19,16,2,14,23,5,13,20,8,12,17},
	{21,9,0,18,10,3,15,11,6,22,19,16,7,4,1,17,12,8,20,13,5,23,14,2},
	{15,11,6,18,10,3,21,9,0,16,19,22,1,4,7,23,14,2,20,13,5,17,12,8},
	{23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0},
	{17,16,15,20,19,18,23,22,21,12,13,14,9,10,11,2,1,0,5,4,3,8,7,6},
	{2,14,23,5,13,20,8,12,17,1,4,7,16,19,22,6,11,15,3,10,18,0,9,21},
	{8,12,17,5,13,20,2,14,23,7,4,1,22,19,16,0,9,21,3,10,18,6,11,15}
};

int adjacent3[9][5] = {
	{1,3,0,0,2},
	{0,2,4,0,3},
	{1,5,0,0,2},
	{0,4,6,0,3},
	{1,3,5,7,4},
	{2,4,8,0,3},
	{3,7,0,0,2},
	{4,6,8,0,3},
	{5,7,0,0,2}
};

int adjacent6[16][5] = {
	{1,6,0,0,2},
	{0,2,4,0,3},
	{1,9,0,0,2},
	{4,7,0,0,2},
	{1,3,5,0,3},
	{4,8,0,0,2},
	{0,7,13,0,3},
	{3,6,10,0,3},
	{5,9,12,0,3},
	{2,8,15,0,3},
	{7,11,0,0,2},
	{10,12,14,0,3},
	{8,11,0,0,2},
	{6,14,0,0,2},
	{11,13,15,0,3},
	{9,14,0,0,2}
};

int adjacent9[24][5] = {
	{1,9,0,0,2},
	{0,2,4,0,3},
	{1,14,0,0,2},
	{4,10,0,0,2},
	{1,3,5,7,4},
	{4,13,0,0,2},
	{7,11,0,0,2},
	{4,6,8,0,3},
	{7,12,0,0,2},
	{0,10,21,0,3},
	{3,9,11,18,4},
	{6,10,15,0,3},
	{8,13,17,0,3},
	{5,12,14,20,4},
	{2,13,23,0,3},
	{11,16,0,0,2},
	{15,17,19,0,3},
	{12,16,0,0,2},
	{10,19,0,0,2},
	{16,18,20,22,4},
	{13,19,0,0,2},
	{9,22,0,0,2},
	{19,21,23,0,3},
	{14,22,0,0,2}
};

int (*symmetriesToUse)[24];
int totalNumSymmetries;

POSITION fact(int n) {
     if (n <= 1) return 1;
	 POSITION prod = 1;
	 for (int i = 2; i <= n; i++)
		 prod *= i;
     return prod;
}

// calculates the number of combinations
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
unsigned long long combinations[25][10][10]; // 2400*8bytes
void combinationsInit() {
    for (int boardsize=0; boardsize<=24; boardsize++)
        for (int numx=0; numx<=9; numx++)
            for (int numo=0; numo<=9; numo++)
                combinations[boardsize][numx][numo] = getNumPos(boardsize, numx, numo);
}

/*************************************************************************
**
** HASH CACHE FUNCTIONS BEGIN
**
**************************************************************************/

// (Perfect) hash the board
unsigned long long hashIt(int boardsize, int numx, int numo, char *board) {
    unsigned long long sum=0;
    for (int i=boardsize-1; i>0; i--) { // no need to calculate i == 0
        switch (board[i]) {
            case 'X':
                numx--;
                break;
            case 'O':
                if (numx > 0) sum += combinations[i][numx-1][numo];
                numo--;
                break;
            case '.':
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
            board[i] = '.';
            pos -= o2;
        }
        else if (pos >= o1) {
            if (numo > 0) {
                board[i] = 'O';
                numo--;
            }
            else
                board[i] = '.';
            pos -= o1;
        }
        else {
            if (numx > 0) {
                board[i] = 'X';
                numx--;
            }
            else if (numo > 0) {
                board[i] = 'O';
                numo--;
            }
            else
                board[i] = '.';
        }
    }
}

typedef struct {
	int tier;
	POSITION position;
	char board[24];
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

/*************************************************************************
**
** HASH CACHE FUNCTIONS END
**
**************************************************************************/

/************************************************************************
**
** NAME:        InitializeGame
**
** DESCRIPTION: Prepares the game for execution.
**              Initializes required variables.
**
************************************************************************/
void InitializeGame() {

	/* FOR THE PURPOSES OF INTERACT. FEEL FREE TO CHANGE IF SOLVING. */ 
	if (gIsInteract) {
		gLoadTierdbArray = FALSE; // SET TO TRUE IF SOLVING
		memset(validTiers, 0, sizeof(char) * 1800);
	}
	/********************************/

	gSymmetries = TRUE; // SYMMETRIES UTILIZED BY DEFAULT.
	int i;
	combinationsInit();
	hashCacheInit();
	// SYMMETRY
	gCanonicalPosition = GetCanonicalPosition;
	if (gameType == 3) changeToThree();
	if (gameType == 6) changeToSix();
	if (gameType == 9) changeToNine();
	char* board = (char*) SafeMalloc(BOARDSIZE * sizeof(char));

	int pminmax[] = {X, 0, maxx, O, 0, maxo, BLANK, BOARDSIZE-maxx-maxo, BOARDSIZE, -1};

	gUnDoMoveFunPtr = &UndoMove;
	gGenerateUndoMovesToTierFunPtr = &GenerateUndoMovesToTier;
	gCustomUnhash = &customUnhash;
	gReturnTurn = &returnTurn;
	SetupTierStuff();

	for(i = 0; i < BOARDSIZE; i++)
		board[i] = BLANK;

	gNumberOfPositions = generic_hash_init(BOARDSIZE, pminmax, NULL, 0);
	gInitialPosition = hash(board, X, maxx+maxo, 0, 0);

	InitializeHelpStrings();

	if (gIsInteract) {
		ReinitializeTierDB(db_functions);
		oldvaluegetter = db_functions->get_value;
		oldremotenessgetter = db_functions->get_remoteness;
		db_functions->get_value = InteractGetValue;
		db_functions->get_remoteness = InteractGetRemoteness;
	}
}

/************************************************************************
**
** NAME:        InitializeHelpStrings
**
** DESCRIPTION: Sets up the help strings based on chosen game options.
**
** NOTES:       Should be called whenever the game options are changed.
**              (e.g., InitializeGame() and GameSpecificMenu())
**
************************************************************************/
void InitializeHelpStrings() {
	kHelpGraphicInterface =
	        "";

	kHelpTextInterface =
	        "";

	kHelpOnYourTurn =
	        "";

	kHelpStandardObjective =
	        "";

	kHelpReverseObjective =
	        "";

	kHelpTieOccursWhen =
	        "A tie occurs when ...";

	kHelpExample =
	        "";

	gMoveToStringFunPtr = &MoveToString;

}


/************************************************************************
**
** NAME:        GenerateMoves
**
** DESCRIPTION: Creates a linked list of every move that can be reached
**              from this position. Returns a pointer to the head of the
**              linked list.
**
** INPUTS:      POSITION position : Current position to generate moves
**
** OUTPUTS:     (MOVELIST *)      : A pointer to the first item of
**                                  the linked list of generated moves
**
** CALLS:       MOVELIST *CreateMovelistNode();
**
************************************************************************/
MOVELIST *GenerateMoves(POSITION position) {
	if (gIsInteract) {
		return InteractGenerateMoves(position);
	}
	MOVELIST *moves = NULL;
	char turn;
	int piecesLeft, numX, numO;
	char *board = unhash(position, &turn, &piecesLeft, &numX, &numO);
	int legalRemoves[BOARDSIZE];
	int numLegalRemoves = findLegalRemoves(board, turn, legalRemoves);
	int *legalTos;
	int numLegalTos;

	if (piecesLeft == 0 && (numX < 3 || numO < 3)) return NULL;

	int allBlanks[BOARDSIZE];
	int numBlanks = 0;
	for (int i = 0; i < BOARDSIZE; i++)
		if (board[i] == BLANK)
			allBlanks[numBlanks++] = i;

	if (piecesLeft > 0) {
		for (int i = 0; i < numBlanks; i++)
			if (closesMillNew(board, turn, 31, allBlanks[i]) && numLegalRemoves > 0)
				for (int j = 0; j < numLegalRemoves; j++)
					/* ONLY this type of move is encoded as "from: to, to: remove, remove: remove" instead of 
					"from: from, to: to, remove: remove" in order to distinguish this type of move (to+remove for placement and removal) 
					from the other type of two-argument move (from+to for sliding) for the sake of correctly converting 
					text inputs to moves. All functions that unhash moves will account for this peculiarity 
					and set from, to, and remove to the correct values.*/
					moves = CreateMovelistNode(MOVE_ENCODE(31, allBlanks[i], legalRemoves[j]), moves);
			else
				moves = CreateMovelistNode(MOVE_ENCODE(31, allBlanks[i], 31), moves);
	} else {
		for (int fromIdx = 0; fromIdx < BOARDSIZE; fromIdx++) {
			if (gFlying && ((turn == X && numX <= 3) || (turn == O && numO <= 3))) {
				legalTos = allBlanks;
				numLegalTos = numBlanks;
			} else {
				legalTos = adjacent[fromIdx];
				numLegalTos = adjacent[fromIdx][4];
			}
			if (board[fromIdx] == turn) {
				for (int i = 0; i < numLegalTos; i++) {
					if (board[legalTos[i]] != BLANK) continue;
					if (closesMillNew(board, turn, fromIdx, legalTos[i]) && numLegalRemoves > 0)
						for (int j = 0; j < numLegalRemoves; j++)
							moves = CreateMovelistNode(MOVE_ENCODE(fromIdx, legalTos[i], legalRemoves[j]), moves);
					else
						moves = CreateMovelistNode(MOVE_ENCODE(fromIdx, legalTos[i], 31), moves);
				}
			}
		}
	}

	SafeFree(board);
	return moves;
}

/*
Returns true if mill would be created if current player
places a piece at toIdx (if fromIdx = 31) or slides a piece from fromIdx to toIdx (otherwise).
*/
BOOLEAN closesMillNew(char *board, char turn, int fromIdx, int toIdx) {
	char copy[BOARDSIZE];
	memcpy(copy, board, sizeof(copy));
	if (fromIdx != 31) copy[fromIdx] = BLANK; // If sliding.
	return checkMill(copy, toIdx, turn);
}

int findLegalRemoves(char *board, char turn, int *legalRemoves) {
	int numLegalRemoves = 0;
	char oppTurn = turn == X ? O : X;
	if (millType == 0) { /* Standard. Removable if piece is not in a mill or all of opponent's pieces are in mills. */
		for (int i = 0; i < BOARDSIZE; i++)
			if (board[i] == oppTurn && (!checkMill(board, i, oppTurn) || all_mills(board, i, oppTurn)))
				legalRemoves[numLegalRemoves++] = i;
	} else if (millType == 1) { /* Any of opponent's pieces are removable. */
		for (int i = 0; i < BOARDSIZE; i++)
			if (board[i] == oppTurn)
				legalRemoves[numLegalRemoves++] = i;
	} else { /* Removable only if piece is not in mill. */
		for (int i = 0; i < BOARDSIZE; i++)
			if (board[i] == oppTurn && !checkMill(board, i, oppTurn))
				legalRemoves[numLegalRemoves++] = i;
	}
	return numLegalRemoves;
}

/************************************************************************
**
**
** BEGIN GENERATEUNDOMOVES FUNCTIONS
**
**
*************************************************************************/

UNDOMOVELIST *GenerateUndoMovesToTier(POSITION position, TIER tier) {
	UNDOMOVELIST *undoMoves = NULL;
	char turn;
	int piecesLeft, numX, numO;
	char *board = unhash(position, &turn, &piecesLeft, &numX, &numO);
	char oppTurn = turn == X ? O : X;
	int *legalFroms;
	int numLegalFroms;
	int undoMoveType;

	if (tier < 100 && ((tier / 10) % 10 == 2 || tier % 10 == 2)) return NULL;
	int currTier = piecesLeft * 100 + numX * 10 + numO;
	if (currTier == tier - (oppTurn == X ? 91 : 109))
		// Placement with removal.
		undoMoveType = 0;
	else if (currTier == tier - (oppTurn == X ? 90 : 99))
		// Placement without removal.
		undoMoveType = 1;
	else if (currTier < 100 && currTier == tier - (oppTurn == X ? 1 : 10))
		// Sliding/Flying with removal.
		undoMoveType = 2;
	else if (currTier < 100 && currTier == tier)
		// Sliding/Flying without removal.
		undoMoveType = 3;
	else {
		// Tier is not a parent tier of this position.
		return NULL;
	}

	int allBlanks[BOARDSIZE];
	int numBlanks = 0;
	for (int i = 0; i < BOARDSIZE; i++)
		if (board[i] == BLANK)
			allBlanks[numBlanks++] = i;
	legalFroms = allBlanks;
	numLegalFroms = numBlanks;

	int numCurr = turn == X ? numX + (piecesLeft / 2) : numO + (piecesLeft + 1 / 2);
	int legalRemoves[BOARDSIZE];
	int numLegalRemoves = findLegalRemovesUndo(board, turn, legalRemoves);

	// if (undoMoveType == 0 && numCurr < maxx) {
	// 	for (int toIdx = 0; toIdx < BOARDSIZE; toIdx++)
	// 		if (board[toIdx] == oppTurn && closesMillNew(board, oppTurn, 31, toIdx))
	// 			for (int i = 0; i < numLegalRemoves; i++)
	// 				/* ONLY this type of undomove is encoded as "from: to, to: remove, remove: remove" instead of 
	// 				"from: from, to: to, remove: remove" in order to distinguish this type of move (to+remove for placement and removal) 
	// 				from the other type of two-argument move (from+to for sliding) for the sake of correctly converting 
	// 				text inputs to moves. All functions that unhash moves will account for this peculiarity 
	// 				and set from, to, and remove to the correct values. */
	// 				undoMoves = CreateUndoMovelistNode(MOVE_ENCODE(toIdx, legalRemoves[i], legalRemoves[i]), undoMoves);
	// } else if (undoMoveType == 1) {
	// 	for (int toIdx = 0; toIdx < BOARDSIZE; toIdx++)
	// 		if (board[toIdx] == oppTurn && (!closesMillNew(board, oppTurn, 31, toIdx) || (millType == 2 && allMillsNew(board, -1, turn))))
	// 			undoMoves = CreateUndoMovelistNode(MOVE_ENCODE(31, toIdx, 31), undoMoves);
	// } else
	if (undoMoveType == 2 && numCurr < maxx) {
		for (int toIdx = 0; toIdx < BOARDSIZE; toIdx++) {
			if (gFlying && ((oppTurn == X && numX <= 3) || (oppTurn == O && numO <= 3))) {
				legalFroms = allBlanks;
				numLegalFroms = numBlanks;
			} else {
				legalFroms = adjacent[toIdx];
				numLegalFroms = adjacent[toIdx][4];
			}
			if (board[toIdx] == oppTurn && closesMillNew(board, oppTurn, 31, toIdx))
				for (int i = 0; i < numLegalFroms; i++)
					if (board[legalFroms[i]] == BLANK)
						for (int j = 0; j < numLegalRemoves; j++)
							if (legalFroms[i] != legalRemoves[j])
								undoMoves = CreateUndoMovelistNode(MOVE_ENCODE(legalFroms[i], toIdx, legalRemoves[j]), undoMoves);
		}
	} else {
		for (int toIdx = 0; toIdx < BOARDSIZE; toIdx++) {
			if (gFlying && ((oppTurn == X && numX <= 3) || (oppTurn == O && numO <= 3))) {
				legalFroms = allBlanks;
				numLegalFroms = numBlanks;
			} else {
				legalFroms = adjacent[toIdx];
				numLegalFroms = adjacent[toIdx][4];
			}
			if (board[toIdx] == oppTurn && (!closesMillNew(board, oppTurn, 31, toIdx) || (millType == 2 && allMillsNew(board, -1, turn))))
				for (int i = 0; i < numLegalFroms; i++)
					if (board[legalFroms[i]] == BLANK)
						undoMoves = CreateUndoMovelistNode(MOVE_ENCODE(legalFroms[i], toIdx, 31), undoMoves);
		}
	}

	SafeFree(board);
	return undoMoves;
}

// Given POSITION, slot
// Return true if player indicated by `turn` has all their pieces in mills.  
BOOLEAN allMillsNew(char *board, int slot, char turn) {
	if (slot != -1)
		board[slot] = turn;

	for (int i = 0; i < BOARDSIZE; i++) {
		if (board[i] == turn) {
			if (!checkMill(board, i, turn)) {
				if (slot != -1)
					board[slot] = BLANK;
				return FALSE;
			}
		}
	}

	if (slot != -1)
		board[slot] = BLANK;

	return TRUE;
}

int findLegalRemovesUndo(char *board, char turn, int *legalRemoves) {
	int numLegalRemoves = 0;
	if (millType == 0) { /* Standard. Removable if piece is not in a mill or all of opponent's pieces are in mills. */
		for (int i = 0; i < BOARDSIZE; i++)
			if (board[i] == BLANK && (!checkMill(board, i, turn) || allMillsNew(board, i, turn)))
				legalRemoves[numLegalRemoves++] = i;
	} else if (millType == 1) { /* Any of opponent's pieces are removable. */
		for (int i = 0; i < BOARDSIZE; i++)
			if (board[i] == BLANK)
				legalRemoves[numLegalRemoves++] = i;
	} else { /* Removable only if piece is not in mill. */
		for (int i = 0; i < BOARDSIZE; i++)
			if (board[i] == BLANK && !checkMill(board, i, turn))
				legalRemoves[numLegalRemoves++] = i;
	}
	return numLegalRemoves;
}

/************************************************************************
**
**
** END GENERATEUNDOMOVES FUNCTIONS
**
**
*************************************************************************/

/************************************************************************
**
** NAME:        DoMove
**
** DESCRIPTION: Applies the move to the position.
**
** INPUTS:      POSITION position : The old position
**              MOVE     move     : The move to apply to the position
**
** OUTPUTS:     (POSITION)        : The position that results from move
**
** CALLS:       Some Board Hash Function
**              Some Board Unhash Function
**
*************************************************************************/
POSITION DoMove(POSITION position, MOVE move) {
	if (gIsInteract) {
		return InteractDoMove(position, move);
	}
	char turn;
	int piecesLeft;
	int numX, numO;
	char* board = unhash(position, &turn, &piecesLeft, &numX, &numO);

	int fromIdx = move >> 10;
	int toIdx = (move >> 5) & 0x1F;
	int removeIdx = move & 0x1F;

	/* Correction for PECULIARITY */
	//if (toIdx == removeIdx) {
	//	toIdx = fromIdx;
	//	fromIdx = 31;
	//}

	board[toIdx] = turn;

	if (turn == X) {
		turn = O;
		if (fromIdx != 31) { // If sliding
			board[fromIdx] = BLANK;
		} else { // Phase 1
			piecesLeft--;
			numX++;
		}
		if (removeIdx != 31) {
			board[removeIdx] = BLANK;
			numO--;
		}
	} else {
		turn = X;
		if (fromIdx != 31) {
			board[fromIdx] = BLANK;
		} else {
			piecesLeft--;
			numO++;
		}
		if (removeIdx != 31) {
			board[removeIdx] = BLANK;
			numX--;
		}
	}

	TIER tier; TIERPOSITION tierposition;
	gUnhashToTierPosition(position, &tierposition, &tier);
	gCurrentTier = tier;

	POSITION toReturn = hash(board, turn, piecesLeft, numX, numO);
	SafeFree(board);

	return toReturn;
}

/************************************************************************
**
**
** BEGIN UNDOMOVE FUNCTIONS
**
**
*************************************************************************/

POSITION UndoMove(POSITION position, UNDOMOVE undoMove) {
	char turn;
	int piecesLeft;
	int numX, numO;
	char* board = unhash(position, &turn, &piecesLeft, &numX, &numO);

	int fromIdx = undoMove >> 10;
	int toIdx = (undoMove >> 5) & 0x1F;
	int removeIdx = undoMove & 0x1F;

	/* Correction for PECULIARITY */
	//if (toIdx == removeIdx) {
	//	toIdx = fromIdx;
	//	fromIdx = 31;
	//}

	board[toIdx] = BLANK;

	if (turn == X) {
		turn = O; /* If turn in child position is X, then in parent position is O. */
		if (fromIdx != 31) { // If sliding
			board[fromIdx] = O;
		} else { // Phase 1
			piecesLeft++;
			numO--;
		}
		if (removeIdx != 31) {
			board[removeIdx] = X;
			numX++;
		}
	} else {
		turn = X;
		if (fromIdx != 31) {
			board[fromIdx] = X;
		} else {
			piecesLeft++;
			numX--;
		}
		if (removeIdx != 31) {
			board[removeIdx] = O;
			numO++;
		}
	}

	POSITION toReturn = hash(board, turn, piecesLeft, numX, numO);
	SafeFree(board);

	return toReturn;
}

/************************************************************************
**
**
** END UNDOMOVE FUNCTIONS
**
**
*************************************************************************/

/************************************************************************
**
** NAME:        Primitive
**
** DESCRIPTION: Returns the value of a position if it fulfills certain
**              'primitive' constraints.
**
**              Example: Tic-tac-toe - Last piece already placed
**
**              Case                                  Return Value
**              *********************************************************
**              Current player sees three in a row    lose
**              Entire board filled                   tie
**              All other cases                       undecided
**
** INPUTS:      POSITION position : The position to inspect.
**
** OUTPUTS:     (VALUE)           : one of
**                                  (win, lose, tie, undecided)
**
** CALLS:       None
**
************************************************************************/

VALUE Primitive (POSITION position) {
	char *board;
	char turn;
	int piecesLeft;
	int numx, numo;

	board = unhash(position, &turn, &piecesLeft, &numx, &numo);
	SafeFree(board);

	if(piecesLeft == 0) { // Check if we are in stage 2 (stage 3 included in 2 with special rules)
		if ((numx < 3) || (numo < 3))
			return gStandardGame ? lose : win;

		MOVELIST* moves = GenerateMoves(position);
		if (NULL == moves)
			return gStandardGame ? lose : win;
		FreeMoveList(moves);
	}
	return undecided;
}


/************************************************************************
**
** NAME:        PrintPosition
**
** DESCRIPTION: Prints the position in a pretty format, including the
**              prediction of the game's outcome.
**
** INPUTS:      POSITION position    : The position to pretty print.
**              STRING   playersName : The name of the player.
**              BOOLEAN  usersTurn   : TRUE <==> it's a user's turn.
**
** CALLS:       Unhash()
**              GetPrediction()      : Returns the prediction of the game
**
************************************************************************/

void PrintPosition (POSITION position, STRING playersName, BOOLEAN usersTurn) {
	char turn;
	int piecesLeft;
	int numx, numo;

	char* board = unhash(position, &turn, &piecesLeft, &numx, &numo);

	if (gameType==3) {
		printf("\n");
		printf("          0 -- 1 -- 2    %c -- %c -- %c   |    %s's turn (%c)\n", board[0], board[1], board[2], playersName, turn);
		printf("          |    |    |    |    |    |   |    Phase: ");
		if (piecesLeft != 0)
			printf("1 : PLACING\n");
		else {
			if  (!gFlying || ((turn == X) && (numx > 3)) || ((turn == O) && (numo > 3)))
				printf("2 : SLIDING\n");
			else
				printf("3 : FLYING\n");
		}
		printf("          3 -- 4 -- 5    %c -- %c -- %c   |    Phase: ", board[3], board[4], board[5]);

		if (piecesLeft != 0)
			printf("X has %d left to place\n",piecesLeft/2);
		else
			printf("X has %d on the board\n", numx);

		printf("          |    |    |    |    |    |   |    Phase: ");

		if (piecesLeft != 0)
			printf("O has %d left to place\n",piecesLeft/2 + piecesLeft%2);
		else
			printf("O has %d on the board\n", numo);

		printf("          6 -- 7 -- 8    %c -- %c -- %c   |    %s\n\n", board[6], board[7], board[8], GetPrediction(position, playersName, usersTurn));


	}

	else if (gameType==6) {

		printf("\n");
		printf("          0 ----- 1 ----- 2    %c ----- %c ----- %c    %s's turn (%c)\n", board[0], board[1], board[2], playersName, turn);
		printf("          |       |       |    |       |       |    \n");
		printf("          |   3 - 4 - 5   |    |   %c - %c - %c   |    Phase: ", board[3], board[4], board[5]);
		if (piecesLeft != 0)
			printf("1 : PLACING\n");
		else {
			if  (!gFlying || ((turn == X) && (numx > 3)) || ((turn == O) && (numo > 3)))
				printf("2 : SLIDING\n");
			else
				printf("3 : FLYING\n");
		}
		printf("          |   |       |   |    |   |       |   |    ");
		if (piecesLeft != 0)
			printf("X has %d left to place\n",piecesLeft/2);
		else
			printf("X has %d on the board\n", numx);
		printf("LEGEND:   6 - 7       8 - 9    %c - %c       %c - %c    ", board[6], board[7], board[8], board[9]);
		if (piecesLeft != 0)
			printf("O has %d left to place\n",piecesLeft/2 + piecesLeft%2);
		else
			printf("O has %d on the board\n", numo);
		printf("          |   |       |   |    |   |       |   |    \n");
		printf("          |  10 - 11- 12  |    |   %c - %c - %c   |  \n", board[10], board[11], board[12] );
		printf("          |       |       |    |       |       |     \n");
		printf("          13 ---- 14 ---- 15   %c ----- %c ----- %c    %s\n\n", board[13], board[14], board[15],
		       GetPrediction(position,playersName,usersTurn));

	} else {
		printf("\n");
		printf("        0 --------- 1 --------- 2       %c --------- %c --------- %c    %s's turn (%c)\n", board[0], board[1], board[2], playersName, turn );
		printf("        |           |           |       |           |           |\n");
		printf("        |   3 ----- 4 ----- 5   |       |   %c ----- %c ----- %c   |    Phase: ", board[3], board[4], board[5]);
		if (piecesLeft != 0)
			printf("1 : PLACING\n");
		else {
			if  (!gFlying || ((turn == X) && (numx > 3)) || ((turn == O) && (numo > 3)))
				printf("2 : SLIDING\n");
			else
				printf("3 : FLYING\n");
		}
		printf("        |   |       |       |   |       |   |       |       |   |    ");
		if (piecesLeft != 0)
			printf("X has %d left to place\n",piecesLeft/2);
		else
			printf("X has %d on the board\n", numx);
		printf("        |   |   6 - 7 - 8   |   |       |   |   %c - %c - %c   |   |    ", board[6], board[7], board[8] );
		if (piecesLeft != 0)
			printf("O has %d left to place\n",piecesLeft/2 + piecesLeft%2);
		else
			printf("O has %d on the board\n", numo);
		printf("        |   |   |       |   |   |       |   |   |       |   |   |\n");
		printf("LEGEND: 9 - 10- 11      12- 13- 14      %c - %c - %c       %c - %c - %c\n", board[9], board[10], board[11], board[12], board[13], board[14]);
		printf("        |   |   |       |   |   |       |   |   |       |   |   |\n");
		printf("        |   |   15- 16- 17  |   |       |   |   %c - %c - %c   |   |\n", board[15], board[16], board[17] );
		printf("        |   |       |       |   |       |   |       |       |   |\n");
		printf("        |   18 ---- 19 ---- 20  |       |   %c ----- %c ----- %c   |\n", board[18], board[19], board[20] );
		printf("        |           |           |       |           |           |\n");
		printf("        21 -------- 22 -------- 23      %c --------- %c --------- %c    %s\n\n", board[21], board[22], board[23], GetPrediction(position, playersName, usersTurn) );

	}
	SafeFree(board);
}


/************************************************************************
**
** NAME:        PrintComputersMove
**
** DESCRIPTION: Nicely formats the computers move.
**
** INPUTS:      MOVE    computersMove : The computer's move.
**              STRING  computersName : The computer's name.
**
************************************************************************/

void PrintComputersMove (MOVE computersMove, STRING computersName) {
	STRING str = MoveToString( computersMove );
	if (gameType == 3) {
		printf("%8s's Move                                     :    %s\n",computersName,str);
	} else if (gameType == 6) {
		printf("%8s's Move                                :    %s\n",computersName,str);
	} else {
		printf("%8s's Move                                                 :    %s\n",computersName,str);
	}
	SafeFree( str );
}


/************************************************************************
**
** NAME:        PrintMove
**
** DESCRIPTION: Prints the move in a nice format.
**
** INPUTS:      MOVE move         : The move to print.
**
************************************************************************/

void PrintMove (MOVE move) {
	STRING str = MoveToString( move );
	printf( "%s", str );
	SafeFree( str );
}


/************************************************************************
**
** NAME:        MoveToString
**
** DESCRIPTION: Returns the move as a STRING
**
** INPUTS:      MOVE *move         : The move to put into a string.
**
************************************************************************/

STRING MoveToString(MOVE move) {
	int fromIdx = move >> 10;
	int toIdx = (move >> 5) & 0x1F;
	int removeIdx = move & 0x1F;
	/* Correction for PECULIARITY */
	//if (toIdx == removeIdx) {
	//	toIdx = fromIdx;
	//	fromIdx = 31;
	//}

	STRING movestring;
	if (fromIdx != 31 && toIdx != 31 && removeIdx != 31) {
		movestring = (STRING) SafeMalloc(12);
		sprintf( movestring, "%d-%dr%d",fromIdx, toIdx, removeIdx);
	} else if (fromIdx != 31 && toIdx != 31 && removeIdx == 31) {
		movestring = (STRING) SafeMalloc(8);
		sprintf( movestring, "%d-%d", fromIdx, toIdx);
	} else if (fromIdx == 31 && toIdx != 31 && removeIdx == 31) {//if 1st == 2nd position in move formula
		movestring = (STRING) SafeMalloc(8);
		sprintf(movestring, "%d", toIdx);
	} else {
		movestring = (STRING) SafeMalloc(8);
		sprintf(movestring, "%dr%d", toIdx, removeIdx);
	}

	return movestring;
}

/************************************************************************
**
** NAME:        GetAndPrintPlayersMove
**
** DESCRIPTION: Finds out if the player wishes to undo, abort, or use
**              some other gamesman option. The gamesman core does
**              most of the work here.
**
** INPUTS:      POSITION position    : Current position
**              MOVE     *move       : The move to fill with user's move.
**              STRING   playersName : Current Player's Name
**
** OUTPUTS:     USERINPUT          : One of
**                                   (Undo, Abort, Continue)
**
** CALLS:       USERINPUT HandleDefaultTextInput(POSITION, MOVE*, STRING)
**                                 : Gamesman Core Input Handling
**
************************************************************************/

USERINPUT GetAndPrintPlayersMove (POSITION position, MOVE *move, STRING playersName) {
	USERINPUT input;
	char turn;
	int piecesLeft, numx, numo;
	char *board = unhash(position, &turn, &piecesLeft, &numx, &numo);
	SafeFree(board);

	BOOLEAN existsRemoves = FALSE;
	BOOLEAN allRemoves = TRUE;
	MOVELIST *moveList = GenerateMoves(position);
	MOVELIST *moveListPtr = moveList;
	for (; moveListPtr != NULL; moveListPtr = moveListPtr->next) {
		if ((moveListPtr->move & 0x1F) != 31) {
			existsRemoves = TRUE;
		} else {
			allRemoves = FALSE;
		}
	}
	SafeFree(moveList);

	do {
		int maxslots = ((gameType == 3) ? 8 : (gameType == 6) ? 15 : 23);
		int spacemaxslots = (gameType == 3) ? 1 : 2;
		printf("%8s's Move: (u)ndo", playersName);
		int numSpaces = (gameType == 9) ? 41 : 24;
		if (!allRemoves) {
			if (piecesLeft != 0) {// STAGE 1 : PLACING
				printf("/[0-%d]", maxslots);
				numSpaces -= (5 + spacemaxslots);
			} else {
				printf("/[0-%d]-[0-%d]", maxslots, maxslots);
				numSpaces -= (10 + spacemaxslots * 2);
			}
		}
		if (existsRemoves) {
			if (piecesLeft != 0) {// STAGE 1 : PLACING
				printf("/[0-%d]r[0-%d]", maxslots, maxslots);
				numSpaces -= (10 + spacemaxslots * 2);
			} else {
				printf("/[0-%d]-[0-%d]r[0-%d]", maxslots, maxslots, maxslots);
				numSpaces -= (15 + spacemaxslots * 3);
			}
		}
		for (int i = 0; i < numSpaces; i++) {
			printf(" ");
		}
		printf(":    ");

		input = HandleDefaultTextInput(position, move, playersName);

		if (input != Continue)
			return input;
	} while (TRUE);

	/* NOTREACHED */
	return (Continue);
}


/************************************************************************
**
** NAME:        ValidTextInput
**
** DESCRIPTION: Rudimentary check to check if input is in the move form
**              you are expecting. Does not check if it is a valid move.
**              Only checks if it fits the move form.
**
**              Reserved Input Characters - DO NOT USE THESE ONE CHARACTER
**                                          COMMANDS IN YOUR GAME
**              ?, s, u, r, h, a, c, q
**                                          However, something like a3
**                                          is okay.
**
**              Example: Tic-tac-toe Move Format : Integer from 1 to 9
**                       Only integers between 1 to 9 are accepted
**                       regardless of board position.
**                       Moves will be checked by the core.
**
** INPUTS:      STRING input : The string input the user typed.
**
** OUTPUTS:     BOOLEAN      : TRUE if the input is a valid text input.
**
************************************************************************/

BOOLEAN ValidTextInput(STRING input) {
	//DONE
	int maxslots = ((gameType == 3) ? 8 : (gameType == 6) ? 15 : 23);
	int i = 0;
	BOOLEAN existsR = FALSE;
	BOOLEAN existsDash = FALSE;
	int currNum = 0;
	while (input[i] != '\0') {
		if (!((input[i] >= 48 && input[i] <= 57) || input[i] == '-' || input[i] == 'r')) {
			return FALSE;
		}
		if (input[i] == '-') {
			if (existsDash || existsR) {
				return FALSE;
			} else {
				existsDash = TRUE;
				currNum = 0;
			}
		} else if (input[i] == 'r') {
			if (existsR) {
				return FALSE;
			} else {
				existsR = TRUE;
				currNum = 0;			
			}
		} else {
			currNum = currNum * 10 + (input[i] - '0');
			if (currNum > maxslots) {
				return FALSE;
			}
		}
		i++;
	}
	return TRUE;
}

/************************************************************************
**
** NAME:        ConvertTextInputToMove
**
** DESCRIPTION: Converts the string input your internal move representation.
**              Gamesman already checked the move with ValidTextInput
**              and ValidMove.
**
** INPUTS:      STRING input : The VALID string input from the user.
**
** OUTPUTS:     MOVE         : Move converted from user input.
**
************************************************************************/

MOVE ConvertTextInputToMove(STRING input) {
	//DONE
	int from = 31;
	int to = 31; //that way if no input for remove, it's equal to from. useful in function DoMove
	int remove = 31; // for stage 1, if there is nothing to remove

	int i = 0;
	int phase = 0;
	int first = 0;
	int second = 0;
	int third = 0;
	BOOLEAN existsSliding = FALSE;
	while (input[i] != '\0') {
		if (input[i] == 'r' || input[i] == '-') {
			if (input[i] == '-') {
				existsSliding = TRUE;
			}
			phase++;
			i++;
			continue;
		}
		if (phase == 0) {
			first = first * 10 + (input[i] - '0');
		} else if (phase == 1) {
			second = second * 10 + (input[i] - '0');
		} else {
			third = third * 10 + (input[i] - '0');
		}
		i++;
	}

	if (phase == 0) { // Placement without removal
		to = first;
	} else if (phase == 2) { // Sliding/flying with removal
		from = first;
		to = second;
		remove = third;
	} else if (existsSliding) { // Sliding/flying without removal
		from = first;
		to = second;
	} else { // Placement with removal peculiarity.
		//from = first;
		//to = second;
		to = first;
		remove = second;
	}
	//printf("converttextinputtomove.... move = %d\n", MOVE_ENCODE(from, to, remove));
	//printf("from: %d, to: %d, remove: %d\n", from, to, remove);
	return MOVE_ENCODE(from, to, remove); //HASHES THE MOVE
}

/************************************************************************
**
** NAME:        GameSpecificMenu
**
** DESCRIPTION: Prints, receives, and sets game-specific parameters.
**
**              Examples
**              Board Size, Board Type
**
**              If kGameSpecificMenu == FALSE
**                   Gamesman will not enable GameSpecificMenu
**                   Gamesman will not call this function
**
**              Resets gNumberOfPositions if necessary
**
************************************************************************/

void GameSpecificMenu() {
	char GetMyChar();
	POSITION GetInitialPosition();

	do {
		printf("\n\t----- Game-specific options for %s -----\n\n", kGameName);


		//printf("\tCurrent Initial Position:\n");
		//PrintPosition(gInitialPosition, gPlayerName[kPlayerOneTurn], kHumansTurn);

		printf("\n");
		//printf("\ti)\tChoose the (I)nitial position\n");
		printf("\tf)\tToggle (F)lying from %s to %s\n",
		       gFlying ? "ON" : "OFF",
		       !gFlying ? "ON" : "OFF");

		if (millType == 0) {
			printf("\tm)\tWhen mill is formed, can remove any opponent's piece.\n");
		}
		else if (millType == 1) {
			printf("\tm)\tWhen mill is formed, can remove opponent's piece if it is not in a mill.\n");
		}
		else if (millType == 2) {
			printf("\tm)\tWhen mill is formed, can remove opponent's piece if it is not in a mill, unless if all the remaining pieces are already in a mill.\n");
		}

		char currentType[10];
		strcpy(currentType, gameType == 3 ? "Three" : gameType == 6 ? "Six" : "Nine");

		if (gameType != 3) {
			printf("\t3)\tSwitch from %s Men's Morris to Three Men's Morris.\n", currentType);
		}
		if (gameType != 6) {
			printf("\t6)\tSwitch from %s Men's Morris to Six Men's Morris.\n", currentType);
		}
		if (gameType != 9) {
			printf("\t9)\tSwitch from %s Men's Morris to Nine Men's Morris.\n", currentType);
		}
		

		printf("\n\n\tb)\t(B)ack = Return to previous activity.\n");
		printf("\n\nSelect an option: ");

		switch(GetMyChar()) {
		case 'M': case 'm':
			if (millType == 0)
				millType = 1;
			else if (millType == 1)
				millType = 2;
			else if (millType == 2)
				millType = 0;
			break;
		case '3':
			changeToThree();
			break;
		case '6':
			changeToSix();
			break;
		case '9':
			changeToNine();
			break;
		case 'Q': case 'q':
			ExitStageRight();
		case 'H': case 'h':
			HelpMenus();
			break;
		//    case 'I': case 'i':
		//gInitialPosition = GetInitialPosition();
		//break;
		case 'F': case 'f':
			gFlying = !gFlying;
			break;
		case 'B': case 'b':
			return;
		default:
			printf("\nSorry, I don't know that option. Try another.\n");
			HitAnyKeyToContinue();
			break;
		}
	} while(TRUE);

	//FINISH COPY OLD CODE
	InitializeHelpStrings();
}


/************************************************************************
**
** NAME:        SetTclCGameSpecificOptions
**
** DESCRIPTION: Set the C game-specific options (called from Tcl)
**              Ignore if you don't care about Tcl for now.
**
************************************************************************/

void SetTclCGameSpecificOptions(int options[]) {
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
	return 36; //misere/standard   flying/no-flying   3mm/6mm/9mm millType = 1/2/3
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
	return 1 + (gFlying<<1) + gStandardGame + ((gameType/3-1)<<2) + (millType<<4);
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
	// In terms of bits, option is one more than 0bFS (F=flying,S=Standard)
	int temp;
	option -= 1;
	gStandardGame = (option % 2);
	gFlying       = (option >> 1) % 2;
	temp = gameType;
	gameType = ((((option>>2) % 4)+1)*3);
	millType = (option >> 4);
	if((temp != gameType) && (gameType==3)) {
		changeToThree();
	}
	else if((temp != gameType) && (gameType==6)) {
		changeToSix();
	}
	else if ((temp != gameType) && (gameType == 9)) {
		changeToNine();
	}
}


/************************************************************************
**
** NAME:        DebugMenu
**
** DESCRIPTION: Game Specific Debug Menu (Gamesman comes with a default
**              debug menu). Menu used to debug internal problems.
**
**              If kDebugMenu == FALSE
**                   Gamesman will not display a debug menu option
**                   Gamesman will not call this function
**
************************************************************************/

void DebugMenu() {

}


/************************************************************************
**
** Everything specific to this module goes below these lines.
**
** Things you want down here:
** Move Hasher
** Move Unhasher
** Any other function you deem necessary to help the ones above.
**
************************************************************************/

//this was made just for "tkAppInit.c" we are returning the turn so that the gui knows whose turn it is.
char returnTurn(POSITION pos) {
	if(gHashWindowInitialized) {
		TIER tier; TIERPOSITION tierposition;
		if (gIsInteract && isIntermediate(pos)) {
			pos -= (1UL << 63);
		}
		gUnhashToTierPosition(pos, &tierposition, &tier);
		generic_hash_context_switch(tier);
		return (generic_hash_turn(tierposition)==PLAYER_ONE ? X : O);
	}
	return '0';
}

//this was made just for "tkAppInit.c" we are returning the board so that the gui knows whose turn it is.
char* customUnhash(POSITION pos) {
	//piecesLeft = total pieces left during stage 1 (x + o)
	char* board = (char*)SafeMalloc(BOARDSIZE * sizeof(char));
	if(gHashWindowInitialized) {
		TIER tier; TIERPOSITION tierposition;
		if (gIsInteract && isIntermediate(pos)) {
			pos -= (1UL << 63);
		}
		gUnhashToTierPosition(pos, &tierposition, &tier);
		generic_hash_context_switch(tier);
		board = (char*)generic_hash_unhash(tierposition, board);
	}
	return board;
}

//this is the main unhash function that the C code uses.
char* unhash(POSITION pos, char* turn, int* piecesLeft, int* numx, int* numo) {
	//piecesLeft = total pieces left during stage 1 (x + o)
	char* board = (char*)SafeMalloc(BOARDSIZE * sizeof(char));
	BOOLEAN intermediate = FALSE;
	if (gIsInteract && isIntermediate(pos)) {
		pos -= (1UL << 63);
		intermediate = TRUE;
	}
	if (gHashWindowInitialized) {
		TIER tier; TIERPOSITION tierposition;
		gUnhashToTierPosition(pos, &tierposition, &tier);
		// printf("THE TIER THAT'S LOADED IS %llu %llu\n", tier, tierposition);
		generic_hash_context_switch(tier);
		(*turn) = (generic_hash_turn(tierposition) == PLAYER_ONE ? X : O);

		*piecesLeft = tier / 100;
		*numx = (tier / 10) % 10;
		*numo = tier % 10;

		if (gIsInteract && intermediate && *piecesLeft > 0) {
			*turn = ((*piecesLeft % 2) == 0) ? O : X;
			//*turn = (*turn == X) ? O : X;
		}/* else if (gIsInteract && *piecesLeft == 0) {
			if (tierposition > combinations[BOARDSIZE][*numx][*numo]) {
				*turn = O;
			} else {
				*turn = X;
			}
		}*/

		BOOLEAN cache_miss = hashCacheGet(tier, tierposition, board);
		if (cache_miss) {
			unhashIt(BOARDSIZE, *numx, *numo, tierposition % combinations[BOARDSIZE][*numx][*numo], board);
			hashCachePut(tier, tierposition, board);
		}
	} else {
		printf("unhashing without tiers... cannot solve game\n");
		generic_hash_unhash(pos, board);
		*piecesLeft = totalPieces; //remove when tiering
		if (generic_hash_turn(pos) == PLAYER_ONE)
			(*turn) = X;
		else (*turn) = O;
		*numx = NUMX; //remove when tiering
		*numo = NUMO; // remove when tiering
	}
	return board;
}

POSITION hash(char* board, char turn, int piecesLeft, int numx, int numo) {
	POSITION pos;
	if (gHashWindowInitialized) {
		TIER tier = piecesLeft * 100 + numx * 10 + numo;
		generic_hash_context_switch(tier);
		TIERPOSITION tierposition = hashIt(BOARDSIZE, numx, numo, board);

		if (piecesLeft == 0 && turn == O) {
			tierposition += combinations[BOARDSIZE][numx][numo];
		}
		pos = gHashToWindowPosition(tierposition, tier);
	} else {
		pos = generic_hash_hash(board, (turn == X ? PLAYER_ONE : PLAYER_TWO));
		if (board != NULL)
			SafeFree(board);

		totalPieces = piecesLeft; //remove when tiering
		NUMX = numx; //remove when tiering
		NUMO = numo; //remove when tiering
	}
	return pos;
}

void SetupTierStuff() {
	generic_hash_destroy();
	char* board = (char*) SafeMalloc(BOARDSIZE * sizeof(char));
	kSupportsTierGamesman = TRUE;
	gTierChildrenFunPtr = &gTierChildren;
	gNumberOfTierPositionsFunPtr = &gNumberOfTierPositions;
	gTierToStringFunPtr = &TierToString;
	int tier, i, piecesLeft, numx, numo;
	generic_hash_custom_context_mode(TRUE);
	//	int pminmax[] = {X, 0, maxx, O, 0, maxo,BLANK, BOARDSIZE-maxx-maxo, BOARDSIZE, -1};
	int pieces_array[] = {X, 0, 0, O, 0, 0, BLANK, 0, 0, -1  };
	kExclusivelyTierGamesman = TRUE;

	for(piecesLeft=0; piecesLeft<maxx+maxo+1; piecesLeft++) {
		for(numx=0; numx<maxx+1; numx++) {
			for(numo=0; numo<maxo+1; numo++) {
				tier=piecesLeft*100+numx*10+numo;
				pieces_array[1]=pieces_array[2]=numx;
				pieces_array[4]=pieces_array[5]=numo;
				pieces_array[7]=pieces_array[8]=BOARDSIZE-numx-numo;
				if (piecesLeft > 0) { //stage1
					//pieces_array[] = {X, 0, piecesLeft/2+1, O, piecesLeft/2+1, 0, BLANK, 0, 0, -1  } ;
					generic_hash_init(BOARDSIZE, pieces_array, NULL, (piecesLeft%2)+1);
				}
				else{
					//pieces_array[] = {X, minx, maxx, O, mino, maxo, BLANK, 0, 0, -1  } ;
					generic_hash_init(BOARDSIZE, pieces_array, NULL, 0);
				}
				generic_hash_set_context(tier);
				//printf("tier %d\t", tier);
				//int j;
				//for (j = 0; j < 10; j++)
				//printf("%d ", pieces_array[j]);
				//printf("\n");
			}
		}
	}

	tier = (maxx+maxo)*100;
	gInitialTier = tier;    //initial pieces on board
	gCurrentTier = tier;
	generic_hash_context_switch(tier);

	for(i = 0; i < BOARDSIZE; i++)
	{
		board[i] = BLANK;
	}
	gInitialTierPosition = generic_hash_hash(board, PLAYER_ONE);
	//SafeFree(board);
	//printf("line 1142 gInitialTierPosition = %d\n",  gInitialTierPosition);
}

TIERLIST* gTierChildren(TIER tier) {
	TIERLIST* list = NULL;
	int piecesLeft = tier / 100;
	int numX = (tier / 10) % 10;
	int numO = tier % 10;

	//printf("gTierChildren called %llu\n", tier);

	if (piecesLeft != 0) {
		if (piecesLeft % 2 == 0) {
			list = CreateTierlistNode(tier-100+10, list);         //adding piece
			validTiers[tier-100+10] = 1;
			if (numX > 1) {
				list = CreateTierlistNode(tier-100+9, list); //adding and removing
				validTiers[tier-100+9] = 1;
			}
		} else {
			list = CreateTierlistNode(tier-100+1, list);         //adding piece
			validTiers[tier-100+1] = 1;
			if (numO > 1) {
				list = CreateTierlistNode(tier-100-9, list); //adding and removing
				validTiers[tier-100-9] = 1;
			}
		}
	} else if (piecesLeft == 0) { //stage 2 or 3. We do not know the turn, so we assume both paths as tier children
		list = CreateTierlistNode(tier, list);
		if (numX > 2 && numO > 2) {
			list = CreateTierlistNode(tier-10, list); //remove X piece
			validTiers[tier-10] = 1;
			list = CreateTierlistNode(tier-1, list); //remove O piece
			validTiers[tier-1] = 1;
		}
	}

	// Support for two-phase moves.
	if (gIsInteract && piecesLeft > 0) {
		if (piecesLeft % 2 == 0) { // O is about to remove
			if (numX > 1 && numO > 2) {
				if (validTiers[tier - 10]) {
					list = CreateTierlistNode(tier - 10, list);
				}
			}
		} else { // X is about to remove
			if (numO > 1 && numX > 2) {
				if (validTiers[tier - 1]) {
					list = CreateTierlistNode(tier - 1, list);
				}
			}
		}
	}

	return list;
}

TIERPOSITION gNumberOfTierPositions (TIER tier) {
	generic_hash_context_switch(tier);
	return generic_hash_max_pos();
}

STRING TierToString(TIER tier) {
	int piecesLeft, numx, numo;
	piecesLeft = (tier/100);
	numx = (tier/10)%10;
	numo = tier%10;
	STRING str = (STRING) SafeMalloc (sizeof (char) * 50);
	if (piecesLeft > 0)
		sprintf(str, "STAGE 1: left-%d x-%d o-%d", piecesLeft, numx, numo);
	else sprintf(str, "STAGE 2: x-%d o-%d", numx, numo);
	return str;
}

// Given POSITION, slot
// Return whether player indicated by `turn` has all pieces in a mill.
BOOLEAN all_mills(char *board, int slot, char turn) {
	for (int i = 0; i < BOARDSIZE; i++) {
		if (board[i] == turn) {
			if (!checkMill(board, i, turn)) {
				return FALSE;
			}
		}
	}
	return TRUE;
}

// Given bboard, int array
// Return number of pieces and array of each slot containing those pieces
int find_pieces(char *board, char piece, int *pieces) {
	int num = 0;

	for (int i = 0; i < BOARDSIZE; i++) {
		if (board[i] == piece) {
			pieces[num] = i;
			num++;
		}
	}

	return num;
}


// given new board, slot
// return true if slot is member of mill
BOOLEAN checkMill(char *board, int slot, char turn) {
	if (gameType == 3) {
		switch (slot) {
		case 0:
			return THREE_IN_A_ROW(board, 1, 2, turn) || THREE_IN_A_ROW(board, 3, 6, turn);
		case 1:
			return THREE_IN_A_ROW(board, 0, 2, turn) || THREE_IN_A_ROW(board, 4, 7, turn);
		case 2:
			return THREE_IN_A_ROW(board, 1, 0, turn) || THREE_IN_A_ROW(board, 5, 8, turn);
		case 3:
			return THREE_IN_A_ROW(board, 4, 5, turn) || THREE_IN_A_ROW(board, 0, 6, turn);
		case 4:
			return THREE_IN_A_ROW(board, 1, 7, turn) || THREE_IN_A_ROW(board, 3, 5, turn);
		case 5:
			return THREE_IN_A_ROW(board, 8, 2, turn) || THREE_IN_A_ROW(board, 3, 4, turn);
		case 6:
			return THREE_IN_A_ROW(board, 0, 3, turn) || THREE_IN_A_ROW(board, 7, 8, turn);
		case 7:
			return THREE_IN_A_ROW(board, 1, 4, turn) || THREE_IN_A_ROW(board, 6, 8, turn);
		case 8:
			return THREE_IN_A_ROW(board, 5, 2, turn) || THREE_IN_A_ROW(board, 6, 7, turn);
		default:
			return FALSE;
		}
	} else if (gameType == 6) {
		switch (slot) {
		case 0:
			return THREE_IN_A_ROW(board, 1, 2, turn) || THREE_IN_A_ROW(board, 6, 13, turn);
		case 1:
			return THREE_IN_A_ROW(board, 0, 2, turn);
		case 2:
			return THREE_IN_A_ROW(board, 1, 0, turn) || THREE_IN_A_ROW(board, 9, 15, turn);
		case 3:
			return THREE_IN_A_ROW(board, 4, 5, turn) || THREE_IN_A_ROW(board, 7, 10, turn);
		case 4:
			return THREE_IN_A_ROW(board, 3, 5, turn);
		case 5:
			return THREE_IN_A_ROW(board, 8, 12, turn) || THREE_IN_A_ROW(board, 3, 4, turn);
		case 6:
			return THREE_IN_A_ROW(board, 0, 13, turn);
		case 7:
			return THREE_IN_A_ROW(board, 3, 10, turn);
		case 8:
			return THREE_IN_A_ROW(board, 5, 12, turn);
		case 9:
			return THREE_IN_A_ROW(board, 2, 15, turn);
		case 10:
			return THREE_IN_A_ROW(board, 3, 7, turn) || THREE_IN_A_ROW(board, 11, 12, turn);
		case 11:
			return THREE_IN_A_ROW(board, 10, 12, turn);
		case 12:
			return THREE_IN_A_ROW(board, 10, 11, turn) || THREE_IN_A_ROW(board, 5, 8, turn);
		case 13:
			return THREE_IN_A_ROW(board, 14, 15, turn) || THREE_IN_A_ROW(board, 0, 6, turn);
		case 14:
			return THREE_IN_A_ROW(board, 13, 15, turn);
		case 15:
			return THREE_IN_A_ROW(board, 13, 14, turn) || THREE_IN_A_ROW(board, 2, 9, turn);
		default:
			return FALSE;
		}
	} else if (gameType == 9) {
		switch (slot) {
		case 0:
			return THREE_IN_A_ROW(board, 1, 2, turn) || THREE_IN_A_ROW(board, 9, 21, turn);
		case 1:
			return THREE_IN_A_ROW(board, 0, 2, turn) || THREE_IN_A_ROW(board, 4, 7, turn);
		case 2:
			return THREE_IN_A_ROW(board, 1, 0, turn) || THREE_IN_A_ROW(board, 14, 23, turn);
		case 3:
			return THREE_IN_A_ROW(board, 4, 5, turn) || THREE_IN_A_ROW(board, 10, 18, turn);
		case 4:
			return THREE_IN_A_ROW(board, 1, 7, turn) || THREE_IN_A_ROW(board, 3, 5, turn);
		case 5:
			return THREE_IN_A_ROW(board, 4, 3, turn) || THREE_IN_A_ROW(board, 13, 20, turn);
		case 6:
			return THREE_IN_A_ROW(board, 7, 8, turn) || THREE_IN_A_ROW(board, 11, 15, turn);
		case 7:
			return THREE_IN_A_ROW(board, 1, 4, turn) || THREE_IN_A_ROW(board, 6, 8, turn);
		case 8:
			return THREE_IN_A_ROW(board, 7, 6, turn) || THREE_IN_A_ROW(board, 12, 17, turn);
		case 9:
			return THREE_IN_A_ROW(board, 0, 21, turn) || THREE_IN_A_ROW(board, 10, 11, turn);
		case 10:
			return THREE_IN_A_ROW(board, 9, 11, turn) || THREE_IN_A_ROW(board, 3, 18, turn);
		case 11:
			return THREE_IN_A_ROW(board, 9, 10, turn) || THREE_IN_A_ROW(board, 6, 15, turn);
		case 12:
			return THREE_IN_A_ROW(board, 8, 17, turn) || THREE_IN_A_ROW(board, 13, 14, turn);
		case 13:
			return THREE_IN_A_ROW(board, 12, 14, turn) || THREE_IN_A_ROW(board, 5, 20, turn);
		case 14:
			return THREE_IN_A_ROW(board, 12, 13, turn) || THREE_IN_A_ROW(board, 2, 23, turn);
		case 15:
			return THREE_IN_A_ROW(board, 6, 11, turn) || THREE_IN_A_ROW(board, 16, 17, turn);
		case 16:
			return THREE_IN_A_ROW(board, 15, 17, turn) || THREE_IN_A_ROW(board, 19, 22, turn);
		case 17:
			return THREE_IN_A_ROW(board, 15, 16, turn) || THREE_IN_A_ROW(board, 8, 12, turn);
		case 18:
			return THREE_IN_A_ROW(board, 3, 10, turn) || THREE_IN_A_ROW(board, 19, 20, turn);
		case 19:
			return THREE_IN_A_ROW(board, 18, 20, turn) || THREE_IN_A_ROW(board, 16, 22, turn);
		case 20:
			return THREE_IN_A_ROW(board, 18, 19, turn) || THREE_IN_A_ROW(board, 5, 13, turn);
		case 21:
			return THREE_IN_A_ROW(board, 0, 9, turn) || THREE_IN_A_ROW(board, 22, 23, turn);
		case 22:
			return THREE_IN_A_ROW(board, 16, 19, turn) || THREE_IN_A_ROW(board, 21, 23, turn);
		case 23:
			return THREE_IN_A_ROW(board, 21, 22, turn) || THREE_IN_A_ROW(board, 2, 14, turn);
		default:
			return FALSE;
		}
	}
	return FALSE;
}

// given new board, slots to compare.  if slots all same, then it's a 3
//BOOLEAN THREE_IN_A_ROW(char *board, int slot1, int slot2, char turn) {
//	return board[slot1] == turn && board[slot2] == turn;
//}

void changeToThree() {
	gameType = 3;
	BOARDSIZE = 9;
	maxx = 3;
	maxo = 3;
	minb = 3;
	maxb = 4;
	totalPieces = maxx + maxo;
	kDBName = "3mm";
	adjacent = adjacent3;
	symmetriesToUse = gSymmetryMatrix3MM;
	totalNumSymmetries = 8;
}

void changeToSix() {
	gameType = 6;
	BOARDSIZE =16;
	maxx = 6;
	maxo = 6;
	minb = 4;
	maxb = 11;
	totalPieces = maxx + maxo;
	kDBName = "6mm";
	adjacent = adjacent6;
	symmetriesToUse = gSymmetryMatrix6MM;
	totalNumSymmetries = 16;
}

void changeToNine() {
	gameType = 9;
	BOARDSIZE = 24;
	maxx = 9;
	maxo = 9;
	minb = 6;
	maxb = 19;
	totalPieces = maxx + maxo;
	kDBName = "9mm";
	adjacent = adjacent9;
	symmetriesToUse = gSymmetryMatrix9MM;
	totalNumSymmetries = 16;
}


/**************************************************/
/************ SYMMETRY FUNCTIONS BEGIN ************/
/**************************************************/

//SYMMETRIES
BOOLEAN kSupportsSymmetries = TRUE;

/************************************************************************
**
** NAME:        GetCanonicalPosition
**
** DESCRIPTION: Go through all of the positions that are symmetrically
**              equivalent and return the SMALLEST, which will be used
**              as the canonical element for the equivalence set.
**
** INPUTS:      POSITION position : The position return the canonical elt. of.
**
** OUTPUTS:     POSITION          : The canonical element of the set.
**
************************************************************************/

POSITION GetCanonicalPosition(POSITION position) {
	if (isIntermediate(position)) {
		return position;
	}
	char turn;
	int piecesLeft, numX, numO;
	char *originalBoard = unhash(position, &turn, &piecesLeft, &numX, &numO);
	char canonBoard[24];
    POSITION canonPos = position;
    int bestSymmetryNum = 0;

    for (int symmetryNum = 1; symmetryNum < totalNumSymmetries; symmetryNum++)
        for (int i = BOARDSIZE - 1; i >= 0; i--) {
            char pieceInSymmetry = originalBoard[symmetriesToUse[symmetryNum][i]];
            char pieceInBest = originalBoard[symmetriesToUse[bestSymmetryNum][i]];
            if (pieceInSymmetry != pieceInBest) {
                if (pieceInSymmetry > pieceInBest) // If new smallest hash.
                    bestSymmetryNum = symmetryNum;
                break;
            }
        };

    if (bestSymmetryNum == 0) {
		SafeFree(originalBoard);
        return position;
	}
    
    for (int i = 0; i < BOARDSIZE; i++) // Transform the rest of the board.
        canonBoard[i] = originalBoard[symmetriesToUse[bestSymmetryNum][i]];

    canonPos = hash(canonBoard, turn, piecesLeft, numX, numO);
	
	SafeFree(originalBoard);

    return canonPos;
}

/************************************************************************
**
** NAME:        DoSymmetry
**
** DESCRIPTION: Perform the symmetry operation specified by the input
**              on the position specified by the input and return the
**              new position, even if it's the same as the input.
**
** INPUTS:      POSITION position : The position to branch the symmetry from.
**              int      symmetry : The number of the symmetry operation.
**
** OUTPUTS:     POSITION, The position after the symmetry operation.
**
************************************************************************/

/**************************************************/
/************* SYMMETRY FUNCTIONS END *************/
/**************************************************/

/************************************************************************
** Changelog
**
** $Log: not supported by cvs2svn $
** Revision 1.3  2008/05/08 02:28:12  noafroboy
** latest version
**
** Revision 1.2  2008/05/01 03:44:52  noafroboy
** added the three variants for removing pieces after forming mills
**
** Revision 1.13  2008/05/01 02:03:56  noafroboy
** latest m6mm.c with some bugs fixed... see wiki for details
**
** Revision 1.12  2008/04/29 08:42:30  noafroboy
** fixed the tcl so that 3mm and 6mm work. 9mm works for first 12 moves. possible database error
**
** Revision 1.11  2008/04/29 07:18:43  noafroboy
** added 3mm. starts with 3mm by default for now.
**
** Revision 1.10  2008/03/18 02:34:09  noafroboy
** This version is for the Stanford Demonstration. 9mm has been removed. In addition, gCurrentTier is set each time DoMove is called in order for the GUI to know the phase it is in.
**
** Revision 1.9  2007/11/29 05:00:29  noafroboy
** temporary hack for m6mm.c
**
** Revision 1.8  2007/11/26 10:41:26  noafroboy
** BUGZID:
** 1.  Added 9 men's morris under game specific options.
** 2.  Polished PrintPosition for 9mm board.
** 3.  Slightly edited SetupTierStuff to accomodate above changes.
**
** Revision 1.7  2007/11/19 04:32:44  ddgarcia
** Sooo many changes:
**
** 1. Added gFlying (global, changed GenerateMoves, added getOption, setoption)
**   (now variants are 1-4 ; before they were 0, default is no flying)
** 2. Supported misere with gStandardGame in Primitive
** 3. polished PrintPosition
** 4. Filled in PrintComputersMove
** 5. Removed [ and ] from single moves in MoveToString
** 6. Cleaned GetAndPrintPlayersMove
**
** Revision 1.6  2007/11/07 03:37:39  patricia_fong
** fixed a small part in MoveToString
**
** Revision 1.5  2007/10/17 10:06:07  patricia_fong
** added functions to m6mm.c for tcl
**
** Revision 1.3  2006/12/19 20:00:50  arabani
** Added Memwatch (memory debugging library) to gamesman. Use 'make memdebug' to compile with Memwatch
**
** Revision 1.2  2006/12/07 03:37:20  max817
** Hash and NMM changes.
**
** Revision 1.10  2006/04/25 01:33:06  ogren
** Added InitialiseHelpStrings() as an additional function for new game modules to write.  This allows dynamic changing of the help strings for every game without adding more bookkeeping to the core.  -Elmer
**
**
   Tier
   1st stage: number of stones for x left, and o, position.
   2nd stage: pieces on the board of each player
   if numx and numo == 9 and piecesleft != 0, then stage 1
   if numx and numo <= 9 and piecesleft == 0, then stage 2

Changes:
   unhash is now
   char* unhash(POSITION pos, char* turn, int* piecesLeft, int* numx, int* numo)
   with piecesLeft, numx, and numo being globals until tiers are implemented.

   hash is now
   POSITION hash(char* board, char turn, int piecesLeft, int numx, int numo)

   with piecesLeft, numx, and numo being globals until tiers are implemented.

   find_adjacent is ready for 6mm.

   ConvertTextInputToMove: need to check:
   IN STAGE 1:
 * 1 number -placing piece
               - [1] ~ board[1] = turn;
 * 2 numbers - placing piece and removing opponent's piece
                        - [1 2] ~ board[1] = turn; board[2] = blank;


   IN STAGE 2:
 * 2 numbers - moving from and to
                - [1 2] ~ board[1] = blank; board[2] = turn <OPPOSITE OF STAGE 1!>
 * 3 numbers - moving from and to and removing opponent's piece
                        - [1 2 3] ~ board[1] = blank; board[2] = turn; board[3] = blank;


   please kill ValidTextInput until we're done with this. Not worth spending time
   on this right now.

   KEVIN: 10/28
        - updated whoseMove to generic_hash_turn
                      generic_hash to generic_hash_hash
                      generic_unhash to generic_hash_unhash
                          because Max changed the hash core to those names
        -fixed a few overlooked bugs: not passing turn, piecesleft, numx, and numo arguments by reference when calling unhash
        -added NUMX and NUMO globals because you forgot
   PrintPosition is ready for 6mm.
        looks good
   Primitive: nothing has been changed for 6mm... should work for 6mm
        looks good
   GENERATEMOVES NEEDS TO BE LOOKED OVER
        pseudocode written, it looks like we will need to copy a few more of their helper functions over. doing it tomorrow. should be quick.
   DOMOVE NEEDS TO BE LOOKED OVER
        finished
   MOVETOSTRING NEEDS TO BE CHANGED SO THAT IT DOESNT INCLUDE REMOVE NUMBERS IF
   THERE IS NOTHING TO REMOVE.
        this was already taken care of previously


   we should use the one liner functions from, to, and remove.
        it makes the code a whole lot easier to read and reduces likely hood of errors... such as the mistake in DoMove earlier
 ************************************************************************/

/// BEGIN ALL INTERACT FUNCTIONS ///

POSITION GetInitialPosition() {
	if (gIsInteract) {
		gInitializeHashWindow(1800, FALSE);
	}
	return 0;
}

BOOLEAN isIntermediate(POSITION pos) {
	return pos >> 63;
}

MOVELIST *InteractGenerateMoves(POSITION position) {
	MOVELIST *moves = NULL;
	char turn;
	int piecesLeft, numX, numO;
	char *board = unhash(position, &turn, &piecesLeft, &numX, &numO);
	int legalRemoves[BOARDSIZE];
	int numLegalRemoves = findLegalRemoves(board, turn, legalRemoves);
	int *legalTos;
	int numLegalTos;

	if (piecesLeft == 0 && (numX < 3 || numO < 3)) return NULL;

	int allBlanks[BOARDSIZE];
	int numBlanks = 0;
	for (int i = 0; i < BOARDSIZE; i++)
		if (board[i] == BLANK)
			allBlanks[numBlanks++] = i;

	if (isIntermediate(position)) {
		for (int j = 0; j < numLegalRemoves; j++)
			moves = CreateMovelistNode(MOVE_ENCODE(31, 31, legalRemoves[j]), moves);
	} else {
		if (piecesLeft > 0) {
			for (int i = 0; i < numBlanks; i++)
				moves = CreateMovelistNode(MOVE_ENCODE(31, allBlanks[i], 31), moves);
		} else {
			for (int fromIdx = 0; fromIdx < BOARDSIZE; fromIdx++) {
				if (gFlying && ((turn == X && numX <= 3) || (turn == O && numO <= 3))) {
					legalTos = allBlanks;
					numLegalTos = numBlanks;
				} else {
					legalTos = adjacent[fromIdx];
					numLegalTos = adjacent[fromIdx][4];
				}
				if (board[fromIdx] == turn) {
					for (int i = 0; i < numLegalTos; i++) {
						if (board[legalTos[i]] != BLANK) continue;
						moves = CreateMovelistNode(MOVE_ENCODE(fromIdx, legalTos[i], 31), moves);
					}
				}
			}
		}
	}

	SafeFree(board);
	return moves;
}

POSITION InteractDoMove(POSITION position, MOVE move) {
	// Slide
	// Remove
	// Place
	POSITION intermediateMarker = 0;

	char turn;
	int piecesLeft;
	int numX, numO;
	char* board = unhash(position, &turn, &piecesLeft, &numX, &numO);

	char newTurn = (turn == X) ? O : X;

	int fromIdx = move >> 10;
	int toIdx = (move >> 5) & 0x1F;
	int removeIdx = move & 0x1F;

	if (fromIdx != 31 && toIdx != 31) {
		board[toIdx] = turn;
		board[fromIdx] = BLANK;
	} else if (removeIdx != 31) {
		board[removeIdx] = BLANK;
		if (turn == X) {
			numO--;
		} else {
			numX--;
		}
	} else {
		board[toIdx] = turn;
		if (turn == X) {
			numX++;
		} else {
			numO++;
		}
		piecesLeft--;
	}
	
	if (!isIntermediate(position)) {
		int legalRemoves[BOARDSIZE];
		int numLegalRemoves = findLegalRemoves(board, turn, legalRemoves);
		if (numLegalRemoves > 0 && closesMillNew(board, turn, 31, toIdx)) {
			intermediateMarker = (1UL << 63);
			newTurn = turn; // Remain in the same turn.
		}
	}

	//printf("Before Hash: %s, %c, %d, %d, %d\n", board, newTurn, piecesLeft, numX, numO);
	POSITION toReturn = hash(board, newTurn, piecesLeft, numX, numO) + intermediateMarker;
	//: %llu\n", toReturn);

	SafeFree(board);

	return toReturn;
}

VALUE InteractGetValue(POSITION pos) {
	// Return regular value unless pos is an intermediate positon.
	// In which case, return best outcome of all children.
	if (isIntermediate(pos)) {
		MOVELIST *ml = InteractGenerateMoves(pos);
		MOVELIST *mlp = ml;
		BOOLEAN existsWin = FALSE;
		BOOLEAN existsTie = FALSE;
		for (; ml != NULL; ml = ml->next) {
			POSITION childpos = InteractDoMove(pos, ml->move);
			VALUE childVal = oldvaluegetter(GetCanonicalPosition(childpos));
			if (childVal == lose) {
				existsWin = TRUE;
				break;
			} else if (childVal == tie) {
				existsTie = TRUE;
			}
		}
		FreeMoveList(mlp);
		return (existsWin) ? win : (existsTie) ? tie : lose;
	} else {
		return oldvaluegetter(pos);
	}
}

REMOTENESS InteractGetRemoteness(POSITION pos) {
	// Return regular remoteness unless pos is an intermediate position.
	// In which case, if pos==win, return smallest win remoteness
	// else if pos==lose, return largest lose remoteness
	// else return MAX_REMOTENESS
	if (isIntermediate(pos)) {
		VALUE val = InteractGetValue(pos);
		if (val == win) {
			REMOTENESS smallestRemoteness = REMOTENESS_MAX;
			MOVELIST *ml = InteractGenerateMoves(pos);
			MOVELIST *mlp = ml;
			for (; ml != NULL; ml = ml->next) {
				POSITION childpos = InteractDoMove(pos, ml->move);
				VALUE childVal = oldvaluegetter(GetCanonicalPosition(childpos));	
				if (childVal == lose) {
					REMOTENESS childRem = oldremotenessgetter(GetCanonicalPosition(childpos));
					smallestRemoteness = (childRem < smallestRemoteness) ? childRem : smallestRemoteness;
				}
			}
			FreeMoveList(mlp);
			return smallestRemoteness + 1;
		} else if (val == lose) {
			REMOTENESS largestRemoteness = 0;
			MOVELIST *ml = InteractGenerateMoves(pos);
			MOVELIST *mlp = ml;
			for (; ml != NULL; ml = ml->next) {
				POSITION childpos = InteractDoMove(pos, ml->move);
				REMOTENESS childRem = oldremotenessgetter(GetCanonicalPosition(childpos));
				largestRemoteness = (childRem > largestRemoteness) ? childRem : largestRemoteness;
			}
			FreeMoveList(mlp);
			return largestRemoteness + 1;
		} else {
			return REMOTENESS_MAX;
		}
	} else {
		return oldremotenessgetter(pos);
	}
}

POSITION InteractStringToPosition(STRING board) {
	POSITION intermediateMarker = (board[10] == 'R') ? (1UL << 63) : 0;
	char realBoard[BOARDSIZE];
	char turn = (board[turnIndex9mmInteractString] == 'A') ? X : O;
	int numX = 0;
	int numO = 0;
	
	for (int i = 0; i < BOARDSIZE; i++) {
		char piece = board[indexMap9mmInteractString[i]];
        if (piece == 's' || piece == '-') {
			realBoard[i] = BLANK;
    	} else {
			if (piece == X) {
				numX++;
			} else {
				numO++;
			}
		    realBoard[i] = piece;
        }
	}

	int piecesLeft = (board[remainingXIndex9mmInteractString] - '0') + (board[remainingOIndex9mmInteractString] - '0');

	gInitializeHashWindow(piecesLeft * 100 + numX * 10 + numO, FALSE);
	POSITION toReturn = hash(realBoard, turn, piecesLeft, numX, numO) + intermediateMarker;
	return toReturn;
}

STRING InteractPositionToString(POSITION pos) {

	char* finalBoard = calloc(65, sizeof(char));
	memcpy(finalBoard, initial9mmInteractString, 64);

	char turn;
	int piecesLeft, numX, numO;
	//printf("Before Unhash: %llu\n", pos);
	char* board = unhash(pos, &turn, &piecesLeft, &numX, &numO);
	//printf("After Unhash: %s, %c, %d, %d, %d\n", board, turn, piecesLeft, numX, numO);

	if (isIntermediate(pos)) {
		for (int i = 9; i <= 13; i++) {
			finalBoard[i] = 'R';
		}
		pos -= (1UL << 63);
	}
	TIER tier;
	TIERPOSITION tierPosition;
	gUnhashToTierPosition(pos, &tierPosition, &tier);
	for (int i = 0; i < BOARDSIZE; i++) {
        if (board[i] != BLANK) {
		    finalBoard[indexMap9mmInteractString[i]] = board[i];
        }
	}
	SafeFree(board);

	finalBoard[turnIndex9mmInteractString] = (turn == X) ? 'A' : 'B';
	finalBoard[remainingXIndex9mmInteractString] = ((tier / 100) / 2) + '0';
	finalBoard[remainingOIndex9mmInteractString] = (((tier / 100) + 1) / 2) + '0';

	return finalBoard;
}

STRING InteractPositionToEndData(POSITION pos) {
	return NULL;
}

STRING InteractMoveToString(POSITION pos, MOVE move) {
	// Move will be of the form:
	// from, to (for sliding only)
	// remove (for removal only)
	// to (for placement only)
	int from = move >> 10;
	int to = (move >> 5) & 0x1F;
	int remove = move & 0x1F;
	char turn;
	int piecesLeft, numX, numO;
	char* board = unhash(pos, &turn, &piecesLeft, &numX, &numO);
	SafeFree(board);

	if (from != 31 && to != 31) {
		return UWAPI_Board_Regular2D_MakeMoveString(indexMap9mmInteractString[from], indexMap9mmInteractString[to]);
	} else if (remove != 31) {
		return UWAPI_Board_Regular2D_MakeAddString((turn == X) ? 'o' : 'x', indexMap9mmInteractString[remove]);
	} else if (to != 31) {
		return UWAPI_Board_Regular2D_MakeAddString(turn, indexMap9mmInteractString[to]);
	} else {
		return NULL;
	}
}