/************************************************************************
**
** NAME:	solveloopy.c
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
#include "solveloopy.h"
#include "analysis.h"
#include "openPositions.h"

/*
** Globals
*/

FRnode*         gHeadWinFR = NULL;      /* The FRontier Win Queue */
FRnode*         gTailWinFR = NULL;
FRnode*         gHeadLoseFR = NULL;     /* The FRontier Lose Queue */
FRnode*         gTailLoseFR = NULL;
FRnode*         gHeadTieFR = NULL;      /* The FRontier Tie Queue */
FRnode*         gTailTieFR = NULL;
POSITIONLIST**  gParents = NULL;        /* The Parent of each node in a list */
char*           gNumberChildren = NULL; /* The Number of children (used for Loopy games) */
char*       gNumberChildrenOriginal = NULL;


/*
** Local function prototypes
*/

static void             ParentInitialize                (void);
static VALUE    DetermineLoopyValue1            (POSITION pos);
static void             ParentFree                      (void);
static void             SetParents                      (POSITION bad, POSITION root);


/*
** Code
*/

void MyPrintParents()
{
	POSITION i;
	POSITIONLIST *ptr;

	printf("PARENTS | #Children | Value\n");

	for(i=0; i<gNumberOfPositions; i++)
		if(Visited(i)) {
			ptr = gParents[i];
			printf(POSITION_FORMAT ": ",i);
			while (ptr != NULL) {
				printf("[" POSITION_FORMAT "] ",ptr->position);
				ptr = ptr->next;
			}
			printf("| %d children | %s value",(int)gNumberChildren[i],gValueString[GetValueOfPosition((POSITION)i)]);
			printf("\n");
		}
}

VALUE DetermineLoopyValue(POSITION position)
{
	VALUE value;

	/* initialize */
	InitializeFR();
	ParentInitialize();
	NumberChildrenInitialize();
	//if (gTwoBits)
	//   InitializeVisitedArray();

	value = DetermineLoopyValue1(gInitialPosition);
	if(gUseOpen) {
		ComputeOpenPositions();
	}
	//printf("Got here\n");
	//PrintOpenDataFormatted();
	/* free */
	NumberChildrenFree();   // Not sure why this was commented out, but not making
	// this call was causing memory leaks
	ParentFree();
	//FreeVisitedArray();

	return value;
}

VALUE DetermineLoopyValue1(POSITION position)
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

	SetParents(kBadPosition,position);
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

		/* With losing children, every parent is winning, so we just go through
		** all the parents and declare them winning */
		if (childValue == lose) {
			ptr = gParents[child];
			while (ptr != NULL) {

				/* Make code easier to read */
				parent = ptr->position;

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
							BadElse("DetermineLoopyValue");
						}

						/* This should always hold because the frontier is a queue.
						** We always examine losing nodes with less remoteness first */
						assert((remotenessChild + 1) >= Remoteness(parent));
					}
				}
				ptr = ptr->next;
			} /* while there are still parents */

			/* With winning children */
		} else if (childValue == win) {
			ptr = gParents[child];
			while (ptr != NULL) {

				/* Make code easier to read */
				parent = ptr->position;

				/* Skip if this is the initial position (parent is kBadPosition) */
				/* If this is the last unknown child and they were all wins, parent is lose */
				if(parent != kBadPosition && --gNumberChildren[parent] == 0) {
					/* no more kids, it's not been seen before, assign it as losing, put at head */
					assert(GetValueOfPosition(parent) == undecided);
					F0EdgeCount -= (gNumberChildrenOriginal[parent] - 1);
					InsertLoseFR(parent);
					if(kDebugDetermineValue) printf("Inserting " POSITION_FORMAT " (%s) into FR head\n",parent,"lose");
					/* We always need to change the remoteness because we examine winning node with
					** less remoteness first. */
					SetRemoteness(parent, remotenessChild + 1);
					StoreValueOfPosition(parent,lose);
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
		FreePositionList(gParents[child]);
		gParents[child] = NULL;

	} /* while still positions in FR */

	/* Now process the tie frontier */

	while(gHeadTieFR != NULL) {
		child = DeQueueTieFR();
		remotenessChild = Remoteness(child);

		ptr = gParents[child];

		while (ptr != NULL) {
			parent = ptr->position;

			if(parent != kBadPosition && GetValueOfPosition(parent) == undecided) {
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
				/*
				   gNumberChildren[parent] -= 1;
				   gNumberChildrenOriginal[parent] -=1; //As it is now, fringe0 can't have tie children
				 */
			}
			ptr = ptr->next;
		}
		FreePositionList(gParents[child]);
		gParents[child] = NULL;
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
				if (gNumberChildren[i] < gNumberChildrenOriginal[i]) {
					F0DrawEdgeCount += gNumberChildren[i];
					F0NodeCount+=1;
				}
				//we are done with this position and no longer need to keep around its list of parents
				/*if (gParents[child])
				   FreePositionList(gParents[child]); */                                       // is this a memory leak?
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
	return(GetValueOfPosition(position));
}


/*
** This function is no longer in use (SetParents was rewritten over a year ago
** to use BFS and solve Gobblet), but here it is for reference purposes.
*/

void DFS_SetParents (POSITION parent, POSITION position)
{
	BOOLEAN Visited();
	MOVELIST *moveptr, *movehead, *GenerateMoves();
	VALUE Primitive(), value;
	POSITION child;
	POSITIONLIST *posptr, *poshead, *StorePositionInList();

	if(kDebugDetermineValue) printf("DV (" POSITION_FORMAT "," POSITION_FORMAT ")\n", parent, position);
	if(Visited(position)) { /* We've been down this path before, don't DFS */
		if(kDebugDetermineValue) printf("Seen\n");
		/* PARENT me */
		gParents[position] = StorePositionInList(parent, gParents[position]);
	} else if((value = Primitive(position)) != undecided) { /* Primitive */
		if(kDebugDetermineValue) printf("PRIM value = %s\n", gValueString[value]);
		SetRemoteness(position,0); /* Primitives are leaves, remoteness = 0 */
		MarkAsVisited(position);
		/* PARENT me */
		gParents[position] = StorePositionInList(parent, gParents[position]);
		/* Add me to FR. (I know i'm not already in the frontier because
		 * this is the first time i've been visited) */
		if(value == lose)
			InsertLoseFR(position);
		else if(value == win)
			InsertWinFR(position);
		else if(value == tie)
			InsertTieFR(position);
		else
			BadElse("DetermineLoopyValue1 found primitive with value other than win/lose/tie");
		/* Set the value */
		StoreValueOfPosition(position,value);
	} else { /* first time, need to recursively determine value */
		/* PARENT me */
		gParents[position] = StorePositionInList(parent, gParents[position]);
		if(kDebugDetermineValue) printf("normal, continue searching\n");
		MarkAsVisited(position);
		movehead = GenerateMoves(position);
		poshead = NULL;

		for (moveptr = movehead; moveptr != NULL; moveptr = moveptr->next) {
			gNumberChildren[(int)position]++; /* Record the number of kids */
			child = DoMove(position, moveptr->move); /* Create the child */
			if (Visited(child)) {        /* Visited? */
				DFS_SetParents(position, child); /* Go ahead and call (it'll be quick) */
			} else {
				/* Visit it later */
				poshead = StorePositionInList(child, poshead);
				/* Lock it so it gets evaluated here (as high in the tree as possible). */
				MarkAsVisited(child);
			}
		}

		FreeMoveList(movehead);

		/* Make DFS calls on all children. */
		for (posptr = poshead; posptr != NULL; posptr = posptr->next) {
			child = posptr->position;
			UnMarkAsVisited(child); /* Release our lock on it */
			DFS_SetParents(position, child); /* DFS Recursion */
		}

		FreePositionList(poshead);
	}
	return;      /* But has been added to satisfy lint */
}


/*
** Requires: the root has not been visited yet
** (We do not check to see if its been visited)
*/

void SetParents (POSITION parent, POSITION root)
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
	gParents[root] = StorePositionInList(parent, gParents[root]);
	if ((value = Primitive(root)) != undecided) {
		SetRemoteness(root, 0);
		switch (value) {
		case lose: InsertLoseFR(root); break;
		case win:  InsertWinFR(root); break;
		case tie:  InsertTieFR(root); break;
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
				++gNumberChildren[(int)pos];
				++gNumberChildrenOriginal[(int)pos];
				gParents[(int)child] = StorePositionInList(pos, gParents[(int)child]);

				if (Visited(child)) continue;
				MarkAsVisited(child);

				if ((value = Primitive(child)) != undecided) {
					SetRemoteness(child, 0);
					switch (value) {
					case lose: InsertLoseFR(child); break;
					case win: InsertWinFR(child);  break;
					case tie: InsertTieFR(child);  break;
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

void ParentInitialize()
{
	POSITION i;

	gParents = (POSITIONLIST **) SafeMalloc (gNumberOfPositions * sizeof(POSITIONLIST *));
	for(i = 0; i < gNumberOfPositions; i++)
		gParents[i] = NULL;
}

void ParentFree()
{
	POSITION i;

	for (i = 0; i < gNumberOfPositions; i++) {
		FreePositionList(gParents[i]);
	}

	SafeFree(gParents);
}

void NumberChildrenInitialize()
{
	POSITION i;

	gNumberChildren = (char *) SafeMalloc (gNumberOfPositions * sizeof(signed char));
	gNumberChildrenOriginal = (char *) SafeMalloc (gNumberOfPositions * sizeof(signed char));
	if (gInterestingness) {
		gAnalysis.Interestingness = (float *) SafeMalloc (gNumberOfPositions * sizeof(float)); /* Interestingness */
	}
	if (gInterestingness) {
		for(i = 0; i < gNumberOfPositions; i++) {
			gNumberChildren[i] = 0;
			gNumberChildrenOriginal[i] = 0;
			gAnalysis.Interestingness[i] = 0.0;
		}
	} else {
		for(i = 0; i < gNumberOfPositions; i++) {
			gNumberChildren[i] = 0;
			gNumberChildrenOriginal[i] = 0;
		}
	}
}

void NumberChildrenFree()
{
	SafeFree(gNumberChildren);
	SafeFree(gNumberChildrenOriginal);
}

void InitializeFR()
{
	gHeadWinFR = NULL;
	gTailWinFR = NULL;
	gHeadLoseFR = NULL;
	gTailLoseFR = NULL;
	gHeadTieFR = NULL;
	gTailTieFR = NULL;
}

static POSITION DeQueueFR(FRnode **gHeadFR, FRnode **gTailFR)
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

POSITION DeQueueWinFR()
{
	return DeQueueFR(&gHeadWinFR, &gTailWinFR);
}

POSITION DeQueueLoseFR()
{
	return DeQueueFR(&gHeadLoseFR, &gTailLoseFR);
}

POSITION DeQueueTieFR()
{
	return DeQueueFR(&gHeadTieFR, &gTailTieFR);
}

static void InsertFR(POSITION position, FRnode **firstnode,
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

void InsertWinFR(POSITION position)
{
	/* printf("Inserting WinFR...\n"); */
	InsertFR(position, &gHeadWinFR, &gTailWinFR);
}

void InsertLoseFR(POSITION position)
{
	/* printf("Inserting LoseFR...\n"); */
	InsertFR(position, &gHeadLoseFR, &gTailLoseFR);
}

void InsertTieFR(POSITION position)
{
	InsertFR(position, &gHeadTieFR, &gTailTieFR);
}

// End Loopy
