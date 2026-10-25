/************************************************************************
**
** NAME:        mclusterfuss.c
**
** DESCRIPTION: Clusterfuss
**
** AUTHOR:      Kai Chen, Matthew Lee
**
** DATE:        2026-09-14
**
************************************************************************/

/*
 * Clusterfuss, by Mark Steere (July 2023).  https://marksteeregames.com
 *
 * Two players, Blue and Red, on a rows x cols board initially filled with
 * checkers in a checkerboard pattern.  On an odd x odd board the centre square
 * is left empty, so both sides start with the same number of checkers.  Blue
 * moves first.  (The colours are only names; Blue is player 1, matching
 * GamesmanUni, which always draws player 1 in blue.)
 *
 * MOVES        Every move is an orthogonal king capture: move one of your own
 *              checkers one square up/down/left/right onto an occupied square,
 *              removing the checker that was there (friendly or enemy).
 * RESTRICTION  A move is legal only if, afterwards, exactly one group (checkers
 *              of either colour connected orthogonally) contains checkers of the
 *              mover's colour.
 * REMOVAL      Any group detached by the move that contains only enemy checkers
 *              is removed, so exactly one group is left at the end of every turn.
 * PASSING      A player with no legal move has their turn skipped ('-').
 * OBJECT       Remove all enemy checkers from the board.
 *
 * This module follows GamesmanPy's games/src/games/clusterfuss.py: the same
 * variants, the same Readable and AUTOGUI strings, and the same raw position
 * encoding, so both solvers can be checked against each other.
 *
 * DATA REPRESENTATION
 *   Cells    Row-major from the top-left: cell i is (i / cols, i % cols).
 *            Squares are named chess-style, e.g. a1 is the bottom-left square.
 *   Board    Two bitmasks, bit i set if cell i holds a Blue / a Red checker.
 *   Raw      The board as a base-3 number, cell i weighing 3^i (0 empty,
 *            1 Blue, 2 Red).  3^20 < 2^32, so it fits in a uint32_t.
 *   POSITION A compact hash over the positions reachable from the start only.
 *            The full board space of 4x5 has billions of hash values but only
 *            about 18.7 million reachable positions, so InitializeGame()
 *            enumerates the reachable positions once into two sorted arrays of
 *            raw boards (Blue to move, then Red to move), and a position is its
 *            index in them.  The arrays are cached in
 *            data/mclusterfuss_<option>_positions.dat.gz.
 *   MOVE     (source << 2) | direction, direction UP 0, RIGHT 1, DOWN 2, LEFT 3.
 *            A skip is cells << 2.
 *
 * WEB UI (multipart moves)
 *   A capture is made in two clicks: select a checker (part-move
 *   SELECT_PART | source, AUTOGUI 'A_-_<source>_-'), then click a capture
 *   arrow (part-move CAPTURE_PART | move, AUTOGUI 'M_<source>_<target>_x').
 *   The selected state is the intermediate position
 *   INTERMEDIATE_FLAG | (source << SELECTED_SHIFT) | position.
 *   AUTOGUI positions are '1_'/'2_' + one char per cell ('x' Blue, 'o' Red,
 *   '-' empty) + a second layer of one char per cell, 's' under the selected
 *   checker.  A skip is the pass-turn button 'A_P_<2 * cells>_-'.
 */

#include "gamesman.h"
#include <stdint.h>
#include <sys/stat.h>
#include <unistd.h>
#include <zlib.h>

CONST_STRING kAuthorName = "Kai Chen, Matthew Lee";
CONST_STRING kGameName = "Clusterfuss";  // Use this spacing and case
CONST_STRING kDBName = "clusterfuss";    // Use this spacing and case

/**
 * @brief An upper bound on the number of reachable positions.
 *
 * @details The hash value of every reachable position must be less
 * than `gNumberOfPositions`.
 */
POSITION gNumberOfPositions = 0;

/**
 * @brief The hash value of the initial position of the default
 * variant of the game.
 *
 * @note If multiple variants are supported and the hash value
 * of the initial position is different among those variants,
 * then ensure that gInitialPosition is modified appropriately
 * in both setOption() and GameSpecificMenu(). You may also
 * choose to modify `gInitialPosition` in InitializeGame().
 */
POSITION gInitialPosition = 0;

/**
 * @brief Indicates whether this game is PARTIZAN, i.e. whether, given
 * a board, each player has a different set of moves
 * available to them on their turn. If the game is impartial, this is FALSE.
 */
BOOLEAN kPartizan = TRUE;

/**
 * @brief Whether a tie or draw is possible in this game.
 */
BOOLEAN kTieIsPossible = FALSE;

/**
 * @brief Whether the game is loopy. It is TRUE if there exists a position
 * P in the game such that, there is a sequence of N >= 1 moves one can
 * make starting from P that allows them to revisit P.
 *
 * @note Every capture removes at least one checker, so no position repeats.
 */
BOOLEAN kLoopy = FALSE;

/**
 * @brief Whether symmetries are supported, i.e., whether there is
 * at least one position P in the game that gCanonicalPosition()
 * maps to a position Q such that P != Q. If gCanonicalPosition
 * (initialized in InitializeGame() in this file), is set to NULL,
 * then this should be set to FALSE.
 */
BOOLEAN kSupportsSymmetries = FALSE;

/**
 * @brief Useful for some solvers. Do not change this.
 */
POSITION kBadPosition = -1;

/**
 * @brief For debugging, some solvers print debug statements while
 * solving a game if kDebugDetermineValue is set to TRUE. If this
 * is set to FALSE, then those solvers do not print the debugging
 * statements.
 */
BOOLEAN kDebugDetermineValue = FALSE;

/**
 * @brief Declaration of optional functions.
 */
POSITION GetCanonicalPosition(POSITION);
void PositionToString(POSITION, char*);
MULTIPARTEDGELIST *GenerateMultipartMoveEdges(POSITION, MOVELIST*, POSITIONLIST*);

/**
 * @brief If multiple variants are supported, set this
 * to true -- GameSpecificMenu() must be implemented.
 */
BOOLEAN kGameSpecificMenu = TRUE;

/**
 * @brief Set this to true if the DebugMenu() is implemented.
 */
BOOLEAN kDebugMenu = FALSE;

/**
 * @brief These variables are not needed for solving but if you have time
 * after you're done solving the game you should initialize them
 * with something helpful, for the TextUI.
 */
CONST_STRING kHelpGraphicInterface = "";

CONST_STRING kHelpTextInterface =
"Type a move as the square of one of your checkers followed by the direction\n\
to capture in: u (up), d (down), l (left) or r (right).  For example, b3u\n\
moves the checker on b3 up onto b4.  Squares are named chess-style: files\n\
a, b, ... from left to right and ranks 1, 2, ... from bottom to top.  If you\n\
have no legal move, type - to skip your turn.";

CONST_STRING kHelpOnYourTurn =
"Pick one of your own checkers and move it onto an orthogonally adjacent\n\
square (up, down, left or right) that holds a checker, capturing it.  You\n\
may capture a friendly or an enemy checker, but afterwards exactly one group\n\
(checkers connected horizontally or vertically, of either colour) may contain\n\
your checkers.  Any group detached by your move that holds only enemy\n\
checkers is removed.  If you have no legal move, your turn is skipped.";

CONST_STRING kHelpStandardObjective =
"Remove all of your opponent's checkers from the board.";

CONST_STRING kHelpReverseObjective = "";

CONST_STRING kHelpTieOccursWhen = /* Should follow 'A Tie occurs when... */ "";

CONST_STRING kHelpExample = "";

/**
 * @brief Tcl-related stuff. Do not change if you do not plan to make a Tcl interface.
 */
void *gGameSpecificTclInit = NULL;
void SetTclCGameSpecificOptions(int theOptions[]) { (void)theOptions; }




/*********** BEGIN GAME-SPECIFIC DEFINITIONS ***********/

#define BLUE 1  /* Player 1, moves first; also cell i's base-3 digit */
#define RED 2

#define BLUE_CHAR 'x'
#define RED_CHAR 'o'
#define EMPTY_CHAR '-'
#define SELECTED_CHAR 's'
#define PASS_CHAR 'P'

enum { UP, RIGHT, DOWN, LEFT };
static const char kDirectionChars[] = "urdl";

#define MAX_CELLS 20  /* the largest variant is 4x5 */

/* Part-moves of a two-click capture (web UI only). Full moves are below 0x100. */
#define SELECT_PART 0x100
#define CAPTURE_PART 0x200
#define INTERMEDIATE_FLAG (1ULL << 63)
#define SELECTED_SHIFT 40

typedef struct {
    int rows;
    int cols;
} Variant;

/* Variant (option) i + 1.  6x6 is too large to solve. */
static const Variant kVariants[] = {{2, 2}, {2, 3}, {3, 3}, {4, 4}, {4, 5}};
#define NUM_VARIANTS ((int) (sizeof(kVariants) / sizeof(kVariants[0])))
#define DEFAULT_VARIANT 3  /* 4x4 */

static int gVariant = DEFAULT_VARIANT;
static int gRows = 4;
static int gCols = 4;
static int gCells = 16;
static uint32_t gFull;      /* every cell */
static uint32_t gNotLeft;   /* every cell but the left column */
static uint32_t gNotRight;  /* every cell but the right column */
static uint32_t gPow3[MAX_CELLS + 1];

/* Compact hash: sorted raw boards of the reachable positions with Blue
   (index 0) and Red (index 1) to move.  A position is its index in
   gRaw[0], or gRawCount[0] + its index in gRaw[1]. */
static uint32_t *gRaw[2] = {NULL, NULL};
static POSITION gRawCount[2] = {0, 0};
static int gTableVariant = -1;

static void ApplyVariant(void);
static void BuildPositionTable(void);
static uint32_t Neighbors(uint32_t cells);
static uint32_t Flood(uint32_t seed, uint32_t occupied);
static int Step(int cell, int direction);
static BOOLEAN Capture(uint32_t own, uint32_t opponent, int source, int target,
                       uint32_t *newOwn, uint32_t *newOpponent);
static int LegalMoves(uint32_t own, uint32_t opponent, MOVE *moves);
static MOVE SkipMove(void);
static void StartBoard(uint32_t *blue, uint32_t *red);
static uint32_t BoardToRaw(uint32_t blue, uint32_t red);
static void RawToBoard(uint32_t raw, uint32_t *blue, uint32_t *red);
static POSITION HashBoard(uint32_t blue, uint32_t red, int turn);
static void UnhashBoard(POSITION position, uint32_t *blue, uint32_t *red, int *turn);
static void SquareName(int cell, char *buffer);

/*********** END GAME-SPECIFIC DEFINITIONS ***********/




/*********** BEGIN SOLVING FUNCIONS ***********/

/**
 * @brief Initialize any global variables.
 */
void InitializeGame(void) {
    gCanonicalPosition = GetCanonicalPosition;

    // The formal position string is '<turn>_<cells>'; the AutoGUI position
    // string adds the selection layer, so they differ.
    gPositionToStringFunPtr = &PositionToString;
    gGenerateMultipartMoveEdgesFunPtr = &GenerateMultipartMoveEdges;

    ApplyVariant();
    BuildPositionTable();
    gNumberOfPositions = gRawCount[0] + gRawCount[1];

    uint32_t blue, red;
    StartBoard(&blue, &red);
    gInitialPosition = HashBoard(blue, red, BLUE);
}

/**
 * @brief Return the head of a list of the legal moves from the input position.
 *
 * @param position The position to branch off of.
 *
 * @return The head of a linked list of the legal moves from the input position.
 *
 * @note Moves are encoded as 32-bit integers. See the MOVE typedef
 * in src/core/types.h.
 *
 * @note It may be helpful to use the CreateMovelistNode() function
 * to assemble the linked list. But remember that this allocates heap space.
 * If you use GenerateMoves in any of the other functions in this file,
 * make sure to free the returned linked list using FreeMoveList().
 * See src/core/misc.c for more information on CreateMovelistNode() and
 * FreeMoveList().
 */
MOVELIST *GenerateMoves(POSITION position) {
    uint32_t blue, red;
    int turn;
    UnhashBoard(position, &blue, &red, &turn);

    MOVE moves[4 * MAX_CELLS];
    int count = (turn == BLUE) ? LegalMoves(blue, red, moves) : LegalMoves(red, blue, moves);

    // CreateMovelistNode() prepends, so add the moves in reverse.
    MOVELIST *moveList = NULL;
    for (int i = count - 1; i >= 0; i--) {
        moveList = CreateMovelistNode(moves[i], moveList);
    }
    return moveList;
}

/**
 * @brief Return the child position reached when the input move
 * is made from the input position.
 *
 * @param position : The old position
 * @param move     : The move to apply.
 *
 * @return The child position, encoded as a 64-bit integer. See
 * the POSITION typedef in src/core/types.h.
 */
POSITION DoMove(POSITION position, MOVE move) {
    uint32_t blue, red;
    int turn;
    UnhashBoard(position, &blue, &red, &turn);

    if (move != SkipMove()) {
        int source = move >> 2;
        int target = Step(source, move & 0b11);
        if (turn == BLUE) {
            Capture(blue, red, source, target, &blue, &red);
        } else {
            Capture(red, blue, source, target, &red, &blue);
        }
    }
    return HashBoard(blue, red, (turn == BLUE) ? RED : BLUE);
}

/**
 * @brief Return win, lose, or tie if the input position
 * is primitive; otherwise return undecided.
 *
 * @param position : The position to inspect.
 *
 * @return an enum; one of (win, lose, tie, undecided). See
 * src/core/types.h for the value enum definition.
 *
 * @note The side to move loses once all of its checkers are gone.
 */
VALUE Primitive(POSITION position) {
    uint32_t blue, red;
    int turn;
    UnhashBoard(position, &blue, &red, &turn);
    uint32_t own = (turn == BLUE) ? blue : red;
    return (own == 0) ? lose : undecided;
}

/**
 * @brief Given a position P, GetCanonicalPosition(P) shall return a
 * position Q such that P is symmetric to Q and
 * GetCanonicalPosition(Q) also returns Q.
 *
 * @note This Q is the "canonical position". For the choice of the
 * canonical position, it is recommended to choose the
 * lowest-hash-value position symmetric to the input position.
 *
 * Ideally, we maximize the number of positions P such that
 * GetCanonicalPosition(P) != P.
 *
 * @param position : The position to inspect.
 */
POSITION GetCanonicalPosition(POSITION position) {
    return position;
}

/*********** END SOLVING FUNCTIONS ***********/






/*********** BEGIN TEXTUI FUNCTIONS ***********/

/**
 * @brief Print the position in a pretty format, including the
 * prediction of the game's outcome.
 *
 * @param position   : The position to pretty-print.
 * @param playerName : The name of the player.
 * @param usersTurn  : TRUE <==> it's a user's turn.
 *
 * @note See GetPrediction() in src/core/gameplay.h to see how
 * to print the prediction of the game's outcome.
 */
void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn) {
    uint32_t blue, red;
    int turn;
    UnhashBoard(position, &blue, &red, &turn);

    // Colour the checkers only on a terminal that supports it.
    const char *term = getenv("TERM");
    BOOLEAN useColor = isatty(fileno(stdout)) && getenv("NO_COLOR") == NULL &&
                       !(term != NULL && strcmp(term, "dumb") == 0);
    const char *blueOn = useColor ? "\033[1;94m" : "";
    const char *redOn = useColor ? "\033[1;91m" : "";
    const char *colorOff = useColor ? "\033[0m" : "";

    printf("\n    ");
    for (int col = 0; col < gCols; col++) printf("+---");
    printf("+\n");
    for (int row = 0; row < gRows; row++) {
        printf("  %d ", gRows - row);
        for (int col = 0; col < gCols; col++) {
            int cell = row * gCols + col;
            if (blue >> cell & 1) {
                printf("| %s%c%s ", blueOn, BLUE_CHAR, colorOff);
            } else if (red >> cell & 1) {
                printf("| %s%c%s ", redOn, RED_CHAR, colorOff);
            } else {
                printf("|   ");
            }
        }
        printf("|\n    ");
        for (int col = 0; col < gCols; col++) printf("+---");
        printf("+\n");
    }
    printf("     ");
    for (int col = 0; col < gCols; col++) printf(" %c  ", 'a' + col);
    printf("\n\n");

    printf("  %s%c Blue%s %d   %s%c Red%s %d   %s to move (%s)\n",
           blueOn, BLUE_CHAR, colorOff, __builtin_popcount(blue),
           redOn, RED_CHAR, colorOff, __builtin_popcount(red),
           playerName, (turn == BLUE) ? "Blue" : "Red");
    printf("  %s\n\n", GetPrediction(position, playerName, usersTurn));
}

/**
 * @brief Find out if the player wants to undo, abort, or neither.
 * If so, return Undo or Abort and don't change `move`.
 * Otherwise, get the new `move` and fill the pointer up.
 *
 * @param position The position the user is at.
 * @param move The move to fill with user's move.
 * @param playerName The name of the player whose turn it is
 *
 * @return One of (Undo, Abort, Continue)
 */
USERINPUT GetAndPrintPlayersMove(POSITION position, MOVE *move, STRING playerName) {
    USERINPUT ret;
    do {
        printf("%8s's move [(undo)/<square><u/d/l/r>, e.g. b3u/(- to skip)] : ", playerName);
        ret = HandleDefaultTextInput(position, move, playerName);
        if (ret != Continue) return ret;
    } while (TRUE);
    return (Continue); /* this is never reached, but lint is now happy */
}

/**
 * @brief Given a move that the user typed while playing a game in the
 * TextUI, return TRUE if the input move string is of the right "form"
 * and can be converted to a move hash.
 *
 * @param input The string input the user typed.
 *
 * @return TRUE iff the input is a valid text input.
 */
BOOLEAN ValidTextInput(STRING input) {
    if (strcmp(input, "-") == 0) {
        return TRUE;
    }
    return strlen(input) == 3 &&
           input[0] >= 'a' && input[0] < 'a' + gCols &&
           input[1] >= '1' && input[1] < '1' + gRows &&
           strchr(kDirectionChars, input[2]) != NULL;
}

/**
 * @brief Convert the string input to the internal move representation.
 *
 * @param input This is a user-inputted move string already validated
 * by ValidTextInput().
 *
 * @return The hash of the move specified by the text input.
 */
MOVE ConvertTextInputToMove(STRING input) {
    if (strcmp(input, "-") == 0) {
        return SkipMove();
    }
    int source = (gRows - (input[1] - '0')) * gCols + (input[0] - 'a');
    int direction = (int) (strchr(kDirectionChars, input[2]) - kDirectionChars);
    return (source << 2) | direction;
}

/**
 * @brief Write a short human-readable string representation
 * of the move to the input buffer.
 *
 * @param move The move hash to convert to a move string.
 *
 * @param moveStringBuffer The buffer to write the move string
 * to.
 *
 * @note The space available in `moveStringBuffer` is MAX_MOVE_STRING_LENGTH
 * (see src/core/autoguistrings.h). Do not write past this limit and ensure
 * that the move string written to `moveStringBuffer` is properly
 * null-terminated.
 *
 * @note A capture is its source square and direction, e.g. 'b3u'; a skip is
 * '-'.  The part-moves of the web UI's two-click captures are the selected
 * square ('b3') and the capture itself ('b3u').
 */
void MoveToString(MOVE move, char *moveStringBuffer) {
    if (move & SELECT_PART) {
        SquareName(move & ~SELECT_PART, moveStringBuffer);
    } else if (move & CAPTURE_PART) {
        MoveToString(move & ~CAPTURE_PART, moveStringBuffer);
    } else if (move == SkipMove()) {
        strcpy(moveStringBuffer, "-");
    } else {
        SquareName(move >> 2, moveStringBuffer);
        size_t length = strlen(moveStringBuffer);
        moveStringBuffer[length] = kDirectionChars[move & 0b11];
        moveStringBuffer[length + 1] = '\0';
    }
}

/**
 * @brief Nicely format the computers move.
 *
 * @param computersMove : The computer's move.
 * @param computersName : The computer's name.
 */
void PrintComputersMove(MOVE computersMove, STRING computersName) {
    char moveStringBuffer[MAX_MOVE_STRING_LENGTH];
    MoveToString(computersMove, moveStringBuffer);
    printf("%s's move: %s\n", computersName, moveStringBuffer);
}

/**
 * @brief Menu used to debug internal problems.
 * Does nothing if kDebugMenu == FALSE.
 */
void DebugMenu(void) {}

/*********** END TEXTUI FUNCTIONS ***********/

/*********** BEGIN VARIANT FUNCTIONS ***********/

/**
 * @return The total number of variants supported.
 */
int NumberOfOptions(void) {
    return NUM_VARIANTS;
}

/**
 * @return The current variant ID.
 */
int getOption(void) {
    return gVariant + 1;
}

/**
 * @brief Set any global variables or data structures according to the
 * variant specified by the input variant ID.
 *
 * @param option An ID specifying the variant that we want to change to.
 *
 * @note The board shape and position table of the new variant are set up
 * together by InitializeGame(), which the core calls before solving or
 * playing, so they never disagree in between.
 */
void setOption(int option) {
    if (option >= 1 && option <= NUM_VARIANTS) {
        gVariant = option - 1;
    }
}

/**
 * @brief Interactive menu used to change the variant, i.e., change
 * game-specific parameters, such as the side-length of a tic-tac-toe
 * board, for example. Does nothing if kGameSpecificMenu == FALSE.
 */
void GameSpecificMenu(void) {
    while (TRUE) {
        printf("\n\t----- Game-specific options for Clusterfuss -----\n\n");
        printf("\tSelect a board size (rows x columns):\n\n");
        for (int i = 0; i < NUM_VARIANTS; i++) {
            printf("\t%d)\t%dx%d %s\n", i + 1, kVariants[i].rows, kVariants[i].cols,
                   (i == gVariant) ? "(currently selected)" : "");
        }
        printf("\n\tb)\t(B)ack to previous menu\n");
        printf("\n\tq)\t(Q)uit\n\n");
        printf("Select an option: ");

        char choice = GetMyChar();
        if (choice >= '1' && choice < '1' + NUM_VARIANTS) {
            setOption(choice - '0');
        } else if (choice == 'b' || choice == 'B') {
            return;
        } else if (choice == 'q' || choice == 'Q') {
            ExitStageRight();
        } else {
            printf("Sorry, I don't know that option.  Try another.\n");
        }
    }
}

/*********** END VARIANT-RELATED FUNCTIONS ***********/





/**
 * @brief Convert the input position to a human-readable formal
 * position string. It must be in STANDARD ONELINE POSITION
 * STRING FORMAT, i.e. it is formatted as <turn>_<board>, where...
 *     `turn` is a character. ('1' if it's Player 1's turn or
 *         '2' if it's Player 2's turn; or '0' if turn is not
 *         encoded in the position because the game is impartial.)
 *     `board` is a string of characters that does NOT contain
 *         colons or semicolons.
 *
 * @param position The position for which to generate the formal
 * position string.
 * @param positionStringBuffer The buffer to write the position string to.
 *
 * @note The space available in `positionStringBuffer` is
 * MAX_POSITION_STRING_LENGTH (see src/core/autoguistrings.h). Do not write
 * past this limit and ensure that the position string written to
 * `positionStringBuffer` is properly null-terminated.
 *
 * @note You need not implement this function if you wish for the
 * AutoGUI Position String to be the same as the Human-Readable Formal
 * Position String. You can in fact delete this function and leave
 * gStringToPositionFunPtr as NULL in InitializeGame().
 *
 * @note The board is one char per cell, row-major from the top-left:
 * 'x' Blue, 'o' Red, '-' empty.
 */
void PositionToString(POSITION position, char *positionStringBuffer) {
    uint32_t blue, red;
    int turn;
    UnhashBoard(position & ~INTERMEDIATE_FLAG & ((1ULL << SELECTED_SHIFT) - 1), &blue, &red, &turn);

    char board[MAX_CELLS + 1];
    for (int cell = 0; cell < gCells; cell++) {
        board[cell] = (blue >> cell & 1) ? BLUE_CHAR : (red >> cell & 1) ? RED_CHAR : EMPTY_CHAR;
    }
    board[gCells] = '\0';
    AutoGUIMakePositionString(turn, board, positionStringBuffer);
}

/**
 * @brief Convert the input position string to
 * the in-game integer representation of the position.
 *
 * @param positionString The position string. This is guaranteed
 * to be shorter than MAX_POSITION_STRING_LENGTH (see src/core/autoguistrings.h).
 *
 * @return If the position string is valid, return the corresponding
 * position hash. Otherwise, return NULL_POSITION.
 *
 * @note DO NOT MODIFY any characters in `positionString`, or, at least,
 * ensure that at the conclusion of this function, the characters
 * are the same as before.
 *
 * @note Positions that cannot be reached from the start are not in the
 * compact hash and are rejected.
 */
POSITION StringToPosition(char *positionString) {
    int turn;
    char *board;
    if (!ParseStandardOnelinePositionString(positionString, &turn, &board) ||
        (turn != BLUE && turn != RED) || (int) strlen(board) != gCells) {
        return NULL_POSITION;
    }

    uint32_t blue = 0, red = 0;
    for (int cell = 0; cell < gCells; cell++) {
        if (board[cell] == BLUE_CHAR) {
            blue |= 1u << cell;
        } else if (board[cell] == RED_CHAR) {
            red |= 1u << cell;
        } else if (board[cell] != EMPTY_CHAR) {
            return NULL_POSITION;
        }
    }
    return HashBoard(blue, red, turn);
}

/**
 * @brief Write an AutoGUI-formatted position string for the given position
 * (which tells the frontend application how to render the position) to the
 * input buffer.
 *
 * @param position The position for which to generate the AutoGUI
 * position string.
 * @param autoguiPositionStringBuffer The buffer to write the AutoGUI
 * position string to.
 *
 * @note You may find AutoGUIMakePositionString() helpful.
 * (See src/core/autoguistrings.h)
 *
 * @note The space available in `autoguiPositionStringBuffer` is
 * MAX_POSITION_STRING_LENGTH (see src/core/autoguistrings.h). Do not write
 * past this limit and ensure that the position string written to
 * `autoguiPositionStringBuffer` is properly null-terminated.
 * AutoGUIMakePositionString() should handle the null-terminator,
 * if you choose to use it.
 *
 * @note If the game is impartial and a turn is not encoded, set the turn
 * character (which is the first character) of autoguiPositionStringBuffer
 * to '0'.
 *
 * @note One char per cell, then the selection layer: one char per cell,
 * 's' under the checker selected by the first click of a capture.
 */
void PositionToAutoGUIString(POSITION position, char *autoguiPositionStringBuffer) {
    int selected = -1;
    if (position & INTERMEDIATE_FLAG) {
        selected = (int) ((position & ~INTERMEDIATE_FLAG) >> SELECTED_SHIFT);
        position &= (1ULL << SELECTED_SHIFT) - 1;
    }

    uint32_t blue, red;
    int turn;
    UnhashBoard(position, &blue, &red, &turn);

    char board[2 * MAX_CELLS + 1];
    for (int cell = 0; cell < gCells; cell++) {
        board[cell] = (blue >> cell & 1) ? BLUE_CHAR : (red >> cell & 1) ? RED_CHAR : EMPTY_CHAR;
        board[gCells + cell] = (cell == selected) ? SELECTED_CHAR : EMPTY_CHAR;
    }
    board[2 * gCells] = '\0';
    AutoGUIMakePositionString(turn, board, autoguiPositionStringBuffer);
}

/**
 * @brief Write an AutoGUI-formatted move string for the given move
 * (which tells the frontend application how to render the move as button) to the
 * input buffer.
 *
 * @param position   : The position from which the move is made. This is helpful
 * if the move button depends on not only the move hash but also the position.
 * @param move : The move hash from which the AutoGUI move string is generated.
 * @param autoguiMoveStringBuffer : The buffer to write the AutoGUI
 * move string to.
 *
 * @note The space available in `autoguiMoveStringBuffer` is MAX_MOVE_STRING_LENGTH
 * (see src/core/autoguistrings.h). Do not write past this limit and ensure that
 * the move string written to `moveStringBuffer` is properly null-terminated.
 *
 * @note You may find the "AutoGUIMakeMoveButton" functions helpful.
 * (See src/core/autoguistrings.h)
 *
 * @note A capture is only made through its two part-moves, so the full
 * move itself has no button.
 */
void MoveToAutoGUIString(POSITION position, MOVE move, char *autoguiMoveStringBuffer) {
    (void) position;
    if (move & SELECT_PART) {
        AutoGUIMakeMoveButtonStringA('-', move & ~SELECT_PART, '-', autoguiMoveStringBuffer);
    } else if (move & CAPTURE_PART) {
        int source = (move & ~CAPTURE_PART) >> 2;
        AutoGUIMakeMoveButtonStringM(source, Step(source, move & 0b11), 'x', autoguiMoveStringBuffer);
    } else if (move == SkipMove()) {
        AutoGUIMakeMoveButtonStringA(PASS_CHAR, 2 * gCells, '-', autoguiMoveStringBuffer);
    } else {
        AutoGUIWriteEmptyString(autoguiMoveStringBuffer);
    }
}

/**
 * @brief Split every capture into two part-moves for the web UI: select the
 * checker, then capture.  Called with the moves GenerateMoves() returned and
 * their child positions.
 */
MULTIPARTEDGELIST *GenerateMultipartMoveEdges(POSITION position, MOVELIST *moveList,
                                              POSITIONLIST *positionList) {
    (void) positionList;
    MULTIPARTEDGELIST *edges = NULL;
    uint32_t selectable = 0;  // checkers that already have a select edge
    for (; moveList != NULL; moveList = moveList->next) {
        MOVE move = moveList->move;
        if (move == SkipMove()) {
            continue;
        }
        int source = move >> 2;
        POSITION selected = INTERMEDIATE_FLAG | ((POSITION) source << SELECTED_SHIFT) | position;
        if (!(selectable >> source & 1)) {
            selectable |= 1u << source;
            edges = CreateMultipartEdgeListNode(NULL_POSITION, selected, SELECT_PART | source, 0, edges);
        }
        edges = CreateMultipartEdgeListNode(selected, NULL_POSITION, CAPTURE_PART | move, move, edges);
    }
    return edges;
}




/*********** BEGIN GAME-SPECIFIC HELPERS ***********/

/**
 * @brief Set the board-shape globals of the current variant.
 */
static void ApplyVariant(void) {
    gRows = kVariants[gVariant].rows;
    gCols = kVariants[gVariant].cols;
    gCells = gRows * gCols;

    gFull = (gCells == 32) ? 0xFFFFFFFFu : (1u << gCells) - 1;
    uint32_t leftColumn = 0, rightColumn = 0;
    for (int row = 0; row < gRows; row++) {
        leftColumn |= 1u << (row * gCols);
        rightColumn |= 1u << (row * gCols + gCols - 1);
    }
    gNotLeft = gFull & ~leftColumn;
    gNotRight = gFull & ~rightColumn;

    gPow3[0] = 1;
    for (int i = 1; i <= gCells; i++) {
        gPow3[i] = gPow3[i - 1] * 3;
    }
}

/**
 * @brief The cells orthogonally adjacent to any cell in `cells`.
 */
static uint32_t Neighbors(uint32_t cells) {
    return (((cells << 1) & gNotLeft) | ((cells >> 1) & gNotRight) |
            (cells << gCols) | (cells >> gCols)) & gFull;
}

/**
 * @brief The group of `occupied` cells connected to `seed`.
 */
static uint32_t Flood(uint32_t seed, uint32_t occupied) {
    uint32_t group = seed & occupied, previous;
    do {
        previous = group;
        group |= Neighbors(group) & occupied;
    } while (group != previous);
    return group;
}

/**
 * @brief The cell one step from `cell` in `direction`, or -1 off the board.
 */
static int Step(int cell, int direction) {
    int row = cell / gCols, col = cell % gCols;
    switch (direction) {
        case UP: return (row > 0) ? cell - gCols : -1;
        case RIGHT: return (col < gCols - 1) ? cell + 1 : -1;
        case DOWN: return (row < gRows - 1) ? cell + gCols : -1;
        case LEFT: return (col > 0) ? cell - 1 : -1;
        default: return -1;
    }
}

/**
 * @brief Capture from `source` onto `target` for the side owning `own`.
 * Returns FALSE if the mover's checkers would end up in more than one group
 * (illegal).  Otherwise writes the new boards: only the group holding the
 * mover's checkers stays, since every other group is enemy-only and detached
 * by the move (the board is one group before every move).
 */
static BOOLEAN Capture(uint32_t own, uint32_t opponent, int source, int target,
                       uint32_t *newOwn, uint32_t *newOpponent) {
    uint32_t mover = (own & ~(1u << source)) | (1u << target);
    uint32_t other = opponent & ~(1u << target);
    uint32_t group = Flood(mover & (~mover + 1), mover | other);
    if (mover & ~group) {
        return FALSE;
    }
    *newOwn = mover;
    *newOpponent = other & group;
    return TRUE;
}

/**
 * @brief Write the legal moves of the side owning `own` into `moves` (room for
 * 4 * MAX_CELLS) and return how many there are.  None if the mover has no
 * checkers (a primitive loss); a skip if no capture is legal.
 */
static int LegalMoves(uint32_t own, uint32_t opponent, MOVE *moves) {
    if (own == 0) {
        return 0;
    }
    uint32_t occupied = own | opponent;
    int count = 0;
    // A capture stays inside its own group, so if two groups hold the
    // mover's checkers, both still do after any move: nothing is legal.
    if ((own & ~Flood(own & (~own + 1), occupied)) == 0) {
        for (int source = 0; source < gCells; source++) {
            if (!(own >> source & 1)) {
                continue;
            }
            for (int direction = UP; direction <= LEFT; direction++) {
                int target = Step(source, direction);
                uint32_t newOwn, newOpponent;
                if (target >= 0 && (occupied >> target & 1) &&
                    Capture(own, opponent, source, target, &newOwn, &newOpponent)) {
                    moves[count++] = (source << 2) | direction;
                }
            }
        }
    }
    if (count == 0) {
        moves[count++] = SkipMove();
    }
    return count;
}

static MOVE SkipMove(void) {
    return gCells << 2;
}

/**
 * @brief The start board: Blue wherever (row + col) is even, Red elsewhere,
 * and on an odd x odd board the centre square left empty.
 */
static void StartBoard(uint32_t *blue, uint32_t *red) {
    *blue = *red = 0;
    for (int cell = 0; cell < gCells; cell++) {
        if (gCells % 2 == 1 && cell == gCells / 2) {
            continue;
        }
        if ((cell / gCols + cell % gCols) % 2 == 0) {
            *blue |= 1u << cell;
        } else {
            *red |= 1u << cell;
        }
    }
}

static uint32_t BoardToRaw(uint32_t blue, uint32_t red) {
    uint32_t raw = 0;
    for (int cell = gCells - 1; cell >= 0; cell--) {
        raw = raw * 3 + ((blue >> cell & 1) ? BLUE : (red >> cell & 1) ? RED : 0);
    }
    return raw;
}

static void RawToBoard(uint32_t raw, uint32_t *blue, uint32_t *red) {
    *blue = *red = 0;
    for (int cell = 0; cell < gCells; cell++, raw /= 3) {
        if (raw % 3 == BLUE) {
            *blue |= 1u << cell;
        } else if (raw % 3 == RED) {
            *red |= 1u << cell;
        }
    }
}

/**
 * @brief The compact hash of a board, or NULL_POSITION if it is not reachable.
 */
static POSITION HashBoard(uint32_t blue, uint32_t red, int turn) {
    int side = (turn == BLUE) ? 0 : 1;
    uint32_t raw = BoardToRaw(blue, red);
    POSITION low = 0, high = gRawCount[side];
    while (low < high) {
        POSITION middle = low + (high - low) / 2;
        if (gRaw[side][middle] < raw) {
            low = middle + 1;
        } else {
            high = middle;
        }
    }
    if (low == gRawCount[side] || gRaw[side][low] != raw) {
        return NULL_POSITION;
    }
    return (side == 0) ? low : gRawCount[0] + low;
}

static void UnhashBoard(POSITION position, uint32_t *blue, uint32_t *red, int *turn) {
    if (position < gRawCount[0]) {
        *turn = BLUE;
        RawToBoard(gRaw[0][position], blue, red);
    } else {
        *turn = RED;
        RawToBoard(gRaw[1][position - gRawCount[0]], blue, red);
    }
}

static void SquareName(int cell, char *buffer) {
    sprintf(buffer, "%c%d", 'a' + cell % gCols, gRows - cell / gCols);
}

/* Cache file of the position table: a header, then gRaw[0] and gRaw[1]. */
#define TABLE_MAGIC "CFPOS001"

typedef struct {
    char magic[8];
    int32_t rows;
    int32_t cols;
    uint64_t counts[2];
} TableHeader;

static void TableFileName(char *buffer) {
    sprintf(buffer, "./data/m%s_%d_positions.dat.gz", kDBName, getOption());
}

static BOOLEAN LoadPositionTable(void) {
    char fileName[256];
    TableFileName(fileName);
    gzFile file = gzopen(fileName, "rb");
    if (file == NULL) {
        return FALSE;
    }

    TableHeader header;
    BOOLEAN ok = gzread(file, &header, sizeof(header)) == sizeof(header) &&
                 memcmp(header.magic, TABLE_MAGIC, sizeof(header.magic)) == 0 &&
                 header.rows == gRows && header.cols == gCols;
    for (int side = 0; ok && side < 2; side++) {
        gRawCount[side] = header.counts[side];
        gRaw[side] = (uint32_t *) SafeMalloc(sizeof(uint32_t) * (gRawCount[side] + 1));
        size_t bytes = sizeof(uint32_t) * gRawCount[side];
        ok = gzfread(gRaw[side], 1, bytes, file) == bytes;
    }
    gzclose(file);
    return ok;
}

static void SavePositionTable(void) {
    char fileName[256], tempName[300];
    TableFileName(fileName);
    sprintf(tempName, "%s.%d.tmp", fileName, (int) getpid());
    mkdir("data", 0755);

    gzFile file = gzopen(tempName, "wb");
    if (file == NULL) {
        return;
    }
    TableHeader header;
    memcpy(header.magic, TABLE_MAGIC, sizeof(header.magic));
    header.rows = gRows;
    header.cols = gCols;
    header.counts[0] = gRawCount[0];
    header.counts[1] = gRawCount[1];
    BOOLEAN ok = gzwrite(file, &header, sizeof(header)) == sizeof(header);
    for (int side = 0; ok && side < 2; side++) {
        size_t bytes = sizeof(uint32_t) * gRawCount[side];
        ok = gzfwrite(gRaw[side], 1, bytes, file) == bytes;
    }
    if (gzclose(file) == Z_OK && ok) {
        rename(tempName, fileName);  // atomic, so readers never see half a file
    } else {
        remove(tempName);
    }
}

static void FreePositionTable(void) {
    for (int side = 0; side < 2; side++) {
        if (gRaw[side] != NULL) {
            SafeFreeAndSetToNull((GENERIC_PTR *) &gRaw[side]);
        }
        gRawCount[side] = 0;
    }
    gTableVariant = -1;
}

/**
 * @brief Enumerate the positions reachable from the start, depth first,
 * marking them in one bitset per side to move over all raw boards.  Scanning
 * the bitsets in order then yields the sorted tables directly.
 */
static void EnumeratePositionTable(void) {
    uint64_t space = gPow3[gCells];  // raw boards per side to move
    uint64_t words = (space + 63) / 64;
    uint64_t *seen[2];
    for (int side = 0; side < 2; side++) {
        seen[side] = (uint64_t *) SafeCalloc(words, sizeof(uint64_t));
    }

    // Stack of (raw << 1) | side.  Each position is pushed once.
    uint64_t capacity = 1 << 16, size = 0;
    uint64_t *stack = (uint64_t *) SafeMalloc(sizeof(uint64_t) * capacity);

    uint32_t blue, red;
    StartBoard(&blue, &red);
    uint32_t startRaw = BoardToRaw(blue, red);
    seen[0][startRaw / 64] |= 1ULL << (startRaw % 64);
    stack[size++] = (uint64_t) startRaw << 1;

    MOVE moves[4 * MAX_CELLS];
    while (size > 0) {
        uint64_t key = stack[--size];
        int side = (int) (key & 1);
        RawToBoard((uint32_t) (key >> 1), &blue, &red);
        uint32_t own = (side == 0) ? blue : red;
        uint32_t opponent = (side == 0) ? red : blue;

        int count = LegalMoves(own, opponent, moves);
        for (int i = 0; i < count; i++) {
            uint32_t newOwn = own, newOpponent = opponent;
            if (moves[i] != SkipMove()) {
                int source = moves[i] >> 2;
                Capture(own, opponent, source, Step(source, moves[i] & 0b11), &newOwn, &newOpponent);
            }
            int childSide = 1 - side;
            uint32_t childRaw = (childSide == 0) ? BoardToRaw(newOpponent, newOwn)
                                                 : BoardToRaw(newOwn, newOpponent);
            uint64_t *bits = &seen[childSide][childRaw / 64];
            uint64_t bit = 1ULL << (childRaw % 64);
            if (*bits & bit) {
                continue;
            }
            *bits |= bit;
            if (size == capacity) {
                capacity *= 2;
                uint64_t *bigger = (uint64_t *) SafeMalloc(sizeof(uint64_t) * capacity);
                memcpy(bigger, stack, sizeof(uint64_t) * size);
                SafeFree(stack);
                stack = bigger;
            }
            stack[size++] = ((uint64_t) childRaw << 1) | (uint64_t) childSide;
        }
    }
    SafeFree(stack);

    for (int side = 0; side < 2; side++) {
        POSITION count = 0;
        for (uint64_t w = 0; w < words; w++) {
            count += __builtin_popcountll(seen[side][w]);
        }
        gRawCount[side] = count;
        gRaw[side] = (uint32_t *) SafeMalloc(sizeof(uint32_t) * (count + 1));
        POSITION index = 0;
        for (uint64_t w = 0; w < words; w++) {
            for (uint64_t bits = seen[side][w]; bits != 0; bits &= bits - 1) {
                gRaw[side][index++] = (uint32_t) (w * 64 + __builtin_ctzll(bits));
            }
        }
        SafeFree(seen[side]);
    }
}

/**
 * @brief Make the position table of the current variant available: keep it
 * if it is already built, else load it from its cache file, else enumerate it
 * and write the cache file.
 */
static void BuildPositionTable(void) {
    if (gTableVariant == gVariant) {
        return;
    }
    FreePositionTable();
    if (!LoadPositionTable()) {
        FreePositionTable();
        EnumeratePositionTable();
        SavePositionTable();
    }
    gTableVariant = gVariant;
}

/*********** END GAME-SPECIFIC HELPERS ***********/
