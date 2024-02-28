/************************************************************************
**
** NAME:        mqx.c
**
** DESCRIPTION: Quick Cross
**
** AUTHOR:      Dan Garcia  -  University of California at Berkeley
**              Copyright (C) Dan Garcia, 1995. All rights reserved.
**              Thomas Yiu - mttt.c to mqx.c conversion
**
** DATE:        10/01
**
**************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/

#include "gamesman.h"

POSITION gNumberOfPositions  = 2 * 43046721;  // changed later if board size changes

POSITION gInitialPosition    =  0;
POSITION gMinimalPosition    =  0;
POSITION kBadPosition        = -1;

CONST_STRING kAuthorName         = "Dan Garcia";
CONST_STRING kGameName           = "Quick Cross";
CONST_STRING kDBName = "quickcross";
BOOLEAN kPartizan           = FALSE;
BOOLEAN kDebugMenu          = TRUE;
BOOLEAN kGameSpecificMenu   = TRUE;
BOOLEAN kTieIsPossible      = FALSE;
BOOLEAN kLoopy               = TRUE;
BOOLEAN kDebugDetermineValue = FALSE;
void*    gGameSpecificTclInit = NULL;

CONST_STRING kHelpGraphicInterface =
        "GUI not available at the moment.";

CONST_STRING kHelpTextInterface    =
        "On your turn, use the LEGEND to determine your desired action (place\n\
horizonally (-), place vertically (|), or switch (x)) and board position\n\
number. If at any point you have made a mistake, you can type u and hit\n\
return and the system will revert back to your most recent position."                                                                                                                                                                                                                                        ;

CONST_STRING kHelpOnYourTurn =
        "You place one of the pieces on one of the empty board positions either\n\
horizontally or vertically, or you switch its orientation."                                                                                   ;

CONST_STRING kHelpStandardObjective =
        "To get three or four pieces in a row, depending on the game mode, either\n\
horizontally, vertically, or diagonally. 3/4-in-a-row WINS."                                                                                     ;

CONST_STRING kHelpReverseObjective =
        "To force your opponent into getting three or four pieces in a row,\n\
depending on the game mode, either horizontally, vertically, or diagonally.\n\
3/4-in-a-row LOSES."                                                                                                                                                              ;

CONST_STRING kHelpTieOccursWhen =   /* Should follow 'A Tie occurs when... */
                            "an infinite loop occurs and no player can be forced into a winning or\n\
losing position."                                                                                                      ;

CONST_STRING kHelpExample =
        "         (  1  2  3  4 )           : o o o o\n\
LEGEND:  (  5  6  7  8 )  TOTAL:   : o o o o\n\
         (  9 10 11 12 )           : o o o o \n\n\
Computer's move              : -10\n\n\
         (  1  2  3  4 )           : o o o o\n\
LEGEND:  (  5  6  7  8 )  TOTAL:   : o o o o\n\
         (  9 10 11 12 )           : o - o o\n\n\
     Dan's move [(u)ndo/(-|x)(1-12)] :  |7\n\
         (  1  2  3  4 )           : o o o o\n\
LEGEND:  (  5  6  7  8 )  TOTAL:   : o o | o\n\
         (  9 10 11 12 )           : o - o o\n\n\
Computer's move              : -1\n\n\
         (  1  2  3  4 )           : - o o o\n\
LEGEND:  (  5  6  7  8 )  TOTAL:   : o o | o\n\
         (  9 10 11 12 )           : o - o o\n\n\
     Dan's move [(u)ndo/(-|x)(1-12)] :  x1\n\n\
         (  1  2  3  4 )           : | o o o\n\
LEGEND:  (  5  6  7  8 )  TOTAL:   : o o | o\n\
         (  9 10 11 12 )           : o - o o \n\
Computer's move              : -5\n\n\
         (  1  2  3  4 )           : | o o o\n\
LEGEND:  (  5  6  7  8 )  TOTAL:   : - o | o\n\
         (  9 10 11 12 )           : o - o o\n\n\
     Dan's move [(u)ndo/(-|x)(1-12)] :  |6\n\n\
         (  1  2  3  4 )           : | o o o\n\
LEGEND:  (  5  6  7  8 )  TOTAL:   : - | | o\n\
         (  9 10 11 12 )           : o - o o\n\n\
Computer's move              : |8\n\n\
         (  1  2  3  4 )           : | o o o\n\
LEGEND:  (  5  6  7  8 )  TOTAL:   : - | | |\n\
         (  9 10 11 12 )           : o - o o\n\n\
Computer wins. Nice try, Dan."                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    ;


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
#define POSITION_OFFSET 43046721 /* 3^16 used like in machi.c */

typedef enum possibleBoards {
	b4x4, b3x4, b3x3, b15_3, b15_4
} Boards;

Boards BOARD = b4x4;
int BOARDSIZE = 16;

typedef enum possibleBoardPieces {
	Blank, H, V
} BlankHV;

char *gBlankHVString[] = { "o", "-", "|" };

/* Powers of 3 - this is the way I encode the position, as an integer */
int g3Array[] =          { 1, 3, 9, 27, 81, 243, 729, 2187, 6561, 19683,
	                   59049, 177147, 531441, 1594323, 4782969,
	                   14348907, 43046721};

/** Function Prototypes **/
void PositionToBlankHV(POSITION thePos, BlankHV *theBlankHV);
BOOLEAN AllFilledIn(BlankHV *theBlankHV);
BOOLEAN FourInARow(BlankHV *theBlankHV, int a, int b, int c, int d);
BOOLEAN ThreeInARow(BlankHV *theBlankHV, int a, int b, int c);
POSITION BlankHVToPosition(BlankHV *theBlankHV);
POSITION                ActualNumberOfPositions(int variant);

/*************************************************************************
**
** Here we declare the global database variables
**
**************************************************************************/

void InitializeGame()
{
	gActualNumberOfPositionsOptFunPtr = &ActualNumberOfPositions;
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

void GameSpecificMenu()
{
	char inp;
	while (TRUE) {
		//inp = getchar(); // get rid of the 'g' from previous menu
		printf("\n\n\n");
		printf("        ----- Game-specific options for Quick Cross -----\n\n");
		printf("        Select a game board:\n\n");
		printf("        1)          3 X 3  Board\n");
		printf("        2)          3 X 4  Board\n");
		printf("        3)      15-square  Board  --  3 in a row\n");
		printf("        4)      15-square  Board  --  4 in a row\n");
		printf("        5)          4 X 4  Board\n\n");
		printf("        b)      (B)ack = Return to previous activity.\n\n\n");
		printf("Select an option: ");
		inp = getchar();
		if (inp == '1') {
			BOARD = b3x3;
			BOARDSIZE = 9;
			gNumberOfPositions = 19683; /*  3^9  */
		}
		else if (inp == '2') {
			BOARD = b3x4;
			BOARDSIZE = 12;
			gNumberOfPositions = 531441; /*  3^12  */
		}
		else if (inp == '3') {
			BOARD = b15_3; // basically a 4 X 4 with a corner square removed
			BOARDSIZE = 15;
			gNumberOfPositions = 14348907; /*  3^15  */
		}
		else if (inp == '4') {
			BOARD = b15_4;
			BOARDSIZE = 15;
			gNumberOfPositions = 14348907; /*  3^15  */
		}
		else if (inp == '5') {
			BOARD = b4x4;
			BOARDSIZE = 16;
			gNumberOfPositions = 43046721; /*  3^16 * turn */
		}
		else if (inp == 'b' || inp == 'B')
			;
		else {
			printf("Invalid input.\n");
			continue;
		}
		break;
	}
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
** CALLS:       PositionToBlankHV(POSITION,*BlankHV)
**
************************************************************************/

POSITION DoMove(POSITION thePosition, MOVE theMove) {
	BlankHV theBlankHV[BOARDSIZE];
	int moveModifier = 0;

	PositionToBlankHV(thePosition,theBlankHV);

	if(0 <= theMove && theMove < BOARDSIZE)
		moveModifier = g3Array[theMove] * (int)H;

	else if(BOARDSIZE <= theMove && theMove < 2 * BOARDSIZE)
		moveModifier = g3Array[theMove - BOARDSIZE] * (int)V;

	else if(theBlankHV[theMove - 2 * BOARDSIZE] == H)
		moveModifier = g3Array[theMove - 2 * BOARDSIZE] * ((int)V - (int)H);

	else if(theBlankHV[theMove - 2 * BOARDSIZE] == V)
		moveModifier = g3Array[theMove - 2 * BOARDSIZE] * ((int)H - (int)V);

	else {
		BadElse("DoMove");
		return(thePosition);
	}

	return thePosition + moveModifier;
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

POSITION GetInitialPosition()
{
	BlankHV theBlankHV[BOARDSIZE]; //, whosTurn;
	signed char c;
	int i;


	printf("\n\n\t----- Get Initial Position -----\n");
	printf("\n\tPlease input the position to begin with.\n");
	printf("\tNote that it should be in the following format:\n\n");
	if (BOARD == b3x3)
		printf("o - |\n| o o            <----- EXAMPLE \no o -\n\n");
	else if (BOARD == b15_3 || BOARD == b15_4)
		printf("o - - |\no - | |            <----- EXAMPLE \n- o o o\n| | o  \n\n");
	else if (BOARD == b3x4)
		printf("o - - |\no - | |            <----- EXAMPLE \n- o | o\n\n");
	else if (BOARD == b4x4)
		printf("o - - |\no - | |            <----- EXAMPLE \n- o o o\n| | o -\n\n");

	i = 0;
	getchar();
	while(i < BOARDSIZE && (c = getchar()) != EOF) {
		if(c == '-' || c == 'h' || c == 'H')
			theBlankHV[i++] = H;
		else if(c == '|' || c == 'v' || c == 'V' || c == '1' || c == 'l')
			theBlankHV[i++] = V;
		else if(c == 'o' || c == 'O' || c == '0')
			theBlankHV[i++] = Blank;
		/* else do nothing */
	}

	return(BlankHVToPosition(theBlankHV));
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
	int squareNum;
	char moveType;

	squareNum = computersMove % BOARDSIZE + 1;

	if(0 <= computersMove && computersMove < BOARDSIZE)
		moveType = '-';
	else if(BOARDSIZE <= computersMove && computersMove < 2 * BOARDSIZE)
		moveType = '|';
	else
		moveType = 'x';

	printf("%8s's move              : %c%d\n", computersName, moveType,
	       squareNum);
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
** CALLS:       BOOLEAN FourInARow()
**              BOOLEAN ThreeInARow()
**              BOOLEAN AllFilledIn()
**              PositionToBlankHV()
**
************************************************************************/

VALUE Primitive(POSITION position) {
	BlankHV theBlankHV[BOARDSIZE];

	PositionToBlankHV(position, theBlankHV);

	/*printf(" & & & & & & &  PRIMITIVE CALLED WITH position = %d",position);*/

	if (BOARD == b3x4) {
		if( ThreeInARow(theBlankHV,0,1,2) ||
		    ThreeInARow(theBlankHV,1,2,3) ||
		    ThreeInARow(theBlankHV,4,5,6) ||
		    ThreeInARow(theBlankHV,5,6,7) ||
		    ThreeInARow(theBlankHV,8,9,10) ||
		    ThreeInARow(theBlankHV,9,10,11) ||
		    ThreeInARow(theBlankHV,0,4,8) ||
		    ThreeInARow(theBlankHV,1,5,9) ||
		    ThreeInARow(theBlankHV,2,6,10) ||
		    ThreeInARow(theBlankHV,3,7,11) ||
		    ThreeInARow(theBlankHV,0,5,10) ||
		    ThreeInARow(theBlankHV,1,6,11) ||
		    ThreeInARow(theBlankHV,2,5,8) ||
		    ThreeInARow(theBlankHV,3,6,9) )
			return(gStandardGame ? lose : win);
		else
			return(undecided);
	}
	else if (BOARD == b4x4) {
		if( FourInARow(theBlankHV,0,1,2,3) ||
		    FourInARow(theBlankHV,4,5,6,7) ||
		    FourInARow(theBlankHV,8,9,10,11) ||
		    FourInARow(theBlankHV,12,13,14,15) ||
		    FourInARow(theBlankHV,0,4,8,12) ||
		    FourInARow(theBlankHV,1,5,9,13) ||
		    FourInARow(theBlankHV,2,6,10,14) ||
		    FourInARow(theBlankHV,3,7,11,15) ||
		    FourInARow(theBlankHV,0,5,10,15) ||
		    FourInARow(theBlankHV,3,6,9,12) )
			return(gStandardGame ? lose : win);
		else
			return(undecided);
	}
	else if (BOARD == b3x3) {
		if( ThreeInARow(theBlankHV,0,1,2) ||
		    ThreeInARow(theBlankHV,3,4,5) ||
		    ThreeInARow(theBlankHV,6,7,8) ||
		    ThreeInARow(theBlankHV,0,3,6) ||
		    ThreeInARow(theBlankHV,1,4,7) ||
		    ThreeInARow(theBlankHV,2,5,8) ||
		    ThreeInARow(theBlankHV,0,4,8) ||
		    ThreeInARow(theBlankHV,2,4,6) )
			return(gStandardGame ? lose : win);
		else
			return(undecided);
	}
	else if (BOARD == b15_3) {
		if( ThreeInARow(theBlankHV,0,1,2) ||
		    ThreeInARow(theBlankHV,1,2,3) ||
		    ThreeInARow(theBlankHV,4,5,6) ||
		    ThreeInARow(theBlankHV,5,6,7) ||
		    ThreeInARow(theBlankHV,8,9,10) ||
		    ThreeInARow(theBlankHV,9,10,11) ||
		    ThreeInARow(theBlankHV,12,13,14) ||
		    ThreeInARow(theBlankHV,0,4,8) ||
		    ThreeInARow(theBlankHV,4,8,12) ||
		    ThreeInARow(theBlankHV,1,5,9) ||
		    ThreeInARow(theBlankHV,5,9,13) ||
		    ThreeInARow(theBlankHV,2,6,10) ||
		    ThreeInARow(theBlankHV,6,10,14) ||
		    ThreeInARow(theBlankHV,3,7,11) ||
		    ThreeInARow(theBlankHV,2,5,8) ||
		    ThreeInARow(theBlankHV,3,6,9) ||
		    ThreeInARow(theBlankHV,6,9,12) ||
		    ThreeInARow(theBlankHV,7,10,13) ||
		    ThreeInARow(theBlankHV,4,9,14) ||
		    ThreeInARow(theBlankHV,0,5,10) ||
		    ThreeInARow(theBlankHV,1,6,11) )
			return(gStandardGame ? lose : win);
		else
			return(undecided);
	}
	else if (BOARD == b15_4) {
		if( FourInARow(theBlankHV,0,1,2,3) ||
		    FourInARow(theBlankHV,4,5,6,7) ||
		    FourInARow(theBlankHV,8,9,10,11) ||
		    FourInARow(theBlankHV,0,4,8,12) ||
		    FourInARow(theBlankHV,1,5,9,13) ||
		    FourInARow(theBlankHV,2,6,10,14) ||
		    FourInARow(theBlankHV,3,6,9,12) )
			return(gStandardGame ? lose : win);
		else
			return(undecided);
	}
	//never reaches here
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
** CALLS:       PositionToBlankHV()
**              GetValueOfPosition()
**              GetPrediction()
**
************************************************************************/

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn) {
	BlankHV theBlankHV[BOARDSIZE];

	PositionToBlankHV(position, theBlankHV); //unhash function

	if (BOARD == b3x4) {
		printf("\n         (  1  2  3  4 )           : %s %s %s %s\n",
		       gBlankHVString[(int)theBlankHV[0]],
		       gBlankHVString[(int)theBlankHV[1]],
		       gBlankHVString[(int)theBlankHV[2]],
		       gBlankHVString[(int)theBlankHV[3]] );
		printf("LEGEND:  (  5  6  7  8 )  TOTAL:   : %s %s %s %s\n",
		       gBlankHVString[(int)theBlankHV[4]],
		       gBlankHVString[(int)theBlankHV[5]],
		       gBlankHVString[(int)theBlankHV[6]],
		       gBlankHVString[(int)theBlankHV[7]] );
		printf("         (  9 10 11 12 )           : %s %s %s %s %s\n\n",
		       gBlankHVString[(int)theBlankHV[8]],
		       gBlankHVString[(int)theBlankHV[9]],
		       gBlankHVString[(int)theBlankHV[10]],
		       gBlankHVString[(int)theBlankHV[11]],
		       GetPrediction(position,playerName,usersTurn));
	}
	else if (BOARD == b4x4) {
		printf("\n         (  1  2  3  4 )           : %s %s %s %s\n",
		       gBlankHVString[(int)theBlankHV[0]],
		       gBlankHVString[(int)theBlankHV[1]],
		       gBlankHVString[(int)theBlankHV[2]],
		       gBlankHVString[(int)theBlankHV[3]] );
		printf("LEGEND:  (  5  6  7  8 )  TOTAL:   : %s %s %s %s\n",
		       gBlankHVString[(int)theBlankHV[4]],
		       gBlankHVString[(int)theBlankHV[5]],
		       gBlankHVString[(int)theBlankHV[6]],
		       gBlankHVString[(int)theBlankHV[7]] );
		printf("         (  9 10 11 12 )           : %s %s %s %s\n",
		       gBlankHVString[(int)theBlankHV[8]],
		       gBlankHVString[(int)theBlankHV[9]],
		       gBlankHVString[(int)theBlankHV[10]],
		       gBlankHVString[(int)theBlankHV[11]] );
		printf("         ( 13 14 15 16 )           : %s %s %s %s %s\n\n",
		       gBlankHVString[(int)theBlankHV[12]],
		       gBlankHVString[(int)theBlankHV[13]],
		       gBlankHVString[(int)theBlankHV[14]],
		       gBlankHVString[(int)theBlankHV[15]],
		       GetPrediction(position,playerName,usersTurn));
	}
	else if (BOARD == b3x3) {
		printf("\n         ( 1 2 3 )           : %s %s %s\n",
		       gBlankHVString[(int)theBlankHV[0]],
		       gBlankHVString[(int)theBlankHV[1]],
		       gBlankHVString[(int)theBlankHV[2]] );
		printf("LEGEND:  ( 4 5 6 )  TOTAL:   : %s %s %s\n",
		       gBlankHVString[(int)theBlankHV[3]],
		       gBlankHVString[(int)theBlankHV[4]],
		       gBlankHVString[(int)theBlankHV[5]] );
		printf("         ( 7 8 9 )           : %s %s %s %s\n\n",
		       gBlankHVString[(int)theBlankHV[6]],
		       gBlankHVString[(int)theBlankHV[7]],
		       gBlankHVString[(int)theBlankHV[8]],
		       GetPrediction(position,playerName,usersTurn));
	}
	else if (BOARD == b15_3 || BOARD == b15_4) {
		printf("\n         (  1  2  3  4 )           : %s %s %s %s\n",
		       gBlankHVString[(int)theBlankHV[0]],
		       gBlankHVString[(int)theBlankHV[1]],
		       gBlankHVString[(int)theBlankHV[2]],
		       gBlankHVString[(int)theBlankHV[3]] );
		printf("LEGEND:  (  5  6  7  8 )  TOTAL:   : %s %s %s %s\n",
		       gBlankHVString[(int)theBlankHV[4]],
		       gBlankHVString[(int)theBlankHV[5]],
		       gBlankHVString[(int)theBlankHV[6]],
		       gBlankHVString[(int)theBlankHV[7]] );
		printf("         (  9 10 11 12 )           : %s %s %s %s\n",
		       gBlankHVString[(int)theBlankHV[8]],
		       gBlankHVString[(int)theBlankHV[9]],
		       gBlankHVString[(int)theBlankHV[10]],
		       gBlankHVString[(int)theBlankHV[11]] );
		printf("         ( 13 14 15    )           : %s %s %s   %s\n\n",
		       gBlankHVString[(int)theBlankHV[12]],
		       gBlankHVString[(int)theBlankHV[13]],
		       gBlankHVString[(int)theBlankHV[14]],
		       GetPrediction(position,playerName,usersTurn));
	}
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
	BlankHV theBlankHV[BOARDSIZE];
	int i;

	if (Primitive(position) == undecided) {
		PositionToBlankHV(position,theBlankHV);
		for(i = 0; i < BOARDSIZE; i++) {
			if(theBlankHV[i] == Blank) {
				head = CreateMovelistNode(i,head);
				head = CreateMovelistNode(i+BOARDSIZE,head);
			} else
				head = CreateMovelistNode(i+2*BOARDSIZE,head);
		}
		return(head);
	} else {
		return(NULL);
	}
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
		printf("%8s's move [(u)ndo/(-|x)(1-%d)] :  ", playerName, BOARDSIZE);

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

BOOLEAN ValidTextInput(STRING input) {
	BOOLEAN valid;

	valid = ((input[0] == '-' || input[0] == '|' || input[0] == 'x' ||
	          input[0] == '1' || input[0] == 'l' || input[0] == 'X') &&
	         (input[1] >= '1' && input[1] <= '9' ));

	if(strlen(input) == 3)
		valid = input[1] == '1' && input[2] >= '0' && input[2] <= ('0'+BOARDSIZE-10);

	return valid;
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
	MOVE theMove;
	int squareNum;

	if (strlen(input) == 2)
		squareNum = input[1] - '1';
	else if (strlen(input) == 3)
		squareNum = input[2] - '1' + 10;

	if (input[0] == '-')
		theMove = (MOVE)squareNum;
	else if(input[0] == '|' || input[0] == '1' || input[0] == 'l')
		theMove = (MOVE) squareNum + BOARDSIZE;
	else if(input[0] == 'x' || input[0] == 'X')
		theMove = (MOVE) squareNum + 2 * BOARDSIZE;
	else
		theMove = -1;

	return theMove;
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

void MoveToString (MOVE theMove, char *moveStringBuffer) {
	int squareNum;
	char moveType;

	squareNum = theMove % BOARDSIZE + 1;

	if(0 <= theMove && theMove < BOARDSIZE)
		moveType = '-';
	else if(BOARDSIZE <= theMove && theMove < 2 * BOARDSIZE)
		moveType = '|';
	else
		moveType = 'x';
	sprintf(moveStringBuffer, "%c%d", moveType, squareNum);
}

/************************************************************************
*************************************************************************
**         EVERYTHING BELOW THESE LINES IS LOCAL TO THIS FILE
*************************************************************************
************************************************************************/

/************************************************************************
**
** NAME:        PositionToBlankHV
**
** DESCRIPTION: convert an internal position to that of a BlankHV.
**
** INPUTS:      POSITION thePos     : The position input.
**              BlankHV *theBlankHV : The converted BlankHV output array.
**
** CALLS:       BadElse()
**
************************************************************************/

void PositionToBlankHV(POSITION thePos, BlankHV *theBlankHV) {
	int i;
	for(i = BOARDSIZE - 1; i >= 0; i--) {
		if(thePos >= (POSITION)(V * g3Array[i])) {
			theBlankHV[i] = V;
			thePos -= V * g3Array[i];
		}
		else if(thePos >= (POSITION)(H * g3Array[i])) {
			theBlankHV[i] = H;
			thePos -= H * g3Array[i];
		}
		else if(thePos >= (POSITION)(Blank * g3Array[i])) {
			theBlankHV[i] = Blank;
			thePos -= Blank * g3Array[i];
		}
		else
			BadElse("PositionToBlankHV");
	}
}

/************************************************************************
**
** NAME:        BlankHVToPosition
**
** DESCRIPTION: convert a BlankHV to that of an internal position.
**
** INPUTS:      BlankHV *theBlankHV : The converted BlankHV output array.
**
** OUTPUTS:     POSITION: The equivalent position given the BlankHV.
**
************************************************************************/

POSITION BlankHVToPosition(BlankHV *theBlankHV) {
	int i;
	POSITION position = 0;

	for(i = 0; i < BOARDSIZE; i++)
		position += g3Array[i] * (int)theBlankHV[i]; /* was (int)position... */

	return(position);
}


/************************************************************************
**
** NAME:        ThreeInARow
**
** DESCRIPTION: Return TRUE iff there are three-in-a-row.
**
** INPUTS:      BlankHV theBlankHV[BOARDSIZE] : The BlankHV array.
**              int a,b,c                     : The 3 positions to check.
**
** OUTPUTS:     (BOOLEAN) TRUE iff there are three-in-a-row.
**
************************************************************************/

BOOLEAN ThreeInARow(BlankHV *theBlankHV, int a, int b, int c) {
	return(theBlankHV[a] == theBlankHV[b] &&
	       theBlankHV[b] == theBlankHV[c] &&
	       theBlankHV[c] != Blank );
}


/************************************************************************
**
** NAME:        FourInARow
**
** DESCRIPTION: Return TRUE iff there are four-in-a-row.
**
** INPUTS:      BlankHV theBlankHV[BOARDSIZE] : The BlankHV array.
**              int a,b,c,d                   : The 4 positions to check.
**
** OUTPUTS:     (BOOLEAN) TRUE iff there are four-in-a-row.
**
************************************************************************/

BOOLEAN FourInARow(BlankHV *theBlankHV, int a, int b, int c, int d) {
	return(theBlankHV[a] == theBlankHV[b] &&
	       theBlankHV[b] == theBlankHV[c] &&
	       theBlankHV[c] == theBlankHV[d] &&
	       theBlankHV[d] != Blank );
}



/************************************************************************
**
** NAME:        AllFilledIn
**
** DESCRIPTION: Return TRUE iff all the blanks are filled in.
**
** INPUTS:      BlankHV theBlankHV[BOARDSIZE] : The BlankHV array.
**
** OUTPUTS:     (BOOLEAN) TRUE iff all the blanks are filled in.
**
************************************************************************/

BOOLEAN AllFilledIn(BlankHV *theBlankHV) {
	BOOLEAN answer = TRUE;
	int i;

	for(i = 0; i < BOARDSIZE; i++)
		answer &= (theBlankHV[i] == H || theBlankHV[i] == V);

	return(answer);
}

int NumberOfOptions()
{
	int numBoardsizes = 5; /* b4x4, b3x4, b3x3, b15_3, b15_4 */
	return 2*numBoardsizes;
}

int getOption()
{
	int boardsizeOption;
	int option = 1;
	if(!gStandardGame) option += 1;

	switch(BOARD) {
	case b3x4: boardsizeOption=0; break;
	case b3x3: boardsizeOption=1; break;
	case b15_3: boardsizeOption=2; break;
	case b15_4: boardsizeOption=3; break;
	case b4x4: boardsizeOption=4; break;
	}
	option += 2*boardsizeOption;

	return option;
}

void setOption(int option)
{
	int boardsizeOption;
	option -= 1;

	if (option%2==0) {
		gStandardGame = TRUE;
	} else {
		gStandardGame = FALSE;
	}

	boardsizeOption = option/2;
	switch(boardsizeOption) {
	case 0: BOARD=b3x4; BOARDSIZE=12; break;
	case 1: BOARD=b3x3; BOARDSIZE=9; break;
	case 2: BOARD=b15_3; BOARDSIZE=15; break;
	case 3: BOARD=b15_4; BOARDSIZE=15; break;
	case 4: BOARD=b4x4; BOARDSIZE=16; break;
	}
}

POSITION ActualNumberOfPositions(int variant) {
	switch(variant) {
	case 1:
	case 2: return 453831; break;
	case 3:
	case 4: return 18753; break;
	case 5:
	case 6: return gNumberOfPositions; break;
	case 7:
	case 8: return gNumberOfPositions; break;
	case 9:
	case 10: return gNumberOfPositions; break;
	default: return gNumberOfPositions; break;
	}
}

POSITION StringToPosition(char *positionString) {
	int turn;
	char *board;
	if (ParseStandardOnelinePositionString(positionString, &turn, &board)) {
		BlankHV enumBoard[BOARDSIZE];
		for (int i = 0; i < BOARDSIZE; i++) {
			switch (board[i]) {
				case '-':
					enumBoard[i] = Blank;
					break;
				case 'h':
					enumBoard[i] = H;
					break;
				case 'v':
					enumBoard[i] = V;
					break;
				default:
					return NULL_POSITION;
					break;
			}
		}
		return BlankHVToPosition(enumBoard);
	}
	return NULL_POSITION;
}

void PositionToAutoGUIString(POSITION position, char *autoguiPositionStringBuffer) {
	//takes in a position hash 'pos'. pos is an integer
	BlankHV enumBoard[BOARDSIZE]; //creating a board array of enums (Blank, H, V)

	PositionToBlankHV(position, enumBoard);
	char board[BOARDSIZE + 1]; 
	for (int i = 0; i < BOARDSIZE; i++) {
		switch (enumBoard[i]) {
			case H:
				board[i] = 'h';
				break;
			case V:
				board[i] = 'v';
				break;
			default:
				board[i] = '-';
				break;
		}
	}
	board[BOARDSIZE] = '\0'; // Make sure to null-terminate your board.
	AutoGUIMakePositionString(0, board, autoguiPositionStringBuffer);
}

void MoveToAutoGUIString(POSITION position, MOVE move, char *autoguiMoveStringBuffer) {
	(void)position;
	int squareNum;
	if (0 <= move && move < BOARDSIZE){
		squareNum = move % BOARDSIZE;
		int left = squareNum + 16;
		int right = squareNum + 32;
		AutoGUIMakeMoveButtonStringL(left, right, 'y', autoguiMoveStringBuffer);
	} else if (BOARDSIZE <= move && move < 2 * BOARDSIZE) {
		squareNum = move % BOARDSIZE;
		int top = squareNum + 32 + 16;
		int bottom = squareNum + 32 + 32;
		AutoGUIMakeMoveButtonStringL(top, bottom, 'y', autoguiMoveStringBuffer);
	} else if (2 * BOARDSIZE <= move && move < 3 * BOARDSIZE) {
		squareNum = move % BOARDSIZE;
		AutoGUIMakeMoveButtonStringA('r', squareNum, 'x', autoguiMoveStringBuffer);
	}
}
