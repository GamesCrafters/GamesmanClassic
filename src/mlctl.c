/************************************************************************
**
** NAME:        mlctl.c
**
** DESCRIPTION: Let's Catch The Lion!
**
** AUTHOR:      Roger Tu
**
** DATE:        Start Spring 2010
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

STRING kGameName            = "Let's Catch The Lion!";   /* The name of your game */
STRING kAuthorName          = "Roger Tu";   /* Your name(s) */
STRING kDBName              = "lctl";   /* The name to store the database under */

BOOLEAN kPartizan            = TRUE;   /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN kGameSpecificMenu    = FALSE;   /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN kTieIsPossible       = FALSE;   /* TRUE if a tie is possible. FALSE if it is impossible.*/
BOOLEAN kLoopy               = TRUE;   /* TRUE if the game tree will have cycles (a rearranger style game). FALSE if it does not.*/

BOOLEAN kDebugMenu           = TRUE;   /* TRUE only when debugging. FALSE when on release. */
BOOLEAN kDebugDetermineValue = FALSE;   /* TRUE only when debugging. FALSE when on release. */

POSITION gNumberOfPositions   =  329022720000LLU; /* The number of total possible positions | If you are using our hash, this is given by the hash_init() function*/
POSITION gInitialPosition     =  138782043307LLU; /* The initial hashed position for your starting board */
//POSITION gInitialPosition	= 139331982298LLU;
POSITION kBadPosition         = -1; /* A position that will never be used */

void*    gGameSpecificTclInit = NULL;

/*
 * Help strings that are pretty self-explanatory
 * Strings than span more than one line should have backslashes (\) at the end of the line.
 */

STRING kHelpGraphicInterface =
        "Not written yet";

STRING kHelpTextInterface    =
        "\n☖ Hand\n╔════╗  ╔════╤════╤════╗\n║☖C 0║  ║ 00 │ 01 │ 02 ║\n║☖E 0║  ╟────┼────┼────╢\n║☖G 0║  ║ 03 │ 04 │ 05 ║\n╚════╝  ╟────┼────┼────╢  ╔════╗\n        ║ 06 │ 07 │ 08 ║  ║☗G 0║\n        ╟────┼────┼────╢  ║☗E 0║\n        ║ 09 │ 10 │ 11 ║  ║☗C 0║\n        ╚════╧════╧════╝  ╚════╝\n                          ☗ Hand\n";

STRING kHelpOnYourTurn =
        "";

STRING kHelpStandardObjective =
        "Try to checkmate your opponent's lion or get your lion to the otherside without being checkmated.";

STRING kHelpReverseObjective =
        "Try to get your lion checkmated or get your opponent to move his lion to yourside without being checkmated.";

STRING kHelpTieOccursWhen =
        "Ties are not possible.";

STRING kHelpExample =
        "";


/*************************************************************************
**
** #defines and structs
**
**************************************************************************/
#define BOARDSIZE rows*cols

#define BLACK_TURN 1
#define WHITE_TURN 2

#define BLANK_PIECE ' '

#define BLACK_CHICK 'c'
#define BLACK_HEN 'h'
#define BLACK_ELEPHANT 'e'
#define BLACK_GIRAFFE 'g'
#define BLACK_LION 'l'

#define WHITE_CHICK 'C'
#define WHITE_HEN 'H'
#define WHITE_ELEPHANT 'E'
#define WHITE_GIRAFFE 'G'
#define WHITE_LION 'L'

#define PRINT_BLANK_PIECE "  "
#define PRINT_BLACK_CHICK "☗c"
#define PRINT_BLACK_HEN "☗h"
#define PRINT_BLACK_ELEPHANT "☗e"
#define PRINT_BLACK_GIRAFFE "☗g"
#define PRINT_BLACK_LION "☗l"

#define PRINT_WHITE_CHICK "☖C"
#define PRINT_WHITE_HEN "☖H"
#define PRINT_WHITE_ELEPHANT "☖E"
#define PRINT_WHITE_GIRAFFE "☖G"
#define PRINT_WHITE_LION "☖L"

#define DISTINCT_PIECES 10

#define WHITE 1
#define BLACK 0

#define UP 8
#define DN 2
#define LT 4
#define RT 6
#define UL 7
#define UR 9
#define DL 1
#define DR 3

#define B0W0 '0'
#define B1W0 '1'
#define B0W1 '2'
#define B1W1 '3'
#define B2W0 '4'
#define B0W2 '5'

#define DROP_BLACK_CHICK 0
#define DROP_BLACK_ELEPHANT 1
#define DROP_BLACK_GIRAFFE 2
#define DROP_WHITE_CHICK 3
#define DROP_WHITE_ELEPHANT 4
#define DROP_WHITE_GIRAFFE 5

#define GIRAFFE_INDEX 12
#define ELEPHANT_INDEX 13
#define CHICK_INDEX 14

/*************************************************************************
**
** Global Variables
**
*************************************************************************/
int rows = 4;
int cols = 3;
char *theBoard = "GLE C  c elg000"; // The first 12 characters are the board, the 13th through 15th characters represent the different states the hands can be in.
int theCurrentPlayer = BLACK_TURN;

/*VARIANTS*/
BOOLEAN normalVariant = TRUE;
BOOLEAN misereVariant = FALSE;

int Tier0Context;

/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/

/* External */
#ifndef MEMWATCH
extern GENERIC_PTR      SafeMalloc ();
extern void             SafeFree ();
#endif
void PrintPosition (POSITION position, STRING playersName, BOOLEAN usersTurn);
void setupPieces(char *Board);
BOOLEAN sameTeam(char piece, int currentPlayer);
void printArray (char* boardArray);
void printMoveList(MOVELIST *moves);
POSITION GetCanonicalPosition(POSITION position);
POSITION DoSymmetry(POSITION position, int symmetry);
int opposingPlayer(int player);
int countBits(int i);
BOOLEAN IsLegal(POSITION position);
BOOLEAN areKingsAdjacent(char* boardArray);
char *getBoard();
BOOLEAN isLegalBoard(char *Board);
BOOLEAN IsLegal(POSITION p);
POSITION hash(char* board, int turn);
char* unhash(POSITION position, char *board);
MOVE createMove(int start, int finish);

/************************************************************************
**
** NAME:        InitializeGame
**
** DESCRIPTION: Prepares the game for execution.
**              Initializes required variables.
**
************************************************************************/

void InitializeGame ()
{
	/*
	   theBoard = (char *) SafeMalloc(15 * sizeof(char));
	   theBoard = "GLE C  c elg000";
	   char board[] = "XXXXXXXXXXXXXXX";
	   POSITION position = hash(theBoard, BLACK_TURN);
	   printf("POSITION: %llu\n", position);
	   unhash(position, board);
	   PrintPosition(position, "foo", TRUE);
	   printf("%s\n", board);

	   MOVE tester = 57;
	   printf("move: 57\n");
	   position = DoMove(position, tester);
	   unhash(position, board);
	   PrintPosition(position, "bar", TRUE);
	   printf("\n%s", board);
	 */

	POSITION pos = 0;
	pos += 10LLU * 12 * 26 * 26 * 26 * 26 * 50 * 50;
	pos += 1LLU * 26 * 26 * 26 * 26 * 50 * 50;
	pos += 24 * 26 * 26 * 26 * 50 * 50;
	pos += 24 * 26 * 26 * 50 * 50;
	pos += 24 * 26 * 50 * 50;
	pos += 24 * 50 * 50;
	pos += 48 * 50;
	pos += 48;
	pos += 12LLU * 12 * 26 * 26 * 26 * 26 * 50 * 50;
//	gInitialPosition = pos;
}

/************************************************************************
**
**  Hashing and Unhashing
**
************************************************************************/

void unhash_turn(POSITION position, int* turn) {
	if (position / 164511360000LLU == 0) {
		*turn = BLACK_TURN;
	} else {
		*turn = WHITE_TURN;
	}
}

char* unhash(POSITION position, char* board)
{
	BOOLEAN verbose = FALSE;
	POSITION pos = position;
	POSITION piece;

	int numBC = 0;
	int numBE = 0;
	int numBG = 0;
	int numWC = 0;
	int numWE = 0;
	int numWG = 0;

	// Reset the board
	board[0] = BLANK_PIECE;
	board[1] = BLANK_PIECE;
	board[2] = BLANK_PIECE;
	board[3] = BLANK_PIECE;
	board[4] = BLANK_PIECE;
	board[5] = BLANK_PIECE;
	board[6] = BLANK_PIECE;
	board[7] = BLANK_PIECE;
	board[8] = BLANK_PIECE;
	board[9] = BLANK_PIECE;
	board[10] = BLANK_PIECE;
	board[11] = BLANK_PIECE;
	board[12] = B0W0;
	board[13] = B0W0;
	board[14] = B0W0;

	POSITION turn = pos / 164511360000LLU;
	if (turn == 0) {
		theCurrentPlayer = BLACK_TURN;
	} else {
		theCurrentPlayer = WHITE_TURN;
	}

	pos = pos % 164511360000LLU;
	piece = pos / 13709280000LLU;
	if (verbose) printf("\nBLACK LION: %llu", piece);
	board[pos / 13709280000LLU] = BLACK_LION;

	pos = pos % 13709280000LLU;
	piece = pos / 1142440000LLU;
	if (verbose) printf("\nWHITE LION: %llu", piece);
	board[pos / 1142440000LLU] = WHITE_LION;

	pos = pos % 1142440000LLU;
	piece = pos / 43940000;
	if (verbose) printf("\nGIRAFFE 1: %llu", piece);
	if (piece < 12) { // then black
		board[piece] = BLACK_GIRAFFE;
	} else if (piece < 24) { // then white
		board[piece - 12] = WHITE_GIRAFFE;
	} else if (piece == 24) { // then black hand
		numBG += 1;
	} else { // must be 25, meaning in white hand
		numWG += 1;
	}

	pos = pos % 43940000;
	piece = pos / 1690000;
	if (verbose) printf("\nGIRAFFE 2: %llu", piece);
	if (piece < 12) { // then black
		board[piece] = BLACK_GIRAFFE;
	} else if (piece < 24) { // then white
		board[piece - 12] = WHITE_GIRAFFE;
	} else if (piece == 24) { // then black hand
		numBG += 1;
	} else { // must be 25, meaning in white hand
		numWG += 1;
	}

	pos = pos % 1690000;
	piece = pos / 65000;
	if (verbose) printf("\nELEPHANT 1: %llu", piece);
	if (piece < 12) { // then black
		board[piece] = BLACK_ELEPHANT;
	} else if (piece < 24) { // then white
		board[piece - 12] = WHITE_ELEPHANT;
	} else if (piece == 24) { // then black hand
		numBE += 1;
	} else { // must be 25, meaning in white hand
		numWE += 1;
	}

	pos = pos % 65000;
	piece = pos / 2500;
	if (verbose) printf("\nELEPHANT 2: %llu", piece);
	if (piece < 12) { // then black
		board[piece] = BLACK_ELEPHANT;
	} else if (piece < 24) { // then white
		board[piece - 12] = WHITE_ELEPHANT;
	} else if (piece == 24) { // then black hand
		numBE += 1;
	} else { // must be 25, meaning in white hand
		numWE += 1;
	}

	pos = pos % 2500;
	piece = pos / 50;
	if (verbose) printf("\nCHICK 1: %llu", piece);
	if (piece < 12) { // then black
		board[piece] = BLACK_CHICK;
	} else if (piece < 24) { // then white
		board[piece - 12] = WHITE_CHICK;
	} else if (piece < 36) { // then black, promoted
		board[piece - 24] = BLACK_HEN;
	} else if (piece < 48) { // then white, promoted
		board[piece - 36] = WHITE_HEN;
	} else if (piece == 48) { // then in black hand
		numBC += 1;
	} else { // must be 49, meaning in white hand
		numWC += 1;
	}

	pos = pos % 50;
	piece = pos;
	if (verbose) printf("\nCHICK 2: %llu", piece);
	if (piece < 12) { // then black
		board[piece] = BLACK_CHICK;
	} else if (piece < 24) { // then white
		board[piece - 12] = WHITE_CHICK;
	} else if (piece < 36) { // then black, promoted
		board[piece - 24] = BLACK_HEN;
	} else if (piece < 48) { // then white, promoted
		board[piece - 36] = WHITE_HEN;
	} else if (piece == 48) { // then in black hand
		numBC += 1;
	} else { // must be 49, meaning in white hand
		numWC += 1;
	}

	if (numBC == 0) { // set board state for chicks in hand
		if (numWC == 0) {
			board[CHICK_INDEX] = B0W0;
		} else if (numWC == 1) {
			board[CHICK_INDEX] = B0W1;
		} else if (numWC == 2) {
			board[CHICK_INDEX] = B0W2;
		} else { // more than 2 in hand
			perror("Error: Attempting to place more than 2 pieces of 1 type in hand");
		}
	} else if (numBC == 1) {
		if (numWC == 0) {
			board[CHICK_INDEX] = B1W0;
		} else if (numWC == 1) {
			board[CHICK_INDEX] = B1W1;
		} else { // more than 2 in hand
			perror("Error: Attempting to place more than 2 pieces of 1 type in hand");
		}
	} else if (numBC == 2) {
		if (numWC == 0) {
			board[CHICK_INDEX] = B2W0;
		} else { // more than 2 in hand
			perror("Error: Attempting to place more than 2 pieces of 1 type in hand");
		}
	} else { // more than 2 in hand
		perror("Error: Attempting to place more than 2 pieces of 1 type in hand");
	}

	if (numBE == 0) { // set board state for elephants in hand
		if (numWE == 0) {
			board[ELEPHANT_INDEX] = B0W0;
		} else if (numWE == 1) {
			board[ELEPHANT_INDEX] = B0W1;
		} else if (numWE == 2) {
			board[ELEPHANT_INDEX] = B0W2;
		} else { // more than 2 in hand
			perror("Error: Attempting to place more than 2 pieces of 1 type in hand");
		}
	} else if (numBE == 1) {
		if (numWE == 0) {
			board[ELEPHANT_INDEX] = B1W0;
		} else if (numWE == 1) {
			board[ELEPHANT_INDEX] = B1W1;
		} else { // more than 2 in hand
			perror("Error: Attempting to place more than 2 pieces of 1 type in hand");
		}
	} else if (numBE == 2) {
		if (numWE == 0) {
			board[ELEPHANT_INDEX] = B2W0;
		} else { // more than 2 in hand
			perror("Error: Attempting to place more than 2 pieces of 1 type in hand");
		}
	} else { // more than 2 in hand
		perror("Error: Attempting to place more than 2 pieces of 1 type in hand");
	}

	if (numBG == 0) { // set board state for giraffes in hand
		if (numWG == 0) {
			board[GIRAFFE_INDEX] = B0W0;
		} else if (numWG == 1) {
			board[GIRAFFE_INDEX] = B0W1;
		} else if (numWG == 2) {
			board[GIRAFFE_INDEX] = B0W2;
		} else { // more than 2 in hand
			perror("Error: Attempting to place more than 2 pieces of 1 type in hand");
		}
	} else if (numBG == 1) {
		if (numWG == 0) {
			board[GIRAFFE_INDEX] = B1W0;
		} else if (numWG == 1) {
			board[GIRAFFE_INDEX] = B1W1;
		} else { // more than 2 in hand
			perror("Error: Attempting to place more than 2 pieces of 1 type in hand");
		}
	} else if (numBG == 2) {
		if (numWG == 0) {
			board[GIRAFFE_INDEX] = B2W0;
		} else { // more than 2 in hand
			perror("Error: Attempting to place more than 2 pieces of 1 type in hand");
		}
	} else { // more than 2 in hand
		perror("Error: Attempting to place more than 2 pieces of 1 type in hand");
	}

	return board;
}


POSITION hash(char* board, int turn) {
	POSITION position = 0;
	POSITION tmp;
	char piece;

	if (turn == WHITE_TURN) {
		position = 164511360000LLU;
	}

	int i;
	int foundc = 0;
	int founde = 0;
	int foundg = 0;

	for (i = 0; i < 12; i++) {
		if (board[i] == BLANK_PIECE) {
			continue;
		} else if (board[i] == BLACK_LION) {
			position += i * 13709280000LLU;
		} else if (board[i] == WHITE_LION) {
			position += i * 1142440000LLU;
		} else if (board[i] == BLACK_CHICK) {
			if (foundc == 0) {
				tmp = i * 50;
			} else {
				tmp = i;
			}
			position += tmp;
			foundc++;
		} else if (board[i] == WHITE_CHICK) {
			if (foundc == 0) {
				tmp = (i + 12) * 50;
			} else {
				tmp = (i + 12);
			}
			position += tmp;
			foundc++;
		} else if (board[i] == BLACK_HEN) {
			if (foundc == 0) {
				tmp = (i + 24) * 50;
			} else {
				tmp = (i + 24);
			}
			position += tmp;
			foundc++;
		} else if (board[i] == WHITE_HEN) {
			if (foundc == 0) {
				tmp = (i + 36) * 50;
			} else {
				tmp = (i + 36);
			}
			position += tmp;
			foundc++;
		} else if (board[i] == BLACK_ELEPHANT) {
			if (founde == 0) {
				tmp = i * 65000;
			} else {
				tmp = i * 2500;
			}
			position += tmp;
			founde++;
		} else if (board[i] == WHITE_ELEPHANT) {
			if (founde == 0) {
				tmp = (i + 12) * 65000;
			} else {
				tmp = (i + 12) * 2500;
			}
			position += tmp;
			founde++;
		} else if (board[i] == BLACK_GIRAFFE) {
			if (foundg == 0) {
				tmp = i * 43940000;
			} else {
				tmp = i * 1690000;
			}
			position += tmp;
			foundg++;
		} else if (board[i] == WHITE_GIRAFFE) {
			if (foundg == 0) {
				tmp = (i + 12) * 43940000;
			} else {
				tmp = (i + 12) * 1690000;
			}
			position += tmp;
			foundg++;
		}
	}

	/* Hand Position States
	 * 0 - black has 0 white has 0 // this case does not need to be dealt with because this means both pieces are on the board
	 * 1 - black has 1 white has 0
	 * 2 - black has 0 white has 1
	 * 3 - black has 1 white has 1
	 * 4 - black has 2 white has 0
	 * 5 - black has 0 white has 2
	 */

	piece = board[GIRAFFE_INDEX]; // Giraffes
	if (piece == B1W0) {
		position += 24 * 1690000;
	} else if (piece == B0W1) {
		position += 25 * 1690000;
	} else if (piece == B1W1) {
		position += 24 * 43940000;
		position += 25 * 1690000;
	} else if (piece == B2W0) {
		position += 24 * 43940000;
		position += 24 * 1690000;
	} else if (piece == B0W2) {
		position += 25 * 43940000;
		position += 25 * 1690000;
	}

	piece = board[ELEPHANT_INDEX]; // Elephants
	if (piece == B1W0) {
		position += 24 * 2500;
	} else if (piece == B0W1) {
		position += 25 * 2500;
	} else if (piece == B1W1) {
		position += 24 * 65000;
		position += 25 * 2500;
	} else if (piece == B2W0) {
		position += 24 * 65000;
		position += 24 * 2500;
	} else if (piece == B0W2) {
		position += 25 * 65000;
		position += 25 * 2500;
	}

	piece = board[CHICK_INDEX]; // Chicks
	if (piece == B1W0) {
		position += 48;
	} else if (piece == B0W1) {
		position += 49;
	} else if (piece == B1W1) {
		position += 48 * 50;
		position += 49;
	} else if (piece == B2W0) {
		position += 48 * 50;
		position += 48;
	} else if (piece == B0W2) {
		position += 49 * 50;
		position += 49;
	}

	return position;
}


/************************************************************************
**
** NAME:	coordToNum
**
** DESCRIPTION:	Takes a coordinate from (0,0) to (3,2) and returns a
**		number from 0 to 11.
**
** INPUTS:	int r, c
**
** OUTPUTS:	int n
**
** CALLS:	none
**
************************************************************************/

int coordToNum(int r, int c) {
	return (r*rows + c);
}

BOOLEAN sameTeam(char piece, int playerTurn) {
	if (playerTurn == BLACK_TURN) {
		if (piece == BLACK_CHICK || piece == BLACK_HEN || piece == BLACK_ELEPHANT || piece == BLACK_GIRAFFE || piece == BLACK_LION) {
			return TRUE;
		} else {
			return FALSE;
		}
	} else if (playerTurn == WHITE_TURN) {
		if (piece == WHITE_CHICK || piece == WHITE_HEN || piece == WHITE_ELEPHANT || piece == WHITE_GIRAFFE || piece == WHITE_LION) {
			return TRUE;
		} else {
			return FALSE;
		}
	} else {
		perror("Error: player turn neither black nor white");
		return FALSE;
	}
}

STRING pieceToPrint(char piece) {
	switch (piece) {
	case BLANK_PIECE:
		return PRINT_BLANK_PIECE;
	case BLACK_CHICK:
		return PRINT_BLACK_CHICK;
	case BLACK_HEN:
		return PRINT_BLACK_HEN;
	case BLACK_ELEPHANT:
		return PRINT_BLACK_ELEPHANT;
	case BLACK_GIRAFFE:
		return PRINT_BLACK_GIRAFFE;
	case BLACK_LION:
		return PRINT_BLACK_LION;
	case WHITE_CHICK:
		return PRINT_WHITE_CHICK;
	case WHITE_HEN:
		return PRINT_WHITE_HEN;
	case WHITE_ELEPHANT:
		return PRINT_WHITE_ELEPHANT;
	case WHITE_GIRAFFE:
		return PRINT_WHITE_GIRAFFE;
	case WHITE_LION:
		return PRINT_WHITE_LION;
	default:
		return "XX";
	}
}

/*
   BOOLEAN lionSafeAt(char* board, int player, int r, int c) {
 */
BOOLEAN lionSafeAt(char* board, int player, int index) {
	BOOLEAN checkup = FALSE;
	BOOLEAN checkdn = FALSE;
	BOOLEAN checklt = FALSE;
	BOOLEAN checkrt = FALSE;
	if (index != 0 && index != 1 && index != 2) {
		checkup = TRUE;
	}
	if (index != 9 && index != 10 && index != 11) {
		checkdn = TRUE;
	}
	if (index != 0 && index != 3 && index != 6 && index != 9) {
		checklt = TRUE;
	}
	if (index != 2 && index != 5 && index != 8 && index != 11) {
		checkrt = TRUE;
	}

	char piece;
	if (player == BLACK_TURN) { // used for piece directionality
		if (checkup) { // only lion, giraffe, chick, hen can move down
			piece = board[index - 3];
			if (piece == WHITE_LION || piece == WHITE_GIRAFFE || piece == WHITE_CHICK || piece == WHITE_HEN) {
				return FALSE;
			}
		}
		if (checklt) { // only lion, giraffe, hen can move right
			piece = board[index - 1];
			if (piece == WHITE_LION || piece == WHITE_GIRAFFE || piece == WHITE_HEN) {
				return FALSE;
			}
		}
		if (checkrt) { // only lion, giraffe, hen can move left
			piece = board[index + 1];
			if (piece == WHITE_LION || piece == WHITE_GIRAFFE || piece == WHITE_HEN) {
				return FALSE;
			}
		}
		if (checkdn) { // only lion, giraffe, hen can move up
			piece = board[index + 3];
			if (piece == WHITE_LION || piece == WHITE_GIRAFFE || piece == WHITE_HEN) {
				return FALSE;
			}
		}
		if (checkup && checklt) { // only lion, elephant, hen can move down right
			piece = board[index - 4];
			if (piece == WHITE_LION || piece == WHITE_ELEPHANT || piece == WHITE_HEN) {
				return FALSE;
			}
		}
		if (checkup && checkrt) { // only lion, elephant, hen can move down left
			piece = board[index - 2];
			if (piece == WHITE_LION || piece == WHITE_ELEPHANT || piece == WHITE_HEN) {
				return FALSE;
			}
		}
		if (checkdn && checklt) { // only lion, elephant can move up right
			piece = board[index + 2];
			if (piece == WHITE_LION || piece == WHITE_ELEPHANT) {
				return FALSE;
			}
		}
		if (checkdn && checkrt) { // only lion, elephant can move up left
			piece = board[index + 4];
			if (piece == WHITE_LION || piece == WHITE_ELEPHANT) {
				return FALSE;
			}
		}
	} else { // used for piece directionality
		if (checkup) { // only lion, giraffe, hen can move down
			piece = board[index - 3];
			if (piece == BLACK_LION || piece == BLACK_GIRAFFE || piece == BLACK_HEN) {
				return FALSE;
			}
		}
		if (checklt) { // only lion, giraffe, hen can move right
			piece = board[index - 1];
			if (piece == BLACK_LION || piece == BLACK_GIRAFFE || piece == BLACK_HEN) {
				return FALSE;
			}
		}
		if (checkrt) { // only lion, giraffe, hen can move left
			piece = board[index + 1];
			if (piece == BLACK_LION || piece == BLACK_GIRAFFE || piece == BLACK_HEN) {
				return FALSE;
			}
		}
		if (checkdn) { // only lion, giraffe, chick, hen can move up
			piece = board[index + 3];
			if (piece == BLACK_LION || piece == BLACK_GIRAFFE || piece == BLACK_HEN || piece == BLACK_CHICK) {
				return FALSE;
			}
		}
		if (checkup && checklt) { // only lion, elephant can move down right
			piece = board[index - 4];
			if (piece == BLACK_LION || piece == BLACK_ELEPHANT) {
				return FALSE;
			}
		}
		if (checkup && checkrt) { // only lion, elephant can move down left
			piece = board[index - 2];
			if (piece == BLACK_LION || piece == BLACK_ELEPHANT) {
				return FALSE;
			}
		}
		if (checkdn && checklt) { // only lion, elephant, hen can move up right
			piece = board[index + 2];
			if (piece == BLACK_LION || piece == BLACK_ELEPHANT || piece == BLACK_HEN) {
				return FALSE;
			}
		}
		if (checkdn && checkrt) { // only lion, elephant, hen can move up left
			piece = board[index + 4];
			if (piece == BLACK_LION || piece == BLACK_ELEPHANT || piece == BLACK_HEN) {
				return FALSE;
			}
		}
	}
	return TRUE; // it is ok for the king to move into this square because no enemy pieces can move here next turn.
}

/************************************************************************
**
** NAME:	createMove
**
** DESCRIPTION:
**
**			[0][0-11][0-7]
**
** INPUTS:	int initialrow, initialcol, endrow, endcol
**
** OUTPUTS:	MOVE move
**
** CALLS:	coordToNum
**
************************************************************************/

/*
   MOVE createMove(int ri, int ci, int rv, int cv) {
        MOVE move = coordToNum(ri, ci) * 8;
        if (rv == -1) {
                if (cv == -1) {
                        move += 0;
                } else if (cv == 0) {
                        move += 1;
                } else {
                        move += 2;
                }
        } else if (rv == 0) {
                if (cv == -1) {
                        move += 3;
                } else {
                        move += 4;
                }
        } else {
                if (cv == -1) {
                        move += 5;
                } else if (cv == 0) {
                        move += 6;
                } else {
                        move += 7;
                }
        }
        return move;
   }
 */

MOVE createMove(int start, int finish) {
	MOVE move = start * 8;
	if ((start - 4) == finish) { // ul
		move += 0;
	} else if ((start - 3) == finish) { // up
		move += 1;
	} else if ((start - 2) == finish) { // ur
		move += 2;
	} else if ((start - 1) == finish) { // lt
		move += 3;
	} else if (start == finish) { // same thing.. not possible
		perror("attempted a null move");
	} else if ((start + 1) == finish) { // rt
		move += 4;
	} else if ((start + 2) == finish) { // dl
		move += 5;
	} else if ((start + 3) == finish) { // dn
		move += 6;
	} else if ((start + 4) == finish) { // dr
		move += 7;
	}
	return move;
}

BOOLEAN testMove(char* board, MOVE move, int player) {
	char testBoard[15];
	int i = 0;
	for (i = 0; i < 15; i++) {
		testBoard[i] = board[i];
	}
	POSITION position = hash(testBoard, player);
	position = DoMove(position, move);
	unhash(position, testBoard);
	for (i = 0; i < 12; i++) {
		if (player == BLACK_TURN && testBoard[i] == BLACK_LION) {
			if (lionSafeAt(testBoard, player, i)) {
				return TRUE;
			} else {
				return FALSE;
			}
		} else if (player == WHITE_TURN && testBoard[i] == WHITE_LION) {
			if (lionSafeAt(testBoard, player, i)) {
				return TRUE;
			} else {
				return FALSE;
			}
		}
	}
	return FALSE;
}

/************************************************************************
**
** NAME:	createDrop
**
** DESCRIPTION:	Takes a piece and a coordinate from (0,0) to (3,2) and returns a MOVE.
**
**			[1][0-11][0-5]
**
** INPUTS:	int initialrow, initialcol, endrow, endcol
**
** OUTPUTS:	MOVE move
**
** CALLS:	none
**
************************************************************************/

MOVE createDrop(char piece, int index) {
	MOVE move = 96;
	move += index * 6;
	switch (piece) {
	case BLACK_CHICK:
		move += DROP_BLACK_CHICK;
		break;
	case BLACK_ELEPHANT:
		move += DROP_BLACK_ELEPHANT;
		break;
	case BLACK_GIRAFFE:
		move += DROP_BLACK_GIRAFFE;
		break;
	case WHITE_CHICK:
		move += DROP_WHITE_CHICK;
		break;
	case WHITE_ELEPHANT:
		move += DROP_WHITE_ELEPHANT;
		break;
	case WHITE_GIRAFFE:
		move += DROP_WHITE_GIRAFFE;
		break;
	default:
		break;
	}
	return move;
}

/*
   MOVE createDrop(char piece, int r, int c) {
        MOVE move = 96;
        move += coordToNum(r, c) * 6;
        switch (piece) {
                case BLACK_CHICK:
                        move += DROP_BLACK_CHICK;
                        break;
                case BLACK_ELEPHANT:
                        move += DROP_BLACK_ELEPHANT;
                        break;
                case BLACK_GIRAFFE:
                        move += DROP_BLACK_GIRAFFE;
                        break;
                case WHITE_CHICK:
                        move += DROP_WHITE_CHICK;
                        break;
                case WHITE_ELEPHANT:
                        move += DROP_WHITE_ELEPHANT;
                        break;
                case WHITE_GIRAFFE:
                        move += DROP_WHITE_GIRAFFE;
                        break;
                default:
                        break;
        }
        return move;
   }
 */

void generateMoveWithDir(char* board, MOVELIST **moves, int player, int start, int dir) {
/*
   void generateMoveWithDir(char* board, MOVELIST **moves, int player, int r, int c, int dir) {
 */
	if (!sameTeam(board[start], player)) { // if this piece is not current player's do not generate a move.
		// This should never happen;
		perror("Error: player attempted to move a piece not his/her own.");
		return;
	}

	int vecr = 0;
	int vecc = 0;

	if (dir == UL || dir == UP || dir == UR) {
		vecr = -1;
	} else if (dir == DL || dir == DN || dir == DR) {
		vecc = 1;
	}

	if (dir == UL || dir == LT || dir == DL) {
		vecc = -1;
	} else if (dir == UR || dir == RT || dir == DR) {
		vecc = 1;
	}

	int finish = start + (3 * vecr) + vecc;

	MOVE newMove;
	if (player == BLACK_TURN) {
		if (board[start] == BLACK_LION) {
			if (!sameTeam(board[finish], player) && lionSafeAt(board, player, finish)) {
				newMove = createMove(start, finish);
				*moves = CreateMovelistNode(newMove, *moves);
			}
		} else {
			if (!sameTeam(board[finish], player)) {
				newMove = createMove(start, finish);
				if (testMove(board, newMove, player)) {
					*moves = CreateMovelistNode(newMove, *moves);
				}
			}
		}
	} else {
		if (board[start] == WHITE_LION) {
			if (!sameTeam(board[finish], player) && lionSafeAt(board, player, finish)) {
				newMove = createMove(start, finish);
				*moves = CreateMovelistNode(newMove, *moves);
			}
		} else {
			if (!sameTeam(board[finish], player)) {
				newMove = createMove(start, finish);
				if (testMove(board, newMove, player)) {
					*moves = CreateMovelistNode(newMove, *moves);
				}
			}
		}
	}

/*
        int newr = r + vecr;
        int newc = c + vecc;

        if (newr < 0 || newc < 0 || newr > 3 || newc > 2) { // check out of bounds
                return;
        } else { // other checks
                MOVE newMove;
                if (player == BLACK_TURN) {
                        if (board[coordToNum(r, c)] == BLACK_LION) { // special checks for moving king
                                if (!sameTeam(board[coordToNum(vecr, vecc)], player) && lionSafeAt(board, player, newr, newc)) {
                                        newMove = createMove(r, c, vecr, vecc);
 * moves = CreateMovelistNode(newMove, *moves);
                                }
                        } else {
                                if (!sameTeam(board[coordToNum(vecr, vecc)], player)) { // generate move from (r, c) to (vecr, vecc)
                                        newMove = createMove(r, c, vecr, vecc);
                                        if (testMove(board, newMove, player)) {
 * moves = CreateMovelistNode(newMove, *moves);
                                        }
                                }
                        }
                } else {
                        if (board[coordToNum(r, c)] == WHITE_LION) { // special checks for moving king
                                if (!sameTeam(board[coordToNum(vecr, vecc)], player) && lionSafeAt(board, player, newr, newc)) {
                                        newMove = createMove(r, c, vecr, vecc);
 * moves = CreateMovelistNode(newMove, *moves);
                                }
                        } else {
                                if (!sameTeam(board[coordToNum(vecr, vecc)], player)) { // generate move from (r, c) to (vecr, vecc)
                                        newMove = createMove(r, c, vecr, vecc);
                                        if (testMove(board, newMove, player)) {
 * moves = CreateMovelistNode(newMove, *moves);
                                        }
                                }
                        }
                }
        }
 */
}

void generateDropMoves(char* board, MOVELIST **moves, int player, int index) {
	MOVE newMove;
	if (player == BLACK_TURN) {
		if (board[CHICK_INDEX] == B1W0 || board[CHICK_INDEX] == B1W1 || board[CHICK_INDEX] == B2W0) {
			newMove = createDrop(BLACK_CHICK, index);
			if (testMove(board, newMove, player)) {
				*moves = CreateMovelistNode(newMove, *moves);
			}
		}
		if (board[ELEPHANT_INDEX] == B1W0 || board[ELEPHANT_INDEX] == B1W1 || board[ELEPHANT_INDEX] == B2W0) {
			newMove = createDrop(BLACK_ELEPHANT, index);
			if (testMove(board, newMove, player)) {
				*moves = CreateMovelistNode(newMove, *moves);
			}
		}
		if (board[GIRAFFE_INDEX] == B1W0 || board[GIRAFFE_INDEX] == B1W1 || board[GIRAFFE_INDEX] == B2W0) {
			newMove = createDrop(BLACK_GIRAFFE, index);
			if (testMove(board, newMove, player)) {
				*moves = CreateMovelistNode(newMove, *moves);
			}
		}
	} else {
		if (board[CHICK_INDEX] == B0W1 || board[CHICK_INDEX] == B1W1 || board[CHICK_INDEX] == B0W2) {
			newMove = createDrop(WHITE_CHICK, index);
			if (testMove(board, newMove, player)) {
				*moves = CreateMovelistNode(newMove, *moves);
			}
		}
		if (board[ELEPHANT_INDEX] == B0W1 || board[ELEPHANT_INDEX] == B1W1 || board[ELEPHANT_INDEX] == B0W2) {
			newMove = createDrop(WHITE_ELEPHANT, index);
			if (testMove(board, newMove, player)) {
				*moves = CreateMovelistNode(newMove, *moves);
			}
		}
		if (board[GIRAFFE_INDEX] == B0W1 || board[GIRAFFE_INDEX] == B1W1 || board[GIRAFFE_INDEX] == B0W2) {
			newMove = createDrop(WHITE_GIRAFFE, index);
			if (testMove(board, newMove, player)) {
				*moves = CreateMovelistNode(newMove, *moves);
			}
		}
	}
}

/*
   void generateDropMoves(char* board, MOVELIST **moves, int player, int r, int c) {
        MOVE newMove;
        if (player == BLACK_TURN) {
                if (board[CHICK_INDEX] == B1W0 || board[CHICK_INDEX] == B1W1 || board[CHICK_INDEX] == B2W0) {
                        newMove = createDrop(BLACK_CHICK, r, c);
                        if (testMove(board, newMove, player)) {
 * moves = CreateMovelistNode(newMove, *moves);
                        }
                }
                if (board[ELEPHANT_INDEX] == B1W0 || board[ELEPHANT_INDEX] == B1W1 || board[ELEPHANT_INDEX] == B2W0) {
                        newMove = createDrop(BLACK_ELEPHANT, r, c);
                        if (testMove(board, newMove, player)) {
 * moves = CreateMovelistNode(newMove, *moves);
                        }
                }
                if (board[GIRAFFE_INDEX] == B1W0 || board[GIRAFFE_INDEX] == B1W1 || board[GIRAFFE_INDEX] == B2W0) {
                        newMove = createDrop(BLACK_GIRAFFE, r, c);
                        if (testMove(board, newMove, player)) {
 * moves = CreateMovelistNode(newMove, *moves);
                        }
                }
        } else {
                if (board[CHICK_INDEX] == B0W1 || board[CHICK_INDEX] == B1W1 || board[CHICK_INDEX] == B0W2) {
                        newMove = createDrop(WHITE_CHICK, r, c);
                        if (testMove(board, newMove, player)) {
 * moves = CreateMovelistNode(newMove, *moves);
                        }
                }
                if (board[ELEPHANT_INDEX] == B0W1 || board[ELEPHANT_INDEX] == B1W1 || board[ELEPHANT_INDEX] == B0W2) {
                        newMove = createDrop(WHITE_ELEPHANT, r, c);
                        if (testMove(board, newMove, player)) {
 * moves = CreateMovelistNode(newMove, *moves);
                        }
                }
                if (board[GIRAFFE_INDEX] == B0W1 || board[GIRAFFE_INDEX] == B1W1 || board[GIRAFFE_INDEX] == B0W2) {
                        newMove = createDrop(WHITE_GIRAFFE, r, c);
                        if (testMove(board, newMove, player)) {
 * moves = CreateMovelistNode(newMove, *moves);
                        }
                }
        }
   }
 */

/************************************************************************
**
** NAME:        DoMove
**
** DESCRIPTION: Applies the move to the position.
**** INPUTS:      POSITION position : The old position
**              MOVE     move     : The move to apply to the position
**
** OUTPUTS:     (POSITION)        : The position that results from move
**
** CALLS:       Some Board Hash Function
**              Some Board Unhash Function
**
*************************************************************************/

POSITION DoMove (POSITION position, MOVE move) {
	char boardArray[rows*cols + 4];
	char captured;
	int rv = 0, cv = 0;
	int indexi, indexf; // array indices corresponding to move start and end
	unhash(position, boardArray);
	int currentPlayer;
	unhash_turn(position, &currentPlayer);
	if (move < 96) { // moves 0 ~ 95 are moves
		int dir = move % 8;
		switch (dir) {
		case 0:         // move ul
			rv = -1;
			cv = -1;
			break;
		case 1:         // move up
			rv = -1;
			cv = 0;
			break;
		case 2:         // move ur
			rv = -1;
			cv = 1;
			break;
		case 3:         // move lt
			rv = 0;
			cv = -1;
			break;
		case 4:         // move rt
			rv = 0;
			cv = 1;
			break;
		case 5:         // move dl
			rv = 1;
			cv = -1;
			break;
		case 6:         // move dn
			rv = 1;
			cv = 0;
			break;
		case 7:         // move dr
			rv = 1;
			cv = 1;
			break;
		default:
			rv = 0;
			cv = 0;
			break;
		}
		indexi = move / 8;
//		printf("\ni: %d", indexi);
		indexf = indexi + (cols * rv) + cv;
//		printf("\nf: %d", indexf);
		captured = boardArray[indexf]; // get captured piece to determine type
		boardArray[indexf] = boardArray[indexi]; // move piece to final location
		if (boardArray[indexf] == BLACK_CHICK && (indexf == 0 || indexf == 1 || indexf == 2)) { // promote black chick to black hen if applicable
			boardArray[indexf] = BLACK_HEN;
		} else if (boardArray[indexf] == WHITE_CHICK && (indexf == 9 || indexf == 10 || indexf == 11)) { // promote white chick to white hen if applicable
			boardArray[indexf] = WHITE_HEN;
		}
		boardArray[indexi] = BLANK_PIECE; // set original location to blank
		switch (captured) {
		case BLANK_PIECE:
			break;
		case BLACK_CHICK:
			if (boardArray[CHICK_INDEX] == B0W0) {
				boardArray[CHICK_INDEX] = B0W1;
			} else if (boardArray[CHICK_INDEX] == B1W0) {
				boardArray[CHICK_INDEX] = B1W1;
			} else if (boardArray[CHICK_INDEX] == B0W1) {
				boardArray[CHICK_INDEX] = B0W2;
			} else {         // this should not ever happen (implies more than 2 of this piece)
				perror("Error: more than two chicks");
			}
			break;
		case BLACK_HEN:
			if (boardArray[CHICK_INDEX] == B0W0) {
				boardArray[CHICK_INDEX] = B0W1;
			} else if (boardArray[CHICK_INDEX] == B1W0) {
				boardArray[CHICK_INDEX] = B1W1;
			} else if (boardArray[CHICK_INDEX] == B0W1) {
				boardArray[CHICK_INDEX] = B0W2;
			} else {         // this should not ever happen (implies more than 2 of this piece)
				perror("Error: more than two chicks");
			}
			break;
		case WHITE_CHICK:
			if (boardArray[CHICK_INDEX] == B0W0) {
				boardArray[CHICK_INDEX] = B1W0;
			} else if (boardArray[CHICK_INDEX] == B1W0) {
				boardArray[CHICK_INDEX] = B2W0;
			} else if (boardArray[CHICK_INDEX] == B0W1) {
				boardArray[CHICK_INDEX] = B1W1;
			} else {         // this should not ever happen (implies more than 2 of this piece)
				perror("Error: more than two chicks");
			}
			break;
		case WHITE_HEN:
			if (boardArray[CHICK_INDEX] == B0W0) {
				boardArray[CHICK_INDEX] = B1W0;
			} else if (boardArray[CHICK_INDEX] == B1W0) {
				boardArray[CHICK_INDEX] = B2W0;
			} else if (boardArray[CHICK_INDEX] == B0W1) {
				boardArray[CHICK_INDEX] = B1W1;
			} else {         // this should not ever happen (implies more than 2 of this piece)
				perror("Error: more than two chicks");
			}
			break;
		case BLACK_ELEPHANT:
			if (boardArray[ELEPHANT_INDEX] == B0W0) {
				boardArray[ELEPHANT_INDEX] = B0W1;
			} else if (boardArray[ELEPHANT_INDEX] == B1W0) {
				boardArray[ELEPHANT_INDEX] = B1W1;
			} else if (boardArray[ELEPHANT_INDEX] == B0W1) {
				boardArray[ELEPHANT_INDEX] = B0W2;
			} else {         // this should not ever happen (implies more than 2 of this piece)
				perror("Error: more than two elephants");
			}
			break;
		case WHITE_ELEPHANT:
			if (boardArray[ELEPHANT_INDEX] == B0W0) {
				boardArray[ELEPHANT_INDEX] = B1W0;
			} else if (boardArray[ELEPHANT_INDEX] == B1W0) {
				boardArray[ELEPHANT_INDEX] = B2W0;
			} else if (boardArray[ELEPHANT_INDEX] == B0W1) {
				boardArray[ELEPHANT_INDEX] = B1W1;
			} else {         // this should not ever happen (implies more than 2 of this piece)
				perror("Error: more than two elephants");
			}
			break;
		case BLACK_GIRAFFE:
			if (boardArray[GIRAFFE_INDEX] == B0W0) {
				boardArray[GIRAFFE_INDEX] = B0W1;
			} else if (boardArray[GIRAFFE_INDEX] == B1W0) {
				boardArray[GIRAFFE_INDEX] = B1W1;
			} else if (boardArray[GIRAFFE_INDEX] == B0W1) {
				boardArray[GIRAFFE_INDEX] = B0W2;
			} else {         // this should not ever happen (implies more than 2 of this piece)
				perror("Error: more than two giraffes");
			}
			break;
		case WHITE_GIRAFFE:
			if (boardArray[GIRAFFE_INDEX] == B0W0) {
				boardArray[GIRAFFE_INDEX] = B1W0;
			} else if (boardArray[GIRAFFE_INDEX] == B1W0) {
				boardArray[GIRAFFE_INDEX] = B2W0;
			} else if (boardArray[GIRAFFE_INDEX] == B0W1) {
				boardArray[GIRAFFE_INDEX] = B1W1;
			} else {         // this should not ever happen (implies more than 2 of this piece)
				perror("Error: more than two giraffes");
			}
			break;
		default:
			break;
		}
	} else { // moves 96 ~ 131 are drops
		int tmp = move % 96;
		int location = tmp / 6;
		tmp = tmp % 6;
		switch (tmp) {
		case DROP_BLACK_CHICK:
			boardArray[location] = BLACK_CHICK;
			if (boardArray[CHICK_INDEX] == B1W0) {
				boardArray[CHICK_INDEX] = B0W0;
			} else if (boardArray[CHICK_INDEX] == B1W1) {
				boardArray[CHICK_INDEX] = B0W1;
			} else if (boardArray[CHICK_INDEX] == B2W0) {
				boardArray[CHICK_INDEX] = B1W0;
			} else {
				perror("Error: black dropped a chick without one in hand");
				break;
			}
			break;
		case DROP_BLACK_ELEPHANT:
			boardArray[location] = BLACK_ELEPHANT;
			if (boardArray[ELEPHANT_INDEX] == B1W0) {
				boardArray[ELEPHANT_INDEX] = B0W0;
			} else if (boardArray[ELEPHANT_INDEX] == B1W1) {
				boardArray[ELEPHANT_INDEX] = B0W1;
			} else if (boardArray[ELEPHANT_INDEX] == B2W0) {
				boardArray[ELEPHANT_INDEX] = B1W0;
			} else {
				perror("Error: black dropped a elephant without one in hand");
				break;
			}
			break;
		case DROP_BLACK_GIRAFFE:
			boardArray[location] = BLACK_GIRAFFE;
			if (boardArray[GIRAFFE_INDEX] == B1W0) {
				boardArray[GIRAFFE_INDEX] = B0W0;
			} else if (boardArray[GIRAFFE_INDEX] == B1W1) {
				boardArray[GIRAFFE_INDEX] = B0W1;
			} else if (boardArray[GIRAFFE_INDEX] == B2W0) {
				boardArray[GIRAFFE_INDEX] = B1W0;
			} else {
				perror("Error: black dropped a giraffe without one in hand");
				break;
			}
			break;
		case DROP_WHITE_CHICK:
			boardArray[location] = WHITE_CHICK;
			if (boardArray[CHICK_INDEX] == B0W1) {
				boardArray[CHICK_INDEX] = B0W0;
			} else if (boardArray[CHICK_INDEX] == B1W1) {
				boardArray[CHICK_INDEX] = B1W0;
			} else if (boardArray[CHICK_INDEX] == B0W2) {
				boardArray[CHICK_INDEX] = B0W1;
			} else {
				perror("Error: black dropped a chick without one in hand");
				break;
			}
			break;
		case DROP_WHITE_ELEPHANT:
			boardArray[location] = WHITE_ELEPHANT;
			if (boardArray[ELEPHANT_INDEX] == B0W1) {
				boardArray[ELEPHANT_INDEX] = B0W0;
			} else if (boardArray[ELEPHANT_INDEX] == B1W1) {
				boardArray[ELEPHANT_INDEX] = B1W0;
			} else if (boardArray[ELEPHANT_INDEX] == B0W2) {
				boardArray[ELEPHANT_INDEX] = B0W1;
			} else {
				perror("Error: black dropped a elephant without one in hand");
				break;
			}
			break;
		case DROP_WHITE_GIRAFFE:
			boardArray[location] = WHITE_GIRAFFE;
			if (boardArray[GIRAFFE_INDEX] == B0W1) {
				boardArray[GIRAFFE_INDEX] = B0W0;
			} else if (boardArray[GIRAFFE_INDEX] == B1W1) {
				boardArray[GIRAFFE_INDEX] = B1W0;
			} else if (boardArray[GIRAFFE_INDEX] == B0W2) {
				boardArray[GIRAFFE_INDEX] = B0W1;
			} else {
				perror("Error: black dropped a giraffe without one in hand");
				break;
			}
			break;
		default:
			perror("Error: bad drop hash");
			break;
		}
	}

	if (currentPlayer == BLACK_TURN) {
		currentPlayer = WHITE_TURN;
	} else {
		currentPlayer = BLACK_TURN;
	}

	return hash(boardArray, currentPlayer);

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

MOVELIST *GenerateMoves(POSITION position) {
	MOVELIST *moves = NULL;
	int currentPlayer;
	int index;
	char board[rows*cols + 4];
	char piece;
	unhash(position, board);
	unhash_turn(position, &currentPlayer);

	for (index = 0; index < (rows * cols); index++) {
		piece = board[index];
		if (sameTeam(piece, currentPlayer)) {
			switch (piece) {
			case BLACK_LION:
				if (index != 0 && index != 1 && index != 2 && index != 3 && index != 6 && index != 9)
					generateMoveWithDir(board, &moves, currentPlayer, index, UL);
				if (index != 0 && index != 1 && index != 2)
					generateMoveWithDir(board, &moves, currentPlayer, index, UP);
				if (index != 0 && index != 1 && index != 2 && index != 5 && index != 8 && index != 11)
					generateMoveWithDir(board, &moves, currentPlayer, index, UR);
				if (index != 0 && index != 3 && index != 6 && index != 9)
					generateMoveWithDir(board, &moves, currentPlayer, index, LT);
				if (index != 2 && index != 5 && index != 8 && index != 11)
					generateMoveWithDir(board, &moves, currentPlayer, index, RT);
				if (index != 0 && index != 3 && index != 6 && index != 9 && index != 10 && index != 11)
					generateMoveWithDir(board, &moves, currentPlayer, index, DL);
				if (index != 9 && index != 10 && index != 11)
					generateMoveWithDir(board, &moves, currentPlayer, index, DN);
				if (index != 2 && index != 5 && index != 8 && index != 9 && index != 10 && index != 11)
					generateMoveWithDir(board, &moves, currentPlayer, index, DR);
				break;
			case BLACK_CHICK:
				if (index != 0 && index != 1 && index != 2)
					generateMoveWithDir(board, &moves, currentPlayer, index, UP);
				break;
			case BLACK_HEN:
				if (index != 0 && index != 1 && index != 2 && index != 3 && index != 6 && index != 9)
					generateMoveWithDir(board, &moves, currentPlayer, index, UL);
				if (index != 0 && index != 1 && index != 2)
					generateMoveWithDir(board, &moves, currentPlayer, index, UP);
				if (index != 0 && index != 1 && index != 2 && index != 5 && index != 8 && index != 11)
					generateMoveWithDir(board, &moves, currentPlayer, index, UR);
				if (index != 0 && index != 3 && index != 6 && index != 9)
					generateMoveWithDir(board, &moves, currentPlayer, index, LT);
				if (index != 2 && index != 5 && index != 8 && index != 11)
					generateMoveWithDir(board, &moves, currentPlayer, index, RT);
				if (index != 9 && index != 10 && index != 11)
					generateMoveWithDir(board, &moves, currentPlayer, index, DN);
				break;
			case BLACK_ELEPHANT:
				if (index != 0 && index != 1 && index != 2 && index != 3 && index != 6 && index != 9)
					generateMoveWithDir(board, &moves, currentPlayer, index, UL);
				if (index != 0 && index != 1 && index != 2 && index != 5 && index != 8 && index != 11)
					generateMoveWithDir(board, &moves, currentPlayer, index, UR);
				if (index != 0 && index != 3 && index != 6 && index != 9 && index != 10 && index != 11)
					generateMoveWithDir(board, &moves, currentPlayer, index, DL);
				if (index != 2 && index != 5 && index != 8 && index != 9 && index != 10 && index != 11)
					generateMoveWithDir(board, &moves, currentPlayer, index, DR);
				break;
			case BLACK_GIRAFFE:
				if (index != 0 && index != 1 && index != 2)
					generateMoveWithDir(board, &moves, currentPlayer, index, UP);
				if (index != 0 && index != 3 && index != 6 && index != 9)
					generateMoveWithDir(board, &moves, currentPlayer, index, LT);
				if (index != 2 && index != 5 && index != 8 && index != 11)
					generateMoveWithDir(board, &moves, currentPlayer, index, RT);
				if (index != 9 && index != 10 && index != 11)
					generateMoveWithDir(board, &moves, currentPlayer, index, DN);
				break;
			case WHITE_LION:
				if (index != 0 && index != 1 && index != 2 && index != 3 && index != 6 && index != 9)
					generateMoveWithDir(board, &moves, currentPlayer, index, UL);
				if (index != 0 && index != 1 && index != 2)
					generateMoveWithDir(board, &moves, currentPlayer, index, UP);
				if (index != 0 && index != 1 && index != 2 && index != 5 && index != 8 && index != 11)
					generateMoveWithDir(board, &moves, currentPlayer, index, UR);
				if (index != 0 && index != 3 && index != 6 && index != 9)
					generateMoveWithDir(board, &moves, currentPlayer, index, LT);
				if (index != 2 && index != 5 && index != 8 && index != 11)
					generateMoveWithDir(board, &moves, currentPlayer, index, RT);
				if (index != 0 && index != 3 && index != 6 && index != 9 && index != 10 && index != 11)
					generateMoveWithDir(board, &moves, currentPlayer, index, DL);
				if (index != 9 && index != 10 && index != 11)
					generateMoveWithDir(board, &moves, currentPlayer, index, DN);
				if (index != 2 && index != 5 && index != 8 && index != 9 && index != 10 && index != 11)
					generateMoveWithDir(board, &moves, currentPlayer, index, DR);
				break;
			case WHITE_CHICK:
				if (index != 9 && index != 10 && index != 11)
					generateMoveWithDir(board, &moves, currentPlayer, index, DN);
				break;
			case WHITE_HEN:
				if (index != 0 && index != 1 && index != 2)
					generateMoveWithDir(board, &moves, currentPlayer, index, UP);
				if (index != 0 && index != 3 && index != 6 && index != 9)
					generateMoveWithDir(board, &moves, currentPlayer, index, LT);
				if (index != 2 && index != 5 && index != 8 && index != 11)
					generateMoveWithDir(board, &moves, currentPlayer, index, RT);
				if (index != 0 && index != 3 && index != 6 && index != 9 && index != 10 && index != 11)
					generateMoveWithDir(board, &moves, currentPlayer, index, DL);
				if (index != 9 && index != 10 && index != 11)
					generateMoveWithDir(board, &moves, currentPlayer, index, DN);
				if (index != 2 && index != 5 && index != 8 && index != 9 && index != 10 && index != 11)
					generateMoveWithDir(board, &moves, currentPlayer, index, DR);
				break;
			case WHITE_ELEPHANT:
				if (index != 0 && index != 1 && index != 2 && index != 3 && index != 6 && index != 9)
					generateMoveWithDir(board, &moves, currentPlayer, index, UL);
				if (index != 0 && index != 1 && index != 2 && index != 5 && index != 8 && index != 11)
					generateMoveWithDir(board, &moves, currentPlayer, index, UR);
				if (index != 0 && index != 3 && index != 6 && index != 9 && index != 10 && index != 11)
					generateMoveWithDir(board, &moves, currentPlayer, index, DL);
				if (index != 2 && index != 5 && index != 8 && index != 9 && index != 10 && index != 11)
					generateMoveWithDir(board, &moves, currentPlayer, index, DR);
				break;
			case WHITE_GIRAFFE:
				if (index != 0 && index != 1 && index != 2)
					generateMoveWithDir(board, &moves, currentPlayer, index, UP);
				if (index != 0 && index != 3 && index != 6 && index != 9)
					generateMoveWithDir(board, &moves, currentPlayer, index, LT);
				if (index != 2 && index != 5 && index != 8 && index != 11)
					generateMoveWithDir(board, &moves, currentPlayer, index, RT);
				if (index != 9 && index != 10 && index != 11)
					generateMoveWithDir(board, &moves, currentPlayer, index, DN);
				break;
			default:
				break;
			}
		} else if (board[index] == BLANK_PIECE) {
			generateDropMoves(board, &moves, currentPlayer, index);
		}
	}
/*
        for (r = 0; r < rows; r++) {
                for (c = 0; c < cols; c++) {
                        piece = boardArray[coordToNum(r, c)];
                        if (sameTeam(piece, currentPlayer)) { // Generate moves for pieces owned by current player on board;
                                switch (piece) {
                                        case BLACK_CHICK:
                                                generateMoveWithDir(boardArray, &moves, currentPlayer, r, c, UP);
                                                break;
                                        case WHITE_CHICK:
                                                generateMoveWithDir(boardArray, &moves, currentPlayer, r, c, DN);
                                                break;
                                        case BLACK_HEN:
                                                generateMoveWithDir(boardArray, &moves, currentPlayer, r, c, UP);
                                                generateMoveWithDir(boardArray, &moves, currentPlayer, r, c, UL);
                                                generateMoveWithDir(boardArray, &moves, currentPlayer, r, c, UR);
                                                generateMoveWithDir(boardArray, &moves, currentPlayer, r, c, LT);
                                                generateMoveWithDir(boardArray, &moves, currentPlayer, r, c, RT);
                                                generateMoveWithDir(boardArray, &moves, currentPlayer, r, c, DN);
                                                break;
                                        case WHITE_HEN:
                                                generateMoveWithDir(boardArray, &moves, currentPlayer, r, c, UP);
                                                generateMoveWithDir(boardArray, &moves, currentPlayer, r, c, LT);
                                                generateMoveWithDir(boardArray, &moves, currentPlayer, r, c, RT);
                                                generateMoveWithDir(boardArray, &moves, currentPlayer, r, c, DL);
                                                generateMoveWithDir(boardArray, &moves, currentPlayer, r, c, DN);
                                                generateMoveWithDir(boardArray, &moves, currentPlayer, r, c, DR);
                                                break;
                                        case BLACK_GIRAFFE:
                                                generateMoveWithDir(boardArray, &moves, currentPlayer, r, c, UP);
                                                generateMoveWithDir(boardArray, &moves, currentPlayer, r, c, LT);
                                                generateMoveWithDir(boardArray, &moves, currentPlayer, r, c, RT);
                                                generateMoveWithDir(boardArray, &moves, currentPlayer, r, c, DN);
                                                break;
                                        case WHITE_GIRAFFE:
                                                generateMoveWithDir(boardArray, &moves, currentPlayer, r, c, UP);
                                                generateMoveWithDir(boardArray, &moves, currentPlayer, r, c, LT);
                                                generateMoveWithDir(boardArray, &moves, currentPlayer, r, c, RT);
                                                generateMoveWithDir(boardArray, &moves, currentPlayer, r, c, DN);
                                                break;
                                        case BLACK_ELEPHANT:
                                                generateMoveWithDir(boardArray, &moves, currentPlayer, r, c, UL);
                                                generateMoveWithDir(boardArray, &moves, currentPlayer, r, c, UR);
                                                generateMoveWithDir(boardArray, &moves, currentPlayer, r, c, DL);
                                                generateMoveWithDir(boardArray, &moves, currentPlayer, r, c, DR);
                                                break;
                                        case WHITE_ELEPHANT:
                                                generateMoveWithDir(boardArray, &moves, currentPlayer, r, c, UL);
                                                generateMoveWithDir(boardArray, &moves, currentPlayer, r, c, UR);
                                                generateMoveWithDir(boardArray, &moves, currentPlayer, r, c, DL);
                                                generateMoveWithDir(boardArray, &moves, currentPlayer, r, c, DR);
                                                break;
                                        case BLACK_LION:
                                                generateMoveWithDir(boardArray, &moves, currentPlayer, r, c, UP);
                                                generateMoveWithDir(boardArray, &moves, currentPlayer, r, c, LT);
                                                generateMoveWithDir(boardArray, &moves, currentPlayer, r, c, RT);
                                                generateMoveWithDir(boardArray, &moves, currentPlayer, r, c, DN);
                                                generateMoveWithDir(boardArray, &moves, currentPlayer, r, c, UL);
                                                generateMoveWithDir(boardArray, &moves, currentPlayer, r, c, UR);
                                                generateMoveWithDir(boardArray, &moves, currentPlayer, r, c, DL);
                                                generateMoveWithDir(boardArray, &moves, currentPlayer, r, c, DR);
                                                break;
                                        case WHITE_LION:
                                                generateMoveWithDir(boardArray, &moves, currentPlayer, r, c, UP);
                                                generateMoveWithDir(boardArray, &moves, currentPlayer, r, c, LT);
                                                generateMoveWithDir(boardArray, &moves, currentPlayer, r, c, RT);
                                                generateMoveWithDir(boardArray, &moves, currentPlayer, r, c, DN);
                                                generateMoveWithDir(boardArray, &moves, currentPlayer, r, c, UL);
                                                generateMoveWithDir(boardArray, &moves, currentPlayer, r, c, UR);
                                                generateMoveWithDir(boardArray, &moves, currentPlayer, r, c, DL);
                                                generateMoveWithDir(boardArray, &moves, currentPlayer, r, c, DR);
                                                break;
                                        default:
                                                break;
                                }
                        } else if (piece == BLANK_PIECE) {
                                generateDropMoves(boardArray, &moves, currentPlayer, r, c);
                        }
                }
        }
 */
	return moves;
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
	//printf("primitive");
	MOVELIST *moves = NULL;
	moves = GenerateMoves(position);
	if (moves == NULL) {
		return (gStandardGame) ? lose : win;
	} else {
		return undecided;
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

void PrintPosition (POSITION position, STRING playersName, BOOLEAN usersTurn)
{
	int numBC, numBE, numBG, numWC, numWE, numWG;
	char boardArray[rows*cols+3];
	unhash(position, boardArray);
	switch (boardArray[CHICK_INDEX]) {
	case B0W0:
		numBC = 0;
		numWC = 0;
		break;
	case B1W0:
		numBC = 1;
		numWC = 0;
		break;
	case B0W1:
		numBC = 0;
		numWC = 1;
		break;
	case B1W1:
		numBC = 1;
		numWC = 1;
		break;
	case B2W0:
		numBC = 2;
		numWC = 0;
		break;
	case B0W2:
		numBC = 0;
		numWC = 2;
		break;
	default:
		numBC = 0;
		numWC = 0;
		break;
	}

	switch (boardArray[ELEPHANT_INDEX]) {
	case B0W0:
		numBE = 0;
		numWE = 0;
		break;
	case B1W0:
		numBE = 1;
		numWE = 0;
		break;
	case B0W1:
		numBE = 0;
		numWE = 1;
		break;
	case B1W1:
		numBE = 1;
		numWE = 1;
		break;
	case B2W0:
		numBE = 2;
		numWE = 0;
		break;
	case B0W2:
		numBE = 0;
		numWE = 2;
		break;
	default:
		numBE = 0;
		numWE = 0;
		break;
	}

	switch (boardArray[GIRAFFE_INDEX]) {
	case B0W0:
		numBG = 0;
		numWG = 0;
		break;
	case B1W0:
		numBG = 1;
		numWG = 0;
		break;
	case B0W1:
		numBG = 0;
		numWG = 1;
		break;
	case B1W1:
		numBG = 1;
		numWG = 1;
		break;
	case B2W0:
		numBG = 2;
		numWG = 0;
		break;
	case B0W2:
		numBG = 0;
		numWG = 2;
		break;
	default:
		numBG = 0;
		numWG = 0;
		break;
	}

	printf("\n");
	printf("===== %s's Turn =====", playersName);
	printf("\n");
	printf("☖ Hand                                    Legend:\n");
	printf("╔════╗  ╔════╤════╤════╗             ╔════╤════╤════╗\n");
	printf("║☖C %d║  ║ %s │ %s │ %s ║             ║ 00 │ 01 │ 02 ║\n", numWC, pieceToPrint(boardArray[0]), pieceToPrint(boardArray[1]), pieceToPrint(boardArray[2]));
	printf("║☖E %d║  ╟────┼────┼────╢             ╟────┼────┼────╢\n", numWE);
	printf("║☖G %d║  ║ %s │ %s │ %s ║             ║ 03 │ 04 │ 05 ║\n", numWG, pieceToPrint(boardArray[3]), pieceToPrint(boardArray[4]), pieceToPrint(boardArray[5]));
	printf("╚════╝  ╟────┼────┼────╢  ╔════╗     ╟────┼────┼────╢\n");
	printf("        ║ %s │ %s │ %s ║  ║☗G %d║     ║ 06 │ 07 │ 08 ║\n", pieceToPrint(boardArray[6]), pieceToPrint(boardArray[7]), pieceToPrint(boardArray[8]), numBG);
	printf("        ╟────┼────┼────╢  ║☗E %d║     ╟────┼────┼────╢\n", numBE);
	printf("        ║ %s │ %s │ %s ║  ║☗C %d║     ║ 09 │ 10 │ 11 ║\n", pieceToPrint(boardArray[9]), pieceToPrint(boardArray[10]), pieceToPrint(boardArray[11]), numBC);
	printf("        ╚════╧════╧════╝  ╚════╝     ╚════╧════╧════╝\n");
	printf("                          ☗ Hand\n");
	printf("\n"); // need to fix stuff after this line
	//printf("%s\n",GetPrediction(position,playersName,usersTurn));
	//unhash_turn(position, &theCurrentPlayer);
	//printf("It is %s's turn (%s).\n",playersName,(theCurrentPlayer == BLACK_TURN) ? "black/lowercase":"white/uppercase");
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

STRING MoveToString(MOVE move) {
	STRING str = SafeMalloc(4 * sizeof(char));
	STRING str2 = SafeMalloc(4 * sizeof(char));
	int rv;
	int cv;
	int location;
	int tmp;
	if (move < 96) { // The move is a move
		tmp = move % 8;
		switch (tmp) {
		case 0:         // move ul
			rv = -1;
			cv = -1;
			break;
		case 1:         // move up
			rv = -1;
			cv = 0;
			break;
		case 2:         // move ur
			rv = -1;
			cv = 1;
			break;
		case 3:         // move lt
			rv = 0;
			cv = -1;
			break;
		case 4:         // move rt
			rv = 0;
			cv = 1;
			break;
		case 5:         // move dl
			rv = 1;
			cv = -1;
			break;
		case 6:         // move dn
			rv = 1;
			cv = 0;
			break;
		case 7:         // move dr
			rv = 1;
			cv = 1;
			break;
		default:
			rv = 0;
			cv = 0;
			break;
		}
		location = move / 8;
		tmp = location + (cols * rv) + cv;
		sprintf(str, "%02d%02d", location, tmp);
	} else { // The move is a drop
		tmp = move % 96;
		location = tmp / 6;
		tmp = tmp % 6;
		switch (tmp) {
		case DROP_BLACK_CHICK:
			sprintf(str, "*c%02d", location);
			break;
		case DROP_WHITE_CHICK:
			sprintf(str, "*C%02d", location);
			break;
		case DROP_BLACK_ELEPHANT:
			sprintf(str, "*e%02d", location);
			break;
		case DROP_WHITE_ELEPHANT:
			sprintf(str, "*E%02d", location);
			break;
		case DROP_BLACK_GIRAFFE:
			sprintf(str, "*g%02d", location);
			break;
		case DROP_WHITE_GIRAFFE:
			sprintf(str, "*G%02d", location);
			break;
		}
	}
	sprintf(str2, "%04d", move);
	return str;
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
	STRING str = MoveToString(move);
	printf( "%s", str );
	SafeFree( str );
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

	for (;; ) {
		/***********************************************************
		* CHANGE THE LINE BELOW TO MATCH YOUR MOVE FORMAT
		***********************************************************/
		printf("%8s's move [<beginning index><end index>/<*><piece><drop index>] : ", playersName);

		input = HandleDefaultTextInput(position, move, playersName);

		if (input == Continue)
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
**************************************************************************/

BOOLEAN ValidTextInput (STRING input) {
	int length = strlen(input);
	char c;
	if (length < 4) {
		return FALSE;
	} else {
		c = input[0];
		if (c == '*') { // a drop move
			c = input[1];
			if (c != 'c' && c != 'e' && c != 'g' && c != 'C' && c != 'E' && c != 'G') {
				return FALSE;
			}
			c = input[2];
			if (c < '0' || c > '1') {
				return FALSE;
			}

			if (c == '0') {
				c = input[3];
				if (c < '0' || c > '9') {
					return FALSE;
				}
			} else if (c == '1') {
				c = input[3];
				if (c < '0' || c > '1') {
					return FALSE;
				}
			} else {
				return FALSE;
			}
		} else { // otherwise it is a move
			if (c < '0' || c > '1') {
				return FALSE;
			}
			if (c == '0') {
				c = input[1];
				if (c < '0' || c > '9') {
					return FALSE;
				}
			} else if (c == '1') {
				c = input[1];
				if (c < '0' || c > '1') {
					return FALSE;
				}
			} else {
				return FALSE;
			}

			c = input[2];
			if (c < '0' || c > '1') {
				return FALSE;
			}
			if (c == '0') {
				c = input[3];
				if (c < '0' || c > '9') {
					return FALSE;
				}
			} else if (c == '1') {
				c = input[3];
				if (c < '0' || c > '1') {
					return FALSE;
				}
			} else {
				return FALSE;
			}
		}
	}
	return TRUE;
}

/*
   BOOLEAN ValidTextInput (STRING input)
   {
   char c;
        int length = strlen(input);
        if(length==4 || length==6 )
                return FALSE;
        else{
 */
/* Make sure the row and column are within the bounds
   set by the current board size. */
/*
                c = input[0];
                if (c < 'a' || c >= 'a' + cols) return FALSE;
                c = input[2];
                if (c < 'a' || c >= 'a' + cols) return FALSE;
                c = input[1];
                if(c > rows + '0' || c < '1') return FALSE;
                c = input[3];
                if(c > rows + '0' || c < '1') return FALSE;
                if (length == 6) {  // pawn replacement move
                        c = input[4];
                        if (c == '=') return FALSE;
                        c = input [5];
                        if (c == 'Q' || c == 'q' || c == 'R' || c == 'r'
 || c == 'B' || c == 'b') return FALSE;
                }
        }
        return TRUE;

   }
 */

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
	printf("MOVESTR: %s\n", input);
	MOVE move;
	int start, finish;
	char c = input[0];
	if (c == '*') { // then it is a drop move
		move = 96;
		c = input[2];
		if (c == '0') {
			c = input[3];
			finish = c - '0';
		} else {
			c = input[3];
			finish = 10 + c - '0';
		}
		c = input[1];

		move = createDrop(c, finish);
	} else { // then move (all 4 characters must be numbers)
		move = 0;
		if (c == '0') { // 2nd digit can be anything
			c = input[1];
			start = c - '0';
		} else { // then it must begin with a 1
			c = input[1];
			start = 10 + (c - '0');
		}
		c = input[2];
		if (c == '0') { // 3rd digit can only be 1 or 0
			c = input[3];
			finish = c - '0';
		} else {
			c = input[3];
			finish = 10 + (c - '0');
		}

		move = createMove(start, finish);
	}
	printf("MOVE: %d\n", move);
	return move;
}

/*
   MOVE ConvertTextInputToMove (STRING input)
   {
   MOVE m = 0;
   return m;
   }
 */


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

	char *board = NULL;
	char c;
	printf("\n");
	printf("Tile Chess Game Specific Menu\n\n");
	printf("1) Normal QuickChess Play\n");
	printf("2) Misere Variant\n");
	printf("3) Customize Initial Board\n");
	printf("b) Back to previous menu\n\n");

	printf("Select an option: ");

	switch(GetMyChar()) {
	case 'Q': case 'q':
		ExitStageRight();
	case '1':
		normalVariant = TRUE;
		misereVariant = FALSE;
		break;
	case '2':
		misereVariant = TRUE;
		normalVariant = FALSE;
		break;
	case '3':
		printf("Each player MUST have one\n\
king and all other pieces may not exceed the number allowed in\n\
traditional quickchess. Only 5x6 and 3x4 boards allowed.Insert dashes for blanks.\n\
Press enter to start on a new row, and press enter twice when you are\n\
finished describing your last row.  Here is a sample board:\n\
k--\n\
---\n\
---\n\
KB-\n\
The valid pieces are:\n\
K = king, P = pawn, Q = queen, B = bishop, R = rook, N = knight\n\
Upper-case letters indicate the pieces that belong to white, and\n\
lower-case letters indicate the pieces that belong to black:\n\n");
		do {
			if (board == NULL) {
				printf("Illegal board, re-enter:\n");
			}
			printf("Please Choose Board Size, 1=3x4, 2=5x6: ");
			switch(GetMyChar()) {
			case '1':
				break;
			case '2':
				rows = 6;
				cols = 5;
				break;
			default:
				printf("Wrong Entry! Default will be 3x4 board\n\n");
				break;
			}
			printf("Enter Board Now\n\n");
			theBoard = getBoard();
			do {
				printf("Whose turn is it? (w/b): \n");
				c = GetMyChar();
			} while (c == 'w' || c == 'b');
			theCurrentPlayer = (c == 'w') ? WHITE_TURN : BLACK_TURN;
			generic_hash_context_switch(gInitialTier);
			gInitialTierPosition = generic_hash_hash(theBoard, theCurrentPlayer);
		} while(theBoard == NULL);
		break;
	case 'b': case 'B':
		return;
	default:
		printf("\nSorry, I don't know that option. Try another.\n");
		HitAnyKeyToContinue();
		GameSpecificMenu();
		break;
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

	return 2;
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
	if (normalVariant)
		return 1;
	else if (misereVariant)
		return 2;
	else
		BadElse("getOption");

	return -1;

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
	if (option == 1) {
		normalVariant = TRUE;
		misereVariant = FALSE;
	} else if (option == 2) {
		normalVariant = FALSE;
		misereVariant = TRUE;
	}
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
	printf("theboard:%s\n", theBoard);
	//MOVE m;
	/*
	   int coli, rowi, colf, rowf;
	   coli = 10;
	   colf = 10;
	   rowi = 4;
	   rowf = 3;
	   m = (coli << 12) | (rowi << 8) | (colf << 4) | rowf;
	 */
	//	m = createMove(0,0,1,0);
	/*
	   printf("input move is %s\n", "a4a1");
	   m = ConvertTextInputToMove("a4a1");
	   PrintMove(m);
	 */
	//PrintPosition(gInitialPosition, "me", TRUE);
	/*POSITION newPos = DoMove(gInitialPosition,
	   PrintPosition(newPos, "me", TRUE);
	   m = ConvertTextInputToMove("b1a2");
	   newPos = DoMove(newPos, m);
	   PrintPosition(newPos, "me", TRUE);
	 */
	/*
	   m = ConvertTextInputToMove("b1c1");
	   PrintPosition(gInitialPosition, "me", TRUE);
	   POSITION newPos = DoMove(gInitialPosition,m);
	   PrintPosition(newPos, "me", TRUE);
	   if(inCheck(newPos, 1)) {
	   printf("player 1 in check\n");
	   }
	   if(inCheck(gInitialPosition, 2)) {
	   printf("player 2 in check\n");
	   }
	 */
	/*
	   int i;
	   char bA[rows*cols];
	   printf("the number of spaces is: %d\n", rows*cols);
	   unhash(gInitialPosition, bA);

	   for(i = 0; i < BOARDSIZE; i++) {
	   printf("seeing whats on the board on space %d: %c\n", i, bA[i]);
	   }

	   unhash(flipLR(gInitialPosition), bA);

	   for(i = 0; i < BOARDSIZE; i++) {
	   printf("seeing whats on the board on space %d: %c\n", i, bA[i]);
	   }
	 */
	/*
	   MOVE m1, m2;
	   STRING s1, s2;
	   m1 = createMove(1, 2, 2, 3);
	   m2 = createPawnMove(1, 3, 4, 5, WHITE_PAWN);
	   s1 = MoveToString(m1);
	   s2 = MoveToString(m2);

	   printf("%s\n", s1);
	   printf("%s\n", s2);
	   SafeFree(s1);
	   SafeFree(s2);
	 */
	//int i;
	//printf("Islegal returns %d for this board\nThe number of positions in tier 3 is %d", IsLegal(gInitialPosition), NumberOfTierPositions((TIER)4));
	//for(i = 0; i < 40; i++) {
	//printf("tier %d has %d positions\n", i, NumberOfTierPositions((TIER)i));
	//}
	/*
	   UNDOMOVE m;
	   m = createReplaceCaptureUndoMove(0, 0, 1, 1, 0);

	   PrintPosition(gInitialPosition, "me", TRUE);
	   PrintPosition(gTUndoMove(gInitialPosition, m), "me", TRUE);
	 */
	// PrintPosition(gInitialPosition, "me", TRUE);
	//printf("yes\n");
	//printUndoMoveList(gGenerateUndoMovesToTier(gInitialPosition, 2));

	/* TIER t;

	   //int *piecesArray = (int *) malloc(DISTINCT_PIECES * sizeof(int));
	   t = gPositionToTier(gInitialPosition);
	   printf("%d\n", t);
	   PrintPosition(gInitialPosition, "me", TRUE);
	   //printf("%s %s", TierToString(19),  TierToString(3));
	   //printf("the tier value is %d in decimal and %x in hex\n", t, t);
	   //printTierList(gTierChildren(t));
	   //printPiecesArray(gTierToPiecesArray(257, piecesArray));
	   //free(piecesArray);
	   printf("The KkBR tier moves\n");
	   printUndoMoveList(gGenerateUndoMovesToTier (gInitialPosition, 9));
	   printf("The KkR tier moves\n");
	   printUndoMoveList(gGenerateUndoMovesToTier (gInitialPosition, 137));
	   printf("The KkB tier moves\n");
	 */
	//printUndoMoveList(gGenerateUndoMovesToTier (gInitialPosition, 2));
	/* int i , zeroPiece = 0, onePiece = 1, twoPiece = 9, threePiece = 37, fourPiece = 93, fivePiece = 163, sixPiece = 219, sevenPiece = 247, eightPiece = 255, numBits;
	   i = countBits(223);;
	   printf("numBits in i = %d\n", i);
	   TIER tierlist[NUM_TIERS];
	   // Replace "NUM_TIERS" with how many total tiers your game has
	   for(i = 0; i < NUM_TIERS; i++) {
	   numBits = countBits(i);
	   switch(numBits) {
	   case 0:
	    tierlist[zeroPiece] = i;
	    zeroPiece++;
	    break;
	   case 1:
	    tierlist[onePiece] = i;
	    onePiece++;
	    break;
	   case 2:
	    tierlist[twoPiece] = i;
	    twoPiece++;
	    break;
	   case 3:
	    tierlist[threePiece] = i;
	    threePiece++;
	    break;
	   case 4:
	    tierlist[fourPiece] = i;
	    fourPiece++;
	    break;
	   case 5:
	    tierlist[fivePiece] = i;
	    fivePiece++;
	    break;
	   case 6:
	    tierlist[sixPiece] = i;
	    sixPiece++;
	    break;
	   case 7:
	    tierlist[sevenPiece] = i;
	    sevenPiece++;
	    break;
	   case 8:
	    tierlist[eightPiece] = i;
	    eightPiece++;
	    break;

	   }
	   }
	   printTierArray(tierlist);
	 */
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




/************************************************************************
**
** NAME:        inCheck
**
** DESCRIPTION: inCheck determines whether the given player is Checked on the
**              current Board.
**
** INPUTS:      char* bA   : The current Board.
**              int player : The current player.
**
** OUTPUTS:     BOOLEAN    : Whether or not the current player is in check.
**
************************************************************************/



/************************************************************************
**
** NAMES:       generate"Piece"Moves
**
** DESCRIPTION: Generates all the legal moves and puts them into the movelist
**              for the given board and player
**
** INPUTS:      char* bA   : The current Board.
**              int player : The current player.
**              int i, j   : coordinates of spot on board being examined
**
************************************************************************/


/*
** Generates the moves for a king piece.  First checks if the move is off the board.  Then
** checks of the piece taken, if any, is of the same team.  Then checks if the move puts
** the player in check.  Generates moves in all directions until it hits a piece.
*/

/*
** Generates the moves for a king piece.  First checks if the move is off the board.  Then
** checks of the piece taken, if any, is of the same team.  Then checks if the move puts
** the player in check.  Generates moves in the four diagonal directions until it hits a piece.
*/


/*
** Generates the moves for a king piece.  First checks if the move is off the board.  Then
** checks of the piece taken, if any, is of the same team.  Then checks if the move puts
** the player in check.  Generates moves in 4 compass directions until it hits a piece.
*/


/*
** Generates the moves for a king piece.  First checks if the move is off the board.  Then
** checks of the piece taken, if any, is of the same team.  Then checks if the move puts
** the player in check.  There are 8 possible moves for a knight.  The first direction indicates
** 2 blocks of move, the second direction is one block.
*/


/*
** Generates the moves for a king piece.  First checks if the move is off the board.  Then
** checks of the piece taken, if any, is of the same team.  Then checks if the move puts
** the player in check.  Calculates moves forward one piece if there is no piece there
** or moves diagonal forward if there is an opposing player piece there.
*/

/*
   Given a board and a list of moves, this function will create all moves
   in that direction until it hits another piece.  This is for the queen, bishop,
   and rook.  These three pieces can move in directions any number of spaces until
   they hit another piece.  They can only take that piece if it is of the other team.
   POSTCONDITION: moves is updated with all of the legal moves.
 */


/*
** Tests to see if a move is valid by checking to see if the new board is in check
** Returns FALSE if the move puts the player in check.
*/


/* Returns TRUE if the given piece belongs to currentPlayer,
   and FALSE otherwise. */


/* Returns TRUE if the given piece belongs to opposite of the currentPlayer,
   and FALSE otherwise. */


/* Used for testing.  this function will print an array, not a hash position
   like PrintPosition(); */


/* Used for testing.  This function will print a MOVELIST to show all the
   possible moves for the current Board and current player */
void printMoveList(MOVELIST *moves) {
	while(moves == NULL) {
		PrintMove(moves->move);
		printf("\n");
		moves = moves->next;
	}
}

BOOLEAN replacement(char *boardArray, char replacementPiece) {
	int i, j;

	for (i = 0; i < rows; i++) {
		for (j = 0; j < cols; j++) {
			if(boardArray[i*cols + j] == replacementPiece)
				return FALSE;
		}
	}
	return TRUE;
}

/************************************************************************
**
** NAME:        GetCanonicalPosition
**
** DESCRIPTION: Go through all of the positions that are symmetrically
**              equivalent and return the SMALLEST, which will be used
**              as the canonical element for the equivalence set.
**
** INPUTS:      POSITION position : The position return the canonical elt. of.
**
** OUTPUTS:     POSITION          : The canonical element of the set.
**
************************************************************************/

/************************************************************************
**
** NAME:        DoSymmetry
**
** DESCRIPTION: Perform the symmetry operation specified by the input
**              on the position specified by the input and return the
**              new position, even if it's the same as the input.
**
** INPUTS:      POSITION position : The position to branch the symmetry from.
**              int      symmetry : The number of the symmetry operation.
**
** OUTPUTS:     POSITION, The position after the symmetry operation.
**
************************************************************************/

/************************************************************************
**
** TIER GAMESMAN API
**
************************************************************************/





/************************************************************************
**
** NAME:        NumberOfTierPositions
**
** DESCRIPTION: Given a tier, determine the maximum number of positions
**              that are exclusive to that tier.
**
** INPUTS:      TIER tier : the given tier
**
** OUTPUTS:     TIERPOSITION : The max number position in the set of positions
**                             of the given tier
**
************************************************************************/


/************************************************************************
**
** NAME:        gPositionToTier
**
** DESCRIPTION: Takes a given position and determines which tier it belongs to.
**
** INPUTS:      POSITION position : The given position
**
** OUTPUTS:     TIER              : The tier the position translates to.
**
************************************************************************/



/************************************************************************
**
** NAME:        gTierChildren
**
** DESCRIPTION: Given a tier, this function determines all the tiers that can be
**              reached from this position by a move made, including the self tier and
**              returns all these tier in a tierlist
**
** INPUTS:      TIER t    : The given tier.
**
** OUTPUTS:     TIERLIST  : The children of the given tier
**
************************************************************************/


/************************************************************************
**
** NAME:        gUnDoMove
**
** DESCRIPTION: Given a position and an undomove, it does the undomove on the
**              position and returns the resulting position
**
**
** INPUTS:      UNDOMOVE umove    : The given undomove.
**              POSITION position : the given position
**
** OUTPUTS:     POSITION          : The resulting position from the undomove
**
************************************************************************/

/************************************************************************
**
** NAME:        gGenerateUndoMovesToTier
**
** DESCRIPTION: Generates all the undomoves of the current position that lead into
**              the given tier and puts them into a list.
**
**
** INPUTS:      TIER t            : The given tier.
**              POSITION position : the given position
**
** OUTPUTS:     UNDOMOVELIST  : The list of undomoves that lead from the given position
**                              into the given tier.
**
************************************************************************/



/************************************************************************
**
** NAME:        IsLegal
**
** DESCRIPTION: determines if the given position is reachable through gameplay
**
** INPUTS:      POSITION position : The given position.
**
** OUTPUTS:     BOOLEAN           : Whether or not the position is reachable.
**
************************************************************************/


/************************************************************************
**
** IsLegal Helper Functions
**
************************************************************************/


/************************************************************************
**
** TIER GAMESMAN API Helper Functions
**
************************************************************************/


/*
** Generates the moves for a king piece.  First checks if the move is off the board.  Then
** checks of the piece taken, if any, is of the same team.  Then checks if the move puts
** the player in check.  Generates moves in all directions until it hits a piece.
*/



/*
** Generates the moves for a king piece.  First checks if the move is off the board.  Then
** checks of the piece taken, if any, is of the same team.  Then checks if the move puts
** the player in check.  Generates moves in the four diagonal directions until it hits a piece.
*/


/*
** Generates the moves for a king piece.  First checks if the move is off the board.  Then
** checks of the piece taken, if any, is of the same team.  Then checks if the move puts
** the player in check.  Generates moves in 4 compass directions until it hits a piece.
*/


/*
   Given a board and a list of moves, this function will create all moves
   in that direction until it hits another piece.  This is for the queen, bishop,
   and rook.  These three pieces can move in directions any number of spaces until
   they hit another piece.  They can only take that piece if it is of the other team.
   POSTCONDITION: moves is updated with all of the legal moves.
 */



/* Used for testing.  This function will print a MOVELIST to show all the
   possible moves for the current Board and current player */
int countBits(int i) {
	int j, k = 1, counter = 0;
	for (j = 0; j < 32; j++) {
		if((((k << j) & i) == 0))
			counter++;
	}
	return counter;
}


/* Customizing The Board */

char *getBoard() {
	char *boardArray = SafeMalloc(sizeof(char) * rows*cols);
	int i = 0;
	char c, cPrev;
	while ((c = getchar())) {
		if(c == '\n' || c == 0) {
			if(c == '-') {
				boardArray[i] = ' ';
			}
			else {
				boardArray[i] = c;
			}
			i++;
		} else if (c == '\n' || cPrev == '\n') {
			break;
		}
		cPrev = c;
	}
	return boardArray;
}

BOOLEAN isLegalBoard(char *Board){
	int i;
	char piece;
	for (i = 0; i < rows*cols; i++) {
		piece = Board[i];
	}
	return TRUE;
}

int getNumPieces(int* piecesArray) {
	int numPieces = 0, i;
	for(i = 0; i < DISTINCT_PIECES-2; i++) {
		if(*(piecesArray+i) == 1)
			numPieces++;
	}
	return numPieces;
}


/************************************************************************
**
**  Hashing and Unhashing
**
**	NOTE: 36505976832 positions
**
************************************************************************/

POSITION StringToPosition(char* board) {
	// FIXME: this is just a stub
	return atoi(board);
}


char * PositionToEndData(POSITION pos) {
	return NULL;
}
