// Dots and Boxes stroke parser
// Note: Dots and Boxes and Tac Tix use similar board parsers - should probably refactor the common code

#include "loader.h"
#include "stroke.h"
#include "tcl.h"
#include <stdio.h>
#include <stdlib.h>


#define TCL_TIMER_MS 100
Tcl_Interp *tclInterpreter = NULL;

#define MIN_BOARD_DISTANCE 10
// won't use all space for dots/moves if width or height < 3
#define MAX_DOTS 4
#define MAX_MOVES 24

typedef struct {
	Sample position;
	int x, y;
} pointWithIndex;

BOOL gameFinished = FALSE;
int width, height;
int numPoints = 0, numMoves = 0;
pointWithIndex boardPoints[MAX_DOTS*MAX_DOTS];
BOOL movesDone[MAX_MOVES];


// should be given the starting point and direction: TRUE for right / FALSE for down
int pointToMove(int x, int y, BOOL isRight) {
	if ((x < 0) || (y < 0) ||
	    (isRight && ((x >= width) || (y > height))) ||
	    (!isRight && ((x > width) || (y >= height)))) {
		return -1;
	}
	if (isRight) {
		return (y * width) + x;
	}
	return (width * (height + 1)) + (x * height) + y;
}


void resetGameState()
{
	gameFinished = FALSE;
	numPoints = 0;
	numMoves = 0;
	int i;
	for (i = 0; i < MAX_MOVES; i++) {
		movesDone[i] = FALSE;
	}
	printLog("Reset game state for Dots and Boxes\n");
}


int closestPoint(double cx, double cy) {
	if (numPoints == 0) {
		return 0;
	}
	int pointIdx = 0;
	double dist, minDist = distance(cx, cy, boardPoints[0].position.x, boardPoints[0].position.y);
	int i;
	for (i = 1; i < numPoints; i++) {
		dist = distance(cx, cy, boardPoints[i].position.x, boardPoints[i].position.y);
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
	if (numPoints == ((width+1)*(height+1))) {
		if (stroke.type != STROKE_LINE) {
			printLog("   Move stroke is not a line, skipping\n");
			return;
		}

		pointWithIndex p1 = boardPoints[closestPoint(stroke.line.x1, stroke.line.y1)];
		pointWithIndex p2 = boardPoints[closestPoint(stroke.line.x2, stroke.line.y2)];
		int dx = p2.x - p1.x;
		int dy = p2.y - p1.y;
		if ((abs(dx) + abs(dy)) != 1) {
			printLog("   Line's starting and ending points are not board neighbors, skipping stroke\n");
			return;
		}

		int move;
		if (dx == -1) {
			move = pointToMove(p2.x, p2.y, TRUE);
		} else if (dx == 1) {
			move = pointToMove(p1.x, p1.y, TRUE);
		} else {
			if (dy == -1) {
				move = pointToMove(p2.x, p2.y, FALSE);
			} else {
				move = pointToMove(p1.x, p1.y, FALSE);
			}
		}
		if (move == -1) {
			printLog("   pointToMove() returned an invalid move, skipping stroke\n");
			return;
		}

		if (movesDone[move]) {
			printLog("   Skipping duplicate move: %d\n", move);
			return;
		}
		movesDone[move] = TRUE;
		numMoves++;
		printLog("   Making move %d\n", move);
		char moveCommand[30];
		sprintf(moveCommand, "ReturnFromHumanMove %d", move);
		Tcl_Eval(tclInterpreter, moveCommand);
		if (numMoves == (2*width*height + width + height)) {
			gameFinished = TRUE;
			printLog("   Game finished\n");
		}
		return;
	}


	// otherwise, add stroke to the temporary board with possibly non-0 based indices
	double px = stroke.bounds.centerX, py = stroke.bounds.centerY;
	boardPoints[numPoints].position = (Sample){px, py};
	if (numPoints == 0) {
		boardPoints[0].x = boardPoints[0].y = 0;
	} else {
		pointWithIndex neighbor = boardPoints[closestPoint(px, py)];
		Line l = newLine(neighbor.position.x, neighbor.position.y, px, py);
		if (l.length < MIN_BOARD_DISTANCE) {
			printLog("    Stroke is too close to neighbor, ignoring\n");
			return;
		}
		double angle = l.angleDegrees;
		if (angle >= 45 && angle < 135) {
			printLog("    Point is Down ");
			boardPoints[numPoints].x = neighbor.x;
			boardPoints[numPoints].y = neighbor.y + 1;
		} else if (angle >= -45 && angle < 45) {
			printLog("    Point is Right ");
			boardPoints[numPoints].x = neighbor.x + 1;
			boardPoints[numPoints].y = neighbor.y;
		} else if (angle >= -135 && angle < -45) {
			printLog("    Point is Up ");
			boardPoints[numPoints].x = neighbor.x;
			boardPoints[numPoints].y = neighbor.y - 1;
		} else {
			printLog("    Point is Left ");
			boardPoints[numPoints].x = neighbor.x - 1;
			boardPoints[numPoints].y = neighbor.y;
		}
		printLog("from %d, %d\n", neighbor.x, neighbor.y);
	}
	numPoints++;
	if (numPoints < ((width+1)*(height+1))) {
		return;
	}

	// determine proper board indices
	int i, minX, maxX, minY, maxY;
	minX = maxX = boardPoints[0].x;
	minY = maxY = boardPoints[0].y;
	for (i = 1; i < numPoints; i++) {
		minX = (boardPoints[i].x < minX) ? boardPoints[i].x : minX;
		maxX = (boardPoints[i].x > maxX) ? boardPoints[i].x : maxX;
		minY = (boardPoints[i].y < minY) ? boardPoints[i].y : minY;
		maxY = (boardPoints[i].y > maxY) ? boardPoints[i].y : maxY;
	}
	if ((maxX - minX > width) || (maxY - minY > height)) {
		printLog("   Board points indices don't fit the given width/height! Starting over completely.\n");
		resetGameState();
		return;
	}
	for (i = 0; i < numPoints; i++) {
		boardPoints[i].x -= minX;
		boardPoints[i].y -= minY;
	}
}


// called by Gamesman when new game is started
void gPenHandleTclMessage(int options[], char *filename, Tcl_Interp *tclInterp, int debug)
{
	if (hasPenLoaderFinished()) {
		// already read through entire file, no need to do it again
		return;
	}
	width = options[0];
	height = options[1];
	if (!(width > 0 && width < MAX_DOTS) ||
	    !(height > 0 && height < MAX_DOTS)) {
		printLog("   Invalid board dimensions: %d, %d - will not be starting loader\n", width, height);
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
