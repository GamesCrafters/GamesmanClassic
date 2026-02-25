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

POSITION gNumberOfPositions;
POSITION kBadPosition = INVALID_POSITION;

POSITION gInitialPosition;
POSITION gMinimalPosition;

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
	
	return NULL_POSITION;
}

void PositionToAutoGUIString(POSITION position, char *autoguiPositionStringBuffer) {
    return;
    // AutoGUIMakePositionString(turn, pieces, autoguiPositionStringBuffer);
}

void MoveToAutoGUIString(POSITION position, MOVE move, char *autoguiMoveStringBuffer) {
    return;
    // AutoGUIMakeMoveButtonStringM(w, w + COLUMNCOUNT, 'x', autoguiMoveStringBuffer);
}

// Need to find shape of position and find its owner
void GetBlobFileNameFromPosition(POSITION p, char *filename) {
    snprintf(filename, 256, "./data/othellofour/test%llu.txt", 1ULL);
    return;
}

// Need to, given the file, find the page, then find the offset/position
UINT64 GetInfoFromBlobFile(POSITION p, FILE *f) {
    return 0;
}

VALUE GetPrimitiveFromInfo(UINT64 info) {
    return 0b11000000 & info;
}

REMOTENESS GetRemotenessFromInfo(UINT64 info) {
    return 0b00111111 & info;
}