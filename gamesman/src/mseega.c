// $id$
// $log$

/* 
To compile, use
gcc -fPIC -O -DSUNOS5  -I/usr/sww/pkg/tcltk-8.4.4/include -I/usr/openwin/include -fPIC -lz -lnsl -lsocket -lm -shared -static-libgcc -L/usr/sww/pkg/tcltk-8.4.4/lib -ltk8.4 -L/usr/sww/pkg/tcltk-8.4.4/lib -ltcl8.4 -L/usr/sww/pkg/tcltk-8.4.4/lib -ltcl8.4 mseega.c 
*/

/* 
 * Above is will include the name and the log of the last
 * person to commit this file to gamesman.
 */
// YR: I don't know how to format that. (updated)

/************************************************************************
**
** NAME:        mseega.c
**
** DESCRIPTION: Seega
**
** AUTHORS:     Emad Salman <emadsalman@berkeley.edu>,
**              Yonathan Randolph <yonran@berkeley.edu>,
**              Peter Wu <peterwu@berkeley.edu>
**
** DATE:        Began 2004-09-29; Finished 2004-10-...
**
** UPDATE HIST: 2004-10-04 Put it into CVS.
**              2004-10-12 Peter: I wrote this earlier, but I didn't know
**               know that checking-in a file would lead to a vi interface
**              2004-10-12 Added more defines, GamesSpecificMenu, PrintPosition
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
#define DEBUGGING 1

extern STRING gValueString[];


POSITION gNumberOfPositions  = 0; /* The number of total possible positions | If you are using our hash, this is given by the hash_init() function*/
POSITION gInitialPosition    = 0; /* The initial position (starting board) */
//POSITION gMinimalPosition    = 0 ;
POSITION kBadPosition        = -1; /* A position that will never be used */

STRING   kGameName           = "Seega"; /* The name of your game */
STRING   kDBName             = ""; /* The name to store the database under */
BOOLEAN  kPartizan           = FALSE; /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
//BOOLEAN  kSupportsHeuristic  = ;
//BOOLEAN  kSupportsSymmetries = ;
BOOLEAN  kSupportsGraphics   = FALSE;
BOOLEAN  kDebugMenu          = TRUE; /* TRUE while debugging */
BOOLEAN  kGameSpecificMenu   = TRUE; /* TRUE if there is a game specific menu*/
BOOLEAN  kTieIsPossible      = FALSE; /* TRUE if a tie is possible */
BOOLEAN  kLoopy              = TRUE; /* TRUE if the game tree will have cycles (a rearranger style game) */
BOOLEAN  kDebugDetermineValue = TRUE; /* TRUE while debugging */
void*    gGameSpecificTclInit = NULL;


/* 
   Help strings that are pretty self-explanatory 
*/

// TODO Look at help strings for Achi! We're going to change these when
// we implement the 2-step process

STRING kHelpGraphicInterface =
"Not written yet";

//TODO
STRING   kHelpTextInterface    =
"TO BE FINISHED"; 

STRING   kHelpOnYourTurn = 
"To move your piece to an unoccupied, adjacent square in the horizontal\n"
"or vertical directions.";

STRING   kHelpStandardObjective =
"To capture the opponent's counters until the opponent only has one\n"
"counter left.";

STRING   kHelpReverseObjective =
"To force your opponent to make moves that will eat of your pieces until\n"
"you are left with one piece.";

STRING   kHelpTieOccursWhen = /* Should follow 'A Tie occurs when... */
"NOT possible in this game.";

//TODO
STRING   kHelpExample =
"TO BE FINISHED";

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

//note: having more than 9 rows would mess up the printf, unless we want to swap the letter and column convention
#define MAXROWS 9
#define MAXCOLS 9 
#define DEFAULTROWS 3
#define DEFAULTCOLS 3

int width=DEFAULTROWS, height=DEFAULTROWS; //changed

int BOARDSIZE = 11; 
/* width*height, plus one for whose move it is and whether we are in
   placing mode.*/

typedef char* Board;
typedef int* SMove;

typedef enum blank_o_x {Blank, x, o} BlankOX;

inline char whoseBoard(Board b);
inline char whoseMove(SMove m);
inline char getpce(Board b, int r);
inline char getPiece(Board b, int x, int y);

inline int fromWhere(SMove m); // applies only if !placingMove(m)
inline int toWhere(SMove m);
inline int toWhere2(SMove m); // applies only if placingMove(m)

inline BOOLEAN placingBoard(Board b);
inline BOOLEAN placingMove(SMove m);

inline void setpce(Board b, int r, char c);
inline void setWhoseMove(Board b, char c);
inline void setMove(SMove m, char who, int rfrom, int rto);
inline void setPlacingBoard(Board b, BOOLEAN t);
inline void setPlacingMove(SMove m, BOOLEAN t);

/* tells whether r is a forbidden spot for placing pieces (i.e. the
   center) */
inline int forbiddenSpot(int r) {return height/2 == r/width && width/2 == r%width;}


/*************************************************************************
**
** Above is where you put your #define's and your global variables, structs
**
*************************************************************************/

/*
** Function Prototypes:
*/

/* Function prototypes here. */

/* External */
extern GENERIC_PTR	SafeMalloc ();
extern void		SafeFree ();

/*************************************************************************
**
** Here we declare the global database variables
**
**************************************************************************/

extern VALUE     *gDatabase;


/************************************************************************
**
** NAME:        InitializeGame
**
** DESCRIPTION: Initialize the gDatabase, a global variable. and the other
**              local variables.
** 
************************************************************************/

void InitializeGame () {
  int boardspec[] = {'x', 0, floor(BOARDSIZE/2), 
		     'o', 0, floor(BOARDSIZE/2),
		     '-', 0, BOARDSIZE-3,
		     -1};
  int maxpos = generic_hash_init(BOARDSIZE, boardspec, NULL);
}


/************************************************************************
**
** NAME:        DebugMenu
**
** DESCRIPTION: Menu used to debub internal problems. Does nothing if
**              kDebugMenu == FALSE
** 
************************************************************************/

void DebugMenu () {
  
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
    
    printf("\n");
    printf("Seega Game Specific Menu\n\n");
    printf("1) Change the board size (default = %d x %d) \n", DEFAULTROWS, DEFAULTCOLS);
    printf("2) To be inserted.. \n");
    printf("b) Back to previous menu\n\n");
    
    //printf("Current option:   %s\n", allDiag ? "All diagonal moves": noDiag ? "No diagonal moves" : "Standard diagonal moves");
    printf("Select an option: ");
    
    switch(GetMyChar()) {
    case 'Q': case 'q':
	ExitStageRight();
	break;
    case '1':
	changeBoard();
	break;

    case 'b': case 'B':
	return;
    default:
	printf("\nSorry, I don't know that option. Try another.\n");
	HitAnyKeyToContinue();
	GameSpecificMenu();
	break;
    }
}

void changeBoard() 
{
  int n_rows, n_cols, valid_cols, valid_rows;
  valid_cols = 0;
  valid_rows = 0;
  printf("Enter the new number of rows (3-%d):  ", MAXROWS);
  (void) scanf("%u", &n_rows);
  if ((n_rows < 3) || (n_rows > MAXROWS)) {
    printf("Number of rows must be between to 3 and %d\n", MAXROWS);
    changeBoard(); //optional - change to better style
  } else {
    printf("Changing number of rows to %d ...\n", n_rows);
    height = n_rows;
  }
  printf("Enter the new number of columns (3-%d):  ", MAXCOLS);
  while (valid_cols == 0){
    (void) scanf("%u", &n_cols);
    if ((n_cols < 3) || (n_cols > MAXCOLS)) {
      printf("Number of rows must be between to 3 and %d\n", MAXCOLS);
    } else {
      printf("Changing number of columns to %d ...\n", n_cols);
      width = n_cols;
      valid_cols = 1;
    }
  }
  InitializeGame();
  //displayBoard(); //this is temporary!!
  //printf("done! \n");
  //GetMyChar();
}
  
/************************************************************************
**
** NAME:        SetTclCGameSpecificOptions
**
** DESCRIPTION: Set the C game-specific options (called from Tcl)
**              Ignore if you don't care about Tcl for now.
** 
************************************************************************/

void SetTclCGameSpecificOptions (int options[]) {
  
}


/************************************************************************
**
** NAME:        DoMove //TEMPORARY!!
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

//TODO CHANGE THIS
POSITION DoMove (POSITION position, MOVE move) {
    
    BlankOX theBlankOX[BOARDSIZE];
    int from, to;
    BlankOX whosTurn;
    BOOLEAN phase1();

    
    PositionToBlankOX(position,theBlankOX, &whosTurn);
    from = 0;
    to = 0;
   
    if(phase1(theBlankOX)) {
	theBlankOX[move-1] = whosTurn;
    }
    else {
	
	from = move/10;
	to = move%10;
	theBlankOX[to-1] = theBlankOX[from-1];
	theBlankOX[from-1] = (int)Blank;
    }
    if(whosTurn == o)
	whosTurn = x;
    else
	whosTurn = o;
    
    return BlankOXToPosition(theBlankOX,whosTurn);
}

BOOLEAN phase1(BlankOX *theBlankOX) {
    int count = 0;
    int i = 0;
    while(i < BOARDSIZE) {
	    if(theBlankOX[i] != Blank)
		count++;
	//TODO WARNING: HARDCODED NUMBER for 3x3 board - start at 5th move or 9th move
	    if(count >= 5)
		return FALSE;
	    i++;
	}
    return TRUE;
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

POSITION GetInitialPosition () {
  return 0;
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

void PrintComputersMove (MOVE computersMove, STRING computersName) {

}


/************************************************************************
**
** NAME:        Primitive
**
** DESCRIPTION: Return the value of a position if it fulfills certain
**              'primitive' constraints. Some examples of this is having
**              three-in-a-row with Gobblet. Three in a row for the player
**              whose turn it is a win, otherwise its a loss.
**              Otherwise undecided.
** 
** INPUTS:      POSITION position : The position to inspect.
**
** OUTPUTS:     (VALUE) an enum which is oneof: (win,lose,tie,undecided)
**
** CALLS:       LIST FUNCTION CALLS
**              
**
************************************************************************/

VALUE Primitive (POSITION position) {
  return undecided;
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


//this will be our print position function - prints for a general sized board
//now we only have to grab the current values of the board!
//TODO: take into account the arguments
void PrintPosition (POSITION position, STRING playerName, BOOLEAN usersTurn){
{
  int currRow;
  char currCol;
  char alphabet[]="abcdefghijklmnopqrstuvwxyz";

  printf("\n");
  printf("          LEGEND:");
  for (currCol = 0; currCol < width; currCol++) {
    printf("  ");
  }
  printf("             TOTAL:\n");
  for (currRow = height; currRow>0; currRow--) {
    //printf("    %d ( a%d b%d c%d )          : - - -\n", currRow, currRow, currRow, currRow);
    printf("    %d ( ", currRow);
    for (currCol = 0; currCol < width; currCol++) {
      printf("%c%d ", alphabet[currCol], currRow);
    }
    printf(")          :");
    for (currCol = 0; currCol < width; currCol++) {
      printf("%c ", '-'); //TODO GET VALUES FROM BOARD
    }
    printf("\n");
  }
  printf("        ");  //a  b  c          PLAYERS TURN\n\n");
  for (currCol = 0; currCol < width; currCol++) {
    printf("%c  ", alphabet[currCol]);  //"          PLAYERS TURN\n\n");
  }
  printf("\n\n");
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

MOVELIST *GenerateMoves (POSITION position) {
  return NULL;
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

USERINPUT GetAndPrintPlayersMove (POSITION position, MOVE* move, STRING playerName) {
  return Abort;
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

BOOLEAN ValidTextInput (STRING input) {
  return FALSE;
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

MOVE ConvertTextInputToMove (STRING input) {
  return 0;
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

void PrintMove (MOVE move) {
  
}


/************************************************************************
**
** NAME:        NumberOfOptions
**
** DESCRIPTION: Calculates and returns the number of option combinations
**				there are with all the game variations you program.
**
** OUTPUTS:     int : the number of option combination there are.
**
************************************************************************/

int NumberOfOptions () {
  return 0;
}


/************************************************************************
**
** NAME:        getOption
**
** DESCRIPTION: A hash function to keep track of all the game variants.
**				Should return a different number for each set of
**				variants.
**
** OUTPUTS:     int : the number representation of the options.
**
************************************************************************/

int getOption () {
  return 0;
}


/************************************************************************
**
** NAME:        setOption
**
** DESCRIPTION: The corresponding unhash for the game variants.
**				Should take the input and set all the appropriate
**				variants.
**
** INPUT:     int : the number representation of the options.
**
************************************************************************/

void setOption (int option) {
  
}


/************************************************************************
*************************************************************************
**         EVERYTHING BELOW THESE LINES IS LOCAL TO THIS FILE
*************************************************************************
************************************************************************/

/************************************************************************
*************************************************************************
**         EVERYTHING BELOW THESE LINES IS LOCAL TO THIS FILE
*************************************************************************
************************************************************************/

/************************************************************************
 **
 ** NAME:        PositionToBlankOX
 **
 ** DESCRIPTION: convert an internal position to that of a BlankOX.
 ** 
 ** INPUTS:      POSITION thePos     : The position input. 
 **              BlankOX *theBlankOx : The converted BlankOX output array. 
 **
 ** CALLS:       BadElse()
 **
 ************************************************************************/

inline char whoseBoard(Board b) {return b[width*height];}
inline char whoseMove(SMove m) {return m[2];}
inline char getpce(Board b, int r) {return b[r];}
inline char getPiece(Board b, int x, int y) {return getpce(b,x*width+y);}

inline int fromWhere(SMove m) {return m[1];} // applies only if !placingMove(m)
inline int toWhere(SMove m) {return m[0];}
inline int toWhere2(SMove m) {return m[1];} // applies only if placingMove(m)

inline BOOLEAN placingBoard(Board b) {return b[width*height+1];}
inline BOOLEAN placingMove(SMove m) {return m[3];}

inline void setpce(Board b, int r, char c) {b[r]=c;}
inline void setWhoseMove(Board b, char c) {b[width*height]=c;}
inline void setMove(SMove m, char who, int rfrom, int rto) {
  m[1]=rfrom; m[0]=rto; m[2]=who;
}
inline void setPlacingBoard(Board b, BOOLEAN t) {b[width*height+1]=t;}
inline void setPlacingMove(SMove m, BOOLEAN t) {m[3]=t;}




PositionToBlankOX(thePos,theBlankOX,whosTurn)
     POSITION thePos;
     BlankOX *theBlankOX, *whosTurn;
{
    int i;
    int POSITION_OFFSET; // NOT SURE WHAT THIS IS -YR
    int g3Array[111]; // NOT SURE WHAT THIS IS -YR
    if(thePos >= POSITION_OFFSET) {
	*whosTurn = x;  /* if the last character in the array is an x */
	thePos -= POSITION_OFFSET;
    }
    else
	*whosTurn = o;
    for(i = 8; i >= 0; i--) {
	if(thePos >= ((int)x * g3Array[i])) {
	    theBlankOX[i] = x;
	    thePos -= (int)x * g3Array[i];
	}
	else if(thePos >= ((int)o * g3Array[i])) {
	    theBlankOX[i] = o;
	    thePos -= (int)o * g3Array[i];
	}
	else if(thePos >= ((int)Blank * g3Array[i])) {
	    theBlankOX[i] = Blank;
	    thePos -= (int)Blank * g3Array[i];
	}
	else
	    BadElse("PositionToBlankOX");
    }
}








/* YR: These are the functions I made before. They don't work yet
   probably.  */
int nextOpenSpot(Board b, int lowerBound) {
  for (; lowerBound<width*height; lowerBound++)
    if (!getpce(b, lowerBound))
      return lowerBound;
  return -1;
}
int nextOpenInitSpot(Board b, int lowerBound) {
  for (; lowerBound<width*height; lowerBound++)
    if (!forbiddenSpot(lowerBound) && !getpce(b, lowerBound))
      return lowerBound;
  return -1;
}
int nextSpotOfType(Board b, int lowerBound, int whoseTurn) {
  for (; lowerBound<width*height; lowerBound++)
    if (getpce(b, lowerBound)==whoseTurn)
      return lowerBound;
  return -1;
}
MOVELIST *GeneratePlacingMoves(POSITION position) {
  MOVELIST *CreateMovelistNode(), *head = NULL;
  int i,j;
  for (i=nextOpenInitSpot((Board)position, 0);i!=-1;i=nextOpenInitSpot((Board)position, i+1))
    for (j=nextOpenInitSpot((Board)position, i+1);j!=-1;j=nextOpenInitSpot((Board)position, j+1))
      // WARNING: HARD-CODED CONSTANT
      head = CreateMovelistNode(i | j<<5, head);
  //  head = head | 1<<sizeof(MOVE)*4-1; // to distinguish it from moving move
  return head;
}
MOVELIST *GenerateMovingMoves(POSITION position) {
  MOVELIST *CreateMovelistNode(), *head = NULL;
  int i,j,w = whoseTurn((Board)position);
  for (i=nextSpotOfType((Board)position, 0,w);i!=-1;i=nextSpotOfType((Board)position, i+1,w))
    for (j=nextOpenSpot((Board)position, 0);j!=-1;j=nextOpenSpot((Board)position, j+1))
      // WARNING: HARD-CODED CONSTANT
      head = CreateMovelistNode(i | j<<5, head);
  return head;
}




/************************************************************************
** This is where you can put any helper functions, including your
** hash and unhash functions if you are not using one of the existing
** ones.
************************************************************************/
