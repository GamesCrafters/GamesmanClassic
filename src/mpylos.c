/*
 * Pylos
 * Eric Siroker
 * February 11, 2004
 */

#include "gamesman.h"
#include "hash.h"

#define BOARD_SPACE                " )          : "
#define BOARD_TITLE                " )  BOARD:  : "
#define DEFAULT_ALLOW_SQUARING     TRUE
#define DEFAULT_ALLOW_STACKING     TRUE
#define DEFAULT_BLANK_PIECE_LABEL  '-'
#define DEFAULT_BOARD_DIMENSION    3
#define DEFAULT_DARK_PIECE_LABEL   'O'
#define DEFAULT_LIGHT_PIECE_LABEL  'X'
#define LEGEND_SPACE               "         ( "
#define LEGEND_TITLE               "LEGEND:  ( "
#define MOVE_PROMPT                "%s's move : "
#define MOVES_IN_GROUP             sizeof(int)
#define NUMBER_OF_OPTIONS          CalculatePower(2, 3)
#define OPPONENT_PIECES_LEFT_TITLE "OPPONENT PIECES LEFT: "
#define PIECES_LEFT_TITLE          "         PIECES LEFT: "

#ifndef WORD_BIT
#define WORD_BIT          CHAR_BIT * sizeof(int)
#endif

/********************************************* Global variables for Gamesman */
POSITION gInitialPosition       = 0;
POSITION gNumberOfPositions     = 0;
STRING kAuthorName            = "Eric Siroker";
POSITION kBadPosition           = -1;
STRING kDBName                = "pylos";
BOOLEAN kDebugDetermineValue   = FALSE;
BOOLEAN kDebugMenu             = FALSE;
STRING kGameName              = "Pylos";
BOOLEAN kGameSpecificMenu      = TRUE;

/* Out of date. */
STRING kHelpExample           = "\
         (  1   2   3 )          : - - -\n\
         (    4   5   )          :  - -\n\
LEGEND:  (  6   7   8 )  BOARD:  : - - -\n\
         (    9  10   )          :  - -\n\
         ( 11  12  13 )          : - - - (Player should Win in 17)\n\n\
PIECES LEFT: XXXXXXX (7)\n\n\
                   Player's move : { 7 }\n\n\
         (  1   2   3 )          : - - -\n\
         (    4   5   )          :  - -\n\
LEGEND:  (  6   7   8 )  BOARD:  : - X -\n\
         (    9  10   )          :  - -\n\
         ( 11  12  13 )          : - - - (Computer should Lose in 16)\n\n\
PIECES LEFT: OOOOOOO (7)\n\n\
                 Computer's move : 8\n\n\
         (  1   2   3 )          : - - -\n\
         (    4   5   )          :  - -\n\
LEGEND:  (  6   7   8 )  BOARD:  : - X O\n\
         (    9  10   )          :  - -\n\
         ( 11  12  13 )          : - - - (Player should Win in 15)\n\n\
PIECES LEFT: XXXXXX (6)\n\n\
                   Player's move : { 6 }\n\n\
         (  1   2   3 )          : - - -\n\
         (    4   5   )          :  - -\n\
LEGEND:  (  6   7   8 )  BOARD:  : X X O\n\
         (    9  10   )          :  - -\n\
         ( 11  12  13 )          : - - - (Computer should Lose in 14)\n\n\
PIECES LEFT: OOOOOO (6)\n\n\
                 Computer's move : 13\n\n\
         (  1   2   3 )          : - - -\n\
         (    4   5   )          :  - -\n\
LEGEND:  (  6   7   8 )  BOARD:  : X X O\n\
         (    9  10   )          :  - -\n\
         ( 11  12  13 )          : - - O (Player should Win in 13)\n\n\
PIECES LEFT: XXXXX (5)\n\n\
                   Player's move : { 11 }\n\n\
         (  1   2   3 )          : - - -\n\
         (    4   5   )          :  - -\n\
LEGEND:  (  6   7   8 )  BOARD:  : X X O\n\
         (    9  10   )          :  - -\n\
         ( 11  12  13 )          : X - O (Computer should Lose in 12)\n\n\
PIECES LEFT: OOOOO (5)\n\n\
                 Computer's move : 1\n\n\
         (  1   2   3 )          : O - -\n\
         (    4   5   )          :  - -\n\
LEGEND:  (  6   7   8 )  BOARD:  : X X O\n\
         (    9  10   )          :  - -\n\
         ( 11  12  13 )          : X - O (Player should Win in 11)\n\n\
PIECES LEFT: XXXX (4)\n\n\
                   Player's move : { 12 12 }\n\n\
         (  1   2   3 )          : O - -\n\
         (    4   5   )          :  - -\n\
LEGEND:  (  6   7   8 )  BOARD:  : X X O\n\
         (    9  10   )          :  - -\n\
         ( 11  12  13 )          : X - O (Computer should Lose in 10)\n\n\
PIECES LEFT: OOOO (4)\n\n\
                 Computer's move : 2\n\n\
         (  1   2   3 )          : O O -\n\
         (    4   5   )          :  - -\n\
LEGEND:  (  6   7   8 )  BOARD:  : X X O\n\
         (    9  10   )          :  - -\n\
         ( 11  12  13 )          : X - O (Player should Win in 9)\n\n\
PIECES LEFT: XXXX (4)\n\n\
                   Player's move : { 12 12 }\n\n\
         (  1   2   3 )          : O O -\n\
         (    4   5   )          :  - -\n\
LEGEND:  (  6   7   8 )  BOARD:  : X X O\n\
         (    9  10   )          :  - -\n\
         ( 11  12  13 )          : X - O (Computer should Lose in 10)\n\n\
PIECES LEFT: OOO (3)\n\n\
                 Computer's move : 4 8\n\n\
         (  1   2   3 )          : O O -\n\
         (    4   5   )          :  O -\n\
LEGEND:  (  6   7   8 )  BOARD:  : X X -\n\
         (    9  10   )          :  - -\n\
         ( 11  12  13 )          : X - O (Player should Win in 9)\n\n\
PIECES LEFT: XXXX (4)\n\n\
                   Player's move : { 12 12 }\n\n\
         (  1   2   3 )          : O O -\n\
         (    4   5   )          :  O -\n\
LEGEND:  (  6   7   8 )  BOARD:  : X X -\n\
         (    9  10   )          :  - -\n\
         ( 11  12  13 )          : X - O (Computer should Lose in 8)\n\n\
PIECES LEFT: OOO (3)\n\n\
                 Computer's move : 12\n\n\
         (  1   2   3 )          : O O -\n\
         (    4   5   )          :  O -\n\
LEGEND:  (  6   7   8 )  BOARD:  : X X -\n\
         (    9  10   )          :  - -\n\
         ( 11  12  13 )          : X O O (Player should Win in 7)\n\n\
PIECES LEFT: XXXX (4)\n\n\
                   Player's move : { 3 }\n\n\
         (  1   2   3 )          : O O X\n\
         (    4   5   )          :  O -\n\
LEGEND:  (  6   7   8 )  BOARD:  : X X -\n\
         (    9  10   )          :  - -\n\
         ( 11  12  13 )          : X O O (Computer should Lose in 6)\n\n\
PIECES LEFT: OO (2)\n\n\
                 Computer's move : 9 13\n\n\
         (  1   2   3 )          : O O X\n\
         (    4   5   )          :  O -\n\
LEGEND:  (  6   7   8 )  BOARD:  : X X -\n\
         (    9  10   )          :  O -\n\
         ( 11  12  13 )          : X O - (Player should Win in 5)\n\n\
PIECES LEFT: XXX (3)\n\n\
                   Player's move : { 13 }\n\n\
         (  1   2   3 )          : O O X\n\
         (    4   5   )          :  O -\n\
LEGEND:  (  6   7   8 )  BOARD:  : X X -\n\
         (    9  10   )          :  O -\n\
         ( 11  12  13 )          : X O X (Computer should Lose in 4)\n\n\
PIECES LEFT: OO (2)\n\n\
                 Computer's move : 8\n\n\
         (  1   2   3 )          : O O X\n\
         (    4   5   )          :  O -\n\
LEGEND:  (  6   7   8 )  BOARD:  : X X O\n\
         (    9  10   )          :  O -\n\
         ( 11  12  13 )          : X O X (Player should Win in 3)\n\n\
PIECES LEFT: XX (2)\n\n\
                   Player's move : { 5 13 }\n\n\
         (  1   2   3 )          : O O X\n\
         (    4   5   )          :  O X\n\
LEGEND:  (  6   7   8 )  BOARD:  : X X O\n\
         (    9  10   )          :  O -\n\
         ( 11  12  13 )          : X O - (Computer should Lose in 2)\n\n\
PIECES LEFT: O (1)\n\n\
                 Computer's move : 13\n\n\
         (  1   2   3 )          : O O X\n\
         (    4   5   )          :  O X\n\
LEGEND:  (  6   7   8 )  BOARD:  : X X O\n\
         (    9  10   )          :  O -\n\
         ( 11  12  13 )          : X O O (Player should Win in 1)\n\n\
PIECES LEFT: XX (2)\n\n\
                   Player's move : { 10 }\n\n\
         (  1   2   3 )          : O O X\n\
         (    4   5   )          :  O X\n\
LEGEND:  (  6   7   8 )  BOARD:  : X - O\n\
         (    9  10   )          :  O X\n\
         ( 11  12  13 )          : X O O (Computer should Lose in 0)\n\n\
PIECES LEFT: (0)\n\n\n\
Excellent! You won!";
STRING kHelpGraphicInterface  = "";
STRING kHelpOnYourTurn        = "\
Place a piece on a blank spot. If a two-by-two square is formed, you must\n\
take back one of your pieces. You may not take back a piece that is being\n\
used to support another piece. If a two-by-two square already exists and you\n\
have any pieces of equal or lower height to the square, you must take one of\n\
those pieces and stack it on top of the square if you intend to put any piece\n\
on top of the square.";
STRING kHelpReverseObjective  = "\
To either use all of your pieces before your opponent does or to complete the\n\
pyramid.";
STRING kHelpStandardObjective = "\
To make your opponent either use all of his or her pieces before you do or\n\
make your opponent complete the pyramid.";
STRING kHelpTextInterface     = "\
The board represents a bird's eye view of a pyramid. Pieces are placed\n\
building this pyramid. A piece can only be placed on a higher level of the\n\
pyramid if all four of its bases have a piece on them. The legend indicates\n\
which vertical column action will be taken on. Typically the action will be\n\
adding a piece, but, if a two-by-two square is formed, a move can consist of\n\
two actions: an addition and a removal. An addition and removal can also\n\
occur when a two-by-two square already exists and a piece already on the\n\
board is moved on top of the square.";
STRING kHelpTieOccursWhen     = "";
BOOLEAN kLoopy                 = TRUE;
BOOLEAN kPartizan              = TRUE;
BOOLEAN kTieIsPossible         = FALSE;

/************************************************ Global variables for Pylos */
typedef enum Direction Direction;
typedef struct MoveGroup MoveGroup;
typedef struct Options Options;
typedef enum Piece Piece;
typedef struct Pyramid Pyramid;

enum Direction {
	INVALID_DIRECTION = -1,
	LOWER_LEFT_DIRECTION,
	LOWER_RIGHT_DIRECTION,
	UPPER_LEFT_DIRECTION,
	UPPER_RIGHT_DIRECTION,
	NUMBER_OF_DIRECTIONS
};

struct MoveGroup {
	char moves[MOVES_IN_GROUP];
};

struct Options {
	BOOLEAN doNotAllowSquaring : 1;
	BOOLEAN doNotAllowStacking : 1;
	BOOLEAN isNotStandardGame : 1;
	int boardDimension : WORD_BIT - 3;
};

enum Piece {
	BLANK_PIECE,
	LIGHT_PIECE,
	DARK_PIECE
};

struct Pyramid {
	char pieceLabel;
	Pyramid *bases[NUMBER_OF_DIRECTIONS], *parents[NUMBER_OF_DIRECTIONS];
};

BOOLEAN gAllowSquaring = DEFAULT_ALLOW_SQUARING;
BOOLEAN gAllowStacking = DEFAULT_ALLOW_STACKING;
int gBoardAddresses;
int gBoardDimension = DEFAULT_BOARD_DIMENSION;
int gBoardPieces;
int gBoardSize;

char gPieceLabels[] = {
	DEFAULT_BLANK_PIECE_LABEL,
	DEFAULT_LIGHT_PIECE_LABEL,
	DEFAULT_DARK_PIECE_LABEL
};

/********************************************* Function prototypes for Pylos */
int CalculateDigits(int integer);
int CalculatePower(int base, int exponent);
int CalculatePyramidHeight(Pyramid *pyramid);
int ConvertAddressToIndex(int address);
int ConvertPyramidIndex(int index, Direction direction);
int CountPieces(char board[], Piece piece);
Pyramid *CreatePyramid(Pyramid pyramids[], int dimension, int baseIndex,
                       Direction parentDirection, int parentIndex);
void CreatePyramidHashTable(Pyramid pyramids[], char board[]);
Direction FlipDirection(Direction direction);
BOOLEAN FormsSquare(Pyramid *pyramid, Piece piece);
BOOLEAN IsBasePyramid(Pyramid *pyramid, Pyramid *parentPyramid);
BOOLEAN IsBlankPyramid(Pyramid *pyramid);
BOOLEAN IsSupportedPyramid(Pyramid *pyramid);
BOOLEAN IsSupportingPyramid(Pyramid *pyramid);
void MakeAddressable(char board[]);
void PrintBinary(int integer);
void PrintMoveGroup(MOVE move);
void PrintMovePrompt(char *name);

STRING MoveGroupToString(MOVE move);
STRING MoveToString(MOVE);

/**************************************************** ConvertTextInputToMove */
MOVE ConvertTextInputToMove(STRING input) {
	int index;
	char *move;
	MoveGroup moveGroup;

	move = strtok(input, " ");

	for (index = 0; index < MOVES_IN_GROUP; index++) {
		moveGroup.moves[index] = move == NULL ? 0 : atoi(move);
		move = strtok(NULL, " ");
	}

	return *(MOVE*)&moveGroup;
}

/***************************************************************** DebugMenu */
void DebugMenu() {
}

/******************************************************************** DoMove */
POSITION DoMove(POSITION position, MOVE move) {
	char board[gBoardSize];
	MoveGroup moveGroup = *(MoveGroup*)&move;
	int index = ConvertAddressToIndex(moveGroup.moves[0]);
	Piece piece = generic_hash_turn(position) == LIGHT_PIECE ? DARK_PIECE : LIGHT_PIECE;

	generic_hash_unhash(position, board);

	/* Hardcoded */
	if (gBoardDimension == 3 && index == 4 &&
	    board[9] != gPieceLabels[BLANK_PIECE] &&
	    board[10] != gPieceLabels[BLANK_PIECE] &&
	    board[11] != gPieceLabels[BLANK_PIECE] &&
	    board[12] != gPieceLabels[BLANK_PIECE])
		index = 13;

	board[index] = gPieceLabels[generic_hash_turn(position)];

	for (index = 1; index < MOVES_IN_GROUP; index++) {
		move = moveGroup.moves[index];

		if (move > 0)
			board[ConvertAddressToIndex(move)] = gPieceLabels[BLANK_PIECE];
	}

	return generic_hash_hash(board, piece);
}

/********************************************************** GameSpecificMenu */
void GameSpecificMenu() {
	int boardDimension;

	while (TRUE) {
		printf("\n\t----- Game-specific options for %s -----\n", kGameName);
		printf("\n\tBoard Options:\n\n");
		printf("\td)\tChange board (D)imension (%d)\n", gBoardDimension);
		printf("\te)\tChange (E)mpty piece (%c)\n", gPieceLabels[BLANK_PIECE]);
		printf("\t1)\tChange player (1)'s piece (%c)\n", gPieceLabels[LIGHT_PIECE]);
		printf("\t2)\tChange player (2)'s piece (%c)\n", gPieceLabels[DARK_PIECE]);
		printf("\n\tRule Options:\n\n");
		printf("\ts)\t%sable (S)quaring (currently %sabled)\n",
		       gAllowSquaring ? "Dis" : "En", gAllowSquaring ? "en" : "dis");
		printf("\tt)\t%sable s(T)acking (currently %sabled)\n",
		       gAllowStacking ? "Dis" : "En", gAllowStacking ? "en" : "dis");
		printf("\n\n\tb)\t(B)ack = Return to previous activity.\n");
		printf("\n\nSelect an option: ");

		switch(GetMyChar()) {
		case 'Q': case 'q':
			ExitStageRight();
		case 'H': case 'h':
			HelpMenus();
			break;
		case 'D': case 'd':
			printf("\nBoard dimension (%d): ", gBoardDimension);
			scanf("%d", &boardDimension);

			if (boardDimension > 0)
				gBoardDimension = boardDimension;

			break;
		case 'E': case 'e':
			printf("\nEmpty piece (%c): ", gPieceLabels[BLANK_PIECE]);
			gPieceLabels[BLANK_PIECE] = GetMyChar();
			break;
		case '1':
			printf("\nPlayer 1's piece (%c): ", gPieceLabels[LIGHT_PIECE]);
			gPieceLabels[LIGHT_PIECE] = GetMyChar();
			break;
		case '2':
			printf("\nPlayer 2's piece (%c): ", gPieceLabels[DARK_PIECE]);
			gPieceLabels[DARK_PIECE] = GetMyChar();
			break;
		case 'S': case 's':
			gAllowSquaring = !gAllowSquaring;
			break;
		case 'T': case 't':
			gAllowStacking = !gAllowStacking;
			break;
		case 'B': case 'b':
			return;
		default:
			BadMenuChoice();
			HitAnyKeyToContinue();
		}
	}
}

/************************************************************* GenerateMoves */
MOVELIST *GenerateMoves(POSITION position) {
	char board[gBoardSize];
	char center; /* Used for hardcoding */
	int height, index;
	MOVELIST* moveList = NULL;
	MoveGroup moveGroup;
	Pyramid *pyramid, pyramids[gBoardAddresses];
	BOOLEAN stacked = FALSE;

	generic_hash_unhash(position, board);

	if (gBoardDimension == 3)
		center = board[4]; /* Used for hardcoding */

	MakeAddressable(board);
	CreatePyramidHashTable(pyramids, board);

	for (index = 0; index < MOVES_IN_GROUP; index++)
		moveGroup.moves[index] = 0;

	for (moveGroup.moves[0] = gBoardAddresses; moveGroup.moves[0] > 0;
	     moveGroup.moves[0]--) {
		pyramid = &pyramids[moveGroup.moves[0] - 1];

		if ((IsBlankPyramid(pyramid) && IsSupportedPyramid(pyramid)) ||
		    (gBoardDimension == 3 && moveGroup.moves[0] == 7 && /* Hardcoded */
		     board[4] == gPieceLabels[BLANK_PIECE])) {
			if (gAllowSquaring && !(gBoardDimension == 3 && /* Hardcoded */
			                        moveGroup.moves[0] == 7 && center != gPieceLabels[BLANK_PIECE]) &&
			    FormsSquare(pyramid, generic_hash_turn(position))) {
				for (moveGroup.moves[1] = gBoardAddresses; moveGroup.moves[1] > 0;
				     moveGroup.moves[1]--) {
					pyramid = &pyramids[moveGroup.moves[1] - 1];

					if (moveGroup.moves[1] == moveGroup.moves[0] ||
					    (pyramid->pieceLabel == gPieceLabels[generic_hash_turn(position)] &&
					     !IsSupportingPyramid(pyramid)))
						moveList = CreateMovelistNode(*(MOVE*)&moveGroup, moveList);
				}
			}
			else if (gAllowStacking && !(gBoardDimension == 3 && /* Hardcoded */
			                             moveGroup.moves[0] == 7 &&
			                             center == gPieceLabels[BLANK_PIECE]) &&
			         (height = CalculatePyramidHeight(pyramid)) > 0) {
				for (moveGroup.moves[1] = gBoardAddresses; moveGroup.moves[1] > 0;
				     moveGroup.moves[1]--) {
					pyramid = &pyramids[moveGroup.moves[1] - 1];

					if (pyramid->pieceLabel == gPieceLabels[generic_hash_turn(position)] &&
					    CalculatePyramidHeight(pyramid) < height &&
					    !IsBasePyramid(pyramid, &pyramids[moveGroup.moves[0] - 1]) &&
					    !IsSupportingPyramid(pyramid)) {
						moveList = CreateMovelistNode(*(MOVE*)&moveGroup, moveList);
						stacked = TRUE;
					}
				}

				if (!stacked)
					moveList = CreateMovelistNode(*(MOVE*)&moveGroup, moveList);
			}
			else
				moveList = CreateMovelistNode(*(MOVE*)&moveGroup, moveList);
		}
	}

	return moveList;
}

/**************************************************** GetAndPrintPlayersMove */
USERINPUT GetAndPrintPlayersMove(POSITION position, MOVE *move, STRING name) {
	USERINPUT userInput = Continue;

	while (userInput == Continue) {
		PrintMovePrompt(name);
		userInput = HandleDefaultTextInput(position, move, name);
	}

	return userInput;
}

/******************************************************** GetInitialPosition */
POSITION GetInitialPosition() {
	return gInitialPosition;
}

/***************************************************************** getOption */
int getOption() {
	Options options;

	options.doNotAllowSquaring = !gAllowSquaring;
	options.doNotAllowStacking = !gAllowStacking;
	options.isNotStandardGame = !gStandardGame;
	options.boardDimension = DEFAULT_BOARD_DIMENSION - gBoardDimension;

	return *(int*)&options + 1;
}

/************************************************************ InitializeGame */
void InitializeGame() {
	int dimension;

	gBoardAddresses = gBoardDimension * gBoardDimension + (gBoardDimension - 1) *
	                  (gBoardDimension - 1);

	for (dimension = gBoardDimension, gBoardSize = 0; dimension > 0; dimension--)
		gBoardSize += dimension * dimension;

	gBoardPieces = gBoardSize / 2 + (gBoardSize % 2 == 1);

	int piecesArray[] = {gPieceLabels[BLANK_PIECE], 0, gBoardSize,
		             gPieceLabels[LIGHT_PIECE], 0, gBoardPieces,
		             gPieceLabels[DARK_PIECE], 0, gBoardPieces, -1};


	gNumberOfPositions = generic_hash_init(gBoardSize, piecesArray, NULL, 0);

	gMoveToStringFunPtr = &MoveToString;
}

/*********************************************************** NumberOfOptions */
int NumberOfOptions() {
	return NUMBER_OF_OPTIONS;
}

/***************************************************************** Primitive */
VALUE Primitive(POSITION position) {
	char board[gBoardSize];
	Piece piece = generic_hash_turn(position) == LIGHT_PIECE ? DARK_PIECE : LIGHT_PIECE;

	generic_hash_unhash(position, board);

	if (board[gBoardSize - 1] == gPieceLabels[piece] ||
	    CountPieces(board, generic_hash_turn(position)) == gBoardPieces)
		return gStandardGame ? lose : win;
	else
		return undecided;
}

/******************************************************** PrintComputersMove */
void PrintComputersMove(MOVE move, STRING name) {
	PrintMovePrompt(name);
	PrintMoveGroup(move);
	putchar('\n');
}

/***************************************************************** PrintMove */
void PrintMove(MOVE move) {
	STRING s = MoveToString( move );
	printf( "%s", s );
	SafeFree(s);
}

/***************************************************************** MoveToString */
STRING MoveToString( MOVE move ) {
	STRING s = (STRING) SafeMalloc( 10 );

	STRING temp = MoveGroupToString(move);

	if( move > UCHAR_MAX )
		sprintf( s, "[%s]", temp );
	else
		sprintf( s, "%s", temp );

	SafeFree( temp );
	return s;
}

/************************************************************* PrintPosition */
void PrintPosition(POSITION position, STRING name, BOOLEAN isUsersTurn) {
	int address, column, count, digits = CalculateDigits(gBoardSize), offset;
	int opponentPieces, pieces, row, rows = gBoardDimension * 2 - 1;
	int columns = strlen(LEGEND_SPACE) + rows * digits +
	              strlen(BOARD_SPACE) + rows + 1; /* + 1 for '\n' */
	char board[gBoardSize], buffer[rows][columns], number[digits];
	char format[CalculateDigits(digits) + 2], *string; /* + 2 for %d */
	Piece opponent = generic_hash_turn(position) == LIGHT_PIECE ? DARK_PIECE :
	                 LIGHT_PIECE;
	char opponentPieceLabel = gPieceLabels[opponent];
	char pieceLabel = gPieceLabels[generic_hash_turn(position)];

	generic_hash_unhash(position, board);
	opponentPieces = gBoardPieces - CountPieces(board, opponent);
	pieces = gBoardPieces - CountPieces(board, generic_hash_turn(position));
	MakeAddressable(board);

	for (address = 1, count = 1, row = 0; row < rows; row++) {
		for (column = 0; column < columns; column++)
			buffer[row][column] = ' ';

		string = row != rows / 2 ? LEGEND_SPACE : LEGEND_TITLE;
		strncpy(buffer[row], string, strlen(string));
		offset = strlen(LEGEND_SPACE);
		sprintf(format, "%%%dd", digits);

		for (column = 0; column < rows; column++) {
			if (column % 2 == row % 2) {
				sprintf(number, format, count++);
				strncpy(buffer[row] + offset + column * digits, number, strlen(number));
			}
		}

		offset += rows * digits;
		string = row != rows / 2 ? BOARD_SPACE : BOARD_TITLE;
		strncpy(buffer[row] + offset, string, strlen(string));
		offset += strlen(BOARD_SPACE);

		for (column = 0; column < rows; column++)
			if (column % 2 == row % 2)
				buffer[row][offset + column] = board[ConvertAddressToIndex(address++)];

		buffer[row][columns - 1] = '\n';
	}

	buffer[rows - 1][columns - 1] = '\0';

	printf("\n%s %s\n\n%s", (char *)buffer, GetPrediction(position, name, isUsersTurn), PIECES_LEFT_TITLE);

	if (pieces == 0)
		printf("None");
	else {
		for (count = 0; count < pieces; count++)
			putchar(pieceLabel);

		printf(" (%d)", pieces);
	}

	printf("\n%s", OPPONENT_PIECES_LEFT_TITLE);

	if (opponentPieces == 0)
		printf("None");
	else {
		for (count = 0; count < opponentPieces; count++)
			putchar(opponentPieceLabel);

		printf(" (%d)", opponentPieces);
	}

	printf("\n\n");
}

/***************************************************************** setOption */
void setOption(int option) {
	option--;

	Options options = *(Options*)&option;

	gAllowSquaring = !options.doNotAllowSquaring;
	gAllowStacking = !options.doNotAllowStacking;
	gStandardGame = !options.isNotStandardGame;
	gBoardDimension = DEFAULT_BOARD_DIMENSION - options.boardDimension;
}

/************************************************************ ValidTextInput */
BOOLEAN ValidTextInput(STRING input) {
	int index;
	char *move, inputBackup[strlen(input)];

	strcpy(inputBackup, input);
	move = strtok(inputBackup, " ");

	for (index = 0; index < MOVES_IN_GROUP && move != NULL; index++) {
		if (atoi(move) < 1)
			return FALSE;

		move = strtok(NULL, " ");
	}

	return TRUE;
}

/*********************************************************** CalculateDigits */
int CalculateDigits(int integer) {
	int digits;

	for (digits = 1; (integer /= 10) > 0; digits++) ;

	return digits;
}

/************************************************************ CalculatePower */
int CalculatePower(int base, int exponent) {
	int power;

	for (power = 1; exponent > 0; exponent--)
		power *= base;

	return power;
}

/**************************************************** CalculatePyramidHeight */
int CalculatePyramidHeight(Pyramid *pyramid) {
	Pyramid *basePyramid = pyramid->bases[UPPER_LEFT_DIRECTION];

	return basePyramid == NULL ? 0 : CalculatePyramidHeight(basePyramid) + 1;
}

/***************************************************** ConvertAddressToIndex */
int ConvertAddressToIndex(int address) {
	int column, dimension, row;

	address--;
	dimension = gBoardDimension * 2 - 1;
	column = address % dimension;
	row = address / dimension;

	return column < gBoardDimension ? row * gBoardDimension + column :
	       gBoardDimension * gBoardDimension + row * (gBoardDimension - 1) +
	       column - gBoardDimension;
}

/******************************************************* ConvertPyramidIndex */
int ConvertPyramidIndex(int index, Direction direction) {
	switch (direction) {
	case LOWER_LEFT_DIRECTION:
		return index + gBoardDimension - 1;
	case LOWER_RIGHT_DIRECTION:
		return index + gBoardDimension;
	case UPPER_LEFT_DIRECTION:
		return index - gBoardDimension;
	case UPPER_RIGHT_DIRECTION:
		return index - gBoardDimension + 1;
	default:
		return index;
	}
}

/*************************************************************** CountPieces */
int CountPieces(char board[], Piece piece) {
	char pieceLabel = gPieceLabels[piece];
	int index, pieces;

	for (index = 0, pieces = 0; index < gBoardSize; index++)
		if (board[index] == pieceLabel)
			pieces++;

	return pieces;
}

/************************************************************* CreatePyramid */
Pyramid *CreatePyramid(Pyramid pyramids[], int dimension, int baseIndex,
                       Direction parentDirection, int parentIndex) {
	Direction direction;
	Pyramid *pyramid = NULL;

	if (dimension > 0) {
		pyramid = &pyramids[baseIndex];

		for (direction = 0; direction < NUMBER_OF_DIRECTIONS; direction++)
			if (pyramid->bases[direction] == NULL)
				pyramid->bases[direction] =
				        CreatePyramid(pyramids, dimension - 1,
				                      ConvertPyramidIndex(baseIndex, direction),
				                      FlipDirection(direction), baseIndex);

		if (parentDirection != INVALID_DIRECTION)
			pyramid->parents[parentDirection] = &pyramids[parentIndex];
	}

	return pyramid;
}

/**************************************************** CreatePyramidHashTable */
void CreatePyramidHashTable(Pyramid pyramids[], char board[]) {
	Direction direction;
	int index;

	for (index = 0; index < gBoardAddresses; index++) {
		for (direction = 0; direction < NUMBER_OF_DIRECTIONS; direction++) {
			pyramids[index].bases[direction] = NULL;
			pyramids[index].parents[direction] = NULL;
		}
	}

	CreatePyramid(pyramids, gBoardDimension, gBoardAddresses / 2,
	              INVALID_DIRECTION, gBoardAddresses / 2);

	for (index = 0; index < gBoardAddresses; index++)
		pyramids[index].pieceLabel = board[ConvertAddressToIndex(index + 1)];
}

/************************************************************* FlipDirection */
Direction FlipDirection(Direction direction) {
	switch (direction) {
	case LOWER_LEFT_DIRECTION:
		return UPPER_RIGHT_DIRECTION;
	case LOWER_RIGHT_DIRECTION:
		return UPPER_LEFT_DIRECTION;
	case UPPER_LEFT_DIRECTION:
		return LOWER_RIGHT_DIRECTION;
	case UPPER_RIGHT_DIRECTION:
		return LOWER_LEFT_DIRECTION;
	default:
		return direction;
	}
}

/*************************************************************** FormsSquare */
BOOLEAN FormsSquare(Pyramid *pyramid, Piece piece) {
	Direction baseDirection, parentDirection;
	Pyramid *basePyramid, *parentPyramid;

	for (parentDirection = 0; parentDirection < NUMBER_OF_DIRECTIONS;
	     parentDirection++) {
		parentPyramid = pyramid->parents[parentDirection];

		if (parentPyramid != NULL) {
			for (baseDirection = 0; baseDirection < NUMBER_OF_DIRECTIONS;
			     baseDirection++) {
				basePyramid = parentPyramid->bases[baseDirection];

				if (basePyramid != pyramid &&
				    basePyramid->pieceLabel != gPieceLabels[piece])
					break;

				if (baseDirection == NUMBER_OF_DIRECTIONS - 1)
					return TRUE;
			}
		}
	}

	return FALSE;
}

/************************************************************* IsBasePyramid */
BOOLEAN IsBasePyramid(Pyramid *pyramid, Pyramid *parentPyramid) {
	Direction direction;

	for (direction = 0; direction < NUMBER_OF_DIRECTIONS; direction++)
		if (pyramid == parentPyramid->bases[direction])
			return TRUE;

	return FALSE;
}

/************************************************************ IsBlankPyramid */
BOOLEAN IsBlankPyramid(Pyramid *pyramid) {
	return pyramid != NULL && pyramid->pieceLabel == gPieceLabels[BLANK_PIECE];
}

/******************************************************** IsSupportedPyramid */
BOOLEAN IsSupportedPyramid(Pyramid *pyramid) {
	Direction direction;

	for (direction = 0; direction < NUMBER_OF_DIRECTIONS; direction++)
		if (IsBlankPyramid(pyramid->bases[direction]))
			return FALSE;

	return TRUE;
}

/******************************************************** IsSupportingPyramid */
BOOLEAN IsSupportingPyramid(Pyramid *pyramid) {
	Direction direction;

	for (direction = 0; direction < NUMBER_OF_DIRECTIONS; direction++)
		if (pyramid->parents[direction] != NULL &&
		    pyramid->parents[direction]->pieceLabel != gPieceLabels[BLANK_PIECE])
			return TRUE;

	return FALSE;
}

/*********************************************************** MakeAddressable */
void MakeAddressable(char board[]) {
	/* Hardcoded */
	if (gBoardDimension == 3 &&
	    board[9] != gPieceLabels[BLANK_PIECE] &&
	    board[10] != gPieceLabels[BLANK_PIECE] &&
	    board[11] != gPieceLabels[BLANK_PIECE] &&
	    board[12] != gPieceLabels[BLANK_PIECE])
		board[4] = board[13];
}

/*************************************************************** PrintBinary */
void PrintBinary(int integer) {
	int power;

	for (power = WORD_BIT - 1; power >= 0; power--) {
		putchar(integer & CalculatePower(2, power) ? '1' : '0');

		if (power % CHAR_BIT == 0)
			putchar(' ');
	}

	putchar('\n');
}

/************************************************************ PrintMoveGroup */
void PrintMoveGroup(MOVE move) {
	STRING s = MoveGroupToString( move );
	printf( "%s", s );
	SafeFree( s );
}

STRING MoveGroupToString(MOVE move) {
	STRING s = (STRING) SafeMalloc( 10 );
	int index;
	MoveGroup moveGroup = *(MoveGroup*)&move;

	for (index = 0; index < MOVES_IN_GROUP; index++) {
		if (moveGroup.moves[index] != 0) {
			if (index > 0)
				sprintf( s, " " );

			sprintf(s, "%d", moveGroup.moves[index]);
		}
	}

	return s;
}

/*********************************************************** PrintMovePrompt */
void PrintMovePrompt(char *name) {
	int promptLength = strlen(LEGEND_SPACE) + (gBoardDimension * 2 - 1) *
	                   CalculateDigits(gBoardSize) + strlen(BOARD_SPACE);
	char format[CalculateDigits(promptLength) + 2]; /* + 2 for %d */
	char prompt[promptLength];

	sprintf(format, "%%%ds", promptLength);
	sprintf(prompt, MOVE_PROMPT, name);

	printf(format, prompt);
}

void *gGameSpecificTclInit = NULL;
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
