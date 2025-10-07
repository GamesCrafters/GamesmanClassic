/************************************************************************
**
** NAME:        mconnect4twist.c
**
** DESCRIPTION: Connect-4 Twist & Turn (5×6 board)
**
** AUTHOR:      Enzo Ribeiro, Weiyi Zhang
**
** DATE:        2025-10-05
**
************************************************************************/

#include "gamesman.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/*----------------------------------------------------------------------
 *  Game metadata
 *--------------------------------------------------------------------*/
CONST_STRING kAuthorName = "Enzo Ribeiro & Weiyi Zhang";
CONST_STRING kGameName   = "Connect-4 Twist & Turn";
CONST_STRING kDBName     = "connect4twist";

BOOLEAN kPartizan           = TRUE;
BOOLEAN kTieIsPossible      = TRUE;
BOOLEAN kLoopy              = FALSE; 
BOOLEAN kSupportsSymmetries = FALSE;
POSITION kBadPosition       = -1;
BOOLEAN kDebugDetermineValue= FALSE;

POSITION gNumberOfPositions = 0;
POSITION gInitialPosition   = 0;

BOOLEAN kGameSpecificMenu   = FALSE;
BOOLEAN kDebugMenu          = FALSE;

CONST_STRING kHelpGraphicInterface  = "";
CONST_STRING kHelpTextInterface     = "";
CONST_STRING kHelpOnYourTurn        = "Drop a piece in a column, then optionally twist one non-empty row left or right.";
CONST_STRING kHelpStandardObjective = "Make 4 in a row (horizontal, vertical, or diagonal).";
CONST_STRING kHelpReverseObjective  = "";
CONST_STRING kHelpTieOccursWhen     = "The board is full and nobody has 4 in a row.";
CONST_STRING kHelpExample           =
"Examples:\n"
"  3 N     -> drop in column 3, no twist\n"
"  4 L 2   -> drop in column 4, then twist row 2 left\n"
"  6 R 5   -> drop in column 6, then twist row 5 right\n";
/*----------------------------------------------------------------------
 *  Board constants and global board
 *--------------------------------------------------------------------*/
enum { ROWS = 4, COLS = 5, CELLS = ROWS * COLS };

/*----------------------------------------------------------------------
 *  Move encoding
 *      bits 0-7   : drop column (0..5)
 *      bits 8-15  : twist row  (0..4) — ignored if direction == NONE
 *      bits 16-17 : direction   (0 = NONE, 1 = LEFT, 2 = RIGHT)
 *--------------------------------------------------------------------*/
typedef enum { DIR_NONE=0, DIR_LEFT=1, DIR_RIGHT=2 } TwistDir;

#define MOVE_MAKE(col,row,dir)   ((MOVE)(((col)&0xFF) | (((row)&0xFF)<<8) | (((dir)&0x3)<<16)))
#define MOVE_COL(m)              ((int)((m) & 0xFF))
#define MOVE_ROW(m)              ((int)(((m)>>8) & 0xFF))
#define MOVE_DIR(m)              ((int)(((m)>>16) & 0x3))

/*----------------------------------------------------------------------
 *  Forward decls: generic_hash thin wrappers (non-tier mode)
 *--------------------------------------------------------------------*/
static void     unhash_pos(char *B, POSITION position, int* turn);
static POSITION hash_pos(char *B, int turn);

/*----------------------------------------------------------------------
 *  Helpers on gBoard (no bit-packing)
 *--------------------------------------------------------------------*/
static inline int  IDX(int r,int c){ return r*COLS + c; }
static inline char AT (char *B, int r,int c){ return B[IDX(r,c)]; }
static inline void SET(char *B, int r,int c,char v){ B[IDX(r,c)] = v; }



/* ===== debug switches ===== */
static int DBG_ON = 1;      // 置 0 关闭全部调试
static int DBG_MOVES = 1;   // 打印 GenerateMoves 列表
static int DBG_DOMOVE = 1;  // 打印 DoMove 前后棋盘

static void dump_board_compact(char *B){
    for(int r=0;r<ROWS;r++){
        for(int c=0;c<COLS;c++){
            char v = AT(B,r,c);
            fputc(v=='*' ? '.' : v, stderr);
        }
        if(r<ROWS-1) fputc('/', stderr);
    }
}
/* ===== debug switches ===== */




static int row_has_piece(char *B, int r){
    for(int c=0;c<COLS;c++) if(AT(B,r,c)!='*') return 1;
    return 0;
}
static int top_full_col(char *B, int c){ return AT(B,0,c)!='*'; }

static void apply_gravity_col(char *B, int c){
    for(int r=ROWS-2;r>=0;r--){
        if(AT(B,r,c)!='*'){
            int rr=r;
            while(rr+1<ROWS && AT(B,rr+1,c)=='*') rr++;
            if(rr!=r){ SET(B,rr,c,AT(B,r,c)); SET(B,r,c,'*'); }
        }
    }
}

static void drop_piece_do(char *B, int col, char me){
    for(int r=ROWS-1;r>=0;r--){
        if(AT(B,r,col)=='*'){ SET(B,r,col,me); return; }
    }
}

static void twist_row_do(char *B, int row, int dir /* -1=LEFT, +1=RIGHT */){
    if(!row_has_piece(B, row)) return;
    char tmp[COLS]; for(int c=0;c<COLS;c++) tmp[c]=AT(B,row,c);
    if(dir<0)  for(int c=0;c<COLS;c++) SET(B,row,c, tmp[(c+1)%COLS]);
    else       for(int c=0;c<COLS;c++) SET(B,row,c, tmp[(c+COLS-1)%COLS]);
    for(int c=0;c<COLS;c++) apply_gravity_col(B, c);
}

static int has_four_for(char *B, char p){
    for(int r=0;r+3<ROWS;r++)
        for(int c=0;c<COLS;c++)
            if(AT(B,r,c)==p && AT(B,r+1,c)==p && AT(B,r+2,c)==p && AT(B,r+3,c)==p)
                return 1;

    for(int r=0;r<ROWS;r++){
        for(int c0=0;c0<COLS;c0++){
            int c1=(c0+1)%COLS, c2=(c0+2)%COLS, c3=(c0+3)%COLS;
            if(AT(B,r,c0)==p && AT(B,r,c1)==p && AT(B,r,c2)==p && AT(B,r,c3)==p)
                return 1;
        }
    }

    for(int r=0;r<ROWS;r++) for(int c=0;c<COLS;c++){
        if (r+3<ROWS && c+3<COLS &&
            AT(B,r,c)==p && AT(B,r+1,c+1)==p && AT(B,r+2,c+2)==p && AT(B,r+3,c+3)==p) return 1;
        if (r-3>=0  && c+3<COLS &&
            AT(B,r,c)==p && AT(B,r-1,c+1)==p && AT(B,r-2,c+2)==p && AT(B,r-3,c+3)==p) return 1;
    }
    return 0;
}

static int top_row_full_all_cols(char *B){
    for(int c=0;c<COLS;c++) if(AT(B,0,c)=='*') return 0;
    return 1;
}

/*----------------------------------------------------------------------
 *  generic_hash wrappers (single global context)
 *--------------------------------------------------------------------*/
static void unhash_pos(char *B, POSITION position, int* turn){
    if (turn) *turn = generic_hash_turn(position);       /* 1 or 2 */
    generic_hash_unhash(position, B);     /* fills B */
}
static POSITION hash_pos(char *B, int turn){
    return generic_hash_hash(B, turn);
}

/*----------------------------------------------------------------------
 *  Optional validity filter (vcfg): X first -> x==o or x==o+1
 *--------------------------------------------------------------------*/
static int vcfg_connect4(int pieces[]){
    // TODO： check floating pieces
    /* pieces[] aligned with pieces_arr below: 'x','o','*' */
    int x = pieces[0], o = pieces[1];
    return (x == o) || (x == o + 1);
}

/*----------------------------------------------------------------------
 *  Core Solver Hooks
 *--------------------------------------------------------------------*/
void InitializeGame(void){
    generic_hash_destroy();

    /* generous upper bounds; legality by vcfg_connect4 */
    int pieces_arr[10] = {
        'x', 0, 15,   /* up to 15 X */
        'o', 0, 15,   /* up to 15 O */
        '*', 0, 30,   /* blanks 0..30 */
        -1
    };

    gNumberOfPositions = generic_hash_init(CELLS, pieces_arr, vcfg_connect4, 0);
    fprintf(stderr, "\n[GM] gnumpos=%d", gNumberOfPositions); 

    /* empty board, X to move (turn=1) */
    char B[CELLS]; memset(B,'*',CELLS);
    gInitialPosition = generic_hash_hash(B, 1);
}

/*----------------------------------------------------------------------
 *  Generate all legal moves from a position
 *--------------------------------------------------------------------*/
MOVELIST *GenerateMoves(POSITION position){
    MOVELIST *list=NULL;
    char B[CELLS]; 
    int turn; unhash_pos(B, position, &turn);

    if (DBG_ON && DBG_MOVES) {
        // fprintf(stderr, "\n[GM] turn=%d board=", turn); dump_board_compact(B); fputc('\n', stderr);
    }
    /* count placed pieces to decide whether twists are allowed */
    int placed=0; for(int i=0;i<CELLS;i++) if(B[i]!='*') placed++;
    for(int c=0;c<COLS;c++){
        if(!top_full_col(B, c)){
            /* twists only allowed after the very first move (spec per你的描述) */
            if(placed>0){
                for(int r=0;r<ROWS;r++){
                    if(row_has_piece(B, r)){
                        list = CreateMovelistNode(MOVE_MAKE(c,r,DIR_LEFT),  list);
                        list = CreateMovelistNode(MOVE_MAKE(c,r,DIR_RIGHT), list);
                    }
                }
            }
            /* plain drop (no twist) always allowed */
            list = CreateMovelistNode(MOVE_MAKE(c,0,DIR_NONE), list);
        }
    }
    return list;
}

/*----------------------------------------------------------------------
 *  Apply a move to produce the child position
 *--------------------------------------------------------------------*/
POSITION DoMove(POSITION position, MOVE m){
    char B[CELLS]; 
    int turn; unhash_pos(B, position, &turn);
    char me = (turn==1)?'x':'o';

    int col = MOVE_COL(m);
    int row = MOVE_ROW(m);
    TwistDir dir = (TwistDir)MOVE_DIR(m);

    drop_piece_do(B, col, me);
    if(dir==DIR_LEFT)  twist_row_do(B, row, -1);
    if(dir==DIR_RIGHT) twist_row_do(B, row, +1);

    int next = (turn==1)?2:1;
    return hash_pos(B, next);
}

/*----------------------------------------------------------------------
 *  Terminal-state evaluation
 *--------------------------------------------------------------------*/
VALUE Primitive(POSITION position){
    char B[CELLS]; 
    int turn; unhash_pos(B, position, &turn);

    int x4 = has_four_for(B, 'x');
    int o4 = has_four_for(B, 'o');

    if (x4 && !o4) {
        return (turn == 2) ? lose : win;
    }
    if (o4 && !x4) {
        return (turn == 1) ? lose : win;
    }
    if (x4 && o4) {
        return lose;
    }
    if (top_row_full_all_cols(B)) return tie;

    return undecided;
}

/*----------------------------------------------------------------------
 *  Text UI helpers
 *--------------------------------------------------------------------*/
static void print_separator(void){
    putchar('+');
    for (int c = 0; c < COLS; c++) printf("---+");
    putchar('\n');
}

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn){
    (void)playerName;
    (void)usersTurn;
    int turn; 
    char B[CELLS]; unhash_pos(B, position, &turn);

    printf("  ");
    for (int c = 0; c < COLS; c++) printf("  %d ", c+1);
    printf("\n");

    print_separator();
    for (int r = 0; r < ROWS; r++){
        printf("|");
        for (int c = 0; c < COLS; c++){
            char v = AT(B,r,c);
            char ch = (v=='x' ? 'X' : (v=='o' ? 'O' : ' '));
            printf(" %c |", ch);
        }
        printf(" %d\n", r+1);
        print_separator();
    }
    printf("%s to move.\n", (turn==1) ? "X" : "O");
}

BOOLEAN ValidTextInput(STRING s){
    /* Accept: "<col> N" or "<col> <L|R> <row>" */
    int c=0, r=0; char d='N';
    int n = sscanf(s, "%d %c %d", &c, &d, &r);
    if (n < 2) return FALSE;

    if (c < 1 || c > COLS) return FALSE;
    d = (char)toupper((unsigned char)d);

    if (d == 'N') {
        return TRUE; /* row ignored */
    } else if (d == 'L' || d == 'R') {
        if (n < 3) return FALSE;
        if (r < 1 || r > ROWS) return FALSE;
        return TRUE;
    }
    return FALSE;
}

MOVE ConvertTextInputToMove(STRING s){
    int c=0, r=0; char d='N';
    (void)sscanf(s, "%d %c %d", &c, &d, &r);
    d = (char)toupper((unsigned char)d);
    if (d == 'N') r = 1; /* row ignored in NONE; keep in range */
    TwistDir dir = (d=='L')?DIR_LEFT : (d=='R')?DIR_RIGHT : DIR_NONE;
    return MOVE_MAKE(c-1, r-1, dir);
}

void MoveToString(MOVE m, char *buf){
    int c = MOVE_COL(m)+1, r = MOVE_ROW(m)+1;
    int d = MOVE_DIR(m);
    char dirChar = (d==DIR_LEFT?'L' : (d==DIR_RIGHT?'R' : 'N'));
    sprintf(buf, "%d %c %d", c, dirChar, r);
}

USERINPUT GetAndPrintPlayersMove(POSITION thePosition, MOVE *theMove, STRING playerName) {
    USERINPUT ret;
    char B[CELLS];
    int turn; unhash_pos(B, thePosition, &turn);
    char whoseTurn = (turn==1)?'x':'o';

    do {
        printf("for a list of valid moves, press ?\n\n");
        printf("%8s's move (%c):  ", playerName, whoseTurn);
        ret = HandleDefaultTextInput(thePosition, theMove, playerName);
        if (ret != Continue) return ret;
    } while (TRUE);
    /* not reached */
}

/* REQUIRED even if kDebugMenu==FALSE */
void DebugMenu(void) { /* no-op */ }

void PrintComputersMove(MOVE mv, STRING name){
    char buf[32];
    MoveToString(mv, buf);
    printf("%s plays: %s\n", name, buf);
}

/*----------------------------------------------------------------------
 *  Variants (none)
 *--------------------------------------------------------------------*/
int NumberOfOptions(void){ return 1; }
int getOption(void){ return 0; }
void setOption(int option){ (void)option; }
void GameSpecificMenu(void){}

/*----------------------------------------------------------------------
 *  Required but unused conversions
 *--------------------------------------------------------------------*/
void PositionToString(POSITION p,char *b){ (void)p;(void)b; }
POSITION StringToPosition(char *s){ (void)s; return 0; }
void PositionToAutoGUIString(POSITION p,char *b){ (void)p;(void)b; }
void MoveToAutoGUIString(POSITION p,MOVE m,char *b){ (void)p;(void)m;(void)b; }
