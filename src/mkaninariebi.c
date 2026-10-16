/************************************************************************
**
** NAME:        mkaninariebi.c
**
** DESCRIPTION: Kani Nari Ebi
**
** AUTHOR:      GamesCrafters
**
** DATE:        2026-10-14
**
************************************************************************/

#include "gamesman.h"

CONST_STRING kAuthorName = "GamesCrafters";
CONST_STRING kGameName = "Kani Nari Ebi";
CONST_STRING kDBName = "kaninariebi";

/**
 * @brief An upper bound on the number of reachable positions.
 *
 * @details The hash value of every reachable position must be less
 * than `gNumberOfPositions`.
 */
POSITION gNumberOfPositions = 2330268745802ULL;

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
BOOLEAN kTieIsPossible = TRUE;

/**
 * @brief Whether the game is loopy. It is TRUE if there exists a position
 * P in the game such that, there is a sequence of N >= 1 moves one can
 * make starting from P that allows them to revisit P.
 */
BOOLEAN kLoopy = TRUE;

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
POSITION ActualNumberOfPositions(int);

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
CONST_STRING kHelpGraphicInterface = "Move a crab horizontally or a shrimp diagonally. A move landing in the sea-current column must choose up or down. Bonded pieces may make one optional diagonal bonus move.";
CONST_STRING kHelpTextInterface = "Enter a move as source square followed by destination square (for example, a5b5). Add u or d for the sea current, p for promotion, and - followed by a second move for a Bond bonus.";
CONST_STRING kHelpOnYourTurn = "Move one of your crabs any unobstructed distance horizontally, or one of your shrimps one diagonal square. A landing in the center sea-current column requires an up/down choice; crabs reaching the opponent's home column may promote. Same-type connected pieces may Bond for an optional diagonal move (same-player pieces in the mixed variant).";
CONST_STRING kHelpReverseObjective = "";
CONST_STRING kHelpStandardObjective = "Win by promoting enough crabs to shrimps, capturing enough opposing pieces, or leaving the opponent with no legal normal move.";
CONST_STRING kHelpTieOccursWhen = /* Should follow 'A Tie occurs when... */ "the position repeats in this loopy game.";
CONST_STRING kHelpExample = "";

/**
 * @brief Tcl-related stuff. Do not change if you do not plan to make a Tcl interface.
 */
void *gGameSpecificTclInit = NULL;
void SetTclCGameSpecificOptions(int theOptions[]) { (void)theOptions; }




/*********** BEGIN SOLVING FUNCTIONS ***********/

#define MAX_BOARD_SIZE 25
#define PART_BITS 13
#define PART_MASK ((1U << PART_BITS) - 1U)
#define BOND_FLAG (1U << 26)

enum {
    EMPTY = 0,
    P1_CRAB = 1,
    P1_SHRIMP = 2,
    P2_CRAB = 3,
    P2_SHRIMP = 4
};

enum {
    CURRENT_NONE = 0,
    CURRENT_UP = 1,
    CURRENT_DOWN = 2
};

typedef struct {
    int board[MAX_BOARD_SIZE];
    int finalSquare;
    int current;
    int promote;
} LANDING;

static int gVariant = 0;
static int gRows = 5;
static int gCols = 5;
static int gBoardSize = 25;
static int gInitialPieces = 5;
static int gWinCount = 3;
static int gCurrentCol = 2;

static int ValidPieceConfiguration(int *pieceCounts) {
    return pieceCounts[P1_CRAB] + pieceCounts[P1_SHRIMP] <= gInitialPieces &&
        pieceCounts[P2_CRAB] + pieceCounts[P2_SHRIMP] <= gInitialPieces;
}

static int IndexOf(int row, int col) {
    return row * gCols + col;
}

static int OnBoard(int row, int col) {
    return row >= 0 && row < gRows && col >= 0 && col < gCols;
}

static int OwnerOf(int piece) {
    if (piece == P1_CRAB || piece == P1_SHRIMP) return 1;
    if (piece == P2_CRAB || piece == P2_SHRIMP) return 2;
    return 0;
}

static int IsCrab(int piece) {
    return piece == P1_CRAB || piece == P2_CRAB;
}

static int IsShrimp(int piece) {
    return piece == P1_SHRIMP || piece == P2_SHRIMP;
}

static int ShrimpFor(int player) {
    return player == 1 ? P1_SHRIMP : P2_SHRIMP;
}

static POSITION Binomial(int n, int k) {
    POSITION result = 1;
    int i;
    if (k < 0 || k > n) return 0;
    if (k > n - k) k = n - k;
    for (i = 1; i <= k; ++i) {
        result = result * (POSITION)(n - k + i) / (POSITION)i;
    }
    return result;
}

static POSITION CompletionCountFor(int squares, int playerOnePieces,
                                   int playerTwoPieces, int maxPieces) {
    POSITION count = 0;
    int one, two;

    /* Count completions while respecting each player's starting material. */
    for (one = 0; one <= maxPieces - playerOnePieces; ++one) {
        for (two = 0; two <= maxPieces - playerTwoPieces; ++two) {
            count += Binomial(squares, one) *
                Binomial(squares - one, two) *
                ((POSITION)1 << (one + two));
        }
    }
    return count;
}

static POSITION HashBoard(const int board[MAX_BOARD_SIZE], int turn) {
    int playerOnePieces = 0;
    int playerTwoPieces = 0;
    char genericBoard[MAX_BOARD_SIZE];
    int square;

    if (turn != 1 && turn != 2) return NULL_POSITION;
    for (square = 0; square < gBoardSize; ++square) {
        int piece = board[square];
        if (piece < EMPTY || piece > P2_SHRIMP) return NULL_POSITION;
        genericBoard[square] = (char)piece;
        if (OwnerOf(piece) == 1) ++playerOnePieces;
        if (OwnerOf(piece) == 2) ++playerTwoPieces;
        if (playerOnePieces > gInitialPieces ||
            playerTwoPieces > gInitialPieces) {
            return NULL_POSITION;
        }
    }
    return generic_hash_hash(genericBoard, turn);
}

static int UnhashPosition(POSITION position, int board[MAX_BOARD_SIZE]) {
    char genericBoard[MAX_BOARD_SIZE];
    int square;

    generic_hash_unhash(position, genericBoard);
    for (square = 0; square < gBoardSize; ++square) {
        board[square] = (unsigned char)genericBoard[square];
    }
    return generic_hash_turn(position);
}

static unsigned int PackPart(int src, int dest, int current, int promote) {
    return (unsigned int)src | ((unsigned int)dest << 5) |
        ((unsigned int)current << 10) | ((unsigned int)promote << 12);
}

static void UnpackPart(unsigned int part, int *src, int *dest,
                       int *current, int *promote) {
    *src = (int)(part & 31U);
    *dest = (int)((part >> 5) & 31U);
    *current = (int)((part >> 10) & 3U);
    *promote = (int)((part >> 12) & 1U);
}

static unsigned int PackMove(unsigned int mainPart, unsigned int bondPart,
                             int hasBond) {
    return mainPart | (hasBond ? (bondPart << PART_BITS) | BOND_FLAG : 0U);
}

static void MovePiece(int board[MAX_BOARD_SIZE], int src, int dest) {
    board[dest] = board[src];
    board[src] = EMPTY;
}

static int NormalDestinations(const int board[MAX_BOARD_SIZE], int src,
                              int destinations[MAX_BOARD_SIZE]) {
    static const int diagonal[4][2] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
    int count = 0;
    int piece = board[src];
    int row = src / gCols;
    int col = src % gCols;

    if (IsCrab(piece)) {
        int direction;
        for (direction = -1; direction <= 1; direction += 2) {
            int nextCol = col + direction;
            while (nextCol >= 0 && nextCol < gCols) {
                int dest = IndexOf(row, nextCol);
                if (board[dest] != EMPTY) break;
                destinations[count++] = dest;
                nextCol += direction;
            }
        }
    } else if (IsShrimp(piece)) {
        int i;
        for (i = 0; i < 4; ++i) {
            int nextRow = row + diagonal[i][0];
            int nextCol = col + diagonal[i][1];
            if (OnBoard(nextRow, nextCol)) {
                int dest = IndexOf(nextRow, nextCol);
                if (board[dest] == EMPTY) destinations[count++] = dest;
            }
        }
    }
    return count;
}

static int BondDestinations(const int board[MAX_BOARD_SIZE], int src,
                            int destinations[MAX_BOARD_SIZE]) {
    static const int diagonal[4][2] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
    int count = 0;
    int row = src / gCols;
    int col = src % gCols;
    int i;

    for (i = 0; i < 4; ++i) {
        int nextRow = row + diagonal[i][0];
        int nextCol = col + diagonal[i][1];
        if (OnBoard(nextRow, nextCol)) {
            int dest = IndexOf(nextRow, nextCol);
            if (board[dest] == EMPTY) destinations[count++] = dest;
        }
    }
    return count;
}

static int CurrentDestination(const int board[MAX_BOARD_SIZE], int src,
                              int current) {
    int row = src / gCols;
    int col = src % gCols;
    int direction;

    if (col != gCurrentCol || current == CURRENT_NONE) return src;
    direction = current == CURRENT_UP ? -1 : 1;
    while (OnBoard(row + direction, col) &&
           board[IndexOf(row + direction, col)] == EMPTY) {
        row += direction;
    }
    return IndexOf(row, col);
}

static int ExpandLanding(const int board[MAX_BOARD_SIZE], int src, int dest,
                         int player, LANDING results[4]) {
    int moved[MAX_BOARD_SIZE];
    int currentChoices[2];
    int choiceCount;
    int resultCount = 0;
    int choice;

    memcpy(moved, board, (size_t)gBoardSize * sizeof(int));
    MovePiece(moved, src, dest);
    if (dest % gCols == gCurrentCol) {
        currentChoices[0] = CURRENT_UP;
        currentChoices[1] = CURRENT_DOWN;
        choiceCount = 2;
    } else {
        currentChoices[0] = CURRENT_NONE;
        choiceCount = 1;
    }

    for (choice = 0; choice < choiceCount; ++choice) {
        int current = currentChoices[choice];
        int afterCurrent[MAX_BOARD_SIZE];
        int finalSquare = dest;
        int canPromote;
        memcpy(afterCurrent, moved, (size_t)gBoardSize * sizeof(int));
        if (current != CURRENT_NONE) {
            finalSquare = CurrentDestination(afterCurrent, dest, current);
            if (finalSquare != dest) MovePiece(afterCurrent, dest, finalSquare);
        }
        canPromote = IsCrab(afterCurrent[finalSquare]) &&
            finalSquare % gCols == (player == 1 ? gCols - 1 : 0);

        memcpy(results[resultCount].board, afterCurrent,
               (size_t)gBoardSize * sizeof(int));
        results[resultCount].finalSquare = finalSquare;
        results[resultCount].current = current;
        results[resultCount].promote = 0;
        ++resultCount;

        if (canPromote) {
            memcpy(results[resultCount].board, afterCurrent,
                   (size_t)gBoardSize * sizeof(int));
            results[resultCount].board[finalSquare] = ShrimpFor(player);
            results[resultCount].finalSquare = finalSquare;
            results[resultCount].current = current;
            results[resultCount].promote = 1;
            ++resultCount;
        }
    }
    return resultCount;
}

static int BondGroup(const int board[MAX_BOARD_SIZE], int start,
                     int group[MAX_BOARD_SIZE]) {
    static const int orthogonal[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    int visited[MAX_BOARD_SIZE] = {0};
    int queue[MAX_BOARD_SIZE];
    int head = 0;
    int tail = 0;
    int piece = board[start];
    int player = OwnerOf(piece);
    int i;

    if (piece == EMPTY) return 0;
    visited[start] = 1;
    queue[tail++] = start;
    while (head < tail) {
        int current = queue[head++];
        for (i = 0; i < 4; ++i) {
            int row = current / gCols + orthogonal[i][0];
            int col = current % gCols + orthogonal[i][1];
            if (OnBoard(row, col)) {
                int neighbor = IndexOf(row, col);
                int matches = gVariant == 1
                    ? OwnerOf(board[neighbor]) == player
                    : board[neighbor] == piece;
                if (!visited[neighbor] && matches) {
                    visited[neighbor] = 1;
                    queue[tail++] = neighbor;
                }
            }
        }
    }

    head = 0;
    for (i = 0; i < gBoardSize; ++i) {
        if (visited[i]) group[head++] = i;
    }
    return head;
}

static void ResolveCaptures(int board[MAX_BOARD_SIZE], int movedSquare,
                            int player) {
    static const int orthogonal[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    int captured[MAX_BOARD_SIZE] = {0};
    int i;

    for (i = 0; i < 4; ++i) {
        int row = movedSquare / gCols + orthogonal[i][0];
        int col = movedSquare % gCols + orthogonal[i][1];
        int line[MAX_BOARD_SIZE];
        int lineCount = 0;
        while (OnBoard(row, col)) {
            int square = IndexOf(row, col);
            int owner = OwnerOf(board[square]);
            if (owner == 3 - player) {
                line[lineCount++] = square;
                row += orthogonal[i][0];
                col += orthogonal[i][1];
                continue;
            }
            if (owner == player && lineCount > 0) {
                int j;
                for (j = 0; j < lineCount; ++j) captured[line[j]] = 1;
            }
            break;
        }
    }
    for (i = 0; i < gBoardSize; ++i) {
        if (captured[i]) board[i] = EMPTY;
    }
}

static int HasEnoughShrimps(const int board[MAX_BOARD_SIZE], int player) {
    int count = 0;
    int i;
    for (i = 0; i < gBoardSize; ++i) {
        if (board[i] == ShrimpFor(player)) ++count;
    }
    return count >= gWinCount;
}

static int CapturedEnough(const int board[MAX_BOARD_SIZE], int player) {
    int opponentCount = 0;
    int i;
    for (i = 0; i < gBoardSize; ++i) {
        if (OwnerOf(board[i]) == 3 - player) ++opponentCount;
    }
    return opponentCount <= gInitialPieces - gWinCount;
}

static int HasAnyNormalMove(const int board[MAX_BOARD_SIZE], int player) {
    int destinations[MAX_BOARD_SIZE];
    int src;
    for (src = 0; src < gBoardSize; ++src) {
        if (OwnerOf(board[src]) == player &&
            NormalDestinations(board, src, destinations) > 0) {
            return 1;
        }
    }
    return 0;
}

static int WinsImmediately(const int board[MAX_BOARD_SIZE], int player) {
    return HasEnoughShrimps(board, player) || CapturedEnough(board, player) ||
        !HasAnyNormalMove(board, 3 - player);
}

static size_t MoveHashIndex(MOVE move, size_t capacity) {
    unsigned int hash = (unsigned int)move;
    hash ^= hash >> 16;
    hash *= 0x7feb352dU;
    hash ^= hash >> 15;
    hash *= 0x846ca68bU;
    hash ^= hash >> 16;
    return (size_t)hash & (capacity - 1);
}

static void InsertSeenMove(MOVE *seen, size_t capacity, MOVE move) {
    size_t index = MoveHashIndex(move, capacity);
    while (seen[index] != -1) index = (index + 1) & (capacity - 1);
    seen[index] = move;
}

static int AddUniqueMove(MOVELIST **moves, MOVELIST **tail, MOVE move,
                         MOVE **seen, size_t *count, size_t *capacity) {
    size_t index;
    if (*capacity == 0 || (*count + 1) * 2 > *capacity) {
        size_t newCapacity = *capacity == 0 ? 64 : *capacity * 2;
        MOVE *newSeen = (MOVE *)SafeMalloc(newCapacity * sizeof(MOVE));
        size_t i;
        for (i = 0; i < newCapacity; ++i) newSeen[i] = -1;
        for (i = 0; i < *capacity; ++i) {
            if ((*seen)[i] != -1) InsertSeenMove(newSeen, newCapacity, (*seen)[i]);
        }
        if (*seen != NULL) SafeFree(*seen);
        *seen = newSeen;
        *capacity = newCapacity;
    }
    index = MoveHashIndex(move, *capacity);
    while ((*seen)[index] != -1) {
        if ((*seen)[index] == move) return 0;
        index = (index + 1) & (*capacity - 1);
    }
    (*seen)[index] = move;
    ++*count;
    if (*moves == NULL) {
        *moves = CreateMovelistNode(move, NULL);
        *tail = *moves;
    } else {
        (*tail)->next = CreateMovelistNode(move, NULL);
        *tail = (*tail)->next;
    }
    return 1;
}

static void ApplyPart(int board[MAX_BOARD_SIZE], unsigned int part,
                      int player) {
    int src, dest, current, promote;
    int finalSquare;
    UnpackPart(part, &src, &dest, &current, &promote);
    MovePiece(board, src, dest);
    finalSquare = current == CURRENT_NONE
        ? dest : CurrentDestination(board, dest, current);
    if (finalSquare != dest) MovePiece(board, dest, finalSquare);
    if (promote) board[finalSquare] = ShrimpFor(player);
    ResolveCaptures(board, finalSquare, player);
}

static POSITION StartingPosition(void) {
    int board[MAX_BOARD_SIZE] = {0};
    int row;
    for (row = 0; row < gRows; ++row) {
        board[IndexOf(row, 0)] = P1_CRAB;
        board[IndexOf(row, gCols - 1)] = P2_CRAB;
    }
    return HashBoard(board, 1);
}

static void ConfigureVariant(int option) {
    int pieces[] = {
        EMPTY, 0, 25,
        P1_CRAB, 0, 5,
        P1_SHRIMP, 0, 5,
        P2_CRAB, 0, 5,
        P2_SHRIMP, 0, 5,
        -1
    };

    gVariant = option;
    gRows = option == 2 ? 3 : 5;
    gCols = option == 3 ? 3 : 5;
    gBoardSize = gRows * gCols;
    gInitialPieces = gRows;
    gWinCount = option == 2 ? 2 : 3;
    gCurrentCol = gCols / 2;
    pieces[2] = gBoardSize;
    pieces[5] = gInitialPieces;
    pieces[8] = gInitialPieces;
    pieces[11] = gInitialPieces;
    pieces[14] = gInitialPieces;
    generic_hash_destroy();
    gNumberOfPositions = generic_hash_init(
        gBoardSize, pieces, ValidPieceConfiguration, 0);
    gInitialPosition = StartingPosition();
}

POSITION ActualNumberOfPositions(int option) {
    int cells;
    int maxPieces;

    if (option < 1 || option > 4) return 0;
    cells = (option == 3 || option == 4) ? 15 : 25;
    maxPieces = option == 3 ? 3 : 5;
    return CompletionCountFor(cells, 0, 0, maxPieces) * 2;
}

/**
 * @brief Initialize any global variables.
 */
void InitializeGame(void) {
    gCanonicalPosition = GetCanonicalPosition;
    gActualNumberOfPositionsOptFunPtr = ActualNumberOfPositions;
    ConfigureVariant(gVariant);

    // If you want formal position strings to
    // be the same as the AutoGUI position strings,
    // then leave gPositionToStringFunPtr as NULL, i.e.,
    // delete this next line and also feel free
    // to delete the PositionToString function.
    gPositionToStringFunPtr = &PositionToString;
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
    MOVELIST *moves = NULL;
    MOVELIST *tail = NULL;
    MOVE *seen = NULL;
    size_t seenCount = 0;
    size_t seenCapacity = 0;
    int board[MAX_BOARD_SIZE];
    int player = UnhashPosition(position, board);
    int src;

    if (HasEnoughShrimps(board, 3 - player) ||
        CapturedEnough(board, 3 - player)) {
        return NULL;
    }

    for (src = 0; src < gBoardSize; ++src) {
        int destinations[MAX_BOARD_SIZE];
        int destinationCount;
        int destIndex;
        if (OwnerOf(board[src]) != player) continue;
        destinationCount = NormalDestinations(board, src, destinations);
        for (destIndex = 0; destIndex < destinationCount; ++destIndex) {
            LANDING landings[4];
            int landingCount = ExpandLanding(
                board, src, destinations[destIndex], player, landings);
            int landingIndex;

            for (landingIndex = 0; landingIndex < landingCount; ++landingIndex) {
                int group[MAX_BOARD_SIZE];
                int groupCount = BondGroup(
                    landings[landingIndex].board,
                    landings[landingIndex].finalSquare, group);
                int afterCapture[MAX_BOARD_SIZE];
                unsigned int mainPart = PackPart(
                    src, destinations[destIndex],
                    landings[landingIndex].current,
                    landings[landingIndex].promote);
                memcpy(afterCapture, landings[landingIndex].board,
                       (size_t)gBoardSize * sizeof(int));
                ResolveCaptures(afterCapture, landings[landingIndex].finalSquare,
                                player);
                AddUniqueMove(&moves, &tail, (MOVE)mainPart, &seen,
                              &seenCount, &seenCapacity);

                if (WinsImmediately(afterCapture, player) || groupCount < 2) {
                    continue;
                }
                {
                    int groupIndex;
                    for (groupIndex = 0; groupIndex < groupCount; ++groupIndex) {
                        int bondSrc = group[groupIndex];
                        int bondDestinations[MAX_BOARD_SIZE];
                        int bondDestCount = BondDestinations(
                            afterCapture, bondSrc, bondDestinations);
                        int bondDestIndex;
                        for (bondDestIndex = 0; bondDestIndex < bondDestCount;
                             ++bondDestIndex) {
                            LANDING bondLandings[4];
                            int bondLandingCount = ExpandLanding(
                                afterCapture, bondSrc,
                                bondDestinations[bondDestIndex], player,
                                bondLandings);
                            int bondLandingIndex;
                            for (bondLandingIndex = 0;
                                 bondLandingIndex < bondLandingCount;
                                 ++bondLandingIndex) {
                                unsigned int bondPart = PackPart(
                                    bondSrc, bondDestinations[bondDestIndex],
                                    bondLandings[bondLandingIndex].current,
                                    bondLandings[bondLandingIndex].promote);
                                MOVE move = (MOVE)PackMove(
                                    mainPart, bondPart, 1);
                                AddUniqueMove(&moves, &tail, move, &seen,
                                              &seenCount, &seenCapacity);
                            }
                        }
                    }
                }
            }
        }
    }
    if (seen != NULL) SafeFree(seen);
    return moves;
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
    int board[MAX_BOARD_SIZE];
    int player = UnhashPosition(position, board);
    unsigned int encodedMove = (unsigned int)move;
    unsigned int mainPart = encodedMove & PART_MASK;
    unsigned int bondPart = (encodedMove >> PART_BITS) & PART_MASK;
    int hasBond = (encodedMove & BOND_FLAG) != 0;

    ApplyPart(board, mainPart, player);
    if (hasBond && !WinsImmediately(board, player)) {
        ApplyPart(board, bondPart, player);
    }
    return HashBoard(board, 3 - player);
}

/**
 * @brief Return win, lose, or tie if the input position
 * is primitive; otherwise return undecided.
 * 
 * @param position : The position to inspect.
 * 
 * @return an enum; one of (win, lose, tie, undecided). See 
 * src/core/types.h for the value enum definition.
 */
VALUE Primitive(POSITION position) {
    int board[MAX_BOARD_SIZE];
    int currentPlayer = UnhashPosition(position, board);
    int previousPlayer = 3 - currentPlayer;

    if (HasEnoughShrimps(board, previousPlayer) ||
        CapturedEnough(board, previousPlayer) ||
        !HasAnyNormalMove(board, currentPlayer)) {
        return lose;
    }
    return undecided;
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

static int ParseMovePartText(const char *text, size_t length,
                             unsigned int *part) {
    int srcCol, srcRow, destCol, destRow;
    int src, dest, current = CURRENT_NONE, promote = 0;
    size_t offset = 4;

    if (length < 4) return 0;
    srcCol = text[0] - 'a';
    srcRow = text[1] - '0';
    destCol = text[2] - 'a';
    destRow = text[3] - '0';
    if (!OnBoard(gRows - srcRow, srcCol) ||
        !OnBoard(gRows - destRow, destCol)) {
        return 0;
    }
    src = IndexOf(gRows - srcRow, srcCol);
    dest = IndexOf(gRows - destRow, destCol);
    if (offset < length && (text[offset] == 'u' || text[offset] == 'd')) {
        current = text[offset] == 'u' ? CURRENT_UP : CURRENT_DOWN;
        ++offset;
    }
    if (offset < length && text[offset] == 'p') {
        promote = 1;
        ++offset;
    }
    if (offset != length) return 0;
    *part = PackPart(src, dest, current, promote);
    return 1;
}

static int ParseTextMove(const char *input, unsigned int *mainPart,
                         unsigned int *bondPart, int *hasBond) {
    const char *separator;
    size_t mainLength;
    size_t bondLength;
    if (input == NULL) return 0;
    separator = strchr(input, '-');
    if (separator == NULL) {
        *hasBond = 0;
        *bondPart = 0;
        return ParseMovePartText(input, strlen(input), mainPart);
    }
    if (strchr(separator + 1, '-') != NULL) return 0;
    mainLength = (size_t)(separator - input);
    bondLength = strlen(separator + 1);
    if (!ParseMovePartText(input, mainLength, mainPart) ||
        !ParseMovePartText(separator + 1, bondLength, bondPart)) {
        return 0;
    }
    *hasBond = 1;
    return 1;
}

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
    static const char *pieceChars[] = {"-", "C", "S", "c", "s"};
    int board[MAX_BOARD_SIZE];
    UnhashPosition(position, board);
    int row, col;

    printf("  ");
    for (col = 0; col < gCols; ++col) printf("%c ", 'a' + col);
    printf("\n");
    for (row = 0; row < gRows; ++row) {
        printf("%d ", gRows - row);
        for (col = 0; col < gCols; ++col) {
            int piece = board[IndexOf(row, col)];
            if (piece == EMPTY && col == gCurrentCol) {
                printf("~ ");
            } else {
                printf("%s ", pieceChars[piece]);
            }
        }
        printf("\n");
    }
    printf("%s\n", GetPrediction(position, playerName, usersTurn));
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
        /* List of available moves */
        // Modify the player's move prompt as you wish
        printf("%8s's move: ", playerName);
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
    unsigned int mainPart;
    unsigned int bondPart;
    int hasBond;
    return input != NULL &&
        ParseTextMove(input, &mainPart, &bondPart, &hasBond);
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
    unsigned int mainPart = 0;
    unsigned int bondPart = 0;
    int hasBond = 0;
    if (!ParseTextMove(input, &mainPart, &bondPart, &hasBond)) return 0;
    return (MOVE)PackMove(mainPart, bondPart, hasBond);
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
 */
void MoveToString(MOVE move, char *moveStringBuffer) {
    unsigned int encodedMove = (unsigned int)move;
    unsigned int mainPart = encodedMove & PART_MASK;
    unsigned int bondPart = (encodedMove >> PART_BITS) & PART_MASK;
    int src, dest, current, promote;

    UnpackPart(mainPart, &src, &dest, &current, &promote);
    snprintf(moveStringBuffer, MAX_MOVE_STRING_LENGTH, "%c%d%c%d%s%s",
             'a' + src % gCols, gRows - src / gCols,
             'a' + dest % gCols, gRows - dest / gCols,
             current == CURRENT_UP ? "u" :
                 (current == CURRENT_DOWN ? "d" : ""),
             promote ? "p" : "");
    if ((encodedMove & BOND_FLAG) != 0) {
        size_t used = strlen(moveStringBuffer);
        UnpackPart(bondPart, &src, &dest, &current, &promote);
        snprintf(moveStringBuffer + used, MAX_MOVE_STRING_LENGTH - used,
                 "-%c%d%c%d%s%s",
                 'a' + src % gCols, gRows - src / gCols,
                 'a' + dest % gCols, gRows - dest / gCols,
                 current == CURRENT_UP ? "u" :
                     (current == CURRENT_DOWN ? "d" : ""),
                 promote ? "p" : "");
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
    return 4;
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
 */
void setOption(int option) {
    if (option < 1 || option > NumberOfOptions()) {
        fprintf(stderr, "Invalid Kani Nari Ebi variant: %d\n", option);
        return;
    }
    ConfigureVariant(option - 1);
}

/**
 * @brief Interactive menu used to change the variant, i.e., change
 * game-specific parameters, such as the side-length of a tic-tac-toe
 * board, for example. Does nothing if kGameSpecificMenu == FALSE.
 */
void GameSpecificMenu(void) {
    int option;
    int ch;
    printf("\nKani Nari Ebi variants:\n");
    printf("1: regular (5x5, same-type bonding)\n");
    printf("2: mixed (5x5, same-player bonding)\n");
    printf("3: 3x5\n");
    printf("4: 5x3\n");
    printf("Select variant: ");
    if (scanf("%d", &option) != 1) {
        while ((ch = getchar()) != '\n' && ch != EOF) {}
        printf("Invalid variant selection.\n");
        return;
    }
    while ((ch = getchar()) != '\n' && ch != EOF) {}
    if (option < 1 || option > NumberOfOptions()) {
        printf("Variant must be between 1 and 4.\n");
        return;
    }
    setOption(option);
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
 */
void PositionToString(POSITION position, char *positionStringBuffer) {
    static const char pieceChars[] = "-CScs";
    int board[MAX_BOARD_SIZE];
    int turn = UnhashPosition(position, board);
    int i;
    positionStringBuffer[0] = (char)('0' + turn);
    positionStringBuffer[1] = '_';
    for (i = 0; i < gBoardSize; ++i) {
        positionStringBuffer[i + 2] = pieceChars[board[i]];
    }
    positionStringBuffer[gBoardSize + 2] = '\0';
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
 */
POSITION StringToPosition(char *positionString) {
	int turn;
	char *board;
	if (ParseStandardOnelinePositionString(positionString, &turn, &board)) {
        int decoded[MAX_BOARD_SIZE];
        int i;
        if (strlen(board) != (size_t)gBoardSize) return NULL_POSITION;
        for (i = 0; i < gBoardSize; ++i) {
            switch (board[i]) {
                case '-': decoded[i] = EMPTY; break;
                case 'C': decoded[i] = P1_CRAB; break;
                case 'S': decoded[i] = P1_SHRIMP; break;
                case 'c': decoded[i] = P2_CRAB; break;
                case 's': decoded[i] = P2_SHRIMP; break;
                default: return NULL_POSITION;
            }
        }
        return HashBoard(decoded, turn);
	}
	return NULL_POSITION;
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
 */
void PositionToAutoGUIString(POSITION position, char *autoguiPositionStringBuffer) {
    static const char pieceChars[] = "-CScs";
    int board[MAX_BOARD_SIZE];
    char boardString[MAX_BOARD_SIZE + 1];
    int turn = UnhashPosition(position, board);
    int i;
    for (i = 0; i < gBoardSize; ++i) {
        boardString[i] = pieceChars[board[i]];
    }
    boardString[gBoardSize] = '\0';
    AutoGUIMakePositionString(turn, boardString, autoguiPositionStringBuffer);
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
 */
void MoveToAutoGUIString(POSITION position, MOVE move, char *autoguiMoveStringBuffer) {
    unsigned int part = (unsigned int)move & PART_MASK;
    int src, dest, current, promote;
    (void)position;
    UnpackPart(part, &src, &dest, &current, &promote);
    (void)current;
    (void)promote;
    AutoGUIMakeMoveButtonStringM((unsigned int)src, (unsigned int)dest,
                                'x', autoguiMoveStringBuffer);
}