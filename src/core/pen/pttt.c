// Tic Tac Toe stroke parser

#include "loader.h"
#include "stroke.h"
#include "tcl.h"
#include <stdio.h>
#include <math.h>


#define TCL_TIMER_MS 100
Tcl_Interp *tclInterpreter = NULL;


// bit masks for determining which board lines were crossed
#define CROSS_NONE      0
#define CROSS_LEFT      1
#define CROSS_RIGHT     2
#define CROSS_TOP       4
#define CROSS_BOTTOM    8

// numeric values:     0  1  2  3   4  5  6  7   8  9  10 11  12  13  14  15
// crossed edges:      N  L  R  RL  T  TL TR TRL B  BL BR BRL BT  BTL BTR BTRL
int CrossToMove[16] = {4, 3, 5, -1, 1, 0, 2, -1, 7, 6, 8, -1, -1, -1, -1, -1};


BOOL gameFinished = FALSE;
int numBoardLines = 0;
Line boardLines[4], *boardLeft, *boardRight, *boardTop, *boardBottom;
double boardCenterX, boardCenterY;
int movesDone[9];
int currentPlayer;


void resetGameState()
{
	int idx;
	for (idx = 0; idx < 9; idx++) {
		movesDone[idx] = 0;
	}
	gameFinished = FALSE;
	numBoardLines = 0;
	currentPlayer = 1;
	printLog("Reset game state for Tic Tac Toe\n");
}


BOOL checkThreeMoves(int a, int b, int c)
{
	return ((movesDone[a] != 0) && (movesDone[a] == movesDone[b]) && (movesDone[a] == movesDone[c]));
}

BOOL checkForFinishCondition()
{
	int i;
	for (i = 0; i < 9; i++) {
		if (movesDone[i] == 0) {
			break;
		}
	}
	if (i == 9) {
		return TRUE; // board is full
	}
	return (checkThreeMoves(0, 1, 2) ||
	        checkThreeMoves(3, 4, 5) ||
	        checkThreeMoves(6, 7, 8) ||
	        checkThreeMoves(0, 3, 6) ||
	        checkThreeMoves(1, 4, 7) ||
	        checkThreeMoves(2, 5, 8) ||
	        checkThreeMoves(0, 4, 8) ||
	        checkThreeMoves(6, 4, 2));
}


// called by Tcl Event loop to check for new data
void TclTimerProc(ClientData clientData)
{
	if (!isPenLoaderStarted()) {
		printLog("Timer stopped\n");
		return;
	}

	// get the next stroke to analyze
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
	if (numBoardLines == 4) {
		Line testLine = newLine(boardCenterX, boardCenterY, stroke.bounds.centerX, stroke.bounds.centerY);
		int crossing = (doLinesIntersect(&testLine, boardLeft)) ? CROSS_LEFT : CROSS_NONE;
		crossing |= (doLinesIntersect(&testLine, boardRight)) ? CROSS_RIGHT : CROSS_NONE;
		crossing |= (doLinesIntersect(&testLine, boardTop)) ? CROSS_TOP : CROSS_NONE;
		crossing |= (doLinesIntersect(&testLine, boardBottom)) ? CROSS_BOTTOM : CROSS_NONE;

		int move = CrossToMove[crossing];
		if (move == -1) {
			printLog("   Invalid crossing value detected: %d, skipping\n", crossing);
			return;
		} else if (movesDone[move] != 0) {
			printLog("   Skipping duplicate move: %d\n", move);
			return;
		}

		movesDone[move] = currentPlayer;
		currentPlayer = (currentPlayer % 2) + 1;
		printLog("   Making move %d\n", move);
		char moveCommand[30];
		sprintf(moveCommand, "ReturnFromHumanMove %d", move);
		Tcl_Eval(tclInterpreter, moveCommand);
		if (checkForFinishCondition()) {
			gameFinished = TRUE;
			printLog("   Game finished\n");
		}
		return;
	}


	// otherwise, add the stroke to the board
	if (stroke.type != STROKE_LINE) {
		printLog("   Board isn't finished, skipping non-LINE stroke\n");
		return;
	}
	boardLines[numBoardLines] = stroke.line;
	numBoardLines++;
	if (numBoardLines < 4) {
		return;
	}


	// determine board lines (two pairs of non-intersecting lines)
	int idx, match1b, match2a, match2b;
	match1b = 0;
	for (idx = 1; idx < 4; idx++) {
		if (!doLinesIntersect(&boardLines[0], &boardLines[idx])) {
			if (match1b != 0) {
				// matched twice, so something is wrong
				match1b = 0;
				break;
			}
			match1b = idx;
			match2a = (idx == 1) ? 2 : 1;
			match2b = (idx == 3) ? 2 : 3;
		}
	}

	// verify that matched lines intersect properly
	if ((match1b == 0) ||
	    (!doLinesIntersect(&boardLines[match1b], &boardLines[match2a])) ||
	    (!doLinesIntersect(&boardLines[match1b], &boardLines[match2b])) ||
	    (doLinesIntersect(&boardLines[match2a], &boardLines[match2b]))) {
		printLog("   Could not match board lines, trying again without first line\n");
		boardLines[0] = boardLines[1];
		boardLines[1] = boardLines[2];
		boardLines[2] = boardLines[3];
		numBoardLines = 3;
		return;
	}

	// assign proper sides (within 90 degree range away from pure vertical or horizontal)
	double angle = fabs(boardLines[0].angleDegrees);
	BOOL isLeft, isTop;
	int leftIdx, rightIdx, topIdx, bottomIdx;
	if (angle >= 45 && angle < 135) {
		// match 1 is vertical, match 2 is horizontal
		isLeft = (boardLines[0].centerX < boardLines[match1b].centerX);
		isTop = (boardLines[match2a].centerY < boardLines[match2b].centerY);
		leftIdx = isLeft ? 0 : match1b;
		rightIdx = isLeft ? match1b : 0;
		topIdx = isTop ? match2a : match2b;
		bottomIdx = isTop ? match2b : match2a;
	} else {
		// match 2 is vertical, match 1 is horizontal
		isLeft = (boardLines[match2a].centerX < boardLines[match2b].centerX);
		isTop = (boardLines[0].centerY < boardLines[match1b].centerY);
		leftIdx = isLeft ? match2a : match2b;
		rightIdx = isLeft ? match2b : match2a;
		topIdx = isTop ? 0 : match1b;
		bottomIdx = isTop ? match1b : 0;
	}
	boardLeft = &boardLines[leftIdx];
	boardRight = &boardLines[rightIdx];
	boardTop = &boardLines[topIdx];
	boardBottom = &boardLines[bottomIdx];

	// calculate center of the board (average of the 4 intersection points)
	double iLeftTop = intersectionPoint(boardLeft, boardTop);
	double iLeftBottom = intersectionPoint(boardLeft, boardBottom);
	double iRightTop = intersectionPoint(boardRight, boardTop);
	double iRightBottom = intersectionPoint(boardRight, boardBottom);
	boardCenterX = projectedX(boardLeft, iLeftTop, TRUE);
	boardCenterX += projectedX(boardLeft, iLeftBottom, TRUE);
	boardCenterX += projectedX(boardRight, iRightTop, TRUE);
	boardCenterX += projectedX(boardRight, iRightBottom, TRUE);
	boardCenterY = projectedY(boardLeft, iLeftTop, TRUE);
	boardCenterY += projectedY(boardLeft, iLeftBottom, TRUE);
	boardCenterY += projectedY(boardRight, iRightTop, TRUE);
	boardCenterY += projectedY(boardRight, iRightBottom, TRUE);
	boardCenterX /= 4;
	boardCenterY /= 4;
	printLog("   Detected board strokes: Left %d, Right %d, Top %d, Bottom %d, with center (%.3f, %.3f)\n", \
	         leftIdx, rightIdx, topIdx, bottomIdx, boardCenterX, boardCenterY);
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
