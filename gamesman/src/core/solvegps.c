/************************************************************************
**
** NAME:	solvegps.c
**
** DESCRIPTION:	Global position solver.
**
** AUTHOR:	Eric Siroker
**		GamesCrafters Research Group, UC Berkeley
**		Supervised by Dan Garcia <ddgarcia@cs.berkeley.edu>
**
** DATE:	2005-03-12
**
** LICENSE:	This file is part of GAMESMAN,
**		The Finite, Two-person Perfect-Information Game Generator
**		Released under the GPL:
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program, in COPYING; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
**************************************************************************/

#include "gamesman.h"

VALUE GPS_DetermineValue(POSITION position) {
    BOOLEAN foundLose = FALSE, foundTie = FALSE, foundWin = FALSE;
    MEXCALC mexCalc = 0;
    MOVE move;
    MOVELIST *moveNode, *moves;
    POSITION child;
    REMOTENESS loseRemoteness = MAXINT2, tieRemoteness = MAXINT2,
               winRemoteness = 0, remoteness;
    VALUE value;

    if (gGPSDoMove == NULL || gGPSGenerateMoves == NULL ||
        gGPSHashPosition == NULL || gGPSPrimitive == NULL ||
        gGPSUndoMove == NULL)
        ExitStageRightErrorString("GPS functions not implemented.");

    if (Visited(position))
        ExitStageRightErrorString("GPS only works on loop-free games.");
    else if ((value = GetValueOfPosition(position)) != undecided)
        return value;
    else if ((value = gGPSPrimitive()) != undecided) {
        SetRemoteness(position, 0);

        if (!kPartizan)
            MexStore(position, MexPrimitive(value));

        return StoreValueOfPosition(position,value);
    }
    else {
        MarkAsVisited(position);

        if (!kPartizan)
            mexCalc = MexCalcInit();

        moves = moveNode = gGPSGenerateMoves();

        while (moveNode != NULL) {
            gAnalysis.TotalMoves++;
            move = moveNode->move;
            gGPSDoMove(move);
            child = gGPSHashPosition();

#ifdef SYMMETRY_REVISITED
            child = GetCanonicalPosition(child);
#endif

            if (child < 0 || child >= gNumberOfPositions)
                FoundBadPosition(child, position, move);

            value = GPS_DetermineValue(child);

            if (gGPSGoAgain != NULL && gGPSGoAgain(move)) {
                if (value == lose)
                    value = win;
                else if (value == win)
                    value = lose;
            }
	    
            remoteness = Remoteness(child);

            if (!kPartizan)
                mexCalc = MexAdd(mexCalc, MexLoad(child));

            switch (value) {
                case lose:
                    if (remoteness < loseRemoteness)
                        loseRemoteness = remoteness;

                    foundLose = TRUE;
                    break;
                case tie:
                    if (remoteness < tieRemoteness)
                        tieRemoteness = remoteness;

                    foundTie = TRUE;
                    break;
                case win:
                    if (remoteness > winRemoteness)
                        winRemoteness = remoteness;

                    foundWin = TRUE;
                    break;
                default:
                    break;
            }

            gGPSUndoMove(move);
            moveNode = moveNode->next;
        }

        FreeMoveList(moves);
        UnMarkAsVisited(position);

        if (!kPartizan)
            MexStore(position, MexCompute(mexCalc));

        if (foundLose) {
            SetRemoteness(position, loseRemoteness + 1);

            return StoreValueOfPosition(position, win);
        }
        else if (foundTie) {
            SetRemoteness(position, tieRemoteness + 1);

            return StoreValueOfPosition(position, tie);
        }
        else if (foundWin) {
            SetRemoteness(position, winRemoteness + 1);

            return StoreValueOfPosition(position, lose);
        }
    }

    return undecided;
}
