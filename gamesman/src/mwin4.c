/************************************************************************
**
** NAME:        mwin4.c
**
** DESCRIPTION: Connect-4
**
** AUTHOR:      Michael Thon, University of Berkeley
**              Copyright (C) Michael Thon, 2002. All rights reserved.
**
** DATE:        07/12/02
**
** UPDATE HIST:
**
**  7-12-02 1.0 : Getting started (not using RCS though)
**                will first attempt only 5x4 size boards
**  9-12-02 1.1 : Board sizes (smaller than 5x5) can be set in mwin4.h
**                Text-based module works for these.
**                ToDo: -graphics
**                      -help-Information
**                      -documentation
** 10-12-02 1.2 : - Board sizes now set in this file (no more mwin4.h)
**                  MUST ALSO SET IN mwin4.tcl !! (gSlotsX and Y)
**                  Feasale sizes are 5x4 or 4x5 (both interesting)
**                - Program will probably SegFault if sizeof(int)<4
**                  Would be nice to have POSITION be long int or even
**                  unsigned long int
**                - Graphical Module mwin4.tcl done
**
**************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/

#include <stdio.h>
#include "gamesman.h"

#define  WIN4_WIDTH          4
#define  WIN4_HEIGHT         4
// Don't forget to set gSlotsX (width) and gSlotsY (height) in win4.tcl !!

extern STRING gValueString[];
POSITION MyInitialPosition();
unsigned long int MyNumberOfPos();
unsigned long int gNumberOfPositions = 0; // Initialized to MyNumberOfPos()
POSITION gInitialPosition    = 0;     // Initialized to MyInitialPosition()
POSITION kBadPosition        = 0;     // This can never be the rep.
                                      // of a position

POSITION gMinimalPosition    =  0;

STRING   kGameName           = "Connect-4";
BOOLEAN  kPartizan           = TRUE;
BOOLEAN  kSupportsHeuristic  = FALSE;
BOOLEAN  kSupportsSymmetries = FALSE;
BOOLEAN  kSupportsGraphics   = TRUE;
BOOLEAN  kDebugMenu          = FALSE; //What for??
BOOLEAN  kGameSpecificMenu   = FALSE;
BOOLEAN  kTieIsPossible      = TRUE;
BOOLEAN  kLoopy               = FALSE;
BOOLEAN  kDebugDetermineValue = FALSE;

STRING   kHelpGraphicInterface =
"Click on a highlighted sqare to make your move there.";

STRING   kHelpTextInterface    =
"On your turn, enter the number (usually 1 through 5) corresponding to a\n\
slot that is not yet full. This will ''drop'' a piece of yours into the\n\
slot. If at any point you have made a mistake, you can type u and hit\n\
return and the system will revert back to your most recent position.";

STRING   kHelpOnYourTurn =
"You ''drop'' one of your pieces into a slot that is not yet full.";

STRING   kHelpStandardObjective =
"To get four of your markers (either X or O) in a row, either\n\
horizontally, vertically, or diagonally. 4-in-a-row WINS.";

STRING   kHelpReverseObjective =
"To force your opponent into getting four of his markers (either X or\n\
O) in a row, either horizontally, vertically, or diagonally. 4-in-a-row\n\
LOSES.";

STRING   kHelpTieOccursWhen = /* Should follow 'A Tie occurs when... */
"the board fills up without either player getting four-in-a-row.";

STRING   kHelpExample = 
"Just go ahead and try it out for yourself. No need to be scared...";

/*************************************************************************
**
** Everything above here must be in every game file
**
**************************************************************************/

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

POSITION GetInitialPosition() {return MyInitialPosition();}


/*************************************************************************
**
** Every variable declared here is only used in this file (game-specific)
**
**************************************************************************/

typedef enum possibleBoardPieces {
	x, o, Blank
} XOBlank;

char *gBlankOXString[] = { "X", "O", "-" };

/*************************************************************************
**
** Here we declare the global database variables
**
**************************************************************************/

extern VALUE     *gDatabase;

InitializeGame()
{
  GENERIC_PTR SafeMalloc();
  unsigned int i, j, temp;
  gNumberOfPositions = MyNumberOfPos();
  gInitialPosition    = MyInitialPosition();

//  gDatabase = (VALUE *) SafeMalloc (gNumberOfPositions * sizeof(VALUE));

  gMinimalPosition = gInitialPosition ;

//  printf("numofpos = %u %x, ptr=%u\n", 
//	 gNumberOfPositions, gNumberOfPositions, gDatabase);
//  for(i = 0; i < gNumberOfPositions; i++)
  //  gDatabase[i] = undecided;
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

GameSpecificMenu() { }

/************************************************************************
**
** NAME:        SetTclCGameSpecificOptions
**
** DESCRIPTION: Set the C game-specific options (called from Tcl)
**              Ignore if you don't care about Tcl for now.
** 
************************************************************************/

SetTclCGameSpecificOptions(theOptions)
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
** CALLS:       XOBlank WhosTurn(POSITION)
**
************************************************************************/

POSITION DoMove(thePosition, theMove)
     POSITION thePosition;
     MOVE theMove;
{
  XOBlank turn,WhoseTurn();
  int i,free=0;
  for (i=theMove*(WIN4_HEIGHT+1)+WIN4_HEIGHT;
       (thePosition & (1 << i)) == 0; --i)
    free++;
  if (free == 0) return kBadPosition;
  turn=WhoseTurn(thePosition);
  thePosition &= ~(1 << i);
  return (thePosition | ((2+(int)turn)<<i));
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

PrintComputersMove(computersMove,computersName)
     MOVE computersMove;
     STRING computersName;
{
  printf("%8s's move              : %2d\n", computersName, computersMove+1);
}

/************************************************************************
**
** NAME:        Primitive
**
** DESCRIPTION: Return the value of a position if it fulfills certain
**              'primitive' constraints. Some examples of this is having
**              four-in-a-row. There are two 
**              primitives it can immediately check for, when the board
**              is filled but nobody has won = primitive tie. Four in
**              a row is a primitive lose, because the player who faces
**              this board has just lost. I.e. the player before him
**              created the board and won. Otherwise undecided.
** 
** INPUTS:      POSITION position : The position to inspect.
**
** OUTPUTS:     (VALUE) an enum which is oneof: (win,lose,tie,undecided)
**
** CALLS:       PositionToBoard()
**
** WARNING:     Behavior undefined for a position which is impossible !!
**
************************************************************************/

VALUE Primitive(pos) 
     POSITION pos;
{
  int ul[WIN4_WIDTH][WIN4_HEIGHT];//upper left
  int l[WIN4_WIDTH][WIN4_HEIGHT];//left
  int ll[WIN4_WIDTH][WIN4_HEIGHT];//lower left
  int u[WIN4_WIDTH][WIN4_HEIGHT];//up
  XOBlank board[WIN4_WIDTH][WIN4_HEIGHT+1];
  XOBlank realboard[WIN4_WIDTH][WIN4_HEIGHT];
  int col,row;
  PositionToBoard(pos,realboard);
  for (col=0;col<WIN4_WIDTH;col++)
    board[col][WIN4_HEIGHT]=2;
  for (col=0;col<WIN4_WIDTH;col++)
    for (row=0;row<WIN4_HEIGHT;row++)
      board[col][row]=realboard[col][row];
  // Check for four in a row
  // First do column 0:
  col=0;
  row=WIN4_HEIGHT-1;
  while (row>=0) {
    if (board[0][row]==2) {
      ul[0][row]=0;
      l[0][row]=0;
      ll[0][row]=0;
      u[0][row]=0;
    }
    else {
      ul[0][row]=1;
      l[0][row]=1;
      ll[0][row]=1;
      if (board[0][row+1]==board[0][row]) {
	u[0][row]=u[0][row+1]+1;
	if (u[0][row]==4) return(gStandardGame ? lose : win);  
      }
      else u[0][row]=1;
    }
    row--;
  }
  // Now do the other columns
  for (col=1;col<WIN4_WIDTH;col++) {
    row=WIN4_HEIGHT-1;
    while (row>0) {
      if (board[col][row]==2) {
	ul[col][row]=0;
	l[col][row]=0;
	ll[col][row]=0;
	u[col][row]=0;
      }
      else {
	if (board[col][row]==board[col][row+1]) {
	  u[col][row]=u[col][row+1]+1;
	  if (u[col][row]==4) return(gStandardGame ? lose : win);
	}
	else u[col][row]=1;
	if (board[col][row]==board[col-1][row+1]) {
	  ul[col][row]=ul[col-1][row+1]+1;
	  if (ul[col][row]==4) return(gStandardGame ? lose : win);
	}
	else ul[col][row]=1;
	if (board[col][row]==board[col-1][row]) {
	  l[col][row]=l[col-1][row]+1;
	  if (l[col][row]==4) return(gStandardGame ? lose : win);
	}
	else l[col][row]=1;
	if (board[col][row]==board[col-1][row-1]) {
	  ll[col][row]=ll[col-1][row-1]+1;
	  if (ll[col][row]==4) return(gStandardGame ? lose : win);
	}
	else ll[col][row]=1;
      }
      row--;
    }
    if (board[col][row]==2) {
      ul[col][row]=0;
      l[col][row]=0;
      ll[col][row]=0;
      u[col][row]=0;
    }
    else {
      if (board[col][row]==board[col][row+1]) {
	u[col][row]=u[col][row+1]+1;
	if (u[col][row]==4) return(gStandardGame ? lose : win);
      }
      else u[col][row]=1;
      if (board[col][row]==board[col-1][row+1]) {
	ul[col][row]=ul[col-1][row+1]+1;
	if (ul[col][row]==4) return(gStandardGame ? lose : win);
      }
      else ul[col][row]=1;
      if (board[col][row]==board[col-1][row]) {
	l[col][row]=l[col-1][row]+1;
	if (l[col][row]==4) return(gStandardGame ? lose : win);
      }
      else l[col][row]=1;
      ll[col][row]=1;
    }
  }
  //Now check if the board is full:
  for (col=0;col<WIN4_WIDTH;col++)
    for (row=0;row<WIN4_HEIGHT;row++)
      if (board[col][row]==2) return(undecided);
  return(tie);
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
**                                    (not used...)
**
** CALLS:       PositionToBoard()
**              GetValueOfPosition()
**              WhoseTurn()
**              GetPrediction()
**
************************************************************************/

PrintPosition(position,playerName,usersTurn)
     POSITION position;
     STRING playerName;
     BOOLEAN  usersTurn;
{
  int i,row;
  STRING GetPrediction();
  VALUE GetValueOfPosition();
  XOBlank board[WIN4_WIDTH][WIN4_HEIGHT], WhoseTurn();

  PositionToBoard(position,board);

  printf("\n      ");
  for (i=1;i<=WIN4_WIDTH;i++)
    printf(" %i",i);
  printf("\n      ");
  for (i=1;i<=WIN4_WIDTH;i++)
    printf(" |");
  printf("\n      ");
  for (i=1;i<=WIN4_WIDTH;i++)
    printf(" V");
  for (row=WIN4_HEIGHT-1;row>=0;row--) {
    printf("\n\n      ");
    for (i=0;i<WIN4_WIDTH;i++)
    printf(" %s",gBlankOXString[(int)board[i][row]]);
  }
  printf("\n\nPrediction: %s",
	 GetPrediction(position,playerName,usersTurn));
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

MOVELIST *GenerateMoves(position)
     POSITION position;
{
  MOVELIST *CreateMovelistNode(), *head = NULL;
  VALUE Primitive();
  XOBlank board[WIN4_WIDTH][WIN4_HEIGHT];
  int i;
  
  if (Primitive(position) == undecided) {
    PositionToBoard(position,board);
    for(i = 0 ; i < WIN4_WIDTH ; i++) {
      if(board[i][WIN4_HEIGHT-1] == Blank)
	head = CreateMovelistNode(i,head);
    }
    return(head);
  } else {
    return(NULL);
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
  int xpos, ypos;
  BOOLEAN ValidMove();
  char input = '0';
  BOOLEAN done = FALSE;
  USERINPUT ret, HandleDefaultTextInput();
  
  do {
    printf("%8s's move [(u)ndo/1-5] :  ", playerName);
    
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
  return(input[0] <= '9' && input[0] >= '1');
  // The '9' should rather be WIN4_WIDTH, but what the heck...
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

MOVE ConvertTextInputToMove(input)
     STRING input;
{
  return((MOVE) input[0] - '1'); /* user input is 1-5, our rep. is 0-4 */
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

PrintMove(theMove)
     MOVE theMove;
{
  /* The plus 1 is because the user thinks it's 1-9, but MOVE is 0-8 */
  printf("%d", theMove + 1); 
}


/************************************************************************
**
** NAME:        PositionToBoard
**
** DESCRIPTION: convert an internal position to a XOBlank-matrix.
** 
** INPUTS:      POSITION thePos   : The position input. 
**              XOBlank *board    : The converted XOBlank output matrix. 
**
************************************************************************/

PositionToBoard(pos,board)
     POSITION pos;
     XOBlank board[WIN4_WIDTH][WIN4_HEIGHT];
     // board is a two-dimensional array of size 
     // WIN4_WIDTH x WIN4_HEIGHT
{
  int col,row,h;
  for (col=0; col<WIN4_WIDTH;col++) {
    row=WIN4_HEIGHT-1;
    for (h=col*(WIN4_HEIGHT+1)+WIN4_HEIGHT;
	 (pos & (1 << h)) == 0; h--) {
      board[col][row]=2;
      row--;
    }
    h--;
    while (row >=0) {
      if ((pos & (1<<h)) != 0) board[col][row]=1;
      else board[col][row]=0;
      row--;
      h--;
    }
  }
}


/************************************************************************
**
** NAME:        WhoseTurn
**
** DESCRIPTION: Return whose turn it is - either x or o. Since x always
**              goes first, we know that if the board has an equal number
**              of x's and o's, that it's x's turn. Otherwise it's o's.
** 
** INPUTS:      POSITION  : a position
**
** OUTPUTS:     (XOBlank) : Either x or o, depending on whose turn it is
**
************************************************************************/

XOBlank WhoseTurn(pos)
      POSITION pos;
{
  int col,row,h;
  int xcount = 0,ocount = 0;
  for (col=0; col<WIN4_WIDTH;col++) {
    row=WIN4_HEIGHT-1;
    for (h=col*(WIN4_HEIGHT+1)+WIN4_HEIGHT;
	 (pos & (1 << h)) == 0; h--) row--;
    h--;
    while (row >=0) {
      if ((pos & (1<<h)) != 0) ocount++;
      else xcount++;
      row--;
      h--;
    }
  }
  if(xcount == ocount)
    return(x);            /* x always goes first */
  else
    return(o);
}


/************************************************************************
**
** NAME:        MyInitialPosition
**
** DESCRIPTION: Calculates the initial position
** 
** INPUTS:      none
**
** OUTPUTS:     POSITION (Initial Position)
**
************************************************************************/

POSITION MyInitialPosition() {
  POSITION p=1;
  int i;
  for (i=1;i<WIN4_WIDTH;++i)
    p = (p << (WIN4_HEIGHT+1))+1;
  return p;
}

/************************************************************************
**
** NAME:        MyNumberOfPos()
**
** DESCRIPTION: Calculates an upper bound for a position
** 
** INPUTS:      none
**
** OUTPUTS:     unsigned long int (number)
**
************************************************************************/
unsigned long int MyNumberOfPos() {
  int i;
  unsigned long int size=1;
  for (i=0;i<(WIN4_HEIGHT+1)*WIN4_WIDTH;i++)
    size *= 2;
  return size;
}


/************************************************************************
**
** NAME:        DebugMenu
**
** DESCRIPTION: Menu used to debub internal problems. Does nothing if
**              kDebugMenu == FALSE
** 
************************************************************************/

DebugMenu()
{
  char GetMyChar();

  do {
    printf("\n\t----- Module DEBUGGER for %s -----\n\n", kGameName);
    printf("\n\n\tb)\t(B)ack = Return to previous activity.\n");
    printf("\n\nSelect an option: ");
    
    switch(GetMyChar()) {
    case 'Q': case 'q':
      ExitStageRight();
    case 'H': case 'h':
      HelpMenus();
      break;
   case 'B': case 'b':
      return;
    default:
      BadMenuChoice();
      HitAnyKeyToContinue();
      break;
    }
  } while(TRUE);  
}

STRING kDBName = "win4" ;

int NumberOfOptions()
{
        return 2 ;
}

int getOption()
{    
        if(gStandardGame) return 1 ;
        return 2 ;
}
   
void setOption(int option)
{
        if(option == 1)
                gStandardGame = TRUE ;
        else
                gStandardGame = FALSE ;
} 

int GameSpecificTclInit(Tcl_Interp* interp,Tk_Window mainWindow) {}

