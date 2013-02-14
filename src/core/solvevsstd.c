/************************************************************************
**
** NAME:	solvevsstd.c
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
#include "bpdb_misc.h"
#include "solvevsstd.h"


/*
** Code
*/

// Data to be stored in each slice of the database
UINT32 VALUESLOT = 0;
UINT32 MEXSLOT = 0;
UINT32 WINBYSLOT = 0;
UINT32 REMSLOT = 0;
UINT32 VISITEDSLOT = 0;

VALUE DetermineValueVSSTD(POSITION position)
{
	GMSTATUS status = STATUS_SUCCESS;
	VALUE gameval = undecided;

	if(!gBitPerfectDB) {
		status = STATUS_MISSING_DEPENDENT_MODULE;
		BPDB_TRACE("DetermineValueVSSTD()", "Bit-Perfect DB must be the selected DB to use the slices solver", status);
		// i wouldn't use this exit call if the status code
		// was allowed to propogate up
		exit(0);
		goto _bailout;
	}

	//
	// add slots to database slices
	//

	status = AddSlot( 2, "VALUE", TRUE, FALSE, FALSE, &VALUESLOT );         // slot 0
	if(!GMSUCCESS(status)) {
		BPDB_TRACE("DetermineValueVSSTD()", "Could not add value slot", status);
		goto _bailout;
	}

	if(gPutWinBy) {
		status = AddSlot( 3, "WINBY", TRUE, TRUE, FALSE, &WINBYSLOT );          // slot 2
		if(!GMSUCCESS(status)) {
			BPDB_TRACE("DetermineValueVSSTD()", "Could not add winby slot", status);
			goto _bailout;
		}
	}

	if(!kPartizan) {
		status = AddSlot( 3, "MEX", TRUE, TRUE, FALSE, &MEXSLOT );          // slot 2
		if(!GMSUCCESS(status)) {
			BPDB_TRACE("DetermineValueVSSTD()", "Could not add mex slot", status);
			goto _bailout;
		}
	}

	status = AddSlot( 5, "REMOTENESS", TRUE, TRUE, TRUE, &REMSLOT );        // slot 4
	if(!GMSUCCESS(status)) {
		BPDB_TRACE("DetermineValueVSSTD()", "Could not add remoteness slot", status);
		goto _bailout;
	}

	status = AddSlot( 1, "VISITED", FALSE, FALSE, FALSE, &VISITEDSLOT );    // slot 1
	if(!GMSUCCESS(status)) {
		BPDB_TRACE("DetermineValueVSSTD()", "Could not add visited slot", status);
		goto _bailout;
	}

	//
	// allocate
	//

	status = Allocate();
	if(!GMSUCCESS(status)) {
		BPDB_TRACE("DetermineValueVSSTD()", "Could not allocate database", status);
		goto _bailout;
	}

	//
	// use standard dfs solver
	//

	gameval = DetermineValueVSSTDHelper( position );

_bailout:
	if(!GMSUCCESS(status)) {
		return undecided;
	} else {
		return gameval;
	}
}

VALUE DetermineValueVSSTDHelper( POSITION position )
{
	BOOLEAN foundTie = FALSE, foundLose = FALSE, foundWin = FALSE;
	MOVELIST *ptr, *head;
	VALUE value;
	POSITION child;
	REMOTENESS maxRemoteness = 0, minRemoteness = MAXINT2;
	REMOTENESS minTieRemoteness = MAXINT2, remoteness;
	MEXCALC theMexCalc = 0; /* default to satisfy compiler */
	int winByValue = 0, minWinByValue = ((1 << (MEX_BITS-1))-1), maxWinByValue = -(1 << (MEX_BITS-1));

	if(GetSlot(position, VISITEDSLOT)) { /* Cycle! */
		printf("Sorry, but I think this is a loopy game. I give up.");
		ExitStageRight();
		exit(0);
	}
	/* It's been seen before and value has been determined */
	else if((value = GetSlot(position, VALUESLOT)) != undecided) {
		return(value);
	} else if((value = Primitive(position)) != undecided) {
		/* first time, end */
		SetSlot(position, REMSLOT, 0); /* terminal positions have 0 remoteness */
		if(!kPartizan && !gTwoBits)
			SetSlot(position, MEXSLOT, MexPrimitive(value)); /* lose=0, win=* */
		else if (kPartizan && gPutWinBy && !gTwoBits)
			SetSlot(position, WINBYSLOT, (gPutWinBy(position) & (MEX_MASK >> MEX_SHIFT)));
		return(SetSlot(position, VALUESLOT, value));
		/* first time, need to recursively determine value */
	} else {
		SetSlot(position, VISITEDSLOT, 1);

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

			value = DetermineValueVSSTDHelper(child); /* DFS call */

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

			//remoteness = Remoteness(child);
			remoteness = GetSlot(child, REMSLOT);
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
		}
		FreeMoveList(head);
		SetSlot(position, VISITEDSLOT, 0);

		if(!kPartizan && !gTwoBits)
			SetSlot(position, MEXSLOT, MexCompute(theMexCalc));
		else if (kPartizan && gPutWinBy && !gTwoBits) {
			int turn = generic_hash_turn(position);
			if (turn == 1)
				winByValue = maxWinByValue;
			else if (turn == 2)
				winByValue = minWinByValue;
			else BadElse("Bad generic_hash_turn(position)");
			SetSlot(position, WINBYSLOT, (winByValue & (MEX_MASK >> MEX_SHIFT)));
		}
		if(foundLose) {
			SetSlot(position, REMSLOT, minRemoteness+1);
			return (SetSlot(position, VALUESLOT, win));
		}
		else if(foundTie) {
			SetSlot(position, REMSLOT, minTieRemoteness+1);
			return (SetSlot(position, VALUESLOT, tie));
		}
		else if (foundWin) {
			SetSlot(position, REMSLOT, maxRemoteness+1);
			return (SetSlot(position, VALUESLOT, lose));
		}
		else
			BadElse("DetermineValue[2]. GenereateMoves most likely didnt return anything.");
	}
	BadElse("DetermineValue[3]"); /* This should NEVER be reached */
	return(undecided);      /* But has been added to satisty lint */
}

