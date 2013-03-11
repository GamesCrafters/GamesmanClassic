/************************************************************************
**
** NAME:	hash.c
**
** DESCRIPTION:	Generic hash function code.
**
** AUTHOR:	Michel D'Sa, Scott Lindeneau
**		GamesCrafters Research Group, UC Berkeley
**		Supervised by Dan Garcia <ddgarcia@cs.berkeley.edu>
**
** DATE:	2005-01-11
**
** LICENSE:	This file is part of GAMESMAN,
**		The Finite, Two-person Perfect-Information Game Generator
**		Released under the GPL:
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program, in COPYING; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
**************************************************************************/

#include "gamesman.h"

/*********************************************************************************
*** A *PERFECT* hash function -
*** - Efficency, Readability, and Multiple Contexts by Scott Lindeneau
*** - User-specified variables version designed and implemented by Michel D'Sa
*** - Original (3-variable only) version designed by Dan Garcia
***   and implemented by Attila Gyulassy
*********************************************************************************/

#define cCon contextList[currentContext]
#define combiHash(x,y,z) ((x*y)+z)
#define combiUnM(x,y)  (x%y)
#define combiUnD(x,y) (x/y)
#define BUFFERSIZE 1000
#define IDENTITY 0
#define REFLECTION 1
#define ROTATION 2
#define UNKNOWN -1
#define RECT 0
#define HEX 1
/* Global Variables */
struct hashContext **contextList = NULL;
int hash_tot_context = 0, currentContext = 0;
BOOLEAN custom_contexts_mode = FALSE;
/* Hashtable Stuff */
// Using a MOVELIST just to get ints
MOVELIST** generic_hash_hashtable = NULL;
// A hash table from "custom context" => "index in contextList of that context"
BOOLEAN hashtableInitialized = FALSE;

/* symmetry stuff. Added 2007-2-14 (Valentines Day!) */
int numSymmetries = 1;  // we'll always have the identity.
int symBoardRows = 0;
int symBoardCols = 0;
struct symEntry* symmetriesList = NULL;
struct symEntry* new_sym_entry(int symType, int symAngle, int flip);
void composeSymmetries();
BOOLEAN sym_exists(int symType, int symAngle);
void printSymmetries(int boardType);
void printXSpaces(int x);
void freeSymmetries();
int convert(int row, int col, int boardType);
//void flipboard(char* board);
/* end of symmetry stuff */



/* DDG Tried to fix build by externing in hash.h, declaring in hash.c */
/* 2004-10-20 */
/*
   POSITION *hash_hashOffset = NULL;
   POSITION *hash_NCR = NULL;
   POSITION *hash_miniOffset = NULL;
   char *hash_pieces = NULL;
   int *hash_localMins = NULL;
   int *hash_thisCount = NULL;
   int *hash_miniIndices = NULL;
   int hash_numPieces = 0;
   int hash_boardSize = 0;
   int hash_usefulSpace = 0;
   int hash_maxPos = 0;
 */

void            hash_combiCalc();
void            freeHashContext(int contextNum);
int             validConfig(int *t);
int             searchIndices(int s);
int             searchOffset(POSITION h);
POSITION        combiCount(int* tc);
int             hash_countPieces(int *pA);
POSITION        hash_cruncher (char* board);
POSITION        hash_cruncher_sym (char* board, struct symEntry* symIndex);
void            hash_uncruncher (POSITION hashed, char *dest);
int             getPieceParams(int *pa,char *pi,int *mi,int *ma);
POSITION        nCr(int n, int r);
void            nCr_init(int a);
int*            gpd (int n);
int             gpi (int n);
int*            gPieceDist (int i);
void            initializeHashtable();
void            hashtablePut(int, int, int);
int         hashtableGet(int);
void            freeHashtable();

/*******************************
**
**  Generic Hash Initialization
**
*******************************/

void hashCounting()
{
	char* buffer=(char*)SafeMalloc(BUFFERSIZE*sizeof(char));
	while(1)
	{
		int size=0;
		int *piecesArray=(int*)SafeMalloc(sizeof(int));
		int arrSize=1;
		POSITION max=0;
		piecesArray[0]=-1;
		int cur=0;
		char read=0;
		printf("Boardsize:\t");
		scanf("%d",&size);
		while(1)
		{
			printf("\nEnter a piece character ('~' to stop):\t");
			fgets(buffer,BUFFERSIZE, stdin);
			scanf("%c",&read);
			cur=(int)read;
			if(cur=='~') break;
			piecesArray=SafeRealloc(piecesArray,(arrSize+3)*sizeof(int));
			arrSize+=3;
			piecesArray[arrSize-1]=-1;
			piecesArray[arrSize-2]=0;
			piecesArray[arrSize-3]=0;
			piecesArray[arrSize-4]=cur;
			printf("Enter the minimum number of this piece on the board:\t");
			scanf("%d",piecesArray+arrSize-3);
			printf("Enter the maximum number of this piece on the board:\t");
			scanf("%d",piecesArray+arrSize-2);
		}
		printf("\n\nSummary:\n");
		for(cur=0; cur<arrSize-1; cur+=3)
		{
			printf("\t%d\t<\t'%c'\t<\t%d\n",piecesArray[cur+1],piecesArray[cur],piecesArray[cur+2]);
		}
		generic_hash_destroy();
		max=generic_hash_init(size,piecesArray,0,0);
		printf("\nMaximum board positions:\t%llu\n\n\n",max);
		SafeFree(piecesArray);
	}
	SafeFree(buffer);
	return;
}

/* initializes hash tables and critical values */
POSITION generic_hash_init(int boardsize, int *pieces_array, int (*fn)(int *), int player)
{
	int i = -1, k;
	int newcntxt;
	POSITION sofar,temp;

	//if(cCon == NULL)
	//        ExitStageRightErrorString("Attempting to Initialize invalid hash context");

	//if(cCon->init)
	//        freeHashContext(cCon);

	//cCon->init = TRUE;

	newcntxt = generic_hash_context_init();

	currentContext = newcntxt; //context switch

	cCon->numPieces = hash_countPieces(pieces_array);
	cCon->gfn = fn;

	cCon->pieces = (char*) SafeMalloc (sizeof(char) * cCon->numPieces);
	cCon->mins = (int*) SafeMalloc (sizeof(int) * cCon->numPieces);
	cCon->maxs = (int*) SafeMalloc (sizeof(int) * cCon->numPieces);
	cCon->nums = (int*) SafeMalloc (sizeof(int) * cCon->numPieces);
	cCon->thisCount = (int*) SafeMalloc (sizeof(int) * cCon->numPieces);
	cCon->localMins = (int*) SafeMalloc (sizeof(int) * cCon->numPieces);
	cCon->gpdStore = (int*) SafeMalloc (sizeof(int) * cCon->numPieces);
	cCon->miniOffset = (POSITION*) SafeMalloc (sizeof(POSITION) * (cCon->numPieces+2));
	cCon->miniIndices = (int*) SafeMalloc (sizeof(int) * (cCon->numPieces+2));

	getPieceParams(pieces_array, cCon->pieces, cCon->mins,cCon->maxs);
	for (i = 0; i < cCon->numPieces; i++) {
		cCon->nums[i] = cCon->maxs[i] - cCon->mins[i] + 1;
	}

	temp = 0;
	cCon->numCfgs = 1;
	for (i = 0; i < cCon->numPieces; i++) {
		if(cCon->numCfgs < temp)
			ExitStageRightErrorString("Generic Hash: Number of Cfg's wrap on current POSITION type");
		cCon->numCfgs *= cCon->nums[i];
		temp = cCon->numCfgs;
	}
	cCon->pieceIndices = (int*) SafeMalloc (sizeof(int) * cCon->numCfgs);


	nCr_init(boardsize);
	//hash_combiCalc();
	cCon->hashOffset = (POSITION*) SafeMalloc(sizeof(POSITION) * (cCon->usefulSpace+2));
	cCon->offsetIndices = (int*) SafeMalloc(sizeof(int) * (cCon->usefulSpace+2));
	for(i = 0; i < cCon->usefulSpace; i++) {
		cCon->hashOffset[i] = -1;
		cCon->offsetIndices[i] = -1;
	}
	cCon->hashOffset[0] = 0;
	cCon->offsetIndices[0] = 0;
	for (k = 1; k <= cCon->usefulSpace; k++) {
		cCon->hashOffset[k] = combiCount(gPieceDist(k-1));
		cCon->offsetIndices[k] = gpi(k-1);
	}
	cCon->hashOffset[k] = -1;
	temp = 0;
	sofar = 0;
	//printf("\n");
	for (i = 1; i <= cCon->usefulSpace; i++) {
		//printf("block: %llu Tot:%llu\n",cCon->hashOffset[i],sofar);
		//fflush(NULL);
		sofar += cCon->hashOffset[i];
		if(sofar<temp) {
			ExitStageRightErrorString("To many positions to represent in current position format");
		}
		cCon->hashOffset[i] = sofar;
		cCon->offsetIndices[i]++;
		temp = sofar;
	}
	cCon->hashOffset[cCon->usefulSpace + 1] = -1;
	cCon->maxPos = sofar;

	cCon->player = player % 3;         // ensures player is either 0, 1, or 2

	if (cCon->player != 0)
		return sofar;
	else return sofar*2;
}

/* initializes pascal's triangle and piece configuration tables */
void nCr_init(int boardsize)
{
	int i, j, k, sum, ctr;
	int *temp = (int*) SafeMalloc (sizeof(int) * cCon->numPieces);
	/*hash_NCR = */
	cCon->NCR = (POSITION*) SafeMalloc(sizeof(POSITION) * (boardsize + 1) * (boardsize + 1));
	/*hash_boardSize = */
	cCon->boardSize = boardsize;

	/*store the left and right wings of pascal's triangle*/
	for(i = 0; i<= boardsize; i++) {
		cCon->NCR[i*(boardsize+1)] = 1;
		cCon->NCR[i*(boardsize+1) + i] = 1;
	}

	/*compute and store the middle of pascal's triangle*/
	for(i = 1; i<= boardsize; i++) {
		for(j = 1; j < i; j++) {
			cCon->NCR[i*(boardsize+1) + j] = cCon->NCR[(i-1)*(boardsize+1) + j-1] + cCon->NCR[(i-1)*(boardsize+1) + j];
		}
	}

	/* compute the gPieceDist[i][j]'s with a method similar to Horner's method of
	   evaluating a polynomial p using only deg(p) multiplications
	 */
	ctr = 0;
	for(i=0; i<cCon->numCfgs; i++) {
		cCon->pieceIndices[i] = -1;
	}

	for (i = 0; i < cCon->numCfgs; i++) {
		sum = 0;
		k = i;
		for (j = 0; j < cCon->numPieces; j++) {
			temp[j] = cCon->mins[j] + (k % (cCon->nums[j]));
			sum += temp[j];
			k = k/(cCon->nums[j]);
		}
		if (sum == boardsize && validConfig(temp)) {
			cCon->pieceIndices[ctr] = i;
			ctr++;
		}
	}
	cCon->usefulSpace = ctr;
	SafeFree(temp);
}


/* memoizes combinations not used right now */
void hash_combiCalc()
{
	int i,sums;
	int *thPieces = (int *) SafeMalloc(sizeof(int) * cCon->numPieces);

	for(i=0; i<cCon->numPieces; i++)
		thPieces[i] = cCon->mins[i];
	sums = 1;
	for(i=0; i<cCon->numPieces; i++) {
		if(cCon->nums[i] != 0)
			sums *= cCon->nums[i];
	}
	cCon->combiArray = (POSITION *) SafeMalloc(sizeof(POSITION) * sums);
	while(thPieces[cCon->numPieces-1] <= cCon->maxs[cCon->numPieces-1]) {

		sums = 0;
		for(i=0; i<cCon->numPieces; i++) {
			sums = combiHash(sums,cCon->nums[i],thPieces[i]);
		}
		cCon->combiArray[sums] = combiCount(thPieces);
		thPieces[0]++;
		i=0;
		while(i<(cCon->numPieces-1) && thPieces[i] > cCon->maxs[i]) {
			thPieces[i]=cCon->mins[i];
			i++;
			thPieces[i]++;
		}
	}

}

/***************************************
**
**      Generic Hashing
**
****************************************/

/* hashes *board to a POSITION */
POSITION generic_hash_hash(char* board, int player) {
	int i, j;
	POSITION temp, sum;
	int boardSize = cCon->boardSize; /*hash_boardSize;*/

	for (i = 0; i < cCon->numPieces; i++)
	{
		cCon->thisCount[i] = 0;
		cCon->localMins[i] = cCon->mins[i];
	}

	for (i = 0; i < boardSize; i++)
	{
		for (j = 0; j < cCon->numPieces; j++) {
			if (board[i] == cCon->pieces[j]) {
				cCon->thisCount[j]++;
			}
		}
	}
	sum = 0;
	for (i = cCon->numPieces-1; i >= 0; i--)
	{
		sum += (cCon->thisCount[i] - cCon->mins[i]);
		if (i > 0) {
			sum *= cCon->nums[i-1];
		}
	}
	temp = cCon->hashOffset[searchIndices(sum)];
	temp += hash_cruncher(board);
	if (temp > cCon->maxPos) {
		ExitStageRightErrorString("generic_hash encountered position larger than maxPos.");
	}
	if (cCon->player != 0) // using single-player boards, ignore "player"
		return temp;
	else return temp + (player-1)*(cCon->maxPos); //accomodates generic_hash_turn
}

//accomodates generic_hash_turn and symmetries
POSITION generic_hash_hash_sym(char* board, int player, POSITION offset, struct symEntry* symIndex)
{
	int i, j;
	POSITION temp;
	int boardSize = cCon->boardSize; /*hash_boardSize;*/

	if (symIndex == NULL)
		ExitStageRightErrorString("Invalid symmetry");

	for (i = 0; i < cCon->numPieces; i++)
	{
		cCon->thisCount[i] = 0;
		cCon->localMins[i] = cCon->mins[i];
	}

	for (i = 0; i < boardSize; i++)
	{
		for (j = 0; j < cCon->numPieces; j++) {
			if (board[symIndex->sym[i]] == cCon->pieces[j]) {
				cCon->thisCount[j]++;
			}
		}
	}
	temp = offset + hash_cruncher_sym(board, symIndex);
	if (cCon->player != 0) // using single-player boards, ignore "player"
		return temp;
	else return temp + (player-1)*(cCon->maxPos); //accomodates generic_hash_turn
}

/* helper func from generic_hash_unhash() computes lexicographic rank of *board
   among boards with the same configuration argument *thiscount */
POSITION hash_cruncher (char* board)
{
	POSITION sum = 0;
	int i = 0, k = 0, max1 = 0;
	int boardSize = cCon->boardSize;

	for(; boardSize>1; boardSize--) {
		i = 0;

		while (board[boardSize - 1] != cCon->pieces[i])
			i++;
		for (k = 0; k < i; k++) {
			max1 = 1;
			if (cCon->localMins[k] > 1)
				max1 = cCon->localMins[k];

			if (cCon->thisCount[k] >= max1) {
				cCon->thisCount[k]--;
				sum += combiCount(cCon->thisCount);
				cCon->thisCount[k]++;
			}
		}
		cCon->thisCount[i]--;
		cCon->localMins[i]--;
	}

	return sum;
}

// symmetry version
POSITION hash_cruncher_sym (char* board, struct symEntry* symIndex)
{
	POSITION sum = 0;
	int i = 0, k = 0, max1 = 0;
	int boardSize = cCon->boardSize;

	for(; boardSize>1; boardSize--) {
		i = 0;

		while (board[symIndex->sym[boardSize - 1]] != cCon->pieces[i])
			i++;
		for (k = 0; k < i; k++) {
			max1 = 1;
			if (cCon->localMins[k] > 1)
				max1 = cCon->localMins[k];

			if (cCon->thisCount[k] >= max1) {
				cCon->thisCount[k]--;
				sum += combiCount(cCon->thisCount);
				cCon->thisCount[k]++;
			}
		}
		cCon->thisCount[i]--;
		cCon->localMins[i]--;
	}

	return sum;
}

/* tells whose move it is, given a board's hash number */
int generic_hash_turn (POSITION hashed)
{
	if (cCon->player != 0)         // using single-player boards
		return cCon->player;
	else return hashed >= cCon->maxPos ? 2 : 1;
}


/*************************************
**
**      Generic UnHashing
**
*************************************/


char* generic_hash_unhash_tcl(POSITION pos)
{
	char* ret = (char*) SafeMalloc (sizeof(char) * cCon->boardSize+1);
	generic_hash_unhash(pos,ret);
	ret[cCon->boardSize] = '\0';
	return ret;
}

/* unhashes hashed to a board */
char* generic_hash_unhash(POSITION hashed, char* dest)
{
	POSITION offst;
	int i, j, boardSize;
	hashed %= cCon->maxPos; //accomodates generic_hash_turn

	boardSize = cCon->boardSize;
	j = searchOffset(hashed);
	offst = cCon->hashOffset[j];
	hashed -= offst;
	for (i = 0; i < cCon->numPieces; i++) {
		cCon->localMins[i] = cCon->mins[i];
		cCon->thisCount[i] = gpd(cCon->offsetIndices[j + 1] - 1)[i];
	}
	hash_uncruncher(hashed, dest);
	return dest;
}

/* helper func from generic_hash_hash() computes a board, given its lexicographic rank hashed
   among boards with the same configuration argument *thiscount*/
void hash_uncruncher (POSITION hashed, char *dest)
{
	int i = 0, j = 0;
	int max1 = 0;
	int boardSize = cCon->boardSize;

	for(; boardSize>0; boardSize--) {
		if (boardSize == 1) {
			i = 0;
			while (cCon->thisCount[i] == 0)
				i++;
			dest[0] = cCon->pieces[i];
		} else {
			cCon->miniOffset[0] = 0;
			cCon->miniIndices[0] = 0;
			j = 1;
			for (i = 0; (i < cCon->numPieces) && (cCon->miniOffset[j-1] <= hashed); i++) {
				max1 = 1;
				if (cCon->localMins[i] > 1)
					max1 = cCon->localMins[i];
				if (cCon->thisCount[i] >= max1) {
					cCon->thisCount[i]--;
					cCon->miniOffset[j] = cCon->miniOffset[j-1] + combiCount(cCon->thisCount);
					cCon->thisCount[i]++;
					cCon->miniIndices[j] = i;
					j++;
				}
			}
			i = j-1;
			cCon->thisCount[cCon->miniIndices[i]]--;
			cCon->localMins[cCon->miniIndices[i]]--;
			dest[boardSize-1] = cCon->pieces[cCon->miniIndices[i]];
			hashed = hashed - cCon->miniOffset[i-1];
		}

	}

}



/************************************
*************************************
**
**  Hash Context Functions
**
*************************************
************************************/

/************************************
**
** generic_hash_context_init()
**
**  Generates a new uninitialized
**  hash context with a unique hash
**  context identifier
**
************************************/
int generic_hash_context_init()
{
	// initializes the context hashtable if it isn't already
	if (!hashtableInitialized)
		initializeHashtable();

	int i;
	struct hashContext **temp;
	struct hashContext *newHashC;

	int myContext = hash_tot_context;
	hash_tot_context++;

	temp = (struct hashContext **) SafeMalloc (sizeof(struct hashContext*)*(hash_tot_context));

	if(contextList != NULL) {
		for(i=0; i<(hash_tot_context-1); i++) //copy the last contexts over
			temp[i] = contextList[i];
		SafeFree(contextList);
		contextList = temp;
	} else {
		contextList = temp;
	}

	newHashC = (struct hashContext *) SafeMalloc (sizeof(struct hashContext));
	newHashC->hashOffset = NULL;
	newHashC->maxPos = 0;
	newHashC->NCR = NULL;
	newHashC->gpdStore = NULL;
	newHashC->offsetIndices = NULL;
	newHashC->pieceIndices = NULL;
	newHashC->boardSize = 0;
	newHashC->numCfgs = 0;
	newHashC->usefulSpace = 0;
	newHashC->numPieces = 0;
	newHashC->pieces = NULL;
	newHashC->nums = NULL;
	newHashC->mins = NULL;
	newHashC->maxs = NULL;
	newHashC->thisCount = NULL;
	newHashC->localMins = NULL;
	newHashC->gfn = NULL;
	newHashC->player = 0;
	//newHashC->init = FALSE;
	newHashC->contextNumber = -1;
	contextList[hash_tot_context-1] = newHashC;

	return myContext;
}

/******************************
**
** generic_hash_context_switch(int context)
**
**  Switches the current hash context
**  to the indicated hash context. Set's global
**  variables for speed improvements during solving
**
******************************/

void generic_hash_context_switch(int context)
{
	if (custom_contexts_mode) {
		currentContext = hashtableGet(context);
		if (currentContext == -1)
			ExitStageRightErrorString("ERROR: Attempting to switch to non-existant hash context");
	} else {
		if(context > hash_tot_context || context < 0)
			ExitStageRightErrorString("Attempting to switch to non-existant hash context");

		currentContext = context;
	}

	//hash_hashOffset = cCon->hashOffset;
	//hash_NCR = cCon->NCR;
	//hash_miniOffset = cCon->miniOffset;
	//hash_pieces = cCon->pieces;
	//hash_thisCount = cCon->thisCount;
	//hash_localMins = cCon->localMins;
	//hash_miniOffset = cCon->miniOffset;
	//hash_miniIndices = cCon->miniIndices;
	//hash_numPieces = cCon->numPieces;
	//hash_boardSize = cCon->boardSize;
	//hash_usefulSpace = cCon->usefulSpace;
	//hash_maxPos = cCon->maxPos;
}

/******************************
**
** generic_hash_cur_context()
**
** Returns the current context value
**
******************************/


int generic_hash_cur_context()
{
	if (custom_contexts_mode)
		return cCon->contextNumber;
	else return currentContext;
}

/******************************
**
** generic_hash_max_pos()
**
** returns the current context's gNumberOfPositions
**
******************************/

POSITION generic_hash_max_pos()
{
	if (cCon->player != 0)         // using single-player boards
		return cCon->maxPos;
	else return (cCon->maxPos)*2;
}

/******************************
**
** generic_hash_any_context_mode()
**
** toggles Custom Contexts Mode on/off.
**
******************************/

void generic_hash_custom_context_mode(BOOLEAN on)
{
	custom_contexts_mode = on;
}

/******************************
**
** generic_hash_set_context(int context)
**
** Changes the custom context number of the current hash context.
**
******************************/

void generic_hash_set_context(int context)
{
	if (context < 0)
		ExitStageRightErrorString("ERROR: Attempting to set a negative custom context");
	hashtablePut(context, currentContext, cCon->contextNumber); // add/change its entry in the hashtable
	cCon->contextNumber = context;
}


/*******************************
**
**      Various Helper functions
**          Not all are used
**
********************************/

/* frees up memory for current context */
void freeHashContext(int contextNum)
{
	if(contextList[contextNum] == NULL)
		ExitStageRightErrorString("Attempting to free an invalid hash context");

	SafeFree(contextList[contextNum]->hashOffset);
	SafeFree(contextList[contextNum]->NCR);
	SafeFree(contextList[contextNum]->gpdStore);
	SafeFree(contextList[contextNum]->offsetIndices);
	SafeFree(contextList[contextNum]->pieceIndices);
	SafeFree(contextList[contextNum]->pieces);
	SafeFree(contextList[contextNum]->nums);
	SafeFree(contextList[contextNum]->mins);
	SafeFree(contextList[contextNum]->maxs);
	SafeFree(contextList[contextNum]->thisCount);
	SafeFree(contextList[contextNum]->localMins);

	SafeFree(contextList[contextNum]);

}

/* determines if a piece distribution is valid or not.
   t[i] denotes the number of occurrences of the ith piece in this cofiguration
 */
int validConfig(int *t)
{
	int cond = (cCon->gfn == NULL) ? 1 : cCon->gfn(t);
	return cond;
}


/* function used to map a member of gHashOffset into gOffsetIndices */
int searchIndices(int s)
{
	int i = cCon->usefulSpace;
	while(cCon->offsetIndices[i] > s)
		i--;
	return i;
}

/* function used to map a member of gOffsetIndices into gHashOffset */
int searchOffset(POSITION h)
{
	int i = cCon->usefulSpace;
	while(cCon->hashOffset[i] > h)
		i--;
	return i;
}

/* helper function used to find n choose (t1,t2,t3,...,tn) where the ti's are
   the members of *tc */
POSITION combiCount(int* tc)
{
	POSITION sum = 0, prod = 1, ind = 0,old=1,hold=0;
	for (ind = 0; ind < cCon->numPieces - 1; ind++) {
		sum += tc[ind];
		hold = nCr(sum+tc[ind+1], sum);
		prod *= hold;
		if(prod/hold !=  old)
			ExitStageRightErrorString("Combination Calculation Wraps");
		old = prod;
	}
	return prod;
}




/* parses the char* input to generic_hash_init() and assigns corresponding values to
   the pieces, minima, and maxima arrays*/
int getPieceParams (int *pa, char *pi, int *mi, int *ma)
{
	int i = 0;
	while (pa[i*3] != -1) {
		pi[i] = (char)pa[i*3];
		mi[i] = pa[i*3 + 1];
		ma[i] = pa[i*3 + 2];
		i++;
	}
	return i;
}
int hash_countPieces(int *pA)
{
	int i = 0;

	while(pA[i*3] != -1)
		i++;

	return i;
}

/* finds the piece distribution whose index is n */
int* gpd (int n)
{
	int k = n, j;
	for (j = 0; j < cCon->numPieces; j++) {
		cCon->gpdStore[j] = cCon->mins[j] + (k % (cCon->nums[j]));
		k = k/(cCon->nums[j]);
	}
	return cCon->gpdStore;
}

/* shorthand for gpd() */
int *gPieceDist(int i)
{
	return gpd(gpi(i));
}

/* shorthand for gPieceIndices */
int gpi (int n)
{
	return (cCon->pieceIndices)[n];
}

/* shorthand for n choose r */
POSITION nCr(int n, int r)
{
	return cCon->NCR[n*(cCon->boardSize+1) + r];
}

void generic_hash_destroy()
{
	int i;
	for (i=0; i < hash_tot_context; i++)
		//if (contextList[i]->init)
		freeHashContext(i);
	if (contextList != NULL) {
		SafeFree(contextList);
		contextList = NULL;
	}
	if (generic_hash_hashtable != NULL) {
		freeHashtable();
	}
	hash_tot_context = currentContext = 0;
	custom_contexts_mode = FALSE;
}



/* Code for the Hashtable of contexts. */

void initializeHashtable() {
	generic_hash_hashtable = (MOVELIST**) SafeMalloc(HASHTABLE_BUCKETS * sizeof(MOVELIST*));
	int i;
	for (i = 0; i < HASHTABLE_BUCKETS; i++)
		generic_hash_hashtable[i] = NULL;
	hashtableInitialized = TRUE;
}

void hashtablePut(int context, int index, int old) {
	MOVELIST* bucket; BOOLEAN found;
	int contextBucket = context % HASHTABLE_BUCKETS;
	if (old != -1) {
		// first, remove old context!
		int oldContextBucket = contextList[index]->contextNumber % HASHTABLE_BUCKETS;
		bucket = generic_hash_hashtable[oldContextBucket];
		MOVELIST* pre = NULL;
		found = FALSE;
		for (; bucket != NULL; bucket = bucket->next) {
			if (bucket->move == index) { // remove from list
				if (pre == NULL) { // first one on list!
					generic_hash_hashtable[oldContextBucket] = bucket->next;
				} else {
					pre->next = bucket->next;
				}
				SafeFree(bucket);
				found = TRUE;
				break;
			}
			pre = bucket;
		}
		if (!found) // if we DIDN'T find old hash context... big problem
			ExitStageRightErrorString("Congratulations! You BROKE Gamesman! YAY!!!!!\n");
	}
	// Now, we can place the context
	bucket = generic_hash_hashtable[contextBucket];
	found = FALSE;
	// let's make sure it's not already in there...
	for (; bucket != NULL; bucket = bucket->next) {
		if (contextList[bucket->move]->contextNumber == context) {
			found = TRUE;
			break;
		}
	}
	if (found) { // duplicate context, error out.
		printf("ERROR: Assigning hash context number %d, but it already exists!\n", context);
		exit(1);
	}
	// FINALLY, we can place the context in the hashtable
	generic_hash_hashtable[contextBucket] =
	        CreateMovelistNode(index, generic_hash_hashtable[contextBucket]);
}

int hashtableGet(int context) {
	MOVELIST* bucket = generic_hash_hashtable[context % HASHTABLE_BUCKETS];
	int index;
	// find the index and return it
	for (; bucket != NULL; bucket = bucket->next) {
		index = bucket->move;
		if (contextList[index]->contextNumber == context) {
			return index;
		}
	}
	// if got here, error
	return -1;
}

void freeHashtable() {
	int i;
	for (i = 0; i < HASHTABLE_BUCKETS; i++)
		FreeMoveList(generic_hash_hashtable[i]);
	if (generic_hash_hashtable != NULL)
		SafeFree(generic_hash_hashtable);
	hashtableInitialized = FALSE;
}

// should be called after generic_hash_init
void generic_hash_init_sym(int boardType, int numRows, int numCols, int* reflections, int numReflects, int* rotations, int numRots, int flippable) {

	int i,j,k,numDiags;
	int boardSize = 0;
	int *hex60Rot = NULL, *tempSym = NULL, *hex0Ref;
	struct symEntry *symIndex = NULL;
	BOOLEAN rectBoard = FALSE;
	BOOLEAN hexBoard = FALSE;

	// 0 for a rectangular board, 1 for a hex board
	if (boardType == RECT) {
		rectBoard = TRUE;
		boardSize = numRows*numCols;
	}
	else if (boardType == HEX) {
		hexBoard = TRUE;
		for (j = 0; j < numRows; j++) {
			for (k = 0; k < numCols; k++) {
				boardSize++;
			}
			if (j < numRows/2)
				numCols++;
			if (j >= numRows/2 && j < numRows-1)
				numCols--;
		}
		// FIGURE THIS OUT
		numDiags = 0;
	}
	symBoardRows = numRows;
	symBoardCols = numCols;

	// setup symmetries list
	symmetriesList = (struct symEntry*) SafeMalloc (sizeof(struct symEntry));
	symmetriesList->angle = 0;
	symmetriesList->next = NULL;
	symmetriesList->type = IDENTITY;
	symmetriesList->flip = flippable; //does NOT mean that we flip the identity if flippable; only serves as indicator that there are flippable positions
	symmetriesList->sym =   (int*) SafeMalloc (sizeof(int) * boardSize);
	for (i = 0; i < boardSize; i++) {
		symmetriesList->sym[i] = i;
	}
	symIndex = symmetriesList;
	for (i = 0; i < numReflects; i++) {
		symIndex->next = new_sym_entry(REFLECTION, reflections[i], 0);
		symIndex = symIndex->next;
	}
	for (i = 0; i < numRots; i++) {
		symIndex->next = new_sym_entry(ROTATION, rotations[i], 0);
		symIndex = symIndex->next;
	}
	//if true, swap colors, swap turn has same value.
	//flip, then do the rest.
	//
	//
	if (flippable == 1) {
		symIndex->next = (struct symEntry*) SafeMalloc (sizeof(struct symEntry));
		symIndex = symIndex->next;
		symIndex->angle = 0;
		symIndex->next = NULL;
		symIndex->type = IDENTITY;
		symIndex->flip = 1;
		symIndex->sym =  (int*) SafeMalloc (sizeof(int) * boardSize);
		for (i = 0; i < boardSize; i++) {
			symIndex->sym[i] = i;
		}

		for (i = 0; i < numReflects; i++) {
			symIndex->next = new_sym_entry(REFLECTION, reflections[i], 1);
			symIndex = symIndex->next;
		}
		for (i = 0; i < numRots; i++) {
			symIndex->next = new_sym_entry(ROTATION, rotations[i], 1);
			symIndex = symIndex->next;
		}
	}

	// + 1 is for identity.
	numSymmetries = (numRots + numReflects + 1) * (flippable + 1);

	// need an equilateral board for this
	//if ((rectBoard && numRows == numCols) || (hexBoard && numRows == numDiags)) {
	//  composeSymmetries();
	//}

	// set up hex stuff for later
	if (hexBoard) {
		hex60Rot = (int*) SafeMalloc(sizeof(int) * boardSize);
		hex0Ref = (int*) SafeMalloc(sizeof(int) * boardSize);
		tempSym = (int*) SafeMalloc(sizeof(int) * boardSize);
		// numCols refers to # of elements on a row. initialize to 60 degree rotation
		// always an odd number of rows so numRows/2 + 1 is the middle row
		for (j = 0; j < numRows; j++) {
			for (k = 0; k < numCols; k++) {
				if (j <= numRows/2) {
					// i = number of rows below middle row
					if (numCols-k-1 > numRows/2)
						i = (numCols-k-1) - (numRows/2);
					else
						i = 0;
					hex60Rot[convert(j,k,HEX)] = convert(numCols-k-1,j-i,HEX);
				}
				else {
					// i = number of rows below middle row
					if (numRows-k-1 > numRows/2)
						i = (numRows-k-1) - (numRows/2);
					else
						i = 0;
					hex60Rot[convert(j,k,HEX)] = convert(numRows-k-1,j-i,HEX);
				}
				hex0Ref[convert(j,k,HEX)] = convert(numRows-j-1,k,HEX);
			}
			if (j < numRows/2)
				numCols++;
			if (j >= numRows/2 && j < numRows-1)
				numCols--;
		}
	}

	symIndex = symmetriesList;
	while (symIndex != NULL) {
		/* set the table of reflections.  Array passed in should specify the
		 * angle of the line over which the reflection will take place
		 * (think polar coordinates).  For example, if the first reflection
		 * is over the N-S axis and the second is over the E-W axis then
		 * reflections = {90, 0}.  This means we want the following transformations
		 *
		 *    0 1 2      2 1 0                0 1 2    6 7 8
		 *    3 4 5  ->  5 4 3        and     3 4 5 -> 3 4 5
		 *    6 7 8      8 7 6                6 7 8    0 1 2
		 *
		 * this also means reflectionSymmetries[0] = {2,1,0,5,4,3,8,7,6} and
		 * reflectionSymmetries[1] = {6,7,8,3,4,5,0,1,2} */
		if (symIndex->type == REFLECTION) {
			if (rectBoard) {
				if (symIndex->angle == 0 || symIndex->angle == 180) {
					// swap rows, leaving the middle one alone
					for (j = 0; j < numRows; j++) {
						// set each element of the row
						for (k = 0; k < numCols; k++) {
							// j*numCols is the first element of the jth row from top
							// (numRows-j-1)*numCols is the first element of the jth row from bottom
							symIndex->sym[j*numCols+k] = (numRows-j-1)*numCols + k;
							//symIndex->sym[(numRows-j-1)*numCols+k] = j*numCols + k;
						}
					}
				}
				else if (symIndex->angle == 45 || symIndex->angle == 315) {
					for (j = 0; j < numRows; j++) {
						for (k = 0; k < numCols; k++) {
							symIndex->sym[j*numCols+k] = (numRows-k-1)*numCols + (numCols-j-1);
						}
					}
				}
				else if (symIndex->angle == 90 || symIndex->angle == 270) {
					// swap columns, leaving the middle one alone
					for (j = 0; j < numRows; j++) {
						// set each element of the column
						for (k = 0; k < numCols; k++) {
							// in same row, but opposite columns
							symIndex->sym[j*numCols + k] = j*numCols + (numCols-k-1);
							//symIndex->sym[j*numCols + (numCols-k-1)] = j*numCols + k;
						}
					}
				}
				else if (symIndex->angle == 135 || symIndex->angle == 225) {
					for (j = 0; j < numRows; j++) {
						for (k = 0; k < numCols; k++) {
							symIndex->sym[j*numCols+k] = k*numCols + j;
						}
					}
				}
			}


			else if (hexBoard && (symIndex->angle == 0 || symIndex->angle == 180)) {
				for (j = 0; j < numRows; j++) {
					for (k = 0; k < numCols; k++) {
						symIndex->sym[convert(j,k,HEX)] = convert(numRows-j-1,k,HEX);
					}
					if (j < numRows/2)
						numCols++;
					if (j >= numRows/2 && j < numRows-1)
						numCols--;
				}
			}
			else if (hexBoard && (symIndex->angle % 30 == 0)) {
				// rotate it a bunch of times
				for (j = 0; j < boardSize; j++) {
					symIndex->sym[j] = j;
					tempSym[j] = j;
				}
				if (symIndex->angle > 150)
					symIndex->angle -= 180;
				for (j = 0; j < symIndex->angle/30; j++) {
					for (k = 0; k < boardSize; k++) {
						tempSym[k] = symIndex->sym[hex60Rot[k]];
					}
					for (k = 0; k < boardSize; k++) {
						symIndex->sym[k] = tempSym[k];
					}
				}
				// reflect over 0
				for (j = 0; j < boardSize; j++) {
					tempSym[j] = symIndex->sym[hex0Ref[j]];
				}
				for (j = 0; j < boardSize; j++) {
					symIndex->sym[j] = tempSym[j];
				}
			}
		}
		/* set the table of rotations.  Array passed in should specify the
		 * angle by which to rotate.  For example, if the first reflection
		 * is 90 degrees, the second is 180 degrees, and the third is 270 degrees,
		 * rotations = {90,180,270}.  This means we want the following transformations
		 *
		 *    0 1 2    6 3 0         0 1 2    8 7 6         0 1 2     2 5 8
		 *    3 4 5 -> 7 4 1   and   3 4 5 -> 5 4 3   and   3 4 5 ->  1 4 7
		 *    6 7 8    8 5 2         6 7 8    2 1 0         6 7 8     0 3 6
		 *
		 * this also means rotationSymmetries[0] = {6,3,0,7,4,1,8,5,2} and
		 * rotationSymmetries[1] = {8,7,6,5,4,3,2,1,0} */
		else if (symIndex->type == ROTATION) {
			// 90 degree rotation on a rectangular board
			if (rectBoard) {
				if (symIndex->angle == 90) {
					// go through each row and replace it by the corresponding column
					for (j = 0; j < numRows; j++) {
						// this is for column math
						for (k = 0; k < numCols; k++) {
							// j*numCols + k is the kth element of the jth row
							// (numRows-k-1)*numCols + j
							symIndex->sym[j*numCols + k] = (numRows-k-1)*numCols + j;
						}
					}
				}
				// 180 degree rotations are simple.  Instead of going from 0 to boardSize-1,
				// go from boardSize-1 to 0
				else if (symIndex->angle == 180) {
					k = 0;
					for (j = boardSize-1; j >=0; j--) {
						symIndex->sym[k] = j;
						k++;
					}
				}
				// -90 degree rotation on a rectangular board
				else if (symIndex->angle == 270) {
					// for jth row and kth column
					for (j = 0; j < numRows; j++) {
						for (k = 0; k < numCols; k++) {
							// j*numCols + k = jth row, kth element from the left
							// k*numCols + (numCols-j-1) = kth row, jth element from the right
							symIndex->sym[j*numCols + k] = k*numCols + (numCols-j-1);
						}
					}
				}
			}
			// hex math is not as easy so I'm just going to make a single
			// 60 degree rotation and do it lots of times.  Assume an equilateral board.
			/*
			     0 1 2         7 3 0            0 1      2 0
			    3 4 5 6       c 8 4 1          2 3 4 -> 5 3 1
			   7 8 9 a b ->  g d 9 5 2          5 6      6 4
			    c d e f       h e a 6
			     g h i         i f b
			 */
			else if (hexBoard && (symIndex->angle % 60 == 0)) {
				for (j = 0; j < boardSize; j++) {
					symIndex->sym[j] = j;
					tempSym[j] = j;
				}
				for (j = 0; j < symIndex->angle/60; j++) {
					for (k = 0; k < boardSize; k++) {
						tempSym[k] = symIndex->sym[hex60Rot[k]];
					}
					for (k = 0; k < boardSize; k++) {
						symIndex->sym[k] = tempSym[k];
					}
				}
			}
		}
		// shouldn't get here, but if we do set it to the identity
		else {
			//printf("setting to identity for some reason\n");
			//printf("angle = %d, type = %d\n", symIndex->angle, symIndex->type);
			for (j = 0; j < boardSize; j++) {
				symIndex->sym[j] = j;
			}
		}
		symIndex = symIndex->next;
	}

	//printSymmetries(boardType);

	if (hexBoard) {
		SafeFree(hex60Rot);
		SafeFree(hex0Ref);
		SafeFree(tempSym);
	}
	// set the function pointer.
	gCanonicalPosition = generic_hash_canonicalPosition;
}

/* print out the symmetryTable for debugging */
void printSymmetries(int boardType) {
	struct symEntry *symIndex =  symmetriesList;
	int i,j,k,numCols;
	numCols = symBoardCols;

	printf("-------------------Symmetries-------------------\n");
	for (i = 0; i < numSymmetries; i++) {
		printf(" Symmetry #%d:",i);
		if (symIndex->type == IDENTITY)
			printf(" Identity\n");
		else if (symIndex->type == REFLECTION)
			printf(" %d degree Reflection\n", symIndex->angle);
		else if (symIndex->type == ROTATION)
			printf(" %d degree Rotation\n", symIndex->angle);
		else
			printf(" Unknown symmetry type\n");
		printf("\n");
		if (boardType == RECT) {
			for (j = 0; j < symBoardRows; j++) {
				printf("\t\t");
				for (k = 0; k < numCols; k++) {
					printf("%d ", symIndex->sym[j*numCols+k]);
				}
				printf("\n");
			}
			printf("\n");
		}
		else if (boardType == HEX) {
			for (j = 0; j < symBoardRows; j++) {
				printf("\t\t");
				printXSpaces(symBoardRows-numCols);
				for (k = 0; k < numCols; k++) {
					printf("%d ", symIndex->sym[convert(j,k,HEX)]);
				}
				printf("\n");
				if (j < symBoardRows/2)
					numCols++;
				if (j >= symBoardRows/2 && j < symBoardRows-1)
					numCols--;
			}
		}
		else
			printf("unknown board type\n");
		symIndex = symIndex->next;
	}
}

void printXSpaces(int x) {
	int i;
	for (i = 0; i < x; i++) {
		printf(" ");
	}
}

POSITION generic_hash_canonicalPosition(POSITION pos) {
	char* board = (char*) SafeMalloc(sizeof(char) * cCon->boardSize);
	char* flippedboard = (char*) SafeMalloc(sizeof(char) * cCon->boardSize);
	struct symEntry* symIndex = symmetriesList->next;
	generic_hash_unhash(pos, board);
	generic_hash_unhash(pos, flippedboard);
	int player = generic_hash_turn(pos);
	int flippedplayer;
	if (player == 1)
		flippedplayer = 2;
	else
		flippedplayer = 1;
	POSITION tempPos, sum, offset, minPos;
	POSITION flippedsum, flippedoffset;
	int i, j, boardSize = cCon->boardSize;

	for (i = 0; i < cCon->numPieces; i++)
	{
		cCon->thisCount[i] = 0;
		cCon->localMins[i] = cCon->mins[i];
	}

	for (i = 0; i < boardSize; i++)
	{
		for (j = 0; j < cCon->numPieces; j++) {
			if (board[symIndex->sym[i]] == cCon->pieces[j]) {
				cCon->thisCount[j]++;
			}
		}
	}
	sum = 0;
	for (i = cCon->numPieces-1; i >= 0; i--)
	{
		sum += (cCon->thisCount[i] - cCon->mins[i]);
		if (i > 0) {
			sum *= cCon->nums[i-1];
		}
	}
	offset = cCon->hashOffset[searchIndices(sum)];

	if (symmetriesList->flip == 1) {
		//for flipped:
		for (i = 0; i < cCon->boardSize; i++) {
			if (flippedboard[i] == cCon->pieces[0])
				flippedboard[i] = cCon->pieces[1];
			else if (flippedboard[i] == cCon->pieces[1])
				flippedboard[i] = cCon->pieces[0];
		}

		for (i = 0; i < cCon->numPieces; i++) {
			cCon->thisCount[i] = 0;
			cCon->localMins[i] = cCon->mins[i];
		}

		for (i = 0; i < boardSize; i++)
		{
			for (j = 0; j < cCon->numPieces; j++) {
				if (flippedboard[symIndex->sym[i]] == cCon->pieces[j]) {
					cCon->thisCount[j]++;
				}
			}
		}
		flippedsum = 0;
		for (i = cCon->numPieces-1; i >= 0; i--)
		{
			flippedsum += (cCon->thisCount[i] - cCon->mins[i]);
			if (i > 0) {
				flippedsum *= cCon->nums[i-1];
			}
		}
		flippedoffset = cCon->hashOffset[searchIndices(flippedsum)];
	}

	minPos = generic_hash_hash_sym(board, player, offset, symmetriesList);
	// try each symmetry, keeping the lowest position hash
	// returns as the canonical position.
	while (symIndex != NULL) {
		if (symIndex->flip == 1)
			tempPos = generic_hash_hash_sym(flippedboard, flippedplayer, flippedoffset, symIndex);
		else
			tempPos = generic_hash_hash_sym(board, player, offset, symIndex);
		minPos = (tempPos < minPos) ? tempPos : minPos;
		symIndex = symIndex->next;

	}

	SafeFree(board);
	SafeFree(flippedboard);
	return minPos;
}

struct symEntry* new_sym_entry(int symType, int symAngle, int flip) {
	struct symEntry* newEntry = (struct symEntry*) SafeMalloc(sizeof(struct symEntry));

	newEntry->type = symType;
	newEntry->angle = symAngle;
	newEntry->flip = flip;
	newEntry->sym = (int*) SafeMalloc(sizeof(int) * cCon->boardSize);
	newEntry->next = NULL;
	return newEntry;
}

void freeSymmetries() {
	struct symEntry *symIndex, *tempNext;
	symIndex = symmetriesList;

	while(symIndex != NULL) {
		tempNext = symIndex->next;
		SafeFree(symIndex->sym);
		SafeFree(symIndex);
		symIndex = tempNext;
	}
}

void generic_hash_add_sym(int* symToAdd) {
	int i;
	struct symEntry* symIndex = symmetriesList;
	// advance pointer to end of the list
	while (symIndex->next != NULL) {symIndex = symIndex->next; }

	struct symEntry* newEntry = new_sym_entry(UNKNOWN, 0, 0);
	for (i = 0; i < cCon->boardSize; i++) {
		newEntry->sym[i] = symToAdd[i];
	}
	symIndex->next = newEntry;
	numSymmetries++;
}

// composes symmetries together, adding new compositions if they are not equal
// to a symmetry already on the list
void composeSymmetries() {
	int composition = 0;
	int symType = UNKNOWN;
	struct symEntry *x, *y;
	// don't want to start with identity
	x = symmetriesList->next;
	y = symmetriesList->next;
	struct symEntry *tableEnd = symmetriesList;
	while (tableEnd->next != NULL) {tableEnd = tableEnd->next; }

	//printf("composing symmetries...\n");

	while (x != NULL) {
		while (y != NULL) {
			// rot(x)rot(y) = rot(x+y)
			//printf(" x is of type %d y is of type %d \n", x->type, y->type);
			if (x->type == ROTATION && y->type == ROTATION) {
				symType = ROTATION;
				composition = x->angle + y->angle;
			}
			// ref(x)ref(y) = rot(2(x-y))
			else if (x->type == REFLECTION && y->type == REFLECTION) {
				symType = ROTATION;
				composition = 2*(x->angle - y->angle);
			}
			// rot(i)ref(j) = ref(j + i/2)
			else if (x->type == ROTATION && y->type == REFLECTION) {
				symType = REFLECTION;
				composition = y->angle + x->angle/2;
			}
			// ref(i)rot(j) = ref(i - j/2)
			else if (x->type == REFLECTION && y->type == ROTATION) {
				symType = REFLECTION;
				composition = x->angle - y->angle/2;
			}
			// identity or unknown transformation.
			else {
				y = y->next;
				continue;
			}
			composition = (composition + 720) % 360; // so it's not negative
			//printf("composition = %d and is of type %d\n", composition, symType);
			if (!sym_exists(symType, composition)) {
				tableEnd->next = new_sym_entry(symType, composition, 0);
				tableEnd = tableEnd->next;
				numSymmetries++;
			}
			y = y->next;
		}
		y = symmetriesList->next;
		x = x->next;
	}

}

// returns TRUE if the symmetry is already in the table.
BOOLEAN sym_exists(int symType, int symAngle) {
	BOOLEAN result = FALSE;
	struct symEntry *symIndex = symmetriesList;
	if (symType == IDENTITY || (symType == ROTATION && symAngle == 0))
		return TRUE;

	while (symIndex != NULL) {
		//printf("looking at angle %d and type %d\n", symIndex->angle, symIndex->type);
		// if it's the right type and angle or if it's a reflection and we see an
		// equivalent angle
		if (symIndex->type == symType &&
		    ((symIndex->angle == symAngle) ||
		     (symIndex->type == REFLECTION &&
		(symIndex->angle == symAngle+180 || symIndex->angle == symAngle-180)))) {
			//printf("found it!\n");
			result = TRUE;
			break;
		}
		symIndex = symIndex->next;
	}
	//if (!result)
	//printf("sym of type %d and angle %d not found\n", symType, symAngle);
	return result;
}

/* converts col'th element in a row into the corresponding number.
   For example, row = 2, pos = 1 on the below board translates to
                         the number 6.  Rows are 0-based, as are
      0 1                columns.  Column refers to the col'th
     2 3 4               element in a row so it changes as you change
      5 6                rows.
 */
int convert(int row, int col, int boardType) {
	int i;
	int result = 0;
	int numCols = symBoardCols;

	for (i = 0; i < symBoardRows; i++) {
		if (i == row)
			break;
		result += numCols;
		if (boardType == HEX && i < symBoardRows/2)
			numCols++;
		else if (boardType == HEX && i >= symBoardRows/2 && i < symBoardRows-1)
			numCols--;
	}

	result += col;
	return result;
}

