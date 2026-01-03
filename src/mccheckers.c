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
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

CONST_STRING kAuthorName         = "Seungyou(Bruce) Kim";
CONST_STRING kGameName           = "Chinese Checkers";
CONST_STRING kDBName = "CCheckers";

POSITION gNumberOfPositions; /* 3542000 Can be reduced to half by reflection; 7x7 game is 85,251,690,988,464*/
POSITION kBadPosition        = -1;
POSITION gInitialPosition;
POSITION gMinimalPosition    =  0;

BOOLEAN kPartizan           = TRUE;
BOOLEAN kDebugMenu          = TRUE;
BOOLEAN kGameSpecificMenu   = TRUE;
BOOLEAN kTieIsPossible      = FALSE;
BOOLEAN kLoopy               = TRUE;
BOOLEAN kDebugDetermineValue = FALSE;
void*    gGameSpecificTclInit = NULL;

CONST_STRING kHelpGraphicInterface =
        "Not written yet";

CONST_STRING kHelpTextInterface    =
        "On your turn, choose the origin and target to make your move you desire and hit return. \n\
Describe the origin and target coordinates using column (A-E) followed by row (1-5).\n\
For example, type 'A1 C1' to move from position A1 to position C1. If at any point\n\
you have made a mistake, you can type u and hit return and the system will revert\n\
back to your most recent position.";

CONST_STRING kHelpOnYourTurn =
        "You move one of your pieces to an empty spot. You can jump over other pieces,\n\
        your own or your opponent's, if the space across the jumped piece is empty.\n\
        You may chain as many jumps as possible.";

CONST_STRING kHelpStandardObjective =
        "All your pieces reach the opposite starting position";

CONST_STRING kHelpReverseObjective =
        "No Reverse Objective in this game";                                                                                                                                                             ;

CONST_STRING kHelpTieOccursWhen =   /* Should follow 'A Tie occurs when... */
                            "No tie in this game";

CONST_STRING kHelpExample =
        "No example play available at this time.";

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

// Coordinate conversion (e.g., "1A" <-> index 0)
int CoordToIndex(const char *coord);
void IndexToCoord(int index, char *coord);

// Debug menu helpers
void PrintBoardForDebug(POSITION position);
void TestCoordConversion();
void TestMoveGeneration();
void TestPrimitive();

// Custom position helpers for GetInitialPosition()
BOOLEAN ParseCoordinateList(const char *input, int *positions, int *count, int maxCount);
POSITION BuildCustomBoardPosition();
int PromptForTurn();
BOOLEAN ValidateBoardConfiguration(char *board, int blueCount, int redCount);
POSITION GetInitialPosition(void);

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

void DebugMenu() {
        do {
                printf("\n\t----- Module DEBUGGER for %s -----\n\n", kGameName);

                printf("\tc)\tTest (C)oordinate Conversion\n");
                printf("\ti)\t(I)nspect Board State\n");
                printf("\tm)\tTest (M)ove Generation\n");
                printf("\tp)\tTest (P)rimitive Function\n");
                printf("\n\tb)\t(B)ack = Return to previous activity.\n");
                printf("\n\nSelect an option: ");

                switch(GetMyChar()) {
                case 'Q': case 'q':
                        ExitStageRight();
                        break;
                case 'C': case 'c':
                        TestCoordConversion();
                        break;
                case 'I': case 'i':
                        PrintBoardForDebug(gInitialPosition);
                        break;
                case 'M': case 'm':
                        TestMoveGeneration();
                        break;
                case 'P': case 'p':
                        TestPrimitive();
                        break;
                case 'B': case 'b':
                        return;
                default:
                        BadMenuChoice();
                        HitAnyKeyToContinue();
                        break;
                }
        } while(TRUE);
}

/************************************************************************
**
** NAME:        GameSpecificMenu
**
** DESCRIPTION: Menu used to change game-specific parmeters, such as
**              the side of the board in an nxn Nim board, etc. Does
**              nothing if kGameSpecificMenu == FALSE
**
************************************************************************/

void GameSpecificMenu() {
        do {
                printf("\n\t----- Game-specific options for %s -----\n\n", kGameName);

                printf("\ti)\t Change the (I)nitial Position.\n");
                printf("\tb)\t (B)ack to the previous menu\n\n");
                printf("Select an option: ");

                switch(GetMyChar()) {
                case 'Q': case 'q':
                        ExitStageRight();
                        break;
                case 'H': case 'h':
                        HelpMenus();
                        break;
                case 'I': case 'i':
                        gInitialPosition = GetInitialPosition();
                        break;
                case 'B': case 'b':
                        return;
                default:
                        BadMenuChoice();
                        HitAnyKeyToContinue();
                        break;
                }
        } while(TRUE);
}

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
        return BuildCustomBoardPosition();
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

  printf("\n\t                             E ↘  _ _\n");
  printf("\t                                /     \\\n");
  printf("\t                      D ↘  _ _ /   %c   \\ _ _                   \n", board[4]);
  printf("\t                         /     \\       /     \\\n");
  printf("\t               C ↘  _ _ /   %c   \\ _ _ /   %c   \\ _ _          \n", board[3], board[9]);
  printf("\t                  /     \\       /     \\       /     \\\n");
  printf("\t        B ↘  _ _ /   %c   \\ _ _ /   %c   \\ _ _ /   %c   \\ _ _   \n", board[2], board[8], board[14]);
  printf("\t           /     \\       /     \\       /     \\       /     \\\n");
  printf("\t A ↘  _ _ /   %c   \\ _ _ /   %c   \\ _ _ /   %c   \\ _ _ /   %c   \\ _ _   \n", board[1], board[7], board[13], board[19]);
  printf("\t    /     \\       /     \\       /     \\       /     \\       /     \\\n");
  printf("\t   /   %c   \\ _ _ /   %c   \\ _ _ /   %c   \\ _ _ /   %c   \\ _ _ /   %c   \\   \n", board[0], board[6], board[12], board[18], board[24]);
  printf("\t   \\       /     \\       /     \\       /     \\       /     \\       /\n");
  printf("\t    \\ _ _ /   %c   \\ _ _ /   %c   \\ _ _ /   %c   \\ _ _ /   %c   \\ _ _ /   \n", board[5], board[11], board[17], board[23]);
  printf("\t    ↗     \\       /     \\       /     \\       /     \\       /\n");
  printf("\t   1       \\ _ _ /   %c   \\ _ _ /   %c   \\ _ _ /   %c   \\ _ _ /        \n", board[10], board[16], board[22]);
  printf("\t           ↗     \\       /     \\       /     \\       /\n");
  printf("\t          2       \\ _ _ /   %c   \\ _ _ /   %c   \\ _ _ /            \n", board[15], board[21]);
  printf("\t                  ↗     \\       /     \\       /\n");
  printf("\t                 3       \\ _ _ /   %c   \\ _ _ /                 \n", board[20]);
  printf("\t                         ↗     \\       /\n");
  printf("\t                        4       \\ _ _ /\n");
  printf("\t                                ↗       \n");
  printf("\t                               5       \n");
  printf("\n\tLegend: B=Blue, R=Red, ' '=Empty\n");
  printf("\tMove format: <origin> <target> (e.g., \"A1 C1\" to move from A1 to C1)\n");
  printf("\tTurn: %s\n", (generic_hash_turn(position) == BLUE) ? "Blue" : "Red");
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
                printf("%8s's move [(u)ndo/(MOVE: source dest)] : ", playerName);
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
        char sourceCoord[10], destCoord[10];

        // Parse input as "A1 C3" format (column A-E, row 1-5)
        if (sscanf(input, "%s %s", sourceCoord, destCoord) != 2) {
                return FALSE;
        }

        // Convert coordinates to indices
        int source = CoordToIndex(sourceCoord);
        int dest = CoordToIndex(destCoord);

        // Check if both coordinates are valid
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
        char sourceCoord[10], destCoord[10];
        sscanf(input, "%s %s", sourceCoord, destCoord);

        int source = CoordToIndex(sourceCoord);
        int dest = CoordToIndex(destCoord);

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
        char sourceCoord[4], destCoord[4];
        IndexToCoord(GetMoveSource(theMove), sourceCoord);
        IndexToCoord(GetMoveDestination(theMove), destCoord);
        sprintf(moveStringBuffer, "%s %s", sourceCoord, destCoord);
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

// 6 hexagonal directions for the diamond-shaped hex board
// This layout uses the same offsets for all positions (no even/odd distinction)
// Directions: up-left, up, left, right, down, down-right
int hexDir[6][2] = {
        {-1, 1},  // up-left diagonal
        {-1,  0},  // up
        { 0, -1},  // left
        { 0,  1},  // right
        { 1,  0},  // down
        { 1,  -1}   // down-right diagonal
};

// Get neighbor in direction d from position (row, col)
void GetNeighbor(int row, int col, int d, int *newRow, int *newCol) {
        *newRow = row + hexDir[d][0];
        *newCol = col + hexDir[d][1];
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

/************************************************************************
** Helper Functions for Coordinate Conversion
************************************************************************/

// Convert coordinate string (e.g., "A1", "C3") to board index (0-24)
// Format: <column><row> where column is A-E and row is 1-5
// Returns -1 if invalid coordinate
int CoordToIndex(const char *coord) {
        if (coord == NULL || coord[0] == '\0' || coord[1] == '\0') {
                return -1;
        }

        // Extract col (A-E or a-e)
        char colChar = coord[0];
        if (colChar >= 'a' && colChar <= 'e') {
                colChar = colChar - 'a' + 'A';  // Convert to uppercase
        }
        if (colChar < 'A' || colChar > 'E') {
                return -1;
        }
        int col = colChar - 'A';

        // Extract row (1-5)
        int row = coord[1] - '0';
        if (row < 1 || row > 5) {
                return -1;
        }

        // Convert to index
        return (row - 1) * side + col;
}

// Convert board index (0-24) to coordinate string (e.g., "A1", "C3")
// Format: <column><row> where column is A-E and row is 1-5
void IndexToCoord(int index, char *coord) {
        if (index < 0 || index >= boardsize) {
                coord[0] = '?';
                coord[1] = '?';
                coord[2] = '\0';
                return;
        }

        int row = (index / side) + 1;
        int col = index % side;

        coord[0] = 'A' + col;           // Column: A-E
        coord[1] = '0' + row;           // Row: 1-5
        coord[2] = '\0';
}

/************************************************************************
** Debug Menu Helper Functions
************************************************************************/

void PrintBoardForDebug(POSITION position) {
        char board[boardsize];
        int turn = generic_hash_turn(position);
        int blueCount = 0, redCount = 0;

        generic_hash_unhash(position, board);

        printf("\n\t=== Board State Debug ===\n");
        printf("\tPosition: %llu\n", position);
        printf("\tCurrent Turn: %s\n", (turn == BLUE) ? "Blue" : "Red");
        printf("\n\tBoard Array (Index: Piece):\n");

        for (int i = 0; i < boardsize; i++) {
                char coord[4];
                IndexToCoord(i, coord);
                printf("\t%2d (%s): '%c'", i, coord, board[i]);

                if (board[i] == BLUEPEG) blueCount++;
                else if (board[i] == REDPEG) redCount++;

                if ((i + 1) % 5 == 0) printf("\n");
                else printf("  ");
        }

        printf("\n\tPiece Counts: Blue=%d, Red=%d\n", blueCount, redCount);
        printf("\t========================\n\n");
}

void TestCoordConversion() {
        char input[10];
        char coord[4];

        printf("\n\t=== Coordinate Conversion Test ===\n");
        printf("\tEnter coordinate (e.g., A1, E5): ");

        if (scanf("%s", input) != 1) {
                printf("\tError: Invalid input\n");
                return;
        }

        int index = CoordToIndex(input);

        if (index < 0 || index >= boardsize) {
                printf("\tError: Invalid coordinate '%s'\n", input);
                printf("\tValid range: A-E (columns), 1-5 (rows)\n");
        } else {
                IndexToCoord(index, coord);
                printf("\tInput: %s\n", input);
                printf("\tConverted to index: %d\n", index);
                printf("\tConverted back to coord: %s\n", coord);

                if (strcmp(input, coord) == 0 ||
                    (toupper(input[0]) == coord[0] && input[1] == coord[1])) {
                        printf("\tStatus: Conversion OK ✓\n");
                } else {
                        printf("\tStatus: Conversion MISMATCH!\n");
                }
        }

        printf("\t================================\n\n");
}

void TestMoveGeneration() {
        POSITION position = gInitialPosition;
        MOVELIST *moves, *curr;
        int moveCount = 0, jumpCount = 0, regularCount = 0;

        printf("\n\t=== Move Generation Test ===\n");
        printf("\tTesting from initial position: %llu\n", position);

        char board[boardsize];
        int turn = generic_hash_turn(position);
        generic_hash_unhash(position, board);

        printf("\tCurrent turn: %s\n", (turn == BLUE) ? "Blue" : "Red");

        moves = GenerateMoves(position);

        if (moves == NULL) {
                printf("\tNo moves available!\n");
        } else {
                printf("\n\tAvailable moves:\n");

                for (curr = moves; curr != NULL; curr = curr->next) {
                        int source = GetMoveSource(curr->move);
                        int dest = GetMoveDestination(curr->move);
                        char sourceCoord[4], destCoord[4];

                        IndexToCoord(source, sourceCoord);
                        IndexToCoord(dest, destCoord);

                        int sourceRow = IndexToRow(source);
                        int sourceCol = IndexToCol(source);
                        int destRow = IndexToRow(dest);
                        int destCol = IndexToCol(dest);

                        int rowDiff = abs(destRow - sourceRow);
                        int colDiff = abs(destCol - sourceCol);
                        BOOLEAN isJump = (rowDiff > 1 || colDiff > 1);

                        printf("\t  %s -> %s  (Move: %d)  [%s]\n",
                               sourceCoord, destCoord, curr->move,
                               isJump ? "JUMP" : "regular");

                        moveCount++;
                        if (isJump) jumpCount++;
                        else regularCount++;
                }

                FreeMoveList(moves);
        }

        printf("\n\tTotal moves: %d (Regular: %d, Jumps: %d)\n",
               moveCount, regularCount, jumpCount);
        printf("\t============================\n\n");
}

void TestPrimitive() {
        POSITION position = gInitialPosition;
        char board[boardsize];
        int turn = generic_hash_turn(position);
        int redStart[3] = {19, 23, 24};
        int blueStart[3] = {0, 1, 5};
        int blueInRedZone = 0, redInBlueZone = 0;

        printf("\n\t=== Primitive Function Test ===\n");
        printf("\tTesting position: %llu\n", position);
        printf("\tCurrent turn: %s\n", (turn == BLUE) ? "Blue" : "Red");

        generic_hash_unhash(position, board);

        for (int i = 0; i < 3; i++) {
                if (board[redStart[i]] == BLUEPEG) {
                        char coord[4];
                        IndexToCoord(redStart[i], coord);
                        printf("\tBlue piece at Red zone: %s\n", coord);
                        blueInRedZone++;
                }
        }

        for (int i = 0; i < 3; i++) {
                if (board[blueStart[i]] == REDPEG) {
                        char coord[4];
                        IndexToCoord(blueStart[i], coord);
                        printf("\tRed piece at Blue zone: %s\n", coord);
                        redInBlueZone++;
                }
        }

        printf("\n\tBlue in Red zone: %d/3\n", blueInRedZone);
        printf("\tRed in Blue zone: %d/3\n", redInBlueZone);

        VALUE result = Primitive(position);
        printf("\n\tPrimitive result: ");

        switch(result) {
        case win:
                printf("WIN (current player wins)\n");
                break;
        case lose:
                printf("LOSE (current player loses)\n");
                break;
        case tie:
                printf("TIE\n");
                break;
        case undecided:
                printf("UNDECIDED (game continues)\n");
                break;
        default:
                printf("UNKNOWN (%d)\n", result);
        }

        printf("\t===============================\n\n");
}

/************************************************************************
**
** NAME:        ParseCoordinateList
**
** DESCRIPTION: Parse comma-separated coordinates into board indices
**
** INPUTS:      const char *input : Input string (e.g., "A1,B1,A2")
**              int maxCount      : Maximum allowed positions
**
** OUTPUTS:     int *positions : Array to store board indices
**              int *count     : Number of positions found
**
** RETURNS:     TRUE if parsing successful, FALSE otherwise
**
************************************************************************/

BOOLEAN ParseCoordinateList(const char *input, int *positions, int *count, int maxCount) {
        char coord[3];  // 2 chars + null terminator
        int i = 0, coordIdx = 0;
        int posCount = 0;

        if (!input || maxCount <= 0) return FALSE;

        while (input[i] != '\0' && posCount < maxCount) {
                // Skip whitespace and commas
                while (input[i] == ' ' || input[i] == ',') i++;

                if (input[i] == '\0') break;

                // Extract coordinate (2 characters: letter + digit)
                coordIdx = 0;
                while (input[i] != '\0' && input[i] != ',' && input[i] != ' ' && coordIdx < 2) {
                        coord[coordIdx++] = input[i++];
                }
                coord[coordIdx] = '\0';

                // Validate coordinate format (should be 2 characters)
                if (coordIdx != 2) {
                        printf("\tError: Invalid coordinate format '%s'\n", coord);
                        return FALSE;
                }

                // Convert to index using existing CoordToIndex()
                int index = CoordToIndex(coord);
                if (index < 0 || index >= boardsize) {
                        printf("\tError: Invalid coordinate '%s' (must be A-E, 1-5)\n", coord);
                        return FALSE;
                }

                // Check for duplicates
                for (int j = 0; j < posCount; j++) {
                        if (positions[j] == index) {
                                printf("\tError: Duplicate position '%s' detected\n", coord);
                                return FALSE;
                        }
                }

                positions[posCount++] = index;
        }

        *count = posCount;
        return TRUE;
}

/************************************************************************
**
** NAME:        ValidateBoardConfiguration
**
** DESCRIPTION: Validate that board has correct number of pieces
**
** INPUTS:      char *board    : Board array to validate
**              int blueCount  : Expected number of blue pieces
**              int redCount   : Expected number of red pieces
**
** RETURNS:     TRUE if valid, FALSE otherwise
**
************************************************************************/

BOOLEAN ValidateBoardConfiguration(char *board, int blueCount, int redCount) {
        int actualBlue = 0, actualRed = 0, actualBlank = 0;

        // Count pieces on board
        for (int i = 0; i < boardsize; i++) {
                if (board[i] == BLUEPEG) actualBlue++;
                else if (board[i] == REDPEG) actualRed++;
                else if (board[i] == BLANK) actualBlank++;
                else {
                        printf("\tError: Invalid character '%c' at position %d\n", board[i], i);
                        return FALSE;
                }
        }

        // Validate counts
        if (actualBlue != blueCount) {
                printf("\tError: Expected %d Blue pieces, found %d\n", blueCount, actualBlue);
                return FALSE;
        }
        if (actualRed != redCount) {
                printf("\tError: Expected %d Red pieces, found %d\n", redCount, actualRed);
                return FALSE;
        }
        if (actualBlank != boardsize - blueCount - redCount) {
                printf("\tError: Incorrect number of blank positions\n");
                return FALSE;
        }

        return TRUE;
}

/************************************************************************
**
** NAME:        PromptForTurn
**
** DESCRIPTION: Ask user to select whose turn it is
**
** RETURNS:     BLUE or RED (int constants)
**
************************************************************************/

int PromptForTurn() {
        char choice;

        while (TRUE) {
                printf("\n\tWhose turn? Enter 'B' for Blue or 'R' for Red: ");
                choice = GetMyChar();

                // Convert to uppercase
                if (choice >= 'a' && choice <= 'z') {
                        choice = choice - 'a' + 'A';
                }

                if (choice == 'B') {
                        return BLUE;
                } else if (choice == 'R') {
                        return RED;
                } else {
                        printf("\tInvalid choice. Please enter 'B' or 'R'.\n");
                }
        }
}

/************************************************************************
**
** NAME:        BuildCustomBoardPosition
**
** DESCRIPTION: Interactive function to build a custom board position
**
** RETURNS:     POSITION (custom position or standard if error)
**
************************************************************************/

POSITION BuildCustomBoardPosition() {
        char board[boardsize];
        int bluePositions[numpegs];
        int redPositions[numpegs];
        int blueCount = 0, redCount = 0;
        char input[100];
        int turn;
        int retries = 0;
        const int MAX_RETRIES = 3;

        printf("\n\n\t=== Custom Board Position Setup ===\n");
        printf("\n\tCoordinate System Reference:\n");
        printf("\t     A   B   C   D   E\n");
        printf("\t  1  0   1   2   3   4\n");
        printf("\t  2  5   6   7   8   9\n");
        printf("\t  3  10  11  12  13  14\n");
        printf("\t  4  15  16  17  18  19\n");
        printf("\t  5  20  21  22  23  24\n");
        printf("\n\tYou need exactly %d Blue pieces and %d Red pieces.\n\n", numpegs, numpegs);

        // Get Blue piece positions
        while (retries < MAX_RETRIES) {
                printf("\tEnter Blue piece coordinates (e.g., 'A1,B1,A2'): ");
                GetMyStr(input, sizeof(input));

                if (ParseCoordinateList(input, bluePositions, &blueCount, numpegs)) {
                        if (blueCount == numpegs) {
                                break;
                        } else {
                                printf("\tError: Expected exactly %d pieces, got %d. Try again.\n",
                                       numpegs, blueCount);
                        }
                }
                retries++;
        }

        if (retries >= MAX_RETRIES) {
                printf("\n\tMax retries exceeded. Using standard initial position.\n");
                return generic_hash_hash(start_standard_board, BLUE);
        }

        // Get Red piece positions
        retries = 0;
        while (retries < MAX_RETRIES) {
                printf("\tEnter Red piece coordinates (e.g., 'D4,D5,E5'): ");
                GetMyStr(input, sizeof(input));

                if (ParseCoordinateList(input, redPositions, &redCount, numpegs)) {
                        if (redCount == numpegs) {
                                // Check for overlap with Blue positions
                                BOOLEAN overlap = FALSE;
                                for (int i = 0; i < blueCount; i++) {
                                        for (int j = 0; j < redCount; j++) {
                                                if (bluePositions[i] == redPositions[j]) {
                                                        printf("\tError: Position %d occupied by both Blue and Red\n",
                                                               bluePositions[i]);
                                                        overlap = TRUE;
                                                        break;
                                                }
                                        }
                                        if (overlap) break;
                                }
                                if (!overlap) break;
                        } else {
                                printf("\tError: Expected exactly %d pieces, got %d. Try again.\n",
                                       numpegs, redCount);
                        }
                }
                retries++;
        }

        if (retries >= MAX_RETRIES) {
                printf("\n\tMax retries exceeded. Using standard initial position.\n");
                return generic_hash_hash(start_standard_board, BLUE);
        }

        // Build board array
        for (int i = 0; i < boardsize; i++) {
                board[i] = BLANK;
        }
        for (int i = 0; i < blueCount; i++) {
                board[bluePositions[i]] = BLUEPEG;
        }
        for (int i = 0; i < redCount; i++) {
                board[redPositions[i]] = REDPEG;
        }

        // Validate board configuration
        if (!ValidateBoardConfiguration(board, blueCount, redCount)) {
                printf("\n\tBoard validation failed. Using standard initial position.\n");
                return generic_hash_hash(start_standard_board, BLUE);
        }

        // Prompt for turn
        turn = PromptForTurn();

        // Create and return position
        POSITION newPosition = generic_hash_hash(board, turn);

        printf("\n\t=== Custom Position Created ===\n");
        printf("\tPosition ID: %llu\n\n", newPosition);

        return newPosition;
}
