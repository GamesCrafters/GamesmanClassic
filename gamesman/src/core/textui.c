char GetMyChar()
{
    char inString[MAXINPUTLENGTH], ans;
    scanf("%s",inString);
    ans = inString[0];
    return(ans);
}



void Menus()
{
    printf(kOpeningCredits,kSolveVersion,kGameName);

    HitAnyKeyToContinue();

    do {
        printf("\n\t----- GAMESMAN version %s with %s module -----\n", 
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

void MenusBeforeEvaluation()
{
    if(gUnsolved) {
	gTwoBits = FALSE;
	gUnsolved = FALSE;
    }
    printf("\n\ts)\t(S)TART THE GAME\n");
    printf("\tw)\tSTART THE GAME (W)ITHOUT SOLVING\n");

    printf("\n\tEvaluation Options:\n\n");
    printf("\to)\t(O)bjective toggle from %s to %s\n",
        gStandardGame ? "STANDARD" : "REVERSE ",
        gStandardGame ? "REVERSE " : "STANDARD");
    if(kDebugMenu)
        printf("\td)\t(D)ebug Module BEFORE Evaluation\n");
    if(kGameSpecificMenu)
        printf("\tg)\t(G)ame-specific options for %s\n",kGameName);
    printf("\t2)\tToggle (2)-bit solving (currently %s)\n", gTwoBits ? "ON" : "OFF");
    printf("\tl)\tToggle (L)ow Mem solving (currently %s)\n", kZeroMemSolver ? "ON" : "OFF");
}

void MenusEvaluated()
{
    VALUE gameValue;
    if(!gUnsolved)
        gameValue = GetValueOfPosition(gInitialPosition);
    
    printf("\n\tPlayer Name Options:\n\n");
    
    printf("\t1)\tChange the name of player 1 (currently %s)\n",gPlayerName[1]);
    printf("\t2)\tChange the name of player 2 (currently %s)\n",gPlayerName[0]);
    if(!gAgainstComputer)
        printf("\t3)\tSwap %s (plays FIRST) with %s (plays SECOND)\n", gPlayerName[1], gPlayerName[0]);
    
    if(!gUnsolved) {
	printf("\n\tGeneric Options:\n\n");
	
	printf("\t4)\tToggle from %sPREDICTIONS to %sPREDICTIONS\n",
	       gPrintPredictions ? "   " : "NO ",
	       !gPrintPredictions ? "   " : "NO ");
	printf("\t5)\tToggle from %sHINTS       to %sHINTS\n",
	       gHints ? "   " : "NO ",
	       !gHints ? "   " : "NO ");
    }
    
    printf("\n\tPlaying Options:\n\n");
    if(!gUnsolved) {
	printf("\t6)\tToggle opponent from a %s to a %s\n",
	       gAgainstComputer ? "COMPUTER" : "HUMAN",
	       gAgainstComputer ? "HUMAN" : "COMPUTER");
    }
    if(gAgainstComputer)
	{
	    if(gameValue == tie)
		printf("\t7)\tToggle from going %s (can tie/lose) to %s (can tie/lose)\n",
		       gHumanGoesFirst ? "FIRST" : "SECOND",
		       gHumanGoesFirst ? "SECOND" : "FIRST");
	    else if (gameValue == lose)
		printf("\t7)\tToggle from going %s (can %s) to %s (can %s)\n",
		       gHumanGoesFirst ? "FIRST" : "SECOND",
		       gHumanGoesFirst ? "only lose" : "win/lose",
		       gHumanGoesFirst ? "SECOND" : "FIRST",
		       gHumanGoesFirst ? "win/lose" : "only lose");
	    else if (gameValue == win)
		printf("\t7)\tToggle from going %s (can %s) to %s (can %s)\n",
		       gHumanGoesFirst ? "FIRST" : "SECOND",
		       gHumanGoesFirst ? "win/lose" : "only lose",
		       gHumanGoesFirst ? "SECOND" : "FIRST",
		       gHumanGoesFirst ? "only lose" : "win/lose");
	    else
		BadElse("Menus");
	    
	    printf("\n\ta)\t(A)nalyze the game\n");
	    printf("\tc)\tAdjust (C)omputer's brain (currently ");
	    if (smartness==SMART) {
		printf("%d%% perfect", scalelvl);
	    }
	    else if (smartness==DUMB) {
		printf("misere-ly");
	    }
	    else if (smartness==RANDOM) {
	    printf("randomly");
	    }
	    printf (" w/%d givebacks)\n", initialGivebacks);
	}
    
    if(kDebugMenu)
        printf("\td)\t(D)ebug Game AFTER Evaluation\n");
    printf("\n\tp)\t(P)LAY GAME.\n");
    
    printf("\n\tm)\tGo to (M)ain Menu to edit game rules or starting position.\n");
}

void ParseMenuChoice(char c)
{
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
      return(FALSE);  /* It was not parsed here */
    }
    return(TRUE);       /* Yep, it was parsed here! */
}

void ParseBeforeEvaluationMenuChoice(char c)
{

    BOOLEAN tempPredictions;
    int timer;
    VALUE gameValue;


    switch(c) {
    case 'G': case 'g':
	if(kGameSpecificMenu)
      {
          tempPredictions = gPrintPredictions ;
          gPrintPredictions = FALSE ;
          GameSpecificMenu();
          gPrintPredictions = tempPredictions ;
      }
	else {
	    BadMenuChoice();
	    HitAnyKeyToContinue();
	}
	break;
    case '2':
	gTwoBits = !gTwoBits;
	break;
    case 'l': case 'L':
	kZeroMemSolver = !kZeroMemSolver;
	break;
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
	gAgainstComputer = TRUE;
	gPrintPredictions = TRUE;
	sprintf(gPlayerName[kPlayerOneTurn],"Player");
	sprintf(gPlayerName[kPlayerTwoTurn],"Computer");
	printf("\nSolving with loopy code %s...%s!",kGameName,kLoopy?"Yes":"No");
	if (kLoopy && gGoAgain!=DefaultGoAgain) printf(" with Go Again support");
	printf("\nSolving with zero solver %s...%s!",kGameName,kZeroMemSolver?"Yes":"No");
	printf("\nRandom(100) three times %s...%d %d %d",kGameName,GetRandomNumber(100),GetRandomNumber(100),GetRandomNumber(100));
	printf("\nInitializing insides of %s...", kGameName);
	fflush(stdout);
	/*      Stopwatch(&sec,&usec);*/
	Stopwatch();
	InitializeDatabases();
	printf("done in %d seconds!", Stopwatch()); // for analysis bookkeeping
	fflush(stdout);
	Stopwatch();
	gPrintDatabaseInfo = TRUE;
	gameValue = DetermineValue(gInitialPosition);
	printf("done in %d seconds!\e[K", gTimer = Stopwatch()); /* Extra Spacing to Clear Status Printing */
	if((Remoteness(gInitialPosition)) == REMOTENESS_MAX){
	printf("\n\nThe Game %s has value: Draw\n\n", kGameName);
	}else{
printf("\n\nThe Game %s has value: %s in %d\n\n", kGameName, gValueString[(int)gameValue],Remoteness(gInitialPosition));
	}
	
	gMenuMode = Evaluated;
	if(gameValue == lose)
	    gHumanGoesFirst = FALSE;
	else
	    gHumanGoesFirst = TRUE ;
	HitAnyKeyToContinue();
	break;
    case 'w': case 'W':
	InitializeGame();
	gTwoBits = TRUE;
	InitializeVisitedArray();
	gUnsolved = TRUE;
	gAgainstComputer = FALSE;
	gPrintPredictions = FALSE;
	sprintf(gPlayerName[kPlayerOneTurn],"Player 1");
	sprintf(gPlayerName[kPlayerTwoTurn],"Player 2");
	printf("\n\nYou have chosen to play the game without solving.  Have fun!\n\n");
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
    char tmpName[MAXNAME];
    
    switch(c) {
    case '1':
	printf("\nEnter the name of player 1 (max. %d chars) [%s] : ",
	       MAXNAME-1, gPlayerName[kPlayerOneTurn]);
	GetMyString(tmpName,MAXNAME,TRUE,FALSE);
	if(strcmp(tmpName,""))
	    (void) sprintf(gPlayerName[kPlayerOneTurn],"%s",tmpName);
	break;
    case '2':
	printf("\nEnter the name of player 2 (max. %d chars) [%s] : ",
	       MAXNAME-1, gPlayerName[kPlayerTwoTurn]);
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
	if(gUnsolved) {
	    BadMenuChoice();
	    HitAnyKeyToContinue();
	}
	gPrintPredictions = !gPrintPredictions;
	break;
    case '5':
	if(gUnsolved) {
	    BadMenuChoice();
	    HitAnyKeyToContinue();
	}
	gHints = !gHints;
	break;
    case '6':
	if(gUnsolved) {
	    BadMenuChoice();
	    HitAnyKeyToContinue();
	}
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
    case 'C': case 'c':
	SmarterComputerMenu();
	break;
    case 'A': case 'a':
      analyze();
	AnalysisMenu();
	break;
    case 'p': case 'P':
	if(gAgainstComputer)
	    PlayAgainstComputer();
	else
	    PlayAgainstHuman();
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
	
        printf("\n\n\tb)\t(B)ack = Return to previous activity.\n");
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
	    if(gAgainstComputer)
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
void SmarterComputerMenu()
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
}

void AnalysisMenu()
{
    POSITIONLIST *badWinPositions = NULL, *badTiePositions = NULL, *badLosePositions = NULL;
    BOOLEAN tempPredictions = gPrintPredictions, CorruptedValuesP();
    MEX mexValue = 0;
    int mexInt, maxPositions = 10;
    char c;
    
    gPrintPredictions = FALSE;
    
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
	case 'i': case 'I':
	    printf("\n\t----- The Initial Position is shown below -----\n");
	    PrintPosition(gInitialPosition, "Nobody", TRUE);
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
	    gPrintPredictions = tempPredictions;
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
    char tmpAns[2], input[MAXINPUTLENGTH];
    
    GetMyString(input,MAXINPUTLENGTH,TRUE,TRUE);
    
    if(input[0] == '\0')
        PrintPossibleMoves(thePosition);
    
    else if (ValidTextInput(input)) {
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
	    GetMyString(tmpAns,2,TRUE,TRUE);
	    printf("\n");
	    if(tmpAns[0] == 'y' || tmpAns[0] == 'Y')
		return(Abort);
	    else
		return(Continue);
	case 'H': case 'h':
	    HelpMenus();
	    printf("");
	    PrintPosition(thePosition, playerName, TRUE);
	    break;
	case 'c': case 'C':
	    SmarterComputerMenu();
	    break;
	case 'r': case 'R':
	    PrintPosition(thePosition, playerName, TRUE);
	    break;
	case 's': case 'S':
	    PrintValueMoves(thePosition);
	    break;
	case 'p': case 'P':
	    gPrintPredictions = !gPrintPredictions;
	    printf("\n Predictions %s\n", gPrintPredictions ? "On." : "Off.");
	    PrintPosition(thePosition, playerName, TRUE);
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
    
    return(Continue);  /* The default action is to return Continue */
}


void GetMyString(char* name, int size, BOOLEAN eatFirstChar, BOOLEAN putCarraigeReturnBack)
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

/* Status Meter */
void showStatus(int done)
{
    static POSITION num_pos_seen = 0;
    static float timeDelayTicks = CLOCKS_PER_SEC / 10;
    static clock_t updateTime = (clock_t) NULL;
    int print_length=0;
    
    if (updateTime == (clock_t) NULL)
	{
	    updateTime = clock() + timeDelayTicks; /* Set Time for the First Time */
	}
    
    switch (done)
	{
	case 0:
	    num_pos_seen++;
	    break;
	case 1:
	    print_length = printf("Writing Database...\e[K");
	    printf("\e[%dD",print_length - 3); /* 3 Characters for the escape sequence */
	    num_pos_seen = 0;
	    updateTime = (clock_t) NULL;
	    return;
	}	
    
    if (num_pos_seen > gNumberOfPositions && clock() > updateTime)
	{
	    fflush(stdout);
	    print_length = printf("Solving... %d Positions Visited - Reported Total Number of Positions: %d\e[K",num_pos_seen,gNumberOfPositions);
	    printf("\e[%dD",print_length - 3); /* 3 Characters for the escape sequence */
	    updateTime = clock() + timeDelayTicks; /* Get the Next Update Time */
	}
    else if (clock() > updateTime)
	{
	    fflush(stdout);
	    print_length = printf("%2.1f%% Done \e[K",(float)num_pos_seen/(float)gNumberOfPositions * 100.0);
	    printf("\e[%dD",print_length - 3); /* 3 Characters for the escape sequence */
	    updateTime = clock() + timeDelayTicks; /* Get the Next Update Time */
	}
}
