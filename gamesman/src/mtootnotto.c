/************************************************************************
**
** NAME:        mtootnotto.c
**
** DESCRIPTION: Toot and Otto
**
** AUTHOR:      Kyler Murlas and Zach Wasserman
**
** DATE:        2006-05-09
**
** UPDATE HIST: RECORD CHANGES YOU HAVE MADE SO THAT TEAMMATES KNOW
**
**              2005-11-28  Added Primitive
**				2006-04-05  Edited Primitive to player1 wins if toot and
**							player2 wins if otto. Also fixed tie condition and
**							default output is now undecided if no win/lose/tie.
**				2006-04-08	Added a lot of printfs for debugging. Gets bus error
**							with GetAndPrintPlayersMove
**				2006-04-11  Fixed bus error and all warnings. DoMove working
**							correctly now. Primitive needs rewrite.
**				2006-04-14  Fixed part of problem with primitive. Doesn't
**							work perfectly, but will solve and it's playable
**							on a 4x4 board. Will not work for bigger boards.
**							number of starting pieces is now a variable.
**				2006-04-15	Fixed primitive, should be working correctly now.
**							Found bug in primitive. Added #define for move
**							hashing and unhashing. Debugging...
**				2006-04-16	Everything seems to be working correctly. Primitive
**							needs to be changed so when a move causes both 
**							players to win it returns tie. or draw?
**				2006-04-21	Primitive now handles both players winning at
**							the same time. It returns a tie. PrintPosition
**							has improvements suggested. Added GameSpecificMenu.
**				2006-05-09	Added MoveToString.
**				2006-05-22	Edited help strings. Fixed primitive to return tie
**							if both players are out of pieces and there is no
**							win or lose.
**				2006-08-19	Changed scanf to use GetMyInt(), bounds checking still needs to be done
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

BOOLEAN  kPartizan            = TRUE ; 
BOOLEAN  kGameSpecificMenu    = TRUE ; 
BOOLEAN  kTieIsPossible       = TRUE ; 
BOOLEAN  kLoopy               = FALSE; 
BOOLEAN  kDebugMenu           = TRUE ; 
BOOLEAN  kDebugDetermineValue = TRUE ; 

POSITION gNumberOfPositions   =  0; 
POSITION gInitialPosition     =  0; 
POSITION kBadPosition         = -1;

void*	 gGameSpecificTclInit = NULL;

/*
 * Help strings that are pretty self-explanatory
 * Strings than span more than one line should have backslashes (\) at the end of the line.
 */

STRING kHelpGraphicInterface =
"Not written yet";

STRING   kHelpTextInterface    =
"On your turn enter the number of the column (each number corresponds to \n\
its respective column) and the piece you would like to place there (t or o). \n\
Below the board is a count of how many Ts and Os that you are still able \n\
to place. If at any point you have made a mistake, you can type u and hit \n\
return and the system will revert back to your most recent position.";

STRING   kHelpOnYourTurn =
"Enter the number of the column and the type of piece, t or o. The piece \n\
that you chose will be 'dropped' into that column on top of any other \n\
pieces, if there are any.";

STRING   kHelpStandardObjective =
"To get your name, either Toot or Otto depending on which player you are, \n\
spelled on the board vertically, horizontally, or diagonally. First player \n\
to spell their name WINS. The first player is Toot, second player Otto.";

STRING   kHelpReverseObjective =
"To force your opponent to spell their name, either Toot or Otto depending \n\
on which player you are, spelled on the board vertically, horizontally, or \n\
diagonally. First player to spell their name LOSES.\n\
The first player is Toot, second player Otto.";

STRING   kHelpTieOccursWhen =
"the board is filled and there is no winner\n\
OR both players win with one move\n\
OR both players run out of pieces.";

STRING   kHelpExample ="";
/*"        +-------------+\n\
        |             |												\n\
        |   1 2 3 4   | You (OTTO) have:        TTTT    OOOO		\n\
        |   | | | |   | Player 2 (TOOT) has:    TTTT    OOOO		\n\
        |   V V V V   |         Player's turn						\n\
        |   - - - -   |												\n\
        |   - - - -   |												\n\
        |   - - - -   |												\n\
        |   - - - -   |    (Player should Tie in 16)				\n\
        |             |												\n\
        +-------------+												\n\n\

  Player's move [(u)ndo/1-4 T or O]     :  1t						\n\n\

        +-------------+												\n\
        |             |												\n\
        |   1 2 3 4   |    Player 1 (OTTO) has: TTT     OOOO		\n\
        |   | | | |   | Player 2 (TOOT) has:    TTTT    OOOO		\n\
        |   V V V V   |         Data's turn							\n\
        |   - - - -   |												\n\
        |   - - - -   |												\n\
        |   - - - -   |												\n\
        |   T - - -   |    (Data will Win in 13)					\n\
        |             |												\n\
        +-------------+												\n\n\

    Data's move                         :  4t						\n\
        +-------------+												\n\
        |             |												\n\
        |   1 2 3 4   | You (OTTO) have:        TTT     OOOO		\n\
        |   | | | |   | Player 2 (TOOT) has:    TTT     OOOO		\n\
        |   V V V V   |         Player's turn						\n\
        |   - - - -   |												\n\
        |   - - - -   |												\n\
        |   - - - -   |												\n\
        |   T - - T   |    (Player will Lose in 12)					\n\
        |             |												\n\
        +-------------+												\n\n\

  Player's move [(u)ndo/1-4 T or O]     :  2o						\n\n\

        +-------------+												\n\
        |             |												\n\
        |   1 2 3 4   |    Player 1 (OTTO) has: TTT     OOO			\n\
        |   | | | |   | Player 2 (TOOT) has:    TTT     OOOO		\n\
        |   V V V V   |         Data's turn							\n\
        |   - - - -   |												\n\
        |   - - - -   |												\n\
        |   - - - -   |												\n\
        |   T O - T   |    (Data will Win in 1)						\n\
        |             |												\n\
        +-------------+												\n\n\

    Data's move                         :  3o						\n\
        +-------------+												\n\
        |             |												\n\
        |   1 2 3 4   | You (OTTO) have:        TTT     OOO			\n\
        |   | | | |   | Player 2 (TOOT) has:    TTT     OOO			\n\
        |   V V V V   |         Player's turn						\n\
        |   - - - -   |												\n\
        |   - - - -   |												\n\
        |   - - - -   |												\n\
        |   T O O T   |    (Player will Lose in 0)					\n\
        |             |												\n\
        +-------------+												\n\n\n\


Data (player two) Wins!";*/



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

#define moveUnhashPiece(move) move > 10 ? 't' : 'o' /*returns piece defined by hashed move*/
#define moveUnhashCol(move) move > 10 ? move-10 : move /*returns column defined by hashed move*/
#define hashMove(col, piece) piece == 't' || piece == 'T' ? col+10 : col /*hashes move*/

/*************************************************************************
**
** Global Variables
**
*************************************************************************/
int TNO_WIDTH = 4;
int TNO_HEIGHT = 4;
int INIT_T = 4;  //Cannot exceed 7
int INIT_O = 4;  //Cannot exceed 7

/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/

/* External */
extern GENERIC_PTR	SafeMalloc ();
extern void		SafeFree ();
STRING MoveToString(MOVE);


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
	POSITION p=0;
	int i;
	p+=INIT_T;
	p = (p << 3);
	p+=INIT_O;
  
	for (i=1;i<=TNO_WIDTH;++i)
		p = (p << (TNO_HEIGHT+1))+1;
	
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
  unsigned long size=1;
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
	gMoveToStringFunPtr = &MoveToString;
}


void PiecesOnBoard(pos)
	 POSITION pos;
{
  Board.t = 0;
  Board.o = 0;
  Board.total = 0;
  
  int row, col;
  TOBlank board[TNO_WIDTH][TNO_HEIGHT+1];
  PositionToBoard(pos,board);
  
  for (row=TNO_HEIGHT-1;row>=0;row--) {
		for (col=0;col<TNO_WIDTH;col++){
			if(board[col][row] == 0) Board.t++;
			else if(board[col][row] == 1) Board.o++;
		}
	}
	Board.total = Board.t + Board.o;
}


void PositionToPieces(pos)
	  POSITION pos;
{
  unsigned long i;
  i = (TNO_HEIGHT+1)*TNO_WIDTH;
  Player1.t = pos >> (i+3);
  Player1.o = (pos >> i) & 7;
  Player1.total = Player1.t + Player1.o;
  PiecesOnBoard(pos);
  Player2.t = INIT_T - (Board.t - (INIT_T - Player1.t));
  Player2.o = INIT_O - (Board.o - (INIT_O - Player1.o));
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
  if(Player1.total == Player2.total){ return 1;
  }
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
	int h, col, n;
	char piece;
	unsigned long int mask, p = 0;
	
	
	piece = moveUnhashPiece(move);
	col = moveUnhashCol(move);
	
	col--;
	
	int player = WhoseTurn(position);
	
	
	for (h=col*(TNO_HEIGHT+1)+TNO_HEIGHT; (position & (1 << h)) == 0; h--){}
	
	
	if(piece == 'o'){
		position = position | (1 << (h + 1));
			
	}										//Change Board
	else{
		position = position | (1<< (h+1));
		position = position & (position - (1 << h));
	}
	
	
	if(player == 1){			//change number of pieces (only if player 1 moved)
		
		mask = 1;
		for(n = 1; n<TNO_WIDTH*(TNO_HEIGHT+1); n++){
			mask = (mask << 1) + 1;
		}
		
		
		if(piece == 't'){
			p += Player1.t-1;
			p = p << 3;
			p += Player1.o;
		}
		else{
			p += Player1.t;
			p = p << 3;
			p += Player1.o-1;
		}
		
	
		p = p << (TNO_WIDTH*(TNO_HEIGHT+1));
		
		position = position & mask;
		position += p;
	}
	
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

  TOBlank board[TNO_WIDTH][TNO_HEIGHT+1];
  int col,row, player1=1, t=1, blank=2, count=0, ottoWins=0, tootWins=0;
  PositionToBoard(position, board); // Temporary storage.



  // Check for toot/otto on the columns
  for(col=0; col<TNO_WIDTH; col++)
  {
	 for(row=0; row<(TNO_HEIGHT-3); row++)
	 {
		if(   (board[col][row]   == board[col][row+3])
			&& (board[col][row+1] == board[col][row+2])
			&& (board[col][row]   != board[col][row+2])
			&& (board[col][row]   != blank) //check blanks
			&& (board[col][row+1] != blank)) //check blanks
		{
			//printf("Column Win [%i][%i]", col, row);
			if(board[col][row]==t)
			{
				tootWins++;
																  //Checks if its toot or otto
				/*if(WhoseTurn(position)==player1)    //then checks whose turns it
					return win;                        //is to return right thing
				else                        //(WhoseTurn(position)==player2)
					return lose;*/
			} // if
			else                           //(board[col]row]==o)
			{
				ottoWins++;

				/*if(WhoseTurn(position)==player1)
					return lose;
				else                        //(WhoseTurn(position)==player2)
					return win;*/
			} // else
		 } // if
	  } // row for
	}// col for


  // Check for toot/otto on the rows
  for(col=0; col<(TNO_WIDTH-3); col++)
  {
	 for(row=0; row<TNO_HEIGHT; row++)
	 {
		if(   (board[col][row]   == board[col+3][row])
			&& (board[col+1][row] == board[col+2][row])
			&& (board[col][row]   != board[col+2][row])
			&& (board[col][row]   != blank)
			&& (board[col+1][row] != blank))
		{
			//printf("Row Win [%i][%i]", col, row);
			if(board[col][row]==t)
			{
				tootWins++;
																  //Checks if its toot or otto
				/*if(WhoseTurn(position)==player1)    //then checks whose turns it
					return win;                        //is to return right thing
				else                        //(WhoseTurn(position)==player2)
					return lose; */
			} // if
			else                           //(board[col]row]==o)
			{
				ottoWins++;

				/*if(WhoseTurn(position)==player1)
					return lose;
				else                        //(WhoseTurn(position)==player2)
					return win;*/
			} // else
		 } // if
	  } // row for
	} // col for

  // Check for toot/otto on the diagonals
  for(col=0; col<(TNO_WIDTH-3); col++)
  {
	 for(row=0; row<(TNO_HEIGHT-3); row++)
	 {
		if(   (board[col][row]     == board[col+3][row+3])
			&& (board[col+1][row+1] == board[col+2][row+2])
			&& (board[col][row]     != board[col+2][row+2])
			&& (board[col][row]     != blank)
			&& (board[col+1][row+1] != blank))
		{
			//printf("Diag A Win [%i][%i]", col, row);
			if(board[col][row]==t)
			{
				tootWins++;
																  //Checks if its toot or otto
				/*if(WhoseTurn(position)==player1)    //then checks whose turns it
					return win;                        //is to return right thing
				else                        //(WhoseTurn(position)==player2)
					return lose;*/
			} // if
			else                           //(board[col]row]==o)
			{
				ottoWins++;
				/*if(WhoseTurn(position)==player1)
					return lose;
				else                        //(WhoseTurn(position)==player2)
					return win;*/
			} // else
		 } // if
	  } // row for
	} // col for

  for(col=0; col<(TNO_WIDTH-3); col++)
  {
	 for(row=3; row<TNO_HEIGHT; row++)
	 {
		if(   (board[col][row]     == board[col+3][row-3])
			&& (board[col+1][row-1] == board[col+2][row-2])
			&& (board[col][row]     != board[col+2][row-2])
			&& (board[col][row]     != blank)
			&& (board[col+1][row-1] != blank))
		{
			//printf("Diag B Win [%i][%i]", col, row);
			if(board[col][row]==t)
			{
				tootWins++;
															//Checks if its toot or otto
				/*if(WhoseTurn(position)==player1) //then checks whose turns it
				{
					return win;
				} //if                     //is to return right thing
				else                       //(WhoseTurn(position)==player2)
					return lose;*/
			} // if
			else                          //(board[col]row]==o)
			{
				ottoWins++;

				/*if(WhoseTurn(position)==player1)
					return lose;
				else                        // then (WhoseTurn(position)==player2)
					return win;*/
			} // else
		 } // if
	  } // row for
	} // col for
	if((tootWins != 0)&&(tootWins == ottoWins))
		return tie;
	else if((tootWins > ottoWins))
	{
		if(WhoseTurn(position)==player1)
			return win;
		else                        // then (WhoseTurn(position)==player2)
			return lose;
	}
	else if((ottoWins > tootWins))
	{
		if(WhoseTurn(position)==player1)
			return lose;
		else                        // then (WhoseTurn(position)==player2)
			return win;
	}

	// If board is filled and there is no winner, game is tie.
	for (col=0;col<TNO_WIDTH;col++){
		for (row=0;row<TNO_HEIGHT;row++){
			if (board[col][row]==0 || board[col][row]==1){
				count++;
			} // if
		} // for
	} // for

	if(count == (TNO_HEIGHT*TNO_WIDTH)){
		return tie;
	} // if
	// If no win and board is not filled, its undecided
	
	PiecesOnBoard(position);
	if(Board.total == (INIT_T + INIT_O) * 2) return tie;

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

void PrintPosition(POSITION position,STRING playerName,BOOLEAN usersTurn)
{
	int i, row, playerTurn, pieceCount;
	TOBlank board[TNO_WIDTH][TNO_HEIGHT+1];
	PositionToBoard(position,board);
	playerTurn = WhoseTurn(position);
	PositionToPieces(position);
	
	printf("\n	+");
	for(i=1; i<6+2*TNO_WIDTH; i++)printf("-");
	printf("+");
	printf("\n	|");
	for(i=1; i<6+2*TNO_WIDTH; i++)printf(" ");
	printf("|");
	printf("\n	|  ");
	for(i=1; i<=TNO_WIDTH; i++)
		printf(" %i", i);
	printf("   |");
	
	//Print player 1's pieces
	if(usersTurn && playerTurn == 1){
		printf("	You (OTTO) have:	");
	}
	else{
		printf("    Player 1 (OTTO) has:	");
	}
	for(pieceCount = 1; pieceCount <= INIT_T; pieceCount++)
		if(Player1.t >= pieceCount)printf("T");
	printf("	");
	for(pieceCount = 1; pieceCount <= INIT_O; pieceCount++)
		if(Player1.o >= pieceCount)printf("O");
	//finish printing pieces
	
	printf("\n	|  ");
	for(i=1; i<=TNO_WIDTH; i++)
		printf(" |");
	printf("   |");
	
	//Print player 2's pieces
	if(usersTurn && playerTurn == 2){
			printf("	You (TOOT) have:	");
	}
	else{
		printf("	Player 2 (TOOT) has:	");
	}
	for(pieceCount = 1; pieceCount <= INIT_T; pieceCount++)
		if(Player2.t >= pieceCount)printf("T");
	printf("	");
	for(pieceCount = 1; pieceCount <= INIT_O; pieceCount++)
		if(Player2.o >= pieceCount)printf("O");
	//finish printing pieces
	
	printf("\n	|  ");
	for(i=1; i<=TNO_WIDTH; i++)
		printf(" V");
	printf("   |");
	printf("		%s's turn", playerName);
	for(row=TNO_HEIGHT-1; row>=0; row--){
		printf("\n	|  ");
		for(i=0; i<TNO_WIDTH; i++)
			printf(" %s", gBlankTOString[(int)board[i][row]]);
		printf("   |");
	}
	printf("	   %s", GetPrediction(position,playerName,usersTurn));
	printf("\n	|");
	for(i=1; i<6+2*TNO_WIDTH; i++)printf(" ");
	printf("|");
	printf("\n	+");
	for(i=1; i<6+2*TNO_WIDTH; i++)printf("-");
	printf("+");
	
	printf("\n\n");


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
	printf("%8s's move				:  %i%c\n",
	computersName, moveUnhashCol(computersMove), moveUnhashPiece(computersMove));
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
	printf("%i%c", moveUnhashCol(move), moveUnhashPiece(move));
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
		printf("%8s's move [(u)ndo/1-4 T or O]	:  ", playersName);
    
		ret = HandleDefaultTextInput(position, move, playersName);
		if(ret != Continue)
			return(ret);
    
	}
	while (TRUE);
	return(Continue);
}


/************************************************************************
**
** NAME:        MoveToString
**
** DESCRIPTION: Returns the move as a STRING
** 
** INPUTS:      MOVE *theMove         : The move to put into a string.
**
************************************************************************/

STRING MoveToString (theMove)
     MOVE theMove;
{
  STRING move = (STRING) SafeMalloc(4);
  
  sprintf(move, "%d%c", moveUnhashCol(theMove), moveUnhashPiece(theMove)); 
  return move;
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
	int column;
	char piece;
	sscanf(input, "%i%c", &column, &piece);
	return hashMove(column, piece);
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
	do {
		printf("?\n\t----- Game-specific options for %s -----\n\n", kGameName);
		printf("\tw)\tChoose the board (W)idth Currently: %d\n",TNO_WIDTH);
		printf("\th)\tChoose the board (H)eight Currently: %d\n",TNO_HEIGHT);
		printf("\t4x4 solves, 4x5 works player vs. player, 4x6 does not work\n\n");
		printf("\tt)\tChoose the starting # of Ts Currently: %d\n", INIT_T);
		printf("\to)\tChoose the starting # of Os Currently: %d\n", INIT_O);
		printf("\n\n\tb)\t(B)ack = Return to previous activity.\n");
		printf("\n\nSelect an option: ");
		
		switch(GetMyChar()) {
		case 'Q': case 'q':
			ExitStageRight();
			break;
		case 'W' : case 'w':
			printf("Enter a width: ");
			TNO_WIDTH = GetMyInt();
			break;
		case 'H': case 'h':
			printf("Enter a height: ");
			TNO_HEIGHT = GetMyInt();
			break;
		case 'T': case 't':
			printf("Enter # of Ts (less than 7): ");
			scanf("%d", &temp);
			INIT_T = temp;
			break;
		case 'O': case 'o':
			printf("Enter # of Os (less than 7): ");
			INIT_O = GetMyInt();
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
    return MyInitialPosition();
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

