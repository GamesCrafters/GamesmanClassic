/************************************************************************
**
** NAME:         gfuzzy.c
**
** DESCRIPTION:  This file contains the fuzzy code used for the static
**               evaluators
**
** AUTHOR:       Dan Garcia  -  University of California at Berkeley
**               Copyright (C) Dan Garcia, 1995. All rights reserved.
**
** DATE:         05/12/92
**
** UPDATE HIST:
**
** 05-15-1995   1.0   : Final release code for M.S.
** 1999-03-09   1.2   : Added Mex printing in the AnalysisMenu
**
**************************************************************************/

#include <stdio.h>
#include <math.h>
#include "gsolve.h"

#define mMax(A,B) ((A) > (B) ? (A) : (B))
#define mMin(A,B) ((A) > (B) ? (B) : (A))

extern STRING   gValueString[];        /* The Value strings */
extern STRING   kGameName;             /* The Name of the Game */
extern int      gLookAheadLevel;       /* The depth of the lookahead */
extern BOOLEAN  gPredictions;          /* TRUE <==> the module shows predictions */
extern BOOLEAN  gExhaustiveSearch;     /* TRUE <==> it was exhaustively searched */
extern BOOLEAN  kSupportsHeuristic;    /* TRUE <==> module supports a heuristic */
extern BOOLEAN  kPartizan;             /* TRUE <==> module is a Partizan game */
extern POSITION kBadPosition;          /* A POSITION that will never be used */
extern POSITION gInitialPosition;      /* The initial position of the game */
extern int      gNumberOfPositions;    /* The number of positions in the game */

/************************************************************************
**
** NAME:        GetHeuristicComputersMove
**
** DESCRIPTION: Get the next move for the computer by using the static
**              evaluator
** 
** INPUTS:      POSITION thePosition : The position in question.
**              MINIMAX  theMinOrMax : minimizing or maximizing
**              int      searchDepth : The Look-Ahead level
**              MOVE     theMove     : The Move pointer to SET!
**
** OUTPUTS:     (FUZZY) : the fuzzy value of the search
**
** CALLS:       FUZZY     GetHeuristicComputersMove(POSITION,MINIMAX,int,*MOVE)
**              FUZZY     FuzzifyValue             (VALUE,MINIMAX)
**              FUZZY     StaticEvaluator          (POSITION)
**              FUZZY     FuzzyComplement          (FUZZY)
**              MOVELIST *GenerateMoves            (POSITION)
**                        FreeMoveList             (*MOVELIST)
**              VALUE     Primitive                (POSITION)
**
************************************************************************/

FUZZY GetHeuristicComputersMove(thePosition, theMinOrMax, searchDepth, theMove)
     POSITION thePosition;
     MINIMAX theMinOrMax;
     int searchDepth;
     MOVE *theMove;
{
  VALUE Primitive(), tempValue;
  FUZZY FuzzifyValue(), StaticEvaluator(), FuzzyComplement(), bestSoFar, tempFuzzy;
  MOVELIST *ptr, *head, *GenerateMoves();
  MOVE tempMove;

  if(!kPartizan) /* If we're in an impartial game, always minimize! */
    theMinOrMax = minimizing; /* because 0 = LOSE, 1 = WIN */

  if((tempValue = Primitive(thePosition)) != undecided)
    return(FuzzifyValue(tempValue,theMinOrMax));
  else if (searchDepth == 0)
    return(StaticEvaluator(thePosition));
  else {
    ptr = head = GenerateMoves(thePosition);

    bestSoFar = theMinOrMax == minimizing ? MAX_FUZZY_VALUE : MIN_FUZZY_VALUE;
    
    while(ptr != NULL) {
      tempFuzzy = GetHeuristicComputersMove(DoMove(thePosition,ptr->move),
					    theMinOrMax == minimizing ? maximizing : minimizing,
					    searchDepth - 1,
					    &tempMove);
      if((theMinOrMax == minimizing && tempFuzzy <= bestSoFar) ||
	 (theMinOrMax == maximizing && tempFuzzy >= bestSoFar)) {
	bestSoFar = tempFuzzy;
	*theMove = ptr->move;
      }
      ptr = ptr->next;
    } 
    FreeMoveList(head);
    return(kPartizan ? bestSoFar : FuzzyComplement(bestSoFar));
  }
}

/************************************************************************
**
** NAME:        FuzzifyValue
**
** DESCRIPTION: Takes win,lose,tie and maps them to the fuzzy values
** 
** INPUTS:      VALUE theValue : The input win,lose,tie (NOT undecided!)
**              MINIMAX  theMinOrMax : minimizing or maximizing
**
** OUTPUTS:     (FUZZY) : win==MAX_FUZZY_VALUE,tie=TIE_FUZZY_VALUE,
**                        lose=MIN_FUZZY_VALUE depending on theMinOrMax
**
** CALLS:       BadElse(STRING)
**
************************************************************************/

FUZZY FuzzifyValue(theValue,theMinOrMax)
     VALUE theValue;
     MINIMAX theMinOrMax;
{
  /* REQUIRES: theValue be oneof (win,lose,tie) */

  if(theValue == lose)
    return(theMinOrMax == minimizing && kPartizan ? MAX_FUZZY_VALUE : MIN_FUZZY_VALUE);
  else if(theValue == win)
    return(theMinOrMax == minimizing && kPartizan ? MIN_FUZZY_VALUE : MAX_FUZZY_VALUE);
  else if(theValue == tie)
    return(TIE_FUZZY_VALUE);
  else
    BadElse("FuzzifyValue");
}

/************************************************************************
**
** NAME:        CorruptedValuesP
**
** DESCRIPTION: Go through the database and search for inconsistent
**              states, for example:
**              1) Lose or Tie position with lose child (should be lose)
**              2) Win position without lose child (should be tie/lose)
**              3) Tie position without tie child (should be lose)
** 
** OUTPUTS:     (BOOLEAN) : TRUE iff a corruption was found
**
************************************************************************/

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

/************************************************************************
**
** NAME:        AnalysisMenu
**
** DESCRIPTION: This gives the user an analysis menu with which to
**              examine the Position/Value tables.
** 
** CALLS:       ExitStageRight()
**              HelpMenus()
**              TestStaticEvaluator()
**              BadMenuChoice()
**              HitAnyKeyToContinue()
**              PrintPosition(POSITION)
**              PrintGameValueSummary()
**              PrintValuePositions(char,int)
**
************************************************************************/

AnalysisMenu()
{
  POSITIONLIST *badWinPositions = NULL, *badTiePositions = NULL, *badLosePositions = NULL;
  BOOLEAN tempPredictions = gPredictions, CorruptedValuesP();
  MEX mexValue = 0;
  int mexInt, maxPositions = 10;
  char GetMyChar(), c;

  gPredictions = FALSE;
  
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

    if(kSupportsHeuristic)
      printf("\n\ts)\tTest the (S)tatic Evaluator using Exhaustive Search Table\n");
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
    case 'S': case 's':
      if(kSupportsHeuristic)
	TestStaticEvaluator(&badWinPositions, &badTiePositions, &badLosePositions);
      else
	BadMenuChoice();
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
** NAME:        PrintRawGameValues
**
** DESCRIPTION: Loop through game values and print them out, either to
**              the screen or to a file.
** 
** INPUTS:      BOOLEAN toFile : TRUE iff should write to file
**
************************************************************************/

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


/************************************************************************
**
** NAME:        PrintBadPositions
**
** DESCRIPTION: This prints up to maxPositions positions of the type
**              requested by c, the input. 
** 
** CALLS:       POSITION GetNextPosition()
**              VALUE GetValueOfPosition(POSITION)
**              PrintPosition(POSITION)
**
************************************************************************/

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

/************************************************************************
**
** NAME:        PrintGameValueSummary
**
** DESCRIPTION: This cycles through every position stored in the
**              system's table and compiles a summary of their values
**              and prints it out in a nice format.
** 
** CALLS:       POSITION GetNextPosition()
**              VALUE GetValueOfPosition(POSITION)
**
************************************************************************/

PrintGameValueSummary()
{
  POSITION thePosition, GetNextPosition();
  VALUE GetValueOfPosition(), theValue;
  int winCount, loseCount, tieCount, undecidedCount, unknownCount;
  int totalPositions;

  totalPositions = winCount = loseCount = tieCount = undecidedCount = unknownCount = 0;
  while((thePosition = GetNextPosition()) != kBadPosition) {
    theValue = GetValueOfPosition(thePosition);
    totalPositions++;
    if(theValue == win)            winCount++;
    else if(theValue == lose)      loseCount++;
    else if(theValue == tie)       tieCount++;
    else if(theValue == undecided) undecidedCount++;
    else                           unknownCount++;
  }
  printf("\n\n\t----- Summmary of Game values -----\n\n");
  
  printf("\tValue       Number       Total\n");
  printf("\t------------------------------\n");
  printf("\tUndecided = %5d out of %d\n",undecidedCount,totalPositions);	
  printf("\tLose      = %5d out of %d\n",loseCount,totalPositions);	
  printf("\tWin       = %5d out of %d\n",winCount,totalPositions);	
  printf("\tTie       = %5d out of %d\n",tieCount,totalPositions);	
  printf("\tUnknown   = %5d out of %d\n",unknownCount,totalPositions);	
  printf("\tTOTAL     = %5d out of %d\n",
	 undecidedCount+loseCount+winCount+tieCount+unknownCount,
	 totalPositions);	
}

/************************************************************************
**
** NAME:        PrintMexValues
**
** DESCRIPTION: This prints positions with the requested mex values
** 
** INPUTS:      MEX mexValue : The requested mex value
**              int maxPositions : The number of positions to print at a time
**
** CALLS:       POSITION GetNextPosition()
**              VALUE GetValueOfPosition(POSITION)
**              PrintPosition(POSITION)
**
************************************************************************/

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

/************************************************************************
**
** NAME:        PrintValuePositions
**
** DESCRIPTION: This prints up to maxPositions positions of the type
**              requested by c, the input. 
** 
** CALLS:       POSITION GetNextPosition()
**              VALUE GetValueOfPosition(POSITION)
**              PrintPosition(POSITION)
**
************************************************************************/

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

/************************************************************************
**
** NAME:        StorePositionInList
**
** INPUTS:      POSITION      thePosition     : The position to store
**              POSITIONLIST *thePositionList : The list to store it in
**
** OUTPUTS:     (*POSITIONLIST) : The appended list. 
**
** DESCRIPTION: This adds a position to the front of the positionlist 
**              and returns the new positionlist.
**
** CALLS:       GENERIC_PTR SafeMalloc(int)
**
************************************************************************/

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

/************************************************************************
**
** NAME:        StoreMoveInList
**
** INPUTS:      MOVE      theMove     : The move to store
**              MOVELIST *theMoveList : The list to store it in
**
** OUTPUTS:     (*MOVELIST) : The appended list. 
**
** DESCRIPTION: This adds a move to the movelist and returns
**              the new movelist.
**
** CALLS:       GENERIC_PTR SafeMalloc(int)
**
************************************************************************/

MOVELIST *StoreMoveInList(theMove,theMoveList)
     MOVE     theMove;
     MOVELIST *theMoveList;
{
  MOVELIST *next, *tmp;
  GENERIC_PTR SafeMalloc();
   
  next = theMoveList;
  tmp = (MOVELIST *) SafeMalloc (sizeof(MOVELIST));
  tmp->move = theMove;
  tmp->next     = next;
  return(tmp);
}

/************************************************************************
**
** NAME:        TestStaticEvaluator
**
** DESCRIPTION: This cycles through every position stored in the
**              system's table and tests the move the StaticEvaluator
**              chooses against the stored valid moves.
** 
** CALLS:       POSITION GetNextPosition()
**              FreeMoveList(*MOVELIST)
**              BadElse(STRING)
**              POSITIONLIST *StorePositionInList(POSITION,*POSITIONLIST)
**
************************************************************************/

TestStaticEvaluator(badWinPositions, badTiePositions, badLosePositions)
     POSITIONLIST **badWinPositions, **badTiePositions, **badLosePositions;
{
  POSITION thePosition, GetNextPosition();
  POSITIONLIST *tmp, *StorePositionInList();
  FUZZY GetHeuristicComputersMove();
  MOVE theMove;
  MOVELIST *ptr, *head, *GenerateMoves(), *GetValueEquivalentMoves();
  VALUE GetValueOfPosition(), theValue, Primitive();
  BOOLEAN found;
  MINIMAX PositionToMinOrMax();
  int winCount, loseCount, tieCount, badWinCount, badLoseCount, badTieCount;
  
  FreePositionList(*badWinPositions);
  FreePositionList(*badTiePositions);
  FreePositionList(*badLosePositions);
  *badWinPositions = *badTiePositions = *badLosePositions = NULL;
  winCount = loseCount = tieCount = badWinCount = badLoseCount = badTieCount = 0;

  printf("\n\t----- Static Evaluator Test Function for %s -----\n\n", kGameName);

  while((thePosition = GetNextPosition()) != kBadPosition) {
    (void) GetHeuristicComputersMove(thePosition,
				     PositionToMinOrMax(thePosition),
				     gLookAheadLevel,
				     &theMove);
    theValue = GetValueOfPosition(thePosition);

    if (Primitive(thePosition) == undecided) {  /* primitives have no next move */
      if     (theValue == win)  winCount++;
      else if(theValue == lose) loseCount++;
      else if(theValue == tie)  tieCount++;
      else    BadElse("TestStaticEvaluator - counting");

      ptr = head = GetValueEquivalentMoves(thePosition);
      found = FALSE;
      while(ptr != NULL && !found) {
	if(ptr->move == theMove) {
	  found = TRUE;
	}
	ptr = ptr->next;
      }
      FreeMoveList(head);
      
      if(!found) {
	if(theValue == win) {
	  *badWinPositions = StorePositionInList(thePosition,*badWinPositions);
	  badWinCount++;
	}
	else if (theValue == tie) {
	  *badTiePositions = StorePositionInList(thePosition,*badTiePositions);
	  badTieCount++;
	}
	else if (theValue == lose) {
	  *badLosePositions = StorePositionInList(thePosition,*badLosePositions);
	  badLoseCount++;
	}
	else
	  BadElse("TestStaticEvaluator");
      }
    }
  }
/*
  if(*badWinPositions != NULL) {
    printf("\nThe following WINNING positions had bad moves. These were:\n\n");
    tmp = *badWinPositions;
    while(tmp!=NULL) {
      PrintPosition(tmp->position,"computer");
v      tmp = tmp->next;
    }
  }
  if(*badTiePositions != NULL) {
    printf("\nThe following TIEING positions had bad moves. These were:\n\n");
    tmp = *badTiePositions;
    while(tmp!=NULL) {
      PrintPosition(tmp->position,"computer");
      tmp = tmp->next;
    }
  }
  if(*badLosePositions != NULL) {
    printf("\nThe following LOSING positions had bad moves. These were:\n\n");
    tmp = *badLosePositions;
    while(tmp!=NULL) {
      PrintPosition(tmp->position,"computer");
      tmp = tmp->next;
    }
  }
*/
  printf("\n\t----- Summary of Heuristic -----\n\n");
  printf("\t%5d of %5d WINNING positions were correct --> %3.0f %%\n",
	 winCount-badWinCount,winCount,
	 winCount == 0.0 ? 100.0 :
	 (float)(winCount-badWinCount)*100.0/(float)winCount);
  printf("\t%5d of %5d TIEING  positions were correct --> %3.0f %%\n",
	 tieCount-badTieCount,tieCount,
	 tieCount == 0.0 ? 100.0 :
	 (float)(tieCount-badTieCount)*100.0/(float)tieCount);
  printf("\t%5d of %5d LOSING  positions were correct --> %3.0f %%\n",
	 loseCount-badLoseCount,loseCount,
	 loseCount == 0.0 ? 100.0 :
	 (float)(loseCount-badLoseCount)*100.0/(float)loseCount);

  if(*badWinPositions == NULL && *badTiePositions == NULL && *badLosePositions == NULL)
    printf("\nA PERFECT HEURISTIC!!!\n");

}

/************************************************************************
**
** NAME:        GetValueEquivalentMoves
**
** INPUTS:      POSITION thePosition : The position to search from 
**
** OUTPUTS:     (*MOVELIST) : The list of value-equivalent moves
**
** DESCRIPTION: This returns a list of value-equivalent moves. A value-
**              equivalent move is one which leads to other positions
**              of appropriate values. For example, if this procedure
**              were called with a win position, it would return all
**              moves that led to a losing position. If called with
**              a losing position, it would return ALL possible moves,
**              since all moves from a losing position lead to wins.
**              If called with a tie position, it returns all tie moves.
**
** CALLS:       FreeMoveList(*MOVELIST)
**              VALUE GetValueOfPosition(POSITION)
**              MOVELIST *GenerateMoves(POSITION)
**              MOVELIST *StoreMoveInList(MOVE, *MOVELIST)
**              BadElse(STRING)
**
************************************************************************/

MOVELIST *GetValueEquivalentMoves(thePosition)
     POSITION thePosition;
{
  MOVELIST *ptr, *head, *valueEquivalentList = NULL, *GenerateMoves();
  MOVELIST *StoreMoveInList();
  VALUE GetValueOfPosition(), theValue, Primitive();
  REMOTENESS parentRemoteness, Remoteness();
  POSITION child;

  if(Primitive(thePosition) != undecided)   /* Primitive positions have no moves */
    return(NULL);

  else if((theValue = GetValueOfPosition(thePosition)) == undecided)
    return(NULL);                           /* undecided positions are invalid */

  else if(theValue == lose)
    return(GenerateMoves(thePosition));     /* Losing positions have EVERY move equivalent */

  else if(theValue != win && theValue != tie)
    BadElse("GetValueEquivalentMoves");     /* This makes sure the value is win | tie */

  else {                                    /* we are guaranteed it's win | tie now */
    parentRemoteness = Remoteness(thePosition);
    ptr = head = GenerateMoves(thePosition);
    while(ptr != NULL) {                    /* otherwise  (theValue = (win|tie) */
      if((theValue == win && 
           (GetValueOfPosition(child = DoMove(thePosition,ptr->move)) == lose) &&
           (parentRemoteness > Remoteness(child))) ||
	 (theValue == tie && (GetValueOfPosition(DoMove(thePosition,ptr->move)) == tie)))
	valueEquivalentList = StoreMoveInList(ptr->move,valueEquivalentList);
      ptr = ptr->next;
    }
    FreeMoveList(head);
  }
  return(valueEquivalentList);
}

/************************************************************************
**
** NAME:        FuzzyComplement
**
** DESCRIPTION: Computes the Fuzzy Complement
** 
** INPUTS:      FUZZY a : The fuzzy [0,1] input
**
** OUTPUTS:     (FUZZY) : returns(1.0 - a)
**
************************************************************************/

FUZZY FuzzyComplement(a)
     FUZZY a;
{
  return(1.0 - a);
}

/************************************************************************
**
** NAME:        FuzzyVery
**
** DESCRIPTION: Computes the Fuzzy Hedge "Very"
** 
** INPUTS:      FUZZY a : The fuzzy [0,1] input
**
** OUTPUTS:     (FUZZY) : returns(a*a)
**
************************************************************************/

FUZZY FuzzyVery(a)
     FUZZY a;
{
  return(a * a);
}

/************************************************************************
**
** NAME:        FuzzyMoreOrLess
**
** DESCRIPTION: Computes the Fuzzy Hedge "More Or Less"
** 
** INPUTS:      FUZZY a : The fuzzy [0,1] input
**
** OUTPUTS:     (FUZZY) : returns(sqrt(a))
**
************************************************************************/

FUZZY FuzzyMoreOrLess(a)
     FUZZY a;
{
  return(sqrt(a));
}

/************************************************************************
**
** NAME:        FuzzyAnd
**
** DESCRIPTION: Computes the Fuzzy Intersection or Conjunction
** 
** INPUTS:      FUZZY a : The fuzzy [0,1] input
**              FUZZY b : The fuzzy [0,1] input
**
** OUTPUTS:     (FUZZY) : returns(min(a,b))
**
************************************************************************/

FUZZY FuzzyAnd(a,b)
     FUZZY a,b;
{
  return(mMin(a,b));
}

/************************************************************************
**
** NAME:        FuzzyIntersection
**
** DESCRIPTION: Computes the Fuzzy Intersection or Conjunction
** 
** INPUTS:      FUZZY a : The fuzzy [0,1] input
**              FUZZY b : The fuzzy [0,1] input
**
** OUTPUTS:     (FUZZY) : returns(min(a,b))
**
************************************************************************/

FUZZY FuzzyIntersection(a,b)
     FUZZY a,b;
{
  return(mMin(a,b));
}

/************************************************************************
**
** NAME:        FuzzyOr
**
** DESCRIPTION: Computes the Fuzzy Union or Disjunction
** 
** INPUTS:      FUZZY a : The fuzzy [0,1] input
**              FUZZY b : The fuzzy [0,1] input
**
** OUTPUTS:     (FUZZY) : returns(max(a,b))
**
************************************************************************/

FUZZY FuzzyOr(a,b)
     FUZZY a,b;
{
  return(mMax(a,b));
}

/************************************************************************
**
** NAME:        FuzzyUnion
**
** DESCRIPTION: Computes the Fuzzy Union or Disjunction
** 
** INPUTS:      FUZZY a : The fuzzy [0,1] input
**              FUZZY b : The fuzzy [0,1] input
**
** OUTPUTS:     (FUZZY) : returns(max(a,b))
**
************************************************************************/

FUZZY FuzzyUnion(a,b)
     FUZZY a,b;
{
  return(mMin(a,b));
}

/************************************************************************
**
** NAME:        FuzzyTnorm
**
** DESCRIPTION: Computes the Fuzzy T-norm
** 
** INPUTS:      FUZZY a : The fuzzy [0,1] input
**              FUZZY b : The fuzzy [0,1] input
**
** OUTPUTS:     (FUZZY) : returns(ab)
**
************************************************************************/

FUZZY FuzzyTnorm(a,b)
     FUZZY a,b;
{
  return(a * b);
}

/************************************************************************
**
** NAME:        FuzzyTnormYager
**
** DESCRIPTION: Computes the Fuzzy T-norm using Yager's operator
** 
** INPUTS:      FUZZY a : The fuzzy [0,1] input
**              FUZZY b : The fuzzy [0,1] input
**              float p : The parameter p (must be >= 1  !!)
**
** OUTPUTS:     (FUZZY) : Yager's operator
**
************************************************************************/

FUZZY FuzzyTnormYager(a,b,p)
     FUZZY a,b;
     float p;
{
  return(1.0 - mMin(1.0,pow(pow((1.0-a),p) + pow((1.0-b),p),1.0/p)));
}

/************************************************************************
**
** NAME:        FuzzyTnormHamacher
**
** DESCRIPTION: Computes the Fuzzy T-norm using Hamacher's operator
** 
** INPUTS:      FUZZY a : The fuzzy [0,1] input
**              FUZZY b : The fuzzy [0,1] input
**              float g : The parameter to the operator, gamma >= 0
**
** OUTPUTS:     (FUZZY) : The output of the T-norm using Hamacher's op.
**
************************************************************************/

FUZZY FuzzyTnormHamacher(a,b,g)
     FUZZY a,b;
     float g;
{
  return((a*b)/(g+(1-g)*(a+b+a*b)));
}

/************************************************************************
**
** NAME:        FuzzySnorm
**
** DESCRIPTION: Computes the Fuzzy S-norm
** 
** INPUTS:      FUZZY a : The fuzzy [0,1] input
**              FUZZY b : The fuzzy [0,1] input
**
** OUTPUTS:     (FUZZY) : returns(a+b-ab)
**
************************************************************************/

FUZZY FuzzySnorm(a,b)
     FUZZY a,b;
{
  return(a + b - (a * b));
}

/************************************************************************
**
** NAME:        FuzzySnormYager
**
** DESCRIPTION: Computes the Fuzzy S-norm using Yager's operator
** 
** INPUTS:      FUZZY a : The fuzzy [0,1] input
**              FUZZY b : The fuzzy [0,1] input
**              float p : The parameter to yager's op. must be >= 1!
**
** OUTPUTS:     (FUZZY) : returns the s-norm value using Yager's op.
**
************************************************************************/

FUZZY FuzzySnormYager(a,b,p)
     FUZZY a,b;
     float p;
{
  return(mMin(1.0,pow(pow(a,p)+pow(b,p),(1.0/p))));
}

/************************************************************************
**
** NAME:        FuzzySnormHamacher
**
** DESCRIPTION: Computes the Fuzzy S-norm using Hamacher's operator
** 
** INPUTS:      FUZZY a : The fuzzy [0,1] input
**              FUZZY b : The fuzzy [0,1] input
**              float g : The parameter to hamacher's op. must be >= 0
**
** OUTPUTS:     (FUZZY) : returns the s-norm value using Hamacher's op.
**
************************************************************************/

FUZZY FuzzySnormHamacher(a,b,g)
     FUZZY a,b;
     float g;
{
  return((a*b*(g-2.0) + a + b) / (a * b * (g - 1.0) + 1.0));
}

/************************************************************************
**
** NAME:        BoundedSum
**
** DESCRIPTION: Computes the Fuzzy Bounded Sum
** 
** INPUTS:      FUZZY a : The fuzzy [0,1] input
**              FUZZY b : The fuzzy [0,1] input
**
** OUTPUTS:     (FUZZY) : returns(min(1.0,(a+b)))
**
************************************************************************/

FUZZY BoundedSum(a,b)
     FUZZY a,b;
{
  return(mMin(1.0,(a + b)));
}

/************************************************************************
**
** NAME:        BoundedDifference
**
** DESCRIPTION: Computes the Fuzzy Bounded Difference
** 
** INPUTS:      FUZZY a : The fuzzy [0,1] input
**              FUZZY b : The fuzzy [0,1] input
**
** OUTPUTS:     (FUZZY) : returns(max(0.0,(a+b-1.0)))
**
************************************************************************/

FUZZY BoundedDifference(a,b)
     FUZZY a,b;
{
  return(mMax(0.0,(a + b - 1.0)));
}

