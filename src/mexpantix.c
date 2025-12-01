/************************************************************************
**
** NAME:        mexpantix.c
**
** DESCRIPTION: ExpanTix
**
** AUTHOR:      Johan Ko
**
** DATE:        2025-10-05
**
************************************************************************/

#include "gamesman.h"
#include <stdint.h>

#define MAXPLACED 3
#define EMPTY '.'
#define PIECE '#'
#define SYMMETRYLOSE FALSE

#define NEXT_PLAYER(player) (1 + (player % 2))

CONST_STRING kAuthorName = "Johan Ko";
CONST_STRING kGameName = "ExpanTix";  // Use this spacing and case
CONST_STRING kDBName = "expantix";      // Use this spacing and case

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
CONST_STRING kHelpGraphicInterface = "";Click the center dot on a grid space to place one tile.\n\
Click a double arrow to place 2 pieces and the triple arrow to place 3 pieces in that direction.

CONST_STRING kHelpTextInterface = "Pick a location to start your move, a direction\n\
(u, d, l, r) to extend your move, and a number of tiles to place. Format is\n\
{x coord}{y coord}{direction}{length}. For example, '34r2'. If placing only one\n\
tile, only the x coordinate and y coordinate are needed, like '34'.";

CONST_STRING kHelpOnYourTurn = "You place one, two, or three tiles in a row on the board.\n\
Your move must touch the existing tiles on the board (connected by at least one edge).\n\
If you expand below or to the left of the board, the board will shift up or right provided there's enough space.";

CONST_STRING kHelpStandardObjective = "Place the last tile to complete the square.";

CONST_STRING kHelpReverseObjective = "Force your opponent to place the last tile to complete the square.";

CONST_STRING kHelpTieOccursWhen = /* Should follow 'A Tie occurs when... */ "";

CONST_STRING kHelpExample = "";

/**
 * @brief Tcl-related stuff. Do not change if you do not plan to make a Tcl interface.
 */
void *gGameSpecificTclInit = NULL;
void SetTclCGameSpecificOptions(int theOptions[]) { (void)theOptions; }


/* Helper Functions */

int BOARDSIZE = 25;
int ROWS = 5;
int COLS = 5;

// Shift the board down by 'shift' columns
void shift_down(char board[BOARDSIZE], int shift){
    for(int r = ROWS-1; r >= shift; r--)
        for(int c = 0; c < COLS; c++)
            board[r*COLS + c] = board[(r-shift)*COLS + c];
    for(int r = 0; r < shift; r++)
        for(int c = 0; c < COLS; c++)
            board[r*COLS + c] = EMPTY;
}

// Shift the board right by 'shift' columns
void shift_right(char board[BOARDSIZE], int shift){
    for(int r = 0; r < ROWS; r++)
        for(int c = COLS-1; c >= shift; c--)
            board[r*COLS + c] = board[r*COLS + c-shift];
    for(int r = 0; r < ROWS; r++)
        for(int c = 0; c < shift; c++)
            board[r*COLS + c] = EMPTY;
}

BOOLEAN is_adjacent_to_piece(char board[BOARDSIZE], int r, int c) {
    // Check above
    if (r - 1 >= 0 && r - 1 < ROWS && c >= 0 && c < COLS)
        if (board[(r - 1) * COLS + c] == PIECE) return TRUE;

    // Check below
    if (r + 1 >= 0 && r + 1 < ROWS && c >= 0 && c < COLS)
        if (board[(r + 1) * COLS + c] == PIECE) return TRUE;

    // Check left
    if (c - 1 >= 0 && c - 1 < COLS && r >= 0 && r < ROWS)
        if (board[r * COLS + (c - 1)] == PIECE) return TRUE;

    // Check right
    if (c + 1 >= 0 && c + 1 < COLS && r >= 0 && r < ROWS)
        if (board[r * COLS + (c + 1)] == PIECE) return TRUE;

    return FALSE;
}

BOOLEAN can_place_move(char board[BOARDSIZE], int start_row, int start_col,
                          int dir, int length,
                          int occupied_rows, int occupied_cols) {
    int dr = 0, dc = 0;
    if (dir == 0) dc = 1;
    else if (dir == 1) dr = 1;
    else if (dir == 2) dc = -1;
	else if (dir == 3) dr = -1;

    int end_row = start_row + dr * (length - 1);
    int end_col = start_col + dc * (length - 1);

    // Determine shift needed for moves that go off the top/left
    int shift_down_needed = 0;
    int shift_right_needed = 0;
	int smallest_row = (end_row < start_row) ? end_row : start_row;
	int smallest_col = (end_col < start_col) ? end_col : start_col;
    if (smallest_row < 0) shift_down_needed = -smallest_row;
    if (smallest_col < 0) shift_right_needed = -smallest_col;

    // Check if board can be shifted to fit move
    if (shift_down_needed + occupied_rows > ROWS) return FALSE;
    if (shift_right_needed + occupied_cols > COLS) return FALSE;
	
	// Check if move is out of bounds
	if (end_row >= ROWS) return FALSE;
	if (end_col >= COLS) return FALSE;

	BOOLEAN any_adjacent = FALSE;
    // Check for overlap with existing 'O's
    for (int i = 0; i < length; i++) {
        int r = start_row + dr * i;
        int c = start_col + dc * i;
		if (r >= ROWS || c >= COLS) return FALSE;
        if (r >= 0 && c >= 0 && board[r * COLS + c] == PIECE) return FALSE;
		if (is_adjacent_to_piece(board, r, c)) any_adjacent = TRUE;
    }

    return any_adjacent;
}

BOOLEAN board_empty(char board[BOARDSIZE]) {
	for (int i = 0; i < BOARDSIZE; i++) {
        if (board[i] != EMPTY)
            return FALSE;
    }
	return TRUE;
}

/*********** BEGIN SOLVING FUNCIONS ***********/

/**
 * @brief Initialize any global variables.
 */

void InitializeGame(void) {
    gCanonicalPosition = GetCanonicalPosition;
	gSupportsMex = TRUE;

	int hash_data[] = {EMPTY, 0, BOARDSIZE, PIECE, 0, BOARDSIZE, -1};
    gNumberOfPositions = generic_hash_init(BOARDSIZE, hash_data, NULL, 0);
    
	// Make this adaptable to any BOARDSIZE
	char start[BOARDSIZE];
	memset(start, EMPTY, BOARDSIZE);

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
MOVELIST *GenerateMoves(POSITION position) {
    MOVELIST *moves = NULL;
    /* 
        NOTE: To add to the linked list, do
        
        moves = CreateMovelistNode(<the move you're adding>, moves);

        See the function CreateMovelistNode in src/core/misc.c
    */

	char board[BOARDSIZE];
    generic_hash_unhash(position, board);

	if (board_empty(board)) {
		int r = 0;
		int c = 0;
		for (int len = 1; len <= MAXPLACED; len++) {
			if (len == 1) {
				int move = (r + 1) * 1000 + (c + 1) * 100 + 10 * 0 + 1; // one-indexed
				moves = CreateMovelistNode(move, moves);
			}
			else {
				for (int d = 0; d < 4; d++) {
					int move = (r + 1) * 1000 + (c + 1) * 100 + 10 * d + len; // one-indexed
					moves = CreateMovelistNode(move, moves);
				}
			}	
		}
		return moves;
	}

	// Calculate occupied rows and columns
    int max_row = 0;
    int max_col = 0;
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            if (board[r * COLS + c] == PIECE) {
                if (r + 1 > max_row) max_row = r + 1;
                if (c + 1 > max_col) max_col = c + 1;
            }
        }
    }

    for (int r = -1; r < ROWS; r++) {
        for (int c = -1; c < COLS; c++) {
            if (r >= 0 && c >= 0 && board[r * COLS + c] == PIECE) continue;

			for (int len = 1; len <= MAXPLACED; len++) {
				if (len == 1) {
					if (can_place_move(board, r, c, 0, 1, max_row, max_col)) {
						int move = (r + 1) * 1000 + (c + 1) * 100 + 10 * 0 + 1; // one-indexed
						moves = CreateMovelistNode(move, moves);
					}
				}
				else {
					for (int d = 0; d < 4; d++) {
						if (can_place_move(board, r, c, d, len, max_row, max_col)) {
							int move = (r + 1) * 1000 + (c + 1) * 100 + 10 * d + len; // one-indexed
							moves = CreateMovelistNode(move, moves);
						}
                	}
				}
            }
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
POSITION DoMove(POSITION position, MOVE move) {
	char board[BOARDSIZE];
    generic_hash_unhash(position, board);
	int player = generic_hash_turn(position);

	int start_row = move / 1000 - 1;
	int start_col = move / 100 % 10 - 1;
	int dir = move / 10 % 10;
	int len = move % 10;

	int dr = 0, dc = 0;
	if (dir == 0) dc = 1;
    else if (dir == 1) dr = 1;
    else if (dir == 2) dc = -1;
	else if (dir == 3) dr = -1;

	int end_row = start_row + dr * (len - 1);
    int end_col = start_col + dc * (len - 1);

    // Determine shift needed for moves that go off the top/left
    int shift_down_needed = 0;
    int shift_right_needed = 0;
	int smallest_row = (end_row < start_row) ? end_row : start_row;
	int smallest_col = (end_col < start_col) ? end_col : start_col;
    if (smallest_row < 0) shift_down_needed = -smallest_row;
    if (smallest_col < 0) shift_right_needed = -smallest_col;

	if (shift_down_needed > 0) shift_down(board, shift_down_needed);
	if (shift_right_needed > 0) shift_right(board, shift_right_needed);

	// Adjust indices to account for the shift
	start_row += shift_down_needed;
    start_col += shift_right_needed;

    // Place the move
    for (int i = 0; i < len; i++) {
        int r = start_row + dr * i;
        int c = start_col + dc * i;
        board[r * COLS + c] = PIECE;
    }

	POSITION out = generic_hash_hash(board, NEXT_PLAYER(player));
    return out;
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
	char board[BOARDSIZE];
    generic_hash_unhash(position, board);

	if (SYMMETRYLOSE && !board_empty(board)) {

		BOOLEAN middle_row_filled = TRUE;
		BOOLEAN middle_col_filled = TRUE;
		BOOLEAN center_filled = TRUE;
		int middle_row = ROWS / 2;
		int middle_col = COLS / 2;
		if (ROWS % 2 != 0) {
			for (int c = 0; c < COLS; c++) {
				if (board[middle_row * COLS + c] == EMPTY) {
					middle_row_filled = FALSE;
					break;
				}
			}
		}
		if (COLS % 2 != 0) {
			for (int r = 0; r < ROWS; r++) {
				if (board[r * COLS + middle_col] == EMPTY) {
					middle_col_filled = FALSE;
					break;
				}
			}
		}
		if ((ROWS % 2 != 0) && (COLS % 2 != 0)) {
			if (board[middle_row * COLS + middle_col] == EMPTY) {
				center_filled = FALSE;
			}
		}


		if (middle_row_filled) {
			BOOLEAN row_symmetric = TRUE;
			for (int r = 0; (r < middle_row) && row_symmetric; r++) {
				for (int c = 0; c < COLS; c++) {
					if (board[r * COLS + c] != board[(ROWS - 1 - r) * COLS + c]) {
						row_symmetric = FALSE;
						break;
					}
				}
			}
			if (row_symmetric) {
				return lose;
			}
		}
		if (middle_col_filled) {
			BOOLEAN col_symmetric = TRUE;
			for (int c = 0; (c < middle_col) && col_symmetric; c++) {
				for (int r = 0; r < ROWS; r++) {
					if (board[r * COLS + c] != board[r * COLS + (COLS - 1 - c)]) {
						col_symmetric = FALSE;
						break;
					}
				}
			}
			if (col_symmetric) {
				return lose;
			}
		}
		if (center_filled) {
			BOOLEAN rotational_symmetric = TRUE;
			for (int r = 0; (r < middle_row) && rotational_symmetric; r++) {
				for (int c = 0; c < COLS; c++) {
					if (board[r * COLS + c] != board[(ROWS - 1 - r) * COLS + (COLS - 1 - c)]) {
						rotational_symmetric = FALSE;
						break;
					}
				}
			}
			if ((ROWS % 2 != 0) && rotational_symmetric) {
				for (int c = 0; c < middle_col; c++) {
					if (board[middle_row * COLS + c] != board[(ROWS - 1 - middle_row) * COLS + (COLS - 1 - c)]) {
						rotational_symmetric = FALSE;
						break;
					}
				}
			}
			
			if (rotational_symmetric) {
				return lose;
			}
		}
	}

	for (int i = 0; i < BOARDSIZE; i++) {
        if (board[i] != PIECE)
            return undecided;
    }
	

    return lose;
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
	char board[BOARDSIZE];
    generic_hash_unhash(position, board);
	
	for (int r = ROWS - 1; r >= 0; r--) {
		putchar('0' + (r + 1));
		putchar(' ');
        for (int c = 0; c < COLS; c++) {
            putchar(board[r * COLS + c]);
        }
        putchar('\n'); // new row
    }

	putchar('0');
	for (int c = -1; c < COLS; c++) {
		putchar(' ');
	}
	printf("       %s", GetPrediction(position, playerName, usersTurn));
	putchar('\n');

	putchar(' ');
	for (int i = 0; i < COLS + 1; i++) {
		putchar('0' + i);
	}
	putchar('\n');

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
	if (strlen(input) == 4) {
		if (!isdigit(input[0]) || input[0] - '0' < 0 || input[0] - '0' > ROWS)
			return FALSE;
		if (!isdigit(input[1]) || input[1] - '0' < 0 || input[1] - '0' > COLS)
			return FALSE;
		if (!(input[2] == 'u' || input[2] == 'd' || input[2] == 'l' || input[2] == 'r'))
			return FALSE;
		if (!isdigit(input[3]) || input[3] - '0' < 1 || input[3] - '0' > MAXPLACED)
			return FALSE;
	}
	else if (strlen(input) == 2) {
		if (!isdigit(input[0]) || input[0] - '0' < 0 || input[0] - '0' > ROWS)
			return FALSE;
		if (!isdigit(input[1]) || input[1] - '0' < 0 || input[1] - '0' > COLS)
			return FALSE;
	} 
	else {
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
	if (strlen(input) == 4) {
		int r = input[1] - '0';
		int c = input[0] - '0';
		int dir;
		switch (input[2]) {
			case 'r': dir = 0; break;
			case 'u': dir = 1; break;
			case 'l': dir = 2; break;
			case 'd': dir = 3; break;
		}
		int len = input[3] - '0';
		return r * 1000 + c * 100 + 10 * dir + len;
	}
	else {
		int r = input[1] - '0';
		int c = input[0] - '0';
		int dir = 0;
		int len = 1;
		return r * 1000 + c * 100 + 10 * dir + len;
	}
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
	char directions[] = {'r', 'u', 'l', 'd'};
	if (move % 10 == 1) {
		sprintf(moveStringBuffer, "%d%d", move / 100 % 10, move / 1000);
	}
	else {
		sprintf(moveStringBuffer, "%d%d%c%d", move / 100 % 10, move / 1000, directions[move / 10 % 10], move % 10);
	}
}

/**
 * @brief Nicely format the computers move.
 * 
 * @param computersMove : The computer's move.
 * @param computersName : The computer's name.
 */
void PrintComputersMove(MOVE computersMove, STRING computersName) {
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
int NumberOfOptions(void) {
    return 3;
}

/**
 * @return The current variant ID.
 */
int getOption(void) {
    return ROWS;
}

/**
 * @brief Set any global variables or data structures according to the
 * variant specified by the input variant ID.
 * 
 * @param option An ID specifying the variant that we want to change to.
 */
void setOption(int option) {
	int dim = option;
	ROWS = dim;
	COLS = dim;
	BOARDSIZE = dim * dim;
	(void)option;
}

/**
 * @brief Interactive menu used to change the variant, i.e., change
 * game-specific parameters, such as the side-length of a tic-tac-toe
 * board, for example. Does nothing if kGameSpecificMenu == FALSE.
 */
void GameSpecificMenu(void) {
	char inp;
    while (TRUE) {
        printf("\n\n\n");
        printf(
            "        ----- Game-specific options for ExpanTix -----\n\n");
        printf("        Select an option:\n\n");
        printf("        3)      %s\n", (ROWS == 3) ? "(Currently Selected)" : "");
		printf("        4)      %s\n", (ROWS == 4) ? "(Currently Selected)" : "");
		printf("        5)      %s\n", (ROWS == 5) ? "(Currently Selected)" : "");
        printf("        b)      Back to previous menu\n\n");
        printf("\nSelect an option: ");
        inp = GetMyChar();
        if (inp == '3') {
            setOption(3);
        } else if (inp == '4') {
            setOption(4);
		} else if (inp == '5') {
            setOption(5);
        } else if (inp == 'b' || inp == 'B') {
            return;
        } else if (inp == 'q' || inp == 'Q') {
            exit(0);
        } else {
            printf("Invalid input.\n");
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
 */
void PositionToString(POSITION position, char *positionStringBuffer) {
	(void)position;
	(void)positionStringBuffer;
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
		char real_board[BOARDSIZE];
        for (int i=0; i<BOARDSIZE; i++) {
            if (board[i] == '-') {
                real_board[i] = EMPTY;
            } else {
                real_board[i] = PIECE;
            }
        }
        return generic_hash_hash(real_board, turn);
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
	char board[BOARDSIZE + 1];
    generic_hash_unhash(position, board);
    for (int i=0; i<BOARDSIZE; i++) {
        board[i] = (board[i] == EMPTY ? '-' : 'X');
    }
    board[BOARDSIZE] = '\0';
    AutoGUIMakePositionString(0, board, autoguiPositionStringBuffer);
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
	(void)position;

	int col = move / 100 % 10;
	int row = move / 1000;
	int direction = move / 10 % 10;
	int length = move % 10;

	int center_num;

	char labels[] = {'r', 'u', 'l', 'd', 'R', 'U', 'L', 'D'};

	if (col > 0 && row > 0) {
		center_num = (row - 1) * COLS + (col - 1);
	} else if (col > 0) {
		center_num = BOARDSIZE + col - 1;
	} else {
		center_num = BOARDSIZE + COLS + (ROWS - row);
	}

	if (length == 1) {
		AutoGUIMakeMoveButtonStringA('-', center_num, 'x', autoguiMoveStringBuffer);
	} else {
		AutoGUIMakeMoveButtonStringA(labels[(length - 2) * 4 + direction], (1 + 2 * direction + (length - 2)) * (ROWS + 1) * (COLS + 1) + center_num, 'x', autoguiMoveStringBuffer);
	}
}