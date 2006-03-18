// $Id: mquickchess.c,v 1.4 2006-03-18 04:16:10 runner139 Exp $

/*
 * The above lines will include the name and log of the last person
 * to commit this file to CVS
 */

/************************************************************************
**
** NAME:        mquickchess.c
**
** DESCRIPTION: Quick Chess
**
** AUTHOR:      Adam Abed, Aaron Levitan
**
** DATE:        Start Spring 2006
**
** UPDATE HIST: 3/11/06 - updated fields
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

/*************************************************************************
**
** Game-specific constants
**
**************************************************************************/

STRING   kGameName            = "Quick Chess"; /* The name of your game */
STRING   kAuthorName          = "Aaron Levitan, Adam Abed"; /* Your name(s) */
STRING   kDBName              = "quickchess"; /* The name to store the database under */

BOOLEAN  kPartizan            = TRUE ; /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN  kGameSpecificMenu    = TRUE ; /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN  kTieIsPossible       = TRUE ; /* TRUE if a tie is possible. FALSE if it is impossible.*/
BOOLEAN  kLoopy               = TRUE ; /* TRUE if the game tree will have cycles (a rearranger style game). FALSE if it does not.*/

BOOLEAN  kDebugMenu           = TRUE ; /* TRUE only when debugging. FALSE when on release. */
BOOLEAN  kDebugDetermineValue = FALSE ; /* TRUE only when debugging. FALSE when on release. */

POSITION gNumberOfPositions   =  0; /* The number of total possible positions | If you are using our hash, this is given by the hash_init() function*/
POSITION gInitialPosition     =  0; /* The initial hashed position for your starting board */
POSITION kBadPosition         = -1; /* A position that will never be used */

void*	 gGameSpecificTclInit = NULL;

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
#define rows 6
#define cols 5 
#define WHITE_TURN 1 
#define BLACK_TURN 0 
#define BLACK_PAWN 'p'
#define BLACK_BISHOP 'b'
#define BLACK_ROOK 'r'
#define BLACK_KNIGHT 'n'
#define BLACK_QUEEN 'q'
#define BLACK_KING 'k'
#define WHITE_PAWN 'P'
#define WHITE_BISHOP 'B'
#define WHITE_ROOK 'R'
#define WHITE_KNIGHT 'N'
#define WHITE_KING 'K'
#define WHITE_QUEEN 'Q'

// Constants specifying directions to "look" on the board 
#define UP 0 
#define DOWN 1 
#define LEFT 2 
#define RIGHT 3 
#define UL 4 
#define UR 5 
#define DL 6 
#define DR 7 
/*************************************************************************
**
** Global Variables
**
*************************************************************************/
char gameBoard[6][5];

/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/

/* External */
extern GENERIC_PTR	SafeMalloc ();
extern void		SafeFree ();
void PrintPosition (POSITION position, STRING playersName, BOOLEAN usersTurn);
void setupPieces(char (* Board)[6][5]);
/************************************************************************
**
** NAME:        InitializeGame
**
** DESCRIPTION: Prepares the game for execution.
**              Initializes required variables.
** 
************************************************************************/

void InitializeGame ()
{
  setupPieces(&gameBoard);
  // gNumberOfPositions = hash_init();
  // gInitialPosition = genericHash(gameBoard);
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
    MOVELIST *moves = NULL;
    /*int currentPlayer, i; 
  char piece; 
  char *boardArray; 
  boardArray = unhashBoard(position); 
  currentPlayer = getCurrTurn(position); 
    */
    /* Use CreateMovelistNode(move, next) to 'cons' together a linked list */
    
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
  char boardArray[6][5];
  char tempPiece;
  char s[4];

  //s = convertMovetoString(move);
  boardArray = unhashBoard(position);
  tempPiece = boardArray[rows-s[1]][s[0]];
  boardArray[rows - s[1]][s[0] - 97] = ' ';
  boardArray[rows - s[3]][s[2] - 97] = tempPiece;
  return hashBoard(boardArray);
 
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
     if (inCheck(position) && !kingCanMove(position)) { 
    // The king is checked and can't move 
    return (gStandardGame) ? lose : win; 
  } 
  else if (!inCheck(position) && !kingCanMove(position)) { 
    // King is not in check and can't move - Stalemate 
    return tie; 
  } else { 
    return undecided; 
  } 

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
  char boardArray[6][5];
  int x, y;
  boardArray = unhashBoard(position);

  printf("\n"); 
  for(x = 0; x < rows; x++){ 
    printf("    +"); 
    for(y = 0; y < cols; y++){ 
      printf("---+"); 
    } 
    printf("\n"); 
    printf("  %d |", rows - x); 
    for(y = 0; y < cols; y++){ 
      printf(" %c |", boardArray[x][y]); 
    } 
    printf("\n"); 
  } 
  printf("    +"); 
  for(y = 0; y < cols; y++){ 
    printf("---+"); 
  } 
  printf("\n"); 
  printf("     "); 
  for(y = 0; y < cols; y++){ 
    printf(" %c  ", 97+y); 
  } 
  printf("\n"); 
  printf("%s\n",GetPrediction(position,playersName,usersTurn)); 
  printf("It is %s's turn (%s).\n",playersName,(usersTurn) ? "white/uppercase":"black/lowercase"); 
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
	printf("%8s's move [(undo)/(MOVE FORMAT)] : ", playersName);
	
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
    return FALSE;
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
    return 0;
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
    return 0;
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
** Everything specific to this module goes below these lines.
**
** Things you want down here:
** Move Hasher
** Move Unhasher
** Any other function you deem necessary to help the ones above.
** 
************************************************************************/


void setupPieces(char (* Board)[6][5]) {
  int x, y;
  /* setup empty spaces */
  for(x = 0; x < rows; x++ ){
    for(y = 0; y < cols; y++) {
     (* Board)[x][y] = ' ';
    }
  }
   /* setup pawns */
  for(y = 0; y < cols; y++ ){
    (* Board)[1][y] = WHITE_PAWN;
    (* Board)[4][y] = BLACK_PAWN;
  }
  /* setup black major pieces */
  (* Board)[5][0] = BLACK_ROOK;
  (* Board)[5][1] = BLACK_BISHOP;
  (* Board)[5][2] = BLACK_KING;
  (* Board)[5][3] = BLACK_QUEEN;
  (* Board)[5][4] = BLACK_KNIGHT;

  /* setup white major pieces */
  (* Board)[0][0] = WHITE_ROOK;
  (* Board)[0][1] = WHITE_BISHOP;
  (* Board)[0][2] = WHITE_KING;
  (* Board)[0][3] = WHITE_QUEEN;
  (* Board)[0][4] = WHITE_KNIGHT;
}

BOOLEAN kingCanMove(POSITION position) {
  
return FALSE;
}
BOOLEAN inCheck(POSITION N, int currentPlayer) { 
  int i, j; 
  char piece;   
  char bA[6][5] = unhashBoard(N);
  for (i = 0; i < rows; i++) {
    for(j = 0; j < cols; j++) {
      piece = bA[i][j]; 
	switch (piece) { 
	case WHITE_QUEEN: case BLACK_QUEEN:  
	  if (queenCheck(&bA, i, j, currentPlayer,piece, WHITE_QUEEN , BLACK_QUEEN) == TRUE) { 
	    return TRUE; 
	  } else { 
	    break;    				 
	  } 
	case WHITE_BISHOP: case BLACK_BISHOP: 
	  if (bishopCheck(&bA, i,j, currentPlayer, piece, WHITE_BISHOP , BLACK_BISHOP) == TRUE) { 
	    return TRUE; 
	  } else { 
	    break; 
	  } 
	case WHITE_ROOK: case BLACK_ROOK:
	  if (rookCheck(&bA, i, j, currentPlayer, piece, WHITE_ROOK , BLACK_ROOK) == TRUE) { 
	    return TRUE; 
	  } else { 
	    break; 
	  } 
	case WHITE_KNIGHT: case BLACK_KNIGHT:
	  if (knightCheck(&bA, i, j, currentPlayer, piece, WHITE_KNIGHT , BLACK_KNIGHT) == TRUE) { 
	    return TRUE; 
	  } else { 
	    break; 
	  } 
	case WHITE_PAWN: case BLACK_PAWN:
	   if (pawnCheck(&bA, i,j, currentPlayer, piece, WHITE_PAWN , BLACK_PAWN) == TRUE) { 
	    return TRUE; 
	  } else { 
	    break; 
	  } 
	default:  
	  break; 
	} 
      } 
    } 
    return FALSE; 
} 

BOOLEAN isKingCapture(char (* Board)[6][5], int row, int col, int currentPlayer, 
		      char currentPiece, char whitePiece, char blackPiece) {
  if(currentPlayer == WHITE_TURN) {
      if(currentPiece == blackPiece) {
	if((* Board)[row][col] == WHITE_KING) {
	  return TRUE;
	} else if((* Board)[row][col] != ' ') {
	  break;
	}
      }
    } else {
      if(currentPiece == whitePiece) {
	if((* Board)[row][col] == BLACK_KING) {
	  return TRUE;
	} else if((* Board)[row][col] != ' ') {
	  break;
	}
      }
    }
  return FALSE;
}
BOOLEAN queenCheck(char (* Board)[6][5], int row, int col, int currentPlayer, char currentPiece, char whitePiece, char blackPiece) {
return (bishopCheck(Board, row, col, currentPlayer, currentPiece, whitePiece, blackPiece) || rookCheck(Board, row, col, currentPlayer, currentPiece, whitePiece, blackPiece));
}

BOOLEAN bishopCheck(char (* Board)[6][5], int row, int col, int currentPlayer, char currentPiece, char whitePiece, char blackPiece) {
  int rowTemp, colTemp;
   
  rowTemp = row;
  colTemp = col;
  // up and left
  while(row-1 >= 0 && col-1 >= 0) {
    row--;
    col--;
    if(isKingCapture(Board, row, col, currentPlayer, currentPiece, whitePiece, blackPiece)){
      return TRUE;
    }
  }
  row = rowTemp;
  col = colTemp; 
  // up and right
  while(row-1 >= 0 && col+1 < cols) {
    row--;
    col++;
    if(isKingCapture(Board, row, col, currentPlayer, currentPiece, whitePiece, blackPiece)){
      return TRUE;
    }
  }
  row = rowTemp;
  col = colTemp;
  // down and left
  while(row+1 < rows && col-1 >= 0) {
    row++;
    col--;
    if(isKingCapture(Board, row, col, currentPlayer, currentPiece, whitePiece, blackPiece)) {
      return TRUE;
    }
  }
  row = rowTemp;
  col = colTemp;
  // down and right
  while(row+1 < rows && col+1 < cols) {
    row++;
    col++;
    if(isKingCapture(Board, row, col, currentPlayer, currentPiece, whitePiece, blackPiece)){
      return TRUE;
    }

  }
  return FALSE;
}

BOOLEAN knightCheck(char (* Board)[6][5], int row, int col, int currentPlayer, char currentPiece, char whitePiece, char blackPiece) {
  int rowTemp, colTemp;

  rowTemp = row;
  colTemp = col;

  // up two left one
  if(row-2 >= 0 && col-1 >= 0) {
     if(isKingCapture(Board, row-2, col-1, currentPlayer, currentPiece, whitePiece, blackPiece)){
      return TRUE;
    }
  }
  // up two right one
  if(row-2 >= 0 && col+1 < cols) {
     if(isKingCapture(Board, row-2, col+1, currentPlayer, currentPiece, whitePiece, blackPiece)){
      return TRUE;
    }
  }
  // right two up one
  if(col+2 < cols && row-1 >= 0) {
     if(isKingCapture(Board, row-1, col+2, currentPlayer, currentPiece, whitePiece, blackPiece)){
      return TRUE;
    }
  }
  // right two down one
  if(col+2 < cols && row+1 < rows) {
     if(isKingCapture(Board, row+1, col+2, currentPlayer, currentPiece, whitePiece, blackPiece)){
      return TRUE;
    }
  }
  // down two left one
  if(row+2 < rows && col-1 >= 0) {
     if(isKingCapture(Board, row+2, col-1, currentPlayer, currentPiece, whitePiece, blackPiece)){
      return TRUE;
    }
  }
  // down two right one
  if(row+2 < rows && col+1 < cols) {
     if(isKingCapture(Board, row+2, col+1, currentPlayer, currentPiece, whitePiece, blackPiece)){
      return TRUE;
    }
  }
  // left two up one
  if(col-2 >= 0 && row-1 >= 0) {
     if(isKingCapture(Board, row-1, col-2, currentPlayer, currentPiece, whitePiece, blackPiece)){
      return TRUE;
    }
  }
  // left two down one
  while(col-2 >= 0 && row+1 < rows) {
     if(isKingCapture(Board, row+1, col-2, currentPlayer, currentPiece, whitePiece, blackPiece)){
      return TRUE;
    }
  }
  return FALSE;
}

BOOLEAN rookCheck(char (* Board)[6][5], int row, int col, int currentPlayer, char currentPiece, char whitePiece, char blackPiece) {
  int rowTemp, colTemp;

  rowTemp = row;
  colTemp = col;
  // up 
  while(row-1 >= 0) {
    row--;
    if(isKingCapture(Board, row, col, currentPlayer, currentPiece, whitePiece, blackPiece)){
      return TRUE;
    }
  }
  row = rowTemp; 
  // right
  while(col+1 < cols) {
    col++;
    if(isKingCapture(Board, row, col, currentPlayer, currentPiece, whitePiece, blackPiece)){
      return TRUE;
    }
  }
  col = colTemp;
  // left
  while(col-1 >= 0) {
    col--;
    if(isKingCapture(Board, row, col, currentPlayer, currentPiece, whitePiece, blackPiece)) {
      return TRUE;
    }
  }

  // down 
  while(row+1 < rows) {
    row++;
    if(isKingCapture(Board, row, col, currentPlayer, currentPiece, whitePiece, blackPiece)) {
      return TRUE;
    }
  }
}
BOOLEAN pawnCheck(char (* Board)[6][5], int row, int col, int currentPlayer, char currentPiece, char whitePiece, char blackPiece) { 
  if(currentPlayer == WHITE_TURN) { 
    if(col-1 >= 0) {
      if(isKingCapture(Board, row-1, col-1, currentPlayer, currentPiece, whitePiece, blackPiece)) {
	return TRUE;
      }
    }
    if(col+1 < cols) {
      if(isKingCapture(Board, row-1, col+1, currentPlayer, currentPiece, whitePiece, blackPiece)) {
	return TRUE;
      }
    }
  } else {
    if(col-1 >= 0) {
      if(isKingCapture(Board, row+1, col-1, currentPlayer, currentPiece, whitePiece, blackPiece)) {
	return TRUE;
      }
    }
    if(col+1 < cols) {
      if(isKingCapture(Board, row+1, col+1, currentPlayer, currentPiece, whitePiece, blackPiece)) {
	return TRUE;
      }
    }
  }
  return FALSE;
}

BOOLEAN kingCanMove(POSITION position) {
  
  MOVELIST *moves = NULL;
  generateKingMoves(position, &moves);
  if(moves == NULL) 
    return FALSE;
  else return TRUE;
}
// $Log: not supported by cvs2svn $
// Revision 1.3  2006/03/12 03:56:42  vert84
// Updated description fields.
//
// Revision 1.2  2006/03/05 03:32:12  yanpeichen
// Yanpei Chen changing mquickchess.c
//
// Deleted a stray character that caused a file inclusion error.
//
// Revision 1.1  2006/03/01 01:49:21  vert84
// Added quickchess to cvs
//
// Revision 1.7  2006/01/29 09:59:47  ddgarcia
// Removed "gDatabase" reference from comment in InitializeGame
//
// Revision 1.6  2005/12/27 10:57:50  hevanm
// almost eliminated the existance of gDatabase in all files, with some declarations commented earlier that need to be hunt down and deleted from the source file.
//
// Revision 1.5  2005/10/06 03:06:11  hevanm
// Changed kDebugDetermineValue to be FALSE.
//
// Revision 1.4  2005/05/02 17:33:01  nizebulous
// mtemplate.c: Added a comment letting people know to include gSymmetries
// 	     in their getOption/setOption hash.
// mttc.c: Edited to handle conflicting types.  Created a PLAYER type for
//         gamesman.  mttc.c had a PLAYER type already, so I changed it.
// analysis.c: Changed initialization of option variable in analyze() to -1.
// db.c: Changed check in the getter functions (GetValueOfPosition and
//       getRemoteness) to check if gMenuMode is Evaluated.
// gameplay.c: Removed PlayAgainstComputer and PlayAgainstHuman.  Wrote PlayGame
//             which is a generic version of the two that uses to PLAYER's.
// gameplay.h: Created the necessary structs and types to have PLAYER's, both
// 	    Human and Computer to be sent in to the PlayGame function.
// gamesman.h: Really don't think I changed anything....
// globals.h: Also don't think I changed anything....both these I just looked at
//            and possibly made some format changes.
// textui.c: Redid the portion of the menu that allows you to choose opponents
// 	  and then play a game.  Added computer vs. computer play.  Also,
//           changed the analysis part of the menu so that analysis should
// 	  work properly with symmetries (if it is in getOption/setOption hash).
//
// Revision 1.3  2005/03/10 02:06:47  ogren
// Capitalized CVS keywords, moved Log to the bottom of the file - Elmer
//
