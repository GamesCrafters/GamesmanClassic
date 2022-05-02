#include "Game.h"

#define COLUMNCOUNT 6
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
	for(int i = 0; i < COLUMNCOUNT; i++) l |= 1ULL<<((ROWCOUNT+1)*i);
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
    return (position ^ 0x8000000000000000L)+(1ULL<<(move+(position >> 63)));
}

/** Returns the list of viable moves.
Assumes retval has length at least COLUMNCOUNT+1*/
int generateMoves(char* retval, game position) {
	int k = 0;
    for(int i = 0; i < COLUMNCOUNT; i++)
    {
        char start = (char)((ROWCOUNT+1)*(i+1) - 1);
        while((position & (1ULL<<start))==0) start--; // this is what sigbit does
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
            while((position & (1ULL<<start))==0) start--;
            position ^= (1ULL<<start);
        }
    }
    else
    { //Check wins of 0s
        for(int i = 0; i < COLUMNCOUNT; i++)
        {
            char start = (char)((ROWCOUNT+1)*(i+1) - 1);
            char start2 = (char)(start+1);
            while((position & (1ULL<<start))==0) start--;
            position |= (1ULL<<(start2))-(1L << start);
        }
        position = ~position;
    }
    position &= (1ULL<<(COLUMNCOUNT*(ROWCOUNT+1)))-1;
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
        uint64_t val = (newpos>>(i*(ROWCOUNT+1)))&((1ULL<<(ROWCOUNT+1))-1);
        oppositepos |= val<<((ROWCOUNT+1)*(COLUMNCOUNT-i-1));
    }
    return oppositepos < newpos ? oppositepos : newpos;
}
uint64_t maxHash()
{
	return 1ULL<<((ROWCOUNT+1)*COLUMNCOUNT);
}
int hashLength()
{
	return ((ROWCOUNT+1)*COLUMNCOUNT);
}
game hashToPosition(gamehash hash) {
    char emptyspots = 0;
    for(int j = 0; j < COLUMNCOUNT; j++) {
        for (int i = ROWCOUNT; i >= 0; i--) {
            if((hash&(1ULL<<(j*(ROWCOUNT+1)+i))) == 0) emptyspots++;
            else break;
        }
    }
    return hash | ((gamehash) ((getSize()-emptyspots)%2)) << 63;
}

/*static void recurse(char* primitives, int* hashsetsize, game position, int layer, char* movestring) {
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
    char* primitives = calloc(sizeof(char),(1ULL<<((COLUMNCOUNT*(ROWCOUNT+1))-3)));
	int hashsetsize = 0;
	char* movestring = calloc(sizeof(char), getSize()+1);
    recurse(primitives, &hashsetsize, pos, 0, movestring);
    printf("Number of primitives: %d", hashsetsize);
	free(primitives);
	free(movestring);
}*/


static inline uint64_t makeColumnMove(uint64_t shard, char move, char mode) {
    return shard + (1<<(move + mode));
}

int getchildrenshards(uint64_t** childrenshards, char shardsize, uint64_t parentshard) {
	// Column size is not 7
	int id_size = hashLength() - shardsize; // size in bits
	int full_cols = (id_size / (ROWCOUNT+1));
	int remainders = id_size % (ROWCOUNT+1);
	int length = 0;

	// Count the number of non-full 7-sized columns
	for (int i = 0; i < full_cols; i++) {
		if (!(parentshard&(1<<(id_size-1-((ROWCOUNT+1)*i))))) {
			length+=2;
		}
	}
    if(remainders) {
	    // Count the number of non-full <7-sized columns
	    int filter = (1<<remainders) - 1;
	    if (!(parentshard&(1<<(remainders-1)))) {
	    	length += (parentshard & filter) ? 2 : 1;
	    } 
    }
	uint64_t* children = malloc(length * sizeof(uint64_t));
    int allocatedchildren = 0;
    int columnremainingbits = ROWCOUNT+1;
    for(int i = id_size - 1; i >= 0;) {
        if(parentshard&(1<<i)) {
            //printf("%d %d\n", i, columnremainingbits);
            if(columnremainingbits != ROWCOUNT+1) {
                children[allocatedchildren] = parentshard+(1<<i);
                children[allocatedchildren+1] = parentshard+(1<<(i+1));
                //printf("%d %d\n", parentshard+(1<<i), parentshard+(1<<(i+1)));
                allocatedchildren+=2;
            }
            i-=columnremainingbits;
            columnremainingbits = ROWCOUNT+1;
        }
        else{
            columnremainingbits--;
            i--;
        }
    }
    if(allocatedchildren < length) {
        children[allocatedchildren] = parentshard+1;
        allocatedchildren++;
    }
    /*printf("Children found for shard %d: ", parentshard);
    for(int i = 0; i < allocatedchildren; i++) printf("%d ", children[i]);
    printf("\n");
    if(allocatedchildren != length) {
        printf("Error: incorrect children found: %d instead of %d\n", allocatedchildren, length);
    }*/
	*childrenshards = children; // no free
	return length;
}

/* KEENE'S PYTHON CODE CAUSE I WAS LAZY TO DEBUG IN C
ROWCOUNT = 6
COLUMNCOUNT = 7
def makeColumnMove(shard, move, mode):
return shard + (1<<(move + mode));
def hashLength():
	return ((ROWCOUNT+1)*COLUMNCOUNT);
def sigbit(shard, col, shardsize):
id_size = hashLength() - shardsize
remainders = id_size % (ROWCOUNT+1)
if (col == 0 and remainders == 0):
	return shard > 0
elif (col == 0):
	return shard & (1 << remainders - 1) > 0
return shard & (1 << (col * (ROWCOUNT+1) + remainders - 1)) > 0

def getchildrenshards(childrenshards, shardsize, parentshard):
id_size = hashLength() - shardsize
full_cols = (int)(id_size / (ROWCOUNT+1))
remainders = id_size % (ROWCOUNT+1)
length = 0
for i in range(1, full_cols+1):
	pp = parentshard
	if not sigbit(pp, i, shardsize):
		length+=2
filter = ((1<<remainders)-1)
if (not sigbit(parentshard, 0, shardsize) and parentshard & filter > 0):
	length+=2
elif (parentshard & filter <= 0 and not sigbit(parentshard, 0, shardsize)):
	length+=1
children = [0 for _ in range(length)]
i = id_size - 1;
added = 0
full_c_passed = 0
while (i >= 0 and added < length):
	if (full_c_passed >= full_cols and parentshard & filter <= 0):
			children[added] = parentshard+1
			added += 1
			break
	if ((parentshard>>i) & 1 > 0):
		if (((i+1)-remainders) % (ROWCOUNT + 1) == 0):
			i = ((i-2)//(ROWCOUNT+1)) * (ROWCOUNT+1) - 1 + remainders

			full_c_passed += 1
		else:
			children[added] = makeColumnMove(parentshard, i, 0)
			added += 1
			children[added] = makeColumnMove(parentshard, i, 1)
			added += 1
			full_c_passed += 1
			i = ((i-1)//(ROWCOUNT+1)) * (ROWCOUNT+1) - 1 + remainders
	else:
		i-=1
return children, length
def print_b(num):
print("{0:b}".format(num))
# print_b(1)
# print_b(makeColumnMove(1, 0, 1))
# print_b(makeColumnMove(1, 0, 0))
c, l = getchildrenshards([], 33, 0b1100011110001101)
print([bin(k) for k in c])
# print(c)
# print(l)
# 4 --> 2
print("YO",((4-1)//(6+1)) * (6+1) + 2)

*/
