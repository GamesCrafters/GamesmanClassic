/************************************************************************
**
** NAME:        mjoust.c
**
** DESCRIPTION: Joust for computers
**
** AUTHOR:      Dave and Isaac  -  University of California at Berkeley
**              Copyright (C) D & I productions, 1995. All rights reserved.
**
** DATE:        08/28/91
**
** UPDATE HIST:
**
**  8-30-91 1.0a1 : Fixed the bug in reading the input - now 'q' doesn't barf.
**  9-06-91 1.0a2 : Added the two extra arguments to PrintPosition
**                  Recoded the way to do "visited" - bitmask
**  9-06-91 1.0a3 : Added Symmetry code - whew was that a lot of code!
**  9-06-91 1.0a4 : Added ability to have random linked list in gNextMove.
**  9-06-91 1.0a5 : Removed redundant code - replaced w/GetRawValueFromDatabase
**  9-17-91 1.0a7 : Added graphics code.
**  5-12-92 1.0a8 : Added Static Evaluation - it's far from perfect, but 
**                  it works!
** 05-15-95 1.0   : Final release code for M.S.
** 97-05-12 1.1   : Removed gNextMove and any storage of computer's move
**
** Decided to check out how much space was wasted with the array:
**
** Without checking for symmetries
**
** Evaluating the value of Tic-Tac-Toe...done in 1.434998 seconds!
** Undecided = 14878 out of 19682
** Lose      =  1304 out of 19682
** Win       =  2495 out of 19682
** Tie       =  1005 out of 19682
** Unk       =     0 out of 19682
** TOTAL     =  4804 out of 19682
**
** Using the new evaluation method:
**
** Undecided = 14205 out of 19683
** Lose      =  1574 out of 19683
** Win       =  2836 out of 19683
** Tie       =  1068 out of 19683
** Unk       =     0 out of 19683
** TOTAL     =  5478 out of 19683
**
** While checking for symmetries and storing a canonical elt from them.
**
** Evaluating the value of Tic-Tac-Toe...done in 5.343184 seconds!
** Undecided = 18917 out of 19682
** Lose      =   224 out of 19682
** Win       =   390 out of 19682
** Tie       =   151 out of 19682
** Unk       =     0 out of 19682
** TOTAL     =   765 out of 19682
**
**     Time Loss : 3.723
** Space Savings : 6.279
**
**************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/
#include <stdio.h>
#include "gsolve.h"
/* 10/12 Isaac: this is new */
#include <math.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
/* end new include */

extern STRING gValueString[];

int gNumberOfPositions = 3932160; //99614720;  /* (20 * 19 * 2^18) */

POSITION gInitialPosition    = 16778208; //Blank with pieces in the corners for 4x4
POSITION kBadPosition        = -1; /* This can never be the rep. of a position */

STRING   kGameName           = "Joust";
BOOLEAN  kPartizan           = TRUE;
BOOLEAN  kSupportsHeuristic  = TRUE;
BOOLEAN  kSupportsSymmetries = FALSE;
BOOLEAN  kSupportsGraphics   = TRUE;
BOOLEAN  kDebugMenu          = TRUE;
BOOLEAN  kGameSpecificMenu   = TRUE; //We use this to set the board
BOOLEAN  kTieIsPossible      = FALSE; /* Isaac 10/24: changed to 
                                       * FALSE */
BOOLEAN  kLoopy               = FALSE;
BOOLEAN  kDebugDetermineValue = FALSE;

STRING   kHelpGraphicInterface =
"The Left button puts an X or O (depending on whether you went first\n\
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
unsigned int BOARDSIZE  =   16;          /* ROWSIZE x COLUMNSIZE board. */
unsigned int ROWSIZE    =   4;           /* Rows on the board. */
unsigned int COLUMNSIZE =   4;           /* Columns on the board. */          
unsigned int BITSIZE    =   4;           /* The number of bits needed to 
                                   * store the largest square number 
                                   * on the board. EX: For a 20 
                                   * square board, you need 4 bits 
                                   * because 20 < 2^4 */
unsigned int BITSIZEMASK =  0xf;         /* Number to mask BITSIZE number 
                                   * of bits */
BOOLEAN kPrintDebug = FALSE;
BOOLEAN kDoMoveDebug = FALSE;
typedef enum possibleBoardPieces { /* O are black pieces, X are white pieces */
	Blank, Burnt, o, x
} BlankBurntOX;

BOOLEAN gBurnMove = TRUE;
BOOLEAN doubleTrouble = FALSE;
int burnType = 0;                /* 0 is for the burnt bridges ver.,
				  * 1 is for the arbitrary grenade 
                                  * 2 is for piece-specific version 
                                  * (Note: analysis of burn AFTER 
                                  * move would be much larger) */
/* Prints an example based on the current Row and Column Sizes */
void PrintExample();

/* Checks if a player can move. */
BOOLEAN CanMove(POSITION);           
   /* helper functions for CanMove */
BOOLEAN CanKnightMove(BlankBurntOX*, int, int);
BOOLEAN CanRookMove(BlankBurntOX*, int, int);
BOOLEAN CanBishopMove(BlankBurntOX*, int, int);

/* pulls the current piece location from the position */
unsigned int GetPFromPosition(POSITION);
unsigned int GetXFromPosition(POSITION);
unsigned int GetOFromPosition(POSITION); 

/* For game specific options */
void PieceMessage(POSITION, BOOLEAN);
void ChangeBoardSize();
void ChangePieces();
void ChangePiece(POSITION);
void ChangeBurn();
void ChangeBoardDimensions();
int GenerateNewInitial();
void ChangeOrder();

char *gBlankBurntOXString[] = { "-", "*", "O", "X" };
STRING kBBExplanation =
"\nIn Burned Bridges, the spots you touch
are burned after you leave them.
There's no turning back!\n";
STRING kUGExplanation =
"\nIn Ultimate Grenade, you have the ability 
to burn ANY spot on the board. Pretty good
arm you've got there!\n";
STRING kBMExplanation =
"\nWith Piece-specific burning, you can
only lob grenades to places your piece
could move. Don't discount the knight
too quickly: he can lob grenades OVER
other burns...\n";

/*************************************************************************
**
** Here we declare the global database variables
**
**************************************************************************/

VALUE     *gDatabase;
typedef enum Pieces {knight, bishop, rook, queen, king} Piecetype;
Piecetype Xtype = knight;
Piecetype Otype = knight;
Piecetype XBurn = knight;
Piecetype OBurn = knight;
Piecetype thePiece;
Piecetype theBurn;

/************************************************************************
**
** NAME:        InitializeDatabases
**
** DESCRIPTION: Initialize the gDatabase, a global variable.
** 
************************************************************************/

InitializeDatabases()
{
  GENERIC_PTR SafeMalloc();
  int i;

  //gDatabase = (VALUE *) SafeMalloc (20 * 19 * pow(2, 18) * sizeof(VALUE));
  gDatabase = (VALUE *) SafeMalloc (203954088 * sizeof(VALUE));
  
  for(i = 0; i < 203954088; i++)
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

GameSpecificMenu() {
  printf("Welcome to Dave and Isaac's user-friendly variant chooser!\n");
  printf("type B to set up the board or,\n");
  printf("type O to change the order of moving/burning,\n");
  printf("type P to choose a new piece,\n");
  printf("type S to change the size of the Board, or\n");
  printf("type T Select a different style of burning\n");
 
 switch(GetMyChar()) {
 case 'B': case 'b':
      GetInitialPosition();
      break;
 case 'o': case 'O':
   ChangeOrder();
   break;
 case 'p': case 'P':
   ChangePieces();
   break;
 case 's': case 'S':
   ChangeBoardSize();
   break;
 case 't': case 'T':
   ChangeBurn();
   break;
   //our "hidden" debug command
 case 'd':
   kPrintDebug = !kPrintDebug;
   printf("Debugging on/off");
   if(kPrintDebug)
    {
      printf("Debug DoMove also? (0 or 1)");
      kDoMoveDebug = GetMyChar();
    }
   break;
 default:
   printf("Thank you, come again!");
   break;
 }
}

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

/* 11/4 Isaac: Mask off the position's turn bit that has been preserved
   * up till now, then put in the opposite of the current turn bit */
POSITION ChangeTurn(POSITION theNewPos, POSITION thePos){
  return ((theNewPos & ~1) | ((thePos + 1) & 1));
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
** CALLS:       PositionToBlankBurntOX(POSITION,*BlankBurntOX)
**              BlankBurntOX WhoseTurn(*BlankBurntOX)
**
************************************************************************/

/* Dave: 10/20.  theMove contains the move (1-20) in the last 5 bits,
 * and the square to be burnt in the first 5 bits. */

POSITION DoMove(thePosition, theMove)
     POSITION thePosition;
     MOVE theMove;
{
  BlankBurntOX  WhoseTurn();

  /*Dave's Note: the number 1f base 16 == 11111 base 2, for masking all but 5 bits */

  /*Dave: Assumes piece move is in the last 5 bits of theMove,
   * and the burnt spot is in the first 5 bits. */
  MOVE extractedMove = ExtractMove(theMove);           //Mask off all but 5 move bits
  MOVE extractedBurnt = ExtractBurn(theMove);          //Mask off all but 5 burn bits.
  MOVE oldSpot;                                        //Remembers the piece's old spot.
  POSITION newPosition = 0;                            //11/4 Isaac: initialized to 0.

  if (kPrintDebug && kDoMoveDebug)
	printf("\nOld 0x%8x\n", thePosition >> (BITSIZE*2+1));
  /* We're taking it easy; masking all but the bits we're actually replacing at 
   * any given point*/
  
  // Set X/O's new position.
  if(WhoseTurn(thePosition) == x){        //Set x, move past the turn and o bits
    oldSpot = GetXFromPosition(thePosition);
      // the old way (thePosition & (BITSIZEMASK << (BITSIZE+1)) >> (BITSIZE+1));
    newPosition = ((thePosition & ~(BITSIZEMASK << (BITSIZE+1))) |
		   (extractedMove << (BITSIZE+1)));
                  //NOTE: this saves the original pos turn bit
    newPosition = newPosition | 1 << ((BITSIZE*2+1) + extractedMove);
  }
  else if(WhoseTurn(thePosition) == o){   //Set o, just move past the turn bit
    oldSpot = GetOFromPosition(thePosition);
      // the old way (thePosition & (BITSIZEMASK << 1) >> 1);
    newPosition = ((thePosition & ~(BITSIZEMASK << 1)) | (extractedMove << 1));
                  //NOTE: this saves the original pos turn bit
    newPosition = newPosition | 1 << ((BITSIZE*2+1) + extractedMove);
  }
  else
    BadElse("Must be either x or o's turn.");

  if (kPrintDebug && kDoMoveDebug)
    printf("Move0x%8x\n", newPosition >> (BITSIZE*2+1));
  /* Set the new mapping of burned/occupied spots on the board, set 
   * the new spot and the grenaded spot(s) as specified by the variant.*/
  if (!doubleTrouble)
    {
      newPosition = newPosition & ~(1 << ((BITSIZE*2+1) + oldSpot));
      // clear old spot if not burning it
      if (kPrintDebug && kDoMoveDebug)
	printf("Mask0x%8x\nOff 0x%8x\n",  ~(1 << oldSpot) >> (BITSIZE*2+1),
	       newPosition >> (BITSIZE*2+1));
    }
  /* burn the given spot (without making any judgements;
   * DoMove simply trusts the data given */
  // burns the same "oldspot" in Burnt Bridges
      newPosition = newPosition | (1 << ((BITSIZE*2+1) + extractedBurnt));
      if (kPrintDebug && kDoMoveDebug)
	printf("Burn0x%8x\n", newPosition >> (BITSIZE*2+1));
  
  
  //newPosition = ChangeTurn(newPosition, thePosition); 
  newPosition = (newPosition & ~1) | ((thePosition + 1) & 1);
  
  if (kPrintDebug && kDoMoveDebug)
    {
      printf("New 0x%8x\n", newPosition >> (BITSIZE*2+1)); 
      printf("\nDoMove called\n"); // for debugging
    }
  return(newPosition);
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
  POSITION BlankBurntOXToPosition();
  BlankBurntOX theBlankBurntOX[BOARDSIZE]; // whosTurn
  signed char c;
  int i, goodInputs = 0, xCount = 0, oCount = 0;


  printf("\n\n\t----- Get Initial Position -----\n");
  printf("\n\tPlease input the position to begin with.\n");
  printf("\tNote that it should be in the following format:\n\n");
  PrintExample();
  /*printf("O - - - -\n- * - - -           <----- EXAMPLE \n- * - - X \n- - - - -\n\n");*/

  i = 0;
  getchar();
  while(i < BOARDSIZE && (c = getchar()) != EOF) {
    if((c == 'x' || c == 'X') && xCount < 1)
      {
	theBlankBurntOX[i++] = x;
	xCount = xCount + 1;
      }
    else if((c == 'o' || c == 'O' || c == '0') && oCount < 1)
      {
      theBlankBurntOX[i++] = o;
      oCount = oCount + 1;
      }
    else if(c == '-')
      theBlankBurntOX[i++] = Blank;
    else if(c == '*')
      theBlankBurntOX[i++] = Burnt;
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

  gInitialPosition = BlankBurntOXToPosition(theBlankBurntOX);
  return(gInitialPosition);
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
** CALLS:       POSITION GetCanonicalPosition (POSITION)
**              MOVE     DecodeMove (POSITION,POSITION,MOVE)
**
************************************************************************/

MOVE GetComputersMove(thePosition)
     POSITION thePosition;
{
  POSITION canPosition;
  MOVE theMove;
  int i, randomMove, numberMoves = 0;
  MOVELIST *ptr, *head, *GetValueEquivalentMoves();
  
  if(gPossibleMoves) 
    printf("%s could equivalently choose [ ", gPlayerName[kComputersTurn]);
  head = ptr = GetValueEquivalentMoves(thePosition);
  if(kPrintDebug)
    printf("Generate moves done"); //for debugging

  while(ptr != NULL) {
    numberMoves++;
    if(gPossibleMoves) 
      printf("%d ",ptr->move+1);
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
** NAME:        ExtractMove
**
** DESCRIPTION: Get a move from something in theMove format.
**
** INPUTS:      MOVE theMove
**
************************************************************************/
int ExtractMove(theMove)
     MOVE theMove;
{
  return (theMove >> BITSIZE);
}

/************************************************************************
**
** NAME:        ExtractBurn
**
** DESCRIPTION: Get a burn from something in theMove format.
** 
** INPUTS:      MOVE theMove
**
************************************************************************/
int ExtractBurn(theMove)
     MOVE theMove;
{
  return (theMove & BITSIZEMASK);
}

/************************************************************************
**
** NAME:        EncodeTheMove
**
** DESCRIPTION: Put a move and a burn into theMove format.
** 
** INPUTS:      int theMove, int theBurn
**
************************************************************************/
MOVE EncodeTheMove(theMove, theBurn)
     MOVE theMove;
     MOVE theBurn;
{
  return (theBurn | (theMove << BITSIZE));
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

PrintComputersMove(computersMove, computersName)
     MOVE computersMove;
     STRING computersName;
{
  
  printf("%8s's move              : %2d  %2d\n", computersName, ExtractMove(computersMove) + 1, ExtractBurn(computersMove) + 1); //Dave: added 11114
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
  BOOLEAN CanMove();
  BOOLEAN CanBurn();
  /* if(CanMove(position))
    {
      return(undecided);
    }
  else
    {
    return(gStandardGame ? lose : win);
    }*/
  if (kPrintDebug)
  printf("\nPrimitive called"); // for debugging
  
  if(!(CanMove(position) && CanBurn(position)))
   return(gStandardGame ? lose : win);
  //Isaac 11/30 need to check if you can burn as well as move
 else
   return(undecided);
}

/**********************************************
 * PrintRows					       
 * Whereas before rows were manually printed, we want to print a 
 * more flexible board. 
 * printf("         ( 11 12 13 14 15 )           : %s  %s  %s  %s\n",
 * PrintRows only need to be passed the current board, it uses the 
 * global dimensions of the board to determine how it prints.
 **********************************************/




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
  int i, j, spot;
  STRING GetPrediction();
  VALUE GetValueOfPosition();
  BlankBurntOX theBlankBurntOX[BOARDSIZE];
  BlankBurntOX WhoseTurn();

  PositionToBlankBurntOX(position, theBlankBurntOX);

  /* shiny, new, flexible way */
  for(i=0; i < ROWSIZE; i++)
    {
      printf("         (");
      for(j=0; j < COLUMNSIZE; j++)
	{
	  spot = (j + i * COLUMNSIZE) + 1;
	  if (spot < 10)
	    printf("  %d", spot);
	  else
	    printf(" %d", spot);
	}
      
      if (i == 0)
	{ printf(" )   TOTAL   :");}
      else
	{ printf(" )           :");}
      
      for(j=0; j < COLUMNSIZE; j++)
	{
	  printf(" %s", gBlankBurntOXString[(int)theBlankBurntOX[(i*COLUMNSIZE) + j]]);
	}
      if (i == (ROWSIZE - 1))
	printf(" %s", GetPrediction(position,playerName,usersTurn));
      printf("\n");
    }
  printf("%s's move  (as a ", gBlankBurntOXString[(int)WhoseTurn(position)]);
  PieceMessage(position, TRUE);
  printf(", burning");
  if(!gBurnMove || burnType == 0)
    printf(" AFTER moving");
  else
    printf(" BEFORE moving");
  printf(" as a ");
  PieceMessage(position, FALSE);
  printf(")");
  printf("\n");

  if (kPrintDebug)
    printf("Position = %d\n", position);
}

/************************************************************************
** NAME:        PossibleMove
**
** DESCRIPTION: This is the big one: PossibleMove checks the given 
**              move in the array and decides whether this move is
**              blocked or not based on the current position. By 
**              moving outward from the position it saves searches.  
** 
** INPUTS:      rp: the row of the position,
**              cp: the column of the Position,
**              rd: the row of the proposed move,
**              cd: the column of the proposed move.
**              2DBlankBurntOX: the current state of the board in a 
**                              2D array
**
** OUTPUTS:  true or false
**
** CALLS:    PossibleRook, PossibleBishop, PossibleQueen, PossibleKnight,
**           PossibleKing
**
************************************************************************/
/*******************
 * Isaac 10/18: For use with GenerateMoves
 * *****************
 * 
 *******************/

BOOLEAN PossibleMove(int theDest, POSITION thePos, BlankBurntOX theBlankBurntOX[])
{
  int playerPos;
  int rp;
  int cp;
  int rd;
  int cd;
  int i;
  int j;
  
  BlankBurntOX  WhoseTurn();
  

  playerPos = GetPFromPosition(thePos);
  
  /* Set the PieceType we'll be working
     with */
  if (WhoseTurn(thePos) == x)
    {
      thePiece = Xtype;
    }
  else
    {
      thePiece = Otype;
    }

  for(i=0; i < ROWSIZE; i++){    //Get the player's location in 2D
    for(j=0; j < COLUMNSIZE; j++){
      if((i*COLUMNSIZE) + j == playerPos){
	rp = i;
	cp = j;
      }
    }
  }
    

  for(i=0; i < ROWSIZE; i++){    //Get theDest's location in 2D
    for(j=0; j < COLUMNSIZE; j++){
      if((i*COLUMNSIZE) + j == theDest){
	rd = i;
	cd = j;
      }
    }
  }
  
  if(rd > (ROWSIZE-1) || cd > (COLUMNSIZE-1) || rd < 0 || cd < 0)
    {
      printf("Off the board");
      return FALSE;
    }
  else
    {
      switch (thePiece) {
      case knight:
	return(PossibleKnight(rp, cp, rd, cd, thePos, theBlankBurntOX));
	break;
      case queen:
	return(PossibleQueen(rp, cp, rd, cd, thePos, theBlankBurntOX));
	break;
      case rook:
	return(PossibleRook(rp, cp, rd, cd, thePos, theBlankBurntOX));
	break;
      case bishop:
	return(PossibleBishop(rp, cp, rd, cd, thePos, theBlankBurntOX));
    break;
      case king:
	return(PossibleKing(rp, cp, rd, cd, thePos, theBlankBurntOX));
	break;
      default:
	BadElse("Error in PossibleMove\n");
	break;
      }
    }
  
}

/*So many possibilities...
    2     cp     1
        <-||->
    ^^^^<-||->^^^^        
  rp----thePos-----
    vvvv<-||->vvvv
        <-||->  
    3            4
*/
BOOLEAN PossibleBishop(int rp, int cp, int rd, int cd, int thePos, BlankBurntOX theBlankBurntOX[])
{

  if (kPrintDebug)
      printf(" PossibleBishop: rd=%d cd=%d|", rd, cd);

  /* is it on the board? */
  if ((rd < 0) || (rd > COLUMNSIZE) || (cd < 0) || (cd > ROWSIZE))
    {
      if (kPrintDebug)
	printf("OffBoard\n");
      return FALSE;
    }
  
/*a. theDest is to the left of the position */
   /*
   <-cd-> cp
     <-|
     <-| 
  rp <-|thePos
     <-|
     <-|
   */
  if (cp > cd )
    {
      /*1a the Dest is below the position */
   /*
   <-cd-> cp
     
        
  rp    thePos
  ^     /
  rd   /
  v   /
   */
      if (rp < rd)
	{
	  /* I don't want to check (rp, cp) itself since it's not 
           * blank, therefore I look one ahead of my spot*/
	  while(rp < rd)
	    {
	      rp++;
	      cp--;
	      if(theBlankBurntOX[rp*COLUMNSIZE + cp] != Blank)
		{
		  if (kPrintDebug)
		     printf("1a");
		  return FALSE;
		}
	    }
	   /* the spots are all Blank --> you can move there if the 
	    * destination actually is on that diagonal!*/
	  if (kPrintDebug)
	    printf("Yes!");
	  return (cp == cd && rp == rd);
	}
      /*2a theDest is above the position */
   /*
   <-cd-> cp
  ^  \    
  rd  \
  v    \            
  rp    thePos
   */
      else if (rp > rd){
	 /* I don't want to check (rp, cp) itself since it's not 
	  * blank, therefore I look one ahead of my spot*/
	while(rp > rd)
	  {
	    rp--;
	    cp--;
	    if(theBlankBurntOX[rp*COLUMNSIZE + cp] != Blank)
	      {
		if (kPrintDebug)
		  printf("2a");
		return FALSE;
	      }
	  }
	/* the spots are all Blank --> you can move there if the 
         * destination actually is on that diagonal! */
	if (kPrintDebug)
	  printf("Yes!");
	return (cp == cd && rp == rd);
      }
      /*3a not a Bishop move */
      else
	{
	   if (kPrintDebug)
		  printf("3a");
	  return FALSE;
	}
    }
  
/*b. theDest is to the right of the position */
    /*
          cp <-cd->
            |->
            |->
  rp    thePos->
            |->
            |->

    */  
  else if (cp < cd )
    {

      /*1b theDest is above the position */
    /*
          cp <-cd->
  ^             /
  rd           /
  v           /
  rp    thePos
    */
      if (rp > rd)
	{
	   /* I don't want to check (rp, cp) itself since it's not 
           * blank, therefore I look one ahead of my spot*/
	  while(rp > rd)
	    {
	      rp--;
	      cp++;
	      if(theBlankBurntOX[rp*COLUMNSIZE + cp] != Blank)
		{
		  if (kPrintDebug)
		  printf("1b");
		  return FALSE;
		}
	    }
      /* the spots are all Blank --> you can move there if the 
       * destination actually is on that diagonal!*/
	  if (kPrintDebug)
	    printf("Yes!");
	  return (cp == cd && rp == rd);
	}
      /*2b theDest is below the position */
    /*
  cp <-cd->

   
  rp    thePos
  ^          \    
  rd          \
  v            \  
    */      
      else if (rp < rd)
	{
	  /* I don't want to check (rp, cp) itself since it's not 
           * blank, therefore I look one ahead of my spot*/
	while(rp < rd)
	  {
	    rp++;
	    cp++;
	    if(theBlankBurntOX[rp*COLUMNSIZE + cp] != Blank)
	      {
		if (kPrintDebug)
		  printf("2b");
		return FALSE;
	      }
	  }
	      /* the spots are all Blank --> you can move thereif the 
	       * destination actually is on that diagonal!*/
	if (kPrintDebug)
	  printf("Yes!");
	return (cp == cd && rp == rd);
	}
      /*3b. not a Bishop move */
      else
	{
	   if (kPrintDebug)
		  printf("3b");
	  return FALSE;
	}
    }
/*c. not a Bishop move */
  else
    {
       if (kPrintDebug)
		  printf("c");
      return FALSE;
    }
}



/*The possibilities make my head hurt......
    2     cp     1
        <-||->
    ^^^^<-||->^^^^        
  rp----thePos-----
    vvvv<-||->vvvv
        <-||->  
    3            4
*/
BOOLEAN PossibleRook(int rp, int cp, int rd, int cd, int thePos, BlankBurntOX theBlankBurntOX[]){

  if (kPrintDebug)
      printf(" PossibleRook: rd=%d cd=%d|", rd, cd);

  /* is it on the board? */
  if ((rd < 0) || (rd > COLUMNSIZE) || (cd < 0) || (cd > ROWSIZE))
    {
      return FALSE;
    }
/* the rows are the same */
/*        cp 

    -------------- 
  rp<---thePos--->
    --------------
*/
  if(rp == rd){
      /* if cp is less than cd, we want to
	 check everything in the row AFTER cp but not AFTER cd */
    if(cp < cd)
      {
	while(cp < cd)
	  {
	    cp++;
	    if(theBlankBurntOX[rp*COLUMNSIZE + cp] != Blank)
	      {
		return FALSE;
	      }
	  }
	   /* the spots are all Blank --> you can move there */
	if (kPrintDebug)
	printf("Yes!");
	    return TRUE;
      }
/* check everything in the row BEFORE cp but not BEFORE cd */
    else if(cp > cd)
      {
	while(cp > cd)
	  {
	    cp--;
	    if(theBlankBurntOX[rp*COLUMNSIZE + cp] != Blank)
	      {return FALSE;}
	  }
/* the spots are all Blank --> you can move there */
	if (kPrintDebug)
	printf("Yes!");
	return TRUE;
      }
    /* the move IS the position (not valid) */
    else if(cp == cd){
      {
	return FALSE;
      }
    }
  }
  
/* the columns are the same */
/*
          cp 
         |^|
         |||   
  rp   thePos
         |||
	 |v|
*/
  else if (cp == cd)
    {
/* if rp is less than rd, we want to
   check everything in the column AFTER rp but not AFTER rd */
      if(rp < rd)
	{
	  while(rp < rd)
	    {
	      rp++;
	      if(theBlankBurntOX[rp*COLUMNSIZE + cp] != Blank)
		{
		  return FALSE;
		}
	    }
/* the spots are all Blank --> you can move there */
	  if (kPrintDebug)
	printf("Yes!");
	  return TRUE;
	}
/* check everything in the column BEFORE rp but not BEFORE rd */
      else if(rp > rd)
	{
	  while(rp > rd)
	    {
	      rp--;
	      if(theBlankBurntOX[rp*COLUMNSIZE + cp] != Blank)
		{
		  return FALSE;
		}
	    }
	  
/* the spots are all Blank --> you can move there */
	  if (kPrintDebug)
	    printf("Yes!");
	  return TRUE;
	}
          /* the move IS the position (not valid) */
    else if(rp == rd)
      {
	return FALSE;
      }
    }
/* not a rook move */
  else
    {
      return FALSE;
    }
}



/**************************************************/
/*Ouch, the Possibilities
     c1 c2 c3 c4
r1      a- -b
r2   c -  |  - d
       thePos  
r3   e -  |  - f
r4      g- -h

*/
  
/* Knight jumps over occupied squares so only the destination 
 * matters, the King only moves one square, so the same is true */
BOOLEAN PossibleKnight(int rp, int cp, int rd, int cd, int thePos, BlankBurntOX theBlankBurntOX[]){
  int r1 = rp - 2;
  int r2 = rp - 1;
  int r3 = rp + 1;
  int r4 = rp + 2;
  int c1 = cp - 2;
  int c2 = cp - 1;
  int c3 = cp + 1;
  int c4 = cp + 2;

//  printf("PossibleKnight Called: rd%d cd%d, rp%d cp%d", rd, cd, rp, cp);
/* faster to simply check the destination and whether
 * the move is something a knight can do */
  if ((rd >= 0) && (rd < ROWSIZE) && (cd >= 0) && (cd < COLUMNSIZE) &&
      (theBlankBurntOX[COLUMNSIZE*rd + cd] == Blank) &&
      ((rd == r1 && (cd == c2 || cd == c3)) ||
       (rd == r2 && (cd == c1 || cd == c4)) ||
       (rd == r3 && (cd == c1 || cd == c4)) ||
       (rd == r4 && (cd == c2 || cd == c3)))
      )
    {
      if (kPrintDebug)
	printf("Yes!");
      return TRUE;
    }
  else
    {
//      printf("Location: %d\n", theBoard[5*rd + cd]);
      return FALSE;
    }
}

/**************************************************/
/*Yow, the Possibilities
     c1  cp  c3
     
r1   a    b   c  
rp   d thePos e 
r3   f    g   h

*/
BOOLEAN PossibleKing(int rp, int cp, int rd, int cd, int thePos, BlankBurntOX theBlankBurntOX[]){

  int r1 = rp + 1;
  int r3 = rp - 1;
  int c1 = cp - 1;
  int c3 = cp + 1;

  if (kPrintDebug)
      printf(" PossibleKing: rd=%d cd=%d|", rd, cd);

  /* faster to simply check the destination and whether
      * the move is something a king can do */
  if ((theBlankBurntOX[rd*COLUMNSIZE + cd] == Blank) &&
      (rd == r1 || (rd == rp && cd != cp) || rd == r3) &&
      (cd == c1 || (cd == cp && rd != rp) || cd == c3))
    {
      if (kPrintDebug)
	printf("Yes!");
      return TRUE;
    }
  else
    {
      return FALSE;
    }
}

/* Oh no, not more Possibilities....
 * Haha, fooled ya! We can cheat and use old stuff here! Nyaaa Nyaaa.. */
BOOLEAN PossibleQueen(int rp, int cp, int rd, int cd, int thePos, BlankBurntOX theBlankBurntOX[])
{
  return (PossibleBishop(rp, cp, rd, cd, thePos, theBlankBurntOX) ||
    PossibleRook(rp, cp, rd, cd, thePos, theBlankBurntOX));
  /* Notice that PossibleBishop and Rook return false when the move 
   * is not of their type */
}

/******************
 * PossibleBurn
 *
 * Uses the burnType global to decide how to evaluate
 * the given burn move.
 * INPUTS: pPos, mPos, bPos, Pos = the current piece position, proposed move,
 *         proposed burn, and board position respectively.
 * 
 ******************/
/* Note, burn AFTER move is implemented merely by passing Possible burn
 * i (the proposed move), along with piecePos (its current position). */ 
BOOLEAN PossibleBurn(unsigned int pPos, unsigned int mPos, unsigned int bPos, POSITION Pos, BlankBurntOX theBlankBurntOX[]){
  int piecePos;
  MOVE aMove;
  POSITION movedPos;

  piecePos = GetPFromPosition(Pos);
  
  switch(burnType){

/* burn old position (it MUST be open) */
  case 0:
    return (pPos == bPos);
    break;
    
/* burn anywhere */
  case 1:
    return (theBlankBurntOX[bPos] == Blank); // || pPos == bPos);
    break;
    
/* burn based on how your piece moves (you can't throw over other 
 * burns) */
  case 2:
    /* Either */
    /* from your starting spot, then move. */
    /* move, then (from your new position) burn. */
    if(!gBurnMove)
      {
	aMove = EncodeTheMove(mPos, mPos);
	Pos = DoMove(Pos, aMove);
	Pos = ChangeTurn(Pos, Pos);
	PositionToBlankBurntOX(Pos, theBlankBurntOX);
      }
    return (PieceBurn(bPos, Pos, theBlankBurntOX));
    break;

/* Not valid burnType == false */
  default:
    if (kPrintDebug)
    printf("\nBad burnType"); // for debugging
    return FALSE;
    break;
  };
}


BOOLEAN PieceBurn(int theDest, POSITION thePos, BlankBurntOX theBlankBurntOX[])
{
  int playerPos;
  int rp;
  int cp;
  int rd;
  int cd;
  int i;
  int j;
  
  BlankBurntOX  WhoseTurn();
  

  playerPos = GetPFromPosition(thePos);
  
  /* Set the PieceType we'll be working
     with */
  if (WhoseTurn(thePos) == x)
    {
      theBurn = XBurn;
    }
  else
    {
      theBurn = OBurn;
    }

  for(i=0; i < ROWSIZE; i++){    //Get the player's location in 2D
    for(j=0; j < COLUMNSIZE; j++){
      if((i*COLUMNSIZE) + j == playerPos){
	rp = i;
	cp = j;
      }
    }
  }
    

  for(i=0; i < ROWSIZE; i++){    //Get theDest's location in 2D
    for(j=0; j < COLUMNSIZE; j++){
      if((i*COLUMNSIZE) + j == theDest){
	rd = i;
	cd = j;
      }
    }
  }
  
  if(rd > (ROWSIZE-1) || cd > (COLUMNSIZE-1) || rd < 0 || cd < 0)
    {
      printf("Off the board");
      return FALSE;
    }
  else
    {
      switch (theBurn) {
      case knight:
	return(PossibleKnight(rp, cp, rd, cd, thePos, theBlankBurntOX));
	break;
      case queen:
	return(PossibleQueen(rp, cp, rd, cd, thePos, theBlankBurntOX));
	break;
      case rook:
	return(PossibleRook(rp, cp, rd, cd, thePos, theBlankBurntOX));
	break;
      case bishop:
	return(PossibleBishop(rp, cp, rd, cd, thePos, theBlankBurntOX));
    break;
      case king:
	return(PossibleKing(rp, cp, rd, cd, thePos, theBlankBurntOX));
	break;
      default:
	BadElse("Error in PossibleMove\n");
	break;
      }
    }
}

/************************************************************************
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
**              BOOLEAN PossibleMove(MOVE, POSITION) 
**
************************************************************************/

MOVELIST *GenerateMoves(position)
  POSITION position;
{
  MOVELIST *CreateMovelistNode();
  MOVELIST *head = NULL;
  VALUE Primitive();
  BlankBurntOX WhoseTurn();

  int i;
  int j;
  int piecePos;
  MOVE aMove;
  BlankBurntOX theBlankBurntOX[BOARDSIZE];
  PositionToBlankBurntOX(position, theBlankBurntOX);
  
  piecePos = GetPFromPosition(position);
  if (kPrintDebug)
    {
      printf("Generate's WhoseTurn thinks: %d is up and in position %d\n",
	     WhoseTurn(position), piecePos); //for debugging 
      printf("0=win, 1=lose, 2=tie, 3=undecided, 4=visited\n"); //for debugging
      printf("Generate's Primitive thinks the board is %d\n", Primitive(position)); //for debugging
    }
  
  
  if (Primitive(position) == undecided)
    {
      for(i = 0; i < BOARDSIZE; i++)
	{
	  if(PossibleMove(i, position, theBlankBurntOX))
	    {
              /*IF it's a possible move, find the burn variations.*/
	      for(j = 0 ; j < BOARDSIZE ; j++)
		{ 
		  if(PossibleBurn(piecePos, i, j, position, theBlankBurntOX)
		     && (i != j))
		    {
		      /* IF it's a possible burn, and the burn isn't 
                       * the same as the move, add the encoded move
		       * to the movelist.*/
		      aMove = EncodeTheMove(i, j);
		      head = CreateMovelistNode(aMove, head);
		      if (kPrintDebug)
			{
			  printf("Move: %d, burn: %d, together %d\n", i, j, aMove); //for debugging
			}
		    }
		}
	    }
	}
      return (head);
    }
  else
    return (NULL);
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
/* 10/19 altered print string... we've got some small problems ahead 
 * (2-part move..)
*/
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
    printf("Input must be in format: ## ## :where each # is a digit.\n");
    printf("%8s's move [(u)ndo/ 1-20] %8s burnt[1-20]:", playerName, playerName);
    
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
**              from the numbers 1-20, and the user chooses 0, it's not
**              valid, but anything from 1-20 IS, regardless if the slot
**              is filled or not. Whether the slot is filled is left up
**              to another routine. The move must also have the burn move.
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
  int currentnum=0;
  int move = 0;
  int burn = 0;

  if (strlen(input) != 5)
    {
      return FALSE;
    }
  else
    {
      if (kPrintDebug)
      printf("ValidTextInput\n"); /*for debugging */

      while(input[i] != '\0')
	{
	  if(isalpha(input[i]) || (isspace(input[i]) && i != 2))
	    {
	      return FALSE;
	    }
	  else
	    {
	      currentnum = input[i] - '0';
	      switch(i){
	      case 0:
		  move = currentnum * 10;
		  break;
	      case 1:
		  move = move + currentnum;
		  break;
	      case 2:
		if(!(isspace(input[i])))
		  {
		    return FALSE;
		  }
		break;
	      case 3:
		  burn = currentnum * 10;
		  break;
	      case 4:
		  burn = burn + currentnum;
		  break;
	      default:
		/* do nothing */
		break;
	      };
	    }
	  i++;
	}
      if(kPrintDebug)
      printf("m: %d, b: %d\n", move, burn); /* for debugging */

      if((move <= BOARDSIZE && move >= 1) && (burn <= BOARDSIZE && burn >= 1))
	{
	  return TRUE;
	}
      else
	{
	  if(kPrintDebug)
	  printf("final If fails"); /* for debugging*/
	  return FALSE;
	}
    }
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
/* Isaac: 10/18 */
MOVE ConvertTextInputToMove(input)
  STRING input;
{
  int j = 0;
  int currentnum = 0;
  int move = 0;
  int burn = 0;
  if (kPrintDebug)
  printf("ConvertText called\n");       /*for debugging */
  
  while(j < 5)
    {
      currentnum = input[j] - '0';
      switch(j){
      case 0:
	move = currentnum * 10;
	break;
      case 1:
	move = move + currentnum;
	break;
      case 3:
	burn = currentnum * 10;
	break;
      case 4:
	burn = burn + currentnum;
	break;
      default:
	/* do nothing */
	break;
      };
      j++;
    }

  /*adjust for the program to use */

  move = move - 1; 
  burn = burn - 1;

  if(kPrintDebug)
  printf("m: %d, b: %d\n", move, burn); /* for debugging */
  
/* This version has the first move input as the first 5 bits. */
  return (EncodeTheMove(move, burn)); 
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
  int burn;
  int move;
  burn = ExtractBurn(theMove);
  move = ExtractMove(theMove);
	/* The plus 1 is because the user thinks it's 1-9, but MOVE is 0-8 */
	printf("m:%d, b:%d", move + 1, burn + 1); 
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
  BlankBurntOX theBlankBurntOX[BOARDSIZE], WhoseTurn(), whoseTurn;
  FUZZY FuzzifyValue(), FuzzyComplement(), FuzzySnorm(), goodness;
  FUZZY GoodMiddleX(), GoodOppositeX();

  PositionToBlankBurntOX(thePosition,theBlankBurntOX);
  whoseTurn = WhoseTurn(thePosition);

  if(gStandardGame)
    return(FuzzifyValue(lose));  /* we don't know - random */
  else {

    /** we evaluate the GOODNESS of the position for player 1
     ** and then take the complement if we find out that it's
     ** really player 2's turn.
     **/

    goodness = FuzzySnorm(GoodMiddleX(theBlankBurntOX),GoodOppositeX(theBlankBurntOX));
    
    if(whoseTurn == x)
      return(goodness);
    else
      return(FuzzyComplement(goodness));
  }
}

/************************************************************************
**
** NAME:        GoodMiddleX
**
** DESCRIPTION: Returns the GOODNESS value of having X in the middle
** 
** INPUTS:      BlankOX theBlankOX : the description of the position
**
** OUTPUTS:     (FUZZY) : returns FUZZY [0,1] based on Goodness for x
**
************************************************************************/

FUZZY GoodMiddleX(theBlankBurntOX)
     BlankBurntOX theBlankBurntOX[];
{
  BlankBurntOX middlePiece;
  middlePiece = theBlankBurntOX[4];  /* the middle piece */
  if(middlePiece = x)
    return(MAX_FUZZY_VALUE);
  else if(middlePiece = o)
    return(MIN_FUZZY_VALUE);
  else
    return(TIE_FUZZY_VALUE);
}
  
/************************************************************************
**
** NAME:        GoodOppositeX
**
** DESCRIPTION: Returns the GOODNESS value for X of having a piece
**              on the opposite side as the other person's piece
** 
** INPUTS:      BlankOX theBlankOX : the description of the position
**
** OUTPUTS:     (FUZZY) : returns FUZZY [0,1] based on Goodness for x
**
************************************************************************/

FUZZY GoodOppositeX(theBlankBurntOX)
     BlankBurntOX theBlankBurntOX[];
{
  FUZZY returnValue = MAX_FUZZY_VALUE, costForNotMatching;
  int i;

  costForNotMatching = 0.2;   /* THIS BETTER NOT DROP BELOW 0.25!! */  
  
  for(i = 0; i < 4; i++)
    if((theBlankBurntOX[i] != theBlankBurntOX[BOARDSIZE-i]) &&
       ((theBlankBurntOX[i] == Blank) || theBlankBurntOX[BOARDSIZE-i] == Blank))
      returnValue -= costForNotMatching;

  return(returnValue);
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
** CALLS:       PositionToBlankOX(POSITION,*BlankOX)
**              BlankOX WhosTurn(*BlankOX)
**
************************************************************************/

MINIMAX PositionToMinOrMax(thePosition)
     POSITION thePosition;
{
  BlankBurntOX WhoseTurn();
  return(WhoseTurn(thePosition) == x ? maximizing : minimizing);
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
** CALLS:       POSITION GetCanonicalPosition (POSITION)
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
**         EVERYTHING BELOW THESE LINES IS LOCAL TO THIS 
 * FILE
*************************************************************************
************************************************************************/

/************************************************************************
 ** NAME:       GetXFromPosition
 **
 ** Desc:       Returns X's location from the Position.  The X is in 
 **             the 7-11 bits (after turn bit and 5 O bits). 
 **
 ** INPUT:      POSITION thePos    : the position input.
 **
 ***********************************************************************/
unsigned int GetXFromPosition(thePos)
     POSITION thePos;
{
  return ((thePos & (BITSIZEMASK << (BITSIZE+1))) >> (BITSIZE+1));
}

/************************************************************************
 ** NAME:       GetOFromPosition
 **
 ** Desc:       Returns O's location from the Position.  The O is in 
 **             the 2-6 bits (after turn bit). 
 **
 ** INPUT:      POSITION thePos    : the position input.
 **
 ***********************************************************************/
unsigned int GetOFromPosition(thePos)
     POSITION thePos;
{
  return ((thePos & (BITSIZEMASK << 1)) >> 1);
}


/************************************************************************
 ** NAME:       GetPFromPosition
 **
 ** Desc:       Returns current Player's location from the Position.  The X is in 
 **             the 7-11 bits (after turn bit and 5 O bits). 
 **
 ** INPUT:      POSITION thePos    : the position input.
 **
 ***********************************************************************/
unsigned int GetPFromPosition (POSITION position){
  unsigned int piecePos;
  
  if(WhoseTurn(position) == x)
    {
      piecePos = GetXFromPosition(position);
    }
  else
    {
      piecePos = GetOFromPosition(position);
    }
  return (piecePos);
}

/************************************************************************
**
** NAME:        PositionToBlankOX
**
** DESCRIPTION: convert an internal position to that of a BlankOX.
** 
** INPUTS:      POSITION thePos     : The position input. 
**              BlankOX *theBlankOx : The converted BlankOX output array. 
**
** CALLS:       BadElse()
**
************************************************************************/

/* Dave: Added a complete rewrite of Hash function: PositionToBlankBurntOX.
 * It uses bitwise operations rather than a g3Array, storing data inside
 * an integer.  First bit is turn bit, next 5 bits is o's position 
 * (0-20),
 * second 5 bits is x's position (0-20), last 20 bits are whether a
 * square is occupied or not
 * (0-1). */

PositionToBlankBurntOX(thePos,theBlankBurntOX)
     POSITION thePos;
     BlankBurntOX *theBlankBurntOX;
{
  int i;
  int TheXSpot = GetXFromPosition(thePos);
  int TheOSpot = GetOFromPosition(thePos);
  for(i = BOARDSIZE-1; i >= 0; i--) {
    int SquareStatus = ((thePos & (1 << (BITSIZE*2+1) + i)) >> (BITSIZE*2+1) + i);
    if(SquareStatus == 0)
      {
      theBlankBurntOX[i] = Blank;
      }
    else if(i == TheXSpot)
      {
	theBlankBurntOX[i] = x;
      }
    else if(i == TheOSpot)
      {
	theBlankBurntOX[i] = o;
      }
    else if(SquareStatus == 1)
      {
      theBlankBurntOX[i] = Burnt;
      }
    else
      {
      BadElse("Square can only be 1 or 0.");
      }
  }
  
  if(TheXSpot == TheOSpot)
    {
      BadElse("X and O can't share the same spot.");
    }
  /* else
    {
      theBlankBurntOX[TheXSpot] = x;
      theBlankBurntOX[TheOSpot] = o;
      } */
}  

/************************************************************************
**
** NAME:        BlankOXToPosition
**
** DESCRIPTION: convert a BlankOX to that of an internal position.
** 
** INPUTS:      BlankOX *theBlankOx : The converted BlankOX output array.
**
** OUTPUTS:     POSITION: The equivalent position given the BlankOX.
**
************************************************************************/


/* Takes in a board configuration theBlankBurntOX, which
 * is in the form of an array with each element indicating
 * what is in each square.  From this, return a hashed number
 * unique to the board configuration stored in an int data type. */

POSITION BlankBurntOXToPosition(theBlankBurntOX)
     BlankBurntOX *theBlankBurntOX;
{
  POSITION position = 0;
  unsigned int i;
  int burntcount = 0;
  // Set occupied spots, and x,o locations.
  for(i = 0; i < BOARDSIZE; i++)
    {
      if((int)theBlankBurntOX[i] == x)
	{	
	  position = position | (i << (BITSIZE+1));
	  position = position | (1 << i + (BITSIZE*2+1));
	}
      else if((int)theBlankBurntOX[i] == o)
	{
	  position = position | (i << 1);
	  position = position | (1 << i + (BITSIZE*2+1));
	}
      else if((int)theBlankBurntOX[i] == Burnt)
	{
	  burntcount++; // Used for determining turn later.
	  position = position | (1 << (BITSIZE*2+1) + i);
	}
      else if((int)theBlankBurntOX[i] == Blank)
	{  //Do nothing because the bit is already 0
	}
      else
	BadElse("Not possible board piece.");
    }
  // Determine Turn and set turn bit.
  if(burntcount % 2 != 0){  //Odd burncount means its o's turn.
    position = position | 1;}
  
  else //Even burntcount means its x's turn.
   {} //Do nothing (its already set to 0)
  return(position);
}


/************************************************************************
**
** NAME:        WhoseTurn
**
** DESCRIPTION: Return whose turn it is - either x or o. Since x always
**              goes first, we know that if the board has an even number
**              of burnt squares, it's x 
** 
** INPUTS:      BlankOX theBlankOX : The input board
**
** OUTPUTS:     (BlankBurntOX) : Either x or o, depending on whose turn it is
**
************************************************************************/
// Dave: Modified 10/7
BlankBurntOX WhoseTurn(thePosition)
     POSITION thePosition;
{
     if((thePosition & 1) == 0){ 
       return(x);
     }/* In our game, x always goes first */
     else{
       return(o);
     }
}

/**************************************************
 * Function: CanBurn
 *   Takes in a Position and determines if the 
 *   player who goes next has any valid Burns for
 *   for their piecetype after having moved.
 *   Note that we are essentially looking for two
 *   spots to occupy: a yet-to-be-made move
 *   and a burn.
 *   For use with Primitive.
 **************************************************/
BOOLEAN CanBurn(POSITION thePos){
  int i, j, piecePos;
  MOVE aMove;
  POSITION movedPos;
  BlankBurntOX theBlankBurntOX[BOARDSIZE];
  BlankBurntOX movedBlankBurntOX[BOARDSIZE];
  PositionToBlankBurntOX(thePos, theBlankBurntOX);

  piecePos = GetPFromPosition(thePos);
  
  /* the Ugly part; I must find a possible move, do it, and 
   * search the resulting position for a possible burn */
  if(burnType == 0)
    { return (CanMove(thePos));}
  else
    {
      for(i = 0; i < BOARDSIZE; i++)
	{
	  if(PossibleMove(i, thePos, theBlankBurntOX))
	    {
/* Burn the spot to which you'd be moving
 * and move to where you already are:
 * (you have to preserve your current position
 * while simulaneously eliminating the move spot
 * as an option), must change the turn bit back so
 * PossibleBurn checks based on the right piece.*/
	      if(gBurnMove)
		{
		  aMove = EncodeTheMove(piecePos, i);
		  movedPos = DoMove(thePos, aMove);
		  movedPos = ChangeTurn(movedPos, movedPos);
		  PositionToBlankBurntOX(movedPos, movedBlankBurntOX);
		  if (kPrintDebug)
		    printf("CanBurn1");
		}
/* Do the actual move, and burn the spot TO WHICH you 
 * move, must change the turn bit back so
 * PossibleBurn checks based on the right piece.*/
	      else
		{
		  aMove = EncodeTheMove(i, i);
		  movedPos = DoMove(thePos, aMove);
		  movedPos = ChangeTurn(movedPos, movedPos);
		  PositionToBlankBurntOX(movedPos, movedBlankBurntOX);
		  if (kPrintDebug)
		    printf("CanBurn2");
		}
	      if (kPrintDebug)
		printf("movedPos0x%8x, aMove %0x\n", movedPos >> (BITSIZE*2+1), aMove>>BITSIZE);
           /* search the new board for a burn */
	      for(j = 0; j < BOARDSIZE; j++)
		{
		  if(PossibleBurn(piecePos, i, j, movedPos, movedBlankBurntOX))
		    {return TRUE;}
		}
	    }   
	}
/* if the loops finish without doing returning true there is 
 * no valid burn */
  return FALSE;
    }
}
    

/**************************************************
 * Function: CanMove
 *   Takes in a Position and determines if the 
 *   player who goes next has any valid moves for
 *   for their piecetype.
 *   For use with Primitive.
 **************************************************/
BOOLEAN CanMove(thePos)
     POSITION thePos;
{
  int r;
  int c;
  int i;
  int j;
  int playerPos;
  Piecetype piece;

  BlankBurntOX theBoard[BOARDSIZE];     /* Our board in 1D form. */
  PositionToBlankBurntOX(thePos, theBoard);

    /* Set the PieceType we'll be working
     with */
  if (WhoseTurn(thePos) == x)
    {
      piece = Xtype; /* We're checking the OPPOSING player's 
			      * piece */
      playerPos = GetXFromPosition(thePos);
    }
  else
    {
      piece = Otype; /* We're checking the OPPOSING player's 
			      * piece */
      playerPos = GetOFromPosition(thePos);
    }
/* Get the player's position in 2d */
  for(i=0; i < ROWSIZE; i++){    
    for(j=0; j < COLUMNSIZE; j++){
      if((i*COLUMNSIZE) + j == playerPos){
	r = i;
	c = j;
      }
    }
  }

/* Now check if the possible move spots immediately next to the piece are off the board
 * or non-blank. If any are not, then a move is possible.*/
   switch(piece){
   case king:  case queen:
    /* note that the combination of CanBishop and CanRook checks all  
     * the spots immediately surrounding the piece */
    return(CanRookMove(theBoard, r, c) 
	   ||
	   CanBishopMove(theBoard, r, c));
    break;
  case rook:
    if (kPrintDebug)
      printf("\nCanRook\n");
    return(CanRookMove(theBoard, r, c));
    break;
  case bishop:
    if (kPrintDebug)
      printf("\nCanBishop\n");
    return(CanBishopMove(theBoard, r, c));
    break;
  case knight:
    if (kPrintDebug)
      printf("\nCanKnight\n");
    return(CanKnightMove(theBoard, r, c));
    break;
  default:   
    break;
  };
}

/**************************************************/
/*
     c1 c2 c3 c4
r1      a- -b
r2   c -  |  - d
       thePos  
r3   e -  |  - f
r4      g- -h

*/
BOOLEAN CanKnightMove(BlankBurntOX theBlankBurntOX[], int r, int c){
  int r1 = r - 2;
  int r2 = r - 1;
  int r3 = r + 1;
  int r4 = r + 2;
  int c1 = c - 2;
  int c2 = c - 1;
  int c3 = c + 1;
  int c4 = c + 2;

  if (kPrintDebug)
    printf("\nCanKnight wanted to move from %d %d\n", r, c);
	
  if(    (r1 < 0 || c2 <  0 || theBlankBurntOX[r1*COLUMNSIZE + c2] != Blank)
      && (r1 < 0 || c3 >= COLUMNSIZE || theBlankBurntOX[r1*COLUMNSIZE + c3] != Blank)
      && (r2 < 0 || c1 <  0 || theBlankBurntOX[r2*COLUMNSIZE + c1] != Blank)
      && (r2 < 0 || c4 >= COLUMNSIZE || theBlankBurntOX[r2*COLUMNSIZE + c4 ] != Blank) 
      && (r3 >= ROWSIZE || c1 < 0 || theBlankBurntOX[r3*COLUMNSIZE + c1] != Blank)
      && (r3 >= ROWSIZE || c4 >= COLUMNSIZE || theBlankBurntOX[r3*COLUMNSIZE + c4] != Blank)
      && (r4 >= ROWSIZE || c3 >= COLUMNSIZE || theBlankBurntOX[r4*COLUMNSIZE + c3] != Blank)      
      && (r4 >= ROWSIZE || c2 < 0 || theBlankBurntOX[r4*COLUMNSIZE + c2] != Blank)
      )
    {
      return FALSE;
      }
    else
      {
	return TRUE;
      }
}

/**************************************************/
/*
    c1  c2  c3
r1      a
r2  b thePos c 
r3      d

*/
BOOLEAN CanRookMove(BlankBurntOX theBlankBurntOX[], int r, int c){
  int r1 = r - 1;
  int r3 = r + 1;
  int c1 = c - 1;
  int c3 = c + 1;

  if (kPrintDebug)
    printf("\nCanRook wanted to move from %d %d\n", r, c);
  
  if((theBlankBurntOX[r1*COLUMNSIZE + c ] == Blank && r1 >= 0)
     ||
     (theBlankBurntOX[r3*COLUMNSIZE + c ] == Blank && r3 <= ROWSIZE)
     ||
     (theBlankBurntOX[r*COLUMNSIZE + c1 ] == Blank && c1 >= 0)
     ||
     (theBlankBurntOX[r*COLUMNSIZE + c3 ] == Blank && c3 <= COLUMNSIZE))
    {
      return TRUE;
    }
  else
    {
     return FALSE;
    } 
}

/**************************************************/
/*
    c1  c2  c3
r1   a      b
r2    thePos  
r3   c      d

*/
BOOLEAN CanBishopMove(BlankBurntOX theBlankBurntOX[], int r, int c){
  int r1 = r - 1;
  int r3 = r + 1;
  int c1 = c - 1;
  int c3 = c + 1;

  if (kPrintDebug)
      printf("\nCanBishop wanted to move from %d %d\n", r, c);
  
  if((theBlankBurntOX[r1*COLUMNSIZE + c1] == Blank && r1 >= 0 && c1 >= 0)
     ||
     (theBlankBurntOX[r3*COLUMNSIZE + c3] == Blank && r3 <= ROWSIZE && c3 <= COLUMNSIZE)
     ||
     (theBlankBurntOX[r1*COLUMNSIZE + c3] == Blank && r1 >= 0 && c3 <= COLUMNSIZE)
     ||
     (theBlankBurntOX[r3*COLUMNSIZE + c1] == Blank && r3 <= ROWSIZE && c1 >= 0 ))
    {
      return TRUE;
    }
  else
    {
      return FALSE;
    } 
}

/************************************************************************
**
** NAME:        PieceMessage
**
** DESCRIPTION: Prints a message based on the Player's piece
** 
** INPUTS:      POSITION gInitialPosition (uses this to know whose 
**              piece to look at it)
**
** OUTPUTS:     Just changes the piece type of the player whose turn 
**              it is.
**
************************************************************************/
void PieceMessage(POSITION thePos, BOOLEAN DisplayPieceType){

  if(WhoseTurn(thePos) == x)
    {
      if(DisplayPieceType)
	thePiece = Xtype;
      else
	thePiece = XBurn;
    }
  else
    {
      if(DisplayPieceType)
	thePiece = Otype;
      else
	thePiece = OBurn; 
    }
  
  switch(thePiece){
  case king:
     printf("King");
     break;
  case queen:
    printf("Queen");
    break;
  case rook:
    printf("Rook");
    break;
  case bishop:
    printf("Bishop");
    break;
  case knight:
    printf("Knight");
    break;
  default:
    printf("Mystery Piece");
    break;
  };
}

/* Got tired of rewriting the switch
 * this takes either case of K, Q, R, B, or N
 * and changes thePiece to match the choice.*/
void ChangeThePiece(char input){
  switch(input) {
  case 'k': case 'K':
    thePiece = king;
    break;
  case 'q': case 'Q':
    thePiece = queen;
    break;
  case 'r': case 'R':
    thePiece = rook;
    break;
  case 'b': case 'B':
    thePiece = bishop;
    break;
  case 'n': case 'N':
    thePiece = knight;
    break;
  default:
    thePiece = knight;
    break;
  };
}

/************************************************************************
**
** NAME:        ChangePiece
**
** DESCRIPTION: Set the Player's piece and method of throwing
** 
** INPUTS:      POSITION thePos (uses this to know whose 
**              piece to look at it)
**
** OUTPUTS:     Just changes the global piece type and burn type of the 
**              player whose turn it is.
**
************************************************************************/

void ChangePiece(POSITION thePos){ 
  if(WhoseTurn(thePos) == x)
    {
      printf("X ");
      thePiece = Xtype;
      theBurn = XBurn;
    }
  else
    {
      printf("O ");
      thePiece = Otype;
      theBurn = OBurn;
    }
  
  printf("moves like a ");
  PieceMessage(thePos, TRUE);
  printf(" and stings like a ");
  PieceMessage(thePos, FALSE);
/* change movement */
  printf(".\nChoose your piece: K, Q, R, B, N\n");
  ChangeThePiece(GetMyChar());
  if(WhoseTurn(thePos) == x)
    {
      Xtype = thePiece;
    }
  else
    {
      Otype = thePiece;
    }
/* change burning */
  printf("Choose your sting: K, Q, R, B, N\n"); 
  ChangeThePiece(GetMyChar());
  printf("\nNow ");
  if(WhoseTurn(thePos) == x)
    {
      XBurn = thePiece;
      printf("X ");
    }
  else
    {
      OBurn = thePiece;
      printf("O ");
    }
  
  printf("moves like a ");
  PieceMessage(thePos, TRUE);
  printf(" and stings like a ");
  PieceMessage(thePos, FALSE);
  printf(".\n");
}

void ChangePieces(){
  ChangePiece(gInitialPosition);
  ChangePiece(ChangeTurn(gInitialPosition, gInitialPosition));
}

void ChangeBurn()
{
  char doubleYes;
  printf("There are 3 burntypes:\n1. Burned Bridges: %s\n2. Ultimate Grenade: %s\n3. Piece-Specific Grenade: %s \n", kBBExplanation, kUGExplanation, kBMExplanation);
  printf("Choose 1, 2, or 3: ");
    switch(GetMyChar()) {
    case '1':
      burnType = 0;
      break;
    case '2':
      burnType = 1;
      break;
    case '3':
      burnType = 2;
      break;
    default:
      BadElse("Nice Try.");
      break;
    }
    printf("This last option is not for the faint of heart.\n");
    printf("\nWould you like to burn your previous spots\nAS WELL AS a spot of your choosing? (Y,N)");
    doubleYes = GetMyChar();

    if(doubleYes == 'y' || doubleYes == 'Y')
      {
	doubleTrouble = TRUE;
	printf("\nNOW You're playing with Fire!");
      }
    else
      { doubleTrouble = FALSE;
	printf("\nThat's ok, not everyone likes to take risks\n");
      }
}

/*Isaac 11/29: untested functions to change the board dimensions
 * this stuff should work, but I don't know how it's going to screw
 * with the rest of the code */
void ChangeBoard(){
  printf("How many rows would you like?");
  COLUMNSIZE = GetMyChar();
  printf("How many columns would you like?");
  ROWSIZE = GetMyChar();
  gInitialPosition = GenerateNewInitial();
  printf("Your new board is\n");
  PrintPosition(gInitialPosition);
}
  
/* for changing the Board size--> this creates and returns
 * a new initial position that is correct for the new size*/
int GenerateNewInitial(){
  BlankBurntOX NewInitial[BOARDSIZE];
  int i, j;
  for(i=0; i < ROWSIZE; i++)
    {
      for(j=0; j < COLUMNSIZE; j++)
	{
	  NewInitial[i*ROWSIZE + j] = Blank;
	}
    }

  NewInitial[i*ROWSIZE + j] = x;
  NewInitial[i*ROWSIZE + j] = o;
  return (BlankBurntOXToPosition(NewInitial));
}

/* Change the Order of Burning and Moving */
void ChangeOrder(){
  gBurnMove = !gBurnMove;
  if(gBurnMove && (burnType == 3))
    printf("New Order: Burn based on current position, then move");
  else if(!gBurnMove && burnType == 3)
    printf("New Order: Move, then burn based on the new position");
  else if(burnType == 0)
    printf("Order: Move, then burn the old position");
  else if(burnType == 2)
    printf("Order: Move, then burn anywhere you can.");
}

void PrintExample(){
  int i, j;

  for(i = 0; i < ROWSIZE; i++)
    {
      for(j = 0; j < COLUMNSIZE; j++)
	{
	  if(i == 0 && j == 0)
	    printf("0 ");
	  else if(i == (ROWSIZE - 1) && j == (COLUMNSIZE - 1))
	    printf("X ");
	  else if(i == 1 && j == (COLUMNSIZE - 1))
	    printf("-             <----- EXAMPLE\n");
	  else if(j == (COLUMNSIZE - 1))
	    printf("-\n");
	  else
	    printf("- ");
	}
    }
}
	 
void ChangeBoardSize(){
  int i, j;
  printf("You may not play with more than 20 spaces\non the board!");
  printf("# of Rows?");
  i = GetMyChar() - '0';
  printf("# of Columns?");
  j = GetMyChar() - '0';

  COLUMNSIZE = j;
  ROWSIZE = i;

  BOARDSIZE = COLUMNSIZE * ROWSIZE;

  if(BOARDSIZE <= 16)
    BITSIZE = 4;
  else
    BITSIZE = 5;

  if(kPrintDebug)
    printf("ROWSIZE %d, COLUMNSIZE %d BITSIZE %d",
	   ROWSIZE, COLUMNSIZE, BITSIZE);
  printf("Your New Board has %d spaces", BOARDSIZE);
}
