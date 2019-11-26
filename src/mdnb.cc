/************************************************************************
**
** NAME:        mdnb.cc
**
** DESCRIPTION: Dots and Boxes
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
#include <string.h>
#include <algorithm> // for max()

#ifndef NO_GRAPHICS
#include "tcl.h"
#include "tk.h"
#else
typedef int Tcl_Interp;
typedef int Tk_Window;
#endif

#define EXTERNC extern "C"

EXTERNC {
#include "gamesman.h"
#define max(a,b) (((a)>(b))?(a):(b))
}

POSITION gNumberOfPositions  =  0;
POSITION gInitialPosition    =  0;
POSITION kBadPosition        = (unsigned long long) -1; /* This can never be the rep. of a position */

STRING   kAuthorName         = "Brian Carnes";
STRING   kGameName           = "Dots and Boxes";
STRING   kDBName             = "dnb";
BOOLEAN  kPartizan           = TRUE;
BOOLEAN  kDebugMenu          = TRUE;
BOOLEAN  kGameSpecificMenu   = TRUE;
BOOLEAN  kTieIsPossible      = TRUE;
BOOLEAN  kLoopy               = FALSE;
BOOLEAN  kDebugDetermineValue = FALSE;

STRING   kHelpGraphicInterface = "No help";

STRING   kHelpTextInterface    = "No help";

STRING   kHelpOnYourTurn = "Players alternate turns connecting vertical or\n\
horizontal lines between dots. To draw a line, select the letter that\n\
corresponds to where you would like the line to be drawn.";

STRING   kHelpStandardObjective = "To create the most boxes.";

STRING   kHelpReverseObjective = "To force your opponent to create the most\n\
boxes.";

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

#ifndef NO_GRAPHICS
static int		ScoredCmd _ANSI_ARGS_((ClientData clientData,
			    Tcl_Interp *interp, int argc, char **argv));
/*
static int		GoAgainCmd _ANSI_ARGS_((ClientData clientData,
			    Tcl_Interp *interp, int argc, char **argv));
*/

#endif

#define MAX_X 3
#define MAX_Y 3

unsigned BoardSizeX = 2;
unsigned BoardSizeY = 2;
//bool SolvePrivately=false;
//bool CompressDatabase=false;
bool TrackBoxOwners=true;  // use extra bits to track who scored? only on small boards
bool ParallelMoves=false; /* Do all moves in parallel, vs. using gGoAgain */

POSITION staticMoves=0;

#define ISSET(POS,P) ((1<<(P))&(POS))

inline MOVE PositionToMove(int p) {
  if (ParallelMoves) return 1<<p;
  return p;
}

struct DNB {

  DNB(POSITION pos) : pos(pos){}
  operator POSITION() const {return pos;}

  int Turn() const {return ISSET(pos,TurnBit);}

  void DoMove(MOVE move) {
    DNB oldboard=*this;
    bool scored=false;

    if (ParallelMoves)
      pos |= move;
    else
      pos |= 1<<move;

    if (ParallelMoves) {
      for (unsigned j = 0; j < BoardSizeY; j++)
	for (unsigned i = 0; i < BoardSizeX; i++)
	  if (!oldboard.BoxCompleted(i,j) && BoxCompleted(i,j)) {
	    scored=true;
	    AddScore(i,j);
	  }
    } else {
      if (move < VertMoveOffset) {
	// horizontal move
	int i = move%BoardSizeX;
	int j = move/BoardSizeX;
	for (int c = 0; c < 2; c++, j--)
	  if (!oldboard.BoxCompleted(i,j) && BoxCompleted(i,j)) {
	    scored=true;
	    AddScore(i,j);
	  }	
      } else {
	// vertical move
	move -= VertMoveOffset;
	int j = move%BoardSizeY;
	int i = move/BoardSizeY;
	for (int c = 0; c < 2; c++, i--)
	  if (!oldboard.BoxCompleted(i,j) && BoxCompleted(i,j)) {
	    scored=true;
	    AddScore(i,j);
	  }	
      }
    }
    if (ParallelMoves || !scored)
      ToggleTurn();
  }

  void AddScore(int i, int j) {
    // someone just scored, record it
    if (!TrackBoxOwners) {
      if (!Turn()) {
	int first,second;
	GetScores(first,second);
	int mask = ((1<<BoxBits)-1)<<MoveBits;
	pos &= ~mask;
	pos |= ((first+1)<<MoveBits)&mask;
      }
      return;
    }
    if (Turn())
      pos |= 1<<(ScoreBit(i,j));
  }

  static int NumberOfPositions() {return 1<<(TurnBit+1);}
  static int Moves() {return MoveBits;}
  static int Boxes() {return BoxCount;}

  bool Scored(int i,int j) { // was this scored by 'O' (true) or 'X' (false)
    return Bit(ScoreBit(i,j));}

  int BoxCompleted(int x,int y) {
    return EdgeH(x,y)&&EdgeV(x,y)&&EdgeH(x,y+1)&&EdgeV(x+1,y);
  }

  void GetScores(int& first, int& second) {
    if (!TrackBoxOwners) {
      first=(pos&~(1<<TurnBit))>>MoveBits;
      second = BoxesCompleted()-first;
      return;
    }
    first=0,second=0;
    for (unsigned j=0; j < BoardSizeY; j++)
      for (unsigned i=0; i < BoardSizeX; i++) {
	if (BoxCompleted(i,j)) {
	  if (Scored(i,j))
	    second++;
	  else 
	    first++;
	}
      }
  }
  
  int BoxesCompleted() {
    int count=0;
    for (unsigned j=0; j < BoardSizeY; j++)
      for (unsigned i=0; i < BoardSizeX; i++)
	if (BoxCompleted(i,j))
	  count++;
    return count;
  }

  int MovesCompleted() {
    int count=0;
    for (int i = 0; i < MoveBits; i++)
      if (ISSET(pos,i)) count++;
    return count;
  }

  bool EdgeH(unsigned x,unsigned y) {
    if (x >= BoardSizeX) return false;
    if (y > BoardSizeY) return false;
    return Bit(BoardSizeX*y+x);
  }

  bool EdgeV(unsigned x,unsigned y) {
    if (x > BoardSizeX) return false;
    if (y >= BoardSizeY) return false;
    return Bit(VertMoveOffset+BoardSizeY*x+y);
  }

  char ChoiceH(int i, int j) {return 'a'+i+j*BoardSizeX;}
  char ChoiceV(int i, int j) {return 'a'+VertMoveOffset+i*BoardSizeY+j;}

  void Print() {
    for (unsigned j = 0; j <= BoardSizeY; j++)
    {
      for (unsigned i = 0; i < BoardSizeX; i++)
      {
        printf("+");
        if (EdgeH(i, j))
          printf("---");
        else
          printf(" %c ", ChoiceH(i, j));
      }
      if (j == BoardSizeY)
        break;
      printf("+\n");

      for (unsigned i = 0; i <= BoardSizeX; i++)
        printf("%c   ", EdgeV(i, j) ? '|' : ' ');
      printf("\n");

      for (unsigned i = 0; i <= BoardSizeX; i++)
      {
        printf("%c", EdgeV(i, j) ? '|' : ChoiceV(i, j));
        if (i == BoardSizeX)
          break;
        printf(" %c ", BoxCompleted(i, j) ? (!TrackBoxOwners ? '?' : Scored(i, j)) ? 'O' : 'X' : ' ');
      }
      printf("\n");

      for (unsigned i = 0; i <= BoardSizeX; i++)
        printf("%c   ", EdgeV(i, j) ? '|' : ' ');

      printf("\n");
    }
    
    int first,second;
    GetScores(first,second);
    printf("+     X has %i, O has %i  (%c's turn)\n ",first,second,Turn()?'O':'X');
    
  }

  static void PreCalc() {
    VertMoveOffset = BoardSizeX*(BoardSizeY+1);
    MoveBits = BoardSizeX*(BoardSizeY+1)+(BoardSizeX+1)*BoardSizeY;
    BoxCount = BoardSizeX*BoardSizeY;
    BoxBits = TrackBoxOwners?BoxCount:3;
    TurnBit = MoveBits+BoxBits;
    //printf("Using %i+%i+1 bits=\n",MoveBits,BoxBits,TurnBit);
  }

private:
  POSITION pos;
  void ToggleTurn() { pos ^= 1<<TurnBit;}

  bool Bit(int i) {return ISSET(pos,i);}
  static int BoxCount;
  static int BoxBits;
  static int MoveBits;
  static int TurnBit;
  static int VertMoveOffset;

  static int ScoreBit(int i,int j) { return MoveBits+i+j*BoardSizeX;}

};

int DNB::VertMoveOffset=0;
int DNB::BoxCount=0;
int DNB::BoxBits=0;
int DNB::MoveBits=0;
int DNB::TurnBit=0;

BOOLEAN DNBGoAgain(POSITION pos,MOVE move) {
  DNB oldpos(pos);
  DNB newpos(pos);
  newpos.DoMove(move);
  return oldpos.BoxesCompleted()!=newpos.BoxesCompleted();
}

/*
// ************************ PRIVATE SOLVE ********************

// the position stored in the database is the score one could expect
// from the remaining unscored boxes in this position, going first


unsigned char*gDatabaseTight=NULL;

signed char *privateDatabase=NULL;
int privateMask=0;

int findPrivateValue(POSITION pos) {

  // clean high bits
  // turn==0
  // all scorebits==0
  pos &= privateMask;

  if (privateDatabase[pos] != -99)
    return privateDatabase[pos];

  DNB oldboard(pos);
  int oldscore=oldboard.BoxesCompleted();

  int best = -99;

  for (int m = 0; m < DNB::Moves(); m++) {
    if (ISSET(pos,m))
      continue;
    
    DNB board(pos);
    board.DoMove(m);    
    
    //    int value = findPrivateValue(board);
    int value = privateDatabase[board&privateMask];
    
    if (board.Turn()) // other guy goes next
      value = -value; // so this is his score
    else
      value += board.BoxesCompleted()-oldscore; // we get this score

    best = max(value,best);
  }

  if (best==-99) // no moves
    best=0; // nobody gets anything

  return privateDatabase[pos] = best;  
}

void DoPrivateSolve() {
  int privateposcount=1<<DNB::Moves();
  privateMask=privateposcount-1;  
  privateDatabase = (signed char*)SafeMalloc(privateposcount);
  memset(privateDatabase,-99,privateposcount);

  char fname[80];
  sprintf(fname,"dnb%ix%i",BoardSizeX,BoardSizeY);
  FILE*f=fopen(fname,"rb");
  if (f) {
    printf("Remembering Private Solution from last time\n");
    fread(privateDatabase,privateposcount,1,f);
    fclose(f);
    return;
  }
  printf("Solving privately...\n");
  for (int i = privateMask; i >= 0; i--) {
    if (!(i&0xfff))
      printf("%i%% completed",100-int(i*100.0/privateMask));
    findPrivateValue(i);
  }
  printf("Done\n");
  f=fopen(fname,"wb");
  if (f) {
    fwrite(privateDatabase,privateposcount,1,f);
    fclose(f);
  }
}
*/

POSITION ActualNumberOfPositions(int variant);

/************************************************************************
**
** NAME:        InitializeGame
**
** DESCRIPTION: Initialize the game
** 
************************************************************************/

EXTERNC void InitializeGame()
{
  if (BoardSizeX*BoardSizeY>=6) {
    TrackBoxOwners=false;
    //CompressDatabase=true;
    //SolvePrivately=true;
  }

  DNB::PreCalc();

  /*
  gInitialPosition = (1<<BoardSizeX)-1;
  gInitialPosition |= 1<<(BoardSizeX*(BoardSizeY+1));
  gInitialPosition |= 1<<(BoardSizeX*(BoardSizeY+2));
  */

  gNumberOfPositions = DNB::NumberOfPositions();

  /*
  if (SolvePrivately) {
    DoPrivateSolve();
  } else if (CompressDatabase) {
    gDatabaseTight = 
      (unsigned char*) SafeMalloc (gNumberOfPositions);
    for(i = 0; i < gNumberOfPositions; i++)
      gDatabaseTight[i] = undecided;
  } else {
    gDatabase = (VALUE *) SafeMalloc (gNumberOfPositions * sizeof(VALUE));
    for(i = 0; i < gNumberOfPositions; i++)
      gDatabase[i] = undecided;
  }
  */

  if (!ParallelMoves) {
    if (!&gGoAgain)
      ParallelMoves=true;
    else
      gGoAgain=DNBGoAgain;
  }
  
  gActualNumberOfPositionsOptFunPtr = &ActualNumberOfPositions;
}

#ifndef NO_GRAPHICS

EXTERNC int GameSpecificTclInit(Tcl_Interp* interp,Tk_Window mainWindow)
{
  Tcl_CreateCommand(interp, "C_Scored", (Tcl_CmdProc*) ScoredCmd, (ClientData) mainWindow,
		    (Tcl_CmdDeleteProc*) NULL);
  /* Tcl_CreateCommand(interp, "C_GoAgain", (Tcl_CmdProc*) GoAgainCmd, (ClientData) mainWindow,
     (Tcl_CmdDeleteProc*) NULL); */
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

EXTERNC void GameSpecificMenu() {
  do { printf("Enter X size (1-%d): ", MAX_X);
  } while (scanf("%i",&BoardSizeX)!=1 || unsigned(BoardSizeX-1)>=MAX_X);
  do { printf("Enter Y size (1-%d): ", MAX_Y);
  } while (scanf("%i",&BoardSizeY)!=1 || unsigned(BoardSizeY-1)>=MAX_Y);

 }

// Anoto pen support - implemented in core/pen/pdnb.c
EXTERNC void gPenHandleTclMessage(int options[], char *filename, Tcl_Interp *tclInterp, int debug);

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
  // Anoto pen support
  if ((gPenFile != NULL) && (gTclInterp != NULL)) {
      gPenHandleTclMessage(theOptions, gPenFile, gTclInterp, gPenDebug);
  }  
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
  DNB board(thePosition);
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

EXTERNC VALUE Primitive(POSITION position) 
{
  DNB board(position);

  int first,second;
  board.GetScores(first,second);

  if (first+second<board.Boxes()) return undecided;
  if (first==second) return tie;
  if (board.Turn())
    return ((first>second) ^ !gStandardGame)?lose:win; // 2nd person just lost/won
  else
    return ((first<second) ^ !gStandardGame)?lose:win; // 1st person...
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
  DNB board(position);
  
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
  int i;

  if (Primitive(position) != undecided)
    return NULL;

  for(i = 0 ; i < DNB::Moves() ; i++) {
    MOVELIST *x,*p;
    int newpos;
    int move = PositionToMove(i);
    if(ISSET(position,i)) 
      continue;

    newpos = DoMove(position,move);
    DNB oldboard(position);
    DNB newboard(newpos);
    if (!ParallelMoves ||
	oldboard.BoxesCompleted() == newboard.BoxesCompleted() || 
	!(x = GenerateMoves(newpos))) {
      head = CreateMovelistNode(move,head);
      continue;
    }

    /* go again, because we scored */
    // be careful not to enter duplicate moves "a->c->d" vs "c->a->d"
    for (p=x;p;p=p->next) {
      MOVE combomove = p->move|move;
      MOVELIST*p2;
      for (p2=head;p2;p2=p2->next)
	if (p2->move==combomove) break;
      if (!p2)
	head=CreateMovelistNode(combomove,head);
    }
    FreeMoveList(x);
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
    printf("%8s's move [(u)ndo/1-%i] :  ", playerName, DNB::Moves());
    
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
  int move=0;
  while (*input)
    move |= PositionToMove(*input++ - 'a');
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

EXTERNC void PrintMove(MOVE theMove)
{
  if (!ParallelMoves) {
    printf("%c",theMove+'a');
    return;
  }
    
  int i;
  for (i = 0; i < DNB::Moves(); i++)
    if (ISSET(theMove,i))
	printf("%c", i+'a'); 
}

EXTERNC POSITION StringToPosition(char* board) {
  // e.g. board == "o010000000000----"
  //   position == 0b10000000000000010
  //                 ^ turn
  //                  ^ 4 owners (<--)
  //                      ^ verticals (<--)
  //                            ^ horizontals (<--)

  POSITION pos = 0;

  unsigned horizontals = BoardSizeX * (BoardSizeY + 1);
  unsigned verticals = (BoardSizeX + 1) * BoardSizeY;
  unsigned owners = BoardSizeX * BoardSizeY;

  unsigned count = 0;

  // FIXME: These for loops can be optimized

  pos |= (board[count] == 'o') << (horizontals + verticals + owners);
  count++;

  for (unsigned j = 0; j <= BoardSizeY; j++) {
    for (unsigned i = 0; i < BoardSizeX; i++) {
      pos |= (board[count] == '1') << (count - 1);
      count++;
    }
  }

  for (unsigned i = 0; i <= BoardSizeX; i++) {
    for (unsigned j = 0; j < BoardSizeY; j++) {
      pos |= (board[count] == '1') << (count - 1);
      count++;
    }
  }

  for (unsigned j = 0; j < BoardSizeY; j++) {
    for (unsigned i = 0; i < BoardSizeX; i++) {
      pos |= (board[count] == 'o' ? 1 : 0) << (count - 1);
      count++;
    }
  }

  return pos;
}

EXTERNC char* PositionToString(POSITION pos) {
  DNB board(pos);

  unsigned horizontals = BoardSizeX * (BoardSizeY + 1);
  unsigned verticals = (BoardSizeX + 1) * BoardSizeY;
  unsigned owners = BoardSizeX * BoardSizeY;

  char *ret = (char *) SafeMalloc(sizeof(char) * (horizontals + verticals + owners + 1 + 1));
  
  unsigned count = 0;

  ret[count++] = (board.Turn() != 0) ? 'o' : 'x';

  for (unsigned j = 0; j <= BoardSizeY; j++) {
    for (unsigned i = 0; i < BoardSizeX; i++) {
      ret[count++] = board.EdgeH(i, j) ? '1' : '0';
    }
  }

  for (unsigned i = 0; i <= BoardSizeX; i++) {
    for (unsigned j = 0; j < BoardSizeY; j++) {
      ret[count++] = board.EdgeV(i, j) ? '1' : '0';
    }
  }

  for (unsigned j = 0; j < BoardSizeY; j++) {
    for (unsigned i = 0; i < BoardSizeX; i++) {
      ret[count++] = board.BoxCompleted(i, j) ? (!TrackBoxOwners ? '?' : board.Scored(i, j)) ? 'o' : 'x' : '-';
    }
  }

  ret[count++] = '\0';

  return ret;
}

EXTERNC STRING MoveToString(MOVE theMove) {
  char *ret = (char *) SafeMalloc(sizeof(char) * (8));
  snprintf(ret, 8, "%d", theMove);
  return ret;
}

EXTERNC char * PositionToEndData(POSITION pos) {
	return NULL;
}
/*
EXTERNC VALUE *GetRawValueFromDatabase(POSITION position)
{
  static POSITION lastpos=-1;
  static VALUE value;

  if (SolvePrivately) {
    DNB board(position);
    int first,second,score;
    board.GetScores(first,second);

    if (board.Turn())
      score=second-first;
    else
      score=first-second;
    score += privateDatabase[position&privateMask];
    value=(score==0)?tie:(score>0)?win:lose;
    // fake remoteness for GetValueEquivalentMoves
    int remoteness = board.Moves()-board.MovesCompleted();
    value = VALUE(value|(remoteness<<REMOTENESS_SHIFT));
    return &value;
} else if (CompressDatabase) {
    // should just change his stuff to use bytes, but do it this way minimize changes in gamesman
    // this'll slow us down a lot, but hey, it's already a function call
    // very poor, he slams stuff in above his enumeration, so no -fshort-enums
    // he uses bits 0,1 for value, bit 2 for visited, and the rest for remoteness
    if (lastpos!= -1)
      gDatabaseTight[lastpos]=value;
    lastpos=position;
    value=(VALUE)gDatabaseTight[lastpos];
    return &value;
  }
  return(&gDatabase[position]);
}
*/

/*** Database ***/

EXTERNC int NumberOfOptions() {
  return MAX_X * MAX_Y * 2;
}

EXTERNC int getOption() {
  int option = 1;
  option += gStandardGame ? 0 : 1;
  option += 2 * (BoardSizeX - 1);
  option += 2 * MAX_X * (BoardSizeY - 1);
  return option;
}

EXTERNC void setOption(int option) {
  option--;
  gStandardGame = (option%2==0);
  BoardSizeX = option/2 % MAX_X + 1;
  BoardSizeY = option/(2*MAX_X) % MAX_Y + 1;
}


POSITION ActualNumberOfPositions(int variant) {
  switch (variant) {
  case 1:
  case 2:
    return 2;
    break;
  case 3:
  case 4:
    return 9;
    break;
  case 5:
  case 6:
    return 51;
    break;
  case 7:
  case 8:
    return 20;
    break;
  case 9:
  case 10:
    return 602;
    break;
  case 11:
  case 12:
    return 9173;
    break;
  case 13:
  case 14:
    return 194;
    break;
  case 15:
  case 16:
    return 16391;
    break;
  case 17:
  case 18:
    return 1484294;
    break;
  default:
    return gNumberOfPositions;
  }
  return (POSITION) -1;
}


/************************************************************************
*************************************************************************
**         EVERYTHING BELOW THESE LINES IS LOCAL TO THIS FILE
*************************************************************************
************************************************************************/

#ifndef NO_GRAPHICS

static int
ScoredCmd(ClientData dummy,Tcl_Interp *interp, int argc, char** argv)
{
  int position,x,y;

  if (argc != 4) {
    Tcl_SetResult(interp,  "wrong # args: Scored (int)Position (int)x (int)y", TCL_STATIC);
    return TCL_ERROR;
  }
  else {
    if(Tcl_GetInt(interp, argv[1], &position) != TCL_OK)
      return TCL_ERROR;
    if(Tcl_GetInt(interp, argv[2], &x) != TCL_OK)
      return TCL_ERROR;
    if(Tcl_GetInt(interp, argv[3], &y) != TCL_OK)
      return TCL_ERROR;

    DNB board(position);
    
    Tcl_SetResult(interp, StrFromI(board.BoxCompleted(x,y)?  (!TrackBoxOwners)?2:board.Scored(x,y)?1:0:-1), SafeFreeString);
    return TCL_OK;
  }
}

/*
static int
GoAgainCmd(ClientData dummy, Tcl_Interp* interp, int argc, char ** argv)
{     
  int position, move;
  POSITION DoMove();
    
  if (argc != 3) {
    Tcl_SetResult(interp,  "wrong # args: GoAgain (int)Position (int)Move", TCL_STATIC);
    return TCL_ERROR;
  }
  else {
    if(Tcl_GetInt(interp, argv[1], &position) != TCL_OK)
      return TCL_ERROR;
    if(Tcl_GetInt(interp, argv[2], &move) != TCL_OK)
      return TCL_ERROR;
    
    Tcl_SetResult(interp, StrFromI(DNBGoAgain(position,move)), SafeFreeString);
    return TCL_OK;
  }
}
*/

#endif

