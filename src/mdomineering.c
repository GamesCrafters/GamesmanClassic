/************************************************************************
**
** NAME:        mdomineering.c
**
** DESCRIPTION: Domineering
**
** AUTHOR:      Cameron Cheung
**
** DATE:        2023-12-08
**
************************************************************************/

#include "gamesman.h"

CONST_STRING kAuthorName = "Cameron Cheung";
CONST_STRING kGameName = "Domineering";
CONST_STRING kDBName = "domineering";
POSITION gNumberOfPositions = 0;
POSITION gInitialPosition = 0;
BOOLEAN kPartizan = TRUE;
BOOLEAN kTieIsPossible = FALSE;
BOOLEAN kLoopy = FALSE;
BOOLEAN kSupportsSymmetries = TRUE;

POSITION GetCanonicalPosition(POSITION);
POSITION kBadPosition = -1;
BOOLEAN kDebugDetermineValue = FALSE;
void *gGameSpecificTclInit = NULL;
BOOLEAN kGameSpecificMenu = TRUE;
BOOLEAN kDebugMenu = FALSE;

/* These variables are not needed for solving but if you have time
after you're done solving the game you should initialize them
with something helpful. */
CONST_STRING kHelpGraphicInterface = "";
CONST_STRING kHelpTextInterface = "";
CONST_STRING kHelpOnYourTurn = "";
CONST_STRING kHelpStandardObjective = "";
CONST_STRING kHelpReverseObjective = "";
CONST_STRING kHelpTieOccursWhen = /* Should follow 'A Tie occurs when... */ "";
CONST_STRING kHelpExample = "";

void DebugMenu() {}
void SetTclCGameSpecificOptions(int theOptions[]) { (void)theOptions; }

/* Tier Functions for TierGamesman Support */
TIERLIST *getTierChildren(TIER tier);
TIERPOSITION numberOfTierPositions(TIER tier);
POSITION reflectOverXAxis(POSITION bitBoard);
POSITION reflectOverYAxis(POSITION bitBoard);
void PositionStringDoMove(char *parentPositionString, MOVE move, char *childAutoGUIPositionStringBuffer);

int sideLength = 6;
int boardSize = 36; // The number of 1x1 spaces on the board.

/*********** BEGIN BASIC SOLVING FUNCIONS ***********/

void printBitBoard(POSITION bitBoard) {
    for (int i = 0; i < sideLength; i++) {
        for (int j = 0; j < sideLength; j++) {
            int idx = i * sideLength + j;
            printf("%c", bitBoard & (((POSITION) 1) << idx) ? '-' : 'X');
        }
        printf("\n");
    }
}

POSITION combinations[37][37];

POSITION unhash(POSITION position, BOOLEAN *isP2Turn) {
    POSITION bitBoard = (((POSITION) 1) << (boardSize + 1)) - 1;
	TIER tier; TIERPOSITION tierPosition;
	gUnhashToTierPosition(position, &tierPosition, &tier);
    POSITION total = 0, q = 0;
    int m = 0;
    int tierDoubled = ((int) tier) << 1;
    for (int i = 0; i < tierDoubled; i++) {
        for (int sw = m; sw < boardSize; sw++) {
            q = combinations[boardSize - 1 - sw][tierDoubled - i - 1];
            if (total + q > tierPosition) {
                bitBoard ^= ((POSITION) 1) << sw;
                m = sw + 1;
                break;
            }
            total += q;
        }
    }
    *isP2Turn = tier & 1;
    return bitBoard;
}

POSITION hash(POSITION bitBoard) {
    POSITION tierPosition = 0;
    int tierDoubled = 0;
    int selectedIdxs[boardSize];
    int i;
    for (i = 0; i < boardSize; i++) {
        if (!((bitBoard >> i) & 1)) {
            selectedIdxs[tierDoubled++] = i;
        }
    }

    int m = 0;
    for (i = 0; i < tierDoubled; i++) {
        for (int sw = m; sw < selectedIdxs[i]; sw++) {
            tierPosition += combinations[boardSize - 1 - sw][tierDoubled - i - 1];
        }
        m = selectedIdxs[i] + 1;
    }
    return gHashToWindowPosition(tierPosition, (TIER) (tierDoubled >> 1));
}

/* Initialize any global variables or data structures needed before
solving or playing the game. */
void InitializeGame() {
    /* FOR THE PURPOSES OF INTERACT. FEEL FREE TO CHANGE IF SOLVING. */ 
	if (gIsInteract) {
		gLoadTierdbArray = FALSE; // SET TO TRUE IF SOLVING
	}
	/********************************/
    gCanonicalPosition = GetCanonicalPosition;
    gTierChildrenFunPtr = &getTierChildren;
    gNumberOfTierPositionsFunPtr = &numberOfTierPositions;
    gPositionStringDoMoveFunPtr = &PositionStringDoMove;
    gInitialTierPosition = gInitialPosition;
    kSupportsTierGamesman = TRUE;
    kExclusivelyTierGamesman = TRUE;
    gInitialTier = 0;
    gSymmetries = TRUE;

    for (int n = 0; n < 37; n++) { // init combinations array
        for (int r = 0; r <= n; r++) {
            if (n == r || r == 0) {
                combinations[n][r] = 1;
            } else {
                combinations[n][r] = combinations[n - 1][r] + combinations[n - 1][r - 1];
            }
        }
    }
}

/* Return the hash value of the initial position within the initial tier. */
POSITION GetInitialPosition() {
    return 0;
}

/* Return a linked list of moves. A move is encoded as the index of the top square
of a vertical piece being placed or by the left square of a horizontal piece being
placed. If the nth bit of the bitBoard is set, that means that index n (space n) 
of the board is unoccupied. */
MOVELIST *GenerateMoves(POSITION position) {
    BOOLEAN isP2Turn; int i;
    POSITION bitBoard = unhash(position, &isP2Turn);
    MOVELIST *moves = NULL;
    if (isP2Turn) { // P2's Turn. Check for available left squares for a new horizontal piece.
        for (int row = 0; row < sideLength; row++) {
            for (int col = 0; col < sideLength - 1; col++) { // Don't need to check last column.
                i = row * sideLength + col;
                if ((bitBoard & (((POSITION) 1) << i)) && (bitBoard & (((POSITION) 1) << (i + 1)))) {
                    moves = CreateMovelistNode(i, moves);
                }
            }
        }
    } else { // P1's Turn. Check for available top squares for a new vertical piece.
        for (i = 0; i < boardSize - sideLength; i++) { // Don't need to check last row.
            if ((bitBoard & (((POSITION) 1) << i)) && (bitBoard & (((POSITION) 1) << (i + sideLength)))) {
                moves = CreateMovelistNode(i, moves);
            }
        }
    }
    return moves;
}

/* Return the position that results from making the
input move on the input position. */
POSITION DoMove(POSITION position, MOVE move) {
    BOOLEAN isP2Turn;
    POSITION bitBoard = unhash(position, &isP2Turn);
    bitBoard ^= ((POSITION) 1) << move;
    if (isP2Turn) { // P2 move, horizontal
        bitBoard ^= ((POSITION) 1) << (move + 1);
    } else { // P1 move, vertical
        bitBoard ^= ((POSITION) 1) << (move + sideLength);
    }
    return hash(bitBoard);
}

/*****************************************************************
**  Determine whether the position is a primitive win,
**  primitive tie, primitive lose, or not primitive.
**  OUTPUTS: (VALUE) an enum; one of: (win, lose, tie, undecided)
**  See src/core/types.h for the value enum definition.
******************************************************************/
VALUE Primitive(POSITION position) {
    MOVELIST *moves = GenerateMoves(position);
    if (moves == NULL) return lose;
    FreeMoveList(moves);
    return undecided;
}

POSITION reflectOverXAxis(POSITION bitBoard) {
    POSITION symmetricBitBoard = 0;
    POSITION mask = (1 << sideLength) - 1;
    for (int i = 0; i < sideLength; i++) {
        symmetricBitBoard |= ((bitBoard >> (sideLength * i)) & mask) << ((sideLength - i - 1) * sideLength);
    }
    return symmetricBitBoard;
}

POSITION reflectOverYAxis(POSITION bitBoard) {
    POSITION symmetricBitBoard = 0;
    POSITION mask = 0;
    if (sideLength == 6) {
        mask = 0b1000001000001000001000001000001;
    } else if (sideLength == 5) {
        mask = 0b100001000010000100001;
    } else {
        mask = 0b1000100010001;
    }

    for (int i = 0; i < sideLength; i++) {
        symmetricBitBoard |= ((bitBoard >> i) & mask) << (sideLength - i - 1);
    }
    return symmetricBitBoard;
}

/* There are four symmetry transformations we consider here, generated by an x-axis reflection
and a y-axis reflection. We acknowledge that you can transpose the board to get the opposite
value, but Classic doesn't have a way to implement this kind of 
symmetry-and-value transformation. */
POSITION GetCanonicalPosition(POSITION position) {
    BOOLEAN isP2Turn;
    POSITION bitBoard = unhash(position, &isP2Turn);
    
    POSITION symmetricBitBoard = reflectOverXAxis(bitBoard);
    POSITION symmetricHash = hash(symmetricBitBoard);
    if (symmetricHash < position) {
        position = symmetricHash;
    }

    symmetricBitBoard = reflectOverYAxis(bitBoard);
    symmetricHash = hash(symmetricBitBoard);
    if (symmetricHash < position) {
        position = symmetricHash;
    }

    symmetricBitBoard = reflectOverXAxis(symmetricBitBoard);
    symmetricHash = hash(symmetricBitBoard);
    if (symmetricHash < position) {
        position = symmetricHash;
    }
    
    return position;
}

/*********** END BASIC SOLVING FUNCTIONS ***********/

/*********** BEGIN TIER/UNDOMOVE FUNCTIONS ***********/

/*
    Return a linked list of child tiers of this tier. 
    `tier` gives us the number of pieces placed so far.
    There is space for another piece as long as `tier`
    is less than the half of `boardSize` (the number of
    1x1 spaces on the board).
*/
TIERLIST *getTierChildren(TIER tier) {
    if (tier < boardSize >> 1) {
        return CreateTierlistNode(tier + 1, NULL);
    }
    return NULL;
}

/* The size of a tier's hash space. */
POSITION numberOfTierPositions(TIER tier) {
    return combinations[boardSize][tier << 1];
}

/*********** END TIER/UNDOMOVE FUNCTIONS ***********/

/*********** BEGIN TEXTUI FUNCTIONS ***********/

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn) {
    BOOLEAN isP2Turn;
    POSITION bitBoard = unhash(position, &isP2Turn);
    printf("\n");
    for (int i = 0; i < sideLength; i++) {
        printf("    %c ", '1' + i);
        for (int j = 0; j < sideLength; j++) {
            int idx = i * sideLength + j;
            printf("%c", (bitBoard & (((POSITION) 1) << idx)) ? '-' : 'X');
        }
        printf("\n");
    }
    printf("      ");
    for (int j = 0; j < sideLength; j++) {
        printf("%c", 'a' + j);
    }
    printf("      Player %d's Turn       %s\n\n", isP2Turn ? 2 : 1, GetPrediction(position, playerName, usersTurn));
}

USERINPUT GetAndPrintPlayersMove(POSITION position, MOVE *move, STRING playerName) {
    USERINPUT ret;
    do {
        printf("%8s's move: ", playerName);
        ret = HandleDefaultTextInput(position, move, playerName);
        if (ret != Continue) return (ret);

    } while (TRUE);
    return (Continue); /* this is never reached, but lint is now happy */
}

/* Return whether the input text signifies a valid move. */
BOOLEAN ValidTextInput(STRING input) {
    return input[0] >= 'a' && input[0] < 'a' + sideLength && input[1] >= '1' && input[1] < '1' + sideLength;
}

/* Assume the text input signifies a valid move. Return
the move hash corresponding to the move. */
MOVE ConvertTextInputToMove(STRING input) {
    return (input[1] - '1') * sideLength + (input[0] - 'a');
}

/* Return the string representation of the move in heap space.
Ideally this matches with what the user is supposed to
type when they specify moves. */
void MoveToString(MOVE move, char *moveStringBuffer) {
    moveStringBuffer[0] = move % sideLength + 'a';
    moveStringBuffer[1] = move / sideLength + '1';
    moveStringBuffer[2] = '\0';
}

/* Basically just print the move. */
void PrintMove(MOVE move) {
    char moveStringBuffer[5];
    MoveToString(move, moveStringBuffer);
    printf("%s", moveStringBuffer);
}

void PrintComputersMove(MOVE computersMove, STRING computersName) {
    printf("%s's move: ", computersName);
    PrintMove(computersMove);
}

/*********** END TEXTUI FUNCTIONS ***********/

/*********** BEGIN VARIANT FUNCTIONS ***********/

/* How many variants we support. We support 4x4, 5x5, and 6x6. */
int NumberOfOptions() {
    return 3;
}

/* Return the current variant id. */
int getOption() {
    return sideLength - 4;
}

/* The input is a variant id. This function sets any global variables
or data structures according to the variant specified by the variant id. */
void setOption(int option) { 
    gSymmetries = TRUE;
    sideLength = option + 4;
    boardSize = sideLength * sideLength;
}

/* For implementing more than one variant */
void GameSpecificMenu() {
    char inp;
    while (TRUE) {
        printf("\n\n\n");
        printf("        ----- Game-specific options for Domineering -----\n\n");
        printf("        Select an option:\n\n");
        printf("        4)      4x4 Board\n");
        printf("        5)      5x5 Board\n");
        printf("        6)      6x6 Board\n");
        printf("        b)      (B)ack = Return to previous activity.\n\n\n");
        printf("Select an option: ");
        inp = GetMyChar();
        if (inp >= '4' && inp <= '6') {
            setOption(inp - '4');
            return;
        } else if (inp == 'b' || inp == 'B') {
            return;
        } else {
            printf("Invalid input.\n");
        }
    }
}

/*********** END VARIANT-RELATED FUNCTIONS ***********/

/* Why this function? Answer:
Our internal representation of the position ignores whether an occupied
space is a blue or red piece. (See StringToPosition). 
We can't convert from position to string using PositionToAutoGUIString, aside from 
the initial position, because the information about where the blue pieces and red 
pieces were is lost. 
This function allows us to get the child position string representation after 
applying a move to the parent string representation. Outside of this code,
we still get the hash representation of the child position to query the database with,
but this function is for the sake of getting the AutoGUI to show the original
red and blue piece placements. */
void PositionStringDoMove(char *parentPositionString, MOVE move, char *childAutoGUIPositionStringBuffer) {
    memcpy(childAutoGUIPositionStringBuffer, parentPositionString, boardSize + 2);
    childAutoGUIPositionStringBuffer[boardSize + 3] = '\0';
    if (parentPositionString[0] == '1') {
        childAutoGUIPositionStringBuffer[move + 2] = 'u';
        childAutoGUIPositionStringBuffer[move + sideLength + 2] = 'd';
        childAutoGUIPositionStringBuffer[0] = '2';
    } else {
        childAutoGUIPositionStringBuffer[move + 2] = 'l';
        childAutoGUIPositionStringBuffer[move + 1 + 2] = 'r';
        childAutoGUIPositionStringBuffer[0] = '1';
    }
}

POSITION StringToPosition(char *positionString) {
	int turn;
	char *board;
	if (ParseAutoGUIFormattedPositionString(positionString, &turn, &board)) {
        POSITION bitBoard = 0;
        TIER tierDoubled = 0;
        for (int i = 0; i < boardSize; i++) {
            if (board[i] == '-') {
                bitBoard |= ((POSITION) 1) << i;
            } else {
                tierDoubled++;
            }
        }
        gInitializeHashWindow(tierDoubled >> 1, FALSE);
        return hash(bitBoard);
	}
	return NULL_POSITION;
}

/* This function will only be called on the initial position because we have
implemented gPositionStringDoMoveFunPtr. */
void PositionToAutoGUIString(POSITION position, char *autoguiPositionStringBuffer) {
	(void) position;
    char board[boardSize + 1];
    memset(board, '-', boardSize * sizeof(char));
    board[boardSize] = '\0';
    AutoGUIMakePositionString(1, board, autoguiPositionStringBuffer);
}

void MoveToAutoGUIString(POSITION position, MOVE move, char *autoguiMoveStringBuffer) {
    /* We are breaking the abstraction barrier a bit in order to
    determine just whose turn it is. 
    There are `boardSize` center coordinates for the piece halves,
    `boardSize-sideLength` center coordinates for the vertical buttons,
    and `boardSize-sideLength` center coordinates for the horizontal buttons */
    TIER tier; TIERPOSITION tierposition;
	gUnhashToTierPosition(position, &tierposition, &tier);
    char token; int to = boardSize;
    if (tier & 1) { // P2's turn, horizontal
        token = 'h';
        to += (move / sideLength) * (sideLength - 1) + (move % sideLength);
    } else { // P1's turn, vertical
        token = 'v';
        to += move + boardSize - sideLength;
    }
    AutoGUIMakeMoveButtonStringA(token, to, 'x', autoguiMoveStringBuffer);
}