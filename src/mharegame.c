/************************************************************************
**
** NAME:        mharegame.c
**
** DESCRIPTION: Hare Game
**
** AUTHOR:      Robert Shi
**
** DATE:        2022-09-09
**
************************************************************************/

#include "gamesman.h"

POSITION gNumberOfPositions; // Initialized in InitializeGame().
POSITION kBadPosition = -1;

POSITION gInitialPosition; // Initialized in InitializeGame().

CONST_STRING kAuthorName = "Robert Shi";
CONST_STRING kGameName = "Hare Game";
CONST_STRING kDBName = "haregame";
BOOLEAN kPartizan = TRUE; // Set this to true if different sets of moves are available to each player given a position.
BOOLEAN kDebugMenu = FALSE;
BOOLEAN kGameSpecificMenu = TRUE; // Game-specific menu to choose the game variant.
BOOLEAN kTieIsPossible = FALSE; // There are draw positions but no ties.
BOOLEAN kLoopy = TRUE;
BOOLEAN kDebugDetermineValue = FALSE;
BOOLEAN kSupportsSymmetries = TRUE; // Flipping the board across the x-axis gives a symmetric position.
void* gGameSpecificTclInit = NULL;

CONST_STRING kHelpGraphicInterface = ""; // Ignore for now.
CONST_STRING kHelpTextInterface = "kHelpTextInterface goes here"; // Answer to "What do I do on MY TURN?"
CONST_STRING kHelpOnYourTurn = "kHelpOnYourTurn goes here"; // Answer to "How do I tell the computer WHICH MOVE I want?"
CONST_STRING kHelpStandardObjective = "kHelpStandardObjective goes here"; // Answer to "What is the %s OBJECTIVE of Hare Game?"
CONST_STRING kHelpReverseObjective = ""; // Ignore for now. 
CONST_STRING kHelpTieOccursWhen = "A tie never occurs."; // Should follow "A Tie occurs when..."
CONST_STRING kHelpExample = "kHelpExample goes here"; // A string that shows an example game.

/*************************************************************************
**
** Everything above here must be in every game file
**
**************************************************************************/

#define DEFAULT_SIZE 0              // Small = 0, medium = 1, large = 2.
#define DEFAULT_HARE_FIRST FALSE    // TRUE if hare goes first, FALSE if hounds go first.

#define DEFAULT_VARIANT ((DEFAULT_SIZE << 1) + DEFAULT_HARE_FIRST)  // Encoding of a variant number.
#define numCells(variant) (((variant) >> 1) * 6 + 11)                 // Returns number of cells on board.

#define SIZE_VARIANT_MAX 2                      // Maximum size index.
#define N_MAX numCells(SIZE_VARIANT_MAX << 1)    // Determines maximum number of rows needed in the Pascal's Triangle.

unsigned long long choose[N_MAX + 1][N_MAX + 1]; // Cached Pascal's Triangle.
static void makeTriangle();
#define swapInts(x, y) { x = x + y; y = x - y; x = x - y; }
#define min(x, y) (x < y ? x : y)

int currVariant = DEFAULT_VARIANT; 
static void switchVariant(int variant);

static int getRow(int idx);
static int getCol(int idx);

static POSITION constructPosition(int *indices, int hare_i, BOOLEAN haresTurn);
static void unpackPosition(POSITION position, int *indices, int *hare_i, BOOLEAN *haresTurn);

static MOVE constructMove(int destIdx, int srcIdx);
static void unpackMove(MOVE move, int *destIdx, int *srcIdx);

static void printBoard(int *indices, int hare_i);

/*************************************************************************
**
** Every variable declared here is only used in this file (game-specific)
**
**************************************************************************/

POSITION GetCanonicalPosition(POSITION);
STRING MoveToString(MOVE);
POSITION ActualNumberOfPositions(int variant);

/************************************************************************
**
** NAME: InitializeDatabases
**
** DESCRIPTION: Initialize the gDatabase, a global variable.
**
************************************************************************/

void InitializeGame() {
    gCanonicalPosition = GetCanonicalPosition;
    gMoveToStringFunPtr = &MoveToString;
    gActualNumberOfPositionsOptFunPtr = &ActualNumberOfPositions;
    kUsePureDraw = TRUE;

    makeTriangle();
    switchVariant(currVariant);
}

/************************************************************************
**
** NAME: DebugMenu
**
** DESCRIPTION: Menu used to debug internal problems. Does nothing if
** kDebugMenu == FALSE
**
************************************************************************/

void DebugMenu() {
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

static const char *variantDesc(int variant) {
    switch (variant) {
        case 0:
            return "(small, hounds first)";
        case 1:
            return "(small, hare first)";
        case 2:
            return "(medium, hounds first)";
        case 3:
            return "(medium, hare first)";
        case 4:
            return "(large, hounds first)";
        case 5:
            return "(large, hare first)";
        default:
            return "";
    }
}

void GameSpecificMenu() {
	char GetMyChar();
    do {
        printf("\n\t----- Game-specific options for %s -----\n\n", kGameName);
        for (int option = 1; option <= (SIZE_VARIANT_MAX << 1) + 2; ++option) {
            if (currVariant != option - 1) {
                printf("\t%d)\tSwitch variant from %s to %s.\n",
                    option, variantDesc(currVariant), variantDesc(option - 1));
            }
        }
        printf("\n\n\tb)\t(B)ack = Return to previous activity.\n");
		    printf("\n\nSelect an option: ");
        char c = GetMyChar();
        switch(c) {
        case '1': case '2': case '3': case '4': case '5': case '6':
            switchVariant(c - '1');
            break;
        case 'Q': case 'q':
            ExitStageRight();
            break;
        case 'H': case 'h':
            HelpMenus();
            break;
        case 'B': case 'b':
            return;
        default:
            printf("\nSorry, I don't know that option. Try another.\n");
            HitAnyKeyToContinue();
            break;
        }
    } while (TRUE);
}

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
** CALLS: 
**
************************************************************************/

static void sortIndices(int *indices, int modifiedIdx) {
    while (modifiedIdx > 0 && indices[modifiedIdx] > indices[modifiedIdx - 1]) {
        swapInts(indices[modifiedIdx - 1], indices[modifiedIdx]);
        --modifiedIdx;
    }
    while (modifiedIdx < 3 && indices[modifiedIdx] < indices[modifiedIdx + 1]) {
        swapInts(indices[modifiedIdx + 1], indices[modifiedIdx]);
        ++modifiedIdx;
    }
}

static int findHare(int *indices, int hareIdx) {
    int i = 0;
    while (i < 4 && indices[i] != hareIdx) {
        ++i;
    }
    return i;
}

POSITION DoMove(POSITION position, MOVE move) {
    int indices[4];
    int hare_i;
    BOOLEAN haresTurn;
    unpackPosition(position, indices, &hare_i, &haresTurn);
    int hareIdx = indices[hare_i];
    int destIdx, srcIdx;
    unpackMove(move, &destIdx, &srcIdx);

    int i = 0;
    while (i < 4 && indices[i] != srcIdx) {
        ++i;
    }
    if (i >= 4) {
        return kBadPosition;
    } else if (i == hare_i) {
        hareIdx = destIdx;
    }
    indices[i] = destIdx;
    sortIndices(indices, i);
    hare_i = findHare(indices, hareIdx);
    POSITION ret = constructPosition(indices, hare_i, !haresTurn);
    return ret;
}

/************************************************************************
**
** NAME: GetInitialPosition
**
** DESCRIPTION: Ask the user for an initial position for testing. Store
** it in the space pointed to by initialPosition;
**
** OUTPUTS: POSITION initialPosition : The position to fill.
**
************************************************************************/

POSITION GetInitialPosition() {
    int pos = GetMyInt();
    return pos;
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
    int destIdx, srcIdx;
    unpackMove(computersMove, &destIdx, &srcIdx);
    printf("%8s's move\t\t\t: [%2d -> %2d]\n", computersName, srcIdx, destIdx);
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
** CALLS: BOOLEAN ThreeInARow()
** BOOLEAN AllFilledIn()
** PositionToBlankOX()
**
************************************************************************/

VALUE Primitive(POSITION position) {
    /* Depending on the rule, hare wins if it reaches index 0 or is
       not to the right of any hound. Hounds win if the hare runs out of
       moves.
       Note that the hounds will never run out of moves, and there are no
       primitive winning positions because a player can never do some move
       and let their opponent win. */
    int indices[4];
    int hare_i;
    BOOLEAN haresTurn;
    VALUE ret = undecided;
    unpackPosition(position, indices, &hare_i, &haresTurn);
    if (indices[hare_i] == 0) {
        ret = lose;
    } else {
        MOVELIST *moves = GenerateMoves(position);
        if (!moves) {
            ret = lose;
        }
        FreeMoveList(moves);
    }
    return ret;
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
** CALLS: GetValueOfPosition()
** GetPrediction()
**
************************************************************************/

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn) {
    int indices[4];
    int hare_i;
    BOOLEAN haresTurn;
    unpackPosition(position, indices, &hare_i, &haresTurn);
    printBoard(indices, hare_i);
    printf("%s\n", GetPrediction(position, playerName, usersTurn));
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

MOVELIST *validDestIndices(int idx, BOOLEAN haresTurn) {
    MOVELIST *ret = NULL;
    int n = numCells(currVariant);
    /* Special case for cell 0: no backward moves available here for hounds. */
    if (idx == 0) {
        for (int i = 1; i < 4; ++i) {
            ret = CreateMovelistNode(i, ret);
        }
        return ret;
    }
    /* Special case for cell (numCells - 1): only backward moves are available
       for hounds. */
    if (idx == n - 1) {
        if (haresTurn) {
            for (int i = n - 4; i < n - 1; ++i) {
                ret = CreateMovelistNode(i, ret);
            }
        } /* Else do nothing and return empty list. */
        return ret;
    }
    int row = getRow(idx);
    /* Consider each possible move. */
    /* UP */
    if (row > 0) {
        ret = CreateMovelistNode(idx - 1, ret);
    }
    /* DOWN */
    if (row < 2) {
        ret = CreateMovelistNode(idx + 1, ret);
    }
    /* LEFT */
    if (haresTurn && idx != 1 && idx != 3) {
        ret = CreateMovelistNode(idx - 3 + (idx == 2), ret);
    }
    /* RIGHT */
    if (idx != n - 2 && idx != n - 4) {
        ret = CreateMovelistNode(idx + 3 - (idx == n - 3), ret);
    }
    /* The following are available only if idx is odd. */
    if (idx & 0b1) {
        /* RIGHT-UP */
        if (row > 0) {
            ret = CreateMovelistNode(idx + 2 - (idx == n - 2), ret);
        }
        /* RIGHT-DOWN */
        if (row < 2) {
            ret = CreateMovelistNode(idx + 4 - (idx == n - 4), ret);
        }
        /* LEFT-UP */
        if (haresTurn && row > 0) {
            ret = CreateMovelistNode(idx - 4 + (idx == 3), ret);
        }
        /* LEFT-DOWN */
        if (haresTurn && row < 2) {
            ret = CreateMovelistNode(idx - 2 + (idx == 1), ret);
        }
    }
    return ret;
}

static BOOLEAN contains(int *a, int size, int n) {
    for (int i = 0; i < size; ++i) {
        if (a[i] == n) {
            return TRUE;
        }
    }
    return FALSE;
}

MOVELIST *GenerateMoves(POSITION position) {
    int indices[4];
    int hare_i;
    BOOLEAN haresTurn;
    unpackPosition(position, indices, &hare_i, &haresTurn);
    MOVELIST *ret = NULL, *validDests = NULL, *last = NULL;
    if (haresTurn) {
        validDests = validDestIndices(indices[hare_i], TRUE);
        while (validDests != NULL) {
            last = validDests;
            validDests = validDests->next;
            if (!contains(indices, 4, last->move)) {
                ret = CreateMovelistNode(constructMove(last->move, indices[hare_i]), ret);
            }
            SafeFree(last);
        }
    } else {
        for (int i = 0; i < 4; ++i) {
            if (i == hare_i) {
                continue;
            }
            validDests = validDestIndices(indices[i], FALSE);
            while (validDests != NULL) {
                last = validDests;
                validDests = validDests->next;
                if (!contains(indices, 4, last->move)) {
                    ret = CreateMovelistNode(constructMove(last->move, indices[i]), ret);
                }
                SafeFree(last);
            }
        }
    }
    return ret;
}

/**************************************************/
/*********** SYMMETRY HANDLING BEGIN **************/
/**************************************************/

/************************************************************************
**
** NAME: GetCanonicalPosition
**
** DESCRIPTION: Go through all of the positions that are symmetrically
** equivalent and return the SMALLEST, which will be used
** as the canonical element for the equivalence set.
**
** INPUTS: POSITION position : The position return the canonical elt. of.
**
** OUTPUTS: POSITION : The canonical element of the set.
**
************************************************************************/

/* Sorts indices in descending order, assuming length 4. */
static void sort(int *indices) {
    if (indices[0] < indices[1]) swapInts(indices[0], indices[1]);
    if (indices[2] < indices[3]) swapInts(indices[2], indices[3]);
    if (indices[0] < indices[2]) swapInts(indices[0], indices[2]);
    if (indices[1] < indices[2]) swapInts(indices[1], indices[2]);
    if (indices[2] < indices[3]) swapInts(indices[2], indices[3]);
}

POSITION GetCanonicalPosition(POSITION position) {
    int indices[4];
    int hare_i, i;
    BOOLEAN haresTurn;
    unpackPosition(position, indices, &hare_i, &haresTurn);
    for (i = 0; i < 4; ++i) {
        int row = getRow(indices[i]);
        if (row == 0) {
            indices[i] += 2;
        } else if (row == 2) {
            indices[i] -= 2;
        }
    }
    int newHareIdx = indices[hare_i];
    sort(indices);
    hare_i = findHare(indices, newHareIdx);
    return min(position, constructPosition(indices, hare_i, haresTurn));
}

/**************************************************/
/*********** SYMMETRY HANDLING END ****************/
/**************************************************/

/************************************************************************
**
** NAME: GetAndPrintPlayersMove
**
** DESCRIPTION: This finds out if the player wanted an undo or abort or not.
** If so, return Undo or Abort and don't change theMove.
** Otherwise get the new theMove and fill the pointer up.
**
** INPUTS: POSITION *position : The position the user is at.
** MOVE *move : The move to fill with user's move.
** STRING playerName : The name of the player whose turn it is
**
** OUTPUTS: USERINPUT : Oneof( Undo, Abort, Continue )
**
** CALLS: ValidMove(MOVE, POSITION)
** BOOLEAN PrintPossibleMoves(POSITION) ...Always True!
**
************************************************************************/

USERINPUT GetAndPrintPlayersMove(POSITION position, MOVE *move, STRING playerName) {
	USERINPUT ret;
	do {
		printf("%8s's move [(u)ndo]/[srcIdx,destIdx] :  ", playerName);
		ret = HandleDefaultTextInput(position, move, playerName);
		if (ret != Continue) {
			return ret;
        }
	} while (TRUE);
	return Continue;
}

/************************************************************************
**
** NAME: ValidTextInput
**
** DESCRIPTION: Return TRUE iff the string input is of the right 'form'.
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
    STRING inputCopy = copyString(input);
    int tokensFound = 0;
    int destIdx, srcIdx;
    static STRING delim = " ,.-_";
    STRING nextToken = strtok(inputCopy, delim);
    if (nextToken) {
        srcIdx = atoi(nextToken);
        ++tokensFound;
        nextToken = strtok(NULL, delim);
    }
    if (nextToken) {
        destIdx = atoi(nextToken);
        ++tokensFound;
        nextToken = strtok(NULL, delim);
    }
    SafeFree(inputCopy);
    if (nextToken) {
        /* Malformed input. */
        return FALSE;
    } else if (tokensFound < 2) {
        /* Less than two indices entered. */
        return FALSE;
    }
    int n = numCells(currVariant);
    return srcIdx >= 0 && destIdx >= 0 && srcIdx < n && destIdx < n;
}

/************************************************************************
**
** NAME: ConvertTextInputToMove
**
** DESCRIPTION: Convert the string input to the internal move representation.
**
** INPUTS: STRING input : The string input the user typed.
**
** OUTPUTS: MOVE : The move corresponding to the user's input.
**
************************************************************************/

MOVE ConvertTextInputToMove(STRING input) {
    STRING inputCopy = copyString(input);
    int destIdx, srcIdx;
    static STRING delim = " ,.-_";
    STRING nextToken = strtok(inputCopy, delim);
    srcIdx = atoi(nextToken);
    nextToken = strtok(NULL, delim);
    destIdx = atoi(nextToken);
    SafeFree(inputCopy);
    return constructMove(destIdx, srcIdx);
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

void PrintMove(MOVE move) {
    STRING str = MoveToString(move);
	printf("%s", str);
	SafeFree(str);
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

STRING MoveToString(MOVE move) {
    STRING m = (STRING)SafeMalloc(12);
    int destIdx, srcIdx;
    unpackMove(move, &destIdx, &srcIdx);
    sprintf(m, "[%d %d]", srcIdx, destIdx);
    return m;
}

/************************************************************************
**
** NAME: NumberOfOptions
**
** DESCRIPTION: Return how many different variants are supported.
**
************************************************************************/
int NumberOfOptions() {
    /* There are (SIZE_VARIANT_MAX + 1) choices of board size, where each size
       has two variants: hounds first or hare first. */
    return (SIZE_VARIANT_MAX + 1) << 1;
}

/************************************************************************
**
** NAME: getOption
**
** DESCRIPTION: Return the variant ID of the variant currently being played.
**
************************************************************************/
int getOption() {
    return 1 + currVariant;
}

/************************************************************************
**
** NAME: setOption
**
** DESCRIPTION: Change variables according to what variant we want to switch to.
** 
** INPUT: The variant ID
**
************************************************************************/
void setOption(int option) {
    switchVariant(option - 1);
}

POSITION ActualNumberOfPositions(int variant) {
    // Ignoring for now.
    (void)variant;
    return 0;
}

POSITION InteractStringToPosition(STRING str) {
    enum UWAPI_Turn turn;
	unsigned int num_rows, num_columns; // Unused
	STRING board;
    int i, j = 3, n = numCells(currVariant), hare_i = 0;
    int indices[4];
	if (!UWAPI_Board_Regular2D_ParsePositionString(str, &turn, &num_rows, &num_columns, &board)) {
		// Failed to parse string
		return INVALID_POSITION;
	}
    for (i = 0; i < n; ++i) {
        switch (board[i]) {
        case '-':
            break;
        case 'd':
            if (j < 0) {
		        return INVALID_POSITION;
            }
            indices[j--] = i;
            break;
        case 'r':
            if (j < 0) {
		        return INVALID_POSITION;
            }
            indices[j] = i;
            hare_i = j--;
            break;
        default:
		    return INVALID_POSITION;
        }
    }
	SafeFreeString(board); // Free the string.
    BOOLEAN haresTurn = ((turn == UWAPI_TURN_A) && (currVariant & 0b1)) ||
        ((turn == UWAPI_TURN_B) && (!(currVariant & 0b1)));
    return constructPosition(indices, hare_i, haresTurn);
}

STRING InteractPositionToString(POSITION position) {
    int indices[4];
    int hare_i, i, j = 3, n = numCells(currVariant);
    BOOLEAN haresTurn;
    char *board = SafeMalloc(n + 1);
    unpackPosition(position, indices, &hare_i, &haresTurn);
    for (i = 0; i < n; ++i) {
        if (indices[j] == i) {
            board[i] = hare_i == j ? 'r' : 'd';
            --j;
        } else {
            board[i] = '-';
        }
    }
    board[n] = '\0';
    enum UWAPI_Turn turn = ((currVariant & 0b1) ^ haresTurn) ? UWAPI_TURN_B : UWAPI_TURN_A;
    STRING ret = UWAPI_Board_Regular2D_MakeBoardString(turn, n, board);
    SafeFree(board);
    return ret;
}

STRING InteractMoveToString(POSITION position, MOVE move) {
    int indices[4];
    int hare_i;
    BOOLEAN haresTurn;
    unpackPosition(position, indices, &hare_i, &haresTurn);
    int destIdx, srcIdx;
    unpackMove(move, &destIdx, &srcIdx);
    char sound = (haresTurn) ? 'r' : 'd';
    return UWAPI_Board_Regular2D_MakeMoveStringWithSound(srcIdx, destIdx, sound);
}

/* Algorithm by Sufian Latif,
   https://stackoverflow.com/a/11809815. */
static void makeTriangle() {
    int i, j;
    choose[0][0] = 1;
    for (i = 1; i <= N_MAX; ++i) {
        choose[i][0] = 1;
        for (j = 1; j <= i; ++j) {
            choose[i][j] = choose[i - 1][j - 1] + choose[i - 1][j];
        }
    }
}

static void switchVariant(int variant) {
    if (variant < 0 || variant > (SIZE_VARIANT_MAX << 1) + 1) {
        return;
    }
    currVariant = variant;
    /* An upper bound for the number of board configurations is 4*(num_cells choose 4):
       choose four cells to place all the characters, then choose one of the four cells
       to place the Hare. Need one more bit for turn. */
    int n = numCells(currVariant);
    gNumberOfPositions = choose[n][4] << 3;
    int indices[4] = {(n - 1), 3, 1, 0};
    gInitialPosition = constructPosition(indices, 0, currVariant & 0b1);
}

static int getRow(int idx) {
    if (idx == 0 || idx == numCells(currVariant) - 1) {
        return 1;
    }
    return (idx - 1) % 3;
}

static int getCol(int idx) {
    if (idx == 0) {
        return 0;
    }
    return (idx - 1) / 3 + 1;
}

/* Assumes INDICES is an array of 4 integers in strict descending order,
   each representing the index of an occupied cell on the board. Also
   assumes that HARE_I is an integer in range [0, 4), indicating which
   of the 4 indices is the index of the hare. */
static POSITION constructPosition(int *indices, int hare_i, BOOLEAN haresTurn) {
    unsigned long long N = 0;
    int i;
    for (i = 0; i < 4; ++i) {
        N += choose[indices[3 - i]][i + 1];
    }
    /* N must be no larger than 61 bits. */
    return (N << 3) + (hare_i << 1) + haresTurn;
}

/* Returns c_k. */
static int NToComboHelper(unsigned long long N, int k) {
    int i = k;
    unsigned long long curr = 0;
    while (curr <= N) {
        curr = choose[i++][k];
    }
    return i - 2;
}

/* Stores 4 occupied indices into INDICES in strictly descending order and sets
   HARE_I according to POSITION. */
static void unpackPosition(POSITION position, int *indices, int *hare_i, BOOLEAN *haresTurn) {
    unsigned long long N = position >> 3;
    *hare_i = (position >> 1) & 0b11;
    *haresTurn = position & 0b1;
    int i = 0, k = 4;
    while (k > 0) {
        indices[i] = NToComboHelper(N, k);
        N -= choose[indices[i++]][k--];
    }
}

static MOVE constructMove(int destIdx, int srcIdx) {
    return (destIdx << 8) + srcIdx;
}

static void unpackMove(MOVE move, int *destIdx, int *srcIdx) {
    *destIdx = move >> 8;
    *srcIdx = move - ((*destIdx) << 8);
}

char nextVertical(int i) {
    switch (i) {
    case 0:
        return '/';
    case 2:
        return '\\';
    case 1: case 3:
        return '|';
    default:
        return ' ';
    }
}

static void printBoard(int *indices, int hare_i) {
    int cols = ((currVariant >> 1) << 2) + 9;
    char board[5][cols + 1];
    /* Initialize board. */
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < cols; ++j) {
            board[i][j] = ' ';
        }
        board[i][cols] = '\0';
    }
    /* Add grid. */
    for (int i = 0; i < 5; i += 2) {
        for (int j = 3; j < cols - 3; j += 2) {
            board[i][j] = '-';
        }
    }
    board[2][1] = board[2][cols - 2] = '-';
    int next = 0;
    for (int j = 1; j < cols - 1; ++j) {
        board[1][j] = board[3][cols - 1 - j] = nextVertical(next);
        next = (next + 1) % 4;
    }
    /* Add characters based on pos. */
    for (int i = 0; i < 4; ++i) {
        int row = getRow(indices[i]);
        int col = getCol(indices[i]);
        board[row << 1][col << 1] = i == hare_i ? 'O' : 'X';
    }

    /* Print board. */
    for (int i = 0; i < 5; ++i) {
        printf("%s\n", board[i]);
    }
}
