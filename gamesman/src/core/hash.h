#ifndef GMCORE_HASH_H
#define GMCORE_HASH_H

#include "gamesman.h"

/************************************************************************
***********************************************************************/
/* DDG Tried to fix build by externing in hash.h, declaring in hash.c */
/* 2004-10-20 */
//extern POSITION *hash_hashOffset;
//extern POSITION *hash_NCR;
//extern POSITION *hash_miniOffset;
//extern char *hash_pieces;
//extern int *hash_localMins;
//extern int *hash_thisCount;
//extern int *hash_miniIndices;
//extern int hash_numPieces;
//extern int hash_boardSize;
//extern int hash_usefulSpace;
//extern int hash_maxPos;

struct hashContext
{

        POSITION *hashOffset;
        POSITION maxPos;
        POSITION *combiArray;
        POSITION *NCR;
        int *gpdStore;
        int *offsetIndices;
        int *pieceIndices;
        int boardSize;

        int numCfgs;
        int usefulSpace;

        int numPieces;
        char *pieces;
        int *nums;
        int *mins;
        int *maxs;

        int *thisCount;
        int *localMins;

        POSITION *miniOffset;
        int *miniIndices;

        int (*gfn)(int *);

        int player;		// 0=Both Player boards (default), 1=1st Player only, 2=2nd only

        //int contextNumber;
};

int generic_hash_context_init();
void generic_hash_context_switch(int context);
void generic_hash_destroy();
int generic_hash_cur_context();
POSITION generic_hash_max_pos();

POSITION generic_hash_init(int boardsize, int*, int (*fn)(int *), int player);
POSITION generic_hash_hash(char* board, int player);
char* generic_hash_unhash_tcl(POSITION pos);
char* generic_hash_unhash(POSITION hashed, char* dest);
int generic_hash_turn (POSITION hashed);
void hashCounting(void);

#endif /* GMCORE_HASH_H */

