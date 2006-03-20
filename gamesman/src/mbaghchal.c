// $id$
// $log$

/*
 * The above lines will include the name and log of the last person
 * to commit this file to CVS
 */

/************************************************************************
**
** NAME:        mbaghchal.c
**
** DESCRIPTION: Bagh Chal (Tigers Move)
**
** AUTHOR:      Damian Hites
**              Max Delgadillo
**              Deepa Mahajan
**
** DATE:        2006.3.12
**
** UPDATE HIST: -2004.10.21 = Original (Dom's) Version
**              -2006.3.2 = Updates + Fixes by Max and Deepa
**                  (Nearly) functional Standard version of game
**              -2006.3.20 = Includes hack version of the Retrograde Solver
**					(FAR from complete, but at least it's a working version)
**					Also a few changes to the game, particular the board display.
**
**************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/

#include <stdio.h>
#include "gamesman.h"
#include "hash.h"
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include "core/solveloopy.h"


/*************************************************************************
**
** Game-specific constants
**
**************************************************************************/

STRING   kGameName            = "Bagh Chal (Tigers Move)"; /* The name of your game */
STRING   kAuthorName          = "Damian Hites, Max Delgadillo, and Deepa Mahajan"; /* Your name(s) */
STRING   kDBName              = "baghchal"; /* The name to store the database under */

BOOLEAN  kPartizan            = TRUE; /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN  kGameSpecificMenu    = TRUE; /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN  kTieIsPossible       = FALSE; /* TRUE if a tie is possible. FALSE if it is impossible.*/
BOOLEAN  kLoopy               = TRUE; /* TRUE if the game tree will have cycles (a rearranger style game). FALSE if it does not.*/

BOOLEAN  kDebugMenu           = FALSE; /* TRUE only when debugging. FALSE when on release. */
BOOLEAN  kDebugDetermineValue = FALSE; /* TRUE only when debugging. FALSE when on release. */

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
"On your move, if you are placing a goat, you enter the\n"
"coordinates of the space where you would like to place\n"
"your goat (ex: a3).  If you are moving a goat or a tiger,\n"
"you enter the coordinates of the piece you would like to\n"
"move and the coordinates of the space you would like to\n"
"move the piece to (ex: a3 b3).";

STRING   kHelpOnYourTurnTiger =
"Move one tiger one space in any of the directions\n"
"indicated by the lines.  You may also jump one goat\n"
"if it is in a space next to you and the space behind\n"
"it is empty.  Jumping a goat removes it from the board.";

STRING   kHelpOnYourTurnGoatPhaseOne =
"Place a goat in a space anywhere on the board that is\n"
"not already occupied.";

char*    kHelpOnYourTurn;

STRING   kHelpOnYourTurnGoatPhaseTwo =
"Move one goat one space in any of the directions\n"
"indicated by the lines.";

STRING   kHelpStandardObjective =
"The objective of the goats is to try and trap the tigers\n"
"so that they are unable to move.  The tigers are trying to\n"
"remove every goat from the board.";

STRING   kHelpReverseObjective =
"This is not implemented because it makes the game\n"
"really really stupid.";

STRING   kHelpTieOccursWhen =
"A tie cannot occur.";

STRING   kHelpExample =
"  1 2 3 4 5\n"
"a *-O-O-O-*\n"
"  |\\|/|\\|/|\n"
"b O-O-O-O-O\n"
"  |/|\\|/|\\|\n"
"c O-O-O-O-O\n"
"  |\\|/|\\|/|\n"
"d O-O-O-O-O\n"
"  |/|\\|/|\\|\n"
"e *-O-O-O-*\n"
"Dan's Move: c3\n\n"
"  1 2 3 4 5\n"
"a *-O-O-O-*\n"
"  |\\|/|\\|/|\\n"
"b O-O-O-O-O\n"
"  |/|\\|/|\\|\n"
"c O-O-G-O-O\n"
"  |\\|/|\\|/|\n"
"d O-O-O-O-O\n"
"  |/|\\|/|\\|\n"
"e *-O-O-O-*\n"
"Computer's Move: e1 d1\n\n"
"  1 2 3 4 5\n"
"a *-O-O-O-*\n"
"  |\\|/|\\|/|\n"
"b O-O-O-O-O\n"
"  |/|\\|/|\\|\n"
"c O-O-G-O-O\n"
"  |\\|/|\\|/|\n"
"d *-O-O-O-O\n"
"  |/|\\|/|\\|\n"
"e O-O-O-O-*\n"
"Dan's Move: c1\n\n"
"  1 2 3 4 5\n"
"a *-O-O-O-*\n"
"  |\\|/|\\|/|\n"
"b O-O-O-O-O\n"
"  |/|\\|/|\\|\n"
"c G-O-G-O-O\n"
"  |\\|/|\\|/|\n"
"d *-O-O-O-O\n"
"  |/|\\|/|\\|\n"
"e O-O-O-O-*\n"
"Computer's Move: d1 b1\n\n"
"  1 2 3 4 5\n"
"a *-O-O-O-*\n"
"  |\\|/|\\|/|\n"
"b *-O-O-O-O\n"
"  |/|\\|/|\\|\n"
"c O-O-G-O-O\n"
"  |\\|/|\\|/|\n"
"d O-O-O-O-O\n"
"  |/|\\|/|\\|\n"
"e O-O-O-O-*\n"
"Dan's Move: d3\n\n"
"etc...";


/*************************************************************************
**
** #defines and structs
**
**************************************************************************/
#define WIDTH_MAX   5
#define LENGTH_MAX  5
#define WIDTH_MIN   3
#define LENGTH_MIN  3
#define GOATS_MAX   20
#define TIGERS_MAX  10

#define GOAT        'G'
#define TIGER       'T'
#define SPACE       '+'
#define PLAYER_ONE  1
#define PLAYER_TWO  2
#define UP          0
#define DOWN        1
#define UP_RIGHT    2
#define UP_LEFT     3
#define DOWN_RIGHT  4
#define DOWN_LEFT   5
#define RIGHT       6
#define LEFT        7

/*************************************************************************
**
** Global Variables
**
*************************************************************************/
int width      = 0;
int length     = 0;
int boardSize  = 0;
int tigers     = 0;
int goats      = 0;
int NumGoats   = 0;

BOOLEAN phase1 = TRUE;
BOOLEAN set    = FALSE;
BOOLEAN retrograde = TRUE;

/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/
int translate (int x, int y);
int get_x (int index);
int get_y (int index);
POSITION hash (char* board, int player);
char* unhash (POSITION position);
int whoseTurn (POSITION position);
void ChangeBoardSize ();
void SetNumGoats ();
void SetNumTigers ();
void Reset ();
void SetupHash ();
void SetInitialPosition ();
void TestHash ();

VALUE DetermineRetrogradeValue(POSITION position);
void SolveUpToTierN(int tierMax);
int WhichTier(POSITION position);

/* External */
extern GENERIC_PTR  SafeMalloc ();
extern void     SafeFree ();

/************************************************************************
**
** NAME:        InitializeGame
**
** DESCRIPTION: Prepares the game for execution.
**              Initializes required variables.
**              Sets up gDatabase (if necessary).
**
************************************************************************/

int vcfg_board(int* configuration) {
    return 1;
}

void InitializeGame ()
{
    if(!set)
        Reset();
    SetupHash();
    if(!set) {
        set = TRUE;
        SetInitialPosition();
    }
    if (retrograde) gSolver = &DetermineRetrogradeValue;
    else gSolver = NULL;
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

MOVELIST *GenerateMoves    (POSITION position)
{/*             UP
				^
		LEFT<---|--> RIGHT
				v
			   DOWN
	(1,1) IS top left corner and (row,col)=(length, width)= bottom right corner
 */
    char* board = unhash(position);
    int turn = whoseTurn(position);
    char animal;
	int row = length, col = width;
    MOVELIST *moves = NULL;
    int i, j;
    if(NumGoats != 0 && turn == PLAYER_ONE)    {
    // Generates the list of all possible drop locations for Phase 1
        for(i = 1; i <= row; i++) {
            for(j = 1; j <= col; j++)    {
                if(board[translate(i, j)] == SPACE)
                    moves = CreateMovelistNode(translate(i, j), moves);
            }
        }
    }
    else {
    // Generates the list of all possible jump/ move locations for Phase 2
        if(PLAYER_ONE == turn)
            animal = GOAT;
        else
            animal = TIGER;

        for(i = 1; i <= row; i++) {
            for(j = 1; j <= col; j++)    {
                if(board[translate(i, j)] == animal) {
                    // Move Up
                    if((i-1 > 0) && (board[translate(i-1, j)] == SPACE))
                        moves = CreateMovelistNode((translate(i, j)*8+UP)*2 + boardSize, moves);
                    // Move Right
                    if((j+1 <= col) && (board[translate(i, j+1)] == SPACE))
                        moves = CreateMovelistNode((translate(i, j)*8+RIGHT)*2 + boardSize, moves);
                    // Move Down
                    if((i+1 <= row) && (board[translate(i+1, j)] == SPACE))
                        moves = CreateMovelistNode((translate(i, j)*8+DOWN)*2 + boardSize, moves);
                    // Move Left
                    if((j-1 > 0) && (board[translate(i, j-1)] == SPACE))
                        moves = CreateMovelistNode((translate(i, j)*8+LEFT)*2 + boardSize, moves);
                    //DIAGONAL MOVES
                    // Move NW
                    if((i-1 > 0) && (j-1 > 0) && (((i + j) % 2) == 0)    &&
                        (board[translate(i-1, j-1)] == SPACE))
                        moves = CreateMovelistNode((translate(i, j)*8+UP_LEFT)*2 + boardSize, moves);
                    // Move NE
                    if((i-1 > 0) && (j+1 <= col) && (((i + j) % 2) == 0) &&
                        (board[translate(i-1, j+1)] == SPACE))
                        moves = CreateMovelistNode((translate(i, j)*8+UP_RIGHT)*2 + boardSize,moves);
                    // Move SE
                    if((i+1 <= row) && (j+1 <= col) && (((i + j) % 2) == 0) &&
                        (board[translate(i+1, j+1)] == SPACE))
                        moves = CreateMovelistNode((translate(i, j)*8+DOWN_RIGHT)*2 + boardSize, moves);
                    // Move SW
                    if((i+1 <= row) && (j-1 > 0) && (((i + j) % 2) == 0) &&
                        (board[translate(i+1, j-1)] == SPACE))
                        moves = CreateMovelistNode((translate(i, j)*8+DOWN_LEFT)*2 + boardSize, moves);
                    if(animal == TIGER)    {
                        // tigers can jump
                        // Jump Up
                        if((i-1 > 1) && (board[translate(i-1, j)] == GOAT) &&
                            (board[translate(i-2, j)] == SPACE))
                            moves = CreateMovelistNode((translate(i, j)*8+UP)*2+1 + boardSize, moves);
                        // Jump Right
                        if((j+1 < width) && (board[translate(i, j+1)] == GOAT)    &&
                            (board[translate(i, j+2)] == SPACE))
                            moves = CreateMovelistNode((translate(i, j)*8+RIGHT)*2+1 + boardSize, moves);
                        // Jump Down
                        if((i+1 < row) && (board[translate(i+1, j)] == GOAT) &&
                            (board[translate(i+2, j)] == SPACE))
                            moves = CreateMovelistNode((translate(i, j)*8+DOWN)*2+1 + boardSize, moves);
                        // Jump Left
                        if((j-1 > 1) && (board[translate(i, j-1)] == GOAT) &&
                            (j-2 > 0) && (board[translate(i, j-2)] == SPACE))
                            moves = CreateMovelistNode((translate(i, j)*8+LEFT)*2+1 + boardSize, moves);
                        // Jump SW
                        if((i+1 < length) && (j-1 > 1)    &&
                            (board[translate(i+1, j-1)] == GOAT)    &&
                            (((i + j) % 2) == 0)    &&
                            (board[translate(i+2, j-2)] == SPACE))
                            moves = CreateMovelistNode((translate(i, j)*8+DOWN_LEFT)*2+1 + boardSize, moves);
                        // Jump SE
                        if((i+1 < length) && (j+1 < width) &&
                            (board[translate(i+1, j+1)] == GOAT)    &&
                            (((i + j) % 2) == 0)    &&
                            (board[translate(i+2, j+2)] == SPACE))
                            moves = CreateMovelistNode((translate(i, j)*8+DOWN_RIGHT)*2+1 + boardSize, moves);
                        // Jump NE
                        if((i-1 > 1) && (j+1 < width) &&
							(board[translate(i-1, j+1)] == GOAT)&& (((i+j) %2) == 0) &&
                            (board[translate(i-2, j+2)] == SPACE))
                            moves = CreateMovelistNode((translate(i, j)*8+UP_RIGHT)*2+1 + boardSize, moves);
                        // Jump NW
                        if((i-1 > 1) && (j-1 > 1) &&
                            (board[translate(i-1, j-1)] == GOAT) &&
							(i-2 >0) && (j-2 >0) && (((i + j) % 2) == 0) && (board[translate(i-2, j-2)] ==  SPACE))
                            moves = CreateMovelistNode((translate(i, j)*8+UP_LEFT)*2+1 + boardSize, moves);
                    }
                }
            }
        }
    }
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
    char* board = unhash(position);
    char piece;
    int turn = whoseTurn(position);
    int jump, direction, i, j, jumpI, jumpJ;
    if(move < boardSize) { // It's only a Goat being placed
        board[move] = GOAT;
        NumGoats--;
        return hash(board, 2);
    }
	move -= boardSize;
    jump = move % 2;
    move /= 2;
    direction = move % 8;
    move /= 8;
    i = jumpI = get_x(move);
    j = jumpJ = get_y(move);
    piece = board[move];
    board[move] = SPACE; // clear the current location of the piece
    switch(direction) { // find the appropriate modifiers to i and j
        case UP: if (jump) { i -= 2; jumpI -= 1; }
                    else i -= 1; break;
        case DOWN: if (jump) { i += 2; jumpI += 1; }
                    else i += 1; break;
        case RIGHT: if (jump) { j += 2; jumpJ += 1; }
                    else j += 1; break;
        case LEFT: if (jump) { j -= 2; jumpJ -= 1; }
                    else j -= 1; break;
        case UP_RIGHT: if (jump) { i -= 2; j += 2; jumpI -= 1; jumpJ += 1; }
                    else { i -= 1; j += 1; }break;
        case UP_LEFT: if (jump) { i -= 2; j -= 2; jumpI -= 1; jumpJ -= 1; }
                    else { i -= 1; j -= 1; } break;
        case DOWN_RIGHT: if (jump) { i += 2; j += 2; jumpI += 1; jumpJ += 1; }
                    else { i += 1; j += 1; } break;
        case DOWN_LEFT: if (jump) { i += 2; j -= 2; jumpI += 1; jumpJ -= 1; }
                    else { i += 1; j -= 1; } break;
    }
    board[translate(i, j)] = piece; // place the piece in its new location
    board[translate(jumpI, jumpJ)] = SPACE; // erase the piece jumped over
    return hash(board, (turn == PLAYER_ONE ? PLAYER_TWO : PLAYER_ONE));
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
    MOVELIST* moves = GenerateMoves(position);
    if(NULL == moves) { // no legal moves left == lose
        FreeMoveList(moves);
        return lose;
    }
    FreeMoveList(moves);
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
    char* board;
    int i, j;
    int turn = whoseTurn(position);
    board = unhash(position);
    if(turn == 1) { // if goat's turn
        if(NumGoats != 0)
            kHelpOnYourTurn = kHelpOnYourTurnGoatPhaseOne;
        else kHelpOnYourTurn = kHelpOnYourTurnGoatPhaseTwo;
    } else kHelpOnYourTurn = kHelpOnYourTurnTiger;
    printf("\t%s's Turn (%s):\n\t  ",playersName,(turn==1?"Goat":"Tiger"));
    for(j = 1; j <= width; j++) // print the column numbers
        printf("%d ", j);
    printf("\n");
    for(i = 1; i <= length; i++) { // print the rows one by one
        printf("\t%c ", i-1+'a'); // first, print the row with the pieces
        for(j = 1; j <= width; j++) {
            printf("%c", board[translate(i, j)]);
            if(j < width)
            printf("-");
        }
        printf("\n\t  ");
        if(i < length) { // then, print the row with the lines (diagonals and such)
            for(j = 1; j < width; j++) {
                if(j % 2 && i % 2)
                    printf("|\\");
                else if(j % 2 && ((i % 2) == 0))
                    printf("|/");
                else if(i % 2)
                    printf("|/");
                else
                    printf("|\\");
                if(j == width - 1)
                    printf("|");
            }
        }
        if(i == 1) {
			if(NumGoats != 0) printf("  <STAGE 1> Goats to still be placed: %d", NumGoats);
			else printf("  <STAGE 2>");
		} else if (i == 2)
			printf("    %s", GetPrediction(position, playersName, usersTurn));
        printf("\n");
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
    printf("%8s's move : ", computersName);
    PrintMove(computersMove);
    printf("\n\n");
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
    int jump, direction, i, j, x, y;
    if(move < boardSize) { // This is just a goat being placed
        i = get_x(move);
        j = get_y(move);
        printf("[%c%d]", i-1+'a', j);
    } else {
		move -= boardSize;
        jump = move % 2;
        move /= 2;
        direction = move % 8;
        move /= 8;
        i = x = get_x(move);
        j = y = get_y(move);
        switch(direction) {
            case UP: if(jump) x -= 2;
                        else x -= 1; break;
            case LEFT: if(jump) y -= 2;
                        else y -= 1; break;
            case RIGHT: if(jump) y += 2;
                        else y += 1; break;
            case DOWN: if(jump) x += 2;
                        else x += 1; break;
            case UP_LEFT: if(jump) { y -= 2; x -= 2; }
                        else { y -= 1; x -= 1; } break;
            case UP_RIGHT: if(jump) { y += 2; x -= 2;  }
                        else { y += 1; x -= 1; } break;
            case DOWN_LEFT: if(jump) { y -= 2; x += 2; }
                        else { y -= 1; x += 1; } break;
            case DOWN_RIGHT: if(jump) { y += 2; x += 2; }
                        else { y += 1; x += 1; } break;
        }
        printf("[%c%d %c%d]", i-1+'a', j, x-1+'a', y);
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

    for (;;) {
        /***********************************************************
         * CHANGE THE LINE BELOW TO MATCH YOUR MOVE FORMAT
         ***********************************************************/
    if(NumGoats != 0)
        printf("%8s's move [(undo)/([%c-%c][%d-%d])] : ", playersName, 'a', length+'a'-1, 1, width);
    else
        printf("%8s's move [(undo)/([%c-%c][%d-%d] [%c-%c][%d-%d])] : ", playersName, 'a', length+'a'-1, 1, width, 'a', length+'a', 1, width);
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
	/* DON'T TOUCH!
	This writes ALL the values of the current database into a file.
	Used for debugging/haxor purposes.

	FILE *f1; int p;
	f1 = fopen ("db.txt","wt");
	for (p = 0; p < gNumberOfPositions; p++)
		 fprintf (f1, "%d\t%d\t%d\n", p, GetValueOfPosition(p), Remoteness(p));
	fclose (f1);
	*/

    int i;
    int size = strlen(input);
    if(size != 2 && size != 5)
        return FALSE;
    for(i = 0; input[i] != '\0'; i++)
        if((input[i] < '0' || input[i] > '9') &&
           (input[i] < 'a' || input[i] > 'z') &&
           (input[i] < 'A' || input[i] > 'Z') &&
           (input[i] != ' '))
            return FALSE;
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
    int x, y, x1, y1;
    MOVE move;
    x = input[0] - 'a' + 1;
    y = input[1] - '0';
    if(input[2] == '\0') {
        move = translate(x, y);
        return move;
    }
    x1 = input[3] - 'a' + 1;
    y1 = input[4] - '0';
    if(x1 == x+2) {
        if(y1 == y+2) {
            move = (translate(x, y)*8+DOWN_RIGHT)*2+1 + boardSize;
            PrintMove(move);
            return move;
        }
        else if(y1 == y)
            return (MOVE) (translate(x, y)*8+DOWN)*2+1 + boardSize;
        else
            return (MOVE) (translate(x, y)*8+DOWN_LEFT)*2+1 + boardSize;
    } else if(x1 == x+1) {
        if(y1 == y+1)
            return (MOVE) (translate(x, y)*8+DOWN_RIGHT)*2 + boardSize;
        else if(y1 == y)
            return (MOVE) (translate(x, y)*8+DOWN)*2 + boardSize;
        else
            return (MOVE) (translate(x, y)*8+DOWN_LEFT)*2 + boardSize;
    } else if(x1 == x) {
        if(y1 == y+2)
            return (MOVE) (translate(x, y)*8+RIGHT)*2+1 + boardSize;
        else if(y1 == y+1)
            return (MOVE) (translate(x, y)*8+RIGHT)*2 + boardSize;
        else if(y1 == y-1)
            return (MOVE) (translate(x, y)*8+LEFT)*2 + boardSize;
        else
            return (MOVE) (translate(x, y)*8+LEFT)*2+1 + boardSize;
    } else if(x1 == x-1) {
        if(y1 == y+1)
            return (MOVE) (translate(x, y)*8+UP_RIGHT)*2 + boardSize;
        else if(y1 == y)
            return (MOVE) (translate(x, y)*8+UP)*2 + boardSize;
        else
            return (MOVE) (translate(x, y)*8+UP_LEFT)*2 + boardSize;
    } else {
        if(y1 == y+2)
            return (MOVE) (translate(x, y)*8+UP_RIGHT)*2+1 + boardSize;
        else if(y1 == y)
            return (MOVE) (translate(x, y)*8+UP)*2+1 + boardSize;
        else
            return (MOVE) (translate(x, y)*8+UP_LEFT)*2+1 + boardSize;
    }
    return 0;
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
	char* initial;
    char c;
    BOOLEAN cont = TRUE;
    c = getc(stdin);
    while(cont) {
        printf("\n\nCurrent %dx%d board with %d goats:  \n", width, length, goats);
        PrintPosition(gInitialPosition, "Fred", 0);
        printf("\tGame Options:\n\n"
               "\tc)\t(C)hange the board size (nxn), currently: %d\n"
               "\ts)\t(S)et the number of goats on the board, currently: %d\n"
               "\tn)\tSet the (N)umber of tigers on the board, currently: %d\n"
               "\ti)\tSet the (I)nitial position (starting position)\n"
               "\tr)\t(R)eset to default settings\n"
               "\tt)\t(T)est the hash function\n"
               "\te)\tUse the (E)xperimental Retrograde Solver!\n"
               "\t1)\tSetup Board (0) (3x3, Stage 2 Initial, MaxTier: 1)\n"
        	   "\t1)\tSetup Board (1) (3x3, Stage 2 Initial, MaxTier: 4)\n"
        	   "\t2)\tSetup Board (2) (4x4, Stage 2 Initial, MaxTier: 11)\n"
        	   "\t3)\tSetup Board (3) (5x5, Stage 2 w/1 Goat, MaxTier: 1)\n"
        	   "\t4)\tSetup Board (4) (5x5, Stage 2 w/2 Goat, MaxTier: 2)\n"
        	   "\t5)\tSetup Board (5) (5x5, Stage 2 w/3 Goat, MaxTier: 3)\n"
               "\tb)\t(B)ack to the main menu\n"
               "\nSelect an option:  ", width, goats, tigers);
        scanf("%c", &c);
        switch(c) {
            case 'c': case 'C':
                ChangeBoardSize();
                break;
            case 'n': case 'N':
                SetNumTigers();
                break;
            case 's': case 'S':
                SetNumGoats();
                break;
            case 'i': case 'I':
                gInitialPosition = GetInitialPosition();
                break;
            case 'r': case 'R':
                Reset();
                SetupHash();
                SetInitialPosition();
                break;
            case 't': case 'T':
                TestHash();
                break;
            case 'b': case 'B':
                cont = FALSE;
                break;
            case 'e': case 'E':
            	if (retrograde)
					retrograde = FALSE;
				else retrograde = TRUE;
				printf("Using Retrograde Solver? = %d", retrograde);
            	break;
            case '0':
				width = length = 3; boardSize = width*length; tigers = 4;
				goats = 1; NumGoats = 0; phase1 = FALSE;
				initial = SafeMalloc(boardSize * sizeof(char));
				sprintf(initial,"T+T+G+T+T");
				SetupHash();
				gInitialPosition = hash(initial, PLAYER_ONE);
				break;
            case '1':
				width = length = 3; boardSize = width*length; tigers = 4;
				goats = boardSize-tigers-1; NumGoats = 0; phase1 = FALSE;
				initial = SafeMalloc(boardSize * sizeof(char));
				sprintf(initial,"TGTG+GTGT");
				SetupHash();
				gInitialPosition = hash(initial, PLAYER_TWO);
				break;
            case '2':
				width = length = 4; boardSize = width*length; tigers = 4;
				goats = boardSize-tigers-1; NumGoats = 0; phase1 = FALSE;
				initial = SafeMalloc(boardSize * sizeof(char));
				sprintf(initial,"TGGTGG+GGGGGTGGT");
				SetupHash();
				gInitialPosition = hash(initial, PLAYER_ONE);
				break;
            case '3':
				width = length = 5; boardSize = width*length; tigers = 4;
				goats = 1; NumGoats = 0; phase1 = FALSE;
				initial = SafeMalloc(boardSize * sizeof(char));
				sprintf(initial,"T+++T+++++++G+++++++T+++T");
				SetupHash();
				gInitialPosition = hash(initial, PLAYER_ONE);
				break;
            case '4':
				width = length = 5; boardSize = width*length; tigers = 4;
				goats = 2; NumGoats = 0; phase1 = FALSE;
				initial = SafeMalloc(boardSize * sizeof(char));
				initial = SafeMalloc(boardSize * sizeof(char));
				sprintf(initial,"T+++T++++++GG+++++++T+++T");
				SetupHash();
				gInitialPosition = hash(initial, PLAYER_ONE);
				break;
            case '5':
				width = length = 5; boardSize = width*length; tigers = 4;
				goats = 3; NumGoats = 0; phase1 = FALSE;
				initial = SafeMalloc(boardSize * sizeof(char));
				sprintf(initial,"T+++T++++++GGG++++++T+++T");
				SetupHash();
				gInitialPosition = hash(initial, PLAYER_ONE);
				break;
            default:
                printf("Invalid option!\n");
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
    int i, j, turn, goatsLeft;
    char line[width], first;
    for(i = 0; i < width; i++)
    	line[i] = SPACE;
    char* board = SafeMalloc(boardSize * sizeof(char));
    BOOLEAN valid = FALSE;
    while(!valid) {
        valid = TRUE;
        printf("\nCurrent Position: \n");
        PrintPosition(gInitialPosition, "Fred", 0);
        printf("Enter a character string to represent the position you want\n"
               "%c for tiger, %c for goat, %c for a space.  Example:\n"
               "For the initial position of a 5X5 board, the string is:\n"
               "%c%c%c%c%c\n"
               "%c%c%c%c%c\n"
               "%c%c%c%c%c\n"
               "%c%c%c%c%c\n"
               "%c%c%c%c%c\n\n"
               "Enter the character string: \n",TIGER,GOAT,SPACE,TIGER,SPACE,SPACE,SPACE,TIGER,
               SPACE,SPACE,SPACE,SPACE,SPACE,SPACE,SPACE,SPACE,SPACE,SPACE,SPACE,SPACE,SPACE,SPACE,SPACE,
               TIGER,SPACE,SPACE,SPACE,TIGER);
        for(i = 1; i <= length; i++) {
            printf(">");
            scanf("%s", &line);
            for(j = 1; j <= width; j++) {
                if(line[j-1] != SPACE && line[j-1] != GOAT && line[j-1] != TIGER)
                    valid = FALSE;
                board[translate(i,j)] = line[j-1];
            }
        }
        printf("Enter how many goats are left to place: ");
        scanf("%d", &goatsLeft);
        printf("Enter who you would like to go first: (g)oats or (t)igers: ");
        scanf("%c", &first);
        scanf("%c", &first);
        if(first == 'g' || first == 'G')
            turn = PLAYER_ONE;
        else if(first == 't' || first == 'G')
            turn = PLAYER_TWO;
        else valid = FALSE;
        goats = 0;
        tigers = 0;
        for(i = 0; i < boardSize; i++) {
            if(board[i] == GOAT)
                goats++;
            else if(board[i] == TIGER)
                tigers++;
        }
        goats += goatsLeft;
        if (goatsLeft < 0 || goats > GOATS_MAX || tigers > TIGERS_MAX)
        	valid = FALSE;
        NumGoats = goatsLeft;
        if(NumGoats == 0)
            phase1 = FALSE;
        else
            phase1 = TRUE;
        if(!valid)
            printf("\n\nInvalid board!!!\n\n");
    }
    SetupHash();
    return hash(board, turn);
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

int translate (int x, int y)
{
    return (x-1)*width+(y-1);
}

int get_x (int index)
{
    return index / length + 1;
}

int get_y (int index)
{
    return index % length + 1;
}

POSITION hash (char* board, int player)
{
    POSITION position = generic_hash(board, player);
    if(board != NULL)
        SafeFree(board);
    if(phase1) {
    	position *= (goats+1);
    	position += NumGoats;
    }
    return position;
}

char* unhash (POSITION position)
{
    char* board = (char *) SafeMalloc(length * width * sizeof(char));
    if(phase1) {
        NumGoats = position % (goats+1);
        position /= (goats+1);
    }
    return (char *) generic_unhash(position, board);
}

int whoseTurn (POSITION position)
{
    if(phase1)
        position /= (goats+1);
    return whoseMove(position);
}

void ChangeBoardSize ()
{
    int change;
    BOOLEAN cont = TRUE;
    while (cont) {
        cont = FALSE;
        printf("\n\nCurrent board of size %d:\n\n", width);
        PrintPosition(gInitialPosition, "Fred", 0);
        printf("\n\nEnter the new board size (%d - %d):  ", WIDTH_MIN, WIDTH_MAX);
        scanf("%d", &change);
        if(change > WIDTH_MAX || change < WIDTH_MIN) {
            printf("\nInvalid base length!\n");
            cont = TRUE;
        }
        else {
            width = length = change;
            boardSize = width*length;
            tigers = 4;
			goats = boardSize-tigers-1;
			NumGoats = goats;
            SetupHash();
            SetInitialPosition();
        }
    }
}

void SetNumGoats ()
{
    int change;
    BOOLEAN cont = TRUE;
    while (cont) {
        cont = FALSE;
        printf("\n\nCurrent number of goats %d:\n\n", goats);
        printf("\n\nEnter the new number of goats (%d - %d):  ", 1, boardSize-tigers-1);
        scanf("%d", &change);
        if(change > boardSize-tigers-1 || change < 1) {
            printf("\nInvalid number of goats for this board!\n");
            cont = TRUE;
        }
        else {
            goats = change;
            SetupHash();
            SetInitialPosition();
        }
    }
}

void SetNumTigers ()
{
    int change;
    BOOLEAN cont = TRUE;
    while (cont) {
        cont = FALSE;
        printf("\n\nCurrent number of tigers %d:\n\n", tigers);
        printf("\n\nEnter the new number of tigers (%d - %d):  ", 1, boardSize-goats-1);
        scanf("%d", &change);
        if(change > boardSize-tigers-1 || change < 1) {
            printf("\nInvalid number of tigers for this board!\n");
            cont = TRUE;
        }
        else {
            tigers = change;
            SetupHash();
            SetInitialPosition();
        }
    }
}

void Reset ()
{
    width      = 3;
    length     = 3;
    boardSize  = width*length;
    tigers     = 4;
    goats      = 4;
    NumGoats   = goats;
    phase1     = TRUE;
    kHelpOnYourTurn = kHelpOnYourTurnGoatPhaseOne;
}


void SetupHash ()
{
    int game[10] = {TIGER, tigers, tigers, GOAT, 0, goats, SPACE, boardSize - tigers - goats, boardSize - tigers, -1};
    gNumberOfPositions = generic_hash_init(boardSize, game, vcfg_board);
    if(phase1)
		gNumberOfPositions *= (goats + 1);
    printf("Number of positions: %ld\n", gNumberOfPositions);
}

void SetInitialPosition ()
{
    int i;
    char* initial = SafeMalloc(width * length * sizeof(char));
    for(i = 0; i < boardSize; i++)
        initial[i] = SPACE;
    initial[translate(1, 1)] = TIGER;
    initial[translate(1, width)] = TIGER;
    initial[translate(length, 1)] = TIGER;
    initial[translate(length, width)] = TIGER;
    gInitialPosition = hash(initial, 1);
}

void TestHash ()
{
    POSITION i, turn;
    BOOLEAN passed = TRUE;
    for(i = 0; i < gNumberOfPositions; i++) {
        turn = whoseTurn(i);
        if(i != hash(unhash(i), turn))
            passed = FALSE;
    }
    if(!passed)
        printf("\n\nThe hash had some problems!\n\n");
}

/* All the code for the (hack) Retrograde Solver is here */

VALUE DetermineRetrogradeValue(POSITION position) {
	char* initial;
    BOOLEAN cont = TRUE;
    char c = getc(stdin);
    while(cont) {
		printf("\n\nWelcome to the (Hack) Bagh Chal Retrograde Solver!\n");
        printf("Current %dx%d board with %d goats:  \n", width, length, goats);
        PrintPosition(position, "Initial", 0);
        printf("\tOptions:\n"
               "\ts)\t(S)olve for Current Initial Position (w/Zero-Mem-like Solver)\n"
               "\te)\t(E)xit without Solving\n"
               "\tFYI, there are %ld positions.\n"
               "\nSelect an option:  ", gNumberOfPositions);
        scanf("%c", &c);
        switch(c) {
            case 's': case 'S':
                SolveUpToTierN(goats);
                cont = FALSE;
                break;
            case 'e': case 'E':
                exit(0);
                break;
            default:
                printf("Invalid option!\n");
        }
    }
    printf("Exiting Retrograde Solver...\n\n");
    return GetValueOfPosition(position);
}

/* Here's an abstraction for the file type that will come later. */
typedef struct tierfile {
	POSITION position;
	POSITIONLIST *children;
	REMOTENESS maxUncWinRemoteness;
	BOOLEAN corruptedWin, seenDraw, corruptedLose;
	struct tierfile *next, *prev;
} TierFile;

// This "writes to file" which is really memory for now
TierFile* WriteToFile(POSITION position, TierFile* file){
	TierFile *tmp, *next = file;
	tmp = (TierFile *) SafeMalloc (sizeof(TierFile));
	tmp->position = position;
	tmp->children = NULL;
	tmp->maxUncWinRemoteness = 0;
	tmp->corruptedWin = tmp->seenDraw = tmp->corruptedLose = FALSE;
	tmp->prev = NULL;
	tmp->next = next;
	if (next != NULL)
		next->prev = tmp;
	return(tmp);
}

// This doesn't actually WORK right now, will fix later
TierFile* RemoveFromFile(TierFile* ptr) {
	TierFile *prev = ptr->prev, *next = ptr->next;
	prev->next = next;
	//if (ptr->children != NULL)
	//	FreePositionList(ptr->children);
	//SafeFree((GENERIC_PTR)ptr);
	return prev;
}

// This DOES though
void FreeTierFile(TierFile* ptr) {
	TierFile *last;
	while (ptr != NULL) {
		last = ptr;
		ptr = ptr->next;
		if (last->children != NULL)
			FreePositionList(last->children);
		SafeFree((GENERIC_PTR)last);
	}
}

/* This uses an algorithm that is a hybrid of the loopy and zero-memory solvers.
-It first enumerates the tier (which will be done MUCH better later).
-It also creates a file entry (for now, the TierFile class) to store info about it.
-Then, it goes through an "initial sweep", which basically makes it so that
you only have to work with Tier N at any time, since you get rid of all
the children. Thus, child pointers are created which point only to other Tier
N positions (also means you only call GenerateMoves() once). Also makes
sure to get rid of Primitives early on.
The loopy version will also use this iteration (or something LIKE it) to
generate its parent pointers (which, again, will only lead to Tier N positions).
-Then, it does the zero-memory like thing. Only it's much much faster since
it (a) only iterates through a tier, (b) already has all the child pointers
in memory so doesn't need GenerateMoves (or DoMove), and (c) those pointers
only include other Tier N positions.
The key reason it looks like hell, though, is because since this is bottom-up,
we can assign a value of WIN to something that has a LOSE child but not know
its true remoteness (since unexplored values might be LOSES with less
remoteness). So the idea is we label it a "corrupted" win. Namely, its
label is correct (it IS a win), but its remoteness might be wrong. So,
if you use a corrupted win to label yourself a lose, you become a "corrupted"
lose. Etc.
Things get really complicated from there, but essentially the key idea
is that corruption means you OVERSHOT the initial WIN remoteness. That means
that, for ANY corrupted position, whether WIN or LOSE:
True remoteness <= Corrupted Remoteness.
So for corrupted positions, we go and check if our corrupted child's remoteness
got smaller so we can update ours.
For MORE complicatedness (because, as the code shows, it's even MORE complicated),
talk to me.
-Anyway, once there's no changes in the above loop, we set the remaining
positions to DRAWs.

Now, this version is slower (and more memory costly) because it keeps the
whole tier list around and iterates through it. Later versions will have
it so that positions can be deleted from the list if they're known and
uncorrupted. The same goes for children pointer lists.
Plus this also goes ahead and solves from Tier 0 to Tier N without
breaks (since, without Scott's filedb, it's hard to do otherwise).
Also, this is VERY hacked, obviously, a problem which will also be
corrected later.

Finally, the Loopy solver won't have this "corruption" problem, but
might be actually be slower depending on the "priority queue" used
for it (because the iterations after the initial sweep are REALLY
fast even now). I intend to code it later and test it against this one.
Perhaps it doesn't even need a queue, and can just use the theory
presented here (so that a corrupted position STAYS on the fringe
and keeps updating whenever it changes).

Anyway, any questions, talk to me.
-Max
*/

void SolveUpToTierN(int tierMax) {
	printf("Solving from Tier 0 to maximum Tier %d...\n\n",tierMax);
	// massive variable inits
	int tier = 0, numKnownChildren;
	TierFile *mainFile, *file;
	POSITION pos, tierSize, numSolved, numCorrupted, move, child;
	POSITIONLIST* positions;
	MOVELIST *moves, *children;
	REMOTENESS remoteness, maxWinRemoteness, maxCorWinRemoteness, minLoseRemoteness;
	VALUE value;
	BOOLEAN change, seenLose, seenDraw, corruptedWin, corruptedLose;

	while (tier <= tierMax) { // for now, start at 0 and go to tierMax
		printf("--Solving Tier %d...\n",tier);

		printf("Initializing tier...\n");
		tierSize = numSolved = numCorrupted = 0;
		mainFile = NULL;
		for (pos = 0; pos < gNumberOfPositions; pos++) { // write to file for every tier position
			if (WhichTier(pos) != tier) continue;
			mainFile = WriteToFile(pos,mainFile);
			tierSize++;
		}
		mainFile = WriteToFile(-1, mainFile); //sentinel
		printf("Size of tier: %ld\n",tierSize);

		printf("Doing an initial sweep, and creating child pointers (could take a WHILE)...\n");
		file = mainFile->next;
		for (; file != NULL; file = file->next) {
			pos = file->position;
			if (Primitive(pos) != undecided) { // check for primitive-ness
				StoreValueOfPosition(pos,Primitive(pos));
				SetRemoteness(pos,0);
				numSolved++; //file = RemoveFromFile(file);
				continue;
			}
			moves = children = GenerateMoves(pos);
			if (moves == NULL) { // no children, a LOSE
				StoreValueOfPosition(pos,Primitive(lose));
				SetRemoteness(pos,0);
				numSolved++; //file = RemoveFromFile(file);
				continue;
			}
			numKnownChildren = 0; // otherwise, let's see what children we can get rid of.
			minLoseRemoteness = REMOTENESS_MAX;
			maxWinRemoteness = maxCorWinRemoteness = 0;
			seenLose = seenDraw = corruptedWin = corruptedLose = FALSE;
			for (; children != NULL; children = children->next) {
				numKnownChildren++;
				move = DoMove(pos,children->move);
				if (tier == 0) value = Primitive(move);
				else value = GetValueOfPosition(move);
				if (value != undecided) {
					numKnownChildren--;
					remoteness = Remoteness(move);
					if (value == tie) seenDraw = TRUE;
					else if (value == lose) {
						seenLose = TRUE;
						if (Visited(move)) {
							corruptedWin = TRUE; // NOT lower tier, but still corrupted tier N, gotta save
							file->children = StorePositionInList(move,file->children);
						}
						if (remoteness < minLoseRemoteness)
							minLoseRemoteness = remoteness;
					} else if (value == win) {
						if (Visited(move)) {
							corruptedLose = TRUE; // NOT lower tier, but still corrupted tier N, gotta save
							if (remoteness > maxCorWinRemoteness)
								maxCorWinRemoteness = remoteness;
							file->children = StorePositionInList(move,file->children);
						} else {
							if (remoteness > maxWinRemoteness)
								maxWinRemoteness = remoteness;
						}
					}
				} else file->children = StorePositionInList(move,file->children);
			}
			FreeMoveList(moves);
			if (numKnownChildren == 0) { // ALL my children were lower tier/already solved
				if (seenLose) {
					StoreValueOfPosition(pos, win);
					SetRemoteness(pos,minLoseRemoteness+1);
					if (corruptedWin) {
						MarkAsVisited(pos);
						file->corruptedWin = TRUE;
						numCorrupted++;
					} else numSolved++; //file = RemoveFromFile(file);
				} else if (seenDraw) {
					StoreValueOfPosition(pos, tie);
					SetRemoteness(pos,REMOTENESS_MAX);
					numSolved++; //file = RemoveFromFile(file);
				} else {
					StoreValueOfPosition(pos, lose);
					if (corruptedLose && maxCorWinRemoteness > maxWinRemoteness) {
						SetRemoteness(pos,maxCorWinRemoteness+1);
						file->corruptedLose = TRUE;
						file->maxUncWinRemoteness = maxWinRemoteness;
						MarkAsVisited(pos);
						numCorrupted++;
					} else {
						SetRemoteness(pos,maxWinRemoteness+1);
						numSolved++; //file = RemoveFromFile(file);
					}
				}
			} else { //ah, too bad, remember values for later:
				if (seenLose) {
					StoreValueOfPosition(pos, win);// make this a "corrupted win"
					SetRemoteness(pos,minLoseRemoteness+1);
					MarkAsVisited(pos);
					file->corruptedWin = TRUE;
					numCorrupted++;
				} else {
					if (seenDraw)
						file->seenDraw = seenDraw;
					else file->maxUncWinRemoteness = maxWinRemoteness;
				}
			}
		}
		change = TRUE; // This next loop may be replaced by the loopy method later (though it's pretty fast)
		while (change) { //Invariant: ALL that's left to check are corrupted and unknown Tier N positions.
			change = FALSE;
			if (numSolved == tierSize) continue; // if we're done, then stop the loop
			printf("%d Positions still unsolved ", tierSize-numSolved);
			printf("(%d corrupted). Doing another sweep...\n", numCorrupted);
			file = mainFile->next;
			for (; file != NULL; file = file->next) {
				pos = file->position;
				positions = file->children;
				numKnownChildren = 0;
				if (GetValueOfPosition(pos) != undecided && Visited(pos) == FALSE) continue;
				// Corrupted Lose: ALL children are KNOWN WIN values, but remoteness too high. Children are all corrupt.
				if (file->corruptedLose) {
					maxWinRemoteness = maxCorWinRemoteness = file->maxUncWinRemoteness; //this is the SMALLEST the true remoteness can be.
					for (; positions != NULL; positions = positions->next) {
						numKnownChildren++;
						child = positions->position;
						remoteness = Remoteness(child);
						if (remoteness < maxWinRemoteness) {// child is lower win, we dont care anymore
							numKnownChildren--;
							//remove child from list
						}
						if (remoteness > maxCorWinRemoteness) {
							maxCorWinRemoteness = remoteness;
							if (!Visited(child)) // if legal, change our minimum estimate
								file->maxUncWinRemoteness = maxWinRemoteness = remoteness;
						} if (!Visited(child)){ // child became legal!
							numKnownChildren--;
							//remove child from list
						}
					}
					if (maxCorWinRemoteness+1 < Remoteness(pos)) {//if our high estimate was lowered
						SetRemoteness(pos, maxCorWinRemoteness+1);
						change = TRUE;
					} if (numKnownChildren == 0){//file->children == NULL) {// I just became uncorrupted!
						UnMarkAsVisited(pos);
						numSolved++; //file = RemoveFromFile(file);
						numCorrupted--;
						change = TRUE;
					}
				// Corrupted Win: I have a KNOWN LOSE child, but not ALL my children are known,
				// so remoteness is wrong. Children are either unknown or corrupt.
				} else if (file->corruptedWin) {
					for (; positions != NULL; positions = positions->next) {
						numKnownChildren++;
						child = positions->position;
						value = GetValueOfPosition(child);
						if (value != undecided) {
							if (value == win || value == tie) {//child must have become WIN or DRAW, good
								//remove child from list
								numKnownChildren--;
								continue;
							} //else child's a LOSE, check remoteness
							remoteness = Remoteness(child);
							if (remoteness+1 < Remoteness(pos)) {//if child changed
								SetRemoteness(pos, remoteness+1);
								change = TRUE;
							} if (!Visited(child)){ // child became/is legal!
								numKnownChildren--;
								//remove child from list
							}
						}
					}
					if (numKnownChildren == 0) {//file->children == NULL) {// I just became uncorrupted!
						UnMarkAsVisited(pos);
						numSolved++; //file = RemoveFromFile(file);
						numCorrupted--;
						change = TRUE;
					}
				// Turns out there's NO corruption, ALL you've seen is wins(perhaps corrupted),
				// So some children are still unknown then!
				} else {
					//numKnownChildren = 0;
					minLoseRemoteness = REMOTENESS_MAX;
					maxWinRemoteness = maxCorWinRemoteness = file->maxUncWinRemoteness;
					seenDraw = file->seenDraw;
					seenLose = corruptedWin = corruptedLose = FALSE;
					for (; positions != NULL; positions = positions->next) {
						numKnownChildren++;
						child = positions->position;
						value = GetValueOfPosition(child);
						if (value != undecided) {
							numKnownChildren--;
							remoteness = Remoteness(child);
							if (value == tie) {
								seenDraw = TRUE;
								// remove child from list
							} else if (value == lose) {
								seenLose = TRUE;
								if (Visited(child)) corruptedWin = TRUE;
								//else remove child from list
								if (remoteness < minLoseRemoteness)
									minLoseRemoteness = remoteness;
							} else if (value == win) {
								if (Visited(child)) {
									if (remoteness > maxCorWinRemoteness)
										maxCorWinRemoteness = remoteness;
									corruptedLose = TRUE;
								} else {
									//remove child from list
									if (remoteness > maxWinRemoteness)
										maxWinRemoteness = remoteness;
								}
							}
						}
					}
					if (numKnownChildren == 0) { //no more unknown children!
						if (seenLose) {
							StoreValueOfPosition(pos, win);
							SetRemoteness(pos,minLoseRemoteness+1);
							change = TRUE;
							if (!corruptedWin) numSolved++; //file = RemoveFromFile(file);
							else { //we ARE corrupted, make Corrupted WIN
								MarkAsVisited(pos);
								file->corruptedWin = TRUE;
								numCorrupted++;
							}
						} else if (seenDraw) { // we're a DRAW through and through
							StoreValueOfPosition(pos, tie);
							SetRemoteness(pos,REMOTENESS_MAX);
							change = TRUE;
							numSolved++; //file = RemoveFromFile(file);
						} else {
							StoreValueOfPosition(pos, lose);
							change = TRUE;
							if (!corruptedLose || maxWinRemoteness > maxCorWinRemoteness) {
								SetRemoteness(pos,maxWinRemoteness+1);
								numSolved++; //file = RemoveFromFile(file);
							} else {//we ARE corrupted, make Corrupted LOSE
								SetRemoteness(pos,maxCorWinRemoteness+1);
								MarkAsVisited(pos);
								file->corruptedLose = TRUE;
								file->maxUncWinRemoteness = maxWinRemoteness;
								numCorrupted++;
							}
						}
					} else if (seenLose) {
						StoreValueOfPosition(pos, win);// make this a "corrupted win"
						SetRemoteness(pos,minLoseRemoteness+1);
						MarkAsVisited(pos);
						file->corruptedWin = TRUE;
						numCorrupted++;
						change = TRUE;
					} else { // STILL unknown, let's save if we're a future DRAW and our max remoteness so far
						if (seenDraw) file->seenDraw = seenDraw;
						else file->maxUncWinRemoteness = maxWinRemoteness;
					}
				}
			}
		}
		printf("Setting undecided to DRAWs and correcting corruption...\n");
		file = mainFile->next;
		for (; file != NULL; file = file->next) {
			pos = file->position;
			if (GetValueOfPosition(pos) == undecided) {
				StoreValueOfPosition(pos, tie);
				SetRemoteness(pos,REMOTENESS_MAX);
			} else if (Visited(pos)) UnMarkAsVisited(pos); // must've been corrupted, now TRUE
			//file = RemoveFromFile(file);
		}
		//mainFile = RemoveFromFile(mainFile);
		FreeTierFile(mainFile); // now fully deleted
		tier++; // done with this tier!
	}
	printf("SAVED!!\n");
}
/* Just a prototype for EnumerateTier/Positions
POSITIONLIST* EnumeratePositions(int tier) {
	POSITIONLIST *positions = NULL;
	POSITION p;
	for (p = 0; p < gNumberOfPositions; p++)
		if (WhichTier(p) == tier)
			positions = StorePositionInList(p, positions);
	return positions;
}
*/
int WhichTier(POSITION position) {
	char* board;
	int goats, i;
	board = unhash(position); goats = 0;
	for(i = 0; i < boardSize; i++)
		if(board[i] == GOAT)
			goats++;
	SafeFree(board);
	return goats;
}
