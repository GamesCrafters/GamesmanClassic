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
** 10/14/03 - Finished other functions. Don't know if it works. - SL
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
#include "gamesman.h"
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

extern STRING gValueString[];

int      gNumberOfPositions  = 0;

POSITION gInitialPosition    = 0;
POSITION gMinimalPosition    = 0;
POSITION kBadPosition        = -1; /* This can never be the rep. of a position */

STRING   kGameName           = "Gobblet Jr";
STRING   kDBName             = "gobblet";
BOOLEAN  kPartizan           = FALSE; /* Unknown For sure.
                                       * but fairly sure this
                                       * this is right. More than
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
"         ( 1 2 3 )            : -- -- --\n\
LEGEND:  ( 4 5 6 )   Board:   : -- -- -- \n\
         ( 7 8 9 )            : -- -- -- \n\
         Stock: X:2 x:2 \n\n\
Computer's move              :  L5    \n\n\
         ( 1 2 3 )             : -- -- --\n\
LEGEND:  ( 4 5 6 )    Board:   : -- O  -- \n\
         ( 7 8 9 )             : -- -- -- \n\
         Stock: X:2 x:2 \n\n\
     Dan's move [(u)ndo/1-9] : { M3 } \n\n\
         ( 1 2 3 )            : -- -- -x \n\
LEGEND:  ( 4 5 6 )   Board:   : --  O -- \n\
         ( 7 8 9 )            : -- -- -- \n\
         Stock: X:2 x:1 \n\n\
Computer's move              :  L3    \n\n\
         ( 1 2 3 )            : -- -- Ox \n\
LEGEND:  ( 4 5 6 )   Board:   : -- O  -- \n\
         ( 7 8 9 )            : -- -- -- \n\
         Stock: X:2 x:1 \n\n\
     Dan's move [(u)ndo/1-9] : { L7 } \n\n\
         ( 1 2 3 )            : -- -- O  \n\
LEGEND:  ( 4 5 6 )   Board:   : -- O  -- \n\
         ( 7 8 9 )            : X  -- -- \n\
         Stock: X:1 x:1 \n\n\
ect...";

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

typedef unsigned short SLOT;
typedef unsigned long layer_t;

// These should actually be smaller. -JJ

#define MAX_BOARD_SIZE		5
#define MAX_PIECE_SIZES		4
#define	MAX_PIECES_PER_SIZE	3

layer_t *pos2hash = NULL;
layer_t *hash2pos = NULL;

int tableSize = 0;
int BOARD_SIZE = 3,
    PIECE_SIZES = 2,
    PIECES_PER_SIZE = 2,
    COLOR_BITS = -1,
    TABLE_BITS = -1,
    COLOR_MASK = -1,
    TABLE_MASK = -1,
    TABLE_SLOTS = -1,
    SLOT_PERMS = -1;

#define CONS_MOVE(s,d)	(((s) * TABLE_SLOTS) + (d)) //Takes the source and dest
                                                   //parts of a move and turns
                                                   //it into a short(or MOVE)
#define	GET_SRC(m)	((m) / TABLE_SLOTS)             //Gets the source location
                                                   //from MOVE m                                      
#define	GET_DEST(m)	((m) % TABLE_SLOTS)             //Gets the dest location
                                                   //from MOVE m 

#define	SRC_STASH(s)	(TABLE_SLOTS + (s))
#define	GET_SRC_SZ(s)	((s) >= TABLE_SLOTS ? ((s) - TABLE_SLOTS) : -1)


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
	SLOT	board[ MAX_BOARD_SIZE * MAX_BOARD_SIZE]; //Stores the pieces in the board
	short   stash[ MAX_PIECE_SIZES * 2]; //Stores the pieces in the stash
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


BOOLEAN gThreeInARowWins = TRUE;


/*
** Function Prototypes:
*/

struct GPosition 	unhash (POSITION);
POSITION		hash (struct GPosition);
int			getTopPieceSize (SLOT);
int			getTopPieceColor (SLOT);
int			charToInt (char);
void			computeTables (void);
void			PrintPosition (POSITION, STRING, BOOLEAN);

// External
extern GENERIC_PTR	SafeMalloc ();
extern void		SafeFree ();

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

void InitializeGame()
{
  int i;
  
  if (BOARD_SIZE      >= MAX_BOARD_SIZE
   || PIECE_SIZES     >= MAX_PIECE_SIZES
   || PIECES_PER_SIZE >= MAX_PIECES_PER_SIZE) {
  	printf( "Illegal board dynamic.\n\n" );
  	exit( 1 );
  }
  
  // Calculate values associated with BOARD_SIZE and friends
  COLOR_BITS = 2 * PIECES_PER_SIZE;
  TABLE_BITS = TABLE_SLOTS = BOARD_SIZE * BOARD_SIZE;
  COLOR_MASK = (1 << COLOR_BITS) - 1;
  TABLE_MASK = ((1 << TABLE_BITS) - 1) << COLOR_BITS;
  SLOT_PERMS = 1 << (PIECE_SIZES * 2);
  
  if (pos2hash) {
    SafeFree( pos2hash );
    pos2hash = NULL;
  }
  
  if (hash2pos) {
    SafeFree( hash2pos );
    hash2pos = NULL;
  }
    
  computeTables();
  
  gNumberOfPositions = 1;
  for (i = 0; i < PIECE_SIZES; i++) {
    if ((UINT_MAX / tableSize) < gNumberOfPositions) {
      printf( "Game too large.\n" );
      gDatabase = NULL;
      return;
    }
    gNumberOfPositions *= tableSize;
  }
  
  if ((UINT_MAX / 2) < gNumberOfPositions) {
    printf( "Game too large.\n" );
    gDatabase = NULL;
    return;
  }
  gNumberOfPositions <<= 1;
  
  if (gDatabase) {
    SafeFree( gDatabase );
    gDatabase = NULL;
  }
  
  gDatabase = (VALUE *) SafeMalloc (gNumberOfPositions * sizeof(VALUE));

  for(i = 0; i < gNumberOfPositions; i++)
    gDatabase[i] = undecided;
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

/*
** Dummy functions -- menu helpers
*/

void SetBoardSize ()
{
}

void SetPieceSizes ()
{
}

void SetNumPieces ()
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
    printf("\n\t----- Game-specific options for %s -----\n\n", kGameName);
    
    printf("\tCurrent Initial Position:\n");
    PrintPosition(gInitialPosition, gPlayerName[kPlayerOneTurn], kHumansTurn);
    
    printf("\tI)\tChoose the (I)nitial position\n");
    printf("\tT)\t(T)hree in a row %s\n", 
	   gThreeInARowWins ? "GOOD (WINNING)" : "BAD (LOSING)");
	   
    printf("\tI)\tChoose the board (S)ize (2 through 9)\n");
    printf("\tI)\tChoose the number of (P)iece sizes (*Note: 3 or more\
        may be unsolvable on a 32 bit machine at 3x3 board.)\n");
    printf("\tI)\tChoose the (N)umber of each piece size\n");
    
    printf("\n\n\tb)\t(B)ack = Return to previous activity.\n");
    printf("\n\nSelect an option: ");
    
    switch(GetMyChar()) {
    case 'Q': case 'q':
      ExitStageRight();
    case 'H': case 'h':
      HelpMenus();
      break;
    case 'S': case 's':
      SetBoardSize();
      break;
    case 'P': case 'p':
      SetPieceSizes();
      break;
    case 'N': case 'n':
      SetNumPieces();
      break;
    case 'I': case 'i':
      gInitialPosition = GetInitialPosition();
      break;
    case 'T': case 't':
      gThreeInARowWins = !gThreeInARowWins;
      break;
    case 'b': case 'B':
      return;
    default:
      printf("\nSorry, I don't know that option. Try another.\n");
      HitAnyKeyToContinue();
      break;
    }
  } while(TRUE);

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
   
   if(srcPos >= TABLE_SLOTS)
   {
      pieceSize = srcPos - TABLE_SLOTS;
   }else{
      pieceSize = getTopPieceSize( newPos.board[ srcPos]);
      newPos.board[ srcPos] ^= PIECE_VALUE(getTopPieceColor( newPos.board[ srcPos] ),pieceSize);
   }
   newPos.board[ destPos] |= PIECE_VALUE(pieceType,pieceSize);
   
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
  struct GPosition myPosition;
  struct GPosition comparePosition;
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
  while(i < TABLE_SLOTS && (c = getchar()) != EOF) {
    j = 0;
    do {
      if((c == 'X') || (c == 'x') || (c == '*'))
        myPosition.board[i] = (myPosition.board[i] << 2) + PIECE_X;
      else if((c == 'O') || (c == 'o') || (c == '.'))
        myPosition.board[i] = (myPosition.board[i] << 2) + PIECE_O;
      j++;
    } while((j < PIECE_SIZES) && (c = getchar()) != EOF);
    i++;
  }
  myPosition.turn = TURN_X;
  comparePosition = unhash(hash(myPosition));
  if((myPosition.board != comparePosition.board) ||
     (myPosition.stash != comparePosition.stash) ||
     (myPosition.turn != comparePosition.turn))
  {
    printf("\n Illegal Board Position Please Re-Enter\n");
    return GetInitialPosition();
  }
  else{
    return(hash(myPosition));
  }
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
  SLOT srcPos, destPos;
  srcPos = GET_SRC(computersMove);
  destPos = GET_DEST(computersMove);
  printf("%8s's move              : %d %d\n", computersName, 
	 srcPos+1,destPos+1);// add if it is from stock.
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

VALUE Primitive ( POSITION h ) //Need to add the 3 in a row is a loss.
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
	  return (win);
	else if (x_wins && pos.turn == TURN_X)
	  return(win);
	else if(o_wins && pos.turn == TURN_X)
	  return(lose);
	else if (o_wins && pos.turn == TURN_O)
	  return(win);
	else if(x_wins && pos.turn == TURN_O)
	  return (lose);
	else 
	  return (undecided);
}

/************************************************************************
**
** NAME:        PrintSpace
**
** DESCRIPTION: Creates the string representation of the board space
** 
** INPUTS:      SLOT the space to create the representation for.
**
************************************************************************/

void gobbletPrintSpace(SLOT mySpace)
{
  int i;
  for(i = 0; i < PIECE_SIZES; i++)
    {
      if(PIECE_VALUE(PIECE_X,i) & mySpace != 0)
	{
	  printf("X");
	}
      else if(PIECE_VALUE(PIECE_O,i) & mySpace != 0)
	{
	  printf("O");
	}
      else
	{
	  printf( "-");
	}
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
  STRING GetPrediction();
  STRING PrintSpace();
  VALUE GetValueOfPosition();
  struct GPosition myPos = unhash(position);
  int i = 1;
  int rows, cols, cols2, pSizes;
  printf("\n");
  for(rows = 0; rows < BOARD_SIZE;rows++)
  {
    printf((BOARD_SIZE/2 == rows ? "LEGEND:  ( " : "         ( "));
    for(cols = 0; cols < BOARD_SIZE;cols++)
    {
        printf("%d ",i++);
    }
    printf((BOARD_SIZE/2 == rows ? ")   Board:   : " : ")            : "));
    for(cols2 = 0; cols2 < BOARD_SIZE;cols2++)
    {
        printf(" ");
	gobbletPrintSpace(myPos.board[rows+cols2]);
    }
    printf("\n");
  }
  printf("       X Stock: ");
  for(pSizes=0; pSizes < PIECE_SIZES*2; pSizes+=2)
  {
    printf("%d(%d): %d ",pSizes+1,pSizes+TABLE_SLOTS,myPos.stash[ pSizes]);
  }
  printf("\n       Y Stock: ");
 for(pSizes=1; pSizes < PIECE_SIZES*2; pSizes+=2)
  {
    printf("%d: %d ",pSizes,myPos.stash[ pSizes]);
  }
  printf("\nGame Prediction: %s \n\n",GetPrediction(position,playerName,usersTurn));
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
  struct GPosition myPosition;
  int topPieceFrom;
  int pieceColorFrom;
  int topPieceTo;
  int stockValue;
  int i, j;
  
  if(Primitive(position) == undecided) {
    myPosition = unhash(position);
    /* For pieces in the stock */
    for(i = 0 + myPosition.turn; i < PIECE_SIZES * 2; i += 2) {
      stockValue = i / 2;
      if(myPosition.stash[i] > 0) {
        for(j = 0; j < TABLE_SLOTS; j++) {
          topPieceTo = getTopPieceSize(myPosition.board[j]);
          if(((topPieceTo > 0) && (topPieceTo < (i / 2))) || 
	     (topPieceTo == -1)) {
            head = CreateMovelistNode(CONS_MOVE(TABLE_SLOTS + (i / 2), j), head);
          }
        }
      }
    }
    /* For pieces already on the board */
    for(i = 0; i < TABLE_SLOTS; i++) {
      topPieceFrom = getTopPieceSize(myPosition.board[i]);
      pieceColorFrom = getTopPieceColor(myPosition.board[i]);
      if((pieceColorFrom == myPosition.turn) && (topPieceFrom > 0)) {
        for(j = 0; j < TABLE_SLOTS; j++) {
          topPieceTo = getTopPieceSize(myPosition.board[j]);
          if(((topPieceTo > 0) && (topPieceTo < topPieceFrom)) ||
	     (topPieceTo == -1)) {
            head = CreateMovelistNode(CONS_MOVE(i, j), head);
          }
        }
      }
    }
    return head;
  }
  else {
    return NULL;
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
  BOOLEAN ValidMove();
  USERINPUT ret, HandleDefaultTextInput();
  
  do {
    printf("%8s's move [(u)ndo/(1-%d 1-%d)] :  ", playerName, TABLE_SLOTS + PIECE_SIZES, TABLE_SLOTS);
    
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
  int i;
  i = atoi(input);
  if((i == 0) || (i < 1 || i > TABLE_SLOTS + PIECE_SIZES) )
    return FALSE;

  if(index(input, ' ') == NULL)
    return FALSE;

  i = atoi(index(input, ' '));
  if(i == 0 || (i < 1 || i > TABLE_SLOTS) )
    return FALSE;

  return TRUE;
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
  SLOT srcPos, destPos;

  srcPos = atoi(input) - 1;

  destPos = atoi(index(input, ' ')) - 1;
  
 
  return ((MOVE) CONS_MOVE(srcPos,destPos));
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
  SLOT srcPos, destPos;
  srcPos = GET_SRC(theMove);
  destPos = GET_DEST(theMove);
  // if(srcPos <= TABLE_SLOTS)
    printf("%d %d\n", srcPos+1,destPos+1);
    // else
    // printf("%d %d\n",  write this later..
}


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

int getTopPieceSize ( SLOT slot )
{
	static int* memoized = NULL;
	
	int i;
	SLOT n;
	
	if (slot > SLOT_PERMS)
		return -1;
	
	if (!memoized) {
		memoized = (int*) SafeMalloc( SLOT_PERMS * sizeof( int ) );
		
		for (i = 0; i < SLOT_PERMS; i++) {
			memoized[ i] = -2;
		}
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
int getTopPieceColor ( SLOT slot )
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
** NAME:        charToInt
**
** INPUTS:      char: character representation of piece
**
** DESCRIPTION: returns the corresponding stock location.
**
************************************************************************/
int charToInt(char pieceRep)
{
  switch (pieceRep) {
    case '*':	return PIECE_SIZES*2 - 6;
    case '.':	return PIECE_SIZES*2 - 5;
    case 'x':	return PIECE_SIZES*2 - 4;
    case 'o':	return PIECE_SIZES*2 - 3;
    case 'X':	return PIECE_SIZES*2 - 2;
    case 'O':	return PIECE_SIZES*2 - 1;
  }
  
  return -1;
}



/************************************************************************
**
** NAME:        countBits
**
** INPUTS:      POSITION: represents a complete hash.
**
** DESCRIPTION: not sure.
**
************************************************************************/

// This will have to be changed if POSITION gets bigger than 32 bits.

int countBits ( POSITION n )
{
	unsigned long w = (unsigned long) n;
	
	// Non-obvious method, but very fast.
	
	w = (0x55555555 & w) + (0x55555555 & (w>> 1));
	w = (0x33333333 & w) + (0x33333333 & (w>> 2));
	w = (0x0f0f0f0f & w) + (0x0f0f0f0f & (w>> 4));
	w = (0x00ff00ff & w) + (0x00ff00ff & (w>> 8));
	w = (0x0000ffff & w) + (0x0000ffff & (w>>16));
	
	return w;
}

/************************************************************************
**
** NAME:        computeTables
**
** DESCRIPTION: not sure.
**
** CALLS:       int countBits ( POSITION n )
**              calloc();
**
************************************************************************/

void computeTables ()
{
	int i, n, bc, cbc, sz;
	layer_t* tmp;
	
	sz = 1 << (TABLE_BITS + COLOR_BITS);
	pos2hash = (layer_t*) SafeMalloc( sz * sizeof( layer_t ) );
	hash2pos = (layer_t*) SafeMalloc( sz * sizeof( layer_t ) );
	
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
	
	tmp = (layer_t*) SafeMalloc( n * sizeof( layer_t ) );
	
	memcpy( tmp, hash2pos, n * sizeof( layer_t ) );
	SafeFree( hash2pos );
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
** OUTPUTS:     (POSITION): The equivelent POSITION of the inputed GPosition.
**
************************************************************************/
POSITION hash ( struct GPosition pos )
{
	unsigned long lpos[ PIECE_SIZES], ret = 0;
	int i, t, pieceNo;
	
	if (! tableSize)
		computeTables();
	
	for (i = 0; i < PIECE_SIZES; i++) {
		lpos[ i] = 0;
		
		for (pieceNo = t = 0; t < TABLE_SLOTS; t++) {
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
** INPUTS:      POSITION h: the internal hash representation.
**
** OUTPUTS:     (GPosition): The equivelent GPosition of the inputed hash.
**
************************************************************************/

struct GPosition unhash ( POSITION h )
{
	struct GPosition ret;
	unsigned long lpos[ PIECE_SIZES];
	int i, t, pieceNo;
	
	if (! tableSize)
		computeTables();
	
	ret.turn = h & 1;
	h >>= 1;
	
	// Initialize board
	for (i = 0; i < TABLE_SLOTS; i++) {
		ret.board[ i] = 0;
	}
	
	// Initialize Stash
	for (i = 0; i < (PIECE_SIZES * PIECES_PER_SIZE * 2); i++) {
		ret.stash[ i] = PIECES_PER_SIZE;
	}
	
	// Check each layer
	for (i = 0; i < PIECE_SIZES; i++) {
		lpos[ i] = hash2pos[ h % tableSize];
		h /= tableSize;
		
		// And each space
		for (t = pieceNo = 0; t < TABLE_SLOTS; t++) {
			if (lpos[ i] & (1 << (COLOR_BITS + t))) {
				if (lpos[ i] & (1 << pieceNo))
					ret.board[ t] |= (2 << (2 * i));
				else
					ret.board[ t] |= (1 << (2 * i));
				
				pieceNo++;
			}
		}
	}
	
	for(i = 0; i < TABLE_SLOTS; i++) {
		for(t = 0; t < PIECE_SIZES; t++) {
			if(ret.board[ i] & PIECE_VALUE(PIECE_O, t))
				--ret.stash[ t * 2];
			
			if(ret.board[ i] & PIECE_VALUE(PIECE_X, t))
				--ret.stash[ t * 2 + 1];
		}
	}
 
	
	
	return ret;
}


int NumberOfOptions()
{
  return 0;
}

int getOption()
{
  return 0;
}

void setOption(int option)
{
}


