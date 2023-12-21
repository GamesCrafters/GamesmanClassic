
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

STRING MoveToString(MOVE);

POSITION hash(int p1Matches, int p2Matches, int isP2Turn) {
	return (p2Matches * 26 + (p1Matches << 1)) | isP2Turn;
}

void unhash(POSITION position, int *p1Matches, int *p2Matches, int *isP2Turn) {
	*p2Matches = position / 26;
	*p1Matches = (position >> 1) % 13;
	*isP2Turn = position & 1;
}

void InitializeGame() { gMoveToStringFunPtr = &MoveToString; }

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
STRING MoveToString(MOVE theMove) {
	STRING move = (STRING) SafeMalloc(2);
	move[0] = theMove + '0';
	move[1] = '\0';
	return move;
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
POSITION GetInitialPosition() { return 0; }
int NumberOfOptions() { return 1; }
int getOption() { return 0; }
void setOption(int option) {}
void DebugMenu() {}
void GameSpecificMenu() {}

POSITION InteractStringToPosition(STRING str) {
	int isP2Turn = str[2] == 'B' ? 1 : 0;
	str += 23;
	int p1Matches = 0, p2Matches = 0;
	for (int i = 0; i < 24; i++) {
		if (str[i] == 'x') {
			p1Matches++;
		} else if (str[i] == 'o') {
			p2Matches++;
		}
	}
	return hash(p1Matches, p2Matches, isP2Turn);
}

STRING InteractPositionToString(POSITION position) {
	int p1Matches, p2Matches, isP2Turn;
	unhash(position, &p1Matches, &p2Matches, &isP2Turn);
	char *buf = (char *) SafeMalloc(60);
	snprintf(buf, 9, "R_%c_0_0_", isP2Turn ? 'B' : 'A');
	memset(buf + 8, '\0', 52);
	memset(buf + 8, '-', 39);
	memset(buf + 8, 'n', 15 - p1Matches - p2Matches);
	memset(buf + 23, 'x', p1Matches);
	memset(buf + 35, 'o', p2Matches);
	if (15 - p1Matches - p2Matches) {
		snprintf(buf + 47, 13, "...~%d~%d~%d", 15 - p1Matches - p2Matches, p1Matches, p2Matches);
	} else {
		snprintf(buf + 47, 13, "...~~%d~%d", p1Matches, p2Matches);
	}
	return buf;
}

STRING InteractMoveToString(POSITION pos, MOVE mv) {
	int p1Matches, p2Matches, isP2Turn;
	unhash(pos, &p1Matches, &p2Matches, &isP2Turn);
	STRING moveString = UWAPI_Board_Regular2D_MakeAddStringWithSound(mv + '0', 15 - p1Matches - p2Matches - mv, 'x');
	moveString[0] = 'T';
	return moveString;
}
