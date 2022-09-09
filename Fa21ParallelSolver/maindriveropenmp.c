#include "Game.h"
#include "memory.h"
#include "solver.h"
#include <omp.h>
#include <unistd.h>
#include <time.h>

#define shardsize 26

//Sends a message to all children/parents that the shard is done computing, and adds workable shards to the work queue
//issolved is 0 if during discovery, and 1 if during solving.
static void addshardstoqueue(shardgraph** topshard, shardgraph** bottomshard, shardgraph* completedshard, int issolved) {
	if (issolved) {
		for(int i = 0 ; i < completedshard->parentcount;i++) {
			shardgraph* parentshard = completedshard->parentshards[i];
			parentshard->childrensolved++;
			if (parentshard->childrensolved == parentshard->childrencount) {
				// printf("Added shard %d to queue for solving\n", parentshard->shardid);
				// fflush(stdout);
				if (*topshard == NULL) {
					*topshard = parentshard;
					*bottomshard = parentshard;
				} else {
					(*bottomshard)->nextinqueue = parentshard;
					*bottomshard = parentshard;
				}
			}
		}
	} else {
		for(int i = 0 ; i < completedshard->childrencount;i++) {
			shardgraph* childshard = completedshard->childrenshards[i];
			childshard->parentsdiscovered++;
			if (childshard->parentsdiscovered == childshard->parentcount) {
				// printf("Added shard %d to queue for discovery\n", childshard->shardid);
				// fflush(stdout);
				if (*topshard == NULL) {
					*topshard = childshard;
					*bottomshard = childshard;	
				} else {
					(*bottomshard)->nextinqueue = childshard;
					*bottomshard = childshard;
				}
			}
		}
		if (completedshard->childrencount == 0) {
			// printf("Added shard %d to queue for solving\n", completedshard->shardid);
			// fflush(stdout);
			if (*topshard == NULL) {
				*topshard = completedshard;
				*bottomshard = completedshard;	
			} else {
				(*bottomshard)->nextinqueue = completedshard;
				*bottomshard = completedshard;
			}
		}
	}
}


int main(int argc, char** argv) {
	if (argc != 2) {
		printf("Usage: %s <foldername>", argv[0]);
		return 1;
	}

	clock_t start, end;
	double cpu_time_used;

	start = clock();

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
	
	printf("Discovering shard %d/%d with shard id %d\n", shardsdiscovered, validshards, (topshard)->shardid);	
	fflush(stdout);
	discoverfragment(workingfolder, topshard, shardsize, true); //Initialize work queue and compute first shard
	shardsdiscovered++;	
	addshardstoqueue(&topshard, &bottomshard, bottomshard, 0);
	topshard->discovered++;
	shardgraph* oldtopshard = topshard;
	topshard = topshard->nextinqueue;
	oldtopshard->nextinqueue = NULL;

	// Lock information
	omp_lock_t shardgraphLock;
	omp_init_lock(&shardgraphLock);
	omp_lock_t terminalLock;
	omp_init_lock(&terminalLock);
	bool isDone = false;

	//Compute remaining shards
	#pragma omp parallel private(oldtopshard)
	while(!isDone) {
		if (topshard == NULL) {
			// printf("Thread %d found no shard, going to sleep\n", omp_get_thread_num());
			// fflush(stdout);
			sleep(1);
			continue;
		}
		omp_set_lock(&shardgraphLock);
		if (topshard == NULL) {
			omp_unset_lock(&shardgraphLock);
			continue;
		}
		oldtopshard = topshard;
		topshard = topshard->nextinqueue;
		oldtopshard->nextinqueue = NULL;
		omp_unset_lock(&shardgraphLock);
		if (oldtopshard->discovered) {
			omp_set_lock(&terminalLock);
			shardssolved++;
			printf("Solving shard %d/%d with shard id %llu by thread %d\n", shardssolved, validshards, oldtopshard->shardid, omp_get_thread_num());
			fflush(stdout);
			omp_unset_lock(&terminalLock);
			solvefragment(workingfolder, oldtopshard, shardsize, oldtopshard == getstartingshard(shardList, shardsize));
			isDone = oldtopshard == getstartingshard(shardList, shardsize);
		} else {
			omp_set_lock(&terminalLock);
			shardsdiscovered++;
			printf("Discovering shard %d/%d with shard id %llu by thread %d\n", shardsdiscovered, validshards, oldtopshard->shardid, omp_get_thread_num());
			fflush(stdout);
			omp_unset_lock(&terminalLock);
			discoverfragment(workingfolder, oldtopshard, shardsize, false);
		}
		omp_set_lock(&shardgraphLock);
		addshardstoqueue(&topshard, &bottomshard, oldtopshard, oldtopshard->discovered);
		oldtopshard->discovered++;
		omp_unset_lock(&shardgraphLock);
	}
	end = clock();
	cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
	printf("Done computing\n");
	printf("Total time taken: %f seconds\n", cpu_time_used);
	fflush(stdout);
	freeshardlist(shardList, shardsize); //Clean up
}
