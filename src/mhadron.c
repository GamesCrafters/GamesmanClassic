/************************************************************************
**
** NAME:        mhadron.c
**
** DESCRIPTION: Hadron, by Mark Steere (2022)
**
** AUTHOR:      GamesCrafters, UC Berkeley
**
** DETAILS:     Blue moves first. A tile may be placed on an empty cell only
**              if it would have as many friendly orthogonal neighbours as
**              enemy ones: 0 and 0, 1 and 1, or 2 and 2. Passing is not
**              allowed, so the last player to place wins and a player with
**              no legal placement loses. There are no ties or draws.
**
**              TIERS. A tier is (number of tiles placed, contents of the
**              top row). Blue moves first and the players alternate, so the
**              piece counts follow from the total alone: blues = ceil(p/2),
**              reds = floor(p/2). Every move adds a tile and a filled cell
**              never empties, so tiers only ever grow heavier -- acyclic,
**              as tier solving requires.
**
**              Why the top row is in the tier key: on 5x5 the piece counts
**              alone put 26.3 billion positions in the largest tier, and
**              GamesmanClassic holds a tier plus all its children in one
**              array at 2 bytes a slot, which is 98 GiB. Fixing the top row
**              as well cuts the largest tier to 133 million and the peak
**              hash window to 1.73 GiB. Only the other N*N-N cells are
**              hashed; the top row is read back from the tier.
**
**              HASH. A position is two bitboards. Within a tier, its number
**              is the rank of the blue cells among all sets of that size,
**              times the number of ways to place the reds, plus the rank
**              of the red cells among the cells blue left empty. Ranks are
**              colex, which for sets of one size is plain numeric order, so
**              one table of 2^(N*N-N) entries serves every set size, and a
**              set of k cells drawn from fewer cells ranks the same way.
**              Compressing the reds into the cells blue left empty, and
**              back, is a bit gather and scatter done 7 bits at a time.
**
**              PRUNING. IsLegal lets the solver skip, and store nothing
**              for, boards that no game can produce. It checks four
**              properties that every reachable board has and that every
**              legal move preserves; the second part matters, because the
**              solver stops if a solved position has an unsolved child.
**              On 5x5 they pass about 3% of slots in the larger tiers.
**              See IsLegal for the argument behind each one.
**
**              SYMMETRY is deliberately not used. Hadron has the usual 8
**              board symmetries, but a rotation moves the top row somewhere
**              else, so a canonical form would cross tier boundaries.
**
************************************************************************/

#include "gamesman.h"
#include <stdint.h>

/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/

POSITION gNumberOfPositions  = 0;
POSITION kBadPosition        = -1;

POSITION gInitialPosition    = 0;
POSITION gMinimalPosition    = 0;

CONST_STRING kAuthorName         = "GamesCrafters";
CONST_STRING kGameName           = "Hadron";
CONST_STRING kDBName             = "hadron";
BOOLEAN kPartizan                = TRUE;
BOOLEAN kDebugMenu               = FALSE;
BOOLEAN kGameSpecificMenu        = TRUE;
BOOLEAN kTieIsPossible           = FALSE;
BOOLEAN kLoopy                   = FALSE;
BOOLEAN kDebugDetermineValue     = FALSE;
void*   gGameSpecificTclInit     = NULL;

CONST_STRING kHelpGraphicInterface =
        "Click an empty cell to place one of your tiles there. A cell is only\n\
playable when it has as many of your tiles next to it as your opponent's.";

CONST_STRING kHelpTextInterface =
        "On your turn, type the column letter followed by the row number of the\n\
cell you want (for example C3) and hit return. Only cells with as many\n\
friendly as enemy orthogonal neighbours may be played.";

CONST_STRING kHelpOnYourTurn =
        "Place one of your tiles on an empty cell that has as many of your tiles\n\
orthogonally adjacent to it as your opponent's: none and none, one and one,\n\
or two and two.";

CONST_STRING kHelpStandardObjective =
        "Make the last legal placement. A player with no legal placement on their\n\
turn loses.";

CONST_STRING kHelpReverseObjective =
        "Force your opponent to make the last legal placement.";

CONST_STRING kHelpTieOccursWhen =
        "never: one player always runs out of placements first.";

CONST_STRING kHelpExample =
        "    A B C D E\n\
 5  - - - - -\n\
 4  - - - - -\n\
 3  - - B - -\n\
 2  - - - - -\n\
 1  - - - - -\n\
\n\
Blue has opened in the centre. Red may now play any cell that is not\n\
orthogonally adjacent to that tile, since such a cell would have one enemy\n\
neighbour and no friendly one.";

/*************************************************************************
**
** Everything above here must be in every game file
**
**************************************************************************/

/*************************************************************************
**
** Game-specific declarations
**
**************************************************************************/

#define BLUE     'B'      /* player 1, moves first */
#define RED      'R'      /* player 2 */
#define BLANK    '-'

#define MAXSIDE  5
#define MAXCELLS (MAXSIDE * MAXSIDE)
#define CHUNK    7        /* bits per gather/scatter table lookup */

typedef uint32_t BITS;    /* one bit per cell, row-major from the top left */

/* Board geometry for the variant in play. */
static int  gSide = 5;    /* N; the standard board, overridden by setOption */
static int  gCells;       /* N*N */
static int  gHashed;      /* N*N - N, the cells below the top row */
static int  gPow3;        /* 3^N, the number of top-row configurations */
static int  gPow3Of[MAXSIDE + 1];
static BITS gFull;        /* every cell */
static BITS gNotFirstCol; /* cells with a neighbour to the left */
static BITS gNotLastCol;  /* cells with a neighbour to the right */
static BITS gRowMask;     /* the top row */
static BITS gHashedMask;  /* the hashed cells, shifted down to bit 0 */

/* Tiers, numbered 0..gNumTiers-1 by tiles placed and then by top row. */
static int  gNumTiers;
static int *gTierOf;      /* [pieces * gPow3 + row0] -> tier, or -1 */
static int *gTierPieces;  /* [tier] -> tiles placed */
static int *gTierRow0;    /* [tier] -> top-row configuration, base 3 */
static int *gTierFreeBlue;/* [tier] -> blues below the top row */
static int *gTierFreeRed; /* [tier] -> reds below the top row */
static BITS *gTierRowBlue;/* [tier] -> the top row's blue cells */
static BITS *gTierRowRed; /* [tier] -> the top row's red cells */
static TIERPOSITION *gTierSize;

/* Ranking tables over the hashed cells. */
static uint32_t *gRankOf;     /* [set] -> rank among sets of its size */
static BITS     *gSetOfRank;  /* sets of each size, in rank order */
static uint32_t  gSetOffset[MAXCELLS + 2]; /* where each size starts in gSetOfRank */
static uint32_t  gChoose[MAXCELLS + 1][MAXCELLS + 1];

/* Gather and scatter through a mask, CHUNK bits at a time. */
static uint8_t gGather[1 << CHUNK][1 << CHUNK];  /* [mask][bits] -> compressed */
static uint8_t gScatter[1 << CHUNK][1 << CHUNK]; /* [mask][compressed] -> bits */
static uint8_t gCount[1 << CHUNK];

/* The last position decoded. The solver asks IsLegal, Primitive,
   GenerateMoves and then DoMove of each child about the same position in
   turn, so decoding it once serves all of them. */
static struct {
	BOOLEAN      valid;
	POSITION     position;
	TIER         windowTier;   /* identifies the hash window it was read in */
	POSITION     windowSize;
	BOOLEAN      windowed;
	int          tier;
	TIERPOSITION tierPosition;
	BITS         blue, red;    /* the whole board */
	BOOLEAN      legalKnown;
	BITS         legal;        /* cells the player to move may take */
} gLast;

/** Function prototypes **/
static void   BuildGeometry(void);
static void   BuildTables(void);
static void   BuildTiers(void);
static int    TierLookup(int pieces, int row0);
static void   Decode(POSITION position);
static POSITION Encode(BITS blue, BITS red);
static TIERPOSITION RankHashed(BITS blue20, BITS red20, int freeBlue, int freeRed);
static void   UnrankHashed(TIERPOSITION tp, int freeBlue, int freeRed, BITS *blue20, BITS *red20);
static BITS   LegalCells(BITS blue, BITS red);
static void   BoardOf(BITS blue, BITS red, char *board);
static BOOLEAN BitsOf(const char *board, BITS *blue, BITS *red);
static char  *customUnhash(POSITION position);
BOOLEAN       IsLegal(POSITION position);

/* TIER GAMESMAN */
void          SetupTierStuff(void);
TIERLIST     *TierChildren(TIER tier);
TIERPOSITION  NumberOfTierPositions(TIER tier);
void          GetInitialTierPosition(TIER *tier, TIERPOSITION *tierposition);
STRING        TierToString(TIER tier);
UNDOMOVELIST *GenerateUndoMovesToTier(POSITION position, TIER tier);
POSITION      UnDoMove(POSITION position, UNDOMOVE undomove);

BOOLEAN kSupportsSymmetries = FALSE;   /* see the note at the top of the file */

/*************************************************************************
**
** Bit helpers
**
**************************************************************************/

static inline int Popcount(BITS x) { return __builtin_popcount(x); }

static inline BITS Gather(BITS bits, BITS mask)
{
	BITS out = 0;
	int shift = 0, at;
	for (at = 0; at < gHashed; at += CHUNK) {
		BITS m = (mask >> at) & ((1u << CHUNK) - 1);
		out |= (BITS) gGather[m][(bits >> at) & ((1u << CHUNK) - 1)] << shift;
		shift += gCount[m];
	}
	return out;
}

static inline BITS Scatter(BITS packed, BITS mask)
{
	BITS out = 0;
	int at;
	for (at = 0; at < gHashed; at += CHUNK) {
		BITS m = (mask >> at) & ((1u << CHUNK) - 1);
		int n = gCount[m];
		out |= (BITS) gScatter[m][packed & ((1u << n) - 1)] << at;
		packed >>= n;
	}
	return out;
}

/* Each cell's number of neighbours in `tiles`, as three bit planes. */
static inline void NeighbourCounts(BITS tiles, BITS *c0, BITS *c1, BITS *c2)
{
	BITS dirs[4] = {
		(tiles << 1) & gNotFirstCol,      /* neighbour to the left  */
		(tiles >> 1) & gNotLastCol,       /* neighbour to the right */
		(tiles << gSide) & gFull,         /* neighbour above        */
		tiles >> gSide                    /* neighbour below        */
	};
	BITS b0 = 0, b1 = 0, b2 = 0;
	int i;
	for (i = 0; i < 4; i++) {
		BITS carry0 = b0 & dirs[i];
		b0 ^= dirs[i];
		BITS carry1 = b1 & carry0;
		b1 ^= carry0;
		b2 |= carry1;
	}
	*c0 = b0; *c1 = b1; *c2 = b2;
}

static inline BITS AnyNeighbour(BITS tiles)
{
	return ((tiles << 1) & gNotFirstCol) | ((tiles >> 1) & gNotLastCol)
	       | ((tiles << gSide) & gFull) | (tiles >> gSide);
}

/* Hadron's one rule, for every cell at once: an empty cell with as many
   friendly neighbours as enemy ones. A cell has at most 4 neighbours, so
   equal counts is exactly 0-0, 1-1 and 2-2. */
static BITS LegalCells(BITS blue, BITS red)
{
	BITS f0, f1, f2, e0, e1, e2;
	BOOLEAN blueToMove = Popcount(blue) == Popcount(red);
	NeighbourCounts(blueToMove ? blue : red, &f0, &f1, &f2);
	NeighbourCounts(blueToMove ? red : blue, &e0, &e1, &e2);
	return ~((f0 ^ e0) | (f1 ^ e1) | (f2 ^ e2)) & ~(blue | red) & gFull;
}

/*************************************************************************
**
** Setup
**
**************************************************************************/

static void BuildGeometry(void)
{
	int i, r, c;

	gCells  = gSide * gSide;
	gHashed = gCells - gSide;

	gPow3Of[0] = 1;
	for (i = 1; i <= gSide; i++)
		gPow3Of[i] = gPow3Of[i - 1] * 3;
	gPow3 = gPow3Of[gSide];

	gFull = (gCells == 32) ? ~0u : ((1u << gCells) - 1);
	gNotFirstCol = gNotLastCol = 0;
	for (r = 0; r < gSide; r++)
		for (c = 0; c < gSide; c++) {
			if (c > 0)         gNotFirstCol |= 1u << (r * gSide + c);
			if (c < gSide - 1) gNotLastCol  |= 1u << (r * gSide + c);
		}
	gRowMask = (1u << gSide) - 1;
	gHashedMask = (1u << gHashed) - 1;
}

static void BuildTables(void)
{
	static BOOLEAN chunksBuilt = FALSE;
	uint32_t sets = 1u << gHashed, s, k;
	uint32_t next[MAXCELLS + 2];
	int n, i;

	if (!chunksBuilt) {
		int m, x;
		for (m = 0; m < (1 << CHUNK); m++) {
			gCount[m] = (uint8_t) __builtin_popcount(m);
			for (x = 0; x < (1 << CHUNK); x++) {
				int g = 0, sc = 0, out = 0, in = 0, bit;
				for (bit = 0; bit < CHUNK; bit++)
					if (m >> bit & 1) {
						if (x >> bit & 1) g |= 1 << out;
						if (x >> in & 1) sc |= 1 << bit;
						out++; in++;
					}
				gGather[m][x] = (uint8_t) g;
				gScatter[m][x] = (uint8_t) sc;
			}
		}
		chunksBuilt = TRUE;
	}

	for (n = 0; n <= MAXCELLS; n++)
		for (i = 0; i <= MAXCELLS; i++)
			gChoose[n][i] = (i == 0) ? 1 : (n == 0) ? 0 : gChoose[n - 1][i - 1] + gChoose[n - 1][i];

	if (gRankOf != NULL) SafeFree(gRankOf);
	if (gSetOfRank != NULL) SafeFree(gSetOfRank);
	gRankOf = (uint32_t *) SafeMalloc(sets * sizeof(uint32_t));
	gSetOfRank = (BITS *) SafeMalloc(sets * sizeof(BITS));

	gSetOffset[0] = 0;
	for (k = 0; k <= (uint32_t) gHashed; k++)
		gSetOffset[k + 1] = gSetOffset[k] + gChoose[gHashed][k];
	for (k = 0; k <= (uint32_t) gHashed; k++)
		next[k] = 0;
	/* numeric order within each size is colex order */
	for (s = 0; s < sets; s++) {
		k = (uint32_t) Popcount(s);
		gRankOf[s] = next[k];
		gSetOfRank[gSetOffset[k] + next[k]] = s;
		next[k]++;
	}
}

static int TierLookup(int pieces, int row0)
{
	if (pieces < 0 || pieces > gCells || row0 < 0 || row0 >= gPow3)
		return -1;
	return gTierOf[pieces * gPow3 + row0];
}

/* Number the tiers, by tiles placed and then by top row. */
static void BuildTiers(void)
{
	int pieces, row0, tier, i;
	int table = (gCells + 1) * gPow3;

	if (gTierOf != NULL) {
		SafeFree(gTierOf); SafeFree(gTierPieces); SafeFree(gTierRow0);
		SafeFree(gTierFreeBlue); SafeFree(gTierFreeRed);
		SafeFree(gTierRowBlue); SafeFree(gTierRowRed); SafeFree(gTierSize);
	}

	gTierOf = (int *) SafeMalloc(table * sizeof(int));
	for (i = 0; i < table; i++)
		gTierOf[i] = -1;

	gNumTiers = 0;
	for (pieces = 0; pieces <= gCells; pieces++) {
		int blue = (pieces + 1) / 2, red = pieces / 2;
		for (row0 = 0; row0 < gPow3; row0++) {
			int b0 = 0, r0 = 0;
			for (i = 0; i < gSide; i++) {
				int digit = (row0 / gPow3Of[i]) % 3;
				if (digit == 1) b0++;
				else if (digit == 2) r0++;
			}
			if (blue - b0 < 0 || red - r0 < 0 || (blue - b0) + (red - r0) > gHashed)
				continue;
			gTierOf[pieces * gPow3 + row0] = gNumTiers++;
		}
	}

	gTierPieces   = (int *) SafeMalloc(gNumTiers * sizeof(int));
	gTierRow0     = (int *) SafeMalloc(gNumTiers * sizeof(int));
	gTierFreeBlue = (int *) SafeMalloc(gNumTiers * sizeof(int));
	gTierFreeRed  = (int *) SafeMalloc(gNumTiers * sizeof(int));
	gTierRowBlue  = (BITS *) SafeMalloc(gNumTiers * sizeof(BITS));
	gTierRowRed   = (BITS *) SafeMalloc(gNumTiers * sizeof(BITS));
	gTierSize     = (TIERPOSITION *) SafeMalloc(gNumTiers * sizeof(TIERPOSITION));

	for (pieces = 0; pieces <= gCells; pieces++)
		for (row0 = 0; row0 < gPow3; row0++) {
			BITS rb = 0, rr = 0;
			int fb, fr;
			tier = gTierOf[pieces * gPow3 + row0];
			if (tier < 0) continue;
			for (i = 0; i < gSide; i++) {
				int digit = (row0 / gPow3Of[i]) % 3;
				if (digit == 1) rb |= 1u << i;
				else if (digit == 2) rr |= 1u << i;
			}
			fb = (pieces + 1) / 2 - Popcount(rb);
			fr = pieces / 2 - Popcount(rr);
			gTierPieces[tier] = pieces;
			gTierRow0[tier] = row0;
			gTierRowBlue[tier] = rb;
			gTierRowRed[tier] = rr;
			gTierFreeBlue[tier] = fb;
			gTierFreeRed[tier] = fr;
			gTierSize[tier] = (TIERPOSITION) gChoose[gHashed][fb] * gChoose[gHashed - fb][fr];
		}
}

/*************************************************************************
**
** Hashing
**
**************************************************************************/

static TIERPOSITION RankHashed(BITS blue20, BITS red20, int freeBlue, int freeRed)
{
	BITS reds = Gather(red20, ~blue20 & gHashedMask);
	return (TIERPOSITION) gRankOf[blue20] * gChoose[gHashed - freeBlue][freeRed] + gRankOf[reds];
}

static void UnrankHashed(TIERPOSITION tp, int freeBlue, int freeRed, BITS *blue20, BITS *red20)
{
	uint32_t ways = gChoose[gHashed - freeBlue][freeRed];
	uint32_t t = (uint32_t) tp;                 /* tiers stay far below 2^32 */
	uint32_t blueRank = t / ways, redRank = t - blueRank * ways;
	BITS blue = gSetOfRank[gSetOffset[freeBlue] + blueRank];
	*blue20 = blue;
	*red20 = Scatter(gSetOfRank[gSetOffset[freeRed] + redRank], ~blue & gHashedMask);
}

/* Fill gLast with the board at `position` in the current hash window. */
static void Decode(POSITION position)
{
	TIERPOSITION tp;
	TIER tier;
	BITS blue20, red20;

	if (gLast.valid && gLast.position == position && gLast.windowed == gHashWindowInitialized
	    && (!gHashWindowInitialized
	        || (gLast.windowTier == gCurrentTier && gLast.windowSize == gNumberOfPositions)))
		return;

	if (gHashWindowInitialized) {
		gUnhashToTierPosition(position, &tp, &tier);
	} else {
		/* Only reached before any hash window exists, where the initial
		   tier is the only one that can be meant. */
		tier = gInitialTier;
		tp = position;
	}

	UnrankHashed(tp, gTierFreeBlue[tier], gTierFreeRed[tier], &blue20, &red20);
	gLast.valid = TRUE;
	gLast.position = position;
	gLast.windowed = gHashWindowInitialized;
	gLast.windowTier = gCurrentTier;
	gLast.windowSize = gNumberOfPositions;
	gLast.tier = (int) tier;
	gLast.tierPosition = tp;
	gLast.blue = gTierRowBlue[tier] | (blue20 << gSide);
	gLast.red = gTierRowRed[tier] | (red20 << gSide);
	gLast.legalKnown = FALSE;
}

static POSITION Encode(BITS blue, BITS red)
{
	int pieces = Popcount(blue | red);
	int row0 = 0, i;
	int tier;
	TIERPOSITION tp;

	for (i = 0; i < gSide; i++)
		row0 += ((blue >> i & 1) ? 1 : (red >> i & 1) ? 2 : 0) * gPow3Of[i];
	tier = TierLookup(pieces, row0);
	tp = RankHashed(blue >> gSide, red >> gSide, gTierFreeBlue[tier], gTierFreeRed[tier]);

	if (gHashWindowInitialized)
		return gHashToWindowPosition(tp, (TIER) tier);
	return tp;      /* outside a window, only the initial tier is meant */
}

static void BoardOf(BITS blue, BITS red, char *board)
{
	int i;
	for (i = 0; i < gCells; i++)
		board[i] = (blue >> i & 1) ? BLUE : (red >> i & 1) ? RED : BLANK;
}

/* Returns FALSE for a board no game produces: wrong tile counts. */
static BOOLEAN BitsOf(const char *board, BITS *blue, BITS *red)
{
	int i, nb, nr;
	*blue = *red = 0;
	for (i = 0; i < gCells; i++) {
		if (board[i] == BLUE) *blue |= 1u << i;
		else if (board[i] == RED) *red |= 1u << i;
	}
	nb = Popcount(*blue); nr = Popcount(*red);
	return nb == nr || nb == nr + 1;
}

static char *customUnhash(POSITION position)
{
	char *board = (char *) SafeMalloc(gCells * sizeof(char));
	Decode(position);
	BoardOf(gLast.blue, gLast.red, board);
	return board;
}

/*************************************************************************
**
** NAME:        InitializeGame
**
**************************************************************************/

void InitializeGame(void)
{
	/* Serving (--interact) reads each value from the tier files' 1 MB
	   gzipped chunks; only solving needs whole tiers in memory. */
	if (gIsInteract)
		gLoadTierdbArray = FALSE;

	BuildGeometry();
	BuildTables();
	BuildTiers();
	gLast.valid = FALSE;

	gCustomUnhash = &customUnhash;
	linearUnhash  = (void *) gCustomUnhash;

	SetupTierStuff();

	/* Deliberately no whole-board hash. One over all 25 cells is about
	   1.7 trillion positions on 5x5, and the database layer sizes its
	   allocation from gNumberOfPositions, so it would get the process killed
	   before solving starts. Tier play never needs it: every position lives
	   in a hash window, and gInitializeHashWindow resets gNumberOfPositions
	   for each one. Until then only the initial tier exists, so its size is
	   the bound; anything larger is an array allocated and filled for nothing
	   at the start of every solver process. */
	gNumberOfPositions = gTierSize[gInitialTier];
	gInitialPosition = gInitialTierPosition;
}

/*************************************************************************
**
** IsLegal
**
** Every tile, when placed, had as many friendly neighbours as enemy ones,
** and tiles never leave. Each property below follows from that, holds for
** the empty board, and survives any legal placement, so a solved position
** never has a child this rejects:
**
**   Balance. The board has as many adjacent same-colour pairs as adjacent
**     opposite-colour pairs. A placement adds f friendly and e enemy
**     adjacencies with f = e.
**   Same-colour pairs. Of two adjacent tiles of one colour, one has an enemy
**     neighbour: whichever came second saw a friend, so it saw an enemy too.
**   Opposite-colour pairs. Of two adjacent tiles of different colours, one
**     has a friendly neighbour, by the same argument.
**   Last tile. Some tile has as many friendly neighbours as enemy ones now:
**     the last one placed, whose neighbours all came before it.
**
** A new tile cannot spoil the pairs it joins, since it has both a friend and
** an enemy whenever it has any neighbour, and it is itself a candidate last
** tile. Old tiles only gain neighbours, which never breaks a pair property.
**
**************************************************************************/

static BOOLEAN LegalBoard(BITS blue, BITS red)
{
	BITS h, v, hasF, hasE, anyB, anyR, bad;
	BITS b0, b1, b2, r0, r1, r2;
	int same, diff;

	if ((blue | red) == 0)
		return TRUE;

	/* balance: count pairs across each cell's right and lower edge */
	h = gNotLastCol;
	same = Popcount(blue & (blue >> 1) & h) + Popcount(blue & (blue >> gSide))
	     + Popcount(red & (red >> 1) & h) + Popcount(red & (red >> gSide));
	diff = Popcount(blue & (red >> 1) & h) + Popcount(red & (blue >> 1) & h)
	     + Popcount(blue & (red >> gSide)) + Popcount(red & (blue >> gSide));
	if (same != diff)
		return FALSE;

	anyB = AnyNeighbour(blue);
	anyR = AnyNeighbour(red);
	hasF = (blue & anyB) | (red & anyR);
	hasE = (blue & anyR) | (red & anyB);

	/* same-colour pairs: bit i marks the pair (i, i+1) or (i, i+N) */
	h = ((blue & (blue >> 1)) | (red & (red >> 1))) & gNotLastCol;
	v = (blue & (blue >> gSide)) | (red & (red >> gSide));
	bad = (h & ~hasE & ~(hasE >> 1)) | (v & ~hasE & ~(hasE >> gSide));
	if (bad)
		return FALSE;

	/* opposite-colour pairs */
	h = ((blue & (red >> 1)) | (red & (blue >> 1))) & gNotLastCol;
	v = (blue & (red >> gSide)) | (red & (blue >> gSide));
	bad = (h & ~hasF & ~(hasF >> 1)) | (v & ~hasF & ~(hasF >> gSide));
	if (bad)
		return FALSE;

	/* last tile: equal neighbour counts is symmetric in the two colours */
	NeighbourCounts(blue, &b0, &b1, &b2);
	NeighbourCounts(red, &r0, &r1, &r2);
	return ((blue | red) & ~((b0 ^ r0) | (b1 ^ r1) | (b2 ^ r2))) != 0;
}

BOOLEAN IsLegal(POSITION position)
{
	Decode(position);
	return LegalBoard(gLast.blue, gLast.red);
}

/*************************************************************************
**
** NAME:        GenerateMoves / DoMove / Primitive
**
**************************************************************************/

static BITS LegalOfLast(void)
{
	if (!gLast.legalKnown) {
		gLast.legal = LegalCells(gLast.blue, gLast.red);
		gLast.legalKnown = TRUE;
	}
	return gLast.legal;
}

MOVELIST *GenerateMoves(POSITION position)
{
	MOVELIST *moves = NULL;
	BITS legal;

	Decode(position);
	legal = LegalOfLast();
	while (legal) {          /* highest first, so the list reads lowest first */
		int cell = 31 - __builtin_clz(legal);
		moves = CreateMovelistNode(cell, moves);
		legal &= ~(1u << cell);
	}
	return moves;
}

POSITION DoMove(POSITION position, MOVE move)
{
	BITS blue, red, cell = 1u << move;
	BOOLEAN blueToMove;
	int pieces, row0, child;
	TIERPOSITION tp;

	Decode(position);
	blue = gLast.blue; red = gLast.red;
	blueToMove = Popcount(blue) == Popcount(red);
	pieces = gTierPieces[gLast.tier] + 1;
	row0 = gTierRow0[gLast.tier];

	if (move < gSide) {
		/* into the top row: the hashed cells, and so the rank, are unchanged */
		row0 += (blueToMove ? 1 : 2) * gPow3Of[move];
		child = TierLookup(pieces, row0);
		tp = gLast.tierPosition;
	} else {
		if (blueToMove) blue |= cell; else red |= cell;
		child = TierLookup(pieces, row0);
		tp = RankHashed(blue >> gSide, red >> gSide, gTierFreeBlue[child], gTierFreeRed[child]);
	}

	if (gHashWindowInitialized)
		return gHashToWindowPosition(tp, (TIER) child);
	return tp;
}

/* A player with no legal placement has lost. Nothing else ends the game,
   and in particular it can end with empty cells still on the board. */
VALUE Primitive(POSITION position)
{
	Decode(position);
	if (LegalOfLast())
		return undecided;
	return gStandardGame ? lose : win;
}

/*************************************************************************
**
** Printing
**
**************************************************************************/

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn)
{
	char board[MAXCELLS];
	int row, col;
	BITS blue, red;

	Decode(position);
	blue = gLast.blue; red = gLast.red;
	BoardOf(blue, red, board);

	printf("\n    ");
	for (col = 0; col < gSide; col++)
		printf("%c ", 'A' + col);
	printf("\n");

	/* Row 1 is printed at the bottom, as on a chess board. Only the printing
	   is upside down: cell names, and so moves typed as A1 to E5, still
	   follow the cells' numbering. */
	for (row = gSide - 1; row >= 0; row--) {
		printf(" %d  ", row + 1);
		for (col = 0; col < gSide; col++)
			printf("%c ", board[row * gSide + col]);
		printf("\n");
	}

	printf("\n%s (%c) to move.  %s\n\n", playerName,
	       Popcount(blue) == Popcount(red) ? BLUE : RED,
	       GetPrediction(position, playerName, usersTurn));
}

void MoveToString(MOVE move, char *m)
{
	sprintf(m, "%c%d", (char) ('A' + move % gSide), move / gSide + 1);
}

void PrintMove(MOVE move)
{
	char m[8];
	MoveToString(move, m);
	printf("%s", m);
}

void PrintComputersMove(MOVE computersMove, STRING computersName)
{
	char m[8];
	MoveToString(computersMove, m);
	printf("%8s plays %s\n", computersName, m);
}

/*************************************************************************
**
** Text input
**
**************************************************************************/

BOOLEAN ValidTextInput(STRING input)
{
	char col, row;

	if (input == NULL || input[0] == '\0' || input[1] == '\0' || input[2] != '\0')
		return FALSE;

	col = input[0];
	row = input[1];
	if (col >= 'a' && col <= 'z') col = col - 'a' + 'A';

	return col >= 'A' && col < 'A' + gSide && row >= '1' && row < '1' + gSide;
}

MOVE ConvertTextInputToMove(STRING input)
{
	char col = input[0];
	int  row = input[1] - '1';

	if (col >= 'a' && col <= 'z') col = col - 'a' + 'A';
	return (MOVE) (row * gSide + (col - 'A'));
}

USERINPUT GetAndPrintPlayersMove(POSITION thePosition, MOVE *theMove, STRING playerName)
{
	USERINPUT input;

	for (;;) {
		printf("%8s's move (u)ndo/[A1-%c%d] : ", playerName,
		       (char) ('A' + gSide - 1), gSide);
		input = HandleDefaultTextInput(thePosition, theMove, playerName);
		if (input != Continue)
			return input;
	}
	return Continue;
}

/*************************************************************************
**
** Variants: the board size
**
**************************************************************************/

void GameSpecificMenu(void)
{
	char c;

	for (;;) {
		printf("\n\t----- Hadron options -----\n\n");
		printf("\t3)\t3x3 board\n");
		printf("\t4)\t4x4 board\n");
		printf("\t5)\t5x5 board (the standard game)\n");
		printf("\tb)\tBack to the main menu\n\n");
		printf("\tCurrently: %dx%d\n\n", gSide, gSide);
		printf("Select an option: ");

		c = GetMyChar();
		switch (c) {
		case '3': gSide = 3; InitializeGame(); break;
		case '4': gSide = 4; InitializeGame(); break;
		case '5': gSide = 5; InitializeGame(); break;
		case 'b': case 'B': return;
		default:  printf("\nSorry, I don't know that option.\n");
		}
	}
}

int NumberOfOptions(void)
{
	return 6;      /* 3 board sizes x standard/misere */
}

int getOption(void)
{
	return (gSide - 3) * 2 + (gStandardGame ? 0 : 1) + 1;
}

void setOption(int option)
{
	option -= 1;
	gStandardGame = (option % 2) == 0;
	gSide = 3 + option / 2;
}

void DebugMenu(void) { }

void SetTclCGameSpecificOptions(int theOptions[])
{
	(void) theOptions;
}

POSITION GetInitialPosition(void)
{
	return gInitialPosition;
}

/*************************************************************************
**
** TIER GAMESMAN
**
**************************************************************************/

void SetupTierStuff(void)
{
	kSupportsTierGamesman = TRUE;
	kExclusivelyTierGamesman = TRUE;

	gTierChildrenFunPtr            = &TierChildren;
	gNumberOfTierPositionsFunPtr   = &NumberOfTierPositions;
	gGetInitialTierPositionFunPtr  = &GetInitialTierPosition;
	gIsLegalFunPtr                 = &IsLegal;
	gGenerateUndoMovesToTierFunPtr = &GenerateUndoMovesToTier;
	gUnDoMoveFunPtr                = &UnDoMove;
	gTierToStringFunPtr            = &TierToString;

	gInitialTier = (TIER) TierLookup(0, 0);
	gInitialTierPosition = RankHashed(0, 0, 0, 0);
}

/* A move adds one tile, either below the top row (which leaves the top row
   alone) or into one of its empty cells (which does not). */
TIERLIST *TierChildren(TIER tier)
{
	TIERLIST *list = NULL;
	int pieces = gTierPieces[tier];
	int row0   = gTierRow0[tier];
	int colour = (pieces % 2 == 0) ? 1 : 2;      /* 1 blue, 2 red */
	int child, i;

	child = TierLookup(pieces + 1, row0);
	if (child >= 0)
		list = CreateTierlistNode((TIER) child, list);

	for (i = 0; i < gSide; i++) {
		if ((row0 / gPow3Of[i]) % 3 != 0)
			continue;
		child = TierLookup(pieces + 1, row0 + colour * gPow3Of[i]);
		if (child >= 0)
			list = CreateTierlistNode((TIER) child, list);
	}

	return list;
}

TIERPOSITION NumberOfTierPositions(TIER tier)
{
	return gTierSize[tier];
}

void GetInitialTierPosition(TIER *tier, TIERPOSITION *tierposition)
{
	*tier = gInitialTier;
	*tierposition = gInitialTierPosition;
}

STRING TierToString(TIER tier)
{
	char *name = (char *) SafeMalloc(64 * sizeof(char));
	char row[MAXSIDE + 1];
	int i;

	for (i = 0; i < gSide; i++)
		row[i] = (gTierRowBlue[tier] >> i & 1) ? BLUE : (gTierRowRed[tier] >> i & 1) ? RED : BLANK;
	row[gSide] = '\0';
	sprintf(name, "%d tile%s placed, top row %s",
	        gTierPieces[tier], gTierPieces[tier] == 1 ? "" : "s", row);
	return name;
}

/* The inverse of a move: lift the tile the previous player just placed.
   Only the undo moves that land in the requested tier are returned. The
   retrograde solver uses these only for loopy tiers, which Hadron has none
   of, but they are part of the tier API. */
UNDOMOVELIST *GenerateUndoMovesToTier(POSITION position, TIER tier)
{
	UNDOMOVELIST *list = NULL;
	BITS blue, red, previous, rest;
	int pieces, row0, cell;

	Decode(position);
	blue = gLast.blue; red = gLast.red;
	previous = (Popcount(blue) == Popcount(red)) ? red : blue;
	pieces = gTierPieces[gLast.tier];
	row0 = gTierRow0[gLast.tier];

	for (rest = previous; rest; rest &= rest - 1) {
		cell = __builtin_ctz(rest);
		int parentRow0 = row0;
		if (cell < gSide)
			parentRow0 -= ((previous == blue) ? 1 : 2) * gPow3Of[cell];
		if (TierLookup(pieces - 1, parentRow0) == (int) tier)
			list = CreateUndoMovelistNode(cell, list);
	}
	return list;
}

POSITION UnDoMove(POSITION position, UNDOMOVE undomove)
{
	BITS cell = 1u << undomove;
	Decode(position);
	return Encode(gLast.blue & ~cell, gLast.red & ~cell);
}

/*************************************************************************
**
** AutoGUI / UWAPI
**
** What the web front end and the server interface call. Blue is 'b' and
** Red is 'r' in these strings, matching the tiles the text interface
** prints.
**
**************************************************************************/

POSITION StringToPosition(char *positionString)
{
	char board[MAXCELLS];
	char *cells;
	int turn, i, tier, row0 = 0;
	BITS blue, red;

	if (!ParseStandardOnelinePositionString(positionString, &turn, &cells))
		return NULL_POSITION;

	for (i = 0; i < gCells; i++) {
		switch (cells[i]) {
		case 'b': case 'B': board[i] = BLUE;  break;
		case 'r': case 'R': board[i] = RED;   break;
		case '-':           board[i] = BLANK; break;
		default:            return NULL_POSITION;
		}
	}
	if (!BitsOf(board, &blue, &red))
		return NULL_POSITION;

	/* A position only has a number inside a hash window that contains its
	   tier, so move the window there before hashing, as the tier API
	   requires. A board no legal game produces has no tier and is rejected. */
	for (i = 0; i < gSide; i++)
		row0 += ((blue >> i & 1) ? 1 : (red >> i & 1) ? 2 : 0) * gPow3Of[i];
	tier = TierLookup(Popcount(blue | red), row0);
	if (tier < 0)
		return NULL_POSITION;
	gInitializeHashWindow((TIER) tier, FALSE);
	return Encode(blue, red);
}

void PositionToAutoGUIString(POSITION position, char *autoguiPositionStringBuffer)
{
	char cells[MAXCELLS + 1];
	int i;

	Decode(position);
	for (i = 0; i < gCells; i++)
		cells[i] = (gLast.blue >> i & 1) ? 'b' : (gLast.red >> i & 1) ? 'r' : '-';
	cells[gCells] = '\0';

	AutoGUIMakePositionString(Popcount(gLast.blue) == Popcount(gLast.red) ? 1 : 2,
	                          cells, autoguiPositionStringBuffer);
}

void MoveToAutoGUIString(POSITION position, MOVE move, char *autoguiMoveStringBuffer)
{
	/* A plain circle on the cell ('-': no image), coloured by the move's
	   value, as in any dartboard game. The sound says who placed: 'b' for
	   Blue, 'r' for Red, which the Hadron theme in GamesCraftersUWAPI maps
	   to two different sound files. */
	Decode(position);
	AutoGUIMakeMoveButtonStringA('-', (unsigned int) move,
	                             Popcount(gLast.blue) == Popcount(gLast.red) ? 'b' : 'r',
	                             autoguiMoveStringBuffer);
}
