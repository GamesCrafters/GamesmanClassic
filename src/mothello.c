/************************************************************************
**
** NAME:        Othello
**
** DESCRIPTION: AKA Reversi
**
** AUTHOR:      Michael Chen
**              Robert Liao
**				Robert Shi
**
** DATE:        1 May 2004
**************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**
*************************************************************************/

#include "gamesman.h"

extern STRING gValueString[];

POSITION gNumberOfPositions =
    0; /* The number of total possible positions | If you are using our hash,
          this is given by the hash_init() function*/

POSITION gInitialPosition = 593531; /* The initial position (starting board) */
POSITION kBadPosition = -1;         /* A position that will never be used */

CONST_STRING kGameName = "Othello"; /* The name of your game */
CONST_STRING kDBName = "othello";   /* The name to store the database under */
CONST_STRING kAuthorName = "Michael Chen and Robert Liao v1, Robert Shi v2";

/* A partizan game is a game where each player has different moves from the same
 * board (chess - different pieces) */
BOOLEAN kPartizan = TRUE;
BOOLEAN kDebugMenu = TRUE;        /* TRUE while debugging */
BOOLEAN kGameSpecificMenu = TRUE; /* TRUE if there is a game specific menu*/
BOOLEAN kTieIsPossible = TRUE;    /* TRUE if a tie is possible */
BOOLEAN kLoopy = FALSE; /* TRUE if the game tree will have cycles (a rearranger
                           style game) */
BOOLEAN kDebugDetermineValue = FALSE; /* TRUE while debugging */
void* gGameSpecificTclInit = NULL;

CONST_STRING kHelpGraphicInterface =
    "No Graphic Interface with Othello Right Now";

CONST_STRING kHelpTextInterface =
    "Enter the coordinate of where you would like to put your piece.\n\
Example: b4\n\
When you cannot place any pieces, hit 'd' to pass.";

CONST_STRING kHelpOnYourTurn =
    "You can place a piece on any blank spot such that it has at least\n\
one of your opponent's pieces adjacent to that spot, and the line\n\
from the blank spot and the opponent's piece has, at the end of it,\n\
one of your own pieces, with all of your opponent's pieces in between.";

CONST_STRING kHelpStandardObjective =
    "When neither player can move (usually this means the entire board\n\
has been filled), be the player with the most pieces.";

CONST_STRING kHelpReverseObjective =
    "When neither player can move (usually this means the entire board\n\
has been filled), be the player with the least number of pieces.";

CONST_STRING kHelpTieOccursWhen = /* Should follow 'A Tie occurs when... */
    "A tie can occur in Othello when neither player can move and both\n\
players have an equal number of pieces.";

CONST_STRING kHelpExample = "Help Example Unavailable.";

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

#define DEBUG 0  // CHANGED FROM 0
#define DEBUGHELPERS 0
#define SOLVERCOUNTER 0

#define BLACK 1
#define WHITE 2
#define BLACKPIECE 'B'
#define WHITEPIECE 'W'
#define BLANKPIECE ' '
#define PASSMOVE -1
#define USERINDICATESPASS 'D'
#define BLANKBOARDPOSITION 0

#define DEFAULTROWS 4
#define DEFAULTCOLS 4
#define MAXROWS                                                      \
    7             /*Using 32-bit architecture, 4X4 yields 86 million \
                     positions*/
#define MAXCOLS 7 /*The 4X5 or 5X4 case yields about 6 billion*/

/*************************************************************************
**
** Below is where you put your #define's and your global variables, structs
**
*************************************************************************/

int solvercounter = 0;
int solvercountermax;
BOOLEAN variant_NoGenMovesRestriction = 0;

int OthRows = 4;
int OthCols = 4;

char start_standard_board[] = {
    ' ', ' ', ' ', ' ', ' ', 'B', 'W', ' ',
    ' ', 'W', 'B', ' ', ' ', ' ', ' ', ' ',
};

/*************************************************************************
**
** Above is where you put your #define's and your global variables, structs
**
*************************************************************************/

void init_board_hash(void);

int PrintPositionRow(int);
void PrintBoard(char[]);

POSITION SetupInitialPosition(void);

void tallyPieces(char* board, int* black, int* white, int* blank);
WINBY computeWinBy(POSITION);
int AddRemovePieces(char[], int, char);
void UserSelectRows(void);
void UserSelectCols(void);
void ChangeRows(int rows);
void ChangeCols(int cols);
POSITION MakeInitialSquare(void);

int InvertRow(int);
int CoordtoArrayNum(int, int);
void ArrayNumtoCoord(int, int[]);
BOOLEAN ValidCoord(int[]);
void Go1Direction(int[], int);
BOOLEAN Check1Spot1Direc(int, char[], char, char, int);
int oppositeturn(int);
int oppositedirection(int);
BOOLEAN IsPlayableBoard(char[]);

BOOLEAN quickgeneratemoves(char[], int);
void PositionToString(POSITION position, char *positionStringBuffer);
char* getBoard(POSITION);
char* getBlankBoard(void);

extern BOOLEAN (*gGoAgain)(POSITION, MOVE);

POSITION ActualNumberOfPositions(int variant);

/* TIER-SPECIFIC FUNCTION DECLARATIONS */
TIERLIST* TierChildren(TIER tier);
TIERPOSITION NumberOfTierPositions(TIER tier);
TIER BoardToTier(char* board);
TIER PositionToTier(POSITION position);
STRING TierToString(TIER tier);
POSITION getPosition(char* board, int player);
int getTurn(POSITION pos);
void SetupTierStuff(void);
void GetInitialTierPosition(TIER* tier, TIERPOSITION* tierposition);
BOOLEAN IsLegal(POSITION position);
int whoseTurnGivenTier(TIER tier);
int tierToNumPieces(TIER tier);
void PrintBoard(char[]);

/************************************************************************
**
** NAME:        InitializeGame
**
** DESCRIPTION: Initialize the gDatabase, a global variable. and the other
**              local variables.
**
************************************************************************/

void InitializeGame(void) {
    int hash_data[] = {
        BLANKPIECE, 0, OthCols * OthRows, WHITEPIECE, 0, OthCols * OthRows,
        BLACKPIECE, 0, OthCols * OthRows, -1};
#if DEBUG
    printf("InitializeGame() Running...\n")
        /* Initialize Hash Function */

        printf("InitializeGame() --> generic_hash_init\n");
    max = generic_hash_init(OthCols * OthRows, hash_data, NULL, 0);
#endif

    if (SOLVERCOUNTER)
        solvercountermax =
            generic_hash_init(OthCols * OthRows, hash_data, NULL, 0);

#if DEBUG
    printf("InitializeGame() <-- generic_hash_init: %d\n", max);
    printf("InitalizeGame() --> generic_hash_hash\n");
    init = getPosition(start_standard_board, BLACK);
    printf("INIT CURRENT BOARD\n, START%s, %cEND", start_standard_board,
           start_standard_board[14]);
    printf("InitializeGame() <-- generic_hash: %d\n", init);
#endif

    init_board_hash();
    int rots[3] = {90, 180, 270};
    int refs[4] = {0, 45, 90, 135};
    if (OthRows == OthCols) {
        generic_hash_init_sym(0, OthRows, OthCols, refs, 4, rots, 3, 0);
    } else {
        rots[0] = 180;
        refs[0] = 0;
        refs[1] = 90;
        generic_hash_init_sym(0, OthRows, OthCols, refs, 2, rots, 1, 0);
    }

#if DEBUG
    char* test_board;
    printf("Hash Test. Unhashed Board. Hash Value %d\n", init);
    test_board = getBoard(gInitialPosition);
    printf("\nBoard is... %s\n", test_board);
    printf("InitializeGame() Done\n");
    free(test_board);
#endif

    fflush(stdout);
    gPutWinBy = &computeWinBy;
    gActualNumberOfPositionsOptFunPtr = &ActualNumberOfPositions;
    gPositionToStringFunPtr = &PositionToString;

    // Setup Tier Stuff
    // SetupTierStuff();
}
/************************************************************************
**
** NAME:        DebugMenu
**
** DESCRIPTION: Menu used to debug internal problems. Does nothing if
**              kDebugMenu == FALSE
**
************************************************************************/

void DebugMenu(void) {}

/************************************************************************
**
** NAME:        GameSpecificMenu
**
** DESCRIPTION: Menu used to change game-specific parmeters, such as
**              the side of the board in an nxn Nim board, etc. Does
**              nothing if kGameSpecificMenu == FALSE
**
*******************************origin*****************************************/

void GameSpecificMenu(void) {
    char selection_command;
    char selection = 'Z';
    // POSITION GetInitialPosition();
    do {
        printf("\n\t----- Game Specific Options for Othello ----- \n\n");
        printf("\tCurrent Number of Maximum Positions: " POSITION_FORMAT,
               gNumberOfPositions);
        printf("\n\n");
        printf("\tm)\t(M)odify Board\n");
        if (variant_NoGenMovesRestriction) {
            printf(
                "\tp)\tSwitch mode from '(P)lace pieces anywhere'\n\t\tto "
                "'Regular piece restrictions'\n");
        } else {
            printf(
                "\tp)\tSwitch mode from 'Regular piece restrictions'\n\t\tto "
                "'(P)lace pieces anywhere'\n");
        }
        printf("\tb)\t(B)ack to previous screen\n\n");
        printf("Please select an option: ");
        selection_command = GetMyChar();
        selection = toupper(selection_command);
        switch (selection) {
            case 'M':
                gInitialPosition = SetupInitialPosition();
                selection = 'Z';
                break;
            case 'P':
                variant_NoGenMovesRestriction =
                    (variant_NoGenMovesRestriction) ? 0 : 1;
                selection = 'Z';
                break;
            case 'B':
                return;
            default:
                printf("Invalid Option.\n");
                selection = 'Z';
                break;
        }
    } while (selection != 'B');
}

/************************************************************************
**
** NAME:        DoMove
**
** DESCRIPTION: Apply the move to the position.
**
** INPUTS:      POSITION thePosition : The old position
**              MOVE     theMove     : The move to apply.
**
** OUTPUTS:     (POSITION) : The position that results after the move.
**
** CALLS:       Hash ()
**              Unhash ()
**	            LIST OTHER CALLS HERE
*************************************************************************/

POSITION DoMove(POSITION thePosition, MOVE theMove) {
    int candidatemove[2], CandidateArrayNum, j, whoseturn, nextplayer;
    char ownpiece, opponentpiece;
    char* board;
    int MoveArrayNum = (int)theMove;

	board = getBoard(thePosition);
	whoseturn = getTurn(thePosition);

    // assigning opponent pieces and own pieces
    if (whoseturn == 1) {
        ownpiece = 'B';
        opponentpiece = 'W';
        nextplayer = 2;
    } else {
        ownpiece = 'W';
        opponentpiece = 'B';
        nextplayer = 1;
    }
    if (MoveArrayNum == PASSMOVE) return getPosition(board, nextplayer);
    board[MoveArrayNum] = ownpiece;
    for (j = 1; j < 9; j++) {
        CandidateArrayNum = MoveArrayNum;
        ArrayNumtoCoord(CandidateArrayNum, candidatemove);
        if (Check1Spot1Direc(MoveArrayNum, board, ownpiece, opponentpiece, j)) {
            Go1Direction(candidatemove, j);
            CandidateArrayNum =
                CoordtoArrayNum(candidatemove[0], candidatemove[1]);
            while (board[CandidateArrayNum] == opponentpiece) {
                board[CandidateArrayNum] = ownpiece;
                Go1Direction(candidatemove, j);
                CandidateArrayNum =
                    CoordtoArrayNum(candidatemove[0], candidatemove[1]);
            }
        }
    }
    return getPosition(board, nextplayer);
}

/************************************************************************
**
** NAME:        GetInitialPosition
**
** DESCRIPTION: returns the initial position of this board.
**
************************************************************************/

POSITION GetInitialPosition() {
    if (gInitialPosition == 0) init_board_hash();
    return gInitialPosition;
}

/************************************************************************
**
** NAME:        SetupInitialPosition
**
** DESCRIPTION: Ask the user for an initial position for testing. Store
**              it in the space pointed to by initialPosition;
**
** OUTPUTS:     POSITION initialPosition : The position to fill.
**
************************************************************************/

POSITION SetupInitialPosition() {
    char selection_command;
    char selection = 'Z';
    char* board;
    int blacktally, whitetally, blanktally;

    do {
        board = getBoard(gInitialPosition);
        tallyPieces(board, &blacktally, &whitetally, &blanktally);
        printf("\n\t----- Othello Initial Position Setup ----- \n\n");
        printf("\tCurrent Number of Maximum Positions: " POSITION_FORMAT,
               gNumberOfPositions);
        printf("\n\n");
        printf("\tCurrent Board\n");
        PrintBoard(board);
        printf("\n\tl)\tAdd/Remove B(L)ack Pieces\n");
        printf("\tw)\tAdd/Remove (W)hite Pieces\n");
        printf("\tr)\tChange number of (R)ows\n");
        printf("\tc)\tChange number of (C)olumns\n");
        if (blacktally > 1 && whitetally > 1) {
            printf("\tb)\t(B)ack to previous menu\n\n");
        } else {
            printf(
                "\n\tWARNING: You must have at least 2 black pieces and\n\t2 "
                "white pieces to play Othello!\n\n");
        }
        printf("Selection: ");
        selection_command = GetMyChar();
        selection = toupper(selection_command);

        switch (selection) {
            case 'L':
                blacktally = AddRemovePieces(board, blacktally, BLACKPIECE);
                selection = 'Z';
                break;
            case 'W':
                whitetally = AddRemovePieces(board, whitetally, WHITEPIECE);
                selection = 'Z';
                break;
            case 'R':
                UserSelectRows();
                break;
            case 'C':
                UserSelectCols();
                break;
            case 'B':
                if (whitetally < 2 || blacktally < 2) selection = 'Z';
                break;
            default:
                printf("Invalid option. Try again\n");
                selection = -1;
        }
    } while (selection != 'B');

    init_board_hash();

    // getPosition will free board.
    gInitialPosition = getPosition(board, BLACK);
    return gInitialPosition;
}

void init_board_hash(void) {
	int hash_data[] = { BLANKPIECE, 0, OthCols * OthRows,
		                WHITEPIECE, 0, OthCols * OthRows,
		                BLACKPIECE, 0, OthCols * OthRows, -1 };
	POSITION max;
	POSITION init;
	max = generic_hash_init(OthCols * OthRows, hash_data, NULL, 0);
	//init = generic_hash_hash(start_standard_board, BLACK);
	init = MakeInitialSquare();
	gInitialPosition = init;
	gNumberOfPositions = max;
}

/************************************************************************
**
** NAME:        PrintComputersMove
**
** DESCRIPTION: Nicely format the computers move.
**
** INPUTS:      MOVE    computersMove : The computer's move.
**              STRING  computersName : The computer's name.
**
************************************************************************/

void PrintComputersMove(MOVE computersMove, STRING computersName) {
    int ArrayNum, move[2];
    char alphabet[] = "abcdefghijklmnopqrstuvwxyz";

    ArrayNum = (int)computersMove;

    if (ArrayNum == PASSMOVE) {
        printf("%s is unable to move. %s passes\n", computersName,
               computersName);
    } else {
        ArrayNumtoCoord(ArrayNum, move);
        printf("%s moved to %c%d\n\n", computersName, alphabet[move[1] - 1],
               InvertRow(move[0]));
    }
}

/************************************************************************
**
** NAME:        Primitive
**
** DESCRIPTION: Return the value of a position if it fulfills certain
**              'primitive' constraints. Some examples of this is having
**              three-in-a-row with Gobblet. Three in a row for the player
**              whose turn it is a win, otherwise its a loss.
**              Otherwise undecided.
**
** INPUTS:      POSITION position : The position to inspect.
**
** OUTPUTS:     (VALUE) an enum which is oneof: (win,lose,tie,undecided)
**
** CALLS:       LIST FUNCTION CALLS
**
**
************************************************************************/

VALUE Primitive(POSITION pos) {
    int blanktally, blacktally, whitetally, whoseturn;
    char* board;

    board = getBoard(pos);
    whoseturn = (int)getTurn(pos);
    tallyPieces(board, &blacktally, &whitetally, &blanktally);
    if (blanktally != 0) {
        if (quickgeneratemoves(board, whoseturn)) {
            free(board);
            return undecided;
        } else if (quickgeneratemoves(board, oppositeturn(whoseturn))) {
            free(board);
            return undecided;
        }
    }
    free(board);

    if (blacktally == whitetally) return tie;
    if (blacktally > whitetally) {
        if (whoseturn == 1)
            return (gStandardGame ? win : lose);  // blackwin
        else
            return (gStandardGame ? lose : win);  // whitelose
    }
    if (whitetally > blacktally) {
        if (whoseturn == 1)
            return (gStandardGame ? lose : win);  // blacklose
        else
            return (gStandardGame ? win : lose);  // whitewin
    }
    printf("ERROR: Primitive returns no value");

    /*control should never reach here*/
    return undecided;
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
** CALLS:       Unhash()
**              GetPrediction()
**              LIST OTHER CALLS HERE
**
************************************************************************/

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn) {
    char alphabet[] = "abcdefghijklmnopqrstuvwxyz", owncolor[6];
    char* board;
    int blanktally, whitetally, blacktally;
    int whoseturn = getTurn(position);
    /*int strlenname = strlen(playerName);*/
    char turnString1[80], turnString2[80], prediction[80];

    // for loops inits
    int i, j, alpha, hyphens;

    /*Information gathering*/
    board = getBoard(position);
    tallyPieces(board, &blacktally, &whitetally, &blanktally);
    if (whoseturn == 1)
        sprintf(owncolor, "Black");
    else
        sprintf(owncolor, "White");

    /*Start Printing Board*/

    /*heading*/
    printf("\t+");
    for (hyphens = 0; hyphens < (2 * OthCols) + 25; hyphens++) printf("-");
    printf("+");

    printf("\n\t| GAMESMAN Othello");
    for (hyphens = 0; hyphens < (2 * OthCols) + 8; hyphens++) printf(" ");
    printf("|\n\t+");

    for (hyphens = 0; hyphens < (2 * OthCols) + 9; hyphens++) printf("-");
    printf("+");
    for (hyphens = 0; hyphens < 15; hyphens++) printf("-");
    printf("+\n\t|");

    for (hyphens = 0; hyphens < (2 * OthCols) + 9; hyphens++) printf(" ");
    printf("|               |\n");

    // top Row Alphabet Legend
    printf("\t|    ");
    for (alpha = 0; alpha < OthCols; alpha++) printf(" %c", alphabet[alpha]);
    printf("     |");

    // End Alphabet Legend

    for (hyphens = 0; hyphens < 15; hyphens++) printf(" ");
    printf("|\n");

    // Main Board Thingie
    for (i = 0; i < (2 * OthRows + 1); i++) {
        if (i % 2 == 0) {
            // Even rows are hyphen-lines
            printf("\t|    ");
            for (hyphens = 0; hyphens <= (2 * OthCols); hyphens++) printf("-");
            printf("    |               |\n");  // End Even rows
        } else {
            // Start Odd Rows, the ones that matter
            printf("\t|  %d ", InvertRow(PrintPositionRow(i)));
            for (j = 0; j < OthCols; j++) {
                printf("|%c",
                       board[CoordtoArrayNum(PrintPositionRow(i), (j + 1))]);
            }
            if (InvertRow(PrintPositionRow(i)) == 1) {
                if (whitetally <= 9)
                    printf("| 1  |   White: 0%d   |\n", whitetally);
                else
                    printf("| 1  |   White: %d   |\n", whitetally);
            } else if (InvertRow(PrintPositionRow(i)) == 2) {
                if (blacktally <= 9)
                    printf("| 2  |   Black: 0%d   |\n", blacktally);
                else
                    printf("| 2  |   Black: %d   |\n", blacktally);
            } else {
                printf("| %d  |               |\n",
                       InvertRow(PrintPositionRow(i)));
            }
        }
    }
    free(board);

    // Bottom Row Alphabet Legend
    printf("\t|    ");
    for (alpha = 0; alpha < OthCols; alpha++) {
        printf(" %c", alphabet[alpha]);
    }
    printf("     |               |\n\t|");
    // End Alphabet Legend

    // End stuff
    for (hyphens = 0; hyphens < (2 * OthCols) + 9; hyphens++) printf(" ");
    printf("|               |\n\t+");

    for (hyphens = 0; hyphens < (2 * OthCols) + 9; hyphens++) printf("-");
    printf("+");
    for (hyphens = 0; hyphens < 15; hyphens++) printf("-");
    printf("+\n");

    // Player Name, Predictions. Stolen directly from Asalto
    sprintf(turnString1, "| It is %s's turn.", playerName);
    printf("\t%s", turnString1);
    if ((int)strlen(turnString1) < (2 * OthCols) + 26)
        for (hyphens = 0;
             hyphens < (2 * OthCols) + 26 - (int)strlen(turnString1); hyphens++)
            printf(" ");
    printf("|\n");

    sprintf(turnString2, "| %s is playing %s", playerName, owncolor);
    printf("\t%s", turnString2);
    if ((int)strlen(turnString2) < (2 * OthCols) + 26)
        for (hyphens = 0;
             hyphens < (2 * OthCols) + 26 - (int)strlen(turnString2); hyphens++)
            printf(" ");
    printf("|\n");

    sprintf(prediction, "| %s", GetPrediction(position, playerName, usersTurn));
    if (prediction[2] == '(') {
        printf("\t%s", prediction);
        if ((int)strlen(prediction) < (2 * OthCols) + 26)
            for (hyphens = 0;
                 hyphens < (2 * OthCols) + 26 - (int)strlen(prediction);
                 hyphens++)
                printf(" ");
        printf("|\n");
    }

    // End
    printf("\t+");
    for (hyphens = 0; hyphens < (2 * OthCols) + 25; hyphens++) printf("-");
    printf("+\n\n");
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
** CALLS:       GENERIC_PTR SafeMalloc(int)
**              LIST OTHER CALLS HERE
**
************************************************************************/

MOVELIST *GenerateMoves(POSITION position) {
	int i, j, whoseturn = getTurn(position);
	char* board;
	char ownpiece, opponentpiece;
	int AnyMovesAtAll = 0;
	int move[2];
	int numsolvercounter;
	MOVELIST *head = NULL;

	if (SOLVERCOUNTER && (solvercounter != -1)) {
		numsolvercounter = printf("%d / %d Positions Solved", solvercounter, solvercountermax);
		for (i = 0; i < numsolvercounter; i++) printf("\b");
		solvercounter++;
		if (solvercounter > solvercountermax - 1) {
			solvercounter = -1;
		}
	}

    board = getBoard(position);
    /*assigning opponent pieces and own pieces*/
    if (whoseturn == 1) {
        ownpiece = 'B';
        opponentpiece = 'W';
    } else {
        ownpiece = 'W';
        opponentpiece = 'B';
    }

    for (i = 0; i < (OthRows * OthCols); i++)
        if (board[i] == BLANKPIECE) {
            if (variant_NoGenMovesRestriction) {
                head = CreateMovelistNode(i, head);
                AnyMovesAtAll = 1;
            } else
                for (j = 1; j < 9; j++) {
                    /*Each spot has 8 directions*/
                    if (Check1Spot1Direc(i, board, ownpiece, opponentpiece,
                                         j)) {
                        /*check to see if it's a good move, then go in that
                         * direction to the opponent's piece*/
                        AnyMovesAtAll = 1;
                        ArrayNumtoCoord(i, move);
                        head = CreateMovelistNode(i, head);
                        break;
                    }
                }
        }
    free(board);
    if (!AnyMovesAtAll) head = CreateMovelistNode(PASSMOVE, head);

	return head;
}

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
** CALLS:       validMove(MOVE, POSITION)
**              BOOLEAN PrintPossibleMoves(POSITION) ...Always True!
**
************************************************************************/

USERINPUT GetAndPrintPlayersMove (POSITION thePosition, MOVE *theMove, STRING playerName) {
	USERINPUT ret;
	char alphabet[] = "abcdefghijklmnopqrstuvwxyz";

	do {
		printf("%8s's move [(u)ndo/([a-%c][1-%d])] :  ", \
		       playerName, alphabet[OthCols - 1], OthRows);

		ret = HandleDefaultTextInput(thePosition, theMove, playerName);

		if (ret != Continue) return ret;
	}
	while (TRUE);
	return Continue; /* this is never reached, but lint is now happy */
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
** OUTPUTS:     BOOLEAN : TRUE if the input is a valid text input.
**
************************************************************************/

BOOLEAN ValidTextInput (STRING input) {
	char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	size_t len = strlen(input);
	if (len == 1) {
		input[0] = toupper(input[0]);
		return (input[0] == USERINDICATESPASS);
	} else if (len == 2) {
		input[0] = toupper(input[0]);
		input[1] = toupper(input[1]);
		return (('A' <= input[0] && input[0] <= alphabet[OthCols - 1])
		        && ('1' <= input[1] && input[1] <= '9'));
	} else if (len == 3) {
		input[0] = toupper(input[0]);
		input[1] = toupper(input[1]);
		input[2] = toupper(input[2]);
		return (('A' <= input[0] && input[0] <= alphabet[OthCols - 1])
		        && ('1' <= input[1] && input[1] <= '9')
		        && ('0' <= input[2] && input[2] <= '9'));
	}
	return FALSE;
}

/************************************************************************
**
** NAME:        ConvertTextInputToMove
**
** DESCRIPTION: Convert the string input to the internal move representation.
**              No checking if the input is valid is needed as it has
**              already been checked!
**
** INPUTS:      STRING input : The string input the user typed.
**
** OUTPUTS:     MOVE : The move corresponding to the user's input.
**
************************************************************************/

MOVE ConvertTextInputToMove(STRING input) {
    int move[2];
    size_t len = strlen(input);

    if (len == 1) {
        return (MOVE)PASSMOVE;
    } else if (len == 2) {
        input[0] = toupper(input[0]);
        move[1] = (int)(input[0] - 'A' + 1);
        move[0] = InvertRow((int)(input[1] - '0'));
        return (MOVE)CoordtoArrayNum(move[0], move[1]);
    } else if (len == 3) {
        input[0] = toupper(input[0]);
        move[1] = (int)(input[0] - 'A' + 1);
        move[0] =
            InvertRow((((int)(input[1] - '0')) * 10 + ((int)(input[2] - '0'))));
        return (MOVE)CoordtoArrayNum(move[0], move[1]);
    }
    /* control should nenvre reach here */
    return (MOVE)0;
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

void MoveToString(MOVE theMove, char *moveStringBuffer) {
    if ((int)theMove == PASSMOVE) {
        snprintf(moveStringBuffer, 6, "P");
    } else {
        snprintf(moveStringBuffer, 6, "%c%d", theMove % OthCols + 'a',
                InvertRow(theMove / OthCols + 1));
    }
}

/************************************************************************
**
** NAME:        NumberOfOptions
**
** DESCRIPTION: Calculates and returns the number of option combinations
**				there are with all the game variations you
*program.
**
** OUTPUTS:     int : the number of option combination there are.
**
************************************************************************/

int NumberOfOptions(void) { return INT_MAX; }

/************************************************************************
**
** NAME:        getOption
**
** DESCRIPTION: A hash function to keep track of all the game variants.
**		Should return a different number for each set of
**		variants.
**
** OUTPUTS:     int : the number representation of the options.
**
************************************************************************/

int getOption(void) {
    return (int)((OthCols << 5) + (OthRows << 1) +
                 variant_NoGenMovesRestriction);
}

/************************************************************************
**
** NAME:        setOption
**
** DESCRIPTION: The corresponding unhash for the game variants.
**				Should take the input and set all the
*appropriate *				variants.
**
** INPUT:     int : the number representation of the options.
**
************************************************************************/

void setOption(int option) {
    variant_NoGenMovesRestriction = option & 0x01;
    ChangeRows((option >> 0x01) & 0x0f);
    ChangeCols(option >> 0x05);
    init_board_hash();
}

/************************************************************************
*************************************************************************
**         EVERYTHING BELOW THESE LINES IS LOCAL TO THIS FILE
*************************************************************************
************************************************************************/

/************************************************************************
** This is where you can put any helper functions, including your
** hash and unhash functions if you are not using one of the existing
** ones.
************************************************************************/

int PrintPositionRow(int i) { return ((i + 1) / 2); }

int InvertRow(int i) { return ((OthRows + 1) - i); }

int CoordtoArrayNum(int row, int col) {
    return ((row - 1) * OthCols + col - 1);
}

void ArrayNumtoCoord(int arraynum, int move[]) { /*(row, col) order*/
    if (DEBUGHELPERS)
        printf("\nArrayNumtoCoord starting\nArrayNum %d\n", arraynum);
    move[0] = (arraynum / OthCols) + 1;
    move[1] = (arraynum % OthCols) + 1;
    if (DEBUGHELPERS) printf("Row: %d\nCol: %d\n\n", move[0], move[1]);
}

int oppositeturn(int turn) { return ((turn == BLACK) ? WHITE : BLACK); }

int oppositedirection(int direction) {
    switch (direction) {
        case 1:
            return 3;
        case 2:
            return 4;
        case 3:
            return 1;
        case 4:
            return 2;
        case 5:
            return 7;
        case 6:
            return 8;
        case 7:
            return 5;
        case 8:
            return 6;
        default:
            printf("Error: oppositeturn");
    }
    /*control should not be here*/
    return 0;
}

BOOLEAN ValidCoord(int coord[]) {
    return ((coord[0] > 0 && coord[0] <= OthRows) &&
            (coord[1] > 0 && coord[1] <= OthCols));
}

BOOLEAN IsPlayableBoard(char board[]) {
    int blacktally, whitetally;
    tallyPieces(board, &blacktally, &whitetally, NULL);
    if (whitetally >= 2 && blacktally >= 2) return TRUE;
    return FALSE;
}

void UserSelectRows() {
    int row = -1;

    do {
        printf("\nPlease enter a row number between 2 and %d: ", MAXROWS + 1);
        row = GetMyInt();
        if (row > 2 && row <= MAXROWS) OthRows = row;
    } while (row <= 2 || (row > MAXROWS));

    printf("Rows have now been set to %d.\n", OthRows);
    init_board_hash();
    gInitialPosition = MakeInitialSquare();
}

void UserSelectCols() {
    int col = -1;

    do {
        printf("\nPlease enter a column number between 2 and %d: ",
               MAXCOLS + 1);
        col = GetMyInt();
        if (col > 2 && col <= MAXCOLS) OthCols = col;
    } while (col <= 2 || (col > MAXCOLS));

    printf("Columns have now been set to %d.\n", OthCols);
    init_board_hash();

    gInitialPosition = MakeInitialSquare();
}

void ChangeRows(int row) {
    if (row > 2 && row <= MAXROWS) {
        OthRows = row;
        init_board_hash();
        gInitialPosition = MakeInitialSquare();
    } else {
        printf("Bad number of rows");
    }
}

void ChangeCols(int cols) {
    if (cols > 2 && cols <= MAXCOLS) {
        OthCols = cols;
        init_board_hash();
        gInitialPosition = MakeInitialSquare();
    } else {
        printf("Bad number of columns");
    }
}

POSITION MakeInitialSquare(void) {
    char* board;
    int ArrayNum, TopLeftOfSq[2];

    board = getBlankBoard();
    if (OthRows % 2 == 0)
        TopLeftOfSq[0] = OthRows / 2;
    else
        TopLeftOfSq[0] = (OthRows - 1) / 2;

    if (OthCols % 2 == 0)
        TopLeftOfSq[1] = OthCols / 2;
    else
        TopLeftOfSq[1] = (OthCols - 1) / 2;

    ArrayNum = CoordtoArrayNum(TopLeftOfSq[0], TopLeftOfSq[1]);
    board[ArrayNum] = 'B';
    ArrayNum = CoordtoArrayNum(TopLeftOfSq[0], TopLeftOfSq[1] + 1);
    board[ArrayNum] = 'W';
    ArrayNum = CoordtoArrayNum(TopLeftOfSq[0] + 1, TopLeftOfSq[1]);
    board[ArrayNum] = 'W';
    ArrayNum = CoordtoArrayNum(TopLeftOfSq[0] + 1, TopLeftOfSq[1] + 1);
    board[ArrayNum] = 'B';

    return getPosition(board, BLACK);
}

void PrintBoard(char board[]) {
    char alphabet[] = "abcdefghijklmnopqrstuvwxyz";
    int i, j, alpha, hyphens;

#if DEBUG
    printf("\nPrintPosition starting\n");
#endif

    // Top Row Alphabet Legend
    printf("\n\t   ");
    for (alpha = 0; alpha < OthCols; alpha++) {
        printf(" %c", alphabet[alpha]);
    }
    printf("\n");
    // End Alphabet Legend

    // Main Board Thingie
    for (i = 0; i < (2 * OthRows + 1); i++) {
        if (i % 2 == 0) {
            // Even rows are hyphen-lines
            printf("\t   ");
            for (hyphens = 0; hyphens <= (2 * OthCols); hyphens++) {
                printf("-");
            }
            printf("\n");  // End Even rows
        } else {
            // Start Odd Rows, the ones that matter
            printf("\t %d ", InvertRow(PrintPositionRow(i)));
            for (j = 0; j < OthCols; j++) {
                printf("|%c",
                       board[CoordtoArrayNum(PrintPositionRow(i), (j + 1))]);
            }
            printf("| %d\n", InvertRow(PrintPositionRow(i)));
        }
    }
    // Top Bottom Row Alphabet Legend
    printf("\t   ");
    for (alpha = 0; alpha < OthCols; alpha++) {
        printf(" %c", alphabet[alpha]);
    }
    printf("\n");
    // End Alphabet Legend

#if DEBUG
    printf("\nEnd Board\n");
#endif
}

int AddRemovePieces(char board[], int tally, char ownpiece) {
    char blackorwhite[6];
    char selection_command[80], selection = 'Z';
    int ArrayNum;

    if (ownpiece == BLACKPIECE)
        sprintf(blackorwhite, "Black");
    else
        sprintf(blackorwhite, "White");
    do {
        printf("\tAdd/Remove %s Pieces\n", blackorwhite);
        printf("\t=======================\n\n");
        PrintBoard(board);
        printf(
            "\n\tPlease enter a coordinate. A coordinate with a %c on it will\n"
            "\tremove the piece, and a coordinate with a blank space on it\n"
            "\twill add a %c. Press 'B' when you are done to go back.\n\n",
            ownpiece, ownpiece);
        printf("Coordinate: ");
        GetMyStr(selection_command, 80);
        if (strlen(selection_command) == 1) {
            selection = toupper(selection_command[0]);
        }
        if (strlen(selection_command) == 2 || strlen(selection_command) == 3) {
            if (ValidTextInput(selection_command)) {
                ArrayNum = ConvertTextInputToMove(selection_command);
                if (board[ArrayNum] == ownpiece) {
                    board[ArrayNum] = BLANKPIECE;
                    tally--;
                } else if (board[ArrayNum] == BLANKPIECE) {
                    board[ArrayNum] = ownpiece;
                    tally++;
                } else {
                    printf(
                        "Invalid Coordinate. Location has opponent's "
                        "piece.\n\n");
                }
            }
        }
    } while (selection != 'B');
    gInitialPosition = getPosition(board, 1);
    return tally;
}

void Go1Direction(int move[], int direction) {
    if (direction < 1 || direction > 8)
        printf("Go1Direction has a bad direction");
    /*      Direction Legend
            1 Up
            2 Right
            3 Down
            4 Left
            5 Up Right
            6 Down Right
            7 Down Left
            8 Up Left
            (Two circles counterclockwise)
     */
    if (direction == 1 || direction == 5 || direction == 8)
        move[0] = move[0] - 1;  // Up
    if (direction == 3 || direction == 6 || direction == 7)
        move[0] = move[0] + 1;  // Down
    if (direction == 2 || direction == 5 || direction == 6)
        move[1] = move[1] + 1;  // Right
    if (direction == 4 || direction == 7 || direction == 8)
        move[1] = move[1] - 1;  // Left
}

BOOLEAN Check1Spot1Direc(int ArrayNum, char board[], char ownpiece,
                         char opponentpiece, int direction) {
    int CandidateArrayNum, CandidateSpot[2];
    char checkatspot;

    if (DEBUGHELPERS) {
        printf("\nCheck1Spot1Direc starting\nArrayNum: %d\nDirection: %d",
               ArrayNum, direction);
    }
    ArrayNumtoCoord(ArrayNum, CandidateSpot);

    // check to see if the first move in this direction is the opposite piece
    // Then, you can just keep going until it's your own piece
    Go1Direction(CandidateSpot, direction);
    if (ValidCoord(CandidateSpot)) {
        CandidateArrayNum = CoordtoArrayNum(CandidateSpot[0], CandidateSpot[1]);
        if (board[CandidateArrayNum] == opponentpiece) {
            checkatspot = board[CandidateArrayNum];

            // First piece in direction is opponent's, now see if at end one is
            // ours
            while (checkatspot == opponentpiece && ValidCoord(CandidateSpot)) {
                CandidateArrayNum =
                    CoordtoArrayNum(CandidateSpot[0], CandidateSpot[1]);
                checkatspot = board[CandidateArrayNum];
                Go1Direction(CandidateSpot, direction);
            }
            if (checkatspot == ownpiece) return 1;
        }
    }
    return 0;
}

BOOLEAN quickgeneratemoves(char board[], int whoseturn) {
    int i, j;
    char ownpiece, opponentpiece;

#if DEBUG
    printf("\nQuick Generate Moves starting\n\n");
#endif

    // assigning opponent pieces and own pieces
    if (whoseturn == 1) {
        ownpiece = 'B';
        opponentpiece = 'W';
    } else {
        ownpiece = 'W';
        opponentpiece = 'B';
    }

    for (i = 0; i < (OthRows * OthCols); i++)
        if (board[i] == BLANKPIECE) {
            // Each spot has 8 directions
            for (j = 1; j < 9; j++) {
                if (Check1Spot1Direc(i, board, ownpiece, opponentpiece, j)) {
                    return 1;
                }
            }
        }
    return 0;
}

POSITION ActualNumberOfPositions(int variant) {
    if (variant & 0x01) return 1231897;
    return 62789;
}

void tallyPieces(char* board, int* black, int* white, int* blank) {
    *white = *black = 0;
    if (blank) *blank = 0;
    for (int i = 0; i < OthCols * OthRows; i++) {
        if (board[i] == 'W')
            ++(*white);
        else if (board[i] == 'B')
            ++(*black);
        else if (blank && board[i] == ' ')
            ++(*blank);
    }
}

WINBY computeWinBy(POSITION position) {
    char* board = getBoard(position);
    int whitetally, blacktally;
    tallyPieces(board, &blacktally, &whitetally, NULL);
    free(board);
    int diff = blacktally - whitetally;
    return diff > 0 ? diff : -diff;
}

/******************* MAXIMIZATION IMPLEMENTATION *******************/

/*Stolen from mttc.c*/
/* "Unhash" */
char* getBoard(POSITION pos) {
    int boardsize;
    char* newBoard;
    boardsize = OthCols * OthRows;
    newBoard = SafeMalloc(boardsize * sizeof(char) + 1);
    /*Stolen from mtttier.c*/      // 11-25-07
    if (gHashWindowInitialized) {  // using hash windows
        //
        printf("UH OH, HASH WINDOW GOT INITIALIZED\n");
        //
        TIERPOSITION tierpos;
        TIER tier;
        gUnhashToTierPosition(pos, &tierpos, &tier);  // get tierpos
        generic_hash_context_switch(tier);  // switch to that tier's context
        newBoard =
            generic_hash_unhash(tierpos, newBoard);  // unhash in that tier
    } else {
        newBoard = generic_hash_unhash(pos, newBoard);
    }
    newBoard[boardsize] = '\0';
    return newBoard;
}

char* getBlankBoard(void) {
    int boardsize;
    char* newBoard;
    boardsize = OthCols * OthRows;
    newBoard = SafeMalloc(boardsize * sizeof(char) + 1);
    /*Stolen from mtttier.c*/      // 11-25-07
    if (gHashWindowInitialized) {  // using hash windows
        //
        printf("UH OH, HASH WINDOW GOT INITIALIZED\n");
        //
        TIERPOSITION tierpos;
        TIER tier;
        gUnhashToTierPosition(BLANKBOARDPOSITION, &tierpos,
                              &tier);       // get tierpos
        generic_hash_context_switch(tier);  // switch to that tier's context
        newBoard =
            generic_hash_unhash(tierpos, newBoard);  // unhash in that tier
    } else {
        newBoard = generic_hash_unhash(BLANKBOARDPOSITION, newBoard);
    }
    return newBoard;
}

/* "Hash" */
POSITION getPosition(char* board, int player) {
    POSITION position;
    TIER tier = BoardToTier(board);  // find this board's tier
    // int whoseTurn = whoseTurnGivenTier(tier);
    if (gHashWindowInitialized) {
        //
        printf("UH OH, HASH WINDOW GOT INITIALIZED\n");
        //
        generic_hash_context_switch(tier);  // switch to that context
        TIERPOSITION tierpos =
            generic_hash_hash((char*)board, player);  // unhash
        position =
            gHashToWindowPosition(tierpos, tier);  // gets TIERPOS, find POS
    } else {
        position = generic_hash_hash((char*)board, player);
    }
    free(board);
    return position;
}

/* Returns the player whose turn it is to play (1 or 2) */  // 11-25-07
int getTurn(POSITION pos) {
    int whoseturn;
    if (gHashWindowInitialized) {
        //
        printf("UH OH, HASH WINDOW GOT INITIALIZED\n");
        //
        TIER tier = PositionToTier(pos);  // find this position's tier
        int numPieces = tierToNumPieces(tier);
        // Assuming black goes first and the initial position contains 4 pieces.
        if ((numPieces % 2) == 0) {
            whoseturn = BLACK;  // black's turn
        } else {
            whoseturn = WHITE;  // white's turn
        }
    } else {  // not in tier mode
        whoseturn = generic_hash_turn(pos);
    }
    return whoseturn;
}

/* x^y */
int power(int x, int y) {
    int retval = 1;
    while (y > 0) {
        retval *= x;
        y--;
    }
    return retval;
}

/* x! */
int fact(int x) {
    int retval = 1;
    while (x > 0) {
        retval *= x;
        x--;
    }
    return retval;
}

/* C(n, k) */
int combination(int n, int k) { return (fact(n) / (fact(k) * fact(n - k))); }

int tierToWhites(TIER tier) {
    int OthArea = OthRows * OthCols;
    int whites = tier / OthArea;
    return whites;
}

int tierToBlacks(TIER tier) {
    int OthArea = OthRows * OthCols;
    int blacks = tier % OthArea;
    return blacks;
}

int tierToNumPieces(TIER tier) {
    int totPieces = tierToWhites(tier) + tierToBlacks(tier);
    return totPieces;
}

int whoseTurnGivenBlacksWhites(int blacks, int whites) {
    int totPieces = blacks + whites;
    return (totPieces % 2 == 0 ? 1 : 2);
}

int whoseTurnGivenTier(TIER tier) {
    return whoseTurnGivenBlacksWhites(tierToBlacks(tier), tierToWhites(tier));
}

/* Initialize Tier Stuff. */
void SetupTierStuff(void) {
    // First 6 lines copied from mwin4.c
    kSupportsTierGamesman = TRUE;
    // kExclusivelyTierGamesman = FALSE;
    // kDebugTierMenu = TRUE;
    gTierChildrenFunPtr = &TierChildren;                    // Defined below
    gNumberOfTierPositionsFunPtr = &NumberOfTierPositions;  // Will define below
    gTierToStringFunPtr = &TierToString;
    gIsLegalFunPtr = &IsLegal;
    /*   gGetInitialTierPositionFunPtr	= &GetInitialTierPosition; */
    // gUnhashToTierPosition(gInitialPosition, &tierpos, &tier); // get tierpos
    // gInitialTierPosition = tierpos; // Really??
    int OthArea = OthCols * OthRows;
    int piecesArray[] = {BLANKPIECE, 0,          0, WHITEPIECE, 0,
                         0,          BLACKPIECE, 0, 0,          -1};

    /* ----  Notes about the for loop below: ----*/
    /* ~ we have redundant code because we think it makes the code more readable
     */
    /* ~ we know that the hash will contain hashes illegal in the specified */
    /* tier, but we didn't change it because it would be very difficult to do
     * so. */
    int whites;
    int blacks;
    for (whites = 0; whites <= OthArea; whites++) {
        for (blacks = 0; blacks <= OthArea - whites; blacks++) {
            piecesArray[1] = piecesArray[2] = OthArea - whites - blacks;
            piecesArray[4] = piecesArray[5] = whites;
            piecesArray[7] = piecesArray[8] = blacks;
            generic_hash_init(OthArea, piecesArray, NULL,
                              whoseTurnGivenBlacksWhites(blacks, whites));
        }
    }

    // Initial Tier when game starts
    // gInitialTier = PositionToTier(gInitialPosition); // 11-25-07
    int initWhites = 2;
    int initBlacks = 2;
    gInitialTier =
        initBlacks +
        OthArea *
            initWhites;  // this is the general hashing function that gives the
                         // tier given number of blacks and whites
    generic_hash_context_switch(gInitialTier);

    // Setup the Board
    char* board = getBlankBoard();
    int ArrayNum, TopLeftOfSq[2];

    if (OthRows % 2 == 0)
        TopLeftOfSq[0] = OthRows / 2;
    else
        TopLeftOfSq[0] = (OthRows - 1) / 2;

    if (OthCols % 2 == 0)
        TopLeftOfSq[1] = OthCols / 2;
    else
        TopLeftOfSq[1] = (OthCols - 1) / 2;

    ArrayNum = CoordtoArrayNum(TopLeftOfSq[0], TopLeftOfSq[1]);
    board[ArrayNum] = 'B';
    ArrayNum = CoordtoArrayNum(TopLeftOfSq[0], TopLeftOfSq[1] + 1);
    board[ArrayNum] = 'W';
    ArrayNum = CoordtoArrayNum(TopLeftOfSq[0] + 1, TopLeftOfSq[1]);
    board[ArrayNum] = 'W';
    ArrayNum = CoordtoArrayNum(TopLeftOfSq[0] + 1, TopLeftOfSq[1] + 1);
    board[ArrayNum] = 'B';

    gInitialTierPosition = getPosition(board, BLACK);  // BLACK'S TURN??
    // gInitialPosition = MakeInitialSquare();
}

// Returns list of all children for a particular tier.
TIERLIST* TierChildren(TIER tier) {
    // New tierization scheme: tier = B + OthArea*W, so tier children =
    TIER OthArea = OthRows * OthCols;
    int blacks = tier % OthArea;
    int whites = tier / OthArea;
    int tempBlacks;
    int tempWhites;
    int numPieces = blacks + whites;

    if ((blacks >= 1) && (whites >= 1) && (tier < OthArea * OthArea) &&
        (numPieces < 16)) {  // no children if one color has no pieces
        TIERLIST* newList = NULL;
        tempBlacks = blacks;
        tempWhites = whites;
        tempBlacks += 2;  // black will definitely gain at least two pieces: put
                          // one, steal one
        tempWhites -= 1;  // white will definitely lose at least one piece
        for (; tempWhites >= 0;
             tempWhites--, tempBlacks++) {  // black can only steal white pcs
            newList = CreateTierlistNode(tempBlacks + tempWhites * OthArea,
                                         newList);  // black+1, white-1
        }
        tempBlacks = blacks;
        tempWhites = whites;
        tempWhites += 2;  // white will definitely gain at least two pieces
        tempBlacks -= 1;  // black will definitely lose at least one piece
        for (; tempBlacks >= 0; tempBlacks--, tempWhites++) {
            newList = CreateTierlistNode(tempBlacks + tempWhites * OthArea,
                                         newList);  // black+1, white-1
        }
        return newList;
    }
    return NULL;
}

/* Returns the number of positions associated with a particular tier. */
TIERPOSITION NumberOfTierPositions(TIER tier) {
    generic_hash_context_switch(tier);
    return generic_hash_max_pos();
}

/* Given a board, returns the tier number of it (i.e. number of pieces placed on
 * the board) */
TIER BoardToTier(char* board) {
    int whitetally, blacktally;
    tallyPieces(board, &blacktally, &whitetally, NULL);
    return whitetally * OthCols * OthRows + blacktally;
}

/* Given a position, returns the tier number of it (i.e. number of pieces placed
 * on the board) */
TIER PositionToTier(POSITION position) {
    char* board = getBoard(position);
    TIER tier = BoardToTier(board);
    free(board);
    return tier;
}

STRING TierToString(TIER tier) {
    STRING tierStr = (STRING)SafeMalloc(sizeof(char) * 50);
    int OthArea = OthRows * OthCols;
    int blacks = tier % OthArea;
    int whites = tier / OthArea;
    int numPieces = blacks + whites;
    sprintf(tierStr, "%d blacks, %d whites, %d Pieces Placed", blacks, whites,
            numPieces);
    return tierStr;
}

BOOLEAN IsLegal(POSITION position) {
    TIER tier = PositionToTier(position);
    if (tier < 5) {
        return FALSE;
    } else {
        return TRUE;
    }
}

// prints out a character string into a board format for easier viewing
void printBoard(char* board) {
    printf("----Board-----\n");
    int i;
    for (i = 0; board[i] != 0; i++) {
        printf("%c", board[i]);
        if (((i + 1) % OthCols) == 0) {
            printf("\n");
        }
    }
    printf("--strlen: %d--\n\n", i);

    // error checking
    if ((i - 1) > (OthCols * OthRows)) {
        /* this is i-1, because i ultimately increments to 1 beyond
           the size of the board.  i should normally go up to OthArea+1 */
        printf(
            "\nHUGE ERROR...I IS TOO BIG, meaning the BOARD SHOULD "
            "BE SMALLER THAN IT REALLY IS!!!! ==> %d\n\n",
            i);
    }
}


void PositionToString(POSITION position, char *positionStringBuffer) {
    char* board = getBoard(position);
    int BOARDSIZE = 16;
    for (int i = 0; i < BOARDSIZE; i++) {
        if (board[i] == ' ') {
            board[i] = '-';
        }
    }
    positionStringBuffer[0] = generic_hash_turn(position) + '0';
    positionStringBuffer[1] = '_';
    memcpy(positionStringBuffer + 2, board, 16);
    positionStringBuffer[18] = '\0';
    free(board);
}

/* The difference between the PositionString and AutoGUIPositionString is that the
AutoGUIPositionString contains characters for the players' scores, while the PositionString
does not. */
POSITION StringToPosition(char *positionString) {
	int turn;
	char *board;
    /* Even though the PositionString and AutoGUIPositionString are not the same,
    we can still use ParseAutoGUIFormattedPositionString because the PositionString
    is still in AutoGUI format. */
	if (ParseAutoGUIFormattedPositionString(positionString, &turn, &board)) {
        // Convert UWAPI standard board string to internal board representation
        int BOARDSIZE = 16;
        char *boardCopy = (char *) malloc(sizeof(char) * BOARDSIZE);
        for (int i = 0; i < BOARDSIZE; i++) {
            if (board[i] == '-') {
                boardCopy[i] = ' ';
            } else {
                boardCopy[i] = board[i];
            }
        }
        return getPosition(boardCopy, (turn == 1) ? BLACK : WHITE);
        // No need to free boardCopy because getPosition does it
	}
	return NULL_POSITION;
}

/* The difference between the PositionString and AutoGUIPositionString is that the
AutoGUIPositionString contains characters for the players' scores, while the PositionString
does not. */
void PositionToAutoGUIString(POSITION position, char *autoguiPositionStringBuffer) {
	char* board = getBoard(position);
    int BOARDSIZE = 16;
    int whitetally, blacktally;

    for (int i = 0; i < BOARDSIZE; i++) {
        if (board[i] == ' ') {
            board[i] = '-';
        }
    }
    char formatted[21];
    memcpy(formatted, board, 16);

    // Append piece counts to the end of the string, assuming enough space.
    tallyPieces(board, &blacktally, &whitetally, NULL);
    formatted[16] = blacktally / 10 + '0';
    formatted[17] = blacktally % 10 + '0';
    formatted[18] = whitetally / 10 + '0';
    formatted[19] = whitetally % 10 + '0';
    formatted[20] = '\0';
    free(board);
    AutoGUIMakePositionString(generic_hash_turn(position), formatted, autoguiPositionStringBuffer);
}

void MoveToAutoGUIString(POSITION position, MOVE move, char *autoguiMoveStringBuffer) {
    (void) position;
    if ((int)move == PASSMOVE) {
        AutoGUIMakeMoveButtonStringA('P', 20, 'x', autoguiMoveStringBuffer);
    } else {
        AutoGUIMakeMoveButtonStringA('h', move, 'x', autoguiMoveStringBuffer);
    }
}

void SetTclCGameSpecificOptions(int options[]) { (void)options; }
