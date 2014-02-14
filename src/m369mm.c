/************************************************************************
**
** NAME:        m6mm.c
**
** DESCRIPTION: SIX MEN'S MORRIS
**
** AUTHOR:      Patricia Fong & Kevin Liu & Erwin A. Vedar, Wei Tu, Elmer Lee
**
** DATE:        too long to remmeber
**
** UPDATE HIST: RECORD CHANGES YOU HAVE MADE SO THAT TEAMMATES KNOW
**
** LAST CHANGE: $Id: m369mm.c,v 1.4 2008-09-29 07:33:40 noafroboy Exp $
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

STRING kGameName            = "369 Men's Morris";   /* The name of your game */
STRING kAuthorName          = "Patricia Fong, Kevin Liu, Erwin A. Vedar, Elmer Lee";   /* Your name(s) */
STRING kDBName              = "369mm";   /* The name to store the database under */

BOOLEAN kPartizan            = TRUE;   /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN kGameSpecificMenu    = TRUE;   /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN kTieIsPossible       = FALSE;   /* TRUE if a tie is possible. FALSE if it is impossible.*/
BOOLEAN kLoopy               = TRUE;   /* TRUE if the game tree will have cycles (a rearranger style game). FALSE if it does not.*/

BOOLEAN kDebugMenu           = TRUE;   /* TRUE only when debugging. FALSE when on release. */
BOOLEAN kDebugDetermineValue = FALSE;   /* TRUE only when debugging. FALSE when on release. */

POSITION gNumberOfPositions   =  0; /* The number of total possible positions | If you are using our hash, this is given by the hash_init() function*/
POSITION gInitialPosition     =  0; /* The initial hashed position for your starting board */
POSITION kBadPosition         = -1; /* A position that will never be used */

void*    gGameSpecificTclInit = NULL;

/**
 * Help strings that are pretty self-explanatory
 * Strings than span more than one line should have backslashes (\) at the end of the line.
 * These help strings should be updated and dynamically changed using
 * InitializeHelpStrings()
 **/

STRING kHelpGraphicInterface =
        "369 Men's Morris does not currently support a Graphical User Interface\n(other than beloved ASCII).";

STRING kHelpTextInterface =
        "The LEGEND shows numbers corresponding to positions on the board.  On your\nturn, use the LEGEND to enter the position your piece currently is, the position\nyour piece is moving to, and (if your move creates a mill) the position of the\npiece you wish to remove from play.  Seperate each number entered with a space\nand hit return to commit your move.  If you ever make a mistake when choosing\nyour move, you can type \"u\" and hit return to revert back to your most recent\nposition.";

STRING kHelpOnYourTurn =
        "Help strings not initialized!";

STRING kHelpStandardObjective =
        "Help strings not initialized!";

STRING kHelpReverseObjective =
        "Help strings not initialized!";

STRING kHelpTieOccursWhen =
        "Help strings not initialized!";

STRING kHelpExample =
        "Help strings not initialized!";

/* Variants */
BOOLEAN gFlying = FALSE;
int gameType = 6; //3,6,9 men's morris
int millType = 0; //0: can remove piece not from mill unless if only mills left. 1: can remove any piece. 2: can not remove pieces from any mill ever
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
int BOARDSIZE; //6mm 16   9mm 24
int minx = 2;
int maxx;  //6mm 6   9mm 9
int mino = 2;
int maxo;  //6mm 6   9mm 9
int minb;
int maxb;



#define BLANK '·'
#define X 'X'
#define O 'O'

#define PLAYER_ONE 1
#define PLAYER_TWO 2



//TEMPORARY GLOBALS, UNTIL TIERS IMPLEMENTED
//KEVIN

int NUMX=0;
int NUMO=0;
int totalPieces = 18; //Remove when tiering


/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/

/* External */
#ifndef MEMWATCH
extern GENERIC_PTR      SafeMalloc ();
extern void             SafeFree ();
#endif

/*function prototypes*/
void InitializeGame ();
void SetupTierStuff();
TIERLIST* gTierChildren(TIER tier);
TIERPOSITION gNumberOfTierPositions (TIER tier);
STRING TierToString(TIER tier);
void InitializeHelpStrings ();
MOVELIST *GenerateMoves (POSITION position);
POSITION DoMove (POSITION position, MOVE move);
VALUE Primitive (POSITION position);
void PrintPosition (POSITION position, STRING playersName, BOOLEAN usersTurn);
void PrintComputersMove (MOVE computersMove, STRING computersName);
void PrintMove (MOVE move);
STRING MoveToString(MOVE move);
USERINPUT GetAndPrintPlayersMove (POSITION position, MOVE *move, STRING playersName);
BOOLEAN ValidTextInput (STRING input);
MOVE ConvertTextInputToMove (STRING input);
void GameSpecificMenu ();
void SetTclCGameSpecificOptions (int options[]);
POSITION GetInitialPosition ();
int NumberOfOptions ();
int getOption ();
void setOption (int option);
void DebugMenu ();
char* unhash(POSITION pos, char* turn, int* piecesLeft, int* numx, int* numo);
POSITION hash(char* board, char turn, int piecesLeft, int numx, int numo);
POSITION updatepieces(char* board,char turn,int piecesLeft,int numx,int numo,MOVE move, POSITION position);
BOOLEAN can_be_taken(POSITION position, int slot);
BOOLEAN all_mills(char *board, int slot);
int find_pieces(char *board, char piece, int *pieces);
BOOLEAN closes_mill(POSITION position, int raw_move);
BOOLEAN check_mill(char *board, int slot, char turn);
BOOLEAN three_in_a_row(char *board, int slot1, int slot2, char turn);
int find_adjacent(int slot, int *slots);
POSITION EvalMove(char* board,char turn,int piecesLeft,int numx,int numo,MOVE move, POSITION position);
char returnTurn(POSITION pos);
char* customUnhash(POSITION pos);
void changetosix();
void changetonine();
void changetothree();
POSITION GetCanonicalPosition(POSITION);
POSITION smallHash(char*, char);
char* smallUnhash(POSITION, char*);


//SYMMETRIES
BOOLEAN kSupportsSymmetries = FALSE; /* Default false, true for 9mm*/

#define NUMSYMMETRIES 9   /*  4 rotations, 4 flipped rotations, outer-inner flip */
#define BOARDSIZE9mm 24
int gSymmetryMatrix[NUMSYMMETRIES][BOARDSIZE9mm];



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
	// SYMMETRY
	gCanonicalPosition = GetCanonicalPosition;
	if (gameType==3) changetothree();
	if (gameType==6) changetosix();
	if (gameType==9) changetonine();
	char* board = (char*) SafeMalloc(BOARDSIZE * sizeof(char));

	int pminmax[] = {X, 0, maxx, O, 0, maxo,BLANK, BOARDSIZE-maxx-maxo, BOARDSIZE, -1};

	gCustomUnhash = &customUnhash;
	gReturnTurn = &returnTurn;
	SetupTierStuff();

	for(i = 0; i < BOARDSIZE; i++)
	{
		board[i] = BLANK;
	}


	gNumberOfPositions = generic_hash_init(BOARDSIZE, pminmax, NULL, 0);
	gInitialPosition = hash(board, X, maxx+maxo, 0, 0);

	InitializeHelpStrings();
	//printf("initialize game done\n");
}


/************************************************************************
**
** NAME:        InitializeHelpStrings
**
** DESCRIPTION: Sets up the help strings based on chosen game options.
**
** NOTES:       Should be called whenever the game options are changed.
**              (e.g., InitializeGame() and GameSpecificMenu())
**
************************************************************************/
void InitializeHelpStrings ()
{

	kHelpGraphicInterface =
	        "";

	kHelpTextInterface =
	        "";

	kHelpOnYourTurn =
	        "";

	kHelpStandardObjective =
	        "";

	kHelpReverseObjective =
	        "";

	kHelpTieOccursWhen =
	        "A tie occurs when ...";

	kHelpExample =
	        "";

	gMoveToStringFunPtr = &MoveToString;

}


/************************************************************************
**
** NAME:        GenerateMoves
**
** DESCRIPTION: Creates a linked list of every move that can be reached
**              from this position. Returns a pointer to the head of the
**              linked list.
**
** INPUTS:      POSITION position : Current position to generate moves
**
** OUTPUTS:     (MOVELIST *)      : A pointer to the first item of
**                                  the linked list of generated moves
**
** CALLS:       MOVELIST *CreateMovelistNode();
**
************************************************************************/

MOVELIST *GenerateMoves (POSITION position)
{
	//printf("%llu\n", position);
	MOVELIST *moves = NULL;
	//MOVELIST *temp = NULL;	//DEBUG
	char turn;
	int piecesLeft;
	int numx, numo;
	int numadjacent;
	int posadjacent[4];
	int from, to;
	int i, k;
	int z = 0;
	char* board = unhash(position, &turn, &piecesLeft, &numx, &numo);
	//printf("\nPRINT POSITION INSIDE GENERATE MOVES\tturn=%c\n", turn); //DEBUG
	//PrintPosition(position, "kevin", turn);//DEBUG

	//printf("GENERATEMOVES turn: %c\n", turn);

	// get NUMX, NUMO from unhash
	//printf("ENTERED GENERATE MOVES\n");
	//stage 2 & 3
	if(piecesLeft == 0) {
		for (from = 0; from < BOARDSIZE; from++) {
			if (board[from] == turn) { //make sure you're trying to move your own stone
				if (!gFlying || ((turn == X) && (numx > 3)) || ((turn == O) && (numo > 3))) { // STAGE 2 : sliding
					//printf("inside stage 2 of GenerateMoves\n");
					numadjacent = find_adjacent(from, posadjacent); //adjacent positions
					for(i = 0; i < numadjacent; i++) { //check if to is an adjacent position
						to = posadjacent[i];
						if(board[to] == BLANK) { //position is empty
							if(closes_mill(position, from*BOARDSIZE*BOARDSIZE+to*BOARDSIZE+from)) { // the position makes a 3 in a row
								for(k=0; k<BOARDSIZE; k++) {
									if (turn != board[k]) { //the piece is opponent's piece
										if(can_be_taken(position, k)) {
											z=1;
											moves = CreateMovelistNode(from*BOARDSIZE*BOARDSIZE + to*BOARDSIZE + k, moves);
										}
									}
								}
								if(millType == 2 && z==0) {
									moves = CreateMovelistNode(from*BOARDSIZE*BOARDSIZE + from*BOARDSIZE+from, moves);
								}
							}
							else
								moves = CreateMovelistNode(from*BOARDSIZE*BOARDSIZE + to*BOARDSIZE + from, moves);
						}
					}
				} else { // STAGE 3 : flying
					//printf("inside stage 3 of GenerateMoves\n");
					for (to = 0; to < BOARDSIZE; to++) {
						if(board[to] == BLANK) { //position is empty
							if(closes_mill(position, from*BOARDSIZE*BOARDSIZE+to*BOARDSIZE+from)) { // made 3 in a row
								for(k=0; k<BOARDSIZE; k++) {
									if(can_be_taken(position, k)) {
										z=1;
										moves = CreateMovelistNode(from*BOARDSIZE*BOARDSIZE + to*BOARDSIZE + k, moves);
									}
								}
								if(millType == 2 && z==0) {
									moves = CreateMovelistNode(from*BOARDSIZE*BOARDSIZE + from*BOARDSIZE+from, moves);
								}
							}
							else
								moves = CreateMovelistNode(from*BOARDSIZE*BOARDSIZE + to*BOARDSIZE + from, moves);
						}
					}
				}
			}
		}
	} else { // STAGE 1, placing
		for (from = 0; from < BOARDSIZE; from++) {
			if(board[from] == BLANK) { //position is empty
				if(closes_mill(position, from*BOARDSIZE*BOARDSIZE+from*BOARDSIZE+from)) {
					//printf("this position closes a mill\n");
					for(to=0; to < BOARDSIZE; to++) {
						if(can_be_taken(position, to)  && (board[to] != turn)) {
							z=1;
							moves = CreateMovelistNode(from*BOARDSIZE*BOARDSIZE + to*BOARDSIZE+from, moves);
						}
					}
					if(millType == 2 && z==0) {
						moves = CreateMovelistNode(from*BOARDSIZE*BOARDSIZE + from*BOARDSIZE+from, moves);
					}
				}
				else {
					moves = CreateMovelistNode(from*BOARDSIZE*BOARDSIZE + from*BOARDSIZE+from, moves);
				}
			}
		}
	}

	//printf("done with GenerateMoves\n");
	SafeFree(board);
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
	char* board;
	char turn;
	int piecesLeft;
	int numx, numo;
	POSITION temp;

	//printf("inside DoMove\n");
	board = unhash(position, &turn, &piecesLeft, &numx, &numo);

	//printf("numx: %d, numo:%d, piecesLeft: %d\n", numx, numo, piecesLeft);

	int to = (move%(BOARDSIZE*BOARDSIZE)) / BOARDSIZE;
	int from = move / (BOARDSIZE * BOARDSIZE);
	int remove = move % BOARDSIZE;

	if (piecesLeft == 0) // STAGE 2 or 3, SLIDING OR FLYING
	{
		//printf("inside DoMove. to=%d, from = %d, remove = %d\n", to, from, remove); //DEBUG
		board[to] = board[from];
		board[from] = BLANK;
		board[remove] = BLANK; //if remove wasn't specified in the string, it is by default equal to from
	}

	else // STAGE 1 : PLACING
	{
		//printf("DoMove stage 1\n");
		board[from] = turn;
		if (from != to)
			board[to] = BLANK;
	}

	//printf("numx: %d numo: %d\n", numx, numo);

	//printf("DOMOVE TURN: %d\n", turn);


	if (turn == X) {
		temp = updatepieces(board,O,piecesLeft, numx, numo, move, position);
		SafeFree(board);

	}
	else{
		temp = updatepieces(board, X, piecesLeft, numx, numo,move,position);
		SafeFree(board);

	}

	TIER tier; TIERPOSITION tierposition;
	gUnhashToTierPosition(position, &tierposition, &tier);
	gCurrentTier = tier;
	//printf("current tier = %d\n", (int)gCurrentTier);

	return temp;
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
	char *board;
	char turn;
	int piecesLeft;
	int numx, numo;

	board = unhash(position, &turn, &piecesLeft, &numx, &numo);
	SafeFree(board);

	if(piecesLeft == 0) { // Check if we are in stage 2 (stage 3 included in 2 with special rules)
		/*
		   if (((numx < 3) && (turn == O)) || ((numo < 3) && (turn == X))) {
		   fprintf(stderr, "ERROR: 6mm Primitive had a position with 2 pieces, but it was their turn!\n");
		   ExitStageRight();
		   }
		 */
		if ((numx < 3) || (numo < 3))
			return gStandardGame ? lose : win;

		MOVELIST* moves = GenerateMoves(position);
		if (NULL == moves)
			return gStandardGame ? lose : win;
		FreeMoveList(moves);
	}
	//printf("inside Primitive\n");
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
	char turn;
	int piecesLeft;
	int numx, numo;

	//printf("PRINTPOSITION PLAYER: %d\n", generic_hash_turn(position));
	char* board = unhash(position, &turn, &piecesLeft, &numx, &numo);
	//printf("PRINTPOSITION PLAYER %d\n", turn);

	if (gameType==3) {
		printf("\n");
		printf("          0 -- 1 -- 2    %c -- %c -- %c   |    %s's turn (%c)\n", board[0], board[1], board[2], playersName, turn);
		printf("          |    |    |    |    |    |   |    Phase: ");
		if (piecesLeft != 0)
			printf("1 : PLACING\n");
		else {
			if  (!gFlying || ((turn == X) && (numx > 3)) || ((turn == O) && (numo > 3)))
				printf("2 : SLIDING\n");
			else
				printf("3 : FLYING\n");
		}
		printf("          3 -- 4 -- 5    %c -- %c -- %c   |    Phase: ", board[3], board[4], board[5]);

		if (piecesLeft != 0)
			printf("X has %d left to place\n",piecesLeft/2);
		else
			printf("X has %d on the board\n", numx);

		printf("          |    |    |    |    |    |   |    Phase: ");

		if (piecesLeft != 0)
			printf("O has %d left to place\n",piecesLeft/2 + piecesLeft%2);
		else
			printf("O has %d on the board\n", numo);

		printf("          6 -- 7 -- 8    %c -- %c -- %c   |    \n\n", board[6], board[7], board[8]);


	}

	else if (gameType==6) {

		printf("\n");
		printf("          0 ----- 1 ----- 2    %c ----- %c ----- %c    %s's turn (%c)\n", board[0], board[1], board[2], playersName, turn);
		printf("          |       |       |    |       |       |    \n");
		printf("          |   3 - 4 - 5   |    |   %c - %c - %c   |    Phase: ", board[3], board[4], board[5]);
		if (piecesLeft != 0)
			printf("1 : PLACING\n");
		else {
			if  (!gFlying || ((turn == X) && (numx > 3)) || ((turn == O) && (numo > 3)))
				printf("2 : SLIDING\n");
			else
				printf("3 : FLYING\n");
		}
		printf("          |   |       |   |    |   |       |   |    ");
		if (piecesLeft != 0)
			printf("X has %d left to place\n",piecesLeft/2);
		else
			printf("X has %d on the board\n", numx);
		printf("LEGEND:   6 - 7       8 - 9    %c - %c       %c - %c    ", board[6], board[7], board[8], board[9]);
		if (piecesLeft != 0)
			printf("O has %d left to place\n",piecesLeft/2 + piecesLeft%2);
		else
			printf("O has %d on the board\n", numo);
		printf("          |   |       |   |    |   |       |   |    \n");
		printf("          |  10 - 11- 12  |    |   %c - %c - %c   |  \n", board[10], board[11], board[12] );
		printf("          |       |       |    |       |       |     \n");
		printf("          13 ---- 14 ---- 15   %c ----- %c ----- %c    %s\n\n", board[13], board[14], board[15],
		       GetPrediction(position,playersName,usersTurn));

	} else {
		printf("\n");
		printf("        0 --------- 1 --------- 2       %c --------- %c --------- %c    %s's turn (%c)\n", board[0], board[1], board[2], playersName, turn );
		printf("        |           |           |       |           |           |\n");
		printf("        |   3 ----- 4 ----- 5   |       |   %c ----- %c ----- %c   |    Phase: ", board[3], board[4], board[5]);
		if (piecesLeft != 0)
			printf("1 : PLACING\n");
		else {
			if  (!gFlying || ((turn == X) && (numx > 3)) || ((turn == O) && (numo > 3)))
				printf("2 : SLIDING\n");
			else
				printf("3 : FLYING\n");
		}
		printf("        |   |       |       |   |       |   |       |       |   |    ");
		if (piecesLeft != 0)
			printf("X has %d left to place\n",piecesLeft/2);
		else
			printf("X has %d on the board\n", numx);
		printf("        |   |   6 - 7 - 8   |   |       |   |   %c - %c - %c   |   |    ", board[6], board[7], board[8] );
		if (piecesLeft != 0)
			printf("O has %d left to place\n",piecesLeft/2 + piecesLeft%2);
		else
			printf("O has %d on the board\n", numo);
		printf("        |   |   |       |   |   |       |   |   |       |   |   |\n");
		printf("LEGEND: 9 - 10- 11      12- 13- 14      %c - %c - %c       %c - %c - %c    Turn: %c\n", board[9], board[10], board[11], board[12], board[13], board[14], turn);
		printf("        |   |   |       |   |   |       |   |   |       |   |   |\n");
		printf("        |   |   15- 16- 17  |   |       |   |   %c - %c - %c   |   |\n", board[15], board[16], board[17] );
		printf("        |   |       |       |   |       |   |       |       |   |\n");
		printf("        |   18 ---- 19 ---- 20  |       |   %c ----- %c ----- %c   |\n", board[18], board[19], board[20] );
		printf("        |           |           |       |           |           |\n");
		printf("        21 -------- 22 -------- 23      %c --------- %c --------- %c\n\n", board[21], board[22], board[23] );

	}
	SafeFree(board);
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
	STRING str = MoveToString( computersMove );
	printf("%8s's move                                     : %s\n",computersName,str);
	SafeFree( str );
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
	STRING str = MoveToString( move );
	printf( "%s", str );
	SafeFree( str );
}


/************************************************************************
**
** NAME:        MoveToString
**
** DESCRIPTION: Returns the move as a STRING
**
** INPUTS:      MOVE *move         : The move to put into a string.
**
************************************************************************/

STRING MoveToString (MOVE move)
{
	//DONE
	int to = (move%(BOARDSIZE*BOARDSIZE)) / BOARDSIZE;
	int from = move / (BOARDSIZE * BOARDSIZE);
	int remove = move % BOARDSIZE;
	int tier, piecesLeft;

	STRING movestring;
	//printf("MOVE TO STRING\n");
	tier = generic_hash_cur_context();
	piecesLeft = tier/100;
	if (piecesLeft == 0)
	{
		if(from != remove)
		{
			movestring = (STRING) SafeMalloc(12);
			sprintf( movestring, "[%d %d %d]",from, to, remove );
		}
		else if(from == to && to == remove) {
			movestring = (STRING) SafeMalloc(8);
			sprintf( movestring, "[%d]", from); //do not remove the '[' or ']' characters. it wil lbreak the gui
		}
		else {
			movestring = (STRING) SafeMalloc(8);
			sprintf( movestring, "[%d %d]", from, to);
		}
	}
	else
	{
		//printf("MOVE TO STRING ELSE\n");
		//printf("from = %d, to = %d, remove = %d\n", from, to, remove);

		if (from == to) //if 1st == 2nd position in move formula
		{
			movestring = (STRING) SafeMalloc(8);
			sprintf(movestring, "[%d]", from);
		}
		else
		{
			movestring = (STRING) SafeMalloc(8);
			sprintf(movestring, "[%d %d]", from, to);
		}
	}


	return movestring;
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
	char turn;
	int piecesLeft, numx, numo;
	char *board = unhash(position, &turn, &piecesLeft, &numx, &numo);
	SafeFree(board);

	do {
		printf("%8s's move: (u)ndo/", playersName);
		if (piecesLeft != 0) // STAGE 1 : PLACING
			printf("0-15/[0-15 0-15]            ");
		else {
			printf("[0-15 0-15]/[0-15 0-15 0-15]");
		}
		printf(": ");

		input = HandleDefaultTextInput(position, move, playersName);

		if (input != Continue)
			return input;
	} while (TRUE);

	/* NOTREACHED */
	return (Continue);
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
	//DONE
	if(input[0]>57 || input[0]<48)
		return FALSE;
	else
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
	//DONE
	int from, to, remove;
	STRING afterSpace;
	STRING after2Space;
	BOOLEAN hasSpace, has2Space;

	from = atoi(input);
	remove = from; //that way if no input for remove, it's equal to from. useful in function DoMove
	to = from; // for stage 1, if there is nothing to remove

	hasSpace = index(input, ' ') != NULL;

	if (hasSpace) {
		afterSpace = index(input, ' ');
		to = atoi(afterSpace);

		has2Space = index(++afterSpace, ' ') != NULL;

		if (has2Space) {
			after2Space = index(afterSpace, ' ');
			remove = atoi(after2Space);
		}

	}
	//printf("converttextinputtomove.... move = %d\n", (from * BOARDSIZE * BOARDSIZE) + (to * BOARDSIZE) + remove);
	//printf("from: %d, to: %d, remove: %d\n", from, to, remove);
	return ((from * BOARDSIZE * BOARDSIZE) + (to * BOARDSIZE) + remove); //HASHES THE MOVE
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
	char GetMyChar();
	POSITION GetInitialPosition();

	do {
		printf("\n\t----- Game-specific options for %s -----\n\n", kGameName);


		//printf("\tCurrent Initial Position:\n");
		//PrintPosition(gInitialPosition, gPlayerName[kPlayerOneTurn], kHumansTurn);

		printf("\n");
		//printf("\ti)\tChoose the (I)nitial position\n");
		printf("\tf)\tToggle (F)lying from %s to %s\n",
		       gFlying ? "ON" : "OFF",
		       !gFlying ? "ON" : "OFF");

		if (millType == 0) {
			printf("\tm)\tWhen mill is formed, can remove any opponent's piece.\n");
		}
		else if (millType == 1) {
			printf("\tm)\tWhen mill is formed, can remove opponent's piece if it is not in a mill.\n");
		}
		else if (millType == 2) {
			printf("\tm)\tWhen mill is formed, can remove opponent's piece if it is not in a mill, unless if all the remaining pieces are already in a mill.\n");
		}

		if (gameType == 3) {
			printf("\tn)\tSwitch to Six Men's Morris.\n");
		}
		else if (gameType == 6) {
			printf("\tn)\tSwitch to Nine Men's Morris.\n");
		}
		else if (gameType == 9) {
			printf("\tn)\tSwitch to Three Men's Morris.\n");
		}

		printf("\n\n\tb)\t(B)ack = Return to previous activity.\n");
		printf("\n\nSelect an option: ");

		switch(GetMyChar()) {
		case 'M': case 'm':
			if (millType == 0)
				millType = 1;
			else if (millType == 1)
				millType = 2;
			else if (millType == 2)
				millType = 0;
			return;
		case 'N': case 'n':
			if (gameType == 3)
				changetosix();
			else if (gameType == 6)
				changetonine();
			else if (gameType == 9)
				changetothree();
			return;
		case 'Q': case 'q':
			ExitStageRight();
		case 'H': case 'h':
			HelpMenus();
			break;
		//    case 'I': case 'i':
		//gInitialPosition = GetInitialPosition();
		//break;
		case 'F': case 'f':
			gFlying = !gFlying;
			break;
		case 'B': case 'b':
			return;
		default:
			printf("\nSorry, I don't know that option. Try another.\n");
			HitAnyKeyToContinue();
			break;
		}
	} while(TRUE);

	//FINISH COPY OLD CODE
	InitializeHelpStrings();
}


/************************************************************************
**
** NAME:        SetTclCGameSpecificOptions
**
** DESCRIPTION: Set the C game-specific options (called from Tcl)
**              Ignore if you don't care about Tcl for now.
**
************************************************************************/

void SetTclCGameSpecificOptions (int options[]){
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
	return 36; //misere/standard   flying/no-flying   3mm/6mm/9mm millType = 1/2/3
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
	//printf("option = %d\n", 1 + (gFlying<<1) + gStandardGame+ ((gameType/3-1)<<2) + (millType << 4));
	//printf("millType = %d\n", millType);
	return 1 + (gFlying<<1) + gStandardGame + ((gameType/3-1)<<2) + (millType<<4);
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
	// In terms of bits, option is one more than 0bFS (F=flying,S=Standard)
	int temp;
	option -= 1;
	gStandardGame = (option % 2);
	gFlying       = (option >> 1) % 2;
	temp = gameType;
	gameType = (((option>>2)+1)*3);
	millType = (option >> 4);
	if((temp != gameType) && (gameType==3)) {
		changetothree();
	}
	else if((temp != gameType) && (gameType==6)) {
		changetosix();
	}
	else if ((temp != gameType) && (gameType == 9)) {
		changetonine();
	}
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

void printBoard(char* board) {
	//int i;
	//printf("Board is= ");
	//for (i = 0; i < BOARDSIZE; i++)
	//printf("%c", board[i]);
	//printf("\n");
}


//this was made just for "tkAppInit.c" we are returning the turn so that the gui knows whose turn it is.
char returnTurn(POSITION pos)
{
	if(gHashWindowInitialized) {
		TIER tier; TIERPOSITION tierposition;
		gUnhashToTierPosition(pos, &tierposition, &tier);
		generic_hash_context_switch(tier);
		return (generic_hash_turn(tierposition)==PLAYER_ONE ? X : O);
	}
	return '0';
}

//this was made just for "tkAppInit.c" we are returning the board so that the gui knows whose turn it is.
char* customUnhash(POSITION pos)
{
	//piecesLeft = total pieces left during stage 1 (x + o)
	char* board = (char*)SafeMalloc(BOARDSIZE * sizeof(char));
	if(gHashWindowInitialized) {
		TIER tier; TIERPOSITION tierposition;
		gUnhashToTierPosition(pos, &tierposition, &tier);
		generic_hash_context_switch(tier);
		board = (char*)generic_hash_unhash(tierposition, board);
	}
	return board;
}

//this is the main unhash function that the C code uses.
char* unhash(POSITION pos, char* turn, int* piecesLeft, int* numx, int* numo)
{
	//piecesLeft = total pieces left during stage 1 (x + o)
	char* board = (char*)SafeMalloc(BOARDSIZE * sizeof(char));
	if(gHashWindowInitialized) {
		//printf("unhashing with tiers\n");
		TIER tier; TIERPOSITION tierposition;
		gUnhashToTierPosition(pos, &tierposition, &tier);
		//printf("unhashing %d\n", tier);
		generic_hash_context_switch(tier);
		(*turn) = (generic_hash_turn(tierposition)==PLAYER_ONE ? X : O);
		board = (char*)generic_hash_unhash(tierposition, board);
		*piecesLeft = tier/100;
		*numx = (tier/10)%10;
		*numo = tier%10;
	}
	else{
		printf("unhashing without tiers... cannot solve game\n");
		generic_hash_unhash(pos, board);
		*piecesLeft = totalPieces; //remove when tiering
		if (generic_hash_turn(pos) == PLAYER_ONE)
			(*turn) = X;
		else (*turn) = O;
		*numx = NUMX; //remove when tiering
		*numo = NUMO; // remove when tiering
	}
	return board;
}

POSITION hash(char* board, char turn, int piecesLeft, int numx, int numo)
{
	POSITION pos;
	if(gHashWindowInitialized) {
		// printf("piecesLeft =%d \t numx: %d \t numo: %d\n", piecesLeft, numx,numo);
		TIER tier = piecesLeft*100+numx*10+numo;

		//printf(" line 1093 tier = %d\n", tier);
		//printf("%d\n", tier);
		generic_hash_context_switch(tier);
		printBoard(board);
		TIERPOSITION tierposition = generic_hash_hash(board, (turn == X ? PLAYER_ONE : PLAYER_TWO));
		//printf("line 1096 tierposition = %d\n", tierposition);
		pos = gHashToWindowPosition(tierposition, tier);
	}
	else{
		pos = generic_hash_hash(board, (turn == X ? PLAYER_ONE : PLAYER_TWO));
		if (board != NULL)
			SafeFree(board);

		totalPieces = piecesLeft; //remove when tiering
		NUMX = numx; //remove when tiering
		NUMO = numo; //remove when tiering
	}
	return pos;
}


void SetupTierStuff(){
	generic_hash_destroy();
	char* board = (char*) SafeMalloc(BOARDSIZE * sizeof(char));
	kSupportsTierGamesman = TRUE;
	gTierChildrenFunPtr = &gTierChildren;
	gNumberOfTierPositionsFunPtr = &gNumberOfTierPositions;
	gTierToStringFunPtr = &TierToString;
	int tier, i, piecesLeft, numx, numo;
	generic_hash_custom_context_mode(TRUE);
	//	int pminmax[] = {X, 0, maxx, O, 0, maxo,BLANK, BOARDSIZE-maxx-maxo, BOARDSIZE, -1};
	int pieces_array[] = {X, 0, 0, O, 0, 0, BLANK, 0, 0, -1  };
	kExclusivelyTierGamesman = TRUE;

	for(piecesLeft=0; piecesLeft<maxx+maxo+1; piecesLeft++) {
		for(numx=0; numx<maxx+1; numx++) {
			for(numo=0; numo<maxo+1; numo++) {
				tier=piecesLeft*100+numx*10+numo;
				pieces_array[1]=pieces_array[2]=numx;
				pieces_array[4]=pieces_array[5]=numo;
				pieces_array[7]=pieces_array[8]=BOARDSIZE-numx-numo;
				if (piecesLeft > 0) { //stage1
					//pieces_array[] = {X, 0, piecesLeft/2+1, O, piecesLeft/2+1, 0, BLANK, 0, 0, -1  } ;
					generic_hash_init(BOARDSIZE, pieces_array, NULL, (piecesLeft%2)+1);
				}
				else{
					//pieces_array[] = {X, minx, maxx, O, mino, maxo, BLANK, 0, 0, -1  } ;
					generic_hash_init(BOARDSIZE, pieces_array, NULL, 0);
				}
				generic_hash_set_context(tier);
				//printf("tier %d\t", tier);
				//int j;
				//for (j = 0; j < 10; j++)
				//printf("%d ", pieces_array[j]);
				//printf("\n");
			}
		}
	}

	tier = (maxx+maxo)*100;
	gInitialTier = tier;    //initial pieces on board
	gCurrentTier = tier;
	generic_hash_context_switch(tier);

	for(i = 0; i < BOARDSIZE; i++)
	{
		board[i] = BLANK;
	}
	gInitialTierPosition = generic_hash_hash(board, PLAYER_ONE);
	//SafeFree(board);
	//printf("line 1142 gInitialTierPosition = %d\n",  gInitialTierPosition);
}

TIERLIST* gTierChildren(TIER tier) {
	TIERLIST* list = NULL;
	int numx, numo, piecesLeft;
	piecesLeft=(tier/100);
	numx=(tier/10)%10;
	numo=tier%10;
//printf("tier = %d\t", tier);
	if (piecesLeft !=0) {
		if(piecesLeft%2==0) { //meaning it is X's turn
			//if (((piecesLeft == 2) && (numo > 1) && (numx+1 > 1) ) || (piecesLeft > 1))
			list = CreateTierlistNode(tier-100+10, list);         //adding piece
			//}
			//if (((piecesLeft > 1) &&(numx > 0)) || ((piecesLeft == 2) && (numo-1 > 1) && (numx+1 > 1)))  {
			if (numx > 1) {
				list = CreateTierlistNode(tier-100+9, list); //adding and removing
			}
		}
		else{
			//if (((piecesLeft == 1) && (numo+1 > 1) &&(numx > 1) ) || (piecesLeft > 1)) {
			list = CreateTierlistNode(tier-100+1, list);         //adding piece
			//}
			//if (((piecesLeft > 1) &&(numx > 0)) || ((piecesLeft == 1) && (numo+1 > 1) && (numx-1 > 1))) {
			if (numo > 1) {
				list = CreateTierlistNode(tier-100-9, list); //adding and removing
			}
		}
	}
	else if (piecesLeft==0) { //stage 2 or 3. We do not know the turn, so we assume both paths as tier children
		list=CreateTierlistNode(tier, list);
		if (numx > 2 && numo > 2) {
			list = CreateTierlistNode(tier-10, list); //remove X piece
			list = CreateTierlistNode(tier-1, list); //remove O piece
		}
	}
	//printf("tier = %d\t", tier);
	return list;
}

TIERPOSITION gNumberOfTierPositions (TIER tier) {
	//printf("%d\n",tier);
	generic_hash_context_switch(tier);
	return generic_hash_max_pos();
}

STRING TierToString(TIER tier) {
	int piecesLeft, numx, numo;
	piecesLeft = (tier/100);
	numx = (tier/10)%10;
	numo = tier%10;
	STRING str = (STRING) SafeMalloc (sizeof (char) * 50);
	if (piecesLeft > 0)
		sprintf(str, "STAGE 1: left-%d x-%d o-%d", piecesLeft, numx, numo);
	else sprintf(str, "STAGE 2: x-%d o-%d", numx, numo);
	return str;
}

//updates totalPieces
POSITION updatepieces(char* board,char turn,int piecesLeft,int numx,int numo,MOVE move, POSITION position)
{

	int to = (move%(BOARDSIZE*BOARDSIZE)) / BOARDSIZE;
	int from = move / (BOARDSIZE * BOARDSIZE);
	int remove = move % BOARDSIZE;


	if (piecesLeft != 0)
	{
		if (turn != X)
		{
			numx++;
			if (from != to) //1st pos != 2nd pos [removing a piece]
				numo--;
		}
		else
		{
			numo++;
			if (from != to) //1st pos != 2nd pos [removing a piece]
				numx--;
		}
		piecesLeft--;
	}
	else if (piecesLeft == 0) //stage 2 & 3
	{
		if (turn != X)
		{
			if (from != remove) //1st pos != 3nd pos [removing a piece]
				numo--;
		}
		else
		{
			if (from != remove) //1st pos != 3rd pos [removing a piece]
				numx--;
		}
	}
	//printf("turn=%d, numx=%d, numo=%d, totalPieces=%d, turn = %c\n", generic_hash_turn(position), numx, numo, piecesLeft, turn); //debug
	return hash(board, turn, piecesLeft, numx, numo);
}


// Given POSITION, slot
// Return true if piece at slot can be taken, false otherwise
BOOLEAN can_be_taken(POSITION position, int slot)
{
	char *board;
	char turn;
	int piecesLeft;
	int numx, numo;
	board = unhash(position, &turn, &piecesLeft, &numx, &numo);
	//blankox piece = board[slot];
	BOOLEAN allMills;
	BOOLEAN canbetaken;
	//printf("can_be_taken!!!!!!!!!!!!!!\n");
	/* According to the rules, a piece can be taken if it is not in a mill
	        or if the opponent only has mills */

	if (board[slot] == BLANK || board[slot] == turn) {
		SafeFree(board);
		return FALSE;
	}

	allMills = all_mills(board, slot);

	if(millType == 0)
		canbetaken = ((!check_mill(board, slot, turn==X ? O : X)) || allMills);
	if(millType == 1)
		canbetaken = TRUE;
	if(millType == 2)
		canbetaken = ((!check_mill(board, slot, turn==X ? O : X)));

	SafeFree(board);
	return canbetaken;
	//check mill to see if the position is in a mill
	//if it is in a mill, it will return false
	//if it is not a mill, it will be true
	//allMills returns true if the opponent's pieces are only mills
	//even if the piece is in a mill, it will return true if allMills is true
}


// Given POSITION, slot
// Return true if at this position, the player at slot only has mills or has only 3 pieces that are in a mill
BOOLEAN all_mills(char *board, int slot)
{
	char type = board[slot];
	int pieces[type == X ? maxx : maxo];
	int num, i;
	BOOLEAN allMills = TRUE;

	num = find_pieces(board, type, pieces);

	for (i = 0; i < num; i++)
	{
		if (!check_mill(board, pieces[i], type)) {
			allMills = FALSE;
		}
	}
	return allMills;
}

// Given bboard, int array
// Return number of pieces and array of each slot containing those pieces
int find_pieces(char *board, char piece, int *pieces)
{
	int i;
	int num = 0;

	for (i = 0; i < BOARDSIZE; i++) {
		if (board[i] == piece) {
			pieces[num] = i;
			num++;
		}
	}

	return num;
}

// given old position and the next move
BOOLEAN closes_mill(POSITION position, int raw_move)
{
	char *board;
	char turn,tempturn;
	int piecesLeft, numx, numo;
	BOOLEAN mill;
	POSITION temppos;
	board=unhash(position, &turn, &piecesLeft, &numx, &numo); //do the move onto board
	tempturn = turn;
	// printf("closesmill tempturn: %c\n", tempturn);
	temppos = EvalMove(board,turn,piecesLeft,numx,numo,raw_move, position);
	//printf("turn after EvalMove: %c\n", turn);
	SafeFree(board);
	board=unhash(temppos, &turn, &piecesLeft, &numx, &numo); //do the move onto board
	turn = tempturn;
	//printf("finalturn: %c\n", turn);
	if (piecesLeft==0) {
		mill = check_mill(board, (raw_move%(BOARDSIZE*BOARDSIZE))/BOARDSIZE, turn);
		SafeFree(board);
		return mill;
	}
	else
	{
		//printf("CLOSE MILL ELSE\n");
		mill = check_mill(board, (raw_move/(BOARDSIZE*BOARDSIZE)), turn);
		SafeFree(board);
		return mill;
	}
}

POSITION EvalMove(char* board,char turn,int piecesLeft,int numx,int numo,MOVE move, POSITION position)
{

	//printf("inside DoMove\n");

	//printf("totalPieces: %d\n", totalPieces);

	int to = (move%(BOARDSIZE*BOARDSIZE)) / BOARDSIZE;
	int from = move / (BOARDSIZE * BOARDSIZE);
	int remove = move % BOARDSIZE;

	if (piecesLeft == 0) // stage 2
	{
		//printf("inside DoMove. to=%d, from = %d, remove = %d\n", to, from, remove); //DEBUG
		board[to] = board[from];
		board[from] = BLANK;
		board[remove] = BLANK; //if remove wasn't specified in the string, it is by default equal to from
	}

	else // stage 1
	{
		//printf("DoMove stage 1\n");
		board[from] = turn;
		if (from != to)
			board[to] = BLANK;
	}

	//printf("numx: %d numo: %d\n", numx, numo);

	//printf("EVALMOVE TURN: %c\n", turn);

	if (turn == X) {
		return updatepieces(board,O,piecesLeft, numx, numo, move, position);
	}
	else{
		return updatepieces(board, X, piecesLeft, numx, numo,move,position);
	}


}

// given new board, slot
// return true if slot is member of mill
BOOLEAN check_mill(char *board, int slot, char turn)
{
	if (gameType == 3) {
		switch (slot) {
		case 0:
			return three_in_a_row(board, 1, 2, turn)|| three_in_a_row(board,3, 6, turn);
			break;
		case 1:
			return three_in_a_row(board, 0, 2, turn)|| three_in_a_row(board,4, 7, turn);
			break;
		case 2:
			return three_in_a_row(board, 1, 0, turn)|| three_in_a_row(board,5, 8, turn);
			break;
		case 3:
			return three_in_a_row(board, 4, 5, turn)|| three_in_a_row(board,0, 6, turn);
			break;
		case 4:
			return three_in_a_row(board, 1, 7, turn)|| three_in_a_row(board, 3, 5, turn);
			break;
		case 5:
			return three_in_a_row(board, 8, 2, turn)|| three_in_a_row(board,3, 4, turn);
			break;
		case 6:
			return three_in_a_row(board, 0, 3, turn)|| three_in_a_row(board,7, 8, turn);
			break;
		case 7:
			return three_in_a_row(board, 1, 4, turn)|| three_in_a_row(board,6, 8, turn);
			break;
		case 8:
			return three_in_a_row(board, 5, 2, turn)|| three_in_a_row(board,6, 7, turn);
			break;
		default:
			return FALSE;
			break;
		}
	}
	else if (gameType == 6)
	{
		switch (slot) {
		case 0:
			return three_in_a_row(board, 1, 2, turn)|| three_in_a_row(board,6, 13, turn);
			break;
		case 1:
			return three_in_a_row(board, 0, 2, turn);
			break;
		case 2:
			return three_in_a_row(board, 1, 0, turn)|| three_in_a_row(board,9, 15, turn);
			break;
		case 3:
			return three_in_a_row(board, 4, 5, turn)|| three_in_a_row(board,7, 10, turn);
			break;
		case 4:
			return three_in_a_row(board, 3, 5, turn);
			break;
		case 5:
			return three_in_a_row(board, 8, 12, turn)|| three_in_a_row(board,3, 4, turn);
			break;
		case 6:
			return three_in_a_row(board, 0, 13, turn);
			break;
		case 7:
			return three_in_a_row(board, 3, 10, turn);
			break;
		case 8:
			return three_in_a_row(board, 5, 12, turn);
			break;
		case 9:
			return three_in_a_row(board, 2, 15, turn);
			break;
		case 10:
			return three_in_a_row(board, 3, 7, turn) || three_in_a_row(board,11,12, turn);
			break;
		case 11:
			return three_in_a_row(board, 10, 12, turn);
			break;
		case 12:
			return three_in_a_row(board, 10, 11, turn)|| three_in_a_row(board,5, 8, turn);
			break;
		case 13:
			return three_in_a_row(board, 14, 15, turn)|| three_in_a_row(board,0, 6, turn);
			break;
		case 14:
			return three_in_a_row(board, 13, 15, turn);
			break;
		case 15:
			return three_in_a_row(board, 13, 14, turn)|| three_in_a_row(board,2, 9, turn);
			break;
		default:
			return FALSE;
			break;
		}
	}
	else if(gameType == 9) {
		switch (slot) {
		case 0:
			return three_in_a_row(board, 1, 2, turn)|| three_in_a_row(board,9, 21, turn);
			break;
		case 1:
			return three_in_a_row(board, 0, 2, turn) || three_in_a_row(board, 4, 7, turn);
			break;
		case 2:
			return three_in_a_row(board, 1, 0, turn)|| three_in_a_row(board,14, 23, turn);
			break;
		case 3:
			return three_in_a_row(board, 4, 5, turn)|| three_in_a_row(board,10, 18, turn);
			break;
		case 4:
			return three_in_a_row(board, 1, 7, turn) || three_in_a_row(board, 3, 5, turn);
			break;
		case 5:
			return three_in_a_row(board, 4, 3, turn)|| three_in_a_row(board,13, 20, turn);
			break;
		case 6:
			return three_in_a_row(board, 7, 8, turn) || three_in_a_row(board, 11, 15, turn);
			break;
		case 7:
			return three_in_a_row(board, 1, 4, turn) || three_in_a_row(board, 6, 8, turn);
			break;
		case 8:
			return three_in_a_row(board, 7, 6, turn) || three_in_a_row(board, 12, 17, turn);
			break;
		case 9:
			return three_in_a_row(board, 0, 21, turn) || three_in_a_row(board, 10, 11, turn);
			break;
		case 10:
			return three_in_a_row(board, 9, 11, turn) || three_in_a_row(board,3,18, turn);
			break;
		case 11:
			return three_in_a_row(board, 9, 10, turn) || three_in_a_row(board, 6, 15, turn);
			break;
		case 12:
			return three_in_a_row(board, 8, 17, turn)|| three_in_a_row(board,13, 14, turn);
			break;
		case 13:
			return three_in_a_row(board, 12, 14, turn)|| three_in_a_row(board,5, 20, turn);
			break;
		case 14:
			return three_in_a_row(board, 12, 13, turn) || three_in_a_row(board, 2, 23, turn);
			break;
		case 15:
			return three_in_a_row(board, 6, 11, turn)|| three_in_a_row(board,16, 17, turn);
			break;
		case 16:
			return three_in_a_row(board, 15, 17, turn)|| three_in_a_row(board,19, 22, turn);
			break;
		case 17:
			return three_in_a_row(board, 15, 16, turn)|| three_in_a_row(board,8, 12, turn);
			break;
		case 18:
			return three_in_a_row(board, 3, 10, turn)|| three_in_a_row(board,19, 20, turn);
			break;
		case 19:
			return three_in_a_row(board, 18, 20, turn)|| three_in_a_row(board,16, 22, turn);
			break;
		case 20:
			return three_in_a_row(board, 18, 19, turn)|| three_in_a_row(board,5, 13, turn);
			break;
		case 21:
			return three_in_a_row(board, 0, 9, turn)|| three_in_a_row(board,22, 23, turn);
			break;
		case 22:
			return three_in_a_row(board, 16, 19, turn)|| three_in_a_row(board,21, 23, turn);
			break;
		case 23:
			return three_in_a_row(board, 21, 22, turn)|| three_in_a_row(board,2, 14, turn);
			break;
		default:
			return FALSE;
			break;
		}
	}

	return FALSE;
}

// given new board, slots to compare.  if slots all same, then it's a 3
BOOLEAN three_in_a_row(char *board, int slot1, int slot2, char turn)
{

	//printf("THREE IN A ROW TURN: %c\n", turn);
	//printf("0: %c\t 1: %c\n", board[0], board[1]);

	return board[slot1] == turn && board[slot2] == turn;
}

// Given slot, int array
// Return number of adjacent slots, array of those slot numbers
int find_adjacent(int slot, int *slots)
{
	// multiples of 3 (0, 3, 6, 9, 12, 15, 18, 21) are left-most edge
	// 0, 1, 2, 3, 5, 6, 8 are top-most
	// 7, 15, 16, 17, 18, 19, 20, 21, 22, 23 are bottom-most
	// 2, 5, 8, 11, 14, 17, 20, 23 are right-most (each differs by 3)
	// 4, 10, 13, 19 are centered (have adjacent pieces in all 4 directions)

	int num = 0;

	if (gameType==3) {
		switch (slot) {
		case 0:
			slots[num++] = 1;
			slots[num++] = 3;
			break;
		case 1:
			slots[num++] = 0;
			slots[num++] = 2;
			slots[num++] = 4;
			break;
		case 2:
			slots[num++] = 1;
			slots[num++] = 5;
			break;
		case 3:
			slots[num++] = 0;
			slots[num++] = 4;
			slots[num++] = 6;
			break;
		case 4:
			slots[num++] = 1;
			slots[num++] = 3;
			slots[num++] = 5;
			slots[num++] = 7;
			break;
		case 5:
			slots[num++] = 2;
			slots[num++] = 4;
			slots[num++] = 8;
			break;
		case 6:
			slots[num++] = 7;
			slots[num++] = 3;
			break;
		case 7:
			slots[num++] = 4;
			slots[num++] = 6;
			slots[num++] = 8;
			break;
		case 8:
			slots[num++] = 5;
			slots[num++] = 7;
			break;
		default:
			num = 0;
			slots[0] = -1;
			break;
		}
	}

	else if (gameType == 6)
	{
		switch (slot) {
		case 0:
			slots[num++] = 1;
			slots[num++] = 6;
			break;
		case 1:
			slots[num++] = 0;
			slots[num++] = 2;
			slots[num++] = 4;
			break;
		case 2:
			slots[num++] = 1;
			slots[num++] = 9;
			break;
		case 3:
			slots[num++] = 4;
			slots[num++] = 7;
			break;
		case 4:
			slots[num++] = 1;
			slots[num++] = 3;
			slots[num++] = 5;
			break;
		case 5:
			slots[num++] = 4;
			slots[num++] = 8;
			break;
		case 6:
			slots[num++] = 0;
			slots[num++] = 7;
			slots[num++] = 13;
			break;
		case 7:
			slots[num++] = 3;
			slots[num++] = 6;
			slots[num++] = 10;
			break;
		case 8:
			slots[num++] = 5;
			slots[num++] = 9;
			slots[num++] = 12;
			break;
		case 9:
			slots[num++] = 2;
			slots[num++] = 8;
			slots[num++] = 15;
			break;
		case 10:
			slots[num++] = 7;
			slots[num++] = 11;
			break;
		case 11:
			slots[num++] = 10;
			slots[num++] = 12;
			slots[num++] = 14;
			break;
		case 12:
			slots[num++] = 8;
			slots[num++] = 11;
			break;
		case 13:
			slots[num++] = 6;
			slots[num++] = 14;
			break;
		case 14:
			slots[num++] = 11;
			slots[num++] = 13;
			slots[num++] = 15;
			break;
		case 15:
			slots[num++] = 9;
			slots[num++] = 14;
			break;
		default:
			num = 0;
			slots[0] = -1;
			break;
		}
	}


	if (gameType == 9)
	{
		switch (slot) {
		case 0:
			// num = 2;
			slots[num++] = slot + 1;
			slots[num++] = slot + 9;
			break;
		case 3:
			// num = 2;
			slots[num++] = slot + 1;
			slots[num++] = slot + 7;
			break;
		case 6:
			// num = 2;
			slots[num++] = slot + 1;
			slots[num++] = slot + 5;
			break;
		case 2:
			// num = 2;
			slots[num++] = slot - 1;
			slots[num++] = slot + 12;
			break;
		case 5:
			// num = 2;
			slots[num++] = slot - 1;
			slots[num++] = slot + 8;
			break;
		case 8:
			// num = 2;
			slots[num++] = slot - 1;
			slots[num++] = slot + 4;
			break;
		case 21:
			// num = 2;
			slots[num++] = slot + 1;
			slots[num++] = slot - 12;
			break;
		case 18:
			// num = 2;
			slots[num++] = slot + 1;
			slots[num++] = slot - 8;
			break;
		case 15:
			// num = 2;
			slots[num++] = slot + 1;
			slots[num++] = slot - 4;
			break;
		case 23:
			// num = 2;
			slots[num++] = slot - 1;
			slots[num++] = slot - 9;
			break;
		case 20:
			// num = 2;
			slots[num++] = slot - 1;
			slots[num++] = slot - 7;
			break;
		case 17:
			// num = 2;
			slots[num++] = slot - 1;
			slots[num++] = slot - 5;
			break;
		case 1:
			// num = 2;
			slots[num++] = slot - 1;
			slots[num++] = slot + 1;
			slots[num++] = slot + 3;
			break;
		case 16:
			// num = 2;
			slots[num++] = slot - 1;
			slots[num++] = slot + 1;
			slots[num++] = slot + 3;
			break;
		case 7:
			// num = 2;
			slots[num++] = slot - 1;
			slots[num++] = slot + 1;
			slots[num++] = slot - 3;
			break;
		case 22:
			// num = 3;
			slots[num++] = slot - 1;
			slots[num++] = slot + 1;
			slots[num++] = slot - 3;
			break;
		case 9:
			// num = 3;
			slots[num++] = slot + 1;
			slots[num++] = slot + 12;
			slots[num++] = slot - 9;
			break;
		case 12:
			// num = 3;
			slots[num++] = slot + 1;
			slots[num++] = slot + 5;
			slots[num++] = slot - 4;
			break;
		case 11:
			// num = 3;
			slots[num++] = slot - 1;
			slots[num++] = slot + 4;
			slots[num++] = slot - 5;
			break;
		case 14:
			// num = 3;
			slots[num++] = slot - 1;
			slots[num++] = slot + 9;
			slots[num++] = slot - 12;
			break;
		case 4: case 19:
			// num = 4;
			slots[num++] = slot + 1;
			slots[num++] = slot - 1;
			slots[num++] = slot + 3;
			slots[num++] = slot - 3;
			break;
		case 10:
			// num = 4;
			slots[num++] = slot + 1;
			slots[num++] = slot - 1;
			slots[num++] = slot + 8;
			slots[num++] = slot - 7;
			break;
		case 13:
			// num = 4;
			slots[num++] = slot + 1;
			slots[num++] = slot - 1;
			slots[num++] = slot + 7;
			slots[num++] = slot - 8;
			break;
		default:
			num = 0;
			slots[0] = -1;
			break;
		}
	}
	return num;
}
void changetothree(){
	gameType = 3;
	BOARDSIZE = 9;
	maxx = 3;
	maxo = 3;
	minb = 3;
	maxb = 4;
	totalPieces = maxx + maxo;
	kDBName = "3mm";
}

void changetosix()
{
	//printf("changing to 6666666\n");
	gameType = 6;
	BOARDSIZE =16;
	maxx = 6;
	maxo = 6;
	minb = 4;
	maxb = 11;
	totalPieces = maxx + maxo;
	kDBName = "6mm";
}

void changetonine()
{
	gameType = 9;
	BOARDSIZE = 24;
	maxx = 9;
	maxo = 9;
	minb = 6;
	maxb = 19;
	totalPieces = maxx + maxo;
	kDBName = "9mm";

	//SYMMETRIES
	kSupportsSymmetries = TRUE; /* Whether we support symmetries */

	int gSymmetryMatrix2[9][24] = { {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23},{2,1,0,5,4,3,8,7,6,14,13,12,11,10,9,17,16,15,20,19,18,23,22,21},{21,22,23,18,19,20,15,16,17,9,10,11,12,13,14,6,7,8,3,4,5,0,1,2},{23,14,2,20,13,5,17,12,8,22,19,16,7,4,1,15,11,6,18,10,3,21,9,0},{0,9,21,3,10,18,6,11,15,1,4,7,16,19,22,8,12,17,5,13,20,2,14,23},{21,9,0,18,10,3,15,11,6,22,19,16,7,4,1,17,12,8,20,13,5,23,14,2},{23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0},{2,14,23,5,13,20,8,12,17,1,4,7,16,19,22,6,11,15,3,10,18,0,9,21},{6,7,8,3,4,5,0,1,2,11,10,9,14,13,12,21,22,23,18,19,20,15,16,17} };

	int i, k;
	for (i=0; i < 9; i++) {
		for (k=0; k < 24; k++) {
			//printf("%d", gSymmetryMatrix2[i][k]);
			gSymmetryMatrix[i][k] = gSymmetryMatrix2[i][k];
		}
	}
	//gSymmetryMatrix = gSymmetryMatrix2;
	/*gSymmetryMatrixptr = (int *) malloc (sizeof(int*)*NUMSYMMETRIES);
	   for (int k=0; k < NUMSYMMETRIES); k++){
	        (*gSymmetryMatrixptr) = (int *) malloc (sizeof(int)*BOARDSIZE);
	   }*/

	/*NUMSYMMETRIES = 9;   //  4 rotations, 4 flipped rotations, outer-inner flip

	   int gSymmetryMatrix[NUMSYMMETRIES][BOARDSIZE];

	   if(kSupportsSymmetries) {
	        gSymmetryMatrix[0] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23};
	        gSymmetryMatrix[1] = {2,1,0,5,4,3,8,7,6,14,13,12,11,10,9,17,16,15,20,19,18,23,22,21};
	        gSymmetryMatrix[2] = {21,22,23,18,19,20,15,16,17,9,10,11,12,13,14,6,7,8,3,4,5,0,1,2};
	        gSymmetryMatrix[3] = {23,14,2,20,13,5,17,12,8,22,19,16,7,4,1,15,11,6,18,10,3,21,9,0};
	        gSymmetryMatrix[4] = {0,9,21,3,10,18,6,11,15,1,4,7,16,19,22,8,12,17,5,13,20,2,14,23};
	        gSymmetryMatrix[5] = {21,9,0,18,10,3,15,11,6,22,19,16,7,4,1,17,12,8,20,13,5,23,14,2};
	        gSymmetryMatrix[6] = {23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0};
	        gSymmetryMatrix[7] = {2,14,23,5,13,20,8,12,17,1,4,7,16,19,22,6,11,15,3,10,18,0,9,21};
	        gSymmetryMatrix[8] = {6,7,8,3,4,5,0,1,2,11,10,9,14,13,12,21,22,23,18,19,20,15,16,17};
	   }*/
}


/**************************************************/
/**************** SYMMETRY FUN BEGIN **************/
/**************************************************/

/************************************************************************
**
** NAME:        GetCanonicalPosition
**
** DESCRIPTION: Go through all of the positions that are symmetrically
**              equivalent and return the SMALLEST, which will be used
**              as the canonical element for the equivalence set.
**
** INPUTS:      POSITION position : The position return the canonical elt. of.
**
** OUTPUTS:     POSITION          : The canonical element of the set.
**
************************************************************************/

POSITION GetCanonicalPosition(POSITION position)
{
	POSITION newPosition, theCanonicalPosition, DoSymmetry();
	int i;

	theCanonicalPosition = position;

	for(i = 0; i < NUMSYMMETRIES; i++) {
		newPosition = DoSymmetry(position, i); /* get new */
		if(newPosition < theCanonicalPosition) /* THIS is the one */
			theCanonicalPosition = newPosition; /* set it to the ans */
	}

	return(theCanonicalPosition);
}

/************************************************************************
**
** NAME:        DoSymmetry
**
** DESCRIPTION: Perform the symmetry operation specified by the input
**              on the position specified by the input and return the
**              new position, even if it's the same as the input.
**
** INPUTS:      POSITION position : The position to branch the symmetry from.
**              int      symmetry : The number of the symmetry operation.
**
** OUTPUTS:     POSITION, The position after the symmetry operation.
**
************************************************************************/

POSITION DoSymmetry(POSITION position, int symmetry)
{
	int i;
	char turn;
	char* board = smallUnhash(position, &turn);
	char* symmBoard = (char*) SafeMalloc(BOARDSIZE * sizeof(char));

	/* Copy from the symmetry matrix */
	//printf("doing symmetry");
	for(i = 0; i < BOARDSIZE; i++)
		symmBoard[i] = board[gSymmetryMatrix[symmetry][i]];

	if (board != NULL)
		SafeFree(board);
	return(smallHash(symmBoard, turn));
}

POSITION smallHash(char* board, char turn){
	POSITION pos;
	TIER tier = generic_hash_cur_context();
	generic_hash_context_switch(tier);
	TIERPOSITION tierpos = generic_hash_hash(board, (turn == X ? PLAYER_ONE : PLAYER_TWO));
	pos = gHashToWindowPosition(tierpos, tier);

	return pos;
}

char* smallUnhash(POSITION pos, char* turn)
{
	char* board = (char*)SafeMalloc(BOARDSIZE * sizeof(char));
	if(gHashWindowInitialized) {
		//printf("unhashing with tiers\n");
		TIER tier; TIERPOSITION tierposition;
		gUnhashToTierPosition(pos, &tierposition, &tier);
		//printf("unhashing %d\n", tier);
		generic_hash_context_switch(tier);
		board = (char*)generic_hash_unhash(tierposition, board);
		(*turn) = generic_hash_turn(tierposition);
	}
	return board;
}


/**************************************************/
/**************** SYMMETRY FUN END ****************/
/**************************************************/



/************************************************************************
** Changelog
**
** $Log: not supported by cvs2svn $
** Revision 1.3  2008/05/08 02:28:12  noafroboy
** latest version
**
** Revision 1.2  2008/05/01 03:44:52  noafroboy
** added the three variants for removing pieces after forming mills
**
** Revision 1.13  2008/05/01 02:03:56  noafroboy
** latest m6mm.c with some bugs fixed... see wiki for details
**
** Revision 1.12  2008/04/29 08:42:30  noafroboy
** fixed the tcl so that 3mm and 6mm work. 9mm works for first 12 moves. possible database error
**
** Revision 1.11  2008/04/29 07:18:43  noafroboy
** added 3mm. starts with 3mm by default for now.
**
** Revision 1.10  2008/03/18 02:34:09  noafroboy
** This version is for the Stanford Demonstration. 9mm has been removed. In addition, gCurrentTier is set each time DoMove is called in order for the GUI to know the phase it is in.
**
** Revision 1.9  2007/11/29 05:00:29  noafroboy
** temporary hack for m6mm.c
**
** Revision 1.8  2007/11/26 10:41:26  noafroboy
** BUGZID:
** 1.  Added 9 men's morris under game specific options.
** 2.  Polished PrintPosition for 9mm board.
** 3.  Slightly edited SetupTierStuff to accomodate above changes.
**
** Revision 1.7  2007/11/19 04:32:44  ddgarcia
** Sooo many changes:
**
** 1. Added gFlying (global, changed GenerateMoves, added getOption, setoption)
**   (now variants are 1-4 ; before they were 0, default is no flying)
** 2. Supported misere with gStandardGame in Primitive
** 3. polished PrintPosition
** 4. Filled in PrintComputersMove
** 5. Removed [ and ] from single moves in MoveToString
** 6. Cleaned GetAndPrintPlayersMove
**
** Revision 1.6  2007/11/07 03:37:39  patricia_fong
** fixed a small part in MoveToString
**
** Revision 1.5  2007/10/17 10:06:07  patricia_fong
** added functions to m6mm.c for tcl
**
** Revision 1.3  2006/12/19 20:00:50  arabani
** Added Memwatch (memory debugging library) to gamesman. Use 'make memdebug' to compile with Memwatch
**
** Revision 1.2  2006/12/07 03:37:20  max817
** Hash and NMM changes.
**
** Revision 1.10  2006/04/25 01:33:06  ogren
** Added InitialiseHelpStrings() as an additional function for new game modules to write.  This allows dynamic changing of the help strings for every game without adding more bookkeeping to the core.  -Elmer
**
************************************************************************/

/*
   Tier
   1st stage: number of stones for x left, and o, position.
   2nd stage: pieces on the board of each player
   if numx and numo == 9 and piecesleft != 0, then stage 1
   if numx and numo <= 9 and piecesleft == 0, then stage 2
 */

/*Changes:
   unhash is now
   char* unhash(POSITION pos, char* turn, int* piecesLeft, int* numx, int* numo)
   with piecesLeft, numx, and numo being globals until tiers are implemented.

   hash is now
   POSITION hash(char* board, char turn, int piecesLeft, int numx, int numo)

   with piecesLeft, numx, and numo being globals until tiers are implemented.

   find_adjacent is ready for 6mm.

   ConvertTextInputToMove: need to check:
   IN STAGE 1:
 * 1 number -placing piece
               - [1] ~ board[1] = turn;
 * 2 numbers - placing piece and removing opponent's piece
                        - [1 2] ~ board[1] = turn; board[2] = blank;


   IN STAGE 2:
 * 2 numbers - moving from and to
                - [1 2] ~ board[1] = blank; board[2] = turn <OPPOSITE OF STAGE 1!>
 * 3 numbers - moving from and to and removing opponent's piece
                        - [1 2 3] ~ board[1] = blank; board[2] = turn; board[3] = blank;


   please kill ValidTextInput until we're done with this. Not worth spending time
   on this right now.

   KEVIN: 10/28
        - updated whoseMove to generic_hash_turn
                      generic_hash to generic_hash_hash
                      generic_unhash to generic_hash_unhash
                          because Max changed the hash core to those names
        -fixed a few overlooked bugs: not passing turn, piecesleft, numx, and numo arguments by reference when calling unhash
        -added NUMX and NUMO globals because you forgot
   PrintPosition is ready for 6mm.
        looks good
   Primitive: nothing has been changed for 6mm... should work for 6mm
        looks good
   GENERATEMOVES NEEDS TO BE LOOKED OVER
        pseudocode written, it looks like we will need to copy a few more of their helper functions over. doing it tomorrow. should be quick.
   DOMOVE NEEDS TO BE LOOKED OVER
        finished
   MOVETOSTRING NEEDS TO BE CHANGED SO THAT IT DOESNT INCLUDE REMOVE NUMBERS IF
   THERE IS NOTHING TO REMOVE.
        this was already taken care of previously


   we should use the one liner functions from, to, and remove.
        it makes the code a whole lot easier to read and reduces likely hood of errors... such as the mistake in DoMove earlier
 */
POSITION StringToPosition(char* board) {

	char realBoard[BOARDSIZE];
	int i = 0;
	for (i = 0; i < BOARDSIZE; i++) {
                if (board[i] == ' ') {
		        realBoard[i] = '·';
                } else {
		        realBoard[i] = board[i];
                }
	}
	return generic_hash_hash(realBoard, 0);
}



char* PositionToString(POSITION pos) {
	char board[BOARDSIZE];
	int i = 0;
	generic_hash_unhash(pos, &board);
	char* finalBoard = calloc((BOARDSIZE+1), sizeof(char));
	for (i = 0; i < BOARDSIZE; i++) {
                if (board[i] == '·') {
                        finalBoard[i] = ' ';
                } else {
		        finalBoard[i] = board[i];
                }
	}
	return finalBoard;
}

char * PositionToEndData(POSITION pos) {
	return NULL;
}
