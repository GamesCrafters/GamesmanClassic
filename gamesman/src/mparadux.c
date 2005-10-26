// $Id: mparadux.c,v 1.9 2005-10-26 09:37:54 yanpeichen Exp $

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
** 10/04/2005 David  - Added a bunch of simple functions--without testing
**                     and which are incomplete; just wanted to get
**                     something out before the meeting.
** 10/18/2005 Yanpei - Tidied up davidPrintPos(), wrote dyPrintPos() using
**                     more tidy code. Proof read hashMove() unhashMove()
** 10/26/2005 Yanpei - PrintPos and initializeGame for odd boards debugged. 
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

/* macros */
//#define nextPlayer(x) (((x) - 1) % 2 + 1)

#define nextPlayer(x) ((x) % 2 + 1)

/* pieces */
#define WHITE 1
#define BLACK 2
#define BLANK 0
#define X 1
#define O 2
#define INVALID 3

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


/* The actual board */
char *board;

/* Mapping from values to characters */
char valToChar[] = { '-', 'X', 'O', '*' };

/* Just for kicks, let the user choose his name.. and the computer's */
STRING playersName = "Humanoid";
STRING computerName = "Robbie";

/*************************************************************************
**
** #defines and structs
**
**************************************************************************/

/* Board Coordinates 

          row,col format          el(ement) format
   
          0,0  0,1  0,2              00  01  02

       1,0  1,1  1,2  1,3          03  04  05  06

     2,0  2,1  2,2  2,3  2,4     07  08  09  10  11

       3,0  3,1  3,2  3,3          12  13  14  15

         4,0  4,1  4,2               16  17  18


*/

/* Initial board - 

         Paradux mini                  Paradux regular

          X   O   X                     X   O   X   O

        O   -   -   -                 O   -   -   -   X

      X   -   -   -   0             X   -   -   X   -   O

        -   -   -   x             O   -   -   -   -   -   X

          0   x   0                 X   -   O   -   -   O

                                      O   -   -   -   X

                                        X   O   X   O
*/

/* MOVE encoding: (position << 6) | (direction << 3) | type

/* On the hexagonal board, only one side needs to be specified */
int boardSide = 3;

/* Magically generated (in InitializeGame) */
int boardSize;
int numX;
int numO;
int numBlank;

/* Other options */
int firstGo = X;

typedef struct board_item {

  short *slots;
  short turn;

} PARABOARD;

typedef PARABOARD* PBPtr;

// must be used on arithmetic expressions and comparables
// "x" and "y" should not contain side effects
#define max(x,y) ((x)>(y) ? (x) : (y))
#define min(x,y) ((x)<(y) ? (x) : (y))

// must be used on ints or POSITION or the like
#define abs(x) (0<(x) ? (x) : -(x))


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

void davidInitGame ();
void dyPrintPos(POSITION position, STRING playersName, BOOLEAN usersTurn);

void                    (*initGame)( ) = &davidInitGame;
void                    (*printPos)(POSITION position, STRING playersName, BOOLEAN usersTurn) = &dyPrintPos;

/* Support Functions */

PBPtr                   MallocBoard();
void                    FreeBoard(PBPtr);
MOVE                    hashMove(int type, int pos1, int pos2);
void                    unhashMove(MOVE move, int* type, int* pos1, int* pos2);

// returns TRUE if square u,v is next to square x,y
// where u,v and x,y are the row,column coordinates
// returns FALSE for invalid coordinates
BOOLEAN                 neighbor(int u, int v, int x, int y);

// if there is no neigher in the specified direction, return INVALID
int                     getNeighbor(int pos, int direction);

/* Direction in which the neighbor neighbors pos:
  0 NW
  1 NE
  2 E
  3 SE
  4 SW
  5 W
 -1 Not neighboring
*/

int                     neighboringDirection(int pos, int neighbor);

// returns slot number when given row and column coordinates
// returns -1 when given invalid coordinates
int                     rcToSlot(int r, int c);

// returns row and column coordinate when given slot number
// returns -1 when given invalid slot number
// return data format = r*100+c
int                     slotToRC(int s);

void                    getColRow(int pos, int* pCol, int* pRow);

int                     whoseMoveF(POSITION p);


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
  //printf("testpoint1\n");
  initGame();
  //printf("testpoint2\n");
}

/* Initialize odd-sided board */
void initOddBoard() {
  int i,row, col, el = 0, maxCol = boardSide - 1, maxRow = boardSide * 2 - 2;

  char* board = (char *) SafeMalloc (sizeof(char) * boardSize);


  for (row = 0; row <= maxRow; row++) {
    if (row < boardSide) {
      maxCol++;
    } else {
      maxCol--;
    }

    for (col = 0; col < maxCol; col++, el++) {
      if (row == 0) {
	board[el] = valToChar[col % 2 + 1];
      } else if (row == maxRow) {
	board[el] = valToChar[(col + 1) % 2 + 1];
      } else if (row <= boardSide - 2) {
	if (col == 0) {
	  board[el] = valToChar[row % 2 + 1];
	} else {
	  board[el] = valToChar[BLANK];
	}
      } else if (row >= boardSide) {
	if (col == maxCol-1) {
	  board[el] = valToChar[(row + 1) % 2 + 1];
	} else {
	  board[el] = valToChar[BLANK];
	}
      } else { // (row == boardSide-1)
	if (col == 0) {
	  board[el] = valToChar[row % 2 + 1];
	} else if (col == maxCol-1) {
	  board[el] = valToChar[(row+1) % 2 + 1];
	} else {
	  board[el] = valToChar[BLANK];
	}
      }
    }
  }
  /*
  for (i=0; i<boardSize; i++) {
    printf("%c",board[i]);
  }
  printf("\n");
  */

  gInitialPosition = generic_hash(board, firstGo);
  SafeFree(board);
}

/* Initialize even-sided board */
void initEvenBoard() {
  int row, col, el = 0, maxCol = boardSide - 1, maxRow = boardSide * 2 - 2;

  char* board = (char *) SafeMalloc (sizeof(char) * boardSize);

  for (row = 0; row <= maxRow; row++) {
    if (row < boardSide) {
      maxCol++;
    } else {
      maxCol--;
    }

    for (col = 0; col < maxCol; col++, el++) {
      if (row == 0 || row == maxRow) {
	board[el] = valToChar[col % 2 + 1];
      } else if (row == boardSide - 2) {
	if (col == 0 || col == boardSide - 1) {
	  board[el] = valToChar[X];
	} else if (col == maxCol-1) {
	  board[el] = valToChar[O];
	} else {
	  board[el] = valToChar[BLANK];
	}
      } else if (row == boardSide) {
	if (col == 0) {
	  board[el] = valToChar[X];
	} else if (col == maxCol-1 || col == boardSide - 2) {
	  board[el] = valToChar[O];
	} else {
	  board[el] = valToChar[BLANK];
	}
      } else {
	if (col == 0) {
	  board[el] = valToChar[row % 2 + 1];
	} else if (col == maxCol-1) {
	  board[el] = valToChar[(row+1) % 2 + 1];
	} else {
	  board[el] = valToChar[BLANK];
	}
      }
    }
  }

  gInitialPosition = generic_hash(board, firstGo);
  SafeFree(board);
}

void davidInitGame ()
{
  int pieces[10];

  if (boardSide < 3) {
    printf("ERROR: boards with side length < 3 cannot be represented or are trivial");
    exit(1);
  }

  /* 3(n-1)n + 1... I think */
  boardSize = 3 * (boardSide - 1) * boardSide + 1;

  //printf("%d\n", boardSize);

  /* boards with side length 1 don't follow this but we'll ignore that */
  if (boardSide % 2) { // odd board
    numX = numO = 2*boardSide - 1;
  } else { // even board
    numX = numO = (boardSide - 1) * 3 + 1;
  }
  numBlank = boardSize - numX - numO;

  pieces[0] = 'X';
  pieces[1] = numX;
  pieces[2] = numX;
  pieces[3] = 'O';
  pieces[4] = numO;
  pieces[5] = numO;
  pieces[6] = '-';
  pieces[7] = numBlank;
  pieces[8] = numBlank;
  pieces[9] = -1;

  //printf("testpoint4\n");

  gNumberOfPositions = generic_hash_init(boardSize, pieces, NULL);

  //printf("testpoint3\n");

  if (boardSide % 2) {
    initOddBoard();
  } else {
    initEvenBoard();
  }

  //printf("%d\n", gInitialPosition);

  PrintPosition(gInitialPosition, playersName, TRUE);
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

void AddMovesPerPair(char *board, int pos1, int pos2) {
}

MOVELIST *GenerateMoves (POSITION position)
{
  MOVELIST *moves = NULL;
    
  int i,j,curPiece;
  char *board = SafeMalloc(sizeof(char) * boardSize);
  int pos, nPos, whoseMove = whoseMoveF(position);
  char pieceA;
  char piece;
  char player = valToChar[whoseMove], otherPlayer = valToChar[nextPlayer(whoseMove)];

  /* Use CreateMovelistNode(move, next) to 'cons' together a linked list */

      for (j = 0; j < boardSide - 1; j++) {
	curPiece = board[getNeighbor(j, E)];

	if (piece != curPiece)
	  break;

	if (j == boardSide - 2) {
	  return (piece == player ? win : lose);
	}
      }

      // Check southeast
      for (j = 0; j < boardSide - 1; j++) {
	curPiece = board[getNeighbor(j, SE)];

	if (piece != curPiece)
	  break;

	if (j == boardSide - 2) {
	  return (piece == player ? win : lose);
	}
      }
      
      // Check southwest
      for (j = 0; j < boardSide - 1; j++) {
	curPiece = board[getNeighbor(j, SW)];

	if (piece != curPiece)
	  break;

	if (j == boardSide - 2) {
	  return (piece == player ? win : lose);
	}
      }


  for (i = 0; i < boardSize; i++) {
    pos = i;

    piece = board[i];

    if (piece != '-') {
      // Check east
      if (board[nPos = getNeighbor(i, E)] == otherPlayer) {
	AddMovesPerPair(board, pos, nPos);
      }

      // Check southeast
      if(board[nPos = getNeighbor(i, SE)] == otherPlayer) {
	AddMovesPerPair(board, pos, nPos);
      }

      // Check southwest
      if (board[nPos = getNeighbor(i, SW)] == otherPlayer) {
	AddMovesPerPair(board, pos, nPos);
      }
    }
  }
    
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
  int type, pos1, pos2, temp;

  char *board = SafeMalloc(sizeof(char) * boardSize);

  unhashMove(move, &type, &pos1, &pos2);
  generic_unhash(position, board);

  if (type == SWAP) {
    temp = board[pos1];
    board[pos1] = board[pos2];
    board[pos2] = temp;
  } else {
    temp = getNeighbor(pos1, type);
    board[temp] = board[pos1];
    board[pos1] = valToChar[BLANK];
    temp = getNeighbor(pos2, type);
    board[temp] = board[pos2];
    board[pos2] = valToChar[BLANK];
  }

  return generic_hash(board, nextPlayer(whoseMove(position)));
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
  char *board = SafeMalloc(sizeof(char) * boardSize);
  int pos, curPos;
  char player = valToChar[whoseMove(position)];
  char piece, curPiece;
  int i,j;

  generic_unhash(position, board);

  for (i = 0; i < boardSize; i++) {
    pos = i;

    piece = board[i];

    if (piece != '-') {
      // Check east
      curPos = pos;

      for (j = 0; j < boardSide - 1; j++) {
	curPos = getNeighbor(curPos, E);

	curPiece = board[curPos];

	if (piece != curPiece)
	  break;

	if (j == boardSide - 2) {
	  return (piece == player ? win : lose);
	}
      }

      // Check southeast
      curPos = pos;

      for (j = 0; j < boardSide - 1; j++) {
	curPos = getNeighbor(curPos, SE);

	curPiece = board[curPos];

	if (piece != curPiece)
	  break;

	if (j == boardSide - 2) {
	  return (piece == player ? win : lose);
	}
      }
      
      // Check southwest
      curPos = pos;

      for (j = 0; j < boardSide - 1; j++) {
	curPos = getNeighbor(curPos, SW);

	curPiece = board[curPos];

	if (piece != curPiece)
	  break;

	if (j == boardSide - 2) {
	  return (piece == player ? win : lose);
	}
      }
    }
  }

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

  printPos(position, playersName, usersTurn);

}

void davidPrintPos(POSITION position, STRING playersName, BOOLEAN usersTurn) 
{

  printf("It is %s's turn.\n\n", (usersTurn ? playersName : computerName));

  int initEl, el = 0;
  int totalCols = boardSide;
  int row, col, i;

  char* board = SafeMalloc(sizeof(char) * boardSize);

  generic_unhash(position, board);

  for (row = 0; row < boardSide * 2 - 1; row++) {
    initEl = el;

    // Leading spaces
    for (i = 0; i < boardSide - row; i++) {
      printf(" ");
    }

    for (col = 0; col < totalCols; col++, el++) {
      printf("%c ", board[el]);
    }
    // Spaces between board and divider
    for (i = 0; i < boardSide - row - 1; i++) {
      printf("------");
    }

    el = initEl;

    // Spaces between divider and legend; and legend
    if (boardSide < 6) {
      for (i = 0; i < boardSide - row; i++) {
	printf("     ");
      }

      for (col = 0; col < totalCols; col++, el++) {
	printf("%2d", el);
      }
    } else {
      for (i = 0; i < boardSide - row; i++) {
	printf("  ");
      }

      for (col = 0; col < totalCols; col++, el++) {
	printf("%3d", el);
      }
    }

    printf("\n\n");
  }

  printf("+---------------------------+------------------------------+\n");
  printf("|        MOVEMENT KEY       |          PREDICTION          |\n");
  printf("|                           |                              |\n");
  printf("|   NW  N  NE               |                              |\n");
  printf("|     \\ | /                 |                              |\n");
  printf("|   E - + - W   or   SWAP   |  %-26s  |\n", GetPrediction(position,playersName,usersTurn));
  printf("|     / | \\                 |                              |\n");
  printf("|   SW  S  SE               |                              |\n");
  printf("|                           |                              |\n");
  printf("+---------------------------+------------------------------+\n\n");

  SafeFree(board);

}

void dyPrintPos(POSITION position, STRING playersName, BOOLEAN usersTurn) 
{

  printf("It is %s's turn.\n\n", (usersTurn ? playersName : computerName));

  //printf("%d\n", position);

  int el = 0, initEl;
  int row, col, i, totalCols = boardSide;

  char* board = (char*) SafeMalloc(sizeof(char) * boardSize);
  generic_unhash(position, board);

  /*
  //board[0] = 'a';
  //board[1] = 'a';
  printf ("testpoint8\n");
  for (i=0; i<boardSize; i++) {
    //printf("testpoint9 %d\n", i);
    printf("%c",board[i]);
  }
  printf("\n");
  */
  //printf("%d %d %d\n",abs(-1+1),abs(-2+1),abs(-1+2));

  for (row = 0; row < boardSide * 2 - 1; row++, (row<boardSide ? totalCols++ : totalCols--)) {
    initEl = el;
  
    // Leading spaces
    printf("     ");

    for (i = 0; i < abs(boardSide - row - 1); i++) {
      printf("  ");
    }
    // Columns
    for (col = 0; col < totalCols; col++, el++) {
      printf("%c   ", board[el]);
    }
    // Spaces between board and divider
    for (i = 0; i < abs(boardSide - row - 1); i++) {
      printf("  ");
    }

    printf("         ");
    el = initEl;
    
    // Spaces between divider and legend; and legend
    if (boardSide < 6) {
      // less than 100 slots
      for (i = 0; i < abs(boardSide - row - 1); i++) {
	printf("  ");
      }
      for (col = 0; col < totalCols; col++, el++) {
	printf("%2d  ", el);
      }
    } else {
      // more than 100 slots
      for (i = 0; i < abs(boardSide - row); i++) {
	printf("   ");
      }
      for (col = 0; col < totalCols; col++, el++) {
	printf("%3d  ", el);
      }
    } // end lengend
    
    printf("\n\n");
  } // end for (row = 0 ...)
        
  printf("+---------------------------+------------------------------+\n");
  printf("|        MOVEMENT KEY       |          PREDICTION          |\n");
  printf("|                           |                              |\n");
  printf("|   NW  N  NE               |                              |\n");
  printf("|     \\ | /                 |                              |\n");
  printf("|   E - + - W   or   SWAP   |  %-26s  |\n", GetPrediction(position,playersName,usersTurn));
  printf("|     / | \\                 |                              |\n");
  printf("|   SW  S  SE               |                              |\n");
  printf("|                           |                              |\n");
  printf("+---------------------------+------------------------------+\n\n");
       
  SafeFree(board);

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

MOVE hashMove (int type, int pos1, int pos2)
{
  int nDirection = neighboringDirection(pos1, pos2);

  return (pos1 << 6) | (nDirection << 3) | type;
}

void unhashMove (MOVE move, int* type, int* pos1, int* pos2) {
  *pos1 = move >> 6;
  *pos2 = getNeighbor(*pos1, (move >> 3) & 7);

  *type = move & 7;
}

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

// if there is no neigher in the specified direction, return INVALID
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

  getColRow(pos, &pCol, &pRow);
  getColRow(pos, &nCol, &nRow);

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
    printf("**** Error -- rcToSlot: Invalid r,c ****");
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
    c = (s--);
  } else {
    c = (s-(z-y+1)+1)%(y+1);
  }

  return 0;

}

void getColRow(int pos, int* pCol, int* pRow) {
  int col = 0, row = 0, rowSize = boardSide, numEls = 0;

  for (; pos >= numEls + rowSize; col++, numEls += rowSize, rowSize += (col > boardSize - 1 ? -1 : 1));

  //  for (; pos < (numEls - rowSize); col++, rowSize += (col > boardSize - 1 ? -1 : 1), numEls += rowSize);

  *pCol = col;
  //  *pRow = pos - numEls + rowSize;

  *pRow = pos - numEls;
}

int whoseMoveF(POSITION p) {
  return p;
}

// $Log: not supported by cvs2svn $
// Revision 1.8  2005/10/18 08:34:01  yanpeichen
// ** 10/18/2005 Yanpei - Tidied up davidPrintPos(), wrote dyPrintPos() using
// **                     more tidy code. Proof read hashMove() unhashMove()
// **
//
// Revision 1.7  2005/10/05 03:23:55  trikeizo
// Added a bunch of small functions, untested.
//
// Revision 1.6  2005/09/28 06:17:18  yanpeichen
// *** empty log message ***
//
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

