#ifndef _MLIB_H_
#define _MLIB_H_

/*function prototypes*/

typedef enum possibleBoardPieces {
  Blank, o, x
} BlankOX;

void LibInitialize(int,int,int,BOOLEAN);
BOOLEAN NinaRow(BlankOX*,BlankOX,int,int);
BOOLEAN AmountOfWhat();
void Test();

#endif
