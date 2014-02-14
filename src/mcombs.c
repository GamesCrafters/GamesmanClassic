#include <stdio.h>
#include <ctype.h>
#include "gamesman.h"
#include <math.h>

/*******************************************************************************
*
* A solver for Combinations, using the GamesmanClassic libraries
* 
* Gameplay:
*   Players go back and forth picking numbers between 1 and 51 (inclusive)
*   When picking a number, a player is confined to only numbers which are NOT
*   linear combinations of numbers already picked.
*   I.e. if Player1 picks 11 and Player2 picks 7, then Player1 now cannot pick
*   any number 'x' such that (k*11)+(n*7)=x for all non-negative whole numbers,
*   'k' and 'n'. Player1 may choose to pick 13, in which case Player2 now must
*   pick some number 'y' s.t. y!=(n*7)+(k*11)+(i*13) for all non-negative
*   whole numbers, i, k, and n.
*
*   The game ends when a player picks the number 1.  This player loses.
*
*******************************************************************************/

int MAX_UPPER = 16;
int MIN_UPPER = 2;
int upper = 16;

/*
*
* POSITION state representation:
*   POSITION is a 64-bit (minimum) unsigned long long.  The lower order 16 bits
*   will be used to store whether or not the corresponding number is available
*   to be picked.  A 0 in a bit location indicates that the corresponding number
*   is open, having not been picked nor a linear combination of anything that 
*   has been picked.
*/
POSITION gNumberOfPositions = 65536;
POSITION gInitialPosition = 0;
POSITION gMinimalPosition = 0;
//POSITION gMinimalPosition = (1<<(ROWS*3)) - 1 ;

BOOLEAN kPartizan              = FALSE;
BOOLEAN kDebugMenu             = FALSE;
BOOLEAN kGameSpecificMenu      = TRUE;
BOOLEAN kTieIsPossible         = FALSE;
BOOLEAN kLoopy                 = FALSE;
BOOLEAN kDebugDetermineValue   = FALSE;
STRING kGameName              = "Combinations";
POSITION kBadPosition           = -1;
void*    gGameSpecificTclInit   = NULL;

STRING kHelpGraphicInterface  = "Help Graphic Interface: Fill in later";
STRING kHelpTextInterface     = "Help Text Interface: Fill in later";
STRING kHelpOnYourTurn        = "Help on your turn";
STRING kHelpStandardObjective = "help standard objective";
STRING kHelpReverseObjective  = "reverse objective";
STRING kHelpTieOccursWhen     = "A tie is not possible in this game";
STRING kHelpExample           = "some really long thing<-Actually play a game,then copy/paste";
STRING kAuthorName            = "Nicholas Herson";


STRING MoveToString(MOVE);

void InitializeGame()
{
	// HERE, YOU SHOULD ASSIGN gNumberOfPositions and gInitialPosition
	gNumberOfPositions = pow(2, upper) ;
	gInitialPosition = 0; // nothing has been picked

	// gInitialPosition = ((1<<(rows*3)) - 1) * 2;
	gMinimalPosition = gInitialPosition;

	gMoveToStringFunPtr = &MoveToString;
	//gGetSEvalCustomFnPtr = &getSEvalCustomFnPtr;
	//gCustomTraits = CUSTOM_TRAITS;
}

// Not sure what this is supposed to do
void FreeGame()
{
}

// Probably not going to implement this
void DebugMenu()
{
}

// Do we have any game options to be set?
// Ask for them here
// Interesting Function calls to note:
//    1. char GetMyChar()
//    2. int GetMyInt()
// Eventually support picking range of values
// for now, this is stubbed out.
void GameSpecificMenu()
{/*
	char option;
	int new_range;
	do
	{
		printf("Currently you can pick from 1 to %d. You can change the upper limit to between %d and %d\n", upper, MIN_UPPER, MAX_UPPER);
		printf("Would you like to change it [y/n]? ");
		option = GetMyChar();
		option = toupper(option);
		if(option == 'Y')
		{
			do
			{
				printf("Enter new upper number [%d-%d] : ", MIN_UPPER, MAX_UPPER);
				new_range = GetMyInt();
			} while(new_range > MAX_UPPER || new_range < MIN_UPPER);
			range = new_range;
		}
		else if(option != 'N')
		{
			printf("Invalid option. Please try again.\n");
		}
	} while(option != 'N');*/
  printf("Not yet implemented. Sorry.");
}

// Probably don't need to write this
void SetTclCGameSpecificOptions(int theOptions [])
{
}

// just a nice maxing function
#define max(a,b) (((a)>(b)) ? (a) : (b))

// Take a POSITION and a MOVE and generate
// the next POSITION
// HARD
POSITION DoMove(POSITION thePosition, MOVE theMove)
{
  // not implemented
  // need to flip the bit of the selected number
  // and also flip the bits of all numbers that are now
  // invalid
  // For now, just inserts a 1 into the POSITION
  // at the position of the picked number
  unsigned long long mask = 1;
  //int* pickedSoFar = malloc((upper+1)*sizeof(int));
  //memset(pickedSoFar, 0, (upper+1)*4);
  //int pickedIndex = 0;
  // first, figure out what's been picked
  /*for (int i=0; i < upper; i++)
  {
    if ((mask << i) && thePosition)
    {
      // else, pickedSoFar[i] will remain 0
      pickedSoFar[pickedIndex] = i+1;
      pickedIndex++;
    }
  }*/
  // ELIMINATE THE MOVE VALUE
  thePosition = thePosition | (mask << (theMove - 1));
  POSITION moveCopy = thePosition;
  //pickedSoFar[pickedIndex] = theMove;
  //ELIMINATE THE NOW-INVALID MOVES
  for (int i=theMove+1; i<=upper; i++)
  {
    if (isLinearCombination(moveCopy, i, 0))
    {
      thePosition = thePosition | (mask << (i-1));
    }
  }
	return thePosition;
}

POSITION GetInitialPosition()
{
	return gInitialPosition;
}

// Print out a computer's move
void PrintComputersMove(MOVE computersMove, STRING computersName)
{
	printf("%s's pick: %d\n", computersName, computersMove);
}

// SUNIL: DONE
VALUE Primitive(POSITION position)
{
  // losing position if the 
  // first bit is on (1 is picked)
  if ((position & 0x6) == 0x6)
      return lose;
	/*if (position % 2 == 1)*/
		/*return win;*/

	return undecided;
}

// Print out a visual representation of the position
void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn)
{
  printf("TURN: %s\n", playerName);
  printf("UNAVAILABLE NUMBERS: ");
  int mask = 1;
  for (int i=1; i<upper; i++) 
  {
    if (position & (mask << i)) 
    {
      printf("%d ", i+1);
    }
  }
  printf("\nAVAILABLE NUMBERS: ");
  for (int i=1; i<upper; i++)
  {
    if (!(position & (mask << i))) 
    {
      printf("%d ", i+1);
    }
  }
  printf("\n");
}

// Generate a list of next moves
MOVELIST *GenerateMoves(POSITION position)
{
	// Here, use head = CreateMovelistNode(move,head) ;
	// then return head when done
	MOVELIST *CreateMovelistNode(), *head = NULL;
	int i = 1;
  unsigned long long mask = 1;
	if(Primitive(position) == undecided)
	{
    while (i < upper)
    {
      if (((mask << i) & position) == 0)
      {
        head = CreateMovelistNode( (MOVE) i+1, head);
      }
      i++;
    }
	}
	return head;
}

// returns 1 iff num is a linear combination of the bits flipped
// on in picks.  Note that picks is 1-indexed.
int isLinearCombination(POSITION picks, int num, int soFar)
{
  // base case 0
  if(soFar > num)
  {
    return 0;
  }
  // base case 1
  else if(soFar == num)
  {
    return 1;
  }
  int mask = 1;
  // recursively see if adding to soFar finds a linear combination of num
  for(int i=0; i<upper; i++)
  {
    // immediately stop searching if we find a linear combination
    if (picks & (mask << i))
    {
      if(isLinearCombination(picks, num, soFar + i+1))
      {
        return 1;
      }
    }
  }
  // couldn't find a linear combination down the search path, return false
  return 0;
}


USERINPUT GetAndPrintPlayersMove(POSITION thePosition, MOVE* theMove, STRING playerName)
{
	USERINPUT ret;
	do
	{
		printf("%8s's move [1-%d] :  ", playerName, upper);
		ret = HandleDefaultTextInput(thePosition, theMove, playerName);
		if(ret != Continue)
			return(ret);
	} while(TRUE);
	return(Continue);
}

BOOLEAN ValidTextInput(STRING input)
{
	int i;
	for(i = 0; i < strlen(input); i++)
		if(!isdigit(input[i]))
			return FALSE;
	return TRUE;
}

MOVE ConvertTextInputToMove(STRING input)
{
	int ret;
	sscanf(input, "%d", &ret);
	return ret;
}

void PrintMove(MOVE theMove)
{
	STRING m = MoveToString( theMove );
	printf( "%s", m );
	SafeFree( m );
}

STRING MoveToString (theMove)
MOVE theMove;
{
	STRING move = (STRING) SafeMalloc(4);
	sprintf(move, "%d", theMove);
	return move;
}

STRING kDBName = "combinations";

int NumberOfOptions()
{
	return 2*(MAX_UPPER-MIN_UPPER+1);
}

int getOption()
{
	int option = 1;
	if (gStandardGame) option += 1;
	option += (upper-MIN_UPPER) * 2;
	return option;
}

void setOption(int option)
{
	option--;
	gStandardGame = option%2==1;
	//upper = option/2%(MAX_ROWS-MIN_ROWS+1)+MIN_ROWS;
}

POSITION StringToPosition(char* board) {
	// FIXME: this is just a stub
	return atoi(board);
}


char* PositionToString(POSITION pos) {
	// FIXME: this is just a stub
	return "Implement Me";
}

char * PositionToEndData(POSITION pos) {
	return NULL;
}
