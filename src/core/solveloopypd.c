/************************************************************************
**
** NAME:	solvevsloopy.c
**
** DESCRIPTION:	Loopy solver with pure draw analysis.
**
** AUTHOR:	Robert Shi <robertyishi@berkeley.edu>
**      GamesCrafters Research Group, UC Berkeley
**		Supervised by Dan Garcia <ddgarcia@cs.berkeley.edu>
**
** DATE:	2022-09-17
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
#include "solveloopypd.h"
#include "analysis.h"

/*
** Globals
*/

/* FRontier Win Queue */
static FRnode*			winFRHead = NULL;       
static FRnode*			winFRTail = NULL;

/* FRontier Lose Queue */
static FRnode*			loseFRHead = NULL;
static FRnode*			loseFRTail = NULL;

/* FRontier Tie Queue */
static FRnode*			tieFRHead = NULL;       
static FRnode*			tieFRTail = NULL;

/* Linked lists of parents of each node. */
static POSITIONLIST**	parentsOf = NULL;

/* Number of children left undecided. */   
static char*			numberChildren = NULL;  

/* Data to be stored in each slice of the database. */
static UINT32 SL_VALUE_SLOT = 0;      /* Value of a position. */
static UINT32 SL_WINBY_SLOT = 0;      /* WinBy of a position. */
static UINT32 SL_REM_SLOT = 0;        /* Remoteness of a position. */
static UINT32 SL_DRAW_LEVEL_SLOT = 0; /* Draw level of a position;
								          DRAW_LEVEL_MAX for all positions
										  if draw is not pure. */
static UINT32 SL_VISITED_SLOT = 0;    /* 1 if position is visited,
										  0 otherwise. */

/*
** Local function prototypes
*/

static void     InitializeFR    (void);
static void     FinalCheckFR    (void);
static void 	InsertWinFR		(POSITION position);
static void 	InsertLoseFR	(POSITION position);
static void 	InsertTieFR		(POSITION position);
static POSITION DeQueueWinFR	(void);
static POSITION DeQueueLoseFR	(void);
static POSITION DeQueueTieFR	(void);

static void		InitializeParents           (void);
static void		FreeParents                 (void);
static void     InitializeNumberChildren    (void);
static void     FreeNumberChildren          (void);

static void		SetParents                  (POSITION root);
static VALUE	DetermineValueHelper        (POSITION pos);

/*
** Code
*/

/* Prints parents of all Visited positions in the game tree. */
void lpds_PrintParents() {
	POSITION i;
	POSITIONLIST *ptr;

	printf("PARENTS | #Children | Value\n");
	for (i = 0; i < gNumberOfPositions; ++i) {
		if (Visited(i)) {
			ptr = parentsOf[i];
			printf(POSITION_FORMAT ": ", i);
			while (ptr != NULL) {
				printf("[" POSITION_FORMAT "] ", ptr->position);
				ptr = ptr->next;
			}
			printf("| %d children | %s value", numberChildren[i], gValueString[GetValueOfPosition(i)]);
			printf("\n");
		}
    }
}

/* Returns the VALUE of the given POSITION. */
VALUE lpds_DetermineValue(POSITION position) {
	GMSTATUS status = STATUS_SUCCESS;
	VALUE value = undecided;

    /* This solver must be used with Bit-Perfect Database. */
	if (!gBitPerfectDB) {
		status = STATUS_MISSING_DEPENDENT_MODULE;
		BPDB_TRACE("lpds_DetermineValue()", "Bit-Perfect DB must be "
            "the selected DB to use the slices solver", status);
		return value;
	}

    /* Add slots to database slices. */
    /* Format: AddSlot(size,  name,       write,  adjust, reservemax, slotindex */
	status =   AddSlot(3,     "VALUE",    TRUE,   FALSE,  FALSE,      &SL_VALUE_SLOT);
	if (!GMSUCCESS(status)) {
		BPDB_TRACE("lpds_DetermineValue()", "Could not add value slot", status);
		return value;
	}
	if (gPutWinBy) {
		status = AddSlot(3, "WINBY", TRUE, TRUE, FALSE, &SL_WINBY_SLOT);
		if(!GMSUCCESS(status)) {
			BPDB_TRACE("lpds_DetermineValue()", "Could not add winby slot", status);
		    return value;
		}
	}
	status = AddSlot(5, "REMOTENESS", TRUE, TRUE, TRUE, &SL_REM_SLOT);
	if (!GMSUCCESS(status)) {
		BPDB_TRACE("lpds_DetermineValue()", "Could not add remoteness slot", status);
		return value;
	}
    status = AddSlot(2, "DRAW LEVEL", TRUE, TRUE, TRUE, &SL_DRAW_LEVEL_SLOT);
	if (!GMSUCCESS(status)) {
		BPDB_TRACE("lpds_DetermineValue()", "Could not add draw level slot", status);
		return value;
	}
	status = AddSlot(1, "VISITED", FALSE, FALSE, FALSE, &SL_VISITED_SLOT);
	if (!GMSUCCESS(status)) {
		BPDB_TRACE("lpds_DetermineValue()", "Could not add visited slot", status);
		return value;
	}

    /* Allocate database. */
	status = Allocate();
	if(!GMSUCCESS(status)) {
		BPDB_TRACE("DetermineValueVSSTD()", "Could not allocate database", status);
		return value;
	}

	/* Only initialize global arrays if database was successfully allocated. */
	InitializeFR();
	InitializeParents();
	InitializeNumberChildren();

    /* Solve from initial position. */
	value = DetermineValueHelper(gInitialPosition);

	/* Free global arrays. */
    FinalCheckFR();
	FreeNumberChildren();
	FreeParents();

	return value;
}

/*
** Helper functions
*/

static void InitializeFR(void) {
	winFRHead = NULL;
	winFRTail = NULL;
	loseFRHead = NULL;
	loseFRTail = NULL;
	tieFRHead = NULL;
	tieFRTail = NULL;
}

static POSITION DeQueueFR(FRnode **gHeadFR, FRnode **gTailFR) {
	POSITION position;
	FRnode *tmp;

	if (*gHeadFR == NULL) {
		return kBadPosition;
	} else {
		position = (*gHeadFR)->position;
		tmp = *gHeadFR;
		(*gHeadFR) = (*gHeadFR)->next;
		free(tmp);

		if (*gHeadFR == NULL) {
			*gTailFR = NULL;
		}
	}
	return position;
}

static POSITION DeQueueWinFR(void) {
	return DeQueueFR(&winFRHead, &winFRTail);
}

static POSITION DeQueueLoseFR(void) {
	return DeQueueFR(&loseFRHead, &loseFRTail);
}

static POSITION DeQueueTieFR(void) {
	return DeQueueFR(&tieFRHead, &tieFRTail);
}

static void InsertFR(POSITION position, FRnode **firstnode, FRnode **lastnode) {
	FRnode *tmp = (FRnode *)SafeMalloc(sizeof(FRnode));

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

static void InsertWinFR(POSITION position) {
	InsertFR(position, &winFRHead, &winFRTail);
}

static void InsertLoseFR(POSITION position) {
	InsertFR(position, &loseFRHead, &loseFRTail);
}

static void InsertTieFR(POSITION position) {
	InsertFR(position, &tieFRHead, &tieFRTail);
}

/* Checks that all FRontier lists are empty at the end of solving.
   Prints out warning message if they are not empty. */
static void FinalCheckFR(void) {
    if (winFRHead || winFRTail) {
        printf("Win frontier list not empty after solving.\n");
    }
    if (loseFRHead || loseFRTail) {
        printf("Lose frontier list not empty after solving.\n");
    }
    if (tieFRHead || tieFRTail) {
        printf("Tie frontier list not empty after solving.\n");
    }
}

static void InitializeParents(void) {
	parentsOf = (POSITIONLIST **)SafeCalloc(gNumberOfPositions, sizeof(POSITIONLIST *));
}

static void FreeParents(void) {
    POSITION i;

	for (i = 0; i < gNumberOfPositions; ++i) {
		FreePositionList(parentsOf[i]);
	}
	SafeFreeAndSetToNull(&parentsOf);
}

static void InitializeNumberChildren(void) {
	numberChildren = (char *)SafeCalloc(gNumberOfPositions, sizeof(signed char));
	if (gInterestingness) {
		gAnalysis.Interestingness = (float *)SafeCalloc(gNumberOfPositions, sizeof(float));
	}
}

static void FreeNumberChildren(void) {
    SafeFreeAndSetToNull(&numberChildren);
}

static VALUE SetPrimitiveOrEnqueue(POSITION pos, POSITIONLIST **nextLevel) {
	VALUE value = Primitive(pos);

	if (value != undecided) {
		SetRemoteness(pos, 0);
		switch (value) {
		case lose: InsertLoseFR(pos); break;
		case win:  InsertWinFR(pos);  break;
		case tie:  InsertTieFR(pos);  break;
		default:   BadElse("SetParents found bad primitive value");
		}
		SetSlot(pos, SL_VALUE_SLOT, value);
	} else {
		*nextLevel = StorePositionInList(pos, *nextLevel);
	}
	return value;
}

/* TODO: check if DFS can be used instead for memory optimizations. */
/* Performs breadth-first search from root position, visiting all reacheable
   positions. Sends all primitive positions to their respective queues.
   Builds a backward graph that shows the parents of each position.  */
static void SetParents(POSITION root) {
	MOVELIST*       moveptr = NULL;
	MOVELIST*       movehead = NULL;
	POSITIONLIST*   posptr = NULL;
	POSITIONLIST*   thisLevel = NULL;
	POSITIONLIST*   nextLevel = NULL;
	POSITIONLIST*   next;
	POSITION pos;
	POSITION child;

	/* Check if root is primitive. */
	MarkAsVisited(root);
	/* Set the only parent of root position as bad. Thus, a bad parent
	   indicates a root position. */
	parentsOf[root] = StorePositionInList(kBadPosition, parentsOf[root]);
	/* Edge case: if root is primitive, store it as the only entry
	   in database and return. */
	if (SetPrimitiveOrEnqueue(root, &nextLevel) != undecided) {
		return;
	}
	/* The root is not a primitive position. Begin BFS. */
	while (nextLevel) {
		thisLevel = nextLevel;
		nextLevel = NULL;
		for (posptr = thisLevel; posptr; posptr = next) {
			/* Extract the next position in list before we free it. */
			next = posptr->next;
			pos = posptr->position;
			movehead = GenerateMoves(pos);
			for (moveptr = movehead; moveptr; moveptr = moveptr->next) {
				child = DoMove(pos, moveptr->move);
				if (gSymmetries) {
					child = gCanonicalPosition(child);
				}
				if (child >= gNumberOfPositions) {
					FoundBadPosition(child, pos, moveptr->move);
				}
				++numberChildren[pos];
				parentsOf[child] = StorePositionInList(pos, parentsOf[child]);
				if (!Visited(child)) {
					MarkAsVisited(child);
					SetPrimitiveOrEnqueue(child, &nextLevel);
					++gTotalMoves;
				}
			}
			/* Free as we go */
			free(posptr);
			FreeMoveList(movehead);
		}
	}
}

static void SetNewLevelFringe(int level) {
	/* TODO: set up some global list of unprocessed winning/draw-winning positions,
	   loop through that to determine the next level fringe, while
	   checking for impurity. If impure, break loop and return false. */
	POSITIONLIST* ptr = NULL;
	POSITION 	  parent;
	POSITION 	  i;
	for(i = 0; i < gNumberOfPositions; ++i) {
		if ((VALUE)GetSlot(i, SL_VALUE_SLOT) == win) {
			for (ptr = gDrawParents[i]; ptr != NULL; ptr = ptr->next) {
				parent = ptr->position;
				if (gPositionValue[parent] == undecided) {
					gPositionValue[parent] = lose;
					gPositionLevel[parent] = level + 1;
					InsertLoseFR(parent);
				}
			}
		}
	}
}

/* Returns the value of pos, solving all positions reacheable
   from it. */
static VALUE DetermineValueHelper(POSITION pos) {
	POSITION child = kBadPosition;
	POSITION parent;
	POSITIONLIST *ptr;
	VALUE childValue, parentValue;
	REMOTENESS remotenessChild;
	POSITION i;
	int level = 0;

	/* Do BFS to set up parent pointers. */
	SetParents(pos);
	if (kDebugDetermineValue) {
		printf("---------------------------------------------------------------\n");
		printf("Number of Positions = [" POSITION_FORMAT "]\n", gNumberOfPositions);
		printf("---------------------------------------------------------------\n");
	}
	
	/* Now, the fun part. Starting from the children, work your way back up. */
	while (loseFRHead || winFRHead) {
		/* Grab a position from lose queue and think of it as child
		   to process it parents. */
		child = DeQueueLoseFR();
		if (child == kBadPosition) {
			/* If the lose queue is empty, grab one from the win queue. */
			child = DeQueueWinFR();
		}
		childValue = GetSlot(child, SL_VALUE_SLOT); // GetValueOfPosition(child);
		remotenessChild = GetSlot(child, SL_REM_SLOT); // Remoteness(child);

		if(kDebugDetermineValue) {
			/* If debugging, print who's in list */
			printf("Grabbing " POSITION_FORMAT " (%s) remoteness = %d off of FR\n",
			       child,gValueString[childValue],remotenessChild);
		}
		/* TODO: shrink this part: move childValue if statement inside for loop. */
		if (childValue == lose) {
			/* With losing child, every parent is winning, so we just go through
		   	   all the parents and declare them winning. */
			for (ptr = parentsOf[child]; ptr; ptr = ptr->next) {
				parent = ptr->position;
				/* Skip if this is the initial position (parent is kBadPosition). */
				if (parent == kBadPosition) {
					continue;
				}
				parentValue = GetSlot(parent, SL_VALUE_SLOT);
				if (parentValue == undecided) {
					/* This is the first time we know the parent is a win. */
					VSInsertWinFR(parent);
					if(kDebugDetermineValue) {
						printf("Inserting " POSITION_FORMAT " (%s) remoteness = %d "
								"into win FR\n", parent, "win", remotenessChild + 1);
					}
					SetSlot(parent, SL_REM_SLOT, remotenessChild + 1);
					SetSlot(parent, SL_VALUE_SLOT, win);
				} else {
					/* We already know the parent is a winning position. */
					if (parentValue != win) {
						printf(POSITION_FORMAT " should be win.  Instead ""it is %s.",
								parent, gValueString[parentValue]);
						BadElse("DetermineLoopyValue");
					}
					/* This should always hold because the frontier is a queue.
						We always examine losing nodes with less remoteness first. */
					assert((remotenessChild + 1) >= (int)GetSlot(parent, SL_REM_SLOT));
				}
			} /* for each parent. */
		} else if (childValue == win) {
			/* With winning child, we can only eliminate one losing move from its parent. */
			for (ptr = parentsOf[child]; ptr; ptr = ptr->next) {
				parent = ptr->position;
				/* Skip if this is the initial position (parent is kBadPosition). */
				if (parent == kBadPosition) {
					continue;
				}
				/* If this is the last unknown child and they were all wins, parent is lose. */
				if(--numberChildren[parent] == 0) {
					/* No more kids, it's not been seen before, assign it as losing and enqueue. */
					assert((VALUE)GetSlot(parent, SL_VALUE_SLOT) == undecided);
					VSInsertLoseFR(parent);
					if(kDebugDetermineValue) {
						printf("Inserting " POSITION_FORMAT " (%s) into lose FR\n",parent,"lose");
					}
					/* We always need to change the remoteness because we examine winning node with
					   less remoteness first. */
					SetSlot(parent, SL_REM_SLOT, remotenessChild + 1);
					SetSlot(parent, SL_VALUE_SLOT, lose);
				}
			} /* for each parent. */
		} else {
			/* We should not see other values DeQueued from win and lose queues. */
			BadElse("DetermineLoopyValue found FR member with other than win/lose value");
		}
		/* We are done with this position and no longer need to keep around its list of parents.
		   The tie frontier will not need this, either, because this child's value has already
		   been determined. It cannot be a tie. */
		FreePositionList(parentsOf[child]);
		parentsOf[child] = NULL;
	} /* while there are still positions in win/lose FR. */

	/* Now process the tie frontier. */
	while (tieFRHead) {
		child = DeQueueTieFR();
		remotenessChild = GetSlot(child, SL_REM_SLOT);
		for (ptr = parentsOf[child]; ptr; ptr = ptr->next) {
			parent = ptr->position;
			/* Skip if this is the initial position (parent is kBadPosition). */
			if (parent == kBadPosition) {
				continue;
			}
			parentValue = GetSlot(parent, SL_VALUE_SLOT);
			/* If parent is undecided and this is the last unknown child, parent is tie. */
			if (parentValue == undecided && --numberChildren[parent] == 0) {
				/* No more kids, it's not been seen before, assign it as tying and enqueue. */
				InsertTieFR(parent);
				if (kDebugDetermineValue) {
					printf("Inserting " POSITION_FORMAT " (%s) remoteness = "
					        "%d into tie FR\n", parent, "tie", remotenessChild + 1);
				}
				SetSlot(parent, SL_REM_SLOT, remotenessChild + 1);
				SetSlot(parent, SL_VALUE_SLOT, tie);
			}
		}
		FreePositionList(parentsOf[child]);
		parentsOf[child] = NULL;
	} /* while there are still positions in tie FR. */
	
	if (kDebugDetermineValue) {
		printf("---------------------------------------------------------------\n");
		//MyPrintFR();
		printf("---------------------------------------------------------------\n");
		VSMyPrintParents();
		printf("---------------------------------------------------------------\n");
		printf("TIE cleanup\n");
	}

	/* Pure draw analysis. */
	for (i = 0; i < gNumberOfPositions; i++) {
		if(Visited(i)) {
			if(kDebugDetermineValue)
				printf(POSITION_FORMAT " was visited...",i);
			if(GetSlot((POSITION) i, SL_VALUE_SLOT) == undecided) {
				SetSlotMax((POSITION) i, SL_REM_SLOT);
				SetSlot((POSITION) i, SL_VALUE_SLOT, tie);

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
	}
	if (gInterestingness) {
		DetermineInterestingness(pos);
	}
	return GetSlot(pos, SL_VALUE_SLOT);
}

