/************************************************************************
**
** NAME:        msantorini.c
**
** DESCRIPTION: Santorini
**
** AUTHOR:      Zachary Leete
**
** DATE:        2023-03-07
**
************************************************************************/

#include <stdio.h>
#include "gamesman.h"

/* IMPORTANT GLOBAL VARIABLES */
STRING kAuthorName = "Zachary Leete";
POSITION gNumberOfPositions = 0; // TODO: Put your number of positions upper bound here.
POSITION gInitialPosition = 0; // TODO: Put the hash value of the initial position.
BOOLEAN kPartizan = FALSE; // TODO: Is the game PARTIZAN i.e. given a board does each player have a different set of moves available to them?
BOOLEAN kTieIsPossible = FALSE; // TODO: Is a tie or draw possible?
BOOLEAN kLoopy = FALSE; // TODO: Is this game loopy?
BOOLEAN kSupportsSymmetries = FALSE; // TODO: Whether symmetries are supported (i.e. whether the GetCanonicalPosition is implemented)

/* Do not change these. */
POSITION GetCanonicalPosition(POSITION);
STRING MoveToString(MOVE);
POSITION kBadPosition = -1;
STRING kGameName = "Santorini";
STRING kDBName = "santorini";
BOOLEAN kDebugDetermineValue = FALSE;
void* gGameSpecificTclInit = NULL;

/* You don't have to change these for now. */
BOOLEAN kGameSpecificMenu = FALSE;
BOOLEAN kDebugMenu = FALSE;

/* These variables are not needed for solving but if you have time 
after you're done solving the game you should initialize them 
with something helpful. */
STRING kHelpGraphicInterface = "";
STRING kHelpTextInterface = "";
STRING kHelpOnYourTurn = "";
STRING kHelpStandardObjective = "";
STRING kHelpReverseObjective = "";
STRING kHelpTieOccursWhen = /* Should follow 'A Tie occurs when... */ "";
STRING kHelpExample = "";

/* You don't have to change this. */
void DebugMenu() {}
/* Ignore this function. */
void SetTclCGameSpecificOptions(int theOptions[]) {}
/* Do not worry about this yet because you will only be supporting 1 variant for now. */
void GameSpecificMenu() {}






/*********** BEGIN SOLVING FUNCIONS ***********/

/* TODO: Add a hashing function and unhashing function, if needed. */

/* Initialize any global variables or data structures needed before
solving or playing the game. */
void InitializeGame() {
  gCanonicalPosition = GetCanonicalPosition;
  gMoveToStringFunPtr = &MoveToString;

  /* YOUR CODE HERE */
  
}

/* Return the hash value of the initial position. */
POSITION GetInitialPosition() {
  /* YOUR CODE HERE */
  return 0;
}

/* Return a linked list of moves. */
MOVELIST *GenerateMoves(POSITION position) {
  MOVELIST *moves = NULL;
  /* YOUR CODE HERE 
     
     To add to the linked list, do 
     moves = CreateMovelistNode(<the move you're adding>, moves);
     See the function CreateMovelistNode in src/core/misc.c
  */
  return moves;
}

/* Return the position that results from making the 
input move on the input position. */
POSITION DoMove(POSITION position, MOVE move) {
  /* YOUR CODE HERE */
  return 0;
}

/* Return lose, win, tie, or undecided. See src/core/types.h
for the value enum definition. */
VALUE Primitive(POSITION position) {
  /* YOUR CODE HERE */
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
}

void PrintComputersMove(MOVE computersMove, STRING computersName) {
  /* YOUR CODE HERE */
}

USERINPUT GetAndPrintPlayersMove(POSITION position, MOVE *move, STRING playerName) {
  /* YOUR CODE HERE */
  return Continue;
}

/* Return whether the input text signifies a valid move. */
BOOLEAN ValidTextInput(STRING input) {
  /* YOUR CODE HERE */
  return TRUE;
}

/* Assume the text input signifies a valid move. Return
the move hash corresponding to the move. */
MOVE ConvertTextInputToMove(STRING input) {
  /* YOUR CODE HERE */
  return 0;
}

/* Return the string representation of the move. 
Ideally this matches with what the user is supposed to
type in. */
STRING MoveToString(MOVE move) {
  /* YOUR CODE HERE */
  return NULL;
}

/* Basically just print the move. */
void PrintMove(MOVE move) {
  /* YOUR CODE HERE */
}

/*********** END TEXTUI FUNCTIONS ***********/









/*********** BEGIN VARIANT FUNCTIONS ***********/

/* How many variants are you supporting? For now, just 1.
Maybe in the future you want to support more variants. */
int NumberOfOptions() {
  /* YOUR CODE HERE MAYBE LATER BUT NOT NOW */
  return 1;
}

/* Return the current variant id (which is 0 in this case since
for now you're only thinking about one variant). */
int getOption() {
  /* YOUR CODE HERE MAYBE LATER BUT NOT NOW */
  return 0;
}

/* The input is a variant id. This function sets any global variables
or data structures according to the variant specified by the variant id. 
But for now you have one variant so don't worry about this. */
void setOption(int option) {
  /* YOUR CODE HERE MAYBE LATER BUT NOT NOW */
}

/*********** END VARIANT-RELATED FUNCTIONS ***********/







/* Don't worry about these Interact functions below yet.
They are used for the AutoGUI which eventually we would
want to implement, but they are not needed for solving. */
POSITION InteractStringToPosition(STRING board) {
  /* YOUR CODE HERE LATER BUT NOT NOW */
  return 0;
}

STRING InteractPositionToString(POSITION position) {
  /* YOUR CODE HERE LATER BUT NOT NOW */
  return NULL;
}

/* Ignore this function. */
STRING InteractPositionToEndData(POSITION position) {
  return NULL;
}

STRING InteractMoveToString(POSITION position, MOVE move) {
  /* YOUR CODE HERE LATER BUT NOT NOW */
  return MoveToString(move);
}


#define height 3
#define width 3

int startState[height][width][2];

void FindLocation(int position[height][width][2], int activePlayer, int *x, int *y){
    for (*x = 0; *x < height; (*x)++){
        for (*y = 0; *y < width; (*y)++){
            if (position[*x][*y][1] == activePlayer){
                return;
            }
        }
    }
}

int FindActivePlayer(int position[height][width][2]){
    int sum = 0;
    for (int x = 0; x < height; x++){
        for (int y = 0; y < width; y++){
            sum += position[x][y][0];
        }
    }
    if (sum % 2 == 0){
        return 1;
    }
    return 2;
}

void DoMove(int position[height][width][2], int *move, int newPosition[height][width][2]){
    int activePlayer = FindActivePlayer(position);
    int activePlayerLocationX, activePlayerLocationY;
    FindLocation(position, activePlayer, &activePlayerLocationX, &activePlayerLocationY);
    for (int x = 0; x < height; x++){
        for (int y = 0; y < width; y++){
            newPosition[x][y][0] = position[x][y][0];
            newPosition[x][y][1] = position[x][y][1];
        }
    }
    newPosition[move[0]][move[1]][1] = activePlayer;
    newPosition[activePlayerLocationX][activePlayerLocationY][1] = 0;
    newPosition[move[2]][move[3]][0] += 1;
}

void MoveDirections(int position[height][width][2], int availableMoves[8][2]) {
    int activePlayer = FindActivePlayer(position);
    int location[2];
    FindLocation(position, activePlayer, &location[0], &location[1]);
    //sides
    if (location[0]+1 < height && position[location[0]+1][location[1]][1] == 0 && position[location[0]+1][location[1]][0] < 4
    && (position[location[0]+1][location[1]][0] - position[location[0]][location[1]][0]) <= 1) { //bottom middle
        availableMoves[0][0] = +1;
        availableMoves[0][1] = 0;
    }
    if (location[0]-1 > -1 && position[location[0]-1][location[1]][1] == 0 && position[location[0]-1][location[1]][0] < 4
    && (position[location[0]-1][location[1]][0] - position[location[0]][location[1]][0]) <= 1) { //top middle
        availableMoves[1][0] = -1;
        availableMoves[1][1] = 0;
    }
    if (location[1]+1 < width && position[location[0]][location[1]+1][1] == 0 && position[location[0]][location[1]+1][0] < 4
    && (position[location[0]][location[1]+1][0] - position[location[0]][location[1]][0]) <= 1) { //right middle
        availableMoves[2][0] = 0;
        availableMoves[2][1] = +1;
    }
    if (location[1]-1 > -1 && position[location[0]][location[1]-1][1] == 0 && position[location[0]][location[1]-1][0] < 4
    && (position[location[0]][location[1]-1][0] - position[location[0]][location[1]][0]) <= 1) { //left middle
        availableMoves[3][0] = 0;
        availableMoves[3][1] = -1;
    }
    //corners
    if (location[0]+1 < height && location[1]+1 < width && position[location[0]+1][location[1]+1][1] == 0 && position[location[0]+1][location[1]+1][0] < 4
    && (position[location[0]+1][location[1]+1][0] - position[location[0]][location[1]][0]) <= 1) { //bottom right
        availableMoves[4][0] = +1;
        availableMoves[4][1] = +1;
    }
    if (location[0]-1 > -1 && location[1]-1 > -1 && position[location[0]-1][location[1]-1][1] == 0 && position[location[0]-1][location[1]-1][0] < 4
    && (position[location[0]-1][location[1]-1][0] - position[location[0]][location[1]][0]) <= 1) { //top left
        availableMoves[5][0] = -1;
        availableMoves[5][1] = -1;
    }
    if (location[0]+1 < height && location[1]-1 > -1 && position[location[0]+1][location[1]-1][1] == 0 && position[location[0]+1][location[1]-1][0] < 4
    && (position[location[0]+1][location[1]-1][0] - position[location[0]][location[1]][0]) <= 1) { //bottom left
        availableMoves[6][0] = +1;
        availableMoves[6][1] = -1;
    }
    if (location[0]-1 > -1 && location[1]+1 < width && position[location[0]-1][location[1]+1][1] == 0 && position[location[0]-1][location[1]+1][0] < 4
    && (position[location[0]-1][location[1]+1][0] - position[location[0]][location[1]][0]) <= 1) { //top right
        availableMoves[7][0] = -1;
        availableMoves[7][1] = +1;
    }
    return;
}

void GenerateMoves(int position[height][width][2], int availableMoves[8][2], int builds[8][8]) {
    int activePlayer = FindActivePlayer(position);
    int location[2];
    FindLocation(position, activePlayer, &location[0], &location[1]);
    int** moves = MoveDirections(position);
    int count = 0;
    for (int i = 0; i < 8; i++) {
        int newRow = location[0] + moves[i][0];
        int newCol = location[1] + moves[i][1];
        if (newRow < 0 || newRow >= height || newCol < 0 || newCol >= width) {
            continue;
        }
        if (position[newRow][newCol][1] != 0) {
            continue;
        }
        int*** newPosition = CopyPosition(position);
        newPosition[newRow][newCol][1] = activePlayer;
        builds[count] = MoveDirections(newPosition);
        count++;
        FreePosition(newPosition);
    }
    FreeMoves(moves);
    return;
}

int PrimitiveValue(int*** position) {
    int activePlayer = FindActivePlayer(position);
    int location[2];
    FindLocation(position, activePlayer, &location[0], &location[1]);
    int value = 0;
    //check if current player is on the opponent's baseline
    if ((activePlayer == 1 && location[0] == 0) || (activePlayer == 2 && location[0] == height-1)) {
        value = "WIN";
    }
    //check if opponent is on the current player's baseline
    else if ((activePlayer == 1 && location[0] == height-1) || (activePlayer == 2 && location[0] == 0)) {
        value = "LOSS";
    }
    return value;
}

int WonCheck(int*** position) {
    int activePlayer = FindActivePlayer(position);
    int location[2];
    FindLocation(position, activePlayer, &location[0], &location[1]);
    //check if current player is on the opponent's baseline
    if ((activePlayer == 1 && location[0] == 0) || (activePlayer == 2 && location[0] == height-1)) {
        return 1;
    }
    //check if opponent is on the current player's baseline
    else if ((activePlayer == 1 && location[0] == height-1) || (activePlayer == 2 && location[0] == 0)) {
        return -1;
    }
    return 0;
}