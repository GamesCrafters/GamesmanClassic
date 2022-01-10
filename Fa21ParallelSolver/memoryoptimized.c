#include "memory.h"
#include <string.h>


/*Struct defined for solving a game*/
struct solverdata
{
char size;
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
void solverinsert(solverdata* data, uint64_t key, char val)
{
	data->data[key] = val;
}

/*Reads a data value at the given key, returning 0 if the value has not been inserted*/
char solverread(solverdata* data, uint64_t key)
{
	return data->data[key];
}

/*Sets all zero values in the given fragment. Returns 0 if all data in that segment is 0, 1 otherwise*/
static int solversavefragment(int size, char* data)
{
  if(size == 1) return *data > 0 ? 1 : 0;
  int first = solversavefragment(size-1, data);
  int second = solversavefragment(size-1, data+(1l<<(size-1)));
  if(first == 0)
  {
  	  if(second == 0) return 0;
	  memset(data, *(data+(1l<<(size-1))), 1l<<(size-1));
	  return 1;
  }
  if(second == 0) memset(data+(1l<<(size-1)), *(data+(1l<<(size-1))-1), 1l<<(size-1));
  return 1;
}
/*Writes to the given file (assumed to be opened and readable) the current solver state.
Is not required to leave the solverdata object unmodified.
The output file is designed to be used with the playerdata object,
but only guarantees values for stored keys; any key not set is set to a random value*/
void solversave(solverdata* data, FILE* fp)
{
	solversavefragment(data->size, data->data);
	fwrite(data, sizeof(char), 1l+(1l<<(data->size)), fp);
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
char playerread(playerdata* data, uint64_t key)
{
	return 0; //TODO
}

/*Frees a player*/
void freeplayer(playerdata* data)
{
	return; //TODO
}