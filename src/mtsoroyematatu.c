/************************************************************************
**
** NAME:        mtsoroyematatu.c
**
** DESCRIPTION: Tsoro Yematatu
**
** AUTHOR:      Ken Zheng
**              Justin Park
**              Michael Palmerlee
**              Jason Ding
**
** DATE:        2024-11-20
**
************************************************************************/

#include "gamesman.h"

CONST_STRING kAuthorName = "Ken Zheng, Justin Park, Michael Palmerlee, Jason Ding";
CONST_STRING kGameName = "Tsoro Yematatu"; 
CONST_STRING kDBName = "tsoroyematatu";

/**
 * @brief An upper bound on the number of reachable positions.
 *
 * @details The hash value of every reachable position must be less
 * than `gNumberOfPositions`.
 */

// Which one should it be?
POSITION gNumberOfPositions = 6561; // > 3^7
//POSITION gNumberOfPositions = 2187; // 3^7
//POSITION gNumberOfPositions = 713;
/*1 for 0 pieces, 7 for 1 piece, 7*6 = 42 for 2 pieces, 
7*6*5/2! = 105 for 3 pieces, 7*6*5*4/2!*2! = 210 for 4 pieces, 
7*6*5*4*3/3!*2! = 210 for 5 pieces, 7*6*5*4*3*2/3!*3! - 2 = 138 
for 6 pieces due to it not being possible to get w,w,w b,b,b 
or b,b,b w,w,w*
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
POSITION gInitialPosition = 2;
//checked

/**
 * @brief Indicates whether this game is PARTIZAN, i.e. whether, given
 * a board, each player has a different set of moves 
 * available to them on their turn. If the game is impartial, this is FALSE.
 */
BOOLEAN kPartizan = TRUE;
//checked

/**
 * @brief Whether a tie or draw is possible in this game.
 */
BOOLEAN kTieIsPossible = FALSE;
//checked

/**
 * @brief Whether the game is loopy. It is TRUE if there exists a position
 * P in the game such that, there is a sequence of N >= 1 moves one can
 * make starting from P that allows them to revisit P.
 */
BOOLEAN kLoopy = TRUE;
//checked

/**
 * @brief Whether symmetries are supported, i.e., whether there is
 * at least one position P in the game that gCanonicalPosition() 
 * maps to a position Q such that P != Q. If gCanonicalPosition
 * (initialized in InitializeGame() in this file), is set to NULL,
 * then this should be set to FALSE.
 */
BOOLEAN kSupportsSymmetries = TRUE;
//work on this

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
POSITION GetCanonicalPosition(POSITION position);

void PositionToString(POSITION, char*); /**{
    char positionString[11];
    positionString[0] = ' '; 
    positionString[2] = ' ';
    positionString[3] = '\n';
    positionString[7] = '\n';
    int piecePositions[] = {1,4,5,6,8,9,10};
    POSITION position;
    for(int i = 0; i < 7; i++) {
        if(position % 3 == 0) {
            positionString[piecePosition[i]] = ' ';
        }
        else if(position % 3 == 1) {
            positionString[piecePosition[i]] = 'W';
        }
        else {
            positionString[piecePosition[i]] = 'B';
        }
        position /= 3;
    }
}**/
//fix this to get right arguments

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
CONST_STRING kHelpGraphicInterface = "";
CONST_STRING kHelpTextInterface = "";
CONST_STRING kHelpOnYourTurn = "Please enter your move in the format 01 (move piece from position 0 to 1) or 4 (to drop a piece at position 4)";
CONST_STRING kHelpStandardObjective = "";
CONST_STRING kHelpReverseObjective = "";
CONST_STRING kHelpTieOccursWhen = /* Should follow 'A Tie occurs when... */ "";
CONST_STRING kHelpExample = "";

#define BOARDSIZE   7

typedef enum possibleBoardPieces {
	Blank, o, x
} BlankOX;

BlankOX PositionToBlankOX(POSITION, BlankOX*);
POSITION BlankOXToPosition(BlankOX*, BlankOX);
BOOLEAN ThreeInARow(BlankOX*, int, int, int);

char *gBlankOXString[] = { " ", "o", "x" };

BlankOX gBoard[BOARDSIZE]; //board of the game

int g3Array[] = {1, 3, 9, 27, 81, 243, 729};

// for printing purposes?
char game_board_blueprint[46] = {' ', ' ', ' ', ' ', '1', ' ', ' ', ' ',' ', 
                                ' ', ' ', ' ', '/', '|', '\\', ' ', ' ', ' ', 
                                ' ', ' ', '2', '-', '3', '-', '4', ' ', ' ',
                                ' ', '/', ' ', ' ', '|', ' ', ' ', '\\', ' ', 
                                '5', '-', '-', '-', '6', '-', '-', '-', '7', 
                                '\0'};

int X_placed = 0; // changes game from placing to moving when second player places third piece



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
    PositionToBlankOX(gInitialPosition, gBoard);
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
//moves are a base 10 number AB where A is the position from and B is the position to
int centerAdjacent[4] = {0, 1, 3, 5};
MOVELIST *GenerateMoves(POSITION position) {
    BlankOX current_player = PositionToBlankOX(position, gBoard);
    BlankOX other_player = current_player == o ? x : o;
    // if (xTurn) {
    //     BlankOX player = x;
    //     BlankOX opp_player = o;
    // }
    // else {
    //     BlankOX player = o;
    //     BlankOX opp_player = x;
    // }
    // xTurn = !xTurn;
    int blankIndex;
    MOVELIST *moves = NULL;
    int blankCount = 0;
    for (int i = 0; i < BOARDSIZE; i++)
        if(gBoard[i] == Blank) {
            blankIndex = i;
            blankCount++;
        }
    if (blankCount > 1) {
        for (int i = 0; i < BOARDSIZE; i++) {
            if (gBoard[i] == Blank) {
                moves = CreateMovelistNode(i, moves);
            }
        }
        return moves;
    }

    if (blankIndex == 0) { //there must be a player piece in 1, 2, or 3
        // for (int i = 1; i <= 2; i++) {
        //     if (gBoard[centerAdjacent[i]] == player) {
        //         moves = CreateMovelistNode(i * 10, moves);
        //     }
        //     if (gBoard[centerAdjacent[3] == player && gBoard[2] == opp_player]) {
        //         move = CreateMovelistNode(50, moves)
        //     }
        // }
        for (int i = 1; i <= 3; i++) {
            if (gBoard[i] == current_player) {
                moves = CreateMovelistNode(i * 10, moves);
            }
        }
        if (moves == NULL) {
            for (int i = 4; i <= 6; i++) {
                if (gBoard[i] == current_player && gBoard[i - 3] == other_player) {
                    moves = CreateMovelistNode(i * 10, moves);
                }
            }
        }
    }
    else if(abs(blankIndex - 2) == 1) { //1 or 3
        if (gBoard[0] == current_player) {
                moves = CreateMovelistNode(blankIndex, moves);
        }
        if (gBoard[2] == current_player) {
            moves = CreateMovelistNode(20 + blankIndex , moves);
        }
        if (gBoard[blankIndex + 3] == current_player) {
            moves = CreateMovelistNode((blankIndex + 3) * 10 + blankIndex , moves);
        }
        if (moves == NULL) {
            if (gBoard[(blankIndex + 2) % 4] == current_player && gBoard[2] == other_player) { //only one possibility for move if no adjacent tiles are the player's pieces
                moves = CreateMovelistNode(((blankIndex + 2) % 4) * 10 + blankIndex , moves);
            }
        }
    }
    // else if(blankIndex == 1) {
    //     if (gBoard[0] == player) {
    //             moves = CreateMovelistNode(blankIndex, moves);
    //     }
    //     if (gBoard[2] == player) {
    //         moves = CreateMovelistNode(20 + blankIndex , moves);
    //     }
    //     if (gBoard[4] == player) {
    //         moves = CreateMovelistNode(40 + blankIndex , moves);
    //     }
    //     if (gBoard[3] == player && gBoard[2] == opp_player) {
    //         moves = CreateMovelistNode(30 + blankIndex , moves);
    //     }
    //     if (moves == NULL) { //only one possibility for move if no adjacent tiles are the player's pieces
    //         moves = CreateMovelistNode(((blankIndex + 2) % 4) * 10 + blankIndex , moves);
    //     }
    // }
    else if (blankIndex == 2) { //center position
        for (int i = 0; i < 4; i++) {
            if (gBoard[centerAdjacent[i]] == current_player) {
                moves = CreateMovelistNode(centerAdjacent[i] * 10 + blankIndex, moves);
            }
        }
    }
    else if(abs(blankIndex - 5) == 1) { //4 or 6
        if (gBoard[blankIndex - 3] == current_player) {
            moves = CreateMovelistNode((blankIndex - 3) * 10 + blankIndex, moves);
        }
        if (gBoard[5] == current_player) {
            moves = CreateMovelistNode(50 + blankIndex, moves);
        }
        if (moves == NULL) {
            if (gBoard[0] == current_player && gBoard[blankIndex - 3] == other_player) {
                moves = CreateMovelistNode(blankIndex, moves);
            }
            if (gBoard[(blankIndex + 2) % 4 + 4] == current_player && gBoard[5] == other_player) {
                moves = CreateMovelistNode(((blankIndex + 2) % 4 + 4) * 10 + blankIndex, moves);
            }
        }
        // if (moves == NULL) {
        //     if (gBoard[0] == player) {
        //         moves = CreateMovelistNode(blankIndex, moves);
        //     }
        //     if (gBoard[(blankIndex + 2) % 4 + 4] == player) {
        //         moves = CreateMovelistNode(((blankIndex + 2) % 4 + 4) * 10 + blankIndex, moves);
        //     }
        // }
    }
    else { //5
        for (int i = 2; i <= 6; i += 2) {
            if (gBoard[i] == current_player) {
                moves = CreateMovelistNode(10 * i + blankIndex, moves);
            }
        }
        if (moves == NULL) {
            if (gBoard[0] == current_player && gBoard[2] == other_player) {
                moves = CreateMovelistNode(5, moves);
            }
        }
    }
    /* 
        NOTE: To add to the linked list, do
        
        moves = CreateMovelistNode(<the move you're adding>, moves);

        See the function CreateMovelistNode in src/core/misc.c
    */
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
    int blank_count = 0;
    BlankOX player = PositionToBlankOX(position, gBoard);
    for (int i = 0; i < BOARDSIZE; i++) {
        if (gBoard[i] == Blank) {
            blank_count++;
        }
    }
    if (blank_count == 1) {
        int from = move / 10;
        int to = move % 10;
        gBoard[to] = gBoard[from];
        gBoard[from] = Blank;
        // return BlankOXToPosition(gBoard);
    } else {
        gBoard[move % 10] = player;
    }
    BlankOX next_player = player == x ? o : x;
    return BlankOXToPosition(gBoard, next_player);
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
    BlankOX theBlankOX[BOARDSIZE];

	PositionToBlankOX(position,theBlankOX);

	if (ThreeInARow(theBlankOX, 0, 1, 4) ||
        ThreeInARow(theBlankOX, 0, 2, 5) ||
	    ThreeInARow(theBlankOX, 0, 3, 6) ||
	    ThreeInARow(theBlankOX, 1, 2, 3) ||
	    ThreeInARow(theBlankOX, 4, 5, 6))
		return gStandardGame ? lose : win;
	//else if (AllFilledIn(theBlankOX))
	//	return tie;

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
    /*POSITION newPosition, theCanonicalPosition;
	int i;

	theCanonicalPosition = position;

	for(i = 0; i < NUMSYMMETRIES; i++) {
		newPosition = DoSymmetry(position, i); /* get new *//*
		if(newPosition < theCanonicalPosition) /* THIS is the one *//*
			theCanonicalPosition = newPosition; /* set it to the ans *//*
	}

	return(theCanonicalPosition);*/
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
	BlankOX theBlankOx[BOARDSIZE];

	PositionToBlankOX(position,theBlankOx);

	printf("\n             1              :        %s         \n",
	       gBlankOXString[(int)theBlankOx[0]]);
    printf("            /|\\             :       /|\\        \n");
	printf("LEGEND:    2-3-4    BOARD:  :      %s-%s-%s       \n",
	       gBlankOXString[(int)theBlankOx[1]],
	       gBlankOXString[(int)theBlankOx[2]],
	       gBlankOXString[(int)theBlankOx[3]] );
    printf("          /  |  \\           :     /  |  \\      \n");
	printf("         5---6---7          :    %s---%s---%s   %s\n\n",
	       gBlankOXString[(int)theBlankOx[4]],
	       gBlankOXString[(int)theBlankOx[5]],
	       gBlankOXString[(int)theBlankOx[6]],
	       GetPrediction(position,playerName,usersTurn));
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
    if (strlen(input) == 1) {
        return input[0] >= '1' && input[0] <= '7';
    } else if (strlen(input) == 2) {
        return input[0] >= '1' && input[0] <= '7' && input[1] >= '1' && input[1] <= '7';
    } else {
        return FALSE;
    }
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
    if (strlen(input) == 1) {
        return ((MOVE) input[0] - '1');
    } else {
        return (MOVE) ((input[0] - '1') * 10 + input[1] - '1');
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
    int blank_count = 0;
    for (int i = 0; i < BOARDSIZE; i++) {
        if (gBoard[i] == Blank) {
            blank_count++;
        }
    }
    if (move < 10 && blank_count > 1) {
        sprintf(moveStringBuffer, "%d", move + 1);
    } else {
        sprintf(moveStringBuffer, "%d%d", move / 10 + 1, move % 10 + 1);
    }
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
	// int turn;
	// char *board;
	// if (ParseStandardOnelinePositionString(positionString, &turn, &board)) {
	// 	return 0;
	// }
	// return NULL_POSITION;
    int turn;
	char *board;
	if (ParseStandardOnelinePositionString(positionString, &turn, &board)) {
		BlankOX oxboard[BOARDSIZE];
		for (int i = 0; i < BOARDSIZE; i++) {
			if (board[i] == 'o') {
				oxboard[i] = o;
			} else if (board[i] == 'x') {
				oxboard[i] = x;
			} else if (board[i] == '-') {
				oxboard[i] = Blank;
			} else {
				return NULL_POSITION;
			}
		}
        BlankOX whose_turn = (turn == 1) ? x : o;
		return BlankOXToPosition(oxboard, whose_turn);
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
    BlankOX oxboard[BOARDSIZE];
	BlankOX whose_turn = PositionToBlankOX(position, oxboard);
	char board[BOARDSIZE + 1];
	for (int i = 0; i < BOARDSIZE; i++) {
		if (oxboard[i] == o) {
			board[i] = 'o';
		} else if (oxboard[i] == x) {
			board[i] = 'x';
		} else {
			board[i] = '-';
		}
	}
	board[BOARDSIZE] = '\0';
	AutoGUIMakePositionString(
		whose_turn == x ? 1 : 2, 
		board, 
		autoguiPositionStringBuffer
	);
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
    // BlankOX oxboard[BOARDSIZE];
	// BlankOX whose_turn = PositionToBlankOX(position, oxboard);
	// char token = (whose_turn == x) ? 'x' : 'o';
	// AutoGUIMakeMoveButtonStringA(token, move, '-', autoguiMoveStringBuffer);
    PositionToBlankOX(position, gBoard);
    (void) position;
    int blank_count = 0;
    for (int i = 0; i < BOARDSIZE; i++) {
        if (gBoard[i] == Blank) {
            blank_count++;
        }
    }
	if (move < 9 && blank_count > 1) { // Place Piece
		AutoGUIMakeMoveButtonStringA('-', move, 'x', autoguiMoveStringBuffer);
	} else { // Slide Piece
		AutoGUIMakeMoveButtonStringM(move / 10, move % 10, 'y', autoguiMoveStringBuffer);
	}
}



/************************************************************************
**
** NAME:        PositionToBlankOX
**
** DESCRIPTION: convert an internal position to that of a BlankOX.
**
** INPUTS:      POSITION thePos     : The position input.
**              BlankOX *theBlankOx : The converted BlankOX output array.
**
** CALLS:       BadElse()
**
************************************************************************/

BlankOX PositionToBlankOX(POSITION thePos, BlankOX *theBlankOX) {
    BlankOX turn = thePos % 3;
    thePos /= 3;
    for (int i = BOARDSIZE - 1; i >= 0; i--) {
        //theBlankOX[i] = thePos % 3;
        if (thePos % 3 == 0) {
            theBlankOX[i] = Blank;
        }
        else if (thePos % 3 == 1) {
            theBlankOX[i] = o;
        }
        else {
            theBlankOX[i] = x;
        }
        thePos /= 3;
    }
    return turn;  
}

/************************************************************************
**
** NAME:        BlankOXToPosition
**
** DESCRIPTION: convert a BlankOX to that of an internal position.
**
** INPUTS:      BlankOX *theBlankOx : The converted BlankOX output array.
**
** OUTPUTS:     POSITION: The equivalent position given the BlankOX.
**
************************************************************************/
//BLANK 0, O 1, X 2
POSITION BlankOXToPosition(BlankOX *theBlankOX, BlankOX turn) {
	POSITION position = 0;

	for (int i = 0; i < BOARDSIZE; i++) {
        if (theBlankOX[i] == o) {
            position += 1;
        }
        else if (theBlankOX[i] == x) {
            position += 2;
        }
        position *= 3;
    }
    position += (POSITION) turn;
    return position;
}


/************************************************************************
**
** NAME:        ThreeInARow
**
** DESCRIPTION: Return TRUE iff there are three-in-a-row.
**
** INPUTS:      BlankOX theBlankOX[BOARDSIZE] : The BlankOX array.
**              int a,b,c                     : The 3 positions to check.
**
** OUTPUTS:     (BOOLEAN) TRUE iff there are three-in-a-row.
**
************************************************************************/

BOOLEAN ThreeInARow(BlankOX *theBlankOX, int a, int b, int c) {
	return(theBlankOX[a] == theBlankOX[b] &&
	        theBlankOX[b] == theBlankOX[c] &&
	        theBlankOX[c] != Blank);
}

