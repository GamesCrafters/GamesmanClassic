#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <zlib.h>

#include "gamesman.h"
#include "loopygasolver.h"



/* analysis and documentation bookkeeping */
// may make this into a struct later


//extern STRING  kGUIAuthorName;    // graphics writers


/* gameplay-related internal function prototypes */
void     Initialize();
void     SetSolver();
void     Menus();
void     MenusBeforeEvaluation();
void     MenusEvaluated();
void     ParseMenuChoice(char c);
BOOLEAN  ParseConstantMenuChoice(char c);
void     ParseBeforeEvaluationMenuChoice(char c);
void     ParseEvaluatedMenuChoice(char c);
void     HelpMenus();
void     ParseHelpMenuChoice(char c);
void     BadMenuChoice();
void     DebugModule();
void     PlayAgainstHuman();
void     PlayAgainstComputer();
int      randSafe();
BOOLEAN  ValidMove(POSITION thePosition, MOVE theMove);
BOOLEAN  PrintPossibleMoves(POSITION thePosition);
int      Stopwatch();
POSITION GetNextPosition();
BOOLEAN  CorruptedValuesP();
void     AnalysisMenu();
void     PrintRawGameValues(BOOLEAN toFile);
void     PrintBadPositions(char c, int maxPositions, POSITIONLIST* badWinPositions, POSITIONLIST* badTiePositions, POSITIONLIST* badLosePositions);
void     PrintGameValueSummary();
void     PrintValuePositions(char c, int maxPositions);
void     PrintComputerValueExplanation();
void     PrintHumanValueExplanation();


/* database function prototypes */
int  writeDatabase();
int  loadDatabase();
VALUE   *GetRawValueFromDatabase(POSITION position);

/* smarter computer function prototypes */
void         PrintMoves(MOVELIST* ptr, REMOTENESSLIST* remoteptr);
void         PrintValueMoves(POSITION thePosition);
MOVE         RandomLargestRemotenessMove(MOVELIST *moveList, REMOTENESSLIST *remotenessList);
MOVE         RandomSmallestRemotenessMove (MOVELIST *moveList, REMOTENESSLIST *remotenessList);
VALUE_MOVES* SortMoves (POSITION thePosition, MOVE move, VALUE_MOVES *valueMoves);
VALUE_MOVES* GetValueMoves(POSITION thePosition);
void         SmarterComputerMenu();
VALUE_MOVES* StoreMoveInList(MOVE theMove, REMOTENESS remoteness, VALUE_MOVES* valueMoves, int typeofMove);

/* undo internal function prototypes */
void    ResetUndoList(UNDO* undo);
UNDO*   HandleUndoRequest(POSITION* thePosition, UNDO* undo, BOOLEAN* error);
UNDO*   UpdateUndo(POSITION thePosition, UNDO* undo, BOOLEAN* abort);
UNDO*   InitializeUndo();
UNDO*   Stalemate(UNDO* undo,POSITION stalematePosition, BOOLEAN* abort);

/* game debugging function */
void	FoundBadPosition(POSITION, POSITION, MOVE);

/* low memory solver internal fucntion prototypes */
VALUE DetermineZeroValue(POSITION position);

/* non-loopy solver internal function prototypes */
VALUE   DetermineValue1(POSITION position);

/* loopy solver internal function prototypes */
void     MyPrintParents();
VALUE    DetermineLoopyValue1(POSITION position);
VALUE    DetermineLoopyValue(POSITION position);
void     SetParents (POSITION parent, POSITION root);
void	 InitializeVisitedArray();
void	 FreeVisitedArray();
void     ParentInitialize();
void     ParentFree();
void     NumberChildrenInitialize();
void     NumberChildrenFree();
void     InitializeFR();
POSITION DeQueueWinFR();
POSITION DeQueueLoseFR();
POSITION DeQueueTieFR();
POSITION DeQueueFR(FRnode **gHeadFR, FRnode **gTailFR);
void     InsertWinFR(POSITION position);
void     InsertLoseFR(POSITION position);
void     InsertTieFR(POSITION position);
void     InsertFR(POSITION position, FRnode **firstnode, FRnode **lastnode);

/* mex values internal function prototypes */
MEXCALC MexAdd(MEXCALC theMexCalc,MEX theMex);
MEX     MexCompute(MEXCALC theMexCalc);
MEXCALC MexCalcInit();
void    MexStore(POSITION position, MEX theMex);
MEX     MexLoad(POSITION position);
void    MexFormat(POSITION position, STRING string);
MEX     MexPrimitive(VALUE value);
void    PrintMexValues(MEX mexValue, int maxPositions);

/* Status Meter */
void showStatus(int done);




