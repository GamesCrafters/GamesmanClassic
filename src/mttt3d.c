/************************************************************************
**
** NAME:        mttt3d.c
**
** DESCRIPTION: <3D XO Chess> (Use this spacing and case)
**
** AUTHOR:      Angela Rodriguez
**
** DATE:        2025-10-06
**
************************************************************************/

#include "gamesman.h"
#include "core/bpdb.h"
#include <string.h>

CONST_STRING kAuthorName = "Angela Rodriguez";
CONST_STRING kGameName = "3D X0 Chess";  // Use this spacing and case
CONST_STRING kDBName = "ttt3d";      // Use this spacing and case

/**
 * @brief The total number of positions calculated by generic_hash_init.
 */
POSITION gNumberOfPositions;
POSITION kBadPosition        = -1;
POSITION gInitialPosition    =  0;
POSITION gMinimalPosition    =  0;

BOOLEAN kPartizan = TRUE;
BOOLEAN kTieIsPossible = TRUE;
BOOLEAN kLoopy = FALSE;
BOOLEAN kSupportsSymmetries = FALSE;
BOOLEAN kDebugDetermineValue = FALSE;
BOOLEAN kGameSpecificMenu = FALSE;
BOOLEAN kDebugMenu = TRUE;
void *gGameSpecificTclInit = NULL;

/**
 * @brief These variables are not needed for solving but if you have time
 * after you're done solving the game you should initialize them
 * with something helpful, for the TextUI.
 */
CONST_STRING kHelpGraphicInterface =
    "The LEFT button puts an X or O (depending on whether you went first\n\
or second) on the spot the cursor was on when you clicked. The MIDDLE\n\
button does nothing, and the RIGHT button is the same as UNDO, in that\n\
it reverts back to your your most recent position.";
CONST_STRING kHelpTextInterface =
    "On your turn, use the LEGEND to determine which number to choose (between\n\
1 and 9, with 1 at the upper left and 9 at the lower right) to correspond\n\
to the empty board position you desire and hit return. Start with filling the pieces\n\
on board A and proceed with stacking on B and then C depending on if the previous boards\n\
contain a piece. If at any point you have made a mistake, you can type u and hit return\n\
and the system will revert back to your most recent position.";
CONST_STRING kHelpOnYourTurn =
    "You place one of your pieces on one of the empty board positions. If you want to\n\
    place a piece on an above level board (e.g. B or C), you want to make sure that the\n\
    previous board (e.g. A or B) has a piece that exists on the same index from 1-9.";
CONST_STRING kHelpStandardObjective =
    "To get three of your markers (either X or O) in a row, either\n\
horizontally, vertically, or diagonally. 3-in-a-row WINS.";
CONST_STRING kHelpReverseObjective =
    "To force your opponent into getting three of his markers (either X or\n\
O) in a row, either horizontally, vertically, or diagonally. 3-in-a-row\n\
LOSES.";
CONST_STRING kHelpTieOccursWhen = "the board fills up without either player getting three-in-a-row.";/* Should follow 'A Tie occurs when... */
CONST_STRING kHelpExample =
"                              : - - -    \n\
                             : - - -  C \n\
                             : - - -    \n\n\
\n\
         ( 1 2 3 )           : - - -   \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - - -  B \n\
         ( 7 8 9 )           : - - -   \n\n\
\n\
                             : - - -    \n\
                             : - - -  A \n\
                             : - - -    \n\n\
Computer's move              :  a3    \n\n\
                             : - - -    \n\
                             : - - -  C \n\
                             : - - -    \n\n\
\n\
         ( 1 2 3 )           : - - -   \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - - -  B \n\
         ( 7 8 9 )           : - - -   \n\n\
\n\
                             : - - X    \n\
                             : - - -  A \n\
                             : - - -    \n\n\
Players's move [(u)ndo/1-9] : { a2 } \n\n\
                             : - - -    \n\
                             : - - -  C \n\
                             : - - -    \n\n\
\n\
         ( 1 2 3 )           : - - -   \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - - -  B \n\
         ( 7 8 9 )           : - - -   \n\n\
\n\
                             : - O X    \n\
                             : - - -  A \n\
                             : - - -    \n\n\
Computer's move              :  b3    \n\n\
                             : - - -    \n\
                             : - - -  C \n\
                             : - - -    \n\n\
\n\
         ( 1 2 3 )           : - - X   \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - - -  B \n\
         ( 7 8 9 )           : - - -   \n\n\
\n\
                             : - O X    \n\
                             : - - -  A \n\
                             : - - -    \n\n\
Players's move [(u)ndo/1-9] : { a8 } \n\n\
                             : - - -    \n\
                             : - - -  C \n\
                             : - - -    \n\n\
\n\
         ( 1 2 3 )           : - - X   \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - - -  B \n\
         ( 7 8 9 )           : - - -   \n\n\
\n\
                             : - O X    \n\
                             : - - -  A \n\
                             : - O -    \n\n\
Computer's move              :  c3    \n\n\
                             : - - X    \n\
                             : - - -  C \n\
                             : - - -    \n\n\
\n\
         ( 1 2 3 )           : - - X   \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - - -  B \n\
         ( 7 8 9 )           : - - -   \n\n\
\n\
                             : - O X    \n\
                             : - - -  A \n\
                             : - O -    \n\n\
Player (player one) Wins!";        

#define BOARDLEN      3
#define BOARDSIZE     27           /* 3x3x3 board */

#define FIRSTPLAYER     1
#define SECONDPLAYER    2
#define Blank           '-'
#define o               'o'
#define x               'x'
typedef char BlankOX;

char *gBlankOXString[] = { "-", "O", "X" };

struct {
    BlankOX board[BOARDSIZE];
    BlankOX nextPiece;
    int piecesPlaced;
} gPosition;



BOOLEAN AllFilledIn(BlankOX*);
void UndoMove(MOVE move);
BOOLEAN ThreeInARow(BlankOX*, int, int, int);
POSITION GetCanonicalPosition(POSITION);
POSITION ActualNumberOfPositions(int variant);


void PositionToBlankOX(POSITION, BlankOX*, int*);
BlankOX WhoseTurn(BlankOX*);
TIER BoardToTier(BlankOX*);





BOOLEAN IsLegal(POSITION);


POSITION UnDoMove(POSITION, UNDOMOVE);

/**
 * @brief Tcl-related stuff. Do not change if you do not plan to make a Tcl interface.
 */
/* This is the array used for flipping along the N-S axis */
int gFlipNewPosition[] = { 2, 1, 0, 5, 4, 3, 8, 7, 6 };

/* This is the array used for rotating 90 degrees clockwise */
int gRotate90CWNewPosition[] = { 6, 3, 0, 7, 4, 1, 8, 5, 2 };

void SetTclCGameSpecificOptions(int theOptions[]) { (void)theOptions; }




/*********** BEGIN SOLVING FUNCIONS ***********/

/**
 * @brief Initialize any global variables.
 */

// for the hash init
int vcfg(int* this_cfg) {
    int o_piece = this_cfg[0];
    int x_piece = this_cfg[1];
    return (x_piece == o_piece) || (x_piece == o_piece + 1);
}

void InitializeGame(void) {
    generic_hash_destroy();
    //have a GLOBAL HASH set up at outset:
    int game[10] = {'o', 0, 13, 'x', 0, 14, Blank, 0, 27, -1 };
    gNumberOfPositions = generic_hash_init(BOARDSIZE, game, &vcfg, 0);

    BlankOX board[BOARDSIZE]; memset(board, Blank, BOARDSIZE);
    memcpy(gPosition.board, board, sizeof(BlankOX) * BOARDSIZE);
    gPosition.nextPiece = x;
    gPosition.piecesPlaced = 0;
    gUndoMove = UndoMove;

    gInitialPosition = generic_hash_hash(board, FIRSTPLAYER);

}

/**
 * @brief Return the head of a list of the legal moves from the input position.
 * 
 * @param position The position to branch off of.
 * 
 * @return The head of a linked list of the legal moves from the input position.
 * 
 * @note Moves are encoded as 32-bit integers. See the MOVE typedef
 * in src/core/types.h.
 * 
 * @note It may be helpful to use the CreateMovelistNode() function
 * to assemble the linked list. But remember that this allocates heap space. 
 * If you use GenerateMoves in any of the other functions in this file, 
 * make sure to free the returned linked list using FreeMoveList(). 
 * See src/core/misc.c for more information on CreateMovelistNode() and
 * FreeMoveList().
 */

BOOLEAN AllFilledIn(BlankOX *theBlankOX) {
    BOOLEAN answer = TRUE;
    int i;

    for(i = 0; i < BOARDSIZE; i++)
        answer &= (theBlankOX[i] == o || theBlankOX[i] == x);

    return(answer);
}

void PositionToBlankOX(POSITION thePos, BlankOX *theBlankOX, int* turn) {
    if (turn)
        *turn = generic_hash_turn(thePos);
    generic_hash_unhash(thePos, theBlankOX);
}

BOOLEAN ThreeInARow(BlankOX *theBlankOX, int a, int b, int c) {
    return(theBlankOX[a] == theBlankOX[b] &&
            theBlankOX[b] == theBlankOX[c] &&
            theBlankOX[c] != Blank);
}

BlankOX WhoseTurn(BlankOX *theBlankOX) {
    int i, xcount = 0, ocount = 0;

    for(i = 0; i < BOARDSIZE; i++)
        if(theBlankOX[i] == x)
            xcount++;
        else if(theBlankOX[i] == o)
            ocount++;
        /* else don't count blanks */

    if(xcount == ocount)
        return(x);  /* in our TicTacToe, x always goes first */
    else
        return(o);
}

MOVELIST *GenerateMoves(POSITION position) {

    /*
        NOTE: To add to the linked list, do

        moves = CreateMovelistNode(<the move you're adding>, moves);

        See the function CreateMovelistNode in src/core/misc.c
    */

    int index;
    int turn;
    MOVELIST *moves = NULL;
    BlankOX board[BOARDSIZE];

    PositionToBlankOX(position, board, &turn);

    for (index = 0; index < 27; index++) {
        if (board[index] == '-') {
            if (index < 9) {
                moves = CreateMovelistNode(index, moves);
            } else {
                if (board[index - 9] != Blank)
                    moves = CreateMovelistNode(index, moves);
            }
        }
    }
    return moves;
}

/**
 * @brief Return the child position reached when the input move
 * is made from the input position.
 * 
 * @param position : The old position
 * @param move     : The move to apply.
 * 
 * @return The child position, encoded as a 64-bit integer. See
 * the POSITION typedef in src/core/types.h.
 */
POSITION DoMove(POSITION position, MOVE move) {
    BlankOX board[BOARDSIZE];
    int turn = generic_hash_turn(position);
    generic_hash_unhash(position, board);
    board[move] = (turn == FIRSTPLAYER ? x : o);
    // update global gPosition
    memcpy(gPosition.board, board, sizeof(BlankOX) * BOARDSIZE);
    int placed = 0;
    for (int i = 0; i < BOARDSIZE; i++) if (board[i] != Blank) placed++;
    gPosition.piecesPlaced = placed;
    gPosition.nextPiece = (turn == FIRSTPLAYER ? o : x);
    return generic_hash_hash(board, turn == FIRSTPLAYER ? SECONDPLAYER : FIRSTPLAYER);

}

void UndoMove(MOVE move) {
    gPosition.board[move] = Blank;
    gPosition.nextPiece = gPosition.nextPiece == x ? o : x;
    --gPosition.piecesPlaced;
}

static int get_1d_index(int z, int y, int m) {
    return z * (BOARDLEN * BOARDLEN) + y * BOARDLEN + m;
}

static BOOLEAN check_line_win(int start_z, int start_y, int start_x, int dz, int dy, int dx) {

    int index1 = get_1d_index(start_z, start_y, start_x);
    int index2 = get_1d_index(start_z + 1 * dz, start_y + 1 * dy, start_x + 1 * dx);
    int index3 = get_1d_index(start_z + 2 * dz, start_y + 2 * dy, start_x + 2 * dx);

    if (ThreeInARow(gPosition.board, index1, index2, index3))
        return TRUE;
    else
        return FALSE;
}

static BOOLEAN check_3d_win() {

    // check all rows (X-direction)
    for (int z = 0; z < BOARDLEN; z++) {
        for (int y = 0; y < BOARDLEN; y++) {
            if (check_line_win(z, y, 0, 0, 0, 1)) return TRUE;
        }
    }

    // Check all columns (Y-direction)
    for (int z = 0; z < BOARDLEN; z++) {
        for (int m = 0; m < BOARDLEN; m++) {
            if (check_line_win(z, 0, m, 0, 1, 0)) return TRUE;
        }
    }

    // Check all vertical lines (Z-direction)
    for (int y = 0; y < BOARDLEN; y++) {
        for (int m = 0; m < BOARDLEN; m++) {
            if (check_line_win(0, y, m, 1, 0, 0)) return TRUE;
        }
    }

    // Check face diagonals on the XY planes
    for (int z = 0; z < BOARDLEN; z++) {
        if (check_line_win(z, 0, 0, 0, 1, 1)) return TRUE;
        if (check_line_win(z, 0, 2, 0, 1, -1)) return TRUE;
    }

    // Check face diagonals on the XZ planes
    for (int y = 0; y < BOARDLEN; y++) {
        if (check_line_win(0, y, 0, 1, 0, 1)) return TRUE;
        if (check_line_win(0, y, 2, 1, 0, -1)) return TRUE;
    }

    // Check face diagonals on the YZ planes
    for (int m = 0; m < BOARDLEN; m++) {
        if (check_line_win(0, 0, m, 1, 1, 0)) return TRUE;
        if (check_line_win(0, 2, m, 1, -1, 0)) return TRUE;
    }

    // Check space diagonals diagonally on the inside
    if (check_line_win(0, 0, 0, 1, 1, 1)) return TRUE;
    if (check_line_win(0, 0, 2, 1, 1, -1)) return TRUE;
    if (check_line_win(0, 2, 0, 1, -1, 1)) return TRUE;
    if (check_line_win(0, 2, 2, 1, -1, -1)) return TRUE;

    return FALSE;
}

/**
 * @brief Return win, lose, or tie if the input position
 * is primitive; otherwise return undecided.
 * 
 * @param position : The position to inspect.
 * 
 * @return an enum; one of (win, lose, tie, undecided). See 
 * src/core/types.h for the value enum definition.
 */
VALUE Primitive(POSITION position) {
    BlankOX board[BOARDSIZE];
    generic_hash_unhash(position, board);
    char player = generic_hash_turn(position);
    player = player == FIRSTPLAYER ? x : o;
    {
    memcpy(gPosition.board, board, sizeof(BlankOX) * BOARDSIZE);
    int placed = 0;
    for (int i = 0; i < BOARDSIZE; i++) if (board[i] != Blank) placed++;
    gPosition.piecesPlaced = placed;
    }
    if (check_3d_win())
        return gStandardGame ? lose : win;
    else if ((gUseGPS && (gPosition.piecesPlaced == BOARDSIZE)) ||
             ((!gUseGPS) && AllFilledIn(gPosition.board)))
        return tie;
    else
        return undecided;
}

/**
 * @brief Given a position P, GetCanonicalPosition(P) shall return a 
 * position Q such that P is symmetric to Q and 
 * GetCanonicalPosition(Q) also returns Q.
 * 
 * @note This Q is the "canonical position". For the choice of the 
 * canonical position, it is recommended to choose the 
 * lowest-hash-value position symmetric to the input position.
 * 
 * Ideally, we maximize the number of positions P such that
 * GetCanonicalPosition(P) != P.
 * 
 * @param position : The position to inspect.
 */
POSITION GetCanonicalPosition(POSITION position) {
    return position;
}

/*********** END SOLVING FUNCTIONS ***********/






/*********** BEGIN TEXTUI FUNCTIONS ***********/

/**
 * @brief Print the position in a pretty format, including the
 * prediction of the game's outcome.
 * 
 * @param position   : The position to pretty-print.
 * @param playerName : The name of the player.
 * @param usersTurn  : TRUE <==> it's a user's turn.
 * 
 * @note See GetPrediction() in src/core/gameplay.h to see how
 * to print the prediction of the game's outcome.
 */
static char* getSymbol(char c) {
    if (c == '-') return "-";
    if (c == 'o') return "O";
    if (c == 'x') return "X";
    return "?";
}

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn) {
    BlankOX theBlankOx[BOARDSIZE];
    int turn;
    PositionToBlankOX(position,theBlankOx, &turn);

    char player = generic_hash_turn(position);
    player = player == 1 ? x : o;

    printf("\n                             : %s %s %s    \n", getSymbol(theBlankOx[18]), getSymbol(theBlankOx[19]), getSymbol(theBlankOx[20]));
    printf("                             : %s %s %s    \n", getSymbol(theBlankOx[21]), getSymbol(theBlankOx[22]), getSymbol(theBlankOx[23]));
    printf("                             : %s %s %s    \n", getSymbol(theBlankOx[24]), getSymbol(theBlankOx[25]), getSymbol(theBlankOx[26]));

    printf("\n         ( 1 2 3 )           : %s %s %s    \n", getSymbol(theBlankOx[9]), getSymbol(theBlankOx[10]), getSymbol(theBlankOx[11]));
    printf("LEGEND:  ( 4 5 6 )  TOTAL:   : %s %s %s    \n", getSymbol(theBlankOx[12]), getSymbol(theBlankOx[13]), getSymbol(theBlankOx[14]));
    printf("         ( 7 8 9 )           : %s %s %s    \n", getSymbol(theBlankOx[15]), getSymbol(theBlankOx[16]), getSymbol(theBlankOx[17]));

    printf("\n                             : %s %s %s    \n", getSymbol(theBlankOx[0]), getSymbol(theBlankOx[1]), getSymbol(theBlankOx[2]));
    printf("                             : %s %s %s    \n", getSymbol(theBlankOx[3]), getSymbol(theBlankOx[4]), getSymbol(theBlankOx[5]));
    printf("                             : %s %s %s %s    \n", getSymbol(theBlankOx[6]), getSymbol(theBlankOx[7]), getSymbol(theBlankOx[8]), GetPrediction(position,playerName,usersTurn));
}

/**
 * @brief Find out if the player wants to undo, abort, or neither.
 * If so, return Undo or Abort and don't change `move`.
 * Otherwise, get the new `move` and fill the pointer up.
 * 
 * @param position The position the user is at.
 * @param move The move to fill with user's move.
 * @param playerName The name of the player whose turn it is
 * 
 * @return One of (Undo, Abort, Continue)
 */
USERINPUT GetAndPrintPlayersMove(POSITION position, MOVE *move, STRING playerName) {
    USERINPUT ret;
    do {
        /* List of available moves */
        // Modify the player's move prompt as you wish
        printf("%8s's move [(u)ndo/[1-9]] :  ", playerName);
        ret = HandleDefaultTextInput(position, move, playerName);
        if (ret != Continue) return ret;
    } while (TRUE);
    return (Continue); /* this is never reached, but lint is now happy */
}

/**
 * @brief Given a move that the user typed while playing a game in the
 * TextUI, return TRUE if the input move string is of the right "form"
 * and can be converted to a move hash.
 * 
 * @param input The string input the user typed.
 * 
 * @return TRUE iff the input is a valid text input.
 */
BOOLEAN ValidTextInput(STRING input) {
    return(input[0] <= '9' && input[0] >= '1');
}

/**
 * @brief Convert the string input to the internal move representation.
 * 
 * @param input This is a user-inputted move string already validated
 * by ValidTextInput().
 * 
 * @return The hash of the move specified by the text input.
 */
MOVE ConvertTextInputToMove(STRING input) {

    int pos = input[0] - '1'; //position 0-8
 
    if (gPosition.board[pos] == Blank) return pos; // layer A
    else if (gPosition.board[pos + 9] == Blank) return pos + 9; // layer B if A has piece
    else if (gPosition.board[pos + 18] == Blank) return pos + 18; // layer C if B has piece
    return -1;
}

/**
 * @brief Write a short human-readable string representation
 * of the move to the input buffer.
 * 
 * @param move The move hash to convert to a move string.
 * 
 * @param moveStringBuffer The buffer to write the move string
 * to.
 * 
 * @note The space available in `moveStringBuffer` is MAX_MOVE_STRING_LENGTH 
 * (see src/core/autoguistrings.h). Do not write past this limit and ensure that
 * the move string written to `moveStringBuffer` is properly null-terminated.
 */
void MoveToString(MOVE move, char *moveStringBuffer) {
    int pos = move % 9;
    moveStringBuffer[0] = '0' + (pos + 1); /* The plus 1 is because the user thinks it's 1-9, but MOVE is 0-8 */
    moveStringBuffer[1] = '\0';
}

/**
 * @brief Nicely format the computers move.
 *
 * @param computersMove : The computer's move.
 * @param computersName : The computer's name.
 */
void PrintComputersMove(MOVE computersMove, STRING computersName) {
    char moveStringBuffer[32];
    MoveToString(computersMove, moveStringBuffer);
    printf("%s's move: %s", computersName, moveStringBuffer);
}

/**
 * @brief Menu used to debug internal problems. 
 * Does nothing if kDebugMenu == FALSE.
 */
void DebugMenu(void) {
    do {
		printf("\n\t----- Module DEBUGGER for %s -----\n\n", kGameName);

		printf("\tc)\tWrite PPM to s(C)reen\n");
		printf("\ti)\tWrite PPM to f(I)le\n");
		printf("\ts)\tWrite Postscript to (S)creen\n");
		printf("\tf)\tWrite Postscript to (F)ile\n");
		printf("\n\n\tb)\t(B)ack = Return to previous activity.\n");
		printf("\n\nSelect an option: ");

		switch(GetMyChar()) {
		case 'Q': case 'q':
			ExitStageRight();
			break;
		case 'H': case 'h':
			HelpMenus();
			break;
		case 'C': case 'c': /* Write PPM to s(C)reen */
			//tttppm(0,0);
			break;
		case 'I': case 'i': /* Write PPM to f(I)le */
			//tttppm(0,1);
			break;
		case 'S': case 's': /* Write Postscript to (S)creen */
			//tttppm(1,0);
			break;
		case 'F': case 'f': /* Write Postscript to (F)ile */
			//tttppm(1,1);
			break;
		case 'B': case 'b':
			return;
		default:
			BadMenuChoice();
			HitAnyKeyToContinue();
			break;
		}
	} while(TRUE);
}

/*********** END TEXTUI FUNCTIONS ***********/

/*********** BEGIN VARIANT FUNCTIONS ***********/

/**
 * @return The total number of variants supported.
 */
int NumberOfOptions(void) {
    return 1;
}

/**
 * @return The current variant ID.
 */
int getOption(void) {
    return 0;
}

/**
 * @brief Set any global variables or data structures according to the
 * variant specified by the input variant ID.
 *
 * @param option An ID specifying the variant that we want to change to.
 */
void setOption(int option) {
    (void)option;
}

/**
 * @brief Interactive menu used to change the variant, i.e., change
 * game-specific parameters, such as the side-length of a tic-tac-toe
 * board, for example. Does nothing if kGameSpecificMenu == FALSE.
 */
void GameSpecificMenu(void) {}

/*********** END VARIANT-RELATED FUNCTIONS ***********/





/**
 * @brief Convert the input position to a human-readable formal
 * position string. It must be in STANDARD ONELINE POSITION 
 * STRING FORMAT, i.e. it is formatted as <turn>_<board>, where...
 *     `turn` is a character. ('1' if it's Player 1's turn or 
 *         '2' if it's Player 2's turn; or '0' if turn is not
 *         encoded in the position because the game is impartial.)
 *     `board` is a string of characters that does NOT contain 
 *         colons or semicolons.
 * 
 * @param position The position for which to generate the formal 
 * position string.
 * @param positionStringBuffer The buffer to write the position string to.
 * 
 * @note The space available in `positionStringBuffer` is 
 * MAX_POSITION_STRING_LENGTH (see src/core/autoguistrings.h). Do not write
 * past this limit and ensure that the position string written to 
 * `positionStringBuffer` is properly null-terminated.
 * 
 * @note You need not implement this function if you wish for the
 * AutoGUI Position String to be the same as the Human-Readable Formal
 * Position String. You can in fact delete this function and leave
 * gStringToPositionFunPtr as NULL in InitializeGame().
 */
void PositionToString(POSITION position, char *positionStringBuffer) {}

/**
 * @brief Convert the input position string to
 * the in-game integer representation of the position.
 * 
 * @param positionString The position string. This is guaranteed
 * to be shorter than MAX_POSITION_STRING_LENGTH (see src/core/autoguistrings.h).
 * 
 * @return If the position string is valid, return the corresponding 
 * position hash. Otherwise, return NULL_POSITION.
 * 
 * @note DO NOT MODIFY any characters in `positionString`, or, at least,
 * ensure that the position string written to `positionStringBuffer` is the same as
 * before.
 */
POSITION StringToPosition(char *positionString) {
    int turn;
    char *board;
    if (ParseStandardOnelinePositionString(positionString, &turn, &board)) {
        return 0;
    }
    return NULL_POSITION;
}

/**
 * @brief Write an AutoGUI-formatted position string for the given position 
 * (which tells the frontend application how to render the position) to the
 * input buffer.
 * 
 * @param position The position for which to generate the AutoGUI 
 * position string.
 * @param autoguiPositionStringBuffer The buffer to write the AutoGUI
 * position string to.
 * 
 * @note You may find AutoGUIMakePositionString() helpful. 
 * (See src/core/autoguistrings.h)
 * 
 * @note The space available in `autoguiPositionStringBuffer` is 
 * MAX_POSITION_STRING_LENGTH (see src/core/autoguistrings.h). Do not write
 * past this limit and ensure that the position string written to 
 * `autoguiPositionStringBuffer` is properly null-terminated.
 * AutoGUIMakePositionString() should handle the null-terminator, 
 * if you choose to use it.
 * 
 * @note If the game is impartial and a turn is not encoded, set the turn
 * character (which is the first character) of autoguiPositionStringBuffer
 * to '0'.
 */
void PositionToAutoGUIString(POSITION position, char *autoguiPositionStringBuffer) {}

/**
 * @brief Write an AutoGUI-formatted move string for the given move 
 * (which tells the frontend application how to render the move as button) to the
 * input buffer.
 * 
 * @param position   : The position from which the move is made. This is helpful
 * if the move button depends on not only the move hash but also the position.
 * @param move : The move hash from which the AutoGUI move string is generated.
 * @param autoguiMoveStringBuffer : The buffer to write the AutoGUI
 * move string to.
 * 
 * @note The space available in `autoguiMoveStringBuffer` is MAX_MOVE_STRING_LENGTH 
 * (see src/core/autoguistrings.h). Do not write past this limit and ensure that
 * the move string written to `moveStringBuffer` is properly null-terminated.
 * 
 * @note You may find the "AutoGUIMakeMoveButton" functions helpful.
 * (See src/core/autoguistrings.h)
 */
void MoveToAutoGUIString(POSITION position, MOVE move, char *autoguiMoveStringBuffer) {}
