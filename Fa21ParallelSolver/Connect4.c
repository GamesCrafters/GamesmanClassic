#include "Game.h"

#define COLUMNCOUNT 5
#define ROWCOUNT 5
#define CONNECT 4

static uint64_t DOWNDIAGWIN;
static uint64_t HORIZONTALWIN;
static uint64_t VERTICALWIN;
static uint64_t UPDIAGWIN;
static uint64_t INITIALPOSITION;

static bool isawin(game position, game pieces);

void initialize_constants() {
	uint64_t l = 0;
	for(int i = 0; i < COLUMNCOUNT; i++) l |= 1L<<((ROWCOUNT+1)*i);
	uint64_t down=0, left=0, updiag=0, downdiag = 0;
	for(int i = 0; i < CONNECT;i++)
	{
		down = (down << 1)|1;
		left = (left << (ROWCOUNT+1))|1;
		downdiag = (downdiag << ROWCOUNT)|1;
		updiag = (updiag << (ROWCOUNT+2))|1;
	}
	DOWNDIAGWIN = downdiag;
	 HORIZONTALWIN = left;
	VERTICALWIN = down;
	UPDIAGWIN = updiag;
	INITIALPOSITION = l;
}

game getStartingPositions() {
    return INITIALPOSITION;
}

/** Returns the result of moving at the given position
    * Bit 63 is flipped to change current player
    * Move is assumed to be the bit position of the empty cell.
    * As such, we add 1<<move for a yellow, and 1<<(move+1) for red
    * 0b0001011 + 1<<move = 0b0010011 = ---YYRR
    * 0b0001011 + 1<<(move+1) = 0b0011011 = ---RYRR*/
game doMove(game position, char move) {

    return (position ^ 0x8000000000000000L)+(1L<<(move+(position >> 63)));

}

/** Returns the list of viable moves.
Assumes retval has length at least COLUMNCOUNT+1*/
int generateMoves(char* retval, game position) {
	int k = 0;
    for(int i = 0; i < COLUMNCOUNT; i++)
    {
        char start = (char)((ROWCOUNT+1)*(i+1) - 1);
        while((position & (1L<<start))==0) start--;
        if(start != (char)(ROWCOUNT+1)*(i+1)-1)
        {
            *retval = start;
            retval++;
			k++;
        }
    }
    *retval = -1;
	return k;
}

/** Returns if the given position is primitive, assuming that the most recent move is as stated*/
char isPrimitive(game position, char mostrecentmove) {
    game origpos = position;
    if((position & 0x8000000000000000L) == 0)
    { // Check wins of 1s
        for(int i = 0; i < COLUMNCOUNT; i++)
        {
            char start = (char)((ROWCOUNT+1)*(i+1) - 1);
            while((position & (1L<<start))==0) start--;
            position ^= (1L<<start);
        }
    }
    else
    { //Check wins of 0s
        for(int i = 0; i < COLUMNCOUNT; i++)
        {
            char start = (char)((ROWCOUNT+1)*(i+1) - 1);
            char start2 = (char)(start+1);
            while((position & (1L<<start))==0) start--;
            position |= (1L<<(start2))-(1L << start);
        }
        position = ~position;
    }
    position &= (1L<<(COLUMNCOUNT*(ROWCOUNT+1)))-1;
    //System.out.printf("%016X %n", position);
    //At this point, the position should contain 1s only on the places that match the most recent move.
    int x = mostrecentmove/(ROWCOUNT+1), y=mostrecentmove%(ROWCOUNT+1);
    //Vertical check
    if((y >= CONNECT-1)&& isawin(position, VERTICALWIN<<(mostrecentmove-(CONNECT-1)))) return LOSS;
    //Horizontal and diagonal checks
    for(int i = 0; i < CONNECT; i++) {
        if (x >= i && (x + ((CONNECT - 1) - i)) < COLUMNCOUNT) {
            //Horizontal check
            if (isawin(position, HORIZONTALWIN << (mostrecentmove - i * (ROWCOUNT + 1)))) return LOSS;
            if (y >= i && (y + ((CONNECT - 1) - i)) < ROWCOUNT) {
                //Up Diagonal check
                if (isawin(position, UPDIAGWIN << (mostrecentmove - i * (ROWCOUNT + 2)))) return LOSS;
            }
            if (y + i < ROWCOUNT && (y - ((CONNECT - 1) - i)) >= 0) {
                //Down Diagonal check
                if (isawin(position, DOWNDIAGWIN << (mostrecentmove - i * (ROWCOUNT)))) return LOSS;
            }
        }
    }
    if((origpos & (INITIALPOSITION << ROWCOUNT)) == INITIALPOSITION << ROWCOUNT) {return TIE;}
    return NOT_PRIMITIVE;
}

static bool isawin(game position, game pieces)
{
    //System.out.printf("%016X %b %n", pieces, (position&pieces) == pieces);
    return (position&pieces) == pieces;
}

int getSize() {
    return COLUMNCOUNT*ROWCOUNT;
}

int getMaxDepth() {
	return getSize();
}

int getMaxMoves() {
	return COLUMNCOUNT+1;
}

gamehash getHash(game position) {
    game newpos= position & 0x7FFFFFFFFFFFFFFFL;
    game oppositepos = 0;
    for(int i = 0; i < COLUMNCOUNT; i++)
    {
        uint64_t val = (newpos>>(i*(ROWCOUNT+1)))&((1L<<(ROWCOUNT+1))-1);
        oppositepos |= val<<((ROWCOUNT+1)*(COLUMNCOUNT-i-1));
    }
    return oppositepos < newpos ? oppositepos : newpos;
}
int maxHash()
{
	return 1<<((ROWCOUNT+1)*COLUMNCOUNT);
}
game hashToPosition(gamehash hash) {
    char emptyspots = 0;
    for(int j = 0; j < COLUMNCOUNT; j++) {
        for (int i = ROWCOUNT; i >= 0; i--) {
            if((hash&(1L<<(j*(ROWCOUNT+1)+i))) == 0) emptyspots++;
            else break;
        }
    }
    return hash | ((gamehash) ((getSize()-emptyspots)%2)) << 63;
}

static void recurse(char* primitives, int* hashsetsize, game position, int layer, char* movestring) {
    if(layer < 5) {printf("%s\n", movestring); fflush(stdout);}
	char moves[COLUMNCOUNT+1];
    generateMoves(moves, position);
    for(int i = 0; i < COLUMNCOUNT;i++)
    {
        if(moves[i] == -1) break;
        game newpos = doMove(position, moves[i]);
        if(isPrimitive(newpos, moves[i]) != (char)NOT_PRIMITIVE)
        {
            gamehash poshash = getHash(newpos);
            if(!((primitives[poshash>>3])&(1<<(poshash&7))))
			{
				primitives[poshash>>3]|=1<<(poshash&7);
				(*hashsetsize)++;
			}
        }
        else
        {
			movestring[layer] = 48+i;
            recurse(primitives,hashsetsize, newpos, layer+1, movestring);
        }
    }
	movestring[layer] = 0;
}
static int testC4() {
	initialize_constants();
    game pos = getStartingPositions();
    char* primitives = calloc(sizeof(char),(1<<((COLUMNCOUNT*(ROWCOUNT+1))-3)));
	int hashsetsize = 0;
	char* movestring = calloc(sizeof(char), getSize()+1);
    recurse(primitives, &hashsetsize, pos, 0, movestring);
    printf("Number of primitives: %d", hashsetsize);
	free(primitives);
	free(movestring);

}
