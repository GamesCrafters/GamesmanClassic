/************************************************************************
**
** NAME:	solveloopyga.c
**
** DESCRIPTION:	Go-again loopy solver
**
** AUTHOR:	Bryon Ross
**		GamesCrafters Research Group, UC Berkeley
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
#include "solveloopyga.h"
#include "solveloopy.h"


/*
** Local types
*/

typedef struct lgas_position_move_list {
	POSITION position;
	MOVE move;
	struct lgas_position_move_list* next;
} POSITIONMOVELIST;


/*
** Local variables
*/

static POSITIONMOVELIST** lgas_gParents = NULL;


/*
** Local function prototypes
*/

VALUE                           lgas_DetermineValue(POSITION position);
static VALUE                    lgas_DetermineLoopyValue1(POSITION position);
static POSITIONMOVELIST*        CreatePositionMoveNode(POSITION position, MOVE move, POSITIONMOVELIST* next);
static void                     lgas_SetParents (POSITION parent, POSITION root);
static void                     FreePositionMoveList(POSITIONMOVELIST* posmovelist);
static void                     lgas_ParentInitialize();
static void                     lgas_ParentFree();


/*
** Code
*/

VALUE lgas_DetermineValue(POSITION position)
{
	VALUE value;

	/* initialize */
	InitializeFR();
	lgas_ParentInitialize();
	NumberChildrenInitialize();

	value = lgas_DetermineLoopyValue1(position);

	/* free */
	NumberChildrenFree();
	lgas_ParentFree();
	SaveDatabase();

	return value;
}


VALUE lgas_DetermineLoopyValue1(position)
POSITION position;
{
	POSITION child=kBadPosition, parent;
	MOVE move;
	POSITIONMOVELIST *ptr;
	VALUE childValue, adjustedChildValue;
	REMOTENESS remotenessChild;
	POSITION i;

	/* Do DFS to set up Parent pointers and initialize KnownList w/Primitives */



	lgas_SetParents(kBadPosition,position);
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
	while ((gHeadLoseFR != NULL) ||
	       (gHeadWinFR != NULL)) {
		if ((child = DeQueueLoseFR()) == kBadPosition)
			child = DeQueueWinFR();

		/* Might as well grab these now, they'll be used later */
		childValue = GetValueOfPosition(child);
		remotenessChild = Remoteness(child);

		/* If debugging, print who's in list */
		if(kDebugDetermineValue)
			printf("Grabbing " POSITION_FORMAT " (%s) remoteness = %d off of FR\n",
			       child,gValueString[childValue],remotenessChild);

		ptr = lgas_gParents[child];

		/* With losing children, every parent is winning, so we just go through
		** all the parents and declare them winning */
		while (ptr != NULL) {

			parent = ptr->position;
			move = ptr->move;

			if (parent != kBadPosition && gGoAgain(parent,move)) {
				switch(childValue) {
				case win:  adjustedChildValue = lose; break;
				case lose: adjustedChildValue = win; break;
				default:   adjustedChildValue = childValue;
				}
			} else {
				adjustedChildValue = childValue;
			}

			if (adjustedChildValue == lose) {

				/* Skip if this is the initial position (parent is kBadPosition) */
				if (parent != kBadPosition) {
					if (GetValueOfPosition(parent) == undecided) {
						/* This is the first time we know the parent is a win */
						InsertWinFR(parent);
						if(kDebugDetermineValue) printf("Inserting " POSITION_FORMAT " (%s) remoteness = %d into win FR\n",parent,"win",remotenessChild+1);
						SetRemoteness(parent, remotenessChild + 1);
						StoreValueOfPosition(parent,win);
					}
					else {
						/* We already know the parent is a winning position. */

						if (GetValueOfPosition(parent) != win) {
							printf(POSITION_FORMAT " should be win.  Instead it is %d.", parent, GetValueOfPosition(parent));
							BadElse("lgas_DetermineValue");
						}

					}


				} /* while there are still parents */

				/* With winning children */
			} else if (adjustedChildValue == win) {

				/* Skip if this is the initial position (parent is kBadPosition) */
				/* If this is the last unknown child and they were all wins, parent is lose */
				if(parent != kBadPosition && --gNumberChildren[parent] == 0) {
					/* no more kids, it's not been seen before, assign it as losing, put at head */
					assert(GetValueOfPosition(parent) == undecided);

					InsertLoseFR(parent);
					if(kDebugDetermineValue) printf("Inserting " POSITION_FORMAT " (%s) into FR head\n",parent,"lose");
					/* We always need to change the remoteness because we examine winning node with
					** less remoteness first. */
					SetRemoteness(parent, remotenessChild + 1);
					StoreValueOfPosition(parent,lose);
				}
			} else {
				BadElse("lgas_DetermineValue found FR member with other than win/lose value");
			}

			ptr = ptr->next;
		} /* With children set to other than win/lose. So stop */

		/* We are done with this position and no longer need to keep around its list of parents
		** The tie frontier will not need this, either, because this child's value has already
		** been determined.  It cannot be a tie. */
		FreePositionMoveList(lgas_gParents[child]);
		lgas_gParents[child] = NULL;

	} /* while still positions in FR */

	/* Now process the tie frontier */

	while(gHeadTieFR != NULL) {
		child = DeQueueTieFR();
		remotenessChild = Remoteness(child);

		ptr = lgas_gParents[child];

		while (ptr != NULL) {
			parent = ptr->position;

			if(GetValueOfPosition(parent) == undecided) {
				/* this position has no losing children but has a tieing position so it must be a
				 * tie. Assign its value and set its remoteness.  Note that
				 * we give ties with lowest remoteness priority (i.e. if a
				 * position has no losing children, a tieing child of
				 * remoteness 2, and a tieing child of remoteness 10, the
				 * position will be a tie of remoteness 3, not 11.  This
				 * decision is pretty arbitrary.  We did it this way to be
				 * consistent with DetermineValue for non-loopy games. */

				InsertTieFR(parent);
				if(kDebugDetermineValue) printf("Inserting " POSITION_FORMAT " (%s) remoteness = %d into win FR\n",parent,"tie",remotenessChild+1);
				SetRemoteness(parent, remotenessChild + 1);
				StoreValueOfPosition(parent,tie);
			}
			ptr = ptr->next;
		}
		FreePositionMoveList(lgas_gParents[child]);
		lgas_gParents[child] = NULL;
	}

	/* Now set all remaining positions to tie with remoteness of REMOTENESS_MAX */

	if(kDebugDetermineValue) {
		printf("---------------------------------------------------------------\n");
		//MyPrintFR();
		printf("---------------------------------------------------------------\n");
		MyPrintParents();
		printf("---------------------------------------------------------------\n");
		printf("TIE cleanup\n");
	}

	for (i = 0; i < gNumberOfPositions; i++)
		if(Visited(i)) {
			if(kDebugDetermineValue)
				printf(POSITION_FORMAT " was visited...",i);
			if(GetValueOfPosition((POSITION)i) == undecided) {
				SetRemoteness((POSITION)i,REMOTENESS_MAX);
				StoreValueOfPosition((POSITION)i,tie);
				//we are done with this position and no longer need to keep around its list of parents
				if (lgas_gParents[child]) FreePositionMoveList(lgas_gParents[child]);
				if(kDebugDetermineValue)
					printf("and was undecided, setting to tie\n");
			} else
			if(kDebugDetermineValue)
				printf("but was decided, ignoring\n");
			UnMarkAsVisited((POSITION)i);
		}

	return(GetValueOfPosition(position));
}


/*
** Requires: the root has not been visited yet
** (We do not check to see if its been visited)
*/

void lgas_SetParents (POSITION parent, POSITION root)
{
	MOVELIST* moveptr, * movehead;
	POSITIONLIST* posptr, * thisLevel, * nextLevel;
	POSITION pos, child;
	VALUE value;
	MOVE move = -1; /* initialized to dummy value */

	posptr = thisLevel = nextLevel = NULL;
	moveptr = movehead = NULL;

	// Check if the top is primitive.

	MarkAsVisited(root);
	lgas_gParents[root] = CreatePositionMoveNode(parent, move, lgas_gParents[root]);
	if ((value = Primitive(root)) != undecided) {
		SetRemoteness(root, 0);
		switch (value) {
		case lose: InsertLoseFR(root); break;
		case win:  InsertWinFR(root); break;
		case tie:  InsertTieFR(root); break;
		default:   BadElse("lgas_SetParents found primitive with value other than win/lose/tie");
		}

		StoreValueOfPosition(root, value);
		return;
	}

	thisLevel = StorePositionInList(root, thisLevel);

	while (thisLevel != NULL) {
		for (posptr = thisLevel; posptr != NULL; posptr = posptr->next) {
			pos = posptr->position;

			movehead = GenerateMoves(pos);

			for (moveptr = movehead; moveptr != NULL; moveptr = moveptr->next) {
				move = moveptr->move;
				child = DoMove(pos, move);
				++gNumberChildren[(int)pos];
				lgas_gParents[(int)child] = CreatePositionMoveNode(pos, move, lgas_gParents[(int)child]);

				if (Visited(child)) continue;
				MarkAsVisited(child);

				if ((value = Primitive(child)) != undecided) {
					SetRemoteness(child, 0);
					switch (value) {
					case lose: InsertLoseFR(child); break;
					case win: InsertWinFR(child);  break;
					case tie: InsertTieFR(child);  break;
					default: BadElse("lgas_SetParents found bad primitive value");
					}
					StoreValueOfPosition(child, value);
				} else {
					nextLevel = StorePositionInList(child, nextLevel);
				}
			}

			FreeMoveList(movehead);
		}

		FreePositionList(thisLevel);

		thisLevel = nextLevel;
		nextLevel = NULL;
	}
}

POSITIONMOVELIST* CreatePositionMoveNode(POSITION position, MOVE move, POSITIONMOVELIST* next) {
	POSITIONMOVELIST* result;

	result = (POSITIONMOVELIST*) SafeMalloc(sizeof(POSITIONMOVELIST));
	result->position = position;
	result->move = move;
	result->next = next;

	return result;
}


void FreePositionMoveList(POSITIONMOVELIST* posmovelist) {
	POSITIONMOVELIST* tmp;

	while(posmovelist != NULL) {
		tmp = posmovelist;
		posmovelist = posmovelist->next;
		SafeFree(tmp);
	}
}


void lgas_ParentInitialize()
{
	POSITION i;

	lgas_gParents = (POSITIONMOVELIST **) SafeMalloc (gNumberOfPositions * sizeof(POSITIONLIST *));
	for(i = 0; i < gNumberOfPositions; i++)
		lgas_gParents[i] = NULL;
}


void lgas_ParentFree()
{
	POSITION i;

	for (i = 0; i < gNumberOfPositions; i++) {
		FreePositionMoveList(lgas_gParents[i]);
	}

	SafeFree(lgas_gParents);
}
