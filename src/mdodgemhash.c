/************************************************************************
**
** NAME:        mdodgem.c
**
** DESCRIPTION: Dodgem
**
** AUTHOR:      Dan Garcia  -  University of California at Berkeley
**              Copyright (C) Dan Garcia, 1995. All rights reserved.
**
** DATE:        04/29/92
**
** UPDATE HIST:
**
** 05-15-95 1.0 : Final release code for M.S.
** 96-04-20 1.1 : Cleaned up the code (reordered it) and tab-aligned it
**
**************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/

#include <stdio.h>
#include "gamesman.h"


POSITION gInitialPosition    = 17524;
POSITION gMinimalPosition    = 17524;

STRING kGameName           = "Dodgem";
BOOLEAN kPartizan           = TRUE;
BOOLEAN kDebugMenu          = FALSE;
BOOLEAN kGameSpecificMenu   = TRUE;
BOOLEAN kTieIsPossible      = FALSE;
BOOLEAN kLoopy               = TRUE;
BOOLEAN kDebugDetermineValue = FALSE;

STRING kHelpGraphicInterface =
        "The LEFT button puts a small circle over your piece. This selects\n\
the FROM slot. The MIDDLE button then selects the TO slot. If you\n\
wish to remove a piece from the board, click the MIDDLE button on\n\
the same place as the FROM slot. The RIGHT button is the same as UNDO,\n\
in that it reverts back to your most recent position."                                                                                                                                                                                                                                                                                                  ;

STRING kHelpTextInterface    =
        "On your turn, use the LEGEND to determine which numbers to choose (between\n\
1 and 9, with 1 at the upper left and 9 at the lower right) to correspond\n\
to the location of your piece and the empty orthogonally-adjacent position\n\
you wish to move that piece to. If you wish to move a piece off of the board,\n\
choose 0 as your destination. Example: '2 0' moves the piece on location\n\
2 off of the board. '5 2' moves your piece from position 5 to position 2."                                                                                                                                                                                                                                                                                                                                                                                                               ;

STRING kHelpOnYourTurn =
        "The moves on your turn are different for different players. Here is a summary:\n\
\n\
O player:      The O player may move his pieces UP, DOWN and to the RIGHT.\n\
               The objective is to be the first to move both of your pieces\n\
 ^             off of the board. You may only move one of your pieces to an\n\
 |             adjacent empty spot on your turn. Moving off of the board\n\
 O->           means moving one of your pieces past the RIGHT-HAND-SIDE\n\
 |             of the board.\n\
 v\n\n\
X player:      The X player may move his pieces LEFT, UP, and to the RIGHT.\n\
               The objective is to be the first to move both of your pieces\n\
    ^          off of the board. You may only move one of your pieces to an\n\
    |          adjacent empty spot on your turn. Moving off of the board\n\
 <- X ->       means moving one of your pieces past the UPPER-SIDE of the board.\n\n\
Note: The circle always goes first."                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         ;

STRING kHelpStandardObjective =
        "To be the FIRST player to move both your pieces off of the board OR prevent\n\
your opponent from moving."                                                                                        ;

STRING kHelpReverseObjective =
        "To be the LAST player to move your pieces off of the board OR to be\n\
prevented from moving by your opponent's pieces."                                                                                ;

STRING kHelpTieOccursWhen =   /* Should follow 'A Tie occurs when... */
                            "";

STRING kHelpExample =
        "         ( 1 2 3 )           : O - -     PLAYER O's turn\n\
LEGEND:  ( 4 5 6 )  TOTAL:   : O - -                     \n\
         ( 7 8 9 )           : - X X                     \n\n\
     Dan's move [(u)ndo/1-9] : 1 2                       \n\n\
         ( 1 2 3 )           : - O -     PLAYER X's turn \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : O - -                     \n\
         ( 7 8 9 )           : - X X                     \n\n\
Computer's move              : 9 6                       \n\n\
         ( 1 2 3 )           : - O -     PLAYER O's turn \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : O - X                     \n\
         ( 7 8 9 )           : - X -                     \n\n\
     Dan's move [(u)ndo/1-9] : 2 3                       \n\n\
         ( 1 2 3 )           : - - O     PLAYER X's turn \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : O - X                     \n\
         ( 7 8 9 )           : - X -                     \n\n\
Computer's move              : 8 7                       \n\n\
         ( 1 2 3 )           : - - O     PLAYER O's turn \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : O - X                     \n\
         ( 7 8 9 )           : X - -                     \n\n\
     Dan's move [(u)ndo/1-9] : 4 5                       \n\n\
         ( 1 2 3 )           : - - O     PLAYER X's turn \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - O X                     \n\
         ( 7 8 9 )           : X - -                     \n\n\
Computer's move              : 7 8                       \n\n\
         ( 1 2 3 )           : - - O     PLAYER O's turn \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - O X                     \n\
         ( 7 8 9 )           : - X -                     \n\n\
     Dan's move [(u)ndo/1-9] : 5 2                       \n\n\
         ( 1 2 3 )           : - O O     PLAYER X's turn \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - - X                     \n\
         ( 7 8 9 )           : - X -                     \n\n\
Computer's move              : 8 5                       \n\n\
         ( 1 2 3 )           : - O O     PLAYER O's turn \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - X X                     \n\
         ( 7 8 9 )           : - - -                     \n\n\
     Dan's move [(u)ndo/1-9] : 3 0                       \n\n\
         ( 1 2 3 )           : - O -     PLAYER X's turn \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - X X                     \n\
         ( 7 8 9 )           : - - -                     \n\n\
Computer's move              : 6 3                       \n\n\
         ( 1 2 3 )           : - O X     PLAYER O's turn \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - X -                     \n\
         ( 7 8 9 )           : - - -                     \n\n\n\
Computer wins. Nice try, Dan."                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   ;


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
char* gBoard = {'b','b','b','b','b','b','b','b','b'};

BOOLEAN gToTrapIsToWin = FALSE;  /* Being stuck is when you can't move. */

InitializeGame()
{
	generic_hash_init(9,0,2,0,2,0);
}

FreeGame()
{
}

/************************************************************************
**
** NAME:        DebugMenu
**
** DESCRIPTION: Menu used to debub internal problems. Does nothing if
**              kDebugMenu == FALSE
**
************************************************************************/

DebugMenu()
{
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

GameSpecificMenu()
{
	char GetMyChar();
	POSITION GetInitialPosition();

	do {
		printf("\n\t----- Game-specific options for %s -----\n\n", kGameName);

		printf("\tCurrent Initial Position:\n");
		PrintPosition(gInitialPosition, gPlayerName[kPlayerOneTurn], kHumansTurn);

		printf("\tI)\tChoose the (I)nitial position\n");
		printf("\tT)\t(T)rapping opponent toggle from %s to %s\n",
		       gToTrapIsToWin ? "GOOD (WINNING)" : "BAD (LOSING)",
		       !gToTrapIsToWin ? "GOOD (WINNING)" : "BAD (LOSING)");

		printf("\n\n\tb)\t(B)ack = Return to previous activity.\n");
		printf("\n\nSelect an option: ");

		switch(GetMyChar()) {
		case 'B': case 'b':
			return;
		case 'Q': case 'q':
			ExitStageRight();
		case 'H': case 'h':
			HelpMenus();
			break;
		case '1':
			gInitialPosition = GetInitialPosition();
			break;
		case 'T': case 't':
			gToTrapIsToWin = !gToTrapIsToWin;
			break;
		default:
			printf("\nSorry, I don't know that option. Try another.\n");
			HitAnyKeyToContinue();
			break;
		}
	} while(TRUE);
}

/************************************************************************
**
** NAME:        SetTclCGameSpecificOptions
**
** DESCRIPTION: Set the C game-specific options (called from Tcl)
**              Ignore if you don't care about Tcl for now.
**
************************************************************************/

SetTclCGameSpecificOptions(theOptions)
int theOptions[];
{
	gToTrapIsToWin = (BOOLEAN) theOptions[0];
}

/************************************************************************
**
** NAME:        DoMove
**
** DESCRIPTION: Apply the move to the position.
**
** INPUTS:      POSITION thePosition : The old position
**              MOVE     theMove     : The move to apply.
**
** OUTPUTS:     (POSITION) : The position that results after the move.
**
** CALLS:       MoveToSlots(MOVE,*SLOT,*SLOT)
**
************************************************************************/

POSITION DoMove(thePosition, theMove)
POSITION thePosition;
MOVE theMove;
{
	generic_hash_unhash(thePosition, gBoard);
	gBoard[theMove/10] = 'b';
	if (theMove%10 != 9) gBoard[theMove%10] = (gWhosTurn ? 'x' : 'o');
	return generic_hash_hash(gBoard);
}

/************************************************************************
**
** NAME:        GetInitialPosition
**
** DESCRIPTION: Ask the user for an initial position for testing. Store
**              it in the space pointed to by initialPosition;
**
** OUTPUTS:     POSITION initialPosition : The position returned
**
************************************************************************/

POSITION GetInitialPosition() /* UNWRITTEN */
{

	printf("\n\n\t----- Get Initial Position -----\n");
	printf("\n\tPlease input the position to begin with.\n");
	printf("\tNote that it should be in the following format:\n\n");
	printf("O - -\nO - -            <----- EXAMPLE \n- X X\n\n");

	i = 0;
	getchar();
	while(i < BOARDSIZE && (c = getchar()) != EOF) {
		if(c == 'x' || c == 'X')
			gBoard[i++] = 'x';
		else if(c == 'o' || c == 'O' || c == '0')
			gBoard[i++] = 'o';
		else if(c == '-')
			gBoard[i++] = 'b';
		else
			; /* do nothing */
	}

	getchar();
	printf("\nNow, whose turn is it? [O/X] : ");
	scanf("%c",&c);
	if(c == 'x' || c == 'X')
		gWhosTurn = 1;
	else
		gWhosTurn = 0;

	return generic_hash_hash(gBoard);
}

/************************************************************************
**
** NAME:        PrintComputersMove
**
** DESCRIPTION: Nicely format the computers move.
**
** INPUTS:      MOVE    computersMove : The computer's move.
**              STRING  computersName : The computer's name.
**
************************************************************************/

PrintComputersMove(computersMove,computersName)
MOVE computersMove;
STRING computersName;
{
	printf ("%8s's move              : %d %d\n", computersName,
	        (computersMove/10)+1,((computersMove%10)==9 ? 0 : (computersMove%10)+1);
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
** CALLS:       BOOLEAN ThreeInARow()
**              BOOLEAN AllFilledIn()
**              PositionToBlankOX()
**              BlankOX OnlyPlayerLeft(*BlankOX)
**
************************************************************************/

	        VALUE Primitive(position)
	        POSITION position;
	        {
	                char theBoard[9];
	                generic_hash_unhash(theBoard, position);
	                if (OnlyPlayerLeft(theBoard) == gWhosTurn)
				return(gStandardGame ? lose : win); /* cause you're the only one left */
	                else if(CantMove(position)) /* the other player just won */
				return(gToTrapIsToWin ? lose : win); /* !gStandardGame */
	                else
				return(undecided); /* no one has won yet */
		}

	        BOOLEAN CantMove(position)
	        POSITION position;
	        {
	                MOVELIST *ptr, *GenerateMoves();
	                BOOLEAN cantMove;

	                ptr = GenerateMoves(position);
	                cantMove = (ptr == NULL);
	                FreeMoveList(ptr);
	                return(cantMove);
		}


	        char OnlyPlayerLeft(theBoard)
	        char* theBoard;
	        {
	                int i;
	                BOOLEAN sawO = FALSE, sawX = FALSE;
	                for(i = 0; i < 9; i++) {
	                        sawO |= (theBoard[i] == 'o');
	                        sawX |= (theBoard[i] == 'x');
			}
	                if(sawX && !sawO)
				return('x');
	                else if (sawO && !sawX)
				return('o');
	                else if (sawO && sawX)
				return('b');
	                else {
	                        printf("Error in OnlyPlayerLeft! the board is blank!!!\n");
	                        return('b');
			}
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

	        PrintPosition(position,playerName,usersTurn)
	        POSITION position;
	        STRING playerName;
	        BOOLEAN usersTurn;
	        {
	                int i;
	                char theBoard[9];

	                generic_hash_unhash(theBoard, position);

	                printf("\n         ( 1 2 3 )           : %s %s %s     PLAYER %c's turn\n",
	                       theBoard[0],
	                       theBoard[1],
	                       theBoard[2],
	                       gWhosTurn);
	                printf("LEGEND:  ( 4 5 6 )  TOTAL:   : %s %s %s\n",
	                       theBoard[3],
	                       theBoard[4],
	                       theBoard[5]);
	                printf("         ( 7 8 9 )           : %s %s %s %s\n\n",
	                       theBoard[6],
	                       theBoard[7],
	                       theBoard[8],
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
** CALLS:       GENERIC_PTR SafeMalloc(int)
**
************************************************************************/

	        MOVELIST *GenerateMoves(position)
	        POSITION position;
	        {
	                MOVELIST *head = NULL;
	                MOVELIST *CreateMovelistNode();
	                char theBoard[9];
	                int i,j; /* Values for J: 0=left,1=straight,2=right */

	                generic_hash_unhash(theBoard, position);

	                for(i = 0; i < 9; i++) { /* enumerate over all FROM slots */
	                        for(j = 0; j < 10; j++) { /* enumerate over all directions */
	                                if(OkMove(theBoard,gWhosTurn,i,j))
						head = CreateMovelistNode(i*10+j,head);
				}
			}
	                return(head);
		}

	        BOOLEAN OkMove(char* theBoard,char whosTurn,int fromSlot,int toslot)
	        {
	                return((theBoard[fromSlot] == whosTurn) &&
	                       (theBoard[toSlot] == Blank || toSlot == OFFTHEBOARD));
		}

	        SLOT GetToSlot(fromSlot,direction,whosTurn)
	        SLOT fromSlot;
	        int direction;
	        {
	                if((fromSlot < 3 && whosTurn == x && direction == 1) ||
	                   (((fromSlot % 3) == 2) && whosTurn == o && direction == 1))
				return(OFFTHEBOARD);
	                else if (((whosTurn == x) &&
	                          ((direction == 2 && ((fromSlot % 3) == 2)) ||
	                           (direction == 0 && ((fromSlot % 3) == 0)))) ||
	                         ((whosTurn == o) &&
	                          ((direction == 2 && (fromSlot > 5)) ||
	     (direction == 0 && (fromSlot < 3)))))
				return(BADSLOT);

	                /** I used this diagram to reference the following.
	                **
	                ** 0 1 2   0            1
	                ** 3 4 5  <O> 1      0 <X> 2
	                ** 6 7 8   2
	                */

	                else if ((whosTurn == o && direction == 1) ||
	                         (whosTurn == x && direction == 2)) /* east */
				return (fromSlot + 1);
	                else if ((whosTurn == o && direction == 0) ||
	                         (whosTurn == x && direction == 1)) /* north */
				return (fromSlot - 3);
	                else if (whosTurn == o && direction == 2) /* south */
				return (fromSlot + 3);
	                else if (whosTurn == x && direction == 0) /* west */
				return (fromSlot - 1);
	                else
				BadElse("GetToSlot");
		}

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

	        USERINPUT GetAndPrintPlayersMove(thePosition, theMove, playerName)
	        POSITION thePosition;
	        MOVE *theMove;
	        STRING playerName;
	        {
	                USERINPUT ret, HandleDefaultTextInput();
	                int xpos, ypos;
	                char input = '0', fromSlotChar, toSlotChar, HandleTextualInput();
	                BOOLEAN done = FALSE, ValidMove();
	                SLOT fromSlot = BADSLOT, toSlot;
	                BlankOX theBlankOX[BOARDSIZE], whosTurn;
	                PositionToBlankOX(thePosition,theBlankOX,&whosTurn);

	                do {
	                        printf("%8s's move [(u)ndo/1-9] : ", playerName);

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

	        BOOLEAN ValidTextInput(input)
	        STRING input;
	        {
	                SLOT fromSlot, toSlot;
	                int ret;
	                ret = sscanf(input,"%d %d", &fromSlot, &toSlot);
	                return(ret == 2 &&
	                       fromSlot <= 9 && fromSlot >= 1 && toSlot <= 9 && toSlot >= 0);
		}

/************************************************************************
**
** NAME:        ConvertTextInputToMove
**
** DESCRIPTION: Convert the string input to the internal move representation.
**              No checking if the input is valid is needed as it has
**              already been checked!
**
** INPUTS:      STRING input : The string input the user typed.
**
** OUTPUTS:     MOVE : The move corresponding to the user's input.
**
************************************************************************/

	        MOVE ConvertTextInputToMove(input)
	        STRING input;
	        {
	                MOVE SlotsToMove();
	                SLOT fromSlot, toSlot;
	                int ret;
	                ret = sscanf(input,"%d %d", &fromSlot, &toSlot);

	                fromSlot--; /* user input is 1-9, our rep. is 0-8 */
	                if(toSlot == 0)
				toSlot = OFFTHEBOARD; /* '0' = offtheboard, convert to OFFTHEBOARD */
	                else
				toSlot--; /* user input is 1-9, our rep. is 0-8 */

	                return(SlotsToMove(fromSlot,toSlot));
		}

/************************************************************************
**
** NAME:        PrintMove
**
** DESCRIPTION: Print the move in a nice format.
**
** INPUTS:      MOVE *theMove         : The move to print.
**
************************************************************************/

	        PrintMove(theMove)
	        MOVE theMove;
	        {
	                SLOT fromSlot, toSlot;
	                MoveToSlots(theMove,&fromSlot,&toSlot);
	                /* The plus 1 is because the user thinks it's 1-9, but MOVE is 0-8 */
	                printf("[ %d %d ] ", fromSlot + 1, toSlot==9 ? 0 : toSlot + 1);
		}

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

	        PositionToBlankOX(thePos,theBlankOX,whosTurn)
	        POSITION thePos;
	        BlankOX *theBlankOX, *whosTurn;
	        {
	                int i;

	                if (thePos >= POSITION_OFFSET) { /* X moves first <==> pos >= offset */
	                        *whosTurn = x;
	                        thePos -= POSITION_OFFSET;
			}
	                else
				*whosTurn = o;

	                for(i = 8; i >= 0; i--) {
	                        if(thePos >= ((int)x * g3Array[i])) {
	                                theBlankOX[i] = x;
	                                thePos -= (int)x * g3Array[i];
				}
	                        else if(thePos >= ((int)o * g3Array[i])) {
	                                theBlankOX[i] = o;
	                                thePos -= (int)o * g3Array[i];
				}
	                        else if(thePos >= ((int)Blank * g3Array[i])) {
	                                theBlankOX[i] = Blank;
	                                thePos -= (int)Blank * g3Array[i];
				}
	                        else
					BadElse("PositionToBlankOX");
			}
		}

/************************************************************************
**
** NAME:        MoveToSlots
**
** DESCRIPTION: convert an internal move to that of two slots
**
** INPUTS:      MOVE theMove    : The move input.
**              SLOT *fromSlot  : The slot the piece moves from (output)
**              SLOT *toSlot    : The slot the piece moves to   (output)
**
************************************************************************/

	        MoveToSlots(theMove, fromSlot, toSlot)
	        MOVE theMove;
	        SLOT *fromSlot, *toSlot;
	        {
	                *fromSlot = theMove % (BOARDSIZE+1);
	                *toSlot   = theMove / (BOARDSIZE+1);
		}

/************************************************************************
**
** NAME:        SlotsToMove
**
** DESCRIPTION: convert two slots (from and to) to an encoded MOVE
**
** INPUT:       SLOT fromSlot   : The slot the piece moves from (0->BOARDSIZE)
**              SLOT toSlot     : The slot the piece moves to   (0->BOARDSIZE)
**
** OUTPUT:      MOVE            : The move corresponding to from->to
**
************************************************************************/

	        MOVE SlotsToMove (fromSlot, toSlot)
	        SLOT fromSlot, toSlot;
	        {
	                return ((MOVE) toSlot*(BOARDSIZE+1) + fromSlot);
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

	        POSITION BlankOXToPosition(theBlankOX,whosTurn)
	        BlankOX *theBlankOX,whosTurn;
	        {
	                int i;
	                POSITION position = 0;

	                for(i = 0; i < BOARDSIZE; i++)
				position += g3Array[i] * (int)theBlankOX[i];

	                if(whosTurn == x)
				position += POSITION_OFFSET; /* account for whose turn it is */

	                return(position);
		}

	        STRING kDBName = "dodgem";

	        int NumberOfOptions()
	        {
	                return 4;
		}

	        int getOption()
	        {
	                if(gStandardGame)
	                {
	                        if(gToTrapIsToWin) return 1;
	                        else return 2;
			}
	                else
	                {
	                        if(gToTrapIsToWin) return 3;
	                        else return 4;
			}
		}

	        void setOption(int option)
	        {
	                if(option == 1)
	                {
	                        gStandardGame = TRUE;
	                        gToTrapIsToWin = TRUE;
			}
	                else if(option == 2)
	                {
	                        gStandardGame = TRUE;
	                        gToTrapIsToWin = FALSE;
			}
	                else if(option == 3)
	                {
	                        gStandardGame = FALSE;
	                        gToTrapIsToWin = TRUE;
			}
	                else
	                {
	                        gStandardGame = FALSE;
	                        gToTrapIsToWin = FALSE;
			}
		}


	        POSITION StringToPosition(char* board) {
	                // FIXME: this is just a stub
	                return atoi(board);
		}


	        char* PositionToString(POSITION pos) {
	                // FIXME: this is just a stub
	                return "Implement Me";
		}

char * PositionToEndData(POSITION pos) {
	return NULL;
}
