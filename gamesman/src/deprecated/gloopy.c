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
#include <assert.h>
#include "gsolve.h"

extern STRING gValueString[];        /* The GAMESMAN Value strings */
extern int    gNumberOfPositions;    /* The number of positions in the game */
extern POSITION kBadPosition;        /* This can never be the rep. of a pos */
extern BOOLEAN kDebugDetermineValue; /* TRUE iff we should debug DV */
extern char *gNumberChildren;        /* The Number of children (used for Loopy games) */

struct FRnode {
  POSITION pos;
  struct FRnode *next;
};

struct FRnode *gHeadWinFR;
struct FRnode *gTailWinFR;

struct FRnode *gHeadLoseFR;
struct FRnode *gTailLoseFR;

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

/*
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
    } */


VALUE DetermineLoopyValue(position)
POSITION position;
{				
  POSITION DeQueueWinFR(), DeQueueLoseFR(), InsertWinFR(), InsertLoseFR(), child, parent;
  POSITIONLIST *head, *ptr;
  VALUE GetValueOfPosition(), childValue, parentValue;
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
    if(kDebugDetermineValue) printf("Grabbing %d (%s) remoteness = %d off of FR\n",
				    child,gValueString[childValue],remotenessChild);

    /* Remove the child from the FRontier, we're working with it now */
    //RemoveFR(child);
    //@@don't need this anymore, frontiers are now stacks

    
    /* With losing children, every parent is winning, so we just go through
    ** all the parents and declare them winning */
    if (childValue == lose) {
      head = ptr = gParents[child];
      while (ptr != NULL) {
	/* Make code easier to read */
	parent = ptr->position;
	
	if ((parentValue = GetValueOfPosition(parent)) == undecided) {
	  /* This is the first time we know the parent is a win */
	  InsertWinFR(parent);
	  if(kDebugDetermineValue) printf("Inserting %d (%s) remoteness = %d into win FR\n",parent,"win",remotenessChild+1);
	  StoreValueOfPosition(parent,win); 
	  SetRemoteness(parent, remotenessChild + 1);
	} 
	else {
	  /* We already know the parent is a winning position. */

	  if (GetValueOfPosition(parent) != win)
	    BadElse("&d should be win.  Instead it is %d.", parent, GetValueOfPosition(parent));

	  /* This should always hold because the frontier is a queue.
	  ** We always examine losing nodes with less remoteness first */
	  assert((remotenessChild + 1) >= Remoteness(parent));
	}
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
	    /* no more kids, it's not been seen before, assign it as losing, put at head */
	  InsertLoseFR(parent);
	  if(kDebugDetermineValue) printf("Inserting %d (%s) into FR head\n",parent,"lose");
	  StoreValueOfPosition(parent,lose);
	  if (remotenessChild + 1 > Remoteness(parent))
	    SetRemoteness(parent, remotenessChild + 1);
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

    //we are done with this position and no longer need to keep around its list of parents
    FreePositionList(gParents[child]);

    
  } /* while still positions in FR */

  /* Now set all remaining positions to tie */

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
	printf("%d was visited...",i);
      if(GetValueOfPosition((POSITION)i) == undecided) {
	StoreValueOfPosition((POSITION)i,tie);
	//we are done with this position and no longer need to keep around its list of parents
	FreePositionList(gParents[child]);
	if(kDebugDetermineValue)
	  printf("and was undecided, setting to tie\n",i);
      } else
	if(kDebugDetermineValue)
	  printf("but was decided, ignoring\n",i);
      UnMarkAsVisited((POSITION)i);
    }

  free(gParents);
  
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
    /* Add me to FR. (I know i'm not already in the frontier because 
     * this is the first time i've been visited) */
    if(value == lose)
      InsertLoseFR(position);
    else if(value == win)
      InsertWinFR(position);
    else
      BadElse("DFS_SetParents value");
        /* Set the value */
    StoreValueOfPosition(position,value);
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
  gHeadWinFR = NULL;
  gTailWinFR = NULL;
  gHeadLoseFR = NULL;
  gTailLoseFR = NULL;
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

POSITION DeQueueWinFR()
{
  printf("DeQueueWinFR...\n");
  DeQueueFR(&gHeadWinFR, &gTailWinFR);
}

POSITION DeQueueLoseFR()
{
  printf("DeQueueLoseFR...\n");
  DeQueueFR(&gHeadLoseFR, &gTailLoseFR);
}

POSITION DeQueueFR(struct FRnode **gHeadFR, struct FRnode **gTailFR)
{
  POSITION position;
  struct FRnode *tmp;
  
  if (*gHeadFR == NULL)
    return kBadPosition;
  else {
    position = (*gHeadFR)->pos;
    tmp = *gHeadFR;
    (*gHeadFR) = (*gHeadFR)->next;
    free(tmp);

    if (*gHeadFR == NULL)
      *gTailFR = NULL;
  }
  return position;
}

InsertWinFR(POSITION position)
{
  printf("Inserting WinFR...\n");
  InsertFR(position, &gHeadWinFR, &gTailWinFR);
}


InsertLoseFR(POSITION position)
{
  printf("Inserting LoseFR...\n");
  InsertFR(position, &gHeadLoseFR, &gTailLoseFR);
}

struct FRnode* InsertFR(POSITION position, struct FRnode **firstnode,
			struct FRnode **lastnode)
{
  struct FRnode *tmp = (struct FRnode *) SafeMalloc(sizeof(struct FRnode));
  tmp->pos = position;
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
