#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

/*Struct defined for solving a game*/
typedef struct solverdata solverdata;
/*Struct defined for playing a game*/
typedef struct playerdata playerdata;

/*Initializes the data structure for solver, which acts as a dictionary with 
keys as keylen-bit integers and value of one (nonzero) byte each. Returns NULL if error,
and assumes keylen <= 64*/
solverdata* initializesolverdata(int keylen);

/*Inserts the given (key,val) pair to a solver. It is assumed that any insert either occurs only once
or occurs with the same value for a given key, and assumes that val != 0*/
void solverinsert(solverdata* data, uint64_t key, char val);


/*Reads a data value at the given key, returning 0 if the value has not been inserted*/
char solverread(solverdata* data, uint64_t key);


/*Writes to the given file (assumed to be opened and readable) the current solver state.
Is not required to leave the solverdata object unmodified.
The output file is designed to be used with the playerdata object,
but only guarantees values for stored keys; any key not set is set to a random value*/
void solversave(solverdata* data, FILE* fp);

/*Frees a solver*/
void freesolver(solverdata* data);

/*Initializes the data structure for a player, read from a given filename*/
playerdata* initializeplayerdata(int keylen, char* filename);

/*Reads a data value at the given key. Returns a random value if the key had not received a
defined value in the corresponding solver.*/
char playerread(playerdata* data, uint64_t key);

/*Frees a player*/
void freeplayer(playerdata* data);

bool verifyPlayerData(const solverdata* sd, const playerdata* pd)
