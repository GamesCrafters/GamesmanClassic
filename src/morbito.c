/************************************************************************
**
** NAME:        morbito.c
**
** DESCRIPTION: Oribito
**
** AUTHOR:      Ariana Abel and Allena Oglivie
**
**************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/

#include "gamesman.h"

#define BOARD_SIZE 16
#define BOARD_ROWS 4
#define BOARD_COLS 4

#define NEXT_PLAYER(player) (1 + ((player)  % 2))

#define ENCODE_MOVE(from, to, drop) (((from) << 8) | ((to) << 4) | (drop))
#define DECODE_MOVE_FROM(move) ((move) >> 8)
#define DECODE_MOVE_TO(move) (0xF & ((move) >> 4))
#define DECODE_MOVE_DROP(move) (0xF & (move))


int num_end_rotations = 5;
int diagonal_variant = 0;
int rotation_variant = 0;
int misere_variant = 0; 

POSITION gNumberOfPositions = 0;
POSITION kBadPosition = -1;

char playerPiece[] = " BW";

POSITION gInitialPosition = 0;
POSITION gMinimalPosition = 0;

CONST_STRING kAuthorName = "Ariana Abel and Allena Oglivie";
CONST_STRING kGameName = "Orbito";
BOOLEAN kPartizan = TRUE;
BOOLEAN kDebugMenu = TRUE;
BOOLEAN kGameSpecificMenu = TRUE;
BOOLEAN kTieIsPossible = TRUE;
BOOLEAN kLoopy = FALSE;
BOOLEAN kDebugDetermineValue = FALSE;
void* gGameSpecificTclInit = NULL;
CONST_STRING kHelpGraphicInterface = "To move your opponent's piece click the arrow pointing to the empty\n"
                                      "space you want to move it to. Then to place your piece click the circle \n"
                                      "on the empty space you want to drop your piece to.";

CONST_STRING kHelpTextInterface = "Enter your move in the format: XYZ or 00Z\n"
                                     "X = opponent's piece position (hex 0-f)\n"
                                     "Y = where to MOVE your opponent piece (hex 0-f)\n"
                                     "Z = where to drop your piece (hex 0-F)\n"
                                      "Use 00Z to skip moving opponent's piece! (just drop your piece at Z)\n"
                                  "Example: '15f' MOVES opponent from 1 to 5, drop at f\n"
                                  "Example: '00f' DROPS at f without moving opponent";

CONST_STRING kHelpOnYourTurn = "Optionally move ONE opponent marble to adjacent empty square, \n"
                               "Then place your marble on any empty sqaure. Board rotates counter clockwise after your turn.\n"
                               "If board is full, type 1000, to rotate board.";

CONST_STRING kHelpStandardObjective = "Get 4 of your marbles in a row (horizontally, vertically, or diagonolly)"
                                      "after the board rotates at the END of your turn.";

CONST_STRING kHelpReverseObjective = "Avoid getting 4 in a row and force your opponent to get 4 in a row";

CONST_STRING kHelpTieOccursWhen = "If both players achieve 4 in a row on the same turn after the board is rotated.\n"
                                "Or if the board is filled completely with no player achieving 4 in a row even after 5 tiebreaker rotates";

CONST_STRING kHelpExample = "Position Grid                          Current Board\n"
"  (0)(1)(2)(3)                            ↓  ←  ←  ←\n"
"  (4)(5)(6)(7)                            ↓  ↓  ←  ↑\n" 
"  (8)(9)(A)(B)                            ↓  →  ↑  ↑\n"
"  (C)(D)(E)(F)                            →  →  →  ↑\n"
"It is Player’s turn (B).\n"
"\n"
"Player’s move:  000\n"
"\n"
"  Position Grid                          Current Board\n"
"  (0)(1)(2)(3)                            ↓  ←  ←  ←\n"
"  (4)(5)(6)(7)                            B  ↓  ←  ↑\n"
"  (8)(9)(A)(B)                            ↓  →  ↑  ↑\n"
"  (C)(D)(E)(F)                            →  →  →  ↑\n"
"It is Data’s turn (W).\n"
"\n"
"Data's moved: 40d"
"\n"
"  Position Grid                          Current Board\n"
"  (0)(1)(2)(3)                            ↓  ←  ←  ←\n"
"  (4)(5)(6)(7)                            B  ↓  ←  ↑\n"
"  (8)(9)(A)(B)                            ↓  →  ↑  ↑\n"
"  (C)(D)(E)(F)                            →  →  W  ↑";

MULTIPARTEDGELIST* GenerateMultipartMoveEdges(POSITION position, MOVELIST *moveList, POSITIONLIST *positionList);

/*************************************************************************
**
** Everything above here must be in every game file
**
**************************************************************************/

/*************************************************************************
**
** Every variable declared here is only used in this file (game-specific)
**
**************************************************************************/

BOOLEAN kSupportsSymmetries = FALSE; /* Whether we support symmetries */

/************************************************************************
**
** NAME: InitializeDatabases
**
** DESCRIPTION: Initialize the gDatabase, a global variable.
**
************************************************************************/
int vcfg(int pieces[]){
    return pieces[1] == pieces[2] || pieces[1] == pieces[2] + 1;
}

void InitializeGame()
{
  int hash_data[] = {' ', 0, 16, 'B', 0, 8, 'W', 0, 8, -1};
  gNumberOfPositions = generic_hash_init(BOARD_SIZE, hash_data, vcfg, 0);
  char start[] = "                ";
  gInitialPosition = generic_hash_hash(start, 1);
  gGenerateMultipartMoveEdgesFunPtr = &GenerateMultipartMoveEdges;
}

/************************************************************************
**
** NAME: DebugMenu
**
** DESCRIPTION: Menu used to debub internal problems. Does nothing if
** kDebugMenu == FALSE
**
************************************************************************/

void DebugMenu()
{
}

/************************************************************************
**
** NAME: GameSpecificMenu
**
** DESCRIPTION: Menu used to change game-specific parmeters, such as
** the side of the board in an nxn Nim board, etc. Does
** nothing if kGameSpecificMenu == FALSE
**
************************************************************************/

void GameSpecificMenu() {
   do {
		printf("\n\t----- Game Specific options for %s -----\n\n", kGameName);

		printf("\n\tCurrent game configuration: \n");

    printf("\n\tMoving Opponents Pieces:");
		if(diagonal_variant) {
			printf("\n\tD)\tChange (D)iagonal moves from allowed to NOT allowed\n");
		}
		else {
			printf("\n\tD)\tChange (D)iagonal moves from NOT allowed to allowed\n");
		}

    printf("\n\tBoard Rotations:");
		if(rotation_variant) {
			printf("\n\tR)\tChange inner (R)otation from OPPOSITE to SAME as outer\n");
		}
		else {
			printf("\n\tR)\tChange inner (R)otation from SAME to OPPOSITE as outer\n");
		}

    printf("\n\tGame Logic Variant:");
    if(misere_variant){
      printf("\n\tM)\tChange (M)isere from ENABLED to DISABLED\n");
    }
    else {
      printf("\n\tM)\tChange (M)isere from DISABLED to ENABLED\n");
    }

    printf("\n\tB)\tTo go back\n");
    printf("\n\tQ)\tTo quit\n");

		switch(GetMyChar())
		{
    case 'M': case 'm':
      setOption(3);
      break;
		case 'D': case 'd':
			setOption(2);
			break;
		case 'R': case 'r':
			setOption(1);
			break;
		case 'Q': case 'q':
			ExitStageRight();
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
** NAME: SetTclCGameSpecificOptions
**
** DESCRIPTION: Set the C game-specific options (called from Tcl)
** Ignore if you don't care about Tcl for now.
**
************************************************************************/

void SetTclCGameSpecificOptions(int theOptions[])
{
  (void)theOptions;
}

/****************************   
!!! Helper function !!! 
Determines current player based on piece counts 
returns 1 for Black's turn B
returns 2 for White's turn W
****************************/
int GetCurrentPlayer(char* board) {
  int blackCount = 0;
  int whiteCount = 0; 

  for (int i = 0; i < BOARD_SIZE; i++) {
    if(board[i] == 'B') {
      blackCount = blackCount + 1;
    }
    else if (board[i] == 'W') {
      whiteCount = whiteCount + 1;
    }
  }

  if (blackCount == whiteCount) {
    return 1;
  }
  else {
    return 2;
  }
}

int IsFull(char* board){
   for(int i = 0; i < BOARD_SIZE; i++){
    if(board[i] == ' '){
      return 0;
    }
   }
   return 1;
}


/************************************************************************
**
** NAME: DoMove
**
** DESCRIPTION: Apply the move to the position.
**
** INPUTS: POSITION position : The old position
** MOVE move : The move to apply.
**
** OUTPUTS: (POSITION) : The position that results after the move.
**
** CALLS: PositionToBlankOX(POSITION,*BlankOX)
** BlankOX WhosTurn(*BlankOX)
**
************************************************************************/

POSITION DoMove(POSITION position, MOVE move) {
    char board[BOARD_SIZE];
    generic_hash_unhash(position, board);
    int from = DECODE_MOVE_FROM(move);
    int to = DECODE_MOVE_TO(move);
    int drop = DECODE_MOVE_DROP(move);
    int player = GetCurrentPlayer(board);
    void RotateBoard(char* board, char* rotate_board);
    
    POSITION out;
    if (!(from == 0 && to == 0)) {
      board[to] = board[from];
      board[from] = ' ';
    }
        
    // Place current player's piece
    board[drop] = playerPiece[player];
    // ROTATE THE BOARD
    char rotated[BOARD_SIZE];
    RotateBoard(board, rotated);
    out = generic_hash_hash(rotated, NEXT_PLAYER(player));
   
    return out;
}

void UndoMove(MOVE move)
{
  (void)move;
  ExitStageRightErrorString("UndoMove not implemented.");
}

/************************************************************************
**
** NAME: PrintComputersMove
**
** DESCRIPTION: Nicely format the computers move.
**
** INPUTS: MOVE *computersMove : The computer's move.
** STRING computersName : The computer's name.
**
************************************************************************/

void PrintComputersMove(MOVE computersMove, STRING computersName) {
  int from = DECODE_MOVE_FROM(computersMove);
  int to = DECODE_MOVE_TO(computersMove);
  int drop = DECODE_MOVE_DROP(computersMove);
  printf("%s moved: %x%x%x\n", computersName, from, to, drop);
}

int FourInARow(char* board, char player_char) {
  //check rows
    // Check rows
for(int row = 0; row < 4; row++){
      for(int col = 0; col <= 4 - 4; col++) {  // Only col 0 works for 4x4
          int start = row * 4 + col;
          if(board[start] == player_char && 
             board[start+1] == player_char && 
             board[start+2] == player_char && 
             board[start+3] == player_char){
              return 1;
          }
      }
  }
  //check columns
  for(int i = 0; i < 4; i++){
      if(board[i] == board[i+4] && board[i] == board[i+8] && board[i] == board[i+12] && board[i] == player_char){
          return 1; 
      }
  }
  //check diagonals
  if(board[0] == board[5] && board[0] == board[10] && board[0] == board[15] && board[0] == player_char){
      return 1; 
  }
  if(board[3] == board[6] && board[3] == board[9] && board[3] == board[12] && board[3] == player_char){
      return 1; 
  }
  return 0;
}


void RotateBoard(char* board, char* rotate_board){
   //top row: 0, 1, 2
   for (int i = 0; i < 3; i++){   
       rotate_board[i] = board[i+1];
   }
   //left column: 4, 8, 12
   for (int i = 4; i < 13; i+=4){   
       rotate_board[i] = board[i-4];
   }
   //bottom row: 13, 14, 15
   for (int i = 13; i < 16; i++){   
       rotate_board[i] = board[i-1];
   }
   //right column: 3, 7, 11
   for (int i = 3; i < 12; i+=4){   
       rotate_board[i] = board[i+4];
   }
   //middle
   //opposite rotation variant
   if(rotation_variant){
       rotate_board[5] = board[9];
       rotate_board[6] = board[5];
       rotate_board[9] = board[10];
       rotate_board[10] = board[6];
   } else {
       //original rotation
       rotate_board[5] = board[6];
       rotate_board[6] = board[10];
       rotate_board[9] = board[5];
       rotate_board[10] = board[9];
   }
}


/************************************************************************
**
** NAME: Primitive
**
** DESCRIPTION: Return the value of a position if it fulfills certain
** 'primitive' constraints. Some examples of this is having
** three-in-a-row with TicTacToe. TicTacToe has two
** primitives it can immediately check for, when the board
** is filled but nobody has one = primitive tie. Three in
** a row is a primitive lose, because the player who faces
** this board has just lost. I.e. the player before him
** created the board and won. Otherwise undecided.
**
** INPUTS: POSITION position : The position to inspect.
**
** OUTPUTS: (VALUE) an enum which is oneof: (win,lose,tie,undecided)
**
** CALLS: BOOLEAN ThreeInARow()
** BOOLEAN AllFilledIn()
** PositionToBlankOX()
**
************************************************************************/

VALUE Primitive(POSITION position)
{
  char board[BOARD_SIZE];
  generic_hash_unhash(position, board);
  int player = GetCurrentPlayer(board);
  char my_char = playerPiece[player];
  char opponent_char = playerPiece[NEXT_PLAYER(player)];

  int op_4 = FourInARow(board, opponent_char);
  int my_4 = FourInARow(board, my_char);
  
  if(op_4 && my_4){
    return tie;
  } else if(op_4){
    if (misere_variant) {
      return win;
    }
    return lose;
  } else if(my_4){
    if (misere_variant) {
      return lose;
    }
    return win;
  }


  int blackCount = 0;
  int whiteCount = 0;
  for (int i = 0; i < BOARD_SIZE; i++) {
    if(board[i] == 'B') {
      blackCount = blackCount + 1;
    }
    else if (board[i] == 'W') {
      whiteCount = whiteCount + 1;
    }
  }

if(blackCount == 8 && whiteCount == 8){
  for(int i = 0; i < num_end_rotations; i++){
        char new_board[BOARD_SIZE];
        RotateBoard(board, new_board);
        
        op_4 = FourInARow(new_board, opponent_char);
        my_4 = FourInARow(new_board, my_char);
        if(op_4 && my_4){
            return tie;
        } else if(op_4){
            if (misere_variant) {
              return win;
            }
            return lose;
        } else if(my_4){
            if (misere_variant) {
              return lose;
            }
            return win;
        }
        
        // Copy new_board back to board for next rotation
        for(int j = 0; j < BOARD_SIZE; j++){
            board[j] = new_board[j];
        }
    }
  return tie;
} else {
    return undecided;
  }
  
}

/************************************************************************
**
** NAME: PrintPosition
**
** DESCRIPTION: Print the position in a pretty format, including the
** prediction of the game's outcome.
**
** INPUTS: POSITION position : The position to pretty print.
** STRING playerName : The name of the player.
** BOOLEAN usersTurn : TRUE <==> it's a user's turn.
** â†’, â†, â†“, â†‘ â† &#x2190;
************************************************************************/

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn)
{
    (void)usersTurn;
    char board[BOARD_SIZE];
    generic_hash_unhash(position, board);
    int currPlayer = GetCurrentPlayer(board);
    
    printf("\n");
    printf("  Position Grid                          Current Board\n");
    
    // Print row by row
    for(int row = 0; row < BOARD_ROWS; row++){
        // Left side: position labels
        printf("  ");
        for(int col = 0; col < BOARD_COLS; col++){
            printf("(%X)", row * BOARD_COLS + col);
        }
        
        printf("                           ");
        
        // Right side: actual board with pieces or arrows
        char* arrows[BOARD_ROWS][BOARD_COLS] = {
            {"↓", "←", "←", "←"},
            {"↓", "↓", "←", "↑"},
            {"↓", "→", "↑", "↑"},
            {"→", "→", "→", "↑"}
        };

        if(rotation_variant) {
          arrows[1][1] = "→";
          arrows[1][2] = "↓";
          arrows[2][1] = "↑";
          arrows[2][2] = "←";
        }
        
        for(int col = 0; col < BOARD_COLS; col++){
            int pos = row * BOARD_COLS + col;
            if(board[pos] == 'B'){
                printf(" B ");
            } else if(board[pos] == 'W'){
                printf(" W ");
            } else {
                printf(" %s ", arrows[row][col]);
            }
        }
        if(row == 1){
            printf("\t%s", GetPrediction(position,playerName,usersTurn));
        }
        printf("\n");
    }
    
    printf("\nIt is %s's turn (%c).\n", playerName, playerPiece[currPlayer]);

    
}

/************************************************************************
**
** NAME: GenerateMoves
**
** DESCRIPTION: Create a linked list of every move that can be reached
** from this position. Return a pointer to the head of the
** linked list.
**
** INPUTS: POSITION position : The position to branch off of.
**
** OUTPUTS: (MOVELIST *), a pointer that points to the first item
** in the linked list of moves that can be generated.
**
** CALLS: MOVELIST *CreateMovelistNode(MOVE,MOVELIST *)
**
************************************************************************/

// Fix GenerateMoves - use actual positions
MOVELIST *GenerateMoves(POSITION position) {
    char board[BOARD_SIZE];
    generic_hash_unhash(position, board);
    int player = GetCurrentPlayer(board);
    char opponent_char = playerPiece[NEXT_PLAYER(player)];
    
    MOVELIST *moves = NULL;
    
    // Generate moves without moving opponent
    for(int i = 0; i < BOARD_SIZE; i++){
        if(board[i] == ' '){
            moves = CreateMovelistNode(ENCODE_MOVE(0, 0, i), moves);
        }
    }
    
    // Generate moves with moving opponent
    for(int from = 0; from < BOARD_SIZE; from++){
        if(board[from] != opponent_char) continue;
        
        if(diagonal_variant){
          int adjacents[8] = {from - 1, from + 1, from - 4, from + 4, from - 3, from + 3, from - 5, from + 5};
          for(int i = 0; i < 8; i++){
            int to = adjacents[i];

            // Validate adjacency and bounds
            if(to < 0 || to >= BOARD_SIZE) continue;
            if(board[to] != ' ') continue;
            
            // Check wrapping (left edge to right edge)
            if(from % 4 == 0 && (to == from - 1 || to == from - 5 || to == from + 3)) continue;
            if(from % 4 == 3 && (to == from + 1 || to == from + 5 || to == from - 3)) continue;
            
            // Generate move for each empty drop position
            for(int drop = 0; drop < BOARD_SIZE; drop++){
                if((board[drop] == ' ' || drop == from) && drop != to){
                    moves = CreateMovelistNode(ENCODE_MOVE(from, to, drop), moves);
                }
            }
          }
        } else {
          // Check 4 adjacent positions
          int adjacents[4] = {from - 1, from + 1, from - 4, from + 4};
          for(int i = 0; i < 4; i++){
              int to = adjacents[i];
              
              // Validate adjacency and bounds
              if(to < 0 || to >= BOARD_SIZE) continue;
              if(board[to] != ' ') continue;
              
              // Check wrapping (left edge to right edge)
              if(from % 4 == 0 && to == from - 1) continue;
              if(from % 4 == 3 && to == from + 1) continue;
              
              // Generate move for each empty drop position
              for(int drop = 0; drop < BOARD_SIZE; drop++){
                  if((board[drop] == ' ' || drop == from) && drop != to){
                      moves = CreateMovelistNode(ENCODE_MOVE(from, to, drop), moves);
                  }
              }
          }
        }
        
    }
    
    return moves;
}
/**************************************************/
/**************** SYMMETRY FUN BEGIN **************/
/**************************************************/

/************************************************************************
**
** NAME: GetCanonicalPosition
**
** DESCRIPTION: Go through all of the positions that are symmetrically
** equivalent and return the SMALLEST, which will be used
** as the canonical element for the equivalence set.
**
** INPUTS: POSITION position : The position return the canonical elt. of.
**
** OUTPUTS: POSITION : The canonical element of the set.
**
************************************************************************/

POSITION GetCanonicalPosition(POSITION position)
{
  return position;
}

/************************************************************************
**
** NAME: DoSymmetry
**
** DESCRIPTION: Perform the symmetry operation specified by the input
** on the position specified by the input and return the
** new position, even if it's the same as the input.
**
** INPUTS: POSITION position : The position to branch the symmetry from.
** int symmetry : The number of the symmetry operation.
**
** OUTPUTS: POSITION, The position after the symmetry operation.
**
************************************************************************/

POSITION DoSymmetry(POSITION position, int symmetry) {
  (void)symmetry;
  return position;
}

/**************************************************/
/**************** SYMMETRY FUN END ****************/
/**************************************************/

/************************************************************************
**
** NAME: GetAndPrintPlayersMove
**
** DESCRIPTION: This finds out if the player wanted an undo or abort or not.
** If so, return Undo or Abort and don't change theMove.
** Otherwise get the new theMove and fill the pointer up.
**
** INPUTS: POSITION *thePosition : The position the user is at.
** MOVE *theMove : The move to fill with user's move.
** STRING playerName : The name of the player whose turn it is
**
** OUTPUTS: USERINPUT : Oneof( Undo, Abort, Continue )
**
** CALLS: ValidMove(MOVE, POSITION)
** BOOLEAN PrintPossibleMoves(POSITION) ...Always True!
**
************************************************************************/

USERINPUT GetAndPrintPlayersMove(POSITION thePosition, MOVE *theMove, STRING playerName) {
  USERINPUT ret;
  do {
    printf("%s's move (XYZ), 00Z to only drop:  ", playerName);
    ret = HandleDefaultTextInput(thePosition, theMove, playerName);
    if (ret != Continue) {
			return ret;
    }
  } while (TRUE);
  return Continue;
}

/************************************************************************
**
** NAME: ValidTextInput
**
** DESCRIPTION: Return TRUE iff the string input is of the right 'form'.
** For example, if the user is allowed to select one slot
** from the numbers 1-9, and the user chooses 0, it's not
** valid, but anything from 1-9 IS, regardless if the slot
** is filled or not. Whether the slot is filled is left up
** to another routine.
**
** INPUTS: STRING input : The string input the user typed.
**
** OUTPUTS: BOOLEAN : TRUE iff the input is a valid text input.
**
************************************************************************/

BOOLEAN ValidTextInput(STRING input)
{
  int len = strlen(input);
  printf("length of input: %d", len);
  //if not 3, not valid 
  if (len == 1 && input[0] == 'n') {
    return TRUE;
  }

  if(len != 3){
    return FALSE;
  }
  if (!isxdigit(input[0])) {
    return FALSE;
  }
  if (!isxdigit(input[1])) {
    return FALSE;
  }
  if (!isxdigit(input[2])) {
    return FALSE;
  }

  return TRUE;
}

/************************************************************************
**
** NAME: ConvertTextInputToMove
**
** DESCRIPTION: Convert the string input to the internal move representation.
**
** INPUTS: STRING input : The string input the user typed.
**
** OUTPUTS: MOVE : The move corresponding to the user's input.
**
************************************************************************/

MOVE ConvertTextInputToMove(STRING input) {
    if(strlen(input) == 1 && input[0] == 'n'){
      return 0x1000;
    }
    int from = (tolower(input[0]) >= 'a') ? 
               (tolower(input[0]) - 'a' + 10) : (input[0] - '0');
    int to = (tolower(input[1]) >= 'a') ? 
             (tolower(input[1]) - 'a' + 10) : (input[1] - '0');
    int drop = (tolower(input[2]) >= 'a') ? 
               (tolower(input[2]) - 'a' + 10) : (input[2] - '0');
    return ENCODE_MOVE(from, to, drop);
}


/************************************************************************
**
** NAME: PrintMove
**
** DESCRIPTION: Print the move in a nice format.
**
** INPUTS: MOVE *theMove : The move to print.
**
************************************************************************/

void PrintMove(MOVE move)
{
  if(move == 0x1000){
    printf("n");
  } else {
    int from = DECODE_MOVE_FROM(move);
    int to = DECODE_MOVE_TO(move);
    int drop = DECODE_MOVE_DROP(move);
    printf("%x%x%x", from, to, drop);
  }
}


/************************************************************************
**
** NAME: MoveToString
**
** DESCRIPTION: Returns the move as a STRING
**
** INPUTS: MOVE *move : The move to put into a string.
**
************************************************************************/

void MoveToString (MOVE move, char *moveStringBuffer)
{
  int real_move = move & 0x0FFF;
  snprintf(moveStringBuffer, 4, "%x%x%x", DECODE_MOVE_FROM(real_move), DECODE_MOVE_TO(real_move), DECODE_MOVE_DROP(real_move));
  
}

CONST_STRING kDBName = "orbito";

int NumberOfOptions()
{
  return 5;
}

int getOption()
{
  if (rotation_variant && diagonal_variant){
    return 4;
  } else if (misere_variant) {
    return 3;
  }
   else if (diagonal_variant){
    return 2;
  } 
  else if (rotation_variant){
    return 1;
  }
  return 0;
}

void setOption(int option)
{
  if(option == 1){
    rotation_variant = (rotation_variant + 1)%2;
  }
  if(option == 2){
    diagonal_variant = (diagonal_variant + 1)%2;
  }
  if(option == 3) {
    misere_variant = (misere_variant + 1)%2;
  }
  if(option == 4) {
    rotation_variant = (rotation_variant + 1)%2;
    diagonal_variant = (diagonal_variant + 1)%2;
  }
}

POSITION ActualNumberOfPositions(int variant)
{
  (void)variant;
  return 0;
}


POSITION StringToPosition(char *positionString) {
	int turn;
	char *board_str;
	if (ParseStandardOnelinePositionString(positionString, &turn, &board_str)) {
    char board[BOARD_SIZE];
    for(int i = 0; i < BOARD_SIZE; i++){
      if(board_str[i] == '-'){
        board[i] = ' ';
      } else {
        board[i] = board_str[i];
      }
    }
		POSITION p = generic_hash_hash(board, turn);
    return p;
	}
	return NULL_POSITION;
}

void PositionToAutoGUIString(POSITION position, char *autoguiPositionStringBuffer) {

  char board[BOARD_SIZE];
  generic_hash_unhash(position, board);
  int player = GetCurrentPlayer(board);

  char positionstring[BOARD_SIZE + 1];
  positionstring[BOARD_SIZE] = '\0';
  for(int i = 0; i < BOARD_SIZE; i++){
    if(board[i] == ' '){
      positionstring[i] = '-';
    } else {
      positionstring[i] = board[i];
    }
  }
  AutoGUIMakePositionString(player, positionstring, autoguiPositionStringBuffer);
}



/**
 * MoveToAutoGUIString
 * Converts a move into a GUI string representation for multipart moves
 * 
 * In Orbito, a full turn consists of:
 * 1. (Optional) Move opponent's marble to adjacent empty square
 * 2. Place your marble on any empty square
 * 3. Board rotates counter-clockwise
 *
 * Move encoding: ENCODE_MOVE(from, to, drop)
 * - from: opponent piece position (0-F, or 0 if not moving)
 * - to: where opponent piece moves (0-F, or 0 if not moving)
 * - drop: where to place your piece (0-F)
 */
void MoveToAutoGUIString(POSITION position, MOVE move, char *autoguiMoveStringBuffer) {
    (void) position;
    
    int from = DECODE_MOVE_FROM(move & 0x0FFF);
    int to = DECODE_MOVE_TO(move & 0x0FFF);
    int drop = DECODE_MOVE_DROP(move & 0x0FFF);
  
    if (move & 0x1000) { 
        AutoGUIMakeMoveButtonStringM(from, to, 'x', autoguiMoveStringBuffer);
    } 
    else if (move & 0x2000) { 
        AutoGUIMakeMoveButtonStringA('-', drop, 'x', autoguiMoveStringBuffer);
    } 
    else {
        // Single-part or complete move
        if (from == 0 && to == 0) { 
            // Single-part: just place marble, don't move opponent piece
            AutoGUIMakeMoveButtonStringA('-', drop, 'x', autoguiMoveStringBuffer);
        } 
        else { 
            // Full multipart move completed
            AutoGUIWriteEmptyString(autoguiMoveStringBuffer);
        }
    }
}

/**
 * GenerateMultipartMoveEdges
 * Generates the intermediate positions and edges for multipart moves
 * 
 * Creates the UI interaction flow for:
 * - Deciding whether to move opponent piece
 * - If yes: selecting which piece and where to move it
 * - Then: selecting drop position for your piece
 */
MULTIPARTEDGELIST* GenerateMultipartMoveEdges(POSITION position, MOVELIST *moveList, POSITIONLIST *positionList) {
    MULTIPARTEDGELIST *mpel = NULL;
    
    while (moveList != NULL) {
        int normal_move = moveList->move  & 0x0FFF;
        int from = DECODE_MOVE_FROM(normal_move);
        int to = DECODE_MOVE_TO(normal_move);
        int drop = DECODE_MOVE_DROP(normal_move);
        
        if (!(from == 0 && to == 0)) {
            // This move involves moving opponent's piece
            // Intermediate position for selecting which piece to move
            char board[BOARD_SIZE];
            generic_hash_unhash(position, board);
            int player = GetCurrentPlayer(board);
            board[to] = board[from];
            board[from] = ' ';

            POSITION movePieceInterPos = generic_hash_hash(board, player);  // Or create intermediate if needed
            
            mpel = CreateMultipartEdgeListNode(NULL_POSITION, movePieceInterPos, (0x1000 | normal_move), 0, mpel);

            mpel = CreateMultipartEdgeListNode(movePieceInterPos, NULL_POSITION, (0x2000 | normal_move), moveList->move, mpel);
        }
        
        moveList = moveList->next;
        positionList = positionList->next;
    }
    
    return mpel;
}

