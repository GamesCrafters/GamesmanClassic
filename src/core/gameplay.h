
#ifndef GMCORE_GAMEPLAY_H
#define GMCORE_GAMEPLAY_H

/* General gameplay entry points */

void            PlayAgainstHuman                ();
void            PlayAgainstComputer             ();

/* Undo handling */

void            ResetUndoList                   (UNDO* list);
UNDO*           HandleUndoRequest               (POSITION* pos, UNDO* list, BOOLEAN* error);
UNDO*           UpdateUndo                      (POSITION pos, UNDO* undo, BOOLEAN* abort);
UNDO*           InitializeUndo                  ();

/* Output functions (mostly explanations) */

void            PrintHumanValueExplanation      ();
void            PrintComputerValueExplanation   ();
void            PrintVisualValueHistory         (POSITION position, int showAllMoves);
void            PrintMoveHistory                (POSITION position);
UNDO*           Stalemate                       (UNDO*, POSITION stalematePos, BOOLEAN* abort);

/* Data output */

void            PrintMoves                      (MOVELIST* ptr, REMOTENESSLIST* remotePtr);
void            PrintValueMoves                 (POSITION pos);
BOOLEAN         PrintPossibleMoves              (POSITION pos);

REMOTENESS      FindDelta                       (REMOTENESS remote, REMOTENESSLIST* remoteptr, VALUE val);

/* Brains of the gameplay */

STRING          GetPrediction                   (POSITION pos, STRING playerName, BOOLEAN usersTurn);
STRING          GetSEvalPrediction                      (POSITION pos, STRING playerName, BOOLEAN usersTurn);
MOVE            GetComputersMove                (POSITION pos);
MOVE    GetSEvalMove        (POSITION pos);
VALUE_MOVES*    GetValueMoves                   (POSITION pos);

/* Player structure */
typedef enum {Human, Computer, Evaluator} PTYPE;

typedef struct Player {
	int turn;
	char* name;
	int type;
	USERINPUT (*GetMove)(POSITION,MOVE*,STRING);
} *PLAYER;

/* Player Constructors */
PLAYER          NewHumanPlayer                  (STRING name,int turn);
PLAYER          NewComputerPlayer               (STRING name,int turn);
PLAYER          NewSEvalPlayer                  (STRING name,int turn);
USERINPUT       RemoteMove                      (POSITION position,MOVE* move, STRING name);
USERINPUT       ComputerMove                    (POSITION position,MOVE* move, STRING name);
USERINPUT LocalPlayersMove(POSITION position, MOVE* move, STRING name);
USERINPUT       SEvalMove                    (POSITION position,MOVE* move, STRING name);

void            PlayGame                        (PLAYER playerOne,PLAYER playerTwo);

/* Move-choosing logic */
MOVE            RandomLargestRemotenessMove     (MOVELIST*, REMOTENESSLIST*);
MOVE            RandomSmallestRemotenessMove    (MOVELIST*, REMOTENESSLIST*);
MOVE            RandomLargestSEvalMove  (POSITIONLIST*, MOVELIST*);
MOVE            LargestWinningSEvalMove (POSITIONLIST*, MOVELIST*, float*);

/* WinBy-choosing logic */
MOVE GetWinByMove                           (POSITION, MOVELIST*);

#endif /* GMCORE_GAMEPLAY_H */
