/************************************************************************
**
** NAME:	solvestd.c
**
** DESCRIPTION:	Vanilla non-loopy solver.
**
** AUTHOR:	GamesCrafters Research Group, UC Berkeley
**		Supervised by Dan Garcia <ddgarcia@cs.berkeley.edu>
**
** DATE:	2005-01-11
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


/*
** Code
*/

VALUE DetermineValueSTD(POSITION position)
{
	BOOLEAN foundTie = FALSE, foundLose = FALSE, foundWin = FALSE;
	MOVELIST *ptr, *head;
	VALUE value;
	POSITION child;
	REMOTENESS maxRemoteness = 0, minRemoteness = MAXINT2;
	REMOTENESS minTieRemoteness = MAXINT2, remoteness;
	MEXCALC theMexCalc = 0; /* default to satisfy compiler */
	int winByValue = 0, minWinByValue = ((1 << (MEX_BITS-1))-1), maxWinByValue = -(1 << (MEX_BITS-1));

	if(Visited(position)) { /* Cycle! */
		printf("Sorry, but I think this is a loopy game. I give up.");
		ExitStageRight();
		exit(0);
	}
	/* It's been seen before and value has been determined */
	else if((value = GetValueOfPosition(position)) != undecided) {
		return(value);
	} else if((value = Primitive(position)) != undecided) {
		/* first time, end */
		SetRemoteness(position,0); /* terminal positions have 0 remoteness */
		if(!kPartizan && !gTwoBits)
			MexStore(position,MexPrimitive(value)); /* lose=0, win=* */
		else if (kPartizan && gPutWinBy && !gTwoBits)
			WinByStore(position,gPutWinBy(position));
		return(StoreValueOfPosition(position,value));
		/* first time, need to recursively determine value */
	} else {
		MarkAsVisited(position);
		if(!kPartizan && !gTwoBits)
			theMexCalc = MexCalcInit();
		head = ptr = GenerateMoves(position);
		while (ptr != NULL) {
			MOVE move = ptr->move;
			gAnalysis.TotalMoves++;
			child = DoMove(position,ptr->move); /* Create the child */

			if(gSymmetries)
				child = gCanonicalPosition(child);

			if (child >= gNumberOfPositions)
				FoundBadPosition(child, position, move);

			value = DetermineValueSTD(child); /* DFS call */

			if (kPartizan && gPutWinBy && !gTwoBits) {
				int childWinByValue = WinByLoad(child);
				if (childWinByValue < minWinByValue)
					minWinByValue = childWinByValue;
				if (childWinByValue > maxWinByValue)
					maxWinByValue = childWinByValue;
			}

			if (gGoAgain(position,move))
				switch(value)
				{
				case lose: value=win; break;
				case win: value=lose; break;
				default: break; /* value stays the same */
				}

			remoteness = Remoteness(child);
			if(!kPartizan && !gTwoBits)
				theMexCalc = MexAdd(theMexCalc,MexLoad(child));
			if(value == lose) { /* found a way to give you a lose */
				foundLose = TRUE; /* thus, it's a winning move      */
				if (remoteness < minRemoteness) minRemoteness = remoteness;
			}
			else if(value == tie) { /* found a way to give you a tie  */
				foundTie = TRUE; /* thus, it's a tieing move       */
				if (remoteness < minTieRemoteness) minTieRemoteness = remoteness;
			}
			else if(value == win) { /* found a way to give you a win  */
				foundWin = TRUE; /* thus, it's a losing move       */
				if (remoteness > maxRemoteness) maxRemoteness = remoteness;
			}
			else
				BadElse("DetermineValue[1]");

			if (gUseGPS)
				gUndoMove(move);

			ptr = ptr->next;
		} //while
		FreeMoveList(head);
		UnMarkAsVisited(position);
		if(!kPartizan && !gTwoBits)
			MexStore(position,MexCompute(theMexCalc));
		else if (kPartizan && gPutWinBy && !gTwoBits) {
			int turn = generic_hash_turn(position);
			if (turn == 1)
				winByValue = maxWinByValue;
			else if (turn == 2)
				winByValue = minWinByValue;
			else BadElse("Bad generic_hash_turn(position)");
			WinByStore(position,winByValue);
		}
		if(foundLose) {
			SetRemoteness(position,minRemoteness+1); /* Winners want to mate soon! */
			return(StoreValueOfPosition(position,win));
		}
		else if(foundTie) {
			SetRemoteness(position,minTieRemoteness+1); /* Tiers want to mate now! */
			return(StoreValueOfPosition(position,tie));
		}
		else if (foundWin) {
			SetRemoteness(position,maxRemoteness+1); /* Losers want to extend! */
			return(StoreValueOfPosition(position,lose));
		}
		else
			BadElse("DetermineValue[2]. GenereateMoves most likely didnt return anything.");
	} // else
	BadElse("DetermineValue[3]"); /* This should NEVER be reached */
	return(undecided);      /* But has been added to satisty lint */
}

