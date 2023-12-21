/************************************************************************
**
** NAME:        mchomp.c
**
** DESCRIPTION: Chomp
**
** AUTHOR:      Cameron Cheung
**
** DATE:        2023-11-29
**
************************************************************************/

#include "gamesman.h"

CONST_STRING kAuthorName = "Cameron Cheung";
CONST_STRING kGameName = "Chomp";
CONST_STRING kDBName = "chomp";
POSITION gNumberOfPositions = 330;
POSITION gInitialPosition = 329;
BOOLEAN kPartizan = FALSE;
BOOLEAN kTieIsPossible = FALSE;
BOOLEAN kLoopy = FALSE;
BOOLEAN kSupportsSymmetries = FALSE;

POSITION GetCanonicalPosition(POSITION);
STRING MoveToString(MOVE);
POSITION kBadPosition = -1;
BOOLEAN kDebugDetermineValue = FALSE;
void *gGameSpecificTclInit = NULL;
BOOLEAN kGameSpecificMenu = TRUE;
BOOLEAN kDebugMenu = FALSE;

CONST_STRING kHelpGraphicInterface = "";
CONST_STRING kHelpTextInterface = "";
CONST_STRING kHelpOnYourTurn = "";
CONST_STRING kHelpStandardObjective = "";
CONST_STRING kHelpReverseObjective = "";
CONST_STRING kHelpTieOccursWhen = /* Should follow 'A Tie occurs when... */ "";
CONST_STRING kHelpExample = "";

void DebugMenu() {}
void SetTclCGameSpecificOptions(int theOptions[]) { (void)theOptions; }

int numRows = 4;
int numCols = 7;

/*********** BEGIN SOLVING FUNCIONS ***********/

POSITION fact[21] = {1, 1, 2, 6, 24, 120, 720, 5040, 40320, 362880, 3628800, 39916800, 479001600, 6227020800, 87178291200, 
1307674368000, 20922789888000, 355687428096000, 6402373705728000, 121645100408832000, 2432902008176640000};

POSITION nCr2(int n, int r) {
    // WARNING: I DID NOT DOUBLE-CHECK IF THIS OVERFLOWS FOR CERTAIN
    // numRows and numCols values.
    return fact[n] / (fact[r] * fact[n - r]);
}

void unhash(POSITION position, int *rowSizes) {
    POSITION prevThreshold, nextThreshold;
    for (int i = 0; i < numRows; i++) {
        prevThreshold = 0;
        for (int j = 0; j < numCols + 1; j++) {
            nextThreshold = prevThreshold + nCr2(j + numRows - i - 1, j);
            if (position < nextThreshold) {
                rowSizes[i] = j;
                position -= prevThreshold;
                break;
            }
            prevThreshold = nextThreshold;
        }
    }
}

POSITION hash(int *rowSizes) {
    POSITION total = 0;
    for (int i = 0; i < numRows; i++) {
        for (int j = 0; j < rowSizes[i]; j++) {
            total += nCr2(j + numRows - i - 1, j);
        }
    }
    return total;
}

/* Initialize any global variables or data structures needed before
solving or playing the game. */
void InitializeGame() {
    gCanonicalPosition = GetCanonicalPosition;
    gMoveToStringFunPtr = &MoveToString;
    kCombinatorial = TRUE;
}

/* Return the hash value of the initial position. */
POSITION GetInitialPosition() {
    return gNumberOfPositions - 1;
}

/* Return a linked list of moves. */
MOVELIST *GenerateMoves(POSITION position) {
    MOVELIST *moves = NULL;
    int rowSizes[numRows];
    unhash(position, rowSizes);
    for (int row = 0; row < numRows; row++) {
        for (int col = 0; col < rowSizes[row]; col++) {
            if (row > 0 || col > 0) {
                moves = CreateMovelistNode(row * numCols + col, moves);
            }
        }
    }
    return moves;
}

/* Return the position that results from making the
input move on the input position. */
POSITION DoMove(POSITION position, MOVE move) {
    int rowSizes[numRows];
    unhash(position, rowSizes);
    int row = move / numCols;
    int col = move % numCols;

    for (int i = row; i < numRows; i++) {
        if (col < rowSizes[i]) {
            rowSizes[i] = col;
        }
    }
    return hash(rowSizes);
}

/*****************************************************************
**  Determine whether the position is a primitive win,
**  primitive tie, primitive lose, or not primitive.
**  OUTPUTS: (VALUE) an enum; one of: (win, lose, tie, undecided)
**  See src/core/types.h for the value enum definition.
******************************************************************/
VALUE Primitive(POSITION position) {
    int rowSizes[numRows];
    unhash(position, rowSizes);
    if (rowSizes[0] == 1 && (numRows == 1 || rowSizes[1] == 0)) {
        return lose;
    }
    return undecided;
}

POSITION GetCanonicalPosition(POSITION position) {
    return position;
}

/*********** END SOLVING FUNCTIONS ***********/

/*********** BEGIN TEXTUI FUNCTIONS ***********/

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn) {
    /* THIS ONE IS MOST IMPORTANT FOR YOUR DEBUGGING */
    /* YOUR CODE HERE */
    int rowSizes[numRows];
    unhash(position, rowSizes);
    printf("\n");
    for (int i = numRows - 1; i >= 0; i--) {
        printf("    ");
        for (int j = 0; j < rowSizes[i]; j++) {
            if (i == 0 && j == 0) {
                printf("0");
            } else {
                printf("X");
            }
        }
        if (i == 0) {
            printf("     %s", GetPrediction(position, playerName, usersTurn));
        }
        printf("\n");
    }
    printf("\n");
}

void PrintComputersMove(MOVE computersMove, STRING computersName) {
    STRING moveString = MoveToString(computersMove);
    printf("%8s's move: %s", computersName, moveString);
    SafeFree(moveString);
}

USERINPUT GetAndPrintPlayersMove(POSITION position, MOVE *move,
                                 STRING playerName) {
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
    return input[0] >= 'A' && input[0] <= 'Z' && atoi(input + 1);
}

/* Assume the text input signifies a valid move. Return
the move hash corresponding to the move. */
MOVE ConvertTextInputToMove(STRING input) {
    int row = atoi(input + 1) - 1;
    int col = input[0] - 'A';
    return row * numCols + col;
}

/* Return the string representation of the move.
Ideally this matches with what the user is supposed to
type when they specify moves. */
STRING MoveToString(MOVE move) {
    int rowOneIndexed = move / numCols + 1;
    int col = move % numCols;
    STRING moveString = SafeMalloc(4);
    moveString[0] = 'A' + col;
    if (rowOneIndexed < 10) {
        moveString[1] = rowOneIndexed + '0';
        moveString[2] = '\0';
    } else {
        moveString[1] = (rowOneIndexed / 10) + '0';
        moveString[2] = (rowOneIndexed % 10) + '0';
        moveString[3] = '\0';
    }
    return moveString;
}

/* Basically just print the move. */
void PrintMove(MOVE move) {
    /* YOUR CODE HERE */
    /* If you malloc anything, make sure to free it! */
    STRING moveString = MoveToString(move);
    printf("%s", moveString);
    SafeFree(moveString);
}

/*********** END TEXTUI FUNCTIONS ***********/

/*********** BEGIN VARIANT FUNCTIONS ***********/

/* How many variants are you supporting? */
int NumberOfOptions() {
    return 100; // We support up to 10 rows and 10 columns
}

/* Return the current variant id. */
int getOption() {
    return (numRows - 1) * 10 + numCols - 1;
}

/* The input is a variant id. This function sets any global variables
or data structures according to the variant specified by the variant id. */
void setOption(int option) {
    numRows = (option / 10) + 1;
    numCols = (option % 10) + 1;
    gNumberOfPositions = nCr2(numRows + numCols, numCols);
    gInitialPosition = gNumberOfPositions - 1; 
}

void GameSpecificMenu() {
    char buff[4];
    int num;
	while (TRUE) {
		printf("\n\n\n");
		printf("        ----- Game-specific options for Chomp -----\n\n");
		printf("        Enter a number of rows (1-10): ");
        fgets(buff, 4, stdin);
        if (buff[0] == 'b' || buff[0] == 'B') {
            return;
        }
        num = atoi(buff);
        if (num == 0 || num > 10) {
            printf("\nInvalid input.\n");
            continue;
        } else {
            numRows = num;
        }

        printf("\n        Enter a number of colums (1-10): ");
        fgets(buff, 4, stdin);
        if (buff[0] == 'b' || buff[0] == 'B') {
            return;
        }
        num = atoi(buff);
        if (num == 0 || num > 10) {
            printf("\nInvalid input.\n");
            continue;
        } else {
            numCols = num;
        }
        setOption(getOption());
        return;
    }
}

/*********** END VARIANT-RELATED FUNCTIONS ***********/

/* Don't worry about these Interact functions below yet.
They are used for the AutoGUI which eventually we would
want to implement, but they are not needed for solving. */
POSITION InteractStringToPosition(STRING str) {
    str += 8;
    int rowSizes[numRows];
    int rowSize;

    for (int i = 0; i < numRows; i++) {
        rowSize = 0;
        for (rowSize = 0; rowSize < numCols; rowSize++) {
            if (str[rowSize] == '-') {
                break;
            }
        }
        rowSizes[i] = rowSize;
        str += numCols;
    }
    return hash(rowSizes);
}

STRING InteractPositionToString(POSITION position) {
    char board[numRows * numCols + 1];
    memset(board, '-', numRows * numCols);

    int rowSizes[numRows];
    unhash(position, rowSizes);
    for (int i = 0; i < numRows; i++) {
        for (int j = 0; j < numCols; j++) {
            if (j < rowSizes[i]) {
                board[i * numCols + j] = 'x';
            }
        }
    }

    board[0] = 'p';
    board[numRows * numCols] = '\0';
    return UWAPI_Board_Regular2D_MakeBoardString(UWAPI_TURN_C, numRows * numCols + 1, board);
}

STRING InteractMoveToString(POSITION position, MOVE move) {
    return UWAPI_Board_Regular2D_MakeAddStringWithSound('t', move, 'x');
}