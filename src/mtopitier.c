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

 // Revision 1.56  2019/12/03 14:46:00  chuxiongquan
 // Complete tier version, but it can't run without better hash function 
 // (Which can hash board with more pieces efficiently)
 //
 // Revision 1.55  2019/12/03 03:20:00  chuxiongquan
 // brand new tier version!(Directly hash the board without three layers)
 //
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
 // gameplay.c: Removed PlayAgainstCost significant digit)
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

STRING kGameName = "Topitop";   /* The name of your game */

STRING kAuthorName = "Alex Choy and Mike Hamada";   /* Your name(s) */
STRING kDBName = "topitop";   /* The name to store the database under */

BOOLEAN kPartizan = TRUE;   /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN kGameSpecificMenu = TRUE;   /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN kTieIsPossible = FALSE;   /* TRUE if a tie is possible. FALSE if it is impossible.*/
BOOLEAN kLoopy = TRUE;   /* TRUE if the game tree will have cycles (a rearranger style game). FALSE if it does not.*/

BOOLEAN kDebugMenu = TRUE;   /* TRUE only when debugging. FALSE when on release. */
BOOLEAN kDebugDetermineValue = TRUE;   /* TRUE only when debugging. FALSE when on release. */

POSITION gNumberOfPositions = 747521822; /* The number of total possible positions | If you are using our hash, this is given by the hash_init() function*/
POSITION gInitialPosition = 0; /* The initial hashed position for your starting board */
POSITION kBadPosition = -1; /* A position that will never be used */

BOOLEAN kSupportsHeuristic = TRUE;
BOOLEAN kSupportsSymmetries = TRUE;
BOOLEAN kSupportsGraphics = FALSE;

void* gGameSpecificTclInit = NULL;

/*
 * Help strings that are pretty self-explanatory
 * Strings than span more than one line should have backslashes (\) at the end of the line.
 */

STRING kHelpGraphicInterface =
"Not written yet.";

STRING kHelpTextInterface =
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
Example: '12' would move a piece in position 1 to position 2.";

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
    or any Sand Castle, to any free space.";

STRING kHelpStandardObjective =
"Be the first to have your 2 Buckets on top of 2 Sand Castles,\n\
where a Sand Castle consists of 1 Small Sand Pile put on top\n\
of 1 Large Sand Pile.";

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

#define BLUETURN 1
#define REDTURN 2

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
	char* theBoard;
	PlayerTurn theTurn;
	BoardData data;
} *BoardAndTurn;

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
int maxL, maxS, maxB = 0;
PlayerTurn gWhosTurn = Blue;
MOVE lastMove = -1;             //If lastMove = -1, there has been no last move
PositionList allPositions = NULL;

/*************************************************************************
 **
 ** Function Prototypes
 **
 *************************************************************************/

 /* External */
extern GENERIC_PTR              SafeMalloc();
extern void                             SafeFree();
extern POSITION         generic_hash_init(int boardsize, int pieces_array[], int (*vcfg_function_ptr)(int* cfg), int player);
extern POSITION         generic_hash_hash(char* board, int player);
extern char* generic_hash_unhash(POSITION hash_number, char* empty_board);
extern int              generic_hash_turn(POSITION hashed);
/* Internal */
void                    InitializeGame();
MOVELIST* GenerateMoves(POSITION position);
POSITION                DoMove(POSITION position, MOVE move);
VALUE                   Primitive(POSITION position);
void                    PrintPosition(POSITION position, STRING playersName, BOOLEAN usersTurn);
void                    PrintComputersMove(MOVE computersMove, STRING computersName);
void                    PrintMove(MOVE move);
USERINPUT               GetAndPrintPlayersMove(POSITION position, MOVE* move, STRING playersName);
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
void                                    printTopRow(int rowNum, char* theBoard);
void                                    printMiddleRow(int rowNum, char* theBoard);
void                                    printBottomRow(int rowNum, char* theBoard);
char                                    BoardPieceToChar(BoardPiece piece);
char                                    HashBoardPieceToChar(HashBoardPiece piece);
BoardPiece                      CharToBoardPiece(char piece);
HashBoardPiece                  CharToHashBoardPiece(char piece);
char* BoardPieceToString(BoardPiece piece);
char* HashBoardPieceToString(HashBoardPiece piece);
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

//TIER GAMESMAN
void                                    SetupTierStuff();
TIER									BoardToTier(BoardAndTurn);
TIERLIST*								TierChildren(TIER);
TIERPOSITION							NumberOfTierPositions(TIER);
void									GetInitialTierPosition(TIER*, TIERPOSITION*);
BOOLEAN									IsLegal(POSITION);
//UNDOMOVELIST*							GenerateUndoMovesToTier(POSITION, TIER);
STRING									TierToString(TIER);
//POSITION								UnDoMove(POSITION, UNDOMOVE);
TIER                                    PiecesToTier(TIER, TIER, TIER, TIER);
void									CompressBoardandHash(int[]);
BOOLEAN									ValidPiecesArray(int[]);



/************************************************************************
 **
 ** NAME:        InitializeGame
 **
 ** DESCRIPTION: Prepares the game for execution.
 **              Initializes required variables.
 **
 ************************************************************************/

void InitializeGame()
{
	printf("INITIALIZE GAME");
	kExclusivelyTierGamesman = TRUE; //Can be deleted later
	//discard current hash
	generic_hash_destroy();

	//Setup Tier Stuff
	SetupTierStuff();

	// GLOBAL HASH
	int TOTALTIERPOSITIONS = 225;
	int piecesArray[] = { LARGEPIECE, 0, 0, SMALLPIECE, 0, 0,
		BLUEBUCKETPIECE, 0, 0, REDBUCKETPIECE, 0, 0, CASTLEPIECE, 0, 0,
		BLUESMALLPIECE, 0, 0, REDSMALLPIECE, 0, 0, BLUECASTLEPIECE, 0, 0,
		REDCASTLEPIECE, 0, 0, BLANKPIECE, 0, 0, -1 };
	for (int tier = 0; tier < TOTALTIERPOSITIONS; tier++) {
		piecesArray[1] = piecesArray[2] = tier / 45;
		piecesArray[4] = piecesArray[5] = (tier % 45) / 9;
		piecesArray[7] = piecesArray[8] = (tier % 9) % 3;
		if (tier % 9 <= 2) {
			piecesArray[10] = piecesArray[11] = 0;
		}
		else if (tier % 9 <= 5) {
			piecesArray[10] = piecesArray[11] = 1;
		}
		else {
			piecesArray[10] = piecesArray[11] = 2;
		}
		piecesArray[28] = piecesArray[29] = 9 - piecesArray[1] - piecesArray[4] - piecesArray[7] - piecesArray[10];
		CompressBoardandHash(piecesArray);
	}

	// gInitialPosition
	BoardAndTurn boardArray = (BoardAndTurn)SafeMalloc(sizeof(struct boardAndTurnRep));
	boardArray->theBoard = (char*)SafeMalloc(boardSize * sizeof(char));
	for (int i = 0; i < boardSize; i++) {
		boardArray->theBoard[i] = BLANKPIECE;
	}
	gWhosTurn = boardArray->theTurn = Blue;
	gInitialPosition = arrayHash(boardArray);
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

MOVELIST* GenerateMoves(POSITION position)
{
	MOVELIST* moves = NULL;
	MOVELIST* CreateMovelistNode();

	printf("GenerateMoves\n");

	BoardAndTurn board = arrayUnhash(position); 
	GMove newMove = (GMove)SafeMalloc(sizeof(struct cleanMove));
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
					printf("NEWMOVE %d %x\n", (int)tempMove, (int)tempMove);
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
					printf("NEWMOVE %d %x\n", (int)tempMove, (int)tempMove);
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
					printf("NEWMOVE %d %x\n", (int)tempMove, (int)tempMove);
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
					printf("NEWMOVE %d %x\n", (int)tempMove, (int)tempMove);
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
			if ((((int)(tempMove = hashMove(newMove))) != ((int)undoMove)) &&
				(validPieceMove(i, j))) {
				if (((board->theBoard[i] == BLUEBUCKETPIECE) && (gWhosTurn == Blue)) ||
					((board->theBoard[i] == REDBUCKETPIECE) && (gWhosTurn == Red))) {
					if ((board->theBoard[j] == BLANKPIECE) || (board->theBoard[j] == SMALLPIECE) ||
						(board->theBoard[j] == CASTLEPIECE)) {
						if (DEBUG_GM) {
							printf("NEWMOVE %d %x\n", (int)tempMove, (int)tempMove);
							printMove(newMove);
							printf("\n");
							printMove(unhashMove(tempMove));
							printf("\n");
						}
						moves = CreateMovelistNode(tempMove, moves);
					}
				}
				else if (((board->theBoard[i] == BLUESMALLPIECE) && (gWhosTurn == Blue)) ||
					((board->theBoard[i] == REDSMALLPIECE) && (gWhosTurn == Red)) ||
					(board->theBoard[i] == SMALLPIECE)) {
					if ((board->theBoard[j] == BLANKPIECE) || (board->theBoard[j] == LARGEPIECE)) {
						if (DEBUG_GM) {
							printf("NEWMOVE %d %x\n", (int)tempMove, (int)tempMove);
							printMove(newMove);
							printf("\n");
							printMove(unhashMove(tempMove));
							printf("\n");
						}
						moves = CreateMovelistNode(tempMove, moves);
					}
				}
				else if (((board->theBoard[i] == BLUECASTLEPIECE) && (gWhosTurn == Blue)) ||
					((board->theBoard[i] == REDCASTLEPIECE) && (gWhosTurn == Red)) ||
					(board->theBoard[i] == LARGEPIECE) || (board->theBoard[i] == CASTLEPIECE)) {
					if (board->theBoard[j] == BLANKPIECE) {
						if (DEBUG_GM) {
							printf("NEWMOVE %d %x\n", (int)tempMove, (int)tempMove);
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

POSITION DoMove(POSITION position, MOVE move) {
	char pieceToMove, pieceInWay;
	POSITION newPosition = 0;

	printf("DoMove\n");

	BoardAndTurn board = arrayUnhash(position);  // here???
	GMove newMove = unhashMove(move);
	lastMove = move;

	if (DEBUG_DM) { printf("\n***** DO MOVE *****\n\n"); }

	if (DEBUG_DM) {
		printf("POSITION# = %d\n", (int)position);
		printf("MOVE# = %d\n", (int)move);
		printMove(newMove);
		printf("\n");
	}

	if (newMove->fromPos == 0) {
		if (DEBUG_DM) { printf("--- PLACE PIECE ---\n"); }
		if (DEBUG_DM) { printf("Piece To Place = %c\n", HashBoardPieceToChar(newMove->movePiece)); }
		board->theBoard[newMove->toPos] = HashBoardPieceToChar(newMove->movePiece);
		if (board->theBoard[newMove->toPos] == SMALLPIECE) {
			board->data->smallSandPiles--;
		}
		else if (board->theBoard[newMove->toPos] == LARGEPIECE) {
			board->data->largeSandPiles--;
		}
		else if (board->theBoard[newMove->toPos] == REDBUCKETPIECE) {
			board->data->redBuckets--;
		}
		else if (board->theBoard[newMove->toPos] == BLUEBUCKETPIECE) {
			board->data->blueBuckets--;
		}
	}
	else {
		if (DEBUG_DM) { printf("--- MOVE PIECE ---\n"); }
		pieceToMove = board->theBoard[newMove->fromPos - 1];
		pieceInWay = board->theBoard[newMove->toPos];
		if (DEBUG_DM) { printf("Piece To Move = %c\n", pieceToMove); }
		if (DEBUG_DM) { printf("Piece In Way = %c\n", pieceInWay); }
		board->theBoard[newMove->fromPos - 1] = BLANKPIECE;

		if (pieceInWay == BLANKPIECE) {
			board->theBoard[newMove->toPos] = pieceToMove;
		}
		else if (pieceInWay == SMALLPIECE) {
			if (pieceToMove == BLUEBUCKETPIECE) {
				board->theBoard[newMove->toPos] = BLUESMALLPIECE;
			}
			else if (pieceToMove == REDBUCKETPIECE) {
				board->theBoard[newMove->toPos] = REDSMALLPIECE;
			}
		}
		else if (pieceInWay == LARGEPIECE) {
			if (pieceToMove == SMALLPIECE) {
				board->theBoard[newMove->toPos] = CASTLEPIECE;
			}
			else if (pieceToMove == BLUESMALLPIECE) {
				board->theBoard[newMove->toPos] = BLUECASTLEPIECE;
				board->data->blueCastles++;
			}
			else if (pieceToMove == REDSMALLPIECE) {
				board->theBoard[newMove->toPos] = REDCASTLEPIECE;
				board->data->redCastles++;
			}
		}
		else if (pieceInWay == CASTLEPIECE) {
			if (pieceToMove == BLUEBUCKETPIECE) {
				board->theBoard[newMove->toPos] = BLUECASTLEPIECE;
				board->data->blueCastles++;
			}
			else if (pieceToMove == REDBUCKETPIECE) {
				board->theBoard[newMove->toPos] = REDCASTLEPIECE;
				board->data->redCastles++;
			}
		}
	}

	if (gWhosTurn == Blue) {
		board->theTurn = gWhosTurn = Red;
	}
	else {
		board->theTurn = gWhosTurn = Blue;
	}

	newPosition = arrayHash(board);

	if (DEBUG_DM) { printf("NEXT BOARD# = %d\n", (int)newPosition); }

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

VALUE Primitive(POSITION position) {

	BoardAndTurn board;

	printf("Primitive\n");

	board = arrayUnhash(position);

	if (((gWhosTurn == Blue) && (board->data->blueCastles >= NUMCASTLESTOWIN)) ||
		((gWhosTurn == Red) && (board->data->redCastles >= NUMCASTLESTOWIN))) {
		return win;
	}
	else if (((gWhosTurn == Blue) && (board->data->redCastles >= NUMCASTLESTOWIN)) ||
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

void PrintPosition(POSITION position, STRING playersName, BOOLEAN usersTurn)
{
	BoardAndTurn board;
	int i;

	printf("PrintPosition\n");

	board = arrayUnhash(position);
	char* theBoard = board->theBoard;
	if (DEBUG_PP) { printf("\nPOSITION# = %d\n", (int)position); }

	/***********************LINE 1**************************/
	printf("\n\n\n       *-----*-----*-----*\n");

	/***********************LINE 2,3,4**************************/
	printf("       |");
	printTopRow(1, theBoard);

	printf("\n       |");
	printMiddleRow(1, theBoard);
	printf("          +------------------------+");

	printf("\n       |");
	printBottomRow(1, theBoard);
	printf("          |    PIECES REMAINING    |");
	printf("\n");

	/***********************LINE 5**************************/
	printf("       *-----+-----+-----*");
	printf("          |------------------------|");

	/***********************LINE 6,7,8**************************/
	printf("\n       |");
	printTopRow(2, theBoard);
	printf("          |Large Piles  [l]:  ");
	for (i = 0; i < board->data->largeSandPiles; i++) {
		printf("*");
	}
	for (i = 0; i < (4 - board->data->largeSandPiles); i++) {
		printf(" ");
	}
	printf(" |");

	printf("\nBOARD: |");
	printMiddleRow(2, theBoard);
	printf("          |Small Piles  [s]:  ");
	for (i = 0; i < board->data->smallSandPiles; i++) {
		printf("*");
	}
	for (i = 0; i < (4 - board->data->smallSandPiles); i++) {
		printf(" ");
	}
	printf(" |");

	printf("\n       |");
	printBottomRow(2, theBoard);
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
	printTopRow(3, theBoard);
	printf("          +------------------------+");

	printf("\n       |");
	printMiddleRow(3, theBoard);

	printf("\n       |");
	printBottomRow(3, theBoard);
	printf("\n");

	/***********************LINE 7**************************/
	printf("       *-----*-----*-----*\n");

	/***********************LINE 8, 9, 10, 11**************************/

	printf("\n\n\n");

	//printf("\n%s\n\n", GetPrediction(position, playerName, usersTurn));
	SafeFree(board->data);
	SafeFree(board->theBoard);
	SafeFree(board);
}

void printTopRow(int rowNum, char* theBoard) {
	int i;
	for (i = (numCols * (rowNum - 1)); i < (numCols * rowNum); i++) {
		switch (theBoard[i]) {
		case BLUECASTLEPIECE:
			printf(BLUEBUCKETPIECENUMSTRING, i + 1);
			break;
		case REDCASTLEPIECE:
			printf(REDBUCKETPIECENUMSTRING, i + 1);
			break;
		default:
			printf(BLANKPIECENUMSTRING, i + 1);
		}
		printf("|");
	}
}

void printMiddleRow(int rowNum, char* theBoard) {
	int i;
	for (i = (numCols * (rowNum - 1)); i < (numCols * rowNum); i++) {
		if (theBoard[i] == CASTLEPIECE || theBoard[i] == BLUECASTLEPIECE || theBoard[i] == REDCASTLEPIECE) {
			printf(SMALLPIECESTRING);
		}
		else if (theBoard[i] == BLUESMALLPIECE) {
			printf(BLUEBUCKETPIECESTRING);
		}
		else if (theBoard[i] == REDSMALLPIECE) {
			printf(REDBUCKETPIECESTRING);
		}
		else {
			printf(BLANKPIECESTRING);
		}
		printf("|");
	}
}

void printBottomRow(int rowNum, char* theBoard) {
	int i;
	for (i = (numCols * (rowNum - 1)); i < (numCols * rowNum); i++) {
		if (theBoard[i] == LARGEPIECE || theBoard[i] == CASTLEPIECE || theBoard[i] == BLUECASTLEPIECE || theBoard[i] == REDCASTLEPIECE) {
			printf(LARGEPIECESTRING);
		}
		else if (theBoard[i] == SMALLPIECE || theBoard[i] == BLUESMALLPIECE || theBoard[i] == REDSMALLPIECE) {
			printf(SMALLPIECESTRING);
		}
		else if (theBoard[i] == BLUEBUCKETPIECE) {
			printf(BLUEBUCKETPIECESTRING);
		}
		else if (theBoard[i] == REDBUCKETPIECE) {
			printf(REDBUCKETPIECESTRING);
		}
		else {
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

void PrintComputersMove(MOVE computersMove, STRING computersName)
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

void PrintMove(MOVE move)
{
	if (DEBUG_PM) { printf("\n***** PRINT MOVE *****\n\n"); }

	GMove newMove = unhashMove(move);
	char toPrint;

	if (newMove->fromPos == 0) {
		if (newMove->movePiece == hBlueBucket) {
			toPrint = 'b';
		}
		else if (newMove->movePiece == hRedBucket) {
			toPrint = 'r';
		}
		else if (newMove->movePiece == hSmallSand) {
			toPrint = 's';
		}
		else if (newMove->movePiece == hLargeSand) {
			toPrint = 'l';
		}
		else {
			toPrint = 'x';
		}
		printf("%c%d", toPrint, newMove->toPos + 1);
	}
	else {
		printf("%d%d", newMove->fromPos, newMove->toPos + 1);
	}

	SafeFree(newMove);

	if (DEBUG_PM) { printf("\n***** END PRINT MOVE *****\n"); }
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

USERINPUT GetAndPrintPlayersMove(POSITION position, MOVE* move, STRING playersName)
{
	USERINPUT input;
	USERINPUT HandleDefaultTextInput();
	char* playerColor;

	printf("GetAndPrintPlayersMove\n");

	BoardAndTurn board = arrayUnhash(position);

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
			}
			else {
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

BOOLEAN ValidTextInput(STRING input)
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

MOVE ConvertTextInputToMove(STRING input)
{
	if (DEBUG_CTITM) { printf("\n***** CONVERT TEXT INPUT TO MOVE *****\n\n"); }
	char first = input[0], second = input[1];
	MOVE thisMove;
	GMove newMove = (GMove)SafeMalloc(sizeof(struct cleanMove));

	BoardAndTurn board = arrayUnhash(getFrontFromAllPositions());
	printf("ConvertTextInputToMove, getFrontFromAllPositions() call - %d\n", getFrontFromAllPositions());

	if (DEBUG_CTITM) {
		printf("First input = %c\nSecond input = %c\n", first, second);
	}

	newMove->fromPos = 0;
	newMove->toPos = (int)(second - '1');
	if (first == 'l') {
		newMove->movePiece = hLargeSand;
	}
	else if (first == 's') {
		newMove->movePiece = hSmallSand;
	}
	else if (first == 'b') {
		newMove->movePiece = hBlueBucket;
	}
	else if (first == 'r') {
		newMove->movePiece = hRedBucket;
	}
	else {
		newMove->fromPos = (int)(first - '0');
		newMove->movePiece = CharToHashBoardPiece(board->theBoard[newMove->fromPos - 1]);
	}

	thisMove = hashMove(newMove);

	if (DEBUG_CTITM) {
		printf("MOVE# = %d %x\n", thisMove, thisMove);
		printMove(newMove);
		printf("\n");
	}

	SafeFree(newMove);
	SafeFree(board->data);
	SafeFree(board->theBoard);
	SafeFree(board);

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

void GameSpecificMenu()
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

void SetTclCGameSpecificOptions(int options[])
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

POSITION GetInitialPosition()
{
	int i;
	BoardAndTurn board;
	char piece, turn;

	InitializeGame();

	board = (BoardAndTurn)SafeMalloc(sizeof(struct boardAndTurnRep));
	board->theBoard = (char*)SafeMalloc(boardSize * sizeof(char));

	getchar(); // for the enter after picking option 1 on the debug menu

	for (i = 1; i <= boardSize; i++) {
		do {
			printf("Input the character at cell %i followed by <enter>: ", i);
			piece = (char)getchar();
			getchar();
		} while (CharToBoardPiece(piece) == UNKNOWNBOARDPIECE);
		board->theBoard[i - 1] = piece;
	}

	do {
		printf("Input whose turn it is (1 for Blue, 2 for Red): ");
		turn = (char)getchar();
		getchar();
	} while ((turn != '1') && (turn != '2'));

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

int NumberOfOptions()
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

int getOption()
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

void setOption(int option)
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

void DebugMenu()
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

char* BoardPieceToString(BoardPiece piece) {
	char* pieceString = (char*)SafeMalloc(30 * sizeof(char));

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
	char* pieceString = (char*)SafeMalloc(30 * sizeof(char));

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

POSITION arrayHash(BoardAndTurn board) {
	POSITION position;
	if (gHashWindowInitialized) {
		TIER tier = BoardToTier(board);
		generic_hash_context_switch(tier);
		TIERPOSITION tierpos = generic_hash_hash(board->theBoard, board->theTurn);
		position = gHashToWindowPosition(tierpos, tier);
	}
	else {
		position = generic_hash_hash(board->theBoard, board->theTurn);
	}
	return position;
}

BoardAndTurn arrayUnhash(POSITION position) {
	BoardAndTurn board = (BoardAndTurn)SafeMalloc(sizeof(struct boardAndTurnRep));
	board->theBoard = (char*)SafeMalloc(boardSize * sizeof(char));
	board->data = (BoardData)SafeMalloc(sizeof(struct boardDataElements));
	if (gHashWindowInitialized) {
		TIERPOSITION tierpos;
		TIER tier;
		gUnhashToTierPosition(position, &tierpos, &tier);
		generic_hash_context_switch(tier);
		generic_hash_unhash(tierpos, board->theBoard);
		int small = 4, large = 4, redB = 2, blueB = 2, redC = 0, blueC = 0;
		for (int i = 0; i < boardSize; i++) {
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
		board->theTurn = generic_hash_turn(position);
		return board;
	}
	else {
		BoardAndTurn board = (BoardAndTurn)SafeMalloc(sizeof(struct boardAndTurnRep));
		board->theBoard = (char*)SafeMalloc(boardSize * sizeof(char));
		board->data = (BoardData)SafeMalloc(sizeof(struct boardDataElements));
		int small = 4, large = 4, redB = 2, blueB = 2, redC = 0, blueC = 0;
		generic_hash_unhash(position, board->theBoard);
		for (int i = 0; i < boardSize; i++) {
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
		board->theTurn = generic_hash_turn(position);
		return board;
	}
}

MOVE hashMove(GMove newMove) {
	if (DEBUG_M) { printf("\n********** HASH MOVE **********\n\n"); }

	if (DEBUG_M) {
		printMove(newMove);
		printf("\n");
	}

	if (newMove->fromPos == 0) {            // movePiece = first 2 bits ; toPos = next 4 bits
		if (DEBUG_M) { printf("MOVE# = %d\n", (((int)newMove->movePiece) | (newMove->toPos << 2))); }
		return ((int)newMove->movePiece) | (newMove->toPos << 2);
	}
	else {        // starting at bit 7 -> movePiece = first 4 bits ;
   // fromPos = next 4 bits ; toPos = next 4 bits
		if (DEBUG_M) { printf("MOVE# = %d\n", (((int)newMove->movePiece | (newMove->fromPos << 4) | (newMove->toPos << 4)) << 7)); }
		return ((int)newMove->movePiece | (newMove->fromPos << 4) | (newMove->toPos << 8)) << 6;
	}

	if (DEBUG_M) { printf("\n********** END HASH MOVE **********\n"); }
}

GMove unhashMove(MOVE newMove) {
	GMove toMove = (GMove)SafeMalloc(sizeof(struct cleanMove));

	if (DEBUG_UM) { printf("\n********** UNHASH MOVE **********\n\n"); }

	if (newMove < (1 << 6)) {
		toMove->movePiece = (HashBoardPiece)(((int)newMove) & 3);
		toMove->toPos = (((int)newMove) & (0xF << 2)) >> 2;
		toMove->fromPos = 0;
	}
	else {
		toMove->movePiece = (HashBoardPiece)(((int)newMove) & (0xF << 6)) >> 6;
		toMove->fromPos = (((int)newMove) & (0xF << 10)) >> 10;
		toMove->toPos = (((int)newMove) & (0xF << 14)) >> 14;
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
		printf("MOVE#  = %d\n", (int)move);
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
		printf("\nIMOVE# = %d\n", (int)moveToReturn);
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
		((int)move->movePiece));
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
	printf("gNumberOfPositions = %d\n\n", (int)gNumberOfPositions);
	for (i = 0; i < gNumberOfPositions; i++) {
		board = arrayUnhash((POSITION)i);
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
	PositionList newPosNode = (PositionList)SafeMalloc(sizeof(struct positionNode));
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

// Compress boards for hashing Tier & TierPosition easier
void CompressBoardandHash(int piecesArray[]) {
	if (ValidPiecesArray(piecesArray)) {
		printf("L:%d S:%d B:%d R:%d LS:%d SB:%d SR:%d LSB:%d LSR:%d BNK:%d\n", piecesArray[1], piecesArray[4], piecesArray[7], piecesArray[10], piecesArray[13], 
			piecesArray[16], piecesArray[19], piecesArray[22], piecesArray[25], piecesArray[28]);
		generic_hash_init(boardSize, piecesArray, NULL, 0);
	}
	int* nPiecesArray = (int*)SafeMalloc(31 * sizeof(int));
	// 1. Large and Small
	if (piecesArray[1] > 0 && piecesArray[4] > 0) {
		for (int i = 0; i < 31; i++) {
			nPiecesArray[i] = piecesArray[i];
		}
		nPiecesArray[1] = nPiecesArray[2] = piecesArray[1] - 1;
		nPiecesArray[4] = nPiecesArray[5] = piecesArray[4] - 1;
		nPiecesArray[13] = nPiecesArray[14] = piecesArray[13] + 1;
		nPiecesArray[28] = nPiecesArray[29] = piecesArray[28] + 1;
		CompressBoardandHash(nPiecesArray);
	}
	// 2. Small and BlueBucket
	if (piecesArray[4] > 0 && piecesArray[7] > 0) {
		for (int i = 0; i < 31; i++) {
			nPiecesArray[i] = piecesArray[i];
		}
		nPiecesArray[4] = nPiecesArray[5] = piecesArray[4] - 1;
		nPiecesArray[7] = nPiecesArray[8] = piecesArray[7] - 1;
		nPiecesArray[16] = nPiecesArray[17] = piecesArray[16] + 1;
		nPiecesArray[28] = nPiecesArray[29] = piecesArray[28] + 1;
		CompressBoardandHash(nPiecesArray);
	}
	// 3. Small and RedBucket
	if (piecesArray[4] > 0 && piecesArray[10] > 0) {
		for (int i = 0; i < 31; i++) {
			nPiecesArray[i] = piecesArray[i];
		}
		nPiecesArray[4] = nPiecesArray[5] = piecesArray[4] - 1;
		nPiecesArray[10] = nPiecesArray[11] = piecesArray[10] - 1;
		nPiecesArray[19] = nPiecesArray[20] = piecesArray[19] + 1;
		nPiecesArray[28] = nPiecesArray[29] = piecesArray[28] + 1;
		CompressBoardandHash(nPiecesArray);
	}
	// 4. Large and SmallBlue
	if (piecesArray[1] > 0 && piecesArray[16] > 0) {
		for (int i = 0; i < 31; i++) {
			nPiecesArray[i] = piecesArray[i];
		}
		nPiecesArray[1] = nPiecesArray[2] = piecesArray[1] - 1;
		nPiecesArray[16] = nPiecesArray[17] = piecesArray[16] - 1;
		nPiecesArray[22] = nPiecesArray[23] = piecesArray[22] + 1;
		nPiecesArray[28] = nPiecesArray[29] = piecesArray[28] + 1;
		CompressBoardandHash(nPiecesArray);
	}
	// 5. Large and SmallRed
	if (piecesArray[1] > 0 && piecesArray[19] > 0) {
		for (int i = 0; i < 31; i++) {
			nPiecesArray[i] = piecesArray[i];
		}
		nPiecesArray[1] = nPiecesArray[2] = piecesArray[1] - 1;
		nPiecesArray[19] = nPiecesArray[20] = piecesArray[19] - 1;
		nPiecesArray[25] = nPiecesArray[26] = piecesArray[25] + 1;
		nPiecesArray[28] = nPiecesArray[29] = piecesArray[28] + 1;
		CompressBoardandHash(nPiecesArray);
	}
	// 6. Castle and BlueBucket
	if (piecesArray[13] > 0 && piecesArray[7] > 0) {
		for (int i = 0; i < 31; i++) {
			nPiecesArray[i] = piecesArray[i];
		}
		nPiecesArray[13] = nPiecesArray[14] = piecesArray[13] - 1;
		nPiecesArray[7] = nPiecesArray[8] = piecesArray[7] - 1;
		nPiecesArray[22] = nPiecesArray[23] = piecesArray[22] + 1;
		nPiecesArray[28] = nPiecesArray[29] = piecesArray[28] + 1;
		CompressBoardandHash(nPiecesArray);
	}
	// 7. Castle and RedBucket
	if (piecesArray[13] > 0 && piecesArray[10] > 0) {
		for (int i = 0; i < 31; i++) {
			nPiecesArray[i] = piecesArray[i];
		}
		nPiecesArray[13] = nPiecesArray[14] = piecesArray[13] - 1;
		nPiecesArray[10] = nPiecesArray[11] = piecesArray[10] - 1;
		nPiecesArray[25] = nPiecesArray[26] = piecesArray[25] + 1;
		nPiecesArray[28] = nPiecesArray[29] = piecesArray[28] + 1;
		CompressBoardandHash(nPiecesArray);
	}
	SafeFree(nPiecesArray);
}

// helper function
BOOLEAN ValidPiecesArray(int piecesArray[]) {
	int totalPieces = 0;
	for (int index = 1; index < 28; index += 3) {
		totalPieces += piecesArray[index];
	}
	return (totalPieces <= 9 ? 1 : 0);
}

TIER BoardToTier(BoardAndTurn board) {
	char* theBoard = board->theBoard;
	TIER small = 0, large = 0, blueB = 0, redB = 0;
	int piece;
	for (int i = 0; i < boardSize; i++) {
		piece = theBoard[i];
		switch (piece)
		{
		case SMALLPIECE:
			small++;
			break;
		case LARGEPIECE:
			large++;
			break;
		case CASTLEPIECE:
			small++;
			large++;
			break;
		case BLUEBUCKETPIECE:
			blueB++;
			break;
		case REDBUCKETPIECE:
			redB++;
			break;
		case BLUESMALLPIECE:
			small++;
			blueB++;
			break;
		case REDSMALLPIECE:
			small++;
			redB++;
			break;
		case BLUECASTLEPIECE:
			small++;
			large++;
			blueB++;
			break;
		case REDCASTLEPIECE:
			small++;
			large++;
			redB++;
			break;
		}
	}
	return PiecesToTier(large, small, blueB, redB);
}

void SetupTierStuff() {
	// kSupportsTierGamesman
	kSupportsTierGamesman = TRUE;
	// All function pointers
	gTierChildrenFunPtr					= &TierChildren;
	gNumberOfTierPositionsFunPtr		= &NumberOfTierPositions;
	gGetInitialTierPositionFunPtr		= &GetInitialTierPosition;
	gIsLegalFunPtr						= &IsLegal;
	//gGenerateUndoMovesToTierFunPtr		= &GenerateUndoMovesToTier;
	//gUnDoMoveFunPtr						= &UnDoMove;
	gTierToStringFunPtr					= &TierToString;

	//Tier-Specific Hashes
	int TOTALTIERPOSITIONS = 225;
	int piecesArray[] = { LARGEPIECE, 0, 0, SMALLPIECE, 0, 0, 
		BLUEBUCKETPIECE, 0, 0, REDBUCKETPIECE, 0, 0, CASTLEPIECE, 0, 0,
		BLUESMALLPIECE, 0, 0, REDSMALLPIECE, 0, 0, BLUECASTLEPIECE, 0, 0,
		REDCASTLEPIECE, 0, 0, BLANKPIECE, 0, 0, -1 };
	for (int tier = 0; tier < TOTALTIERPOSITIONS; tier++) {
		piecesArray[1] = piecesArray[2] = tier / 45;
		piecesArray[4] = piecesArray[5] = (tier % 45) / 9;
		piecesArray[7] = piecesArray[8] = (tier % 9) % 3;
		if (tier % 9 <= 2) {
			piecesArray[10] = piecesArray[11] = 0;
		}
		else if (tier % 9 <= 5) {
			piecesArray[10] = piecesArray[11] = 1;
		}
		else {
			piecesArray[10] = piecesArray[11] = 2;
		}
		piecesArray[28] = piecesArray[29] = 9 - piecesArray[1] - piecesArray[4] - piecesArray[7] - piecesArray[10];
		CompressBoardandHash(piecesArray);
	}
	// initial tier
	gInitialTier = 0;
	// initial tierPosition
	BoardAndTurn boardArray = (BoardAndTurn)SafeMalloc(sizeof(struct boardAndTurnRep));
	boardArray->theBoard = (char*)SafeMalloc(boardSize * sizeof(char));
	for (int i = 0; i < boardSize; i++) {
		boardArray->theBoard[i] = BLANKPIECE;
	}
	boardArray->theTurn = Blue;
	gInitialTierPosition = arrayHash(boardArray);
}

TIERLIST* TierChildren(TIER tier) {
	TIERLIST* tierlist = NULL;
	TIER large, small, blueB, redB;
	large = tier / 45;
	small = (tier % 45) / 9;
	blueB = (tier % 9) % 3;
	if (tier % 9 <= 2) {
		redB = 0;
	}
	else if (tier % 9 <= 5) {
		redB = 1;
	}
	else {
		redB = 2;
	}
	if (large < 4) {
		tierlist = CreateTierlistNode(PiecesToTier(large + 1, small, blueB, redB), tierlist);
	}
	if (small < 4) {
		tierlist = CreateTierlistNode(PiecesToTier(large, small + 1, blueB, redB), tierlist);
	}
	if (blueB < 2) {
		tierlist = CreateTierlistNode(PiecesToTier(large, small, blueB + 1, redB), tierlist);
	}
	if (redB < 2) {
		tierlist = CreateTierlistNode(PiecesToTier(large, small, blueB, redB + 1), tierlist);
	}
	return tierlist;
}

//helper function
TIER PiecesToTier(TIER large, TIER small, TIER blueB, TIER redB) {
	return (45 * large + 9 * small + 3 * redB + blueB);
}

TIERPOSITION NumberOfTierPositions(TIER tier) {
	generic_hash_context_switch(tier);
	return generic_hash_max_pos();
}

void GetInitialTierPosition(TIER* tier, TIERPOSITION* tierposition) {
	BoardAndTurn board;
	char piece, turn;
	board = (BoardAndTurn)SafeMalloc(sizeof(struct boardAndTurnRep));
	board->theBoard = (char*)SafeMalloc(boardSize * sizeof(char));
	for (int i = 1; i <= boardSize; i++) {
		do {
			printf("Input the character at cell %i followed by <enter>: ", i);
			piece = (char)getchar();
			getchar();
		} while (CharToBoardPiece(piece) == UNKNOWNBOARDPIECE);
		board->theBoard[i - 1] = piece;
	}
	do {
		printf("Input whose turn it is (1 for Blue, 2 for Red): ");
		turn = (char)getchar();
		getchar();
	} while ((turn != '1') && (turn != '2'));

	board->theTurn = atoi(&turn);
	(*tier) = BoardToTier(board);
	generic_hash_context_switch(*tier);
	(*tierposition) = generic_hash_hash(board->theBoard, turn);
	SafeFree(board->theBoard);
	SafeFree(board);
}

BOOLEAN IsLegal(POSITION position) {
	return TRUE;
}

/* UNDOMOVELIST* GenerateUndoMovesToTier(POSITION position, TIER tier) {
	return NULL;
} */

/* POSITION UnDoMove(POSITION position, UNDOMOVE undomove) {
	return NULL;
} */

STRING TierToString(TIER tier) {
	STRING tierStr = (STRING)SafeMalloc(30 * sizeof(char));
	TIER large, small, blueB, redB;
	large = tier / 45;
	small = ((tier % 45) / 9);
	blueB = (tier % 9) % 3;
	if (tier % 9 <= 2) {
		redB = 0;
	}
	else if (tier % 9 <= 5) {
		redB = 1;
	}
	else {
		redB = 2;
	}
	sprintf(tierStr, "large = %llu, small = %llu, red = %llu, blue = %llu", large, small, redB, blueB);
	return tierStr;
}

POSITION InteractStringToPosition(STRING board) {
	POSITION pos = INVALID_POSITION;
	GetValue(board, "pos", GetUnsignedLongLong, &pos);
	return pos;
}

STRING InteractPositionToString(POSITION pos) {
	BoardAndTurn bt = arrayUnhash(pos);
	char* out = MakeBoardString(bt->theBoard,
		"turn", StrFromI(bt->theTurn),
		"smallSandPiles", StrFromI(bt->data->smallSandPiles),
		"largeSandPiles", StrFromI(bt->data->largeSandPiles),
		"redBuckets", StrFromI(bt->data->redBuckets),
		"blueBuckets", StrFromI(bt->data->blueBuckets),
		"redCastles", StrFromI(bt->data->redCastles),
		"blueCastles", StrFromI(bt->data->blueCastles),
		"pos", StrFromI(pos),
		"");
	SafeFree(bt);
	return out;
}

STRING MoveToString(MOVE theMove)
{
	STRING moveStr = (STRING)SafeMalloc(sizeof(char) * 2);

	return "1";
}

STRING InteractPositionToEndData(POSITION Pos) {
	return NULL;
}

STRING InteractMoveToString(POSITION pos, MOVE mv) {
	return MoveToString(mv);
}
