#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include "gamesman.h"

#define TRUE    1
#define FALSE   0
#define FREQTABLESIZE   256
#define HUFFTREESIZE    550
/* reserved value for parent, lChild, rChild fields */
#define NONE            -1
#define lessThan    -1
#define equalTo      0
#define greaterThan  1
#define MAGIC        0xC0FFEE
#define mMax(A,B) ((A) > (B) ? (A) : (B))
#define mMin(A,B) ((A) > (B) ? (B) : (A))

typedef struct
{
	int nodeUsed;
	int character;
	long frequency;
	int parent, lChild, rChild;
}HuffTreeNode;

typedef struct
{
	HuffTreeNode tree[HUFFTREESIZE];
	int encoderLUT[256];
	int nodeCount;
	int rootNode;
}HuffTree;

STRING   kOpeningCredits =
"\nWelcome to GAMESMAN, version %s, written by Dan Garcia.\n\n\
	(G)ame-independent\n\
	(A)utomatic\n\
	(M)ove-tree\n\
	(E)xhaustive\n\
	(S)earch,\n\
	(M)aniuplation\n\
	(A)nd\n\
	(N)avigation\n\n\
This program will determine the value of %s, and allow you to play\n\
the computer or another human. Have fun!\n";
  
STRING   kHelpValueBeforeEvaluation =
"At this point, the computer does not know the value of the game\n\
because you have not chosen to EVALUATE it. Once you choose to\n\
do that, the value will be determined told to you. Then you can \n\
bring this help menu back up to find out what it means.";

STRING   kHelpWhatIsGameValue =
"A game VALUE is one of either WIN, LOSE, or TIE.  That is, if a game \n\
is played by two perfect opponents, the following ALWAYS happens:\n\
(A perfect opponent never makes a bad move, and only loses if there is\n\
NOTHING he/she can do about it.)\n\n\
WIN:\n\n\
The player who goes first always wins.\n\n\
LOSE:\n\n\
The player who goes first always loses.\n\n\
TIE:\n\n\
The player who goes first cannot force a win, but also cannot be\n\
forced into losing. The game always ends in a draw. It was a TIE\n\
game that caused a character from the movie 'War Games' to comment:\n\n\
	       The only way to win is not to play at all.\n\n\
The reason it is important to know the value of the game is that it\n\
determines whether going first or second is better. If the game is a\n\
WIN game, it is better to go first. Why? Well, theoretically, it is\n\
possible to win, regardless of what your opponent does. This applies\n\
equally to going second while playing a LOSE game. It doesn't mean that\n\
the person going first with a WIN game will ALWAYS win, it just says\n\
the potential exists. In GAMESMAN, the computer plays the part of the\n\
perfect opponent, but gives you, the human opponent the advantage. If\n\
you make a mistake, the computer pounces on you.";

STRING   kHelpWhatIsEvaluation =
"Evaluation is the process the computer undergoes to determine the value\n\
of the game. (See 'What is a game VALUE?', the previous topic, for more)\n\
The computer searches the MOVE-TREE (the connected tree of all the moves\n\
that are possible from a certain position that lead to other positions)\n\
recursively (Depth-First Search, for the computer literate) until it \n\
reaches a position which it considers a PRIMITIVE win, lose or draw.\n\
\n\
A PRIMITIVE position is defined by the game itself. For example, with\n\
Tic-tac-toe, if the board is full, that is considered a primitive TIE.\n\
If a position has 3-in-a-row of X, then that position is considered a \n\
primitive LOSE position, because that means player O, who is staring\n\
blankly at the board, has just LOST. Once a primitive position has been\n\
reached, the algorithm backtracks in an attempt to determine the value\n\
of THAT position. It uses the following scheme to figure out the value\n\
of a non-primitive position:\n\
\n\
A non-primitive WIN position is one in which THERE EXISTS AT LEAST ONE\n\
move that will give my opponent a LOSE position.\n\
\n\
A non-primitive LOSE position is one in which ALL MOVES that are possible\n\
will give my opponent a WIN position.\n\
\n\
A non-primitive TIE position is one in which there are NO MOVES which\n\
will give my opponent a LOSE position, but there are SOME MOVES which\n\
will give my opponent another TIE position. This translates to: \n\
'Well, I can't win, but at least my opponent can win either'.\n\
\n\
The algorithm continues until all positions that can be reached from\n\
the starting position (the blank board in Tic-Tac-Toe, for instance)\n\
have had their value determined, and in the meantime the computer has\n\
figured out PERFECT paths for itself that always force you to LOSE.";

STRING   kHelpWhatArePredictions =
"Predictions are the words in parenthesis you see when you are playing\n\
the game that 'predict' who will win and who will lose. Since the\n\
computer is a perfect opponent, if, while playing, it discovers it has\n\
a WIN position, there's nothing you can do, so the prediction is: 'Player\n\
will Lose' and 'Computer will Win'. However, if you have the WIN\n\
position, it's always possible for you to make a really silly move and\n\
give the COMPUTER the WIN position, so the prediction says: 'Player\n\
should Win' and 'Computer should Lose'.";

STRING   kHelpWhatAreHints =
"Hints are a list of moves that are VALUE-wise equivalent. For example,\n\
if you have a WIN position, then all the moves that will give your\n\
opponent a LOSE position are listed in the hints. If you have a LOSE\n\
position, ALL the possible moves are listed in the hints. The reason\n\
for this is explained in the help topic: 'What is EVALUATION?'. If you\n\
have a TIE position, the moves that give your opponent a TIE position\n\
are listed in the hints.";

STRING  kHandleDefaultTextInputHelp = 
"\n\
Text Input Commands:\n\
-------------------\n\
?           : Brings up this list of Text Input Commands available\n\
s (or S)    : (S)how the values of all possible moves\n\
u (or U)    : (U)ndo last move (not possible at beginning position)\n\
r (or R)    : (R)eprint the position\n\
h (or H)    : (H)elp\n\
a (or A)    : (A)bort the game\n\
c (or C)    : Adjust (C)omputer's brain\n\
q (or Q)    : (Q)uit";


//// Gameline Information
int gameline = 0 ;
int sockfd = -1 ;

static VALUE   gValue;                  /* The value of the game */
static BOOLEAN gAgainstComputer;        /* TRUE iff the user is playing the computer */
static BOOLEAN gHumanGoesFirst;         /* TRUE iff the user goes first vs. computer */
BOOLEAN gStandardGame = TRUE;           /* TRUE iff game is STANDARD (not REVERSE) */
static BOOLEAN gPrintPredictions ;       /* TRUE iff the predictions should be printed */
static BOOLEAN gHints;          	 /* TRUE iff possible moves should be printed */
char    gPlayerName[2][MAXNAME] = {"", ""}; /* The names of the players user/user or comp/user */
VALUE * gDatabase = NULL;
STRING kSolveVersion = "3.02.03" ;    /* This will be valid for the next hundred years hehehe */

int smartness = SMART;
int scalelvl = MAXSCALE;
int remainingGivebacks = 0;
int initialGivebacks = 0;
int oldValueOfPosition = tie;

static MENU gMenuMode ;
BOOLEAN gPrintHints = FALSE ;
STRING kAuthorName = "Dan Garcia" ;
extern POSITION kBadPosition;          /* A POSITION that will never be used */

/* Start Loopy */
FRnode *gHeadWinFR = NULL;               /* The FRontier Win Queue */
FRnode *gTailWinFR = NULL;
FRnode *gHeadLoseFR = NULL;              /* The FRontier Lose Queue */
FRnode *gTailLoseFR = NULL;
FRnode *gHeadTieFR = NULL;               /* The FRontier Tie Queue */
FRnode *gTailTieFR = NULL;
POSITIONLIST **gParents = NULL;         /* The Parent of each node in a list */
char *gNumberChildren = NULL;           /* The Number of children (used for Loopy games) */
/* End Loopy */

char *gValueString[] =
{
	"Win", "Lose", "Tie", "Undecided",
	"Win-Visited", "Lose-Visited", "Tie-Visited", "Undecided-Visited"
};

/*************************************************************************
**
** Everything below here references variables that must be in every game file
**
**************************************************************************/

extern STRING   kHelpGraphicInterface; /* The Graphical Interface Help string. */
extern STRING   kHelpTextInterface;    /* The Help for Text Interface string. */
extern STRING   kHelpOnYourTurn;       /* The Help for Your turn string. */
extern STRING   kHelpStandardObjective;/* The Help for Objective string. */
extern STRING   kHelpReverseObjective; /* The Help for reverse Objective string. */
extern STRING   kHelpTieOccursWhen;    /* The Help for Tie occuring string. */
extern STRING   kHelpExample;          /* The Help for Exmaples string. */
extern STRING   kAuthorName;	       /* Name of the Author ... */

extern POSITION gInitialPosition;      /* The initial position of the game */
extern POSITION gMinimalPosition;
extern BOOLEAN  kPartizan;             /* TRUE <==> module is a Partizan game */
extern BOOLEAN  kGameSpecificMenu;     /* TRUE <==> module supports GameSpecificMenu() */
extern BOOLEAN  kTieIsPossible;        /* TRUE <==> A Tie is possible */
extern BOOLEAN  kLoopy;                /* TRUE <==> Game graph has cycles */
extern int      gNumberOfPositions;    /* The number of positions in the game */
extern BOOLEAN  gPrintHints ;
extern STRING   kGameName ;
extern STRING   kAuthorName ;
extern BOOLEAN  kDebugMenu ;
extern STRING   kDBName ;
extern BOOLEAN  kDebugDetermineValue ;

// for atilla's hash code ...
int *gHashOffset = NULL ;
int *gNCR = NULL ;
static int gHashOffsetSize;
static int gHashBoardSize;
static int gHashMinMax[4];
static int gHashNumberOfPos;
// end for atilla's hash code

BOOLEAN DefaultGoAgain(POSITION pos,MOVE move)
{
  return FALSE; /* Always toggle turn by default */
}

BOOLEAN (*gGoAgain)(POSITION,MOVE)=DefaultGoAgain;

char GetMyChar()
{
  char inString[MAXINPUTLENGTH], ans;
  scanf("%s",inString);
  ans = inString[0];
  return(ans);
}

InitializeDatabases()
{
  GENERIC_PTR SafeMalloc();
  int i;
  if (gDatabase!=NULL) {
    SafeFree((GENERIC_PTR) gDatabase);
    gDatabase = NULL;
    //    gBytesInUse -= gNumberOfPositions * sizeof(VALUE);
  }

  gDatabase = (VALUE *) SafeMalloc (gNumberOfPositions * sizeof(VALUE));
  for(i = 0; i < gNumberOfPositions; i++)
    gDatabase[i] = undecided;
}

Initialize()
{
  srand(time(NULL));

  gValue            = undecided;
  gMenuMode         = BeforeEvaluation;
  gAgainstComputer  = TRUE;
  gStandardGame     = TRUE;
  gPrintPredictions   = TRUE;
  gPrintHints = TRUE;

  sprintf(gPlayerName[kPlayerOneTurn],"Player");
  sprintf(gPlayerName[kPlayerTwoTurn],"Computer");

  InitializeGame() ;
}


Menus()
{
  printf(kOpeningCredits,kSolveVersion,kGameName);

  HitAnyKeyToContinue();
  
  do {
    printf("\n\t----- GAMESMAN version %s with %s module -----\n", 
	   kSolveVersion, kGameName);
    
    if(gMenuMode == BeforeEvaluation)
      MenusBeforeEvaluation();
    else if(gMenuMode == Evaluated)
      MenusEvaluated();

    printf("\n\th)\t(H)elp.\n");
    printf("\n\n\tq)\t(Q)uit.\n");
    printf("\n\nSelect an option: ");
    
    ParseMenuChoice(GetMyChar());
  } while(TRUE);
}

MenusBeforeEvaluation()
{
  printf("\n\ts)\t(S)TART THE GAME\n");

  printf("\n\tEvaluation Options:\n\n");
  printf("\to)\t(O)bjective toggle from %s to %s\n",
	 gStandardGame ? "STANDARD" : "REVERSE ",
	 gStandardGame ? "REVERSE " : "STANDARD");
  if(kDebugMenu)
    printf("\td)\t(D)ebug Module BEFORE Evaluation\n");
  if(kGameSpecificMenu)
    printf("\tg)\t(G)ame-specific options for %s\n",kGameName);
}

MenusEvaluated()
{
  printf("\n\tPlayer Name Options:\n\n");
      
  printf("\t1)\tChange the name of player 1 (currently %s)\n",gPlayerName[1]);
  printf("\t2)\tChange the name of player 2 (currently %s)\n",gPlayerName[0]);
  printf("\t3)\tSwap player 1 (plays FIRST) with player 2 (plays SECOND)\n");
      
  printf("\n\tGeneric Options:\n\n");

  printf("\t4)\tToggle from %sPREDICTIONS to %sPREDICTIONS\n",
	 gPrintPredictions ? "   " : "NO ",
	 !gPrintPredictions ? "   " : "NO ");
  printf("\t5)\tToggle from %sHINTS       to %sHINTS\n",
	 gHints ? "   " : "NO ",
	 !gHints ? "   " : "NO ");
  
  printf("\n\tPlaying Options:\n\n");
  printf("\t6)\tToggle opponent from a %s to a %s\n",
	 gAgainstComputer ? "COMPUTER" : "HUMAN",
	 gAgainstComputer ? "HUMAN" : "COMPUTER");
  if(gAgainstComputer )
  {
    if(gValue == tie)
      printf("\t7)\tToggle from going %s (can tie/lose) to %s (can tie/lose)\n",
	     gHumanGoesFirst ? "FIRST" : "SECOND",
	     gHumanGoesFirst ? "SECOND" : "FIRST");
    else if (gValue == lose)
      printf("\t7)\tToggle from going %s (can %s) to %s (can %s)\n",
	     gHumanGoesFirst ? "FIRST" : "SECOND",
	     gHumanGoesFirst ? "only lose" : "win/lose",
	     gHumanGoesFirst ? "SECOND" : "FIRST",
	     gHumanGoesFirst ? "win/lose" : "only lose");
    else if (gValue == win)
      printf("\t7)\tToggle from going %s (can %s) to %s (can %s)\n",
	     gHumanGoesFirst ? "FIRST" : "SECOND",
	     gHumanGoesFirst ? "win/lose" : "only lose",
	     gHumanGoesFirst ? "SECOND" : "FIRST",
	     gHumanGoesFirst ? "only lose" : "win/lose");
    else
      BadElse("Menus");
    
    printf("\n\ta)\t(A)nalyze the game\n");
  }
  printf("\tc)\tAdjust (C)omputer's brain (currently ");
  if (smartness==SMART) {
    printf("%d%% perfect", scalelvl);
  }
  else if (smartness==DUMB) {
    printf("misere-ly");
  }
  else if (smartness==RANDOM) {
    printf("randomly");
  }
  printf (" w%/%d givebacks)\n", initialGivebacks);
  
  if(kDebugMenu)
    printf("\td)\t(D)ebug Game AFTER Evaluation\n");
  printf("\n\tp)\t(P)LAY GAME.\n");

  printf("\n\tm)\tGo to (M)ain Menu to edit game rules or starting position.\n");
}

ParseMenuChoice(c)
char c;
{
  BOOLEAN ParseConstantMenuChoice();	

  if (ParseConstantMenuChoice(c));
  else if(gMenuMode == BeforeEvaluation)
    ParseBeforeEvaluationMenuChoice(c);
  else if(gMenuMode == Evaluated)
    ParseEvaluatedMenuChoice(c);
  else {
    BadElse("ParseMenuChoice");
    HitAnyKeyToContinue();
  }
}

BOOLEAN ParseConstantMenuChoice(c)
     char c;
{
  switch(c) {
  case 'Q': case 'q':
    ExitStageRight();
  case 'h': case 'H':
    HelpMenus();
    break;
  default:
    return(FALSE);  /* It was not parsed here */
  }
  return(TRUE);       /* Yep, it was parsed here! */
}

ParseBeforeEvaluationMenuChoice(c)
     char c;
{
  VALUE DetermineValue(), DetermineLoopyValue();
  BOOLEAN tempPredictions ;

  switch(c) {
  case 'G': case 'g':
    if(kGameSpecificMenu)
    {
      tempPredictions = gPrintPredictions ;
      gPrintPredictions = FALSE ;
      GameSpecificMenu();
      gPrintPredictions = tempPredictions ;
    }
    else {
      BadMenuChoice();
      HitAnyKeyToContinue();
    }
    break;
  case 'o': case 'O':
    gStandardGame = !gStandardGame;
    break;
  case 'D': case 'd':
    if(kDebugMenu)
      DebugModule();
    else
      BadMenuChoice();
    break;
  case 's': case 'S':
      printf("\nSolving with loopy code %s...%s!",kGameName,kLoopy?"Yes":"No");
      printf("\nRandom(100) three times %s...%d %d %d",kGameName,GetRandomNumber(100),GetRandomNumber(100),GetRandomNumber(100));
      printf("\nInitializing insides of %s...", kGameName);
      /*      Stopwatch(&sec,&usec);*/
      (void) Stopwatch();
      InitializeDatabases();
      printf("done in %d seconds!", Stopwatch());

      printf("\nEvaluating the value of %s...", kGameName);

      if (kLoopy)
	gValue = DetermineLoopyValue(gInitialPosition);
      else
	gValue = DetermineValue(gInitialPosition);

      printf("done in %d seconds!", Stopwatch());

      
      
      printf("\n\nThe Game %s has value: %s\n\n", kGameName, gValueString[(int)gValue]);
      gMenuMode = Evaluated;
	if(gValue == lose)
		gHumanGoesFirst = FALSE;
	else
		gHumanGoesFirst = TRUE ;
    HitAnyKeyToContinue();
    break;
  default:
    BadMenuChoice();
    HitAnyKeyToContinue();
    break;
  }
}

ParseEvaluatedMenuChoice(c)
     char c;
{
  char tmpName[MAXNAME];
  
  switch(c) {
  case '1':
    printf("\nEnter the name of player 1 (max. %d chars) [%s] : ",
		MAXNAME-1, gPlayerName[kPlayerOneTurn]);
    GetMyString(tmpName,MAXNAME,TRUE,FALSE);
	if(strcmp(tmpName,""))
    	(void) sprintf(gPlayerName[kPlayerOneTurn],"%s",tmpName);
    break;
  case '2':
    printf("\nEnter the name of player 2 (max. %d chars) [%s] : ",
	        MAXNAME-1, gPlayerName[kPlayerTwoTurn]);
    GetMyString(tmpName,MAXNAME,TRUE,FALSE);
	if(strcmp(tmpName,""))
    	(void) sprintf(gPlayerName[kPlayerTwoTurn],"%s",tmpName);
    break;
  case '3':
    (void) sprintf(tmpName,"%s",gPlayerName[kPlayerOneTurn]);
    (void) sprintf(gPlayerName[kPlayerOneTurn],"%s",gPlayerName[kPlayerTwoTurn]);
    (void) sprintf(gPlayerName[kPlayerTwoTurn],"%s",tmpName);
    break;
  case '4':
    gPrintPredictions = !gPrintPredictions;
    break;
  case '5':
    gHints = !gHints;
    break;
  case '6':
    gAgainstComputer = !gAgainstComputer;
    break;
  case '7':
    if(gAgainstComputer)
      gHumanGoesFirst = !gHumanGoesFirst;
    else {
      BadMenuChoice();
      HitAnyKeyToContinue();
    }
    break;
  case 'D': case 'd':
    if(kDebugMenu)
      DebugMenu();
    else {
      BadMenuChoice();
      HitAnyKeyToContinue();
    }
    break;
  case 'C': case 'c':
    SmarterComputerMenu();
    break;
  case 'A': case 'a':
    AnalysisMenu();
    break;
  case 'p': case 'P':
    if(gAgainstComputer)
      PlayAgainstComputer();
    else
      PlayAgainstHuman();
    HitAnyKeyToContinue();
    break;
  case 'm': case 'M':
    gMenuMode = BeforeEvaluation;
    break;
  default:
    BadMenuChoice();
    HitAnyKeyToContinue();
    break;
  }
}

HelpMenus()
{
  char c, GetMyChar();

  do {
    printf("\n\t----- HELP for %s module -----\n\n", kGameName);
		
    printf("\t%s Help:\n\n",kGameName);

    printf("\t1)\tWhat do I do on MY TURN?\n");
    printf("\t2)\tHow do tell the computer WHICH MOVE I want?\n");
    printf("\t3)\tWhat is the %s OBJECTIVE of %s?\n", 
	   gStandardGame ? "STANDARD" : "REVERSE", kGameName);
    printf("\t4)\tIs a TIE possible?\n");
    printf("\t5)\tWhat does the VALUE of this game mean?\n");
    printf("\t6)\tShow SAMPLE %s game.\n",kGameName);

    printf("\n\tGAMESMAN Help:\n\n");

    printf("\t7)\tWhat is a game VALUE?\n");
    printf("\t8)\tWhat is EVALUATION?\n");

    printf("\n\tGeneric Options Help:\n\n");

    printf("\t9)\tWhat are PREDICTIONS?\n");
    printf("\t0)\tWhat are HINTS?\n");

    printf("\n\n\tb)\t(B)ack = Return to previous activity.\n");
    printf("\n\nSelect an option: ");

    ParseHelpMenuChoice(c = GetMyChar());

  } while(c != 'b' && c != 'B');
}

ParseHelpMenuChoice(c)
     char c;
{
  switch(c) {
  case 'Q': case 'q':
    ExitStageRight();
  case '1':
    printf("\n\t----- What do I do on MY TURN? -----\n\n");
    printf("%s\n",kHelpOnYourTurn);
    break;
  case '2':
    printf("\n\t----- How do I tell the computer WHICH MOVE I want? -----\n\n");
    printf("%s\n",kHelpTextInterface);
    break;
  case '3':
    printf("\n\t----- What is the %s OBJECTIVE of %s? -----\n\n", 
	   gStandardGame ? "STANDARD" : "REVERSE", kGameName);
    printf("%s\n",
	   gStandardGame ? kHelpStandardObjective : kHelpReverseObjective);
    break;
  case '4':
    printf("\n\t----- Is a TIE possible? -----\n\n");
    printf("%s\n",!kTieIsPossible ? "NO, a tie is not possible in this game." :
	   "YES, a tie is possible in this game. A tie occurs when");
    if(kTieIsPossible)
      printf("%s\n",kHelpTieOccursWhen);
    break;
  case '5':
    printf("\n\t----- What does the VALUE of %s mean? -----\n\n",kGameName);
    if(gMenuMode == Evaluated) {
      if(gAgainstComputer)
	PrintComputerValueExplanation();
      else
	PrintHumanValueExplanation();
    }
    else {
      printf("%s\n",kHelpValueBeforeEvaluation);
    }
    break;
  case '6':
    printf("\n\t----- Show SAMPLE %s game {Your answers in curly brackets} -----\n\n",kGameName);
    printf("%s\n",kHelpExample);
    break;
  case '7':
    printf("\n\t----- What is a game VALUE? -----\n\n");
    printf("%s\n",kHelpWhatIsGameValue);
    break;
  case '8':
    printf("\n\t----- What is EVALUATION? -----\n\n");
    printf("%s\n",kHelpWhatIsEvaluation);
    break;
  case '9':
    printf("\n\t----- What are PREDICTIONS? -----\n\n");
    printf("%s\n",kHelpWhatArePredictions);
    break;
  case '0':
    printf("\n\t----- What are HINTS? -----\n\n");
    printf("%s\n",kHelpWhatAreHints);
    break;
  case 'b': case 'B':
    return;
  default:
    BadMenuChoice();
    break;
  }
  HitAnyKeyToContinue();
}

BadMenuChoice()
{
  printf("\nSorry, I don't know that option. Try another.\n");
}

DebugModule()
{
  char GetMyChar();
  int numberMoves = 0, move;
  POSITION DoMove(), GetInitialPosition();
  VALUE Primitive();
  MOVELIST *head = NULL, *ptr, *GenerateMoves();
  MOVE theMove;
  BOOLEAN haveMove = FALSE, tempPredictions = gPrintPredictions;
  USERINPUT GetAndPrintPlayersMove();

  gPrintPredictions = FALSE;
  
  do {
    printf("\n\t----- Module DEBUGGER for %s -----\n\n", kGameName);
    
    printf("\tCurrent Position:\n");
    PrintPosition(gInitialPosition, gPlayerName[kPlayerOneTurn], kHumansTurn);
    
    printf("\t1)\tGet the debugger initial position (GetInitialPosition)\n");
    printf("\t2)\tPrint what PRIMITIVE() would return for this position\n"); 
    printf("\t3)\tPrint the list of moves available for this position\n"); 
    printf("\t4)\tChoose a move (GetAndPrintPlayersMove)\n");
    if (haveMove)
      printf("\t5)\tDo that Move (DoMove)\n");
    
    printf("\n\n\tb)\t(B)ack = Return to previous activity.\n");
    printf("\n\nSelect an option: ");
    
    switch(GetMyChar()) {
    case 'Q': case 'q':
      ExitStageRight();
    case 'H': case 'h':
      HelpMenus();
      break;
    case '1':
      gInitialPosition = GetInitialPosition();
      break;
    case '2':
      printf("\nPrimitive Returns: %s\n",gValueString[Primitive(gInitialPosition)]);
      break;
    case '3':
      printf("\nThe list of available moves is:\n");
      head = ptr = GenerateMoves(gInitialPosition);
      numberMoves = 0;
      while(ptr != NULL) {
	numberMoves++;
	printf("%2d : ",numberMoves);
	PrintMove(ptr->move);
	printf("\n");
	ptr = ptr->next;
      }
      FreeMoveList(head);
      break;
    case '4':
      printf("\n");
      (void) GetAndPrintPlayersMove(gInitialPosition, &theMove, gPlayerName[kPlayerOneTurn]);
      printf("You Chose: ");
      PrintMove(theMove);
      haveMove = TRUE;
      break;
    case '5':
      if(haveMove) {
	gInitialPosition = DoMove(gInitialPosition, theMove);
	haveMove = FALSE;
      }
      else
	BadMenuChoice();
      break;
    case 'b': case 'B':
      gPrintPredictions = tempPredictions;
      return;
    default:
      BadMenuChoice();
      HitAnyKeyToContinue();
      break;
    }
  } while(TRUE);
  
}

PlayAgainstHuman()
{
  POSITION currentPosition;
  MOVE theMove;
  VALUE Primitive();
  UNDO *undo, *InitializeUndo(), *HandleUndoRequest(), *UpdateUndo();
  BOOLEAN playerOneTurn = TRUE, error, player_draw;
  USERINPUT userInput, GetAndPrintPlayersMove();

  currentPosition = gInitialPosition;
  undo = InitializeUndo();

#ifndef X
  printf("Type '?' if you need assistance...\n\n");
#endif

  PrintPosition(currentPosition, gPlayerName[playerOneTurn], kHumansTurn);

  while(Primitive(currentPosition) == undecided) { /* Not dead yet! */

    while((userInput = GetAndPrintPlayersMove(currentPosition,&theMove,
					      gPlayerName[playerOneTurn])) == Undo) {
      undo = HandleUndoRequest(&currentPosition,undo,&error);
      if(!error)
	playerOneTurn = !playerOneTurn;		/* the other's turn */
      PrintPosition(currentPosition,gPlayerName[playerOneTurn], kHumansTurn);
    }
    if(userInput == Abort)
      break;                 /* jump out of while loop */

    if (!gGoAgain(currentPosition,theMove))
      playerOneTurn = !playerOneTurn;           /* the other's turn */

    /* It's !playerOneTurn because the text (Player will lose) is for the other */
    PrintPosition(currentPosition = DoMove(currentPosition,theMove),
		  gPlayerName[playerOneTurn], kHumansTurn);

    undo = UpdateUndo(currentPosition, undo, &player_draw);
    if(player_draw)
      break;

  }
  if(Primitive(currentPosition) == tie)
    printf("The match ends in a draw. Excellent strategies, %s and %s.\n\n", 
	   gPlayerName[0], gPlayerName[1]);
  else if(Primitive(currentPosition) == lose)
    printf("\n%s (player %s) Wins!\n\n", gPlayerName[!playerOneTurn], 
	   playerOneTurn ? "two" : "one"); 
  else if(Primitive(currentPosition) == win)
    printf("\n%s (player %s) Wins!\n\n", gPlayerName[playerOneTurn], 
	   playerOneTurn ? "one" : "two");
  else if(userInput == Abort)
    printf("Your abort command has been received and successfully processed!\n");
  else if (player_draw == TRUE) { /* Player chooses to end the game in a draw */
    printf("The match ends in a draw.  Excellent strategy, %s and %s. \n\n",
	   gPlayerName[0], gPlayerName[1]);
  }
  else
    BadElse("PlayAgainstHuman"); 

  ResetUndoList(undo);
}

PlayAgainstComputer()
{
  POSITION thePosition;
  MOVE theMove, GetComputersMove();
  VALUE Primitive();
  UNDO *undo, *InitializeUndo(), *HandleUndoRequest(), *UpdateUndo();
  BOOLEAN usersTurn, error, player_draw;
  USERINPUT userInput, GetAndPrintPlayersMove();
  int oldRemainingGivebacks;

  thePosition = gInitialPosition;
  remainingGivebacks = initialGivebacks;
  undo = InitializeUndo();
  usersTurn = gHumanGoesFirst;

  printf("\nOk, %s and %s, let us begin.\n\n",
	 gPlayerName[kPlayerOneTurn], gPlayerName[kPlayerTwoTurn]);

#ifndef X
  printf("Type '?' if you need assistance...\n\n");
#endif

  PrintPosition(thePosition,gPlayerName[usersTurn],usersTurn);

  while(Primitive(thePosition) == undecided) { /* Not dead yet! */

    oldRemainingGivebacks = remainingGivebacks; /* keep track of giveback usage for undo */

    if(usersTurn) {		/* User's turn */

      while((userInput = GetAndPrintPlayersMove(thePosition,
						&theMove, 
						gPlayerName[usersTurn])) == Undo) {
	undo = HandleUndoRequest(&thePosition,undo,&error); /* undo */
	PrintPosition(thePosition,gPlayerName[usersTurn],usersTurn);
      }
    }
    else {				/* Computer's turn */
      theMove = GetComputersMove(thePosition);
      PrintComputersMove(theMove,gPlayerName[usersTurn]);
    }
    if(userInput == Abort)
      break;                 /* jump out of while loop */

    if (!gGoAgain(thePosition,theMove))
      usersTurn = !usersTurn;           /* The other person's turn */

    PrintPosition(thePosition = DoMove(thePosition,theMove),
		  gPlayerName[usersTurn],usersTurn);

    undo = UpdateUndo(thePosition, undo, &player_draw);
    undo->givebackUsed = oldRemainingGivebacks>remainingGivebacks;
    if(player_draw)
      break;

  }
  if((Primitive(thePosition) == lose && usersTurn) || 
     (Primitive(thePosition) == win && !usersTurn))
    printf("\n%s wins. Nice try, %s.\n\n", gPlayerName[kComputersTurn],
	   gPlayerName[kHumansTurn]); 
  else if((Primitive(thePosition) == lose && !usersTurn) ||
	  (Primitive(thePosition) == win && usersTurn))
    printf("\nExcellent! You won!\n\n");
  else if(Primitive(thePosition) == tie)
    printf("The match ends in a tie. Excellent strategy, %s.\n\n", 
	   gPlayerName[kHumansTurn]);
  else if(userInput == Abort)
    printf("Your abort command has been received and successfully processed!\n");
  else if (player_draw == TRUE) { /* Player chooses to end the game in a draw */
    printf("The match ends in a draw.  Excellent strategy, %s. \n\n",
	   gPlayerName[kHumansTurn]);
  }
  else
    BadElse("PlayAgainstHuman"); 

  ResetUndoList(undo);
}

ResetUndoList(undo)
     UNDO *undo;
{
  POSITION position;
  BOOLEAN error, oldAgainstComputer; /* kludge so that it resets everything */
  UNDO *HandleUndoRequest();

  oldAgainstComputer = gAgainstComputer;
  gAgainstComputer = FALSE;
  while(undo->next != NULL)
    undo = HandleUndoRequest(&position, undo, &error);
  UnMarkAsVisited(undo->position);
  SafeFree((GENERIC_PTR)undo);
  gAgainstComputer = oldAgainstComputer;
}

UNDO *HandleUndoRequest(thePosition, undo, error)
     POSITION *thePosition;
     UNDO *undo;
     BOOLEAN *error;
{
  UNDO *tmp;

  if(*error = ((undo->next == NULL) ||
	       (gAgainstComputer && (undo->next->next == NULL)))) {

    printf("\nSorry - can't undo, I'm already at beginning!\n");
    return(undo);
  }

  /* undo the first move */

  UnMarkAsVisited(undo->position);
  if (undo->givebackUsed) {
    remainingGivebacks++;
  }
  tmp = undo;
  undo = undo->next;
  SafeFree((GENERIC_PTR)tmp);
  *thePosition = undo->position;

  /* If playing against the computer, undo the users move here */

  if(gAgainstComputer) {
    UnMarkAsVisited(undo->position);
    tmp = undo;
    undo = undo->next;
    SafeFree((GENERIC_PTR)tmp);
    *thePosition = undo->position;
  }

  return(undo);
}

UNDO *UpdateUndo(thePosition, undo, abort)
     POSITION thePosition;
     UNDO *undo;
     BOOLEAN *abort;
{
  BOOLEAN Visited();
  UNDO *tmp, *Stalemate();
  GENERIC_PTR SafeMalloc();

  if(Visited(thePosition)) 
    undo = Stalemate(undo,thePosition,abort);
  else {
    MarkAsVisited(thePosition);
    tmp = undo; 
    undo = (UNDO *) SafeMalloc (sizeof(UNDO));
    undo->position = thePosition;
    undo->givebackUsed = FALSE; /* set this in PlayAgainstComputer */
    undo->next = tmp;

    *abort = FALSE;
  }
  return(undo);
}

UNDO *InitializeUndo()
{
  UNDO *undo;
  GENERIC_PTR SafeMalloc();

  undo = (UNDO *) SafeMalloc (sizeof(UNDO));    /* Initialize the undo list */
  undo->position = gInitialPosition;
  undo->givebackUsed = FALSE;
  undo->next = NULL;
  return(undo);
}

PrintHumanValueExplanation()
{
  if(gValue == tie) {
    printf("Since this game is a TIE game, the following should happen. The player\n");
    printf("who goes first should not be able to win, because the second person\n");
    printf("should always be able to force a tie. It is possible for either player\n");
    printf("to WIN, but it is easier for the first player to do so. Thus, you may\n");
    printf("wish to have the stronger player play SECOND and try to steal a win.\n\n");
  }
  else {
    printf("You should know that since this is a %s game, the player\n",
	   gValueString[(int)gValue]);
    printf("who goes first _should_ %s. Thus, when playing against each other,\n", 
	   gValueString[(int)gValue]);
    printf("it is usually more enjoyable to have the stronger player play %s,\n",
	   gValue == lose ? "first" : "second");
    printf("and try to take the lead and then win the game. The weaker player\n");
    printf("tries to hold the lead and not give it up to the stronger player.\n\n");
  }
}

PrintComputerValueExplanation()
{
  if(gValue == tie) {
    printf("You should know that since this is a TIE game, the player who goes\n");
    printf("first *cannot* win unless the second player makes a mistake. Thus, when\n");
    printf("playing against me, the computer, there are two options as to how to\n");
    printf("play the game. You can either play first and if you tie, consider it a\n");
    printf("'win'. However, you *can* lose!  Or you can play second and if you tie,\n");
    printf("also consider it a 'win'. You can lose here *also*.\n\n");
  }
  else if(gValue == win || gValue == lose) {
    printf("You should know that since this is a %s game, the player who goes\n",
	   gValueString[(int)gValue]);
    printf("first *should* %s. Thus, you have two options:\n", 
	   gValueString[(int)gValue]);
    printf("\n");
    printf("A. Choose to play %s.\n", gValue == lose ? "FIRST" : "SECOND");
    printf("\n");
    printf("   Before you choose this, you must understand that I CANNOT lose.\n");
    printf("   Some find this option helpful when they get really headstrong and\n");
    printf("   think they cannot lose a certain game. If you have any friends who\n");
    printf("   think they're unbeatable, have them choose this option. As Bill\n");
    printf("   Cosby would say: 'The game will beat the snot out of him'. :-)\n");
    printf("\n");
    printf("B. Choose to play %s.\n", gValue == win ? "FIRST" : "SECOND");
    printf("\n");
    printf("   In this mode, you CAN win if you choose the right moves, but it's\n");
    printf("   hard to do that with certain games. This is analogous to crossing\n");
    printf("   a mine-field, where every move you take is a step forward in the field.\n");
    printf("   It IS possible to cross unscathed (win the game), but if you make ONE\n");
    printf("   wrong move, the hint will say '<computer> WILL win' and then\n");
    printf("   there's NO WAY for you to win. At this point, you may choose to (u)ndo,\n");
    printf("   which will return you to you previous winning position.\n");
  }
  else
    BadElse("PrintComputerValueExplanation");
}

UNDO *Stalemate(undo,stalematePosition, abort)
     UNDO *undo;
     POSITION stalematePosition;
     BOOLEAN *abort;
{
  char GetMyChar();
  UNDO *tmp;

  printf("\nWe have reached a position we have already encountered. We have\n");
  printf("achieved a STALEMATE. Now, we could go on forever playing like this\n");
  printf("or we could just stop now. Should we continue (y/n) ? ");

  if(GetMyChar() != 'y') {      /* quit */
    *abort = TRUE;
  }
  else {
    while(undo->next != NULL && undo->position != stalematePosition) {
      UnMarkAsVisited(undo->position);
      /* don't return givebacks to user when rolling back stalemates */
      tmp = undo;
      undo = undo->next;
      SafeFree((GENERIC_PTR)tmp);
    } 
    MarkAsVisited(undo->position);
    *abort = FALSE;
  }
  return(undo);
}

VALUE DetermineValue1(position)
POSITION position;
{
  BOOLEAN foundTie = FALSE, foundLose = FALSE, foundWin = FALSE, Visited();
  MOVELIST *ptr, *head, *GenerateMoves();
  VALUE StoreValueOfPosition(), GetValueOfPosition(), Primitive(), value;
  BOOLEAN dan = FALSE;
  POSITION child;
  REMOTENESS maxRemoteness = 0, minRemoteness = MAXINT2;
  REMOTENESS minTieRemoteness = MAXINT2, remoteness;
  MEXCALC theMexCalc, MexAdd(), MexCalcInit();
  MEX MexCompute(), MexLoad(), MexPrimitive();

  if(Visited(position)) { /* Cycle! */
    return(win);
  }
  /* It's been seen before and value has been determined */
  else if((value = GetValueOfPosition(position)) != undecided) {
    return(value);
  } else if((value = Primitive(position)) != undecided) { 
    /* first time, end */
    SetRemoteness(position,0); /* terminal positions have 0 remoteness */
    if(!kPartizan)
      MexStore(position,MexPrimitive(value)); /* lose=0, win=* */
    return(StoreValueOfPosition(position,value));
  /* first time, need to recursively determine value */
  } else {
    MarkAsVisited(position);
    if(!kPartizan)
      theMexCalc = MexCalcInit();
    head = ptr = GenerateMoves(position);
    while (ptr != NULL) {
      MOVE move = ptr->move ;
      child = DoMove(position,ptr->move);  /* Create the child */
      value = DetermineValue1(child);       /* DFS call */

      if (gGoAgain(position,move))
        switch(value)
        {
        case lose: value=win;break;
        case win: value=lose;break;
        }

      remoteness = Remoteness(child);
      if(!kPartizan)
	theMexCalc = MexAdd(theMexCalc,MexLoad(child));
      if(value == lose) {        /* found a way to give you a lose */
	foundLose = TRUE;        /* thus, it's a winning move      */
        if (remoteness < minRemoteness) minRemoteness = remoteness;
      }
      else if(value == tie) {    /* found a way to give you a tie  */
	foundTie = TRUE;         /* thus, it's a tieing move       */
        if (remoteness < minTieRemoteness) minTieRemoteness = remoteness;
      }
      else if(value == win) {    /* found a way to give you a win  */
	foundWin = TRUE;         /* thus, it's a losing move       */
        if (remoteness > maxRemoteness) maxRemoteness = remoteness;
      }
      else
	BadElse("DetermineValue (1) ");
      ptr = ptr->next;
    }
    FreeMoveList(head);
    UnMarkAsVisited(position);
    if(!kPartizan)
      MexStore(position,MexCompute(theMexCalc));
    if(foundLose) {
      SetRemoteness(position,minRemoteness+1); /* Winners want to mate soon! */
      return(StoreValueOfPosition(position,win));
    }
    else if(foundTie) {
      SetRemoteness(position,minTieRemoteness+1); /* Tiers want to mate now! */
      return(StoreValueOfPosition(position,tie));
    }
    else if (foundWin) {
      SetRemoteness(position,maxRemoteness+1); /* Losers want to extend! */
      return(StoreValueOfPosition(position,lose));
    }
    else
      BadElse("DetermineValue (2)");
  }
  BadElse("DetermineValue (3)");  /* This should NEVER be reached */
  return(undecided);          /* But has been added to satisty lint */
}

VALUE DetermineValue(POSITION position)
{
	if(!loadDatabase())
	{
		gValue = DetermineValue1(gMinimalPosition) ;
		writeDatabase() ;
		return gDatabase[position] % 4;
        }
	return gDatabase[position] % 4 ;
}

MEXCALC MexAdd(theMexCalc,theMex)
MEXCALC theMexCalc;
MEX theMex;
{
  if(theMex > 31) {
    printf("Error: MexAdd handed a theMex greater than 31\n");
    ExitStageRight();
  } else if (theMex == kBadMexValue) {
    printf("Error: MexAdd handed a kBadMexValue for theMex\n");
    ExitStageRight();
  }
  return(theMexCalc | (1 << theMex));
}

MEX MexCompute(theMexCalc)
MEXCALC theMexCalc;
{
  MEX ans = 0;
  while(theMexCalc & (1 << ans))
    ans++;
  return(ans);
}

MEXCALC MexCalcInit()
{
  return((MEXCALC) 0);
}

MexStore(position,theMex)
POSITION position;
MEX theMex;
{
  gDatabase[position] |= ((theMex % 32) * 8) ;
}

MEX MexLoad(position)
POSITION position;
{
  if(gameline)
  {
	char buffer[256] ;
	char userinput[256] ;
	int a ;

	if(sockfd < 0)
		InitClient() ;

	sprintf(userinput, "%s -RawRead %d %d", kDBName, getOption(), position) ;
	a = stuff(userinput, buffer) ;
	writen(sockfd, buffer, a) ;
	readn(sockfd, buffer, 2) ;
	a = getLength(buffer) ;
	a -= 2 ;
	readn(sockfd, buffer, a) ;
	buffer[a] = (char)NULL ;
	//printf("'%s'\n", buffer) ;
	sscanf(buffer, "%d", &position) ;
	return((position /8)%32);
  }
  else
  {
	return (gDatabase[position]/8) % 32 ;
  }
}

MexFormat(position,string)
POSITION position;
STRING string;
{
  MEX theMex, MexLoad();
  char tmp[5];

  if (!kPartizan) { /* Impartial, mex value available */
    theMex = MexLoad(position);
    if(theMex == (MEX)0)
      (void) sprintf(tmp, "0");
    else if(theMex == (MEX)1)
      (void) sprintf(tmp, "*");
    else
      (void) sprintf(tmp, "*%d", (int)theMex);
      
    (void) sprintf(string,"[Val = %s]",tmp);
  } else
    (void) sprintf(string,"");
}

MEX MexPrimitive(value)
VALUE value;
{
  if(value == undecided) {
    printf("Error: MexPrimitive handed a value other than win/lose (undecided)\n");
    ExitStageRight();
  } else if(value == tie) {
    printf("Error: MexPrimitive handed a value other than win/lose (tie)\n");
    ExitStageRight();
  } else if(value == win) 
    return((MEX)1); /* A win terminal is not ideal, but it's a star */
  else if (value == lose)
    return((MEX)0); /* A lose is a zero, that's clear */
  else {
    BadElse("MexPrimitive");
    ExitStageRight();
  }
}

FreeMoveList(ptr)
     MOVELIST *ptr;
{
  MOVELIST *last;
  while (ptr != NULL) {
    last = ptr;
    ptr = ptr->next;
    SafeFree((GENERIC_PTR)last);
  }
}

FreeRemotenessList(REMOTENESSLIST* ptr) 
{
  REMOTENESSLIST* last;
  while (ptr != NULL) {
    last = ptr;
    ptr = ptr->next;
    //    gBytesInUse -= sizeof(REMOTENESSLIST);
    SafeFree((GENERIC_PTR)last);
  }
}

FreePositionList(ptr)
     POSITIONLIST *ptr;
{
  POSITIONLIST *last;
  while (ptr != NULL) {
    last = ptr;
    ptr = ptr->next;
    SafeFree((GENERIC_PTR)last);
  }
}

void FreeValueMoves(VALUE_MOVES *ptr) {
  int i;
  
  if (!ptr) return;
  
  for (i=0; i<3; i++) {
    FreeMoveList(ptr->moveList[i]);
    FreeRemotenessList(ptr->remotenessList[i]);
  }
  //  gBytesInUse -= sizeof(VALUE_MOVES);
  SafeFree((GENERIC_PTR)ptr);
}

int GetRandomNumber(n)
int n;
{
  int ans,nRequest,mulFactor;

  if(n <= 0) {
    printf("Error: GetRandomNumber called with n <= 0! \n");
    return(0);
  }

  /* Try a random number and if it's too big, try again */
  do {
    ans = 0;
    nRequest = n;
    mulFactor = 1;

    while (nRequest >= RAND_MAX) {
      ans += (GetSmallRandomNumber(RAND_MAX) * mulFactor);
      nRequest /= RAND_MAX;
      nRequest += 1;
      mulFactor *= RAND_MAX;
    }
    ans += GetSmallRandomNumber(nRequest) * mulFactor;

  } while (ans >= n);

  return(ans);
}

int GetSmallRandomNumber(n)
     int n;
{
    return(n * ((double)randSafe()/RAND_MAX));
}

int randSafe()
{
  int ans;
  while((ans = rand()) == RAND_MAX)
    ;
  return(ans);
}

BOOLEAN ValidMove(thePosition, theMove)
     POSITION thePosition;
     MOVE theMove;
{
  MOVELIST *ptr, *head, *GenerateMoves();

  head = ptr = GenerateMoves(thePosition);
  while (ptr != NULL) {
    if (theMove == ptr->move) {
      FreeMoveList(head);
      return(TRUE);
    }
    ptr = ptr->next;
  }
  FreeMoveList(head);
  return(FALSE);
}

BOOLEAN PrintPossibleMoves(thePosition)
     POSITION thePosition;
{
  MOVELIST *ptr, *head, *GenerateMoves();

  head = ptr = GenerateMoves(thePosition);
  printf("\nValid Moves : [ ");
  while (ptr != NULL) {
    PrintMove(ptr->move);
    printf(" ");
    ptr = ptr->next;
  }
  printf("]\n\n");
  FreeMoveList(head);
  return(TRUE); /* This should always return true for GetAndPrintPlayersMove */
}

/* Jiong */
PrintMoves(ptr, remoteptr)
     MOVELIST *ptr;
     REMOTENESSLIST *remoteptr;
{
  while (ptr != NULL) {
    printf("\n\t\t");
    PrintMove(ptr->move);
    printf(" \t");
    if(((int) remoteptr->remoteness) == REMOTENESS_MAX)
      printf("Draw");
    else
      printf("%d", (int) remoteptr->remoteness);
    ptr = ptr->next;
    remoteptr = remoteptr->next;
  }
  printf("\n");
}

/* Jiong */
PrintValueMoves(thePosition)
     POSITION thePosition;
{
  VALUE_MOVES *ptr, *GetValueMoves();
  GENERIC_PTR SafeMalloc();

  ptr = GetValueMoves(thePosition);

  printf("\nHere are the values of all possible moves: \n");
  printf("\t\tMove \tRemoteness\n");
  printf("Winning Moves: \t");
  PrintMoves(ptr->moveList[WINMOVE], ptr->remotenessList[WINMOVE]);
  printf("Tieing Moves: \t");
  PrintMoves(ptr->moveList[TIEMOVE], ptr->remotenessList[TIEMOVE]);
  printf("Losing Moves: \t");
  PrintMoves(ptr->moveList[LOSEMOVE], ptr->remotenessList[LOSEMOVE]);
  printf("\n");

  FreeValueMoves(ptr);
}

STRING GetPrediction(position,playerName,usersTurn)
     POSITION position;
     STRING playerName;
     BOOLEAN  usersTurn;
{
  static char prediction[80];
  char mexString[20];
  REMOTENESS Remoteness();
  VALUE value, GetValueOfPosition();

  if(gPrintPredictions && (gMenuMode == Evaluated)) {
    MexFormat(position,mexString);
    value = GetValueOfPosition(position);

    if (value == tie && Remoteness(position) == REMOTENESS_MAX) {
      (void) sprintf(prediction, "(%s %s draw) %s",
		   playerName,
		   ((value == lose && usersTurn && gAgainstComputer) ||
		    (value == win && !usersTurn && gAgainstComputer)) ?
		   "will" : "should",
		   mexString);
    }
    else {
      (void) sprintf(prediction, "(%s %s %s in %d) %s",
		   playerName,
		   ((value == lose && usersTurn && gAgainstComputer) ||
		    (value == win && !usersTurn && gAgainstComputer)) ?
		   "will" : "should",
		   gValueString[(int)value],
		   Remoteness(position),
		   mexString);
    }
  }
  else
    (void) sprintf(prediction,"");

  return(prediction);
}

Stopwatch()
{
  static int first = 1;
  static time_t oldT, newT;

  if(first) {
    first = 0;
    newT = time(NULL);
  }
  oldT = newT;
  newT = time(NULL);
  return(difftime(newT, oldT));
}

ExitStageRight()
{
  printf("\nThanks for playing %s!\n",kGameName); /* quit */
  // This is good practice
  if (gDatabase != NULL) SafeFree(gDatabase);
  exit(0);
}

ExitStageRightErrorString(errorMsg)
char errorMsg[];
{
  printf("\nError: %s\n",errorMsg);
  exit(0);
}

GENERIC_PTR SafeMalloc(amount)
     int amount;
{
  GENERIC_PTR ptr;

  /* Mando's Fix is to put a ckalloc here */
  if((ptr = malloc(amount)) == NULL) {
    printf("Error: SafeMalloc could not allocate the requested %d bytes\n",amount);
    ExitStageRight();
    return(ptr); /* never reached - added to make lint happy */
  }
  else {
    return(ptr);
  }
}

SafeFree(ptr)
     GENERIC_PTR ptr;
{
  free(ptr);
}

VALUE StoreValueOfPosition(position, value)
     POSITION position;
     VALUE value;
{
  VALUE *GetRawValueFromDatabase(), *ptr, GetValueOfPosition(), oldValue;
		
  ptr = GetRawValueFromDatabase(position);

//  if((oldValue = GetValueOfPosition(position)) != undecided)
  //  printf("Error: StoreValueOfPosition(%d,%s) found a slot already filled with %s.\n",position,gValueString[value],gValueString[oldValue]);

  /* put it in the right position, but we have to blank field and then
  ** add new value to right slot, keeping old slots */
  return((*ptr = ((*ptr & ~VALUE_MASK) | (value & VALUE_MASK))) & VALUE_MASK); 
}

// This is it
VALUE GetValueOfPosition(position)
     POSITION position;
{
  if(gameline)
  {
	char buffer[256] ;
	char userinput[256] ;
	int a ;

	if(sockfd < 0)
		InitClient() ;

	sprintf(userinput, "%s -RawRead %d %d", kDBName, getOption(), position) ;
	a = stuff(userinput, buffer) ;
	writen(sockfd, buffer, a) ;
	readn(sockfd, buffer, 2) ;
	a = getLength(buffer) ;
	a -= 2 ;
	readn(sockfd, buffer, a) ;
	buffer[a] = (char)NULL ;
	//printf("'%s'\n", buffer) ;
	sscanf(buffer, "%d", &position) ;
	return position % 4;
  }
  else
  {
	VALUE *GetRawValueFromDatabase(), *ptr;
	ptr = GetRawValueFromDatabase(position);
	return((VALUE)((int)*ptr & VALUE_MASK)); /* return pure value */
  }
}

MOVELIST *CreateMovelistNode(theMove, theNextMove)
     MOVE theMove;
     MOVELIST *theNextMove;
{
  MOVELIST *theHead;
  GENERIC_PTR SafeMalloc();

  theHead = (MOVELIST *) SafeMalloc (sizeof(MOVELIST));
  theHead->move = theMove;
  theHead->next = theNextMove;

  return(theHead);
}

MOVELIST *CopyMovelist(theMovelist)
     MOVELIST *theMovelist;
{
  MOVELIST *ptr, *head = NULL;
  MOVELIST *CreateMovelistNode();
  
  /* Walk down the graph children and copy it into a new structure */
  /* Unfortunately, it reverses the order, which is ok */

  ptr = theMovelist;
  while (ptr != NULL) {
    head = CreateMovelistNode(ptr->move, head);
    ptr = ptr->next;
  }

  return(head);
}

REMOTENESS Remoteness(position)
     POSITION position;
{
  if(gameline)
  {
	char buffer[256] ;
	char userinput[256] ;
	int a ;

	if(sockfd < 0)
		InitClient() ;

	sprintf(userinput, "%s -RawRead %d %d", kDBName, getOption(), position) ;
	a = stuff(userinput, buffer) ;
	writen(sockfd, buffer, a) ;
	readn(sockfd, buffer, 2) ;
	a = getLength(buffer) ;
	a -= 2 ;
	readn(sockfd, buffer, a) ;
	buffer[a] = (char)NULL ;
	//printf("'%s'\n", buffer) ;
	sscanf(buffer, "%d", &position) ;
	return ( (position & REMOTENESS_MASK) >> REMOTENESS_SHIFT );
  }
  else
  {
	VALUE *GetRawValueFromDatabase(), *ptr;
	ptr = GetRawValueFromDatabase(position);
	return((((int)*ptr & REMOTENESS_MASK) >> REMOTENESS_SHIFT));
  }
}

SetRemoteness (position, remoteness)
     POSITION position;
     REMOTENESS remoteness;
{
  VALUE *GetRawValueFromDatabase(), *ptr;

  ptr = GetRawValueFromDatabase(position);

  if(remoteness > REMOTENESS_MAX) {
    printf("Remoteness request (%d) for %d larger than Max Remoteness (%d)\n",remoteness,position,REMOTENESS_MAX);
    ExitStageRight();
  }

  /* blank field then add new remoteness */
  *ptr = (VALUE)(((int)*ptr & ~REMOTENESS_MASK) | 
		 (remoteness << REMOTENESS_SHIFT));       
}

BOOLEAN Visited(position)
     POSITION position;
{
  VALUE *GetRawValueFromDatabase(), *ptr;

  ptr = GetRawValueFromDatabase(position);

  return((((int)*ptr & VISITED_MASK) == VISITED_MASK)); /* Is bit set? */
}

MarkAsVisited (position)
     POSITION position;
{
  VALUE *GetRawValueFromDatabase(), *ptr;

  ptr = GetRawValueFromDatabase(position);

  *ptr = (VALUE)((int)*ptr | VISITED_MASK);       /* Turn bit on */
}

UnMarkAsVisited (position)
     POSITION position;
{
  VALUE *GetRawValueFromDatabase(), *ptr;

  ptr = GetRawValueFromDatabase(position);

  *ptr = (VALUE)((int)*ptr & ~VISITED_MASK);      /* Turn bit off */
}

BadElse(function)
     STRING function;
{
  printf("Error: %s() just reached an else clause it shouldn't have!\n\n",function);
}

HitAnyKeyToContinue()
{
  static BOOLEAN first = TRUE;

  printf("\n\t----- Hit <return> to continue -----");
  first ? (first = FALSE) : getchar(); /* to make lint happy */
  while(getchar() != '\n');
}

USERINPUT HandleDefaultTextInput(thePosition, theMove, playerName)
     POSITION thePosition;
     MOVE *theMove;
     STRING playerName;
{
  MOVE ConvertTextInputToMove(), tmpMove;
  BOOLEAN ValidTextInput();
  char tmpAns[2], input[MAXINPUTLENGTH];

  GetMyString(input,MAXINPUTLENGTH,TRUE,TRUE);

  if(input[0] == '\0')
    PrintPossibleMoves(thePosition);

  else if (ValidTextInput(input))
    if(ValidMove(thePosition,tmpMove = ConvertTextInputToMove(input))) {
      *theMove = tmpMove;
      return(Move);
    }
    else
      PrintPossibleMoves(thePosition);

  else	
    switch(input[0]) {
    case 'Q': case 'q':
      ExitStageRight();
    case 'u': case 'U':
      return(Undo);
    case 'a': case 'A':
      printf("\nSure you want to Abort? [no] :  ");
      GetMyString(tmpAns,2,TRUE,TRUE);
      printf("\n");
      if(tmpAns[0] == 'y' || tmpAns[0] == 'Y')
	return(Abort);
      else
	return(Continue);
    case 'H': case 'h':
      HelpMenus();
      printf("");
      PrintPosition(thePosition, playerName);
      break;
    case 'c': case 'C':
      SmarterComputerMenu();
      break;
    case 'r': case 'R':
      PrintPosition(thePosition, playerName);
      break;
    case 's': case 'S':
      PrintValueMoves(thePosition);
      break;
    case '?':
      printf("%s",kHandleDefaultTextInputHelp);
      PrintPossibleMoves(thePosition);
      break;
    default:
      BadMenuChoice();
      printf("%s",kHandleDefaultTextInputHelp);
      PrintPossibleMoves(thePosition);
      break;
    }

  return(Continue);  /* The default action is to return Continue */
}


GetMyString(name, size, eatFirstChar, putCarraigeReturnBack)
     char *name;
     int   size;
     BOOLEAN eatFirstChar, putCarraigeReturnBack;
{	
  int ctr = 0;
  BOOLEAN seenFirstNonSpace = FALSE;
  signed char c;

  if(eatFirstChar)
    (void) getchar();

  while((c = getchar()) != '\n' && c != EOF) {

    if(!seenFirstNonSpace && c != ' ')
      seenFirstNonSpace = TRUE;

    if(ctr < size - 1 && seenFirstNonSpace)
      name[ctr++] = c;
  }
  name[ctr] = '\0';

  if(putCarraigeReturnBack)
    ungetc('\n',stdin);  /* Put the \n back on the input */
}

VALUE *GetRawValueFromDatabase(position)
     POSITION position;
{
  return(&gDatabase[position]);
}

/* Jiong */
MOVE GetComputersMove(thePosition)
     POSITION thePosition;
{
  MOVE DecodeMove(), theMove;
  MOVE RandomLargestRemotenessMove(), RandomSmallestRemotenessMove();
  int i, randomMove, numberMoves = 0;
  MOVELIST *ptr, *head, *prev;
  VALUE_MOVES *moves, *GetValueMoves();
  REMOTENESSLIST *rptr, *rhead;
  BOOLEAN setBackSmartness = FALSE;
  BOOLEAN moveFound;
  int moveType;
  int oldsmartness = smartness;
  ptr = head = prev = NULL;
  i = 0;
  
  moves = GetValueMoves(thePosition);

  // Play Imperfectly
  if (GetRandomNumber(MAXSCALE+1) > scalelvl && smartness == SMART) {
    smartness = RANDOM;
    setBackSmartness = TRUE;
  }

  // Use givebacks
  if (remainingGivebacks>0 && GetValueOfPosition(thePosition) < oldValueOfPosition) {
    if(gHints) {
      printf("Using giveback: %d givebacks left\n", remainingGivebacks-1);
      printf("%s choose [ ", gPlayerName[kComputersTurn]);
    }
    while (ptr == NULL) {  //try to restore the old status, if lose then get a lose move, if tie then get tie move
      ptr = moves->moveList[oldValueOfPosition];
      oldValueOfPosition--;
    }
    while(ptr->next != NULL) 
      ptr = ptr->next;
    if(gHints) {
      PrintMove(ptr->move);
      printf(" ]\n\n");
    }
    oldValueOfPosition++;
    remainingGivebacks--;
    theMove = ptr->move;
    FreeValueMoves(moves);
    return (theMove);
  }

  oldValueOfPosition = GetValueOfPosition(thePosition);

  if (smartness == SMART) {
    if(gHints) {
      printf("Smart move: \n");
      printf("%s could equivalently choose [ ", gPlayerName[kComputersTurn]);
    }

    //Find best game outcome
    ptr = NULL;
    while (ptr == NULL && i <= LOSEMOVE) {
      head = ptr = moves->moveList[i];
      rhead = rptr = moves->remotenessList[i];
      moveType = i;
      i++;
    }

    if (ptr == NULL || rptr == NULL) {
      printf("Error in GetComputersMove: Either no available moves or corrupted database");
      exit(0);
    }

    if(gHints) {
      while(ptr != NULL) {
        PrintMove(ptr->move);
	printf(" ");
	ptr = ptr->next;
      }
      printf("]\n\n");
    }

    if (moveType == WINMOVE) {
      // WINMOVE: Win as quickly as possible (smallest remoteness best)
      theMove = RandomSmallestRemotenessMove(moves->moveList[moveType], moves->remotenessList[moveType]);
    } else if (moveType == TIEMOVE) {
      // TIEMOVE: Tie as quickly as possible when smart???
      theMove = RandomSmallestRemotenessMove(moves->moveList[moveType], moves->remotenessList[moveType]);
    } else {
      // LOSEMOVE: Prolong the game as much as possible (largest remoteness is best).
      theMove = RandomLargestRemotenessMove(moves->moveList[moveType], moves->remotenessList[moveType]);
    }
    FreeValueMoves(moves);
    return (theMove);
  }

  else if (smartness == RANDOM) {
    if (setBackSmartness == TRUE) {
      smartness = oldsmartness;
      setBackSmartness = FALSE;
    }
    if(gHints) {
      printf("Random move: \n");
      printf("%s could equivalently choose [ ", gPlayerName[kComputersTurn]);
    }
    
    for (head = NULL, i = 0; i <= LOSEMOVE; i++) {
      ptr = moves -> moveList[i];
      while (ptr) {
        if (gHints) {
          PrintMove(ptr -> move);
          printf(" ");
        }
        head = CreateMovelistNode(ptr -> move, head);
        ptr = ptr -> next;
        numberMoves++;
      }
    }
    
    if(gHints)
      printf("]\n\n");
    
    randomMove = GetRandomNumber(numberMoves);
    for (ptr = head; randomMove > 0; --randomMove)
      ptr = ptr -> next;
    
    theMove = ptr -> move;
    
    FreeMoveList(head);
    FreeValueMoves(moves);
    return(theMove);
  }

  else if (smartness == DUMB) {
    if(gHints) {
      printf("Dumb move: \n");
      printf("%s could equivalently choose [ ", gPlayerName[kComputersTurn]);
    }
    
    for (i=LOSEMOVE, ptr=NULL; i >= WINMOVE && ptr == NULL; i--) {
      ptr = moves->moveList[i];
      moveType = i;
    }
    if (ptr == NULL) {
      printf("Error in GetComputersMove: Either no available move or corrupted database");
      exit(0);
    }

    if (gHints) {
      while(ptr != NULL) {
        PrintMove(ptr->move);
	printf(" ");
      }
      printf("]\n\n");
    }

    if (moveType == LOSEMOVE) {
      theMove = RandomSmallestRemotenessMove(moves->moveList[moveType], moves->remotenessList[moveType]);
    }
    else if (moveType == WINMOVE || moveType == TIEMOVE) {
      theMove = RandomLargestRemotenessMove(moves->moveList[moveType], moves->remotenessList[moveType]);
    }
    FreeValueMoves(moves);
    return (theMove);

  } else {
    printf("Error in GetComputerMove: no such intelligence level!\n");
  }
}

MOVE RandomLargestRemotenessMove(MOVELIST *moveList, REMOTENESSLIST *remotenessList) {
  MOVELIST *maxRemotenessMoveList;
  REMOTENESS maxRemoteness;
  MOVE theMove;
  int numMoves, random;

  numMoves = 0;
  maxRemoteness = -1;
  while(remotenessList != NULL) {
    if (remotenessList->remoteness > maxRemoteness) {
      numMoves = 1;
      maxRemoteness = remotenessList->remoteness;
      maxRemotenessMoveList = moveList;
    }
    else if (remotenessList->remoteness == maxRemoteness) {
      numMoves++;
    }
    moveList = moveList->next;
    remotenessList = remotenessList->next;
  }

  if (numMoves<=0) {
    return -1;
  }

  random = GetRandomNumber(numMoves);
  for (; random>0; random--) {
    maxRemotenessMoveList = maxRemotenessMoveList->next;
  }
  return (maxRemotenessMoveList->move);
}

MOVE RandomSmallestRemotenessMove (MOVELIST *moveList, REMOTENESSLIST *remotenessList) {
  int numMoves, random;
  MOVE theMove;
  REMOTENESS minRemoteness;

  numMoves = 0;
  minRemoteness = REMOTENESS_MAX;
  while(remotenessList!=NULL && remotenessList->remoteness <= minRemoteness) {
    numMoves++;
    minRemoteness = remotenessList->remoteness;
    remotenessList = remotenessList->next;
  }

  if (numMoves<=0) {
    return -1;
  }

  random = GetRandomNumber(numMoves);
  for (; random>0; random--) {
    moveList = moveList->next;
  }
  return (moveList->move);
}     

POSITION GetNextPosition()
{
  VALUE GetValueOfPosition();
  static POSITION thePosition = 0; /* Cycle through every position */
  POSITION returnPosition;
  
  while(thePosition < gNumberOfPositions &&
        GetValueOfPosition(thePosition) == undecided)
    thePosition++;
    
  if(thePosition == gNumberOfPositions) {
    thePosition = 0;
    return(kBadPosition);
  }
  else {
    returnPosition = thePosition++;
    return(returnPosition);
  }
}  

/* Jiong */
VALUE_MOVES* SortMoves (thePosition, move, valueMoves) 
     POSITION thePosition;
     MOVE move;
     VALUE_MOVES *valueMoves;
{
  MOVELIST *GenerateMoves();
  POSITION child;
  VALUE_MOVES* StoreMoveInList(); 
  VALUE GetValueOfPosition(), Primitive();
  REMOTENESS Remoteness();

  if (GetValueOfPosition(child = DoMove(thePosition, move)) == lose) {  //winning moves
    valueMoves = StoreMoveInList(move, Remoteness(child), valueMoves,  WINMOVE);
  } else if (GetValueOfPosition(child = DoMove(thePosition, move)) == tie) {  //tie moves
    valueMoves = StoreMoveInList(move, Remoteness(child), valueMoves,  TIEMOVE);
  } else if (GetValueOfPosition(child = DoMove(thePosition, move)) == win) {  //lose moves
    valueMoves = StoreMoveInList(move, Remoteness(child), valueMoves, LOSEMOVE);
  }
  return valueMoves;
}

/* Jiong */
VALUE_MOVES* GetValueMoves(thePosition)
     POSITION thePosition;
{
  MOVELIST *ptr, *head, *GenerateMoves();
  VALUE_MOVES *SortMoves(), *valueMoves;
  VALUE GetValueOfPosition(), theValue, Primitive();
  POSITION child;
  GENERIC_PTR SafeMalloc();

  valueMoves = (VALUE_MOVES *) SafeMalloc (sizeof(VALUE_MOVES));
  valueMoves->moveList[0]=valueMoves->moveList[1]=valueMoves->moveList[2]=NULL;
  valueMoves->remotenessList[0]=valueMoves->remotenessList[1]=valueMoves->remotenessList[2]=NULL;

  if(Primitive(thePosition) != undecided)   /* Primitive positions have no moves */
    return(valueMoves);

  else if((theValue = GetValueOfPosition(thePosition)) == undecided)
    return(valueMoves);                           /* undecided positions are invalid */
 
  else {                                    /* we are guaranteed it's win | tie now */
    head = ptr = GenerateMoves(thePosition);
    while(ptr != NULL) {                    /* otherwise  (theValue = (win|tie) */
      valueMoves = SortMoves(thePosition, ptr->move, valueMoves);
      ptr = ptr->next;
    }
    FreeMoveList(head);
  }
  return(valueMoves);
}

BOOLEAN CorruptedValuesP()
{
  MOVELIST *ptr, *head, *GenerateMoves();
  VALUE GetValueOfPosition(), Primitive(), parentValue, childValue;
  POSITION position, child;
  BOOLEAN parentIsWin, foundLosingChild, parentIsTie, foundTieingChild, corrupted;

  corrupted = FALSE;
  for(position=0 ; position<gNumberOfPositions ; position++) { /* for all positions */
    parentIsWin = FALSE;
    foundLosingChild = FALSE;
    parentIsTie = FALSE;
    foundTieingChild = FALSE;
    if((parentValue = GetValueOfPosition(position)) != undecided) { /* if valid position */
      if(Primitive(position) == undecided) { /* Not Primitive, children */
	head = ptr = GenerateMoves(position);
	while (ptr != NULL) {
	  child = DoMove(position,ptr->move);  /* Create the child */
	  childValue = GetValueOfPosition(child); /* Get its value */
	  if(parentValue == lose) {
	    if(childValue != win) {
	      corrupted = TRUE;
	      printf("Corruption: Losing Parent %d has %s child %d, shouldn't be losing\n",position,gValueString[childValue],child);
	    }
	  } else if (parentValue == win) {
	    parentIsWin = TRUE;
	    if(childValue == lose)
	      foundLosingChild = TRUE;
	  } else if (parentValue == tie) {
	    parentIsTie = TRUE;
	    if(childValue == lose) {
	      corrupted = TRUE;
	      printf("Corruption: Tieing Parent %d has Lose child %d, should be win\n",position,child);
	    } else if (childValue == tie)
	      foundTieingChild = TRUE;
	  } else 
	    BadElse("CorruptedValuesP");
	  ptr = ptr->next;                     /* Go to the next child */
	} /* while ptr != NULL (for all children) */
	FreeMoveList(head);
	if(parentIsWin && !foundLosingChild) {
	  corrupted = TRUE;
	  printf("Corruption: Winning Parent %d has no losing children, shouldn't be win\n",position);
	}
	if(parentIsTie && !foundTieingChild) {
	  corrupted = TRUE;
	  printf("Corruption: Tieing Parent %d has no tieing children, should be a lose\n",position);
	}
      } /* if not primitive */
    } /* if valid position */
  } /* for all positions */
  return(corrupted);
}

/* Jiong */
SmarterComputerMenu()
{
  char c;
  do {
    printf("\n\t----- Adjust COMPUTER'S brain menu for %s -----\n\n", kGameName);
    printf("\tp)\t(P)erfectly always\n");
    printf("\ti)\t(I)mperfectly (Perfect some %% of time, Randomly others)\n");
    printf("\tr)\t(R)andomly always\n");
    printf("\tm)\t(M)isere-ly always (i.e., trying to lose!)\n\n");
    printf("\tg)\tChange the number of (G)ive-backs (currently %d)\n\n", initialGivebacks);
    printf("\th)\t(H)elp\n\n");
    printf("\tb)\t(B)ack = Return to previous activity\n\n");
    printf("\nSelect an option: ");
    
    switch(c = GetMyChar()) {
    case 'P': case 'p':
      smartness = SMART;
      scalelvl = 100;
      HitAnyKeyToContinue();
      break;
    case 'I': case 'i':
      smartness = SMART;
      printf("\nPlease enter the chance %% of time the computer plays perfectly (0-100): ");
      scanf("%d", &scalelvl);
      while (scalelvl < 0 || scalelvl > 100) {
	printf("\nPlease enter the chance %% of time the computer plays perfectly (0-100): ");
	scanf("%d", &scalelvl);
      }
      HitAnyKeyToContinue();
      break;
    case 'R': case 'r':
      smartness = RANDOM;
      HitAnyKeyToContinue();
      break;
    case 'M': case 'm':
      smartness = DUMB;
      HitAnyKeyToContinue();
      break;
    case 'G': case 'g':
      printf("\nPlease enter the number of give-backs the computer will perform (0-%d): ", MAXGIVEBACKS);
      scanf("%d", &initialGivebacks);
      while (initialGivebacks > MAXGIVEBACKS || initialGivebacks < 0) {
	printf("\nPlease enter the number of give-backs the computer will perform (0-%d): ", MAXGIVEBACKS);
	scanf("%d", &initialGivebacks);
      } 
      remainingGivebacks = initialGivebacks;
      HitAnyKeyToContinue();
      break;
    case 'H': case 'h':
      HelpMenus();
      break;
    case 'B': case 'b':
      return;
    default:
      BadMenuChoice();
      HitAnyKeyToContinue();
      break;
    }
  } while(TRUE);
}

AnalysisMenu()
{
  POSITIONLIST *badWinPositions = NULL, *badTiePositions = NULL, *badLosePositions = NULL;
  BOOLEAN tempPredictions = gPrintPredictions, CorruptedValuesP();
  MEX mexValue = 0;
  int mexInt, maxPositions = 10;
  char GetMyChar(), c;

  gPrintPredictions = FALSE;
  
  do {
    printf("\n\t----- Post-Evaluation ANALYSIS menu for %s -----\n\n", kGameName);
    
    printf("\ti)\tPrint the (I)nitial position\n");
    printf("\tn)\tChange the (N)umber of printed positions (currently %d)\n",maxPositions);
    if(!kPartizan) { /* Impartial */
      printf("\tv)\tChange the Mex (V)alue (currently %d)\n",(int)mexValue);
      printf("\n\tm)\tPrint up to %d positions with (M)ex value %d\n",maxPositions,(int)mexValue);
    } else
      printf("\n");
    printf("\tw)\tPrint up to %d (W)inning positions\n",maxPositions);
    printf("\tl)\tPrint up to %d (L)osing  positions\n",maxPositions);
    printf("\tt)\tPrint up to %d (T)ieing  positions\n",maxPositions);
    printf("\n\tp)\t(P)rint the overall summmary of game values\n");
    printf("\tf)\tPrint to an ascii (F)ile the raw game values + remoteness\n");
    printf("\to)\tPrint to std(O)ut the raw game values + remoteness\n");

    printf("\n\tc)\t(C)heck if value database is corrupted\n");

    if(badWinPositions != NULL)
      printf("\t1)\tPrint up to %d (W)inning INCORRECT positions\n",maxPositions);
    if(badTiePositions != NULL)
      printf("\t2)\tPrint up to %d (T)ieing  INCORRECT positions\n",maxPositions);
    if(badLosePositions != NULL)
      printf("\t3)\tPrint up to %d (L)osing  INCORRECT positions\n",maxPositions);
    
    printf("\n\th)\t(H)elp\n");
    printf("\n\tb)\t(B)ack = Return to previous activity\n");
    printf("\n\nSelect an option: ");
    
    switch(c = GetMyChar()) {
    case 'Q': case 'q':
      ExitStageRight();
    case 'H': case 'h':
      HelpMenus();
      break;
    case 'C': case 'c':
      if(CorruptedValuesP())
	printf("\nCorrupted values found and printed above. Sorry.\n");
      else
	printf("\nNo Corrupted Values found!\n");
      HitAnyKeyToContinue();
      break;
    case 'F': case 'f':
      PrintRawGameValues(TRUE);
      break;
    case 'O': case 'o':
      PrintRawGameValues(FALSE);
      HitAnyKeyToContinue();
      break;
    case 'i': case 'I':
      printf("\n\t----- The Initial Position is shown below -----\n");
      PrintPosition(gInitialPosition);
      HitAnyKeyToContinue();
      break;
    case 'p': case 'P':
      PrintGameValueSummary();
      HitAnyKeyToContinue();
      break;
    case 'm': case 'M':
      if(!kPartizan) /* Impartial */
	PrintMexValues(mexValue,maxPositions);
      else {
	BadMenuChoice();
	HitAnyKeyToContinue();
      }
      break;
    case 'n': case 'N':
      printf("\nPlease enter the MAX number of positions : ");
      scanf("%d", &maxPositions);
      break;
    case 'v': case 'V':
      if(!kPartizan) { /* Impartial */
	printf("\nPlease enter the MEX number : ");
	scanf("%d", &mexInt);
	mexValue = (MEX) mexInt;
      } else {
	BadMenuChoice();
	HitAnyKeyToContinue();
      }
      break;
    case 'w': case 'W': case 'l': case 'L': case 't': case 'T':
      PrintValuePositions(c,maxPositions);
      break;
    case '1': case '2': case '3':
      PrintBadPositions(c,maxPositions,badWinPositions, badTiePositions, badLosePositions);
      break;
    case 'b': case 'B':
      FreePositionList(badWinPositions);
      FreePositionList(badTiePositions);
      FreePositionList(badLosePositions);
      gPrintPredictions = tempPredictions;
      return;
    default:
      BadMenuChoice();
      HitAnyKeyToContinue();
      break;
    }
  } while(TRUE);
}

PrintRawGameValues(toFile)
BOOLEAN toFile;
{
  BOOLEAN Visited();
  FILE *fp;
  char filename[80];
  int i;
  VALUE GetValueOfPosition(), value;

  if(toFile) {
    printf("File to save to: ");
    scanf("%s",filename);
    
    if((fp = fopen(filename, "w")) == NULL)
      ExitStageRightErrorString("Couldn't open file, sorry.");
  } else
    fp = stdout;
  
  fprintf(fp,"Position/Value list for %s\n\n", kGameName);
  fprintf(fp,"POS  | VISITED-FLAG VALUE in REMOTENESS\n");
  
  for(i=0 ; i<gNumberOfPositions ; i++)
    if((value = GetValueOfPosition((POSITION)i)) != undecided)
      fprintf(fp,"%00005d | %c %4s in %d\n",
	      i,
	      Visited((POSITION)i) ? 'V' : '-',
	      gValueString[value],
	      Remoteness((POSITION)i));

  if(toFile)
    fclose(fp);
}

PrintBadPositions(c,maxPositions,badWinPositions, badTiePositions, badLosePositions)
     char c;
     int maxPositions;
     POSITIONLIST *badWinPositions, *badTiePositions, *badLosePositions;
{
  POSITIONLIST *ptr;
  BOOLEAN continueSearching = TRUE;
  POSITION thePosition, GetNextPosition();
  VALUE GetValueOfPosition(), theValue;
  int j;
  char yesOrNo;

  if     (c == '1') ptr = badWinPositions;
  else if(c == '2') ptr = badTiePositions;
  else if(c == '3') ptr = badLosePositions;
  else BadElse("PrintBadPositions");

  j = 0;
  continueSearching = TRUE;
  do {
    for(j = 0 ; ptr != NULL && j < maxPositions ;j++) {
      thePosition = ptr->position;
      PrintPosition(thePosition);
      ptr = ptr->next;
    }
    if(ptr != NULL) {
      printf("\nDo you want more? [Y/N] : ");
      scanf("%c",&yesOrNo);
      scanf("%c",&yesOrNo);
      continueSearching = (yesOrNo == 'y' || yesOrNo == 'Y');
    }
    else
      printf("\nThere are no more %s positions to list...\n",
	     c == '1' ? "winning" : c == '3' ? "losing" : "tieing");
  } while (continueSearching && ((ptr = ptr->next) != NULL));
}

PrintGameValueSummary()
{
  POSITION thePosition, GetNextPosition();
  VALUE GetValueOfPosition(), theValue;
  int winCount, loseCount, tieCount, unknownCount;
  int totalPositions;

  totalPositions = winCount = loseCount = tieCount = unknownCount = 0;
  for(thePosition = 0 ; thePosition < gNumberOfPositions ; thePosition++) 
  {
    theValue = GetValueOfPosition(thePosition);
    if (theValue != undecided) {
      totalPositions++;
      if(theValue == win)            winCount++;
      else if(theValue == lose)      loseCount++;
      else if(theValue == tie)       tieCount++;
      else                           unknownCount++;
    }
  }
  printf("\n\n\t----- Summmary of Game values -----\n\n");
  
  printf("\tValue       Number       Total\n");
  printf("\t------------------------------\n");
  printf("\tLose      = %5d out of %d\n",loseCount,totalPositions);	
  printf("\tWin       = %5d out of %d\n",winCount,totalPositions);	
  printf("\tTie       = %5d out of %d\n",tieCount,totalPositions);	
  printf("\tUnknown   = %5d out of %d\n",unknownCount,totalPositions);	
  printf("\tTOTAL     = %5d out of %d\n",
	 loseCount+winCount+tieCount+unknownCount,
	 totalPositions);	
}

PrintMexValues(mexValue,maxPositions)
     MEX mexValue;
     int maxPositions;
{
  BOOLEAN continueSearching = TRUE;
  POSITION thePosition, GetNextPosition();
  VALUE GetValueOfPosition(), theValue;
  MEX MexLoad();
  int j;
  char yesOrNo;

  j = 0;
  continueSearching = TRUE;
  do {
    for(j = 0 ; ((thePosition = GetNextPosition()) != kBadPosition) && j < maxPositions ;) {
      if (MexLoad(thePosition) == mexValue) {
	PrintPosition(thePosition);
	j++;
      }
    }
    if(thePosition != kBadPosition) {
      printf("\nDo you want more? [Y/N] : ");
      scanf("%c",&yesOrNo);
      scanf("%c",&yesOrNo);
      continueSearching = (yesOrNo == 'y' || yesOrNo == 'Y');
    }
    else
      printf("\nThere are no more positions with Mex value %d to list...\n",
	     (int)mexValue);
  } while (continueSearching && (thePosition != kBadPosition));
	HitAnyKeyToContinue();
}

PrintValuePositions(c,maxPositions)
     char c;
     int maxPositions;
{      
  BOOLEAN continueSearching = TRUE;
  POSITION thePosition, GetNextPosition();
  VALUE GetValueOfPosition(), theValue;
  int j;
  char yesOrNo;

  j = 0;
  continueSearching = TRUE;
  do {
    for(j = 0 ; ((thePosition = GetNextPosition()) != kBadPosition) && j < maxPositions ;) {
      theValue = GetValueOfPosition(thePosition);
      if((theValue == win  && (c == 'w' || c == 'W')) || 
	 (theValue == lose && (c == 'l' || c == 'L')) ||
	 (theValue == tie  && (c == 't' || c == 'T'))) {
	PrintPosition(thePosition);
	j++;
      }
    }
    if(thePosition != kBadPosition) {
      printf("\nDo you want more? [Y/N] : ");
      scanf("%c",&yesOrNo);
      scanf("%c",&yesOrNo);
      continueSearching = (yesOrNo == 'y' || yesOrNo == 'Y');
    }
    else
      printf("\nThere are no more %s positions to list...\n",
	     c == 'w' || c == 'W' ? "winning" : c == 'l' || c == 'L' ? "losing" : "tieing");
  } while (continueSearching && (thePosition != kBadPosition));
	HitAnyKeyToContinue();
}

POSITIONLIST *StorePositionInList(thePosition,thePositionList)
     POSITION     thePosition;
     POSITIONLIST *thePositionList;
{
  POSITIONLIST *next, *tmp;
  GENERIC_PTR SafeMalloc();
  
  next = thePositionList;
  tmp = (POSITIONLIST *) SafeMalloc (sizeof(POSITIONLIST));
  tmp->position = thePosition;
  tmp->next     = next;

  return(tmp);
}

/* Jiong */
VALUE_MOVES* StoreMoveInList(theMove, remoteness, valueMoves, typeofMove)
     MOVE     theMove;
     int      remoteness, typeofMove;
     VALUE_MOVES *valueMoves;
{
  MOVELIST *moveList, *newMove, *prevMoveList;
  REMOTENESSLIST *remotenessList, *newRemoteness, *prevRemotenessList;
  GENERIC_PTR SafeMalloc();
  
  moveList = valueMoves->moveList[typeofMove];
  remotenessList = valueMoves->remotenessList[typeofMove];

  newMove = (MOVELIST *) SafeMalloc (sizeof(MOVELIST));
  newRemoteness = (REMOTENESSLIST *) SafeMalloc (sizeof(REMOTENESSLIST));
  newMove->move = theMove;
  newMove->next = NULL;
  newRemoteness->remoteness = remoteness;
  newRemoteness->next = NULL;

  prevMoveList = NULL;
  prevRemotenessList = NULL;

  if (moveList == NULL) {
    valueMoves->moveList[typeofMove] = newMove;
    valueMoves->remotenessList[typeofMove] = newRemoteness;
    return valueMoves;
  }

  while(moveList != NULL) {
    if (remotenessList->remoteness >= remoteness) {
      newMove->next = moveList;
      newRemoteness->next = remotenessList;

      if (prevMoveList==NULL) {
	valueMoves->moveList[typeofMove] = newMove;
        valueMoves->remotenessList[typeofMove] = newRemoteness;
      }
      else {
	prevMoveList->next = newMove;
	prevRemotenessList->next = newRemoteness;
      }
      return (valueMoves);
    }
    else {
      prevMoveList = moveList;
      moveList = moveList->next;
      prevRemotenessList = remotenessList;
      remotenessList = remotenessList->next;
    }
  }

  prevMoveList->next = newMove;
  prevRemotenessList->next = newRemoteness;
  return (valueMoves);
}

//// START LOOPY

MyPrintParents()
{
  int i;
  POSITIONLIST *ptr;
  VALUE GetValueOfPosition();
  BOOLEAN Visited();

  printf("PARENTS | #Children | Value\n");
  
  for(i=0 ; i<gNumberOfPositions ; i++)
    if(Visited(i)) {
      ptr = gParents[i];
      printf("%2d: ",i);
      while (ptr != NULL) {
	printf("[%2d] ",ptr->position);
	ptr = ptr->next;
      }
      printf("| %d children | %s value",(int)gNumberChildren[i],gValueString[GetValueOfPosition((POSITION)i)]);
      printf("\n");
    }
}

VALUE DetermineLoopyValue1(position)
POSITION position;
{				
  POSITION DeQueueWinFR(), DeQueueLoseFR(), DeQueueTieFR(), child, parent;
  POSITIONLIST *ptr;
  VALUE GetValueOfPosition(), childValue;
  REMOTENESS remotenessChild, Remoteness();
  void SetParents();
  int i;

  /* Do DFS to set up Parent pointers and initialize KnownList w/Primitives */

  SetParents(kBadPosition,position);
  if(kDebugDetermineValue) {
    printf("---------------------------------------------------------------\n");
    printf("Number of Positions = [%d]\n",gNumberOfPositions);
    printf("---------------------------------------------------------------\n");
    // MyPrintParents();
    printf("---------------------------------------------------------------\n");
    //MyPrintFR();
    printf("---------------------------------------------------------------\n");
  }

  /* Now, the fun part. Starting from the children, work your way back up. */
  //@@ separate lose/win frontiers
  while ((gHeadLoseFR != NULL) ||
	 (gHeadWinFR != NULL)) {

    if ((child = DeQueueLoseFR()) == kBadPosition)
      child = DeQueueWinFR();
    
    /* Might as well grab these now, they'll be used later */
    childValue = GetValueOfPosition(child);
    remotenessChild = Remoteness(child);

    /* If debugging, print who's in list */
    if(kDebugDetermineValue)
      printf("Grabbing %d (%s) remoteness = %d off of FR\n",
	     child,gValueString[childValue],remotenessChild);

    /* Remove the child from the FRontier, we're working with it now */
    //RemoveFR(child);
    //@@don't need this anymore, frontiers are now queues.
    
    /* With losing children, every parent is winning, so we just go through
    ** all the parents and declare them winning */
    if (childValue == lose) {
      ptr = gParents[child];
      while (ptr != NULL) {
	/* Make code easier to read */
	parent = ptr->position;
	
	/* Skip if this is the initial position (parent is kBadPosition) */
	if (parent != kBadPosition) {	
	  if (GetValueOfPosition(parent) == undecided) {
	    /* This is the first time we know the parent is a win */
	    InsertWinFR(parent);
	    if(kDebugDetermineValue) printf("Inserting %d (%s) remoteness = %d into win FR\n",parent,"win",remotenessChild+1);
	    StoreValueOfPosition(parent,win); 
	    SetRemoteness(parent, remotenessChild + 1);
	  } 
	  else {
	    /* We already know the parent is a winning position. */
	    
	    if (GetValueOfPosition(parent) != win)
	      BadElse("&d should be win.  Instead it is %d.", parent, GetValueOfPosition(parent));
	    
	    /* This should always hold because the frontier is a queue.
	    ** We always examine losing nodes with less remoteness first */
	    assert((remotenessChild + 1) >= Remoteness(parent));
	  }
	}
	ptr = ptr->next;
      } /* while there are still parents */

    /* With winning children */
    } else if (childValue == win) {
      ptr = gParents[child];
      while (ptr != NULL) {
	/* Make code easier to read */
	parent = ptr->position;

	/* Skip if this is the initial position (parent is kBadPosition) */
	/* If this is the last unknown child and they were all wins, parent is lose */
	if(parent != kBadPosition && --gNumberChildren[parent] == 0) {
	    /* no more kids, it's not been seen before, assign it as losing, put at head */
	  assert(GetValueOfPosition(parent) == undecided);

	  InsertLoseFR(parent);
	  if(kDebugDetermineValue) printf("Inserting %d (%s) into FR head\n",parent,"lose");
	  StoreValueOfPosition(parent,lose);
	  /* We always need to change the remoteness because we examine winning node with
	  ** less remoteness first. */
	  SetRemoteness(parent, remotenessChild + 1);
	}
	ptr = ptr->next;  
      } /* while there are still parents */

    /* With children set to other than win/lose. So stop */
    } else {
      BadElse("DetermineLoopyValue found FR member with other than win/lose value");
    } /* else */

    /* We are done with this position and no longer need to keep around its list of parents
    ** The tie frontier will not need this, either, because this child's value has already
    ** been determined.  It cannot be a tie. */
    FreePositionList(gParents[child]);
    gParents[child] = NULL;
    
  } /* while still positions in FR */

  /* Now process the tie frontier */

  while(gHeadTieFR != NULL) {
    child = DeQueueTieFR();
    remotenessChild = Remoteness(child);

    ptr = gParents[child];

    while (ptr != NULL) {
      parent = ptr->position;

      if(GetValueOfPosition(parent) == undecided) {
	/* this position has no losing children but has a tieing position so it must be a 
         * tie. Assign its value and set its remoteness.  Note that 
         * we give ties with lowest remoteness priority (i.e. if a 
         * position has no losing children, a tieing child of 
         * remoteness 2, and a tieing child of remoteness 10, the 
         * position will be a tie of remoteness 3, not 11.  This 
         * decision is pretty arbitrary.  We did it this way to be 
         * consistent with DetermineValue for non-loopy games. */
	
	InsertTieFR(parent);
	if(kDebugDetermineValue) printf("Inserting %d (%s) remoteness = %d into win FR\n",parent,"tie",remotenessChild+1);
	StoreValueOfPosition(parent,tie); 
	SetRemoteness(parent, remotenessChild + 1);
      }
      ptr = ptr->next;
    }
    FreePositionList(gParents[child]);
    gParents[child] = NULL;
  }

  /* Now set all remaining positions to tie with remoteness of REMOTENESS_MAX */

  if(kDebugDetermineValue) {
    printf("---------------------------------------------------------------\n");
    //MyPrintFR();
    printf("---------------------------------------------------------------\n");
    MyPrintParents();
    printf("---------------------------------------------------------------\n");
    printf("TIE cleanup\n");
  }
  
  for (i = 0; i < gNumberOfPositions; i++)
    if(Visited(i)) {
      if(kDebugDetermineValue)
	printf("%d was visited...",i);
      if(GetValueOfPosition((POSITION)i) == undecided) {
	StoreValueOfPosition((POSITION)i,tie);
	SetRemoteness((POSITION)i,REMOTENESS_MAX);
	//we are done with this position and no longer need to keep around its list of parents
	if (gParents[child]) FreePositionList(gParents[child]);
	if(kDebugDetermineValue)
	  printf("and was undecided, setting to tie\n",i);
      } else
	if(kDebugDetermineValue)
	  printf("but was decided, ignoring\n",i);
      UnMarkAsVisited((POSITION)i);
    }

  return(GetValueOfPosition(position));
}

VALUE DetermineLoopyValue(position)
POSITION position;
{
  if(!loadDatabase())
    {
      InitializeFR();
      ParentInitialize();
      NumberChildrenInitialize();
      gValue = DetermineLoopyValue1(gMinimalPosition) ;
      NumberChildrenFree();
      ParentFree();
      writeDatabase() ;
      return gDatabase[position] % 4;
    }
  return gDatabase[position] % 4 ;
}

/*
** Requires: the root has not been visited yet
** (We do not check to see if its been visited)
*/

void SetParents (POSITION parent, POSITION root)
{
  // Prototypes
  BOOLEAN Visited();
  MOVELIST* GenerateMoves();
  VALUE Primitive();
  POSITIONLIST* StorePositionInList();
  
  MOVELIST* moveptr, * movehead;
  POSITIONLIST* posptr, * thisLevel, * nextLevel;
  POSITION pos, child;
  VALUE value;
  
  posptr = thisLevel = nextLevel = NULL;
  moveptr = movehead = NULL;
  
  // Check if the top is primitive.
  MarkAsVisited(root);
  gParents[root] = StorePositionInList(parent, gParents[root]);
  if ((value = Primitive(root)) != undecided) {
    SetRemoteness(root, 0);
    switch (value) {
      case lose: InsertLoseFR(root); break;
      case win:  InsertWinFR(root); break;
      case tie:  InsertTieFR(root); break;
      default:   BadElse("SetParents found primitive with value other than win/lose/tie");
    }
    
    StoreValueOfPosition(root, value);
    return;
  }
  
  thisLevel = StorePositionInList(root, thisLevel);
  
  while (thisLevel != NULL) {
    for (posptr = thisLevel; posptr != NULL; posptr = posptr -> next) {
      pos = posptr -> position;

      movehead = GenerateMoves(pos);
      
      for (moveptr = movehead; moveptr != NULL; moveptr = moveptr -> next) {
	child = DoMove(pos, moveptr -> move);
	++gNumberChildren[(int)pos];
	gParents[(int)child] = StorePositionInList(pos, gParents[(int)child]);
	
	if (Visited(child)) continue;
	MarkAsVisited(child);
	
	if ((value = Primitive(child)) != undecided) {
	  SetRemoteness(child, 0);
	  switch (value) {
	    case lose: InsertLoseFR(child); break;
	    case win : InsertWinFR(child);  break;
	    case tie : InsertTieFR(child);  break;
	    default  : BadElse("SetParents found bad primitive value");
	  }
	  StoreValueOfPosition(child, value);
	} else {
	  nextLevel = StorePositionInList(child, nextLevel);
	}
      }
      
      FreeMoveList(movehead);
    }
    
    FreePositionList(thisLevel);
    
    thisLevel = nextLevel;
    nextLevel = NULL;
  }
}

ParentInitialize()
{
  GENERIC_PTR SafeMalloc();
  int i;

  gParents = (POSITIONLIST **) SafeMalloc (gNumberOfPositions * sizeof(POSITIONLIST *));
  for(i = 0; i < gNumberOfPositions; i++)
    gParents[i] = NULL;
}

ParentFree()
{
  int i;

  for (i = 0; i < gNumberOfPositions; i++) {
    FreePositionList(gParents[i]);
  }

  SafeFree(gParents);
}

NumberChildrenInitialize()
{
  GENERIC_PTR SafeMalloc();
  int i;

  gNumberChildren = (char *) SafeMalloc (gNumberOfPositions * sizeof(signed char));
  for(i = 0; i < gNumberOfPositions; i++)
    gNumberChildren[i] = 0;
}

NumberChildrenFree()
{
  SafeFree(gNumberChildren);
}

InitializeFR()
{
  gHeadWinFR = NULL;
  gTailWinFR = NULL;
  gHeadLoseFR = NULL;
  gTailLoseFR = NULL;
  gHeadTieFR = NULL;
  gTailTieFR = NULL;
}

POSITION DeQueueWinFR()
{
  /* printf("DeQueueWinFR...\n"); */
  return DeQueueFR(&gHeadWinFR, &gTailWinFR);
}

POSITION DeQueueLoseFR()
{
  /* printf("DeQueueLoseFR...\n"); */
  return DeQueueFR(&gHeadLoseFR, &gTailLoseFR);
}

POSITION DeQueueTieFR()
{
  return DeQueueFR(&gHeadTieFR, &gTailTieFR);
}

POSITION DeQueueFR(FRnode **gHeadFR, FRnode **gTailFR)
{
  POSITION position;
  FRnode *tmp;
  
  if (*gHeadFR == NULL)
    return kBadPosition;
  else {
    position = (*gHeadFR)->position;
    tmp = *gHeadFR;
    (*gHeadFR) = (*gHeadFR)->next;
    SafeFree(tmp);

    if (*gHeadFR == NULL)
      *gTailFR = NULL;
  }
  return position;
}

InsertWinFR(POSITION position)
{
  /* printf("Inserting WinFR...\n"); */
  InsertFR(position, &gHeadWinFR, &gTailWinFR);
}


InsertLoseFR(POSITION position)
{
  /* printf("Inserting LoseFR...\n"); */
  InsertFR(position, &gHeadLoseFR, &gTailLoseFR);
}

InsertTieFR(POSITION position)
{
  InsertFR(position, &gHeadTieFR, &gTailTieFR);
}

InsertFR(POSITION position, FRnode **firstnode,
			FRnode **lastnode)
{
  FRnode *tmp = (FRnode *) SafeMalloc(sizeof(FRnode));
  tmp->position = position;
  tmp->next = NULL;

  if (*lastnode == NULL) {
    assert(*firstnode == NULL);
    *firstnode = tmp;
    *lastnode = tmp;
  } else {
    assert((*lastnode)->next == NULL);
    (*lastnode)->next = tmp;
    *lastnode = tmp;
  }
}

// End Loopy

///// SUNIL DATABASE STUFF

void SolveAndStoreRaw(int option)
{
	int i, j ;
	int n = NumberOfOptions() ;
	char outfilename[256] ;
	FILE * filep ;

	for(i = 0 ; i < gNumberOfPositions ; i++)
		gDatabase[i] = undecided ;

	sprintf(outfilename, "data/m%s_%d_raw.dat", kDBName, option) ;
	if((filep = fopen(outfilename, "w")) == NULL)
		printf("Unable to open file to write. Quitting.\n"), exit(1) ;

	setOption(option) ;

	if(kLoopy)
		gValue = DetermineLoopyValue(gMinimalPosition) ;
	else
		gValue = DetermineValue(gMinimalPosition) ;

	fwrite(gDatabase, sizeof(int), gNumberOfPositions, filep) ;
}


void SolveAndStore(int option)
{
	int i, j ;
	int n = NumberOfOptions() ;

	for(i = 0 ; i < gNumberOfPositions ; i++)
		gDatabase[i] = undecided ;

	setOption(option) ;
	if(kLoopy)
		gValue = DetermineLoopyValue(gInitialPosition) ;
	else
		gValue = DetermineValue(gInitialPosition) ;

	writeDatabase() ;
}

int writeDatabase()
{
	int i ;
	int badcount ;
	FILE * filep ;
	FILE * rawfilep ;
	STRING tempfilename = "gamesman.dump" ;
	char outfilename[256] ;

	mkdir("data", 0755) ;

	sprintf(outfilename, "data/m%s_%d.dat", kDBName, getOption()) ;

	if((filep = fopen(tempfilename, "w")) == NULL) // ... if we are  unable to create a temporary file
	{
		printf("Unable to create temporary file") ;
		return 0 ;
	}

	// start our compression algorithm
	for(i = 0, badcount = 0 ; i < gNumberOfPositions ; i++)
	{
		if(gDatabase[i] % 4 == 3)
			badcount++ ;
		else
		{
			if(badcount > 0)
			{
				badcount = 4*badcount + 3 ;
				fwrite(&badcount, sizeof(int), 1, filep) ;
				badcount = 0 ;
			}
			fwrite(gDatabase+i, sizeof(int), 1, filep) ;
		}
	}
	if(badcount > 0)
	{
		badcount = 4*badcount + 3 ;
		fwrite(&badcount, sizeof(int), 1, filep) ;
	}
	fwrite(&gNumberOfPositions, sizeof(int), 1, filep) ;

	fclose(filep) ;

	CompressFile(tempfilename, outfilename) ;
	remove(tempfilename) ;
}

int loadDatabase()
{
	int i, temp ;
	FILE * filep ;
	STRING tempfilename = "/tmp/gamesman/gamesman.dump" ;
	char outfilename[256] ;

	sprintf(outfilename, "data/m%s_%d.dat", kDBName, getOption()) ;

	if((filep = fopen(outfilename, "r")) == NULL) return 0 ;
	fclose(filep) ;

	mkdir("/tmp/gamesman", 0755) ;

	UncompressFile(outfilename, tempfilename) ;

	if((filep = fopen(tempfilename, "r")) == NULL) return 0 ;

	i = 0 ;
	while(i < gNumberOfPositions && !feof(filep))
	{
		fread(&temp, sizeof(int), 1, filep) ;
		if(temp % 4 == 3)
			i += temp/4 ;
		else
			gDatabase[i++] = temp ;
	}

	fclose(filep) ;

	remove(tempfilename) ;

	return 1 ;
}

int BuildFreqTable(char fileName[], unsigned int freqTable[FREQTABLESIZE])
{
	FILE *fileHandle;
	long byteCount = 0;

	if((fileHandle = fopen(fileName, "r")) == NULL) return FALSE ;

	memset(freqTable, 0, FREQTABLESIZE*sizeof(unsigned int));

	while (!feof(fileHandle))
	{
		freqTable[fgetc(fileHandle)]++;
		++byteCount;
	}
	
	fclose(fileHandle);
	
	return byteCount-1;
}


int LocateTwoSmallestNodes(HuffTree *huffTree, int *first, int *second)
{
	int smallNodesFound = 0;
	int execFlag;
	int loopCounter;

	loopCounter = 0;
	execFlag = TRUE;

	while (execFlag == TRUE)
	{
		if (loopCounter == huffTree->nodeCount)
			execFlag = FALSE;

		if (  (huffTree->tree[loopCounter].parent == NONE) 
		   && (huffTree->tree[loopCounter].nodeUsed == TRUE) )
		{
			*first = loopCounter;
			execFlag = FALSE;
			smallNodesFound = 1;
		} 
		++loopCounter;
	}

	if (smallNodesFound == 0)
		return 0;

	execFlag = TRUE;

	while (execFlag == TRUE)
	{
		if (loopCounter == huffTree->nodeCount)
			execFlag = FALSE;

		if ((execFlag == TRUE)
		  && (huffTree->tree[loopCounter].parent == NONE)
		  && (huffTree->tree[loopCounter].nodeUsed == TRUE))
		{
			if ((smallNodesFound == 1) 
			  && (huffTree->tree[loopCounter].frequency
			   > huffTree->tree[*first].frequency))
			{
				*second = loopCounter;
				smallNodesFound = 2;

			}
			else if (huffTree->tree[loopCounter].frequency
			         <= huffTree->tree[*first].frequency)
			{
				*second = *first;
				*first = loopCounter;
				smallNodesFound = 2;

			}
			else if ((huffTree->tree[loopCounter].frequency
				 <= huffTree->tree[*second].frequency)
				 && (smallNodesFound == 2))
			{
				*second = loopCounter;
			}
		} 
		++loopCounter;
	}

	return smallNodesFound;
}


void BuildHuffmanTree(unsigned int freqTable[FREQTABLESIZE], HuffTree *huffTree)
{
	int loopCounter;
	int smallest, secondSmallest;
	int index;

	memset(huffTree, 0, sizeof(HuffTree));

	for (loopCounter = 0; loopCounter < FREQTABLESIZE; ++loopCounter)
	{
		if (freqTable[loopCounter])
		{
			huffTree->tree[huffTree->nodeCount].nodeUsed = TRUE;
			huffTree->tree[huffTree->nodeCount].parent = NONE;
			huffTree->tree[huffTree->nodeCount].lChild = NONE;
			huffTree->tree[huffTree->nodeCount].rChild = NONE;
			huffTree->tree[huffTree->nodeCount].character = loopCounter;
			huffTree->tree[huffTree->nodeCount].frequency =
				freqTable[loopCounter];
			huffTree->encoderLUT[loopCounter] = huffTree->nodeCount;
			++huffTree->nodeCount;
		}
	}

	while (LocateTwoSmallestNodes(huffTree, &smallest, &secondSmallest) == 2)
	{
		index = huffTree->nodeCount; /* pos. of new node */
		huffTree->tree[index].nodeUsed = TRUE;
		huffTree->tree[smallest].parent = index;
		huffTree->tree[secondSmallest].parent = index;
		huffTree->tree[index].lChild = smallest;
		huffTree->tree[index].rChild = secondSmallest;
		huffTree->tree[index].parent = NONE;
		huffTree->tree[index].frequency = 
			huffTree->tree[smallest].frequency
		      + huffTree->tree[secondSmallest].frequency;
		++huffTree->nodeCount;
	}

	huffTree->rootNode = smallest;
}


int CompressFile(char inFileName[], char outFileName[])
{
	FILE *inFileHandle;
	FILE *outFileHandle;

	unsigned int freqTable[FREQTABLESIZE];   /* the frequency data */
	HuffTree huffTree;                       /* Huffman tree */
	long bytesInInputFile;                   /* stored in output file */
	int bitPointer;        /* used to track when to write a byteful of data out */
	char byteToBeWritten;
	int loopCounter;
	unsigned char charToEncode;   /* the character read in, to be encoded */
	int nodePointer;         /* Huffman tree pointer used in encoding chars */
	int parentNode;          /* the parent of the current node */
	char buffer[512];
	char bufferPointer;

	long magicNumber = MAGIC;  /* used to verify files are huff-compressed */


	bitPointer = 0;

	inFileHandle = fopen(inFileName, "r");
	if (inFileHandle == '\0') return FALSE;

	outFileHandle = fopen(outFileName, "w");
	if (outFileHandle == '\0') return FALSE;

	bytesInInputFile = BuildFreqTable(inFileName, freqTable);
	if (bytesInInputFile == 0) return FALSE;

	BuildHuffmanTree(freqTable, &huffTree);

	fwrite(&magicNumber, sizeof(long), 1, outFileHandle);          /* magic */
	fwrite(&bytesInInputFile, sizeof(long), 1, outFileHandle);     /* orig. file size */
	fwrite(freqTable, sizeof(int), FREQTABLESIZE, outFileHandle);  /* freq table */

	while (!feof(inFileHandle))
	{
		charToEncode = fgetc(inFileHandle);

		nodePointer = huffTree.encoderLUT[(int)charToEncode];

		bufferPointer = 0;    /* init buffer ptr */

		while (huffTree.tree[nodePointer].parent != NONE)
		{
			parentNode = huffTree.tree[nodePointer].parent;

			if (nodePointer == huffTree.tree[parentNode].lChild)
			{
				buffer[(int)bufferPointer++] = FALSE;
			}
			else
			{
				assert(nodePointer == huffTree.tree[parentNode].rChild);
				buffer[(int)bufferPointer++] = TRUE;
			}
			nodePointer = parentNode;  /* move up one node */
		}

		for (loopCounter = bufferPointer-1; loopCounter >= 0; --loopCounter)
		{
			byteToBeWritten = byteToBeWritten << 1;  /* bitwise shift left */

			if (buffer[loopCounter])
				byteToBeWritten = byteToBeWritten | 1;
			++bitPointer;
			if (bitPointer == 8)
			{
				fputc(byteToBeWritten, outFileHandle);
				byteToBeWritten = 0;
				bitPointer = 0;
			}
		}
	}

	fclose(inFileHandle);
	fclose(outFileHandle);

	return TRUE;
}

int UncompressFile(char inFileName[], char outFileName[])
{
	FILE *inFileHandle;
	FILE *outFileHandle;

	HuffTree huffTree;                      /* Huffman tree */
	unsigned int freqTable[FREQTABLESIZE];  /* freq table */
	long bytesInOutputFile;     /* loaded in from input file */
	int bitPointer;   /* used to track when to read in the next byte */
	char outChar;
	char charToDecode;    /* the character to be decoded and written out */
	int nodePointer;      /* Huffman tree pointer used in decoding chars */
	long magicNumber;    /* used to verify input file is huff-compressed */

	inFileHandle = fopen(inFileName, "r");
	if (inFileHandle == '\0') return FALSE;

	outFileHandle = fopen(outFileName, "w");
	if (outFileHandle == '\0') return FALSE;

	fread(&magicNumber, sizeof(long), 1, inFileHandle);
	if (magicNumber != MAGIC) return FALSE;

	fread(&bytesInOutputFile, sizeof(long), 1, inFileHandle); /* size of output file */

	fread(freqTable, sizeof(int), FREQTABLESIZE, inFileHandle);

	BuildHuffmanTree(freqTable, &huffTree);

	charToDecode = fgetc(inFileHandle);
	bitPointer = 0;

	while ((!feof(inFileHandle)) && (bytesInOutputFile > 0))
	{
		nodePointer = huffTree.rootNode;

		while ((huffTree.tree[nodePointer].lChild != NONE) ||
		       (huffTree.tree[nodePointer].rChild != NONE))
		{
			if (charToDecode & 128)
			{
				nodePointer = huffTree.tree[nodePointer].rChild;
			}
			else
			{
				nodePointer = huffTree.tree[nodePointer].lChild;
			}

			charToDecode <<= 1;   /* shift, to get the next bit */
			++bitPointer;

			/* have we run out of bits in this byte?  Get another byte */
			if (bitPointer == 8)
			{
				bitPointer = 0;
				charToDecode = fgetc(inFileHandle);
			}

		}
		outChar = huffTree.tree[nodePointer].character;
		fputc(outChar, outFileHandle);
		--bytesInOutputFile;
	}

	fclose(inFileHandle);
	fclose(outFileHandle);

	return TRUE;
}

int main(int argc, char ** argv)
{
	Initialize();

	if(argc == 1)
	{
		gPrintPredictions = TRUE ;
		Menus();
	}
	else if(!strcmp(argv[1], "-s"))
	{
		if(argc != 3)
			printf("Usage %s [-s <n>|-n]\n", argv[0]) ;
		else
		{
			int a ;
			sscanf(argv[2], "%d", &a) ;
			SolveAndStore(a) ;
		}
	}
	else if(!strcmp(argv[1], "-numberOfOptions"))
		printf("%d", NumberOfOptions()) ;
	else if(!strcmp(argv[1], "-numberOfPositions"))
	{
		int gameoption ;
		sscanf(argv[2], "%d", &gameoption) ;
		setOption(gameoption) ;
		printf("'%s' has %d positions.", kGameName, gNumberOfPositions) ;
	}
	else if(!strcmp(argv[1], "-PrintPosition"))
	{
		int myposition ;
		char playername[256] ;
		int kPlayersTurn ;
		int gameoption ;

		sscanf(argv[2], "%d", &gameoption) ;
		setOption(gameoption) ;
		sscanf(argv[3], "%d", &myposition) ;
		sscanf(argv[4], "%s", playername) ;
		sscanf(argv[5], "%d", &kPlayersTurn) ;
		PrintPosition(myposition, playername, kPlayersTurn) ;
	}
	else if(!strcmp(argv[1], "-DoMove"))
	{
		int myposition ;
		int mymove ;
		int gameoption ;

		sscanf(argv[2], "%d", &gameoption) ;
		sscanf(argv[3], "%d", &myposition) ;
		sscanf(argv[4], "%d", &mymove) ;
		setOption(gameoption) ;

		printf("%d", DoMove(myposition, mymove)) ;
	}
	else if(!strcmp(argv[1], "-GenerateMoves"))
	{
		int myposition ;
		int gameoption ;
		MOVELIST * head = NULL ;
		MOVELIST * tail ;
		int count = 0 ;

		sscanf(argv[2], "%d", &gameoption) ;
		sscanf(argv[3], "%d", &myposition) ;

		setOption(gameoption) ;

		tail = head = (MOVELIST*)GenerateMoves(myposition) ;

		while(tail != NULL)
		{
			count++ ;
			tail = tail -> next ;
		}
		printf("%d", count) ;
		tail = head ;
		while(tail != NULL)
		{
			printf("|%d", tail -> move) ;
			tail = tail -> next ;
		}
	}
	else if(!strcmp(argv[1], "-Primitive"))
	{
		int myposition ;
		int gameoption ;

		sscanf(argv[2], "%d", &gameoption) ;
		sscanf(argv[3], "%d", &myposition) ;

		setOption(gameoption) ;

		printf("%d", Primitive(myposition)) ;
	}
	else if(!strcmp(argv[1], "-GoAgain"))
	{
		int myposition ;
		int gameoption ;
		int mymove ;

		sscanf(argv[2], "%d", &gameoption) ;
		sscanf(argv[3], "%d", &myposition) ;
		sscanf(argv[4], "%d", &mymove) ;

		setOption(gameoption) ;

		printf("%d", gGoAgain(myposition, mymove)) ;
	}
	else if(!strcmp(argv[1], "-ConvertTextInputToMove"))
	{
		int gameoption ;
		int count ;
		char movestring[256] ;
		int i ;

		sscanf(argv[2], "%d", &gameoption) ;
		setOption(gameoption) ;

		strcpy(movestring, argv[3]) ;
		for(i = 4 ; i < argc ; i++)
		{
			strcat(movestring, " ") ;
			strcat(movestring, argv[i]) ;
		}
		printf("%d", ConvertTextInputToMove(movestring)) ;
	}
	else if(!strcmp(argv[1], "-PrintMove"))
	{
		int gameoption ;
		int move ;
		sscanf(argv[2], "%d", &gameoption) ;
		sscanf(argv[3], "%d", &move) ;
		setOption(gameoption) ;
		PrintMove(move) ;
	}
	else if(!strcmp(argv[1], "-RawDump"))
	{
		int gameoption ;
		sscanf(argv[2], "%d", &gameoption) ;
		SolveAndStoreRaw(gameoption) ;
	}
	else if(!strcmp(argv[1], "-RawRead"))
	{
		int gameoption ;
		char outfilename[256] ;
		FILE * filep ;
		int position ;
		int value ;
		sscanf(argv[2], "%d", &gameoption) ;
		sscanf(argv[3], "%d", &position) ;
		sprintf(outfilename, "data/m%s_%d_raw.dat", kDBName, gameoption) ;
		if((filep = fopen(outfilename, "r")) == NULL)
			printf("Unable to open file to read. Quitting.\n"), exit(1) ;
		fseek(filep, position*sizeof(int), SEEK_SET) ;
		fread(&value, sizeof(int), 1, filep) ;
		printf("%d", value) ;
		fclose(filep) ;
	}
	else
		printf("Usage %s [-s <n>|-n|-p]\n", argv[0]) ;

	return(1); /* this satisfies lint's complaint... */
}


/// FROM HERE, ATILLA's CODE ...

void hash_free()
{
	if(gHashOffset)
	{
		SafeFree(gHashOffset) ;
		gHashOffset = NULL ;
	}
	if(gNCR)
	{
		SafeFree(gNCR) ;
		gHashOffset = NULL ;
	}
}

int generic_hash_init(int boardsize, int minOs, int maxOs, int minXs, int maxXs)
{
	int i, j, temp, sofar, numOs, numXs;
	numOs = maxOs-minOs;
	numXs = maxXs-minXs;
	gHashBoardSize = boardsize;
	gHashOffset = (int*) SafeMalloc(sizeof(int) * ((numOs+1) * (numXs+1) + 1));
	for(i = 0; i < numOs * numXs; i++){
	  gHashOffset[i] = 0;
	}
	gHashOffsetSize = numOs * numXs;
	gHashMinMax[0] = minOs;
	gHashMinMax[1] = maxOs;
	gHashMinMax[2] = minXs;
	gHashMinMax[3] = maxXs;
	nCr_init(boardsize);
	for (i=0; i <= numOs; i++)
	{
		for (j = 0; j <= numXs; j++)
		{
			gHashOffset[i*(numXs+1) + j] = nCr((j+minXs) + (i+minOs), (i+minOs)) * nCr(boardsize, (minOs + minXs + i + j));
		}		
	}
	temp = 0;
	sofar = 0;
	for (i=0; i <= numOs; i++)
	{
		for (j = 0; j <= numXs; j++)
		{
			temp = gHashOffset[i*(numXs+1) + j];
			gHashOffset[i*(numXs+1) + j] = sofar;
			sofar += temp;
		}
	}
	gHashOffset[(numOs+1)*(numXs+1)] = sofar;
	gHashNumberOfPos = sofar;
	return gHashNumberOfPos ;
}

int generic_hash(char* board)
{
	int temp, i, numxs,  numos, numXs;
	int boardsize, maxnumxs, maxnumos;
	maxnumos = maxnumxs = 0;
	boardsize = gHashBoardSize;
	numXs = gHashMinMax[3]-gHashMinMax[2] + 1;
	for (i = 0; i < boardsize; i++)
	{
		if (board[i] == 'x')
		{
			maxnumxs++;
		}
		else if (board[i] == 'o')
		{
			maxnumos++;
		}
	}
	numxs = maxnumxs;
	numos = maxnumos;
	temp = gHashOffset[maxnumos*numXs + maxnumxs];
	for (i = 0; i < gHashBoardSize; i++)
	{

	  if (board[i] == 'b')
	    {
	      boardsize--;
	    } 
	  else if (board[i] == 'o')
	    {
	      temp += nCr(numxs + numos, numos) * nCr(boardsize - 1, numxs + numos);
	      boardsize--;
	      numos--;
	    }
	  else
	    {
	      temp += nCr(numxs + numos, numos) * 
		nCr(boardsize - 1, numxs + numos) +
		nCr (numxs + numos - 1, numxs) *
		nCr(boardsize - 1,  numxs + numos - 1);
	      boardsize--;
	      numxs--;
	    }
	}
	return temp;
}

BOOLEAN generic_unhash(int hashed, char* dest)
{
	int i, j, offst, numxs, numos, temp, boardsize, width, height;
	offst = 0;
	j = 0;
	width = gHashMinMax[3]-gHashMinMax[2]+1;
	height = gHashMinMax[1]-gHashMinMax[0]+1;
	while (j <= height*width &&
	       hashed > gHashOffset[(j + 1)])
	  {
	    j++;
	  }	
	offst = gHashOffset[j];
	hashed -= offst;
	numxs = j % width;
	numos = (int) j / width;
	boardsize = gHashBoardSize;
	for (i = 0; i < gHashBoardSize; i++)
	{
	  temp = nCr(numxs + numos, numos) * 
	    nCr(boardsize - 1, numxs + numos);
	  if (numxs + numos <= 0){
	    dest[i] = 'b';
	  } else if (temp + nCr (numxs + numos - 1, numxs) * nCr(boardsize - 1, numxs + numos - 1) <= hashed)
	    {
	      hashed -= (temp + nCr (numxs + numos - 1, numxs) * nCr(boardsize - 1,  numxs + numos - 1));
	      dest[i] = 'x';
	      numxs--;
	    }
	  else if (temp <= hashed)
	    {
	      hashed -= temp;
	      dest[i] = 'o';
	      numos--;  
	    }
	  else
	    {
	      dest[i] = 'b';
	    }
	  boardsize--;
	}
	return TRUE;
}

int dartboard_hash_init(int boardsize, int minOs, int maxOs, int minXs, int maxXs)
{
	int i, j, temp, sofar, numOs, numXs;
	numOs = maxOs-minOs;
	numXs = maxXs-minXs;
	gHashBoardSize = boardsize;
	gHashOffset = (int*) SafeMalloc(sizeof(int) * (numOs +  numXs + 1));
	for(i = 0; i < numOs + numXs + 1; i++){
	  gHashOffset[i] = 0;
	}
	gHashOffsetSize = numOs + numXs + 1;
	gHashMinMax[0] = minOs;
	gHashMinMax[1] = maxOs;
	gHashMinMax[2] = minXs;
	gHashMinMax[3] = maxXs;
	nCr_init(boardsize);
	for (i=0; i <= numOs + numXs; i++)
	{
	  gHashOffset[i] = nCr((i/2+minXs) + (i/2 + i%2 + minOs), (i/2+minXs)) * nCr(boardsize, (i/2 + i%2 + minOs + i/2 + minXs ));
	}
	temp = 0;
	sofar = 0;
	for (i=0; i <= numOs + numXs; i++)
	{
	  temp = gHashOffset[i];
	  gHashOffset[i] = sofar;
	  sofar += temp;
	}
	gHashOffset[numOs + numXs + 1] = sofar;
	gHashNumberOfPos = sofar;
	return gHashNumberOfPos ;
}

int dartboard_hash(char* board)
{
	int temp, i, numxs,  numos, numXs;
	int boardsize, maxnumxs, maxnumos;
	maxnumos = maxnumxs = 0;
	boardsize = gHashBoardSize;
	numXs = gHashMinMax[3]-gHashMinMax[2] + 1;
	for (i = 0; i < boardsize; i++)
	{
		if (board[i] == 'x')
		{
			maxnumxs++;
		}
		else if (board[i] == 'o')
		{
			maxnumos++;
		}
	}
	numxs = maxnumxs;
	numos = maxnumos;
	if (numxs > numos){
	  temp = gHashOffset[numos*2+1];
	} else {
	  temp = gHashOffset[numos*2];
	}
	for (i = 0; i < gHashBoardSize; i++)
	{
	  if (board[i] == 'b')
	    {
	      boardsize--;
	    } 
	  else if (board[i] == 'o')
	    {
	      temp += nCr(numxs + numos, numos) * nCr(boardsize - 1, numxs + numos);
	      boardsize--;
	      numos--;
	    }
	  else
	    {
	      temp += nCr(numxs + numos, numos) * 
		nCr(boardsize - 1, numxs + numos) +
		nCr (numxs + numos - 1, numxs) *
		nCr(boardsize - 1,  numxs + numos - 1);
	      boardsize--;
	      numxs--;
	    }
	}
	return temp;
}

BOOLEAN dartboard_unhash(int hashed, char* dest)
{
	int i, j, offst, numxs, numos, temp, boardsize, width, height;
	offst = 0;
	j = 0;
	while (j <= gHashOffsetSize &&
	       hashed > gHashOffset[j + 1])
	  {
	    j++;
	  }	
	offst = gHashOffset[j];
	hashed -= offst;
	numxs = j / 2 + j % 2;
	numos = j / 2;
	boardsize = gHashBoardSize;
	for (i = 0; i < gHashBoardSize; i++)
	{
	  temp = nCr(numxs + numos, numos) * 
	    nCr(boardsize - 1, numxs + numos);
	  if (numxs + numos <= 0){
	    dest[i] = 'b';
	  } else if (temp + nCr (numxs + numos - 1, numxs) * nCr(boardsize - 1, numxs + numos - 1) <= hashed)
	    {
	      hashed -= (temp + nCr (numxs + numos - 1, numxs) * nCr(boardsize - 1,  numxs + numos - 1));
	      dest[i] = 'x';
	      numxs--;
	    }
	  else if (temp <= hashed)
	    {
	      hashed -= temp;
	      dest[i] = 'o';
	      numos--;  
	    }
	  else
	    {
	      dest[i] = 'b';
	    }
	  boardsize--;
	}
	return TRUE;
}

int rearranger_hash_init(int boardsize, int numOs, int numXs)
{
  gHashMinMax[0] = numOs;
  gHashMinMax[1] = numOs;
  gHashMinMax[2] = numXs;
  gHashMinMax[3] = numXs;
  gHashBoardSize = boardsize;
  nCr_init(boardsize);
  gHashNumberOfPos = nCr(numXs+numOs, numXs) * nCr(boardsize, numXs+numOs);   
  return gHashNumberOfPos ;
}

int rearranger_hash(char* board)
{
	int temp, i, numxs,  numos, numXs;
	int boardsize;
	numxs = gHashMinMax[3];
	numos = gHashMinMax[1];
	boardsize = gHashBoardSize;
	temp = 0;
	for (i = 0; i < gHashBoardSize; i++)
	{
	  if (board[i] == 'b')
	    {
	      boardsize--;
	    } 

	  else if (board[i] == 'o')
	    {
	      temp += nCr(numxs + numos, numos) * nCr(boardsize - 1, numxs + numos);
	      boardsize--;
	      numos--;
	    }
	  else
	    {
	      temp += nCr(numxs + numos, numos) * 
		nCr(boardsize - 1, numxs + numos) +
		nCr (numxs + numos - 1, numxs) *
		nCr(boardsize - 1,  numxs + numos - 1);
	      boardsize--;
	      numxs--;
	    }
	}
	return temp;
}

BOOLEAN rearranger_unhash(int hashed, char* dest)
{
	int i, j, offst, numxs, numos, temp, boardsize;
	j = 0;
	boardsize = gHashBoardSize;
	numxs = gHashMinMax[3];
	numos = gHashMinMax[1];
	for (i = 0; i < gHashBoardSize; i++)
	{
	  temp = nCr(numxs + numos, numos) * 
	    nCr(boardsize - 1, numxs + numos);
	  if (numxs + numos <= 0){
	    dest[i] = 'b';
	  } else if (temp + nCr (numxs + numos - 1, numxs) * nCr(boardsize - 1, numxs + numos - 1) <= hashed)
	    {
	      hashed -= (temp + nCr (numxs + numos - 1, numxs) * nCr(boardsize - 1,  numxs + numos - 1));
	      dest[i] = 'x';
	      numxs--;
	    }
	  else if (temp <= hashed)
	    {
	      hashed -= temp;
	      dest[i] = 'o';
	      numos--;  
	    }
	  else
	    {
	      dest[i] = 'b';
	    }
	  boardsize--;
	}
	return TRUE;
}

void nCr_init(boardsize)
{
	int i, j;
	gNCR = (int*) SafeMalloc(sizeof(int) * (boardsize + 1) * (boardsize + 1));
	for(i = 0; i<= boardsize; i++)
	{
		gNCR[i*(boardsize+1)] = 1;
		gNCR[i*(boardsize+1) + i] = 1;
	}
	for(i = 1; i<= boardsize; i++)
	{
		for(j = 1; j < i ; j++)
		{
			gNCR[i*(boardsize+1) + j] = gNCR[(i-1)*(boardsize+1) + j-1] + gNCR[(i-1)*(boardsize+1) + j];
		}
	}
}

int nCr(int n, int r)
{
  return gNCR[n*(gHashBoardSize+1) + r];
}

// Start Gameline functions
#include "gamesman.h"

////// SUNIL's CLIENT/SERVER CODE
#define MAXLINE 4096

#define max(a,b) (((a)>(b))?(a):(b))

int getLength(void * line)
{
	short ret ;
	memcpy(&ret, line, 2) ;
	ret = ntohs(ret) ;
	return (int)ret ;
}

void setLength(void * line, short length)
{
	length = htons(length) ;
	memcpy(line, &length, 2) ;
}

int stuff(char * source, char * dest)
{
	int a = strlen(source) ;
	memcpy(dest+2, source, a) ;
	setLength(dest, a+2) ;
	return a+2 ;
}

int InitClient()
{
	struct sockaddr_in servaddr ;
	struct hostent * he ;
	char hostname[256] ;
	short port ;
	int iport ;
	FILE * filep ;

	if(!(filep = fopen(hostname, "r")))
		printf("Unable to open server.txt. Please reinstall Gameline\n"), exit(1) ;

	fgets(hostname, 254, filep) ;
	if(hostname[strlen(hostname)-1] == (char)NULL)
		hostname[strlen(hostname)-1] = (char)NULL ;
	fscanf(filep, "%d", &iport) ;
	port = (short)iport ;

	fclose(filep) ;

	if((he = gethostbyname(hostname)) == NULL)
	{
		printf("sunil: gethostbyname failed\n") ;
		return 0 ;
	}
	if(port <= 0)
	{
		printf("sunil: port <= 0\n") ;
		return 0 ;
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0) ;

	bzero(&servaddr, sizeof(servaddr)) ;
	servaddr.sin_family = AF_INET ;
	servaddr.sin_port = htons(port) ;
	servaddr.sin_addr = *((struct in_addr *)he->h_addr) ;
	memset(&(servaddr.sin_zero), '\0', 8) ;  // zero the rest of the structure

	if(connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1)
	{
		printf("gameline: connect failed\n") ;
		return 0 ;
	}
}

ssize_t writen(int fd, const void *vptr, size_t n)
{
	size_t		nleft;
	ssize_t		nwritten;
	const char	*ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0)
	{
		if ( (nwritten = write(fd, ptr, nleft)) <= 0)
		{
			if (errno == EINTR)
				nwritten = 0;		/* and call write() again */
			else
				return(-1);			/* error */
		}

		nleft -= nwritten;
		ptr   += nwritten;
	}
	return(n);
}

ssize_t readn(int fd, void *vptr, size_t n)
{
	size_t	nleft;
	ssize_t	nread;
	char	*ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0)
	{
		if ( (nread = read(fd, ptr, nleft)) < 0)
		{
			if (errno == EINTR)
				nread = 0;		/* and call read() again */
			else
				return(-1);
		}
		else if (nread == 0)
			break;				/* EOF */

		nleft -= nread;
		ptr   += nread;
	}
	return(n - nleft);		/* return >= 0 */
}
