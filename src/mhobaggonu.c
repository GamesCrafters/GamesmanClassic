/************************************************************************
** NAME:        mhobaggonu.c
** DESCRIPTION: Ho Bag Gonu (ludii.com)
** AUTHOR:      Nakul Srikanth, Ryan Lee, Alec Van Kerckhove
** DATE:        2004-10-18
** UPDATE HIST: Version 1.0
**************************************************************************/

#include "gamesman.h"

/* IMPORTANT GLOBAL VARIABLES */
CONST_STRING kAuthorName = "Nakul Srikanth, Ryan Lee, Alec Van Kerckhove";
CONST_STRING kGameName = "Ho Bag Gonu";
CONST_STRING kDBName = "hobaggonu";
POSITION gNumberOfPositions = 0;
POSITION gInitialPosition = 0;
BOOLEAN kPartizan = FALSE;
BOOLEAN kTieIsPossible = FALSE;
BOOLEAN kLoopy = TRUE;
BOOLEAN kSupportsSymmetries = FALSE;

/* Likely you do not have to change these. */
POSITION GetCanonicalPosition(POSITION);
// STRING MoveToString(MOVE);
POSITION kBadPosition = -1;
BOOLEAN kDebugDetermineValue = FALSE;
void* gGameSpecificTclInit = NULL;

/* You do not have to change these for now. */
BOOLEAN kGameSpecificMenu = FALSE;
BOOLEAN kDebugMenu = FALSE;


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

/* Do not worry about this yet because you will only be supporting 1 variant for now. */
void GameSpecificMenu() {}

#define MAX_ITEMS 11 // Maximum number of key-value pairs
#define MAX_KEY_SIZE 50
#define MAX_VALUE_SIZE 50
#define ENCODE_MOVE(start, end) ((start) * 11 + end)
#define DECODE_MOVE_START(move) (move / 11)
#define DECODE_MOVE_END(move) (move % 11)
// Based on mponghauki, player is either 1 or 2.
#define NEXT_PLAYER(player) (1 + (player % 2))
#define BOARDSIZE 11
char pieces[] = " XO";


// Define a struct to hold key-value pairs
typedef struct {
    char key[MAX_KEY_SIZE];
    char value[MAX_KEY_SIZE];
} DictionaryItem;

// Define a dictionary struct
typedef struct {
    DictionaryItem items[MAX_ITEMS];
    int size; // To keep track of the number of key-value pairs
} Dictionary;

// Function to initialize the dictionary
void initDictionary(Dictionary *dict) {
    dict->size = 0;
}

// Function to add a key-value pair to the dictionary
void addItem(Dictionary *dict, const char *key, const char *value) {
    if (dict->size < MAX_ITEMS) {
        strncpy(dict->items[dict->size].key, key, MAX_KEY_SIZE - 1); // Prevent overflow
        strncpy(dict->items[dict->size].value, value, MAX_VALUE_SIZE - 1); // Prevent overflow
        dict->items[dict->size].key[MAX_KEY_SIZE - 1] = '\0';  // Ensure null-termination
        dict->items[dict->size].value[MAX_VALUE_SIZE - 1] = '\0';  // Ensure null-termination
        dict->size++;
    } else {
        printf("Dictionary is full!\n");
    }
}


// Function to get the value associated with a key
const char* getItem(Dictionary *dict, const char *key) {
    for (int i = 0; i < dict->size; i++) {
        if (strcmp(dict->items[i].key, key) == 0) {
            return dict->items[i].value;
        }
    }
    return NULL; // Key not found
}

/*********** BEGIN SOLVING FUNCIONS ***********/
Dictionary moves_lookup;

/* TODO: Add a hashing function and unhashing function, if needed. */

/* Initialize any global variables or data structures needed before
solving or playing the game. */
void InitializeGame() {
  gCanonicalPosition = GetCanonicalPosition;
  // gMoveToStringFunPtr = &MoveToString;

  initDictionary(&moves_lookup);

  addItem(&moves_lookup, "0", "1");
  addItem(&moves_lookup, "1", "0,2,3");
  addItem(&moves_lookup, "2", "1");
  addItem(&moves_lookup, "3", "1,4,5,6");
  addItem(&moves_lookup, "4", "3,5,7");
  addItem(&moves_lookup, "5", "3,4,6,7");
  addItem(&moves_lookup, "6", "3,5,7");
  addItem(&moves_lookup, "7", "4,5,6,9");
  addItem(&moves_lookup, "8", "9");
  addItem(&moves_lookup, "9", "7,8,10");
  addItem(&moves_lookup, "10", "9");
  
  

  char board[] = "XXX-----OOO";
  

  int hash_data[] = {'-', 5, 5, 'X', 3, 3, 'O', 3, 3, -1};
  gNumberOfPositions = generic_hash_init(BOARDSIZE, hash_data, NULL, 0);
  gInitialPosition = generic_hash_hash(board, 1);

}

/* Return the hash value of the initial position. */
POSITION GetInitialPosition() {
  /* YOUR CODE HERE */
  return gInitialPosition;
}

/* Return a linked list of moves. */
MOVELIST *GenerateMoves(POSITION position) {
  /* YOUR CODE HERE 
     
     To add to the linked list, do 
     moves = CreateMovelistNode(<the move you're adding>, moves);
     See the function CreateMovelistNode in src/core/misc.c
  */
  char board[BOARDSIZE];
  generic_hash_unhash(position, board);
  int player = generic_hash_turn(position);
  MOVELIST *moves = NULL;


  // loop through char for each position... 

  // for each possibe move for that position(dictionary), create moves lisgt noe(encode move())
  //x player at 3 cannot go into 0,1,2 --> O player in 7 cannot go into  8,9,10

  for(int cpos = 0; cpos < BOARDSIZE;cpos++){
    if(pieces[player] == board[cpos]){ // should this be the position decoded... 
      //x player at 3 cannot go into 0,1,2 --> O player in 7 cannot go into  8,9,10
      char key[3];
      sprintf(key, "%d", cpos);
      
      const char *possibleMoves = getItem(&moves_lookup, key); //pull from dictionary
      char tempMoves[100];  // Or some appropriate size
      strcpy(tempMoves, possibleMoves);  // Copy the string to a modifiable array 


      int moveCount = 1; //move count working, temp moves working ex: "0,2,3"
      for (int i = 0; tempMoves[i] != '\0'; i++) { //find number of moves for iteration
          if (tempMoves[i] == ',') {
            moveCount++;
          }
      }

      char *token = strtok(tempMoves, ",");

      for (int i = 0; i < moveCount;i++){
          int targetPos = atoi(token);

          if (cpos >= 3 && pieces[player] == 'X'){
            if(targetPos == 0 || targetPos == 1 || targetPos == 2){
              token = strtok(NULL, ", ");
              continue;
            }
          } else if (cpos <= 7 && pieces[player] == 'O') {
            if(targetPos == 8 || targetPos == 9 || targetPos == 10){
              token = strtok(NULL, ", ");
              continue;
            }
          }
          if (board[targetPos] != '-') {
            token = strtok(NULL, ", ");
            continue;
          }

          // printf("Generate Move from %d\n", cpos);
          // printf("Move to          : %d\n", targetPos);
          // printf("________\n");
            
          moves = CreateMovelistNode(ENCODE_MOVE(cpos, targetPos), moves);
          token = strtok(NULL, ", ");
      }
    }
  }

  return moves;
}


/* Return the position that results from making the 
input move on the input position. */
POSITION DoMove(POSITION position, MOVE move) {
  /* YOUR CODE HERE */
  
  char* board = (char*) SafeMalloc(16 * sizeof(char));
  board = generic_hash_unhash(position, board);

  // Get start and end position
  int spos = DECODE_MOVE_START(move);
  int npos = DECODE_MOVE_END(move);
  int player = generic_hash_turn(position);

  assert(board[spos] == pieces[player]);
  assert(board[npos] == '-');


  // printf("DO MOVES Player: %d\n", player);
  // printf("Do MOVES move from: %d\n",spos);
  // printf("Do MOVES move to: %d\n",npos);


  // Perform the move
  board[spos] = '-';
  board[npos] = pieces[player];

  // Create the new position
  POSITION out = generic_hash_hash(board, NEXT_PLAYER(player));
  SafeFree(board);
  return out;
 
}

/*****************************************************************
**  Determine whether the position is a primitive win,
**  primitive tie, primitive lose, or not primitive.
**  OUTPUTS: (VALUE) an enum; one of: (win, lose, tie, undecided)
**  See src/core/types.h for the value enum definition.
******************************************************************/
VALUE Primitive(POSITION position) {
  /* YOUR CODE HERE */
  //if generate moves on position is null, then return lose
  if(GenerateMoves(position) == NULL)
    return lose;
  return undecided;
  
}

/* Symmetry Handling: Return the canonical position. */
POSITION GetCanonicalPosition(POSITION position) {
  /* YOUR CODE HERE */
  return position;
}

/*********** END SOLVING FUNCTIONS ***********/







/*********** BEGIN TEXTUI FUNCTIONS ***********/

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn) {
  /* THIS ONE IS MOST IMPORTANT FOR YOUR DEBUGGING */
  /* YOUR CODE HERE */

  char board[BOARDSIZE];
  generic_hash_unhash(position, board);
  int player = generic_hash_turn(position);

  printf("\n          0-1-2          :     ");
  printf("%c-", board[0]);
  printf("%c-", board[1]);
  printf("%c", board[2]);
  printf("\n            |                    |");
  printf("\nLEGEND:     3       BOARD:       ");
  printf("%c", board[3]);

  printf("     %s", GetPrediction(position, playerName, usersTurn));
  printf("\n           /|\\                  /|\\ ");
  printf("\n          4-5-6          :     ");
  printf("%c ", board[4]);
  printf("%c ", board[5]);
  printf("%c", board[6]);
  printf("\n           \\|/                  \\|/ ");
  printf("\nLEGEND:     7       BOARD:       ");
  printf("%c", board[7]);
  printf("\n            |                    |");

  printf("\n          8-9-10         :     ");
  printf("%c-", board[8]);
  printf("%c-", board[9]);
  printf("%c", board[10]);


  printf("\n\nIt is %s's turn (%c).\n", playerName, pieces[player]);
}

void PrintComputersMove(MOVE computersMove, STRING computersName) {
  /* YOUR CODE HERE */
  char moveStringBuffer[32];
  MoveToString(computersMove, moveStringBuffer);
  printf("%s's move: %s\n", computersName, moveStringBuffer);
}

// referenced from mharegame.c
USERINPUT GetAndPrintPlayersMove(POSITION position, MOVE *move, STRING playerName) {
  /* YOUR CODE HERE */
  USERINPUT ret;
	do {
		printf("%8s's move [(u)ndo]/[src,dest] :  ", playerName);
		ret = HandleDefaultTextInput(position, move, playerName);
		if (ret != Continue) {
			return ret;
        }
	} while (TRUE);
	return Continue;
}

/* Return whether the input text signifies a valid move. */
BOOLEAN ValidTextInput(STRING input) {
  /* YOUR CODE HERE */
  // if '10' is not the starting/ending position, then the string must be of length 2
  if (strlen(input) == 2) {
    return ((input[0] <= '9' && input[0] >= '0') && (input[1] <= '9' && input[1] >= '0'));
  } else {
    //if the secd
    if (input[2] == '0') {
      return input[1] == '1' && (input[0] <= '9' && input[0] >= '0');
    } else {
      return input[0] == '1' && input[1] == '0' && (input[2] <= '9' && input[2] >= '0');
    }
  }
  return FALSE;
}

/* Assume the text input signifies a valid move. Return
the move hash corresponding to the move. */
MOVE ConvertTextInputToMove(STRING input) {
  /* YOUR CODE HERE */
  int dest, src;

  if (strlen(input) == 2) {
    src = input[0] - '0';
    dest = input[1] - '0';
  } else {
    if (input[1] == '0') { // src is 10
      src = 10;
      dest = input[2] - '0';
    } else { //dest is 10
      src = input[0] - '0';
      dest = 10;
    }
  }

  return ENCODE_MOVE(src, dest);
}

/* Return the string representation of the move. 
Ideally this matches with what the user is supposed to
type when they specify moves. */
// STRING MoveToString(MOVE move) {
//   /* YOUR CODE HERE */
//   //printf(m, "%d", move); //not sure where to return this to, using mharegame as reference but idk what 'm' is
// }

/* Basically just print the move. */
void PrintMove(MOVE move) {
  /* YOUR CODE HERE */
  printf  ("%02d", move);
}

/*********** END TEXTUI FUNCTIONS ***********/


/*********** BEGIN VARIANT FUNCTIONS ***********/

/* How many variants are you supporting? */
int NumberOfOptions() {
  /* YOUR CODE HERE */
  return 1;
}

/* Return the current variant id. */
int getOption() {
  /* YOUR CODE HERE */
  return 0;
}

/* The input is a variant id. This function sets any global variables
or data structures according to the variant specified by the variant id. */
void setOption(int option) {
  (void)option;
}

/*********** END VARIANT-RELATED FUNCTIONS ***********/

// converts formal position strings to position hashes
POSITION StringToPosition(char *positionString) {
	int turn;
	char *board;

	if (ParseStandardOnelinePositionString(positionString, &turn, &board)) {
    // for (int i=0; i<BOARDSIZE; i+=1) {
    //   if (board[i] == '-') {
    //     board[i] = ' ';
    //   }
    // }
		return generic_hash_hash(board, turn);
	}

	return NULL_POSITION;
}

// converts position hashes to AutoGUI position strings
void PositionToAutoGUIString(POSITION position, char *autoguiPositionStringBuffer) {
  char board[BOARDSIZE+1];
  generic_hash_unhash(position, board);
  
  AutoGUIMakePositionString(generic_hash_turn(position), board, autoguiPositionStringBuffer);
}

// converts internal representations of moves to AutoGUI move strings
void MoveToAutoGUIString(POSITION position, MOVE move, char *autoguiMoveStringBuffer) {
  (void)position;
  // Get start and end position
  int spos = DECODE_MOVE_START(move);
  int npos = DECODE_MOVE_END(move);

  AutoGUIMakeMoveButtonStringM(spos, npos, 's', autoguiMoveStringBuffer);
}


void MoveToString (MOVE move, char *moveStringBuffer) {
  // Get start and end position
  int spos = DECODE_MOVE_START(move);
  int npos = DECODE_MOVE_END(move);

  AutoGUIMakeMoveButtonStringM(spos, npos, 's', moveStringBuffer);
}
