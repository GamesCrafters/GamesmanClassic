/************************************************************************
**
** NAME:        mccheckers.c
**
** DESCRIPTION: Chinese Checkers
**
** AUTHORS:     Seungyou(Bruce) Kim - University of California Berkeley
**
**
** DATE:        12/21/2025
**
**************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/

#include "gamesman.h"

CONST_STRING kAuthorName         = "Seungyou(Bruce) Kim";
CONST_STRING kGameName           = "Chinese Checkers";
CONST_STRING kDBName = "CCheckers";

POSITION gNumberOfPositions; /* 3542000 Can be reduced to half by reflection; 7x7 game is 85,251,690,988,464*/
POSITION kBadPosition        = -1;
POSITION gInitialPosition;
POSITION gMinimalPosition    =  0;

BOOLEAN kPartizan           = TRUE;
BOOLEAN kDebugMenu          = FALSE;
BOOLEAN kGameSpecificMenu   = FALSE;
BOOLEAN kTieIsPossible      = FALSE;
BOOLEAN kLoopy               = TRUE;
BOOLEAN kDebugDetermineValue = FALSE;
void*    gGameSpecificTclInit = NULL;

CONST_STRING kHelpGraphicInterface =
        "Not written yet";

CONST_STRING kHelpTextInterface    =
        "On your turn, choose the origin and target to make your move you desire and hit return. \n\
Describe the origin and target coordinates by the corresponding row (numeric)\n\
and column (alphabet). If at any point you have made a mistake,\n\
you can type u and hit return and the system will revert back to your most recent position.";

CONST_STRING kHelpOnYourTurn =
        "You move one of your pieces to an empty spot. You can jump over other pieces,\n\
        your own or your opponent's, if the space across the jumped piece is empty.\n\
        You may chain as many jumps as possible.";

CONST_STRING kHelpStandardObjective =
        "All your pieces reach the opposite starting position";

CONST_STRING kHelpReverseObjective =
        ""                                                                                                                                                             ;

CONST_STRING kHelpTieOccursWhen =   /* Should follow 'A Tie occurs when... */
                            "No tie in this game";

CONST_STRING kHelpExample =
        "";

/*************************************************************************
**
** Everything above here must be in every game file
**
**************************************************************************/

/*************************************************************************
**
** Every variable declared here is only used in this file (game-specific)
**
**************************************************************************/

int boardsize = 25;
int side = 5;
int numpegs = 3;

#define BLUE 1
#define RED 2
#define BLUEPEG 'B'
#define REDPEG 'R'
#define BLANK ' '

char start_standard_board[] = {
'B', 'B', ' ', ' ', ' ',
'B', ' ', ' ', ' ', ' ',
' ', ' ', ' ', ' ', ' ',
' ', ' ', ' ', ' ', 'R',
' ', ' ', ' ', 'R', 'R'
};

/************************************************************************
** Forward Declarations for Helper Functions
************************************************************************/

// Board navigation
int IndexToRow(int index);
int IndexToCol(int index);
int RowColToIndex(int row, int col);
BOOLEAN IsValidPosition(int row, int col);
void GetNeighbor(int row, int col, int d, int *newRow, int *newCol);

// Move encoding
MOVE EncodeMove(int source, int dest);
int GetMoveSource(MOVE move);
int GetMoveDestination(MOVE move);

// Jump generation
void GenerateJumpsFrom(int origSource, int currentPos, char *board,
                       BOOLEAN *visited, MOVELIST **moves);

/************************************************************************
**
** NAME:        InitializeGame
**
** DESCRIPTION: Initialize the gDatabase, a global variable. and the other
**              local variables.
**
************************************************************************/

void InitializeGame() {

        int piecesArray[10];

        piecesArray[0] = BLANK;
        piecesArray[1] = boardsize - 2*(numpegs);
        piecesArray[2] = boardsize - 2*(numpegs);
        piecesArray[3] = BLUEPEG;
        piecesArray[4] = numpegs;
        piecesArray[5] = numpegs;
        piecesArray[6] = REDPEG;
        piecesArray[7] = numpegs;
        piecesArray[8] = numpegs;
        piecesArray[9] = -1;

        gNumberOfPositions = generic_hash_init(boardsize, piecesArray, NULL, 0);
        gInitialPosition = generic_hash_hash(start_standard_board, BLUE);
}

/************************************************************************
**
** NAME:        DebugMenu
**
** DESCRIPTION: Menu used to debub internal problems. Does nothing if
**              kDebugMenu == FALSE
**
************************************************************************/

void DebugMenu()
{}

/************************************************************************
**
** NAME:        GameSpecificMenu
**
** DESCRIPTION: Menu used to change game-specific parmeters, such as
**              the side of the board in an nxn Nim board, etc. Does
**              nothing if kGameSpecificMenu == FALSE
**
************************************************************************/

void GameSpecificMenu() {}

/************************************************************************
**
** NAME:        SetTclCGameSpecificOptions
**
** DESCRIPTION: Set the C game-specific options (called from Tcl)
**              Ignore if you don't care about Tcl for now.
**
************************************************************************/

void SetTclCGameSpecificOptions (int options[]) {
	(void)options;
}

/************************************************************************
**
** NAME:        DoMove
**
** DESCRIPTION: Apply the move to the position.
**
** INPUTS:      POSITION thePosition : The old position
**              MOVE     theMove     : The move to apply.
**
** OUTPUTS:     (POSITION) : The position that results after the move.
**
** CALLS:       Hash ()
**              Unhash ()
**	            LIST OTHER CALLS HERE
*************************************************************************/
POSITION DoMove (POSITION thePosition, MOVE theMove) {
        char board[boardsize];
        int turn = generic_hash_turn(thePosition);
        int source = GetMoveSource(theMove);
        int dest = GetMoveDestination(theMove);

        generic_hash_unhash(thePosition, board);
        board[dest] = board[source];
        board[source] = BLANK;

        int newTurn = (turn == BLUE) ? RED : BLUE;
        return generic_hash_hash(board, newTurn);
}

/************************************************************************
**
** NAME:        GetInitialPosition
**
** DESCRIPTION: Ask the user for an initial position for testing. Store
**              it in the space pointed to by initialPosition;
**
** OUTPUTS:     POSITION initialPosition : The position to fill.
**
************************************************************************/

POSITION GetInitialPosition() {
        return gInitialPosition;
}

/************************************************************************
**
** NAME:        PrintComputersMove
**
** DESCRIPTION: Nicely format the computers move.
**
** INPUTS:      MOVE    computersMove : The computer's move.
**              STRING  computersName : The computer's name.
**
************************************************************************/

void PrintComputersMove(MOVE computersMove, STRING computersName) {
        char moveStr[20];
        MoveToString(computersMove, moveStr);
        printf("%s's move: %s\n", computersName, moveStr);
}

/************************************************************************
**
** NAME:        Primitive
**
** DESCRIPTION: Return the value of a position if it fulfills certain
**              'primitive' constraints. Some examples of this is having
**              three-in-a-row with Gobblet. Three in a row for the player
**              whose turn it is a win, otherwise its a loss.
**              Otherwise undecided.
**
** INPUTS:      POSITION position : The position to inspect.
**
** OUTPUTS:     (VALUE) an enum which is oneof: (win,lose,tie,undecided)
**
** CALLS:       LIST FUNCTION CALLS
**
**
************************************************************************/

VALUE Primitive(POSITION position) {
        char board[boardsize];
        int turn = generic_hash_turn(position);
        int redStart[3] = {19, 23, 24};
        int blueStart[3] = {0, 1, 5};
        int blueInRedZone = 0, redInBlueZone = 0;

        generic_hash_unhash(position, board);

        // Count Blue pieces in Red's starting zone
        for (int i = 0; i < 3; i++) {
                if (board[redStart[i]] == BLUEPEG) {
                        blueInRedZone++;
                }
        }

        // Count Red pieces in Blue's starting zone
        for (int i = 0; i < 3; i++) {
                if (board[blueStart[i]] == REDPEG) {
                        redInBlueZone++;
                }
        }

        // If all 3 Blue pieces in Red zone: Blue won
        if (blueInRedZone == 3) {
                return (turn == BLUE) ? win : lose;
        }

        // If all 3 Red pieces in Blue zone: Red won
        if (redInBlueZone == 3) {
                return (turn == RED) ? win : lose;
        }

        return undecided;
}

/************************************************************************
**
** NAME:        PrintPosition
**
** DESCRIPTION: Print the position in a pretty format, including the
**              prediction of the game's outcome.
**
** INPUTS:      POSITION position   : The position to pretty print.
**              STRING   playerName : The name of the player.
**              BOOLEAN  usersTurn  : TRUE <==> it's a user's turn.
**
** CALLS:       Unhash()
**              GetPrediction()
**              LIST OTHER CALLS HERE
**
************************************************************************/

void PrintPosition (POSITION position, STRING playerName, BOOLEAN usersTurn) {

  /*
                               _ _  
                             /     \
                        _ _ /       \ _ _ 
                      /     \       /     \
                 _ _ /       \ _ _ /       \ _ _
               /     \       /     \       /     \
          _ _ /       \ _ _ /       \ _ _ /       \ _ _
        /     \       /     \       /     \       /     \
   _ _ /       \ _ _ /       \ _ _ /       \ _ _ /       \ _ _
 /     \       /     \       /     \       /     \       /     \
/       \ _ _ /       \ _ _ /       \ _ _ /       \ _ _ /       \
\       /     \       /     \       /     \       /     \       /
 \ _ _ /       \ _ _ /       \ _ _ /       \ _ _ /       \ _ _ /
       \       /     \       /     \       /     \       /
        \ _ _ /       \ _ _ /       \ _ _ /       \ _ _ /
              \       /     \       /     \       /
               \ _ _ /       \ _ _ /       \ _ _ /
                     \       /     \       /
                      \ _ _ /       \ _ _ /
                            \       /
                             \ _ _ /

  */

  char board[boardsize];

  generic_hash_unhash(position, board);

  printf("\t                               _ _\n");
  printf("\t                             /     \\\n");
  printf("\t                        _ _ /   %c   \\ _ _\n", board[4]);
  printf("\t                      /     \\       /     \\\n");
  printf("\t                 _ _ /   %c   \\ _ _ /   %c   \\ _ _\n", board[3], board[9]);
  printf("\t               /     \\       /     \\       /     \\\n");
  printf("\t          _ _ /   %c   \\ _ _ /   %c   \\ _ _ /   %c   \\ _ _\n",
         board[2], board[8], board[14]);
  printf("\t        /     \\       /     \\       /     \\       /     \\\n");
  printf("\t   _ _ /   %c   \\ _ _ /   %c   \\ _ _ /   %c   \\ _ _ /   %c   \\ _ _\n",
         board[1], board[7], board[13], board[19]);
  printf("\t /     \\       /     \\       /     \\       /     \\       /     \\\n");
  printf("\t/   %c   \\ _ _ /   %c   \\ _ _ /   %c   \\ _ _ /   %c   \\ _ _ /   %c   \\\n",
         board[0], board[6], board[12], board[18], board[24]);
  printf("\t\\       /     \\       /     \\       /     \\       /     \\       /\n");
  printf("\t \\ _ _ /   %c   \\ _ _ /   %c   \\ _ _ /   %c   \\ _ _ /   %c   \\ _ _ /\n",
         board[5], board[11], board[17], board[23]);
  printf("\t       \\       /     \\       /     \\       /     \\       /\n");
  printf("\t        \\ _ _ /   %c   \\ _ _ /   %c   \\ _ _ /   %c   \\ _ _ /\n",
         board[10], board[16], board[22]);
  printf("\t              \\       /     \\       /     \\       /\n");
  printf("\t               \\ _ _ /   %c   \\ _ _ /   %c   \\ _ _ /\n",
         board[15], board[21]);
  printf("\t                     \\       /     \\       /\n");
  printf("\t                      \\ _ _ /   %c   \\ _ _ /\n", board[20]);
  printf("\t                            \\       /\n");
  printf("\t                             \\ _ _ /\n");

  printf("\n\t%s\n\n", GetPrediction(position,playerName,usersTurn));

}

/************************************************************************
**
** NAME:        GenerateMoves
**
** DESCRIPTION: Create a linked list of every move that can be reached
**              from this position. Return a pointer to the head of the
**              linked list.
**
** INPUTS:      POSITION position : The position to branch off of.
**
** OUTPUTS:     (MOVELIST *), a pointer that points to the first item
**              in the linked list of moves that can be generated.
**
** CALLS:       GENERIC_PTR SafeMalloc(int)
**              LIST OTHER CALLS HERE
**
************************************************************************/
MOVELIST *GenerateMoves(POSITION position) {
        MOVELIST *moves = NULL;
        char board[boardsize];
        int turn = generic_hash_turn(position);
        char myPiece = (turn == BLUE) ? BLUEPEG : REDPEG;

        generic_hash_unhash(position, board);

        // For each piece of current player
        for (int source = 0; source < boardsize; source++) {
                if (board[source] != myPiece) continue;

                int row = IndexToRow(source);
                int col = IndexToCol(source);

                // Generate simple adjacent moves (6 directions)
                for (int d = 0; d < 6; d++) {
                        int newRow, newCol;
                        GetNeighbor(row, col, d, &newRow, &newCol);

                        if (IsValidPosition(newRow, newCol)) {
                                int dest = RowColToIndex(newRow, newCol);
                                if (board[dest] == BLANK) {
                                        moves = CreateMovelistNode(EncodeMove(source, dest), moves);
                                }
                        }
                }

                // Generate all jump destinations (including chain jumps)
                BOOLEAN visited[boardsize];
                for (int i = 0; i < boardsize; i++) visited[i] = FALSE;
                visited[source] = TRUE;
                GenerateJumpsFrom(source, source, board, visited, &moves);
        }

        return moves;
}

/************************************************************************
**
** NAME:        GetAndPrintPlayersMove
**
** DESCRIPTION: This finds out if the player wanted an undo or abort or not.
**              If so, return Undo or Abort and don't change theMove.
**              Otherwise get the new theMove and fill the pointer up.
**
** INPUTS:      POSITION *thePosition : The position the user is at.
**              MOVE *theMove         : The move to fill with user's move.
**              STRING playerName     : The name of the player whose turn it is
**
** OUTPUTS:     USERINPUT             : Oneof( Undo, Abort, Continue )
**
** CALLS:       ValidMove(MOVE, POSITION)
**              BOOLEAN PrintPossibleMoves(POSITION) ...Always True!
**
************************************************************************/

USERINPUT GetAndPrintPlayersMove(POSITION thePosition, MOVE *theMove, STRING playerName) {
        USERINPUT ret;

        do {
                printf("%s's move [(u)ndo/(MOVE: source dest)] : ", playerName);
                ret = HandleDefaultTextInput(thePosition, theMove, playerName);
                if (ret != Continue) {
                        return ret;
                }
        } while (TRUE);

        return Continue;
}

/************************************************************************
**
** NAME:        ValidTextInput
**
** DESCRIPTION: Return TRUE iff the string input is of the right 'form'.
**              For example, if the user is allowed to select one slot
**              from the numbers 1-9, and the user chooses 0, it's not
**              valid, but anything from 1-9 IS, regardless if the slot
**              is filled or not. Whether the slot is filled is left up
**              to another routine.
**
** INPUTS:      STRING input : The string input the user typed.
**
** OUTPUTS:     BOOLEAN : TRUE if the input is a valid text input.
**
************************************************************************/

BOOLEAN ValidTextInput(STRING input) {
        int source, dest;

        if (sscanf(input, "%d %d", &source, &dest) != 2) {
                return FALSE;
        }

        return (source >= 0 && source < boardsize &&
                dest >= 0 && dest < boardsize);
}

/************************************************************************
**
** NAME:        ConvertTextInputToMove
**
** DESCRIPTION: Convert the string input to the internal move representation.
**              No checking if the input is valid is needed as it has
**              already been checked!
**
** INPUTS:      STRING input : The string input the user typed.
**
** OUTPUTS:     MOVE : The move corresponding to the user's input.
**
************************************************************************/

MOVE ConvertTextInputToMove(STRING input) {
        int source, dest;
        sscanf(input, "%d %d", &source, &dest);
        return EncodeMove(source, dest);
}

/************************************************************************
**
** NAME:        MoveToString
**
** DESCRIPTION: Returns the move as a STRING
**
** INPUTS:      MOVE *theMove         : The move to put into a string.
**              char *moveStringBuffer : Buffer to write movestring to
**
************************************************************************/

void MoveToString(MOVE theMove, char *moveStringBuffer) {
        sprintf(moveStringBuffer, "%d %d",
                GetMoveSource(theMove), GetMoveDestination(theMove));
}

/************************************************************************
**
** NAME:        NumberOfOptions
**
** DESCRIPTION: Calculates and returns the number of option combinations
**				there are with all the game variations you program.
**
** OUTPUTS:     int : the number of option combination there are.
**
************************************************************************/

int NumberOfOptions() {
        return 1;
}

/************************************************************************
**
** NAME:        getOption
**
** DESCRIPTION: A hash function to keep track of all the game variants.
**				Should return a different number for each set of
**				variants.
**
** OUTPUTS:     int : the number representation of the options.
**
************************************************************************/

int getOption() {
        return 0;
}

/************************************************************************
**
** NAME:        setOption
**
** DESCRIPTION: The corresponding unhash for the game variants.
**				Should take the input and set all the appropriate
**				variants.
**
** INPUT:     int : the number representation of the options.
**
************************************************************************/

void setOption(int option) {
        (void)option;
}

/************************************************************************
**
** NAME:        PositionToAutoGUIString
**
** DESCRIPTION: Converts position to AutoGUI string representation
**
************************************************************************/

STRING PositionToAutoGUIString(POSITION position) {
        (void)position;
        return "0";  // Stub implementation
}

/************************************************************************
**
** NAME:        MoveToAutoGUIString
**
** DESCRIPTION: Converts move to AutoGUI string representation
**
************************************************************************/

STRING MoveToAutoGUIString(POSITION position, MOVE move) {
        (void)position;
        (void)move;
        return "0_0";  // Stub implementation
}

/************************************************************************
**
** NAME:        StringToPosition
**
** DESCRIPTION: Converts string to position
**
************************************************************************/

POSITION StringToPosition(STRING positionString) {
        (void)positionString;
        return gInitialPosition;  // Stub implementation
}

/************************************************************************
*************************************************************************
**         EVERYTHING BELOW THESE LINES IS LOCAL TO THIS FILE
*************************************************************************
************************************************************************/


/************************************************************************
** This is where you can put any helper functions, including your
** hash and unhash functions if you are not using one of the existing
** ones.
************************************************************************/

/************************************************************************
** Helper Functions for Board Navigation
************************************************************************/

// Convert index ↔ row/col for 5x5 board
int IndexToRow(int index) {
        return index / side;
}

int IndexToCol(int index) {
        return index % side;
}

int RowColToIndex(int row, int col) {
        return row * side + col;
}

BOOLEAN IsValidPosition(int row, int col) {
        return (row >= 0 && row < side && col >= 0 && col < side);
}

// 6 hexagonal directions using offset row pattern
// Order: NE, E, SE, SW, W, NW
// Even rows (0,2,4) have different offsets than odd rows (1,3)
int evenRowDir[6][2] = {
        {-1, 0},  // NE
        { 0, 1},  // E
        { 1, 0},  // SE
        { 1,-1},  // SW
        { 0,-1},  // W
        {-1,-1}   // NW
};

int oddRowDir[6][2] = {
        {-1, 1},  // NE
        { 0, 1},  // E
        { 1, 1},  // SE
        { 1, 0},  // SW
        { 0,-1},  // W
        {-1, 0}   // NW
};

// Get neighbor in direction d from position (row, col)
void GetNeighbor(int row, int col, int d, int *newRow, int *newCol) {
        if (row % 2 == 0) {
                *newRow = row + evenRowDir[d][0];
                *newCol = col + evenRowDir[d][1];
        } else {
                *newRow = row + oddRowDir[d][0];
                *newCol = col + oddRowDir[d][1];
        }
}

/************************************************************************
** Helper Functions for Move Encoding
************************************************************************/

MOVE EncodeMove(int source, int dest) {
        return source * 100 + dest;
}

int GetMoveSource(MOVE move) {
        return move / 100;
}

int GetMoveDestination(MOVE move) {
        return move % 100;
}

/************************************************************************
** Helper Function for Jump Generation
************************************************************************/

void GenerateJumpsFrom(int origSource, int currentPos, char *board,
                       BOOLEAN *visited, MOVELIST **moves) {
        int currentRow = IndexToRow(currentPos);
        int currentCol = IndexToCol(currentPos);

        // For each of 6 hexagonal directions:
        for (int d = 0; d < 6; d++) {
                int jumpRow, jumpCol, landRow, landCol;

                // Get adjacent position (where jumped piece is)
                GetNeighbor(currentRow, currentCol, d, &jumpRow, &jumpCol);
                if (!IsValidPosition(jumpRow, jumpCol)) continue;

                // Get landing position (2 steps in same direction)
                GetNeighbor(jumpRow, jumpCol, d, &landRow, &landCol);
                if (!IsValidPosition(landRow, landCol)) continue;

                int jumpPos = RowColToIndex(jumpRow, jumpCol);
                int landPos = RowColToIndex(landRow, landCol);

                // Can jump if: piece at jumpPos, empty at landPos, not visited
                if (board[jumpPos] != BLANK && board[landPos] == BLANK && !visited[landPos]) {
                        visited[landPos] = TRUE;
                        *moves = CreateMovelistNode(EncodeMove(origSource, landPos), *moves);
                        GenerateJumpsFrom(origSource, landPos, board, visited, moves);
                        visited[landPos] = FALSE;  // Backtrack
                }
        }
}