/************************************************************************
**
** NAME:         gloopy.c
**
** DESCRIPTION:  This file contains the code to discover loopy positions
**
** AUTHOR:       Dan Garcia  -  University of California at Berkeley
**               Copyright (C) Dan Garcia, 1999. All rights reserved.
**
** DATE:         1999-02-23
**
**************************************************************************/

#include <stdio.h>
#include <math.h>
#include "gsolve.h"

extern STRING gValueString[];        /* The GAMESMAN Value strings */
extern int    gNumberOfPositions;    /* The number of positions in the game */
extern POSITION kBadPosition;        /* This can never be the rep. of a pos */
extern BOOLEAN kDebugDetermineValue; /* TRUE iff we should debug DV */
extern char *gNumberChildren;        /* The Number of children (used for Loopy games) */

/* Function prototypes */
BOOLEAN MemberFR(POSITION pos);

/* External function prototypes */
extern BOOLEAN Visited(POSITION position);

/************************************************************************
**
** NAME:        DetermineLoopyValue
**
** DESCRIPTION: Return the value of the input position by running the
**              generalized Sprague-Grundy function on them. Here a tie
**              means an infinite draw!
** 
** INPUTS:      POSITION position : The position to determine the value of.
**
** OUTPUTS:     (VALUE) an enum which is oneof: (win,lose,tie,undecided)
**
** CALLS:       Visited()
**              GetValueOfPosition()
**              Primitive()
**              StoreValueOfPosition()
**              MarkAsVisited()
**              GenerateMoves()
**              DetermineValue()
**              FreeMoveList()
**
************************************************************************/

MyPrintParents()
{
  int i;
  POSITIONLIST *ptr;
  VALUE GetValueOfPosition();
  BOOLEAN Visited();

  printf("PARENTS | #Children | Value\n");
  
  for(i=0 ; i<gNumberOfPositions ; i++)
    if(Visited(i)) {
      ptr = gParents[i];
      printf("%2d: ",i);
      while (ptr != NULL) {
	printf("[%2d] ",ptr->position);
	ptr = ptr->next;
      }
      printf("| %d children | %s value",(int)gNumberChildren[i],gValueString[GetValueOfPosition((POSITION)i)]);
      printf("\n");
    }
}

void MyPrintFR()
{
  BOOLEAN Visited();
  int i;

  printf("HEAD = [%2d]\n",gHeadFR);
  for (i = 0; i < gNumberOfPositions; i++)
    if(Visited(i)) {
      printf("%2d : [%2d] [%2d] [%d]=Member?\n",i,gPrevFR[i],gNextFR[i],MemberFR(i));
    }
  printf("TAIL = [%2d]\n",gTailFR);
}


VALUE DetermineLoopyValue(position)
POSITION position;
{				
  POSITION GetHeadFR(), child, parent;
  POSITIONLIST *head, *ptr;
  VALUE GetValueOfPosition(), childValue, parentValue;
  BOOLEAN MemberFR();
  REMOTENESS remotenessChild, Remoteness();
  int i;

  /* Do DFS to set up Parent pointers and initialize KnownList w/Primitives */

  if(kDebugDetermineValue) printf("\n");
  DFS_SetParents(kBadPosition,position);
  if(kDebugDetermineValue) {
    printf("---------------------------------------------------------------\n");
    printf("Number of Positions = [%d]\n",gNumberOfPositions);
    printf("---------------------------------------------------------------\n");
    MyPrintParents();
    printf("---------------------------------------------------------------\n");
    MyPrintFR();
    printf("---------------------------------------------------------------\n");
  }

  /* Now, the fun part. Starting from the children, work your way back up. */

  while ((child = GetHeadFR()) != kBadPosition) {

    /* Might as well grab these now, they'll be used later */
    childValue = GetValueOfPosition(child);
    remotenessChild = Remoteness(child);

    /* If debugging, print who's in list */
    if(kDebugDetermineValue) printf("Grabbing %d (%s) remoteness = %d off of FR\n",
				    child,gValueString[childValue],remotenessChild);

    /* Remove the child from the FRontier, we're working with it now */
    RemoveFR(child);

    /* With losing children, every parent is winning, so we just go through
    ** all the parents and declare them winning */
    if (childValue == lose) {
      head = ptr = gParents[child];
      while (ptr != NULL) {
	/* Make code easier to read */
	parent = ptr->position;
	/* It's already been visited in this loop before */
	if (MemberFR(parent)) { 
	  /* Set remoteness if you've found a faster mate than I had already */
	  if((remotenessChild + 1) < Remoteness(parent)) {
	    SetRemoteness(parent, remotenessChild + 1);
	    if(kDebugDetermineValue) printf("Found faster mate for %d to %d\n",parent,remotenessChild+1);
	  }
	} else { 
	  /* It's not in the frontier now */
	  if ((parentValue = GetValueOfPosition(parent)) == undecided) {
	    /* Nor has it been seen before, assign it as winning, put it at the end */
	    InsertTailFR(parent);
	    if(kDebugDetermineValue) printf("Inserting %d (%s) remoteness = %d into FR tail\n",parent,"win",remotenessChild+1);
	    StoreValueOfPosition(parent,win); 
	    SetRemoteness(parent, remotenessChild + 1);
	  } else {
	    /* It has been seen before, it's just not on the list anymore */
	    /* Set remoteness if you've found a faster mate than I had already */
	    if((remotenessChild + 1) < Remoteness(parent)) {
	      SetRemoteness(parent, remotenessChild + 1);
	      if(kDebugDetermineValue) printf("Found faster mate for %d to %d\n",parent,remotenessChild+1);
	    }
	  } /* else not undecided */
	} /* else not member */
	ptr = ptr->next;
      } /* while there are still parents */

    /* With winning children */
    } else if (childValue == win) {
      head = ptr = gParents[child];
      while (ptr != NULL) {
	/* Make code easier to read */
	parent = ptr->position;

	/* If this is the last unknown child and they were all wins, parent is lose */
	if(--gNumberChildren[parent] == 0) {
	  if(MemberFR(parent))
	    printf("Error in DetermineLoopyValue : parent shouldn't be in FR already\n");
	  else {
	    /* no more kids, it's not been seen before, assign it as losing, put at head */
	    InsertHeadFR(parent);
	    if(kDebugDetermineValue) printf("Inserting %d (%s) into FR head\n",parent,"lose");
	    StoreValueOfPosition(parent,lose);
	    SetRemoteness(parent, remotenessChild + 1);
	  } /* Not a member of FR, outside */
	} else {
	  /* Still children, not ready for FR, just set remoteness and continue */
	  /* If piece is undecided, that means it might be a lose, have to remember this */
	  /* Set remoteness if I can delay mate longer */
	  if ((parentValue = GetValueOfPosition(parent)) == undecided) {
	    if((remotenessChild + 1) > Remoteness(parent)) {
	      SetRemoteness(parent, remotenessChild + 1);
	      if(kDebugDetermineValue) printf("Found way to extend lose for %d to %d\n",parent,remotenessChild+1);
	    } /* if remoteness change */
	  } /* if parent is undecided, so need to store remoteness */
	} /* else still have children */
	ptr = ptr->next;
      } /* while there are still parents */

    /* With children set to other than win/lose. So stop */
    } else {
      BadElse("DetermineLoopyValue found FR member with other than win/lose value");
    } /* else */
  } /* while GetHeadFR != kBadPosition (while still positions in FR) */

  /* Now set all remaining positions to tie */

  if(kDebugDetermineValue) {
    printf("---------------------------------------------------------------\n");
    MyPrintFR();
    printf("---------------------------------------------------------------\n");
    MyPrintParents();
    printf("---------------------------------------------------------------\n");
    printf("TIE cleanup\n");
  }

  for (i = 0; i < gNumberOfPositions; i++)
    if(Visited(i)) {
      if(kDebugDetermineValue)
	printf("%d was visited...",i);
      if(GetValueOfPosition((POSITION)i) == undecided) {
	StoreValueOfPosition((POSITION)i,tie);
	if(kDebugDetermineValue)
	  printf("and was undecided, setting to tie\n",i);
      } else
	if(kDebugDetermineValue)
	  printf("but was decided, ignoring\n",i);
      UnMarkAsVisited((POSITION)i);
    }

  return(GetValueOfPosition(position));
}

/************************************************************************
**
** NAME:        DFS_SetParents
**
** DESCRIPTION: Do a DFS on the game tree and set the parents of each node.
**              As a side-effect, the global array gParents is modified.
** 
** INPUTS:      POSITION parent   : The parent of position
**              POSITION position : The position to start with.
**
************************************************************************/

DFS_SetParents(parent,position)
POSITION parent,position;
{				
  BOOLEAN Visited();
  MOVELIST *ptr, *head, *GenerateMoves();
  VALUE Primitive(), value;
  POSITION child;
  POSITIONLIST *StorePositionInList();

  if(kDebugDetermineValue) printf("DV (%d,%d)\n", parent,position);
  if(Visited(position)) { /* We've been down this path before, don't DFS */
    if(kDebugDetermineValue) printf("Seen\n");
    /* PARENT me */
    gParents[position] = StorePositionInList(parent, gParents[position]);
    return;
  } else if((value = Primitive(position)) != undecided) { /* Primitive */
    if(kDebugDetermineValue) printf("PRIM value = %s\n", gValueString[value]);
    SetRemoteness(position,0); /* Primitives are leaves, remoteness = 0 */
    MarkAsVisited(position);
    /* PARENT me */
    gParents[position] = StorePositionInList(parent, gParents[position]);
    /* Set the value */
    StoreValueOfPosition(position,Primitive(position));
    /* Add me to FR, if not already */
    if (! MemberFR(position)) {
      if(value == lose)
	InsertHeadFR(position);
      else if(value == win)
	InsertTailFR(position);
      else
	BadElse("DFS_SetParents value");
    }
    return;
  } else { /* first time, need to recursively determine value */
    /* PARENT me */
    gParents[position] = StorePositionInList(parent, gParents[position]);
    if(kDebugDetermineValue) printf("normal, continue searching\n");
    MarkAsVisited(position);
    head = ptr = GenerateMoves(position);
    while (ptr != NULL) {
      gNumberChildren[(int)position]++;    /* Record the number of kids */
      child = DoMove(position,ptr->move);  /* Create the child */
      DFS_SetParents(position,child);      /* DFS call */
      ptr = ptr->next;                     /* Go to the next child */
    }
    FreeMoveList(head);
    /* UnMarkAsVisited(position); */ /* Not unmarking it so we don't loop */
  }
  return;          /* But has been added to satisty lint */
}

/************************************************************************
**
** NAME:        ParentInitialize
**
** DESCRIPTION: Allocate & Reset Parent database to all NULL
** 
************************************************************************/

ParentInitialize()
{
  GENERIC_PTR SafeMalloc();
  int i;

  gParents = (POSITIONLIST **) SafeMalloc (gNumberOfPositions * sizeof(POSITIONLIST *));
  for(i = 0; i < gNumberOfPositions; i++)
    gParents[i] = NULL;
}

/************************************************************************
**
** NAME:        NumberChildrenInitialize
**
** DESCRIPTION: Allocate & Reset Parent database to all NULL
** 
************************************************************************/

NumberChildrenInitialize()
{
  GENERIC_PTR SafeMalloc();
  int i;

  gNumberChildren = (signed char *) SafeMalloc (gNumberOfPositions * sizeof(signed char));
  for(i = 0; i < gNumberOfPositions; i++)
    gNumberChildren[i] = 0;
}

/************************************************************************
**
** NAME:        InitializeFR
**
** DESCRIPTION: Initialize the FRontier, a double-linked position list
**              The FRontier is really a set of known positions with
**              parents we haven't looked at yet. We call
**              it the FRontier because it is the frontier of known 
**              positions. Everything below it is known (that is, every
**              position that once was in the FRontier, the FR alumni)
**              and most of the parents of elements in the FRontier have
**              values yet to be determined.
** 
************************************************************************/

InitializeFR()
{
  int i;
  gHeadFR = kBadPosition;
  gTailFR = kBadPosition;
  gPrevFR = (POSITION *) malloc (sizeof(POSITION) * gNumberOfPositions);
  gNextFR = (POSITION *) malloc (sizeof(POSITION) * gNumberOfPositions);
  for (i = 0; i < gNumberOfPositions; i++) {
    gPrevFR[i] = kBadPosition;
    gNextFR[i] = kBadPosition;
  }
}

/************************************************************************
**
** NAME:        GetHeadFR
**
** DESCRIPTION: Return the Head of the FRontier, kBadPosition if empty.
** 
** OUTPUTS:     POSITION : The Head, kBadPosition if FRontier empty
**
************************************************************************/

POSITION GetHeadFR()
{
  return(gHeadFR);
}

/************************************************************************
**
** NAME:        GetTailFR
**
** DESCRIPTION: Return the Tail of the FRontier, kBadPosition if empty.
** 
** OUTPUTS:     POSITION : The Tail, kBadPosition if FRontier empty
**
************************************************************************/

POSITION GetTailFR()
{
  return(gTailFR);
}

/************************************************************************
**
** NAME:        InsertHeadFR
**
** DESCRIPTION: Insert a position into the head of the FRontier
**              Returns an error if the position is in there already.
** 
** INPUTS:      POSITION pos : Position to insert
**
************************************************************************/

InsertHeadFR(pos)
POSITION pos;
{
  /* Check to make sure it's not a member already! */
  if(MemberFR(pos)) {
    printf("Error: Inserting a position (%d) into FR list that already exists!\n", pos);
    ExitStageRight();
  }		
  /* Otherwise, if the list is empty, insert me */
  else if(gHeadFR == kBadPosition) {
    gHeadFR = pos;
    gTailFR = pos;
  }
  /* Otherwise, set the old first guy's prev to point to me,
  ** set my next pointer to be the old first guy,
  ** and tell the head that I'm the first guy */
  else {
    gPrevFR[gHeadFR] = pos;
    gNextFR[pos]     = gHeadFR;
    gHeadFR          = pos;
  }
}

/************************************************************************
**
** NAME:        InsertTailFR
**
** DESCRIPTION: Insert a position into the tail of the FRontier.
**              Returns an error if the position is in there already.
** 
** INPUTS:      POSITION pos : Position to insert
**
************************************************************************/

InsertTailFR(pos)
POSITION pos;
{
  /* Check to make sure it's not a member already! */
  if(MemberFR(pos)) {
    printf("Error: Inserting a position (%d) into FR list that already exists!\n", pos);
    ExitStageRight();
  }		
  /* Otherwise, if the list is empty, insert me */
  else if(gTailFR == kBadPosition) {
    gHeadFR = pos;
    gTailFR = pos;
  }
  /* Otherwise, set the old last guy's next to point to me,
  ** set my previous pointer to be the old last guy,
  ** and tell the tail that I'm the last guy */
  else {
    gNextFR[gTailFR] = pos;
    gPrevFR[pos]     = gTailFR;
    gTailFR          = pos;
  }
}

/************************************************************************
**
** NAME:        MemberFR
**
** DESCRIPTION: Check if the position (pos) is in the FRontier
** 
** INPUTS:      POSITION pos : Position to check
**
** OUTPUTS:     BOOLEAN : TRUE iff the position is in the FRontier
**
************************************************************************/

BOOLEAN MemberFR(pos)
POSITION pos;
{
  /* If you're in the list, either your prev pointer is non-null,
  ** or you're the head, in which case the head is you */
  return(gPrevFR[pos] != kBadPosition || gHeadFR == pos);
}

/************************************************************************
**
** NAME:        RemoveFR
**
** DESCRIPTION: Remove a position from the FRontier
**              Returns an error if the position isn't there.
** 
** INPUTS:      POSITION pos : Position to remove
**
************************************************************************/

RemoveFR(pos)
POSITION pos;
{
  POSITION newHead, newTail, thePrev, theNext;
  
  /* Check to make sure it's in there! */
  if(! MemberFR(pos)) {
    printf("Error: You want to remove a position (%d) that isn't in FR!\n", pos);
    ExitStageRight();
  }		
  /* Otherwise, if it's the only one, make the list empty */
  else if (gHeadFR == pos && gTailFR == pos) {
    gHeadFR = gTailFR = kBadPosition;
  } 
  /* Otherwise, if it's the first, remove it.
  ** First, set the new head to be the second guy,
  ** Then, set this head's next to null,
  ** Then, set the second guy's prev to null,
  ** Then set the official Head to be the second guy */
  else if (gHeadFR == pos) {
    newHead = gNextFR[gHeadFR];
    gNextFR[pos] = kBadPosition;
    gPrevFR[newHead] = kBadPosition;
    gHeadFR = newHead;
  }
  /* Otherwise, if it's the last, remove it.
  ** First, set the new tail to be the second guy,
  ** Then, set this tail's prev to null,
  ** Then, set the second guy's next to null,
  ** Then set the official Tail to be the second guy */
  else if (gTailFR == pos) {
    newTail = gPrevFR[gTailFR];
    gPrevFR[pos] = kBadPosition;
    gNextFR[newTail] = kBadPosition;
    gTailFR = newTail;
  }
  /* Otherwise, it's in the middle.
  ** First, set find the prev and next positions,
  ** Then, set them to each other,
  ** Finally, set both prev and next pointers pos to NULL */
  else {
    thePrev          = gPrevFR[pos];
    theNext          = gNextFR[pos];
    gNextFR[thePrev] = theNext;
    gPrevFR[theNext] = thePrev;
    gPrevFR[pos]     = kBadPosition;
    gNextFR[pos]     = kBadPosition;
  }
}
