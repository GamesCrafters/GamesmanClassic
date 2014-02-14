/************************************************************************
   c**
 ** NAME:        mwin4.c
 **
 ** DESCRIPTION: Connect-4
 **
 ** AUTHOR:      Michael Thon, University of Berkeley
 **              Copyright (C) Michael Thon, 2002. All rights reserved.
 **
 ** DATE:        07/12/02
 **
 ** UPDATE HIST:
 **
 **  7-12-02 1.0 : Getting started (not using RCS though)
 **                will first attempt only 5x4 size boards
 **  9-12-02 1.1 : Board sizes (smaller than 5x5) can be set in mwin4.h
 **                Text-based module works for these.
 **                ToDo: -graphics
 **                      -help-Information
 **                      -documentation
 ** 10-12-02 1.2 : - Board sizes now set in this file (no more mwin4.h)
 **                  MUST ALSO SET IN mwin4.tcl !! (gSlotsX and Y)
 **                  Feasale sizes are 5x4 or 4x5 (both interesting)
 **                - Program will probably SegFault if sizeof(int)<4
 **                  Would be nice to have POSITION be long int or even
 **                  unsigned long int
 **                - Graphical Module mwin4.tcl done
 **
 ** 08-19-06	: change to GetMyInt();
 **
 ** 02-23-06       Ilya Landa
 **                Implementing symmetries
 **************************************************************************/

/* a position seem to be made with this: (4x4)
 * 10100 10010 11010 01110
 * The encoding made by an author is ingennious but was a bit tiesome to decode.
 * Each segment is a column going up.
 * 1 is "O", 0 is "X" unless either is followed by all 0's,
 *   in which case the bit is the beginning of "on pieces" part of the column.
 */
// !!!THE GAME WILL CRASH IF WIDTH * (HEIGHT + 1) >= 32!!!

/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/

#include "gamesman.h"

POSITION gNumberOfPositions   = 0; // Initialized to MyNumberOfPos()
POSITION gInitialPosition     = 0;     // Initialized to MyInitialPosition()
POSITION kBadPosition         = 0;     // This can never be the rep.
                                       // of a position

POSITION gMinimalPosition     =  0;
STRING kDBName              = "win4";
STRING kAuthorName          = "Michael Thon";
STRING kGameName            = "Connect-4";
BOOLEAN kPartizan            = TRUE;
BOOLEAN kSupportsHeuristic   = FALSE;
BOOLEAN kSupportsSymmetries  = FALSE;
BOOLEAN kSupportsGraphics    = TRUE;
BOOLEAN kDebugMenu           = FALSE;  //What for??
BOOLEAN kGameSpecificMenu    = TRUE;
BOOLEAN kTieIsPossible       = TRUE;
BOOLEAN kLoopy               = FALSE;
BOOLEAN kDebugDetermineValue = FALSE;

void*    gGameSpecificTclInit = NULL;

STRING kHelpGraphicInterface =
        "Click on a highlighted sqare to make your move there.";

STRING kHelpTextInterface    =
        "On your turn, enter the number (usually 1 through 5) corresponding to a\n\
slot that is not yet full. This will ''drop'' a piece of yours into the\n\
slot. If at any point you have made a mistake, you can type u and hit\n\
return and the system will revert back to your most recent position."                                                                                                                                                                                                                                        ;

STRING kHelpOnYourTurn =
        "''Drop'' one of your pieces into an open slot by entering the \n\
corresponding number. "                                                                           ;

STRING kHelpStandardObjective =
        "To get four of your pieces in a row, either horizontally, vertically,\n\
 or diagonally."                                                                                  ;

STRING kHelpReverseObjective =
        "To force your opponent into getting four of his pieces  in a row, either \n\
 horizontally, vertically, or diagonally."                                                                                      ;

STRING kHelpTieOccursWhen =   /* Should follow 'A Tie occurs when... */
                            "the board fills up without either player getting four-in-a-row.";

STRING kHelpExample =
        "Just go ahead and try it out for yourself. No need to be scared...";

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

/************************************************************************
**
**  Game Specific Global Variables
**
************************************************************************/
int WIN4_WIDTH = 4;
int WIN4_HEIGHT = 4;
int TIER_COL_BITS;  //Represents how many bits are needed to encode the number of pieces in all columns.
int COLSIGBITS;  //Represents how many bits are needed to encode the number of pieces per column.
int WIN4_WIDTH_MINUS_ONE; //Stored globally for optimization purposes.
int WIN4_HEIGHT_PLUS_ONE; //Stored globally for optimization purposes.
int WIN4_WIDTH_PLUS_ONE;
#define MAXW 9
#define MAXH 9
#define MINW 1
#define MINH 1
#define TOTAL_STAGES (WIN4_WIDTH*WIN4_HEIGHT)

#define DIRECTION_PAIRS 4
#define NO_COLUMN -1

BOOLEAN gLibraries           = FALSE;

typedef enum possibleBoardPieces {
	x, o, Blank
} XOBlank;

char           *gBlankOXString[] = { "X", "O", "-" };

int gContinuousPiecesGoal = 4;

typedef struct {
	int *convert;
} NumToPieceConv;

NumToPieceConv *NumToPieceDist;
NumToPieceConv *PieceDistToNum;

/* Global position solver variables.*/

struct {
	XOBlank board[MAXW][MAXH];
	POSITION heights[MAXW];
	POSITION lastColumn;
	XOBlank nextPiece;
	POSITION piecesPlaced;
	POSITION previousColumn;
} gPosition;


typedef enum {
	NO_DIRECTION,
	DOWN,
	LEFT,
	RIGHT,
	UP,
	DIRECTIONS
} Direction;

Direction gDirections[DIRECTION_PAIRS][2] = {{LEFT, UP},
					     {NO_DIRECTION, UP},
					     {RIGHT, UP},
					     {LEFT, NO_DIRECTION}};

Direction gOppositeDirections[DIRECTIONS];

/* stage-based bottom-up solver variables */
POSITION currentHeights[MAXW];

POSITION currentPieces[MAXW];

POSITIONLIST   *currentStage;


/** Function Prototypes **/
POSITION        MyInitialPosition();
POSITION        MyNumberOfPos();

void            CountPieces(POSITION pos, int *xcount, int *ocount);

void            SetHeights(int currentcol, int piecesleft);
void            SetPieces(int currentcol);
void            RecordPosition(POSITION pos, POSITIONLIST *head);
POSITIONLIST   *EnumerateWithinStage(int stage);

int             CountContinuousPieces(int column, int row, Direction horizontalDirection,
                                      Direction verticalDirection);
void            PositionToBoard(POSITION pos, XOBlank board[MAXW][MAXH]);

void            linearUnhash2(POSITION pos, XOBlank* board);
void            InitPieceToNumConvs();
void            UndoMove(MOVE move);
void            SetupTierStuff();
void            positionToBinary(POSITION p);
STRING          MoveToString( MOVE );
POSITION        GetCanonicalPosition(POSITION position);

TIER            PositionToTier(POSITION pos);
TIERPOSITION    PositionToTierPos(POSITION pos, TIER tier);
TIERLIST        *TierChildren(TIER tier);
int             **GeneratePermutations(int x, int y);
BOOLEAN         IsLegal(POSITION pos);
STRING          TierToString(TIER tier);
TIERPOSITION    PiecePermutation(int pcs, int cols);
TIERPOSITION    NumberOfTierPositions(TIER tier);
int             MostSigBit(uint num);
POSITION        ModPosToPosition(POSITION p);
POSITION        PositionToModPos(POSITION p, TIER t);
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
	return MyInitialPosition();
}

void InitializeGame()
{
	unsigned int i, j;
	gNumberOfPositions = MyNumberOfPos();
	gInitialPosition    = MyInitialPosition();
	gEnumerateWithinStage = &EnumerateWithinStage;

	gMinimalPosition = gInitialPosition;

	gOppositeDirections[NO_DIRECTION] = NO_DIRECTION;
	gOppositeDirections[DOWN] = UP;
	gOppositeDirections[LEFT] = RIGHT;
	gOppositeDirections[RIGHT] = LEFT;
	gOppositeDirections[UP] = DOWN;
	SetupTierStuff();
	PositionToBoard(gInitialPosition, gPosition.board);

	for (i = 0; i < WIN4_WIDTH; ++i) {
		gPosition.heights[i] = 0;

		for (j = 0; j < WIN4_HEIGHT; ++j) {
			if (gPosition.board[i][j] == Blank)
				break;

			++gPosition.heights[i];
		}
	}

	gPosition.lastColumn = NO_COLUMN;
	gPosition.nextPiece = x;
	gPosition.piecesPlaced = 0;
	gUndoMove = UndoMove;
	gMoveToStringFunPtr =  &MoveToString;
	gCanonicalPosition = GetCanonicalPosition;
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

	int temp;
	char tChar;

	do {
		printf("?\n\t----- Game-specific options for %s -----\n\n", kGameName);
		printf("\tp)\tContinuous (P)ieces goal (%d)\n", gContinuousPiecesGoal);
		printf("\tw)\tChoose the board (W)idth (%d through %d) Currently: %d\n",MINW,MAXW,WIN4_WIDTH);
		printf("\th)\tChoose the board (H)eight (%d through %d) Currently: %d\n",MINH,MAXH,WIN4_HEIGHT);
		printf("\n\n\tb)\t(B)ack = Return to previous activity.\n");
		if (gLibraries) {
			printf("\tl)\tToggle use of game function libraries. Currently: On");
		} else {
			printf("\tl)\tToggle use of game function libraries. Currently: Off");
		}
		printf("\n\nSelect an option: ");

		switch(GetMyChar()) {


		case 'Q': case 'q':
			ExitStageRight();
			break;
		case 'P': case 'p':
			printf("Enter continuous pieces goal: ");
			gContinuousPiecesGoal = GetMyInt();
			break;
		case 'W': case 'w':
			printf("Enter a width (%d through %d): ",MINW,MAXW);
			temp = GetMyInt();

			while(temp > MAXW || temp < MINW) {
				printf("Out of range\n");
				printf("Enter a width (%d through %d): ",MINW,MAXW);
				tChar = GetMyChar();
				temp = atoi(&tChar);
			}
			WIN4_WIDTH = temp;
			break;
		case 'H': case 'h':
			printf("Enter a height (%d through %d): ",MINH,MAXH);
			temp = GetMyInt();

			while(temp > MAXH || temp < MINH) {
				printf("Out of range\n");
				printf("Enter a height (%d through %d): ",MINH,MAXH);
				tChar = GetMyChar();
				temp = atoi(&tChar);
			}
			WIN4_HEIGHT = temp;
			break;

		case 'L': case 'l':
			if (gLibraries) {
				gLibraries = FALSE;
			} else {
				gLibraries = TRUE;
			}
			break;

		case 'b': case 'B':

			if (gLibraries) {
				LibInitialize(4,WIN4_HEIGHT,WIN4_WIDTH,TRUE);
				Test();
			}

			return;
		default:
			printf("\nSorry, I don't know that option. Try another.\n");
			HitAnyKeyToContinue();
			break;
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

void SetTclCGameSpecificOptions(int theOptions[])

{
	/* No need to have anything here, we have no extra options */
}

/************************************************************************
**
** NAME:        DoMove
**
** DESCRIPTION: Apply the move to the position.
**
** INPUTS:      POSITION position : The old position
**              MOVE     move     : The move to apply.
**
** OUTPUTS:     (POSITION) : The position that results after the move.
**
** CALLS:       XOBlank WhosTurn(POSITION)
**
************************************************************************/

POSITION DoMove(POSITION position, MOVE move)
{
	XOBlank turn,WhoseTurn();
	POSITION turn2;
	int i,free=0;
	POSITION permutation_index, sizebits, remainderbits, temp=1;
	TIER tier; TIERPOSITION tierpos;
	POSITION modpos, bitmask, tierbits, temp1, temp2, temp3=2;

	if (gHashWindowInitialized) {
		gUnhashToTierPosition(position, &tierpos, &tier);
		tierbits = temp << tier;
		permutation_index = position / tierbits;
		tierpos = (NumToPieceDist[tier].convert[permutation_index]);
		tierpos <<= tier;
		modpos = tierpos + (position % tierbits);
		modpos <<= (64 - tier - TIER_COL_BITS);
		position = ModPosToPosition(modpos);

	}

	for (i=move*(WIN4_HEIGHT+1)+WIN4_HEIGHT; (position & (temp1 = (temp << i))) == 0; --i)
		free++;

	if (free == 0) return kBadPosition;

	turn = gUseGPS ? gPosition.nextPiece : WhoseTurn(position);
	turn2 = (POSITION)turn;
	position &= ~(temp1);

	if (gUseGPS) {
		gPosition.board[move][gPosition.heights[move]++] = gPosition.nextPiece;
		gPosition.previousColumn = gPosition.lastColumn;
		gPosition.lastColumn = move;
		gPosition.nextPiece = gPosition.nextPiece == x ? o : x;
		++gPosition.piecesPlaced;
	}

	position = (position | (temp2 = (temp3+turn2)<<i));

	if (gHashWindowInitialized) {
		tier = PositionToTier(position);
		tierbits = temp << tier;
		tierpos = PositionToModPos(position, tier);
		bitmask = tierbits - 1;
		sizebits = tierpos >> (64 - TIER_COL_BITS);
		remainderbits = (tierpos >> (64 - tier - TIER_COL_BITS)) & bitmask;
		permutation_index = PieceDistToNum[tier].convert[sizebits];
		tierpos = permutation_index * tierbits + remainderbits;
		position = gHashToWindowPosition(tierpos, tier);
	}

	return position;
}

void UndoMove(MOVE move)
{
	gPosition.board[move][--gPosition.heights[move]] = Blank;
	gPosition.lastColumn = gPosition.previousColumn;
	gPosition.nextPiece = gPosition.nextPiece == x ? o : x;
	--gPosition.piecesPlaced;
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

void PrintComputersMove(MOVE computersMove,STRING computersName)
{
	printf("%8s's move              : %2d\n", computersName, computersMove+1);
}

/************************************************************************
**
** NAME:        Primitive
**
** DESCRIPTION: Return the value of a position if it fulfills certain
**              'primitive' constraints. Some examples of this is having
**              four-in-a-row. There are two
**              primitives it can immediately check for, when the board
**              is filled but nobody has won = primitive tie. Four in
**              a row is a primitive lose, because the player who faces
**              this board has just lost. I.e. the player before him
**              created the board and won. Otherwise undecided.
**
** INPUTS:      POSITION position : The position to inspect.
**
** OUTPUTS:     (VALUE) an enum which is oneof: (win,lose,tie,undecided)
**
** CALLS:       PositionToBoard()
**
** WARNING:     Behavior undefined for a position which is impossible !!
**
************************************************************************/

VALUE Primitive(POSITION position)
{
	if (!gLibraries) {

		if (gUseGPS) {
			int count, index;
			Direction horizontalDirection, verticalDirection;
			int lastRow = gPosition.heights[gPosition.lastColumn] - 1;

			for (index = 0; index < DIRECTION_PAIRS; ++index) {
				count = 1;
				horizontalDirection = gDirections[index][0];
				verticalDirection = gDirections[index][1];
				count += CountContinuousPieces(gPosition.lastColumn, lastRow,
				                               horizontalDirection, verticalDirection);

				if (count >= gContinuousPiecesGoal)
					return gStandardGame ? lose : win;

				count += CountContinuousPieces(gPosition.lastColumn, lastRow,
				                               gOppositeDirections[horizontalDirection],
				                               gOppositeDirections[verticalDirection]);

				if (count >= gContinuousPiecesGoal)
					return gStandardGame ? lose : win;
			}

			return gPosition.piecesPlaced == WIN4_WIDTH * WIN4_HEIGHT ? tie : undecided;
		}

		int ul[WIN4_WIDTH][WIN4_HEIGHT]; //upper left
		int l[WIN4_WIDTH][WIN4_HEIGHT]; //left
		int ll[WIN4_WIDTH][WIN4_HEIGHT]; //lower left
		int u[WIN4_WIDTH][WIN4_HEIGHT]; //up
		XOBlank board[WIN4_WIDTH][WIN4_HEIGHT+1];
		int col,row;
		PositionToBoard(position, gPosition.board); // Temporary storage.
		for (col=0; col<WIN4_WIDTH; col++)
			board[col][WIN4_HEIGHT]=2;
		for (col=0; col<WIN4_WIDTH; col++)
			for (row=0; row<WIN4_HEIGHT; row++)
				board[col][row] = gPosition.board[col][row];
		// Check for four in a row
		// First do column 0:
		col=0;
		row=WIN4_HEIGHT-1;
		while (row>=0) {
			if (board[0][row]==2) {
				ul[0][row]=0;
				l[0][row]=0;
				ll[0][row]=0;
				u[0][row]=0;
			}
			else {
				ul[0][row]=1;
				l[0][row]=1;
				ll[0][row]=1;
				if (board[0][row+1]==board[0][row]) {
					u[0][row]=u[0][row+1]+1;
					if (u[0][row] == gContinuousPiecesGoal)
						return gStandardGame ? lose : win;
				}
				else u[0][row]=1;
			}
			row--;
		}
		// Now do the other columns
		for (col=1; col<WIN4_WIDTH; col++) {
			row=WIN4_HEIGHT-1;
			while (row>0) {
				if (board[col][row]==2) {
					ul[col][row]=0;
					l[col][row]=0;
					ll[col][row]=0;
					u[col][row]=0;
				}
				else {
					if (board[col][row]==board[col][row+1]) {
						u[col][row]=u[col][row+1]+1;
						if (u[col][row] == gContinuousPiecesGoal)
							return gStandardGame ? lose : win;
					}
					else u[col][row]=1;
					if (board[col][row]==board[col-1][row+1]) {
						ul[col][row]=ul[col-1][row+1]+1;
						if (ul[col][row] == gContinuousPiecesGoal)
							return gStandardGame ? lose : win;
					}
					else ul[col][row]=1;
					if (board[col][row]==board[col-1][row]) {
						l[col][row]=l[col-1][row]+1;
						if (l[col][row] == gContinuousPiecesGoal)
							return gStandardGame ? lose : win;
					}
					else l[col][row]=1;
					if (board[col][row]==board[col-1][row-1]) {
						ll[col][row]=ll[col-1][row-1]+1;
						if (ll[col][row] == gContinuousPiecesGoal)
							return gStandardGame ? lose : win;
					}
					else ll[col][row]=1;
				}
				row--;
			}
			if (board[col][row]==2) {
				ul[col][row]=0;
				l[col][row]=0;
				ll[col][row]=0;
				u[col][row]=0;
			}
			else {
				if (board[col][row]==board[col][row+1]) {
					u[col][row]=u[col][row+1]+1;
					if (u[col][row] == gContinuousPiecesGoal)
						return gStandardGame ? lose : win;
				}
				else u[col][row]=1;
				if (board[col][row]==board[col-1][row+1]) {
					ul[col][row]=ul[col-1][row+1]+1;
					if (ul[col][row] == gContinuousPiecesGoal)
						return gStandardGame ? lose : win;
				}
				else ul[col][row]=1;
				if (board[col][row]==board[col-1][row]) {
					l[col][row]=l[col-1][row]+1;
					if (l[col][row] == gContinuousPiecesGoal)
						return gStandardGame ? lose : win;
				}
				else l[col][row]=1;
				ll[col][row]=1;
			}
		}

		//Now check if the board is full:
		for (col=0; col<WIN4_WIDTH; col++)
			for (row=0; row<WIN4_HEIGHT; row++)
				if (board[col][row]==2) return(undecided);

		return(tie);

	} else {

		int col, row, xx=0, oo=1, bb=2;
		XOBlank linearBoard[WIN4_WIDTH*WIN4_HEIGHT], WhoseTurn();

		if (gUseGPS) {
			int lastRow = gPosition.heights[gPosition.lastColumn];

			//copy the board into 1D representation
			for(row=WIN4_HEIGHT-1; row>=0; row--) {
				for(col=0; col<WIN4_WIDTH; col++) {
					linearBoard[col+WIN4_WIDTH*(WIN4_HEIGHT - 1 - row)] = gPosition.board[col][row];
				}
			}

			if (gPosition.nextPiece == o) {
				if (NinaRow(linearBoard,&xx,gPosition.lastColumn + WIN4_WIDTH*(WIN4_HEIGHT - lastRow),gContinuousPiecesGoal)) {
					return gStandardGame ? lose : win;
				}
			} else { //nextPiece == x
				if (NinaRow(linearBoard,&oo,gPosition.lastColumn + WIN4_WIDTH*(WIN4_HEIGHT - lastRow),gContinuousPiecesGoal)) {
					return gStandardGame ? lose : win;
				}
			}

			return gPosition.piecesPlaced == WIN4_WIDTH * WIN4_HEIGHT ? tie : undecided;
		}

		linearUnhash2(position, linearBoard); // Temporary storage.

		if (WhoseTurn(position) == x) {
			if (statelessNinaRow(linearBoard,&oo,gContinuousPiecesGoal)) {
				return gStandardGame ? lose : win;
			}
		} else { //o's turn
			if (statelessNinaRow(linearBoard,&xx,gContinuousPiecesGoal)) {
				return gStandardGame ? lose : win;
			}
		}

		return amountOfWhat(linearBoard,&bb,1,TRUE) ? undecided : tie;

	}

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
**                                    (not used...)
**
** CALLS:       PositionToBoard()
**              GetValueOfPosition()
**              WhoseTurn()
**              GetPrediction()
**
************************************************************************/

void PrintPosition(POSITION position,STRING playerName,BOOLEAN usersTurn)
{
	int i,row;
	//  VALUE GetValueOfPosition();
	XOBlank board[MAXW][MAXH], WhoseTurn();

	PositionToBoard(position,board);

	printf("\n      ");
	for (i=1; i<=WIN4_WIDTH; i++)
		printf(" %i",i);
	printf("\n      ");
	for (i=1; i<=WIN4_WIDTH; i++)
		printf(" |");
	printf("\n      ");
	for (i=1; i<=WIN4_WIDTH; i++)
		printf(" V");
	for (row=WIN4_HEIGHT-1; row>=0; row--) {
		printf("\n\n      ");
		for (i=0; i<WIN4_WIDTH; i++)
			printf(" %s",gBlankOXString[(int)board[i][row]]);
	}

	//for (i=0; i< 20; i++) {
	//	printf("%d", (int)(position & 1));
	//	position = position >> 1;
	//}

	printf("\n");

	printf("\n\nPrediction: %s",
	       GetPrediction(position,playerName,usersTurn));
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

MOVELIST *GenerateMoves(POSITION position)
{
	MOVELIST *head = NULL;
	int i;

	if (!gUseGPS)
		PositionToBoard(position, gPosition.board); // Temporary storage.

	for(i = 0; i < WIN4_WIDTH; i++) {
		if(gPosition.board[i][WIN4_HEIGHT-1] == Blank)
			head = CreateMovelistNode(i,head);
	}

	return(head);

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

USERINPUT GetAndPrintPlayersMove(POSITION thePosition, MOVE *theMove, STRING playerName)
{
	USERINPUT ret;

	do {
		printf("%8s's move [(u)ndo/1-5] :  ", playerName);

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

BOOLEAN ValidTextInput(STRING input)
{
	return(input[0] <= '9' && input[0] >= '1');
	// The '9' should rather be WIN4_WIDTH, but what the heck...
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

MOVE ConvertTextInputToMove(STRING input)
{
	return((MOVE) input[0] - '1'); /* user input is 1-5, our rep. is 0-4 */
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

void PrintMove(MOVE theMove)
{
	STRING str = MoveToString( theMove );
	printf( "%s", str );
	SafeFree( str );
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

STRING MoveToString (MOVE theMove)
{
	STRING m = (STRING) SafeMalloc( 3 );
	/* The plus 1 is because the user thinks it's 1-9, but MOVE is 0-8 */
	sprintf( m, "%d", theMove + 1);

	return m;
}


/************************************************************************
**
** NAME:        PositionToBoard
**
** DESCRIPTION: convert an internal position to a XOBlank-matrix.
**
** INPUTS:      POSITION thePos   : The position input.
**              XOBlank *board    : The converted XOBlank output matrix.
**
************************************************************************/

void PositionToBoard(POSITION pos, XOBlank board[MAXW][MAXH])
{
	int col, row, h;
	POSITION permutation_index, tierbits, temp1, temp2, temp3=1;
	TIER tier;
	TIERPOSITION tierpos, modpos;

	if (gHashWindowInitialized) {
		gUnhashToTierPosition(pos, &tierpos, &tier);
		tierbits = temp3 << tier;
		permutation_index = pos / tierbits;
		tierpos = (NumToPieceDist[tier].convert[permutation_index]);
		tierpos <<= tier;
		modpos = tierpos + (pos % tierbits);
		modpos <<= (64 - tier - TIER_COL_BITS);
		pos = ModPosToPosition(modpos);
	}


	for (col=0; col<WIN4_WIDTH; col++) {
		row=WIN4_HEIGHT-1;
		for (h=col*(WIN4_HEIGHT+1)+WIN4_HEIGHT;
		     (pos & (temp1 = (temp3 << h))) == 0;
		     h--) {
			// These two lines cause a crash when printing position 0
			board[col][row]=2;
			row--;
		}
		h--;
		while (row >=0) {
			if ((pos & (temp2 = (temp3 << h))) != 0) board[col][row]=1;
			else board[col][row]=0;
			row--;
			h--;
		}
	}
}


void linearUnhash2(POSITION pos, XOBlank board[WIN4_HEIGHT*WIN4_WIDTH]) {
	int col, row, h;
	POSITION permutation_index, tierbits, temp1, temp2, temp3 = 1;
	TIER tier;
	TIERPOSITION tierpos, modpos;

	if (gHashWindowInitialized) {
		gUnhashToTierPosition(pos, &tierpos, &tier);
		tierbits = temp3 << tier;
		permutation_index = pos / tierbits;
		tierpos = (NumToPieceDist[tier].convert[permutation_index]);
		tierpos <<= tier;
		modpos = tierpos + (pos % tierbits);
		modpos <<= (64 - tier - TIER_COL_BITS);
		pos = ModPosToPosition(modpos);
	}



	for (col=0; col<WIN4_WIDTH; col++) {
		row=WIN4_HEIGHT-1;
		for (h=col*(WIN4_HEIGHT+1)+WIN4_HEIGHT;
		     (pos & (temp1 = (temp3 << h))) == 0;
		     h--) {
			board[col + WIN4_WIDTH*(WIN4_HEIGHT - 1 - row)] = 2;
			row--;
		}
		h--;
		while (row >=0) {
			if ((pos & (temp2 = (temp3<<h))) != 0) board[col + WIN4_WIDTH*(WIN4_HEIGHT - 1 - row)] = 1;
			else board[col + WIN4_WIDTH*(WIN4_HEIGHT - 1 - row)] = 0;
			row--;
			h--;
		}
	}
}


/************************************************************************
**
** NAME:        WhoseTurn
**
** DESCRIPTION: Return whose turn it is - either x or o. Since x always
**              goes first, we know that if the board has an equal number
**              of x's and o's, that it's x's turn. Otherwise it's o's.
**
** INPUTS:      POSITION  : a position
**
** OUTPUTS:     (XOBlank) : Either x or o, depending on whose turn it is
**
************************************************************************/

XOBlank WhoseTurn(POSITION pos)
{
	int xcount = 0,ocount = 0;

	CountPieces(pos, &xcount, &ocount);

	if(xcount == ocount)
		return(x);            /* x always goes first */
	else
		return(o);
}


/************************************************************************
**
** NAME:        MyInitialPosition
**
** DESCRIPTION: Calculates the initial position
**
** INPUTS:      none
**
** OUTPUTS:     POSITION (Initial Position)
**
************************************************************************/

POSITION MyInitialPosition()
{
	POSITION p=1;
	int i;
	for (i=1; i<WIN4_WIDTH; ++i)
		p = (p << (WIN4_HEIGHT+1))+1;
	return p;
}

/************************************************************************
**
** NAME:        MyNumberOfPos()
**
** DESCRIPTION: Calculates an upper bound for a position
**
** INPUTS:      none
**
** OUTPUTS:     unsigned long int (number)
**
************************************************************************/
POSITION MyNumberOfPos()
{
	int i;
	POSITION size=1;
	for (i=0; i<(WIN4_HEIGHT+1)*WIN4_WIDTH; i++)
		size *= 2;
	return size;
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

	do {
		printf("\n\t----- Module DEBUGGER for %s -----\n\n", kGameName);
		printf("\n\n\tb)\t(B)ack = Return to previous activity.\n");
		printf("\n\nSelect an option: ");

		switch(GetMyChar()) {
		case 'Q': case 'q':
			ExitStageRight();
		case 'H': case 'h':
			HelpMenus();
			break;
		case 'B': case 'b':
			return;
		default:
			BadMenuChoice();
			HitAnyKeyToContinue();
			break;
		}
	} while(TRUE);
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

int CountContinuousPieces(int column, int row, Direction horizontalDirection,
                          Direction verticalDirection) {
	int count = -1;
	XOBlank piece;

	do {
		++count;
		piece = gPosition.board[column][row];

		switch (horizontalDirection) {
		case LEFT:
			--column;
			break;
		case RIGHT:
			++column;
			break;
		default:
			;
		}

		switch (verticalDirection) {
		case DOWN:
			--row;
			break;
		case UP:
			++row;
			break;
		default:
			;
		}
	}
	while (column >= 0 && column < WIN4_WIDTH && row >= 0 && row < WIN4_HEIGHT &&
	       gPosition.board[column][row] == piece);

	return count;
}

void CountPieces(POSITION pos, int *xcount, int *ocount)
{
	int row, col, h;
	POSITION temp1, temp2, temp3=1;
	for (col=0; col<WIN4_WIDTH; col++) {
		row=WIN4_HEIGHT-1;
		for (h=col*(WIN4_HEIGHT+1)+WIN4_HEIGHT; (pos & (temp1=(temp3 << h)))==0; h--)
			row--;
		h--;
		while (row >=0) {
			if ((pos & (temp2=(temp3<<h))) != 0) (*ocount)++;
			else (*xcount)++;
			row--;
			h--;
		}
	}
}

/* bottom-up solver support functions */

int min(int a, int b)
{
	if (a < b)
		return a;
	return b;
}


void SetPieces(int currentcol)
{
	int xcount = 0, ocount = 0, i;
	POSITION pos = 0, temp1, temp2, temp3=1;

	if (currentcol > WIN4_WIDTH) {
		for(i=0; i<WIN4_WIDTH; i++)
			pos |= (currentPieces[MAXW] << (i*(WIN4_HEIGHT+1)));
		CountPieces(pos, &xcount, &ocount);
		if ((xcount == ocount) || (xcount == ocount+1)) { //seems like a valid pos
			StorePositionInList(pos, currentStage);

			for (i=0; i< 20; i++) {
				printf("%d", (int)(pos & 1));
				pos = pos >> 1;
			}

		}
	} else {
		for (currentPieces[currentcol] = (temp1 = (temp3 << currentHeights[currentcol]));
		     currentPieces[currentcol] < (temp2 = (temp3 << (currentHeights[currentcol]+1)));
		     currentPieces[currentcol]++)
			SetPieces(currentcol + 1);
	}
}

void SetHeights(int currentcol, int piecesleft)
{
	if ((currentcol > WIN4_WIDTH) && (piecesleft == 0)) {
		SetPieces(0);
	} else {
		for (currentHeights[currentcol] = min(WIN4_HEIGHT, piecesleft);
		     currentHeights[currentcol] >= 0;
		     currentHeights[currentcol]--) {
			SetHeights(currentcol + 1, piecesleft - currentHeights[currentcol]);
		}
	}
}


//take the stage, generate all combinations of the columns, and then hash it, and append it to the list.
//using an array and doubling length when expanding size for ammortized O(n) running time
POSITIONLIST *EnumerateWithinStage(int stage) {

	/*
	   if(stage == TOTALSTAGE)
	        return head;
	 */

	assert(currentStage == NULL);

	SetHeights(0, stage);

	//gotta free this in the bottom up solver.
	return currentStage;
}


/************************************************************************
**
** NAME:        array_symm
**
** DESCRIPTION: Takes a 1D position array, imagines it as a
**                2D [column][row] positon array, and swaps
**                columns around the middle.
**
** INPUTS:      short* bits - 1D positon array
**              short  columns - # of columns on the board
**
** OUTPUTS:     "bits" is modified to represent a mirror
**                reflection of the original position.
**
************************************************************************/
void array_symm(short* bits, short columns){
	short a, t, i = -1, j = columns;
	short col_sz = WIN4_HEIGHT + 1;
	while (++i < --j) {
		for (a = 0; a < (WIN4_HEIGHT + 1); a++) {
			t = bits[i * col_sz + a];
			bits[i * col_sz + a] = bits[j * col_sz + a];
			bits[j * col_sz + a] = t;
		}
	}
}

/************************************************************************
**
** NAME:        GetCanonicalPosition
**
** DESCRIPTION: Looks at a position and returns its canonical form
**
** INPUTS:      POSITION p: a passed position
**
** OUTPUTS:     POSITION  : position's canonical form
**
************************************************************************/
POSITION GetCanonicalPosition(POSITION p){
	POSITION temp, hold; // new, hold;
	int column = WIN4_HEIGHT + 1;
	short i, size = column * WIN4_WIDTH;
	short bits[size];

	//Copy position into an array
	temp = p;
	//                               Copy the orig position into an array
	hold = temp;
	for (i = 0; i < size; i++) {
		bits[i] = temp & 1;
		temp = temp >> 1;
	}
	temp = hold;
	//                               Process an array
	array_symm(bits, WIN4_WIDTH );
	//                               Copy an array into the new position
	temp = 0;
	for (i = size-1; i >= 0; i--) {
		temp = temp | bits[i];
		if (i != 0)
			temp = temp << 1;
	}
	return ((temp < p) ? temp : p); // Choose the smallest position
}



/******************************* MAXIMIZATION IMPLEMENTATION *****************************/

/* Initialize Tier Stuff. */
void SetupTierStuff() {
	kSupportsTierGamesman = TRUE;
	gTierChildrenFunPtr             = &TierChildren;
	gNumberOfTierPositionsFunPtr    = &NumberOfTierPositions;
	gTierToStringFunPtr             = &TierToString;
	gInitialTier = 0;
	gInitialTierPosition = 0;
	COLSIGBITS = MostSigBit(WIN4_HEIGHT);
	TIER_COL_BITS = WIN4_WIDTH * COLSIGBITS;
	WIN4_WIDTH_MINUS_ONE = WIN4_WIDTH - 1;
	WIN4_HEIGHT_PLUS_ONE = WIN4_HEIGHT + 1;
	WIN4_WIDTH_PLUS_ONE = WIN4_WIDTH + 1;

	NumToPieceDist = (NumToPieceConv *)(SafeMalloc(sizeof(NumToPieceConv)*(WIN4_WIDTH*WIN4_HEIGHT+1)));
	PieceDistToNum = (NumToPieceConv *)(SafeMalloc(sizeof(NumToPieceConv)*(WIN4_WIDTH*WIN4_HEIGHT+1)));
	InitPieceToNumConvs();
}


/* Returns list of all children for a particular tier. */
TIERLIST* TierChildren(TIER tier) {
	if (tier<WIN4_WIDTH*WIN4_HEIGHT)
		return CreateTierlistNode(tier+1, NULL);
	else
		return NULL;
}

/* Returns the number of positions associated with a particular tier. */
TIERPOSITION NumberOfTierPositions(TIER tier) {
	return PiecePermutation(tier, WIN4_WIDTH) * (1 << tier);
}

/* Given a position, returns the tier number of it (i.e. number of pieces placed on the board) */
TIER PositionToTier(POSITION position) {
	int bitmask = (1 << WIN4_HEIGHT_PLUS_ONE ) - 1, num_pieces = 0, i;

	for (i=0; i<WIN4_WIDTH; i++) {
		num_pieces += MostSigBit(bitmask & position) - 1;
		position >>= WIN4_HEIGHT_PLUS_ONE;
	}

	return num_pieces;
}

/* Given a number, returns the position of the most significant bit. */
int MostSigBit(uint num) {
	int k=0;

	while (num > 0) {
		num >>= 1;
		k++;
	}

	return k;
}

/* Initializes hashes that go from position number to piece distributions. */
void InitPieceToNumConvs() {
	int tiernum, num_dists, **board_confs, bitrep, j, k;

	for (tiernum=0; tiernum <= WIN4_WIDTH*WIN4_HEIGHT; tiernum++) {
		num_dists = PiecePermutation(tiernum, WIN4_WIDTH);
		NumToPieceDist[tiernum].convert = (int *)(SafeMalloc(sizeof(int) * num_dists));
		PieceDistToNum[tiernum].convert = (int *)(SafeMalloc(sizeof(int) * (1 << (COLSIGBITS * WIN4_WIDTH))));
		board_confs = GeneratePermutations(tiernum, WIN4_WIDTH);

		for (j=0; j<num_dists; j++) {
			bitrep = 0;

			for (k=0; k<WIN4_WIDTH; k++) {
				bitrep += board_confs[j][k];

				if (k<WIN4_WIDTH-1)
					bitrep <<= COLSIGBITS;
			}

			SafeFree(board_confs[j]);
			NumToPieceDist[tiernum].convert[j] = bitrep;
			PieceDistToNum[tiernum].convert[bitrep] = j;
		}
		SafeFree(board_confs);
	}
}

/* Calculates the number of ways X pieces can be distributed among Y columns. */
TIERPOSITION PiecePermutation(int num_pieces, int num_cols) {
	int i, num_ways = 0;

	if (num_pieces < 0)
		return 0;

	if (num_cols == 0 && num_pieces > 0)
		return 0;

	if (num_pieces == 0)
		return 1;


	if (num_cols == 1 && num_pieces > WIN4_HEIGHT)
		return 0;

	for (i=0; i<=WIN4_HEIGHT; i++)
		num_ways += PiecePermutation(num_pieces-i, num_cols-1);

	return num_ways;
}

/* Generates a list of permutations for distributing X pieces among Y columns. */
int **GeneratePermutations(int num_pieces, int num_cols) {
	int i, j, k, perm=0, num_perms = PiecePermutation(num_pieces, num_cols);
	int **sub_perm_list, **perm_list;

	if (!num_perms)
		return NULL;

	perm_list = (int **)(SafeMalloc(sizeof(int *) * num_perms));

	for (k=0; k<num_perms; k++)
		perm_list[k] = (int *)(SafeMalloc(sizeof(int) * num_cols));

	if (num_cols == 1) {
		perm_list[perm][0] = num_pieces;
		return perm_list;
	}

	for (i=0; i<=WIN4_HEIGHT; i++) {
		num_perms = PiecePermutation(num_pieces-i, num_cols-1);
		sub_perm_list = GeneratePermutations(num_pieces - i, num_cols - 1);

		if (sub_perm_list) {
			for (k=0; k<num_perms; k++) {
				perm_list[perm][0]=i;

				for (j=0; j<num_cols-1; j++)
					perm_list[perm][j+1] = sub_perm_list[k][j];

				SafeFree(sub_perm_list[k]);
				perm++;
			}
			SafeFree(sub_perm_list);
		}
	}

	return perm_list;
}

POSITION ModPosToPosition(POSITION modpos) {
	int i, offset=0, col_heights[WIN4_WIDTH];
	POSITION bitmask, pos=0, modpos_cpy = modpos << TIER_COL_BITS;

	modpos >>= 64 - TIER_COL_BITS;
	bitmask = (1 << COLSIGBITS) - 1;

	for (i=0; i<WIN4_WIDTH; i++) {
		col_heights[WIN4_WIDTH_MINUS_ONE - i] = bitmask & modpos;
		modpos >>= COLSIGBITS;
	}

	for (i=0; i<WIN4_WIDTH; i++) {
		modpos = modpos_cpy;
		offset += col_heights[i];
		modpos >>= (64 - offset);
		bitmask = (1 << col_heights[i]) - 1;
		modpos &= bitmask;
		modpos += 1 << col_heights[i];
		pos += modpos << (i * WIN4_HEIGHT_PLUS_ONE);
	}

	return pos;
}


POSITION PositionToModPos(POSITION position, TIER tier) {
	int total_bits = TIER_COL_BITS + tier, i, num_pieces_col;
	TIERPOSITION modpos=0;
	POSITION position_cpy = position;
	POSITION bitmask = (1 << WIN4_HEIGHT_PLUS_ONE) - 1;

	for (i=0; i<WIN4_WIDTH_MINUS_ONE; i++) {
		modpos += MostSigBit(bitmask & position) - 1;
		position >>= WIN4_HEIGHT_PLUS_ONE;
		modpos <<= COLSIGBITS;
	}

	modpos += MostSigBit(bitmask & position) - 1;

	for (i=0; i<WIN4_WIDTH; i++) {
		num_pieces_col = MostSigBit(bitmask & position_cpy) - 1;

		if (!num_pieces_col) {
			position_cpy >>= WIN4_HEIGHT_PLUS_ONE;
			continue;
		}

		bitmask = (1 << num_pieces_col) - 1;
		modpos <<= num_pieces_col;
		modpos += bitmask & position_cpy;
		position_cpy >>= WIN4_HEIGHT_PLUS_ONE;
		bitmask = (1 << WIN4_HEIGHT_PLUS_ONE) - 1;
	}

	return modpos << (64 - total_bits);
}



/* Converts Tier Number to a String. */
STRING TierToString(TIER tier) {
	STRING str = (STRING)(SafeMalloc(sizeof(STRING)*15));
	sprintf(str, "%d pieces\n", (int)tier);
	return str;
}


/* Given a 64-bit position, prints out its binary representation.*/
void positionToBinary(POSITION pos) {
	char *board = (char *)SafeMalloc(sizeof(char)*65+1);
	int i=0;

	for (; i<64; i++) {
		if (pos % 2)
			board[63-i]='1';
		else
			board[63-i]='0';

		pos = pos/2;
	}

	board[64]='\n';
	board[65]='\0';

	printf(board);
	SafeFree(board);
}



POSITION StringToPosition(char* board) {
	// FIXME: this is just a stub
	return atoi(board);
}


char* PositionToString(POSITION pos) {
	// FIXME: this is just a stub
	return "Implement Me";
}

//GM_DEFINE_BLANKOX_ENUM_BOARDSTRINGS()

char * PositionToEndData(POSITION pos) {
	return NULL;
}
