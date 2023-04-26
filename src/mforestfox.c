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
BOOLEAN kDebugMenu = TRUE;

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
[0/1] lead player has not/ has moved 
[0-7] the current score of the first player
[16] decree card (only 1~15 are valid)
[16] last card (0 if it is the leading turn)
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
BOOLEAN leadPlayerMoved(POSITION p){
  return p&1;
}
SCORE firstPlayerScore(POSITION p){
  return (p>>1)&7;
}
CARD getDecreeCard(POSITION p){
  return (p>>4)&15;
}
CARD getLastCard(POSITION p){
  return (p>>8)&15;
}
STATUS getCardStatus(POSITION p){
  return (p>>12);
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
POSITION setPositionHash(BOOLEAN moved,SCORE score,CARD decreecard,CARD lastcard,STATUS status){
  return (((((((status<<4)|lastcard)<<4)|decreecard)<<3)|score)<<1|moved);
}
void getPositionHash(BOOLEAN *moved,SCORE *score,CARD *decreecard,CARD *lastcard,STATUS *status,POSITION p){
  moved = leadPlayerMoved(p);
  score = firstPlayerScore(p);
  decreecard = getDecreeCard(p);
  lastcard = getLastCard(p);
  status = getCardStatus(p);
}
/*int max(int a,int b){
  return a>b?a:b;
}*/
MOVELIST *GenerateMoves(POSITION position) {
  MOVELIST *moves = NULL;
  if (position == 0ll){
    CARD a[16]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    for(int d=1;d<=15;d++){
      for(int msk=0;msk<(1<<14);msk++){
        int cnt =0,x=msk;
        while(x) cnt+=(x&1),x>>=1;
        if(cnt!=7) continue;
        int stat=0;
        for(int i=0;i<14;i++){
          if(i<d){
            if((msk>>i)&1) stat += (1ll<<(i*2));
            else stat += (2ll<<(i*2));
          }
          else{
            if((msk>>i)&1) stat += (1ll<<((i+1)*2));
            else stat += (2ll<<((i+1)*2));
          } 
        }
        moves = CreateMovelistNode(stat, moves);
      }
    }
  }else{
    CARD decreeCard=getDecreeCard(position),lastCard=getLastCard(position);
    STATUS status=getCardStatus(position);
    SCORE score=firstPlayerScore(position);
    BOOLEAN moved=leadPlayerMoved(position);
    //getPositionHash(&moved,&score,&decreeCard,&lastCard,&status,position);
    if(moved){
      int num = getCardNum(lastCard),suit = getCardSuit(lastCard);
      Bool hasSuit = False;
      for(int i=0;i<30;i+=2){
        CARD card = i/2+1;
        int cardCol = (status>>i)&3;
        if(cardCol==2&&getCardSuit(card)==suit){
          hasSuit = True;
          break;
        }
      }
      if(!hasSuit){
        for(int i=0;i<30;i+=2){
          CARD card = i/2+1;
          int cardCol = (status>>i)&3;
          if(cardCol==2){
            moves = CreateMovelistNode(card, moves);
          }
        }
      }else{
        if(num == 5){
          int mx = 0;
          for(int i=0;i<30;i+=2){
            CARD card = i/2+1;
            int cardCol = (status>>i)&3;
            if(cardCol==2&&getCardSuit(card)==suit){
              mx = fmax(getCardNum(card),mx);
            }
          }
          for(int i=0;i<30;i+=2){
            CARD card = i/2+1;
            int cardCol = (status>>i)&3;
            if(cardCol==2&&getCardSuit(card)==suit&&(getCardNum(card)==mx||getCardNum(card)==1)){
              moves = CreateMovelistNode(card, moves);
            }
          }
        }else{
          for(int i=0;i<30;i+=2){
            CARD card = i/2+1;
            int cardCol = (status>>i)&3;
            if(cardCol==2&&getCardSuit(card)==suit){
              moves = CreateMovelistNode(card, moves);
            }
          }
        }
      }
    }else{//all cards of player 1 is available
      for(int i=0;i<30;i+=2){
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
    SCORE score;
    BOOLEAN moved;
    moved = 0;
    score = 0;
    for(int i=0;i<30;i+=2){
      CARD card = i/2+1;
      int col = (move>>i)&3;
      printf("card %d's color = %d\n",card,col);
      if(!col) decreeCard = card;
    }
    printf("decreeCard = %d",decreeCard);
    status = move;
    lastCard = 0;
    POSITION ret = setPositionHash(moved,score,decreeCard,lastCard,status);
    printf("ret = %lld %x\n",ret,ret);
    printf("decree Card of ret = %d\n",getDecreeCard(ret));
    return ret;
  }
  SCORE score = firstPlayerScore(position);
  CARD decreecard = getDecreeCard(position);
  STATUS cur_status = getCardStatus(position);
  CARD lastcard = getLastCard(position);

  cur_status &= ~(3ll << (2 * (move-1)));
  if (getCardNum(move) == 1) {
    CARD tmp = decreecard;
    decreecard = move;
    move = tmp;
  }
  //SCORE add_points = getAdditionalPoint(position);
  // update card's status
  
  //cur_status |= (3ll<<(2*(move-1)));
  //cur_status &= ~(3l << (2 * (move-1)));  // play the card out

  if (!leadPlayerMoved(position)) {  // lead player's move
    lastcard = move;
    return setPositionHash(1, score, decreecard, lastcard, cur_status);
  }
  else {
    // to check who wins
    BOOLEAN lead_player_win;
    SUIT decreecard_suit = getCardSuit(decreecard);
    SUIT leadcard_suit = getCardSuit(lastcard);
    SUIT other_player_suit = getCardSuit(move);
    int lead_num = getCardNum(decreecard);
    int other_num = getCardNum(move);
    if(!(lead_num == 3&&other_num == 3)){
        if(lead_num == 3){//trumpth
        leadcard_suit = decreecard_suit;
      }
      if(other_num == 3){
        other_player_suit = decreecard_suit;
      }
    }
    if(other_player_suit != leadcard_suit){
      if(other_player_suit == decreecard_suit || leadcard_suit == decreecard_suit)
        lead_player_win = (leadcard_suit = decreecard_suit);
      else lead_player_win = TRUE;
    }else{
      lead_player_win = (lead_num > other_num);
    }
    // if (other_player_suit != decreecard_suit && other_player_suit != leadcard_suit) {
    //   lead_player_win = TRUE;
    // }
    // else if (other_player_suit == leadcard_suit && getCardNum(move) < getCardNum(lastcard)) {
    //   lead_player_win = TRUE;
    // }
    // else {
    //   lead_player_win = FALSE;
    // }

    // to update score and additional points
    if (lead_player_win) {
      score++;
    }

    return setPositionHash(0, score, decreecard, 0, cur_status);
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
  if(position ==0) return undecided;
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
  printf("Current Location = %lld %x\n",position,position);
  printf("Decree Card: %d\n", getDecreeCard(position));
  STATUS status = getCardStatus(position);
  printf("Current Status = %d %x\n",status,status);
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
	USERINPUT ret;
	do {
		printf("%8s's move [(u)ndo]/[1-15] :  ", playerName);
		ret = HandleDefaultTextInput(position, move, playerName);
		if (ret != Continue) {
			return ret;
        }
	} while (TRUE);
	return Continue;
}

/* Return whether the input text signifies a valid move. */
BOOLEAN ValidTextInput(STRING input) {
  //return TRUE;
  int len = strlen(input);
  if (len == 1) return (input[0] <= '9' && input[0] >= '1');
  else if (len == 2) return (input[0] == '1' && input[1] <= '5' && input[1] >= '0');
  int num = atoi(input),cnt1 = 0,cnt2=0;
  for(int i=0;i<30;i+=2) {
    int x=(num>>i)&3;
    if(x==1) cnt1++;
    else if(x==2) cnt2++;
  }
  return cnt1==7&&cnt2==7;
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
  printf("The move is %d %x\n", move,move);
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
POSITION InteractStringToPosition(STRING str) {
  /* YOUR CODE HERE */
  /* R_A_0_0_abdce--hijkl--o(decree card)-(first card)f(second card)3(first score)0(second score) */

  BOOLEAN moved = (str[2] == 'A') ? FALSE : TRUE; 

  STATUS status = 0;
  // first check first player's cards
  for (int i = 8; i < 15; i++) {
    if (str[i] == '-') continue;
    int card = str[i] - 96;
    status |= (0b01 << (card - 1));
  }
  // check the second player's cards
  for (int i = 15; i < 22; i++) {
    if (str[i] == '-') continue;
    int card = str[i] - 96;
    status |= (0b10 << (card - 1));
  }

  CARD decreecard = str[22] - 96;
  CARD firstcard = (str[23] != '-') ? str[23] - 96 : 0;
  CARD secondcard = (str[24] != '-') ? str[24] - 96 : 0;
  CARD lastcard = (firstcard > 0) ? firstcard : secondcard;
  SCORE firstscore = atoi(str[25]);

  return setPositionHash(moved, firstscore, decreecard, lastcard, status);
}

STRING InteractPositionToString(POSITION position) {
  /* YOUR CODE HERE */
  /* R_A_0_0_abdce--hijkl--o(decree card)-(first card)f(second card)3(first score)0(second score) */

  // str[0-1] = 'R_'
  // str[2] = 'A' / 'B', 根据position判断，如果当前是玩家1的轮数就A，否则B
  // str[3-7] = '_0_0_'
  // str[8-14] 为玩家1的卡牌，是哪几张，a-o分别对应1-15，如果手上只有4张牌了，就只有前四个char是字母，后3个都是'-'
  // str[15-21] 玩家2的卡牌，同上
  // str[22] decree card是哪一张，用a-o表示
  // str[23] 玩家1出的牌
  // str[24] 玩家2出的牌，str[23]和str[24]不能同时是字母，要么是 [字母][-]，要么是 [-][字母]，该字母就是last card，具体谁的是字母，如果玩家1的轮数，字母就是玩家2的，即 [-][字母]
  // str[25] 玩家1的分数，score转换就行
  // str[26] 玩家2的分数, 玩了的局数-score就行

  STRING str = (STRING) SafeMalloc(sizeof(char) * 27);
  sprintf(str, "R_%c_0_0_", leadPlayerMoved(position) ? 'B' : 'A');
  
  STATUS status = getCardStatus(position);
  int index = 8;
  // set the first player's cards
  for (int i = 1; i <= 15; i++) {
    unsigned int mask = 0x3 << ((i-1)*2);
    unsigned int bits = (status & mask) >> ((i-1)*2);
    if (bits == 0b01) {
      str[index] = i + 96;
      index++;
    }
  }
  for (; index < 15; index++) {
    str[index] = '-';
  }
  // set the second
  for (int i = 1; i <= 15; i++) {
    unsigned int mask = 0x3 << ((i-1)*2);
    unsigned int bits = (status & mask) >> ((i-1)*2);
    if (bits == 0b10) {
      str[index] = i + 96;
      index++;
    }
  }
  int tricks = 0;
  for (; index < 22; index++) {
    str[index] = '-';
    tricks++;
  }
  
  // set the remaining information
  str[22] = getDecreeCard(position) + 96;
  if (str[2] == 'A') {
    str[23] = '-';
    str[24] = getLastCard(position) + 96;
  }
  else {
    str[23] = getLastCard(position) + 96;
    str[24] = '-';
  }
  SCORE firstscore = firstPlayerScore(position);
  str[25] = firstscore + '0';
  str[26] = tricks - firstscore + '0';

  return str;
}

/* Optional. */
STRING InteractPositionToEndData(POSITION position) {
  return NULL;
}

STRING InteractMoveToString(POSITION position, MOVE move) {
  /* YOUR CODE HERE */
  STRING str = (STRING) SafeMalloc(sizeof(char) * 5);
  STATUS status = getCardStatus(position);
  BOOLEAN firstplayer = leadPlayerMoved(position) ? FALSE : TRUE;
  int cnt = 0;
  for (int i = 1; i <= 15; i++) {
    unsigned int mask = 0x3 << ((i-1)*2);
    unsigned int bits = (status & mask) >> ((i-1)*2);
    if (firstplayer && bits == 0b01) {
      if (i == move) break;
      cnt++;
    }
    else if (!firstplayer && bits == 0b10) {
      if (i == move) break;
      cnt++;
    }
  }
  int fromindex = (firstplayer) ? cnt : (cnt + 7);
  int toindex = firstplayer ? 15 : 16;

  sprintf(str, "M_%d_%d", fromindex, toindex);

  return str;
}