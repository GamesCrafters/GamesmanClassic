/************************************************************************
**
** NAME:	solveloopypd.c
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

#define LPDS_DEBUG TRUE

/*
** Globals
*/

/* FRontier Win Queue */
static FRnode *winFRHead = NULL;       
static FRnode *winFRTail = NULL;

/* FRontier Lose Queue */
static FRnode *loseFRHead = NULL;
static FRnode *loseFRTail = NULL;

/* FRontier Tie Queue */
static FRnode *tieFRHead = NULL;       
static FRnode *tieFRTail = NULL;

/* Unanalyzed win positions list. */
static FRnode *unanalyzedWinList = NULL;

/* Linked lists of parents of each node. */
static POSITIONLIST **parentsOf = NULL;

/* Number of children left undecided. */   
static char* numberChildren = NULL;  

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

static void     FreeFRs    			(void);
static void 	InsertWinFR			(POSITION position);
static void 	InsertLoseFR		(POSITION position);
static void 	InsertTieFR			(POSITION position);
static void 	InsertUnanalyzedWin	(POSITION position);
static POSITION DeQueueWinFR		(void);
static POSITION DeQueueLoseFR		(void);
static POSITION DeQueueTieFR		(void);
static POSITION DeQueueUnanalyzedWin(void);

static void		InitializeParents       (void);
static void		FreeParents             (void);
static void     InitializeNumberChildren(void);
static void     FreeNumberChildren      (void);

static VALUE 	GetValueFromBPDB	(POSITION pos);
static void 	SetValueInBPDB		(POSITION pos, VALUE val);

static void		SetParents          (POSITION root);
static VALUE	DetermineValueHelper(POSITION pos);

static BOOLEAN SanityCheckDatabase(void);

/*
** Code
*/

/* Prints parents of all Visited positions in the game tree. */
void lpds_PrintParents() {
	POSITION i;
	POSITIONLIST *ptr;

	printf("PARENTS | #Children | Value\n");
	for (i = 0; i < gNumberOfPositions; ++i) {
		if (GetSlot(i, SL_VISITED_SLOT)) {
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

BOOLEAN initializePureDrawAnalysisDB(BOOLEAN allocate) {
	GMSTATUS status = STATUS_SUCCESS;

    /* This solver must be used with Bit-Perfect Database. */
	if (!gBitPerfectDB) {
		status = STATUS_MISSING_DEPENDENT_MODULE;
		BPDB_TRACE("lpds_DetermineValue()", "Bit-Perfect DB must be "
            "the selected DB to use the slices solver", status);
		return FALSE;
	}

    /* Add slots to database slices. */
    /* Format: AddSlot(size,  name,       write,  adjust, reservemax, slotindex */
	status =   AddSlot(3,     "VALUE",    TRUE,   FALSE,  FALSE,      &SL_VALUE_SLOT);
	if (!GMSUCCESS(status)) {
		BPDB_TRACE("lpds_DetermineValue()", "Could not add value slot", status);
		return FALSE;
	}
	if (gPutWinBy) {
		status = AddSlot(3, "WINBY", TRUE, TRUE, FALSE, &SL_WINBY_SLOT);
		if(!GMSUCCESS(status)) {
			BPDB_TRACE("lpds_DetermineValue()", "Could not add winby slot", status);
		    return FALSE;
		}
	}
	status = AddSlot(5, "REMOTENESS", TRUE, TRUE, TRUE, &SL_REM_SLOT);
	if (!GMSUCCESS(status)) {
		BPDB_TRACE("lpds_DetermineValue()", "Could not add remoteness slot", status);
		return FALSE;
	}
    status = AddSlot(2, "DRAWLEVEL", TRUE, TRUE, TRUE, &SL_DRAW_LEVEL_SLOT);
	if (!GMSUCCESS(status)) {
		BPDB_TRACE("lpds_DetermineValue()", "Could not add draw level slot", status);
		return FALSE;
	}

	status = AddSlot(1, "VISITED", FALSE, FALSE, FALSE, &SL_VISITED_SLOT);
	if (!GMSUCCESS(status)) {
		BPDB_TRACE("lpds_DetermineValue()", "Could not add visited slot", status);
		return FALSE;
	}

    /* Allocate database. */
	if (allocate) {
		status = Allocate();
		if(!GMSUCCESS(status)) {
			BPDB_TRACE("lpds_DetermineValue()", "Could not allocate database", status);
			return FALSE;
		}
	}
}

/* Returns the VALUE of the given POSITION. */
VALUE lpds_DetermineValue(POSITION position) {
	(void) position;

	VALUE value = undecided;
	if (!initializePureDrawAnalysisDB(TRUE)) {
		return value;
	}

	/* Only initialize global arrays if database was successfully allocated. */
	InitializeParents();
	InitializeNumberChildren();

    /* Solve from initial position. */
	value = DetermineValueHelper(gInitialPosition);

	/* Free global arrays. */
    FreeFRs();
	FreeNumberChildren();
	FreeParents();

	/* Debug */
	if (LPDS_DEBUG) {
		POSITION i;
		int stat[7] = {0};
		for (i = 0; i < gNumberOfPositions; ++i) {
			if (GetSlot(i, SL_VISITED_SLOT)) {
				++stat[GetValueFromBPDB(i)];
			}
		}
		int total = 0;
		for (i = 0; i < 7; ++i) {
			total += stat[i];
		}
		printf("\n\nLoopy solver with Pure Draw Analysis stats:\n"
				"\tund\twin\tlose\ttie\tdw\tdl\tdt\ttot\n"
				"---------------------------------------------------------------------------------\n"
				"\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n\n",
				stat[0], stat[1], stat[2], stat[3], stat[4], stat[5], stat[6], total);
		if (SanityCheckDatabase()) {
			printf("SanityCheckDatabase passed!.\n");
		}
	}

	return value;
}

/*
** Helper functions
*/

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

static POSITION DeQueueUnanalyzedWin(void) {
	POSITIONLIST *oldHead = unanalyzedWinList;
	POSITION pos = oldHead->position;
	unanalyzedWinList = oldHead->next;
	free(oldHead);
	return pos;
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

static void InsertUnanalyzedWin(POSITION position) {
	unanalyzedWinList = StorePositionInList(position, unanalyzedWinList);
}

static void FreeFRs(void) {
	FreePositionList(winFRHead);
	FreePositionList(loseFRHead);
	FreePositionList(tieFRHead);
	winFRHead = winFRTail = NULL;
	loseFRHead = loseFRTail = NULL;
	tieFRHead = tieFRTail = NULL;
}

static void InitializeParents(void) {
	parentsOf = (POSITIONLIST **)SafeCalloc(gNumberOfPositions, sizeof(POSITIONLIST *));
}

static void FreeParents(void) {
    POSITION i;

	for (i = 0; i < gNumberOfPositions; ++i) {
		FreePositionList(parentsOf[i]);
	}
	SafeFreeAndSetToNull((GENERIC_PTR *)&parentsOf);
}

static void InitializeNumberChildren(void) {
	numberChildren = (char *)SafeCalloc(gNumberOfPositions, sizeof(signed char));
	if (gInterestingness) {
		gAnalysis.Interestingness = (float *)SafeCalloc(gNumberOfPositions, sizeof(float));
	}
}

static void FreeNumberChildren(void) {
    SafeFreeAndSetToNull((GENERIC_PTR *)&numberChildren);
}

static VALUE GetValueFromBPDB(POSITION pos) {
	return GetSlot(pos, SL_VALUE_SLOT);
}

static void SetValueInBPDB(POSITION pos, VALUE val) {
	SetSlot(pos, SL_VALUE_SLOT, val);
}

static VALUE SetPrimitiveOrEnqueue(POSITION pos, POSITIONLIST **nextLevel) {
	VALUE value = Primitive(pos);

	if (value != undecided) {
		SetRemoteness(pos, 0);
		switch (value) {
		case lose: 
			InsertLoseFR(pos);
			break;

		case win:  
			InsertWinFR(pos);
			InsertUnanalyzedWin(pos);
			break;

		case tie:
			InsertTieFR(pos);
			break;

		default:
			BadElse("SetParents found bad primitive value");
		}
		SetValueInBPDB(pos, value);
		SetSlotMax(pos, SL_DRAW_LEVEL_SLOT);
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
	SetSlot(root, SL_VISITED_SLOT, TRUE);
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
				if (!GetSlot(child, SL_VISITED_SLOT)) {
					SetSlot(child, SL_VISITED_SLOT, TRUE);
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

static void ProcessWinLose(VALUE valForWin, VALUE valForLose, int level) {
	POSITION child, parent;
	VALUE childValue, parentValue;
	POSITIONLIST *ptr;
	REMOTENESS childRemoteness;

	assert(valForLose == lose && valForWin == win && level == -1 ||
			 valForLose == drawlose && valForWin == drawwin);

	while (loseFRHead || winFRHead) {
		/* Grab a position from lose queue and use it as child
		   to process its parents. */
		child = DeQueueLoseFR();
		if (child == kBadPosition) {
			/* If the lose queue is empty, grab one from the win queue.
			   Note that the other queue must not be empty, otherwise
			   we wouldn't enter this while loop. */
			child = DeQueueWinFR();
		}
		childValue = GetValueFromBPDB(child);
		childRemoteness = GetSlot(child, SL_REM_SLOT);

		for (ptr = parentsOf[child]; ptr; ptr = ptr->next) {
			parent = ptr->position;
			if (parent != kBadPosition) {
				if (childValue == valForLose) {
					/* With losing child, every parent is winning, so we just go through
					all the parents and declare them winning. */
					parentValue = GetValueFromBPDB(parent);
					if (parentValue == undecided) {
						/* This is the first time we know the parent is a win. */
						InsertWinFR(parent);
						InsertUnanalyzedWin(parent);
						SetSlot(parent, SL_REM_SLOT, childRemoteness + 1);
						SetValueInBPDB(parent, valForWin);
						if (valForWin == win) {
							SetSlotMax(parent, SL_DRAW_LEVEL_SLOT);
						} else {
							SetSlot(parent, SL_DRAW_LEVEL_SLOT, level);
						}
					} else {
						/* We already know the value for parent, which can only be winning
						or draw-losing. Otherwise there is a bug. */
						if (parentValue == drawlose || parentValue == drawdraw) {
							/* There is a pure draw violation:
							a draw-lose has another draw-lose as its parent.
							Therefore, the child position is part of a nonpure draw cluster.
							Thus, insert it into the tie frontier */
							InsertTieFR(child);
							SetValueInBPDB(child, drawdraw);
							SetSlotMax(child, SL_REM_SLOT);
							SetSlotMax(child, SL_DRAW_LEVEL_SLOT);
							break;
						} else if (parentValue != win && parentValue != drawwin) {
							printf("PWL: %d\n", parentValue);
							BadElse("ProcessWinLose");
						}
					}
				} else if (childValue == valForWin) {
					/* With winning child, we can only eliminate one losing move from its parent.
					If this is the last unknown child and they were all wins, parent is lose. */
					parentValue = GetValueFromBPDB(parent);
					if (parentValue == undecided && --numberChildren[parent] == 0) {
						/* No more kids, it's not been seen before, assign it as losing and enqueue. */
						InsertLoseFR(parent);
						/* We always need to change the remoteness because we examine winning node with
						less remoteness first. */
						SetSlot(parent, SL_REM_SLOT, childRemoteness + 1);
						if (valForWin == win) {
							SetSlotMax(parent, SL_DRAW_LEVEL_SLOT);
						} else {
							SetSlot(parent, SL_DRAW_LEVEL_SLOT, level);
						}
						SetValueInBPDB(parent, valForLose);
					}
				} else {
					/* We should not see other values DeQueued from win and lose queues. */
					printf("PWL2: %d\n", childValue);
					BadElse("ProcessWinLose2");
				}
			}
		}
	} /* while there are still positions in win/lose FR. */
}

static void ProcessTie() {
	POSITION child, parent;
	VALUE parentValue;
	POSITIONLIST *ptr;
	REMOTENESS childRemoteness;

	while (tieFRHead) {
		child = DeQueueTieFR();
		childRemoteness = GetSlot(child, SL_REM_SLOT);
		for (ptr = parentsOf[child]; ptr; ptr = ptr->next) {
			parent = ptr->position;
			/* Skip if this is the initial position (parent is kBadPosition). */
			if (parent == kBadPosition) {
				parentValue = GetValueFromBPDB(parent);
				/* If parent is undecided and this is the last unknown child, parent is tie. */
				if (parentValue == undecided && --numberChildren[parent] == 0) {
					/* No more kids and parent has not been seen before,
					assign it as tying and enqueue. */
					InsertTieFR(parent);
					SetSlot(parent, SL_REM_SLOT, childRemoteness + 1);
					SetValueInBPDB(parent, tie);
				}
			}
		}
		/* We won't need to visit the parents of a tying position again. */
		FreePositionList(parentsOf[child]);
		parentsOf[child] = NULL;
	} /* while there are still positions in tie FR. */
}

/* All draw ancestors of a nonpure draw position such that there exists a path
from the draw ancestor to the nonpure draw position consisting of only draw moves
are also nonpure draws.
Mark all nonpure draw positions as drawdraws. The Tie frontier is repurposed
to contain nonpure draw positions. */
static BOOLEAN ProcessDrawDraws() {
	POSITION child, parent;
	VALUE parentValue;
	POSITIONLIST *ptr;

	BOOLEAN nonpureDrawsExist = tieFRHead != NULL;
	while (tieFRHead) {
		child = DeQueueTieFR();
		for (ptr = parentsOf[child]; ptr; ptr = ptr->next) {
			parent = ptr->position;
			/* Skip if this is the initial position (parent is kBadPosition). */
			if (parent != kBadPosition) {
				parentValue = GetValueFromBPDB(parent);
				if (parentValue == drawwin || parentValue == drawlose || parentValue == undecided) {
					InsertTieFR(parent);
					SetValueInBPDB(parent, drawdraw);
					SetSlotMax(parent, SL_REM_SLOT);
					SetSlotMax(parent, SL_DRAW_LEVEL_SLOT);
				}
			}
		}
		/* We won't need to visit the parents of a draw-draw position again. */
		FreePositionList(parentsOf[child]);
		parentsOf[child] = NULL;
	} /* while there are still positions in tie FR. */
	return nonpureDrawsExist;
}

/* Draw analysis: find all draw-lose positions of level LEVEL
   by marking all visited but undecided positions as draw-loses.
   Note that this function does not check for impurity. The
   check is handled by ProcessWinLose() which terminates and
   returns FALSE if a draw-losing positions is found to have a
   parent that is also a draw-lose.
   The unanalyzedWinList is guaranteed to be empty after this
   function call. */
static void ProcessLevelFringe(int level) {
	POSITION child, parent;
	VALUE parentValue;

	while (unanalyzedWinList) {
		child = DeQueueUnanalyzedWin();
		while (parentsOf[child]) {
			parent = RemovePositionFromQueue(&parentsOf[child]);
			if (parent != kBadPosition) {
				parentValue = GetValueFromBPDB(parent);
				if (parentValue == undecided) {
					SetValueInBPDB(parent, drawlose);
					SetSlot(parent, SL_REM_SLOT, 0);
					SetSlot(parent, SL_DRAW_LEVEL_SLOT, level);
					InsertLoseFR(parent);
				}
			}
		}
	}
}

static BOOLEAN CheckExistenceOfPureDrawClusters() {
	VALUE v;
	for (POSITION p = 0; p < gNumberOfPositions; p++) {
		v = GetValueFromBPDB(p);
		if (v == drawlose || v == drawwin) {
			return TRUE;
		}
	}
	return FALSE;
}

/* Returns the value of pos, solving all positions reacheable
   from it. */
static VALUE DetermineValueHelper(POSITION pos) {

	/* Do BFS to set up parent pointers. */
	SetParents(pos);

	/* Now, the fun part. Starting from the children, work your way back up. */
	ProcessWinLose(win, lose, -1);
	printf("Finished processing win/lose frontier.\n");

	/* Process the tie frontier. */
	ProcessTie();
	printf("Finished processing tie frontier.\n");

	/* Determine all draw-win and draw-lose positions. */
	int level = 0;
	while (unanalyzedWinList) {
		ProcessLevelFringe(level);
		
		/* In a similar way, work your way back up from draw-lose primitives. */
		ProcessWinLose(drawwin, drawlose, level);
		++level;
	}
	BOOLEAN nonpureDrawsExist = ProcessDrawDraws();
	BOOLEAN pureDrawsExist = CheckExistenceOfPureDrawClusters();

	printf("Pure Draw Analysis: ");
	printf("Nonpure draw clusters%s exist. ", nonpureDrawsExist ? "" : " do not");
	printf("Pure draw clusters%s exist.\n", pureDrawsExist ? "": " do not");

	return GetValueFromBPDB(pos);
}

static BOOLEAN OnlyHasChildrenOf(POSITION parent, int allowed[static 7]) {
	MOVELIST *moves = GenerateMoves(parent);
	MOVELIST *walker;
	BOOLEAN valid = TRUE;

	for (walker = moves; walker != NULL; walker = walker->next) {
		if (!allowed[GetValueFromBPDB(DoMove(parent, walker->move))]) {
			valid = FALSE;
			break;
		}
	}
	FreeMoveList(moves);
	return valid;
}

static BOOLEAN HasChild(POSITION parent, VALUE childVal) {
	MOVELIST *moves = GenerateMoves(parent);
	MOVELIST *walker;
	BOOLEAN found = FALSE;

	for (walker = moves; walker != NULL; walker = walker->next) {
		if (GetValueFromBPDB(DoMove(parent, walker->move)) == childVal) {
			found = TRUE;
			break;
		}
	}
	FreeMoveList(moves);
	return found;
}

static BOOLEAN SanityCheckDatabase(void) {
	POSITION i;
	VALUE v;
	BOOLEAN valid = TRUE;

	for (i = 0; valid && i < gNumberOfPositions; ++i) {
		v = GetValueFromBPDB(i);
		switch (v) {
		case undecided:
			break;
			
		case win:
			valid = Primitive(i) == win || HasChild(i, lose);
			break;
			
		case lose:
			valid = Primitive(i) == lose || OnlyHasChildrenOf(i, (int[7]){0,1,0,0,0,0,0});
			break;
			
		case tie:
			valid = Primitive(i) == tie || (OnlyHasChildrenOf(i, (int[7]){0,1,0,1,0,0,0}) && HasChild(i, tie));
			break;
			
		case drawwin:
			valid = OnlyHasChildrenOf(i, (int[7]){0,1,0,1,1,1,1}) && HasChild(i, drawlose);
			break;
			
		case drawlose:
			valid = OnlyHasChildrenOf(i, (int[7]){0,1,0,1,1,0,0}) && HasChild(i, drawwin);
			break;
			
		case drawdraw:
			valid = OnlyHasChildrenOf(i, (int[7]){0,1,0,1,1,0,1}) && HasChild(i, drawdraw);
			break;
		
		default:
			valid = FALSE;
			break;
		}
	}

	int totalrems = 100;
	int totaldls = 100;
	POSITION drawLevelCount[totalrems * totaldls];
	memset(drawLevelCount, 0, sizeof(POSITION) * totalrems * totaldls);
	for (POSITION po = 0; po < gNumberOfPositions; po++) {
		VALUE va = GetValueFromBPDB(po);
		if (va != undecided) {
			REMOTENESS rem = (REMOTENESS) GetSlot(po, SL_REM_SLOT);
			DRAWLEVEL dl = (DRAWLEVEL) GetSlot(po, SL_DRAW_LEVEL_SLOT);
			// PrintPosition(po, "", FALSE);
			// printf("%llu %s Remoteness: %d; Level: %d\n", po, gValueString[va], rem, dl);
			drawLevelCount[dl * totalrems + rem]++;
		}
	}
	for (int i = 0; i < totaldls; i++) {
		for (int j = 0; j < totalrems; j++) {
			if (drawLevelCount[i * totalrems + j] > 0) {
				printf("DrawLevel: %d | Remoteness: %d | Count %llu\n", i, j, drawLevelCount[i * totalrems + j]);
			}
		}
	}
	if (!valid) {
		printf("Invalid position value at %llu. It is value %d\n", i - 1, v);
	}
	return valid;
}
