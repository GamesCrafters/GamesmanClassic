/************************************************************************
**
** NAME:        m1210.c
**
** DESCRIPTION: The 1,2,...10 game
**
** AUTHOR:      Dan Garcia  -  University of California at Berkeley
**              Copyright (C) Dan Garcia, 1995. All rights reserved.
**
** DATE:        08/25/91
**
** UPDATE HIST:
**
**  9-04-91 1.0a1 : Added <crs> to PrintPosition and added call
**                  to ValidMove() and PrintPossibleMoves()
**  9-06-91 1.0a2 : Added the two extra arguments to PrintPosition
**                  Recoded the way to do "visited" - bitmask
**  9-16-91 1.0a6 : Replaced redundant with GetRawValueFromDatabase
**  5-12-92 1.0a7 : Added Static Evaluation - it's perfect!
** 05-15-95 1.0   : Final release code for M.S.
**
**************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/

#include "gamesman.h"

/*************************************************************************
**
** Game-specific constants
**
**************************************************************************/

CONST_STRING kGameName            = "1,2,...,10";
CONST_STRING kAuthorName          = "Dan Garcia and his GamesCrafters";
CONST_STRING kDBName              = "1210";

BOOLEAN kPartizan            = FALSE;
BOOLEAN kGameSpecificMenu    = FALSE;
BOOLEAN kTieIsPossible       = FALSE;
BOOLEAN kLoopy               = FALSE;

BOOLEAN kDebugMenu           = FALSE;
BOOLEAN kDebugDetermineValue = FALSE;

POSITION gNumberOfPositions   = 11;       /* Every board from 0 to 10 */
POSITION gInitialPosition     = 0;
POSITION kBadPosition         = -1;

void*    gGameSpecificTclInit = NULL;

POSITION gMinimalPosition     = 0;        /* Is this used by anyone? */

CONST_STRING kHelpGraphicInterface =
        "Not written yet";

CONST_STRING kHelpTextInterface    =
/*
   --------------------------------------------------------------------------------
 */
        "On your turn, select the number 1 or 2 to raise the total sum.\n"
        "After selecting your move, the total is displayed. \n"
        "The play then alternates to the second player, who also has the \n"
        "choice of raising the sum by 1 or 2 points. The winner is the first\n"
        "person to raise the total sum to exactly 10.";

CONST_STRING kHelpOnYourTurn =
        "Type 1 or 2 to choose how much you'd like to increase the total.";

CONST_STRING kHelpStandardObjective =
        "To be the first player to raise the total to 10.";

CONST_STRING kHelpReverseObjective =
        "To force your opponent to raise the total to 10.";

CONST_STRING kHelpTieOccursWhen = "";   /* empty since kTieIsPossible == FALSE */

CONST_STRING kHelpExample =
        "TOTAL                        :  0   \n\n\
     Dan's move [(u)ndo/1/2] : { 2 } \n\n\
TOTAL                        :  2    \n\n\
Computer's move              :  2    \n\n\
TOTAL                        :  4    \n\n\
     Dan's move [(u)ndo/1/2] : { 1 } \n\n\
TOTAL                        :  5    \n\n\
Computer's move              :  2    \n\n\
TOTAL                        :  7    \n\n\
     Dan's move [(u)ndo/1/2] : { 2 } \n\n\
TOTAL                        :  9    \n\n\
Computer's move              :  2    \n\n\
TOTAL                        : 11    \n\n\
Computer wins. Nice try, Dan."                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       ;

void PositionToString(POSITION position, char *positionStringBuffer);

/*************************************************************************
**
** #defines and structs
**
**************************************************************************/


/*************************************************************************
**
** Global Variables
**
*************************************************************************/


/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/


/************************************************************************
**
** NAME:        InitializeGame
**
** DESCRIPTION: Prepares the game for execution.
**              Initializes required variables.
**
************************************************************************/

void InitializeGame() {
	gPositionToStringFunPtr = &PositionToString;
	gSupportsMex = TRUE;
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
** CALLS:       MOVELIST *CreateMovelistNode(MOVE,MOVELIST *)
**
************************************************************************/

MOVELIST *GenerateMoves(POSITION position) {
	MOVELIST *head = NULL;

	/* If at 9, you can only go 1 to 10. Otherwise you can go 1 or 2 */
	if (position < 9)
		head = CreateMovelistNode(2,head);

	head = CreateMovelistNode(1,head);

	return(head);
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
************************************************************************/

POSITION DoMove(POSITION thePosition, MOVE theMove) {
	return(thePosition + theMove);
}


/************************************************************************
**
** NAME:        Primitive
**
** DESCRIPTION: Return the value of a position if it fulfills certain
**              'primitive' constraints. Some examples of this is having
**              three-in-a-row with TicTacToe. TicTacToe has two
**              primitives it can immediately check for, when the board
**              is filled but nobody has one = primitive tie. Three in
**              a row is a primitive lose, because the player who faces
**              this board has just lost. I.e. the player before him
**              created the board and won. Otherwise undecided.
**
** INPUTS:      POSITION position : The position to inspect.
**
** OUTPUTS:     (VALUE) an enum which is oneof: (win,lose,tie,undecided)
**
************************************************************************/

VALUE Primitive(POSITION position) {
	if(position == 10) /* If it's your turn, and it's 10, THEY got there! */
		return(gStandardGame ? lose : win);
	else
		return(undecided);
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
** CALLS:       PositionToBlankOX()
**              GetValueOfPosition()
**              GetPrediction()
**
************************************************************************/

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn) {
	//  VALUE GetValueOfPosition();

	printf("\nTOTAL                        : " POSITION_FORMAT " %s \n\n",
	       position, GetPrediction(position,playerName,usersTurn));
}


/************************************************************************
**
** NAME:        PrintComputersMove
**
** DESCRIPTION: Nicely format the computers move.
**
** INPUTS:      MOVE   *computersMove : The computer's move.
**              STRING  computersName : The computer's name.
**
************************************************************************/

void PrintComputersMove(MOVE computersMove, STRING computersName) {
	printf("%8s's move              : %1d\n", computersName, computersMove);
}

/************************************************************************
**
** NAME:        GetAndPrintPlayersMove
**
** DESCRIPTION: This finds out if the player wanted an undo or abort or not.
**              If so, return Undo or Abort and don't change theMove.
**              Otherwise get the new theMove and fill the pointer up.
**
** INPUTS:      POSITION *thePosition : The position the user is at.
**              MOVE *theMove         : The move to fill with user's move.
**              STRING playerName     : The name of the player whose turn it is
**
** OUTPUTS:     USERINPUT             : Oneof( Undo, Abort, Continue )
**
** CALLS:       ValidMove(MOVE, POSITION)
**              BOOLEAN PrintPossibleMoves(POSITION) ...Always True!
**
************************************************************************/

USERINPUT GetAndPrintPlayersMove(POSITION thePosition, MOVE *theMove, STRING playerName) {
	USERINPUT ret;

	do {
		printf("%8s's move [(u)ndo/1/2] : ", playerName);

		ret = HandleDefaultTextInput(thePosition, theMove, playerName);
		if(ret != Continue)
			return(ret);
	}
	while (TRUE);
	return(Continue); /* this is never reached, but lint is now happy */
}


/************************************************************************
**
** NAME:        ValidTextInput
**
** DESCRIPTION: Rudimentary check to check if input is in the move form
**              you are expecting. Does not check if it is a valid move.
**              Only checks if it fits the move form.
**
**              Reserved Input Characters - DO NOT USE THESE ONE CHARACTER
**                                          COMMANDS IN YOUR GAME
**              ?, s, u, r, h, a, c, q
**                                          However, something like a3
**                                          is okay.
**
**              Example: Tic-tac-toe Move Format : Integer from 1 to 9
**                       Only integers between 1 to 9 are accepted
**                       regardless of board position.
**                       Moves will be checked by the core.
**
** INPUTS:      STRING input : The string input the user typed.
**
** OUTPUTS:     BOOLEAN      : TRUE if the input is a valid text input.
**
************************************************************************/

BOOLEAN ValidTextInput(STRING input) {
	return input[0] == '1' || input[0] == '2';
}


/************************************************************************
**
** NAME:        ConvertTextInputToMove
**
** DESCRIPTION: Convert the string input to the internal move representation.
**
** INPUTS:      STRING input : The string input the user typed.
**
** OUTPUTS:     MOVE : The move corresponding to the user's input.
**
************************************************************************/

MOVE ConvertTextInputToMove(STRING input) {
	return((MOVE) input[0] - '0'); /* user inputs '1','2', our rep. is 1,2 */
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

void GameSpecificMenu() {}


/************************************************************************
**
** NAME:        SetTclCGameSpecificOptions
**
** DESCRIPTION: Set the C game-specific options (called from Tcl)
**              Ignore if you don't care about Tcl for now.
**
************************************************************************/

void SetTclCGameSpecificOptions(int theOptions[]) {
	/* No need to have anything here, we have no extra options */
	(void)theOptions;
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

void MoveToString(MOVE move, char *moveStringBuffer) {
	moveStringBuffer[0] = move + '0';
	moveStringBuffer[1] = '\0';
}

/************************************************************************
**
** NAME:        NumberOfOptions
**
** DESCRIPTION: Calculates and returns the number of variants
**              your game supports.
**
** OUTPUTS:     int : Number of Game Variants
**
************************************************************************/

int NumberOfOptions() {
	return 2;
}


/************************************************************************
**
** NAME:        getOption
**
** DESCRIPTION: A hash function that returns a number corresponding
**              to the current variant of the game.
**              Each set of variants needs to have a different number.
**
** OUTPUTS:     int : the number representation of the options.
**
************************************************************************/

int getOption() {
	return(gStandardGame ? 1 : 2);
}


/************************************************************************
**
** NAME:        setOption
**
** DESCRIPTION: The corresponding unhash function for game variants.
**              Unhashes option and sets the necessary variants.
**
** INPUT:       int option : the number representation of the options.
**
************************************************************************/

void setOption(int option) {
	if      (option == 1)
		gStandardGame = TRUE;
	else if (option == 2)
		gStandardGame = FALSE;
	else
		BadElse("setOption\n");
}

/************************************************************************
**
** NAME:        DebugMenu
**
** DESCRIPTION: Game Specific Debug Menu (Gamesman comes with a default
**              debug menu). Menu used to debug internal problems.
**
**              If kDebugMenu == FALSE
**                   Gamesman will not display a debug menu option
**                   Gamesman will not call this function
**
************************************************************************/

void DebugMenu() {}


STRING GetNextMoveValues(char* board, int option) {
	setOption(option);
	Initialize();
	InitializeDatabases();

	POSITION pos = atoi(board);

	// check for primitive:
	if (Primitive(pos)) {
		// primitive! return value of this board?
		printf("Primitive Position: %lld, Value: %d\n", pos, Primitive(pos));
	}
	else {
		MOVELIST* moves = GenerateMoves(pos);
		printf("Not Primitive Position: %lld\n", pos);
		while (moves != NULL) {
			POSITION child = DoMove(pos, moves->move);
			moves = moves->next;
			printf("Child position: %lld, Value: %d\n", child, DetermineValue(child));

		}
	}


	return NULL;
}

/* Position String is formatted: <turn>_<number>. Example: 2_5 means that
it is Player 2's turn and the current number is 5. However, since
this is an impartial game, we will set the turn character to 0. */
void PositionToString(POSITION position, char *positionStringBuffer) {
	snprintf(positionStringBuffer, 8, "0_%d", (int) position);
}

POSITION StringToPosition(char *positionString) {
	int turn;
	char *number;
	if (ParseStandardOnelinePositionString(positionString, &turn, &number)) {
		POSITION position = (POSITION) atoi(number);
		if (position >= gNumberOfPositions) {
			return NULL_POSITION;
		} else {
			return position;
		}
	}
	return NULL_POSITION;
}

void PositionToAutoGUIString(POSITION position, char *autoguiPositionStringBuffer) {
	char entityString[gNumberOfPositions + 1];
	for (unsigned i = 0; i < gNumberOfPositions; i++) {
		entityString[i] = '-';
	}
	entityString[position] = 'x';
	entityString[gNumberOfPositions] = '\0';
  	AutoGUIMakePositionString(0, entityString, autoguiPositionStringBuffer);
}

void MoveToAutoGUIString(POSITION position, MOVE move, char *autoguiMoveStringBuffer) {
  	AutoGUIMakeMoveButtonStringM(position, position + move, 'x', autoguiMoveStringBuffer);
}