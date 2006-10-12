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
**              10/8/06- (Michael) Implemented 4 trait functions.
**              
**************************************************************************/
#include <stdio.h>
#include gamesman.h
#include string.h

/*lBoard will contain board processing data. Requires call
  of LibInitialize by the module. For information see mlib.c and mlib.h.*/
#include mlib.h



#define compare(slot,piece) !memcmp(lBoard+slot*lBoard.eltSize,piece,lBoard.eltSize)
#define getSlot(row,col) (row*lBoard.cols+cols)
#define getRow(slot) (slot/lBoard.numCols)
#define getCol(slot) (slot%lBoard.numRows)
#define min(a,b) (a<=b ? a : b)

/************************************************************************
**
** Implementation of traits starts here.
**
** Current list:
**
** -Number of pieces (X)
** -Connections (how many pieces "see" each other)
** -NinaRow, AlmostNinaRow.
** -Side locality. (X)
** -Grouping, clustering. (X)
** -Weighting of pieces for above traits.
** -Piece centric traits- distance to piece, connections to piece
** -Slot centric traits- distance, connections, number on slots.
** -Remaining moves (requires module info)
** 
** Possible optimization: Store a list of piece locations for use by
** each instance of static evaluation. There is a lot of redundant 
** comparison to determine piece locations on the board.
**
************************************************************************/

//Returns the number of the given piece on the board.
int numPieces(void* board,void* piece) {
  int count=0;i;
  for(i=0;i<lBoard.size;i++) {
    if (compare(i,piece)) {
      count++;
    }
  }
  return count;
}

/*Returns the sum of the distance of each piece from the given edge of
**the board. 0 = top, 1 = right, 2 = bottom, 3 = left.
*/
int numFromEdge(void* board,void* piece,int edge) {
  int count=0,i;
  for(i=0;i<lBoard.size;i++) {
    if(compare(i,piece)) {
      if (edge==0) {
	count+= (i/lBoard.cols);
      } else if (edge==1) {
	count+= (lBoard.cols-(i%lBoard.cols));
      } else if (edge==2) {
	connt+= (lBoard.rows-(i/lBoard.cols));
      } else if (edge==3) {
	count+= (i%lBoard.cols);
      } else {
	printf("Error, unknown edge input");
      }
    }
  }
  return count;
}

/*Returns a measurement of the clustering of pieces on the board. First
**calculates the centroid, then measures the mean squared distance of all
**pieces from the centroid.
*/
double clustering(void* board,void* piece) {
  int rowAverage=0,colAverage=0,numPieces=0,i=0;
  double value=0;
  for(i=0;i<lBoard.size;i++) {
    if(compare(i,piece)) {
      rowAverage+=getRow(i);
      colAverage+=getCol(i);
      numPieces++;
    }
  }
  //compute actual averages
  rowAverage = rowAverage/numPieces;
  colAverage = colAverage/numPieces;
  
  for(i=0;i<lBoard.size;i++) {
    if(compare(i,piece)) {
      value = (getRow(i)-rowAverage)*(getRow(i)-rowAverage) + (getCol(i)-colAverage)*(getCol(i)-colAverage);
    }
  }
  value /= numPieces;
  return value;
}

/*Returns the number of connections between pieces on the board.
**A connection is defined as a string of blank spaces, possibly
**diagonal, between two of the given piece.
*/
int connections(void* board,void* piece,void* blank,boolean diagonal) {
  int count=0,i,j,pathLength,loc;
  for(i=0;i<lBoard.size;i++) {
    if (compare(i,piece)) {
      for(direction = 3;direction<7; direction+= (lBoard.diagonals ? 1 : 2)) {	

	//compute distance to edge of board along direction
	if (direction==3) {
	  pathLength = lBoard.cols-1-getCol(i);
	} else if (direction==4) {
	  pathLength = min(lBoard.cols-1-getCol(i),lBoard.rows-1-getRow(i));
	} else if (direction==5) {
	  pathLength = lBoard.rows-1-getRow(i);
	} else { //direction==6
	  pathLength = min(lBoard.rows-1-getRow(i),getCol(i));
	}
	
	loc =i;
	for(j=0;j<pathLength;j++) {
	  loc+=lBoard.directionMap(direction);
	  if (compare(loc,piece)) {
	    count++;
	    break;
	  } else if (!compare(loc,blank)) {
	    break;
	  }
	}
	
      }
    }
  }
  return count;
}

	  
	  
	  
   
