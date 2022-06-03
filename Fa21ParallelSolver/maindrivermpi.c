#include "Game.h"
#include "memory.h"
#include "solver.h"
#include <mpi.h>
#include <unistd.h>
#include <time.h>

#define shardsize 28
//General form of this program: Process 0 will act as the main driver, request other processes to compute various shards, and output progress data to the terminal. All other child processes wait for work to be assigned from process 0.
//Communication protocol from driver tp child:
/*
-1: Not enough work currently available. Sleep for 1 second to wait for more work.
-2: All work done. Terminate process.
0b00xxxx... : Discover shard 0bxxxx...
0b01xxxx... : Solve shard 0bxxxx...

Communication protocol from child to driver:

-1: No work done. Checking in for more work.
0b00xxxx... : Shard 0bxxxx done computing.
*/
#define NOT_ENOUGH_WORK (-1)
#define TERMINATE (-2)
#define send_discovery_request(a) (a)
#define send_solve_request(a) ((a) | 1ULL<<62)
#define getshardID(a) ((a) & 0x3FFFFFFFFFFFFFFFULL)
#define issolve(a) (((a) & 1ULL<<62)>0)

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
	MPI_Init(&argc, &argv); //Initialize the MPI environment (for multiple node work). All code between here and finalize gets run by all nodes.
	int processID, clusterSize;
    MPI_Comm_size(MPI_COMM_WORLD, &clusterSize);
    MPI_Comm_rank(MPI_COMM_WORLD, &processID);

	if(clusterSize<=1) {
		printf("Not enough nodes for MPI; use standard code");
		MPI_Finalize();
		return 1;
	}
	
	clock_t start, end;
	double cpu_time_used; //Variables used for timekeeping. Technically only used by process 0, so can be optimized a bit here.
	start = clock();

  	initialize_constants(); //These constants are used in solving positions, and so should be initialized from all processes.
	char* workingfolder = argv[1];

	shardgraph* shardList;
	int validshards = initializeshardlist(&shardList, shardsize); //All processes should initialize the shard list; this shouldn't take much time, and isn't really worth it to parallelize.
	if(processID == 0) { //Only process 0 should send messages to stdout.
		printf("Shard graph computed: %d shards will be computed\n", validshards);
		fflush(stdout);

		int shardsdiscovered = 0;
		int shardssolved = 0;
		shardgraph* topshard = getstartingshard(shardList, shardsize);
		shardgraph* bottomshard = topshard; //pointers to front and back of work queue
	
		printf("Discovering shard %d/%d with shard id %d\n", shardsdiscovered, validshards, (topshard)->shardid);	
		fflush(stdout);
		//First shard can't be parallelized, and process 0 needs to finish shard computing anyway, so let process 0 run discovery on starting fragment.
		discoverfragment(workingfolder, topshard, shardsize, true); //Initialize work queue and compute first shard
		shardsdiscovered++;	
		addshardstoqueue(&topshard, &bottomshard, bottomshard, 0);
		topshard->discovered++;
		shardgraph* oldtopshard = topshard;
		topshard = topshard->nextinqueue;
		oldtopshard->nextinqueue = NULL;


		while(true) {
			MPI_Status status;
			uint64_t shardcompleted;
			MPI_Recv(&shardcompleted, 1, MPI_UINT64_T, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD,&status); //Receive a request from one child process for work
			if(shardcompleted!= -1) {
				//Some shard was completed. Update the work queue
				//Note: Assumes that shardList[shardcompleted] has shardid of shardcompleted
				addshardstoqueue(&topshard, &bottomshard, shardList+shardcompleted, (shardList[shardcompleted]).discovered);
				(shardList[shardcompleted]).discovered++;
				if(shardList+shardcompleted == getstartingshard(shardList, shardsize)) {
					//The starting shard was worked on. This indicates that it was solved (since discovery happened earlier), and as such, the solve is complete. Begin termination.
					uint64_t response = TERMINATE;
					MPI_Send(&response, 1, MPI_UINT64_T, status.MPI_SOURCE, 0, MPI_COMM_WORLD); //Send termination message. We will send termination messages to remaining processes after the while loop.
					break;
				}
			}
			if (topshard == NULL) {
				// printf("Process %d has no work, going to sleep\n", status.MPI_SOURCE);
				// fflush(stdout);
				uint64_t response = NOT_ENOUGH_WORK;
				MPI_Send(&response, 1, MPI_UINT64_T, status.MPI_SOURCE, 0, MPI_COMM_WORLD); //If there's currently no work to do, send a waiting message
				continue;
			}
			oldtopshard = topshard;
			topshard = topshard->nextinqueue;
			oldtopshard->nextinqueue = NULL;
			if (oldtopshard->discovered) {
				shardssolved++;
				printf("Solving shard %d/%d with shard id %llu by process %d\n", shardssolved, validshards, oldtopshard->shardid, status.MPI_SOURCE);
				fflush(stdout);
				uint64_t response = send_solve_request(oldtopshard->shardid);
				MPI_Send(&response, 1, MPI_UINT64_T, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
			} else {
				shardsdiscovered++;
				printf("Discovering shard %d/%d with shard id %llu by process %d\n", shardsdiscovered, validshards, oldtopshard->shardid, status.MPI_SOURCE);
				fflush(stdout);
				uint64_t response = send_discovery_request(oldtopshard->shardid);
				MPI_Send(&response, 1, MPI_UINT64_T, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
			}
		}
		printf("Done computing. Sending termination messages to remaining processes\n");
		int processesterminated = 1; //One process was terminated in the main loop
		while(processesterminated < (clusterSize - 1)) { //Process 0 doesn't need a termination message
			MPI_Status status;
			uint64_t shardcompleted;
			MPI_Recv(&shardcompleted, 1, MPI_UINT64_T, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD,&status);
			uint64_t response = TERMINATE;
			MPI_Send(&response, 1, MPI_UINT64_T, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
		}
		end = clock();
		cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
		printf("Total time taken: %f seconds\n", cpu_time_used);
		fflush(stdout);
	} else {
		uint64_t senddata = -1; //Done setting up shard graph. Send to main that this process is ready to work.
		MPI_Send(&senddata, 1, MPI_UINT64_T, 0, 0, MPI_COMM_WORLD);
		while(true) {
			uint64_t parentmessage;
			MPI_Recv(&parentmessage, 1, MPI_UINT64_T, 0, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
			if(parentmessage == NOT_ENOUGH_WORK) {
				sleep(1);
				senddata = -1;
				MPI_Send(&senddata, 1, MPI_UINT64_T, 0, 0, MPI_COMM_WORLD);
			}
			else if(parentmessage == TERMINATE) {
				break;
			}
			else {
				uint64_t targetshardID = getshardID(parentmessage);
				shardgraph* targetshard = shardList+targetshardID;
				if(issolve(parentmessage)) {
					solvefragment(workingfolder, targetshard, shardsize, targetshard == getstartingshard(shardList, shardsize));
				}
				else {
					discoverfragment(workingfolder, targetshard, shardsize, false);
				}
			}
		}
	}




	freeshardlist(shardList, shardsize); //Clean up
	MPI_Finalize();
}
