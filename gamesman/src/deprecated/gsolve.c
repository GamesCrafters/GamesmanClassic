/************************************************************************
**
** NAME:        gsolve.c
**
** DESCRIPTION: The source code and heart of a Master's project
**              entitled GAMESMAN, which is a polymorphic perfect-
**              information game generator.
**
** AUTHOR:      Dan Garcia  -  University of California at Berkeley
**              Copyright (C) Dan Garcia, 1995. All rights reserved.
**
** DATE:        07/15/91
**
** UPDATE HIST:
**
** 07-17-91 0.9a1 : Implemented the undo facility, PrintPosition()
** 08-23-91 0.9a2 : Restructured PlayAgainstComputer to make cleaner & 
**                  smaller as well as cleaned up the undo facilities.
** 08-25-91 0.9a3 : Wrote PlayAgainstHuman,  & included the names of the
**                  players. Rewrote the Undo code to include the calls
**                  to Visited, MarkAsVisited, and UnMarkAsVisited (all new)
**                  Removed the 1210-specific calls. Changed Positionlist
**                  to Movelist. Modularize into separate files.
** 08-27-91 0.9a4 : Remove the typedefs from solve.h.
** 08-28-91 1.0a0 : Wrote GetRandomNumber() for random playing, wrote ttt.c
** 09-04-91 1.0a1 : Added version numbers, kSolveVersion
**                  Added PrintPossibleMoves() and ValidMove()
**                  Can now handle a Primitive that returns a win.
** 09-06-91 1.0a2 : Now it says (Dan should win) instead of (win)
**                  Fixed a lot of lint problems, lint is now much happier.
**                  Made the program say "Evaluating...done" when evaluating.
**                  Recoded the way to do "visited" - bitmask
**                  The user may now choose to go first with a lose game.
** 09-07-91 1.0a3 : All mallocs are now SafeMallocs, checking for NULL.
**                  Made playerName global and called it gPlayerName.
**                  Fixed bug in DetermineValue which resulted in some 
**                  positions never being evaluated. Made gInitialPosition
**                  global, and added Symmetry code to ttt.c and gSymmetry
** 09-12-91 1.0a4 : Made the Move database contain a linked list of moves.
** 09-13-91 1.0a5 : Added the following to solve.c: StoreInDatabase(), 
**                  GetFromDatabase(), MarkAsVisited(), UnmarkAsVisited,
**                  and Visited(), and had them call GetRawValueFromDatabase().
** 09-16-91 1.0a6 : Made the linked list of moves standard.
** 09-17-91 1.0a7 : Incorporated Help Facility, Graphic input/output into ttt.c
** 09-18-91 1.0a8 : Improved the interface to include help, added Nim!
** 09-19-91 1.0a9 : Fixed help, fixed nim a bit.
** 09-23-91 1.0a10: Fixed the Stopwatched time using FormatTime, added the
**                  local calls to GameSpecificMenu(), split up ParseMenuChoice.
** 09-25-91 1.0a11: Made a ^L call to SOLVE, and HitAnyKeyToContinue()
** 09-30-91 1.0a12: Added hints and prediction options & nim I/O fix
** 10-17-91 1.0a13: Renamed it to GAMESMAN, and cleaned up menus.
** 11-13-91 1.0a14: Removed the symmetry menu - made it game-specific. Also
**                  added preliminary help facilities.
** 11-18-91 1.0a15: Added serious help facilities.
** 11-20-91 1.0a16: Added more help.
** 11-22-91 1.0a17: Changed nim to 4x4 and added the facility to get a random
**                  gInitialPosition and play with that.
** 02-03-92 1.0a18: Changed the default graphics boolean to false
** 05-03-92 1.0a19: Implemented DebugModule. Fixed the Determinevalue bug
**                  that resulted in dodgem to be a tie game. Revamped the
**                  textual interface.
** 05-07-92 1.0a20: Fixed typo in beginning credits and set them to be a 
**                  string kOpeningCredits, defined in strings.h
** 05-12-92 1.0a21: Added kSupportsHeuristic, gExhaustiveSearch, menu
**                  to support the addition of Heuristics!
** 05-13-92 1.0a22: Added AnalysisMenu and TestStaticEvaluator
** 05-15-92 1.0a23: Fixed the AnalysisMenu to evaluate heuristic better
** 06-08-92 1.0a24: Created X and non-X applications and removed request
** 06-23-92 1.0a25: Fixed GetPlayersName and wrote GetMyString, fixed name
**                  bug created when undo was chosen with 2 human players
** 05-15-95 1.0   : Final release code for M.S.
**
**************************************************************************/
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
/*#include <sys/time.h>*/

#include "gsolve.h"
#include "gstrings.h"

/*************************************************************************
**
** Global Variables. These are constants for each game and parameters 
** about each game as well.
**
**************************************************************************/

VALUE   gValue;                  /* The value of the game */
MENU    gMenuMode;               /* The state of the main menu */
BOOLEAN gAgainstComputer;        /* TRUE iff the user is playing the computer */
BOOLEAN gHumanGoesFirst;         /* TRUE iff the user goes first vs. computer */
BOOLEAN gStandardGame;           /* TRUE iff game is STANDARD (not REVERSE) */
BOOLEAN gExhaustiveSearch;       /* TRUE iff game uses Exhaustive Search to solve */
BOOLEAN gSymmetries;             /* TRUE iff the symmetries should be searched */
BOOLEAN gPredictions;            /* TRUE iff the predictions should be printed */
BOOLEAN gPossibleMoves;          /* TRUE iff possible moves should be printed */
char    gPlayerName[2][MAXNAME]; /* The names of the players user/user or comp/user */
int     gLookAheadLevel;         /* The lookahead level for fuzzy heuristics */
int     gBytesMalloced;          /* The number of bytes SafeMalloc requested */
MEX    *gMexVals;                /* The array which stores the Mex vals */
POSITION  gHeadFR;               /* The FRontier Head position */
POSITION  gTailFR;               /* The FRontier Tail position */
POSITION *gPrevFR;               /* The FRontier Prev array */
POSITION *gNextFR;               /* The FRontier Next array */
POSITIONLIST **gParents;         /* The Parent of each node in a list */
char *gNumberChildren;           /* The Number of children (used for Loopy games) */

char *gValueString[] = {
	"Win", "Lose", "Tie", "Undecided",
	"Win-Visited", "Lose-Visited", "Tie-Visited", "Undecided-Visited"
};

BOOLEAN DefaultGoAgain(POSITION pos,MOVE move)
{
  return FALSE; /* Always toggle turn by default */
}

BOOLEAN (*gGoAgain)(POSITION,MOVE)=DefaultGoAgain;
        
/*************************************************************************
**
** Everything below here references variables that must be in every game file
**
**************************************************************************/

extern POSITION gInitialPosition;      /* The initial position of the game */
extern POSITION kBadPosition;          /* A POSITION that will never be used */

extern STRING   kGameName;             /* The Name of the Game */
extern STRING   kHelpGraphicInterface; /* The Graphical Interface Help string. */
extern STRING   kHelpTextInterface;    /* The Help for Text Interface string. */
extern STRING   kHelpOnYourTurn;       /* The Help for Your turn string. */
extern STRING   kHelpStandardObjective;/* The Help for Objective string. */
extern STRING   kHelpReverseObjective; /* The Help for reverse Objective string. */
extern STRING   kHelpTieOccursWhen;    /* The Help for Tie occuring string. */
extern STRING   kHelpExample;          /* The Help for Exmaples string. */
extern BOOLEAN  kPartizan;             /* TRUE <==> module is a Partizan game */
extern BOOLEAN  kSupportsHeuristic;    /* TRUE <==> module supports a heuristic */
extern BOOLEAN  kSupportsSymmetries;   /* TRUE <==> module supports symmetries */
extern BOOLEAN  kSupportsGraphics;     /* TRUE <==> module supports graphics */
extern BOOLEAN  kDebugMenu;            /* TRUE <==> module supports DebugMenu() */
extern BOOLEAN  kGameSpecificMenu;     /* TRUE <==> module supports GameSpecificMenu() */
extern BOOLEAN  kTieIsPossible;        /* TRUE <==> A Tie is possible */
extern BOOLEAN  kLoopy;                /* TRUE <==> Game graph has cycles */
extern BOOLEAN  kDebugDetermineValue;  /* TRUE <==> Print visited nodes */

extern int      gNumberOfPositions;    /* The number of positions in the game */

/************************************************************************
**
** NAME:        Initialize
**
** DESCRIPTION: Initialize all of the global variables that need it. 
**
** CALLS:       InitializeDatabases
**
************************************************************************/
 
Initialize()
{
  srand(time(NULL));

  gValue            = undecided;
  gMenuMode         = BeforeEvaluation;
  gAgainstComputer  = TRUE;
  gStandardGame     = TRUE;
  gExhaustiveSearch = TRUE;
  gSymmetries       = FALSE;
  gPredictions      = TRUE;
  gPossibleMoves    = FALSE;
  gHumanGoesFirst   = TRUE;
  gLookAheadLevel   = 2;
  gBytesMalloced    = 0;

  (void) sprintf(gPlayerName[kPlayerOneTurn],"Dan");
  (void) sprintf(gPlayerName[kPlayerTwoTurn],"Computer");

  /* Store mex values for impartial games */
  if(!kPartizan) { 
    MexInitialize(); /* Set Mex values for all positions to kBadMexValue */
  }

  if(kLoopy) {
    InitializeFR();
    ParentInitialize();
    NumberChildrenInitialize();
  }
}

/************************************************************************
**
** NAME:        Menus
**
** DESCRIPTION: The main menu-driven driver. It simply prints it pretty.
** 
** CALLS:       MenusBeforeEvaluation()
**              MenusEvaluated()
**              ParseMenuChoice()
**              HitAnyKeyToContinue()
**              BadElse()
**
************************************************************************/

Menus()
{
  char GetMyChar();

  printf(kOpeningCredits,kSolveVersion,kGameName);

  HitAnyKeyToContinue();
  
  do {
    printf("\n\t----- GAMESMAN version %s with %s module -----\n", 
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

/************************************************************************
**
** NAME:        MenusBeforeEvaluation
**
** DESCRIPTION: The menu-driver for BeforeEvaluation mode
** 
************************************************************************/

MenusBeforeEvaluation()
{
  printf("\n\ts)\t(S)TART THE GAME\n");

  printf("\n\tEvaluation Options:\n\n");
  printf("\to)\t(O)bjective toggle from %s to %s\n",
	 gStandardGame ? "STANDARD" : "REVERSE ",
	 gStandardGame ? "REVERSE " : "STANDARD");
  if(kSupportsSymmetries)
    printf("\t8)\tToggle storing symmetries from %s to %s\n",
	   gSymmetries ? "YES" : "NO",
	   gSymmetries ? "NO" : "YES");
  if(kDebugMenu)
    printf("\td)\t(D)ebug Module BEFORE Evaluation\n");
  if(kGameSpecificMenu)
    printf("\tg)\t(G)ame-specific options for %s\n",kGameName);
}

/************************************************************************
**
** NAME:        MenusEvaluated
**
** DESCRIPTION: The menu-driver for Evaluated mode
** 
** CALLS:       BadElse()
**
************************************************************************/

MenusEvaluated()
{
  printf("\n\tPlayer Name Options:\n\n");
      
  printf("\t1)\tChange the name of player 1 (currently %s)\n",gPlayerName[1]);
  printf("\t2)\tChange the name of player 2 (currently %s)\n",gPlayerName[0]);
  printf("\t3)\tSwap player 1 (plays FIRST) with player 2 (plays SECOND)\n");
      
  printf("\n\tGeneric Options:\n\n");

  printf("\t4)\tToggle from %sPREDICTIONS to %sPREDICTIONS\n",
	 gPredictions ? "   " : "NO ",
	 !gPredictions ? "   " : "NO ");
  printf("\t5)\tToggle from %sHINTS       to %sHINTS\n",
	 gPossibleMoves ? "   " : "NO ",
	 !gPossibleMoves ? "   " : "NO ");
  
  printf("\n\tPlaying Options:\n\n");
  printf("\t6)\tToggle opponent from a %s to a %s\n",
	 gAgainstComputer ? "COMPUTER" : "HUMAN",
	 gAgainstComputer ? "HUMAN" : "COMPUTER");
  if(gAgainstComputer ) {
    if(!gExhaustiveSearch)
      printf("\t7)\tToggle from going %s to %s\n",
	     gHumanGoesFirst ? "FIRST" : "SECOND",
	     gHumanGoesFirst ? "SECOND" : "FIRST");
    else if(gValue == tie)
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
    
    if(kSupportsHeuristic)
      printf("\te)\t(E)valuator toggle from %s to %s\n",
	     gExhaustiveSearch ? "EXHAUSTIVE SEARCH" : "HEURISTIC",
	     gExhaustiveSearch ? "HEURISTIC" : "EXHAUSTIVE SEARCH");
    if(kSupportsHeuristic)
      printf("\tl)\tChange (L)ookahead used for Heuristic (currently %d)\n",gLookAheadLevel);
    printf("\n\ta)\t(A)nalyze the game\n");
  }
  if(kDebugMenu)
    printf("\td)\t(D)ebug Game AFTER Evaluation\n");
  printf("\n\tp)\t(P)LAY GAME.\n");
}

/************************************************************************
**
** NAME:        ParseMenuChoice
**
** DESCRIPTION: The main menu parser. Depending on the mode you're in
**              (BeforeEvaluation or Evaluated) or whether the input
**              you typed is for a field that is constant regardless of
**              mode, then call certain individual parsers.
** 
** INPUT:       char c : The character the user typed in.
** 
** CALLS:       BOOLEAN ParseConstantMenuChoice()
**                      ParseBeforeEvaluationMenuChoice()
**                      ParseEvaluatedMenuChoice()
**                      BadElse()
**
************************************************************************/

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

/************************************************************************
**
** NAME:        ParseConstantMenuChoice
**
** DESCRIPTION: Parse the command chosen. If it was for a constant menu
**              choice, return TRUE because it was parsed here. Other-
**              wise return FALSE so it may be parsed elsewhere.
** 
** INPUTS:      char c  : The character the user typed in.
** 
** OUTPUTS:     BOOLEAN : TRUE <==> The character was parsed here.
** 
** CALLS:       ExitStageRight()
**              HelpMenus()
**
************************************************************************/

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

/************************************************************************
**
** NAME:        ParseBeforeEvaluationMenuChoice
**
** DESCRIPTION: Parse the command chosen while in BeforeEvaluation mode
** 
** INPUTS:      char c  : The character the user typed in.
** 
** CALLS:       GameSpecificMenu()
**              Stopwatch(int *, int *)
**              VALUE DetermineValue(POSITION)
**              BadMenuChoice()
**
************************************************************************/

ParseBeforeEvaluationMenuChoice(c)
     char c;
{
  VALUE DetermineValue(), DetermineLoopyValue();

  switch(c) {
  case 'G': case 'g':
    if(kGameSpecificMenu)
      GameSpecificMenu();
    else {
      BadMenuChoice();
      HitAnyKeyToContinue();
    }
    break;
  case 'o': case 'O':
    gStandardGame = !gStandardGame;
    break;
  case '8':
    if(kSupportsSymmetries)
      gSymmetries = !gSymmetries;
    else {
      BadMenuChoice();
      HitAnyKeyToContinue();
    }
    break;
  case 'D': case 'd':
    if(kDebugMenu)
      DebugModule();
    else
      BadMenuChoice();
    break;
  case 's': case 'S':
    if(gExhaustiveSearch) {
      printf("\nSolving with loopy code %s...%s!",kGameName,kLoopy?"Yes":"No");
      printf("\nRandom(100) three times %s...%d %d %d",kGameName,GetRandomNumber(100),GetRandomNumber(100),GetRandomNumber(100));
      printf("\nNumber of Bytes used in %s...%d",kGameName,gBytesMalloced);
      printf("\nInitializing insides of %s...", kGameName);
      /*      Stopwatch(&sec,&usec);*/
      (void) Stopwatch();
      InitializeDatabases();
      gBytesMalloced += (0 * sizeof(VALUE));
      gBytesMalloced += (0 * sizeof(MOVELIST *));
      printf("done in %d seconds!", Stopwatch());
      
      printf("\nNumber of Bytes used in %s...%d",kGameName,gBytesMalloced);
      printf("\nEvaluating the value of %s...", kGameName);

      if (kLoopy)
	gValue = DetermineLoopyValue(gInitialPosition);
      else
	gValue = DetermineValue(gInitialPosition);
      printf("done in %d seconds!", Stopwatch());
      printf("\nNumber of Bytes used in %s...%d",kGameName,gBytesMalloced);
      printf("\n\nThe Game %s has value: %s\n\n", kGameName, gValueString[(int)gValue]);
      gMenuMode = Evaluated;
      if(gValue == lose)
	gHumanGoesFirst = FALSE;
    }
    else {
      gPredictions = FALSE;
      gPossibleMoves = FALSE;
      gMenuMode = Evaluated;
      printf("\nPreparing Static Evaluator...done\n");
    }
    HitAnyKeyToContinue();
    break;
  default:
    BadMenuChoice();
    HitAnyKeyToContinue();
    break;
  }
}

/************************************************************************
**
** NAME:        ParseEvaluatedMenuChoice
**
** DESCRIPTION: Parse the command chosen while in Evaluated mode
** 
** INPUTS:      char c  : The character the user typed in.
** 
** CALLS:       DebugMenu()
**              PlayAgainstComputer()
**              PlayAgainstHuman()
**              BadMenuChoice()
**
************************************************************************/

ParseEvaluatedMenuChoice(c)
     char c;
{
  char tmpName[MAXNAME];
  
  switch(c) {
  case '1':
    printf("\nEnter the name of player 1 (max. 8 chars) [%s] : ",
		gPlayerName[kPlayerOneTurn]);
    GetMyString(tmpName,MAXNAME,TRUE,FALSE);
	if(strcmp(tmpName,""))
    	(void) sprintf(gPlayerName[kPlayerOneTurn],"%s",tmpName);
    break;
  case '2':
    printf("\nEnter the name of player 2 (max. 8 chars) [%s] : ",
		gPlayerName[kPlayerTwoTurn]);
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
    gPredictions = !gPredictions;
    break;
  case '5':
    gPossibleMoves = !gPossibleMoves;
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
  case 'E': case 'e':
    if(kSupportsHeuristic && gAgainstComputer)
      gExhaustiveSearch = !gExhaustiveSearch;
    else {
      BadMenuChoice();
      HitAnyKeyToContinue();
    }
    break;
  case 'L': case 'l':
    if(kSupportsHeuristic && gAgainstComputer) {
      printf("\nEnter the new search level: ");
      scanf("%d",&gLookAheadLevel);
    }
    else {
      BadMenuChoice();
      HitAnyKeyToContinue();
    }
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
  default:
    BadMenuChoice();
    HitAnyKeyToContinue();
    break;
  }
}

/************************************************************************
**
** NAME:        HelpMenus
**
** DESCRIPTION: The main menu-driven driver for Help, I need somebody,
**              help, I need someone, help, I need somebody, heeelp.
** 
** CALLS:       ExitStageRight()
**              InterfaceHelp()
**              GameInstructionsHelp()
**              ParseHelpMenuChoice()
**
************************************************************************/

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

/************************************************************************
**
** NAME:        ParseHelpMenuChoice
**
** DESCRIPTION: The main menu parser. Depending on the mode you're in
**              (BeforeEvaluation or Evaluated) or whether the input
**              you typed is for a field that is constant regardless of
**              mode, then call certain individual parsers.
** 
** INPUT:       char c : The character the user typed in.
** 
** CALLS:       BadElse()
**
************************************************************************/

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

/************************************************************************
**
** NAME:        BadMenuChoice
**
** DESCRIPTION: Print a standard "Bad Choice, hosehead" message.
**
************************************************************************/

BadMenuChoice()
{
  printf("\nSorry, I don't know that option. Try another.\n");
}

/************************************************************************
**
** NAME:        DebugModule
**
** DESCRIPTION: Debug Module help facility. This is a tool to aid the
**              development of a new module.
** 
************************************************************************/

DebugModule()
{
  char GetMyChar();
  int numberMoves = 0, move;
  POSITION DoMove(), GetInitialPosition();
  VALUE Primitive();
  MOVELIST *head = NULL, *ptr, *GenerateMoves();
  MOVE theMove;
  BOOLEAN haveMove = FALSE, tempPredictions = gPredictions;
  USERINPUT GetAndPrintPlayersMove();

  gPredictions = FALSE;
  
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
      gPredictions = tempPredictions;
      return;
    default:
      BadMenuChoice();
      HitAnyKeyToContinue();
      break;
    }
  } while(TRUE);
  
}


/************************************************************************
**
** NAME:        PlayAgainstHuman
**
** DESCRIPTION: Play the game against another user. The computer tells
**              the users about the value of the game, etc, and then
**              sets the board up and oversees their play.
** 
** CALLS:       InitializeUndo()
**              PrintPosition()
**              Primitive()
**              GetAndPrintPlayersMove()
**              HandleUndoRequest()
**              DoMove()
**              UpdateUndo()
**              ResetUndoList()
**
************************************************************************/

PlayAgainstHuman()
{
  POSITION currentPosition;
  MOVE theMove;
  VALUE Primitive();
  UNDO *undo, *InitializeUndo(), *HandleUndoRequest(), *UpdateUndo();
  BOOLEAN playerOneTurn = TRUE, error, abort;
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

    /* It's !playerOneTurn because the text (Dan will lose) is for the other */
    PrintPosition(currentPosition = DoMove(currentPosition,theMove),
		  gPlayerName[!playerOneTurn], kHumansTurn);

    undo = UpdateUndo(currentPosition, undo, &abort);
    if(abort)
      break;

    playerOneTurn = !playerOneTurn;		/* the other's turn */
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
  else if(userInput == Abort || abort)
    printf("Your abort command has been received and successfully processed!\n");
  else
    BadElse("PlayAgainstHuman"); 

  ResetUndoList(undo);
}

/************************************************************************
**
** NAME:        PlayAgainstComputer
**
** DESCRIPTION: Play the game against the computer. The computer plays
**              either second or first depending on the value of the
**              game. If the game is a win, the computer plays second.
**              If the game is a lose, the computer plays first. If the
**              game is a tie, the user is asked which he would rather
**              play, and the computer takes the other one. 
** 
** CALLS:       InitializeUndo()
**              PrintPosition(POSITION)
**              VALUE Primitive(POSITION)
**              GetAndPrintPlayersMove()
**              UNDO *HandleUndoRequest(*POSITION,*UNDO,*BOOLEAN)
**              POSITION DoMove(POSITION,MOVE)
**              MOVE GetComputersMove(POSITION)
**              PrintComputersMove(MOVE,STRING)
**              UNDO *UpdateUndo(POSITION,*UNDO)
**              ResetUndoList(*UNDO)
**              FUZZY GetHeuristicComputersMove(POSITION,MINIMAX,int,*MOVE)
**
************************************************************************/

PlayAgainstComputer()
{
  POSITION thePosition;
  MOVE theMove, GetComputersMove();
  VALUE Primitive();
  UNDO *undo, *InitializeUndo(), *HandleUndoRequest(), *UpdateUndo();
  BOOLEAN usersTurn, error, abort;
  FUZZY GetHeuristicComputersMove();
  USERINPUT userInput, GetAndPrintPlayersMove();

  thePosition = gInitialPosition;
  undo = InitializeUndo();
  usersTurn = gHumanGoesFirst;

  printf("\nOk, %s and %s, let us begin.\n\n",
	 gPlayerName[kPlayerOneTurn], gPlayerName[kPlayerTwoTurn]);

#ifndef X
  printf("Type '?' if you need assistance...\n\n");
#endif

  PrintPosition(thePosition,gPlayerName[usersTurn],usersTurn);

  while(Primitive(thePosition) == undecided) { /* Not dead yet! */

    if(usersTurn) {		/* User's turn */

      while((userInput = GetAndPrintPlayersMove(thePosition,
						&theMove, 
						gPlayerName[usersTurn])) == Undo) {
	undo = HandleUndoRequest(&thePosition,undo,&error); /* undo */
	PrintPosition(thePosition,gPlayerName[usersTurn],usersTurn);
      }

    }
    else {				/* Computer's turn */
      if(gExhaustiveSearch)
	theMove = GetComputersMove(thePosition);
      else if(kPartizan)
	(void) GetHeuristicComputersMove(thePosition,
					 ((gHumanGoesFirst && usersTurn) ||
					  (!gHumanGoesFirst && !usersTurn))
					 ? maximizing : minimizing,
					 gLookAheadLevel,&theMove);
      else /* IMPARTIAL => ALWAYS MINIMIZE */
	(void) GetHeuristicComputersMove(thePosition,minimizing,gLookAheadLevel,&theMove);
      PrintComputersMove(theMove,gPlayerName[usersTurn]);
    }
    if(userInput == Abort)
      break;                 /* jump out of while loop */

    /* It's !usersTurn because the prediction (Dan will lose) is for the other */
    PrintPosition(thePosition = DoMove(thePosition,theMove),
		  gPlayerName[!usersTurn],!usersTurn);

    undo = UpdateUndo(thePosition, undo, &abort);
    if(abort)
      break;

    usersTurn = !usersTurn;		/* The other person's turn */
  }
  if((Primitive(thePosition) == lose && usersTurn) || 
     (Primitive(thePosition) == win && !usersTurn))
    printf("\n%s wins. Nice try, %s.\n\n", gPlayerName[kComputersTurn],
	   gPlayerName[kHumansTurn]); 
  else if((Primitive(thePosition) == lose && !usersTurn) ||
	  (Primitive(thePosition) == win && usersTurn))
    printf("\nExcellent! You won!\n\n");
  else if(Primitive(thePosition) == tie)
    printf("The match ends in a draw. Excellent strategy, %s.\n\n", 
	   gPlayerName[kHumansTurn]);
  else if(userInput == Abort)
    printf("Your abort command has been received and successfully processed!\n");
  else
    BadElse("PlayAgainstHuman"); 

  ResetUndoList(undo);
}

/************************************************************************
**
** NAME:        ResetUndoList
**
** DESCRIPTION: Reset and free the space from the undo list.
** 
** INPUTS:      *UNDO    undo             : The undo pointer.
**
** CALLS:       HandleUndoRequest()
**
************************************************************************/

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

/************************************************************************
**
** NAME:        HandleUndoRequest
**
** DESCRIPTION: An undo was requested. Either tell the user that the
**              undo is impossible or perform the undo and set the 
**              thePosition to the correct position.
** 
** INPUTS:      *POSITION thePosition     : Pointer to current position.
**              *UNDO    undo             : The undo pointer.
**              *BOOLEAN  error           : Error if can't undo.
**
** OUTPUTS:     *UNDO                     : pointer to the new space.
**
** CALLS:       UnMarkAsVisited()
**
************************************************************************/

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

/************************************************************************
**
** NAME:        UpdateUndo
**
** DESCRIPTION: If the position has been seen before, check with the
**              user if he wants to quit or not. Otherwise, add the
**              new position to the undo list. In any case, return
**              with the pointer to the updated undo. (Except in the
**              case when the user quit when he was told he had a
**              stalemate.)
** 
** INPUTS:      POSITION thePosition     : The current position.
**              UNDO    *undo            : The previous undo pointer.
**              BOOLEAN *abort           : The abort pointer to fill.
**
** OUTPUTS:     *UNDO                    : pointer to the new space.
**
** CALLS:       Visited()
**              Stalemate()
**              MarkAsVisited()
**              GENERIC_PTR SafeMalloc
**
************************************************************************/

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
    undo->next = tmp;

    *abort = FALSE;
  }
  return(undo);
}

/************************************************************************
**
** NAME:        InitializeUndo
**
** DESCRIPTION: Initialize the Undo variable and return a pointer to it.
** 
** OUTPUTS:     *UNDO : pointer to the new space.
**
** CALLS:       GENERIC_PTR SafeMalloc
**
************************************************************************/

UNDO *InitializeUndo()
{
  UNDO *undo;
  GENERIC_PTR SafeMalloc();

  undo = (UNDO *) SafeMalloc (sizeof(UNDO));    /* Initialize the undo list */
  undo->position = gInitialPosition;
  undo->next = NULL;
  return(undo);
}

/************************************************************************
**
** NAME:        PrintHumanValueExplanation
**
** DESCRIPTION: Tell the users about the game they're about to play.
** 
************************************************************************/

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

/************************************************************************
**
** NAME:        PrintComputerValueExplanation
**
** DESCRIPTION: Tell the user about the game he/she is about to play.
** 
** CALLS:       BadElse()
** 
************************************************************************/

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

/************************************************************************
**
** NAME:        Stalemate
**
** DESCRIPTION: Ask the user if he/she would like to quit now since we
**              have already seen this position. If so, quit. Otherwise
**              clear the cycle from memory and return the pointer to
**              the undo list without the cycle.
** 
** INPUTS:      POSITION stalematePosition : The position we've already seen.
**              UNDO *undo                 : The linked list of past moves.
**              BOOLEAN *abort             : The abort pointer to fill.
**
** OUTPUTS:     (*UNDO) : a pointer to the linked list of undone moves.
**
** CALLS:       UnMarkAsVisited()
**              MarkAsVisited()
**
************************************************************************/

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
      tmp = undo;
      undo = undo->next;
      SafeFree((GENERIC_PTR)tmp);
    } 
    MarkAsVisited(undo->position);
    *abort = FALSE;
  }
  return(undo);
}

/************************************************************************
**
** NAME:        DetermineValue
**
** DESCRIPTION: Return the value of the input position by recursively
**              asking what the children of that position are in a DFS
**              manner. 
** 
** INPUTS:      POSITION position : The position to determine the value of.
**
** OUTPUTS:     (VALUE) an enum which is oneof: (win,lose,tie,undecided)
**
** CALLS:       Visited()
**              GetValueOfPosition()
**              Primitive()
**              StoreValueOfPosition()
**              MarkAsVisited()
**              GenerateMoves()
**              DetermineValue()
**              FreeMoveList()
**
************************************************************************/

VALUE DetermineValue(position)
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

  if(kDebugDetermineValue)
    printf("\nDV %d ", position);
  if(Visited(position)) { /* Cycle! */
    if(kDebugDetermineValue)
      printf("LOOP");
    return(win);

    /*	  return(tie);  This used to be what it was, but it fail on the
     **                   following case. Here is the edge-list:
     **                   a: b,c
     **                   b is unknown  a <-> c -> d (win)
     **                   c: a d         \
     **                   d is a win      -> b (unknown)
     **
     **  The reason if fails is the following chart.
     **
     **  Value for b    |    Value for a    |     Value for c
     **  ----------------------------------------------------
     **       L                  W                     L
     **       T                  T                     T
     **       W                  T                     T
     **
     **  Or, basically, the game can't handle when a position
     **  points back to an ancestor. Therefore the hack I've
     **  put in forces the computer to not repeat a position,
     **  as in 'go' by declaring that a repeat position is an
     **  immediate win for the other player.
     */
  }

  /* It's been seen before and value has been determined */
  else if((value = GetValueOfPosition(position)) != undecided) {
    if(kDebugDetermineValue)
      printf("visited, value = %s", gValueString[value]);
    return(value);
  } else if((value = Primitive(position)) != undecided) { 
    /* first time, end */
    if(kDebugDetermineValue)
      printf("primitive, value = %s", gValueString[value]);
    SetRemoteness(position,0); /* terminal positions have 0 remoteness */
    if(!kPartizan)
      MexStore(position,MexPrimitive(value)); /* lose=0, win=* */
    return(StoreValueOfPosition(position,value));
  /* first time, need to recursively determine value */
  } else {
    if(kDebugDetermineValue)
      printf("normal, continue searching");
    MarkAsVisited(position);
    if(!kPartizan)
      theMexCalc = MexCalcInit();
    head = ptr = GenerateMoves(position);
    while (ptr != NULL) {
      child = DoMove(position,ptr->move);  /* Create the child */
      value = DetermineValue(child);       /* DFS call */
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

/************************************************************************
**
** NAME:        MexAdd
**
** DESCRIPTION: Return a new MEXCALC var once you add the input theMex
**              value to the other mex values already stored in theMexCalc.
**              Note this is not the same as Nimsum, just Mex(a,b,c) but 
**              since we know a, b and c at different times, we first 
**              intialize theMexCalc, then add a, then add b, then add c.
** 
** INPUTS:      MEXCALC : The mex calculation var
** OUTPUTS:     MEX : The new mex number to add to the sum stored in MEXCALC
**
** OUTPUTS:     MEXCALC : The new result of the sum.
**
************************************************************************/

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

/************************************************************************
**
** NAME:        MexCompute
**
** DESCRIPTION: Return whatever Mex value is stored in the MEXCALC var
** 
** INPUTS:      MEXCALC : The mex calculation var
**
** OUTPUTS:     MEX : The actual mex variable stored in the calculation var
**
************************************************************************/

MEX MexCompute(theMexCalc)
MEXCALC theMexCalc;
{
  MEX ans = 0;
  while(theMexCalc & (1 << ans))
    ans++;
  return(ans);
}

/************************************************************************
**
** NAME:        MexCalcInit
**
** DESCRIPTION: Like a (New) procedure, return the initial value for a
**              MEXCALC variable, which is 0.
** 
** OUTPUTS:     MEXCALC : The initial Mex Calc value (0)
**
************************************************************************/

MEXCALC MexCalcInit()
{
  return((MEXCALC) 0);
}

/************************************************************************
**
** NAME:        MexStore
**
** DESCRIPTION: Store the input mex value into the MEX database at the
**              requested position.
** 
** INPUTS:      POSITION position : The position to store the mex value of
**              MEX theMex        : The mex value to store
**
************************************************************************/

MexStore(position,theMex)
POSITION position;
MEX theMex;
{
  gMexVals[(int)position] = theMex;
}

/************************************************************************
**
** NAME:        MexInitialize
**
** DESCRIPTION: Allocate & Reset MEX database to all invalid MEX positions
** 
************************************************************************/

MexInitialize()
{
  GENERIC_PTR SafeMalloc();
  int i;

  gMexVals = (MEX *) SafeMalloc (gNumberOfPositions * sizeof(MEX));
  for(i = 0; i < gNumberOfPositions; i++)
    gMexVals[i] = kBadMexValue;
}

/************************************************************************
**
** NAME:        MexLoad
**
** DESCRIPTION: Lookup the MEX value of the requested position and return it
** 
** INPUTS:      POSITION position : The position to find the mex value of
**
** OUTPUTS:     MEX : The mex value corresponding to the input position
**
************************************************************************/

MEX MexLoad(position)
POSITION position;
{
  return(gMexVals[(int)position]);
}

/************************************************************************
**
** NAME:        MexFormat
**
** DESCRIPTION: Stuff the input string argument with a formatted string
**              which looks like:
**              "[Val = 0]"
**              "[Val = *]"
**              "[Val = *n]" (where n is greater than 1)
** 
** INPUTS:      POSITION position : The position to format the mex value of
**              STRING string     : The string to stuff with the answer
**
************************************************************************/

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

/************************************************************************
**
** NAME:        MexPrimitive
**
** DESCRIPTION: The mex value for primitive position values.
**              value = lose, Mex value returned = 0
**              value = win,  Mex value returned = 1 (*)
**              Error for all other values (tie, undecided, other)
** 
** INPUTS:      VALUE value : The value of the position
**
** OUTPUTS:     MEX : The mex value corresponding to the input value
**
************************************************************************/

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

/************************************************************************
**
** NAME:        FreeMoveList
**
** DESCRIPTION: Free every element in the linked list pointed to by ptr
** 
** INPUTS:      MOVELIST *ptr : The pointer to the head of the list.
**
************************************************************************/

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

/************************************************************************
**
** NAME:        FreePositionList
**
** DESCRIPTION: Free every element in the linked list pointed to by ptr
** 
** INPUTS:      POSITIONLIST *ptr : The pointer to the head of the list.
**
************************************************************************/

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

/************************************************************************
**
** NAME:        GetRandomNumber
**
** DESCRIPTION: Return a random integer between 0 and n-1, inclusive.
** 
** INPUTS:      INT n : The limits of the random generation.
**
** OUTPUTS:     INT: A random number between 0 and n-1.
**
************************************************************************/

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

/************************************************************************
**
** NAME:        GetSmallRandomNumber
**
** DESCRIPTION: Return a random integer between 0 and n-1, inclusive.
** 
** INPUTS:      INT n : The limits of the random generation.
**              ASSUMPTION - n is smaller than RAND_MAX
**
** OUTPUTS:     INT: A random number between 0 and n-1.
**
************************************************************************/

int GetSmallRandomNumber(n)
     int n;
{
    return(n * ((double)randSafe()/RAND_MAX));
}

/************************************************************************
**
** NAME:        randSafe
**
** DESCRIPTION: Return a random integer between 0 and RAND_MAX-1
**              rand() returned between 0 and RAND_MAX, we just call
**              it until it doesn't return RAND_MAX
** 
** OUTPUTS:     INT: A random number between 0 and RAND_MAX-1
**
************************************************************************/

int randSafe()
{
  int ans;
  while((ans = rand()) == RAND_MAX)
    ;
  return(ans);
}

/************************************************************************
**
** NAME:        ValidMove
**
** DESCRIPTION: Return TRUE iff theMove is in GenerateMoves list.
** 
** INPUTS:      POSITION *thePosition : The position the user is at. 
**              MOVE *theMove         : The move to check the validity of. 
**
** OUTPUTS:     BOOLEAN: True iff the move is valid.
**
** CALLS:       GenerateMoves()
**              FreeMoveList(ptr)
**
************************************************************************/

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

/************************************************************************
**
** NAME:        PrintPossibleMoves
**
** DESCRIPTION: Print the possible moves a user can choose from.
** 
** INPUTS:      POSITION *thePosition : The position the user is at. 
**
** OUTPUTS:     BOOLEAN: True always as a hack to GetAndPrintPlayersMove.
**
** CALLS:       GenerateMoves()
**              PrintMove(ptr)
**              FreeMoveList(ptr)
**
************************************************************************/

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

/************************************************************************
**
** NAME:        PrintValueEquivalentMoves
**
** DESCRIPTION: Print the value-equivalent moves a user can choose from.
** 
** INPUTS:      POSITION *thePosition : The position the user is at. 
**
** CALLS:       GetValueEquivalentMoves()
**              PrintMove(ptr)
**              FreeMoveList(ptr)
**
************************************************************************/

PrintValueEquivalentMoves(thePosition)
     POSITION thePosition;
{
  MOVELIST *ptr, *head, *GetValueEquivalentMoves();

  head = ptr = GetValueEquivalentMoves(thePosition);
  printf("\nHere are some 'safe' moves   : [ ");
  while (ptr != NULL) {
    PrintMove(ptr->move);
    printf(" ");
    ptr = ptr->next;
  }
  printf("]\n\n");
  FreeMoveList(head);
}

/************************************************************************
**
** NAME:        GetPrediction
**
** DESCRIPTION: Return the string describing the prediction of the game.
** 
** INPUTS:      POSITION position   : The position to get prediction
**              STRING   playerName : The name of the player.
**              BOOLEAN  usersTurn  : TRUE <==> it's a user's turn.
**
** OUTPUTS:     STRING: True always as a hack to GetAndPrintPlayersMove.
**
************************************************************************/

STRING GetPrediction(position,playerName,usersTurn)
     POSITION position;
     STRING playerName;
     BOOLEAN  usersTurn;
{
  static char prediction[80];
  char mexString[20];
  REMOTENESS Remoteness();
  VALUE value, GetValueOfPosition();

  MexFormat(position,mexString);

  if(gPredictions && (gMenuMode == Evaluated)) {
    value = GetValueOfPosition(position);
    (void) sprintf(prediction, "(%s %s %s in %d) %s",
		   playerName,
		   ((value == lose && usersTurn && gAgainstComputer) ||
		    (value == win && !usersTurn && gAgainstComputer)) ?
		   "will" : "should",
		   gValueString[(int)value],
		   Remoteness(position),
		   mexString);
  }
  else
    (void) sprintf(prediction,"");

  return(prediction);
}

/************************************************************************
**
** NAME:        Stopwatch
**
** DESCRIPTION: Set a stopwatch to record the time it took to evaluate.
**              With every call to Stopwatch, it records the time since
**              the last call to it.
**
**              I had trouble with time, because I hadn't 
**              #included any of the necessary files. My quote:
**
**               "This computer won't even give me the time of day!"
** 
** OUTPUTS:     (int) the number of seconds since the first call to it.
**
** CALLS:       time, difftime
**
************************************************************************/

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

/************************************************************************
**
** NAME:        ExitStageRight
**
** DESCRIPTION: Thank the user and exit(0). 
**
** CALLS:       exit()
**
************************************************************************/
 
ExitStageRight()
{
  printf("\nThanks for playing %s!\n",kGameName); /* quit */
  exit(0);
}

/************************************************************************
**
** NAME:        ExitStageRightErrorString
**
** DESCRIPTION: Print the error message and exit
** 
** INPUTS:      char errorMsg[] : The error message to print
**
************************************************************************/

ExitStageRightErrorString(errorMsg)
char errorMsg[];
{
  printf("\nError: %s\n",errorMsg);
  exit(0);
}

/************************************************************************
**
** NAME:        SafeMalloc
**
** DESCRIPTION: Call malloc() and barf if there's an error, otherwise
**              be invisible and return what malloc returns.
** 
** INPUTS:      int amount : The amount of memory to malloc.
**
** OUTPUTS:     GENERIC_PTR: A pointer to the memory returned by malloc.
**
** CALLS:       GENERIC_PTR malloc
**
************************************************************************/
 
GENERIC_PTR SafeMalloc(amount)
     int amount;
{
  GENERIC_PTR ptr;
	
  /* Mando's Fix is to put a ckalloc here */
  if((ptr = malloc(amount)) == NULL) {
    printf("Error: SafeMalloc could not allocate the requested %d bytes\n",amount);
    printf("       You've already asked for (and I gave you) %d bytes!\n",gBytesMalloced);
    ExitStageRight();
    return(ptr); /* never reached - added to make lint happy */
  }
  else {
    gBytesMalloced += amount;
    return(ptr);
  }
}

/************************************************************************
**
** NAME:        SafeFree
**
** DESCRIPTION: Call free(). This is set up to help debug the tcl memory
**              problems.
** 
** INPUTS:      GENERIC_PTR : A pointer to the memory to be freed
**
** CALLS:       (void) free
**
************************************************************************/
 
SafeFree(ptr)
     GENERIC_PTR ptr;
{
  free(ptr);
}

/************************************************************************
**
** NAME:        MemberofPositionList
**
** DESCRIPTION: Scan down the linked list of positions. If the current
**              position equals the input, return TRUE, otherwise FALSE.
** 
** INPUTS:      POSITIONLIST *ptr : The pointer to the head of the list.
**              POSITION position : The position to check.
**
** OUTPUTS:     BOOLEAN: TRUE iff the position is a member of the list
**
************************************************************************/

/*
BOOLEAN MemberofPositionList(ptr, thePosition)
POSITIONLIST *ptr;
POSITION thePosition;
{
        while (ptr != NULL) {
		if(thePosition == ptr->position)
			return(TRUE);
		ptr = ptr->next;
	}

	return(FALSE);
}
*/

/************************************************************************
**
** NAME:        StoreValueOfPosition
**
** DESCRIPTION: Store the value of the position into the Database, then
**              return that value.
** 
** INPUTS:      POSITION position : The position to return the value of.
**              VALUE    value    : The value to store
**
** OUTPUTS:     (VALUE) an enum which is oneof: (win,lose,tie,undecided)
**
** CALLS:       VALUE    *GetRawValueFromDatabase (POSITION)
**
************************************************************************/

VALUE StoreValueOfPosition(position, value)
     POSITION position;
     VALUE value;
{
  VALUE *GetRawValueFromDatabase(), *ptr, GetValueOfPosition(), oldValue;
		
  ptr = GetRawValueFromDatabase(position);

  if((oldValue = GetValueOfPosition(position)) != undecided)
    printf("Error: StoreValueOfPosition(%d,%s) found a slot already filled with %s.\n",position,gValueString[value],gValueString[oldValue]);

  /* put it in the right position, but we have to blank field and then
  ** add new value to right slot, keeping old slots */
  return((*ptr = ((*ptr & ~VALUE_MASK) | (value & VALUE_MASK))) & VALUE_MASK); 
}

/************************************************************************
**
** NAME:        GetValueOfPosition
**
** DESCRIPTION: Get the value of the position from the Database.
** 
** INPUTS:      POSITION position : The position to return the value of.
**
** OUTPUTS:     (VALUE) an enum which is oneof: (win,lose,tie,undecided)
**
** CALLS:       VALUE *GetRawValueFromDatabase (POSITION)
**
************************************************************************/

VALUE GetValueOfPosition(position)
     POSITION position;
{
  VALUE *GetRawValueFromDatabase(), *ptr;

  ptr = GetRawValueFromDatabase(position);

  return((VALUE)((int)*ptr & VALUE_MASK)); /* return pure value */
}

/************************************************************************
**
** NAME:        CreateMovelistNode
**
** DESCRIPTION: Create and return a MOVELIST node
** 
** INPUTS:      MOVE theMove         : The move for the 'move' field
**              MOVELIST theNextMove : The next move for the 'next' field
**
** OUTPUTS:     (MOVELIST *) the pointer to the new node (or list)
**
** CALLS:       GENERIC_PTR SafeMalloc(size_of)
**
************************************************************************/

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

/************************************************************************
**
** NAME:        CopyMovelist
**
** DESCRIPTION: Copy and return a MOVELIST list
** 
** INPUTS:      MOVELIST theMovelist : The head of the movelist
**
** OUTPUTS:     (MOVELIST *) the pointer to the newly created list
**
** CALLS:       MOVELIST *CreateMovelistNode(MOVE,MOVELIST *)
**
************************************************************************/

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

/************************************************************************
**
** NAME:        Remoteness
**
** DESCRIPTION: Return the remoteness of the position
** 
** INPUTS:      POSITION position : The position to query.
**
** OUTPUTS:     (REMOTENESS) The remoteness amount
**
** CALLS:       VALUE *GetRawValueFromDatabase (POSITION)
**
************************************************************************/

REMOTENESS Remoteness(position)
     POSITION position;
{
  VALUE *GetRawValueFromDatabase(), *ptr;

  ptr = GetRawValueFromDatabase(position);

  return((((int)*ptr & REMOTENESS_MASK) >> REMOTENESS_SHIFT));
}

/************************************************************************
**
** NAME:        SetRemoteness
**
** DESCRIPTION: Assign the remoteness value to the position's value
** 
** INPUTS:      POSITION position     : The position to set
**              REMOTENESS remoteness : The remoteness value to set
**
** CALLS:       VALUE *GetRawValueFromDatabase (POSITION)
**
************************************************************************/

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

/************************************************************************
**
** NAME:        Visited
**
** DESCRIPTION: Return TRUE iff the position has been visited.
** 
** INPUTS:      POSITION position : The position to query.
**
** OUTPUTS:     TRUE iff the position has been visited.
**
** CALLS:       VALUE *GetRawValueFromDatabase (POSITION)
**
************************************************************************/

BOOLEAN Visited(position)
     POSITION position;
{
  VALUE *GetRawValueFromDatabase(), *ptr;

  ptr = GetRawValueFromDatabase(position);

  return((((int)*ptr & VISITED_MASK) == VISITED_MASK)); /* Is bit set? */
}

/************************************************************************
**
** NAME:        MarkAsVisited
**
** DESCRIPTION: Mark the position as visited in the Database.
** 
** INPUTS:      POSITION position : The position to mark.
**
** CALLS:       VALUE *GetRawValueFromDatabase (POSITION)
**
************************************************************************/

MarkAsVisited (position)
     POSITION position;
{
  VALUE *GetRawValueFromDatabase(), *ptr;

  ptr = GetRawValueFromDatabase(position);

  *ptr = (VALUE)((int)*ptr | VISITED_MASK);       /* Turn bit on */
}

/************************************************************************
**
** NAME:        UnMarkAsVisited
**
** DESCRIPTION: Mark the position as not visited in the Database.
** 
** INPUTS:      POSITION position : The position to mark.
**
** CALLS:       VALUE *GetRawValueFromDatabase (POSITION)
**
************************************************************************/

UnMarkAsVisited (position)
     POSITION position;
{
  VALUE *GetRawValueFromDatabase(), *ptr;

  ptr = GetRawValueFromDatabase(position);

  *ptr = (VALUE)((int)*ptr & ~VISITED_MASK);      /* Turn bit off */
}

/************************************************************************
**
** NAME:        BadElse
**
** DESCRIPTION: Print an error when an invalid else condition is reached.
** 
** INPUTS:      STRING function : The name of the offending function.
**
************************************************************************/

BadElse(function)
     STRING function;
{
  printf("Error: %s() just reached an else clause it shouldn't have!\n\n",function);
}

/************************************************************************
**
** NAME:        HitAnyKeyToContinue
**
** DESCRIPTION: Wait until a key is hit, then returns. The first kludge
**              is to account for the problem I have with getchars. The
**              first time this is called, it only reads one getchar. 
**              Subsequent times it reads two - one for the LAST return
**              and one for THIS return.
** 
************************************************************************/

HitAnyKeyToContinue()
{
  static BOOLEAN first = TRUE;

  printf("\n\t----- Hit <return> to continue -----");
  first ? (first = FALSE) : getchar(); /* to make lint happy */
  while(getchar() != '\n');
}

/************************************************************************
**
** NAME:        HandleDefaultTextInput
**
** DESCRIPTION: Dispatch on the command the user types.
** 
** INPUTS:      POSITION *thePosition : The position the user is at. 
**              MOVE *theMove         : The move to fill with user's move. 
**              STRING playerName     : The name of the player whose turn it is
**  
** OUTPUTS:     USERINPUT : Oneof( Undo, Abort, Continue )
**
************************************************************************/

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
    case 'r': case 'R':
      PrintPosition(thePosition, playerName);
      break;
    case 's': case 'S':
      PrintValueEquivalentMoves(thePosition);
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

/************************************************************************
**
** NAME:        GetMyChar
**
** DESCRIPTION: Return the typed character
**
** RETURNS:     The first character of the string typed
**
************************************************************************/

char GetMyChar()
{
  char inString[MAXINPUTLENGTH], ans;
  scanf("%s",inString);
  ans = inString[0];
  return(ans);
}

/************************************************************************
**
** NAME:        GetMyString
**
** DESCRIPTION: Read in <size> characters and put it into <name>. name
**              MUST have enough space for size-1 characters and a \0,
**              as this is not tested. Carraige Returns are not read,
**              as they terminate a string when reading. Example:
**
**              #define SIZE 9   ...8 characters & '\0' 
**              main() {
**                 char name[SIZE];
**                 GetMyString(name,SIZE,TRUE,TRUE); <- eat 1st char, put CR back
**              }
** 
** INPUTS:      char *name           : The string to fill
**              int   size           : The number of total chars to read
**              BOOLEAN eatFirstChar : A boolean to eat the first char
**              BOOLEAN putCarraigeReturnBack : A boolean to put a CR back
** 
************************************************************************/

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


