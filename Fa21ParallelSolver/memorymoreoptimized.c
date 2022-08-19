#include "memory.h"
#include <string.h>


/*Struct defined for solving a game*/
struct solverdata
{
char size;
char maxtier;
char data[];
};
/*Struct defined for playing a game*/
struct playerdata {};

/*Initializes the data structure for solver, which acts as a dictionary with
keys as keylen-bit integers and value of one (nonzero) byte each. Returns NULL if error,
and assumes keylen <= 64*/
solverdata* initializesolverdata(int keylen)
{
	solverdata* s = calloc(sizeof(char), 1l+(1l << keylen));
	if(!s) {return NULL;}
	s->size = keylen;
	return s;
}

/*Inserts the given (key,val) pair to a solver. It is assumed that any insert either occurs only once
or occurs with the same value for a given key, and assumes that val != 0*/
void solverinsert(solverdata* data, uint64_t key, unsigned char val)
{
	data->data[key] = val;
}

/*Reads a data value at the given key, returning 0 if the value has not been inserted*/
unsigned char solverread(solverdata* data, uint64_t key)
{
	return data->data[key];
}

static int getpointerlength(int size)
{
	return (size+9)>>3;
}

static void storepointer(int64_t pointer, int size, char* output) {
	for(int i = 0; i < size; i++)
	{
		*output = pointer;
		output++;
		pointer >>= 8;
	}
}

/*Writes to output the shard of 2^size length.
Returns -n if n is the unique nonzero value in the shard, 0 if the shard is empty, and the number of bytes written otherwise*/
static int64_t solversavefragment(int size, char* data, char* output)
{
  //fflush(stdout);
  int pointerlength = getpointerlength(size);
  /*Format of this file structure:
  Each block of size 2^n begins with a pointer.
  If pointer == 0, then the next byte contains the unique value stored in the given block.
  If pointer == 1, then left and right blocks are identical, and what follows is the structure for a block of size 2^(n-1)
  Otherwise, the left and right blocks are different. Then the pointer lists the length of the left block. What follows is the left block, then the right block.
  Since all blocks contain at least one pointer of length at least one byte, and at least one byte of data, subblocks are at least 2 bytes long.
  */
  if(size == 0) {
		return -*data;
  }
  int64_t leftlength = solversavefragment(size-1, data, output+pointerlength);
  //if(leftlength != 0) printf("LeftLength: %d\n", leftlength);
  if(leftlength>0) {
	  int64_t rightlength = solversavefragment(size-1, data+(1l<<(size-1)), output+pointerlength+leftlength);
	  //if(rightlength != 0) printf("RightLength: %d\n", rightlength);
	  if(rightlength > 0)
	  {
		storepointer(leftlength, pointerlength, output);
		return leftlength+rightlength+pointerlength;
	  }
	  else if(rightlength == 0)
	  {
	    storepointer(1l, pointerlength, output);
		return leftlength+pointerlength;
	  }
	  else
	  {
		int subpointerlength = getpointerlength(size-1);
	    storepointer(leftlength, pointerlength, output);
		storepointer(0, subpointerlength, output+pointerlength+leftlength);
		*(output+pointerlength+leftlength+subpointerlength) = -rightlength;
		return leftlength+pointerlength+subpointerlength+1l;
	  }
  }
  else if(leftlength == 0)
  {
		int64_t rightlength = solversavefragment(size-1, data+(1l<<(size-1)), output+pointerlength);
		//if(rightlength != 0) printf("RightLength: %d\n", rightlength);
		if(rightlength <= 0) return rightlength;
		storepointer(1l, pointerlength, output);
		return rightlength+pointerlength;
  }
  else
  {
	  int subpointerlength = getpointerlength(size-1);
	  int64_t rightlength = solversavefragment(size-1, data+(1l<<(size-1)), output+pointerlength+subpointerlength+1l);
	  //if(rightlength != 0) printf("RightLength: %d\n", rightlength);
	  if(rightlength > 0)
	  {
	    storepointer(subpointerlength+1l, pointerlength, output);
		storepointer(0l, subpointerlength, output+pointerlength);
		*((output+pointerlength+subpointerlength)) = -leftlength;
		return rightlength+pointerlength+subpointerlength+1l;
	  }
	  else if(rightlength == 0)
	  {
		return leftlength;
	  }
	  else
	  {
		if(rightlength == leftlength) return leftlength;
	    storepointer(subpointerlength+1l, pointerlength, output);
		storepointer(0l, subpointerlength, output+pointerlength);
		*((output+pointerlength+subpointerlength)) = -leftlength;
		storepointer(0l, subpointerlength, output+pointerlength+subpointerlength+1);
		*((output+pointerlength+(subpointerlength<<1)+1)) = -rightlength;
		return pointerlength+(subpointerlength<<1)+2l;
	  }
  }
}
/*Writes to the given file (assumed to be opened and readable) the current solver state.
Is not required to leave the solverdata object unmodified.
The output file is designed to be used with the playerdata object,
but only guarantees values for stored keys; any key not set is set to a random value*/
void solversave(solverdata* data, FILE* fp)
{
	char* result = calloc(sizeof(char), (1l << (data->size-2)));
	if(result == NULL)
	{
		printf("Memory allocation error");
	}
	int length = solversavefragment(data->size, data->data, result);
	fwrite(&(data->size), sizeof(char), 1, fp);
	if(length <= 0) {
		printf("Compression complete. New length: 2 bytes\n");
		result[0] = 0;
		result[1] = -length;
		fwrite(result, sizeof(char), 2, fp);
	}
	else {
		printf("Compression complete. New length: %d bytes\n", length);
		fwrite(result, sizeof(char), length, fp);
	}
	free(result);
}

/*Frees a solver*/
void freesolver(solverdata* data)
{
	free(data);
}

/*Initializes the data structure for a player, read from a given filename*/
playerdata* initializeplayerdata(int keylen, char* filename)
{
	return NULL; //TODO
}

/*Reads a data value at the given key. Returns a random value if the key had not received a
defined value in the corresponding solver.*/
unsigned char playerread(playerdata* data, uint64_t key)
{
	return 0; //TODO
}

/*Frees a player*/
void freeplayer(playerdata* data)
{
	return; //TODO
}
bool verifyPlayerData(solverdata* sd, playerdata* pd) {
    int size = (int)sd->size;
    for (uint64_t i = 0; i < (1ul << size); ++i) {
        if (i % 1000000 == 0) {
            printf("verifying %lld\n", i);
            fflush(stdout);
        }
        unsigned char solverVal = solverread(sd, i);
        if (solverVal != 0) {
            unsigned char playerVal = playerread(pd, i);
            if (solverVal != playerVal) {
                printf("inconsistent value at key %llx: solver value %d and player value %d\n", i, solverVal, playerVal);
                fflush(stdout);
                return false;
            }
        }
    }
    return true;
}
