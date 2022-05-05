#include "Game.h"
#include "memory.h"
#include "solver.h"

#define shardsize 28

//Sends a message to all children/parents that the shard is done computing, and adds workable shards to the work queue
//issolved is 0 if during discovery, and 1 if during solving.
static shardgraph* addshardstoqueue(shardgraph* bottomshard, shardgraph* completedshard, int issolved) {
	if(issolved) {
		for(int i = 0 ; i < completedshard->parentcount;i++) {
			shardgraph* parentshard = completedshard->parentshards[i];
			parentshard->childrensolved++;
			if(parentshard->childrensolved == parentshard->childrencount)
			{
				//printf("Added shard %d to queue for solving\n", parentshard->shardid);
				bottomshard->nextinqueue = parentshard;
				bottomshard = parentshard;
			}
		}
	}
	else {
		for(int i = 0 ; i < completedshard->childrencount;i++) {
			shardgraph* childshard = completedshard->childrenshards[i];
			childshard->parentsdiscovered++;
			if(childshard->parentsdiscovered == childshard->parentcount)
			{
				//printf("Added shard %d to queue for discovery\n", childshard->shardid);
				bottomshard->nextinqueue = childshard;
				bottomshard = childshard;
			}
		}
		if(completedshard->childrencount == 0) {
			//printf("Added shard %d to queue for solving\n", completedshard->shardid);
			bottomshard->nextinqueue = completedshard;
			bottomshard = completedshard;
		}
	}
	return bottomshard;
}


int main(int argc, char** argv)
{
	if(argc != 2)
	{
		printf("Usage: %s <foldername>", argv[0]);
		return 1;
	}
  	initialize_constants();

	  shardgraph* shardList;
	int validshards = initializeshardlist(&shardList, shardsize);
	printf("Shard graph computed: %d shards will be computed\n", validshards);
	fflush(stdout);


	int shardsdiscovered = 0;
	int shardssolved = 0;
	shardgraph* topshard = getstartingshard(shardList, shardsize);
	shardgraph* bottomshard = topshard; //pointers to front and back of work queue
	char* workingfolder = argv[1];
	
	
	printf("Discovering shard %d/%d with shard id %d\n", shardsdiscovered, validshards, topshard->shardid);	
	discoverfragment(workingfolder, topshard, shardsize, true); //Initialize work queue and compute first shard
	shardsdiscovered++;	
	bottomshard = addshardstoqueue(bottomshard, bottomshard, 0);
	topshard->discovered++;
	shardgraph* oldtopshard = topshard;
	topshard = topshard->nextinqueue;
	oldtopshard->nextinqueue = NULL;

	//Compute remaining shards
	while(topshard!= NULL) {
		oldtopshard = topshard;
		if(oldtopshard->discovered) {
			printf("Solving shard %d/%d with shard id %d\n", shardssolved, validshards, oldtopshard->shardid);
			solvefragment(workingfolder, oldtopshard, shardsize);
			shardssolved++;
		}
		else {
			printf("Discovering shard %d/%d with shard id %d\n", shardsdiscovered, validshards, oldtopshard->shardid);
			discoverfragment(workingfolder, oldtopshard, shardsize, false);
			shardsdiscovered++;
		}
		bottomshard = addshardstoqueue(bottomshard, oldtopshard, oldtopshard->discovered);
		topshard = topshard->nextinqueue;
		oldtopshard->nextinqueue = NULL;
		oldtopshard->discovered++;

	}
	printf("Done computing\n");
	freeshardlist(shardList, shardsize); //Clean up
}
