/************************************************************************
**
** NAME:        dorvoncag.c
**
** DESCRIPTION: Dorvon cag
**
** AUTHOR:      Yishen, Ethan, Vad
**
**************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/

#include "gamesman.h"

#define BOARD_SIZE 7
#define MAX_MOVE_STRING_SIZE (20 * 2 + 2)

#define ENCODE_MOVE(start, end) ((start) << 16 | (end))
#define DECODE_MOVE_START(move) ((move) >> 16)
#define DECODE_MOVE_END(move) (0xffff & (move))
#define NEXT_PLAYER(player) (1 + (player  % 2))

POSITION gNumberOfPositions = 0;
POSITION kBadPosition = -1;

char playerPiece[] = " XO";

POSITION gInitialPosition = 0;
POSITION gMinimalPosition = 0;

CONST_STRING kAuthorName = "Ethan Van Yishen";
CONST_STRING kGameName = "Dörvön Cag";
BOOLEAN kPartizan = TRUE;
BOOLEAN kDebugMenu = TRUE;
BOOLEAN kGameSpecificMenu = FALSE;
BOOLEAN kTieIsPossible = TRUE;
BOOLEAN kLoopy = TRUE;
BOOLEAN kDebugDetermineValue = FALSE;
void* gGameSpecificTclInit = NULL;

CONST_STRING kHelpGraphicInterface = "";

CONST_STRING kHelpTextInterface = "";

CONST_STRING kHelpOnYourTurn = "Please enter your move in the format 0-4,0-4";

CONST_STRING kHelpStandardObjective = "Slide your pieces along the lines to prevent your opponent from moving.";

CONST_STRING kHelpReverseObjective = "";

CONST_STRING kHelpTieOccursWhen = /* Should follow 'A Tie occurs when... */ "";

CONST_STRING kHelpExample = "";

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

void InitializeGame()
{
  int hash_data[] = {' ', 1, 5, 'O', 1, 3, 'X', 1, 3, -1};
  gNumberOfPositions = generic_hash_init(BOARD_SIZE, hash_data, NULL, 0);
  char start[] = "     XO";
  gInitialPosition = generic_hash_hash(start, 1);
  /* This game is the same game as Blocking and is known to be pure draw. */
  kUsePureDraw = TRUE;
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

POSITION DoMove(POSITION position, MOVE move)
{
  char board[BOARD_SIZE];
  generic_hash_unhash(position, board);
  int start = DECODE_MOVE_START(move);
  int end = DECODE_MOVE_END(move);
  int player = generic_hash_turn(position);

  //printf("DoMove called with start: %d, end: %d, player: %d\n", start, end, player);
  
  if (start == 5 || start == 6) {
    // 放置棋子
    //printf("I love jasmine\n");
    assert(board[end] == ' ');
    assert(board[start] != ' ');
    board[end] = board[start];
  } else {
    // 正常移动
    assert(board[end] == ' ');
    assert(board[start] != ' ');
    board[end] = board[start];
    board[start] = ' ';
  }
  
  POSITION out = generic_hash_hash(board, NEXT_PLAYER(player));
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
  int start = DECODE_MOVE_START(computersMove);
  int end = DECODE_MOVE_END(computersMove);
  printf("%s moved: %d, %d\n", computersName, start, end);
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
  MOVELIST *moves = GenerateMoves(position);
  if (moves != NULL) {
    FreeMoveList(moves);
    return undecided;
  } else {
    return lose;
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
**
** CALLS: PositionToBlankOX()
** GetValueOfPosition()
** GetPrediction()
**
************************************************************************/

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn)
{
  (void)usersTurn;
  char board[BOARD_SIZE];
  generic_hash_unhash(position, board);
  printf("\n");
  printf("    2 -- 1    %c -- %c\n", board[2], board[1]);
  printf("    |\\   |     |\\  |\n");
  printf("    | 0  |     | %c |\n", board[0]);
  printf("    |  \\ |     |  \\|\n");
  printf("    3 -- 4    %c -- %c\n", board[3], board[4]);
  //printf("             %c -- %c\n", board[5], board[6]);
  printf("\n");
  printf("It is %s's turn (%c).\n", playerName, playerPiece[generic_hash_turn(position)]);
  printf("\n");
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
MOVELIST *GenerateMoves(POSITION position)
{
  char board[BOARD_SIZE];
  generic_hash_unhash(position, board);
  int player = generic_hash_turn(position);
  MOVELIST *moves = NULL;

  int piecesOnBoard = 0;
  for (int i = 0; i < 7; i++) {
    if (board[i] == playerPiece[1] || board[i] == playerPiece[2]) {
      piecesOnBoard++;
    }
  }

  if (piecesOnBoard < 6 && player == 1)
  {
    for (int i = 0; i < 5; i++) {
      if (board[i] == ' ') {  
        moves = CreateMovelistNode(ENCODE_MOVE(5, i), moves); 
      }
  }
  }
  else if (piecesOnBoard < 6 && player == 2)
  {
    for (int i = 0; i < 5; i++) {
      if (board[i] == ' ') {  
        moves = CreateMovelistNode(ENCODE_MOVE(6, i), moves); 
      }
  }
  }
  
  else {
    for (int i = 0; i < 5; i++) {
    if (playerPiece[player] == board[i]) {
      if (i == 0) {
          if (board[2] == ' ') {
            moves = CreateMovelistNode(ENCODE_MOVE(i, 2), moves);
          }
          if (board[4] == ' ') {
            moves = CreateMovelistNode(ENCODE_MOVE(i, 4), moves);
          }
        }
      else if (i == 1)
      {
        if (board[2] == ' ') {
            moves = CreateMovelistNode(ENCODE_MOVE(i, 2), moves);
          }
          if (board[4] == ' ') {
            moves = CreateMovelistNode(ENCODE_MOVE(i, 4), moves);
          }
      }
      else if (i == 3)
      {
        if (board[2] == ' ') {
            moves = CreateMovelistNode(ENCODE_MOVE(i, 2), moves);
          }
          if (board[4] == ' ') {
            moves = CreateMovelistNode(ENCODE_MOVE(i, 4), moves);
          }
      }
      else {
          int next = (i + 1) % 4;
          int prev = (i + (4 - 1)) % 4;
          int center = 0;
          if (board[next] == ' ') {
            moves = CreateMovelistNode(ENCODE_MOVE(i, next), moves);
          } else if (board[prev] == ' ') {
            moves = CreateMovelistNode(ENCODE_MOVE(i, prev), moves);
          } else if (board[center] == ' ' && (i == 4 || i == 2)) {
            moves = CreateMovelistNode(ENCODE_MOVE(i, center), moves);
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
  USERINPUT ret = Continue;
  do {
    printf("%s's move [(u)ndo/0-4,0-4] :  ", playerName);

    ret = HandleDefaultTextInput(thePosition, theMove, playerName);
  } while (ret == Continue);
  return ret;
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
  int i = 0;
  while (input[i] && isdigit(input[i])) {
    i++;
  }
  while (input[i] == ' ') {
    i++;
  }
  if (input[i] != ',') {
    return FALSE;
  } else {
    i++;
  }
  while (input[i] == ' ') {
    i++;
  }
  while (input[i] && isdigit(input[i])) {
    i++;
  }
  return !input[i];
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

MOVE ConvertTextInputToMove(STRING input)
{
  char * c = input;
  long start = strtol(c, &c, 10);
  while (*c == ' ' || *c == ',') {
    c++;
  }
  long end = strtol(c, &c, 10);
  return ENCODE_MOVE(start, end);
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
  int start = DECODE_MOVE_START(move);
  int end = DECODE_MOVE_END(move);
  printf("%d,%d", start, end);
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
  snprintf(moveStringBuffer, MAX_MOVE_STRING_SIZE, "%u,%u", DECODE_MOVE_START(move), DECODE_MOVE_END(move));
}

CONST_STRING kDBName = "dorvoncag";

int NumberOfOptions()
{
  return 1;
}

int getOption()
{
  return 0;
}

void setOption(int option)
{
  (void)option;
}

POSITION ActualNumberOfPositions(int variant)
{
  (void)variant;
  return 0;
}


POSITION StringToPosition(char *positionString) {
	(void) positionString;
	return NULL_POSITION;
}

void PositionToAutoGUIString(POSITION position, char *autoguiPositionStringBuffer) {
	(void) position;
	(void) autoguiPositionStringBuffer;
}

void MoveToAutoGUIString(POSITION position, MOVE move, char *autoguiMoveStringBuffer) {
	(void) position;
	(void) move;
	(void) autoguiMoveStringBuffer;
}
