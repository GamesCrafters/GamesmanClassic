/************************************************************************
**
** NAME:        mslide5.c
**
** DESCRIPTION: Slide-5
**
** AUTHOR:      Harnoor Dhillon
**
** DATE:        2023-04-07
**
************************************************************************/

#include "gamesman.h"

/* IMPORTANT GLOBAL VARIABLES */
CONST_STRING kAuthorName = "Harnoor Dhillon";
CONST_STRING kGameName = "Slide-5"; 
CONST_STRING kDBName = "slide5"; 
POSITION gNumberOfPositions = 77834825526; 
POSITION gInitialPosition = 0; // TODO: Put the hash value of the initial position.
BOOLEAN kPartizan = TRUE; // TODO: Is the game PARTIZAN i.e. given a board does each player have a different set of moves available to them?
BOOLEAN kTieIsPossible = TRUE; // TODO: Is a tie or draw possible?
BOOLEAN kLoopy = TRUE; // TODO: Is this game loopy?
BOOLEAN kSupportsSymmetries = TRUE; // TODO: Whether symmetries are supported (i.e. whether the GetCanonicalPosition is implemented)

/* Likely you do not have to change these. */
POSITION GetCanonicalPosition(POSITION);
STRING MoveToString(MOVE);
POSITION kBadPosition = -1;
BOOLEAN kDebugDetermineValue = FALSE;
void* gGameSpecificTclInit = NULL;

/* You do not have to change these for now. */
BOOLEAN kGameSpecificMenu = TRUE;
BOOLEAN kDebugMenu = FALSE;
TIERLIST *getTierChildren(TIER tier);
TIERPOSITION numberOfTierPositions(TIER tier);

/* These variables are not needed for solving but if you have time 
after you're done solving the game you should initialize them 
with something helpful. */
CONST_STRING kHelpGraphicInterface = "";
CONST_STRING kHelpTextInterface = "";
CONST_STRING kHelpOnYourTurn = "";
CONST_STRING kHelpStandardObjective = "";
CONST_STRING kHelpReverseObjective = "";
CONST_STRING kHelpTieOccursWhen = /* Should follow 'A Tie occurs when... */ "";
CONST_STRING kHelpExample = "";

/* You don't have to change this. */
void DebugMenu() {}
/* Ignore this function. */
void SetTclCGameSpecificOptions(int theOptions[]) {
  (void)theOptions;
}

// Defining Slide5Board
typedef struct {
    int board[26];
} Slide5Board;


VALUE tieResult = lose;

int move_indexes[][5] = {
        {0, 1, 2, 3, 4},
        {5, 6, 7, 8, 9},
        {10, 11, 12, 13, 14},
        {15, 16, 17, 18, 19},
        {20, 21, 22, 23, 24},
        {20, 15, 10, 5, 0},
        {21, 16, 11, 6, 1},
        {22, 17, 12, 7, 2},
        {23, 18, 13, 8, 3},
        {24, 19, 14, 9, 4},
    };

int indexes[][5] = {
        {1, 2, 3, 4, 5},
        {6, 7, 8, 9, 10},
        {11, 12, 13, 14, 15},
        {16, 17, 18, 19, 20},
        {21, 22, 23, 24, 25},
        {21, 16, 11, 6, 1},
        {22, 17, 12, 7, 2},
        {23, 18, 13, 8, 3},
        {24, 19, 14, 9, 4},
        {25, 20, 15, 10, 5},
        {21, 17, 13, 9, 5},
        {1, 7, 13, 19, 25}
    };



/*********** BEGIN SOLVING FUNCIONS ***********/

/* TODO: Add a hashing function and unhashing function, if needed. */
uint64_t Hash(Slide5Board *board) {

  TIER tier = 0;
  TIERPOSITION tier_position = 0;
  // Calculate the tier
  for (int i = 25; i > 0; i--) {
    tier = (tier << 1) | (board->board[i] > 0 ? 1 : 0);
  }

  // Calculate the tier position 
  uint64_t bit_position = 1;
  for (int i = 1; i < 26; i++) {
    if (board->board[i] > 0) {
      tier_position |= (board->board[i] - 1) * bit_position;
      bit_position <<= 1;
    }
  }
  tier_position = (tier_position << 1) | board->board[0];

  POSITION pos = 0;
  if (gHashWindowInitialized) {
    pos = gHashToWindowPosition(tier_position, tier);
  }

  return pos;
}

Slide5Board* Unhash(uint64_t hash_value) {
  Slide5Board *board = (Slide5Board *) malloc(sizeof(Slide5Board));

  // Gets tier and tier_position from hash
  TIER tier;
  TIERPOSITION tier_position;

  gUnhashToTierPosition(hash_value, &tier_position, &tier);

  board->board[0] = tier_position & 1;
  tier_position >>= 1;

  for (int i = 1; i < 26; i++) {
    if (tier & 1) {
      board->board[i] = (tier_position & 1) + 1;
      tier_position >>= 1;
    } else {
      board->board[i] = 0;
    }
    tier >>= 1;
  }

  return board;
}

// Helper function for getTierChildren
TIER apply_move_to_tier(TIER tier, int move) {
    TIER new_tier = tier;

    int emptyIndex = 4;
    for (int i = 0; i < 5; i++) {
      if ((tier >> move_indexes[move][i] & 1) == 0) {
        emptyIndex = i;
        break;
      }
    }

    for (int i = emptyIndex; i > 0; i--) {
      new_tier |= ((tier >> move_indexes[move][i - 1]) & 1) << move_indexes[move][i];
    }
    new_tier |= 1 << move_indexes[move][0];

    return new_tier;
}
/* The tier graph is just a single tier with id=0. */
TIERLIST *getTierChildren(TIER tier) {
  TIERLIST *childTiers = NULL;
  
  for (int move = 0; move < 10; move++) {
    TIER new_tier = apply_move_to_tier(tier, move);
    
    BOOLEAN tier_exists = FALSE;
    TIERLIST *current = childTiers;
    
    while (current != NULL) {
      if (current->tier == new_tier) {
        tier_exists = TRUE;
        break;
      }
      current = current->next;
    }
    
    if (!tier_exists) {
      childTiers = CreateTierlistNode(new_tier, childTiers);
    }
  }

  return childTiers;
}
TIERPOSITION numberOfTierPositions(TIER tier) {
  int count = 0;

  for (int i = 0; i < 25; i++) {
    if (tier & (1 << i)) {
      count++;
    }
  }

  return 2 << count;
}

/* Initialize any global variables or data structures needed before
solving or playing the game. */
void InitializeGame() {
  gMoveToStringFunPtr = &MoveToString;
  gInitialPosition = 0;
  gCanonicalPosition = GetCanonicalPosition;
  gSymmetries = TRUE;

  /* FOR THE PURPOSES OF INTERACT. FEEL FREE TO CHANGE IF SOLVING. */ 
	if (gIsInteract) {
		gLoadTierdbArray = FALSE; // SET TO TRUE IF SOLVING
	}
	/********************************/

  /* Tier-Related Initialization */
  gTierChildrenFunPtr = &getTierChildren;
  gNumberOfTierPositionsFunPtr = &numberOfTierPositions;
  gInitialTierPosition = gInitialPosition;
  kSupportsTierGamesman = TRUE;
  kExclusivelyTierGamesman = TRUE;
  gInitialTier = 0; // There will only be one tier and its ID will be 0
  //gUnDoMoveFunPtr = gUndoMove;
  //gGenerateUndoMovesToTierFunPtr = gGenerateUndoMovesToTier;
}

/* Return the hash value of the initial position. */
POSITION GetInitialPosition() {
  Slide5Board initial_board;

  initial_board.board[0] = 0; // Set player 0's turn
  for (int i = 1; i < 26; i++) {
    initial_board.board[i] = 0; 
  }

  return Hash(&initial_board);
}

/* Return a linked list of moves. */
MOVELIST *GenerateMoves(POSITION position) {
  (void)position;
  MOVELIST *moves = NULL;

  for (int i = 0; i < 10; i++) {
    moves = CreateMovelistNode(i, moves);
  }

  return moves;
}

int* get_indexes(int move) {
    return indexes[move];
}

/* Return the position that results from making the 
input move on the input position. */
POSITION DoMove(POSITION position, MOVE move) {
    Slide5Board* board = Unhash(position);
    int* indexes = get_indexes(move);
    int currentPlayerPiece = board->board[0] == 0 ? 1 : 2;

    int emptyIndex = 4;
    for (int i = 0; i < 5; i++) {
        if (board->board[indexes[i]] == 0) {
            emptyIndex = i;
            break;
        }
    }

    for (int i = emptyIndex; i > 0; i--) {
      board->board[indexes[i]] = board->board[indexes[i - 1]];
    }
    board->board[indexes[0]] = currentPlayerPiece;

    // Switch current player
    board->board[0] = 1 - board->board[0];

    POSITION toReturn = Hash(board);
    free(board);
    return toReturn;
}

/*****************************************************************
**  Determine whether the position is a primitive win,
**  primitive tie, primitive lose, or not primitive.
**  OUTPUTS: (VALUE) an enum; one of: (win, lose, tie, undecided)
**  See src/core/types.h for the value enum definition.
******************************************************************/
VALUE Primitive(POSITION position) {
  Slide5Board* board = Unhash(position);
  BOOLEAN blue_five = FALSE;
  BOOLEAN red_five = FALSE;

  for (int run = 0; run < 12; run++) {
    int *indexes = get_indexes(run);

    int blue_count = 0;
    int red_count = 0;

    for (int i = 0; i < 5; i++) {
      if (board->board[indexes[i]] == 1) {
        blue_count++;
      } else if (board->board[indexes[i]] == 2) {
        red_count++;
      }
    }

    if (blue_count == 5) {
      blue_five = TRUE;
    }
    if (red_count == 5) {
      red_five = TRUE;
    }
  }

  int turn = board->board[0];
  free(board);

  if (blue_five && red_five) {
    return tieResult;
  } else if (blue_five) {
    return turn == 0 ? win : lose;
  } else if (red_five) {
    return turn == 0 ? lose : win;
  } else {
    return undecided;
  }
}

/* Symmetry Handling: Return the canonical position. */
POSITION GetCanonicalPosition(POSITION position) {
    Slide5Board* board = Unhash(position);
    Slide5Board transformed_board;

  
    transformed_board.board[0] = board->board[0] == 0 ? 1 : 0;
    for (int i = 1; i < 26; i++) {
        if (board->board[i] == 1) {
            transformed_board.board[i] = 2;
        } else if (board->board[i] == 2) {
            transformed_board.board[i] = 1;
        } else {
            transformed_board.board[i] = 0;
        }
    }

    POSITION transformed_hash = Hash(&transformed_board);
    free(board);
    return (position < transformed_hash) ? position : transformed_hash;
}

/*********** END SOLVING FUNCTIONS ***********/







/*********** BEGIN TEXTUI FUNCTIONS ***********/

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn) {
  (void)playerName;
  (void)usersTurn;
  Slide5Board* board = Unhash(position);

  printf("  1 2 3 4 5\n");
  for (int i = 0; i < 5; i++) {
    printf("%d ", i + 1);
    for (int j = 0; j < 5; j++) {
      int cell_value = board->board[i * 5 + j + 1];
      printf("%d ", cell_value);
    }
    printf("\n");
  }

  if (board->board[0] == 0) {
    printf("It is Player 1's turn!\n");
  } else {
    printf("It is Player 2's turn!\n");
  }
  free(board);
}

void PrintComputersMove(MOVE computersMove, STRING computersName) {
  (void)computersMove;
  (void)computersName;
}

USERINPUT GetAndPrintPlayersMove(POSITION position, MOVE *move, STRING playerName) {
  USERINPUT ret;

	do {
		printf("%8s's move:  ", playerName);

		ret = HandleDefaultTextInput(position, move, playerName);
		if(ret != Continue)
			return(ret);

	}
	while (TRUE);
	return(Continue); /* this is never reached, but lint is now happy */
}

/* Return whether the input text signifies a valid move. */
BOOLEAN ValidTextInput(STRING input) {
  return input[0] >= '0' && input[0] <= '9';
}

/* Assume the text input signifies a valid move. Return
the move hash corresponding to the move. */
MOVE ConvertTextInputToMove(STRING input) {
  return atoi(input);
}

/* Return the string representation of the move. 
Ideally this matches with what the user is supposed to
type when they specify moves. */
STRING MoveToString(MOVE move) {
  char * move_str = malloc(sizeof(char) * 3);
  snprintf(move_str, 2, "%d", move);
  return move_str;
}

/* Basically just print the move. */
void PrintMove(MOVE move) {
  printf("%d", move);
}

/*********** END TEXTUI FUNCTIONS ***********/









/*********** BEGIN VARIANT FUNCTIONS ***********/

/* For implementing more than one variant */
void GameSpecificMenu() {
  char inp;
	while (TRUE) {
		printf("\n\n\n");
		printf("        ----- Game-specific options for Slide 5 -----\n\n");
		printf("        Select a game board:\n\n");
		printf("        0)      If 5-in-a-rows of both colors exist, the player who created it wins. \n");
		printf("        1)      If 5-in-a-rows of both colors exist, the game is a tie. \n");
    printf("        B)      (B)ack = Return to previous activity.\n\n\n");
		printf("\nSelect an option: ");
		inp = GetMyChar();
		if (inp == '0') {
      setOption(0);
      return;
		}
		else if (inp == '1') {
      setOption(1);
      return;
		}
		else if (inp == 'b' || inp == 'B') return;
		else {
			printf("Invalid input.\n");
		}
	}
}

/* How many variants are you supporting? */
int NumberOfOptions() {
  return 2;
}

/* Return the current variant id. */
int getOption() {
  return tieResult == lose ? 0 : 1;
}

/* The input is a variant id. This function sets any global variables
or data structures according to the variant specified by the variant id. */
void setOption(int option) {
  tieResult = option == 0 ? lose : tie;
}

/*********** END VARIANT-RELATED FUNCTIONS ***********/







/* Don't worry about these Interact functions below yet.
They are used for the AutoGUI which eventually we would
want to implement, but they are not needed for solving. */
POSITION InteractStringToPosition(STRING str) {
  enum UWAPI_Turn turn;
	unsigned int num_rows, num_columns; // Unused
	STRING board;
	if (!UWAPI_Board_Regular2D_ParsePositionString(str, &turn, &num_rows, &num_columns, &board)) {
		// Failed to parse string
		return INVALID_POSITION;
	}

	Slide5Board s5b;
	for (int i = 0; i < 25; i++) {
		switch (board[i]) {
			default:
				fprintf(stderr, "Error: Unexpected char in position\n");
				break;
			case '-':
				s5b.board[i + 1] = 0;
				break;
			case 'X':
				s5b.board[i + 1] = 1;
				break;
			case 'O':
				s5b.board[i + 1] = 2;
				break;
		}
	}
  s5b.board[0] = (turn == UWAPI_TURN_A) ? 0 : 1;

  TIER tier = 0;
  for (int i = 25; i > 0; i--) {
    tier = (tier << 1) | (s5b.board[i] > 0 ? 1 : 0);
  }
  gInitializeHashWindow(tier, FALSE);

	SafeFreeString(board); // Free the string.
	return Hash(&s5b);
}

STRING InteractPositionToString(POSITION position) {
  Slide5Board *s5b = Unhash(position);

	char board[26];
	for (int i = 1; i < 26; i++) {
		switch (s5b->board[i]) {
			default:
				fprintf(stderr, "Error: Unexpected position\n");
				break;
			case 0:
				board[i - 1] = '-';
				break;
			case 1:
				board[i - 1] = 'X';
				break;
			case 2:
				board[i - 1] = 'O';
				break;
		}
	}
	board[25] = '\0'; // Make sure to null-terminate your board.

	enum UWAPI_Turn turn = s5b->board[0] ? UWAPI_TURN_B : UWAPI_TURN_A;
  free(s5b);
	return UWAPI_Board_Regular2D_MakeBoardString(turn, 25, board);
}

STRING InteractMoveToString(POSITION position, MOVE move) {
  (void)position;
  return UWAPI_Board_Regular2D_MakeMoveStringWithSound(25 + move * 2, 26 + move * 2, 'x');
}