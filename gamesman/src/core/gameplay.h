
#ifndef GM_GAMEPLAY_H
#define GM_GAMEPLAY_H

/* General gameplay entry points */

void		PlayAgainstHuman		();
void		PlayAgainstComputer		();

/* Undo handling */

void		ResetUndoList			(UNDO* list);
UNDO*		HandleUndoRequest		(POSITION* pos, UNDO* list, BOOLEAN* error);
UNDO*		UpdateUndo			(POSITION pos, UNDO* undo, BOOLEAN* abort);
UNDO*		InitializeUndo			();

/* Output functions (mostly explanations) */

void		PrintHumanValueExplanation	();
void		PrintComputerValueExplanation	();
UNDO*		Stalemate			(UNDO*, POSITION stalematePos, BOOLEAN* abort);

/* Data output */

void		PrintMoves			(MOVELIST* ptr, REMOTENESSLIST* remotePtr);
void		PrintValueMoves			(POSITION pos);
BOOLEAN		PrintPossibleMoves		(POSITION pos);

/* Brains of the gameplay */

STRING		GetPrediction			(POSITION pos, STRING playerName, BOOLEAN usersTurn);
MOVE		GetComputersMove		(POSITION pos);
VALUE_MOVES*	GetValueMoves			(POSITION pos);


#endif /* GM_GAMEPLAY_H */
