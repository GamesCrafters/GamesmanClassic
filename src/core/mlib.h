#ifndef _MLIB_H_
#define _MLIB_H_

/*function prototypes*/

void LibInitialize(int,int,int,BOOLEAN);
BOOLEAN NinaRow(void*,void*,int,int);
BOOLEAN statelessNinaRow(void*,void*,int);
BOOLEAN amountOfWhat(void*,void*,int,BOOLEAN);
BOOLEAN OneDMatch(void*,void*,BOOLEAN*,BOOLEAN,int);
BOOLEAN TwoDMatch(void*,void*);
void Test();

BOOLEAN mymemcmp(void*,void*,int);

//watch out for collisions of these constant names. Always prepend the module name before them.
#define MLIB_MAXBOARDSIZE 64
#define MLIB_MAXPATTERN 16

typedef struct lb {
	int eltSize;
	int rows;
	int cols;
	int size;
	BOOLEAN diagonals;
	void* blankPiece;
	void* opponentPlayerPiece;
	void* initialPlayerPiece;
	int directionMap[8];
	int scratchBoard[MLIB_MAXBOARDSIZE];
	int overflowBoards[MLIB_MAXPATTERN][MLIB_MAXBOARDSIZE];
} LocalBoard;

extern LocalBoard lBoard;

#define GM_DEFINE_BLANKOX_ENUM_BOARDSTRINGS() \
	POSITION StringToPosition(char* boardStr) { \
		BlankOX board[BOARDSIZE]; \
		int i; \
		if (strlen(boardStr) != BOARDSIZE) { \
			return -1; \
		} \
		for (i = 0; i < BOARDSIZE; i++) { \
			if (boardStr[i] == 'o') { \
				board[i] = o; \
			} \
			else if (boardStr[i] == 'x') { \
				board[i] = x; \
			} \
			else if (boardStr[i] == ' ') { \
				board[i] = Blank; \
			} \
			else { \
				return INVALID_POSITION; \
			} \
		} \
		return BlankOXToPosition(board); \
	} \
	char* PositionToString(POSITION pos) { \
		BlankOX theBlankOX[BOARDSIZE]; \
		int i; \
		char * board = (char*) SafeMalloc(BOARDSIZE + 1); \
		memset(board, 0, BOARDSIZE + 1); \
		PositionToBlankOX(pos, theBlankOX); \
		for (i = 0; i < BOARDSIZE; i++) { \
			board[i] = *gBlankOXString[(int)theBlankOX[i]]; \
		} \
		return board; \
	}

#endif
