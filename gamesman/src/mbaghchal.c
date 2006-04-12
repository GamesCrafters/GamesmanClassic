// $Id: mbaghchal.c,v 1.12 2006-04-12 03:02:12 max817 Exp $
// $Log: not supported by cvs2svn $

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
** DATE:        2006.4.11
**
** UPDATE HIST: -2004.10.21 = Original (Dom's) Version
**              -2006.3.2 = Updates + Fixes by Max and Deepa
**                  (Nearly) functional Standard version of game
**              -2006.3.20 = Includes hack version of the Retrograde Solver
**					(FAR from complete, but at least it's a working version)
**					Also a few changes to the game, particular the board display.
**				-2006.4.11 = Retrograde Solver now moved to its own file.
**					Diagonals and Variants now implemented correctly.
**					Fixed a few more little bugs, non 5x5 game is fully complete.
**					Added RetrogradeTierValue function pointer implementation.
**					Added CheckLegality code, but will implement in next update.
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
#include "hash.h"
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>


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
"a T-+-+-+-T\n"
"  |\\|/|\\|/|\n"
"b +-+-+-+-+\n"
"  |/|\\|/|\\|\n"
"c +-+-+-+-+\n"
"  |\\|/|\\|/|\n"
"d +-+-+-+-+\n"
"  |/|\\|/|\\|\n"
"e T-+-+-+-T\n"
"Dan's Move: c3\n\n"
"  1 2 3 4 5\n"
"a T-+-+-+-T\n"
"  |\\|/|\\|/|\\n"
"b +-+-+-+-+\n"
"  |/|\\|/|\\|\n"
"c +-+-G-+-+\n"
"  |\\|/|\\|/|\n"
"d +-+-+-+-+\n"
"  |/|\\|/|\\|\n"
"e T-+-+-+-T\n"
"Computer's Move: e1 d1\n\n"
"  1 2 3 4 5\n"
"a T-+-+-+-T\n"
"  |\\|/|\\|/|\n"
"b +-+-+-+-+\n"
"  |/|\\|/|\\|\n"
"c +-+-G-+-+\n"
"  |\\|/|\\|/|\n"
"d T-+-+-+-+\n"
"  |/|\\|/|\\|\n"
"e +-+-+-+-T\n"
"Dan's Move: c1\n\n"
"  1 2 3 4 5\n"
"a T-+-+-+-T\n"
"  |\\|/|\\|/|\n"
"b +-+-+-+-+\n"
"  |/|\\|/|\\|\n"
"c G-+-G-+-+\n"
"  |\\|/|\\|/|\n"
"d T-+-+-+-+\n"
"  |/|\\|/|\\|\n"
"e +-+-+-+-T\n"
"Computer's Move: d1 b1\n\n"
"  1 2 3 4 5\n"
"a T-+-+-+-T\n"
"  |\\|/|\\|/|\n"
"b T-+-+-+-+\n"
"  |/|\\|/|\\|\n"
"c +-+-G-+-+\n"
"  |\\|/|\\|/|\n"
"d +-+-+-+-+\n"
"  |/|\\|/|\\|\n"
"e +-+-+-+-T\n"
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
BOOLEAN diagonals = TRUE;
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
BOOLEAN CheckLegality (POSITION position);
MOVELIST* GenerateUndoMoves ( POSITION position);
int RetrogradeTierValue(POSITION position);

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
    if (retrograde) gRetrogradeTierValue = &RetrogradeTierValue;
    else gRetrogradeTierValue = NULL;
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
					if(diagonals){// Move NW
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
					}
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
						if(diagonals){// Jump SW
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
					if (diagonals){case UP_RIGHT: if (jump) { i -= 2; j += 2; jumpI -= 1; jumpJ += 1; }
                    else { i -= 1; j += 1; }break;
        case UP_LEFT: if (jump) { i -= 2; j -= 2; jumpI -= 1; jumpJ -= 1; }
                    else { i -= 1; j -= 1; } break;
        case DOWN_RIGHT: if (jump) { i += 2; j += 2; jumpI += 1; jumpJ += 1; }
                    else { i += 1; j += 1; } break;
        case DOWN_LEFT: if (jump) { i += 2; j -= 2; jumpI += 1; jumpJ -= 1; }
                    else { i += 1; j -= 1; } break;
					}
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
				if(diagonals) {
					if(j % 2 && i % 2)
						printf("|\\");
					else if(j % 2 && ((i % 2) == 0))
						printf("|/");
					else if(i % 2)
						printf("|/");
					else
						printf("|\\");
				}
				else printf("| ");

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
			if(diagonals){

            case UP_LEFT: if(jump) { y -= 2; x -= 2; }
                        else { y -= 1; x -= 1; } break;
            case UP_RIGHT: if(jump) { y += 2; x -= 2;  }
                        else { y += 1; x -= 1; } break;
            case DOWN_LEFT: if(jump) { y -= 2; x += 2; }
                        else { y -= 1; x += 1; } break;
            case DOWN_RIGHT: if(jump) { y += 2; x += 2; }
                        else { y += 1; x += 1; } break;
			}
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
    int size = strlen(input);
    if(size != 2 && size != 5)
        return FALSE;
    if (!isalpha(input[0]) || !isdigit(input[1]) ||
 		(size == 2 || (input[2] != ' ' ||
 		!isalpha(input[3]) || !isdigit(input[4]))))
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
        if(y1 == y+2 && diagonals) {
            move = (translate(x, y)*8+DOWN_RIGHT)*2+1 + boardSize;
            PrintMove(move);
            return move;
        }
        else if(y1 == y)
            return (MOVE) (translate(x, y)*8+DOWN)*2+1 + boardSize;
        else if(diagonals)
            return (MOVE) (translate(x, y)*8+DOWN_LEFT)*2+1 + boardSize;
		else return 0;
    } else if(x1 == x+1) {
        if(y1 == y+1 && diagonals)
            return (MOVE) (translate(x, y)*8+DOWN_RIGHT)*2 + boardSize;
        else if(y1 == y)
            return (MOVE) (translate(x, y)*8+DOWN)*2 + boardSize;
        else if(diagonals)
            return (MOVE) (translate(x, y)*8+DOWN_LEFT)*2 + boardSize;
		else
			return 0;
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
        if(y1 == y+1 && diagonals)
            return (MOVE) (translate(x, y)*8+UP_RIGHT)*2 + boardSize;
        else if(y1 == y)
            return (MOVE) (translate(x, y)*8+UP)*2 + boardSize;
        else if (diagonals)
            return (MOVE) (translate(x, y)*8+UP_LEFT)*2 + boardSize;
		else return 0;
    } else {
        if(y1 == y+2 && diagonals)
            return (MOVE) (translate(x, y)*8+UP_RIGHT)*2+1 + boardSize;
        else if(y1 == y)
            return (MOVE) (translate(x, y)*8+UP)*2+1 + boardSize;
        else if(diagonals)
            return (MOVE) (translate(x, y)*8+UP_LEFT)*2+1 + boardSize;
		else return 0;
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
    while(cont) {
        printf("\n\nCurrent %dx%d board with %d goats:  \n", width, length, goats);
        PrintPosition(gInitialPosition, "Fred", 0);
        printf("\tGame Options:\n\n"
               "\tc)\t(C)hange the board size (nxn), currently: %d\n"
               "\ts)\t(S)et the number of goats on the board, currently: %d\n"
               "\tn)\tSet the (N)umber of tigers on the board, currently: %d\n"
			   "\td)\tTurn (D)iagonals %s\n"
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
        	   "\tf)\t(F)reaking delete the current database (to re-solve)\n"
               "\tb)\t(B)ack to the main menu\n"
               "\nSelect an option:  ", width, goats, tigers, diagonals ? "off" : "on");
        c = GetMyChar();
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
			case 'd': case 'D':
				diagonals= !diagonals;
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
			case 'f': case 'F':
				// HAXXX WARNING!! This clears the current databases for this game:
				system("rm ./data/mbaghchal_*_memdb.dat.gz");
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
            scanf("%s", line);
            for(j = 1; j <= width; j++) {
                if(line[j-1] != SPACE && line[j-1] != GOAT && line[j-1] != TIGER)
                    valid = FALSE;
                board[translate(i,j)] = line[j-1];
            }
        }
        printf("Enter how many goats are left to place: ");
        goatsLeft = GetMyChar()-48;
        printf("Enter who you would like to go first: (g)oats or (t)igers: ");
        first = GetMyChar();
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
	int option = 0;
	option += ((length-3)*TIGERS_MAX*GOATS_MAX + tigers*GOATS_MAX + goats) *2;
	if (diagonals == FALSE)
		option +=1;
    return option;
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
	if(option%2==1){
		diagonals = FALSE;
		option-=1;
	}
	else
		diagonals = TRUE;
	option /= 2;
	length = option / (TIGERS_MAX * GOATS_MAX);
	tigers = (option % (TIGERS_MAX * GOATS_MAX)) / GOATS_MAX;
	goats = (option % (TIGERS_MAX * GOATS_MAX)) % GOATS_MAX;
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
        change = GetMyChar()-48;
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
			phase1 = TRUE;
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
        change = GetMyChar()-48;
        if(change > boardSize-tigers-1 || change < 1) {
            printf("\nInvalid number of goats for this board!\n");
            cont = TRUE;
        }
        else {
            goats = change;
            NumGoats = goats;
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
        change = GetMyChar()-48;
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
    diagonals  = TRUE;
    kHelpOnYourTurn = kHelpOnYourTurnGoatPhaseOne;
}


void SetupHash ()
{
    int game[10] = {TIGER, tigers, tigers, GOAT, 0, goats, SPACE, boardSize - tigers - goats, boardSize - tigers, -1};
    gNumberOfPositions = generic_hash_init(boardSize, game, vcfg_board);
    if(phase1)
		gNumberOfPositions *= (goats + 1);
    printf("Number of positions: %lld\n", gNumberOfPositions);
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


BOOLEAN CheckLegality (POSITION position) {
  char* board = unhash(position);
  int goatsOnBoard = 0, i, j;
  //determines the number of goats on the board
  for (i = 1; i <= length; i++){
	for (j = 1; j <= width; j++){
		if (board[translate(i,j)] == GOAT) goatsOnBoard++;
	}
  }
  if (phase1 == TRUE && (goatsOnBoard + NumGoats) != goats)
    return FALSE;
  MOVELIST* moves = GenerateUndoMoves(position);
  if (moves == NULL)
    return FALSE;
  else return TRUE;
}

MOVELIST* GenerateUndoMoves ( POSITION position){
	/*             UP
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
                if(board[translate(i, j)] == GOAT)
                    moves = CreateMovelistNode(translate(i, j), moves);
            }
        }
    }
    else {
    // Generates the list of all possible jump/ move locations for Phase 2
        if(PLAYER_ONE == turn)
            animal = TIGER;//opposite of what it should be, means player 2, tiger, just moved
        else
            animal = GOAT; // means player 1 = goat just moved

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
					if(diagonals){
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
					}
                    if(animal == TIGER)    {
                        // tigers can jump
                        // Jump Up
                        if((i-1 > 1) && (board[translate(i-1, j)] == SPACE) &&
                            (board[translate(i-2, j)] == GOAT))
                            moves = CreateMovelistNode((translate(i, j)*8+UP)*2+1 + boardSize, moves);
                        // Jump Right
                        if((j+1 < width) && (board[translate(i, j+1)] == SPACE)    &&
                            (board[translate(i, j+2)] == GOAT))
                            moves = CreateMovelistNode((translate(i, j)*8+RIGHT)*2+1 + boardSize, moves);
                        // Jump Down
                        if((i+1 < row) && (board[translate(i+1, j)] == SPACE) &&
                            (board[translate(i+2, j)] == GOAT))
                            moves = CreateMovelistNode((translate(i, j)*8+DOWN)*2+1 + boardSize, moves);
                        // Jump Left
                        if((j-1 > 1) && (board[translate(i, j-1)] == SPACE) &&
                            (j-2 > 0) && (board[translate(i, j-2)] == GOAT))
                            moves = CreateMovelistNode((translate(i, j)*8+LEFT)*2+1 + boardSize, moves);
                        // Jump SW
						if(diagonals){
						if((i+1 < length) && (j-1 > 1)    &&
                            (board[translate(i+1, j-1)] == SPACE)    &&
                            (((i + j) % 2) == 0)    &&
                            (board[translate(i+2, j-2)] == GOAT))
                            moves = CreateMovelistNode((translate(i, j)*8+DOWN_LEFT)*2+1 + boardSize, moves);
                        // Jump SE
                        if((i+1 < length) && (j+1 < width) &&
                            (board[translate(i+1, j+1)] == SPACE)    &&
                            (((i + j) % 2) == 0)    &&
                            (board[translate(i+2, j+2)] == GOAT))
                            moves = CreateMovelistNode((translate(i, j)*8+DOWN_RIGHT)*2+1 + boardSize, moves);
                        // Jump NE
                        if((i-1 > 1) && (j+1 < width) && (board[translate(i-1, j+1)] == SPACE)&& (((i+j) %2) == 0) &&
                            (board[translate(i-2, j+2)] == GOAT))
                            moves = CreateMovelistNode((translate(i, j)*8+UP_RIGHT)*2+1 + boardSize, moves);
                        // Jump NW
                        if((i-1 > 1) && (j-1 > 1) &&
                            (board[translate(i-1, j-1)] == SPACE) &&
	                        (i-2 >0) && (j-2 >0) && (((i + j) % 2) == 0) && (board[translate(i-2, j-2)] ==  GOAT))
                            moves = CreateMovelistNode((translate(i, j)*8+UP_LEFT)*2+1 + boardSize, moves);
						}
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
** NAME:        RetrogradeTierValue
**
** DESCRIPTION: The ONE function the retrograde solver requires modules to code.
**				0 - 20 : Stage 2 positions.
**				21 - 40 : Stage 1 positions (for 5x5 board).
**				See core/solveretrograde.c for details.
**				This will change to a better implementation later.
**
************************************************************************/

int RetrogradeTierValue(POSITION position) {
	if (position == kBadPosition) {
		if (phase1) return goats*2;
		else return goats;
	}
	char* board = unhash(position);
	int goat = 0, i;
	for(i = 0; i < boardSize; i++)
		if(board[i] == GOAT)
			goat++;
	SafeFree(board);
	if (NumGoats > 0) {//stage one
		if (NumGoats+goat <= goats) //Check Legality
			return goats+NumGoats;
		else return -1; //a bad position
	}
	return goat;
}
