#include <stdio.h>
#include <ctype.h>
#include "gamesman.h"

#define MIN_ROWS 1
#define MAX_ROWS 9

int rows = 4;

POSITION gNumberOfPositions = 0;
POSITION gInitialPosition = 0;
POSITION gMinimalPosition = 0;
//POSITION gMinimalPosition = (1<<(ROWS*3)) - 1 ;

BOOLEAN kPartizan              = FALSE;
BOOLEAN kDebugMenu             = FALSE;
BOOLEAN kGameSpecificMenu      = TRUE;
BOOLEAN kTieIsPossible         = FALSE;
BOOLEAN kLoopy                 = FALSE;
BOOLEAN kDebugDetermineValue   = FALSE;
STRING kGameName              = "2-D Nim";
POSITION kBadPosition           = -1;
void*    gGameSpecificTclInit   = NULL;

STRING kHelpGraphicInterface  = "Help Graphic Interface: Fill in later";
STRING kHelpTextInterface     = "Help Text Interface: Fill in later";
STRING kHelpOnYourTurn        = "Help on your turn";
STRING kHelpStandardObjective = "help standard objective";
STRING kHelpReverseObjective  = "reverse objective";
STRING kHelpTieOccursWhen     = "A tie is not possible in this game";
STRING kHelpExample           = "some really long thing<-Actually play a game,then copy/paste";
STRING kAuthorName            = "Gamescrafters";

STRING CUSTOM_TRAITS[3] = {"XORofColumns", "MisereXORofColumns", ""};

STRING MoveToString(MOVE);

float XORofColumns(POSITION p){
	int i, colxor=0;
	p >>= 1;
	for(i = 0; i < rows; i++) {
		colxor ^= p & 7;
		p >>= 3;
	}
	return (float)(colxor!=0 ? 1 : -1);
}

float MisereXORofColumns(POSITION p){
	BOOLEAN oneColumnHasOne=FALSE, lastMove=FALSE;
	int numColumns=0, i;
	p >>= 1;
	for(i = 0; i < rows; i++) {
		numColumns += (p & 7) ? 0 : 1;
		if(!oneColumnHasOne) oneColumnHasOne = ((p & 7)==1);
		p >>= 3;
	}

	lastMove = ( oneColumnHasOne && numColumns==2 );

	if( lastMove )
		return !XORofColumns(p);

	return XORofColumns(p);
}

featureEvaluatorCustom getSEvalCustomFnPtr(STRING fnName){
	return (strcmp(fnName,"MisereXORofColumns")==0) ? &MisereXORofColumns : &XORofColumns;
}

void InitializeGame()
{
	// HERE, YOU SHOULD ASSIGN gNumberOfPositions and gInitialPosition
	gNumberOfPositions =(1<<(rows*3)) * 2;
	gInitialPosition = 7858; // 1, 3, 5, 7-piece rows

	// gInitialPosition = ((1<<(rows*3)) - 1) * 2;
	gMinimalPosition = gInitialPosition;

	gMoveToStringFunPtr = &MoveToString;
	gGetSEvalCustomFnPtr = &getSEvalCustomFnPtr;
	gCustomTraits = CUSTOM_TRAITS;
}

// SUNIL: NOT WRITING
void FreeGame()
{
}

// SUNIL: NOT WRITING
void DebugMenu()
{
}

// SUNIL: NOT WRITING
void GameSpecificMenu()
{
	char option;
	int newrows;
	do
	{
		printf("Currently you have %d rows. Rows can range between %d and %d\n", rows, MIN_ROWS, MAX_ROWS);
		printf("Would you like to change it [y/n]? ");
		option = GetMyChar();
		option = toupper(option);
		if(option == 'Y')
		{
			do
			{
				printf("Enter new number of rows [%d-%d] : ", MIN_ROWS, MAX_ROWS);
				newrows = GetMyInt();
			} while(newrows > MAX_ROWS || newrows < MIN_ROWS);
			rows = newrows;
		}
		else if(option != 'N')
		{
			printf("Invalid option. Please try again.\n");
		}
	} while(option != 'N');
}

// SUNIL: NOT WRITING
void SetTclCGameSpecificOptions(int theOptions [])
{
}

#define max(a,b) (((a)>(b)) ? (a) : (b))

// SUNIL: DONE
POSITION DoMove(POSITION thePosition, MOVE theMove)
{
	int * array;
	int i;
	int row;        // the Row the move refers to (between 1 and ROWS)
	int num;        // the number to remove from row specified
	int turnbit = thePosition & 1;

	thePosition >>= 1;

	array = SafeMalloc(sizeof(int)*rows);

	for(i = 0; i < rows; i++)
	{
		array[i] = thePosition & 7;
		thePosition = thePosition >> 3;
	}
	row = (theMove / 10);
	num = (theMove % 10);

	array[row-1] = array[row-1]-num;

	thePosition = 0;

	for(i = rows - 1; i >= 0; i--)
	{
		thePosition = thePosition << 3;
		thePosition += array[i];
	}

	free(array);

	return thePosition*2+(1-turnbit);
}

POSITION GetInitialPosition()
{
	return gInitialPosition;
}

// SUNIL: DONE
void PrintComputersMove(MOVE computersMove, STRING computersName)
{
	printf("%s's move: %d\n", computersName, computersMove);
}

// SUNIL: DONE
VALUE Primitive(POSITION position)
{
	if (position == 0 || position == 1)
		return (gStandardGame ? lose : win);

	return undecided;
}

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn)
{
	int i, j;
	int positionBak = position;
	position /=2;
	printf("\n");
	for(i = 0; i < rows; i++)
	{
		printf("%d: ", i+1);
		for(j = 0; j < position % 8; j++)
			printf("O ");
		printf("\n");
		position = position >> 3;
	}
	printf("\n%s\n", GetPrediction(positionBak, playerName, usersTurn));
	printf("%s\n\n", GetSEvalPrediction(positionBak, playerName, usersTurn));
}

MOVELIST *GenerateMoves(POSITION position)
{
	// Here, use head = CreateMovelistNode(move,head) ;
	// then return head when done
	MOVELIST *CreateMovelistNode(), *head = NULL;
	int i, j;

	if(Primitive(position) == undecided)
	{
		position /= 2;
		for(i = 0; i < rows; i++)
		{
			int piecesinrow = position % 8;
			position = position >> 3;
			for(j = 1; j <= piecesinrow; j++)
				head = CreateMovelistNode( (MOVE)((i+1)*10 + j), head);
		}
	}

	return head;
}

USERINPUT GetAndPrintPlayersMove(POSITION thePosition, MOVE * theMove, STRING playerName)
{
	USERINPUT ret;
	do
	{
		printf("%8s's move [[1-%d][1-7]] :  ", playerName, rows);
		ret = HandleDefaultTextInput(thePosition, theMove, playerName);
		if(ret != Continue)
			return(ret);
	} while(TRUE);
	return(Continue);
}

BOOLEAN ValidTextInput(STRING input)
{
	int i;
	for(i = 0; i < strlen(input); i++)
		if(!isdigit(input[i]))
			return FALSE;
	return TRUE;
}

MOVE ConvertTextInputToMove(STRING input)
{
	int ret;
	sscanf(input, "%d", &ret);
	return ret;
}

void PrintMove(MOVE theMove)
{
	STRING m = MoveToString( theMove );
	printf( "%s", m );
	SafeFree( m );
}

STRING MoveToString (theMove)
MOVE theMove;
{
	STRING move = (STRING) SafeMalloc(4);
	sprintf(move, "%d", theMove);
	return move;
}

STRING kDBName = "nim";

int NumberOfOptions()
{
	return 2*(MAX_ROWS-MIN_ROWS+1);
}

int getOption()
{
	int option = 1;
	if (gStandardGame) option += 1;
	option += (rows-MIN_ROWS) * 2;
	return option;
}

void setOption(int option)
{
	option--;
	gStandardGame = option%2==1;
	rows = option/2%(MAX_ROWS-MIN_ROWS+1)+MIN_ROWS;
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
