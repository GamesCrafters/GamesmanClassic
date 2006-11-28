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
**              11/10/06- (Michael) Implemented skeleton of data structures
**                        and core functions.
**              
**************************************************************************/
#include <stdio.h>
#include <string.h>
#include <math.h>
#include gamesman.h

/*lBoard will contain board processing data. Requires call
  of LibInitialize by the module. For information see mlib.c and mlib.h.*/
#include mlib.h



#define compare(slot,piece) !memcmp(lBoard+slot*lBoard.eltSize,piece,lBoard.eltSize)
#define getSlot(row,col) (row*lBoard.cols+cols)
#define getRow(slot) (slot/lBoard.numCols)
#define getCol(slot) (slot%lBoard.numRows)
#define min(a,b) (a<=b ? a : b)

#define NUMTRAITS 4
#define ERROR -2

int fuck = 1;

int AdditionalParamList[NUMTRAITS] = {1, 2, 1, 3};
char* TraitNames[NUMTRAITS] = {"Number of Pieces","Localization","Clustering","Connections"};

/**************************************************************************
**
** Core functions
**              
**************************************************************************/

void SEvalInitialize() {
  
}

//Applies the given static evaluator instance to the given board
int Evaluate(EVALUATOR e, void* board) {
  double sum=0.0;
  double raw, scaled;
  int i,a,b,weightSum=0;
  TRAITLIST traits = e->myTraits;
  TRAIT current;

  if (e->numTraits==0) {
    printf("ERROR, static evaluator has no defined traits");
    return ERROR;
  }
  
  for(i=0;i< e->numTraits ,i++) {
    current = traits;

    //This part should be implemented later with function pointers
    switch(current->id) {
    case 0:
      raw = (double) numPieces(board,(current->additionalParams)[0]);
      break;
    case 1:
      raw = (double) numFromEdge(board,(current->additionalParams)[0],(current->additionalParams)[1]);
      break;
    case 2:
      raw = clustering(board,(current->additionalParams)[0]);
      break;
    case 3:
      raw = connections(board,(current->additionalParams)[0],(current->additionalParams)[1],(current->additionalParams)[2]);
      break;
    default:
      printf("ERROR, unrecognized trait");
      return ERROR;
    }

    switch(current->scaleType) {
    case Linear:
      a = current->scaleParams[0];
      b = current->scaleParams[1];

      if (raw < current->scaleParams[0]) {
	scaled = -1;
      } else if (raw > current->scaleParams[1]) {
	scaled = 1;
      } else {
	scaled = (2 / (b-a)) * (raw - a);
      }
	
      break;
    case Logistic:
      shift = current->scaleParams[0];
      scale = current->scaleParams[1];

      scaled = (1 / (1 + Math.exp(-scale(raw - shift) ) ) );
      
      break;
    default:
      printf("ERROR, unrecognized scaling type");
    }
    
    weightSum += current->weight;
    sum += scaled * current->weight;
    current = current->next;
  }

  sum /= weightSum;
  
  return sum;
}

void 


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
int NumPieces(void* board,void* piece) {
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
int NumFromEdge(void* board,void* piece,int edge) {
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
double Clustering(void* board,void* piece) {
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
int Connections(void* board,void* piece,void* blank,boolean diagonal) {
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

	  
	  
	  
   
