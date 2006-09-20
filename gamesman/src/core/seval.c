 /************************************************************************
**
** NAME:        seval.c
**
** DESCRIPTION: Static evaluator to be used with gamesman modules.
**
** AUTHOR:      Michael Greenbaum
**              Brian Zimmer
**
** DATE:        Start: 19 September 2006
**
** UPDATE HIST: 
**              9/19/06- Initial draft of seval.
**              
**************************************************************************/
#include <stdio.h>
#include gamesman.h
#include string.h

/*lBoard will contain board processing data. Requires call
  of LibInitialize by the module. For information see mlib.c and mlib.h.*/
#include mlib.h

/************************************************************************
**
** Implementation of traits starts here.
**
** Current list:
**
** -Number of pieces
** -Connections (how many pieces "see" each other)
** -NinaRow, AlmostNinaRow.
** -Side locality.
** -Grouping, clustering.
** -Weighting of pieces for above traits.
** -Piece centric traits- distance to piece, connections to piece
** -Slot centric traits- distance, connections, number on slots.
** -Remaining moves (requires module info)
** 
************************************************************************/
