// $id$
// $log$

/*
  * The above lines will include the name and log of the last person
  * to commit this file to CVS
  */

/************************************************************************
**
** NAME:        mtore.c
**
** DESCRIPTION: Mu Torere
**
** AUTHOR:      Joe Jing, Jeff Chou
**
** DATE:        2004-9-28 Started Module.
**
** UPDATE HIST: 2004-9-28 Started Module. Print Position, Do Move, Hash Stuff.
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

STRING   kGameName            = ""; /* The name of your game */
//STRING   kAuthorName          = ""; /* Your name(s) */
STRING   kDBName              = ""; /* The name to store the database under */

BOOLEAN  kPartizan            = FALSE ; /* A partizan game is a game where each player has 
different moves from the same board (chess - different pieces) */
BOOLEAN  kGameSpecificMenu    = FALSE ; /* TRUE if there is a game specific menu. FALSE if there 
is not one. */
BOOLEAN  kTieIsPossible       = FALSE ; /* TRUE if a tie is possible. FALSE if it is impossible.*/
BOOLEAN  kLoopy               = FALSE ; /* TRUE if the game tree will have cycles (a rearranger 
style game). FALSE if it does not.*/

BOOLEAN  kDebugMenu           = TRUE ; /* TRUE only when debugging. FALSE when on release. */
BOOLEAN  kDebugDetermineValue = TRUE ; /* TRUE only when debugging. FALSE when on release. */

POSITION gNumberOfPositions   =  0; /* The number of total possible positions | If you are using 
our hash, this is given by the hash_init() function*/
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
#define move_from(x) (x / 9)
#define move_to(x) (x % 9)

#define move_make(x,y) ((x * 9) + y)
/*************************************************************************
**
** Global Variables
**
*************************************************************************/

char gBoard[9];

/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/

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
   int init_array[10];
   init_array[0] = '_';
   init_array[1] = 1;
   init_array[2] = 1;
   init_array[3] = 'x';
   init_array[4] = 4;
   init_array[5] = 4;
   init_array[6] = 'o';
   init_array[7] = 4;
   init_array[8] = 4;
   init_array[9] = -1;


   gNumberOfPositions = generic_hash_init(9, init_array, NULL);

   gBoard[1] = gBoard[2] = gBoard[3] = gBoard[4] = 'o';
   gBoard[5] = gBoard[6] = gBoard[7] = gBoard[8] = 'x';
   gBoard[0] = '_';

   gInitialPosition = generic_hash(gBoard, 1); }


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

MOVELIST *GenerateMoves (POSITION position) {String turn;
  String opp;
  if (whoseMove(position) == 1){
    turn = 'x';
    opp = 'o';}
  else {
    turn = 'o';
    opp = 'x';}
     MOVELIST *moves = NULL;
     MOVELIST *CreameMoveListNode();
     for (int i = 0; i < 9; i++){
       if (gBoard[i] == turn){
	if (i == 0){
	  for (int x = 1; x < 9; x++){
	    if (gBoard[x] == '_'){
	      moves = CreateMovelistNode(move_make(i, x), moves);}}}
	else 
	  if (i == 1){
	    if (gBoard[8] == '_'){
	      moves = CreateMovelistNode(move_make(i, 8), moves);}
	    else if (gBoard[8] == opp && gBoard[0] == '_'){
	      moves = CreateMovelistNode(move_make(i, 0), moves);}
	    else if (gBoard[i+1] == '_'){
	      moves = CreateMovelistNode(move_make(i, i+1), moves);}
	    else if (gBoard[i+1] == opp && gBoard[0] == '_'){
	      moves = CreateMovelistNode(move_make(i, 0), moves);}}
	else 
	  if (i == 8){
	     if (gBoard[1] == '_'){
	      moves = CreateMovelistNode(move_make(i, 1), moves);}
	    else if (gBoard[1] == opp && gBoard[0] == '_'){
	      moves = CreateMovelistNode(move_make(i, 0), moves);}
	    else if (gBoard[i-1] == '_'){
	      moves = CreateMovelistNode(move_make(i, i-1), moves);}
	    else if (gBoard[i-1] == opp && gBoard[0] == '_'){
	      moves = CreateMovelistNode(move_make(i, 0), moves);}}
	else {
	  if (gBoard[i-1] == '_'){
	    moves = CreateMovelistNode(move_make(i, i-1), moves);}
	  else if (gBoard[i+1] == '_'){
	    moves = CreateMovelistNode(move_make(i, i+1), moves);}
	  else if (gBoard[i-1] == opp && gBoard[0] == '_'){
	    moves = CreateMovelistNode(move_make(i, 0), moves);}
	  else if (gBoard[i+1] == opp && gBoard[0] == '_'){
	    moves = CreateMovelistNode(move_make(i, 0), moves);}}}}
	    
	      
	
     
     /* Use CreateMovelistNode(move, next) to 'cons' together a linked list */
     
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

POSITION DoMove (POSITION position, MOVE move) { int from, to;
  char oldc; 
   generic_unhash(position,gBoard);
   from = move_from(move);
   to = move_to(move);
   oldc = gBoard[from];
   gBoard[from] = '_';
   gBoard[to] = oldc;
   if (whoseMove(position) == 1) 
     return generic_hash(gBoard,2);
   else
     return generic_hash(gBoard, 1);	
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
*********************************************
	***************************/

VALUE Primitive (POSITION position)
{if (*GenerateMoves(position) != NULL){
   return (undecided);}
  else{ 
   if (*GenerateMoves(position) == NULL){ 
   return (lose);}
  else 
    {return (win);}}}





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
   generic_unhash(position, gBoard);
   printf("     %c\n",gBoard[2]);
   printf("  %c  |  %c\n", gBoard[1], gBoard[3]);
   printf("   \\ | /\n");
   printf(" %c-- %c --%c\n", gBoard[4], gBoard[0], gBoard[5]);
   printf("   / | \\\n");
   printf("  %c  |  %c\n", gBoard[6], gBoard[8]);
   printf("     %c\n", gBoard[7]);
   printf("\n%s's Turn\n", playersName);
   
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

void PrintComputersMove (MOVE computersMove, STRING computersName) {
   printf("%8s's move               : %2d\n", computersName, move_to(computersMove));

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
   printf("moved to %d from %d", move_to(move), move_froom(move));    
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

USERINPUT GetAndPrintPlayersMove (POSITION position, MOVE *move, STRING playersName) {
     USERINPUT input;
     USERINPUT HandleDefaultTextInput();
     
     for (;;) {
         /***********************************************************
          * CHANGE THE LINE BELOW TO MATCH YOUR MOVE FORMAT
          ***********************************************************/
	printf("%8s's move [(undo)/(MOVE FORMAT)] : ", playersName);
	
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
{unsigned int theInput = atoi(input);
  return (theInput <= 87 && theInput >= 01);} 


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
     return ((MOVE) make_move(input[0], input[1])); }


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

void SetTclCGameSpecificOptions (int options[]) {
     
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


