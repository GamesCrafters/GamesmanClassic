/*****************************************************************************
 **
 ** NAME:        mrcheckers.c
 **
 ** DESCRIPTION: Rubik's Checkers
 **
 ** AUTHOR:      Johnny Tran
 **              Steve Wu
 **
 ** DATE:        Sunday, February 26 2006
 **
 ** UPDATE HIST:
 ** 3/19/2006  	 fixed DoMove, added ConvertTextInputToMove, printMove(for debugging)
 **              Problems with internal row representation: printposition prints "bottom-up",
 **              while directional functions use implied "top-down."
 ** 3/12/2006    Modified Forward-directions to return index instead of char
 ** 2/26/2006    GenerateMoves, DoMoves, forward-directions
 ** 2/13/2006    Started. Much of the code is shamelessly borrowed from
 **              m1210.c, which is used as a template. Done: InitializeGame,
 **              PrintPosition
 **
 *****************************************************************************/

/*****************************************************************************
 **
 ** Everything below here must be in every game file
 **
 *****************************************************************************/
#include <stdio.h>
#include "gamesman.h"

POSITION gNumberOfPositions   = 0;      /* Calculated in InitializeGame */
POSITION kBadPosition         = -1;

POSITION gInitialPosition     = 0;      /* Calculated in InitializeGame */

//POSITION gMinimalPosition     = 0;       /* TODO: ? */

STRING   kAuthorName          = "Johnny Tran and Steve Wu";
STRING   kGameName            = "Rubik's Checkers";
BOOLEAN  kPartizan            = TRUE;
BOOLEAN  kDebugMenu           = TRUE;
BOOLEAN  kGameSpecificMenu    = FALSE;
BOOLEAN  kTieIsPossible       = FALSE;  /* TODO: Can both players be locked? */
BOOLEAN  kLoopy               = TRUE;
BOOLEAN  kDebugDetermineValue = FALSE;
void*    gGameSpecificTclInit = NULL;

STRING   kHelpGraphicInterface = "";    /* TODO */

STRING   kHelpTextInterface    =
"Play like checkers, except only kings can capture, men can only move backwards\n and cannot capture, and jumping a piece demotes a king to a man and captures a man.";

STRING   kHelpOnYourTurn =
"Select a piece and its destination(s). (i.e. a3-c5-e3 to double-capture; a1+\nto promote)";

STRING   kHelpStandardObjective =
"Eliminate all your opponent's pieces or block them from moving.";

STRING   kHelpReverseObjective =
"Eliminate all your pieces or block them from moving.";

STRING   kHelpTieOccursWhen = ""; /* empty since kTieIsPossible == FALSE */

STRING   kHelpExample =           /* TODO */
"Help Example";

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

// External Functions
extern GENERIC_PTR      SafeMalloc();
extern void             SafeFree();
extern POSITION         generic_hash_init(int boardsize, int *pieces_array, int (*fn)(int *));
extern POSITION         generic_hash(char* board, int player);
extern char             *generic_unhash(POSITION hash_number, char *empty_board);
extern int              whoseMove (POSITION hashed);



// How a piece can move
#define FORWARD               1
#define BACKWARD              2
#define CAPTURE               4

// Player representation
#define P1                    1
#define P2                    2

//Piece Movement representation
#define FORWARDLEFT           0
#define FORWARDRIGHT          1
#define BACKWARDRIGHT         2
#define BACKWARDLEFT          3

// Piece representation
#define EMPTY                 ' '
#define P1KING                'G'
#define P1MAN                 'g'
#define P2KING                'O'
#define P2MAN                 'o'
#define P1NAME                "Green"
#define P2NAME                "Orange"

//POSSIBLY TEMPORARY:
unsigned int currentTurn = P1;

// How large the board is
unsigned int rows           = 6;  // Rubik's: 6, Checkers: 8
unsigned int cols           = 2;  // Rubik's: 4, Checkers: 4
unsigned int boardSize;     /* initialized in InitializeGame */

// How many rows closest to a player start out with pieces (< ROWS/2)
unsigned int startRows      = 1;

// Rubik's Checkers specific options
// You can change to regular checkers rules by changing these options
BOOLEAN demote              = TRUE;      // Capturing demotes a piece
unsigned int kingMobility   = (FORWARD | BACKWARD | CAPTURE);
unsigned int manMobility    = BACKWARD;  // Man can only move backwards
BOOLEAN forceCapture        = FALSE;     // No forced captures
BOOLEAN startPromoted       = TRUE;      // Starting pieces are already kings
unsigned int promoteRow     = BACKWARD;  // Promote on your row

/*
typedef enum square {
    empty, orangeKing, orangeMan, greenKing, greenMan
} squareState;
*/

//number of bits to use to represent board index in move hash
unsigned int MVHASHACC = 8;

void InitializeGame()
{
    int maxPieces = startRows * cols;
    boardSize = rows * cols;
    int pieces[] = { P1KING, 0, maxPieces,
                     P1MAN,  0, maxPieces,
                     P2KING, 0, maxPieces,
                     P2MAN,  0, maxPieces,
                     EMPTY,  boardSize-(maxPieces*2), boardSize-1,
                     -1 };
    char* initialPosition = (char*)SafeMalloc(boardSize * sizeof(char));
    int i;
    
    gNumberOfPositions = generic_hash_init(boardSize, pieces, NULL);

    // Create initial position
    for (i = 0; i < maxPieces; i++) {
        initialPosition[i] = (startPromoted ? P1KING : P1MAN);
        initialPosition[boardSize-1 - i] = (startPromoted ? P2KING : P2MAN);
    }
    
    for (i = maxPieces; i < (boardSize - maxPieces); i++) {
        initialPosition[i] = EMPTY;
    }       
    gInitialPosition = generic_hash(initialPosition, P1);
    SafeFree(initialPosition);
}

void FreeGame()
{
}

int forwardRight(int whosTurn, int currentIndex){
   int isEvenRow = ((currentIndex/cols)%2 == 0),
     isBegOfRow = ((currentIndex%cols) == 0),
     isLastOfRow = ((currentIndex%cols) == cols-1),
     isFirstRow = ((currentIndex/cols) == 0),
     isLastRow = ((currentIndex/cols) == rows-1);

   if(whosTurn == P1){
     if(!isLastRow){
       if(isEvenRow && !isBegOfRow)
	 return currentIndex + cols-1;
       else if(!isEvenRow)
	 return currentIndex + cols;
     }
   }
   else{ //P2
     if(!isFirstRow){
       if(!isEvenRow && !isLastOfRow)//odd row, not last column
	 return currentIndex - cols+1;
       else if(isEvenRow)

	 return currentIndex - cols;
     }
   }
   return -1;//forward-right DNE;
}

int forwardLeft(int whosTurn, int currentIndex){
  int isEvenRow = ((currentIndex/cols)%2 == 0),
    isBegOfRow = ((currentIndex%cols) == 0),
    isLastOfRow = ((currentIndex%cols) == cols-1),
    isFirstRow = ((currentIndex/cols) == 0),
    isLastRow = ((currentIndex/cols) == rows-1);
  
  if(whosTurn == P1){
    if(!isLastRow){
      if(!isEvenRow && !isLastOfRow)//odd row, not last column
	return currentIndex + cols + 1;
      else if(isEvenRow)
	return currentIndex + cols;
    }
  }
  else{ //P2
    if(!isFirstRow){
      if(isEvenRow && !isBegOfRow)
	return currentIndex - cols - 1;
      else if(!isEvenRow)
	return currentIndex - cols;
    }
  }
  return -1;//forward-left DNE;
}

int backwardLeft(int whosTurn, int currentIndex){
  return forwardRight(whosTurn%2 + 1, currentIndex);
}

int backwardRight(int whosTurn, int currentIndex){
  return forwardLeft(whosTurn%2 + 1, currentIndex);
}

int oppositeMove(int previousMove){
  return (previousMove+2)%4;
}

POSITION unHashMove(POSITION myPosition, int theMove)
{
  char thePosition[boardSize];
  generic_unhash(myPosition, thePosition);
  int done = FALSE;
  unsigned int index = theMove >> (32-MVHASHACC), whosTurn = whoseMove(myPosition);
  char myPiece = thePosition[index];
  theMove = theMove << MVHASHACC;
  unsigned int nextMove = (theMove >> 30)&0x00000003, previousMove = nextMove;
  unsigned int myForwardRight = forwardRight(whosTurn, index), myForwardLeft = forwardLeft(whosTurn, index), myBackwardRight = backwardRight(whosTurn, index), myBackwardLeft = backwardLeft(whosTurn, index);
  int opposingKing = (whosTurn == P1 ? P2KING:P1KING);
  int opposingMan = (whosTurn == P1 ? P2MAN:P1MAN);
  int isFirstRow = ((index/cols) == 0),
    isLastRow = ((index/cols) == rows-1);  
   
  if(promoteRow == BACKWARD){
    if(thePosition[index] == P1MAN){
      if(isFirstRow){
	thePosition[index] = P1KING;
	done = TRUE;
      }
    }
    else if(thePosition[index] == P2MAN){
      if(isLastRow){
	thePosition[index] = P2KING;
	done = TRUE;
      }
    }
  }
  else if(promoteRow == FORWARD){
    if(thePosition[index] == P1MAN){
      if(isLastRow){
	thePosition[index] = P1KING;
	done = TRUE;
      }
    }
    else if(thePosition[index] == P2MAN){
      if(isFirstRow){
	thePosition[index] = P2KING;
	done = TRUE;
      }
    }
  }
  while(!done){
    myForwardRight = forwardRight(whosTurn, index), myForwardLeft = forwardLeft(whosTurn, index), myBackwardRight = backwardRight(whosTurn, index), myBackwardLeft = backwardLeft(whosTurn, index);
    thePosition[index] = EMPTY;
    switch(nextMove){
    case FORWARDRIGHT: 
      if(thePosition[myForwardRight] == EMPTY){
	index = myForwardRight;
	done = TRUE;
      }
      else{
	thePosition[forwardRight(whosTurn, myForwardRight)] = thePosition[index];
	if(thePosition[myForwardRight] == opposingMan)
	  thePosition[myForwardRight] = EMPTY;		    
	else if (thePosition[myForwardRight] == opposingKing)
	  thePosition[myForwardRight] = opposingMan; 
	index = forwardRight(whosTurn, myForwardRight);     
      }
      break;
    case FORWARDLEFT:
      if(thePosition[myForwardLeft] == EMPTY){
	index = myForwardLeft;
	done = TRUE;	
      }
      else{
	thePosition[forwardLeft(whosTurn, myForwardLeft)] = thePosition[index];
	if(thePosition[myForwardLeft] == opposingMan)
	  thePosition[myForwardLeft] = EMPTY;		    
	else if (thePosition[myForwardLeft] == opposingKing)
	  thePosition[myForwardLeft] = opposingMan;      
	index = forwardLeft(whosTurn, myForwardLeft);
      }
      break;
    case BACKWARDLEFT:
      if(thePosition[myBackwardLeft] == EMPTY){
	index = myBackwardLeft;
	done = TRUE;
      }	
      else{
	thePosition[backwardLeft(whosTurn, myBackwardLeft)] = thePosition[index];
	if(thePosition[myBackwardLeft] == opposingMan)
	  thePosition[myBackwardLeft] = EMPTY;		    
	else if (thePosition[myBackwardLeft] == opposingKing)
	  thePosition[myBackwardLeft] = opposingMan;      
	index = backwardLeft(whosTurn, myBackwardLeft);
      }
      break;
    case BACKWARDRIGHT:
      if(thePosition[myBackwardRight] == EMPTY){
	index = myBackwardRight;
	done = TRUE;
      }
      else{
	thePosition[backwardRight(whosTurn, backwardRight(whosTurn, index))] = thePosition[index];
	if(thePosition[myBackwardRight] == opposingMan)
	  thePosition[myBackwardRight] = EMPTY;		    
	else if (thePosition[myBackwardRight] == opposingKing)
	  thePosition[myBackwardRight] = opposingMan; 	
	index = backwardRight(whosTurn, myBackwardRight);
      }       
      break;
    }
    thePosition[index] = myPiece;
    previousMove = nextMove;
    theMove = theMove << 2;
    nextMove = (theMove >> 30)&0x00000003;
    if(oppositeMove(previousMove)==nextMove)
      done = TRUE;
  }
  currentTurn = whosTurn = (whosTurn == P1 ? P2:P1);//switch players
  return generic_hash(thePosition, whosTurn);
}
  
void undoCapture(char *initialPosition, int whosTurn, int index, int previousMove){
  int myForwardRight = forwardRight(whosTurn, index), myForwardLeft = forwardLeft(whosTurn, index), myBackwardRight = backwardRight(whosTurn, index), myBackwardLeft = backwardLeft(whosTurn, index);  
  int opposingMan, opposingKing;
  if(whosTurn == P1){
    opposingMan = P2MAN;
    opposingKing = P2KING;
  }
  else{
    opposingMan = P1MAN;
    opposingKing = P1KING;
  }
  switch(previousMove){
  case FORWARDRIGHT:
    if(initialPosition[myBackwardLeft] == EMPTY)
      initialPosition[myBackwardLeft] = opposingMan;
    else if(initialPosition[myBackwardLeft] == opposingMan)
      initialPosition[myBackwardLeft] = opposingKing;
    initialPosition[backwardLeft(whosTurn, myBackwardLeft)] = initialPosition[index];
    break;
  case FORWARDLEFT:
    if(initialPosition[myBackwardRight] == EMPTY)
      initialPosition[myBackwardRight] = opposingMan;
    else if(initialPosition[myBackwardRight] == opposingMan)
      initialPosition[myBackwardRight] = opposingKing;
    initialPosition[backwardRight(whosTurn, myBackwardRight)] = initialPosition[index];
    break;
  case BACKWARDRIGHT:
    if(initialPosition[myForwardLeft] == EMPTY)
      initialPosition[myForwardLeft] = opposingMan;
    else if(initialPosition[myForwardLeft] == opposingMan)
      initialPosition[myForwardLeft] = opposingKing;
    initialPosition[forwardLeft(whosTurn, myForwardLeft)] = initialPosition[index];
    break;
  case BACKWARDLEFT:
    if(initialPosition[myForwardRight] == EMPTY)
      initialPosition[myForwardRight] = opposingMan;
    else if(initialPosition[myForwardRight] == opposingMan)
      initialPosition[myForwardRight] = opposingKing;
    initialPosition[forwardRight(whosTurn, myForwardRight)] = initialPosition[index];
    break;
  }
}

/************************************************************************
**
** NAME:        DebugMenu
**
** DESCRIPTION: Menu used to debug internal problems. Does nothing if
**              kDebugMenu == FALSE
** 
************************************************************************/

void DebugMenu() { }

/************************************************************************
**
** NAME:        GameSpecificMenu
**
** DESCRIPTION: Menu used to change game-specific parmeters, such as
**              the side of the board in an nxn Nim board, etc. Does
**              nothing if kGameSpecificMenu == FALSE
** 
************************************************************************/

void GameSpecificMenu() {
    // TODO: Allow changing of the board size and rules
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
  /* No need to have anything here, we have no extra options */
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
************************************************************************/

POSITION DoMove(thePosition, theMove)
     POSITION thePosition;
     MOVE theMove;
{
  return unHashMove(thePosition, theMove);
}

/************************************************************************
**
** NAME:        GetInitialPosition
**
** DESCRIPTION: Ask the user for an initial position for testing. Store
**              it in the space pointed to by initialPosition;
** 
** INPUTS:      POSITION initialPosition : The position to fill.
**
************************************************************************/

POSITION GetInitialPosition()
{
    // TODO
    return gInitialPosition;
}

/************************************************************************
**
** NAME:        PrintComputersMove
**
** DESCRIPTION: Nicely format the computers move.
** 
** INPUTS:      MOVE   *computersMove : The computer's move. 
**              STRING  computersName : The computer's name. 
**
************************************************************************/

void PrintComputersMove(computersMove,computersName)
     MOVE computersMove;
     STRING computersName;
{
    // TODO
}

/************************************************************************
**
** NAME:        CountPieces
**
** DESCRIPTION: Count how many pieces each player has.
** 
** INPUTS:      char board[]: the board to inspect
**              unsigned int *p1Pieces: holds number of pieces Player 1 has
**              unsigned int *p2Pieces: holds number of pieces Player 2 has
**
** OUTPUTS:     total number of pieces
**
************************************************************************/

unsigned int CountPieces(char board[],
                         unsigned int *p1Pieces, unsigned int *p2Pieces) {
    unsigned int i;
    
    *p1Pieces = *p2Pieces = 0;
    
    // Check if one player has no more pieces
    for (i = 0; i < boardSize; i++) {
        switch (board[i]) {
        case P1KING:
        case P1MAN:
            (*p1Pieces)++;
            break;
        case P2KING:
        case P2MAN:
            (*p2Pieces)++;
        }
    }
    
    return *p1Pieces + *p2Pieces;
}


/************************************************************************
**
** NAME:        Primitive
**
** DESCRIPTION: Return the value of a position if it fulfills certain
**              'primitive' constraints. Some examples of this is having
**              three-in-a-row with TicTacToe. TicTacToe has two
**              primitives it can immediately check for, when the board
**              is filled but nobody has one = primitive tie. Three in
**              a row is a primitive lose, because the player who faces
**              this board has just lost. I.e. the player before him
**              created the board and won. Otherwise undecided.
** 
** INPUTS:      POSITION position : The position to inspect.
**
** OUTPUTS:     (VALUE) an enum which is oneof: (win,lose,tie,undecided)
**
**              We assume Player 1 is always the human
**              No more pieces: primitive lose
**              All Locked pieces: primitive lose
**              Else: undecided
**
************************************************************************/

VALUE Primitive(position) 
     POSITION position;
{
    char board[boardSize];
    unsigned int p1Pieces, p2Pieces;
    int whosTurn = whoseMove(position);
    // Check for no more pieces
    CountPieces(board, &p1Pieces, &p2Pieces);
    if(whosTurn == P1){
      if (&p1Pieces == 0) return lose;  // Player 1 has no more pieces
    }
    else if(&p2Pieces == 0) return lose;// P2 has no more pieces
    
    // TODO: Check for all pieces being locked (unable to move)
    if(GenerateMoves(position) == NULL)
      return lose; //undecided;
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
************************************************************************/

void PrintPosition(position,playerName,usersTurn)
     POSITION position;
     STRING playerName;
     BOOLEAN  usersTurn;
{
    char board[boardSize];
    int player;
    unsigned int i, j, k = 0;
    unsigned int p1Pieces, p2Pieces;
    
    generic_unhash(position, board);  // Obtain board state
    player = whoseMove(position);
    
    printf("RUBIK'S CHECKERS\n\n  ");
    
    // Print column letters **POSSIBLE OVERFLOW**
    for (i = 0; i < (cols*2); i++) {
        printf("%c", 'a' + i);
    }
    
    printf("\n --");
    for (i = 0; i < (cols*2); i++) printf("-");
    printf("\n");
    
    CountPieces(board, &p1Pieces, &p2Pieces);

    for (i = rows; i > 0; i--) {
        printf("%d|", i);  // Row number
        if ((i % 2) != 0) printf("%c", EMPTY);  // Shift alternating rows
        
        for (j = 0; j < cols; j++) {
            // Print square
            printf("%c", board[k++]);
            
            // Print empty squares in between
            if ((j != (cols-1)) || (i % 2) == 0) printf("%c", EMPTY);
        }
        printf("|");
        if (i == rows) {  // Print player 2's number of pieces
            printf("    %-8s: %d pieces", P2NAME, p2Pieces);
            if (player == P2) printf(" (%s's turn)", P2NAME);
        } else if (i == 1) {  // Print player 1's number of pieces
            printf("    %-8s: %d pieces", P1NAME, p1Pieces);
            if (player == P1) printf(" (%s's turn)", P1NAME);
        }
        printf("\n");
    }
    
    printf(" --");
    for (i = 0; i < (cols*2); i++) printf("-");
    printf("\n");
}



/************************************************************************
**
** NAME:        makeMove
**
** DESCRIPTION: Finds and creates integer representations of moves(not captures),
**              and appends these moves to the list.
**              
**              The first n=MVHASHACC bits represent the index, and the rest of the
**              bits represent, in pairs of bits, the directions of movement.
**              Move termination is represented by a repeat of the previous move.
** 
** INPUTS:      char *initialPosition : The current board.
**              int whosTurn          : Self-explanatory, P1 or P2.
**              int currentIndex      : Index of the piece to be moved.
**              MOVELIST *head        : The list to be added to.
** OUTPUTS:     NONE
**
** CALLS:      
**
************************************************************************/
//currentIndex
MOVELIST *makeMove(char *initialPosition, int whosTurn, int currentIndex, MOVELIST *head){
  unsigned int myMove = (currentIndex << (32-MVHASHACC));
  int whatIsForward, currentMobility;
//int isEndOfRow, isBegOfRow, isEvenRow;
  if(whosTurn == P1){
    whatIsForward = 1;
  }
  else{
    whatIsForward = -1;
  }

  if(initialPosition[currentIndex] == P1KING || initialPosition[currentIndex] == P2KING)
    currentMobility = kingMobility;
  else if(initialPosition[currentIndex] == P1MAN || initialPosition[currentIndex] == P2MAN)
    currentMobility = manMobility;

  if((currentMobility&FORWARD) == FORWARD){
    if(forwardLeft(whosTurn, currentIndex)!= -1 && initialPosition[forwardLeft(whosTurn, currentIndex)] == EMPTY)
      head = CreateMovelistNode(myMove|(FORWARDLEFT<<(32-MVHASHACC-2))|(BACKWARDRIGHT<<(32-MVHASHACC-4)), head);
    if(forwardRight(whosTurn, currentIndex)!= -1 && initialPosition[forwardRight(whosTurn, currentIndex)] == EMPTY)
      head = CreateMovelistNode(myMove|(FORWARDRIGHT<<(32-MVHASHACC-2))|(BACKWARDLEFT<<(32-MVHASHACC-4)), head);
  }
  if((currentMobility&BACKWARD) == BACKWARD){
    if(backwardLeft(whosTurn, currentIndex)!= -1 && initialPosition[backwardLeft(whosTurn, currentIndex)] == EMPTY)
      head = CreateMovelistNode(myMove|(BACKWARDLEFT<<(32-MVHASHACC-2))|(FORWARDRIGHT<<(32-MVHASHACC-4)), head);
    if(backwardRight(whosTurn, currentIndex)!= -1 && initialPosition[backwardRight(whosTurn, currentIndex)] == EMPTY)
      head = CreateMovelistNode(myMove|(BACKWARDRIGHT<<(32-MVHASHACC-2))|(FORWARDLEFT<<(32-MVHASHACC-4)), head);
  }
  return head;
}





//traverse board recursively, adding moves to currentMove.  When cannot find more moves, add currentMove to MOVELIST
//and then undo changes to initialPosition
MOVELIST *makeCapture(char *initialPosition, int whosTurn, int currentIndex, MOVELIST *head, unsigned int currentMove, int offset){
  unsigned int previousMove = (currentMove<<(MVHASHACC+offset-4))>>30;
  int opposingMan, opposingKing, currentMobility;
  int myForwardRight = forwardRight(whosTurn, currentIndex), 
    myForwardLeft = forwardLeft(whosTurn, currentIndex),
    myBackwardRight = backwardRight(whosTurn, currentIndex),
    myBackwardLeft = backwardLeft(whosTurn, currentIndex);

  if(whosTurn == P1){
    opposingMan = P2MAN;
    opposingKing = P2KING;
  }
  else{
    opposingMan = P1MAN;
    opposingKing = P1KING;
  }

  if(initialPosition[currentIndex] == P1KING || initialPosition[currentIndex] == P2KING)
    currentMobility = kingMobility;
  else if(initialPosition[currentIndex] == P1MAN || initialPosition[currentIndex] == P2MAN)
    currentMobility = manMobility;  
  
  if((currentMobility&CAPTURE) == CAPTURE){
    if((currentMobility&FORWARD) == FORWARD){
      if(myForwardLeft!=-1 && ((initialPosition[myForwardLeft]==opposingMan)||(initialPosition[myForwardLeft]==opposingKing))){
	if(forwardLeft(whosTurn, myForwardLeft)!=-1 && initialPosition[forwardLeft(whosTurn, myForwardLeft)]==EMPTY){
	  head = makeCapture(initialPosition, whosTurn,
			     forwardLeft(whosTurn, myForwardLeft),
			     head, currentMove|(FORWARDLEFT<<(32-MVHASHACC-offset)),
			     offset+2);
	}       
      }
      if(myForwardRight!=-1 && ((initialPosition[myForwardRight]==opposingMan)||(initialPosition[myForwardRight]==opposingKing))){
	if(forwardRight(whosTurn, myForwardRight)!=-1 && initialPosition[forwardRight(whosTurn, myForwardRight)]==EMPTY){
	  head = makeCapture(initialPosition, whosTurn,
			     forwardRight(whosTurn, myForwardRight),
			     head, currentMove|(FORWARDRIGHT<<(32-MVHASHACC-offset)),
			     offset+2);		  
	}
      }
    }
    if((currentMobility&BACKWARD) == BACKWARD){
      if(myBackwardRight!=-1 && ((initialPosition[myBackwardRight]==opposingMan)||(initialPosition[myBackwardRight]==opposingKing))){
	if(backwardRight(whosTurn, myBackwardRight)!=-1 && initialPosition[backwardRight(whosTurn, myBackwardRight)]==EMPTY){
	  head = makeCapture(initialPosition, whosTurn,
			     backwardRight(whosTurn, myBackwardRight),
			     head, currentMove|(BACKWARDRIGHT<<(32-MVHASHACC-offset)),
			     offset+2);
	}
      }
      if(myBackwardLeft!=-1 && ((initialPosition[myBackwardLeft]==opposingMan)||(initialPosition[myBackwardLeft]==opposingKing))){
	if(backwardLeft(whosTurn, myBackwardLeft)!=-1 && initialPosition[backwardLeft(whosTurn, myBackwardLeft)]==EMPTY){
	  head = makeCapture(initialPosition, whosTurn,
			     backwardLeft(whosTurn, myBackwardLeft),
			     head, currentMove|(BACKWARDLEFT<<(32-MVHASHACC-offset)),
			     offset+2);
	}
      }
    }
  }
  if(offset>2){
    undoCapture(initialPosition, whosTurn, currentIndex, previousMove); 
    head = CreateMovelistNode(currentMove|(oppositeMove(previousMove)<<(32-MVHASHACC-offset)), head);
  }
  return head;
}
    
    
MOVELIST *makePromote(char *initialPosition, int whosTurn, int currentIndex, MOVELIST *head){
  int isFirstRow = ((currentIndex/cols) == 0),
    isLastRow = ((currentIndex/cols) == rows-1);  
  if(promoteRow == BACKWARD){
    if(initialPosition[currentIndex] == P1MAN){
      if(isFirstRow)
	head = CreateMovelistNode((currentIndex<<(32-MVHASHACC)), head);
    }
    else if(initialPosition[currentIndex] == P2MAN){
      if(isLastRow)
	head = CreateMovelistNode((currentIndex<<(32-MVHASHACC)), head);
    }
  }
  else if(promoteRow == FORWARD){
    if(initialPosition[currentIndex] == P1MAN){
      if(isLastRow)
	head = CreateMovelistNode((currentIndex<<(32-MVHASHACC)), head);
    }
    else if(initialPosition[currentIndex] == P2MAN){
      if(isFirstRow)
	head = CreateMovelistNode((currentIndex<<(32-MVHASHACC)), head);
    }
  }
  return head;
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
** CALLS:       MOVELIST *CreateMovelistNode(MOVE,MOVELIST *)
**
************************************************************************/

//need to write makePromote
//
MOVELIST *GenerateMoves(position)
     POSITION position;
{
    MOVELIST *head = NULL;
    MOVELIST *CreateMovelistNode();
    char initialPosition[boardSize];
    //char tempPositions[boardSize];

    int whosTurn = whoseMove(position);
    char currentKing, currentMan;
    int i;
  
    generic_unhash(position, initialPosition);
    if(whosTurn == P1){
      currentKing = P1KING;
      currentMan = P1MAN;
    }
    else{
      currentKing = P2KING;
      currentMan = P2MAN;
    }
    for(i = 0; i < boardSize; i++){
      if((initialPosition[i] == currentKing) || (initialPosition[i] == currentMan)){
	head = makeMove(initialPosition, whosTurn, i, head);
	head = makePromote(initialPosition, whosTurn, i, head);
	head = makeCapture(initialPosition, whosTurn, i, head, (i<<(32-MVHASHACC)), 2);
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
  USERINPUT ret, HandleDefaultTextInput();
  BOOLEAN ValidMove();
  char input[2];
  // TODO
  
  input[0] = '3';
  do{
    printf("%8s's move [(u)ndo/1/2] : ", playerName);
    ret = HandleDefaultTextInput(thePosition, theMove, playerName);
    if(ret != Continue)
      return(ret);
  }
  while (TRUE);
  return(Continue); /* this is never reached, but lint is now happy */
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
** OUTPUTS:     BOOLEAN : TRUE iff the input is a valid text input.
**
************************************************************************/

BOOLEAN ValidTextInput(input)
     STRING input;
{
    // TODO
    return(TRUE);
}



int getIndexFromText(char currentRow, char currentCol){
  return -((currentRow-'0')-rows)*cols + (currentCol-'a')/2;
}

/************************************************************************
**
** NAME:        ConvertTextInputToMove
**
** DESCRIPTION: Convert the string input to the internal move representation.
** 
** INPUTS:      STRING input : The string input the user typed.
**
** OUTPUTS:     MOVE : The move corresponding to the user's input.
**
************************************************************************/

//NO ERROR CHECKING YET
//directional functions will not work for P2-no way to find out who's turn it is.
//write 
MOVE ConvertTextInputToMove(input)
     STRING input;
{
  int i = 2;
  char currentRow = input[1], currentCol = input[0];
  unsigned int myMove, previousMove = -1, currentIndex, nextIndex;
  /*  int isFirstRow = ((currentIndex/cols) == 0),
    isLastRow = ((currentIndex/cols) == rows-1);
  */
  myMove = currentIndex = getIndexFromText(currentRow, currentCol);
  myMove = myMove<<(32-MVHASHACC);
  printf("%d", myMove);

  /*if(input[0] == P1KING || input[0] == P1MAN)
    whosTurn = P1;
  else if (input[0] == P2KING || input[0] == P2MAN)
  whosTurn = P2;*/

  while(input[i]!=0){
    currentCol = input[i];
    currentRow = input[i+1];
    nextIndex = getIndexFromText(currentRow, currentCol);
    if(nextIndex == forwardRight(currentTurn, currentIndex)){
      myMove = myMove|(FORWARDRIGHT<<(32-MVHASHACC-i));
      previousMove = FORWARDRIGHT;
    }
    else if(nextIndex == forwardLeft(currentTurn, currentIndex)){
      myMove = myMove|(FORWARDLEFT<<(32-MVHASHACC-i));
      previousMove = FORWARDLEFT;
    }
    else if(nextIndex == backwardLeft(currentTurn, currentIndex)){
      myMove = myMove|(BACKWARDLEFT<<(32-MVHASHACC-i));
      previousMove = BACKWARDLEFT;
    }
    else if(nextIndex == backwardRight(currentTurn, currentIndex)){
      myMove = myMove|(BACKWARDRIGHT<<(32-MVHASHACC-i));
      previousMove = BACKWARDRIGHT;
    }		      
    i=i+2;
    printf("%d", previousMove);
  }
  if(previousMove != -1)
    myMove = myMove|(oppositeMove(previousMove)<<(32-MVHASHACC-(i)));
  return(myMove);
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
  unsigned int currentMove, previousMove, counter=0, done = FALSE;
  char *myMove = (char *)malloc((32-MVHASHACC)/2*sizeof(char));
  printf("(%d ", theMove>>(32-MVHASHACC)), counter = 0;
  theMove = theMove<<MVHASHACC;
  currentMove = previousMove = theMove>>30;
  while(counter<(32-MVHASHACC)/2 && !done){  
    previousMove = currentMove;
    currentMove = (theMove>>30)&0x00000003;
    if(currentMove != oppositeMove(previousMove))
      myMove[counter] = currentMove+'0';
    else
      done = TRUE;
    theMove = theMove<<2;
    counter++;
  }
  myMove[counter-1] = 0;
  if(counter<=(32-MVHASHACC)/2)
    printf("%s", myMove);
  printf(")");
  SafeFree(myMove);

  // TODO
}


STRING kDBName = "Rubik's Checkers" ;

int NumberOfOptions()
{
    // TODO
    return 0;
}

int getOption()
{
    // TODO
    return 0;
}

void setOption(int option)
{
    // TODO
}
