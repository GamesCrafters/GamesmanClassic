/************************************************************************
**
** NAME:        Othello Four (Used Blob Database)
**
** DESCRIPTION: AKA Reversi
**
** AUTHOR:      Abraham Hsu
**              Aryaman Asthana
**
** DATE:        5 February 2026
**************************************************************************/

#include "gamesman.h"
#include <zstd.h>
#include "stdlib.h"

POSITION gNumberOfPositions;
POSITION kBadPosition = INVALID_POSITION;

POSITION gInitialPosition = (POSITION){ 0b0000010000100000ULL, 0b0000001001000000ULL };
POSITION gMinimalPosition = (POSITION){ 0b0ULL, 0b0ULL };

CONST_STRING kAuthorName = "Abraham Hsu, Aryaman Asthana";
CONST_STRING kGameName = "mothellofour";
CONST_STRING kDBName = "mothelloblob";
BOOLEAN kPartizan = TRUE;
BOOLEAN kDebugMenu = FALSE;
BOOLEAN kGameSpecificMenu = FALSE;
BOOLEAN kTieIsPossible = TRUE;
BOOLEAN kLoopy = FALSE;
BOOLEAN kDebugDetermineValue = FALSE;
BOOLEAN kSupportsSymmetries = FALSE;
void *gGameSpecificTclInit = NULL;

CONST_STRING kHelpGraphicInterface = "";

CONST_STRING kHelpTextInterface = "";

CONST_STRING kHelpOnYourTurn = "";

CONST_STRING kHelpStandardObjective = "";

CONST_STRING kHelpReverseObjective = "";

CONST_STRING kHelpTieOccursWhen =
    /* Should follow 'A Tie occurs when... */ "";

CONST_STRING kHelpExample = "";

/*************************************************************************
**
** Every variable declared here is only used in this file
*(game-specific)
**
**************************************************************************/

#define PAGE_BITS 12
#define N 4
#define CELLS (N * N)
#define FULL ((1ULL << CELLS) - 1ULL)

#define R0 0xFULL
#define R1 (R0 << 4)
#define R2 (R0 << 8)
#define R3 (R0 << 12)

#define C0 0x1111ULL
#define C1 0x2222ULL
#define C2 0x4444ULL
#define C3 0x8888ULL

#define A_FILE C0
#define D_FILE C3
#define NOT_A (FULL ^ A_FILE)
#define NOT_D (FULL ^ D_FILE)
#define min(a,b) (((a) < (b)) ? (a) : (b))


/* Utility Functions for Computing Canonical Positions */
BITBOARD vertical4(BITBOARD b) {
    BITBOARD res = 0;
    res |= ((b & C0) << 3);  // col0 -> col3
    res |= ((b & C1) << 1);  // col1 -> col2
    res |= ((b & C2) >> 1);  // col2 -> col1
    res |= ((b & C3) >> 3);  // col3 -> col0
    return res;
}

BITBOARD horizontal4(BITBOARD b) {
    BITBOARD res = 0;
    res |= ((b & R0) << 12); // row0 -> row3
    res |= ((b & R1) << 4 ); // row1 -> row2
    res |= ((b & R2) >> 4 ); // row2 -> row1
    res |= ((b & R3) >> 12); // row3 -> row0
    return res;
}

BITBOARD transpose4(BITBOARD b) {
    BITBOARD res = 0;

    // Diagonal stays
    BITBOARD DIAG = (1ULL<<0) | (1ULL<<5) | (1ULL<<10) | (1ULL<<15);
    res |= (b & DIAG);

    // Offset 1: (0,1),(1,2),(2,3) <-> (1,0),(2,1),(3,2)
    BITBOARD OFF1 = (1ULL<<1) | (1ULL<<6) | (1ULL<<11);
    BITBOARD SWAP1 = (OFF1 << 3);
    res |= ((b & OFF1) << 3);
    res |= ((b & SWAP1) >> 3);

    // Offset 2: (0,2),(1,3) <-> (2,0),(3,1)
    BITBOARD OFF2 = (1ULL<<2) | (1ULL<<7);
    BITBOARD SWAP2 = (OFF2 << 6);
    res |= ((b & OFF2) << 6);
    res |= ((b & SWAP2) >> 6);

    // Offset 3: (0,3) <-> (3,0)
    BITBOARD OFF3 = (1ULL<<3);
    BITBOARD SWAP3 = (OFF3 << 9);
    res |= ((b & OFF3) << 9);
    res |= ((b & SWAP3) >> 9);

    return res;
}
/* Utility Functions for Canonical Positions End Here */

POSITION GetCanonicalPosition(POSITION position) {
    BITBOARD me = position.player & FULL;
    BITBOARD opp = position.opponent & FULL;
    BITBOARD occ = me | opp;

    /* Finding "blob" symmetries */
    BITBOARD occ0 = occ;
    BITBOARD occ1 = vertical4(occ);
    BITBOARD occ2 = horizontal4(occ);
    BITBOARD occ3 = transpose4(occ);
    BITBOARD occ5 = vertical4(occ3);
    BITBOARD occ7 = horizontal4(occ3);
    BITBOARD occ6 = horizontal4(occ1);
    BITBOARD occ4 = horizontal4(occ5);

    /* Find Minimal Occupancy and Best Encoding for Canonical */
    BITBOARD best_occ = occ0;
    BITBOARD best_me = me;
    BITBOARD t = transpose4(me);
    BITBOARD v = vertical4(me);
    BITBOARD vt = vertical4(t);

    if (occ1 <= best_occ) {
        if (occ1 < best_occ) {
            best_occ = occ1;
            best_me = v;
        } else {
            best_me = min(v, best_me);
        }
    }

    if (occ2 <= best_occ) {
        BITBOARD h = horizontal4(me);
        if (occ2 < best_occ) {
            best_occ = occ2;
            best_me = h;
        } else {
            best_me = min(h, best_me);
        }
    }

    if (occ3 <= best_occ) {
        if (occ3 < best_occ) {
            best_occ = occ3;
            best_me = t;
        } else {
            best_me = min(t, best_me);
        }
    }

    if (occ5 <= best_occ) {
        if (occ5 < best_occ) {
            best_occ = occ5;
            best_me = vt;
        } else {
            best_me = min(vt, best_me);
        }
    }

    if (occ7 <= best_occ) {
        BITBOARD ht = horizontal4(t);
        if (occ7 < best_occ) {
            best_occ = occ7;
            best_me = ht;
        } else {
            best_me = min(ht, best_me);
        }
    }

    if (occ6 <= best_occ) {
        BITBOARD hv = horizontal4(v);
        if (occ6 < best_occ) {
            best_occ = occ6;
            best_me = hv;
        } else {
            best_me = min(hv, best_me);
        }
    }

    if (occ4 <= best_occ) {
        BITBOARD hvt = horizontal4(vt);
        if (occ4 < best_occ) {
            best_occ = occ4;
            best_me = hvt;
        } else {
            best_me = min(hvt, best_me);
        }
    }

    return (POSITION){ best_me, best_occ ^ best_me };
}

void DebugMenu() {}

void SetTclCGameSpecificOptions(int theOptions[]) {
    (void)theOptions;
}

POSITION starting_position() {
    return (POSITION){ 0b0000010000100000ULL, 0b0000001001000000ULL };
}

void StartingPositionToString(char* buf) {
    PositionToAutoGUIString(starting_position(), buf+2);
    buf[0] = '1';
    buf[1] = '_';
    return;
}
/************************************************************************
**
** NAME: InitializeDatabases
**
** DESCRIPTION: Initialize the gDatabase, a global variable.
**
************************************************************************/
void InitializeGame() {
    kUsesBlobGamesman = TRUE;
    gCanonicalPosition = GetCanonicalPosition;

    gInitialPosition = starting_position();
}

/************************************************************************
**
** NAME: GameSpecificMenu
**
** DESCRIPTION: Menu used to change game-specific parmeters, such as
** the side of the board in an nxn Nim board, etc. Does
** nothing if kGameSpecificMenu == FALSE
**
************************************************************************/

void GameSpecificMenu() {}

/************************************************************************
**
** NAME: DoMove
**
** DESCRIPTION: Apply the move to the position.
**
** INPUTS: POSITION position : The old position
** MOVE move : The move to apply.
**
** OUTPUTS: (POSITION) : The position that results after the move.
**
** CALLS: PositionToBlankOX(POSITION,*BlankOX)
** BlankOX WhosTurn(*BlankOX)
**
************************************************************************/

BITBOARD sweep_l(BITBOARD mv, BITBOARD me, BITBOARD opp, int s, BITBOARD mask) {
    BITBOARD flips = 0;
    BITBOARD cur = (mv << s) & mask;
    // Walk through opponent stones
    while (cur && (cur & opp)) {
        flips |= cur;
        cur = (cur << s) & mask;
    }
    // If we ended on our own stone, the path is valid; otherwise discard
    return (cur & me) ? flips : 0;
}

// Collect flips in the "right-shift" directions (W, S, SW, SE)
BITBOARD sweep_r(BITBOARD mv, BITBOARD me, BITBOARD opp, int s, BITBOARD mask) {
    BITBOARD flips = 0;
    BITBOARD cur   = (mv >> s) & mask;
    while (cur && (cur & opp)) {
        flips |= cur;
        cur = (cur >> s) & mask;
    }
    return (cur & me) ? flips : 0;
}

POSITION DoMove(POSITION position, MOVE move) {
    BITBOARD me = position.player & FULL;
    BITBOARD opp = position.opponent & FULL;

    BITBOARD flips =
        sweep_l(move, me, opp, 1,    NOT_A) |  // East
        sweep_r(move, me, opp, 1,    NOT_D) |  // West
        sweep_l(move, me, opp, N,    FULL ) |  // North
        sweep_r(move, me, opp, N,    FULL ) |  // South
        sweep_l(move, me, opp, N+1,  NOT_A) |  // NE
        sweep_r(move, me, opp, N+1,  NOT_D) |  // SW
        sweep_l(move, me, opp, N-1,  NOT_D) |  // NW
        sweep_r(move, me, opp, N-1,  NOT_A);   // SE

    me  ^= flips;
    opp ^= flips;
    me  |= move;

    /* 
    We implicitly flip this so that when we are
    looking for children nodes, we do not need to 
    flip it manually.
    */
    return (POSITION){ opp & FULL , me & FULL };
}

/************************************************************************
**
** NAME: PrintComputersMove
**
** DESCRIPTION: Nicely format the computers move.
**
** INPUTS: MOVE *computersMove : The computer's move.
** STRING computersName : The computer's name.
**
************************************************************************/

void PrintComputersMove(MOVE computersMove, STRING computersName) {
    (void)computersMove;
    (void)computersName;
    printf("the computer moved\n");
}

/************************************************************************
**
** NAME: Primitive
**
** DESCRIPTION: Return the value of a position if it fulfills certain
** 'primitive' constraints. Some examples of this is having
** three-in-a-row with TicTacToe. TicTacToe has two
** primitives it can immediately check for, when the board
** is filled but nobody has one = primitive tie. Three in
** a row is a primitive lose, because the player who faces
** this board has just lost. I.e. the player before him
** created the board and won. Otherwise undecided.
**
** INPUTS: POSITION position : The position to inspect.
**
** OUTPUTS: (VALUE) an enum which is oneof: (win,lose,tie,undecided)
**
************************************************************************/

VALUE Primitive(POSITION position) {
    (void)position;
    return undecided;
}

/************************************************************************
**
** NAME: PrintPosition
**
** DESCRIPTION: Print the position in a pretty format, including the
** prediction of the game's outcome.
**
** INPUTS: POSITION position : The position to pretty print.
** STRING playerName : The name of the player.
** BOOLEAN usersTurn : TRUE <==> it's a user's turn.
**
** CALLS: PositionToBlankOX()
** GetValueOfPosition()
** GetPrediction()
**
************************************************************************/

void PrintPosition(POSITION position, STRING playerName,
                   BOOLEAN usersTurn) {
    return;
}

/************************************************************************
**
** NAME: GenerateMoves
**
** DESCRIPTION: Create a linked list of every move that can be reached
** from this position. Return a pointer to the head of the
** linked list.
**
** INPUTS: POSITION position : The position to branch off of.
**
** OUTPUTS: (MOVELIST *), a pointer that points to the first item
** in the linked list of moves that can be generated.
**
** CALLS: MOVELIST *CreateMovelistNode(MOVE,MOVELIST *)
**
************************************************************************/

BITBOARD ks_dir_l(BITBOARD me, BITBOARD opp, BITBOARD empty, int s, BITBOARD mask) {
    BITBOARD t = opp & mask & (me << s);
    t |= opp & mask & (t << s);
    t |= opp & mask & (t << s);
    return (t << s) & mask & empty;
}
BITBOARD ks_dir_r(BITBOARD me, BITBOARD opp, BITBOARD empty, int s, BITBOARD mask) {
    BITBOARD t = opp & mask & (me >> s);
    t |= opp & mask & (t >> s);
    t |= opp & mask & (t >> s);
    return (t >> s) & mask & empty;
}

MOVELIST *GenerateMoves(POSITION position) {
    MOVELIST *moves = NULL;
    
    BITBOARD me = position.player & FULL;
    BITBOARD opp = position.opponent & FULL;
    BITBOARD empty = FULL & ~(me | opp);

    BITBOARD m = 0;
    m |= ks_dir_l(me, opp, empty, 1,   NOT_A); // east
    m |= ks_dir_r(me, opp, empty, 1,   NOT_D); // west
    m |= ks_dir_l(me, opp, empty, N,   FULL ); // north
    m |= ks_dir_r(me, opp, empty, N,   FULL ); // south
    m |= ks_dir_l(me, opp, empty, N+1, NOT_A); // NE
    m |= ks_dir_r(me, opp, empty, N+1, NOT_D); // SW
    m |= ks_dir_l(me, opp, empty, N-1, NOT_D); // NW
    m |= ks_dir_r(me, opp, empty, N-1, NOT_A); // SE

    MOVES set_of_moves = m & FULL;
    while (set_of_moves) {
        BITBOARD lsb = set_of_moves & -set_of_moves;
        moves = CreateMovelistNode(lsb, moves);
        set_of_moves ^= lsb;
    }
    return moves;
}

/************************************************************************
**
** NAME: GetAndPrintPlayersMove
**
** DESCRIPTION: This finds out if the player wanted an undo or abort
*or not.
** If so, return Undo or Abort and don't change theMove.
** Otherwise get the new theMove and fill the pointer up.
**
** INPUTS: POSITION *thePosition : The position the user is at.
** MOVE *theMove : The move to fill with user's move.
** STRING playerName : The name of the player whose turn it is
**
** OUTPUTS: USERINPUT : Oneof( Undo, Abort, Continue )
**
** CALLS: ValidMove(MOVE, POSITION)
** BOOLEAN PrintPossibleMoves(POSITION) ...Always True!
**
************************************************************************/

USERINPUT GetAndPrintPlayersMove(POSITION position, MOVE *move,
                                 STRING playerName) {
    /* local variables */
    USERINPUT ret;
    do {
        printf("Enter your move here: ");
        ret = HandleDefaultTextInput(position, move, playerName);
        if (ret != Continue) return (ret);
    } while (TRUE);
    return (
        Continue); /* this is never reached, but link is now happy */
}

/************************************************************************
**
** NAME: ValidTextInput
**
** DESCRIPTION: Return TRUE iff the string input is of the right
*'form'.
** For example, if the user is allowed to select one slot
** from the numbers 1-9, and the user chooses 0, it's not
** valid, but anything from 1-9 IS, regardless if the slot
** is filled or not. Whether the slot is filled is left up
** to another routine.
**
** INPUTS: STRING input : The string input the user typed.
**
** OUTPUTS: BOOLEAN : TRUE iff the input is a valid text input.
**
************************************************************************/

BOOLEAN ValidTextInput(STRING input) {
    (void)input;
    return TRUE;
}

/************************************************************************
**
** NAME: ConvertTextInputToMove
**
** DESCRIPTION: Convert the string input to the internal move
*representation.
**
** INPUTS: STRING input : The string input the user typed.
**
** OUTPUTS: MOVE : The move corresponding to the user's input.
**
************************************************************************/

MOVE ConvertTextInputToMove(STRING input) {
    return 0;
}

/************************************************************************
**
** NAME: MoveToString
**
** DESCRIPTION: Returns the move as a STRING
**
** INPUTS: MOVE *move : The move to put into a string.
**
************************************************************************/

void MoveToString(MOVE move, char *moveStringBuffer) {
    return;
}

int NumberOfOptions() {
    return 0;
}

int getOption() {
    return 0;
}

void setOption(int option) {
    return;
}

POSITION StringToPosition(char *positionString) {
    
    POSITION ret_pos;
    int turn;
    char *currBoard;
    if (!ParseStandardOnelinePositionString(positionString, &turn, &currBoard)) {
        return NULL_POSITION;
	}
    int n = strlen(currBoard);
    ret_pos.player = 0;
    ret_pos.opponent = 0;
	for (int idx = 0; idx < n; idx++) {
        if ((turn == 1 && currBoard[idx] == 'b') || (turn == 2 && currBoard[idx] == 'w')) {
            ret_pos.player |= (1ULL << idx);
        } else if ((turn == 2 && currBoard[idx] == 'b') || (turn == 1 && currBoard[idx] == 'w')){
            ret_pos.opponent |= (1ULL << idx);
        }
    }
	return ret_pos;
}

void PositionToAutoGUIString(POSITION position, char *autoguiPositionStringBuffer) {
    for (int i = 0; i < CELLS; i++) {
        if ((position.player >> i) & 1) {
            autoguiPositionStringBuffer[i] = 'b';
        } else if ((position.opponent >> i) & 1) {
            autoguiPositionStringBuffer[i] = 'w';
        } else {
            autoguiPositionStringBuffer[i] = '-';
        }
    }
    autoguiPositionStringBuffer[CELLS] = '\0';
    return;
}

void MoveToAutoGUIString(POSITION position, MOVE move, char *autoguiMoveStringBuffer) {
    int move_idx = -1;
    for (int i = 0; i < CELLS; i++) {
        if ((move >> i) & 1) {
            move_idx = i;
            break;
        }
    }
    if (move_idx == -1) {
        snprintf(autoguiMoveStringBuffer, 16, "INVALID");
        return;
    }
    snprintf(autoguiMoveStringBuffer, 16, "A_-_%d_x", move_idx);
    return;
}

BITBOARD shape(const POSITION* p) {
    return (p->player | p->opponent) & FULL;
}

uint8_t tier_of(uint64_t shapeMask) {
#if defined(_MSC_VER)
    return (uint8_t)__popcnt64(shapeMask);

#elif defined(__GNUC__) || defined(__clang__)
    return (uint8_t)__builtin_popcountll(shapeMask);

#else
    uint64_t x = shapeMask;
    int c = 0;
    while (x) {
        x &= (x - 1);
        ++c;
    }
    return (uint8_t)c;
#endif
}

#pragma pack(push,1)
typedef struct PageIdxRec {
    uint64_t shape;
    uint32_t page;
    uint64_t off;
} PageIdxRec;
#pragma pack(pop)

_Static_assert(sizeof(PageIdxRec) == 20, "PageIdxRec must be 20 bytes");

uint64_t hash(const POSITION *p) {
    BITBOARD sh = shape(p);

#if defined(__BMI2__)
    return _pext_u64(p->player, sh);
#else
    /* Portable fallback: iterate set bits of sh (in index order) and pack. */
    uint64_t h = 0;
    uint64_t pos = 0;
    BITBOARD m = sh;

    while (m) {
        BITBOARD b = m & -m;
        if (p->player & b)
            h |= (1ULL << pos);

        ++pos;
        m ^= b;   /* clear the bit */
    }

    return h;
#endif
}

uint64_t mix64(uint64_t x) {
    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdULL;
    x ^= x >> 33;
    x *= 0xc4ceb9fe1a85ec53ULL;
    x ^= x >> 33;
    return x;
}

uint64_t fast_reduce(uint64_t x, uint64_t n) {
#if defined(_MSC_VER) && defined(_M_X64)
    unsigned __int64 hi;
    (void)_umul128(x, n, &hi);  // high 64 bits of x*n
    return hi;
#elif defined(__SIZEOF_INT128__)
    // Portable GCC/Clang version using __uint128_t
    __uint128_t prod = (__uint128_t)x * (__uint128_t)n;
    return (uint64_t)(prod >> 64);
#else
    // Fallback: naive modulo (not constant-time / less uniform)
    return x % n;
#endif
}

int owner_of_shape(uint64_t shape, int W) {
    if (W <= 0) return 0;

    const uint64_t kSalt = 0x9E3779B97F4A7C15ULL; // golden ratio-based
    uint64_t h = mix64(shape ^ kSalt);

    return (int)fast_reduce(h, (uint64_t)W);
}

POSITION flip(POSITION p) {
    return (POSITION){ p.opponent, p.player };
}

void decode_key_value_pairs(const uint8_t* buf, size_t len, uint8_t* page) {
    memset(page, 0, 1 << PAGE_BITS);

    size_t i = 0;
    while (i < len) {
        uint8_t key   = buf[i++];
        uint8_t count = buf[i++] - 1;
        for (uint8_t j = 0; j < count; j++) {
            uint16_t idx = ((uint16_t)buf[i] << 8) | buf[i+1];
            i += 2;
            page[idx] = key;
        }
    }
}

void decode_group_combinations(const uint8_t* encoded, uint8_t* page) {
    int limit = 1 << PAGE_BITS;

    size_t idx = 0;
    page[0] = encoded[idx++];

    for (uint8_t k = 1; k <= PAGE_BITS; k++) {
        int set = (1 << k) - 1;
        while (set < limit) {
            page[set] = encoded[idx++];
            int c = set & -set;
            int r = set + c;
            set = (((r ^ set) >> 2) / c) | r;
        }
    }
}

void decode_collect_leaves(const uint16_t* leaves, size_t count, uint8_t* page, size_t page_size) {
    memset(page, 0, page_size);

    for (size_t i = 0; i + 1 < count; i += 2) {
        uint16_t left_word  = leaves[i];
        uint16_t right_word = leaves[i+1];

        uint8_t upper_nibble = left_word  >> 12;
        uint8_t lower_nibble = right_word >> 12;
        uint8_t value        = (upper_nibble << 4) | lower_nibble;

        uint16_t l   =  left_word  & 0x0FFF;
        uint16_t r_1 =  right_word & 0x0FFF;

        for (uint16_t pos = l; pos <= r_1; pos++)
            page[pos] = value;
    }
}

uint64_t array_max(const uint64_t* arr, size_t n) {
    if (n == 0) return 0; // or handle error
    uint64_t max = arr[0];
    for (size_t i = 1; i < n; i++) {
        if (arr[i] > max) max = arr[i];
    }
    return max;
}

int cmp_uint64(const void *a, const void *b) {
    uint64_t x = *(const uint64_t *)a;
    uint64_t y = *(const uint64_t *)b;
    return (x > y) - (x < y);
}

uint64_t get_next_offset(const uint64_t *vec, size_t n, uint64_t curr) {
    // Make a copy (like std::vector sorted = vec)
    uint64_t *sorted = malloc(n * sizeof(uint64_t));
    if (!sorted) return 0;

    for (size_t i = 0; i < n; i++) {
        sorted[i] = vec[i];
    }

    // Sort the copy
    qsort(sorted, n, sizeof(uint64_t), cmp_uint64);

    // Find first element > curr
    for (size_t i = 0; i < n; i++) {
        if (sorted[i] > curr) {
            uint64_t result = sorted[i];
            free(sorted);
            return result;
        }
    }

    free(sorted);
    return 0;
}

// Need to find shape of position and find its owner
void GetBlobFileNameFromPosition(POSITION p, char *filename) {
    const POSITION c  = GetCanonicalPosition(p);
    uint8_t tier = tier_of(shape(&c));
    // printf("tier: %u\n", tier);
    snprintf(filename, 256, "./data/othellofour/tier_%02u/tier.dat", (int)tier);
    return;
}

// Need to, given the file, find the page, then find the offset/position
UINT64 GetInfoFromBlobFile(POSITION p, FILE *f) {
    /* Accessing metadata and offset files*/
    uint64_t W = 0;

    FILE* metadata = fopen("./data/othellofour/metadata.bin", "rb");
    if (!metadata) {
        printf("Metadata open error\n");
        return 0;
    }

    if ((fread(&W, sizeof(W)/2, 1, metadata)) != 1) {
        printf("Metadata read error\n");
        return 0;
    }

    fclose(metadata);

    /* Retrieving offsets to compressed data/page records (tier, owner) */
    const POSITION c  = GetCanonicalPosition(p);
    const BITBOARD sh = shape(&c);
    const uint64_t h  = hash(&c);

    int owner = owner_of_shape(sh, W);
    uint8_t tier = tier_of(sh);

    uint64_t tier_idx = (tier - 4) * 2 * (W * sizeof(W));

    uint64_t comp_data_offsets[W];
    uint64_t comp_idx_offsets[W];

    FILE* offsets = fopen("./data/othellofour/offsets.bin", "rb");
    if (!offsets) {
        printf("Offset open error\n");
        return 0;
    }

    fseek(offsets, tier_idx, SEEK_SET);
    if ((fread(comp_data_offsets, sizeof(uint64_t), W, offsets)) != W) {
        printf("Offset read error\n");
        fclose(offsets);
        return 0;
    }

    if ((fread(comp_idx_offsets, sizeof(uint64_t), W, offsets)) != W) {
        printf("Offset read error\n");
        fclose(offsets);
        return 0;
    }

    fclose(offsets);

    /* Determine size of compressed data in bytes */
    fseek(f, 0, SEEK_END); 

    char filename[256];
    snprintf(filename, 256, "./data/othellofour/tier_%02u/tier.idx", (int)tier);
    FILE* rec_file = fopen(filename, "rb");
    if (!rec_file) {
        printf("Record open error\n");
        return 0;
    }
    fseek(rec_file, 0, SEEK_END);

    uint64_t comp_data_size = (comp_data_offsets[owner] == array_max(comp_data_offsets, W))
    ? ftell(f) - comp_data_offsets[owner]
    : get_next_offset(comp_data_offsets, W, comp_data_offsets[owner]) - comp_data_offsets[owner];

    fseek(f, comp_data_offsets[owner], SEEK_SET); // (tier, owner) based offset to compressed data

    uint64_t comp_idx_size = (comp_idx_offsets[owner] == array_max(comp_idx_offsets, W))
    ? ftell(rec_file) - comp_idx_offsets[owner]
    : get_next_offset(comp_idx_offsets, W, comp_idx_offsets[owner]) - comp_idx_offsets[owner];
    fseek(rec_file, comp_idx_offsets[owner], SEEK_SET); // (tier, owner) based offset to compressed records

    /* Load the relevant compressed data into buffers */
    char data_buffer[comp_data_size];
    char idx_buffer[comp_idx_size];

    if ((fread(data_buffer, 1, comp_data_size, f)) != comp_data_size) {
        printf("Compressed data read error\n");
        fclose(rec_file);
        return 0;
    }

    if ((fread(idx_buffer, 1, comp_idx_size, rec_file)) != comp_idx_size) {
        printf("Compressed record read error\n");
        fclose(rec_file);
        return 0;
    }

    /* Find the matching page record */
    size_t idx_size = comp_idx_size; // uncompressed page records (only gzipped)

    size_t num_records = idx_size / sizeof(struct PageIdxRec);
    struct PageIdxRec *records = (struct PageIdxRec *)idx_buffer;

    struct PageIdxRec *match = NULL;
    for (size_t i = 0; i < num_records; i++) {
        if (records[i].shape == sh && records[i].page == (h >> PAGE_BITS)) {
            match = &records[i];
        }
    }

    if (!match) {
        printf("No matching page record for shape %lu\n", (unsigned long)sh);
        fclose(rec_file);
        return 0;
    }

    uint64_t raw_off   = match->off & ~(3ULL << 62);
    uint64_t next_off  = 0;

    size_t match_index = match - records;

    // Determine compressed chunk size by peeking at the next record's offset 
    if (match_index < num_records - 1) {
        next_off = records[match_index + 1].off & ~(3ULL << 62);
    } else {
        // Last record
        next_off = comp_data_size;
    }

    uint8_t *chunk = (uint8_t *)data_buffer + raw_off;
    size_t chunk_size = ZSTD_findFrameCompressedSize(chunk, next_off - raw_off);

    /* Get decompressed size and allocate */
    size_t decomp_size = ZSTD_getFrameContentSize(chunk, chunk_size);
    if (decomp_size == ZSTD_CONTENTSIZE_UNKNOWN || decomp_size == ZSTD_CONTENTSIZE_ERROR) {
        printf("Decompression size error\n");
        fclose(rec_file);
        return 0;
    }

    uint8_t *decomp = malloc(decomp_size);
    if (!decomp) {
        printf("Allocation error\n");
        fclose(rec_file);
        return 0;
    }

    size_t result = ZSTD_decompress(decomp, decomp_size, chunk, chunk_size);
    if (ZSTD_isError(result)) {
        printf("Decompression error: %s\n", ZSTD_getErrorName(result));
        free(decomp);
        fclose(rec_file);
        return 0;
    }

    /* Reformat decompressed data based on mode of compression */
    uint16_t pos_off = (uint16_t)(h & ((1 << PAGE_BITS) - 1));
    uint8_t comp_mode = (match->off >> 62) & 0b11;
    uint8_t value;

    if (comp_mode == 0) {
        // printf("Case 0\n");
        uint8_t page[1 << PAGE_BITS];
        decode_key_value_pairs(decomp, decomp_size, page);
        value = page[pos_off];
    } else if (comp_mode == 1) {
        // printf("Case 1\n");
        // simply index into buffer
        value = decomp[pos_off];
    } else if (comp_mode == 2) {
        // printf("Case 2\n");
        uint8_t page[1 << PAGE_BITS];
        decode_group_combinations(decomp, page);
        value = page[pos_off];
    } else {
        // printf("Case 3\n");
        uint8_t page[1 << PAGE_BITS];
        decode_collect_leaves((const uint16_t *)decomp,
                                  decomp_size / sizeof(uint16_t),
                                  page,
                                  1 << PAGE_BITS);
        value = page[pos_off];
    }

    /* Free resources and return if possible */
    free(decomp);
    fclose(rec_file);
    if (value) return value;

    // printf("SKIP MOVE\n");

    /* If skip, make one more function call */
    return GetInfoFromBlobFile(flip(p), f);
}

STRING GetPrimitiveFromInfo(UINT64 info) {
    UINT64 prim_num = 0b11000000 & info;
    if (prim_num == 0b11000000) {
        return "win";
    } else if (prim_num == 0b01000000) {
        return "lose";
    } else if (prim_num == 0b10000000) {
        return "tie";
    }
    return "INVALID";
}

REMOTENESS GetRemotenessFromInfo(UINT64 info) {
    return 0b00111111 & info;
}