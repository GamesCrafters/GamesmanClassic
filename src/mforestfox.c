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

#include "gamesman.h"

/* IMPORTANT GLOBAL VARIABLES */
CONST_STRING kAuthorName = "Jiachun Li, Yifan Zhou";
CONST_STRING kGameName = "Forest Fox"; //  use this spacing and case
CONST_STRING kDBName = "forestfox"; // use this spacing and case
POSITION gNumberOfPositions = 7000000000; // TODO: Put your number of positions upper bound here.
POSITION gInitialPosition = 0; // TODO: Put the hash value of the initial position.
BOOLEAN kPartizan = TRUE; // TODO: Is the game PARTIZAN i.e. given a board does each player have a different set of moves available to them?
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
CONST_STRING kHelpGraphicInterface = "";
CONST_STRING kHelpTextInterface = "";
CONST_STRING kHelpOnYourTurn = "";
CONST_STRING kHelpStandardObjective = "";
CONST_STRING kHelpReverseObjective = "";
CONST_STRING kHelpTieOccursWhen = /* Should follow 'A Tie occurs when... */ "";
CONST_STRING kHelpExample = "";

char * board;

TIERLIST *getTierChildren(TIER tier);
TIERPOSITION numberOfTierPositions(TIER tier);
BOOLEAN isLegal(POSITION position);

/* You don't have to change this. */
void DebugMenu() {}
/* Ignore this function. */
void SetTclCGameSpecificOptions(int theOptions[]) {
  (void)theOptions;
}
/* Do not worry about this yet because you will only be supporting 1 variant for now. */
void GameSpecificMenu() {}






/*********** BEGIN SOLVING FUNCIONS ***********/

/* The tier graph is just a single tier with id=0. */
TIERLIST *getTierChildren(TIER tier) {
  (void)tier;
  return CreateTierlistNode(0, NULL);
}

/* We use a single tier for this entire game. This
is returns the upper bound */
TIERPOSITION numberOfTierPositions(TIER tier) {
  (void)tier;
  return gNumberOfPositions;
}

/* TODO: Add a hashing function and unhashing function, if needed. */

/* Initialize any global variables or data structures needed before
solving or playing the game. */

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
BOOLEAN vcfg(int *pieces){
  //'3': decree card
  //'4': last card
  int p1 = pieces[1]+pieces[6],p2 = pieces[2]+pieces[7];
  return ((p1==p2||p1==p2-1)&&(p1<=7&&p2<=7)&&
    (pieces[5]+pieces[6]+pieces[7]+pieces[8]+pieces[9]==1))||//normal process
    (p1==7&&p2==7&&(pieces[5]+pieces[6]+pieces[7]+pieces[8]+pieces[9]==1))||//only shuffled
    (pieces[0]==15);//unshuffled
}
void hash_init(){
  int pieces_array[31]={'0',0,15,'1',0,7,'2',0,7,'3',0,1,'4',0,1,'5',0,1,'6',0,1,'7',0,1,'8',0,1,'9',0,1,-1};
  int boardsize = 15;
  board = (char *) SafeMalloc (boardsize * sizeof(char));
  gNumberOfPositions = generic_hash_init(boardsize,pieces_array,vcfg,0);
  printf("total number = %lld\n",gNumberOfPositions);
}
typedef int CARD;
typedef int SCORE;
typedef int SUIT;
typedef int NUM;
typedef POSITION STATUS;
/* Return a linked list of moves. */
BOOLEAN  leadPlayerMoved1(){
  int c1=0,c2=0;
  for(int i=0;i<15;i++)
  {
    int x=board[i]>='5'?board[i]-'5':board[i]-'0';
    if(x==1) c1++;
    if(x==2) c2++;
  }
  return c1^c2;
}
SCORE firstPlayerScore1(){
  for(int i=0;i<=7;i++)
  {
    if(board[i]>='5') return i;
  }
  return 0;
}
CARD getDecreeCard1(){//3
  for(int i=0;i<15;i++)
  {
    int x=board[i]>='5'?board[i]-'5':board[i]-'0';
    if(x==3) return i+1;
  }
  return 0;
}
CARD getLastCard1(){//4
  for(int i=0;i<15;i++)
  {
    int x=board[i]>='5'?board[i]-'5':board[i]-'0';
    if(x==4) return i+1;
  }
  return 0;
}
STATUS getCardStatus1(){
  int ret = 0;
  for(int i=0;i<15;i++)
  {
    int x=board[i]>='5'?board[i]-'5':board[i]-'0';
    if(x==1||x==2) ret|=(x<<(i*2));
  }
  return ret;
}
void getBoard(POSITION p){
  // char board[15];
  board = generic_hash_unhash(p,board);
  //printf("getBoard----------------\n");
  //for(int i=0;i<15;i++) printf("%c",board[i]);
  //printf("---------------------\n");
  //return board;
}
BOOLEAN leadPlayerMoved(POSITION p){
  getBoard(p);
  return leadPlayerMoved1();
}
SCORE firstPlayerScore(POSITION p){
  getBoard(p);
  return firstPlayerScore1();
}
CARD getDecreeCard(POSITION p){
  getBoard(p);
  return getDecreeCard1();
}
CARD getLastCard(POSITION p){
  getBoard(p);
  return getLastCard1();
}
STATUS getCardStatus(POSITION p){
  getBoard(p);
  return getCardStatus1();
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
POSITION setPositionHash(BOOLEAN moved,SCORE score,CARD decreecard,CARD lastcard,STATUS status){
  if(moved +score +decreecard+lastcard+status ==0) return 0;
  for(int i=0;i<15;i++){
    board[i]=score==i?'5':'0';
  }
  for(int i=0;i<15;i++){
    if(decreecard==i+1) board[i]+=3;
    else if(lastcard == i+1) board[i]+=4;
    else{
      board[i] += ((status>>(i*2))&3);
    }
  }
  // for(int i=0;i<15;i++) printf("%c",board[i]);
  // printf("\n");
  POSITION p = generic_hash_hash(board,1);
  // printf("set hash position = %d\n",p);
  // printf("--------------------immediate unhash\n");
  // generic_hash_unhash(p,board);
  // for(int i=0;i<15;i++) printf("%c",board[i]);
  // printf("\n-------------------\n");
  
  return p;
}

BOOLEAN isLegal(POSITION position) {
  CARD preDecreeCard = getDecreeCard(position);
  return preDecreeCard || position == 0;
}

void InitializeGame() {

  gCanonicalPosition = GetCanonicalPosition;
  gMoveToStringFunPtr = &MoveToString;
  hash_init();
  /* YOUR CODE HERE */
  gTierChildrenFunPtr = &getTierChildren;
  gNumberOfTierPositionsFunPtr = &numberOfTierPositions;
  gInitialTierPosition = gInitialPosition;
  kSupportsTierGamesman = TRUE;
  kExclusivelyTierGamesman = TRUE;
  gInitialTier = 0; 
  gIsLegalFunPtr = &isLegal;
  if (gIsInteract) {
    gLoadTierdbArray = FALSE; 
  } 
}

POSITION GetInitialPosition() {
  
  /* YOUR CODE HERE */
  return setPositionHash(0,0,0,0,0);
}
// void getPositionHash(BOOLEAN *moved,SCORE *score,CARD *decreecard,CARD *lastcard,STATUS *status,POSITION p){
//   char *board = getBoard(p);
//   moved = leadPlayerMoved(board);
//   score = firstPlayerScore(board);
//   decreecard = getDecreeCard(board);
//   lastcard = getLastCard(board);
//   status = getCardStatus(board);
// }
/*int max(int a,int b){
  return a>b?a:b;
}*/
MOVELIST *GenerateMoves(POSITION position) {
  MOVELIST *moves = NULL;
  CARD preDecreeCard = getDecreeCard(position);
  
  if (!preDecreeCard){
    //CARD a[16]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
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
    // printf("position = %d\n",position);
    //CARD decreeCard=getDecreeCard(position);
    CARD lastCard=getLastCard(position);
    STATUS status=getCardStatus(position);
    //SCORE score=firstPlayerScore(position);
    BOOLEAN moved=leadPlayerMoved(position);
    getBoard(position);
    // printf("Generating moves------------------------\n");
    // for(int i=0;i<15;i++) printf("%c",board[i]);
    // printf("\n");
    // printf("decreeCard = %d\n lastCard = %d\nstatus = %d\nscore = %d\nmoved = %d\n",decreeCard,lastCard,status,score,moved);
    //getPositionHash(&moved,&score,&decreeCard,&lastCard,&status,position);
    if(moved){
      int num = getCardNum(lastCard),suit = getCardSuit(lastCard);
      BOOLEAN hasSuit = FALSE;
      for(int i=0;i<30;i+=2){
        CARD card = i/2+1;
        int cardCol = (status>>i)&3;
        if(cardCol==2&&getCardSuit(card)==suit){
          hasSuit = TRUE;
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
  CARD preDecree = getDecreeCard(position);
  if(!preDecree){
    CARD decreeCard,lastCard;
    STATUS status;
    SCORE score;
    BOOLEAN moved;
    moved = 0;
    score = 0;
    for(int i=0;i<30;i+=2){
      CARD card = i/2+1;
      int col = (move>>i)&3;
      // printf("card %d's color = %d\n",card,col);
      if(!col) decreeCard = card;
    }
    // printf("decreeCard = %d",decreeCard);
    status = move;
    lastCard = 0;
    POSITION ret = setPositionHash(moved,score,decreeCard,lastCard,status);
    // printf("ret = %lld %x\n",ret,ret);
    // printf("decree Card of ret = %d\n",getDecreeCard(ret));
    return ret;
  }
  SCORE score = firstPlayerScore(position);
  CARD decreecard = getDecreeCard(position);
  STATUS cur_status = getCardStatus(position);
  CARD lastcard = getLastCard(position);

  //BOOLEAN swaped = FALSE;
  cur_status &= ~(3ll << (2 * (move-1)));
  // printf("cur_status = %d %x\n",cur_status,cur_status);
  BOOLEAN moved = leadPlayerMoved(position);
  // printf("moved = %d\n",moved);
  if (getCardNum(move) == 1) {
    //swaped = TRUE;
    CARD tmp = decreecard;
    decreecard = move;
    move = tmp;
  }
  // printf("move = %d\n",move);
  //SCORE add_points = getAdditionalPoint(position);
  // update card's status
  
  //cur_status |= (3ll<<(2*(move-1)));
  //cur_status &= ~(3l << (2 * (move-1)));  // play the card out

  if (!moved) {  // lead player's move
    lastcard = move;
    return setPositionHash(1, score, decreecard, lastcard, cur_status);
  }
  else {
    // to check who wins
    BOOLEAN lead_player_win;
    SUIT decreecard_suit = getCardSuit(decreecard);
    SUIT leadcard_suit = getCardSuit(lastcard);
    SUIT other_player_suit = getCardSuit(move);
    int lead_num = getCardNum(lastcard);
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
        lead_player_win = (leadcard_suit == decreecard_suit);
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
    lastcard = move;
    return setPositionHash(0, score, decreecard, lastcard, cur_status);
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
  printf("Current Location = %lld %llx\n",position,position);
  printf("Decree Card: %d\n", getDecreeCard(position));
  STATUS status = getCardStatus(position);
  printf("Current Status = %lld %llx\n",status,status);
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
  printf("LastCard = %d\n",getLastCard(position));
  printf("FirstPlayerScore = %d\n",firstPlayerScore(position));
  printf("%s\n", GetPrediction(position, playerName, usersTurn));
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
  (void)option;
}

/*********** END VARIANT-RELATED FUNCTIONS ***********/







/* Don't worry about these Interact functions below yet.
They are used for the AutoGUI which eventually we would
want to implement, but they are not needed for solving. */
POSITION InteractStringToPosition(STRING str) {
  /* YOUR CODE HERE */
  /* R_A_0_0_abdce--hijkl--o(decree card)-(first card)f(second card)3(first score)0(second score) */

  if (strcmp(str, "R_R_0_0_-------------------\0") == 0) return 0;

  BOOLEAN moved = (str[2] == 'A') ? FALSE : TRUE; 

  STATUS status = 0;
  // first check first player's cards
  for (int i = 8; i < 15; i++) {
    if (str[i] == '-') continue;
    int card = str[i] - 96;
    status |= (0b01 << 2*(card - 1));
  }
  // check the second player's cards
  for (int i = 15; i < 22; i++) {
    if (str[i] == '-') continue;
    int card = str[i] - 96;
    status |= (0b10 << 2*(card - 1));
  }

  CARD decreecard = str[22] - 96;
  CARD lastcard;
  if (str[23] == '-' && str[24] == '-') lastcard = 0;
  else {
    CARD firstcard = (str[23] != '-') ? str[23] - 96 : 0;
    CARD secondcard = (str[24] != '-') ? str[24] - 96 : 0;
    lastcard = (firstcard > 0) ? firstcard : secondcard;
  }
  
  SCORE firstscore = str[25]-'0';

  return setPositionHash(moved, firstscore, decreecard, lastcard, status);
}

STRING InteractPositionToString(POSITION position) {
  /* YOUR CODE HERE */
  /* R_A_0_0_abdce--hijkl--o(decree card)-(first card)f(second card)3(first score)0(second score) */
  
  STRING str = (STRING) SafeMalloc(sizeof(char) * 27);

  if (position == 0) {
    sprintf(str, "R_R_0_0_-------------------");
    return str;
  }

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
    CARD second_card = getLastCard(position);
    if (second_card == 0) str[24] = '-';
    else str[24] = getLastCard(position) + 96;
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

STRING InteractMoveToString(POSITION position, MOVE move) {
  /* YOUR CODE HERE */

  STRING str = (STRING) SafeMalloc(sizeof(char) * 10);

  // shuffled moves
  if (position == 0) {
    sprintf(str, "R_-_0");
    return str;
  }

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

  sprintf(str, "A_z_%d_x", fromindex + 19);

  return str;
}