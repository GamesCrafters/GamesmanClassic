// Alex Wallisch
// $log$

/*
 * The above lines will include the name and log of the last person
 * to commit this file to CVS
 */

/************************************************************************
**
** NAME:        mqland.c
**
** DESCRIPTION: Queensland
**
** AUTHOR:      Steven Kusalo, Alex Wallisch
**
** DATE:        2004-09-13
**
** UPDATE HIST: 2004-10-03      Wrote Primitive
**				Wrote ValidTextInput
**				Wrote ConvertTextInputToMove
**                              Wrote PrintComputersMove
**                              Wrote PrintMove
**              2004-10-02:	Wrote GetMoveList
**		2004-09-27:    	Wrote vcfg
**				Wrote DoMove
**		2004-09-26:	Wrote InitializeBoard
**		2004-09-25:	Wrote PrintPosition
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
#include <ctype.h>


/*************************************************************************
**
** Game-specific constants
**
**************************************************************************/

STRING   kGameName            = "Queensland"; /* The name of your game */
STRING   kAuthorName          = "Steven Kusalo, Alex Wallisch"; /* Your name(s) */
STRING   kDBName              = "qland"; /* The name to store the database under */

BOOLEAN  kPartizan            = TRUE ; /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
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
"On your turn, use the LEGEND to choose the starting position and ending position of a piece that you want to move (or 0 if you don't want to move). Then enter an empty position where you want to place your piece."; 

STRING   kHelpOnYourTurn =
"If you want to move a piece, type the position of the piece and the position you want to move it to (e.g. \"3 1\").  Type 0 if you don't want to move.  Then, type the number of an empty position where you want to place a new piece (e.g. \"4\").  A complete move will look something like this: \"3 1 4\"";

STRING   kHelpStandardObjective =
"When all pieces are on the board, the game ends.  Any two pieces of your color that are connected by a straight (horizontal, vertical, or diagonal), unbroken line score one point for each empty space they cover.  You win if you score MORE points than your opponent.";

STRING   kHelpReverseObjective =
"When all pieces are on the board, the game ends.  Any two pieces of your color that are connected by a straight (horizontal, vertical, or diagonal), unbroken line score one point for each empty space they cover.  You win if you score MORE points than your opponent.";

STRING   kHelpTieOccursWhen =
"A tie occurs when each player has played all their pieces and have the same number of points.";

STRING   kHelpExample =
"";


/*************************************************************************
**
** #defines and structs
**
**************************************************************************/
#define BLANK '.'
#define WHITE 'X'
#define BLACK 'O'

/*typedef enum board_position { BLANK, WHITE, BLACK } BWB;
/*typedef int TURN;
struct Board_Rep {
	BWB* spaces;
	TURN turn;
};
#define getpiece(B, x, y) ((B)->spaces[(y)*width + (x) + 1])
/* Do we need getpiece and Board_Rep?? */

#define pieceat(B, x, y) ((B)[(y) * width + (x) + 1])
#define get_location(x, y) ((y) * width + (x))
#define get_x_coord(location) ((location) % width)
#define get_y_coord(location) ((location) / width)

/* The two leftmost bits of a MOVE are unused.
 * The next 10 bits encode the source of the player's SLIDE move.
 * The next 10 bits encode the destination of the player's SLIDE move.
 * The rightmost 10 bits encode the player's PLACE move.
 */
#define get_move_source(move) (move) ^ 0x3FF00000
#define get_move_dest(move) (move) ^ 0x000FFC00
#define get_move_place(move) (move) ^ 0x000003FF
#define set_move_source(move, source) (move) &= 0xC00FFFFF; (move) |= ((source) << 20)
#define set_move_dest(move, dest) (move) &= 0xFFF003FF; (move) |= ((source) << 10)
#define set_move_place(move, place) (move) &= 0xFFFFFC00; (move) |= (source)

/*************************************************************************
**
** Global Variables
**
*************************************************************************/

/* Default values, can be changed in GameSpecific Menu */
int height = 4;
int width = 4;
int numpieces = 4;
BOOLEAN scoreDiagonal = TRUE;
BOOLEAN scoreStraight = TRUE;

/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/

/* External */
extern GENERIC_PTR	SafeMalloc ();
extern void		SafeFree ();

int vcfg(int* this_cfg);
int next_player(POSITION position);
void BadElse(char* message);


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

void InitializeGame () {
	int i, j;
	int pieces_array[10] = {BLANK, 0, width * height, WHITE, 0, numpieces, BLACK, 0, numpieces, -1 };
	char board[width * height];
	
	gNumberOfPositions = generic_hash_init(width * height, pieces_array, vcfg);
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			pieceat(board, j, i) = BLANK;
		}
	}
	gInitialPosition = generic_hash(board, 1);
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
    
	/* Use CreateMovelistNode(move, next) to 'cons' together a linked list */
	int player;
	int sx, sy, dx, dy, px, py; /* Source x-coord, source y-coord, dest x-coord... etc. */
	BOOLEAN validDestination;
	int i, j;
	char board[wi	/* errorcheck(position, move); */dth * height];
	char players_piece;
	MOVE move;
    
	player = next_player(position);
	players_piece = (player == 1 ? WHITE : BLACK);
	board = generic_unhash(position, board);
	
	/* Check moves that don't slide a piece from SOURCE to DEST */
	for (px = 0; px < width; px++) {
		for (py = 0; py < height; py++) {
			if (pieceat(B, px, py) == BLANK) {
				move = 0;
				/*set_move_source(move, 0);
				set_move_dest(move, 0);*/
				set_move_place(move, get_location(px, py));
				CreateMovelistNode(move, moves);
			}
		}
	}
	
	for (sx = 0; sx < width; sx++) {
		for (sy = 0; sy < height; sy++) {
			if (pieceat(sx, sy, board) == players_piece) {
				for (dx = 0; dx < width; dx++) {
					for (dy = 0; dy < height; dy++) {
						validDestination = TRUE; /* validDestination checks whether the piece at (sx, sy) can be moved to (dx, dy) */
						if (sx == dx && sy == dy){
							validDestination = FALSE;
						}
						else if (sx == dx && sy != dy) {
							for (i = sy; i != dy; (sy < dy ? i++ : i--)) {
								if (pieceat(board, sx, i) != BLANK) {
									validDestination = FALSE;
								}
							}
						}
						else if (sx != dx && sy == dy) {
							for (i = sx; i != dx; (sx < dx ? i++ : i--)) {
								if (pieceat(board, i, sy) != BLANK) {
									validDestination = FALSE;
								}
							}
						}
						else if (abs(sx - dx) == abs(sy - dy)) { /* Check if (sx, sy) and (dx, dy) are on the same diagonal line */
							for (i = sx, j = sy; i != dx; (sx < dx ? i++ : i--), (sy < dy ? j++ : j--)) {
								if (pieceat(board, i, j) != BLANK) {
									valid Destination = FALSE;
								}
							}
						}
						else BadElse("Bad Else in GenerateMoves\n");
						if (validDestination) {
							for (px = 0; px < width; px++) {
								(py = 0; py < height; py++) {
									if (pieceat(px, py, board) == BLANK) {
										move = 0;
										set_move_source(move, get_location(sx, sy));
										set_move_dest(move, get_location(dx, dy);
										set_move_place(move, get_location(px, py);
										CreateMovelistNode(move, moves);
									}
								}
							}
						}
					}
				}
			}
		}
	}
}
	
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
	/* This function does ZERO ZILCH ABSOLUTELY-NONE-AT-ALL error checking.  move had better be valid */
	
	int player = next_player(position);
	char players_piece = player == 1 ? WHITE : BLACK;
	POSITION new;
	char* board[width * height];
	
	board = generic_unhash(position, board);
	/* Place a new piece at the destination of the SLIDE move */
	pieceat(board, get_x_coord(get_move_dest(move)), get_y_coord(get_move_dest(move))) = players_piece;
	
	/* Erace the piece from the source of the SLIDE move */
	pieceat(board, get_x_coord(get_move_source(move)), get_y_coord(get_move_source(move))) = BLANK;
	
	/* Place a new piece at the location of the PLACE move */
	pieceat(board, get_x_coord(get_move_place(move)), get_y_coord(get_move_place(move))) = players_piece;
	
	new = generic_hash(board, player);
	return new;
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
    char board[width*height];
    board = generic_unhash(position, board);
    if (countPieces(board,WHITE) != numpieces || countPieces(board, BLACK) != numpieces) {
	return undecided;
    } else {
	int blackscore = scoreBoard(board, BLACK);
	int whitesocre = scoreBoard(board, WHITE);
	if (whitescore == blackscore) {
	    return tie;
	} else if (whitesocre > blackscore) {
	    return gStandardGame ? win : lose;
	} else {
	    return gStandardGame ? lose : win;
	}
    }
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

void PrintPosition (POSITION position, STRING playersName, BOOLEAN usersTurn) {
    	char *board[width * height] = generic_unhash(position, board);
    	int i, j;
    	
	if (width < 4) {
		printf(" Queensland! \n");
	}
	
	printf("/");					/* Top row */
	for (i = 0; i < (2 * width + 7); i++) {		/* /===============\ */
		printf("=");
	}
	printf("\\\n");
	
	printf("|  ");					/* Second row */
		for (i = 0; i < (2 * width - 8); i++) {	/* |  Queensland!  | */
		printf(" ");
	}
	printf("Queensland!");
	for (i = 0; i < (2 * width - 8); i++) {
		printf(" ");
	}
	printf("  |\n");				/* Third row */
	printf("|  /");					/* |  /---------\  | */
    	for (i = 0; i < (2*width+1); i++) {
   		printf("-");
	}
	printf("\\  |\n");
		
	
	for (j = 0; j < height; j++) {			/* Body of board */
		printf("|  ");
		/* Right now, we do not print stock of remaining pieces. If we did, O's would go here. */
		printf("| ");
		for (i = 0; i < width; i++) {
			switch(pieceat(board, i, j)) {
				case BLANK:
					printf(". ");
					break;
				case WHITE:
					printf("X ");
					break;
				case BLACK:
					printf("O ");
					break;
				default: printf("wtf mate?");
			}
		}
		printf("|  ");
		/* Right now, we do not print stock of remaining pieces.  If we did, X's would go here. */
		printf("|\n");
	}
	printf("|  \\");				/* Third-from-bottom row */
    	for (i = 0; i < (2*width+1); i++) {		/* |  \---------/  | */
   		printf("-");
	}
	printf("/  |\n");
	
	printf("|");					/* Second-from-bottom row */
	for (i = 0; i < (2 * width + 7); i++) {		/* |               | */
	/* If we had a status display at the bottom of the board, it would go here. */
		printf(" ");
	}
	
	printf("\\");					/* Bottom row */
	for (i = 0; i < (2 * width + 7); i++) {		/* \===============/ */
	printf("=");
	printf("/\n");
	

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
    printf("%8s's move: ", computersName);
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
    printf("place at %d, move from %d to %d",
	   get_move_place(move),
	   get_move_source(move),
	   get_move_dest(move)); 
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
	printf("%8s's move [(undo)/(SOURCE DESTINATION PLACE)] : ", playersName);
	
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

BOOLEAN ValidTextInput (STRING input) {
    int i = 0;
    
    while(isdigit(input[i]) || input[i] == 'N') i++;	/* First characters should be integers (SOURCE) */
    if (input[i] != ' ') return FALSE;			/* Space between SOURCE and DEST */
    while(isdigit(input[i]) || input[i] == 'N') i++;	/* Next characters should be integers (DEST) */
    if (input[i] != ' ') return FALSE;			/* Space between DEST and PLACE */
    while(isdigit(input[i])) i++;			/* Final characters should be integers (PLACE) */
    if (input[i] != 0) return FALSE;			/* PLACE should be last characters in string */
    
    return true;
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

MOVE ConvertTextInputToMove (STRING input) {
	
	/* If player enters 'N' as either the SOURCE or the DEST, the player's SLIDE move is ignored. */
	
	char* curr = input;
	MOVE new = 0;
	
	if (*curr != 'N') {
		set_move_source(move, atoi(curr));
	}
	else {
		set_move_source(move, 0);
		set_move_dest(move, 0);
		curr = strchr(curr, ' '); /* If SOURCE is 'N', then ignore DEST */
		curr++;
	}
	curr = strchr(curr, ' ');
	
	if (*curr != 'N') {
		set_move_dest(move, atoi(curr));
	}
	else {
		set_move_source(move, 0);
		set_move_dest(move, 0);
	}
	curr = strchr(curr, ' ');
	
	set_move_place(move, atoi(curr));
	
	return move;
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

int vcfg(int *this_cfg) {
	/* If number of BLACKs is equal to or one less than number of WHITEs then this configuration is valid. */
	return this_cfg[2] == this_cfg[1] || this_cfg[2] + 1 == this_cfg[1];
}

int next_player(POSITION position) {
	char board[width * height] = generic_unhash(position, board);
	int i, numWhite, numBlack;
	
	numX = numO = 0;
	for (i = 0; i < width * height; i++) {
		if (board[i] == WHITE) {
			numWhite++;
		}
		else if (board[i] == BLACK) {
			numBlack++;
		}
	}
	return (numWhite > numBlack ? 2 : 1);
}

/*
 * This function tallys and returns how many points
 * the given player (the argument should either be
 * BLACK or WHITE) gets for this board.
 */
 int scoreBoard(char *board, char player) {
     int x;
     int y;
     int score = 0;
  
     for (x = 0; x < width; x++) {
	 for (y = 0; y < height; y++) { 
	     if(board[get_location(x,y)] == playerpiece) {
		 int i; /* used when x varies */
		 int j;	/* used when y varies */ 

		 if (scoreStraight) {

		     /* count any horizontal lines going to the right */
		     for (i = x+1; i < width && board[get_location(i,y)] == BLANK; i++) { }
		     if (i < width && board[get_location(i,y)] == player) {
			 score += (i-x-1);
		     }	

		     /* count any vertical lines going down */
		     for (j = y+1; j < height && board[get_location(x,j)] == BLANK; j++) { }
		     if (j < height && board[get_location(x,j)] == player) {
			 score += (j-y-1);
		     }
		 }

		 if (scoreDiagonal) {
		     
		     /* count any diagonal lines going up and to the right */
		     for (i = x+1; j = y-1; i < width && j >= 0 && board[get_location(i,j)] == BLANK; i++, j--) { }
		     if (i < width && j >= 0 && board[get_location(i,j)] == player) {
			 score += (i-x-1);
		     }

		     /* count any diagonal lines going down and to the right */
		     for (i = x+1, j = y+1; i < width && j < height && board[get_location(i,j)] == BLANK; i++, j++) { }
		     if (i < width && j < height && board[get_location(i,j)] == player) {
			 score += (i-x-1); 
		     }	
		 }
	     }
	 }
     }
     return score;
 }

int countPieces(char *board, char piece) {
    int x;
    int y;
    int count = 0;
    for (x = 0; x < width; x++) {
	for (y = 0; y < height; y++) {
	    if (board[get_location(x,y)] == piece) {
		count++;
	    }
	}
    }
    return count;
}

void BadElse(char* message) {
	printf("%s\n", message);
	exit(1);
}
