
#ifndef GM_HASH_H
#define GM_HASH_H

#include <stdio.h>

#ifdef	HASH_LOCAL
#define	H_EXTERN
#else
#define	H_EXTERN	extern
#endif


/************************************************************************
***********************************************************************/

/* debugging flag */
H_EXTERN int DEBUG, DEBUG2, DEBUG3, DEBUG4;

/*globals*/


extern int* gNCR;		/* These two are already in gamesman.c */
extern int* gHashOffset;

H_EXTERN int TERM, cTERM, MAX_PIECES, MAX_MAXES, *gOffsetIndices;
H_EXTERN int *gFACTORIAL, *gpd_store, *newcount, *thiscount, *local_mins;
H_EXTERN int *miniOffset, *miniIndices, *gPieceIndices;
/* H_EXTERN int **gPieceDist; */
H_EXTERN char *pieces;
H_EXTERN int *mins, *maxs, *nums, num_pieces, num_cfgs, useful_space, gHashOffsetSize;
H_EXTERN int gHashBoardSize, gHashNumberOfPos;

void	freeAll();
int	dartboard_condition(int *t);
int	validConfig(int *t);
int	searchIndices(int s);
int	searchOffset(int h);
int	combiCount(int* tc);
void	printStats ();
void	printAr(int* a);
void	printcAr(char* a);
int	hash_cruncher (char* board, int boardsize, int* thiscount, int* local_mins);
void	hash_uncruncher (int hashed, int boardsize, int* thiscount, char* dest, int* local_mins);
int	getPieceParams (int pa[], char *pi, int *mi, int *ma);

int	generic_hash_init(int boardsize, int[], int (*fn)(int *));
int	generic_hash(char* board);
char*	generic_unhash(int hashed, char* dest);

void	dartboard_hash_init(int boardsize, int minOs, int maxOs, int minXs, int maxXs);
int	dartboard_hash(char* board);
char*	dartboard_unhash(int hashed, char* dest);

void	rearranger_hash_init(int boardsize, int numOs, int numXs);
int	rearranger_hash(char* board);
char*	rearranger_unhash(int hashed, char* dest);

int	fac(int n);
int	nCr(int n, int r);
void	nCr_init(int a);
int*	gpd (int n);
int	gpi (int n);
int*	gPieceDist (int i);



#endif /* GM_HASH_H */
