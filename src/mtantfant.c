
/************************************************************************
**
** NAME:        mtantfant.c
**
** DESCRIPTION: Tant Fant
** https://ludii.games/details.php?keyword=Tant%20Fant
**
** AUTHOR:      Josh Zhang, Daniel Liu, Johan Ko
**
** DATE:        2024-10-20
**
************************************************************************/

#include "gamesman.h"

#define BOARDSIZE 9
#define BOARDROWS 3
#define BOARDCOLS 3

// Moves are defined as a 2 digit number, specifying the start and end.
#define ENCODE_MOVE(start, end) ((start) * 10 + end)
#define DECODE_MOVE_START(move) (move / 10)
#define DECODE_MOVE_END(move) (move % 10)
// Based on mponghauki, player is either 1 or 2.
#define NEXT_PLAYER(player) (1 + (player % 2))

BOOLEAN ThreeInARow(char *, int, int, int, char);

char pieces[] = " XO";

CONST_STRING kAuthorName = "Josh Zhang, Daniel Liu, Johan Ko";
CONST_STRING kGameName = "Tant Fant"; // Use this spacing and case
CONST_STRING kDBName = "tantfant";    // Use this spacing and case

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
BOOLEAN kPartizan = FALSE;

/**
 * @brief Whether a tie or draw is possible in this game.
 */
BOOLEAN kTieIsPossible = FALSE;

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
void PositionToString(POSITION, char *);

/**
 * @brief If multiple variants are supported, set this
 * to true -- GameSpecificMenu() must be implemented.
 */
BOOLEAN kGameSpecificMenu = FALSE;

/**
 * @brief Set this to true if the DebugMenu() is implemented.
 */
BOOLEAN kDebugMenu = FALSE;

/**
 * @brief These variables are not needed for solving but if you have time
 * after you're done solving the game you should initialize them
 * with something helpful, for the TextUI.
 */
CONST_STRING kHelpGraphicInterface = "GUI has not been implemented yet.";
CONST_STRING kHelpTextInterface =
    "On your turn, use the LEGEND to find a piece of yours to move and an empty space you want to move to.\
    Each of these is specified by a digit between 1 and 9.\
    After deciding your move, enter a TWO digit number denoting your start and end spaces, then hit ENTER.\
    If you make a mistake, type 'U' to revert to a previous game board.";

CONST_STRING kHelpOnYourTurn = "Move one of your pieces to an allowed empty space, denoted by board lines.";
CONST_STRING kHelpStandardObjective = "Get your 3 pieces in a row horizontally, vertically, or diagonally.\
The 3-in-a-row position from which a game starts cannot count for wins.";
CONST_STRING kHelpReverseObjective = "Reverse objective has not been implemented yet.";
CONST_STRING kHelpTieOccursWhen = /* Should follow 'A Tie occurs when... */ "";

CONST_STRING kHelpExample = "          1-2-3          :     O-O-O\n\
          |\\|/|                |\\|/|\n\
LEGEND:   4-5-6     BOARD:      - -      (Player should Tie in 15)  \n\
          |/|\\|                |/|\\|\n\
          7-8-9          :     X-X-X\n\
It is Player's turn (X).\n\
  Player's move: 85\n\
          1-2-3          :     O-O-O\n\
          |\\|/|                |\\|/|\n\
LEGEND:   4-5-6     BOARD:      -X-      (Data should Tie in 15)  \n\
          |/|\\|                |/|\\|\n\
          7-8-9          :     X- -X\n\
It is Data's turn (O).\n\
Data's move: 14\n\
          1-2-3          :      -O-O\n\
          |\\|/|                |\\|/|\n\
LEGEND:   4-5-6     BOARD:     O-X-      (Player should Tie in 15)  \n\
          |/|\\|                |/|\\|\n\
          7-8-9          :     X- -X\n\
It is Player's turn (X).\n\
  Player's move: 96\n\
          1-2-3          :      -O-O\n\
          |\\|/|                |\\|/|\n\
LEGEND:   4-5-6     BOARD:     O-X-X     (Data should Tie in 15)  \n\
          |/|\\|                |/|\\|\n\
          7-8-9          :     X- - \n\
It is Data's turn (O).\n\
Data's move: 21\n\
          1-2-3          :     O- -O\n\
          |\\|/|                |\\|/|\n\
LEGEND:   4-5-6     BOARD:     O-X-X     (Player should Tie in 15)  \n\
          |/|\\|                |/|\\|\n\
          7-8-9          :     X- - \n\
It is Player's turn (X).\n\
  Player's move: 52\n\
          1-2-3          :     O-X-O\n\
          |\\|/|                |\\|/|\n\
LEGEND:   4-5-6     BOARD:     O- -X     (Data should Tie in 15)  \n\
          |/|\\|                |/|\\|\n\
          7-8-9          :     X- - \n\
It is Data's turn (O).\n\
Data's move: 15\n\
          1-2-3          :      -X-O\n\
          |\\|/|                |\\|/|\n\
LEGEND:   4-5-6     BOARD:     O-O-X     (Player should Tie in 15)  \n\
          |/|\\|                |/|\\|\n\
          7-8-9          :     X- - \n\
It is Player's turn (X).\n\
  Player's move: 78\n\
          1-2-3          :      -X-O\n\
          |\\|/|                |\\|/|\n\
LEGEND:   4-5-6     BOARD:     O-O-X     (Data will Win in 1)  \n\
          |/|\\|                |/|\\|\n\
          7-8-9          :      -X- \n\
It is Data's turn (O).\n\
Data's move: 47\n\
          1-2-3          :      -X-O\n\
          |\\|/|                |\\|/|\n\
LEGEND:   4-5-6     BOARD:      -O-X     (Player will Lose in 0)  \n\
          |/|\\|                |/|\\|\n\
          7-8-9          :     O-X- \n\
It is Player's turn (X).\n\
Data (player two) Wins!";

/**
 * @brief Tcl-related stuff. Do not change if you do not plan to make a Tcl interface.
 */
void *gGameSpecificTclInit = NULL;
void SetTclCGameSpecificOptions(int theOptions[]) { (void)theOptions; }

/*********** BEGIN SOLVING FUNCIONS ***********/

/**
 * @brief Initialize any global variables.
 */
void InitializeGame(void)
{
    gCanonicalPosition = GetCanonicalPosition;

    // We always have 3 blanks, 3 X, 3 O pieces.
    int hash_data[] = {' ', 3, 3, 'X', 3, 3, 'O', 3, 3, -1};
    gNumberOfPositions = generic_hash_init(BOARDSIZE, hash_data, NULL, 0);
    char start[] = "OOO   XXX";
    gInitialPosition = generic_hash_hash(start, 1);

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
MOVELIST *GenerateMoves(POSITION position)
{
    /*
        NOTE: To add to the linked list, do

        moves = CreateMovelistNode(<the move you're adding>, moves);

        See the function CreateMovelistNode in src/core/misc.c
    */
    char board[BOARDSIZE];
    generic_hash_unhash(position, board);
    int player = generic_hash_turn(position);
    MOVELIST *moves = NULL;

    // Deltas
    int dr[] = {-1, 0, 0, 1};
    int dc[] = {0, 1, -1, 0};

    for (int cpos = 0; cpos < BOARDSIZE; cpos++)
    {
        if (cpos == 4)
            continue;
        // Check for the locations of all pieces on the board
        if (pieces[player] == board[cpos])
        {
            // Check adjacent squares for any empty spaces
            int cr = cpos / 3;
            int cc = cpos % 3;
            for (int j = 0; j < 4; j++)
            {
                int nr = cr + dr[j];
                int nc = cc + dc[j];
                if (nr < 0 || nc < 0 || nr >= BOARDROWS || nc >= BOARDCOLS)
                    continue;
                int npos = nr * BOARDCOLS + nc;
                if (board[npos] != ' ')
                    continue;
                moves = CreateMovelistNode(ENCODE_MOVE(cpos, npos), moves);
            }
            // Don't double count edge moves
            if (board[4] == ' ' && cpos % 2 == 0 && cpos != 4)
                moves = CreateMovelistNode(ENCODE_MOVE(cpos, 4), moves);
        }
    }

    if (board[4] == pieces[player])
    {
        for (int npos = 0; npos < BOARDSIZE; npos++)
        {
            if (board[npos] == ' ')
                moves = CreateMovelistNode(ENCODE_MOVE(4, npos), moves);
        }
    }

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
POSITION DoMove(POSITION position, MOVE move)
{
    // Decode board
    char board[BOARDSIZE];
    generic_hash_unhash(position, board);

    // Get start and end position
    int spos = DECODE_MOVE_START(move);
    int npos = DECODE_MOVE_END(move);
    int player = generic_hash_turn(position);

    assert(board[spos] == pieces[player]);
    assert(board[npos] == ' ');

    // Perform the move
    board[spos] = ' ';
    board[npos] = pieces[player];

    // Create the new position
    POSITION out = generic_hash_hash(board, NEXT_PLAYER(player));
    return out;
}

BOOLEAN ThreeInARow(char *board, int a, int b, int c, char piece)
{
    return (board[a] == piece && board[b] == piece && board[c] == piece);
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
VALUE Primitive(POSITION position)
{
    // Always a loss if there exists a 3 in a row.
    char board[BOARDSIZE];
    generic_hash_unhash(position, board);
    int player = generic_hash_turn(position);

    char piece = pieces[NEXT_PLAYER(player)];
    if (ThreeInARow(board, 3, 4, 5, piece) ||
        // Verticals
        ThreeInARow(board, 0, 3, 6, piece) ||
        ThreeInARow(board, 1, 4, 7, piece) ||
        ThreeInARow(board, 2, 5, 8, piece) ||
        // Diagonals
        ThreeInARow(board, 0, 4, 8, piece) ||
        ThreeInARow(board, 2, 4, 6, piece))
    {
        return lose;
    }
    if (player == 2 && ThreeInARow(board, 0, 1, 2, piece))
        return lose;
    if (player == 1 && ThreeInARow(board, 6, 7, 8, piece))
        return lose;
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
POSITION GetCanonicalPosition(POSITION position)
{
    // Just not going to deal with symmetries because of the rule that the 3 in a row can't be the starting position :/
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
void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn)
{
    char board[BOARDSIZE];
    generic_hash_unhash(position, board);
    int player = generic_hash_turn(position);

    printf("\n          1-2-3          :     ");
    printf("%c-", board[0]);
    printf("%c-", board[1]);
    printf("%c", board[2]);
    printf("\n          |\\|/|                |\\|/|");
    printf("\nLEGEND:   4-5-6     BOARD:     ");
    printf("%c-", board[3]);
    printf("%c-", board[4]);
    printf("%c", board[5]);

    printf("     %s", GetPrediction(position, playerName, usersTurn));
    printf("\n          |/|\\|                |/|\\|");
    printf("\n          7-8-9          :     ");
    printf("%c-", board[6]);
    printf("%c-", board[7]);
    printf("%c", board[8]);
    printf("\n\nIt is %s's turn (%c).\n", playerName, pieces[player]);
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
            return ret;
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
BOOLEAN ValidTextInput(STRING input)
{
    // Number between 11 and 99
    if (strlen(input) != 2)
        return FALSE;
    if (!isdigit(input[0]) || input[0] - '0' < 1 || input[0] - '0' > 9)
        return FALSE;
    if (!isdigit(input[1]) || input[1] - '0' < 1 || input[1] - '0' > 9)
        return FALSE;
    return TRUE;
}

/**
 * @brief Convert the string input to the internal move representation.
 *
 * @param input This is a user-inputted move string already validated
 * by ValidTextInput().
 *
 * @return The hash of the move specified by the text input.
 */
MOVE ConvertTextInputToMove(STRING input)
{
    return ENCODE_MOVE(input[0] - '1', input[1] - '1');
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
void MoveToString(MOVE move, char *moveStringBuffer)
{
    sprintf(moveStringBuffer, "%d%d", DECODE_MOVE_START(move) + 1, DECODE_MOVE_END(move) + 1);
}

/**
 * @brief Nicely format the computers move.
 *
 * @param computersMove : The computer's move.
 * @param computersName : The computer's name.
 */
void PrintComputersMove(MOVE computersMove, STRING computersName)
{
    char moveStringBuffer[32];
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
int NumberOfOptions(void)
{
    return 1;
}

/**
 * @return The current variant ID.
 */
int getOption(void)
{
    return 0;
}

/**
 * @brief Set any global variables or data structures according to the
 * variant specified by the input variant ID.
 *
 * @param option An ID specifying the variant that we want to change to.
 */
void setOption(int option)
{
}

/**
 * @brief Interactive menu used to change the variant, i.e., change
 * game-specific parameters, such as the side-length of a tic-tac-toe
 * board, for example. Does nothing if kGameSpecificMenu == FALSE.
 */
void GameSpecificMenu(void) {}

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
void PositionToString(POSITION position, char *positionStringBuffer)
{
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
POSITION StringToPosition(char *positionString)
{
    int turn;
    char *board;
    if (ParseStandardOnelinePositionString(positionString, &turn, &board))
    {
        return 0;
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
void PositionToAutoGUIString(POSITION position, char *autoguiPositionStringBuffer) {}

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
void MoveToAutoGUIString(POSITION position, MOVE move, char *autoguiMoveStringBuffer) {}
