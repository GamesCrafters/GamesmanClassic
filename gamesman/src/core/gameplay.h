
#ifndef GMCORE_GAMEPLAY_H
#define GMCORE_GAMEPLAY_H

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
void		PrintVisualValueHistory		(POSITION position);
void            PrintMoveHistory                (POSITION position);
UNDO*		Stalemate			(UNDO*, POSITION stalematePos, BOOLEAN* abort);

/* Data output */

void		PrintMoves			(MOVELIST* ptr, REMOTENESSLIST* remotePtr);
void		PrintValueMoves			(POSITION pos);
BOOLEAN		PrintPossibleMoves		(POSITION pos);

REMOTENESS      FindDelta                       (REMOTENESS remote, REMOTENESSLIST* remoteptr, VALUE val);

/* Brains of the gameplay */

STRING		GetPrediction			(POSITION pos, STRING playerName, BOOLEAN usersTurn);
MOVE		GetComputersMove		(POSITION pos);
VALUE_MOVES*	GetValueMoves			(POSITION pos);

/* Player structure */
typedef enum player_enum {Human, Computer} PTYPE;

typedef struct Player {
    int turn;
    char* name;
    PTYPE type;
    USERINPUT (*GetMove)(POSITION,MOVE*,STRING);
} *PLAYER;

/* Player Constructors */
PLAYER          NewHumanPlayer                  (STRING name,int turn);
PLAYER          NewComputerPlayer               (STRING name,int turn);
USERINPUT       ComputerMove                    (POSITION position,MOVE* move, STRING name);

void            PlayGame                        (PLAYER playerOne,PLAYER playerTwo);

#endif /* GMCORE_GAMEPLAY_H */
