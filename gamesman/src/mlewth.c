// $id$
// $log$


/*
 * The above lines will include the name and log of the last person
 * to commit this file to CVS
 */

/************************************************************************
**
** NAME:        mlewthwaite.c
**
** DESCRIPTION: Lewthwaite's Game
**
** AUTHOR:      Cindy Song, Yuliya Sarkisyan
**
** DATE:        Feb 17, 2005
**
** UPDATE HIST: 2/19 - Cindy - Changed constants;
**			       Primitive and two helpers;
**			       PrintPosition
**              2/22 - Yuliya - DoMove and two Primitive helpers
**                              Added some constants
**                              Moved helpers to the bottom of file to the
**                                 space allocated for them
**                     NOTE: All functions are tentative until we come up
**                           with a final representation of position
**              2/25 - Cindy - Fixed PrintPosition
**			     - Fixed Primitive
**		       NOTE: Primitive will work with the variation of
**			     the game
**              2/26 - Yuliya - Changed game type to LOOPY
**                            - Adjusted constants a little
**                            - Did hash intialization in initializeGame()
**                            - Fixed DoMove to handle sliding more than
**                              one square at a time
**                            - Fixed GetSpace, GetPlayer and added one
**                              more helper
**                            - Wrote functions for hashing and unhashing
**                              moves
**		2/26 - Cindy - PrintPosition uses generic hash
**			     - Fixed DoMove minor errors
**			     - Add rule variation constant
**			     - Implemented GenerateMoves
**			     - Fixed and added helpers
**              2/27 - Yuliya - Added some more rule variation constants
**                              See comment
**                            - Finished implementing IntializeGame
**                            - Implemented PrintComputersMove, PrintMove
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
#include "hash.h"


/*************************************************************************
**
** Game-specific constants
**
**************************************************************************/

STRING   kGameName            = "Lewthwaite's Game"; /* The name of your game */
STRING   kAuthorName          = "Cindy Song, Yuliya Sarkisyan"; /* Your name(s) */
STRING   kDBName              = ""; /* The name to store the database under */

BOOLEAN  kPartizan            = TRUE ; /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN  kGameSpecificMenu    = TRUE ; /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN  kTieIsPossible       = FALSE ; /* TRUE if a tie is possible. FALSE if it is impossible.*/
BOOLEAN  kLoopy               = TRUE ; /* TRUE if the game tree will have cycles (a rearranger style game). FALSE if it does not.*/

BOOLEAN  kDebugMenu           = TRUE ; /* TRUE only when debugging. FALSE when on release. */
BOOLEAN  kDebugDetermineValue = TRUE ; /* TRUE only when debugging. FALSE when on release. */

POSITION gNumberOfPositions   =  0; /* The number of total possible positions | If you are using our hash, this is given by the hash_init() function*/
POSITION gInitialPosition     =  0; /* The initial hashed position for your starting board */
POSITION kBadPosition         = -1; /* A position that will never be used */

/*************************************************************************
**
** Start of game-specific variables
**
**************************************************************************/

/* constants for board size */
int boardWidth 		      = 5;
int boardHeight		      = 5;

int numBlackPieces            = 12;
int numWhitePieces            = 12;

/* other constants */

#define black  1   /* Has to be 0 or 1 */
#define white  2   /* Has to 1 - black */
#define space -1

#define up     0
#define down   1
#define left   2
#define right  3

BOOLEAN multiplePieceMoves     = FALSE;


/* Custom board allows the user to choose a rectangular board of any size,
any number of white and black pieces (as long as exactly one empty space
remains), and any starting configuration.  Should this option be chosen,
all three as well as who is to move first must be specified.  Another
option would be and odd x odd board with an even number of pieces for each
player and a symmetric configuration with the space in the center.  The
default will be this configuration with board size 5x5. */

BOOLEAN gCustomBoard          = FALSE;
int gFirstToMove              = black;
BOOLEAN positionInitialized   = FALSE;

BOOLEAN detailedDebug         = FALSE;

/*************************************************************************
**
** End of game-specific variables
**
**************************************************************************/

/*
 * Help strings that are pretty self-explanatory
 * Strings than span more than one line should have backslashes (\) at the end of the line.
 */

STRING kHelpGraphicInterface =
"Not written yet";

STRING   kHelpTextInterface    =
"";

STRING   kHelpOnYourTurn =
"";

STRING   kHelpStandardObjective =
"";

STRING   kHelpReverseObjective =
"";

STRING   kHelpTieOccursWhen =
"A tie occurs when ...";

STRING   kHelpExample =
"";


/*************************************************************************
**
** #defines and structs
**
**************************************************************************/


/*************************************************************************
**
** Global Variables
**
*************************************************************************/


/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/

/* Internal */
void setDefaultInitialPosition();
int GetSpace(POSITION);
int GetPlayer(POSITION, int, int, int);
int otherPlayer(int);
char getPiece(int);
int ModifyOffset(int, int);
int validCellToCheck(int, int, int);
MOVE hashMove(int, int);
int getDirection(MOVE);
int getNumPieces(MOVE);
MOVELIST* CreateMovelistNode(MOVE, MOVELIST*);


/* External */
extern GENERIC_PTR	SafeMalloc ();
extern void		SafeFree ();


/*************************************************************************
**
** Global Database Declaration
**
**************************************************************************/

extern VALUE     *gDatabase;


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
  int boardSize = boardWidth*boardHeight;
  int pieces[] = {'b', numBlackPieces, numBlackPieces,
		 'w', numWhitePieces, numWhitePieces,
		 '-', 1, 1,
		 -1};

  gNumberOfPositions =
    generic_hash_init(boardSize, pieces, NULL);

  if (!positionInitialized) {
    int i;
    char gBoard[boardSize];
 
    for (i=0; i<boardSize;i++) {
      if (i%2 == 0) {
	gBoard[i] = 'w';
      } else {
	gBoard[i] = 'b';
      }
    }
  
    if (gBoard[boardSize/2] == 'w') {
      gBoard[boardSize/2] = '-';
    } else {
      gBoard[boardSize/2 - boardWidth] = '-';
    }

    gInitialPosition = generic_hash(gBoard, gFirstToMove);
    positionInitialized = TRUE;
  }
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

MOVELIST *GenerateMoves (POSITION position)
{
  if (detailedDebug) printf("GenerateMoves start\n");
  MOVELIST *moves = NULL;
  int spaceIndex, dir, numToMove;
  int player = whoseMove(position);
  
 /* Use CreateMovelistNode(move, next) to 'cons' together a linked list */
  
  spaceIndex = GetSpace(position);

  for (dir=0; dir<4; dir++) {
    if (validCellToCheck(1,dir,spaceIndex) 
	&& GetPlayer(position, spaceIndex, 1, dir) == player){
      
      moves = CreateMovelistNode(hashMove(dir, 1), moves);        
    
      if (multiplePieceMoves) {
	numToMove = 2;
	while(validCellToCheck(numToMove,dir,spaceIndex)){
	  	  
	  if (GetPlayer(position,spaceIndex,numToMove,dir) == player) {
	    moves = CreateMovelistNode(hashMove(dir, numToMove), moves);
	  }
	  
	  numToMove = numToMove+1;
	}	    
      }
    }
  }
  if (detailedDebug) printf("GenerateMoves end\n");
  return moves;
}


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

POSITION DoMove (POSITION position, MOVE move)
{
  char posArray[boardWidth*boardHeight];
  generic_unhash(position, posArray);
  int whosMove = whoseMove(position);
  int empty_space = GetSpace(position);

  int direction = getDirection(move);
  int numPieces = getNumPieces(move);

  int i;
  if (direction == up) {
    for (i=0; i<numPieces; i++) {
      posArray[empty_space+boardWidth*i] = posArray[empty_space+boardWidth*(i+1)];
    }
    posArray[empty_space+boardWidth*i] = getPiece(space);
  } else if (direction == down) {
    for (i=0; i<numPieces; i++) {
      posArray[empty_space-boardWidth*i] = posArray[empty_space-boardWidth*(i+1)];
    }
    posArray[empty_space-boardWidth*numPieces] = getPiece(space);
  } else if (direction == left) {
    for (i=0; i<numPieces; i++) {
      posArray[empty_space+i] = posArray[empty_space+(i+1)];
    }
    posArray[empty_space+numPieces] = getPiece(space);
  } else if (direction == right) {
    for (i=0; i<numPieces; i++) {
      posArray[empty_space-i] = posArray[empty_space-(i+1)];
    }
    posArray[empty_space-numPieces] = getPiece(space);
  } else {
    //error("Invalid Move");
  }
  return generic_hash(posArray, otherPlayer(whosMove));

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

VALUE Primitive (POSITION position)
{
   int spaceIndex,dir;
   int player;
  
   spaceIndex = GetSpace(position);
   player = whoseMove(position);
  
   /* Current player loses when all 4 cells around the space
    * belong to the opponent
    */
   for (dir=0; dir<4; dir++){
     if (validCellToCheck(1, dir,spaceIndex)){
       if (GetPlayer(position, spaceIndex, 1, dir) == player){
	 return undecided;
       }
     }
   }
   return lose;
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

void PrintPosition (POSITION position, STRING playersName, BOOLEAN usersTurn)
{
  if (detailedDebug) printf("PrintPosition start\n");
  printf("\n");
  int i;
  STRING name = "";

  char board[boardWidth*boardHeight];
  generic_unhash(position, board);
  
  printf("\t\t");
  for (i=1; i<=boardHeight*boardWidth; i++){
    printf("%c ", board[i-1]);
    if (i%boardWidth == 0){
      printf("\n\t\t");
    }
  }
  
  if (usersTurn){
    name = playersName;
  } else {
    name = "Computer";
  }
  printf("\n%s: %s\n", name, GetPrediction(position, playersName, usersTurn));
  if (detailedDebug) printf("PrintPosition end\n");
  
}


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
  printf("%s has moved ", computersName);
  PrintMove(computersMove);
  printf("\n\n");
}


/************************************************************************
**
** NAME:        PrintMove
**
** DESCRIPTION: Prints the move in a nice format.
**
** INPUTS:      MOVE move         : The move to print.
**
************************************************************************/

void PrintMove (MOVE move)
{
  char* pieceString = "";
  char* dirString = "";
  int numPieces = getNumPieces(move);
  int direction = getDirection(move);

  if (numPieces == 1) {
    pieceString = "piece";
  } else {
    pieceString = "pieces";
  }

  if (direction == up) {
    dirString = "u";
  } else if (direction == down) {
    dirString = "d";
  } else if (direction == left) {
    dirString = "l";
  } else if (direction == right) {
    dirString = "r";
  }

  printf("%d%s", numPieces, dirString);
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

USERINPUT GetAndPrintPlayersMove (POSITION position, MOVE *move, STRING playersName)
{
  USERINPUT input;
  USERINPUT HandleDefaultTextInput();
  
  for (;;) {
    /***********************************************************
     * CHANGE THE LINE BELOW TO MATCH YOUR MOVE FORMAT
     ***********************************************************/
    printf("%8s's move [(u)ndo/(xy) : x = # pieces, y in {u, d, l, r}] : ", 
	   playersName);
    
    input = HandleDefaultTextInput(position, move, playersName);
    
    if (input != Continue)
      return input;
  }
  /* NOTREACHED */
  return Continue;
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
  char c = *input;
  int direction = 0;
  int numPieces = 0;
  
  if (c < '0' || c > '9') {
    return FALSE;
  }

  while ((c = *input) >= '0' && c <='9') {
    numPieces = (numPieces * 10) + (c - '0');
    input++;
  }

  if (c == 'u') {
    direction = up;
  } else if (c == 'd') {
    direction = down;
  } else if (c == 'l') {
    direction = left;
  } else if (c == 'r') {
    direction = right;
  } else {
    return FALSE;
  }
  return TRUE; //////////// NOT IMPLEMENTED!!!!!!!!!!!!1
  
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
  char c;
  int direction = 0;
  int numPieces = 0;
  
  while ((c = *input) >= '0' && c <='9') {
    numPieces = (numPieces * 10) + (c - '0');
    input++;
  }

  if (c == 'u') {
    direction = up;
  } else if (c == 'd') {
    direction = down;
  } else if (c == 'l') {
    direction = left;
  } else if (c == 'r') {
    direction = right;
  } else {
    //error("Invalid direction");
  }

  return hashMove(direction, numPieces);
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
  while (TRUE) {
    printf("\n\t----- Game-specific options for %s -----\n", kGameName);
    printf("\n\tBoard Options:\n\n");
    printf("\td)\tChange board (D)imension (%d,%d)\n", boardWidth, boardHeight);
    printf("\ti)\tChange (i)nitial position.  Current position is:\n");
    PrintPosition(gInitialPosition, "Player", TRUE);
    printf("\n\tRule Options:\n\n");
    printf("\tp)\tMultiple (p)iece moves (%s)\n", multiplePieceMoves ? "On" : "Off");
    printf("\n\n\tb)\t(B)ack = Return to previous activity.\n");
    printf("\n\nSelect an option: ");
    
    switch(GetMyChar()) {
    case 'Q': case 'q':
      ExitStageRight();

    case 'H': case 'h':
      HelpMenus();
      break;

    case 'D': case 'd':
      printf("\nPlease enter a new width (3 or 5): ");
      scanf("%d", &boardWidth);
      while (boardWidth != 3 && boardWidth != 5) {
	printf("Please enter a new width (3 or 5): ");
	scanf("%d", &boardWidth);
      }
      
      printf("Please enter a new height (3 or 5): ");
      scanf("%d", &boardHeight);
      while (boardHeight != 3 && boardHeight != 5) {
	printf("Please enter a new height (3 or 5): ");
	scanf("%d", &boardHeight);
      }	

      numWhitePieces = boardHeight*boardWidth/2;
      numBlackPieces = numWhitePieces;
      positionInitialized = FALSE;
      InitializeGame();
      break;

    case 'P': case 'p':
      multiplePieceMoves = !multiplePieceMoves;
      break;

    case 'I': case 'i':
      gInitialPosition = GetInitialPosition();
      positionInitialized = TRUE;
      break;

    case 'B': case 'b':
      return;

    default:
      BadMenuChoice();
      HitAnyKeyToContinue();
    }
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

void SetTclCGameSpecificOptions (int options[])
{

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
  int i,j, length;
  int numWhite = 0;
  int numBlack = 0;
  int boardSize = boardWidth*boardHeight;
  char row[boardSize],board[boardSize],c;
  BOOLEAN spaceExists = FALSE;

  i = 0;
  printf("\n");
  while(i < boardHeight){
    printf("Please enter row %d without any spaces: ", i+1);
    scanf("%s", row);
    
    length = strlen(row);

    if (length < boardWidth) {
      printf("\nToo few pieces.  Try again.\n");
      continue;
    } else if (length > boardWidth) {
      printf("\nToo many pieces.  Try again.\n");
      continue;
    }
    for (j=0;j<boardWidth;j++) {
      c = row[j];

      if (c == 'w') {
	numWhite++;
	board[j+i*boardWidth] = c;
	
      } else if (c == 'b') {
	numBlack++;
	board[j+i*boardWidth] = c;
      
      } else if (c == '-') {
	if (spaceExists) {
	  printf("\nThe board already contains a space.  Try again.\n");
	  continue;
	}
	spaceExists = TRUE;
	board[j+i*boardWidth] = c;

      } else {
	printf("\nUnknown piece %c.  Try again.\n", c);
	i--;
	break;
      }

    }
    i++;
  }

  if (!spaceExists) {
    printf("\nThe board you entered does not contain a space.  Try again.\n");
    return GetInitialPosition();
  }

  numWhitePieces = numWhite;
  numBlackPieces = numBlack;
  InitializeGame();
  return generic_hash(board, gFirstToMove);
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
  return 2 /* width */
    * 2 /* height */
    * 2 /* mulptiple peice moves */
    ;
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
    return 0;
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
** Everything specific to this module goes below these lines.
**
** Things you want down here:
** Move Hasher
** Move Unhasher
** Any other function you deem necessary to help the ones above.
**
************************************************************************/

/**********************************************************************
**
** NAME: 		GetSpace
**
** DESCRIPTION: Return the index of the cell which nobody occupies.
**
** INPUTS: 		POSITION position : The current board position
**
** CALLS:		None (Unhash?)
**
***********************************************************************/

int GetSpace(POSITION position) {
  char posArray[boardWidth*boardHeight];
  generic_unhash(position, posArray);
  int i;
  for (i=0;i<boardWidth*boardHeight;i++) {
    if (posArray[i] == '-') {
      return i;
    }
  }
  //error("Invalid position.");
  return -1;
}


/***********************************************************************
**
** NAME:		GetPlayer
**
** DESCRIPTION: Return which player is the owner of the given board cell
**
** INPUTS: 		POSITION position: The current board position
**			INT index: The index of the cell to inspect.
**
** CALLS: 		generic_unhash()
**
***********************************************************************/

int GetPlayer(POSITION position, int spaceIndex, int numAway, int dir) {
  char posArray[boardWidth*boardHeight];
  generic_unhash(position, posArray);

  int index;
  if (dir == down) {
    index = spaceIndex-numAway*boardWidth;
  } else if (dir == up) {
    index = spaceIndex+numAway*boardWidth;
  } else if (dir == left) {
    index = spaceIndex+numAway;
  } else {
    index = spaceIndex-numAway;
  }

  if (posArray[index] == 'b') {
    return black;
  } else if (posArray[index] == 'w') {
    return white;
  } else if (posArray[index] == '-') {
    return space;
  } else {
    //error("Invalid position.");
  }
  return -1;
}


int otherPlayer(int player) {
  if (player == white) {
    return black;
  }
  return white;
}


/***********************************************************************
**
** NAME:		getPiece
**
** DESCRIPTION: Return character representation of one of: black, white,
**              space.
**
** INPUTS: 	        int piece_or_player: numerical representation of
**                                           peice or player
**
** CALLS: 		None
**
***********************************************************************/

char getPiece(int piece_or_player) {
  if (piece_or_player == black) {
    return 'b';
  } else if (piece_or_player == white) {
    return 'w';
  } else if (piece_or_player == space) {
    return '-';
  } else {
    //error("Invalid piece or player.");
  }
  return -1;
}


int validCellToCheck(int numPieces, int dir, int indSpace){
  if (dir == down) {
    if (indSpace-numPieces*boardWidth < 0) 
      return 0;
  } else if (dir == up) { // bottom of space
    if (indSpace+numPieces*boardWidth >= boardWidth*boardHeight) 
      return 0;
  } else if (dir == right) { // left of space
    if ((indSpace-numPieces)/boardWidth != indSpace/boardWidth)
      return 0;
  } else if (dir == left) { // right of space
    if ((indSpace+numPieces)/boardWidth != indSpace/boardWidth)
     
      return 0;
  }

  return 1;
}


/*********************************************************************
 **
 **  START OF MOVE HASHING CODE
 **
 ********************************************************************/


/*********************************************************************
**
** NAME: 		hashMove
**
** DESCRIPTION: Hashes move to an int
**
** INPUTS: 		int direction: direction of move
**              int numPieces: number of pieces to be moved
**
** CALLS: 		None
**
*********************************************************************/

MOVE hashMove(int direction, int numPieces){
  int boardSize = boardWidth;
  if (boardSize < boardHeight) {
    boardSize = boardHeight;
  }
  return direction*(boardSize-1)+numPieces;
}

/*********************************************************************
**
** NAME: 		getDirection
**
** DESCRIPTION: Get direction from hashed move
**
** INPUTS: 		MOVE move
**
** CALLS: 		None
**
*********************************************************************/

int getDirection(MOVE move){
  int boardSize = boardWidth;
  if (boardSize < boardHeight) {
    boardSize = boardHeight;
  }
  return move/(boardSize-1);
}


/*********************************************************************
**
** NAME: 		getNumPieces
**
** DESCRIPTION: Get number of pieces to be moved from hashed move
**
** INPUTS: 		MOVE move
**
** CALLS: 		None
**
*********************************************************************/

int getNumPieces(MOVE move){
  int boardSize = boardWidth;
  if (boardSize < boardHeight) {
    boardSize = boardHeight;
  }
  return move%(boardSize-1);
}



/*********************************************************************
 **
 **  END OF MOVE HASHING CODE
 **
 ********************************************************************/
