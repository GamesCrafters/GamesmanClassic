#include "solver.h"


void discoverfragment(char* workingfolder, shardgraph* targetshard, char fragmentsize, bool isstartingfragment) {
	uint64_t currentshardid = targetshard->shardid;

	// Gives list of children shards that we send discovery children to that we send to
	int childrenshardcount = targetshard->childrencount;
	solverdata** childrenshards = malloc(sizeof(solverdata*) * childrenshardcount);

	game g;
	game newg;
	gamehash h;
	gamehash minindex = maxHash();
	gamehash maxindex = 0;
	char primitive;

	int index = 0;

	// Next Tier: Use CUDA Malloc when moving to GPU for "moves" and "fringe"
	solverdata* localpositions = initializesolverdata(fragmentsize);
	for(int i = 0; i < childrenshardcount; i++) {
		childrenshards[i] = initializesolverdata(fragmentsize);
		if(childrenshards[i] == NULL) {
			printf("Memory allocation error\n");
			fflush(stdout);
			return;
		}
	}
	char moves[getMaxMoves()];
	game* fringe = calloc(sizeof(game), getMaxMoves() * getMaxDepth());
	if (localpositions == NULL || fringe == NULL) {
		printf("Memory allocation error\n");
		fflush(stdout);
		return;
	}

	// Add incoming Discovery states from parent
	// Multiple top node in shard
	//Set up the file name
	int filenamemaxlength = strlen("/transfer-100000000-100000000-x-primitive")+1;
	char* filename = malloc(sizeof(char)*(filenamemaxlength + strlen(workingfolder)));
	if(filename == NULL) {
		printf("Memory allocation error\n");
		fflush(stdout);
		return;
	}
	strncpy(filename, workingfolder, strlen(workingfolder));
	char* filenamewriteaddr = filename+strlen(workingfolder);
	const int SHARDOFFSETMASK = (1ULL << fragmentsize) - 1;
	if(isstartingfragment) {
		fringe[0] = getStartingPositions();
		index = 1;

		int movecount, k, oldindex, newpositionshard;
		int positionschecked = 0;
		while (index) {
			oldindex = index;
			g = fringe[index - 1];
			index--; //For discovery, there's no need to keep the current position on the fringe, since we don't need to go back to it.
			h = getHash(g);
			if (solverread(localpositions, h&(SHARDOFFSETMASK)) == 0) { //If we don't find this position in our solver, expand it for discovery
				solverinsert(localpositions, h&(SHARDOFFSETMASK), 1); //Insert 1 here to that position to signify that it has been expanded. Do this now to minimize the time other threads can access this and try to duplicate work.
					
				movecount = generateMoves((char*) &moves, g);
				for (k = 0; k < movecount; k++) { //Iterate through all children of the current position
					newg = doMove(g, moves[k]);
					h = getHash(newg);
					newpositionshard = h >> fragmentsize;
					if(newpositionshard == currentshardid && !(solverread(localpositions, h&SHARDOFFSETMASK))) { //If we have a position in our current shard that hasn't been expanded, add it to the fringe
						if(isPrimitive(newg, moves[k]) == NOT_PRIMITIVE)
							fringe[index++] = newg;
						else
							solverinsert(localpositions, h&(SHARDOFFSETMASK), 2);
					}
					else if(newpositionshard != currentshardid) { // If the child is not in the current shard, insert it into the appropriate child shard
						for(int l = 0; l < targetshard->childrencount;l++) {
							if(newpositionshard == targetshard->childrenshards[l]->shardid) {
								if(!solverread(childrenshards[l], h&SHARDOFFSETMASK))
									solverinsert(childrenshards[l], h&SHARDOFFSETMASK, isPrimitive(newg, moves[k]));
								break;
							}
						}
					}
				}
			}
		}
	}
	else {
	//Find all children in childrenshards
		for (int i = 0; i < targetshard->parentcount; i++) {
			//Open the file and read the first word to determine how many children are to be read
			snprintf(filenamewriteaddr,filenamemaxlength, "/transfer-%llu-%llu", targetshard->parentshards[i]->shardid, targetshard->shardid);
			FILE* parentfile = fopen(filename, "rb");
			if(parentfile == NULL) {
				printf("File Open Error: %s\n", filename);
				fflush(stdout);
				return;
			}
			int positioncountfromparent;
			fread(&positioncountfromparent, sizeof(int), 1, parentfile);
			//printf("%d positions will be checked from parent %llu\n", positioncountfromparent, targetshard->parentshards[i]->shardid);
			for(int j = 0; j < positioncountfromparent; j++) {
				//For each position in the file, add it to the fringe and run graph traversal from there
				uint32_t newpositionoffset;
				fread(&newpositionoffset, sizeof(uint32_t), 1, parentfile);
				if(solverread(localpositions, newpositionoffset)) continue; //If the position already is in the solver, we've already traversed from there, so ignore the position.		
				gamehash newpositionhash = (((uint64_t) targetshard->shardid) << fragmentsize) + newpositionoffset; //Otherwise, set up the game corresponding to the saved offset.
				fringe[0] = hashToPosition(newpositionhash);
				index = 1;

				int movecount, k, oldindex, newpositionshard;
				while (index) {
					oldindex = index;
					g = fringe[index - 1];
					index--; //For discovery, there's no need to keep the current position on the fringe, since we don't need to go back to it.
					h = getHash(g);
					if (solverread(localpositions, h&(SHARDOFFSETMASK)) == 0) { //If we don't find this position in our solver, expand it for discovery
						solverinsert(localpositions, h&(SHARDOFFSETMASK), 1); //Insert 1 here to that position to signify that it has been expanded. Do this now to minimize the time other threads can access this and try to duplicate work.
					
						movecount = generateMoves((char*) &moves, g);
						for (k = 0; k < movecount; k++) { //Iterate through all children of the current position
							newg = doMove(g, moves[k]);
							h = getHash(newg);
							newpositionshard = h >> fragmentsize;
							if(newpositionshard == currentshardid && !(solverread(localpositions, h&SHARDOFFSETMASK))) { //If we have a position in our current shard that hasn't been expanded, add it to the fringe
								if(isPrimitive(newg, moves[k]) == NOT_PRIMITIVE)
									fringe[index++] = newg;
								else
									solverinsert(localpositions, h&(SHARDOFFSETMASK), 2);
							}
							else if(newpositionshard != currentshardid) { // If the child is not in the current shard, insert it into the appropriate child shard
								for(int l = 0; l < targetshard->childrencount;l++) {
									if(newpositionshard == targetshard->childrenshards[l]->shardid) {
										if(!solverread(childrenshards[l], h&SHARDOFFSETMASK))
											solverinsert(childrenshards[l], h&SHARDOFFSETMASK, isPrimitive(newg, moves[k]));
										break;
									}
								}
							}
						}
					}
				}
			}

			fclose(parentfile); //Clean up
		}
	}
	//Save all children to appropriate files
	for(int i = 0; i < childrenshardcount; i++) {
		snprintf(filenamewriteaddr,filenamemaxlength, "/transfer-%llu-%llu", currentshardid, targetshard->childrenshards[i]->shardid);
		FILE* childnonprimitivefile = fopen(filename, "wb"); //Open non-primitive file
		snprintf(filenamewriteaddr,filenamemaxlength, "/transfer-%llu-%llu-l-primitive", currentshardid, targetshard->childrenshards[i]->shardid);
		FILE* childlossfile = fopen(filename, "wb"); //Open loss file
		snprintf(filenamewriteaddr,filenamemaxlength, "/transfer-%llu-%llu-t-primitive", currentshardid, targetshard->childrenshards[i]->shardid);
		FILE* childtiefile = fopen(filename, "wb"); //Open tie file
		int nppositionsfound = 0, losspositionsfound = 0, tiepositionsfound = 0;
		fwrite(&nppositionsfound, sizeof(int), 1, childnonprimitivefile); // Store a dummy space of 4 bytes to later save the number of positions found
		fwrite(&nppositionsfound, sizeof(int), 1, childlossfile); // Store a dummy space of 4 bytes to later save the number of positions found
		fwrite(&nppositionsfound, sizeof(int), 1, childtiefile); // Store a dummy space of 4 bytes to later save the number of positions found
		for(uint32_t j = 0; j < 1<<fragmentsize; j++) { //Could probably be made more efficient, but I think this setup is more easily parallelizable
			switch(solverread(childrenshards[i], j)) { //If the position was found in discovery, save it to the file and increment the number of positions found
				case NOT_PRIMITIVE: {
					fwrite(&j, sizeof(uint32_t), 1, childnonprimitivefile);
					nppositionsfound++;
					break;
				}
				case LOSS: {
					fwrite(&j, sizeof(uint32_t), 1, childlossfile);
					losspositionsfound++;
					break;
				}
				case TIE: {
					fwrite(&j, sizeof(uint32_t), 1, childtiefile);
					tiepositionsfound++;
					break;
				}
			}
		}
		//printf("%d positions found\n", positionsfound);
		fseek(childnonprimitivefile, 0, SEEK_SET);
		fwrite(&nppositionsfound, sizeof(int), 1, childnonprimitivefile);
		fseek(childlossfile, 0, SEEK_SET);
		fwrite(&losspositionsfound, sizeof(int), 1, childlossfile);
		fseek(childtiefile, 0, SEEK_SET);
		fwrite(&tiepositionsfound, sizeof(int), 1, childtiefile);
		fclose(childnonprimitivefile);
		fclose(childlossfile);
		fclose(childtiefile);
	}
	//Clean up
	free(filename);
	freesolver(localpositions);
	for(int i = 0; i < childrenshardcount; i++) {
		freesolver(childrenshards[i]);
	}
	free(childrenshards);
	free(fringe);
}

void solvefragment(char* workingfolder, shardgraph* targetshard, char fragmentsize)
{
	uint64_t currentshardid = targetshard->shardid;

	// Gives list of children shards that we send discovery children to that we send to
	int childrenshardcount = targetshard->childrencount;

	playerdata** childrenshards = malloc(sizeof(playerdata*) * childrenshardcount);

	game g;
	game newg;
	gamehash h;
	gamehash minindex = maxHash();
	gamehash maxindex = 0;
	char primitive;

	int index = 0;

	// Next Tier: Use CUDA Malloc when moving to GPU for "moves" and "fringe"
	solverdata* localpositions = initializesolverdata(fragmentsize);
	int solvedshardfilenamemaxlength 
	= strlen("/solved-100000000")+1;
	char* solvedshardfilename = malloc(sizeof(char)*(solvedshardfilenamemaxlength + strlen(workingfolder)));
	strncpy(solvedshardfilename, workingfolder, strlen(workingfolder));
	char* solvedshardfilenamewriteaddr = solvedshardfilename+strlen(workingfolder);
	for(int i = 0; i < childrenshardcount; i++) {
		snprintf(solvedshardfilenamewriteaddr,solvedshardfilenamemaxlength, "/solved-%d", targetshard->childrenshards[i]->shardid);
		childrenshards[i] = initializeplayerdata(fragmentsize, solvedshardfilename);
		printf("Shard %d player loaded\n", targetshard->childrenshards[i]->shardid);
		if(childrenshards[i] == NULL) {
			printf("Memory allocation error\n");
			return;
		}
	}
	char moves[getMaxMoves()];
	game* fringe = calloc(sizeof(game), getMaxMoves() * getMaxDepth());
	if (localpositions == NULL || fringe == NULL) {
		printf("Memory allocation error\n");
		return;
	}

	// Add incoming Discovery states from parent
	// Multiple top node in shard
	//Set up the file name
	int filenamemaxlength = strlen("/transfer-100000000-100000000-x-primitive")+1;
	char* filename = malloc(sizeof(char)*(filenamemaxlength + strlen(workingfolder)));
	strncpy(filename, workingfolder, strlen(workingfolder));
	char* filenamewriteaddr = filename+strlen(workingfolder);
	const int SHARDOFFSETMASK = (1ULL << fragmentsize) - 1;
	//Find all children in childrenshards
	for (int i = 0; i < targetshard->parentcount; i++) {
		//Open the file and read the first word to determine how many children are to be read
		int positioncountfromparent;

		//Insert the loss positions found from the parent shard
		snprintf(filenamewriteaddr,filenamemaxlength, "/transfer-%llu-%llu-l-primitive", targetshard->parentshards[i]->shardid, targetshard->shardid);
		fflush(stdout);
		FILE* childlossfile = fopen(filename, "rb"); //Open loss file
		fread(&positioncountfromparent, sizeof(int), 1, childlossfile);
		for(int j = 0; j < positioncountfromparent; j++) {
			uint32_t newpositionoffset;
			fread(&newpositionoffset, sizeof(uint32_t), 1, childlossfile);
			solverinsert(localpositions, newpositionoffset, LOSS); //Insert is going to be faster than a read and check, so might as well just insert directly.
		}
		fclose(childlossfile);

		//Insert the tie positions found from the parent shard
		snprintf(filenamewriteaddr,filenamemaxlength, "/transfer-%llu-%llu-t-primitive", targetshard->parentshards[i]->shardid, targetshard->shardid);
		FILE* childtiefile = fopen(filename, "rb"); //Open tie file
		fread(&positioncountfromparent, sizeof(int), 1, childtiefile);
		for(int j = 0; j < positioncountfromparent; j++) {
			uint32_t newpositionoffset;
			fread(&newpositionoffset, sizeof(uint32_t), 1, childtiefile);
			solverinsert(localpositions, newpositionoffset, TIE);
		}
		fclose(childtiefile);
		//Begin computing any nonprimitives found from the parent shard
		snprintf(filenamewriteaddr,filenamemaxlength, "/transfer-%llu-%llu", targetshard->parentshards[i]->shardid, targetshard->shardid);
		FILE* parentfile = fopen(filename, "rb");
		fread(&positioncountfromparent, sizeof(int), 1, parentfile);
		int itcount = 0, misscount = 0;
		for(int j = 0; j < positioncountfromparent; j++) {
			//For each position in the file, add it to the fringe and run graph traversal from there
			uint32_t newpositionoffset;
			fread(&newpositionoffset, sizeof(uint32_t), 1, parentfile);
			if(solverread(localpositions, newpositionoffset)) continue; //If the position already is in the solver, we've already traversed from there, so ignore the position.
			gamehash newpositionhash = (((uint64_t) targetshard->shardid) << fragmentsize) + newpositionoffset; //Otherwise, set up the game corresponding to the saved offset.
			fringe[0] = hashToPosition(newpositionhash);
			index = 1;


			int movecount, k, oldindex, newpositionshard;
			char minprimitive;

			while (index) {
				/*if(itcount <100) {
					printf("Done with iteration %d with j=%d and miss=%d on position %llx with index %d\n", itcount, j, misscount, fringe[index-1], index);
					fflush(stdout);
				}
				itcount++;*/
				minprimitive = 255;
				oldindex = index;
				g = fringe[index-1];
				h = getHash(g);
				if(solverread(localpositions, h&(SHARDOFFSETMASK)) == 0) //If we don't find this position in our solver, expand it for discovery
				{
					misscount++;
				movecount = generateMoves((char*)&moves, g);
				for(k = 0; k < movecount; k++) //Iterate through all children of the current position
				{
					newg = doMove(g, moves[k]);
					h = getHash(newg);
					newpositionshard = h >> fragmentsize;
					if(newpositionshard != currentshardid) { //If the position isn't in our current shard, we've already solved it. Read the corresponding data
						for(int l = 0; l < targetshard->childrencount;l++) {
							if(newpositionshard == targetshard->childrenshards[l]->shardid) {
								primitive = playerread(childrenshards[l], h&SHARDOFFSETMASK);
								if(primitive == 0)
									printf("Error in primitive value: position %llx\n", h);
								break;
							}
							if(l == targetshard->childrencount - 1) {
								printf("Shard not found in children: %d, %llx\n", newpositionshard, h);
								exit(1);
							}
						}
					}
					else primitive = solverread(localpositions, h&SHARDOFFSETMASK); //Otherwise, check its value in the current shard
					if(!primitive) //Since playerread guarantees nonzero values, only goes through here if it's a local position
					{
						primitive = isPrimitive(newg, moves[i]);
						if(primitive != (char) NOT_PRIMITIVE) {
							solverinsert(localpositions,h&SHARDOFFSETMASK,primitive);
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
					solverinsert(localpositions,h&SHARDOFFSETMASK,minprimitive);
					index--;
				}
				}
				else { index--;}
			}
		}

		fclose(parentfile); //Clean up
	}
	//Save shard
	snprintf(solvedshardfilenamewriteaddr,solvedshardfilenamemaxlength, "/solved-%llu", currentshardid);
	FILE* childfile = fopen(solvedshardfilename, "wb");
	solversave(localpositions, childfile);
	fclose(childfile);

	//Clean up
	free(filename);
	free(solvedshardfilename);
	freesolver(localpositions);
	for(int i = 0; i < childrenshardcount; i++) {
		freeplayer(childrenshards[i]);
	}
	free(childrenshards);
	free(fringe);
    return;
}



static int initializeshard(shardgraph* shardlist, char* shardinitialized, uint32_t shardsize, uint32_t startingshard) {
	if(!shardinitialized[startingshard]) {
		//printf("Initializing shard %d\n", startingshard);
		//fflush(stdout);
		shardinitialized[startingshard] = 1;
		uint64_t* childrenshards;
		int childrencount = getchildrenshards(&childrenshards, shardsize, startingshard);

		shardlist[startingshard].shardid = startingshard;
		shardlist[startingshard].childrencount = childrencount;
		//printf("Shard %d has %d children\n", startingshard, childrencount);
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
