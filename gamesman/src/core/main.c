
/* solver function pointer */
VALUE (*gSolver)(POSITION);

/* go again function pointer */
BOOLEAN (*gGoAgain)(POSITION,MOVE);

static VALUE   gValue = undecided;          /* The value of the game */
static BOOLEAN gAgainstComputer = TRUE;     /* TRUE iff the user is playing the computer */
static BOOLEAN gHumanGoesFirst;             /* TRUE iff the user goes first vs. computer */
static BOOLEAN gPrintPredictions = TRUE;    /* TRUE iff the predictions should be printed */
static BOOLEAN gHints = FALSE;              /* TRUE iff possible moves should be printed */
static BOOLEAN gUnsolved = FALSE;           /* TRUE iff playing without solving */

extern STRING kAuthorName;

BOOLEAN gStandardGame = TRUE;               /* TRUE iff game is STANDARD (not REVERSE) */
BOOLEAN gWriteDatabase = TRUE;    /* Default is to write the database */
BOOLEAN gReadDatabase = TRUE;     /* Default is to read the database if it exists */
BOOLEAN gPrintDatabaseInfo = FALSE; /* Print to the console */
BOOLEAN gJustSolving = FALSE;     /* Default is playing game, not just solving*/
BOOLEAN gMessage = FALSE;         /* Default is no message */
BOOLEAN gSolvingAll = FALSE;      /* Default is to not solve all */
BOOLEAN gTwoBits = FALSE;	      /* Two bit solver, default: FALSE */
BOOLEAN kZeroMemSolver = FALSE;	  /* Zero Memory Overhead Solver, default: FALSE */
BOOLEAN gAnalyzing = FALSE;       /* Write analysis for each variant 
				   * solved, default: FALSE */

char    gPlayerName[2][MAXNAME] = {"", ""}; /* The names of the players user/user or comp/user */
VALUE * gDatabase = NULL;
char *  gVisited = NULL;
STRING  kSolveVersion = "2004.05.05" ;

int   smartness = SMART;
int   scalelvl = MAXSCALE;
int   remainingGivebacks = 0;
int   initialGivebacks = 0;
VALUE oldValueOfPosition = tie;

static MENU gMenuMode = BeforeEvaluation;
BOOLEAN gPrintHints = TRUE;




void InitializeDatabases()
{
    POSITION i;
    if (gDatabase) {
        SafeFree((GENERIC_PTR) gDatabase);
        gDatabase = NULL;
    }
    
    if (gTwoBits) {	/* VALUE is always 32 bits */
        size_t dbSize = sizeof(VALUE) * (1 + gNumberOfPositions / (sizeof(VALUE) * 4));
        gDatabase = (VALUE *) SafeMalloc (dbSize);
        memset(gDatabase, 0xff, dbSize);	/* This _ASSUMES_ undecided = 3 */
    } else {
        gDatabase = (VALUE *) SafeMalloc (gNumberOfPositions * sizeof(VALUE));
        for(i = 0; i < gNumberOfPositions; i++)
            gDatabase[i] = undecided;
    }
}

void Initialize()
{
    srand(time(NULL));
    
    /* set default solver */
    gSolver = NULL;
    
    /* set default go again */
    gGoAgain=DefaultGoAgain;
    
    sprintf(gPlayerName[kPlayerOneTurn],"Player");
    sprintf(gPlayerName[kPlayerTwoTurn],"Computer");
    InitializeGame();
    SetSolver();
}

void SetSolver() 
{
    
    /* if solver set externally, leave alone */
    if (gSolver != NULL)
        return;
    
    if(kZeroMemSolver)
        gSolver = DetermineZeroValue;
    
    else if(kLoopy) {
        if (gGoAgain == DefaultGoAgain)
            gSolver = DetermineLoopyValue;
        else
            gSolver = lgas_DetermineValue;
    }

    else
        gSolver = DetermineValue1;
}




/* Starts a normal textbased game. */
void StartGame()
{
    Initialize();
    Menus();
}

/* Solves the game and stores it, without anybody actually playing it */
void SolveAndStore()
{
    Initialize();
    InitializeDatabases();
    DetermineValue(gInitialPosition);
    // analysis
    if (gAnalyzing) {
        analyze(); // sets global variables
        createAnalysisVarDir();
        writeVarHTML();
    }
}

/* Handles the command line arguments by setting flags and options */
void HandleArguments (int argc, char *argv[])
{
    int i, option;
    for(i = 1; i < argc; i++) {
        if(!strcasecmp(argv[i], "--nodb")) {
            gWriteDatabase = FALSE;
            gReadDatabase = FALSE;
        }
        else if(!strcasecmp(argv[i], "--newdb"))
            gReadDatabase = FALSE;
        else if(!strcasecmp(argv[i], "--numoptions")) {
            fprintf(stderr, "\nNumber of Options: %d\n", NumberOfOptions());
            gMessage = TRUE;
        }
        else if(!strcasecmp(argv[i], "--curroption")) {
            fprintf(stderr, "\nCurrent Option: %d\n", getOption());
            gMessage = TRUE;
        }
        else if(!strcasecmp(argv[i], "--option")) {
            if(argc < (i + 2)) {
                fprintf(stderr, "\nUsage: %s --option <n>\n\n", argv[0]);
                gMessage = TRUE;
            }
            else {
                option = atoi(argv[++i]);
                if(!option || option > NumberOfOptions()) {
                    fprintf(stderr, "\nInvalid option configuration!\n\n");
                    gMessage = TRUE;
                }
                else
                    setOption(option);
            }
        }
        else if(!strcasecmp(argv[i], "--2bit")) {
            gTwoBits = TRUE;
        }
        else if(!strcasecmp(argv[i], "--lowmem")) {
            kZeroMemSolver = TRUE;
        }
        else if(!strcasecmp(argv[i], "--solve")) {
            gJustSolving = TRUE;
            if((i + 1) < argc && !strcasecmp(argv[++i], "all"))
                gSolvingAll = TRUE;
            option = atoi(argv[i]);
            if(option > NumberOfOptions()) {
                fprintf(stderr, "Invalid option configuration!\n\n");
                gMessage = TRUE;
            }
            else if(option)
                setOption(option);
        }
        else if(!strcasecmp(argv[i], "--analyze")) {
            gJustSolving = TRUE;
            gAnalyzing = TRUE;
            gSolvingAll = TRUE;
            createAnalysisGameDir();
            writeGameHTML();
            createVarTable();
        }
        else if(!strcasecmp(argv[i], "--DoMove")) {
            InitializeGame();
            if(argc != 4)
                fprintf(stderr, "\nInvalid arguments!\n\n");
            else
                printf("\nDoMove returns: %u\n\n", DoMove(atoi(argv[2]), atoi(argv[3])));
            i += argc;
            gMessage = TRUE;
        }
        else if(!strcasecmp(argv[i], "--Primitive")) {
            InitializeGame();
            if(argc != 3)
                fprintf(stderr, "\nInvalid arguments!\n\n");
            else 
                printf("\nPrimitive returns: %u\n\n", Primitive(atoi(argv[2])));
            i += argc;
            gMessage = TRUE;
        }
        else if(!strcasecmp(argv[i], "--PrintPosition")) {
            InitializeGame();
            if(argc != 5)
                fprintf(stderr, "\nInvalid arguments!\n\n");
            else {
                printf("\nPrintPosition:\n\n");
                PrintPosition(atoi(argv[2]), argv[3], atoi(argv[4]));
            }
            i += argc;
            gMessage = TRUE;
        }
        else if(!strcasecmp(argv[i], "--GenerateMoves")) {
            i += argc;
            gMessage = TRUE;
        }
        else if(!strcasecmp(argv[i], "--help")) {
            printf("\nSyntax:\n"
		   "%s {--nodb | --newdb | --curroption | --numoptions |"
		   "\t\t --option <n> | --solve [<n> | <all>] | --analyze [ <linkname> ]|\n"
		   "\t\t--DoMove <args> <move> | --Primitive <args> | \n"
		   "\t\t--PrintPosition <args> --GenerateMoves <args>} | --help}\n\n"
		   "--nodb\t\t\tStarts game without loading or saving to the database.\n"
		   "--newdb\t\t\tStarts game and clobbers the old database.\n"
		   "--numoptions\t\tPrints the number of options.\n"
		   "--curroption\t\tPrints the current option.\n"
		   "--option <n>\t\tStarts game with the n option configuration.\n"
		   "--solve [<n> | <all>]\tSolves game with the n option configuration.\n"
		   "--2bit\t\t\tStarts game with two-bit solving enabled.\n"
		   "--lowmem\t\tStarts game with low memory overhead solver enabled.\n"
		   "\t\t\tTo solve all option configurations of game, use <all>.\n"
		   "\t\t\tIf <n> and <all> are ommited, it will solve the default\n"
		   "\t\t\tconfiguration.\n"
		   "\t\t\tExamples:\n"
		   "\t\t\t%s --solve\n"
		   "\t\t\t%s --solve 2\n"
		   "\t\t\t%s --solve all\n"
		   "--analyze\t\tCreates the analysis directory with info on all variants\n"
		   "--DoMove <args>\n"
		   "--Primitive <args>\n"
		   "--PrintPosition <args>\n"
		   "--GenerateMoves <args>\n\n", argv[0], argv[0], argv[0], argv[0]);
            gMessage = TRUE;
            i += argc;
        }
        else {
            fprintf(stderr, "\nInvalid option or missing parameter, use %s --help for help\n\n", argv[0]);
            gMessage = TRUE;
            i += argc;
        }
    }
}

int main(int argc, char *argv[])
{
    HandleArguments(argc, argv);
    
    
    if(!gMessage) {
        if(!gJustSolving)
            StartGame();
        else if(!gSolvingAll) {
            fprintf(stderr, "Solving \"%s\" option %u....", kGameName, getOption());
            fflush(stderr);
            SolveAndStore();
            fprintf(stderr, "done.\n");
        }
        else {
            int i;
            fprintf(stderr, "Solving \"%s\" option ", kGameName);
            for(i = 1; i <= NumberOfOptions(); i++) {
                fprintf(stderr, "%c[s%u of %u....", 27, i, NumberOfOptions());
                fflush(stderr);
                setOption(i);
                SolveAndStore();
                fprintf(stderr, "%c[u", 27);
            }
            fprintf(stderr, "%u of %u....done.\n", i - 1, NumberOfOptions());
        }
    }
    return 0;
}
