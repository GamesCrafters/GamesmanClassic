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
** LAST CHANGE: $Id: tkAppInit.c,v 1.35 2007-02-21 18:49:38 scarr2508 Exp $
**
**************************************************************************/

#include "tk.h"
#include "gamesman.h"
#include "hash.h"
#include <string.h>
#include "core/analysis.h"
#include "core/gameplay.h"

extern STRING gValueString[]; /* The GAMESMAN Value strings */
extern BOOLEAN gStandardGame;
extern BOOLEAN kPartizan;
extern POSITION gInitialPosition;
extern int smartness;
extern int scalelvl;
extern MENU gMenuMode;

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


static int		InitialPositionCmd _ANSI_ARGS_((ClientData clientData,
			    Tcl_Interp *interp, int argc, char **argv));

static int		GenericUnhashCmd _ANSI_ARGS_((ClientData clientData,
			    Tcl_Interp *interp, int argc, char **argv));
static int		CustomUnhashCmd _ANSI_ARGS_((ClientData clientData,
			    Tcl_Interp *interp, int argc, char **argv));

static int		FooCmd _ANSI_ARGS_((ClientData clientData,
			    Tcl_Interp *interp, int argc, char **argv));
static int		InitializeCmd _ANSI_ARGS_((ClientData clientData,
			    Tcl_Interp *interp, int argc, char **argv));
static int		InitializeDatabasesCmd _ANSI_ARGS_((ClientData clientData,
			    Tcl_Interp *interp, int argc, char **argv));
static int              InitializeGameCmd _ANSI_ARGS_((ClientData clientData,
			    Tcl_Interp *interp, int argc, char **argv));
static int		GetComputersMoveCmd _ANSI_ARGS_((ClientData clientData,
			    Tcl_Interp *interp, int argc, char **argv));
static int		SetGameSpecificOptionsCmd _ANSI_ARGS_((ClientData clientData,
			    Tcl_Interp *interp, int argc, char **argv));
static int		DetermineValueCmd _ANSI_ARGS_((ClientData clientData,
			    Tcl_Interp *interp, int argc, char **argv));
static int		GetValueOfPositionCmd _ANSI_ARGS_((ClientData clientData,
			    Tcl_Interp *interp, int argc, char **argv));
static int		RemotenessCmd _ANSI_ARGS_((ClientData clientData,
			    Tcl_Interp *interp, int argc, char **argv));
static int		MexCmd _ANSI_ARGS_((ClientData clientData,
			    Tcl_Interp *interp, int argc, char **argv));
static int		DoMoveCmd _ANSI_ARGS_((ClientData clientData,
			    Tcl_Interp *interp, int argc, char **argv));
static int		PrimitiveCmd _ANSI_ARGS_((ClientData clientData,
			    Tcl_Interp *interp, int argc, char **argv));
static int		GetValueMovesCmd _ANSI_ARGS_((ClientData clientData,
			    Tcl_Interp *interp, int argc, char **argv));
static int		RandomCmd _ANSI_ARGS_((ClientData clientData,
			    Tcl_Interp *interp, int argc, char **argv));
 #ifdef COMPUTEC
static int		ComputeCCmd _ANSI_ARGS_((ClientData clientData,
			    Tcl_Interp *interp, int argc, char **argv));
 #endif
static int		GoAgainCmd _ANSI_ARGS_((ClientData clientData,
			    Tcl_Interp *interp, int argc, char **argv));
static int		GetPredictionCmd _ANSI_ARGS_((ClientData clientData,
			    Tcl_Interp *interp, int argc, char **argv));
static int              SetSmarterComputerCmd _ANSI_ARGS_((ClientData clientData,
			    Tcl_Interp *interp, int argc, char **argv));
static int		GetOptionCmd _ANSI_ARGS_((ClientData clientData,
			    Tcl_Interp *interp, int argc, char **argv));
static int		SetOptionCmd _ANSI_ARGS_((ClientData clientData,
			    Tcl_Interp *interp, int argc, char **argv));

static int      percentDoneCmd _ANSI_ARGS_((ClientData clientData,
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
    Tcl_Interp *interp;		/* Interpreter for application. */
{
    Tk_Window mainWindow;

    mainWindow = Tk_MainWindow(interp);

    Tcl_CreateCommand(interp, "C_InitialPosition", (Tcl_CmdProc*) InitialPositionCmd, (ClientData) mainWindow,
		      (Tcl_CmdDeleteProc*) NULL);

    Tcl_CreateCommand(interp, "C_GenericUnhash", (Tcl_CmdProc*) GenericUnhashCmd, (ClientData) mainWindow,
		      (Tcl_CmdDeleteProc*) NULL);
    Tcl_CreateCommand(interp, "C_CustomUnhash", (Tcl_CmdProc*) CustomUnhashCmd, (ClientData) mainWindow,
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

    Tcl_CreateCommand(interp, "C_PercentDone", (Tcl_CmdProc*) percentDoneCmd, (ClientData) mainWindow,
              (Tcl_CmdDeleteProc*) NULL);


    {
    int (*fptr)(Tcl_Interp *interp,Tk_Window) = (int(*)(Tcl_Interp*, Tk_Window))gGameSpecificTclInit;

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
    interp->result ="wrong # args: shouldn't be any";
    return TCL_ERROR;
  }
  // Ported from tkAppInitHash, correct version tbd
  sprintf(interp->result,"%d",(int)gInitialPosition);
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
    interp->result = "wrong # args: should be 1 (or 2 for backwards compat)";
    return TCL_ERROR;
  }
  // Ported from tkAppInitHash, correct version tbd
  char *board;
  POSITION pos = atoi(argv[1]);
  board = generic_hash_unhash_tcl(pos);
  sprintf(interp->result, "%s",board);
  SafeFree(board);
  return TCL_OK;
  /*
  char *board;
  int n;
  board = (char *) SafeMalloc (sizeof(char)*(atoi(argv[2])));
  generic_hash_unhash(atoi(argv[1]),board);
  sprintf(interp->result, "%s",board);
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
    interp->result = "wrong # args: should be 1 (or 2 for backwards compat)";
    return TCL_ERROR;
  } else if (gCustomUnhash == NULL) {
    interp->result = "CustomUnhash is not defined for this game";
    return TCL_ERROR;
  }
  // Ported from tkAppInitHash, correct version tbd
  char *board;
  POSITION pos = atoi(argv[1]);
  board = gCustomUnhash(pos);
  sprintf(interp->result, "%s",board);
  SafeFree(board);
  return TCL_OK;
 }


static int
FooCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    if (argc != 4) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" x y z\"", (char *) NULL);
	return TCL_ERROR;
    }
    else {
	interp->result = "Dan Garcia";
	return TCL_OK;
    }
}

static int
InitializeCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
  char **ap, *args[20];
  int numArgs = 0;
  if (argc != 2) {
    interp->result = "wrong # args: shouldn't have any args";
    return TCL_ERROR;
  }
  else {
    for (ap = args; (*ap = strsep(&argv[1], " \t")) != NULL;)
      if (**ap != '\0')
	if (++ap >= &args[20])
	  break;
	else
	  numArgs++;
    
    HandleArguments(numArgs, args);
    Initialize();
    interp->result = "System Initialized";
    return TCL_OK;
  }
}

static int
InitializeGameCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
  if (argc != 1) {
    interp->result = "wrong # args: shouldn't have any args";
    return TCL_ERROR;
  }
  else {
    InitializeGame();
    interp->result = "Game Initialized";
    return TCL_OK;
  }
}

static int
InitializeDatabasesCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
  if (argc != 1) {
    interp->result = "wrong # args: shouldn't have any args";
    return TCL_ERROR;
  }
  else {
    InitializeDatabases();
    interp->result = "Databases Initialized";
    return TCL_OK;
  }
}

static int
GetComputersMoveCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
  MOVE GetComputersMove();
  POSITION position;

  if (argc != 2) {
    interp->result = "wrong # args: GetComputerMove (int)Position";
    return TCL_ERROR;
  }
  else {
    if (sscanf(argv[1], POSITION_FORMAT, &position) == EOF)
      return TCL_ERROR;

    sprintf(interp->result,"%d",(int)GetComputersMove(position));
    return TCL_OK;
  }
}

static int
SetGameSpecificOptionsCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
  int i, theOptions[100];
  int standardGame = 0;

  if (argc < 2) {
    interp->result = "wrong # args: SetGameSpecificOptions (boolean)Standard-Game-p (optional)Other-Game-Specific-Options";
    return TCL_ERROR;
  }

  if (argc > 102) {
    interp->result = "too many arguments: SetGameSpecificOptions (boolean)Standard-Game-p [, (boolean)option_n]*";
    return TCL_ERROR;
  }

  if(Tcl_GetInt(interp, argv[1], &standardGame) != TCL_OK)
    return TCL_ERROR;

  if (argc > 2)
    for(i=2;i<argc;i++)
      if(Tcl_GetInt(interp, argv[i], &theOptions[i-2]) != TCL_OK)
	return TCL_ERROR;

  gStandardGame = standardGame;
  SetTclCGameSpecificOptions(theOptions);

  return TCL_OK;
}

static int
DetermineValueCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
  POSITION position;
  VALUE DetermineValue(), DetermineLoopyValue();
  gTclInterp = interp;
  char script[50] = "advanceProgressBar 0";

  if (argc != 2) {
    interp->result = "wrong # args: DetermineValue (POSITION)Position";
    return TCL_ERROR;
  }
  else {
    if (sscanf(argv[1], POSITION_FORMAT, &position) == EOF)
      return TCL_ERROR;

    Tcl_Eval(interp, script);

    gMenuMode = BeforeEvaluation; /* Some solvers use this for optimization */
    interp->result = gValueString[(int)DetermineValue(position)];
    gMenuMode = Evaluated;


    return TCL_OK;
  }
}

static int
GetValueOfPositionCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
  POSITION position;
  VALUE GetValueOfPosition();

  if (argc != 2) {
    interp->result = "wrong # args: GetValueOfPosition (POSITION)Position";
    return TCL_ERROR;
  }
  else {
    if (sscanf(argv[1], POSITION_FORMAT, &position) == EOF)
      return TCL_ERROR;

    interp->result = gValueString[(int)GetValueOfPosition(position)];
    return TCL_OK;
  }
}

/* [C_Remoteness $position] */
static int
RemotenessCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
  POSITION position;
  REMOTENESS Remoteness();

  if (argc != 2) {
    interp->result = "wrong # args: Remoteness (POSITION)Position";
    return TCL_ERROR;
  }
  else {
    if (sscanf(argv[1], POSITION_FORMAT, &position) == EOF)
      return TCL_ERROR;

    sprintf(interp->result,"%d",(int)Remoteness(position));
    return TCL_OK;
  }
}

static int
MexCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
  POSITION position;
  REMOTENESS Remoteness();
  void MexFormat();

  if (argc != 2) {
    interp->result = "wrong # args: Mex (POSITION)Position";
    return TCL_ERROR;
  }
  else {
    if (sscanf(argv[1], POSITION_FORMAT, &position) == EOF)
      return TCL_ERROR;

    if(!kPartizan)
      MexFormat(position,interp->result);
    else
      sprintf(interp->result," ");
    return TCL_OK;
  }
}

/* [C_DoMove $position $move] */
static int
DoMoveCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
  POSITION position;
  MOVE move;
  POSITION DoMove();

  if (argc != 3) {
    interp->result = "wrong # args: DoMove (int)Position (int)Move";
    return TCL_ERROR;
  }
  else {
    if (sscanf(argv[1], POSITION_FORMAT, &position) == EOF)
      return TCL_ERROR;
    if(Tcl_GetInt(interp, argv[2], &move) != TCL_OK)
      return TCL_ERROR;

    sprintf(interp->result,"%d",(int)DoMove(position,move));
    return TCL_OK;
  }
}

static int
GoAgainCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
  POSITION position;
  MOVE move;
  extern BOOLEAN (*gGoAgain)(POSITION,MOVE);

  if (argc != 3) {
    interp->result = "wrong # args: GoAgain (int)Position (int)Move";
    return TCL_ERROR;
  }
  else {
    if (sscanf(argv[1], POSITION_FORMAT, &position) == EOF)
      return TCL_ERROR;
    if(Tcl_GetInt(interp, argv[2], &move) != TCL_OK)
      return TCL_ERROR;

    sprintf(interp->result,"%d",(int)gGoAgain(position,move));
    return TCL_OK;
  }
}

static int
PrimitiveCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
  POSITION position;
  VALUE Primitive();

  if (argc != 2) {
    interp->result = "wrong # args: Primitive (int)Position";
    return TCL_ERROR;
  }
  else {
    if (sscanf(argv[1], POSITION_FORMAT, &position) == EOF)
      return TCL_ERROR;

    interp->result = gValueString[(int)Primitive(position)];
    return TCL_OK;
  }
}

/* [C_GetValueMoves $position]
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
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
  POSITION position;
  MOVELIST *ptr, *head;
  VALUE value, j_value, m_value;
  char theAnswer[10000], tmp[1000];
  VALUE_MOVES *vMoves;
  REMOTENESS remote, delta;
  if (argc != 2) {
    interp->result = "wrong # args: GetValueMoves (int)Position";
    return TCL_ERROR;
  }
  else {
    if (sscanf(argv[1], POSITION_FORMAT, &position) == EOF)
      return TCL_ERROR;
    head = ptr = GenerateMoves(position);
    theAnswer[0] = '\0';
    while (ptr != NULL) {
      POSITION temp = DoMove(position,ptr->move);
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
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
  int GetRandomNumber();
  int n;

  if (argc != 2) {
    interp->result = "wrong # args: Random (int)n";
    return TCL_ERROR;
  }
  else {
    if(Tcl_GetInt(interp, argv[1], &n) != TCL_OK)
      return TCL_ERROR;

    sprintf(interp->result,"%d",GetRandomNumber(n));
    return TCL_OK;
  }
}

static int
GetPredictionCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
  POSITION position;
  STRING playerName, prediction;

  if (argc != 3) {
    interp->result = "wrong # args: GetPrediction position playerName";
    return TCL_ERROR;
  }

  else {
    if (sscanf(argv[1], POSITION_FORMAT, &position) == EOF)
      return TCL_ERROR;
    playerName = (STRING) argv[2];

    prediction =  GetPrediction(position, playerName, TRUE);
    sprintf(interp->result,"%s",prediction);
    return TCL_OK;
  }
}

static int
SetSmarterComputerCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
  int smartnessScale;
  STRING smartnessString;

  if (argc > 3 || argc < 2) {
    interp->result = "wrong # args: SetSmarterComputer smartness [scale]";
    return TCL_ERROR;
  }

  else {
    smartnessString = (STRING) argv[1];
    sprintf(interp->result, "Setting smarter computer to %s\n", smartnessString);

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
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
  int option;

  if (argc != 2) {
    interp->result = "wrong # args: SetOption (int)option";
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
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
  int option;

  if (argc != 1) {
    interp->result = "wrong # args: GetOption";
    return TCL_ERROR;
  }
  else {

    option = getOption();
    sprintf(interp->result,"%d",option);

    return TCL_OK;
  }
}

static int
percentDoneCmd(dummy, interp, argc, argv)
    ClientData dummy;           /* Not used. */
    Tcl_Interp *interp;         /* Current interpreter. */
    int argc;               /* Number of arguments. */
    char **argv;            /* Argument strings. */
{
  if (argc != 1) {
    interp->result = "wrong # args: PercentDone";
    return TCL_ERROR;
  }
  else {
    sprintf(interp->result,"%f3.0",PercentDone(2));
    return TCL_OK;
  }
}


 #ifdef COMPUTEC
static int
ComputeCCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
  int ComputeC();
  int n,k;

  if (argc != 3) {
    interp->result = "wrong # args: Random (int)n";
    return TCL_ERROR;
  }
  else {
    if(Tcl_GetInt(interp, argv[1], &n) != TCL_OK)
      return TCL_ERROR;
    if(Tcl_GetInt(interp, argv[2], &k) != TCL_OK)
      return TCL_ERROR;

    sprintf(interp->result,"%d",ComputeC(n,k));
    return TCL_OK;
  }
}
 #endif



