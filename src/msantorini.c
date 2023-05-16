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
#include <stdlib.h>
#include "gamesman.h"

/* IMPORTANT GLOBAL VARIABLES */
STRING kAuthorName = "Zachary Leete";
POSITION gNumberOfPositions = 17580000000000; // TODO: Put your number of positions upper bound here.
POSITION gInitialPosition;                    // TODO: Put the hash value of the initial position.
BOOLEAN kPartizan = TRUE;                     // TODO: Is the game PARTIZAN i.e. given a board does each player have a different set of moves available to them?
BOOLEAN kTieIsPossible = FALSE;               // TODO: Is a tie or draw possible?
BOOLEAN kLoopy = FALSE;                       // TODO: Is this game loopy?
BOOLEAN kSupportsSymmetries = TRUE;          // TODO: Whether symmetries are supported (i.e. whether the GetCanonicalPosition is implemented)

/* Do not change these. */
POSITION GetCanonicalPosition(POSITION);
STRING MoveToString(MOVE);
POSITION kBadPosition = -1;
STRING kGameName = "Santorini";
STRING kDBName = "santorini";
BOOLEAN kDebugDetermineValue = FALSE;
void *gGameSpecificTclInit = NULL;

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

#define height 4
#define width 4
#define size 16
POSITION encodePosition(int[height][width][2]);
void decodePosition(POSITION, int[height][width][2]);
MOVE encodeMove(int, int, int, int);
void decodeMove(MOVE, int[4]);
int FindActivePlayer(int[height][width][2]);
void FindLocation(int[height][width][2], int, int *, int *);
TIERLIST *GetTierChildren(TIER);
TIERPOSITION NumberOfPositionsPerTier(TIER);
void getSymmetricPawnPositions(int *, int *, int);

// debug functions
void printUnhashedPosition(int position[height][width][2])
{
  for (int i = 0; i < height; i++)
  {
    for (int j = 0; j < width; j++)
    {
      printf("[%d, ", position[i][j][0]);
      printf("%d] ", position[i][j][1]);
    }
    printf("\n");
  }
  printf("\n\n");
}

void printUnhashedMove(int move[4])
{
  printf("Move: x: %d, y: %d, buildX: %d, buildY: %d\n", move[0], move[1], move[2], move[3]);
}

void printMoveList(MOVELIST *moves)
{
  while (moves != NULL)
  {
    int decodedMove[4];
    decodeMove(moves->move, decodedMove);
    printUnhashedMove(decodedMove);
    moves = moves->next;
  }
}

/*********** BEGIN SOLVING FUNCIONS ***********/

/* TODO: Add a hashing function and unhashing function, if needed. */

/* Initialize any global variables or data structures needed before
solving or playing the game. */
void InitializeGame()
{
  gCanonicalPosition = GetCanonicalPosition;
  gMoveToStringFunPtr = &MoveToString;
  int startState[height][width][2];
  for (int i = 0; i < height; i++)
  {
    for (int j = 0; j < width; j++)
    {
      startState[i][j][0] = 0;
      startState[i][j][1] = 0;
    }
  }
  startState[0][0][1] = 1;
  startState[height - 1][width - 1][1] = 2;
  gInitialPosition = encodePosition(startState);
  printf("hashed start state: %llu\n", gInitialPosition);
  gTierChildrenFunPtr = &GetTierChildren;
  gNumberOfTierPositionsFunPtr = &NumberOfPositionsPerTier;
  kSupportsTierGamesman = TRUE;
  gInitialTier = 0;
  gInitialTierPosition = gInitialPosition;
}

/* Return the hash value of the initial position. */
POSITION GetInitialPosition()
{
  /* YOUR CODE HERE */
  return gInitialPosition;
}

void MoveDirections(int position[height][width][2], int availableDirections[8][2])
{
  int activePlayer = FindActivePlayer(position);
  int location[2];
  FindLocation(position, activePlayer, &location[0], &location[1]);
  // sides
  if (location[0] + 1 < height && position[location[0] + 1][location[1]][1] == 0 && position[location[0] + 1][location[1]][0] < 4 && (position[location[0] + 1][location[1]][0] - position[location[0]][location[1]][0]) <= 1)
  { // bottom middle
    availableDirections[0][0] = +1;
    availableDirections[0][1] = 0;
  }
  else
  {
    availableDirections[0][0] = -5;
    availableDirections[0][1] = -5;
  }
  if (location[0] - 1 > -1 && position[location[0] - 1][location[1]][1] == 0 && position[location[0] - 1][location[1]][0] < 4 && (position[location[0] - 1][location[1]][0] - position[location[0]][location[1]][0]) <= 1)
  { // top middle
    availableDirections[1][0] = -1;
    availableDirections[1][1] = 0;
  }
  else
  {
    availableDirections[1][0] = -5;
    availableDirections[1][1] = -5;
  }
  if (location[1] + 1 < width && position[location[0]][location[1] + 1][1] == 0 && position[location[0]][location[1] + 1][0] < 4 && (position[location[0]][location[1] + 1][0] - position[location[0]][location[1]][0]) <= 1)
  { // right middle
    availableDirections[2][0] = 0;
    availableDirections[2][1] = +1;
  }
  else
  {
    availableDirections[2][0] = -5;
    availableDirections[2][1] = -5;
  }
  if (location[1] - 1 > -1 && position[location[0]][location[1] - 1][1] == 0 && position[location[0]][location[1] - 1][0] < 4 && (position[location[0]][location[1] - 1][0] - position[location[0]][location[1]][0]) <= 1)
  { // left middle
    availableDirections[3][0] = 0;
    availableDirections[3][1] = -1;
  }
  else
  {
    availableDirections[3][0] = -5;
    availableDirections[3][1] = -5;
  }
  // corners
  if (location[0] + 1 < height && location[1] + 1 < width && position[location[0] + 1][location[1] + 1][1] == 0 && position[location[0] + 1][location[1] + 1][0] < 4 && (position[location[0] + 1][location[1] + 1][0] - position[location[0]][location[1]][0]) <= 1)
  { // bottom right
    availableDirections[4][0] = +1;
    availableDirections[4][1] = +1;
  }
  else
  {
    availableDirections[4][0] = -5;
    availableDirections[4][1] = -5;
  }
  if (location[0] - 1 > -1 && location[1] - 1 > -1 && position[location[0] - 1][location[1] - 1][1] == 0 && position[location[0] - 1][location[1] - 1][0] < 4 && (position[location[0] - 1][location[1] - 1][0] - position[location[0]][location[1]][0]) <= 1)
  { // top left
    availableDirections[5][0] = -1;
    availableDirections[5][1] = -1;
  }
  else
  {
    availableDirections[5][0] = -5;
    availableDirections[5][1] = -5;
  }
  if (location[0] + 1 < height && location[1] - 1 > -1 && position[location[0] + 1][location[1] - 1][1] == 0 && position[location[0] + 1][location[1] - 1][0] < 4 && (position[location[0] + 1][location[1] - 1][0] - position[location[0]][location[1]][0]) <= 1)
  { // bottom left
    availableDirections[6][0] = +1;
    availableDirections[6][1] = -1;
  }
  else
  {
    availableDirections[6][0] = -5;
    availableDirections[6][1] = -5;
  }
  if (location[0] - 1 > -1 && location[1] + 1 < width && position[location[0] - 1][location[1] + 1][1] == 0 && position[location[0] - 1][location[1] + 1][0] < 4 && (position[location[0] - 1][location[1] + 1][0] - position[location[0]][location[1]][0]) <= 1)
  { // top right
    availableDirections[7][0] = -1;
    availableDirections[7][1] = +1;
  }
  else
  {
    availableDirections[7][0] = -5;
    availableDirections[7][1] = -5;
  }
  return;
}

/* Return a linked list of moves. */
MOVELIST *GenerateMoves(POSITION encodedPosition)
{
  printf("generating moves\n");
  MOVELIST *moves = NULL;
  MOVE theMove;
  int availableDirections[8][2];
  int position[height][width][2];
  decodePosition(encodedPosition, position);
  MoveDirections(position, availableDirections);
  int activePlayer = FindActivePlayer(position);
  int location[2];
  FindLocation(position, activePlayer, &location[0], &location[1]);
  for (int i = 0; i < 8; i++)
  {
    if (availableDirections[i][0] == -5)
    {
      continue;
    }
    int newLocation[2];
    newLocation[0] = location[0] + availableDirections[i][0];
    newLocation[1] = location[1] + availableDirections[i][1];
    // Initialize movedPosition as a copy of position
    int movedPosition[height][width][2];
    for (int j = 0; j < height; j++)
    {
      for (int k = 0; k < width; k++)
      {
        movedPosition[j][k][0] = position[j][k][0];
        movedPosition[j][k][1] = position[j][k][1];
      }
    }
    // Update movedPosition based on the move and activePlayer
    movedPosition[newLocation[0]][newLocation[1]][1] = activePlayer;
    movedPosition[location[0]][location[1]][1] = 0;
    if (newLocation[0] + 1 < height && movedPosition[newLocation[0] + 1][newLocation[1]][1] == 0 && movedPosition[newLocation[0] + 1][newLocation[1]][0] < 4)
    { // bottom middle
      theMove = encodeMove(newLocation[0], newLocation[1], newLocation[0] + 1, newLocation[1]);
      moves = CreateMovelistNode(theMove, moves);
    }
    if (newLocation[0] - 1 > -1 && movedPosition[newLocation[0] - 1][newLocation[1]][1] == 0 && movedPosition[newLocation[0] - 1][newLocation[1]][0] < 4)
    { // top middle
      theMove = encodeMove(newLocation[0], newLocation[1], newLocation[0] - 1, newLocation[1]);
      moves = CreateMovelistNode(theMove, moves);
    }
    if (newLocation[1] + 1 < width && movedPosition[newLocation[0]][newLocation[1] + 1][1] == 0 && movedPosition[newLocation[0]][newLocation[1] + 1][0] < 4)
    { // right middle
      theMove = encodeMove(newLocation[0], newLocation[1], newLocation[0], newLocation[1] + 1);
      moves = CreateMovelistNode(theMove, moves);
    }
    if (newLocation[1] - 1 > -1 && movedPosition[newLocation[0]][newLocation[1] - 1][1] == 0 && movedPosition[newLocation[0]][newLocation[1] - 1][0] < 4)
    { // left middle
      theMove = encodeMove(newLocation[0], newLocation[1], newLocation[0], newLocation[1] - 1);
      moves = CreateMovelistNode(theMove, moves);
    }
    if (newLocation[0] + 1 < height && newLocation[1] + 1 < width && movedPosition[newLocation[0] + 1][newLocation[1] + 1][1] == 0 && movedPosition[newLocation[0] + 1][newLocation[1] + 1][0] < 4)
    { // bottom right
      theMove = encodeMove(newLocation[0], newLocation[1], newLocation[0] + 1, newLocation[1] + 1);
      moves = CreateMovelistNode(theMove, moves);
    }
    if (newLocation[0] - 1 > -1 && newLocation[1] + 1 < width && movedPosition[newLocation[0] - 1][newLocation[1] + 1][1] == 0 && movedPosition[newLocation[0] - 1][newLocation[1] + 1][0] < 4)
    { // top right
      theMove = encodeMove(newLocation[0], newLocation[1], newLocation[0] - 1, newLocation[1] + 1);
      moves = CreateMovelistNode(theMove, moves);
    }
    if (newLocation[0] + 1 < height && newLocation[1] - 1 > -1 && movedPosition[newLocation[0] + 1][newLocation[1] - 1][1] == 0 && movedPosition[newLocation[0] + 1][newLocation[1] - 1][0] < 4)
    { // bottom left
      theMove = encodeMove(newLocation[0], newLocation[1], newLocation[0] + 1, newLocation[1] - 1);
      moves = CreateMovelistNode(theMove, moves);
    }
    if (newLocation[0] - 1 > -1 && newLocation[1] - 1 > -1 && movedPosition[newLocation[0] - 1][newLocation[1] - 1][1] == 0 && movedPosition[newLocation[0] - 1][newLocation[1] - 1][0] < 4)
    { // top left
      theMove = encodeMove(newLocation[0], newLocation[1], newLocation[0] - 1, newLocation[1] - 1);
      moves = CreateMovelistNode(theMove, moves);
    }
  }
  printf("generated moves:\n");
  //printMoveList(moves);
  return moves;
}

/* Return the position that results from making the
input move on the input position. */
POSITION DoMove(POSITION encodedPosition, MOVE encodedMove)
{
  int position[height][width][2];
  decodePosition(encodedPosition, position);
  printf("position before doing move: \n");
  printUnhashedPosition(position);
  printf("move passed into domove: %d\n", encodedMove);
  int move[4];
  decodeMove(encodedMove, move);
  printf("move: ");
  printUnhashedMove(move);
  int activePlayer = FindActivePlayer(position);
  int activePlayerLocationX, activePlayerLocationY;
  FindLocation(position, activePlayer, &activePlayerLocationX, &activePlayerLocationY);
  position[move[0]][move[1]][1] = activePlayer;
  position[activePlayerLocationX][activePlayerLocationY][1] = 0;
  position[move[2]][move[3]][0] += 1;
  printf("position after doing move: \n");
  printUnhashedPosition(position);
  POSITION hashedPosition = encodePosition(position);
  printf("hashed position after doing move: %llu\n", hashedPosition);
  return hashedPosition;
}

/* Return lose, win, tie, or undecided. See src/core/types.h
for the value enum definition. */
VALUE Primitive(POSITION position)
{
  printf("primitive trigger\n");
  printf("hashed position: %llu\n", position);
  int unhashedPosition[height][width][2];
  decodePosition(position, unhashedPosition);
  printf("unhashedPosition in primitive check: \n");
  printUnhashedPosition(unhashedPosition);
  int activePlayer = FindActivePlayer(unhashedPosition);
  int location[2];
  FindLocation(unhashedPosition, 3 - activePlayer, &location[0], &location[1]);
  if ((unhashedPosition[location[0]][location[1]][0] == 3 || GenerateMoves(position) == NULL))
  {
    return lose;
  }
  return undecided;
}

/* Symmetry Handling: Return the canonical position. */
POSITION GetCanonicalPosition(POSITION position)
{
  int unhashedPosition[height][width][2];
  decodePosition(position, unhashedPosition);
  int P1location[2];
  FindLocation(unhashedPosition, 1, &P1location[0], &P1location[1]);
  int P2location[2];
  FindLocation(unhashedPosition, 2, &P2location[0], &P2location[1]);
  int lowestCanonicalPosition[height][width][2];
  for (int i = 0; i < height; i++)
  {
    for (int j = 0; j < width; j++)
    {
      lowestCanonicalPosition[i][j][0] = unhashedPosition[i][j][0];
      lowestCanonicalPosition[i][j][1] = unhashedPosition[i][j][1];
    }
  }
  int lowestCanonicalSymmetry = 0;
  int minCanonicalValue = INT_MAX;

  for (int i = 0; i <= 7; i++)
  {
    int canonicalValue = 0;
    int P1Symmetric[2], P2Symmetric[2];

    // Get symmetric pawn positions
    P1Symmetric[0] = P1location[0];
    P1Symmetric[1] = P1location[1];
    P2Symmetric[0] = P2location[0];
    P2Symmetric[1] = P2location[1];
    getSymmetricPawnPositions(P1Symmetric, P2Symmetric, i);

    // Calculate canonical value using symmetric pawn positions
    canonicalValue = ((P1Symmetric[0] * width + P1Symmetric[1]) * size + (P2Symmetric[0] * width + P2Symmetric[1]));

    // Update minimum canonical value
    if (canonicalValue < minCanonicalValue)
    {
      minCanonicalValue = canonicalValue;
      lowestCanonicalSymmetry = i;
    }
  }

  // Create symmetric copies based on lowestCanonicalSymmetry
  if (lowestCanonicalSymmetry == 1)
  {
    // Rotate board state by 90 degrees and create second symmetric copy
    for (int i = 0; i < width; i++)
    {
      for (int j = 0; j < height; j++)
      {
        lowestCanonicalPosition[i][j][0] = unhashedPosition[height - 1 - j][i][0];
        lowestCanonicalPosition[i][j][1] = unhashedPosition[height - 1 - j][i][1];
      }
    }
  }
  else if (lowestCanonicalSymmetry == 2)
  {
    // Rotate board state by 180 degrees and create third symmetric copy
    for (int i = 0; i < height; i++)
    {
      for (int j = 0; j < width; j++)
      {
        lowestCanonicalPosition[i][j][0] = unhashedPosition[height - 1 - i][width - 1 - j][0];
        lowestCanonicalPosition[i][j][1] = unhashedPosition[height - 1 - i][width - 1 - j][1];
      }
    }
  }
  else if (lowestCanonicalSymmetry == 3)
  {
    // Rotate board state by 270 degrees and create fourth symmetric copy
    for (int i = 0; i < width; i++)
    {
      for (int j = 0; j < height; j++)
      {
        lowestCanonicalPosition[i][j][0] = unhashedPosition[j][width - 1 - i][0];
        lowestCanonicalPosition[i][j][1] = unhashedPosition[j][width - 1 - i][1];
      }
    }
  }
  else if (lowestCanonicalSymmetry == 4)
  {
    // Flip board horizontally and create fifth symmetric copy
    for (int i = 0; i < height; i++)
    {
      for (int j = 0; j < width; j++)
      {
        lowestCanonicalPosition[i][j][0] = unhashedPosition[i][width - 1 - j][0];
        lowestCanonicalPosition[i][j][1] = unhashedPosition[i][width - 1 - j][1];
      }
    }
  }
  else if (lowestCanonicalSymmetry == 5)
  {
    // Flip board vertically and create sixth symmetric copy
    for (int i = 0; i < height; i++)
    {
      for (int j = 0; j < width; j++)
      {
        lowestCanonicalPosition[i][j][0] = unhashedPosition[height - 1 - i][j][0];
        lowestCanonicalPosition[i][j][1] = unhashedPosition[height - 1 - i][j][1];
      }
    }
  }
  else if (lowestCanonicalSymmetry == 6)
  {
    // Flip board diagonally and create seventh symmetric copy
    for (int i = 0; i < height; i++)
    {
      for (int j = 0; j < width; j++)
      {
        lowestCanonicalPosition[i][j][0] = unhashedPosition[j][i][0];
        lowestCanonicalPosition[i][j][1] = unhashedPosition[j][i][1];
      }
    }
  }
  else if (lowestCanonicalSymmetry == 7)
  {
    // Flip board anti-diagonally and create eighth symmetric copy
    for (int i = 0; i < height; i++)
    {
      for (int j = 0; j < width; j++)
      {
        lowestCanonicalPosition[i][j][0] = unhashedPosition[height - 1 - j][width - 1 - i][0];
        lowestCanonicalPosition[i][j][1] = unhashedPosition[height - 1 - j][width - 1 - i][1];
      }
    }
  }

  return encodePosition(lowestCanonicalPosition);
}

TIERLIST *GetTierChildren(TIER tier)
{
  TIERLIST *list = NULL;
  for (int i = 0; i < 4; i++)
  {
    if ((int)(floor(tier / pow(size, i))) % (int)(pow(size, i + 1)) > 1)
    {
      list = CreateTierlistNode(tier - pow(size, i) + pow(size, i + 1), list);
    }
  }
  return NULL;
}

TIERPOSITION NumberOfPositionsPerTier(TIER tier)
{
  // Not a generic function, only works for 4x4
  return pow(2, 56);
}
/*********** END SOLVING FUNCTIONS ***********/

/*********** START HELPER FUNCTIONS ***********/

void getSymmetricPawnPositions(int *pawn1, int *pawn2, int symmetry)
{
  int temp[2];

  switch (symmetry)
  {
  case 0:
    // Original board state, no transformation needed
    break;

  case 1:
    // 90 degrees clockwise
    temp[0] = pawn1[0];
    pawn1[0] = pawn1[1];
    pawn1[1] = width - temp[0] - 1;

    temp[0] = pawn2[0];
    pawn2[0] = pawn2[1];
    pawn2[1] = width - temp[0] - 1;
    break;

  case 2:
    // 180 degrees clockwise
    pawn1[0] = height - pawn1[0] - 1;
    pawn1[1] = width - pawn1[1] - 1;

    pawn2[0] = height - pawn2[0] - 1;
    pawn2[1] = width - pawn2[1] - 1;
    break;

  case 3:
    // 270 degrees clockwise
    temp[0] = pawn1[0];
    pawn1[0] = height - pawn1[1] - 1;
    pawn1[1] = temp[0];

    temp[0] = pawn2[0];
    pawn2[0] = height - pawn2[1] - 1;
    pawn2[1] = temp[0];
    break;

  case 4:
    // Horizontal reflection
    pawn1[1] = width - pawn1[1] - 1;
    pawn2[1] = width - pawn2[1] - 1;
    break;

  case 5:
    // Vertical reflection
    pawn1[0] = height - pawn1[0] - 1;
    pawn2[0] = height - pawn2[0] - 1;
    break;

  case 6:
    // Diagonal reflection
    temp[0] = pawn1[0];
    pawn1[0] = pawn1[1];
    pawn1[1] = temp[0];

    temp[0] = pawn2[0];
    pawn2[0] = pawn2[1];
    pawn2[1] = temp[0];
    break;

  case 7:
    // Anti-diagonal reflection
    temp[0] = pawn1[0];
    pawn1[0] = height - pawn1[1] - 1;
    pawn1[1] = width - temp[0] - 1;

    temp[0] = pawn2[0];
    pawn2[0] = height - pawn2[1] - 1;
    pawn2[1] = width - temp[0] - 1;
    break;
  }
}

void FindLocation(int position[height][width][2], int activePlayer, int *x, int *y)
{
  for (*x = 0; *x < height; (*x)++)
  {
    for (*y = 0; *y < width; (*y)++)
    {
      if (position[*x][*y][1] == activePlayer)
      {
        return;
      }
    }
  }
}

int FindActivePlayer(int position[height][width][2])
{
  int sum = 0;
  for (int x = 0; x < height; x++)
  {
    for (int y = 0; y < width; y++)
    {
      sum += position[x][y][0];
    }
  }
  if (sum % 2 == 0)
  {
    return 1;
  }
  return 2;
}

MOVE encodeMove(int x, int y, int buildx, int buildy)
{
  //printf("move being encoded:\n");
  int unhashedMove[4] = {x, y, buildx, buildy};
  //printUnhashedMove(unhashedMove);
  MOVE move = 0;
  move = move | (x << (int)((2 * ceil(log2(width))) + ceil(log2(height))));
  move = move | (y << (int)(ceil(log2(width)) + ceil(log2(height))));
  move = move | (buildx << (int)(ceil(log2(width))));
  move = move | buildy;
  //printf("encoded move: %d\n", move);
  return move;
}

void decodeMove(MOVE move, int decodedMove[4])
{
  printf("move being decoded: %d\n", move);
  decodedMove[0] = (move >> (int)((2 * ceil(log2(width))) + ceil(log2(height)))) & ((1 << (int)(ceil(log2(width)))) - 1);
  decodedMove[1] = (move >> (int)(ceil(log2(width)) + ceil(log2(height)))) & ((1 << (int)(ceil(log2(width)))) - 1);
  decodedMove[2] = (move >> (int)(ceil(log2(width)))) & ((1 << (int)(ceil(log2(width)))) - 1);
  decodedMove[3] = move & ((1 << (int)(ceil(log2(width)))) - 1);
  printf("decoded move:\n");
  printUnhashedMove(decodedMove);
  return;
}

TIER BoardToTier(int position[height][width][2])
{
  int buckets[5];
  for (int i = 0; i < 5; i++)
  {
    buckets[i] = 0;
  }
  for (int i = 0; i < height; i++)
  {
    for (int j = 0; j < width; j++)
    {
      buckets[position[i][j][0]] += 1;
    }
  }
  TIER tier = 0UL;
  for (int i = 0; i < 5; i++)
  {
    tier = tier | (buckets[i] << ((int)(ceil(log2(size)))) * i);
  }
  return tier;
}

POSITION encodePosition(int position[height][width][2])
{
  POSITION encodedPosition = 0ULL;
  // printf("encoded height before inserting anything %llu\n", encodedPosition);
  // printf("first height: %d\n", position[0][0][0]);
  for (int x = 0; x < height; x++)
  {
    for (int y = 0; y < width; y++)
    {
      encodedPosition = encodedPosition | ((unsigned long long)position[x][y][0] << (x * width * 3 + y * 3));
      // printf("encoded height after inserting %d, %d: %llu\n", x, y, encodedPosition);
    }
  }
  // printf("encoded heights: %llu\n", encodedPosition);
  int x;
  int y;
  FindLocation(position, 1, &x, &y);
  printf("player 1 location being encoded: %d, %d\n", x, y);
  encodedPosition = encodedPosition | ((unsigned long long)(x * width + y) << (size * 3));
  // printf("encoded first player position: %llu\n", encodedPosition);
  FindLocation(position, 2, &x, &y);
  printf("player 2 location being encoded: %d, %d\n", x, y);
  // printf("amount we're shifting by: %d\n", (int)(size*3+ceil(log2(size))));
  // printf("shifted p2 location: %llu\n", (unsigned long long)(x*width+y) << (int)(size*3+ceil(log2(size))));
  encodedPosition = encodedPosition | ((unsigned long long)(x * width + y) << (int)(size * 3 + ceil(log2(size))));
  // printf("encoded position before tier stuff: %llu\n", encodedPosition);
  if (gHashWindowInitialized)
  {
    TIER tier = BoardToTier(position);
    return gHashToWindowPosition(encodedPosition, tier);
  }
  else
    return encodedPosition;
}

void decodePosition(POSITION position, int decodedPosition[height][width][2])
{
  TIERPOSITION tierPos;
  TIER tier;
  if (gHashWindowInitialized)
  {
    gUnhashToTierPosition(position, &tierPos, &tier);
  }
  else
  {
    tierPos = position;
  }
  // printf("encoded tierPosition: %llu\n", tierPos);
  for (int x = 0; x < height; x++)
  {
    for (int y = 0; y < width; y++)
    {
      decodedPosition[x][y][0] = (tierPos >> (x * width * 3 + y * 3)) % 8;
      decodedPosition[x][y][1] = 0;
    }
  }
  int p1position = (tierPos >> (size * 3)) % size;
  decodedPosition[(int)(floor(p1position / width))][p1position % width][1] = 1;
  int p2position = (tierPos >> (int)(size * 3 + ceil(log2(size)))) % size;
  decodedPosition[(int)(floor(p2position / width))][p2position % width][1] = 2;
}

/*********** END HELPER FUNCTIONS ***********/

/*********** BEGIN TEXTUI FUNCTIONS ***********/

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn)
{
  /* THIS ONE IS MOST IMPORTANT FOR YOUR DEBUGGING */
  int decodedPosition[height][width][2];
  decodePosition(position, decodedPosition);
  printUnhashedPosition(decodedPosition);
}

/* Basically just print the move. */
void PrintMove(MOVE move)
{
  int decodedMove[4];
  decodeMove(move, decodedMove);
  printUnhashedMove(decodedMove);
}

void PrintComputersMove(MOVE computersMove, STRING computersName)
{
  printf("Computer's move: ");
  PrintMove(computersMove);
}

USERINPUT GetAndPrintPlayersMove(POSITION thePosition, MOVE *theMove, STRING playerName)
{
  USERINPUT ret;
  do
  {
    printf("%8s's move x,y,buildx,buildy :  ", playerName);

    ret = HandleDefaultTextInput(thePosition, theMove, playerName);
    if (ret != Continue)
      return (ret);

  } while (TRUE);
  return (Continue);
}

/* Return whether the input text signifies a valid move. */
BOOLEAN ValidTextInput(STRING input)
{
  /* YOUR CODE HERE */
  return TRUE;
}

/* Assume the text input signifies a valid move. Return
the move hash corresponding to the move. */
MOVE ConvertTextInputToMove(STRING input)
{
  STRING token = strtok(input, ",");
  int move[4];
  int i = 0;
  while (token != NULL)
  {
    move[i] = atoi(token);
    token = strtok(NULL, ",");
    i++;
  }
  return encodeMove(move[0], move[1], move[2], move[3]);
}

/* Return the string representation of the move.
Ideally this matches with what the user is supposed to
type in. */
STRING MoveToString(MOVE move)
{
  int decodedMove[4];
  decodeMove(move, decodedMove);
  STRING m = (STRING)SafeMalloc(12);
  sprintf(m, "%d,%d,%d,%d", decodedMove[0], decodedMove[1], decodedMove[2], decodedMove[3]);
  return m;
}

/*********** END TEXTUI FUNCTIONS ***********/

/*********** BEGIN VARIANT FUNCTIONS ***********/

/* How many variants are you supporting? For now, just 1.
Maybe in the future you want to support more variants. */
int NumberOfOptions()
{
  /* YOUR CODE HERE MAYBE LATER BUT NOT NOW */
  return 1;
}

/* Return the current variant id (which is 0 in this case since
for now you're only thinking about one variant). */
int getOption()
{
  /* YOUR CODE HERE MAYBE LATER BUT NOT NOW */
  return 0;
}

/* The input is a variant id. This function sets any global variables
or data structures according to the variant specified by the variant id.
But for now you have one variant so don't worry about this. */
void setOption(int option)
{
  /* YOUR CODE HERE MAYBE LATER BUT NOT NOW */
}

/*********** END VARIANT-RELATED FUNCTIONS ***********/

/* Don't worry about these Interact functions below yet.
They are used for the AutoGUI which eventually we would
want to implement, but they are not needed for solving. */
POSITION InteractStringToPosition(STRING board)
{
  /* YOUR CODE HERE LATER BUT NOT NOW */
  return 0;
}

STRING InteractPositionToString(POSITION position)
{
  /* YOUR CODE HERE LATER BUT NOT NOW */
  return NULL;
}

/* Ignore this function. */
STRING InteractPositionToEndData(POSITION position)
{
  return NULL;
}

STRING InteractMoveToString(POSITION position, MOVE move)
{
  /* YOUR CODE HERE LATER BUT NOT NOW */
  return MoveToString(move);
}
