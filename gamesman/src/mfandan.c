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
**              03/02/04 Wrote BlankOXToPosition(...)
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
**              04/11/04 Changed the erronous naming of PositionToBoard(...) -> PositionToBlankOX(...)
**              04/17/04 debugged PrintPosition(..)
**              04/23/04 debugged EachDirection(...)
**              04/23/04 debugged ConvertTextToInput(...)
**              04/24/04 wrote PrintMove(...)
**              04/24/04 debugged generateMoves(...) still needs work
**              04/24/04 debugged EachDirection(...)
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
#include "hash.h"

int debug = 3;
int printMethods = 0;

extern STRING gValueString[];

POSITION gNumberOfPositions  = 0;     /* don't initialize yet, let generic_hash_init(...) decide */
POSITION kBadPosition        = -1;

POSITION gInitialPosition    =  0;
POSITION gMinimalPosition    =  0;

STRING   kGameName           = "Fandango";
BOOLEAN  kPartizan           = TRUE;
BOOLEAN  kDebugMenu          = TRUE;
BOOLEAN  kGameSpecificMenu   = FALSE;
BOOLEAN  kTieIsPossible      = TRUE;
BOOLEAN  kLoopy               = TRUE;
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


#define BOARDSIZE     12           /* 4x3 board, must agree with the 2 definitions below */
#define BOARDHEIGHT    3           /* dimensions of the board */
#define BOARDWIDTH     4           /*  "               "      */
#define MAX_X          5           /* maximum number of pieces of X that is possible in a game */
#define MAX_O          5           /*  "            "            "O                  "         */

#define COMMENTSPACE   5           /* number of spaces to the right of board before comment starts */

/* oh shit, API doesn't use 32 bit elements for board array!
typedef enum possibleBoardPieces {
	Blank, o, x
} BlankOX;
*/
// this will have to do
typedef char BlankOX;
BlankOX X = 'X';
BlankOX O = 'O';
BlankOX B = '.';

typedef enum _direction { nodir=0,lowleft=1,down=2,lowright=3,left=4,right=6,upleft=7,up=8,upright=9 } Direction;
typedef struct _coord { int x,y; } Coordinates;

char gBlankOXString[] = { '.', 'O', 'X' };
int myPieces_array[10] = { '.', (BOARDSIZE-MAX_O-MAX_X), (BOARDSIZE-1),  /* treat empty spaces as pieces as well */
                          'O', 0, MAX_O,          /* info about the game pieces' diff. types and possible number of them in a game */
                          'X', 0, MAX_X,          /* used to pass into generic_hash_init(...) */
			  -1 };                   // mark the end of array
BlankOX turn = 'X';                                 /* keep track of whose turn it is, used by WhoseTurn(...) */

//---- Shing ----------------------------------------------
char slash[] = {'|',' ',' ','\\',' ',' ','|',' ',' ','/',' ',' '}; /* HRS: now just an array instead of pointer to array, and also \ -> \\ */
//---------------------------------------------------------

/////////////////////////////////////////// F U N C T I O N   P R O T O T Y P E S ///////////////////////////////////////////////////////
void space(int n);
POSITION BlankOXToPosition(BlankOX* board);
int AnyPiecesLeft(BlankOX theBoard[], BlankOX thePiece);
Coordinates IndexToCoordinates(int index);
Coordinates Neighbor(Coordinates pos, Direction dir);

void dbg(char *);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void dbg(char* msg) {
  if(printMethods) puts(msg);

};

/************************************************************************
**
** NAME:        InitializeDatabases
**
** DESCRIPTION: Initialize the gDatabase, a global variable.
** 
************************************************************************/

void InitializeGame()
{
  dbg("->InitializeGame");
  gNumberOfPositions = generic_hash_init(BOARDSIZE, myPieces_array, NULL);  // pass null for function pointer
  char initialBoard[BOARDSIZE] = { O,O,O,O,
				   X,B,B,O,
				   X,X,X,X };
  POSITION initialPos =  BlankOXToPosition(initialBoard);
  gInitialPosition = initialPos;
  gMinimalPosition = initialPos;
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
  printf("->DoMove\n");
  // fill me
  
  int index,intdir,cap;
  Direction dir;
  BlankOX board[BOARDSIZE];
  char movingPiece;
  Coordinates coord;
  POSITION newPosition;

  cap = 3 & theMove;
  intdir = (28 & theMove) >> 2;
  index = (0xffffffe0 & theMove) >> 5;

  if(intdir>=5)
    intdir++;
  intdir++;
  dir = (Direction)intdir;

  coord = IndexToCoordinates(index);

  printf("DoMove sees: %d\t\n",theMove);
  PositionToBlankOX(thePosition, board);
  movingPiece = board[index];
  board[index] = B;
  board[CoordinatesToIndex(Neighbor(coord,(Direction)dir))] = movingPiece;

  newPosition = BlankOXToPosition(board);
  return newPosition;
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
  dbg("->PrintComputersMove");
  printf("%8s's move              : %2d\n", computersName, computersMove+1);
}

/************************************************************************
**
** NAME:        Primitive
**
** DESCRIPTION: Determines whether it's win/lose/tie or undecided based
**              on what's left on the board.
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
  dbg("->Primitive");
  BlankOX board[BOARDSIZE];

  PositionToBlankOX(position,board);
  
  if (!AnyPiecesLeft(board,X) || !AnyPiecesLeft(board,O))   /* if either type of pieces is extinct */
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
  dbg("->PrintPos");
  int x, y;
  int maxx = (BOARDWIDTH - 1 ) * 6;
  int maxy = (BOARDHEIGHT - 1) * 2;

  int slashFlag = 0;
  int commentx = maxx + 1 + COMMENTSPACE;

  STRING  GetPrediction();
  VALUE   GetValueOfPosition();

  BlankOX theBoard[BOARDSIZE];

  PositionToBlankOX(position,theBoard);

  for (y = 0; y <= maxy; y++) {
    for ( x = 0; x <= maxx; x++) {
      if ( y % 2 == 1 ) {                                      // rows with slashes  |  \  |  / ...
	putchar(slash[(x + (slashFlag? 0 : 6)) % 12]);
	slashFlag = !slashFlag;
      }
      else {                                                     // rows with cells
	if ((x % 6) == 0) {                                      // if it's time to print a cell
	  int boardIndex = (y/2) * BOARDWIDTH + (x/6);
	  putchar(theBoard[boardIndex]);
	} else if ( (x % 6) == 1 || (x % 6) == 5 )
	  putchar(' ');
	else
	  putchar('-');
      }
    }
    puts("");
  }

  puts("");
  puts(GetPrediction(position,playerName,usersTurn));
  dbg("<-PrintPos");
}

void space(int n) {
  int i = 0;
  for(; i < n; i++)
    putchar(' ');
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
  dbg("->GenerateMoves");
  MOVELIST *CreateMovelistNode(), *head = NULL;
  VALUE Primitive();
  Coordinates IndexToCoordinates(int);
  int CoordinatesToIndex(Coordinates);
  int InBounds(Coordinates);
  Coordinates Neighbor(Coordinates,Direction);
  Direction EachDirection();
  Direction OtherDirection(Direction);
  BlankOX OtherPlayer();
  BlankOX WhoseTurn(int);

  // update turn (todo: not need?)
  turn = (whoseMove(position) == X ? 2 : 1);
  printf("whoseMove: %d\n",whoseMove(position));


  BlankOX board[BOARDSIZE];
  int i;
  Direction dir;
  int intdir;
  Coordinates pos,attackingpos;
  int cap=0;   // capture mode
  int numofcaps; //number of captures possible for one direction of move
  int move;

  if (Primitive(position) == undecided) {
    PositionToBlankOX(position,board);
    if(debug>2) printf("Primitive undecided\n");
    for(i = 0 ; i < BOARDSIZE ; i++) {
      if(board[i] == WhoseTurn(position)) {      // if current player's piece then we consider where we can move this
	pos = IndexToCoordinates(i);
	if(debug>2) printf("Found side's piece at %d,%d\n",pos.x,pos.y);
	for(;dir = EachDirection();) {
	  intdir = (int)dir;
	  if(intdir>=5)  // collapse direction into 3 bits instead of 4
	    intdir--;    // 5 is not a direction, don't need that
	  intdir--;      // start at 0

	  numofcaps=0;
	  if(InBounds(Neighbor(pos,dir)) && board[CoordinatesToIndex(Neighbor(pos,dir))]==B) {      // vacant neighboring cell exists
	    if(debug>2) printf("Found vacant neighbor at dir: %d\n",dir);
	    attackingpos = Neighbor(Neighbor(pos,dir),dir);
	    if(InBounds(attackingpos) && board[CoordinatesToIndex(attackingpos)]==OtherPlayer()) {  // have enemy, can attack by approach
	      numofcaps++;
	      if(debug>2) printf("Can attack someone by approach @ dir: %d\n",dir);
	      cap = 1;  // attack by approach mode
	      move = CoordinatesToIndex(pos)<<5 | intdir<<2 | cap;
	      if(debug>2) printf("Generated move: %d\n",move);
	      head = CreateMovelistNode(move, head);
	    } // end if can attack by approach
	    
	    attackingpos = Neighbor(pos,OtherDirection(dir));
	    if(InBounds(attackingpos) && board[CoordinatesToIndex(attackingpos)]==OtherPlayer()) { // have enemy, can attack by withdraw
	      numofcaps++;
	      if(debug>2) printf("Can attack someone by withdraw @ dir: %d\n",OtherDirection(dir));
	      cap = 2; // attack by withdraw mode
	      move = CoordinatesToIndex(pos)<<5 | intdir<<2 | cap;
	      if(debug>2) printf("Generated move: %d\n",move);
	      head = CreateMovelistNode( move, head);
	    } // end if can attack by withdraw
	    
	    if(numofcaps==0) { // no captures possible for this move
	      cap = 0;
	      move = CoordinatesToIndex(pos)<<5 | intdir<<2 | cap;
	      if(debug>2) printf("Generated move: %d\n",move);
	      head = CreateMovelistNode( move, head);
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
  static Direction dir = lowleft;

  switch(dir) {
  case lowleft: return dir=left;
  case left: return dir=upleft;
  case upleft: return dir=up;
  case up: return dir=upright;
  case upright: return dir=right;
  case right: return dir=lowright;
  case lowright: return dir=down;
  case down: return dir=nodir;
  case nodir: return dir=lowleft;
  }
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
  dbg("->GetAndPrintPlayersMove");
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
  dbg("->ValidTextInput");

  int pos,dir;
  int cap;
  int scan_result;

  scan_result = sscanf(input,"%d %d %d",&pos,&dir,&cap);

  if(scan_result != 3)
    return FALSE;
  
  if(pos<1 || pos>BOARDSIZE)
    return FALSE;

  if(dir==5 || dir<0 || dir>9)
    return FALSE;  

  if(cap<0 || cap>2)
    return FALSE;

  return TRUE;
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
  /* we want:
     ___________________________________________
     |              27: pos     | 3:dir | 2:cap |
     -------------------------------------------*/
      /* encode 'a' as 01, 'w' as 10 and n as 00 */

  int pos,dir,cap;

  dbg("->ConvertTextInputToMove");

  int scan_result = sscanf(input,"%d %d %d", &pos, &dir, &cap);
  
  pos--;  // we start at 0 while user starts at 1

  if(dir >= 5)
    dir--;    // 5 is not a dir
  dir--;      // make dir start at 0

  int move = (pos<<5) | (dir<<2) | cap;
  if(debug>1) printf("Resulting move %d\n",move);

  // mask everything into the MOVE to return
  return move;
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
   int pos,dir,cap;

  cap = 3 & theMove;
  dir = (28 & theMove) >> 2;
  pos = (0xffffffe0 & theMove) >> 5;

  if(dir >= 5)
    dir++;
  dir++;

  printf(" [%d %d %d]",pos+1,dir,cap);
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
PositionToBlankOX(POSITION position, BlankOX *board) {
  dbg("->PositionToBlankOX");
  generic_unhash(position,(char *)board);  // TODO: doesn't API stick with their own types?? (arg2)
}

/************************************************************************
 *
 * Input: A board
 *
 * Desc: Returns a POSITION (which is an int) that is a board encoded as an int
 *
 * Calls: generic_hash(...)
 ***********************************************************************/
POSITION BlankOXToPosition(BlankOX *board) {
  dbg("->BlankOXToPosition");
  return generic_hash((char *)board, turn==X?2:1 );  // TODO: doesn't API stick with their own types?? (arg 1)
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
  dbg("->AnyPiecesLeft");
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
BlankOX WhoseTurn(int pos) {
  return (whoseMove(pos) == 2 ? X : O);
}

BlankOX OtherPlayer() {
  return turn==X ? O : X;
}

STRING kDBName = "fandan" ;
     
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


  // ----- Shing ----------------------------------------

  //  o --- o --- o --- o --- o
  //  |  \  |  /  |  \  |  /  |
  //  o --- x --- . --- o --- x
  //  |  /  |  \  |  /  |  \  |
  //  x --- x --- x --- x --- x

  //  (o)-(o)-(o)-(o)-(o)
  //   | \ | / | \ | / |
  //  (o)-(x)-( )-(o)-(x)
  //   | / | \ | / | \ |
  //  (x)-(x)-(x)-(x)-(x)

  //  [o]-[o]-[o]-[o]-[o]
  //   | \ | / | \ | / |
  //  (o)-(x)-( )-(o)-(x)
  //   | / | \ | / | \ |
  //  (x)-(x)-(x)-(x)-(x)

  // ----------------------------------------------------
