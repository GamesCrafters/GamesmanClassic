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
**              3/5/06 - Created basic framework, possibly functional draft
**                       of NinaRow.
**************************************************************************/
#include "mlib.h"

struct {
  int eltSize;
  int rows;
  int cols;
  BOOLEAN diagonals;
  int directionMap[8];
} LocalBoard;

/*Set local variables for processing input arrays*/
void LibInitialize(int eltSize, int rows, int cols, BOOLEAN diagonals) {
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
}

/*Returns true if boardArray contains n pieces of given type in a row
**Currently assumes that the board is a square, but it is unlikely this function
**will be called for non-square boards.
**assumes n>1 */
BOOLEAN NinaRow(BlankOX* boardArray, BlankOX type, int indexOfLast, int n) {
  int i, direction, soFarUp, soFarDown, location,count;
  for(direction = (LocalBoard.diagonals ? 0 : 1);direction<4; direction+= (LocalBoard.diagonals ? 1 : 2)) {
    count = 0;
    i = 1;
    soFarUp = 1;
    soFarDown = 1;

    while (i<n && count<n && (soFarUp || soFarDown)) {
      
      //expand search in the positive direction
      location = indexOfLast + i*LocalBoard.directionMap[direction];
      
      if ( (indexOfLast != location % LocalBoard.rows) ||  //new location crossed the edge of the board
	   (location < 0) ||                                  //new location is out of bounds
	   (location > LocalBoard.rows*LocalBoard.cols - 1)) {
      } else {	
	soFarUp = (soFarUp && boardArray[location] == type);
      }

      count+=soFarUp;

      //expand search in the negative direction
      location = indexOfLast + -1*i*LocalBoard.directionMap[direction];
      
      if ( (indexOfLast != location % LocalBoard.rows) ||  //new location crossed the edge of the board
	   (location < 0) ||                                  //new location is out of bounds
	   (location > LocalBoard.rows*LocalBoard.cols - 1)) {
      } else {	
	soFarDown = (soFarDown && boardArray[location] == type);
      }

      count+=soFarDown;

      i++;
    }
    
    if (count >= n) {
      return 1;
    }
  }
  return 0;
}
    


//can I call functions from this module remotely?
void Test() {
  printf("Now using game function libraries\n");
}




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
