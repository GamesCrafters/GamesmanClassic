/************************************************************************
**
** NAME:        mlaukatikata.c
**
** DESCRIPTION: Lau Kati Kata - Indian Checkers-style Strategy Game
**
** AUTHOR:      Ryan Lee, Zoe Zhang, Reed Yalouh
**
** DATE:        2025-10-06
**
************************************************************************/
#include "gamesman.h"
#include <ctype.h>
//#include "gameplay.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#define GetPrediction(p, ...) Primitive(p)

MOVELIST *GenerateMoves(POSITION pos);
void FreeMoveList(MOVELIST *ml);


// ---- Draw-stability types (paste near top) ----
typedef enum {
    V_WIN = 0, V_LOSE = 1, V_DW = 2, V_DL = 3  // V_DL means "some DLk"; see draw_level
} ValueExt;

typedef struct {
    ValueExt value;          // V_WIN / V_LOSE / V_DW / V_DL
    uint16_t ply_to_outcome; // normal remoteness for WIN/LOSE (optional for tie)
    uint16_t draw_level;     // k for DLk; UINT16_MAX for DW; 0/1/... valid only when value==V_DL
} Eval;

// ---- Minimal open-address hash from POSITION -> Eval (resizable) ----
typedef struct {
    POSITION key;
    Eval     val;
    int      used;
} DLSlot;

static DLSlot *gDL = NULL;
static size_t  gDLcap = 0;
static size_t  gDLcount = 0;

static uint64_t mix64(uint64_t x){
    x^=x>>33; x*=0xff51afd7ed558ccdULL; x^=x>>33; x*=0xc4ceb9fe1a85ec53ULL; return x^(x>>33);
}

__attribute__((unused)) static void DL_Reset(void){
    if (gDL){ free(gDL); gDL=NULL; gDLcap=0; gDLcount=0; }
}

static void DL_Rehash(size_t newcap) {
    DLSlot *old = gDL; size_t oldcap = gDLcap;
    gDL = (DLSlot*)calloc(newcap, sizeof(DLSlot));
    gDLcap = newcap; gDLcount = 0;
    size_t m = gDLcap - 1;
    for (size_t i = 0; i < oldcap; ++i) {
        if (!old[i].used) continue;
        POSITION k = old[i].key;
        size_t j = (size_t)mix64((uint64_t)k) & m;
        while (gDL[j].used) j = (j + 1) & m;
        gDL[j].used = 1; gDL[j].key = k; gDL[j].val = old[i].val; gDLcount++;
    }
    free(old);
}

static void DL_Ensure(size_t cap) {
    if (gDL) return;
    gDLcap = 1; while (gDLcap < cap) gDLcap <<= 1;
    gDL = (DLSlot*)calloc(gDLcap, sizeof(DLSlot));
    gDLcount = 0;
}

static Eval* DL_Get(POSITION k, int create) {
    if (!gDL) DL_Ensure(1<<16);
    if (create && gDLcount * 10 >= gDLcap * 7) DL_Rehash(gDLcap << 1);

    size_t m = gDLcap - 1;
    size_t i = (size_t)mix64((uint64_t)k) & m;
    for(;;){
        if (!gDL[i].used){
            if (!create) return NULL;
            gDL[i].used = 1; gDL[i].key = k; gDLcount++;
            gDL[i].val.value = V_DW;
            gDL[i].val.draw_level = UINT16_MAX;
            gDL[i].val.ply_to_outcome = 0;
            return &gDL[i].val;
        }
        if (gDL[i].key == k) return &gDL[i].val;
        i = (i+1) & m;
    }
}


// Small helpers
static inline const char* DL_Label(const Eval *e) {
    if (e->value == V_WIN)  return "WIN";
    if (e->value == V_LOSE) return "LOSE";
    if (e->value == V_DW)   return "DW";
    return "DL"; // use draw_level for exact print
}



// Game metadata
CONST_STRING kAuthorName = "Ryan Lee, Zoe Zhang, Reed Yalouh";
CONST_STRING kGameName = "Lau Kati Kata";  // Use this spacing and case
CONST_STRING kDBName = "laukatikata-v2";

POSITION gNumberOfPositions = 0; 
POSITION gInitialPosition = 0;
BOOLEAN kPartizan = TRUE;
BOOLEAN kTieIsPossible = TRUE;
BOOLEAN kLoopy = TRUE;  
BOOLEAN kSupportsSymmetries = FALSE; // to do later
POSITION kBadPosition = -1;
BOOLEAN kDebugDetermineValue = FALSE;

POSITION GetCanonicalPosition(POSITION);
void PositionToString(POSITION, char*);
BOOLEAN kGameSpecificMenu = FALSE;
BOOLEAN kDebugMenu = FALSE;

CONST_STRING kHelpGraphicInterface = "The graphical interface is not available yet for Lau Kati Kata.\n"
    "Use the text interface to play by typing moves such as '1-4' or '1x5x9'.";
CONST_STRING kHelpTextInterface =
    "During your turn, type a move in the form 'from-to' for a normal move,\n"
    "or 'from x mid x to' for a capturing jump. For example: 4-8 or 1x5x9.\n"
    "Type 'u' to undo, 'r' to redo, or 'q' to quit.";
CONST_STRING kHelpOnYourTurn =
    "Players take turns moving one of their pieces along the lines on the board.\n"
    "If a jump (capture) is possible, it must be made. Multiple jumps in one turn are allowed.";
CONST_STRING kHelpStandardObjective =
    "The goal is to capture all of your opponent's pieces, or to block them\n"
    "so they cannot move.";
CONST_STRING kHelpReverseObjective =
    "In the reverse version, the goal is to force your opponent to capture all of your pieces\n"
    "or to make yourself unable to move first.";
CONST_STRING kHelpTieOccursWhen =
    "A tie occurs when both players have the same number of pieces and no further captures\n"
    "or moves are possible.";
CONST_STRING kHelpExample =
    "Example moves:\n"
    "  1-4   (move from position 1 to 4)\n"
    "  1x5x9 (jump from 1 over 5 to 9, capturing the piece on 5)\n";

void *gGameSpecificTclInit = NULL;
void SetTclCGameSpecificOptions(int theOptions[]) { (void)theOptions; }
/*********** BEGIN SOLVING FUNCTIONS ***********/

/**
 * @brief Initial board state (player 1 starts as black)
 */
#define BOARD_LEN 14
#define BOARD_SIZE 13
#define win_for_player_one   win
#define win_for_player_two   lose  /* from current player's POV */

/* 13 board cells only; player-to-move is passed separately to the hash */
char initial_board[BOARD_SIZE] = {
    'B','B','B',
    'B','B','B',
    '-',            /* cell 10 is empty */
    'W','W','W',
    'W','W','W'
};


/* Jump must stay on the same straight line: from - mid - to.
   This lookup returns the unique landing square "to" when jumping over "mid" from "from";
   returns 0 if there is no straight continuation. Indexing is 1-based. */
static const int kOpposite[20][20] = {
/* 0  */ {0},
/* 1  */ { [2]=3,  [4]=7 },
/* 2  */ { [5]=7 },
/* 3  */ { [2]=1,  [6]=7 },
/* 4  */ { [5]=6,  [7]=10 },
/* 5  */ { [7]=9 },
/* 6  */ { [5]=4,  [7]=8 },
/* 7  */ { [4]=1, [5]=2,[6]=3 ,[8]=11, [9]=12, [10]=13 },   
/* 8  */ { [7]=6,[9]=10},            
/* 9  */ { [7]=5},   
/* 10 */ { [9]=8,  [7]=4},
/* 11 */ { [8]=7,  [12]=13 },  
/* 12 */ { [9]=7 },
/* 13 */ { [10]=7,  [12]=11},
};

/* Adjacency (step) neighbors, 1-based; each row terminated by 0 */
static const int neighbors[20][9] = {
    {}, {2,4,0}, {1,3,5,0}, {2,6,0}, {1,5,7,0}, {2,4,6,7,0},
    {3,5,7,0}, {4,5,6,8,9,10,0}, {7,9,11,0}, {7,8,10,12,0}, {7,9,13,0},
    {8,12,0}, {9,11,13,0}, {10,12,0}
};



/* ============================================================
   Helper: Check if the given player has any legal capturing move
   ============================================================ */
static inline int is_me_piece(char c, char meU)   { return toupper((unsigned char)c) == meU; }
static inline int is_opp_piece(char c, char oppU) { return toupper((unsigned char)c) == oppU; }

/* Any capture for a color anywhere on the board */
__attribute__((unused)) static int HasAnyCaptureFor(const char board[BOARD_SIZE], char meU, char oppU) {


    for (int from = 1; from <= BOARD_SIZE; ++from) {
        if (!is_me_piece(board[from - 1], meU)) continue;
        for (int i = 0; neighbors[from][i] != 0; ++i) {
            int mid = neighbors[from][i];
            int to  = kOpposite[from][mid];    /* colinear only */
            if (to == 0) continue;
            if (is_opp_piece(board[mid - 1], oppU) && board[to - 1] == '-') return 1;
        }
    }
    return 0;
}


/**
 * @brief Helper: recursively find jumps from a square and build full jump sequences
 * Enforces straight-line (colinear) captures using kOpposite.
 * Indexing for squares is 1..19; board[] is 0-based storage.
 */
/* One-ply capture generator: add each legal first jump from `from`. */


// ---- Multi-pass draw classification over the local draw component ----
typedef struct { POSITION p; } QItem;


// ---- Local visited set (pure C) used only inside the DL builder ----
// ---- Local visited set (pure C) used only inside the DL builder ----
typedef struct { POSITION key; int used; } VisSlot;

static VisSlot* vis = NULL;
static size_t viscap = 0;
static size_t viscount = 0;

static void vis_free(void){ if (vis){ free(vis); vis=NULL; viscap=0; viscount=0; } }

static void vis_rehash(size_t newcap) {
    VisSlot *old = vis; size_t oldcap = viscap;
    vis = (VisSlot*)calloc(newcap, sizeof(VisSlot));
    viscap = newcap;
    viscount = 0;
    size_t m = viscap - 1;
    for (size_t i = 0; i < oldcap; ++i) {
        if (!old[i].used) continue;
        POSITION k = old[i].key;
        size_t j = (size_t)mix64((uint64_t)k) & m;
        while (vis[j].used) j = (j + 1) & m;
        vis[j].used = 1; vis[j].key = k; viscount++;
    }
    free(old);
}

static void vis_init(size_t want) {
    vis_free();
    viscap = 1; while (viscap < want) viscap <<= 1;
    vis = (VisSlot*)calloc(viscap, sizeof(VisSlot));
    viscount = 0;
}

static int vis_has(POSITION k) {
    size_t m = viscap - 1, i = (size_t)mix64((uint64_t)k) & m;
    for (;;) {
        if (!vis[i].used) return 0;
        if (vis[i].key == k) return 1;
        i = (i + 1) & m;
    }
}

static void vis_add(POSITION k) {
    // Grow at ~70% load
    if (viscount * 10 >= viscap * 7) vis_rehash(viscap << 1);
    size_t m = viscap - 1, i = (size_t)mix64((uint64_t)k) & m;
    for (;;) {
        if (!vis[i].used) { vis[i].used = 1; vis[i].key = k; viscount++; return; }
        if (vis[i].key == k) return;
        i = (i + 1) & m;
    }
}


/* Build the list of draw (tie) nodes reachable from root, no adjacency stored. */
static POSITION* BuildDrawList(POSITION root, int *N_out) {
    *N_out = 0;
    if (GetPrediction(root) != tie) return NULL;

    // simple queue
    int qh = 0, qt = 0, qcap = 1024;
    POSITION *Q = (POSITION*)malloc(qcap * sizeof(POSITION));

    // result vector
    int ncap = 1024, n = 0;
    POSITION *nodes = (POSITION*)malloc(ncap * sizeof(POSITION));

    vis_init(1 << 16);           // temp visited set
    Q[qt++] = root; vis_add(root);

    while (qh < qt) {
        POSITION p = Q[qh++];

        // push into result
        if (n == ncap) { ncap <<= 1; nodes = (POSITION*)realloc(nodes, ncap * sizeof(POSITION)); }
        nodes[n++] = p;

        // expand only through tie children
        MOVELIST *ml = GenerateMoves(p);
        for (MOVELIST *it = ml; it; it = it->next) {
            POSITION c = DoMove(p, it->move);
            if (GetPrediction(c) == tie && !vis_has(c)) {
                if (qt == qcap) { qcap <<= 1; Q = (POSITION*)realloc(Q, qcap * sizeof(POSITION)); }
                Q[qt++] = c;
                vis_add(c);
            }
        }
        if (ml) FreeMoveList(ml);
    }

    free(Q);
    *N_out = n;
    return nodes;  // caller frees
}

/* “Child is opponent WIN” = child is win for the side-to-move at child and the turn flipped. */
__attribute__((unused))
static int child_is_opponent_WIN(POSITION parent, POSITION child){
    if (GetPrediction(child) != win) return 0;
    return generic_hash_turn(child) != generic_hash_turn(parent);
}

/* Multi-pass DL classification over the reachable draw component (no adjacency storage). */
static void ClassifyDrawLevelsFrom(POSITION root){
    int N = 0;
    POSITION *nodes = BuildDrawList(root, &N);
    if (!nodes || N == 0) { 
        if (nodes) free(nodes); 
        vis_free(); 
        return; 
    }

    // Initialize cached evals: assume DW; tighten below.
    for (int i = 0; i < N; ++i) {
        Eval *e = DL_Get(nodes[i], 1);
        e->value = V_DW;
        e->draw_level = UINT16_MAX;
        e->ply_to_outcome = 0;
    }

    // Pass A: DL0 fringe
    int marks = 0;
    for (int i = 0; i < N; ++i) {
        POSITION p = nodes[i];
        int hit = 0;
        MOVELIST *ml = GenerateMoves(p);
        for (MOVELIST *it = ml; it; it = it->next) {
            POSITION c = DoMove(p, it->move);
            if (GetPrediction(c) == win && generic_hash_turn(c) != generic_hash_turn(p)) { hit = 1; break; }
        }
        if (ml) FreeMoveList(ml);
        if (hit) {
            Eval *e = DL_Get(p, 1);
            e->value = V_DL;
            e->draw_level = 0;
            marks++;
        }
    }

    // Pass B: DLk for k >= 1
    for (uint16_t k = 1; marks > 0; ++k) {
        marks = 0;
        for (int i = 0; i < N; ++i) {
            Eval *e = DL_Get(nodes[i], 1);
            // only process nodes still unmarked (DW with UINT16_MAX level)
            if (!(e->value == V_DW && e->draw_level == UINT16_MAX)) continue;
            int hit = 0;
            MOVELIST *ml = GenerateMoves(nodes[i]);
            for (MOVELIST *it = ml; it; it = it->next) {
                POSITION c = DoMove(nodes[i], it->move);
                Eval *ce = DL_Get(c, 0);
                if (ce && ce->value == V_DL && ce->draw_level == (uint16_t)(k - 1)) { hit = 1; break; }
            }
            if (ml) FreeMoveList(ml);
            if (hit) {
                e->value = V_DL;
                e->draw_level = k;
                marks++;
            }
        }
    }

    free(nodes);
    vis_free();   // IMPORTANT: free temp visited set
}





void InitializeGame(void) {
    gCanonicalPosition = GetCanonicalPosition;

    // Bounded counts to keep the state space sane.
    int pieceSpec[] = {
        'B', 0, 6,             // up to 6 black
        'W', 0, 6,             // up to 6 white
        'b', 0, 1,             // at most 1 locked black
        'w', 0, 1,             // at most 1 locked white
        '-', 1, BOARD_SIZE-1,  // at least 1 empty, up to 12
        -1
    };



    generic_hash_destroy();
    gNumberOfPositions = generic_hash_init(BOARD_SIZE, pieceSpec, NULL, 0);

    /* 1 = Black to move first; change to 2 if White should start */
    int initialPlayer = 1;
    gInitialPosition = generic_hash_hash(initial_board, initialPlayer);

    printf("[LKK] build ok. neighbors[9]:");    for (int i=0; neighbors[9][i]; ++i)  printf(" %d", neighbors[9][i]);  puts("");
printf("[LKK] neighbors[10]:");              for (int i=0; neighbors[10][i]; ++i) printf(" %d", neighbors[10][i]); puts("");

printf("[LKK] %s %s\n", __DATE__, __TIME__);
printf("[LKK] n[9]:");  for (int i=0; neighbors[9][i];  ++i) printf(" %d", neighbors[9][i]);  puts("");
printf("[LKK] n[10]:"); for (int i=0; neighbors[10][i]; ++i) printf(" %d", neighbors[10][i]); puts("");


}

// (Optional) forward declare if GenerateJumpsFrom is below:

static void GenerateJumpsFrom(int from,
                              const char board[BOARD_SIZE],
                              char meU, char oppU,
                              MOVELIST **jumpList)
{
    (void)meU; // not needed here

    if (from < 1 || from > BOARD_SIZE) return;

    for (int n = 0; neighbors[from][n] != 0; ++n) {
        int mid = neighbors[from][n];
        if (mid < 1 || mid > BOARD_SIZE) continue;

        int to = kOpposite[from][mid];   // force colinearity
        if (to < 1 || to > BOARD_SIZE) continue;

        if (toupper((unsigned char)board[mid - 1]) == oppU && board[to - 1] == '-') {
            MOVE mv = from * 10000 + mid * 100 + to;  // from x mid x to
            *jumpList = CreateMovelistNode(mv, *jumpList);
        }
    }
}





/* Can the piece at `from` continue capturing? (colinear rule) */
static int HasAnyCaptureFrom(const char board[BOARD_SIZE], int from, char oppU) {

    for (int i = 0; neighbors[from][i] != 0; ++i) {
        int mid = neighbors[from][i];
        int to  = kOpposite[from][mid];
        if (to == 0) continue;
        if (is_opp_piece(board[mid - 1], oppU) && board[to - 1] == '-') return 1;
    }
    return 0;
}



/**
 * @brief Apply a move and return the resulting position
 */
POSITION DoMove(POSITION position, MOVE move) {
    char board[BOARD_SIZE];
    generic_hash_unhash(position, board);
    int turn = generic_hash_turn(position);  /* 1 = Black, 2 = White */

    char meU  = (turn == 1 ? 'B' : 'W');
    char oppU = (turn == 1 ? 'W' : 'B');

    if (move > 9999) {  /* jump: a x m x b  (we'll keep going if forced) */
        int from = move / 10000;
        int mid  = (move / 100) % 100;
        int to   = move % 100;

        board[from - 1] = '-';
        board[mid  - 1] = '-';
        board[to   - 1] = meU;

        /* Keep auto-jumping while there is exactly one forced continuation. */
        for (;;) {
            // build one-ply jumps from current 'to'
            MOVELIST *jl = NULL;
            // reuse your helper
            GenerateJumpsFrom(to, board, meU, oppU, &jl);

            if (!jl) break;                 // no more jumps: chain ends
            if (jl->next) {                 // multiple choices => stop here (let UI choose)
                FreeMoveList(jl);
                board[to - 1] = (turn == 1 ? 'b' : 'w');  // lock the piece
                return generic_hash_hash(board, turn);    // keep jumping
            }

            // exactly one forced jump; apply it and continue
            MOVE cont = jl->move; FreeMoveList(jl);
            int mid2 = (cont / 100) % 100;
            int to2  = cont % 100;

            board[mid2 - 1] = '-';
            board[to2  - 1] = meU;
            board[to   - 1] = '-';
            to = to2;
        }
    } else {          /* step: a-b */
        int from = move / 100;
        int to   = move % 100;
        board[to - 1]   = meU;
        board[from - 1] = '-';
    }
    for (int i = 0; i < BOARD_SIZE; ++i) {  //clear the locked piece    
    if (board[i] == (turn == 1 ? 'b' : 'w'))
        board[i] = (turn == 1 ? 'B' : 'W');}

    int nextTurn = (turn == 1 ? 2 : 1);
    return generic_hash_hash(board, nextTurn);
}
// ---- Legal move generator (captures preferred; locked piece forces continuation)
MOVELIST *GenerateMoves(POSITION pos) {
    char board[BOARD_SIZE];
    generic_hash_unhash(pos, board);
    int turn = generic_hash_turn(pos);   // 1=Black, 2=White

    char meU    = (turn == 1 ? 'B' : 'W');
    char meLock = (turn == 1 ? 'b' : 'w');
    char oppU   = (turn == 1 ? 'W' : 'B');

    MOVELIST *list = NULL;
    // If we’re in the middle of a capture chain, only that locked piece moves (must jump)
    int locked_from = 0;
    for (int i = 1; i <= BOARD_SIZE; ++i) {
        if (board[i - 1] == meLock) { locked_from = i; break; }
    }
    if (locked_from) {
        GenerateJumpsFrom(locked_from, board, meU, oppU, &list);
        return list; // no steps allowed while locked
    }

    // Mandatory capture: if any capture exists for any piece, only generate jumps
    int anyCap = 0;
    for (int from = 1; from <= BOARD_SIZE; ++from) {
        if (toupper((unsigned char)board[from - 1]) != meU) continue;
        MOVELIST *before = list;
        GenerateJumpsFrom(from, board, meU, oppU, &list);
        if (list != before) anyCap = 1;
    }
    if (anyCap) return list;

    // Otherwise, generate simple step moves to any adjacent empty neighbor
    for (int from = 1; from <= BOARD_SIZE; ++from) {
        if (toupper((unsigned char)board[from - 1]) != meU) continue;
        for (int i = 0; neighbors[from][i] != 0; ++i) {
            int to = neighbors[from][i];
            if (board[to - 1] == '-') {
                MOVE mv = from * 100 + to;
                list = CreateMovelistNode(mv, list);
            }
        }
    }
    return list;
}


VALUE Primitive(POSITION position) {
    char board[BOARD_SIZE];
    generic_hash_unhash(position, board);

    int turn = generic_hash_turn(position);   /* 1 = Black, 2 = White */
    int bCnt = 0, wCnt = 0;
    for (int i = 0; i < BOARD_SIZE; ++i) {
        char u = toupper((unsigned char)board[i]);
        if (u == 'B') ++bCnt;
        else if (u == 'W') ++wCnt;
    }

    /* (1) Capture of all opponent pieces wins */
    if (bCnt == 0) return (turn == 1 ? lose : win);
    if (wCnt == 0) return (turn == 2 ? lose : win);

    /* (2) Check if both sides can move (not just capture) */
    MOVELIST *moves_black = NULL;
    MOVELIST *moves_white = NULL;

    POSITION pB = generic_hash_hash(board, 1);
    POSITION pW = generic_hash_hash(board, 2);

    moves_black = GenerateMoves(pB);
    moves_white = GenerateMoves(pW);

    int blackHas = (moves_black != NULL);
    int whiteHas = (moves_white != NULL);

    if (moves_black) FreeMoveList(moves_black);
    if (moves_white) FreeMoveList(moves_white);

    /* (3) If both sides cannot move → tiebreak by material */
    if (!blackHas && !whiteHas) {
        if (bCnt > wCnt) return (turn == 1 ? win : lose);
        if (wCnt > bCnt) return (turn == 2 ? win : lose);
        return tie;
    }

    /* (4) If side to move cannot move → lose */
    if (turn == 1 && !blackHas) return lose;
    if (turn == 2 && !whiteHas) return lose;

    return undecided;
}








/**
 * @brief Canonical position (no symmetry used).
 */
POSITION GetCanonicalPosition(POSITION position) {
    return position;
}


/**
 * @brief Dummy implementation of GenerateParents (required for loopy games).
 */
void GenerateParents(POSITION position, MOVE **moveList) {
    (void)position;
    // IMPORTANT: always initialize so the solver never walks garbage.
    if (moveList) *moveList = NULL;

    // TODO: implement true reverse move generation for full loopy solving.
}




/*********** END SOLVING FUNCTIONS ***********/







/* ===================== TUI HELPERS (PRINT & PARSE) ===================== */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

/* Helper: convert board cell to printable char */
static inline char CellChar(char c) {
    if (c == 'B' || c == 'W' || c == 'b' || c == 'w') return c;
    return '.'; /* empty */
}


/* Helper: get board[14] from position */
static void LoadBoard(POSITION position, char board[14]) {
    char raw[BOARD_SIZE];
    generic_hash_unhash(position, raw);
    board[0] = 0;
    for (int i = 0; i < BOARD_SIZE; i++)
        board[i + 1] = raw[i];
}


/* Pretty print the board (with labels 1..19 on the left, pieces on the right) */
void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn) {
    (void)playerName; (void)usersTurn;

    char b[14]; LoadBoard(position, b);
    /* indexes 1..13 hold cells */
    char c1=CellChar(b[1]),  c2=CellChar(b[2]),  c3=CellChar(b[3]);
    char c4=CellChar(b[4]),  c5=CellChar(b[5]),  c6=CellChar(b[6]);
    char c7=CellChar(b[7]),  c8=CellChar(b[8]),  c9=CellChar(b[9]);
    char c10=CellChar(b[10]),c11=CellChar(b[11]),c12=CellChar(b[12]),c13=CellChar(b[13]);

    printf("\n");
    printf("   1-----2-----3        :     %c-----%c-----%c\n", c1, c2, c3);
    printf("    \\    |    /         :      \\    |    /\n");
    printf("     4---5---6          :       %c---%c---%c\n", c4, c5, c6);
    printf("      \\  |  /           :        \\  |  /\n");
    printf("         7              :           %c\n", c7);
    printf("      /  |  \\           :        /  |  \\\n");
    printf("     8---9---10         :       %c---%c---%c\n", c8, c9, c10);
    printf("    /    |    \\         :      /    |    \\\n");
    printf("  11-----12-----13      :     %c-----%c-----%c\n", c11, c12, c13);


    /* Optional: show prediction if desired
       (See GetPrediction in core/gameplay.h) */
    // PrintPrediction(position);

    // ---- Add after the big board ascii ----
{
    VALUE pv = GetPrediction(position);
    if (pv == win) {
        printf("  Position: WIN\n\n");
    } else if (pv == lose) {
        printf("  Position: LOSE\n\n");
    } else if (pv == tie) {
    // Use cached DL if available; otherwise classify once for this component.
    Eval *e = DL_Get(position, 0);
    if (!e || (e->value != V_DL && e->value != V_DW)) {
        ClassifyDrawLevelsFrom(position);
        e = DL_Get(position, 1);
    } 
    if (e->value == V_DW) {
        printf("  Position: DW (safe draw)\n\n");
    } else if (e->value == V_DL) {
        printf("  Position: DL%u\n\n", (unsigned)e->draw_level);
    } else {
        printf("  Position: (unexpected)\n\n");
    }

    } else {
        printf("  Position: (DL: N/A — unsolved)\n\n");
    }
}

MOVELIST *ml = GenerateMoves(position);
printf("[LKK] moves: ");
for (MOVELIST *p=ml; p; p=p->next) { char s[MAX_MOVE_STRING_LENGTH]; MoveToString(p->move, s); printf("[%s] ", s); }
puts(""); if (ml) FreeMoveList(ml);


}

/* TextUI prompt – we reuse the default input handler for Undo/Abort/Help, etc. */
USERINPUT GetAndPrintPlayersMove(POSITION position, MOVE *move, STRING playerName) {
    USERINPUT ret;
    do {
        printf("%8s's move (e.g., 1-4 or 1x5x9): ", playerName);
        ret = HandleDefaultTextInput(position, move, playerName);
        if (ret != Continue) return ret;
    } while (TRUE);
}

/* ------------------ Parsing support ------------------ */

/*
Accepted syntax (whitespace ignored):
  - Simple step: "a-b"  (from-to)
  - Single jump: "a x m x b"  (from-captured-to)
Where a,b,m are integers in [1,19].
We don’t validate legality here (only syntax); Gamesman will check move legality.
*/

static int parse_int_1_13(const char **p) {
    while (isspace((unsigned char)**p)) (*p)++;
    if (!isdigit((unsigned char)**p)) return -1;
    long v = 0;
    while (isdigit((unsigned char)**p)) { v = v*10 + (**p - '0'); (*p)++; }
    if (v < 1 || v > 13) return -1;
    return (int)v;
}

static int consume_char(const char **p, char c1, char c2) {
    while (isspace((unsigned char)**p)) (*p)++;
    char c = **p;
    if (c == c1 || c == c2) { (*p)++; return 1; }
    return 0;
}

/* Is the raw text input “shaped” like a move? (syntax only) */
BOOLEAN ValidTextInput(STRING input) {
    if (input == NULL) return FALSE;
    const char *p = input;

    int a = parse_int_1_13(&p); if (a == -1) return FALSE;

    /* either '-' for step, or 'x' for capture */
    if (consume_char(&p, '-', '-')) {
        int b = parse_int_1_13(&p); if (b == -1) return FALSE;
        while (isspace((unsigned char)*p)) p++;
        return (*p == '\0'); /* only "a-b" */
    } else if (consume_char(&p, 'x', 'X')) {
        int m = parse_int_1_13(&p); if (m == -1) return FALSE;
        if (!consume_char(&p, 'x', 'X')) return FALSE;
        int b = parse_int_1_13(&p); if (b == -1) return FALSE;
        while (isspace((unsigned char)*p)) p++;
        return (*p == '\0'); /* only "a x m x b" */
    }

    return FALSE;
}

/* Convert text to internal MOVE encoding used in your GenerateMoves/ */
MOVE ConvertTextInputToMove(STRING input) {
    const char *p = input;
    int a = parse_int_1_13(&p);
    if (a == -1) return 0;

    if (consume_char(&p, '-', '-')) {
        int b = parse_int_1_13(&p); if (b == -1) return 0;
        return (MOVE)(a * 100 + b); /* step: from*100 + to */
    } else if (consume_char(&p, 'x', 'X')) {
        int m = parse_int_1_13(&p); if (m == -1) return 0;
        if (!consume_char(&p, 'x', 'X')) return 0;
        int b = parse_int_1_13(&p); if (b == -1) return 0;
        return (MOVE)(a * 10000 + m * 100 + b); /* jump: from*10000 + mid*100 + to */
    }

    return 0;
}

/* Human-readable move string */
void MoveToString(MOVE move, char *moveStringBuffer) {
    /* MAX_MOVE_STRING_LENGTH exists in core; keep output short and safe */
    if (move > 9999) {
        int from = (int)(move / 10000);
        int mid  = (int)((move / 100) % 100);
        int to   = (int)(move % 100);
        snprintf(moveStringBuffer, MAX_MOVE_STRING_LENGTH, "%d x %d x %d", from, mid, to);
    } else {
        int from = (int)(move / 100);
        int to   = (int)(move % 100);
        snprintf(moveStringBuffer, MAX_MOVE_STRING_LENGTH, "%d-%d", from, to);
    }
}

/* Print the computer’s move nicely */
void PrintComputersMove(MOVE computersMove, STRING computersName) {
    char buf[MAX_MOVE_STRING_LENGTH];
    MoveToString(computersMove, buf);
    printf("%s's move: %s\n", computersName, buf);
}
void DebugMenu(void) {
    printf("\n=== Lau Kati Kata Instructions ===\n");
    printf("Board: 13 connected points forming a cross-like structure.\n");
    printf("Pieces: 6 Black pieces vs 6 White pieces.\n");
    printf("Rules:\n");
    printf("1. Players move pieces along edges.\n");
    printf("2. Captures are mandatory when available.\n");
    printf("3. Captures must be colinear: from - mid - to.\n");
    printf("4. Captured piece is removed instantly.\n");
    printf("5. If multiple capture continuations exist, the piece is locked until resolved.\n");
    printf("To Win: Capture all opponent pieces or leave opponent without moves.\n");
}



/* Keep these as no-ops unless you want string/AutoGUI support now */
//void PositionToString(POSITION position, char *positionStringBuffer) { (void)position; (void)positionStringBuffer; }
//POSITION StringToPosition(char *positionString) { (void)positionString; return NULL_POSITION; }
//void PositionToAutoGUIString(POSITION position, char *autoguiPositionStringBuffer) { (void)position; (void)autoguiPositionStringBuffer; }
//void MoveToAutoGUIString(POSITION position, MOVE move, char *autoguiMoveStringBuffer) { (void)position; (void)move; (void)autoguiMoveStringBuffer; }

/* ===================== END TUI HELPERS ===================== */




/*********** BEGIN VARIANT FUNCTIONS ***********/

/**
 * @return The total number of variants supported.
 */
int NumberOfOptions(void) {
    return 1;
}

/**
 * @return The current variant ID.
 */
int getOption(void) {
    return 0;
}

/**
 * @brief Set any global variables or data structures according to the
 * variant specified by the input variant ID.
 * 
 * @param option An ID specifying the variant that we want to change to.
 */
void setOption(int option) {
    (void)option;
}

/**
 * @brief Interactive menu used to change the variant, i.e., change
 * game-specific parameters, such as the side-length of a tic-tac-toe
 * board, for example. Does nothing if kGameSpecificMenu == FALSE.
 */
void GameSpecificMenu(void) {}

/*********** END VARIANT-RELATED FUNCTIONS ***********/





/* ------------------ Global board layout definitions for AutoGUI ------------------ */
/* Normalized X coordinates for nodes (1–13) */
const double kNodeX[14] = {
    0.0,   // dummy
    0.10, 0.50, 0.90,    // 1–3
    0.25, 0.50, 0.75,    // 4–6
    0.50,                // 7
    0.25, 0.50, 0.75,    // 8–10
    0.10, 0.50, 0.90     // 11–13
};

/* Normalized Y coordinates for nodes (1–13) */
const double kNodeY[14] = {
    0.0,
    0.05, 0.05, 0.05,    // top row
    0.20, 0.20, 0.20,    // 2nd row
    0.35,                // 7
    0.55, 0.55, 0.55,    // 8–10
    0.75, 0.75, 0.75     // 11–13
};

/* Board edges: each pair {a,b} draws a line between nodes a and b */
const int kEdges[][2] = {
    {1,2},{2,3},           // top row
    {1,4},{2,5},{3,6},     // diagonals
    {4,5},{5,6},           // 2nd row
    {4,7},{5,7},{6,7},     // down to 7
    {7,8},{7,9},{7,10},    // down to next row
    {8,9},{9,10},          // horizontal
    {8,11},{9,12},{10,13}, // diagonals down
    {11,12},{12,13},       // bottom row
    {0,0}
};





/**
 * @brief Convert the input position to a human-readable formal
 * position string. It must be in STANDARD ONELINE POSITION 
 * STRING FORMAT, i.e. it is formatted as <turn>_<board>, where...
 *     `turn` is a character. ('1' if it's Player 1's turn or 
 *         '2' if it's Player 2's turn; or '0' if turn is not
 *         encoded in the position because the game is impartial.)
 *     `board` is a string of characters that does NOT contain 
 *         colons or semicolons.
 * 
 * @param position The position for which to generate the formal 
 * position string.
 * @param positionStringBuffer The buffer to write the position string to.
 * 
 * @note The space available in `positionStringBuffer` is 
 * MAX_POSITION_STRING_LENGTH (see src/core/autoguistrings.h). Do not write
 * past this limit and ensure that the position string written to 
 * `positionStringBuffer` is properly null-terminated.
 * 
 * @note You need not implement this function if you wish for the
 * AutoGUI Position String to be the same as the Human-Readable Formal
 * Position String. You can in fact delete this function and leave
 * gStringToPositionFunPtr as NULL in InitializeGame().
 */
void PositionToString(POSITION position, char *positionStringBuffer) {
    char board[BOARD_SIZE];
    generic_hash_unhash(position, board);
    int turn = generic_hash_turn(position);

    snprintf(positionStringBuffer, MAX_POSITION_STRING_LENGTH, "%d_", turn);
    for (int i = 0; i < BOARD_SIZE; ++i) {
        char temp[2] = { board[i], '\0' };
        strncat(positionStringBuffer, temp, 1);
    }
}

/**
 * @brief Convert the input position string to
 * the in-game integer representation of the position.
 * 
 * @param positionString The position string. This is guaranteed
 * to be shorter than MAX_POSITION_STRING_LENGTH (see src/core/autoguistrings.h).
 * 
 * @return If the position string is valid, return the corresponding 
 * position hash. Otherwise, return NULL_POSITION.
 * 
 * @note DO NOT MODIFY any characters in `positionString`, or, at least,
 * ensure that at the conclusion of this function, the characters
 * are the same as before.
 */
POSITION StringToPosition(char *positionString) {
    if (!positionString || strlen(positionString) < 3) return NULL_POSITION;

    int turn = positionString[0] - '0';
    const char *boardStr = positionString + 2;

    char board[BOARD_SIZE];
    int len = strlen(boardStr);
    for (int i = 0; i < BOARD_SIZE; ++i)
        board[i] = (i < len ? boardStr[i] : '-');

    return generic_hash_hash(board, turn);
}


/**
 * @brief Write an AutoGUI-formatted position string for the given position 
 * (which tells the frontend application how to render the position) to the
 * input buffer.
 * 
 * @param position The position for which to generate the AutoGUI 
 * position string.
 * @param autoguiPositionStringBuffer The buffer to write the AutoGUI
 * position string to.
 * 
 * @note You may find AutoGUIMakePositionString() helpful. 
 * (See src/core/autoguistrings.h)
 * 
 * @note The space available in `autoguiPositionStringBuffer` is 
 * MAX_POSITION_STRING_LENGTH (see src/core/autoguistrings.h). Do not write
 * past this limit and ensure that the position string written to 
 * `autoguiPositionStringBuffer` is properly null-terminated.
 * AutoGUIMakePositionString() should handle the null-terminator, 
 * if you choose to use it.
 * 
 * @note If the game is impartial and a turn is not encoded, set the turn
 * character (which is the first character) of autoguiPositionStringBuffer
 * to '0'.
 */
void PositionToAutoGUIString(POSITION position, char *autoguiPositionStringBuffer) {
    char board[BOARD_SIZE + 1];
    generic_hash_unhash(position, board);
    board[BOARD_SIZE] = '\0';

    // Create AutoGUI string for the front-end
    AutoGUIMakePositionString(generic_hash_turn(position), board, autoguiPositionStringBuffer);
}


/**
 * @brief Write an AutoGUI-formatted move string for the given move 
 * (which tells the frontend application how to render the move as button) to the
 * input buffer.
 * 
 * @param position   : The position from which the move is made. This is helpful
 * if the move button depends on not only the move hash but also the position.
 * @param move : The move hash from which the AutoGUI move string is generated.
 * @param autoguiMoveStringBuffer : The buffer to write the AutoGUI
 * move string to.
 * 
 * @note The space available in `autoguiMoveStringBuffer` is MAX_MOVE_STRING_LENGTH 
 * (see src/core/autoguistrings.h). Do not write past this limit and ensure that
 * the move string written to `moveStringBuffer` is properly null-terminated.
 * 
 * @note You may find the "AutoGUIMakeMoveButton" functions helpful.
 * (See src/core/autoguistrings.h)
 */
__attribute__((unused))
void MoveToAutoGUIString(POSITION position, MOVE move, char *autoguiMoveStringBuffer) {
    (void)position;
    if (move > 9999) {  // jump move: from x mid x to
        int from = move / 10000;
        int to   = move % 100;
        AutoGUIMakeMoveButtonStringM(from - 1, to - 1, 'x', autoguiMoveStringBuffer);
    } else {            // step move: from-to
        int from = move / 100;
        int to   = move % 100;
        AutoGUIMakeMoveButtonStringM(from - 1, to - 1, '-', autoguiMoveStringBuffer);
    }
}

#include "gamesman.h"

// Compatibility wrapper for new GamesmanClassic GUI
void GS_InitGameSpecific(void) {
    // Just call your old initializer
    InitializeGame();
}

//#include <tcl.h>

int Mlaukatikata_Init(Tcl_Interp *interp) {
    (void)interp;
    GS_InitGameSpecific();
    return TCL_OK;
}