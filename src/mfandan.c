/************************************************************************
**
** NAME:        mfandan.c
**
** DESCRIPTION: Fandango
**
** AUTHOR:
**
** TODO:
**
** DATE:        02/28/04
**
**************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/

#include <stdio.h>
#include "gamesman.h"
#include "hash.h"
// #include "loopygasolver.h"

int debug = 0;
int printMethods = 0;

extern STRING gValueString[];

POSITION gNumberOfPositions  = 0;     /* don't initialize to anything interesting yet, let generic_hash_init(...) decide */
POSITION kBadPosition        = -1;

POSITION gInitialPosition    =  0;
POSITION gMinimalPosition    =  0;

STRING kAuthorName         = "Gamescrafters";
STRING kGameName           = "Fandango";
BOOLEAN kPartizan           = TRUE;
BOOLEAN kDebugMenu          = TRUE;
BOOLEAN kGameSpecificMenu   = TRUE;
BOOLEAN kTieIsPossible      = FALSE;
BOOLEAN kLoopy               = TRUE;
BOOLEAN kDebugDetermineValue = FALSE;

STRING kHelpGraphicInterface ="";

STRING kHelpTextInterface    =
        "On your turn use the file letter and row number to determine the piece you wish\n\
to move, the direction and attack method. A move consists of moving a piece to an\n\
empty adjacent spot. You can capture either by approach or withdrawal. If you move\n\
towards the opponent, so that your piece is next to the opposing piece, then you have\n\
captured by approach. Any pieces that are in the same line are also captured unless they\n\
are separated by empty spaces or the opponent's piece. To capture by withdrawal, a piece\n\
must be moved away from the opponent's piece. This will capture any pieces that are in a\n\
continuous line with the captured piece. Any captured pieces are removed from the board\n\
If you have made a wrong move at any point,you can type u to revert back to the previous state.\n\
"                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  ;


STRING kHelpOnYourTurn =
        "Move one of your pieces in one of the eight directions by selecting the letter and row\n"
        "number,direction, and attack method. Capturing can be done by withdrawal or approach. To\n"
        "capture by approach the player must move towards the opponent piece so that it is\n"
        "adjacent to the piece. Any enemy pieces that are in a continuous line with the piece\n"
        "are also captured. To capture by withdrawal, move a piece that is adjacent to the\n"
        "opponent's piece away from the opposing piece.";


STRING kHelpStandardObjective =
        "To capture all of your opponents pieces or to leave them in a position where they have\n"
        "no more available moves.";


STRING kHelpReverseObjective =
        "To get all of your pieces captured.\n";


STRING kHelpTieOccursWhen ="";

STRING kHelpExample =
        "            BOARD \n\n\
        A     B     C          DIRECTION        ACTION\n\n\
   1    O --- O --- O          NW  N  NE        N = no action \n\
        |  \\  |  /  |            \\ | / \n\
   2    X --- . --- O          W - * - E        A = capture by approach\n\
        |  /  |  \\  |            / | \\ \n\
   3    X --- X --- X          SW  S  SE        W = capture by withdraw \n\n\n\n\
(Player should Win in 11)  \n\
  Player\'s move [(u)ndo/1-9] :   b3 n a\n\n\
            BOARD\n\n\
        A     B     C          DIRECTION        ACTION\n\n\
   1    O --- . --- O          NW  N  NE        N = no action\n\
        |  \\  |  /  |            \\ | / \n\
   2    X --- X --- O          W - * - E        A = capture by approach \n\
        |  /  |  \\  |            / | \\ \n\
   3    X --- . --- X          SW  S  SE        W = capture by withdraw \n\n\n\n\
(Computer should Lose in 12)  \n\
Computer\'s move              : 17 \n\n\n\
            BOARD\n\n\
        A     B     C          DIRECTION        ACTION\n\n\
   1    . --- O --- O          NW  N  NE        N = no action\n\
        |  \\  |  /  |            \\ | / \n\
   2    X --- X --- O          W - * - E        A = capture by approach\n\
        |  /  |  \\  |            / | \\\n\
   3    X --- . --- X          SW  S  SE        W = capture by withdraw\n\n\n\n\
(Player should Win in 11)  \n\
  Player\'s move [(u)ndo/1-9] : c3 w n\n\n\n\n\
            BOARD\n\n\
        A     B     C          DIRECTION        ACTION\n\n\
   1    . --- O --- O          NW  N  NE        N = no action\n\
        |  \\  |  /  |            \\ | / \n\
   2    X --- X --- O          W - * - E        A = capture by approach\n\
        |  /  |  \\  |            / | \\\n\
   3    X --- X --- .          SW  S  SE        W = capture by withdraw\n\n\n\n\
(Computer should Lose in 14)  \n\
Computer\'s move              : 45\n\n\n\
            BOARD\n\n\
        A     B     C          DIRECTION        ACTION \n\n\
   1    O --- . --- O          NW  N  NE        N = no action \n\
        |  \\  |  /  |            \\ | / \n\
   2    X --- X --- O          W - * - E        A = capture by approach \n\
        |  /  |  \\  |            / | \\ \n\
   3    X --- X --- .          SW  S  SE        W = capture by withdraw \n\n\n\n\
(Player should Win in 13)  \n\
  Player\'s move [(u)ndo/1-9] : b2 se w\n\n\n\
            BOARD\n\n\
        A     B     C          DIRECTION        ACTION\n\n\
   1    . --- . --- O          NW  N  NE        N = no action\n\
        |  \\  |  /  |            \\ | / \n\
   2    X --- . --- O          W - * - E        A = capture by approach\n\
        |  /  |  \\  |            / | \\\n\
   3    X --- X --- X          SW  S  SE        W = capture by withdraw\n\n\n\n\
(Computer should draw)  \n\
Computer\'s move              : 66 \n\n\n\
            BOARD\n\n\
        A     B     C          DIRECTION        ACTION\n\n\
   1    . --- . --- .          NW  N  NE        N = no action \n\
        |  \\  |  /  |            \\ | / \n\
   2    X --- O --- O          W - * - E        A = capture by approach \n\
        |  /  |  \\  |            / | \\ \n\
   3    . --- X --- X          SW  S  SE        W = capture by withdraw \n\n\n\n\
(Player should draw)  \n\
  Player\'s move [(u)ndo/1-9] : a2 n n\n\n\n\
            BOARD\n\n\
        A     B     C          DIRECTION        ACTION\n\n\
   1    X --- . --- .          NW  N  NE        N = no action\n\
        |  \\  |  /  |            \\ | / \n\
   2    . --- O --- O          W - * - E        A = capture by approach\n\
        |  /  |  \\  |            / | \\\n\
   3    . --- X --- X          SW  S  SE        W = capture by withdraw \n\n\n\n\
(Computer will Win in 13)  \n\
Computer\'s move              : 155 \n\n\n\
            BOARD\n\n\
        A     B     C          DIRECTION        ACTION \n\n\
   1    X --- O --- .          NW  N  NE        N = no action \n\
        |  \\  |  /  |            \\ | / \n\
   2    . --- . --- O          W - * - E        A = capture by approach\n\
        |  /  |  \\  |            / | \\\n\
   3    . --- . --- X          SW  S  SE        W = capture by withdraw\n\n\n\n\
(Player will Lose in 12)  \n\
  Player\'s move [(u)ndo/1-9] : a1 se n\n\n\n\
            BOARD\n\n\
        A     B     C          DIRECTION        ACTION\n\n\
   1    . --- O --- .          NW  N  NE        N = no action\n\
        |  \\  |  /  |            \\ | / \n\
   2    . --- X --- O          W - * - E        A = capture by approach\n\
        |  /  |  \\  |            / | \\\n\
   3    . --- . --- X          SW  S  SE        W = capture by withdraw\n\n\n\n\
(Computer will Win in 5)  \n\
Computer\'s move              : 187\n\n\n\
            BOARD\n\n\
        A     B     C          DIRECTION        ACTION\n\n\
   1    . --- O --- O          NW  N  NE        N = no action\n\
        |  \\  |  /  |            \\ | / \n\
   2    . --- X --- .          W - * - E        A = capture by approach\n\
        |  /  |  \\  |            / | \\\n\
   3    . --- . --- .          SW  S  SE        W = capture by withdraw\n\n\n\n\
(Player will Lose in 4)  \n\
  Player\'s move [(u)ndo/1-9] :  b2 s w \n\n\n\
            BOARD\n\n\
        A     B     C          DIRECTION        ACTION \n\n\
   1    . --- . --- O          NW  N  NE        N = no action \n\
        |  \\  |  /  |            \\ | / \n\
   2    . --- . --- .          W - * - E        A = capture by approach\n\
        |  /  |  \\  |            / | \\\n\
   3    . --- X --- .          SW  S  SE        W = capture by withdraw\n\n\n\n\
(Computer will Win in 3)  \n\
Computer\'s move              : 65 \n\n\
            BOARD\n\n\
        A     B     C          DIRECTION        ACTION\n\n\
   1    . --- . --- .          NW  N  NE        N = no action\n\
        |  \\  |  /  |            \\ | / \n\
   2    . --- O --- .          W - * - E        A = capture by approach\n\
        |  /  |  \\  |            / | \\\n\
   3    . --- X --- .          SW  S  SE        W = capture by withdraw\n\n\n\n\
(Player will Lose in 2)  \n\
  Player\'s move [(u)ndo/1-9] :  b3 w n \n\n\n\
            BOARD\n\n\
        A     B     C          DIRECTION        ACTION\n\n\
   1    . --- . --- .          NW  N  NE        N = no action\n\
        |  \\  |  /  |            \\ | / \n\
   2    . --- O --- .          W - * - E        A = capture by approach\n\
        |  /  |  \\  |            / | \\\n\
   3    X --- . --- .          SW  S  SE        W = capture by withdraw\n\n\n\n\
(Computer will Win in 1)  \n\
Computer\'s move              : 159\n\n\n\
            BOARD\n\n\
        A     B     C          DIRECTION        ACTION\n\n\
   1    . --- . --- O          NW  N  NE        N = no action\n\
        |  \\  |  /  |            \\ | / \n\
   2    . --- . --- .          W - * - E        A = capture by approach\n\
        |  /  |  \\  |            / | \\\n\
   3    . --- . --- .          SW  S  SE        W = capture by withdraw\n\n\n\n\
(Player will Lose in 0)  \n\n\
Computer wins. Nice try, Player.\n"                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 ;

/*************************************************************************
**
** Everything above here must be in every game file
**
**************************************************************************/
extern BOOLEAN (*gGoAgain)(POSITION,MOVE);

int BOARDHEIGHT=3;                                /* dimensions of the board */
int BOARDWIDTH=3;                                 /*  "             "           */
int BOARDSIZE=9;                                /* 4x3 board, must agree with the 2 definitions below */
int MAX_X;                                      /* maximum number of pieces of X that is possible in a game */
int MAX_O;                                      /*  "            "            "O                  "         */

#define COMMENTSPACE 5                               /* number of spaces to the right of board before comment starts */

typedef char BlankOX;
BlankOX X = 'X';
BlankOX O = 'O';
BlankOX B = '.';

typedef enum _direction { nodir=0,lowleft=1,down=2,lowright=3,left=4,right=6,upleft=7,up=8,upright=9 } Direction;
typedef struct _coord { int x,y; } Coordinates;

char gBlankOXString[3];
int myPieces_array[10];

char slash[] = {'|',' ',' ','\\',' ',' ','|',' ',' ','/',' ',' '};
//               0   1   2    3   4   5   6   7   8   9   10  11

Coordinates lastMovedAt;
BOOLEAN movingAgain = FALSE;
BOOLEAN placedPieces = FALSE;
BOOLEAN firstTimeInit = TRUE;
BOOLEAN changedBoardSize = FALSE;

BOOLEAN forcedCapture = TRUE;

/////////////////////////////////////////// F U N C T I O N   P R O T O T Y P E S ///////////////////////////////////////////////////////
void space(int n);
POSITION BlankOXToPosition(BlankOX* board, char turn);
int AnyPiecesLeft(BlankOX theBoard[], BlankOX thePiece);
Coordinates IndexToCoordinates(int index);
Coordinates Neighbor(Coordinates pos, Direction dir);
Direction OtherDirection(Direction);
BlankOX WhoseTurn(int);
void ChangeBoardSize();
BOOLEAN CanStillCap(BlankOX*, Coordinates, Direction);
Direction NextDirection(Coordinates, Direction);
BOOLEAN GoAgain(POSITION position, MOVE move);
Coordinates CarryOutMove(BlankOX*, char, MOVE, BOOLEAN*);
int FileRowToIndex(char,int);
void PlacePieces();
void InitHash();
void DefaultPieces(BlankOX*);
char* StringDir(int);
char* StringPos(int);
char* StringCap(int);
void ClearBoardPieces();

void PositionToBlankOX(POSITION position, BlankOX *board);
int CoordinatesToIndex(Coordinates pos);
int InBounds(Coordinates pos);
int getNumOfDigit(int n);

void dbg(char *);

STRING MoveToString(MOVE);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void dbg(char* msg) {
	if(printMethods) puts(msg);
};

/************************************************************************
**
** NAME:        InitializeDatabases
**
** DESCRIPTION: Initialize the gDatabase, a global variable.
**
************************************************************************/

void InitHash() {
	gBlankOXString[0]=B;
	gBlankOXString[1]=O;
	gBlankOXString[2]=X;

	myPieces_array[0]=B;
	myPieces_array[1]=BOARDSIZE-MAX_O-MAX_X;
	myPieces_array[2]=BOARDSIZE-1;
	myPieces_array[3]=O;
	myPieces_array[4]=0;
	myPieces_array[5]=MAX_O;
	myPieces_array[6]=X;
	myPieces_array[7]=0;
	myPieces_array[8]=MAX_X;
	myPieces_array[9]=-1;

	gNumberOfPositions = generic_hash_init(BOARDSIZE, myPieces_array, NULL, 0); // pass null for function pointer
	//printf(POSITION_FORMAT"\n", gNumberOfPositions);
}

void DefaultPieces(BlankOX* board) {
	int i;
	int topRow = 0;
	int bottomRow = BOARDHEIGHT - 1;

	// clear board
	for(i = 0; i < BOARDSIZE; i++)
		board[i] = B;

	// sanity check
	if( (BOARDHEIGHT < 3) || (BOARDWIDTH < 3))
		return; // blank board

	while( (bottomRow - topRow) > 1) {
		// fill a row with a player's pieces for each side
		for(i = 0; i < BOARDWIDTH; i++) {
			board[topRow*BOARDWIDTH + i] = O; MAX_O++;
			board[bottomRow*BOARDWIDTH + i] = X; MAX_X++;
		}
		bottomRow--; topRow++;
	}

	int forward = 0;
	int backward = BOARDWIDTH - 1;

	for(forward = 0; (forward+1) < backward; forward++,backward--) {
		board[topRow*BOARDWIDTH + forward] = (forward % 2) == 0 ? X : O;
		board[topRow*BOARDWIDTH + backward] = (forward % 2) == 0 ? O : X;
		if(topRow < bottomRow) {
			board[bottomRow*BOARDWIDTH + forward] = (forward % 2) == 0 ? X : O;
			board[bottomRow*BOARDWIDTH + backward] = (forward % 2) == 0 ? O : X;
		}
	}

	// count X and O s
	MAX_X = 0;
	MAX_O = 0;
	for(i = 0; i < BOARDSIZE; i++)
		if(board[i] == X)
			MAX_X++;
		else if(board[i] == O)
			MAX_O++;
}

void dispVitals() {
	printf("BS BW BH MAX_X MAX_O %d %d %d, %d %d\n",BOARDSIZE,BOARDWIDTH, BOARDHEIGHT, MAX_X, MAX_O);
}
void printBoard(BlankOX* board) {
	int i = 0;
	for(i =0; i<BOARDSIZE; i++) {
		if(!((i)%BOARDWIDTH))
			puts("");
		putchar(board[i]);
	}
	puts("");
}

void InitializeGame()
{
	BlankOX initialBoard[BOARDSIZE];

	if(placedPieces) {
		placedPieces = FALSE;
		PositionToBlankOX(gInitialPosition, initialBoard);
	} else { // changedBoardSize or firstime
		DefaultPieces(initialBoard);
		InitHash();
	}

	char turn = X; // start out with X's turn

	POSITION initialPos =  BlankOXToPosition(initialBoard, turn);

	// set the function to handle GoAgain? decisions
	//  gGoAgain = GoAgain;
	//  gSolver = lgas_DetermineValue;

	movingAgain = FALSE;

	gInitialPosition = initialPos;
	gMinimalPosition = initialPos;

	gMoveToStringFunPtr = &MoveToString;
}

void FreeGame()
{
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

void GameSpecificMenu() {
	char GetMyChar();
	int done = 0;
	do {
		printf("\t----- Game specific options for %s -----\n\n",kGameName);
		printf("\tCurrent Initial Position:\n");
		PrintPosition(gInitialPosition, gPlayerName[kPlayerOneTurn], kHumansTurn);
		puts("");
		printf("\tD)\tChange board (d)imensions.\n");
		printf("\tC)\t(C)lear board pieces.\n");
		printf("\tP)\tManually (p)lace board pieces.\n");
		printf("\tF)\tToggle Forced Capture. Currently: "); forcedCapture ? puts("ON") : puts("OFF");
		printf("\tH)\t(H)elp\n");
		printf("\n\tQ)\t(Q)uit\n");
		printf("\n\tB)\t(B)ack to previous activity.\n");
		printf("\nSelect an option: ");

		switch(toupper(GetMyChar())) {
		case 'Q':
			ExitStageRight();
		case 'H':
			HelpMenus();
			break;
		case 'D':
			ChangeBoardSize();
			break;
		case 'C':
			ClearBoardPieces();
			break;
		case 'F':
			forcedCapture = FALSE;
			break;
		case 'P':
			PlacePieces();
			break;
		case 'B':
			done = 1;
			break;
		default:
			printf("\nSorry, I don't know that option. Try another.\n");
			HitAnyKeyToContinue();
			break;
		}
	} while(!done);
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
** CALLS:       PositionToBlankOX(POSITION,*BlankOX)
**              BlankOX WhosTurn(*BlankOX)
**
************************************************************************/

POSITION DoMove(thePosition, theMove)
POSITION thePosition;
MOVE theMove;
{
	BlankOX board[BOARDSIZE];
	POSITION newPosition;
	char turn = WhoseTurn(thePosition);
	BOOLEAN captured;

	PositionToBlankOX(thePosition, board);
	CarryOutMove(board, turn, theMove, &captured);

	if(GoAgain(thePosition, theMove)) {
		movingAgain = TRUE;
	} else {
		turn = (turn==X ? O : X);
		movingAgain = FALSE;
	}

	// return new state in hashed form
	newPosition = BlankOXToPosition(board, turn);
	return newPosition;
}

Coordinates CarryOutMove(BlankOX* board, char turn, MOVE theMove, BOOLEAN* captured)
{
	int index,intdir,cap;
	Direction dir;
	char movingPiece;
	Coordinates coord;
	Coordinates result;
	*captured = FALSE;

	// extract the information from theMove
	cap = 3 & theMove;
	intdir = (28 & theMove) >> 2;
	index = (0xffffffe0 & theMove) >> 5;

	// unpack the direction to more than 3 bits
	intdir++;
	if(intdir>=5)
		intdir++;

	// convert integer direction to enum direction
	dir = (Direction)intdir;
	Direction otherDir = OtherDirection(dir);

	coord = IndexToCoordinates(index);

	// do the actual move
	movingPiece = board[index];
	char otherPlayer = (movingPiece == X ? O : X);
	board[index] = B;
	board[CoordinatesToIndex(Neighbor(coord,dir))] = movingPiece;
	result = Neighbor(coord,dir);
	lastMovedAt = result;

	Coordinates attacking;
	// check for capture, railgun style
	if(cap == 1) { // if by approach
		attacking = Neighbor(Neighbor(coord,dir),dir);
		while(InBounds(attacking) && (board[CoordinatesToIndex(attacking)] == otherPlayer)) {
			board[CoordinatesToIndex(attacking)] = B;
			attacking = Neighbor(attacking,dir);
		}
		*captured = TRUE;
	}else if(cap == 2) { // if by withdraw
		attacking = Neighbor(coord,otherDir);
		while(InBounds(attacking) && (board[CoordinatesToIndex(attacking)] == otherPlayer)) {
			board[CoordinatesToIndex(attacking)] = B;
			attacking = Neighbor(attacking,otherDir);
		}
		*captured = TRUE;
	}
	return result;
}

BOOLEAN CanStillCap(BlankOX* board, Coordinates pos, Direction prevdir) {
	Direction dir;
	Coordinates attack, approach, withdraw;
	char movingPiece = board[CoordinatesToIndex(pos)];
	char enemy = (movingPiece == X ? O : X);

	for(dir=left; dir!=0; dir=NextDirection(pos, dir)) {
		attack = Neighbor(pos,dir);
		if( InBounds(attack) &&
		    ( (dir != prevdir) || (dir != OtherDirection(prevdir)) ) && // if not on same line
		    ( board[CoordinatesToIndex(attack)] == B ) ) {
			approach = Neighbor(attack,dir);
			withdraw = Neighbor(pos,OtherDirection(dir));
			if( InBounds(approach) &&
			    (board[CoordinatesToIndex(approach)] == enemy) )
				return TRUE;
			else if( InBounds(withdraw) &&
			         (board[CoordinatesToIndex(withdraw)] == enemy) )
				return TRUE;
		}
	}
	// if got to here, there was no continuiation attack
	return FALSE;
}

BOOLEAN GoAgain(POSITION position, MOVE theMove) {
	return FALSE;
	BlankOX board[BOARDSIZE];
	int intdir;
	BOOLEAN captured;
	char turn = WhoseTurn(position);

	intdir = (28 & theMove) >> 2;
	// unpack the direction to more than 3 bits
	intdir++;
	if(intdir>=5)
		intdir++;
	Direction dir = (Direction)intdir;

	PositionToBlankOX(position, board);
	//Coordinates moved = CarryOutMove(board, wturn, theMove, &captured);
	Coordinates moved = CarryOutMove(board, WhoseTurn(position), theMove, &captured);
	putchar(turn);
	if(captured) { // if the move had captures
		BOOLEAN temp = CanStillCap(board, moved, dir); // and can still cap with that piece
		temp ? puts("GA:TRUE") : puts("GA:FALSE");
		return temp;
	} else { // no chance to move again without any capture
		puts("GA:FALSE");
		return FALSE;
	}
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
	return gInitialPosition;
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
	dbg("->PrintComputersMove");
	printf("%8s's move              : %2d\n", computersName, computersMove+1);
}

/************************************************************************
**
** NAME:        Primitive
**
** DESCRIPTION: Determines whether it's win/lose/tie or undecided based
**              on what's left on the board.
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

VALUE Primitive(position)
POSITION position;
{
	dbg("->Primitive");
	BlankOX board[BOARDSIZE];

	PositionToBlankOX(position,board);

	if (!AnyPiecesLeft(board,X) || !AnyPiecesLeft(board,O)) /* if either type of pieces is extinct */
		return(gStandardGame ? lose : win);         /* that means last player to go captured last piece of current player? */
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

//shing
void PrintPosition(position,playerName,usersTurn)
POSITION position;
STRING playerName;
BOOLEAN usersTurn;
{
	dbg("->PrintPos");
	int x, y;
	int displayBoardWidth = (BOARDWIDTH - 1 ) * 6 + 1;
	int displayBoardHeight = (BOARDHEIGHT - 1) * 2 + 1;
	int numOfDigit = 0;
	int temp;
	int maxX;
	int maxY;
	int slashFlag = 1;
	int commentx;
	int myBoardStartAt;
	int myDirectionStartAt;
	int myActionStartAt;
	int boardIndex;
	int rowNum = 1;

	//STRING  GetPrediction();
	//VALUE   GetValueOfPosition();

	BlankOX theBoard[BOARDSIZE];

	PositionToBlankOX(position,theBoard);

	// get the maximum num of digit of the position
	temp = BOARDWIDTH*BOARDHEIGHT;
	numOfDigit = getNumOfDigit(temp) - 1;

	// the word "BOARD" starts at location:
	myBoardStartAt = ceil((displayBoardWidth - 5)/2.0)  + 8;
	// the word "LEGEND" starts at location:
	// myLegendStartAt = ceil((displayBoardWidth -6)/2.0) + 8 + 8 + displayBoardWidth;
	// the word "DIREDTION" starts at location:
	//myDirectionStartAt = 8 + displayBoardWidth + 8 + displayBoardWidth + numOfDigit + 8 + 2;
	myDirectionStartAt = 5 + displayBoardWidth + 8;
	// the word "ACTION" starts at location:
	myActionStartAt = myDirectionStartAt + 9 + 8 + 1;

	// get the maximum width
	//maxX = 8 + displayBoardWidth + 8 + displayBoardWidth + numOfDigit + 8 + 9 + 8 + 24;
	maxX = 5 + displayBoardWidth + 8 + 9 + 8 + 24;

	maxY = displayBoardHeight;

	// hrs to shing: commented below because bug when #rows is 2

	// get the maximum height
	if (displayBoardHeight>5)
		maxY = displayBoardHeight;
	else
		maxY = 5;

	maxY = maxY + 2;
	commentx = maxX + 1 + COMMENTSPACE;


	/*
	   // Display title
	   for (x=0; x<maxX; x++)
	   {
	    // display "BOARD"
	    if ((x >= myBoardStartAt) && ( x <= myBoardStartAt +5))
	      {
	        printf("%c",myBoard[x - myBoardStartAt]);
	      }
	    // display "LEGEND"
	    else if ((x >= myLegendStartAt) && ( x <= myLegendStartAt +6))
	      {
	        printf("%c",myLegend[x - myLegendStartAt]);
	      }
	    // display "DIRECTION"
	    else if ((x >= myDirectionStartAt) && ( x <= myDirectionStartAt +9))
	      {
	        printf("%c",myDirection[x - myDirectionStartAt]);
	      }
	    // display "ACTION"
	    else if ((x >= myActionStartAt) && ( x <= myActionStartAt +6))
	      {
	        printf("%c",myAction[x - myActionStartAt]);
	      }
	    else
	      space(1);

	   }
	   puts("");
	   puts("");
	 */


	// Display BOARD
	puts("");
	puts("");
	space(myBoardStartAt);
	printf("BOARD");

	puts("");
	puts("");

	// Display coordinate A, B, C, ...
	printf("        ");
	for (x=0; x<BOARDWIDTH; x++)
		printf("%c     ",x+65);
	space(5);
	printf("DIRECTION");
	space(8);
	printf("ACTION");
	printf("\n");
	printf("\n");

	// for board
	for (y = 0; y < maxY; y++) {

		space(3);
		if ((y % 2 == 0) && (y < displayBoardHeight))
		{
			printf("%d",rowNum++);
			for (x=1; x<=5 - getNumOfDigit(rowNum - 1); x++)
				printf(" ");
		}
		else
			space(5);

		slashFlag = !slashFlag;

		// display board
		if (y < displayBoardHeight)
			for ( x = 0; x < displayBoardWidth; x++) {
				if ( y % 2 == 1 ) {                  // rows with slashes  |  \  |  / ...
					putchar(slash[(x + (slashFlag ? 0 : 6)) % 12]);
					slashFlag = !slashFlag;
				}
				else {                                 // rows with cells
					if ((x % 6) == 0) {            // if it's time to print a cell
						boardIndex = (y/2) * BOARDWIDTH + (x/6);
						putchar(theBoard[boardIndex]);
					} else if ( (x % 6) == 1 || (x % 6) == 5 )
						putchar(' ');
					else
						putchar('-');
				}
			} // end display board
		else
			space(displayBoardWidth);

		space(10);
		//slashFlag = 0;
		slashFlag = !slashFlag;

		/*
		   //display legend
		   if (y < displayBoardHeight)
		   for ( x = 0; x < displayBoardWidth+1; x++) {
		    if ( y % 2 == 1 ) {                                      // rows with slashes  |  \  |  / ...
		      putchar(slash[(x + (slashFlag? 0 : 6)) % 12]);
		      slashFlag = !slashFlag;
		    }
		    else  if ((x % 6) == 0) {                                      // if it's time to print a cell
		      //boardIndex = (y/2) * BOARDWIDTH + (x/6);
		      //putchar(theBoard[boardIndex]);
		      legendNum++;
		      printf("%d",legendNum);
		      x = x + getNumOfDigit(legendNum) -1;

		    }
		    else
		      {

		        temp = getNumOfDigit(legendNum);
		        // (temp == 1 )
		        temp --;
		        if (  (x % 6 == (1+temp)) || (x % 6 ==  5      ) )
		          putchar(' ');
		        else
		          putchar('-');
		      }

		   } // end display board
		   else
		   space(displayBoardWidth);
		 */

		//display direction
		//	if (y < 5)
		//space(8-getNumOfDigit(legendNum)-1);
		//space(8);
		switch(y)
		{
		case 0:
			printf("NW  N  NE        N = no action");
			break;
		case 1:
			printf("  \\ | / ");
			break;
		case 2:
			printf("W - * - E        A = capture by approach");
			break;
		case 3:
			printf("  / | \\");
			break;
		case 4:
			printf("SW  S  SE        W = capture by withdraw");
			break;
		}





		puts("");

	} // end loop y





	puts("");
	puts(GetPrediction(position,playerName,usersTurn));
	dbg("<-PrintPos");
}

// if it's one digit, return 0
// if it's two digit, return 1
//             .
//             .
//             .
// if it's n digit, return n-1
int getNumOfDigit(int n)
{
	int numOfDigit = 0;

	while (n != 0)
	{
		n = n /10;
		numOfDigit++;
	}
	return numOfDigit;

	return numOfDigit;
}

void space(int n) {
	int i = 0;
	for(; i < n; i++)
		putchar(' ');
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
**              WhoseTurn(...),InBounds(...),CoordinatesToIndex(...),
**              IndexToCoordinates(...),Primitive(...),
**              Neighbor(...),OtherDirection(...),NextDirection(...),OtherPlayer
**
************************************************************************/

MOVELIST *GenerateMoves(position)
POSITION position;
{
	MOVELIST *CreateMovelistNode(), *head = NULL;
	VALUE Primitive();
	Coordinates IndexToCoordinates(int);
	int CoordinatesToIndex(Coordinates);
	int InBounds(Coordinates);
	Coordinates Neighbor(Coordinates,Direction);
	Direction NextDirection(Coordinates, Direction);
	Direction OtherDirection(Direction);
	BlankOX OtherPlayer(char current);
	BlankOX WhoseTurn(int);

	char thisPlayer = WhoseTurn(position);
	char otherPlayer = thisPlayer==X ? O : X;


	BlankOX board[BOARDSIZE];
	int i;
	Direction dir;
	int intdir;
	Coordinates pos,attackingpos;
	int cap=0; // capture mode
	int numofcaps; //number of captures possible for one direction of move
	int move;

	if (Primitive(position) == undecided) {
		PositionToBlankOX(position,board);
		for(i = 0; i < BOARDSIZE; i++) {
			if((board[i] == thisPlayer) && (movingAgain ? CoordinatesToIndex(lastMovedAt)==i : TRUE)) {
				// if current player's piece and if movingAgain must be same piece then we consider where we can move this
				pos = IndexToCoordinates(i);
				for(dir = left; dir!=0; dir = NextDirection(pos, dir)) {
					intdir = (int)dir;
					if(intdir>=5) // collapse direction into 3 bits instead of 4
						intdir--; // 5 is not a direction, don't need that
					intdir--; // start at 0

					numofcaps=0;
					if(InBounds(Neighbor(pos,dir)) && board[CoordinatesToIndex(Neighbor(pos,dir))]==B) { // vacant neighboring cell exists
						attackingpos = Neighbor(Neighbor(pos,dir),dir);

						// by approach
						if( InBounds(attackingpos) && // position is within board
						    board[CoordinatesToIndex(attackingpos)]==otherPlayer) { // have enemy, can attack by approach
							cap = 1; // attack by approach mode
							move = CoordinatesToIndex(pos)<<5 | intdir<<2 | cap;
							if(movingAgain) { // if moving again, then we need to move the same piece as b4
								if(CoordinatesToIndex(lastMovedAt)==i) {
									numofcaps++;
									head = CreateMovelistNode(move, head);
								}
							} else {
								numofcaps++;
								head = CreateMovelistNode(move, head);
							}
							if(!forcedCapture) { // not forced to capture, then can move beside enemy with no killing
								move = CoordinatesToIndex(pos)<<5 | intdir<<2 | 0; // cap = 0
								head = CreateMovelistNode(move, head);
							}
						} // end if can attack by approach

						// by withdraw
						attackingpos = Neighbor(pos,OtherDirection(dir));
						if(InBounds(attackingpos) && // position is within board
						   board[CoordinatesToIndex(attackingpos)]==otherPlayer) { // have enemy, can attack by withdraw
							cap = 2; // attack by withdraw mode
							move = CoordinatesToIndex(pos)<<5 | intdir<<2 | cap;
							if(movingAgain) { // if moving again, we need to move same piece as before
								if(CoordinatesToIndex(lastMovedAt)==i) {
									numofcaps++;
									head = CreateMovelistNode(move, head);
								}
							}else {
								numofcaps++;
								head = CreateMovelistNode(move, head);
							}
							if(!forcedCapture) {
								move = CoordinatesToIndex(pos)<<5 | intdir<<2 | 0; // cap = 0
								printf("Adding move:\n");
								PrintMove(move);
								head = CreateMovelistNode(move, head);
							}
						} // end if can attack by withdraw

						if(numofcaps==0) { // no captures possible for this move direction
							cap = 0;
							move = CoordinatesToIndex(pos)<<5 | intdir<<2 | cap;
							head = CreateMovelistNode( move, head);
						} //end if no possible captures
					} // end if vacant target cell (i.e. can move)
				} // end for this direction
			} // end if this piece is current players piece
		} // end for this cell of board
		return(head);
	} else { // end if undecided (not win or lose)
		return(NULL);
	}
}

/************************************************************************
 * Inputs:
 * Description: enumerates all possible directions. ie. one direction per call
 *              in the end, will return nodir = 0
 * Calls:
 ***********************************************************************/
Direction NextDirection(Coordinates pos, Direction dir) {
	if( (pos.x & 1) == (pos.y & 1) ) {
		switch(dir) {
		case left: return dir=upleft;
		case upleft: return dir=up;
		case up: return dir=upright;
		case upright: return dir=right;
		case right: return dir=lowright;
		case lowright: return dir=down;
		case down: return dir=lowleft;
		case lowleft: return dir=nodir;
		case nodir: return dir=left;
		}
	}else {
		switch(dir) {
		case left: return dir=up;
		case up: return dir=right;
		case right: return dir=down;
		case down: return dir=nodir;
		case nodir: return dir=left;
		default: return nodir; //this should never happen, I think
		}
	}

	return nodir; //should never be reached
}

/************************************************************************
 * Inputs: Direction
 * Description: returns opposite direction
 * Calls:
 ***********************************************************************/
Direction OtherDirection(Direction dir) {
	switch(dir) {
	case up: return down;
	case down: return up;
	case left: return right;
	case right: return left;
	case upleft: return lowright;
	case upright: return lowleft;
	case lowleft: return upright;
	case lowright: return upleft;
	default: return nodir;
	}
	return nodir; // never reached
}

/************************************************************************
 * Inputs:      Coordinates pos, Direction dir
 * Description: returns the Coordinates for the neighboring space of pos
 * Calls:
 ***********************************************************************/
Coordinates Neighbor(Coordinates pos, Direction dir) {
	if (dir == up || dir == upleft || dir == upright)
		pos.y--;
	else if (dir == down || dir == lowleft || dir == lowright)
		pos.y++;

	if (dir == left || dir == upleft || dir == lowleft)
		pos.x--;
	else if (dir == right || dir == upright || dir == lowright)
		pos.x++;

	return pos;
}

/************************************************************************
 * Inputs:      Coordinates
 * Description: true if within board boundary, false otherwise
 * Calls: IndexToCoordinates(...)
 ***********************************************************************/
int InBounds(Coordinates pos) {
	return (pos.x>=0) && (pos.x<BOARDWIDTH) && (pos.y>=0) && (pos.y<BOARDHEIGHT);
}

/************************************************************************
 * Inputs:      integer - array index of board
 * Description: returns Coordinates - position on board corresponding to the index
 *              x,y both start at 0
 * Calls:
 ***********************************************************************/
Coordinates IndexToCoordinates(int index) {
	Coordinates pos;
	pos.y = index / BOARDWIDTH;
	pos.x = index % BOARDWIDTH;
	return pos;
}

/************************************************************************
 * Inputs:      Coordinates
 * Description: returns array index corresponding to position given by input
 * Calls:
 ***********************************************************************/
int CoordinatesToIndex(Coordinates pos) {
	return pos.x + pos.y*BOARDWIDTH;
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
	dbg("->GetAndPrintPlayersMove");
	BOOLEAN ValidMove();
	USERINPUT ret, HandleDefaultTextInput();

	do {
		printf("%8s's move [(u)ndo/1-9] :  ", playerName);

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
**
** INPUTS:      STRING input : The string input the user typed.
**
** OUTPUTS:     BOOLEAN : TRUE iff the input is a valid text input.
**
************************************************************************/

BOOLEAN ValidTextInput(input)
STRING input;
{
	dbg("->ValidTextInput");

	char file;
	int row;
	char temp[4];
	int dir;
	int cap;
	int i;
	char c,d;

	while(*input == ' ') input++;
	if(!(*input)) return FALSE;

	if(!isalpha(*input))
		return FALSE;
	else {
		file = toupper(*input);
		input++;
	}

	i = 0;
	while(!isalpha(input[i])) {
		if(i >3)
			return FALSE;
		else {
			temp[i] = input[i];
			i++;
		}
	}
	input += i;
	temp[i] = (char)0;

	row = atoi(temp);

	while(*input == ' ') input++;
	if(!(*input)) return FALSE;

	c = toupper(*input); input++;
	d = toupper(*input); input++;

	if(c == 'N') {
		dir = 8;
		if(d == 'E')
			dir++;
		else if(d == 'W')
			dir--;
		else if(d != ' ')
			return FALSE;
	}else if(c == 'S') {
		dir = 2;
		if(d == 'E')
			dir++;
		else if(d == 'W')
			dir--;
		else if(d != ' ')
			return FALSE;
	}else if(c=='W') {
		dir = 4;
		if(d!=' ')
			return FALSE;
	}else if(c=='E') {
		dir = 6;
		if(d!=' ')
			return FALSE;
	}else {
		return FALSE;
	}

	while(*input == ' ') input++;
	if(!(*input)) return FALSE;

	c = toupper(*input);

	cap = -1;
	if(c == 'A') {
		cap = 1;
	} else if(c == 'W')
		cap = 2;
	else if(c == 'N')
		cap = 0;
	else {
		return FALSE;
	}

	int pos = (row-1)*BOARDWIDTH + (file - 'A');

	if(pos<0 || pos>=BOARDSIZE)
		return FALSE;

	if(dir==5 || dir<0 || dir>9)
		return FALSE;

	if(cap<0 || cap>3)
		return FALSE;

	return TRUE;
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

	char file;
	int row;
	char temp[4];
	int dir;
	int cap;
	int i;
	char c,d;

	while(*input == ' ') input++;

	if(!isalpha(*input))
		return FALSE;
	else {
		file = toupper(*input);
		input++;
	}

	i = 0;
	while(!isalpha(input[i])) {
		if(i >3)
			return FALSE;
		else {
			temp[i] = input[i];
			i++;
		}
	}
	input += i;
	temp[i] = (char)0; // null char

	row = atoi(temp);

	int pos = (row-1)*BOARDWIDTH + (file - 'A');

	while(*input == ' ') input++;

	c = toupper(*input); input++;
	d = toupper(*input); input++;

	if(c == 'N') {
		dir = 8;
		if(d == 'E')
			dir++;
		else if(d == 'W')
			dir--;
		else if(d != ' ')
			return FALSE;
	}else if(c == 'S') {
		dir = 2;
		if(d == 'E')
			dir++;
		else if(d == 'W')
			dir--;
		else if(d != ' ')
			return FALSE;
	}else if(c=='W') {
		dir = 4;
		if(d!=' ')
			return FALSE;
	}else if(c=='E') {
		dir = 6;
		if(d!=' ')
			return FALSE;
	}else {
		return FALSE;
	}


	while(*input == ' ') input++;
	if(!(*input)) return FALSE;

	c = toupper(*input);

	cap = -1;
	if(c == 'A') {
		cap = 1;
	} else if(c == 'W')
		cap = 2;
	else if(c == 'N')
		cap = 0;
	else {
		return FALSE;
	}

	/* we want:
	   ___________________________________________
	 |              27: pos     | 3:dir | 2:cap |
	   -------------------------------------------*/
	/* encode 'a' as 01, 'w' as 10 and n as 00 */

	if(dir >= 5)
		dir--; // 5 is not a dir
	dir--; // make dir start at 0

	int move = (pos<<5) | (dir<<2) | cap;
	if(debug>1) printf("Resulting move %d\n",move);

	// mask everything into the MOVE to return
	return move;
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
	printf( "%s", MoveToString(theMove) );
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
	STRING move = (STRING) SafeMalloc( 13 );
	int pos,dir,cap;

	cap = 3 & theMove;
	dir = (28 & theMove) >> 2;
	pos = (0xffffffe0 & theMove) >> 5;

	dir++;
	if(dir >= 5)
		dir++;

	char* position = StringPos( pos );
	char* direction = StringDir( dir );
	char* capture = StringCap( cap );

	sprintf( move, "[ %s %s %s ]", position, direction, capture );


	SafeFree( position );
	SafeFree( direction );
	SafeFree( capture );

	return move;
}

char* StringCap(int cap) {
	char* capture = (char*) SafeMalloc( 2 );

	if(cap==0)
		sprintf( capture, "N" );
	else if(cap==1)
		sprintf( capture, "A" );
	else
		sprintf( capture, "W" );

	return capture;
}

char* StringDir(int dir) {
	char* direction = (char*) SafeMalloc( 3 );

	if(dir==8)
		sprintf( direction, "N" );
	else if(dir==9)
		sprintf( direction, "NE" );
	else if(dir==6)
		sprintf( direction, "E" );
	else if(dir==3)
		sprintf( direction, "SE" );
	else if(dir==2)
		sprintf( direction, "S" );
	else if(dir==1)
		sprintf( direction, "SW" );
	else if(dir==4)
		sprintf( direction, "W" );
	else
		sprintf( direction, "NW" );

	return direction;
}

char* StringPos(int pos) {
	int row = (pos/BOARDWIDTH) + 1;
	char file = 'a' + (pos%BOARDWIDTH);
	char* position = (char*) SafeMalloc(3);
	sprintf( position, "%c%d", file, row );

	return position;
}

/************************************************************************
*************************************************************************
** BEGIN   FUZZY STATIC EVALUATION ROUTINES. DON'T WORRY ABOUT UNLESS
**         YOU'RE NOT GOING TO EXHAUSTIVELY SEARCH THIS GAME
*************************************************************************
************************************************************************/

/************************************************************************
*************************************************************************
**         EVERYTHING BELOW THESE LINES IS LOCAL TO THIS FILE
*************************************************************************
************************************************************************/

/***********************************************************************
 *
 * Input: A position (an integer), a board (an array)
 *
 * Desc: Changes the value of the given board based on the given position (int)
 *
 * Calls: generic_hash_unhash(...)
 **********************************************************************/
void PositionToBlankOX(POSITION position, BlankOX *board) {
	dbg("->PositionToBlankOX");
	generic_hash_unhash(position,(char *)board);
}

/************************************************************************
 *
 * Input: A board
 *
 * Desc: Returns a POSITION (which is an int) that is a board encoded as an int
 *
 * Calls: generic_hash_hash(...)
 ***********************************************************************/
POSITION BlankOXToPosition(BlankOX *board, char turn) {
	dbg("->BlankOXToPosition");
	return generic_hash_hash((char *)board, turn==X ? 1 : 2 );
}

/***********************************************************************
 *
 * Inputs: A board, a piece type
 *
 * Desc: Returns true if the given board has at least one piece of given type
 *       Otherwise, return false
 * Calls:
 **********************************************************************/
int AnyPiecesLeft(BlankOX theBoard[], BlankOX thePiece) {
	dbg("->AnyPiecesLeft");
	int i;
	int anyLeft = 0; /* assume there are none left, then set true if we see one */

	for(i=0; i<BOARDSIZE; i++)
		if(theBoard[i] == thePiece)
			anyLeft = 1; /* we saw one, so set to true */

	return anyLeft;
}

/************************************************************************
 *
 * Inputs:
 *
 * Desc: Returns either o or x (which are from an enumeration) based on
 *       whose turn it is. i.e. return x if it's x's turn.
 *
 * Calls:
 ***********************************************************************/
BlankOX WhoseTurn(int pos) {
	//printf("WhoseMove: %d\n", generic_hash_turn(pos));
	return (generic_hash_turn(pos) == 1 ? X : O);
}

BlankOX OtherPlayer(char current) {
	return current==X ? O : X;
}

STRING kDBName = "fandan";

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

void ChangeBoardSize() {
	char GetMyChar();
	int width, height;
	printf("\nEnter the dimensions of the board (width then height): ");
	scanf("%d %d", &width, &height);
	BOARDWIDTH = width;
	BOARDHEIGHT = height;
	BOARDSIZE = width*height;
	changedBoardSize = TRUE;
	InitializeGame();
}

void ClearBoardPieces() {
	BlankOX board[BOARDSIZE];
	int i;
	for(i = 1; i < BOARDSIZE; i++) {
		board[i] = B;
	}
	board[0] = O;
	board[BOARDSIZE-1] = X;

	MAX_X = 1;
	MAX_O = 1;
	InitHash();
	placedPieces = TRUE; // to signal initializer not to stomp our new board placement
	gInitialPosition = BlankOXToPosition(board, X);
	gMinimalPosition = gInitialPosition;
}

void PlacePieces() {
	char GetMyChar();
	char file;
	int row;
	char piece;
	int done = 0;
	int index;
	BlankOX board[BOARDSIZE];
	PositionToBlankOX(gInitialPosition, board);

	// get the file letter
	while(!done) {
		printf("\nEnter the file to place piece or Q to quit: ");
		file = toupper(GetMyChar());
		if('Q' == file)
			return;

		if(('A' <= file) && (file <= ('A' + BOARDWIDTH)))
			done = 1;
		else
			printf("\n\\Incorrect file\n");
	}

	// get the row number
	done = 0;
	while(!done) {
		printf("\nEnter the row to place piece: ");
		if(0 == scanf("%d",&row)) {
			printf("Please enter a number.\n");
			getchar();
		} else
			done = 1;
	}
	index = FileRowToIndex(file,row);

	// get the piece to place
	done = 0;
	while(!done) {
		printf("\nEnter the piece to place X O or D to delete: ");
		piece = toupper(GetMyChar());
		switch(piece) {
		case 'X':
			if(board[index] == X) {
			}else if(board[index] == B) {
				MAX_X++;
			}else if(board[index] == O) {
				MAX_O--;
				MAX_X++;
			}
			board[index] = piece;
			done = 1;
			break;
		case 'O':
			if(board[index] == O) {
			}else if(board[index] == B) {
				MAX_O++;
			}else if(board[index] == X) {
				MAX_X--;
				MAX_O++;
			}
			board[index] = piece;
			done = 1;
			break;
		case 'D':
			if((MAX_X + MAX_O) == 1) {
				printf("\nSorry, must maintain at least one piece on board.\n");
				break;
			}
			if(board[index] == O)
				MAX_O--;
			else if(board[index] == X)
				MAX_X--;
			board[index] = B;
			done = 1;
			break;
		default:
			printf("Unknown piece\n");
		}
	}

	placedPieces = TRUE;
	InitHash();
	gInitialPosition = BlankOXToPosition(board, X);
	gMinimalPosition = gInitialPosition;
}

int FileRowToIndex(char file, int row) {
	return(((row-1)*BOARDWIDTH)+(toupper(file) - 'A'));
}

/* ----- Shing ----------------------------------------



               BOARD

     A     B     C     D     E         DIRECTION          ACTION

   1    o --- o --- o --- o --- o         7   8   9          0 = no action
 |  \  |  /  |  \  |  /  |           \ | /
   2    o --- x --- . --- o --- x         4 - * - 6          1 = capture by approach
 |  /  |  \  |  /  |  \  |           / | \
   3    x --- x --- x --- x --- x         1   2   3          2 = capture by withdraw





                 2 5
                                  111 - 112 - 113 - 114 - 115
                        4,6,10,12,16



        o --- o --- o --- o         1 --- 2 --- 3 --- 4            7   8   9          0 = no action
 |  \  |  /  |  \  |         |  \  |  /  |  \  |              \ | /
        o --- x --- . --- o         6 --- 7 --- 8 --- 9           4 - * - 6          1 = capture by approach
 |  /  |  \  |  /  |         |  /  |  \  |  /  |              / | \
        x --- x --- x --- x         11 -- 12 -- 13 -- 14            1   2   3          2 = capture by withdraw


   space(8)  board space(8) legend space(8) direction space(8) action



   //  (o)-(o)-(o)-(o)-(o)
   //   | \ | / | \ | / |
   //  (o)-(x)-( )-(o)-(x)
   //   | / | \ | / | \ |
   //  (x)-(x)-(x)-(x)-(x)

   //  [o]-[o]-[o]-[o]-[o]
   //   | \ | / | \ | / |
   //  (o)-(x)-( )-(o)-(x)
   //   | / | \ | / | \ |
   //  (x)-(x)-(x)-(x)-(x)

   // ----------------------------------------------------
 */

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
