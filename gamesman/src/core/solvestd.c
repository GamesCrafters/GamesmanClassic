VALUE DetermineValue1(position)
     POSITION position;
{
    BOOLEAN foundTie = FALSE, foundLose = FALSE, foundWin = FALSE;
    MOVELIST *ptr, *head;
    VALUE value;
    POSITION child;
    REMOTENESS maxRemoteness = 0, minRemoteness = MAXINT2;
    REMOTENESS minTieRemoteness = MAXINT2, remoteness;
    MEXCALC theMexCalc = 0; /* default to satisfy compiler */
    
    if(Visited(position)) { /* Cycle! */
        return(win);
    }
    /* It's been seen before and value has been determined */
    else if((value = GetValueOfPosition(position)) != undecided) { 
        return(value);
    } else if((value = Primitive(position)) != undecided) {  
        /* first time, end */
        SetRemoteness(position,0); /* terminal positions have 0 remoteness */
        if(!kPartizan)
            MexStore(position,MexPrimitive(value)); /* lose=0, win=* */
        return(StoreValueOfPosition(position,value));
        /* first time, need to recursively determine value */
    } else { 
        MarkAsVisited(position);
        if(!kPartizan)
            theMexCalc = MexCalcInit();
        head = ptr = GenerateMoves(position);
        while (ptr != NULL) {
            MOVE move = ptr->move ;
            gTotalMoves++;
            child = DoMove(position,ptr->move);  /* Create the child */
            if (child < 0 || child >= gNumberOfPositions)
                FoundBadPosition(child, position, move);
            value = DetermineValue1(child);       /* DFS call */
	    
            if (gGoAgain(position,move))
                switch(value)
		    {
		    case lose: value=win;break;
		    case win: value=lose;break;
		    default: break; /* value stays the same */
		    }
	    
            remoteness = Remoteness(child);
            if(!kPartizan)
                theMexCalc = MexAdd(theMexCalc,MexLoad(child));
            if(value == lose) {        /* found a way to give you a lose */
                foundLose = TRUE;        /* thus, it's a winning move      */
                if (remoteness < minRemoteness) minRemoteness = remoteness;
            }
            else if(value == tie) {    /* found a way to give you a tie  */
                foundTie = TRUE;         /* thus, it's a tieing move       */
                if (remoteness < minTieRemoteness) minTieRemoteness = remoteness;
            }
            else if(value == win) {    /* found a way to give you a win  */
                foundWin = TRUE;         /* thus, it's a losing move       */
                if (remoteness > maxRemoteness) maxRemoteness = remoteness;
            }
            else
                BadElse("DetermineValue (1) ");
            ptr = ptr->next;
        }
        FreeMoveList(head);
        UnMarkAsVisited(position);
        if(!kPartizan)
            MexStore(position,MexCompute(theMexCalc));
        if(foundLose) {
            SetRemoteness(position,minRemoteness+1); /* Winners want to mate soon! */
            return(StoreValueOfPosition(position,win));
        }
        else if(foundTie) {
            SetRemoteness(position,minTieRemoteness+1); /* Tiers want to mate now! */
            return(StoreValueOfPosition(position,tie));
        }
        else if (foundWin) {
            SetRemoteness(position,maxRemoteness+1); /* Losers want to extend! */
            return(StoreValueOfPosition(position,lose));
        }
        else
            BadElse("DetermineValue (2)");
    }
    BadElse("DetermineValue (3)");  /* This should NEVER be reached */
    return(undecided);          /* But has been added to satisty lint */
}

VALUE DetermineValue(POSITION position)
{
    if(gReadDatabase && loadDatabase()) {
        if (gPrintDatabaseInfo) printf("\nLoading %s from Database...",kGameName);
	
        if (GetValueOfPosition(position) == undecided) {
            if (gPrintDatabaseInfo) printf("\nRe-evaluating the value of %s...", kGameName);
            gSolver(position);
            if(gWriteDatabase)
                writeDatabase();
        }
    }
    else {
        if (gPrintDatabaseInfo) printf("\nEvaluating the value of %s...", kGameName);
        gSolver(position);
        showStatus(1);
        if(gWriteDatabase)
            writeDatabase();
    }
    
    gValue = GetValueOfPosition(position);
    return gValue;
}

MEXCALC MexAdd(MEXCALC theMexCalc, MEX theMex)
{
    if(theMex > 31) {
        printf("Error: MexAdd handed a theMex greater than 31\n");
        ExitStageRight();
        exit(0);
    } else if (theMex == kBadMexValue) {
        printf("Error: MexAdd handed a kBadMexValue for theMex\n");
        ExitStageRight();
        exit(0);
    }
    return(theMexCalc | (1 << theMex));
}

MEX MexCompute(MEXCALC theMexCalc)
{
    MEX ans = 0;
    while(theMexCalc & (1 << ans))
        ans++;
    return(ans);
}

MEXCALC MexCalcInit()
{
    return((MEXCALC) 0);
}

void MexStore(POSITION position, MEX theMex)
{
    if (!gTwoBits)
        gDatabase[position] |= ((theMex % 32) * 8) ;
}

MEX MexLoad(POSITION position)
{
    //Gameline code removed
    return (gTwoBits ? 0 : (gDatabase[position]/8) % 32);
}

void MexFormat(POSITION position, STRING string)
{
    MEX theMex;
    char tmp[5];
    
    if (!kPartizan) { /* Impartial, mex value available */
        theMex = MexLoad(position);
        if(theMex == (MEX)0)
            (void) sprintf(tmp, "0");
        else if(theMex == (MEX)1)
            (void) sprintf(tmp, "*");
        else
            (void) sprintf(tmp, "*%d", (int)theMex);
	
        (void) sprintf(string,"[Val = %s]",tmp);
    } else
        sprintf(string, " ");
}

MEX MexPrimitive(VALUE value)
{
    if(value == undecided) {
        printf("Error: MexPrimitive handed a value other than win/lose (undecided)\n");
        ExitStageRight();
        exit(0);
    } else if(value == tie) {
        printf("Error: MexPrimitive handed a value other than win/lose (tie)\n");
        ExitStageRight();
        exit(0);
    } else if(value == win) 
        return((MEX)1); /* A win terminal is not ideal, but it's a star */
    else if (value == lose)
        return((MEX)0); /* A lose is a zero, that's clear */
    else {
        BadElse("MexPrimitive");
        ExitStageRight();
        exit(0);
    }
}
