/************************************************************************
**
** NAME:        mfoxes.c
**
** DESCRIPTION: Foxes and Geese
**
** AUTHOR:      Sergey Kirshner - University of California at Berkeley
**              Copyright (C) Dan Garcia, 1997. All rights reserved.
**
** DATE:        To be filled later
**
** UPDATE HIST:
**
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
#include "deprecated/gsolve.h"

#include "tcl.h"
#include "tk.h"


#define FOXESGEESETRAP 0
#define FOXESGEESENOTRAP 1


extern STRING gValueString[];

/* In this version, the inital position is:
** 
**  - - f -  Which corrseponds to 17482
** - - - -
**  - - - -
** g g g g
**
*/

POSITION gInitialPosition  = 82620 /*82620*//*3080*//*17482*/;  /*to be changed later in the file --
				   in the later versions will be computed
				   based on the given information */
POSITION gMinimalPosition  = 82620 ;

VALUE * gDatabase = (VALUE*)NULL;

BOOLEAN kSupportsHeuristic = FALSE ;
BOOLEAN kBadPosition = -1 ;
BOOLEAN kSupportsSymmetries = FALSE ;

STRING   kGameName           = "Foxes and Geese";
STRING   KDBName             = "foxes";
BOOLEAN  kPartizan           = TRUE;
BOOLEAN  kDebugMenu          = TRUE;
BOOLEAN  kGameSpecificMenu   = TRUE;
BOOLEAN  kTieIsPossible      = FALSE;
BOOLEAN  kLoopy               = TRUE;
BOOLEAN  kDebugDetermineValue = FALSE;

STRING   kHelpGraphicInterface =
"";

STRING   kHelpTextInterface    =
"On your turn, use the LEGEND to detemine which numbers to choose (between\n\
1 and the current size of the board , with 1 at the lower left) to correspond\n\
to the location of your piece and the empty diagonally-adjacent position\n\
you wish to move that piece to. Example: '1 5' will move your piece from\n\
square 1 to square 5.";

STRING   kHelpOnYourTurn =
"";

STRING   kHelpStandardObjective =
"";

STRING   kHelpReverseObjective =
"";

STRING   kHelpTieOccursWhen = 
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

#define BADSLOT -2
#define MAXBOARDSIZE 35          /* To keep compiler happy */

int FOXES = 1;                   /* Number of foxes */
int GEESE = 4;                   /* Number of geese */
int TOTALPIECES = 4 + 1;         /* Total number of pieces on the baord */
int ROWS = 8;

int BOARDSIZE = 4*8;             /* 4x4 (changeable) board, only black squares used
				    imported from the file feature later */
//#define MAXNUMPOSITIONS 4000000  /* Some very big number */
int gNumberOfPositions = 2013760;        /* C(BOARDSIZE,TOTALPIECES)*C(TOTALPIECES,FOXES)*2 */
int POSITION_OFFSET = 1006880;     /* C(BOARDSIZE,TOTALPIECES)*C(TOTALPIECES,FOXES) */

#define MAXNUMBERROWS 8

int C[MAXBOARDSIZE][MAXBOARDSIZE];
BOOLEAN connected[MAXBOARDSIZE][MAXBOARDSIZE];
int order[MAXBOARDSIZE];

typedef enum possibleBoardPieces {
	Blank, f, g
} BlankFG;

typedef int SLOT;     /* A slot is the place where a piece moves from or to */

char *gBlankFGString[] = { "-", "F", "G" };

int gGameType = FOXESGEESETRAP;

/************************************************************************
**
** NAME:        InitializeDatabases
**
** DESCRIPTION: Initialize the gDatabase, a global variable.
** 
** CALLS:       InitializeC()
**
************************************************************************/

InitializeDatabases()
{
  GENERIC_PTR SafeMalloc();
  int i;

  InitializeC();
  InitializeAdjacency();
  InitializeOrder();

  BOARDSIZE = 4 * ROWS;
  POSITION_OFFSET = C[BOARDSIZE][TOTALPIECES]*C[TOTALPIECES][GEESE];
  
  gNumberOfPositions = 2*POSITION_OFFSET;

  gDatabase = (VALUE *)malloc(gNumberOfPositions * sizeof(int)) ;
  for(i = 0 ; i < gNumberOfPositions ; i++)
	gDatabase[i] = undecided ;

  TOTALPIECES = FOXES + GEESE;
}

FreeGame()
{}

MinimalInitialize()
{
  TOTALPIECES = FOXES + GEESE;
  InitializeC();
  InitializeOrder();
  int i ;
  BOARDSIZE = 4 * ROWS;
  POSITION_OFFSET = C[BOARDSIZE][TOTALPIECES]*C[TOTALPIECES][GEESE];
  gNumberOfPositions = 2*POSITION_OFFSET;
  if(!gDatabase)
 	free(gDatabase) ;
  gDatabase = (VALUE *)malloc(gNumberOfPositions * sizeof(int)) ;
  for(i = 0 ; i < gNumberOfPositions ; i++)
	gDatabase[i] = undecided ;
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

GameSpecificMenu() 
{
  char GetMyChar();
  POSITION GetInitialPosition();

  /*  BlankFG whosTurn;*/

  MinimalInitialize();
  
  do {
    printf("\n\t----- Game-specific options for %s -----\n\n", kGameName);
    
    printf("\tCurrent Initial Position:\n");
    /*    whosTurn = ((gInitialPosition>=POSITION_OFFSET)? g : f);*/
    PrintPosition(gInitialPosition, gPlayerName[kPlayerOneTurn], kHumansTurn);

    printf("\n\tCurrent Number of Foxes: %d\n", FOXES);
    printf("\tCurrent Number of Geese: %d\n\n", GEESE);
    printf("\t1)\tChoose (manually) the initial position\n");
    printf("\t2)\tChange the number of pieces and size of the board\n");
    printf("\t3)\tChoose who goes first\n");

    printf("\n\n\tb)\t(B)ack = Return to previous activity.\n");
    printf("\n\nSelect an option: ");
    
    //scanf("%s", c);
    
    switch(GetMyChar()) {
    case 'Q': case 'q':
      ExitStageRight();
    case 'H': case 'h':
      HelpMenus();
      break;
    case '1':
      gInitialPosition = GetInitialPosition();
      break;
    case '2':
      ChangeInitialValues();
      break;
    case '3':
      //printf("Abstraction-broken is: %d",gInitialPosition);
      ChangeTurn();
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

SetTclCGameSpecificOptions(theOptions)
int theOptions[];
{
  BlankFG theBlankFG[BOARDSIZE], whosTurn;

  gInitialPosition = theOptions[4];
  if (theOptions[0]) {
    /* foxes go first */
    PositionToBlankFG(gInitialPosition, theBlankFG, &whosTurn);
    gInitialPosition = BlankFGToPosition(&theBlankFG, f);
   
  }

  TOTALPIECES = theOptions[1];
  FOXES = theOptions[2];
  GEESE = TOTALPIECES - FOXES;
  BOARDSIZE = theOptions[3];
  ROWS = BOARDSIZE/4;

  InitializeDatabases();
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
** CALLS:       MoveToSlots(MOVE,*SLOT,*SLOT)
**
************************************************************************/

POSITION DoMove(thePosition, theMove)
     POSITION thePosition;
     MOVE theMove;
{
  SLOT fromSlot, toSlot;
  BlankFG theBlankFG[BOARDSIZE], whosTurn;
  
  PositionToBlankFG(thePosition,theBlankFG,&whosTurn);
  MoveToSlots(theMove, &fromSlot, &toSlot);
  theBlankFG[toSlot] = theBlankFG[fromSlot];
  theBlankFG[fromSlot]=Blank;

  return(BlankFGToPosition(&theBlankFG,(whosTurn == f ? g : f)));

}  
 

/************************************************************************
**
** NAME:        GetInitialPosition
**
** DESCRIPTION: Ask the user for an initial position for testing. Store
**              it in the space pointed to by initialPosition;
** 
** OUTPUTS:     POSITION initialPosition : The position returned
**
************************************************************************/

POSITION GetInitialPosition() /* UNWRITTEN */
{
  POSITION BlankFGToPosition();
  BlankFG theBlankFG[BOARDSIZE], whosTurn;
  signed char c;
  int i, goodInputs = 0;

  printf("\n\n\t----- Get Initial Position -----\n");
  printf("\n\tPlease input the position to begin with.\n");
  printf("\tNote that it should be in the following format:\n\n");
  printf(" - - F -\n- - - -             <----- EXAMPLE \n - - - -\nG G G G \n\n");

  i = 0;
  getchar();
  while(i < BOARDSIZE && (c = getchar()) != EOF) {
    if(c == 'f' || c == 'F')
      theBlankFG[i++] = f;
    else if(c == 'g' || c == 'G')
      theBlankFG[i++] = g;
    else if(c == '-')
      theBlankFG[i++] = Blank;
    else
      ;   /* do nothing */
  }

  getchar();
  printf("\nNow, whose turn is it? [G/F] : ");
  scanf("%c",&c);
  if(c == 'f' || c == 'F')
    whosTurn = f;
  else
    whosTurn = g;

  return(BlankFGToPosition(theBlankFG,whosTurn));
}

/************************************************************************
**
** NAME:        ChangeInitialValues
**
** DESCRIPTION: Ask the user for new values of FOXES, GEESE and something
**              else 
** 
** OUTPUTS:     None
**
************************************************************************/

ChangeInitialValues() /* UNWRITTEN */
{
  STRING input;
  BlankFG *theBlankFG;
  GENERIC_PTR SafeMalloc();
  int numBlanks, i, k, j, l;

  printf("\n\n\t----- Change Initial Values -----\n");
  printf("\n\tPlease enter the new number of foxes: ");
  scanf("%d", &FOXES);
  printf("\n\tPlease enter the new number of geese: ");
  scanf("%d", &GEESE);
  printf("\n\tPlease enter the new number of rows: ");
  scanf("%d", &ROWS);

  theBlankFG = (BlankFG *) SafeMalloc( (ROWS*4) * sizeof(BlankFG) );
  numBlanks = (ROWS * 4) - (FOXES + GEESE);


  for (j=0; j<FOXES; j++) {
    theBlankFG[j] = f;
    printf("%d\n", j);
  }
  for (k=j; k<(numBlanks+FOXES); k++) {
    theBlankFG[k] = Blank;
    printf("%d\n", k);
  }
  for (l=k; l<(FOXES+GEESE+numBlanks); l++) {
    theBlankFG[l] = g;
    printf("%d\n", l);
  }
  
  MinimalInitialize();

  gInitialPosition = BlankFGToPosition( theBlankFG, g );
  //MinimalInitialize();
  
  free(theBlankFG);
  return;
}

ChangeTurn()
{
  char GetMyChar();
  BlankFG theBlankFG[BOARDSIZE], whosTurn;
  PositionToBlankFG( gInitialPosition, theBlankFG, &whosTurn );

  printf("\n\n\t----- Change Who Goes First -----\n");
  printf("\n\tPlease enter who goes first (f/g): ");
   
  switch (GetMyChar()) {
    case 'G': case 'g':
      gInitialPosition = BlankFGToPosition( theBlankFG, g );
      break;
    case 'F': case 'f':
      gInitialPosition = BlankFGToPosition( theBlankFG, f );
      break;
  default:
    printf("Can't change who goes first to that");
  }

  return;
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

PrintComputersMove(computersMove,computersName)
     MOVE computersMove;
     STRING computersName;
{
  SLOT fromSlot,toSlot;

  MoveToSlots(computersMove,&fromSlot,&toSlot);
  printf("%8s's move              : %d %d\n", computersName, 
	 fromSlot+1,toSlot+1);

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
** CALLS:       
**
************************************************************************/

VALUE Primitive(position) 
     POSITION position;
{
  BlankFG theBlankFG[BOARDSIZE],whosTurn;
  int i, foxMax=0, gooseMax=0;
  
  PositionToBlankFG(position,theBlankFG,&whosTurn);
  /*  for( i=0; i<BOARDSIZE; i++ )
    if( theBlankFG[i]==f )
      if( order[i]==0 )
	return(gStandardGame ? lose : win);*/
  
  /* my primitive */
  for ( i=0; i<BOARDSIZE; i++ ) {
    if ( theBlankFG[i] == f) {
      if ( i/4 > foxMax ) 
	foxMax = i/4;
    } else if ( theBlankFG[i] == g ) {
      if ( i/4 > gooseMax )
	gooseMax = i/4;
    }
  }
  if ( foxMax >= gooseMax && whosTurn == g ) {
    return (gStandardGame ? lose : win);
  } else if ( foxMax >= gooseMax && whosTurn == f ) {
    return (gStandardGame ? win: lose);
  } else if ( CantMove(position) ) {
    return (gStandardGame ? lose : win);
  } else {
    return (undecided);
  }

  
  /* old primitive */ 
  //  if(CantMove(position)) { /* the other player just won */
  //  return(gStandardGame ? lose : win);       
  // }
  //else {
  //  return(undecided);   /* no one has won yet */
  // }
  
}

BOOLEAN CantMove(position)
     POSITION position;
{
  MOVELIST *ptr, *GenerateMoves();
  BOOLEAN cantMove;

  ptr = GenerateMoves(position);
  cantMove = (ptr == NULL);
  FreeMoveList(ptr);
  return(cantMove);
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
** CALLS:       PositionToBlankFG()
**              GetValueOfPosition()
**              GetPrediction()
**
************************************************************************/

PrintPosition(position,playerName,usersTurn)
     POSITION position;
     STRING playerName;
     BOOLEAN  usersTurn;
{
  int i;
  STRING GetPrediction();
  VALUE GetValueOfPosition();
  BlankFG theBlankFg[BOARDSIZE], whosTurn;

  PositionToBlankFG(position,theBlankFg,&whosTurn);

  printf("\t\tLEGEND:\t\t\tPLAYER %s's turn\n\n",gBlankFGString[(int)whosTurn]);
  for( i=0; i<(ROWS/2); i++ ){
  
    printf("         (   ");
    if( 8*i<9 )
      printf(" %d  ", 8*i+1);
    else
      printf("%d  ", 8*i+1);
    if( 8*i<8 )
      printf(" %d  ", 8*i+2);
    else
      printf("%d  ", 8*i+2);
    if( 8*i<7 )
      printf(" %d  ", 8*i+3);
    else
      printf("%d  ", 8*i+3);
    if( 8*i<6 )
      printf(" %d ", 8*i+4);
    else
      printf("%d ", 8*i+4);
    printf(")           :    %s   %s   %s   %s\n",
	   gBlankFGString[(int)theBlankFg[8*i]],
	   gBlankFGString[(int)theBlankFg[8*i+1]],
	   gBlankFGString[(int)theBlankFg[8*i+2]],
	   gBlankFGString[(int)theBlankFg[8*i+3]]);
    printf("         ( ");
    if( 8*i<5 )
      printf(" %d  ", 8*i+5);
    else
      printf("%d  ", 8*i+5);
    if( 8*i<4 )
      printf(" %d  ", 8*i+6);
    else
      printf("%d  ", 8*i+6);
    if( 8*i<3 )
      printf(" %d  ", 8*i+7);
    else
      printf("%d  ", 8*i+7);
    if( 8*i<2 )
      printf(" %d  ", 8*i+8);
    else
      printf("%d  ", 8*i+8);
    printf(" )           :  %s   %s   %s   %s\n",
	   gBlankFGString[(int)theBlankFg[8*i+4]],
	   gBlankFGString[(int)theBlankFg[8*i+5]],
	   gBlankFGString[(int)theBlankFg[8*i+6]],
	   gBlankFGString[(int)theBlankFg[8*i+7]] );
  };
  if( (ROWS%2)!=0 ) {
    i = ROWS-1;
    printf("         (   ");
    if( 4*i<9 )
      printf(" %d  ", 4*i+1);
    else
      printf("%d  ", 4*i+1);
    if( 4*i<8 )
      printf(" %d  ", 4*i+2);
    else
      printf("%d  ", 4*i+2);
    if( 4*i<7 )
      printf(" %d  ", 4*i+3);
    else
      printf("%d  ", 4*i+3);
    if( 4*i<6 )
      printf(" %d ", 4*i+4);
    else
      printf("%d ", 4*i+4);
    printf(")           :    %s   %s   %s   %s\n",
	   gBlankFGString[(int)theBlankFg[4*i]],
	   gBlankFGString[(int)theBlankFg[4*i+1]],
	   gBlankFGString[(int)theBlankFg[4*i+2]],
	   gBlankFGString[(int)theBlankFg[4*i+3]]);
  };    
  printf("\n%s\n\n",GetPrediction(position,playerName,usersTurn));
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
  MOVELIST *head = NULL, *temp;
  MOVELIST *CreateMovelistNode();
  BlankFG theBlankFG[BOARDSIZE], whosTurn;
  int i,j;

  PositionToBlankFG(position,theBlankFG,&whosTurn);
  
  for(i = 0 ; i < BOARDSIZE ; i++) {     /* enumerate over all FROM slots */
    for(j = 0 ; j < BOARDSIZE ; j++) {   /* enumerate over all TO slots */
      if(OkMove(theBlankFG,whosTurn,(SLOT)i,(SLOT)j)) 
	head = CreateMovelistNode(SlotsToMove((SLOT)i,(SLOT)j),head);
    }
  }
  
  /* some testing code - delete temp later 
  temp = head;
  i = 1;
  while (temp != NULL) {
    printf("Move %d is %d\n", i, *temp);
    temp = temp->next;
    i++;
  }
   end of testing code */

  return(head);
}

BOOLEAN OkMove(theBlankFG,whosTurn,fromSlot,toSlot)
     BlankFG *theBlankFG, whosTurn;
     SLOT fromSlot, toSlot;
{

  if( whosTurn==g ) {
    return((theBlankFG[fromSlot] == whosTurn) &&
	   (theBlankFG[toSlot] == Blank) &&
	   (connected[fromSlot][toSlot]) &&
	   (order[toSlot]>order[fromSlot]));
  }
  else {
    return((theBlankFG[fromSlot] == whosTurn) &&
	   (theBlankFG[toSlot] == Blank) &&
	   (connected[fromSlot][toSlot]));
  };
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
  USERINPUT ret, HandleDefaultTextInput();
  int xpos, ypos;
  char input = '0', fromSlotChar, toSlotChar, HandleTextualInput();
  BOOLEAN done = FALSE, ValidMove();
  SLOT fromSlot = BADSLOT, toSlot;
  BlankFG theBlankFG[BOARDSIZE], whosTurn;

  PositionToBlankFG(thePosition,theBlankFG,&whosTurn);
  
  do {
    printf("%8s's move [(u)ndo/1-9] : ", playerName);
    
    ret = HandleDefaultTextInput(thePosition, theMove, playerName);
    if(ret != Continue) {
      return(ret);
    };
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
  SLOT fromSlot, toSlot;
  int ret;

  ret = sscanf(input,"%d %d", &fromSlot, &toSlot);
  return(ret == 2 &&
	 fromSlot <= BOARDSIZE && fromSlot >= 1 && toSlot <= BOARDSIZE && toSlot >= 1);
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
  MOVE SlotsToMove();
  SLOT fromSlot, toSlot;
  int ret;

  ret = sscanf(input,"%d %d", &fromSlot, &toSlot);
  
  fromSlot--;               /* user input is 1-16, our rep. is 0-15 */
  toSlot--;                 /* user input is 1-16, our rep. is 0-15 */
  
  return(SlotsToMove(fromSlot,toSlot));
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
  SLOT fromSlot, toSlot;

  MoveToSlots(theMove,&fromSlot,&toSlot);
  /* The plus 1 is because the user thinks it's 1-9, but MOVE is 0-8 */
  printf("[ %d %d ] ", fromSlot + 1, toSlot + 1);

}

/************************************************************************
*************************************************************************
**         EVERYTHING BELOW THESE LINES IS LOCAL TO THIS FILE
*************************************************************************
************************************************************************/

/************************************************************************
**
** NAME:        PositionToBlankFG
**
** DESCRIPTION: convert an internal position to that of a BlankFG.
** 
** INPUTS:      POSITION thePos     : The position input. 
**              BlankFG *theBlankFG : The converted BlankFG output array. 
**
** CALLS:       FillPosition()
**
************************************************************************/

PositionToBlankFG(thePos,theBlankFG,whosTurn)
     POSITION thePos;
     BlankFG *theBlankFG, *whosTurn;
{
  POSITION piecesPos;  /* position of all pieces (indistinguishable) on the board */
  POSITION insidePos;  /* position of foxes and geese inside the pieces */

  BlankFG tempBlankFG1[BOARDSIZE];
  BlankFG tempBlankFG2[TOTALPIECES];
  
  int i;
  int index = 0;

  if (thePos >= POSITION_OFFSET) {   /* F moves first <==> pos >= offset */
    *whosTurn = f;
    thePos -= POSITION_OFFSET;
  }
  else
    *whosTurn = g;
  

  InitializeC();
  piecesPos = thePos % C[BOARDSIZE][TOTALPIECES];
  insidePos = thePos / C[BOARDSIZE][TOTALPIECES];

  FillPosition(piecesPos, BOARDSIZE, TOTALPIECES, &tempBlankFG1, 0);
  FillPosition(insidePos, TOTALPIECES, FOXES, &tempBlankFG2, 0);

  for( i=0; i<BOARDSIZE; i++ )
    if( tempBlankFG1[i]==g ) {
      if( tempBlankFG2[index]==g )
	theBlankFG[i]=f;
      else
	theBlankFG[i]=g;
      index++;
    }
    else
      theBlankFG[i]=Blank;
  
}  

/************************************************************************
**
** NAME:        FillPosition
**
** DESCRIPTION: Recusively converts POSITION into Blank's and g's
** 
** INPUTS:      POSITION thePos       : The position input. 
**              int      totalPlaces  : The total number of places in the position
**              int      numberPieces : The number of pieces in the position
**              BlankFG  *theBlankFG  : The converted BlankFG output array. 
**              int      startEntry   : The entry to fill pieces into
**
** CALLS:       FillPosition()
**
************************************************************************/

FillPosition (thePos, totalPlaces, numberPieces, theBlankFG, startEntry)
     POSITION thePos;
     int totalPlaces, numberPieces;
     BlankFG *theBlankFG;
     int startEntry;
{
  if( totalPlaces==1 )
    if( numberPieces==1 )
      theBlankFG[startEntry]=g;
    else
      theBlankFG[startEntry]=Blank;
  else
    if( numberPieces==0 ) {
      theBlankFG[startEntry]=Blank;
      FillPosition(thePos, totalPlaces-1, 0, theBlankFG, startEntry+1);
    }
    else
      if( thePos<C[totalPlaces-1][numberPieces-1] ) {
	theBlankFG[startEntry]=g;
	FillPosition(thePos, totalPlaces-1, numberPieces-1, theBlankFG, startEntry+1);
      }
      else {
	theBlankFG[startEntry]=Blank;
	FillPosition(thePos-C[totalPlaces-1][numberPieces-1],
		     totalPlaces-1, numberPieces, theBlankFG, startEntry+1);
      };

  return;
}

/************************************************************************
**
** NAME:        MoveToSlots
**
** DESCRIPTION: convert an internal move to that of two slots
** 
** INPUTS:      MOVE theMove    : The move input. 
**              SLOT *fromSlot  : The slot the piece moves from (output)
**              SLOT *toSlot    : The slot the piece moves to   (output)
**
************************************************************************/

MoveToSlots(theMove, fromSlot, toSlot)
     MOVE theMove;
     SLOT *fromSlot, *toSlot;
{
  *fromSlot = theMove % BOARDSIZE;
  *toSlot   = theMove / BOARDSIZE;

}

/************************************************************************
**
** NAME:        SlotsToMove
**
** DESCRIPTION: convert two slots (from and to) to an encoded MOVE
** 
** INPUT:       SLOT fromSlot   : The slot the piece moves from (0->BOARDSIZE)
**              SLOT toSlot     : The slot the piece moves to   (0->BOARDSIZE)
**
** OUTPUT:      MOVE            : The move corresponding to from->to
**
************************************************************************/

MOVE SlotsToMove (fromSlot, toSlot)
     SLOT fromSlot, toSlot;
{
  return ((MOVE) toSlot*BOARDSIZE + fromSlot);
}

/************************************************************************
**
** NAME:        BlankFGToPosition
**
** DESCRIPTION: convert a BlankFG to that of an internal position.
** 
** INPUTS:      BlankFG *theBlankFG : The converted BlankFG output array.
**
** OUTPUTS:     POSITION: The equivalent position given the BlankFG.
**
************************************************************************/

POSITION BlankFGToPosition(theBlankFG,whosTurn)
     BlankFG *theBlankFG,whosTurn;
{
  int i;
  POSITION position = 0;
  POSITION allpiecesposition = 0;
  POSITION insidepiecesposition = 0;
  int piecesLeft = TOTALPIECES;
  int foxesLeft = FOXES;

  
  for( i=0; i<BOARDSIZE; i++ )
    if( theBlankFG[i]==Blank ) {
      if (piecesLeft>0) {
	allpiecesposition += C[(BOARDSIZE-i)-1][piecesLeft-1];
      }
    }
    else {
      if( theBlankFG[i]==g ) {
	if (piecesLeft>0 && foxesLeft>0) {
	  insidepiecesposition += C[piecesLeft-1][foxesLeft-1];
	}
      }
      else
	foxesLeft--;
      piecesLeft--;
    };
  
  position = insidepiecesposition*C[BOARDSIZE][TOTALPIECES] +
    allpiecesposition;

  if( whosTurn==f )
    position += POSITION_OFFSET;   /* account for whose turn it is */

  return(position);
}

/************************************************************************
**
** NAME:        InitializeC
**
** DESCRIPTION: Initializes an array of binomial coefficients
** 
** INPUTS:      None
**
** OUTPUTS:     None
**
** CALLS:       ComputeC()
**
************************************************************************/

InitializeC (void) {
  int i,j;

  for( i=0; i<MAXBOARDSIZE; i++ )
    for( j=0; j<=i; j++ )
      C[i][j]=-1;  /* Value is not computed yet */

  for( i=0; i<MAXBOARDSIZE; i++ )
    ComputeC(MAXBOARDSIZE-1, i);

  return;
};

/************************************************************************
**
** NAME:        ComputeC
**
** DESCRIPTION: Recusively fills out a Pascal Triangle in the array C.
** 
** INPUTS:      int n, k
**
** OUTPUTS:     None
**
************************************************************************/

int ComputeC (int n, int k) {

  if( C[n][k] == -1 )      /* the value C(n,k) is not yet computed */
    if( (k==0) || (n==k) ) /* If k=0 or n=k, then C(n,k)=1 */
      C[n][k]=1;
    else {                 /* Else we use the formula */
      ComputeC(n-1, k);      /* C(n,k)=C(n-1,k)+C(n-1,k-1) */
      ComputeC(n-1, k-1);
      C[n][k]=C[n-1][k]+C[n-1][k-1];
    };
  return(C[n][k]);
}

/************************************************************************
**
** NAME:        InitializeAdjacency
**
** DESCRIPTION: Initializes adjacency matrix 'conncted'.
** 
** INPUTS:      None
**
** OUTPUTS:     None
**
************************************************************************/

InitializeAdjacency (void) {
  int i, j;
  
  for( i=0; i<MAXBOARDSIZE; i++ )
    for( j=i; j<MAXBOARDSIZE; j++ )
      if( ((j-i)==4) ||
	  ( (j-i)==5 && ( (i % 8)==0 || (i % 8)==1 || (i % 8)==2 ) ) ||
	  ( (j-i)==3 && ( (i % 8)==5 || (i % 8)==6 || (i % 8)==7 ) )) {
	connected[i][j]=1;
	connected[j][i]=1;
      }
      else {
	connected[i][j]=0;
	connected[j][i]=0;
      };

  return;
}

/************************************************************************
**
** NAME:        InitializeOrder
**
** DESCRIPTION: Initializes order of the nodes (array order).
** 
** INPUTS:      None
**
** OUTPUTS:     None
**
************************************************************************/

InitializeOrder (void) {
  int i;

  for( i=0; i<ROWS; i++ ) {
    order[4*i]=(ROWS-1)-i;
    order[4*i+1]=(ROWS-1)-i;
    order[4*i+2]=(ROWS-1)-i;
    order[4*i+3]=(ROWS-1)-i;
  }

  return;
}

/************************************************************************
**
** NAME:        ReadDataFromFile
**
** DESCRIPTION: Reads data from file
** 
** INPUTS:      None
**
** OUTPUTS:     None
**
************************************************************************/

/* ReadDataFromFile(void) {
  
   
   return;
   }
*/

STRING kDBName = "foxes" ;
     
int NumberOfOptions()
{    
        return 0 ;
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


MOVE GetComputersMove(thePosition)
     POSITION thePosition;
{
  int i, randomMove, numberMoves = 0;
  MOVELIST *ptr, *head, *GetValueEquivalentMoves();
  MOVE theMove;
  
  if(gPossibleMoves) 
    printf("%s could equivalently choose [ ", gPlayerName[kComputersTurn]);
  head = ptr = GetValueEquivalentMoves(thePosition);
  while(ptr != NULL) {
    numberMoves++;
    if(gPossibleMoves) 
      printf("%d ",ptr->move);
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
  return(minimizing);  /* since we're impartial */
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
  return (VALUE*)(&gDatabase[position]);
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

int GameSpecificTclInit(Tcl_Interp* interp,Tk_Window mainWindow) {}
