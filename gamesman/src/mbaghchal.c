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
**
** DATE:        2004.10.21
**
** UPDATE HIST: Never update anything.
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
STRING   kAuthorName          = "Damian Hites"; /* Your name(s) */
STRING   kDBName              = "mbaghchal"; /* The name to store the database under */

BOOLEAN  kPartizan            = TRUE; /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN  kGameSpecificMenu    = TRUE; /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN  kTieIsPossible       = FALSE; /* TRUE if a tie is possible. FALSE if it is impossible.*/
BOOLEAN  kLoopy               = TRUE; /* TRUE if the game tree will have cycles (a rearranger style game). FALSE if it does not.*/

BOOLEAN  kDebugMenu           = TRUE; /* TRUE only when debugging. FALSE when on release. */
BOOLEAN  kDebugDetermineValue = TRUE; /* TRUE only when debugging. FALSE when on release. */

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
#define WIDTH_MAX   5
#define LENGTH_MAX  5
#define WIDTH_MIN   1
#define LENGTH_MIN  1
#define GOAT        'G'
#define TIGER       '*'
#define SPACE       'O'
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
int width      = 5;
int length     = 5;
int boardSize  = 25;
int tigers     = 4;
int goats      = 20;
int NumGoats   = 0;

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

/* External */
extern GENERIC_PTR	SafeMalloc ();
extern void		SafeFree ();


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

int vcfg_board(int* configuration) {
    return 1;
}

void InitializeGame ()
{
    int i;
    int game[10] = {TIGER, tigers, tigers, GOAT, 0, goats, SPACE, 1, width*length - 4, -1};
    char* initial = SafeMalloc(width * length * sizeof(char));
    boardSize = width*length;
    if(goats >= boardSize-tigers)
	goats = boardSize - tigers - 1;
    NumGoats = goats;
    for(i = 0; i < boardSize; i++)
	initial[i] = SPACE;
    gNumberOfPositions = (goats+1)*generic_hash_init(boardSize, game, vcfg_board);
    initial[translate(1, 1)] = TIGER;
    initial[translate(1, width)] = TIGER;
    initial[translate(length, 1)] = TIGER;
    initial[translate(length, width)] = TIGER;
    gInitialPosition = hash(initial, 1);
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
    char* board = unhash(position);
    int turn = whoseTurn(position);
    char animal;
    MOVELIST *moves = NULL;
    int i, j;
    if(NumGoats != 0 && turn == PLAYER_ONE) {
	for(i = 1; i <= length; i++) {
	    for(j = 1; j <= width; j++) {
		if(board[translate(i, j)] == SPACE)
		    moves = CreateMovelistNode(translate(i, j), moves);
	    }
	}
    }
    else {
	int shift = boardSize;
	if(PLAYER_ONE == turn)
	    animal = GOAT;
	else
	    animal = TIGER;
	for(i = 1; i <= length; i++) {
	    for(j = 1; j <= width; j++) {
		if(board[translate(i, j)] == animal) {
		    if((i+1 <= length) && (board[translate(i+1, j)] == SPACE))
			moves = CreateMovelistNode((translate(i, j)*8+UP)*2+shift, moves);
		    if((j+1 <= width) && (board[translate(i, j+1)] == SPACE))
			moves = CreateMovelistNode((translate(i, j)*8+RIGHT)*2+shift, moves);
		    if((i-1 > 0) && (board[translate(i-1, j)] == SPACE))
			moves = CreateMovelistNode((translate(i, j)*8+DOWN)*2+shift, moves);
		    if((j-1 > 0) && (board[translate(i, j-1)] == SPACE))
			moves = CreateMovelistNode((translate(i, j)*8+LEFT)*2+shift, moves);
		    if((i+1 <= length) && (j-1 > 0) && (((i + j) % 2) == 0) &&
		       (board[translate(i+1, j-1)] == SPACE))
			moves = CreateMovelistNode((translate(i, j)*8+UP_LEFT)*2+shift, 
						   moves);
		    if((i+1 <= length) && (j+1 <= width) && (((i + j) % 2) == 0) &&
		       (board[translate(i+1, j+1)] == SPACE))
			moves = CreateMovelistNode((translate(i, j)*8+UP_RIGHT)*2+shift, 
						   moves);
		    if((i-1 > 0) && (j+1 <= width) && (((i + j) % 2) == 0) &&
		       (board[translate(i-1, j+1)] == SPACE))
			moves = CreateMovelistNode((translate(i, j)*8+DOWN_RIGHT)*2+shift, 
						   moves);
		    if((i-1 > 0) && (j-1 > 0) && (((i + j) % 2) == 0) &&
		       (board[translate(i-1, j-1)] == SPACE))
			moves = CreateMovelistNode((translate(i, j)*8+DOWN_LEFT)*2+shift, 
						   moves);
		    if(animal == TIGER) {
			if((i+1 <= length) && (board[translate(i+1, j)] == GOAT) &&
			   (i+2 <= length) && (board[translate(i+2, j)] == SPACE))
			    moves = CreateMovelistNode((translate(i, j)*8+UP)*2+1+shift, 
						       moves);
			if((j+1 <= width) && (board[translate(i, j+1)] == GOAT) &&
			   (j+2 <= width) && (board[translate(i, j+2)] == SPACE))
			    moves = CreateMovelistNode((translate(i, j)*8+RIGHT)*2+1+shift, 
						       moves);
			if((i-1 > 0) && (board[translate(i-1, j)] == GOAT) &&
			   (i-2 > 0) && (board[translate(i-2, j)] == SPACE))
			    moves = CreateMovelistNode((translate(i, j)*8+DOWN)*2+1+shift, 
						       moves);
			if((j-1 > 0) && (board[translate(i, j-1)] == GOAT) &&
			   (j-2 > 0) && (board[translate(i, j-2)] == SPACE))
			    moves = CreateMovelistNode((translate(i, j)*8+LEFT)*2+1+shift, 
						       moves);
			if((i+1 <= length) && (j-1 < 0) && 
			   (board[translate(i+1, j-1)] == GOAT) &&
			   (i+2 <= length) && (j-2 < 0) &&
			   (((i + j) % 2) == 0) &&
			   (board[translate(i+2, j-2)] == SPACE))
			    moves = CreateMovelistNode((translate(i, j)*8+UP_LEFT)*2+1+shift, 
						       moves);
			if((i+1 <= length) && (j+1 <= width) && 
			   (board[translate(i+1, j+1)] == GOAT) &&
			   (i+2 <= length) && (j+2 <= width) &&
			   (((i + j) % 2) == 0) &&
			   (board[translate(i+2, j+2)] == SPACE))
			    moves = CreateMovelistNode((translate(i, j)*8+UP_RIGHT)*2+1+shift
						       ,moves);
			if((i-1 > 0) && (j+1 <= width) && 
			   (board[translate(i-1, j+1)] == GOAT) &&
			   (i-2 > 0) && (j+2 <= width) &&
			   (((i + j) % 2) == 0) &&
			   (board[translate(i-2, j+2)] == SPACE))
			    moves = CreateMovelistNode((translate(i, j)*8+DOWN_RIGHT)*2+1+
						       shift, moves);
			if((i-1 > 0) && (j-1 > 0) && 
			   (board[translate(i-1, j-1)] == GOAT) &&
			   (((i + j) % 2) == 0) &&
			   (i-2 > 0) && (j-2 > 0) && (board[translate(i-2, j-2)] == SPACE))
			    moves = CreateMovelistNode((translate(i, j)*8+DOWN_LEFT)*2+1+
						       shift, moves);
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
    int jump, direction, i, j;
    if(move < boardSize) {
	board[move] = GOAT;
	NumGoats--;
	return hash(board, 2);
    }
    move -= boardSize;
    jump = move % 2;
    move /= 2;
    direction = move % 8;
    move /= 8;
    i = get_x(move);
    j = get_y(move);
    piece = board[move];
    board[move] = SPACE;
    switch(direction) {
    case UP:
	if(jump) {
	    board[translate(i+2, j)] = piece;
	    board[translate(i+1, j)] = SPACE;
	}
	else
	    board[translate(i+1, j)] = piece;
	break;
    case DOWN:
	if(jump) {
	    board[translate(i-2, j)] = piece;
	    board[translate(i-1, j)] = SPACE;
	}
	else
	    board[translate(i-1, j)] = piece;
	break;
    case RIGHT:
	if(jump) {
	    board[translate(i, j+2)] = piece;
	    board[translate(i, j+1)] = SPACE;
	}
	else
	    board[translate(i, j+1)] = piece;
	break;
    case LEFT:
	if(jump) {
	    board[translate(i, j-2)] = piece;
	    board[translate(i, j-1)] = SPACE;
	}
	else
	    board[translate(i, j-1)] = piece;
	break;
    case UP_RIGHT:
	if(jump) {
	    board[translate(i+2, j+2)] = piece;
	    board[translate(i+1, j+1)] = SPACE;
	}
	else
	    board[translate(i+1, j+1)] = piece;
	break;
    case UP_LEFT:
	if(jump) {
	    board[translate(i+2, j-2)] = piece;
	    board[translate(i+1, j-1)] = SPACE;
	}
	else
	    board[translate(i+1, j-1)] = piece;
	break;
    case DOWN_RIGHT:
	if(jump) {
	    board[translate(i-2, j+2)] = piece;
	    board[translate(i-1, j+1)] = SPACE;
	}
	else
	    board[translate(i-1, j+1)] = piece;
	break;
    case DOWN_LEFT:
	if(jump) {
	    board[translate(i-2, j-2)] = piece;
	    board[translate(i-1, j-1)] = SPACE;
	}
	else
	    board[translate(i-1, j-1)] = piece;
	break;
    }
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
    if(NULL == moves) {
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
    int i, j, goatCount = 0;
    int turn = whoseTurn(position);
    board = unhash(position);
    printf("\n\t  ");
    for(j = 1; j <= width; j++)
	printf("%d ", j);
    printf("\n\t");
    for(i = 1; i <= length; i++) {
	printf("%c ", i-1+'a');
	for(j = 1; j <= width; j++) {
	    printf("%c", board[translate(i, j)]);
	    if(board[translate(i, j)] == GOAT)
		goatCount++;
	    if(j < width)
		printf("-");
	}
	printf("\n\t  ");
	if(i < length) {
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
	printf("\n\t");
    }
    printf("          %s\n\n", GetPrediction(position, playersName, usersTurn));
    NumGoats = goatCount;
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
    printf("%8s's move              : ", computersName);
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
    int jump, direction, i, j;
    if(move < boardSize) {
	i = get_x(move);
	j = get_y(move);
	printf("[%c%d]", i-1+'a', j);
    }
    else {
	printf(" move: %d ",move);
	move -= boardSize;
	jump = move % 2;
	move /= 2;
	direction = move % 8;
	move /= 8;
	i = get_x(move);
	j = get_y(move);
	printf("[%c%d %d]", i-1+'a', j, direction+1);
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
    int i, x, y, x1, y1;
    int shift = boardSize;
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
	    move = (translate(x, y)*8+UP_RIGHT)*2+1+shift;
	    PrintMove(move);
	    return move;
	}
	else if(y1 == y)
	    return (MOVE) (translate(x, y)*8+UP)*2+1+shift;
	else
	    return (MOVE) (translate(x, y)*8+UP_LEFT)*2+1+shift;
    }
    else if(x1 == x+1) {
	if(y1 == y+1)
	    return (MOVE) (translate(x, y)*8+UP_RIGHT)*2+shift;
	else if(y1 == y)
	    return (MOVE) (translate(x, y)*8+UP)*2+shift;
	else
	    return (MOVE) (translate(x, y)*8+UP_LEFT)*2+shift;
    }
    else if(x1 == x) {
	if(y1 == y+2)
	    return (MOVE) (translate(x, y)*8+RIGHT)*2+1+shift;
	else if(y1 == y+1)
	    return (MOVE) (translate(x, y)*8+RIGHT)*2+shift;
	else if(y1 == y-1)
	    return (MOVE) (translate(x, y)*8+LEFT)*2+shift;
	else
	    return (MOVE) (translate(x, y)*8+LEFT)*2+1+shift;
    }
    else if(x1 == x-1) {
	if(y1 == y+1)
	    return (MOVE) (translate(x, y)*8+DOWN_RIGHT)*2+shift;
	else if(y1 == y)
	    return (MOVE) (translate(x, y)*8+DOWN)*2+shift;
	else
	    return (MOVE) (translate(x, y)*8+DOWN_LEFT)*2+shift;
    }
    else {
	if(y1 == y+2)
	    return (MOVE) (translate(x, y)*8+DOWN_RIGHT)*2+1+shift;
	else if(y1 == y)
	    return (MOVE) (translate(x, y)*8+DOWN)*2+1+shift;
	else
	    return (MOVE) (translate(x, y)*8+DOWN_LEFT)*2+1+shift;
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
    char c;
    BOOLEAN cont = TRUE;
    c = getc(stdin);
    while(cont) {
	printf("\n\nCurrent %dx%d board with %d goats:  \n", width, length, goats);
	PrintPosition(gInitialPosition, "Fred", 0);
	printf("\tGame Options:\n\n"
	       "\tc)\t(C)hange the board size (nxn), currently: %d\n"
	       "\ts)\t(S)et the number of goats on the board, currently: %d\n"
	       "\tb)\t(B)ack to the main menu\n"
	       "\nSelect an option:  ", width, goats);
	scanf("%c", &c);
	switch(c) {
	case 'c': case 'C':
	    ChangeBoardSize();
	    break;
	case 's': case 'S':
	    SetNumGoats();
	    break;
	case 'b': case 'B':
	    cont = FALSE;
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
    int i, j;
    char line[width];
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
	       "Enter the character string: \n",TIGER,GOAT,SPACE,TIGER,GOAT,GOAT,GOAT,TIGER,
	       GOAT,GOAT,GOAT,GOAT,GOAT,GOAT,GOAT,GOAT,GOAT,GOAT,GOAT,GOAT,GOAT,GOAT,GOAT,
	       TIGER,GOAT,GOAT,GOAT,TIGER);
	for(i = 1; i <= length; i++) {
	    printf(">");
	    scanf("%s", &line);
	    for(j = 1; j <= width; j++) {
		if(line[j-1] != SPACE && line[j-1] != GOAT && line[j-1] != TIGER)
		    valid = FALSE;
		board[translate(i,j)] = line[j-1];
	    }
	}
	if(!valid)
	    printf("\n\nInvalid board!!!\n\n");
    }
    return hash(board, 1);
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
    position *= (goats+1);
    position += NumGoats;
    return position;
}

char* unhash (POSITION position)
{
    char* board = (char *) SafeMalloc(length * width * sizeof(char));
    NumGoats = position % (goats+1);
    position /= (goats+1);
    return (char *) generic_unhash(position, board);
}

int whoseTurn (POSITION position)
{
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
	    InitializeGame();
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
	    printf("\nInvalid base length!\n");
	    cont = TRUE;
	}
	else {
	    goats = change;
	    InitializeGame();
	}
    }
}
