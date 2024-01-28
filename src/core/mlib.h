#ifndef _MLIB_H_
#define _MLIB_H_

// Include vendored libUWAPI for board string formatting
#include "../../extern/libUWAPI/UWAPI_boardstrings.h"

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
	\
	static char gBlankOXUWAPIChar[] = { '-', 'o', 'x' }; \
	\
	POSITION InteractStringToPosition(STRING str) { \
		unsigned int num_rows, num_columns; \
		STRING board; \
		\
		if (!UWAPI_Board_Regular2D_ParsePositionString(str, NULL, &num_rows, &num_columns, &board)) { \
			return INVALID_POSITION; \
		} \
 		\
		BlankOX oxboard[BOARDSIZE]; \
		int i; \
		for (i = 0; i < BOARDSIZE; i++) { \
			if (board[i] == 'o') { \
				oxboard[i] = o; \
			} else if (board[i] == 'x') { \
				oxboard[i] = x; \
			} else if (board[i] == '-') { \
				oxboard[i] = Blank; \
			} else { \
				SafeFreeString(board); \
				return INVALID_POSITION; \
			} \
		} \
		SafeFreeString(board); \
		return BlankOXToPosition(oxboard); \
	} \
	\
	STRING InteractPositionToString(POSITION pos) { \
		BlankOX oxboard[BOARDSIZE]; \
		PositionToBlankOX(pos, oxboard); \
		\
		char board[BOARDSIZE + 1]; \
		int i; \
		for (i = 0; i < BOARDSIZE; i++) { \
			board[i] = gBlankOXUWAPIChar[oxboard[i]]; \
		} \
		board[BOARDSIZE] = '\0'; \
		\
		return UWAPI_Board_Regular2D_MakeBoardString( \
			WhoseTurn(oxboard) == x ? UWAPI_TURN_A : UWAPI_TURN_B, \
			BOARDROWS * BOARDCOLS, board); \
	} \
	\
	STRING InteractMoveToString(POSITION pos, MOVE mv) { \
		BlankOX oxboard[BOARDSIZE]; \
		PositionToBlankOX(pos, oxboard); \
		return UWAPI_Board_Regular2D_MakeAddString(gBlankOXUWAPIChar[WhoseTurn(oxboard)], mv); \
	}

#endif
