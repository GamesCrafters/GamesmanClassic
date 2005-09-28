// $Id: mparadux.c,v 1.6 2005-09-28 06:17:18 yanpeichen Exp $

/*
 * The above lines will include the name and log of the last person
 * to commit this file to CVS
 */

/************************************************************************
**
** NAME:        mparadux.c
**
** DESCRIPTION: Paradux
**
** AUTHOR:      David Chen, Yanpei Chen
**
** DATE:        09/13/03
**
** UPDATE HIST: 
**
** 09/13/2005 David  - First Revision
** 09/14/2005 Yanpei - Fixed some typo in InitializeGame().
**                     Proposed alternative board indexing.
**                     Drew initial board position.
** 09/26/2005 Yanpei - Re-drew board position and coordinates.
** 09/27/2005 Yanpei - Some support functions and data structs
** 09/27/2005 David  - Merged some changes from my version (CVS doesn't
**                     work for me?)
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

/*************************************************************************
**
** Game-specific constants
**
**************************************************************************/

STRING   kGameName            = "Paradux"; /* The name of your game */
STRING   kAuthorName          = "David Chen, Yanpei Chen"; /* Your name(s) */
STRING   kDBName              = ""; /* The name to store the database under */

BOOLEAN  kPartizan            = FALSE ; /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN  kGameSpecificMenu    = FALSE ; /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN  kTieIsPossible       = FALSE ; /* TRUE if a tie is possible. FALSE if it is impossible.*/
BOOLEAN  kLoopy               = TRUE ; /* TRUE if the game tree will have cycles (a rearranger style game). FALSE if it does not.*/

BOOLEAN  kDebugMenu           = TRUE ; /* TRUE only when debugging. FALSE when on release. */
BOOLEAN  kDebugDetermineValue = TRUE ; /* TRUE only when debugging. FALSE when on release. */

POSITION gNumberOfPositions   =  0; /* The number of total possible positions | If you are using our hash, this is given by the hash_init() function*/
POSITION gInitialPosition     =  0; /* The initial hashed position for your starting board */
POSITION kBadPosition         = -1; /* A position that will never be used */

void*	 gGameSpecificTclInit = NULL;

/* 
 * Help strings that are pretty self-explanatory
 * Strings than span more than one line should have backslashes (\) at the end of the line.
 */

STRING kHelpGraphicInterface =
"Not written yet";

STRING   kHelpTextInterface    =
""; 

STRING   kHelpOnYourTurn =
"";

STRING   kHelpStandardObjective =
"";

STRING   kHelpReverseObjective =
"";

STRING   kHelpTieOccursWhen =
"A tie occurs when ...";

STRING   kHelpExample =
"";

/* pieces */
#define WHITE 1
#define BLACK 2
#define X 1
#define O 2

/* directions */
#define NW   0
#define NE   1
#define E    2
#define SE   3
#define SW   4
#define W    5
#define SWAP 6
#define NA -1

/*************************************************************************
**
** Game-specific variables
**
**************************************************************************/

/* Magically generated (in InitializeGame) */
int boardSize;
int numX;
int numO;
int numBlank;

/* The actual board */
char *board;

/* Mapping from values to characters */
char[] valToChar = { '-', 'X', 'O' };

/*************************************************************************
**
** #defines and structs
**
**************************************************************************/

/* Board Coordinates
   
          0,0  0,1  0,2              00  01  02

       1,0  1,1  1,2  1,3          03  04  05  06

     2,0  2,1  2,2  2,3  2,4     07  08  09  10  11

       3,0  3,1  3,2  3,3          12  13  14  15

         4,0  4,1  4,2               16  17  18


*/

/* Initial board - Paradux mini

          X   O   X 

        O   -   -   - 

      X   -   -   -   0 

        -   -   -   x   

          0   x   0     

*/

/* On the hexagonal board, only one side needs to be specified */
int boardSide = 3

/* Other options */
#define firstGo X

typedef struct board_item {

    short *slots;        

} PARABOARD;

typedef PARABOARD* PBPtr;

// must be used on arithmetic expressions and comparables
// "x" and "y" should not contain side effects
#define max(x,y) ((x)>(y) ? (x) : (y))
#define min(x,y) ((x)<(y) ? (x) : (y))


/*************************************************************************
**
** Global Variables
**
*************************************************************************/


/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/

/* External */
extern GENERIC_PTR	SafeMalloc ();
extern void		SafeFree ();

/* Multiple Implementations */

void                    (*initGame)( ) = &davidInitGame;

/* Support Functions */

PBPtr                   MallocBoard();
void                    FreeBoard(PBPtr);

// returns TRUE if square u,v is next to square x,y
// where u,v and x,y are the row,column coordinates
// returns FALSE for invalid coordinates
BOOLEAN                 neighbor(int u, int v, int x, int y);

// returns slot number when given row and column coordinates
// returns -1 when given invalid coordinates
int                     rcToSlot(int r, int c);

// returns row and column coordinate when given slot number
// returns -1 when given invalid slot number
// return data format = r*100+c
int                     slotToRC(int s);

/*************************************************************************
**
** Global Database Declaration
**
**************************************************************************/

extern VALUE     *gDatabase;


/************************************************************************
**
** NAME:        InitializeGame
**
** DESCRIPTION: Prepares the game for execution.
**              Initializes required variables.
**              Sets up gDatabase (if necessary).
** 
************************************************************************/

void InitializeGame ()
{
  initGame();
}

/* Initialize odd-sided board */
void initOddBoard() {
  int col, row, el = 0, maxRow = boardSide - 1, maxCol = boardSize * 2 - 2;

  for (col = 0; col <= maxCol; col++) {
    if (col < boardSide) {
      maxRow++;
    } else {
      maxRow--;
    }

    for (row = 0; row < maxRow; ro++, el++) {
      if (col == 0) {
	board[el] = valToChar[row % 2 + 1];
      } else if (col == maxCol) {
	board[el] = valToChar[(row + 1) % 2 + 1];
      } else if (col <= boardSize - 2) {
	if (row == 0) {
	  board[el] = valToChar[(col + 1) % 2 + 1];
	} else {
	  board[el] = valToChar[BLANK];
	}
      } else if (col >= boardSize) {
	if (row == maxRow) {
	  board[el] = valToChar[col % 2 + 1];
	} else {
	  board[el] = valToChar[BLANK];
	}
      } else {
	if (row == 0) {
	  board[el] = valToChar[(col + 1) % 2 + 1];
	} else if (row == maxRow) {
	  board[el] = valToChar[col % 2 + 1];
	} else {
	  board[el] = valToChar[BLANK];
	}
      }
    }
  }

  gInitialPosition = generic_hash(board, firstGo);
}

/* Initialize even-sided board */
void initEvenBoard() {
  int col, row, el = 0, maxRow = boardSide - 1, maxCol = boardSize * 2 - 2;

  for (col = 0; col <= maxCol; col++) {
    if (col < boardSide) {
      maxRow++;
    } else {
      maxRow--;
    }

    for (row = 0; row < maxRow; row++, el++) {
      if (col == 0 || col == maxCol) {
	board[el] = valToChar[row % 2 + 1];
      } else if (col == boardSide - 2) {
	if (row == 0 || row == boardSide - 1) {
	  board[el] = valToChar[X];
	} else if (row == maxRow) {
	  board[el] = valToChar[O];
	} else {
	  board[el] = valToChar[BLANK];
	}
      } else if (col == boardSide) {
	if (row == 0) {
	  board[el] = valToChar[X];
	} else if (row == maxRow || row == maxRow - boardSide + 2) {
	  board[el] = valToChar[O];
	} else {
	  board[el] = valToChar[BLANK];
	}
      } else {
	if (row == 0) {
	  board[el] = valToChar[(col + 1) % 2 + 1];
	} else if (row == maxRow) {
	  board[el] = valToChar[col % 2 + 1];
	} else {
	  board[el] = valToChar[BLANK];
	}
      }
    }
  }

  gInitialPosition = generic_hash(board, firstGo);
}

void davidInitGame ()
{
  if (boardSide < 3) {
    printf("ERROR: boards with side length < 3 cannot be represented or are trivial");
    exit(1);
  }

  /* 3(n-1)n + 1... I think */
  boardSize = 3 * (boardSide - 1) * boardSide + 1;

  /* boards with side length 1 don't follow this but we'll ignore that */
  numX = numO = (boardSide - 1) * 6 + 1;
  numBlank = boardSize - numX - numO;

  int pieces[] = { 'X', numXpieces, numXpieces,
		   'O', numOpieces, numOpieces,
		   '-', numBlank, numBlank,
		   0 };

  gNumberOfPositions = generic_hash_init(boardSize, pieces, NULL);

  board = (char *) SafeMalloc (sizeof(char) * boardSize);

  if (boardSide % 2) {
    initOddBoard();
  } else {
    initEvenBoard();
  }
}

/************************************************************************
**
** NAME:        GenerateMoves
**
** DESCRIPTION: Creates a linked list of every move that can be reached
**              from this position. Returns a pointer to the head of the
**              linked list.
** 
** INPUTS:      POSITION position : Current position for move
**                                  generation.
**
** OUTPUTS:     (MOVELIST *)      : A pointer to the first item of
**                                  the linked list of generated moves
**
** CALLS:       MOVELIST *CreateMovelistNode();
**
************************************************************************/

MOVELIST *GenerateMoves (POSITION position)
{
    MOVELIST *moves = NULL;
    
    /* Use CreateMovelistNode(move, next) to 'cons' together a linked list */
    
    return moves;
}


/************************************************************************
**
** NAME:        DoMove
**
** DESCRIPTION: Applies the move to the position.
** 
** INPUTS:      POSITION position : The old position
**              MOVE     move     : The move to apply to the position
**
** OUTPUTS:     (POSITION)        : The position that results from move
**
** CALLS:       Some Board Hash Function
**              Some Board Unhash Function
**
*************************************************************************/

POSITION DoMove (POSITION position, MOVE move)
{
    return 0;
}


/************************************************************************
**
** NAME:        Primitive
**
** DESCRIPTION: Returns the value of a position if it fulfills certain
**              'primitive' constraints.
**
**              Example: Tic-tac-toe - Last piece already placed
**
**              Case                                  Return Value
**              *********************************************************
**              Current player sees three in a row    lose
**              Entire board filled                   tie
**              All other cases                       undecided
** 
** INPUTS:      POSITION position : The position to inspect.
**
** OUTPUTS:     (VALUE)           : one of
**                                  (win, lose, tie, undecided)
**
** CALLS:       None              
**
************************************************************************/

VALUE Primitive (POSITION position)
{
    return undecided;
}


/************************************************************************
**
** NAME:        PrintPosition
**
** DESCRIPTION: Prints the position in a pretty format, including the
**              prediction of the game's outcome.
** 
** INPUTS:      POSITION position    : The position to pretty print.
**              STRING   playersName : The name of the player.
**              BOOLEAN  usersTurn   : TRUE <==> it's a user's turn.
**
** CALLS:       Unhash()
**              GetPrediction()      : Returns the prediction of the game
**
************************************************************************/

void PrintPosition (POSITION position, STRING playersName, BOOLEAN usersTurn)
{
    
}


/************************************************************************
**
** NAME:        PrintComputersMove
**
** DESCRIPTION: Nicely formats the computers move.
** 
** INPUTS:      MOVE    computersMove : The computer's move. 
**              STRING  computersName : The computer's name. 
**
************************************************************************/

void PrintComputersMove (MOVE computersMove, STRING computersName)
{
    
}


/************************************************************************
**
** NAME:        PrintMove
**
** DESCRIPTION: Prints the move in a nice format.
** 
** INPUTS:      MOVE move         : The move to print. 
**
************************************************************************/

void PrintMove (MOVE move)
{
    
}


/************************************************************************
**
** NAME:        GetAndPrintPlayersMove
**
** DESCRIPTION: Finds out if the player wishes to undo, abort, or use
**              some other gamesman option. The gamesman core does
**              most of the work here. 
**
** INPUTS:      POSITION position    : Current position
**              MOVE     *move       : The move to fill with user's move. 
**              STRING   playersName : Current Player's Name
**
** OUTPUTS:     USERINPUT          : One of
**                                   (Undo, Abort, Continue)
**
** CALLS:       USERINPUT HandleDefaultTextInput(POSITION, MOVE*, STRING)
**                                 : Gamesman Core Input Handling
**
************************************************************************/

USERINPUT GetAndPrintPlayersMove (POSITION position, MOVE *move, STRING playersName)
{
    USERINPUT input;
    USERINPUT HandleDefaultTextInput();
    
    for (;;) {
        /***********************************************************
         * CHANGE THE LINE BELOW TO MATCH YOUR MOVE FORMAT
         ***********************************************************/
	printf("%8s's move [(undo)/(MOVE FORMAT)] : ", playersName);
	
	input = HandleDefaultTextInput(position, move, playersName);
	
	if (input != Continue)
		return input;
    }

    /* NOTREACHED */
    return Continue;
}


/************************************************************************
**
** NAME:        ValidTextInput
**
** DESCRIPTION: Rudimentary check to check if input is in the move form
**              you are expecting. Does not check if it is a valid move.
**              Only checks if it fits the move form.
**
**              Reserved Input Characters - DO NOT USE THESE ONE CHARACTER
**                                          COMMANDS IN YOUR GAME
**              ?, s, u, r, h, a, c, q
**                                          However, something like a3
**                                          is okay.
** 
**              Example: Tic-tac-toe Move Format : Integer from 1 to 9
**                       Only integers between 1 to 9 are accepted
**                       regardless of board position.
**                       Moves will be checked by the core.
**
** INPUTS:      STRING input : The string input the user typed.
**
** OUTPUTS:     BOOLEAN      : TRUE if the input is a valid text input.
**
************************************************************************/

BOOLEAN ValidTextInput (STRING input)
{
    return FALSE;
}


/************************************************************************
**
** NAME:        ConvertTextInputToMove
**
** DESCRIPTION: Converts the string input your internal move representation.
**              Gamesman already checked the move with ValidTextInput
**              and ValidMove.
** 
** INPUTS:      STRING input : The VALID string input from the user.
**
** OUTPUTS:     MOVE         : Move converted from user input.
**
************************************************************************/

MOVE ConvertTextInputToMove (STRING input)
{
    return 0;
}


/************************************************************************
**
** NAME:        GameSpecificMenu
**
** DESCRIPTION: Prints, receives, and sets game-specific parameters.
**
**              Examples
**              Board Size, Board Type
**
**              If kGameSpecificMenu == FALSE
**                   Gamesman will not enable GameSpecificMenu
**                   Gamesman will not call this function
** 
**              Resets gNumberOfPositions if necessary
**
************************************************************************/

void GameSpecificMenu ()
{
    
}


/************************************************************************
**
** NAME:        SetTclCGameSpecificOptions
**
** DESCRIPTION: Set the C game-specific options (called from Tcl)
**              Ignore if you don't care about Tcl for now.
** 
************************************************************************/

void SetTclCGameSpecificOptions (int options[])
{
    
}
  
  
/************************************************************************
**
** NAME:        GetInitialPosition
**
** DESCRIPTION: Called when the user wishes to change the initial
**              position. Asks the user for an initial position.
**              Sets new user defined gInitialPosition and resets
**              gNumberOfPositions if necessary
** 
** OUTPUTS:     POSITION : New Initial Position
**
************************************************************************/

POSITION GetInitialPosition ()
{
    return 0;
}


/************************************************************************
**
** NAME:        NumberOfOptions
**
** DESCRIPTION: Calculates and returns the number of variants
**              your game supports.
**
** OUTPUTS:     int : Number of Game Variants
**
************************************************************************/

int NumberOfOptions ()
{
    return 0;
}


/************************************************************************
**
** NAME:        getOption
**
** DESCRIPTION: A hash function that returns a number corresponding
**              to the current variant of the game.
**              Each set of variants needs to have a different number.
**
** OUTPUTS:     int : the number representation of the options.
**
************************************************************************/

int getOption ()
{
    /* If you have implemented symmetries you should
       include the boolean variable gSymmetries in your
       hash */
    return 0;
}


/************************************************************************
**
** NAME:        setOption
**
** DESCRIPTION: The corresponding unhash function for game variants.
**              Unhashes option and sets the necessary variants.
**
** INPUT:       int option : the number representation of the options.
**
************************************************************************/

void setOption (int option)
{
    /* If you have implemented symmetries you should
       include the boolean variable gSymmetries in your
       hash */
}


/************************************************************************
**
** NAME:        DebugMenu
**
** DESCRIPTION: Game Specific Debug Menu (Gamesman comes with a default
**              debug menu). Menu used to debug internal problems.
**
**              If kDebugMenu == FALSE
**                   Gamesman will not display a debug menu option
**                   Gamesman will not call this function
** 
************************************************************************/

void DebugMenu ()
{
    
}

MOVE hashMove (int direction, int pos1, int pos2)
{
  int nDirection = neighboringDirection(pos1, pos2);

  return (pos1 << 26) & (nDirection << 3) & direction;
}

void unhashMove (MOVE move, int* direction, int* pos1, int* pos2) {
  *pos1 = move >> 26;
  *pos2 = getNeighbor(*pos1, (move >> 3) & 7);

  *direction = move & 7;
}

/************************************************************************
**
** Everything specific to this module goes below these lines.
**
** Things you want down here:
** Move Hasher
** Move Unhasher
** Any other function you deem necessary to help the ones above.
** 
************************************************************************/

/************************************************************************
**
**  SUPPORT FUNCTIONS
**
************************************************************************/

PBPtr MallocBoard() {
    PBPtr toReturn = (PBPtr) SafeMalloc(sizeof(PARABOARD));
    toReturn->slots = (short *) SafeMalloc(boardSize);
    return toReturn;
}

void FreeBoard(PBPtr b) {
    SafeFree(b->slots);
    SafeFree(b);
}

int getNeighbor(int pos, int direction) {
  return 0;
}

/* Direction in which the neighbor neighbors pos:
  0 NW
  1 NE
  2 E
  3 SE
  4 SW
  5 W
 -1 Not neighboring
*/

int neighboringDirection(int pos, int neighbor) {
  int pCol, pRow;
  int nCol, nRow;

  getColRow(&pCol, &pRow);
  getColRow(&nCol, &nRow);

  int dCol = nCol - pCol,
      dRow = nRow - pRow;

  if (pCol < boardSize - 1) {
    switch (dCol) {
    case -1:
      switch (dRow) {
      case -1:
	return NW;
      case 0:
	return NE;
      default:
	return NA;
      }
      break;
    case 1:
      switch (dRow) {
      case 1:
	return  SE;
      case 0:
	return SW;
      default:
	return NA;
      }
      break;
    case 0:
      switch (dRow) {
      case -1:
	return W;
      case 1:
	return E;
      default:
	return NA;
      }
      break;
    default:
      return NA;
    }
  } else if (pCol > boardSize) {
    switch (dCol) {
    case -1:
      switch (dRow) {
      case 1:
	return NE;
      case 0:
	return NW;
      default:
	return NA;
      }
      break;
    case 1:
      switch (dRow) {
      case -1:
	return SW;
      case 0:
	return SE;
      default:
	return NA;
      }
      break;
    case 0:
      switch (dRow) {
      case -1:
	return W;
      case 1:
	return E;
      default:
	return -1;
      }
      break;
    default:
      return -1;
    }
  } else {
    switch (dCol) {
    case -1:
      switch (dRow) {
      case -1:
	return NW;
      case 0:
	return NE;
     default:
	return -1;
      }
      break;
    case 1:
      switch (dRow) {
      case -1:
	return SW;
      case 0:
	return SE;
      default:
	return -1;
      }
      break;
    case 0:
      switch (dRow) {
      case -1:
	return W;
      case 1:
	return E;
      default:
	return -1;
      }
      break;
    default:
      return -1;
    }
  }
}

// returns TRUE if square u,v is next to square x,y
// where u,v and x,y are the row,column coordinates
// returns FALSE for invalid coordinates
BOOLEAN neighbor(int u, int v, int x, int y) {

  return
    // valid rows
    (u>=0 && u<(2*boardSide-1) && x>=0 && x<(2*boardSide-1)) &&
    // valid columns
    (v>=0 && v<(u<boardSide ? boardSide+u : 2*boardSide - u%boardSide - 2)) &&    
    (y>=0 && y<(x<boardSide ? boardSide+x : 2*boardSide - x%boardSide - 2)) &&
    // neighbor same row, two possible neighbors, two cases
    (((u==x) && ((v==y-1) || (v==y+1))) ||
     // neighbor different rows, four possible neighbors, four cases
     (((u==x-1) || (u==x+1)) && 
      ((v==y) ||
       (x<(boardSide-1) ? ((u==x-1) && (v==y-1)) : ((u==x+1) && (v==y+1))) ||
       (x>(boardSide-1) ? ((u==x-1) && (v==y+1)) : ((u==x+1) && (v==y-1))) ||
       (x==(boardSide-1) && (v==y-1))
       )
      ) // end neighbor different rows
     ) // end all neighbors
    ; // end return

}

// returns slot number when given row and column coordinates
// returns -1 when given invalid coordinates
int rcToSlot(int r, int c) {

  int x,y=boardSide,toReturn=-1;

  if (    
      // valid r
      (r>=0 && r<(2*boardSide-1)) &&
      // valid columns
      (c>=0 && c<(r<boardSide ? boardSide+r : 2*boardSide - r%boardSide - 2))
      ) {
    for (x=0; x<min(boardSide,r); x++) {
      toReturn += y++;
    }
    for (; x<min(2*boardSide-1,r); x++) {
      toReturn += y--;
    }
    return toReturn += c;
  } else {
    // invalid r,c
    return -1;
  }

}

// returns row and column coordinate when given slot number
// returns -1 when given invalid slot number
// return data format = r*100+c
int slotToRC(int s) {

  int x=0,y=boardSide,z=-1;
  int r,c;

  while (z<s && x<boardSide) {
    z += y++;
    x++;
  }
  if (x==(boardSide-1)) y--;
  while (z<s && x<(2*boardSide-1)) {
    z += y--;
    x++;
  }

  r = x-1;
  if (r<boardSide-1) {
    c = (s-(z-y+1)-1)%(y-1);
  } else if (r==boardSize-1) {
    c = (s-);
  } else {
    c = (s-(z-y+1)+1)%(y+1);
  }

  return 0;

}

void getColRow(int pos, int* pCol, int* pRow) {
  int col = 0, row = 0, rowSize = boardSide, numEls = 0;

  for (; pos >= numEls + rowSize; col++, numEls += rowSize, rowSize += (col > boardSide - 1 ? -1 : 1 ));

  *pCol = col;
  *pRow = pos - numEls + rowSize;
}

// $Log: not supported by cvs2svn $
// Revision 1.5  2005/09/28 05:27:24  trikeizo
// See change log.
//
// Revision 1.4  2005/09/28 04:54:03  yanpeichen
// 09/27/2005 Yanpei - Some support functions and data structs
//
// Revision 1.3  2005/09/28 04:24:24  yanpeichen
// *** empty log message ***
//
// Revision 1.2  2005/09/26 08:09:43  yanpeichen
// yanpei: additional comments
//
// Revision 1.1  2005/09/14 19:57:44  yanpeichen
// David Chen, Yanpei Chen modifying mparadux.c
//
// First revision
//
// Revision 1.4  2005/05/02 17:33:01  nizebulous
// mtemplate.c: Added a comment letting people know to include gSymmetries
// 	     in their getOption/setOption hash.
// mttc.c: Edited to handle conflicting types.  Created a PLAYER type for
//         gamesman.  mttc.c had a PLAYER type already, so I changed it.
// analysis.c: Changed initialization of option variable in analyze() to -1.
// db.c: Changed check in the getter functions (GetValueOfPosition and
//       getRemoteness) to check if gMenuMode is Evaluated.
// gameplay.c: Removed PlayAgainstComputer and PlayAgainstHuman.  Wrote PlayGame
//             which is a generic version of the two that uses to PLAYER's.
// gameplay.h: Created the necessary structs and types to have PLAYER's, both
// 	    Human and Computer to be sent in to the PlayGame function.
// gamesman.h: Really don't think I changed anything....
// globals.h: Also don't think I changed anything....both these I just looked at
//            and possibly made some format changes.
// textui.c: Redid the portion of the menu that allows you to choose opponents
// 	  and then play a game.  Added computer vs. computer play.  Also,
//           changed the analysis part of the menu so that analysis should
// 	  work properly with symmetries (if it is in getOption/setOption hash).
//
// Revision 1.3  2005/03/10 02:06:47  ogren
// Capitalized CVS keywords, moved Log to the bottom of the file - Elmer
//

