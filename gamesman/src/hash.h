#include <stdio.h>
#include <math.h>

/************************************************************************
***********************************************************************/


//globals
int *gHashOffset;
int *gNCR;
int gHashOffsetSize;
int gHashBoardSize;
int gHashMinMax[4];
int gHashNumberOfPos;

//the next pairs of numbers each defines a min and max number of pieces
//for pieces of that type
//functions

void generic_hash_init(int boardsize, int minOs, int maxOs, int minXs, int maxXs);
int generic_hash(char* board);
char* generic_unhash(int hashed, char* dest);

void dartboard_hash_init(int boardsize, int minOs, int maxOs, int minXs, int maxXs);
int dartboard_hash(char* board);
char* dartboard_unhash(int hashed, char* dest);

void rearranger_hash_init(int boardsize, int numOs, int numXs);
int rearranger_hash(char* board);
char* rearranger_unhash(int hashed, char* dest);

int nCr(int n, int r);
void nCr_init();

