/************************************************************************
**
** NAME:	textui.c
**
** DESCRIPTION:	Contains menus and other functions to get user input
**		and determine what to do given an input.
**
** AUTHOR:	GamesCrafters Research Group, UC Berkeley
**		Supervised by Dan Garcia <ddgarcia@cs.berkeley.edu>
**
** DATE:	2005-01-11
**
** LICENSE:	This file is part of GAMESMAN,
**		The Finite, Two-person Perfect-Information Game Generator
**		Released under the GPL:
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program, in COPYING; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
**************************************************************************/

#include "gamesman.h"
#include <time.h>
#include "solveretrograde.h" // for InitTierGamesman()
#include "openPositions.h"

#define DEFAULTLENGTH MAXINPUTLENGTH

extern int SetupNetworkGame(STRING);
/*
** Local function prototypes
*/

static void     MenusBeforeEvaluation           (void);
static void     MenusEvaluated                  (void);
static void     ParseMenuChoice                 (char c);
static BOOLEAN  ParseConstantMenuChoice         (char c);
static void     ParseBeforeEvaluationMenuChoice (char c);
static void     ParseEvaluatedMenuChoice        (char c);
static void     ParseHelpMenuChoice             (char c);
static void     AnalysisMenu                    (void);
static void     GamePrintMenu                   (POSITION thePosition, STRING playerName, BOOLEAN usersTurn, char input);
static void VisualizationMenu(void);



/*
** Code
*/

void HitAnyKeyToContinue()
{
/*
    static BOOLEAN first = TRUE;*/

	printf("\n\t----- Hit <return> to continue -----");
/*  first ? (first = FALSE) : getchar();  to make lint happy
    while(getchar() != '\n');
 */
	GetMyChar();
}

/*
        Functions to read input from stdin
        After calling, out will contain data of format type
        also input buffer will be flushed
        fflush(stdin) is not defined for input streams


 */
void GetMy(char *format, GENERIC_PTR out, int length, BOOLEAN keepSpaces) {
	char buffer[length];
	int blen = 0;

	fgets(buffer, length, stdin);
	blen = strlen(buffer);

	if (keepSpaces) {
		if (blen > 0 && buffer[blen - 1] == '\n') {
			buffer[blen - 1] = '\0';
			blen--;
		}

		memcpy(out, buffer, blen + 1);
	}
	else {
		sscanf(buffer, format, out);
	}

	while (blen == length - 1) {
		/* the buffer is full, possible there is extra input
		 *  so we need to clear that out
		 */
		ungetc(buffer[length - 2],stdin); /* unget the last non null                                                         character, handles case
		                                     when we have read exactly
		                                     size - 1;
		                                   */
		fgets(buffer,length, stdin);      /* read stdin again
		                                     repeat until stdin
		                                     is empty
		                                   */
		blen = strlen(buffer);
	}
}

void GetMyHelper(char *format, GENERIC_PTR out) {
	GetMy(format, out, DEFAULTLENGTH, FALSE);
}

unsigned int GetMyUInt() {
	unsigned int temp = 0;
	GetMyHelper("%u", &temp);
	return temp;
}

/* get next int from stdin and return */
int GetMyInt() {
	int temp = 0;
	GetMyHelper("%d", &temp);
	return temp;
}

/* get next char from stdin and return */
char GetMyChar() {
	char temp = '\0';
	GetMyHelper("%c", &temp);
	return temp;
}

void GetMyStr(STRING str, int len) {
	if (len < 0) {
		fprintf(stderr, "Error in GetMyStr(), expected nonnegative \
				length but got %d\n", len);
	}
	GetMy("%s", str, len, TRUE);
}

/*char GetMyChar()
   {
    char inString[MAXINPUTLENGTH], ans = '\0';
        fgets(inString, MAXINPUTLENGTH, stdin);
        sscanf(inString, "%c", &ans);
    return ans;
   }*/


void Menus(STRING executableName)
{
	printf(kOpeningCredits,kSolveVersion,executableName,kGameName,kAuthorName);

	HitAnyKeyToContinue();

	do {
		if(gMenuMode == BeforeEvaluation)
			MenusBeforeEvaluation();
		else if(gMenuMode == Evaluated)
			MenusEvaluated();

		printf("\n\th)\t(H)elp\n");
		printf("\n\tq)\t(Q)uit\n");
		printf("\n\nSelect an option: ");

		ParseMenuChoice(GetMyChar());
	} while(TRUE);
}

void MenusBeforeEvaluation()
{
	gUnsolved = FALSE;
	if(!gSEvalLoaded) TryToLoadAnEvaluator();

	printf("\n\t----- Main (Pre-Solved) Menu for %s (variant %d) -----\n", kGameName, getOption());

	printf("\n\ts)\t(S)TART THE GAME\n");
	printf("\tw)\tSTART THE GAME (W)ITHOUT SOLVING\n");

	printf("\n\tEvaluation Options:\n\n");
	if (kSupportsTierGamesman && !kExclusivelyTierGamesman)
		// Only show/accept this choice if module supports Tier Gamesman, and can switch on/off
		printf("\tt)\tToggle (T)ier-Gamesman Mode (currently %s)\n", gTierGamesman ? "ON" : "OFF");
	// if (kLoopy)
	// 	printf("\tc)\tRun Purity Check\n"); // Matthew's WORK***************
	printf("\to)\t(O)bjective toggle from %s to %s\n",
	       gStandardGame ? "STANDARD" : "REVERSE ",
	       gStandardGame ? "REVERSE " : "STANDARD");
	if(kDebugMenu)
		printf("\td)\t(D)ebug Module BEFORE Evaluation\n");
	if(kGameSpecificMenu)
		printf("\tg)\t(G)ame-specific options for %s\n",kGameName);
	printf("\te)\tConfigure Static (E)valuator\n");
	printf("\t2)\tToggle (2)-bit solving (currently %s)\n", gTwoBits ? "ON" : "OFF");
	printf("\tp)\tToggle Global (P)osition solving (currently %s)\n", gGlobalPositionSolver ? "ON" : "OFF");
	printf("\tl)\tToggle (L)ow Mem solving (currently %s)\n", gZeroMemSolver ? "ON" : "OFF");
	printf("\tm)\tToggle Sy(M)metries (currently %s)\n", gSymmetries ? "ON" : "OFF");

}

int max(int a, int b, int c) {
	if (a>=b) {
		if (a>=c)
			return a;
		if (c>=a)
			return c;
	} else {
		if (b>=c)
			return b;
		if (c>=b)
			return c;
	}
	return -99999;
}

void printGrid(int* maxs, int numCol) {
	int ct = 0;
	int i;
	printf("\t|");
	for (i=0; i<numCol; i++) {
		printf("-");
		while(ct < (maxs[i]+3)) {
			printf("-");
			ct++;
		}
		ct = 0;
		printf("|");
	}
}

void printCell(STRING str, int max) {
	printf("| %s", str);
	max = (strlen(str)-max);
	while (max<0) {
		printf(" ");
		max++;
	}
	printf("   ");
}



void printPlayerConfig(STRING type1, STRING type2) {
	char tmp[80];
	int max1 = max(strlen(gPlayerName[kPlayerOneTurn]) + 3, strlen("Player 1"), strlen(type1)+3);
	int max2 = max(strlen(gPlayerName[kPlayerTwoTurn]) + 3, strlen("Player 2"), strlen(type2)+3);
	int max0 = 4;
	int maxs[] = {max0, max1, max2};

	printGrid(maxs, 3);

	printf("\n\t");
	printCell(" ", max0);
	printCell("Player 1", max1);
	printCell("Player 2", max2);
	printf("|\n");

	printGrid(maxs, 3);

	printf("\n\t");
	printCell("Name", max0);
	sprintf(tmp, "1) %s", gPlayerName[kPlayerOneTurn]);
	printCell(tmp, max1);
	sprintf(tmp, "2) %s", gPlayerName[kPlayerTwoTurn]);
	printCell(tmp, max2);
	printf("|\n");

	printGrid(maxs, 3);

	printf("\n\t");
	printCell("Type", max0);
	if (gUnsolved && !gSEvalLoaded) {

		sprintf(tmp, "%s", type1);
		printCell(tmp, max1);
		sprintf(tmp, "%s", type2);
		printCell(tmp, max2);
	}
	else {
		sprintf(tmp, "3) %s", type1);
		printCell(tmp, max1);
		sprintf(tmp, "4) %s", type2);
		printCell(tmp, max2);
	}
	printf("|\n");

	printGrid(maxs, 3);

	printf("\n\n");
}

void ChoosePlayerTypeMenu(int playerNum){
	char c;

	if(playerNum==1 || playerNum==2) {
		BOOLEAN computerIsPlayer = ( (gOpponent == AgainstComputer && ((playerNum==1 && !gHumanGoesFirst) || (playerNum==2 && gHumanGoesFirst))) ||
		                             gOpponent == ComputerComputer );
		BOOLEAN humanIsPlayer = ( (gOpponent == AgainstComputer && ((playerNum==1 && gHumanGoesFirst) || (playerNum==2 && !gHumanGoesFirst))) ||
		                          gOpponent == AgainstHuman );
		BOOLEAN evaluatorIsPlayer = (gOpponent == AgainstEvaluator && ((playerNum==1 && !gHumanGoesFirst) || (playerNum==2 && gHumanGoesFirst)));
		do {
			printf("\n\tPlayer Options:\n");
			printf("\t-----------------\n");
			printf("\tUse the following characters to change the type for player %d:\n", playerNum);
			if( !computerIsPlayer && !gUnsolved)
				printf("\tc)\t(C)omputer\n");
			if( !evaluatorIsPlayer && gSEvalLoaded )
				printf("\te)\tStatic (E)valuator\n");
			if( !humanIsPlayer )
				printf("\th)\t(H)uman\n");
			printf("\n\tb)\t(B)ack = Return to previous activity");
			printf("\n\tq)\t(Q)uit\n");
			printf("\n\nSelect an option: ");

			switch(c = GetMyChar()) {
			case 'c': case 'C':
				if( gUnsolved ) {
					BadMenuChoice();
					HitAnyKeyToContinue();
					break;
				}
				if(gOpponent == AgainstComputer) {
					// Assume that they didn't choose computer again. If so, no change.
					if( (playerNum==1 && !gHumanGoesFirst) || (playerNum==2 && gHumanGoesFirst) ) {
						gOpponent = ComputerComputer;
						printf("\nPlayer %d is now a computer.\n\n", playerNum);
						return;
					}
					else continue; // Do Nothing
				}
				else if(gOpponent == AgainstHuman) {
					gOpponent = AgainstComputer;
					gHumanGoesFirst = (playerNum==2) ? TRUE : FALSE;
					printf("\nPlayer %d is now a computer.\n\n", playerNum);
					return;
				}
				else if(gOpponent == ComputerComputer) {
					// Do nothing
				}
				else{ // gOpponent == Evaluator
					if( (playerNum==1 && gHumanGoesFirst) || (playerNum==2 && !gHumanGoesFirst) )
						printf("Can't have computer vs. evaluator yet. Try again.");
					else{
						gOpponent = AgainstComputer;
						gHumanGoesFirst = (playerNum==2) ? TRUE : FALSE;
						printf("\nPlayer %d is now a computer.\n\n", playerNum);
						return;
					}
				}
				break;
			case 'e': case 'E':
				if( !gSEvalLoaded ) {
					BadMenuChoice();
					HitAnyKeyToContinue();
					break;
				}
				if(gOpponent == AgainstComputer) {
					if( (playerNum==1 && gHumanGoesFirst) || (playerNum==2 && !gHumanGoesFirst) )
						printf("Can't have computer vs. evaluator yet. Try again.");
					else{
						gOpponent = AgainstEvaluator;
						gHumanGoesFirst = (playerNum==2) ? TRUE : FALSE;
						printf("\nPlayer %d is now a static evaluator.\n\n", playerNum);
						return;
					}
				}
				else if(gOpponent == AgainstHuman) {
					gOpponent = AgainstEvaluator;
					gHumanGoesFirst = (playerNum==2) ? TRUE : FALSE;
					printf("\nPlayer %d is now a static evaluator.\n\n", playerNum);
					return;
				}
				else if(gOpponent == ComputerComputer) {
					printf("Can't have computer vs. evaluator yet. Try again.");
				}
				else{ // gOpponent == Evaluator
					// Assume that they didn't choose computer again. If so, no change.
					if( (playerNum==1 && !gHumanGoesFirst) || (playerNum==2 && gHumanGoesFirst) ) {
						printf("\nPlayer %d is now a computer.\n\n", playerNum);
						return;
					}
					else continue; // Do Nothing
				}
				break;
			case 'h': case 'H':
				if(gOpponent == AgainstComputer) {
					// Assume that they didn't choose human again. If so, no change.
					if( (playerNum==1 && !gHumanGoesFirst) || (playerNum==2 && gHumanGoesFirst) ) {
						gOpponent = AgainstHuman;
						printf("\nPlayer %d is now a computer.\n\n", playerNum);
						return;
					}
					else continue; //Do nothing
				}
				else if(gOpponent == ComputerComputer) {
					gOpponent = AgainstHuman;
					gHumanGoesFirst = (playerNum==1) ? TRUE : FALSE;
					printf("\nPlayer %d is now a computer.\n\n", playerNum);
					return;
				}
				else if(gOpponent == AgainstHuman) {
					continue; //Do nothing
				}
				else{ // gOpponent == Evaluator
					if( (playerNum==1 && !gHumanGoesFirst) || (playerNum==2 && gHumanGoesFirst) )
						printf("Can't have computer vs. evaluator yet. Try again.");
					else{
						gOpponent = AgainstComputer;
						gHumanGoesFirst = (playerNum==1) ? TRUE : FALSE;
						printf("\nPlayer %d is now a computer.\n\n", playerNum);
						return;
					}
				}
				break;
			case 'B': case 'b':
				printf("\n\n");
				return;
			case 'Q': case 'q':
				ExitStageRight();
				exit(0);
			default:
				BadMenuChoice();
				HitAnyKeyToContinue();
				break;
			}
		} while(TRUE);
	}
	else{
		BadElse("ChoosePlayerTypeMenu");
	}
}

USERINPUT ConfigurationMenu()
{
	USERINPUT result = Configure;
	char c;
	char tmpName[MAXNAME];
	VALUE gameValue = undecided;
	STRING type1;
	STRING type2;
	int humanIndex, compIndex;


	if(!gUnsolved)
		gameValue = GetValueOfPosition(gInitialPosition);

	do {
		printf("\n\t----- Configuration Menu for %s -----\n\n", kGameName);

		type1 = "Human";
		type2 = "Computer";
		if(gOpponent == AgainstHuman)
			type2 = type1;
		else if(gOpponent == ComputerComputer)
			type1 = type2;
		else if(gOpponent == AgainstComputer) {
			if(!gHumanGoesFirst) {
				type2 = type1;
				type1 = "Computer";
			}
		}else if(gOpponent == AgainstEvaluator) {
			type2 = "Evaluator";
			if(!gHumanGoesFirst) {
				type2 = type1;
				type1 = "Evaluator";
			}
		}

		printPlayerConfig(type1, type2);


		printf("\tPlayer Options:\n");
		printf("\t-----------------\n");

		printf("\tUse the numbers to change the corresponding name or type of players:\n");
		printf("\t1)\tChange the name of player 1 (currently %s)\n",gPlayerName[1]);
		printf("\t2)\tChange the name of player 2 (currently %s)\n",gPlayerName[0]);

		if (!gUnsolved || gSEvalLoaded) {
			printf("\t3)\tChange player type of %s (currently %s)\n", gPlayerName[1], type1);
			printf("\t4)\tChange player type of %s (currently %s)\n", gPlayerName[0], type2);
		}
		printf("\n");
		if(gOpponent == AgainstHuman || gOpponent == AgainstEvaluator)
			printf("\tf)\t(F)lip board to swap players 1 and 2");
		else if(gOpponent == AgainstComputer)
		{
			humanIndex = kPlayerOneTurn;
			compIndex = kPlayerTwoTurn;
			if (!gHumanGoesFirst) {
				humanIndex = kPlayerTwoTurn;
				compIndex = kPlayerOneTurn;
			}
			printf("\tf)\t(F)lip board to swap players 1 and 2\n");
			if(gameValue == tie)
				printf("\t\tNOTE: FIRST - can tie/lose; SECOND - can tie/lose\n");
			else if (gameValue == lose)
				printf("\t\tNOTE: First - can %s; Second - can %s",
				       gHumanGoesFirst ? "only lose" : "win/lose",
				       gHumanGoesFirst ? "win/lose" : "only lose");
			else if (gameValue == win)
				printf("\t\tNOTE: First - can %s; Second - can %s",
				       gHumanGoesFirst ? "win/lose" : "only lose",
				       gHumanGoesFirst ? "only lose" : "win/lose");
			else
				BadElse("Menus");

			/*analysis taken out*/

			printf("\n\n\tAI Options:\n");
			printf("\t-------------\n");
			printf("\tAdjust AI %s's brain (currently ", gPlayerName[compIndex]);
			if (smartness==SMART) {
				printf("%d%% perfect", scalelvl);
			}
			else if (smartness==DUMB) {
				printf("misere-ly");
			}
			else if (smartness==RANDOM) {
				printf("randomly");
			}
			printf (" w/%d giveback%s)\n", initialGivebacks, initialGivebacks == 1 ? "" : "s");
			printf("\t\tp)\t(P)erfectly always\n");
			printf("\t\ti)\t(I)mperfectly (Plays randomly at times)\n");
			printf("\t\tr)\t(R)andomly always\n");
			printf("\t\tm)\t(M)isere-ly always (i.e., trying to lose!)\n\n");
			if (gPutWinBy) {
				printf("\t\tw)\tToggle from %s\n", (gWinBy ? "(W)inBy to Remoteness" : "Remoteness to (W)inBy"));
				if (gWinBy) {
					printf("\t\tc)\tToggle from %s\n", (gWinByClose ? "WinBy(C)lose to WinBy" : "WinBy to WinBy(C)lose"));
				}
			}
			printf("\t\tg)\tChange the number of (G)ive-backs\n");
		}


		printf("\n\tPlaying Options:\n");
		printf("\t------------------\n");
		if(!gUnsolved) {

			printf("\td)\tToggle from %sPRE(D)ICTIONS to %sPREDICTIONS\n",
			       gPrintPredictions ? "   " : "NO ",
			       !gPrintPredictions ? "   " : "NO ");
			printf("\tn)\tToggle from %sHI(N)TS       to %sHINTS\n",
			       gHints ? "   " : "NO ",
			       !gHints ? "   " : "NO ");
		}

		if(gSEvalLoaded) {

			printf("\te)\tToggle from %s(E)VAL-PREDICTIONS to %sEVAL-PREDICTIONS\n",
			       gPrintSEvalPredictions ? "   " : "NO ",
			       !gPrintSEvalPredictions ? "   " : "NO ");
		}

		printf("\ts)\tIf only one move is available, (S)kip user input (currently %s)\n\n",
		       gSkipInputOnSingleMove ? "ON" : "OFF");
		printf("\th)\t(H)elp\n");
		printf("\tb)\t(B)ack = Return to previous activity");
		printf("\n\tq)\t(Q)uit");
		printf("\n\nSelect an option: ");

		switch(c = GetMyChar()) {
		case '1':
			printf("\nEnter the name of player 1 (max. %d chars) [%s] : ",
			       MAXNAME-1, gPlayerName[kPlayerOneTurn]);
			GetMyStr(tmpName,MAXNAME);
			if(strcmp(tmpName,""))
				(void) sprintf(gPlayerName[kPlayerOneTurn],"%s",tmpName);
			break;
		case '2':
			printf("\nEnter the name of player 2 (max. %d chars) [%s] : ",
			       MAXNAME-1, gPlayerName[kPlayerTwoTurn]);
			GetMyStr(tmpName,MAXNAME);
			if(strcmp(tmpName,""))
				(void) sprintf(gPlayerName[kPlayerTwoTurn],"%s",tmpName);
			break;
		case '3':
			if(gUnsolved && !gSEvalLoaded) {
				BadMenuChoice();
				HitAnyKeyToContinue();
				break;
			}
			ChoosePlayerTypeMenu(1);
			/*result = Configure;*/
			break;
		case '4':
			if(gUnsolved && !gSEvalLoaded) {
				BadMenuChoice();
				HitAnyKeyToContinue();
				break;
			}
			ChoosePlayerTypeMenu(2);
			/*result = Configure;*/
			break;

		case 'D': case 'd':
			if(gUnsolved) {
				BadMenuChoice();
				HitAnyKeyToContinue();
				break;
			}
			gPrintPredictions = !gPrintPredictions;
			break;
		case 'E': case 'e':
			if(!gSEvalLoaded) {
				BadMenuChoice();
				HitAnyKeyToContinue();
				break;
			}
			gPrintSEvalPredictions = !gPrintSEvalPredictions;
			break;
		case 'N': case 'n':
			if(gUnsolved) {
				BadMenuChoice();
				HitAnyKeyToContinue();
				break;
			}
			gHints = !gHints;
			break;
		case 'F': case 'f':
			if(gOpponent == AgainstComputer || gOpponent==AgainstEvaluator) {
				gHumanGoesFirst = !gHumanGoesFirst;
				(void) sprintf(tmpName,"%s",gPlayerName[kPlayerOneTurn]);
				(void) sprintf(gPlayerName[kPlayerOneTurn],"%s",gPlayerName[kPlayerTwoTurn]);
				(void) sprintf(gPlayerName[kPlayerTwoTurn],"%s",tmpName);
			} else if(gOpponent == AgainstHuman) {
				(void) sprintf(tmpName,"%s",gPlayerName[kPlayerOneTurn]);
				(void) sprintf(gPlayerName[kPlayerOneTurn],"%s",gPlayerName[kPlayerTwoTurn]);
				(void) sprintf(gPlayerName[kPlayerTwoTurn],"%s",tmpName);
			} else {
				BadMenuChoice();
				HitAnyKeyToContinue();
			}
			break;

		case 's': case 'S':
			gSkipInputOnSingleMove = !gSkipInputOnSingleMove;
			break;
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
		case 'W': case 'w':
			if (!gPutWinBy) {
				BadMenuChoice();
			} else {
				gWinBy = !gWinBy;
				if (gWinBy == FALSE) {
					gWinByClose = FALSE;
				}
			}
			HitAnyKeyToContinue();
			break;
		case 'C': case 'c':
			if (!gPutWinBy || !gWinBy) {
				BadMenuChoice();
			} else {
				gWinByClose = !gWinByClose;
			}
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
		case 'h': case 'H':
			HelpMenus();
			break;
		case 'B': case 'b':
			printf("\n\n");
			return result;

		case 'Q': case 'q':
			ExitStageRight();
			exit(0);
		default:
			BadMenuChoice();
			HitAnyKeyToContinue();
			break;
		}


	} while (TRUE);
}

void MenusEvaluated()
{
	printf("\n\t----- Play (Post-Solved) Menu for %s (variant %d) -----\n", kGameName, getOption());

	/*took out configuration options*/

	printf("\n\tp)\t(P)LAY NEW GAME\n");
	printf("\n\tn)\tPLAY (N)ETWORKED GAME\n");
	printf("\n\tc)\t(C)onfigure play options\n");

	if(!gUnsolved)
	{
		printf("\n\ta)\t(A)nalyze the game\n");
	}

	if(kLoopy && !(gTierGamesman && kSupportsTierGamesman)) {
		printf("\n\tr)\t(R)un Purity Check\n");
	}


	if(kDebugMenu)
		printf("\n\td)\t(D)ebug Game AFTER Evaluation\n");

	printf("\n\tm)\tGo to the (M)ain Menu to edit the rules / starting position\n");
}

void ParseMenuChoice(char c)
{
	if (ParseConstantMenuChoice(c)) ;
	else if(gMenuMode == BeforeEvaluation)
		ParseBeforeEvaluationMenuChoice(c);
	else if(gMenuMode == Evaluated)
		ParseEvaluatedMenuChoice(c);
	else {
		BadElse("ParseMenuChoice");
		HitAnyKeyToContinue();
	}
}

BOOLEAN ParseConstantMenuChoice(char c)
{
	switch(c) {
	case 'Q': case 'q':
		ExitStageRight();
		exit(0);
	case 'h': case 'H':
		HelpMenus();
		break;
	default:
		return(FALSE); /* It was not parsed here */
	}
	return(TRUE);   /* Yep, it was parsed here! */
}

void ParseBeforeEvaluationMenuChoice(char c)
{

	BOOLEAN tempPredictions;
	VALUE gameValue;
	char mexString[20]; /* Mex values read "[Val = *14]", easily in 20 chars */
	char tmpString[80]; /* Just for saving the remoteness value in it */

	switch(c) {
	case 'G': case 'g':
		if(kGameSpecificMenu)
		{
			tempPredictions = gPrintPredictions;
			gPrintPredictions = FALSE;
			GameSpecificMenu();
			gPrintPredictions = tempPredictions;
		}
		else {
			BadMenuChoice();
			HitAnyKeyToContinue();
		}
		break;
	case '2':
		gTwoBits = !gTwoBits;
		break;
	case 'p': case 'P':
		gGlobalPositionSolver = !gGlobalPositionSolver;
		break;
	case 'l': case 'L':
		gZeroMemSolver = !gZeroMemSolver;
		break;
	case 'e': case 'E':
		StaticEvaluatorMenu();
		break;
	case 'm': case 'M': {
		if(gCanonicalPosition)
			gSymmetries = !gSymmetries;
		else printf("Sorry, but the game does not seem to have support for symmetries.\n");
		break;
	}
	case 't': case 'T': {
		if (kSupportsTierGamesman && !kExclusivelyTierGamesman)
			gTierGamesman = !gTierGamesman;
		else BadMenuChoice();
		break;
	}
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
		Initialize();
		gOpponent = AgainstComputer;
		gPrintPredictions = TRUE;
		gPrintSEvalPredictions = FALSE;
		sprintf(gPlayerName[kPlayerOneTurn],"Player");
		sprintf(gPlayerName[kPlayerTwoTurn],"Data");
		printf("\nSolving with loopy code %s...%s!",kGameName,kLoopy ? "Yes" : "No");
		if (kLoopy && gGoAgain!=DefaultGoAgain) printf(" with Go Again support");
		printf("\nSolving with ZeroSolver %s...%s!",kGameName,gZeroMemSolver ? "Yes" : "No");
		printf("\nRandom(100) three times %s...%d %d %d",kGameName,GetRandomNumber(100),GetRandomNumber(100),GetRandomNumber(100));
		printf("\nInitializing insides of %s...", kGameName);
		fflush(stdout);
		Stopwatch();
		if(!(kSupportsTierGamesman && gTierGamesman)) { //If no TIER GAMESMAN
			InitializeDatabases();
			if (gUseOpen) InitializeOpenPositions(gNumberOfPositions);
		}
		printf("done in %u seconds!", Stopwatch()); // for analysis bookkeeping
		fflush(stdout);
		Stopwatch();
		gPrintDatabaseInfo = TRUE;
		gameValue = DetermineValue(gInitialPosition);
		if((Remoteness(gInitialPosition)) == REMOTENESS_MAX && !gTwoBits) {
			printf("\n\nThe Game %s has value: Draw\n\n", kGameName);
		} else {
			MexFormat(gInitialPosition, mexString); /* Mex value not so well-defined for draws */
			sprintf(tmpString, "in %d", Remoteness(gInitialPosition));
			printf("\n\nThe Game %s has value: %s %s %s\n\n",
			       kGameName,
			       gValueString[(int)gameValue],
			       gTwoBits ? "" : tmpString, /* TwoBit solvers have no remoteness */
			       mexString);
		}
		gMenuMode = Evaluated;
		if(gameValue == lose) {
			gHumanGoesFirst = FALSE;

			sprintf(gPlayerName[kPlayerTwoTurn],"Player");
			sprintf(gPlayerName[kPlayerOneTurn],"Data");
		}
		else
			gHumanGoesFirst = TRUE;
		HitAnyKeyToContinue();
		break;
	case 'w': case 'W':
		InitializeGame();
		gUnsolved = TRUE;
		if(gSEvalLoaded) {
			gOpponent = AgainstEvaluator;
			gPrintSEvalPredictions = TRUE;
		}
		else
			gOpponent = AgainstHuman;
		gPrintPredictions = FALSE;
		if(kSupportsTierGamesman && gTierGamesman) //TIER GAMESMAN
			gInitialPosition = InitTierGamesman();
		printf("\n\nYou have chosen to play the game without solving.\n");
		sprintf(gPlayerName[kPlayerOneTurn],"Player");
		if(gOpponent==AgainstEvaluator) {
			printf("I found a static evaluator loaded, so I'm making it your opponent.\n");
			sprintf(gPlayerName[kPlayerTwoTurn],"Evaluator");
		}
		else {
			printf("I did not find an evaluator, so I'm making this a two player game.\n");
			sprintf(gPlayerName[kPlayerTwoTurn],"Challenger");
		}
		printf("Have fun!\n\n");
		gMenuMode = Evaluated;
		HitAnyKeyToContinue();
		break;
	default:
		BadMenuChoice();
		HitAnyKeyToContinue();
		break;
	}
}





void ParseEvaluatedMenuChoice(char c)
{

	PLAYER playerOne,playerTwo;

	switch(c) {

	case 'D': case 'd':
		if(kDebugMenu)
			DebugMenu();
		else {
			BadMenuChoice();
			HitAnyKeyToContinue();
		}
		break;
	case 'C': case 'c':
		ConfigurationMenu();
		break;
	case 'A': case 'a':
		//analyze();
		if (!gUnsolved || (gOpponent == AgainstComputer))
			AnalysisMenu();
		else {
			BadMenuChoice();
			HitAnyKeyToContinue();
		}
		break;
	case 'n': case 'N':
		playerOne = NewHumanPlayer("Player One", 0);
		playerTwo = NewHumanPlayer("Player Two", 1);
		if (SetupNetworkGame(kGameName)) {
			playerOne->GetMove = RemoteMove;
			playerTwo->GetMove = LocalPlayersMove;
		} else {
			playerOne->GetMove = LocalPlayersMove;
			playerTwo->GetMove = RemoteMove;
		}
		PlayGame(playerOne, playerTwo);
	case 'p': case 'P':
		if(gOpponent == AgainstComputer) {
			if(gHumanGoesFirst) {
				playerOne = NewHumanPlayer(gPlayerName[kPlayerOneTurn],0);
				playerTwo = NewComputerPlayer(gPlayerName[kPlayerTwoTurn],1);
			} else {
				playerOne = NewComputerPlayer(gPlayerName[kPlayerOneTurn],0);
				playerTwo = NewHumanPlayer(gPlayerName[kPlayerTwoTurn],1);
			}
			PlayGame(playerOne,playerTwo);
		}
		else if(gOpponent == AgainstHuman) {
			playerOne = NewHumanPlayer(gPlayerName[kPlayerOneTurn],0);
			playerTwo = NewHumanPlayer(gPlayerName[kPlayerTwoTurn],1);
			PlayGame(playerOne,playerTwo);
		}
		else if(gOpponent == ComputerComputer) {
			playerOne = NewComputerPlayer(gPlayerName[kPlayerOneTurn],0);
			playerTwo = NewComputerPlayer(gPlayerName[kPlayerTwoTurn],1);
			PlayGame(playerOne,playerTwo);
		}
		else if(gOpponent == AgainstEvaluator) {
			if(gHumanGoesFirst) {
				playerOne = NewHumanPlayer(gPlayerName[kPlayerOneTurn],0);
				playerTwo = NewSEvalPlayer(gPlayerName[kPlayerTwoTurn],1);
			} else {
				playerOne = NewSEvalPlayer(gPlayerName[kPlayerOneTurn],0);
				playerTwo = NewHumanPlayer(gPlayerName[kPlayerTwoTurn],1);
			}
			PlayGame(playerOne,playerTwo);
		}
		else
			BadElse("Invalid Opponent to play!");
		break;
	case 'r': case 'R':
		if (kLoopy && !(gTierGamesman && kSupportsTierGamesman)) {
			if (gCheckPure) {
				gPure = DeterminePure(gInitialPosition);
				gCheckPure = FALSE;
			}
			printf("\tThe game is%s pure\n", gPure ? "" : " not");
		} else {
			BadMenuChoice();
		}
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

void HelpMenus()
{
	char c;

	do {
		printf("\n\t----- HELP for %s module -----\n\n", kGameName);

		printf("\t%s Help:\n\n",kGameName);

		printf("\t1)\tWhat do I do on MY TURN?\n");
		printf("\t2)\tHow to tell the computer WHICH MOVE I want?\n");
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

		printf("\n\n\tb)\t(B)ack = Return to previous activity\n");
		printf("\n\nSelect an option: ");

		ParseHelpMenuChoice(c = GetMyChar());

	} while(c != 'b' && c != 'B');
}

void ParseHelpMenuChoice(char c)
{
	switch(c) {
	case 'Q': case 'q':
		ExitStageRight();
		exit(0);
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
			if(gOpponent == AgainstComputer)
				PrintComputerValueExplanation();
			else
				PrintHumanValueExplanation();
		}
		else {
			printf("%s\n", kHelpValueBeforeEvaluation);
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

void BadMenuChoice()
{
	printf("\nSorry, I don't know that option. Try another.\n");
}

/* Jiong */
/*void SmarterComputerMenu()
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
   }*/

void AnalysisMenu()
{
	POSITIONLIST *badWinPositions = NULL, *badTiePositions = NULL, *badLosePositions = NULL;
	BOOLEAN tempPredictions = gPrintPredictions, CorruptedValuesP();
	MEX mexValue = 0;
	int mexInt;
	unsigned int maxPositions = 10;
	char c;

	gPrintPredictions = FALSE;
	gMenuMode = Analysis; /* By default, no symmetries -- raw values */

	/* Check if analysis DB loaded */
	do {
		if(!(gAnalysisLoaded = LoadAnalysis())) {
			printf("\nNo analysis DB found.\nA new one must be generated in order to view analysis results.\n");
			printf("\tr)\t(R)e-solve %s and generate an analysis DB.\n\t\t  WARNING: this could take a very long time.\n", kGameName);
			printf("\tb)\t(B)ack = Return to previous activity\n");
			printf("\n\nSelect an option: ");
			switch(c = GetMyChar()) {
			case 'R': case 'r':
				analyze();
				break;
			case 'B': case 'b':
				FreePositionList(badWinPositions);
				FreePositionList(badTiePositions);
				FreePositionList(badLosePositions);
				gPrintPredictions = tempPredictions;
				gMenuMode = Evaluated; /* Return to simple 'Evaluated' mode/menu */
				return;
			default:
				BadMenuChoice();
				HitAnyKeyToContinue();
				break;
			}
		} else {
			break;
		}
	} while(TRUE);

	do {
		printf("\n\t----- Post-Evaluation ANALYSIS menu for %s -----\n\n", kGameName);
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
		printf("\tu)\tPrint up to %d (U)ndecided positions\n",maxPositions);
		printf("\ty)\tPrint up to %d  S(Y)mmetric positions\n",maxPositions);
		printf("\tr)\tPrint up to %d Win, Lose, Tie, o(R) Undecided positions\n",maxPositions);
		printf("\n\tp)\t(P)rint the overall summmary of game values\n");
		printf("\ta)\tPrint detailed (A)nalysis of game values and remoteness distribution\n");  // New printf added
		printf("\tf)\tPrint to an ascii (F)ile the raw game values + remoteness\n");
		printf("\tz)\tPrint to an binary export File the raw game values + remoteness (z)\n");
		printf("\to)\tPrint to std(O)ut the raw game values + remoteness\n");

		printf("\td)\tPrint (D)atabase comb visualization (POSitions vs NEGativeSpace)\n");
		printf("\tg)\t(G)enerate visualization of game tree in DOT format\n");
		printf("\n\tc)\t(C)heck if value database is corrupted\n");

		printf("\ts)\t(S)olve for interestingness (takes some time)\n");
		printf("\te)\tS(E)t initial position to the most interesting (only after interestingness solver)\n");

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
			exit(0);
		case 'H': case 'h':
			HelpMenus();
			break;
		case 'D': case 'd':
			DatabaseCombVisualization();
			break;
		case 'C': case 'c':
			gMenuMode = Evaluated;
			if(CorruptedValuesP())
				printf("\nCorrupted values found and printed above. Sorry.\n");
			else
				printf("\nNo Corrupted Values found!\n");
			gMenuMode = Analysis;
			HitAnyKeyToContinue();
			break;
		case 'F': case 'f':
			PrintRawGameValues(TRUE);
			break;
		case 'Z': case 'z':
			PrintBinaryGameValuesToFile(NULL);
			break;
		case 'O': case 'o':
			PrintRawGameValues(FALSE);
			HitAnyKeyToContinue();
			break;
		case 'i': case 'I':
			printf("\n\t----- The Initial Position is shown below -----\n");
			PrintPosition(gInitialPosition, "Nobody", TRUE);
			HitAnyKeyToContinue();
			break;
		case 'p': case 'P':
			//analyze();
			PrintGameValueSummary();
			HitAnyKeyToContinue();
			break;
		case 'a': case 'A':                             // New detailed analysis implementation
			//analyze(); Changed analysis to hook into solver, so don't need this call
			//printf("\ngot through analyzing, segfault later!\n");
			PrintDetailedGameValueSummary();
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
			maxPositions = GetMyUInt();
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
		case 'w': case 'W': case 'l': case 'L': case 't': case 'T': case 'r': case 'R': case 'u': case 'U': case 'y': case 'Y':
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
			gMenuMode = Evaluated; /* Return to simple 'Evaluated' mode/menu */
			return;
		case 'G': case 'g':
			VisualizationMenu();
			break;
		case 'E': case 'e':
			if (gAnalysis.MostInteresting != 0) {
				gInitialPosition = gAnalysis.MostInteresting;
				printf("\nSet to position %llu with interestingness %f\n\n",gInitialPosition,gAnalysis.MaxInterestingness);
			} else {
				printf("\nNo interesting position set... run interestingness solver first!.\n");
			}

			break;
		case 'S': case 's':
			DetermineInterestingness(gInitialPosition);
			break;
		default:
			BadMenuChoice();
			HitAnyKeyToContinue();
			break;
		}
	} while(TRUE);
}

void VisualizationMenu()
{
	char c;

	do {
		printf("\n\t----- Post-Evaluation VISUALIZATION menu for %s -----\n\n", kGameName);
		printf("\n\te)\t Toggle (E)dge drawing (currently %s)", gDrawEdges ? "ON" : "OFF");
		printf("\n\tn)\t Toggle writing text file with (N)ode visualization (currently %s)", gGenerateNodeViz ? "ON" : "OFF");
		printf("\n\tr)\t Toggle ordering nodes by (R)emoteness (currently %s)", gRemotenessOrder ? "ON" : "OFF");
		printf("\n\n\tv)\t Visualize game graph");
		printf("\n\n\th)\t(H)elp\n");
		printf("\n\tb)\t(B)ack = Return to previous activity\n");
		printf("\n\nSelect an option: ");

		switch(c = GetMyChar()) {
		case 'Q': case 'q':
			ExitStageRight();
			exit(0);
		case 'H': case 'h':
			HelpMenus();
			break;
		case 'E': case 'e':
			gDrawEdges = !gDrawEdges;
			break;
		case 'N': case 'n':
			gGenerateNodeViz = !gGenerateNodeViz;
			break;
		case 'R': case 'r':
			gRemotenessOrder = !gRemotenessOrder;
			break;
		case 'V': case 'v':
			Visualize();
			HitAnyKeyToContinue();
			break;
		case 'b': case 'B':
			gMenuMode = Evaluated; /* Return to simple 'Evaluated' mode/menu */
			return;
		default:
			BadMenuChoice();
			HitAnyKeyToContinue();
			break;
		}
	} while(TRUE);
}

USERINPUT HandleDefaultTextInput(POSITION thePosition, MOVE* theMove, STRING playerName)
{
	MOVE tmpMove;
	char tmpAns, input[MAXINPUTLENGTH];
	MOVELIST* head;
	int onlyOneMove;
	int i = 0;

	for (i = 0; i < MAXINPUTLENGTH; ++i) {
		input[i] = '\0';
	}
	/*to skip input we have to see what moves are available first, and since we
	   are not solving there is esstially no performance issue here*/
	head = GenerateMoves(thePosition); /* What are all moves available? */
	onlyOneMove = (head != NULL && head->next == NULL);
	*theMove = head->move;
	if ( gSkipInputOnSingleMove && onlyOneMove ) {
		printf("\n---------- SELECTING THE ONLY MOVE ---------> ");
		PrintMove(*theMove);
		printf("\n");
		return (Move);
	}
	FreeMoveList(head);

	GetMyStr(input, MAXINPUTLENGTH);

	if(input[0] == '\0') {
		/* [DDG 2005-01-09] Check if there is only one move to be made.
		 * If so, this can be a shortcut for moving, just hitting enter! */
		/*      head = GenerateMoves(thePosition); What are all moves available? */
		/* There's exactly one */
		/*if (onlyOneMove = (head != NULL && head->next == NULL)){
		 *  *theMove = head->move;
		 * printf("----- AUTO-MOVE-SELECTED ------------> ");
		 * PrintMove(*theMove);
		 * printf("\n");
		 *}
		 * FreeMoveList(head);*/

		if ( onlyOneMove ) {
			printf("\n-------- SELECTING THE ONLY MOVE --------> ");
			PrintMove(*theMove);
			printf("\n");
			return(Move);
		} else
			PrintPossibleMoves(thePosition);

	} else if (ValidTextInput(input)) {
		if(ValidMove(thePosition,tmpMove = ConvertTextInputToMove(input))) {
			*theMove = tmpMove;
			return(Move);
		}
		else
			PrintPossibleMoves(thePosition);

	} else {
		switch(input[0]) {
		case 'Q': case 'q':
			ExitStageRight();
			exit(0);
		case 'u': case 'U':
			return(Undo);
		case 'a': case 'A':
			printf("\nSure you want to Abort? [no] :  ");
			tmpAns = GetMyChar();
			printf("\n");
			if(tmpAns == 'y' || tmpAns == 'Y')
				return(Abort);
			else{
				PrintPosition(thePosition, playerName, TRUE);
				return(Continue);
			}
		case 'H': case 'h':
			HelpMenus();
			printf("");
			PrintPosition(thePosition, playerName, TRUE);
			break;
		case 'c': case 'C':
			return ConfigurationMenu();
			/*PrintPosition(thePosition, playerName, TRUE);*/
			break;
		case 'p': case 'P':
			GamePrintMenu(thePosition, playerName, TRUE, input[1]);
			break;
		case 'e': case 'E':
			printf("\n\nValue for this position: %.3f [-1,1]\n\n", evaluatePosition(thePosition));
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
	}

	return(Continue); /* The default action is to return Continue */
}

void GamePrintMenu(POSITION thePosition, STRING playerName, BOOLEAN usersTurn, char input)
{
	char c;

	do {

		if (input == '\0')
		{

			printf("\n\t----- Print Menu for %s -----\n", kGameName);
			printf("%s", kPrintMenu);

			if (!gUnsolved)
				printf("%s", kPrintMenuWithSolving);
			printf("%s", kPrintMenuEnd);
			printf("\nSelect an option: ");
			c = GetMyChar();
		}
		else {
			c = input;
		}
		switch(c) {
		case 'r': case 'R':
			PrintPosition(thePosition, playerName, TRUE);
			return;
		case 's': case 'S':
			if (gUnsolved) {
				BadMenuChoice();
				HitAnyKeyToContinue();
				return;
			} else {
				PrintValueMoves(thePosition);
				PrintPosition(thePosition, playerName, TRUE);
				return;
			}
		case 'v': case 'V':
			if (gUnsolved) {
				BadMenuChoice();
				HitAnyKeyToContinue();
				return;
			} else {
				PrintVisualValueHistory(thePosition, 0);
				return;
			}
		case 'w': case 'W':
			if (gUnsolved) {
				BadMenuChoice();
				HitAnyKeyToContinue();
				return;
			} else {
				PrintVisualValueHistory(thePosition, 1);
				return;
			}
		case 'm': case 'M':
			PrintMoveHistory(thePosition);
			return;
		case 'H': case 'h':
			HelpMenus();
			printf("");
			PrintPosition(thePosition, playerName, TRUE);
			return;
		case 'b': case 'B':
			PrintPosition(thePosition, playerName, TRUE);
			return;

		case 'Q': case 'q':
			ExitStageRight();
			exit(0);
		default:
			BadMenuChoice();
			HitAnyKeyToContinue();
			break;
		}
	} while(TRUE);
}

/* Status Meter */
void showStatus(STATICMESSAGE msg)
{

	static float timeDelayTicks = CLOCKS_PER_SEC / 10;
	static clock_t updateTime = (clock_t) NULL;
	int print_length=0;
	float percent = PercentDone(msg);

	if (updateTime == (clock_t) NULL)
	{
		updateTime = clock() + timeDelayTicks; /* Set Time for the First Time */
	}

	switch (msg)
	{
	case Clean:
		/*if(gWriteDatabase)*/

		if(gSaveDatabase)
		{
			print_length = fprintf(stderr,"Writing Database...\e[K");
			fprintf(stderr,"\e[%dD",print_length - 3); /* 3 Characters for the escape sequence */
		}
		updateTime = (clock_t) NULL;
		return;
	default:
		break;
	}

	if (clock() > updateTime)
	{
		fflush(stdout);
		fflush(stderr);
		print_length = fprintf(stderr,"%2.1f%% Done \e[K",percent);
		fprintf(stderr,"\e[%dD",print_length - 3); /* 3 Characters for the escape sequence */
		updateTime = clock() + timeDelayTicks; /* Get the Next Update Time */
	}
}


/* DB Loading Status Meter */
void showDBLoadingStatus(STATICMESSAGE msg)
{

	static float timeDelayTicks = CLOCKS_PER_SEC / 10;
	static clock_t updateTime = (clock_t) NULL;
	int print_length=0;
	if (!gPrintDatabaseInfo)
		return;
	float percent = PercentLoaded(msg);

	if (updateTime == (clock_t) NULL)
	{
		updateTime = clock() + timeDelayTicks; /* Set Time for the First Time */
	}

	switch (msg)
	{
	case Clean:
		if(gLoadDatabase)
		{
			printf("\nLoading in Database for %s...",kGameName);
		}
		updateTime = (clock_t) NULL;
		return;
	default:
		break;
	}

	if (clock() > updateTime)
	{
		fflush(stdout);
		fflush(stderr);
		print_length = fprintf(stderr,"%2.1f%% Done \e[K",percent);
		fprintf(stderr,"\e[%dD",print_length - 3); /* 3 Characters for the escape sequence */
		updateTime = clock() + timeDelayTicks; /* Get the Next Update Time */
	}
}


