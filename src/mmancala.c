/* mmancala.c */

/** Game module for the game Mancala implemented to work with
 *  Dan Garcia's Gamesman game solving/playing system.
 *
 *  Authors:
 *  Spencer Ray and Keith Ho (Spring 2003)
 *
 *  NOTES/ERRATA:
 *  Currently supports a total of 80
 *  different variants to the original rules of the game.
 *  The addition of having multiple turns is ready to be added
 *  but awaits the pending change to the Gamesman system.
 *  The game also features the ability to play with varying
 *  board sizes and piece/stone counts.  The problem lies in the
 *  fact that Gamesman MUST reinitialize the game internally
 *  with every change to the board's configuration.  That is also
 *  another detail that Gamesman itself must change before the
 *  ability to change board configuration dynamically is able
 *  to work.  As of now, you can change the global variables
 *  boardSize and numOfPieces in order to play with different
 *  board configurations, but you would also have to recompile.
 *  However, the module IS compatible with such changes, and the
 *  game WILL work with different configurations.  The only
 *  problem is that you can't change it dynamically through the
 *  game specific menu.  Once Gamesman is changed so that re-
 *  initialization is possible, no changes will be necessary to
 *  this module, because it already supports the ability to change.
 */

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "gamesman.h"

int *array_hash (POSITION position);
POSITION array_unhash (int *hashed);

/* DEBUGGING = 0/1 = NO/YES debug statements printed onscreen */
int DEBUGGING = 0;

POSITION gNumberOfPositions;
POSITION gInitialPosition;

BOOLEAN kPartizan            = TRUE;
BOOLEAN kDebugMenu           = TRUE;
BOOLEAN kGameSpecificMenu    = TRUE;
BOOLEAN kTieIsPossible       = TRUE;
BOOLEAN kLoopy               = FALSE;
BOOLEAN kDebugDetermineValue = FALSE;
STRING kAuthorName          = "Spencer Ray and Keith Ho";
STRING kGameName            = "Mancala";
STRING kDBName              = "mancala";
POSITION kBadPosition         = -1;

TIER BoardToTier(int* board);
void SetupTierStuff();
STRING TierToString(TIER tier);
TIERLIST* TierChildren(TIER tier);
TIERPOSITION NumberOfTierPositions(TIER tier);
int* ToTierArrayBoard(int *ArrayBoard);
int* ToArrayBoard(int *TierArrayBoard, TIER tierNum);

STRING kHelpGraphicInterface = "";

STRING kHelpTextInterface    =
        "On your turn, use the numbers next to 'P1 Bin #' or 'P2 Bin #' to choose\n\
a bin from which to move your stones. Player 1 (P1) chooses from the top \n\
row, and Player 2 (P2) chooses from the bottom. The brackets [ ] represent\n\
a bin on the board. The number in the brackets represent the number of \n\
stones in that bin. The far left and far right bins are the mancalas.\n\
The far left mancala belongs to P1 and the other to P2. You cannot move \n\
from a mancala."                                                                                                                                                                                                                                                                                                                                                                                                                                                                                ;

STRING kHelpOnYourTurn =
        "Choose one of your bins. Player 1 chooses from the top, player 2 chooses\n\
from the bottom. This will disperse your stones counterclockwise around \n\
the board from that bin. The bin you choose must NOT be empty, and you \n\
CANNOT choose either mancala (the far left or right bin)."                                                                                                                                                                                                                                            ;

STRING kHelpStandardObjective =
        "To play until all the stones are contained in the two mancalas and no \n\
more moves are possible. The player whose mancala contains more stones wins."                                                                                   ;

STRING kHelpReverseObjective =
        "To play until all the stones are contained in the two mancalas and no \n\
more moves are possible. The player whose mancala contains fewer stones wins."                                                                                   ;

STRING kHelpTieOccursWhen =
        "both mancalas contain an equal number of stones at the end of the game.";

STRING kHelpExample = "\
P2 Bin #         1       2       3		\n\
                [4]     [4]     [4]		\n\
        [0]                             [0]	\n\
                [4]     [4]     [4]		\n\
P1 Bin #         5       6       7		\n\n\
Dan Garcia's move [5-7] :  6			\n\n\
P2 Bin #         1       2       3		\n\
                [4]     [5]     [5]		\n\
        [0]                             [1]	\n\
                [4]     [0]     [5]		\n\
P1 Bin #         5       6       7		\n\n\
Computer's move: 3				\n\n\
P2 Bin #         1       2       3		\n\
                [5]     [6]     [0]		\n\
        [1]                             [1]	\n\
                [5]     [1]     [5]		\n\
P1 Bin #         5       6       7		\n\n\
Dan Garcia's move [5-7] :  7			\n\n\
P2 Bin #         1       2       3		\n\
                [6]     [7]     [1]		\n\
        [1]                             [2]	\n\
                [6]     [1]     [0]		\n\
P1 Bin #         5       6       7		\n\n\
Computer's move: 1				\n\n\
P2 Bin #         1       2       3		\n\
                [0]     [8]     [2]		\n\
        [2]                             [2]	\n\
                [7]     [2]     [1]		\n\
P1 Bin #         5       6       7		\n\n\
Dan Garcia's move [5-7] :  6			\n\n\
P2 Bin #         1       2       3		\n\
                [0]     [8]     [2]		\n\
        [2]                             [3]	\n\
                [7]     [0]     [2]		\n\
P1 Bin #         5       6       7		\n\n\
Computer's move: 3				\n\n\
P2 Bin #         1       2       3		\n\
                [0]     [9]     [0]		\n\
        [10]                            [3]	\n\
                [0]     [0]     [2]		\n\
P1 Bin #         5       6       7		\n\n\
Dan Garcia's move [5-7] :  7			\n\n\
P2 Bin #         1       2       3		\n\
                [0]     [9]     [1]		\n\
        [10]                            [4]	\n\
                [0]     [0]     [0]		\n\
P1 Bin #         5       6       7		\n\n\
Computer's move: 3				\n\n\
P2 Bin #         1       2       3		\n\
                [0]     [0]     [0]		\n\
        [20]                            [4]	\n\
                [0]     [0]     [0]		\n\
P1 Bin #         5       6       7		\n\n\
Computer wins. Nice try, Dan.";


/****** START added specifically for mancala */

int boardSize = 8;    /* default values allow 3 bins/player and 4 pieces/bin */
int numOfPieces = 24;
int mancalaL;         /* mancala on the left side */
int mancalaR;         /* mancala on the right (default as p1's)*/
int turn;             /* 0 = player 1's turn, 1 = player 2's turn */
int rsize;
int turnOffset;

BOOLEAN OPT_NOCAPTURE = FALSE;  /* Landing on your own empty bin does what?
                                   0: Capture YOUR and OPPONENT's stones
                                   1: YOUR turn ends. */
BOOLEAN OPT_CHAIN = FALSE;      /* Landing on a non-empty bin does what?
                                   0: Nothing
                                   1: Chain-despersal */
BOOLEAN OPT_MOVEOPP = FALSE;    /* From which bins can you move?
                                   0: YOUR own side
                                   1: YOUR own or OPPONENT's side */
BOOLEAN OPT_NOEXTRATURN = TRUE; /* Landing in your mancala does what?
                                   0: YOU go again
                                   1: Nothing */
int OPT_WINEMPTY = 0;           /* Emptying your side of the board does what?
                                   0: OPPONENT captures all stones in his bins at
                                   the end of YOUR turn
                                   1: OPPONENT captures all stones in his bins at
                                   the start of YOUR next turn
                                   2: can chose to disperse stones from OPPONENT'S
                                   bins
                                   3: YOU win
                                   4: YOU pass */



/****** END added specifically for mancala */

BOOLEAN rearranger_unhash(int, char*);
int rearranger_hash(char*);
void my_nCr_init(int);
int my_nCr(int, int);
void UpdateGameSpecs();
void BadMenuChoice();
int rearranger_hash_init(int, int, int);

STRING MoveToString(MOVE);

/************************************************************************
**
** NAME:        InitializeGame
**
** DESCRIPTION: Initialize mancala-specific globals as well as
**              gNumberOfPositions, gInitialPosition.
**              Also initializes the rearranger_hash methods necessary
**              for this game's hash routines.
**
************************************************************************/

void InitializeGame()
{
	int i, piecesPerBin, *arrayBoard;
	arrayBoard = (int *) SafeMalloc ((boardSize+1) * sizeof (int));
	UpdateGameSpecs();
	generic_hash_destroy();
	SetupTierStuff();
	piecesPerBin = numOfPieces / (boardSize - 2);
	for(i = 0; i < boardSize; i += 1) {
		arrayBoard[i] = piecesPerBin;
	}
	arrayBoard[mancalaL] = 0;
	arrayBoard[mancalaR] = 0;
	arrayBoard[turn] = 0;

	/* this is the max number the rearrangerHash can generate
	   given the limitations of the 32-bit cpu 2 * (31 my_nCr 15) */
	//gNumberOfPositions = 601080390;

	gInitialPosition = array_unhash(arrayBoard);
	//int z;
	//  for(z = 0;z < boardSize + 1; z++)
	// printf("slot %d: %d\n",z,arrayBoard[z]);

	SafeFree (arrayBoard);

	gMoveToStringFunPtr = MoveToString;

}


void SetupTierStuff() {
	int mancL, mancR;
	//TIERPOSITION tierPos, POSITION initialPos;
	// kSupportsTierGamesman
	kSupportsTierGamesman = TRUE;
	// function pointers
	gTierChildrenFunPtr = &TierChildren;
	gNumberOfTierPositionsFunPtr = &NumberOfTierPositions;
	gTierToStringFunPtr = &TierToString;

	// hashes

	generic_hash_custom_context_mode(TRUE);
	// Tier-Specific Hashes
	int piecesArray[] = {'o', numOfPieces, numOfPieces, 'x', boardSize-2-1, boardSize-2-1, -1};
	//'o' is the pieces that are not in a mancala, 'x' is a bin excluding the two mancalas
	for(mancL = 0; mancL <= numOfPieces; mancL++) {
		for(mancR = 0; mancR <= (numOfPieces-mancL); mancR++) {
			piecesArray[1] = numOfPieces-mancL-mancR;
			piecesArray[2] = numOfPieces-mancL-mancR;
			generic_hash_init(boardSize-2+numOfPieces-mancL-mancR-1, piecesArray, NULL, 0);
			generic_hash_set_context(10000*mancL+mancR);
		}
	}

	int i, piecesPerBin, *arrayBoard;
	arrayBoard = (int *) SafeMalloc ((boardSize+1) * sizeof (int));
	piecesPerBin = numOfPieces / (boardSize - 2);
	for(i = 0; i < boardSize; i += 1) {
		arrayBoard[i] = piecesPerBin;
	}
	arrayBoard[mancalaL] = 0;
	arrayBoard[mancalaR] = 0;
	arrayBoard[turn] = 0;

	gInitialTier = 0; //0 pieces in the mancala bins
	generic_hash_context_switch(gInitialTier);

	int array_size = boardSize - 2 + numOfPieces - 1;
	int j = 0;
	char *dest = (char *) SafeMalloc (array_size * sizeof(char));

	for (i = 0; i < array_size; i++) {
		if(j ==  mancalaL || j == mancalaR)
			j++;
		if (arrayBoard[j] > 0) {
			dest[i] = 'o';
			arrayBoard[j] -= 1;
		}
		else {
			dest[i] = 'x';
			j += 1;
		}
	}

	gInitialTierPosition = generic_hash_hash(dest, 1);
	/*generic_hash_context_switch(gInitialTier);
	      tierPos = generic_hash_hash(dest, hashed[turn]+1);
	   generic_hash_context_switch(tier);
	   result = gHashToWindowPosition(tierPos, tier);
	 */

	/*
	   for(i = 0;i < array_size; i ++)
	   printf("%c",dest[i]);
	   printf("above is arrray board before hash\n");
	 */
	SafeFree(dest);
	SafeFree(arrayBoard);



}

TIERLIST* TierChildren(TIER tier)
{
	int i,j;
	TIERLIST* list = NULL;
	list = CreateTierlistNode(tier, list);

	for(i=0; i <= numOfPieces - (tier/10000)- (tier%10000); i++) {
		for(j=0; j <= numOfPieces - (tier/10000) - (tier%10000) - i; j++) {
			if(i != 0 || j!= 0)
				list = CreateTierlistNode(tier+(10000*i)+j,list);
		}
	}
	return list;
	/*if(tier%100 + tier/100 == numOfPieces)
	   return list;
	   else {
	   for(i=1;i <=numOfPieces-(tier/100)-(tier%100); i++) {
	    list = CreateTierlistNode(tier+i, list);
	    list = CreateTierlistNode(tier+(100*i),list);
	   }
	   return list;
	   }*/
}

TIER BoardToTier(int* board)
{
	return 10000*board[mancalaL] + board[mancalaR];
}

TIERPOSITION NumberOfTierPositions(TIER tier)
{
	generic_hash_context_switch(tier);
	//restore old context?
	return generic_hash_max_pos();
}

STRING TierToString(TIER tier) {
	STRING tierStr = (STRING) SafeMalloc(sizeof(char)*100);
	sprintf(tierStr,
	        "%llu Pieces In Left Mancala, %llu Pieces in Right Mancala",
	        tier / 10000,
	        tier % 10000);
	return tierStr;
}



/************************************************************************
**
** NAME:        UpdateGameSpecs
**
** DESCRIPTION: Working with the preset global variables boardSize and
**              numOfPieces, UpdateGameSpecs updates the other essential
**              global variables for mancala such as rsize, turnOffset,
**              mancalaL, mancalaR, and turn.
**              This method also re-initializes the rearranger_hash
**              function to work properly with the new board specification.
**
************************************************************************/

void UpdateGameSpecs()
{
	int i, j, done, tBig, tSmall, tEnd, tArray[32];
	mancalaL = 0;
	mancalaR = boardSize / 2;
	turn = boardSize;
	rsize = boardSize + numOfPieces - 1;

	rearranger_hash_init(rsize, numOfPieces, boardSize - 1);

	/* calculate the turnOffset = rsize! / (tSmall! * tBig!) */
	if(numOfPieces >= boardSize - 1) {
		tBig = numOfPieces;
		tSmall = boardSize - 1;
	}
	else {
		tBig = boardSize - 1;
		tSmall = numOfPieces;
	}

	/* initialize the array to store each factor of (rsize!/tBig!)  */
	for(i = rsize, j = 0; i > tBig; tArray[j] = i, j++, i--) ;
	tArray[j] = -1;
	tArray[++j] = -1;
	tEnd = j;

	/* try to simplify the factors in tempArray by dividing them by
	   each factor of tSmall! */
	for(; tSmall > 1; tSmall--) {
		/* try to replace each factor in tArray with a smaller one */
		for(i = 0, done = 0; tArray[i] != -1 && done == 0; i++) {
			if(tArray[i] % tSmall == 0) {
				tArray[i] /= tSmall;
				done = 1;
			}
		}
		/* if tSmall couldn't divide into any entries, then must add tSmall
		   as a new entry in the array */
		if(done == 0) {
			tArray[tEnd] = tSmall;
			tArray[++tEnd] = -1;
		}
	}
	/* now multiply out the remaining factors for the result */
	for(i = 0, turnOffset = 1; tArray[i] != -1; i++) {
		turnOffset *= tArray[i];
	}
	/* and divide the remaining factors */
	for(i += 1; tArray[i] != -1; i++) {
		turnOffset /= tArray[i];
	}

	gNumberOfPositions = turnOffset*2;

	if(DEBUGGING) printf("\nGAME INFO\nboardSize: %d\nnumOfPieces: %d\nturnOffset: %d\n"
		             "gNumberOfPositions: " POSITION_FORMAT "\n", boardSize,
		             numOfPieces, turnOffset, gNumberOfPositions);
}

/************************************************************************
**
** NAME:        FreeGame
**
** DESCRIPTION:
**
************************************************************************/

void FreeGame()
{
}


/************************************************************************
**
** NAME:        DebugMenu
**
** DESCRIPTION:
**
************************************************************************/

void DebugMenu()
{
}


/************************************************************************
**
** NAME:        GameSpecificMenu
**
** DESCRIPTION:
**
************************************************************************/

void GameSpecificMenu()
{
	int temp = 0, temp2 = 0;
	do {
		printf("\n\t----- Game Specific options for %s -----\n\n", kGameName);

		printf("\n\tCurrent game configuration: \n");

		if(gStandardGame == TRUE) {
			printf("\n\tG)\tChange (G)ame type FROM standard TO misere\n");
		}
		else {
			printf("\n\tG)\tChange (G)ame type FROM misere TO standard\n");
		}

		if(OPT_NOCAPTURE == 0) {
			printf("\n\tE)\tChange landing in an (E)mpty bin rule\n\t\tFROM capturing yours and opponent's stones TO your turn just ending\n");
		}
		else {
			printf("\n\tE)\tChange landing in an (E)mpty bin rule\n\t\tFROM your turn just ending TO capturing yours and opponent's stones\n");
		}

		if(OPT_CHAIN == 0) {
			printf("\n\tN)\tChange landing in a (N)on-empty bin rule\n\t\tFROM doing nothing TO chaining the dispersal\n");
		}
		else {
			printf("\n\tN)\tChange landing in a (N)on-empty bin rule\n\t\tFROM chaining the dispersal TO doing nothing\n");
		}

		if(OPT_MOVEOPP == 0) {
			printf("\n\tS)\tChange (S)electable bins rule\n\t\tFROM just your own TO any\n");
		}
		else {
			printf("\n\tS)\tChange (S)electable bins rule\n\t\tFROM any TO just your own\n");
		}

		if(OPT_NOEXTRATURN == 0) {
			printf("\n\tL)\tChange (L)anding in own kalaha rule\n\t\tFROM getting an extra turn TO doing nothing\n\t\t(CURRENTLY NOT SUPPORTED - NO EFFECT)\n");
		}
		else {
			printf("\n\tL)\tChange (L)anding in own kalaha rule\n\t\tFROM doing nothing TO getting an extra turn\n\t\t(CURRENTLY NOT SUPPORTED - NO EFFECT)\n");
		}

		switch(OPT_WINEMPTY) {
		case 0:
			printf("\n\tY)\tChange emptying (Y)our side of the board rule\n\t\tFROM your opponent capturing all his stones at the end of your turn\n\t\tTO your opponent capturing all his stones at the start of your next turn\n");
			break;
		case 1:
			printf("\n\tY)\tChange emptying (Y)our side of the board rule\n\t\tFROM your opponent capturing all his stones at the start of your next turn\n\t\tTO choosing to disperse stones from your opponent's bins\n");
			break;
		case 2:
			printf("\n\tY)\tChange emptying (Y)our side of the board rule\n\t\tFROM choosing to disperse stones from your opponent's bins\n\t\tTO you winning at the start of your next turn\n");
			break;
		case 3:
			printf("\n\tY)\tChange emptying (Y)our side of the board rule\n\t\tFROM you winning at the start of your next turn\n\t\tTO you passing\n");
			break;
		case 4:
			printf("\n\tY)\tChange emptying (Y)our side of the board rule\n\t\tFROM you passing\n\t\tTO your opponent capturing all his stones at the end of your turn\n");
		}

		printf("\n\tT)\tChange (T)he number of bins per player\n\t\t(CURRENTLY NOT SUPPORTED - CRASHES IF OPTION IS MODIFIED)\n");
		printf("\tP)\tChange number of (P)ieces per bin\n\t\t(CURRENTLY NOT SUPPORTED - CRASHES IF OPTION IS MODIFIED)\n");
		printf("\n\tB)\t(B)ack = Return to previous activity.\n");
		printf("\n\nSelect an option: ");

		switch(GetMyChar())
		{
		case 'G': case 'g':
			gStandardGame = !gStandardGame;
			break;
		case 'E': case 'e':
			OPT_NOCAPTURE = !OPT_NOCAPTURE;
			break;
		case 'N': case 'n':
			OPT_CHAIN = !OPT_CHAIN;
			break;
		case 'S': case 's':
			OPT_MOVEOPP = !OPT_MOVEOPP;
			break;
		case 'L': case 'l':
			OPT_NOEXTRATURN = !OPT_NOEXTRATURN;
			break;
		case 'Y': case 'y':
			OPT_WINEMPTY = (OPT_WINEMPTY += 1) > 4 ? 0 : OPT_WINEMPTY;
			break;
		case 'Q': case 'q':
			ExitStageRight();
			break;

		case 'T': case 't':
			printf( "Enter #N bins/player (N > 0): " );
			/*fflush(stdin);	no longer needed
			   scanf( "%d", &temp );*/
			temp = GetMyInt();
			temp2 = numOfPieces / (boardSize - 2);
			temp2 *= (temp * 2);
			temp *= 2;
			temp += 2;
			if(temp + temp2 <= 32) {
				boardSize = temp;
				numOfPieces = temp2;
				UpdateGameSpecs();
			}
			else {
				printf( "\nERROR: (#bins + #pieces + 2) > 32!!!\n" );
			}
			break;

		case 'P': case 'p':
			printf( "Enter #N pieces/bin (N > 0): " );
			/*fflush(stdin);		no longer needed
			   scanf( "%d", &temp );*/
			temp *= (boardSize - 2);
			if(temp + boardSize <= 32) {
				numOfPieces = temp;
				UpdateGameSpecs();
			}
			else {
				printf( "\nERROR: (#bins + #pieces + 2) > 32!!!\n" );
			}
			break;

		case 'B': case 'b':
			return;
		default:
			BadMenuChoice();
			HitAnyKeyToContinue();
			break;
		}

		/*fflush(stdin); no longer needed */
	} while(TRUE);
}


/************************************************************************
**
** NAME:        SetTclGameSpecificOptions
**
** DESCRIPTION:
**
************************************************************************/

void SetTclCGameSpecificOptions(int theOptions [])
{
}


/************************************************************************
**
** NAME:        DoMove
**
** DESCRIPTION: Unhashes the current position into a board configuration.
**              Then applies the move by dispersing the stones into the
**              proper bins.  Rehashes the new board configuration and
**              return the new position.
**
************************************************************************/

POSITION DoMove(POSITION thePosition, MOVE theMove)
{
	int *arrayHashedBoard;
	int i, startBin, done;
	int numPieces, t, sumTop, sumBottom, j, k;
	POSITION newPosition;

	arrayHashedBoard = array_hash(thePosition);

	t = arrayHashedBoard[turn];

	/* on my turn starting, i'm empty -> i pass */
	if(theMove == 0) {
		arrayHashedBoard[turn] = !arrayHashedBoard[turn];
		newPosition = array_unhash(arrayHashedBoard);
		SafeFree(arrayHashedBoard);
		return newPosition;
	}

	/* disperse stones, do chained-dispersal if option is set */
	startBin = theMove;
	numPieces = arrayHashedBoard[startBin];
	i = theMove - 1;
	do {
		for(; numPieces > 0; i -= 1, numPieces -= 1) {
			if(i < mancalaL) {
				i = boardSize - 1;
			}
			if((t == 0 && i == mancalaL) || (t == 1 && i == mancalaR)) {
				numPieces += 1;
			}
			else {
				arrayHashedBoard[i] += 1;
				arrayHashedBoard[startBin] -= 1;
			}
		}
		if((i + 1) != mancalaL && (i + 1) != mancalaR) {
			numPieces = arrayHashedBoard[i + 1];
			startBin = i + 1;
		}
	} while(OPT_CHAIN == 1 && numPieces > 1);

	i += 1;
	/* landed on one of your own empty bins -> capture your's and opponent's stones */
	if(OPT_NOCAPTURE == 0) {
		if ( t == 0 && (i > mancalaR && i < boardSize) && arrayHashedBoard[i] == 1) {
			arrayHashedBoard[mancalaR] += (arrayHashedBoard[i] + arrayHashedBoard[boardSize - i]);
			arrayHashedBoard[i] = 0;
			arrayHashedBoard[boardSize - i] = 0;
		}
		if ( t == 1 && (i > mancalaL && i < mancalaR) && arrayHashedBoard[i] == 1) {
			arrayHashedBoard[mancalaL] += (arrayHashedBoard[i] + arrayHashedBoard[boardSize - i]);
			arrayHashedBoard[i] = 0;
			arrayHashedBoard[boardSize - i] = 0;
		}
	}

	/* check if the game is already finished from the dispersals */
	done = 0;
	if(arrayHashedBoard[mancalaL] +
	   arrayHashedBoard[mancalaR] == numOfPieces) {
		done = 1;
	}

	/* sum up the top and bottom rows of bins */
	for(j = mancalaL + 1, k = mancalaR + 1, sumTop = 0, sumBottom = 0;
	    j < mancalaR && k < boardSize;
	    sumTop += arrayHashedBoard[j], sumBottom += arrayHashedBoard[k], j += 1, k += 1) ;

	/* on my turn ending, i'm empty so my opponent gets all the stones in his bins */
	if(OPT_WINEMPTY == 0 && !done) {
		if(t == 0 && sumBottom == 0) {
			for(j = mancalaL + 1; j < mancalaR; arrayHashedBoard[j] = 0, j += 1) ;
			arrayHashedBoard[mancalaL] += sumTop;
		}
		else if(t == 1 && sumTop == 0) {
			for(k = mancalaR + 1; k < boardSize; arrayHashedBoard[k] = 0, k += 1) ;
			arrayHashedBoard[mancalaR] += sumBottom;
		}
	}
	/* on my turn starting, i'm empty so my opponent gets all the stones in his bins */
	else if(OPT_WINEMPTY == 1 && !done) {
		if(t == 0 && sumTop == 0) {
			for(k = mancalaR + 1; k < boardSize; arrayHashedBoard[k] = 0, k += 1) ;
			arrayHashedBoard[mancalaR] += sumBottom;
		}
		else if(t == 1 && sumBottom == 0) {
			for(j = mancalaL + 1; j < mancalaR; arrayHashedBoard[j] = 0, j += 1) ;
			arrayHashedBoard[mancalaL] += sumTop;
		}
	}
	/* on my turn starting, i'm empty -> i win */
	else if(OPT_WINEMPTY == 3 && !done) {
		if(t == 0 && sumTop == 0) {
			for(k = mancalaR + 1; k < boardSize; arrayHashedBoard[k] = 0, k += 1) ;
			arrayHashedBoard[mancalaL] = numOfPieces - 1;
			arrayHashedBoard[mancalaR] = 1;
		}
		else if(t == 1 && sumBottom == 0) {
			for(j = mancalaL + 1; j < mancalaR; arrayHashedBoard[j] = 0, j += 1) ;
			arrayHashedBoard[mancalaR] = numOfPieces - 1;
			arrayHashedBoard[mancalaL] = 1;
		}
	}

	arrayHashedBoard[turn] = !arrayHashedBoard[turn];
	newPosition = array_unhash(arrayHashedBoard);

	SafeFree(arrayHashedBoard);
	return newPosition;
}


/************************************************************************
**
** NAME:        GetInitialPosition
**
** DESCRIPTION:
**
************************************************************************/

POSITION GetInitialPosition()
{
	int i = 0;
	int result = 0;
	signed char c;
	int temp = 0;
	int *boardArray = (int *) SafeMalloc (boardSize * sizeof (int));

	printf("\n\n\t----- Get Initial Position -----\n");
	printf("\nPlease input the position to begin with.");
	printf("\nEnter %d numbers, each separated by a comma.", boardSize);
	printf("\nThese are the number of stones per bin, clockwise from the left mancala. ");
	printf("\nThe first %d numbers are player 2's bins from left to right.", boardSize / 2);
	printf("\nThe first %d numbers are player 1's bins from right to left.", boardSize / 2);
	printf("\n\nExample: 0,1,2,3,4,5,6,7,1\n");
	printf("\n     [1] [2] [3]   ");
	printf("\n  [0]           [4]");
	printf("\n     [7] [6] [5]   ");
	printf("\n  Player 1's turn  ");
	printf("\n\n Enter position: ");

	getchar();
	while (i < boardSize && (c = getchar()) != EOF) {
		if (c >= '0' && c <= '9') {
			(temp == 0) ? (temp = c - '0') : (temp += (c - '0') * 10);
		}
		else if (c == ',' || c == 13 || c== 10) {
			boardArray[i] = temp;
			i += 1;
			temp = 0;
		}
	}

	printf("\nNow, which player's turn is it? [1/2]: ");
	c = getchar();
	if (c == '1') {
		boardArray[turn] = 0;
	} else boardArray[turn] = 1;

	result = array_unhash(boardArray);
	SafeFree (boardArray);
	return result;
}


/************************************************************************
**
** NAME:        PrintComputersMove
**
** DESCRIPTION: Print out the computer's move given its name and move
**
************************************************************************/

void PrintComputersMove(MOVE computersMove, STRING computersName)
{
	if(computersMove > mancalaR) {
		computersMove = boardSize - computersMove + mancalaR;
	}
	printf("%s's move: %d\n", computersName, computersMove);
}


/************************************************************************
**
** NAME:        Primitive
**
** DESCRIPTION: Determines whether the current board configuration is at
**              a primitive state IE: has someone won/lost/tie?
**              Any configuration that isn't either a win/lose/tie is
**              said to be "undecided."
**
************************************************************************/

VALUE Primitive(POSITION position) {
	int *boardArray;
	int m1, m2, t;
	boardArray = array_hash (position);
	m1  = boardArray[mancalaL];
	m2  = boardArray[mancalaR];
	t   = boardArray[turn];

	SafeFree (boardArray);

	if(m1 + m2 == numOfPieces) {
		if(m1 == m2)
			return tie;
		else if((t == 1 && m1 > m2) || (t == 0 && m1 < m2))
			return (gStandardGame ? win : lose);
		else return (gStandardGame ? lose : win);
	}
	return undecided;
}


/************************************************************************
**
** NAME:        PrintPosition
**
** DESCRIPTION: Print out a current board configuration as well as whose
**              turn it currently is, and the name of the player.
**              Information is printed along with the board to let human
**              players know which bins are valid for selection on their
**              respective turns.  Player1 controls the lower bins while
**              player2 controls the upper bins.
**
************************************************************************/

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn)
{
	int *arrayHashedBoard, i;

	arrayHashedBoard = array_hash(position);

	for(i = 1, printf("\nP2 Bin #  "); i < mancalaR; printf("\t %d", i), i += 1) ;
	printf("\n\t");
	for(i = 1; i < mancalaR; i += 1) {
		printf("\t[%d]", arrayHashedBoard[i]);
	}

	printf("\n\t[%d]", arrayHashedBoard[mancalaL]);
	for(i = 1; i <= mancalaR; printf("\t"), i += 1) ;
	printf("[%d]\n\t", arrayHashedBoard[mancalaR]);

	for(i = boardSize - 1; i > mancalaR; i -= 1) {
		printf("\t[%d]", arrayHashedBoard[i]);
	}
	for(i = mancalaR + 1, printf("\nP1 Bin #  "); i < boardSize;
	    printf("\t %d", i), i += 1) ;

	printf("\n%s\n\n", GetPrediction(position, playerName, usersTurn));

	SafeFree(arrayHashedBoard);
}


/************************************************************************
**
** NAME:        GenerateMoves
**
** DESCRIPTION: Creates a linked list of EVERY valid move at a given
**              board configuration.  This is the method in the mmancala.c
**              file that actually checks for validity of a move depending
**              on the current player's turn.
**
************************************************************************/

MOVELIST *GenerateMoves(POSITION position) {
	int i, j, t, sumTop, sumBottom;
	int startBin = mancalaL + 1;
	int endBin = mancalaR;
	int *arrayHashedBoard;
	MOVELIST *CreateMovelistNode(), *head = NULL;

	arrayHashedBoard = array_hash(position);
	t = arrayHashedBoard[turn];

	if(t == 0) {
		startBin = mancalaR + 1;
		endBin = boardSize;
	}

	/* sum up the top and bottom rows of bins */
	for(i = mancalaL + 1, j = mancalaR + 1, sumTop = 0, sumBottom = 0;
	    i < mancalaR && j < boardSize;
	    sumTop += arrayHashedBoard[i], sumBottom += arrayHashedBoard[j], i += 1, j += 1) ;

	if(Primitive(position) == undecided) {

		/* on my turn starting, i'm empty -> i get to move from my opponent's bins
		   OR the option is set in which you can choose from any bin on the board */
		if(OPT_WINEMPTY == 2 || OPT_MOVEOPP == 1) {
			if(t == 0 && ((sumBottom == 0 && OPT_WINEMPTY == 2) || OPT_MOVEOPP == 1)) {
				for(i = mancalaL + 1; i < mancalaR; i += 1) {
					if(arrayHashedBoard[i] > 0) {
						head = CreateMovelistNode(i, head);
					}
				}
			}
			else if(t == 1 && ((sumTop == 0 && OPT_WINEMPTY == 2) || OPT_MOVEOPP == 1)) {
				for(i = mancalaR + 1; i < boardSize; i += 1) {
					if(arrayHashedBoard[i] > 0) {
						head = CreateMovelistNode(i, head);
					}
				}
			}
		}

		/* on my turn starting, i'm empty -> i pass */
		if(OPT_WINEMPTY == 4 && ((t == 0 && sumBottom == 0) || (t == 1 && sumTop == 0))  ) {
			head = CreateMovelistNode(0, head);
		}

		/* the default moves available */
		else {
			for(i = startBin; i < endBin; i += 1) {
				if(arrayHashedBoard[i] > 0) {
					head = CreateMovelistNode(i, head);
				}
			}
		}
		SafeFree(arrayHashedBoard);
		if (head == NULL) {
			head = CreateMovelistNode(0, head);
		}
		return head;
	}

	else {
		SafeFree(arrayHashedBoard);
		return NULL;
	}
}


/************************************************************************
**
** NAME:        GetAndPrintPlayersMove
**
** DESCRIPTION:
**
************************************************************************/

USERINPUT GetAndPrintPlayersMove(POSITION thePosition, MOVE * theMove, STRING playerName)
{
	int *boardArray;
	int j, k, sumTop, sumBottom, t;
	int sb = mancalaL + 1;
	int eb = mancalaR - 1;
	USERINPUT ret;

	boardArray = array_hash (thePosition);
	t = boardArray[turn];

	/* sum up the top and bottom rows of bins */
	for(j = mancalaL + 1, k = mancalaR + 1, sumTop = 0, sumBottom = 0;
	    j < mancalaR && k < boardSize;
	    sumTop += boardArray[j], sumBottom += boardArray[k], j += 1, k += 1) ;

	if(OPT_MOVEOPP == 1) {
		sb = mancalaL + 1;
		eb = boardSize - 1;
	}
	else if(OPT_WINEMPTY == 2 &&
	        (sumBottom == 0 || sumTop == 0)) {
		if(t == 1 && sumTop == 0) {
			sb = mancalaR + 1;
			eb = boardSize - 1;
		}
	}
	else if(OPT_WINEMPTY == 4 &&
	        (sumBottom == 0 || sumTop == 0)) {
		if((t == 1 && sumTop == 0) ||
		   (t == 0 && sumBottom == 0)) {
			sb = 0;
			eb = 0;
		}
	}
	else if(t == 0) {
		sb = mancalaR + 1;
		eb = boardSize - 1;
	}

	SafeFree (boardArray);

	do {
		printf("%8s's move [%d-%d] :  ", playerName, sb, eb);
		ret = HandleDefaultTextInput(thePosition, theMove, playerName);
		if(ret != Continue)
			return(ret);
	} while(TRUE);

	return(Continue);
}



/************************************************************************
**
** NAME:        ValidTextInput
**
** DESCRIPTION: Valid moves MUST be a number representing any one of the
**              bins on the top and bottom rows, but NOT the mancalas on
**              left and right of the middle row.
**
************************************************************************/
BOOLEAN ValidTextInput(STRING input) {
	int i = input[0] - '0';
	return ( (i >= mancalaL && i < mancalaR) ||
	         (i >  mancalaR && i < boardSize) );
}


/************************************************************************
**
** NAME:        ConvertTextInputToMove
**
** DESCRIPTION: Return an un-validated move that the user enters from
**              the keyboard.  Because the bins on the bottom row don't
**              index perfectly with our board's array configuration,
**              an extra calculation must be done for a selection on those
**              bins.
**
************************************************************************/
MOVE ConvertTextInputToMove(STRING input)
{
	MOVE newMove = (MOVE) (input[0] - '0');
	return (newMove > mancalaR ? boardSize - newMove + mancalaR : newMove);
}


/************************************************************************
**
** NAME:        PrintMove
**
** DESCRIPTION: Print the move that has just been carried out.  The move
**              represents the bin # from the board configuration.
**              Because the bins on the bottom row don't
**              index perfectly with our board's array configuration,
**              an extra calculation must be done for a selection on those
**              bins.
**
************************************************************************/

void PrintMove(MOVE theMove)
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
	STRING move = (STRING) SafeMalloc(8);
	if(theMove > mancalaR) {
		theMove = boardSize - theMove + mancalaR;
	}
	sprintf(move, "%d", theMove);
	return move;
}


/************************************************************************
**
** NAME:        NumberOfOptions
**
** DESCRIPTION: The total number of options (game variants) to the original
**              version of mancala.  The standard extra option is usually
**              the misere version of the game IE: the winning and losing
**              conditions are switched.
**
************************************************************************/

int NumberOfOptions()
{
	/* There are 4 different options each with 2 possible values
	   and 1 remaining option with 5 possible values:
	   2 * 2 * 2 * 2 * 5 = 80
	   This number doesn't include the option to take an extra turn
	   which currently ISN'T implemented in the game */
	return 2*2*2*2*5;
}


/************************************************************************
**
** NAME:        getOption
**
** DESCRIPTION: Returns the current game option being played.  The standard
**              game is given the value of 1, the misere the value of 2.
**              Any other variants must be numbers other than 1 and 2.
**
************************************************************************/

int getOption()
{
	/* There are 4 different options each with 2 possible values
	   and 1 remaining option with 5 possible values */
	int option = 0;

	option *= 5;
	option += OPT_WINEMPTY;

	option *= 2;
	option += (OPT_MOVEOPP ? 1 : 0);

	option *= 2;
	option += (OPT_CHAIN ? 1 : 0);

	option *= 2;
	option += (OPT_NOCAPTURE ? 1 : 0);

	option *= 2;
	option += (gStandardGame ? 0 : 1);

	option++;

	return option;
}


/************************************************************************
**
** NAME:        setOption
**
** DESCRIPTION: Set which type of game is to be played.  Follows the same
**              option-conventions as getOption.
**
************************************************************************/

void setOption(int option)
{
	option--;

	gStandardGame = (option%2 == 0);
	option /= 2;

	OPT_NOCAPTURE = (option%2 == 1);
	option /= 2;

	OPT_CHAIN = (option%2 == 1);
	option /= 2;

	OPT_MOVEOPP = (option%2 == 1);
	option /= 2;

	OPT_WINEMPTY = (option%5);
	option /= 5;
}


/************************************************************************
**
** NAME:        array_hash
**
** DESCRIPTION: Given a board configuration hashed by the rearranger_hash
**              method, returns the same board configuration in an
**              implementation easier to manipulate when needed in other
**              methods.
**              The new representation is an array of integers:
**                 [1] [2] [3]
**              [0]           [4]
**                 [7] [6] [5]
**              An extra element is used in the array to discern whose
**              turn it is.
**              A value of 0 = p1's turn, anything else = p2's turn
**
************************************************************************/

int *array_hash (POSITION position) {
	if (gHashWindowInitialized) {
		//printf("in array_hash position:%d \n",position);
		TIER tier;
		TIERPOSITION tierposition;
		int array_size, unhash_turn, i, j=0;
		gUnhashToTierPosition(position, &tierposition, &tier);
		//printf("in array_hash tier: %d, tierposition:%d\n",tier,tierposition);
		array_size = boardSize-2 + numOfPieces - tier/10000 - tier%10000 - 1;
		char *board = (char *) SafeMalloc (array_size * sizeof(char));
		int *result = (int *) SafeMalloc ((boardSize + 1) * sizeof (int));

		generic_hash_context_switch(tier);
		generic_hash_unhash(tierposition, board);

		/****
		   printf("position:%d, tierposition:%d",position,tierposition);

		   for(i = 0; i < array_size; i++)
		   printf("%c",board[i]);
		   printf(" above is board in array_hash\n");
		 ***/
		unhash_turn = generic_hash_turn(tierposition)-1;
		//printf("turn:%d\n",unhash_turn);
		for (i = 0; i < boardSize; i++) {
			result[i] = 0;
		}

		for (i = 0; i < array_size; i += 1) {
			if (j == mancalaL || j == mancalaR)
				j++;
			if (board[i] == 'o')
				result [j]++;
			else if (board[i] == 'x')
				j++;
		}

		result[mancalaL] = tier/10000;
		result[mancalaR] = tier%10000;
		result[turn] = unhash_turn;
		//printf("in array_hash position:%d tierposition:%d tier:%d\n",position, tierposition, tier);
		/*
		   for(i = 0; i < boardSize +1; i++)
		   printf("%d:%d ",i,result[i]);
		   printf("tier:%d tier/100:%d tier%%100:%d mancalaL:%d mancalaR: %d\n", tier,tier/100, tier%100, mancalaL, mancalaR);
		 ****/
		SafeFree(board);
		return result;

	} else {

		int i, j = 0;
		char *board = (char *) SafeMalloc (rsize * sizeof(char));
		int *result = (int *) SafeMalloc ((boardSize + 1) * sizeof (int));

		if (position > turnOffset) {
			position -= turnOffset;
			result[turn] = 1;
		} else result[turn] = 0;


		rearranger_unhash (position, board); //analogous to generic hash

		for (i = 0; i < boardSize; i++) {
			result[i] = 0;
		}

		for (i = 0; i < rsize; i += 1) {
			if (board[i] == 'o')
				result [j]++;
			else if (board[i] == 'x')
				j++;
		}

		SafeFree (board);
		return result;
	}
}


/************************************************************************
**
** NAME:        array_unhash
**
** DESCRIPTION: Unhashes the array of integers created by the array_hash
**              method back into the implementation of the board config
**              that the rearranger_hash method understands.  The variable
**              dest MUST have memory properly allocated to it, because
**              memory allocation for that variable is not done here.
**
************************************************************************/

POSITION array_unhash (int *hashed) {
	if(gHashWindowInitialized) {
		TIER tier = 10000*hashed[mancalaL] + hashed[mancalaR];
		int i = 0, j = 0;
		/*
		   printf("in array_unhash tier:%d ",tier);
		   for(i=0;i<boardSize+1;i++)
		   printf("%d:%d ",i,hashed[i]);
		 */
		TIERPOSITION tierPos;
		POSITION result;

		int array_size = boardSize - 2 + numOfPieces - tier/10000 - tier%10000 - 1;


		char *dest = (char *) SafeMalloc (array_size * sizeof(char));

		//printf("mancalaL:%d, mancalaR:%d ",mancalaL, mancalaR);
		for (i=0; i < array_size; i++) {
			if(j ==  mancalaL || j == mancalaR) {
				//printf("skipping j=%d i=%d",j,i);
				j++;
			}
			if (hashed[j] > 0) {
				//printf("i:%d %d:%d pieces\n",i,j,hashed[j]);
				dest[i] = 'o';
				hashed[j] -= 1;
			}
			else {
				dest[i] = 'x';
				//printf("i:%d j:%d to %d\n",i,j,j+1);
				j += 1;
			}

		}
		/*
		   for(i=0;i<array_size;i++)
		   printf("%c",dest[i]);
		   printf("\n");
		 */
		generic_hash_context_switch(tier);
		tierPos = generic_hash_hash(dest, hashed[turn]+1);
		generic_hash_context_switch(tier);
		result = gHashToWindowPosition(tierPos, tier);
		//printf("result position in array_unhash= %d\n",result);
		SafeFree(dest);
		return result;
	} else {

		int i = 0, j = 0, k = rsize;
		POSITION result = 0;
		char *dest = (char *) SafeMalloc (rsize * sizeof(char));


		for (; i < k; i++) {
			if (hashed[j] > 0) {
				dest[i] = 'o';
				hashed[j] -= 1;
			}
			else {
				dest[i] = 'x';
				j += 1;
			}
		}

		result = rearranger_hash(dest);

		if (hashed[turn] == 1)
			result += turnOffset;

		SafeFree (dest);
		return result;
	}
}

// for atilla's hash code ...
int *my_gHashOffset = NULL;
int *my_gNCR = NULL;
int my_gHashBoardSize;
int my_gHashMinMax[4];
int my_gHashNumberOfPos;
// end for atilla's hash code

void hash_free()
{
	if(my_gHashOffset)
	{
		SafeFree(my_gHashOffset);
		my_gHashOffset = NULL;
	}
	if(my_gNCR)
	{
		SafeFree(my_gNCR);
		my_gHashOffset = NULL;
	}
}

int rearranger_hash_init(int boardsize, int numOs, int numXs)
{
	my_gHashMinMax[0] = numOs;
	my_gHashMinMax[1] = numOs;
	my_gHashMinMax[2] = numXs;
	my_gHashMinMax[3] = numXs;
	my_gHashBoardSize = boardsize;
	my_nCr_init(boardsize);
	my_gHashNumberOfPos = my_nCr(numXs+numOs, numXs) * my_nCr(boardsize, numXs+numOs);
	return my_gHashNumberOfPos;
}

int rearranger_hash(char* board)
{
	int temp, i, numxs,  numos;
	int boardsize;

	numxs = my_gHashMinMax[3];
	numos = my_gHashMinMax[1];
	boardsize = my_gHashBoardSize;
	temp = 0;

	for (i = 0; i < my_gHashBoardSize; i++)
	{
		if (board[i] == 'b')
		{
			boardsize--;
		}

		else if (board[i] == 'o')
		{
			temp += my_nCr(numxs + numos, numos) * my_nCr(boardsize - 1, numxs + numos);
			boardsize--;
			numos--;
		}
		else
		{
			temp += my_nCr(numxs + numos, numos) *
			        my_nCr(boardsize - 1, numxs + numos) +
			        my_nCr (numxs + numos - 1, numxs) *
			        my_nCr(boardsize - 1,  numxs + numos - 1);
			boardsize--;
			numxs--;
		}
	}
	return temp;


}

BOOLEAN rearranger_unhash(int hashed, char* dest)
{
	int i, j, numxs, numos, temp, boardsize;
	j = 0;
	boardsize = my_gHashBoardSize;
	numxs = my_gHashMinMax[3];
	numos = my_gHashMinMax[1];
	for (i = 0; i < my_gHashBoardSize; i++)
	{
		temp = my_nCr(numxs + numos, numos) *
		       my_nCr(boardsize - 1, numxs + numos);
		if (numxs + numos <= 0) {
			dest[i] = 'b';
		} else if (temp + my_nCr (numxs + numos - 1, numxs) * my_nCr(boardsize - 1, numxs + numos - 1) <= hashed)
		{
			hashed -= (temp + my_nCr (numxs + numos - 1, numxs) * my_nCr(boardsize - 1,  numxs + numos - 1));
			dest[i] = 'x';
			numxs--;
		}
		else if (temp <= hashed)
		{
			hashed -= temp;
			dest[i] = 'o';
			numos--;
		}
		else
		{
			dest[i] = 'b';
		}
		boardsize--;
	}
	return TRUE;
}

void my_nCr_init(int boardsize)
{
	int i, j;
	my_gNCR = (int*) SafeMalloc(sizeof(int) * (boardsize + 1) * (boardsize + 1));
	for(i = 0; i<= boardsize; i++)
	{
		my_gNCR[i*(boardsize+1)] = 1;
		my_gNCR[i*(boardsize+1) + i] = 1;
	}
	for(i = 1; i<= boardsize; i++)
	{
		for(j = 1; j < i; j++)
		{
			my_gNCR[i*(boardsize+1) + j] = my_gNCR[(i-1)*(boardsize+1) + j-1] + my_gNCR[(i-1)*(boardsize+1) + j];
		}
	}
}

int my_nCr(int n, int r)
{
	if (r>n) return 0;
	return my_gNCR[n*(my_gHashBoardSize+1) + r];
}



void* gGameSpecificTclInit = NULL;
POSITION StringToPosition(char* board) {
    int *arrayHashBoard = SafeMalloc(sizeof(int) * (boardSize + 1)), i;
    char *split;
    for (i = 0; i < boardSize; i++) {
        split = strchr(board, 's');
        if (!split) {
            split = strchr(board, ';');
        }
        *split = '\0';
        arrayHashBoard[i] = atoi(board);
        board = split + 1;
    }
    board --;
    *board = ';';
    POSITION pos;
    if (GetValue(board, "turn", GetInt, &arrayHashBoard[turn]) == 0) {
        pos = INVALID_POSITION;
    } else {
        pos = array_unhash(arrayHashBoard);
    }
    SafeFree(arrayHashBoard);
    return pos;
}

int numDigits(int number){
    int digits = 0;
    if (number == 0) digits = 1;
    while (number > 0) {
        number /= 10;
        digits++;
    }
    return digits;
}

char* PositionToString(POSITION pos) {
	int *arrayHashedBoard = array_hash(pos), i;
    int digitLength = 0;
    for (i = 0; i < boardSize; i++) {
        digitLength += numDigits(arrayHashedBoard[i]);
    }
    char *board = SafeMalloc(boardSize + digitLength + 1);
    char *start = board;
    for(i = 0; i < boardSize; i++){
        board += sprintf(board, "%d", arrayHashedBoard[i]); 
        if (i < boardSize - 1) {
            *board = 's';
            board++;
        }
    }
    
    *board = '\0';
    char* turnValue = (char*) SafeMalloc(numDigits(arrayHashedBoard[turn]));
    sprintf(turnValue, "%d", arrayHashedBoard[turn]);
    char* retString = MakeBoardString(start, "turn", turnValue, "");

    SafeFree(arrayHashedBoard);
	return retString;
}

char * PositionToEndData(POSITION pos) {
	return NULL;
}
