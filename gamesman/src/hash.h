#include <stdio.h>
#include <stdlib.h>
#include "gamesman.h"

/************************************************************************
***********************************************************************/
POSITION *hash_hashOffset;
POSITION *hash_NCR;
POSITION *hash_miniOffset;
char *hash_pieces;
int *hash_localMins;
int *hash_thisCount;
int *hash_miniIndices;
int hash_numPieces;
int hash_boardSize;
int hash_usefulSpace;
int hash_maxPos;

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

#define cCon (contextList)
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
void printStats ();
void printAr(int* a);
void printcAr(char* a);
void clearAr (int* a, int n);
POSITION hash_cruncher (char* board);
void hash_uncruncher (POSITION hashed, char *dest);
POSITION generic_hash_init(POSITION boardsize, int*, int (*fn)(int *));
POSITION generic_hash(char* board, int player);
char* generic_unhash(POSITION hashed, char* dest);
int whoseMove (POSITION hashed);

int nCr(int n, int r);
void nCr_init(int a);
int* gpd (int n);
int gpi (int n);
int* gPieceDist (int i);



