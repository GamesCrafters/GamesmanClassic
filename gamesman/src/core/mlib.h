#ifndef _MLIB_H_
#define _MLIB_H_

/*function prototypes*/

void LibInitialize(int,int,int,BOOLEAN);
BOOLEAN NinaRow(void*,void*,int,int);
BOOLEAN statelessNinaRow(void*,void*,int);
BOOLEAN amountOfWhat(void*,void*,int,BOOLEAN);
BOOLEAN OneDMatch(void*,void*,BOOLEAN*,BOOLEAN,int);
BOOLEAN TwoDMatch(void*,void*);
void Test();

BOOLEAN mymemcmp(void*,void*,int);

#endif
