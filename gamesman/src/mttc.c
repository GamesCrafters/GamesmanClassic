/************************************************************************
**
** NAME:        mttc.c
**
** DESCRIPTION: Tic Tac Chek
**
** AUTHOR:      Reman Child, Johnathon Tsai
**              University of California at Berkeley
**              Copyright (C) 2004. All rights reserved.
**
** 2004.3.30    First compilation of subroutines into mttc.c; this update
**              basically includes all functions below except for 
**              ttc_hash and ttc_unhash, which still needs to be written
**                                                                -- rc
** 2004.3.31    Added row nums and letters to print position. Shall we
**              consider changing the Queen to Bishop? That's what the
**              original game has. I'll add some variants later.
**                                                                -- jt
**
** 2004.4.6     Added in support for generic_hash; still have to add in
**              modified ruleset as per discussion with Dom.  
**                                                                -- rc
**
** 2004.4.18    Fixed some stuff, found lots more to fix, etc. etc.
**                                                                -- rc
** 2004.4.25    Complete rewrite! This version makes all board options
**              configurable, notably: board width, board height, number
**              of pieces for each side, types of pieces for each side,
**              and number of pieces necessary to get in a row to win!
**              Still need to rewrite GenerateMoves() and
**              GameSpecificMenu().
**                                                                --rc,jt
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

extern STRING gValueString[];

POSITION gNumberOfPositions  = 0; 
POSITION gInitialPosition    = 0; 
POSITION gMinimalPosition    = 0; 
POSITION kBadPosition        = -1; /* Need to ask michel if this is correct */

STRING   kGameName           = "Tic-Tac-Chek"; 
STRING   kDBName             = "ttc"; 
BOOLEAN  kPartizan           = TRUE; 
BOOLEAN  kDebugMenu          = TRUE; 
BOOLEAN  kGameSpecificMenu   = FALSE;
BOOLEAN  kTieIsPossible      = FALSE; 
BOOLEAN  kLoopy              = TRUE; 
BOOLEAN  kDebugDetermineValue = TRUE;

/* 
   Help strings that are pretty self-explanatory 
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

STRING   kHelpTieOccursWhen = /* Should follow 'A Tie occurs when... */
"";

STRING   kHelpExample =
"";

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

/** Structs *************************************************************/

/* Used mainly for setting up/changing the board */
struct pieceType {
  int id;   // Piece ID 
  int freq; // Number of occurrences
};


/** Type Definitions ******************************************************/

typedef char* BOARD;
typedef int PIECE;
typedef int PLAYER;
typedef int CELL;

/** Global Variables *****************************************************/

int numRows;
int numCols;
int winCondition;
/* defines how many of a piece are available; ends in a BLNK */
struct pieceType *initPieces;

enum pieceID { B_KG,B_QN,B_RK,B_BP,B_KN,B_PN,
	       W_KG,W_QN,W_RK,W_BP,W_KN,W_PN,BLNK };

enum players {BLACK, WHITE};

char piece_strings[13] = {'K','Q','R','B','N','P',
			  'k','q','r','b','n','p','-'};

/** #defines **************************************************************/

#define INIT_NUMROWS 4
#define INIT_NUMCOLS 4
#define INIT_NUMPTYPES 3
#define INIT_WINCONDITION 4
#define NUM_PIECES 13
#define CELL_LENGTH 8
#define MOVE_LENGTH 5

#define min(a,b) ((a < b)? a : b)
#define max(a,b) ((a > b)? a : b)

/*************************************************************************/

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
  int *getPieceArray(struct pieceType *, int), getBoardSize(),i;
  BOARD board;
  /* Initialize Global Variables */
  numRows = INIT_NUMROWS;
  numCols = INIT_NUMCOLS;
  winCondition = INIT_WINCONDITION;
  initPieces = (struct pieceType *)SafeMalloc(INIT_NUMPTYPES*sizeof(struct pieceType));
  /* define initial pieces for board - two bishops for each side; */
  initPieces[0].id = B_BP;
  initPieces[0].freq = 2;
  initPieces[1].id = W_BP;
  initPieces[1].freq = 2;
  initPieces[2].id = BLNK; // end condition
  initPieces[2].freq = getBoardSize();
  gNumberOfPositions = generic_hash_init(getBoardSize(),
					 getPieceArray(initPieces,INIT_NUMPTYPES),
					 NULL);
  /* define initial board as being all blank */
  board = (BOARD)SafeMalloc(getBoardSize()*sizeof(char)); //how do i abstract this???
  for (i = 0; i < getBoardSize(); i++)
    board[i] = BLNK;
  gInitialPosition = generic_hash(board,WHITE); // white goes first
  return;
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
  return;
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

void GameSpecificMenu () {
  return;
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
  return;
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

POSITION DoMove (POSITION pos, MOVE move) {
  BOARD getBoard(POSITION), newBoard;
  PLAYER getPlayer(POSITION), newPlayer;
  POSITION makePosition(BOARD, PLAYER);
  newPlayer = (getPlayer(pos) == WHITE)? BLACK : WHITE;
  newBoard = getBoard(pos);
  newBoard[getDest(move)] = getPiece(move);
  if (!offBoard(move)) 
    newBoard[getSource(move)] = BLNK;
  return makePosition(newBoard,newPlayer);
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
  char inString[MOVE_LENGTH+1];
  POSITION pos = gInitialPosition;
  while(TRUE) {
    PrintPosition(pos,"Debugger",FALSE);
    printf("Input a valid move (? for help): ");
    scanf("%s",inString);
    if (!strcmp(inString,"q"))
      break;
    if (inString[0] == '?') {
      printf("\n\n\n\n");
      printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
      printf("|\n");
      printf("|  Move Format: Piece [Source] Destination\n|\n");
      printf("|  *Note* if placing a piece on the board, no source is necessary\n");
      printf("|\n");
      printf("|  Examples:\n");
      printf("|    Qa1b2 , Rd3d1, pb2, Ba3\n");
      printf("|\n");
      printf("|  'q' to exit, '?' for help\n|\n");
      printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
      continue;
    }
    if (ValidTextInput(inString)) {
      if (kDebugMenu)
	printf("DEBUG> Valid Move? TRUE\n");
      pos = DoMove(pos,ConvertTextInputToMove(inString));
    } else {
      printf("\nINVALID MOVE\n");
    }
  }
  return pos;
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
  printf("Implement later");
  return;
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
  BOARD board, getBoard(POSITION);
  int i, numInRow(CELL,BOARD);
  board = getBoard(position);
  for (i = 0; i < getBoardSize(); i++) {
    if (numInRow(i,board) >= winCondition)
      return lose;
  }
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
void PrintPosition (POSITION position, STRING playerName, BOOLEAN usersTurn) {
  BOARD board, getBoard(POSITION);
  struct pieceType *piecesOffBoard(struct pieceType *, BOARD),*pt;
  board = getBoard(position);
  int row,col,i;
  BOOLEAN isWhite(PIECE), isBlack(PIECE);

  // Ascertains the pieces off the board
  for (i = 0; i < sizeOfPieceType(initPieces); i++) {
    pt = piecesOffBoard(initPieces,board);
  }

  printf("\n  tic-tac-chek");
  for (i = 0; i < (numCols*5); i++) 
    printf(" ");
  printf("Win Condition: %d\n ",winCondition);
  for (i = 0; i < (numCols * 5) + 30;i++) {
    printf("-");
  }
  printf("\n\n     ");
  for (col = 0; col < numCols; col++)
    printf("--- ");
  printf("\n");
  for(row = 0; row < numRows; row++) {
    //printf("  %c",'a'+(numRows-1)-row);
    printf("  %d",numRows-row);
    for (col = 0; col < numCols; col++)
      printf(" | %c" ,piece_strings[board[row*numCols+col]]);
    printf(" |      ");
    switch(row) {
    case 0: printf("WHITE's offboard pieces:"); break;
    case 2: printf("BLACK's offboard pieces:");
    }
    printf("\n     ");
    for (col = 0; col < numCols; col++)
      printf("--- ");
    switch(row) {
    case 0: 
      printf("        [ ");
      for (i = 0; i < sizeOfPieceType(pt); i++) {
	if (pt[i].freq == 1 && isWhite(pt[i].id))
	  printf("%c ",piece_strings[pt[i].id]);
	if  (pt[i].freq > 1 && isWhite(pt[i].id))
	  printf("%cx%d ",piece_strings[pt[i].id],pt[i].freq);
      }
      printf("]");
      break;
    case 2: 
      printf("        [ ");
      for (i = 0; i < sizeOfPieceType(pt); i++) {
	if (pt[i].freq == 1 && isBlack(pt[i].id))
	  printf("%c ",piece_strings[pt[i].id]);
	if  (pt[i].freq > 1 && isBlack(pt[i].id))
	  printf("%cx%d ",piece_strings[pt[i].id],pt[i].freq);
      }
      printf("]");
      break;
    }
    printf("\n");
  }
  printf("     ");
  for(i = 0; i < numCols; i++) {
    //printf(" %d  ",i);
    printf(" %c  ",'a'+i);
  }
  printf("\n\n ");
  for (i = 0; i < (numCols * 5) + 30;i++) {
    printf("-");
  }
  printf("\n\n  Game Prediction:  \n\n");

  return;
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
** CALLS:       GENERIC_PTR SafeSafeMalloc(int)
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
  BOOLEAN isPiece(char),isRow(char),isCol(char);
  // Can be either length 3 (placing piece) or 5 (moving piece)
  BOOLEAN valid = TRUE;
  valid &= isPiece(input[0]);
  valid &= isRow(input[1]);
  valid &= isCol(input[2]);
  if (input[3] == '\0') 
    return valid;
  valid &= isRow(input[3]);
  valid &= isCol(input[4]);
  return valid;
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
  /* Move is represented by: lowest CELL_LENGTH bits is the dest, 
     next CELL_LENGTH bits is the source, then the rest corresponds 
     to the piece itself */
  CELL strToCell(STRING);
  int i;
  PIECE piece;
  for (i = 0; i < BLNK; i++)
    if (input[0] == piece_strings[i])
      piece = i;
  if (!(strlen(input) == MOVE_LENGTH)) {
    if (kDebugMenu) {
      printf("DEBUG> Destination str: %s\n",input+1);
      printf("DEBUG> Type: placing, Piece: %c, Dest: %d",piece_strings[piece],strToCell(input+1));
    }
    return makeMove(piece,0xFF,strToCell(input+1));
  }
  return makeMove(piece,strToCell(input+1),strToCell(input+3));
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
  printf("%c",piece_strings[getPiece(move)]);
  if (!offBoard(move)) {
    printf("%c%d",getCol(getSource(move))+'a',
	   getRow(getSource)+1);
  }
  printf("%c%d",getCol(getDest(move))+'a',
	 getRow(getDest(move))+1);
  return;
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

/** HOW DOES THIS WORK IN OURS? INFINITE? **/

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
  return;
}


/************************************************************************
*************************************************************************
**         EVERYTHING BELOW THESE LINES IS LOCAL TO THIS FILE
*************************************************************************
************************************************************************/

/** Utility Functions **************************************************/

/* Gets the board length */
int getBoardSize() {
  return numRows * numCols;
}

/* Gets the row from a cell number */
int getRow(CELL cell) {
  return cell / numRows;
}

/* Gets the col from a cell number */
int getCol(CELL cell) {
  return cell % numCols;
}

/** Primitive Helper Functions *****************************************/
BOOLEAN isBlack(PIECE piece) {
  return piece < B_PN + 1;
}

BOOLEAN isWhite(PIECE piece) {
  return piece > B_PN && piece < BLNK;
}

/* Identifies whether the pieces on the two cells are of the same color */
BOOLEAN sameColor(CELL first, CELL second, BOARD board) {
  return (isWhite(board[first]) && isWhite(board[second])) ||
    (isBlack(board[first]) && isBlack(board[second]));
}

/** return the number of pieces consecutively adjacent including current piece
 *  vertically, horizonally, or diagonally
 */
int numInRow(CELL start, BOARD board) {
  int numUp=0, numDown=0, numLeft=0, numRight=0, numUL=0, numDR=0, numUR=0, numDL=0;
  int i=0, j=0;
  int row = (start / numCols) + 1; // rows start from 1
  int col = (start % numCols) + 1; // cols start from 1
  BOOLEAN sameColor(CELL,CELL,BOARD);
  if (board[start] == BLNK)
    return 0;
  else {
    for(i = 1; col + i <= numCols; i++) {
      if (sameColor(start, start+i, board))
	numRight++;
      else
	break;
    }
    for(i = 1; col - i > 0; i++) {
      if (sameColor(start, start-i, board))
	numLeft++;
      else
	break;
    }

    for(i = 1; row + i <= numRows; i++) {
      if (sameColor(start, start + numCols*i, board))
	numDown++;
      else
	break;
    }
    for(i = 1; row - i > 0; i++) {
      if (sameColor(start, start - numCols*i, board))
	numUp++;
      else
	break;
    }

    for (i=1; row+i <= numRows && col+i <= numCols; i++) {
      if (sameColor(start, start+numCols*i+1, board))
	numDR++;
      else
	break;
    }
    for (i=1; row-i > 0 && col-i > 0; i++) {
      if (sameColor(start, start-numCols*i-1, board))
	numUL++;
      else
	break;
    }

    for (i=1; row+i <= numRows && col-i > 0; i++) {
      if (sameColor(start, start+numCols*i-1, board))
	numDL++;
      else
	break;
    }
    for (i=1; row-i > 0 && col+i <= numCols; i++) {
      if (sameColor(start, start-numCols*i+i, board))
	numUR++;
      else
	break;
    }

    i = max (numRight+numLeft, numDown+numUp);
    j = max (numDR+numUL, numDL+numUR);

    //printf("\nnumUP %d numDown %d\nnumLeft %d numRight%d\nnumUL %d numDR %d\nnumUR %d numDL %d\n\n", numUp, numDown, numLeft, numRight, numUL, numDR, numUR, numDL);

    return 1+max(i, j);
  }
  return 0;
}

/** PRINT POSITION helper functions ************************************/

struct pieceType *piecesOffBoard(struct pieceType *pieces, BOARD board) {
  int getBoardSize(), sizeOfPieceType(struct pieceType *),i;
  struct pieceType *newPT;
  newPT = (struct pieceType *)SafeMalloc(sizeOfPieceType(pieces) * sizeof(struct pieceType));
  for (i = 0; i < sizeOfPieceType(pieces)+1; i++) {
    newPT[i].id = pieces[i].id;
    newPT[i].freq = pieces[i].freq - numOnBoard(pieces[i].id,board);
  }
  return newPT;
}

/* Counts the number of occurrences of specified piece on the board */
int numOnBoard(PIECE piece, BOARD board) {
  int i, count=0;
  for (i = 0; i < getBoardSize(); i++)
    if (board[i] == piece)
      count++;
  return count;
}

/* Returns the size of a struct pieceType array (assumes ends with a BLNK)*/
int sizeOfPieceType(struct pieceType *pt) {
  int i = 0;
  while(pt[i].id != BLNK) {
    i++;
  }
  return i;
}

/** Valid Text Input Helpers *******************************************/
BOOLEAN isPiece(char c) {
  int i;
  BOOLEAN valid = FALSE;
  for (i = 0; i < sizeOfPieceType(initPieces); i++)
    if (c == piece_strings[initPieces[i].id])
      valid = TRUE;
  return valid;
}

BOOLEAN isRow(char c) {
  return (c >= 'a' && c < 'a' + numRows);
}

BOOLEAN isCol(char c) {
  return (c >= '0' && c <= '9'); //places upper bound on numRows
}

/** ConvertTextInputToMove Helper Functions ****************************/

/* Converts a string of the form 'a5', 'b6',etc to cell on board */
CELL strToCell(STRING str) {
  return (str[0] - 'a') + numCols*(numRows-(str[1]-'0'));
}

/** Hash Interaction Functions *****************************************/

/* Gets the Board from the hash position */
BOARD getBoard(POSITION pos) {
  int getBoardSize();
  char * generic_unhash(int,char *); // ?????? 
  BOARD newBoard;
  newBoard = (BOARD) SafeMalloc(getBoardSize()*sizeof(char));
  newBoard = generic_unhash(pos,newBoard);
  return newBoard;
}

/* Gets the Player from the hash position */

PLAYER getPlayer(POSITION pos) { // Note: WHITE is player 1
  return (whoseMove(pos) == 1)? WHITE : BLACK;
}

POSITION makePosition(BOARD board, PLAYER player) {
  return generic_hash(board,(player == WHITE)? 1 : 2);
}

/* Converts the pieceType array into one more palatable to the hash function 
 * Inputs are the pieceType array itself and the size of the pieceType Array 
 * REQUIRES: BLNK is the last piece in the pieceType array
 */
int *getPieceArray(struct pieceType *types, int size) {
  int *pieceArray, i, numPieces = 0, minblanks, getBoardSize();
  pieceArray = (int *) SafeMalloc ((3*size+1) * sizeof(int));
  for (i = 0; i < size; i++) {
    pieceArray[3*i] = types[i].id;
    if (types[i].id == BLNK) {
      minblanks = getBoardSize() - numPieces;
      pieceArray[3*i+1] = (minblanks > 0)? minblanks : 0;
    } else {
      pieceArray[3*i+1] = 0;
    }
    pieceArray[3*i+2] = types[i].freq;
    numPieces += types[i].freq;
  }
  pieceArray[3*size] = -1;
  return pieceArray;
}

/** Move representation helper functions *********************************/

// A MOVE is an int whose lowest 8 bits represent the dest number,
// whose next 8 bits represent the source number, and whose next 4
// bits represent the piece being moved
// NOTE: 0xff for the source represents OFF the board
MOVE makeMove(PIECE piece, CELL source, CELL dest) {
  int move = dest;
  move += source << CELL_LENGTH;
  move += piece << 2*CELL_LENGTH;
  return move;
}

PIECE getPiece(MOVE move) {
  return move >> 2*CELL_LENGTH;
}

CELL getSource(MOVE move) {
  return (move >> CELL_LENGTH) & 0xFF;
}

CELL getDest(MOVE move) {
  return move & 0xFF;
}

/* Returns TRUE if we're placing a piece from off the board, else FALSE */
BOOLEAN offBoard(MOVE move) {
  return (getSource(move) == 0xFF);
}

/*************************************************************************/

/** TESTING FUNCTIONS ***************************************************/

/* Generates a random board */

#define NULLPIECE 15 // we know that 15 doesn't correspond to a real piece
#define MAX 50
BOARD generateBoard(struct pieceType *pieces) {
  int i, j, k,pieceCount = 0, getBoardSize();
  char *pieceBank;
  BOARD newBoard;
  srand(time(NULL));
  for (i = 0; ; i++) {
    if (pieces[i].id == BLNK) {
      break;
    } else {
      printf("id: %d, %d ",pieces[i].id,i);
    }
  }
  pieceBank = (char *) SafeMalloc(MAX*sizeof(char));
  for (i = 0,k=0;;i++) {
    if (pieces[i].id == BLNK) // loop terminates on BLNK
      break;
    pieceCount += pieces[i].freq;
    for (j = 0; j < pieces[i].freq; j++) {
      pieceBank[k++] =  pieces[i].id;
    }
  }
  printf("Number of pieces to possibly place: %d\n",pieceCount);
  newBoard = (BOARD) SafeMalloc (getBoardSize() * sizeof(char));
  for(i = 0; i < getBoardSize(); i++) {
    j = rand() % (2*pieceCount); // the probability of getting a blank is < 50%
    if (j < pieceCount && pieceBank[j] != NULLPIECE) {
      newBoard[i] = pieceBank[j];
      pieceBank[j] = NULLPIECE;
    } else {
      newBoard[i] = BLNK;
    }
  }
  return newBoard;
}
