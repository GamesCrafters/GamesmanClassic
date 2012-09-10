#ifndef GMCORE_CONSTANTS_H
#define GMCORE_CONSTANTS_H

/* constant definitions and declarations */

#ifndef  FALSE
#define  FALSE                  0
#endif

#ifndef  TRUE
#define  TRUE                   1
#endif

#define POSITION_FORMAT         "%llu"

/* smarter computer */
#define WINMOVE                 0
#define TIEMOVE                 1
#define LOSEMOVE                2
#define SMART                   0
#define RANDOM                  1
#define DUMB                    2
#define MAXSCALE                100
#define MAXGIVEBACKS            9

#define MAXINT2                 1073741823 /* 2^30 - 1 */
#define MAXNAME                 15         /* arbitrary */
#define MAXINPUTLENGTH          80         /* arbitrary */

#define kPlayerOneTurn          1 /* used for human/human games */
#define kPlayerTwoTurn          0
#define kHumansTurn             1 /* used for computer/human games */
#define kComputersTurn          0

extern STRING kOpeningCredits,
              kHelpValueBeforeEvaluation,
              kHelpWhatIsGameValue,
              kHelpWhatIsEvaluation,
              kHelpWhatArePredictions,
              kHelpWhatAreHints,
              kHandleDefaultTextInputHelp,
              kCommandSyntaxHelp,
              kSolveVersion,
              kPrintMenu,
              kPrintMenuWithSolving,
              kPrintMenuEnd;

extern STRING gValueString[];
extern char gValueLetter[];

#endif /* GMCORE_CONSTANTS_H */
