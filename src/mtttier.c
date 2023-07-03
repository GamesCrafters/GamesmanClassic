/************************************************************************
**
** NAME:        mtttier.c
**
** DESCRIPTION: Tic-Tac-Tier!
**				Basically, this is a copy-pasted version of mttt.c,
**				only changed to use generic hash and Tier Gamesman,
**				removing unnecessary things like GPS stuff.
**
** AUTHOR:      Dan Garcia  -  University of California at Berkeley
**              Copyright (C) Dan Garcia, 1995. All rights reserved.
**				Copy-Pasted and Revised 2006 by Max Delgadillo
**
** DATE:        09/27/06
**
** UPDATE HIST:
**
**  7/24/06 :	First version of the file. SHOULD be bug-free...
**				Four of the six API functions are implemented. Only
**				gInitHashWindow and gPositionToTierPosition are left
**				to write (since the current version of the solver
**				doesn't use either of the two functions yet). They
**				should be added by the next update.
**  7/29/06 :	Fixed a BIG gNumberOfPositions glitch, as well as a
**				WhoseMove glitch that was causing double boards in the
**				hash. Also, gInitHashWindow and gPositionToTierPosition
**				are now correctly implemented, finally completing the
**				entire Tier Gamesman API example.
**	8/01/06	:	Quite a big change. This now uses the NEW API, which
**				includes the use of the generic Hash Window functions.
**				It now works with the newest version of the solver and
**				generates 100% correct memdb tier databases.
**				The last things left for this file is prettying up the
**				code (particularly WhoseTurn) and settling on how to
**				handle gInitialPosition.
**	8/03/06:	Added support for game playing by adding gInitialTier
**				and gInitialTierPosition.
**	8/05/06:	Made the hashes 100% efficient. IsLegal is no longer
**				required!
**	8/07/06:	The global hash now uses singleplayer mode like the
**				tier hashes. Also I realize the hashes aren't ACTUALLY
**				100% efficient (and IsLegal is still needed) since there's
**				some positions where a player has already won but more
**				pieces are still placed. But... aside from that (if I ever
**				choose to make that fix anyway), this is as close to the
**				final version of the file as it gets.
**	8/08/06:	Added TierToString.
**	9/27/06:	Removed instances of "Tier0Context", and instead use the
**				generic hash destructor (which works now!).
**
**
**************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/

#include "gamesman.h"

POSITION gNumberOfPositions  = 0;
POSITION kBadPosition        = -1;

POSITION gInitialPosition    =  0;
POSITION gMinimalPosition    =  0;

CONST_STRING kAuthorName         = "Dan Garcia (and Max Delgadillo)";
CONST_STRING kGameName           = "Tic-Tac-Tier";
CONST_STRING kDBName             = "tttier";
BOOLEAN kPartizan           = TRUE;
BOOLEAN kDebugMenu          = FALSE;
BOOLEAN kGameSpecificMenu   = FALSE;
BOOLEAN kTieIsPossible      = TRUE;
BOOLEAN kLoopy               = FALSE;
BOOLEAN kDebugDetermineValue = FALSE;
void*    gGameSpecificTclInit = NULL;

CONST_STRING kHelpGraphicInterface =
        "The LEFT button puts an X or O (depending on whether you went first\n\
or second) on the spot the cursor was on when you clicked. The MIDDLE\n\
button does nothing, and the RIGHT button is the same as UNDO, in that\n\
it reverts back to your your most recent position."                                                                                                                                                                                                                                   ;

CONST_STRING kHelpTextInterface    =
        "On your turn, use the LEGEND to determine which number to choose (between\n\
1 and 9, with 1 at the upper left and 9 at the lower right) to correspond\n\
to the empty board position you desire and hit return. If at any point\n\
you have made a mistake, you can type u and hit return and the system will\n\
revert back to your most recent position."                                                                                                                                                                                                                                                                                                                           ;

CONST_STRING kHelpOnYourTurn =
        "You place one of your pieces on one of the empty board positions.";

CONST_STRING kHelpStandardObjective =
        "To get three of your markers (either X or O) in a row, either\n\
horizontally, vertically, or diagonally. 3-in-a-row WINS."                                                                          ;

CONST_STRING kHelpReverseObjective =
        "To force your opponent into getting three of his markers (either X or\n\
O) in a row, either horizontally, vertically, or diagonally. 3-in-a-row\n\
LOSES."                                                                                                                                                             ;

CONST_STRING kHelpTieOccursWhen =   /* Should follow 'A Tie occurs when... */
                            "the board fills up without either player getting three-in-a-row.";

CONST_STRING kHelpExample =
        "         ( 1 2 3 )           : - - -\n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - - - \n\
         ( 7 8 9 )           : - - - \n\n\
Computer's move              :  3    \n\n\
         ( 1 2 3 )           : - - X \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - - - \n\
         ( 7 8 9 )           : - - - \n\n\
     Dan's move [(u)ndo/1-9] : { 2 } \n\n\
         ( 1 2 3 )           : - O X \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - - - \n\
         ( 7 8 9 )           : - - - \n\n\
Computer's move              :  6    \n\n\
         ( 1 2 3 )           : - O X \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - - X \n\
         ( 7 8 9 )           : - - - \n\n\
     Dan's move [(u)ndo/1-9] : { 9 } \n\n\
         ( 1 2 3 )           : - O X \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - - X \n\
         ( 7 8 9 )           : - - O \n\n\
Computer's move              :  5    \n\n\
         ( 1 2 3 )           : - O X \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - X X \n\
         ( 7 8 9 )           : - - O \n\n\
     Dan's move [(u)ndo/1-9] : { 7 } \n\n\
         ( 1 2 3 )           : - O X \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - X X \n\
         ( 7 8 9 )           : O - O \n\n\
Computer's move              :  4    \n\n\
         ( 1 2 3 )           : - O X \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : X X X \n\
         ( 7 8 9 )           : O - O \n\n\
Computer wins. Nice try, Dan."                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             ;

/*************************************************************************
**
** Everything above here must be in every game file
**
**************************************************************************/

/*************************************************************************
**
** Every variable declared here is only used in this file (game-specific)
**
**************************************************************************/

#define BOARDSIZE     9           /* 3x3 board */
// I decided on chars instead of the "BlankOX" struct because it's
// easier for the generic hash to handle.
#define Blank   '-'
#define o               'O'
#define x               'X'
// This is so I don't have to change "BlankOX" occurences everywhere.
typedef char BlankOX;

/** Function Prototypes **/
BOOLEAN AllFilledIn(BlankOX*);
BOOLEAN ThreeInARow(BlankOX*, int, int, int);
POSITION GetCanonicalPosition(POSITION);
STRING MoveToString(MOVE);

// HASH/UNHASH
char* customUnhash(POSITION);
POSITION BlankOXToPosition(BlankOX*);
BlankOX* PositionToBlankOX(POSITION);
BlankOX WhoseTurn(BlankOX*);
TIER BoardToTier(BlankOX*);

//TIER GAMESMAN
void SetupTierStuff();
TIERLIST* TierChildren(TIER);
TIERPOSITION NumberOfTierPositions(TIER);
void GetInitialTierPosition(TIER*, TIERPOSITION*);
BOOLEAN IsLegal(POSITION);
UNDOMOVELIST* GenerateUndoMovesToTier(POSITION, TIER);
STRING TierToString(TIER);
POSITION UnDoMove(POSITION, UNDOMOVE);
// Actual functions are at the end of this file

//SYMMETRIES
BOOLEAN kSupportsSymmetries = TRUE; /* Whether we support symmetries */

#define NUMSYMMETRIES 8   /*  4 rotations, 4 flipped rotations */

int gSymmetryMatrix[NUMSYMMETRIES][BOARDSIZE];

/* Proofs of correctness for the below arrays:
**
** FLIP						ROTATE
**
** 0 1 2	2 1 0		0 1 2		6 3 0		8 7 6		2 5 8
** 3 4 5 -> 5 4 3		3 4 5	->	7 4 1  ->	5 4 3	->	1 4 7
** 6 7 8	8 7 6		6 7 8		8 5 2		2 1 0		2 1 0
*/

/* This is the array used for flipping along the N-S axis */
int gFlipNewPosition[] = { 2, 1, 0, 5, 4, 3, 8, 7, 6 };

/* This is the array used for rotating 90 degrees clockwise */
int gRotate90CWNewPosition[] = { 6, 3, 0, 7, 4, 1, 8, 5, 2 };


/************************************************************************
**
** NAME:        InitializeGame
**
** DESCRIPTION: Prepares the game for execution.
**              Initializes required variables.
**              Sets up gDatabase (if necessary).
**
************************************************************************/

// for the hash init
int vcfg(int* this_cfg) {
	return (this_cfg[0] == this_cfg[1] || this_cfg[0] + 1 == this_cfg[1]);
}

void InitializeGame()
{
	// SYMMETRY
	gCanonicalPosition = GetCanonicalPosition;
	int i, j, temp; /* temp is used for debugging */
	if(kSupportsSymmetries) { /* Initialize gSymmetryMatrix[][] */
		for(i = 0; i < BOARDSIZE; i++) {
			temp = i;
			for(j = 0; j < NUMSYMMETRIES; j++) {
				if(j == NUMSYMMETRIES/2)
					temp = gFlipNewPosition[i];
				if(j < NUMSYMMETRIES/2)
					temp = gSymmetryMatrix[j][i] = gRotate90CWNewPosition[temp];
				else
					temp = gSymmetryMatrix[j][i] = gRotate90CWNewPosition[temp];
			}
		}
	}

	gMoveToStringFunPtr = &MoveToString;
	gCustomUnhash = &customUnhash;
	// gCustomUnhash is a (STRING) char *
	// linearUnhash expects void *
	// dchan 10-16-07
	linearUnhash = (void *) gCustomUnhash;

	//discard current hash
	generic_hash_destroy();

	//Setup Tier Stuff (at bottom)
	SetupTierStuff();

	//have a GLOBAL HASH set up at outset:
	int game[10] = { o, 0, 4, x, 0, 5, Blank, 0, 9, -1 };
	gNumberOfPositions = generic_hash_init(BOARDSIZE, game, vcfg, 1);

	// gInitialPosition
	BlankOX* board = (BlankOX *) SafeMalloc(BOARDSIZE * sizeof(BlankOX));
	for (i = 0; i < BOARDSIZE; i++)
		board[i] = Blank;
	gInitialPosition = BlankOXToPosition(board);
}

void FreeGame()
{
}

/************************************************************************
**
** NAME:        DebugMenu
**
** DESCRIPTION: Menu used to debub internal problems. Does nothing if
**              kDebugMenu == FALSE
**
************************************************************************/

void DebugMenu() {
}

/************************************************************************
**
** NAME:        GameSpecificMenu
**
** DESCRIPTION: Menu used to change game-specific parmeters, such as
**              the side of the board in an nxn Nim board, etc. Does
**              nothing if kGameSpecificMenu == FALSE
**
************************************************************************/

void GameSpecificMenu() {
}

/************************************************************************
**
** NAME:        SetTclCGameSpecificOptions
**
** DESCRIPTION: Set the C game-specific options (called from Tcl)
**              Ignore if you don't care about Tcl for now.
**
************************************************************************/

void SetTclCGameSpecificOptions(int theOptions[])
{
	/* No need to have anything here, we have no extra options */
	(void)theOptions;
}

/************************************************************************
**
** NAME:        DoMove
**
** DESCRIPTION: Apply the move to the position.
**
** INPUTS:      POSITION position : The old position
**              MOVE     move     : The move to apply.
**
** OUTPUTS:     (POSITION) : The position that results after the move.
**
************************************************************************/

POSITION DoMove(POSITION position, MOVE move)
{
	BlankOX* board = PositionToBlankOX(position);
	board[move] = WhoseTurn(board);
	return BlankOXToPosition(board);
}

/************************************************************************
**
** NAME:        GetInitialPosition
**
** DESCRIPTION: Ask the user for an initial position for testing. Store
**              it in the space pointed to by initialPosition;
**
** OUTPUTS:     POSITION initialPosition : The position to fill.
**
************************************************************************/

POSITION GetInitialPosition()
{
	BlankOX* board = (BlankOX *) SafeMalloc(BOARDSIZE * sizeof(BlankOX));
	signed char c;
	int i = 0, xcount = 0, ycount = 0;

	printf("\n\n\t----- Get Initial Position -----\n");
	printf("\n\tPlease input the position to begin with.\n");
	printf("\tNote that it should be in the following format:\n\n");
	printf("O - -\nO - -            <----- EXAMPLE \n- X X\n\n");

	getchar();
	while(i < BOARDSIZE && (c = getchar()) != EOF) {
		if(c == 'x' || c == 'X') {
			board[i++] = x; xcount++;
		} else if(c == 'o' || c == 'O' || c == '0') {
			board[i++] = o; ycount++;
		} else if(c == '-')
			board[i++] = Blank;
		/* else do nothing */
	}

	return(BlankOXToPosition(board));
}

/************************************************************************
**
** NAME:        PrintComputersMove
**
** DESCRIPTION: Nicely format the computers move.
**
** INPUTS:      MOVE   *computersMove : The computer's move.
**              STRING  computersName : The computer's name.
**
************************************************************************/

void PrintComputersMove(computersMove,computersName)
MOVE computersMove;
STRING computersName;
{
	printf("%8s's move              : %2d\n", computersName, computersMove+1);
}

/************************************************************************
**
** NAME:        Primitive
**
** DESCRIPTION: Return the value of a position if it fulfills certain
**              'primitive' constraints. Some examples of this is having
**              three-in-a-row with TicTacToe. TicTacToe has two
**              primitives it can immediately check for, when the board
**              is filled but nobody has one = primitive tie. Three in
**              a row is a primitive lose, because the player who faces
**              this board has just lost. I.e. the player before him
**              created the board and won. Otherwise undecided.
**
** INPUTS:      POSITION position : The position to inspect.
**
** OUTPUTS:     (VALUE) an enum which is oneof: (win,lose,tie,undecided)
**
** CALLS:       BOOLEAN ThreeInARow()
**              BOOLEAN AllFilledIn()
**              PositionToBlankOX()
**
************************************************************************/

VALUE Primitive(POSITION position)
{
	BlankOX* board = PositionToBlankOX(position);
	VALUE value;

	if (ThreeInARow(board, 0, 1, 2) ||
	    ThreeInARow(board, 3, 4, 5) ||
	    ThreeInARow(board, 6, 7, 8) ||
	    ThreeInARow(board, 0, 3, 6) ||
	    ThreeInARow(board, 1, 4, 7) ||
	    ThreeInARow(board, 2, 5, 8) ||
	    ThreeInARow(board, 0, 4, 8) ||
	    ThreeInARow(board, 2, 4, 6))
		value = gStandardGame ? lose : win;
	else if (AllFilledIn(board))
		value = tie;
	else
		value = undecided;

	if (board != NULL)
		SafeFree(board);
	return value;
}

/************************************************************************
**
** NAME:        PrintPosition
**
** DESCRIPTION: Print the position in a pretty format, including the
**              prediction of the game's outcome.
**
** INPUTS:      POSITION position   : The position to pretty print.
**              STRING   playerName : The name of the player.
**              BOOLEAN  usersTurn  : TRUE <==> it's a user's turn.
**
** CALLS:       PositionToBlankOX()
**              GetValueOfPosition()
**              GetPrediction()
**
************************************************************************/

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn)
{
	BlankOX* board = PositionToBlankOX(position);

	printf("\n         ( 1 2 3 )           : %c %c %c (%c's Turn)\n",
	       board[0], board[1], board[2], WhoseTurn(board));
	printf("LEGEND:  ( 4 5 6 )  TOTAL:   : %c %c %c\n",
	       board[3], board[4], board[5]);
	printf("         ( 7 8 9 )           : %c %c %c %s\n\n",
	       board[6], board[7], board[8], GetSEvalPrediction(position,playerName,usersTurn));

	if (board != NULL)
		SafeFree(board);
}

/************************************************************************
**
** NAME:        GenerateMoves
**
** DESCRIPTION: Create a linked list of every move that can be reached
**              from this position. Return a pointer to the head of the
**              linked list.
**
** INPUTS:      POSITION position : The position to branch off of.
**
** OUTPUTS:     (MOVELIST *), a pointer that points to the first item
**              in the linked list of moves that can be generated.
**
** CALLS:       MOVELIST *CreateMovelistNode(MOVE,MOVELIST *)
**
************************************************************************/

MOVELIST* GenerateMoves(POSITION position)
{
	int i;
	MOVELIST* moves = NULL;

	BlankOX* board = PositionToBlankOX(position);

	for (i = BOARDSIZE-1; i >= 0; i--)
		if (board[i] == Blank)
			moves = CreateMovelistNode(i, moves);

	if (board != NULL)
		SafeFree(board);
	return moves;
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
	BlankOX *board, *symmBoard;

	board = PositionToBlankOX(position);
	symmBoard = (BlankOX *) SafeMalloc(BOARDSIZE * sizeof(BlankOX));

	/* Copy from the symmetry matrix */

	for(i = 0; i < BOARDSIZE; i++)
		symmBoard[i] = board[gSymmetryMatrix[symmetry][i]];

	if (board != NULL)
		SafeFree(board);
	return(BlankOXToPosition(symmBoard));
}

/**************************************************/
/**************** SYMMETRY FUN END ****************/
/**************************************************/

/************************************************************************
**
** NAME:        GetAndPrintPlayersMove
**
** DESCRIPTION: This finds out if the player wanted an undo or abort or not.
**              If so, return Undo or Abort and don't change theMove.
**              Otherwise get the new theMove and fill the pointer up.
**
** INPUTS:      POSITION *thePosition : The position the user is at.
**              MOVE *theMove         : The move to fill with user's move.
**              STRING playerName     : The name of the player whose turn it is
**
** OUTPUTS:     USERINPUT             : Oneof( Undo, Abort, Continue )
**
** CALLS:       ValidMove(MOVE, POSITION)
**              BOOLEAN PrintPossibleMoves(POSITION) ...Always True!
**
************************************************************************/

USERINPUT GetAndPrintPlayersMove(POSITION thePosition, MOVE *theMove, STRING playerName)
{
	USERINPUT ret;

	do {
		printf("%8s's move [(u)ndo/1-9] :  ", playerName);

		ret = HandleDefaultTextInput(thePosition, theMove, playerName);
		if(ret != Continue)
			return(ret);

	}
	while (TRUE);
	return(Continue); /* this is never reached, but lint is now happy */
}

/************************************************************************
**
** NAME:        ValidTextInput
**
** DESCRIPTION: Return TRUE iff the string input is of the right 'form'.
**              For example, if the user is allowed to select one slot
**              from the numbers 1-9, and the user chooses 0, it's not
**              valid, but anything from 1-9 IS, regardless if the slot
**              is filled or not. Whether the slot is filled is left up
**              to another routine.
**
** INPUTS:      STRING input : The string input the user typed.
**
** OUTPUTS:     BOOLEAN : TRUE iff the input is a valid text input.
**
************************************************************************/

BOOLEAN ValidTextInput(STRING input)
{
	return(input[0] <= '9' && input[0] >= '1');
}

/************************************************************************
**
** NAME:        ConvertTextInputToMove
**
** DESCRIPTION: Convert the string input to the internal move representation.
**
** INPUTS:      STRING input : The string input the user typed.
**
** OUTPUTS:     MOVE : The move corresponding to the user's input.
**
************************************************************************/

MOVE ConvertTextInputToMove(STRING input)
{
	return((MOVE) input[0] - '1'); /* user input is 1-9, our rep. is 0-8 */
}

/************************************************************************
**
** NAME:        PrintMove
**
** DESCRIPTION: Print the move in a nice format.
**
** INPUTS:      MOVE *theMove         : The move to print.
**
************************************************************************/

void PrintMove(MOVE theMove)
{
	STRING str = MoveToString( theMove );
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

STRING MoveToString (MOVE theMove)
{
	STRING m = (STRING) SafeMalloc( 3 );
	/* The plus 1 is because the user thinks it's 1-9, but MOVE is 0-8 */
	sprintf( m, "%d", theMove + 1);

	return m;
}

BOOLEAN ThreeInARow(BlankOX* board, int a, int b, int c)
{
	return(board[a] == board[b] &&
	       board[b] == board[c] &&
	       board[c] != Blank );
}

BOOLEAN AllFilledIn(BlankOX* board)
{
	BOOLEAN answer = TRUE;
	int i;

	for(i = 0; i < BOARDSIZE; i++)
		answer &= (board[i] == o || board[i] == x);

	return(answer);
}

int NumberOfOptions()
{
	return 4;
}

int getOption()
{
	int option = 0;
	option += (gStandardGame ? 1 : 0);
	option += (gSymmetries ? 2 : 0);
	return option+1;
}

void setOption(int option)
{
	option -= 1;
	gStandardGame = option % 2;
	option -= 2;
	gSymmetries = option % 2;
}

// HASH/UNHASH

char* customUnhash(POSITION position) {
	return (char*)PositionToBlankOX(position);
}

// "Unhash"
BlankOX* PositionToBlankOX(POSITION position)
{
	char* board = (char *) SafeMalloc(BOARDSIZE * sizeof(char)); // make board space
	if (gHashWindowInitialized) { // using hash windows
		TIERPOSITION tierpos; TIER tier;
		gUnhashToTierPosition(position, &tierpos, &tier); // get tierpos
		generic_hash_context_switch(tier); // switch to that tier's context
		return (BlankOX *) generic_hash_unhash(tierpos, board); // unhash in that tier
	} else return (BlankOX *) generic_hash_unhash(position, board);
}

// "Hash"
POSITION BlankOXToPosition(BlankOX* board)
{
	POSITION position;
	if (gHashWindowInitialized) {
		TIER tier = BoardToTier(board); // find this board's tier
		generic_hash_context_switch(tier); // switch to that context
		TIERPOSITION tierpos = generic_hash_hash((char*)board, 1); //unhash
		position = gHashToWindowPosition(tierpos, tier); //gets TIERPOS, find POS
	} else position = generic_hash_hash((char*)board, 1);
	if(board != NULL)
		SafeFree(board);
	return position;
}

BlankOX WhoseTurn(BlankOX* board)
{
	// This turns out to be equivalent to calling WhoseMove
	// for the Tier-specific generic hashes.
	int i, xcount = 0, ocount = 0;

	for(i = 0; i < BOARDSIZE; i++) {
		if(board[i] == x)
			xcount++;
		else if(board[i] == o)
			ocount++;
	}

	if(xcount == ocount)
		return x;
	else return o;
}

// Tier = Pieces left to place. So count the pieces on the
// board and minus it from BOARDSIZE(a.k.a. 9).
TIER BoardToTier(BlankOX* board) {
	int i, piecesOnBoard = 0;
	for (i = 0; i < BOARDSIZE; i++)
		if (board[i] != Blank)
			piecesOnBoard++;
	return BOARDSIZE - piecesOnBoard;
}


//TIER GAMESMAN

void SetupTierStuff() {
	// kSupportsTierGamesman
	kSupportsTierGamesman = TRUE;
	// All function pointers
	gTierChildrenFunPtr                             = &TierChildren;
	gNumberOfTierPositionsFunPtr    = &NumberOfTierPositions;
	gGetInitialTierPositionFunPtr   = &GetInitialTierPosition;
	//gIsLegalFunPtr				= &IsLegal;
	gGenerateUndoMovesToTierFunPtr  = &GenerateUndoMovesToTier;
	gUnDoMoveFunPtr                                 = &UnDoMove;
	gTierToStringFunPtr                             = &TierToString;
	// Tier-Specific Hashes
	int piecesArray[10] = { o, 0, 0, x, 0, 0, Blank, 0, 0, -1 };
	int piecesOnBoard, tier;
	for (tier = 0; tier <= BOARDSIZE; tier++) {
		piecesOnBoard = BOARDSIZE - tier;
		// Os = piecesOnBoard / 2
		piecesArray[1] = piecesArray[2] = piecesOnBoard/2;
		// Xs = piecesOnBoard / 2 (+1 if tier % 2 == 0, else +0)
		piecesArray[4] = piecesArray[5] = piecesOnBoard/2 + (tier%2 == 0 ? 1 : 0);
		// Blanks = tier
		piecesArray[7] = piecesArray[8] = tier;
		// make the hashes
		generic_hash_init(BOARDSIZE, piecesArray, NULL, (tier%2==0 ? 2 : 1));
	}
	// initial tier
	gInitialTier = BOARDSIZE;
	// it's already in the final hash context, so set the position:
	BlankOX* board = (BlankOX *) SafeMalloc(BOARDSIZE * sizeof(BlankOX));
	int i;
	for (i = 0; i < BOARDSIZE; i++)
		board[i] = Blank;
	gInitialTierPosition = BlankOXToPosition(board);
}

// Tier = Pieces left to place. So a tier's child is always
// itself - 1. Except for Tier 0, of course.
TIERLIST* TierChildren(TIER tier) {
	TIERLIST* tierlist = NULL;
	if (tier != 0)
		tierlist = CreateTierlistNode(tier-1, tierlist);
	return tierlist;
}

// Switch to the correct hash and call generic_hash_max_pos.
TIERPOSITION NumberOfTierPositions(TIER tier) {
	generic_hash_context_switch(tier);
	return generic_hash_max_pos();
}

void GetInitialTierPosition(TIER* tier, TIERPOSITION* tierposition) {
	BlankOX* board = (BlankOX *) SafeMalloc(BOARDSIZE * sizeof(BlankOX));
	signed char c;
	int i = 0, xcount = 0, ycount = 0;

	printf("\n\n\t----- Get Initial Position -----\n");
	printf("\n\tPlease input the position to begin with.\n");
	printf("\tNote that it should be in the following format:\n\n");
	printf("O - -\nO - -            <----- EXAMPLE \n- X X\n\n");

	while(i < BOARDSIZE && (c = GetMyChar()) != EOF) {
		if(c == 'x' || c == 'X') {
			board[i++] = x; xcount++;
		} else if(c == 'o' || c == 'O' || c == '0') {
			board[i++] = o; ycount++;
		} else if(c == '-')
			board[i++] = Blank;
		/* else do nothing */
	}

	(*tier) = BoardToTier(board);
	generic_hash_context_switch(*tier);
	(*tierposition) = generic_hash_hash(board,1);
}

// Not used, since we deal with the double-hash problem already.
BOOLEAN IsLegal(POSITION position) {
	(void)position;
	return TRUE;
}

// Basically the OPPOSITE of GenerateMoves: look for the
// pieces that opponent has on the board, and those are the undomoves.
// Also, ALWAYS generate to tier one above it.
UNDOMOVELIST* GenerateUndoMovesToTier(POSITION position, TIER tier) {
	BlankOX* board = PositionToBlankOX(position);
	int i;
	UNDOMOVELIST* undomoves = NULL;
	BlankOX turn = (WhoseTurn(board) == x ? o : x);
	for (i = BOARDSIZE-1; i >= 0; i--) {
		if (board[i] == turn) //It's opposing player's turn
			undomoves = CreateUndoMovelistNode(i, undomoves);
	}
	TIER myTier = BoardToTier(board);
	if (board != NULL)
		SafeFree(board);
	// Check TIER, should be one above current tier
	if (myTier + 1 != tier) {
		FreeUndoMoveList(undomoves); // since it won't be used
		return NULL;
	}
	return undomoves;
}

// UNDOMOVE = Just like a MOVE (0-8) but it tells
// where to TAKE a piece rather than PLACE it.
POSITION UnDoMove(POSITION position, UNDOMOVE undomove) {
	BlankOX* board = PositionToBlankOX(position);
	board[undomove] = Blank;
	return BlankOXToPosition(board);
}

// Tier = Number of pieces left to place.
STRING TierToString(TIER tier) {
	STRING tierStr = (STRING) SafeMalloc(sizeof(char)*15);
	sprintf(tierStr, "%d Pieces Placed", BOARDSIZE-((int)tier));
	return tierStr;
}

POSITION InteractStringToPosition(STRING boardStr) {
	// change boardStr to BlankOX
	BlankOX *board = malloc(sizeof(BlankOX)* BOARDSIZE);
	if (strlen(boardStr) < BOARDSIZE) {
		printf("String to Position for ACHI failed\n");
		return -1;
	}
	int i;
	for (i = 0; i < BOARDSIZE; i++) {
		if (boardStr[i] == 'o') {
			board[i] = o;
		}
		else if (boardStr[i] == 'x') {
			board[i] = x;
		}
		else if (boardStr[i] == ' ') {
			board[i] = Blank;
		}
	}
	return BlankOXToPosition(board);
}

STRING InteractPositionToString(POSITION pos) {
    BlankOX *board = PositionToBlankOX(pos);
    int i;
    char* safe_board = (char*)SafeMalloc((BOARDSIZE + 1) * sizeof(char));
		char* whole_board = NULL;
    for (i = 0; i < BOARDSIZE; i++) {
        if (board[i] == o) {
            safe_board[i] = 'o';
        }
        else if (board[i] == x) {
            safe_board[i] = 'x';
        }
        else if (board[i] == Blank) {
            safe_board[i] = ' ';
        }
    }
    *(safe_board+BOARDSIZE) = '\0';
		whole_board = MakeBoardString(safe_board,
				"tier", TierstringFromPosition(pos),
				"");
		SafeFree(safe_board);
	return whole_board;
}

STRING InteractMoveToString(POSITION pos, MOVE mv) {
	(void)pos;
	return MoveToString(mv);
}
