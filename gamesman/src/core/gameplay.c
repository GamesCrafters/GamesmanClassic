
void PlayAgainstHuman()
{
    POSITION currentPosition;
    MOVE theMove;
    UNDO *undo;
    BOOLEAN playerOneTurn = TRUE, error, player_draw;
    USERINPUT userInput = Continue; /* default added to satify compiler */
    
    currentPosition = gInitialPosition;
    undo = InitializeUndo();
    
#ifndef X
    printf("Type '?' if you need assistance...\n\n");
#endif
    
    PrintPosition(currentPosition, gPlayerName[playerOneTurn], kHumansTurn);
    
    while(Primitive(currentPosition) == undecided) { /* Not dead yet! */
	
        while((userInput = GetAndPrintPlayersMove(currentPosition,&theMove,
						  gPlayerName[playerOneTurn])) == Undo) {
	    undo = HandleUndoRequest(&currentPosition,undo,&error);
	    if(!error)
		playerOneTurn = !playerOneTurn;		/* the other's turn */
	    PrintPosition(currentPosition,gPlayerName[playerOneTurn], kHumansTurn);
	}
	if(userInput == Abort)
	    break;                 /* jump out of while loop */
	
	if (!gGoAgain(currentPosition,theMove))
	    playerOneTurn = !playerOneTurn;           /* the other's turn */
	
	/* It's !playerOneTurn because the text (Player will lose) is for the other */
	PrintPosition(currentPosition = DoMove(currentPosition,theMove),
		      gPlayerName[playerOneTurn], kHumansTurn);
	
	undo = UpdateUndo(currentPosition, undo, &player_draw);
	if(player_draw)
	    break;
	
    }
    if(Primitive(currentPosition) == tie)
        printf("The match ends in a draw. Excellent strategies, %s and %s.\n\n", 
	       gPlayerName[0], gPlayerName[1]);
    else if(Primitive(currentPosition) == lose)
        printf("\n%s (player %s) Wins!\n\n", gPlayerName[!playerOneTurn], 
	       playerOneTurn ? "two" : "one"); 
    else if(Primitive(currentPosition) == win)
        printf("\n%s (player %s) Wins!\n\n", gPlayerName[playerOneTurn], 
	       playerOneTurn ? "one" : "two");
    else if(userInput == Abort)
        printf("Your abort command has been received and successfully processed!\n");
    else if (player_draw == TRUE) { /* Player chooses to end the game in a draw */
        printf("The match ends in a draw.  Excellent strategy, %s and %s. \n\n",
	       gPlayerName[0], gPlayerName[1]);
    }
    else
        BadElse("PlayAgainstHuman"); 
    
    ResetUndoList(undo);
}

void PlayAgainstComputer()
{
    POSITION thePosition;
    MOVE theMove;
    UNDO *undo;
    BOOLEAN usersTurn, error, player_draw;
    USERINPUT userInput = Continue; /* default added to satisfy compiler */
    int oldRemainingGivebacks;
    
    thePosition = gInitialPosition;
    remainingGivebacks = initialGivebacks;
    undo = InitializeUndo();
    usersTurn = gHumanGoesFirst;
    
    printf("\nOk, %s and %s, let us begin.\n\n",
	   gPlayerName[kPlayerOneTurn], gPlayerName[kPlayerTwoTurn]);
    
#ifndef X
    printf("Type '?' if you need assistance...\n\n");
#endif
    
    PrintPosition(thePosition,gPlayerName[usersTurn],usersTurn);
    
    while(Primitive(thePosition) == undecided) { /* Not dead yet! */
	
        oldRemainingGivebacks = remainingGivebacks; /* keep track of giveback usage for undo */
	
        if(usersTurn) {		/* User's turn */
	    
            while((userInput = GetAndPrintPlayersMove(thePosition,
						      &theMove, 
						      gPlayerName[usersTurn])) == Undo) {
		undo = HandleUndoRequest(&thePosition,undo,&error); /* undo */
		PrintPosition(thePosition,gPlayerName[usersTurn],usersTurn);
	    }
        }
        else {				/* Computer's turn */
            theMove = GetComputersMove(thePosition);
            PrintComputersMove(theMove,gPlayerName[usersTurn]);
        }
        if(userInput == Abort)
            break;                 /* jump out of while loop */
	
        if (!gGoAgain(thePosition,theMove))
            usersTurn = !usersTurn;           /* The other person's turn */
	
        PrintPosition(thePosition = DoMove(thePosition,theMove),
		      gPlayerName[usersTurn],usersTurn);
	
        undo = UpdateUndo(thePosition, undo, &player_draw);
        undo->givebackUsed = oldRemainingGivebacks>remainingGivebacks;
        if(player_draw)
            break;
	
    }
    if((Primitive(thePosition) == lose && usersTurn) || 
       (Primitive(thePosition) == win && !usersTurn))
        printf("\n%s wins. Nice try, %s.\n\n", gPlayerName[kComputersTurn],
	       gPlayerName[kHumansTurn]); 
    else if((Primitive(thePosition) == lose && !usersTurn) ||
	    (Primitive(thePosition) == win && usersTurn))
        printf("\nExcellent! You won!\n\n");
    else if(Primitive(thePosition) == tie)
        printf("The match ends in a tie. Excellent strategy, %s.\n\n", 
	       gPlayerName[kHumansTurn]);
    else if(userInput == Abort)
        printf("Your abort command has been received and successfully processed!\n");
    else if (player_draw == TRUE) { /* Player chooses to end the game in a draw */
        printf("The match ends in a draw.  Excellent strategy, %s. \n\n",
	       gPlayerName[kHumansTurn]);
    }
    else
        BadElse("PlayAgainstHuman"); 
    
    ResetUndoList(undo);
}


void ResetUndoList(UNDO* undo)
{
    POSITION position;
    BOOLEAN error, oldAgainstComputer; /* kludge so that it resets everything */
    
    oldAgainstComputer = gAgainstComputer;
    gAgainstComputer = FALSE;
    while(undo->next != NULL)
        undo = HandleUndoRequest(&position, undo, &error);
    UnMarkAsVisited(undo->position);
    SafeFree((GENERIC_PTR)undo);
    gAgainstComputer = oldAgainstComputer;
}

UNDO *HandleUndoRequest(POSITION* thePosition, UNDO* undo, BOOLEAN* error)
{
    UNDO *tmp;
    
    if((*error = ((undo->next == NULL) ||
		  (gAgainstComputer && (undo->next->next == NULL))))) {
	
	printf("\nSorry - can't undo, I'm already at beginning!\n");
	return(undo);
    }
    
    /* undo the first move */
    
    UnMarkAsVisited(undo->position);
    if (undo->givebackUsed) {
	remainingGivebacks++;
    }
    tmp = undo;
    undo = undo->next;
    SafeFree((GENERIC_PTR)tmp);
    *thePosition = undo->position;
    
    /* If playing against the computer, undo the users move here */
    
    if(gAgainstComputer) {
	UnMarkAsVisited(undo->position);
	tmp = undo;
	undo = undo->next;
	SafeFree((GENERIC_PTR)tmp);
	*thePosition = undo->position;
    }
    
    return(undo);
}

UNDO *UpdateUndo(POSITION thePosition, UNDO* undo, BOOLEAN* abort)
{
    UNDO *tmp;
    
    if(Visited(thePosition)) 
        undo = Stalemate(undo,thePosition,abort);
    else {
        MarkAsVisited(thePosition);
        tmp = undo; 
        undo = (UNDO *) SafeMalloc (sizeof(UNDO));
        undo->position = thePosition;
        undo->givebackUsed = FALSE; /* set this in PlayAgainstComputer */
        undo->next = tmp;
	
        *abort = FALSE;
    }
    return(undo);
}

UNDO *InitializeUndo()
{
    UNDO *undo;
    
    undo = (UNDO *) SafeMalloc (sizeof(UNDO));    /* Initialize the undo list */
    undo->position = gInitialPosition;
    undo->givebackUsed = FALSE;
    undo->next = NULL;
    return(undo);
}

void PrintHumanValueExplanation()
{
    if(gValue == tie) {
        printf("Since this game is a TIE game, the following should happen. The player\n");
        printf("who goes first should not be able to win, because the second person\n");
        printf("should always be able to force a tie. It is possible for either player\n");
        printf("to WIN, but it is easier for the first player to do so. Thus, you may\n");
        printf("wish to have the stronger player play SECOND and try to steal a win.\n\n");
    }
    else {
        printf("You should know that since this is a %s game, the player\n",
	       gValueString[(int)gValue]);
        printf("who goes first _should_ %s. Thus, when playing against each other,\n", 
	       gValueString[(int)gValue]);
        printf("it is usually more enjoyable to have the stronger player play %s,\n",
	       gValue == lose ? "first" : "second");
        printf("and try to take the lead and then win the game. The weaker player\n");
        printf("tries to hold the lead and not give it up to the stronger player.\n\n");
    }
}

void PrintComputerValueExplanation()
{
    if(gValue == tie) {
        printf("You should know that since this is a TIE game, the player who goes\n");
        printf("first *cannot* win unless the second player makes a mistake. Thus, when\n");
        printf("playing against me, the computer, there are two options as to how to\n");
        printf("play the game. You can either play first and if you tie, consider it a\n");
        printf("'win'. However, you *can* lose!  Or you can play second and if you tie,\n");
        printf("also consider it a 'win'. You can lose here *also*.\n\n");
    }
    else if(gValue == win || gValue == lose) {
        printf("You should know that since this is a %s game, the player who goes\n",
	       gValueString[(int)gValue]);
        printf("first *should* %s. Thus, you have two options:\n", 
	       gValueString[(int)gValue]);
        printf("\n");
        printf("A. Choose to play %s.\n", gValue == lose ? "FIRST" : "SECOND");
        printf("\n");
        printf("   Before you choose this, you must understand that I CANNOT lose.\n");
        printf("   Some find this option helpful when they get really headstrong and\n");
        printf("   think they cannot lose a certain game. If you have any friends who\n");
        printf("   think they're unbeatable, have them choose this option. As Bill\n");
        printf("   Cosby would say: 'The game will beat the snot out of him'. :-)\n");
        printf("\n");
        printf("B. Choose to play %s.\n", gValue == win ? "FIRST" : "SECOND");
        printf("\n");
        printf("   In this mode, you CAN win if you choose the right moves, but it's\n");
        printf("   hard to do that with certain games. This is analogous to crossing\n");
        printf("   a mine-field, where every move you take is a step forward in the field.\n");
        printf("   It IS possible to cross unscathed (win the game), but if you make ONE\n");
        printf("   wrong move, the hint will say '<computer> WILL win' and then\n");
        printf("   there's NO WAY for you to win. At this point, you may choose to (u)ndo,\n");
        printf("   which will return you to you previous winning position.\n");
    }
    else
        BadElse("PrintComputerValueExplanation");
}

UNDO *Stalemate(UNDO* undo, POSITION stalematePosition, BOOLEAN* abort)
{
    UNDO *tmp;
    
    printf("\nWe have reached a position we have already encountered. We have\n");
    printf("achieved a STALEMATE. Now, we could go on forever playing like this\n");
    printf("or we could just stop now. Should we continue (y/n) ? ");
    
    if(GetMyChar() != 'y') {      /* quit */
        *abort = TRUE;
    }
    else {
        while(undo->next != NULL && undo->position != stalematePosition) {
            UnMarkAsVisited(undo->position);
            /* don't return givebacks to user when rolling back stalemates */
            tmp = undo;
            undo = undo->next;
            SafeFree((GENERIC_PTR)tmp);
        } 
        MarkAsVisited(undo->position);
        *abort = FALSE;
    }
    return(undo);
}

/* Jiong */
void PrintMoves(MOVELIST* ptr, REMOTENESSLIST* remoteptr)
{
    while (ptr != NULL) {
        printf("\n\t\t");
        PrintMove(ptr->move);
        printf(" \t");
        if((remoteptr->remoteness) == REMOTENESS_MAX)
            printf("Draw");
        else
            printf("%d", (int) remoteptr->remoteness);
        ptr = ptr->next;
        remoteptr = remoteptr->next;
    }
    printf("\n");
}

/* Jiong */
void PrintValueMoves(POSITION thePosition)
{
    VALUE_MOVES *ptr;
    
    ptr = GetValueMoves(thePosition);
    
    printf("\nHere are the values of all possible moves: \n");
    printf("\t\tMove \tRemoteness\n");
    printf("Winning Moves: \t");
    PrintMoves(ptr->moveList[WINMOVE], ptr->remotenessList[WINMOVE]);
    printf("Tieing Moves: \t");
    PrintMoves(ptr->moveList[TIEMOVE], ptr->remotenessList[TIEMOVE]);
    printf("Losing Moves: \t");
    PrintMoves(ptr->moveList[LOSEMOVE], ptr->remotenessList[LOSEMOVE]);
    printf("\n");
    
    FreeValueMoves(ptr);
}

STRING GetPrediction(POSITION position, STRING playerName, BOOLEAN usersTurn)
{
    static char prediction[80];
    char mexString[20];
    VALUE value;
    
    if(gPrintPredictions && (gMenuMode == Evaluated)) {
        MexFormat(position,mexString);
        value = GetValueOfPosition(position);
	
        if (value == tie && Remoteness(position) == REMOTENESS_MAX) {
            (void) sprintf(prediction, "(%s %s draw) %s",
			   playerName,
			   ((value == lose && usersTurn && gAgainstComputer) ||
			    (value == win && !usersTurn && gAgainstComputer)) ?
			   "will" : "should",
			   mexString);
        } else {
            if (gTwoBits) {
                sprintf(prediction, "(%s %s %s)",
			playerName,
			((value == lose && usersTurn && gAgainstComputer) ||
			 (value == win && !usersTurn && gAgainstComputer)) ?
			"will" : "should",
			gValueString[(int)value]);
            } else {
                sprintf(prediction, "(%s %s %s in %d) %s",
			playerName,
			((value == lose && usersTurn && gAgainstComputer) ||
			 (value == win && !usersTurn && gAgainstComputer)) ?
			"will" : "should",
			gValueString[(int)value],
			Remoteness(position),
			mexString);
            }
        }
    }
    else
        (void) sprintf(prediction," ");
    
    return(prediction);
}

/* Jiong */
MOVE GetComputersMove(POSITION thePosition)
{
    MOVE theMove = -1;
    int i, randomMove, numberMoves = 0;
    MOVELIST *ptr, *head, *prev;
    VALUE_MOVES *moves;
    REMOTENESSLIST *rptr=NULL, *rhead;
    BOOLEAN setBackSmartness = FALSE;
    int moveType = -1;
    int oldsmartness = smartness;
    ptr = head = prev = NULL;
    i = 0;
    
    moves = GetValueMoves(thePosition);
    
    // Play Imperfectly
    if (GetRandomNumber(MAXSCALE+1) > scalelvl && smartness == SMART) {
        smartness = RANDOM;
        setBackSmartness = TRUE;
    }
    
    // Use givebacks
    if (remainingGivebacks>0 && GetValueOfPosition(thePosition) < oldValueOfPosition) {
        if(gHints) {
            printf("Using giveback: %d givebacks left\n", remainingGivebacks-1);
            printf("%s choose [ ", gPlayerName[kComputersTurn]);
        }
        while (ptr == NULL) {  //try to restore the old status, if lose then get a lose move, if tie then get tie move
            ptr = moves->moveList[oldValueOfPosition];
            oldValueOfPosition--;
        }
        while(ptr->next != NULL) 
            ptr = ptr->next;
        if(gHints) {
            PrintMove(ptr->move);
            printf(" ]\n\n");
        }
        oldValueOfPosition++;
        remainingGivebacks--;
        theMove = ptr->move;
        FreeValueMoves(moves);
        return (theMove);
    }
    
    oldValueOfPosition = GetValueOfPosition(thePosition);
    
    if (smartness == SMART) {
        if(gHints) {
            printf("Smart move: \n");
            printf("%s could equivalently choose [ ", gPlayerName[kComputersTurn]);
        }
	
        //Find best game outcome
        ptr = NULL;
        while (ptr == NULL && i <= LOSEMOVE) {
            head = ptr = moves->moveList[i];
            rhead = rptr = moves->remotenessList[i];
            moveType = i;
            i++;
        }
	
        if (ptr == NULL || rptr == NULL) {
            printf("Error in GetComputersMove: Either no available moves or corrupted database");
            exit(0);
        }
	
        if(gHints) {
            while(ptr != NULL) {
                PrintMove(ptr->move);
                printf(" ");
                ptr = ptr->next;
            }
            printf("]\n\n");
        }
	
        if (moveType == WINMOVE) {
            // WINMOVE: Win as quickly as possible (smallest remoteness best)
            theMove = RandomSmallestRemotenessMove(moves->moveList[moveType], moves->remotenessList[moveType]);
        } else if (moveType == TIEMOVE) {
            // TIEMOVE: Tie as quickly as possible when smart???
            theMove = RandomSmallestRemotenessMove(moves->moveList[moveType], moves->remotenessList[moveType]);
        } else {
            // LOSEMOVE: Prolong the game as much as possible (largest remoteness is best).
            theMove = RandomLargestRemotenessMove(moves->moveList[moveType], moves->remotenessList[moveType]);
        }
        FreeValueMoves(moves);
        return (theMove);
    }
    
    else if (smartness == RANDOM) {
        if (setBackSmartness == TRUE) {
            smartness = oldsmartness;
            setBackSmartness = FALSE;
        }
        if(gHints) {
            printf("Random move: \n");
            printf("%s could equivalently choose [ ", gPlayerName[kComputersTurn]);      }
	
        for (head = NULL, i = 0; i <= LOSEMOVE; i++) {
            ptr = moves -> moveList[i];
            while (ptr) {
                if (gHints) {
                    PrintMove(ptr -> move);
                    printf(" ");
                }
                head = CreateMovelistNode(ptr -> move, head);
                ptr = ptr -> next;
                numberMoves++;
            }
        }
	
        if(gHints)
            printf("]\n\n");
	
        randomMove = GetRandomNumber(numberMoves);
        for (ptr = head; randomMove > 0; --randomMove)
            ptr = ptr -> next;
	
        theMove = ptr -> move;
	
        FreeMoveList(head);
        FreeValueMoves(moves);
        return(theMove);
    }
    
    else if (smartness == DUMB) {
        if(gHints) {
            printf("Dumb move: \n");
            printf("%s could equivalently choose [ ", gPlayerName[kComputersTurn]);
        }
	
        for (i=LOSEMOVE, ptr=NULL; i >= WINMOVE && ptr == NULL; i--) {
            ptr = moves->moveList[i];
            moveType = i;
        }
        if (ptr == NULL) {
            printf("Error in GetComputersMove: Either no available move or corrupted database");
            exit(0);
        }
	
        if (gHints) {
            while(ptr != NULL) {
                PrintMove(ptr->move);
                printf(" ");
            }
            printf("]\n\n");
        }
	
        if (moveType == LOSEMOVE) {
            theMove = RandomSmallestRemotenessMove(moves->moveList[moveType], moves->remotenessList[moveType]);
        }
        else if (moveType == WINMOVE || moveType == TIEMOVE) {
            theMove = RandomLargestRemotenessMove(moves->moveList[moveType], moves->remotenessList[moveType]);
        }
        FreeValueMoves(moves);
        return (theMove);
	
    } else {
        printf("Error in GetComputerMove: no such intelligence level!\n");
        ExitStageRight();
        exit(0);
    }
}

MOVE RandomLargestRemotenessMove(MOVELIST *moveList, REMOTENESSLIST *remotenessList)
{
    MOVELIST *maxRemotenessMoveList = NULL;
    REMOTENESS maxRemoteness;
    int numMoves, random;
    
    numMoves = 0;
    maxRemoteness = -1;
    while(remotenessList != NULL) {
        if (remotenessList->remoteness > maxRemoteness) {
            numMoves = 1;
            maxRemoteness = remotenessList->remoteness;
            maxRemotenessMoveList = moveList;
        }
        else if (remotenessList->remoteness == maxRemoteness) {
            numMoves++;
        }
        moveList = moveList->next;
        remotenessList = remotenessList->next;
    }
    
    if (numMoves<=0) {
        return -1;
    }
    
    random = GetRandomNumber(numMoves);
    for (; random>0; random--) {
        maxRemotenessMoveList = maxRemotenessMoveList->next;
    }
    return (maxRemotenessMoveList->move);
}

MOVE RandomSmallestRemotenessMove (MOVELIST *moveList, REMOTENESSLIST *remotenessList)
{
    int numMoves, random;
    REMOTENESS minRemoteness;
    
    numMoves = 0;
    minRemoteness = REMOTENESS_MAX;
    while(remotenessList!=NULL && remotenessList->remoteness <= minRemoteness) {
        numMoves++;
        minRemoteness = remotenessList->remoteness;
        remotenessList = remotenessList->next;
    }
    
    if (numMoves<=0) {
        return -1;
    }
    
    random = GetRandomNumber(numMoves);
    for (; random>0; random--) {
        moveList = moveList->next;
    }
    return (moveList->move);
}     

/* Jiong */
VALUE_MOVES* SortMoves (POSITION thePosition, MOVE move, VALUE_MOVES* valueMoves) 
{
    POSITION child;
    VALUE childValue;
    
    child = DoMove(thePosition, move);
    childValue = GetValueOfPosition(child);
    if (gGoAgain(thePosition, move)) {
        switch(childValue) {
	case win: childValue = lose; break;
	case lose: childValue = win; break;
	default: childValue = childValue;
        }
    }
    
    if (childValue == lose) {  //winning moves
        valueMoves = StoreMoveInList(move, Remoteness(child), valueMoves,  WINMOVE);
    } else if (childValue == tie) {  //tie moves
        valueMoves = StoreMoveInList(move, Remoteness(child), valueMoves,  TIEMOVE);
    } else if (childValue == win) {  //lose moves
        valueMoves = StoreMoveInList(move, Remoteness(child), valueMoves, LOSEMOVE);
    }
    return valueMoves;
}

/* Jiong */
VALUE_MOVES* GetValueMoves(POSITION thePosition)
{
    MOVELIST *ptr, *head;
    VALUE_MOVES *valueMoves;
    VALUE theValue;
    
    valueMoves = (VALUE_MOVES *) SafeMalloc (sizeof(VALUE_MOVES));
    valueMoves->moveList[0]=valueMoves->moveList[1]=valueMoves->moveList[2]=NULL;
    valueMoves->remotenessList[0]=valueMoves->remotenessList[1]=valueMoves->remotenessList[2]=NULL;
    
    if(Primitive(thePosition) != undecided)   /* Primitive positions have no moves */
        return(valueMoves);
    
    else if((theValue = GetValueOfPosition(thePosition)) == undecided)
        return(valueMoves);                           /* undecided positions are invalid */
    
    else {                                    /* we are guaranteed it's win | tie now */
        head = ptr = GenerateMoves(thePosition);
        while(ptr != NULL) {                    /* otherwise  (theValue = (win|tie) */
            valueMoves = SortMoves(thePosition, ptr->move, valueMoves);
            ptr = ptr->next;
        }
        FreeMoveList(head);
    }
    return(valueMoves);
}


/* Jiong */
VALUE_MOVES* StoreMoveInList(MOVE theMove, REMOTENESS remoteness, VALUE_MOVES* valueMoves, int typeofMove)
{
    MOVELIST *moveList, *newMove, *prevMoveList;
    REMOTENESSLIST *remotenessList, *newRemoteness, *prevRemotenessList;
    
    moveList = valueMoves->moveList[typeofMove];
    remotenessList = valueMoves->remotenessList[typeofMove];
    
    newMove = (MOVELIST *) SafeMalloc (sizeof(MOVELIST));
    newRemoteness = (REMOTENESSLIST *) SafeMalloc (sizeof(REMOTENESSLIST));
    newMove->move = theMove;
    newMove->next = NULL;
    newRemoteness->remoteness = remoteness;
    newRemoteness->next = NULL;
    
    prevMoveList = NULL;
    prevRemotenessList = NULL;
    
    if (moveList == NULL) {
        valueMoves->moveList[typeofMove] = newMove;
        valueMoves->remotenessList[typeofMove] = newRemoteness;
        return valueMoves;
    }
    
    while(moveList != NULL) {
        if (remotenessList->remoteness >= remoteness) {
            newMove->next = moveList;
            newRemoteness->next = remotenessList;
	    
            if (prevMoveList==NULL) {
                valueMoves->moveList[typeofMove] = newMove;
                valueMoves->remotenessList[typeofMove] = newRemoteness;
            }
            else {
                prevMoveList->next = newMove;
                prevRemotenessList->next = newRemoteness;
            }
            return (valueMoves);
        }
        else {
            prevMoveList = moveList;
            moveList = moveList->next;
            prevRemotenessList = remotenessList;
            remotenessList = remotenessList->next;
        }
    }
    
    prevMoveList->next = newMove;
    prevRemotenessList->next = newRemoteness;
    return (valueMoves);
}

BOOLEAN PrintPossibleMoves(POSITION thePosition)
{
    MOVELIST *ptr, *head;
    
    head = ptr = GenerateMoves(thePosition);
    printf("\nValid Moves : [ ");
    while (ptr != NULL) {
        PrintMove(ptr->move);
        printf(" ");
        ptr = ptr->next;
    }
    printf("]\n\n");
    FreeMoveList(head);
    return(TRUE); /* This should always return true for GetAndPrintPlayersMove */
}
