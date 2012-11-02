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
**	              END: 04/09/2006
**
** UPDATE HIST:
**
**	    02/20/2006 - Setup #defines & data-structs
**                                       Wrote InitializeGame(), PrintPosition()
**      02/22/2006 - Added CharToBoardPiece(), arrayHash(), and arrayUnhash()
**                   Still need to edit above functions with these new fcts
**                   Need to make arrayHash() a for loop
**	    02/26/2006 - Fixed errors that prevented game from being built
**			                 Edited InitializeGame(), PrintPosition() to use new hashes
**                                       Fixed struct for board representation
**                                       Changed PrintPosition() since Extended-ASCII does not work
**			                 Changed arrayHash() to use a for-loop to calculate hash
**			                 Wrote Primitive() (unsure if it is finished or not)
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

STRING kGameName            = "Topitop";   /* The name of your game */

STRING kAuthorName          = "Alex Choy and Mike Hamada";   /* Your name(s) */
STRING kDBName              = "topitop";   /* The name to store the database under */

BOOLEAN kPartizan            = TRUE;   /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN kGameSpecificMenu    = TRUE;   /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN kTieIsPossible       = FALSE;   /* TRUE if a tie is possible. FALSE if it is impossible.*/
BOOLEAN kLoopy               = TRUE;   /* TRUE if the game tree will have cycles (a rearranger style game). FALSE if it does not.*/

BOOLEAN kDebugMenu           = TRUE;   /* TRUE only when debugging. FALSE when on release. */
BOOLEAN kDebugDetermineValue = TRUE;   /* TRUE only when debugging. FALSE when on release. */

POSITION gNumberOfPositions   =  747521822; /* The number of total possible positions | If you are using our hash, this is given by the hash_init() function*/
POSITION gInitialPosition     =  0; /* The initial hashed position for your starting board */
POSITION kBadPosition         = -1; /* A position that will never be used */

BOOLEAN kSupportsHeuristic  = TRUE;
BOOLEAN kSupportsSymmetries = TRUE;
BOOLEAN kSupportsGraphics   = FALSE;

void*    gGameSpecificTclInit = NULL;

/*
 * Help strings that are pretty self-explanatory
 * Strings than span more than one line should have backslashes (\) at the end of the line.
 */

STRING kHelpGraphicInterface =
        "Not written yet.";

STRING kHelpTextInterface    =
        "Use the BOARD to determine which numbers to choose to\n\
correspond to either a piece to place and a board slot where to place it\n\
OR to the location of your piece or a neutral piece and the empty\n\
adjacent position you wish to move that piece to.\n\
Note that a player CANNOT undo an oppontent's move that was just made.\n\n\
A neutral piece is either:\n\
   's': Small Sand Pile\n\
   'l': Large Sand Pile\n\
   'b': Blue Bucket\n\
   'r': Red Bucket\n\n\
Example: 's1' would place a Small Sand Pile in position 1\n\
Example: '12' would move a piece in position 1 to position 2."                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    ;

STRING kHelpOnYourTurn =
        "Each player takes turns making one valid move, also noting\n\
that player CANNOT reverse an opponent's move that was just made.\n\
Use the BOARD to determine which numbers to choose to\n\
correspond to either a piece to place and a board slot to place it\n\
OR to the location of your piece or a neutral piece and the empty\n\
adjacent position you wish to move that piece to.\n\
\n\
VALID MOVES:\n\
1.) Place 1 of your Buckets or 1 of the Small or Large Sand Piles\n\
    on any free space of the board.\n\
2.) Move 1 of your Buckets or any Small or Large Sand Pile already\n\
    placed on the board from one square to another (one space at\n\
    a time in any direction).\n\
       a.) Any piece already on the board can be moved to an \n\
           adjacent free space.\n\
       b.) A Bucket can go on top of a Small Sand Pile.\n\
       c.) A Small Sand Pile can go on top of a Large Sand Pile.\n\
       d.) A Small Sand Pile with a Bucket on its top can go on\n\
           top of a Large Sand Pile.\n\
       e.) A Bucket can go on top of a Sand Castle.\n\
    NOTE: You CANNOT place a piece on top of another piece.\n\
          Pieces must be MOVED on top of other pieces.\n\
3.) Move any combination of Sand Piles with your Buckets on top,\n\
    or any Sand Castle, to any free space."                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  ;

STRING kHelpStandardObjective =
        "Be the first to have your 2 Buckets on top of 2 Sand Castles,\n\
where a Sand Castle consists of 1 Small Sand Pile put on top\n\
of 1 Large Sand Pile."                                                                                                                                          ;

STRING kHelpReverseObjective =
        ""; /* There exists no reverse objective */

STRING kHelpTieOccursWhen =
        "A tie never occurs.";

STRING kHelpExample =
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
#define UNKNOWNPIECE '0'
#define UNKNOWNBOARDPIECE -1

#define BLANKPIECESTRING                "     "
#define SMALLPIECESTRING                " /-\\ "
#define SMALLPIECENUMSTRING             " /-\\ "
#define LARGEPIECESTRING                "/---\\"
#define BLUEBUCKETPIECESTRING   " |b| "
#define REDBUCKETPIECESTRING    " |r| "
#define UNKNOWNPIECESTRING              "XXXXX"

#define BLANKPIECENUMSTRING      "%d    "
#define BLUEBUCKETPIECENUMSTRING "%d|b| "
#define REDBUCKETPIECENUMSTRING  "%d|r| "

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
	hBlueSmall, hRedSmall, hBlueCastle, hRedCastle, hUnknownPiece
} HashBoardPiece;

typedef enum playerTurn {
	Blue = 1, Red
} PlayerTurn;

typedef struct boardDataElements {
	int smallSandPiles;
	int largeSandPiles;
	int redBuckets;
	int blueBuckets;
	int redCastles;
	int blueCastles;
} *BoardData;

typedef struct boardAndTurnRep {
	char *theBoard;
	PlayerTurn theTurn;
	BoardData data;
} *BoardAndTurn;

typedef struct tripleBoardRep {
	char *boardL;   // Holds blanks (0) and large sand piles (1)
	char *boardS;   // Holds blanks (0) and small sand piles (2)
	char *boardB;   // Holds blanks (0), blue buckets (1), and red buckets (2).
} *BoardRep;

typedef struct threePieces {
	char L;
	char S;
	char B;
} *ThreePiece;

typedef struct cleanMove {
	int fromPos;    // 0 = dart-board ; 1-9 = move from position
	int toPos;              // 0-8 = move to position
	HashBoardPiece movePiece;
} *GMove;

typedef struct positionNode {
	POSITION pos;
	struct positionNode* next;
} *PositionList;

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
int DEBUG_AU = 0;
int DEBUG_UM = 0;
int DEBUG_PM = 0;
int DEBUG_DM = 0;
int DEBUG_CTITM = 0;
int DEBUG_VPM = 0;
int DEBUG_IM = 0;
int DEBUG_GAPPM = 0;
int DEBUG_TEST = 0;

int gameType;
//int maxL, maxS, maxB = 0;
PlayerTurn gWhosTurn = Blue;
MOVE lastMove = -1;             //If lastMove = -1, there has been no last move
PositionList allPositions = NULL;

BoardAndTurn globalBoard = NULL;

/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/

/* External */
extern GENERIC_PTR              SafeMalloc ();
extern void                             SafeFree ();
extern POSITION         generic_hash_init(int boardsize, int pieces_array[], int (*vcfg_function_ptr)(int* cfg), int player);
extern POSITION         generic_hash_hash(char *board, int player);
extern char             *generic_hash_unhash(POSITION hash_number, char *empty_board);
extern int              generic_hash_turn (POSITION hashed);
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
void                                    printTopRow(int rowNum, BoardRep toHash);
void                                    printMiddleRow(int rowNum, BoardRep toHash);
void                                    printBottomRow(int rowNum, BoardRep toHash);
char                                    BoardPieceToChar(BoardPiece piece);
char                                    HashBoardPieceToChar(HashBoardPiece piece);
BoardPiece                      CharToBoardPiece(char piece);
HashBoardPiece                  CharToHashBoardPiece(char piece);
BoardPiece                              ThreePieceToBoardPiece(ThreePiece lsb);
char                                    ThreePieceToChar(ThreePiece lsb);
ThreePiece                              BoardPieceToThreePiece(BoardPiece piece);
ThreePiece                              CharToThreePiece(char piece);
char*                                   BoardPieceToString(BoardPiece piece);
char*                                   HashBoardPieceToString(HashBoardPiece piece);
BoardRep                                splitBoardLSB(BoardAndTurn board);
POSITION                                arrayHash(BoardAndTurn board);
BoardAndTurn                    arrayUnhash(POSITION hashNumber);
MOVE                                    hashMove(GMove newMove);
GMove                                   unhashMove(MOVE newMove);
MOVE                                    inverseMove(MOVE move);
void                                    printMove(GMove move);
int                                             validPieceMove(int fromPos, int toPos);
void                                    testHash();
void                                    printBoard(BoardAndTurn board);
void                                    addToAllPositions(POSITION newPos);
POSITION                                getFrontFromAllPositions();
void                                    removeFrontFromAllPositions();

// TIER
TIER BoardToTier(char* theBoard);
STRING TierToString(TIER tier);
TIERLIST* TierChildren(TIER tier);
void SetupTierStuff();
TIERPOSITION NumberOfTierPositions(TIER tier);
BOOLEAN IsLegal(POSITION);

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
	/*    int i;
	   int LpiecesArray[] = { HASHBLANK, 5, 9, HASHSANDPILE, 0, 4, -1 };
	   int SpiecesArray[] = { HASHBLANK, 5, 9, HASHSANDPILE, 0, 4, -1 };
	   int BpiecesArray[] = { HASHBLANK, 5, 9, HASHREDBUCKET, 0, 2, HASHBLUEBUCKET, 0, 2, -1 };

	   BoardAndTurn boardArray;
	   boardArray = (BoardAndTurn) SafeMalloc(sizeof(struct boardAndTurnRep));
	   boardArray->theBoard = (char *) SafeMalloc(boardSize * sizeof(char));

	   if (DEBUG_G) { printf("maxL = %d\n", maxL = generic_hash_init(boardSize, LpiecesArray, NULL, 0)); }
	   if (DEBUG_G) { printf("maxS = %d\n", maxS = generic_hash_init(boardSize, SpiecesArray, NULL, 0)); }
	   if (DEBUG_G) { printf("maxB = %d\n", maxB = generic_hash_init(boardSize, BpiecesArray, NULL, 0)); }

	   // init the hash values
	   maxL = generic_hash_init(boardSize, LpiecesArray, NULL, 0);
	   maxS = generic_hash_init(boardSize, SpiecesArray, NULL, 0);
	   maxB = generic_hash_init(boardSize, BpiecesArray, NULL, 0);

	   gNumberOfPositions = maxB + maxS * maxB + maxL * maxS * maxB;
	   gWhosTurn = boardArray->theTurn = Blue;

	   // begin with the default board, all blanks
	   for (i = 0; i < boardSize; i++) {
	      boardArray->theBoard[i] = BLANKPIECE;
	   }

	   gInitialPosition = arrayHash(boardArray); // = currentBoard = prevBoard
	   SafeFree(boardArray->theBoard);
	   SafeFree(boardArray);
	   if (DEBUG_G) { printf("# Of Pos: %d\n", (int) gNumberOfPositions); }
	   if (DEBUG_G) { printf("Init Pos: %d\n", (int) gInitialPosition); }
	   if (DEBUG_TEST) { testHash(); }
	 */
	// comment out old stuff probably
	//gUsingTierGamesman = TRUE;
	kSupportsTierGamesman = TRUE;
	kExclusivelyTierGamesman = TRUE;

	// always tiering
	generic_hash_destroy();
	SetupTierStuff();

	// call  SetupTierStuff() somewhere here
	// call generic_hash_destroy() before this - no need for this one here since I don't have an option to use the old hash
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

	printf("GenerateMoves\n");

	BoardAndTurn board = arrayUnhash(position); // here???
	GMove newMove = (GMove) SafeMalloc(sizeof(struct cleanMove));
	MOVE tempMove, undoMove = inverseMove(lastMove);
	int i, j;

	if (DEBUG_GM) { printf("\n***** GENERATE MOVES *****\n\n"); }

	newMove->fromPos = 0;

	if (DEBUG_GM) { printf("---- Place Moves ----\n"); }

	/* loop through the board and determine the dartboard moves that can be made */
	for (i = 0; i < boardSize; i++) {
		if (board->theBoard[i] == BLANKPIECE) {
			newMove->toPos = i;
			if (board->data->smallSandPiles > 0) {
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
			if (board->data->largeSandPiles > 0) {
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
			if ((gWhosTurn == Blue) && (board->data->blueBuckets > 0)) {
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
			if ((gWhosTurn == Red) && (board->data->redBuckets > 0)) {
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

	/* loop through the board and determine the moves from one square to another that can be made */
	for (i = 0; i < boardSize; i++) {
		for (j = 0; j < boardSize; j++) {
			newMove->fromPos = i + 1;
			newMove->toPos = j;
			newMove->movePiece = CharToHashBoardPiece(board->theBoard[newMove->fromPos - 1]);
			if ((((int) (tempMove = hashMove(newMove))) != ((int) undoMove)) &&
			    (validPieceMove(i, j))) {
				if (((board->theBoard[i] == BLUEBUCKETPIECE) && (gWhosTurn == Blue)) ||
				    ((board->theBoard[i] == REDBUCKETPIECE) && (gWhosTurn == Red))) {
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
				} else if (((board->theBoard[i] == BLUESMALLPIECE) && (gWhosTurn == Blue)) ||
				           ((board->theBoard[i] == REDSMALLPIECE) && (gWhosTurn == Red)) ||
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
				} else if (((board->theBoard[i] == BLUECASTLEPIECE) && (gWhosTurn == Blue)) ||
				           ((board->theBoard[i] == REDCASTLEPIECE) && (gWhosTurn == Red)) ||
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
	SafeFree(board->data);
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

	printf("DoMove\n");

	BoardAndTurn board = arrayUnhash(position);  // here???
	GMove newMove = unhashMove(move);
	lastMove = move;

	if (DEBUG_DM) { printf("\n***** DO MOVE *****\n\n"); }

	if (DEBUG_DM) {
		printf("POSITION# = %d\n", (int) position);
		printf("MOVE# = %d\n", (int) move);
		printMove(newMove);
		printf("\n");
	}

	/*if ((newMove->movePiece == hUnknownPiece) && (newMove->fromPos != 0)) {
	        newMove->movePiece = board->theBoard[newMove->fromPos - 1];
	   }*/

	if (newMove->fromPos == 0) {
		if (DEBUG_DM) { printf("--- PLACE PIECE ---\n"); }
		if (DEBUG_DM) { printf("Piece To Place = %c\n", HashBoardPieceToChar(newMove->movePiece)); }
		board->theBoard[newMove->toPos] = HashBoardPieceToChar(newMove->movePiece);
		if (board->theBoard[newMove->toPos] == SMALLPIECE) {
			board->data->smallSandPiles--;
		} else if (board->theBoard[newMove->toPos] == LARGEPIECE) {
			board->data->largeSandPiles--;
		} else if (board->theBoard[newMove->toPos] == REDBUCKETPIECE) {
			board->data->redBuckets--;
		} else if (board->theBoard[newMove->toPos] == BLUEBUCKETPIECE) {
			board->data->blueBuckets--;
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
				board->data->blueCastles++;
			} else if (pieceToMove == REDSMALLPIECE) {
				board->theBoard[newMove->toPos] = REDCASTLEPIECE;
				board->data->redCastles++;
			}
		} else if (pieceInWay == CASTLEPIECE) {
			if (pieceToMove == BLUEBUCKETPIECE) {
				board->theBoard[newMove->toPos] = BLUECASTLEPIECE;
				board->data->blueCastles++;
			} else if (pieceToMove == REDBUCKETPIECE) {
				board->theBoard[newMove->toPos] = REDCASTLEPIECE;
				board->data->redCastles++;
			}
		}
	}

	if (gWhosTurn == Blue) {
		board->theTurn = gWhosTurn = Red;
	} else {
		board->theTurn = gWhosTurn = Blue;
	}

	//prevBoard = currentBoard;
	newPosition  = arrayHash(board); // = currentBoard

	if (DEBUG_DM) { printf("NEXT BOARD# = %d\n", (int) newPosition); }

	//SafeFree(globalBoard->data);
	//SafeFree(globalBoard->theBoard);
	//SafeFree(globalBoard);

	globalBoard = board;

	//SafeFree(board->data);
	//SafeFree(board->theBoard);
	//SafeFree(board);
	SafeFree(newMove);

	if (DEBUG_DM) { printf("\n***** END DO MOVE *****\n"); }

	addToAllPositions(newPosition);

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
//	printf("POSITION %llu\n", position);
//PrintPosition(position, "", TRUE);

	BoardAndTurn board;

	printf("Primitive\n");

	board = arrayUnhash(position);

	if (((gWhosTurn == Blue) && (board->data->blueCastles >= NUMCASTLESTOWIN)) ||
	    ((gWhosTurn == Red) && (board->data->redCastles >= NUMCASTLESTOWIN))) {
		return win;
	} else if (((gWhosTurn == Blue) && (board->data->redCastles >= NUMCASTLESTOWIN)) ||
	           ((gWhosTurn == Red) && (board->data->blueCastles >= NUMCASTLESTOWIN))) {
		return lose;
	}

	SafeFree(board->data);
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

//void PrintPosition (POSITION position, STRING playersName, BOOLEAN usersTurn)
//{
//	BoardAndTurn board;
//      int i;
//
//	board = arrayUnhash(position);
//
//	if (DEBUG_PP) { printf("\nPOSITION# = %d\n", (int) position); }
//
//	/***********************LINE 1**************************/
//	printf("\n\n\n       *-*-*-*");
//	printf("                     ");
//	printf("%c = Blank\n", BLANKPIECE);
//
//	/***********************LINE 2**************************/
//	printf("       |");
//	for (i = 0; i < numCols; i++) {
//		printf("%c|", board->theBoard[i]);
//	}
//
//	printf("          ( 1 2 3 )");
//	printf("  %c = Small Sand Pile\n", SMALLPIECE);
//
//	/***********************LINE 3**************************/
//	printf("       *-+-+-*");
//	printf("                     ");
//	printf("%c = Large Sand Pile\n", LARGEPIECE);
//
//	/***********************LINE 4**************************/
//	printf("BOARD: |");
//	for (i = numCols; i < (numCols*2); i++) {
//		printf("%c|", board->theBoard[i]);
//	}
//
//	printf("  LEGEND: ( 4 5 6 )");
//	printf("  %c = Sand Castle\n", CASTLEPIECE);
//
//	/***********************LINE 5**************************/
//	printf("       *-+-+-*");
//	printf("                     ");
//	printf("%c = Blue Bucket\n", BLUEBUCKETPIECE);
//
//	/***********************LINE 6**************************/
//	printf("       |");
//	for (i = numCols*2; i < (numCols*3); i++) {
//		printf("%c|", board->theBoard[i]);
//	}
//
//	printf("          ( 7 8 9 )");
//	printf("  %c = Red Bucket\n", REDBUCKETPIECE);
//
//	/***********************LINE 7**************************/
//	printf("       *-*-*-*");
//	printf("                     ");
//	printf("%c = Blue Small Piece\n", BLUESMALLPIECE);
//
//	/***********************LINE 8, 9, 10, 11**************************/
//	printf("                                   ");
//	printf("%c = Red Small Piece\n", REDSMALLPIECE);
//	printf("# Remaining:                       ");
//	printf("%c = Blue Sand Castle\n", BLUECASTLEPIECE);
//	printf("  Large = %d  |  Small = %d          ",
//			board->data->largeSandPiles, board->data->smallSandPiles);
//	printf("%c = Red Sand Castle\n", REDCASTLEPIECE);
//	printf("  RedB  = %d  |  BlueB = %d\n\n\n",
//			board->data->redBuckets, board->data->blueBuckets);
//	//printf("\n%s\n\n", GetPrediction(position, playerName, usersTurn));
//	SafeFree(board->data);
//	SafeFree(board->theBoard);
//	SafeFree(board);
//}

void PrintPosition (POSITION position, STRING playersName, BOOLEAN usersTurn)
{
	BoardAndTurn board;
	BoardRep toHash;
	int i;

	printf("PrintPosition\n");

	board = arrayUnhash(position);  // here???
	toHash = splitBoardLSB(board);

	if (DEBUG_PP) { printf("\nPOSITION# = %d\n", (int) position); }

	/***********************LINE 1**************************/
	printf("\n\n\n       *-----*-----*-----*\n");

	/***********************LINE 2,3,4**************************/
	printf("       |");
	printTopRow(1, toHash);

	printf("\n       |");
	printMiddleRow(1, toHash);
	printf("          +------------------------+");

	printf("\n       |");
	printBottomRow(1, toHash);
	printf("          |    PIECES REMAINING    |");
	printf("\n");

	/***********************LINE 5**************************/
	printf("       *-----+-----+-----*");
	printf("          |------------------------|");

	/***********************LINE 6,7,8**************************/
	printf("\n       |");
	printTopRow(2, toHash);
	printf("          |Large Piles  [l]:  ");
	for (i = 0; i < board->data->largeSandPiles; i++) {
		printf("*");
	}
	for (i = 0; i < (4 - board->data->largeSandPiles); i++) {
		printf(" ");
	}
	printf(" |");

	printf("\nBOARD: |");
	printMiddleRow(2, toHash);
	printf("          |Small Piles  [s]:  ");
	for (i = 0; i < board->data->smallSandPiles; i++) {
		printf("*");
	}
	for (i = 0; i < (4 - board->data->smallSandPiles); i++) {
		printf(" ");
	}
	printf(" |");

	printf("\n       |");
	printBottomRow(2, toHash);
	printf("          |Blue Buckets [b]:  ");
	for (i = 0; i < board->data->blueBuckets; i++) {
		printf("*");
	}
	for (i = 0; i < (2 - board->data->blueBuckets); i++) {
		printf(" ");
	}
	printf("   |");
	printf("\n");

	/***********************LINE 9**************************/
	printf("       *-----+-----+-----*");
	printf("          |Red Buckets  [r]:  ");
	for (i = 0; i < board->data->redBuckets; i++) {
		printf("*");
	}
	for (i = 0; i < (2 - board->data->redBuckets); i++) {
		printf(" ");
	}
	printf("   |");

	/***********************LINE 10,11,12**************************/
	printf("\n       |");
	printTopRow(3, toHash);
	printf("          +------------------------+");

	printf("\n       |");
	printMiddleRow(3, toHash);

	printf("\n       |");
	printBottomRow(3, toHash);
	printf("\n");

	/***********************LINE 7**************************/
	printf("       *-----*-----*-----*\n");

	/***********************LINE 8, 9, 10, 11**************************/

	printf("\n\n\n");

	//printf("\n%s\n\n", GetPrediction(position, playerName, usersTurn));
	SafeFree(board->data);
	SafeFree(board->theBoard);
	SafeFree(board);
	SafeFree(toHash->boardL);
	SafeFree(toHash->boardS);
	SafeFree(toHash->boardB);
	SafeFree(toHash);
}

void printTopRow(int rowNum, BoardRep toHash) {
	int i;

	for (i = (numCols * (rowNum - 1)); i < (numCols * rowNum); i++) {
		if (toHash->boardL[i] && toHash->boardS[i]) {
			if (toHash->boardB[i] == HASHBLUEBUCKET) {
				printf(BLUEBUCKETPIECENUMSTRING, i + 1);
			} else if (toHash->boardB[i] == HASHREDBUCKET) {
				printf(REDBUCKETPIECENUMSTRING, i + 1);
			} else {
				printf(BLANKPIECENUMSTRING, i + 1);
			}
		} else {
			printf(BLANKPIECENUMSTRING, i + 1);
		}
		printf("|");
	}
}

void printMiddleRow(int rowNum, BoardRep toHash) {
	int i;

	for (i = (numCols * (rowNum - 1)); i < (numCols * rowNum); i++) {
		if (toHash->boardL[i]) {
			if (toHash->boardS[i]) {
				printf(SMALLPIECESTRING);
			} else {
				printf(BLANKPIECESTRING);
			}
		} else if (toHash->boardS[i]) {
			if (toHash->boardB[i] == HASHBLUEBUCKET) {
				printf(BLUEBUCKETPIECESTRING);
			} else if (toHash->boardB[i] == HASHREDBUCKET) {
				printf(REDBUCKETPIECESTRING);
			} else {
				printf(BLANKPIECESTRING);
			}
		} else {
			printf(BLANKPIECESTRING);
		}
		printf("|");
	}
}

void printBottomRow(int rowNum, BoardRep toHash) {
	int i;

	for (i = (numCols * (rowNum - 1)); i < (numCols * rowNum); i++) {
		if (toHash->boardL[i]) {
			printf(LARGEPIECESTRING);
		} else if (toHash->boardS[i]) {
			printf(SMALLPIECESTRING);
		} else if (toHash->boardB[i] == HASHBLUEBUCKET) {
			printf(BLUEBUCKETPIECESTRING);
		} else if (toHash->boardB[i] == HASHREDBUCKET) {
			printf(REDBUCKETPIECESTRING);
		} else {
			printf(BLANKPIECESTRING);
		}
		printf("|");
	}
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

	if (newMove->fromPos == 0) {
		if (newMove->movePiece == hBlueBucket) {
			toPrint = 'b';
		} else if (newMove->movePiece == hRedBucket) {
			toPrint = 'r';
		} else if (newMove->movePiece == hSmallSand) {
			toPrint = 's';
		} else if (newMove->movePiece == hLargeSand) {
			toPrint = 'l';
		} else {
			toPrint = 'x';
		}
		printf("%c%d", toPrint, newMove->toPos + 1);
	} else {
		printf("%d%d", newMove->fromPos, newMove->toPos + 1);
	}

	SafeFree(newMove);

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
	char* playerColor;

	printf("GetAndPrintPlayersMove\n");

	BoardAndTurn board = arrayUnhash(position); // here???

	if (gWhosTurn == Blue) {
		playerColor = "Blue";
	}
	else {
		playerColor = "Red";
	}

	if (DEBUG_GAPPM) { printf("theTurn = %d\n", board->theTurn); }

	for (;; ) {
		/***********************************************************
		* CHANGE THE LINE BELOW TO MATCH YOUR MOVE FORMAT
		***********************************************************/
		//printf("Move key: l = large sand pile, s = small sand pile, b = %s\n", playerColor);
		if (gWhosTurn == Blue) {
			printf("%s's (%s) move [(u)ndo/([l,s,b][1-%d] OR [1-%d][1-%d])] : ",
			       playersName, playerColor, boardSize, boardSize, boardSize);
		}
		else {
			printf("%s's (%s) move [(u)ndo/([l,s,r][1-%d] OR [1-%d][1-%d])] : ",
			       playersName, playerColor, boardSize, boardSize, boardSize);
		}

		input = HandleDefaultTextInput(position, move, playersName);

		if ((input == Undo) && (position != gInitialPosition)) {
			if (gWhosTurn == Blue) {
				gWhosTurn = Red;
			} else {
				gWhosTurn = Blue;
			}

			removeFrontFromAllPositions();
		}

		if (input != Continue)
			return input;
	}

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
	    (input[0] == 'l') || (input[0] == 's') || (input[0] == 'b') || (input[0] == 'r')) {
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
	GMove newMove = (GMove) SafeMalloc(sizeof(struct cleanMove));

	printf("ConvertTextInputToMove, getFrontFromAllPositions() call - %d\n", getFrontFromAllPositions());

	//BoardAndTurn board = arrayUnhash(getFrontFromAllPositions());

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
		newMove->movePiece = hBlueBucket;
	} else if (first == 'r') {
		newMove->movePiece = hRedBucket;
	} else {
		newMove->fromPos = (int) (first - '0');
		newMove->movePiece = CharToHashBoardPiece(globalBoard->theBoard[newMove->fromPos - 1]);
	}

	thisMove = hashMove(newMove);

	if (DEBUG_CTITM) {
		printf("MOVE# = %d %x\n", thisMove, thisMove);
		printMove(newMove);
		printf("\n");
	}

	SafeFree(newMove);
	//SafeFree(board->data);
	//SafeFree(board->theBoard);
	//SafeFree(board);

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
	}

	do {
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
	case Blank:                     return BLANKPIECE;
	case SmallSand:                 return SMALLPIECE;
	case LargeSand:                 return LARGEPIECE;
	case SandCastle:                return CASTLEPIECE;
	case BlueBucket:                return BLUEBUCKETPIECE;
	case RedBucket:                 return REDBUCKETPIECE;
	case BlueSmall:                 return BLUESMALLPIECE;
	case RedSmall:                  return REDSMALLPIECE;
	case BlueCastle:                return BLUECASTLEPIECE;
	case RedCastle:                 return REDCASTLEPIECE;
	}

	return UNKNOWNPIECE;
}

char HashBoardPieceToChar(HashBoardPiece piece) {
	switch (piece) {
	case hSmallSand:                return SMALLPIECE;
	case hLargeSand:                return LARGEPIECE;
	case hSandCastle:               return CASTLEPIECE;
	case hBlueBucket:               return BLUEBUCKETPIECE;
	case hRedBucket:                return REDBUCKETPIECE;
	case hBlueSmall:                return BLUESMALLPIECE;
	case hRedSmall:                 return REDSMALLPIECE;
	case hBlueCastle:               return BLUECASTLEPIECE;
	case hRedCastle:                return REDCASTLEPIECE;
	case hUnknownPiece:             return UNKNOWNPIECE;
	}

	return UNKNOWNPIECE;
}

BoardPiece CharToBoardPiece(char piece) {
	switch (piece) {
	case BLANKPIECE:              return Blank;
	case SMALLPIECE:              return SmallSand;
	case LARGEPIECE:              return LargeSand;
	case CASTLEPIECE:             return SandCastle;
	case BLUEBUCKETPIECE:         return BlueBucket;
	case REDBUCKETPIECE:          return RedBucket;
	case BLUESMALLPIECE:          return BlueSmall;
	case REDSMALLPIECE:           return RedSmall;
	case BLUECASTLEPIECE:         return BlueCastle;
	case REDCASTLEPIECE:          return RedCastle;
	}

	return UNKNOWNBOARDPIECE;
}

HashBoardPiece CharToHashBoardPiece(char piece) {
	switch (piece) {
	case SMALLPIECE:              return hSmallSand;
	case LARGEPIECE:              return hLargeSand;
	case CASTLEPIECE:             return hSandCastle;
	case BLUEBUCKETPIECE:         return hBlueBucket;
	case REDBUCKETPIECE:          return hRedBucket;
	case BLUESMALLPIECE:          return hBlueSmall;
	case REDSMALLPIECE:           return hRedSmall;
	case BLUECASTLEPIECE:         return hBlueCastle;
	case REDCASTLEPIECE:          return hRedCastle;
	case UNKNOWNPIECE:            return hUnknownPiece;
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
	case Blank:                     pieceString = BLANKSTRING;
		break;
	case SmallSand:                 pieceString = SMALLSTRING;
		break;
	case LargeSand:                 pieceString = LARGESTRING;
		break;
	case SandCastle:                pieceString = CASTLESTRING;
		break;
	case BlueBucket:                pieceString = BLUEBUCKETSTRING;
		break;
	case RedBucket:                 pieceString = REDBUCKETSTRING;
		break;
	case BlueSmall:                 pieceString = BLUESMALLSTRING;
		break;
	case RedSmall:                  pieceString = REDSMALLSTRING;
		break;
	case BlueCastle:                pieceString = BLUECASTLESTRING;
		break;
	case RedCastle:                 pieceString = REDCASTLESTRING;
		break;
	default:                                pieceString = UNKNOWNSTRING;
	}

	return pieceString;
}

char* HashBoardPieceToString(HashBoardPiece piece) {
	char* pieceString = (char*) SafeMalloc(30 * sizeof(char));

	switch (piece) {
	case hSmallSand:                pieceString = SMALLSTRING;
		break;
	case hLargeSand:                pieceString = LARGESTRING;
		break;
	case hSandCastle:               pieceString = CASTLESTRING;
		break;
	case hBlueBucket:               pieceString = BLUEBUCKETSTRING;
		break;
	case hRedBucket:                pieceString = REDBUCKETSTRING;
		break;
	case hBlueSmall:                pieceString = BLUESMALLSTRING;
		break;
	case hRedSmall:                 pieceString = REDSMALLSTRING;
		break;
	case hBlueCastle:               pieceString = BLUECASTLESTRING;
		break;
	case hRedCastle:                pieceString = REDCASTLESTRING;
		break;
	case hUnknownPiece:
	default:                                pieceString = UNKNOWNSTRING;
	}

	return pieceString;
}

BoardRep splitBoardLSB(BoardAndTurn board) {
	BoardRep toHash = (BoardRep) SafeMalloc(sizeof(struct tripleBoardRep));
	toHash->boardL = (char *) SafeMalloc(boardSize * sizeof(char));
	toHash->boardS = (char *) SafeMalloc(boardSize * sizeof(char));
	toHash->boardB = (char *) SafeMalloc(boardSize * sizeof(char));
	ThreePiece piece;
	int i;

	for (i = 0; i < boardSize; i++) {
		piece = CharToThreePiece(board->theBoard[i]);
		toHash->boardL[i] = piece->L;
		toHash->boardS[i] = piece->S;
		toHash->boardB[i] = piece->B;
		//if (DEBUG_G) { printf("boardL[%d] = %d\n", i, toHash->boardL[i]); }
		//if (DEBUG_G) { printf("boardS[%d] = %d\n", i, toHash->boardS[i]); }
		//if (DEBUG_G) { printf("boardB[%d] = %d\n", i, toHash->boardB[i]); }
	}

	SafeFree(piece);
	return toHash;
}

/*
   arrayHash - hashes the board to a number
   Since there are 10 different pieces, this hash utilizes this fact and
 */
POSITION arrayHash(BoardAndTurn board) {
	BoardRep toHash;
	TIERPOSITION L, S, B;
	POSITION maxL, maxS, maxB;

	//if (DEBUG_G) { printf("\n********** arrayHASH **********\n"); }
//	printf("arrayHash() 1 \n");

	TIER tierNum = BoardToTier(board->theBoard);  //BoardToTier(char* theBoard)

//	printf("arrayHash() 3 \n");

	toHash = splitBoardLSB(board);

	//printf("arrayHash() 4, %llu, %d\n", tierNum, gWhosTurn);

//	int x;
//	for (x = 0; x < boardSize; x++) {
//		printf("boardL[%d] = %c\n", x, toHash->boardL[x]+48);
//		printf("boardS[%d] = %c\n", x, toHash->boardS[x]+48);
//		printf("boardB[%d] = %c\n", x, toHash->boardB[x]+48);
//	}

	POSITION posL, posS, posB;

	//printf("AAAAA\n");

	generic_hash_context_switch(tierNum);
	posL = generic_hash_hash(toHash->boardL, gWhosTurn);
	//L = generic_hash_hash(toHash->boardL, gWhosTurn);
	//posL = gHashToWindowPosition(L, tierNum);  // problem here
	maxL = generic_hash_max_pos();

	//printf("BBBBB\n");

	generic_hash_context_switch(tierNum + 1);
	posS = generic_hash_hash(toHash->boardS, gWhosTurn);
	//S = generic_hash_hash(toHash->boardS, gWhosTurn);
	//posS = gHashToWindowPosition(S, tierNum + 1);
	maxS = generic_hash_max_pos();

	//printf("CCCCC\n");

	generic_hash_context_switch(tierNum + 2);
	posB = generic_hash_hash(toHash->boardB, gWhosTurn);
	//B = generic_hash_hash(toHash->boardB, gWhosTurn);
	//posB = gHashToWindowPosition(B, tierNum + 2);
	maxB = generic_hash_max_pos();

	generic_hash_context_switch(tierNum);

	/*printf("arrayHash() 2 \n");
	   int asdf = 0;
	   for (; asdf < 9; asdf++) {
	        printf("arrayHash() 2, %c, %c, %c \n", (toHash->boardL)[asdf]+48, (toHash->boardS)[asdf]+48, (toHash->boardB)[asdf]+48);
	   }*/

	//SafeFree(toHash->boardL);
	//SafeFree(toHash->boardS);
	//SafeFree(toHash->boardB);
	//SafeFree(toHash);

//	printf("arrayHash() --- %llu\n", posB + posS * maxB + posL * maxS * maxB);

	//return B + (S * maxB) + (L * maxS * maxB);
	return posB + posS * maxB + posL * maxS * maxB;

/*      // old way
        toHash = splitBoardLSB(board);

        generic_hash_context_switch(0);
        L = generic_hash_hash(toHash->boardL, gWhosTurn);
        generic_hash_context_switch(1);
        S = generic_hash_hash(toHash->boardS, gWhosTurn);
        generic_hash_context_switch(2);
        B = generic_hash_hash(toHash->boardB, gWhosTurn);
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

        return B + (S * maxB) + (L * maxS * maxB);
 */
}

BoardAndTurn arrayUnhash(POSITION hashNumber) {
	/* Max's code
	   TIERPOSITION tierpos; TIER tier;
	              gUnhashToTierPosition(position, &tierpos, &tier);

	              generic_hash_context_switch(tier);
	              generic_hash_unhash(tierpos, board);
	              turn = generic_hash_turn(tierpos);*/

	//(9*small + 45*large + blueB + redB) * 3

	BoardAndTurn board = (BoardAndTurn) SafeMalloc(sizeof(struct boardAndTurnRep));
	board->theBoard = (char *) SafeMalloc(boardSize * sizeof(char));
	board->data = (BoardData) SafeMalloc(sizeof(struct boardDataElements));
	BoardRep toHash = (BoardRep) SafeMalloc(sizeof(struct tripleBoardRep));
	toHash->boardL = (char *) SafeMalloc(boardSize * sizeof(char));
	toHash->boardS = (char *) SafeMalloc(boardSize * sizeof(char));
	toHash->boardB = (char *) SafeMalloc(boardSize * sizeof(char));
	ThreePiece newPiece = (ThreePiece) SafeMalloc(sizeof(struct threePieces));
	int i, small = 4, large = 4, redB = 2, blueB = 2, redC = 0, blueC = 0;

	if (DEBUG_AU) { printf("\n********** arrayUNHASH **********\n"); }

	if (DEBUG_AU) { printf("HASHED # = %d\n", (int) hashNumber); }

	//printf("arrayUnhash 3\n");

	if (hashNumber == 1512)
		printf("aoirjaorijfc\n");

	// unnecessary if use tierpos
	//POSITION L = hashNumber / (maxS * maxB);
	//POSITION S = (hashNumber %(maxS * maxB)) / maxB;
	//POSITION B = hashNumber % maxB;
	POSITION L, S, B, hashNumAdjust = 0;

	TIERPOSITION tierpos; TIER tier;

	printf("hashNumber = %llu\n", hashNumber);

	int s, l, b, r;

	for (i = 0; i < boardSize; i++) {
		if (globalBoard->theBoard[i] == SMALLPIECE) { s++; }
		if (globalBoard->theBoard[i] == LARGEPIECE) { l++; }
		if (globalBoard->theBoard[i] == CASTLEPIECE) { s++; l++; }
		if (globalBoard->theBoard[i] == BLUEBUCKETPIECE) { b++; }
		if (globalBoard->theBoard[i] == REDBUCKETPIECE) { r++; }
		if (globalBoard->theBoard[i] == BLUESMALLPIECE) { s++; b++; }
		if (globalBoard->theBoard[i] == REDSMALLPIECE) { s++; r++; }
		if (globalBoard->theBoard[i] == BLUECASTLEPIECE) {
			s++; l++; b++;
		}
		if (board->theBoard[i] == REDCASTLEPIECE) {
			s++; l++; r++;
		}

		// make simplest board in the tier


		//hashNumAdjust = arrayHash();
	}

	gUnhashToTierPosition(hashNumber - hashNumAdjust, &tierpos, &tier); // get tierpos and tier - not sure how to do this...
	//generic_hash_context_switch(tier);  // switch to that tier's context

	if (hashNumber == 1512)
		printf("in arrayUnhash() tierpos = %llu, tier = %llu\n", tierpos, tier);

	if (DEBUG_AU) {
		printf("L = %d\n", (int) L);
		printf("S = %d\n", (int) S);
		printf("B = %d\n", (int) B);
	}

	// changed this from what i had before (commented out below)
	generic_hash_context_switch(tier);
	//generic_hash_unhash(tierpos/45, toHash->boardL);
	L = generic_hash_max_pos();
	generic_hash_context_switch(tier + 1);
	//generic_hash_unhash((tierpos%45)/9, toHash->boardS);
	S = generic_hash_max_pos();
	generic_hash_context_switch(tier + 2);
	//generic_hash_unhash(tierpos%9, toHash->boardB);
	B = generic_hash_max_pos();

	if (hashNumber == 1512)
		printf("in arrayUnhash() L = %llu, S = %llu, B = %llu\n", L, S, B);
	//sum = b + (s * b) + (l * s * b);
	printf("in arrayUnhash() L = %llu, S = %llu, B = %llu\n", L, S, B);

	generic_hash_context_switch(tier);
	generic_hash_unhash(tierpos/(S*B), toHash->boardL);
	if (hashNumber == 1512)
		printf("in arrayUnhash() tierpos/(S*B) = %llu\n", tierpos/(S*B));

	generic_hash_context_switch(tier + 1);
	generic_hash_unhash((tierpos%(S*B))/B, toHash->boardS);
	if (hashNumber == 1512)
		printf("in arrayUnhash() (tierpos%(S*B))/B = %llu\n", (tierpos%(S*B))/B);

	generic_hash_context_switch(tier + 2);
	generic_hash_unhash(tierpos%B, toHash->boardB);
	if (hashNumber == 1512)
		printf("in arrayUnhash() tierpos%B = %llu\n", tierpos%B);

	int as = 0;
	if (hashNumber == 1512) {
		for(; as < 9; as++) {
			printf("%c, %c, %c\n", (toHash->boardL)[as]+48, (toHash->boardS)[as]+48, (toHash->boardB)[as]+48);
		}
	}


	// changed this from what i had before (commented out below)
	//generic_hash_context_switch(tier);
	//generic_hash_unhash(tierpos/45, toHash->boardL);
	//L = generic_hash_max_pos();
	//generic_hash_context_switch(tier + 1);
	//generic_hash_unhash((tierpos%45)/9, toHash->boardS);
	//S = generic_hash_max_pos();
	//generic_hash_context_switch(tier + 2);
	//generic_hash_unhash(tierpos%9, toHash->boardB);
	//B = generic_hash_max_pos();

	//generic_hash_context_switch(tier);

	//  generic_hash_context_switch(tier*3);
	//l =  generic_hash_max_pos();
	//generic_hash_context_switch(tier*3 + 1);
	//s = generic_hash_max_pos();
	//generic_hash_context_switch(tier*3 + 2);
	//b = generic_hash_max_pos();
	//sum = b + (s * b) + (l * s * b);

	//printf("end arrayUnhash 2");

	for (i = 0; i < boardSize; i++) {
		newPiece->L = toHash->boardL[i];
		newPiece->S = toHash->boardS[i];
		newPiece->B = toHash->boardB[i];

		board->theBoard[i] = ThreePieceToChar(newPiece);

		if (board->theBoard[i] == SMALLPIECE) { small--; }
		if (board->theBoard[i] == LARGEPIECE) { large--; }
		if (board->theBoard[i] == CASTLEPIECE) { small--; large--; }
		if (board->theBoard[i] == BLUEBUCKETPIECE) { blueB--; }
		if (board->theBoard[i] == REDBUCKETPIECE) { redB--; }
		if (board->theBoard[i] == BLUESMALLPIECE) { small--; blueB--; }
		if (board->theBoard[i] == REDSMALLPIECE) { small--; redB--; }
		if (board->theBoard[i] == BLUECASTLEPIECE) {
			small--; large--; blueB--; blueC++;
		}
		if (board->theBoard[i] == REDCASTLEPIECE) {
			small--; large--; redB--; redC++;
		}
	}

	board->data->smallSandPiles = small;
	board->data->largeSandPiles = large;
	board->data->redBuckets = redB;
	board->data->blueBuckets = blueB;
	board->data->redCastles = redC;
	board->data->blueCastles = blueC;
	board->theTurn = generic_hash_turn(hashNumber);

	if (DEBUG_AU) { printf("\ngeneric_hash_turn(hashNumber) = %d\n", generic_hash_turn(hashNumber)); }

	if (DEBUG_AU) { printf("\n********** END arrayUNHASH **********\n"); }

//  printf("arrayUnhash, tier = %llu, small = %d", tier, small);
	//printf("end arrayUnhash\n");

	SafeFree(toHash->boardL);
	SafeFree(toHash->boardS);
	SafeFree(toHash->boardB);
	SafeFree(toHash);
	SafeFree(newPiece);

	return board;

	/*  // old way
	   BoardAndTurn board = (BoardAndTurn) SafeMalloc(sizeof(struct boardAndTurnRep));
	   board->theBoard = (char *) SafeMalloc(boardSize * sizeof(char));
	   board->data = (BoardData) SafeMalloc(sizeof(struct boardDataElements));
	   BoardRep toHash = (BoardRep) SafeMalloc(sizeof(struct tripleBoardRep));
	   toHash->boardL = (char *) SafeMalloc(boardSize * sizeof(char));
	   toHash->boardS = (char *) SafeMalloc(boardSize * sizeof(char));
	   toHash->boardB = (char *) SafeMalloc(boardSize * sizeof(char));
	   ThreePiece newPiece = (ThreePiece) SafeMalloc(sizeof(struct threePieces));
	   int i, small = 4, large = 4, redB = 2, blueB = 2, redC = 0, blueC = 0;

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
	   generic_hash_unhash(L, toHash->boardL);
	   generic_hash_context_switch(1);
	   generic_hash_unhash(S, toHash->boardS);
	   generic_hash_context_switch(2);
	   generic_hash_unhash(B, toHash->boardB);

	   for (i = 0; i < boardSize; i++) {
	      newPiece->L = toHash->boardL[i];
	      newPiece->S = toHash->boardS[i];
	      newPiece->B = toHash->boardB[i];
	      //if (DEBUG_AU) { printf("boardL[%d] = %d\n", i, toHash->boardL[i]); }
	      //if (DEBUG_AU) { printf("boardS[%d] = %d\n", i, toHash->boardS[i]); }
	      //if (DEBUG_AU) { printf("boardB[%d] = %d\n", i, toHash->boardB[i]); }
	      board->theBoard[i] = ThreePieceToChar(newPiece);
	      //if (DEBUG_AU) { printf("board[%d] = %c\n", i, board->theBoard[i]); }
	      if (board->theBoard[i] == SMALLPIECE) { small--; }
	      if (board->theBoard[i] == LARGEPIECE) { large--; }
	      if (board->theBoard[i] == CASTLEPIECE) { small--; large--;}
	      if (board->theBoard[i] == BLUEBUCKETPIECE) { blueB--; }
	      if (board->theBoard[i] == REDBUCKETPIECE) { redB--; }
	      if (board->theBoard[i] == BLUESMALLPIECE) { small--; blueB--; }
	      if (board->theBoard[i] == REDSMALLPIECE) { small--; redB--;}
	      if (board->theBoard[i] == BLUECASTLEPIECE) {
	              small--; large--; blueB--; blueC++;
	      }
	      if (board->theBoard[i] == REDCASTLEPIECE) {
	              small--; large--; redB--; redC++;
	      }
	   }

	   board->data->smallSandPiles = small;
	   board->data->largeSandPiles = large;
	   board->data->redBuckets = redB;
	   board->data->blueBuckets = blueB;
	   board->data->redCastles = redC;
	   board->data->blueCastles = blueC;
	   board->theTurn = generic_hash_turn(hashNumber);

	   if (DEBUG_AU) { printf("\ngeneric_hash_turn(hashNumber) = %d\n", generic_hash_turn(hashNumber)); }

	   if (DEBUG_AU) { printf("\n********** END arrayUNHASH **********\n"); }

	      SafeFree(toHash->boardL);
	      SafeFree(toHash->boardS);
	      SafeFree(toHash->boardB);
	      SafeFree(toHash);
	      SafeFree(newPiece);

	   return board;
	 */
}

MOVE hashMove(GMove newMove) {
	if (DEBUG_M) { printf("\n********** HASH MOVE **********\n\n"); }

	if (DEBUG_M) {
		printMove(newMove);
		printf("\n");
	}

	if (newMove->fromPos == 0) {            // movePiece = first 2 bits ; toPos = next 4 bits
		if (DEBUG_M) { printf("MOVE# = %d\n", (((int) newMove->movePiece) | (newMove->toPos << 2))); }
		return ((int) newMove->movePiece) | (newMove->toPos << 2);
	} else {        // starting at bit 7 -> movePiece = first 4 bits ;
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

void testHash() {
	int i;
	BoardAndTurn board;

	printf("\n***** TESTING HASH *****\n\n");
	printf("gNumberOfPositions = %d\n\n", (int) gNumberOfPositions);
	for (i = 0; i < gNumberOfPositions; i++) {
		board = arrayUnhash((POSITION) i);
		printf("POSITION = %d", i);
		printBoard(board);
		SafeFree(board->data);
		SafeFree(board->theBoard);
		SafeFree(board);
		getchar();
	}

	printf("\n***** FINISHED TESTING HASH *****\n");
}

void printBoard(BoardAndTurn board) {
	printf("\n*-*-*-*\n");
	printf("|%c|%c|%c|\n", board->theBoard[0], board->theBoard[1],
	       board->theBoard[2]);
	printf("*-+-+-*\n");
	printf("|%c|%c|%c|\n", board->theBoard[3], board->theBoard[4],
	       board->theBoard[5]);
	printf("*-+-+-*\n");
	printf("|%c|%c|%c|\n", board->theBoard[6], board->theBoard[7],
	       board->theBoard[8]);
	printf("*-*-*-*\n");
}

void addToAllPositions(POSITION newPos) {
	PositionList newPosNode = (PositionList) SafeMalloc(sizeof(struct positionNode));
	newPosNode->pos = newPos;
	newPosNode->next = allPositions;
	allPositions = newPosNode;
}

POSITION getFrontFromAllPositions() {
	if (allPositions != NULL) {
		return allPositions->pos;
	}

	return kBadPosition;
}

void removeFrontFromAllPositions() {
	PositionList posNode;

	if (allPositions != NULL) {
		posNode = allPositions;
		allPositions = allPositions->next;
		posNode->next = NULL;
		SafeFree(posNode);
	}
}

/*int checkValidBoardPositions() {
   int l, s, b, cnt, allValid = 1;
   BoardAndTurn boardTurn;

   for (b = 0; l < maxB; l++) {
    for (s = 0; s < maxS; s++) {
      for (l = 0; b < maxL; b++) {
        boardTurn = arrayUnhash(b + (s * maxB) + (l * maxS * maxB));
        for (cnt = 0; cnt < boardSize; cnt++) {
          if (CharToThreePiece(boardTurn->theBoard[cnt]) == NULL) {
            // handle error case (add to an array, continue to generate list, etc)
            // will do this later
            allValid = 0;
          }
        }
      }
    }
   }
   return allValid;
   }*/

///////////////////////////////////////////////////////////////////////////////////////////////////
// TIER functions
///////////////////////////////////////////////////////////////////////////////////////////////////

// may need to change TIER scheme to reflect what is on notes with tier = buckets + 9*small + 45*large
// or something like that

// tier by total number of pieces on the board
// theBoard is a representation
TIER BoardToTier(char* theBoard) {
	int i;
	TIER small = 0, large = 0, blueB = 0, redB = 0, tmp;
	int piece;
	for (i = 0; i < boardSize; i++) {
		piece = theBoard[i];
//	printf("&&%d*%c", piece, theBoard[i]);
		if (piece == SMALLPIECE) { small++; }
		if (piece == LARGEPIECE) { large++; }
		if (piece == CASTLEPIECE) { small++; large++; }
		if (piece == BLUEBUCKETPIECE) { blueB++; }
		if (piece == REDBUCKETPIECE) { redB++; }
		if (piece == BLUESMALLPIECE) { small++; blueB++; }
		if (piece == REDSMALLPIECE) { small++; redB++; }
		if (piece == BLUECASTLEPIECE) { small++; large++; blueB++; }
		if (piece == REDCASTLEPIECE) { small++; large++; redB++; }
	}
	// board->theBaord[i] -- was like this before instead of just theBoard[i]

	tmp = (9*small + 45*large + blueB + redB*3) * 3;
	//printf("@@@@@@ redB = %llu, blueB = %llu, small = %llu, large = %llu, TIER = %llu \n", redB, blueB, small, large, tmp);

	return (9*small + 45*large + blueB + redB*3) * 3;
}

STRING TierToString(TIER tier) {
	STRING str = (STRING) SafeMalloc(30*sizeof(char));

	unsigned long long large, small, blueBuckets, redBuckets;
	unsigned long long tierDiv3 = tier / 3;
	large = tierDiv3 / 45;
	small = ((tierDiv3 % 45) / 9);

	blueBuckets = (tierDiv3 % 9) % 3;
	if (tierDiv3 % 9 <= 2) {
		redBuckets = 0;
	}
	else if (tierDiv3 % 9 <= 5) {
		redBuckets = 1;
	}
	else {
		redBuckets = 2;
	}

	sprintf(str, "large = %llu, small = %llu, red = %llu, blue = %llu", large, small, redBuckets, blueBuckets);
	return str;
}

// needs to make the "transitions" of the nodes in the Tier tree
TIERLIST* TierChildren(TIER tier) {
	TIERLIST* list = NULL;
	list = CreateTierlistNode(tier, list); // takes the list pointer and creates a new node with that tier and make it its next pointer
	//^~~~ link to yourself (tier1->tier1)

	// -- need to add more links --
	// check tier's pieces - # of L's, S's, and B's
	// tier = (buckets + 9*small + 45*large)*3
	// use formula similar to the one in SetupTierStuff()
	unsigned long long large, small, blueBuckets, redBuckets; // can use type UINT64 too
	unsigned long long tempLarge, tempSmall, tempBlueBuckets, tempRedBuckets, tempTier;
	unsigned long long tierDiv3;
	tierDiv3 = tier/3;
	large = (tierDiv3 / 45);
	small = ((tierDiv3 % 45) / 9);

	blueBuckets = (tierDiv3 % 9) % 3;
	if (tierDiv3 % 9 <= 2) {
		redBuckets = 0;
	}
	else if (tierDiv3 % 9 <= 5) {
		redBuckets = 1;
	}
	else {
		redBuckets = 2;
	}

	//printf("~~~~~ start of redB = %llu, blueB = %llu, small = %llu, large = %llu, TIER = %llu, tierDiv3 = %llu\n", redBuckets, blueBuckets, small, large, tier, tierDiv3);

	// see what tier's those pieces can go to (basically, add a piece to the board)
	if ((large + small + redBuckets + blueBuckets) < 12) {
		// check large pieces
		if (large < 4) {
			// compute that new tier
			tempLarge = large + 1;
			tempTier = (3*redBuckets + blueBuckets + 9*small + 45*tempLarge)*3;
			list = CreateTierlistNode(tempTier, list);
//      printf("redB = %llu, blueB = %llu, small = %llu, large = %llu, tempTier = %llu \n", redBuckets, blueBuckets, small, tempLarge, tempTier);
		}

		// check small pieces
		if (small < 4) {
			// compute that new tier
			tempSmall = small + 1;
			tempTier = (3*redBuckets + blueBuckets + 9*tempSmall + 45*large)*3;
			list = CreateTierlistNode(tempTier, list);
//      printf("redB = %llu, blueB = %llu, small = %llu, large = %llu, tempTier = %llu \n", redBuckets, blueBuckets, tempSmall, large, tempTier);
		}

		// check redBuckets
		if (redBuckets < 2) {
			// compute that new tier
			tempRedBuckets = redBuckets + 1;
			tempTier = (3*tempRedBuckets + blueBuckets + 9*small + 45*large)*3;
			list = CreateTierlistNode(tempTier, list);
//      printf("redB = %llu, blueB = %llu, small = %llu, large = %llu, tempTier = %llu \n", tempRedBuckets, blueBuckets, small, large, tempTier);
		}

		// check blueBuckets
		if (blueBuckets < 2) {
			// compute that new tier
			tempBlueBuckets = blueBuckets + 1;
			tempTier = (3*redBuckets + tempBlueBuckets + 9*small + 45*large)*3;
			list = CreateTierlistNode(tempTier, list);
//      printf("redB = %llu, blueB = %llu, small = %llu, large = %llu, tempTier = %llu \n", redBuckets, tempBlueBuckets, small, large, tempTier);
		}
	}

	//printf("hereherehere\n");
	//printf("%s\n", list);

	return list;
}

void SetupTierStuff() {
	kSupportsTierGamesman = TRUE;
	gTierChildrenFunPtr = &TierChildren;
	gNumberOfTierPositionsFunPtr = &NumberOfTierPositions;
	gTierToStringFunPtr = &TierToString;
	generic_hash_custom_context_mode(TRUE);
	gIsLegalFunPtr = &IsLegal;

	int i = 0;
	int tier = 0, b = 0, r = 0;
	int tempHash, maxL, maxS, maxB;
	int LpiecesArray[] = { HASHBLANK, 5, 9, HASHSANDPILE, 0, 4, -1 };
	int SpiecesArray[] = { HASHBLANK, 5, 9, HASHSANDPILE, 0, 4, -1 };
	int BpiecesArray[] = { HASHBLANK, 5, 9, HASHREDBUCKET, 0, 2, HASHBLUEBUCKET, 0, 2, -1 };

	BoardAndTurn boardArray;
	boardArray = (BoardAndTurn) SafeMalloc(sizeof(struct boardAndTurnRep));
	boardArray->theBoard = (char *) SafeMalloc(boardSize * sizeof(char));

//  printf("1\n");

	for(; tier < 45*5; tier++) { // 12 total pieces - 4 large, 4 small, 2 redB, 2blueB
		// tier = buckets + 9*small + 45*large
		LpiecesArray[1] = 9 - (tier / 45); //5
		LpiecesArray[2] = 9 - (tier / 45);
		LpiecesArray[4] = (tier / 45); //0
		LpiecesArray[5] = (tier / 45);

		SpiecesArray[1] = 9 - ((tier % 45) / 9); //5;
		SpiecesArray[2] = 9 - ((tier % 45) / 9);
		SpiecesArray[4] = ((tier % 45) / 9); //0;
		SpiecesArray[5] = ((tier % 45) / 9);

		// 0 = 0 blue, 0 red / 1 = 1 blue, 0 red / 2 = 2 blue, 0 red
		// 3 = 0 blue, 1 red / 4 = 1 blue, 1 red / 5 = 2 blue, 1 red
		// 6 = 0 blue, 2 red / 7 = 1 blue, 2 red / 8 = 2 blue, 2 red
		b = (tier % 9) % 3;
		if (tier % 9 <= 2) {
			r = 0;
		}
		else if (tier % 9 <= 5) {
			r = 1;
		}
		else {
			r = 2;
		}
		BpiecesArray[1] = 9 - b - r; //5;
		BpiecesArray[2] = 9 - b - r;
		BpiecesArray[4] = r; //0;
		BpiecesArray[5] = r;
		BpiecesArray[7] = b; //0;
		BpiecesArray[8] = b;

		maxL = generic_hash_init(boardSize, LpiecesArray, NULL, 0);
		generic_hash_set_context(tier*3);

		maxS = generic_hash_init(boardSize, SpiecesArray, NULL, 0);
		generic_hash_set_context(tier*3 + 1);

		maxB = generic_hash_init(boardSize, BpiecesArray, NULL, 0);
		generic_hash_set_context(tier*3 + 2);

		//printf("one: %d, two: %d, three: %d\n", tier*3, tier*3+1, tier*3+2);

		//printf("**************tier: %d, tier+1: %d, tier+2: %d\n", tier*3, tier*3+1, tier*3+2);

		tempHash = maxB + maxS * maxB + maxL * maxS * maxB; // what do i do with this hash #????
		// tier position

		// how it was in Max's doc
		//generic_hash_init(boardsize, pieces_array, NULL, 0);
		//generic_hash_set_context(tier);  // sets context for this generic hash
	}

	//gNumberOfPositions = maxB + maxS * maxB + maxL * maxS * maxB;
	gWhosTurn = boardArray->theTurn = Blue;

//  printf("2\n");

	// begin with the default board, all blanks
	for (i = 0; i < boardSize; i++) {
		boardArray->theBoard[i] = BLANKPIECE;
	}

	//  printf("3\n");

	gInitialTier = 0;
	generic_hash_context_switch(gInitialTier);

	//  printf("3.5\n");
	gInitialTierPosition = 0; //arrayHash(boardArray);

	//  printf("6\n");

	gNumberOfPositions = NumberOfTierPositions(gInitialTier); // ?

//printf("5\n");

	globalBoard = boardArray;

	gInitialPosition = arrayHash(boardArray); // = currentBoard = prevBoard
	//SafeFree(boardArray->theBoard);
	//SafeFree(boardArray);

	//printf("4, end of setupTIER\n");
}

TIERPOSITION NumberOfTierPositions(TIER tier) {
	POSITION l, s, b;
	TIERPOSITION sum;

	printf("NumberOfTierPositions(%llu)\n", tier);

	generic_hash_context_switch(tier);
	l =  generic_hash_max_pos();
	generic_hash_context_switch(tier + 1);
	s = generic_hash_max_pos();
	generic_hash_context_switch(tier + 2);
	b = generic_hash_max_pos();
	sum = b + (s * b) + (l * s * b);
	//sum = b + s + l;
	return sum;
	//(B + (S * maxB) + (L * maxS * maxB))
}

// isLegal
BOOLEAN IsLegal(POSITION position) {
	BoardAndTurn board = (BoardAndTurn) SafeMalloc(sizeof(struct boardAndTurnRep));
	board->theBoard = (char *) SafeMalloc(boardSize * sizeof(char));
	board->data = (BoardData) SafeMalloc(sizeof(struct boardDataElements));
	BoardRep toHash = (BoardRep) SafeMalloc(sizeof(struct tripleBoardRep));
	toHash->boardL = (char *) SafeMalloc(boardSize * sizeof(char));
	toHash->boardS = (char *) SafeMalloc(boardSize * sizeof(char));
	toHash->boardB = (char *) SafeMalloc(boardSize * sizeof(char));
	ThreePiece lsb = (ThreePiece) SafeMalloc(sizeof(struct threePieces));

	POSITION L, S, B;

	TIERPOSITION tierpos; TIER tier;
	gUnhashToTierPosition(position, &tierpos, &tier); // get tierpos and tier - not sure how to do this...
	//generic_hash_context_switch(tier);  // switch to that tier's context

	// changed this from what i had before (commented out below)
	generic_hash_context_switch(tier);
	//generic_hash_unhash(tierpos/45, toHash->boardL);
	L = generic_hash_max_pos();
	generic_hash_context_switch(tier + 1);
	//generic_hash_unhash((tierpos%45)/9, toHash->boardS);
	S = generic_hash_max_pos();
	generic_hash_context_switch(tier + 2);
	//generic_hash_unhash(tierpos%9, toHash->boardB);
	B = generic_hash_max_pos();

	generic_hash_context_switch(tier);
	generic_hash_unhash(tierpos/(S*B), toHash->boardL);

	generic_hash_context_switch(tier + 1);
	generic_hash_unhash((tierpos%(S*B))/B, toHash->boardS);

	generic_hash_context_switch(tier + 2);
	generic_hash_unhash(tierpos%B, toHash->boardB);

	int as = 0, piece = 0;
	for(; as < 9; as++) {
		lsb->L = (toHash->boardL)[as]+48;
		lsb->S = (toHash->boardS)[as]+48;
		lsb->B = (toHash->boardB)[as]+48;
		piece = ThreePieceToBoardPiece(lsb);
		//printf("%c, %c, %c\n", (toHash->boardL)[as]+48, (toHash->boardS)[as]+48, (toHash->boardB)[as]+48);
		if (piece == UNKNOWNBOARDPIECE) {
			SafeFree(toHash->boardL);
			SafeFree(toHash->boardS);
			SafeFree(toHash->boardB);
			SafeFree(toHash);
			SafeFree(lsb);
			return FALSE;
		}
	}
	SafeFree(toHash->boardL);
	SafeFree(toHash->boardS);
	SafeFree(toHash->boardB);
	SafeFree(toHash);
	SafeFree(lsb);
	return TRUE;
}

// $Log: not supported by cvs2svn $
// Revision 1.54  2007/05/17 01:29:09  alexchoy
// *** empty log message ***
//
// Revision 1.53  2007/05/16 22:37:15  alexchoy
// *** empty log message ***
//
// Revision 1.52  2007/05/08 00:23:15  alexchoy
// *** empty log message ***
//
// Revision 1.51  2007/05/02 22:34:47  alexchoy
// *** empty log message ***
//
// Revision 1.50  2007/04/25 20:01:19  alexchoy
// *** empty log message ***
//
// Revision 1.49  2007/04/24 07:50:32  alexchoy
// finished TierChildren()
//
// Revision 1.48  2007/04/23 23:52:31  alexchoy
// *** empty log message ***
//
// Revision 1.47  2007/04/23 23:26:08  alexchoy
// *** empty log message ***
//
// Revision 1.46  2007/04/22 01:30:18  alexchoy
// initial TIERing code almost complete
//
// Revision 1.45  2007/04/09 22:28:20  alexchoy
// minor changes
//
// Revision 1.44  2007/04/09 22:25:41  alexchoy
// minor changes
//
// Revision 1.43  2007/04/05 04:51:07  alexchoy
// mtopitop with TIERing, not done yet
//
// Revision 1.1  2007/04/01 20:49:49  alexchoy
// TIERing topitop
//
// Revision 1.42  2006/10/17 10:45:21  max817
// HUGE amount of changes to all generic_hash games, so that they call the
// new versions of the functions.
//
// Revision 1.41  2006/04/17 09:37:38  alexchoy
// *** empty log message ***
//
// Revision 1.40  2006/04/16 07:07:29  mikehamada
// *** empty log message ***
//
// Revision 1.39  2006/04/16 07:05:37  mikehamada
// Updated kHelpTextInterface & kHelpOnYourTurn
// help strings.
//
// Revision 1.38  2006/04/15 07:32:41  mikehamada
// *** empty log message ***
//
// Revision 1.37  2006/04/15 07:29:30  mikehamada
// Updated PrintPosition() format and UI!
//
// Revision 1.36  2006/04/13 11:00:29  mikehamada
// Fixed Undo!   Keeps a local, global list of all POSITIONS
// and updates the list accordingly.
//
// Revision 1.35  2006/04/11 03:03:36  alexchoy
// added some comments and 'r' input
//
// Revision 1.34  2006/04/11 00:54:26  mikehamada
// Testing Hash
//
// Revision 1.33  2006/04/10 06:43:19  mikehamada
// WORKING TOPITOP!!!!
//
// Revision 1.32  2006/04/10 06:16:04  mikehamada
// Removed needless comments and turned off all debugs
//
// Revision 1.31  2006/04/10 06:14:47  mikehamada
// Removed prevBoard, curBoard, smallSandPiles, largeSandPiles,
// blueBuckets, redBuckets, blueCastles, redCastles global variable
// dependence and incorporated it into a single new structure
// BoardData that is now apart of the BoardAndTurn structure.
// arrayUnhash will set these values correctly when looking through
// the board for each board.  So now...UNDO and changing the order
// of players works!!!
//
// Revision 1.30  2006/04/10 01:54:11  mikehamada
// *** empty log message ***
//
// Revision 1.29  2006/04/05 05:15:14  alexchoy
// error when using arrayUnhash with generic_hash_turn(), which always returns Red (2) after the first (Blue's) move
//
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
//           in their getOption/setOption hash.
// mttc.c: Edited to handle conflicting types.  Created a PLAYER type for
//         gamesman.  mttc.c had a PLAYER type already, so I changed it.
// analysis.c: Changed initialization of option variable in analyze() to -1.
// db.c: Changed check in the getter functions (GetValueOfPosition and
//       getRemoteness) to check if gMenuMode is Evaluated.
// gameplay.c: Removed PlayAgainstComputer and PlayAgainstHuman.  Wrote PlayGame
//             which is a generic version of the two that uses to PLAYER's.
// gameplay.h: Created the necessary structs and types to have PLAYER's, both
//          Human and Computer to be sent in to the PlayGame function.
// gamesman.h: Really don't think I changed anything....
// globals.h: Also don't think I changed anything....both these I just looked at
//            and possibly made some format changes.
// textui.c: Redid the portion of the menu that allows you to choose opponents
//        and then play a game.  Added computer vs. computer play.  Also,
//           changed the analysis part of the menu so that analysis should
//        work properly with symmetries (if it is in getOption/setOption hash).
//
// Revision 1.3  2005/03/10 02:06:47  ogren
// Capitalized CVS keywords, moved Log to the bottom of the file - Elmer
//
POSITION StringToPosition(char* board) {
	// FIXME: this is just a stub
	return atoi(board);
}


char* PositionToString(POSITION pos) {
	// FIXME: this is just a stub
	return "Implement Me";
}
