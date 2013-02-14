/************************************************************************
**
** NAME:	solvevsloopy.c
**
** DESCRIPTION:	The infamous loopy solver.
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
#include "solvevsloopy.h"
#include "analysis.h"
#include "openPositions.h"

/*
** Globals
*/

FRnode*         gVSHeadWinFR = NULL;    /* The FRontier Win Queue */
FRnode*         gVSTailWinFR = NULL;
FRnode*         gVSHeadLoseFR = NULL;   /* The FRontier Lose Queue */
FRnode*         gVSTailLoseFR = NULL;
FRnode*         gVSHeadTieFR = NULL;    /* The FRontier Tie Queue */
FRnode*         gVSTailTieFR = NULL;
POSITIONLIST**  gVSParents = NULL;      /* The Parent of each node in a list */
char*           gVSNumberChildren = NULL;       /* The Number of children (used for Loopy games) */
char*       gVSNumberChildrenOriginal = NULL; /* Open Positions: for finding level1 frontier */

// Data to be stored in each slice of the database
UINT32 SL_VALUESLOT = 0;
UINT32 SL_MEXSLOT = 0;
UINT32 SL_WINBYSLOT = 0;
UINT32 SL_REMSLOT = 0;
UINT32 SL_VISITEDSLOT = 0;

/*
** Local function prototypes
*/

static void             VSParentInitialize              (void);
static VALUE    VSDetermineLoopyValue1          (POSITION pos);
static void             VSParentFree                    (void);
static void             VSSetParents                    (POSITION bad, POSITION root);


/*
** Code
*/

void VSMyPrintParents()
{
	POSITION i;
	POSITIONLIST *ptr;

	printf("PARENTS | #Children | Value\n");

	for(i=0; i<gNumberOfPositions; i++)
		if(Visited(i)) {
			ptr = gVSParents[i];
			printf(POSITION_FORMAT ": ",i);
			while (ptr != NULL) {
				printf("[" POSITION_FORMAT "] ",ptr->position);
				ptr = ptr->next;
			}
			printf("| %d children | %s value",(int)gVSNumberChildren[i],gValueString[GetValueOfPosition((POSITION)i)]);
			printf("\n");
		}
}

VALUE VSDetermineLoopyValue(POSITION position)
{
	GMSTATUS status = STATUS_SUCCESS;
	VALUE value;

	/* initialize */
	VSInitializeFR();
	VSParentInitialize();
	VSNumberChildrenInitialize();
	//if (gTwoBits)
	//   InitializeVisitedArray();


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

	status = AddSlot( 2, "VALUE", TRUE, FALSE, FALSE, &SL_VALUESLOT );         // slot 0
	if(!GMSUCCESS(status)) {
		BPDB_TRACE("DetermineValueVSSTD()", "Could not add value slot", status);
		goto _bailout;
	}

	if(gPutWinBy) {
		status = AddSlot( 3, "WINBY", TRUE, TRUE, FALSE, &SL_WINBYSLOT );          // slot 2
		if(!GMSUCCESS(status)) {
			BPDB_TRACE("DetermineValueVSSTD()", "Could not add winby slot", status);
			goto _bailout;
		}
	}

	if(!kPartizan) {
		status = AddSlot( 3, "MEX", TRUE, TRUE, FALSE, &SL_MEXSLOT );          // slot 2
		if(!GMSUCCESS(status)) {
			BPDB_TRACE("DetermineValueVSSTD()", "Could not add mex slot", status);
			goto _bailout;
		}
	}

	status = AddSlot( 5, "REMOTENESS", TRUE, TRUE, TRUE, &SL_REMSLOT );        // slot 4
	if(!GMSUCCESS(status)) {
		BPDB_TRACE("DetermineValueVSSTD()", "Could not add remoteness slot", status);
		goto _bailout;
	}

	status = AddSlot( 1, "VISITED", FALSE, FALSE, FALSE, &SL_VISITEDSLOT );    // slot 1
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


	value = VSDetermineLoopyValue1(gInitialPosition);
	if(gUseOpen) {
		ComputeOpenPositions();
	}

	//PrintOpenDataFormatted();
	/* free */
	VSNumberChildrenFree(); // Not sure why this was commented out, but not making
	// this call was causing memory leaks
	VSParentFree();
	//FreeVisitedArray();

_bailout:
	if(!GMSUCCESS(status)) {
		return undecided;
	} else {
		return value;
	}
}

VALUE VSDetermineLoopyValue1(POSITION position)
{
	POSITION child=kBadPosition, parent;
	POSITIONLIST *ptr;
	VALUE childValue;
	REMOTENESS remotenessChild;
	POSITION i;
	POSITION F0EdgeCount = 0;
	POSITION F0NodeCount = 0;
	POSITION F0DrawEdgeCount = 0;

	/* Do DFS to set up Parent pointers and initialize KnownList w/Primitives */
	VSSetParents(kBadPosition,position);
	if(kDebugDetermineValue) {
		printf("---------------------------------------------------------------\n");
		printf("Number of Positions = [" POSITION_FORMAT "]\n",gNumberOfPositions);
		printf("---------------------------------------------------------------\n");
		// MyPrintParents();
		printf("---------------------------------------------------------------\n");
		//MyPrintFR();
		printf("---------------------------------------------------------------\n");
	}
	/* Now, the fun part. Starting from the children, work your way back up. */
	//@@ separate lose/win frontiers
	while ((gVSHeadLoseFR != NULL) ||
	       (gVSHeadWinFR != NULL)) {

		if ((child = VSDeQueueLoseFR()) == kBadPosition)
			child = VSDeQueueWinFR();

		/* Might as well grab these now, they'll be used later */
		childValue = GetSlot(child, SL_VALUESLOT); //GetValueOfPosition(child);
		remotenessChild = GetSlot(child, SL_REMSLOT); //Remoteness(child);

		/* If debugging, print who's in list */
		if(kDebugDetermineValue)
			printf("Grabbing " POSITION_FORMAT " (%s) remoteness = %d off of FR\n",
			       child,gValueString[childValue],remotenessChild);

		/* With losing children, every parent is winning, so we just go through
		** all the parents and declare them winning */
		if (childValue == lose) {
			ptr = gVSParents[child];
			while (ptr != NULL) {

				/* Make code easier to read */
				parent = ptr->position;

				/* Skip if this is the initial position (parent is kBadPosition) */
				if (parent != kBadPosition) {
					if (GetSlot(parent, SL_VALUESLOT) == undecided) {
						/* This is the first time we know the parent is a win */
						VSInsertWinFR(parent);
						if(kDebugDetermineValue) printf("Inserting " POSITION_FORMAT " (%s) remoteness = %d into win FR\n",parent,"win",remotenessChild+1);
						SetSlot(parent, SL_REMSLOT, remotenessChild + 1);
						SetSlot(parent, SL_VALUESLOT, win);

					}
					else {
						/* We already know the parent is a winning position. */

						if (GetSlot(parent, SL_VALUESLOT) != win) {
							printf(POSITION_FORMAT " should be win.  Instead it is %d.", parent, (int)GetSlot(parent, SL_VALUESLOT));
							BadElse("DetermineLoopyValue");
						}

						/* This should always hold because the frontier is a queue.
						** We always examine losing nodes with less remoteness first */
						assert((remotenessChild + 1) >= GetSlot(parent, SL_REMSLOT));
					}
				}
				ptr = ptr->next;
			} /* while there are still parents */

			/* With winning children */
		} else if (childValue == win) {
			ptr = gVSParents[child];
			while (ptr != NULL) {

				/* Make code easier to read */
				parent = ptr->position;

				/* Skip if this is the initial position (parent is kBadPosition) */
				/* If this is the last unknown child and they were all wins, parent is lose */
				if(parent != kBadPosition && --gVSNumberChildren[parent] == 0) {
					/* no more kids, it's not been seen before, assign it as losing, put at head */
					assert(GetSlot(parent, SL_VALUESLOT) == undecided);
					F0EdgeCount -= (gVSNumberChildrenOriginal[parent] - 1);
					VSInsertLoseFR(parent);
					if(kDebugDetermineValue) printf("Inserting " POSITION_FORMAT " (%s) into FR head\n",parent,"lose");
					/* We always need to change the remoteness because we examine winning node with
					** less remoteness first. */
					SetSlot(parent, SL_REMSLOT, remotenessChild + 1);
					SetSlot(parent, SL_VALUESLOT, lose);
				} else if (parent != kBadPosition) {
					F0EdgeCount++;
				}
				ptr = ptr->next;
			} /* while there are still parents */

			/* With children set to other than win/lose. So stop */
		} else {
			BadElse("DetermineLoopyValue found FR member with other than win/lose value");
		} /* else */

		/* We are done with this position and no longer need to keep around its list of parents
		** The tie frontier will not need this, either, because this child's value has already
		** been determined.  It cannot be a tie. */
		FreePositionList(gVSParents[child]);
		gVSParents[child] = NULL;

	} /* while still positions in FR */

	/* Now process the tie frontier */

	while(gVSHeadTieFR != NULL) {
		child = VSDeQueueTieFR();
		remotenessChild = GetSlot(child, SL_REMSLOT);

		ptr = gVSParents[child];

		while (ptr != NULL) {
			parent = ptr->position;

			if(parent != kBadPosition && GetSlot(parent, SL_VALUESLOT) == undecided) {
				/* this position has no losing children but has a tieing position so it must be a
				 * tie. Assign its value and set its remoteness.  Note that
				 * we give ties with lowest remoteness priority (i.e. if a
				 * position has no losing children, a tieing child of
				 * remoteness 2, and a tieing child of remoteness 10, the
				 * position will be a tie of remoteness 3, not 11.  This
				 * decision is pretty arbitrary.  We did it this way to be
				 * consistent with DetermineValue for non-loopy games. */

				VSInsertTieFR(parent);
				if(kDebugDetermineValue) printf("Inserting " POSITION_FORMAT " (%s) remoteness = %d into win FR\n",parent,"tie",remotenessChild+1);
				SetSlot(parent, SL_REMSLOT, remotenessChild + 1);
				SetSlot(parent, SL_VALUESLOT, tie);

				/*
				   gVSNumberChildren[parent] -= 1;
				   gVSNumberChildrenOriginal[parent] -=1; //As it is now, fringe0 can't have tie children
				 */
			}
			ptr = ptr->next;
		}
		FreePositionList(gVSParents[child]);
		gVSParents[child] = NULL;
	}

	/* Now set all remaining positions to tie with remoteness of REMOTENESS_MAX */

	if(kDebugDetermineValue) {
		printf("---------------------------------------------------------------\n");
		//MyPrintFR();
		printf("---------------------------------------------------------------\n");
		VSMyPrintParents();
		printf("---------------------------------------------------------------\n");
		printf("TIE cleanup\n");
	}

	for (i = 0; i < gNumberOfPositions; i++)
		if(Visited(i)) {
			if(kDebugDetermineValue)
				printf(POSITION_FORMAT " was visited...",i);
			if(GetSlot((POSITION) i, SL_VALUESLOT) == undecided) {
				SetSlotMax((POSITION) i, SL_REMSLOT);
				SetSlot((POSITION) i, SL_VALUESLOT, tie);

				if (gVSNumberChildren[i] < gVSNumberChildrenOriginal[i]) {
					F0DrawEdgeCount += gVSNumberChildren[i];
					F0NodeCount+=1;
				}
				//we are done with this position and no longer need to keep around its list of parents
				/*if (gVSParents[child])
				   FreePositionList(gVSParents[child]); */                                         // is this a memory leak?
				if(kDebugDetermineValue)
					printf("and was undecided, setting to draw\n");
			} else {
				if(kDebugDetermineValue)
					printf("but was decided, ignoring\n");
			}
			UnMarkAsVisited((POSITION)i);
		}

	if (gInterestingness) {
		DetermineInterestingness(position);
	}


	gAnalysis.F0EdgeCount = F0EdgeCount;
	gAnalysis.F0NodeCount = F0NodeCount;
	gAnalysis.F0DrawEdgeCount = F0DrawEdgeCount;
	return(GetSlot(position, SL_VALUESLOT));
}


/*
** Requires: the root has not been visited yet
** (We do not check to see if its been visited)
*/

void VSSetParents (POSITION parent, POSITION root)
{
	MOVELIST*       moveptr;
	MOVELIST*       movehead;
	POSITIONLIST*   posptr;
	POSITIONLIST*   thisLevel;
	POSITIONLIST*   nextLevel;
	POSITION pos;
	POSITION child;
	VALUE value;

	posptr = thisLevel = nextLevel = NULL;
	moveptr = movehead = NULL;

	// Check if the top is primitive.
	MarkAsVisited(root);

	gVSParents[root] = StorePositionInList(parent, gVSParents[root]);

	if ((value = Primitive(root)) != undecided) {
		SetRemoteness(root, 0);
		switch (value) {
		case lose: VSInsertLoseFR(root); break;
		case win:  VSInsertWinFR(root); break;
		case tie:  VSInsertTieFR(root); break;
		default:   BadElse("SetParents found primitive with value other than win/lose/tie");
		}

		StoreValueOfPosition(root, value);
		return;
	}

	thisLevel = StorePositionInList(root, thisLevel);

	while (thisLevel != NULL) {
		POSITIONLIST* next;

		for (posptr = thisLevel; posptr != NULL; posptr = next) {
			next = posptr->next;
			pos = posptr->position;

			movehead = GenerateMoves(pos);

			for (moveptr = movehead; moveptr != NULL; moveptr = moveptr->next) {
				child = DoMove(pos, moveptr->move);
				if (gSymmetries)
					child = gCanonicalPosition(child);

				if (child >= gNumberOfPositions)
					FoundBadPosition(child, pos, moveptr->move);
				++gVSNumberChildren[(int)pos];
				++gVSNumberChildrenOriginal[(int)pos];
				gVSParents[(int)child] = StorePositionInList(pos, gVSParents[(int)child]);

				if (Visited(child)) continue;
				MarkAsVisited(child);

				if ((value = Primitive(child)) != undecided) {
					SetRemoteness(child, 0);
					switch (value) {
					case lose: VSInsertLoseFR(child); break;
					case win: VSInsertWinFR(child);  break;
					case tie: VSInsertTieFR(child);  break;
					default: BadElse("SetParents found bad primitive value");
					}
					StoreValueOfPosition(child, value);
				} else {
					nextLevel = StorePositionInList(child, nextLevel);
				}
				gTotalMoves++;
			}

			FreeMoveList(movehead);

			/* Free as we go */
			free(posptr);
		}

		thisLevel = nextLevel;
		nextLevel = NULL;
	}
}


//void InitializeVisitedArray()
//{
//    size_t sz = (gNumberOfPositions >> 3) + 1;
//    gVisited = (char*) SafeMalloc (sz);
//    memset(gVisited, 0, sz);
//}

//void FreeVisitedArray()
//{
//    if (gVisited) SafeFree(gVisited);
//    gVisited = NULL;
//}

void VSParentInitialize()
{
	POSITION i;

	gVSParents = (POSITIONLIST **) SafeMalloc (gNumberOfPositions * sizeof(POSITIONLIST *));
	for(i = 0; i < gNumberOfPositions; i++)
		gVSParents[i] = NULL;
}

void VSParentFree()
{
	POSITION i;

	for (i = 0; i < gNumberOfPositions; i++) {
		FreePositionList(gVSParents[i]);
	}

	SafeFree(gVSParents);
}

void VSNumberChildrenInitialize()
{
	POSITION i;

	gVSNumberChildren = (char *) SafeMalloc (gNumberOfPositions * sizeof(signed char));
	gVSNumberChildrenOriginal = (char *) SafeMalloc (gNumberOfPositions * sizeof(signed char));     /* Open Positions: for finding level1 frontier */
	if (gInterestingness) {
		gAnalysis.Interestingness = (float *) SafeMalloc (gNumberOfPositions * sizeof(float)); /* Interestingness */
	}

	if (gInterestingness) {
		for(i = 0; i < gNumberOfPositions; i++) {
			gVSNumberChildren[i] = 0;
			gVSNumberChildrenOriginal[i] = 0;
			gAnalysis.Interestingness[i] = 0.0;
		}
	} else {
		for(i = 0; i < gNumberOfPositions; i++) {
			gVSNumberChildren[i] = 0;
			gVSNumberChildrenOriginal[i] = 0;
		}
	}

}

void VSNumberChildrenFree()
{                                                                                                      /* Open Positions: for finding level1 frontier */
	SafeFree(gVSNumberChildren);
	SafeFree(gVSNumberChildrenOriginal);
}

void VSInitializeFR()
{
	gVSHeadWinFR = NULL;
	gVSTailWinFR = NULL;
	gVSHeadLoseFR = NULL;
	gVSTailLoseFR = NULL;
	gVSHeadTieFR = NULL;
	gVSTailTieFR = NULL;
}

static POSITION VSDeQueueFR(FRnode **gHeadFR, FRnode **gTailFR)
{
	POSITION position;
	FRnode *tmp;

	if (*gHeadFR == NULL)
		return kBadPosition;
	else {
		position = (*gHeadFR)->position;
		tmp = *gHeadFR;
		(*gHeadFR) = (*gHeadFR)->next;
		SafeFree(tmp);

		if (*gHeadFR == NULL)
			*gTailFR = NULL;
	}
	return position;
}

POSITION VSDeQueueWinFR()
{
	return VSDeQueueFR(&gVSHeadWinFR, &gVSTailWinFR);
}

POSITION VSDeQueueLoseFR()
{
	return VSDeQueueFR(&gVSHeadLoseFR, &gVSTailLoseFR);
}

POSITION VSDeQueueTieFR()
{
	return VSDeQueueFR(&gVSHeadTieFR, &gVSTailTieFR);
}

static void VSInsertFR(POSITION position, FRnode **firstnode,
                       FRnode **lastnode)
{
	FRnode *tmp = (FRnode *) SafeMalloc(sizeof(FRnode));
	tmp->position = position;
	tmp->next = NULL;

	if (*lastnode == NULL) {
		assert(*firstnode == NULL);
		*firstnode = tmp;
		*lastnode = tmp;
	} else {
		assert((*lastnode)->next == NULL);
		(*lastnode)->next = tmp;
		*lastnode = tmp;
	}
}

void VSInsertWinFR(POSITION position)
{
	/* printf("Inserting WinFR...\n"); */
	VSInsertFR(position, &gVSHeadWinFR, &gVSTailWinFR);
}

void VSInsertLoseFR(POSITION position)
{
	/* printf("Inserting LoseFR...\n"); */
	VSInsertFR(position, &gVSHeadLoseFR, &gVSTailLoseFR);
}

void VSInsertTieFR(POSITION position)
{
	VSInsertFR(position, &gVSHeadTieFR, &gVSTailTieFR);
}

// End Loopy
