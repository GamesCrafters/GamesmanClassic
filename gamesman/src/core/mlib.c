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
**************************************************************************/
#include <stdio.h>
#include "gamesman.h"
#include "mlib.h"
#include "string.h"

#define MAXBOARDSIZE 64

struct {
  int count;
  int eltSize;
  int rows;
  int cols;
  BOOLEAN diagonals;
  int directionMap[8];
  int scratchBoard[MAXBOARDSIZE];
} LocalBoard;

int powersOf2[] = {1,2,4,8};

/*Set local variables for processing input arrays*/
void LibInitialize(int eltSize, int rows, int cols, BOOLEAN diagonals) {
  int i;
  LocalBoard.count = 0;
  LocalBoard.eltSize = eltSize;
  LocalBoard.rows = rows;
  LocalBoard.cols = cols;
  LocalBoard.diagonals = diagonals;

  LocalBoard.directionMap[0] = -cols - 1; //used in NinaRow
  LocalBoard.directionMap[1] = -cols;
  LocalBoard.directionMap[2] = -cols + 1;
  LocalBoard.directionMap[3] = 1;
  LocalBoard.directionMap[4] = cols + 1;
  LocalBoard.directionMap[5] = cols;
  LocalBoard.directionMap[6] = cols - 1;
  LocalBoard.directionMap[7] = -1;

  for(i=0;i<MAXBOARDSIZE;i++) {
    LocalBoard.scratchBoard[i] = 0;
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
  
  LocalBoard.count++;
  //printf("NinaRow has been called with last index %d for the %dth time\n",indexOfLast,LocalBoard.count);
  
  if (indexOfLast<0) {  //No valid last index passed, call stateless NinaRow
    return statelessNinaRow(boardArray,type,n);
  }

  if (memcmp(boardArray + LocalBoard.eltSize*indexOfLast, type, LocalBoard.eltSize)) { //piece at last index isn't desired type
    return FALSE;
  }

  for(direction = (LocalBoard.diagonals ? 0 : 1);direction<4; direction+= (LocalBoard.diagonals ? 1 : 2)) {
    count = 1; //we start with one piece in a row
    i = 1;
    soFar = TRUE;

    while (i<n && count<n && soFar) {
      
      //expand search in the positive direction
      location = indexOfLast + i*LocalBoard.directionMap[direction];
      
      overflow = (location%LocalBoard.rows - indexOfLast%LocalBoard.rows > 0); //true if location jumped edge 
      if (direction > 1) {
	overflow = !(overflow); //adjust for other directions
      }

      if ( overflow ||  
	   (location < 0) ||                                  //new location is out of bounds
	   (location > LocalBoard.rows*LocalBoard.cols - 1) ||
	   (memcmp(boardArray + LocalBoard.eltSize*location, type, LocalBoard.eltSize))) { //piece at location isn't a match
	soFar = FALSE;
      } else {	
	count++;
	//printf("positive count incremented to %d with location %d and type %d and direction %d\n",count,location,*(int*)type,direction);
      }
      
      i++;
    }

    if (count>=n) {
      printf("Positive edge N in a row found in direction %d\n",direction);
      return TRUE;
    }
    
    i = 1;
    soFar = TRUE;

    while (i<n && count<n && soFar) {
      
      //expand search in the positive direction
      location = indexOfLast + -1*i*LocalBoard.directionMap[direction];
      
      overflow = (location%LocalBoard.rows - indexOfLast%LocalBoard.rows < 0); //sign reversed for opposite direction
      if (direction > 1) {
	overflow = !(overflow); //adjust for other directions
      }

      if ( overflow || 
	   (location < 0) ||                                  //new location is out of bounds
	   (location > LocalBoard.rows*LocalBoard.cols - 1) ||
	   (memcmp(boardArray + LocalBoard.eltSize*location, type, LocalBoard.eltSize))) { //piece at location isn't a match
	soFar = FALSE;
      } else {
	count++;
	//printf("negative count incremented to %d with location %d and type %d and direction %d\n",count,location,*(int*)type,direction);
      }

      i++;
    }
    
    if (count >= n) {
      printf("Negative edge or center N in a row found in direction %d\n",direction);
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

  for(i=0;i<LocalBoard.rows*LocalBoard.cols;i++) { //reset search for all positions
    LocalBoard.scratchBoard[i] = 0;
  }
  
  for(z=0;z<LocalBoard.rows*LocalBoard.cols;z++) { //for each board slot
    
    if (!memcmp(boardArray + LocalBoard.eltSize*z, type, LocalBoard.eltSize)) { //piece at slot is right type, continue search
      for(direction = (LocalBoard.diagonals ? 0 : 1);direction<4; direction+= (LocalBoard.diagonals ? 1 : 2)) {
	
	//check if we should search in this direction
	if (!(powersOf2[direction] & LocalBoard.scratchBoard[z])) {	
	  
	  count = 1; 
	  i = 1;
	  soFar = TRUE;
	  
	  while (i<n && count<n && soFar) {
	    
	    //expand search
	    location = z + i*LocalBoard.directionMap[direction+3]; //starting at upper left, search from directions 3 to 6
	    
	    overflow = (location%LocalBoard.rows - z%LocalBoard.rows < 0); //true if location jumped edge 
	    if (direction > 2) {
	      overflow = !(overflow); //adjust for other directions
	    }
	    
	    //printf("Searching location %d in directon %d\n",location,direction+3);
	    
	    //mark location as read in this direction
	    if (!overflow) {
	      LocalBoard.scratchBoard[location] += powersOf2[direction];
	    }

	    if ( overflow ||  
		 (location < 0) ||                                  //new location is out of bounds
		 (location > LocalBoard.rows*LocalBoard.cols - 1) ||
		 (memcmp(boardArray + LocalBoard.eltSize*location, type, LocalBoard.eltSize))) { //piece at location isn't a match
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

/*Returns whether the board contains amount of given item 'what'*/
BOOLEAN amountOfWhat(void* boardArray,void* what,int amount,BOOLEAN atLeast) {
  int i,count=0;
  
  if (atLeast) {
    if (amount==0) {
      return TRUE;
    }
  }
    
  for(i=0;i<LocalBoard.rows*LocalBoard.cols;i++) {
    if (!memcmp(boardArray + LocalBoard.eltSize*i,what,LocalBoard.eltSize)) { //element found
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
  
//can I call functions from this module remotely?
void Test() {
  printf("Now using game function libraries\n");
}

/**************************DEPRECIATED CODE***********************
Assuming BlankOX representation is used, prints the board*
void printBoard(BlankOX* board) {
  int i;
  for(i=0;i<LocalBoard.rows*LocalBoard.cols;i++) {
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
  /*  for(direction = (LocalBoard.diagonals ? 0 : 1);direction<8; direction+= (LocalBoard.diagonals ? 1 : 2)) { //test for each direction
    soFar = 1;
    i = 0;

    while(i<n && soFar) { //perform n slot checks on boardArray, using soFar as a condition to short circuit

      //printf("Solving %dth in a row\n",i);

      location = indexOfLast + i*LocalBoard.directionMap[direction];
      
      if ( (indexOfLast != location % LocalBoard.rows) ||  //new location crossed the edge of the board
	   (location < 0) ||                                  //new location is out of bounds
	   (location > LocalBoard.rows*LocalBoard.cols - 1)) {
	soFar = 0;
      } else {	
	soFar = (soFar && (boardArray[location] == type));
      }
      i++;
    }

    found+=soFar;
  }
  */
