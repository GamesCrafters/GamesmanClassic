
#include "gamesman.h"

void DebugModule()
{
    int numberMoves = 0;
    MOVELIST *head = NULL, *ptr;
    MOVE theMove;
    BOOLEAN haveMove = FALSE, tempPredictions = gPrintPredictions;
    
    gPrintPredictions = FALSE;
    
    do {
        printf("\n\t----- Module DEBUGGER for %s -----\n\n", kGameName);
	
        printf("\tCurrent Position:\n");
        PrintPosition(gInitialPosition, gPlayerName[kPlayerOneTurn], kHumansTurn);

        printf("\t1)\tGet the debugger initial position (GetInitialPosition)\n");
        printf("\t2)\tPrint what PRIMITIVE() would return for this position\n"); 
        printf("\t3)\tPrint the list of moves available for this position\n"); 
        printf("\t4)\tChoose a move (GetAndPrintPlayersMove)\n");
        if (haveMove)
            printf("\t5)\tDo that Move (DoMove)\n");
	
        printf("\n\n\tb)\t(B)ack = Return to previous activity.\n");
        printf("\n\nSelect an option: ");

        switch(GetMyChar()) {
	case 'Q': case 'q':
	    ExitStageRight();
	    exit(0);
	case 'H': case 'h':
	    HelpMenus();
	    break;
	case '1':
	    gInitialPosition = GetInitialPosition();
	    break;
	case '2':
	    printf("\nPrimitive Returns: %s\n",gValueString[Primitive(gInitialPosition)]);
	    break;
	case '3':
	    printf("\nThe list of available moves is:\n");
	    head = ptr = GenerateMoves(gInitialPosition);
	    numberMoves = 0;
	    while(ptr != NULL) {
		numberMoves++;
		printf("%2d : ",numberMoves);
		PrintMove(ptr->move);
		printf("\n");
		ptr = ptr->next;
	    }
	    FreeMoveList(head);
	    break;
	case '4':
	    printf("\n");
	    (void) GetAndPrintPlayersMove(gInitialPosition, &theMove, gPlayerName[kPlayerOneTurn]);
	    printf("You Chose: ");
	    PrintMove(theMove);
	    haveMove = TRUE;
	    break;
	case '5':
	    if(haveMove) {
		gInitialPosition = DoMove(gInitialPosition, theMove);
		haveMove = FALSE;
	    }
	    else
		BadMenuChoice();
	    break;
	case 'b': case 'B':
	    gPrintPredictions = tempPredictions;
	    return;
	default:
	    BadMenuChoice();
	    HitAnyKeyToContinue();
	    break;
        }
    } while(TRUE);
    
}

