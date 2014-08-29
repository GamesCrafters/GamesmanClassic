/************************************************************************
**
** NAME:        mago.c
**
** DESCRIPTION: Atari Go
**
** AUTHOR:      Ofer Sadgat
**
** DATE:        2006-11-10
**
** UPDATE HIST: -2006.11.10 = First version, includes most functionality.
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

STRING kGameName            = "Atari Go";   /* The name of your game */
STRING kAuthorName          = "Ofer Sadgat";   /* Your name(s) */
STRING kDBName              = "mago";   /* The name to store the database under */

BOOLEAN kPartizan            = TRUE;   /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN kGameSpecificMenu    = TRUE;   /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN kTieIsPossible       = FALSE;   /* TRUE if a tie is possible. FALSE if it is impossible.*/
BOOLEAN kLoopy               = FALSE;  /* TRUE if the game tree will have cycles (a rearranger style game). FALSE if it does not.*/

BOOLEAN kDebugMenu           = FALSE;   /* TRUE only when debugging. FALSE when on release. */
BOOLEAN kDebugDetermineValue = FALSE;   /* TRUE only when debugging. FALSE when on release. */

POSITION gNumberOfPositions   =  0; /* The number of total possible positions | If you are using our hash, this is given by the hash_init() function*/
POSITION gInitialPosition     =  0; /* The initial hashed position for your starting board */
POSITION kBadPosition         = -1; /* A position that will never be used */

void*    gGameSpecificTclInit = NULL;

/**
 * Help strings that are pretty self-explanatory
 * Strings than span more than one line should have backslashes (\) at the end of the line.
 * These help strings should be updated and dynamically changed using
 * InitializeHelpStrings()
 **/

STRING kHelpGraphicInterface =
        "Help strings not initialized!";

STRING kHelpTextInterface =
        "Help strings not initialized!";

STRING kHelpOnYourTurn =
        "Help strings not initialized!";

STRING kHelpStandardObjective =
        "Help strings not initialized!";

STRING kHelpReverseObjective =
        "Help strings not initialized!";

STRING kHelpTieOccursWhen =
        "Help strings not initialized!";

STRING kHelpExample =
        "Help strings not initialized!";



/*************************************************************************
**
** #defines and structs
**
**************************************************************************/
#define WIDTH_MAX       19
#define LENGTH_MAX      19
#define WIDTH_MIN       1
#define LENGTH_MIN      1

#define WHITE           'o'
#define BLACK           'X'
#define SPACE           ' '
#define PLAYER_ONE  1
#define PLAYER_TWO  2

struct linked_list {
	int x, y;
	struct linked_list *next;
};

/*************************************************************************
**
** Global Variables
**
*************************************************************************/
int width               = 0;
int length              = 0;
int boardsize   = 0;

BOOLEAN set             = FALSE;

/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/
//void InitializeHelpStrings();
void Reset();
void SetupGame();
POSITION hash (char*, int);
char* unhash (POSITION, int*);
int toIndex (int, int);
int legalCoords (int, int);
void countPieces (char*, int*, int*);
void ChangeBoardSize();

TIER BoardToTier(char* board);
void SetupTierStuff();
STRING TierToString(TIER tier);
TIERLIST* TierChildren(TIER tier);
TIERPOSITION NumberOfTierPositions(TIER tier);
/* External */
#ifndef MEMWATCH
extern GENERIC_PTR SafeMalloc ();
extern void     SafeFree ();
#endif

STRING MoveToString(MOVE move);

void listFree(struct linked_list **lst);
struct linked_list* listAdd(struct linked_list *lst, int x,  int y);
int listMember(struct linked_list *lst, int x, int y);
int evaluateLiberties(char *board, int x, int y, struct linked_list *waiting, struct linked_list *evaled, struct linked_list *nolibs, char col, int turn);
struct linked_list* listRemove(struct linked_list *lst, int x,  int y);
/************************************************************************
**
** NAME:        InitializeGame
**
** DESCRIPTION: Prepares the game for execution.
**              Initializes required variables.
**
************************************************************************/

void InitializeGame (){
	if(!set) {
		Reset();
		set = TRUE;
	}
	//InitializeHelpStrings();
	SetupGame();

	gMoveToStringFunPtr = &MoveToString;
}


/************************************************************************
**
** NAME:        InitializeHelpStrings
**
** DESCRIPTION: Sets up the help strings based on chosen game options.
**
** NOTES:       Should be called whenever the game options are changed.
**              (e.g., InitializeGame() and GameSpecificMenu())
**
************************************************************************/
void InitializeHelpStrings ()
{
	kHelpGraphicInterface =
	        "There is no graphic interface.";

	kHelpTextInterface =
	        "";

	kHelpOnYourTurn =
	        "Place a piece into an empty space.";

	kHelpStandardObjective =
	        "To surround an opponents stone or group of stones completely.";

	kHelpReverseObjective =
	        "Have one or more of your own stones be completely surrounded.";

	kHelpTieOccursWhen =
	        "A tie can never happen.";

	kHelpExample =
	        "3   - o -   \n2 o - X - o \n1   - o -   \n\nHere X is completely surrounded.";

	gMoveToStringFunPtr = &MoveToString;

}


/************************************************************************
**
** NAME:        GenerateMoves
**
** DESCRIPTION: Creates a linked list of every move that can be reached
**              from this position. Returns a pointer to the head of the
**              linked list.
**
** INPUTS:      POSITION position : Current position to generate moves
**
** OUTPUTS:     (MOVELIST *)      : A pointer to the first item of
**                                  the linked list of generated moves
**
** CALLS:       MOVELIST *CreateMovelistNode();
**
************************************************************************/

MOVELIST *GenerateMoves (POSITION position){
	MOVELIST* moves = NULL;
	char* board;
	int turn, x, y;
	board = unhash(position, &turn);
	for (y = 1; y <= length; y++) { // look through all the rows, bottom-up
		for (x = 1; x <= width; x++) { // look through the columns, left-to-right
			if (board[toIndex(x,y)] == SPACE) {
				moves = CreateMovelistNode((x*100) + y, moves);
			}
		}
	}
	if(board != NULL)
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

POSITION DoMove (POSITION position, MOVE move){
	/* MOVE = two digits, are just coords. So a4 is 14. */
	char* board;
	int turn, x1, y1;
	//unhash the move
	x1 = move/100;
	y1 = move % 100;
	// now make the board
	board = unhash(position, &turn);
	char myPiece = (turn==PLAYER_ONE ? BLACK : WHITE);
	board[toIndex(x1,y1)] = myPiece;
	return hash(board, (turn==PLAYER_ONE ? PLAYER_TWO : PLAYER_ONE));
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

VALUE Primitive (POSITION position){
	char* board;
	int turn; //, reds, blues;
	//BOOLEAN redWon;
	board = unhash(position, &turn);
	int x;
	int y;

	BOOLEAN SELFKILL = FALSE;

	// These lists reduce the time down from theta(N^2) to theta(N)

	struct linked_list *whave_liberties = (struct linked_list *) SafeMalloc(sizeof(struct linked_list));
	(*whave_liberties).x = -2;
	(*whave_liberties).y = -2;
	(*whave_liberties).next = NULL;

	struct linked_list *wno_liberties = (struct linked_list *) SafeMalloc(sizeof(struct linked_list));
	(*wno_liberties).x = -2;
	(*wno_liberties).y = -2;
	(*wno_liberties).next = NULL;

	struct linked_list *bhave_liberties = (struct linked_list *) SafeMalloc(sizeof(struct linked_list));
	(*bhave_liberties).x = -2;
	(*bhave_liberties).y = -2;
	(*bhave_liberties).next = NULL;

	struct linked_list *bno_liberties = (struct linked_list *) SafeMalloc(sizeof(struct linked_list));
	(*bno_liberties).x = -2;
	(*bno_liberties).y = -2;
	(*bno_liberties).next = NULL;

	for (y = 1; y <= length; y++) { // look through all the rows, bottom-up
		for (x = 1; x <= width; x++) { // look through the columns, left-to-right
			int result;
			if (board[toIndex(x,y)] == BLACK) {
				result = evaluateLiberties(board, x, y, NULL, bhave_liberties, bno_liberties, board[toIndex(x,y)],turn);
			} else {
				result = evaluateLiberties(board, x, y, NULL, whave_liberties, wno_liberties, board[toIndex(x,y)],turn);
			}
			struct linked_list *tmp_liberties = bhave_liberties;
			int ct = 0;
			while (tmp_liberties != NULL) {
				ct = ct + 1;
				tmp_liberties = (*tmp_liberties).next;
			}

			if (result == FALSE) {
				if (((turn == PLAYER_ONE) && (board[toIndex(x,y)] == WHITE))
				    || ((turn == PLAYER_TWO) && (board[toIndex(x,y)] == BLACK))) {
					SELFKILL = TRUE;
				} else {
					listFree(&bhave_liberties);
					listFree(&bno_liberties);
					listFree(&whave_liberties);
					listFree(&wno_liberties);
					return (gStandardGame ? lose : win);
				}
			}
		}
	}
	listFree(&bhave_liberties);
	listFree(&bno_liberties);
	listFree(&whave_liberties);
	listFree(&wno_liberties);
	if (SELFKILL == TRUE) {
		return (gStandardGame ? win : lose);
	}
	return undecided;
}

int UNDECIDED = -1;

int evaluateLiberties(char *board, int x, int y, struct linked_list *waiting, struct linked_list *evaled, struct linked_list *nolibs, char col, int turn){
	BOOLEAN first = (waiting == NULL);
	if ((x > width) || (y > length) || (x <= 0) || (y <= 0)) {
		return UNDECIDED;
	} else if (board[toIndex(x,y)] == SPACE) {
		return TRUE;
	} else if (board[toIndex(x,y)] != col) {
		return FALSE;
	} else if (listMember(waiting, x, y) == TRUE) {
		return UNDECIDED;
	} else if (listMember(evaled, x, y) == TRUE) {
		return TRUE;
	} else if (listMember(nolibs, x, y) == TRUE) {
		return FALSE;
	} else {
		waiting = listAdd(waiting, x, y);
		if (evaluateLiberties(board, x+1, y, waiting, evaled, nolibs, col, turn) == TRUE
		    || evaluateLiberties(board, x, y+1, waiting, evaled, nolibs, col, turn) == TRUE
		    || evaluateLiberties(board, x, y-1, waiting, evaled, nolibs, col, turn) == TRUE
		    || evaluateLiberties(board, x-1, y, waiting, evaled, nolibs, col, turn) == TRUE) {
			listAdd(evaled, x, y);
			waiting = listRemove(waiting, x, y);
			return TRUE;
		} else {
			if (first == TRUE) {
				while ((*waiting).next != NULL) {
					listAdd(nolibs, (*(*waiting).next).x, (*(*waiting).next).y);
					(*waiting).next = listRemove((*waiting).next, (*(*waiting).next).x, (*(*waiting).next).y);
				}
				listAdd(nolibs, (*waiting).x, (*waiting).y);
				SafeFree(waiting);
				return FALSE;
			} else {
				return UNDECIDED;
			}
		}
	}
}

int listMember(struct linked_list *lst, int x, int y){
	if (lst == NULL) {
		return FALSE;
	}
	if ((*lst).x == x && (*lst).y == y) {
		return TRUE;
	} else {
		return listMember((*lst).next, x, y);
	}
}

struct linked_list* listAdd(struct linked_list *lst, int x,  int y){
	struct linked_list *val = (struct linked_list *) SafeMalloc(sizeof(struct linked_list));
	(*val).x = x;
	(*val).y = y;
	if (lst == NULL) {
		(*val).next = NULL;
		return val;
	}
	(*val).next = (*lst).next;
	(*lst).next = val;
	return lst;
}

struct linked_list* listRemove(struct linked_list *lst, int x,  int y){
	struct linked_list *last = NULL;
	while (lst != NULL) {
		if ((*lst).x == x && (*lst).y == y) {
			if (last == NULL) {
				SafeFree(lst);
				return NULL;
			}
			(*last).next = (*lst).next;
			SafeFree(lst);
			return lst;
		} else {
			last = lst;
			lst = (*lst).next;
		}
	}
	return lst;
}

void listFree(struct linked_list **lst){
	if (lst == NULL) {
		return;
	} else if ((*lst) == NULL) {
		return;
	} else {
		struct linked_list *list = (*lst);
		struct linked_list *next;
		while (list != NULL) {
			next = (*list).next;
			SafeFree(list);
			list = next;
		}
		(*lst) = NULL;
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

void PrintPosition (POSITION position, STRING playersName, BOOLEAN usersTurn){
	char* board;
	int turn, x, y, reds, blues;
	board = unhash(position, &turn);
	countPieces(board, &reds, &blues);
	printf("\t%s's Turn (%s):\n  ",playersName,(turn==PLAYER_ONE ? "BLACK" : "WHITE"));
	printf("%s\n", GetPrediction(position, playersName, usersTurn));
	for (y = length; y >= 0; y--) { // for all the rows
		if (y == 0) {
			printf("    ");
			for (x = 0; x < width; x = x + 1) {
				printf("%c   ", 'a' + x);
			}
		} else {
			printf(" %2d ", y); //print row number
			for (x = 1; x <= width; x++) {
				if (x+1 <= width) {
					printf("%c - ",board[toIndex(x,y)]);
				} else {
					printf("%c",board[toIndex(x,y)]);
				}
			}
			printf("\n");
			if (y > 1) {
				printf("    ");
				for (x = 0; x < width; x++) {
					printf("|   ");
				}
				printf("\n");
			}
		}
	}
	printf("\n\n");
	if(board != NULL)
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

void PrintComputersMove (MOVE computersMove, STRING computersName){
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

void PrintMove (MOVE move){
	STRING str = MoveToString(move);
	printf("%s", str);
	SafeFree(str);
}


/************************************************************************
**
** NAME:        MoveToString
**
** DESCRIPTION: Returns the move as a STRING
**
** INPUTS:      MOVE *move         : The move to put into a string.
**
************************************************************************/

STRING MoveToString (MOVE move){
	STRING moveStr = (STRING) SafeMalloc(sizeof(char)*3);
	moveStr[0] = (move/100)+'a'-1;
	moveStr[1] = (move % 100) + '0';
	moveStr[2] = '\0';
	return moveStr;
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

USERINPUT GetAndPrintPlayersMove (POSITION position, MOVE *move, STRING playersName){

	USERINPUT input;
	USERINPUT HandleDefaultTextInput();

	for (;; ) {
		/***********************************************************
		* CHANGE THE LINE BELOW TO MATCH YOUR MOVE FORMAT
		***********************************************************/

		printf("%8s's move [(undo)/([%c-%c][%d-%d])] : ", playersName,'a', width+'a', 1, length);
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

BOOLEAN ValidTextInput (STRING input){
	if(strlen(input) < 2 || strlen(input) > 3)
		return FALSE;
	if (!isalpha(input[0]) || !isdigit(input[1]))
		return FALSE;
	if (strlen(input) == 3 && !isdigit(input[2]))
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

MOVE ConvertTextInputToMove (STRING input){
	return (100*(input[0]-'a'+1) + input[1]-'0');
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

void GameSpecificMenu (){
	char c;
	BOOLEAN cont = TRUE;
	while(cont) {
		printf("\n\nCurrent %dx%d board:  \n", width, length);
		PrintPosition(gInitialPosition, "Gamesman", 0);
		printf("\tGame Options:\n\n"
		       "\tc)\t(C)hange the board size (nxn), currently: %dx%d\n"
		       //"\ti)\tSet the (I)nitial position\n"
		       "\tr)\t(R)eset to default settings\n"
		       "\tb)\t(B)ack to the main menu\n"
		       "\nSelect an option:  ", width, length);
		c = GetMyChar();
		switch(c) {
		case 'c': case 'C':
			ChangeBoardSize();
			break;
		case 'i': case 'I':
			GetInitialPosition();
			break;
		case 'r': case 'R':
			Reset();
			SetupGame();
			break;
		case 'b': case 'B':
			cont = FALSE;
			break;
		default:
			printf("Invalid option!\n");
		}
	}
	//InitializeHelpStrings();
}


/************************************************************************
**
** NAME:        SetTclCGameSpecificOptions
**
** DESCRIPTION: Set the C game-specific options (called from Tcl)
**              Ignore if you don't care about Tcl for now.
**
************************************************************************/

void SetTclCGameSpecificOptions (int options[]){

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

POSITION GetInitialPosition (){
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

int NumberOfOptions (){
	return 19*19*2;
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

int getOption (){
	/* If you have implemented symmetries you should
	   include the boolean variable gSymmetries in your
	   hash */
	if (gStandardGame == TRUE) {
		return width*100 + length;
	} else {
		return (width*100 + length)* -1;
	}
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

void setOption (int option){
	/* If you have implemented symmetries you should
	   include the boolean variable gSymmetries in your
	   hash */
	if (option < 0) {
		gStandardGame = FALSE;
		option = option * -1;
	}
	width = option / 100;
	length = option % 100;
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

void DebugMenu (){

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

// This counts the number of pieces on the board.
void countPieces (char* board, int* blacks, int* whites) {
	int i, black = 0, white = 0;
	for (i = 0; i < boardsize; i++) { //count the pieces on the board
		if (board[i] == BLACK) black++;
		else if (board[i] == WHITE) white++;
	}
	(*blacks) = black;
	(*whites) = white;
}

/*
   The BOARD the player sees is as follows:
   3 + - + - + - +
 |   |   |   |
   2 + - + - + - +
 |   |   |   |
   1 + - + - + - +
   a   b   c   d

   length = 3, width = 4, boardsize = 12.

   So on the array, it's like this:
   { a3, b3, c3, d3, a2, b2, c2, d2, a1, b1, c1, d1 }
   0   1   2   3   4   5   6   7   8   9   10  11

   For the game, letters are translated to numbers (a = 1, b = 2, etc.)
   So (1,1) is the bottom left, (4,3) is the top right.

   toIndex takes in the coord and produces its index.
   So toIndex(1,1) = 8, toIndex(4,3) = 3.
 */

//a precursor to toIndex = checks if x and y are legal coords
int legalCoords (int x, int y) {
	return ((x > 0 && x <= width) && (y > 0 && y <= length));
}

//goes from 2D coords to 1D array
int toIndex (int x, int y) {
	return (length-y)*width + x-1;
}

void ChangeBoardSize (){
	int newWidth, newLength;
	while (TRUE) {
		printf("\n\nCurrent board of size %dx%d:\n\n", width, length);
		PrintPosition(gInitialPosition, "Gamesman", 0);
		printf("\n\nEnter the new width (%d - %d):  ", WIDTH_MIN, WIDTH_MAX);
		newWidth = GetMyChar()-48;
		if(newWidth > WIDTH_MAX || newWidth < WIDTH_MIN) {
			printf("\nInvalid width!\n");
			continue;
		}
		printf("\n\nEnter the new length (%d - %d):  ", LENGTH_MIN, LENGTH_MAX);
		newLength = GetMyChar()-48;
		if(newLength > LENGTH_MAX || newLength < LENGTH_MIN) {
			printf("\nInvalid length!\n");
			continue;
		}
		width = newWidth;
		length = newLength;
		boardsize = width*length;
		SetupGame();
		break;
	}
}

void Reset() {
	length = 3;
	width = 3;
	boardsize = length*width;
}

void SetupGame() {
	generic_hash_destroy();
	SetupTierStuff();
	if (width < 5 && length < 5) {
		int pieces_array[10] = {BLACK, 0, (boardsize+1)/2, WHITE, 0, boardsize/2, SPACE, 0, boardsize, -1};
		gNumberOfPositions = generic_hash_init(boardsize, pieces_array, NULL, 0);
		// initial position
		int i;
		char* initial = (char *) SafeMalloc(boardsize * sizeof(char));
		for(i = 0; i < boardsize; i++)
			initial[i] = SPACE;
		gInitialPosition = hash(initial, 1);
	}
}

char* unhash (POSITION position, int* turn){
	char* board = (char *) SafeMalloc(boardsize * sizeof(char));
	if (gHashWindowInitialized) {
		TIERPOSITION tierPos; TIER tier;
		gUnhashToTierPosition(position, &tierPos, &tier);
		generic_hash_context_switch(tier);
		(*turn) = generic_hash_turn(tierPos);
		board = (char *) generic_hash_unhash(tierPos, board);
	} else {
		(*turn) = generic_hash_turn(position);
		board = (char *) generic_hash_unhash(position, board);
	}
	return board;
}

POSITION hash (char* board, int turn){
	POSITION position;
	if (gHashWindowInitialized) {
		TIER tier = BoardToTier(board);
		generic_hash_context_switch(tier);
		TIERPOSITION tierPos = generic_hash_hash(board, turn);
		position = gHashToWindowPosition(tierPos, tier);
	} else position = generic_hash_hash(board, turn);

	if(board != NULL)
		SafeFree(board);
	return position;
}

/* TIERS: Number of pieces left to place. Alternately, number of spaces
   on the board.
   That gives tiers solvable from 0 to boardsize.
   Pieces on board = boardsize-tier
 */

TIER BoardToTier(char* board) {
	int blacks, whites;
	countPieces(board, &blacks, &whites);
	int totalPieces = blacks+whites;
	return boardsize-totalPieces;
}

void SetupTierStuff() {
	// kSupportsTierGamesman
	kSupportsTierGamesman = TRUE;
	// function pointers
	gTierChildrenFunPtr = &TierChildren;
	gNumberOfTierPositionsFunPtr = &NumberOfTierPositions;
	gTierToStringFunPtr = &TierToString;
	// hashes
	// Tier-Specific Hashes
	int piecesArray[10] = { BLACK, 0, 0, WHITE, 0, 0, SPACE, 0, 0, -1 };
	int blackPiecesOnBoard, whitePiecesOnBoard, tier;
	for (tier = 0; tier <= boardsize; tier++) {
		blackPiecesOnBoard = (boardsize - tier + 1) / 2;
		whitePiecesOnBoard = (boardsize - tier) / 2;
		// Reds AND Blues = from 0 to piecesOnBoard
		piecesArray[1] = blackPiecesOnBoard;
		piecesArray[2] = blackPiecesOnBoard;
		piecesArray[4] = whitePiecesOnBoard;
		piecesArray[5] = whitePiecesOnBoard;
		// Blanks = tier
		piecesArray[7] = piecesArray[8] = tier;
		// make the hashes
		generic_hash_init(boardsize, piecesArray, NULL, 0);
	}
	// Initial
	int i;
	char* initial = (char *) SafeMalloc(boardsize * sizeof(char));
	for(i = 0; i < boardsize; i++)
		initial[i] = SPACE;
	gInitialTier = boardsize;
	generic_hash_context_switch(gInitialTier);
	gInitialTierPosition = hash(initial, 1);
}

// children = always me and one below
TIERLIST* TierChildren(TIER tier) {
	TIERLIST* list = NULL;
	if (tier != 0)
		list = CreateTierlistNode(tier-1, list);
	return list;
}

TIERPOSITION NumberOfTierPositions(TIER tier) {
	generic_hash_context_switch(tier);
	return generic_hash_max_pos();
}

// Tier = Number of pieces left to place.
STRING TierToString(TIER tier) {
	// made buffer size a little larger
	// since unsigned long long can be up to 20 digits
	STRING tierStr = (STRING) SafeMalloc(sizeof(char)*64);
	// boardsize is int, tier is (TIER) unsigned long long
	// need %llu due to type promotion
	// %d would lose bits
	sprintf(tierStr, "%llu Pieces Placed", boardsize-tier);
	return tierStr;
}
POSITION StringToPosition(char* board) {
	POSITION pos = 0;
	if (GetValue(board, "pos", GetUnsignedLongLong, &pos)) {
		return pos;
	} else {
		return INVALID_POSITION;
	}
}


char* PositionToString(POSITION pos) {
	int turn;
    char *board = unhash(pos, &turn);
	char *out = MakeBoardString(board,
							    "turn", StrFromI(turn),
								"pos", StrFromI(pos),
								"");
	SafeFree(board);
	return out;
}

char * PositionToEndData(POSITION pos) {
	return NULL;
}
