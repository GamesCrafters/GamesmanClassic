#include <stdio.h>
#include <stdlib.h>


/************************************************************************
***********************************************************************/

void freeAll();
int dartboard_condition(int *t);
int validConfig(int *t);
int searchIndices(int s);
int searchOffset(int h);
int combiCount(int* tc);
void printStats ();
void printAr(int* a);
void printcAr(char* a);
int hash_cruncher (char* board, int boardsize, int* thiscount, int* local_mins);
void hash_uncruncher (int hashed, int boardsize, int* thiscount, char* dest, int* local_mins);

int generic_hash_init(int boardsize, int*, int (*fn)(int *));
int generic_hash(char* board, int player);
char* generic_unhash(int hashed, char* dest);
int whoseMove (int hashed);

void dartboard_hash_init(int boardsize, int minOs, int maxOs, int minXs, int maxXs);
int dartboard_hash(char* board);
char* dartboard_unhash(int hashed, char* dest);

void rearranger_hash_init(int boardsize, int numOs, int numXs);
int rearranger_hash(char* board);
char* rearranger_unhash(int hashed, char* dest);

int fac(int n);
int nCr(int n, int r);
void nCr_init(int a);
int* gpd (int n);
int gpi (int n);
int* gPieceDist (int i);



