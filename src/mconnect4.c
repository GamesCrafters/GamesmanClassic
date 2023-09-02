/************************************************************************
**
** NAME:        mconnect4.c
**
** DESCRIPTION: Connect 4
**
** AUTHOR:      Justin Yokota^^^, Cameron Cheung^
**
** DATE:        2022-10-23
**
************************************************************************/

#include "gamesman.h"

POSITION gNumberOfPositions = 0;
POSITION kBadPosition = -1;

POSITION gInitialPosition = 0;
POSITION gMinimalPosition = 0;

CONST_STRING kAuthorName = "Justin Yokota^^^, Cameron Cheung^";
CONST_STRING kGameName = "mconnect4";
CONST_STRING kDBName = "connect4new";  // should not be used
BOOLEAN kPartizan = TRUE;
BOOLEAN kDebugMenu = FALSE;
BOOLEAN kGameSpecificMenu = FALSE;
BOOLEAN kTieIsPossible = TRUE;
BOOLEAN kLoopy = FALSE;
BOOLEAN kDebugDetermineValue = FALSE;
BOOLEAN kSupportsSymmetries = FALSE; /* Whether we support symmetries */
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
** Everything above here must be in every game file
**
**************************************************************************/

/*************************************************************************
**
** Every variable declared here is only used in this file
*(game-specific)
**
**************************************************************************/

POSITION GetCanonicalPosition(POSITION);
STRING MoveToString(MOVE);

int ROWCOUNT = 6;
int COLUMNCOUNT = 6;
int CONNECT = 4;

static uint64_t DOWNDIAGWIN;
static uint64_t HORIZONTALWIN;
static uint64_t VERTICALWIN;
static uint64_t UPDIAGWIN;
static uint64_t INITIALPOSITION;

POSITION GetCanonicalPosition(POSITION position) { return position; }

void DebugMenu() {}

void SetTclCGameSpecificOptions(int theOptions[]) {
    (void)theOptions;
}

/************************************************************************
**
** NAME: InitializeDatabases
**
** DESCRIPTION: Initialize the gDatabase, a global variable.
**
************************************************************************/

void InitializeGame() {
    kSupportsShardGamesman = TRUE;
    gCanonicalPosition = GetCanonicalPosition;
    gMoveToStringFunPtr = &MoveToString;

    uint64_t l = 0;
    for (int i = 0; i < COLUMNCOUNT; i++)
        l |= 1ULL << ((ROWCOUNT + 1) * i);
    uint64_t down = 0, left = 0, updiag = 0, downdiag = 0;
    for (int i = 0; i < CONNECT; i++) {
        down = (down << 1) | 1;
        left = (left << (ROWCOUNT + 1)) | 1;
        downdiag = (downdiag << ROWCOUNT) | 1;
        updiag = (updiag << (ROWCOUNT + 2)) | 1;
    }
    DOWNDIAGWIN = downdiag;
    HORIZONTALWIN = left;
    VERTICALWIN = down;
    UPDIAGWIN = updiag;
    INITIALPOSITION = l;

    gInitialPosition = INITIALPOSITION;
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

/** Returns the result of moving at the given position
 * Bit 63 is flipped to change current player
 * Bit 62-55 will have the most recent move (because primitive relies
 * on it; when querying db this will be truncated) Move is assumed to
 * be the bit position of the empty cell. As such, we add 1<<move for
 * a yellow, and 1<<(move+1) for red 0b0001011 + 1<<move = 0b0010011 =
 * ---YYRR 0b0001011 + 1<<(move+1) = 0b0011011 = ---RYRR ASSUMES
 * boardhash is at most 54 bits */
POSITION DoMove(POSITION position, MOVE move) {
    return (((position & 0x807FFFFFFFFFFFFFL) ^ 0x8000000000000000L) |
            ((POSITION)move << 55)) +
           (1ULL << (move + (position >> 63)));
}

/************************************************************************
**
** NAME: GetInitialPosition
**
** DESCRIPTION: Ask the user for an initial position for testing.
*Store
** it in the space pointed to by initialPosition;
**
** OUTPUTS: POSITION initialPosition : The position to fill.
**
************************************************************************/

POSITION GetInitialPosition() {
    // printf("Initialposition: %llu\n", INITIALPOSITION);
    return INITIALPOSITION;
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

/*
static BOOLEAN isawin(POSITION position, POSITION pieces)
{
    //System.out.printf("%016X %b %n", pieces, (position&pieces) ==
pieces); return (position&pieces) == pieces;
}
*/

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
    (void)playerName;
    (void)usersTurn;
    for (int i = 63; i >= 0; i--) {
        printf("%llu", (position >> i) & 1);
    }
    printf("\n");
    char pieces[ROWCOUNT * COLUMNCOUNT];
    int k = 0;
    BOOLEAN pastSigBit = FALSE;
    for (int i = (ROWCOUNT + 1) * COLUMNCOUNT - 1; i >= 0; i--) {
        BOOLEAN bit = (position >> i) & 1;
        if (i % (ROWCOUNT + 1) == ROWCOUNT) {
            pastSigBit = bit;
        } else if (bit) {
            if (pastSigBit) {
                pieces[k] = 'O';
            } else {
                pieces[k] = '-';
                pastSigBit = TRUE;
            }
            k++;
        } else {
            if (pastSigBit) {
                pieces[k] = 'X';
            } else {
                pieces[k] = '-';
            }
            k++;
        }
    }
    for (int i = 0; i < ROWCOUNT; i++) {
        for (int j = 0; j < COLUMNCOUNT; j++) {
            printf("%c", pieces[i + j * ROWCOUNT]);
        }
        printf("\n");
    }
    printf("Most Recent Move: %llu\n",
           (((position >> 55) & 0xFF) / (ROWCOUNT + 1)));
    printf("Turn: %s\n", (position >> 63) ? "Yellow" : "Red");
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

MOVELIST *GenerateMoves(POSITION position) {
    MOVELIST *moves = NULL;
    for (int i = 0; i < COLUMNCOUNT; i++) {
        MOVE start = (ROWCOUNT + 1) * (i + 1) - 1;
        while ((position & (1ULL << start)) == 0)
            start--;  // This is what sigbit does
        if (start != (ROWCOUNT + 1) * (i + 1) -
                         1) {  // If there is space in this column
            moves = CreateMovelistNode(start, moves);
        }
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
    USERINPUT ret, HandleDefaultTextInput();
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
    if (input[1]) {
        return (input[0] - '0') * 10 + (input[1] - '0');
    } else {
        return input[0] - '0';
    }
}

/************************************************************************
**
** NAME: PrintMove
**
** DESCRIPTION: Print the move in a nice format.
**
** INPUTS: MOVE *theMove : The move to print.
**
************************************************************************/

void PrintMove(MOVE move) { printf("%s", MoveToString(move)); }

/************************************************************************
**
** NAME: MoveToString
**
** DESCRIPTION: Returns the move as a STRING
**
** INPUTS: MOVE *move : The move to put into a string.
**
************************************************************************/

STRING MoveToString(MOVE move) {
    STRING movestring = (STRING)SafeMalloc(5);
    movestring[1] = '\0';
    movestring[2] = '\0';
    if (gIsInteract) {
        snprintf(movestring, 4, "%d",
                 COLUMNCOUNT - (move / (ROWCOUNT + 1)));
    } else {
        snprintf(movestring, 4, "%d", move);
    }
    return movestring;
}

int NumberOfOptions() { return 2; }

int getOption() { return (COLUMNCOUNT == 7) ? 2 : 1; }

void setOption(int option) {
    if (option == 2) {
        COLUMNCOUNT = 7;
    } else {
        COLUMNCOUNT = 6;
    }
    uint64_t l = 0;
    for (int i = 0; i < COLUMNCOUNT; i++)
        l |= 1ULL << ((ROWCOUNT + 1) * i);
    uint64_t down = 0, left = 0, updiag = 0, downdiag = 0;
    for (int i = 0; i < CONNECT; i++) {
        down = (down << 1) | 1;
        left = (left << (ROWCOUNT + 1)) | 1;
        downdiag = (downdiag << ROWCOUNT) | 1;
        updiag = (updiag << (ROWCOUNT + 2)) | 1;
    }
    DOWNDIAGWIN = downdiag;
    HORIZONTALWIN = left;
    VERTICALWIN = down;
    UPDIAGWIN = updiag;
    INITIALPOSITION = l;

    gInitialPosition = INITIALPOSITION;
}

POSITION InteractStringToPosition(STRING str) {
  char *board = str + 8;
  POSITION pos = 0;
  for (int c = 0; c < COLUMNCOUNT; c++) {
    POSITION colbits = 0;
    BOOLEAN endFound = FALSE;
    for (int r = 0; !endFound && r < ROWCOUNT; r++) {
      char piece = board[ROWCOUNT * (c + 1) - 1 - r];
      if (piece == 'O') {
        colbits |= (1 << r);
      } else if (piece == '-') {
        endFound = TRUE;
        colbits |= (1 << r);
      }
    }
    if (!endFound) colbits |= (1 << ROWCOUNT);
    pos |= (colbits << ((ROWCOUNT + 1) * (COLUMNCOUNT - 1 - c)));
  }
  if (str[2] == 'B') pos |= (1ULL << 63);
  return pos;
}

STRING InteractPositionToString(POSITION position) {
  int strLen = ROWCOUNT * COLUMNCOUNT + 1;
  char pieces[strLen];
  int piecesPlaced = 0;
  int k = 0;
  BOOLEAN pastSigBit = FALSE;

  for (int i = (ROWCOUNT + 1) * COLUMNCOUNT - 1; i >= 0; i--) {
    BOOLEAN bit = (position >> i) & 1;
    if (i % (ROWCOUNT + 1) == ROWCOUNT) {
      pastSigBit = bit;
    } else if (bit) {
      if (pastSigBit) {
        pieces[k] = 'O';
        piecesPlaced++;
      } else {
        pieces[k] = '-';
        pastSigBit = TRUE;
      }
      k++;
    } else {
      if (pastSigBit) {
        pieces[k] = 'X';
        piecesPlaced++;
      } else {
        pieces[k] = '-';
      }
      k++;
    }
  }

  pieces[ROWCOUNT * COLUMNCOUNT] = '\0';
  enum UWAPI_Turn turn = (piecesPlaced & 1) ? UWAPI_TURN_B : UWAPI_TURN_A; 
  return UWAPI_Board_Regular2D_MakeBoardString(turn, strLen, pieces);
}

STRING InteractMoveToString(POSITION position, MOVE move) {
    (void)position;
    return UWAPI_Board_Regular2D_MakeAddString(
        'a',
        (ROWCOUNT + 1) * COLUMNCOUNT - 1 - (move / (ROWCOUNT + 1)));
}