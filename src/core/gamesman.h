#ifndef GMCORE_GAMESMAN_H
#define GMCORE_GAMESMAN_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/utsname.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <errno.h>

#include <sys/time.h>
#include <limits.h>
#include "types.h"
#include "hash.h"
#include "hashwindow.h"
#include "db.h"
#include "analysis.h"
#include "visualization.h"
#include "openPositions.h"
#include "constants.h"
#include "globals.h"
#include "debug.h"
#include "gameplay.h"
#include "misc.h"
#include "textui.h"
#include "mlib.h"
#include "interact.h"
#include "main.h"
#include "seval.h"

/* For memory debugging */
#include "memwatch.h"

/* function prototypes implemented in the module code */

void                    InitializeGame                  (void);
POSITION                DoMove                          (POSITION thePosition, MOVE theMove);
POSITION                GetInitialPosition              (void);
void                    PrintComputersMove              (MOVE computersMove, STRING computersName);
VALUE                   Primitive                       (POSITION position);
void                    PrintPosition                   (POSITION position, STRING playerName, BOOLEAN usersTurn);
MOVELIST*               GenerateMoves                   (POSITION position);
USERINPUT               GetAndPrintPlayersMove          (POSITION thePosition, MOVE* theMove, STRING playerName);
BOOLEAN                 ValidTextInput                  (STRING input);
MOVE                    ConvertTextInputToMove          (STRING input);
void                    PrintMove                       (MOVE theMove);
int                     NumberOfOptions                 (void);
int                     getOption                       (void);
void                    setOption                       (int option);
void SetTclCGameSpecificOptions      (int theOptions[]);
void                    GameSpecificMenu                (void);
void                    DebugMenu                       (void);

#endif /* GMCORE_GAMESMAN_H */
