// $Id: mquarto.c,v 1.75 2008-03-11 02:40:54 ethanr Exp $


/*
 * The above lines will include the name and log of the last person
 * to commit this file to CVS
 */

/************************************************************************
**
** NAME:        mquarto.c
**
** DESCRIPTION: Quarto
**
** AUTHORS:      Yanpei CHEN  <ychen@berkeley.edu>
**               Amy HSUEH    <amyhsueh@berkeley.edu>
**               Mario TANEV  <mtanev@berkeley.edu>
**
** DATE:        Began Jan 2005;
**
** UPDATE HIST: RECORD CHANGES YOU HAVE MADE SO THAT TEAMMATES KNOW
**
** 30 Jan 2005 Yanpei: the data structure framework added, PrintPosition() coded
** 01 Feb 2005 Yanpei: PrintPosition() wrong, must be corrected later
** 08 Feb 2005 Amy:    corrected my name, changed kTieIsPossible to TRUE.
** 11 Feb 2005 Yanpei: added hashQuarto(), hashQuartoHelper(), setFactorialTable(),
**                     permutation(), combination(), setOffsetTable();
**                     killed incorrect comments and code for PrintPosition().
** 14 Feb 2005 Yanpei: one line fix to hashQuarto()
**                     changed static declarations of factorialTable and offsetTable
** 27 Feb 2005 Yanpei: more changes to hash() etc to enable new non-redundant
**                     implementation. unhash() in the works. need printPosition()
**                     before code can be tested independent of core.
** 27 Feb 2005 Yanpei: more changes to hash() and unhash(), both yet to be ready.
** 28 Feb 2005 Amy:    added gGameSpecificTclInit, as suggested in email
** 05 Mar 2005 Mario:  fixed some ungodly compilation errors and warnings to get this to compile, diff for details
** 06 Mar 2005 Mario:  corrected incorrect behavior of combination() and permutation()
**                     added non-memoizing factorial for debugging purposes (#define DEBUG to enable), currently enabled
**                     corrected defines as ^ is not a power operator in C, but XOR
**                     changed QTBOard field sizes to short as it should be sufficient
**                     added function pointers hash and unhash, to be set to the default implementations
**                     coded inefficient bitpacking hasher and testing function so we can proceed with further coding
**                     added print_board and boards_equal to print and compare board contents (for internal use)
**                     coded PrintPosition and auxilliary functions
**                     buggy implementation of ValidTextInput
**                     made PrintPosition work correctly with >2 dimension boards, try setting GAMEDIMENSION to 3 or 4
** 07 Mar 2005 Mario:  added DoMove, but it doesn't work yet (maybe I am missing a step here, what is ValidMove? )
**                     feel free to modify
** 08 Mar 2005 Mario:  added CreateMove(), GetMovePiece(), GetMoveSlot() abstractions as I realized I
**                     after I realized I had repetitions of code
**                     seemingly complete () coded, whoever is to code the other MOVE related functions
**                     should look at the implementation of PrintMove
**                     started using define constants for things like index of hand in slot array (HAND)
**                     and first and last board indices in slot array (FIRSTSLOT, LASTSLOT)
**                     GenerateMoves() and DoMove() fully coded.
**                     YOU CAN NOW MOVE AROUND BOARD. Try with ws:H then ws:0 and so on.
**                     There's a bug in that players are switched by gamesman, even though a player may get 2 moves
**                     One for placing the piece from the hand to the board, and the other into oponent's hand
**                     Not sure of how to avoid it,
**                     but I dislike stacking two moves into one as parsing would get even uglier.
**                     HOT & SPICY :There is a scary bug somewhere, a
**                     apparently with a dangling reference to an overwritten stack
**                     It might be in my code or the core code. If anyone dares to solve it,
**                     please look at GetAndPrintPlayersMove and remove the noted comment and run. YIKES!
** 08 Mar 2005 Yanpei: added Primitive(), introduced global constant EMPTYSLOT
**                     to replace NUMPIECES to encode empty slots in QTBOARD->slots[];
** 08 Mar 2005 Yanpei: EMPTYSLOT changed to 0 to accomodate for present hash()/unhash();
**                     QTBOARD invariants temporarily violated to make things work;
**                     Primitive() changed to reflect this; must change back later.
** 09 Mar 2005 Mario:  Updated code to use EMPTYSLOT, updated EMPTYSLOT to be NUMPIECES instead of 0. Seems to work.
** 09 Mar 2005 Amy:    added move format in getandPrintPlayersMove(), printComputerMove() coded.
** 10 Mar 2005 Mario:  corrected problem after switch to EMPTYSLOT, made game kPartizan
**                     For some reason the second move is always a win, maybe Primitive needs to be updated
**                     introducing some indirection for manipulating the board, still deciding what
** 11 Mar 2005 Yanpei: EMPTYSLOT issue seems to be over. Primitive() strange behavior caused by toggling
**                     of board->usersTurn upon each move. To deal with this we must have place piece and
**                     select next piece combined into one move as in [square]:[nextPiece].
**                     Else we need to have [nextPiece]:H moves not toggle board->usersTurn.
**                     See new debugging printf's.
**                     Kind of got the full version of hash figured out. If no bugs, hopefully working tomorrow.
** 14 Mar 2005 Yanpei: HOORAY!!!!! The full blown version of hash and unhash works for GAMEDIMENSION = 2, 3!!!!
**                     Takes a split second to test all 317 positions for GAMEDIMENSION = 2 .... ok.
**                     Takes 15 min approx to test all 8419329 positions for GAMEDIMENSION = 3 .... scary!
*                     Needs 64 bit machine to test for GAMEDIMENSION = 4.
**                     Try to understand the full blown hash/unhash if you are looking for something to do.
**                     I barely understand it myself ... yes it is that ugly and complicated.
**                     Also added some function points to accomodate multiple implementations.
** 22 Mar 2005 Mario:  Switched to marioInitialize to showcase (seemingly) hashing/unhashing error
**                     Yanpei, please review: compile and run, and watch error output.
**                     Reverted to yanpeiInitialize
** 26 Mar 2005 Yanpei: Some structural changes to allow for variable GAMEDIMENSION. Effect on
**                     existing code should be minimal. Use MallocBoard() and FreeBoard() now
**                     for memory management with boards. Use yanpeiInitializaGame().
** 26 Mar 2005 Yanpei: getCanonical() coded and tested. Very straight forward in fact.
**                     Combined Mario's and Prof Garcia's ideas.
** 26 Mar 2005 Yanpei: Some data of interest:
**                     GAMEDIMENSION = 2: 317 positions, 17 canonicals
**                     GAMEDIMENSION = 3: 8419329 positions,
** 27 Mar 2005 Yanpei: Tried counting total canonical positions for GAMEDIMENSION = 3
**                     not enough memory.
** 29 Mar 2005 Yanpei: One line fix to logical error in Primitive().
** 03 Apr 2005 Mario:  Modified DoMove, GenerateMoves to perform 2-moves
**                     Switched to non-memoizing factorial as memoizing produces arithmetic exceptions
**                     I still however have hashing errors
**                     Example:
**                     [| 3 | X  X  X  X ] pieces = 1; squares = 0; turn = 1
**                     hashes to
**                     [| X | 0  2  3  1 ] pieces = 4; squares = 4; turn = 0
**                     Input handling is not modified yet, but in theory it should work as is
**                     I did update : to . to remind us of the change
** 06 Apr 2005 Mario:  removed packhash(), packunhash(), marioInitializeGame()
**                     When game is started with w, p it is initialized a second time
**                     Thus changed the spot where the is set variables are set - yanpeiInitialize now
**                     Changed print layout
**                     Changed GAMEDIMENSION to 3, game is "playable" now
** 07 Apr 2005 Mario:  Removed PrintBoard(), TestHash()
**                     Made POSITION 64-bits so it works in 4 dimensions (in gamesman.h )
**                     Updated printf format strings for POSITION to POSITION_FORMAT
**                     Changed Primitive() to declare tie if all pieces are on board and no win on board
**                     Previous behavior was if board was full, which doesn't work well in 3 dimensions
**                     Stopped using fflush() for inputs (game-specific menu) as its behavior is undefined
**                     Game is now solvable in 2 and 3D and doesn't have enough mem for 4D solving
** 21 Apr 2005 Yanpei: Should not peek under the hood and set gSymmetries. One line fix in
**                     yanpeiInitializeGame()
** 25 Apr 2005 Mario:  Added GPS support. Works only if symmetries are on.
**                     I don't know why, but in 3 dimensions if symmetries are OFF, I get errors even in non-GPS.
** 08 Sep 2005 Yanpei: What is going on? Quarto has error now!!!! Mario what did you change with GPS?
**                     kGameName changed to "Quarto" at Dan's request.
** 09 Sep 2005 Amy:    Changed the way moves are printed/inputted. Also support for 1-dim quarto added. yay! Format for moves is now
**                     [slot to place piece],[new piece to select]
** 18 Sep 2005 Yanpei: Bug in setOffset() fixed. Now solves for 3-D as well.
** 10 Oct 2005 Amy:    fixed weird bug in 2nd move where generatemoves() wants to put moves into HAND slot when it is already occupied.
** 26 Oct 2005 Yanpei: symmetries VERY inefficient right now. MUST CHANGE EVENTUALLY.
**
** 10 Oct 2005 Amy:    fixed weird bug in 2nd move where generatemoves() wants to put moves into HAND slot when it is already occupied.
** 13 Nov 2005 Mario:  Canonicals should now truly be canonicals, added normalize function to normalize pieces after group actions
** 13 Nov 2005 Mario:  Avoid memory leaks by freeing boards when necessary
** 20 Nov 2005 Amy:    changed piece representation to use {0,1}. Added help strings.
** 16 Jan 2006 Yanpei: Coded generic canonicals for n-dimensional game based on
**                     generalized rotation of n-dimensional pieces hypercube.
**                     In testing phase. Have disagreements w/ mario's cannonicals.
** 28 Jan 2006 Yanpei: Canonicals fully debugged. Mario and Yanpei's canonicals
**                     cross checked and mutually agrees. Algorithms timed and
**                     compared. Mario's is MUCH MUCH faster.
**
** 21 Aug 2006 dmchan: changed to GetMyInt();
**
** 23 Apr 2007 Benno: Finishe my teirification work. I feel like i don't understand the system enough to debug it. Gotta ask Max some questions i guess.
** 02 Feb 2008 EthanR+AlanW: implemented gStandardGame checking into Primitivies.
**                           Added bounds checking for options.
**                           Implemented get, set, and return options
** 10 Mar 2008 EthanR+AlanW: Further refined options
** 04 Apr 2010 ide: Updated function declarations for gcc 4.4
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

#define DEBUG 1

/***************** ********************************************************
**
** Game-specific constants
**
**************************************************************************/

STRING kGameName            = "Quarto";   /* The name of your game */
STRING kAuthorName          = "Yanpei CHEN, Amy HSUEH, Mario TANEV, Benno";   /* Your name(s) */
STRING kDBName              = "quarto";   /* The name to store the database under */

BOOLEAN kPartizan            = TRUE;   /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN kGameSpecificMenu    = TRUE;   /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN kTieIsPossible       = TRUE;   /* TRUE if a tie is possible. FALSE if it is impossible.*/
BOOLEAN kLoopy               = FALSE;   /* TRUE if the game tree will have cycles (a rearranger style game). FALSE if it does not.*/

BOOLEAN kDebugMenu           = FALSE;   /* TRUE only when debugging. FALSE when on release. */
BOOLEAN kDebugDetermineValue = FALSE;   /* TRUE only when debugging. FALSE when on release. */

POSITION gNumberOfPositions   =  0; /* The number of total possible positions | If you are using our hash, this is given by the hash_init() function*/
POSITION gInitialPosition     =  0; /* The initial hashed position for your starting board */
POSITION kBadPosition         = -1; /* A position that will never be used */

/*
 * Help strings that are pretty self-explanatory
 * Strings than span more than one line should have backslashes (\) at the end of the line.
 */

STRING kHelpGraphicInterface =
        "Not written yet";

STRING kHelpTextInterface    =
        "If it is the first move, place a piece into the 'hand' slot \n\
by looking up which piece you want and entering H,[piece].\n\
[piece] is a string such as 0100 where each character specifies a\n\
characteristic of the piece. On a four-dimensional game, a \n\
piece will have 4 characters, etc. For example, the pieces 0000 and\n\
1100 have the last two characteristics in common (they are both 0)\n\n\
Example of a first move: H,0100\n\n\
If there is already a piece in the hand slot, place the piece in the \n\
hand slot onto an empty position on the board by looking up the character\n\
that represents the slot in the legend, then choose another piece to \n\
put in the hand slot just like the first move:\n\n\
Example: 2,0100 (places the piece in the hand splot into slot labeled 2, \n\
then place piece '0100' into the hand slot"                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               ;

STRING kHelpOnYourTurn =
        "If it is the first move, place a piece into the 'hand' slot \n\
If there is already a piece in the hand slot, place the piece in the\n\
slot into an empty position on the board and then choose another piece\n\
to place into the now-empty 'hand' slot."                                                                                                                                                                                                                           ;

STRING kHelpStandardObjective =
        "Be the first player to place a piece that finishes off a row of \n\
four pieces (or less if the game dimension is less than four) that \n\
have something in common."                                                                                                                                                    ;

STRING kHelpReverseObjective =
        "";

STRING kHelpTieOccursWhen =
        "A tie occurs when all the available pieces have been placed and the\n\
'hand' slot is also empty, but there are no rows of four pieces (or \n\
less if the game dimension is less than four) that have something in \n\
common.\n"                                                                                                                                                                                                                                 ;

STRING kHelpExample =
        "        +--+--+--+                +--+--+--+\n\
LEGEND: | H| 0| 1|         BOARD: |  |  |  |\n\
        +--+--+--+                +--+--+--+\n\
           | 2| 3|                   |  |  |\n\
           +--+--+                   +--+--+\n\
Available pieces: 00, 10, 01, 11\n\
Computer's move was: H,01\n\
        +--+--+--+                +--+--+--+\n\
LEGEND: | H| 0| 1|         BOARD: |01|  |  |\n\
        +--+--+--+                +--+--+--+\n\
           | 2| 3|                   |  |  |\n\
           +--+--+                   +--+--+\n\
Available pieces: 00, 10, 11\n\
Player's move [(undo)/[0-3],[(0,1)(0,1)]]: 0,00\n\
is valid\n\
        +--+--+--+                +--+--+--+\n\
LEGEND: | H| 0| 1|         BOARD: |00|01|  |\n\
        +--+--+--+                +--+--+--+\n\
           | 2| 3|                   |  |  |\n\
           +--+--+                   +--+--+\n\
Available pieces: 10, 11\n\
Computer's move was: 2,11\n\
        +--+--+--+                +--+--+--+\n\
LEGEND: | H| 0| 1|         BOARD: |11|01|  |\n\
        +--+--+--+                +--+--+--+\n\
           | 2| 3|                   |00|  |\n\
           +--+--+                   +--+--+\n\
Available pieces: 10\n\
Computer (player one) Wins!\n"                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          ;


/*************************************************************************
**
** #defines and structs
**
**************************************************************************/

/* Basic error codes */
#define ERROR   1
#define SUCCESS 0

#define newline() printf( "\n" )

/* Squares x */
#define square(x) ((x)*(x))

/* Creates sequence n least significant 1 bits, preceeded by 0 bits */
#define maskseq(n) ~(~0<<(n))

#define get_bit(x, n)                           \
	(x >> n) & 1

#define set_bit(x, n, y)                        \
	x &= ~(1 << n);                               \
	x |= (y & 1) << n

int GAMEDIMENSION = 3;

int BOARDSIZE;
int NUMPIECES;

int EMPTYSLOT;

int FIRSTSLOT;
int LASTSLOT;

int FACTORIALMAX;

int HAND = 0;

typedef struct board_item {

	short *slots;         // to record the 0 to NUMPIECES-1 pieces contained in each slot
	// slots[1-16] = board squares, slots[0] = next piece
	// 0 to NUMPIECES-1 encode the pieces, EMPTYSLOT encodes an empty slot
	short squaresOccupied; // number of squares occupied
	short piecesInPlay;   // number of pieces in play
	BOOLEAN usersTurn;    // whose turn it is

} QTBOARD;

typedef QTBOARD* QTBPtr;

/* Letter codes for the different piece states */
//char states[][2]={{'w', 'B'}, {'s', 'T'}, {'h', 'S'}, {'r', 'E'}};
char states[][2]={{'0', '1'}, {'0', '1'}, {'0', '1'}, {'0', '1'}};

/* ASCII Hex */
char hex_ascii[] = { 'H', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'y', 'Z' };

/* for generic cannonicals */
int **trivialTable=NULL;
int **lookupTable=NULL;
int twoPowers[] = {1,2,4,8,16,32,64,128,256};

/*************************************************************************
**
** Global Variables
**
*************************************************************************/

BOOLEAN factorialTableSet;
POSITION *factorialTable;
BOOLEAN offsetTableSet;
POSITION *offsetTable;

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
void*                   gGameSpecificTclInit = NULL;  /* newly added to tempalte */

/* External */
QTBPtr                  MallocBoard();
void                    FreeBoard(QTBPtr b);
void                    setOffsetTable();
POSITION                combination(int n, int r);
POSITION                permutation(int n, int r);

/* Implementations of hash/unhash */
POSITION                hashUnsymQuarto(QTBPtr b);
QTBPtr                  unhashUnsymQuarto(POSITION p);
/* Implementations of InitializeGame */
void                    yanpeiInitializeGame();
/* Implementations of PrintPosition */
void                    marioPrintPos(POSITION position, STRING playersName, BOOLEAN usersTurn );
void                    yanpeiPrintSlots(POSITION position, STRING playersName, BOOLEAN usersTurn );
/* Implementations of factorial */
POSITION                factorialMem(int n);
POSITION                factorialNoMem(int n);
/* Implementations of getCanonical */
POSITION                yanpeiGetCanonical(POSITION p);
POSITION                marioGetCanonical(POSITION position);

/* Ben's Extra Special TierHashes */
POSITION TierHash(QTBPtr);
QTBPtr TierUnhash(POSITION);

/* Since we may switch implementations, here are function pointers to be set in choosing implementation */
POSITION (*hash)( QTBPtr ) = &TierHash;               //&hashUnsymQuarto;
QTBPtr (*unhash)( POSITION ) = &TierUnhash;                 //&unhashUnsymQuarto;
void (*initGame)( ) = &yanpeiInitializeGame;
void (*printPos)(POSITION position, STRING playersName, BOOLEAN usersTurn ) = &marioPrintPos;
//void                    (*printPos)(POSITION position, STRING playersName, BOOLEAN usersTurn ) = &yanpeiPrintSlots;
POSITION (*factorial)(int n) = &factorialMem;
POSITION (*getCanonical)(POSITION p) = &marioGetCanonical;

/* support functions */
POSITION                hashUnsymQuartoHelper(QTBPtr b, int baseSlot);
void                    unhashUnsymQuartoHelper(POSITION p, int baseSlot, QTBPtr toReturn);

BOOLEAN                 searchPrimitive(short *);

POSITION                rotatePosition90(POSITION p);
POSITION                reflectPosition(POSITION p);
POSITION                maskBoard(POSITION p, short mask);

int                     **getLookupTable();
int                     **getLookupTableHelper(int d, int **prevTable);
void                    printLookupTable(int **lookupTable);

void                    yanpeiTestOffset();
void                    yanpeiTestHash();
void                    yanpeiTestCanonicalSupport();
void                    yanpeiTestCanonical();

void                    FreePosList(POSITIONLIST *l);
BOOLEAN                 SearchPosList(POSITIONLIST *l, POSITION p);

BOOLEAN                 boards_equal ( QTBPtr, QTBPtr );
void                    copy_board ( QTBPtr, QTBPtr );
void                    print_board( QTBPtr );
QTBPtr                  TestHash( QTBPtr, int );

MOVE                    CreateMove( MOVE slot, MOVE piece );
MOVE                    GetMovePiece( MOVE move );
MOVE                    GetMoveSlot( MOVE move );
unsigned short          GetHandPiece( QTBPtr );
void                    SetHandPiece( QTBPtr, unsigned short );
unsigned short          GetBoardPiece( QTBPtr, unsigned short );

void PrintCell( void *cell, char (*CellContent)( short, void * ) );
void PrintBoard( void *cells, size_t content_size, char *heading, char (*CellContent)( short, void * ) );
char LegendCoordinate( short pad, void *p_coordinate );
char PieceTrait( short trait, void *p_piece );
char BlankCell( short ignored, void *p_ignored );
char BorderCell( short ignored, void* p_ignored );
void PrintHorizontalBorder( char fill, char border, char *startmark, char *endmark );
void PrintRange( void *cells, size_t content_size, int offset, int size, char (*CellContent)( short, void * ), char border, char *endmark );
char readchar( );
void UndoMove(MOVE move);


QTBPtr GPSBoard;


/** Benjamin Sussman Jumps In!!
**		Here follows Ben's attempt at the Teirification of Quarto!
**		TeirGamesman function and variable declarations:
**/
void SetupTierStuff();
TIERLIST* TierChildren(TIER);
TIERPOSITION NumberOfTierPositions(TIER);
void GetInitialTierPosition(TIER*, TIERPOSITION*);
BOOLEAN IsLegal(POSITION);
//UNDOMOVELIST* GenerateUndoMovesToTier(POSITION, TIER); Outdated?
STRING TierToString(TIER);
void setPiecesAndSquares(QTBPtr);
char * consCharArrayFromBoard(QTBPtr);
//POSITION UnDoMove(POSITION, UNDOMOVE); Outdated?



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
	initGame();

	SetupTierStuff();

}

// Yanpei's implementation, to be used with the full blown hash/unhash
// Rips off some code from Mario's implementation
void yanpeiInitializeGame() {

	QTBPtr board;
	short slot;
	int i;

	if (lookupTable) SafeFree(lookupTable);
	/* initializing globals */
	factorialTableSet = FALSE; // Mario added to work around double calling of InitializeGame
	offsetTableSet = FALSE; // Mario added to work around double calling of InitializeGame
	BOARDSIZE = square(GAMEDIMENSION);
	NUMPIECES = (1 << GAMEDIMENSION);
	EMPTYSLOT = NUMPIECES; // generic cannonical depend on this, do not change lightly
	FIRSTSLOT = 1;
	LASTSLOT = BOARDSIZE;
	if (BOARDSIZE<NUMPIECES)
		FACTORIALMAX = (NUMPIECES+1);
	else
		FACTORIALMAX = (BOARDSIZE+1);
	//factorialTable = (POSITION *) SafeMalloc(FACTORIALMAX*(sizeof(POSITION)));
	factorialTable = (POSITION *) malloc( FACTORIALMAX * (sizeof( POSITION ) ) );
	offsetTable = (POSITION *) SafeMalloc((NUMPIECES+2)*(sizeof(POSITION)));

	if(!offsetTableSet) setOffsetTable();
	board = MallocBoard();

	/* Initialize all fields to 0 */
	board->squaresOccupied = 0;
	board->piecesInPlay = 0;
	board->usersTurn = FALSE;

	/* Initialize all slots to EMPTYSLOT */
	for(slot=0; slot<BOARDSIZE+1; slot++) {
		board->slots[slot] = EMPTYSLOT;
	}

	/* Set initial position to empty board */
	gCanonicalPosition = getCanonical;
	gInitialPosition = hash(board);
	gNumberOfPositions = offsetTable[NUMPIECES+1] + 1;

	gMaxRemoteness = NUMPIECES + 1;
	gMinRemoteness = GAMEDIMENSION + 1;

	GPSBoard = board;
	gUndoMove = &UndoMove;

	/*
	   printf("\n");
	   printf("gInitialPosition = " POSITION_FORMAT "\n",gInitialPosition);
	   printf("gNumberOfPositions = " POSITION_FORMAT "\n",gNumberOfPositions);
	 */

	if (!trivialTable) {
		trivialTable = (int**)SafeMalloc(8*sizeof(int*));
		for (i=0; i<8; i++) {
			trivialTable[i] = (int*)SafeMalloc(4*sizeof(int));
		}
		trivialTable[0][0]=0;
		trivialTable[0][1]=1;
		trivialTable[0][2]=2;
		trivialTable[0][3]=3;
		trivialTable[1][0]=0;
		trivialTable[1][1]=2;
		trivialTable[1][2]=1;
		trivialTable[1][3]=3;
		trivialTable[2][0]=2;
		trivialTable[2][1]=0;
		trivialTable[2][2]=3;
		trivialTable[2][3]=1;
		trivialTable[3][0]=2;
		trivialTable[3][1]=3;
		trivialTable[3][2]=0;
		trivialTable[3][3]=1;
		trivialTable[4][0]=3;
		trivialTable[4][1]=2;
		trivialTable[4][2]=1;
		trivialTable[4][3]=0;
		trivialTable[5][0]=3;
		trivialTable[5][1]=1;
		trivialTable[5][2]=2;
		trivialTable[5][3]=0;
		trivialTable[6][0]=1;
		trivialTable[6][1]=3;
		trivialTable[6][2]=0;
		trivialTable[6][3]=2;
		trivialTable[7][0]=1;
		trivialTable[7][1]=0;
		trivialTable[7][2]=3;
		trivialTable[7][3]=2;
	}

	lookupTable = getLookupTable();

	/* calls to test functions */
	//yanpeiTestOffset();
	yanpeiTestHash();
	//yanpeiTestCanonicalSupport();
	//yanpeiTestCanonical();
	//printLookupTable(lookupTable);

	/* Cannonical cross check
	   for (i=0; i<20; i++) {
	   test = rand()*gNumberOfPositions/RAND_MAX;
	   printf(POSITION_FORMAT " -> " POSITION_FORMAT " " POSITION_FORMAT "\n",
	         test,
	         result = yanpeiGetCanonical(test),
	         marioGetCanonical(test));
	   //printPos(test,"",TRUE);
	   //printPos(result,"",TRUE);
	   }
	 */

	/* Cannonical timing test
	   for (i=0; i<1000; i++) {
	   test = rand()*gNumberOfPositions/RAND_MAX;
	   printf("%4d: " POSITION_FORMAT " -> " POSITION_FORMAT "\n",
	         i,
	         test,
	         result = marioGetCanonical(test));
	   }
	 */

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
int FlagAvailablePieces( QTBPtr board, BOOLEAN pieces[] ) {

	int slot;
	int available = NUMPIECES;

	/* Initialize array of pieces */
	memset( pieces, TRUE, sizeof( *pieces ) * NUMPIECES );

	/* For each slot on board */
	for( slot = 0; slot < BOARDSIZE + 1; slot++ ) {

		/* If slot is not empty */
		if( board->slots[slot] != EMPTYSLOT ) {

			/* Mark piece unavailable */
			pieces[board->slots[slot]] = FALSE;
			available--;

		}

	}

	return available;

}

MOVELIST *GenerateMoves (POSITION position)
{
	//if (position == 181801) {
	//printf("we got here: %llu\n", position);
	//PrintPosition(position, "", TRUE);
	//}

	QTBPtr board;
	MOVELIST *moves       = NULL;
	MOVE slot;
	MOVE piece;

	/* Use GPS board if GPS solving, otherwise use unhashed position */
	board = gUseGPS ? GPSBoard : unhash( position );

	/* If there are no pieces on the board, the only valid moves are the ones placing a piece into the hand */
	/* These are special moves, as their board position indicates HAND, and are only valid for a first move */
	/* MAKE SURE HAND IS NOT OCCUPIED */
	if( board->squaresOccupied == 0 && board->slots[HAND] == EMPTYSLOT) {

		/* For every piece possible (since board is initial, every piece is allowed into the game */
		for( piece = 0; piece < NUMPIECES; piece++ ) {

			/* Generate a move that places the piece into the hand. The slot = HAND is indicative of initial move */
			moves     = CreateMovelistNode( CreateMove( HAND, piece ), moves );

		}

	} else {

		BOOLEAN available_pieces[NUMPIECES];
		MOVE piece;
		int available;

		available = FlagAvailablePieces( board, available_pieces );

		/* For each slot on board */
		for( slot = FIRSTSLOT; slot <= LASTSLOT; slot++ ) {

			/* If slot is empty */
			if ( board->slots[slot] == EMPTYSLOT ) {

				if( available == 0 ) {

					moves = CreateMovelistNode( CreateMove( slot, board->slots[HAND] ), moves );

				} else {

					/* For each piece */
					for( piece = 0; piece < NUMPIECES; piece++ ) {

						/* If piece is available */
						if( available_pieces[piece] != FALSE ) {

							/* Add move which moves item from hand into slot, and piece into hand */
							moves     = CreateMovelistNode( CreateMove( slot, piece ), moves );

						}

					}

				}

			}

		}

	}

	/* Deallocate board if not using GPS */
	if (!gUseGPS) {
		FreeBoard(board);
	}

//  if (position == 181801) {
//MOVELIST* ptr;
//for (ptr = moves; ptr != NULL; ptr = ptr->next) {
//    POSITION child = DoMove(position, ptr->move);
//    printf("child: %llu\n", child);
//    PrintPosition(child, "", TRUE);
//}
//  }

	/* Return list of valid moves */
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

	QTBPtr board;
	int piece, slot;
	POSITION newposition;

	/* Use GPS board if GPS solving, otherwise use unhashed position */
	board = gUseGPS ? GPSBoard : unhash( position );

	/* Determine slot on board piece from hand is to go into */
	slot = GetMoveSlot( move );

	/* Determine which piece is to go into hand */
	piece = GetMovePiece( move );

	/* Increment number of pieces */
	board->piecesInPlay += ( piece == GetHandPiece( board ) ) ? 0 : 1;

	/* Place hand piece into indicated slot */
	board->slots[slot] = GetHandPiece( board );

	/* Place indicated piece into hand */
	SetHandPiece( board, ( piece == board->slots[HAND] ) ? EMPTYSLOT : piece );

	/* If indicated slot is not hand, also increment number of squares */
	board->squaresOccupied += ( slot == HAND ) ? 0 : 1;

	newposition = hash(board);

	/* Deallocate board if not using GPS */
	if (!gUseGPS) {
		FreeBoard(board);
	}

	/* Return hashed board */
	return newposition;

}

void UndoMove( MOVE move )
{

	int piece, slot;
	QTBPtr board = GPSBoard;

	/* Determine slot on board piece from hand is to go into */
	slot = GetMoveSlot( move );

	/* Determine which piece is to go into hand */
	piece = GetMovePiece( move );

	/* Decrement number of pieces */
	board->piecesInPlay -= ( GetHandPiece( board ) == EMPTYSLOT ) ? 0 : 1;

	/* Move piece from board into hand */
	SetHandPiece( board, board->slots[slot] );

	/* Empty indicated slot */
	board->slots[slot] = EMPTYSLOT;

	/* If indicated slot is not hand, also decrement number of squares */
	board->squaresOccupied -= ( slot == HAND ) ? 0 : 1;

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
	QTBPtr b;
	short *rowColDiag = (short *) SafeMalloc(GAMEDIMENSION*sizeof(short));
	BOOLEAN primitiveFound = FALSE;
	BOOLEAN emptyFound = FALSE;
	int i,j;
	VALUE toReturn;

	/* Use GPS board if GPS solving, otherwise use unhashed position */
	b = gUseGPS ? GPSBoard : unhash( position );

	// print debugging stuff
	/*
	   printf("**** Primitive()\n");
	   printf("**** The slots are: ");
	   for (i=0; i<BOARDSIZE+1; i++) {
	   printf("%d,",b->slots[i]);
	   }
	   printf("\n**** usersTurn is %s\n", (b->usersTurn) ? "TRUE" : "FALSE");
	   printf("**** Primitive()\n");
	 */

	// checking the ranks/files in one direction
	i=0;
	while (!primitiveFound && i<GAMEDIMENSION) {
		j=0;
		while (!emptyFound && j<GAMEDIMENSION) {
			if (b->slots[i*GAMEDIMENSION+j+1] == EMPTYSLOT) {
				emptyFound = TRUE;
			}
			rowColDiag[j] = b->slots[i*GAMEDIMENSION+j+1];
			j++;
		}
		if (!emptyFound) primitiveFound = searchPrimitive(rowColDiag);
		i++;
		emptyFound = FALSE;
	}

	// checking the ranks/files in the other direction
	i=0;
	while (!primitiveFound && i<GAMEDIMENSION) {
		j=0;
		while (!emptyFound && j<GAMEDIMENSION) {
			if (b->slots[j*GAMEDIMENSION+i+1] == EMPTYSLOT) {
				emptyFound = TRUE;
			}
			rowColDiag[j] = b->slots[j*GAMEDIMENSION+i+1];
			j++;
		}
		if (!emptyFound) primitiveFound = searchPrimitive(rowColDiag);
		i++;
		emptyFound = FALSE;
	}

	// checking one of the diagonals
	i=0;
	while (!primitiveFound && !emptyFound && i<GAMEDIMENSION) {
		if (b->slots[i*GAMEDIMENSION+i+1] == EMPTYSLOT) {
			emptyFound = TRUE;
		}
		rowColDiag[i] = b->slots[i*GAMEDIMENSION+i+1];
		i++;
	}
	if (!emptyFound) primitiveFound = searchPrimitive(rowColDiag);
	emptyFound = FALSE;

	// checking the other diagonal
	i=0;
	while (!primitiveFound && !emptyFound && i<GAMEDIMENSION) {
		if (b->slots[(i+1)*GAMEDIMENSION-i] == EMPTYSLOT) {
			emptyFound = TRUE;
		}
		rowColDiag[i] = b->slots[(i+1)*GAMEDIMENSION-i];
		i++;
	}
	if (!emptyFound) primitiveFound = searchPrimitive(rowColDiag);
	emptyFound = FALSE;

	// returning stuff
	if (primitiveFound) {
		toReturn = gStandardGame ? lose : win;
	} else {
		// Mario: comparing to NUMPIECES instead of BOARDSIZE
		toReturn = (b->squaresOccupied<NUMPIECES) ? undecided : tie;
	}

	SafeFree(rowColDiag);

	/* Deallocate board if not using GPS */
	if (!gUseGPS) {
		FreeBoard(b);
	}

	return toReturn;

}

// helper function called by Primitive()
// precondition: all elements of rowColDiag != EMPTYSLOT
// returns true iff rowColDiag is a primitive
BOOLEAN searchPrimitive(short *rowColDiag) {

	short inverterMask = NUMPIECES-1;
	short noninvertedResult = rowColDiag[0];
	short invertedResult = inverterMask ^ rowColDiag[0];
	short i;

	// print debugging stuff
	/*
	   printf("**** searchPrimitive() examines ");
	   for (i=0; i<GAMEDIMENSION; i++) {
	   printf("%d,",rowColDiag[i]);
	   }
	   printf("\n");
	 */

	// detects primitives through cumulative bitwise &
	for (i=0; i<GAMEDIMENSION; i++) {
		noninvertedResult &= rowColDiag[i];
		invertedResult &= inverterMask ^ rowColDiag[i];
	}

	return (noninvertedResult>0 || invertedResult>0);

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


void PrintPosition (POSITION position, STRING playersName, BOOLEAN usersTurn )
{
	newline();
	printPos(position,playersName,usersTurn);
}

void marioPrintPos(POSITION position, STRING playersName, BOOLEAN usersTurn )
{

	QTBPtr board;
	int j, rsize;
	BOOLEAN pieces[NUMPIECES], comma;

	/* Use GPS board if GPS solving, otherwise use unhashed position */
	board = gUseGPS ? GPSBoard : unhash( position );

	for( rsize = GAMEDIMENSION, j = FIRSTSLOT; j <= LASTSLOT; j+=rsize ) {

		if( j == FIRSTSLOT ) {
			//printf("debugging mode:");
			// newline();
			printf("        +");
			PrintCell( NULL, BorderCell );
			PrintRange( hex_ascii, sizeof( *hex_ascii ), j, rsize, BorderCell, '+', "" );
			printf("                +");
			PrintCell( NULL, BorderCell );
			PrintRange( hex_ascii, sizeof( *hex_ascii ), j, rsize, BorderCell, '+', "\n" );
			printf("LEGEND: |");
			PrintCell( hex_ascii + HAND * sizeof( char ), LegendCoordinate );

		} else {

			printf( "         " );
			PrintCell( NULL, BlankCell );

		}
		PrintRange( hex_ascii, sizeof( *hex_ascii ), j, rsize, LegendCoordinate, '|', "" );
		if( j == FIRSTSLOT ) {
			printf("         BOARD: |");
			PrintCell( board->slots + HAND * sizeof( *board->slots ), PieceTrait );

		} else {

			printf( "                 " );
			PrintCell( NULL, BlankCell );

		}
		PrintRange( board->slots, sizeof( *board->slots ), j, rsize, PieceTrait, '|', "\n" );

		if( j == FIRSTSLOT ) {

			printf("        +");
			PrintCell( NULL, BorderCell );
			PrintRange( hex_ascii, sizeof( *hex_ascii ), j, rsize, BorderCell, '+', "" );
			printf("                +");
			PrintCell( NULL, BorderCell );
			PrintRange( hex_ascii, sizeof( *hex_ascii ), j, rsize, BorderCell, '+', "\n" );

		} else {

			printf("         ");
			PrintCell( NULL, BlankCell );
			PrintRange( hex_ascii, sizeof( *hex_ascii ), j, rsize, BorderCell, '+', "" );
			printf("                 ");
			PrintCell( NULL, BlankCell );
			PrintRange( hex_ascii, sizeof( *hex_ascii ), j, rsize, BorderCell, '+', "\n" );

		}

	}

	FlagAvailablePieces( board, pieces );
	printf("\nAvailable pieces: ");

	for( comma = FALSE, j = 0; j < NUMPIECES; j++ ) {

		if( pieces[j] ) {

			if ( comma ) printf( ", " ); else comma = TRUE;
			PrintCell( &j, PieceTrait );

		}

	}
	newline();

	/* Deallocate board if not using GPS */
	if (!gUseGPS) {
		FreeBoard(board);
	}

}

// very crude printSlots for testing hash/unhash
void yanpeiPrintSlots(POSITION position, STRING playersName, BOOLEAN usersTurn ) {
/*
    QTBPtr b = unhash(position);
    short i;

    printf("  hash value " POSITION_FORMAT, position);
    printf("; slots: ");
    for (i=0; i<BOARDSIZE+1; i++) {
        if (b->slots[i] != EMPTYSLOT) {
            printf("%3d",b->slots[i]);
        } else {
            printf("  -");
        }
    }
    newline();*/
	//printf("squaresOccupied %d\n",b->squaresOccupied);
	//printf("piecesInPlay    %d\n",b->piecesInPlay);


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
	printf( "%s's move was: ", computersName);
	PrintMove(computersMove);
	newline();
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

void PrintMove( MOVE move )
{

	unsigned short slot, piece, trait;

	/* Determine piece information */
	piece       = GetMovePiece( move );

	/* Determine slot information */
	slot        = GetMoveSlot( move );

	/* Print character describing position on board/hand */
	printf( "%c,", hex_ascii[slot] );

	/* For each piece trait */
	for( trait = 0; trait < GAMEDIMENSION; trait++ ) {

		/* Print the corresponding character describing its state */
		printf( "%c", states[trait][(piece >> trait) & 1] );

	}



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

		int i;
		printf("\n%s's move [(undo)/", playersName );

		//printf("[%c,%c-%c]], ", hex_ascii[HAND], hex_ascii[FIRSTSLOT], hex_ascii[LASTSLOT] );
		printf("[%c-%c],[", hex_ascii[FIRSTSLOT], hex_ascii[LASTSLOT] );
		for( i = 0; i < GAMEDIMENSION; i++ ) {

			printf( "(%c,%c)", states[i][0], states[i][1] );

		}
		printf("]]: ");
		input = HandleDefaultTextInput(position, move, playersName);

		if (input != Continue) {

			return input;

		}

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

BOOLEAN ValidTextInput( STRING input )
{
	BOOLEAN positionValid = FALSE;
	BOOLEAN traitValid = FALSE;

	if ( ( strlen( input ) == ( 2 + GAMEDIMENSION ) ) && input[1] == ',' ) {
		int i;
		int valid_traits[GAMEDIMENSION];
		memset( valid_traits, 0, GAMEDIMENSION * sizeof( int ) );
		// Checking if position indicated is valid
		for( i = 0; i < BOARDSIZE+1; i++ ) {
			if( input[0] == hex_ascii[i] ) {
				positionValid = TRUE;
				break;
			}
		}
		if ( positionValid ) {
			//check traits
			int trait;
			for( trait = 2; trait < GAMEDIMENSION+1; trait++ ) {
				for( i = 0; i < GAMEDIMENSION; i++ ) {
					if ( input[trait] == states[i][0] || input[trait] == states[i][1] ) {
						traitValid = TRUE; //if the trait is valid, go one to next piece...
						break;
					}
				}
				if (traitValid == FALSE) break;
			}
		}
	}
	(positionValid && traitValid) ? printf ("is valid") : printf("not valid");
	return (positionValid && traitValid);
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

	MOVE piece = 0, slot = 0;
	int i,  k;

	/* Lower GAMEDIMENSION + 1 bits for position */
	for( slot = 0; ( slot < BOARDSIZE + 1 ) && hex_ascii[slot] != input[0]; slot++ ) ;

	/* Adjacent GAMEDIMENSION bits for piece */
	for( i = 2; i < GAMEDIMENSION+2; i++ ) {
		BOOLEAN ready = FALSE;
		//for( j = 0; j < GAMEDIMENSION; j++ ) {
		for( k = 0; k < 2; k++ ) {
			if( states[i-2][k] == input[i] ) {
				piece += (k << (i-2));
				ready = TRUE;
				break;
			}
		}
		//if ( ready ) {
		//	break;
		//}
		//}
	}
	return CreateMove( slot, piece );
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
	char choice;
	BOOLEAN validInput = FALSE;

	while (!validInput) {
		printf("\t---- mquarto Option Menu ---- \n\n");
		printf("\ti)\tChange Game D(i)mension: Currently %d-Dimensional\n\n",GAMEDIMENSION);

		printf("Select an option: ");
		//fflush(stdin); // This should only work on windows, flushing input is an ambiguous concept
		//flush();
		choice = GetMyChar();
		switch(choice) {
		case 'I': case 'i':
			printf("Please enter the new GAMEDIMENSION (must be between 2 to 4): ");
			/*scanf("%d",&GAMEDIMENSION);*/
			int temp = GetMyInt();
			if(temp < 2 || temp > 4) {
				printf("Board Dimensions must be between 2 to 4.\n");
				HitAnyKeyToContinue();
				break;
			}
			GAMEDIMENSION = temp;
			InitializeGame( );
			validInput = TRUE;
			break;
		default:
			printf("Not a valid option.\n");
			break;
		}
	}

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
	return 6;
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
	int temp = 0;
	if(gStandardGame)
		temp += 3;
	temp += (GAMEDIMENSION - 1);
	return temp;
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
	if(option > 3) { //aka 4, 5, or 6
		option -= 3;
		gStandardGame = 1;
		setOption(option);
	}
	GAMEDIMENSION = option + 1;
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

// printing an unsigned int as a GAMEDIMENSION digits binary characters
STRING binStr(unsigned int x) {

	int i;
	STRING toReturn = (STRING) SafeMalloc(GAMEDIMENSION*sizeof(char));

	for (i=0; i<GAMEDIMENSION; i++) {
		toReturn[GAMEDIMENSION-1-i] = (x % 2) + '0';
		x = x/2;
	}

	return toReturn;

}

QTBPtr MallocBoard() {
	QTBPtr toReturn = (QTBPtr) SafeMalloc(sizeof(QTBOARD));
	toReturn->slots = (short *) SafeMalloc((BOARDSIZE+1)*sizeof(short));
	return toReturn;
}

void FreeBoard(QTBPtr b) {
	SafeFree(b->slots);
	SafeFree(b);
}

/* Mario: returns true if board contents match exactly */
BOOLEAN boards_equal ( QTBPtr b1, QTBPtr b2 ) {

	BOOLEAN equal = TRUE;
	int slot;

	equal &= ( b1->piecesInPlay == b2->piecesInPlay );
	equal &= ( b1->squaresOccupied == b2->squaresOccupied );
	equal &= ( b1->usersTurn == b2->usersTurn );

	for( slot = 0; slot < BOARDSIZE + 1; slot++ ) {

		equal &= ( b1->slots[slot] == b2->slots[slot] );

	}

	return equal;

}

/* Yanpei: copies board contents to b1 from b2 exactly */
void copy_board ( QTBPtr b1, QTBPtr b2 ) {

	int slot;

	b1->piecesInPlay = b2->piecesInPlay;
	b1->squaresOccupied = b2->squaresOccupied;
	b1->usersTurn = b2->usersTurn;

	for( slot = 0; slot < BOARDSIZE + 1; slot++ ) {

		b1->slots[slot] = b2->slots[slot];

	}


}

/* Mario: prints board (for internal use)*/
void print_board( QTBPtr board ) {

	int slot;

	fprintf( stderr, "[" );
	for ( slot = 0; slot < BOARDSIZE + 1; slot++ ) {

		if ( slot == HAND ) {

			fprintf( stderr, "|" );

		}

		if( board->slots[slot] == EMPTYSLOT ) {

			fprintf( stderr, " X " );

		} else {

			fprintf( stderr, " %d ", board->slots[slot] );

		}

		if ( slot == HAND ) {

			fprintf( stderr, "|" );

		}

	}
	fprintf( stderr, "] pieces = %d; squares = %d; turn = %d ", board->piecesInPlay, board->squaresOccupied, board->usersTurn );

}

// hashing an internally represented QTBOARD into a POSITION
// fully general, works for any GAMEDIMENSION
POSITION hashUnsymQuarto(QTBPtr b) {

	POSITION toReturn;
	QTBPtr helperBoard = MallocBoard();
	POSITION squaresOccupiedOffset, firstSlotOffset;
	short i;

	if (!offsetTableSet) setOffsetTable();

	if (b->squaresOccupied==0 && b->piecesInPlay==0) {
		// no piece in hand, no pieces on board, trivial case
		toReturn = 0;
	} else if (b->squaresOccupied==0 && b->piecesInPlay==1) {
		// only piece in hand, no pieces on board, also straight forward
		toReturn = b->slots[0] + offsetTable[b->squaresOccupied];
	} else {
		// pieces on board, a bit more complicated
		// make deep structural copy of b
		for (i=0; i<BOARDSIZE+1; i++) {
			helperBoard->slots[i] = b->slots[i];
		}
		helperBoard->squaresOccupied = b->squaresOccupied;
		helperBoard->piecesInPlay = b->piecesInPlay;
		helperBoard->usersTurn = b->usersTurn;
		// finding some offsets
		squaresOccupiedOffset = offsetTable[b->squaresOccupied];
		firstSlotOffset =  permutation(NUMPIECES-1,b->squaresOccupied)
		                  *combination(BOARDSIZE,  b->squaresOccupied);
		// prepare helperBoard for recursive call
		for (i=1; i<BOARDSIZE+1; i++) {
			if (helperBoard->slots[i] >= b->slots[0] &&
			    helperBoard->slots[i] != EMPTYSLOT) {
				helperBoard->slots[i]--;
			}
		}
		// throw the rest of the problem to a helper function
		if (b->squaresOccupied < NUMPIECES) {
			// pieces in hand and on board
			toReturn =   squaresOccupiedOffset
			           + b->slots[0]*firstSlotOffset
			           + hashUnsymQuartoHelper(helperBoard, 1);
		} else {
			// no piece in hand, all pieces on board
			toReturn =   squaresOccupiedOffset
			           + hashUnsymQuartoHelper(helperBoard, 1);
		}

	}


	/*
	   if (b->usersTurn) {
	   toReturn = toReturn + offsetTable[NUMPIECES];
	   }
	 */

	// giving space back to the malloc Godfather
	FreeBoard(helperBoard);
	return toReturn;

}

POSITION hashUnsymQuartoHelper(QTBPtr b, int baseSlot) {

	short slotsSubset = BOARDSIZE - baseSlot + 1; // # of slots starting from baseSlot
	short slotsOccupiedSubset = 0; // # of occupied slots starting from baseSlot
	short firstSlot = 0; // first occupied slot
	short firstPiece = EMPTYSLOT; // piece in firstSlot
	short piecesBeforeBase = 0; // # of pieces before baseSlot
	POSITION firstPieceOffset = 0;
	POSITION firstSlotOffset = 0;
	short i;
	POSITION toReturn;

	// traverse b->slots to set up local vars
	for (i=0; i<BOARDSIZE+1; i++) {
		if (i<baseSlot && b->slots[i]!=EMPTYSLOT) {
			piecesBeforeBase++;
		}
		if (i>=baseSlot && b->slots[i]!=EMPTYSLOT) {
			if (firstSlot == 0) firstSlot = i;
			if (firstPiece == EMPTYSLOT) firstPiece = b->slots[i];
		}
	}
	slotsOccupiedSubset = b->piecesInPlay - piecesBeforeBase;

	if (slotsOccupiedSubset == 0) {
		// error: recursive call should never have no pieces starting from baseSlot
		toReturn = 0;
		printf("\nError: hashUnsymQuartoHelper() check recursive call\n");
	} else if (slotsOccupiedSubset == 1) {
		// base case
		toReturn = firstPiece*slotsSubset + (firstSlot - baseSlot);
	} else {
		// more complicated
		// calculating a couple of offsets
		for (i=slotsSubset-1; i>=slotsOccupiedSubset-1; i--) {
			if (slotsSubset-i-1 == firstSlot-baseSlot) {
				firstSlotOffset = firstPieceOffset;
			}
			firstPieceOffset +=
			        permutation(NUMPIECES-piecesBeforeBase-1,slotsOccupiedSubset-1)
			        *combination(i,slotsOccupiedSubset-1);
		}
		// preparing b for recursive call
		for (i=firstSlot+1; i<BOARDSIZE+1; i++) {
			if (b->slots[i] > b->slots[firstSlot] &&
			    b->slots[i] != EMPTYSLOT) {
				b->slots[i]--;
			}
		}
		// throw the rest of the problem onto a recursive call
		toReturn =  firstPiece*firstPieceOffset
		           +firstSlotOffset
		           +hashUnsymQuartoHelper(b,firstSlot+1);
	}

	return toReturn;
}

// unhashing a POSITION into an internally represented QTBOARD
QTBPtr unhashUnsymQuarto(POSITION p) {

	QTBPtr toReturn = MallocBoard();
	short i;
	POSITION firstSlotOffset, squaresOccupiedOffset;
	POSITION pHelper;

	// set all empty square to EMPTYSLOT at first, to be filled up w/ pieces
	for (i=0; i<BOARDSIZE+1; i++) {
		toReturn->slots[i] = EMPTYSLOT;
	}

	if (p == 0) {
		// no piece in hand and no piece on board, trivial case
		toReturn->squaresOccupied = 0;
		toReturn->piecesInPlay = 0;
	} else if (p < offsetTable[NUMPIECES]) {
		// piece in hand and possibly on board, more complicated
		for (i=1; i<NUMPIECES+1; i++) {
			if (p>=offsetTable[i-1] && p<offsetTable[i]) {
				// figure out these values
				toReturn->squaresOccupied = i-1;
				toReturn->piecesInPlay = i;
				// calculating a couple of offsets
				squaresOccupiedOffset = offsetTable[toReturn->squaresOccupied];
				firstSlotOffset =  permutation(NUMPIECES-1,toReturn->squaresOccupied)
				                  *combination(BOARDSIZE,  toReturn->squaresOccupied);
				// the first piece is easy enough
				toReturn->slots[0] = (p - squaresOccupiedOffset) / firstSlotOffset;
				// prepare for recursion
				pHelper = p - squaresOccupiedOffset
				          - toReturn->slots[0]*firstSlotOffset;
				// throw rest of problem to helper function
				unhashUnsymQuartoHelper(pHelper, 1, toReturn);
			}
		}
	} else {
		// no piece in hand, all pieces on board
		toReturn->squaresOccupied = NUMPIECES;
		toReturn->piecesInPlay = NUMPIECES;
		squaresOccupiedOffset = offsetTable[toReturn->squaresOccupied];
		pHelper = p - squaresOccupiedOffset;
		unhashUnsymQuartoHelper(pHelper, 1, toReturn);
	}

	return toReturn;

}

void unhashUnsymQuartoHelper(POSITION p, int baseSlot, QTBPtr toReturn) {

	short i,j,k;
	BOOLEAN appearedBeforeBase = FALSE;
	short piecesBeforeBase = 0;
	short slotsOccupiedSubset = 0;
	short slotsSubset = BOARDSIZE - baseSlot + 1;
	short firstSlot=-1,firstPiece=0;
	short *piecesSubset = SafeMalloc(NUMPIECES*sizeof(short));
	POSITION firstSlotOffset=0, firstPieceOffset=0;

	// traversing toReturn to setup local vars
	for (i=0; i<BOARDSIZE+1; i++) {
		if ((i<baseSlot) && (toReturn->slots[i] != EMPTYSLOT)) {
			piecesBeforeBase++;
		}
	}
	slotsOccupiedSubset = toReturn->piecesInPlay - piecesBeforeBase;

	if (slotsOccupiedSubset == 0) {
		// yay we're done!
	} else if (slotsOccupiedSubset == 1) {
		// base case
		firstPiece = p / slotsSubset;
		firstSlot = p - firstPiece*slotsSubset;
		// extracting the offsetted pieces to pieceSubset[]
		k=0;
		for (j=0; j<NUMPIECES; j++) {
			appearedBeforeBase = FALSE;
			for (i=0; i<baseSlot; i++) {
				if (j == toReturn->slots[i]) appearedBeforeBase = TRUE;
			}
			if (!appearedBeforeBase) {
				piecesSubset[k] = j;
				k++;
			}
		}
		// assigning the piece just unhashed to its slot
		toReturn->slots[firstSlot+baseSlot] = piecesSubset[firstPiece];
	} else {
		// more complicated
		// calculating an offsets
		for (i=0; i<=slotsSubset-slotsOccupiedSubset; i++) {
			firstPieceOffset +=
			        permutation(NUMPIECES-piecesBeforeBase-1,slotsOccupiedSubset-1)
			        *combination(slotsSubset-i-1,slotsOccupiedSubset-1);
		}
		// finding and setting firstPiece and firstSlot
		firstPiece = p / firstPieceOffset;
		i=0;
		while (i<=slotsSubset-slotsOccupiedSubset && firstSlot<0) {
			firstSlotOffset +=
			        permutation(NUMPIECES-piecesBeforeBase-1,slotsOccupiedSubset-1)
			        *combination(slotsSubset-i-1,slotsOccupiedSubset-1);
			if (p-firstPiece*firstPieceOffset < firstSlotOffset) {
				firstSlot = i;
			}
			i++;
		}
		// finding another offset
		firstSlotOffset -=  permutation(NUMPIECES-piecesBeforeBase-1,slotsOccupiedSubset-1)
		                   *combination(slotsSubset-firstSlot-1,slotsOccupiedSubset-1);
		// extracting the offsetted pieces to pieceSubset[]
		k=0;
		for (j=0; j<NUMPIECES; j++) {
			appearedBeforeBase = FALSE;
			for (i=0; i<baseSlot; i++) {
				if (j == toReturn->slots[i]) appearedBeforeBase = TRUE;
			}
			if (!appearedBeforeBase) {
				piecesSubset[k] = j;
				k++;
			}
		}
		// assigning the piece just unhashed to its slot
		toReturn->slots[firstSlot+baseSlot] = piecesSubset[firstPiece];
		// preparing for recursion
		p = p - firstPiece*firstPieceOffset - firstSlotOffset;
		// throw the rest of the problem onto a recursive call
		unhashUnsymQuartoHelper(p, firstSlot+baseSlot+1, toReturn);
	}

	// giving space back to the malloc Godfather
	SafeFree(piecesSubset);
}


// set factorialTable
void setFactorialTable() {

	int i;

	if (!factorialTableSet) {
		factorialTable[0] = 1;
		for (i=1; i<FACTORIALMAX; i++) {
			factorialTable[i] = factorialTable[i-1] * i;
		}
		factorialTableSet = TRUE;
	}
}

POSITION factorialMem(int n) {
	if (!factorialTableSet) setFactorialTable();
	if (n<=0) {
		return 1;
	} else if (FACTORIALMAX<=n) {
		return factorialTable[FACTORIALMAX-1];
	} else {
		return factorialTable[n];
	}
}

/* Non-memoizing factorial */
POSITION factorialNoMem( int n ) {

	POSITION result = 1;

	while ( n > 0 ) {

		result*=n--;

	}

	return result;

}

// returns n P r, the number of ordered arrangements of
// r items selected from a set of n items
POSITION permutation(int n, int r) {
	/* Mario: updating base cases, should return 1 when r <= 0, 0 when otherwise n <= 0
	   Used to return 1 if either n or r are greater than 0, which is virtually always */
	if (r <= 0) {
		return 1;
	} else if (n <= 0) {
		return 0;
	} else {
		return factorial(n) / factorial(n-r);
	}
}

// returns n C r, the number of unordered arrangements of
// r items selected from a set of n items
POSITION combination(int n, int r) {
	/* Mario: updating base cases, should return 1 when r <= 0, 0 when otherwise n <= 0
	   Used to return 1 if either n or r are greater than 0, which is virtually always */
	if (r <= 0) {
		return 1;
	} else if (n <= 0) {
		return 0;
	} else {
		return factorial(n) / factorial(n-r) / factorial(r);
	}
}

// sets the offsetTable
void setOffsetTable() {

	int i;

	if (!offsetTableSet) {
		offsetTable[0] = 1;
		for (i=1; i<NUMPIECES+2; i++) {
			offsetTable[i] = offsetTable[i-1] +
			                 permutation(NUMPIECES,i-1)
			                 *combination(BOARDSIZE,i-1)
			                 *(NUMPIECES-i+1 ? NUMPIECES-i+1 : 1);
		}
		//offsetTable[NUMPIECES+1] = offsetTable[NUMPIECES] + factorial(NUMPIECES);
		// ABOVE IS ERROR CODE
		offsetTableSet = TRUE;
	}
}


/* Creates move given slot and piece */
MOVE CreateMove( MOVE slot, MOVE piece ) {

	return piece + ( slot << ( GAMEDIMENSION ) );


}

/* Returns piece given move */
MOVE GetMoveSlot( MOVE move ) {

	return move >> ( GAMEDIMENSION );

}

/* Returns slot given move */
MOVE GetMovePiece( MOVE move ) {

	return move & maskseq( GAMEDIMENSION );

}

/* Prints horizontal border for board */
void PrintHorizontalBorder( char fill, char border, char *startmark, char *endmark ) {

	int position;

	printf( "%s%c", startmark, border );

	for( position = 1; position < (GAMEDIMENSION + 1) * GAMEDIMENSION; position++ ) {

		if( position % ( GAMEDIMENSION + 1) ) {

			printf( "%c", fill );

		} else {

			printf( "%c", border );

		}

	}

	printf( "%c%s", border, endmark );

}

/* Extracts character information about piece trait */
char PieceTrait( short trait, void *p_piece ) {

	short piece = *((short *) p_piece);

	return ( piece == EMPTYSLOT ) ? ' ' : states[trait][( piece >> trait ) & 1];

}

char LegendCoordinate( short pad, void *p_coordinate ) {

	char coordinate = *((char *) p_coordinate);

	return ( pad < GAMEDIMENSION - 1 ) ? ' ' : coordinate;

}

char BorderCell( short ignored, void* p_ignored ) {

	return '-';

}

char BlankCell( short ignored, void* p_ignored ) {

	return ' ';

}

/* Prints cell for board */
void PrintCell( void *cell, char (*CellContent)( short, void * ) ) {

	short trait;

	for( trait = 0; trait < GAMEDIMENSION; trait++ ) {

		printf( "%c", CellContent( trait, cell ) );

	}

}

void PrintRange( void *cells, size_t content_size, int offset, int size, char (*CellContent)( short, void * ), char border, char *endmark ) {

	int column;

	printf( "%c", border );
	for( column = 0; column < size; column++ ) {

		PrintCell( cells + (offset + column)*content_size, CellContent );
		printf( "%c", border );

	}

	printf( "%s", endmark );
}

unsigned short GetHandPiece( QTBPtr board ) {

	return board->slots[HAND];

}

void SetHandPiece( QTBPtr board, unsigned short piece ) {

	board->slots[HAND] = piece;

}

unsigned short GetBoardPiece( QTBPtr board, unsigned short slot ) {

	return board->slots[slot + FIRSTSLOT];

}

void yanpeiTestOffset() {

	int i;

	printf("\nTesting offsetTable ... \n");
	for (i=0; i<NUMPIECES+2; i++) {
		printf("offsetTable[%d] = " POSITION_FORMAT "\n",i,offsetTable[i]);
	}
	printf("\n");

}

void yanpeiTestHash() {

	POSITION i,h;
	BOOLEAN allPassed = TRUE;
	void (*oldPrintPos)(POSITION position, STRING playersName, BOOLEAN usersTurn ) = printPos;

	printf("\nTesting full blown hash/unhash ... \n");

	printPos = &yanpeiPrintSlots;

	//i=offsetTable[NUMPIECES];
	//while (i<offsetTable[NUMPIECES+1] && allPassed) {
	i = 0;
	while (i<offsetTable[2]+50 && allPassed) {
		//printf("%8d\n",i);
		if (i != (h=hash(unhash(i)))) {
			allPassed = FALSE;
			printf("hash/unhash error:\n");
			printf("position: " POSITION_FORMAT ", hashed: " POSITION_FORMAT,i,h);
			newline();
		}
		//PrintPosition(i,"",TRUE);
		i++;
	}
	if (allPassed) printf("\n ... testHash() passed.\n");
	else printf("\n ... testHash() failed.\n");

	printPos = oldPrintPos;
}

static QTBPtr rotateBoard90(QTBPtr b) {

	QTBPtr c = MallocBoard();
	short i,j;

	c->squaresOccupied = b->squaresOccupied;
	c->piecesInPlay = b->piecesInPlay;
	c->usersTurn = b->usersTurn;

	for (i=0; i<GAMEDIMENSION; i++) {
		for (j=0; j<GAMEDIMENSION; j++) {
			c->slots[GAMEDIMENSION*i + j + 1]
			        = b->slots[GAMEDIMENSION*j + (GAMEDIMENSION-i-1) + 1];
		}
	}
	c->slots[0] = b->slots[0];

	return c;
}

static QTBPtr reflectBoard(QTBPtr b) {

	QTBPtr c = MallocBoard();
	short i,j;

	c->squaresOccupied = b->squaresOccupied;
	c->piecesInPlay = b->piecesInPlay;
	c->usersTurn = b->usersTurn;

	for (i=0; i<GAMEDIMENSION; i++) {
		for(j=0; j<GAMEDIMENSION; j++) {
			c->slots[GAMEDIMENSION*i + j + 1]
			        = b->slots[GAMEDIMENSION*i + (GAMEDIMENSION-j-1) + 1];
		}
	}
	c->slots[0] = b->slots[0];

	return c;
}

static void swap_columns(short *pieces, short count, short this, short that) {

	/* Placeholder for index to pieces in pieces array */
	int piece;

	// Go through each piece
	for (piece = 0; piece < count; piece++) {

		/* Placeholders to bits this and that in pieces array at index piece */
		short this_bit, that_bit;

		// Extract the this bit
		this_bit = get_bit(pieces[piece], this);

		// Extract the that bit
		that_bit = get_bit(pieces[piece], that);

		// Set the this bit of piece to the that bit
		set_bit(pieces[piece], this, that_bit);

		// Set the that bit of piece to the this bit
		set_bit(pieces[piece], that, this_bit);

	}

}


static short *normalize(short *pieces, short count) {

	/* Array to store sequences of individual bits across the different pieces */
	/* The first row of each column contains 0 (which will be the final value of the first piece */
	short bit_columns[GAMEDIMENSION];

	/* Placeholder for number of bit sequences that can no longer be exchanged */
	short invariants;

	/* Placeholders for locations of bits in a bit array (integer) */
	short this, that;

	/* Placeholders for bits of bit array (integer) */
	short this_bit, that_bit;

	/* Placeholder for indexing of individual pieces of the pieces array */
	short piece;

	/* Initialize each bit column to 0 */
	for (this = 0; this < GAMEDIMENSION; this++) {

		/* Set column to 0 */
		bit_columns[this] = 0;

	}

	/* Toggle every bit of every piece which is set on the first piece
	   However, the first piece should be filtered last
	   Sample input: { 1011, 0101, 1010, 1110 }
	   Corresponding output: { 0000, 1110, 0001, 0101 }
	 */
	for (piece = count - 1; piece >= 0; piece--) {

		/* The bit array of the first piece serves as the XOR mask for all of them */
		pieces[piece] ^= pieces[0];

	}

	/* Go through every piece starting with the second piece, and swap two bits uniformly,
	   while preserving the equivalence of individual bit columns
	   Sample input: { 0000, 1110, 0001, 0101 }, bit_column = { 0, 0, 0, 0 }, invariants = 0
	   After 2nd piece: { 0000, 0111, 1000, 1100 }, bit_column = { 00, 01, 01, 01 }, invariants = 1
	   After 3rd piece: { 0000, 0111, 1000, 1100 }, bit_column = { 001, 010, 011, 010 }, invariants = 2
	   After 4th piece: { 0000, 0111, 1000, 1001 }, bit_column = { 0011, 0100, 0110, 0101 }, invariants = 4
	 */
	for (piece = 1, invariants = 0; invariants < GAMEDIMENSION && piece < count; piece++) {

		/* Mark all columns as invariant; to be decremented later when equivalent columns are known better */
		invariants = GAMEDIMENSION;

		/* Start from the most significant bit, and pair it with a less significant bit */
		for (this = GAMEDIMENSION - 1; this >= 0; this--) {

			/* Placeholder for number of columns equivalent to this column */
			int equivalences = 0;

			/* Loop through every less significant bit than this bit.
			   Stop if this_bit turns to be 0, or have exhausted all less signifcant bits
			 */
			for (that = this - 1, this_bit = get_bit(pieces[piece], this);
			     this_bit != 0 && that >= 0;
			     that--) {

				/* Obtain the less significant bit (indexed by that) */
				that_bit = get_bit(pieces[piece], that);

				/* If the more significant bit is 1 and the less significant bit is 0 */
				if (this_bit > that_bit) {

					/* If this and that columns are equivalent, swap the two columns */
					if (bit_columns[this] == bit_columns[that]) {

						/* Swap columns this and that on the array starting at piece piece.
						   It should be noted that the result on the entire array ought to be the same
						 */
						swap_columns(pieces + piece, count - piece, this, that);

						/* Update this_bit to the swapped-in value */
						this_bit = get_bit(pieces[piece], this);

					}

				}

			}

			/* Now, this column is fixed, so add this row cell to it.
			   Shift the other rows up by 1 and add this bit
			 */
			bit_columns[this] <<= 1;
			bit_columns[this] += this_bit;

			/* Now find number of columns equivalent to this column */
			for (that = GAMEDIMENSION - 2; that >= 0; that--) {

				/* Obtain the less significant bit (indexed by that) */
				that_bit = get_bit(pieces[piece], that);

				/* If this column equals to that column (in its current state, but up to permutation of columns) */
				if (bit_columns[this] == (bit_columns[that] << 1) + that_bit) {

					/* Increment number of equivalent columns */
					equivalences++;

				}

			}

			/* Decrement invariants by 1 if there are more than two equivalences */
			if (equivalences > 1) {
				invariants--;
			}
			/* Decrement invariants by 2 if there is one equivalence */
			else if (equivalences == 1) {
				invariants -= 2;
			}

		}

	}

	return pieces;

}



static QTBPtr normalizeBoard(QTBPtr board) {

	// Array of ordered pieces to be normalized
	short pieces[board->piecesInPlay];

	// Locations of ordered pieces on the board
	short locations[board->piecesInPlay];

	// Index to location on board
	int index;

	// Index to piece in ordered set of pieces
	int piece;

	// For every location on board, until all pieces have been examined
	for (index = 0, piece = 0; piece < board->piecesInPlay; index++) {

		// If board location contains a piece
		if (board->slots[index] != EMPTYSLOT) {

			// Store the piece value in ordered array of pieces
			pieces[piece] = board->slots[index];

			// Store the piece location on board
			locations[piece] = index;

			// Increment piece index
			piece++;

		}

	}

	// Normalize pieces
	normalize(pieces, board->piecesInPlay);

	// For every piece in ordered set of normalized pieces
	for (piece = 0; piece < board->piecesInPlay; piece++) {

		// Place normalized piece back on the board in the same location
		board->slots[locations[piece]] = pieces[piece];

	}

	return board;

}

// Mario's Cannonical stuff

POSITION marioGetCanonical(POSITION position) {

	QTBPtr orbit[8];
	short group;

	orbit[0] = unhash(position);
	orbit[1] = rotateBoard90(orbit[0]);
	orbit[2] = rotateBoard90(orbit[1]);
	orbit[3] = rotateBoard90(orbit[2]);
	orbit[4] = reflectBoard(orbit[0]);
	orbit[5] = reflectBoard(orbit[1]);
	orbit[6] = reflectBoard(orbit[2]);
	orbit[7] = reflectBoard(orbit[3]);

	position = offsetTable[NUMPIECES+1];
	for (group = 0; group < 8; group++) {
		POSITION temp = hash(normalizeBoard(orbit[group]));
		position = (temp < position) ? temp : position;
		/* Free allocated board */
		FreeBoard(orbit[group]);
	}

	return position;

}

// Yanpei's Cannonical stuff

int **getLookupTableHelper(int d, int **prevTable) {

	int i,j,k,l,insert,temp,index;
	BOOLEAN pass = TRUE;
	int *hyperface = (int*)SafeMalloc(twoPowers[d-1] * sizeof(int));
	int *oppositeface = (int*)SafeMalloc(twoPowers[d-1] * sizeof(int));
	int **currentTable = (int **)SafeMalloc(twoPowers[d] * factorial(d) * sizeof(int *));

	for (i=0; i<(twoPowers[d] * factorial(d)); i++)
		currentTable[i] = (int *)SafeMalloc(twoPowers[d] * sizeof(int));

	// filling up the current lookup table
	index = 0;
	for (i=0; i<d; i++) { // places to insert 0 and 1 to generate hyperfaces
		             // for d dimensional hypercube from vertices of
		             // d-1 dimensional hypercube
		for (insert=0; insert<=1; insert++) { // insert 0 then insert 1

			for (j=0; j<twoPowers[d-1]; j++) { // to get a hyperface, one-to-one correspondence w/
				                          // vertices of d-1 dimensional hypercube
				hyperface[j] = (~(~0 << i) & j) | (((~0 << i) & j) << 1) | (insert << i);
			}

			/* print hyperface
			   printf("%14s","hyperface:");
			   for (test1=0; test1<twoPowers[d-1]; test1++) {
			   printf("  %2d",hyperface[test1]);
			   }
			   printf("\n");
			 */

			for (j=0; j<twoPowers[d-1]; j++) { // to get the hyperface opposite the previous
				for (k=0; k<d; k++) { // vertices are adjacent if they differ by 1 bit
					temp = hyperface[j] ^ (1 << k);
					for (l=0; l<twoPowers[d-1]; l++) { // test if temp is in hyperface and thus not the opposite
						pass = pass && (temp!=hyperface[l] || l==j);
					}
					if (pass) oppositeface[j] = temp;
					else pass = TRUE;
				}
			}

			/* print opposite face
			   printf("%14s","oppositeface:");
			   for (test1=0; test1<twoPowers[d-1]; test1++) {
			   printf("  %2d",oppositeface[test1]);
			   }
			   printf("\n");
			 */

			for (j=0; j < twoPowers[d-1]*factorial(d-1); j++) { // for each rotation in hyperface
				for (k=0; k<twoPowers[d-1]; k++) { // for each vertex in hyperface and oppositeface
					                          // derive corresponding vertex from prevTable
					//printf("%3d %3d %3d\n",j,k,index);
					currentTable[index+j][k]                = hyperface   [prevTable[j][k]];
					currentTable[index+j][k+twoPowers[d-1]] = oppositeface[prevTable[j][k]];
				}
				/* print row just completed
				   printf("%d: ", index+j);
				   for (k=0; k<twoPowers[d]; k++) {
				   printf("  %2d",currentTable[index+j][k]);
				   }
				   printf("\n");
				 */
			} // finished one hyperface

			index = index + twoPowers[d-1]*factorial(d-1);

		} // loop for another hyperface insert 0 or 1
	} // loop for another pair of hyperfaces by changing bit position where 0 or 1 is inserted

	// freeing stuff
	SafeFree(hyperface);
	SafeFree(oppositeface);
	if (prevTable != trivialTable) {
		for (i=0; i<(twoPowers[d-1] * factorial(d-1)); i++)
			SafeFree(prevTable[i]);
		SafeFree(prevTable);
	}

	// returning and recursing
	if (d==GAMEDIMENSION)
		return currentTable;
	else
		return getLookupTableHelper(d+1,currentTable);

}


int **getLookupTable() {

	if (GAMEDIMENSION==2) {
		return trivialTable;
	} else {
		return getLookupTableHelper(3, trivialTable);
	}

}

void printLookupTable(int **lookupTable) {

	int i,j;

	printf("\nlookupTable for GAMEDIMENSION %d\n",GAMEDIMENSION);

	if (GAMEDIMENSION>2) {
		printf("\n    ");
		for (j=0; j<twoPowers[GAMEDIMENSION]; j++) {
			printf(" %2d ",j);
		}
		printf("\n");
	}
	printf("\n");

	for (i=0; i<(twoPowers[GAMEDIMENSION]*factorial(GAMEDIMENSION)); i++) {

		printf("%5d:",i);
		for (j=0; j<twoPowers[GAMEDIMENSION]; j++) {
			printf(" %2d ",lookupTable[i][j]);
		}
		printf("\n");

		if (GAMEDIMENSION>2 &&
		    !((i+1) % (twoPowers[GAMEDIMENSION-1]*factorial(GAMEDIMENSION-1))))  {
			printf("\n      ");
			for (j=0; j<twoPowers[GAMEDIMENSION]; j++) {
				printf(" %2d ",j);
			}
			printf("\n\n");
		}
	}
}


POSITION yanpeiGetCanonical(POSITION p) {

	QTBPtr geometricSym[8];
	POSITION toReturn = offsetTable[NUMPIECES+1];
	POSITION temp;
	int i,j,k;
	QTBPtr tempBoard = MallocBoard();

	geometricSym[0] = unhash(p);
	geometricSym[1] = rotateBoard90(geometricSym[0]);
	geometricSym[2] = rotateBoard90(geometricSym[1]);
	geometricSym[3] = rotateBoard90(geometricSym[2]);
	geometricSym[4] = reflectBoard(geometricSym[0]);
	geometricSym[5] = reflectBoard(geometricSym[1]);
	geometricSym[6] = reflectBoard(geometricSym[2]);
	geometricSym[7] = reflectBoard(geometricSym[3]);

	for (i=0; i<8; i++) { // iterate thru geometric symmetries
		for (j=0; j<twoPowers[GAMEDIMENSION] * factorial(GAMEDIMENSION); j++) {
			// iterate thru rotation of pieces hypercube
			copy_board(tempBoard,geometricSym[i]);
			/*
			   printf("i %d, j %d\n",i,j);
			   temp = hash(tempBoard);
			   printPos(temp,"",TRUE);

			   for (k=0; k<twoPowers[GAMEDIMENSION]; k++) {
			   printf(" %2d ",lookupTable[j][k]);
			   }
			   printf("\n");
			 */
			for (k=0; k<BOARDSIZE+1; k++) {
				// change pieces on tempBoard to equivalents
				if (tempBoard->slots[k] != EMPTYSLOT) {
					tempBoard->slots[k] = lookupTable[j][tempBoard->slots[k]];
				}
			}
			temp = hash(tempBoard);
			/*
			   printPos(temp,"",TRUE);
			   printf("\n\n");
			 */
			toReturn = (temp < toReturn) ? temp : toReturn;
		}
		SafeFree(geometricSym[i]);
	}

	SafeFree(tempBoard);

	return toReturn;

}

POSITION rotatePosition90(POSITION p) {

	QTBPtr b = unhash(p);
	QTBPtr c = MallocBoard();
	short i,j;
	POSITION toReturn;

	c->squaresOccupied = b->squaresOccupied;
	c->piecesInPlay = b->piecesInPlay;
	c->usersTurn = b->usersTurn;

	for (i=0; i<GAMEDIMENSION; i++) {
		for (j=0; j<GAMEDIMENSION; j++) {
			c->slots[GAMEDIMENSION*i + j + 1]
			        = b->slots[GAMEDIMENSION*j + (GAMEDIMENSION-i-1) + 1];
		}
	}
	c->slots[0] = b->slots[0];

	toReturn = hash(c);
	FreeBoard(b);
	FreeBoard(c);

	return toReturn;
}

POSITION reflectPosition(POSITION p) {

	QTBPtr b = unhash(p);
	QTBPtr c = MallocBoard();
	short i,j;
	POSITION toReturn;

	c->squaresOccupied = b->squaresOccupied;
	c->piecesInPlay = b->piecesInPlay;
	c->usersTurn = b->usersTurn;

	for (i=0; i<GAMEDIMENSION; i++) {
		for(j=0; j<GAMEDIMENSION; j++) {
			c->slots[GAMEDIMENSION*i + j + 1]
			        = b->slots[GAMEDIMENSION*i + (GAMEDIMENSION-j-1) + 1];
		}
	}
	c->slots[0] = b->slots[0];

	toReturn = hash(c);
	FreeBoard(b);
	FreeBoard(c);

	return toReturn;
}


POSITION maskBoard(POSITION p, short mask) {

	QTBPtr b = unhash(p);
	short i;
	short pad;
	POSITION toReturn;

	pad = maskseq(GAMEDIMENSION);
	for (i=0; i<BOARDSIZE+1; i++) {
		if (b->slots[i] != EMPTYSLOT) {
			b->slots[i] ^= mask;
			b->slots[i] &= pad;
		}
	}

	toReturn = hash(b);
	FreeBoard(b);

	return toReturn;
}

void yanpeiTestRotate();
void yanpeiTestReflect();
void yanpeiTestMask();

void yanpeiTestCanonicalSupport() {
	printf("\nTesting support functions for GetCanonicalPosition()\n");
	yanpeiTestRotate();
	yanpeiTestReflect();
	yanpeiTestMask();
}

void yanpeiTestRotate() {

	short i;
	QTBPtr b = MallocBoard();
	POSITION p,q;
	void (*oldPrintPos)(POSITION position, STRING playersName, BOOLEAN usersTurn ) = printPos;

	printf("\nTesting rotatePosition()\n");

	printPos = &yanpeiPrintSlots;

	b->usersTurn = FALSE;
	b->squaresOccupied = (NUMPIECES<BOARDSIZE) ? NUMPIECES : BOARDSIZE-1;
	b->piecesInPlay = (NUMPIECES<BOARDSIZE) ? NUMPIECES : BOARDSIZE-2;
	for (i=0; i<BOARDSIZE; i++) {
		b->slots[i] = (i<NUMPIECES) ? i : EMPTYSLOT;
	}
	b->slots[BOARDSIZE] = EMPTYSLOT;

	p = hash(b);
	q = p;
	PrintPosition(q,"",TRUE);
	printf("Rotating board ...\n");
	q = rotatePosition90(q);
	PrintPosition(q,"",TRUE);
	printf("Rotating board ...\n");
	q = rotatePosition90(q);
	PrintPosition(q,"",TRUE);
	printf("Rotating board ...\n");
	q = rotatePosition90(q);
	PrintPosition(q,"",TRUE);
	printf("Rotating board ...\n");
	q = rotatePosition90(q);
	PrintPosition(q,"",TRUE);

	if (p == q) printf("\n ... testRotate() passed.\n");
	else printf("\n ... testRotate() failed.\n");

	printPos = oldPrintPos;
	FreeBoard(b);

}


void yanpeiTestReflect() {

	short i;
	QTBPtr b = MallocBoard();
	POSITION p,q;
	void (*oldPrintPos)(POSITION position, STRING playersName, BOOLEAN usersTurn ) = printPos;

	printf("\nTesting reflectPosition()\n");

	printPos = &yanpeiPrintSlots;

	b->usersTurn = FALSE;
	b->squaresOccupied = (NUMPIECES<BOARDSIZE) ? NUMPIECES-1 : BOARDSIZE-1;
	b->piecesInPlay = (NUMPIECES<BOARDSIZE) ? NUMPIECES : BOARDSIZE-1;
	for (i=0; i<BOARDSIZE; i++) {
		b->slots[i] = (i<NUMPIECES) ? i : EMPTYSLOT;
	}
	b->slots[BOARDSIZE] = EMPTYSLOT;

	p = hash(b);
	q = p;
	PrintPosition(q,"",TRUE);
	printf("Reflecting board ...\n");
	q = reflectPosition(q);
	PrintPosition(q,"",TRUE);
	printf("Reflecting board ...\n");
	q = reflectPosition(q);
	PrintPosition(q,"",TRUE);

	if (p == q) printf("\n ... testReflect() passed.\n");
	else printf("\n ... testReflect() failed.\n");

	printPos = oldPrintPos;
	FreeBoard(b);

}

void yanpeiTestMask() {

	short i;
	short mask = 1;
	QTBPtr b = MallocBoard();
	POSITION p,q;
	void (*oldPrintPos)(POSITION position, STRING playersName, BOOLEAN usersTurn ) = printPos;

	printf("\nTesting maskBoard()\n");

	printPos = &yanpeiPrintSlots;

	b->usersTurn = FALSE;
	b->squaresOccupied = (NUMPIECES<BOARDSIZE) ? NUMPIECES-1 : BOARDSIZE-1;
	b->piecesInPlay = (NUMPIECES<BOARDSIZE) ? NUMPIECES : BOARDSIZE-1;
	for (i=0; i<BOARDSIZE; i++) {
		b->slots[i] = (i<NUMPIECES) ? i : EMPTYSLOT;
	}
	b->slots[BOARDSIZE] = EMPTYSLOT;

	p = hash(b);
	q = p;
	PrintPosition(q,"",TRUE);
	printf("Masking board with %d ...\n",mask);
	q = maskBoard(q,mask);
	PrintPosition(q,"",TRUE);
	printf("Masking board with %d ...\n",mask);
	q = maskBoard(q,mask);
	PrintPosition(q,"",TRUE);

	if (p == q) printf("\n ... testMask() passed.\n");
	else printf("\n ... testMask() failed.\n");

	printPos = oldPrintPos;
	FreeBoard(b);

}

void yanpeiTestCanonical() {

	POSITION i,c;
	POSITION canonicalCount = 0;
	POSITIONLIST *canonicals = NULL;

	void (*oldPrintPos)(POSITION position, STRING playersName, BOOLEAN usersTurn ) = printPos;

	printf("\nTesting getCanonicalPosition() ... \n");

	printPos = &yanpeiPrintSlots;

	for (i=0; i<offsetTable[NUMPIECES+1]; i++) {
		printf("Position = %3llu\t\t",i);
		PrintPosition(i,"",TRUE);
		c = getCanonical(i);
		printf("Canonical = %3llu\t",c);
		PrintPosition(c,"",TRUE);
		/*
		   if (!SearchPosList(canonicals,c)) {
		   canonicalCount++;
		   newNode = SafeMalloc(sizeof(POSITIONLIST));
		   newNode->position = c;
		   if (canonicals!=NULL) {
		   newNode->next = canonicals->next;
		   canonicals->next = newNode;
		   } else {
		   newNode->next = NULL;
		   canonicals = newNode;
		   }
		   }
		 */
	}

	printf("\n ... " POSITION_FORMAT " canonical positions in total.\n",canonicalCount);

	FreePosList(canonicals);
	printPos = oldPrintPos;
}

BOOLEAN SearchPosList(POSITIONLIST *l, POSITION p) {
	if (l==NULL) {
		return FALSE;
	} else {
		return ((l->position==p) || SearchPosList(l->next,p));
	}
}

void FreePosList(POSITIONLIST *l) {
	if (l!=NULL) {
		if (l->next!=NULL) {
			FreePosList(l->next);
		}
		SafeFree(l);
	}
}

char readchar( ) {

	while( getchar( ) != '\n' ) ;
	return getchar( );

}



/** Benjamin Sussman Jumps In!!
**		Here follows Ben's attempt at the Teirification of Quarto!
**		TeirGamesman function and variable declarations:
**/

/* Tier = Arangement of non-symetric pieces placed on the board. Teir Zero
 * is the first TWO moves
 * (The choice of the piece to give to the opponent and then the placement
 * of said piece and the choice of the next.)
 */
TIER BoardToTier(QTBPtr board) {
	TIER thistier = 0;
	int x;
	for(x = 0; x < BOARDSIZE; x++) {
		if(board->slots[x+1] != EMPTYSLOT) //0 is the hand slot, so skip it. Go from 1-16
			thistier = thistier | (1 << board->slots[x+1]);
	}
	return thistier;
}

//Small helper to go through a tier and return the number of empty slots (number of off bits)
int getPiecesLeft(TIER tier) {
	int x;
	int numBlankSlots = 0;
	for(x = 0; x < NUMPIECES; x++) {
		if (((tier >> x) & 1) == 0) numBlankSlots++;
	}
	return numBlankSlots;
}

TIERLIST* TierChildren(TIER tier) {
	TIERLIST* tierlist = NULL;
	int x;
	if (tier == 0)
		tierlist = CreateTierlistNode(tier, tierlist);
	for (x = 0; x < NUMPIECES; x++) {
		if (!((tier >> x) & 1))
			tierlist = CreateTierlistNode((tier | (1 << x)),tierlist);
	}
	return tierlist;
}

TIERPOSITION NumberOfTierPositions(TIER tier){
	generic_hash_context_switch(tier);
	TIERPOSITION numPos;
	if (tier == 0)
		numPos = generic_hash_max_pos() * (NUMPIECES + 1);
	else if (getPiecesLeft(tier)==0)
		numPos = generic_hash_max_pos();
	else numPos = generic_hash_max_pos() * getPiecesLeft(tier);
	return numPos;
}

void GetInitialTierPosition(TIER* tier, TIERPOSITION* tierposition) {

}


/**Important Constants to remember:
 * BOARDSIZE, EMPTYSLOT, QTBPtr, QTBOARD
 */
void SetupTierStuff() {
	// kSupportsTierGamesman
	kSupportsTierGamesman = TRUE;
	// All function pointers
	gTierChildrenFunPtr                             = &TierChildren;
	gNumberOfTierPositionsFunPtr    = &NumberOfTierPositions;
	gGetInitialTierPositionFunPtr   = &GetInitialTierPosition;
	//gIsLegalFunPtr				    = &IsLegal;
	//gGenerateUndoMovesToTierFunPtr= &GenerateUndoMovesToTier; Unnecessary, but faster!
	//gUnDoMoveFunPtr				= &UnDoMove;
	gTierToStringFunPtr                             = &TierToString;
	generic_hash_destroy();
	// Tier-Specific Hashes: BENNO ATTACK!!!
	int piecesArray[(NUMPIECES * 3) + 4];
	int pieceCounter;
	TIER tierCounter;
	for(pieceCounter = 0; pieceCounter < NUMPIECES; pieceCounter++) {
		piecesArray[pieceCounter*3] = pieceCounter;
		piecesArray[pieceCounter*3 + 1] = piecesArray[pieceCounter*3 + 2] = 0;
	}
	piecesArray[NUMPIECES*3] = EMPTYSLOT;
	piecesArray[NUMPIECES*3 + 1] = piecesArray[NUMPIECES*3 + 2] = 0;
	piecesArray[NUMPIECES*3 + 3] = -1;
	// Now the piecesArray is INITIALIZED!! Wooohoo!

	// Ok, prepare to generic_hash_init 2^16 Times! One for each set of on-board pieces
	TIER numberOfTiers = 1 << NUMPIECES; /*This is 2^16*/
	int piecesLeft, piecesOnBoard;
	for(tierCounter = 0; tierCounter < numberOfTiers; tierCounter++) {
		for(pieceCounter = 0; pieceCounter < NUMPIECES; pieceCounter++) {
			/**Ok, what's going on here is that the tier Counter is iterating through the
			   2^NUMPIECES possibly configurations, where each bit represents whether or not
			   some piece is present. This makes things easier to code (life1D status)**/
			piecesArray[pieceCounter*3 + 1] = (tierCounter >> pieceCounter) & 1;
			piecesArray[pieceCounter*3 + 2] = (tierCounter >> pieceCounter) & 1;
		}
		piecesLeft = getPiecesLeft(tierCounter);
		piecesOnBoard = NUMPIECES - piecesLeft;
		piecesArray[NUMPIECES*3 + 1] = piecesArray[NUMPIECES*3 + 2] = piecesLeft+(BOARDSIZE - NUMPIECES);
		generic_hash_init(BOARDSIZE, piecesArray, NULL, ((piecesOnBoard == 0) ? 0 : ((piecesOnBoard & 1) ? 1 : 2)));
	}
	gInitialTier = 0;
	generic_hash_context_switch(gInitialTier);
	QTBPtr board = MallocBoard();
	int x;
	for(x = 0; x <= BOARDSIZE; x++)
		board->slots[x] = EMPTYSLOT;
	board->squaresOccupied = 0;
	board->piecesInPlay = 0;
	board->usersTurn = FALSE;

	char* hashBoard = consCharArrayFromBoard(board);
	TIERPOSITION tierpos = generic_hash_hash(hashBoard, 1);
	SafeFree(hashBoard);
	gInitialTierPosition = tierpos + (generic_hash_max_pos()*board->slots[0]);
	if(board != NULL)
		FreeBoard(board);
}

POSITION TierHash(QTBPtr board) {
	POSITION position;
	if (gHashWindowInitialized) { //What the hell does this mean???
		TIER tier = BoardToTier(board); // find this board's tier
		generic_hash_context_switch(tier); // switch to that context
		char* hashBoard = consCharArrayFromBoard(board);
		TIERPOSITION tierpos = generic_hash_hash(hashBoard, ((board->usersTurn) ? 2 : 1));
		SafeFree(hashBoard);
		/*Confusing addition up ahead: the plan is to NOT use generichash for the
		   hand piece. So whatever GenHash returns, we add the maximum positions for
		   that hash times the piece in hand (plus one so we never add zero if the hand
		   is zero). This way the generic_hash value is salvagable as the new value modulo
		   the max_positions. The hand piece is salvagable and the quotient of the new
		   value over the max positions. (how many max positions fit). WHAT ABOUT IF THE
		   HASH IS EXACTLY MAX_POS?!?! SPECIAL CASE!?!??!?!?! OR USE MAX_POS+1??
		 */
		if(tier == 0)
			tierpos += generic_hash_max_pos()*board->slots[0]; //If in initial tier, let everything thru
		else if(getPiecesLeft(tier) != 0) { //If we're NOT in the final tier, encode the hand
			int pieceOffset = 0;
			int piece;
			for (piece = 0; piece < NUMPIECES; piece++) {
				if ((tier >> piece) & 1)
					continue;
				else if (board->slots[0] == piece)
					break;
				pieceOffset++;
			}
			tierpos += generic_hash_max_pos()*pieceOffset;
		} // If we're IN the final tier, the hand is for sure empty so leave tierpos be
		position = gHashToWindowPosition(tierpos, tier); //gets TIERPOS, find POS
		//printf("HASH: tierpos: %llu, Tier: %llu, position: %llu\n", tierpos, tier, position);
	} else position = hashUnsymQuarto(board);
	return position;
}


QTBPtr TierUnhash(POSITION position) {
	QTBPtr board = (QTBPtr) MallocBoard();
	if (gHashWindowInitialized) {
		TIERPOSITION tierpos; TIER tier;
		gUnhashToTierPosition(position, &tierpos, &tier); // get tierpos
		generic_hash_context_switch(tier); // switch to that tier's context
		//printf("UNHASH: tierpos: %llu, Tier: %llu, position: %llu\n", tierpos, tier, position);
		int encodedHand = tierpos / generic_hash_max_pos(); // I need this to be TRUNCATED!
		tierpos = tierpos % generic_hash_max_pos();
		if(tier == 0) board->slots[0] = encodedHand;
		else if (getPiecesLeft(tier) == 0) board->slots[0] = EMPTYSLOT;
		else {
			int piece;
			for(piece = 0; piece < NUMPIECES; piece++) {
				if((tier>>piece) & 1)
					continue;
				else if (encodedHand == 0) {
					board->slots[0] = piece;
					break;
				} else encodedHand--;
			}
		}
		char * hashBoard = (char*)SafeMalloc(NUMPIECES*sizeof(char));
		generic_hash_unhash(tierpos, hashBoard); // unhash in that tier
		int x;
		for(x=0; x<BOARDSIZE; x++)
			board->slots[x+1] = hashBoard[x];
		SafeFree(hashBoard);
		setPiecesAndSquares(board);
		return board;
	} else return unhashUnsymQuarto(position);
}

void setPiecesAndSquares(QTBPtr board) {
	int piecesinplay = 0;
	int squaresoccupied = 0;
	int x;
	if (board->slots[0] != EMPTYSLOT) piecesinplay++;
	for(x=1; x<=BOARDSIZE; x++) {
		if(board->slots[x] != EMPTYSLOT) {
			piecesinplay++;
			squaresoccupied++;
		}
	}
	board->piecesInPlay = piecesinplay;
	board->squaresOccupied = squaresoccupied;
	board->usersTurn = (piecesinplay & 1) ? TRUE : FALSE;
}


char * consCharArrayFromBoard(QTBPtr board) {
	char * charArray = (char*)SafeMalloc(BOARDSIZE*sizeof(char));
	int x;
	for(x=0; x<BOARDSIZE; x++) {
		charArray[x] = board->slots[x+1];
	}
	return charArray;
}

STRING TierToString(TIER tier) {
	STRING thisTier = (STRING)SafeMalloc((NUMPIECES+1)* sizeof(char));
	int x;
	for (x = 0; x < NUMPIECES; x++) {
		if((tier >> x) & 1)
			thisTier[x] = '1';
		else thisTier[x] = '0';
	}
	thisTier[NUMPIECES] = '\0';
	return thisTier;
}
/*
   BOOLEAN IsLegal(POSITION position) {
    QTBPtr board = gUseGPS ? GPSBoard : unhash( position );
    BOOLEAN returnVal = TRUE;

    if(BoardToTier(board) != 0) {
        if(getPiecesLeft(BoardToTier(board)) == 0)
            returnVal = (board->slots[0] == EMPTYSLOT);
        else if (board->slots[0] == EMPTYSLOT)
            returnVal = FALSE;
        else {
            int x;
            for(x=0;x<BOARDSIZE;x++) {
                if(board->slots[x+1] == board->slots[0]) {
                    returnVal = FALSE;
                    break;
                }
            }
        }
    }
    if (!gUseGPS)
        FreeBoard(board);
    return returnVal;
   }
 */
// $Log: not supported by cvs2svn $
// Revision 1.74  2008/02/28 03:04:34  ethanr
// Options team implemented options on mquarto
//
// Revision 1.73  2007/11/19 03:40:23  ddgarcia
// Rearranged the functions since my gcc is a one-pass compiler;
// also, I have to add -std=c99 to get mquarto.c to compile. I guess I
// need to do some configure.ac hacking to get that put in... :( -dan
//
// Revision 1.72  2007/05/07 03:05:28  max817
// IT WORKS
//
// Revision 1.71  2007/05/07 01:30:50  max817
// Quarto works, and fixed a display bug with hash efficiency.
//
// Revision 1.70  2007/05/02 18:06:40  bensussman
// BUGZID:
// Fixed a small bug, but not the one which prevents a solve from occuring. That's still busted.
//
// Revision 1.69  2007/04/25 04:58:50  max817
// Tiers
//
// Revision 1.68  2007/04/23 09:14:46  bensussman
// BUGZID: 69770
//
// Benno here. I finished my code that i feel like i am currently able to do. It appears that the bug i have is that the tierification core does not think there are any boards in each tier. It thinks they are empty for some reason. Strange...
//
// Revision 1.67  2007/04/11 04:09:18  rodarmor
// Unborked the build.
//
// Revision 1.66  2007/04/09 22:33:38  max817
// Notes from out talk.
//
// Revision 1.65  2007/04/07 22:31:08  bensussman
// BUGZID:666
// This is for Yanpei
// Only method that is "finished" is setupTierStuff()
//
// Revision 1.64  2006/12/30 04:11:01  yanpeichen
// fixed some minor typos
//
// Revision 1.63  2006/12/19 20:00:51  arabani
// Added Memwatch (memory debugging library) to gamesman. Use 'make memdebug' to compile with Memwatch
//
// Revision 1.62  2006/08/21 23:49:09  dmchan
// commented out calls to fflush(stdin) and changed to GetMyInt() and GetMyChar() where appropriate
//
// Revision 1.61  2006/02/03 06:08:39  hevanm
// fixed warnings. I will leave the real bugs to retro hehehehe.
//
// Revision 1.60  2006/01/28 10:40:51  yanpeichen
//  28 Jan 2006 Yanpei: Canonicals fully debugged. Mario and Yanpei's canonicals
//                      cross checked and mutually agrees. Algorithms timed and
//                      compared. Mario's is MUCH MUCH faster.
//
// Revision 1.59  2006/01/17 08:01:51  yanpeichen
// yanpei chen: generic canonicals coded. under testing. need to ask mario
// about matching his results.
//
// Revision 1.58  2006/01/12 02:12:36  mtanev
//
// Fix super-duper-ridiculous bug of declaring temp position of type int in getCanonical
//
// Revision 1.57  2006/01/11 03:02:45  mtanev
//
// Eliminate make warnings
//
// Revision 1.56  2005/12/27 10:57:50  hevanm
// almost eliminated the existance of gDatabase in all files, with some declarations commented earlier that need to be hunt down and deleted from the source file.
//
// Revision 1.55  2005/12/23 05:13:27  mtanev
//
// Initial version of alpha-beta solver, very experimental and very buggy
//
// Revision 1.54  2005/12/05 11:14:00  hevanm
// Fixed up the memory database to write to a file when instructed. Moved around some constants and #defines to contants.h, and added globals.c to initialize the global variables. This breaks all other databases, the fixing of which I (and hopefully Mario too, if he is not too angry about my breaking his stuff) will follow up in the near future.
//
// Revision 1.53  2005/11/21 04:14:23  neyiah
// changed piece representation to use {0,1}, added help strings.
//
// Revision 1.52  2005/11/13 11:45:06  mtanev
//
// Fix memory leaks
//
// Revision 1.51  2005/11/13 11:00:41  mtanev
//
// add 1 to gNumberOfPositions, otherwise it fails for 4D
//
// Revision 1.50  2005/11/13 10:54:08  mtanev
//
// Ahh, don't memory leak on getCanonical
//
// Revision 1.49  2005/11/12 22:06:06  mtanev
//
// Added normalize function to properly normalize board.
//
// Revision 1.48  2005/10/10 08:09:57  neyiah
// Fixed bug where generatemoves() generates move to place piece in hand slot when hand is already occupied
//
// Revision 1.47  2005/09/19 05:29:48  yanpeichen
// 18 Sep 2005 Yanpei Chen changing mquarto.c
//
// Bug in setOffset() fixed. Now solves for 3-D as well.
//
// Revision 1.46  2005/09/15 04:34:32  ogren
// capitalized CVS keywords
//
POSITION StringToPosition(char* board) {
	// FIXME: this is just a stub
	return atoi(board);
}


char* PositionToString(POSITION pos) {
	// FIXME: this is just a stub
	return "Implement Me";
}

STRING MoveToString(MOVE theMove) {
	return "Implement MoveToString";
}

char * PositionToEndData(POSITION pos) {
	return NULL;
}
