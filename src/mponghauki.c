#include <stdio.h>
#include "gamesman.h"

#define BOARD_SIZE 5
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

STRING kAuthorName = "Kyle Zentner";
STRING kGameName = "Pong Hau K'i";
BOOLEAN kPartizan = TRUE;
BOOLEAN kDebugMenu = TRUE;
BOOLEAN kGameSpecificMenu = FALSE;
BOOLEAN kTieIsPossible = TRUE;
BOOLEAN kLoopy = TRUE;
BOOLEAN kDebugDetermineValue = FALSE;
void* gGameSpecificTclInit = NULL;

STRING kHelpGraphicInterface = "";

STRING kHelpTextInterface = "";

STRING kHelpOnYourTurn = "Please enter your move in the format 0-4,0-4";

STRING kHelpStandardObjective = "Slide your pieces along the lines to prevent your opponent from moving.";

STRING kHelpReverseObjective = "";

STRING kHelpTieOccursWhen = /* Should follow 'A Tie occurs when... */ "";

STRING kHelpExample = "";

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
  int hash_data[] = {' ', 1, 1, 'X', 2, 2, 'O', 2, 2, -1};
  gNumberOfPositions = generic_hash_init(BOARD_SIZE, hash_data, NULL, 0);
  char start[] = " XOXO";
  gInitialPosition = generic_hash_hash(start, 1);
}

void FreeGame()
{
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
  assert(board[end] == ' ');
  assert(board[start] != ' ');
  board[end] = board[start];
  board[start] = ' ';
  POSITION out = generic_hash_hash(board, NEXT_PLAYER(player));
  return out;
}

void UndoMove(MOVE move)
{
  assert(0);
}

/************************************************************************
**
** NAME: GetInitialPosition
**
** DESCRIPTION: Ask the user for an initial position for testing. Store
** it in the space pointed to by initialPosition;
**
** OUTPUTS: POSITION initialPosition : The position to fill.
**
************************************************************************/

POSITION GetInitialPosition()
{
  return gInitialPosition;
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

void PrintComputersMove(computersMove,computersName)
MOVE computersMove;
STRING computersName;
{
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
  char board[BOARD_SIZE];
  generic_hash_unhash(position, board);
  printf("\n");
  printf("    3 -- 4    %c -- %c\n", board[3], board[4]);
  printf("    | \\ /     | \\ /\n");
  printf("    |  0      |  %c\n", board[0]);
  printf("    | / \\     | / \\\n");
  printf("    2 -- 1    %c -- %c\n", board[2], board[1]);
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
  for (int i = 0; i < BOARD_SIZE; i++) {
    if (playerPiece[player] == board[i]) {
      if (i == 0) {
        for (int j = 1; j < BOARD_SIZE; j++) {
          if (board[j] == ' ') {
            moves = CreateMovelistNode(ENCODE_MOVE(i, j), moves);
          }
        }
      } else {
        int next = (i + 1) % BOARD_SIZE;
        int prev = (i + (BOARD_SIZE - 1)) % BOARD_SIZE;
        int center = 0;
        if (board[next] == ' ') {
          moves = CreateMovelistNode(ENCODE_MOVE(i, next), moves);
        } else if (board[prev] == ' ') {
          moves = CreateMovelistNode(ENCODE_MOVE(i, prev), moves);
        } else if (board[center] == ' ') {
          moves = CreateMovelistNode(ENCODE_MOVE(i, center), moves);
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

POSITION DoSymmetry(POSITION position, int symmetry)
{
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

USERINPUT GetAndPrintPlayersMove(thePosition, theMove, playerName)
POSITION thePosition;
MOVE *theMove;
STRING playerName;
{
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

STRING MoveToString (MOVE move)
{
  char * c = malloc(MAX_MOVE_STRING_SIZE);
  snprintf(c, MAX_MOVE_STRING_SIZE, "%u,%u", DECODE_MOVE_START(move), DECODE_MOVE_END(move));
  return c;
}

STRING kDBName = "ponghauki";

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
}

POSITION ActualNumberOfPositions(int variant)
{
  return 0;
}


POSITION StringToPosition(STRING board)
{
  int i, turn;
  char board_char[BOARD_SIZE];
  for(i = 0; i < BOARD_SIZE; i++){
    board_char[i] = board[i];
  }

  int success = GetValue(board, "turn", GetInt, &turn);
  if (success) {
    return generic_hash_hash(board_char, turn);
  } else {
    return INVALID_POSITION;
  }
}

STRING PositionToString(POSITION pos)
{
  char board[BOARD_SIZE + 1];
  int current_player = generic_hash_turn(pos);
  generic_hash_unhash(pos, board);
  return MakeBoardString(board, "turn",  StrFromI(current_player), "");
}

char * PositionToEndData(POSITION pos)
{
  return NULL;
}
