#include "hash.h"
#include <stdlib.h>


/*********************************************************************************
*** A *PERFECT* hash function -
*** - User-specified variables version designed and implemented by Michel D'Sa
*** - Original (3-variable only) version designed by Dan Garcia
***   and implemented by Attila Gyulassy
*********************************************************************************/

int DEBUG = 0;
int DEBUG2 = 0;
int DEBUG3 = 1;
int DEBUG4 = 1;
int TERM = -1;
int cTERM = 0;
int MAX_PIECES = 50;
int MAX_MAXES = 50;
int (*gfn)(int *);

/* frees up memory when hash/unhash is no longer needed */
void freeAll()
{
    if (DEBUG2) printf("-=0");
    free(newcount); if (DEBUG2) printf("a");
    free(thiscount); if (DEBUG2) printf("b");
    free(local_mins); if (DEBUG2) printf("c");
    free(miniOffset); if (DEBUG2) printf("d");
    free(miniIndices); if (DEBUG2) printf("e");
	free(gHashOffset); if (DEBUG2) printf("1");
	free(gOffsetIndices);  if (DEBUG2) printf("2");
	free(gNCR);    if (DEBUG2) printf("3");
	free(gFACTORIAL);  if (DEBUG2) printf("4");
    free(gpd_store); if (DEBUG2) printf("5");
	free(gPieceIndices);   if (DEBUG2) printf("6");
	free(pieces);    if (DEBUG2) printf("7");
	free(mins);    if (DEBUG2) printf("8");
	free(maxs);   if (DEBUG2) printf("9");
	free(nums);     if (DEBUG2) printf("f=-");
}

/* determines if a piece distribution is valid or not.
   t[i] denotes the number of occurrences of the ith piece in this cofiguration
   */
int validConfig(int *t)
{
	int cond = (gfn == NULL)? 1 : gfn(t);
	return cond;
}


/* function used to map a member of gHashOffset into gOffsetIndices */
int searchIndices(int s)
{
	int i = useful_space;
	while(gOffsetIndices[i] > s) i--;
	return i;
}

/* function used to map a member of gOffsetIndices into gHashOffset */
int searchOffset(int h)
{
	int i = useful_space;
	while(gHashOffset[i] > h) i--;
	return i;
}

/* helper function used to find n choose (t1,t2,t3,...,tn) where the ti's are
   the members of *tc */
int combiCount(int* tc)
{
	int i = 0, sum = 0, prod = 1, ctr = 1, ind = 0;
     int* cumul_count;
     for (ctr = 0;tc[ctr] != TERM;ctr++);
     cumul_count = (int*) malloc ((ctr+1) * sizeof(int));
     ctr = 0;
	while (tc[ctr] != TERM)
        {
        sum += tc[ctr];
        cumul_count[ctr] = sum;
        ctr++;
        }
        cumul_count[ctr] = TERM;
    for (ind = 0;ind < ctr - 1;ind++)
    {
      prod *= nCr(cumul_count[ind + 1], cumul_count[ind]);
      }
      return prod;

}

/* debugging procedure to list critical variable values*/
void printStats ()
{
	int i = 0;
	printf("\n=======STATUS=======");
	printf("\n char Array pieces = "); printcAr(pieces);
	printf("\n int Array mins = "); printAr(mins);
	printf("\n int Array maxs = "); printAr(maxs);
	printf("\n int Array nums = "); printAr(nums);
	printf("\n int num_pieces = %d", num_pieces);
	printf("\n int num_cfgs = %d", num_cfgs);
	printf("\n int useful_space = %d", useful_space);
	printf("\n int Array gHashOffset = "); printAr(gHashOffset);
	printf("\n int Array gOffsetIndices = "); printAr(gOffsetIndices);
	printf("\n int Array gPieceIndices = "); printAr(gPieceIndices);
	for (i = 0;i < useful_space;i++)
	{
		printf("\n");
		printAr(gpd(gpi(i)));
	}
	printf("\n=======E-STAT=======\n");
}

/* shorthand for printing an int array */
void printAr(int* a)
{
	int i = 0, j = 0;
	while (a[i] != TERM) i++;
	printf("{ ");
	for (j = 0;j < i;j++)
	{
		printf("%d ", a[j]);
	}
	printf("}");
}

/* shorthand for printing a char array */
void printcAr(char* a)
{
	int i = 0, j = 0;
	while (a[i] != cTERM) i++;
	printf("{ ");
	for (j = 0;j < i;j++)
	{
		printf("%c ", a[j]);
	}
	printf("}");
}

/* helper func from generic_unhash() computes lexicographic rank of *board
   among boards with the same configuration argument *thiscount */
int hash_cruncher (char* board, int boardsize, int* thiscount, int* local_mins)
{
	int i = 0, sum = 0, k = 0, max1 = 0;

    if (DEBUG) printf("hash_cruncher 0.1\n");
	if (boardsize == 1) return 0;
    if (DEBUG) printf("hash_cruncher 0.2\n");
	for (i = 0; i < num_pieces + 1;i++)
	{
		newcount[i] = thiscount[i];
	}
    if (DEBUG) printf("hash_cruncher 0.5\n");
	newcount[num_pieces] = TERM;
	i = 0;
    if (DEBUG) printf("b[bs-1]=%c\n", board[boardsize - 1]);
	while (board[boardsize - 1] != pieces[i]) i++;
	for (k = 0;k < i;k++)
	{
        if (DEBUG) printf("\nhash_cruncher (loop.0) k=%d\n", k);
		max1 = 1;
		if (local_mins[k] > 1) max1 = local_mins[k];
        if (DEBUG) printf("hash_cruncher (loop.0) i=%d\n", i);
        if (DEBUG) printAr(newcount);
		if (newcount[k] >= max1)
		{
			newcount[k]--;
			sum += combiCount(newcount);
            if (DEBUG) printf("\nhash_cruncher (loop.1)\n");
            if (DEBUG) printAr(newcount);
			newcount[k]++;
		}
	}
    if (DEBUG) printf("hash_cruncher 1\n");
	newcount[i]--;
	local_mins[i]--;
    if (DEBUG) printf("hash_cruncher 2\n");
	return sum + hash_cruncher(board, boardsize - 1, newcount, local_mins);
}

/* helper func from generic_hash() computes a board, given its lexicographic rank hashed
   among boards with the same configuration argument *thiscount*/
void hash_uncruncher (int hashed, int boardsize, int* thiscount, char* dest, int* local_mins)
{
	int bigfac = fac(boardsize - 1);
	int sum = 0, i = 0, prod = 1, j = 0;
	int max1 = 0;
 if (DEBUG) printf("hash_uncruncher 0\n");

	if (boardsize == 1)
	{
		i = 0;
		while (thiscount[i] == 0) i++;
		dest[0] = pieces[i];
		return;
	}
	for (i = 0;i < boardsize;i++)
	{
		dest[i] = cTERM;
	}
	for (i = 0;i < num_pieces + 2;i++)
	{
		miniOffset[i] = TERM;
	}
	for (i = 0;i < num_pieces + 2;i++)
	{
		miniIndices[i] = TERM;
	}
	/*for (i = 0;i < num_pieces;i++)
	{
		sum += thiscount[i];
		prod *= fac(thiscount[i]);
	}        */
	for (i = 0; i < num_pieces;i++)
	{
		newcount[i] = thiscount[i];
	}
    newcount[num_pieces] = TERM;
	miniOffset[0] = 0;
	miniIndices[0] = 0;
	j = 1;
	for (i = 0; i < num_pieces;i++)
	{
		max1 = 1;
		if (local_mins[i] > 1) max1 = local_mins[i];
		if (newcount[i] >= max1)
		{
            newcount[i]--;
			miniOffset[j] = miniOffset[j-1] + combiCount(newcount);  // (bigfac*newcount[i])/prod;
            newcount[i]++;
			miniIndices[j] = i;
			j++;
		}
	}
	i = j-1;
	while (miniOffset[i-1] > hashed) i--;
	if (miniOffset[j-1] <= hashed) i = j-1;

	newcount[miniIndices[i]]--;
	local_mins[miniIndices[i]]--;
	dest[boardsize-1] = pieces[miniIndices[i]];
	hash_uncruncher(hashed - miniOffset[i-1], boardsize - 1, newcount, dest, local_mins);
}

/* parses the char* input to generic_hash_init() and assigns corresponding values to
   the pieces, minima, and maxima arrays*/
int getPieceParams (int *pa, char *pi, int *mi, int *ma)
{
	int i = 0;
	while (pa[i*3] != TERM)
	{
	pi[i] = (char)pa[i*3];
	mi[i] = pa[i*3 + 1];
	ma[i] = pa[i*3 + 2];
	i++;
	}
	pi[i] = cTERM;
	mi[i] = TERM;
	ma[i] = TERM;
	return i;
}
/**************************************************************************************************/
/* initializes hash tables and critical values */
int generic_hash_init(int boardsize, int *pieces_array, int (*fn)(int *))
{
	int i = -1, j, prod = 1, temp, sofar, k;
     gfn = fn;

         if (DEBUG) printf("generic_hash_init -1\n");

	pieces = (char*) malloc (sizeof(char) * MAX_PIECES);
	mins = (int*) malloc (sizeof(int) * MAX_PIECES);
	maxs = (int*) malloc (sizeof(int) * MAX_PIECES);
	nums = (int*) malloc (sizeof(int) * MAX_PIECES);
	num_pieces = getPieceParams(pieces_array, pieces, mins, maxs);
         if (DEBUG) printf("generic_hash_init -0.99\n");
    /*bad. should be local to the helper fcn's, but segfaults occur */
    newcount = (int*) malloc (sizeof(int) * (num_pieces + 1));
    thiscount = (int*) malloc (sizeof(int) * (num_pieces + 1));
    local_mins = (int*) malloc (sizeof(int) * (num_pieces + 1));
    miniOffset = (int*) malloc (sizeof(int) * (num_pieces + 2));
	miniIndices = (int*) malloc (sizeof(int) * (num_pieces + 2));
 	gFACTORIAL = (int*) malloc(sizeof(int) * (boardsize + 2));
	gPieceIndices = (int*) malloc (sizeof(int) * (num_cfgs + 2));
    gNCR = (int*) malloc(sizeof(int) * (boardsize + 1) * (boardsize + 1));

	gpd_store = (int*) malloc (sizeof(int) * (num_pieces + 1));
	gpd_store[num_pieces] = TERM;
	
 if (DEBUG) printf("generic_hash_init -0.9\n");
	for (i = 0; i < num_pieces;i++)
	{
		nums[i] = maxs[i] - mins[i] + 1;
	}
	nums[num_pieces] = TERM;

	num_cfgs = 1;
	for (i = 0; i < num_pieces;i++)
	{
		num_cfgs *= nums[i];
	}
		
	gHashBoardSize = boardsize;
	gHashOffsetSize = num_cfgs;

	nCr_init(boardsize);
    gHashOffset = (int*) malloc(sizeof(int) * (useful_space + 2));
	gOffsetIndices = (int*) malloc(sizeof(int) * (useful_space + 2));
             if (DEBUG) printf("generic_hash_init -0.8\n");
	for(i = 0; i < useful_space; i++)
	{
		gHashOffset[i] = TERM;
		gOffsetIndices[i] = TERM;
	}
	gHashOffset[0] = 0;
	gOffsetIndices[0] = 0;

 if (DEBUG) printf("generic_hash_init -0.7\n");
	for (k = 1;k < useful_space + 1;k++)
		{
			prod = 1;
			/*for (j = 0;j < num_pieces;j++)
			{
				prod *= fac(gPieceDist(k-1)[j]);
			}*/
			gHashOffset[k] = combiCount(gPieceDist(k-1)); //fac(boardsize)/prod;
			gOffsetIndices[k] = gpi(k-1);
		}
	gHashOffset[k] = TERM;
	gOffsetIndices[k] = TERM;
	
	if (DEBUG) printStats();
     if (DEBUG) printf("generic_hash_init -0.6\n");
	temp = 0;
	sofar = 0;
	for (i = 1; i < useful_space + 1; i++)
	{
		temp = gHashOffset[i];
		sofar += temp;
		gHashOffset[i] = sofar;
		gOffsetIndices[i]++;
	}	
	gHashOffset[useful_space + 1] = TERM;
	gHashNumberOfPos = sofar;

	if (DEBUG) printStats();
    if (DEBUG) printf("generic_hash_init 0\n");
	return sofar;
}

/* hashes *board to an int */
int generic_hash(char* board)
{
	int temp, i, j, sum;
	int boardsize;

    if (DEBUG) printf("generic_hash 0\n");
	for (i = 0;i < num_pieces;i++)
	{
		thiscount[i] = 0;
		local_mins[i] = mins[i];
	}
	boardsize = gHashBoardSize;

	for (i = 0; i < boardsize; i++)
	{
		for (j = 0; j < num_pieces; j++)
		{
			if (board[i] == pieces[j])
			{
				thiscount[j]++;
			}
		}
	}
	sum = 0;
	for (i = num_pieces-1;i >= 0;i--)
	{
		sum += (thiscount[i] - mins[i]);
		if (i > 0)
		{
			sum *= nums[i-1];
		}
	}
	temp = gHashOffset[searchIndices(sum)];
    if (DEBUG) printf("generic_hash 1\n");
	temp += hash_cruncher(board, gHashBoardSize, thiscount, local_mins);
    if (DEBUG) printf("generic_hash 2\n");
	return temp;
}

/* unhashes hashed to a board */
char* generic_unhash(int hashed, char* dest)
{
	int i, j, offst, boardsize, sum;
    if (DEBUG) printf("generic_unhash 0\n");
	for (i = 0; i < num_pieces;i++)
	{
		thiscount[i] = TERM;
		local_mins[i] = mins[i];
	}
	boardsize = gHashBoardSize;
	offst = 0;
	j = 0;
    if (DEBUG) printf("generic_unhash 1\n");
	for (i = 0;i < boardsize;i++)
	{
		dest[i] = cTERM;
	}
	j = searchOffset(hashed);
	offst = gHashOffset[j];
	hashed -= offst;
    if (DEBUG) printf("generic_unhash 2\n");
	sum = 0;
	for (i = 0;i < num_pieces;i++)
	{
		thiscount[i] = gpd(gOffsetIndices[j + 1] - 1)[i];
		sum += thiscount[i];
	}
    if (DEBUG) printf("generic_unhash 3\n");
	hash_uncruncher(hashed, boardsize, thiscount, dest, local_mins);
	return dest;
}

/* initializes pascal's triangle, factorial table, and piece configuration tables */
void nCr_init(int boardsize)
{
	int i, j, k, sum, ctr;
	int *temp = (int*) malloc (sizeof(int) * num_pieces);

	/*store the left and right wings of pascal's triangle*/
	for(i = 0; i<= boardsize; i++)
	{
		gNCR[i*(boardsize+1)] = 1;
		gNCR[i*(boardsize+1) + i] = 1;
	}

	/*compute and store the middle of pascal's triangle*/
	for(i = 1; i<= boardsize; i++)
	{
		for(j = 1; j < i ; j++)
		{
			gNCR[i*(boardsize+1) + j] = gNCR[(i-1)*(boardsize+1) + j-1] + gNCR[(i-1)*(boardsize+1) + j];
		}
	}

	/*compute and store factorials up to the maximum used, which is boardsize*/
	gFACTORIAL[0] = 1;
	for(i = 1; i <= boardsize;i++)
	{
		gFACTORIAL[i] = i*gFACTORIAL[i-1];
	}
    gFACTORIAL[boardsize+1] = TERM;
	
	/* compute the gPieceDist[i][j]'s with a method similar to Horner's method of
	evaluating a polynomial p using only deg(p) multiplications
	*/
	ctr = 0;
	for (i = 0;i < num_cfgs;i++)
	{
		sum = 0;
		k = i;
		for (j = 0;j < num_pieces;j++)
		{
			temp[j] = mins[j] + (k % (nums[j]));
			sum += temp[j];
			k = k/(nums[j]);
		}
		if (sum == boardsize && validConfig(temp))
		{
			gPieceIndices[ctr] = i;
			ctr++;
		}
	}
	useful_space = ctr;
	gPieceIndices[useful_space] = TERM;
	free(temp);
}

/* finds the piece distribution whose index is n */
int* gpd (int n)
{
    int k = n, j;
    for (j = 0;j < num_pieces;j++)
      	{
      		gpd_store[j] = mins[j] + (k % (nums[j]));
       		k = k/(nums[j]);
       	}
    return gpd_store;
}

/* shorthand for gpd() */
int *gPieceDist(int i)
{
    return gpd(gpi(i));
}

/* shorthand for gPieceIndices */
int gpi (int n)
{
	return gPieceIndices[n];
}

/* shorthand for n choose r */
int nCr(int n, int r)
{
	return gNCR[n*(gHashBoardSize+1) + r];
}

/* shorthand for n factorial */
int fac(int n)
{
	return gFACTORIAL[n];
}



