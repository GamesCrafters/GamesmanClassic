// $id$
// $log$

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
** 11 Feb 2005 Yanpei: added Cindy's name to author's list
**                     added hashQuarto(), hashQuartoHelper(), setFactorialTable(),
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
**                     and first and last board indices in slot array (BOARD_FIRST, BOARD_LAST)
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

#define	DEBUG

/*************************************************************************
**
** Game-specific constants
**
**************************************************************************/

STRING   kGameName            = "QUARTO"; /* The name of your game */
STRING   kAuthorName          = "Yanpei CHEN, Amy HSUEH, Mario TANEV"; /* Your name(s) */
STRING   kDBName              = ""; /* The name to store the database under */

BOOLEAN  kPartizan            = FALSE ; /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN  kGameSpecificMenu    = FALSE ; /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN  kTieIsPossible       = TRUE ; /* TRUE if a tie is possible. FALSE if it is impossible.*/
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

/* Basic error codes */
#define ERROR	1
#define SUCCESS	0

/* Squares x */
#define square(x) ((x)*(x))

/* Creates sequence n least significant 1 bits, preceeded by 0 bits */
#define maskseq(n) ~(~0<<(n))

#define GAMEDIMENSION 2
#define BOARDSIZE square(GAMEDIMENSION)
#define NUMPIECES (1 << GAMEDIMENSION)

#define PIECESTATES (BOARDSIZE+1)
#define EMPTYSLOT 0

#if BOARDSIZE<NUMPIECES
    #define FACTORIALMAX (NUMPIECES+1)
#else 
    #define FACTORIALMAX (BOARDSIZE+1)
#endif

#define HAND			0
#define EMPTY 			0
#define BOARD_FIRST		1
#define BOARD_LAST		BOARDSIZE+1

typedef struct board_item {

  short slots[BOARDSIZE+1]; // to record the 0 to NUMPIECES-1 pieces contained in each slot
                            // slots[1-16] = board squares, slots[0] = next piece
                            // 0 to NUMPIECES-1 encode the pieces, EMPTYSLOT encodes an empty slot
  short squaresOccupied;    // number of squares occupied
  short piecesInPlay;       // number of pieces in play
  BOOLEAN usersTurn;        // whose turn it is

} QTBOARD;

typedef QTBOARD* QTBPtr;

/* Letter codes for the different piece states */
char states[][2]={{'w', 'B'}, {'s', 'T'}, {'h', 'S'}, {'r', 'E'}};

/* ASCII Hex */
char hex_ascii[] = { 'H', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

/*************************************************************************
**
** Global Variables
**
*************************************************************************/

static BOOLEAN factorialTableSet =  FALSE;
static POSITION factorialTable[FACTORIALMAX];
static BOOLEAN offsetTableSet = FALSE;
static POSITION offsetTable[NUMPIECES+1];

/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/

/* External */
extern GENERIC_PTR	SafeMalloc ();
extern void		SafeFree ();
void* 			gGameSpecificTclInit = NULL;  /* newly added to tempalte */

/* External */
void			setOffsetTable();
POSITION		factorial( int n );
POSITION		combination(int n, int r);
POSITION		permutation(int n, int r);
POSITION		hashUnsymQuartoHelper(QTBPtr b, int baseSlot);
void			unhashUnsymQuartoHelper(POSITION p, int baseSlot, QTBPtr toReturn);
POSITION		qhash( QTBPtr );	// Incomplete hash, if someone is done with the real hash, remove this

/* Implementation of the set of hash functions using a simple and wasteful bitpacking algorithm */
POSITION		packhash( QTBPtr );
QTBPtr			packunhash( POSITION );

/* Since we may switch hash implementations, here are function pointers to be set in choosing implementation */
POSITION		(*hash)( QTBPtr ) = &packhash;
QTBPtr			(*unhash)( POSITION ) = &packunhash;

BOOLEAN			boards_equal ( QTBPtr, QTBPtr );
void			print_board( QTBPtr );
QTBPtr			TestHash( QTBPtr, int );


MOVE CreateMove( MOVE slot, MOVE piece );
MOVE GetMovePiece( MOVE move );
MOVE GetMoveSlot( MOVE move );

void PrintCell( void *cell, char (*CellContent)( short, void * ) );
void PrintBoard( void *cells, size_t content_size, char *heading, char (*CellContent)( short, void * ) );
char LegendCoordinate( short pad, void *p_coordinate );
char PieceTrait( short trait, void *p_piece );
void PrintHorizontalBorder( char fill, char border, char *startmark, char *endmark );

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
	
	QTBPtr board = (QTBPtr) SafeMalloc( sizeof ( QTBOARD ) );
	QTBPtr error_board;
	
	/* Initialize board to empty */
	memset( board, 0, sizeof( QTBOARD ) );

	/* Set initial position to empty board */
	gInitialPosition = hash( board );
	
	/* Test hasher on board, should print error if mismatch */
	error_board = TestHash( board, 0 );
	
	if ( error_board ) {

		POSITION p = hash( board );

		fprintf( stderr, "Hashing error:\nboard\t\t" );
		print_board( board );
		fprintf( stderr, "\nhashes to\t%lu\nunhashes to\t", p );
		print_board( unhash( p ) );
		fprintf( stderr, "\n" );
	
	}

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
    
	QTBPtr board;
	MOVELIST *moves	= NULL;
	MOVE slot;
    
	/* Unhash board */
	board	= unhash( position );
	
	/* If there's piece in hand, the only valid moves are the ones placing the piece in board slot */
	if( board->slots[HAND] != EMPTY ) {
		
		/* For each slot on board */
		for( slot = BOARD_FIRST; slot < BOARD_LAST; slot++ ) {
			
			/* If slot is empty */
			if ( board->slots[slot] == EMPTY ) {
				
				/* Add move which moves piece from hand into empty slot */
				moves	= CreateMovelistNode( CreateMove( slot, board->slots[HAND] ), moves );
				
			}
			
		}
	
	/* If there's no piece in hand, the valid moves are the ones placing an available piece into hand */
	} else {
		
		BOOLEAN available_pieces[NUMPIECES];
		MOVE piece;
		
		/* Initialize array of available pieces */
		memset( available_pieces, TRUE, sizeof( *available_pieces ) * NUMPIECES );
		
		/* For each slot on board */
		for( slot = BOARD_FIRST; slot < BOARD_LAST; slot++ ) {
			
			/* If slot is not empty */
			if( board->slots[slot] != EMPTY ) {
				
				/* Remove piece from array of available pieces */
				available_pieces[board->slots[slot] - BOARD_FIRST] = FALSE;
				
			}
			
		}
		
		/* For each piece */
		for( piece = 0; piece < NUMPIECES; piece++ ) {
			
			/* If piece is available */
			if( available_pieces[piece] != FALSE ) {
		
				/* Add move which moves piece into hand */
				moves	= CreateMovelistNode( CreateMove( HAND, piece + BOARD_FIRST ), moves );
				
			}
			
		}
		
	}
	
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
		
	/* Determine slot on board piece is to go into */
	slot					= GetMoveSlot( move );
	
	/* Determine which piece is to go into board slot */
	piece					= GetMovePiece( move );
	
	/* Unhash board */
	board					= unhash( position );
	
	/* Place piece into slot */
	board->slots[slot]		= piece;
	
	/* If move places piece into hand */
	if ( slot == HAND ) {
		
		/* Change player */
		board->usersTurn	= !board->usersTurn;

	/* Otherwise, if there's piece in hand */
	} else if ( board->slots[HAND] != EMPTY ) {
		
		/* Take piece out of hand */
		board->slots[HAND]	= EMPTY;
		
	}
	
	/* Return hashed board */
	return hash( board );
	
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
    QTBPtr b = unhash(position);
    short *rowColDiag = (short *) SafeMalloc(GAMEDIMENSION*sizeof(short));
    BOOLEAN primitiveFound = FALSE;
    BOOLEAN emptyFound = FALSE;
    int i,j;

    printf("**** Primitive() ****\n");
    printf("**** The slots are: ****");
    for (i=0; i<BOARDSIZE+1; i++) {
      printf("%d,",b->slots[i]);
    }
    printf("\n**** Primitive() ****\n");

    // checking the ranks/files in one direction
    i=0;
    while (!primitiveFound && i<GAMEDIMENSION) {
      j=0;
      while (!emptyFound && j<GAMEDIMENSION) {
	if (b->slots[i*GAMEDIMENSION+j+1] == EMPTYSLOT) {
	  emptyFound = TRUE;
	}
	rowColDiag[j] = b->slots[i*GAMEDIMENSION+j+1] - 1;
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
	rowColDiag[j] = b->slots[j*GAMEDIMENSION+i+1] - 1;
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
      rowColDiag[i] = b->slots[i*GAMEDIMENSION+i+1] - 1;
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
      rowColDiag[i] = b->slots[(i+1)*GAMEDIMENSION-i] - 1;
      i++;
    }
    if (!emptyFound) primitiveFound = searchPrimitive(rowColDiag);
    emptyFound = FALSE;

    SafeFree(rowColDiag);

    // returning stuff
    if (primitiveFound && b->usersTurn) {
      return lose;
    } else if (primitiveFound && !b->usersTurn) {
      return win;
    } else {
      return undecided;
    }

}

// helper function called by Primitive()
// precondition: all elements of rowColDiag != EMPTYSLOT
BOOLEAN searchPrimitive(short *rowColDiag) {

  short inverterMask = NUMPIECES-1;
  short noninvertedResult = rowColDiag[0];
  short invertedResult = inverterMask ^ rowColDiag[0];
  short i;

  printf("**** searchPrimitive() examines ");
  for (i=0; i<GAMEDIMENSION; i++) {
    printf("%d,",rowColDiag[i]);
  }
  printf(" ****\n");


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

	
void PrintPosition ( POSITION position, STRING playersName, BOOLEAN usersTurn )
{

	QTBPtr board;
	
	/* Unhash position into internal board representation */
	board = unhash( position );
	
	/* Print legend for board */
	PrintBoard( hex_ascii, sizeof( *hex_ascii ), "LEGEND:", &LegendCoordinate );
	
	/* Print actual board */
	PrintBoard( board->slots, sizeof( *board->slots ), "BOARD: ", &PieceTrait );
	

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
	
	int slot, piece, trait;
	
	/* Determine piece information */
	piece	= GetMovePiece( move ) - 1;
	
	/* Determine slot information */
	slot	= GetMoveSlot( move );
	
	/* For each piece trait */
	for( trait = 0; trait < GAMEDIMENSION; trait++ ) {
		
		/* Print the corresponding character describing its state */
		printf( "%c", states[trait][(piece >> trait) & 1] );
				
	}
	
	/* Print character describing position on board/hand */
	printf( ":%c", hex_ascii[slot] );
	
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
		/* REMOVE COMMENT OF FOLLOWING LINE TO BREAK EVERYTHING */
		/*printf("blabla");*/
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

BOOLEAN ValidTextInput( STRING input )
{
	
	BOOLEAN valid = FALSE;
	
	if ( ( strlen( input ) == ( 2 + GAMEDIMENSION ) ) && input[GAMEDIMENSION] == ':' ) {
		
		int i;
		int valid_traits[GAMEDIMENSION];
		
		memset( valid_traits, 0, GAMEDIMENSION * sizeof( int ) );		
		
		// Checking if position indicated is valid
		for( i = 0; i < BOARDSIZE + 1; i++ ) {
			
			if( input[GAMEDIMENSION+1] == hex_ascii[i] ) {
				
				valid = TRUE;
				break;
				
			}
			
		}
		
		if ( valid ) {
		   
			int trait;
			
			for( trait = 0; trait < GAMEDIMENSION; trait++ ) {
				for( i = 0; i < GAMEDIMENSION; i++ ) {
					if ( input[trait] == states[i][0] || input[trait] == states[i][1] ) {
						BOOLEAN repeat = FALSE;
						int j;
						
						for( j = 0; j < trait; j++ ) {
							if ( valid_traits[j] == i + 1 ) {
								repeat = TRUE;
							}
						}
						if ( !repeat ) {
							valid_traits[trait] = i + 1;
							break;
						}
					}
				}
				
				if ( valid_traits[trait] == 0 ) {
					
					valid = FALSE;
					break;
					
				}
			
			}
			
			
		}
		
	} 
	
	printf("Move does%s adhere to valid syntax\n", valid ? "" : " not" );
	return valid;
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
	int i, j, k;
	
	/* Lower GAMEDIMENSION + 1 bits for position */
	for( slot = 0; ( slot < BOARDSIZE + 1 ) && hex_ascii[slot] != input[GAMEDIMENSION + 1]; slot++ );
	
	/* Adjacent GAMEDIMENSION bits for piece */
	for( i = 0; i < GAMEDIMENSION; i++ ) {
		
		BOOLEAN ready = FALSE;
		
		for( j = 0; j < GAMEDIMENSION; j++ ) {
			
			for( k = 0; k < 2; k++ ) {
				
				if( states[j][k] == input[i] ) {
				
					piece += k << j;
					ready = TRUE;
					break;
					
				}
				
			}
			
			if ( ready ) {
				
				break;
				
			}
			
		}
		
	}

	return CreateMove( slot, piece + BOARD_FIRST );

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

/* Mario: prints board (for internal use)*/
void print_board( QTBPtr board ) {

	int slot;
	
	fprintf( stderr, "[" );
	for ( slot = 0; slot < BOARDSIZE + 1; slot++ ) {
		
		fprintf( stderr, " %d ", board->slots[slot] );

	}
	fprintf( stderr, "] pieces = %d; squares = %d; turn = %d ", board->piecesInPlay, board->squaresOccupied, board->usersTurn );

}

/*Mario: Hash Tester, should work with any hash implementation */
QTBPtr TestHash( QTBPtr board, int slot ) {
	
	int i;

	for( i = 0; i < NUMPIECES + 1; i++ ) {

		int j, repeat = FALSE, pieces = 0, squares = 0;;
		
		/* For every slot preceeding this one */
		for( j = 0; j < slot; j++ ) {

			if( board->slots[j] != 0 ) {

				/* Invalid board */
				if( i == board->slots[j] ) {

					repeat = TRUE;
					break;

				} else {

					if( j != 0 ) {

						squares++;

					}

					pieces++;

				}
			}

		}

		if ( !repeat ) {

			board->slots[slot] = i;
			if ( slot < BOARDSIZE ) {

				QTBPtr error_board = TestHash( board, slot + 1 );

				if ( error_board ) {

					return error_board;

				}

			} else {

				if ( i != 0 ) {

					pieces++;
					if ( slot != 0 ) {

						squares++;

					}

				}

				board->piecesInPlay = pieces;
				board->squaresOccupied = squares;
				board->usersTurn = FALSE;

				if ( !boards_equal( board, unhash( hash ( board ) ) ) ) {

					return board;

				} else {

					return NULL;

				}
				

			}

		}
			
	}

	return NULL;

}

/* Mario: This hash algorithm is wasteful. We just bitpack the board into an integer.
          GAMEDIMENSION+1 bits for each square and hand, squaresOccupied
          and piecesInPlay and 1 bit for usersTurn.
          This will fail to pack for large game dimensions as POSITION is just a long. */
POSITION packhash( QTBPtr board ) {

	POSITION hash = 0;
	int slot, shift;

	for( slot = 0, shift = 0; slot < BOARDSIZE + 1; slot++, shift += GAMEDIMENSION + 1 ) {

		hash += board->slots[slot] << shift;

	}

	hash += board->piecesInPlay << shift;
	hash += board->squaresOccupied << ( shift += GAMEDIMENSION + 1 );
	hash += board->usersTurn << ( shift += GAMEDIMENSION + 1 );

	return hash;

}

QTBPtr packunhash( POSITION hash ) {

	QTBPtr board = (QTBPtr) SafeMalloc( sizeof( QTBOARD ) );
	int slot, shift, mask = maskseq( GAMEDIMENSION + 1 );

	for( slot = 0, shift = 0; slot < BOARDSIZE + 1; slot++, shift += GAMEDIMENSION + 1 ) {

		board->slots[slot] = ( hash >> shift ) & mask;

	}

	board->piecesInPlay = ( hash >> shift ) & mask;
	board->squaresOccupied = ( hash >> ( shift += GAMEDIMENSION + 1 ) ) & mask;
	board->usersTurn = ( hash >> ( shift += GAMEDIMENSION + 1 ) ) & 1;

	return board;

}

/* Mario: non-gap hash function, INCOMPLETE!!! */
POSITION qhash( QTBPtr board ) {

	int pieces, stage;
        POSITION base = 0;

	/* stage is the number of pieces in play, with an added 1 if the number of pieces in play
	   is the same as the number of pieces on the board (i.e. full board) */
	stage	= board->piecesInPlay + ( board->piecesInPlay == board->squaresOccupied );

	for ( pieces = stage - 1; pieces >= 0; pieces-- ) {

		/* the base offset contains (NUMPIECES order pieces) * (BOARDSIZE order pieces - 1)
		   for each pieces less than stage */
		base += permutation( NUMPIECES, pieces ) * combination( BOARDSIZE, pieces - 1 );

	}

	return base;

}
/*
// hashing an internally represented QTBOARD into a POSITION
POSITION hashUnsymQuarto(QTBPtr b) {

  POSITION toReturn;
  QTBPtr helperBoard = (QTBPtr) SafeMalloc(sizeof(QTBOARD));

  if (!offsetTableSet) setOffsetTable();

  if (b->squaresOccupied==0 && b->piecesInPlay==0) {
    toReturn = 0;
  } else if (b->squaresOccupied==0 && b->piecesInPlay==1) {
    toReturn = b->slots[0] + offsetTable[b->squaresOccupied];
  } else {
    toReturn = b->slots[0]*permutation(NUMPIECES,b->squaresOccupied)
                          *combination(BOARDSIZE,b->squaresOccupied)
               + hashUnsymQuartoHelper(b, 1) 
               + offsetTable[b->squaresOccupied];
  }

  if (b->usersTurn) {
    toReturn = toReturn + offsetTable[NUMPIECES];
  }

  return toReturn;

}

POSITION hashUnsymQuartoHelper(QTBPtr b, int baseSlot) {

  int i;
  int numOccSubset = 0; // # of occupied slots starting from baseSlot
  int numNotOccSubset;  // # of empty slots starting from baseSlot
  int numSlotSubset = BOARDSIZE+1 - baseSlot;
                        // # of total slots starting from baseSlot
  POSITION toReturn;
  int pieces[b->squaresOccupied];  // Array to store the pieces occupying each square
                               // accending order indexed by slots starting from baseSlot
  int squares[b->squaresOccupied]; // indices of non-empty squares
                               // accending order indexed by slots starting from baseSlot

  // setting up local vars
  localB->squaresOccupied = b->squaresOccupied;
  localB->piecesInPlay = b->piecesInPlay;
  localB->usersTurn = b->usersTurn;
  for (i=0; i<BOARDSIZE+1; i++) {
    localB->slots[i] = b->slots[i];
    if ((i >= baseSlot) && (b->slots[i] != NUMPIECES)) {
      if ((b->slots[i] > b->slots[0]) && (b->slots[i] != NUMPIECES)) {
	pieces[numOccSubset] = b->slots[i] - 1;
      } else {
	pieces[numOccSubset] = b->slots[i];
      }
      squares[numOccSubset] = i;
      numOccSubset++;
    }
  }
  numNotOccSubset = numSlotSubset - numOccSubset;

  if (numOccSubset == 0) {
    toReturn = 0;
  } else if (numOccSubset == 1) {
    toReturn = (pieces[0]*(numSlotSubset) + squares[0] - baseSlot);
  } else {
    int j;
    for (j=0; j<numOccSubset; j++) {
      if (localB->slots[squares[j]] > localB->slots[squares[0]]) {
	localB->slots[squares[j]]--;
      }
    }
    toReturn = (pieces[0]*permutation(NUMPIECES,numOccSubset-1)
		         *combination(BOARDSIZE-baseSlot+1,numOccSubset-1)
	        + hashUnsymQuartoHelper(localB,baseSlot+1));
  }

  free(localB);
  return toReturn;
}

// unhashing a POSITION into an internally represented QTBOARD
QTBPtr unhashUnsymQuarto(POSITION p) {

  QTBPtr toReturn = (QTBPtr) SafeMalloc(sizeof(QTBOARD));
  int i;

  if (!offsetTableSet) setOffsetTable();

  if (p >= offsetTable[NUMPIECES]) {
    toReturn->usersTurn = TRUE;
    p -= offsetTable[NUMPIECES];
  } else {
    toReturn->usersTurn = FALSE;
  }

  for (i=0; i<BOARDSIZE+1; i++) {
    toReturn->slots[i] = NUMPIECES; // to encode empty squares
  }
  // all empty square at first, to be filled up w/ pieces

  if (p == 0) {
    toReturn->squaresOccupied = 0;
    toReturn->piecesInPlay = 0;
  } else if (p < offsetTable[NUMPIECES]) {
    for (i=1; i<NUMPIECES; i++) {
      if (p>=offsetTable[i-1] && p<offsetTable[i]) {
	toReturn->squaresOccupied = i-1;
	toReturn->piecesInPlay = i;
	toReturn->slots[0] = (p - offsetTable[i-1]) / 
	                       (permutation(NUMPIECES,toReturn->squaresOccupied)
                                *combination(BOARDSIZE,toReturn->squaresOccupied));
	/* Mario: I am not sure what this code does, but I had to modify this to get it compile 
	/* Please diff for details 
	unhashUnsymQuartoHelper(p-offsetTable[i-1] -
				  toReturn->slots[0]
				  *permutation(NUMPIECES,toReturn->squaresOccupied)
				  *combination(BOARDSIZE,toReturn->squaresOccupied),
				1,
				toReturn);
      }
    }
  } else {
    printf("unhashUnsymQuarto() -- p out of range: %lu\n",p);
    toReturn = NULL;
  }

  return toReturn;

}

void unhashUnsymQuartoHelper(POSITION p, int baseSlot, QTBPtr toReturn) {

  int i;
  int numUnhashed = 0;
  int numRemaining;
  int numSlotSubset = BOARDSIZE - baseSlot;
  int nextSquare,nextPiece;

  for (i=0; i<baseSlot; i++) {
    if (toReturn->slots[i] != NUMPIECES) {
      numUnhashed++;
    }
  }
  numRemaining = toReturn->piecesInPlay - numUnhashed + 1;

  if (numRemaining == 0) {
    // we're done!
  } else if (numRemaining == 1) {
    nextPiece = p / numSlotSubset;
    nextSquare = p - nextPiece*numSlotSubset + 1;
    toReturn->slots[nextSquare] = nextPiece;
  } else {

  }
}
*/

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

#ifndef DEBUG

POSITION factorial(int n) {
  if (n<=0) {
    return 1;
  } else if (FACTORIALMAX<=n) {
    return factorialTable[FACTORIALMAX-1];
  } else {
    return factorialTable[n];
  }
}*/

#else
/* Non-memoizing factorial */
POSITION factorial( int n ) {

	POSITION result = 1;

	while ( n > 0 ) {

		result*=n--;

	}

	return result;

}
#endif

// returns n P r, the number of ordered arrangements of 
// r items selected from a set of n items
POSITION permutation(int n, int r) {
  if (!factorialTableSet) setFactorialTable();
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
  if (!factorialTableSet) setFactorialTable();
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
    for (i=1; i<NUMPIECES; i++) {
      offsetTable[i] = offsetTable[i-1] + 
	               permutation(NUMPIECES,i)*combination(BOARDSIZE,i)*(NUMPIECES-1);
    }
    offsetTable[NUMPIECES] = offsetTable[NUMPIECES-1] + factorial(NUMPIECES);
    offsetTableSet = TRUE;
  }
}


/* Creates move given slot and piece */
MOVE CreateMove( MOVE slot, MOVE piece ) {
	
	return slot + ( piece << ( GAMEDIMENSION + 1 ) );
	
	
}

/* Returns piece given move */
MOVE GetMovePiece( MOVE move ) {
	
	return move >> ( GAMEDIMENSION + 1 );
	
}

/* Returns slot given move */
MOVE GetMoveSlot( MOVE move ) {
	
	return move & maskseq( GAMEDIMENSION + 1 );
	
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
	
	return ( piece == 0 ) ? ' ' : states[trait][( ( piece - 1 ) >> trait ) & 1];
	
}

char LegendCoordinate( short pad, void *p_coordinate ) {
	
	char coordinate = *((char *) p_coordinate);
	
	return ( pad < GAMEDIMENSION - 1 ) ? ' ' : coordinate;
	
}

/* Prints cell for board */
void PrintCell( void *cell, char (*CellContent)( short, void * ) ) {
	
	short trait;
	
	for( trait = 0; trait < GAMEDIMENSION; trait++ ) {
	
		printf( "%c", CellContent( trait, cell ) );	
		
	}
	
	printf( "|" );
	
}

/* Prints board */
void PrintBoard( void *cells, size_t content_size, char *heading, char (*CellContent)( short, void * ) ) {
	
	const int header_length = 20;
	int i, cell, heading_length, hand_label_length;
	char *pad = "\n       ";
	char *hand_label = " Hand ";
	
	heading_length		= strlen( heading );
	hand_label_length	= strlen( hand_label );
	
	for ( i = 0; i < header_length; i++ ) {
		
		printf( " " );
		
	}
	
	printf( "|" );
	for ( i = 0; i < hand_label_length; i++ ) {
		
		printf( "^" );
		
	}
	printf( "|" );
	for ( i = 0; i < GAMEDIMENSION; i++ ) {
		
		printf( "^" );
		
	}
	printf( "|" );
	
	printf( "\n%s", heading );
	for ( i = 0; i < header_length - heading_length; i++ ) {
		
		printf( " " );
		
	}
	
	printf( "|%s|", hand_label );
	PrintCell( cells, CellContent );
	printf( "\n" );
	
	for ( i = 0; i < header_length; i++ ) {
		
		printf( " " );
		
	}
	
	printf( "|" );
	for ( i = 0; i < hand_label_length; i++ ) {
		
		printf( "_" );
		
	}
	printf( "|" );
	for ( i = 0; i < GAMEDIMENSION; i++ ) {
		
		printf( "_" );
		
	}
	printf( "|" );

	for ( i = 0; i < header_length; i++ ) {
		
		printf( " " );
		
	}
	
	for( cell = 1; cell <= BOARDSIZE; cell++ ) {
		
		if ( !( ( cell - 1 ) % GAMEDIMENSION ) ) {
			PrintHorizontalBorder( '-', ' ', pad, "" );
			printf( "%s|", pad );
			
		}
		
		PrintCell( cells + cell*content_size, CellContent );
		
	}
	
	PrintHorizontalBorder( '-', ' ', pad, "\n" );

}
