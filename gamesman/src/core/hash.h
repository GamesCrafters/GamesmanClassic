#ifndef __HASH_H__
#define __HASH_H__


#include <stdio.h>
#include <stdlib.h>
#include "gamesman.h"

/************************************************************************
***********************************************************************/
/* DDG Tried to fix build by externing in hash.h, declaring in hash.c */
/* 2004-10-20 */
extern POSITION *hash_hashOffset;
extern POSITION *hash_NCR;
extern POSITION *hash_miniOffset;
extern char *hash_pieces;
extern int *hash_localMins;
extern int *hash_thisCount;
extern int *hash_miniIndices;
extern int hash_numPieces;
extern int hash_boardSize;
extern int hash_usefulSpace;
extern int hash_maxPos;

struct hashContext {

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
  
  
  BOOLEAN init;
  int contextNumber;
};

#define cCon (*contextList)
#define combiHash(x,y,z) ((x*y)+z)
#define combiUnM(x,y)  (x%y)
#define combiUnD(x,y) (x/y)

int generic_hash_context_init();
void generic_hash_context_switch(int context);
void hash_combiCalc();

void freeHashContext();
int validConfig(int *t);
int searchIndices(int s);
int searchOffset(int h);
POSITION combiCount(int* tc);
int hash_countPieces(int *pA);
POSITION hash_cruncher (char* board);
void hash_uncruncher (POSITION hashed, char *dest);
POSITION generic_hash_init(int boardsize, int*, int (*fn)(int *));
POSITION generic_hash(char* board, int player);
char* generic_unhash(POSITION hashed, char* dest);
int whoseMove (POSITION hashed);
int getPieceParams(int *pa,char *pi,int *mi,int *ma);

int nCr(int n, int r);
void nCr_init(int a);
int* gpd (int n);
int gpi (int n);
int* gPieceDist (int i);

#endif

