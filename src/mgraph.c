/************************************************************************
**
** NAME:        mgraph.c
**
** DESCRIPTION: The generic graph game
**
** AUTHOR:      Dan Garcia  -  University of California at Berkeley
**              Copyright (C) Dan Garcia, 1996. All rights reserved.
**
** DATE:        96-04-20
**
**************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/

#include "gamesman.h"

/* variables */
POSITION gNumberOfPositions  = 50; /* Arbitrary upper-limit on graph nodes */

POSITION gInitialPosition    = 0;
POSITION gMinimalPosition    = 0;
POSITION kBadPosition           = -1;

CONST_STRING kAuthorName     = "Dan Garcia";
CONST_STRING kGameName       = "Graph";
CONST_STRING kDBName         = "graph";
STRING kDBNameVolatile 	     = NULL;
BOOLEAN kPartizan            = TRUE;
BOOLEAN kDebugMenu           = TRUE;
BOOLEAN kGameSpecificMenu    = TRUE;
BOOLEAN kTieIsPossible       = FALSE;
BOOLEAN kLoopy               = TRUE;  /* set this to true */
BOOLEAN kDebugDetermineValue = FALSE;
void*    gGameSpecificTclInit = NULL;

CONST_STRING kHelpGraphicInterface = "";    /* empty since kSupportsGraphics == FALSE */

CONST_STRING kHelpTextInterface    =
        "On your turn, type in the number 1 or 2 and hit return. If at any point\n\
you have made a mistake, you can type u and hit return and the system will\n\
revert back to your most recent position."                                                                                                                                                                  ;

CONST_STRING kHelpOnYourTurn =
        "You say either 1 or 2. A running total (sum) is kept.";

CONST_STRING kHelpStandardObjective =
        "To be the FIRST player to raise the total above 10.";

CONST_STRING kHelpReverseObjective =
        "To be the LAST player to raise the total above 10. (i.e. to force your\n\
opponent into raising the total above 10 first."                                                                                   ;

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

/*************************************************************************
**
** Everything above here must be in every game file
**
**************************************************************************/

/*************************************************************************
**
** Every variable declared here is only used in this file (game-specific)
**
**************************************************************************/

/* function prototypes */
VALUE TextToValue(char c);
void LoadGraphFromFile(STRING graphFilename);
BOOLEAN GraphGoAgain(POSITION position, MOVE move);

/*** These are so that we can bootstrap and load the graphs on the fly ***/

char gGraphFilename[MAXINPUTLENGTH];
BOOLEAN gGraphFilenameSet = FALSE;

VALUE *gGraphPrimitiveList = NULL;
POSITIONLIST **gGraphNeighborList = NULL;
POSITION gLargestNodeNumber;

/************************************************************************
**
** NAME:        InitializeGame
**
** DESCRIPTION: Initialize the game's global variables
**
************************************************************************/

void InitializeGame()
{
	if (gGraphPrimitiveList != NULL) {
		SafeFree(gGraphPrimitiveList);
	}
	gGraphPrimitiveList = (VALUE *) SafeMalloc (gNumberOfPositions * sizeof(VALUE));

	if (gGraphNeighborList != NULL) {
		SafeFree(gGraphNeighborList);
	}
	gGraphNeighborList = (POSITIONLIST **) SafeMalloc (gNumberOfPositions * sizeof(POSITIONLIST *));

	if (!gGraphFilenameSet) {
		(void) sprintf((char *)gGraphFilename, "../meta/default.grf");
		if (kDBNameVolatile) SafeFree(kDBNameVolatile);
		kDBNameVolatile = (STRING) SafeMalloc(sizeof(char)*MAXINPUTLENGTH);
		sprintf(kDBNameVolatile, "graph-default");
		kDBName = kDBNameVolatile;
	}

	for(POSITION i = 0; i < gNumberOfPositions; i++) {
		gGraphPrimitiveList[i] = undecided;
		gGraphNeighborList[i]  = NULL;
	}

	LoadGraphFromFile(gGraphFilename);

	gGoAgain = GraphGoAgain;
}

void FreeGame()
{
	if (gGraphPrimitiveList==NULL) {
		SafeFree(gGraphPrimitiveList);
	}
	if (gGraphNeighborList==NULL) {
		SafeFree(gGraphNeighborList);
	}

}

void LoadGraphFromFile(graphFilename)
STRING graphFilename;
{
	POSITIONLIST *head = NULL;
	FILE *fp;
	POSITION nodeNumber, nodeChild, i, numberNodes;
	int j, numChildren, largestNodeSoFar = -1;
	char theValueChar;
	char goAgainChar;

	/*** See if the file can even be read ***/

	if((fp = fopen(graphFilename, "r")) == NULL) {
		printf("LoadGraphFromFile Error: couldn't open %s for reading\n", graphFilename);
		ExitStageRight();
	}

	/*** See if we haven't requested too many nodes ***/

	if (fscanf(fp, "%llu", &numberNodes) == EOF) {
		printf("LoadGraphFromFile Error: couldn't read number of nodes from %s\n", graphFilename);
		ExitStageRight();
	}

	if (numberNodes > gNumberOfPositions) {
		printf("LoadGraphFromFile Error: %s had more graph nodes than I've allocated!\n", graphFilename);
		ExitStageRight();
	}

	/*** for every node in the file ***/

	for (i = 0; i < numberNodes; i++) {
		if (fscanf(fp, POSITION_FORMAT " %c %d", &nodeNumber, &theValueChar, &numChildren) == EOF) {
			printf("LoadGraphFromFile Error: couldn't read node %llu from %s\n", i, graphFilename);
			ExitStageRight();
		}


		/*** See if the node number will be outside allocated array bounds  ***/

		if (nodeNumber >= gNumberOfPositions) {
			printf("LoadGraphFromFile Error: %s had a node with index " POSITION_FORMAT " which was larger than your reserved array of size " POSITION_FORMAT "\n", graphFilename, nodeNumber, gNumberOfPositions);
			ExitStageRight();
		}

		/*** Find the largest node number ***/

		if ((int)nodeNumber > largestNodeSoFar)
			largestNodeSoFar = nodeNumber;

		/*** Store the value from its text character ***/

		gGraphPrimitiveList[nodeNumber] = TextToValue(theValueChar);

		/*** Store its children ***/

		head = NULL;
		for(j = 0; j < numChildren; j++) {
			if (fscanf(fp, POSITION_FORMAT, &nodeChild) == EOF) {
				printf("LoadGraphFromFile Error: couldn't read child %d from node %llu in %s\n", j, i, graphFilename);
				ExitStageRight();
			}

			goAgainChar = getc(fp);
			if (goAgainChar == 'g' || goAgainChar == 'G') {
				nodeChild += gNumberOfPositions;
			}
			else {
				ungetc(goAgainChar, fp);
			}

			head = StorePositionInList(nodeChild, head);
		}
		gGraphNeighborList[nodeNumber] = head;
	}

	/*** Keep track of the largest node number so far ***/

	gLargestNodeNumber = largestNodeSoFar;

	fclose(fp);
}

BOOLEAN GraphGoAgain(POSITION position, MOVE move) {
	(void)position;
	return (POSITION)move >= gNumberOfPositions;
}


VALUE TextToValue(char c) {
	if (c == 'u' || c == 'U')
		return(undecided);
	else if (c == 'l' || c == 'L')
		return(lose);
	else if (c == 'w' || c == 'W')
		return(win);
	else if (c == 't' || c == 'T')
		return(tie);
	else
		BadElse("TextToValue");
	return undecided; //should never be reached
}

/************************************************************************
**
** NAME:        DebugMenu
**
** DESCRIPTION: Menu used to debub internal problems. Does nothing if
**              kDebugMenu == FALSE
**
************************************************************************/

void DebugMenu()
{
	POSITION i;

	for (i = 0; i <= gLargestNodeNumber; i++) {
		if(GetValueOfPosition(i) != undecided)
			printf("position[" POSITION_FORMAT "] = R(%2d) = %s\n", i,
			       (int)Remoteness((REMOTENESS)i), gValueString[GetValueOfPosition(i)]);
	}
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

void GameSpecificMenu() {
	char tmp[MAXINPUTLENGTH - 20];
	FILE* fp;

	gGraphFilenameSet = FALSE;

	while (!gGraphFilenameSet) {
		printf("\nSpecify one of the files in the ../meta directory");
		printf("\n(But don't add the .grf at the end)\n\n");
		system("ls ../meta");
		printf("\nLoad Graph from : ");
		scanf("%s", tmp);
		(void) sprintf(gGraphFilename, "../meta/%s.grf", tmp);

		if((fp = fopen(gGraphFilename, "r")) != NULL) {
			gGraphFilenameSet = TRUE;
			if (kDBNameVolatile) SafeFree(kDBNameVolatile);
			kDBNameVolatile = (STRING) SafeMalloc(sizeof(char)*MAXINPUTLENGTH);
			sprintf(kDBNameVolatile, "graph-%s", tmp);
			kDBName = kDBNameVolatile;
		}
		else {
			printf("Invalid file name\n");
		}
	}

	FreeGame();
	InitializeGame();
}

/************************************************************************
**
** NAME:        SetTclCGameSpecificOptions
**
** DESCRIPTION: Set the C game-specific options (called from Tcl)
**              Ignore if you don't care about Tcl for now.
**
************************************************************************/

void SetTclCGameSpecificOptions(int theOptions[])
{
	/* No need to have anything here, we have no extra options */
	(void)theOptions;
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

POSITION DoMove(POSITION thePosition, MOVE theMove)
{
	/* the move IS the position */
	(void)thePosition;
	return(theMove%gNumberOfPositions);
}

/************************************************************************
**
** NAME:        GetInitialPosition
**
** DESCRIPTION: Ask the user for an initial position for testing. Store
**              it in the space pointed to by initialPosition;
**
** INPUTS:      POSITION initialPosition : The position to fill.
**
************************************************************************/

POSITION GetInitialPosition()
{
	POSITION initialPosition;
	printf("Please input the starting node number   : ");
	scanf(POSITION_FORMAT, &initialPosition);
	return initialPosition;
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

void PrintComputersMove(computersMove,computersName)
MOVE computersMove;
STRING computersName;
{
	printf("%8s's move              : %2d\n", computersName, computersMove);
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

VALUE Primitive(position)
POSITION position;
{
	VALUE theValue;
	if((theValue = gGraphPrimitiveList[position]) != undecided)
		if (theValue == lose)
			return(gStandardGame ? lose : win);
		else if (theValue == tie)
			return(tie);
		else if (theValue == win)
			return(gStandardGame ? win : lose);
		else
			BadElse("Primitive");
	else
		return(undecided);
	//should never reach here
	return undecided;
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

void PrintPosition(position,playerName,usersTurn)
POSITION position;
STRING playerName;
BOOLEAN usersTurn;
{
	printf("\nTOTAL                        : " POSITION_FORMAT " %s \n\n",
	       position, GetPrediction(position,playerName,usersTurn));
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
** CALLS:       MOVELIST *CopyMovelist(MOVELIST *)
**
************************************************************************/

MOVELIST *GenerateMoves(position)
POSITION position;
{
	POSITIONLIST* temp;
	MOVELIST* result;

	result = NULL;
	temp = gGraphNeighborList[position];

	while(temp!=NULL) {
		/* this is potentially dangerous if gNumberOfPositions exceeds the maximum value for type MOVE */
		result = CreateMovelistNode((MOVE) temp->position, result);
		temp = temp->next;
	}
	return(result);
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
** CALLS:       BOOLEAN PrintPossibleMoves(POSITION) ...Always True!
**
************************************************************************/

USERINPUT GetAndPrintPlayersMove(thePosition, theMove, playerName)
POSITION thePosition;
MOVE *theMove;
STRING playerName;
{
	USERINPUT ret, HandleDefaultTextInput();

	do {
		printf("%8s's move [(u)ndo/num] :  ", playerName);

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
** DESCRIPTION: Return TRUE iff the string input is of the right 'form'.
**              For example, if the user is allowed to select one slot
**              from the numbers 1-9, and the user chooses 0, it's not
**              valid, but anything from 1-9 IS, regardless if the slot
**              is filled or not. Whether the slot is filled is left up
**              to another routine.
**
** INPUTS:      STRING input : The string input the user typed.
**
** OUTPUTS:     BOOLEAN : TRUE iff the input is a valid text input.
**
************************************************************************/

BOOLEAN ValidTextInput(input)
STRING input;
{
	int i;

	return(input[0] >= (int)'0' &&
	       input[0] <= (int)'9' &&
	       sscanf(input, "%d", &i));
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

MOVE ConvertTextInputToMove(input)
STRING input;
{
	int ans;
	char goAgainChar;

	if (sscanf(input, "%d%c", &ans, &goAgainChar) != EOF) {
		if (goAgainChar=='g' || goAgainChar=='G') {
			ans += gNumberOfPositions;
		}
	} else {
		sscanf(input, "%d", &ans);
	}

	return(ans);
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

void PrintMove(theMove)
MOVE theMove;
{
	if ((POSITION)theMove >= gNumberOfPositions) {
		printf(POSITION_FORMAT "g", (POSITION)theMove%gNumberOfPositions);
	} else {
		printf("%d", theMove);
	}
}

int NumberOfOptions()
{
	return 2;
}

int getOption()
{
	if(gStandardGame) return 1;
	return 2;
}

void setOption(int option)
{
	if(option == 1)
		gStandardGame = TRUE;
	else
		gStandardGame = FALSE;
}


POSITION InteractStringToPosition(STRING board) {
	// FIXME: this is just a stub
	return atoi(board);
}

STRING InteractPositionToString(POSITION pos) {
	// FIXME: this is just a stub
	(void)pos;
	return "Implement Me";
}

STRING MoveToString(MOVE theMove) {
	(void)theMove;
	return "Implement MoveToString";
}

STRING InteractMoveToString(POSITION pos, MOVE mv) {
	(void)pos;
	return MoveToString(mv);
}
