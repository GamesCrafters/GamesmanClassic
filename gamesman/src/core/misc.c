
#include "gamesman.h"

int   gTimer = 0;


void FreeMoveList(MOVELIST* ptr)
{
    MOVELIST *last;
    while (ptr != NULL) {
        last = ptr;
        ptr = ptr->next;
        SafeFree((GENERIC_PTR)last);
    }
}

void FreeRemotenessList(REMOTENESSLIST* ptr) 
{
    REMOTENESSLIST* last;
    while (ptr != NULL) {
        last = ptr;
        ptr = ptr->next;
        //    gBytesInUse -= sizeof(REMOTENESSLIST);
        SafeFree((GENERIC_PTR)last);
    }
}

void FreePositionList(POSITIONLIST* ptr)
{
    POSITIONLIST *last;
    while (ptr != NULL) {
        last = ptr;
        ptr = ptr->next;
        SafeFree((GENERIC_PTR)last);
    }
}

void FreeValueMoves(VALUE_MOVES *ptr)
{
    int i;
    
    if (!ptr) return;
    
    for (i=0; i<3; i++) {
        FreeMoveList(ptr->moveList[i]);
        FreeRemotenessList(ptr->remotenessList[i]);
    }
    //  gBytesInUse -= sizeof(VALUE_MOVES);
    SafeFree((GENERIC_PTR)ptr);
}

BOOLEAN ValidMove(POSITION thePosition, MOVE theMove)
{
    MOVELIST *ptr, *head;
    
    head = ptr = GenerateMoves(thePosition);
    while (ptr != NULL) {
        if (theMove == ptr->move) {
            FreeMoveList(head);
            return(TRUE);
        }
        ptr = ptr->next;
    }
    FreeMoveList(head);
    return(FALSE);
}

int GetRandomNumber(int n)
{
    int ans,nRequest,mulFactor;
    
    if(n <= 0) {
        printf("Error: GetRandomNumber called with n <= 0! \n");
        return(0);
    }
    
    /* Try a random number and if it's too big, try again */
    do {
        ans = 0;
        nRequest = n;
        mulFactor = 1;
	
        while (nRequest >= RAND_MAX) {
            ans += (GetSmallRandomNumber(RAND_MAX) * mulFactor);
            nRequest /= RAND_MAX;
            nRequest += 1;
            mulFactor *= RAND_MAX;
        }
        ans += GetSmallRandomNumber(nRequest) * mulFactor;
	
    } while (ans >= n);
    
    return(ans);
}

int GetSmallRandomNumber(int n)
{
    return(n * ((double)randSafe()/RAND_MAX));
}

int randSafe()
{
    int ans;
    while((ans = rand()) == RAND_MAX)
        ;
    return(ans);
}

int Stopwatch()
{
    static int first = 1;
    static time_t oldT, newT;
    
    if(first) {
        first = 0;
        newT = time(NULL);
    }else{
        first = 1;
    }
    oldT = newT;
    newT = time(NULL);
    return(difftime(newT, oldT));
}

void ExitStageRight()
{
    printf("\nThanks for playing %s!\n",kGameName); /* quit */
    // This is good practice
    if (gDatabase != NULL) SafeFree(gDatabase);
    exit(0);
}

void ExitStageRightErrorString(char errorMsg[])
{
    printf("\nError: %s\n",errorMsg);
    exit(1);
}

GENERIC_PTR SafeMalloc(size_t amount)
{
    GENERIC_PTR ptr;
    
    /* Mando's Fix is to put a ckalloc here */
    if((ptr = malloc(amount)) == NULL) {
        printf("Error: SafeMalloc could not allocate the requested %lu bytes\n",amount);
        ExitStageRight();
        exit(0);
    }
    else {
        return(ptr);
    }
}

void SafeFree(GENERIC_PTR ptr)
{
    if(NULL == ptr)
      ExitStageRightErrorString("Error: SafeFree was handed a NULL ptr!\n");
    else
      free(ptr);
}

void BadElse(STRING function)
{
    printf("Error: %s() just reached an else clause it shouldn't have!\n\n",function);
}

MOVELIST *CreateMovelistNode(MOVE theMove, MOVELIST* theNextMove)
{
    MOVELIST *theHead;
    
    theHead = (MOVELIST *) SafeMalloc (sizeof(MOVELIST));
    theHead->move = theMove;
    theHead->next = theNextMove;
    
    return(theHead);
}

MOVELIST *CopyMovelist(MOVELIST* theMovelist)
{
    MOVELIST *ptr, *head = NULL;
    
    /* Walk down the graph children and copy it into a new structure */
    /* Unfortunately, it reverses the order, which is ok */
    
    ptr = theMovelist;
    while (ptr != NULL) {
        head = CreateMovelistNode(ptr->move, head);
        ptr = ptr->next;
    }
    
    return(head);
}


POSITIONLIST *StorePositionInList(POSITION thePosition, POSITIONLIST* thePositionList)
{
    POSITIONLIST *next, *tmp;
    
    next = thePositionList;
    tmp = (POSITIONLIST *) SafeMalloc (sizeof(POSITIONLIST));
    tmp->position = thePosition;
    tmp->next     = next;
    
    return(tmp);
}

POSITIONLIST *CopyPositionlist(POSITIONLIST* thePositionlist)
{
    POSITIONLIST *ptr, *head = NULL;
    
    ptr = thePositionlist;
    while (ptr != NULL) {
        head = StorePositionInList(ptr->position, head);
        ptr = ptr->next;
    }
    
    return(head);
}

void FoundBadPosition (POSITION pos, POSITION parent, MOVE move)
{
#ifdef dup2	/* Redirect stdout to stderr */
    close(1);
    dup2(2, 1);
#endif
    printf("\n*** ERROR: Invalid position (" POSITION_FORMAT ") encountered.", pos);
    printf("\n*** ERROR: Parent=" POSITION_FORMAT ", move=%d", parent, move);
    printf("\n*** ERROR: Representation of parent position:\n\n");
    PrintPosition(pos, "debug", 0);
    fflush(stdout);
    ExitStageRight();
}

VALUE *GetRawValueFromDatabase(POSITION position)
{
    return(&gDatabase[position]);
}


BOOLEAN DefaultGoAgain(POSITION pos,MOVE move)
{
    return FALSE; /* Always toggle turn by default */
}



POSITION GetNextPosition()
{
    static POSITION thePosition = 0; /* Cycle through every position */
    POSITION returnPosition;
    
    while(thePosition < gNumberOfPositions &&
	  GetValueOfPosition(thePosition) == undecided)
        thePosition++;
    
    if(thePosition == gNumberOfPositions) {
        thePosition = 0;
        return(kBadPosition);
    }
    else {
        returnPosition = thePosition++;
        return(returnPosition);
    }
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
