/************************************************************************
**
** NAME:        mjan.c
**
** DESCRIPTION: Jan 4x4 (A Simple Game)
**
** AUTHOR:      Andy Tae Kyu Kim, Andrew Jacob DeMarinis, Benjamin Riley Zimmerman
**
** DATE:        WIP
**
************************************************************************/

#include "gamesman.h" 

CONST_STRING kAuthorName = "Andy Kim Aj DeMarinis Benjamin Zimmerman";
CONST_STRING kGameName = "Jan4x4";
CONST_STRING kDBName = "jan4x4";

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
BOOLEAN kSupportsSymmetries = TRUE;

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
CONST_STRING kHelpTextInterface = "";
CONST_STRING kHelpOnYourTurn = "";
CONST_STRING kHelpStandardObjective = "";
CONST_STRING kHelpReverseObjective = "";
CONST_STRING kHelpTieOccursWhen = /* Should follow 'A Tie occurs when... */ "";
CONST_STRING kHelpExample = "";

/**
 * @brief Tcl-related stuff. Do not change if you do not plan to make a Tcl interface.
 */
void *gGameSpecificTclInit = NULL;
void SetTclCGameSpecificOptions(int theOptions[]) { (void)theOptions; }




/*********** BEGIN SOLVING FUNCIONS ***********/

/**
 * @brief Initialize any global variables.
 */
void InitializeGame(void) {
    gCanonicalPosition = GetCanonicalPosition;
    gNumberOfPositions = generic_hash_init(16, ['w', 4, 4, 'b', 4, 4, '-', 8, -1], null, 0);
    gInitialPosition = generic_hash_hash("bwbw--------wbwb", 1);

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
    /* 
        NOTE: To add to the linked list, do
        
        moves = CreateMovelistNode(<the move you're adding>, moves);

        See the function CreateMovelistNode in src/core/misc.c
    */
    char* board = (char*) SafeMalloc(16 * sizeof(char));
    char* board = generic_hash_unhash(position, board);

    int player = generic_hash_turn(position);
    char piece = (player == 1) ? 'w' : 'b';
    
    // Down: -4, Up: 4, Left: -1, Right: 1
    int directions[4] = {-4, 4, -1, 1};

    for (int i = 0; i < 16; i++) {
        if (board[i] == piece) {

            for (int d = 0; d < 4; d++) {
                int new_pos = i + directions[d]

                if (new_pos >= 0 && new_pos < 16 && board[new_pos] == '-') {
                    if (directions[d] == -1 && (i % 4 == 0)) continue;
                    if (directions[d] == 1 && (i % 4 == 3)) continue;

                    int encoded_move = (i << 4) | (new_pos & 0x0F);

                    moves = CreateMovelistNode(encoded_move, moves);
                }
            }
        }
    }
    SafeFree(board)

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
    char* board = (char*) SafeMalloc(16 * sizeof(char));
    char* board = generic_hash_unhash(position, board);
    int curr_player = generic_hash_turn(position);

    int start_pos = ((move >> 4) & 0x0F);
    int end_pos = (move & 0x0F);

    char piece = board[start_pos];
    board[end_pos] = piece;
    board[start_pos] = '-';

    POSITION child_position = generic_hash_hash(board, (curr_player + 1) % 2)
    SafeFree(board)

    return child_position;
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
    char* p_str = (char*)SafeMalloc(possize);
    p_str = generic_hash_unhash(position, p_str);
    int player = generic_hash_turn(position);
    int deciding_idx = -1;
    for (int idx = 0; idx < 8; idx ++) {
        if (p_str[idx] == '-') {
            continue;
        }
        if(p_str[idx] == p_str[idx + 4] && p_str[idx + 4] == p_str[idx + 8]) { // explore down
            deciding_idx = idx;
            break;
        }
        if (idx % 4 > 1) { // explore left
            if(p_str[idx] == p_str[idx - 1] && p_str[idx - 1] == p_str[idx - 2]) { // left
                deciding_idx = idx;
                break;
            }
            if(p_str[idx] == p_str[idx + 3] && p_str[idx + 3] == p_str[idx + 6]) { // left diagonal
                deciding_idx = idx;
                break;
            }
        }
        if (idx % 4 < 2) { // explore right 
            if(p_str[idx] == p_str[idx + 1] && p_str[idx + 1] == p_str[idx + 2]) { // right
                deciding_idx = idx;
                break;
            }
            if(p_str[idx] == p_str[idx + 5] && p_str[idx + 10] == p_str[idx + 6]) { // right diagonal
                deciding_idx = idx;
                break;
            }
        }
    }
    if (deciding_idx != -1) {
        return (p_str[deciding_idx] == 'w' && player == 1) || (p_str[deciding_idx] == 'b' && player == 2) ? win : lose;
    }
    SafeFree(p_str);
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
    char* board = (char*)SafeMalloc(possize);
	generic_hash_unhash(position, board);
    int player = generic_hash_turn(position);
    char* prediction = GetPrediction(position, playerName, usersTurn);
    if (usersTurn) {
        printf("%s's turn: True\n\n", playerName);
    } else {
        printf("%s's turn: False\n\n", playerName);
    }
    for (int idx = 0; idx < 16; idx += 4) {
        printf("%c%c%c%c\n", board[idx], board[idx + 1], board[idx + 2], board[idx + 3]);
    }
    printf("\nPrediction: %s\n", prediction);
    SafeFree(board);
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
    int start_pos, end_pos;
    char extra;

    if (sscanf(input, "%d %d %c", &start_pos, &end_pos, &extra) != 2) {
        return FALSE;  
    }

    if (start_pos < 0 || start_pos > 15 || end_pos < 0 || end_pos > 15) {
        return FALSE;
    }

    if (start_pos == end_pos) {
        return FALSE;
    }

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
MOVE ConvertTextInputToMove(STRING input) {
    if (ValidTextInput(input) == FALSE) {
        return 0;
    }
    int start_pos, end_pos;
    
    sscanf(input, "%d %d", &start_pos, &end_pos) != 2)
    
    MOVE encoded_move = ((start_pos & 0x0F) << 4) | (end_pos & 0x0F);
    
    return encoded_move;
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
    int start_pos = (move >> 4) & 0x0F; 
    int end_pos = move & 0x0F;          
    
    snprintf(moveStringBuffer, MAX_MOVE_STRING_LENGTH, "%d %d", start_pos, end_pos);

    moveStringBuffer[MAX_MOVE_STRING_LENGTH - 1] = '\0';
}

/**
 * @brief Nicely format the computers move.
 * 
 * @param computersMove : The computer's move.
 * @param computersName : The computer's name.
 */
void PrintComputersMove(MOVE computersMove, STRING computersName) {
    char *moveStringBuffer[32];
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
    return 1;
}

/**
 * @return The current variant ID.
 */
int getOption(void) {
    return 0;
}

/**
 * @brief Set any global variables or data structures according to the
 * variant specified by the input variant ID.
 * 
 * @param option An ID specifying the variant that we want to change to.
 */
void setOption(int option) {
}

/**
 * @brief Interactive menu used to change the variant, i.e., change
 * game-specific parameters, such as the side-length of a tic-tac-toe
 * board, for example. Does nothing if kGameSpecificMenu == FALSE.
 */
void GameSpecificMenu(void) {
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
void PositionToString(POSITION position, char *positionStringBuffer) {}

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