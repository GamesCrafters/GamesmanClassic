/************************************************************************
**
** NAME:        mgobblejr.c
**
** DESCRIPTION: Gobble It Jr.
**
** AUTHOR:      Scott Lindeneau, Damian Hites, John Jordan
**              University of California at Berkeley
**              Copyright (C) 2003. All rights reserved.
**
** DATE:        10/7/03
**
** UPDATE HIST:
** 9/12/03 - Some Work on Text Interface, and file maintnence. -SL
** 10/7/03 - Imported Our functions from other files. -SL
**
** 
**
**************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/

#include <stdio.h>
#include "gsolve.h"
#include <stdlib.h>
#include <unistd.h>

extern STRING gValueString[];

int      gNumberOfPositions  = 56623103;  /** 384 Possible Postions(PP) for small
                                           ** 384 + 384*384 PP for med 
                                           **    (384 for iteslf, 384*384 for when smalls are present)
                                           ** 384 + 2*384*384 +384*384*384 PP for large 
                                           **    (384 for itself, 2*384*384 for each when smalls or meds are present
                                           **        and 384*384*384 for when both smalls and meds are present)
                                           **/

POSITION gInitialPosition    = 0;  //Fairly sure this is right.
POSITION kBadPosition        = -1; /* This can never be the rep. of a position */

STRING   kGameName           = "Gobble It Jr";
BOOLEAN  kPartizan           = FALSE; /* Unknown For sure.
                                       * but fairly sure this
                                       * this is right. More then
                                       * one type of piece.*/
BOOLEAN  kSupportsHeuristic  = FALSE;
BOOLEAN  kSupportsSymmetries = FALSE;
BOOLEAN  kSupportsGraphics   = FALSE;
BOOLEAN  kDebugMenu          = FALSE;
BOOLEAN  kGameSpecificMenu   = TRUE;
BOOLEAN  kTieIsPossible      = FALSE;
BOOLEAN  kLoopy               = TRUE;
BOOLEAN  kDebugDetermineValue = FALSE;

STRING kHelpGraphicInterface =
"Not written yet";

STRING   kHelpTextInterface    =
"On your turn, determine if you wish to move a new piece onto the board\n\
 or move an existing piece on the board. If you wish to move an existing piece,\n\
 use the LEGEND to determine which numbers to choose (between\n1 and 9, with 1\n\
 at the upper left and 9 at the lower right) to correspond\nto the board\n\
 position that you desire to move from and the empty board position\n you\n\
 desire to move to. (i.e. if you wish to move from the 2nd board position\n\
 the the 3rd you would type \"23\" without the qoutes and hit return.)\n\
 If you wish to move a piece onto the board from your stock, check your total\n\
 pieces in stock, select a size with \"L\" \"M\" or \"S\" and the board postion\n\
 you wish to move to. (i.e. if you wish to move a Large piece to the 1st\n\
 postion you would type \"L1\" without the qoutes.) If at any point you have\n\
 made a mistake\nyou can type u and hit return and the system will revert\n\
 back to your most recent\nposition."; 
 //Yea we can change this as we get more functions written but it feels like a good outline.

STRING   kHelpOnYourTurn =
"You move a piece, either from the board or stock, to an applicable board position.";

STRING   kHelpStandardObjective =
"To make three in a row, horizontally, vertically, or diagonally.";

STRING   kHelpReverseObjective =
"To force the other player into three in a row.";

STRING   kHelpTieOccursWhen = /* Should follow 'A Tie occurs when... */
"A tie is not possible in this game.";

//might need to change this.
STRING   kHelpExample =
"         ( 1 2 3 )       L:2          : - - -\n\
LEGEND:  ( 4 5 6 )  Stock M:2 Board:   : - - - \n\
         ( 7 8 9 )        S:2          : - - - \n\n\
Computer's move              :  L5    \n\n\
         ( 1 2 3 )        L:2          : - - -\n\
LEGEND:  ( 4 5 6 )  Stock M:2 Board:   : - D - \n\
         ( 7 8 9 )        S:2          : - - - \n\n\
     Dan's move [(u)ndo/1-9] : { M3 } \n\n\
         ( 1 2 3 )        L:2          : - - M \n\
LEGEND:  ( 4 5 6 )  Stock M:1 Board:   : - D - \n\
         ( 7 8 9 )        S:2          : - - - \n\n\
Computer's move              :  L3    \n\n\
         ( 1 2 3 )        L:2          : - - D \n\
LEGEND:  ( 4 5 6 )  Stock M:1 Board:   : - D - \n\
         ( 7 8 9 )        S:2          : - - - \n\n\
     Dan's move [(u)ndo/1-9] : { L7 } \n\n\
         ( 1 2 3 )        L:1          : - - D \n\
LEGEND:  ( 4 5 6 )  Stock M:1 Board:   : - D - \n\
         ( 7 8 9 )        S:2          : L - - \n\n\
ect..."

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

#define	BOARD_SIZE	3
#define	PIECE_SIZES	2
#define	PIECES_PER_SIZE	2

typedef unsigned long hash_t;
typedef unsigned short layer_t;

layer_t *pos2hash = NULL;
layer_t *hash2pos = NULL;

int tableSize = 0;

#define	COLOR_BITS	(2 * PIECES_PER_SIZE)
#define	TABLE_BITS	(BOARD_SIZE * BOARD_SIZE)

#define	COLOR_MASK	((1 << COLOR_BITS) - 1)
#define	TABLE_MASK	(((1 << TABLE_BITS) - 1) << COLOR_BITS)

#define CONS_MOVE(s,d)	(((d) * TABLE_BITS) + (s)) //Takes the source and dest
                                                   //parts of a move and turns
                                                   //it into a short(or MOVE)
#define	GET_SRC(m)	((m) % TABLE_BITS)             //Gets the source location
                                                   //from MOVE m                                      
#define	GET_DEST(m)	((m) / TABLE_BITS)             //Gets the dest location
                                                   //from MOVE m 

#define	SRC_STASH(s)	(TABLE_BITS + (s))
#define	GET_SRC_SZ(s)	((s) >= TABLE_BITS ? ((s) - TABLE_BITS) : -1)


/*
** Position Structure
**
** board[ 0] thru board[ 8] each represent each space on the 
** board.  For multiple pieces on each space, OR some constants
** (below) together.
**
** Constants are at two bit increments (for sizes -- smallest to largest).
** Red = lower bit, Yellow = upper bit.
*/

struct GPosition {
	short	board[ TABLE_BITS]; //Stores the pieces in the board
	short   stash[ PIECE_SIZES * PIECES_PER_SIZE]; //Stores the pieces in the stash
	short	turn; //Stores whos turn it is.
};


#define	PIECE_NONE		0x0
#define	PIECE_O			0x1	// Red
#define	PIECE_X			0x2	// Yellow

#define	TURN_O			0x0
#define	TURN_X			0x1

//Creates the bitwise representation of the piece p of size s.
#define PIECE_VALUE(p,s)	( (p) << (2 * (s)) ) 

#define	POS_NUMBER(r,c)		( (c) + (BOARD_SIZE * (r)) )
#define	POS_GETROW(p)		(int)( (p) / BOARD_SIZE )
#define POS_GETCOL(p)		(int)( (p) % BOARD_SIZE )


/*************************************************************************
**
** Here we declare the global database variables
**
**************************************************************************/

VALUE     *gDatabase;

/************************************************************************
**
** NAME:        InitializeDatabases
**
** DESCRIPTION: Initialize the gDatabase, a global variable.
** 
************************************************************************/

void InitializeDatabases()
{
  GENERIC_PTR SafeMalloc();
  int i;

  gDatabase = (VALUE *) SafeMalloc (gNumberOfPositions * sizeof(VALUE));

  for(i = 0; i < gNumberOfPositions; i++)
    gDatabase[i] = undecided;
}
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
** CALLS:       SnakeHash ()
**              SnakeUnhash ()
**              whoseTurn ()
**	      MoveToSlots()
*************************************************************************/
POSITION DoMove(thePosition, theMove)
     POSITION thePosition;
     MOVE theMove;
{
   SLOT srcPos, destPos;
   srcPos = GET_SRC(theMove);
   destPos = GET_DEST(theMove);
   struct GPosition newPos;
   newPos = unhash(thePosition);
   int pieceType = (newPos.turn == TURN_O ? PIECE_O : PIECE_X);
   int pieceSize;
   
   if(srcPos >= TABLE_BITS)
   {
      pieceSize = srcPos - TABLE_BITS;
   }else{
      pieceSize = getTopPieceSize( newPos.board[ srcPos])
      newPos.board[ srcPos] |= PIECE_VALUE(PIECE_NONE,srcPos));
   }
   newPos.board[ destPos] |= PIECE_VALUE(pieceType,pieceSize));
   
   newPos.turn =(newPos.turn == TURN_O ? TURN_X : TURN_O);
   
  return hash(newPos);
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
  POSITION hash(); 
  struct Gposition myPosition;
  signed char c;
  int i;
  int j;
  
  printf("\n\n\t----- Get Initial Position -----\n");
  printf("\n\tPlease input the position to begin with.\n");
  printf("\n\tNote that X will always go first\n");
  printf("\tNote that it should be in the following format:\n\n");
  printf("-- X  -x\n");
  printf("O  X  --         <----- EXAMPLE\n");
  printf("-o O  --\n");
  printf("For example, to get the position printed above, type:\n");
  printf("--X--x \nO-X--- \n-oO--- \n");
  
  i = 0;
  getchar();
  while(i < TABLE_BITS && (c = getchar()) != EOF) {
    j = 0;
    do {
      if((c == 'X') || (c == 'x') || (c == '*'))
        Gposition.board[i] = (Gposition.board[i] << 2) + PIECE_X;
      else if((c == 'O') || (c == 'o') || (c == '.'))
        Gposition.board[i] = (Gposition.board[i] << 2) + PIECE_O;
      j++;
    } while((j < PIECE_SIZES) && (c = getchar()) != EOF);
    i++;
  }
  myPosition.turn = TURN_X;
  return(hash(myPosition));
}

/************************************************************************
**
** NAME:        GetComputersMove
**
** DESCRIPTION: Get the next move for the computer from the gDatabase
** 
** INPUTS:      POSITION thePosition : The position in question.
**
** OUTPUTS:     (MOVE) : the next move that the computer will take
**
** CALLS:       int GetRandomNumber()
**
************************************************************************/

MOVE GetComputersMove(thePosition)
     POSITION thePosition;
{
  int i, randomMove, numberMoves = 0;
  MOVELIST *ptr, *head, *GetValueEquivalentMoves();
  MOVE theMove;
  
  if(gPossibleMoves) 
    printf("%s could equivalently choose [ ", gPlayerName[kComputersTurn]);
  head = ptr = GetValueEquivalentMoves(thePosition);
  while(tr != NULL) {
    numberMoves++;
    if(gPossibleMoves) 
      PrintMove(ptr->move);
    ptr = ptr->next;
  }
  if(gPossibleMoves) 
    printf("]\n\n");
  randomMove = GetRandomNumber(numberMoves);
  ptr = head;
  for(i = 0; i < randomMove ; i++)
    ptr = ptr->next;

  theMove = ptr->move;
  FreeMoveList(head);
  return(theMove);
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
** CALLS:       inline int getTopPieceColor(layer_t slot)
**              int getTopPieceSize ( layer_t slot )
**              
**
************************************************************************/

VALUE primitive ( hash_t h )
{
	struct GPosition pos = unhash( h );
	int i, t, x_wins, o_wins;
	int piece;
	
	x_wins = o_wins = 0;
	
	// Diagonals
	piece = getTopPieceColor( pos . board[ POS_NUMBER( 0, 0 )] );
	for (i = 1; i < BOARD_SIZE; i++) {
		if (getTopPieceColor( pos . board[ POS_NUMBER( i, i )] ) != piece) {
			piece = PIECE_NONE;
			break;
		}
	}
	
	if (piece == PIECE_X)
		x_wins++;
	else if (piece == PIECE_O)
		o_wins++;
	
	piece = getTopPieceColor( pos . board[ POS_NUMBER( BOARD_SIZE, 0 )] );
	for (i = 1; i < BOARD_SIZE; i++) {
		if (getTopPieceColor( pos . board[ POS_NUMBER( BOARD_SIZE - i, i )] ) != piece) {
			piece = PIECE_NONE;
			break;
		}
	}
	
	if (piece == PIECE_X)
		x_wins++;
	else if (piece == PIECE_O)
		o_wins++;
	
	
	// Rows
	
	for (i = 0; i < BOARD_SIZE; i++) {
		piece = getTopPieceColor( pos . board[ POS_NUMBER( i, 0 )] );
		for (t = 1; t < BOARD_SIZE; t++) {
			if (getTopPieceColor( pos . board[ POS_NUMBER( i, t )] ) != piece) {
				piece = PIECE_NONE;
				break;
			}
		}
		
		if (piece == PIECE_X)
			x_wins++;
		else if (piece == PIECE_O)
			o_wins++;
	}
	
	
	// Columns
	
	for (t = 0; t < BOARD_SIZE; t++) {
		piece = getTopPieceColor( pos . board[ POS_NUMBER( 0, t )] );
		for (i = 1; i < BOARD_SIZE; i++) {
			if (getTopPieceColor( pos . board[ POS_NUMBER( i, t )] ) != piece) {
				piece = PIECE_NONE;
				break;
			}
		}
		
		if (piece == PIECE_X)
			x_wins++;
		else if (piece == PIECE_O)
			o_wins++;
	}
	
	if (x_wins && o_wins)
		return(tie);
	else if (x_wins && pos.turn == TURN_X)
		return(win);
	else 
        return(lose);
	if (o_wins && pos.turn == TURN_O)
		return(win);
	else
	    return(lose);
	
	return(undecided);
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
** CALLS:       SnakeUnhash()
**              GetValueOfPosition();
**              GetPrediction()
**
************************************************************************/

void PrintPosition(position, playerName, usersTurn)
     POSITION position;
     STRING playerName;
     BOOLEAN usersTurn;
{
 
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
**
************************************************************************/

MOVELIST *GenerateMoves(position)
         POSITION position;
{
  MOVELIST *CreateMovelistNode(), *head = NULL;
  VALUE Primitive();
  struct Gposition myPosition;
  int topPieceFrom;
  int pieceColorFrom;
  int topPieceTo;
  int stockValue;
  
  if(Primitive(position) == undecided) {
    myPosition = unhash(position);
    /* For pieces in the stock */
    for(int i = 0 + myPosition.turn; i < PIECE_SIZES * 2; i += 2) {
      stockValue = i / 2;
      if(myPosition.stash[i] > 0) {
        for(int j = 0; j < TABLE_BITS; j++) {
          topPieceTo = getTopPieceSize(myPosition.board[j]);
          if((topPieceTo > 0) && (topPieceTo < (i / 2)) {
            head = CreateMovelistNode(CONS_MOVE(TABLE_BITS + (i / 2), j), head);
          }
        }
      }
    }
    /* For pieces already on the board */
    for(int i = 0; i < TABLE_BITS; i++) {
      topPieceFrom = getTopPieceSize(myPosition.board[i]);
      pieceColorFrom = getTopPieceColor(myPosition.board[i]);
      if((pieceColorFrom == myPosition.turn) && (topPieceFrom > 0)) {
        for(int j = 0; j < TABLE_BITS; j++) {
          topPieceTo = getTopPieceSize(myPosition.board[j]);
          if((topPieceTo > 0) && (topPieceTo < TopPieceFrom)) {
            head = CreateMoveListNode(CONS_MOVE(i, j), head);
          }
        }
      }
    }
    return head;
  }
  else {
    return null;
    }
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
  int i = 0;
  int j = 0;
  int start;
  int end;
  if((intput[i] != 'X') &&
     (input[i] != 'x') &&
     (input[i] != '*') &&
     (input[i] != 'O') &&
     (input[i] != 'o') &&
     (input[i] != '.')) {
    while((input[i] <= '9') && (input[i] >= '1')) {
      start += ((int) input[i]) + (10 * i);
      i++;
    }
  }
  if(i == 0)
    return false;
  if(start > TABLE_BITS)
    return false;
  if(input[i] != ' ')
    return false;
  i++;
  while(i < input.length) {
    if((input[i] > '9') || (input[i] < '1'))
      return false;
    end += ((int) input[i]) + (10 * j);
    i++;
    j++;
  }
  if(end > TABLE_BITS)
    return false;
  return true;
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
  
}

/************************************************************************
*************************************************************************
** BEGIN   FUZZY STATIC EVALUATION ROUTINES. DON'T WORRY ABOUT UNLESS
**         YOU'RE NOT GOING TO EXHAUSTIVELY SEARCH THIS GAME
*************************************************************************
************************************************************************/

/************************************************************************
**
** NAME:        StaticEvaluator
**
** DESCRIPTION: Return the Static Evaluator value
**
**              If the game is PARTIZAN:
**              the value 0 => player 2's advantage
**              the value 1 => player 1's advantage
**              player 1 MAXIMIZES and player 2 MINIMIZES
**
**              If the game is IMPARTIAL
**              the value 0 => losing position
**              the value 1 => winning position
**
**              Not called if kSupportsHeuristic == FALSE
** 
** INPUTS:      POSITION thePosition : The position in question.
**
** OUTPUTS:     (FUZZY) : the Fuzzy Static Evaluation value
**
************************************************************************/

FUZZY StaticEvaluator(thePosition)
     POSITION thePosition;
{
}

/************************************************************************
**
** NAME:        PositionToMinOrMax
**
** DESCRIPTION: Given any position, this returns whether the player who
**              has the position is a MAXIMIZER or MINIMIZER. If the
**              game is IMPARTIAL (kPartizan == FALSE) then this procedure
**              always returns MINIMIZER. See StaticEvaluator for the 
**              reason. Note that for PARTIZAN games (kPartizan == TRUE):
**              
**              Player 1 MAXIMIZES
**              Player 2 MINIMIZES
**
**              Not called if kSupportsHeuristic == FALSE
** 
** INPUTS:      POSITION thePosition : The position in question.
**
** OUTPUTS:     (MINIMAX) : either minimizing or maximizing
**
************************************************************************/

MINIMAX PositionToMinOrMax(thePosition)
     POSITION thePosition;
{
}

/************************************************************************
*************************************************************************
** END     FUZZY STATIC EVALUATION ROUTINES. DON'T WORRY ABOUT UNLESS
**         YOU'RE NOT GOING TO EXHAUSTIVELY SEARCH THIS GAME
*************************************************************************
************************************************************************/

/************************************************************************
*************************************************************************
** BEGIN   PROBABLY DON'T HAVE TO CHANGE THESE SUBROUTINES UNLESS YOU
**         FUNDAMENTALLY WANT TO CHANGE THE WAY YOUR GAME STORES ITS
**         POSITIONS IN THE TABLE FROM AN ARRAY TO SOMETHING ELSE
**         AND ALSO CHANGE THE DEFINITION OF A POSITION (NOW AN INT)
*************************************************************************
************************************************************************/

/************************************************************************
**
** NAME:        GetRawValueFromDatabase
**
** DESCRIPTION: Get a pointer to the value of the position from gDatabase.
** 
** INPUTS:      POSITION position : The position to return the value of.
**
** OUTPUTS:     (VALUE *) a pointer to the actual value.
**
************************************************************************/

VALUE *GetRawValueFromDatabase(position)
     POSITION position;
{
  return(&gDatabase[position]);
}

/************************************************************************
**
** NAME:        GetNextPosition
**
** DESCRIPTION: Return the next non-undecided position when called 
**              consecutively. When done, return kBadPosition and
**              reset internal counter so that if called again,
**              would start from the beginning.
** 
** OUTPUTS:     (POSITION) : the next non-Undecided position
**
************************************************************************/

POSITION GetNextPosition()
{
  VALUE GetValueOfPosition();
  static POSITION thePosition = 0; /* Cycle through every position */
  POSITION returnPosition;
  
  while(thePosition < gNumberOfPositions &&
	GetValueOfPosition(thePosition) == undecided)
    thePosition++;
  
  if(thePosition == gNumberOfPositions) {
    thePosition = 0;
    return(kBadPosition);
  }
  else {
    returnPosition = thePosition++;
    return(returnPosition);
  }
}

/************************************************************************
*************************************************************************
** END     PROBABLY DON'T HAVE TO CHANGE THESE SUBROUTINES UNLESS YOU
**         FUNDAMENTALLY WANT TO CHANGE THE WAY YOUR GAME STORES ITS
**         POSITIONS IN THE TABLE FROM AN ARRAY TO SOMETHING ELSE
**         AND ALSO CHANGE THE DEFINITION OF A POSITION (NOW AN INT)
*************************************************************************
************************************************************************/

/************************************************************************
*************************************************************************
**         EVERYTHING BELOW THESE LINES IS LOCAL TO THIS FILE
*************************************************************************
************************************************************************/

/*
*** Primateive Helper Functions.
*/
/************************************************************************
**
** NAME:        getTopPieceSize
**
** INPUTS:      layer_t: an individual layer from the hash
**
** DESCRIPTION: not sure.
** 
** OUTPUTS:     (int) : not sure
**
************************************************************************/
int getTopPieceSize ( layer_t slot )
{
	static int memoized[ SLOT_PERMS];
	static int memoizedInit = 0;
	
	int i;
	layer_t n;
	
	if (slot > SLOT_PERMS)
		return -1;
	
	if (memoizedInit == 0) {
		for (i = 0; i < SLOT_PERMS; i++) {
			memoized[ i] = -2;
		}
		
		memoizedInit = 1;
	} else if (memoized[ slot] != -2) {
		return memoized[ slot];
	}
	
	i = PIECE_SIZES - 1;
	n = 3 << (i * 2);
	
	for (; i >= 0; i--, n >>= 2) {
		if (slot & n) {
			memoized[ slot] = i;
			return i;
		}
	}
	
	memoized[ slot] = -1;
	return -1;
}
/************************************************************************
**
** NAME:        getTopPieceColor
**
** INPUTS:      layer_t: an individual layer from the hash
**
** DESCRIPTION: not sure.
** 
** OUTPUTS:     (inline int) : not sure
**
************************************************************************/
inline int getTopPieceColor ( layer_t slot )
{
	int i = getTopPieceSize( slot );
	
	if (i < 0)
		return i;
	else if (slot & PIECE_VALUE( PIECE_O, i ))
		return PIECE_O;
	
	return PIECE_X;
}

/*
** Util functions
*/

/************************************************************************
**
** NAME:        countBits
**
** INPUTS:      hash_t: represents a complete hash.
**
** DESCRIPTION: not sure.
**
************************************************************************/

int countBits ( hash_t n )
{
	int bitCount = 0;
	
	for (; n; n = (n & 0xfffffffe) >> 1)
		bitCount += (n & 1);
	
	return bitCount;
}

/************************************************************************
**
** NAME:        computeTables
**
** DESCRIPTION: not sure.
**
** CALLS:       int countBits ( hash_t n )
**              calloc();
**
************************************************************************/

void computeTables ()
{
	int i, n, bc, cbc, sz;
	layer_t *tmp;
	
	sz = 1 << (TABLE_BITS + COLOR_BITS);
	pos2hash = calloc( sz, sizeof( layer_t ) );
	hash2pos = calloc( sz, sizeof( layer_t ) );
	
	if ( !pos2hash || !hash2pos )
		perror( "calloc" );
	
	for (i = n = 0; i < sz; i++) {
		bc = countBits( i & TABLE_MASK );
		pos2hash[ i] = -1;
		
		// check: legal number of pieces on board
		if (bc > (PIECES_PER_SIZE * 2))
			continue;
		
		// check: no 1's out-of-bounds
		if (i & (COLOR_MASK ^ ((1 << bc) - 1)))
			continue;
		
		cbc = countBits( i & COLOR_MASK );
		
		// check: number of X's/O's are balanced
		if (cbc > PIECES_PER_SIZE || (bc - cbc) > PIECES_PER_SIZE)
			continue;
		
		pos2hash[ i] = n;
		hash2pos[ n] = i;
		n++;
	}
	
	tmp = calloc( n, sizeof( layer_t ) );
	if ( !tmp )
		perror( "calloc" );
	
	memcpy( tmp, hash2pos, n * sizeof( layer_t ) );
	free( hash2pos );
	hash2pos = tmp;
	
	tableSize = n;
}


/*
** Hashing functions
*/

/************************************************************************
**
** NAME:        hash
**
** DESCRIPTION: convert an internal position to that of a GPosition.
** 
** INPUTS:      GPosition: the external position structure to be hashed.
**
** CALLS:       void computeTables ()
**
** OUTPUTS:     (hash_t): The equivelent hash_t of the inputed GPosition.
**
************************************************************************/
hash_t hash ( struct GPosition pos )
{
	unsigned long lpos[ PIECE_SIZES], ret = 0;
	int i, t, pieceNo;
	
	if (! tableSize)
		computeTables();
	
	for (i = 0; i < PIECE_SIZES; i++) {
		lpos[ i] = 0;
		
		for (pieceNo = t = 0; t < TABLE_BITS; t++) {
			if (pos.board[ t] & (0x1 << (2 * i))) {		// O
				lpos[ i] |= 1 << (COLOR_BITS + t);
				pieceNo++;
			} else if (pos.board[ t] & (0x2 << (2 * i))) {
				lpos[ i] |= (1 << (COLOR_BITS + t)) | (COLOR_MASK & (1 << pieceNo));
				pieceNo++;
			}
		}
	}
	
	for (i = PIECE_SIZES - 1, ret = 0; i > -1; i--) {
		ret *= tableSize;
		ret += pos2hash[ lpos[ i]];
	}
	
	return (ret << 1) | (0x1 & pos.turn);
}


/************************************************************************
**
** NAME:        unhash
**
** DESCRIPTION: convert an internal position to that of a GPosition.
** 
** INPUTS:      hash_t h: the internal hash representation.
**
** OUTPUTS:     (GPosition): The equivelent GPosition of the inputed hash.
**
************************************************************************/

struct GPosition unhash ( hash_t h )
{
	struct GPosition ret;
	unsigned long lpos[ PIECE_SIZES];
	int i, t, pieceNo, color;
	
	if (! tableSize)
		computeTables();
	
	ret.turn = h & 1;
	h >>= 1;
	
	// Initialize board
	for (i = 0; i < TABLE_BITS; i++) {
		ret.board[ i] = 0;
	}
	
	// Initialize Stash
	for (i = 0; i < PIECE_SIZES * PIECES_PER_SIZE; i++) {
		ret.stash[ i] = PIECES_PER_SIZE;
	}
	
	// Check each layer
	for (i = 0; i < PIECE_SIZES; i++) {
		lpos[ i] = hash2pos[ h % tableSize];
		h /= tableSize;
		
		// And each space
		for (t = pieceNo = 0; t < TABLE_BITS; t++) {
			if (lpos[ i] & (1 << (COLOR_BITS + t))) {
				if (lpos[ i] & (1 << pieceNo))
					ret.board[ t] |= (2 << (2 * i));
				else
					ret.board[ t] |= (1 << (2 * i));
				
				pieceNo++;
			}
		}
	}
	
	for(i = 0; i < TABLE_BITS;i++) { //Iters over every board position.
	   for(t = 0; t < PIECE_SIZES; t++) { //Iters over all possible piece sizes
	       if(ret.board[ i] & PIECE_VALUE(PIECE_O, t))
                ret.stash[t*2]--;
	       if(ret.board[ i] & PIECE_VALUE(PIECE_X, t))
                ret.stash[t*2+1]--;
       }
    }
 
	
	
	return ret;
}

