#include<stdio.h>
#include "gamesman.h"

extern STRING gValueString[];

int      gNumberOfPositions ;
POSITION gInitialPosition ;
POSITION gMinimalPosition ;

BOOLEAN  kPartizan           = FALSE ;
BOOLEAN  kDebugMenu          = FALSE;
BOOLEAN  kGameSpecificMenu   = FALSE;
BOOLEAN  kTieIsPossible      = FALSE;
BOOLEAN  kLoopy               = FALSE;
BOOLEAN  kDebugDetermineValue = FALSE;
STRING   kGameName           = "";
void*	 gGameSpecificTclInit = NULL;

STRING   kHelpGraphicInterface = "" ;
STRING   kHelpTextInterface    = "" ;
STRING   kHelpOnYourTurn = "" ;
STRING   kHelpStandardObjective = "" ;
STRING   kHelpReverseObjective = "" ;
STRING   kHelpTieOccursWhen = "" ;
STRING   kHelpExample = "" ;

InitializeGame()
{
	// HERE, YOU SHOULD ASSIGN gNumberOfPositions and gInitialPosition and gMinimalPosition
}

FreeGame()
{
}

DebugMenu()
{
}

GameSpecificMenu()
{
}

SetTclCGameSpecificOptions(int theOptions [])
{
}

POSITION DoMove(POSITION thePosition, MOVE theMove)
{
}

GetInitialPosition()
{
}

PrintComputersMove(MOVE computersMove, STRING computersName)
{
}

VALUE Primitive(POSITION position)
{
}

PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn)
{
}

MOVELIST *GenerateMoves(POSITION position)
{
}

USERINPUT GetAndPrintPlayersMove(POSITION thePosition, MOVE * theMove, STRING playerName)
{
}

BOOLEAN ValidTextInput(STRING input)
{
}

MOVE ConvertTextInputToMove(STRING input)
{
}

PrintMove(MOVE theMove)
{
}

// example: for tic-tac-toe : mttt-> "ttt"
STRING kDBName = "<ENTER MODULE NAME HERE>" ;

int NumberOfOptions()
{    
	return 2 ;
} 

int getOption()
{
	if(gStandardGame) return 1 ;
	return 2 ;
} 

void setOption(int option)
{
	if(option == 1) gStandardGame = TRUE ;
	else gStandardGame = FALSE ;
}

