// $Id: mfoxes.c,v 1.16 2008-02-20 06:13:23 ungu1d3d_s0ul Exp $

/*
 * The above lines will include the name and log of the last person
 * to commit this file to CVS
 */

/************************************************************************
**
** NAME:        mfoxes_new.c
**
** DESCRIPTION: Fox and Geese
**
** AUTHOR:      Sergey Kirshner - University of California at Berkeley
**              Copyright (C) Dan Garcia, 1997. All rights reserved.
**              Copied/pasted/modified to fit the current Gamesman core by Evan Huang.
**
** DATE:        Adoption of code started on 4/7/05
**
** UPDATE HIST: (was blank in the original)
**              4/7/05  [+]Initial adoption of the old module to the current core
**              4/11/05 [-]realized that ComputeC is deprecated, and removed it.
**              4/16/05 [*]Geese go first. That was the right default.
**              2/9/08  numOfOptions returns 2 now (May need more work).
**
**************************************************************************/


/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/

#include "gamesman.h"

/*************************************************************************
**
** Game-specific constants
**
**************************************************************************/

STRING kGameName            = "Foxes and Geese";   /* The name of your game */
STRING kAuthorName          = "Sergey Kirshner";   /* Your name(s) */
STRING kDBName              = "foxes";   /* The name to store the database under */

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

/*
 * Help strings that are pretty self-explanatory
 * Strings than span more than one line should have backslashes (\) at the end of the line.
 */

STRING kHelpGraphicInterface =
        "Not written yet";

STRING kHelpTextInterface    =
        "Move your piece to an adjacent point on the line. If you are\n\
a geese, then you can only move forward, diagonally forward or\n\
sideways. Use the LEGEND to detemine which numbers to choose\n\
to correspond to the location of your piece and the empty\n\
diagonally-adjacent position you wish to move that piece to. \n\
Example: '1 5' will move your piece from square 1 to square 5.\n\
To capture a goose, a fox must be adjacent to the goose and the\n\
goose must have an empty square behind it. The fox captures by\n\
jumping over the goose to the empty space. Once the goose is\n\
captured, it is removed from the board.  The geese cannot\n\
jump over the fox, but it can try to maneuver it into a position\n\
where it cannot move."                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 ;

STRING kHelpOnYourTurn =
        "If you are a fox, you may move along the line to an empty spot\n\
or capture a goose, by jumping over it to and open spot. The fox\n\
may move forwards, backwards, diagonally or to the sides. If you\n\
are a goose, you may only move forward or sideways and you cannot\n\
jump over the fox. "                                                                                                                                                                                                                                                                                        ;

STRING kHelpStandardObjective =
        "If you are the fox, the objective is to eat up all the geese until\n\
they are unable to trap you. If you are a goose, the objective is to\n\
trap the fox so that it is impossible for him to move."                                                                                                                                                       ;

STRING kHelpReverseObjective =
        "";

STRING kHelpTieOccursWhen =
        "A tie occurs when ...";

STRING kHelpExample =
        "";


/*************************************************************************
**
** #defines and structs
**
**************************************************************************/
#define FOXESGEESETRAP 0
#define FOXESGEESENOTRAP 1

#define BADSLOT -2
#define MAXBOARDSIZE 35          /* To keep compiler happy */

#define FOXCOUNT 1
#define GEESECOUNT 4
#define ROWCOUNT 8

#define MAXNUMBERROWS 8

#define FOXTURN 1
#define GOOSETURN 2

#define BLANKPIECE '-'
#define FOXPIECE 'F'
#define GOOSEPIECE 'G'

typedef int SLOT;     /* A slot is the place where a piece moves from or to */

/*************************************************************************
**
** Global Variables
**
*************************************************************************/
int FOXES = FOXCOUNT;                   /* Number of foxes */
int GEESE = GEESECOUNT;                   /* Number of geese */
int TOTALPIECES = FOXCOUNT + GEESECOUNT;         /* Total number of pieces on the board */
int ROWS = ROWCOUNT;
int gGameType = FOXESGEESETRAP;
int Player1Side = FOXTURN;              /*player 1's side in the game*/
int Player2Side = GOOSETURN;            /*player 2's side in the game*/

int WhoGoesFirst = GOOSETURN;             /*by default, geese go first*/
int BOARDSIZE = ROWCOUNT*4;

static BOOLEAN connected[MAXBOARDSIZE][MAXBOARDSIZE];
static int order[MAXBOARDSIZE];

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
extern POSITION         generic_hash_init(int boardsize, int pieces_array[], int (*vcfg_function_ptr)(int* cfg), int player);
extern POSITION         generic_hash_hash(char *board, int player);
extern char            *generic_hash_unhash(POSITION hash_number, char *empty_board);
extern int              generic_hash_turn (POSITION hashed);
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
BOOLEAN                 OkMove(char *theBlankFG, int whosTurn, SLOT fromSlot,SLOT toSlot);
BOOLEAN                 CantMove(POSITION position);
void                    ChangeBoard();
void                    MoveToSlots(MOVE theMove, SLOT *fromSlot, SLOT *toSlot);
MOVE                    SlotsToMove (SLOT fromSlot, SLOT toSlot);
void                    InitializeAdjacency();
void                    InitializeOrder();

STRING MoveToString(MOVE);

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
	/*get the initialboard*/
	char *theBlankFG;
	int numBlanks = BOARDSIZE - FOXES - GEESE, j, k, l;
	int p_array[10] = {FOXPIECE, 1, FOXES, GOOSEPIECE, 1, GEESE, BLANKPIECE, 0, numBlanks, -1};

	InitializeAdjacency();
	InitializeOrder();
	theBlankFG = (char *) SafeMalloc( (BOARDSIZE) *sizeof(char) );
	gNumberOfPositions = generic_hash_init(BOARDSIZE, p_array, NULL, 0);

	/*  PositionToBlankFG(position,theBlankFg,&whosTurn);,)*/

	for (j=0; j<FOXES; j++) {
		theBlankFG[j] = FOXPIECE;
		/*	printf("%d\n", j);*/
	}
	for (k=j; k<(numBlanks+FOXES); k++) {
		theBlankFG[k] = BLANKPIECE;
		/*	printf("%d\n", k);*/
	}
	for (l=k; l<(FOXES+GEESE+numBlanks); l++) {
		theBlankFG[l] = GOOSEPIECE;
		/*	printf("%d\n", l);*/
	}

	gInitialPosition = generic_hash_hash( theBlankFG, WhoGoesFirst );
	SafeFree(theBlankFG);

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

	char theBlankFG[BOARDSIZE];
	int whosTurn, i,j;

	generic_hash_unhash(position,theBlankFG);
	whosTurn = generic_hash_turn(position);

	for(i = 0; i < BOARDSIZE; i++) {   /* enumerate over all FROM slots */
		for(j = 0; j < BOARDSIZE; j++) { /* enumerate over all TO slots */
			if(OkMove(theBlankFG, whosTurn, (SLOT)i, (SLOT)j))
				moves = CreateMovelistNode(SlotsToMove((SLOT)i,(SLOT)j),moves);
		}
	}

	return moves;
}

BOOLEAN OkMove(char *theBlankFG, int whosTurn, SLOT fromSlot, SLOT toSlot)
{

	if( whosTurn == GOOSETURN ) {
		return((theBlankFG[fromSlot] == GOOSEPIECE) &&
		       (theBlankFG[toSlot] == BLANKPIECE) &&
		       (connected[fromSlot][toSlot]) &&
		       (order[toSlot]>order[fromSlot]));
	}
	else {
		return((theBlankFG[fromSlot] == FOXPIECE) &&
		       (theBlankFG[toSlot] == BLANKPIECE) &&
		       (connected[fromSlot][toSlot]));
	};
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
	SLOT fromSlot, toSlot;
	char theBlankFG[BOARDSIZE];
	int whosTurn;

	generic_hash_unhash(position, theBlankFG);
	whosTurn = generic_hash_turn(position);

	MoveToSlots(move, &fromSlot, &toSlot);
	theBlankFG[toSlot] = theBlankFG[fromSlot];
	theBlankFG[fromSlot] = BLANKPIECE;

	return(generic_hash_hash(theBlankFG,(whosTurn == FOXTURN ? GOOSETURN : FOXTURN)));
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
	char theBlankFG[BOARDSIZE];
	int whosTurn;
	int i, foxMax = 0, gooseMax = 0;

	generic_hash_unhash(position,theBlankFG);
	whosTurn = generic_hash_turn(position);

	for ( i=0; i<BOARDSIZE; i++ ) {
		if ( theBlankFG[i] == FOXPIECE) {
			if ( i/4 > foxMax )
				foxMax = i/4;
		} else if ( theBlankFG[i] == GOOSEPIECE ) {
			if ( i/4 > gooseMax )
				gooseMax = i/4;
		}
	}
	if ( foxMax >= gooseMax && whosTurn == GOOSETURN ) {
		return (gStandardGame ? lose : win);
	} else if ( foxMax >= gooseMax && whosTurn == FOXTURN ) {
		return (gStandardGame ? win : lose);
	} else if ( CantMove(position) ) {
		return (gStandardGame ? lose : win);
	} else {
		return (undecided);
	}
}

BOOLEAN CantMove(POSITION position)
{
	MOVELIST *ptr;
	BOOLEAN cantMove;

	ptr = GenerateMoves(position);
	cantMove = (ptr == NULL);
	if (!cantMove) FreeMoveList(ptr);
	return(cantMove);
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
	int i;
	char theBlankFg[BOARDSIZE];
	int whosTurn;

	generic_hash_unhash(position, theBlankFg);
	whosTurn = generic_hash_turn(position);

	printf("\t\tLEGEND:\t\t\tPLAYER %s, a.k.a %s's turn\n\n",
	       playersName,
	       whosTurn == FOXTURN ? "FOX" : "GOOSE");
	for( i=0; i<(ROWS/2); i++ ) {

		printf("         (   ");
		if( 8*i<9 )
			printf(" %d  ", 8*i+1);
		else
			printf("%d  ", 8*i+1);
		if( 8*i<8 )
			printf(" %d  ", 8*i+2);
		else
			printf("%d  ", 8*i+2);
		if( 8*i<7 )
			printf(" %d  ", 8*i+3);
		else
			printf("%d  ", 8*i+3);
		if( 8*i<6 )
			printf(" %d ", 8*i+4);
		else
			printf("%d ", 8*i+4);
		printf(")           :    %c   %c   %c   %c\n",
		       theBlankFg[8*i],
		       theBlankFg[8*i+1],
		       theBlankFg[8*i+2],
		       theBlankFg[8*i+3]);
		printf("         ( ");
		if( 8*i<5 )
			printf(" %d  ", 8*i+5);
		else
			printf("%d  ", 8*i+5);
		if( 8*i<4 )
			printf(" %d  ", 8*i+6);
		else
			printf("%d  ", 8*i+6);
		if( 8*i<3 )
			printf(" %d  ", 8*i+7);
		else
			printf("%d  ", 8*i+7);
		if( 8*i<2 )
			printf(" %d  ", 8*i+8);
		else
			printf("%d  ", 8*i+8);
		printf(" )           :  %c   %c   %c   %c\n",
		       theBlankFg[8*i+4],
		       theBlankFg[8*i+5],
		       theBlankFg[8*i+6],
		       theBlankFg[8*i+7] );
	};
	if( (ROWS%2)!=0 ) {
		i = ROWS-1;
		printf("         (   ");
		if( 4*i<9 )
			printf(" %d  ", 4*i+1);
		else
			printf("%d  ", 4*i+1);
		if( 4*i<8 )
			printf(" %d  ", 4*i+2);
		else
			printf("%d  ", 4*i+2);
		if( 4*i<7 )
			printf(" %d  ", 4*i+3);
		else
			printf("%d  ", 4*i+3);
		if( 4*i<6 )
			printf(" %d ", 4*i+4);
		else
			printf("%d ", 4*i+4);
		printf(")           :    %c   %c   %c   %c\n",
		       theBlankFg[4*i],
		       theBlankFg[4*i+1],
		       theBlankFg[4*i+2],
		       theBlankFg[4*i+3]);
	};
	if (gPrintPredictions && (!gUnsolved))
		printf("\n%s\n\n",GetPrediction(position,playersName,usersTurn));
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
	SLOT fromSlot,toSlot;

	MoveToSlots(computersMove,&fromSlot,&toSlot);
	printf("%8s's move              : %d %d\n", computersName,
	       fromSlot+1,toSlot+1);
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
	STRING m = MoveToString( move );
	printf( "%s", m );
	SafeFree( m );
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
	STRING move = (STRING) SafeMalloc(10);
	SLOT fromSlot, toSlot;

	MoveToSlots(theMove,&fromSlot,&toSlot);
	/* The plus 1 is because the user thinks it's 1-9, but MOVE is 0-8 */
	sprintf( move, "[ %d %d ]", fromSlot + 1, toSlot + 1);

	return move;
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

	for (;; ) {
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
	SLOT fromSlot, toSlot;
	int ret;

	ret = sscanf(input,"%d %d", &fromSlot, &toSlot);
	return(ret == 2 &&
	       fromSlot <= BOARDSIZE && fromSlot >= 1 && toSlot <= BOARDSIZE && toSlot >= 1);
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
	SLOT fromSlot, toSlot;
	int ret;

	ret = sscanf(input,"%d %d", &fromSlot, &toSlot);

	fromSlot--;           /* user input is 1-16, our rep. is 0-15 */
	toSlot--;             /* user input is 1-16, our rep. is 0-15 */

	return(SlotsToMove(fromSlot,toSlot));
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
		printf("\n\t----- Game-specific options for %s -----\n\n", kGameName);

		printf("\tCurrent Initial Position:\n");
		/*    whosTurn = ((gInitialPosition>=POSITION_OFFSET)? g : f);*/
		PrintPosition(gInitialPosition, gPlayerName[kPlayerOneTurn], kHumansTurn);

		printf("\n\tCurrent Number of Foxes: %d\n", FOXES);
		printf("\tCurrent Number of Geese: %d\n\n", GEESE);
		printf("\t1)\tChoose (manually) the initial position\n");
		printf("\t2)\tChange the number of pieces and size of the board (the default position will be used)\n");
		printf("\n\n\tb)\t(B)ack = Return to previous activity.\n");
		printf("\n\nSelect an option: ");

		//scanf("%s", c);

		switch(GetMyChar()) {
		case 'Q': case 'q':
			ExitStageRight();
		case 'H': case 'h':
			HelpMenus();
			break;
		case '1':
			gInitialPosition = GetInitialPosition();
			break;
		case '2':
			ChangeBoard();
			break;
		case 'b': case 'B':
			return;
		default:
			BadMenuChoice();
		}
	} while(TRUE);
}

void ChangeBoard()
{
	printf("\n\n\t----- Change Board -----\n");
	printf("\n\tPlease enter the new number of foxes (currently %d): ",FOXES);
	scanf("%d", &FOXES);
	printf("\n\tPlease enter the new number of geese (currently %d): ", GEESE);
	scanf("%d", &GEESE);
	printf("\n\tPlease enter the new number of rows (currently %d): ", ROWS);
	scanf("%d", &ROWS);

	BOARDSIZE = ROWS*4;

	printf("\n\tPlease choose a side, \"f\" for FOX and \"g\" for GOOSE, to go FIRST (currently %s): ", WhoGoesFirst == FOXTURN ? "FOX" : "GOOSE");
	switch (GetMyChar()) {
	case 'G': case 'g':
		WhoGoesFirst = GOOSETURN;
		break;
	case 'F': case 'f':
		WhoGoesFirst = FOXTURN;
		break;
	default:
		BadMenuChoice();
	}
	InitializeGame();
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
	char theBlankFG[BOARDSIZE];
	/*    int whosTurn;*/

	gInitialPosition = options[4];
	if (options[0]) {
		/* foxes go first */

		generic_hash_unhash(gInitialPosition, theBlankFG);
		/*	whosTurn = generic_hash_turn(gInitialPosition);*/

		gInitialPosition = generic_hash_hash(theBlankFG, FOXTURN);

	}

	TOTALPIECES = options[1];
	FOXES = options[2];
	GEESE = TOTALPIECES - FOXES;
	BOARDSIZE = options[3];
	ROWS = BOARDSIZE/4;
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
	char theBlankFG[BOARDSIZE];
	int whosTurn;
	signed char c;
	int i;

	printf("\n\n\t----- Get Initial Position -----\n");
	printf("\n\tPlease input the position to begin with.\n");
	printf("\tNote that it should be in the following format:\n\n");
	printf(" - - F -\n- - - -             <----- EXAMPLE \n - - - -\nG G G G \n\n");

	i = 0;
	getchar();
	while(i < BOARDSIZE && (c = getchar()) != EOF) {
		if(c == 'f' || c == 'F')
			theBlankFG[i++] = FOXPIECE;
		else if(c == 'g' || c == 'G')
			theBlankFG[i++] = GOOSEPIECE;
		else if(c == '-')
			theBlankFG[i++] = BLANKPIECE;
		else
			; /* do nothing */
	}

	getchar();
	printf("\nNow, whose turn is it? [G/F] : ");
	scanf("%c",&c);
	if(c == 'f' || c == 'F')
		whosTurn = FOXTURN;
	else
		whosTurn = GOOSETURN;

	return(generic_hash_hash(theBlankFG, whosTurn));
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
	return 2;
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
	if(gStandardGame) return 1;
	return 2;
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
	if(option == 1)
		gStandardGame = TRUE;
	else
		gStandardGame = FALSE;
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
** NAME:        MoveToSlots
**
** DESCRIPTION: convert an internal move to that of two slots
**
** INPUTS:      MOVE theMove    : The move input.
**              SLOT *fromSlot  : The slot the piece moves from (output)
**              SLOT *toSlot    : The slot the piece moves to   (output)
**
************************************************************************/

void MoveToSlots(MOVE theMove, SLOT *fromSlot, SLOT *toSlot)
{
	*fromSlot = theMove % BOARDSIZE;
	*toSlot   = theMove / BOARDSIZE;

}

/************************************************************************
**
** NAME:        SlotsToMove
**
** DESCRIPTION: convert two slots (from and to) to an encoded MOVE
**
** INPUT:       SLOT fromSlot   : The slot the piece moves from (0->BOARDSIZE)
**              SLOT toSlot     : The slot the piece moves to   (0->BOARDSIZE)
**
** OUTPUT:      MOVE            : The move corresponding to from->to
**
************************************************************************/

MOVE SlotsToMove (SLOT fromSlot, SLOT toSlot)
{
	return ((MOVE) toSlot*BOARDSIZE + fromSlot);
}


/************************************************************************
**
** NAME:        InitializeAdjacency
**
** DESCRIPTION: Initializes adjacency matrix 'conncted'.
**
** INPUTS:      None
**
** OUTPUTS:     None
**
************************************************************************/

void InitializeAdjacency () {
	int i, j;

	for( i=0; i<MAXBOARDSIZE; i++ )
		for( j=i; j<MAXBOARDSIZE; j++ )
			if( ((j-i)==4) ||
			    ( (j-i)==5 && ( (i % 8)==0 || (i % 8)==1 || (i % 8)==2 ) ) ||
			    ( (j-i)==3 && ( (i % 8)==5 || (i % 8)==6 || (i % 8)==7 ) )) {
				connected[i][j]=1;
				connected[j][i]=1;
			}
			else {
				connected[i][j]=0;
				connected[j][i]=0;
			};

	return;
}

/************************************************************************
**
** NAME:        InitializeOrder
**
** DESCRIPTION: Initializes order of the nodes (array order).
**
** INPUTS:      None
**
** OUTPUTS:     None
**
************************************************************************/

void InitializeOrder () {
	int i;

	for( i=0; i<ROWS; i++ ) {
		order[4*i]=(ROWS-1)-i;
		order[4*i+1]=(ROWS-1)-i;
		order[4*i+2]=(ROWS-1)-i;
		order[4*i+3]=(ROWS-1)-i;
	}

	return;
}

// $Log: not supported by cvs2svn $
// Revision 1.15  2006/12/19 20:00:50  arabani
// Added Memwatch (memory debugging library) to gamesman. Use 'make memdebug' to compile with Memwatch
//
// Revision 1.14  2006/10/17 10:45:20  max817
// HUGE amount of changes to all generic_hash games, so that they call the
// new versions of the functions.
//
// Revision 1.13  2006/03/20 23:56:56  kmowery
//
// Added MoveToString and set gMoveToStringFunPtr, required for Visual Value History.
//
// Revision 1.12  2006/01/11 22:55:03  hevanm
// Hopefully really fixed twobitdb. Changed compiler flags for all platforms to "-Wall -g". Look at those warnings. They are not critical, but they are there. Removed the configure script. From now on please remember to (re)grenerate it yourself if/when configure.ac changes, by simply running autoconf.
//
// Revision 1.11  2006/01/03 00:19:35  hevanm
// Added types.h. Cleaned stuff up a little. Bye bye gDatabase.
//
// Revision 1.10  2005/12/27 10:57:50  hevanm
// almost eliminated the existance of gDatabase in all files, with some declarations commented earlier that need to be hunt down and deleted from the source file.
//
// Revision 1.9  2005/11/10 20:58:51  hevanm
// Renaming mfoxes_new to mfoxes. You need to regenerate the makefiles with config.status to adopt the changes.
//
// Revision 1.3  2005/04/27 22:47:48  ciokita
// update helpstrings
//
// Revision 1.2  2005/04/17 00:45:20  hevanm
// Cleaned up unused functions and let the geese go first by default. This completes all major work on bringing F&G to the current core.
//
// Revision 1.1  2005/04/11 18:01:10  hevanm
// A version of mfoxes that uses the current core API. Everything except the GUI works, with the exception of a few strange bugs here and there...
//
// Revision 1.3  2005/03/10 02:06:47  ogren
// Capitalized CVS keywords, moved Log to the bottom of the file - Elmer
//

POSITION StringToPosition(char* board) {
	int whosTurn = board[0] == 'F' ? FOXTURN : GOOSETURN;
	return generic_hash_hash(&board[1], whosTurn);
}

char* PositionToString(POSITION pos) {
	char board[BOARDSIZE];
	generic_hash_unhash(pos, board);
	int whosTurn = generic_hash_turn(pos);

	char *ret = SafeMalloc(sizeof(char) * (1 + BOARDSIZE + 1));

	ret[0] = whosTurn == FOXTURN ? 'F' : 'G';
	memcpy(&ret[1], board, sizeof(char) * BOARDSIZE);
	ret[1 + BOARDSIZE] = '\0';

	return ret;
}

char * PositionToEndData(POSITION pos) {
	return NULL;
}
