#include "Game.h"


int main()
{
  	initialize_constants();
    game pos = getStartingPositions();
    char* primitives = calloc(sizeof(char),maxHash());
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
	int* positionsfound = calloc(sizeof(int), 256);
	printf("Beginning main loop\n");
	fflush(stdout);
	while(index)
	{
		minprimitive = 255;
		oldindex = index;
		g = fringe[index-1];
		movecount = generateMoves((char*)&moves, g);
		for(i=0;i<movecount;i++)
		{
			newg = doMove(g, moves[i]);
			h = getHash(newg);
			primitive = primitives[h];
			if(!primitive)
			{
				primitive = isPrimitive(newg, moves[i]);
				if(primitive != (char) NOT_PRIMITIVE) {
					primitives[h] = primitive;
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
			primitives[getHash(g)] = minprimitive;
			//printf("Position 0x%08x determined fully\n", g);
			positionsfound[minprimitive]++;
			index--;
		}
	}
	printf("Done computing, listing statistics\n");
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
	printf("Starting position has value %d\n", primitives[getStartingPositions()]);

	free(primitives);
	free(fringe);
	free(positionsfound);
	return 0;
}
