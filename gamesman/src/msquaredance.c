#define YELLOW 0
#define BLUE 1
#define DOWN 0
#define UP 1
#define EMPTY 0
#define POSSIBLE_SQUARE_VALUES 5  // 5 possible values for a square, should not be changed


#define ROW_START '0'
#define COL_START '0'


/* Note: DOES NOT support size of 4 x 4 or greater
 */

#define DEFAULT_BOARD_WIDTH 3
#define DEFAULT_BOARD_HEIGHT 3

/* Note:
 * comment out this line for Gamesman compiling
 * keep this when run the main() test on its own
 */
//#define SQUAREDANCEDEBUG

#ifdef SQUAREDANCEDEBUG
	#define NULL 0
	#define FALSE 0
	#define TRUE 1
	typedef char* STRING;
	typedef int BOOLEAN;
	typedef long int POSITION;
	typedef int MOVE;
	BOOLEAN gStandardGame = TRUE;
	typedef enum {
        win, lose, draw, tie, undecided } VALUE;
#endif


/************************************************************************
**
** NAME:        msquaredance.c
**
** DESCRIPTION: Square Dance
**
** AUTHOR:      Yu-TE (Jack) Hsu
**				Joey Corless
**
** DATE:        WHEN YOU START/FINISH
**
** UPDATE HIST: RECORD CHANGES YOU HAVE MADE SO THAT TEAMMATES KNOW
**
** 2005/11/11
**   Jack - add more #ifdef and #ifndef for selftest main(), we can remove the line:
**              #define SQUAREDANCEDEBUG
**          to compile this file alone without importing
**   Joey - Fixed various bugs, most notably fixing Primitive to return properly
**          for standard game.  Also changed size to 3x2, since 3x3 and above fails.
**   Joey - Incomplete version of squaredance. Compiled and runs, but crashes
**          upon starting game with or without solve.
**************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/
#ifndef SQUAREDANCEDEBUG

#include <stdio.h>
#include "gamesman.h"
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

#endif
/*************************************************************************
**
** Game-specific constants
**
**************************************************************************/

STRING   kGameName            = "Square Dance"; /* The name of your game */
STRING   kAuthorName          = "Jack Hsu, Joey Corless"; /* Your name(s) */
STRING   kDBName              = "squaredance"; /* The name to store the database under */

void* gGameSpecificTclInit = NULL;

BOOLEAN  kPartizan            = TRUE ; /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN  kGameSpecificMenu    = FALSE ; /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN  kTieIsPossible       = TRUE ; /* TRUE if a tie is possible. FALSE if it is impossible.*/
BOOLEAN  kLoopy               = FALSE ; /* TRUE if the game tree will have cycles (a rearranger style game). FALSE if it does not.*/

BOOLEAN  kDebugMenu           = TRUE ; /* TRUE only when debugging. FALSE when on release. */
BOOLEAN  kDebugDetermineValue = TRUE ; /* TRUE only when debugging. FALSE when on release. */

POSITION gNumberOfPositions   =  3000000; /* The number of total possible positions | If you are using our hash, this is given by the hash_init() function*/
						 /* Don't know */
POSITION gInitialPosition     =  0; /* The initial hashed position for your starting board: 0 in this game */
POSITION kBadPosition         = -1; /* A position that will never be used */

/* 
 * Help strings that are pretty self-explanatory
 * Strings than span more than one line should have backslashes (\) at the end of the line.
 */

STRING kHelpGraphicInterface ="Not written yet";

STRING   kHelpTextInterface=""; 

STRING   kHelpOnYourTurn ="";

STRING   kHelpStandardObjective ="";

STRING   kHelpReverseObjective ="";

STRING   kHelpTieOccursWhen ="the board is full.";

STRING   kHelpExample ="";

/*************************************************************************
**
** #defines and structs
**
**************************************************************************/

#define FIRST_TURN 1
#define SECOND_TURN 2

/*************************************************************************
**
** Global Variables
**
*************************************************************************/

typedef struct {
  short *squares;
  int squaresOccupied;
  int currentTurn;
} SDBoard, *SDBPtr;

/* variants */
int BOARD_WIDTH = DEFAULT_BOARD_WIDTH, BOARD_HEIGHT =DEFAULT_BOARD_HEIGHT;
int     BOARD_ROWS          = DEFAULT_BOARD_HEIGHT;
int     BOARD_COLS          = DEFAULT_BOARD_WIDTH;
BOOLEAN bCanWinByColor = TRUE;
BOOLEAN bCanWinByUD = TRUE; 

/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/

BOOLEAN isSquareWin(short slot1, short slot2, short slot3, short slot4);
int boardIndex(int x, int y);

/* Square */
int hashSquare(int ud, int color);
int getSquare(SDBPtr board, int x, int y);
BOOLEAN isSquareEmpty(SDBPtr board, int x, int y);
int getSquareColor(SDBPtr board, int x, int y);
int getSquareUD(SDBPtr board, int x, int y);
int setSquare(SDBPtr board, int x, int y, int value);
int setSquareEmpty(SDBPtr board, int x, int y);
int setSquareColor(SDBPtr board, int x, int y, int color);
int setSquareUD(SDBPtr board, int x, int y, int ud);

/* Move */
MOVE hashMove(int x, int y, int ud);
int unhashMoveToX(MOVE move);
int unhashMoveToY(MOVE move);
int unhashMoveToUD(MOVE move);
//int unhashMoveToColor(MOVE move);

/* Board */
POSITION hashBoard(SDBPtr board);
SDBPtr unhashBoard(POSITION position);
// unhashBoard will create a board, need to call freeBoard in the end
void freeBoard(SDBPtr board); // need to call this as soon as we dont need the board
BOOLEAN isEmpty(SDBPtr board, int x, int y);

/************************************************************************
**
** NAME:        InitializeGame
**
** DESCRIPTION: Prepares the game for execution.
**              Initializes required variables.
**              Sets up gDatabase (if necessary).
** 
************************************************************************/

void InitializeGame ()
{
    
}

/************************************************************************
**
** NAME:        GenerateMoves
**
** DESCRIPTION: Creates a linked list of every move that can be reached
**              from this position. Returns a pointer to the head of the
**              linked list.
** 
** INPUTS:      POSITION position : Current position for move
**                                  generation.
**
** OUTPUTS:     (MOVELIST *)      : A pointer to the first item of
**                                  the linked list of generated moves
**
** CALLS:       MOVELIST *CreateMovelistNode();
**
************************************************************************/
/*------------------- implementation ------------------------*/
/* MOVE format: move_x, move_y, move_ud, move_color */
#ifndef SQUAREDANCEDEBUG
MOVELIST *GenerateMoves (POSITION position)
{
	int x, y, valid_color;
	MOVELIST *moves = NULL;
	SDBPtr board = unhashBoard(position);
	valid_color = getCurrentTurn(board);
	
	for(x=0;x<BOARD_WIDTH;x++) {
		for(y=0;y<BOARD_HEIGHT;y++) {
			if(isSquareEmpty(board,x,y)) { // for each empty slots
				moves = CreateMovelistNode(hashMove(x,y,UP),moves);
				moves = CreateMovelistNode(hashMove(x,y,DOWN),moves);
			}
		}
	}
	return moves;
}
#endif

/************************************************************************
**
** NAME:        DoMove
**
** DESCRIPTION: Applies the move to the position.
** 
** INPUTS:      POSITION position : The old position
**              MOVE     move     : The move to apply to the position
**
** OUTPUTS:     (POSITION)        : The position that results from move
**
** CALLS:       Some Board Hash Function
**              Some Board Unhash Function
**
*************************************************************************/

POSITION DoMove(POSITION position, MOVE move ) {
  /* unhash move */
  int move_x = unhashMoveToX(move);
  int move_y = unhashMoveToY(move);
  int move_ud = unhashMoveToUD(move);
  int move_color = getCurrentTurn(position);
  /* unhash board */
  SDBPtr board = unhashBoard(position);
  


  board->squares[boardIndex(move_x, move_y)] = (move_color << 1) + move_ud + 1;
  board->squaresOccupied++;
  board->currentTurn = !board->currentTurn;

  POSITION hash = hashBoard(board);
  freeBoard(board);
  return hash;
}

/************************************************************************
**
** NAME:        Primitive
**
** DESCRIPTION: Returns the value of a position if it fulfills certain
**              'primitive' constraints.
**
**              Example: Tic-tac-toe - Last piece already placed
**
**              Case                                  Return Value
**              *********************************************************
**              Current player sees three in a row    lose
**              Entire board filled                   tie
**              All other cases                       undecided
** 
** INPUTS:      POSITION position : The position to inspect.
**
** OUTPUTS:     (VALUE)           : one of
**                                  (win, lose, tie, undecided)
**
** CALLS:       None              
**
************************************************************************/

/* determine the result of the game */
VALUE Primitive (POSITION position) { //(POSITION position) {
  SDBPtr board = unhashBoard(position);
  int x1, y1, x2, y2, w, slot1, slot2, slot3, slot4;
  for(x1=0;x1<BOARD_WIDTH-1;x1++) {
    for(y1=0;y1<BOARD_HEIGHT-1;y1++) {
      slot1 = board->squares[y1*BOARD_WIDTH+x1];
      for(w=1;x1+w<BOARD_WIDTH && y1+w<BOARD_HEIGHT;w++) {
	x2 = x1+w;
	y2 = y1+w;
	slot2 = board->squares[boardIndex(x2,y1)];
	slot3 = board->squares[boardIndex(x1,y2)];
	slot4 = board->squares[boardIndex(x2,y2)];
	if(isSquareWin(slot1,slot2,slot3,slot4))
	  return gStandardGame ? lose : win;
      }
    }
  }
  
  //check for squares rotated 90 degree
  int xc, yc;
  for(xc=1;xc<BOARD_WIDTH-1;xc++) {
    for(yc=1;yc<BOARD_HEIGHT-1;yc++) {
      for(w=1; (x1=xc-w)>=0 && (x2=xc+w)<=BOARD_WIDTH-1 && (y1=yc-w)>=0 && (y2=yc+w)<=BOARD_HEIGHT-1; w++) {
    	slot1 = board->squares[y1*BOARD_WIDTH+xc];
    	slot2 = board->squares[y2*BOARD_WIDTH+xc];
    	slot3 = board->squares[yc*BOARD_WIDTH+x1];
    	slot4 = board->squares[yc*BOARD_WIDTH+x2];
      }
      if(isSquareWin(slot1,slot2,slot3,slot4)) {
	 return gStandardGame ? lose : win;
      }
    }
  }
  
  //hard code for 4x4 board
  if(BOARD_WIDTH==4 && BOARD_HEIGHT==4){
    slot1 = board->squares[0*BOARD_WIDTH+1];
    slot2 = board->squares[1*BOARD_WIDTH+3];
    slot3 = board->squares[3*BOARD_WIDTH+2];
    slot4 = board->squares[2*BOARD_WIDTH+0];
    if(isSquareWin(slot1,slot2,slot3,slot4)) return win;
    slot1 = board->squares[0*BOARD_WIDTH+2];
    slot2 = board->squares[1*BOARD_WIDTH+0];
    slot3 = board->squares[2*BOARD_WIDTH+3];
    slot4 = board->squares[3*BOARD_WIDTH+1];
    if(isSquareWin(slot1,slot2,slot3,slot4)) return win;
  }
  
  if(board->squaresOccupied==BOARD_WIDTH*BOARD_HEIGHT)
    return tie;
  else
    return undecided;
  
}

/************************************************************************
**
** NAME:        PrintPosition
**
** DESCRIPTION: Prints the position in a pretty format, including the
**              prediction of the game's outcome.
** 
** INPUTS:      POSITION position    : The position to pretty print.
**              STRING   playersName : The name of the player.
**              BOOLEAN  usersTurn   : TRUE <==> it's a user's turn.
**
** CALLS:       Unhash()
**              GetPrediction()      : Returns the prediction of the game
**
************************************************************************/
void PrintPosition (POSITION position, STRING playersName, BOOLEAN usersTurn )
{
  int x, y;
  SDBPtr board = unhashBoard(position);
  
  printf("==================================\n");
  printf("|    ");
  for(x=0;x<BOARD_WIDTH;x++)
    printf("---");
  printf("-\n");
  
  for(y=BOARD_HEIGHT-1;y>=0;y--) {
    printf("|  %d ", y);
    for(x=0;x<BOARD_WIDTH;x++) {
      printf("|");
      if(isSquareEmpty(board,x,y))
		  printf("  ");
      else {
		char colorToPrint = (getSquareColor(board,x,y) == YELLOW) ? 'Y' : 'B';
		char udToPrint = (getSquareUD(board,x,y) == UP) ? 'U' : 'D';
		printf("%c%c",colorToPrint,udToPrint);
		}
    }
    // format between lines
    if(y!=0) {
      printf("|\n|    |");
      for(x=0;x<BOARD_WIDTH;x++) {
	printf("--+");
      }
      printf("\b|\n");
    }
  }
  
  printf("|\n|    ");
  for(x=0;x<BOARD_WIDTH;x++)
    printf("---");
  printf("-\n|     ");
  for(x=0;x<BOARD_WIDTH;x++)
    printf("  %d",x);
  printf("\n");
  printf("| %s's (%s) turn now.\n",playersName
										,(board->currentTurn==YELLOW ? "yellow" : "blue"));
  printf("==================================\n");
  freeBoard(board);
  
}

/************* Other helper functions ******************/

/* for debugging
void printBinary(int x){
  int i;
  for(i=0;i<32;i++,x=x>>1)
    printf("%d",x%2);
       printf("\n");
}
*/

/************************************************************************
** Utility functions Implementation. Private to this file.
*************************************************************************/

/* Square */
int boardIndex(int x, int y) { return y*BOARD_WIDTH+x; }
int getSquare(SDBPtr board, int x, int y) { return board->squares[boardIndex(x,y)]; }
BOOLEAN isSquareEmpty(SDBPtr board, int x, int y) { return getSquare(board,x,y)==0; }
int getSquareColor(SDBPtr board, int x, int y) { return getSquare(board,x,y)>=3; }
int getSquareUD(SDBPtr board, int x, int y) { return (getSquare(board,x,y)&1)==0; } //Joey made a change
int setSquare(SDBPtr board, int x, int y, int value) { board->squares[boardIndex(x,y)] = value; }
int setSquareEmpty(SDBPtr board, int x, int y) { setSquare(board,x,y,EMPTY); }
/*
int hashSquare(int ud, int color) { return (color<<1)+ud; }
int setSquareColor(SDBPtr board, int x, int y, int color) {
	setSquare(board,x,y,hashSquare(getSquareUD(board,x,y),color)); 
}
int setSquareUD(SDBPtr board, int x, int y, int ud) {
	setSquare(board,x,y,hashSquare(ud,getSquareColor(board,x,y)));
}
*/

/* Move */
MOVE hashMove(int x, int y, int ud) { return ((x << 24) | (y << 16) | (ud << 8)); }
int unhashMoveToX(MOVE move)  { return move >> 24; }
int unhashMoveToY(MOVE move)  { return (move >> 16) & 0xff; }
int unhashMoveToUD(MOVE move) { return (move >> 8) &  0xff; }
//int unhashMoveToColor(MOVE move) { return move & 0xff; }

/* Board */

/*************************************************
** hashBoard
** note: use the general hash function for now
**       but limited to 3x4, 4x3
***************************************************/
POSITION hashBoard(SDBPtr board) {
	POSITION hash = 0;
	int base = POSSIBLE_SQUARE_VALUES;
	int x, y;
	for(y=0;y<BOARD_HEIGHT;y++)
		for(x=0;x<BOARD_WIDTH;x++)
			hash = hash * base + getSquare(board,x,y);
	hash = (hash << 1) + board->currentTurn; // last bit stores the current turn
	return hash;
}

/*************************************************
** unhashBoard
** note: use the general hash function for now
**       but limited to 3x4, 4x3
***************************************************/
SDBPtr unhashBoard(POSITION position) {
    
	SDBPtr board = (SDBPtr) malloc(sizeof(SDBoard));
	board->squares = (short*) malloc(sizeof(BOARD_WIDTH*BOARD_HEIGHT));
	
	int x, y, squareValue;
	int base = POSSIBLE_SQUARE_VALUES;
	
	board->squaresOccupied = 0;
	board->currentTurn = position & 1; // last bit
	
    position = position >> 1;
	
	for(y=BOARD_HEIGHT-1; y>=0; y--) {
		for(x=BOARD_WIDTH-1; x>=0; x--) {
			squareValue = position % base;
			setSquare(board,x,y,squareValue);
			if(squareValue!=EMPTY)
				board->squaresOccupied ++;
			position /= base;			
		}
	}
	
	return board;
}

/* return the color of the current turn */
int getCurrentTurn(POSITION position) {
	return position & 1; // the last bit
}

/* free the memory space for the board
  called when board is not needed anymore */
void freeBoard(SDBPtr board) {
  free(board->squares);
  free(board);
}

/********** helper function for Primitive **************/
BOOLEAN isSquareWin(short slot1, short slot2, short slot3, short slot4) {
  short slots[4]={slot1,slot2,slot3,slot4}, colors[4], uds[4], colorMatch=1, udMatch=1;
  int i;

  if(slot1==0  || slot2==0 || slot3==0 || slot4==0) {
    return FALSE;
  }
  
  for(i=0;i<4;i++) {
    colors[i] = (slots[i]-1) >> 1;
    uds[i]=(slots[i]-1) & 1;
  }
  
  for(i=1;i<4;i++) {  
     if(colors[i]==colors[0]) colorMatch++;
     if(uds[i]==uds[0]) udMatch++;
   }
   return ((bCanWinByColor && colorMatch==4) || (bCanWinByUD && udMatch==4));
   
}



/******************* MAIN ***********************/
#ifdef SQUAREDANCEDEBUG
int main() {//int argc, char [] argv) {
  int i;
  POSITION position=gInitialPosition;
  BOOLEAN turn = TRUE; // user first
    
  STRING playerName = "User Name";
  STRING computerName = "Computer Name";
  
  position = 0;
  PrintPosition(position, playerName, turn);
  
  for(i=0;i<4;i++) {
    switch(i) {
       case 0: position = DoMove(position, hashMove(0,0, UP));  break;
  	   case 1: position = DoMove(position, hashMove(0,BOARD_HEIGHT-1, UP)); break;
  	   case 2: position = DoMove(position, hashMove(BOARD_WIDTH-1,0, UP)); break;
  	   case 3: position = DoMove(position, hashMove(BOARD_WIDTH-1,BOARD_HEIGHT-1, DOWN)); break;
	     // default: 
  }
    turn = ! turn;
    PrintPosition(position, (turn ? playerName : computerName), turn);
  }
//  printf("%d",position);
  //printf("Primitive = %d\n",Primitive(board));
  getchar();
  return 0;
    
}
#endif


/************************************************************************
**
** NAME:        PrintMove
**
** DESCRIPTION: Prints the move in a nice format.
** 
** INPUTS:      MOVE move         : The move to print. 
**
************************************************************************/


void PrintMove(MOVE move)
{
  int col;
  int row;
  char UD;
  col = unhashMoveToX(move);
  row = unhashMoveToY(move);
  if(unhashMoveToUD(move)==UP)
    {
      UD='u';
    }
  else
    {
      UD='d';
    }
  printf("%d%d%c", col, row, UD);
  //printf("%d%d%c", unhashMoveToX, unhashMoveToY, unhashMoveToUD);
}



/************************************************************************
**
** NAME:        getOption
**
** DESCRIPTION: A hash function that returns a number corresponding
**              to the current variant of the game.
**              Each set of variants needs to have a different number.
**
** OUTPUTS:     int : the number representation of the options.
**
************************************************************************/

int getOption ()
{
    /* If you have implemented symmetries you should
       include the boolean variable gSymmetries in your
       hash */
    return 0;
}


/************************************************************************
**
** NAME:        NumberOfOptions
**
** DESCRIPTION: Calculates and returns the number of variants
**              your game supports.
**
** OUTPUTS:     int : Number of Game Variants
**
************************************************************************/

int NumberOfOptions ()
{
    return 1;
}


/************************************************************************
**
** NAME:        GameSpecificMenu
**
** DESCRIPTION: Prints, receives, and sets game-specific parameters.
**
**              Examples
**              Board Size, Board Type
**
**              If kGameSpecificMenu == FALSE
**                   Gamesman will not enable GameSpecificMenu
**                   Gamesman will not call this function
** 
**              Resets gNumberOfPositions if necessary
**
************************************************************************/

void GameSpecificMenu ()
{
    
}


/************************************************************************
**
** NAME:        setOption
**
** DESCRIPTION: The corresponding unhash function for game variants.
**              Unhashes option and sets the necessary variants.
**
** INPUT:       int option : the number representation of the options.
**
************************************************************************/

void setOption (int option)
{
    /* If you have implemented symmetries you should
       include the boolean variable gSymmetries in your
       hash */
}


/************************************************************************
**
** NAME:        DebugMenu
**
** DESCRIPTION: Game Specific Debug Menu (Gamesman comes with a default
**              debug menu). Menu used to debug internal problems.
**
**              If kDebugMenu == FALSE
**                   Gamesman will not display a debug menu option
**                   Gamesman will not call this function
** 
************************************************************************/

void DebugMenu ()
{
    
}


/************************************************************************
**
** NAME:        ValidTextInput
**
** DESCRIPTION: Rudimentary check to check if input is in the move form
**              you are expecting. Does not check if it is a valid move.
**              Only checks if it fits the move form.
**
**              Reserved Input Characters - DO NOT USE THESE ONE CHARACTER
**                                          COMMANDS IN YOUR GAME
**              ?, s, u, r, h, a, c, q
**                                          However, something like a3
**                                          is okay.
** 
**              Example: Tic-tac-toe Move Format : Integer from 1 to 9
**                       Only integers between 1 to 9 are accepted
**                       regardless of board position.
**                       Moves will be checked by the core.
**
** INPUTS:      STRING input : The string input the user typed.
**
** OUTPUTS:     BOOLEAN      : TRUE if the input is a valid text input.
**
************************************************************************/

BOOLEAN ValidTextInput (STRING input)
{
  int i;
  if (strlen(input) != 3)
    return FALSE;
  //if ((input[0] < COL_START) || (input[0] > (COL_START+BOARD_ROWS)) ||
  //  (input[1] < ROW_START) || (input[1] > ROW_START+BOARD_COLS))
  // return FALSE;
  if ((input[2] != 'u') && (input[2] != 'd') && (input[2] != 'U') && input[2] != 'D')
    return FALSE;
  return TRUE;
}

/************************************************************************
**
** NAME:        ConvertTextInputToMove
**
** DESCRIPTION: Converts the string input your internal move representation.
**              Gamesman already checked the move with ValidTextInput
**              and ValidMove.
** 
** INPUTS:      STRING input : The VALID string input from the user.
**
** OUTPUTS:     MOVE         : Move converted from user input.
**
************************************************************************/

MOVE ConvertTextInputToMove (STRING input)
{
  int row, col, updown;

  col = input[0] - ROW_START;
  row = input[1] - COL_START;
  if(input[2]=='u' || input[2]=='U')
    {
      updown = UP;
    }
  else
    {
      updown = DOWN;
    }
  PrintMove(hashMove (col, row, updown));
  return hashMove (col, row, updown);
}


/************************************************************************
**
** NAME:        GetAndPrintPlayersMove
**
** DESCRIPTION: Finds out if the player wishes to undo, abort, or use
**              some other gamesman option. The gamesman core does
**              most of the work here. 
**
** INPUTS:      POSITION position    : Current position
**              MOVE     *move       : The move to fill with user's move. 
**              STRING   playersName : Current Player's Name
**
** OUTPUTS:     USERINPUT          : One of
**                                   (Undo, Abort, Continue)
**
** CALLS:       USERINPUT HandleDefaultTextInput(POSITION, MOVE*, STRING)
**                                 : Gamesman Core Input Handling
**
************************************************************************/
#ifndef SQUAREDANCEDEBUG
USERINPUT GetAndPrintPlayersMove (POSITION position, MOVE *move, STRING playersName)
{
  USERINPUT input;
  USERINPUT HandleDefaultTextInput();
  char player_char = (getCurrentTurn(position) == YELLOW)?YELLOW:BLUE;
    
  for (;;) {
    /***********************************************************
     * CHANGE THE LINE BELOW TO MATCH YOUR MOVE FORMAT
     ***********************************************************/
    printf("%8s's (%c) move [<col><row><u/d>] : ", playersName, player_char);
	
    input = HandleDefaultTextInput(position, move, playersName);
	
    if (input != Continue)
      return input;
  }
    
  /* NOTREACHED */
  return Continue;
}
#endif

/************************************************************************
**
** NAME:        PrintComputersMove
**
** DESCRIPTION: Nicely formats the computers move.
** 
** INPUTS:      MOVE    computersMove : The computer's move. 
**              STRING  computersName : The computer's name. 
**
************************************************************************/

void PrintComputersMove (MOVE computersMove, STRING computersName)
{
  /* int position = Unhasher_Index(computersMove);
  int direction = Unhasher_Direction(computersMove);
    
  printf ("%s moves the piece %c%d%s\n", computersName, \
	  Column(position)+ROW_START, BOARD_ROWS-Row (position), \
	  directions[direction]);*/
}

/************************************************************************
**
** NAME:        GetInitialPosition
**
** DESCRIPTION: Called when the user wishes to change the initial
**              position. Asks the user for an initial position.
**              Sets new user defined gInitialPosition and resets
**              gNumberOfPositions if necessary
** 
** OUTPUTS:     POSITION : New Initial Position
**
************************************************************************/

POSITION GetInitialPosition ()
{
    return gInitialPosition;
}
