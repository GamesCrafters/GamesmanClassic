// Pylos
// Eric Siroker
// February 11, 2004

#include "gamesman.h"
#include "hash.h"

int      gBoardHeight           = 3;
int      gBoardWidth            = 3;
POSITION gInitialPosition       = 0;
POSITION gMinimalPosition       = 0;
POSITION gNumberOfPositions     = 0;
POSITION kBadPosition           = -1;
STRING   kDBName                = "pylos";
BOOLEAN  kDebugDetermineValue   = FALSE;
BOOLEAN  kDebugMenu             = FALSE;
STRING   kGameName              = "Pylos";
BOOLEAN  kGameSpecificMenu      = TRUE;
STRING   kHelpExample           = "Not written yet.";
STRING   kHelpGraphicInterface  = "Not written yet.";
STRING   kHelpOnYourTurn        = "Not written yet.";
STRING   kHelpReverseObjective  = "Not written yet.";
STRING   kHelpStandardObjective = "Not written yet.";
STRING   kHelpTextInterface     = "Not written yet.";
STRING   kHelpTieOccursWhen     = "Not written yet.";
BOOLEAN  kLoopy                 = TRUE;
BOOLEAN  kPartizan              = TRUE;
BOOLEAN  kTieIsPossible         = FALSE;

MOVE ConvertTextInputToMove(STRING input) {
  return 0;
}

void DebugMenu() {}

POSITION DoMove(POSITION position, MOVE move) {
  return 0;
}

void GameSpecificMenu() {}

MOVELIST *GenerateMoves(POSITION position) {
  return 0;
}

USERINPUT GetAndPrintPlayersMove(POSITION position, MOVE *move, STRING name) {
  return 0;
}

POSITION GetInitialPosition() {
  return 0;
}

int getOption() {
  return 0;
}

void InitializeGame() {
  int boardSize = gBoardHeight * gBoardWidth;
  int pieces[] = {'-', 0, boardSize,
                  'O', 0, boardSize / 2,
                  'X', 0, boardSize / 2, -1};

  gNumberOfPositions = generic_hash_init(boardSize, pieces, NULL);
}

int NumberOfOptions() {
  return 0;
}

VALUE Primitive(POSITION position) {
  return 0;
}

void PrintComputersMove(MOVE move, STRING name) {}

void PrintMove(MOVE move) {}

void PrintPosition(POSITION position, STRING name, BOOLEAN isUsersTurn) {}

void setOption(int option) {}

BOOLEAN ValidTextInput(STRING input) {
  return 0;
}


/*
** Unfortunately, this is needed for now. -JJ
*/

int GameSpecificTclInit (interp, mainWindow) 
	Tcl_Interp* interp;
	Tk_Window mainWindow;
{
}
