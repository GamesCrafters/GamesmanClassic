 /************************************************************************
**
** NAME:        mlib.c
**
** DESCRIPTION: Library of functions for game modules.
**
** AUTHOR:      Michael Greenbaum
**
** DATE:        Start: 5 March 2006
**
** UPDATE HIST: 
**              3/5/06 - Created basic framework, draft of NinaRow
**              3/11/06 - Non-crashing BlankOX dependant NinaRow, modifying
**                        to use void*
**              3/13/06 - Writing statelessNinaRow. Possible lastIndex
**                        NinaRow bug found.
**              3/14/06 - statelessNinaRow seems to work, but analysis
**                        results are weird.
**              3/18/06 - statelessNinaRow confirmed to work with mttt and
**                       mwin4.
**              3/19/06 - wrote amountOfWhat. Not thoroughly tested.
**              4/04/06 - Functionality of stateful NinaRow confirmed.
**              4/29/06 - Added OneDMatch, no support for transparencies.
**              4/30/06 - OneDMatch confirmed to produce equivolent
**                        database for 4x4 Toot & Otto. Attempt at creating
**                        preprocessing to avoid board "overflow."
**                        Currently implemented in OneDMatching, will be
**                        soon in stateless Ninarow.
**************************************************************************/
#include <stdio.h>
#include "gamesman.h"
#include "mlib.h"
#include "string.h"

#define MAXBOARDSIZE 64
#define MAXPATTERN 16

void overflowGuard(int);

struct {
  int count;
  int eltSize;
  int rows;
  int cols;
  BOOLEAN diagonals;
  int directionMap[8];
  int scratchBoard[MAXBOARDSIZE];
  int overflowBoards[MAXPATTERN][MAXBOARDSIZE];
} lBoard;

int powersOf2[] = {1,2,4,8};

/*Set local variables for processing input arrays*/
void LibInitialize(int eltSize, int rows, int cols, BOOLEAN diagonals) {
  int i;
  lBoard.count = 0;
  lBoard.eltSize = eltSize;
  lBoard.rows = rows;
  lBoard.cols = cols;
  lBoard.diagonals = diagonals;

  lBoard.directionMap[0] = -cols - 1; //used in NinaRow
  lBoard.directionMap[1] = -cols;
  lBoard.directionMap[2] = -cols + 1;
  lBoard.directionMap[3] = 1;
  lBoard.directionMap[4] = cols + 1;
  lBoard.directionMap[5] = cols;
  lBoard.directionMap[6] = cols - 1;
  lBoard.directionMap[7] = -1;
  
  for(i=0;i<MAXBOARDSIZE;i++) {
    lBoard.scratchBoard[i] = 0;
  }
  for(i=0;i<MAXPATTERN;i++) {
    overflowGuard(i);
  }

}


/*Returns true if boardArray contains n pieces of given type in a row
**Currently assumes that the board is a square, but it is unlikely this function
**will be called for non-square boards.
**assumes n>1
**
**boardArray is any array of some element, while type is a type of that element.
**memcmp is used to match elements of boardArray with type*/
BOOLEAN NinaRow(void* boardArray,void* type, int indexOfLast, int n) {
  int i, direction,location,count;
  BOOLEAN soFar,overflow;
  
  lBoard.count++;
  //printf("NinaRow has been called with last index %d for the %dth time\n",indexOfLast,lBoard.count);
  
  if (indexOfLast<0) {  //No valid last index passed, call stateless NinaRow
    return statelessNinaRow(boardArray,type,n);
  }

  if (memcmp(boardArray + lBoard.eltSize*indexOfLast, type, lBoard.eltSize)) { //piece at last index isn't desired type
    return FALSE;
  }

  for(direction = (lBoard.diagonals ? 0 : 1);direction<4; direction+= (lBoard.diagonals ? 1 : 2)) {
    count = 1; //we start with one piece in a row
    i = 1;
    soFar = TRUE;

    while (i<n && count<n && soFar) {
      
      //expand search in the positive direction
      location = indexOfLast + i*lBoard.directionMap[direction];
      
      overflow = (location%lBoard.cols - indexOfLast%lBoard.cols > 0); //true if location jumped edge 
      if (direction > 1) {
	overflow = !(overflow); //adjust for other directions
      }

      if ( overflow ||  
	   (location < 0) ||                                  //new location is out of bounds
	   (location > lBoard.rows*lBoard.cols - 1) ||
	   (memcmp(boardArray + lBoard.eltSize*location, type, lBoard.eltSize))) { //piece at location isn't a match
	soFar = FALSE;
      } else {	
	count++;
	//printf("positive count incremented to %d with location %d and type %d and direction %d\n",count,location,*(int*)type,direction);
      }
      
      i++;
    }

    if (count>=n) {
      //printf("Positive edge N in a row found in direction %d\n",direction);
      return TRUE;
    }
    
    i = 1;
    soFar = TRUE;

    while (i<n && count<n && soFar) {
      
      //expand search in the positive direction
      location = indexOfLast + -1*i*lBoard.directionMap[direction];
      
      overflow = (location%lBoard.cols - indexOfLast%lBoard.cols < 0); //sign reversed for opposite direction
      if (direction > 1) {
	overflow = !(overflow); //adjust for other directions
      }

      if ( overflow || 
	   (location < 0) ||                                  //new location is out of bounds
	   (location > lBoard.rows*lBoard.cols - 1) ||
	   (memcmp(boardArray + lBoard.eltSize*location, type, lBoard.eltSize))) { //piece at location isn't a match
	soFar = FALSE;
      } else {
	count++;
	//printf("negative count incremented to %d with location %d and type %d and direction %d\n",count,location,*(int*)type,direction);
      }

      i++;
    }
    
    if (count >= n) {
      //printf("Negative edge or center N in a row found in direction %d\n",direction);
      return TRUE;
    }
  }
return FALSE;
}


/*stateless version of NinaRow, which is called when no indexOfLast is kept.
**Checks the entire board for NinaRow, with minimal redundant calculations.*/
BOOLEAN statelessNinaRow(void* boardArray,void* type,int n) {
  int z;
  int i, direction,location,count;
  BOOLEAN soFar,overflow;

  for(i=0;i<lBoard.rows*lBoard.cols;i++) { //reset search for all positions
    lBoard.scratchBoard[i] = 0;
  }
  
  for(z=0;z<lBoard.rows*lBoard.cols;z++) { //for each board slot
    
    if (!memcmp(boardArray + lBoard.eltSize*z, type, lBoard.eltSize)) { //piece at slot is right type, continue search
      for(direction = (lBoard.diagonals ? 0 : 1);direction<4; direction+= (lBoard.diagonals ? 1 : 2)) {
	
	//check if we should search in this direction
	if (!(powersOf2[direction] & lBoard.scratchBoard[z])) {	
	  
	  count = 1; 
	  i = 1;
	  soFar = TRUE;
	  
	  while (i<n && count<n && soFar) {
	    
	    //expand search
	    location = z + i*lBoard.directionMap[direction+3]; //starting at upper left, search from directions 3 to 6
	    
	    overflow = (location%lBoard.cols - z%lBoard.cols < 0); //true if location jumped edge 
	    if (direction > 2) {
	      overflow = !(overflow); //adjust for other directions
	    }
	    
	    //printf("Searching location %d in directon %d\n",location,direction+3);
	    
	    //mark location as read in this direction
	    if (!overflow) {
	      lBoard.scratchBoard[location] += powersOf2[direction];
	    }

	    if ( overflow ||  
		 (location < 0) ||                                  //new location is out of bounds
		 (location > lBoard.rows*lBoard.cols - 1) ||
		 (memcmp(boardArray + lBoard.eltSize*location, type, lBoard.eltSize))) { //piece at location isn't a match
	      soFar = FALSE;
	    } else {	
	      count++;
	      //printf("positive count incremented to %d with location %d and type %d and direction %d\n",count,location,*(int*)type,direction+3);
	    }
	    
	    i++;
	  }
	  
	  if (count>=n) {
	    //printf("N in a row found in direction %d\n",direction);
	    return TRUE;
	  }
	}
      }
    }
  }
  return FALSE;
}

/*Returns whether the board contains amount of given item 'what'
//if atLeast is true, function will short curcuit once amount is found*/
BOOLEAN amountOfWhat(void* boardArray,void* what,int amount,BOOLEAN atLeast) {
  int i,count=0;
  
  if (atLeast) {
    if (amount==0) {
      return TRUE;
    }
  }
    
  for(i=0;i<lBoard.rows*lBoard.cols;i++) {
    if (!memcmp(boardArray + lBoard.eltSize*i,what,lBoard.eltSize)) { //element found
      count++;
      if (atLeast) {
	if (count == amount) {
	  return TRUE;
	}
      }
    }
  }

  if (count==amount) {
    return TRUE;
  }
  return FALSE;
}

BOOLEAN OneDMatch(void* boardArray, void* pattern, BOOLEAN* transparencies, BOOLEAN symmetrical, int len) {
  int z;
  int direction,location,count;
  BOOLEAN soFar; //overflow;

  for(z=0;z<lBoard.rows*lBoard.cols;z++) { //for each board slot
    
    if (!memcmp(boardArray + lBoard.eltSize*z, pattern, lBoard.eltSize)) { //piece at slot is right type, continue search
      for(direction = (symmetrical ? 3 : (lBoard.diagonals ? 0 : 1)); direction < (symmetrical ? 7 : 8); direction+= (lBoard.diagonals ? 1 : 2)) {	
	count = 1; 
	soFar = TRUE;
	
	if ( lBoard.overflowBoards[len][z] & (1 << direction) ) { //will not run into the boundries searching in this direction  
	  while (count<len && soFar) {
	    
	    //expand search
	    location = z + count*lBoard.directionMap[direction];
	    
	    if (memcmp(boardArray + lBoard.eltSize*location, pattern + lBoard.eltSize*count, lBoard.eltSize)) { //piece at location isn't a match
	      soFar = FALSE;
	    } else {	
	      count++;
	    }
	  }
	  
	  if (count>=len) {
	    //printf("N in a row found in direction %d\n",direction);
	    return TRUE;
	  }
	}
      }
    }
  }
  return FALSE;
}

/*Computes, for each slot, which directions can be searched without exceeding board boundries*/
void overflowGuard(int len) {
  int z,upCheck, rightCheck, leftCheck, downCheck;

  for(z=0;z<lBoard.rows*lBoard.cols;z++) { //for each board slot
    lBoard.overflowBoards[len][z]=0;

    upCheck = (z/lBoard.rows >= len-1);
    leftCheck = (z%lBoard.cols >= len-1);
    rightCheck = (lBoard.cols - z%lBoard.cols >= len);
    downCheck = (lBoard.rows - z/lBoard.rows >= len);
    
    lBoard.overflowBoards[len][z] = 
      (upCheck && leftCheck) + 
      ((upCheck) << 1) +
      ((upCheck && rightCheck) << 2) + 
      ((rightCheck) << 3) +
      ((rightCheck && downCheck) << 4) +
      (downCheck << 5) + 
      ((downCheck && leftCheck) << 6) +
      (leftCheck << 7);
  }
}  

//can I call functions from this module remotely?
void Test() {
  printf("Now using game function libraries\n");
}

/**************************DEPRECIATED CODE***********************
Assuming BlankOX representation is used, prints the board*
void printBoard(BlankOX* board) {
  int i;
  for(i=0;i<lBoard.rows*lBoard.cols;i++) {
    printf("%d ",board[i]);
  }
  printf("\n");
}

BOOLEAN mymemcmp(void* data1,void* data2,int amt) {
  int i;
  char* d1;
  char* d2;
  d1 = (char*)data1;
  d2 = (char*)data2;
  for(i=0;i<amt;i++) {
    printf("Data 1 location is %d, data 2 location is %d, index is %d\n",d1+i,d2+i,i);
    if (d1[i] != d2[i]) {
      printf("memcmp returning you dolt\n");
      return FALSE;
    }
  }
  printf("memcmp returning you dolt\n");
  return TRUE;
}
*/


//old broken code for NinaRow
  /*  for(direction = (lBoard.diagonals ? 0 : 1);direction<8; direction+= (lBoard.diagonals ? 1 : 2)) { //test for each direction
    soFar = 1;
    i = 0;

    while(i<n && soFar) { //perform n slot checks on boardArray, using soFar as a condition to short circuit

      //printf("Solving %dth in a row\n",i);

      location = indexOfLast + i*lBoard.directionMap[direction];
      
      if ( (indexOfLast != location % lBoard.rows) ||  //new location crossed the edge of the board
	   (location < 0) ||                                  //new location is out of bounds
	   (location > lBoard.rows*lBoard.cols - 1)) {
	soFar = 0;
      } else {	
	soFar = (soFar && (boardArray[location] == type));
      }
      i++;
    }

    found+=soFar;
  }

//old inner loop for OneDMatch
	while (count<len && soFar) {
	  
	  //expand search
	  location = z + count*lBoard.directionMap[direction];
	  
	  overflow = (location%lBoard.cols - z%lBoard.cols > 0); //true if location jumped edge 
	  if (direction > 1 && direction < 5) {
	    overflow = !(overflow); //adjust for other directions
	  }
	  
	  //printf("Searching location %d in directon %d, from slot %d\n",location,direction,z);
	  //printf("Count is %d\n",count);
	  
           if ( //overflow ||  
	       //(location < 0) ||                                  //new location is out of bounds
	       //(location > lBoard.rows*lBoard.cols - 1) ||
	       (memcmp(boardArray + lBoard.eltSize*location, pattern + lBoard.eltSize*count, lBoard.eltSize))) { //piece at location isn't a match
	    soFar = FALSE;
	  } else {	
	    count++;
	  }
	}
*/
