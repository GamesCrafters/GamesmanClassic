#include "hash.h"

/*********************************************************************************
 *** A *PERFECT* hash function - designed by Dan Garcia
 *** 			       - implemented by Attila Gyulassy
 *********************************************************************************/

void generic_hash_init(int boardsize, int minOs, int maxOs, int minXs, int maxXs)
{
	int i, j, temp, sofar, numOs, numXs;
	numOs = maxOs-minOs;
	numXs = maxXs-minXs;
	gHashBoardSize = boardsize;
	gHashOffset = (int*) malloc(sizeof(int) * ((numOs+1) * (numXs+1) + 1));
	for(i = 0; i < numOs * numXs; i++){
	  gHashOffset[i] = 0;
	}
	gHashOffsetSize = numOs * numXs;
	gHashMinMax[0] = minOs;
	gHashMinMax[1] = maxOs;
	gHashMinMax[2] = minXs;
	gHashMinMax[3] = maxXs;
	nCr_init(boardsize);
	for (i=0; i <= numOs; i++)
	{
		for (j = 0; j <= numXs; j++)
		{
			gHashOffset[i*(numXs+1) + j] = nCr((j+minXs) + (i+minOs), (i+minOs)) * nCr(boardsize, (minOs + minXs + i + j));
		}		
	}
	temp = 0;
	sofar = 0;
	for (i=0; i <= numOs; i++)
	{
		for (j = 0; j <= numXs; j++)
		{
			temp = gHashOffset[i*(numXs+1) + j];
			gHashOffset[i*(numXs+1) + j] = sofar;
			sofar += temp;
		}
	}
	gHashOffset[(numOs+1)*(numXs+1)] = sofar;
	gHashNumberOfPos = sofar;
}

int generic_hash(char* board)
{
	int temp, i, numxs,  numos, numXs;
	int boardsize, maxnumxs, maxnumos;
	maxnumos = maxnumxs = 0;
	boardsize = gHashBoardSize;
	numXs = gHashMinMax[3]-gHashMinMax[2] + 1;
	for (i = 0; i < boardsize; i++)
	{
		if (board[i] == 'x')
		{
			maxnumxs++;
		}
		else if (board[i] == 'o')
		{
			maxnumos++;
		}
	}
	numxs = maxnumxs;
	numos = maxnumos;
	temp = gHashOffset[maxnumos*numXs + maxnumxs];
	for (i = 0; i < gHashBoardSize; i++)
	{

	  if (board[i] == 'b')
	    {
	      boardsize--;
	    } 
	  else if (board[i] == 'o')
	    {
	      temp += nCr(numxs + numos, numos) * nCr(boardsize - 1, numxs + numos);
	      boardsize--;
	      numos--;
	    }
	  else
	    {
	      temp += nCr(numxs + numos, numos) * 
		nCr(boardsize - 1, numxs + numos) +
		nCr (numxs + numos - 1, numxs) *
		nCr(boardsize - 1,  numxs + numos - 1);
	      boardsize--;
	      numxs--;
	    }
	}
	return temp;
}

char* generic_unhash(int hashed, char* dest)
{
	int i, j, offst, numxs, numos, temp, boardsize, width, height;
	offst = 0;
	j = 0;
	width = gHashMinMax[3]-gHashMinMax[2]+1;
	height = gHashMinMax[1]-gHashMinMax[0]+1;
	while (j <= height*width &&
	       hashed > gHashOffset[(j + 1)])
	  {
	    j++;
	  }	
	offst = gHashOffset[j];
	hashed -= offst;
	numxs = j % width;
	numos = (int) j / width;
	boardsize = gHashBoardSize;
	for (i = 0; i < gHashBoardSize; i++)
	{
	  temp = nCr(numxs + numos, numos) * 
	    nCr(boardsize - 1, numxs + numos);
	  if (numxs + numos <= 0){
	    dest[i] = 'b';
	  } else if (temp + nCr (numxs + numos - 1, numxs) * nCr(boardsize - 1, numxs + numos - 1) <= hashed)
	    {
	      hashed -= (temp + nCr (numxs + numos - 1, numxs) * nCr(boardsize - 1,  numxs + numos - 1));
	      dest[i] = 'x';
	      numxs--;
	    }
	  else if (temp <= hashed)
	    {
	      hashed -= temp;
	      dest[i] = 'o';
	      numos--;  
	    }
	  else
	    {
	      dest[i] = 'b';
	    }
	  boardsize--;
	}
	return dest;
}

void dartboard_hash_init(int boardsize, int minOs, int maxOs, int minXs, int maxXs)
{
	int i, j, temp, sofar, numOs, numXs;
	numOs = maxOs-minOs;
	numXs = maxXs-minXs;
	gHashBoardSize = boardsize;
	gHashOffset = (int*) malloc(sizeof(int) * (numOs +  numXs + 1));
	for(i = 0; i < numOs + numXs + 1; i++){
	  gHashOffset[i] = 0;
	}
	gHashOffsetSize = numOs + numXs + 1;
	gHashMinMax[0] = minOs;
	gHashMinMax[1] = maxOs;
	gHashMinMax[2] = minXs;
	gHashMinMax[3] = maxXs;
	nCr_init(boardsize);
	for (i=0; i <= numOs + numXs; i++)
	{
	  gHashOffset[i] = nCr((i/2+minXs) + (i/2 + i%2 + minOs), (i/2+minXs)) * nCr(boardsize, (i/2 + i%2 + minOs + i/2 + minXs ));
	}
	temp = 0;
	sofar = 0;
	for (i=0; i <= numOs + numXs; i++)
	{
	  temp = gHashOffset[i];
	  gHashOffset[i] = sofar;
	  sofar += temp;
	}
	gHashOffset[numOs + numXs + 1] = sofar;
	gHashNumberOfPos = sofar;
}

int dartboard_hash(char* board)
{
	int temp, i, numxs,  numos, numXs;
	int boardsize, maxnumxs, maxnumos;
	maxnumos = maxnumxs = 0;
	boardsize = gHashBoardSize;
	numXs = gHashMinMax[3]-gHashMinMax[2] + 1;
	for (i = 0; i < boardsize; i++)
	{
		if (board[i] == 'x')
		{
			maxnumxs++;
		}
		else if (board[i] == 'o')
		{
			maxnumos++;
		}
	}
	numxs = maxnumxs;
	numos = maxnumos;
	if (numxs > numos){
	  temp = gHashOffset[numos*2+1];
	} else {
	  temp = gHashOffset[numos*2];
	}
	for (i = 0; i < gHashBoardSize; i++)
	{
	  if (board[i] == 'b')
	    {
	      boardsize--;
	    } 
	  else if (board[i] == 'o')
	    {
	      temp += nCr(numxs + numos, numos) * nCr(boardsize - 1, numxs + numos);
	      boardsize--;
	      numos--;
	    }
	  else
	    {
	      temp += nCr(numxs + numos, numos) * 
		nCr(boardsize - 1, numxs + numos) +
		nCr (numxs + numos - 1, numxs) *
		nCr(boardsize - 1,  numxs + numos - 1);
	      boardsize--;
	      numxs--;
	    }
	}
	return temp;
}

char* dartboard_unhash(int hashed, char* dest)
{
	int i, j, offst, numxs, numos, temp, boardsize, width, height;
	offst = 0;
	j = 0;
	while (j <= gHashOffsetSize &&
	       hashed > gHashOffset[j + 1])
	  {
	    j++;
	  }	
	offst = gHashOffset[j];
	hashed -= offst;
	numxs = j / 2 + j % 2;
	numos = j / 2;
	boardsize = gHashBoardSize;
	for (i = 0; i < gHashBoardSize; i++)
	{
	  temp = nCr(numxs + numos, numos) * 
	    nCr(boardsize - 1, numxs + numos);
	  if (numxs + numos <= 0){
	    dest[i] = 'b';
	  } else if (temp + nCr (numxs + numos - 1, numxs) * nCr(boardsize - 1, numxs + numos - 1) <= hashed)
	    {
	      hashed -= (temp + nCr (numxs + numos - 1, numxs) * nCr(boardsize - 1,  numxs + numos - 1));
	      dest[i] = 'x';
	      numxs--;
	    }
	  else if (temp <= hashed)
	    {
	      hashed -= temp;
	      dest[i] = 'o';
	      numos--;  
	    }
	  else
	    {
	      dest[i] = 'b';
	    }
	  boardsize--;
	}
	return dest;
}

void rearranger_hash_init(int boardsize, int numOs, int numXs)
{
  gHashMinMax[0] = numOs;
  gHashMinMax[1] = numOs;
  gHashMinMax[2] = numXs;
  gHashMinMax[3] = numXs;
  gHashBoardSize = boardsize;
  nCr_init(boardsize);
  gHashNumberOfPos = nCr(numXs+numOs, numXs) * nCr(boardsize, numXs+numOs);   
}

int rearranger_hash(char* board)
{
	int temp, i, numxs,  numos, numXs;
	int boardsize;
	numxs = gHashMinMax[3];
	numos = gHashMinMax[1];
	boardsize = gHashBoardSize;
	temp = 0;
	for (i = 0; i < gHashBoardSize; i++)
	{
	  if (board[i] == 'b')
	    {
	      boardsize--;
	    } 

	  else if (board[i] == 'o')
	    {
	      temp += nCr(numxs + numos, numos) * nCr(boardsize - 1, numxs + numos);
	      boardsize--;
	      numos--;
	    }
	  else
	    {
	      temp += nCr(numxs + numos, numos) * 
		nCr(boardsize - 1, numxs + numos) +
		nCr (numxs + numos - 1, numxs) *
		nCr(boardsize - 1,  numxs + numos - 1);
	      boardsize--;
	      numxs--;
	    }
	}
	return temp;
}

char* rearranger_unhash(int hashed, char* dest)
{
	int i, j, offst, numxs, numos, temp, boardsize;
	j = 0;
	boardsize = gHashBoardSize;
	numxs = gHashMinMax[3];
	numos = gHashMinMax[1];
	for (i = 0; i < gHashBoardSize; i++)
	{
	  temp = nCr(numxs + numos, numos) * 
	    nCr(boardsize - 1, numxs + numos);
	  if (numxs + numos <= 0){
	    dest[i] = 'b';
	  } else if (temp + nCr (numxs + numos - 1, numxs) * nCr(boardsize - 1, numxs + numos - 1) <= hashed)
	    {
	      hashed -= (temp + nCr (numxs + numos - 1, numxs) * nCr(boardsize - 1,  numxs + numos - 1));
	      dest[i] = 'x';
	      numxs--;
	    }
	  else if (temp <= hashed)
	    {
	      hashed -= temp;
	      dest[i] = 'o';
	      numos--;  
	    }
	  else
	    {
	      dest[i] = 'b';
	    }
	  boardsize--;
	}
	return dest;
}

void nCr_init(boardsize)
{
	int i, j;
	gNCR = (int*) malloc(sizeof(int) * (boardsize + 1) * (boardsize + 1));
	for(i = 0; i<= boardsize; i++)
	{
		gNCR[i*(boardsize+1)] = 1;
		gNCR[i*(boardsize+1) + i] = 1;
	}
	for(i = 1; i<= boardsize; i++)
	{
		for(j = 1; j < i ; j++)
		{
			gNCR[i*(boardsize+1) + j] = gNCR[(i-1)*(boardsize+1) + j-1] + gNCR[(i-1)*(boardsize+1) + j];
		}
	}
}

int nCr(int n, int r)
{
  return gNCR[n*(gHashBoardSize+1) + r];
}





