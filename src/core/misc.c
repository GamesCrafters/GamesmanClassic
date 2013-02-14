/************************************************************************
**
** NAME:	misc.c
**
** DESCRIPTION:	Miscellaneous functions available to the gamesman core.
**		Mostly have to do with data structure creation/destruction.
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

#include <time.h>
#include "gamesman.h"


/*
** Globals
*/

int gTimer = 0;

/*
** Misc. variables
*/

char varstr[1024];
char kHelpStar[1024];

/*
** Code
*/

size_t MoveListLength(MOVELIST *ptr)
{
	size_t output = 0;
	while (ptr) {
		ptr = ptr->next;
		++output;
	}
	return output;
}

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

	if (!ptr)
		return;

	for (i=0; i<3; i++) {
		FreeMoveList(ptr->moveList[i]);
		FreeRemotenessList(ptr->remotenessList[i]);
	}
	//  gBytesInUse -= sizeof(VALUE_MOVES);
	SafeFree((GENERIC_PTR)ptr);
}

void FreeTierList(TIERLIST* ptr)
{
	TIERLIST *last;
	while (ptr != NULL) {
		last = ptr;
		ptr = ptr->next;
		SafeFree((GENERIC_PTR)last);
	}
}

void FreeUndoMoveList(UNDOMOVELIST* ptr)
{
	UNDOMOVELIST *last;
	while (ptr != NULL) {
		last = ptr;
		ptr = ptr->next;
		SafeFree((GENERIC_PTR)last);
	}
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
		fprintf(stderr, "Error: GetRandomNumber called with n <= 0! \n");
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

unsigned int Stopwatch()
{
	static int first = TRUE;
	static time_t oldT, newT;

	if(first) {
		first = FALSE;
		newT = time(NULL);
	} else {
		first = TRUE;
	}
	oldT = newT;
	newT = time(NULL);
	return(difftime(newT, oldT));
}

void ExitStageRight()
{
	printf("\nThanks for playing %s!\n",kGameName); /* quit */
	// This is good practice
	generic_hash_destroy();
	DestroyDatabases();
	exit(0);
}

void ExitStageRightErrorString(char errorMsg[])
{
	fprintf(stderr, "\nError: %s\n",errorMsg);
	exit(1);
}

// A Helper added by Ofer and Max: prints only if mustBeTrue is true
void ifprintf(BOOLEAN mustBeTrue, char* formatstring, ...) {
	if (mustBeTrue) {
		va_list argp;
		va_start(argp, formatstring);
		vprintf(formatstring, argp);
		va_end(argp);
	}
}

/* We define these only if not using Memwatch for memory debugging.
 * Otherwise, we replace all calls to these functions with calls to
 * Memwatch equivalents.
 */

#ifndef MEMWATCH
GENERIC_PTR SafeMalloc(size_t amount)
{
	GENERIC_PTR ptr;

	/* Mando's Fix is to put a ckalloc here */
	if((ptr = malloc(amount)) == NULL) {
		fprintf(stderr, "Error: SafeMalloc could not allocate the requested %lu bytes\n",(long) amount);
		ExitStageRight();
		exit(0);
	} else {
		return(ptr);
	}
}

GENERIC_PTR SafeRealloc(GENERIC_PTR ptr, size_t amount)
{
	if(ptr == NULL) {
		ExitStageRightErrorString("Error: SafeRealloc was handed a NULL ptr!\n");
		exit(0);
	} else if((ptr = realloc(ptr, amount)) == NULL) {
		fprintf(stderr, "Error: SafeRealloc could not allocate the requested %lu bytes\n", (long) amount);
		ExitStageRight();
		exit(0);
	} else {
		return(ptr);
	}
}

void SafeFree(GENERIC_PTR ptr)
{
	if(ptr == NULL)
		ExitStageRightErrorString("Error: SafeFree was handed a NULL ptr!\n");
	else {
		free(ptr);
		ptr = NULL;
	}
}
#endif

void BadElse(STRING function)
{
	fprintf(stderr, "Error: %s() just reached an else clause it shouldn't have!\n\n",function);
}

// Adapted from CreateMovelistNode
REMOTENESSLIST *CreateRemotenesslistNode(REMOTENESS theRemoteness, REMOTENESSLIST* theNextRemoteness)
{
	REMOTENESSLIST *theHead;

	theHead = (REMOTENESSLIST *) SafeMalloc (sizeof(REMOTENESSLIST));
	theHead->remoteness = theRemoteness;
	theHead->next = theNextRemoteness;

	return(theHead);
}

// Adapted from CopyMovelist, same problem of reverse order
REMOTENESSLIST *CopyRemotenesslist(REMOTENESSLIST* theRemotenesslist)
{
	REMOTENESSLIST *ptr, *head = NULL;

	ptr = theRemotenesslist;
	while (ptr != NULL) {
		head = CreateRemotenesslistNode(ptr->remoteness, head);
		ptr = ptr->next;
	}

	return(head);
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

/* be sure that queue points to the TAIL of the queue,
   otherwise this will be an insert
 */
void AddPositionToQueue (POSITION pos, POSITIONQUEUE** tail)
{
	POSITIONQUEUE* new_node = (POSITIONQUEUE *) SafeMalloc (sizeof(POSITIONQUEUE));

	new_node->position = pos;
	new_node->next = NULL;
	if (*tail)
		(*tail)->next = new_node;
	(*tail) = new_node;
	return;
}

/* returns the first position in the queue as well as changing queue
   to point to the next position
 */
POSITION RemovePositionFromQueue(POSITIONQUEUE** head)
{
	POSITION result = (*head)->position;
	POSITIONQUEUE *nextnode = (*head)->next;

	SafeFree(*head);

	(*head) = nextnode;

	return result;
}

// list constructor function for TierLists:
TIERLIST *CreateTierlistNode(TIER theTier, TIERLIST* theNextTier)
{
	TIERLIST *theHead;

	theHead = (TIERLIST *) SafeMalloc (sizeof(TIERLIST));
	theHead->tier = theTier;
	theHead->next = theNextTier;

	return(theHead);
}

// return a complete copy of the list, reversed
TIERLIST *CopyTierlist(TIERLIST* theTierlist)
{
	TIERLIST *ptr, *head = NULL;

	ptr = theTierlist;
	while (ptr != NULL) {
		head = CreateTierlistNode(ptr->tier, head);
		ptr = ptr->next;
	}

	return(head);
}

// move the tier to the TOP of the tierlist
// if element not it list, does nothing
TIERLIST *MoveToFrontOfTierlist(TIER theTier, TIERLIST* theTierlist)
{
	TIERLIST *ptr = NULL, *prev = NULL;

	ptr = theTierlist;
	while (ptr != NULL) {
		if (ptr->tier == theTier) {         //move ptr to the front
			if(prev == NULL)         //this IS the first element!
				return theTierlist;
			else {
				prev->next = ptr->next;
				ptr->next = theTierlist;
				return ptr;
			}
		}
		prev = ptr;
		ptr = ptr->next;
	}

	return(theTierlist);
}

// check whether the TIER is in TIERLIST
// true if so, false otherwise
BOOLEAN TierInList(TIER theTier, TIERLIST* theTierlist)
{
	TIERLIST *ptr = theTierlist;

	for (; ptr != NULL; ptr = ptr->next)
		if (ptr->tier == theTier)
			return TRUE;
	return FALSE;
}

// removes the tier from the list
BOOLEAN RemoveTierFromList (TIER theTier, TIERLIST** theTierlist) {
	if (!TierInList(theTier, (*theTierlist)))
		return FALSE;
	*theTierlist = MoveToFrontOfTierlist(theTier, (*theTierlist));
	TIERLIST* temp = *theTierlist;
	(*theTierlist) = (*theTierlist)->next;
	SafeFree(temp);
	return TRUE;
}

// list constructor function for UndoMoveLists:
UNDOMOVELIST *CreateUndoMovelistNode(UNDOMOVE theUndoMove, UNDOMOVELIST* theNextUndoMove)
{
	UNDOMOVELIST *theHead;

	theHead = (UNDOMOVELIST *) SafeMalloc (sizeof(UNDOMOVELIST));
	theHead->undomove = theUndoMove;
	theHead->next = theNextUndoMove;

	return(theHead);
}

void FoundBadPosition (POSITION pos, POSITION parent, MOVE move)
{
#ifdef dup2     /* Redirect stdout to stderr */
	close(1);
	dup2(2, 1);
#endif

	fprintf(stderr, "\n*** ERROR: Invalid position (" POSITION_FORMAT ") encountered.", pos);
	fprintf(stderr, "\n*** ERROR: Parent=" POSITION_FORMAT ", move=%d", parent, move);
	fprintf(stderr, "\n*** ERROR: Representation of parent position:\n\n");
	PrintPosition(pos, "debug", 0);
	fflush(stdout);
	ExitStageRight();
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
	} else {
		returnPosition = thePosition++;
		return(returnPosition);
	}
}

MEXCALC MexAdd(MEXCALC theMexCalc, MEX theMex)
{
	if(theMex > 31) {
		fprintf(stderr, "Error: MexAdd handed a theMex greater than 31\n");
		ExitStageRight();
		exit(0);
	} else if (theMex == kBadMexValue) {
		fprintf(stderr, "Error: MexAdd handed a kBadMexValue for theMex\n");
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

	if (!kPartizan && !gTwoBits) { /* Impartial, mex value available */
		theMex = MexLoad(position);
		if(theMex == (MEX)0)
			(void) sprintf(tmp, "0");
		else if(theMex == (MEX)1)
			(void) sprintf(tmp, "*");
		else
			(void) sprintf(tmp, "*%d", (int)theMex);

		(void) sprintf(string,"[Grundy # = %s]",tmp);
	} else
		sprintf(string, " ");
}

MEX MexPrimitive(VALUE value)
{
	if(value == undecided) {
		fprintf(stderr, "Error: MexPrimitive handed a value other than win/lose (undecided)\n");
		ExitStageRight();
		exit(0);
	} else if(value == tie) {
		fprintf(stderr, "Error: MexPrimitive handed a value other than win/lose (tie)\n");
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

STRING get_var_string() {
	sprintf(varstr,"%s game, with%ssymmetries %sloopy and%spartizan) Hashcode: %d",
	        (gStandardGame) ? "Standard" : "Mis�re", (gSymmetries) ? " " : "out ",
	        (kLoopy) ? "(" : "(non-", (kPartizan) ? " " : " non-", getOption());
	return varstr;
}

/* Returns a string containing which kHelp* strings haven't been written */
STRING kHelpStarWritten() {
	sprintf(kHelpStar,"%s%s%s%s%s%s%s ",
	        strcmp(kHelpGraphicInterface, "Not written yet") ? "" : "GUI help not written, ",
	        strcmp(kHelpTextInterface, "Not written yet") ? "" : "CLI help not written, ",
	        strcmp(kHelpOnYourTurn, "Not written yet") ? "" : "Move help not written, ",
	        strcmp(kHelpStandardObjective, "Not written yet") ? "" : "Standard objective help not written, ",
	        strcmp(kHelpReverseObjective, "Not written yet") ? "" : "Mis�re objective help not written, ",
	        strcmp(kHelpTieOccursWhen, "Not written yet") ? "" : "Tie help not written, ",
	        strcmp(kHelpExample, "Not written yet") ? "" : "Example help not written");
	return kHelpStar;
}

BOOLEAN FirstWordMatches(STRING to_compare, STRING word) {
	return (strstr(to_compare, word) == to_compare &&
	        (strchr(to_compare, ' ') == to_compare + strlen(word) ||
	         strlen(to_compare) == strlen(word)));
}
