// Tac Tix stroke parser
// Note: Dots and Boxes and Tac Tix use similar board parsers - should probably refactor the common code

#include "loader.h"
#include "stroke.h"
#include "tcl.h"
#include <stdio.h>
#include <stdlib.h>


#define TCL_TIMER_MS 100
Tcl_Interp *tclInterpreter = NULL;

#define MIN_BOARD_DISTANCE 10
#define BOARD_EDGE 4
#define BOARD_SIZE 16

typedef struct {
	Stroke stroke;
	int x, y;
} strokeWithIndex;

BOOL gameFinished = FALSE;
int numBoardStrokes = 0;
// both of these are indexed by order they are drawn, for game bit position use (y*BOARD_EDGE + x)
strokeWithIndex boardStrokes[BOARD_SIZE];
BOOL crossedStrokes[BOARD_SIZE];


void resetGameState()
{
	gameFinished = FALSE;
	numBoardStrokes = 0;
	int i;
	for (i = 0; i < BOARD_SIZE; i++) {
		crossedStrokes[i] = FALSE;
	}
	printLog("Reset game state for Tac Tix\n");
}

BOOL isGameOver() {
	int i;
	for (i = 0; i < BOARD_SIZE; i++) {
		if (crossedStrokes[i] == FALSE) {
			return FALSE;
		}
	}
	return TRUE;
}

int closestStroke(double cx, double cy) {
	if (numBoardStrokes == 0) {
		return 0;
	}
	int pointIdx = 0;
	double dist, minDist = distance(cx, cy, boardStrokes[0].stroke.bounds.centerX, boardStrokes[0].stroke.bounds.centerY);
	int i;
	for (i = 1; i < numBoardStrokes; i++) {
		dist = distance(cx, cy, boardStrokes[i].stroke.bounds.centerX, boardStrokes[i].stroke.bounds.centerY);
		if (dist < minDist) {
			minDist = dist;
			pointIdx = i;
		}
	}
	return pointIdx;
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


	// if already have the board, determine and make the move
	if (numBoardStrokes == BOARD_SIZE) {
		if (stroke.type != STROKE_LINE) {
			printLog("   Move stroke is not a line, skipping\n");
			return;
		}

		int i, j, crossed[4], numCrossed = 0;
		for (i = 0; i < numBoardStrokes; i++) {
			if (!doesLineIntersectBoundingCircle(&stroke.line, &boardStrokes[i].stroke)) {
				continue;
			}
			if (crossedStrokes[i]) {
				printLog("   Ignoring crossed stroke that was already used\n");
				continue;
			}
			if (numCrossed == 4) {
				printLog("   Crossed more than 4 board strokes, skipping\n");
				return;
			}
			crossed[numCrossed] = i;
			numCrossed++;
		}
		if (numCrossed == 0) {
			printLog("   No board strokes crossed, skipping\n");
			return;
		}
		for (i = 0; i < numCrossed; i++) {
			BOOL haveNeighbor = FALSE;
			for (j = 0; j < numCrossed; j++) {
				if (i == j) continue;
				int dx = boardStrokes[crossed[i]].x - boardStrokes[crossed[j]].x;
				int dy = boardStrokes[crossed[i]].y - boardStrokes[crossed[j]].y;
				if (abs(dx) + abs(dy) == 1) {
					haveNeighbor = TRUE;
					break;
				}
			}
			if ((numCrossed > 1) && (!haveNeighbor)) {
				printLog("   Crossed strokes are not continuous, skipping\n");
				return;
			}
		}
		int bit, move = 0;
		for (i = 0; i < numCrossed; i++) {
			j = crossed[i];
			crossedStrokes[j] = TRUE;
			bit = (boardStrokes[j].y * BOARD_EDGE) + boardStrokes[j].x;
			move |= (1 << bit);
			printLog("   Crossed board stroke at %d, %d (bit %d)\n", boardStrokes[j].x, boardStrokes[j].y, bit);
		}
		printLog("   Making move %#x (%d)\n", move, move);
		char moveCommand[30];
		sprintf(moveCommand, "ReturnFromHumanMove %d", move);
		Tcl_Eval(tclInterpreter, moveCommand);
		if (isGameOver()) {
			gameFinished = TRUE;
			printLog("   Game finished\n");
		}
		return;
	}


	// otherwise, add stroke to the temporary board with possibly non-0 based indices
	double px = stroke.bounds.centerX, py = stroke.bounds.centerY;
	boardStrokes[numBoardStrokes].stroke = stroke;
	if (numBoardStrokes == 0) {
		boardStrokes[0].x = boardStrokes[0].y = 0;
	} else {
		strokeWithIndex neighbor = boardStrokes[closestStroke(px, py)];
		Line l = newLine(neighbor.stroke.bounds.centerX, neighbor.stroke.bounds.centerY, px, py);
		if (l.length < MIN_BOARD_DISTANCE) {
			printLog("   Stroke is too close to neighbor, ignoring\n");
			return;
		}
		double angle = l.angleDegrees;
		if (angle >= 45 && angle < 135) {
			printLog("    Stroke is Down ");
			boardStrokes[numBoardStrokes].x = neighbor.x;
			boardStrokes[numBoardStrokes].y = neighbor.y + 1;
		} else if (angle >= -45 && angle < 45) {
			printLog("    Stroke is Right ");
			boardStrokes[numBoardStrokes].x = neighbor.x + 1;
			boardStrokes[numBoardStrokes].y = neighbor.y;
		} else if (angle >= -135 && angle < -45) {
			printLog("    Stroke is Up ");
			boardStrokes[numBoardStrokes].x = neighbor.x;
			boardStrokes[numBoardStrokes].y = neighbor.y - 1;
		} else {
			printLog("    Stroke is Left ");
			boardStrokes[numBoardStrokes].x = neighbor.x - 1;
			boardStrokes[numBoardStrokes].y = neighbor.y;
		}
		printLog("from %d, %d\n", neighbor.x, neighbor.y);
	}
	numBoardStrokes++;
	if (numBoardStrokes < BOARD_SIZE) {
		return;
	}

	// determine proper board indices
	int i, minX, maxX, minY, maxY;
	minX = maxX = boardStrokes[0].x;
	minY = maxY = boardStrokes[0].y;
	for (i = 1; i < numBoardStrokes; i++) {
		minX = (boardStrokes[i].x < minX) ? boardStrokes[i].x : minX;
		maxX = (boardStrokes[i].x > maxX) ? boardStrokes[i].x : maxX;
		minY = (boardStrokes[i].y < minY) ? boardStrokes[i].y : minY;
		maxY = (boardStrokes[i].y > maxY) ? boardStrokes[i].y : maxY;
	}
	if ((maxX - minX >= BOARD_EDGE) || (maxY - minY >= BOARD_EDGE)) {
		printLog("   Board points indices don't fit the correct board size! Starting over completely.\n");
		resetGameState();
		return;
	}
	for (i = 0; i < numBoardStrokes; i++) {
		boardStrokes[i].x -= minX;
		boardStrokes[i].y -= minY;
	}
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
