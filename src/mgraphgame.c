/************************************************************************
**
** NAME:        m<Graph Game>.c
**
** DESCRIPTION: <This is a game based on an example graph with pure draw positions. This game is used to show that our system is capable of working with hgiher levels of draw-remoteness and draw levels than we encounter in existing pure draw games in the system (Fall 2023).> (use this spacing and case)
**
** AUTHOR:      Arihant Choudhary, Cameron Cheung
**
** DATE:        2023-11-16
**
************************************************************************/

#include "gamesman.h"

/* IMPORTANT GLOBAL VARIABLES */
CONST_STRING kAuthorName = "Arihant Choudhary, Cameron Cheung";
CONST_STRING kGameName = "Example Graph Game";
CONST_STRING kDBName = "graphgame";
POSITION gNumberOfPositions = 21;
POSITION gInitialPosition = 0;
BOOLEAN kPartizan = FALSE;
BOOLEAN kTieIsPossible = TRUE;
BOOLEAN kLoopy = TRUE;
BOOLEAN kSupportsSymmetries = FALSE;  

/* Likely you do not have to change these. */
POSITION GetCanonicalPosition(POSITION);
STRING MoveToString(MOVE);
POSITION kBadPosition = -1;
BOOLEAN kDebugDetermineValue = FALSE;
void *gGameSpecificTclInit = NULL;

/* You do not have to change these for now. */
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

/* You don't have to change this. */
void DebugMenu() {}
void SetTclCGameSpecificOptions(int theOptions[]) { (void)theOptions; }


/*********** BEGIN SOLVING FUNCIONS ***********/
int listOfMoves0[21][4] = {
    {1, 1}, {2, 0, 2}, {1, 3}, {1, 4}, {2, 6, 7}, {2, 3, 4}, {2, 5, 8},
    {2, 5, 8}, {1, 9}, {2, 8, 10}, {3, 9, 11, 12}, {1, 10}, {2, 13, 17},
    {1, 15}, {1, 12}, {2, 14, 16}, {1, 18}, {3, 11, 18, 20},
    {0}, {2, 16, 20}, {2, 16, 19}
};

int listOfMoves1[11][4] = {
    {0}, {1, 0}, {2, 1, 0}, {2, 2, 1}, {2, 3, 2}, {2, 4, 3}, {2, 5, 4}, {2, 6, 5},
    {2, 7, 6}, {2, 8, 7}, {2, 9, 8}
};

int listOfMoves2[16][4] = {
    {2, 1, 2}, {3, 2, 3, 4}, {1, 5}, {2, 7, 8}, {2, 8, 9}, {3, 6, 9, 10}, 
    {3, 5, 10, 11}, {2, 8, 12}, {2, 7, 13}, {2, 13, 14}, {1, 15}, {0},
    {0}, {0}, {0}, {0}
};

int (*listOfMoves)[4] = listOfMoves0;

/* Initialize any global variables or data structures needed before
solving or playing the game. */
void InitializeGame() {
    gCanonicalPosition = GetCanonicalPosition;
    gMoveToStringFunPtr = &MoveToString;

    kUsePureDraw = TRUE;
    kCombinatorial = FALSE;
    kLoopy = TRUE;
    setOption(getOption());
}

/* Return the hash value of the initial position. */
POSITION GetInitialPosition() {
    return gInitialPosition;
}

/* Return a linked list of moves. */
MOVELIST *GenerateMoves(POSITION position) {
    MOVELIST *moves = NULL;
    for (int i = 1; i <= listOfMoves[position][0]; i++) {
        moves = CreateMovelistNode(listOfMoves[position][i], moves);
    }
    return moves;
}

/* Return the position that results from making the
input move on the input position. */
POSITION DoMove(POSITION position, MOVE move) {
    return move;
}

/*****************************************************************
**  Determine whether the position is a primitive win,
**  primitive tie, primitive lose, or not primitive.
**  OUTPUTS: (VALUE) an enum; one of: (win, lose, tie, undecided)
**  See src/core/types.h for the value enum definition.
******************************************************************/
VALUE Primitive(POSITION position) {
    /* YOUR CODE HERE */
    if (listOfMoves == listOfMoves2) {
        if (position == 11) {
            return lose;
        } else if (position >= 12 && position <= 14) {
            return win;
        } else if (position == 15) {
            return tie;
        }
    } else if (listOfMoves == listOfMoves1) {
        if (position == 0) return lose;
    } else {
        if (position == 18) return lose;
    }
    return undecided;
}

/* Symmetry Handling: Return the canonical position. */
POSITION GetCanonicalPosition(POSITION position) {
    return position;
}

/*********** END SOLVING FUNCTIONS ***********/

/*********** BEGIN TEXTUI FUNCTIONS ***********/

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn) {
    printf("%llu\n", position);
}

void PrintComputersMove(MOVE computersMove, STRING computersName) {
    printf("%8s's move: %d\n", computersName, computersMove);
}

USERINPUT GetAndPrintPlayersMove(POSITION position, MOVE *move, STRING playerName) {
    USERINPUT ret;
    do {
        printf("%8s's move: ", playerName);

        ret = HandleDefaultTextInput(position, move, playerName);
        if (ret != Continue)
            return (ret);

    } while (TRUE);
    return (Continue); /* this is never reached, but lint is now happy */
}

/* Return whether the input text signifies a valid move. */
BOOLEAN ValidTextInput(STRING input) {
    return input[0] >= '0' && input[0] <= '9';
}

/* Assume the text input signifies a valid move. Return
the move hash corresponding to the move. */
MOVE ConvertTextInputToMove(STRING input) {
    return atoi(input);
}

/* Return the string representation of the move.
Ideally this matches with what the user is supposed to
type when they specify moves. */
STRING MoveToString(MOVE move) {
    STRING s = malloc(sizeof(char)*3);
    snprintf(s, 3, "%d", move);
    return s;
}

/* Basically just print the move. */
void PrintMove(MOVE move) {
    printf("%d", move);
}

/*********** END TEXTUI FUNCTIONS ***********/

/*********** BEGIN VARIANT FUNCTIONS ***********/

/* How many variants are you supporting? */
int NumberOfOptions() {
    return 3;
}

/* Return the current variant id. */
int getOption() {
    if (listOfMoves == listOfMoves0) {
        return 0;
    } else if (listOfMoves == listOfMoves1) {
        return 1;
    } else {
        return 2;
    }
}

/* The input is a variant id. This function sets any global variables
or data structures according to the variant specified by the variant id. */
void setOption(int option) {
    kCombinatorial = FALSE;
    kUsePureDraw = FALSE;
    kLoopy = TRUE;
    gInitialPosition = 0;
    if (option == 0) {
        kUsePureDraw = TRUE;
        listOfMoves = listOfMoves0;
        gNumberOfPositions = 21;
    } else if (option == 1) {
        kCombinatorial = TRUE;
        kLoopy = FALSE;
        gInitialPosition = 10;
        listOfMoves = listOfMoves1;
        gNumberOfPositions = 11;
    } else {
        listOfMoves = listOfMoves2;
        gNumberOfPositions = 16;
    }
}

void GameSpecificMenu() {
    char inp;
	while (TRUE) {
		printf("\n\n\n");
		printf("        ----- Game-specific options for Five Field Kono -----\n\n");
		printf("        Select an option:\n\n");
		printf("        0)      Graph with Interesting Pure Draw Levels\n");
		printf("        1)      10-to-0-by-1-or-2 Graph\n");
        printf("        2)      Graph with Various Primitive Values\n");
		printf("\nSelect an option: ");
		inp = GetMyChar();
		if (inp == '0') {
            setOption(0);
            return;
		} else if (inp == '1') {
            setOption(1);
            return;
		} else if (inp == '2') {
            setOption(2);
            return;
        }
		else if (inp == 'b' || inp == 'B') return;
		else {
			printf("Invalid input.\n");
		}
	}
}

/*********** END VARIANT-RELATED FUNCTIONS ***********/

/* Don't worry about these Interact functions below yet.
They are used for the AutoGUI which eventually we would
want to implement, but they are not needed for solving. */
POSITION InteractStringToPosition(STRING str) {
    str += 8;
    for (POSITION p = 0; p < gNumberOfPositions; p++) {
        if (str[p] == 'x') {
            return p;
        }
    }
    return 0;
}

STRING InteractPositionToString(POSITION position) {
    char board[gNumberOfPositions + 1];
    memset(board, '-', gNumberOfPositions);
    board[position] = 'x';
    board[gNumberOfPositions] = '\0';
    return UWAPI_Board_Regular2D_MakeBoardString(UWAPI_TURN_C, gNumberOfPositions + 1, board);
}

STRING InteractMoveToString(POSITION position, MOVE move) {
    return UWAPI_Board_Regular2D_MakeMoveStringWithSound((unsigned int) position, (unsigned int) move, 'x');
}