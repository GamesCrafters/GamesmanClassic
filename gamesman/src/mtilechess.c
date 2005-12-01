// $Id: mtilechess.c,v 1.1 2005-12-01 03:14:15 zwizeguy Exp $

/*
 * The above lines will include the name and log of the last person
 * to commit this file to CVS
 */

/************************************************************************
**
** NAME:        mtilechess.c
**
** DESCRIPTION: Tile Chess
**
** AUTHOR:      Brian Zimmer; Alan Roytman
**
** DATE:        WHEN YOU START/FINISH
**
** UPDATE HIST: RECORD CHANGES YOU HAVE MADE SO THAT TEAMMATES KNOW
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

STRING   kGameName            = ""; /* The name of your game */
STRING   kAuthorName          = ""; /* Your name(s) */
STRING   kDBName              = ""; /* The name to store the database under */

BOOLEAN  kPartizan            = FALSE ; /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN  kGameSpecificMenu    = FALSE ; /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN  kTieIsPossible       = FALSE ; /* TRUE if a tie is possible. FALSE if it is impossible.*/
BOOLEAN  kLoopy               = FALSE ; /* TRUE if the game tree will have cycles (a rearranger style game). FALSE if it does not.*/

BOOLEAN  kDebugMenu           = TRUE ; /* TRUE only when debugging. FALSE when on release. */
BOOLEAN  kDebugDetermineValue = FALSE ; /* TRUE only when debugging. FALSE when on release. */

POSITION gNumberOfPositions   =  0; /* The number of total possible positions | If you are using our hash, this is given by the hash_init() function*/
POSITION gInitialPosition     =  0; /* The initial hashed position for your starting board */
POSITION kBadPosition         = -1; /* A position that will never be used */

void*	 gGameSpecificTclInit = NULL;

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
#define BLACK 0, int pieceIndex
#define WHITE 1
#define BMAX 6
#define PLAYER1_TURN 1
#define PLAYER2_TURN 0

/*************************************************************************
**
** Global Variables
**
*************************************************************************/
int TOTAL_PIECES = 3;
int BOARD_SIZE = 9;
int hashed[168];
int unhashed[1024];
char *piecesunhashed[6];
int currTurn;

/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/
void reverse(char s[]);
void itoa(int n, char s[], int base);
int atobi(char s[], int base);
BOOLEAN isEqualString(char s[], char t[]);
int hashPieces(char pieceArray[]);
int isLegalBoard(int place);
void PreProcess();
POSITION hashBoard(char bA[], int currentPlayer);
int getCurrTurn(POSITION position);
char *unhashBoard(POSITION position);
void initializePiecesArray(int p_a[]);
char *FillBoardArray(char *pieceArray, char *placeArray);
BOOLEAN isLetter(char c);
void generateKingMoves(POSITION N, MOVELIST **moves, int place);
void generateKnightMoves(POSITION N, MOVELIST **moves, int place);
BOOLEAN isSameTeam(char piece, int currentPlayer);
MOVE createMove(int init, int final, int sidelength);
BOOLEAN testMove(int newspot, int origspot, POSITION N);
BOOLEAN inCheck(POSITION N);
BOOLEAN knightCheck(char *bA, int place, int currentPlayer);
BOOLEAN kingCheck(char *bA, int place, int currentPlayer);

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

void InitializeGame ()
{
    PreProcess();
//   	printf("INITGAME1");
/*	int init_pieces[10];
  	char board[] = "000111000";
    initializePiecesArray(init_pieces);
	generic_hash_init (BOARD_SIZE, init_pieces, NULL);
	gNumberOfPositions = 504; //Placement_max (84) * pieces_max (6)
    gInitialPosition = generic_hash(board, PLAYER1_TURN) + hashPieces("KNk");
*/
	gNumberOfPositions = 1008; //Placement_max (84) * pieces_max (6) * players (2)
    gInitialPosition = (((56 | (int)(pow(2,BOARD_SIZE)))*BMAX + hashPieces("KNk")) << 1)+PLAYER1_TURN;
    currTurn = PLAYER1_TURN;
//   printf("INITGAME2");
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
	/* Use CreateMovelistNode(move, next) to 'cons' together a linked list */
//	printf("GENMOVES1\n");
    MOVELIST *moves = NULL;
    int currentPlayer, i;
    char piece;
    char *boardArray;
    boardArray = unhashBoard(position);
    currentPlayer = getCurrTurn(position);
    for (i = 0; i < strlen(boardArray); i++) {
    	piece = boardArray[i];
    	if (isSameTeam(piece,currentPlayer)){
	    	if (piece >= 'a' && piece <= 'z') {
	    		piece = piece - 'a' + 'A';
	    	}
//	    	printf("%c\n",piece);
	    	switch (piece) {
	    		case 'K': 
	    			generateKingMoves(position,&moves,i);
//    				PrintMove(moves->move);
//	    			printf("GENKINGMOVES");
	    			break;    				
	    		case 'N': 
	    			generateKnightMoves(position,&moves,i);
//	    			printf("GENKNIGHTMOVES");
	    			break;
	    		default: 
	    			break;
	    	}
    	}
    }
//    printf("GENMOVES2");
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
//	printf("DOMOVE1");
	char *boardArray;
    int filef;
    int rankf;
    int filei;
    int ranki;
    char pieceMoved;
    int sideLength = (int)pow(BOARD_SIZE,0.5)+2;
    boardArray = unhashBoard(position);
    printf("IT IS THE FOLLOWING P'S TURN: %d\n",getCurrTurn(position));
//    printf("%s\n",boardArray);
//    printf("%d\n",position);
//	PrintPosition(position,"HELL",currTurn);
    //ANDing by 2^(4)-1 will give the last 4 bits
    filef = move & 15;
    rankf = (move >> 4) & 15;
    filei = (move >> 8) & 15;
    ranki = (move >> 12) & 15;
    ranki -= 10;
    rankf -= 10;
    pieceMoved = boardArray[(sideLength-filei)*sideLength+ranki];
//    printf("Piece to be moved: %c\nFrom %c%d[%d] to %c%d[%d]\n",pieceMoved,ranki+97,filei,((sideLength-filei)*sideLength+ranki),rankf+97,filef,((sideLength-filef)*sideLength+rankf));
    boardArray[(sideLength-filei)*sideLength+ranki] = ' ';
    if(boardArray[(sideLength-filef)*sideLength+rankf] != ' ') 
    	TOTAL_PIECES--;
    boardArray[(sideLength-filef)*sideLength+rankf] = pieceMoved;
//    printf("%s\n",boardArray);
//    printf("%d\n",hashBoard(boardArray));
//    printf("DOMOVE2");
//    printf("%s\n",boardArray);
//	PrintPosition(hashBoard(boardArray),"HELL2",currTurn);
	return hashBoard(boardArray,!getCurrTurn(position));
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
//	printf("PRIMITIVE");
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
     char *boardArray;
     int r, c, sideLength;
     boardArray = unhashBoard(position);
//     printf("'%s'\n",boardArray);
//     printf("'%llu'\n",position);
     sideLength = pow(BOARD_SIZE,0.5)+2;
     for(r = 0; r < sideLength; r++){
         printf("  +");
         for(c = 0; c < sideLength; c++){
             printf("---+");
         }
         printf("\n");
         printf("%d |", sideLength - r);
         for(c = 0; c < sideLength; c++){
             printf(" %c |", boardArray[r*sideLength+c]);
         }
         printf("\n");
     }
     printf("  +");
     for(c = 0; c < sideLength; c++){
         printf("---+");
     }
     printf("\n");
     printf("   ");
     for(c = 0; c < sideLength; c++){
         printf(" %c  ", 97+c);
     }
     printf("\n");
     printf("It is Player %d's turn (%s).\n",(usersTurn == TRUE) ? 1 : 2,(usersTurn == TRUE) ? "white/uppercase":"black/lowercase");
//     printf("PRINTPOSITION\n");
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
    printf("%x",move);
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
	printf("%8s's move [(undo)/<ranki><filei><rankf><filef>] : ", playersName);
	
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
	if(strlen(input)!=4)
    	return FALSE;
    else{
    	int sidelength = pow(BOARD_SIZE, 0.5)+2;
    	char maxChar = 'A' + sidelength-1;
    	char maxInt = '1' + sidelength-1;
//    	printf("\n%c\n%c\n",maxChar,maxInt);
    	char c = input[0];
    	if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';
    	if(c > maxChar || c < 'A') return FALSE;
//    	printf("REACHED 2");
    	c = input[2];
    	if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';
    	if(c > maxChar || c < 'A') return FALSE;
//    	printf("REACHED 1");
    	c = input[1];
    	if(c > maxInt || c < '1') return FALSE;
//    	printf("REACHED 3");
    	c = input[3];
    	if(c > maxInt || c < '1') return FALSE;
//    	printf("REACHED LAST");
    }
//    printf("REACHED TRUE");
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
    return (MOVE)atobi(input,16);
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

void reverse(char s[]) {
	int c, i, j;
	for (i = 0, j = strlen(s)-1; i < j; i++, j--) {
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}

int atobi(char s[], int base) {
	int i;
	int total = 0;
	char c;
	for (i = 0; s[i] != '\0'; i++) {
		c = s[i];
		total = total*base + ((isLetter(c))?c-'A'+10:c-'0');
	}
	return total;
}

void itoa(int n, char s[], int base){
	int i, sign, nmodbase;
	if ((sign = n) <0)
		n = -n;
	i=0;
	do {
		if ((nmodbase = n%base) > 9)
			s[i++] = nmodbase-10 + 'a';
		else
			s[i++] = n% base + '0';
	} while((n = n / base)>0);
	if(sign<0)
		s[i++] = '-';
	s[i] = '\0';
	reverse(s);
}

void PreProcess() {
	int i, legalcounter, hashednum;
	legalcounter = 0;
	for (i = 0; i < 512; i++) {
		if (isLegalBoard(i)) {
//			hashednum = (i << 1) + PLAYER1_TURN;
			hashed[legalcounter] = i; //hashednum;
			unhashed[i] = legalcounter; //hashednum
//			legalcounter++;
//			hashednum = (i << 1) + PLAYER2_TURN;
//			hashed[legalcounter] = i; //hashednum;
//			unhashed[i] = legalcounter; //hashednum
			legalcounter++;
		}
	}
	piecesunhashed[0] = "kKN";
	piecesunhashed[1] = "kNK";
	piecesunhashed[2] = "NkK";
	piecesunhashed[3] = "NKk";
	piecesunhashed[4] = "KNk";
	piecesunhashed[5] = "KkN";
}

BOOLEAN isEqualString(char s[], char t[]) {
	int i;
	for (i = 0; s[i] != '\0' && t[i] != '\0'; i++) {
		if (s[i] != t[i]) {
			return FALSE;
		}
	}
	return TRUE;
}

int hashPieces(char pieceArray[]) {
	if (isEqualString(pieceArray,"kKN")) {
		return 0;
	} else if (isEqualString(pieceArray,"kNK")) {
		return 1;
	} else if (isEqualString(pieceArray,"NkK")) {
		return 2;
	} else if (isEqualString(pieceArray,"NKk")) {
		return 3;
	} else if (isEqualString(pieceArray,"KNk")) {
		return 4;
	} else if (isEqualString(pieceArray,"KkN")) {
		return 5;
	} else {
		return -1;
	}
}

int isLegalBoard(int place) {
	char *pA;
	int i, pieces, surrounding, sidelength;
	pieces = surrounding = 0;
	if ((pA = malloc((BOARD_SIZE+1) * sizeof(char))) == NULL) {
        (void)fprintf(stderr, "ERROR: Malloc failed");
        (void)exit(EXIT_FAILURE);    /* or return EXIT_FAILURE; */
    }
    place = place | (int)pow(2,BOARD_SIZE);
    itoa(place,pA,2);
	sidelength = pow(BOARD_SIZE,0.5);
	pA++;
	for (i = 0; i < BOARD_SIZE; i++) {
		if (pA[i] == '1') {
			pieces++;
			if (i-sidelength >= 0 && pA[i-sidelength] == '1') {
				surrounding++;
			}
			if (i+sidelength < BOARD_SIZE && pA[i+sidelength] == '1') {
				surrounding++;
			}
			if ((i+1)%sidelength != 0 && pA[i+1] == '1') {
				surrounding++;
			}
			if (i%sidelength != 0 && pA[i-1] == '1') {
				surrounding++;
			}
			if (i-sidelength >= 0 && (i+1)%sidelength != 0 && pA[i-sidelength+1] == '1') {
				surrounding++;
			}
			if (i-sidelength >= 0 && i%sidelength != 0 && pA[i-sidelength-1] == '1') {
				surrounding++;
			}
			if (i+sidelength < BOARD_SIZE && (i+1)%sidelength != 0 && pA[i+sidelength+1] == '1') {
				surrounding++;
			}
			if (i+sidelength < BOARD_SIZE && i%sidelength != 0 && pA[i+sidelength-1] == '1') {
				surrounding++;
			}
			if (surrounding == 0) {
//				SafeFree(pA);
				return 0;
			} else {
				surrounding = 0;
			}
		}
	}
//	SafeFree(pA);
	if (pieces != 3) {
		return 0;
	} else {
		return 1;
	}
}

POSITION hashBoard(char boardArray[], int currentPlayer) {
	POSITION N;
	int newPlacement = 0;
    int A, B;
    char *snewPieces;
    int sideLength = pow(strlen(boardArray),0.5);
    int bi=0,pi=0,x,y;
    int minx=sideLength, maxx=0, miny=sideLength, maxy=0, difference;
    if ((snewPieces = malloc((TOTAL_PIECES+1) * sizeof(char))) == NULL) {
         (void)fprintf(stderr, "ERROR: Malloc failed");
         (void)exit(EXIT_FAILURE);    /* or return EXIT_FAILURE; */
    }
    for(bi=0;bi<strlen(boardArray);bi++){
        if(boardArray[bi]!=' '){
            x = bi%sideLength;
            y = bi/sideLength;
            //printf("BI: %d X:%d Y:%d\n", bi, x, y);
            if(x<minx) minx=x;
            if(x>maxx) maxx=x;
            if(y<miny) miny=y;
            if(y>maxy) maxy=y;
        }
    }
    //printf("Min X: %d, Max X: %d, Min Y: %d, Max Y: %d\n", minx, maxx, miny, maxy);
    difference = (maxx-minx)-(maxy-miny);
    if(difference!=0){
        //printf("Squarizing Board...");
        if(difference<0){
            difference=-difference;
            while(difference>0){
                if(minx>0)
                    minx--;
                else
                    maxx++;
                difference--;
            }
        }
        else{
            while(difference>0){
                if(miny>0)
                    miny--;
                else
                    maxy++;
                difference--;
            }
        }
        //printf("Done\n");
    }
    
    //KLUDGE
    if((maxx-minx)<2){
    	if(maxx<sideLength-1)
    		maxx++;
    	else
    		minx--;
    	
    	if(miny>0)
    		miny--;
    	else
    		maxy++;
    }
    
//    printf("Min X: %d, Max X: %d, Min Y: %d, Max Y: %d\n", minx, maxx, miny, maxy);
    //printf("'");
//  BOARD_SIZE = (maxy-miny+1)*(maxy-miny+1);
    for(y=miny;y<=maxy;y++){
        for(x=minx;x<=maxx;x++){
            bi=y*sideLength+x;
            //printf("%c", boardArray[bi]);
            if(boardArray[bi]!=' '){
                newPlacement = newPlacement | (1<<(BOARD_SIZE-((y-miny)*(maxy-miny+1)+(x-minx))-1)); // (strlen(boardArray)-1-bi) ---> boardSize-index of piece
                //printf("[%d]",(BOARD_SIZE-((y-miny)*(maxy-miny+1)+(x-minx))-1));
                snewPieces[pi++] = boardArray[bi];
            } 
        }
    }
    //printf("'\n");
    A = newPlacement | (int)pow(2,BOARD_SIZE);
    B = hashPieces(snewPieces);
    N = ((A*BMAX+B) << 1) | currentPlayer;
//    printf("A: %d, B: %d, N: %d\n",A,B,N);
//    SafeFree(snewPieces);
    return N;
}

int getCurrTurn(POSITION position) {
	return (position & 1);	
}

char *unhashBoard(POSITION position) {
	int A, B, currentPlayer;
	char *pieces;
	char *placement;
	char *bA;
	position = position >> 1;
	currentPlayer = getCurrTurn(position);
	if ((pieces = malloc((TOTAL_PIECES + 1) * sizeof(char))) == NULL) {
         (void)fprintf(stderr, "ERROR: Malloc failed");
         (void)exit(EXIT_FAILURE);    /* or return EXIT_FAILURE; */
    }
    if ((placement = malloc((BOARD_SIZE+1) * sizeof(char))) == NULL) {
         (void)fprintf(stderr, "ERROR: Malloc failed");
         (void)exit(EXIT_FAILURE);    /* or return EXIT_FAILURE; */
    }
	A = position/BMAX;
	B = position%BMAX;
	itoa(A,placement,2);
	pieces = piecesunhashed[B];
	bA = FillBoardArray(pieces,placement);
//	SafeFree(pieces);
//	SafeFree(placement);
	return bA;
}

char *FillBoardArray(char *pieceArray, char *placeArray) {
      int c, r, sideLength, displayBoardSize;
      char *bA;
      sideLength = (int)pow(BOARD_SIZE,0.5)+2; // +2 is for border
      displayBoardSize = sideLength*sideLength;
      if ((bA = malloc((displayBoardSize + 1) * sizeof(char))) == NULL) {
         (void)fprintf(stderr, "ERROR: Malloc failed");
         (void)exit(EXIT_FAILURE);    /* or return EXIT_FAILURE; */
      }
      placeArray++; //Ignore bit that indicates number of significant bits
      for(c = 0; c < sideLength; c++){
            bA[c] = ' ';
      }
      for (r = 1; r < sideLength-1; r++) {
            bA[r*sideLength] = ' ';
            for(c = 1; c < sideLength-1; c++){
                  if (*placeArray == '1') {
                        bA[r*sideLength+c] = *pieceArray;
                        pieceArray++;
                  }
                  else {
                        bA[r*sideLength+c] = ' ';
                  }
                  placeArray++;
            }
            bA[r*sideLength+c] = ' ';
      }
      for(c = 0; c < sideLength; c++){
            bA[displayBoardSize-sideLength+c] = ' ';
      }
      bA[displayBoardSize] = '\0';
      return bA;
}

void initializePiecesArray(int p_a[]) {
  /* {'1', 3, 3, '0', 6, 6, ' ', 0, 0, -1} */
  /*   0   1  2   3   4  5   6   7  8  9  */
  p_a[0] = '1';
  p_a[3] = '0';
  p_a[6] = ' ';
  p_a[9] = -1;
  p_a[1] = p_a[2] = 3;
  p_a[4] = p_a[5] = 6;
  p_a[7] = p_a[8] = 0;
}

BOOLEAN isLetter(char c) {
	return (c >= 'A' && c <= 'Z') ? TRUE : FALSE;
}

void generateKingMoves(POSITION N, MOVELIST **moves, int place) {
	int direction, sidelength;
	char *boardArray = unhashBoard(N);
	sidelength = pow(strlen(boardArray),0.5);
	/*MOVE UP*/
	direction = place-sidelength;
	if (testMove(direction,place,N) == TRUE) {
//		printf("KMOVE1\n");
		*moves = CreateMovelistNode(createMove(place,direction,sidelength), *moves);
	}
	/*MOVE DOWN*/
	direction = place+sidelength;
	if (testMove(direction,place,N) == TRUE) {
//		printf("KMOVE2\n");
		*moves = CreateMovelistNode(createMove(place,direction,sidelength), *moves);
	}
	/*MOVE RIGHT*/
	direction = place+1;
	if (testMove(direction,place,N) == TRUE) {
//		printf("KMOVE3\n");
		*moves = CreateMovelistNode(createMove(place,direction,sidelength), *moves);
	}
	/*MOVE LEFT*/
	direction = place-1;
	if (testMove(direction,place,N) == TRUE) {
//		printf("KMOVE4\n");
		*moves = CreateMovelistNode(createMove(place,direction,sidelength), *moves);
	}
	/*MOVE UPPER-RIGHT*/
	direction = place-sidelength+1;
	if (testMove(direction,place,N) == TRUE) {
//		printf("KMOVE5\n");
		*moves = CreateMovelistNode(createMove(place,direction,sidelength), *moves);
	}
	/*MOVE UPPER-LEFT*/
	direction = place-sidelength-1;
	if (testMove(direction,place,N) == TRUE) {
//		printf("KMOVE6\n");
		*moves = CreateMovelistNode(createMove(place,direction,sidelength), *moves);
	}
	/*MOVE LOWER-RIGHT*/
	direction = place+sidelength+1;
	if (testMove(direction,place,N) == TRUE) {
//		printf("KMOVE7\n");
		*moves = CreateMovelistNode(createMove(place,direction,sidelength), *moves);
	}
	/*MOVE LOWER-LEFT*/
	direction = place+sidelength-1;
	if (testMove(direction,place,N) == TRUE) {
//		printf("KMOVE8\n");
		*moves = CreateMovelistNode(createMove(place,direction,sidelength), *moves);
	}
}

void generateKnightMoves(POSITION N, MOVELIST **moves, int place) {
//	printf("INSIDEGENKNIGHTMOVES");
	return;
}

BOOLEAN isSameTeam(char piece, int currentPlayer) {
	if (piece >= 'a' && piece <= 'z' && currentPlayer == PLAYER2_TURN) {
		return TRUE;
	} else if (piece >= 'A' && piece <= 'Z' && currentPlayer == PLAYER1_TURN) {
		return TRUE;
	} else {
		return FALSE;
	}
}

MOVE createMove(int init, int final, int sidelength) {
	MOVE m;
	int filei, ranki, filef, rankf;
	filei = 10+init%sidelength;
	filef = 10+final%sidelength;
	ranki = sidelength-(init/sidelength);
	rankf = sidelength-(final/sidelength);
	m = (filei << 12) | (ranki << 8) | (filef << 4) | rankf;
	return m;
}

BOOLEAN testMove(int newspot, int origspot, POSITION N) {
	int sidelength, placement, A;
	char newpiece;
	POSITION NEW_N;
	char *bA = unhashBoard(N);
	sidelength = pow(strlen(bA),0.5);
	newpiece = bA[newspot];
	if (!isSameTeam(newpiece,getCurrTurn(N))) {
		bA[newspot] = bA[origspot];
		bA[origspot] = ' ';
		NEW_N = hashBoard(bA,getCurrTurn(N));
		NEW_N = NEW_N >> 1;
		A = NEW_N/BMAX;
		placement = A & (int)(pow(2,BOARD_SIZE)-1);
		if (isLegalBoard(placement) && !inCheck(NEW_N)) {
			bA[origspot] = bA[newspot];
			bA[newspot] = newpiece;
			return TRUE;
		}
		bA[origspot] = bA[newspot];
		bA[newspot] = newpiece;
	}
	return FALSE;
}

BOOLEAN inCheck(POSITION N) {
	int i;
	char piece;
	int currentPlayer = getCurrTurn(N);
	char *bA = unhashBoard(N);
	for (i = 0; i < strlen(bA); i++) {
		piece = bA[i];
		if (!isSameTeam(piece,currentPlayer)) {
			if (piece >= 'a' && piece <= 'z') {
	    		piece = piece - 'a' + 'A';
	    	}
	    	switch (piece) {
	    		case 'K': 
	    			if (kingCheck(bA, i, currentPlayer) == TRUE) {
		    			return TRUE;
	    			} else {
		    			break;    				
	    			}
	    		case 'N': 
	    			if (knightCheck(bA, i, currentPlayer) == TRUE) {
		    			return TRUE;
	    			} else {
		    			break;    				
	    			}
	    		default: 
	    			break;
	    	}
		}
	}
	return FALSE;
}

BOOLEAN knightCheck(char *bA, int place, int currentPlayer){
	return FALSE;
}

BOOLEAN kingCheck(char *bA, int place, int currentPlayer){
	char king;
	int sidelength;
	king = (currentPlayer == PLAYER2_TURN) ? 'k' : 'K';
	sidelength = pow(strlen(bA),0.5);
	return  ((bA[place-sidelength]==king) ||
			 (bA[place+sidelength]==king) ||
			 (bA[place+1]==king) ||
			 (bA[place-1]==king) ||
			 (bA[place-sidelength+1]==king) ||
			 (bA[place-sidelength-1]==king) ||
			 (bA[place+sidelength+1]==king) ||
			 (bA[place+sidelength-1]==king)) ? TRUE : FALSE;
}

/*
int main() {
	PreProcess();
	GenerateMoves((((84 | (int)(pow(2,BOARD_SIZE)))*BMAX + hashPieces("kNK")) << 1)+PLAYER1_TURN);
	inCheck("            Nk  K        ",PLAYER2_TURN);
	GenerateMoves((((84 | (int)(pow(2,BOARD_SIZE)))*BMAX + hashPieces("kNK")) << 1)+PLAYER1_TURN);
	InitializeGame();

//   	printf("INITGAME1");
	int init_pieces[10];
  	char board[] = "000111000";
    initializePiecesArray(init_pieces);
//	generic_hash_init (BOARD_SIZE, init_pieces, NULL);
	gNumberOfPositions = 504; //Placement_max (84) * pieces_max (6)
//  gInitialPosition = generic_hash(board, PLAYER1_TURN) + hashPieces("KNk");
	gNumberOfPositions = 504; //Placement_max (84) * pieces_max (6)
    gInitialPosition = ((unhashed[56]*BMAX + hashPieces("KNk")) << 1)+PLAYER1_TURN;
    currTurn = PLAYER1_TURN;
//   printf("INITGAME2");
	hashBoard("   KNk   ");
	hashBoard("    NkK  ");
	DoMove(6824,46002);
	return 0;	
}
*/

// $Log: not supported by cvs2svn $
// Revision 1.5  2005/10/06 03:06:11  hevanm
// Changed kDebugDetermineValue to be FALSE.
//
// Revision 1.4  2005/05/02 17:33:01  nizebulous
// mtemplate.c: Added a comment letting people know to include gSymmetries
// 	     in their getOption/setOption hash.
// mttc.c: Edited to handle conflicting types.  Created a PLAYER type for
//         gamesman.  mttc.c had a PLAYER type already, so I changed it.
// analysis.c: Changed initialization of option variable in analyze() to -1.
// db.c: Changed check in the getter functions (GetValueOfPosition and
//       getRemoteness) to check if gMenuMode is Evaluated.
// gameplay.c: Removed PlayAgainstComputer and PlayAgainstHuman.  Wrote PlayGame
//             which is a generic version of the two that uses to PLAYER's.
// gameplay.h: Created the necessary structs and types to have PLAYER's, both
// 	    Human and Computer to be sent in to the PlayGame function.
// gamesman.h: Really don't think I changed anything....
// globals.h: Also don't think I changed anything....both these I just looked at
//            and possibly made some format changes.
// textui.c: Redid the portion of the menu that allows you to choose opponents
// 	  and then play a game.  Added computer vs. computer play.  Also,
//           changed the analysis part of the menu so that analysis should
// 	  work properly with symmetries (if it is in getOption/setOption hash).
//
// Revision 1.3  2005/03/10 02:06:47  ogren
// Capitalized CVS keywords, moved Log to the bottom of the file - Elmer
//
