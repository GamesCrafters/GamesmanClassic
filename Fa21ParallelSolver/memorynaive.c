#include "memory.h"


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
	solverdata* s = calloc(sizeof(char), 1l+(1 << keylen));
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

/*Writes to the given file (assumed to be opened and readable) the current solver state.
Is not required to leave the solverdata object unmodified.
The output file is designed to be used with the playerdata object,
but only guarantees values for stored keys; any key not set is set to a random value*/
void solversave(solverdata* data, FILE* fp)
{
	fwrite(data, sizeof(char), 1l+(1<<(data->size)), fp);
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
