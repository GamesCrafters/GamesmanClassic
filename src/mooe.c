
/************************************************************************
**
** NAME:        mooe.c
**
** DESCRIPTION: ODD OR EVEN
**
** AUTHOR:      PAI-HSIEN (PETER) YU
**
**************************************************************************/

#include "gamesman.h"

CONST_STRING kGameName            = "Odd or Even";   /* The name of your game */
CONST_STRING kAuthorName          = "Peter Yu";   /* Your name(s) */
CONST_STRING kDBName              = "oddoreven";   /* The name to store the database under */

void* gGameSpecificTclInit = NULL;

BOOLEAN kPartizan            = FALSE;   /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN kGameSpecificMenu    = FALSE;   /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN kTieIsPossible       = FALSE;   /* TRUE if a tie is possible. FALSE if it is impossible.*/
BOOLEAN kLoopy               = FALSE;   /* TRUE if the game tree will have cycles (a rearranger style game). FALSE if it does not.*/
BOOLEAN kDebugMenu           = FALSE;   /* TRUE only when debugging. FALSE when on release. */
BOOLEAN kDebugDetermineValue = FALSE;

POSITION gNumberOfPositions   =  338;
POSITION gInitialPosition     =  0;
POSITION kBadPosition         = -1; /* A position that will never be used */

CONST_STRING kHelpGraphicInterface =
        "Not written yet";

CONST_STRING kHelpTextInterface    =
        "On your turn, type in the number 1, 2, or 3 and hit return. If at any point\n\
you have made a mistake, you can type u and hit return and the system will\n\
revert back to your most recent position.";

CONST_STRING kHelpOnYourTurn =
        "You can enter 1, 2, or 3 to indicate how many match(es) you want take off board.\
A running total of how many matches you and your opponents have will be kept.\
Keep in mind though that the objective is not to get the last match; instead, you would have to have\
an even number of matches to win";

CONST_STRING kHelpStandardObjective =
        "At the end game, to have an even number of matches.";

CONST_STRING kHelpReverseObjective =
        "At the end game, to have an odd number of matches. (i.e. to force your opponent to take a total of even number\
of matches.";

CONST_STRING kHelpTieOccursWhen =
        "There cannot be a tie";

CONST_STRING kHelpExample =
        "";

void PositionToString(POSITION position, char *positionStringBuffer);

POSITION hash(int p1Matches, int p2Matches, int isP2Turn) {
	return (p2Matches * 26 + (p1Matches << 1)) | isP2Turn;
}

void unhash(POSITION position, int *p1Matches, int *p2Matches, int *isP2Turn) {
	*p2Matches = position / 26;
	*p1Matches = (position >> 1) % 13;
	*isP2Turn = position & 1;
}

void InitializeGame() { gPositionToStringFunPtr = &PositionToString; }

MOVELIST *GenerateMoves(POSITION position) {
	MOVELIST *moves = NULL;
	int p1Matches, p2Matches, isP2Turn;
	unhash(position, &p1Matches, &p2Matches, &isP2Turn);
	for (int i = 1; i <= 3 && i <= 15 - p1Matches - p2Matches; i++) {
		moves = CreateMovelistNode(i, moves);
	}
	return moves;
}

POSITION DoMove(POSITION position, MOVE move) {
	int p1Matches, p2Matches, isP2Turn;
	unhash(position, &p1Matches, &p2Matches, &isP2Turn);
	if (position & 1) { // Player 2's turn
		return hash(p1Matches, p2Matches + move, 0);
	} else { // Player 1's turn
		return hash(p1Matches + move, p2Matches, 1);
	}
}

VALUE Primitive(POSITION position) {
	int p1Matches, p2Matches, isP2Turn;
	unhash(position, &p1Matches, &p2Matches, &isP2Turn);
	if (p1Matches + p2Matches == 15) {
		if (isP2Turn) {
			return p1Matches & 1 ? win : lose;
		} else {
			return p2Matches & 1 ? win : lose;
		}
	} else {
		return undecided;
	}
}


/************************************************************************
**
** NAME:        PrintPosition
**
** DESCRIPTION: Prints the position in a pretty format, including the
**              prediction of the game's outcome.
**
** INPUTS:      POSITION position    : The position to pretty print.
**              STRING   playersName : The name of the player.
**              BOOLEAN  usersTurn   : TRUE <==> it's a user's turn.
**
** CALLS:       Unhash()
**              GetPrediction()      : Returns the prediction of the game
**
************************************************************************/
void PrintPosition(POSITION position, STRING playersName, BOOLEAN usersTurn) {
	int p1Matches, p2Matches, isP2Turn;
	unhash(position, &p1Matches, &p2Matches, &isP2Turn);
	printf("\n");
	printf("The number of matches currently on the board: %d\n", 15 - p1Matches - p2Matches);
	printf("The number of matches first player has is: %d\n", p1Matches);
	printf("The number of matches second player has is: %d\n", p2Matches);
	printf("It is Player %d's turn. %s\n", isP2Turn ? 2 : 1, GetPrediction(position, playersName, usersTurn));
	printf("\n");
}


/************************************************************************
**
** NAME:        PrintComputersMove
**
** DESCRIPTION: Nicely formats the computers move.
**
** INPUTS:      MOVE    computersMove : The computer's move.
**              STRING  computersName : The computer's name.
**
************************************************************************/
void PrintComputersMove(MOVE computersMove, STRING computersName) {
	printf("%8s's move: %d\n", computersName, computersMove);
}

/************************************************************************
**
** NAME:        PrintMove
**
** DESCRIPTION: Prints the move in a nice format.
**
** INPUTS:      MOVE move         : The move to print.
**
************************************************************************/
void PrintMove(MOVE move) {
	printf("%d", move);
}

/************************************************************************
**
** NAME:        MoveToString
**
** DESCRIPTION: Returns the move as a STRING
**
** INPUTS:      MOVE *theMove         : The move to put into a string.
**
************************************************************************/
void MoveToString(MOVE theMove, char *moveStringBuffer) {
	moveStringBuffer[0] = theMove + '0';
	moveStringBuffer[1] = '\0';
}

/************************************************************************
**
** NAME:        GetAndPrintPlayersMove
**
** DESCRIPTION: Finds out if the player wishes to undo, abort, or use
**              some other gamesman option. The gamesman core does
**              most of the work here.
**
** INPUTS:      POSITION position    : Current position
**              MOVE     *move       : The move to fill with user's move.
**              STRING   playersName : Current Player's Name
**
** OUTPUTS:     USERINPUT          : One of
**                                   (Undo, Abort, Continue)
**
** CALLS:       USERINPUT HandleDefaultTextInput(POSITION, MOVE*, STRING)
**                                 : Gamesman Core Input Handling
**
************************************************************************/

USERINPUT GetAndPrintPlayersMove(POSITION position, MOVE *move, STRING playersName) {
	USERINPUT input;
	for (;;) {
		printf("%8s's move [(u)ndo/1/2/3] : ", playersName);
		input = HandleDefaultTextInput(position, move, playersName);
		if (input != Continue) return input;
	}
	/* NOTREACHED */
	return Continue;
}

BOOLEAN ValidTextInput(STRING input) { return input[0] >= '1' && input[0] <= '3'; }
MOVE ConvertTextInputToMove(STRING input) { return input[0] - '0'; }
void SetTclCGameSpecificOptions(int options[]) { (void)options; }
int NumberOfOptions() { return 1; }
int getOption() { return 0; }
void setOption(int option) {
	(void) option;
}
void DebugMenu() {}
void GameSpecificMenu() {}

void PositionToString(POSITION position, char *positionStringBuffer) {
	int p1Matches, p2Matches, isP2Turn;
	unhash(position, &p1Matches, &p2Matches, &isP2Turn);
	snprintf(positionStringBuffer, 12, "%c_%d_%d", isP2Turn ? '2' : '1', p1Matches, p2Matches);
}

POSITION StringToPosition(char *positionString) {
	int turn;
	char *board;
	if (ParseStandardOnelinePositionString(positionString, &turn, &board)) {
		int isP2Turn = turn == 2 ? 1 : 0;
		int p1Matches = 0;
		int p2Matches = 0;
		int idxOfUnderscore = 7;
		for (int i = 0; i < 6; i++) {
			if (board[i] == '_') {
				idxOfUnderscore = i;
				break;
			} else if (board[i] == '\0') {
				break;
			}
		}
		if (idxOfUnderscore < 7) {
			board[idxOfUnderscore] = '\0';
			p1Matches = atoi(board);
			board[idxOfUnderscore] = '_';
			p2Matches = atoi(board + idxOfUnderscore + 1);
		} else {
			return NULL_POSITION;
		}
		return hash(p1Matches, p2Matches, isP2Turn);
	}
	return NULL_POSITION;
}

void PositionToAutoGUIString(POSITION position, char *buf) {
	int p1Matches, p2Matches, isP2Turn;
	unhash(position, &p1Matches, &p2Matches, &isP2Turn);
	buf[0] = isP2Turn ? '2' : '1';
	buf[1] = '_';
	memset(buf + 2, '-', 41);
	memset(buf + 2, 'n', 15 - p1Matches - p2Matches);
	memset(buf + 17, 'x', p1Matches);
	memset(buf + 29, 'o', p2Matches);
	if (15 - p1Matches - p2Matches) {
		snprintf(buf + 41, 13, "...~%d~%d~%d", 15 - p1Matches - p2Matches, p1Matches, p2Matches);
	} else {
		snprintf(buf + 41, 13, "...~~%d~%d", p1Matches, p2Matches);
	}
}

void MoveToAutoGUIString(POSITION position, MOVE move, char *autoguiMoveStringBuffer) {
  	int p1Matches, p2Matches, isP2Turn;
	unhash(position, &p1Matches, &p2Matches, &isP2Turn);
	char text[2];
	text[0] = move + '0';
	text[1] = '\0';
	AutoGUIMakeMoveButtonStringT(text, 15 - p1Matches - p2Matches - move, 'x', autoguiMoveStringBuffer);
}