

/*
 * The above lines will include the name and log of the last person
 * to commit this file to CVS
 */

/************************************************************************
**
** NAME:        mtopitop.c
**
** DESCRIPTION: Topitop
**
** AUTHOR:      Mike Hamada and Alex Choy
**
** DATE:        BEGIN: 02/20/2006
**	              END: ???
**
** UPDATE HIST:
**	
**	    02/20/2006 - Setup #defines & data-structs
** 			 		 Wrote InitializeGame(), PrintPosition()
**      02/22/2006 - Added CharToBoardPiece(), arrayHash(), and arrayUnhash()
**                   Still need to edit above functions with these new fcts
**                   Need to make arrayHash() a for loop
**	    02/26/2006 - Fixed errors that prevented game from being built
**			 		 Edited InitializeGame(), PrintPosition() to use new hashes
** 			 		 Fixed struct for board representation
** 					 Changed PrintPosition() since Extended-ASCII does not work
**			 		 Changed arrayHash() to use a for-loop to calculate hash
**			 		 Wrote Primitive() (unsure if it is finished or not)
**      02/26/2006 - Not sure if total num of positions should include player 
**                   turn (0 and 1 at msg, most significant digit)
**                   Wrote GetInitialPosition()
**                   For reference, MOVE = int, POSITION = int (from core/type.h)
**                   Wrote DoMove(), didn't test yet
**                   A move is represented using same hash/unhash as board, but
**                   only has the moved piece (if has one) and the new piece
**                       
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

STRING   kGameName            = "Topitop"; /* The name of your game */
/*STRING	 kGameName			  =
"\n\n\
TTTTTTTTTTT    OOOOOO    PPPPPPPP  III  TTTTTTTTTTT    OOOOOO    PPPPPPPP\n\
TTTTTTTTTTT   OOOOOOOO   PPPPPPPP  III  TTTTTTTTTTT   OOOOOOOO   PPPPPPPP\n\
   TTTT      OOO    OOO  PP    PP  III     TTTT      OOO    OOO  PP    PP\n\
   TTTT      OOO    OOO  PPPPPPPP  III     TTTT      OOO    OOO  PPPPPPPP\n\
   TTTT       OOOOOOOO   PP        III     TTTT       OOOOOOOO   PP\n\
   TTTT        OOOOOO    PP        III     TTTT        OOOOOO    PP\n\n";*/
    
STRING   kAuthorName          = "Alex Choy and Mike Hamada"; /* Your name(s) */
STRING   kDBName              = "topitop"; /* The name to store the database under */

BOOLEAN  kPartizan            = TRUE ; /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN  kGameSpecificMenu    = TRUE ; /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN  kTieIsPossible       = FALSE ; /* TRUE if a tie is possible. FALSE if it is impossible.*/
BOOLEAN  kLoopy               = TRUE ; /* TRUE if the game tree will have cycles (a rearranger style game). FALSE if it does not.*/

BOOLEAN  kDebugMenu           = TRUE ; /* TRUE only when debugging. FALSE when on release. */
BOOLEAN  kDebugDetermineValue = FALSE ; /* TRUE only when debugging. FALSE when on release. */

POSITION gNumberOfPositions   =  747521822; /* The number of total possible positions | If you are using our hash, this is given by the hash_init() function*/
POSITION gInitialPosition     =  0; /* The initial hashed position for your starting board */
POSITION kBadPosition         = -1; /* A position that will never be used */

void*	 gGameSpecificTclInit = NULL;

/* 
 * Help strings that are pretty self-explanatory
 * Strings than span more than one line should have backslashes (\) at the end of the line.
 */

STRING kHelpGraphicInterface =
"Not written yet.";

STRING   kHelpTextInterface    =
"Note that a player CANNOT undo an oppontent's move that was just made.\n\
Use the LEGEND to determine which numbers to choose to\n\
correspond to either a piece to place and a board slot where to place it\n\
OR to the location of your piece or a neutral piece and the empty\n\
adjacent position you wish to move that piece to.\n\
Example: 's1' would place a Small Sand Pile in position 1\n\
Example: '12' would move a piece in position 1 to position 2.";

STRING   kHelpOnYourTurn =
"Each player takes turns making one valid move, also noting\n\
that player CANNOT reverse an opponent's move that was just made.\n\
Use the LEGEND to determine which numbers to choose to\n\
correspond to either a piece to place and a board slot to place it\n\
OR to the location of your piece or a neutral piece and the empty\n\
adjacent position you wish to move that piece to.\n\
\n\
VALID MOVES:\n\
1.) Place 1 of your Buckets or 1 of the Small or Big Sand Piles\n\
    on any free space of the board.\n\
2.) Move 1 of your Buckets or any Small or Big Sand Pile already\n\
    placed on the board from one square to another (one space at\n\
    a time in any direction).\n\
       a.) Any piece already on the board can be moved to an \n\
           adjacent free space.\n\
       b.) A Bucket can go on top of a Small Sand Pile.\n\
       c.) A Small Sand Pile can go on top of a Big Sand Pile.\n\
       d.) A Small Sand Pile with a Bucket on its top can go on\n\
           top of a Big Sand Pile.\n\
       e.) A Bucket can go on top of a Sand Castle.\n\
    NOTE: You CANNOT place a piece on top of another piece.\n\
          Pieces must be MOVED on top of other pieces.\n\
3.) Move any combination of Sand Piles with your Buckets on top,\n\
    or any Sand Castle, to any free space.";

STRING   kHelpStandardObjective =
"Be the first to have your 2 Buckets on top of 2 Sand Castles,\n\
where a Sand Castle consists of 1 Small Sand Pile put on top\n\
of 1 Big Sand Pile.";

STRING   kHelpReverseObjective =
"";	/* There exists no reverse objective */

STRING   kHelpTieOccursWhen =
"A tie never occurs.";

STRING   kHelpExample =
"";


/*************************************************************************
**
** #defines and structs
**
**************************************************************************/

#define ROWCOUNT 3
#define COLCOUNT 3

#define BLANKPIECE '_'
#define SMALLPIECE 's'
#define LARGEPIECE 'l'
#define CASTLEPIECE 'c'
#define BLUEBUCKETPIECE 'b'
#define REDBUCKETPIECE 'r'
#define BLUESMALLPIECE 'X'
#define REDSMALLPIECE 'O'
#define BLUECASTLEPIECE 'B'
#define REDCASTLEPIECE 'R'
#define UNKNOWNPIECE '0'  // hopefully none of these b/c can't be represented by a digit from 0 - 9
#define UNKNOWNBOARDPIECE -1

#define BLANKSTRING "Blank"
#define SMALLSTRING "Small Sand Pile"
#define LARGESTRING "Large Sand Pile"
#define CASTLESTRING "Sand Castle"
#define BLUEBUCKETSTRING "Blue Bucket"
#define REDBUCKETSTRING "Red Bucket"
#define BLUESMALLSTRING "Blue Small Sand Pile"
#define REDSMALLSTRING "Red Small Sand Pile"
#define BLUECASTLESTRING "Blue Sand Castle"
#define REDCASTLESTRING "Red Sand Castle"
#define UNKNOWNSTRING "UNKNOWN PIECE!"

#define HASHBLANK 0
#define HASHSANDPILE 1
#define HASHBLUEBUCKET 1
#define HASHREDBUCKET 2

#define BLUETURN 0
#define REDTURN 1

#define NUMCASTLESTOWIN 2

typedef enum possibleBoardPieces {
    Blank = 0, SmallSand, LargeSand, SandCastle, BlueBucket, 
    RedBucket, BlueSmall, RedSmall, BlueCastle, RedCastle
} BoardPiece;

typedef enum hashBoardPieces {
	hSmallSand = 0, hLargeSand, hBlueBucket, hRedBucket, hSandCastle,
	hBlueSmall, hRedSmall, hBlueCastle, hRedCastle
} HashBoardPiece;

typedef enum playerTurn {
	Blue = 1, Red
} PlayerTurn;

typedef struct boardAndTurnRep {
  char *theBoard;
  PlayerTurn theTurn;
} *BoardAndTurn;

typedef struct tripleBoardRep {
	char *boardL;	// Holds blanks (0) and large sand piles (1)
	char *boardS;	// Holds blanks (0) and small sand piles (2)
	char *boardB;	// Holds blanks (0), blue buckets (1), and red buckets (2).
} *BoardRep;

typedef struct threePieces {
	char L;
	char S;
	char B;
} *ThreePiece;

typedef struct cleanMove {
	int fromPos;	// 0 = dart-board ; 1-9 = move from position
	int toPos;		// 0-8 = move to position
	HashBoardPiece movePiece;
} *GMove;

/*************************************************************************
**
** Global Variables
**
*************************************************************************/

int boardSize = ROWCOUNT * COLCOUNT;
int numCols = COLCOUNT;
int numRows = ROWCOUNT;
int DEBUG_G = 0;
int DEBUG_GM = 0;
int DEBUG_M = 0;
int DEBUG_PP = 0;
int DEBUG_AU = 1;
int DEBUG_UM = 0;
int DEBUG_PM = 0;
int DEBUG_DM = 0;
int DEBUG_CTITM = 1;
int DEBUG_VPM = 0;
int DEBUG_IM = 0;
int DEBUG_GAPPM = 0;
int gameType;

int smallSandPiles = 4, largeSandPiles = 4, redBuckets = 2, blueBuckets = 2;
int redCastles = 0, blueCastles = 0;
int maxL, maxS, maxB = 0;
PlayerTurn gWhosTurn = Blue;
POSITION currentBoard = -1;		//If currentBoard = -1, has not been initialized
MOVE lastMove = -1;		//If lastMove = -1, there has been no last move


/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/

/* External */
extern GENERIC_PTR		SafeMalloc ();
extern void				SafeFree ();
extern POSITION         generic_hash_init(int boardsize, int pieces_array[], int (*vcfg_function_ptr)(int* cfg));
extern POSITION         generic_hash(char *board, int player);
extern char             *generic_unhash(POSITION hash_number, char *empty_board);
extern int              whoseMove (POSITION hashed);
/* Internal */
void                    InitializeGame();
MOVELIST                *GenerateMoves(POSITION position);
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
char					BoardPieceToChar(BoardPiece piece);
char					HashBoardPieceToChar(HashBoardPiece piece);
BoardPiece 	        	CharToBoardPiece(char piece);
HashBoardPiece 			CharToHashBoardPiece(char piece);
BoardPiece 				ThreePieceToBoardPiece(ThreePiece lsb);
char 					ThreePieceToChar(ThreePiece lsb);
ThreePiece 				BoardPieceToThreePiece(BoardPiece piece);
ThreePiece 				CharToThreePiece(char piece);
char* 					BoardPieceToString(BoardPiece piece);
char* 					HashBoardPieceToString(HashBoardPiece piece);
POSITION				arrayHash(BoardAndTurn board);
BoardAndTurn			arrayUnhash(POSITION hashNumber);
MOVE					hashMove(GMove newMove);
GMove					unhashMove(MOVE newMove);
MOVE 					inverseMove(MOVE move);
void 					printMove(GMove move);
int						validPieceMove(int fromPos, int toPos);


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
    int LpiecesArray[] = { HASHBLANK, 5, 9, HASHSANDPILE, 0, 4, -1 };
    int SpiecesArray[] = { HASHBLANK, 5, 9, HASHSANDPILE, 0, 4, -1 };
    int BpiecesArray[] = { HASHBLANK, 5, 9, HASHREDBUCKET, 0, 2, HASHBLUEBUCKET, 0, 2, -1 };
    
    BoardAndTurn boardArray;
    boardArray = (BoardAndTurn) SafeMalloc(sizeof(struct boardAndTurnRep));
    boardArray->theBoard = (char *) SafeMalloc(boardSize * sizeof(char));
    
    if (DEBUG_G) { printf("maxL = %d\n", maxL = generic_hash_init(boardSize, LpiecesArray, NULL)); }
    if (DEBUG_G) { printf("maxS = %d\n", maxS = generic_hash_init(boardSize, SpiecesArray, NULL)); }
    if (DEBUG_G) { printf("maxB = %d\n", maxB = generic_hash_init(boardSize, BpiecesArray, NULL)); }
    
    maxL = generic_hash_init(boardSize, LpiecesArray, NULL);
    maxS = generic_hash_init(boardSize, SpiecesArray, NULL);
    maxB = generic_hash_init(boardSize, BpiecesArray, NULL);
    
    gNumberOfPositions = maxB + maxS * maxB + maxL * maxS * maxB;
    gWhosTurn = boardArray->theTurn = Blue;
    
    for (i = 0; i < boardSize; i++) {
    	boardArray->theBoard[i] = BLANKPIECE;
    }
    
    gInitialPosition = currentBoard = arrayHash(boardArray);
    SafeFree(boardArray->theBoard);
    SafeFree(boardArray);
    if (DEBUG_G) { printf("# Of Pos: %d\n", (int) gNumberOfPositions); }
    if (DEBUG_G) { printf("Init Pos: %d\n", (int) gInitialPosition); }
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
    MOVELIST *CreateMovelistNode();
    BoardAndTurn board = arrayUnhash(position);
    GMove newMove = (GMove) SafeMalloc(sizeof(struct cleanMove));
    MOVE tempMove, undoMove = inverseMove(lastMove);
    int i, j;
   	
    if (DEBUG_GM) { printf("\n***** GENERATE MOVES *****\n\n"); }
    
    newMove->fromPos = 0;
    if (DEBUG_GM) { printf("---- Place Moves ----\n"); }
    for (i = 0; i < boardSize; i++) {
    	if (board->theBoard[i] == BLANKPIECE) {
	    	newMove->toPos = i;
    		if (smallSandPiles > 0) {
    			newMove->movePiece = hSmallSand;
    			tempMove = hashMove(newMove);
    			if (DEBUG_GM) { 
    				printf("NEWMOVE %d %x\n", (int) tempMove, (int) tempMove);
    				printMove(newMove);
				printf("\n");
				printMove(unhashMove(tempMove));
				printf("\n");
    			}
    			moves = CreateMovelistNode(tempMove, moves);
	    	}
    		if (largeSandPiles > 0) {
    			newMove->movePiece = hLargeSand;
    			tempMove = hashMove(newMove);
    			if (DEBUG_GM) { 
    				printf("NEWMOVE %d %x\n", (int) tempMove, (int) tempMove);
    				printMove(newMove);
				printf("\n");
				printMove(unhashMove(tempMove));
				printf("\n");
			}
    			moves = CreateMovelistNode(tempMove, moves);
	    	}
	    	if ((board->theTurn == Blue) && (blueBuckets > 0)) {
	    		newMove->movePiece = hBlueBucket;
	    		tempMove = hashMove(newMove);
	    		if (DEBUG_GM) { 
    				printf("NEWMOVE %d %x\n", (int) tempMove, (int) tempMove); 
    				printMove(newMove);
				printf("\n");
				printMove(unhashMove(tempMove));
				printf("\n");
			}
			moves = CreateMovelistNode(tempMove, moves);
	    	}
	    	if ((board->theTurn == Red) && (redBuckets > 0)) {
	    		newMove->movePiece = hRedBucket;
	    		tempMove = hashMove(newMove);
	    		if (DEBUG_GM) { 
    				printf("NEWMOVE %d %x\n", (int) tempMove, (int) tempMove); 
    				printMove(newMove);
				printf("\n");
				printMove(unhashMove(tempMove));
				printf("\n");
			}
			moves = CreateMovelistNode(tempMove, moves);
	    	}
    	}
    }
    
    if (DEBUG_GM) { printf("---- Piece Moves ----\n"); }
    
    for (i = 0; i < boardSize; i++) {
    	for (j = 0; j < boardSize; j++) {
	    	newMove->fromPos = i + 1;
	    	newMove->toPos = j;
    		newMove->movePiece = CharToHashBoardPiece(board->theBoard[i]);
    		if ((((int) (tempMove = hashMove(newMove))) != ((int) undoMove)) && 
    			(validPieceMove(i, j))) {
			
    			if (((board->theBoard[i] == BLUEBUCKETPIECE) && (board->theTurn == Blue)) || 
    	 			((board->theBoard[i] == REDBUCKETPIECE) && (board->theTurn == Red))) {
    				if ((board->theBoard[j] == BLANKPIECE) || (board->theBoard[j] == SMALLPIECE) ||
    					(board->theBoard[j] == CASTLEPIECE)) {
    					if (DEBUG_GM) { 
    						printf("NEWMOVE %d %x\n", (int) tempMove, (int) tempMove);
    						printMove(newMove);
						printf("\n");
						printMove(unhashMove(tempMove));
						printf("\n");
					}
					moves = CreateMovelistNode(tempMove, moves);
    				}
    	 		} else if (((board->theBoard[i] == BLUESMALLPIECE) && (board->theTurn == Blue)) || 
    	 			((board->theBoard[i] == REDSMALLPIECE) && (board->theTurn == Red)) ||
    	 			(board->theBoard[i] == SMALLPIECE)) {
    				if ((board->theBoard[j] == BLANKPIECE) || (board->theBoard[j] == LARGEPIECE)) {
    					if (DEBUG_GM) { 
    						printf("NEWMOVE %d %x\n", (int) tempMove, (int) tempMove); 
    						printMove(newMove);
						printf("\n");
						printMove(unhashMove(tempMove));
						printf("\n");
					}
					moves = CreateMovelistNode(tempMove, moves);
    				}
    	 		} else if (((board->theBoard[i] == BLUECASTLEPIECE) && (board->theTurn == Blue)) || 
    	 			((board->theBoard[i] == REDCASTLEPIECE) && (board->theTurn == Red)) ||
    	 			(board->theBoard[i] == LARGEPIECE) || (board->theBoard[i] == CASTLEPIECE)) {
    				if (board->theBoard[j] == BLANKPIECE) {
    					if (DEBUG_GM) { 
    						printf("NEWMOVE %d %x\n", (int) tempMove, (int) tempMove); 
    						printMove(newMove);
						printf("\n");
						printMove(unhashMove(tempMove));
						printf("\n");
					}
					moves = CreateMovelistNode(tempMove, moves);
    				}
    	 		}
    		}
    	}
    }
        
    SafeFree(newMove);
    SafeFree(board->theBoard);
    SafeFree(board);
    
    if (DEBUG_GM) { printf("\n***** END GENERATE MOVES *****\n"); }
    
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

POSITION DoMove (POSITION position, MOVE move) {
 	char pieceToMove, pieceInWay;
	POSITION newPosition = 0;
	BoardAndTurn board = arrayUnhash(position);
	GMove newMove = unhashMove(move);
	lastMove = move;
  
  	if (DEBUG_DM) { printf("\n***** DO MOVE *****\n\n"); }
  	
  	if (DEBUG_DM) {
  		printf("POSITION# = %d\n", (int) position);
  		printf("MOVE# = %d\n", (int) move);
  		printMove(newMove);
  		printf("\n");
  	}
  
  	if (newMove->fromPos == 0) {
  		if (DEBUG_DM) { printf("--- PLACE PIECE ---\n"); }
  		if (DEBUG_DM) { printf("Piece To Place = %c\n", HashBoardPieceToChar(newMove->movePiece)); }
  		board->theBoard[newMove->toPos] = HashBoardPieceToChar(newMove->movePiece);
  		if (board->theBoard[newMove->toPos] == SMALLPIECE) {
  			smallSandPiles--;
  		} else if (board->theBoard[newMove->toPos] == LARGEPIECE) {
  			largeSandPiles--;
  		} else if (board->theBoard[newMove->toPos] == REDBUCKETPIECE) {
  			redBuckets--;
  		} else if (board->theBoard[newMove->toPos] == BLUEBUCKETPIECE) {
  			blueBuckets--;
  		}
  	} else {
  		if (DEBUG_DM) { printf("--- MOVE PIECE ---\n"); }
  		pieceToMove = board->theBoard[newMove->fromPos - 1];
  		pieceInWay = board->theBoard[newMove->toPos];
  		if (DEBUG_DM) { printf("Piece To Move = %c\n", pieceToMove); }
  		if (DEBUG_DM) { printf("Piece In Way = %c\n", pieceInWay); }
  		board->theBoard[newMove->fromPos - 1] = BLANKPIECE;

  		if (pieceInWay == BLANKPIECE) {
  			board->theBoard[newMove->toPos] = pieceToMove;
  		} else if (pieceInWay == SMALLPIECE) {
  			if (pieceToMove == BLUEBUCKETPIECE) {
  				board->theBoard[newMove->toPos] = BLUESMALLPIECE;
  			} else if (pieceToMove == REDBUCKETPIECE) {
  				board->theBoard[newMove->toPos] = REDSMALLPIECE;
  			}
  		} else if (pieceInWay == LARGEPIECE) {
  			if (pieceToMove == SMALLPIECE) {
  				board->theBoard[newMove->toPos] = CASTLEPIECE;
  			} else if (pieceToMove == BLUESMALLPIECE) {
  				board->theBoard[newMove->toPos] = BLUECASTLEPIECE;
  				blueCastles++;
  			} else if (pieceToMove == REDSMALLPIECE) {
  				board->theBoard[newMove->toPos] = REDCASTLEPIECE;
  				redCastles++;
  			}
  		} else if (pieceInWay == CASTLEPIECE) {
  			if (pieceToMove == BLUEBUCKETPIECE) {
  				board->theBoard[newMove->toPos] = BLUECASTLEPIECE;
  				blueCastles++;
  			} else if (pieceToMove == REDBUCKETPIECE) {
  				board->theBoard[newMove->toPos] = REDCASTLEPIECE;
  				redCastles++;
  			}
  		}
  	}
  
	if (board->theTurn == Blue) {
		board->theTurn = gWhosTurn = Red;
	} else {
		board->theTurn = gWhosTurn = Blue;
	}
	
	newPosition = currentBoard = arrayHash(board);
	
	if (DEBUG_DM) { printf("NEXT BOARD# = %d\n", (int) newPosition); }
	
	SafeFree(board->theBoard);
    SafeFree(board);
    SafeFree(newMove);
    
    if (DEBUG_DM) { printf("\n***** END DO MOVE *****\n"); }

	return newPosition;
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

VALUE Primitive (POSITION position) {
    int i, blueCastlesMade = 0, redCastlesMade = 0;
    BoardAndTurn board;
    
    board = arrayUnhash(position);
    
    for (i = 0; i < boardSize; i++) {
    	if (board->theBoard[i] == BLUECASTLEPIECE) {
    		blueCastlesMade++;
    	} else if (board->theBoard[i] == REDCASTLEPIECE) {
    		redCastlesMade++;
    	}
    }
    
    if (((board->theTurn == Blue) && (blueCastlesMade >= NUMCASTLESTOWIN)) ||
    	((board->theTurn == Red) && (redCastlesMade >= NUMCASTLESTOWIN))) {
    	return win;
    } else if (((board->theTurn == Blue) && (redCastlesMade >= NUMCASTLESTOWIN)) ||
   		((board->theTurn == Red) && (blueCastlesMade >= NUMCASTLESTOWIN))) {
   		return lose;
    }
   	
   	SafeFree(board->theBoard);
   	SafeFree(board);
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
	BoardAndTurn arrayHashedBoard;
 	int i;
  
	arrayHashedBoard = arrayUnhash(position);
	
	if (DEBUG_PP) { printf("\nPOSITION# = %d\n", (int) position); }
	
	/***********************LINE 1**************************/
	printf("\n\n\n       *-*-*-*");
	printf("                     ");
	printf("%c = Blank\n", BLANKPIECE);
	
	/*printf("       %c%c%c%c%c%c%c\n", BOLD_UL_CORNER, BOLD_HOR, BOLD_HOR_DOWN, BOLD_HOR,
									BOLD_HOR_DOWN, BOLD_HOR, BOLD_UR_CORNER);*/
	/***********************LINE 2**************************/
	printf("       |");
	for (i = 0; i < numCols; i++) {
		printf("%c|", arrayHashedBoard->theBoard[i]);
	}
	
	/*printf("       %c", BOLD_VERT);
	for (i = 0; i < numCols; i++) {
		printf("%c%c", BoardPieceToChar(arrayHashedBoard->theBoard[i]), 
										((i == (numCols-1)) ? VERT_LINE : BOLD_VERT));
	}*/
	printf("          ( 1 2 3 )");
	printf("  %c = Small Sand Pile\n", SMALLPIECE);
	/***********************LINE 3**************************/
	printf("       *-+-+-*");
	printf("                     ");
	printf("%c = Large Sand Pile\n", LARGEPIECE);
	
	/*printf("       %c%c%c%c%c%c%c\n", BOLD_VERT_RIGHT, HOR_LINE, CROSS_LINE, HOR_LINE,
										CROSS_LINE, HOR_LINE, BOLD_VERT_LEFT);*/							
	/***********************LINE 4**************************/
	printf("BOARD: |");
	for (i = numCols; i < (numCols*2); i++) {
		printf("%c|", arrayHashedBoard->theBoard[i]);
	}
	
	/*printf("BOARD: %c", BOLD_VERT);
	for (i = numCols; i < (numCols*2); i++) {
		printf("%c%c", BoardPieceToChar(arrayHashedBoard->theBoard[i]),
										((i == ((numCols*2)-1)) ? VERT_LINE : BOLD_VERT));
	}*/
	printf("  LEGEND: ( 4 5 6 )");
	printf("  %c = Sand Castle\n", CASTLEPIECE);
	/***********************LINE 5**************************/
	printf("       *-+-+-*");
	printf("                     ");
	printf("%c = Blue Bucket\n", BLUEBUCKETPIECE);
	
	/*printf("       %c%c%c%c%c%c%c\n", BOLD_VERT_RIGHT, HOR_LINE, CROSS_LINE, HOR_LINE,
										CROSS_LINE, HOR_LINE, BOLD_VERT_LEFT);*/
	/***********************LINE 6**************************/
	printf("       |");
	for (i = numCols*2; i < (numCols*3); i++) {
		printf("%c|", arrayHashedBoard->theBoard[i]);
	}
	
	/*printf("       %c", BOLD_VERT);
	for (i = numCols*2; i < (numCols*3); i++) {
		printf("%c%c", BoardPieceToChar(arrayHashedBoard->theBoard[i]),
										((i == ((numCols*3)-1)) ? VERT_LINE : BOLD_VERT));
	}*/
	printf("          ( 7 8 9 )");
	printf("  %c = Red Bucket\n", REDBUCKETPIECE);
	/***********************LINE 7**************************/
	printf("       *-*-*-*");
	printf("                     ");
	printf("%c = Blue Small Piece\n", BLUESMALLPIECE);
	
	/*printf("       %c%c%c%c%c%c%c\n", BOLD_LL_CORNER, BOLD_HOR, BOLD_HOR_UP, BOLD_HOR,
									BOLD_HOR_UP, BOLD_HOR, BOLD_LR_CORNER);*/
	/***********************LINE 8, 9, 10, 11**************************/
	printf("                                   ");
	printf("%c = Red Small Piece\n", REDSMALLPIECE);
	printf("# Remaining:                       ");
	printf("%c = Blue Sand Castle\n", BLUECASTLEPIECE);
	printf("  Large = %d  |  Small = %d          ", largeSandPiles, smallSandPiles);
	printf("%c = Red Sand Castle\n", REDCASTLEPIECE);
	printf("  RedB  = %d  |  BlueB = %d\n\n\n", redBuckets, blueBuckets);
	//printf("# Remaining: Large = %d | Small = %d | RedB = %d | BlueB = %d\n\n",
		//	largeSandPiles, smallSandPiles, redBuckets, blueBuckets);
	//printf("\n%s\n\n", GetPrediction(position, playerName, usersTurn));
	SafeFree(arrayHashedBoard->theBoard);
	SafeFree(arrayHashedBoard);
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
    printf("%s's move: ", computersName);
    PrintMove(computersMove);
  	printf("\n");
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
	if ( DEBUG_PM ) { printf("\n***** PRINT MOVE *****\n\n"); }
    GMove newMove = unhashMove(move);
    char toPrint;
    //char* pieceString = HashBoardPieceToString(newMove->movePiece);
    
    if (newMove->fromPos == 0) {
    	//printf("Placed %s at Position %d\n", pieceString, newMove->toPos);
    	if ((newMove->movePiece == hBlueBucket) || (newMove->movePiece == hRedBucket)) {
    		toPrint = 'b';
    	} else if (newMove->movePiece == hSmallSand) {
    		toPrint = 's';
    	} else if (newMove->movePiece == hLargeSand) {
    		toPrint = 'l';
    	} else {
    		toPrint = 'x';
    	}
    	printf("%c%d", toPrint, newMove->toPos + 1);
    } else {
    	//printf("Moved %s from Position %d to %d\n", pieceString, newMove->fromPos, newMove->toPos);
    	printf("%d%d", newMove->fromPos, newMove->toPos + 1);
    }
    
    SafeFree(newMove);
    //SafeFree(pieceString);
    
    if ( DEBUG_PM ) { printf("\n***** END PRINT MOVE *****\n"); }
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
    int numSlots = numCols*numRows;
    char* playerColor;
    
    BoardAndTurn board = arrayUnhash(position);

    if (board->theTurn == Blue) {
      playerColor = BLUEBUCKETSTRING;
    }
    else {
      playerColor = REDBUCKETSTRING;
    }

    if (DEBUG_GAPPM) { printf("theTurn = %d\n", board->theTurn); }
    
    for (;;) {
        /***********************************************************
         * CHANGE THE LINE BELOW TO MATCH YOUR MOVE FORMAT
         ***********************************************************/
                printf(" move key: l = large sand pile, s = small sand pile, b = %s\n", playerColor);
		printf("%8s's move [(u)ndo/([l,s,b][1-%d] OR [1-%d][1-%d])] : ", 
		       playersName, numSlots, numSlots, numSlots);
	
		input = HandleDefaultTextInput(position, move, playersName);
		
		if ((input == Undo) && (position != gInitialPosition)) {
			/*smallSandPiles = largeSandPiles = 4;
    		blueBuckets = redBuckets = 2;
    		for (i = 0; i < boardSize; i++) {
    			switch (board->theBoard[i]) {
    				case SMALLPIECE:		smallSandPiles--;
    										break;
		    		case LARGEPIECE:		largeSandPiles--;
    										break;
		    		case CASTLEPIECE:		smallSandPiles--;
    										largeSandPiles--;
    										break;
		    		case BLUEBUCKETPIECE:	blueBuckets--;
    										break;
		    		case REDBUCKETPIECE:	redBuckets--;
    										break;
		    		case BLUESMALLPIECE:	smallSandPiles--;
    										blueBuckets--;
    										break;
		    		case REDSMALLPIECE:		smallSandPiles--;
    										redBuckets--;
    										break;
		    		case BLUECASTLEPIECE:	smallSandPiles--;
    										largeSandPiles--;
    										blueBuckets--;
    										break;
		    		case REDCASTLEPIECE:	smallSandPiles--;
    										largeSandPiles--;
    										redBuckets--;
    										break;
		    	}
		    }*/
		    
		    if (board->theTurn == Blue) {
    			board->theTurn = gWhosTurn = Red;
		    } else {
    			board->theTurn = gWhosTurn = Blue;
		    }
		}
		
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
	if (DEBUG_CTITM) { printf("\n***** VALID TEXT INPUT *****\n\n"); }
    
    if (DEBUG_CTITM) {
    	printf("input[0] = %c\n", input[0]);
    	printf("input[1] = %c\n", input[1]);
    }
    
    if (((input[0] >= '1') && (input[0] <= '9')) ||
    	(input[0] == 'l') || (input[0] == 's') || (input[0] == 'b')) {
    	if ((input[1] >= '1') && (input[1] <= '9')) {
    		if (DEBUG_CTITM) { printf("\n***** END VALID TEXT INPUT *****\n"); }
    		return TRUE;
    	}
    }
    
    if (DEBUG_CTITM) { printf("\n***** END VALID TEXT INPUT *****\n"); }
    
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
	if (DEBUG_CTITM) { printf("\n***** CONVERT TEXT INPUT TO MOVE *****\n\n"); }
    char first = input[0], second = input[1];
    MOVE thisMove;
    BoardAndTurn curBoard = arrayUnhash(currentBoard);
    GMove newMove = (GMove) SafeMalloc(sizeof(struct cleanMove));
    
    if (DEBUG_CTITM) { 
    	printf("First input = %c\nSecond input = %c\n", first, second);
    }
    
    newMove->fromPos = 0;
    newMove->toPos = (int) (second - '1');
    if (first == 'l') {
    	newMove->movePiece = hLargeSand;
    } else if (first == 's') {
    	newMove->movePiece = hSmallSand;
    } else if (first == 'b') {
    	if (curBoard->theTurn == Blue) {
    		newMove->movePiece = hBlueBucket;
    	} else {
    		newMove->movePiece = hRedBucket;
    	}
    } else {
    	newMove->fromPos = (int) (first - '0');
    	newMove->movePiece = CharToHashBoardPiece(curBoard->theBoard[newMove->fromPos - 1]);
    }
    
    thisMove = hashMove(newMove);
    
    if (DEBUG_CTITM) {
    	printf("MOVE# = %d %x\n", thisMove, thisMove);
    	printMove(newMove);
    	printf("\n");
    }
    
    SafeFree(newMove);
    SafeFree(curBoard->theBoard);
    SafeFree(curBoard);
    
    if (DEBUG_CTITM) { printf("\n***** END CONVERT TEXT INPUT TO MOVE *****\n"); }
    
    return thisMove;
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
    printf(" THIS IS GAME SPECIFIC MENU!\n");
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
	BoardAndTurn board;
	char piece, turn;
	
	InitializeGame();
	
	board = (BoardAndTurn) SafeMalloc(sizeof(struct boardAndTurnRep));
	board->theBoard = (char *) SafeMalloc(boardSize * sizeof(char));
    
  	getchar(); // for the enter after picking option 1 on the debug menu
  
  	for(i = 1; i <= boardSize; i++) {       
  		do {
			printf("Input the character at cell %i followed by <enter>: ", i);
			piece = (char) getchar();
			getchar();
		} while(CharToBoardPiece(piece) == UNKNOWNBOARDPIECE);
		board->theBoard[i-1] = piece;
		switch (piece) {
			case SMALLPIECE:
				smallSandPiles--;
				break;
			case LARGEPIECE:
				largeSandPiles--;
				break;
			case CASTLEPIECE:
				smallSandPiles--;
				largeSandPiles--;
				break;
			case BLUEBUCKETPIECE:
				blueBuckets--;
				break;
			case REDBUCKETPIECE:
				redBuckets--;
				break;
			case BLUESMALLPIECE:
				blueBuckets--;
				smallSandPiles--;
				break;
			case REDSMALLPIECE:
				redBuckets--;
				smallSandPiles--;
				break;
			case BLUECASTLEPIECE:
				blueCastles++;
				blueBuckets--;
				smallSandPiles--;
				largeSandPiles--;
				break;
			case REDCASTLEPIECE:
				redCastles++;
				redBuckets--;
				smallSandPiles--;
				largeSandPiles--;
				break;
		}
  	}
  	
  	do{
		printf("Input whose turn it is (1 for Blue, 2 for Red): ");
		turn = (char) getchar();
		getchar();
	} while((turn != '1') && (turn != '2')); 
	
  	board->theTurn = gWhosTurn = atoi(&turn);
  	gInitialPosition = arrayHash(board);

	SafeFree(board->theBoard);
	SafeFree(board);
  
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

char BoardPieceToChar(BoardPiece piece) {
	switch (piece) {
		case Blank:			return BLANKPIECE;
		case SmallSand:			return SMALLPIECE;
		case LargeSand:			return LARGEPIECE;
		case SandCastle:		return CASTLEPIECE;
		case BlueBucket:		return BLUEBUCKETPIECE;
		case RedBucket:			return REDBUCKETPIECE;
		case BlueSmall:			return BLUESMALLPIECE;
		case RedSmall:			return REDSMALLPIECE;
		case BlueCastle:		return BLUECASTLEPIECE;
		case RedCastle:			return REDCASTLEPIECE;
	}
	
	return UNKNOWNPIECE;
}

char HashBoardPieceToChar(HashBoardPiece piece) {
	switch (piece) {
		case hSmallSand:		return SMALLPIECE;
		case hLargeSand:		return LARGEPIECE;
		case hSandCastle:		return CASTLEPIECE;
		case hBlueBucket:		return BLUEBUCKETPIECE;
		case hRedBucket:		return REDBUCKETPIECE;
		case hBlueSmall:		return BLUESMALLPIECE;
		case hRedSmall:			return REDSMALLPIECE;
		case hBlueCastle:		return BLUECASTLEPIECE;
		case hRedCastle:		return REDCASTLEPIECE;
	}
	
	return UNKNOWNPIECE;
}

BoardPiece CharToBoardPiece(char piece) {
	switch (piece) {
	  case BLANKPIECE:		return Blank;
	  case SMALLPIECE:	        return SmallSand;
	  case LARGEPIECE:		return LargeSand;
	  case CASTLEPIECE:		return SandCastle;
	  case BLUEBUCKETPIECE:		return BlueBucket;
	  case REDBUCKETPIECE:		return RedBucket;
	  case BLUESMALLPIECE:		return BlueSmall;
	  case REDSMALLPIECE: 		return RedSmall;
	  case BLUECASTLEPIECE:		return BlueCastle;
	  case REDCASTLEPIECE:		return RedCastle;
	}
	
	return UNKNOWNBOARDPIECE;
}

HashBoardPiece CharToHashBoardPiece(char piece) {
	switch (piece) {
	  case SMALLPIECE:		return hSmallSand;
	  case LARGEPIECE:		return hLargeSand;
	  case CASTLEPIECE:		return hSandCastle;
	  case BLUEBUCKETPIECE:		return hBlueBucket;
	  case REDBUCKETPIECE:		return hRedBucket;
	  case BLUESMALLPIECE:		return hBlueSmall;
	  case REDSMALLPIECE: 		return hRedSmall;
	  case BLUECASTLEPIECE:		return hBlueCastle;
	  case REDCASTLEPIECE:		return hRedCastle;
	}
	
	return UNKNOWNBOARDPIECE;
}

BoardPiece ThreePieceToBoardPiece(ThreePiece lsb) {
	if (lsb->L == HASHBLANK) {
		if (lsb->S == HASHBLANK) {
			if (lsb->B == HASHBLANK) { 
			  return Blank; 
			}
			else if (lsb->B == HASHBLUEBUCKET) { 
			  return BlueBucket; 
			}
			else if (lsb->B == HASHREDBUCKET) { 
			  return RedBucket; 
			}
		} else if (lsb->S == HASHSANDPILE) {
			if (lsb->B == HASHBLANK) { 
			  return SmallSand; 
			}
			else if (lsb->B == HASHBLUEBUCKET) { 
			  return BlueSmall; 
			}
			else if (lsb->B == HASHREDBUCKET) { 
			  return RedSmall; 
			}
		}
	} else if (lsb->L == HASHSANDPILE) {
		if (lsb->S == HASHBLANK) {
			if (lsb->B == HASHBLANK) { 
			  return LargeSand; 
			}
			else if (lsb->B == HASHBLUEBUCKET) { 
			  return UNKNOWNBOARDPIECE; 
			}
			else if (lsb->B == HASHREDBUCKET) { 
			  return UNKNOWNBOARDPIECE; 
			}
		} else if (lsb->S == HASHSANDPILE) {
			if (lsb->B == HASHBLANK) { 
			  return SandCastle; 
			}
			else if (lsb->B == HASHBLUEBUCKET) { 
			  return BlueCastle; 
			}
			else if (lsb->B == HASHREDBUCKET) { 
			  return RedCastle; 
			}
		}
	}
	
	return UNKNOWNBOARDPIECE;
}

char ThreePieceToChar(ThreePiece lsb) {
	return BoardPieceToChar(ThreePieceToBoardPiece(lsb));
}

ThreePiece BoardPieceToThreePiece(BoardPiece piece) {
	ThreePiece newPiece = (ThreePiece) SafeMalloc(sizeof(struct threePieces));
	
	switch (piece) {
		case Blank:
			newPiece->L = 0;
			newPiece->S = 0;
			newPiece->B = 0;
			break;
	  	case BlueBucket:
	  		newPiece->L = 0;
			newPiece->S = 0;
			newPiece->B = 1;
			break;
	  	case RedBucket:
	  		newPiece->L = 0;
			newPiece->S = 0;
			newPiece->B = 2;
			break;
	  	case SmallSand:
	  		newPiece->L = 0;
			newPiece->S = 1;
			newPiece->B = 0;
			break;
	  	case BlueSmall:
	  		newPiece->L = 0;
			newPiece->S = 1;
			newPiece->B = 1;
			break;
	  	case RedSmall:
	  		newPiece->L = 0;
			newPiece->S = 1;
			newPiece->B = 2;
			break;
	  	case LargeSand:
	  		newPiece->L = 1;
			newPiece->S = 0;
			newPiece->B = 0;
			break;
	  	case SandCastle:
	  		newPiece->L = 1;
			newPiece->S = 1;
			newPiece->B = 0;
			break;
	  	case BlueCastle:
	  		newPiece->L = 1;
			newPiece->S = 1;
			newPiece->B = 1;
			break;
	  	case RedCastle:
	  		newPiece->L = 1;
			newPiece->S = 1;
			newPiece->B = 2;
			break;
		default:
			return NULL;
	}
	
	return newPiece;
}

ThreePiece CharToThreePiece(char piece) {
	return BoardPieceToThreePiece(CharToBoardPiece(piece));
}

char* BoardPieceToString(BoardPiece piece) {
	char* pieceString = (char*) SafeMalloc(30 * sizeof(char));
	
	switch (piece) {
		case Blank:		       	pieceString = BLANKSTRING;
		                                                break;
		case SmallSand:			pieceString = SMALLSTRING;
								break;
		case LargeSand:			pieceString = LARGESTRING;
								break;
		case SandCastle:		pieceString = CASTLESTRING;
								break;
		case BlueBucket:		pieceString = BLUEBUCKETSTRING;
								break;
		case RedBucket:			pieceString = REDBUCKETSTRING;
								break;
		case BlueSmall:			pieceString = BLUESMALLSTRING;
								break;
		case RedSmall:			pieceString = REDSMALLSTRING;
								break;
		case BlueCastle:		pieceString = BLUECASTLESTRING;
								break;
		case RedCastle:			pieceString = REDCASTLESTRING;
								break;
		default:			pieceString = UNKNOWNSTRING;
	}
	
	return pieceString;
}

char* HashBoardPieceToString(HashBoardPiece piece) {
	char* pieceString = (char*) SafeMalloc(30 * sizeof(char));
	
	switch (piece) {
		case hSmallSand:		pieceString = SMALLSTRING;
								break;
		case hLargeSand:		pieceString = LARGESTRING;
								break;
		case hSandCastle:		pieceString = CASTLESTRING;
								break;
		case hBlueBucket:		pieceString = BLUEBUCKETSTRING;
								break;
		case hRedBucket:		pieceString = REDBUCKETSTRING;
								break;
		case hBlueSmall:		pieceString = BLUESMALLSTRING;
								break;
		case hRedSmall:			pieceString = REDSMALLSTRING;
								break;
		case hBlueCastle:		pieceString = BLUECASTLESTRING;
								break;
		case hRedCastle:		pieceString = REDCASTLESTRING;
								break;
		default:			pieceString = UNKNOWNSTRING;
	}
	
	return pieceString;
}

/*
  arrayHash - hashes the board to a number
  Since there are 10 different pieces, this hash utilizes this fact and 
*/
POSITION arrayHash(BoardAndTurn board) {
	BoardRep toHash = (BoardRep) SafeMalloc(sizeof(struct tripleBoardRep));
	toHash->boardL = (char *) SafeMalloc(boardSize * sizeof(char));
	toHash->boardS = (char *) SafeMalloc(boardSize * sizeof(char));
	toHash->boardB = (char *) SafeMalloc(boardSize * sizeof(char));
	ThreePiece piece;
	POSITION L, S, B;
	int i;
	
	if (DEBUG_G) { printf("\n********** arrayHASH **********\n"); }
	
	for (i = 0; i < boardSize; i++) {
		piece = CharToThreePiece(board->theBoard[i]);
		toHash->boardL[i] = piece->L;
		toHash->boardS[i] = piece->S;
		toHash->boardB[i] = piece->B;
		//if (DEBUG_G) { printf("boardL[%d] = %d\n", i, toHash->boardL[i]); }
		//if (DEBUG_G) { printf("boardS[%d] = %d\n", i, toHash->boardS[i]); }
		//if (DEBUG_G) { printf("boardB[%d] = %d\n", i, toHash->boardB[i]); }
	}
	
	generic_hash_context_switch(0);
	L = generic_hash(toHash->boardL, board->theTurn);
	generic_hash_context_switch(1);
	S = generic_hash(toHash->boardS, board->theTurn);
	generic_hash_context_switch(2);
	B = generic_hash(toHash->boardB, board->theTurn);
	if (DEBUG_G) { 
		printf("L = %d\n", (int) L);
		printf("S = %d\n", (int) S);
		printf("B = %d\n", (int) B);
		printf("HASHED # = %d\n", (int) (B + (S * maxB) + (L * maxS * maxB)));
	}
	if (DEBUG_G) { printf("\n********** END arrayHASH **********\n"); }
	
	SafeFree(toHash->boardL);
	SafeFree(toHash->boardS);
	SafeFree(toHash->boardB);
	SafeFree(toHash);
	SafeFree(piece);
	
	return B + (S * maxB) + (L * maxS * maxB);
}

BoardAndTurn arrayUnhash(POSITION hashNumber) {
  BoardAndTurn board = (BoardAndTurn) SafeMalloc(sizeof(struct boardAndTurnRep));
  board->theBoard = (char *) SafeMalloc(boardSize * sizeof(char));
  BoardRep toHash = (BoardRep) SafeMalloc(sizeof(struct tripleBoardRep));
  toHash->boardL = (char *) SafeMalloc(boardSize * sizeof(char));
  toHash->boardS = (char *) SafeMalloc(boardSize * sizeof(char));
  toHash->boardB = (char *) SafeMalloc(boardSize * sizeof(char));
  ThreePiece newPiece = (ThreePiece) SafeMalloc(sizeof(struct threePieces));
  int i;
  
  if (DEBUG_AU) { printf("\n********** arrayUNHASH **********\n"); }
  
  if (DEBUG_AU) { printf("HASHED # = %d\n", (int) hashNumber); }
  
  POSITION L = hashNumber / (maxS * maxB);
  POSITION S = (hashNumber %(maxS * maxB)) / maxB;
  POSITION B = hashNumber % maxB;
  
  if (DEBUG_AU) { 
	  printf("L = %d\n", (int) L);
	  printf("S = %d\n", (int) S);
	  printf("B = %d\n", (int) B);
  }
  
  generic_hash_context_switch(0);
  generic_unhash(L, toHash->boardL);
  generic_hash_context_switch(1);
  generic_unhash(S, toHash->boardS);
  generic_hash_context_switch(2);
  generic_unhash(B, toHash->boardB);

  for (i = 0; i < boardSize; i++) {
  	newPiece->L = toHash->boardL[i];
  	newPiece->S = toHash->boardS[i];
  	newPiece->B = toHash->boardB[i];
  	if (DEBUG_AU) { printf("boardL[%d] = %d\n", i, toHash->boardL[i]); }
  	if (DEBUG_AU) { printf("boardS[%d] = %d\n", i, toHash->boardS[i]); }
  	if (DEBUG_AU) { printf("boardB[%d] = %d\n", i, toHash->boardB[i]); }
  	board->theBoard[i] = ThreePieceToChar(newPiece);
  	if (DEBUG_AU) { printf("board[%d] = %c\n", i, board->theBoard[i]); }
  }
  
  board->theTurn = whoseMove(hashNumber);

  if (DEBUG_AU) { printf("\nwhoseMove(hashNumber) = %d\n", whoseMove(hashNumber)); }
  
  if (DEBUG_AU) { printf("\n********** END arrayUNHASH **********\n"); }
  
  	SafeFree(toHash->boardL);
	SafeFree(toHash->boardS);
	SafeFree(toHash->boardB);
	SafeFree(toHash);
	SafeFree(newPiece);
  
  return board;
}

MOVE hashMove(GMove newMove) {
	if (DEBUG_M) { printf("\n********** HASH MOVE **********\n\n"); }
	
	if (DEBUG_M) { 
		printMove(newMove);
		printf("\n");
	}
	
	if (newMove->fromPos == 0) {		// movePiece = first 2 bits ; toPos = next 4 bits
		if (DEBUG_M) { printf("MOVE# = %d\n", (((int) newMove->movePiece) | (newMove->toPos << 2))); }
		return ((int) newMove->movePiece) | (newMove->toPos << 2);
	} else {	// starting at bit 7 -> movePiece = first 4 bits ; 
				// fromPos = next 4 bits ; toPos = next 4 bits
		if (DEBUG_M) { printf("MOVE# = %d\n", (((int) newMove->movePiece | (newMove->fromPos << 4) | (newMove->toPos << 4)) << 7)); }
		return ((int) newMove->movePiece | (newMove->fromPos << 4) | (newMove->toPos << 8)) << 6;
	}
	
	if (DEBUG_M) { printf("\n********** END HASH MOVE **********\n"); }
}

GMove unhashMove(MOVE newMove) {
	GMove toMove = (GMove) SafeMalloc(sizeof(struct cleanMove));
	
	if (DEBUG_UM) { printf("\n********** UNHASH MOVE **********\n\n"); }
	
	if (newMove < (1 << 6)) {
		toMove->movePiece = (HashBoardPiece) (((int) newMove) & 3);
		toMove->toPos = (((int) newMove) & (0xF << 2)) >> 2;
		toMove->fromPos = 0;
	} else {
		toMove->movePiece = (HashBoardPiece) (((int) newMove) & (0xF << 6)) >> 6;
		toMove->fromPos = (((int) newMove) & (0xF << 10)) >> 10;
		toMove->toPos = (((int) newMove) & (0xF << 14)) >> 14;
	}
	
	if (DEBUG_UM) { 
		printf("MOVE# = %d\n", newMove);
		printMove(toMove);
		printf("\n");
	}
	
	if (DEBUG_UM) { printf("\n********** END UNHASH MOVE **********\n"); }
	
	return toMove;
}

MOVE inverseMove(MOVE move) {
	
	if (DEBUG_IM) { printf("\n***** INVERSE MOVE *****\n\n"); }
	GMove theMove = unhashMove(move);
	if (DEBUG_IM) {
		printf("MOVE#  = %d\n", (int) move);
		printMove(theMove);
		printf("\n");
	}
	int temp;
	MOVE moveToReturn;
	
	temp = theMove->toPos;
	theMove->toPos = theMove->fromPos - 1;
	theMove->fromPos = temp + 1;
	
	moveToReturn = hashMove(theMove);
	
	if (DEBUG_IM) {
		printf("\nIMOVE# = %d\n", (int) moveToReturn);
		printMove(theMove);
		printf("\n");
	}
	
	SafeFree(theMove);
	
	if (DEBUG_IM) { printf("\n***** END INVERSE MOVE *****\n"); }
	
	return moveToReturn;
}

void printMove(GMove move) {
	char* theMove = HashBoardPieceToString(move->movePiece);
	
	printf("MOVE: fromPos %d | toPos %d (POS %d) | piece %s(%d)",
			move->fromPos, move->toPos, move->toPos + 1, theMove,
			((int) move->movePiece));
	//SafeFree(theMove);
}

int validPieceMove(int fromP, int toP) {
	int fromX = fromP % numCols;
	int fromY = fromP / numCols;
        int toX = toP % numCols;
	int toY = toP / numCols;
	int x, y, valid = 0;
	
	if (DEBUG_VPM) { printf("\n***** VALID PIECE MOVE *****\n\n"); }
	
	if (DEBUG_VPM) { 
		printf("fromPos %d -> fromX(%d) fromY(%d)\n",
				fromP, fromX, fromY);
		printf("toPos %d -> toX(%d) toY(%d)\n",
				toP, toX, toY);
	}
	
	if (fromP == toP) { return 0; }
	
	if ((fromP >= 0) && (fromP < boardSize) &&
		(toP >= 0) && (toP < boardSize)) {
		for (x = -1; x <= 1; x++) {
			for (y = -1; y <= 1; y++) {
				if (((fromX + x) == toX) && ((fromY + y) == toY)) {
					valid = 1;
					if (DEBUG_VPM) { printf("VALID!\n"); }
				}
			}
		}
	}
	
	if (DEBUG_VPM) { printf("\n***** END VALID PIECE MOVE *****\n"); }
	
	return valid;
}

	

// $Log: not supported by cvs2svn $
// Revision 1.28  2006/04/05 04:01:34  mikehamada
// *** empty log message ***
//
// Revision 1.27  2006/04/05 03:35:39  alexchoy
// *** empty log message ***
//
// Revision 1.26  2006/04/04 23:47:46  mikehamada
// *** empty log message ***
//
// Revision 1.25  2006/04/04 23:46:20  mikehamada
// *** empty log message ***
//
// Revision 1.24  2006/04/04 23:33:12  mikehamada
// A WHOLE BUNCH of changes...but IT WORKS!
// Still must work out the bugs with UNDO though...
//
// Revision 1.23  2006/03/29 02:50:51  mikehamada
// Hash Not Working yet...
//
// Revision 1.22  2006/03/29 01:44:41  mikehamada
// *** empty log message ***
//
// Revision 1.21  2006/03/28 02:03:30  mikehamada
// *** empty log message ***
//
// Revision 1.20  2006/03/28 00:22:15  mikehamada
// *** empty log message ***
//
// Revision 1.19  2006/03/20 02:21:09  mikehamada
// Finished arrayHash() and arrayUnhash()?
//
// Revision 1.18  2006/03/15 07:46:12  mikehamada
// Added HASHBLANK, HASHSANDPILE, HASHBLUEBUCKET, HASHREDBUCKET defines.
//
// Added BoardRep representation and ThreePiece representation (for a board, use a ThreePiece array!).
//
// Updated InitializeGame() to use LSB generic_hash_init (internal board will still be kept as a BoardAndTurn though!).
//
// Added ThreePieceToBoardPiece(), ThreePieceToChar(), BoardPieceToThreePiece(), CharToThreePiece() methods used for arrayHash() and arrayUnhash().
//
// Revision 1.17  2006/03/14 03:02:58  mikehamada
// Changed InitializeGame(), added BoardRep, changed BoardPiece
//
// Revision 1.16  2006/03/08 01:22:30  mikehamada
// *** empty log message ***
//
// Revision 1.15  2006/03/08 01:19:37  mikehamada
// Formatted DoMove Code
//
// Revision 1.14  2006/03/02 05:43:16  mikehamada
// *** empty log message ***
//
// Revision 1.13  2006/02/27 23:37:40  mikehamada
// *** empty log message ***
//
// Revision 1.12  2006/02/27 23:28:08  mikehamada
// Fixed Errors in DoMove and Primitive
//
// Revision 1.11  2006/02/27 00:19:08  alexchoy
// wrote DoMove and GetInitialPosition, untested
//
// Revision 1.9  2006/02/26 08:31:26  mikehamada
// Fixed errors that prevented game from being built
// Edited InitializeGame(), PrintPosition() to use new hashes
// Fixed struct for board representation
// Changed PrintPosition() since Extended-ASCII does not work
// Changed arrayHash() to use a for-loop to calculate hash
// Wrote Primitive() (unsure if it is finished or not)
//
// Revision 1.8  2006/02/25 19:20:15  mikehamada
// *** empty log message ***
//
// Revision 1.7  2006/02/25 09:33:55  mikehamada
// *** empty log message ***
//
// Revision 1.6  2006/02/25 06:32:09  mikehamada
// *** empty log message ***
//
// Revision 1.5  2006/02/24 17:34:43  mikehamada
// *** empty log message ***
//
// Revision 1.4  2006/02/23 07:19:20  mikehamada
// *** empty log message ***
//
// Revision 1.3  2006/02/22 09:49:04  alexchoy
// edited unhash and hash to make them more correct
//
// Revision 1.2  2006/02/22 09:37:24  alexchoy
// added hashing and unhashing functions
//
// Revision 1.1  2006/02/20 19:36:45  mikehamada
// First addition to repository for Topitop by Mike Hamada
// Setup #defines & data-structs
// Wrote InitializeGame() and PrintPosition()
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
