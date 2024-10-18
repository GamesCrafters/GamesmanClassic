// Template stroke parser

#include "loader.h"
#include "stroke.h"
#include "tcl.h"
#include <stdio.h>


#define TCL_TIMER_MS 100
Tcl_Interp *tclInterpreter = NULL;

BOOL gameFinished = FALSE;


void resetGameState()
{
	gameFinished = FALSE;
}


// called by Tcl Event loop to check for new data
void TclTimerProc(ClientData clientData)
{
	if (!isPenLoaderStarted()) {
		printLog("Timer stopped\n");
		return;
	}

	Stroke stroke = getNextAvailableStroke();
	Tcl_CreateTimerHandler(TCL_TIMER_MS, &TclTimerProc, NULL);
	if (stroke.nSamples == 0) {
		return;
	}

	if (gameFinished) {
		printLog("   This game is already finished, skipping stroke\n");
		return;
	}

	// sample game (one move, no board detection)
	printLog("   Making move 1\n");
	Tcl_Eval(tclInterpreter, "ReturnFromHumanMove 1");
	gameFinished = TRUE;
}


// called by Gamesman when new game is started
void gPenHandleTclMessage(int options[], char *filename, Tcl_Interp *tclInterp, int debug)
{
	if (hasPenLoaderFinished()) {
		// already read through entire file, no need to do it again
		return;
	}
	if (isPenLoaderStarted()) {
		// loader already started - reset state, change output to new file
		resetGameState();
		startNewPenOutputFile();
	} else if (startPenLoader(filename, debug) == 0) {
		// started loader for the first time - set initial state, start timer
		resetGameState();
		tclInterpreter = tclInterp;
		Tcl_CreateTimerHandler(TCL_TIMER_MS, &TclTimerProc, NULL);
	}
}
