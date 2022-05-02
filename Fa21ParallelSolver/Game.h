#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#define LOSS 1
#define TIE 128
#define NOT_PRIMITIVE 255

typedef uint64_t game;
typedef uint64_t gamehash;


void initialize_constants();

game getStartingPositions();

/** Returns the result of moving at the given position
    * Bit 63 is flipped to change current player
    * Move is assumed to be the bit position of the empty cell.
    * As such, we add 1<<move for a yellow, and 1<<(move+1) for red
    * 0b0001011 + 1<<move = 0b0010011 = ---YYRR
    * 0b0001011 + 1<<(move+1) = 0b0011011 = ---RYRR*/
game doMove(game position, char move);

/** Returns the list of viable moves.
Assumes retval has length at least MaxMoves*/
int generateMoves(char* retval, game position);
/** Returns if the given position is primitive, assuming that the most recent move is as stated*/
char isPrimitive(game position, char mostrecentmove);


int getSize();

int getMaxDepth();

int getMaxMoves();

gamehash getHash(game position);

gamehash maxHash();

int hashLength();

game hashToPosition(gamehash hash);

/*Returns a list of shard ids which the given parent shard has as children. Ideally, this should be fast enough that the shard graph can be determined by one thread. Return value is the number of children shards.*/
int getchildrenshards(uint64_t** childrenshards, char shardsize, uint64_t parentshard); //TODO

