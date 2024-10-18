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
** LAST CHANGE: $Id: tkAppInit.c,v 1.44 2007-11-15 02:53:19 dounanshi Exp $
**
**************************************************************************/

#include "tk.h"
#include "gamesman.h"
#include "hash.h"
#include <string.h>
#include "core/analysis.h"
#include "core/gameplay.h"
#include "core/solveretrograde.h" // For InitTierGamesman()

extern STRING gValueString[]; /* The GAMESMAN Value strings */
extern BOOLEAN gStandardGame;
extern BOOLEAN kPartizan;
extern POSITION gInitialPosition;
extern int smartness;
extern int scalelvl;
extern MENU gMenuMode;
extern STRING kHelpStandardObjective;
extern STRING kHelpReverseObjective;

/*
 * The following variable is a special hack that is needed in order for
 * Sun shared libraries to be used for Tcl.
 * 
 * For more recent versions of Tcl, the hack may no longer be applicable.
 * We avoid the matherr() function for Tcl beginning 8.4.
 * See: https://wiki.tcl-lang.org/page/tclDummyMathPtr
 */

#define TCL_VERSION_NUM (TCL_MAJOR_VERSION * 100 + TCL_MINOR_VERSION)

#if TCL_VERSION_NUM < 804
extern int matherr();
int *tclDummyMathPtr = (int *) matherr;
#endif

/*************************************************************************
**
** Begin prototype for commands invoked from tcl command requests
**
**************************************************************************/

static int InitialPositionCmd _ANSI_ARGS_((ClientData clientData,
                                           Tcl_Interp *interp, int argc, char **argv));

static int GenericUnhashCmd _ANSI_ARGS_((ClientData clientData,
                                         Tcl_Interp *interp, int argc, char **argv));
static int CustomUnhashCmd _ANSI_ARGS_((ClientData clientData,
                                        Tcl_Interp *interp, int argc, char **argv));
static int ReturnTurnCmd _ANSI_ARGS_((ClientData clientData,
                                      Tcl_Interp *interp, int argc, char **argv));
static int PossibleMoves _ANSI_ARGS_((ClientData clientData,
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
static int GetRemotePlayersMoveCmd _ANSI_ARGS_((ClientData clientData,
                                                Tcl_Interp *interp, int argc, char **argv));
static int SendLocalPlayersMoveCmd _ANSI_ARGS_((ClientData clientData,
                                                Tcl_Interp *interp, int argc, char **argv));
static int SendGameOverCmd _ANSI_ARGS_((ClientData clientData,
                                        Tcl_Interp *interp, int argc, char **argv));
static int SendResignCmd _ANSI_ARGS_((ClientData clientData,
                                      Tcl_Interp *interp, int argc, char **argv));
static int LoginUserCmd _ANSI_ARGS_((ClientData clientData,
                                     Tcl_Interp *interp, int argc, char **argv));
static int LogoutUserCmd _ANSI_ARGS_((ClientData clientData,
                                      Tcl_Interp *interp, int argc, char **argv));
static int RegisterUserCmd _ANSI_ARGS_((ClientData clientData,
                                        Tcl_Interp *interp, int argc, char **argv));
static int GetUsersCmd _ANSI_ARGS_((ClientData clientData,
                                    Tcl_Interp *interp, int argc, char **argv));
static int RegisterGameCmd _ANSI_ARGS_((ClientData clientData,
                                        Tcl_Interp *interp, int argc, char **argv));
static int JoinGameCmd _ANSI_ARGS_((ClientData clientData,
                                    Tcl_Interp *interp, int argc, char **argv));
static int ReceivedChallengeCmd _ANSI_ARGS_((ClientData clientData,
                                             Tcl_Interp *interp, int argc, char **argv));
static int AcceptChallengeCmd _ANSI_ARGS_((ClientData clientData,
                                           Tcl_Interp *interp, int argc, char **argv));
static int GetGameStatusCmd _ANSI_ARGS_((ClientData clientData,
                                         Tcl_Interp *interp, int argc, char **argv));
static int AcceptedChallengeCmd _ANSI_ARGS_((ClientData clientData,
                                             Tcl_Interp *interp, int argc, char **argv));
static int UnregisterGameCmd _ANSI_ARGS_((ClientData clientData,
                                          Tcl_Interp *interp, int argc, char **argv));
static int DeselectChallengerCmd _ANSI_ARGS_((ClientData clientData,
                                              Tcl_Interp *interp, int argc, char **argv));
static int SelectChallengerCmd _ANSI_ARGS_((ClientData clientData,
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

static int percentDoneCmd _ANSI_ARGS_((ClientData clientData,
                                       Tcl_Interp *interp, int argc, char **argv));
static int GetStandardObjStringCmd _ANSI_ARGS_((ClientData clientData,
                                                Tcl_Interp *interp, int argc, char **argv));
static int GetReverseObjStringCmd _ANSI_ARGS_((ClientData clientData,
                                               Tcl_Interp *interp, int argc, char **argv));


static int moveToStringCmd _ANSI_ARGS_((ClientData clientData,
                                        Tcl_Interp *interp, int argc, char **argv));

static int UsingTiersCmd _ANSI_ARGS_((ClientData clientData,
                                      Tcl_Interp *interp, int argc, char **argv));
static int InitHashWindowCmd _ANSI_ARGS_((ClientData clientData,
                                          Tcl_Interp *interp, int argc, char **argv));
static int HashWindowCmd _ANSI_ARGS_((ClientData clientData,
                                      Tcl_Interp *interp, int argc, char **argv));
static int HashWindowUndoCmd _ANSI_ARGS_((ClientData clientData,
                                          Tcl_Interp *interp, int argc, char **argv));
static int CurrentTierCmd _ANSI_ARGS_((ClientData clientData,
                                       Tcl_Interp *interp, int argc, char **argv));
static int InitTierGamesmanIfNeededCmd _ANSI_ARGS_((ClientData clientData,
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

	Tcl_CreateCommand(interp, "C_InitialPosition", (Tcl_CmdProc*) InitialPositionCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);

	Tcl_CreateCommand(interp, "C_GenericUnhash", (Tcl_CmdProc*) GenericUnhashCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "C_CustomUnhash", (Tcl_CmdProc*) CustomUnhashCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "C_ReturnTurn", (Tcl_CmdProc*) ReturnTurnCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "C_PossibleMoves", (Tcl_CmdProc*) PossibleMoves, (ClientData) mainWindow,
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
	Tcl_CreateCommand(interp, "C_GetRemotePlayersMove", (Tcl_CmdProc*) GetRemotePlayersMoveCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "C_SendLocalPlayersMove", (Tcl_CmdProc*) SendLocalPlayersMoveCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "C_SendGameOver", (Tcl_CmdProc*) SendGameOverCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "C_SendResign", (Tcl_CmdProc*) SendResignCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "C_LoginUser", (Tcl_CmdProc*) LoginUserCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "C_LogoutUser", (Tcl_CmdProc*) LogoutUserCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "C_RegisterUser", (Tcl_CmdProc*) RegisterUserCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "C_GetUsers", (Tcl_CmdProc*) GetUsersCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "C_RegisterGame", (Tcl_CmdProc*) RegisterGameCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "C_JoinGame", (Tcl_CmdProc*) JoinGameCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "C_ReceivedChallenge", (Tcl_CmdProc*) ReceivedChallengeCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "C_AcceptChallenge", (Tcl_CmdProc*) AcceptChallengeCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "C_GetGameStatus", (Tcl_CmdProc*) GetGameStatusCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "C_AcceptedChallenge", (Tcl_CmdProc*) AcceptedChallengeCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "C_UnregisterGame", (Tcl_CmdProc*) UnregisterGameCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "C_DeselectChallenger", (Tcl_CmdProc*) DeselectChallengerCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "C_SelectChallenger", (Tcl_CmdProc*) SelectChallengerCmd, (ClientData) mainWindow,
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

	Tcl_CreateCommand(interp, "C_PercentDone", (Tcl_CmdProc*) percentDoneCmd, (ClientData) mainWindow, (Tcl_CmdDeleteProc*) NULL);

	Tcl_CreateCommand(interp, "C_GetStandardObjString", (Tcl_CmdProc*) GetStandardObjStringCmd, (ClientData) mainWindow, (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "C_GetReverseObjString", (Tcl_CmdProc*) GetReverseObjStringCmd, (ClientData) mainWindow, (Tcl_CmdDeleteProc*) NULL);

	Tcl_CreateCommand(interp, "C_MoveToString", (Tcl_CmdProc*) moveToStringCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);

	Tcl_CreateCommand(interp, "C_UsingTiers", (Tcl_CmdProc*) UsingTiersCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "C_InitHashWindow", (Tcl_CmdProc*) InitHashWindowCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "C_HashWindow", (Tcl_CmdProc*) HashWindowCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "C_HashWindowUndo", (Tcl_CmdProc*) HashWindowUndoCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "C_CurrentTier", (Tcl_CmdProc*) CurrentTierCmd, (ClientData) mainWindow,
	                  (Tcl_CmdDeleteProc*) NULL);
	Tcl_CreateCommand(interp, "C_InitTierGamesmanIfNeeded", (Tcl_CmdProc*) InitTierGamesmanIfNeededCmd, (ClientData) mainWindow,
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
** Begin commands invoked from the tcl command requests coming in
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
	// Ported from tkAppInitHash, correct version tbd
	Tcl_SetResult(interp, StrFromI(gInitialPosition), SafeFreeString);
	return TCL_OK;
}

static int
GenericUnhashCmd(dummy, interp, argc, argv)
ClientData dummy;
Tcl_Interp *interp;
int argc;
char **argv;
{
	/* argv[1] is position, argv[2] USED to be boardsize */
	if (!(argc == 2 || argc == 3)) {
		Tcl_SetResult(interp,  "wrong # args: should be 1 (or 2 for backwards compat)", TCL_STATIC);
		return TCL_ERROR;
	}
	// Ported from tkAppInitHash, correct version tbd
	char *board;
	POSITION pos = atoi(argv[1]);
	board = generic_hash_unhash_tcl(pos);
	Tcl_SetResult(interp, board, SafeFreeString);
	return TCL_OK;
	/*
	   char *board;
	   int n;
	   board = (char *) SafeMalloc (sizeof(char)*(atoi(argv[2])));
	   generic_hash_unhash(atoi(argv[1]),board);
	   Tcl_SetResult(interp, board, SafeFreeString);
	   return TCL_OK;
	 */
}

static int
CustomUnhashCmd(dummy, interp, argc, argv)
ClientData dummy;
Tcl_Interp *interp;
int argc;
char **argv;
{
	/* argv[1] is position, argv[2] USED to be boardsize */
	if (!(argc == 2 || argc == 3)) {
		Tcl_SetResult(interp,  "wrong # args: should be 1 (or 2 for backwards compat)", TCL_STATIC);
		return TCL_ERROR;
	} else if (gCustomUnhash == NULL) {
		Tcl_SetResult(interp,  "CustomUnhash is not defined for this game", TCL_STATIC);
		return TCL_ERROR;
	}
	// Ported from tkAppInitHash, correct version tbd
	char *board;
	POSITION pos = atoi(argv[1]);
	board = gCustomUnhash(pos);
	Tcl_SetResult(interp, board, SafeFreeString);
	return TCL_OK;
}

//if the turn has been encoded elsewhere besides the position (i.e. tierposition), it can be retrieved here
static int
ReturnTurnCmd(dummy, interp, argc, argv)
ClientData dummy;
Tcl_Interp *interp;
int argc;
char **argv;
{
	/* argv[1] is position, argv[2] USED to be boardsize */
	if (!(argc == 2 || argc == 3)) {
		Tcl_SetResult(interp,  "wrong # args: should be 1 (or 2 for backwards compat)", TCL_STATIC);
		return TCL_ERROR;
	} else if (gReturnTurn == NULL) {
		Tcl_SetResult(interp,  "ReturnTurn is not defined for this game", TCL_STATIC);
		return TCL_ERROR;
	}
	// Ported from tkAppInitHash, correct version tbd
	char turn;
	POSITION pos = atoi(argv[1]);
	turn = gReturnTurn(pos);
	Tcl_SetResult(interp, StringFormat(1, "%c", turn), SafeFreeString);
	return TCL_OK;
}

static int
PossibleMoves(dummy, interp, argc, argv)
ClientData dummy;
Tcl_Interp *interp;
int argc;
char **argv;
{
	POSITION position;
	MOVELIST *ptr, *head;
	char theAnswer[10000], tmp[1000];
	char *temp = (char *) malloc (1000);
	int blah;
	//char *theAnswer = (char *) malloc (10000);
	//char *tmp = (char *) malloc (10000);
	if (argc != 2) {
		Tcl_SetResult(interp,  "wrong # args: PossibleMoves (int)Position", TCL_STATIC);
		return TCL_ERROR;
	}
	else {
		if (sscanf(argv[1], POSITION_FORMAT, &position) == EOF)
			return TCL_ERROR;
		head = ptr = GenerateMoves(position);
		theAnswer[0] = '\0';
		while (ptr != NULL) {
			temp = gMoveToStringFunPtr(ptr->move);
			blah = strlen(temp);
			temp[0]='{';
			temp[blah-1]='}';
			sprintf(tmp,"%s ", temp);
			strcpy(theAnswer,(char *)strcat(theAnswer,tmp));
			ptr = ptr->next;
		}
		Tcl_SetResult(interp,theAnswer,TCL_VOLATILE);
		//Tcl_SetResult(interp,theAnswer,TCL_VOLATILE);
		//SafeFree(theAnswer);
		//SafeFree(tmp);
		FreeMoveList(head);
		return TCL_OK;
	}
}


static int
FooCmd(dummy, interp, argc, argv)
ClientData dummy;                       /* Not used. */
Tcl_Interp *interp;                     /* Current interpreter. */
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
	char **ap, *args[20];
	int numArgs = 0;
	if (argc != 2) {
		Tcl_SetResult(interp,  "wrong # args: shouldn't have any args", TCL_STATIC);
		return TCL_ERROR;
	}
	else {
		for (ap = args; (*ap = strsep(&argv[1], " \t")) != NULL; ) {
			if (**ap != '\0') {
				if (++ap >= &args[20])
					break;
				else
					numArgs++;
			}
		}

		HandleArguments(numArgs, args);
		gTierSolverMenu = FALSE; // For TIER-GAMESMAN, makes sure to turn off the menu and auto-solve
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
		gHashWindowInitialized = FALSE; // For TIER-GAMESMAN, just in case
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
		Tcl_SetResult(interp,  "wrong # args: GetComputersMove (int)Position", TCL_STATIC);
		return TCL_ERROR;
	}
	else {
		if (sscanf(argv[1], POSITION_FORMAT, &position) == EOF)
			return TCL_ERROR;

		Tcl_SetResult(interp, StrFromI(GetComputersMove(position)), SafeFreeString);
		return TCL_OK;
	}
}

/* [C_GetRemotePlayersMove $username $password $sessionId $gameId $position] */
static int
GetRemotePlayersMoveCmd(dummy, interp, argc, argv)
ClientData dummy;                       /* Not used. */
Tcl_Interp *interp;                     /* Current interpreter. */
int argc;                               /* Number of arguments. */
char **argv;                            /* Argument strings. */
{
	int GetLastOnlineMove();
	POSITION position;
	int errCode;
	char* errMsg;
	char* move;

	if (argc != 6) {
		Tcl_SetResult(interp,  "wrong # args: GetRemotePlayersMove (char*)$username (char*)$password (char*)$sessionId (char*)gameId (int)$position", TCL_STATIC);
		return TCL_ERROR;
	}
	else
	{
		// position is not currently used
		if (sscanf(argv[5], POSITION_FORMAT, &position) == EOF)
			return TCL_ERROR;

		errCode = (int)GetLastOnlineMove((STRING)argv[1], (STRING)argv[2], (STRING)argv[3], (STRING)argv[4], &move, &errMsg);
		if (errCode == 0)
		{
			if (move == NULL)
			{
				Tcl_SetResult(interp, StringFormat(256, "%d:",errCode), SafeFreeString);
			}
			else
			{
				Tcl_SetResult(interp, StringFormat(256, "%d:%s",errCode,move), SafeFreeString);
				free(move);
			}
		}
		else
		{
			Tcl_SetResult(interp, StringFormat(256, "%d:%s",errCode,errMsg), SafeFreeString);
			if (errMsg != NULL)
				free(errMsg);
		}
		return TCL_OK;
	}
}

/* [C_SendLocalPlayersMove $username $password $sessionId $gameId $position $prevMove] */
static int
SendLocalPlayersMoveCmd(dummy, interp, argc, argv)
ClientData dummy;                       /* Not used. */
Tcl_Interp *interp;                     /* Current interpreter. */
int argc;                               /* Number of arguments. */
char **argv;                            /* Argument strings. */
{
	int SendNewOnlineMove();
	POSITION position;
	int errCode;
	char* errMsg;

	if (argc != 7) {
		Tcl_SetResult(interp,  "wrong # args: SendLocalPlayersMove (char*)$username (char*)$password (char*)$sessionId (char*)$gameId (int)$position (int)$prevMove", TCL_STATIC);
		return TCL_ERROR;
	}
	else
	{
		// position not currently used
		if (sscanf(argv[5], POSITION_FORMAT, &position) == EOF)
			return TCL_ERROR;

		errCode = (int)SendNewOnlineMove((STRING)argv[1], (STRING)argv[2], (STRING)argv[3], (STRING)argv[4], (STRING)argv[6], &errMsg);
		if (errCode == 0)
		{
			Tcl_SetResult(interp, StringFormat(256, "%d:",errCode), SafeFreeString);
		}
		else
		{
			Tcl_SetResult(interp, StringFormat(256, "%d:%s",errCode,errMsg), SafeFreeString);
			if (errMsg != NULL)
				free(errMsg);
		}
		return TCL_OK;
	}
}

/* [C_SendGameOver $username $password $sessionId $gameId] */
static int
SendGameOverCmd(dummy, interp, argc, argv)
ClientData dummy;                       /* Not used. */
Tcl_Interp *interp;                     /* Current interpreter. */
int argc;                               /* Number of arguments. */
char **argv;                            /* Argument strings. */
{
	int SendGameOver();
	int errCode;
	char* errMsg;

	if (argc != 5) {
		Tcl_SetResult(interp,  "wrong # args: SendGameOver (char*)$username (char*)$password (char*)$sessionId (char*)$gameId", TCL_STATIC);
		return TCL_ERROR;
	}
	else
	{
		errCode = (int)SendGameOver((STRING)argv[1], (STRING)argv[2], (STRING)argv[3], (STRING)argv[4], &errMsg);
		if (errCode == 0)
		{
			Tcl_SetResult(interp, StringFormat(256, "%d:",errCode), SafeFreeString);
		}
		else
		{
			Tcl_SetResult(interp, StringFormat(256, "%d:%s",errCode,errMsg), SafeFreeString);
			if (errMsg != NULL)
				free(errMsg);
		}
		return TCL_OK;
	}
}

/* [C_SendResign $username $password $sessionId $gameId] */
static int
SendResignCmd(dummy, interp, argc, argv)
ClientData dummy;                       /* Not used. */
Tcl_Interp *interp;                     /* Current interpreter. */
int argc;                               /* Number of arguments. */
char **argv;                            /* Argument strings. */
{
	int SendResign();
	int errCode;
	char* errMsg;

	if (argc != 5) {
		Tcl_SetResult(interp,  "wrong # args: SendResign (char*)$username (char*)$password (char*)$sessionId (char*)$gameId", TCL_STATIC);
		return TCL_ERROR;
	}
	else
	{
		errCode = (int)SendResign((STRING)argv[1], (STRING)argv[2], (STRING)argv[3], (STRING)argv[4], &errMsg);
		if (errCode == 0)
		{
			Tcl_SetResult(interp, StringFormat(256, "%d:",errCode), SafeFreeString);
		}
		else
		{
			Tcl_SetResult(interp, StringFormat(256, "%d:%s",errCode,errMsg), SafeFreeString);
			if (errMsg != NULL)
				free(errMsg);
		}
		return TCL_OK;
	}
}

/* [C_LoginUser $username $password] */
static int
LoginUserCmd(dummy, interp, argc, argv)
ClientData dummy;                       /* Not used. */
Tcl_Interp *interp;                     /* Current interpreter. */
int argc;                               /* Number of arguments. */
char **argv;                            /* Argument strings. */
{
	int LoginUser();
	int errCode;
	char* errMsg;
	char* sessionId;

	if (argc != 3)
	{
		Tcl_SetResult(interp,  "wrong # args: LoginUser (char*)$username (char*)$password", TCL_STATIC);
		return TCL_ERROR;
	}
	else
	{
		errCode = (int)LoginUser((STRING)argv[1], (STRING)argv[2], &sessionId, &errMsg);
		if (errCode == 0)
		{
			Tcl_SetResult(interp, StringFormat(256, "%d:%s",errCode,sessionId), SafeFreeString);
			if (sessionId != NULL)
				free(sessionId);
		}
		else
		{
			Tcl_SetResult(interp, StringFormat(256, "%d:%s",errCode,errMsg), SafeFreeString);
			if (errMsg != NULL)
				free(errMsg);
		}
		return TCL_OK;
	}
}

/* [C_LogoutUser $username $password $sessionId] */
static int
LogoutUserCmd(dummy, interp, argc, argv)
ClientData dummy;                       /* Not used. */
Tcl_Interp *interp;                     /* Current interpreter. */
int argc;                               /* Number of arguments. */
char **argv;                            /* Argument strings. */
{
	int LogoutUser();
	int errCode;
	char* errMsg;

	if (argc != 4)
	{
		Tcl_SetResult(interp,  "wrong # args: LogoutUser (char*)$username (char*)$password (char*)$sessionId", TCL_STATIC);
		return TCL_ERROR;
	}
	else
	{
		errCode = (int)LogoutUser((STRING)argv[1], (STRING)argv[2], (STRING)argv[3], &errMsg);
		if (errCode == 0)
		{
			Tcl_SetResult(interp, StringFormat(256, "%d:",errCode), SafeFreeString);
		}
		else
		{
			Tcl_SetResult(interp, StringFormat(256, "%d:%s",errCode,errMsg), SafeFreeString);
			if (errMsg != NULL)
				free(errMsg);
		}
		return TCL_OK;
	}
}

/* [C_RegisterUser $username $password] */
static int
RegisterUserCmd(dummy, interp, argc, argv)
ClientData dummy;                       /* Not used. */
Tcl_Interp *interp;                     /* Current interpreter. */
int argc;                               /* Number of arguments. */
char **argv;                            /* Argument strings. */
{
	int RegisterUser();
	int errCode;
	char* errMsg;

	if (argc != 3)
	{
		Tcl_SetResult(interp,  "wrong # args: RegisterUser (char*)$username (char*)$password", TCL_STATIC);
		return TCL_ERROR;
	}
	else
	{
		errCode = (int)RegisterUser((STRING)argv[1], (STRING)argv[2], &errMsg);
		if (errCode == 0)
		{
			Tcl_SetResult(interp, StringFormat(256, "%d:",errCode), SafeFreeString);
		}
		else
		{
			Tcl_SetResult(interp, StringFormat(256, "%d:%s",errCode,errMsg), SafeFreeString);
			if (errMsg != NULL)
				free(errMsg);
		}
		return TCL_OK;
	}
}

/* [C_GetUsers $username $password $sessionId] */
static int
GetUsersCmd(dummy, interp, argc, argv)
ClientData dummy;                       /* Not used. */
Tcl_Interp *interp;                     /* Current interpreter. */
int argc;                               /* Number of arguments. */
char **argv;                            /* Argument strings. */
{
	int GetUsers();
	int errCode;
	char* errMsg;
	char* users;

	if (argc != 4)
	{
		Tcl_SetResult(interp,  "wrong # args: GetUsers (char*)$username (char*)$password (char*)$sessionId", TCL_STATIC);
		return TCL_ERROR;
	}
	else
	{
		errCode = (int)GetUsers((STRING)argv[1], (STRING)argv[2], (STRING)argv[3], &users, &errMsg);
		if (errCode == 0)
		{
			if (users != NULL)
			{
				// Return the request body (users variable) that should contain user/game records.
				// Each one terminated by "\n". Where the fields in each record are delimited by ":".
				Tcl_SetResult(interp, StringFormat(256, "%d:%s",errCode,users), SafeFreeString);
				free(users);
			}
			else
			{
				Tcl_SetResult(interp, StringFormat(256, "%d:",errCode), SafeFreeString);
			}
		}
		else
		{
			Tcl_SetResult(interp, StringFormat(256, "%d:%s",errCode,errMsg), SafeFreeString);
			if (errMsg != NULL)
				free(errMsg);
		}
	}
	return TCL_OK;
}

/* [C_RegisterGame $username $password $sessionId $gamename $gamevariation $gamedesc $iMoveFirst] */
static int
RegisterGameCmd(dummy, interp, argc, argv)
ClientData dummy;                       /* Not used. */
Tcl_Interp *interp;                     /* Current interpreter. */
int argc;                               /* Number of arguments. */
char **argv;                            /* Argument strings. */
{
	int RegisterGame();
	int errCode;
	char* errMsg;
	char* gameId;

	if (argc != 8)
	{
		Tcl_SetResult(interp,  "wrong # args: RegisterGame (char*)$username (char*)$password (char*)$sessionId (char*)$gamename (char*)$gamevariation (char*)$gamedesc (char*)$iMoveFirst", TCL_STATIC);
		return TCL_ERROR;
	}
	else
	{
		errCode = (int)RegisterGame((STRING)argv[1], (STRING)argv[2], (STRING)argv[3], (STRING)argv[4], (STRING)argv[5], (STRING)argv[6], (STRING)argv[7], &gameId, &errMsg);
		if (errCode == 0)
		{
			Tcl_SetResult(interp, StringFormat(256, "%d:%s",errCode,gameId), SafeFreeString);
			if (gameId != NULL)
				free(gameId);
		}
		else
		{
			Tcl_SetResult(interp, StringFormat(256, "%d:%s",errCode,errMsg), SafeFreeString);
			if (errMsg != NULL)
				free(errMsg);
		}
		return TCL_OK;
	}
}

/* [C_JoinGame $username $password $sessionId $gameId] */
static int
JoinGameCmd(dummy, interp, argc, argv)
ClientData dummy;                       /* Not used. */
Tcl_Interp *interp;                     /* Current interpreter. */
int argc;                               /* Number of arguments. */
char **argv;                            /* Argument strings. */
{
	int JoinGame();
	int errCode;
	char* errMsg;

	if (argc != 5)
	{
		Tcl_SetResult(interp,  "wrong # args: JoinGame (char*)$username (char*)$password (char*)$sessionId (char*)$gameId", TCL_STATIC);
		return TCL_ERROR;
	}
	else
	{
		errCode = (int)JoinGame((STRING)argv[1], (STRING)argv[2], (STRING)argv[3], (STRING)argv[4], &errMsg);
		if (errCode == 0)
		{
			Tcl_SetResult(interp, StringFormat(256, "%d:",errCode), SafeFreeString);
		}
		else
		{
			Tcl_SetResult(interp, StringFormat(256, "%d:%s",errCode,errMsg), SafeFreeString);
			if (errMsg != NULL)
				free(errMsg);
		}
		return TCL_OK;
	}
}

/* [C_ReceivedChallenge $username $password $sessionId $gameId] */
static int
ReceivedChallengeCmd(dummy, interp, argc, argv)
ClientData dummy;                       /* Not used. */
Tcl_Interp *interp;                     /* Current interpreter. */
int argc;                               /* Number of arguments. */
char **argv;                            /* Argument strings. */
{
	int ReceivedChallenge();
	int errCode;
	char* errMsg;
	char* status;

	if (argc != 5)
	{
		Tcl_SetResult(interp,  "wrong # args: ReceivedChallenge (char*)$username (char*)$password (char*)$sessionId (char*)$gameId", TCL_STATIC);
		return TCL_ERROR;
	}
	else
	{
		errCode = (int)ReceivedChallenge((STRING)argv[1], (STRING)argv[2], (STRING)argv[3], (STRING)argv[4], &status, &errMsg);
		if (errCode == 0)
		{
			Tcl_SetResult(interp, StringFormat(256, "%d:%s",errCode,status), SafeFreeString);
			if (status != NULL)
				free(status);
		}
		else
		{
			Tcl_SetResult(interp, StringFormat(256, "%d:%s",errCode,errMsg), SafeFreeString);
			if (errMsg != NULL)
				free(errMsg);
		}
		return TCL_OK;
	}
}

/* [C_AcceptChallenge $username $password $sessionId $gameId $accept] */
static int
AcceptChallengeCmd(dummy, interp, argc, argv)
ClientData dummy;                       /* Not used. */
Tcl_Interp *interp;                     /* Current interpreter. */
int argc;                               /* Number of arguments. */
char **argv;                            /* Argument strings. */
{
	int AcceptChallenge();
	int errCode;
	char* errMsg;
	char* moveFirst;

	if (argc != 6)
	{
		Tcl_SetResult(interp,  "wrong # args: AcceptChallenge (char*)$username (char*)$password (char*)$sessionId (char*)$gameId (char*)$accept", TCL_STATIC);
		return TCL_ERROR;
	}
	else
	{
		errCode = (int)AcceptChallenge((STRING)argv[1], (STRING)argv[2], (STRING)argv[3], (STRING)argv[4], (STRING)argv[5], &moveFirst, &errMsg);
		if (errCode == 0)
		{
			Tcl_SetResult(interp, StringFormat(256, "%d:%s",errCode,moveFirst), SafeFreeString);
			if (moveFirst != NULL)
				free(moveFirst);
		}
		else
		{
			Tcl_SetResult(interp, StringFormat(256, "%d:%s",errCode,errMsg), SafeFreeString);
			if (errMsg != NULL)
				free(errMsg);
		}
		return TCL_OK;
	}
}

/* [C_GetGameStatus $username $password $sessionId $gameId] */
static int
GetGameStatusCmd(dummy, interp, argc, argv)
ClientData dummy;                       /* Not used. */
Tcl_Interp *interp;                     /* Current interpreter. */
int argc;                               /* Number of arguments. */
char **argv;                            /* Argument strings. */
{
	int GetGameStatus();
	int errCode;
	char* errMsg;
	char* users;

	if (argc != 5)
	{
		Tcl_SetResult(interp,  "wrong # args: GetGameStatus (char*)$username (char*)$password (char*)$sessionId (char*)$gameId", TCL_STATIC);
		return TCL_ERROR;
	}
	else
	{
		errCode = (int)GetGameStatus((STRING)argv[1], (STRING)argv[2], (STRING)argv[3], (STRING)argv[4], &users, &errMsg);
		if (errCode == 0)
		{
			if (users != NULL)
			{
				// Return the request body (users variable) that should contain user records.
				// Each one terminated by "\n". Where the fields in each record are delimited by ":".
				Tcl_SetResult(interp, StringFormat(256, "%d:%s",errCode,users), SafeFreeString);
				free(users);
			}
			else
			{
				Tcl_SetResult(interp, StringFormat(256, "%d:",errCode), SafeFreeString);
			}
		}
		else
		{
			Tcl_SetResult(interp, StringFormat(256, "%d:%s",errCode,errMsg), SafeFreeString);
			if (errMsg != NULL)
				free(errMsg);
		}
		return TCL_OK;
	}
}

/* [C_AcceptedChallenge $username $password $sessionId $gameId] */
static int
AcceptedChallengeCmd(dummy, interp, argc, argv)
ClientData dummy;                       /* Not used. */
Tcl_Interp *interp;                     /* Current interpreter. */
int argc;                               /* Number of arguments. */
char **argv;                            /* Argument strings. */
{
	int AcceptedChallenge();
	int errCode;
	char* errMsg;
	char* moveFirst;
	char* status;

	if (argc != 5)
	{
		Tcl_SetResult(interp,  "wrong # args: AcceptedChallenge (char*)$username (char*)$password (char*)$sessionId (char*)$gameId", TCL_STATIC);
		return TCL_ERROR;
	}
	else
	{
		errCode = (int)AcceptedChallenge((STRING)argv[1], (STRING)argv[2], (STRING)argv[3], (STRING)argv[4], &status, &moveFirst, &errMsg);
		if (errCode == 0)
		{
			if (moveFirst != NULL)
			{
				Tcl_SetResult(interp, StringFormat(256, "%d:%s:%s",errCode,status,moveFirst), SafeFreeString);
				free(moveFirst);
			}
			else
			{
				Tcl_SetResult(interp, StringFormat(256, "%d:%s:",errCode,status), SafeFreeString);
			}
			if (status != NULL)
				free(status);
		}
		else
		{
			Tcl_SetResult(interp, StringFormat(256, "%d:%s",errCode,errMsg), SafeFreeString);
			if (errMsg != NULL)
				free(errMsg);
		}
		return TCL_OK;
	}
}

/* [C_UnregisterGame $username $password $sessionId $gameId] */
static int
UnregisterGameCmd(dummy, interp, argc, argv)
ClientData dummy;                       /* Not used. */
Tcl_Interp *interp;                     /* Current interpreter. */
int argc;                               /* Number of arguments. */
char **argv;                            /* Argument strings. */
{
	int UnregisterGame();
	int errCode;
	char* errMsg;
	char* challenger;

	if (argc != 5)
	{
		Tcl_SetResult(interp,  "wrong # args: UnregisterGame (char*)$username (char*)$password (char*)$sessionId (char*)$gameId", TCL_STATIC);
		return TCL_ERROR;
	}
	else
	{
		errCode = (int)UnregisterGame((STRING)argv[1], (STRING)argv[2], (STRING)argv[3], (STRING)argv[4], &challenger, &errMsg);
		if (errCode == 0)
		{
			Tcl_SetResult(interp, StringFormat(256, "%d:",errCode), SafeFreeString);
		}
		else if (errCode == 312)
		{
			Tcl_SetResult(interp, StringFormat(256, "%d:%s",errCode,challenger), SafeFreeString);
			if (challenger != NULL)
				free(challenger);
		}
		else
		{
			Tcl_SetResult(interp, StringFormat(256, "%d:%s",errCode,errMsg), SafeFreeString);
			if (errMsg != NULL)
				free(errMsg);
		}
		return TCL_OK;
	}
}

/* [C_DeselectChallenger $username $password $sessionId $gameId] */
static int
DeselectChallengerCmd(dummy, interp, argc, argv)
ClientData dummy;                       /* Not used. */
Tcl_Interp *interp;                     /* Current interpreter. */
int argc;                               /* Number of arguments. */
char **argv;                            /* Argument strings. */
{
	int DeselectChallenger();
	int errCode;
	char* errMsg;
	char* challenger;

	if (argc != 5)
	{
		Tcl_SetResult(interp,  "wrong # args: DeselectChallenger (char*)$username (char*)$password (char*)$sessionId (char*)$gameId", TCL_STATIC);
		return TCL_ERROR;
	}
	else
	{
		errCode = (int)DeselectChallenger((STRING)argv[1], (STRING)argv[2], (STRING)argv[3], (STRING)argv[4], &challenger, &errMsg);
		if (errCode == 0)
		{
			Tcl_SetResult(interp, StringFormat(256, "%d:",errCode), SafeFreeString);
		}
		else if (errCode == 312)
		{
			Tcl_SetResult(interp, StringFormat(256, "%d:%s",errCode,challenger), SafeFreeString);
			if (challenger != NULL)
				free(challenger);
		}
		else
		{
			Tcl_SetResult(interp, StringFormat(256, "%d:%s",errCode,errMsg), SafeFreeString);
			if (errMsg != NULL)
				free(errMsg);
		}
		return TCL_OK;
	}
}

/* [C_SelectChallenger $username $password $sessionId $gameId $selChallenger] */
static int
SelectChallengerCmd(dummy, interp, argc, argv)
ClientData dummy;                       /* Not used. */
Tcl_Interp *interp;                     /* Current interpreter. */
int argc;                               /* Number of arguments. */
char **argv;                            /* Argument strings. */
{
	int SelectChallenger();
	int errCode;
	char* errMsg;
	char* challenger;

	if (argc != 6)
	{
		Tcl_SetResult(interp,  "wrong # args: SelectChallenger (char*)$username (char*)$password (char*)$sessionId (char*)$gameId (char*)$selChallenger", TCL_STATIC);
		return TCL_ERROR;
	}
	else
	{
		errCode = (int)SelectChallenger((STRING)argv[1], (STRING)argv[2], (STRING)argv[3], (STRING)argv[4], (STRING)argv[5], &challenger, &errMsg);
		if (errCode == 0)
		{
			Tcl_SetResult(interp, StringFormat(256, "%d:",errCode), SafeFreeString);
		}
		else if (errCode == 312)
		{
			Tcl_SetResult(interp, StringFormat(256, "%d:%s",errCode,challenger), SafeFreeString);
			if (challenger != NULL)
				free(challenger);
		}
		else
		{
			Tcl_SetResult(interp, StringFormat(256, "%d:%s",errCode,errMsg), SafeFreeString);
			if (errMsg != NULL)
				free(errMsg);
		}
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

	gTclInterp = interp;
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
	gTclInterp = interp;
	char script[50] = "advanceProgressBar 0";

	if (argc != 2) {
		Tcl_SetResult(interp,  "wrong # args: DetermineValue (POSITION)Position", TCL_STATIC);
		return TCL_ERROR;
	}
	else {
		if (sscanf(argv[1], POSITION_FORMAT, &position) == EOF)
			return TCL_ERROR;

		Tcl_Eval(interp, script);

		gMenuMode = BeforeEvaluation; /* Some solvers use this for optimization */
		Tcl_SetResult(interp,  gValueString[(int)DetermineValue(position)], TCL_STATIC);
		gMenuMode = Evaluated;

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

/* [C_Remoteness $position] */
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
		Tcl_SetResult(interp, StrFromI(Remoteness(position)), SafeFreeString);
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
	void MexFormat();

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

/* [C_DoMove $position $move] */
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

		Tcl_SetResult(interp, StrFromI(DoMove(position,move)), SafeFreeString);
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

		Tcl_SetResult(interp, StrFromI(gGoAgain(position,move)), SafeFreeString);
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

/* [C_GetValueMoves $position $gameSolved]
 * returns {move value remoteness delta_remoteness}
 *
 * Caveat: C_GetValueMoves is the only way Tcl has to get moves.
 *  This means that C_GetValueMoves will be called regardless of whether
 *  the game is actually solved or not.
 *  Therefore, any piece of code required by GetValueMovesCmd can not
 *  have any type of error side effect due to an unsolved game.
 */
static int
GetValueMovesCmd(dummy, interp, argc, argv)
ClientData dummy;                       /* Not used. */
Tcl_Interp *interp;                     /* Current interpreter. */
int argc;                               /* Number of arguments. */
char **argv;                            /* Argument strings. */
{
	POSITION position;
	BOOLEAN solved = TRUE;
	MOVELIST *ptr, *head;
	VALUE value, j_value, m_value;
	char theAnswer[10000], tmp[1000];
	VALUE_MOVES *vMoves;
	REMOTENESS remote, delta;
	if (argc != 3) {
		Tcl_SetResult(interp,  "wrong # args: GetValueMoves (int)Position", TCL_STATIC);
		return TCL_ERROR;
	}
	else {
		if (sscanf(argv[1], POSITION_FORMAT, &position) == EOF)
			return TCL_ERROR;
		if (0 == strcmp("true", argv[2])) {
			solved = FALSE;
		}
		head = ptr = GenerateMoves(position);
		theAnswer[0] = '\0';
		while (ptr != NULL) {
			POSITION temp = DoMove(position,ptr->move);
			if (solved) {
				value = GetValueOfPosition(temp);
				//value = GetValueOfPosition(DoMove(position,ptr->move));
				remote = Remoteness(temp);
				vMoves = GetValueMoves(position);

				//Flip values since moving to a winning position is a losing move
				if (value == lose) {
					j_value = WINMOVE;
					m_value = win;
				} else if (value == tie) {
					j_value = TIEMOVE;
					m_value = tie;
				} else if (value == win) {
					j_value = LOSEMOVE;
					m_value = lose;
				} else {
					j_value = -1;
					m_value = -1;
				}
				delta = FindDelta(remote, vMoves->remotenessList[j_value], m_value);
				if (gGoAgain(position,ptr->move)) {
					switch(value) {
					case win: value = lose; break;
					case lose: value = win; break;
					default: value = value;
					}
				}
			} else {
				value = lose;
				remote = 0;
				delta = 0;
			}

			/*
			   sprintf(tmp,"{ %d %s } ",
			        (ptr->move),
			        gValueString[value]);
			 */

			/* save  move, value, remoteness, delta_remoteness */
			sprintf(tmp,"{ %d %s %d %d } ",
			        (ptr->move),
			        gValueString[value],
			        (int) remote,
			        (int) delta);

			/*
			   printf("Move: %d, Value: %s, Remoteness: %d, Delta: %d \n",
			       (ptr->move),
			       gValueString[value],
			       (int)remote,
			       (int)delta);
			 */

			strcat(theAnswer, (const char*) tmp);
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

static int
percentDoneCmd(dummy, interp, argc, argv)
ClientData dummy;               /* Not used. */
Tcl_Interp *interp;             /* Current interpreter. */
int argc;                   /* Number of arguments. */
char **argv;                /* Argument strings. */
{
	if (argc != 1) {
		Tcl_SetResult(interp,  "wrong # args: PercentDone", TCL_STATIC);
		return TCL_ERROR;
	}
	else {
		Tcl_SetResult(interp, StringFormat(256, "%f3.0",PercentDone(2)), SafeFreeString);
		return TCL_OK;
	}
}

static int
GetStandardObjStringCmd(dummy, interp, argc, argv)
ClientData dummy;                       /* Not used. */
Tcl_Interp *interp;                     /* Current interpreter. */
int argc;                               /* Number of arguments. */
char **argv;                            /* Argument strings. */
{

	if (argc != 1) {
		Tcl_SetResult(interp, "wrong # args: shouldn't be any", TCL_STATIC);
		return TCL_ERROR;
	}

	else {
		Tcl_SetResult(interp,  kHelpStandardObjective, TCL_STATIC);
		return TCL_OK;
	}
}

static int
GetReverseObjStringCmd(dummy, interp, argc, argv)
ClientData dummy;                       /* Not used. */
Tcl_Interp *interp;                     /* Current interpreter. */
int argc;                               /* Number of arguments. */
char **argv;                            /* Argument strings. */
{

	if (argc != 1) {
		Tcl_SetResult(interp, "wrong # args: shouldn't be any", TCL_STATIC);
		return TCL_ERROR;
	}

	else {
		Tcl_SetResult(interp,  kHelpReverseObjective, TCL_STATIC);
		return TCL_OK;
	}
}

static int
moveToStringCmd(dummy, interp, argc, argv)
ClientData dummy;               /* Not used. */
Tcl_Interp *interp;             /* Current interpreter. */
int argc;                   /* Number of arguments. */
char **argv;                /* Argument strings. */
{
	if (argc != 2) {
		Tcl_SetResult(interp,  "wrong # args: MoveToString", TCL_STATIC);
		return TCL_ERROR;
	}
	else {
		MOVE theMove;
		if(Tcl_GetInt(interp, argv[1], &theMove) != TCL_OK)
			return TCL_ERROR;
		if (gMoveToStringFunPtr == NULL) {
			Tcl_SetResult(interp, StrFromI(theMove), SafeFreeString);
		} else {
			Tcl_SetResult(interp, StringDup(gMoveToStringFunPtr(theMove)), SafeFreeString);
		}
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

// TIER FUN!
static int
UsingTiersCmd(dummy, interp, argc, argv)
ClientData dummy;               /* Not used. */
Tcl_Interp *interp;             /* Current interpreter. */
int argc;                   /* Number of arguments. */
char **argv;                /* Argument strings. */
{
	if (argc != 1) {
		Tcl_SetResult(interp,  "wrong # args: UsingTiers", TCL_STATIC);
		return TCL_ERROR;
	}
	else {
		if (gHashWindowInitialized) // TIER GAMESMAN
			Tcl_SetResult(interp, StrFromI(1), SafeFreeString);
		else
			Tcl_SetResult(interp, StrFromI(0), SafeFreeString);
		return TCL_OK;
	}
}

static int
InitHashWindowCmd(dummy, interp, argc, argv)
ClientData dummy;               /* Not used. */
Tcl_Interp *interp;             /* Current interpreter. */
int argc;                   /* Number of arguments. */
char **argv;                /* Argument strings. */
{
	POSITION position;

	if (argc != 2) {
		Tcl_SetResult(interp,  "wrong # args: InitHashWindow (int)Position", TCL_STATIC);
		return TCL_ERROR;
	}
	else {
		if (sscanf(argv[1], POSITION_FORMAT, &position) == EOF)
			return TCL_ERROR;

		if (gHashWindowInitialized) { // TIER GAMESMAN
			gInitializeHashWindow(gInitialTier, TRUE);
			position = gHashToWindowPosition(gInitialTierPosition, gInitialTier);
		} // else, just spit the argument back (for non-tier games)

		Tcl_SetResult(interp, StrFromI(position), SafeFreeString);
		return TCL_OK;
	}
}

static int
HashWindowCmd(dummy, interp, argc, argv)
ClientData dummy;               /* Not used. */
Tcl_Interp *interp;             /* Current interpreter. */
int argc;                   /* Number of arguments. */
char **argv;                /* Argument strings. */
{
	POSITION position;

	if (argc != 2) {
		Tcl_SetResult(interp,  "wrong # args: HashWindow (int)Position", TCL_STATIC);
		return TCL_ERROR;
	}
	else {
		if (sscanf(argv[1], POSITION_FORMAT, &position) == EOF)
			return TCL_ERROR;

		if (gHashWindowInitialized)
			gInitializeHashWindowToPosition(&position, TRUE);

		Tcl_SetResult(interp, StrFromI(position), SafeFreeString);
		return TCL_OK;
	}
}

static int
HashWindowUndoCmd(dummy, interp, argc, argv)
ClientData dummy;               /* Not used. */
Tcl_Interp *interp;             /* Current interpreter. */
int argc;                   /* Number of arguments. */
char **argv;                /* Argument strings. */
{
	TIER tier;

	if (argc != 2) {
		Tcl_SetResult(interp,  "wrong # args: HashWindowUndo (int)Position", TCL_STATIC);
		return TCL_ERROR;
	}
	else {
		if (sscanf(argv[1], POSITION_FORMAT, &tier) == EOF)
			return TCL_ERROR;

		if (gHashWindowInitialized)
			gInitializeHashWindow(tier, TRUE);

		Tcl_SetResult(interp, StrFromI(1), SafeFreeString);
		return TCL_OK;
	}
}

static int
CurrentTierCmd(dummy, interp, argc, argv)
ClientData dummy;               /* Not used. */
Tcl_Interp *interp;             /* Current interpreter. */
int argc;                   /* Number of arguments. */
char **argv;                /* Argument strings. */
{
	if (argc != 1) {
		Tcl_SetResult(interp,  "wrong # args: UsingTiers", TCL_STATIC);
		return TCL_ERROR;
	}
	else {
		if (gHashWindowInitialized) // TIER GAMESMAN
			Tcl_SetResult(interp, StrFromI(gCurrentTier), SafeFreeString);
		else
			Tcl_SetResult(interp, StrFromI(-1), SafeFreeString);
		return TCL_OK;
	}
}

static int
InitTierGamesmanIfNeededCmd(dummy, interp, argc, argv)
ClientData dummy;               /* Not used. */
Tcl_Interp *interp;             /* Current interpreter. */
int argc;                   /* Number of arguments. */
char **argv;                /* Argument strings. */
{
	POSITION position;

	if (argc != 2) {
		Tcl_SetResult(interp,  "wrong # args: UsingTiers", TCL_STATIC);
		return TCL_ERROR;
	}
	else {
		if (sscanf(argv[1], POSITION_FORMAT, &position) == EOF)
			return TCL_ERROR;

		if (kSupportsTierGamesman && gTierGamesman) // TIER GAMESMAN
			Tcl_SetResult(interp, StrFromI(InitTierGamesman()), SafeFreeString);
		else // no tiers, just return the position directly for non-tier games
			Tcl_SetResult(interp, StrFromI(position), SafeFreeString);
		return TCL_OK;
	}
}
