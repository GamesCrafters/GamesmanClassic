#include "solver.h"


void discoverstartingfragment(char* workingfolder, char fragmentsize) {
	return;
}
void discoverfragment(char* workingfolder, shardgraph* targetshard, char fragmentsize) {
	return;
}

void solvefragment(char* workingfolder, shardgraph* targetshard, char fragmentsize)
{
  /*gamehash* childrenshards;
  int shardcount = getchildrenshards(&childrenshards, fragmentsize, minhash);
  //Load solved fragments into array on GPU
  //Load list of target hashes into array on GPU
  //Create a new solver on GPU
  for(int i = ; i >= ; i--) // First two values in input file contain the max and min depth
  {
  	  //GPU runs one tier of the solver
  }
  //Collect the data from the GPU
  //Compress and save the shard into solvedfragmentfolder*/
  return;
}



static int initializeshard(shardgraph* shardlist, char* shardinitialized, uint32_t shardsize, uint32_t startingshard) {
	printf("Initializing shard %d\n", startingshard);
	fflush(stdout);
	if(!shardinitialized[startingshard]) {
		shardinitialized[startingshard] = 1;
		uint64_t* childrenshards;
		int childrencount = getchildrenshards(&childrenshards, shardsize, startingshard);
		shardlist[startingshard].shardid = startingshard;
		shardlist[startingshard].childrencount = childrencount;
		shardlist[startingshard].childrenshards = calloc(childrencount, sizeof(shardgraph*));
		int subshardsadded = 1;
		for(int i = 0; i < childrencount; i++) {
			subshardsadded+= initializeshard(shardlist, shardinitialized, shardsize, childrenshards[i]);
			shardlist[startingshard].childrenshards[i] = shardlist+childrenshards[i];
			shardlist[childrenshards[i]].parentcount++;
		}
		free(childrenshards);
		return subshardsadded;
	}
	else return 0;
}
static void initializeparentshard(shardgraph* shardlist, shardgraph* startingshard) {
	if(!startingshard->parentshards) {
		startingshard->parentshards = calloc(startingshard->parentcount, sizeof(shardgraph*));
		startingshard->parentcount = 0;
		for(int i = 0; i < startingshard->childrencount; i++)
		{
			initializeparentshard(shardlist, startingshard->childrenshards[i]);
			(startingshard->childrenshards[i])->parentshards[(startingshard->childrenshards[i])->parentcount] = startingshard;
			((startingshard->childrenshards[i])->parentcount)++;
		}
	}
}

shardgraph* getstartingshard(shardgraph* shardlist, int shardsize)
{
	return shardlist+(getHash(getStartingPositions()) >> shardsize);
}
//Initializes all relevant shards. Returns the number of shards actually created.
int initializeshardlist(shardgraph** shardlistptr, uint32_t shardsize) {
	uint32_t shardcount = 1 << (hashLength() - shardsize);
	shardgraph* shardlist = calloc(shardcount, sizeof(shardgraph));
	*shardlistptr = shardlist;
	char* shardinitialized = calloc(shardcount, sizeof(char));
	uint32_t startingshard = getHash(getStartingPositions()) >> shardsize;
	int validshards = initializeshard(shardlist, shardinitialized, shardsize, startingshard);
	printf("%p\n", shardlist);
	fflush(stdout);
	initializeparentshard(shardlist, shardlist+startingshard);
	free(shardinitialized);
	return validshards;
}
void freeshardlist(shardgraph* shardlist, uint32_t shardsize) {
	uint32_t shardcount = 1 << (hashLength() - shardsize);
	for(int i = 0; i < shardcount; i++) if(shardlist[i].childrenshards != NULL) {free(shardlist[i].childrenshards); free(shardlist[i].parentshards);}
	free(shardlist);
}





/*int main(int argc, char** argv)
{
	if(argc != 2)
	{
		printf("Usage: %s <filename>\n", argv[0]);
		return 1;
	}
  	initialize_constants();
    game pos = getStartingPositions();
    solverdata* primitives = initializesolverdata(hashLength());
	game* fringe = calloc(sizeof(game), getMaxMoves()*getMaxDepth());
	if(primitives == NULL || fringe == NULL) {
		printf("Memory allocation error\n");
		return 1;
	}
	int index = 1;
	fringe[0] = getStartingPositions();
	game g;
	game newg;
	gamehash h;
	char primitive;
	char minprimitive;
	char moves[getMaxMoves()];
	int movecount;
	int i;
	int oldindex;
	gamehash minindex = maxHash();
	gamehash maxindex = 0;
	int* positionsfound = calloc(sizeof(int), 256);
	printf("Beginning main loop\n");
	fflush(stdout);
	while(index)
	{
		minprimitive = 255;
		oldindex = index;
		g = fringe[index-1];
		h = getHash(g);
		if(solverread(primitives, h)== 0)
		{
		movecount = generateMoves((char*)&moves, g);
		for(i=0;i<movecount;i++)
		{
			newg = doMove(g, moves[i]);
			h = getHash(newg);
			primitive = solverread(primitives, h);
			if(!primitive)
			{
				primitive = isPrimitive(newg, moves[i]);
				if(primitive != (char) NOT_PRIMITIVE) {
					solverinsert(primitives,h,primitive);
					if(h < minindex) {minindex = h;}
					if(h > maxindex) {maxindex = h;}
					//printf("Position 0x%08x determined primitive\n", newg);
					positionsfound[primitive]++;
					minprimitive = minprimitive <= primitive ? minprimitive : primitive;
				}
				else {
					fringe[index] = newg;
					index++;
				}
			}
			else
			{
				minprimitive = minprimitive <= primitive ? minprimitive : primitive;
			}
		}
		if(index == oldindex) 
		{
			if(minprimitive & 128)
			{
				if(minprimitive & 64) minprimitive = 257-minprimitive;
				else minprimitive = minprimitive + 1;
			}
			else minprimitive = 255-minprimitive;
			h = getHash(g);
			solverinsert(primitives,h,minprimitive);
			if(h < minindex) minindex = h;
			if(h > maxindex) maxindex = h;
			//printf("Position 0x%08x determined fully\n", g);
			positionsfound[minprimitive]++;
			index--;
		}
		}
		else { index--;}
	}
	printf("Done computing, listing statistics\n");
	fflush(stdout);
	int totalpositioncount = 0;
	for(int i = 1; i < 64; i++)
	{
		if(positionsfound[i]) printf("Loss in %d: %d\n", i-1, positionsfound[i]);
		totalpositioncount+=positionsfound[i];
	}
	for(int i = 128; i < 192; i++)
	{
		if(positionsfound[i]) printf("Tie in %d: %d\n", i-128, positionsfound[i]);
		totalpositioncount+=positionsfound[i];
	}
	for(int i = 254; i >= 192; i--)
	{
		if(positionsfound[i]) printf("Win in %d: %d\n", 255-i, positionsfound[i]);
		totalpositioncount+=positionsfound[i];
	}
	printf("In total, %d positions found\n", totalpositioncount);
	printf("%d primitive positions found\n", positionsfound[LOSS]+positionsfound[TIE]);
	printf("Starting position has value %d\n", solverread(primitives, getStartingPositions()));
	printf("%llx, %llx\n", minindex, maxindex);
	printf("Starting output to file %s\n", argv[1]);
	fflush(stdout);
	FILE* file = fopen(argv[1], "w");
	solversave(primitives, file);
	freesolver(primitives);
	printf("Done outputting\n");
	fflush(stdout);
	free(fringe);
	free(positionsfound);
	return 0;
}*/
