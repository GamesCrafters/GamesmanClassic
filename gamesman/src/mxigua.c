// $id$
// $log$

/*
 * The above lines will include the name and log of the last person
 * to commit this file to CVS
 */

/************************************************************************
**
** NAME:        xigua.c 
**
** DESCRIPTION: XiGua 
**
** AUTHOR:      Joshua Kocher, Daniel Honegger, Gerardo Snyder
**
** DATE:        1.27.05 - ???? 
**
** UPDATE HIST: RECORD CHANGES YOU HAVE MADE SO THAT TEAMMATES KNOW
** 		-- 1.27.05 -- Created file and started adding in game info. -JK
**		-- 1.30.05 -- Added to displayasciitable.
** 			      Filled in most of PrintPosition.
**                            Still need code to unhash.  -JK
**		-- 2.9.05 -- Filled in code for the following functions:
** 			     - NumberOfOptions()
**			     - getOption()
**                           - setOption()
**              -- 2.23.05 -- Fixed Initialize game (set gNumberOfPositions and gInitialPosition)
**                            Filled in DoMove and GenerateMoves();
**                            Actually allocated space for the board in PrintPosition
**              -- 3.1.05 -- Added in wrapped hash functions
**			     Wrote Primitive()
** 		  	     Changed functions to reflect the new hash functions
**			     Get a floating point exception now though when trying to go into the debug
**			     menu.
**			     Also cannot represent more than 6 empty spaces...maybe something to do with
**		             my new hash functions. Need the symmetries.
**              -- 3.01.05 -- Switched to SafeMalloc and SafeFree
**                            Made #define NUM_HASHED_WRAPPER_BITS to hold '5' for use in hash wrapper
**                            Filled in GenerateMoves to be more selective (use isValidMove()) (incomplete)
**                             - uses adjacency information implemented for DoMove
**                             - uses isSurrounded
**                            DoMove now captures stones; requires helpers
**                             - isSurrounded
**                             - removeStones
**                             - adjacency information initialized in InitializeGame
**                            Note: solving algorithm does not account for "pass" ability - plays out all outcomes to unquestionable end (unlike game of Go between humans) to enable this, use Chinese rules for Go territory counting
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

/* didn't see hash.h included in gamesman.h */
#include "hash.h"

/*************************************************************************
**
** Game-specific constants
**
**************************************************************************/

STRING   kGameName            = "XiGua"; /* The name of your game */
STRING   kAuthorName          = "Joshua Kocher, Daniel Honegger, Gerardo Snyder"; /* Your name(s) */
STRING   kDBName              = "xigua.db"; /* The name to store the database under */

BOOLEAN  kPartizan            = FALSE ; /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN  kGameSpecificMenu    = FALSE ; /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN  kTieIsPossible       = FALSE ; /* TRUE if a tie is possible. FALSE if it is impossible.*/
BOOLEAN  kLoopy               = FALSE ; /* TRUE if the game tree will have cycles (a rearranger style game). FALSE if it does not.*/

BOOLEAN  kDebugMenu           = TRUE ; /* TRUE only when debugging. FALSE when on release. */
BOOLEAN  kDebugDetermineValue = TRUE ; /* TRUE only when debugging. FALSE when on release. */

POSITION gNumberOfPositions   =  0; /* The number of total possible positions | If you are using our hash, this is given by the hash_init() function*/
POSITION gInitialPosition     =  0; /* The initial hashed position for your starting board */
POSITION kBadPosition         = -1; /* A position that will never be used */

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

#define NUM_HASH_WRAPPER_BITS 5

/* Game options */
int boardsize; /* 0-4 Board size is 5 + 4 * boardsize */
int rulesvariant; /* 0/1 normal rules / inverted rules */
int handicapping; /* 0/1 Handicapped or not */
int towin; /* 0-2  0 - counting territory, 1 - captured pieces, 2 - both */

/* max board size */
int maxsize;

char *board;

/*struct with adjacency information - built in Initialize Game */
typedef struct adjacency_info {
  int numAdjacent;
  MOVE *adj;
} adjacency;

adjacency *adjacent;

/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/

/* External */
extern GENERIC_PTR	SafeMalloc ();
extern void		SafeFree ();

void* gGameSpecificTclInit = NULL;

/* Internal */
void displayasciiboard(char *, char *);
int NumberOfOptions();
int getOption();
void setOption(int);
char *emptyboard(char *);
POSITION hash(char *,int, int);
char *unhashboard(POSITION,char *);
int hash_init(int, int*, int (*vcfg_function_ptr)(int* cfg));
int getplayer(POSITION);
int getturnnumber(POSITION);
int countboard(char *,char);
char *getprediction(char *);
BOOLEAN isValidMove(POSITION, MOVE);      /* helper for GenerateMoves */
VALUE countWinner(POSITION);              /* helper for Primitive */
BOOLEAN isSurrounded(char *, MOVE, char, BOOLEAN *);  /* helper for DoMove, isValidMove */
void removeStones(char *, MOVE, char, BOOLEAN *) /* helper for DoMove */

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
	/* need to change this to reflect the board size */
        maxsize=5+4*boardsize;
	int piecesarray[]={'X',0,maxsize-1,'*',0,maxsize-1,'O',0,maxsize,-1};

	/* manually setting options right now */
	gNumberOfPositions=hash_init(maxsize, piecesarray, NULL);    /* initialize the hash */
	board = emptyboard(board);
	gInitialPosition=hash(board,1,0);

	adjacent = (adjacency *) SafeMalloc(sizeof(adjacency) * maxsize);

	switch(boardsize) {
	case 0: adjacent[0].numAdjacent = adjacent[1].numAdjacent = adjacent[3].numAdjacent = adjacent[4].numAdjacent = 3;
	  adjacent[2].numAdjacent = 4;
	  adjacent[0].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[0].numAdjacent);
	  adjacent[0].adj[0] = 1;
	  adjacent[0].adj[1] = 2;
	  adjacent[0].adj[2] = 3;
	  adjacent[1].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[1].numAdjacent);
	  adjacent[1].adj[0] = 0;
	  adjacent[1].adj[1] = 2;
	  adjacent[1].adj[2] = 4;
	  adjacent[3].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[3].numAdjacent);
	  adjacent[3].adj[0] = 0;
	  adjacent[3].adj[1] = 2;
	  adjacent[3].adj[2] = 4;
	  adjacent[4].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[4].numAdjacent);
	  adjacent[4].adj[0] = 1;
	  adjacent[4].adj[1] = 2;
	  adjacent[4].adj[2] = 3;
	  adjacent[2].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[0].numAdjacent);
	  adjacent[2].adj[0] = 0;
	  adjacent[2].adj[1] = 1;
	  adjacent[2].adj[2] = 3;
	  adjacent[2].adj[3] = 4;
	  break;
	case 1:
	  break;
	case 2:
	  break;
	case 3: /* all these cases aren't symmetrical, and I don't like copy pasting that much */
	  break;
	case 4: adjacent[0].numAdjacent = adjacent[1].numAdjacent = adjacent[2].numAdjacent = \
		  adjacent[4].numAdjacent = adjacent[6].numAdjacent = adjacent[7].numAdjacent = \
		  adjacent[13].numAdjacent = adjacent[14].numAdjacent = adjacent[16].numAdjacent = \
		  adjacent[18].numAdjacent = adjacent[19].numAdjacent = adjacent[20].numAdjacent = 3;
                adjacent[3].numAdjacent = adjacent[5].numAdjacent = adjacent[8].numAdjacent = \
		  adjacent[9].numAdjacent = adjacent[10].numAdjacent = adjacent[11].numAdjacent = \
		  adjacent[12].numAdjacent = adjacent[15].numAdjacent = adjacent[17].numAdjacent = 4;
		adjacent[0].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[0].numAdjacent);
		adjacent[0].adj[0] = 1;
		adjacent[0].adj[1] = 3;
		adjacent[0].adj[2] = 4;
		adjacent[1].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[1].numAdjacent);
		adjacent[1].adj[0] = 0;
		adjacent[1].adj[1] = 2;
		adjacent[1].adj[2] = 3;
		adjacent[2].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[2].numAdjacent);
		adjacent[2].adj[0] = 1;
		adjacent[2].adj[1] = 3;
		adjacent[2].adj[2] = 6;
		adjacent[4].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[4].numAdjacent);
		adjacent[4].adj[0] = 0;
		adjacent[4].adj[1] = 7;
		adjacent[4].adj[2] = 8;
		adjacent[6].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[6].numAdjacent);
		adjacent[6].adj[0] = 2;
		adjacent[6].adj[1] = 12;
		adjacent[6].adj[2] = 13;
		adjacent[7].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[7].numAdjacent);
		adjacent[7].adj[0] = 4;
		adjacent[7].adj[1] = 8;
		adjacent[7].adj[2] = 14;
		adjacent[13].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[13].numAdjacent);
		adjacent[13].adj[0] = 6;
		adjacent[13].adj[1] = 12;
		adjacent[13].adj[2] = 16;
		adjacent[14].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[14].numAdjacent);
		adjacent[14].adj[0] = 7;
		adjacent[14].adj[1] = 8;
		adjacent[14].adj[2] = 18;
		adjacent[16].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[16].numAdjacent);
		adjacent[16].adj[0] = 12;
		adjacent[16].adj[1] = 13;
		adjacent[16].adj[2] = 20;
		adjacent[18].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[18].numAdjacent);
		adjacent[18].adj[0] = 14;
		adjacent[18].adj[1] = 17;
		adjacent[18].adj[2] = 19;
		adjacent[19].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[19].numAdjacent);
		adjacent[19].adj[0] = 17;
		adjacent[19].adj[1] = 18;
		adjacent[19].adj[2] = 20;
		adjacent[20].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[20].numAdjacent);
		adjacent[20].adj[0] = 16;
		adjacent[20].adj[1] = 17;
		adjacent[20].adj[2] = 19;
		adjacent[3].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[3].numAdjacent);
		adjacent[3].adj[0] = 0;
		adjacent[3].adj[1] = 1;
		adjacent[3].adj[2] = 2;
		adjacent[3].adj[3] = 5;
		adjacent[5].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[5].numAdjacent);
		adjacent[5].adj[0] = 3;
		adjacent[5].adj[1] = 9;
		adjacent[5].adj[2] = 10;
		adjacent[5].adj[3] = 11;
		adjacent[8].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[8].numAdjacent);
		adjacent[8].adj[0] = 4;
		adjacent[8].adj[1] = 7;
		adjacent[8].adj[2] = 9;
		adjacent[8].adj[3] = 14;
		adjacent[9].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[9].numAdjacent);
		adjacent[9].adj[0] = 5;
		adjacent[9].adj[1] = 8;
		adjacent[9].adj[2] = 10;
		adjacent[9].adj[3] = 15;
		adjacent[10].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[10].numAdjacent);
		adjacent[10].adj[0] = 5;
		adjacent[10].adj[1] = 9;
		adjacent[10].adj[2] = 11;
		adjacent[10].adj[3] = 15;
		adjacent[11].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[11].numAdjacent);
		adjacent[11].adj[0] = 5;
		adjacent[11].adj[1] = 10;
		adjacent[11].adj[2] = 12;
		adjacent[11].adj[3] = 15;
		adjacent[12].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[12].numAdjacent);
		adjacent[12].adj[0] = 6;
		adjacent[12].adj[1] = 11;
		adjacent[12].adj[2] = 13;
		adjacent[12].adj[3] = 16;
		adjacent[15].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[15].numAdjacent);
		adjacent[15].adj[0] = 9;
		adjacent[15].adj[1] = 10;
		adjacent[15].adj[2] = 11;
		adjacent[15].adj[3] = 15;
		adjacent[17].adj = (MOVE *) SafeMalloc(sizeof(MOVE) * adjacent[17].numAdjacent);
		adjacent[17].adj[0] = 15;
		adjacent[17].adj[1] = 18;
		adjacent[17].adj[2] = 19;
		adjacent[17].adj[3] = 20;
		break; /*NOTE: need to free all this she-bop */
	default:
	  printf("We should never get here inside InitializeGame while creating the adjacency table information\n");
	  break;
	}
}

char * emptyboard(char *board) {
	int i;

	board = (char *) SafeMalloc(sizeof(char) * maxsize);
	for(i=0;i<maxsize;i++) ret[i]='O';

	return board;
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
    MOVELIST *moves = NULL; /* has fields move and next */
	int i;
   
	board=emptyboard();	 
    /* Use CreateMovelistNode(move, next) to 'cons' together a linked list */
	/* wow, talk about making something foolproof for having students coding games */
	board=unhashboard(position,board);
	for(i=0;i<maxsize;i++) {
		if(board[i]=='O' && isValidMove(position, (MOVE) i)) {
			moves = CreateMovelistNode((MOVE)i,moves);
		}
	}   	

    return moves;
}

/* Generate Moves helper function isValidMove */
BOOLEAN isValidMove(POSITION pos, MOVE mv) {

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
	char piece;
	int player, turn;
	
	board=unhash(position,board);
	player=getplayer(position);
	turn=getturnnumber(position);
	if(player==1) {
		piece='X';
	} else if (player==2) {
		piece='*';
		turn++; /* sneak incrementing the turn, because next it will be player 1's turn */
	} else {
		fprintf(stderr,"Bad else: Do Move (Piece selection)\n");
		exit(1);
	}
	
	board[move]=piece;

	/* check for capture */
	for(i = 0; i < adjacent[move].numAdjacent; i++)
	  if(board[adjacent[move].adj[i]] == oPiece && !checked[adjacent[move].adj[i]]) {
	    checked[adjacent[move].adj[i]] = TRUE;
	    if(isSurrounded(board, adjacent[move].adj[i], oPiece, checked)) {
	      for(j = 0; j < maxsize; j++)
		checked[j] = FALSE;
	      removeStones(board, adjacent[move].adj[i], oPiece, checked);
	    }
	  }

	player^=3; /* slickness, 1 becomes 2 becomes 1.... */	
    	position = hash(board,player,turn); 

	return position;
}

BOOLEAN isSurrounded(char *board, MOVE move, char p, BOOLEAN *check) {
  if(board[move] != p)
    return (board[move] == 'O') ? FALSE : TRUE;
  else {
    int i;
    BOOLEAN ret = TRUE;

    for(i = 0; i < adjacent[move].numAdjacent && ret; i++)
      if(!check[adjacent[move].adj[i]]) {
	check[adjacent[move].adj[i]] = TRUE;
	ret = (ret && isSurrounded(board, adjacent[move].adj[i], p, check));
      }

    return ret;
  }
}

void removeStones(char *board, MOVE move, char p, BOOLEAN *check) {
  if(board[move] == p) {
    checked[move] = TRUE;
    board[move] = 'O';

    for(i = 0; i < adjacent[move].numAdjacent; i++)
      if(!check[adjacent[move].adj[i]])
	removeStones(board, adjacent[move].adj[i], p, check);
  }
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
	int player,turn;
	int p1c, p2c;
	VALUE ret;
	
	turn=getturnnumber(position);

	board=unhashboard(position,board);
	/* if we've ran out of pieces or there are no more spaces on the board */
	if(turn==maxsize-1 || countboard(board,'O')==0) {
		player=getplayer(position);
		p1c=countboard(board,'X');
		p2c=countboard(board,'*');
		/* if its player 2's turn and he's ahead, this spots a win */
		if(p1c<p2c && player==2) {
			ret=win;
		/* if its player 1's turn and he's ahead, this spots a tie */
		} else if(p1c>p2c && player==1) {
			ret=win;
		/* if they are equal, this spots a tie */
		} else if(p1c==p2c) {
			ret=tie;
		/* otherwise it must be a lose */
		} else {
			ret=lose;
		}
	} else {
		ret=undecided;
	}
	
    return ret;
}


/************************************************************************
** displayasciiboard() -- displays the board in a nice to view manner
** 
** Takes in a char array that represents each of the positions and a character
** array with the prediction.
**
** IMPORTANT: Prediction needs to be 15 characters, 15 is the number of characters
** for 16 is too much and 14 is too few. 15 is the number of characters needed
** for the output to look proper  -JK
**
** Returns nothing.
**
** Still need to add the prediction into the drawing. -JK
** 
*************************************************************************/

void displayasciiboard(char *positionvalues, char *prediction) {
	/* dirty but should work */
	char *pos = positionvalues; /* decided i didn't want to write positionvalues over and over */
        printf("Legend:    /1-2-3\\     Current:        /%c-%c-%c\\\n",pos[0],pos[1],pos[2]);
        printf("          /..\\|/..\\    Player1: *     /..\\|/..\\\n");
        printf("         /....4....\\   Player2: X    /....%c....\\\n",pos[3]);
        printf("        /.....|.....\\               /.....|.....\\\n");
        printf("       5......6......7             %c......%c......%c\n",pos[4],pos[5],pos[6]);
        printf("       |\\..../|\\..../|             |\\..../|\\..../|\n");
        printf("       8-9--A-B-C--D-E             %c-%c--%c-%c-%c--%c-%c\n",pos[7],pos[8],pos[9],pos[10],pos[11],pos[12],pos[13]);
        printf("       |/....\\|/....\\|             |/....\\|/....\\|\n");
        printf("       F......G......H             %c......%c......%c\n",pos[14],pos[15],pos[16]);
        printf("        \\.....|...../               \\.....|...../\n");
        printf("         \\....I..../                 \\....%c..../\n",pos[17]);
        printf("          \\../|\\../   Prediction:     \\../|\\../\n");
        printf("           \\J-K-L/  (%s)  \\%c-%c-%c/\n",prediction,pos[18],pos[19],pos[20]);

}

char *getprediction(char *pred) {
	int i=0;
	/* code to initialize it to blank values until a later time */
	for(;i<16;i++) pred[i]=(char)32; /* fill with spaces */
	pred[i]='\0';
	return pred;
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
	/* just a doodle of the gameboard
	 * - Josh
	 * 		/0-0-0\
         *             /..\|/..\  
	 *	      /....0....\
	 *           /.....|.....\
         *          0......0......0
         *          |\..../|\..../|
         *          0-0--0-0-0--0-0
         *          |/....\|/....\|
         *          0......0......0
         *           \.....|...../
         *            \....0..../
         *             \../|\../
         *              \0-0-0/
	 *
	 * all thanks to boring lectures =)
	 */
	char *toprint, *prediction;
	toprint=(char *)malloc(sizeof(char)*(5+4*boardsize));
	prediction=(char *)malloc(sizeof(char)*17); /* put my magic numbers by my magic markers */
	/* need to get prediction, till then... */
	prediction=getprediction(prediction);
	toprint = unhashboard(position,toprint);
	displayasciiboard(toprint,prediction); 
	free(toprint);
	free(prediction);
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
	int curoptions=getOption();
	int numberOfOptions;
	/* set all the options at max */
	boardsize=4;
	rulesvariant=1;
	handicapping=1;
	towin=2;
	numberOfOptions=getOption();
	/* return the options to their previous values */
	setOption(curoptions);		
    return numberOfOptions;
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
	/**
	 ** Options:
	 ** 1/0 Inverse/Normal Rules
 	 ** 1/0 Handicapped/Not
         ** 0-2 Counting Territory, captures, or both
	 ** 0-4 Boardsize
         **/
	return 30*rulesvariant+15*handicapping+5*towin+boardsize;	
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
   	rulesvariant=option/30;
	option-=(rulesvariant*30);
	handicapping=option/15;
	option-=(handicapping*15); 
	towin=option/5;
	option-=(towin*5);
	boardsize=option;
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

/************************************************************************
** 
** int hash(char *board,int player,int turn) 
** takes in a board, player number, and turn number (starting from 0)
** returns a hash of that
**
** uses the generic_hash() to hash the board and position
** shifts that left 5 bits and encodes the turn number
**
**************************************************************************/
POSITION hash(char *board, int player, int turn) {
	POSITION ret;
	
	/* do the generic hash */
	ret=generic_hash(board,player);

	ret=ret<<NUM_HASH_WRAPPER_BITS;
	ret|=turn;
	
	return ret;	
}

/*************************************************************************
**
** char *unhashboard(int hashed)
** returns a board from a hashed board
** 
***************************************************************************/

char *unhashboard(POSITION hashed, char *board){
	hashed=hashed>>NUM_HASH_WRAPPER_BITS; /* get rid of the turn encoding */
	/* better have a non null board */
	return generic_unhash(hashed, board);
}

/*************************************************************************
**
** char *unhashboard(int hashed)
** returns a board from a hashed board
** 
***************************************************************************/

int hash_init(int boardsize, int pieces_array[], int (*vcfg_function_ptr)(int* cfg)) {
        return (generic_hash_init(boardsize, piecesarray, NULL) << NUM_HASH_WRAPPER_BITS);    /* initialize the hash */
}

/***************************************************************************
** 
** int getplayer(int hashed)
** returns a player number (whose turn it is currently)
** takes in a hashed board
**
*****************************************************************************/

int getplayer(POSITION hashed) {
	hashed = hashed>>NUM_HASH_WRAPPER_BITS;
	return whoseMove(hashed);
}

/****************************************************************************
**
** int getturnnumber(int hashed)
** returns the turn number of the current player
**
*****************************************************************************/

int getturnnumber(POSITION hashed) {
	return hashed&31;
}

/*********
** 
** int countboard(char *board, char tocount)
** returns the number of occurances of a character in a board
**
***********/

int countboard(char *board,char tocount) {
	int counter=0,i=0;
	for(;i<maxsize;i++) 
		if(board[i]==tocount)
			counter++;
	return counter;
}
