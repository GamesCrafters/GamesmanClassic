/************************************************************************
**
** NAME:         miceblocks.c
**
** DESCRIPTION:  Ice Blocks
**
** AUTHOR:       Kevin Duncan, Neil Trotter
**
** DATE:         2004.05.01
**
** UPDATE HIST:  a lot of shit happened.
**
**************************************************************************/

#include <stdio.h>
#include "gamesman.h"
#include "hash.h"
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

#define X               99999
#define O               9999
#define MAX_BOARD_SIZE  6
#define MIN_BOARD_SIZE  2
#define NUM_OPTIONS     3

/* External Globals */
#ifndef MEMWATCH
extern GENERIC_PTR SafeMalloc ();
extern void SafeFree ();
#endif
//extern VALUE *gDatabase;
extern STRING gValueString[];
/* External Globals */

/* Globals setup for use by outside functions */
POSITION gNumberOfPositions   = 0;
POSITION gInitialPosition     = 0;
POSITION kBadPosition         = -1;
STRING kAuthorName          = "Kevin Duncan and Neil Trotter";
STRING kGameName            = "Ice Blocks";
STRING kDBName              = "iceblocks";
BOOLEAN kPartizan            = TRUE;
BOOLEAN kDebugMenu           = FALSE;
BOOLEAN kGameSpecificMenu    = TRUE;
BOOLEAN kTieIsPossible       = TRUE;
BOOLEAN kLoopy               = FALSE;
BOOLEAN kDebugDetermineValue = FALSE;
void*    gGameSpecificTclInit = NULL;
/* Globals setup for use by outside functions */

/* Help strings for this game */
STRING gStandard =
        "The objective of this game is to obtain more points than your opponent.\n\
You get 3 points for getting 3 blocks in a row and two additional points\n\
for every following block.\n\
Example: 3 in a row gets 3 points, 4 in a row gets 5 points,5 in a row gets\n\
7 points. Diagonals are valid."                                                                                                                                                                                                                                                                             ;
STRING gTallyBlocks =
        "The objective of this game is to get the most number of blocks in a single\n\
row."                                                                                       ;
STRING gTallyThrees =
        "The objective of this game is to get more threes in a row than your\n\
opponent.  If you have 4 in a row, it will be counted as two threes in a\n\
row.  Same goes for any sequence of blocks greater than 4. Diagonals are\n\
valid."                                                                                                                                                                                                                                        ;
STRING gStandardR =
        "The objective of this game is to obtain less points than your opponent.\n\
You get 3 points for getting 3 blocks in a row and two additional points\n\
for every following block.\n\
Example: 3 in a row gets 3 points, 4 in a row gets 5 points,5 in a row gets\n\
7 points. Diagonals are valid."                                                                                                                                                                                                                                                                             ;
STRING gTallyBlocksR =
        "The objective of this game is to not get the most number of blocks in a\n\
single row."                                                                                    ;
STRING gTallyThreesR =
        "The objective of this game is to get less threes in a row than your\n\
opponent. If you have 4 in a row, it will be counted as two threes in a row.\n\
Same goes for any sequence of blocks greater than 4. Diagonals are valid."                                                                                                                                                                ;
STRING kHelpGraphicInterface =
        "Not written yet";
STRING kHelpTextInterface =
        "On your turn, determine where you wish to move your piece.Simply type in\n\
the numeric representation of that position as it is presented on the board."                                                                                     ;
STRING kHelpOnYourTurn =
        "Place a block in the pyramid by entering the corresponding number.\n\
You may only place a block if it is on the bottom row, or on top of two\n\
adjacent pieces."                                                                                                                                                          ;
STRING kHelpStandardObjective = NULL;
STRING kHelpReverseObjective = NULL;
STRING kHelpTieOccursWhen =
        "each player has accumulated an equal\n"
        "number of points by the end of the game.";
STRING kHelpExample =
        "      [10]\n"
        "    [08][09]\n"
        "  [05][06][07]\n"
        "[01][02][03][04]\n\n"
        "Dan's Move: 2\n\n"
        "      [10]\n"
        "    [08][09]\n"
        "  [05][06][07]\n"
        "[01][XX][03][04]\n\n"
        "Computer's Move: 3\n\n"
        "      [10]\n"
        "    [08][09]\n"
        "  [05][06][07]\n"
        "[01][XX][OO][04]\n\n"
        "Dan's Move: 6\n\n"
        "      [10]\n"
        "    [08][09]\n"
        "  [05][XX][07]\n"
        "[01][XX][OO][04]\n\n"
        "etc...";
/* Help strings for this game */

/* Board representation for this game */
typedef enum win_condition {
	standard, tallyblocks, tallythrees
} CONDITION;
typedef int TURN;
typedef struct boardRep *BOARD;
struct boardRep {
	int **spaces;
	TURN turn;
};
int base = 5;
CONDITION WinningCondition = standard;
/* Board representation for this game */

BOOLEAN gMenu = FALSE;

/* Function declarations */
int sumto (int);
int vcfg (int *);
BOARD arraytoboard (POSITION);
POSITION boardtoarray (BOARD);
void ChangeBoardSize();
void SetWinningCondition();
WINBY computeWinBy (POSITION);

STRING MoveToString(MOVE);
/* Function declarations */


/*************************** GAME FUNCTIONS ****************************/


/************************************************************************
**
** NAME:        DebugMenu
**
** DESCRIPTION: Menu used to debub internal problems. Does nothing if
**              kDebugMenu == FALSE
**
************************************************************************/

void DebugMenu () {
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

void GameSpecificMenu () {
	char c;
	BOOLEAN cont = TRUE;
	while (cont) {
		printf("\n\nGame Options:\n\n"
		       "\tc)\t(C)hange the size of the board (change the base length)\n"
		       "\tm)\t(M)odify the winning conditions of the game\n"
		       "\tt)\t(T)oggle from ");
		if(gStandardGame)
			printf("STANDARD to MISERE\n");
		else
			printf("MISERE to STANDARD\n");
		printf("\tb)\t(B)ack to the main menu\n"
		       "\nSelect an option:  ");
		getc(stdin);
		c = tolower(getc(stdin));
		switch (c) {
		case 'c':
			ChangeBoardSize();
			break;
		case 'm':
			SetWinningCondition();
			break;
		case 't':
			gStandardGame = !gStandardGame;
			if(gStandardGame)
				printf("STANDARD\n");
			else
				printf("MISERE\n");
			break;
		case 'b':
			cont = FALSE;
		default:
			printf("Invalid option!\n");
		}
	}
}

/************************************************************************
**
** NAME:        InitializeGame
**
** DESCRIPTION: Initialize the gDatabase, a global variable. and the other
**              local variables.
**
************************************************************************/

void InitializeGame () {
	int i, sum = sumto(base);
	char board[sum];
	int theBoard[10] = {'x', 0, ((sum / 2) + (sum % 2)), 'o', 0, (sum / 2), '-', 0, sum, -1};
	gNumberOfPositions = generic_hash_init(sum, theBoard, vcfg, 0);
	for(i = 0; i < sum; i++)
		board[i] = '-';
	gInitialPosition = generic_hash_hash(board, 1);
	if(!kHelpStandardObjective) {
		kHelpStandardObjective = gStandard;
		kHelpReverseObjective = gStandardR;
	}

	gMoveToStringFunPtr = &MoveToString;
	gPutWinBy = &computeWinBy;
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

MOVELIST *GenerateMoves (POSITION position) {
	int i, j;
	MOVELIST *head = NULL;
	MOVELIST *CreateMovelistNode();
	BOARD board = arraytoboard(position);
	for (i = 0; i < (base - 1); i++) {
		for (j = 0; j < (base - i); j++) {
			if ((i == 0) && (board->spaces[i][j] < 999))
				head = CreateMovelistNode(j + 1, head);
			else if((board->spaces[i][j] >= 999) &&
			        ((j + 1) < (base - i)) &&
			        (board->spaces[i][j+1] >= 999) &&
			        ((i+1) < base) &&
			        (board->spaces[i + 1][j] < 999))
				head = CreateMovelistNode(((i + 1) * base - sumto(i) + j + 1), head);
		}
	}
	return head;
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
	BOARD board = arraytoboard(thePosition);
	int i;
	for (i = 0; theMove - (base - i) > 0; theMove -= (base - i), i++) ;
	board->spaces[i][theMove - 1] = board->turn;
	if(board->turn == X)
		board->turn = O;
	else
		board->turn = X;
	return boardtoarray(board);
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

VALUE Primitive (POSITION pos) {
	BOARD board = arraytoboard(pos);
	int i, j, countX = 0, countO = 0, color, k, m, count;
	int pointsX = 0, pointsO = 0, threesX = 0, threesO = 0;
	int dlvisited[base][base], drvisited[base][base], hvisited[base][base];
	for(i = 0; i < base; i++) {
		for(j = 0; j < base; j++) {
			dlvisited[i][j] = 0;
			drvisited[i][j] = 0;
			hvisited[i][j] = 0;
		}
	}
	for(i = 0; i < base; i++) {
		for(j = 0; j < (base - i); j++) {
			if(board->spaces[i][j] < 999)
				return undecided;
			color = board->spaces[i][j];
			for(k = i, m = j, count = 0; k < base && m >= 0 &&
			    ((WinningCondition == tallythrees) ? TRUE : !dlvisited[k][m]) &&
			    board->spaces[k][m] == color; k++, m--) {
				count++;
				dlvisited[k][m] = 1;
			}
			if((count > countX) && (color == X))
				countX = count;
			else if ((count > countO) && (color == O))
				countO = count;
			if((count > 2) && (color == X)) {
				pointsX += 3 + (count - 3) * 2;
				threesX++;
			}
			else if((count > 2) && (color == O)) {
				pointsO += 3 + (count - 3) * 2;
				threesO++;
			}
			for (k = i, m = j, count = 0; k < base && m < (base - k) &&
			     ((WinningCondition == tallythrees) ? TRUE : !drvisited[k][m]) &&
			     board->spaces[k][m] == color; k++) {
				count++;
				drvisited[k][m] = 1;
			}
			if ((count > countX) && (color == X))
				countX = count;
			else if ((count > countO) && (color == O))
				countO = count;
			if((count > 2) && (color == X)) {
				pointsX += 3 + (count - 3) * 2;
				threesX++;
			}
			else if((count > 2) && (color == O)) {
				pointsO += 3 + (count - 3) * 2;
				threesO++;
			}
			for (k = i, m = j, count = 0; m < (base - k) &&
			     ((WinningCondition == tallythrees) ? TRUE : !hvisited[k][m]) &&
			     board->spaces[k][m] == color; m++) {
				count++;
				hvisited[k][m] = 1;
			}
			if((count > countX) && (color == X))
				countX = count;
			else if ((count > countO) && (color == O))
				countO = count;
			if((count > 2) && (color == X)) {
				pointsX += 3 + (count - 3) * 2;
				threesX++;
			}
			else if((count > 2) && (color == O)) {
				pointsO += 3 + (count - 3) * 2;
				threesO++;
			}
		}
	}
	switch (WinningCondition) {
	case standard:
		if(pointsX > pointsO) {
			if(board->turn == X)
				return (gStandardGame ? win : lose);
			else
				return (gStandardGame ? lose : win);
		}
		else if(pointsO > pointsX) {
			if(board->turn == O)
				return (gStandardGame ? win : lose);
			else
				return (gStandardGame ? lose : win);
		}
		else
			return tie;
	case tallyblocks:
		if(countX > countO) {
			if(board->turn == X)
				return (gStandardGame ? win : lose);
			else
				return (gStandardGame ? lose : win);
		}
		else if(countO > countX) {
			if(board->turn == O)
				return (gStandardGame ? win : lose);
			else
				return (gStandardGame ? lose : win);
		}
		else
			return tie;
	case tallythrees:
		if(threesX > threesO) {
			if(board->turn == X)
				return (gStandardGame ? win : lose);
			else
				return (gStandardGame ? lose : win);
		}
		else if(threesO > threesX) {
			if(board->turn == O)
				return (gStandardGame ? win : lose);
			else
				return (gStandardGame ? lose : win);
		}
		else
			return tie;
	}

	//should never reach here
	return undecided;
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
	int i, numX, numO, turn;
	char input[sumto(base)], c;
	BOOLEAN cont = TRUE;
	while(cont) {
		cont = FALSE;
		printf("type in a %d character string of X's, O's and -'s (- denotes empty):  ", sumto(base));
		scanf("%s", input);
		for(i = 0; i < sumto(base); i++) {
			c = tolower(input[i]);
			if(c == 'x')
				numX++;
			else if(c == 'o')
				numO++;
		}
		if((numX - numO) != 0 || (numX - numO) != 1) {
			printf("\nInvalid Board!\n\n");
			cont = TRUE;
		}
		else if(numX > numO)
			turn = 0;
		else
			turn = 1;
	}
	return generic_hash_hash(input, turn);
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
	BOARD board = arraytoboard(position);
	int i, j;
	for(i = base - 1; i >= 0; i--) {
		printf("  ");
		for(j = 0; j < i; j++)
			printf("  ");
		for(j = 0; j < (base - i); j++) {
			printf("[");
			if(board->spaces[i][j] == X)
				printf("XX]");
			else if(board->spaces[i][j] == O)
				printf("OO]");
			else {
				if(board->spaces[i][j] < 10)
					printf("0");
				printf("%d]", board->spaces[i][j]);
			}
		}
		printf("\n");
	}
	if(board->turn == X)
		printf("  Player X's turn.");
	else
		printf("  Player O's turn.");
	if(!gMenu)
		printf("          %s\n\n", GetPrediction(position, playerName, usersTurn));
}

/************************************************************************
**
** NAME:        GetAndPrintPlayersMove
**
** DESCRIPTION: This finds out if the player wanted an undo or abort or not.
**              If so, return Undo or Abort and don't change theMove.
**              Otherwise get the new theMove and fill the pointer up.
**
** INPUTS:      POSITION thePosition : The position the user is at.
**              MOVE theMove         : The move to fill with user's move.
**              STRING playerName     : The name of the player whose turn it is
**
** OUTPUTS:     USERINPUT             : Oneof( Undo, Abort, Continue )
**
** CALLS:       ValidMove(MOVE, POSITION)
**              BOOLEAN PrintPossibleMoves(POSITION) ...Always True!
**
************************************************************************/

USERINPUT GetAndPrintPlayersMove (POSITION thePosition, MOVE *theMove, STRING playerName) {
	BOOLEAN ValidMove();
	USERINPUT ret, HandleDefaultTextInput();
	do {
		printf("%8s's move [(u)ndo/(1-%d)] :  ", playerName, sumto(base));
		ret = HandleDefaultTextInput(thePosition, theMove, playerName);
		if(ret != Continue)
			return(ret);
	}
	while (TRUE);
	return(Continue); /* this is never reached, but lint is now happy */
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
	printf("%8s's move              : %d\n\n", computersName, computersMove);
}

/************************************************************************
**
** NAME:        PrintMove
**
** DESCRIPTION: Print the move in a nice format.
**
** INPUTS:      MOVE *theMove         : The move to print.
**
************************************************************************/

void PrintMove (MOVE move) {
	STRING m = MoveToString( move );
	printf( "%s", m );
	SafeFree( m );
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

STRING MoveToString (theMove)
MOVE theMove;
{
	STRING move = (STRING) SafeMalloc(3);
	sprintf(move, "%d", theMove);
	return move;
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

BOOLEAN ValidTextInput (STRING input) {
	int i;
	i = atoi(input);
	return ((i > 0) && (i <= sumto(base)));
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

MOVE ConvertTextInputToMove (STRING input) {
	return ((MOVE) atoi(input));
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

int NumberOfOptions () {
	return (MAX_BOARD_SIZE - MIN_BOARD_SIZE + 1) * NUM_OPTIONS * 2;
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

int getOption () {
	int option;
	option = base - MIN_BOARD_SIZE;
	option *= NUM_OPTIONS;
	switch(WinningCondition) {
	case standard:
		option += 0;
		break;
	case tallyblocks:
		option += 1;
		break;
	case tallythrees:
		option += 2;
		break;
	}
	option *= 2;
	if(!gStandardGame)
		option += 1;
	option++;
	return option;
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
	option--;
	if(option % 2)
		gStandardGame = FALSE;
	else
		gStandardGame = TRUE;
	option /= 2;
	switch(option % NUM_OPTIONS) {
	case 0:
		WinningCondition = standard;
		break;
	case 1:
		WinningCondition = tallyblocks;
		break;
	case 2:
		WinningCondition = tallythrees;
		break;
	}
	option /= NUM_OPTIONS;
	base = option + MIN_BOARD_SIZE;
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
}


/***********************************************************************
 ***********************************************************************
 ** Helper functions local to this file...
 ***********************************************************************
 **********************************************************************/

/***********************************************************************
**
** NAME:         sumto
**
** DESCRIPTION:  Returns a cumulative sum from 1 to i.
**
** INPUT:        i, the integer to sum to.
**
** OUTPUT:       the cumulative sum.
**
***********************************************************************/
int sumto (int i) {
	int ans = 0;
	while(i > 0) {
		ans += i;
		i--;
	}
	return ans;
}

/* Required function by Michel's hash for dartboard behavior from hash */
int vcfg (int *this_cfg) {
	return ((this_cfg[0] == this_cfg[1]) || (this_cfg[0] == (this_cfg[1] + 1)));
}

/************************************************************************
**
** NAME:         arraytoboard
**
** DESCRIPTION:  Intermediate function between our board representation
**               and Michel's hash.  Takes in the POSITION, unhashes it
**               and then turns the resulting array into our board
**               representation.
**
** INPUT:        position, the hashed POSITION of the board.
**
** OUPTUT:       A BOARD, our board representation.
**
***********************************************************************/

BOARD arraytoboard (POSITION position) {
	char *theBoard = (char *) SafeMalloc(sumto(base) * sizeof(char));
	int i, j, k;
	BOARD board = (BOARD) SafeMalloc(sizeof(struct boardRep));
	board->spaces = (int **) SafeMalloc(base * sizeof(int *));
	for(i = 0; i < base; i++)
		board->spaces[i] = (int *) SafeMalloc((base - i) * sizeof(int));
	theBoard = generic_hash_unhash(position, theBoard);
	for(i = 0, k = 0; i < base; i++) {
		for(j = 0; j < (base - i); j++, k++) {
			if(theBoard[k] == '-')
				board->spaces[i][j] = i * base - sumto(i - 1) + j + 1;
			else if(theBoard[k] == 'x')
				board->spaces[i][j] = X;
			else if(theBoard[k] == 'o')
				board->spaces[i][j] = O;
		}
	}
	if(generic_hash_turn(position) - 1)
		board->turn = O;
	else
		board->turn = X;
	return board;
}



/************************************************************************
**
** NAME:         boardtoarray
**
** DESCRIPTION:  Intermediate function between our board representation
**               and Michel's hash.  Takes in the BOARD, converts it
**               into an the array appropriate for Michel's hash and
**               then calls Michel's hash to get the hashed POSITION of
**               the board.
**
** INPUT:        BOARD, the representation of that board.
**
** OUPTUT:       A POSITION, the hashed value of that BOARD.
**
***********************************************************************/

POSITION boardtoarray (BOARD board) {
	char theBoard[sumto(base)];
	int i, j, k;
	TURN whoseTurn;
	for(i = 0, k = 0; i < base; i++) {
		for(j = 0; j < base - i; j++, k++) {
			if(board->spaces[i][j] < 999)
				theBoard[k] = '-';
			else if(board->spaces[i][j] == X)
				theBoard[k] = 'x';
			else if(board->spaces[i][j] == O)
				theBoard[k] = 'o';
		}
	}
	if(board->turn == X)
		whoseTurn = 1;
	else
		whoseTurn = 2;
	for(i = 0; i < base; i++)
		SafeFree(board->spaces[i]);
	SafeFree(board->spaces);
	SafeFree(board);
	return generic_hash_hash(theBoard, whoseTurn);
}

/************************************************************************
**
** NAME:         ChangeBoardSize
**
** DESCRIPTION:  Handles menu option to change the board size.
**
***********************************************************************/

void ChangeBoardSize () {
	int change;
	BOOLEAN cont = TRUE;
	while (cont) {
		cont = FALSE;
		printf("\n\nCurrent board with base %d:\n\n", base);
		gMenu = TRUE;
		PrintPosition(gInitialPosition, "Fred", 0);
		gMenu = FALSE;
		printf("\n\nEnter the new base length (%d - %d):  ", MIN_BOARD_SIZE, MAX_BOARD_SIZE);
		scanf("%d", &change);
		if(change > MAX_BOARD_SIZE || change < 2) {
			printf("\nInvalid base length!\n");
			cont = TRUE;
		}
		else {
			base = change;
			InitializeGame();
		}
	}
}

/************************************************************************
**
** NAME:         SetWinningCondition
**
** DESCRIPTION:  Handles menu option to change the winning condition.
**
***********************************************************************/

void SetWinningCondition () {
	char c;
	BOOLEAN cont = TRUE;
	while(cont) {
		cont = FALSE;
		printf("\n\nWinning Condition Options:\n\n"
		       "\ts)\t(S)tandard game. Three points for every\n"
		       "\t\tstring of three and two additional points\n"
		       "\t\tfor every additional block.\n"
		       "\tm)\t(M)ost number of blocks in a row\n"
		       "\tn)\tMost (n)umber of three's in a row\n"
		       "\nEnter your choice for the winning condition:  ");
		getc(stdin);
		c = tolower(getc(stdin));
		switch (c) {
		case 's':
			WinningCondition = standard;
			kHelpStandardObjective = gStandard;
			kHelpReverseObjective = gStandardR;
			break;
		case 'm':
			WinningCondition = tallyblocks;
			kHelpStandardObjective = gTallyBlocks;
			kHelpReverseObjective = gTallyBlocksR;
			break;
		case 'n':
			WinningCondition = tallythrees;
			kHelpStandardObjective = gTallyThrees;
			kHelpReverseObjective = gTallyThreesR;
			break;
		default:
			cont = TRUE;
		}
	}
}

WINBY computeWinBy(POSITION pos) {
	BOARD board = arraytoboard(pos);
	int i, j, countX = 0, countO = 0, color, k, m, count;
	int pointsX = 0, pointsO = 0, threesX = 0, threesO = 0;
	int dlvisited[base][base], drvisited[base][base], hvisited[base][base];
	for(i = 0; i < base; i++) {
		for(j = 0; j < base; j++) {
			dlvisited[i][j] = 0;
			drvisited[i][j] = 0;
			hvisited[i][j] = 0;
		}
	}
	for(i = 0; i < base; i++) {
		for(j = 0; j < (base - i); j++) {
			color = board->spaces[i][j];
			for(k = i, m = j, count = 0; k < base && m >= 0 &&
			    ((WinningCondition == tallythrees) ? TRUE : !dlvisited[k][m]) &&
			    board->spaces[k][m] == color; k++, m--) {
				count++;
				dlvisited[k][m] = 1;
			}
			if((count > countX) && (color == X))
				countX = count;
			else if ((count > countO) && (color == O))
				countO = count;
			if((count > 2) && (color == X)) {
				pointsX += 3 + (count - 3) * 2;
				threesX++;
			}
			else if((count > 2) && (color == O)) {
				pointsO += 3 + (count - 3) * 2;
				threesO++;
			}
			for (k = i, m = j, count = 0; k < base && m < (base - k) &&
			     ((WinningCondition == tallythrees) ? TRUE : !drvisited[k][m]) &&
			     board->spaces[k][m] == color; k++) {
				count++;
				drvisited[k][m] = 1;
			}
			if ((count > countX) && (color == X))
				countX = count;
			else if ((count > countO) && (color == O))
				countO = count;
			if((count > 2) && (color == X)) {
				pointsX += 3 + (count - 3) * 2;
				threesX++;
			}
			else if((count > 2) && (color == O)) {
				pointsO += 3 + (count - 3) * 2;
				threesO++;
			}
			for (k = i, m = j, count = 0; m < (base - k) &&
			     ((WinningCondition == tallythrees) ? TRUE : !hvisited[k][m]) &&
			     board->spaces[k][m] == color; m++) {
				count++;
				hvisited[k][m] = 1;
			}
			if((count > countX) && (color == X))
				countX = count;
			else if ((count > countO) && (color == O))
				countO = count;
			if((count > 2) && (color == X)) {
				pointsX += 3 + (count - 3) * 2;
				threesX++;
			}
			else if((count > 2) && (color == O)) {
				pointsO += 3 + (count - 3) * 2;
				threesO++;
			}
		}
	}
	return pointsX - pointsO;
}

/* end of file. */
POSITION StringToPosition(char* board) {
	// FIXME: this is just a stub
	return atoi(board);
}


char* PositionToString(POSITION pos) {
	// FIXME: this is just a stub
	return "Implement Me";
}

char * PositionToEndData(POSITION pos) {
	return NULL;
}
