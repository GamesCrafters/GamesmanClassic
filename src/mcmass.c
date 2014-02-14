#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gamesman.h"


void stringsandwich( char* A, char* B, char* C );
void APPEND(char* C, char c);

#define kMaxNeighbors 10

typedef unsigned char bool;
typedef int Player;

#define false FALSE
#define true TRUE


typedef struct {
	int piececount;
	Player Owner;
	Player queueOwner;
	int queue;

	int neighborcount;
	struct Node* Neighbors[kMaxNeighbors];
}Node;


void InitNode( Node* this );
Node* NewNode(void);
void DeleteNode( Node* this );

void AddPiece( Node* this, Player );
bool AddNeighbor( Node* theNode, Node* NewNeighbor );
void AddPieceToQueue( Node* this, Player );
bool Burst( Node* this );
bool TransferQueue( Node* this );

void ConnectBoardWithDiag( Node* theNodes, int Rows, int Columns );
void ConnectBoardCardinal( Node* theNodes, int Rows, int Columns );
void InitBoard( Node* theBoard, int Size );
void PrintBoard( Node* theBoard, int Rows, int Columns );
void SprintBoard( char* theBuffer, Node* theBoard, int Rows, int Columns );

char GetPlayerCharacter( Player i );

void UpdateBoard( Node* theBoard, int nodecount );

void UpdateBoard( Node* theBoard, int nodecount );
bool BurstBoardIfNeeded( Node* theBoard, int nodecount );
Player Winner( Node* theBoard, int nodecount );


POSITION EncodeBoard( Node* theBoard, long NodeCount );
Player DecodeBoard( Node* theBoard, long NodeCount, POSITION inPos );
long GetTotalCombinations( Node* theBoard, int NodeCount );


/*for a 3x3 board, it's 5 possible in the corners, 7 in the edges and 9 in the middle * 2 possible WhoseTurn's*/
/*5*5*5*5*7*7*7*7*9*2 = 27,011,250*/

POSITION gNumberOfPositions  = 27011250;
//This is changed later in the actual code, here it's set nice and high for robustness sake

POSITION gInitialPosition    =  0;
POSITION gMinimalPosition    = 0;

STRING kAuthorName         = "Peterson Tretheway";
STRING kGameName           = "Critical Mass";
BOOLEAN kPartizan           = TRUE;
BOOLEAN kDebugMenu          = TRUE;
BOOLEAN kGameSpecificMenu   = TRUE;
BOOLEAN kTieIsPossible      = FALSE;
BOOLEAN kLoopy               = FALSE;
BOOLEAN kDebugDetermineValue = FALSE;
POSITION kBadPosition           = -1;
void*    gGameSpecificTclInit = NULL;

STRING kHelpGraphicInterface =
        "Nothing yet";

STRING kHelpTextInterface    =
        "Players alternate turns placing pieces in one of the squares that do not \n\
contain the opponent's piece. When the box is filled, as shown by the counter \n\
next to the player's piece, the slot explodes. One piece remains in the \n\
original spot, while the others  are distributed to each adjacent square. \n\
If the spot is occupied by the opponent, their piece is removed and replaced \n\
by your piece."                                                                                                                                                                                                                                                                                                                                                                                                                   ;

STRING kHelpOnYourTurn =
        "Select the desired spot to place your piece by using the legend. A counter \n\
next to your piece keeps track of how many of your pieces are in the spot. "                                                                                        ;

STRING kHelpStandardObjective =
        "To occupy the whole board.";

STRING kHelpReverseObjective =
        "To be the first player to completely remove all his or her pieces.";

STRING kHelpTieOccursWhen =   /* Should follow 'A Tie occurs when... */
                            "A tie is not possible";

STRING kHelpExample =
        "It's not rocket science.";

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

#define MIN_WIDTH 1
#define MAX_WIDTH 4
#define MIN_HEIGHT 1
#define MAX_HEIGHT 4

int gBoardWidth  = 3;
int gBoardHeight = 3;

#define BOARDSIZE ((gBoardWidth)*(gBoardHeight))

#define CONNECTBOARD ConnectBoardCardinal

/**/


#define NUMSYMMETRIES 1           /* 4 rotations, 4 flipped rotations */




/*All stuff below is to let residual ttt code compile*/


typedef enum possibleBoardPieces {
	Blank, o, x
} BlankOX;

char *gBlankOXString[] = { "-", "O", "X" };

/* Powers of 3 - this is the way I encode the position, as an integer */
int g3Array[] = { 1, 3, 9, 27, 81, 243, 729, 2187, 6561 };


/*I'm just keeping this stuff for the time being cuz it wont compile without it*/

/* This is the array used for flipping along the N-S axis */
int gFlipNewPosition[] = { 2, 1, 0, 5, 4, 3, 8, 7, 6 };

/* This is the array used for rotating 90 degrees clockwise */
int gRotate90CWNewPosition[] = { 6, 3, 0, 7, 4, 1, 8, 5, 2 };

/************************************************************************
**
** NAME:        InitializeDatabases
**
** DESCRIPTION: Initialize the gDatabase, a global variable.
**
************************************************************************/


void InitializeGame()
{
	int i;
	Node* theBoard;

	theBoard = (Node*)SafeMalloc( sizeof(Node)*BOARDSIZE );

	InitBoard( theBoard,BOARDSIZE);
	CONNECTBOARD( theBoard, gBoardHeight, gBoardWidth );

	gNumberOfPositions = 1;

	for( i=0; i<BOARDSIZE; i++ )
	{
		gNumberOfPositions*=((theBoard[i].neighborcount)*2+1);
	}
	gNumberOfPositions*=2;

	free( theBoard );

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
#if 0

	/*I don't know what this is for so I'm commenting it out.*/
	char GetMyChar();

	do {
		printf("\n\t----- Module DEBUGGER for %s -----\n\n", kGameName);

		printf("\tc)\tWrite PPM to s(C)reen\n");
		printf("\ti)\tWrite PPM to f(I)le\n");
		printf("\ts)\tWrite Postscript to (S)creen\n");
		printf("\tf)\tWrite Postscript to (F)ile\n");
		printf("\n\n\tb)\t(B)ack = Return to previous activity.\n");
		printf("\n\nSelect an option: ");

		switch(GetMyChar()) {
		case 'Q': case 'q':
			ExitStageRight();
		case 'H': case 'h':
			HelpMenus();
			break;
		case 'C': case 'c': /* Write PPM to s(C)reen */
			tttppm(0,0);
			break;
		case 'I': case 'i': /* Write PPM to f(I)le */
			tttppm(0,1);
			break;
		case 'S': case 's': /* Write Postscript to (S)creen */
			tttppm(1,0);
			break;
		case 'F': case 'f': /* Write Postscript to (F)ile */
			tttppm(1,1);
			break;
		case 'B': case 'b':
			return;
		default:
			BadMenuChoice();
			HitAnyKeyToContinue();
			break;
		}
	} while(TRUE);


  #endif
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

	do {
		printf("\n\t----- Game Specific options for %s -----\n\n", kGameName);

		printf("\n\t Current board size is %dx%d\n", gBoardHeight, gBoardWidth );

		printf("\tw)\t to change the board width\n");
		printf("\th)\t to change the board height\n");

		printf("\n\tb)\t(B)ack = Return to previous activity.\n");
		printf("\n\nSelect an option: ");

		switch(GetMyChar())
		{
		case 'Q':
		case 'q':
			ExitStageRight();
			break;

		case 'h':
			printf( "Enter a board height (%d..%d) ->", MIN_HEIGHT, MAX_HEIGHT );
			/*fflush(stdin);		no longer needed
			   scanf( "%d", &gBoardHeight );*/
			gBoardHeight = GetMyInt();
			break;

		case 'w':
			printf( "Enter a board width (%d..%d) ->", MIN_WIDTH, MAX_WIDTH);
			/*fflush(stdin);		no longer needed
			   scanf( "%d", &gBoardWidth );*/
			break;

		case 'B': case 'b':
			return;
		default:
			BadMenuChoice();
			HitAnyKeyToContinue();
			break;
		}

		/*fflush(stdin);	no longer needed*/
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
	if( theOptions[0] ) //2x2
	{
		gBoardWidth = 2; gBoardHeight = 2;
	}
	else if( theOptions[1] ) //2x3
	{
		gBoardWidth = 2; gBoardHeight = 3;
	}
	else if( theOptions[2] ) //2x4
	{
		gBoardWidth = 2; gBoardHeight = 4;
	}
	else if( theOptions[3] ) //3x3
	{
		gBoardWidth = 3; gBoardHeight = 3;
	}
	else //default 2x2 because it's faster
	{
		gBoardWidth = 2; gBoardHeight = 2;
	}
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
** CALLS:       InitBoard
                CONNECTBOARD
                DecodeBoard
                AddPiece
                BurstBoardIfNeeded
                EncodeBoard
**
************************************************************************/

POSITION DoMove(thePosition, theMove)
POSITION thePosition;
MOVE theMove;
{
	POSITION realPosition=0;
	POSITION outPosition=0;
	Player WhoseTurnItIs = 1;
	long totalcombos = 0;
	static int lastboardsize = 0;
	static Node* theBoard = NULL;


	if( lastboardsize!=BOARDSIZE || theBoard == NULL )
	{
		theBoard = (Node*)SafeMalloc( sizeof(Node)*BOARDSIZE );
		lastboardsize = BOARDSIZE;
	}


	InitBoard(theBoard, BOARDSIZE);
	CONNECTBOARD( theBoard, gBoardHeight, gBoardWidth );

	totalcombos = GetTotalCombinations( theBoard, BOARDSIZE );


	if( thePosition>=totalcombos ) //then we do this to see whose turn it is
	{
		realPosition = thePosition-totalcombos; //to indicate that it's player 2's turn, we add a really big number
		WhoseTurnItIs = 2;
	}
	else
	{
		realPosition = thePosition;
		WhoseTurnItIs = 1;
	}

	DecodeBoard( theBoard, BOARDSIZE, realPosition ); //Decode the position
	AddPiece( &(theBoard[theMove]), WhoseTurnItIs ); //make the move and
	while( BurstBoardIfNeeded( theBoard, BOARDSIZE ) ) {} //compute the resulting board state

	outPosition = EncodeBoard( theBoard, BOARDSIZE ); //make the outPosition to the coded new Board

	//if it's player 1's turn make it player 2's turn
	if( WhoseTurnItIs == 1 ) //I don't know whether this will work.  It should make the turn alternate.
	{
		outPosition+=totalcombos;
	}

	return outPosition;
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
	return((POSITION)0);
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
	printf("%8s's move              : %2d\n", computersName, computersMove+1);
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
** OUTPUTS:     (VALUE) an enum which is one of: (win,lose,tie,undecided)
**
** CALLS:       InitBoard
                CONNECTBOARD
                DecodeBoard
                Winner
**
************************************************************************/

VALUE Primitive(position)
POSITION position;
{
	VALUE outValue;
	Player theWinner;
	Player WhoseTurnItIs = 0;
	long totalcombos=0;
	static int lastboardsize = 0;
	static Node* theBoard = NULL;

	if( lastboardsize!=BOARDSIZE || theBoard == NULL )
	{
		if( theBoard ) free( theBoard );
		theBoard = (Node*)SafeMalloc( sizeof(Node)*BOARDSIZE );
		lastboardsize = BOARDSIZE;
	}

	InitBoard(theBoard, BOARDSIZE);
	CONNECTBOARD( theBoard, gBoardHeight, gBoardWidth );

	totalcombos = GetTotalCombinations( theBoard, BOARDSIZE );
	if( position>=totalcombos )
	{
		position-=totalcombos;
		WhoseTurnItIs = 2;
	}
	else
	{
		WhoseTurnItIs = 1;
	}

	DecodeBoard( theBoard, BOARDSIZE, position );

	theWinner = Winner(theBoard,BOARDSIZE);

	if( theWinner != 0 )
	{
		if( gStandardGame )
		{
			if( theWinner == WhoseTurnItIs )
				outValue = win;
			else
				outValue = lose;
		}
		else
		{
			if( theWinner == WhoseTurnItIs )
				outValue = lose;
			else
				outValue = win;
		}
	}
	else
		outValue = undecided;

	return outValue;
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
	char theBuffer[1000];
	char theInst[1000];
	char theSand[1000];
	int x=0,y=0;
	int i=0;
	static int lastboardsize = 0;
	static Node* theBoard = NULL;

	if( lastboardsize!=BOARDSIZE || theBoard == NULL )
	{
		if(theBoard) free(theBoard);
		theBoard = (Node*)SafeMalloc( sizeof(Node)*BOARDSIZE );
		lastboardsize = BOARDSIZE;
	}

	InitBoard(theBoard,BOARDSIZE);
	CONNECTBOARD( theBoard, gBoardHeight, gBoardWidth );

	theBuffer[0]=0;
	theSand[0]= 0;

	DecodeBoard( theBoard, BOARDSIZE, position );
	SprintBoard( theBuffer, theBoard, gBoardHeight, gBoardWidth );

	/*printf up the instructions in a sepereate string*/
	theInst[0]=0;
	i = 1;

	for( y = 0; y<gBoardHeight; y++ )
	{
		sprintf( theInst+strlen(theInst), "( " );
		for( x = 0; x<gBoardWidth; x++ )
		{
			sprintf( theInst+strlen(theInst), "%d ", i );
			i++;
		}
		sprintf( theInst+strlen(theInst), ")       \n" );
	}

	stringsandwich( theInst, theBuffer, theSand );
	printf( theSand );

	printf( "\n\n" );
	printf( "%s", (char*)GetPrediction(position,playerName,usersTurn) );
	printf( "\n\n" );
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

MOVELIST *GenerateMoves(position)
POSITION position;
{
	MOVELIST *CreateMovelistNode(), *head = NULL;
	VALUE Primitive();
	int i;
	Player WhoseTurnItIs=0;
	POSITION realPosition;
	long totalcombos=0;
	static int lastboardsize = 0;
	static Node* theBoard = NULL;

	if( lastboardsize!=BOARDSIZE || theBoard == NULL )
	{
		if(theBoard) free(theBoard);
		theBoard = (Node*)SafeMalloc( sizeof(Node)*BOARDSIZE );
		lastboardsize = BOARDSIZE;
	}

	InitBoard(theBoard,BOARDSIZE);
	CONNECTBOARD( theBoard, gBoardHeight, gBoardWidth );

	totalcombos = GetTotalCombinations( theBoard, BOARDSIZE );

	if( position>=totalcombos ) //then we do this to see whose turn it is
	{
		realPosition = position-totalcombos; //to indicate that it's player 2's turn, we add a really big number
		WhoseTurnItIs = 2;
	}
	else
	{
		realPosition = position;
		WhoseTurnItIs = 1;
	}

	DecodeBoard( theBoard, BOARDSIZE, realPosition );


	if( Primitive(position) == undecided )
	{
		for(i = 0; i<BOARDSIZE; i++)
		{
			if( theBoard[i].Owner == 0 ||
			    theBoard[i].Owner == WhoseTurnItIs )
				head = CreateMovelistNode( i,head );
		}
		return(head);
	}
	else
	{
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

USERINPUT GetAndPrintPlayersMove(thePosition, theMove, playerName)
POSITION thePosition;
MOVE *theMove;
STRING playerName;
{
	BOOLEAN ValidMove();
	USERINPUT ret, HandleDefaultTextInput();


	do {

		printf("%s's move [(u)ndo/1-%d] :  ", playerName, BOARDSIZE);

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
	return(input[0] <= '9' && input[0] >= '1');
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
	return((MOVE) input[0] - '1'); /* user input is 1-9, our rep. is 0-8 */
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
	STRING move = (STRING) SafeMalloc(3);

	/* The plus 1 is because the user thinks it's 1-9, but MOVE is 0-8 */
	sprintf( move, "%d", theMove+1 );

	return move;
}


/************************************************************************
**
** NAME:        PositionToBlankOX
**
** DESCRIPTION: convert an internal position to that of a BlankOX.
**
** INPUTS:      POSITION thePos     : The position input.
**              BlankOX *theBlankOx : The converted BlankOX output array.
**
** CALLS:       BadElse()
**
************************************************************************/
/* Not really used in this game. (thankfully)
   void PositionToBlankOX(thePos,theBlankOX)
     POSITION thePos;
     BlankOX *theBlankOX;
   {
   int i;
   for(i = 8; i >= 0; i--) {
    if(thePos >= ((int)x * g3Array[i])) {
      theBlankOX[i] = x;
      thePos -= (int)x * g3Array[i];
    }
    else if(thePos >= ((int)o * g3Array[i])) {
      theBlankOX[i] = o;
      thePos -= (int)o * g3Array[i];
    }
    else if(thePos >= ((int)Blank * g3Array[i])) {
      theBlankOX[i] = Blank;
      thePos -= (int)Blank * g3Array[i];
    }
    else
      BadElse("PositionToBlankOX");
   }
   }
 */
/************************************************************************
**
** NAME:        BlankOXToPosition
**
** DESCRIPTION: convert a BlankOX to that of an internal position.
**
** INPUTS:      BlankOX *theBlankOx : The converted BlankOX output array.
**
** OUTPUTS:     POSITION: The equivalent position given the BlankOX.
**
************************************************************************/

POSITION BlankOXToPosition(theBlankOX)
BlankOX *theBlankOX;
{
	int i;
	POSITION position = 0;

	for(i = 0; i < BOARDSIZE; i++)
		position += g3Array[i] * (int)theBlankOX[i]; /* was (int)position... */

	return(position);
}

/************************************************************************
**
** NAME:        ThreeInARow
**
** DESCRIPTION: Return TRUE iff there are three-in-a-row.
**
** INPUTS:      BlankOX theBlankOX[BOARDSIZE] : The BlankOX array.
**              int a,b,c                     : The 3 positions to check.
**
** OUTPUTS:     (BOOLEAN) TRUE iff there are three-in-a-row.
**
************************************************************************/

BOOLEAN ThreeInARow(theBlankOX,a,b,c)
BlankOX theBlankOX[];
int a,b,c;
{
	return(theBlankOX[a] == theBlankOX[b] &&
	       theBlankOX[b] == theBlankOX[c] &&
	       theBlankOX[c] != Blank );
}

/************************************************************************
**
** NAME:        AllFilledIn
**
** DESCRIPTION: Return TRUE iff all the blanks are filled in.
**
** INPUTS:      BlankOX theBlankOX[BOARDSIZE] : The BlankOX array.
**
** OUTPUTS:     (BOOLEAN) TRUE iff all the blanks are filled in.
**
************************************************************************/

BOOLEAN AllFilledIn(theBlankOX)
BlankOX theBlankOX[];
{
	BOOLEAN answer = TRUE;
	int i;

	for(i = 0; i < BOARDSIZE; i++)
		answer &= (theBlankOX[i] == o || theBlankOX[i] == x);

	return(answer);
}

/************************************************************************
**
** NAME:        WhoseTurn
**
** DESCRIPTION: Return whose turn it is - either x or o. Since x always
**              goes first, we know that if the board has an equal number
**              of x's and o's, that it's x's turn. Otherwise it's o's.
**
** INPUTS:      BlankOX theBlankOX : The input board
**
** OUTPUTS:     (BlankOX) : Either x or o, depending on whose turn it is
**
************************************************************************/

BlankOX WhoseTurn(theBlankOX)
BlankOX *theBlankOX;
{
	int i, xcount = 0, ocount = 0;

	for(i = 0; i < BOARDSIZE; i++)
		if(theBlankOX[i] == x)
			xcount++;
		else if(theBlankOX[i] == o)
			ocount++;
		else ;    /* don't count blanks */

	if(xcount == ocount)
		return(x);  /* in our TicTacToe, x always goes first */
	else
		return(o);
}






void InitNode( Node* this )
{
	int i;

	this->neighborcount = 0;
	this->queue = 0;
	for( i=0; i<kMaxNeighbors; i++ )
	{
		this->Neighbors[i] = NULL;
	}
	this->piececount=0;
	this->Owner=0;
}


Node* NewNode(void)
{
	return (Node*)(malloc(sizeof( Node )));
}


void PrintNodes( Node* theNodes, int NodeCount )
{
	int i;

	for( i=0; i<NodeCount; i++ )
	{
		// what happened to this printf?
	}
}



void AddPiece( Node* this, Player thePlayer )
{
	this->piececount++;
	this->Owner=thePlayer;
}



void  AddPieceToQueue( Node* this, Player thePlayer )
{
	this->queue++;
	this->queueOwner=thePlayer;
}




bool Burst( Node* this )
{
	bool retval=false;
	int i=0;

	for( i=0; i<this->neighborcount && this->piececount>0; i++ )
	{
		if( this->Neighbors[i]!=NULL )
		{
			retval=true;
			AddPieceToQueue( (Node*)(this->Neighbors[i]), this->Owner );
			this->piececount--;
		}
		else
		{
			break;
		}
	}

	return retval;
}


bool TransferQueue( Node* this )
{
	if( this->queue )
	{
		this->piececount+=this->queue;
		this->Owner=this->queueOwner;
		this->queue=0;
		this->queueOwner=0;
		return true;
	}
	else
	{
		return false;
	}
}


bool AddNeighbor( Node* theNode, Node* NewNeighbor )
{
	if( theNode->neighborcount<kMaxNeighbors )
	{
		theNode->Neighbors[theNode->neighborcount] = (struct Node*)NewNeighbor;
		theNode->neighborcount++;
		return true;
	}
	else
	{
		return false;
	}
}



void ConnectBoardWithDiag( Node* theNodes, int Rows, int Columns )
{
	int x,y;
	int litx, lity;

	for( x=0; x<Columns; x++ )
		for( y=0; y<Rows; y++ )
		{
			for( litx=-1; litx<=1; litx++)
				for( lity=-1; lity<=1; lity++)
				{
					if( (litx!=0 || lity!=0) && (litx+x)<Columns && (litx+x)>0 && (lity+y)<Rows && (lity+y)>0 )
					{
						AddNeighbor( (Node*)(theNodes+(Columns*y+x)), (Node*)(theNodes+(Columns*(y+lity)+(x+litx))) );
					}
				}
		}
}


void ConnectBoardCardinal( Node* theNodes, int Rows, int Columns )
{
	int x,y;
	int litx, lity;

	for( x=0; x<Columns; x++ )
		for( y=0; y<Rows; y++ )
		{
			for( lity=-1; lity<=1; lity++)
				for( litx=-1; litx<=1; litx++)
				{
					if( (litx!=0 || lity!=0) &&
					    (litx==0 || lity==0) &&
					    (litx+x)<Columns &&
					    (litx+x)>=0 &&
					    (lity+y)<Rows &&
					    (lity+y)>=0 )
					{
						AddNeighbor( theNodes+(Columns*y+x), theNodes+(Columns*(y+lity)+(x+litx)) );
					}
				}
		}

}



void InitBoard( Node* theBoard, int Size )
{
	int i;
	for( i=0; i<Size; i++)
	{
		InitNode( theBoard+i );
	}
}



void PrintBoard( Node* theBoard, int Rows, int Columns )
{
	int x,y;
	char c;

	for( y=0; y<Rows; y++ )
	{
		printf( "\n\n" );
		for( x=0; x<Columns; x++ )
		{
			c = GetPlayerCharacter( theBoard[y*Columns + x].Owner );
			printf( "%d",theBoard[y*Columns + x].piececount );
			putchar( c ); printf( " " );
		}
	}
	printf( "\n" );
}


void SprintBoard( char* theBuffer, Node* theBoard, int Rows, int Columns )
{
	int x,y;
	int sl;
	char c;

	for( y=0; y<Rows; y++ )
	{
		for( x=0; x<Columns; x++ )
		{
			c = GetPlayerCharacter( theBoard[y*Columns + x].Owner );
			sprintf( theBuffer+strlen(theBuffer), "%d",theBoard[y*Columns + x].piececount );

			sl = strlen(theBuffer);
			theBuffer[sl+1]=0;
			theBuffer[strlen(theBuffer)]=c;

			sprintf( theBuffer+strlen(theBuffer), " " );
		}
		sprintf( theBuffer+strlen(theBuffer), "\n" );
	}
	sprintf( theBuffer+strlen(theBuffer), "\n" );
}



char GetPlayerCharacter( Player i )
{
	char CharArray[] = {' ','x','o'};
	if( i<sizeof( CharArray ) && i>=0 )
		return CharArray[i];
	else
		return ' ';
}



void UpdateBoard( Node* theBoard, int nodecount )
{
	int i;
	for( i=0; i<nodecount; i++ )
	{
		TransferQueue( &(theBoard[i]) );
	}
}





bool BurstBoardIfNeeded( Node* theBoard, int nodecount )
{
	bool retval=false;
	int i;

	if( !Winner(theBoard,nodecount) )
	{
		for( i=0; i<nodecount; i++ )
		{
			if( theBoard[i].piececount>theBoard[i].neighborcount )
			{
				Burst( &(theBoard[i]) );
				retval=true;
			}
		}
	}

	UpdateBoard( theBoard,nodecount );

	return retval;
}




/*simply checks whether anyone occupies the whole board*/
Player Winner( Node* theBoard, int nodecount )
{
	Player retval=0;
	int i;

	retval = theBoard[0].Owner;
	for(i=0; i<nodecount; i++)
	{
		if( theBoard[i].Owner != theBoard[0].Owner )
		{
			retval=0;
		}
	}

	return retval;
}




void APPEND(char* C, char c)
{
	C[strlen(C)+1]=0; C[strlen(C)]=(c);
}


void stringsandwich( char* A, char* B, char* C )
{
	int a=0, b=0;

	while( A[a] != 0 && B[b]!=0 )
	{
		while( A[a] != '\n' && A[a] != 0 )
		{
			APPEND(C, A[a]);
			a++;
		}

		while( B[b] != '\n' && B[b] != 0 )
		{
			APPEND(C, B[b]);
			b++;
		}

		if( B[b]=='\n' || A[a]=='\n' )
		{
			APPEND(C, '\n');
		}

		b++;
		a++;
	}
}



long GetTotalCombinations( Node* theBoard, int NodeCount )
{
	long counter;
	long i;

	counter = 1;
	for( i=0; i<NodeCount; i++ )
	{
		counter*=( theBoard[i].neighborcount )*2+1;
	}

	return counter;
}






POSITION EncodeBoard( Node* theBoard, long NodeCount )
{
	long i = 0;
	POSITION counter = 0;
	unsigned long runningproduct = 1;

	Player thewinner = Winner( theBoard,NodeCount );

	if( !thewinner )
	{
		for( i = 0; i<NodeCount; i++ )
		{
			long modulus = (theBoard[i].neighborcount)*2+1;
			long residue = 0;

			residue = theBoard[i].piececount;
			if( theBoard[i].Owner == 2 ) residue+=theBoard[i].neighborcount;

			counter+=runningproduct*( residue );
			runningproduct *= modulus;
		}
	}
	else
	{
		for( i = 0; i<NodeCount; i++ )
		{
			long modulus = (theBoard[i].neighborcount)*2+1;
			long residue = 0;

			residue = theBoard[i].neighborcount;
			if( thewinner == 2 ) residue+=theBoard[i].neighborcount;

			counter+=runningproduct*( residue );
			runningproduct *= modulus;
		}
	}

	return counter;
}






// The board passed in must be empty but not unitialized you see.

Player DecodeBoard( Node* theBoard, long NodeCount, POSITION inPos )
{
	long i = 0;
	unsigned long runningquotient;

	runningquotient = inPos;

	for( i = 0; i<NodeCount; i++ )
	{
		long modulus = (theBoard[i].neighborcount)*2+1;
		long residue = runningquotient % modulus;

		if( residue == 0 )
		{
			theBoard[i].Owner = 0;
			theBoard[i].piececount = 0;
		}
		else if( residue > theBoard[i].neighborcount )
		{
			theBoard[i].Owner = 2;
			theBoard[i].piececount = residue-theBoard[i].neighborcount;
		}
		else
		{
			theBoard[i].Owner = 1;
			theBoard[i].piececount = residue;
		}
		runningquotient/=modulus;
	}

	return 0; //for now
}


STRING kDBName = "cmass";

int NumberOfOptions()
{
	return 2*(MAX_WIDTH-MIN_WIDTH+1)*(MAX_HEIGHT-MIN_HEIGHT+1);
}

int getOption()
{
	int option = 1;

	if ( gStandardGame ) option += 1;
	option += gBoardWidth *2;
	option += gBoardHeight *2*(MAX_WIDTH-MIN_WIDTH+1);

	return option;
}

void setOption(int option)
{
	option -= 1;
	gStandardGame = option%2;
	gBoardWidth = option/2%(MAX_WIDTH-MIN_WIDTH+1);
	gBoardHeight = option/(2*(MAX_WIDTH-MIN_WIDTH+1))%(MAX_HEIGHT-MIN_HEIGHT+1);
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
