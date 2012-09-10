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

#endif
