/************************************************************************
**
** NAME:        mabrobad.c
**
** DESCRIPTION: Abrobad
**
** AUTHOR:      Aditya Tummala***, Robert Shi*
**
** DATE:        2024-11-01
**

************************************************************************/

#include "gamesman.h"

#define INVALID_MOVE -10000
#define BOARDSIZE 12
#define ADJACENCY 6
#define MAX_NEIGHBOR_TYPES 8
#define MAX_NEIGHBORS 8
#define END_MOVE 0
#define PLACE_PIECE(pos) (pos)
#define MOVE_PIECE(start, end) (((start * 13) + end) + 12)
#define NEXT_PLAYER(player) (1 + (player % 2))
#define DEBUG 1

int adj[BOARDSIZE][ADJACENCY] = {   
	{1, 2, 3, -1, -1, -1},
	{0, 3, 4, -1, -1, -1},
	{0, 3, 5, 6, -1, -1},
	{0, 1, 2, 4, 6, 7},
	{1, 3, 7, 8, -1, -1},
	{2, 6, 9, -1, -1, -1},
	{2, 3, 5, 7, 9, 10},
	{3, 4, 6, 8, 10, 11},
	{4, 7, 11, -1, -1, -1},
	{5, 6, 10, -1, -1, -1},
	{6, 7, 9, 11, -1, -1},
	{7, 8, 10, -1, -1, -1}
};

int numAdj[BOARDSIZE] = {3, 3, 4, 6, 4, 3, 6, 6, 3, 3, 4, 3};

// For variant. You can jump to any position...
int allNeighbors[BOARDSIZE][MAX_NEIGHBORS] = {
	{1, 3, 7, 11, 2, 5, -1, -1},
	{4, 8, 3, 6, 9, 0, -1, -1},
	{0, 3, 4, 6, 10, 5, -1, -1},
	{1, 4, 7, 11, 6, 9, 2, 0},
	{8, 7, 10, 3, 2, 1, -1, -1},
	{2, 0, 6, 7, 8, 9, -1, -1},
	{3, 1, 7, 8, 10, 9, 5, 2},
	{4, 8, 11, 10, 6, 5, 3, 0},
	{11, 7, 6, 5, 4, 1, -1, -1},
	{6, 3, 1, 10, 11, 5, -1, -1},
	{7, 4, 11, 9, 6, 2, -1, -1},
	{8, 10, 9, 7, 3, 0, -1, -1}
};
int numAN[BOARDSIZE] = {6, 6, 6, 8, 6, 6, 8, 8, 6, 6, 6, 6};

int neighbors[BOARDSIZE][6][4] = {
	{ // 1
		{2, -1},
		{4, 8, 12, -1}, 
		{3, 6, -1},
		{-1},
		{-1},
		{-1}
	},
	{ // 2
		{5, 9, -1},
		{4, 7, 10, -1},
		{1, -1},
		{-1},
		{-1},
		{-1}
	},
	{ // 3
		{1, -1},
		{4, 5, -1},
		{7, 11, -1},
		{6},
		{-1},
		{-1}
	},
	{ // 4
		{2, -1},
		{5, -1},
		{8, 12, -1},
		{7, 10, -1},
		{3, -1},
		{1, -1}
	},
	{ // 5
		{9, -1},
		{8, 11, -1},
		{4, 3, -1},
		{2, -1},
		{-1},
		{-1}
	},
	{ // 6
		{3, 1, -1},
		{7, 8, 9, -1},
		{10, -1},
		{-1},
		{-1},
		{-1}
	},
	{ // 7
		{4, 2, -1},
		{8, 9, -1},
		{11, -1},
		{10, -1},
		{6, -1},
		{3, -1}
	},
	{ // 8
		{5, -1},
		{9, -1},
		{12, -1},
		{11, -1},
		{7, 6, -1},
		{4, 1, -1}
	},
	{ // 9
		{12, -1},
		{8, 7, 6, -1},
		{5, 2, -1},
		{-1},
		{-1},
		{-1}
	},
	{ //10
		{7, 4, 2, -1},
		{11, 12, -1},
		{6, -1}, 
		{-1},
		{-1},
		{-1}
	},
	{ //11
		{8, 5, -1},
		{12, -1},
		{10, -1},
		{7, 3, -1},
		{-1},
		{-1} 
	},
	{ //12
		{9, -1},
		{11, 10, -1},
		{8, 4, 1, -1},
		{-1},
		{-1},
		{-1}
	}
};


void decode_move(MOVE move, int* start, int* end);
POSITION DoMove(POSITION position, MOVE move);
void genHelper(char* board, int i, MOVELIST **moves);
BOOLEAN checkAdj(char* board, int i, int player);
void DFS(int node, char piece, BOOLEAN visited[], char board[BOARDSIZE]);

CONST_STRING kAuthorName = "Aditya Tummala";
CONST_STRING kGameName = "Abrobad";
CONST_STRING kDBName = "abrobad";

char pieces[] = "-XO";

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
BOOLEAN kDebugDetermineValue = TRUE;

/**
 * @brief Declaration of optional functions.
 */
POSITION GetCanonicalPosition(POSITION);
void PositionToString(POSITION, char*);

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
	int hash_data[] = {'-', 0, 12, 'X', 0, 12, 'O', 0, 12, -1};
	gNumberOfPositions = generic_hash_init(BOARDSIZE, hash_data, NULL, 0);
	char start[BOARDSIZE + 1] = "------------";
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
	char board[BOARDSIZE + 1];
	generic_hash_unhash(position, board);
	int player = generic_hash_turn(position);
	char piece = pieces[player];
	MOVELIST *moves = NULL;
	BOOLEAN canPlace = FALSE;

	for (int i = 0; i < BOARDSIZE; i++) {
		if (board[i] == '-' && checkAdj(board, i, player)) {
			canPlace = TRUE;
			MOVE place = PLACE_PIECE(i + 1);
			moves = CreateMovelistNode(place, moves);
		}
	}

	if (!canPlace) {
		int piecePositions[BOARDSIZE];
		int numPieces = 0;
		for (int i = 0; i < BOARDSIZE; i++) {
			if (board[i] == piece) { piecePositions[numPieces++] = i; }
		}
		for (int i = 0; i < numPieces; i++) {
			int pos = piecePositions[i];
			genHelper(board, pos, &moves);
		}
	}
	MOVELIST *prev = NULL;
	MOVELIST *current = moves;
	MOVELIST *next = NULL;
	
	while (current != NULL) {
		next = current->next;
		current->next = prev;
		prev = current;
		current = next;
	}
	if (!canPlace){ prev = CreateMovelistNode(END_MOVE, prev); }
	return prev;
}

void genHelper(char* board, int i, MOVELIST **moves) {
	for (int b = 0; b < 6; b++) {
		if (neighbors[i][b][0] == -1) { continue; }
		for (int c = 0; c < 4; c++) {
			int pos = neighbors[i][b][c];
			if (pos == -1) { break; }
			if (board[pos - 1] == '-') {
				MOVE move = MOVE_PIECE(i, pos) + 13;
				*moves = CreateMovelistNode(move, *moves);
				break;
			}
		}
	}
}



BOOLEAN checkAdj(char* board, int i, int player) {
	int* lst = adj[i];
	char piece = pieces[player];
	for (int j = 0; j < numAdj[i]; j++) { if (board[lst[j]] == piece) { return FALSE; } }
	return TRUE;
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

void decode_move(MOVE move, int* start, int* end) { int temp = move - 12; *start = temp / 13; *end = temp % 13; }


POSITION DoMove(POSITION position, MOVE move) {
	char board[BOARDSIZE + 1];
	generic_hash_unhash(position, board);
	board[BOARDSIZE] = '\0';
	int player = generic_hash_turn(position);
	char piece = pieces[player];

	if (move == END_MOVE) {
		int opponent = (player == 1) ? 2 : 1;
		char opponent_piece = pieces[opponent];
		for (int i = 0; i < BOARDSIZE; i++) {
			if (board[i] == '-') {board[i] = opponent_piece; } 
		}
	} else if (move <= 12) { board[move - 1] = piece;
	} else {
		int start, end;
		decode_move(move, &start, &end);
		board[end - 1] = board[start - 1];
		board[start - 1] = '-';
	}
	return generic_hash_hash(board, NEXT_PLAYER(player));
 
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


int CountPlayerGroups(int player, char *board) {
	int groupCount = 0; 
	BOOLEAN visited[BOARDSIZE] = {0};
	char piece = pieces[player];
	for (int i = 0; i < BOARDSIZE; i++) {
		if (board[i] == piece && !visited[i]) {
			DFS(i, piece, visited, board);
			groupCount++;
		}
	}
	return groupCount;
}

VALUE Primitive(POSITION position) {
	char board[BOARDSIZE + 1];
	generic_hash_unhash(position, board);
	
	if (strchr(board, '-') != NULL) return undecided;
			
	int player = generic_hash_turn(position);
	int op = 3 - player;
	
	return (CountPlayerGroups(player, board) >= CountPlayerGroups(op, board)) 
					? lose 
					: win;
}


void DFS(int node, char piece, BOOLEAN visited[], char board[BOARDSIZE]) {
	visited[node] = TRUE;
	for (int i = 0; i < numAdj[node]; i++) {
		int neighbor = adj[node][i];
		if (!visited[neighbor] && board[neighbor] == piece) { DFS(neighbor, piece, visited, board); }
	}
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
	char board[BOARDSIZE + 1];
	generic_hash_unhash(position, board);
	board[BOARDSIZE] = '\0';
	int player = generic_hash_turn(position);

	printf("%s\n", board);
	printf("Player: %d\n", player);

	printf("        Current Board               Legend");
	printf("\n         Turn: %s\n\n", playerName);

	printf("             %c %c          |          1  2\n", board[0], board[1]);
	printf("            %c %c %c         |         3  4  5\n", board[2], board[3], board[4]);
	printf("           %c %c %c %c        |        6  7  8  9\n", board[5], board[6], board[7], board[8]);
	printf("            %c %c %c         |        10  11  12\n", board[9], board[10], board[11]);
	printf("%s", GetPrediction(position, playerName, usersTurn));
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
	MOVELIST *movesList = GenerateMoves(position);
	MOVELIST *currMove;

	do {
		currMove = movesList;
		while (currMove != NULL) {
			char moveString[MAX_MOVE_STRING_LENGTH];
			MoveToString(currMove->move, moveString);
			currMove = currMove->next;
		}
		printf("\n");
		printf("%s's move: ", playerName);
		ret = HandleDefaultTextInput(position, move, playerName);

		if (ret != Continue) {
			FreeMoveList(movesList);
			return ret; 
		}

	} while (TRUE);

	FreeMoveList(movesList);
	return Continue;
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
	while (isspace(*input)) input++;

	if (strcmp(input, "0") == 0) { return TRUE; }

	int len = strlen(input);
	if (len == 0) return FALSE;

	char *endPtr;
	int firstNum = strtol(input, &endPtr, 10);
	if (endPtr == input) { return FALSE; }

	if (*endPtr == '\0') { return (firstNum >= 1 && firstNum <= BOARDSIZE); }

	char *dashPtr = strchr(input, '-');
	if (!dashPtr || dashPtr == input || *(dashPtr + 1) == '\0') { return FALSE; }

	char firstNumStr[16], secondNumStr[16];
	size_t firstNumLen = dashPtr - input;
	strncpy(firstNumStr, input, firstNumLen);
	firstNumStr[firstNumLen] = '\0';
	strcpy(secondNumStr, dashPtr + 1); 

	firstNum = atoi(firstNumStr);
	int secondNum = atoi(secondNumStr);

	return (firstNum >= 1 && firstNum <= BOARDSIZE && secondNum >= 1 && secondNum <= BOARDSIZE && secondNum != firstNum);

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
	if (strcmp(input, "0") == 0) { return END_MOVE; }

	char *dashPtr = strchr(input, '-');
	
	if (dashPtr && dashPtr != input && *(dashPtr + 1) != '\0') {
		char firstNumStr[16], secondNumStr[16];
		size_t firstNumLen = dashPtr - input;
		strncpy(firstNumStr, input, firstNumLen);
		firstNumStr[firstNumLen] = '\0';
		strcpy(secondNumStr, dashPtr + 1);
		
		int start = atoi(firstNumStr);
		int end = atoi(secondNumStr);
		
		if (start >= 1 && start <= BOARDSIZE && end >= 1 && end <= BOARDSIZE && start != end) {
			MOVE move = MOVE_PIECE(start, end);
			return move;
		}
		return INVALID_MOVE;
	} else {
		char *endPtr;
		int num = strtol(input, &endPtr, 10);

		if (*endPtr == '\0' && num >= 1 && num <= BOARDSIZE) { return PLACE_PIECE(num); }
		return INVALID_MOVE; 
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
	if (move == END_MOVE) { snprintf(moveStringBuffer, MAX_MOVE_STRING_LENGTH, "0");
	} else if (move <= 12) { snprintf(moveStringBuffer, MAX_MOVE_STRING_LENGTH, "%d", move);
	} else {
		int temp = move - 13;
		int start = (temp / 13);
		int end = (temp % 13) + 1;
		snprintf(moveStringBuffer, MAX_MOVE_STRING_LENGTH, "%d-%d", start, end);
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
	printf("\n%s's move: %s\n", computersName, moveStringBuffer); 
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
	(void)option; 
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
void PositionToString(POSITION position, char *positionStringBuffer) {
	char board[BOARDSIZE + 1];
	int player = generic_hash_turn(position);
	generic_hash_unhash(position, board);
	board[BOARDSIZE] = '\0';
	snprintf(positionStringBuffer, MAX_POSITION_STRING_LENGTH, "%d_%s", player, board);
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
	char *entityString;

	if (ParseStandardOnelinePositionString(positionString, &turn, &entityString)) {
		char board[BOARDSIZE + 1];
		strncpy(board, entityString, BOARDSIZE);
		board[BOARDSIZE] = '\0';

		POSITION position = generic_hash_hash(board, turn);
		return position;
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
	char board[BOARDSIZE + 2];
	int player = generic_hash_turn(position);
	generic_hash_unhash(position, board);
	board[BOARDSIZE] = '\0';
	
	BOOLEAN canPlace = FALSE;

	for (int i = 0; i < BOARDSIZE; i++) {
		if (board[i] == '-' && checkAdj(board, i, player)) { canPlace = TRUE; break; }
	}

	board[BOARDSIZE] = canPlace ? '0' : '1';
	board[BOARDSIZE + 1] = '\0';
	AutoGUIMakePositionString(player, board, autoguiPositionStringBuffer);
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
	if (move == END_MOVE) {AutoGUIMakeMoveButtonStringA('E', 12, 'x', autoguiMoveStringBuffer);
	} else if (move <= 12) {
		int tile = move;
		AutoGUIMakeMoveButtonStringA('t', tile - 1, 'x', autoguiMoveStringBuffer);
	} else {
		int start, end;
		decode_move(move, &start, &end);
		AutoGUIMakeMoveButtonStringM(start - 1, end - 1, 'y', autoguiMoveStringBuffer);
	}
}