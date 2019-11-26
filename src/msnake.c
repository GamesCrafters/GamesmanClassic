/************************************************************************
**
** NAME:        msnake.c
**
** DESCRIPTION: Snake
**
** AUTHOR:      Alice Chang, Judy Chen, Eleen Chiang, Peter Foo
**              University of California at Berkeley
**              Copyright (C) 2003. All rights reserved.
**
** DATE:        02/25/03
**
** UPDATE HIST:
**
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

/* The beginning position is:
**
**   -   -   -   -
**
**   -   H---*   -
**           |
**   -   -   T   -
**
**   -   -   -   -
**  which corresponds to 1*2^13 + 1*2^7 + 1*2^5 + 1*2^2 + 1*2^0 = 8357
**  since body = 1, blank = 0, and there are 22 bits.
**
** This is the player's legend-> 1   2   3   4
**
**                               5   6   7   8
**
**                               9   10  11  12
**
**                               13  14  15  16
**
** This is the programmer's legend for initial board-> 1   2   3   4
**
**                                                     5   *   6   7
**
**                                                     8   9   *   10
**
**                                                     11  12  13  14
**
** Note that the programmer's legend will change each time a move
** is made.
*/

POSITION gNumberOfPositions  = 4194304;  /* 2^14 times 256 */

POSITION gInitialPosition    = 8357;
POSITION gMinimalPosition    = 8357; /* This may not be the minimal position. Use the upward solver when it's invented */
POSITION kBadPosition        = -1; /* This can never be the rep. of a position */

STRING kAuthorName         = "Alice Chang, Judy Chen, Eleen Chiang, and Peter Foo";
STRING kGameName           = "Snake";
STRING kDBName             = "snake";
BOOLEAN kPartizan           = TRUE;  /* each player only has control
                                      * over one type of piece, so
                                      * both players do not have the
                                      * exact same moves */
BOOLEAN kSupportsHeuristic  = FALSE;
BOOLEAN kSupportsSymmetries = FALSE;
BOOLEAN kSupportsGraphics   = TRUE;
BOOLEAN kDebugMenu          = FALSE;
BOOLEAN kGameSpecificMenu   = TRUE;
BOOLEAN kTieIsPossible      = FALSE;
BOOLEAN kLoopy               = FALSE;
BOOLEAN kDebugDetermineValue = FALSE;
void*    gGameSpecificTclInit = NULL;

STRING kHelpGraphicInterface =
        "Not written yet";

STRING kHelpTextInterface    =
        "On your turn, use the LEGEND to determine which numbers to choose (between\n1 and 16, with 1 at the upper left and 16 at the lower right) to correspond\nto the board position that you desire to move from and the empty board position\n you desire to move to and hit return. If at any point you have made a mistake\nyou can type u and hit return and the system will revert back to your most recent\nposition.";

STRING kHelpOnYourTurn =
        "You move your piece to one of the empty board positions.";

STRING kHelpStandardObjective =
        "To trap your opponent so that he/she is no longer able to make a valid move.";

STRING kHelpReverseObjective =
        "To trap yourself so that you are no longer able to make a valid move.";

STRING kHelpTieOccursWhen =   /* Should follow 'A Tie occurs when... */
                            "A tie is not possible in this game.";

STRING kHelpExample =
        "          1   2   3   4             +   +   +   +\nLEGEND:   5   6   7   8  TOTAL:     +   H   B   + \n          9   10  11  12            +   +   T   +\n          13  14  15  16            +   +   +   + \n\
Head's turn.\n     Dan's move [(u)ndo/1-9] : 6 10\n          1   2   3   4             +   +   +   + \n\
LEGEND:   5   6   7   8  TOTAL:     +   B   B   + \n          9   10  11  12            +   H   T   + \n          13  14  15  16            +   +   +   + \nTail's turn.\nComputer's move              : 11 15\n          1   2   3   4             +   +   +   + \nLEGEND:   5   6   7   8  TOTAL:     +   B   B   + \n          9   10  11  12            +   H   B   + \n          13  14  15  16            +   +   T   +\n\
Head's turn.\n     Dan's move [(u)ndo/1-9] : 10 14\n          1   2   3   4             +   +   +   + \nLEGEND:   5   6   7   8  TOTAL:     +   B   B   + \n          9   10  11  12            +   B   B   + \n          13  14  15  16            +   H   T   +\nTail's turn.\nComputer's move              : 15 16\n          1   2   3   4             +   +   +   + \nLEGEND:   5   6   7   8  TOTAL:     +   B   B   + \n          9   10  11  12            +   B   B   + \n          13  14  15  16            +   H   B   T \nHead's turn. \n     Dan's move [(u)ndo/1-9] : 14 13 \n          1   2   3   4             +   +   +   +  \nLEGEND:   5   6   7   8  TOTAL:     +   B   B   +  \n          9   10  11  12            +   B   B   +  \n          13  14  15  16            H   B   B   T  \n\
Tail's turn.   \nComputer's move              : 16 12\n          1   2   3   4             +   +   +   + \nLEGEND:   5   6   7   8  TOTAL:     +   B   B   + \n          9   10  11  12            +   B   B   T \n          13  14  15  16            H   B   B   B\nHead's turn.\n     Dan's move [(u)ndo/1-9] : 13 9\n          1   2   3   4             +   +   +   + \nLEGEND:   5   6   7   8  TOTAL:     +   B   B   + \n          9   10  11  12            H   B   B   T \n          13  14  15  16            B   B   B   B \nTail's turn.\nComputer's move              : 12 8\n          1   2   3   4             +   +   +   + \nLEGEND:   5   6   7   8  TOTAL:     +   B   B   T \n          9   10  11  12            H   B   B   B \n          13  14  15  16            B   B   B   B \nHead's turn.\n     Dan's move [(u)ndo/1-9] : 9 5\n          1   2   3   4             +   +   +   + \nLEGEND:   5   6   7   8  TOTAL:     H   B   B   T \n          9   10  11  12            B   B   B   B \n          13  14  15  16            B   B   B   B\n\
Tail's turn.\nComputer's move              : 8 4\n          1   2   3   4             +   +   +   T \nLEGEND:   5   6   7   8  TOTAL:     H   B   B   B \n          9   10  11  12            B   B   B   B \n          13  14  15  16            B   B   B   B \nHead's turn. \n    Dan's move [(u)ndo/1-9] : 5 1\n          1   2   3   4             H   +   +   T \nLEGEND:   5   6   7   8  TOTAL:     B   B   B   B \n          9   10  11  12            B   B   B   B \n          13  14  15  16            B   B   B   B \nTail's turn. \nComputer's move              : 4 3 \n          1   2   3   4             H   +   T   B  \nLEGEND:   5   6   7   8  TOTAL:     B   B   B   B  \n          9   10  11  12            B   B   B   B  \n          13  14  15  16            B   B   B   B   \n\
Head's turn. \n     Dan's move [(u)ndo/1-9] : 1 2 \n          1   2   3   4             B   H   T   B  \nLEGEND:   5   6   7   8  TOTAL:     B   B   B   B  \n          9   10  11  12            B   B   B   B  \n          13  14  15  16            B   B   B   B   \nTail's turn. \nExcellent! You won!"                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    ;


/*************************************************************************
**
* Everything above here must be in every game file
**
**************************************************************************/

/*************************************************************************
**
** Every variable declared here is only used in this file (game-specific)
**
**************************************************************************/

#define BOARDSIZE        16           /* 4x4 board */

#define BOARDWIDTH       4            /* board length 4 */

#define BADSLOT          -2           /* slot does not exist */

typedef enum possibleSlots {
	Blank, b, h, t
} BlankBHT;

typedef int SLOT;     /* A slot is the place where a piece moves from or to */

// The following struct will be used for constructing the body of the snake.
struct node {
	int position;
	BlankBHT piece;
	int depth;
	struct node *left;
	struct node *down;
	struct node *right;
	struct node *up;
	struct node *parent;
};

char *gBlankBHTString[] = {"+", "B", "H", "T", "|", "-", " "};

/* Powers of 2 for the body of the snake, so starts from the 2^8 */
int g2Body[] = { 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072,
	         262144, 524288, 1048576, 2097152 };
/* Variants */
BlankBHT moveHandT = FALSE;  /* Giving the player the option to move
                              * either Head or Tail. */

BlankBHT whoseTurn(BlankBHT *theBlankBHT);
void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn);
void PrintMove(MOVE theMove);
void SnakeUnhash(POSITION thePos, BlankBHT *theBlankBHT);
void MoveToSlots(MOVE theMove, SLOT *fromSlot, MOVE *toSlot);
POSITION SnakeHash(BlankBHT* theBlankBHT);
MOVE SlotsToMove(SLOT fromSlot, SLOT toSlot);

STRING MoveToString( MOVE );

void InitializeGame() {
	gMoveToStringFunPtr = &MoveToString;
}

/************************************************************************
**
** NAME:        DebugMenu
**
** DESCRIPTION: Menu used to debub internal problems. Does nothing if
**              kDebugMenu == FALSE
**
************************************************************************/

void DebugMenu()
{
}

/************************************************************************
**
** NAME:        GameSpecificMenu
**
** DESCRIPTION: Menu used to change game-specific parmeters, such as
**              the side of the board in an nxn Nim board, etc. Does
**              nothing if kGameSpecificMenu == FALSE
**
************************************************************************/

void GameSpecificMenu()
{
	char GetMyChar();
	POSITION GetInitialPosition();

	do {
		printf(" \n\t----- Game-specific options for %s -----\n\n", kGameName);

		printf("\tCurrent Initial Position:\n");
		PrintPosition(gInitialPosition, gPlayerName[kPlayerOneTurn], kHumansTurn);

		printf("\tI)\tChoose the (I)nitial position\n");

		printf("\tM)\tSwitch (M)ovable parts from %s to %s\n",
		       !moveHandT ? "HEAD ONLY" : "EITHER HEAD OR TAIL",
		       moveHandT ? "HEAD ONLY" : "EITHER HEAD OR TAIL");

		printf("\n\n\tb)\t(B)ack = Return to previous activity.\n");
		printf("\n\nSelect an option: ");

		switch(GetMyChar()) {
		case 'Q': case 'q':
			ExitStageRight();
		case 'H': case 'h':
			HelpMenus();
			break;
		case 'I': case 'i':
			gInitialPosition = GetInitialPosition();
			break;
		case 'b': case 'B':
			return;
		case 'M': case 'm':
			moveHandT = !moveHandT;
			break;
		default:
			printf("\nSorry, I don't know that option. Try another.\n");
			HitAnyKeyToContinue();
			break;
		}
	} while(TRUE);

}

/************************************************************************
**
** NAME:        SetTclCGameSpecificOptions
**
** DESCRIPTION: Set the C game-specific options (called from Tcl)
**              Ignore if you don't care about Tcl for now.
**
************************************************************************/

void SetTclCGameSpecificOptions(theOptions)
int theOptions[];
{
	moveHandT = (BOOLEAN) theOptions[1];
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
** CALLS:       SnakeHash ()
**              SnakeUnhash ()
**              whoseTurn ()
**	      MoveToSlots()
*************************************************************************/
SLOT DESIREDPIECE; /* Global variable to determine whose turn it is. */

POSITION DoMove(thePosition, theMove)
POSITION thePosition;
MOVE theMove;
{
	SLOT fromSlot, toSlot;
	BlankBHT theBlankBHT[BOARDSIZE], who;
	POSITION newPosition;

	SnakeUnhash(thePosition, theBlankBHT);
	MoveToSlots(theMove, &fromSlot, &toSlot);
	if (!moveHandT) {
		who = whoseTurn(theBlankBHT);

		theBlankBHT[fromSlot] = b;
		if((int)who == (int)h)
			theBlankBHT[toSlot] = h;
		else
			theBlankBHT[toSlot] = t;

		newPosition = SnakeHash(theBlankBHT);
	} else {
		who = theBlankBHT[DESIREDPIECE];
		theBlankBHT[fromSlot] = b;
		if((int)who == (int)h)
			theBlankBHT[toSlot] = h;
		else
			theBlankBHT[toSlot] = t;

		newPosition = SnakeHash(theBlankBHT);

		return newPosition;
	}

	/* should never reach here? */
	return newPosition;
}

/* whoseTurn takes in theBlankBHT and returns H or T in a length 1
 * BlankBHT */
BlankBHT whoseTurn(theBlankBHT)
BlankBHT *theBlankBHT;
{
	int i;
	int count = 0;
	BlankBHT who;

	for(i = 0; i < BOARDSIZE; i++) {
		if(theBlankBHT[i] == b)
			count++;
	}

	if((count % 2) == 0)
		who = t;
	else
		who = h;

	return who;
}

/************************************************************************
**
** NAME:        GetInitialPosition
**
** DESCRIPTION: Ask the user for an initial position for testing. Store
**              it in the space pointed to by initialPosition;
**
** OUTPUTS:     POSITION initialPosition : The position to fill.
**
************************************************************************/

POSITION GetInitialPosition()
{
	BlankBHT theBlankBHT[BOARDSIZE];
	signed char c;
	int i;

	printf("\n\n\t----- Get Initial Position -----\n");
	printf("\n\tPlease input the position to begin with.\n");
	printf("\tNote that it should be in the following format:\n\n");
	printf("+   +   +   +\n");
	printf("             \n");
	printf("+   H - B   +\n");
	printf("        |           <----- EXAMPLE\n");
	printf("+   +   T   +\n");
	printf("             \n");
	printf("+   +   +   +\n");
	printf("For example, to get the position printed above, type:\n");
	printf("++++ \n+hb+ \n++t+ \n++++ \n");
	printf("Input position here: \n");

	i = 0;
	getchar();
	while(i < BOARDSIZE && (c = getchar()) != EOF) {
		if(c == 'h' || c == 'H')
			theBlankBHT[i++] = h;
		else if(c == 't' || c == 'T')
			theBlankBHT[i++] = t;
		else if(c == 'b' || c == 'B')
			theBlankBHT[i++] = b;
		else if(c == '+')
			theBlankBHT[i++] = Blank;
		else
			; /* do nothing */
	}

	printf("In this game, the head of the snake always goes first.\n");

	return(SnakeHash(theBlankBHT));
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

void PrintComputersMove(computersMove, computersName)
MOVE computersMove;
STRING computersName;
{
	SLOT fromSlot,toSlot;
	MoveToSlots(computersMove,&fromSlot,&toSlot);
	printf("%8s's move              : %d %d\n", computersName,
	       fromSlot+1,toSlot+1);
}

/************************************************************************
**
** NAME:        Primitive
**
** DESCRIPTION: Return the value of a position if it fulfills certain
**              'primitive' constraints. Some examples of this is having
**              no blanks to go to with Snake.  Having no blanks to move to
**              is a primitive lose, because the player who faces
**              this board has just lost. I.e. the player before him
**              created the board and won. Otherwise undecided.  Remember:
**              there are no primitive ties possible in this game.
**
** INPUTS:      POSITION position : The position to inspect.
**
** OUTPUTS:     (VALUE) an enum which is oneof: (win,lose,tie,undecided)
**
** CALLS:       SnakeUnhash ()
**              trapped ()
**              whoseTurn ()
**
************************************************************************/

VALUE Primitive(position)
POSITION position;
{
	BOOLEAN trapped();
	BlankBHT theBlankBHT[BOARDSIZE];
	BlankBHT who;

	SnakeUnhash(position,theBlankBHT);
	who = whoseTurn(theBlankBHT);


	if(trapped(theBlankBHT, who))
		return(gStandardGame ? lose : win);
	else
		return (undecided);
}

BOOLEAN trapped(theBlankBHT,who)
BlankBHT *theBlankBHT;
BlankBHT who;
{
	int whoIndex;
	int i;

	// finds index of where who is
	for(i = 0; i < BOARDSIZE; i++) {
		if(who == theBlankBHT[i])
			whoIndex = i;
	}

	// checking above
	if((whoIndex - BOARDWIDTH) >= 0 &&
	   theBlankBHT[whoIndex - BOARDWIDTH] == Blank)
		return FALSE;
	// checking below
	else if((whoIndex + BOARDWIDTH) < BOARDSIZE &&
	        theBlankBHT[whoIndex + BOARDWIDTH] == Blank)
		return FALSE;
	// checking left
	else if((whoIndex % BOARDWIDTH) != 0 &&
	        theBlankBHT[whoIndex - 1] == Blank)
		return FALSE;
	// checking right
	else if(((whoIndex + 1) % BOARDWIDTH) != 0 &&
	        theBlankBHT[whoIndex + 1] == Blank)
		return FALSE;
	// if all surrounding slots are full, then return true
	else return TRUE;
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
** CALLS:       SnakeUnhash()
**              GetValueOfPosition();
**              GetPrediction()
**              constructTree ();
**              farthestTail ();
**              path ();
**              PrintBoard ();
**
************************************************************************/
/* PrintPosition function call prototypes:  */
struct node * constructTree (struct node* node, BlankBHT boardArray[36], struct node* nodeArray[36], struct node *root);
struct node* farthestTail (struct node *tree, BlankBHT boardArray[36]);
void path(struct node *tailNode, struct node* pathArray[tailNode->depth]);
void PrintBoard(struct node* *pathArray, int pathArrayLength, BlankBHT board[36]);

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn) {

	int i;
	//    VALUE GetValueOfPosition();
	BlankBHT theBlankBHT[BOARDSIZE];

	SnakeUnhash(position, theBlankBHT);

	//make a 6x6 board, which is the 4x4 board with a border around it.
	BlankBHT board[36];

	//All the borders of BOARD will be 'Blank'
	for (i=0; i<36; i++)
		board[i] = Blank;

	//Fit theBlankBHT into BOARD
	for (i=0; i<4; i++)
		board[i+7] = theBlankBHT[i];
	for (i=4; i<8; i++)
		board[i+9] = theBlankBHT[i];
	for (i=8; i<12; i++)
		board[i+11] = theBlankBHT[i];
	for (i=12; i<16; i++)
		board[i+13] = theBlankBHT[i];

	//A 4x4 array of nodes, all initially NULL
	struct node * nodeArray[36];

	for (i=0; i<36; i++) {
		nodeArray[i] = NULL;
	}

	//find what slot of BOARD the head is in
	struct node* headNode = (struct node*)malloc(sizeof(struct node));
	for (i=0; i<36; i++) {
		if (board[i] == h)
			headNode->position = i;
	}

	headNode->parent = NULL;
	headNode->up = headNode->left = headNode->right = headNode->down = NULL;
	headNode->piece = h;
	headNode->depth = 1;

	nodeArray[headNode->position] = headNode;

	headNode = constructTree(headNode, board, nodeArray, headNode);

	struct node *tail = (struct node*)malloc(sizeof(struct node));
	tail->piece = t;
	tail = farthestTail(headNode, board);
	struct node* pathArray[tail->depth];
	path(tail,pathArray);

	PrintBoard(pathArray,tail->depth,board);
}

// True iff ANCESTOR is an ancestor of NODE.
int isAncestorOf(struct node *ancestor, struct node *node, struct node *root) {
	if (ancestor == NULL)
		return 0;
	if (node == NULL)
		return 0;
	if (node == root)
		return 0;
	if (node->parent == ancestor)
		return 1;
	else return isAncestorOf (ancestor, node->parent, root);
}

//Constructs the tree for all possible paths from the head (stored in NODE) to the tail.
struct node * constructTree (struct node* node, BlankBHT boardArray[36], struct node* nodeArray[36], struct node *root) {

	int pos = node->position;
	if (boardArray[pos] == t)
		return node;
	if ( ((boardArray[pos+1] == b) || (boardArray[pos+1] == t)) &&
	     (isAncestorOf (nodeArray[pos+1], node, root) != 1) ) {
		struct node *rNode = (struct node *)malloc(sizeof(struct node));

		rNode->position = pos+1;
		rNode->parent = node;
		rNode->left = rNode->right = rNode->up = rNode->down = NULL;
		rNode->depth = (node->depth)+1;
		node->right = rNode;
		nodeArray[pos+1] = rNode;
		constructTree (rNode, boardArray, nodeArray, root);
	}

	if ( ((boardArray[pos-1] == b) || (boardArray[pos-1] == t)) &&
	     (isAncestorOf (nodeArray[pos-1], node, root) == 0) ) {
		struct node *lNode = (struct node *)malloc(sizeof(struct node));
		lNode->position = pos-1;
		lNode->parent = node;
		lNode->left = lNode->right = lNode->up = lNode->down = NULL;
		lNode->depth = (node->depth)+1;
		node->left = lNode;
		nodeArray[pos-1] = lNode;
		constructTree (lNode, boardArray, nodeArray, root);
	}

	if ( ((boardArray[pos+6] == b) || (boardArray[pos+6] == t)) &&
	     (isAncestorOf (nodeArray[pos+6], node, root) == 0) ) {
		struct node *dNode = (struct node *)malloc(sizeof(struct node));
		dNode->position = pos+6;
		dNode->parent = node;
		dNode->left = dNode->right = dNode->up = dNode->down = NULL;
		dNode->depth = (node->depth)+1;
		node->down = dNode;
		nodeArray[pos+6] = dNode;
		constructTree (dNode, boardArray, nodeArray, root);
	}

	if ( ((boardArray[pos-6] == b) || (boardArray[pos-6] == t)) &&
	     (isAncestorOf (nodeArray[pos-6], node, root) == 0) ) {
		struct node *uNode = (struct node *)malloc(sizeof(struct node));
		uNode->position = pos-6;
		uNode->parent = node;
		uNode->left = uNode->right = uNode->up = uNode->down = NULL;
		uNode->depth = (node->depth)+1;
		node->up = uNode;
		nodeArray[pos-6] = uNode;
		constructTree (uNode, boardArray, nodeArray, root);
	}

	return node;
}

// If the node is a leaf node. (It has no children).
int isTrapped (struct node *node) {
	if ((node->left == NULL) && (node->right == NULL) && (node->up == NULL) && (node->down == NULL))
		return 1;
	else
		return 0;
}

// Given 4 nodes, returns the one that has the largest depth.
struct node* maxDepthOf(struct node *a, struct node *b, struct node *c, struct node *d) {

	if (a != NULL)
		if ( ( (b == NULL) || (a->depth >= b->depth) ) &&
		     ( (c == NULL) || (a->depth >= c->depth) ) &&
		     ( (d == NULL) || (a->depth >= d->depth) ) )
			return a;

	if (b != NULL)
		if ( ( (a == NULL) || (b->depth >= a->depth) ) &&
		     ( (c == NULL) || (b->depth >= c->depth) ) &&
		     ( (d == NULL) || (b->depth >= d->depth) ) )
			return b;

	if (c != NULL)
		if ( ( (a == NULL) || (c->depth >= a->depth) ) &&
		     ( (b == NULL) || (c->depth >= b->depth) ) &&
		     ( (d == NULL) || (c->depth >= d->depth) ) )
			return c;

	if (d != NULL)
		if ( ( (a == NULL) || (d->depth >= a->depth) ) &&
		     ( (b == NULL) || (d->depth >= b->depth) ) &&
		     ( (c == NULL) || (d->depth >= c->depth) ) )
			return d;

	//should never reach here
	return NULL;
}

// Returns the node of the tree which is the tail piece farthest away (most deep) from the head.
struct node* farthestTail (struct node *tree, BlankBHT boardArray[36]) {
	if (tree == NULL)
		return NULL;
	if (boardArray[tree->position] == t)
		return tree;
	return maxDepthOf(farthestTail(tree->left, boardArray),
	                  farthestTail(tree->right, boardArray),
	                  farthestTail(tree->up, boardArray),
	                  farthestTail(tree->down, boardArray));
}


// took tailNode->depth out of pathArray (we don't need to know its size to declare
// the function) -JJ
void path(struct node *tailNode, struct node* pathArray[]) {
	struct node* temp = tailNode;
	//struct node* pathArray[temp->depth];
	int i = (temp->depth)-1;

	while (i >= 0) {
		pathArray[i] = temp;
		temp = temp->parent;
		i--;
	}
	//return pathArray;
}

//Takes the array of the path from head to tail, and the board, and print out the snake.
void PrintBoard(struct node* *pathArray, int pathArrayLength, BlankBHT board[36]) {


	//Create a 7x7 board to hold all the pieces as well as the body segments
	int bigBoard[49];
	int i;

	//initialize the 7x7 board to all "" characters first
	for (i=0; i<49; i++)
		bigBoard[i] = 6; //6 is for a "" character

	//board[9] = Blank;
	//Stick all the B, H, and T pieces into the 7*7 bigBoard
	for (i = 7; i < 11; i++)
		bigBoard[(i-7)*2] = (int)board[i];
	for (i = 13; i < 17; i++)
		bigBoard[(i-9)*2+6] = (int)board[i];
	for (i = 19; i < 23; i++)
		bigBoard[(i-11)*2+12] = (int)board[i];
	for (i = 25; i < 29; i++)
		bigBoard[(i-13)*2+18] = (int)board[i];


	//Look for the head's position, start building the body by following pathArray
	int pos;
	for (i = 0; i < 49; i=i+2) {
		if (bigBoard[i] == h)
			pos = i;
	}

	//temp pointers to move down the pathArray
	struct node *temp = pathArray[0];
	struct node *tempnext = pathArray[1];

	int k = 1;
	while (temp != tempnext) {
		if (tempnext->position == (temp->position+6)) {
			bigBoard[pos+7] = 4; // 4 is the representation for '|'
			pos = pos+14;
		}else if (tempnext->position == (temp->position-6)) {
			bigBoard[pos-7] = 4; // 4 is the representation for '|'
			pos= pos-14;
		}else if (tempnext->position == (temp->position+1)) {
			bigBoard[pos+1] = 5; // 5 is the representation for '-'
			pos = pos+2;
		}else if (tempnext->position == (temp->position-1)) {
			bigBoard[pos-1] = 5; // 5 is the representation for '-'
			pos = pos-2;
		}
		temp = tempnext;
		k++;
		if (k < pathArrayLength)
			tempnext = pathArray[k];
	}

	printf("\n          1   2   3   4             %s %s %s %s %s %s %s \n",

	       gBlankBHTString[bigBoard[0]],
	       gBlankBHTString[bigBoard[1]],
	       gBlankBHTString[bigBoard[2]],
	       gBlankBHTString[bigBoard[3]],
	       gBlankBHTString[bigBoard[4]],
	       gBlankBHTString[bigBoard[5]],
	       gBlankBHTString[bigBoard[6]]);

	printf("                                    %s %s %s %s %s %s %s \n",

	       gBlankBHTString[bigBoard[7]],
	       gBlankBHTString[bigBoard[8]],
	       gBlankBHTString[bigBoard[9]],
	       gBlankBHTString[bigBoard[10]],
	       gBlankBHTString[bigBoard[11]],
	       gBlankBHTString[bigBoard[12]],
	       gBlankBHTString[bigBoard[13]]);

	printf("LEGEND:   5   6   7   8  TOTAL:     %s %s %s %s %s %s %s \n",
	       gBlankBHTString[bigBoard[14]],
	       gBlankBHTString[bigBoard[15]],
	       gBlankBHTString[bigBoard[16]],
	       gBlankBHTString[bigBoard[17]],
	       gBlankBHTString[bigBoard[18]],
	       gBlankBHTString[bigBoard[19]],
	       gBlankBHTString[bigBoard[20]]);
	printf("                                    %s %s %s %s %s %s %s \n",

	       gBlankBHTString[bigBoard[21]],
	       gBlankBHTString[bigBoard[22]],
	       gBlankBHTString[bigBoard[23]],
	       gBlankBHTString[bigBoard[24]],
	       gBlankBHTString[bigBoard[25]],
	       gBlankBHTString[bigBoard[26]],
	       gBlankBHTString[bigBoard[27]]);
	printf("          9   10  11  12            %s %s %s %s %s %s %s \n",

	       gBlankBHTString[bigBoard[28]],
	       gBlankBHTString[bigBoard[29]],
	       gBlankBHTString[bigBoard[30]],
	       gBlankBHTString[bigBoard[31]],
	       gBlankBHTString[bigBoard[32]],
	       gBlankBHTString[bigBoard[33]],
	       gBlankBHTString[bigBoard[34]]);
	printf("                                    %s %s %s %s %s %s %s \n",
	       gBlankBHTString[bigBoard[35]],
	       gBlankBHTString[bigBoard[36]],
	       gBlankBHTString[bigBoard[37]],
	       gBlankBHTString[bigBoard[38]],
	       gBlankBHTString[bigBoard[39]],
	       gBlankBHTString[bigBoard[40]],
	       gBlankBHTString[bigBoard[41]]);
	printf("          13  14  15  16            %s %s %s %s %s %s %s \n\n",
	       gBlankBHTString[bigBoard[42]],
	       gBlankBHTString[bigBoard[43]],
	       gBlankBHTString[bigBoard[44]],
	       gBlankBHTString[bigBoard[45]],
	       gBlankBHTString[bigBoard[46]],
	       gBlankBHTString[bigBoard[47]],
	       gBlankBHTString[bigBoard[48]]);
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
**
************************************************************************/

MOVELIST *GenerateMoves(position)
POSITION position;
{
	MOVELIST *CreateMovelistNode(), *head = NULL;
	VALUE Primitive();
	BlankBHT theBlankBHT[BOARDSIZE], who;
	SLOT fromSlot, toSlot;
	int i;

	if (!moveHandT) {

		if(Primitive(position) == undecided) {

			SnakeUnhash(position, theBlankBHT);
			who = whoseTurn(theBlankBHT);

			// fromSlot is where who is
			for(i = 0; i < BOARDSIZE; i++) {
				if(theBlankBHT[i] == who)
					fromSlot = i;
			}

			// moving up
			toSlot = fromSlot - BOARDWIDTH;
			if(toSlot >= 0 && theBlankBHT[toSlot] == Blank)
				head = CreateMovelistNode(SlotsToMove(fromSlot,toSlot), head);

			// moving down
			toSlot = fromSlot + BOARDWIDTH;
			if(toSlot < BOARDSIZE && theBlankBHT[toSlot] == Blank)
				head = CreateMovelistNode(SlotsToMove(fromSlot,toSlot), head);

			// moving left
			toSlot = fromSlot - 1;
			if((fromSlot % BOARDWIDTH) != 0 && theBlankBHT[toSlot] == Blank)
				head = CreateMovelistNode(SlotsToMove(fromSlot,toSlot), head);

			// moving right
			toSlot = fromSlot + 1;
			if((toSlot % BOARDWIDTH) != 0 && theBlankBHT[toSlot] == Blank)
				head = CreateMovelistNode(SlotsToMove(fromSlot,toSlot), head);

			return(head);
		} else {
			return(NULL);
		}
	} else {
		if(Primitive(position) == undecided) {
			SnakeUnhash(position, theBlankBHT);
			who = h;

			// fromSlot is where who is
			for(i = 0; i < BOARDSIZE; i++) {
				if(theBlankBHT[i] == who)
					fromSlot = i;
			}

			// moving up
			toSlot = fromSlot - BOARDWIDTH;
			if(toSlot >= 0 && theBlankBHT[toSlot] == Blank)
				head = CreateMovelistNode(SlotsToMove(fromSlot,toSlot), head);

			// moving down
			toSlot = fromSlot + BOARDWIDTH;
			if(toSlot < BOARDSIZE && theBlankBHT[toSlot] == Blank)
				head = CreateMovelistNode(SlotsToMove(fromSlot,toSlot), head);

			// moving left
			toSlot = fromSlot - 1;
			if((fromSlot % BOARDWIDTH) != 0 && theBlankBHT[toSlot] == Blank)
				head = CreateMovelistNode(SlotsToMove(fromSlot,toSlot), head);

			// moving right
			toSlot = fromSlot + 1;
			if((toSlot % BOARDWIDTH) != 0 && theBlankBHT[toSlot] == Blank)
				head = CreateMovelistNode(SlotsToMove(fromSlot,toSlot), head);

			who = t;

			// fromSlot is where who is
			for(i = 0; i < BOARDSIZE; i++) {
				if(theBlankBHT[i] == who)
					fromSlot = i;
			}

			// moving up
			toSlot = fromSlot - BOARDWIDTH;
			if(toSlot >= 0 && theBlankBHT[toSlot] == Blank)
				head = CreateMovelistNode(SlotsToMove(fromSlot,toSlot), head);

			// moving down
			toSlot = fromSlot + BOARDWIDTH;
			if(toSlot < BOARDSIZE && theBlankBHT[toSlot] == Blank)
				head = CreateMovelistNode(SlotsToMove(fromSlot,toSlot), head);

			// moving left
			toSlot = fromSlot - 1;
			if((fromSlot % BOARDWIDTH) != 0 && theBlankBHT[toSlot] == Blank)
				head = CreateMovelistNode(SlotsToMove(fromSlot,toSlot), head);

			// moving right
			toSlot = fromSlot + 1;
			if((toSlot % BOARDWIDTH) != 0 && theBlankBHT[toSlot] == Blank)
				head = CreateMovelistNode(SlotsToMove(fromSlot,toSlot), head);

			return(head);
		} else {
			return(NULL);
		}
	}
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
** CALLS:       ValidMove(MOVE, POSITION)
**              BOOLEAN PrintPossibleMoves(POSITION) ...Always True!
**
************************************************************************/

USERINPUT GetAndPrintPlayersMove(thePosition, theMove, playerName)
POSITION thePosition;
MOVE *theMove;
STRING playerName;
{
	USERINPUT ret;
	BlankBHT theBlankBHT[BOARDSIZE];
	SnakeUnhash(thePosition,theBlankBHT);

	do {
		printf("%8s's move [(u)ndo/1-%d 1-%d] : ", playerName, BOARDSIZE, BOARDSIZE);

		ret = HandleDefaultTextInput(thePosition, theMove, playerName);
		if(ret != Continue)
			return(ret);
	}
	while (TRUE);
	return(Continue); /* this is never reached, but lint is now happy */
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
** OUTPUTS:     BOOLEAN : TRUE iff the input is a valid text input.
**
************************************************************************/

BOOLEAN ValidTextInput(input)
STRING input;
{
	SLOT fromSlot, toSlot;
	int ret;
	ret = sscanf(input,"%d %d", &fromSlot, &toSlot);
	return(ret == 2 &&
	       fromSlot <= BOARDSIZE &&
	       fromSlot >= 1 &&
	       toSlot <= BOARDSIZE &&
	       toSlot >= 1);
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

MOVE ConvertTextInputToMove(input)
STRING input;
{
	MOVE SlotsToMove();
	SLOT fromSlot, toSlot;
	int text;
	text = sscanf(input, "%d %d", &fromSlot, &toSlot);

	fromSlot--;
	toSlot--;

	return(SlotsToMove(fromSlot,toSlot));
}

/************************************************************************
**
** NAME:        PrintMove
**
** DESCRIPTION: Print the move in a nice format.
**
** INPUTS:      MOVE *theMove         : The move to print.
**
************************************************************************/

void PrintMove(theMove)
MOVE theMove;
{
	STRING m = MoveToString( theMove );
	printf( "%s", m );
	SafeFree( m );
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

STRING MoveToString (theMove)
MOVE theMove;
{
	STRING m = (STRING) SafeMalloc( 8 );
	SLOT fromSlot, toSlot;
	MoveToSlots(theMove, &fromSlot, &toSlot);
	/* The plus 1 is because the user thinks it's 1-9, but MOVE is 0-8 */
	sprintf( m, "[%d %d]", fromSlot+1, toSlot+1);
	return m;
}


/************************************************************************
*************************************************************************
**         EVERYTHING BELOW THESE LINES IS LOCAL TO THIS FILE
*************************************************************************
************************************************************************/

/************************************************************************
**
** NAME:        SnakeUnhash
**
** DESCRIPTION: convert an internal position to that of a BlankBHT.
**
** INPUTS:      POSITION thePos     : The position input.
**              BlankBHT *theBlankBHT : The converted BlankBHT output array.
**
** CALLS:       BadElse()
**
************************************************************************/

void SnakeUnhash(thePos,theBlankBHT)
POSITION thePos;
BlankBHT *theBlankBHT;
{
	int findHead = (thePos & 0xf);
	int findTail = ((thePos >> 4) & 0xf);
	int findBody = (thePos >> 8);
	int i;

	// initialize the board to be all Blank
	for (i = 0; i < BOARDSIZE; i++)
		theBlankBHT[i] = Blank;

	// get head bits and place head on the board
	theBlankBHT[findHead] = h;

	// get tail bits and place tail on the board
	theBlankBHT[findTail] = t;

	// get slot bits and fill in the board
	for(i = 0; i < BOARDSIZE; i++) {
		if(theBlankBHT[i] == Blank) {
			if(findBody & 1) {
				theBlankBHT[i] = b;
				findBody = findBody >> 1;
			}
			else {
				theBlankBHT[i] = Blank;
				findBody = findBody >> 1;
			}
		}
	}
}

/************************************************************************
**
** NAME:        MoveToSlots
**
** DESCRIPTION: convert an internal move to that of two slots
**
** INPUTS:      MOVE theMove    : The move input.
**              SLOT *fromSlot  : The slot the piece moves from (output)
**              SLOT *toSlot    : The slot the piece moves to   (output)
**
************************************************************************/

void MoveToSlots(theMove, fromSlot, toSlot)
MOVE theMove;
SLOT *fromSlot, *toSlot;
{
	*fromSlot = theMove % (BOARDSIZE+1);
	*toSlot   = theMove / (BOARDSIZE+1);
	DESIREDPIECE = *fromSlot; /*added*/
}

/************************************************************************
**
** NAME:        SlotsToMove
**
** DESCRIPTION: convert two slots (from and to) to an encoded MOVE
**
** INPUT:       SLOT fromSlot   : The slot the piece moves from (0->BOARDSIZE)
**              SLOT toSlot     : The slot the piece moves to   (0->BOARDSIZE)
**
** OUTPUT:      MOVE            : The move corresponding to from->to
**
************************************************************************/

MOVE SlotsToMove(fromSlot, toSlot)
SLOT fromSlot, toSlot;
{
	return ((MOVE) toSlot*(BOARDSIZE+1) + fromSlot);
}

/************************************************************************
**
** NAME:        SnakeHash
**
** DESCRIPTION: convert a BlankBHT to that of an internal position.
**
** INPUTS:      BlankBHT *theBlankBHT : The converted BlankBHT output array.
**
** OUTPUTS:     POSITION: The equivalent position given the BlankBHT.
**
************************************************************************/

POSITION SnakeHash(theBlankBHT)
BlankBHT *theBlankBHT;
{
	POSITION thePosition = 0;
	int i;
	int bodyIndex = 0;

	for(i = 0; i < BOARDSIZE; i++) {
		// hash where the head is
		if(theBlankBHT[i] == h) {
			thePosition = (thePosition | i);
		}
		// hash where the tail is
		else if(theBlankBHT[i] == t) {
			thePosition = (thePosition | (i << 4));
		}
		// hash all parts of the body
		else if(theBlankBHT[i] == b) {
			thePosition = (thePosition | (1 << (bodyIndex + 8)));
			bodyIndex++;
		}
		else {
			bodyIndex++;
		}
	}

	return thePosition;
}


/* Database info */

int NumberOfOptions()
{
	return 2*2;
}

int getOption()
{
	int option = 1;
	option += gStandardGame ? 0 : 1;
	option += 2 * (moveHandT ? 1 : 0);
	return option;
}

void setOption(int option)
{
	option -= 1;

	if (option >= 2) {
		option -= 2;
		moveHandT = TRUE;
	}
	else
		moveHandT = FALSE;

	if (option >= 1) {
		option -= 1;
		gStandardGame = FALSE;
	}
	else
		gStandardGame = TRUE;

}

POSITION StringToPosition(char* board) {
	BlankBHT realBoard[BOARDSIZE];

	for (int i = 0; i < BOARDSIZE; i++) {
		switch (board[i]) {
			default:
				fprintf(stderr, "Error: Unexpected char in position\n");
				break;
			case ' ':
				realBoard[i] = Blank;
				break;
			case 'b':
				realBoard[i] = b;
				break;
			case 'h':
				realBoard[i] = h;
				break;
			case 't':
				realBoard[i] = t;
				break;
		}
	}

	return SnakeHash(realBoard);
}

char* PositionToString(POSITION pos) {
	BlankBHT board[BOARDSIZE];
	SnakeUnhash(pos, board);

	char *ret = SafeMalloc(sizeof(*ret) * (BOARDSIZE + 1));
	for (int i = 0; i < BOARDSIZE; i++) {
		switch (board[i]) {
			default:
				fprintf(stderr, "Error: Unexpected position\n");
				break;
			case Blank:
				ret[i] = ' ';
				break;
			case b:
				ret[i] = 'b';
				break;
			case h:
				ret[i] = 'h';
				break;
			case t:
				ret[i] = 't';
				break;
		}
	}
	ret[BOARDSIZE] = '\0';

	return ret;
}

char * PositionToEndData(POSITION pos) {
	return NULL;
}
