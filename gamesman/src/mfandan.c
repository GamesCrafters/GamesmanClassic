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
** TODO LIST:
**              03/02/04 PrintPosition() needs to print info about how to make moves
**              03/02/04 copy from ValidTextInput(...) to do first parts of ConvertTextInputToMove(...)
**              03/02/04 Write DoMove(...)
**              03/02/04 add a call to generic_hash_init(...)
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

int      gNumberOfPositions  = 19683;  /* 3^9 */
int	 kBadPosition		= -1;

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
"The LEFT button puts an X or O (depending on whether you went first\n\
or second) on the spot the cursor was on when you clicked. The MIDDLE\n\
button does nothing, and the RIGHT button is the same as UNDO, in that\n\
it reverts back to your your most recent position.";

STRING   kHelpTextInterface    =
"On your turn, use the LEGEND to determine which number to choose (between\n\
1 and 9, with 1 at the upper left and 9 at the lower right) to correspond\n\
to the empty board position you desire and hit return. If at any point\n\
you have made a mistake, you can type u and hit return and the system will\n\
revert back to your most recent position.";

STRING   kHelpOnYourTurn =
"You place one of your pieces on one of the empty board positions.";

STRING   kHelpStandardObjective =
"To get three of your markers (either X or O) in a row, either\n\
horizontally, vertically, or diagonally. 3-in-a-row WINS.";

STRING   kHelpReverseObjective =
"To force your opponent into getting three of his markers (either X or\n\
O) in a row, either horizontally, vertically, or diagonally. 3-in-a-row\n\
LOSES.";

STRING   kHelpTieOccursWhen = /* Should follow 'A Tie occurs when... */
"the board fills up without either player getting three-in-a-row.";

STRING   kHelpExample =
"         ( 1 2 3 )           : - - -\n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - - - \n\
         ( 7 8 9 )           : - - - \n\n\
Computer's move              :  3    \n\n\
         ( 1 2 3 )           : - - X \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - - - \n\
         ( 7 8 9 )           : - - - \n\n\
     Dan's move [(u)ndo/1-9] : { 2 } \n\n\
         ( 1 2 3 )           : - O X \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - - - \n\
         ( 7 8 9 )           : - - - \n\n\
Computer's move              :  6    \n\n\
         ( 1 2 3 )           : - O X \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - - X \n\
         ( 7 8 9 )           : - - - \n\n\
     Dan's move [(u)ndo/1-9] : { 9 } \n\n\
         ( 1 2 3 )           : - O X \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - - X \n\
         ( 7 8 9 )           : - - O \n\n\
Computer's move              :  5    \n\n\
         ( 1 2 3 )           : - O X \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - X X \n\
         ( 7 8 9 )           : - - O \n\n\
     Dan's move [(u)ndo/1-9] : { 7 } \n\n\
         ( 1 2 3 )           : - O X \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - X X \n\
         ( 7 8 9 )           : O - O \n\n\
Computer's move              :  4    \n\n\
         ( 1 2 3 )           : - O X \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : X X X \n\
         ( 7 8 9 )           : O - O \n\n\
Computer wins. Nice try, Dan.";

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

#define BOARDSIZE     12           /* 4x3 board, must agree with the 2 definitions below */
#define BOARDHEIGHT    3           /* dimensions of the board */
#define BOARDWIDTH     4           /*  "               "      */
#define MAX_X          5           /* maximum number of pieces of X that is possible in a game */
#define MAX_O          5           /*  "            "            "                   "         */

typedef enum possibleBoardPieces {
	Blank, o, x
} BlankOX;

char *gBlankOXString[] = { '.', 'O', 'X' };
int myPieces_array[6] = { 'O', 0, MAX_O,          /* info about the game pieces' diff. types and possible number of them in a game */
                          'X', 0, MAX_X };        /* used to pass into generic_hash_init(...) */

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

InitializeGame()
{
}

FreeGame()
{}

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
    
    printf("\tc)\tWrite PPM to s(C)reen\n");
    printf("\ti)\tWrite PPM to f(I)le\n");
    printf("\ts)\tWrite Postscript to (S)creen\n");
    printf("\tf)\tWrite Postscript to (F)ile\n");
    printf("\n\n\tb)\t(B)ack = Return to previous activity.\n");
    printf("\n\nSelect an option: ");
    
    switch(GetMyChar()) {
    case 'Q': case 'q':
      ExitStageRight();
    case 'H': case 'h':
      HelpMenus();
      break;
    case 'C': case 'c': /* Write PPM to s(C)reen */
      tttppm(0,0);
      break;
    case 'I': case 'i': /* Write PPM to f(I)le */
      tttppm(0,1);
      break;
    case 'S': case 's': /* Write Postscript to (S)creen */
      tttppm(1,0);
      break;
    case 'F': case 'f': /* Write Postscript to (F)ile */
      tttppm(1,1);
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
** CALLS:       PositionToBlankOX(POSITION,*BlankOX)
**              BlankOX WhosTurn(*BlankOX)
**
************************************************************************/

POSITION DoMove(thePosition, theMove)
     POSITION thePosition;
     MOVE theMove;
{
  // fill me
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

GetInitialPosition()
{
  POSITION BlankOXToPosition();
  BlankOX theBlankOX[BOARDSIZE], whosTurn;
  signed char c;
  int i, goodInputs = 0;


  printf("\n\n\t----- Get Initial Position -----\n");
  printf("\n\tPlease input the position to begin with.\n");
  printf("\tNote that it should be in the following format:\n\n");
  printf("O - -\nO - -            <----- EXAMPLE \n- X X\n\n");

  i = 0;
  getchar();
  while(i < BOARDSIZE && (c = getchar()) != EOF) {
    if(c == 'x' || c == 'X')
      theBlankOX[i++] = x;
    else if(c == 'o' || c == 'O' || c == '0')
      theBlankOX[i++] = o;
    else if(c == '-')
      theBlankOX[i++] = Blank;
    else
      ;   /* do nothing */
  }

  /*
  getchar();
  printf("\nNow, whose turn is it? [O/X] : ");
  scanf("%c",&c);
  if(c == 'x' || c == 'X')
    whosTurn = x;
  else
    whosTurn = o;
    */

  return(BlankOXToPosition(theBlankOX,whosTurn));
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
  BOOLEAN ThreeInARow(), AllFilledIn();
  BlankOX theBlankOX[BOARDSIZE];

  PositionToBlankOX(position,theBlankOX);

  if( ThreeInARow(theBlankOX,0,1,2) || 
      ThreeInARow(theBlankOX,3,4,5) || 
      ThreeInARow(theBlankOX,6,7,8) || 
      ThreeInARow(theBlankOX,0,3,6) || 
      ThreeInARow(theBlankOX,1,4,7) || 
      ThreeInARow(theBlankOX,2,5,8) || 
      ThreeInARow(theBlankOX,0,4,8) || 
      ThreeInARow(theBlankOX,2,4,6) )
    return(gStandardGame ? lose : win);
  else if(AllFilledIn(theBlankOX))
    return(tie);
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

PrintPosition(position,playerName,usersTurn)
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
	      putchar(*gBlankOXString[(int)theBlankOx[j-1]]); /* HRS */
	    else if ((j-1) % 6 == 1)
	      putchar(' ');
	    else
	      putchar('-');
	  }
      }
    }

  
  puts(GetPrediction(position,playerName,usersTurn)); /* HRS */

	  
	
      


  /*
  printf("\n         ( 1 2 3 )           : %s %s %s\n",
	 gBlankOXString[(int)theBlankOx[0]],
	 gBlankOXString[(int)theBlankOx[1]],
	 gBlankOXString[(int)theBlankOx[2]] );
  printf("LEGEND:  ( 4 5 6 )  TOTAL:   : %s %s %s\n",
	 gBlankOXString[(int)theBlankOx[3]],
	 gBlankOXString[(int)theBlankOx[4]],
	 gBlankOXString[(int)theBlankOx[5]] );
  printf("         ( 7 8 9 )           : %s %s %s %s\n\n",
	 gBlankOXString[(int)theBlankOx[6]],
	 gBlankOXString[(int)theBlankOx[7]],
	 gBlankOXString[(int)theBlankOx[8]],
	 GetPrediction(position,playerName,usersTurn));
  */
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
  BlankOX theBlankOX[BOARDSIZE];
  int i;
  
  if (Primitive(position) == undecided) {
    PositionToBlankOX(position,theBlankOX);
    for(i = 0 ; i < BOARDSIZE ; i++) {
      if(theBlankOX[i] == Blank)
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
  char col;                           /* column */                                                              
  int row,dir,cap;                    /* row,direction,capture mode ('a','w','n')*/
  int scan_result =  sscanf(input,"%c%d-%d-%c",col,row,dir,cap);

  return
    (scan_result == 4) &&                 /* did we manage to get suitable vals for col,row,dir,cap? */
    isalpha(col) &&                       /* is the column a character? */
    isalpha(cap);                         /* is capture mode a character? */    
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
  return((MOVE) input[0] - '1'); /* user input is 1-9, our rep. is 0-8 */
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
BlankOX WhoseTurn()
{
  static BlankOX turn = x;
  if (turn == x) {
    turn = o;
    return x;
  }else {
    turn = x;
    return o;
  }
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
int GameSpecificTclInit(Tcl_Interp* interp,Tk_Window mainWindow) {}

