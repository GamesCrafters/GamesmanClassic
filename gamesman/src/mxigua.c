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

/* Game options */
int boardsize; /* 0-4 Board size is 5 + 4 * boardsize */
int rulesvariant; /* 0/1 normal rules / inverted rules */
int handicapping; /* 0/1 Handicapped or not */
int towin; /* 0-2  0 - counting territory, 1 - captured pieces, 2 - both */

/* max board size */
int maxsize;
/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/

/* External */
extern GENERIC_PTR	SafeMalloc ();
extern void		SafeFree ();

/* Internal */
void displayasciiboard(char *, char *);
int NumberOfOptions();
int getOption();
void setOption(int);
char *emptyboard();

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
	int piecesarray[]={'X',0,maxsize-1,'*',0,maxsize-1,'O',0,maxsize,-1};
	char *eboard;
	int i;

	maxsize=5+4*boardsize;	
	gNumberOfPositions=generic_hash_init(maxsize, piecesarray, NULL);    /* initialize the hash */
	eboard=emptyboard();
	gInitialPosition=generic_hash(eboard,1);
	free(eboard);		
}

char *emptyboard() {
	char *ret;
	int i;
	ret=(char *)malloc(sizeof(char)*maxsize);
	for(i=0;i<maxsize;i++) ret[i]='O';
	return ret;
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
	char *board;
	int i;
   
	board=emptyboard();	 
    /* Use CreateMovelistNode(move, next) to 'cons' together a linked list */
	/* wow, talk about making something foolproof for having students coding games */
	board=generic_unhash(position,board);
	for(i=0;i<maxsize;i++) {
		if(board[i]=='O') {
			moves = CreateMovelistNode((MOVE)i,moves);
		}
	}   	

	free(board); 
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
	char *board,piece;
	int maxsize=5+4*boardsize,wmove;	
	POSITION hashed;
	
	board=(char *)malloc(sizeof(char)*maxsize);
	board=emptyboard();
	board=generic_unhash(position,board);

	wmove=whoseMove(position);
	if(wmove==1) {
		piece='X';
	} else if (wmove==2) {
		piece='*';
	} else {
		fprintf(stderr,"Bad else: Do Move (Piece selection)\n");
		exit(1);
	}
	
	board[move]=piece;
	
    	hashed = generic_hash(board,(piece=='X')?2:1); 
	free(board);
	return hashed;
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
        printf("           \\J-K-L/  (%s)  \\%c-%c-%c/\n",pos[18],pos[19],pos[20]);

}

void getprediction(char *pred) {
	int i=0;
	/* code to initialize it to blank values until a later time */
	pred = (char *)malloc(sizeof(char)*17);
	for(;i<16;i++) pred[i]=(char)32; /* fill with spaces */
	pred[i]='\0';
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
	/* need to get prediction, till then... */
	getprediction(prediction);
	toprint = generic_unhash(position,toprint);
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


