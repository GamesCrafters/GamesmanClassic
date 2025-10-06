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

/*----------------------------------------------------------------------
 *  Game Metadata
 *--------------------------------------------------------------------*/
CONST_STRING kAuthorName = "Enzo Ribeiro & Weiyi Zhang";
CONST_STRING kGameName   = "Connect-4 Twist & Turn";
CONST_STRING kDBName     = "connect4twist";

/*----------------------------------------------------------------------
 *  Game Characteristics
 *--------------------------------------------------------------------*/
BOOLEAN kPartizan           = FALSE;   /* both players share identical move sets */
BOOLEAN kTieIsPossible      = TRUE;
BOOLEAN kLoopy              = FALSE;   /* every move adds a new piece ⇒ acyclic */
BOOLEAN kSupportsSymmetries = FALSE;   /* we skip symmetry canonicalization     */
POSITION kBadPosition       = -1;
BOOLEAN kDebugDetermineValue= FALSE;

POSITION gNumberOfPositions = (POSITION)(1ULL << 61);       /* not strictly used */
POSITION gInitialPosition   = 0;

BOOLEAN kGameSpecificMenu   = FALSE;
BOOLEAN kDebugMenu          = FALSE;

/*----------------------------------------------------------------------
 *  Help Strings (for the Text UI)
 *--------------------------------------------------------------------*/
CONST_STRING kHelpGraphicInterface  = "";
CONST_STRING kHelpTextInterface     = "";
CONST_STRING kHelpOnYourTurn        = "Drop a piece in a column, then optionally twist one non-empty row left or right.";
CONST_STRING kHelpStandardObjective = "Make 4 in a row (horizontal, vertical, or diagonal).";
CONST_STRING kHelpReverseObjective  = "";
CONST_STRING kHelpTieOccursWhen     = "the board is full and nobody has 4 in a row.";
CONST_STRING kHelpExample           = "";

/*----------------------------------------------------------------------
 *  Tcl / GUI hooks (unused)
 *--------------------------------------------------------------------*/
void *gGameSpecificTclInit = NULL;
void SetTclCGameSpecificOptions(int theOptions[]) { (void)theOptions; }

/*----------------------------------------------------------------------
 *  Board Constants
 *--------------------------------------------------------------------*/
enum { ROWS = 5, COLS = 6, CELLS = ROWS * COLS };

/*----------------------------------------------------------------------
 *  Position Encoding
 *
 *  Each board cell uses 2 bits:
 *      00 = empty, 01 = Player1 (X), 10 = Player2 (O)
 *  The 5×6 board requires 30 cells → 60 bits total.
 *  Bit 60 stores side-to-move (0 = P1, 1 = P2).
 *
 *  MOVE encoding (32-bit):
 *      bits 0-7   : drop column (0..5)
 *      bits 8-15  : twist row  (0..4) — ignored if direction == NONE
 *      bits 16-17 : direction   (0 = NONE, 1 = LEFT, 2 = RIGHT)
 *--------------------------------------------------------------------*/
typedef unsigned long long U64;

enum { CELL_EMPTY=0, CELL_P1=1, CELL_P2=2 };
typedef enum { DIR_NONE=0, DIR_LEFT=1, DIR_RIGHT=2 } TwistDir;

#define MOVE_MAKE(col,row,dir)   ((MOVE)(((col)&0xFF) | (((row)&0xFF)<<8) | (((dir)&0x3)<<16)))
#define MOVE_COL(m)              ((int)((m) & 0xFF))
#define MOVE_ROW(m)              ((int)(((m)>>8) & 0xFF))
#define MOVE_DIR(m)              ((int)(((m)>>16) & 0x3))

static inline int POS_TURN(POSITION p){ return (int)((((U64)p)>>60) & 1ULL); }
static inline POSITION POS_SET_TURN(POSITION p,int t){
    U64 x=(U64)p; x&=~(1ULL<<60); x|=((U64)(t&1))<<60; return (POSITION)x;
}

/* 2-bit access helpers */
static inline int cell_get(POSITION p,int r,int c){
    int idx=r*COLS+c, shift=idx*2;
    return (int)((((U64)p)>>shift)&0x3ULL);
}
static inline POSITION cell_set(POSITION p,int r,int c,int v){
    int idx=r*COLS+c, shift=idx*2;
    U64 mask=~(0x3ULL<<shift);
    U64 x=(U64)p; x&=mask; x|=((U64)(v&0x3))<<shift;
    return (POSITION)x;
}

/*----------------------------------------------------------------------
 *  Utility: count number of placed pieces
 *--------------------------------------------------------------------*/
static int count_pieces(POSITION p){
    int cnt=0;
    for(int r=0;r<ROWS;r++)
        for(int c=0;c<COLS;c++)
            if(cell_get(p,r,c)!=CELL_EMPTY) cnt++;
    return cnt;
}

/*----------------------------------------------------------------------
 *  Drop a piece into a column (gravity)
 *--------------------------------------------------------------------*/
static POSITION drop_piece(POSITION pos,int col){
    int turn=POS_TURN(pos);
    int piece=(turn==0)?CELL_P1:CELL_P2;
    for(int r=ROWS-1;r>=0;r--){
        if(cell_get(pos,r,col)==CELL_EMPTY){
            pos=cell_set(pos,r,col,piece);
            return pos;
        }
    }
    return pos; /* full column => unchanged */
}

/*----------------------------------------------------------------------
 *  Apply gravity to a single column (after twisting)
 *--------------------------------------------------------------------*/
static POSITION apply_gravity(POSITION pos,int col){
    for(int r=ROWS-2;r>=0;r--){
        int v=cell_get(pos,r,col);
        if(v!=CELL_EMPTY){
            int rr=r;
            while(rr+1<ROWS && cell_get(pos,rr+1,col)==CELL_EMPTY) rr++;
            if(rr!=r){
                pos=cell_set(pos,rr,col,v);
                pos=cell_set(pos,r,col,CELL_EMPTY);
            }
        }
    }
    return pos;
}

/*----------------------------------------------------------------------
 *  Twist a specific row left or right, then re-apply gravity
 *--------------------------------------------------------------------*/
static POSITION twist_row(POSITION pos,int row,TwistDir dir){
    if(dir==DIR_NONE) return pos;

    int temp[COLS];
    for(int c=0;c<COLS;c++) temp[c]=cell_get(pos,row,c);

    if(dir==DIR_LEFT){
        for(int c=0;c<COLS;c++)
            pos=cell_set(pos,row,c,temp[(c+1)%COLS]);
    }else if(dir==DIR_RIGHT){
        for(int c=0;c<COLS;c++)
            pos=cell_set(pos,row,c,temp[(c+COLS-1)%COLS]);
    }

    for(int c=0;c<COLS;c++)
        pos=apply_gravity(pos,c);

    return pos;
}

/*----------------------------------------------------------------------
 *  Check if someone has 4 in a row
 *--------------------------------------------------------------------*/
static int check_winner(POSITION p){
    for(int r=0;r<ROWS;r++)
        for(int c=0;c<COLS;c++){
            int v=cell_get(p,r,c);
            if(v==CELL_EMPTY) continue;
            /* vertical */
            if(r+3<ROWS &&
               v==cell_get(p,r+1,c)&&v==cell_get(p,r+2,c)&&v==cell_get(p,r+3,c))
                return 1;
            /* horizontal */
            if(c+3<COLS &&
               v==cell_get(p,r,c+1)&&v==cell_get(p,r,c+2)&&v==cell_get(p,r,c+3))
                return 1;
            /* diag down-right */
            if(r+3<ROWS && c+3<COLS &&
               v==cell_get(p,r+1,c+1)&&v==cell_get(p,r+2,c+2)&&v==cell_get(p,r+3,c+3))
                return 1;
            /* diag up-right */
            if(r-3>=0 && c+3<COLS &&
               v==cell_get(p,r-1,c+1)&&v==cell_get(p,r-2,c+2)&&v==cell_get(p,r-3,c+3))
                return 1;
        }
    return 0;
}

/*----------------------------------------------------------------------
 *  Core Solver Hooks
 *--------------------------------------------------------------------*/
void InitializeGame(void){
    gCanonicalPosition = NULL;
    gPositionToStringFunPtr = NULL;
    /* initial position = empty board, P1 to move */
    gInitialPosition = POS_SET_TURN(0ULL,0);
}

/*----------------------------------------------------------------------
 *  Generate all legal moves from a position
 *--------------------------------------------------------------------*/
MOVELIST *GenerateMoves(POSITION p){
    MOVELIST *list=NULL;
    int turnCount=count_pieces(p);

    /* Step 1: drop moves for each non-full column */
    for(int c=0;c<COLS;c++){
        if(cell_get(p,0,c)==CELL_EMPTY){
            /* If not the first move, we also allow row twists */
            if(turnCount>0){
                /* For each non-empty row, two directions */
                for(int r=0;r<ROWS;r++){
                    int nonEmpty=0;
                    for(int cc=0;cc<COLS;cc++)
                        if(cell_get(p,r,cc)!=CELL_EMPTY){nonEmpty=1;break;}
                    if(nonEmpty){
                        list=CreateMovelistNode(MOVE_MAKE(c,r,DIR_LEFT),list);
                        list=CreateMovelistNode(MOVE_MAKE(c,r,DIR_RIGHT),list);
                    }
                }
            }
            /* Always allow plain drop (no twist) */
            list=CreateMovelistNode(MOVE_MAKE(c,0,DIR_NONE),list);
        }
    }
    return list;
}

/*----------------------------------------------------------------------
 *  Apply a move to produce the child position
 *--------------------------------------------------------------------*/
POSITION DoMove(POSITION p,MOVE m){
    int col=MOVE_COL(m);
    int row=MOVE_ROW(m);
    TwistDir dir=(TwistDir)MOVE_DIR(m);

    p=drop_piece(p,col);
    if(dir!=DIR_NONE)
        p=twist_row(p,row,dir);

    /* Switch side to move */
    int turn=POS_TURN(p);
    p=POS_SET_TURN(p,1-turn);
    return p;
}

/*----------------------------------------------------------------------
 *  Terminal-state evaluation
 *--------------------------------------------------------------------*/
VALUE Primitive(POSITION p){
    if(check_winner(p))
        return lose; /* current player to move loses (since previous won) */

    /* Tie if top row full */
    int full=1;
    for(int c=0;c<COLS;c++)
        if(cell_get(p,0,c)==CELL_EMPTY) full=0;
    if(full) return tie;

    return undecided;
}
/*----------------------------------------------------------------------
 *  Simple text interface helpers
 *--------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>

BOOLEAN ValidTextInput(STRING s){
    /* Accept: "<col> N" or "<col> <L|R> <row>" */
    int c=0, r=0; char d='N';
    int n = sscanf(s, "%d %c %d", &c, &d, &r);
    if (n < 2) return FALSE;

    if (c < 1 || c > COLS) return FALSE;
    d = (char)toupper((unsigned char)d);

    if (d == 'N') {
        /* form: "<col> N" (2 tokens ok; third ignored if present) */
        return TRUE;
    } else if (d == 'L' || d == 'R') {
        if (n < 3) return FALSE;       /* need the row */
        if (r < 1 || r > ROWS) return FALSE;
        return TRUE;
    }
    return FALSE;
}

MOVE ConvertTextInputToMove(STRING s){
    int c=0, r=0; char d='N';
    sscanf(s, "%d %c %d", &c, &d, &r);
    d = (char)toupper((unsigned char)d);
    if (d == 'N') r = 1; /* row is ignored for NONE; any in-range value is fine */
    TwistDir dir = (d=='L')?DIR_LEFT : (d=='R')?DIR_RIGHT : DIR_NONE;
    return MOVE_MAKE(c-1, r-1, dir);
}

void MoveToString(MOVE m, char *buf){
    int c = MOVE_COL(m)+1, r = MOVE_ROW(m)+1;
    int d = MOVE_DIR(m);
    char dirChar = (d==DIR_LEFT?'L' : (d==DIR_RIGHT?'R' : 'N'));
    /* keep it short; buffer from caller is at least ~32 */
    sprintf(buf, "%d %c %d", c, dirChar, r);
}

/* single definition; also silence unused-parameter warning */
/* neat grid separator like +---+---+... */
static void print_separator(void){
    putchar('+');
    for (int c = 0; c < COLS; c++) printf("---+");
    putchar('\n');
}

void PrintPosition(POSITION p, STRING name, BOOLEAN usersTurn){
    (void)usersTurn;

    /* column header */
    printf("  ");               
    for (int c = 0; c < COLS; c++)
        printf("  %d ", c+1);    
    printf("\n");

    print_separator();
    for (int r = 0; r < ROWS; r++){
        printf("|");
        for (int c = 0; c < COLS; c++){
            int v = cell_get(p,r,c);
            char ch = (v==CELL_P1?'X' : (v==CELL_P2?'O' : ' '));
            printf(" %c |", ch);
        }
        printf(" %d\n", r+1);  
        print_separator();
    }
    printf("%s to move.\n", name);
}


/* REQUIRED: used by TextUI */
USERINPUT GetAndPrintPlayersMove(POSITION position, MOVE *move, STRING playerName){
    printf("%s, enter: <col> <L|R|N> [row]\n", playerName);
    printf("Examples: 3 N   |   4 L 2   |   6 R 5\n> ");
    return HandleDefaultTextInput(position, move, playerName);
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

