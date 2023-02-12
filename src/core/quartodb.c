/************************************************************************
**
** NAME:	quartodb.c
**
** DESCRIPTION:	Accessor functions for Quarto
**
** AUTHOR:	Cameron Cheung | GamesCrafters Research Group, UC Berkeley
**		Supervised by Dan Garcia <ddgarcia@cs.berkeley.edu>
**
** DATE:	2023-01-20
**
**************************************************************************/

#include "gamesman.h"
#include "interact.h"
#include "quartodb.h"

#define MAX16BIT 65536
#define MAX20BIT 1048576
#define FULL_OCCUPIED_SLOTS_MASK 0xFFFFFFFFFFFFFFFF
#define PRIMITIVE_FULL 255

static const int8_t QUARTO_UNDECIDED = -2, QUARTO_LOSE0 = 0, QUARTO_TIE0_16 = 1;

/*internal declarations and definitions*/

void            quartodb_free                     ();

/* Value */
VALUE           quartodb_get_value                (POSITION pos);
VALUE           quartodb_set_value                (POSITION pos, VALUE val);

/* Remoteness */
REMOTENESS      quartodb_get_remoteness           (POSITION pos);
void            quartodb_set_remoteness           (POSITION pos, REMOTENESS val);

/* Visited */
BOOLEAN         quartodb_check_visited            (POSITION pos);
void            quartodb_mark_visited             (POSITION pos);
void            quartodb_unmark_visited           (POSITION pos);

/* Mex */
MEX             quartodb_get_mex                  (POSITION pos);
void            quartodb_set_mex                  (POSITION pos, MEX mex);

/* saving to/reading from a file */
BOOLEAN         quartodb_save_database            ();
BOOLEAN         quartodb_load_database            ();

/*
** Code
*/

char values[17][18] = {
//   00  01  02  03  04  05  06  07  08  09  10  11  12  13  14  15  16  17
	{'L','L','L','L','L','L','L','L','L','T','W','W','W','W','W','W','W','W'},
    {'L','L','L','L','L','L','L','L','T','W','W','W','W','W','W','W','W'},
	{'L','L','L','L','L','L','L','L','T','W','W','W','W','W','W','W'},
	{'L','L','L','L','L','L','L','T','W','W','W','W','W','W','W'},
	{'L','L','L','L','L','L','L','T','W','W','W','W','W','W'},
	{'L','L','L','L','L','L','T','W','W','W','W','W','W'},
	{'L','L','L','L','L','L','T','W','W','W','W','W'},
	{'L','L','L','L','L','T','W','W','W','W','W'},
	{'L','L','L','L','L','T','W','W','W','W'},
	{'L','L','L','L','T','W','W','W','W'},
	{'L','L','L','L','T','W','W','W'},
	{'L','L','L','T','W','W','W'},
	{'L','L','L','T','W','W'},
	{'L','L','T','W','W'},
	{'L','L','T','W'},
	{'L','T','W'},
	{'L','T'}
};

REMOTENESS remotenesses[17][18] = {
//   00  01  02  03  04  05  06  07  08  09  10  11  12  13  14  15  16  17
	{ 0,  2,  4,  6,  8, 10, 12, 14, 16, 16, 15, 13, 11,  9,  7,  5,  3,  1},
    { 0,  2,  4,  6,  8, 10, 12, 14, 15, 15, 13, 11,  9,  7,  5,  3,  1},
	{ 0,  2,  4,  6,  8, 10, 12, 14, 14, 13, 11,  9,  7,  5,  3,  1},
	{ 0,  2,  4,  6,  8, 10, 12, 13, 13, 11,  9,  7,  5,  3,  1},
	{ 0,  2,  4,  6,  8, 10, 12, 12, 11,  9,  7,  5,  3,  1},
	{ 0,  2,  4,  6,  8, 10, 11, 11,  9,  7,  5,  3,  1},
	{ 0,  2,  4,  6,  8, 10, 10,  9,  7,  5,  3,  1},
	{ 0,  2,  4,  6,  8,  9,  9,  7,  5,  3,  1},
	{ 0,  2,  4,  6,  8,  8,  7,  5,  3,  1},
	{ 0,  2,  4,  6,  7,  7,  5,  3,  1},
	{ 0,  2,  4,  6,  6,  5,  3,  1},
	{ 0,  2,  4,  5,  5,  3,  1},
	{ 0,  2,  4,  4,  3,  1},
	{ 0,  2,  3,  3,  1},
	{ 0,  2,  2,  1},
	{ 0,  1,  1},
	{ 0,  0}
};

typedef struct quartotier {
    uint8_t level; // Number of pieces on board
    uint8_t pieceToPlace; // 0-15; undefined if level=16 
    uint16_t piecesPlaced; // 16-bit map; nth LSB is 1 if piece n is on board
    uint16_t occupiedSlots; // 16-bit map; nth LSB is 1 if nth slot is nonempty (0 = top left; row-major order)
    uint8_t occupiedSlotsListX4[16]; // list of occupied slots, multiplied by 4
    uint64_t occupiedSlotsMask;
} QUARTOTIER;

void buildTier(QUARTOTIER *tier, uint8_t pieceToPlace, uint16_t piecesPlaced, uint16_t occupiedSlots) {
    tier->pieceToPlace = pieceToPlace;
    tier->piecesPlaced = piecesPlaced;
    tier->occupiedSlots = occupiedSlots;
    tier->occupiedSlotsMask = 0;

    uint8_t counter = 0;
    for (int i = 0; i < 16; i++) {
        if (occupiedSlots & (1 << i)) {
            tier->occupiedSlotsListX4[counter++] = i << 2;
            tier->occupiedSlotsMask |= UINT64_C(0xF) << (i << 2);
        }
    }
    tier->level = counter;
}

uint8_t *unsetBitLists = NULL;
void initializeSolving() {
    // Initialize unsetBitLists Table
    unsetBitLists = (uint8_t *) malloc(sizeof(uint8_t) * MAX20BIT);
    if (unsetBitLists == NULL) {
        printf("malloc failed for unsetBitLists Table\n");
        exit(0);
    }
    uint32_t i, j, start, curr;
    for (i = 0; i < 65536; i++) {
        start = i << 4;
        curr = 0;
        for (j = 0; j < 16; j++) {
            if (!((i >> j) & 1)) {
                unsetBitLists[start | curr] = j;
                curr++;
            }
        }
    }
}

typedef struct {
    uint16_t canon;
    uint8_t transform;  // 0-31 for os, 0-23 for pp
} CANON_STRUCT;


CANON_STRUCT osCanonForward[MAX16BIT];
CANON_STRUCT ppCanonForward[MAX20BIT];
uint8_t osTransforms[32][16];
uint8_t reorderMatrix[24][16];

// generates the transform map of the 32 OS symmetries
void generateOSTransforms() {
    uint8_t symMatrix[5][16] = {
        {0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15},  // transpose
        {12, 8, 4, 0, 13, 9, 5, 1, 14, 10, 6, 2, 15, 11, 7, 3},  // rotation clockwise 90 degrees
        {15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},  // rotation clockwise 180 degrees
        {0, 2, 1, 3, 8, 10, 9, 11, 4, 6, 5, 7, 12, 14, 13, 15},  // swap inner
        {5, 4, 7, 6, 1, 0, 3, 2, 13, 12, 15, 14, 9, 8, 11, 10}   // swap outer
    };

    for (uint8_t sym = 0; sym < 32; sym++) { // 32 symmetries
        uint8_t map[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};   // start with original
        for (uint8_t transform = 0; transform < 5; transform++) {
            if (sym & 1 << transform) { // applies one transform at a time
                uint8_t map_temp[16];
                for (uint8_t i = 0; i < 16; i++) { // for each bit, rearrange
                    map_temp[i] = map[symMatrix[transform][i]];
                }
                for (uint8_t i = 0; i < 16; i++) {
                    map[i] = map_temp[i];
                }
            }
        }
        for (uint8_t i = 0; i < 16; i++) {
            osTransforms[sym][i] = map[i];
        }
    }
}

void initializeCanonOS() {
    generateOSTransforms();

    for (uint32_t v = 0; v < MAX16BIT; v++) {
        osCanonForward[v].canon = v;
        osCanonForward[v].transform = 0;
        for (uint8_t sym = 1; sym < 32; sym++) { // 32 symmetries
            uint16_t vSym = 0;

            for (uint8_t i = 0; i < 16; i++) {          // for each new position
                if (v & 1 << osTransforms[sym][i]) {  // if old position is occupied
                    vSym |= 1 << i;
                }
            }

            if (vSym < osCanonForward[v].canon) { // find the minimum
                osCanonForward[v].canon = vSym;
                osCanonForward[v].transform = sym;
            }
        }
    }
}

void calculateReorderMatrix() {
    uint8_t symReorderMatrix[24][4] = {
        {0, 1, 2, 3}, {0, 1, 3, 2}, {0, 2, 1, 3}, {0, 2, 3, 1}, {0, 3, 1, 2}, {0, 3, 2, 1},
        {1, 0, 2, 3}, {1, 0, 3, 2}, {1, 2, 0, 3}, {1, 2, 3, 0}, {1, 3, 0, 2}, {1, 3, 2, 0},
        {2, 0, 1, 3}, {2, 0, 3, 1}, {2, 1, 0, 3}, {2, 1, 3, 0}, {2, 3, 0, 1}, {2, 3, 1, 0},
        {3, 0, 1, 2}, {3, 0, 2, 1}, {3, 1, 0, 2}, {3, 1, 2, 0}, {3, 2, 0, 1}, {3, 2, 1, 0}
    }; // e.g. {3, 0, 2, 1} means 0th bit is orginal 3rd bit

    for (uint8_t v = 0; v < 16; v++) { // for each piece
        for (uint8_t reorder = 0; reorder < 24; reorder++) { 
            reorderMatrix[reorder][v] =
                (v >> symReorderMatrix[reorder][0] & 1) |
                (v >> symReorderMatrix[reorder][1] & 1) << 1 |
                (v >> symReorderMatrix[reorder][2] & 1) << 2 |
                (v >> symReorderMatrix[reorder][3] & 1) << 3;
        }
    }
}

void initializeCanonPP() {
    calculateReorderMatrix();

    for (uint8_t nextPiece = 0; nextPiece < 16; nextPiece++) {
        for (uint32_t v = 0; v < MAX16BIT; v++) {
            if ((v & 1 << nextPiece) && v != MAX16BIT - 1) continue;   // next piece can't be on the board

            uint32_t index = nextPiece << 16 | v;
            ppCanonForward[index].canon = MAX16BIT - 1;

            for (uint8_t reorder = 0; reorder < 24; reorder++) {
                uint16_t vSym = 0;
                for (uint8_t i = 0; i < 16; i++) {                          // for each piece
                    if (v & 1 << i) {                                     // if currently placed
                        vSym |= 1 << reorderMatrix[reorder][i^nextPiece]; // transform to another piece
                    }
                }
                if (vSym < ppCanonForward[index].canon) {
                    ppCanonForward[index].canon = vSym;
                    ppCanonForward[index].transform = reorder;
                }
            }
        }
    }
}

// Lookup table [bitstring (16b), index of bit (4b)] -> n such that index is nth set bit
// e.g. whichSetBit[0b0101011101101000 0110] = 3 (6th LSB is 3rd set bit)
uint8_t* whichSetBit = NULL;

void initializeHashing() {
    // Initialize whichSetBit Table
    whichSetBit = (uint8_t *) malloc(sizeof(uint8_t) * MAX20BIT);
    uint8_t which;
    uint32_t bitString;
    uint8_t n;
    if (whichSetBit == NULL) {
        printf("malloc failed for whichSetBit Table\n");
        exit(0);
    }
    for (uint32_t i = 0; i < MAX20BIT; i++) {
        n = i & 0xF;
        bitString = i >> 4;
        which = 0;
        for (uint8_t j = 0; j < n; j++) {
            if ((bitString >> j) & 1) which++;
        }
        whichSetBit[i] = which;
    }
}

void quartodb_init(DB_Table *new_db) {
	new_db->put_value = quartodb_set_value;
	new_db->put_remoteness = quartodb_set_remoteness;
	new_db->mark_visited = quartodb_mark_visited;
	new_db->unmark_visited = quartodb_unmark_visited;
	new_db->put_mex = quartodb_set_mex;
	new_db->free_db = quartodb_free;

	new_db->get_value = quartodb_get_value;
	new_db->get_remoteness = quartodb_get_remoteness;
	new_db->check_visited = quartodb_check_visited;
	new_db->get_mex = quartodb_get_mex;
	new_db->save_database = quartodb_save_database;
	new_db->load_database = quartodb_load_database;

    initializeHashing();
    initializeSolving();
    initializeCanonOS();
    initializeCanonPP();
}

void quartodb_free() {
    SafeFree(whichSetBit);
    SafeFree(unsetBitLists);
}

int numBitsPerValue[17] = {5, 5, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 2, 2, 1};
int8_t quartodb_get_valueremoteness_from_file(QUARTOTIER *tier, TIERPOSITION tierPosition) {
	int bitsPerValue = numBitsPerValue[tier->level];
    char filename[100];
    snprintf(filename, 100, "./data/quarto/database/%02d/%04X%04X", tier->level, tier->piecesPlaced, tier->occupiedSlots);
    FILE *fptr = fopen(filename, "r");
    if (fptr == NULL) {
        return 0;
    }
    uint16_t twoValues;
    uint64_t m = tierPosition * bitsPerValue;
    fseek(fptr, m >> 3, SEEK_SET);
    fread(&twoValues, sizeof(int16_t), 1, fptr);
    fclose(fptr);
    return (twoValues >> (m & 0b111)) & ((1 << bitsPerValue) - 1);
}

/*
    Return LOSE IN 0 if there exists a win quartet, regardless of whether the position is reachable. 
    Otherwise, return TIE IN 0 if the board is full, else QUARTO_UNDECIDED.
*/
int8_t primitiveFull(uint64_t bitBoard, uint64_t occupiedSlotsMask) {
    // Flip all bits except at where there are blanks.
    // Row and Column are zero-indexed
    uint64_t invBitBoard = bitBoard ^ occupiedSlotsMask;

    if (
        ( // Check row
            (
                (
                    bitBoard & 
                    (bitBoard >> 4) & 
                    (bitBoard >> 8) & 
                    (bitBoard >> 12)
                ) | (
                    invBitBoard & 
                    (invBitBoard >> 4) & 
                    (invBitBoard >> 8) & 
                    (invBitBoard >> 12)
                )
            ) & 0x000F000F000F000F
        ) || ( // Check column
            (
                (
                    bitBoard & 
                    (bitBoard >> 16) & 
                    (bitBoard >> 32) & 
                    (bitBoard >> 48)
                ) | (
                    invBitBoard & 
                    (invBitBoard >> 16) & 
                    (invBitBoard >> 32) & 
                    (invBitBoard >> 48)
                )
            ) & 0xFFFF
        ) || ( // Check main diagonal if slot is on main diagonal
            (
                (
                    bitBoard & 
                    (bitBoard >> 20) & 
                    (bitBoard >> 40) & 
                    (bitBoard >> 60)
                ) | (
                    invBitBoard & 
                    (invBitBoard >> 20) & 
                    (invBitBoard >> 40) & 
                    (invBitBoard >> 60)
                )
            ) & 0xF
        ) || ( // Check antidiagonal if slot is on antidiagonal
            (
                (
                    bitBoard & 
                    (bitBoard >> 12) & 
                    (bitBoard >> 24) & 
                    (bitBoard >> 36)
                ) | (
                    invBitBoard & 
                    (invBitBoard >> 12) & 
                    (invBitBoard >> 24) & 
                    (invBitBoard >> 36)
                )
            ) & 0xF000
        )
    ) return QUARTO_LOSE0;
    return (occupiedSlotsMask == FULL_OCCUPIED_SLOTS_MASK) ? QUARTO_TIE0_16 : QUARTO_UNDECIDED;
}

int8_t primitivePartial(uint64_t bitBoard, uint64_t occupiedSlotsMask, uint8_t slot) {
    // Row and Column are zero-indexed
    uint8_t row = slot >> 2;
    uint8_t col = slot & 0b11;
    uint64_t invBitBoard = bitBoard ^ occupiedSlotsMask;

    if (
        ( // Check row
            (
                (
                    bitBoard & 
                    (bitBoard >> 4) & 
                    (bitBoard >> 8) & 
                    (bitBoard >> 12)
                ) | (
                    invBitBoard & 
                    (invBitBoard >> 4) & 
                    (invBitBoard >> 8) & 
                    (invBitBoard >> 12)
                )
            ) & 0x000F000F000F000F
        ) || ( // Check column
            (
                (
                    bitBoard & 
                    (bitBoard >> 16) & 
                    (bitBoard >> 32) & 
                    (bitBoard >> 48)
                ) | (
                    invBitBoard & 
                    (invBitBoard >> 16) & 
                    (invBitBoard >> 32) & 
                    (invBitBoard >> 48)
                )
            ) & 0xFFFF
        ) || ( // Check main diagonal if slot is on main diagonal
            (row == col) &&
            (
                (
                    bitBoard & 
                    (bitBoard >> 20) & 
                    (bitBoard >> 40) & 
                    (bitBoard >> 60)
                ) | (
                    invBitBoard & 
                    (invBitBoard >> 20) & 
                    (invBitBoard >> 40) & 
                    (invBitBoard >> 60)
                )
            ) & 0xF
        ) || ( // Check antidiagonal if slot is on antidiagonal
            (row + col == 3) &&
            (
                (
                    bitBoard & 
                    (bitBoard >> 12) & 
                    (bitBoard >> 24) & 
                    (bitBoard >> 36)
                ) | (
                    invBitBoard & 
                    (invBitBoard >> 12) & 
                    (invBitBoard >> 24) & 
                    (invBitBoard >> 36)
                )
            ) & 0xF000
        )
    ) return QUARTO_LOSE0;
    return (occupiedSlotsMask == FULL_OCCUPIED_SLOTS_MASK) ? QUARTO_TIE0_16 : QUARTO_UNDECIDED;
}

uint64_t factmul[256] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 
    0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 
    0, 6, 12, 18, 24, 30, 36, 42, 48, 54, 60, 66, 72, 78, 84, 90, 
    0, 24, 48, 72, 96, 120, 144, 168, 192, 216, 240, 264, 288, 312, 336, 360, 
    0, 120, 240, 360, 480, 600, 720, 840, 960, 1080, 1200, 1320, 1440, 1560, 1680, 1800, 
    0, 720, 1440, 2160, 2880, 3600, 4320, 5040, 5760, 6480, 7200, 7920, 8640, 9360, 10080, 10800, 
    0, 5040, 10080, 15120, 20160, 25200, 30240, 35280, 40320, 45360, 50400, 55440, 60480, 65520, 70560, 75600, 
    0, 40320, 80640, 120960, 161280, 201600, 241920, 282240, 322560, 362880, 403200, 443520, 483840, 524160, 564480, 604800, 
    0, 362880, 725760, 1088640, 1451520, 1814400, 2177280, 2540160, 2903040, 3265920, 3628800, 3991680, 4354560, 4717440, 5080320, 5443200, 
    0, 3628800, 7257600, 10886400, 14515200, 18144000, 21772800, 25401600, 29030400, 32659200, 36288000, 39916800, 43545600, 47174400, 50803200, 54432000, 
    0, 39916800, 79833600, 119750400, 159667200, 199584000, 239500800, 279417600, 319334400, 359251200, 399168000, 439084800, 479001600, 518918400, 558835200, 598752000, 
    0, 479001600, 958003200, 1437004800, 1916006400, 2395008000, 2874009600, 3353011200, 3832012800, 4311014400, 4790016000, 5269017600, 5748019200, 6227020800, 6706022400, 7185024000, 
    0, 6227020800, 12454041600, 18681062400, 24908083200, 31135104000, 37362124800, 43589145600, 49816166400, 56043187200, 62270208000, 68497228800, 74724249600, 80951270400, 87178291200, 93405312000, 
    0, 87178291200, 174356582400, 261534873600, 348713164800, 435891456000, 523069747200, 610248038400, 697426329600, 784604620800, 871782912000, 958961203200, 1046139494400, 1133317785600, 1220496076800, 1307674368000, 
    0, 1307674368000, 2615348736000, 3923023104000, 5230697472000, 6538371840000, 7846046208000, 9153720576000, 10461394944000, 11769069312000, 13076743680000, 14384418048000, 15692092416000, 16999766784000, 18307441152000, 19615115520000
};

// BitBoard to TierPosition | Unhash Function Not Needed Here
uint64_t quartoHash(QUARTOTIER *tier, uint64_t bitBoard) {
    uint8_t i, idx = 0;
    uint8_t pieces[16];
    uint64_t tierPosition = 0;
    uint32_t remaining = 0;

    for (i = 0; i < tier->level; i++) {
        idx = (bitBoard >> tier->occupiedSlotsListX4[i]) & 0xF;
        remaining ^= 1 << idx;
        pieces[i] = whichSetBit[remaining << 4 | idx];
    }

    for (i = 1; i < tier->level; i++) {
        tierPosition += factmul[i << 4 | pieces[i]];
    }

    return tierPosition;
}

int8_t solvePositionLive(QUARTOTIER *tier, uint64_t bitBoard, uint8_t slot) {
    // First, check if current position is primitive.
    int8_t value;
    if (slot == PRIMITIVE_FULL) {
        value = primitiveFull(bitBoard, tier->occupiedSlotsMask);
    } else {
        value = primitivePartial(bitBoard, tier->occupiedSlotsMask, slot);
    }

    if (value != QUARTO_UNDECIDED) {
        return value;
    }

    // If not, then check child positons.
    int8_t i, j, nextSlot, childValue, minChildValue = 24;
    uint64_t childBitBoard;
    QUARTOTIER childTier;
    childTier.level = tier->level + 1;
    uint8_t *emptySlots = unsetBitLists + (tier->occupiedSlots << 4);

    if (tier->level < 15) {
        childTier.piecesPlaced = tier->piecesPlaced | (1 << tier->pieceToPlace);
        uint8_t *remainingPieces = unsetBitLists + ((childTier.piecesPlaced) << 4);
        for (i = 0; i < 16 - tier->level; i++) { // Iterate through all possible empty slots
            nextSlot = emptySlots[i];
            childTier.occupiedSlots = tier->occupiedSlots | (1 << nextSlot);
            childTier.occupiedSlotsMask = tier->occupiedSlotsMask | (UINT64_C(0xF) << (nextSlot << 2));
            childBitBoard = bitBoard | (((uint64_t) tier->pieceToPlace) << (nextSlot << 2));
            for (j = 0; j < 15 - tier->level; j++) {  // Iterate through all possible remaining pieces
                childTier.pieceToPlace = remainingPieces[j];
                childValue = solvePositionLive(&childTier, childBitBoard, nextSlot);
                if (childValue == QUARTO_LOSE0) {
                    return 17 - tier->level; // Max value for a level
                } else if (childValue < minChildValue) {
                    minChildValue = childValue;
                }
            }
        }
    } else {
        nextSlot = emptySlots[0];
        childTier.occupiedSlotsMask = UINT64_C(-1);
        childBitBoard = bitBoard | (((uint64_t) tier->pieceToPlace) << (nextSlot << 2));
        /* childTier.piecesPlaced,occupiedSlots, pieceToPlace are only needed 
        for generateMoves and level 16 is primitive, so no need to initialize them */
        minChildValue = solvePositionLive(&childTier, childBitBoard, nextSlot);
    }
    return 17 - tier->level - minChildValue;
}

VALUE quartodb_set_value(POSITION pos, VALUE val) {
	return 0;
}

VALUE quartodb_get_value(POSITION pos) {
	return 0;
}

REMOTENESS quartodb_get_remoteness(POSITION pos) {
	return 1;
}

void quartodb_set_remoteness(POSITION pos, REMOTENESS val) {
	return;
}

BOOLEAN quartodb_check_visited(POSITION pos) {
	return FALSE;
}

void quartodb_mark_visited (POSITION pos) {
	return;
}

void quartodb_unmark_visited (POSITION pos) {
	return;
}

void quartodb_set_mex(POSITION pos, MEX mex) {
	return;
}

MEX quartodb_get_mex(POSITION pos) {
	return 0;
}

BOOLEAN quartodb_save_database () {
	return FALSE;
}

BOOLEAN quartodb_load_database() {
	return TRUE;
}

STRING vctvs(char value_char) {
	switch(value_char) {
		case 'T':
			return "tie";
		case 'W':
			return "win";
		case 'L':
			return "lose";
		default:
			return "error";
	}
}

uint64_t canonicalize(QUARTOTIER *tier, QUARTOTIER *symmetricTier, uint64_t bitBoard) {
    CANON_STRUCT cspp = ppCanonForward[(((uint32_t) tier->pieceToPlace) << 16) | tier->piecesPlaced];
    CANON_STRUCT csos = osCanonForward[tier->occupiedSlots];
    uint8_t *ppTransform = reorderMatrix[cspp.transform];
    uint8_t *osTransform = osTransforms[csos.transform];
    uint64_t xorBitBoard = (bitBoard ^ (tier->pieceToPlace * 0x1111111111111111)) & tier->occupiedSlotsMask;
    uint64_t symmetricBitBoard = 0;
    for (int i = 0; i < 16; i++) {
        symmetricBitBoard |= ((uint64_t) ppTransform[((xorBitBoard >> (osTransform[i] << 2)) & 0xF)]) << (i << 2);
    }

    buildTier(symmetricTier, 0, cspp.canon, csos.canon);

    return symmetricBitBoard;
}

void getValueRemoteness(int level, QUARTOTIER *tier, uint64_t bitBoard, char *valueChar, int *remoteness) {
    if (level <= 2) {
        *valueChar = 'T';
        *remoteness = 16 - level;
    } else if (level < 13) {
        QUARTOTIER canonicalTier;
        uint64_t symmetricBitBoard = canonicalize(tier, &canonicalTier, bitBoard);
        uint64_t tierPosition = quartoHash(&canonicalTier, symmetricBitBoard);
        int8_t vr = quartodb_get_valueremoteness_from_file(&canonicalTier, tierPosition);
        *valueChar = values[level][vr];
        *remoteness = remotenesses[level][vr];
    } else {
        int8_t vr = solvePositionLive(tier, bitBoard, PRIMITIVE_FULL);
        *valueChar = values[level][vr];
        *remoteness = remotenesses[level][vr];
    }
}

void quartoDetailedPositionResponse(STRING str) {
	printf("result =>> {\"status\":\"ok\",\"response\":{");

    enum UWAPI_Turn turn;
	unsigned int num_rows, num_columns; // Unused
    STRING board;
	if (!UWAPI_Board_Regular2D_ParsePositionString(str, &turn, &num_rows, &num_columns, &board)) {
		// Failed to parse string
		printf("}}");
        return;
	}

    int level = 16;
    uint16_t piecesPlaced = 0;
    uint16_t occupiedSlots = 0xFFFF;
    uint64_t bitBoard = 0;
    uint64_t piece;
    int i, j;

    for (i = 0; i < 16; i++) {
        if (board[i] == '\0') {
            printf("}}"); // Invalid board string: board string not long enough
            return;
        } else if (board[i] == '-') {
            level--;
            occupiedSlots ^= UINT16_C(1) << i;
        } else {
            piece = board[i] - 'A';
            if (piece < 0 || piece > 15) {
                printf("}}"); // Invalid board string: contains invalid characters
                return;
            }
            piecesPlaced |= UINT16_C(1) << piece;
            bitBoard |= piece << (i << 2);
        }
    }
    int piecesPlacedCount, occupiedSlotsCount;
    for (i = 0; i < 16; i++) {
        if (piecesPlaced & (1 << i)) piecesPlacedCount++;
        if (occupiedSlots & (1 << i)) occupiedSlotsCount++;
    }
    if (piecesPlacedCount != occupiedSlotsCount) {
        printf("}}"); // Invalid board string: number of pieces placed does not match number of occupied slots
        return;
    }

    char valueChar;
    int remoteness;
    QUARTOTIER tier;
    uint8_t pieceToPlace = 0;
    BOOLEAN isPrimitive = FALSE;
    if (board[16] == '-') {
        if (level == 0) {
            level = -1;
        } else {
            isPrimitive = TRUE;
        }
    } else {
        pieceToPlace = board[16] - 'A';
    }

    buildTier(&tier, pieceToPlace, piecesPlaced, occupiedSlots);
    getValueRemoteness(level, &tier, bitBoard, &valueChar, &remoteness);

	printf("\"board\":\"%s\",", str);
	printf("\"remoteness\":%d,", remoteness);
    printf("\"value\":\"%s\",", vctvs(valueChar));
	printf("\"moves\":[");

    if (isPrimitive) { // handles moves list for primitive and level 16
        printf("]}}");
        return;
    }

    //////////////

    turn = (turn == UWAPI_TURN_A) ? UWAPI_TURN_B : UWAPI_TURN_A;
    uint8_t *emptySlots = unsetBitLists + (tier.occupiedSlots << 4);
    QUARTOTIER childTier;
    childTier.level = tier.level + 1;
    int8_t nextSlot;
    uint64_t childBitBoard;
    if (level == -1) {
        for (i = 0; i < 16; i++) {
            printf("{\"board\":\"R_B_17_1_----------------%c\",\"remoteness\":16,\"value\":\"tie\",", i + 'A');
            printf("\"move\":\"A_%c_%d\",", i + 'A', 272 + i); /// TODO
            printf("\"moveName\":\"%d%d%d%d\"}", (i>>3)&1, (i>>2)&1, (i>>1)&1, i&1);
            if (i < 15) {
                printf(",");
            }
        }
    } else if (level < 15) {
        childTier.piecesPlaced = tier.piecesPlaced | (1 << tier.pieceToPlace);
        uint8_t *remainingPieces = unsetBitLists + ((childTier.piecesPlaced) << 4);
        for (i = 0; i < 16 - level; i++) { // Iterate through all possible empty slots
            nextSlot = emptySlots[i];
            childTier.occupiedSlots = tier.occupiedSlots | (1 << nextSlot);
            childTier.occupiedSlotsMask = tier.occupiedSlotsMask | (UINT64_C(0xF) << (nextSlot << 2));
            childBitBoard = bitBoard | (((uint64_t) tier.pieceToPlace) << (nextSlot << 2));
            board[nextSlot] = pieceToPlace + 'A';
            for (j = 0; j < 15 - level; j++) {  // Iterate through all possible remaining pieces
                childTier.pieceToPlace = remainingPieces[j];
                getValueRemoteness(level + 1, &childTier, childBitBoard, &valueChar, &remoteness);
                if (remoteness) { // non-primitive child
                    board[16] = childTier.pieceToPlace + 'A';
                    printf("{\"board\":\"%s\",", UWAPI_Board_Regular2D_MakePositionString(turn, 17, 1, board));
                    printf("\"remoteness\":%d,", remoteness);
                    printf("\"value\":\"%s\",", vctvs(valueChar));
                    printf("\"move\":\"A_%c_%d\",", childTier.pieceToPlace + 'a', 16 + nextSlot * 16 + childTier.pieceToPlace);
                    printf("\"moveName\":\"%d-%d%d%d%d\"}", nextSlot+1, (childTier.pieceToPlace>>3)&1, (childTier.pieceToPlace>>2)&1, (childTier.pieceToPlace>>1)&1, childTier.pieceToPlace&1);  // slot is 1-indexed in moveName
                    if (i < 15 - level || j < 14 - level) {
                        printf(",");
                    }
                } else {
                    board[16] = '-';
                    printf("{\"board\":\"%s\",", UWAPI_Board_Regular2D_MakePositionString(turn, 17, 1, board));
                    printf("\"remoteness\":%d,", remoteness);
                    printf("\"value\":\"%s\",", vctvs(valueChar));
                    printf("\"move\":\"A_-_%d\",", nextSlot); 
                    printf("\"moveName\":\"%d\"}", nextSlot+1); // slot is 1-indexed in moveName
                    if (i < 15 - level) {
                        printf(",");
                    }
                    break;
                }
            }
            board[nextSlot] = '-';
        }
        board[16] = pieceToPlace + 'A';
    } else { // level == 15
        nextSlot = emptySlots[0];
        childTier.occupiedSlotsMask = UINT64_C(-1);
        childBitBoard = bitBoard | (((uint64_t) tier.pieceToPlace) << (nextSlot << 2));
        getValueRemoteness(level + 1, &childTier, childBitBoard, &valueChar, &remoteness);
        board[nextSlot] = pieceToPlace + 'A';
        board[16] = '-';
        printf("{\"board\":\"%s\",", UWAPI_Board_Regular2D_MakePositionString(turn, 17, 1, board));
        printf("\"remoteness\":%d,", remoteness);
        printf("\"value\":\"%s\",", vctvs(valueChar));
        printf("\"move\":\"A_-_%d\",", nextSlot); 
        printf("\"moveName\":\"%d\"}", nextSlot+1); // slot is 1-indexed in moveName
        board[16] = pieceToPlace + 'A';
    }

	printf("]}}");
}