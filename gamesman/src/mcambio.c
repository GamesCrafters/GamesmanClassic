// $Id: mcambio.c,v 1.25 2006-04-29 01:40:56 simontaotw Exp $

/*
 * The above lines will include the name and log of the last person
 * to commit this file to CVS
 */

/************************************************************************
**
** NAME:        mcambio.c
**
** DESCRIPTION: Cambio
** AUTHOR:      Albert Chae and Simon Tao
**
** DATE:        Begin: 2/20/2006 End: 
**
** UPDATE HIST: 2/20/2006 - Updated game-specific constants.
**              2/21/2006 - Updated defines and structs, global variables, and InitializeGame(). Corrected CVS log.
**              2/26/2006 - Updated PrintPosition() (Modified PrintPosition() from mtopitop.c).
**              3/02/2006 - Fixed various errors.
**              3/04/2006 - Fixed compile errors.
**              3/05/2006 - Updated Primitive() and added FiveInARow(). Updated tie possible.
**              3/06/2006 - Updated GetInitialPosition().
**	        3/12/2006 - Fixed PrintPosition() seg fault. Fixed GetInitialPosition() polling loo
**                                    Removed blanks from the game.
**              3/14/2006 - Reducing game to 4x4 to see if that fixes a hash problem.
**              3/16/2006 - Changed board printout. Made GetInitialPosition check inputs.
**              3/18/2006 - Added GenerateMove() and PrintMove() and ValidTextInput() and ConvertTextInputToMove().
**                                    Also added DoMove(). Some bugs left to work out with involving the alternation of moves etc.
**              3/19/2006 - Changed the Legend so it looks less cramped.
**	        4/16/2006 - Trying 3x3 board to see if it solves
**              4/28/2006 - Added new input format with 3x3 board.
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

STRING   kGameName            = "Cambio"; /* The name of your game */
STRING   kAuthorName          = "Albert Chae and Simon Tao"; /* Your name(s) */
STRING   kDBName              = "cambio"; /* The name to store the database under */

BOOLEAN  kPartizan            = TRUE ; /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN  kGameSpecificMenu    = FALSE ; /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN  kTieIsPossible       = TRUE ; /* TRUE if a tie is possible. FALSE if it is impossible.*/
BOOLEAN  kLoopy               = TRUE ; /* TRUE if the game tree will have cycles (a rearranger style game). FALSE if it does not.*/

BOOLEAN  kDebugMenu           = TRUE ; /* TRUE only when debugging. FALSE when on release. */
BOOLEAN  kDebugDetermineValue = FALSE ; /* TRUE only when debugging. FALSE when on release. */

POSITION gNumberOfPositions   = 0; /* The number of total possible positions | If you are using our hash, this is given by the hash_init() function*/

POSITION gInitialPosition     = 0; /* The initial hashed position for your starting board */
POSITION kBadPosition         = -1; /* A position that will never be used */

void*	 gGameSpecificTclInit = NULL;

/* 
 * Help strings that are pretty self-explanatory
 * Strings than span more than one line should have backslashes (\) at the end of the line.
 */

STRING kHelpGraphicInterface =
"Not written yet";

STRING   kHelpTextInterface    =
"BOARD SETUP:\n\
1. The players each select a symbol to be their own, and decide\n\
   who will begin the game.\n\
2. Fill the four corners with neutral symbols.\n\
3. Player A creates a starting position for Player B by placing\n\
   4 cubes into the tray, with player B's symbol.\n\
   These cubes may be placed in any of the positions that a cube\n\
   may occupy when the tray is full.\n\
4. Player B now places 3 of the first player A's symbol in any\n\
   free positions.\n\
5. Fill in the rest of the positions with neutral symbols.\n\n\
PLAY:\n\
Each player in turn places a spare cube with his/her own symbol, at\n\
the beginning of any row or column; frees the end cube by lifting\n\
it from the board; then pushes all the pieces in that line along\n\
one place. A player MAY NOT push cubes with your opponents symbol\n\
showing OFF the board.\n"; 

STRING   kHelpOnYourTurn =
"You may only push off neutral cubes, or cubes of your own symbol.\n\
As the game progresses, you should have more and more of your own\n\
cubes in play.\n";

STRING   kHelpStandardObjective =
"To make a line composed of all 4 of your own symbol - \n\
horizontally, vertically or diagonally.\n";

STRING   kHelpReverseObjective =
"";

STRING   kHelpTieOccursWhen =
"A tie is possible when a player's move causes two 4 in a rows \n\
simultaneously.";

STRING   kHelpExample =
"";


/*************************************************************************
**
** #defines and structs
**
**************************************************************************/

#define ROWCOUNT 3;
#define COLCOUNT 3;
#define BOARDSIZE 9;

#define NEUTRAL '+';
#define A_PIECE 'X';
#define B_PIECE 'O';
#define UNKNOWN '?';

typedef enum player {
  playerA = 1, playerB = 2
} Player;


/*************************************************************************
**
** Global Variables
**
*************************************************************************/

int rowcount = ROWCOUNT;
int colcount = COLCOUNT;
int boardSize = BOARDSIZE;
char neutral = NEUTRAL;
char aPiece = A_PIECE;
char bPiece = B_PIECE;
char unknown = UNKNOWN;

int piecesArray[] = {'+', 0, 9, 'X', 0, 9, 'O', 0, 9, -1 };

STRING alphaArray[] = { "a1", "a2", "a3", 
		        "b1", "b2", "b3",
		        "c1", "c2", "c3" };
/* to convert, use alphaArray[i]
                      {   0,    1,    2,
                          3,    4,    5,
                          6,    7,    8 };
*/

STRING shiftArray[] = { "a", "b", "c", 
		        "4", "5", "6",
		        "f", "e", "d",
		        "3", "2", "1" };
/* to convert, use shiftArray[i-16]
                      {   9,  10,  11,
                         12,  13,  14,
                         15,  16,  17,
                         18,  19,  20 };

*/

Player gWhosTurn; /* 1 for player A, 2 for player B */

char *gBoard;

/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/

/* External */
extern GENERIC_PTR	SafeMalloc ();
extern void		SafeFree ();
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
BOOLEAN                 ThreeInARow(char *board, char symbol);

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

  char boardArray[boardSize];

  gNumberOfPositions = generic_hash_init(boardSize, piecesArray, NULL);
  gWhosTurn = playerA;
    
  /* filling up the board; with Neutrals */
  for (i = 0; i < boardSize; i++)
    boardArray[i] = neutral;
	
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
/* must check all the math used for general case, such as 5x5 and on */
{
    MOVELIST *moves = NULL;
	MOVELIST *CreateMovelistNode();
	char *gBoard = (char *) malloc(boardSize*sizeof(char));
	char opposymbol;
	int countA, countB, turn, i;
	
	generic_unhash(position, gBoard);
	
	countA = 0; countB = 0;
	turn = whoseMove(position);
	
	/* count the number of pieces for each player */
	for(i = 0; i < boardSize; i++)
	{
		if(gBoard[i] == aPiece)
			countA++;
		else if(gBoard[i] == bPiece)
			countB++;
	}
	
	/* Assign pieces for each player */
	if(turn == playerB) {
		opposymbol = aPiece;
	}
	else {
		opposymbol = bPiece;
	}
	
	/* Phase 1: Less than 3 of PlayerB's pieces on the board. */
	if(countB < 2)
	{
		for(i = 0; i < boardSize; i++)
		{
			if((gBoard[i] != bPiece)  && 
			   ((i != 0)                      && (i != (colcount-1)) && 
			    (i != (boardSize - colcount)) && (i != (boardSize - 1))))
				{
					moves = CreateMovelistNode(i, moves);
				}
		}
	}
	/*.Phase 2: Less than 2 of PlayerA's pieces on the board. */
	else if(countA < 1)
	{
		for(i = 0; i < boardSize; i++)
		{
			if((gBoard[i] != bPiece)  && (gBoard[i] != aPiece) &&
			   ((i != 0)                      && (i != (colcount-1)) && 
			    (i != (boardSize - colcount)) && (i != (boardSize - 1))))
				{
					moves = CreateMovelistNode(i, moves);
				}
		}
	}
	/* Phase 3:The main phase of the game. Place your piece at the end of one row and push the piece at the other side off */
	else
	{
		/* top row moves */
		for(i = 0; i < colcount; i++)
		{
			if(gBoard[i + (boardSize-colcount)] != opposymbol) 
				moves = CreateMovelistNode(i + boardSize, moves); 
		}
		/* right column moves */
		for(i = 0; i < rowcount; i++) 
		{
			if(gBoard[((i+1)*colcount) - colcount] != opposymbol) 
				moves = CreateMovelistNode(i + boardSize + colcount, moves); 
		}
		/* bottom row moves */
		for(i = 0; i < colcount; i++)
		{
			if(gBoard[(colcount - i - 1)] != opposymbol) 
				moves = CreateMovelistNode(i + boardSize + colcount + rowcount, moves); 
		}
		/* left column moves */
		for(i = 0; i < rowcount; i++)
		{
			if(gBoard[((colcount - i)*colcount) - 1] != opposymbol) 
				moves = CreateMovelistNode(i + boardSize + colcount + rowcount + colcount, moves); 
		}
	}
	
	free(gBoard);
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
	char symbol, opposymbol;
	char *gBoard = (char *) malloc(boardSize*sizeof(char));
	int turn, countA, countB, i;
	int pushon, pushoff;
	POSITION positionAfterMove;
	
	generic_unhash(position, gBoard);
	turn = whoseMove(position);
	countA = 0;
	countB = 0;
    
	/* Assign pieces for each player */
	if(turn == playerA) {
		symbol = aPiece;
		opposymbol = bPiece;
	}
	else {
		symbol = bPiece;
		opposymbol = aPiece;
	}
	
	/* count the number of pieces for each player */
	for(i = 0; i < boardSize; i++)
	{
		if(gBoard[i] == aPiece)
			countA++;
		else if(gBoard[i] == bPiece)
			countB++;
	}
	
	if(move < boardSize)
		gBoard[move] = opposymbol;
	else 
	{
		if (move < boardSize + colcount)
		{
			pushon = (move - boardSize);
			pushoff = (move - colcount);
			gBoard[pushoff] = gBoard[pushoff-colcount];
			gBoard[pushoff-colcount] = gBoard[pushoff-colcount*2];
			gBoard[pushoff-colcount*2] = gBoard[pushon];
			gBoard[pushon] = symbol;
		}
		else if (move < boardSize + colcount + rowcount)
		{
			pushon = (move - boardSize - colcount + 1)*colcount - 1;
			pushoff = (move - boardSize - colcount)*colcount;
			gBoard[pushoff] = gBoard[pushoff+1];
			gBoard[pushoff+1] = gBoard[pushoff+2];
			gBoard[pushoff+2] = gBoard[pushon];
			gBoard[pushon] = symbol;
		}
		else if (move < boardSize + colcount + rowcount + colcount)
		{
			pushon = boardSize - (move - boardSize - colcount - rowcount + 1);
			pushoff = colcount - (move - boardSize - colcount - rowcount) - 1;
			gBoard[pushoff] = gBoard[pushoff+colcount];
			gBoard[pushoff+colcount] = gBoard[pushoff+colcount*2];
			gBoard[pushoff+colcount*2] = gBoard[pushon];
			gBoard[pushon] = symbol;
		}
		else if (move < boardSize + colcount + rowcount + colcount + rowcount)
		{
			pushon = boardSize - (move - boardSize - colcount*2 - rowcount + 1)*colcount;
			pushoff = boardSize - (move - boardSize - colcount*2 - rowcount + 1)*colcount + colcount - 1;
			gBoard[pushoff] = gBoard[pushoff-1];
			gBoard[pushoff-1] = gBoard[pushoff-2];
			gBoard[pushoff-2] = gBoard[pushon];
			gBoard[pushon] = symbol;
		}	
	}	

	/* change turns */
	if(countB == 2)
		turn = playerB;
	else if(countA == 1)
		turn = playerA;
	else if((countB>=3) && (countA>=2))
	{
		if(turn == playerA)
			turn = playerB;
		else if(turn == playerB)
			turn = playerA;
	}
	
	positionAfterMove = generic_hash(gBoard, turn);
	free(gBoard);
	
	return positionAfterMove;
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
  int turn = whoseMove(position);
  char symbol, opposymbol;
  char *gBoard = (char *) malloc(boardSize*sizeof(char));
  BOOLEAN playerWin, oppoWin;
  
  
  generic_unhash(position, gBoard);
  
  if(turn == playerB) {
    symbol = bPiece;
    opposymbol = aPiece;
  }
  else {
    symbol = aPiece;
    opposymbol = bPiece;
  }

  
  playerWin = ThreeInARow(gBoard, symbol);
  oppoWin = ThreeInARow(gBoard, opposymbol);

  
  free(gBoard);
  
  if (playerWin && oppoWin)
    return tie;
  else if (oppoWin)
    return gStandardGame ? lose : win;
  else if (playerWin)
	return gStandardGame ? win : lose;
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

void PrintPosition (POSITION position, STRING playersName, BOOLEAN usersTurn)
{

  char *gBoard = (char *) malloc(boardSize*sizeof(char));
  int countA = 0, countB = 0, i = 0, turn;
	
  generic_unhash(position, gBoard);
	
  turn = whoseMove(position);
	
  /* count the number of pieces for each player */
  for(i = 0; i < boardSize; i++)
    {
      if(gBoard[i] == aPiece)
	countA++;
      else if(gBoard[i] == bPiece)
	countB++;
    }
	
  /* Phase 1: Less than 2 of PlayerB's pieces or less than 1 of PlayerB's pieces on the board. */
  if(countB < 2 || countA < 1)
    {
      printf("\n");
      printf("                1 2 3 \n");
      printf("               -------\n");

      for(i = 0; i < boardSize; i++)
	{
	  //indentation
	  if((i % 3) == 0)
	    {
	      printf("             %c ", (i/3)+97);
	    }

	  if(gBoard[i] == bPiece)
	    {
	      printf("|%c", aPiece);
	    }
	  else if(gBoard[i] == aPiece)
	    {
	      printf("|%c", bPiece);
	    }
	  else if(gBoard[i] == neutral)
	    {
	      printf("|%c", neutral);
	    }
	  else
	    {
	      printf("Error in: PrintPosition Phase 1 Loop");
	    }

	  if((i != 0) && (((i+1) % 3) == 0))
	    {
	      printf("| %c\n", (i/3)+97);
	    }
	}

      printf("               -------\n");
      printf("                1 2 3 \n");
      printf("\n");
      printf("                           A's Symbol = %c\n", aPiece);
      printf("                           B's Symbol = %c\n", bPiece);
      printf("                              Neutral = %c\n\n", neutral);
    }
  /* Phase 2: The main phase of the game. Place your piece at the end of one row and push the piece at the other side off */
  else if(countB >= 2 && countA >= 1)
    {
      printf("\n");
      printf("                a b c \n");
      printf("               -------\n");

      for(i = 0; i < boardSize; i++)
	{
	  //indentation
	  if((i % 3) == 0)
	    {
	      printf("             %d ", (i/3)+1);
	    }

	  if(gBoard[i] == bPiece)
	    {
	      printf("|%c", aPiece);
	    }
	  else if(gBoard[i] == aPiece)
	    {
	      printf("|%c", bPiece);
	    }
	  else if(gBoard[i] == neutral)
	    {
	      printf("|%c", neutral);
	    }
	  else
	    {
	      printf("Error in: PrintPosition Phase 2 Loop");
	    }

	  if((i != 0) && (((i+1) % 3) == 0))
	    {
	      printf("| %d\n", (i/3)+4);
	    }
	}

      printf("               -------\n");
      printf("                d e f \n");
      printf("\n");
      printf("                           A's Symbol = %c\n", aPiece);
      printf("                           B's Symbol = %c\n", bPiece);
      printf("                              Neutral = %c\n\n", neutral);
    }
  /* bad else */
  else
    {
      printf("Error in: PrintPosition Phase Conditional\n"); 
    }

  
  free(gBoard);
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
/* must check all the math used for general case, such as 5x5 and on */
{

	if(move < boardSize)
		printf("%s", alphaArray[move]);
	else if (move < boardSize + colcount)
		printf("%s", shiftArray[move-9]);
	else if (move < boardSize + colcount + rowcount)
		printf("%s", shiftArray[move-9]);
	else if (move < boardSize + colcount + rowcount + colcount)
		printf("%s", shiftArray[move-9]);
	else if (move < boardSize + colcount + rowcount + colcount + rowcount)
		printf("%s", shiftArray[move-9]);
		
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
    // use whoseMove(position) = turn to get whose turn it is

  USERINPUT input;
  USERINPUT HandleDefaultTextInput();

  char *gBoard = (char *) malloc(boardSize*sizeof(char));
  generic_unhash(position, gBoard);

  int countA = 0, countB = 0, i = 0, turn = whoseMove(position);
	
  /* count the number of pieces for each player */
  for(i = 0; i < boardSize; i++)
    {
      if(gBoard[i] == aPiece)
	countA++;
      else if(gBoard[i] == bPiece)
	countB++;
    }

  /* Phase 1: Less than 2 of PlayerB's pieces */
  if(countB < 2 && turn == playerA)
    {
      for (;;) {
	/***********************************************************
	 * CHANGE THE LINE BELOW TO MATCH YOUR MOVE FORMAT
	 ***********************************************************/
	printf("    Player's move [(u)ndo/([a-c][1-3])] : ");
	
	input = HandleDefaultTextInput(position, move, "Player");
	
	if (input != Continue)
	  return input;
      }
    }
  else if(countB == 2 && countA < 1 && turn == playerA)
    {
      /* this is an intermediate phase */
      printf("Switching player...\n");
    }
  /* Phase 2: Less than 1 of PlayerB's pieces on the board. */
  else if(countB == 2 && countA < 1 && turn == playerB)
    {
      for (;;) {
	/***********************************************************
	 * CHANGE THE LINE BELOW TO MATCH YOUR MOVE FORMAT
	 ***********************************************************/
	printf("Challenger's move [(u)ndo/([a-c][1-3])] : ");
	
	input = HandleDefaultTextInput(position, move, "Challenger");
	
	if (input != Continue)
	  return input;
      }
    }
  /* Phase 3: The main phase of the game. Place your piece at the end of one row and push the piece at the other side off */
  else if(countB >= 2 && countA >= 1 && turn == playerA)
    {
      for (;;) {
	/***********************************************************
	 * CHANGE THE LINE BELOW TO MATCH YOUR MOVE FORMAT
	 ***********************************************************/
	printf("%8s's move [(u)ndo/([a-f or 1-6])]  : ", playersName);
	
	input = HandleDefaultTextInput(position, move, playersName);
	
	if (input != Continue)
	  return input;
      }
    }
  else
    {
      printf("Error in: GetAndPrintPlayersMove Phase 2");
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
  BOOLEAN flag = TRUE;

  if(strlen(input) > 3)
    flag = FALSE;
	
  if(!(((input[0] == 'a') && (input[1] == '1')) ||
       ((input[0] == 'a') && (input[1] == '2')) ||
       ((input[0] == 'a') && (input[1] == '3')) ||
       ((input[0] == 'b') && (input[1] == '1')) ||
       ((input[0] == 'b') && (input[1] == '2')) ||
       ((input[0] == 'b') && (input[1] == '3')) ||
       ((input[0] == 'c') && (input[1] == '1')) ||
       ((input[0] == 'c') && (input[1] == '2')) ||
       ((input[0] == 'c') && (input[1] == '3')) ||
       (input[0] == 'a') ||
       (input[0] == 'b') ||
       (input[0] == 'c') ||
       (input[0] == 'd') ||
       (input[0] == 'e') ||
       (input[0] == 'f') ||
       (input[0] == '1') ||
       (input[0] == '2') ||
       (input[0] == '3') ||
       (input[0] == '4') ||
       (input[0] == '5') ||
       (input[0] == '6') ))
    flag = FALSE;

  
  return flag;
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
  int move1 = (int)input[0];
  int move2 = (int)input[1];
  int move;

  if(move1 == 97 && move2 >= 49 && move2 <= 51)
    move = move2 - 49;
  else if(move1 == 98 && move2 >= 49 && move2 <= 51)
    move = move2 - 46;
  else if(move1 == 99 && move2 >= 49 && move2 <= 51)
    move = move2 - 43;
  else if(move1 >= 97 && move1 <= 99)
    move = move1 - 88;
  else if(move1 >= 52 && move1 <= 54)
    move = move1 - 40;
  else if(move1 >= 100 && move1 <= 102)
    move = 117 - move1;
  else if(move1 >= 49 && move1 <= 51)
    move = 69 - move1;
  else
    printf("Error in: ConvertTextInputToMove");

  
  return (MOVE) move;
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
  int i;
  char *boardArray;
  char piece, turn;
  
  boardArray = (char*) malloc(boardSize*sizeof(char));

  getchar(); // for the enter after picking option 1 on the debug menu
  
  for(i = 0; i < boardSize; i++) {       
	do{
		printf("Input the character at cell %i: ", i);
		piece = (char) getchar();
		getchar();
	}while((piece != '+') && (piece != 'X') && (piece != 'O'));
	boardArray[i] = piece;
  }
  
  
  do{
		printf("Input whose turn it is (1 for player A, 2 for player B): ");
		turn = (char) getchar();
		getchar();
	}while((turn != '1') && (turn != '2')); 
  gWhosTurn = atoi(&turn);

  gNumberOfPositions = generic_hash_init(boardSize, piecesArray, NULL);
  gInitialPosition = generic_hash(boardArray, gWhosTurn);

  free(boardArray);
  
  return gInitialPosition;
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
BOOLEAN ThreeInARow(char *board, char symbol)
{
  return (/* horizontal */
         (board[0]  == symbol && board[1]  == symbol && board[2]  == symbol) ||
         (board[3]  == symbol && board[4]  == symbol && board[5]  == symbol) ||
         (board[6]  == symbol && board[7]  == symbol && board[8]  == symbol) ||
         /* vertical */
         (board[0]  == symbol && board[3] == symbol && board[6]  == symbol) ||
         (board[1]  == symbol && board[4] == symbol && board[7]  == symbol) ||
         (board[2]  == symbol && board[5] == symbol && board[8]  == symbol) ||
         /* diagonal */
         (board[0]  == symbol && board[4] == symbol && board[8]  == symbol) ||
         (board[2]  == symbol && board[4] == symbol && board[6]  == symbol));
}



// $Log: not supported by cvs2svn $
// Revision 1.19  2006/03/19 20:39:15  simontaotw
// Changed the legend so it looks less cramped.
//
// Revision 1.18  2006/03/19 06:21:27  albertchae
// DoMove() works somewhat. Able to play human v human except for a bug
// with alternating player's move and challenger's move.
//
// Revision 1.17  2006/03/18 10:36:35  albertchae
// Wrote the code for GenerateMoves and PrintMoves. Tried to keep
// the logic general in case we have variant board sizes. Still needs some
// testing.
//
// Revision 1.16  2006/03/14 12:38:29  albertchae
// Minor edit. Changed help strings to reflect 4x4. Added some ideas for
// generate move.
//
// Revision 1.15  2006/03/14 12:06:05  albertchae
// Changed game to 4x4 variant. Added test case to Primitive().
//
// Revision 1.14  2006/03/13 06:15:12  albertchae
// Removed blanks as a possible piece. By rewriting the instructions, we can play using only neutrals.
//
// Revision 1.13  2006/03/12 22:16:55  albertchae
// I fixed the PrintPosition() seg fault. generic_hash requires an empty board which we did not have allocated before.
// I also fixed the loop in GetInitialPosition() that asks for input. There is something wrong with hashing.
//
// Revision 1.12  2006/03/12 22:13:02  albertchae
// *** empty log message ***
//
// Revision 1.11  2006/03/08 06:25:41  simontaotw
// Updated GetInitialPosition().
//
// Revision 1.10  2006/03/07 07:59:11  albertchae
// Minor change to the help strings regarding ties.
//
// Revision 1.9  2006/03/06 06:07:10  simontaotw
// Updated tie possible.
//
// Revision 1.8  2006/03/06 01:57:32  simontaotw
// Updated Primitive() and added FiveInARow().
//
// Revision 1.7  2006/03/05 03:28:15  yanpeichen
// Yanpei Chen changing mcambio.c
//
// deleted two stray characters that caused a compiler error.
//
// Revision 1.6  2006/03/04 20:00:54  simontaotw
// Fixed compile errors.
//
// Revision 1.5  2006/03/03 05:19:49  simontaotw
// Fixed various errors.
//
// Revision 1.4  2006/02/26 20:36:53  simontaotw
// Updated PrintPosition() (Modified PrintPosition() in mtopitop.c).
//
// Revision 1.3  2006/02/22 02:54:48  simontaotw
// Updated defines and structs, global variables, and InitializeGame(). Corrected CVS log.
//
// Revision 1.1  2006/02/21 03:17:00  simontaotw
// Updated game-specific constants
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
