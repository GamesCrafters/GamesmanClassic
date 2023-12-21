/************************************************************************
**
** NAME:        m3spot.c
**
** DESCRIPTION: 3Dot
**
** AUTHOR:      Attila Gyulassy, Cameron Cheung
**
** DATE:        02/02/02
**
**************************************************************************/

/*************************************************************************
** FOR REFERENCE THIS IS THE BOARD, POSITIONS, INITIAL POSITION
** +-+-+-+    +-+-+-+    +-+-+-+
** | | | |    |1|2|3|    | | | |
** +-+-+-+    +-+-+-+    +-+-+-+
** | | | |    |4|5|6|    |R|W|B|  ==  1 0000 0000 100 1 101 1 110 1
** +-+-+-+    +-+-+-+    +-+-+-+      t ptsA ptsB -RED- -WHT- -BLU-
** |o|o|o|    |7|8|9|    |R|W|B|  ==  0x1009BD
** +-+-+-+    +-+-+-+    +-+-+-+
**                                    if t = 1, it is player 1's turn
** +-+-+-+
** |X| | |
** +-+-+-+
** |X| | | == 001 1
** +-+-+-+       +-+ = oriented vertically
** |o|o|o|            !! put a 0 here to orient board 90 degrees clockwise
** +-+-+-+
**
** +-+-+-+
** | |X| |
** +-+-+-+
** | |X| | == 010
** +-+-+-+
** |o|o|o|
** +-+-+-+
**
** +-+-+-+
** | | |X|
** +-+-+-+
** | | |X| == 011
** +-+-+-+
** |o|o|o|
** +-+-+-+
**
** +-+-+-+
** | | | |
** +-+-+-+
** |X| | | == 100
** +-+-+-+
** |X|o|o|
** +-+-+-+
**
** +-+-+-+
** | | | |
** +-+-+-+
** | |X| | == 101
** +-+-+-+
** |o|X|o|
** +-+-+-+
**
** +-+-+-+
** | | | |
** +-+-+-+
** | | |X| == 110
** +-+-+-+
** |o|o|X|
** +-+-+-+
**
** MOVES ARE REPRESENTED AS FOLLOWS
** first is the person's piece then the white piece:
**   4 bits for first, 4 bits for second new position.
**
*************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/

#include <stdio.h>
#include "gamesman.h"

POSITION gNumberOfPositions  = 2097152;  /* 2^21 */

POSITION gInitialPosition    =  0x001009BD;
POSITION gMinimalPosition = 0x001009BD;
POSITION kBadPosition = -1;

CONST_STRING kAuthorName         = "Attila Gyulassy, Matthew Yu, Cameron Cheung";
CONST_STRING kGameName           = "3Spot";

//?
BOOLEAN kPartizan           = TRUE;
BOOLEAN kDebugMenu          = FALSE;
BOOLEAN kGameSpecificMenu   = TRUE;
BOOLEAN kTieIsPossible      = FALSE;
BOOLEAN kLoopy              = TRUE;

//?
BOOLEAN kDebugDetermineValue = FALSE;
CONST_STRING kHelpGraphicInterface = "";

void*    gGameSpecificTclInit = NULL;

CONST_STRING kHelpTextInterface    =
        "Umm, i'm not sure what you want to hear here, so\n\
i'm not going to tell you anything... =)\n"                                                             ;

CONST_STRING kHelpOnYourTurn =
        "It is your turn: you need to input a valid move:\n\
VALID MOVES: a valid move is a pair of a pair of a \n\
number(1-8) and a letter indicating the orientation\n\
of the piece. The number represents the \"head\" of \n\
your piece.\n\n\
EXAMPLE: 1v 2v\n\
This will first move your piece to 1v, and then the \n\
white piece to 2v, so that the board will look like:\n\n\
BEFORE: ..B    AFTER: RWB\n\
        RWB           RWB\n\
        RWo           ooo\n\n\
You must change the position of your piece, and the\n\
the white piece. You earn points by covering up the\n\
squares with \'o\'s.\n "                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               ;

CONST_STRING kHelpStandardObjective ="PlayerX wins if:\n\
PlayerX >= PlayerX's winning score AND\n\
PlayerY >= PlayerY's minimum score. \n\n\
PlayerX loses if:\n\
PlayerX >= PlayerX's winning score AND\n\
PlayerY < PlayerY's minimum score.\n";

CONST_STRING kHelpReverseObjective ="";
CONST_STRING kHelpTieOccursWhen ="";
CONST_STRING kHelpExample = "";
CONST_STRING kDBName = "3spot";

STRING MoveToString(MOVE);

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

#define BOARDSIZE     9           /* 3x3 board */
//#define NUMSYMMETRIES 2           /* symmetry down middle but not implemented */

#define MIN_SCORE 1     /* Lowest valid winning score */
#define MAX_SCORE 15    /* Highest valid winning score */

int winScore1 = 12;
int winScore2 = 6;
//normal game has win for player 1 when p1 >= win1 & p2>=win2
//inverted game has win for player1 when p1>=win1 & p2<win2

int invertedScoring = 0;
int noMoveWhite = 0;

char moveToTextInt[] = {'7','1','4','2','1','3','8','4','5','5','2','6'};
char moveToTextOri[] = {'h','v','h','v','h','v','h','v','h','v','h','v'};
int pointsEarned[] =   { 2, 0, 0, 0, 0, 0, 2, 1, 0, 1, 0, 1 };
int posToBoard[] =     { 7, 1, 4, 2, 1, 3, 8, 4, 5, 5, 2, 6 };
int boardToPosition[] = {6,12,0,4,10,0,2,8,0};

char theBoard[] = {' ',' ',' ',' ',' ',' ','o','o','o'};
char theInitialBoard[] =   {' ',' ',' ',' ',' ',' ','o','o','o'};

/** Function Prototypes **/
int boardToPos(int pos, char orientation);
int ctoi (char c);
void SetTargetScoresMenu();
void SetDotPositionMenu();
void ClearTheBoard();
int pieceToBoard(int pos, int piecePart);
MULTIPARTEDGELIST* GenerateMultipartMoveEdges(POSITION position, MOVELIST *moveList, POSITIONLIST *positionList);

/*********************************************
** get the red piece's position on the board *
*********************************************/
int GetRedPosition(POSITION thePosition) {
	int temp = thePosition;
	temp = temp & 0x00000f00;
	temp = temp >> 8;
	return temp;
}

/*********************************************
** set a new position for the red piece     **
*********************************************/
int SetRedPosition(POSITION thePosition, int place) {
	thePosition = thePosition & 0xfffff0ff;
	thePosition = thePosition | (place << 8);
	return thePosition;
}

/*********************************************
** get the wht piece's position on the board *
*********************************************/
int GetWhitePosition(POSITION thePosition) {
	int temp = thePosition;
	temp = temp & 0x000000f0;
	temp = temp >> 4;
	return temp;
}

/*********************************************
** set a new position for the wht piece     **
*********************************************/
int SetWhitePosition(POSITION thePosition, int place) {
	thePosition = thePosition & 0xffffff0f;
	thePosition = thePosition | (place << 4);
	return thePosition;
}

/*********************************************
** get the blu piece's position on the board *
*********************************************/
int GetBluePosition(POSITION thePosition) {
	int temp = thePosition;
	temp = temp & 0x0000000F;
	return temp;
}

/*********************************************
** set a new position for the wht piece     **
*********************************************/
int SetBluePosition(POSITION thePosition, int place) {
	thePosition = thePosition & 0xfffffff0;
	thePosition = thePosition | place;
	return thePosition;
}

/*
 * get the score of player 1
 */
int GetPlayer1Score(POSITION thePosition) {
	int temp = 0;
	temp = thePosition  & 0x000F0000;
	temp = temp >> 16;
	return temp;
}

/*
 * get the score of player 2
 */
int GetPlayer2Score(POSITION thePosition) {
	int temp = 0;
	temp = thePosition  & 0x0000F000;
	temp = temp >> 12;
	return temp;
}

/*
 * get whose turn it is, 1 for player1 , 0 for p2
 */
int GetWhoseTurn(POSITION thePosition) {
	return (thePosition >> 20) & 1;
}

/************************************************************************
**
** NAME:        InitializeDatabases
**
** DESCRIPTION: Initialize the gDatabase, a global variable.
**
************************************************************************/

void InitializeGame() {
	gMoveToStringFunPtr = &MoveToString;
	gGenerateMultipartMoveEdgesFunPtr = &GenerateMultipartMoveEdges;
}

void FreeGame() {}

/************************************************************************
**
** NAME:        DebugMenu
**
** DESCRIPTION: Menu used to debub internal problems. Does nothing if
**              kDebugMenu == FALSE
**
************************************************************************/

void DebugMenu() {}

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
	char GetMyChar();
	POSITION GetInitialPosition();

	do {
		printf("\n\t----- Game-specific options for %s -----\n\n", kGameName);

		printf("\tCurrent Initial Position:\n");
		PrintPosition(gInitialPosition, gPlayerName[kPlayerOneTurn], kHumansTurn);
		printf("\t\t\t\tInitial target Score1: %d\n\t\t\t\tInitial target Score2: %d\n\n"
		       ,winScore1,winScore2);
		printf("\tF)\tToggle (F)orcing to move the white piece (currently %s)\n",
		       (noMoveWhite ? "disabled" : "enabled"));
		printf("\tR)\tToggle scoring mode to (R)everse (currently %s)\n",
		       invertedScoring ? "enabled" : "disabled");
		printf("\tS)\tSet the target (S)cores for player1 and 2\n");
		printf("\tI)\tChoose the (I)nitial position for the dots\n");
		printf("\n\n\tb)\t(B)ack = Return to previous activity.\n");
		printf("\n\nSelect an option: ");

		switch(GetMyChar()) {
		case 'Q': case 'q':
			ExitStageRight();
			break;
		case 'H': case 'h':
			HelpMenus();
			break;
		case 'F': case 'f':
			noMoveWhite = !noMoveWhite;
			break;
		case 'S': case 's':
			SetTargetScoresMenu();
			break;
		case '1':
			gInitialPosition = GetInitialPosition();
			break;
		case 'R': case 'r':
			invertedScoring  = !invertedScoring;
			if(invertedScoring) {
				printf("\nReverse Scoring enabled\n");
			} else {
				printf("\nReverse Scoring disabled\n");
			}
			break;
		case 'I': case 'i':
			SetDotPositionMenu();
			break;
		case 'B': case 'b':
			return;
		default:
			printf("\nSorry, I don't know that option. Try another.\n");
			HitAnyKeyToContinue();
			break;
		}
	} while(TRUE);


}



/// 2nd menu ///
void SetTargetScoresMenu() {

	printf("\n\t----- Game-specific options for %s -----\n", kGameName);
	printf("\n\t--------- Score Selection Menu ---------\n\n");

	printf("\tCurrent Initial Win Conditions:\n\n");
	printf("\tFor normal scoring:\n\
\t\tplayer A WINS if player A has >= score1 and\n\
\t\tplayer B has >= score2\n\
\tFor Inverted scoring\n\
\t\tplayer A WINS if player A has >= score1 and\n\
\t\tplayer B has < score2\n\n\
\tCurrent Score1 : %d\n\
\tCurrent Score2 : %d\n\n\
\tinput new Score1 (%d-%d) : ", winScore1, winScore2, MIN_SCORE, MAX_SCORE);
	printf("\n\tinput new Score2 (%d-%d) : ", MIN_SCORE, MAX_SCORE);
	winScore1 = GetMyInt();
	winScore2 = GetMyInt();
	return;

}

void SetDotPositionMenu() {

	//int dots[] = {0,0,0,0,0,0,0,0,0};
	int i = 0;
	char line[20];
	int temp;

	printf("\n\t----- Game-specific options for %s -----\n", kGameName);
	printf("\n\t----- Dot Position Selection Menu! -----\n\n");
	printf("\
\tThe Current Board Configuration is:\n\n\
\t\t+-+-+-+\t+-+-+-+\n\
\t\t|1|2|3|\t|%c|%c|%c|\n\
\t\t+-+-+-+\t+-+-+-+\n\
\t\t|4|5|6|\t|%c|%c|%c|\n\
\t\t+-+-+-+\t+-+-+-+\n\
\t\t|7|8|9|\t|%c|%c|%c|\n\
\t\t+-+-+-+\t+-+-+-+\n\n\
\tPlease Input New Positions for (any number of) dots(ex: \"1,2,3,4\"): ",
	       theInitialBoard[0],
	       theInitialBoard[1],
	       theInitialBoard[2],
	       theInitialBoard[3],
	       theInitialBoard[4],
	       theInitialBoard[5],
	       theInitialBoard[6],
	       theInitialBoard[7],
	       theInitialBoard[8]);

	//get the positions for the dots and set the initial board
	GetMyStr(line, 20);
	printf("\n\nhello1%s", line);
	for (i = 0; i <= 8; i++) {
		theInitialBoard[i] = ' ';
	}
	printf("\n\nhello2%s", line);

	for (i = 0; i <= 19; i++) {
		temp = ctoi(line[i]);
		if (temp != -1) {
			theInitialBoard[temp] = 'o';
		}
	}

	printf("%s", theInitialBoard);

	for (i = 0; i < 12; i++) {
		pointsEarned[i] = 0;
	}

	if (theInitialBoard[0] == 'o' || theInitialBoard[3] == 'o') pointsEarned[1] = 1;
	if (theInitialBoard[1] == 'o' || theInitialBoard[4] == 'o') pointsEarned[3] = 1;
	if (theInitialBoard[2] == 'o' || theInitialBoard[5] == 'o') pointsEarned[5] = 1;
	if (theInitialBoard[3] == 'o' || theInitialBoard[6] == 'o') pointsEarned[7] = 1;
	if (theInitialBoard[4] == 'o' || theInitialBoard[7] == 'o') pointsEarned[9] = 1;
	if (theInitialBoard[5] == 'o' || theInitialBoard[8] == 'o') pointsEarned[11] = 1;
	if (theInitialBoard[6] == 'o' || theInitialBoard[7] == 'o') pointsEarned[0] = 1;
	if (theInitialBoard[3] == 'o' || theInitialBoard[4] == 'o') pointsEarned[2] = 1;
	if (theInitialBoard[0] == 'o' || theInitialBoard[1] == 'o') pointsEarned[4] = 1;
	if (theInitialBoard[7] == 'o' || theInitialBoard[8] == 'o') pointsEarned[6] = 1;
	if (theInitialBoard[4] == 'o' || theInitialBoard[5] == 'o') pointsEarned[8] = 1;
	if (theInitialBoard[1] == 'o' || theInitialBoard[2] == 'o') pointsEarned[10] = 1;


	if (theInitialBoard[0] == 'o' && theInitialBoard[3] == 'o') pointsEarned[1] = 2;
	if (theInitialBoard[1] == 'o' && theInitialBoard[4] == 'o') pointsEarned[3] = 2;
	if (theInitialBoard[2] == 'o' && theInitialBoard[5] == 'o') pointsEarned[5] = 2;
	if (theInitialBoard[3] == 'o' && theInitialBoard[6] == 'o') pointsEarned[7] = 2;
	if (theInitialBoard[4] == 'o' && theInitialBoard[7] == 'o') pointsEarned[9] = 2;
	if (theInitialBoard[5] == 'o' && theInitialBoard[8] == 'o') pointsEarned[11] = 2;
	if (theInitialBoard[6] == 'o' && theInitialBoard[7] == 'o') pointsEarned[0] = 2;
	if (theInitialBoard[3] == 'o' && theInitialBoard[4] == 'o') pointsEarned[2] = 2;
	if (theInitialBoard[0] == 'o' && theInitialBoard[1] == 'o') pointsEarned[4] = 2;
	if (theInitialBoard[7] == 'o' && theInitialBoard[8] == 'o') pointsEarned[6] = 2;
	if (theInitialBoard[4] == 'o' && theInitialBoard[5] == 'o') pointsEarned[8] = 2;
	if (theInitialBoard[1] == 'o' && theInitialBoard[2] == 'o') pointsEarned[10] = 2;
	return;
}


/************************************************************************
**
** NAME:        SetTclCGameSpecificOptions
**
** DESCRIPTION: Set the C game-specific options (called from Tcl)
**              Ignore if you don't care about Tcl for now.
**
************************************************************************/

void SetTclCGameSpecificOptions(int theOptions[]) {
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

// I want to change whoseturn to take a POSITION as an input and keep
// track of things like that -- since whose turn it is is part of the
// information kept in a position

POSITION DoMove(POSITION thePosition, MOVE theMove) {
	int score = 0;
	int temp = 0;

	if (GetWhoseTurn(thePosition)) {
		//it is player 1's turn
		temp = GetRedPosition(thePosition);
		score = GetPlayer1Score(thePosition);
		score += pointsEarned[(theMove >> 4) - 2];
		if ((theMove & 0x0000000F) == 0) {
			//            printf("this should nt happen\n");
			thePosition = thePosition & 0xFFF0F0FF;
		} else {
			thePosition = thePosition & 0xFFF0F00F;
		}
		thePosition = thePosition | (score << 16);
		thePosition = thePosition | (theMove << 4);
		thePosition = thePosition ^ 0x00100000;
#ifdef debugdomove
		printf("1DID MOVE: %x NEW POS: %x\n", theMove, thePosition);
#endif
		return thePosition;
	} else {
		//it is player 2's turn
		score = GetPlayer2Score(thePosition);

#ifdef debugdomove
		printf("2DID0 MOVE: %x NEW POS: %x\n", theMove, thePosition);
#endif
		temp = GetBluePosition(thePosition);
		if ((theMove & 0x0000000F) == 0) {
			//  printf("this should nt happen\n");
			thePosition = thePosition & 0xFFFF0FF0;
		} else {
			thePosition = thePosition & 0xFFFF0F00;
		}


#ifdef debugdomove
		printf("2DID1 MOVE: %x NEW POS: %x\n", theMove, thePosition);
#endif

		score += pointsEarned[(theMove >> 4) - 2];
#ifdef debugdomove
		printf("2DID2 MOVE: %x NEW POS: %x SCORE: %x\n", theMove, thePosition, score);
#endif
		thePosition = thePosition | (score << 12);

#ifdef debugdomove
		printf("2DID3 MOVE: %x NEW POS: %x\n", theMove, thePosition);
#endif
		temp = (theMove & 0x0000000F) << 4;
		thePosition = (thePosition | temp) | (theMove >> 4);

#ifdef debugdomove
		printf("2DID4 MOVE: %x NEW POS: %x\n", theMove, thePosition);
#endif
		thePosition = thePosition ^ 0x00100000;

#ifdef debugdomove
		printf("2DID5 MOVE: %x NEW POS: %x\n", theMove, thePosition);
#endif

		return thePosition;
	}
	return 0;
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
	return 0x001009BD;
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
	printf("In his infinite wisdom, %8s's move = ",computersName);
	PrintMove(computersMove);
	printf("\n");
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
** CALLS:       BOOLEAN ThreeInARow()
**              BOOLEAN AllFilledIn()
**              PositionToBlankOX()
**
************************************************************************/

//erroneous since there can exist a primitive winning condition
VALUE Primitive(POSITION position) {
	if (invertedScoring == 0) {
		if (GetWhoseTurn(position) == 0) {
			//if it is player2's turn and player1 has more than the min
			//then it is a lose
			if (GetPlayer2Score(position) >= winScore1 &&
			    GetPlayer1Score(position) >= winScore2) {
				//printf("Func call: Primitive(0x%x) = lose\n", position);
				return (gStandardGame ? win : lose);
			}
			if (GetPlayer2Score(position) >= winScore2 &&
			    GetPlayer1Score(position) >= winScore1) {
				//printf("Func call: Primitive(0x%x) = lose\n", position);
				return (gStandardGame ? lose : win);
			}

			if (GetPlayer2Score(position) >= winScore1 &&
			    GetPlayer1Score(position) < winScore2) {
				return (gStandardGame ? lose : win);
			}
			if (GetPlayer2Score(position) < winScore2 &&
			    GetPlayer1Score(position) >= winScore1) {
				return (gStandardGame ? win : lose);
			}

		} else {

			//if it is player2's turn and player1 has more than the min
			//then it is a lose
			if (GetPlayer1Score(position) >= winScore1 &&
			    GetPlayer2Score(position) >= winScore2) {
				//printf("Func call: Primitive(0x%x) = lose\n", position);
				return (gStandardGame ? win : lose); /////////////////////////win
			}
			if (GetPlayer1Score(position) >= winScore2 &&
			    GetPlayer2Score(position) >= winScore1) {
				//printf("Func call: Primitive(0x%x) = lose\n", position);
				return (gStandardGame ? lose : win); /////////////////////////win
			}

			if (GetPlayer1Score(position) >= winScore1 &&
			    GetPlayer2Score(position) < winScore2) {
				return (gStandardGame ? lose : win);
			}
			if (GetPlayer1Score(position) < winScore2 &&
			    GetPlayer2Score(position) >= winScore1) {
				return (gStandardGame ? win : lose);
			}
		}
	}
	//inverted scoring
	else if (invertedScoring == 1) {
		if (GetWhoseTurn(position) == 0) {
			//if it is player2's turn and player2 has more than the max
			//and player1 has less than the min, it is a lose
			if (GetPlayer2Score(position) >= winScore1 &&
			    GetPlayer1Score(position) < winScore2) {
				return (gStandardGame ? win : lose);
			}
			//if it is player2's turn and player1 has more than the max
			//and player2 has less than the min, it is a win
			if (GetPlayer1Score(position) >= winScore1 &&
			    GetPlayer2Score(position) < winScore2) {
				return (gStandardGame ? lose : win);
			}
			if (GetPlayer2Score(position) >= winScore2 &&
			    GetPlayer1Score(position) >= winScore1) {
				return (gStandardGame ? lose : win);
			}

		} else {
			//if it is player1's turn and player1 has more than the max
			//and player2 has less than the min, it is a lose
			if (GetPlayer1Score(position) >= winScore1 &&
			    GetPlayer2Score(position) < winScore2) {
				return (gStandardGame ? win : lose);
			}
			//if it is player1's turn and player2 has more than the max
			//and player1 has less than the min, it is a win
			if (GetPlayer2Score(position) >= winScore1 &&
			    GetPlayer1Score(position) < winScore2) {
				return (gStandardGame ? lose : win);
			}
			if (GetPlayer1Score(position) >= winScore2 &&
			    GetPlayer2Score(position) >= winScore1) {
				return (gStandardGame ? lose : win);
			}
		}
	}
	//printf("Func call: Primitive(0x%x) = undecided\n", position);
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
char GetInitChar(int i, int j){
	if (!j) {
		if (theInitialBoard[i] == 'o') {
			return '<';
		} else {
			return ' ';
		}
	} else {
		if (theInitialBoard[i] == 'o') {
			return '>';
		} else {
			return ' ';
		}
	}
}

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn) {

	ClearTheBoard();
	theBoard[pieceToBoard(GetRedPosition(position),0)] = 'R';
	theBoard[pieceToBoard(GetRedPosition(position),1)] = 'R';
	theBoard[pieceToBoard(GetBluePosition(position),0)] = 'B';
	theBoard[pieceToBoard(GetBluePosition(position),1)] = 'B';
	theBoard[pieceToBoard(GetWhitePosition(position),0)] = 'W';
	theBoard[pieceToBoard(GetWhitePosition(position),1)] = 'W';
	printf("\n\t\t+---+---+---+\t\t+---+---+---+\n");
	printf("  \t\t|%c1%c|%c2%c|%c3%c|\t\t| %c | %c | %c |\n",
	       GetInitChar(0,0),GetInitChar(0,1),GetInitChar(1,0),
	       GetInitChar(1,1),GetInitChar(2,0),GetInitChar(2,1),
	       theBoard[0],
	       theBoard[1],
	       theBoard[2]);
	printf("\t\t+---+---+---+\t\t+---+---+---+\n");
	printf("LEGEND:\t\t|%c4%c|%c5%c|%c6%c|\t\t| %c | %c | %c |\t RedScore: %d BlueScore: %d\n",
	       GetInitChar(3,0),GetInitChar(3,1),GetInitChar(4,0),
	       GetInitChar(4,1),GetInitChar(5,0),GetInitChar(5,1),
	       theBoard[3],
	       theBoard[4],
	       theBoard[5],
	       GetPlayer1Score(position),
	       GetPlayer2Score(position));
	printf("\t\t+---+---+---+\t\t+---+---+---+\n");
	printf("  \t\t|%c7%c|%c8%c|%c9%c|\t\t| %c | %c | %c |\n",
	       GetInitChar(6,0),GetInitChar(6,1),GetInitChar(7,0),
	       GetInitChar(7,1),GetInitChar(8,0),GetInitChar(8,1),
	       theBoard[6],
	       theBoard[7],
	       theBoard[8]);
	printf("\t\t+---+---+---+\t\t+---+---+---+\n");
	printf("\t\t%s\n",
	       GetPrediction(position,playerName,usersTurn));
	return;
}

void ClearTheBoard() {
	int i;
	for (i = 0; i < 9; i++) {
		theBoard[i] = theInitialBoard[i];
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
	//////////////////////////////////////////////////
	//         DECLARE VARIABLES                    //
	//////////////////////////////////////////////////
	MOVELIST *head = NULL;
	int theBoard[] = {0,0,0,0,0,0,0,0,0};
	int i, j, k, otherPlayerPos = 0, whitePos = 0, myPos = 0;
	int myNewPos = 0;
	int whiteNewPos = 0;
	int newMove = 0;

	//////////////////////////////////////////////////
	//          INITIALIZE THE VARIABLES            //
	//////////////////////////////////////////////////
	if (Primitive(position) == undecided) {
		if (!GetWhoseTurn(position)) {
			// printf("it is blue's turn\n");
			myPos = GetBluePosition(position);
			otherPlayerPos = GetRedPosition(position);
		}
		else {
			//printf("it is red's turn\n");
			myPos = GetRedPosition(position);
			otherPlayerPos = GetBluePosition(position);
		}
		whitePos = GetWhitePosition(position);

		//////////////////////////////////////////////////
		//   FROM HERE DOWN BOARD IS REP AS 0-8         //
		//////////////////////////////////////////////////
		//        SET UP THE BOARD ABSTRACTION          //
		//////////////////////////////////////////////////
		theBoard[pieceToBoard(otherPlayerPos, 0)] = 1;
		theBoard[pieceToBoard(otherPlayerPos, 1)] = 1;
		theBoard[pieceToBoard(whitePos, 0)] = 2;
		theBoard[pieceToBoard(whitePos, 1)] = 2;
		theBoard[pieceToBoard(myPos, 0)] = 3;
		theBoard[pieceToBoard(myPos, 1)] = 3;

		//////////////////////////////////////////////////
		//  TRY ALL MOVES WITH BOTH PIECES HORIZONTAL   //
		//////////////////////////////////////////////////

		for (j = 0; j <= 8; j+= 1) {
			theBoard[pieceToBoard(myPos, 0)] = 3;
			theBoard[pieceToBoard(myPos, 1)] = 3;
			myNewPos = 0;
			whiteNewPos = 0;

			//////////////////////////////////////////////////
			// PLACE THE FIRST PIECE HORIZONTALLY IF POSSIB //
			//////////////////////////////////////////////////
			if(!(j == 2 || j == 5 || j == 8)) {
				if((theBoard[j] == 0 &&
				    (theBoard[j + 1] == 0 || theBoard[j + 1] == 3)) ||
				   (theBoard[j] == 3 &&
				    theBoard[j + 1] == 0)) {
					myNewPos = boardToPos(j, 'h');
					for (i = 0; i <= 8; i++) {
						if (theBoard[i] == 3) theBoard[i] = 0;
					}
					theBoard[j] = 3;
					theBoard[j + 1] = 3;

					//
					if (noMoveWhite) head = CreateMovelistNode(myNewPos << 4, head);
					//

					//////////////////////////////////////////////////
					//   PLACE 2ND PIECE HORIZONTALLY IF POSSIBLE   //
					//////////////////////////////////////////////////
					for (k = 0; k <= 8; k++) {
						if(!(k == 2 || k == 5 || k == 8)) {
							if ((theBoard[k] == 0 &&
							     (theBoard[k + 1] == 0 || theBoard[k + 1] == 2)) ||
							    (theBoard[k] == 2 &&
							     theBoard[k + 1] == 0)) {
								whiteNewPos = boardToPos(k,'h');
								newMove = (myNewPos << 4) | whiteNewPos;
								head = CreateMovelistNode(newMove, head);
							}
						}
					}
					theBoard[j] = 0;
					theBoard[j + 1] = 0;
				}
			}
		}

		//////////////////////////////////////////////////
		//     PLACE 1ST HORIZ 2ND VERT IF POSSIBLE     //
		//////////////////////////////////////////////////
		for (j = 0; j <= 8; j+= 1) {
			theBoard[pieceToBoard(myPos, 0)] = 3;
			theBoard[pieceToBoard(myPos, 1)] = 3;
			myNewPos = 0;
			whiteNewPos = 0;

			//////////////////////////////////////////////////
			// PLACE THE FIRST PIECE HORIZONTALLY IF POSSIB //
			//////////////////////////////////////////////////
			if(!(j == 2 || j == 5 || j == 8)) {
				if((theBoard[j] == 0 &&
				    (theBoard[j + 1] == 0 || theBoard[j + 1] == 3)) ||
				   (theBoard[j] == 3 &&
				    theBoard[j + 1] == 0)) {
					myNewPos = boardToPos(j, 'h');
					for (i = 0; i <= 8; i++) {
						if (theBoard[i] == 3) theBoard[i] = 0;
					}
					theBoard[j] = 3;
					theBoard[j + 1] = 3;

					//
					if (noMoveWhite) head = CreateMovelistNode(myNewPos << 4, head);
					//

					//////////////////////////////////////////////////
					//   PLACE 2ND PIECE VERTICALLY IF POSSIBLE     //
					//////////////////////////////////////////////////
					for (k = 0; k <= 8; k++) {
						if(!(k == 6 || k == 7 || k == 8)) {
							if ((theBoard[k] == 0 &&
							     (theBoard[k + 3] == 0 || theBoard[k + 3] == 2)) ||
							    (theBoard[k] == 2 &&
							     theBoard[k + 3] == 0)) {
								whiteNewPos = boardToPos(k,'v');
								newMove = (myNewPos << 4) | whiteNewPos;
								head = CreateMovelistNode(newMove, head);
							}
						}
					}

					theBoard[j] = 0;
					theBoard[j + 1] = 0;
				}
			}
		}

		//////////////////////////////////////////////////
		// FIRST PLACE 1ST PIECS VERT AND 2ND HORIZONTA //
		//////////////////////////////////////////////////

		for (j = 0; j <= 8; j+= 1) {
			theBoard[pieceToBoard(myPos, 0)] = 3;
			theBoard[pieceToBoard(myPos, 1)] = 3;
			myNewPos = 0;
			whiteNewPos = 0;

			//////////////////////////////////////////////////
			// PLACE THE FIRST PIECE  VERTICALLY  IF POSSIB //
			//////////////////////////////////////////////////
			if(!(j == 6 || j == 7 || j == 8)) {
				if ((theBoard[j] == 0 &&
				     (theBoard[j + 3] == 0 || theBoard[j + 3] == 3)) ||
				    (theBoard[j] == 3 &&
				     theBoard[j + 3] == 0)) {
					myNewPos = boardToPos(j, 'v');
					for (i = 0; i <= 8; i++) {
						if (theBoard[i] == 3) theBoard[i] = 0;
					}
					theBoard[j] = 3;
					theBoard[j + 3] = 3;

					//
					if (noMoveWhite) head = CreateMovelistNode(myNewPos << 4, head);
					//


					//////////////////////////////////////////////////
					// PLACE THE 2ND PIECE HORIZONTALLY IF POSSIBLE //
					//////////////////////////////////////////////////
					for (k = 0; k <= 8; k++) {
						if(!(k == 2 || k == 5 || k == 8)) {
							if ((theBoard[k] == 0 &&
							     (theBoard[k + 1] == 0 || theBoard[k + 1] == 2)) ||
							    (theBoard[k] == 2 &&
							     theBoard[k + 1] == 0)) {
								whiteNewPos = boardToPos(k,'h');
								newMove = (myNewPos << 4) | whiteNewPos;
								head = CreateMovelistNode(newMove, head);
							}
						}
					}
					theBoard[j] = 0;
					theBoard[j + 3] = 0;
				}
			}
		}


		//////////////////////////////////////////////////
		// FOR WHEN BOTH PIECES CAN BE PLACD VERTICALLY //
		//////////////////////////////////////////////////
		for (j = 0; j <= 8; j+= 1) {
			theBoard[pieceToBoard(myPos, 0)] = 3;
			theBoard[pieceToBoard(myPos, 1)] = 3;
			myNewPos = 0;
			whiteNewPos = 0;

			//////////////////////////////////////////////////
			// PLACE THE FIRST PIECE  VERTICALLY  IF POSSIB //
			//////////////////////////////////////////////////
			if(!(j == 6 || j == 7 || j == 8)) {
				if ((theBoard[j] == 0 &&
				     (theBoard[j + 3] == 0 || theBoard[j + 3] == 3)) ||
				    (theBoard[j] == 3 &&
				     theBoard[j + 3] == 0)) {
					myNewPos = boardToPos(j, 'v');
					for (i = 0; i <= 8; i++) {
						if (theBoard[i] == 3) theBoard[i] = 0;
					}
					theBoard[j] = 3;
					theBoard[j + 3] = 3;

					//
					if (noMoveWhite) head = CreateMovelistNode(myNewPos << 4, head);
					//


					//////////////////////////////////////////////////
					// PLACE THE 2ND PIECE VERTICALLY  IF  POSSIBLE //
					//////////////////////////////////////////////////
					for (k = 0; k <= 8; k++) {
						if(!(k == 6 || k == 7 || k == 8)) {
							if ((theBoard[k] == 0 &&
							     (theBoard[k + 3] == 0 || theBoard[k + 3] == 2)) ||
							    (theBoard[k] == 2 &&
							     theBoard[k + 3] == 0)) {
								whiteNewPos = boardToPos(k,'v');
								newMove = (myNewPos << 4) | whiteNewPos;
								head = CreateMovelistNode(newMove, head);
							}
						}
					}
					theBoard[j] = 0;
					theBoard[j + 3] = 0;
				}
			}
		}
		return(head);
	} else {
		return(NULL);
	}
}

//return the num 0-8 that corresponds to the part of the piece starting
//at pos
int pieceToBoard(int pos, int piecePart){
	int move = 0;
	if (piecePart == 0) {
		move = posToBoard[pos - 2];
		return move - 1; //////////////////
	}
	else {
		move = posToBoard[pos - 2];
		if ((pos & 0x00000001) == 0) {
			move++;
		} else {
			move += 3;
		}
		return move -1;
	}
	return move;
}


//return the position of the current piece with the orientation
int boardToPos(int pos, char orientation){
	int move = 0;
	if (orientation == 'v') {
		move = ((pos + 1) << 1) + 1;
	} else {
		move = boardToPosition[pos];
	}
	return move;
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
		printf("%8s's move [(u)ndo/(1-8)(v|h) (1-8)(v|h) ] :", playerName);

		ret = HandleDefaultTextInput(thePosition, theMove, playerName);
		if(ret != Continue)
			return(ret);

	}
	while (TRUE);
	return(Continue); /* this is never reached, but lint is now happy */
	return 0;
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
	return
	        ((input[0] <= '8' && input[0] >= '1') &&
	         (input[1] <= 'v' || input[1] >= 'h') &&
	         (input[3] <= '8' && input[3] >= '1') &&
	         (input[4] <= 'v' || input[4] >= 'h')) ||
	        ((input[0] <= '8' && input[0] >= '1') &&
	         (input[1] <= 'v' || input[1] >= 'h'));

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

int ctoi(char c) {
	switch (c) {
	case '1': return 0;
	case '2': return 1;
	case '3': return 2;
	case '4': return 3;
	case '5': return 4;
	case '6': return 5;
	case '7': return 6;
	case '8': return 7;
	case '9': return 8;
	case 'v': return 0;
	case 'h': return 1;
	}
	return -1;
}

STRING MoveToString(MOVE theMove) {
	STRING move = (STRING) SafeMalloc(8);
	if( (theMove & 0x0F) == 0 ) {
		sprintf( move, "(%c%c)", moveToTextInt[(theMove >> 4) -2], moveToTextOri[(theMove >> 4) -2] );
	} else {
		sprintf( move, "(%c%c %c%c)",
		         moveToTextInt[(theMove >> 4) -2], moveToTextOri[(theMove >> 4) -2],
		         moveToTextInt[(theMove & 0x0F) -2], moveToTextOri[(theMove & 0x0F) -2] );
	}

	return move;
}

MOVE ConvertTextInputToMove(STRING input) {
	MOVE temp;
	temp = boardToPos(ctoi(input[0]), input[1]);
	temp = temp << 4;
	if (input[3] >= '1' &&
	    input[3] <= '9') {
		temp = temp | boardToPos(ctoi(input[3]), input[4]);
	}
	return temp;
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

void PrintMove(MOVE theMove) {
	STRING moveString = MoveToString(theMove);
	printf( "%s", moveString );
	SafeFree(moveString);
}

int NumberOfOptions() {
	return 2*2*2*(MAX_SCORE-MIN_SCORE+1)*(MAX_SCORE-MIN_SCORE+1);
}

int getOption() {
	int option = 1;
	if(gStandardGame) option += 1;
	if(invertedScoring) option += 1*2;
	if(noMoveWhite) option += 1*(2*2);
	option += (winScore1-MIN_SCORE) * (2*2*2);
	option += (winScore2-MIN_SCORE) * (2*2*2*MAX_SCORE);
	return option;
}

void setOption(int option) {
	option -= 1;
	gStandardGame = option%2==1;
	invertedScoring = option/2%2==1;
	noMoveWhite = option/(2*2)%2==1;
	winScore1 = option/(2*2*2)%(MAX_SCORE-MIN_SCORE+1)+MIN_SCORE;
	winScore2 = option/(2*2*2*MAX_SCORE)%(MAX_SCORE-MIN_SCORE+1)+MIN_SCORE;
}

int betweenMap[14] = {-1,-1,24,16,19,17,14,18,25,21,20,22,15,23};
int horList[2][6] = {{0,1,3,4,6,7}, {1,2,4,5,7,8}};
int verList[2][6] = {{0,1,2,3,4,5}, {3,4,5,6,7,8}};

POSITION InteractStringToPosition(STRING str) {
	enum UWAPI_Turn t;
	unsigned int r, c; // Unused
	STRING board;
	if (!UWAPI_Board_Regular2D_ParsePositionString(str, &t, &r, &c, &board)) {
		return INVALID_POSITION;
	}
	POSITION turn = (t == UWAPI_TURN_A) ? 0x100000LL : 0;
	POSITION red, white, blue;

	for (int i = 0; i < 6; i++) {
		char h1 = board[horList[0][i]];
		char h2 = board[horList[1][i]];
		char v1 = board[verList[0][i]];
		char v2 = board[verList[1][i]];
		// We're actually switching B and R here because we want B to be the first player
		// but the game was coded to have red go first.
		if (h1 == h2) {
			int value = boardToPos(horList[0][i], 'h');
			if (h1 == 'B') {
				red = value << 8;
			} else if (h1 == 'W') {
				white = value << 4;
			} else if (h1 == 'R') {
				blue = value;
			}
		}
		if (v1 == v2) {
			int value = boardToPos(verList[0][i], 'v');
			if (v1 == 'B') {
				red = value << 8;
			} else if (v1 == 'W') {
				white = value << 4;
			} else if (v1 == 'R') {
				blue = value;
			}
		}
	}

	POSITION ptsA = (10 * (board[9] - '0') + (board[10] - '0'));
	POSITION ptsB = (10 * (board[11] - '0') + (board[12] - '0'));
	// Conversion from intermediate state to real position
	if (board[13] != '-') {
		if (turn) {
			ptsA -= pointsEarned[(red >> 8) - 2];
			red = (board[13] - 'a') << 8;
		} else {
			ptsB -= pointsEarned[blue - 2];
			blue = board[13] - 'a';
		}
	}
	ptsA <<= 16;
	ptsB <<= 12;
	SafeFreeString(board);
	return turn | ptsA | ptsB | red | white | blue;
}

STRING InteractPositionToString(POSITION pos) {
	char board[15];
	memset(board, '-', 15 * sizeof(char));
	
	enum UWAPI_Turn turn = (GetWhoseTurn(pos)) ? UWAPI_TURN_A : UWAPI_TURN_B;

	if (pos >> 63) { // Convert intermediate state to real position
		board[13] = 'a' + ((pos >> 59) & 0b1111);
		pos &= 0xFFFFFFFFFFF;
	}

	// We're actually switching B and R here because we want B to be the first player
	// but the game was coded to have red go first.
	board[pieceToBoard(GetRedPosition(pos), 0)] = 'B';
	board[pieceToBoard(GetRedPosition(pos), 1)] = 'B';
	board[pieceToBoard(GetBluePosition(pos), 0)] = 'R';
	board[pieceToBoard(GetBluePosition(pos), 1)] = 'R';
	board[pieceToBoard(GetWhitePosition(pos), 0)] = 'W';
	board[pieceToBoard(GetWhitePosition(pos), 1)] = 'W';

	int player1Score = GetPlayer1Score(pos);
	int player2Score = GetPlayer2Score(pos);
	board[9] = (player1Score / 10) + '0';
	board[10] = (player1Score % 10) + '0';
	board[11] = (player2Score / 10) + '0';
	board[12] = (player2Score % 10) + '0';
	board[14] = '\0';

	return UWAPI_Board_Regular2D_MakeBoardString(turn, 16, board);
}

STRING InteractMoveToString(POSITION pos, MOVE mv) {
	(void)pos;
	if (mv >> 9) { // Placing player's piece.	
		return UWAPI_Board_Regular2D_MakeAddStringWithSound(moveToTextOri[((mv & 0xFF) >> 4)-2], betweenMap[(mv >> 4) & 0xF], 'x');
	} else if ((mv >> 8) & 1) { // Placing neutral piece.
		return UWAPI_Board_Regular2D_MakeAddStringWithSound(moveToTextOri[(mv & 0x0F)-2], betweenMap[mv & 0xF], 'x');
	} else {
		return MoveToString(mv);
	}
}

MULTIPARTEDGELIST* GenerateMultipartMoveEdges(POSITION position, MOVELIST *moveList, POSITIONLIST *positionList) {
	MULTIPARTEDGELIST *mpel = NULL;
	int edgeFromAdded = 0;

	if (GetWhoseTurn(position)) {
		while (moveList != NULL) {
			MOVE move = moveList->move;
			POSITION origRed = (position >> 8) & 0xF;
			POSITION interPos = (1LL << 63) | (origRed << 59) | (position & 0xFFF0F0FF) | (positionList->position & 0xF0000) | ((move >> 4) << 8);
			if (!(edgeFromAdded & (1 << (move >> 4)))) {
				mpel = CreateMultipartEdgeListNode(position, interPos, move | 0x200, 0, FALSE, mpel);
				edgeFromAdded |= (1 << (move >> 4));
			}
			mpel = CreateMultipartEdgeListNode(interPos, positionList->position, move | 0x100, move, TRUE, mpel);
			
			moveList = moveList->next;
			positionList = positionList->next;
		}
	} else {
		while (moveList != NULL) {
			MOVE move = moveList->move;

			POSITION origBlue = position & 0xF;
			POSITION interPos = (1LL << 63) | (origBlue << 59) | (position & 0xFFFF0FF0) | (positionList->position & 0xF000) | (move >> 4);
			if (!(edgeFromAdded & (1 << (move >> 4)))) {
				mpel = CreateMultipartEdgeListNode(position, interPos, move | 0x200, 0, FALSE, mpel);
				edgeFromAdded |= (1 << (move >> 4));
			}
			mpel = CreateMultipartEdgeListNode(interPos, positionList->position, move | 0x100, move, TRUE, mpel);
			
			moveList = moveList->next;
			positionList = positionList->next;
		}
	}

	return mpel;
}
