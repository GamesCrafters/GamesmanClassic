
/*****************************************************************************
**
** NAME:        mrcheckers.c
**
** DESCRIPTION: Rubik's Checkers
**
** AUTHOR:      Johnny Tran
**              Steve Wu
**
** DATE:        Sunday, February 26 2006
**
** UPDATE HIST:
** 3/19/2006     fixed DoMove, added ConvertTextInputToMove, printMove(for debugging)
**              Problems with internal row representation: printposition prints "bottom-up",
**              while directional functions use implied "top-down."
** 3/12/2006    Modified Forward-directions to return index instead of char
** 2/26/2006    GenerateMoves, DoMoves, forward-directions
** 2/13/2006    Started. Much of the code is shamelessly borrowed from
**              m1210.c, which is used as a template. Done: InitializeGame,
**              PrintPosition
** 2/20/2008    Added bounds checking, adjusted bounds to be from 2 to 9 and
**              implemented options commands -EthanR+AlanW
** 3/10/2008    Further refined functions for options. -EthanR+AlanW
*****************************************************************************/

/*****************************************************************************
**
** Everything below here must be in every game file
**
*****************************************************************************/
#include <stdio.h>
#include "gamesman.h"

POSITION gNumberOfPositions   = 0;      /* Calculated in InitializeGame */
POSITION kBadPosition         = -1;

POSITION gInitialPosition     = 0;      /* Calculated in InitializeGame */

//POSITION gMinimalPosition     = 0;       /* TODO: ? */

STRING kAuthorName          = "Johnny Tran and Steve Wu";
STRING kGameName            = "Rubik's Checkers";
BOOLEAN kPartizan            = TRUE;
BOOLEAN kDebugMenu           = TRUE;
BOOLEAN kGameSpecificMenu    = TRUE;
BOOLEAN kTieIsPossible       = FALSE;   /* TODO: Can both players be locked? */
BOOLEAN kLoopy               = TRUE;
BOOLEAN kDebugDetermineValue = FALSE;
void*    gGameSpecificTclInit = NULL;

STRING kHelpGraphicInterface = "";      /* TODO */

STRING kHelpTextInterface    =
        "Play like checkers, except only kings can capture, men can only move backwards\n and cannot capture, and jumping a piece demotes a king to a man and captures a man.";

STRING kHelpOnYourTurn =
        "Select a piece and its destination(s). (i.e. a2c4a6 to double-capture; a2\nto promote)";

STRING kHelpStandardObjective =
        "Eliminate all your opponent's pieces or block them from moving.";

STRING kHelpReverseObjective =
        "Eliminate all your pieces or block them from moving.";

STRING kHelpTieOccursWhen = "";   /* empty since kTieIsPossible == FALSE */

STRING kHelpExample =             /* TODO */
                      "Help Example";

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

// External Functions
#ifndef MEMWATCH
extern GENERIC_PTR      SafeMalloc();
extern void             SafeFree();
#endif
extern POSITION         generic_hash_init(int boardsize, int *pieces_array, int (*fn)(int *), int player);
extern POSITION         generic_hash_hash(char* board, int player);
extern char             *generic_hash_unhash(POSITION hash_number, char *empty_board);
extern int              generic_hash_turn (POSITION hashed);

STRING MoveToString(MOVE);



// How a piece can move
#define FORWARD               1
#define BACKWARD              2
#define CAPTURE               4

// Player representation
#define P1                    1
#define P2                    2

//Piece Movement representation
#define FORWARDLEFT           0
#define FORWARDRIGHT          1
#define BACKWARDRIGHT         2
#define BACKWARDLEFT          3

// Piece representation
#define EMPTY                 ' '
#define P1KING                'X'
#define P1MAN                 'x'
#define P2KING                'O'
#define P2MAN                 'o'
#define P1NAME                "X's"
#define P2NAME                "O's"

//POSSIBLY TEMPORARY:
//unsigned int currentTurn = P1;
//POSITION curBoard = 0;
int initialized = FALSE;

// How large the board is
unsigned int rows           = 6;  // Rubik's: 6, Checkers: 8
unsigned int cols           = 2;  // Rubik's: 4, Checkers: 4
unsigned int boardSize;     /* initialized in InitializeGame */

// How many rows closest to a player start out with pieces (< ROWS/2)
unsigned int startRows      = 1;

// Rubik's Checkers specific options
// You can change to regular checkers rules by changing these options
BOOLEAN demote              = TRUE;      // Capturing demotes a piece
unsigned int kingMobility   = (FORWARD | BACKWARD | CAPTURE);
unsigned int manMobility    = BACKWARD;  // Man can only move backwards
BOOLEAN forceCapture        = FALSE;     // No forced captures
BOOLEAN startPromoted       = TRUE;      // Starting pieces are already kings
unsigned int promoteRow     = BACKWARD;  // Promote on your row

/*
   typedef enum square {
    empty, orangeKing, orangeMan, greenKing, greenMan
   } squareState;
 */

//number of bits to use to represent board index in move hash
unsigned int MVHASHACC = 8;

int vcfg(int* pieces) {
	int maxPieces = startRows * cols;

	return (((pieces[0] + pieces[1]) <= maxPieces) &&
	        ((pieces[2] + pieces[3]) <= maxPieces));
}

void ForceInitialize() {
	initialized = TRUE;

	int maxPieces = startRows * cols;
	boardSize = rows * cols;
	int pieces[] = { P1KING, 0, maxPieces,
		         P1MAN,  0, maxPieces,
		         P2KING, 0, maxPieces,
		         P2MAN,  0, maxPieces,
		         EMPTY,  boardSize-(maxPieces*2), boardSize-1,
		         -1 };
	char* initialPosition = (char*)SafeMalloc(boardSize * sizeof(char));
	int i;
	gNumberOfPositions = generic_hash_init(boardSize, pieces, vcfg, 0);
	// Create initial position
	for (i = 0; i < maxPieces; i++) {
		initialPosition[i] = (startPromoted ? P1KING : P1MAN);
		initialPosition[boardSize-1 - i] = (startPromoted ? P2KING : P2MAN);
	}
	for (i = maxPieces; i < (boardSize - maxPieces); i++) {
		initialPosition[i] = EMPTY;
	}
	gInitialPosition = generic_hash_hash(initialPosition, P1);
	SafeFree(initialPosition);
	gMoveToStringFunPtr = &MoveToString;
}

void InitializeOnce() {
	if (initialized) return;
	initialized = TRUE;

	ForceInitialize();
}

void InitializeGame()
{
	InitializeOnce();
}
/*void InitializeGame()
   {
    int maxPieces = startRows * cols;
    boardSize = rows * cols;
    int pieces[] = { P1KING, 0, maxPieces,
                     P1MAN,  0, maxPieces,
                     P2KING, 0, maxPieces,
                     P2MAN,  0, maxPieces,
                     EMPTY,  boardSize-(maxPieces*2), boardSize-1,
                     -1 };
    char* initialPosition = (char*)SafeMalloc(boardSize * sizeof(char));
    int i;

    gNumberOfPositions = generic_hash_init(boardSize, pieces, NULL, 0);

    // Create initial position
    for (i = 0; i < maxPieces; i++) {
        initialPosition[i] = (startPromoted ? P2KING : P2MAN);
        initialPosition[boardSize-1 - i] = (startPromoted ? P1KING : P1MAN);
    }

    for (i = maxPieces; i < (boardSize - maxPieces); i++) {
        initialPosition[i] = EMPTY;
    }
    gInitialPosition = generic_hash_hash(initialPosition, P1);
    SafeFree(initialPosition);
   }*/

void FreeGame()
{
}

//Switches the forward and backward mobility for P2 -
//used for absolute movement directions
int reverseMobility(int mobility){
	if((mobility&FORWARD) == FORWARD && (mobility&BACKWARD) != BACKWARD)
		return (mobility|BACKWARD)&(CAPTURE|BACKWARD);
	else if ((mobility&BACKWARD) == BACKWARD && (mobility&FORWARD) != FORWARD)
		return (mobility|FORWARD)&(CAPTURE|FORWARD);
	else return mobility;
}

//Absolute directions: forward is defined to be from
//  P1 to P2, and right would be the right side while facing that direction
int forwardRight(int currentIndex){
	int isEvenRow = ((currentIndex/cols)%2 == 0),
	    isBegOfRow = ((currentIndex%cols) == 0),
	    isLastRow = ((currentIndex/cols) == rows-1);

	if(!isLastRow) {
		if(isEvenRow && !isBegOfRow)
			return currentIndex + cols-1;
		else if(!isEvenRow)
			return currentIndex + cols;
	}

	return -1; //forward-right DNE;
}

//Absolute directions: forward is defined to be from
//  P1 to P2, and left would be the left side while facing that direction
int forwardLeft(int currentIndex){
	int isEvenRow = ((currentIndex/cols)%2 == 0),
	    isLastOfRow = ((currentIndex%cols) == cols-1),
	    isLastRow = ((currentIndex/cols) == rows-1);

	if(!isLastRow) {
		if(!isEvenRow && !isLastOfRow) //odd row, not last column
			return currentIndex + cols + 1;
		else if(isEvenRow)
			return currentIndex + cols;
	}

	return -1; //forward-left DNE;
}

//Absolute directions: backward  is defined to be from
//  P2 to P1, and left would be the left side while facing that direction
int backwardLeft(int currentIndex){
	int isEvenRow = ((currentIndex/cols)%2 == 0),
	    isLastOfRow = ((currentIndex%cols) == cols-1),
	    isFirstRow = ((currentIndex/cols) == 0);
	if(!isFirstRow) {
		if(!isEvenRow && !isLastOfRow) //odd row, not last column
			return currentIndex - cols+1;
		else if(isEvenRow)
			return currentIndex - cols;
	}
	return -1;
}

//Absolute directions: backward is defined to be from
//  P2 to P1, and right would be the right side while facing that direction
int backwardRight(int currentIndex){
	int isEvenRow = ((currentIndex/cols)%2 == 0),
	    isBegOfRow = ((currentIndex%cols) == 0),
	    isFirstRow = ((currentIndex/cols) == 0);
	if(!isFirstRow) {
		if(isEvenRow && !isBegOfRow)
			return currentIndex - cols - 1;
		else if(!isEvenRow)
			return currentIndex - cols;
	}
	return -1;
}

//returns the 180 degree rotation of the input
//i.e.: forward-right returns backward-left
int oppositeMove(int previousMove){
	return (previousMove+2)%4;
}



/*void undoCapture(char *initialPosition, int whosTurn, int index, int previousMove){
   int myForwardRight = forwardRight(index), myForwardLeft = forwardLeft(index), myBackwardRight = backwardRight(index), myBackwardLeft = backwardLeft(index);
   int opposingMan, opposingKing;
   if(whosTurn == P1){
    opposingMan = P2MAN;
    opposingKing = P2KING;
   }
   else{
    opposingMan = P1MAN;
    opposingKing = P1KING;
   }
   switch(previousMove){
   case FORWARDRIGHT:
    if(initialPosition[myBackwardLeft] == EMPTY)
      initialPosition[myBackwardLeft] = opposingMan;
    else if(initialPosition[myBackwardLeft] == opposingMan)
      initialPosition[myBackwardLeft] = opposingKing;
    initialPosition[backwardLeft(myBackwardLeft)] = initialPosition[index];
    break;
   case FORWARDLEFT:
    if(initialPosition[myBackwardRight] == EMPTY)
      initialPosition[myBackwardRight] = opposingMan;
    else if(initialPosition[myBackwardRight] == opposingMan)
      initialPosition[myBackwardRight] = opposingKing;
    initialPosition[backwardRight(myBackwardRight)] = initialPosition[index];
    break;
   case BACKWARDRIGHT:
    if(initialPosition[myForwardLeft] == EMPTY)
      initialPosition[myForwardLeft] = opposingMan;
    else if(initialPosition[myForwardLeft] == opposingMan)
      initialPosition[myForwardLeft] = opposingKing;
    initialPosition[forwardLeft(myForwardLeft)] = initialPosition[index];
    break;
   case BACKWARDLEFT:
    if(initialPosition[myForwardRight] == EMPTY)
      initialPosition[myForwardRight] = opposingMan;
    else if(initialPosition[myForwardRight] == opposingMan)
      initialPosition[myForwardRight] = opposingKing;
    initialPosition[forwardRight(myForwardRight)] = initialPosition[index];
    break;
   }
   }
 */

/************************************************************************
**
** NAME:        DebugMenu
**
** DESCRIPTION: Menu used to debug internal problems. Does nothing if
**              kDebugMenu == FALSE
**
************************************************************************/

void DebugMenu() {
}


void setBoardSize() {

	int temp;

	// Prompt for board size
	printf("What is the board height? ");
	//getchar();
	temp = GetMyInt();
	if ((temp <= 1) || (temp > 9)) {
		printf("Board height should be between 2 to 9.\n");
		HitAnyKeyToContinue();
		return;
	}
	rows = temp;

	printf("What is the board width? ");
	temp = GetMyInt();
	if ((temp <= 1) || (temp > 9)) {
		printf("Board width should be between 2 to 9.\n");
		HitAnyKeyToContinue();
		return;
	}
	cols = temp;

	printf("How many starting rows per player? ");
	temp = GetMyInt();
	if ((temp < 1) || (temp >= rows/2)) {
		printf("Starting rows should be between 1 and 1/2 the number of rows\n");
		HitAnyKeyToContinue();
		return;
	}
	startRows = temp;

	ForceInitialize();
}

void changeRules() {
	if (demote) { // Switch to Checkers
		demote = FALSE;
		manMobility = FORWARD | CAPTURE;
		forceCapture = TRUE;
		startPromoted = FALSE;
		promoteRow = FORWARD;
	} else { // Switch to Rubik's
		demote = TRUE;
		manMobility = BACKWARD;
		forceCapture = FALSE;
		startPromoted = TRUE;
		promoteRow = BACKWARD;
	}

	ForceInitialize();
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
	// TODO: Allow changing of the board size and rules

	do {
		printf("\n\t----- Game-specific options for %s -----\n\n", kGameName);

		printf("\t1)\tChoose (manually) the initial position\n");
		printf("\t2)\tChange board size (the default position will be used)\n");
		if (demote)
			printf("\t3)\tUse Checkers rules\n");
		else
			printf("\t3)\tUse Rubik's Checkers rules\n");

		printf("\n\n\tb)\t(B)ack = Return to previous activity.\n");
		printf("\n\nSelect an option: ");

		//scanf("%s", c);

		switch(GetMyChar()) {
		case 'Q': case 'q':
			ExitStageRight();
		case 'H': case 'h':
			HelpMenus();
			break;
		case '1':
			gInitialPosition = GetInitialPosition();
			break;
		case '2':
			setBoardSize();
			break;
		case '3':
			changeRules();
			break;
		case 'b': case 'B':
			return;
		default:
			BadMenuChoice();
		}
	} while(TRUE);
}

/************************************************************************
**
** NAME:        SetTclCGameSpecificOptions
**
** DESCRIPTION: Set the C game-specific options (called from Tcl)
**              Ignore if you don't care about Tcl for now.
**
************************************************************************/

void SetTclCGameSpecificOptions(theOptions)
int theOptions[];
{
	/* No need to have anything here, we have no extra options */
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

POSITION DoMove(thePosition, theMove)
POSITION thePosition;
MOVE theMove;
{
	//int myPosition = thePosition;
	char myPosition[boardSize];
	POSITION retValue;
	generic_hash_unhash(thePosition, myPosition);
	int done = FALSE;
	unsigned int index = theMove >> (32-MVHASHACC+1), whosTurn = generic_hash_turn(thePosition);
	char myPiece = myPosition[index];
	theMove = theMove << MVHASHACC;
	unsigned int nextMove = (theMove >> 30)&0x00000003, previousMove = nextMove;
	unsigned int myForwardRight = forwardRight(index), myForwardLeft = forwardLeft(index), myBackwardRight = backwardRight(index), myBackwardLeft = backwardLeft(index);
	int opposingKing = (whosTurn == P1 ? P2KING : P1KING);
	int opposingMan = (whosTurn == P1 ? P2MAN : P1MAN);
	int isFirstRow = ((index/cols) == 0),
	    isLastRow = ((index/cols) == rows-1);

	if(promoteRow == BACKWARD) {
		if(myPosition[index] == P1MAN) {
			if(isFirstRow) {
				myPosition[index] = P1KING;
				done = TRUE;
			}
		}
		else if(myPosition[index] == P2MAN) {
			if(isLastRow) {
				myPosition[index] = P2KING;
				done = TRUE;
			}
		}
	}
	else if(promoteRow == FORWARD) {
		if(myPosition[index] == P1MAN) {
			if(isLastRow) {
				myPosition[index] = P1KING;
				done = TRUE;
			}
		}
		else if(myPosition[index] == P2MAN) {
			if(isFirstRow) {
				myPosition[index] = P2KING;
				done = TRUE;
			}
		}
	}
	while(!done) {
		myForwardRight = forwardRight(index), myForwardLeft = forwardLeft(index), myBackwardRight = backwardRight(index), myBackwardLeft = backwardLeft(index);
		myPosition[index] = EMPTY;
		switch(nextMove) {
		case FORWARDRIGHT:
			if(myPosition[myForwardRight] == EMPTY) {
				index = myForwardRight;
				done = TRUE;
			}
			else{
				myPosition[forwardRight(myForwardRight)] = myPosition[index];
				if (!demote || (myPosition[myForwardRight] == opposingMan))
					myPosition[myForwardRight] = EMPTY;
				else if (myPosition[myForwardRight] == opposingKing)
					myPosition[myForwardRight] = opposingMan;
				index = forwardRight(myForwardRight);
			}
			break;
		case FORWARDLEFT:
			if(myPosition[myForwardLeft] == EMPTY) {
				index = myForwardLeft;
				done = TRUE;
			}
			else{
				myPosition[forwardLeft(myForwardLeft)] = myPosition[index];
				if (!demote || (myPosition[myForwardLeft] == opposingMan))
					myPosition[myForwardLeft] = EMPTY;
				else if (myPosition[myForwardLeft] == opposingKing)
					myPosition[myForwardLeft] = opposingMan;
				index = forwardLeft(myForwardLeft);
			}
			break;
		case BACKWARDLEFT:
			if(myPosition[myBackwardLeft] == EMPTY) {
				index = myBackwardLeft;
				done = TRUE;
			}
			else{
				myPosition[backwardLeft(myBackwardLeft)] = myPosition[index];
				if (!demote || (myPosition[myBackwardLeft] == opposingMan))
					myPosition[myBackwardLeft] = EMPTY;
				else if (myPosition[myBackwardLeft] == opposingKing)
					myPosition[myBackwardLeft] = opposingMan;
				index = backwardLeft(myBackwardLeft);
			}
			break;
		case BACKWARDRIGHT:
			if(myPosition[myBackwardRight] == EMPTY) {
				index = myBackwardRight;
				done = TRUE;
			}
			else{
				myPosition[backwardRight(backwardRight(index))] = myPosition[index];
				if (!demote || (myPosition[myBackwardRight] == opposingMan))
					myPosition[myBackwardRight] = EMPTY;
				else if (myPosition[myBackwardRight] == opposingKing)
					myPosition[myBackwardRight] = opposingMan;
				index = backwardRight(myBackwardRight);
			}
			break;
		}
		myPosition[index] = myPiece;
		previousMove = nextMove;
		theMove = theMove << 2;
		nextMove = (theMove >> 30)&0x00000003;
		if(oppositeMove(previousMove)==nextMove)
			done = TRUE;
	}

	isFirstRow = ((index/cols) == 0);
	isLastRow = ((index/cols) == rows-1);

	if(promoteRow == FORWARD) {
		if(myPosition[index] == P1MAN) {
			if(isLastRow) {
				myPosition[index] = P1KING;
				done = TRUE;
			}
		}
		else if(myPosition[index] == P2MAN) {
			if(isFirstRow) {
				myPosition[index] = P2KING;
				done = TRUE;
			}
		}
	}

	whosTurn = (whosTurn == P1 ? P2 : P1); //switch players
	retValue = generic_hash_hash(myPosition, whosTurn);
	return retValue;
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
	int i = 0;
	boardSize = rows * cols;
	char* initialPosition = (char*)SafeMalloc(boardSize * sizeof(char));
	char c;
	int currentTurn = -1;
	POSITION initialPositionHash;

	printf("Type in a board representation, using spaces for blank squares and letters for pieces.\n(e.g. 'XOOX    oXox'): ");

	// Prompt for initial position
	getchar();
	while(i < boardSize && (c = getchar()) != EOF) {
		if ((c == P1MAN) || (c == P1KING) ||
		    (c == P2MAN) || (c == P2KING) || (c == EMPTY)) {
			initialPosition[i++] = c; //initialPosition[boardSize - ++i] = c;  // fill from the top down
		} else {
			; //initialPosition[i++] = EMPTY;
		}
	}
	printf("Whose turn will this be? (1 or 2) ");
	//Prompt for turn
	getchar();
	c = getchar();
	if(c == '1') {
		currentTurn = 1;
		printf("Player 1 to play.\n");
	} else if(c == '2') {
		currentTurn = 2;
		printf("Player 2 to play.\n");
	} else {
		currentTurn = 1;
		printf("invalid");
	}

	initialPositionHash = generic_hash_hash(initialPosition, currentTurn);
	//generic_hash_unhash(initialPositionHash,initialPosition);
	SafeFree(initialPosition);

	gInitialPosition = initialPositionHash;
	return initialPositionHash;
	/*
	   int i = 0;
	   boardSize = rows * cols;

	   char* initialPosition = (char*)SafeMalloc(boardSize * sizeof(char));
	   char c;
	   POSITION initialPositionHash;

	   printf("Type in a board representation, using spaces for blank squares and letters for pieces.\n(e.g. 'GOOg    oGoG'): ");

	   // Prompt for initial position
	   getchar();
	   while(i < boardSize && (c = getchar()) != EOF) {
	    if ((c == P1MAN) || (c == P1KING) ||
	            (c == P2MAN) || (c == P2KING) || (c == EMPTY)) {
	        initialPosition[boardSize - ++i] = c;  // fill from the top down
	            } else {
	                ;//initialPosition[i++] = EMPTY;
	            }
	   }

	   initialPositionHash = generic_hash_hash(initialPosition, P1);
	   SafeFree(initialPosition);

	   gInitialPosition = initialPositionHash;

	   //curBoard = gInitialPosition;
	   return initialPositionHash;*/
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
	// TODO
}

/************************************************************************
**
** NAME:        CountPieces
**
** DESCRIPTION: Count how many pieces each player has.
**
** INPUTS:      char board[]: the board to inspect
**              unsigned int *p1Pieces: holds number of pieces Player 1 has
**              unsigned int *p2Pieces: holds number of pieces Player 2 has
**
** OUTPUTS:     total number of pieces
**
************************************************************************/

unsigned int CountPieces(char board[],
                         unsigned int *p1Pieces, unsigned int *p2Pieces) {
	unsigned int i;

	*p1Pieces = *p2Pieces = 0;

	// Check if one player has no more pieces
	for (i = 0; i < boardSize; i++) {
		switch (board[i]) {
		case P1KING:
		case P1MAN:
			(*p1Pieces)++;
			break;
		case P2KING:
		case P2MAN:
			(*p2Pieces)++;
		}
	}

	return *p1Pieces + *p2Pieces;
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
**              We assume Player 1 is always the human
**              No more pieces: primitive lose
**              All Locked pieces: primitive lose
**              Else: undecided
**
************************************************************************/

VALUE Primitive(position)
POSITION position;
{
	char board[boardSize];
	unsigned int p1Pieces, p2Pieces;
	int whosTurn = generic_hash_turn(position);
	// Check for no more pieces
	CountPieces(board, &p1Pieces, &p2Pieces);
	if(whosTurn == P1) {
		if (&p1Pieces == 0) return (gStandardGame ? lose : win); // Player 1 has no more pieces
	}
	else if(&p2Pieces == 0) return (gStandardGame ? lose : win); // P2 has no more pieces

	// TODO: Check for all pieces being locked (unable to move)
	if(GenerateMoves(position) == NULL)
		return (gStandardGame ? lose : win);
	return undecided;
	/*    char board[boardSize];
	   unsigned int p1Pieces, p2Pieces;
	   int whosTurn = generic_hash_turn(position);
	   // Check for no more pieces
	   CountPieces(board, &p1Pieces, &p2Pieces);
	   if(whosTurn == P1){
	   if (&p1Pieces == 0) return lose;  // Player 1 has no more pieces
	   }
	   else if(&p2Pieces == 0) return lose;// P2 has no more pieces

	   // TODO: Check for all pieces being locked (unable to move)
	   if(GenerateMoves(position) == NULL)
	   return lose; //undecided;
	 */
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
************************************************************************/

void PrintPosition(position,playerName,usersTurn)
POSITION position;
STRING playerName;
BOOLEAN usersTurn;
{
	char board[boardSize];
	int player;
	unsigned int i, j, k = 0;
	unsigned int p1Pieces, p2Pieces;
	int swapRow = 0;

	generic_hash_unhash(position, board); // Obtain board state
	player = generic_hash_turn(position);


	printf("\nRUBIK'S CHECKERS\n");

	CountPieces(board, &p1Pieces, &p2Pieces);

	if (TRUE) { //player == P1) {
		// Print column letters **POSSIBLE OVERFLOW**
		//for (i = 0; i < (cols*2); i++) {
		//    printf("%c ", 'a' + i);
		//}

		// Row separators
		printf("\n +");
		for (j = 0; j < (cols*2); j++) printf("-+");
		printf("\n");

		k = 0; //boardSize;
		for (i = rows; i > 0; i--) {
			printf("%d|", i); // Row number
			if ((swapRow % 2) != 0) printf("%c|", EMPTY); // Shift alternating rows
			for (j = 0; j < cols; j++) {
				// Print square plus a column separator
				printf("%c|", board[k++]); //--k

				// Print empty squares in between plus a column separator
				if ((j != (cols-1)) || (swapRow % 2) == 0) printf("%c|", EMPTY);
			}

			if (i == 1) { // Print player 2's number of pieces
				printf("    %4s: %d pieces", P2NAME, p2Pieces);
				if (player == P2) printf(" (%s's turn)", playerName);
			} else if (i == rows) { // Print player 1's number of pieces
				printf("    %4s: %d pieces", P1NAME, p1Pieces);
				if (player == P1) printf(" (%s's turn)", playerName);
			}

			// Row separators
			printf("\n +");
			for (j = 0; j < (cols*2); j++) printf("-+");
			printf("\n");

			swapRow++;

		}

		// Print column letters **POSSIBLE OVERFLOW**
		printf("  ");
		for (i = 0; i < (cols*2); i++) {
			printf("%c ", 'a' + i);
		}
		printf("\n");

	} else if (player == P2) { // Swap board
		// Print column letters **POSSIBLE OVERFLOW**
		for (i = 0; i < (cols*2); i++) {
			printf("%c ", 'a' + (cols*2 - i - 1));
		}

		// Row separators
		printf("\n +");
		for (j = 0; j < (cols*2); j++) printf("-+");
		printf("\n");

		k = boardSize;
		for (i = rows; i > 0; i--) {
			printf("%d|", rows - i + 1); // Row number
			if ((i % 2) != 0) printf("%c|", EMPTY); // Shift alternating rows
			for (j = 0; j < cols; j++) {
				// Print square plus a column separator
				printf("%c|", board[--k]); //--k

				// Print empty squares in between plus a column separator
				if ((j != (cols-1)) || (i % 2) == 0) printf("%c|", EMPTY);
			}

			if (i == rows) { // Print player 2's number of pieces
				printf("    %8s: %d pieces", P2NAME, p2Pieces);
				if (player == P2) printf(" (%s's turn)", P2NAME);
			} else if (i == 1) { // Print player 1's number of pieces
				printf("    %8s: %d pieces", P1NAME, p1Pieces);
				if (player == P1) printf(" (%s's turn)", P1NAME);
			}

			// Row separators
			printf("\n +");
			for (j = 0; j < (cols*2); j++) printf("-+");
			printf("\n");
		}
	} else BadElse("PrintPosition");

	if (gPrintPredictions && (!gUnsolved)) {
		printf("\n%s\n\n",GetPrediction(position,playerName,usersTurn));
	} else {
		printf("\n\n");
	}
}




/************************************************************************
**
** NAME:        makeMove
**
** DESCRIPTION: Finds and creates integer representations of moves(not captures),
**              and appends these moves to the list.
**
**              The first n=MVHASHACC bits represent the index, and the rest of the
**              bits represent, in pairs of bits, the directions of movement.
**              Move termination is represented by a repeat of the previous move.
**
** INPUTS:      char *initialPosition : The current board.
**              int whosTurn          : Self-explanatory, P1 or P2.
**              int currentIndex      : Index of the piece to be moved.
**              MOVELIST *head        : The list to be added to.
** OUTPUTS:     NONE
**
** CALLS:
**
************************************************************************/
//currentIndex
MOVELIST *makeMove(char *initialPosition, int whosTurn, int currentIndex, MOVELIST *head){
	unsigned int myMove = (currentIndex << (32-MVHASHACC+1));
	int currentMobility;

	if(initialPosition[currentIndex] == P1KING)
		currentMobility = kingMobility;
	else if(initialPosition[currentIndex] == P2KING)
		currentMobility = reverseMobility(kingMobility);
	else if(initialPosition[currentIndex] == P1MAN)
		currentMobility = manMobility;
	else if(initialPosition[currentIndex] == P2MAN)
		currentMobility = reverseMobility(manMobility);
	else BadElse("makeMove");

	if((currentMobility&FORWARD) == FORWARD) {
		if(forwardLeft(currentIndex)!= -1 && initialPosition[forwardLeft(currentIndex)] == EMPTY)
			head = CreateMovelistNode(myMove|(0<<(32-MVHASHACC))|(FORWARDLEFT<<(32-MVHASHACC-2))|(BACKWARDRIGHT<<(32-MVHASHACC-4)), head);
		if(forwardRight(currentIndex)!= -1 && initialPosition[forwardRight(currentIndex)] == EMPTY)
			head = CreateMovelistNode(myMove|(0<<(32-MVHASHACC))|(FORWARDRIGHT<<(32-MVHASHACC-2))|(BACKWARDLEFT<<(32-MVHASHACC-4)), head);
	}
	if((currentMobility&BACKWARD) == BACKWARD) {
		if(backwardLeft(currentIndex)!= -1 && initialPosition[backwardLeft(currentIndex)] == EMPTY)
			head = CreateMovelistNode(myMove|(0<<(32-MVHASHACC))|(BACKWARDLEFT<<(32-MVHASHACC-2))|(FORWARDRIGHT<<(32-MVHASHACC-4)), head);
		if(backwardRight(currentIndex)!= -1 && initialPosition[backwardRight(currentIndex)] == EMPTY)
			head = CreateMovelistNode(myMove|(0<<(32-MVHASHACC))|(BACKWARDRIGHT<<(32-MVHASHACC-2))|(FORWARDLEFT<<(32-MVHASHACC-4)), head);
	}
	return head;
}





//traverse board recursively, adding moves to currentMove.  When cannot find more moves, add currentMove to MOVELIST
//and then undo changes to initialPosition
MOVELIST *makeCapture(char *initialPosition, int whosTurn, int currentIndex, MOVELIST *head, unsigned int currentMove, int offset, unsigned int currentMobility){
	int previousMove;
	int opposingMan, opposingKing;
	int myForwardRight = forwardRight(currentIndex),
	    myForwardLeft = forwardLeft(currentIndex),
	    myBackwardRight = backwardRight(currentIndex),
	    myBackwardLeft = backwardLeft(currentIndex);
	MOVELIST *oldHead = head;

	char tmp[boardSize];

	if (offset == 2) {
		previousMove = -1;
	} else {
		previousMove = (currentMove<<(MVHASHACC+offset-4))>>30;
	}

	strncpy(tmp, initialPosition, boardSize);

	if(whosTurn == P1) {
		opposingMan = P2MAN;
		opposingKing = P2KING;
	}
	else if(whosTurn == P2) {
		opposingMan = P1MAN;
		opposingKing = P1KING;
	}
	else
		BadElse("makeCapture");

	if((currentMobility&CAPTURE) == CAPTURE) {
		if((currentMobility&FORWARD) == FORWARD) {
			if(myForwardLeft!=-1 && ((initialPosition[myForwardLeft]==opposingMan)||(initialPosition[myForwardLeft]==opposingKing))) {
				if(forwardLeft(myForwardLeft)!=-1 && initialPosition[forwardLeft(myForwardLeft)]==EMPTY) {
					//add board changing: if(opposingMan), change to empty...
					if(initialPosition[myForwardLeft] == opposingMan)
						initialPosition[myForwardLeft] = EMPTY;
					else if (initialPosition[myForwardLeft] == opposingKing)
						initialPosition[myForwardLeft] = opposingMan;
					else BadElse("makeCapture");

					if(previousMove != BACKWARDRIGHT)
						head = makeCapture(initialPosition, whosTurn,
						                   forwardLeft(myForwardLeft),
						                   head, currentMove|(FORWARDLEFT<<(32-MVHASHACC-offset)),
						                   offset+2, currentMobility);
					strncpy(initialPosition, tmp, boardSize);
				}
			}
			if(myForwardRight!=-1 && ((initialPosition[myForwardRight]==opposingMan)||(initialPosition[myForwardRight]==opposingKing))) {
				if(forwardRight(myForwardRight)!=-1 && initialPosition[forwardRight(myForwardRight)]==EMPTY) {
					if(initialPosition[myForwardRight] == opposingMan)
						initialPosition[myForwardRight] = EMPTY;
					else if (initialPosition[myForwardRight] == opposingKing)
						initialPosition[myForwardRight] = opposingMan;
					else BadElse("makeCapture");

					if(previousMove != BACKWARDLEFT)
						head = makeCapture(initialPosition, whosTurn,
						                   forwardRight(myForwardRight),
						                   head, currentMove|(FORWARDRIGHT<<(32-MVHASHACC-offset)),
						                   offset+2, currentMobility);
					strncpy(initialPosition, tmp, boardSize);
				}
			}
		}
		if((currentMobility&BACKWARD) == BACKWARD) {
			if(myBackwardRight!=-1 && ((initialPosition[myBackwardRight]==opposingMan)||(initialPosition[myBackwardRight]==opposingKing))) {
				if(backwardRight(myBackwardRight)!=-1 && initialPosition[backwardRight(myBackwardRight)]==EMPTY) {
					if(initialPosition[myBackwardRight] == opposingMan)
						initialPosition[myBackwardRight] = EMPTY;
					else if (initialPosition[myBackwardRight] == opposingKing)
						initialPosition[myBackwardRight] = opposingMan;
					else BadElse("makeCapture");

					if(previousMove != FORWARDLEFT)
						head = makeCapture(initialPosition, whosTurn,
						                   backwardRight(myBackwardRight),
						                   head, currentMove|(BACKWARDRIGHT<<(32-MVHASHACC-offset)),
						                   offset+2, currentMobility);
					strncpy(initialPosition, tmp, boardSize);
				}
			}
			if(myBackwardLeft!=-1 && ((initialPosition[myBackwardLeft]==opposingMan)||(initialPosition[myBackwardLeft]==opposingKing))) {
				if(backwardLeft(myBackwardLeft)!=-1 && initialPosition[backwardLeft(myBackwardLeft)]==EMPTY) {
					if(initialPosition[myBackwardLeft] == opposingMan)
						initialPosition[myBackwardLeft] = EMPTY;
					else if (initialPosition[myBackwardLeft] == opposingKing)
						initialPosition[myBackwardLeft] = opposingMan;
					else BadElse("makeCapture");

					if(previousMove != FORWARDRIGHT)
						head = makeCapture(initialPosition, whosTurn,
						                   backwardLeft(myBackwardLeft),
						                   head, currentMove|(BACKWARDLEFT<<(32-MVHASHACC-offset)),
						                   offset+2, currentMobility);
					strncpy(initialPosition, tmp, boardSize);
				}
			}
		}
	}
	if (offset>2) {
		//strncpy(tmp, initialPosition,boardSize);
		//undoCapture(initialPosition, whosTurn, currentIndex, previousMove);
		if (!forceCapture || (head == oldHead)) {
			head = CreateMovelistNode(currentMove|(1<<(32-MVHASHACC))|(oppositeMove(previousMove)<<(32-MVHASHACC-offset)), head);
		}
		//1 represents capture
	}
	return head;
}


MOVELIST *makePromote(char *initialPosition, int whosTurn, int currentIndex, MOVELIST *head){
	int isFirstRow = ((currentIndex/cols) == 0),
	    isLastRow = ((currentIndex/cols) == rows-1);
	if(promoteRow == BACKWARD) {
		if(initialPosition[currentIndex] == P1MAN) {
			if(isFirstRow)
				head = CreateMovelistNode((currentIndex<<(32-MVHASHACC+1))|(1<<(32-MVHASHACC)), head);
		}
		else if(initialPosition[currentIndex] == P2MAN) {
			if(isLastRow)
				head = CreateMovelistNode((currentIndex<<(32-MVHASHACC+1))|(1<<(32-MVHASHACC)), head);
		}
	}
	else if(promoteRow == FORWARD) {
		if(initialPosition[currentIndex] == P1MAN) {
			if(isLastRow)
				head = CreateMovelistNode((currentIndex<<(32-MVHASHACC+1))|(1<<(32-MVHASHACC)), head);
		}
		else if(initialPosition[currentIndex] == P2MAN) {
			if(isFirstRow)
				head = CreateMovelistNode((currentIndex<<(32-MVHASHACC+1))|(1<<(32-MVHASHACC)), head);
		}
	}
	return head;
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

//need to write makePromote
//
MOVELIST *GenerateMoves(position)
POSITION position;
{
	MOVELIST *head = NULL, *oldHead = NULL;
	MOVELIST *CreateMovelistNode();
	char initialPosition[boardSize];
	//char tempPositions[boardSize];

	int whosTurn = generic_hash_turn(position);
	char currentKing, currentMan;
	int i;
	int hasCaptures = 0;

	generic_hash_unhash(position, initialPosition);
	if(whosTurn == P1) {
		currentKing = P1KING;
		currentMan = P1MAN;
	}
	else{
		currentKing = P2KING;
		currentMan = P2MAN;
	}
	for(i = 0; i < boardSize; i++) {
		if((initialPosition[i] == currentKing)) {
			if(whosTurn == P1)
				head = makeCapture(initialPosition, whosTurn, i, head, (i<<(32-MVHASHACC+1)), 2, kingMobility);
			else if (whosTurn == P2)
				head = makeCapture(initialPosition, whosTurn, i, head, (i<<(32-MVHASHACC+1)), 2, reverseMobility(kingMobility));

			if (head != oldHead) hasCaptures = 1;
			if (!forceCapture || !hasCaptures) {
				head = makeMove(initialPosition, whosTurn, i, head);
				head = makePromote(initialPosition, whosTurn, i, head);
			}
		}
		else if (initialPosition[i] == currentMan) {
			if(whosTurn == P1)
				head = makeCapture(initialPosition, whosTurn, i, head, (i<<(32-MVHASHACC+1)), 2, manMobility);
			else if(whosTurn == P2)
				head = makeCapture(initialPosition, whosTurn, i, head, (i<<(32-MVHASHACC+1)), 2, reverseMobility(manMobility));

			if (head != oldHead) hasCaptures = 1;
			if (!forceCapture || !hasCaptures) {
				head = makeMove(initialPosition, whosTurn, i, head);
				head = makePromote(initialPosition, whosTurn, i, head);
			}
		}

		oldHead = head;
	}
	return head;
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

USERINPUT GetAndPrintPlayersMove(thePosition, theMove, playerName)
POSITION thePosition;
MOVE *theMove;
STRING playerName;
{
	USERINPUT ret, HandleDefaultTextInput();
	BOOLEAN ValidMove();
	char input[2];
	// TODO

	input[0] = '3';
	do {
		printf("%8s's move [ (u)ndo/(a-%c)(1-%d)[(a-%c)(1-%d)]* ] : ", playerName, 'a' - 1 + cols*2, rows, 'a' - 1 + cols*2, rows);
		ret = HandleDefaultTextInput(thePosition, theMove, playerName);
		if(ret != Continue)
			return(ret);
	}
	while (TRUE);
	return(Continue); /* this is never reached, but lint is now happy */
}

int getIndexFromText(char currentRow, char currentCol){
	return (rows - (currentRow-'0'))*cols + (currentCol-'a')/2;
}

void getTextFromIndex(int index, char* pos) {
	int row = 0, col = 0;

	row = rows -(index / cols);
	col = (index % cols) * 2;

	// Stagger each other row
	if ((rows-row) % 2 == 1) col++;

	pos[0] = 'a' + col;
	pos[1] = '0' + row;
	pos[2] = 0;
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
	// TODO
	unsigned int i = 1, myIndex;
	char currentRow = input[1], currentCol = input[0];
	myIndex = getIndexFromText(currentRow, currentCol);
	int isEvenRow = (((rows+'0'- currentRow))%2 == 0),
	    isEvenCol = ((currentCol - 'a')%2 == 0);


	if(currentCol == 0)
		return(FALSE);
	if(currentRow == 0)
		return(FALSE);
	if(!isEvenRow && isEvenCol)
		return(FALSE);
	if(isEvenRow && !isEvenCol)
		return(FALSE);

	while(input[i-1]!=0 && input[i]!=0) {
		currentRow = input[i];
		currentCol = input[i-1];
		myIndex = getIndexFromText(currentRow, currentCol);
		isEvenRow = (((rows+'0')-currentRow)%2 == 0);
		isEvenCol = ((currentCol - 'a')%2 == 0);

		if(currentRow-'1'>rows || (currentCol-'a')/2>cols)
			return(FALSE);
		if(currentRow-'1'<0 || currentCol-'a'<0)
			return(FALSE);
		//printf("%d\n", myIndex);
		if(myIndex<0 || myIndex>boardSize)
			return(FALSE);
		i= i+2;
		if(!isEvenRow && isEvenCol)
			return(FALSE);
		if(isEvenRow && !isEvenCol)
			return(FALSE);
	}
	return(TRUE);
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

//NO ERROR CHECKING YET
//write
MOVE ConvertTextInputToMove(input)
STRING input;
{
	int i = 2;
	char currentRow = input[1], currentCol = input[0];
	unsigned int myMove, currentIndex, nextIndex;
	int previousMove = -1;
	myMove = currentIndex = getIndexFromText(currentRow, currentCol);
	myMove = myMove<<(32-MVHASHACC+1);
	currentCol = input[i];
	currentRow = input[i+1];
	nextIndex = getIndexFromText(currentRow, currentCol);
	if(nextIndex == forwardRight(currentIndex)) {
		myMove = myMove|(FORWARDRIGHT<<(32-MVHASHACC-i));
		previousMove = FORWARDRIGHT;
	}
	else if(nextIndex == forwardLeft(currentIndex)) {
		myMove = myMove|(FORWARDLEFT<<(32-MVHASHACC-i));
		previousMove = FORWARDLEFT;
	}
	else if(nextIndex == backwardLeft(currentIndex)) {
		myMove = myMove|(BACKWARDLEFT<<(32-MVHASHACC-i));
		previousMove = BACKWARDLEFT;
	}
	else if(nextIndex == backwardRight(currentIndex)) {
		myMove = myMove|(BACKWARDRIGHT<<(32-MVHASHACC-i));
		previousMove = BACKWARDRIGHT;
	}
	i = i + 2;
	if(previousMove != -1)
		myMove = myMove|(0<<(32-MVHASHACC));
	if(previousMove == -1) {
		myMove = myMove|(1<<(32-MVHASHACC));
		i = 2;
		while(input[i]!=0) {
			currentCol = input[i];
			currentRow = input[i+1];
			nextIndex = getIndexFromText(currentRow, currentCol);
			if(forwardRight(currentIndex)!=-1 &&
			   nextIndex == forwardRight(forwardRight(currentIndex))) {
				myMove = myMove|(FORWARDRIGHT<<(32-MVHASHACC-i));
				previousMove = FORWARDRIGHT;
			}
			else if(forwardLeft(currentIndex)!=-1 &&
			        nextIndex == forwardLeft(forwardLeft(currentIndex))) {
				myMove = myMove|(FORWARDLEFT<<(32-MVHASHACC-i));
				previousMove = FORWARDLEFT;
			}
			else if(backwardLeft(currentIndex)!=-1 &&
			        nextIndex == backwardLeft(backwardLeft(currentIndex))) {
				myMove = myMove|(BACKWARDLEFT<<(32-MVHASHACC-i));
				previousMove = BACKWARDLEFT;
			}
			else if(backwardRight(currentIndex)!=-1 &&
			        nextIndex == backwardRight(backwardRight(currentIndex))) {
				myMove = myMove|(BACKWARDRIGHT<<(32-MVHASHACC-i));
				previousMove = BACKWARDRIGHT;
			}
			i=i+2;
			//printf("%d", previousMove);
			currentIndex = nextIndex;
		}
	}
	if(previousMove != -1)
		myMove = myMove|(oppositeMove(previousMove)<<(32-MVHASHACC-(i)));
	return(myMove);
}

int canPromote(int index, POSITION position) {
	char board[boardSize];
	int player;
	int row = rows - (index / cols);

	generic_hash_unhash(position, board); // Obtain board state
	player = generic_hash_turn(position);

	if ((board[index] == P1KING) || (board[index] == P2KING)) return FALSE;

	if (promoteRow == BACKWARD) {
		if (player == P1) {
			return (row == rows);
		} else if (player == P2) {
			return (row == 1);
		} else BadElse("canPromote()");
	} else if (promoteRow == FORWARD) {
		if (player == P1) {
			return (row == 1);
		} else if (player == P2) {
			return (row == rows);
		} else BadElse("canPromote()");
	}

	return FALSE;
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
	STRING s = (STRING) SafeMalloc(100); // replace with exact #
	s[0] = 0; // Initialize string to null

	unsigned int move = theMove;
	unsigned int startIndex = (move >> (32-MVHASHACC+1));
	unsigned int currentIndex = startIndex;
	unsigned int currentMove = 0, previousMove = 0;
	char startSq[3], nextSq[3];
	unsigned int maxJumps = (32-MVHASHACC)/2, i = 0;
	int done = FALSE, jump = FALSE;

	//generic_hash_unhash(curBoard, myBoard);
	// Print initial square
	getTextFromIndex(startIndex, startSq);
	strcat(s, startSq);
	move = move << MVHASHACC;

	if(((theMove>>(32-MVHASHACC))&0x00000001) == 1)
		jump = TRUE;
	else{
		//getTextFromIndex(currentIndex, nextSq);
		//sprintf(s, "%s", nextSq);
		currentMove = move>>30;
		switch(currentMove) {
		case FORWARDLEFT:
			currentIndex = forwardLeft(currentIndex);
			break;
		case FORWARDRIGHT:
			currentIndex = forwardRight(currentIndex);
			break;
		case BACKWARDLEFT:
			currentIndex = backwardLeft(currentIndex);
			break;
		case BACKWARDRIGHT:
			currentIndex = backwardRight(currentIndex);
			break;
		default:
			BadElse("PrintMove");
		}
		getTextFromIndex(currentIndex, nextSq);
		strcat(s, nextSq);
	}

	//if (canPromote(startIndex, curBoard)) return;

	// Loop through jumps
	if ((jump == TRUE) && (move != 0)) {

		for (i = 0; (i < maxJumps) && !done; i++) {
			previousMove = currentMove;
			currentMove = (move >> 30)&0x00000003;
			if ((currentMove == oppositeMove(previousMove)) && (i != 0)) {
				done = TRUE;
			} else {
				//sprintf(s, "%d", currentIndex);
				switch (currentMove) {
				case FORWARDLEFT:
					currentIndex = forwardLeft(forwardLeft(currentIndex));
					currentMove = FORWARDLEFT;
					break;
				case FORWARDRIGHT:
					currentIndex = forwardRight(forwardRight(currentIndex));
					currentMove = FORWARDRIGHT;
					break;
				case BACKWARDRIGHT:
					//sprintf(s, "~%d%d%c~", currentTurn, currentIndex, myBoard[backwardRight(currentTurn, currentIndex)]);
					currentIndex = backwardRight(backwardRight(currentIndex));
					currentMove = BACKWARDRIGHT;
					break;
				case BACKWARDLEFT:
					currentIndex = backwardLeft(backwardLeft(currentIndex));
					currentMove = BACKWARDLEFT;
					break;
				default:
					BadElse("PrintMove");
				}
				getTextFromIndex(currentIndex, nextSq);
				strcat(s, nextSq);
			}

			move = move << 2;
		}
	}

	return s;
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
	/*  unsigned int currentMove, previousMove, counter=0, done = FALSE;
	   char *myMove = (char *)malloc((32-MVHASHACC)/2*sizeof(char));
	   printf("(%d ", theMove>>(32-MVHASHACC)), counter = 0;
	   theMove = theMove<<MVHASHACC;
	   currentMove = previousMove = theMove>>30;
	   while(counter<(32-MVHASHACC)/2 && !done){
	   previousMove = currentMove;
	   currentMove = (theMove>>30)&0x00000003;
	   if(currentMove != oppositeMove(previousMove))
	    myMove[counter] = currentMove+'0';
	   else
	    done = TRUE;
	   theMove = theMove<<2;
	   counter++;
	   }
	   myMove[counter-1] = 0;
	   if(counter<=(32-MVHASHACC)/2)
	   printf("%s", myMove);
	   printf(")");
	   SafeFree(myMove);
	 */ /*
	   unsigned int move = theMove;
	   unsigned int startIndex = (move >> (32-MVHASHACC+1));
	   unsigned int currentIndex = startIndex;
	   unsigned int currentMove = 0, previousMove = 0;
	   char startSq[3], nextSq[3];
	   unsigned int maxJumps = (32-MVHASHACC)/2, i = 0;
	   int done = FALSE, jump = FALSE;

	   //generic_hash_unhash(curBoard, myBoard);
	   // Print initial square
	   getTextFromIndex(startIndex, startSq);
	   printf("%s", startSq);
	   move = move << MVHASHACC;

	   if(((theMove>>(32-MVHASHACC))&0x00000001) == 1)
	    jump = TRUE;
	   else{
	    //getTextFromIndex(currentIndex, nextSq);
	    //printf("%s", nextSq);
	    currentMove = move>>30;
	    switch(currentMove){
	    case FORWARDLEFT:
	      currentIndex = forwardLeft(currentIndex);
	      break;
	    case FORWARDRIGHT:
	      currentIndex = forwardRight(currentIndex);
	      break;
	    case BACKWARDLEFT:
	      currentIndex = backwardLeft(currentIndex);
	      break;
	    case BACKWARDRIGHT:
	      currentIndex = backwardRight(currentIndex);
	      break;
	    default:
	      BadElse("PrintMove");
	    }
	    getTextFromIndex(currentIndex, nextSq);
	    printf("%s", nextSq);
	   }

	   //if (canPromote(startIndex, curBoard)) return;

	   // Loop through jumps
	   if(jump == TRUE){
	      if (move == 0) return;

	   for (i = 0; (i < maxJumps) && !done; i++) {
	      previousMove = currentMove;
	      currentMove = (move >> 30)&0x00000003;
	      if ((currentMove == oppositeMove(previousMove)) && (i != 0)) {
	          done = TRUE;
	      } else {
	        //printf("%d", currentIndex);
	          switch (currentMove) {
	              case FORWARDLEFT:
	                  currentIndex = forwardLeft(forwardLeft(currentIndex));
	                  currentMove = FORWARDLEFT;
	                break;
	              case FORWARDRIGHT:
	                  currentIndex = forwardRight(forwardRight(currentIndex));
	                  currentMove = FORWARDRIGHT;
	                break;
	              case BACKWARDRIGHT:
	                //printf("~%d%d%c~", currentTurn, currentIndex, myBoard[backwardRight(currentTurn, currentIndex)]);
	                  currentIndex = backwardRight(backwardRight(currentIndex));
	                  currentMove = BACKWARDRIGHT;
	                break;
	              case BACKWARDLEFT:
	                  currentIndex = backwardLeft(backwardLeft(currentIndex));
	                  currentMove = BACKWARDLEFT;
	                break;
	          default:
	                BadElse("PrintMove");
	          }
	          getTextFromIndex(currentIndex, nextSq);
	          printf("%s", nextSq);
	      }

	      move = move << 2;
	      }
	   }
	   // TODO */

	STRING s = MoveToString(theMove);
	printf("%s",s);
	SafeFree(s);
}


STRING kDBName = "Rubik's Checkers";

int NumberOfOptions()
{
	// TODO
	return 1024;
}

int getOption()
{
	int extract;
	if(gStandardGame)
		extract = 1;
	else
		extract = 0;
	extract = extract << 9;
	extract = extract | (demote << 8);
	extract = extract | (startRows << 6);
	extract = extract | (rows << 3);
	extract = extract | (cols);
	return extract;
}

void setOption(int option)
{
	int extract = (option >> 9) & 1;
	if(extract == 1) {
		gStandardGame = 0;
	}else{
		gStandardGame = 1;
	}
	extract = (option >> 8) & 1;
	if (!extract) { // Switch to Checkers
		demote = FALSE;
		manMobility = FORWARD | CAPTURE;
		forceCapture = TRUE;
		startPromoted = FALSE;
		promoteRow = FORWARD;
	} else { // Switch to Rubik's
		demote = TRUE;
		manMobility = BACKWARD;
		forceCapture = FALSE;
		startPromoted = TRUE;
		promoteRow = BACKWARD;
	}
	extract = (option >> 6) & 3;
	startRows = extract;
	extract = (option >> 3) & 7;
	rows = extract;
	extract = option & 7;
	cols = extract;
}
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
