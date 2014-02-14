/************************************************************************
**
** NAME:        mcon.cc
**
** DESCRIPTION: The Connections Game
**
** AUTHOR:      Brian Carnes
**
**************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/

#include <stdio.h>


#define EXTERNC extern "C"

EXTERNC
{
#include "gamesman.h"

	//#pragma weak Tcl_CreateCommand
}

POSITION gNumberOfPositions  =  0;
POSITION gInitialPosition    =  0;
POSITION gMinimalPosition    =  0;
POSITION kBadPosition        = (unsigned long long)-1; /* This can never be the rep. of a position */

STRING   kAuthorName         = "Brian Carnes";
STRING   kGameName           = "Connections";
BOOLEAN  kPartizan           = TRUE;
BOOLEAN  kSupportsHeuristic  = FALSE;
BOOLEAN  kSupportsSymmetries = FALSE;
BOOLEAN  kSupportsGraphics   = TRUE;
BOOLEAN  kDebugMenu          = TRUE;
BOOLEAN  kGameSpecificMenu   = FALSE;
BOOLEAN  kTieIsPossible      = TRUE;
BOOLEAN  kLoopy               = FALSE;
BOOLEAN  kDebugDetermineValue = FALSE;

STRING   kHelpGraphicInterface = "No help";

STRING   kHelpTextInterface    = "No help";

STRING   kHelpOnYourTurn = "No help";

STRING   kHelpStandardObjective = "No help";

STRING   kHelpReverseObjective = "No help";

STRING   kHelpTieOccursWhen = "No help";

STRING   kHelpExample = "No help";

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

bool UnderTcl=false;
unsigned BoardSize = 2   *2;

const int Empty=0;
const int Red=1;
const int White=2;

const int BadRed=-Red;
const int BadWhite=-White;
const int BadCorner=-99;
const int BadOOB=-98; // Out of Bounds


#define ISSET(POS,P) ((1<<(P))&(POS))

struct Con {

  Con(POSITION pos) : pos(pos){}
  operator POSITION() const {return pos;}

  int Turn() const {
    int moves=0;
    for (int p=pos; p; p>>=1)
      moves += p&1;
    return !!(moves&1);
  }

  int PieceAt(int i, int j) const {
    int p = Pos(i,j);
    if (p < 0) return Empty;
    return PieceAt(p);
  }

  int PieceAt(int p) const {
    return (pos>>(p*2))&3;
  }

  void DoMove(MOVE move) {
    pos |= (Turn()?White:Red)<<(move*2);
  }

  static int NumberOfPositions() {return 1<<(Moves()*2);}
  static int Moves() {return BoardSize/4*(BoardSize+1)+BoardSize;}

  static int Pos(unsigned i, unsigned j) {
    if (i > BoardSize || j > BoardSize) return BadOOB;
    if ((i == 0 || i == BoardSize) && (j == 0 || j == BoardSize)) 
      return BadCorner;
    if (i&1 && !(j&1)) return BadRed;
    if (j&1 && !(i&1)) return BadWhite;

    if (j==0) i += 2;
    if (j==BoardSize) i -= 2;
    if (j&1) return (BoardSize/2)-1 + i/2 + (j-1)/2*(BoardSize+1);
    else return -2 + i/2 + j/2*(BoardSize+1);
    return 0;
  }

  void Print() {
    for (unsigned j = 0; j < BoardSize*2+1; j++) {
      for (unsigned i = 0; i < BoardSize*2+1; i++) {
	if (i&1 || j&1) {
	  printf(" ");
	  continue;
	}
	int p = Pos(i/2,j/2);
	switch (p) {
	case BadCorner: printf(" "); break;
	case BadRed: printf("R"); break;
	case BadWhite: printf("W"); break;
	default: {
	  int color = PieceAt(p);
	  printf("%c",color==Red?'R':color==White?'W':p+'a'); 
	  } break;
	}
      }
      printf("\n");
    }    
  }

  static void PreCalc() {
  }

private:
  POSITION pos;
};


/************************************************************************
**
** NAME:        InitializeDatabases
**
** DESCRIPTION: Initialize the gDatabase, a global variable.
** 
************************************************************************/

EXTERNC void InitializeDatabasesForReal()
{
  Con::PreCalc();

  gNumberOfPositions = Con::NumberOfPositions();
}


EXTERNC void InitializeGame()
{
    InitializeDatabasesForReal();
}

EXTERNC void FreeGame()
{}


#ifndef NO_GRAPHICS

static int
XYtoPos(ClientData dummy,Tcl_Interp *interp, int argc, char** argv)
{
  int x,y;

  if (argc != 3) {
    Tcl_SetResult(interp,  "wrong # args: Scored (int)x (int)y", TCL_STATIC);
    return TCL_ERROR;
  }
  else {
    if (sscanf(argv[1], "%d", &x) == EOF)
      return TCL_ERROR;
    if (sscanf(argv[2], "%d", &y) == EOF)
      return TCL_ERROR;
    
		Tcl_SetResult(interp, StrFromI((POSITION)Con::Pos(x,y)), SafeFreeString);
    return TCL_OK;
  }
}

static int
GetTurn(ClientData dummy,Tcl_Interp *interp, int argc, char** argv)
{
  POSITION pos;

  if (argc != 2) {
    Tcl_SetResult(interp,  "wrong # args: Scored (int)Position", TCL_STATIC);
    return TCL_ERROR;
  }
  else {
    if (sscanf(argv[1], POSITION_FORMAT, &pos) == EOF)
      return TCL_ERROR;
    
    Con board(pos);
    Tcl_SetResult(interp, StrFromI(board.Turn()), SafeFreeString);
    return TCL_OK;
  }
}

static int
MoveToInter(ClientData dummy,Tcl_Interp *interp, int argc, char** argv)
{
  POSITION pos;

  if (argc != 2) {
    Tcl_SetResult(interp,  "wrong # args: Scored (int)Position", TCL_STATIC);
    return TCL_ERROR;
  }
  else {
    if (sscanf(argv[1], POSITION_FORMAT, &pos) == EOF)
      return TCL_ERROR;
    
    int xypos=0;
    // do this really cheesily, since it's only needed by the GUI
    for (unsigned i = 0; i <= BoardSize; i++)
      for (unsigned j = 0; j <= BoardSize; j++) {
	if ((unsigned)Con::Pos(i,j)==pos)
	  xypos=i+j*(BoardSize+1);
      }
	
    Tcl_SetResult(interp, StrFromI(xypos), SafeFreeString);
    return TCL_OK;
  }
}

EXTERNC int GameSpecificTclInit(Tcl_Interp* interp, Tk_Window mainWindow) {

  UnderTcl=true;
  Tcl_CreateCommand(interp, "C_XYtoPos", (Tcl_CmdProc*) XYtoPos, (ClientData) mainWindow,
		    (Tcl_CmdDeleteProc*) NULL);
  Tcl_CreateCommand(interp, "C_GetTurn", (Tcl_CmdProc*) GetTurn, (ClientData) mainWindow,
		    (Tcl_CmdDeleteProc*) NULL);
  Tcl_CreateCommand(interp, "C_MoveToInter", (Tcl_CmdProc*) MoveToInter, (ClientData) mainWindow,
		    (Tcl_CmdDeleteProc*) NULL);
  return TCL_OK;
}

void* gGameSpecificTclInit = (void*) GameSpecificTclInit;

#else
void* gGameSpecificTclInit = NULL;
#endif

/************************************************************************
**
** NAME:        DebugMenu
**
** DESCRIPTION: Menu used to debub internal problems. Does nothing if
**              kDebugMenu == FALSE
** 
************************************************************************/

EXTERNC void DebugMenu()
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

EXTERNC void GameSpecificMenu() { }

/************************************************************************
**
** NAME:        SetTclCGameSpecificOptions
**
** DESCRIPTION: Set the C game-specific options (called from Tcl)
**              Ignore if you don't care about Tcl for now.
** 
************************************************************************/

EXTERNC void SetTclCGameSpecificOptions(int theOptions[])
{
  InitializeDatabasesForReal();
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

EXTERNC POSITION DoMove(POSITION thePosition, MOVE theMove)
{
  Con board(thePosition);
  board.DoMove(theMove);
  return board;
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

EXTERNC POSITION GetInitialPosition()
{
  return 0;
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

EXTERNC void PrintComputersMove(MOVE computersMove,STRING computersName)
{
  printf("%8s's move              : ", computersName);
  PrintMove(computersMove);
  printf("\n");
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

// Can we get from x,y to Bottom/Right Edge 
bool Traverse(const Con& board,int x,int y,int Color,int visited) {
  if (Color == Red) {
    if ((unsigned)y == BoardSize) return true;
  } else {
    if ((unsigned)x == BoardSize) return true;
  }
     
  for (int d = 0; d < 4; d++) {
    int dx=0,dy=0;
    switch (d) {
    case 0: dx=-1; break;
    case 1: dx=+1; break;
    case 2: dy=-1; break;
    case 3: dy=+1; break;
    }
    
    int p = board.Pos(x+dx,y+dy);
    if (p < 0) 
      continue; // can't go that way
    //    printf("In traverse, pos %i,%i, got pos %i\n",x,y,p);
    if (board.PieceAt(p) != Color)
      continue; // no stepping stone
    if (visited&(1<<p))
      continue; // already been there

    visited |= 1<<p; // use that stepping stone
    
    if (Traverse(board,x+dx*2,y+dy*2,Color,visited)) 
      return true; // made it
  }
  return false;
}

EXTERNC VALUE Primitive(POSITION position) 
{
  Con board(position);

  int whowon=0;

  for (unsigned i = 1; i < BoardSize; i+=2)
    if (Traverse(board,i,0,Red,0)) {
      whowon=Red; goto wehaveawinner;
    }

  for (unsigned j = 1; j < BoardSize; j+=2)
    if (Traverse(board,0,j,White,0)) {
      whowon=White; goto wehaveawinner;
    }
  
  return undecided;

wehaveawinner:

  return lose;
  if (board.Turn())
    return (whowon==Red)?lose:win; // 2nd person just lost/won
  else
    return (whowon==White)?lose:win; // 1st person...
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

EXTERNC void PrintPosition(POSITION position,STRING playerName,BOOLEAN usersTurn)
{
  Con board(position);
  
  board.Print();
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

EXTERNC MOVELIST *GenerateMoves(POSITION position)
{
  MOVELIST *head = NULL;
  
  //  printf("got GenerateMoves call with position %08x\n",position);

  if (Primitive(position) != undecided)
    return NULL;

  Con board(position);

  int turn = board.Turn();
  
  for (unsigned j = 0; j <= BoardSize; j++)
    for (unsigned i = 0; i <= BoardSize; i++) {
      int p = board.Pos(i,j);
      if (p < 0) continue;
      if (turn)
	if (j==0 || j==BoardSize) continue; else;
      else
	if (i==0 || i==BoardSize) continue; else;
      if (board.PieceAt(p)) continue;
      head = CreateMovelistNode(p,head);
      //      printf("Found a move %i\n",p);
    }

  return(head);
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

EXTERNC USERINPUT GetAndPrintPlayersMove(POSITION thePosition, MOVE *theMove, STRING playerName)
{
  BOOLEAN ValidMove = FALSE;
  USERINPUT ret; 
  
  do {
    printf("%8s's move [(u)ndo/1-%i] :  ", playerName, Con::Moves());
    
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

EXTERNC BOOLEAN ValidTextInput(STRING input)
{
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

EXTERNC MOVE ConvertTextInputToMove(STRING input)
{
  return *input - 'a';
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

EXTERNC void PrintMove(MOVE theMove)
{
    printf("%c",'a'+theMove);
    return;
}

EXTERNC POSITION StringToPosition(char* board) {
    // FIXME: this is just a stub    
    return atoi(board);
}

EXTERNC char* PositionToString(POSITION pos) {
    // FIXME: this is just a stub
    return "Implement Me";
}

EXTERNC char * PositionToEndData(POSITION pos) {
	return NULL;
}

EXTERNC STRING MoveToString(MOVE theMove) {
    return "Implement MoveToString";
}
/************************************************************************
*************************************************************************
**         EVERYTHING BELOW THESE LINES IS LOCAL TO THIS FILE
*************************************************************************
************************************************************************/

STRING kDBName = "con" ;

EXTERNC
{
 
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
}

