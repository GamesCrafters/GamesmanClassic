/************************************************************************
**
** NAME:        m9mm.c
**
** DESCRIPTION: Nine Men's Morris
**
** AUTHOR:      Erwin A. Vedar, Wei Tu, Elmer Lee
**
** DATE:        Start: 10am 2004.2.22
**              Finish: Never
**
**************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/

#include <stdio.h>
#include "gamesman.h"
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include "hash.h"
//#include "loopyupsolver.h"

POSITION gNumberOfPositions  = 0;

POSITION gInitialPosition    = 432840;
POSITION gMinimalPosition    = 432840;
POSITION kBadPosition        = -1;

/**************************************************************************
 ** POSITION 432840:      POSITION 1329:
 **
 **    x  _  o               x  x  x
 **     _ _ _                 _ _ _
 **      _x_                   ___
 **    ___ ___               ___ ___
 **      _o_                   ___
 **     _ _ _                 _ _ _
 **    o  _  x               o  o  o
 **                                
 **   x to move             x to move 
 **************************************************************************/


STRING   kGameName           = "Nine Men's Morris";
STRING   kDBName             = "9mm";
BOOLEAN  kPartizan           = TRUE; 
BOOLEAN  kDebugMenu          = TRUE;
BOOLEAN  kGameSpecificMenu   = TRUE;
BOOLEAN  kTieIsPossible      = FALSE;
BOOLEAN  kLoopy               = TRUE;
BOOLEAN  kDebugDetermineValue = FALSE;
void*	 gGameSpecificTclInit = NULL;

STRING kHelpGraphicInterface =
"Nine Men's Morris does not currently support a Graphical User Interface\n(other than beloved ASCII).";

STRING kHelpTextInterface =
"The LEGEND shows numbers corresponding to positions on the board.  On your\nturn, use the LEGEND to enter the position your piece currently is, the position\nyour piece is moving to, and (if your move creates a mill) the position of the\npiece you wish to remove from play.  Seperate each number entered with a space\nand hit return to commit your move.  If you ever make a mistake when choosing\nyour move, you can type \"u\" and hit return to revert back to your most recent\nposition."; 

STRING kHelpOnYourTurn;

// default kHelpOnYourTurn
STRING kHelpOnYourTurn0 = 
"If you still have pieces not on the board, place them in any open position.\nWhen all of your pieces have been placed on the board, you can choose any one of\nyour pieces and move it to an adjacent, open position.  If any of your moves\nresults in a 3-in-a-row, you can remove one of your opponent's pieces that is\nnot in a mill.  If your opponent only has 3 pieces remaining which happen to be\nin a mill, you can choose to remove any 1 of those 3 pieces.";

// kHelpWithFlying appears only if gFlying is TRUE
STRING kHelpWithFlying = 
"\n\nSpecial Rule: Flying\nIf you only have 3 pieces remaining, you may choose to move your piece to\nany open position in addition to the open positions adjacent to your piece.";

STRING kHelpStandardObjective =
"A victory in Nine Men's Morris is won by reducing the number of your\nopponent's pieces down to two or by blocking all of your opponent's pieces so that\nthe opponent can no longer move.";

STRING kHelpReverseObjective =
"A misere victory in Nine Men's Morris is won by being the first player to\nhave only two pieces on the board or the first player to be totally blocked\n(and have no more legal moves).";

STRING kHelpTieOccursWhen = /* Should follow 'A Tie occurs when... */
"A Tie will never occur in a standard game of Nine Men's Morris.";

STRING kHelpExample =
"\
        0-----------1-----------2\t_-----------x-----------_\n\
        |           |           |\t|           |           |\n\
        |           |           |\t|           |           |\n\
        |   3-------4-------5   |\t|   _-------_-------_   |\n\
        |   |       |       |   |\t|   |       |       |   |\n\
        |   |   6---7---8   |   |\t|   |   _---x---_   |   |\n\
        |   |   |       |   |   |\t|   |   |       |   |   |\n\
LEGEND: 9---10--11      12--13--14\t_---_---_       _---_---_\tTurn: x\n\
        |   |   |       |   |   |\t|   |   |       |   |   |\n\
        |   |   15--16--17  |   |\t|   |   _---x---o   |   |\n\
        |   |       |       |   |\t|   |       |       |   |\n\
        |   18------19------20  |\t|   _-------_-------_   |\n\
        |           |           |\t|           |           |\n\
        |           |           |\t|           |           |\n\
        21----------22----------23\t_-----------o-----------o\n\
\n\
Player's move [(u)ndo/0-23 0-23 (0-23)] : { 16 15 }\n\
\n\n\
        0-----------1-----------2\t_-----------x-----------_\n\
        |           |           |\t|           |           |\n\
        |           |           |\t|           |           |\n\
        |   3-------4-------5   |\t|   _-------_-------_   |\n\
        |   |       |       |   |\t|   |       |       |   |\n\
        |   |   6---7---8   |   |\t|   |   _---x---_   |   |\n\
        |   |   |       |   |   |\t|   |   |       |   |   |\n\
LEGEND: 9---10--11      12--13--14\t_---_---_       _---_---_\tTurn: o\n\
        |   |   |       |   |   |\t|   |   |       |   |   |\n\
        |   |   15--16--17  |   |\t|   |   x---_---o   |   |\n\
        |   |       |       |   |\t|   |       |       |   |\n\
        |   18------19------20  |\t|   _-------_-------_   |\n\
        |           |           |\t|           |           |\n\
        |           |           |\t|           |           |\n\
        21----------22----------23\t_-----------o-----------o\n\
\n\
Computer's move                         : 17 16\n\
\n\n\
        0-----------1-----------2\t_-----------x-----------_\n\
        |           |           |\t|           |           |\n\
        |           |           |\t|           |           |\n\
        |   3-------4-------5   |\t|   _-------_-------_   |\n\
        |   |       |       |   |\t|   |       |       |   |\n\
        |   |   6---7---8   |   |\t|   |   _---x---_   |   |\n\
        |   |   |       |   |   |\t|   |   |       |   |   |\n\
LEGEND: 9---10--11      12--13--14\t_---_---_       _---_---_\tTurn: x\n\
        |   |   |       |   |   |\t|   |   |       |   |   |\n\
        |   |   15--16--17  |   |\t|   |   x---o---_   |   |\n\
        |   |       |       |   |\t|   |       |       |   |\n\
        |   18------19------20  |\t|   _-------_-------_   |\n\
        |           |           |\t|           |           |\n\
        |           |           |\t|           |           |\n\
        21----------22----------23\t_-----------o-----------o\n\
\n\
Player's move [(u)ndo/0-23 0-23 (0-23)] : { 15 11 }\n\
\n\n\
        0-----------1-----------2\t_-----------x-----------_\n\
        |           |           |\t|           |           |\n\
        |           |           |\t|           |           |\n\
        |   3-------4-------5   |\t|   _-------_-------_   |\n\
        |   |       |       |   |\t|   |       |       |   |\n\
        |   |   6---7---8   |   |\t|   |   _---x---_   |   |\n\
        |   |   |       |   |   |\t|   |   |       |   |   |\n\
LEGEND: 9---10--11      12--13--14\t_---_---x       _---_---_\tTurn: o\n\
        |   |   |       |   |   |\t|   |   |       |   |   |\n\
        |   |   15--16--17  |   |\t|   |   _---o---_   |   |\n\
        |   |       |       |   |\t|   |       |       |   |\n\
        |   18------19------20  |\t|   _-------_-------_   |\n\
        |           |           |\t|           |           |\n\
        |           |           |\t|           |           |\n\
        21----------22----------23\t_-----------o-----------o\n\
\n\
Computer's move                         : 16 19\n\
\n\n\
        0-----------1-----------2\t_-----------x-----------_\n\
        |           |           |\t|           |           |\n\
        |           |           |\t|           |           |\n\
        |   3-------4-------5   |\t|   _-------_-------_   |\n\
        |   |       |       |   |\t|   |       |       |   |\n\
        |   |   6---7---8   |   |\t|   |   _---x---_   |   |\n\
        |   |   |       |   |   |\t|   |   |       |   |   |\n\
LEGEND: 9---10--11      12--13--14\t_---_---x       _---_---_\tTurn: x\n\
        |   |   |       |   |   |\t|   |   |       |   |   |\n\
        |   |   15--16--17  |   |\t|   |   _---_---_   |   |\n\
        |   |       |       |   |\t|   |       |       |   |\n\
        |   18------19------20  |\t|   _-------o-------_   |\n\
        |           |           |\t|           |           |\n\
        |           |           |\t|           |           |\n\
        21----------22----------23\t_-----------o-----------o\n\
\n\
Player's move [(u)ndo/0-23 0-23 (0-23)] : { 11 6 }\n\
\n\n\
        0-----------1-----------2\t_-----------x-----------_\n\
        |           |           |\t|           |           |\n\
        |           |           |\t|           |           |\n\
        |   3-------4-------5   |\t|   _-------_-------_   |\n\
        |   |       |       |   |\t|   |       |       |   |\n\
        |   |   6---7---8   |   |\t|   |   x---x---_   |   |\n\
        |   |   |       |   |   |\t|   |   |       |   |   |\n\
LEGEND: 9---10--11      12--13--14\t_---_---_       _---_---_\tTurn: o\n\
        |   |   |       |   |   |\t|   |   |       |   |   |\n\
        |   |   15--16--17  |   |\t|   |   _---_---_   |   |\n\
        |   |       |       |   |\t|   |       |       |   |\n\
        |   18------19------20  |\t|   _-------o-------_   |\n\
        |           |           |\t|           |           |\n\
        |           |           |\t|           |           |\n\
        21----------22----------23\t_-----------o-----------o\n\
\n\
Computer's move                         : 22 21\n\
\n\n\
        0-----------1-----------2\t_-----------x-----------_\n\
        |           |           |\t|           |           |\n\
        |           |           |\t|           |           |\n\
        |   3-------4-------5   |\t|   _-------_-------_   |\n\
        |   |       |       |   |\t|   |       |       |   |\n\
        |   |   6---7---8   |   |\t|   |   x---x---_   |   |\n\
        |   |   |       |   |   |\t|   |   |       |   |   |\n\
LEGEND: 9---10--11      12--13--14\t_---_---_       _---_---_\tTurn: x\n\
        |   |   |       |   |   |\t|   |   |       |   |   |\n\
        |   |   15--16--17  |   |\t|   |   _---_---_   |   |\n\
        |   |       |       |   |\t|   |       |       |   |\n\
        |   18------19------20  |\t|   _-------o-------_   |\n\
        |           |           |\t|           |           |\n\
        |           |           |\t|           |           |\n\
        21----------22----------23\to-----------_-----------o\n\
\n\
Player's move [(u)ndo/0-23 0-23 (0-23)] : { 7 8 }\n\
\n\n\
        0-----------1-----------2\t_-----------x-----------_\n\
        |           |           |\t|           |           |\n\
        |           |           |\t|           |           |\n\
        |   3-------4-------5   |\t|   _-------_-------_   |\n\
        |   |       |       |   |\t|   |       |       |   |\n\
        |   |   6---7---8   |   |\t|   |   x---_---x   |   |\n\
        |   |   |       |   |   |\t|   |   |       |   |   |\n\
LEGEND: 9---10--11      12--13--14\t_---_---_       _---_---_\tTurn: o\n\
        |   |   |       |   |   |\t|   |   |       |   |   |\n\
        |   |   15--16--17  |   |\t|   |   _---_---_   |   |\n\
        |   |       |       |   |\t|   |       |       |   |\n\
        |   18------19------20  |\t|   _-------o-------_   |\n\
        |           |           |\t|           |           |\n\
        |           |           |\t|           |           |\n\
        21----------22----------23\to-----------_-----------o\n\
\n\
Computer's move                         : 19 22 1\n\
\n\n\
        0-----------1-----------2\t_-----------_-----------_\n\
        |           |           |\t|           |           |\n\
        |           |           |\t|           |           |\n\
        |   3-------4-------5   |\t|   _-------_-------_   |\n\
        |   |       |       |   |\t|   |       |       |   |\n\
        |   |   6---7---8   |   |\t|   |   x---_---x   |   |\n\
        |   |   |       |   |   |\t|   |   |       |   |   |\n\
LEGEND: 9---10--11      12--13--14\t_---_---_       _---_---_\tTurn: x\n\
        |   |   |       |   |   |\t|   |   |       |   |   |\n\
        |   |   15--16--17  |   |\t|   |   _---_---_   |   |\n\
        |   |       |       |   |\t|   |       |       |   |\n\
        |   18------19------20  |\t|   _-------_-------_   |\n\
        |           |           |\t|           |           |\n\
        |           |           |\t|           |           |\n\
        21----------22----------23\to-----------o-----------o\n\
\n\
Computer wins.  Nice try, Player.\n\
";

STRING kHelpExample0 = 
"\
        0-----------1-----------2\tx-----------_-----------o\n\
        |           |           |\t|           |           |\n\
        |           |           |\t|           |           |\n\
        |   3-------4-------5   |\t|   _-------_-------_   |\n\
        |   |       |       |   |\t|   |       |       |   |\n\
        |   |   6---7---8   |   |\t|   |   _---x---_   |   |\n\
        |   |   |       |   |   |\t|   |   |       |   |   |\n\
LEGEND: 9---10--11      12--13--14\t_---_---_       _---_---_\tTurn: x\n\
        |   |   |       |   |   |\t|   |   |       |   |   |\n\
        |   |   15--16--17  |   |\t|   |   _---o---_   |   |\n\
        |   |       |       |   |\t|   |       |       |   |\n\
        |   18------19------20  |\t|   _-------_-------_   |\n\
        |           |           |\t|           |           |\n\
        |           |           |\t|           |           |\n\
        21----------22----------23\to-----------_-----------x\n\
\n\
Player's move [(u)ndo/0-23 0-23 (0-23)] : { 0 1 }\n\
\n\n\
        0-----------1-----------2\t_-----------x-----------o\n\
        |           |           |\t|           |           |\n\
        |           |           |\t|           |           |\n\
        |   3-------4-------5   |\t|   _-------_-------_   |\n\
        |   |       |       |   |\t|   |       |       |   |\n\
        |   |   6---7---8   |   |\t|   |   _---x---_   |   |\n\
        |   |   |       |   |   |\t|   |   |       |   |   |\n\
LEGEND: 9---10--11      12--13--14\t_---_---_       _---_---_\tTurn: o\n\
        |   |   |       |   |   |\t|   |   |       |   |   |\n\
        |   |   15--16--17  |   |\t|   |   _---o---_   |   |\n\
        |   |       |       |   |\t|   |       |       |   |\n\
        |   18------19------20  |\t|   _-------_-------_   |\n\
        |           |           |\t|           |           |\n\
        |           |           |\t|           |           |\n\
        21----------22----------23\to-----------_-----------x\n\
\n\
Computer's move                         : 2 14\n\
\n\n\"\
        0-----------1-----------2\tx-----------_-----------o\n\
        |           |           |\t|           |           |\n\
        |           |           |\t|           |           |\n\
        |   3-------4-------5   |\t|   _-------_-------_   |\n\
        |   |       |       |   |\t|   |       |       |   |\n\
        |   |   6---7---8   |   |\t|   |   _---x---_   |   |\n\
        |   |   |       |   |   |\t|   |   |       |   |   |\n\
LEGEND: 9---10--11      12--13--14\t_---_---_       _---_---_\tTurn: x\n\
        |   |   |       |   |   |\t|   |   |       |   |   |\n\
        |   |   15--16--17  |   |\t|   |   _---o---_   |   |\n\
        |   |       |       |   |\t|   |       |       |   |\n\
        |   18------19------20  |\t|   _-------_-------_   |\n\
        |           |           |\t|           |           |\n\
        |           |           |\t|           |           |\n\
        21----------22----------23\to-----------_-----------x\n\
\n\
Player's move [(u)ndo/0-23 0-23 (0-23)] : { 0 1 }\n\
\n\n\
        0-----------1-----------2\t_-----------x-----------o\n\
        |           |           |\t|           |           |\n\
        |           |           |\t|           |           |\n\
        |   3-------4-------5   |\t|   _-------_-------_   |\n\
        |   |       |       |   |\t|   |       |       |   |\n\
        |   |   6---7---8   |   |\t|   |   _---x---_   |   |\n\
        |   |   |       |   |   |\t|   |   |       |   |   |\n\
LEGEND: 9---10--11      12--13--14\t_---_---_       _---_---_\tTurn: o\n\
        |   |   |       |   |   |\t|   |   |       |   |   |\n\
        |   |   15--16--17  |   |\t|   |   _---o---_   |   |\n\
        |   |       |       |   |\t|   |       |       |   |\n\
        |   18------19------20  |\t|   _-------_-------_   |\n\
        |           |           |\t|           |           |\n\
        |           |           |\t|           |           |\n\
        21----------22----------23\to-----------_-----------x\n\
\n\
Computer's move                         : 2 14\n\
\n\n\
        0-----------1-----------2\t_-----------x-----------_\n\
        |           |           |\t|           |           |\n\
        |           |           |\t|           |           |\n\
        |   3-------4-------5   |\t|   _-------_-------_   |\n\
        |   |       |       |   |\t|   |       |       |   |\n\
        |   |   6---7---8   |   |\t|   |   _---x---_   |   |\n\
        |   |   |       |   |   |\t|   |   |       |   |   |\n\
LEGEND: 9---10--11      12--13--14\t_---_---_       _---_---o\tTurn: x\n\
        |   |   |       |   |   |\t|   |   |       |   |   |\n\
        |   |   15--16--17  |   |\t|   |   _---o---_   |   |\n\
        |   |       |       |   |\t|   |       |       |   |\n\
        |   18------19------20  |\t|   _-------_-------_   |\n\
        |           |           |\t|           |           |\n\
        |           |           |\t|           |           |\n\
        21----------22----------23\to-----------_-----------x\n\
\n\
Player's move [(u)ndo/0-23 0-23 (0-23)] : { 23 22 }\n\
\n\n\
        0-----------1-----------2\t_-----------x-----------_\n\
        |           |           |\t|           |           |\n\
        |           |           |\t|           |           |\n\
        |   3-------4-------5   |\t|   _-------_-------_   |\n\
        |   |       |       |   |\t|   |       |       |   |\n\
        |   |   6---7---8   |   |\t|   |   _---x---_   |   |\n\
        |   |   |       |   |   |\t|   |   |       |   |   |\n\
LEGEND: 9---10--11      12--13--14\t_---_---_       _---_---o\tTurn: o\n\
        |   |   |       |   |   |\t|   |   |       |   |   |\n\
        |   |   15--16--17  |   |\t|   |   _---o---_   |   |\n\
        |   |       |       |   |\t|   |       |       |   |\n\
        |   18------19------20  |\t|   _-------_-------_   |\n\
        |           |           |\t|           |           |\n\
        |           |           |\t|           |           |\n\
        21----------22----------23\to-----------x-----------_\n\
\n\
Computer's move                         : 16 17\n\
\n\n\
";

STRING kHelpExample1 = 
"\
        0-----------1-----------2\t_-----------x-----------_\n\
        |           |           |\t|           |           |\n\
        |           |           |\t|           |           |\n\
        |   3-------4-------5   |\t|   _-------_-------_   |\n\
        |   |       |       |   |\t|   |       |       |   |\n\
        |   |   6---7---8   |   |\t|   |   _---x---_   |   |\n\
        |   |   |       |   |   |\t|   |   |       |   |   |\n\
LEGEND: 9---10--11      12--13--14\t_---_---_       _---_---o\tTurn: x\n\
        |   |   |       |   |   |\t|   |   |       |   |   |\n\
        |   |   15--16--17  |   |\t|   |   _---_---o   |   |\n\
        |   |       |       |   |\t|   |       |       |   |\n\
        |   18------19------20  |\t|   _-------_-------_   |\n\
        |           |           |\t|           |           |\n\
        |           |           |\t|           |           |\n\
        21----------22----------23\to-----------x-----------_\n\
\n\
Player's move [(u)ndo/0-23 0-23 (0-23)] : { 22 19 }\n\
\n\n\
        0-----------1-----------2\t_-----------x-----------_\n\
        |           |           |\t|           |           |\n\
        |           |           |\t|           |           |\n\
        |   3-------4-------5   |\t|   _-------_-------_   |\n\
        |   |       |       |   |\t|   |       |       |   |\n\
        |   |   6---7---8   |   |\t|   |   _---x---_   |   |\n\
        |   |   |       |   |   |\t|   |   |       |   |   |\n\
LEGEND: 9---10--11      12--13--14\t_---_---_       _---_---o\tTurn: o\n\
        |   |   |       |   |   |\t|   |   |       |   |   |\n\
        |   |   15--16--17  |   |\t|   |   _---_---o   |   |\n\
        |   |       |       |   |\t|   |       |       |   |\n\
        |   18------19------20  |\t|   _-------x-------_   |\n\
        |           |           |\t|           |           |\n\
        |           |           |\t|           |           |\n\
        21----------22----------23\to-----------_-----------_\n\
\n\
Computer's move                         : 21 22\n\
\n\n\
        0-----------1-----------2\t_-----------x-----------_\n\
        |           |           |\t|           |           |\n\
        |           |           |\t|           |           |\n\
        |   3-------4-------5   |\t|   _-------_-------_   |\n\
        |   |       |       |   |\t|   |       |       |   |\n\
        |   |   6---7---8   |   |\t|   |   _---x---_   |   |\n\
        |   |   |       |   |   |\t|   |   |       |   |   |\n\
LEGEND: 9---10--11      12--13--14\t_---_---_       _---_---o\tTurn: x\n\
        |   |   |       |   |   |\t|   |   |       |   |   |\n\
        |   |   15--16--17  |   |\t|   |   _---_---o   |   |\n\
        |   |       |       |   |\t|   |       |       |   |\n\
        |   18------19------20  |\t|   _-------x-------_   |\n\
        |           |           |\t|           |           |\n\
        |           |           |\t|           |           |\n\
        21----------22----------23\t_-----------o-----------_\n\
\n\
Player's move [(u)ndo/0-23 0-23 (0-23)] : { 19 16 }\n\
\n\n\
        0-----------1-----------2\t_-----------x-----------_\n\
        |           |           |\t|           |           |\n\
        |           |           |\t|           |           |\n\
        |   3-------4-------5   |\t|   _-------_-------_   |\n\
        |   |       |       |   |\t|   |       |       |   |\n\
        |   |   6---7---8   |   |\t|   |   _---x---_   |   |\n\
        |   |   |       |   |   |\t|   |   |       |   |   |\n\
LEGEND: 9---10--11      12--13--14\t_---_---_       _---_---o\tTurn: o\n\
        |   |   |       |   |   |\t|   |   |       |   |   |\n\
        |   |   15--16--17  |   |\t|   |   _---x---o   |   |\n\
        |   |       |       |   |\t|   |       |       |   |\n\
        |   18------19------20  |\t|   _-------_-------_   |\n\
        |           |           |\t|           |           |\n\
        |           |           |\t|           |           |\n\
        21----------22----------23\t_-----------o-----------_\n\
\n\
Computer's move                         : 14 23\n\
\n\n\
";


STRING kHelpExampleWithFlying =
"\
        0-----------1-----------2\tx-----------_-----------o\n\
        |           |           |\t|           |           |\n\
        |           |           |\t|           |           |\n\
        |   3-------4-------5   |\t|   _-------_-------_   |\n\
        |   |       |       |   |\t|   |       |       |   |\n\
        |   |   6---7---8   |   |\t|   |   _---x---_   |   |\n\
        |   |   |       |   |   |\t|   |   |       |   |   |\n\
LEGEND: 9---10--11      12--13--14\t_---_---_       _---_---_\tTurn: x\n\
        |   |   |       |   |   |\t|   |   |       |   |   |\n\
        |   |   15--16--17  |   |\t|   |   _---o---_   |   |\n\
        |   |       |       |   |\t|   |       |       |   |\n\
        |   18------19------20  |\t|   _-------_-------_   |\n\
        |           |           |\t|           |           |\n\
        |           |           |\t|           |           |\n\
        21----------22----------23\to-----------_-----------x\n\
\n\
Player's move [(u)ndo/0-23 0-23 (0-23)] : { 23 1 }\n\
\n\n\
        0-----------1-----------2\tx-----------x-----------o\n\
        |           |           |\t|           |           |\n\
        |           |           |\t|           |           |\n\
        |   3-------4-------5   |\t|   _-------_-------_   |\n\
        |   |       |       |   |\t|   |       |       |   |\n\
        |   |   6---7---8   |   |\t|   |   _---x---_   |   |\n\
        |   |   |       |   |   |\t|   |   |       |   |   |\n\
LEGEND: 9---10--11      12--13--14\t_---_---_       _---_---_\tTurn: o\n\
        |   |   |       |   |   |\t|   |   |       |   |   |\n\
        |   |   15--16--17  |   |\t|   |   _---o---_   |   |\n\
        |   |       |       |   |\t|   |       |       |   |\n\
        |   18------19------20  |\t|   _-------_-------_   |\n\
        |           |           |\t|           |           |\n\
        |           |           |\t|           |           |\n\
        21----------22----------23\to-----------_-----------_\n\
\n\
Computer's move                         : 21 19\n\
\n\n\
        0-----------1-----------2\tx-----------x-----------o\n\
        |           |           |\t|           |           |\n\
        |           |           |\t|           |           |\n\
        |   3-------4-------5   |\t|   _-------_-------_   |\n\
        |   |       |       |   |\t|   |       |       |   |\n\
        |   |   6---7---8   |   |\t|   |   _---x---_   |   |\n\
        |   |   |       |   |   |\t|   |   |       |   |   |\n\
LEGEND: 9---10--11      12--13--14\t_---_---_       _---_---_\tTurn: x\n\
        |   |   |       |   |   |\t|   |   |       |   |   |\n\
        |   |   15--16--17  |   |\t|   |   _---o---_   |   |\n\
        |   |       |       |   |\t|   |       |       |   |\n\
        |   18------19------20  |\t|   _-------o-------_   |\n\
        |           |           |\t|           |           |\n\
        |           |           |\t|           |           |\n\
        21----------22----------23\t_-----------_-----------_\n\
\n\
Player's move [(u)ndo/0-23 0-23 (0-23)] : { 0 4 2 }\n\
\n\n\
        0-----------1-----------2\t_-----------x-----------_\n\
        |           |           |\t|           |           |\n\
        |           |           |\t|           |           |\n\
        |   3-------4-------5   |\t|   _-------x-------_   |\n\
        |   |       |       |   |\t|   |       |       |   |\n\
        |   |   6---7---8   |   |\t|   |   _---x---_   |   |\n\
        |   |   |       |   |   |\t|   |   |       |   |   |\n\
LEGEND: 9---10--11      12--13--14\t_---_---_       _---_---_\tTurn: o\n\
        |   |   |       |   |   |\t|   |   |       |   |   |\n\
        |   |   15--16--17  |   |\t|   |   _---o---_   |   |\n\
        |   |       |       |   |\t|   |       |       |   |\n\
        |   18------19------20  |\t|   _-------o-------_   |\n\
        |           |           |\t|           |           |\n\
        |           |           |\t|           |           |\n\
        21----------22----------23\t_-----------_-----------_\n\
\n\
Excellent! You won!\n\
";

/*************************************************************************
**
** Everything above here must be in every game file
**
**************************************************************************/

/*************************************************************************
**
** Every variable declared here is only used in this file (game-specific)
**
**************************************************************************/

/*************************************************************************
**
** Below is where you put your #define's and your global variables, structs
**
*************************************************************************/

BOOLEAN debug = FALSE;

#define BOARDSIZE 24
#define minx  2
#define maxx  3
#define mino  2
#define maxo  3
#define minb  BOARDSIZE - maxo - maxx
#define maxb  BOARDSIZE - mino - minx
#define toFly 3

typedef enum Pieces {
  blank, x, o
} blankox;
char gblankoxChar[] = { '_', 'x', 'o'};

// Game Options
BOOLEAN gFlying = FALSE; // Flying for 3rd Phase

/*************************************************************************
**
** Above is where you put your #define's and your global variables, structs
**
*************************************************************************/

/*
** Function Prototypes:
*/

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn); // not sure why..
POSITION hash(blankox *board, blankox turn);
blankox *unhash(int hash_val, blankox *dest);
void parse_board(char *c_board, blankox *b_board);
void unparse_board(blankox *b_board, char *c_board);
blankox whose_turn(int hash_val);
MOVE hash_move(int from, int to, int remove);
int from(MOVE the_move);
int to(MOVE the_move);
int remove_piece(MOVE the_move);
blankox opponent(blankox player);
BOOLEAN can_be_taken(POSITION the_position, int slot);
BOOLEAN closes_mill(POSITION position, int raw_move);
BOOLEAN check_mill(blankox *board, int slot);
BOOLEAN three_in_a_row(blankox *board, int slot1, int slot2, int slot3, int slot);
blankox parse_char(char c);
char unparse_blankox(blankox b);
BOOLEAN closes_mill_move(MOVE the_move);
int count_mills(POSITION position, blankox player);
BOOLEAN all_mills(blankox *board, int slot);
int find_pieces(blankox *board, blankox piece, int *pieces);
int find_adj_pieces(blankox *board, int slot, blankox piece, int *pieces);
int find_adjacent(int slot, int *slots);
int count_pieces(blankox *board, blankox piece);
BOOLEAN full_board(POSITION position);
void copy_bboard(blankox *from, blankox *to);
void copy_cboard(char *from, char *to);

BOOLEAN trapped(blankox[], blankox, int);

// GameSpecificMenu
void setFlyingText();

// Solving
POSITIONLIST *GenerateParents (POSITION position);
POSITIONLIST *AppendFormedMill(blankox *board, int slot, POSITIONLIST *plist) ;
POSITIONLIST *AppendNeutralMove(blankox *board, int slot, POSITIONLIST *plist);
POSITIONLIST *AppendAllMoves(POSITION position, MOVELIST *ml, POSITIONLIST *pl);

// Debugging
void debugBoard(blankox *bboard, char *cboard);
void debugPosition(POSITION h);
void debugBBoard(blankox *bboard);
void debugCBoard(char *cboard);
void debugMiniCBoard(char *cboard);
void debugMiniBBoard(blankox *bboard);


/************************************************************************
**
** NAME:        InitializeGame
**
** DESCRIPTION: Initialize the gDatabase, a global variable. and the other
**              local variables.
** 
************************************************************************/

void InitializeGame()
{
  int b_size = BOARDSIZE;
  int pminmax[] = {gblankoxChar[2], mino, maxo, gblankoxChar[1], minx, maxx, gblankoxChar[0], minb, maxb, -1};
  //set mino, mninx to be 0


  gNumberOfPositions = generic_hash_init(b_size, pminmax, NULL);
  //printf("numPos: %d\n", gNumberOfPositions);
  
  setFlyingText();

  //gSolver = loopyup_DetermineValue;
}

/************************************************************************
**
** NAME:        DebugMenu
**
** DESCRIPTION: Menu used to debub internal problems. Does nothing if
**              kDebugMenu == FALSE
** 
************************************************************************/

void DebugMenu()
{
}

/************************************************************************
**
** NAME:        GameSpecificMenu
**
** DESCRIPTION: Menu used to change game-specific parmeters, such as
**              the side of the board in an nxn Nim board, etc. Does
**              nothing if kGameSpecificMenu == FALSE
** 
************************************************************************/

void GameSpecificMenu() 
{
  char GetMyChar();
  POSITION GetInitialPosition();
  
  do {
    printf("?\n\t----- Game-specific options for %s -----\n\n", kGameName);
    
    printf("\tCurrent Initial Position:\n");
    PrintPosition(gInitialPosition, gPlayerName[kPlayerOneTurn], kHumansTurn);
    
    printf("\n");
    printf("\ti)\tChoose the (I)nitial position\n");
    printf("\tf)\tToggle (F)lying from %s to %s\n", 
	   gFlying ? "ON" : "OFF",
	   !gFlying ? "ON" : "OFF"); 
    
    printf("\n\n\tb)\t(B)ack = Return to previous activity.\n");
    printf("\n\nSelect an option: ");
    
    switch(GetMyChar()) {
    case 'F': case 'f':
      gFlying = !gFlying;
      setFlyingText();
      break;
    case 'Q': case 'q':
      ExitStageRight();
    case 'H': case 'h':
      HelpMenus();
      break;
    case 'I': case 'i':
      gInitialPosition = GetInitialPosition();
      break;
    case 'B': case 'b':
      return;
    default:
      printf("\nSorry, I don't know that option. Try another.\n");
      HitAnyKeyToContinue();
      break;
    }
  } while(TRUE);
	  
}

// Changes kHelpOnYourTurn depending on gFlying
void setFlyingText()
{
  int newHelpSize = strlen(kHelpOnYourTurn0) + strlen(kHelpWithFlying) + 1;
  char* newHelp = NULL;

  if (kHelpOnYourTurn != NULL) {
   free(kHelpOnYourTurn);
  }
 
  newHelp = (char*)malloc(newHelpSize*sizeof(char));

  // debug 
  if (debug) {
	 printf("gFlying = %d\n", gFlying);
	 printf("The newHelpSize is %d\n", newHelpSize);
  }

  // change kHelpOnYourTurn if gFlying is TRUE
  if (gFlying && (newHelp != NULL)) {
    strcpy(newHelp, kHelpOnYourTurn0);
    
    // debug
    if (debug) {
      printf("newHelp(%d) after strcpy: %s\n\n",strlen(newHelp), newHelp);
    }
    
    strcat(newHelp, kHelpWithFlying);
    
    //debug
    if (debug) {
      printf("newHelp(%d) after strcat: %s\n\n",strlen(newHelp), newHelp);
    }
    
    kHelpOnYourTurn = newHelp;
  } else {
    kHelpOnYourTurn = newHelp;
    strcpy(newHelp, kHelpOnYourTurn0);
  }

  // debug
  if (debug) {
    printf("gFlying = %d\n", gFlying);
    printf("kHelpOnYourTurn(%d):%s\n",strlen(kHelpOnYourTurn),kHelpOnYourTurn);
  }
}

/************************************************************************
**
** NAME:        SetTclCGameSpecificOptions
**
** DESCRIPTION: Set the C game-specific options (called from Tcl)
**              Ignore if you don't care about Tcl for now.
** 
************************************************************************/

void SetTclCGameSpecificOptions(theOptions)
     int theOptions[];
{
	
}

/************************************************************************
**
** NAME:        DoMove
**
** DESCRIPTION: Apply the move to the position.
** 
** INPUTS:      POSITION thePosition : The old position
**              MOVE     theMove     : The move to apply.
**
** OUTPUTS:     (POSITION) : The position that results after the move.
**
** CALLS:       Hash ()
**              Unhash ()
**	            LIST OTHER CALLS HERE
*************************************************************************/
POSITION DoMove(thePosition, theMove)
     POSITION thePosition;
     MOVE theMove;
{
 
  //debug
  char cboard[BOARDSIZE];

  blankox board [BOARDSIZE];  
  int from_slot = from(theMove);
  int to_slot = to(theMove);
  int remove_slot = remove_piece(theMove);

  
  //debug
  if (debug) {
    printf("The move doMove got is: %d\n", theMove);
    printf("In doMove, the from, to, remove are: %d %d %d\n", from_slot, to_slot, remove_slot);
  }
 
  unhash(thePosition, board);
  //debug
  if (debug) {
    printf("thePosition is: %ld\n", thePosition);
    printf("It is this person's turn: %d\n", whoseMove(thePosition));
  }

  board[to_slot] = board[from_slot];
  board[from_slot] = blank;
  board[remove_slot] = blank; // if no piece is removed, remove = from

  //debug
  if (debug) {
    unparse_board(board, cboard);
    printf("The board after the move is: \n");
    debugBoard(board,cboard);
  }
  
  return hash(board, whose_turn(thePosition) == x ? o : x);
}

/************************************************************************
**
** NAME:        GetInitialPosition
**
** DESCRIPTION: Ask the user for an initial position for testing. Store
**              it in the space pointed to by initialPosition;
** 
** OUTPUTS:     POSITION initialPosition : The position to fill.
**
************************************************************************/

POSITION GetInitialPosition()
{
  int xOnBoard, oOnBoard, bOnBoard, i;
  signed char c;
  blankox board[BOARDSIZE];
  blankox turn;
  
  i = xOnBoard = oOnBoard = bOnBoard = 0;
  turn = blank; 
  
  printf("\n\n\t----- Get Initial Position -----\n");
  printf("\n\tPlease input the position to begin with.\n");
  printf("\nUse x for left player, o for right player, and _ for blank spaces\n");
  printf("Example:\n");
  printf("        x-----------_-----------o\n");
  printf("        |           |           |\n");
  printf("        |           |           |\n");
  printf("        |   _-------_-------_   |\n");
  printf("        |   |       |       |   |\n");
  printf("        |   |   _---x---_   |   |\n");
  printf("        |   |   |       |   |   |\n");
  printf("        _---_---_       _---_---_\n");
  printf("        |   |   |       |   |   |\n");
  printf("        |   |   _---o---_   |   |\n");
  printf("        |   |       |       |   |\n");
  printf("        |   _-------_-------_   |\n");
  printf("        |           |           |\n");
  printf("        |           |           |\n");
  printf("        o-----------_-----------x\n");
  printf("Is input as \n\tx _ o\n\t_ _ _\n\t_ x _ \n\t_ _ _ _ _ _\n\t_ o _ \n\t_ _ _\n\to _ x\n\nYour board:\n");

  getchar(); // dump a char 

  while (i < BOARDSIZE && (c = getchar()) != EOF) {
    if (c == 'x' || c == 'X')
      xOnBoard++;
    else if (c == 'o' || c == 'O' || c == '0')
      oOnBoard++;
    else if (c == '_' || c == '-' || c == 'b' || c == 'B')
      bOnBoard++;
    else {
      continue; // don't recognize the char, continue to the next
    }
    board[i++] = parse_char(c); // we recognize the char, add it to board
  }

  // hard coded sanity check for maxx vs maxo
  // should add a hash/unhash sanity check
  if (xOnBoard < minx || xOnBoard > maxx ||
      oOnBoard < mino || oOnBoard > maxo ||
      bOnBoard > maxb) {
    printf("\n Illegal Board Position Please Re-Enter\n");
    return GetInitialPosition();
  }

  // now get the turn
  getchar(); // dump another character
 
  printf("\nWhose turn is it? (x/o) ");
  while (turn == blank) {
    scanf("%c",&c);
    turn = parse_char(c);
  }
  
  return hash(board, turn);
}


/************************************************************************
**
** NAME:        PrintComputersMove
**
** DESCRIPTION: Nicely format the computers move.
** 
** INPUTS:      MOVE    computersMove : The computer's move. 
**              STRING  computersName : The computer's name. 
**
************************************************************************/

void PrintComputersMove(computersMove, computersName)
     MOVE computersMove;
     STRING computersName;
{
  printf("%8s's move              : from %d  to %d\n", computersName, from(computersMove), to(computersMove));
  if (closes_mill_move(computersMove))
      printf("\tand removes %d", remove_piece(computersMove));
}


/************************************************************************
**
** NAME:        Primitive
**
** DESCRIPTION: Return the value of a position if it fulfills certain
**              'primitive' constraints. If the current player only has
**              2 pieces remaining or has no possible moves, then this
**              position is a loss.  If the current player has more than
**              2 pieces and the opponent either has only 2 pieces or
**              cannot move, then this position is a win.  Otherwise,
**              this position is undecided.
** 
** INPUTS:      POSITION position : The position to inspect.
**
** OUTPUTS:     (VALUE) an enum which is oneof: (win,lose,tie,undecided)
**
** CALLS:       LIST FUNCTION CALLS
**              
**
************************************************************************/

VALUE Primitive ( POSITION h )
{
  blankox dest[BOARDSIZE];
  int numXs = 0;
  int numOs = 0;
  int numPlayer, minPlayer;
  int i;
  blankox turn = whose_turn(h);
  
  unhash(h, dest);

  for (i = 0; i < BOARDSIZE; i++) {
    if (dest[i] == x) {
      numXs++;
    }
    else if (dest[i] == o) {
      numOs++;
    }
  }

  //debug
  if (debug) {
    printf("Primitive is checking: \n");
    debugPosition(h);
    printf("And has found %dXs, %dOs.\n", numXs, numOs);
  }

  if (turn == o) {
    numPlayer = numOs;
    minPlayer = mino;
  }
  else {
    numPlayer = numXs;
    minPlayer = minx;
  }

  if (numPlayer == minPlayer)
    return (gStandardGame ? lose : win );
  else if (trapped(dest, turn, numPlayer))
    return (gStandardGame ? lose : win);
  else {
    return undecided;
  }
 
}


/************************************************************************
**
** NAME:        PrintPosition
**
** DESCRIPTION: Print the position in a pretty format, including the
**              prediction of the game's outcome.
** 
** INPUTS:      POSITION position   : The position to pretty print.
**              STRING   playerName : The name of the player.
**              BOOLEAN  usersTurn  : TRUE <==> it's a user's turn.
**
** CALLS:       Unhash()
**              GetPrediction()
**              LIST OTHER CALLS HERE
**
************************************************************************/

void PrintPosition(position, playerName, usersTurn)
     POSITION position;
     STRING playerName;
     BOOLEAN usersTurn;
{
  blankox board[BOARDSIZE];
  blankox turn;
  char c_board[BOARDSIZE];
  
  unhash(position, board);

  unparse_board(board, c_board);
  turn = whose_turn(position);
  
  
  printf("\n");
  printf("        0-----------1-----------2       %c-----------%c-----------%c\n", c_board[0], c_board[1], c_board[2] );
  printf("        |           |           |       |           |           |\n");
  printf("        |           |           |       |           |           |\n");
  printf("        |   3-------4-------5   |       |   %c-------%c-------%c   |\n", c_board[3], c_board[4], c_board[5] );
  printf("        |   |       |       |   |       |   |       |       |   |\n");
  printf("        |   |   6---7---8   |   |       |   |   %c---%c---%c   |   |\n", c_board[6], c_board[7], c_board[8] );
  printf("        |   |   |       |   |   |       |   |   |       |   |   |\n");
  printf("LEGEND: 9---10--11      12--13--14      %c---%c---%c       %c---%c---%c    Turn: %c\n", c_board[9], c_board[10], c_board[11], c_board[12], c_board[13], c_board[14], unparse_blankox(turn) );
  printf("        |   |   |       |   |   |       |   |   |       |   |   |\n");
  printf("        |   |   15--16--17  |   |       |   |   %c---%c---%c   |   |\n", c_board[15], c_board[16], c_board[17] );
  printf("        |   |       |       |   |       |   |       |       |   |\n");
  printf("        |   18------19------20  |       |   %c-------%c-------%c   |\n", c_board[18], c_board[19], c_board[20] );
  printf("        |           |           |       |           |           |\n");
  printf("        |           |           |       |           |           |\n");
  printf("        21----------22----------23      %c-----------%c-----------%c\n", c_board[21], c_board[22], c_board[23] );
  GetPrediction(position,playerName,usersTurn);
  /*
  0-----------1-----------2
  |           |           |
  |           |           |
  |   3-------4-------5   |
  |   |       |       |   |
  |   |   6---7---8   |   |
  |   |   |       |   |   |
  9---10--11      12--13--14
  |   |   |       |   |   |
  |   |   15--16--17  |   |
  |   |       |       |   |
  |   18------19------20  |
  |           |           |
  |           |           |
  21----------22----------23
  */
}

/************************************************************************
**
** NAME:        GenerateMoves
**
** DESCRIPTION: Create a linked list of every move that can be reached
**              from this position. Return a pointer to the head of the
**              linked list.
** 
** INPUTS:      POSITION position : The position to branch off of.
**
** OUTPUTS:     (MOVELIST *), a pointer that points to the first item  
**              in the linked list of moves that can be generated.
**
** CALLS:       GENERIC_PTR SafeMalloc(int)
**              LIST OTHER CALLS HERE
**
************************************************************************/

MOVELIST *GenerateMoves(POSITION position)         
{
  int i, j, k, x_count, o_count, blank_count;
  int player_count, opponent_count, raw_move;
  MOVELIST *CreateMovelistNode(), *head = NULL;
  MOVELIST *temp_head = NULL;
  blankox dest[BOARDSIZE];
  int x_pieces[maxx];
  int o_pieces[maxo];
  int x_adjBlanks[maxx];
  int o_adjBlanks[maxo];
  int player_adj[maxx>maxo ? maxx : maxo][4];
  int *player_adjBlanks;
  int *player_pieces;
  int *opponent_pieces;
  int blanks[BOARDSIZE];
  int adj[4];
  int adjs;
  int counter;
  blankox turn = whose_turn(position);
  x_count = o_count = blank_count = 0;
  
  unhash(position, dest);
  
  for (i = 0; i < BOARDSIZE; i++)
    {
      if (dest[i] == x)
	x_pieces[x_count++] = i;
      else if (dest[i] == o)
	o_pieces[o_count++] = i;
    }
  
  if (turn == x)
    {
      player_pieces = x_pieces;
      player_count = x_count;
      player_adjBlanks = x_adjBlanks;
      opponent_count = o_count;
      opponent_pieces = o_pieces;
    }
  else
    {
      player_pieces = o_pieces;
      player_count = o_count;
      player_adjBlanks = o_adjBlanks;
      opponent_count = x_count;
      opponent_pieces = x_pieces;
    }
  
  if (gFlying && player_count<=toFly) {
    blank_count = find_pieces(dest, blank, blanks);
  }
  else {
    for (i=0; i<player_count; i++) {
      player_adjBlanks[i] = 0;
      adjs = find_adjacent(player_pieces[i], adj);
      for (j=0; j<adjs; j++) {
	if (dest[adj[j]] == blank) {
	  player_adj[i][player_adjBlanks[i]++] = adj[j];
	}
      }
    }
  }
  
  for (i = 0; i < player_count; i++)
    {
      if(gFlying && player_count<=toFly) {
	counter = blank_count;
      }
      else {
	counter = player_adjBlanks[i];
      }
      for (j = 0; j < counter; j++)
	{
	  if (gFlying && player_count<=toFly) {
	    raw_move = (player_pieces[i] * BOARDSIZE * BOARDSIZE) +
	      (blanks[j] * BOARDSIZE) + player_pieces[i];
	  }
	  else {
	    raw_move = (player_pieces[i] * BOARDSIZE * BOARDSIZE) +
	      (player_adj[i][j] * BOARDSIZE) + player_pieces[i];
	  }
	  
	  //debug
	  if (debug) {
	    printf ("the raw_move is: %d\n", raw_move);
	  }
	  
	  if (closes_mill(position, raw_move))
	    {
	      for (k = 0; k < opponent_count; k++)
		if (can_be_taken(position, opponent_pieces[k]))
		  head = CreateMovelistNode((raw_move + opponent_pieces[k]-player_pieces[i]) , head);
	    }
	  else
	    head = CreateMovelistNode(raw_move, head); 
	}
    }
  
  return head;
}

 
/************************************************************************
**
** NAME:        GetAndPrintPlayersMove
**
** DESCRIPTION: This finds out if the player wanted an undo or abort or not.
**              If so, return Undo or Abort and don't change theMove.
**              Otherwise get the new theMove and fill the pointer up.
** 
** INPUTS:      POSITION *thePosition : The position the user is at. 
**              MOVE *theMove         : The move to fill with user's move. 
**              STRING playerName     : The name of the player whose turn it is
**
** OUTPUTS:     USERINPUT             : Oneof( Undo, Abort, Continue )
**
** CALLS:       ValidMove(MOVE, POSITION)
**              BOOLEAN PrintPossibleMoves(POSITION) ...Always True!
**
************************************************************************/

USERINPUT GetAndPrintPlayersMove(thePosition, theMove, playerName)
     POSITION thePosition;
     MOVE *theMove;
     STRING playerName;
{
  BOOLEAN ValidMove();
  USERINPUT ret, HandleDefaultTextInput();
  
  do {
    printf("%8s's move [(u)ndo/0-23 0-23 (0-23)] :  ", playerName);

    ret = HandleDefaultTextInput(thePosition, theMove, playerName);
    if(ret != Continue)
      return(ret);
  }
  while (TRUE);

  return (Continue);
}

/************************************************************************
**
** NAME:        ValidTextInput
**
** DESCRIPTION: Return TRUE iff the string input is of the right 'form'.
**              For example, if the user is allowed to select one slot
**              from the numbers 1-9, and the user chooses 0, it's not
**              valid, but anything from 1-9 IS, regardless if the slot
**              is filled or not. Whether the slot is filled is left up
**              to another routine.
** 
** INPUTS:      STRING input : The string input the user typed.
**
** OUTPUTS:     BOOLEAN : TRUE if the input is a valid text input.
**
************************************************************************/

BOOLEAN ValidTextInput(input)
     STRING input;
{
  // we could bulletproof this a lot more
  
  int moveFrom, moveTo, moveRemove;

  BOOLEAN hasSpace, has2Space;
  STRING afterSpace;
  STRING after2Space;
  
  //debug
  if (debug) {
    printf("ValidTextInput evaluating: %s \n", input);
  }

  moveFrom = atoi(input);

  //debug
  if (debug) {
    printf("moveFrom of input: %d", moveFrom);
  }
  
  hasSpace = index(input, ' ') != NULL;
  
  
  if (hasSpace) {
    afterSpace = index(input, ' ');
    //debug
    if (debug) {
      printf("index of the space: %d", afterSpace);
    }
  } else {
    return FALSE;
  }
    
  if (moveFrom < 0 || moveFrom >= BOARDSIZE){
    //debug
    if (debug) {
      printf("move check fails b/c of Moveto");
    }
    return FALSE;
  }
  
  moveTo = atoi(afterSpace);

  //debug
  if (debug) {
    printf("moveTo of input: %d", moveTo);
  }
  
  if (moveTo < 0 || moveTo >= BOARDSIZE) {
    // debug
    if (debug) {
      printf("move check fails b/c of Moveto");
    }
  	return FALSE;
  }
  
  has2Space = index(++afterSpace, ' ') != NULL;
  if (has2Space) {
    after2Space = index(afterSpace, ' ');
    moveRemove = atoi(after2Space);
    if (moveRemove < 0 || moveRemove >=BOARDSIZE) {
      // debug
    if (debug) {
      printf("move check fails b/c of MoveRemove");
    }
      return FALSE;
    }
    else 
      return TRUE;
  } else {
    return TRUE;
  }

  return FALSE; // should never be reached
}

/************************************************************************
**
** NAME:        ConvertTextInputToMove
**
** DESCRIPTION: Convert the string input to the internal move representation.
**              No checking if the input is valid is needed as it has
**              already been checked!
** 
** INPUTS:      STRING input : The string input the user typed.
**
** OUTPUTS:     MOVE : The move corresponding to the user's input.
**
************************************************************************/

MOVE ConvertTextInputToMove(input)
     STRING input;
{
  int from, to, remove;
  STRING afterSpace;
  STRING after2Space;
  BOOLEAN hasSpace, has2Space;
  
  from = atoi(input);
  remove = from;
  hasSpace = index(input, ' ') != NULL;
  
  if (hasSpace) {
    afterSpace = index(input, ' ');
    to = atoi(afterSpace);

    has2Space = index(++afterSpace, ' ') != NULL;

    if (has2Space) {
      after2Space = index(afterSpace, ' ');
      remove = atoi(after2Space);
    } 
    
  } else {
    return 0; // Should be a bad else
  }
      
  //debug
  if (debug) {
    printf ("in InputHandler, the from, to, remove is: %d %d %d\n", from, to, remove);
  }

  return hash_move(from, to, remove);
}

/************************************************************************
**
** NAME:        PrintMove
**
** DESCRIPTION: Print the move in a nice format.
** 
** INPUTS:      MOVE *theMove         : The move to print. 
**
************************************************************************/

void PrintMove(theMove)
     MOVE theMove;
{
      printf("[%d %d", from(theMove), to(theMove));
      if (closes_mill_move(theMove))
		  printf(" %d", remove_piece(theMove));
      printf("]");
}

/************************************************************************
**
** NAME:        NumberOfOptions
**
** DESCRIPTION: Calculates and returns the number of option combinations
**		there are with all the game variations you program.
**
** OUTPUTS:     int : the number of option combination there are.
**
************************************************************************/

int NumberOfOptions()
{
  return 2*2;
}

/************************************************************************
**
** NAME:        getOption
**
** DESCRIPTION: A hash function to keep track of all the game variants.
**		Should return a different number for each set of
**				variants.
**
** OUTPUTS:     int : the number representation of the options.
**
************************************************************************/

int getOption()
{
  int option = 1;
  option += (gStandardGame ? 0 : 1);
  option += 2* (gFlying ? 0 : 1);
  
  return option;
}

/************************************************************************
**
** NAME:        setOption
**
** DESCRIPTION: The corresponding unhash for the game variants.
**		Should take the input and set all the appropriate
**		variants.
**
** INPUT:     int : the number representation of the options.
**
************************************************************************/
void setOption(int option)
{
  option--;

  gStandardGame = (option%2==0);
  gFlying = (option/2%2==0);
}

/************************************************************************
*************************************************************************
**         Everything BELOW THESE LINES IS LOCAL TO THIS FILE
*************************************************************************
************************************************************************/

/************************************************************************
** This is where you can put any helper functions, including your
** hash and unhash functions if you are not using one of the existing
** ones.
************************************************************************/

POSITION hash(blankox *b_board, blankox turn)
{

  char c_board[BOARDSIZE];
  int raw_hash;
  int generic_hash();
  int player;

  if (turn == x) 
    player = 1;
  else 
    player = 2;

  unparse_board(b_board, c_board);
  
  return generic_hash(c_board, player); 
}

blankox *unhash(int hash_val, blankox *b_board)
{
  char c_board [BOARDSIZE];
  
  generic_unhash(hash_val, c_board);
  
  //debug
  if (debug) {
    printf("The hash value being unhashed is: %d\n", hash_val);
  }
  
  parse_board(c_board, b_board);

  //debug
  if (debug) {
    printf("unhash found: \n");
    debugBoard(b_board,c_board);
  }

  return b_board;
}


// char to blankox conversion
void parse_board(char *c_board, blankox *b_board)
{
  int i;
  for (i = 0; i < BOARDSIZE; i++) 
    { 
      if (c_board[i] == 'o' || c_board[i] == 'O')
	b_board[i] = o;
      else if (c_board[i] == 'x' || c_board[i] == 'X')
	b_board[i] = x;
      else if (c_board[i] == '_' || c_board[i] == '-' ||
	       c_board[i] == 'b' || c_board[i] == 'B')
	b_board[i] = blank;
    }
}

//char to blankox
blankox parse_char(char c) {
  if (c == 'x' || c == 'X')
    return x;
  else if (c == 'o' || c == '0' || c == 'O')
    return o;
  else if (c == '-' || c == '_' || c == 'b' || c == 'B')
    return blank;
  else
    return blank; // fix this so that it's a badelse
}


//blankox to char
char unparse_blankox(blankox b)
{
  return gblankoxChar[b];
}

//blankox to char conversion
void unparse_board(blankox *b_board, char *c_board)
{
  int i;
  for (i = 0; i < BOARDSIZE; i++)
    {
      c_board[i] = unparse_blankox(b_board[i]);
    }
}

blankox whose_turn(int hash_val)
{
  if (whoseMove (hash_val) == 1) 
    return x;
  else return o;
}

blankox opponent (blankox player)
{
  return (player == o ? x : o);
}

// Given bboard, int array
// Return number of pieces and array of each slot containing those pieces
int find_pieces(blankox *board, blankox piece, int *pieces)
{
  int i;
  int num = 0;

  for (i = 0; i < BOARDSIZE; i++) {
    if (board[i] == piece)
      pieces[num++] = i;
  }

  return num;
}

// Given bboard, slot, int array
// Return number of adjacent pieces and array of slots containing those pieces
int find_adj_pieces(blankox *board, int slot, blankox piece, int *pieces)
{
  int i;
  int adjs[4];
  int num = find_adjacent(slot, adjs);
  int pieceCount = 0;

  for (i = 0; i < num; i++) {
	 if (board[adjs[i]] == piece) {
	   pieces[pieceCount++] = adjs[i];
	 }
  }

  return pieceCount;
}

// Given slot, int array
// Return number of adjacent slots, array of those slot numbers
int find_adjacent(int slot, int *slots)
{
  // multiples of 3 (0, 3, 6, 9, 12, 15, 18, 21) are left-most edge
  // 0, 1, 2, 3, 5, 6, 8 are top-most
  // 7, 15, 16, 17, 18, 19, 20, 21, 22, 23 are bottom-most
  // 2, 5, 8, 11, 14, 17, 20, 23 are right-most (each differs by 3)
  // 4, 10, 13, 19 are centered (have adjacent pieces in all 4 directions)

  int num = 0;

  switch (slot) {
  case 0: 
	 // num = 2;
    slots[num++] = slot + 1;
    slots[num++] = slot + 9;
    break;
  case 3:
	 // num = 2;
	 slots[num++] = slot + 1;
	 slots[num++] = slot + 7;
	 break;
  case 6:
	 // num = 2;
	 slots[num++] = slot + 1;
	 slots[num++] = slot + 5;
	 break;
  case 2:
	 // num = 2;
	 slots[num++] = slot - 1;
	 slots[num++] = slot + 12;
	 break;
  case 5:
	 // num = 2;
	 slots[num++] = slot - 1;
	 slots[num++] = slot + 8;
	 break;
  case 8:
	 // num = 2;
	 slots[num++] = slot - 1;
	 slots[num++] = slot + 4;
	 break;
  case 21:
	 // num = 2;
	 slots[num++] = slot + 1;
	 slots[num++] = slot - 12;
	 break;
  case 18:
	 // num = 2;
	 slots[num++] = slot + 1;
	 slots[num++] = slot - 8;
	 break;
  case 15:
	 // num = 2;
	 slots[num++] = slot + 1;
	 slots[num++] = slot - 4;
	 break;
  case 23:
	 // num = 2;
	 slots[num++] = slot - 1;
	 slots[num++] = slot - 9;
	 break;
  case 20:
	 // num = 2;
	 slots[num++] = slot - 1;
	 slots[num++] = slot - 7;
	 break;
  case 17:
	 // num = 2;
	 slots[num++] = slot - 1;
	 slots[num++] = slot - 5;
	 break;
  case 1:
	 // num = 2;
    slots[num++] = slot - 1;
    slots[num++] = slot + 1;
    slots[num++] = slot + 3;
    break;
  case 16:
	 // num = 2;
	 slots[num++] = slot - 1;
	 slots[num++] = slot + 1;
	 slots[num++] = slot + 3;
	 break;
  case 7:
	 // num = 2;
	 slots[num++] = slot - 1;
	 slots[num++] = slot + 1;
	 slots[num++] = slot - 3;
	 break;
  case 22:
	 // num = 3;
	 slots[num++] = slot - 1;
	 slots[num++] = slot + 1;
	 slots[num++] = slot - 3;
	 break;
  case 9:
	 // num = 3;
	 slots[num++] = slot + 1;
	 slots[num++] = slot + 12;
	 slots[num++] = slot - 9;
	 break;
  case 12:
	 // num = 3;
	 slots[num++] = slot + 1;
	 slots[num++] = slot + 5;
	 slots[num++] = slot - 4;
	 break;
  case 11:
	 // num = 3;
	 slots[num++] = slot - 1;
	 slots[num++] = slot + 4;
	 slots[num++] = slot - 5;
	 break;
  case 14:
	 // num = 3;
	 slots[num++] = slot - 1;
	 slots[num++] = slot + 9;
	 slots[num++] = slot - 12;
	 break;
  case 4: case 19:
	 // num = 4;
	 slots[num++] = slot + 1;
	 slots[num++] = slot - 1;
	 slots[num++] = slot + 3;
	 slots[num++] = slot - 3;
	 break;
  case 10:
	 // num = 4;
	 slots[num++] = slot + 1;
	 slots[num++] = slot - 1;
	 slots[num++] = slot + 8;
	 slots[num++] = slot - 7;
	 break;
  case 13:
	 // num = 4;
	 slots[num++] = slot + 1;
	 slots[num++] = slot - 1;
	 slots[num++] = slot + 7;
	 slots[num++] = slot - 8;
	 break;
  default:
	 num = 0;
	 slots[0] = -1;
    break;
  }

  return num;
}
  /*
  0-----------1-----------2
  |           |           |
  |           |           |
  |   3-------4-------5   |
  |   |       |       |   |
  |   |   6---7---8   |   |
  |   |   |       |   |   |
  9---10--11      12--13--14
  |   |   |       |   |   |
  |   |   15--16--17  |   |
  |   |       |       |   |
  |   18------19------20  |
  |           |           |
  |           |           |
  21----------22----------23
  */

// Given bboard, piece
// Return the number of instances of piece on the board
int count_pieces(blankox *board, blankox piece)
{
  int i;
  int num = 0;

  for (i = 0; i < BOARDSIZE; i++) {
	 if (board[i] == piece) {
		num++;
	 }
  }

  return num;
}

// Given position
// Return true if this position is a full board, false otherwise
BOOLEAN full_board(POSITION position)
{
  blankox board[BOARDSIZE];

  unhash(position, board);

  return (count_pieces(board, x) == maxx) && (count_pieces(board, o) == maxo)
	 && (count_pieces(board, blank) == maxb);
}

// Given blankox *FROM, blankox *TO
// Copy FROM to TO
void copy_bboard(blankox *from, blankox *to)
{
  int i;
  for (i = 0; i < BOARDSIZE; i++) {
	 to[i] = from[i];
  }
}

// Given char *FROM, char*TO
// Copy FROM to TO
void copy_cboard(char *from, char *to)
{
  int i;
  for (i = 0; i < BOARDSIZE; i++) {
	 to[i] = from[i];
  }
}

/******************** MOVE abstractions ********************/

// if there is no removal, then from == remove
MOVE hash_move(int from, int to, int remove)
{
  return ((from * BOARDSIZE * BOARDSIZE) + (to * BOARDSIZE) + remove);
}

int from(MOVE move)
{
  return (move / (BOARDSIZE * BOARDSIZE));
}

int to(MOVE move)
{
  return ((move % (BOARDSIZE * BOARDSIZE)) / (BOARDSIZE));
}

int remove_piece(MOVE the_move)
{
  return (the_move % BOARDSIZE);
}

// Given POSITION, slot
// Return true if piece at slot can be taken, false otherwise
BOOLEAN can_be_taken(POSITION position, int slot)
{
  blankox board[BOARDSIZE];
  unhash(position, board);
  blankox piece = board[slot];
  BOOLEAN allMills;

  /* According to the rules, a piece can be taken if it is not in a mill
	  or if the opponent only has mills */

  if (board[slot] == blank) {
	 return FALSE;
  }

  // debug
  if (debug) {
	 printf("can_be_taken checking slot %d of this position:\n", slot);
	 debugPosition(position);
  }

  allMills = all_mills(board, slot);

  return (count_pieces(board, piece) < 4 ||
			 !check_mill(board, slot) ||
			 allMills);
} 


// Given position, player, count # of mills for player on board
int count_mills(POSITION position, blankox player)
{
  blankox board[BOARDSIZE];
  int i, mills;
  unhash(position, board);
  mills = 0;

  for (i = 0; i < BOARDSIZE; i++) {
    if (board[i] == player) {
      if (check_mill(board, i)) {
	mills++;
      }
    }
  }

  if (debug) {
    printf("At position %ld, count_mills found %d mill(s) for player %c.\n", position, mills, unparse_blankox(player));
  }

  return mills;
}
  
BOOLEAN closes_mill_move(MOVE the_move) {
  return from(the_move) != remove_piece(the_move);
}

// given old position and the next move
BOOLEAN closes_mill(POSITION position, int raw_move)
{
  blankox board[BOARDSIZE];
  unhash(DoMove(position, raw_move), board); //do the move onto board
  return check_mill(board, to(raw_move)); 
}

// given new board, slot
// return true if slot is member of mill
BOOLEAN check_mill(blankox *board, int slot)
{
  
  //debug
  if (debug) {
    printf("check_mill checking:\n");
    debugMiniBBoard(board);
  }

  return three_in_a_row(board, 0, 1,  2, slot) || // horizontals
    three_in_a_row(board, 3, 4, 5, slot) ||
    three_in_a_row(board, 6, 7, 8, slot) ||
    three_in_a_row(board, 9, 10, 11, slot) ||
    three_in_a_row(board, 12, 13, 14, slot) ||
    three_in_a_row(board, 15, 16, 17, slot) ||
    three_in_a_row(board, 18, 19, 20, slot) ||
    three_in_a_row(board, 21, 22, 23, slot) ||
    three_in_a_row(board, 0, 9, 21, slot) || // verticals
    three_in_a_row(board, 3, 10, 18, slot) ||
    three_in_a_row(board, 6, 11, 15, slot) ||
    three_in_a_row(board, 1, 4, 7, slot) ||
    three_in_a_row(board, 16, 19, 22, slot) ||
    three_in_a_row(board, 8, 12, 17, slot) ||
    three_in_a_row(board, 5, 13, 20, slot) ||
    three_in_a_row(board, 2, 14, 23, slot);
}

// given new board, slots to compare.  if slots all same, then it's a 3
BOOLEAN three_in_a_row(blankox *board, int slot1, int slot2, int slot3, int slot)
{
  return board[slot] == board[slot1] &&
    board[slot] == board[slot2] &&
    board[slot] == board[slot3] &&
    (slot == slot1 || slot == slot2 || slot == slot3);
}

// Given POSITION, slot
// Return true if at this position, the player at slot only has mills
BOOLEAN all_mills(blankox *board, int slot)
{
  blankox turn = board[slot];
  int pieces[turn == x ? maxx : maxo];
  int num, i;
  BOOLEAN allMills = TRUE;

  num = find_pieces(board, turn, pieces);

  for (i = 0; i < num; i++) {
	 if (!check_mill(board, pieces[i])) {
		allMills = FALSE;
	 }
  }

  // debug
  if (debug) {
	 printf("all_mills checked this board:\n");
	 debugMiniBBoard(board);
	 printf("and is returning %b", allMills);
  }

  return allMills;
}
  

/************ GenerateParents for Bryon's 9mm reverse solver ************/
// Given POSITION
// Return POSITIONLIST of Parents of give POSITION
POSITIONLIST *GenerateParents (POSITION position) 
{
  POSITIONLIST *head = NULL;
  blankox turn = opponent(whose_turn(position));
  blankox board[BOARDSIZE];
  int i;

  unhash(position, board);

  for (i = 0; i < BOARDSIZE; i++) {
    if (board[i] == turn) {
      if (check_mill(board, i)) {
		  head = AppendFormedMill(board, i, head); // piece formed a mill
      } else {
		  head = AppendNeutralMove(board, i, head); // piece didnt form mill
      }
    }
  }

  return head;
}

// Given the current board, slot of interest, POSITIONLIST
// Append POSITIONLIST of Parents involving slot (with milling)
POSITIONLIST *AppendFormedMill (blankox *board, int slot, POSITIONLIST *plist) 
{
  blankox thisTurn = board[slot];
  blankox prevTurn = opponent(thisTurn);
  int i, j, k;
  int numBlanks, prevSlot, prevBlank;
  int blanks[maxb];
  int allBlanks[maxb];
  int numAllBlanks = find_pieces(board, blank, allBlanks);
  int countO, countX;

  if (gFlying) {
	 numBlanks = find_pieces(board, blank, blanks);
  } else {
	 numBlanks = find_adj_pieces(board, slot, blank, blanks);
  }

  for (i = 0; i < numBlanks; i++) {
	 board[slot] = blank;  // remove most recent piece
	 prevSlot = blanks[i];
	 board[prevSlot] = thisTurn; // replace most recent piece
	 for (j = 0; j < numAllBlanks; j++) {
		prevBlank = allBlanks[j];
		if (prevBlank != slot && prevBlank != prevSlot) {
		  board[prevBlank] = prevTurn;
		  
		  /* count pieces */
		  countX = countO = 0;
		  for (k=0; k<BOARDSIZE; k++) {
		    if (board[k]==x) countX++;
		    else if (board[k]==o) countO++;
		  }
		  if (countX<=3 && countO<=3) {
		    plist = StorePositionInList(hash(board, thisTurn), plist);
		  }
		  /* revert removed piece */
		  board[prevBlank] = blank;
		}
	 }
	 /* revert moved piece */
	 board[prevSlot] = blank;
	 board[slot] = thisTurn;
  }
  
  return plist;
}

// Given the current board, slot of interest, POSITIONLIST
// Append POSITIONLIST of Parents involving slot (without milling)
POSITIONLIST *AppendNeutralMove(blankox *board, int slot, POSITIONLIST *plist) 
{
  blankox thisTurn = board[slot];
  int blanks[maxb];
  int numBlanks, i;
  int blankSlot;

  if (gFlying) {
    numBlanks = find_pieces(board, blank, blanks);
  } else {
    numBlanks = find_adj_pieces(board, slot, blank, blanks);
  }
  for (i = 0; i < numBlanks; i++) {
    /* transform position */
    int blankSlot = blanks[i];
    board[blankSlot] = board[slot];
    board[slot] = blank; 

    /* store position */
    plist = StorePositionInList(hash(board, thisTurn), plist); 

    /* revert position */
    board[slot] = board[blankSlot];
    board[blankSlot] = blank;
  }

  return plist;
}

// Given POSITION, MOVELIST, POSITIONLIST
// Return the POSITIONLIST with the resoution of moves in MOVELIST appended
//
// For each move in the MOVELIST, do that move and append the POSITION
//  onto the POSITIONLIST
POSITIONLIST *AppendAllMoves(POSITION position, MOVELIST *ml, POSITIONLIST *pl)
{
  POSITION tempPosition;

  while (ml != NULL) {
	 tempPosition = DoMove(position, ml->move);
	 pl = StorePositionInList(tempPosition, pl);
	 ml = ml->next;
  }

  return pl;
}

BOOLEAN trapped(blankox board[], blankox turn, int playerCount) {
  int i, j;
  int adj[4];
  int adjs;

  if (gFlying && playerCount<=toFly) return FALSE;

  for (i=0; i<BOARDSIZE; i++) {
    if (board[i] == turn) {
      adjs = find_adjacent(i, adj);
      for (j=0; j<adjs; j++) {
	if (board[adj[j]] == blank)
	  return FALSE;
      }
    }
  }
  return TRUE;
}
  
  

/* typedef struct positionlist_item */
/* { */
/* 	POSITION position; */
/* 	struct positionlist_item *next; */
/* } */
/* POSITIONLIST, FRnode; */

/* POSITIONLIST* StorePositionInList(POSITION thePosition, POSITIONLIST* thePositionList); */
/* POSITIONLIST* CopyPositionlist(POSITIONLIST* thePositionlist); */



/******************** Some Debugging Functions ********************/

//Given b_board, print b_board and c_board
void debugBBoard(blankox *bboard)
{
  char cboard[BOARDSIZE];

  unparse_board(bboard, cboard);
    
  debugBoard(bboard, cboard);
  
}
 
//Given c_board, print b_board and c_board
void debugCBoard(char *cboard)
{
  blankox bboard[BOARDSIZE];

  parse_board(cboard, bboard);

  debugBoard(bboard, cboard);
}

//Given bboard, print a mini-cboard
void debugMiniBBoard(blankox *bboard)
{
  char cboard[BOARDSIZE];

  unparse_board(bboard, cboard);

  debugMiniCBoard(cboard);

}

//Given cboard, print a mini-board
void debugMiniCBoard(char *cboard)
{
  printf("%c   %c   %c\t00       01       02\n", cboard[0], cboard[1], cboard[2]);
  printf("  %c %c %c  \t   03    04    05   \n", cboard[3], cboard[4], cboard[5]);
  printf("   %c%c%c   \t      06 07 08       \n", cboard[6], cboard[7], cboard[8]);
  printf("%c%c%c   %c%c%c\t09 10 11    12 13 14\n", cboard[9], cboard[10], cboard[11], cboard[12], cboard[13], cboard[14]);
  printf("   %c%c%c   \t      15 16 17      \n", cboard[15], cboard[16], cboard[17]);
  printf("  %c %c %c  \t   18    19    20   \n", cboard[18], cboard[19], cboard[20]);
  printf("%c   %c   %c\t21       22      23\n", cboard[21], cboard[22], cboard[23]);
}


//Given the b_board and c_board, print them
void debugBoard(blankox *bboard, char *cboard)
{
  int i;

  for (i = 0; i < BOARDSIZE; i++)
    printf("%d", bboard[i]);
  printf("\n");
  for (i = 0; i < BOARDSIZE; i++)
    printf("%c", cboard[i]);
  printf("\n");
  debugMiniBBoard(bboard);

}

//Given the position, print it as a b_board and c_board
void debugPosition(POSITION h)
{
  blankox bboard[BOARDSIZE];
  char cboard[BOARDSIZE];

  unhash(h, bboard);
  unparse_board(bboard, cboard);
  printf("Current position = %ld, ", h);
  printf("%c player's turn.\n", gblankoxChar[whose_turn(h)]);
  printf("Current board = \n");
  debugBoard(bboard, cboard);

}



//$Log: not supported by cvs2svn $
//Revision 1.68  2004/05/05 23:59:26  boundsyco
//added text to tell from whence the data is comming from. changed m9mm to use a working solver. -SL
//
//Revision 1.67  2004/05/05 23:26:04  ogren
//started kHelpExample -Elmer.
//
//Revision 1.66  2004/05/05 22:58:46  ogren
//GetInitialPosition now restricts piece placement to be between min and max, inclusive. -Elmer
//
//Revision 1.65  2004/05/05 22:40:16  ogren
//turned off debugging -Elmer
//
//Revision 1.64  2004/05/05 22:38:55  ogren
//fixed mutating help text, with expertise of Bryon -Elmer
//
//Revision 1.63  2004/05/05 22:14:14  bryonr
//Fixed GenerateParents and solver
//
//Revision 1.62  2004/05/05 10:45:36  bryonr
//Added loopy bottom-up solver for use with Nine Men's Morris.
//
//Revision 1.61  2004/05/05 04:56:45  ogren
//Primitive checks for sticky again, removed GenerateMove's dependance on primitive, since if there's only 2 pieces, primitive will catch it anyways. -Elmer
//
//Revision 1.60  2004/05/05 04:31:32  ogren
//m9mm now doest die during Primitive, but hacked, not fixed. -Elmer
//
//Revision 1.59  2004/05/05 04:27:33  ogren
//added all_mills(board, piece) to return true if piece only has mills on board.  Used for can_be_taken -Elmer
//
//Revision 1.58  2004/05/05 04:02:28  ogren
//primitive now also checks for getting stuck -Elmer
//
//Revision 1.57  2004/05/02 03:40:19  ogren
//GenerateParents fully written, fully untested. -Elmer
//
//Revision 1.56  2004/05/01 23:24:22  ogren
//Hardcoded find_adj_pieces and find_adjacent nastily, yet to finish AppendFormedMill -Elmer
//
//Revision 1.55  2004/05/01 04:04:20  ogren
//Still need to write find_adj_pieces and AppendFormedMill to complete GenerateParents. -Elmer
//
//Revision 1.54  2004/04/29 16:24:06  ogren
//Not much work on find_adj_pieces, cant figure out an algorithm... -Elmer
//
//Revision 1.53  2004/04/29 15:52:07  ogren
//wrote count_pieces, full_board, skeleton for find_adj_pieces, all of which are untested. -Elmer
//
//Revision 1.52  2004/04/29 15:31:53  ogren
//started implementing gamespecific menu.  Mutating kHelpOnYourTurn is very, very wrong, however.  -Elmer
//
//Revision 1.51  2004/04/29 08:45:52  ogren
//no longer seg faults on cygwin due to using sizeof instead of strlen.  HelpOnYourTurn is still printing a mystery x after the normal strings, though. -Elmer
//
//Revision 1.50  2004/04/29 08:12:51  ogren
//A little more work on GenerateParents.  NOTE: can't use GenerateMoves since an undo into a mill does not remove pieces. -Elmer
//
//Revision 1.49  2004/04/29 00:01:09  ogren
//Started writing functinos for GenerateParents to support Bryon's 9mm reverse solver. -Elmer
//
//Revision 1.48  2004/04/28 21:57:35  ogren
//kHelpOnYourTurn now changes depending on whether the gFlying option is set to TRUE or FALSE -Elmer
//
//Revision 1.47  2004/04/28 19:57:15  ogren
//Started help txt, currently ugly and unclear.  Also, put the text for flying in a seperate STRING, but have yet to figure out how to concat it when the option is turned on.  -Elmer
//
//Revision 1.46  2004/04/25 10:00:04  ogren
//added boolean flying for future use.  Initial Position is also now a little more interesting. -Elmer
//
//Revision 1.45  2004/04/25 09:29:08  ogren
//changed move printing to [] to please Dan. -Elmer
//
//Revision 1.44  2004/04/23 05:59:46  ogren
//position 245103 is current initial position.  It has a good mix for now, no 1-move wins, but i would eventually like to find a position with pieces in all 4 quadrants. -Elmer
//
//Revision 1.43  2004/04/22 03:18:30  jjjordan
//Fixed gNumberOfPositions (InitializeGame). -JJ
//
//Revision 1.42  2004/04/21 03:47:43  weitu
//changed generate moves, still buggy tho
//
//Revision 1.41  2004/04/15 00:13:06  ogren
//Added count_mills for the future, currently incorrectly implemented.... -Elmer
//
//Revision 1.40  2004/04/15 00:11:40  ogren
//Added count_mills for the future, currently incorrectly implemented.... -Elmer
//
//Revision 1.39  2004/04/14 23:46:52  ogren
//debug can print a mini board with actual board on left, legend on right (kinda ugly).  No major changes, generate moves is still funny -Elmer
//
//Revision 1.38  2004/04/14 20:00:42  ogren
//minor debugging and spacing changes, maybe print mini-boards for debugging in future -Elmer
//
//Revision 1.37  2004/04/13 18:18:08  evedar
//Input and output of moves now works... Changed move handling functions.  Moves are now from<single space>to<single space>remove .  The remove is optional --Erwin
//
//Revision 1.36  2004/04/13 05:23:23  jjjordan
//You can now compile text games without having tcl. -JJ
//
//Revision 1.35  2004/04/12 19:34:11  ogren
//Minor movements of comments -Elmer
//
//Revision 1.34  2004/04/12 19:21:11  ogren
//parse_char and getinitialposition recognize 'b' and 'B' as blanks as well.  Does this conflict with anything?  -Elmer
//
//Revision 1.33  2004/04/12 16:13:12  evedar
//Fixed internal representation so that b's are now _'s.  The i/o of positions is now consistent, and turn prints in print position
//
//Revision 1.32  2004/04/12 00:11:31  ogren
//Added minb, made minb and maxb dependent upon BOARDSIZE, maxo, maxb, mino, minb, instead of just constants.  GetInitialPosition looks a little nicer. -Elmer
//
//Revision 1.31  2004/04/11 23:17:13  ogren
//added constant maxb, GetInitialPosition almost works, but it prompts which player to choose twice for some reason... -Elmer
//
//Revision 1.30  2004/04/10 19:50:09  ogren
//debugging is a little prettier, but maybe too much printing -Elmer
//
//Revision 1.29  2004/04/07 23:45:23  ogren
//expanded debugPosition (still untested) and added a place for future char unparse_blankox (just for balance)
//
//Revision 1.28  2004/04/07 22:57:37  ogren
//added debug function void debugPosition to print the c/b boards given position h, all debug functions untested =D -Elmer
//
//Revision 1.27  2004/04/07 21:40:31  ogren
//kTieIsPossible = FALSE, repositioned BOOLEAN debug variable -Elmer
//
//Revision 1.26  2004/04/07 19:39:04  ogren
//added debug variable and debugBoard(bboard,cboard) -Elmer
//
//Revision 1.25  2004/04/05 01:58:40  weitu
//m9mm fixed no segfaults, need to take care of input
//
//Revision 1.24  2004/04/04 19:08:11  weitu
//hash doesn't work? see debug messages
//
//Revision 1.22  2004/03/17 11:39:04  bryonr
//Changed type of gNumberOfPositions from int to POSITION. -Bryon
//
//Revision 1.21  2004/03/16 01:58:39  weitu
//Fixed to fit hash.c, now runs but wrong init position.
//
//Revision 1.20  2004/03/16 01:46:18  ogren
//set kdebugmenu  TRUE - Elmer
//
//Revision 1.19  2004/03/15 23:36:18  bryonr
//Changed some return types to match function prototypes in gamesman.h. Made minor fixes where necessary.
//
//Revision 1.18  2004/03/14 22:28:13  jjjordan
//Prototype fix. -JJ
//
//Revision 1.17  2004/03/11 02:25:27  evedar
//9mm compiles (2 warnings), still need to fix things mentioned in last entry.  9mm now included in makefile
//
//Revision 1.16  2004/03/11 01:42:48  evedar
//Finished m9mm functions.  Still needs: fixes from last checkin, help strings, etc.  Need to compile this latest version.  Specifically: filled in get option, set option, game-specific menu.
//
//Revision 1.15  2004/03/10 23:47:07  evedar
//Filled in almost all user i/o functions.  Need to fix: int/MOVE abstraction, badElse's.  Had to hard code in get initial position.  Should add a position hash/unhash sanity check to that function.  Commented 9mm out of Makefile while getting this stuff to compile.
//
//Revision 1.14  2004/03/07 20:49:09  evedar
//Added printposition, changed internal rep of board locations
//
//Revision 1.13  2004/03/07 19:28:31  evedar
//Parse errors fixed
//
//Revision 1.12  2004/03/07 19:05:03  weitu
//1.12 added hash.h and changed to fit new hash function.
//

//Revision 1.11  2004/03/07 18:59:04  ogren
//commet changes -Elmer
//
//Revision 1.10  2004/03/07 18:56:29  ogren
//misc. helper changes -Elmer
//
//Revision 1.9  2004/03/07 18:54:06  ogren
//Fixed some small syntax errors -Elmer
//
//Revision 1.8  2004/03/07 18:41:41  ogren
//removed semicolons from #define - Elmer
//
//Revision 1.7  2004/03/07 18:12:12  ogren
//DoMove compiles, changed piece enumeration from "b,o,x" to "blank,o,x" - Elmer
//
//Revision 1.6  2004/02/29 04:41:02  evedar
//Fixed some compile bugs.  Apparently boolean should be BOOLEAN.  Seems to be some problem with #define's
//
//Revision 1.5  2004/02/26 02:34:36  ogren
//Added CVS Id, Log tags to code
//
