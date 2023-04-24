/************************************************************************
**
** NAME:        mforestfox.c
**
** DESCRIPTION: Forest Fox
**
** AUTHOR:      Jiachun Li, Yifan Zhou
**
** DATE:        2023-04-22
**
************************************************************************/

#include <stdio.h>
#include "gamesman.h"

/* IMPORTANT GLOBAL VARIABLES */
STRING kAuthorName = "Jiachun Li, Yifan Zhou";
STRING kGameName = "Forest Fox"; //  use this spacing and case
STRING kDBName = "forestfox"; // use this spacing and case
POSITION gNumberOfPositions = 1000000000; // TODO: Put your number of positions upper bound here.
POSITION gInitialPosition = 0; // TODO: Put the hash value of the initial position.
BOOLEAN kPartizan = FALSE; // TODO: Is the game PARTIZAN i.e. given a board does each player have a different set of moves available to them?
BOOLEAN kTieIsPossible = FALSE; // TODO: Is a tie or draw possible?
BOOLEAN kLoopy = FALSE; // TODO: Is this game loopy?
BOOLEAN kSupportsSymmetries = FALSE; // TODO: Whether symmetries are supported (i.e. whether the GetCanonicalPosition is implemented)



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
/*
1,2,3,4,5 || 6,7,8,9,10 || 11,12,13,14,15
*/

/* position:
[0/1] first player moves first/ second player moves first in this turn 
[0/1] lead player has not/ has moved 
[0-7] the current score of the first player
[16] decree card (only 1~15 are valid)
[16] last card (0 if it is the leading turn)
[0-3] additional points of the first player (rank 3 card winning)
[4^15] 01: first player, 10: second player, 00: already played (2 bits for each card, 11 is not valid)

Special position: if init, (cards not shuffled) all 0s
*/
/* Return the hash value of the initial position. */
POSITION GetInitialPosition() {
  /* YOUR CODE HERE */
  return 0;
}
typedef int CARD;
typedef int SCORE;
typedef int SUIT;
typedef int NUM;
typedef POSITION STATUS;
/* Return a linked list of moves. */
BOOLEAN leadPlayer(POSITION p){
  return p&1;
}
BOOLEAN leadPlayerMoved(POSITION p){
  return (p>>1)&1;
}
SCORE firstPlayerScore(POSITION p){
  return (p>>2)&7;
}
CARD getDecreeCard(POSITION p){
  return (p>>5)&15;
}
CARD getLastCard(POSITION p){
  return (p>>9)&15;
}
SCORE getAdditionalPoint(POSITION p){
  return (p>>13)&3;
}
STATUS getCardStatus(POSITION p){
  return (p>>15);
}
SUIT getCardSuit(CARD card){
  if(card>10) return 3;
  if(card>5) return 2;
  return 1;
}
NUM getCardNum(CARD card){
  if (card>10) return card-10;
  if (card>5) return card-5;
  return card;
}
Bool next_permutation(int* begin, int* end) {
    // Find the last element in the non-increasing suffix
    int* i = end - 1;
    while (i > begin && *(i - 1) >= *i) {
        --i;
    }

    // If the sequence is already the largest permutation, return false
    if (i == begin) {
        return False;
    }

    // Find the smallest element in the suffix that is greater than the pivot
    int* j = end - 1;
    while (*j <= *(i - 1)) {
        --j;
    }

    // Swap the pivot with the smallest element greater than the pivot
    int temp = *(i - 1);
    *(i - 1) = *j;
    *j = temp;

    // Reverse the suffix
    j = end - 1;
    while (i < j) {
        temp = *i;
        *i = *j;
        *j = temp;
        ++i;
        --j;
    }

    return True;
}
POSITION setPositionHash(BOOLEAN lead,BOOLEAN moved,SCORE score,CARD decreecard,CARD lastcard,SCORE add,STATUS status){
  return (((((((((((status<<4)|add)<<2)|lastcard)<<4)|decreecard)<<3)|score)<<1|moved)<<1)|lead);
}
void getPositionHash(BOOLEAN *lead,BOOLEAN *moved,SCORE *score,CARD *decreecard,CARD *lastcard,SCORE *add,STATUS *status,POSITION p){
  lead = leadPlayer(p);
  moved = leadPlayerMoved(p);
  score = firstPlayerScore(p);
  decreecard = getDecreeCard(p);
  lastcard = getLastCard(p);
  add = getAdditionalPoint(p);
  status = getCardStatus(p);
}
/*int max(int a,int b){
  return a>b?a:b;
}*/
MOVELIST *GenerateMoves(POSITION position) {
  MOVELIST *moves = NULL;
  if (position == 0ll){
    CARD a[16]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    do{
      CARD decreeCard = a[1],lastCard = 0;
      STATUS status = 0;
      for(int i=2;i<=8;i++) status|=(1ll<<((a[i]-1)*2));// first player 01
      for(int i=9;i<=15;i++) status |=(3ll<<((a[i]-1)*2));//second player 10
      BOOLEAN lead = 0,moved = 0;
      SCORE score = 0, add=0;
      POSITION nextMove = setPositionHash(lead,moved,score,decreeCard,lastCard,add,status);
      moves = CreateMovelistNode(nextMove, status);
      // moves = CreateMovelistNode(nextMove, moves);
    }while(next_permutation(a+1,a+1+15));
  }else{
    CARD decreeCard,lastCard;
    STATUS status;
    SCORE score,add;
    BOOLEAN lead,moved;
    getPositionHash(&lead,&moved,&score,&decreeCard,&lastCard,&add,&status,position);
    if(moved){
      int num = getCardNum(lastCard),suit = getCardSuit(lastCard);
      Bool hasSuit = False;
      for(int i=0;i<32;i+=2){
        CARD card = i/2+1;
        int cardCol = (status>>i)&3;
        if(cardCol==2&&getCardSuit(card)==suit){
          hasSuit = True;
          break;
        }
      }
      if(!hasSuit){
        for(int i=0;i<32;i+=2){
          CARD card = i/2+1;
          int cardCol = (status>>i)&3;
          if(cardCol==2){
            moves = CreateMovelistNode(card, moves);
          }
        }
      }else{
        if(num == 5){
          int mx = 0;
          for(int i=0;i<32;i+=2){
            CARD card = i/2+1;
            int cardCol = (status>>i)&3;
            if(cardCol==2&&getCardSuit(card)==suit){
              mx = fmax(getCardNum(card),mx);
            }
          }
          for(int i=0;i<32;i+=2){
            CARD card = i/2+1;
            int cardCol = (status>>i)&3;
            if(cardCol==2&&getCardSuit(card)==suit&&(getCardNum(card)==mx||getCardNum(card)==1)){
              moves = CreateMovelistNode(card, moves);
            }
          }
        }else{
          for(int i=0;i<32;i+=2){
            CARD card = i/2+1;
            int cardCol = (status>>i)&3;
            if(cardCol==2&&getCardSuit(card)==suit){
              moves = CreateMovelistNode(card, moves);
            }
          }
        }
      }
    }else{//all cards of player 1 is available
      for(int i=0;i<32;i+=2){
        CARD card = i/2+1;
        int cardCol = (status>>i)&3;
        if(cardCol==1){
          moves = CreateMovelistNode(card, moves);
        }
      }
    }
  }
  /* YOUR CODE HERE 
     
     To add to the linked list, do 
     moves = CreateMovelistNode(<the move you're adding>, moves);
     See the function CreateMovelistNode in src/core/misc.c
  */
  return moves;
}

/* move
return the card id
*/
/* Return the position that results from making the 
input move on the input position. */
POSITION DoMove(POSITION position, MOVE move) {
  if(position == 0){
    CARD decreeCard,lastCard;
    STATUS status;
    SCORE score,add;
    BOOLEAN lead,moved;
    lead = 0,moved = 0;
    score = 0,add = 0;
    for(int i=0;i<32;i+=2){
      CARD card = i/2+1;
      int col = (move>>i)&3;
      if(!col) decreeCard = card;
    }
    status = move;
    lastCard = 0;
    POSITION ret = setPositionHash(lead,moved,score,decreeCard,lastCard,add,status);
    return ret;
  }
  BOOLEAN first_lead = leadPlayer(position); // true if first player is the lead player
  SCORE score = firstPlayerScore(position);
  CARD decreecard = getDecreeCard(position);
  CARD lastcard = move;
  if (getCardNum(move) == 1) {
    lastcard = decreecard;
    decreecard = move;
  }
  SCORE add_points = getAdditionalPoint(position);
  // update card's status
  STATUS cur_status = getCardStatus(position);
  cur_status &= ~(3l << (2 * (move-1)));  // play the card out

  if (!leadPlayerMoved(position)) {  // lead player's move
    return setPositionHash(first_lead, 1, score, decreecard, lastcard, add_points, cur_status);
  }
  else {
    // to check who wins
    BOOLEAN lead_player_win;
    SUIT decreecard_suit = getCardSuit(decreecard);
    SUIT leadcard_suit = getCardSuit(lastcard);
    SUIT other_player_suit = getCardSuit(move);
    if (other_player_suit != decreecard_suit && other_player_suit != leadcard_suit) {
      lead_player_win = TRUE;
    }
    else if (other_player_suit == leadcard_suit && getCardNum(move) < getCardNum(lastcard)) {
      lead_player_win = TRUE;
    }
    else {
      lead_player_win = FALSE;
    }

    // to update score and additional points
    if (lead_player_win && first_lead) {
      score++;
      if (lastcard == 3) add_points++;
    }
    else if (!lead_player_win && !first_lead) {
      score++;
      if (move == 3) add_points++;
    }

    return setPositionHash(~first_lead, 0, score, decreecard, 0, add_points, cur_status);
  }
  return 0;
}

/*****************************************************************
**  Determine whether the position is a primitive win,
**  primitive tie, primitive lose, or not primitive.
**  OUTPUTS: (VALUE) an enum; one of: (win, lose, tie, undecided)
**  See src/core/types.h for the value enum definition.
******************************************************************/
VALUE Primitive(POSITION position) {
  STATUS status = getCardStatus(position);
  if (status) return undecided;
  if(status == 0){
    SCORE score = firstPlayerScore(position);
    if (score>=6||(score>=2&&score<=3)) return lose;
    else return win; 
  }
  /* YOUR CODE HERE */
  return undecided;
}

/*WINBY computeWinBy(POSITION position) {
  Primitive(firstPlayerScore(position)) + getAdditionalPoint(position) - 
	
  int whitetally, blacktally;
	char *board = getBoard(position);
	tallyPieces(board, &blacktally, &whitetally, NULL);
	free(board);
	return blacktally - whitetally;
}*/

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
  printf("Decree Card: %d\n", getDecreeCard(position));
  STATUS status = getCardStatus(position);
  printf("First Player's Card: ");
  int i;
  for (i = 1; i <= 15; i++) {
    unsigned int mask = 0x3 << ((i-1)*2);
    unsigned int bits = (status & mask) >> ((i-1)*2);
    if (bits == 0b01) {
      printf("%d ", i);
    }
  }
  printf("\nSecond Player's Card: ");
  for (i = 1; i <= 15; i++) {
    unsigned int mask = 0x3 << ((i-1)*2);
    unsigned int bits = (status & mask) >> ((i-1)*2);
    if (bits == 0b10) {
      printf("%d ", i);
    }
  }
  if (usersTurn) printf("\nIt's %s's turn\n", playerName);
  else printf("\nIt's not %s's turn\n", playerName);
}

void PrintComputersMove(MOVE computersMove, STRING computersName) {
  printf("%s played %d\n", computersName, computersMove);
}

USERINPUT GetAndPrintPlayersMove(POSITION position, MOVE *move, STRING playerName) {
  printf("%s played %d\n", playerName, *move);
  PrintPosition(position, playerName, TRUE);
  return Continue;
}

/* Return whether the input text signifies a valid move. */
BOOLEAN ValidTextInput(STRING input) {
  int len = strlen(input);
  if (len == 1) return (input[0] <= '9' && input[0] >= '1');
  else if (len == 2) return (input[0] == '1' && input[1] <= '5' && input[1] >= '0');

  return FALSE;
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
  STRING s = (STRING) SafeMalloc(sizeof(char) * 2);
	sprintf(s, "%d", move);
  return s;
}

/* Basically just print the move. */
void PrintMove(MOVE move) {
  printf("The move is %d\n", move);
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
  /* YOUR CODE HERE  */
}

/*********** END VARIANT-RELATED FUNCTIONS ***********/







/* Don't worry about these Interact functions below yet.
They are used for the AutoGUI which eventually we would
want to implement, but they are not needed for solving. */
POSITION InteractStringToPosition(STRING board) {
  /* YOUR CODE HERE */
  return 0;
}

STRING InteractPositionToString(POSITION position) {
  /* YOUR CODE HERE */
  return NULL;
}

/* Optional. */
STRING InteractPositionToEndData(POSITION position) {
  return NULL;
}

STRING InteractMoveToString(POSITION position, MOVE move) {
  /* YOUR CODE HERE */
  return MoveToString(move);
}