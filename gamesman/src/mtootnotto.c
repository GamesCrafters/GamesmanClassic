/************************************************************************
**
** NAME:        mtootnotto.c
**
** DESCRIPTION: Toot and Otto
**
** AUTHOR:      Kyler Murlas and Zach Wasserman
**
** DATE:        YYYY-MM-DD 2005-10-12 FINISH
**
** UPDATE HIST: RECORD CHANGES YOU HAVE MADE SO THAT TEAMMATES KNOW
**
**              2005-11-28 Added Primitive
**				2006-04-05 Edited Primitive to player1 wins if toot and
**						   player2 wins if otto. Also fixed tie condition and
**                         default output is now undecided if no win/lose/tie.
**				2006-04-08 Added a lot of printfs for debugging. Gets bus error
**						   with GetAndPrintPlayersMove
**				2006-04-11 Fixed bus error and all warnings. DoMove needs rewrite.
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

STRING   kGameName            = "Toot and Otto"; /* The name of your game */
STRING   kAuthorName          = "Kyler Murlas and Zach Wasserman"; /* Your name(s) */
STRING   kDBName              = "mtootnotto"; /* The name to store the database under */

BOOLEAN  kPartizan            = TRUE ; /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN  kGameSpecificMenu    = FALSE ; /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN  kTieIsPossible       = TRUE ; /* TRUE if a tie is possible. FALSE if it is impossible.*/
BOOLEAN  kLoopy               = FALSE ; /* TRUE if the game tree will have cycles (a rearranger style game). FALSE if it does not.*/

BOOLEAN  kDebugMenu           = TRUE ; /* TRUE only when debugging. FALSE when on release. */
BOOLEAN  kDebugDetermineValue = TRUE ; /* TRUE only when debugging. FALSE when on release. */

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

struct {
  int t;
  int o;
  int total;
} Board;

struct {
  int t;
  int o;
  int total;
} Player1;

struct {
  int t;
  int o;
  int total;
} Player2;

typedef enum{
  PLAYER1,
  PLAYER2
} Players;

typedef enum possibleBoardPieces {
	t, o, Blank
} TOBlank;

char *gBlankTOString[] = { "T", "O", "-" };

/*************************************************************************
**
** Global Variables
**
*************************************************************************/
int TNO_WIDTH = 4;
int TNO_HEIGHT = 4;

/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/

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
** NAME:        MyInitialPosition
**
** DESCRIPTION: Calculates the initial position
**
** INPUTS:      none
**
** OUTPUTS:     POSITION (Initial Position)
**
************************************************************************/

int MyInitialPosition() {
  unsigned long int p=0;
  int i;
  p+=6;
  p = (p << 3);
  p+=6;
  
  
  /*for (i=1;i<=TNO_WIDTH;++i){
	 p = ((p << 1) + 1 << (TNO_HEIGHT));
	}
	*/
	for (i=1;i<=TNO_WIDTH;++i)
		p = (p << (TNO_HEIGHT+1))+1;
		
	printf("%li", p);
	 return p;
}

/************************************************************************
**
** NAME:        MyNumberOfPos()
**
** DESCRIPTION: Calculates an upper bound for a position
**
** INPUTS:      none
**
** OUTPUTS:     unsigned long int (number)
**
************************************************************************/
int MyNumberOfPos() {
  int i;
  unsigned long int size=1;
  for (i=0;i<(TNO_HEIGHT+1)*TNO_WIDTH+6;i++)
	 size *= 2;
  return size;
}

/************************************************************************
**
** NAME:        PositionToBoard
**
** DESCRIPTION: convert an internal position to a TOBlank-matrix.
**
** INPUTS:      POSITION thePos   : The position input.
**              TOBlank *board    : The converted TOBlank output matrix.
**
************************************************************************/

void PositionToBoard(pos,board)
	  POSITION pos;
	  TOBlank board[TNO_WIDTH][TNO_HEIGHT+1];
	  // board is a two-dimensional array of size
	  // TNO_WIDTH x TNO_HEIGHT
{
  int col,row,h;
  for (col=0; col<TNO_WIDTH;col++) {
	 row=TNO_HEIGHT-1;
	 for (h=col*(TNO_HEIGHT+1)+TNO_HEIGHT; (pos & (1 << h)) == 0; h--) {
		board[col][row]=2;
      row--;
    }
    h--;
    while (row >=0) {
      if ((pos & (1<<h)) != 0) board[col][row]=1;
      else board[col][row]=0;
      row--;
		h--;
    }
  }
}

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
	gNumberOfPositions = MyNumberOfPos();
	gInitialPosition = MyInitialPosition();

}


void PiecesOnBoard(pos)
	 POSITION pos;
{
  Board.t = 0;
  Board.o = 0;
  Board.total = 0;
  
  /*
  int col,row,h;
  for (col=0; col<TNO_WIDTH;col++) {
	 row=TNO_HEIGHT-1;
	 for (h=col*(TNO_HEIGHT+1)+TNO_HEIGHT; (pos & (1 << h)) == 0; h--) {
      row--;
    }
    h--;
    while (row >=0) {
      if ((pos & (1<<h)) != 0) Board.o++;
      else Board.t++;
      row--;
		h--;
    }
  }
  Board.total = Board.t + Board.o;
  */
  int row, col;
  TOBlank board[TNO_WIDTH][TNO_HEIGHT+1];
  PositionToBoard(pos,board);
  
  for (row=TNO_HEIGHT-1;row>=0;row--) {
		for (col=0;col<TNO_WIDTH;col++){
			if(board[col][row] == 0) Board.t++;
			else if(board[col][row] == 1) Board.o++;
			Board.total++;
		}
	}
  
}


void PositionToPieces(pos)
	  POSITION pos;
{
  int i;
  i = (TNO_HEIGHT+1)*TNO_WIDTH;
  Player1.t = pos >> (i+3);
  Player1.o = (pos >> i) & 7;
  Player1.total = Player1.t + Player1.o;
  PiecesOnBoard(pos);
  Player2.t = 6 - Board.t - (6 - Player1.t);
  Player2.o = 6 - Board.o - (6 - Player1.o);
  Player2.total = Player2.t + Player2.o;
}



/************************************************************************
**
** NAME:        WhoseTurn
**
** DESCRIPTION: Return whose turn it is - either x or o. Since x always
**              goes first, we know that if the board has an equal number
**              of x's and o's, that it's x's turn. Otherwise it's o's.
**
** INPUTS:      POSITION  : a position
**
** OUTPUTS:     1 or 2
************************************************************************/
int WhoseTurn(pos)
POSITION pos;
{
  PositionToPieces(pos);
  printf("\n%i\n", Player1.total);
  printf("\n%i\n", Player2.total);
  if(Player1.total == Player2.total) return 1;
  else return 2;
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
  int col, player, move;

  player = WhoseTurn(position);
  PositionToPieces(position);
  TOBlank board[TNO_WIDTH][TNO_HEIGHT+1];
  PositionToBoard(position,board);
  
  for (col=0; col<TNO_WIDTH;col++) {
	move = 0;
	if(board[col][TNO_HEIGHT-1] == 2){
		move += col + 1;
		if(player == 1){
			if(Player1.t > 0){
				moves = CreateMovelistNode(move + 10, moves);
			}
			if(Player1.o > 0){
				moves = CreateMovelistNode(move, moves);
			}
		}
		else{
			if(Player2.t > 0){
				moves = CreateMovelistNode(move + 10, moves);
			}
			if(Player2.o > 0){
			moves = CreateMovelistNode(move, moves);
			}
		}

	 }
  }


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
  /*printf("start domove");
  int player, col, bit, mask, i, x;
  char piece;
  i = (TNO_HEIGHT+1)*(TNO_WIDTH);
  if(move - 10 > 0){
	piece = 't';
	col = move - 10;
  }
  else{
	piece = 'o';
	col = move;
  }
  player = WhoseTurn();
  printf("beginning");
  
  //unsigned long int temppos = position;
  //temppos >>= bit - (TNO_HEIGHT+1);
  //temppos 
  
  for(bit = col*(TNO_HEIGHT+1); position >> bit == 0; bit--)
  
  printf("\n%i\n", bit);

  mask = 1 << (bit+1);
  position = position | mask;

  mask = 0;
  for(x = 1; x <= i; x++){
	 mask <<= 1;
	if(x != bit || piece == 't') mask += 1;
  }
  position = position & mask;

  if(player == 1){
	 if(piece == 't'){
	  for(x = 1; x <= i + 3; x++){
		 mask <<= 1;
		mask += 1;
	  }
	  position = position & mask;
	  mask = (Player1.t - 1) << (i + 3);
	  position = position | mask;
	}
	else{
	  for(x = 1; x <= i; x++){
		 mask <<= 1;
		mask += 1;
	  }
	  for(x = 1; x<=3; x++){
		 mask <<= 1;
	  }
	  for(x = 1; x<=3; x++){
		 mask <<= 1;
		mask += 1;
	  }
	  position = position & mask;
	  mask = (Player1.o - 1) << i;
	  position = position | mask;
	 }
  }
  printf("end domove");*/
  
	int h, col;
	char piece;
	
	if(move - 10 > 0){
		piece = 't';
		col = move - 10;
	}
	else{
		piece = 'o';
		col = move;
	}
	
	col--;
	
	
	for (h=col*(TNO_HEIGHT+1)+TNO_HEIGHT; (position & (1 << h)) == 0; h--)
	
	printf("\n%i\n", h);
	
	if(piece == 'o'){
		position = position | (1 << (h + 1));
			
	}										//Change Board
	else{
		position = position | (1<< (h+1));
		position = position & (position - (1 << h));
	}
	
	unsigned long int pos = position;
	
	printf("%li", pos);
	
	
	return position;
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

  printf("start primitive");
  TOBlank board[TNO_WIDTH][TNO_HEIGHT+1];
  int col,row, player1=1, t=1, blank=0, count=0;
  PositionToBoard(position, board); // Temporary storage.


/*  for (col=0;col<TNO_WIDTH;col++)
	 board[col][TNO_HEIGHT]=2;
  for (col=0;col<TNO_WIDTH;col++)
	 for (row=0;row<TNO_HEIGHT;row++)
		board[col][row] = gPosition.board[col][row];*/

  // Check for toot/otto on the columns
  for(col=0; col<TNO_WIDTH-1; col++)
	 for(row=0; row<(TNO_HEIGHT-3); row++)
		if(   (board[col][row]==board[col][row+3])
			&& (board[col][row+1]==board[col][row+2])
			&& (board[col][row] != board[col][row+2])
			&& (board[col][row]!=blank)
			&& (board[col][row+1]!=blank))
		{
			if(board[col][row]==t)                   //Checks if its toot or otto
				if(WhoseTurn(position)==player1)      //then checks whose turns it
					return win;                        //is to return right thing
				else                        //(WhoseTurn(position)==player2)
					return lose;
			else                           //(board[col]row]==o)
				if(WhoseTurn(position)==player1)
					return win;
				else                        //(WhoseTurn(position)==player2)
					return lose;
		}


  // Check for toot/otto on the rows
  for(row=0; row<TNO_HEIGHT-1; row++)
	 for(col=0; col<(TNO_WIDTH-3); col++)
		if(    (board[col][row]==board[col+3][row])
			&& (board[col+1][row]==board[col+2][row])
			&& (board[col][row] != board[col+2][row]) 
			&& (board[col][row]!=blank)
			&& (board[col+1][row]!=blank))
		  {
			if(board[col][row]==t)                   //Checks if its toot or otto
				if(WhoseTurn(position)==player1)      //then checks whose turns it
					return win;                        //is to return right thing
				else                        //(WhoseTurn(position)==player2)
					return lose;
			else                           //(board[col]row]==o)
				if(WhoseTurn(position)==player1)
					return win;
				else                        //(WhoseTurn(position)==player2)
					return lose;
		} // if

  // Check for toot/otto on the diagonals
  for(row=0; row<(TNO_HEIGHT-3); row++)
	 for(col=0; col<(TNO_WIDTH-3); col++)
		if(   (board[col][row]==board[col+3][row+3])
			&& (board[col+1][row+1]==board[col+2][row+2])
			&& (board[col][row] != board[col+2][row+2])
            && (board[col][row]!=blank)
			&& (board[col+1][row+1]!=blank))
		{
			if(board[col][row]==t)                   //Checks if its toot or otto
				if(WhoseTurn(position)==player1)      //then checks whose turns it
					return win;                        //is to return right thing
				else                        //(WhoseTurn(position)==player2)
					return lose;
			else                           //(board[col]row]==o)
				if(WhoseTurn(position)==player1)
					return win;
				else                        //(WhoseTurn(position)==player2)
					return lose;
		} // if

  for(row=4; row<TNO_HEIGHT; row++){
	 for(col=0; col<TNO_WIDTH; col++){
		if(   (board[col][row]==board[col+3][row-3])
			&& (board[col+1][row-1]==board[col+2][row-2])
			&& (board[col][row] != board[col+2][row-2])
			&& (board[col][row]!=blank)
			&& (board[col+1][row-1]!=blank))
		{
			if(board[col][row]==t){                   //Checks if its toot or otto
				if(WhoseTurn(position)==player1){      //then checks whose turns it
					return win;
				}                        //is to return right thing
				else{                       //(WhoseTurn(position)==player2)
					return lose;
				}
			}
			else{                          //(board[col]row]==o)
				if(WhoseTurn(position)==player1){
					return win;
				}
				else{                        //(WhoseTurn(position)==player2)
					return lose;
				}
			}
		} // if
	 }
  }

  // If board is filled and there is no winner, game is tie.
  for (col=0;col<TNO_WIDTH;col++){
	 for (row=0;row<TNO_HEIGHT;row++){
		if (board[col][row]==0 || board[col][row]==1){
			count++;
		}
	 }
  }

  if(count == (TNO_HEIGHT*TNO_WIDTH)){
		return tie;
	}
  // If no win and board is not filled, its undecided
  
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
	int i, row, playerTurn;
	TOBlank board[TNO_WIDTH][TNO_HEIGHT+1];
	PositionToBoard(position,board);


	printf("\n	+-------------+");
	printf("\n	|Current Board|");
	printf("\n	+-------------+");
	printf("\n	|             |");
	printf("\n	|  ");
	for(i=1; i<=TNO_WIDTH; i++)
		printf(" %i", i);
	printf("   |");
	printf("\n	|  ");
	for(i=1; i<=TNO_WIDTH; i++)
		printf(" |");
	printf("   |");
	printf("\n	|  ");
	for(i=1; i<=TNO_WIDTH; i++)
		printf(" V");
	printf("   |");
	for(row=TNO_HEIGHT-1; row>=0; row--){
		printf("\n	|  ");
		for(i=0; i<TNO_WIDTH; i++)
			printf(" %s", gBlankTOString[(int)board[i][row]]);
		printf("   |");
	}
	printf("\n	|             |");
	printf("\n	+-------------+");

	printf("\n");
	PositionToPieces(position);
	playerTurn = WhoseTurn(position);
	printf("%s's turn\n", playersName);
	if(playerTurn == 1)
	{
		printf("    You have T:%i, O:%i.\n",Player1.t, Player1.o);
		printf("    OTTO has T:%i, O:%i.\n",Player2.t, Player2.o);
	}
	else
	{
		printf("    You have T:%i, O:%i.\n",Player2.t, Player2.o);
		printf("    TOOT has T:%i, O:%i.\n",Player1.t, Player1.o);
	}
	//printf("\n	%8s's move", playersName);
		  //printf("	Prediction: %s", GetPrediction(position, playersName, usersTurn);

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

void PrintComputersMove(MOVE computersMove, STRING computersName)
{
	if(computersMove-10<0)
		printf("%8s's move              : %io", computersName, computersMove);
	else
		printf("%8s's move              : %it", computersName, (computersMove-10));
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

void PrintMove (MOVE move){
	if(move-10<0)
		printf("%io", move);
	else
		printf("%it", (move-10));
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
	 
	 USERINPUT ret;

	do {
		printf("%8s's move [(u)ndo/1-4 T or O] :  ", playersName);
    
		ret = HandleDefaultTextInput(position, move, playersName);
		if(ret != Continue)
			return(ret);
    
	}
	while (TRUE);
	return(Continue);
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
	printf("start validtextinput");
	int column;
	char piece;
	sscanf(input, "%d%c", &column, &piece);
	
	if(column > 0 && column <= TNO_WIDTH && (piece=='t' || piece=='T' || piece=='o' || piece=='O')){
			return TRUE;
	}
	else{
		return FALSE;
	}
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
	int column, theMove=0;
	char piece;
	sscanf(input, "%d%c", &column, &piece);
	
	if(piece=='t' || piece=='T')
		theMove += 10;
	theMove += column;
	return theMove;
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

