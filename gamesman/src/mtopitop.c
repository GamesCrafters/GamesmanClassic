

/*
 * The above lines will include the name and log of the last person
 * to commit this file to CVS
 */

/************************************************************************
**
** NAME:        mtopitop.c
**
** DESCRIPTION: Topitop
**
** AUTHOR:      Mike Hamada, Alex Choy
**
** DATE:        BEGIN: 02/20/2006
**	              END: ???
**
** UPDATE HIST:
**	
**	    02/20/2006 - Setup #defines & data-structs
** 			         Wrote InitializeGame(), PrintPosition()
**      02/22/2006 - Added CharToBoardPiece(), arrayHash(), and arrayUnhash()
**                   Still need to edit above functions with these new fcts
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

STRING   kGameName            = "Topitop"; /* The name of your game */
STRING   kAuthorName          = "Alex Choy and Mike Hamada"; /* Your name(s) */
STRING   kDBName              = "topitop"; /* The name to store the database under */

BOOLEAN  kPartizan            = TRUE ; /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN  kGameSpecificMenu    = TRUE ; /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN  kTieIsPossible       = FALSE ; /* TRUE if a tie is possible. FALSE if it is impossible.*/
BOOLEAN  kLoopy               = TRUE ; /* TRUE if the game tree will have cycles (a rearranger style game). FALSE if it does not.*/

BOOLEAN  kDebugMenu           = TRUE ; /* TRUE only when debugging. FALSE when on release. */
BOOLEAN  kDebugDetermineValue = FALSE ; /* TRUE only when debugging. FALSE when on release. */

POSITION gNumberOfPositions   =  2000000000; /* The number of total possible positions | If you are using our hash, this is given by the hash_init() function*/
											 /* 10^9 * 2 = 2,000,000,000 */
POSITION gInitialPosition     =  0; /* The initial hashed position for your starting board */
POSITION kBadPosition         = -1; /* A position that will never be used */

void*	 gGameSpecificTclInit = NULL;

/* 
 * Help strings that are pretty self-explanatory
 * Strings than span more than one line should have backslashes (\) at the end of the line.
 */

STRING kHelpGraphicInterface =
"Not written yet.";

STRING   kHelpTextInterface    =
"VALID MOVES:\n\
1.) Place 1 of your Buckets or 1 of the Small or Big Sand Piles\n\
    on any free space of the board.\n\
2.) Move 1 of your Buckets or any Small or Big Sand Pile already\n\
    placed on the board from one square to another (one space at\n\
    a time in any direction).\n\
       a.) Any piece already on the board can be moved to an \n\
           adjacent free space.\n\
       b.) A Bucket can go on top of a Small Sand Pile.\n\
       c.) A Small Sand Pile can go on top of a Big Sand Pile.\n\
       d.) A Small Sand Pile with a Bucket on its top can go on\n\
           top of a Big Sand Pile.\n\
       e.) A Bucket can go on top of a Sand Castle.\n\
    NOTE: You CANNOT place a piece on top of another piece.\n\
          Pieces must be MOVED on top of other pieces.\n\
3.) Move any combination of Sand Piles with your Buckets on top,\n\
    or any Sand Castle, to any free space.\n\
\n\
Each player takes turns making one valid move, also noting\n\
that player CANNOT reverse an opponent's move that was just made.\n\
Use the LEGEND to determine which numbers to choose to\n\
correspond to either a piece to place and a board slot to place it\n\
OR to the location of your piece or a neutral piece and the empty\n\
adjacent position you wish to move that piece to.";

STRING   kHelpOnYourTurn =
"Note that a player CANNOT undo an oppontent's move that was just made.\n\
Use the LEGEND to determine which numbers to choose to\n\
correspond to either a piece to place and a board slot where to place it\n\
OR to the location of your piece or a neutral piece and the empty\n\
adjacent position you wish to move that piece to.\n\
Example: 's 1' would place a Small Sand Pile in position 1\n\
Example: '1 2' would move a piece in position 1 to position 2.";

STRING   kHelpStandardObjective =
"Be the first to have your 2 Buckets on top of 2 Sand Castles,\n\
where a Sand Castle consists of 1 Small Sand Pile put on top\n\
of 1 Big Sand Pile.";

STRING   kHelpReverseObjective =
"";	/* There exists no reverse objective */

STRING   kHelpTieOccursWhen =
"A tie never occurs.";

STRING   kHelpExample =
"";


/*************************************************************************
**
** #defines and structs
**
**************************************************************************/

#define ROWCOUNT 3;
#define COLCOUNT 3;

#define BLANKPIECE '_';
#define SMALLPIECE 's';
#define LARGEPIECE 'l';
#define CASTLEPIECE 'c';
#define BLUEBUCKETPIECE 'b';
#define REDBUCKETPIECE 'r';
#define BLUESMALLPIECE 'B';
#define REDSMALLPIECE 'R';
#define BLUECASTLEPIECE 'X';
#define REDCASTLEPIECE 'O';
#define UNKNOWNPIECE '0';  // hopefully none of these b/c can't be represented by a digit from 0 - 9

#define BLUETURN 0;
#define REDTURN 1;

#define BOLD_UL_CORNER 201
#define BOLD_UR_CORNER 187
#define BOLD_LL_CORNER 200
#define BOLD_LR_CORNER 188
#define BOLD_HOR 205
#define BOLD_VERT 186
#define BOLD_HOR_DOWN 209
#define BOLD_HOR_UP 207
#define BOLD_VERT_LEFT 182
#define BOLD_VERT_RIGHT 199
#define HOR_LINE 196
#define VERT_LINE 179
#define CROSS_LINE 197

typedef enum possibleBoardPieces {
    Blank = 0, SmallSand, LargeSand, SandCastle, BlueBucket, 
    RedBucket, BlueSmall, RedSmall, BlueCastle, RedCastle
} BoardPiece;

typedef enum playerTurn {
	Blue, Red
} PlayerTurn;

// need this for unhashing...
// "the" used for structs in code (use as convention??)
typedef struct boardAndTurnRep {
  char *theBoard;
  PlayerTurn theTurn;
} boardAndTurn;

/*************************************************************************
**
** Global Variables
**
*************************************************************************/

int boardSize = ROWCOUNT * COLCOUNT;
int rowWidth = COLCOUNT;
int redCastles = 0;
int blueCastles = 0;
int totalBoardPieces = 0;

PlayerTurn gWhosTurn;
int gameType;

/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/

/* External */
extern GENERIC_PTR		SafeMalloc ();
extern void				SafeFree ();
extern POSITION         generic_hash_init(int boardsize, int pieces_array[], int (*vcfg_function_ptr)(int* cfg));
extern POSITION         generic_hash(char *board, int player);
extern char            *generic_unhash(POSITION hash_number, char *empty_board);
extern int              whoseMove (POSITION hashed);
/* Internal */
void                    InitializeGame();
MOVELIST               *GenerateMoves(POSITION position);
POSITION                DoMove (POSITION position, MOVE move);
VALUE                   Primitive (POSITION position);
void                    PrintPosition(POSITION position, STRING playersName, BOOLEAN usersTurn);
void                    PrintComputersMove(MOVE computersMove, STRING computersName);
void                    PrintMove(MOVE move);
USERINPUT               GetAndPrintPlayersMove (POSITION position, MOVE *move, STRING playersName);
BOOLEAN                 ValidTextInput(STRING input);
MOVE                    ConvertTextInputToMove(STRING input);
void                    GameSpecificMenu();
void                    SetTclCGameSpecificOptions(int options[]);
POSITION                GetInitialPosition();
int                     NumberOfOptions();
int                     getOption();
void                    setOption(int option);
void                    DebugMenu();
/* Game-specific */
char					BoardPieceToChar(BoardPiece piece);
POSITION				arrayHash(char *board, PlayerTurn player);
char					*arrayUnhash(POSITION hashNumber);
/*BOOLEAN               OkMove(char *theBlankFG, int whosTurn, SLOT fromSlot,SLOT toSlot);
BOOLEAN                 CantMove(POSITION position);
void                    ChangeBoard();
void                    MoveToSlots(MOVE theMove, SLOT *fromSlot, SLOT *toSlot);
MOVE                    SlotsToMove (SLOT fromSlot, SLOT toSlot);
void                    InitializeAdjacency();
void                    InitializeOrder();*/


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
    int i;
    int piecesArray[] = { Blank, SmallSand, LargeSand, SandCastle,
						  BlueBucket, RedBucket, BlueSmall, RedSmall,
						  BlueCastle, RedCastle };
    BoardPiece boardArray[boardSize];
    
    gNumberOfPositions = generic_hash_init(boardSize, piecesArray, NULL);
    gWhosTurn = Blue;
    
    for (i = 0; i < boardSize; i++) {
    	boardArray[i] = Blank;
    }
    
    gInitialPosition = generic_hash(boardArray, gWhosTurn);
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
    return 0;
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

void PrintPosition (POSITION position, STRING playersName, BOOLEAN usersTurn)
{
	#define BOLD_UL_CORNER 201
	#define BOLD_UR_CORNER 187
	#define BOLD_LL_CORNER 200
	#define BOLD_LR_CORNER 188
	#define BOLD_HOR 205
	#define BOLD_VERT 186
	#define BOLD_HOR_DOWN 209
	#define BOLD_HOR_UP 207
	#define BOLD_VERT_LEFT 182
	#define BOLD_VERT_RIGHT 199
	#define HOR_LINE 196
	#define VERT_LINE 179
	#define CROSS_LINE 197

 	int *arrayHashedBoard, i;
  
	arrayHashedBoard = arrayUnhash(position);
	
	/***********************LINE 1**************************/
	printf("       %c%c%c%c%c%c%c\n", BOLD_UL_CORNER, BOLD_HOR, BOLD_HOR_DOWN, BOLD_HOR,
									BOLD_HOR_DOWN, BOLD_HOR, BOLD_UR_CORNER);
	/***********************LINE 2**************************/
	printf("       %c", BOLD_VERT);
	for (i = 0; i < rowWidth; i++) {
		printf("%c%c", BoardPieceToChar(arrayHashedBoard[i], 
										(i == (rowWidth-1)) ? VERT_LINE : BOLD_VERT);
	}
	printf("          ( 1 2 3 )\n");
	/***********************LINE 3**************************/
	printf("       %c%c%c%c%c%c%c\n", BOLD_VERT_RIGHT, HOR_LINE, CROSS_LINE, HOR_LINE,
										CROSS_LINE, HOR_LINE, BOLD_VERT_LEFT);							
	/***********************LINE 4**************************/
	printf("BOARD: %c", BOLD_VERT);
	for (i = rowWidth; i < (rowWidth*2); i++) {
		printf("%c%c", BoardPieceToChar(arrayHashedBoard[i], 
										(i == ((rowWidth*2)-1)) ? VERT_LINE : BOLD_VERT);
	}
	printf("  LEGEND: ( 4 5 6)\n");
	/***********************LINE 5**************************/
	printf("       %c%c%c%c%c%c%c\n", BOLD_VERT_RIGHT, HOR_LINE, CROSS_LINE, HOR_LINE,
										CROSS_LINE, HOR_LINE, BOLD_VERT_LEFT);
	/***********************LINE 6**************************/
	for (i = rowWidth*2; i < (rowWidth*3); i++) {
		printf("%c%c", BoardPieceToChar(arrayHashedBoard[i], 
										(i == ((rowWidth*3)-1)) ? VERT_LINE : BOLD_VERT);
	}
	printf("          ( 7 8 9 )\n");
	/***********************LINE 7**************************/
	printf("       %c%c%c%c%c%c%c\n", BOLD_LL_CORNER, BOLD_HOR, BOLD_HOR_UP, BOLD_HOR,
									BOLD_HOR_UP, BOLD_HOR, BOLD_LR_CORNER);
	/***********************LINE 8, 9, 10, 11**************************/
	printf("                 Small Sand Pile = s\n");
	printf("                 Large Sand Pile = l\n");
	printf("                          Bucket = b\n");
	printf("\n%s\n\n", GetPrediction(position, playerName, usersTurn));
	SafeFree(arrayHashedBoard);
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

char BoardPieceToChar(BoardPiece piece) {
	switch (piece) {
		case Blank:		return BLANKPIECE;
		case SmallSand:		return SMALLPIECE;
		case LargeSand:		return LARGEPIECE;
		case SandCastle:	return CASTLEPIECE;
		case BlueBucket:	return BLUEBUCKETPIECE;
		case RedBucket:		return REDBUCKETPIECE;
		case BlueSmall:		return BLUESMALLPIECE;
		case RedSmall:		return REDSMALLPIECE;
		case BlueCastle:	return BLUECASTLEPIECE;
		case RedCastle:		return REDCASTLEPIECE;
	}
	
	return UNKNOWNPIECE;
}

int CharToBoardPiece(char piece) {
	switch (piece) {
	  case BLANKPIECE:            return Blank;
	  case SMALLPIECE:            return SmallSand;
	  case LARGEPIECE:            return LargeSand;
	  case CASTLEPIECE:           return SandCastle;
	  case BLUEBUCKETPIECE:       return BlueBucket;
	  case REDBUCKETPIECE:        return RedBucket;
	  case BLUESMALLPIECE:        return BlueSmall;
	  case REDSMALLPIECE:         return RedSmall;
	  case BLUECASTLEPIECE:       return BlueCastle;
	  case REDCASTLEPIECE:        return RedCastle;
	}
	
	return -1;
}

/*
  arrayHash - hashes the board to a number
  Since there are 10 different pieces, this hash utilizes this fact and 
*/
POSITION arrayHash(char *board, PlayerTurn player) {
  int hashNum = 
    CharToBoardPiece(board[0]) + 
    CharToBoardPiece(board[1]) * 10 + 
    CharToBoardPiece(board[2]) * 100 + 
    CharToBoardPiece(board[3]) * 1000 + 
    CharToBoardPiece(board[4]) * 10000 + 
    CharToBoardPiece(board[5]) * 100000 + 
    CharToBoardPiece(board[6]) * 1000000 + 
    CharToBoardPiece(board[7]) * 10000000 + 
    CharToBoardPiece(board[8]) * 100000000 + 
    player * 1000000000;
  return hashNum;
}

boardAndTurn* arrayUnhash(POSITION hashNumber) {
  //char boardTemp[9];
  //char *boardTemp;

  struct boardAndTurn bat;
  int i, j;
  bat.theBoard = (char *)SafeMalloc(ROWCOUNT * COLCOUNT * sizeOf(char));

  // should only be 9 entries (3 by 3 board)
  for (i = 0; i < ROWCOUNT; i++) {
    for (j = 0; j < COLCOUNT; j++) {
      bat.theBoard[j + i*ROWCOUNT] = hashNumber % 10;
      hashNumber = hashNumber / 10;
    }
  }
  bat.theTurn = hashNumber % 2;
  return boardTemp;
}

// $Log: not supported by cvs2svn $
// Revision 1.3  2006/02/22 09:49:04  alexchoy
// edited unhash and hash to make them more correct
//
// Revision 1.2  2006/02/22 09:37:24  alexchoy
// added hashing and unhashing functions
//
// Revision 1.1  2006/02/20 19:36:45  mikehamada
// First addition to repository for Topitop by Mike Hamada
// Setup #defines & data-structs
// Wrote InitializeGame() and PrintPosition()
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
