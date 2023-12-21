/************************************************************************
**
** NAME:        mkonane.c
**
** DESCRIPTION: Konane
**
** AUTHOR:      Haonan Huang
**
** DATE:        2023-04-08
**
************************************************************************/

#include "gamesman.h"

#ifndef max
    #define max(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef min
    #define min(a,b) ((a) < (b) ? (a) : (b))
#endif

/* IMPORTANT GLOBAL VARIABLES */
CONST_STRING kAuthorName = "Haonan Huang";
CONST_STRING kGameName = "Konane"; //  use this spacing and case
CONST_STRING kDBName = "konane"; // use this spacing and case
POSITION gNumberOfPositions; // Set in InitializeGame().
POSITION gInitialPosition; // Set in InitializeGame().
BOOLEAN kPartizan = TRUE; //Is the game PARTIZAN i.e. given a board does each player have a different set of moves available to them?
BOOLEAN kTieIsPossible = FALSE; //Is a tie or draw possible?
BOOLEAN kLoopy = FALSE; //Is this game loopy?
BOOLEAN kSupportsSymmetries = TRUE; //Whether symmetries are supported (i.e. whether the GetCanonicalPosition is implemented)

/* Likely you do not have to change these. */
POSITION GetCanonicalPosition(POSITION);
STRING MoveToString(MOVE);
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
/* Ignore this function. */
void SetTclCGameSpecificOptions(int theOptions[]) { (void) theOptions; }
/* Do not worry about this yet because you will only be supporting 1 variant for now. */
void GameSpecificMenu() {}

int gSymmetryMatrix5x6[8][30] = {
	{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29},
  {24,25,26,27,28,29,18,19,20,21,22,23,12,13,14,15,16,17,6,7,8,9,10,11,0,1,2,3,4,5},
  {29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0},
  {5,4,3,2,1,0,11,10,9,8,7,6,17,16,15,14,13,12,23,22,21,20,19,18,29,28,27,26,25,24}
};

int gSymmetryMatrix5x5[8][30] = {
	{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,0,0,0,0,0},
	{4,3,2,1,0,9,8,7,6,5,14,13,12,11,10,19,18,17,16,15,24,23,22,21,20,0,0,0,0,0},
	{20,15,10,5,0,21,16,11,6,1,22,17,12,7,2,23,18,13,8,3,24,19,14,9,4,0,0,0,0,0},
	{0,5,10,15,20,1,6,11,16,21,2,7,12,17,22,3,8,13,18,23,4,9,14,19,24,0,0,0,0,0},
	{24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,0,0,0,0,0},
	{20,21,22,23,24,15,16,17,18,19,10,11,12,13,14,5,6,7,8,9,0,1,2,3,4,0,0,0,0,0},
	{4,9,14,19,24,3,8,13,18,23,2,7,12,17,22,1,6,11,16,21,0,5,10,15,20,0,0,0,0,0},
	{24,19,14,9,4,23,18,13,8,3,22,17,12,7,2,21,16,11,6,1,20,15,10,5,0,0,0,0,0,0}
};
int gSymmetryMatrix4x5[4][30] = {
	{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,0,0,0,0,0,0,0,0,0,0},
  {4,3,2,1,0,9,8,7,6,5,14,13,12,11,10,19,18,17,16,15,0,0,0,0,0,0,0,0,0,0},
  {19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,0,0,0,0,0,0,0,0,0,0},
  {15,16,17,18,19,10,11,12,13,14,5,6,7,8,9,0,1,2,3,4,0,0,0,0,0,0,0,0,0,0}
};
int gSymmetryMatrix4x4[8][30] = {
	{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,4,8,12,1,5,9,13,2,6,10,14,3,7,11,15,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{15,11,7,3,14,10,6,2,13,9,5,1,12,8,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{3,2,1,0,7,6,5,4,11,10,9,8,15,14,13,12,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{12,8,4,0,13,9,5,1,14,10,6,2,15,11,7,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{12,13,14,15,8,9,10,11,4,5,6,7,0,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{3,7,11,15,2,6,10,14,1,5,9,13,0,4,8,12,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};


int boardDimensionX = 4; //default
int boardDimensionY = 4; //default
int boardSize = 16; //default
const char emptyPiece = '-';
const char piece = 'O';
const char printPiece[2] = {'x', 'o'};
int (*symmetriesToUse)[30] = gSymmetryMatrix4x4; //default
int maxSymmetry = 8; //default


// HASH/UNHASH
TIER BoardToTier(char*);
POSITION BoardToPosition(char*, int);
void PositionToBoard(POSITION, char*, int*);

//TIER GAMESMAN
void SetupTierStuff();
TIERLIST* TierChildren(TIER);
TIERPOSITION NumberOfTierPositions(TIER);
STRING TierToString(TIER);

// SOLVING FUNCTION
BOOLEAN inBoard(int, int);
int boardStatus(POSITION);
void InitializeGame();
POSITION GetInitialPosition();
MOVELIST *GenerateMoves(POSITION);
POSITION DoMove(POSITION, MOVE);
VALUE Primitive(POSITION);
POSITION GetCanonicalPosition(POSITION);


// TEXTUI FUNCTION
void PrintBoard(char*);
void PrintPosition(POSITION, STRING, BOOLEAN);
void PrintComputersMove(MOVE, STRING);
USERINPUT GetAndPrintPlayersMove(POSITION, MOVE *, STRING);
BOOLEAN ValidTextInput(STRING);
MOVE ConvertTextInputToMove(STRING);
STRING MoveToString(MOVE);
void PrintMove(MOVE);



// Interact functions
POSITION InteractStringToPosition(STRING);
STRING InteractPositionToString(POSITION);
STRING InteractPositionToEndData(POSITION);
STRING InteractMoveToString(POSITION, MOVE);



/*********** BEGIN TIER FUNCIONS ***********/
void SetupTierStuff() {
	// kSupportsTierGamesman
  kExclusivelyTierGamesman = TRUE;
	kSupportsTierGamesman = TRUE;
	// All function pointers-
	gTierChildrenFunPtr= &TierChildren;
	gNumberOfTierPositionsFunPtr    = &NumberOfTierPositions;
	// gGetInitialTierPositionFunPtr   = &GetInitialTierPosition;
	//gIsLegalFunPtr				= &IsLegal;
	// gGenerateUndoMovesToTierFunPtr  = &GenerateUndoMovesToTier;
	// gUnDoMoveFunPtr                                 = &UnDoMove;
	gTierToStringFunPtr = &TierToString;

	// Tier-Specific Hashes
	int piecesArray[7] = { piece, 0, 0, emptyPiece, 0, 0, -1 };

  for (int i = 0; i <= boardSize; i++){
    piecesArray[1] = piecesArray[2] = i;
    piecesArray[4] = piecesArray[5] = boardSize-i;
    generic_hash_init(boardSize, piecesArray, NULL, 0);
    generic_hash_set_context(i);
  }

	// it's already in the final hash context, so set the position:
  char board[boardSize];
  for (int i = 0; i < boardSize; i++){
    board[i] = piece;
  }
  //initial tier
  gInitialTier = BoardToTier(board);
	gInitialTierPosition = BoardToPosition(board, 1);
}


TIER BoardToTier(char* board) {
	int tier = 0;
	for (int i = 0; i < boardSize; i++){
    if (board[i] == piece)
      tier++;
  }
	return tier;
}

TIERPOSITION NumberOfTierPositions(TIER tier) {
	generic_hash_context_switch(tier);
	return generic_hash_max_pos();
}

TIERLIST* TierChildren(TIER tier) {
	TIERLIST* tierlist = NULL;
  if (tier == (POSITION) boardSize || tier == (POSITION) boardSize - 1){
    tierlist = CreateTierlistNode(tier-1, tierlist);
  } else if (tier != 0) {
    for (int i = 1; i < (boardDimensionY+1) / 2; i++){
      if ((int)tier - i >= 0){
        tierlist = CreateTierlistNode(tier-i, tierlist);
      }
    }
  }
	return tierlist;
}

void PositionToBoard(POSITION position, char* board, int* player)
{
	if (gHashWindowInitialized) { // using hash windows
		TIERPOSITION tierpos; TIER tier;
		gUnhashToTierPosition(position, &tierpos, &tier); // get tierpos
		generic_hash_context_switch(tier); // switch to that tier's context
    generic_hash_unhash(tierpos, board);
    *player = generic_hash_turn(tierpos);
	} else {
    generic_hash_unhash(position, board);
    *player = generic_hash_turn(position);
  }
}

POSITION BoardToPosition(char* board, int player)
{
	POSITION position;
	if (gHashWindowInitialized) {
		TIER tier = BoardToTier(board); // find this board's tier
		generic_hash_context_switch(tier); // switch to that context
		TIERPOSITION tierpos = generic_hash_hash((char*)board, player); //unhash
		position = gHashToWindowPosition(tierpos, tier); //gets TIERPOS, find POS
	} else position = generic_hash_hash((char*)board, player);
	return position;
}

STRING TierToString(TIER tier) {
	STRING tierStr = (STRING) SafeMalloc(sizeof(char)*20);
	sprintf(tierStr, "%lld Pieces Placed", tier);
	return tierStr;
}

/*********** END TIER FUNCIONS ***********/


/*********** BEGIN SOLVING FUNCIONS ***********/

BOOLEAN inBoard(int x, int y) {
  return x >= 0 && y >= 0 && x < boardDimensionX && y < boardDimensionY;
}

int boardStatus(POSITION position){
  char board[boardSize];
  int emptyPieceCnt = 0;
  int player;
  PositionToBoard(position, board, &player);
  // generic_hash_unhash(position, board);
  for (int i = 0; i < boardSize; i++){
    if (board[i] == emptyPiece){
      emptyPieceCnt++;
    }
    if (emptyPieceCnt >= 2){
      return 1;
    }
  }
  return 0;
}


/* Initialize any global variables or data structures needed before
solving or playing the game. */
void InitializeGame() {

  /* FOR THE PURPOSES OF INTERACT. FEEL FREE TO CHANGE IF SOLVING. */ 
  if (gIsInteract) {
    gLoadTierdbArray = FALSE; // SET TO TRUE IF SOLVING
  }
  /********************************/

  gCanonicalPosition = GetCanonicalPosition;
  gMoveToStringFunPtr = &MoveToString;
  gSymmetries = TRUE;

  boardSize = boardDimensionX * boardDimensionY;

  //discard current hash
	generic_hash_destroy();
	//Setup Tier Stuff (at bottom)
	SetupTierStuff();

	int piecesArray[7] = { piece, 0, boardSize, emptyPiece, 0, boardSize, -1 };
  gNumberOfPositions = generic_hash_init(boardSize, piecesArray, NULL, 0);
  // gNumberOfPositions = pow(2, boardSize);
  char initialBoard[boardSize];
  for (int i = 0; i < boardSize; i++){
    initialBoard[i] = piece;
  }
  gInitialPosition = BoardToPosition(initialBoard, 1);
}

/* Return the hash value of the initial position. */
POSITION GetInitialPosition() {
  return gInitialPosition;
}

/* Return a linked list of moves. */
MOVELIST *GenerateMoves(POSITION position) {
  char board[boardSize];
  int player;
  PositionToBoard(position, board, &player);
  // generic_hash_unhash(position, board);
  // int player = generic_hash_turn(position);

  int dx[4] = {0, 1, 0, -1};
  int dy[4] = {1, 0, -1, 0};

  MOVELIST *moves = NULL;
  if (boardStatus(position) == 1){
    for (int i = 0; i < boardDimensionX; i++){
      for (int j = 0; j < boardDimensionY; j++){
        if (board[i*boardDimensionY+j] == piece && player % 2 == (i+j+1) % 2){
          for (int dir = 0; dir < 4; dir++){
            int d = 1;
            BOOLEAN canmove = TRUE;
            while (canmove) {
              canmove = FALSE;
              int nx = d * dx[dir] + i, ny = d * dy[dir] + j;
              if (inBoard(nx, ny) && board[nx*boardDimensionY+ny] != emptyPiece) {
                int mx = (d + 1) * dx[dir] + i, my = (d + 1) * dy[dir] + j;
                if (inBoard(mx, my) && board[mx*boardDimensionY+my] == emptyPiece) {
                    int moveValue = (i * boardDimensionY + j) * boardSize + (mx * boardDimensionY + my);
                    moveValue = moveValue * 2 + 1;
                    moves = CreateMovelistNode(moveValue, moves);
                  canmove = TRUE;
                  d += 2;
                }
              }
            }
          }
        }
      }
    }
  } else { // Start of the game (removing a piece from the board)
    // P1 removes one of P1's pieces at the center or corners of the board
    // There are two "center" pieces that can be removed in an EVENxEVEN board 
    // and one "center" piece that can be removed in an ODDxEVEN or ODDxODD board.
    // P2 removes one of P2's pieces orthogonally adjacent to the hole created by P1
    int pm = player % 2;
    int moveValue;
    if (player == 1) { // P1's turn
      // corner moves
      for (int i = 0; i < boardDimensionX; i++){
        for (int j = 0; j < boardDimensionY; j++) {
          if ((i == 0 || i == boardDimensionX - 1) && (j == 0 || j == boardDimensionY - 1)) {
            if (board[i*boardDimensionY+j] == piece && player % 2 == (i+j+1) % 2) {
              int moveValue = i * boardDimensionY + j;
              moveValue = moveValue * 2;
              moves = CreateMovelistNode(moveValue, moves);
            }
          }
        }
      }
      for (int i = (boardDimensionX - 1) >> 1; i <= boardDimensionX >> 1; i++) {
        for (int j = (boardDimensionY - 1) >> 1; j <= boardDimensionY >> 1; j++) {
          if (board[i*boardDimensionY+j] == piece && player % 2 == (i+j+1) % 2) {
            int moveValue = i * boardDimensionY + j;
            moveValue = moveValue * 2;
            moves = CreateMovelistNode(moveValue, moves);
          }
        }
      }
    } else { // P2's turn
      for (int i = 0; i < boardDimensionX; i++) {
        for (int j = 0; j < boardDimensionY; j++) {
          if (board[i * boardDimensionY + j] == emptyPiece) {
            if (i != 0 && pm % 2 == ((i-1)+j+1) % 2) {
              moveValue = ((i-1) * boardDimensionY + j) * 2;
              moves = CreateMovelistNode(moveValue, moves);
            }
            if (i != boardDimensionX-1 && pm % 2 == ((i+1)+j+1) % 2) {
              moveValue = ((i+1) * boardDimensionY + j) * 2;
              moves = CreateMovelistNode(moveValue, moves);
            }
            if (j != 0 && pm % 2 == (i+(j-1)+1) % 2) {
              moveValue = (i * boardDimensionY + (j-1)) * 2;
              moves = CreateMovelistNode(moveValue, moves);
            }
            if (j != boardDimensionY-1 && pm % 2 == (i+(j+1)+1) % 2) {
              moveValue = (i * boardDimensionY + (j+1)) * 2;
              moves = CreateMovelistNode(moveValue, moves);
            }
            return moves;
          }
        }
      }
    }
  }
  return moves;
}

/* Return the position that results from making the 
input move on the input position. */
POSITION DoMove(POSITION position, MOVE move) {
  // GetCanonicalPosition(position);
  char board[boardSize];
  int player;
  PositionToBoard(position, board, &player);
  // char board[boardSize];
  // generic_hash_unhash(position, board);
  // int player = generic_hash_turn(position);
  int simMove = move / 2;
  if (move % 2 == 0){
    int x = simMove / boardDimensionY;
    int y = simMove % boardDimensionY;
    board[x*boardDimensionY+y] = emptyPiece;
  } else {
    int from = simMove / boardSize;
    int to = simMove % boardSize;
    int fromx = from / boardDimensionY;
    int fromy = from % boardDimensionY;
    int tox = to / boardDimensionY;
    int toy = to % boardDimensionY;

    int minx = min(fromx, tox);
    int maxx = max(fromx, tox);
    int miny = min(fromy, toy);
    int maxy = max(fromy, toy);
    for (int x = minx; x <= maxx; x++) {
        for (int y = miny; y <= maxy; y++) {
            board[x*boardDimensionY+y] = emptyPiece;
        }
    }
    board[tox*boardDimensionY+toy] = piece;
  }
  return BoardToPosition(board, (player % 2) + 1);
}

/*****************************************************************
**  Determine whether the position is a primitive win,
**  primitive tie, primitive lose, or not primitive.
**  OUTPUTS: (VALUE) an enum; one of: (win, lose, tie, undecided)
**  See src/core/types.h for the value enum definition.
******************************************************************/
VALUE Primitive(POSITION position) {
  MOVELIST *moveList = GenerateMoves(position);
  if (moveList != NULL){
    FreeMoveList(moveList);
    return undecided;
  }
  return lose;
}

/* Symmetry Handling: Return the canonical position. */
POSITION GetCanonicalPosition(POSITION position) {
  char originalBoard[boardSize];
  int player;
  PositionToBoard(position, originalBoard, &player);
  int otherplayer = player == 1 ? 2 : 1;
  char canonBoard[boardSize];
  POSITION canonPos = position;
  int bestSymmetryNum = 0;

  for (int symmetryNum = 1; symmetryNum < maxSymmetry; symmetryNum++){
    for (int i = boardSize - 1; i >= 0; i--) {
      char pieceInSymmetry = originalBoard[symmetriesToUse[symmetryNum][i]];
      char pieceInBest = originalBoard[symmetriesToUse[bestSymmetryNum][i]];
      if (pieceInSymmetry != pieceInBest) {
        if (pieceInSymmetry > pieceInBest) // If new smallest hash.
          bestSymmetryNum = symmetryNum;
        break;
      }
    }
  }
  if (bestSymmetryNum == 0) {
    return position;
	}

  for (int i = 0; i < boardSize; i++) // Transform the rest of the board.
      canonBoard[i] = originalBoard[symmetriesToUse[bestSymmetryNum][i]];
  canonPos = BoardToPosition(canonBoard, (boardSize == 25 || bestSymmetryNum < maxSymmetry/2) ? player : otherplayer);
  return canonPos;
}

/*********** END SOLVING FUNCTIONS ***********/




/*********** BEGIN TEXTUI FUNCTIONS ***********/
void PrintBoard(char* board){
  printf("\n");
  for (int i = 0; i < boardDimensionX; i++){
    for (int j = 0; j < boardDimensionY; j++){
      if (board[i*boardDimensionY+j] == emptyPiece)
        printf("%c", board[i*boardDimensionY+j]);
      else printf("%c", printPiece[(i+j+1)%2]);
    }
    printf("\n");
  }
  printf("\n");
}

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn) {
  /* THIS ONE IS MOST IMPORTANT FOR YOUR DEBUGGING */
  
  char board[boardSize];
  // generic_hash_unhash(position, board);
  int player;
  PositionToBoard(position, board, &player);
  PrintBoard(board);
  printf("\t%s\n\n", GetPrediction(position,playerName,usersTurn));
}

void PrintComputersMove(MOVE computersMove, STRING computersName) {
  (void) computersName;
  PrintMove(computersMove);
}

USERINPUT GetAndPrintPlayersMove(POSITION position, MOVE *move, STRING playerName) {
  USERINPUT ret;
	do {
		printf("%8s's move [(u)ndo]/[<origin> <target>] :  ", playerName);
		ret = HandleDefaultTextInput(position, move, playerName);
		if (ret != Continue) {
			return ret;
    }
	} while (TRUE);
	return Continue;
}

/* Return whether the input text signifies a valid move. */
BOOLEAN ValidTextInput(STRING input) {
  return strlen(input) == 3 || strlen(input) == 7;
}

/* Assume the text input signifies a valid move. Return
the move hash corresponding to the move. */
MOVE ConvertTextInputToMove(STRING input) {
  if (strlen(input) == 3){
    int move = (((int)input[0] - 48) * boardDimensionY + ((int)input[2] - 48))*2;
    PrintMove(move);
    return move;
  } else {
    int move = ((int)input[0] - 48) * boardDimensionY + ((int)input[2] - 48);
    move *= boardSize;
    move += ((int)input[4] - 48) * boardDimensionY + ((int)input[6] - 48);
    move = move*2+1;
    PrintMove(move);
    return move;
  }
}

/* Return the string representation of the move. 
Ideally this matches with what the user is supposed to
type when they specify moves. */
STRING MoveToString(MOVE move) {
  char* result = (char*) SafeMalloc(30);
  int simMove = move / 2;
  if (move % 2 == 0){
    sprintf(result, "%d", simMove);
  } else {
    int from = simMove / boardSize;
    int to = simMove % boardSize;
    sprintf(result, "%d -> %d", from, to);
  }    
  return result;
}

/* Basically just print the move. */
void PrintMove(MOVE move) {
  STRING moveString = MoveToString(move);
  printf("%s", moveString);
  SafeFree(moveString);
}

/*********** END TEXTUI FUNCTIONS ***********/




/*********** BEGIN VARIANT FUNCTIONS ***********/

/* How many variants are you supporting? */
int NumberOfOptions() {
  return 4;
}

/* Return the current variant id. */
int getOption() {
  return boardDimensionX + boardDimensionY - 8;
}

/* The input is a variant id. This function sets any global variables
or data structures according to the variant specified by the variant id. */
void setOption(int option) {
  if (option == 0){
    boardDimensionX = boardDimensionY = 4;
    symmetriesToUse = gSymmetryMatrix4x4; 
    maxSymmetry = 8;
  } else if (option == 1){
    boardDimensionX = 4;
    boardDimensionY = 5;
    symmetriesToUse = gSymmetryMatrix4x5;
    maxSymmetry = 4;
  } else if (option == 2){
    boardDimensionX = boardDimensionY = 5;
    symmetriesToUse = gSymmetryMatrix5x5; 
    maxSymmetry = 8;
  } else if (option == 3){
    boardDimensionX = 5;
    boardDimensionY = 6;
    symmetriesToUse = gSymmetryMatrix5x6; 
    maxSymmetry = 4;
  }
}

/*********** END VARIANT-RELATED FUNCTIONS ***********/




/* Don't worry about these Interact functions below yet.
They are used for the AutoGUI which eventually we would
want to implement, but they are not needed for solving. */
POSITION InteractStringToPosition(STRING board) {
  // Ignore the first 8 characters
  char realBoard[boardSize];

  for (int i = 0; i < boardSize; i++) {
    if (board[i + 8] == emptyPiece) {
      realBoard[i] = emptyPiece;
    } else {
      realBoard[i] = piece;
    }
  }
  int player = board[2] == 'A' ? 1 : 2;

  POSITION position;
	if (gHashWindowInitialized) {
		TIER tier = BoardToTier(realBoard); // find this board's tier
		generic_hash_context_switch(tier); // switch to that context
		TIERPOSITION tierpos = generic_hash_hash((char*)realBoard, player); //unhash

    gInitializeHashWindow(tier, FALSE); // TODO: should be called in interact.c @Cameron :)

		position = gHashToWindowPosition(tierpos, tier); //gets TIERPOS, find POS

	} else position = generic_hash_hash((char*)realBoard, player);
	return position;

  // return BoardToPosition(realBoard, player);
}

STRING InteractPositionToString(POSITION position) {
  char board[boardSize];
  int player;
  PositionToBoard(position, board, &player);
  STRING result = (STRING) SafeMalloc(9 + boardSize);
  result[0] = 'R';
  result[1] = '_';
  result[2] = player == 1 ? 'A' : 'B';
  result[3] = '_';
  result[4] = '0';
  result[5] = '_';
  result[6] = '0';
  result[7] = '_';
  for (int i = 0; i < boardDimensionX; i++){
    for (int j = 0; j < boardDimensionY; j++){
      if (board[i*boardDimensionY+j] == emptyPiece)
        result[i*boardDimensionY+j+8] = emptyPiece;
      else
        result[i*boardDimensionY+j+8] = printPiece[(i+j+1)%2];
    }
  }
  result[8 + boardSize] = '\0';
  return result;
}

STRING InteractMoveToString(POSITION position, MOVE move) {
  (void) position;
  char* result = (char*) SafeMalloc(16);
  int simMove = move / 2;
  if (move % 2 == 0){
    sprintf(result, "A_h_%d_x", simMove);
  } else {
    int from = simMove / boardSize;
    int to = simMove % boardSize;
    sprintf(result, "M_%d_%d_y", from, to);
  }    
  return result;
}