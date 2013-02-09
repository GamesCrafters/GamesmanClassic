/************************************************************************
**
** NAME:        tkAppInit.c
**
** DESCRIPTION: Code for creating .so files so that the wish will know
**              the C commands in Gamesman.
**
** AUTHOR:      Dan Garcia  -  University of California at Berkeley
**              Copyright (C) Dan Garcia, 1999. All rights reserved.
**
** DATE:        1999-04-02
**
**************************************************************************/

#include "tk.h"
#include "gamesman.h"
#include <string.h>

extern STRING gValueString[];        /* The GAMESMAN Value strings */
extern BOOLEAN gStandardGame;
extern BOOLEAN kPartizan;
extern int smartness;
extern int scalelvl;

/*prototypes from the game module and core that are used here*/
extern char     *generic_hash_unhash(POSITION hash_number, char *empty_board);
extern int       move_hash (int, int, int, int);
extern POSITION  getInitialPosition();
extern int       generic_hash_turn (POSITION hashed);
extern int       destination (int, int);

/*
 * The following variable is a special hack that is needed in order for
 * Sun shared libraries to be used for Tcl.
 */

extern int matherr();
int *tclDummyMathPtr = (int *) matherr;

/*************************************************************************
**
** Begin prototype for commands invoked from tcl command requests
**
**************************************************************************/
static int PrintCmd _ANSI_ARGS_((ClientData clientData,
                                 Tcl_Interp *interp, int argc, char **argv));
static int InitialPositionCmd _ANSI_ARGS_((ClientData clientData,
                                           Tcl_Interp *interp, int argc, char **argv));
static int GenericUnhashCmd _ANSI_ARGS_((ClientData clientData,
                                         Tcl_Interp *interp, int argc, char **argv));
static int DestinationCmd _ANSI_ARGS_((ClientData clientData,
                                       Tcl_Interp *interp, int argc, char **argv));
static int MoveHashCmd _ANSI_ARGS_((ClientData clientData,
                                    Tcl_Interp *interp, int argc, char **argv));
static int WhoseMoveCmd _ANSI_ARGS_((ClientData clientData,
                                     Tcl_Interp *interp, int argc, char **argv));
static int FooCmd _ANSI_ARGS_((ClientData clientData,
                               Tcl_Interp *interp, int argc, char **argv));
static int InitializeCmd _ANSI_ARGS_((ClientData clientData,
                                      Tcl_Interp *interp, int argc, char **argv));
static int InitializeDatabasesCmd _ANSI_ARGS_((ClientData clientData,
                                               Tcl_Interp *interp, int argc, char **argv));
static int InitializeGameCmd _ANSI_ARGS_((ClientData clientData,
                                          Tcl_Interp *interp, int argc, char **argv));
static int GetComputersMoveCmd _ANSI_ARGS_((ClientData clientData,
                                            Tcl_Interp *interp, int argc, char **argv));
static int SetGameSpecificOptionsCmd _ANSI_ARGS_((ClientData clientData,
                                                  Tcl_Interp *interp, int argc, char **argv));
static int DetermineValueCmd _ANSI_ARGS_((ClientData clientData,
                                          Tcl_Interp *interp, int argc, char **argv));
static int GetValueOfPositionCmd _ANSI_ARGS_((ClientData clientData,
                                              Tcl_Interp *interp, int argc, char **argv));
static int RemotenessCmd _ANSI_ARGS_((ClientData clientData,
                                      Tcl_Interp *interp, int argc, char **argv));
static int MexCmd _ANSI_ARGS_((ClientData clientData,
                               Tcl_Interp *interp, int argc, char **argv));
static int DoMoveCmd _ANSI_ARGS_((ClientData clientData,
                                  Tcl_Interp *interp, int argc, char **argv));
static int PrimitiveCmd _ANSI_ARGS_((ClientData clientData,
                                     Tcl_Interp *interp, int argc, char **argv));
static int GetValueMovesCmd _ANSI_ARGS_((ClientData clientData,
                                         Tcl_Interp *interp, int argc, char **argv));
static int RandomCmd _ANSI_ARGS_((ClientData clientData,
                                  Tcl_Interp *interp, int argc, char **argv));
 #ifdef COMPUTEC
static int ComputeCCmd _ANSI_ARGS_((ClientData clientData,
                                    Tcl_Interp *interp, int argc, char **argv));
 #endif
static int GoAgainCmd _ANSI_ARGS_((ClientData clientData,
                                   Tcl_Interp *interp, int argc, char **argv));
static int GetPredictionCmd _ANSI_ARGS_((ClientData clientData,
                                         Tcl_Interp *interp, int argc, char **argv));
static int SetSmarterComputerCmd _ANSI_ARGS_((ClientData clientData,
                                              Tcl_Interp *interp, int argc, char **argv));
static int GetOptionCmd _ANSI_ARGS_((ClientData clientData,
                                     Tcl_Interp *interp, int argc, char **argv));
static int SetOptionCmd _ANSI_ARGS_((ClientData clientData,
                                     Tcl_Interp *interp, int argc, char **argv));

/************************************************************************
**
** NAME:        Gamesman_Init
**
** DESCRIPTION: This is the
**
** INPUTS:      Tcl_Interp *interp : Interpreter for application
**
** OUTPUTS:     TCL_OK : Confirmation that we exited successfully
**
** CALLS:       Tcl_CreateCommand
**
************************************************************************/

int
Gamesman_Init(interp)
Tcl_Interp *interp;             /* Interpreter for application. */
{
	Tk_Window mainWindow;

	mainWindow = Tk_MainWindow(interp);
	Tcl_CreateCommand(interp, "Print", (Tcl_CmdProc*) PrintCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "C_InitialPosition", (Tcl_CmdProc*) InitialPositionCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "C_GenericUnhash", (Tcl_CmdProc*) GenericUnhashCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "C_Destination", (Tcl_CmdProc*) DestinationCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "C_MoveHash", (Tcl_CmdProc*) MoveHashCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "C_WhoseMove", (Tcl_CmdProc*) WhoseMoveCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "foo", (Tcl_CmdProc*) FooCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "C_Initialize", (Tcl_CmdProc*) InitializeCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "C_InitializeDatabases", (Tcl_CmdProc*) InitializeDatabasesCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "C_InitializeGame", (Tcl_CmdProc*) InitializeGameCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "C_GetComputersMove", (Tcl_CmdProc*) GetComputersMoveCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "C_SetGameSpecificOptions", (Tcl_CmdProc*) SetGameSpecificOptionsCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "C_DetermineValue", (Tcl_CmdProc*) DetermineValueCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "C_GetValueOfPosition", (Tcl_CmdProc*) GetValueOfPositionCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "C_Remoteness", (Tcl_CmdProc*) RemotenessCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "C_Mex", (Tcl_CmdProc*) MexCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "C_DoMove", (Tcl_CmdProc*) DoMoveCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "C_Primitive", (Tcl_CmdProc*) PrimitiveCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "C_GetValueMoves", (Tcl_CmdProc*) GetValueMovesCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "C_Random", (Tcl_CmdProc*) RandomCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
#ifdef COMPUTEC
	Tcl_CreateCommand(interp, "C_ComputeC", (Tcl_CmdProc*) ComputeCCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
#endif
	Tcl_CreateCommand(interp, "C_GoAgain", (Tcl_CmdProc*) GoAgainCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "C_GetPrediction", (Tcl_CmdProc*) GetPredictionCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "C_SetSmarterComputer", (Tcl_CmdProc*) SetSmarterComputerCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "C_GetOption", (Tcl_CmdProc*) GetOptionCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "C_SetOption", (Tcl_CmdProc*) SetOptionCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);

	{
		int (*fptr)(Tcl_Interp *interp,Tk_Window) = (int (*)(Tcl_Interp*, Tk_Window))gGameSpecificTclInit;

		if (fptr)
			(fptr)(interp,mainWindow);
	}

	return TCL_OK;
}

/*************************************************************************
**
** Begin commands invoked from the tcl command requests coming ni
**
**************************************************************************/
static int
InitialPositionCmd(dummy, interp, argc, argv)
ClientData dummy;
Tcl_Interp *interp;
int argc;
char **argv;
{
	if (argc != 1) {
		Tcl_SetResult(interp, "wrong # args: shouldn't be any", TCL_STATIC);
		return TCL_ERROR;
	}
	Tcl_SetResult(interp, StrFromI((int)getInitialPosition()), SafeFreeString);
	return TCL_OK;
}

static int
GenericUnhashCmd(dummy, interp, argc, argv)
ClientData dummy;
Tcl_Interp *interp;
int argc;
char **argv;
{
	//argv[1] is position, argv[2] is boardsize
	if (argc != 3) {
		Tcl_SetResult(interp,  "wrong # args: should be 2", TCL_STATIC);
		return TCL_ERROR;
	}
	char *board;
	board = (char *) SafeMalloc (sizeof(char)*(atoi(argv[2])));
	generic_hash_unhash(atoi(argv[1]),board);
	Tcl_SetResult(interp, StringDup(board), SafeFreeString);
	return TCL_OK;
}

static int
DestinationCmd(dummy, interp, argc, argv)
ClientData dummy;
Tcl_Interp *interp;
int argc;
char **argv;
{
	//argv[1] is slot#, argv[2] is direction#
	if (argc != 3) {
		Tcl_SetResult(interp,  "wrong # args: should be 2", TCL_STATIC);
		return TCL_ERROR;
	}
	Tcl_SetResult(interp, StrFromI( destination(atoi(argv[1]), atoi(argv[2]))), SafeFreeString);
	return TCL_OK;
}

static int
MoveHashCmd(dummy, interp, argc, argv)
ClientData dummy;
Tcl_Interp *interp;
int argc;
char **argv;
{
	//argv1-3 are slot #s, argv[4] is direction #
	if (argc !=5) {
		Tcl_SetResult(interp,  "wrong # args: should be 4", TCL_STATIC);
		return TCL_ERROR;
	}
	Tcl_SetResult(interp, StrFromI( move_hash(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4]))), SafeFreeString);
	return TCL_OK;
}

static int
PrintCmd(dummy, interp, argc, argv)
ClientData dummy;
Tcl_Interp *interp;
int argc;
char **argv;
{
	printf("%s\n",argv[1]);
	return TCL_OK;
}


static int
WhoseMoveCmd(dummy, interp, argc, argv)
ClientData dummy;
Tcl_Interp *interp;
int argc;
char **argv;
{
	if (argc !=2) {
		Tcl_SetResult(interp,  "wrong # args: should be one", TCL_STATIC);
		return TCL_ERROR;
	}
	Tcl_SetResult(interp, StrFromI( generic_hash_turn(atoi(argv[1]))), SafeFreeString);
	return TCL_OK;
}

static int
FooCmd(dummy, interp, argc, argv)
ClientData dummy;                       /* Not used. */
Tcl_Interp *interp;                             /* Current interpreter. */
int argc;                               /* Number of arguments. */
char **argv;                            /* Argument strings. */
{
	if (argc != 4) {
		Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
		                 " x y z\"", (char *) NULL);
		return TCL_ERROR;
	}
	else {
		Tcl_SetResult(interp,  "Dan Garcia", TCL_STATIC);
		return TCL_OK;
	}
}

static int
InitializeCmd(dummy, interp, argc, argv)
ClientData dummy;                       /* Not used. */
Tcl_Interp *interp;                     /* Current interpreter. */
int argc;                               /* Number of arguments. */
char **argv;                            /* Argument strings. */
{
	if (argc != 1) {
		Tcl_SetResult(interp,  "wrong # args: shouldn't have any args", TCL_STATIC);
		return TCL_ERROR;
	}
	else {
		Initialize();
		Tcl_SetResult(interp,  "System Initialized", TCL_STATIC);
		return TCL_OK;
	}
}

static int
InitializeGameCmd(dummy, interp, argc, argv)
ClientData dummy;                       /* Not used. */
Tcl_Interp *interp;                     /* Current interpreter. */
int argc;                               /* Number of arguments. */
char **argv;                            /* Argument strings. */
{
	if (argc != 1) {
		Tcl_SetResult(interp,  "wrong # args: shouldn't have any args", TCL_STATIC);
		return TCL_ERROR;
	}
	else {
		InitializeGame();
		Tcl_SetResult(interp,  "Game Initialized", TCL_STATIC);
		return TCL_OK;
	}
}

static int
InitializeDatabasesCmd(dummy, interp, argc, argv)
ClientData dummy;                       /* Not used. */
Tcl_Interp *interp;                     /* Current interpreter. */
int argc;                               /* Number of arguments. */
char **argv;                            /* Argument strings. */
{
	if (argc != 1) {
		Tcl_SetResult(interp,  "wrong # args: shouldn't have any args", TCL_STATIC);
		return TCL_ERROR;
	}
	else {
		InitializeDatabases();
		Tcl_SetResult(interp,  "Databases Initialized", TCL_STATIC);
		return TCL_OK;
	}
}

static int
GetComputersMoveCmd(dummy, interp, argc, argv)
ClientData dummy;                       /* Not used. */
Tcl_Interp *interp;                     /* Current interpreter. */
int argc;                               /* Number of arguments. */
char **argv;                            /* Argument strings. */
{
	MOVE GetComputersMove();
	POSITION position;

	if (argc != 2) {
		Tcl_SetResult(interp,  "wrong # args: GetComputerMove (int)Position", TCL_STATIC);
		return TCL_ERROR;
	}
	else {
		if (sscanf(argv[1], POSITION_FORMAT, &position) == EOF)
			return TCL_ERROR;

		Tcl_SetResult(interp, StrFromI((int)GetComputersMove(position)), SafeFreeString);
		return TCL_OK;
	}
}

static int
SetGameSpecificOptionsCmd(dummy, interp, argc, argv)
ClientData dummy;                       /* Not used. */
Tcl_Interp *interp;                     /* Current interpreter. */
int argc;                               /* Number of arguments. */
char **argv;                            /* Argument strings. */
{
	int i, theOptions[100];
	int standardGame = 0;

	if (argc < 2) {
		Tcl_SetResult(interp,  "wrong # args: SetGameSpecificOptions (boolean)Standard-Game-p (optional)Other-Game-Specific-Options", TCL_STATIC);
		return TCL_ERROR;
	}

	if (argc > 102) {
		Tcl_SetResult(interp,  "too many arguments: SetGameSpecificOptions (boolean)Standard-Game-p [, (boolean)option_n]*", TCL_STATIC);
		return TCL_ERROR;
	}

	if(Tcl_GetInt(interp, argv[1], &standardGame) != TCL_OK)
		return TCL_ERROR;

	if (argc > 2)
		for(i=2; i<argc; i++)
			if(Tcl_GetInt(interp, argv[i], &theOptions[i-2]) != TCL_OK)
				return TCL_ERROR;

	gStandardGame = standardGame;
	SetTclCGameSpecificOptions(theOptions);

	return TCL_OK;
}

static int
DetermineValueCmd(dummy, interp, argc, argv)
ClientData dummy;                       /* Not used. */
Tcl_Interp *interp;                     /* Current interpreter. */
int argc;                               /* Number of arguments. */
char **argv;                            /* Argument strings. */
{
	POSITION position;
	VALUE DetermineValue(), DetermineLoopyValue();

	if (argc != 2) {
		Tcl_SetResult(interp,  "wrong # args: DetermineValue (POSITION)Position", TCL_STATIC);
		return TCL_ERROR;
	}
	else {
		if (sscanf(argv[1], POSITION_FORMAT, &position) == EOF)
			return TCL_ERROR;

		Tcl_SetResult(interp,  gValueString[(int)DetermineValue(position)], TCL_STATIC);

		return TCL_OK;
	}
}

static int
GetValueOfPositionCmd(dummy, interp, argc, argv)
ClientData dummy;                       /* Not used. */
Tcl_Interp *interp;                     /* Current interpreter. */
int argc;                               /* Number of arguments. */
char **argv;                            /* Argument strings. */
{
	POSITION position;
	VALUE GetValueOfPosition();

	if (argc != 2) {
		Tcl_SetResult(interp,  "wrong # args: GetValueOfPosition (POSITION)Position", TCL_STATIC);
		return TCL_ERROR;
	}
	else {
		if (sscanf(argv[1], POSITION_FORMAT, &position) == EOF)
			return TCL_ERROR;

		Tcl_SetResult(interp,  gValueString[(int)GetValueOfPosition(position)], TCL_STATIC);
		return TCL_OK;
	}
}

static int
RemotenessCmd(dummy, interp, argc, argv)
ClientData dummy;                       /* Not used. */
Tcl_Interp *interp;                     /* Current interpreter. */
int argc;                               /* Number of arguments. */
char **argv;                            /* Argument strings. */
{
	POSITION position;
	REMOTENESS Remoteness();

	if (argc != 2) {
		Tcl_SetResult(interp,  "wrong # args: Remoteness (POSITION)Position", TCL_STATIC);
		return TCL_ERROR;
	}
	else {
		if (sscanf(argv[1], POSITION_FORMAT, &position) == EOF)
			return TCL_ERROR;

		Tcl_SetResult(interp, StrFromI((int)Remoteness(position)), SafeFreeString);
		return TCL_OK;
	}
}

static int
MexCmd(dummy, interp, argc, argv)
ClientData dummy;                       /* Not used. */
Tcl_Interp *interp;                     /* Current interpreter. */
int argc;                               /* Number of arguments. */
char **argv;                            /* Argument strings. */
{
	POSITION position;
	REMOTENESS Remoteness();

	if (argc != 2) {
		Tcl_SetResult(interp,  "wrong # args: Mex (POSITION)Position", TCL_STATIC);
		return TCL_ERROR;
	}
	else {
		if (sscanf(argv[1], POSITION_FORMAT, &position) == EOF)
			return TCL_ERROR;

		if(!kPartizan)
			MexFormat(position,Tcl_GetStringResult(interp));
		else
			Tcl_SetResult(interp, " ", TCL_STATIC);
		return TCL_OK;
	}
}

static int
DoMoveCmd(dummy, interp, argc, argv)
ClientData dummy;                       /* Not used. */
Tcl_Interp *interp;                     /* Current interpreter. */
int argc;                               /* Number of arguments. */
char **argv;                            /* Argument strings. */
{
	POSITION position;
	MOVE move;
	POSITION DoMove();

	if (argc != 3) {
		Tcl_SetResult(interp,  "wrong # args: DoMove (int)Position (int)Move", TCL_STATIC);
		return TCL_ERROR;
	}
	else {
		if (sscanf(argv[1], POSITION_FORMAT, &position) == EOF)
			return TCL_ERROR;
		if(Tcl_GetInt(interp, argv[2], &move) != TCL_OK)
			return TCL_ERROR;

		Tcl_SetResult(interp, StrFromI((int)DoMove(position,move)), SafeFreeString);
		return TCL_OK;
	}
}

static int
GoAgainCmd(dummy, interp, argc, argv)
ClientData dummy;                       /* Not used. */
Tcl_Interp *interp;                     /* Current interpreter. */
int argc;                               /* Number of arguments. */
char **argv;                            /* Argument strings. */
{
	POSITION position;
	MOVE move;
	extern BOOLEAN (*gGoAgain)(POSITION,MOVE);

	if (argc != 3) {
		Tcl_SetResult(interp,  "wrong # args: GoAgain (int)Position (int)Move", TCL_STATIC);
		return TCL_ERROR;
	}
	else {
		if (sscanf(argv[1], POSITION_FORMAT, &position) == EOF)
			return TCL_ERROR;
		if(Tcl_GetInt(interp, argv[2], &move) != TCL_OK)
			return TCL_ERROR;

		Tcl_SetResult(interp, StrFromI((int)gGoAgain(position,move)), SafeFreeString);
		return TCL_OK;
	}
}

static int
PrimitiveCmd(dummy, interp, argc, argv)
ClientData dummy;                       /* Not used. */
Tcl_Interp *interp;                     /* Current interpreter. */
int argc;                               /* Number of arguments. */
char **argv;                            /* Argument strings. */
{
	POSITION position;
	VALUE Primitive();

	if (argc != 2) {
		Tcl_SetResult(interp,  "wrong # args: Primitive (int)Position", TCL_STATIC);
		return TCL_ERROR;
	}
	else {
		if (sscanf(argv[1], POSITION_FORMAT, &position) == EOF)
			return TCL_ERROR;

		Tcl_SetResult(interp,  gValueString[(int)Primitive(position)], TCL_STATIC);
		return TCL_OK;
	}
}

static int
GetValueMovesCmd(dummy, interp, argc, argv)
ClientData dummy;                       /* Not used. */
Tcl_Interp *interp;                     /* Current interpreter. */
int argc;                               /* Number of arguments. */
char **argv;                            /* Argument strings. */
{
	POSITION position;
	MOVELIST *ptr, *head;
	VALUE value;
	char theAnswer[10000], tmp[1000];

	if (argc != 2) {
		Tcl_SetResult(interp,  "wrong # args: GetValueMoves (int)Position", TCL_STATIC);
		return TCL_ERROR;
	}
	else {
		if (sscanf(argv[1], POSITION_FORMAT, &position) == EOF)
			return TCL_ERROR;

		head = ptr = GenerateMoves(position);
		theAnswer[0] = '\0';
		while (ptr != NULL) {
			value = GetValueOfPosition(DoMove(position,ptr->move));

			if (gGoAgain(position,ptr->move)) {
				switch(value) {
				case win: value = lose; break;
				case lose: value = win; break;
				default: value = value;
				}
			}

			sprintf(tmp,"{ %d %s } ",
			        (ptr->move),
			        gValueString[value]);
			/* If this barfs, change 'char' to 'const char' */
			strcpy(theAnswer,(char *)strcat(theAnswer,tmp));
			ptr = ptr->next;
		}

		Tcl_SetResult(interp,theAnswer,TCL_VOLATILE);

		FreeMoveList(head);
		return TCL_OK;
	}
}

static int
RandomCmd(dummy, interp, argc, argv)
ClientData dummy;                       /* Not used. */
Tcl_Interp *interp;                     /* Current interpreter. */
int argc;                               /* Number of arguments. */
char **argv;                            /* Argument strings. */
{
	int GetRandomNumber();
	int n;

	if (argc != 2) {
		Tcl_SetResult(interp,  "wrong # args: Random (int)n", TCL_STATIC);
		return TCL_ERROR;
	}
	else {
		if(Tcl_GetInt(interp, argv[1], &n) != TCL_OK)
			return TCL_ERROR;

		Tcl_SetResult(interp, StrFromI(GetRandomNumber(n)), SafeFreeString);
		return TCL_OK;
	}
}

static int
GetPredictionCmd(dummy, interp, argc, argv)
ClientData dummy;                       /* Not used. */
Tcl_Interp *interp;                     /* Current interpreter. */
int argc;                               /* Number of arguments. */
char **argv;                            /* Argument strings. */
{
	POSITION position;
	STRING playerName, prediction;

	if (argc != 3) {
		Tcl_SetResult(interp,  "wrong # args: GetPrediction position playerName", TCL_STATIC);
		return TCL_ERROR;
	}

	else {
		if (sscanf(argv[1], POSITION_FORMAT, &position) == EOF)
			return TCL_ERROR;
		playerName = (STRING) argv[2];

		prediction =  GetPrediction(position, playerName, TRUE);
		Tcl_SetResult(interp, StringDup(prediction), SafeFreeString);
		return TCL_OK;
	}
}

static int
SetSmarterComputerCmd(dummy, interp, argc, argv)
ClientData dummy;                       /* Not used. */
Tcl_Interp *interp;                     /* Current interpreter. */
int argc;                               /* Number of arguments. */
char **argv;                            /* Argument strings. */
{
	int smartnessScale;
	STRING smartnessString;

	if (argc > 3 || argc < 2) {
		Tcl_SetResult(interp,  "wrong # args: SetSmarterComputer smartness [scale]", TCL_STATIC);
		return TCL_ERROR;
	}

	else {
		smartnessString = (STRING) argv[1];
		Tcl_SetResult(interp, StringFormat(256,  "Setting smarter computer to %s\n", smartnessString), SafeFreeString);

		if (strcmp(smartnessString, "Perfectly") == 0) {
			smartness = SMART;
			scalelvl = 100;
		}
		else if (strcmp(smartnessString, "Imperfectly") == 0) {
			smartness = SMART;
			if(Tcl_GetInt(interp, argv[2], &smartnessScale) != TCL_OK)
				return TCL_ERROR;
			scalelvl = smartnessScale;
		}
		else if (strcmp(smartnessString, "Randomly") == 0) {
			smartness = RANDOM;
		}
		else if (strcmp(smartnessString, "Miserely") == 0) {
			smartness = DUMB;
		}
		else {
			return TCL_ERROR;
		}

		return TCL_OK;
	}
}

static int
SetOptionCmd(dummy, interp, argc, argv)
ClientData dummy;                       /* Not used. */
Tcl_Interp *interp;                     /* Current interpreter. */
int argc;                               /* Number of arguments. */
char **argv;                            /* Argument strings. */
{
	int option;

	if (argc != 2) {
		Tcl_SetResult(interp,  "wrong # args: SetOption (int)option", TCL_STATIC);
		return TCL_ERROR;
	}
	else {
		if(Tcl_GetInt(interp, argv[1], &option) != TCL_OK)
			return TCL_ERROR;

		setOption(option);

		return TCL_OK;
	}
}

static int
GetOptionCmd(dummy, interp, argc, argv)
ClientData dummy;                       /* Not used. */
Tcl_Interp *interp;                     /* Current interpreter. */
int argc;                               /* Number of arguments. */
char **argv;                            /* Argument strings. */
{
	int option;

	if (argc != 1) {
		Tcl_SetResult(interp,  "wrong # args: GetOption", TCL_STATIC);
		return TCL_ERROR;
	}
	else {

		option = getOption();
		Tcl_SetResult(interp, StrFromI(option), SafeFreeString);

		return TCL_OK;
	}
}

 #ifdef COMPUTEC
static int
ComputeCCmd(dummy, interp, argc, argv)
ClientData dummy;                       /* Not used. */
Tcl_Interp *interp;                     /* Current interpreter. */
int argc;                               /* Number of arguments. */
char **argv;                            /* Argument strings. */
{
	int ComputeC();
	int n,k;

	if (argc != 3) {
		Tcl_SetResult(interp,  "wrong # args: Random (int)n", TCL_STATIC);
		return TCL_ERROR;
	}
	else {
		if(Tcl_GetInt(interp, argv[1], &n) != TCL_OK)
			return TCL_ERROR;
		if(Tcl_GetInt(interp, argv[2], &k) != TCL_OK)
			return TCL_ERROR;

		Tcl_SetResult(interp, StrFromI(ComputeC(n,k)), SafeFreeString);
		return TCL_OK;
	}
}
 #endif



