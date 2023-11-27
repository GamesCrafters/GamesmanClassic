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
CONST_STRING kGameName = "Example Graph Game for Draw Remoteness/Level Demo"; //  use this spacing and case
CONST_STRING kDBName = "graphgame";     // use this spacing and case
POSITION gNumberOfPositions = 21;           // TODO: Put your number of positions upper bound here.
POSITION gInitialPosition = 0;             // TODO: Put the hash value of the initial position.
BOOLEAN kPartizan = FALSE;                 // TODO: Is the game PARTIZAN i.e. given a board does each player have a different set of moves available to them?
BOOLEAN kTieIsPossible = TRUE;            // TODO: Is a tie or draw possible?
BOOLEAN kLoopy = TRUE;                    // TODO: Is this game loopy?
BOOLEAN kSupportsSymmetries = FALSE;       // TODO: Whether symmetries are supported (i.e. whether the GetCanonicalPosition is implemented)

/* Likely you do not have to change these. */
POSITION GetCanonicalPosition(POSITION);
STRING MoveToString(MOVE);
POSITION kBadPosition = -1;
BOOLEAN kDebugDetermineValue = FALSE;
void *gGameSpecificTclInit = NULL;

/* You do not have to change these for now. */
BOOLEAN kGameSpecificMenu = FALSE;
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
/* Ignore this function. */
void SetTclCGameSpecificOptions(int theOptions[])
{
    (void)theOptions;
}
/* Do not worry about this yet because you will only be supporting 1 variant for now. */
void GameSpecificMenu() {}

/*********** BEGIN SOLVING FUNCIONS ***********/
int listOfMoves[][4] = {
    {1, 1}, //0
    {2, 0, 2}, // 1
    {1, 3}, // 2
    {1, 4}, //3
    {2, 6, 7}, //4
    {2, 3, 4}, // 5
    {2, 5, 8}, //6
    {2, 5, 8}, //7
    {1, 9}, // 8
    {2, 8, 10}, // 9
    {3, 9, 11, 12}, //10
    {1, 10}, //11
    {2, 13, 17}, //12
    {1, 15}, // 13
    {1, 12}, //14
    {2, 14, 16}, //15
    {1, 18}, //16
    {3, 11, 18, 20}, //17
    {0}, //18 Loss State
    {2, 16, 20}, //19
    {2, 16, 19} //20
    };

/* TODO: Add a hashing function and unhashing function, if needed. */

/* Initialize any global variables or data structures needed before
solving or playing the game. */
void InitializeGame()
{
    gCanonicalPosition = GetCanonicalPosition;
    gMoveToStringFunPtr = &MoveToString;

    /* YOUR CODE HERE */
    kUsePureDraw = TRUE;

}

/* Return the hash value of the initial position. */
POSITION GetInitialPosition()
{
    /* YOUR CODE HERE */
    return 0;
}

/* Return a linked list of moves. */
MOVELIST *GenerateMoves(POSITION position)
{
    MOVELIST *moves = NULL;
    /* YOUR CODE HERE
       To add to the linked list, do
       moves = CreateMovelistNode(<the move you're adding>, moves);
       See the function CreateMovelistNode in src/core/misc.c
    */
    for (int i = 1; i <= listOfMoves[position][0]; i++)
    {
        moves = CreateMovelistNode(listOfMoves[position][i], moves);
    }

    return moves;
}

/* Return the position that results from making the
input move on the input position. */
POSITION DoMove(POSITION position, MOVE move)
{
    /* YOUR CODE HERE */
    return move;
}

/*****************************************************************
**  Determine whether the position is a primitive win,
**  primitive tie, primitive lose, or not primitive.
**  OUTPUTS: (VALUE) an enum; one of: (win, lose, tie, undecided)
**  See src/core/types.h for the value enum definition.
******************************************************************/
VALUE Primitive(POSITION position)
{
    /* YOUR CODE HERE */
    if (position == 18) {
        return lose;
    } else {
        return undecided;
    }
}

/* Symmetry Handling: Return the canonical position. */
POSITION GetCanonicalPosition(POSITION position)
{
    /* YOUR CODE HERE */
    return position;
}

/*********** END SOLVING FUNCTIONS ***********/

/*********** BEGIN TEXTUI FUNCTIONS ***********/

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn)
{
    /* THIS ONE IS MOST IMPORTANT FOR YOUR DEBUGGING */
    /* YOUR CODE HERE */
    printf("%llu", position);
    printf("\n");

}

void PrintComputersMove(MOVE computersMove, STRING computersName)
{
    /* YOUR CODE HERE */
    printf("%d", computersMove);
    printf("\n");
}

USERINPUT GetAndPrintPlayersMove(POSITION position, MOVE *move, STRING playerName)
{
    USERINPUT ret;
    do
    {
        /* List of available moves */
        // Modify the player's move prompt as you wish
        printf("%8s's move: ", playerName);

        ret = HandleDefaultTextInput(position, move, playerName);
        if (ret != Continue)
            return (ret);

    } while (TRUE);
    return (Continue); /* this is never reached, but lint is now happy */
}

/* Return whether the input text signifies a valid move. */
BOOLEAN ValidTextInput(STRING input)
{
    /* YOUR CODE HERE */
    return input[0] >= '0' && input[0] <= '9';
}

/* Assume the text input signifies a valid move. Return
the move hash corresponding to the move. */
MOVE ConvertTextInputToMove(STRING input)
{
    /* YOUR CODE HERE */
    return atoi(input);
}

/* Return the string representation of the move.
Ideally this matches with what the user is supposed to
type when they specify moves. */
STRING MoveToString(MOVE move)
{
    /* YOUR CODE HERE */
    STRING s = malloc(sizeof(char)*3);
    snprintf(s, 3, "%d", move);
    return s;
}

/* Basically just print the move. */
void PrintMove(MOVE move)
{
    /* YOUR CODE HERE */
    printf("%d", move);
}

/*********** END TEXTUI FUNCTIONS ***********/

/*********** BEGIN VARIANT FUNCTIONS ***********/

/* How many variants are you supporting? */
int NumberOfOptions()
{
    /* YOUR CODE HERE */
    return 1;
}

/* Return the current variant id. */
int getOption()
{
    /* YOUR CODE HERE */
    return 0;
}

/* The input is a variant id. This function sets any global variables
or data structures according to the variant specified by the variant id. */
void setOption(int option)
{
    /* YOUR CODE HERE  */
}

/*********** END VARIANT-RELATED FUNCTIONS ***********/

/* Don't worry about these Interact functions below yet.
They are used for the AutoGUI which eventually we would
want to implement, but they are not needed for solving. */
POSITION InteractStringToPosition(STRING str) {
    str += 8;
    for (POSITION p = 0; p < 21; p++) {
        if (str[p] == 'x') {
            return p;
        }
    }
    return 0;
}

STRING InteractPositionToString(POSITION position) {
    char board[22];
    memset(board, '-', 21);
    board[position] = 'x';
    board[21] = '\0';
    return UWAPI_Board_Regular2D_MakeBoardString(UWAPI_TURN_C, 22, board);
}

STRING InteractMoveToString(POSITION position, MOVE move) {
    return UWAPI_Board_Regular2D_MakeMoveStringWithSound((unsigned int) position, (unsigned int) move, 'x');
}