#ifndef _MLIB_H_
#define _MLIB_H_

/*temporary fix for the general board array problem
**Modules must delete their BlankOX declaration and include this file*/
typedef enum possibleBoardPieces {
  Blank, o, x
} BlankOX;

/*function prototypes*/

void LibInitialize(int,int,int,BOOLEAN);
BOOLEAN NinaRow(BlankOX*,BlankOX,int,int);
BOOLEAN AmountOfWhat();
void Test();

#endif
