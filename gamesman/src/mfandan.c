/************************************************************************
**
** NAME:        mfandan.c
**
** DESCRIPTION: Fandango
**
** AUTHOR:      
**              
**
** DATE:        02/28/04
**
** UPDATE HIST:
**              02/28/04 Wrote WhoseTurn(...)
**              03/02/04 Wrote AnyPiecesLeft(...)
**              03/02/04 Wrote BoardToPosition(...)
**              03/02/04 Wrote PositionToBoard(...)
**              03/02/04 Debugged PrintPosition(...)
**              03/02/04 Wrote ValidTextInput(...)
**              03/03/04 Wrote Primitive(...)
**              03/05/04 Wrote ConvertTextToInput(...)
**              03/06/04 Wrote IndexToCoordinates(...)
**              03/06/04 Wrote CoordinatesToIndex(...)
**              03/06/04 Wrote Neighbor(...)
**              03/06/04 Wrote InBounds(...)
**              03/06/04 Wrote OtherDirection(...)
**              03/06/04 Wrote GenerateMoves(...)
** TODO LIST:
**              03/02/04 PrintPosition() needs to print info about how to make moves
**              03/02/04 Write DoMove(...)
**
**************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/

#include <stdio.h>
#include "gamesman.h"

extern STRING gValueString[];

POSITION gNumberOfPositions  = 0;     /* don't initialize yet, let generic_hash_init(...) decide */
POSITION kBadPosition        = -1;

POSITION gInitialPosition    =  0;
POSITION gMinimalPosition    =  0;

STRING   kGameName           = "Tic-Tac-Toe";
BOOLEAN  kPartizan           = TRUE;
BOOLEAN  kDebugMenu          = TRUE;
BOOLEAN  kGameSpecificMenu   = FALSE;
BOOLEAN  kTieIsPossible      = TRUE;
BOOLEAN  kLoopy               = FALSE;
BOOLEAN  kDebugDetermineValue = FALSE;

STRING   kHelpGraphicInterface =
"";

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
void PositionToBlankOX (POSITION temp , char* temp2){


    };
/*************************************************************************
**
** Every variable declared here is only used in this file (game-specific)
**
**************************************************************************/

#define BOARDSIZE     12           /* 4x3 board, must agree with the 2 definitions below */
#define BOARDHEIGHT    3           /* dimensions of the board */
#define BOARDWIDTH     4           /*  "               "      */
#define MAX_X          5           /* maximum number of pieces of X that is possible in a game */
#define MAX_O          5           /*  "            "            "O                  "         */

typedef enum possibleBoardPieces {
	Blank, o, x
} BlankOX;

typedef enum _direction { nodir=0,lowleft=1,down=2,lowright=3,left=4,right=6,upleft=7,up=8,upright=9 } Direction;
typedef struct _coord { int x,y; } Coordinates;

char gBlankOXString[] = { '.', 'O', 'X' };
int myPieces_array[9] = { '.', (BOARDSIZE-MAX_O-MAX_X), (BOARDSIZE-1),  /* treat empty spaces as pieces as well */
                          'O', 0, MAX_O,          /* info about the game pieces' diff. types and possible number of them in a game */
                          'X', 0, MAX_X };        /* used to pass into generic_hash_init(...) */

BlankOX turn = x;                                 /* keep track of whose turn it is, used by WhoseTurn(...) */

//---- Shing ----------------------------------------------
char slash[] = { '|' , '\\' , '|' , '/' , '|' }; /* HRS: now just an array instead of pointer to array, and also \ -> \\ */
//---------------------------------------------------------

/************************************************************************
**
** NAME:        InitializeDatabases
**
** DESCRIPTION: Initialize the gDatabase, a global variable.
** 
************************************************************************/

void InitializeGame()
{
  generic_hash_init(BOARDSIZE, myPieces_array, 0);  // pass null for function pointer
}

void FreeGame()
{}

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
  // fill me? If need, can get from mttt.c
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

void GameSpecificMenu() { }

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
** CALLS:       PositionToBlankOX(POSITION,*BlankOX)
**              BlankOX WhosTurn(*BlankOX)
**
************************************************************************/

POSITION DoMove(thePosition, theMove)
     POSITION thePosition;
     MOVE theMove;
{
  // fill me
  int i = 1;
  printf("%d\n",i);
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
  // fill me? if need, can get from mttt.c
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
  printf("%8s's move              : %2d\n", computersName, computersMove+1);
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
** CALLS:       BOOLEAN ThreeInARow()
**              BOOLEAN AllFilledIn()
**              PositionToBlankOX()
**
************************************************************************/

VALUE Primitive(position) 
     POSITION position;
{
  BlankOX board[BOARDSIZE];

  PositionToBoard(position,board);
  
  if (!AnyPiecesLeft(board,x) || !AnyPiecesLeft(board,o))   /* if either type of pieces is extinct */
    return(gStandardGame ? lose : win);                     /* that means last player to go captured last piece of current player? */
  else
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
** CALLS:       PositionToBlankOX()
**              GetValueOfPosition()
**              GetPrediction()
**
************************************************************************/

void PrintPosition(position,playerName,usersTurn)
     POSITION position;
     STRING playerName;
     BOOLEAN  usersTurn;
{


  // ----- Shing ----------------------------------------
  int i,j; /*HRS*/

  //  o --- o --- o --- o --- o
  //  |  \  |  /  |  \  |  /  |
  //  o --- x --- . --- o --- x
  //  |  /  |  \  |  /  |  \  |
  //  x --- x --- x --- x --- x
  

  // ----------------------------------------------------

  STRING GetPrediction();
  VALUE GetValueOfPosition();
  BlankOX theBlankOx[BOARDSIZE]; /* HRS: boardSize -> BOARDSIZE */

  PositionToBlankOX(position,theBlankOx);


  for (i=1; i<=BOARDHEIGHT; i++)          // for row
    {
      putchar('\n');    
      putchar('(');
      for ( j=1; j<=BOARDWIDTH; j++)
      {
	putchar(' ');
	putchar(j);
      }
      putchar(' ');
      putchar(')');
      putchar('\n');
      putchar(':');

      

      for ( j=1; j<=BOARDWIDTH; j++)   // for column
      {
	  putchar(' ');
	  if ( i % 2 == 0 )
	  {
	    putchar(slash[ j % 5 ]);
	    putchar(' ');
	    putchar(' ');
	  }
	  else
	  {
	    if ((j-1) % 6 == 0)
	      putchar(gBlankOXString[(int)theBlankOx[j-1]]); /* HRS */
	    else if ((j-1) % 6 == 1)
	      putchar(' ');
	    else
	      putchar('-');
	  }
      }
    }

  puts(GetPrediction(position,playerName,usersTurn)); /* HRS */
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
**              WhoseTurn(...),InBounds(...),CoordinatesToIndex(...),
**              IndexToCoordinates(...),Primitive(...),
**              Neighbor(...),OtherDirection(...),EachDirection(...),OtherPlayer
**
************************************************************************/

MOVELIST *GenerateMoves(position)
     POSITION position;
{
  MOVELIST *CreateMovelistNode(), *head = NULL;
  VALUE Primitive();
  Coordinates IndexToCoordinates(int);
  int CoordinatesToIndex(Coordinates);
  int InBounds(Coordinates);
  Coordinates Neighbor(Coordinates,Direction);
  Direction EachDirection();
  Direction OtherDirection(Direction);
  BlankOX OtherPlayer();
  BlankOX WhoseTurn();

  BlankOX board[BOARDSIZE];
  int i;
  Direction dir;
  int intdir;
  Coordinates pos,attackingpos;
  int cap=0;   // capture mode
  int numofcaps; //number of captures possible for one direction of move

  if (Primitive(position) == undecided) {
    PositionToBoard(position,board);
    for(i = 0 ; i < BOARDSIZE ; i++) {
      if(board[i] == WhoseTurn()) {      // if current player's piece then we consider where we can move this
	pos = IndexToCoordinates(i);
	for(;dir = EachDirection();) {
	  intdir = (int)dir;
	  if(intdir>=5)  // collapse direction into 3 bits instead of 4
	    intdir--;    // 5 is not a direction, don't need that
	  intdir--;      // start at 0

	  if(InBounds(Neighbor(pos,dir)) && board[i]==Blank) {      // vacant neighboring cell exists
	    numofcaps=0;
	    
	    attackingpos = Neighbor(Neighbor(pos,dir),dir);
	    if(InBounds(attackingpos) && board[CoordinatesToIndex(attackingpos)]==OtherPlayer()) {  // have enemy, can attack by approach
	      numofcaps++;
	      cap = 1;  // attack by approach mode
	      head = CreateMovelistNode(pos.x<<18 | pos.y<<5 | intdir<<2 | cap, head);
	    } // end if can attack by approach
	    
	    attackingpos = Neighbor(pos,OtherDirection(dir));
	    if(InBounds(attackingpos) && board[CoordinatesToIndex(attackingpos)]==OtherPlayer()) { // have enemy, can attack by withdraw
	      numofcaps++;
	      cap = 2; // attack by withdraw mode
	      head = CreateMovelistNode(pos.x<<18 | pos.y<<5 | intdir<<2 | cap, head);
	    } // end if can attack by withdraw
	    
	    if(numofcaps==0) { // no captures possible for this move
	      cap = 0;
	      head = CreateMovelistNode(pos.x<<18 | pos.y<<5 | intdir<<2 | cap, head);
	    } //end if no possible captures
	  } // end if vacant target cell (i.e. can move)
	} // end for this direction
      } // end if this piece is current players piece
    } // end for this cell of board
    return(head);
  } else { // end if undecided (not win or lose)
    return(NULL);
  }
}

/************************************************************************
 * Inputs:
 * Description: enumerates all possible directions. ie. one direction per call
 *              in the end, will return nodir = 0
 * Calls:
 ***********************************************************************/
Direction EachDirection() {
  static int dir = 0;
  return (Direction)( ((++dir)%10==5) ? dir=6 : dir );
}

/************************************************************************
 * Inputs: Direction
 * Description: returns opposite direction
 * Calls:
 ***********************************************************************/
Direction OtherDirection(Direction dir) {
  switch(dir) {
  case up: return down;
  case down: return up;
  case left: return right;
  case right: return left;
  case upleft: return lowright;
  case upright: return lowleft;
  case lowleft: return upright;
  case lowright: return upleft;
  default: return nodir;
  }
  return nodir;  // never reached
}

/************************************************************************
 * Inputs:      Coordinates pos, Direction dir
 * Description: returns the Coordinates for the neighboring space of pos
 * Calls: 
 ***********************************************************************/
Coordinates Neighbor(Coordinates pos, Direction dir) {
  if (dir == up || dir == upleft || dir == upright)
    pos.y--;
  else if (dir == down || dir == lowleft || dir == lowright)
    pos.y++;

  if (dir == left || dir == upleft || dir == lowleft)
    pos.x--;
  else if (dir == right || dir == upright || dir == lowright)
    pos.x++;

  return pos;
}

/************************************************************************
 * Inputs:      Coordinates
 * Description: true if within board boundary, false otherwise
 * Calls: IndexToCoordinates(...)
 ***********************************************************************/
int InBounds(Coordinates pos) {
  return (pos.x>=0) && (pos.x<BOARDWIDTH) && (pos.y>=0) && (pos.y<BOARDHEIGHT);
}

/************************************************************************
 * Inputs:      integer - array index of board
 * Description: returns Coordinates - position on board corresponding to the index
 *              x,y both start at 0
 * Calls:
 ***********************************************************************/
Coordinates IndexToCoordinates(int index) {
  Coordinates pos;
  pos.y = index / BOARDWIDTH;
  pos.x = index % BOARDWIDTH;
  return pos;
}

/************************************************************************
 * Inputs:      Coordinates
 * Description: returns array index corresponding to position given by input
 * Calls:
 ***********************************************************************/
 int CoordinatesToIndex(Coordinates pos) {
   return pos.x + pos.y*BOARDWIDTH;
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
    printf("%8s's move [(u)ndo/1-9] :  ", playerName);
    
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
**              ** 
** INPUTS:      STRING input : The string input the user typed.
**
** OUTPUTS:     BOOLEAN : TRUE iff the input is a valid text input.
**
************************************************************************/

BOOLEAN ValidTextInput(input)
     STRING input;
{
  char col;                           /* column: a, b, c, etc. */                                                              
  int row,dir,cap;                    /* row: 1,2,3,etc.,
					 direction: 1,2,3,4,6,7,8,9 (based on numpad),
					 ,capture mode: a, w, n*/
  int scan_result =  sscanf(input,"%c%d-%d-%c",col,row,dir,cap);

  col = tolower(col);     /* to make case insensitive */
  cap = tolower(cap);     /* "          "         "   */

  return
    (scan_result == 4) &&                        /* did we manage to get suitable vals for col,row,dir,cap? */
    ('a' <= col < ('a'+BOARDWIDTH)) &&           /* is the column a character and in range? */
    (1 <= row <= BOARDHEIGHT) &&                 /* is the row in range? i.e. not out of board */
    ((cap == 'a')||(cap == 'w')||(cap == 'n'));  /* is capture mode a valid character? */    
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
  char col;                           /* column */                                                              
  int row,dir,cap;                    /* row,direction,capture mode ('a','w','n')*/
  int scan_result =  sscanf(input,"%c%d-%d-%c",col,row,dir,cap);
  col = tolower(col);     /* to make case insensitive */
  cap = tolower(cap);     /* "          "         "   */

  /* we want:
     ___________________________________________
     |1:unused| 13:col | 13:row | 3:dir | 2:cap |
     -------------------------------------------*/
  if(cap == 'a')          /* encode 'a' as 01, 'w' as 10 and n as 00 */
    cap = 1;
  else if(cap == 'w')
    cap = 2;
  else // cap == 'n'
    cap = 0;

  row -= 1;    // we start at 0, user starts at 1
  col = col - 'a';   // make col start at 0 and count up each alpha. i.e. 'b' = 1, 'c' = 2

  if(dir >= 5)
    dir--;    // 5 is not a dir
  dir--;      // make dir start at 0

  // mask everything into the MOVE to return
  return (col<<18) | (row<<5) | (dir<<2) | cap;
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
	/* The plus 1 is because the user thinks it's 1-9, but MOVE is 0-8 */
	printf("%d", theMove + 1); 
}

/************************************************************************
*************************************************************************
** BEGIN   FUZZY STATIC EVALUATION ROUTINES. DON'T WORRY ABOUT UNLESS
**         YOU'RE NOT GOING TO EXHAUSTIVELY SEARCH THIS GAME
*************************************************************************
************************************************************************/

/************************************************************************
*************************************************************************
**         EVERYTHING BELOW THESE LINES IS LOCAL TO THIS FILE
*************************************************************************
************************************************************************/

/***********************************************************************
 *
 * Input: A position (an integer), a board (an array)
 *
 * Desc: Changes the value of the given board based on the given position (int)
 *
 * Calls: generic_unhash(...)
 **********************************************************************/
PositionToBoard(POSITION position, BlankOX *board) {
  generic_unhash(position,board);
}

/************************************************************************
 *
 * Input: A board
 *
 * Desc: Returns a POSITION (which is an int) that is a board encoded as an int
 *
 * Calls: generic_hash(...)
 ***********************************************************************/
POSITION BoardToPosition(BlankOX *board) {
  return generic_hash(board);
}

/***********************************************************************
 *
 * Inputs: A board, a piece type
 *
 * Desc: Returns true if the given board has at least one piece of given type
 *       Otherwise, return false
 * Calls:
 **********************************************************************/
int AnyPiecesLeft(BlankOX theBoard[], BlankOX thePiece) {
  int i;
  int anyLeft = 0;    /* assume there are none left, then set true if we see one */
  
  for(i=0; i<BOARDSIZE; i++)
    if(theBoard[i] == thePiece)
      anyLeft = 1;    /* we saw one, so set to true */

  return anyLeft;
}

/************************************************************************
 *
 * Inputs:
 *
 * Desc: Returns either o or x (which are from an enumeration) based on
 *       whose turn it is. i.e. return x if it's x's turn.
 *
 * Calls:
 ***********************************************************************/
BlankOX WhoseTurn() {
  return turn;   /* turn is a global variable that keeps track of this */
}

BlankOX OtherPlayer() {
  return turn==x ? o : x;
}

STRING kDBName = "ttt" ;
     
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

