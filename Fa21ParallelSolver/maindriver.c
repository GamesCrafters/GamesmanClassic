#include "Game.h"
#include "memory.h"
#include "solver.h"

#define shardsize 28

typedef struct shardgraph {
	gamehash minhash;
	int childrencount;
	int parentcount;
	int parentsfinished;
	int childrensolved;
	shardgraph** childrenshards;
} shardgraph;
static int initializeshard(shardgraph* shardlist, char* shardinitialized, uint32_t shardcount, uint32_t startingshard) {
	
}
//Initializes all relevant shards. Returns the number of shards actually created.
static int initializeshardlist(shardgraph* shardlist, uint32_t shardcount) {
	char* shardinitialized = calloc(shardcount, sizeof(char));
	uint32_t startingshard = getHash(getStartingPositions()) >> shardsize;
	int validshards = initializeshard(shardlist, shardinitialized, shardcount, startingshard);
}
static void freeshardlist(shardgraph* shardlist, uint32_t shardcount) {
	for(int i = 0; i < shardcount; i++) if(shardlist[i].childrenshards != NULL) free(shardlist[i].childrenshards);
	free(shardlist);
}

int main(int argc, char** argv)
{
	if(argc != 2)
	{
		printf("Usage: %s <foldername>", argv[0]);
		return 1;
	}
  	initialize_constants();
	uint32_t shardcount = 1 << (hashLength() - shardsize);
	shardgraph* shardList = calloc(shardcount, sizeof(shardgraph));
	int validshards = initializeshardlist(shardList, shardcount);
	freeshardlist();
}
