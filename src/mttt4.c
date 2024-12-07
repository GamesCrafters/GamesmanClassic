/************************************************************************
**
** NAME:        mttt4.c
**
** DESCRIPTION: Four Square Tic-Tac-Toe
**
** AUTHOR:      Edward Cheng and Ijin Yu  -  University of California at Berkeley
**              Copyright (C) Dan Garcia, 1995. All rights reserved.
**
** DATE:        10/24/2024
**
** UPDATE HIST:
**
**************************************************************************/


/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/


#include "gamesman.h"


POSITION gNumberOfPositions  = 43046721;  /* 3^16 */
POSITION kBadPosition        = -1;


POSITION gInitialPosition    =  0;
POSITION gMinimalPosition    =  0;


CONST_STRING kAuthorName         = "Edward Cheng and Ijin Yu";
CONST_STRING kGameName           = "Four Square Tic-Tac-Toe";
CONST_STRING kDBName = "ttt4";
BOOLEAN kPartizan           = TRUE;
BOOLEAN kDebugMenu          = TRUE;
BOOLEAN kGameSpecificMenu   = TRUE;
BOOLEAN kTieIsPossible      = TRUE;
BOOLEAN kLoopy               = FALSE;
BOOLEAN kDebugDetermineValue = FALSE;


BOOLEAN Diamond = FALSE;


void*    gGameSpecificTclInit = NULL;


CONST_STRING kHelpGraphicInterface =
        "The LEFT button puts an X or O (depending on whether you went first\n\
or second) on the spot the cursor was on when you clicked. The MIDDLE\n\
button does nothing, and the RIGHT button is the same as UNDO, in that\n\
it reverts back to your your most recent position."                                                                                                                                                                                                                                   ;


CONST_STRING kHelpTextInterface    =
        "On your turn, use the LEGEND to determine which number to choose (between\n\
1 and 16, with 1 at the upper left and 16 at the lower right) to correspond\n\
to the empty board position you desire and hit return. If at any point\n\
you have made a mistake, you can type u and hit return and the system will\n\
revert back to your most recent position."                                                                                                                                                                                                                                                                                                                           ;


CONST_STRING kHelpOnYourTurn =
        "You place one of your pieces on one of the empty board positions.";


CONST_STRING kHelpStandardObjective =
        "To get four of your markers (either X or O) in a row, either\n\
horizontally, vertically, or diagonally. Occupying each corner or\n\
any 2x2 square also wins. Enable Diamond option to add occupying 4 squares in a dimaond-shape as a win condition."                                                                          ;


CONST_STRING kHelpReverseObjective =
        "To force your opponent into getting four of his markers (either X or\n\
O) in a row, either horizontally, vertically, or diagonally. Forcing your opponent to\n\
occupy each corner or any 2x2 square also loses. Enable Diamond option to add occupying 4 squares in a dimaond-shape as a lose condition."                                                                                                                                                           ;


CONST_STRING kHelpTieOccursWhen =   /* Should follow 'A Tie occurs when... */
                            "The board fills up without either player getting four-in-a-row, one in each corner, or a two-by-two square.";


CONST_STRING kHelpExample =
        "         ( 1  2  3  4  )           : - - - -\n\
LEGEND:  ( 5  6  7  8  )  TOTAL:   : - - - - \n\
         ( 9  10 11 12 )           : - - - - \n\
         ( 13 14 15 16 )           : - - - - \n\n\
Computer's move              :  3    \n\n\
         ( 1  2  3  4  )           : - - O - \n\
LEGEND:  ( 5  6  7  8  )  TOTAL:   : - - - - \n\
         ( 9  10 11 12 )           : - - - - \n\
         ( 13 14 15 16 )           : - - - - \n\n\
     Dan's move [(u)ndo/1-9] : { 2 } \n\n\
         ( 1  2  3  4  )           : - X O - \n\
LEGEND:  ( 5  6  7  8  )  TOTAL:   : - - - - \n\
         ( 9  10 11 12 )           : - - - - \n\
         ( 13 14 15 16 )           : - - - - \n\n\
Computer's move              :  7    \n\n\
         ( 1  2  3  4  )           : - X O - \n\
LEGEND:  ( 5  6  7  8  )  TOTAL:   : - - O - \n\
         ( 9  10 11 12 )           : - - - - \n\
         ( 13 14 15 16 )           : - - - - \n\n\
     Dan's move [(u)ndo/1-9] : { 10 } \n\n\
         ( 1  2  3  4  )           : - X O - \n\
LEGEND:  ( 5  6  7  8  )  TOTAL:   : - - O - \n\
         ( 9  10 11 12 )           : - X - - \n\
         ( 13 14 15 16 )           : - - - - \n\n\
Computer's move              :  4    \n\n\
         ( 1  2  3  4  )           : - X O O \n\
LEGEND:  ( 5  6  7  8  )  TOTAL:   : - - O - \n\
         ( 9  10 11 12 )           : - X - - \n\
         ( 13 14 15 16 )           : - - - - \n\n\
     Dan's move [(u)ndo/1-9] : { 5 } \n\n\
         ( 1  2  3  4  )           : - X O O \n\
LEGEND:  ( 5  6  7  8  )  TOTAL:   : X - O - \n\
         ( 9  10 11 12 )           : - X - - \n\
         ( 13 14 15 16 )           : - - - - \n\n\
Computer's move              :  8    \n\n\
         ( 1  2  3  4  )           : - X O O \n\
LEGEND:  ( 5  6  7  8  )  TOTAL:   : X - O O \n\
         ( 9  10 11 12 )           : - X - - \n\
         ( 13 14 15 16 )           : - - - - \n\n\
Computer wins. Nice try, Dan."                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             ;


/*************************************************************************
**
** Everything above here must be in every game file
**
**************************************************************************/


/*************************************************************************
**
** Every variable declared here is only used in this file (game-specific)
**
**************************************************************************/


#define BOARDSIZE     16           /* 4x4 board */
#define BOARDROWS     4
#define BOARDCOLS     4


typedef enum possibleBoardPieces {
    Blank, o, x
} BlankOX;


char *gBlankOXString[] = { "-", "O", "X" };


/* Powers of 3 - this is the way I encode the position, as an integer */
int g3Array[] = { 1, 3, 9, 27, 81, 243, 729, 2187, 6561, 19683, 59049, 177147, 531441, 1594323, 4782969, 14348907};


/* Global position solver variables.*/
struct {
    BlankOX board[BOARDSIZE];
    BlankOX nextPiece;
    int piecesPlaced;
} gPosition;


/** Function Prototypes **/
BOOLEAN AllFilledIn(BlankOX theBlankOX[]);
POSITION BlankOXToPosition(BlankOX *theBlankOX);
POSITION GetCanonicalPosition(POSITION position);
void PositionToBlankOX(POSITION thePos,BlankOX *theBlankOX);
BOOLEAN FourOfTheSame(BlankOX[], int, int, int, int);
void UndoMove(MOVE move);
BlankOX WhoseTurn(BlankOX *theBlankOX);
int tttppm(int x, int y);


POSITION ActualNumberOfPositions(int variant);


/**************************************************/
/**************** SYMMETRY FUN BEGIN **************/
/**************************************************/


BOOLEAN kSupportsSymmetries = FALSE; /* Whether we support symmetries */



/************************************************************************
**
** NAME:        InitializeDatabases
**
** DESCRIPTION: Initialize the gDatabase, a global variable.
**
************************************************************************/


void InitializeGame() {
    //Ignore all Symmetry


    PositionToBlankOX(gInitialPosition, gPosition.board);
    gPosition.nextPiece = x;
    gPosition.piecesPlaced = 0;
    gUndoMove = UndoMove;
    gActualNumberOfPositionsOptFunPtr = &ActualNumberOfPositions;
}


/************************************************************************
**
** NAME:        DebugMenu
**
** DESCRIPTION: Menu used to debub internal problems. Does nothing if
**              kDebugMenu == FALSE
**
************************************************************************/


void DebugMenu()
{


    do {
        printf("\n\t----- Module DEBUGGER for %s -----\n\n", kGameName);


        printf("\tc)\tWrite PPM to s(C)reen\n");
        printf("\ti)\tWrite PPM to f(I)le\n");
        printf("\ts)\tWrite Postscript to (S)creen\n");
        printf("\tf)\tWrite Postscript to (F)ile\n");
        printf("\n\n\tb)\t(B)ack = Return to previous activity.\n");
        printf("\n\nSelect an option: ");


        switch(GetMyChar()) {
        case 'Q': case 'q':
            ExitStageRight();
            break;
        case 'H': case 'h':
            HelpMenus();
            break;
        case 'C': case 'c': /* Write PPM to s(C)reen */
            //tttppm(0,0);
            break;
        case 'I': case 'i': /* Write PPM to f(I)le */
            //tttppm(0,1);
            break;
        case 'S': case 's': /* Write Postscript to (S)creen */
            //tttppm(1,0);
            break;
        case 'F': case 'f': /* Write Postscript to (F)ile */
            //tttppm(1,1);
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


/************************************************************************
**
** NAME:        GameSpecificMenu
**
** DESCRIPTION: Menu used to change game-specific parmeters, such as
**              the side of the board in an nxn Nim board, etc. Does
**              nothing if kGameSpecificMenu == FALSE
**
************************************************************************/


void GameSpecificMenu() {
    BOOLEAN tryagain = TRUE;
    char c;
    char* diamondLabel;
    while (tryagain) {
        // Display whether diamond win condition is enabled or not
        diamondLabel = (Diamond) ? "ENABLED" : "DISABLED";
       
        // Menu options
        printf("\nHere is where you set game options.\n\n");
        printf("\td)\ttoggle (D)iamond win condition. It is currently %s\n", diamondLabel);
        printf("\tb)\t(B)ack to the main menu.\n\n");
        printf("Enter your selection: ");
       
        // Get user input
        c = GetMyChar();
       
        // Process input
        if ((c == 'D') || (c == 'd')) {
            // Toggle diamond win condition
            Diamond = (Diamond) ? FALSE : TRUE;
        } else if ((c == 'B') || (c == 'b')) {
            // Exit the options menu
            tryagain = FALSE;
        } else {
            // Invalid input
            printf("Invalid option, please try again.\n");
        }
    }
   
    InitializeGame();
}


// Anoto pen support - implemented in core/pen/pttt.c
extern void gPenHandleTclMessage(int options[], char *filename, Tcl_Interp *tclInterp, int debug);


/************************************************************************
**
** NAME:        SetTclCGameSpecificOptions
**
** DESCRIPTION: Set the C game-specific options (called from Tcl)
**              Ignore if you don't care about Tcl for now.
**
************************************************************************/


#ifndef NO_GRAPHICS
void SetTclCGameSpecificOptions(int theOptions[]) {
    // Anoto pen support
    if ((gPenFile != NULL) && (gTclInterp != NULL)) {
        gPenHandleTclMessage(theOptions, gPenFile, gTclInterp, gPenDebug);
    }
}
#endif


/************************************************************************
**
** NAME:        DoMove
**
** DESCRIPTION: Apply the move to the position.
**
** INPUTS:      POSITION position : The old position
**              MOVE     move     : The move to apply.
**
** OUTPUTS:     (POSITION) : The position that results after the move.
**
** CALLS:       PositionToBlankOX(POSITION,*BlankOX)
**              BlankOX WhosTurn(*BlankOX)
**
************************************************************************/


POSITION DoMove(POSITION position, MOVE move) {
    if (gUseGPS) {
        gPosition.board[move] = gPosition.nextPiece;
        gPosition.nextPiece = gPosition.nextPiece == x ? o : x;
        ++gPosition.piecesPlaced;


        return BlankOXToPosition(gPosition.board);
    }
    else {
        BlankOX board[BOARDSIZE];


        PositionToBlankOX(position, board);


        return position + g3Array[move] * (int) WhoseTurn(board);
    }
}


void UndoMove(MOVE move) {
    gPosition.board[move] = Blank;
    gPosition.nextPiece = gPosition.nextPiece == x ? o : x;
    --gPosition.piecesPlaced;
}


/************************************************************************
**
** NAME:        PrintComputersMove
**
** DESCRIPTION: Nicely format the computers move.
**
** INPUTS:      MOVE   *computersMove : The computer's move.
**              STRING  computersName : The computer's name.
**
************************************************************************/


void PrintComputersMove(MOVE computersMove, STRING computersName) {
    printf("%8s's move              : %2d\n", computersName, computersMove+1);
}


/************************************************************************
**
** NAME:        Primitive
**
** DESCRIPTION: Return the value of a position if it fulfills certain
**              'primitive' constraints. Some examples of this is having
**              three-in-a-row with TicTacToe. TicTacToe has two
**              primitives it can immediately check for, when the board
**              is filled but nobody has one = primitive tie. Three in
**              a row is a primitive lose, because the player who faces
**              this board has just lost. I.e. the player before him
**              created the board and won. Otherwise undecided.
**
** INPUTS:      POSITION position : The position to inspect.
**
** OUTPUTS:     (VALUE) an enum which is oneof: (win,lose,tie,undecided)
**
** CALLS:       BOOLEAN FourOfTheSame()
**              BOOLEAN AllFilledIn()
**              PositionToBlankOX()
**
************************************************************************/


VALUE Primitive(POSITION position) {
    if (!gUseGPS)
        PositionToBlankOX(position, gPosition.board); // Temporary storage.


    if (Diamond) {
        if (FourOfTheSame(gPosition.board, 1, 4, 6, 9) ||
            FourOfTheSame(gPosition.board, 2, 5, 7, 10) ||
            FourOfTheSame(gPosition.board, 5, 8, 10, 13) ||
            FourOfTheSame(gPosition.board, 6, 9, 11, 14) ||
            FourOfTheSame(gPosition.board, 1, 7, 8, 14) || //Tilted
            FourOfTheSame(gPosition.board, 2, 4, 11, 13)) // Tilted
            return gStandardGame ? lose : win;
    }


    if (FourOfTheSame(gPosition.board, 0, 1, 2, 3) ||
        FourOfTheSame(gPosition.board, 4, 5, 6, 7) ||
        FourOfTheSame(gPosition.board, 8, 9, 10, 11) ||
        FourOfTheSame(gPosition.board, 12, 13, 14, 15) ||
        FourOfTheSame(gPosition.board, 0, 4, 8, 12) ||
        FourOfTheSame(gPosition.board, 1, 5, 9, 13) ||
        FourOfTheSame(gPosition.board, 2, 6, 10, 14) ||
        FourOfTheSame(gPosition.board, 3, 7, 11, 15) ||
        //Diagonals
        FourOfTheSame(gPosition.board, 0, 5, 10, 15) ||
        FourOfTheSame(gPosition.board, 3, 6, 9, 12) ||
        //Corners
        FourOfTheSame(gPosition.board, 0, 3, 12, 15) ||
        //Blocks of 4
        FourOfTheSame(gPosition.board, 0, 1, 4, 5) ||
        FourOfTheSame(gPosition.board, 1, 2, 5, 6) ||
        FourOfTheSame(gPosition.board, 2, 3, 6, 7) ||
        FourOfTheSame(gPosition.board, 4, 5, 8, 9) ||
        FourOfTheSame(gPosition.board, 5, 6, 9, 10) ||
        FourOfTheSame(gPosition.board, 6, 7, 10, 11) ||
        FourOfTheSame(gPosition.board, 8, 9, 12, 13) ||
        FourOfTheSame(gPosition.board, 9, 10, 13, 14) ||
        FourOfTheSame(gPosition.board, 10, 11, 14, 15))
        return gStandardGame ? lose : win;
    else if ((gUseGPS && (gPosition.piecesPlaced == BOARDSIZE)) ||
             ((!gUseGPS) && AllFilledIn(gPosition.board)))
        return tie;
    else
        return undecided;
}


/************************************************************************
**
** NAME:        PrintPosition
**
** DESCRIPTION: Print the position in a pretty format, including the
**              prediction of the game's outcome.
**
** INPUTS:      POSITION position   : The position to pretty print.
**              STRING   playerName : The name of the player.
**              BOOLEAN  usersTurn  : TRUE <==> it's a user's turn.
**
** CALLS:       PositionToBlankOX()
**              GetValueOfPosition()
**              GetPrediction()
**
************************************************************************/


void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn) {
    BlankOX theBlankOx[BOARDSIZE];


    PositionToBlankOX(position,theBlankOx);


    printf("\n        ( 1  2  3  4  )       : %s %s %s %s\n",
           gBlankOXString[(int)theBlankOx[0]],
           gBlankOXString[(int)theBlankOx[1]],
           gBlankOXString[(int)theBlankOx[2]],
           gBlankOXString[(int)theBlankOx[3]] );
    printf("LEGEND: ( 5  6  7  8  ) TOTAL : %s %s %s %s\n",
           gBlankOXString[(int)theBlankOx[4]],
           gBlankOXString[(int)theBlankOx[5]],
           gBlankOXString[(int)theBlankOx[6]],
           gBlankOXString[(int)theBlankOx[7]] );
    printf("        ( 9  10 11 12 )       : %s %s %s %s\n",
           gBlankOXString[(int)theBlankOx[8]],
           gBlankOXString[(int)theBlankOx[9]],
           gBlankOXString[(int)theBlankOx[10]],
           gBlankOXString[(int)theBlankOx[11]] );
    printf("        ( 13 14 15 16 )       : %s %s %s %s %s\n\n",
           gBlankOXString[(int)theBlankOx[12]],
           gBlankOXString[(int)theBlankOx[13]],
           gBlankOXString[(int)theBlankOx[14]],
           gBlankOXString[(int)theBlankOx[15]],
           GetPrediction(position,playerName,usersTurn));
}


/************************************************************************
**
** NAME:        GenerateMoves
**
** DESCRIPTION: Create a linked list of every move that can be reached
**              from this position. Return a pointer to the head of the
**              linked list.
**
** INPUTS:      POSITION position : The position to branch off of.
**
** OUTPUTS:     (MOVELIST *), a pointer that points to the first item
**              in the linked list of moves that can be generated.
**
** CALLS:       MOVELIST *CreateMovelistNode(MOVE,MOVELIST *)
**
************************************************************************/


MOVELIST *GenerateMoves(POSITION position) {
    int index;
    MOVELIST *moves = NULL;


    if (!gUseGPS)
        PositionToBlankOX(position, gPosition.board); // Temporary storage.


    for (index = 0; index < BOARDSIZE; ++index)
        if (gPosition.board[index] == Blank)
            moves = CreateMovelistNode(index, moves);


    return moves;
}


/**************************************************/
/**************** SYMMETRY FUN BEGIN **************/
/**************************************************/


/************************************************************************
**
** NAME:        DoSymmetry
**
** DESCRIPTION: Perform the symmetry operation specified by the input
**              on the position specified by the input and return the
**              new position, even if it's the same as the input.
**
** INPUTS:      POSITION position : The position to branch the symmetry from.
**              int      symmetry : The number of the symmetry operation.
**
** OUTPUTS:     POSITION, The position after the symmetry operation.
**
************************************************************************/


POSITION DoSymmetry(POSITION position, int symmetry) {
    int i;
    BlankOX theBlankOx[BOARDSIZE], symmBlankOx[BOARDSIZE];


    PositionToBlankOX(position,theBlankOx);
    PositionToBlankOX(position,symmBlankOx); /* Make copy */


    /* Copy from the symmetry matrix */


    for(i = 0; i < BOARDSIZE; i++)
        symmBlankOx[i] = theBlankOx[gSymmetryMatrix[symmetry][i]];


    return(BlankOXToPosition(symmBlankOx));
}


/************************************************************************
**
** NAME:        GetCanonicalPosition
**
** DESCRIPTION: Go through all of the positions that are symmetrically
**              equivalent and return the SMALLEST, which will be used
**              as the canonical element for the equivalence set.
**
** INPUTS:      POSITION position : The position return the canonical elt. of.
**
** OUTPUTS:     POSITION          : The canonical element of the set.
**
************************************************************************/


POSITION GetCanonicalPosition(POSITION position) {
    POSITION newPosition, theCanonicalPosition;
    int i;


    theCanonicalPosition = position;


    for(i = 0; i < NUMSYMMETRIES; i++) {


        newPosition = DoSymmetry(position, i); /* get new */
        if(newPosition < theCanonicalPosition) /* THIS is the one */
            theCanonicalPosition = newPosition; /* set it to the ans */
    }


    return(theCanonicalPosition);
}


/**************************************************/
/**************** SYMMETRY FUN END ****************/
/**************************************************/


/************************************************************************
**
** NAME:        GetAndPrintPlayersMove
**
** DESCRIPTION: This finds out if the player wanted an undo or abort or not.
**              If so, return Undo or Abort and don't change theMove.
**              Otherwise get the new theMove and fill the pointer up.
**
** INPUTS:      POSITION *thePosition : The position the user is at.
**              MOVE *theMove         : The move to fill with user's move.
**              STRING playerName     : The name of the player whose turn it is
**
** OUTPUTS:     USERINPUT             : Oneof( Undo, Abort, Continue )
**
** CALLS:       ValidMove(MOVE, POSITION)
**              BOOLEAN PrintPossibleMoves(POSITION) ...Always True!
**
************************************************************************/


USERINPUT GetAndPrintPlayersMove(POSITION thePosition, MOVE *theMove, STRING playerName) {
    USERINPUT ret;


    do {
        printf("%8s's move [(u)ndo/1-16] : ", playerName);


        ret = HandleDefaultTextInput(thePosition, theMove, playerName);
        if(ret != Continue)
            return(ret);


    }
    while (TRUE);
    return(Continue); /* this is never reached, but lint is now happy */
}


/************************************************************************
**
** NAME:        ValidTextInput
**
** DESCRIPTION: Return TRUE iff the string input is of the right 'form'.
**              For example, if the user is allowed to select one slot
**              from the numbers 1-9, and the user chooses 0, it's not
**              valid, but anything from 1-9 IS, regardless if the slot
**              is filled or not. Whether the slot is filled is left up
**              to another routine.
**
** INPUTS:      STRING input : The string input the user typed.
**
** OUTPUTS:     BOOLEAN : TRUE iff the input is a valid text input.
**
************************************************************************/


BOOLEAN ValidTextInput(STRING input) {
    char* end;
    long number = strtol(input, &end, 10);  // Convert input to integer


    // Check if conversion was successful (end points to '\0') and number is in the range 1 to 16
    return (*end == '\0' && number >= 1 && number <= 16);
}


/************************************************************************
**
** NAME:        ConvertTextInputToMove
**
** DESCRIPTION: Convert the string input to the internal move representation.
**
** INPUTS:      STRING input : The string input the user typed.
**
** OUTPUTS:     MOVE : The move corresponding to the user's input.
**
************************************************************************/


MOVE ConvertTextInputToMove(STRING input) {
    char* end;
    long number = strtol(input, &end, 10);
    return((MOVE) number - 1); /* user input is 1-16, our rep. is 0-15 */
}


/************************************************************************
**
** NAME:        MoveToString
**
** DESCRIPTION: Writes string representation of move to moveStringBuffer
**
** INPUTS:      MOVE *move         : The move to put into a string.
**              char *moveStringBuffer : Buffer to write movestring to
**
************************************************************************/


void MoveToString(MOVE move, char *moveStringBuffer) {
    /* The plus 1 is because the user thinks it's 1-16, but MOVE is 0-15 */
    sprintf(moveStringBuffer, "%d", move + 1);
}


/************************************************************************
*************************************************************************
** BEGIN   FUZZY STATIC EVALUATION ROUTINES. DON'T WORRY ABOUT UNLESS
**         YOU'RE NOT GOING TO EXHAUSTIVELY SEARCH THIS GAME
*************************************************************************
************************************************************************/


/************************************************************************
*************************************************************************
**         EVERYTHING BELOW THESE LINES IS LOCAL TO THIS FILE
*************************************************************************
************************************************************************/


/************************************************************************
**
** NAME:        PositionToBlankOX
**
** DESCRIPTION: convert an internal position to that of a BlankOX.
**
** INPUTS:      POSITION thePos     : The position input.
**              BlankOX *theBlankOx : The converted BlankOX output array.
**
** CALLS:       BadElse()
**
************************************************************************/


void PositionToBlankOX(POSITION thePos, BlankOX *theBlankOX) {
    int i;
    for(i = BOARDSIZE - 1; i >= 0; i--) {
        if(thePos >= (POSITION)(x * g3Array[i])) {
            theBlankOX[i] = x;
            thePos -= x * g3Array[i];
        }
        else if(thePos >= (POSITION)(o * g3Array[i])) {
            theBlankOX[i] = o;
            thePos -= o * g3Array[i];
        }
        else if(thePos >= (POSITION)(Blank * g3Array[i])) {
            theBlankOX[i] = Blank;
            thePos -= Blank * g3Array[i];
        }
        else
            BadElse("PositionToBlankOX");
    }
}


/************************************************************************
**
** NAME:        BlankOXToPosition
**
** DESCRIPTION: convert a BlankOX to that of an internal position.
**
** INPUTS:      BlankOX *theBlankOx : The converted BlankOX output array.
**
** OUTPUTS:     POSITION: The equivalent position given the BlankOX.
**
************************************************************************/


POSITION BlankOXToPosition(BlankOX *theBlankOX) {
    int i;
    POSITION position = 0;


    for(i = 0; i < BOARDSIZE; i++)
        position += g3Array[i] * (int)theBlankOX[i]; /* was (int)position... */


    return(position);
}


/************************************************************************
**
** NAME:        FourOfTheSame
**
** DESCRIPTION: Return TRUE iff there are three-in-a-row.
**
** INPUTS:      BlankOX theBlankOX[BOARDSIZE] : The BlankOX array.
**              int a,b,c                     : The 3 positions to check.
**
** OUTPUTS:     (BOOLEAN) TRUE iff there are three-in-a-row.
**
************************************************************************/


BOOLEAN FourOfTheSame(BlankOX *theBlankOX, int a, int b, int c, int d) {
    return(       theBlankOX[a] == theBlankOX[b] &&
                  theBlankOX[b] == theBlankOX[c] &&
                  theBlankOX[c] == theBlankOX[d] &&
                  theBlankOX[d] != Blank );
}


/************************************************************************
**
** NAME:        AllFilledIn
**
** DESCRIPTION: Return TRUE iff all the blanks are filled in.
**
** INPUTS:      BlankOX theBlankOX[BOARDSIZE] : The BlankOX array.
**
** OUTPUTS:     (BOOLEAN) TRUE iff all the blanks are filled in.
**
************************************************************************/


BOOLEAN AllFilledIn(BlankOX *theBlankOX) {
    BOOLEAN answer = TRUE;
    int i;


    for(i = 0; i < BOARDSIZE; i++)
        answer &= (theBlankOX[i] == o || theBlankOX[i] == x);


    return(answer);
}


/************************************************************************
**
** NAME:        WhoseTurn
**
** DESCRIPTION: Return whose turn it is - either x or o. Since x always
**              goes first, we know that if the board has an equal number
**              of x's and o's, that it's x's turn. Otherwise it's o's.
**
** INPUTS:      BlankOX theBlankOX : The input board
**
** OUTPUTS:     (BlankOX) : Either x or o, depending on whose turn it is
**
************************************************************************/


BlankOX WhoseTurn(BlankOX *theBlankOX) {
    int i, xcount = 0, ocount = 0;


    for(i = 0; i < BOARDSIZE; i++)
        if(theBlankOX[i] == x)
            xcount++;
        else if(theBlankOX[i] == o)
            ocount++;
        /* else don't count blanks */


    if(xcount == ocount)
        return(x);  /* in our TicTacToe, x always goes first */
    else
        return(o);
}


int NumberOfOptions() {
    return 2;
}
 


int getOption()
{
    int option = 0;
    option += gStandardGame;
    option *= 2;
    option += (int)Diamond;
    return option+1;
}


void setOption(int option)
{
    option -= 1;
    Diamond = (BOOLEAN) (option % 2);
    option /= 2;
    gStandardGame = option;
}


POSITION ActualNumberOfPositions(int variant) {
    (void)variant;
    return 3287691;
}


POSITION StringToPosition(char *positionString) {
    int turn;
    char *board;
    if (ParseStandardOnelinePositionString(positionString, &turn, &board)) {
        BlankOX oxboard[BOARDSIZE];
        for (int i = 0; i < BOARDSIZE; i++) {
            if (board[i] == 'O') {
                oxboard[i] = o;
            } else if (board[i] == 'X') {
                oxboard[i] = x;
            } else if (board[i] == '-') {
                oxboard[i] = Blank;
            } else {
                return NULL_POSITION;
            }
        }
        return BlankOXToPosition(oxboard);
    }
    return NULL_POSITION;
}


void PositionToAutoGUIString(POSITION position, char *autoguiPositionStringBuffer) {
    BlankOX oxboard[BOARDSIZE];
    PositionToBlankOX(position, oxboard);
    char board[BOARDSIZE + 1];
    for (int i = 0; i < BOARDSIZE; i++) {
        if (oxboard[i] == o) {
            board[i] = 'O';
        } else if (oxboard[i] == x) {
            board[i] = 'X';
        } else {
            board[i] = '-';
        }
    }
    board[BOARDSIZE] = '\0';
    AutoGUIMakePositionString(
        WhoseTurn(oxboard) == x ? 1 : 2,
        board,
        autoguiPositionStringBuffer
    );
}


void MoveToAutoGUIString(POSITION position, MOVE move, char *autoguiMoveStringBuffer) {
    BlankOX oxboard[BOARDSIZE];
    PositionToBlankOX(position, oxboard);
    char token = (WhoseTurn(oxboard) == x) ? 'x' : 'o';
    AutoGUIMakeMoveButtonStringA(token, move, 'x', autoguiMoveStringBuffer);
}

