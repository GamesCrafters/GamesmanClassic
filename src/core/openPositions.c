/*
 * Experimental Open Positions code by David Eitan Poll
 * Exploring re-evaluation of Draw Positions
 */
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include "openPositions.h"
#include "solveloopy.h"
#include "gameplay.h"
#include "misc.h"

#include "analysis.h"
//MATT

OPEN_POS_DATA* openPosData=0;
POSITION openPosArrLen;
BOOLEAN gOpenDataLoaded = FALSE;
POSITIONLIST* headNodeDP;
POSITIONLIST* tailNodeDP;
char* corruptedPositions;
char* fringePositions;
extern char* gNumberChildren;
extern char* gNumberChildrenOriginal;
extern POSITION gNumberOfPositions;
extern POSITIONLIST** gParents;
FILE *openData;

static void             DrawParentInitialize                (void);
static void    			DeterminePure1           			(POSITION pos);
static void             DrawParentFree                      (void);
static void             SetDrawParents                      (POSITION bad, POSITION root);

void InitializeOpenPositions(int numPossiblePositions)
{
	OPEN_POS_DATA init=SetCorruptionLevel(SetDrawValue(0,undecided),CORRUPTION_MAX);
	OPEN_POS_DATA* p;
	char* c;
	if(!openPosData)
	{
		openPosData=(OPEN_POS_DATA*)malloc(numPossiblePositions*sizeof(OPEN_POS_DATA));
		corruptedPositions=(char*)malloc(numPossiblePositions*sizeof(char));
		fringePositions=(char*)malloc(numPossiblePositions*sizeof(char));
		if(!openPosData) return;
	}
	else if(openPosArrLen!=numPossiblePositions)
	{
		CleanupOpenPositions();
		InitializeOpenPositions(numPossiblePositions);
		return;
	}
	for(p=openPosData; p<openPosData+numPossiblePositions; p++)
		*p=init;
	for(c=corruptedPositions; c<corruptedPositions+numPossiblePositions; c++)
		*c=0;
	for(c=fringePositions; c<fringePositions+numPossiblePositions; c++)
		*c=0;
	openPosArrLen=numPossiblePositions;
	while(headNodeDP) DequeueDP();
	return;
}
int OpenIsInitialized(void)
{
	return openPosData ? 1 : 0;
}
void CleanupOpenPositions(void)
{
	//printf("HERE!!!!!\n");
	SafeFree(corruptedPositions);
	//printf("HERE!!!!!\n");
	SafeFree(fringePositions);
	//printf("HERE!!!!!\n");
	while(headNodeDP) DequeueDP();
	//printf("HERE!!!!!\n");
	tailNodeDP=NULL;
	return;
}
void FreeOpenPositions(void)
{
	if(!openPosData) return;
	//printf("HERE!!!!!\n");
	SafeFree(openPosData);
	openPosData=NULL;
}

void RegisterDrawPosition(POSITION pos)
{
	EnqueueDP(pos);
	return;
}

void EnqueueDP(POSITION pos)
{
	POSITIONLIST* node;
	node=(POSITIONLIST*)malloc(sizeof(POSITIONLIST));
	if(!node) return;
	node->position=pos;
	node->next=NULL;
	if(!headNodeDP)
	{
		headNodeDP=node;
		tailNodeDP=node;
	}
	else if(tailNodeDP)
	{
		tailNodeDP->next=node;
		tailNodeDP=node;
	}
	return;
}

POSITION DequeueDP()
{
	POSITION p;
	POSITIONLIST* head=headNodeDP;
	if(!head) return 0;
	p=head->position;
	headNodeDP=head->next;
	free(head);
	return p;
}

void SetOpenData(POSITION pos, OPEN_POS_DATA value)
{
	if(!openPosData) return;
	openPosData[pos]=value;
}
OPEN_POS_DATA GetOpenData(POSITION pos)
{
	if(!openPosData) return SetCorruptionLevel(SetDrawValue(0,undecided),CORRUPTION_MAX);
	return openPosData[pos];
}
void PrintChildrenCounts()
{
	POSITION x;
	for(x=0; x<gNumberOfPositions; x++)
		printf("Parent: %d  NumChildren: %d  Orig: %d\n",(int)x,(int)gNumberChildren[x],(int)gNumberChildrenOriginal[x]);
}
void PropogateFreAndCorUp(POSITION p)
{
	PropogateFreAndCorUpFringe(p, 0);
}
OPEN_POS_DATA DetermineFreAndCorDown1LevelForWin(POSITION p)
{
	OPEN_POS_DATA dat=GetOpenData(p);
	MOVELIST* moves=GenerateMoves(p);
	MOVELIST* temp=moves;
	int count=0;
	int minFremoteness=FREMOTENESS_MAX;
	int minCorruption=CORRUPTION_MAX;
	for(; moves; moves=moves->next)
	{
		POSITION child=DoMove(p,moves->move);
		OPEN_POS_DATA cdat=GetOpenData(child);
		if(GetDrawValue(cdat)!=lose || GetLevelNumber(cdat)!=GetLevelNumber(dat)) continue;
		if(GetCorruptionLevel(cdat)==minCorruption && GetFremoteness(cdat)<minFremoteness)
		{
			minFremoteness=GetFremoteness(cdat);
			corruptedPositions[p]=corruptedPositions[p]||corruptedPositions[child];
		}
		if(GetCorruptionLevel(cdat)<minCorruption)
		{
			minCorruption=GetCorruptionLevel(cdat);
			minFremoteness=GetFremoteness(cdat);
			corruptedPositions[p]=corruptedPositions[child];
		}
		count++;
	}
	FreeMoveList(temp);
	if(count==0) return GetOpenData(p);
	dat=SetFremoteness(SetCorruptionLevel(dat,minCorruption),minFremoteness+1);
	return dat;
}
void PropogateFreAndCorUpFringe(POSITION p, char fringe)
{
	OPEN_POS_DATA dat=GetOpenData(p);
	POSITIONLIST* parents=gParents[p];
	if(GetDrawValue(dat)==undecided) return;
	for(; parents; parents=parents->next)
	{
		OPEN_POS_DATA pdat=GetOpenData(parents->position);
		OPEN_POS_DATA old=pdat;
		if(GetLevelNumber(pdat)!=GetLevelNumber(dat)) continue;
		if(!fringe && GetFringe(pdat))
		{
			pdat=SetFremoteness(pdat,0);
			pdat=SetFringe(pdat,1);
			SetOpenData(parents->position,pdat);
			continue;
		}
		switch(GetDrawValue(dat))
		{
		case win:
			switch(GetDrawValue(pdat))
			{
			case win:
				break;
			case lose:
				if(GetCorruptionLevel(dat)>GetCorruptionLevel(pdat))
				{
					pdat=SetCorruptionLevel(pdat,GetCorruptionLevel(dat));
					pdat=SetFremoteness(pdat,GetFremoteness(dat)+1);
					corruptedPositions[parents->position]=corruptedPositions[parents->position]||corruptedPositions[p];
					if(!fringe) pdat=SetFringe(pdat,0);
				}
				else if(GetCorruptionLevel(dat)==GetCorruptionLevel(pdat) && GetFremoteness(dat)+1>GetFremoteness(pdat))
				{
					pdat=SetFremoteness(pdat,GetFremoteness(dat)+1);
					corruptedPositions[parents->position]=corruptedPositions[parents->position]||corruptedPositions[p];
					if(!fringe) pdat=SetFringe(pdat,0);
				}
				break;
			}
			break;
		case lose:
			if(GetDrawValue(pdat)==win)
			{
				/*if(GetCorruptionLevel(dat)<GetCorruptionLevel(pdat))
				   {
				        pdat=SetCorruptionLevel(pdat,GetCorruptionLevel(dat));
				        pdat=SetFremoteness(pdat,GetFremoteness(dat)+1);
				        corruptedPositions[parents->position]=corruptedPositions[parents->position]||corruptedPositions[p];
				        if(!fringe) pdat=SetFringe(pdat,0);
				   }
				   else if(GetCorruptionLevel(dat)==GetCorruptionLevel(pdat) && GetFremoteness(dat)+1<GetFremoteness(pdat))
				   {
				        pdat=SetFremoteness(pdat,GetFremoteness(dat)+1);
				        corruptedPositions[parents->position]=corruptedPositions[parents->position]||corruptedPositions[p];
				        if(!fringe) pdat=SetFringe(pdat,0);
				   }
				   else
				   {*/
				pdat=DetermineFreAndCorDown1LevelForWin(parents->position);
				if(!fringe) pdat=SetFringe(pdat,0);        /*
				                                              }*/
			}
			break;
		}
		if(!fringe && GetFringe(old))
		{
			pdat=SetFremoteness(old,0);
			return;
			//pdat=SetFringe(pdat,1);
		}
		if(pdat!=old)
		{
			SetOpenData(parents->position,pdat);
			PropogateFreAndCorUpFringe(parents->position,fringe);
		}
		if(GetFringe(pdat) && GetFremoteness(pdat)) printf("DAVID!!!!\n");
	}
}
void AddToParentsChildrenCount(POSITION child, int amt)
{
	POSITIONLIST* parents=gParents[child];
	for(; parents; parents=parents->next)
		gNumberChildren[parents->position]+=amt;
}
void ComputeOpenPositions()
{
	int curLevel=1;
	POSITION iter;
	InitializeOpenPositions(gNumberOfPositions);
	if(!openPosData) return;
	//PrintChildrenCounts();

	InitializeFR();

	while(1)
	{
		int fringePosCount=0;
		int maxCorruption=0;
		int i;
		/* first, find all fringe positions */
		printf("Get going!\n");
		for(iter=0; iter<gNumberOfPositions; iter++)
		{
			OPEN_POS_DATA dat=GetOpenData(iter);

			/* if the number of children of an undecided value is less than the original number of children but >0, it
			   has a winning child and is therefore a fringe */
			//printf("Pos %d has %d/%d children\n",iter,(int)gNumberChildren[iter],(int)gNumberChildrenOriginal[iter]);
			if(gNumberChildren[iter]>0&&gNumberChildren[iter]<gNumberChildrenOriginal[iter]&&GetDrawValue(dat)==undecided && GetValueOfPosition(iter)==tie && Remoteness(iter)==REMOTENESS_MAX)
			{
				if(curLevel==1) dat=SetCorruptionLevel(dat,0);
				else
				{
					int maxWinChildCorr=0;
					MOVELIST* moves=GenerateMoves(iter);
					MOVELIST* temp=moves;
					for(; moves; moves=moves->next)
					{
						POSITION child=DoMove(iter,moves->move);
						OPEN_POS_DATA cdat=GetOpenData(child);
						if(GetDrawValue(cdat)!=win) continue;
						if(GetCorruptionLevel(cdat)>maxWinChildCorr) maxWinChildCorr=GetCorruptionLevel(cdat);
					}
					FreeMoveList(temp);
					dat=SetCorruptionLevel(dat,maxWinChildCorr);
				}
				//printf("Corruption level of %d is %d\n",iter,GetCorruptionLevel(dat));
				SetOpenData(iter,SetFringe(SetLevelNumber(SetFremoteness(SetDrawValue(dat,lose),0),curLevel),1));
				InsertLoseFR(iter);
				fringePosCount++;
				fringePositions[iter]=1;
			}
		}
		/* if we didn't find any fringe positions, we just label everyone else as pure ties */
		if(fringePosCount==0) {
			//printf("Done!!!\n");
			gAnalysis.LargestFoundLevel = curLevel-1;                               //MATT
			break;
		}
		/* do paper-ish solving of the level.  Corruption and Fremoteness propogate up here */
		while(gHeadLoseFR || gHeadWinFR)
		{
			//printf("HERE\n");
			while(gHeadLoseFR)
			{
				POSITION pos=DeQueueLoseFR();
				POSITIONLIST* parents=gParents[pos];
				OPEN_POS_DATA dat=GetOpenData(pos);
				//printf("Looping!\n");
				for(; parents; parents=parents->next)
				{
					OPEN_POS_DATA pdat;
					OPEN_POS_DATA old;
					if(!(GetValueOfPosition(parents->position)==tie && Remoteness(parents->position)==REMOTENESS_MAX)) continue;
					pdat=GetOpenData(parents->position);
					/* If my parent is already a lose and not already corrupted, corrupt it and move on */
					if(GetDrawValue(pdat)==lose)
					{
						if(!corruptedPositions[parents->position]&&GetFringe(pdat))
						{
							corruptedPositions[parents->position]=1;
							pdat=SetCorruptionLevel(pdat,GetCorruptionLevel(pdat)+1);
							SetOpenData(parents->position,pdat);
							if(GetCorruptionLevel(pdat)>curLevel)
							{
								printf("This is not good!\n");
								PrintSingleOpenData(parents->position);
							}
							PropogateFreAndCorUp(parents->position);
							if(GetCorruptionLevel(pdat)>maxCorruption)
							{
								maxCorruption=GetCorruptionLevel(pdat);
							}
							if(GetFremoteness(pdat)>0)
							{
								printf("No, no, no!!!");
							}
						}
						continue;
					}
					//printf("Parent: %d\n",parents->position);
					if(GetFringe(pdat)) continue;
					old=pdat;
					pdat=SetDrawValue(pdat,win);
					pdat=SetLevelNumber(pdat,curLevel);
					if(GetFremoteness(pdat)>GetFremoteness(dat)+1 || GetDrawValue(old)==undecided)
						pdat=SetFremoteness(pdat,GetFremoteness(dat)+1);
					if(GetCorruptionLevel(pdat)>GetCorruptionLevel(dat) || GetCorruptionLevel(pdat)==CORRUPTION_MAX)
					{
						pdat=SetCorruptionLevel(pdat,GetCorruptionLevel(dat));
						corruptedPositions[parents->position]=corruptedPositions[pos];
					}
					SetOpenData(parents->position,pdat);
					if(pdat!=old)
					{
						InsertWinFR(parents->position);
					}
					if(GetCorruptionLevel(pdat)!=GetCorruptionLevel(old) || GetFremoteness(pdat)!=GetFremoteness(old))
					{
						PropogateFreAndCorUp(parents->position);
					}
				}
			}
			//PrintOpenDataFormatted();
			//printf("HERE1\n");
			while(gHeadWinFR)
			{
				POSITION pos=DeQueueWinFR();
				POSITIONLIST* parents=gParents[pos];
				OPEN_POS_DATA dat=GetOpenData(pos);
				char timeToBreak=0;
				if(pos==kBadPosition) continue;
				for(; parents; parents=parents->next)
				{
					OPEN_POS_DATA pdat;
					OPEN_POS_DATA old;
					if(!(GetValueOfPosition(parents->position)==tie && Remoteness(parents->position)==REMOTENESS_MAX)) continue;
					pdat=GetOpenData(parents->position);
					if(GetFringe(pdat)) continue;
					if(--gNumberChildren[parents->position]==0)
					{
						pdat=SetDrawValue(pdat,lose);
						pdat=SetLevelNumber(pdat,curLevel);
						SetOpenData(parents->position,pdat);
						InsertLoseFR(parents->position);
						timeToBreak=1;
					}
					old=pdat;
					if(GetFremoteness(pdat)<GetFremoteness(dat)+1 && !GetFringe(pdat))
						pdat=SetFremoteness(pdat,GetFremoteness(dat)+1);
					if((GetCorruptionLevel(pdat)<GetCorruptionLevel(dat) || GetCorruptionLevel(pdat)==CORRUPTION_MAX) && GetDrawValue(pdat)==lose)
					{
						pdat=SetCorruptionLevel(pdat,GetCorruptionLevel(dat));
						corruptedPositions[parents->position]=corruptedPositions[pos];
					}
					SetOpenData(parents->position,pdat);
					if(pdat!=old) PropogateFreAndCorUp(parents->position);
				}
				if(timeToBreak) break;
			}
		}
		//printf("HERE!!!!!!!\n");
		/* all that's left is to do fixing at each corruption level. */
		for(i=0; i<curLevel; i++)
		{
			/* load the win/lose frontier and subtract off children from the counts of their parents if they're not in the
			   current level and they are losing */
			for(iter=0; iter<gNumberOfPositions; iter++)
			{
				OPEN_POS_DATA dat=GetOpenData(iter);
				corruptedPositions[iter]=0;
				if(GetFringe(dat) && GetFremoteness(dat)) printf("ASDFDASFDASDFA!\n");
				if((GetCorruptionLevel(dat)>i && GetDrawValue(dat)==lose && GetLevelNumber(dat)==curLevel))
				{
					if(GetDrawValue(dat)==undecided) continue;
					AddToParentsChildrenCount(iter,-1);
				}
				else if(GetCorruptionLevel(dat)==i && GetLevelNumber(dat)==curLevel)
				{
					if(GetDrawValue(dat)==win)
					{
						InsertWinFR(iter);
						AddToParentsChildrenCount(iter,1);
					}
					else if(GetDrawValue(dat)==lose)
						InsertLoseFR(iter);
				}
			}
			/* solve, fixing */
			printf("here1\n");
			while(gHeadLoseFR || gHeadWinFR)
			{
				printf("here1.1\n");
				while(gHeadLoseFR)
				{
					POSITION pos=DeQueueLoseFR();
					POSITIONLIST* parents=gParents[pos];
					OPEN_POS_DATA dat=GetOpenData(pos);
					for(; parents; parents=parents->next)
					{
						OPEN_POS_DATA pdat=GetOpenData(parents->position);
						OPEN_POS_DATA old;
						if(!(GetValueOfPosition(parents->position)==tie && Remoteness(parents->position)==REMOTENESS_MAX)) continue;
						old=pdat;
						/* If I've got a losing parent of the same corruption level, it's legit. */
						if(GetDrawValue(pdat)==lose && GetCorruptionLevel(pdat)==i) continue;
						pdat=SetDrawValue(pdat,win);
						pdat=SetLevelNumber(pdat,curLevel);
						fringePositions[parents->position]=0;
						pdat=SetFringe(pdat, 0);
						if(GetCorruptionLevel(pdat)<i) continue;
						if(GetCorruptionLevel(pdat)>i)
						{
							pdat=SetCorruptionLevel(pdat, i);
							pdat=SetFremoteness(pdat, GetFremoteness(dat)+1);
						}
						if((GetFremoteness(pdat)>GetFremoteness(dat)+1 || GetDrawValue(old)!=GetDrawValue(pdat)) && !GetFringe(old))
							pdat=SetFremoteness(pdat,GetFremoteness(dat)+1);
						if(GetDrawValue(old)==GetDrawValue(pdat) && GetCorruptionLevel(old)==GetCorruptionLevel(pdat) && GetFringe(old)) continue;
						SetOpenData(parents->position,pdat);
						if(GetFremoteness(pdat) && GetFringe(pdat)) printf("Dis not good\n");
						//printf("start propogating\n");
						//PrintSingleOpenData(parents->position);
						if(pdat!=old) PropogateFreAndCorUpFringe(parents->position,0);
						//printf("done propogating\n");
						if(pdat!=old || GetDrawValue(pdat)!=GetDrawValue(old))
						{
							InsertWinFR(parents->position);
						}
						if(GetDrawValue(pdat)==win && GetDrawValue(old)==lose)
						{
							AddToParentsChildrenCount(parents->position,1);
							//if(GetLevelNumber(pdat)==2) printf("l-->w%d\n",parents->position);
						}
						//else if(pdat!=old && GetLevelNumber(pdat)==2) printf("-->w%d\n",parents->position);
					}
				}
				printf("here1.2\n");
				while(gHeadWinFR)
				{
					POSITION pos=DeQueueWinFR();
					POSITIONLIST* parents=gParents[pos];
					OPEN_POS_DATA dat=GetOpenData(pos);
					char timeToBreak=0;

					for(; parents; parents=parents->next)
					{
						OPEN_POS_DATA pdat;
						OPEN_POS_DATA old;
						if(!(GetValueOfPosition(parents->position)==tie && Remoteness(parents->position)==REMOTENESS_MAX)) continue;
						pdat=GetOpenData(parents->position);
						old=pdat;
						if(GetCorruptionLevel(pdat)<i) continue;
						if(--gNumberChildren[parents->position]==0)
						{
							pdat=SetDrawValue(pdat,lose);
							pdat=SetLevelNumber(pdat,curLevel);
							pdat=SetCorruptionLevel(pdat,i);
							SetOpenData(parents->position,pdat);
							fringePositions[parents->position]=0;
							pdat=SetFringe(pdat,0);
							InsertLoseFR(parents->position);
							timeToBreak=1;
						}
						if(GetFremoteness(pdat) && GetFringe(pdat)) printf("Dis not good2.0\n");
						//winning positions cannot uncorrupt above them unless they've actually fixed the parent
						/*if(GetCorruptionLevel(pdat)>GetCorruptionLevel(dat) && GetDrawValue(pdat)==lose)
						   {
						        pdat=SetCorruptionLevel(pdat,GetCorruptionLevel(dat));
						        pdat=SetFremoteness(pdat,GetFremoteness(dat)+1);
						        if(GetFremoteness(pdat) && GetFringe(pdat)) printf("Dis not good2.1\n");
						   }
						   else*/if(GetCorruptionLevel(pdat)==GetCorruptionLevel(dat) && GetFremoteness(pdat)<GetFremoteness(dat)+1 && !GetFringe(pdat) && !(GetDrawValue(pdat)==win && GetCorruptionLevel(pdat)==i))
						{
							pdat=SetFremoteness(pdat,GetFremoteness(dat)+1);
							if(GetFremoteness(pdat) && GetFringe(pdat)) printf("Dis not good2.2\n");
						}
						if(GetDrawValue(old)==GetDrawValue(pdat) && GetCorruptionLevel(old)==GetCorruptionLevel(pdat) && GetFringe(old)) continue;
						SetOpenData(parents->position,pdat);
						if(GetFremoteness(pdat) && GetFringe(pdat)) printf("Dis not good2\n");
						if(pdat!=old) PropogateFreAndCorUpFringe(parents->position,0);
						if(GetDrawValue(old)==win && GetDrawValue(pdat)==lose) AddToParentsChildrenCount(parents->position,1);
					}
					//printf("Done fixing:\n");
					if(timeToBreak) break;
				}
			}
			/* undo our first step after having fixed */
			for(iter=0; iter<gNumberOfPositions; iter++)
			{
				OPEN_POS_DATA dat=GetOpenData(iter);
				if(GetCorruptionLevel(dat)>i && GetDrawValue(dat)==lose && GetLevelNumber(dat)==curLevel)
				{
					if(GetDrawValue(dat)==undecided) continue;
					AddToParentsChildrenCount(iter,1);
				}
			}
			printf("here2\n");
		}
		//PrintChildrenCounts();

		curLevel++;
	}
	/* One more thing... find loop lengths for fringe positions and label drawdraws */
	for(iter=0; iter<gNumberOfPositions; iter++)
	{
		int maxFremote=0;
		OPEN_POS_DATA dat=GetOpenData(iter);
		MOVELIST* moves;
		//printf("There%d\n",iter);
		//PrintSingleOpenData(iter);
		if(GetValueOfPosition(iter)==tie && Remoteness(iter)==REMOTENESS_MAX) {
			if (GetDrawValue(dat) == undecided) {
				dat=SetDrawValue(dat,tie);
				dat=SetFremoteness(dat,0xFFFFFFFF);
				SetOpenData(iter,dat);
				gAnalysis.DrawDraws +=1;                                        //MATT
			} else {
				gAnalysis.DetailedOpenSummary[GetLevelNumber(dat)][GetCorruptionLevel(dat)][GetFremoteness(dat)][GetDrawValue(dat)]+=1;
				gAnalysis.OpenSummary[GetDrawValue(dat)]+=1;
				if(GetCorruptionLevel(dat)>gAnalysis.LargestFoundCorruption) gAnalysis.LargestFoundCorruption=GetCorruptionLevel(dat);  //MATT/David
			}
			if(GetCorruptionLevel(dat)>GetLevelNumber(dat)) PrintSingleOpenData(iter);
			if(GetDrawValue(dat)==win && GetFremoteness(dat)==0) PrintSingleOpenData(iter);
		}
		else continue;
		if(!GetFringe(dat)) continue;
		if(GetFremoteness(dat)!=0)
		{
			printf("ACKKKK!\n");
			PrintSingleOpenData(iter);
		}
		moves=GenerateMoves(iter);
		for(; moves; moves=moves->next)
		{
			POSITION child=DoMove(iter,moves->move);
			OPEN_POS_DATA cdat=GetOpenData(child);
			if(!GetFringe(cdat) && GetLevelNumber(cdat)==GetLevelNumber(dat) && GetFremoteness(cdat)>maxFremote)
				maxFremote=GetFremoteness(cdat);
		}
		dat=SetFremoteness(dat,maxFremote+1);
		SetOpenData(iter,dat);
	}
	CleanupOpenPositions();
	return;
}

void CleanUpBeneathCL(int cl)
{
	POSITION p;
	OPEN_POS_DATA dat;
	if(!headNodeDP) return;
	p=DequeueDP();
	CleanUpBeneathCL(cl);
	dat=GetOpenData(p);
	if(GetDrawValue(dat)==undecided || GetCorruptionLevel(dat)>cl)
		EnqueueDP(p);
}

void PrintOpenDataFormatted(void)
{
	POSITION i;
	for(i=0; i<openPosArrLen; i++)
	{
		PrintSingleOpenData(i);
	}
}
void PrintSingleOpenData(POSITION p)
{
	OPEN_POS_DATA dat=GetOpenData(p);
	if(GetDrawValue(dat)==undecided) return;
	PrintPosition(p, "", 0);
	printf("Level number: %d\n",GetLevelNumber(dat));
	printf("OpenPosition value: ");
	switch(GetDrawValue(dat))
	{
	case win:
		printf("win");
		break;
	case lose:
		printf("lose");
		break;
	case tie:
		printf("tie");
		break;
	}
	printf("\nCorruption level: %d\n",GetCorruptionLevel(dat));
	printf("Fremoteness: %d\n",GetFremoteness(dat));
	printf("Fringe?: %s\n",GetFringe(dat) ? "yes" : "no");
}
void PrintOpenAnalysis(void)
{
	POSITION i;
	int wins=0, losses=0, ties=0;
	for(i=0; i<openPosArrLen; i++)
	{
		OPEN_POS_DATA dat=GetOpenData(i);
		if(GetDrawValue(dat)==undecided) continue;
		switch(GetDrawValue(dat))
		{
		case win:
			wins++;
			break;
		case lose:
			losses++;
			break;
		case tie:
			ties++;
			break;
		}
	}
	printf("Draw Wins: %d Draw Loses: %d Draw Ties: %d\n",wins,losses,ties);
}
MOVE ChooseSmartComputerMove(POSITION from, MOVELIST * moves, REMOTENESSLIST * remotenesses)
{
	MOVE m;
	MOVELIST *wM, *lM, *dM;
	REMOTENESSLIST *wF, *lF, *dF;
	char collectingFringes=0;
	int winCorrMax=0, loseCorrMin=CORRUPTION_MAX;
	wM=lM=dM=NULL; wF=lF=dF=NULL;
	/* If I'm not at a draw position, don't try to handle it like it is, just default */
	if(!(GetValueOfPosition(from)==tie && Remoteness(from)==REMOTENESS_MAX) || !OpenIsInitialized())
		return RandomSmallestRemotenessMove(moves, remotenesses);
	printf("\n\n\nChoosing From:\n");
	for(; moves; moves=moves->next)
	{
		POSITION child=DoMove(from,moves->move);
		OPEN_POS_DATA cdat=GetOpenData(child);
		if(GetDrawValue(cdat)==undecided || GetLevelNumber(cdat)!=GetLevelNumber(GetOpenData(from))) continue;
		PrintSingleOpenData(child);
		switch(GetDrawValue(cdat))
		{
		case win:
			if(GetCorruptionLevel(cdat)>winCorrMax)
			{
				FreeMoveList(wM);
				FreeRemotenessList(wF);
				winCorrMax=GetCorruptionLevel(cdat);
				wM=NULL;
				wF=NULL;
			}
			if(GetCorruptionLevel(cdat)==winCorrMax)
			{
				wM=CreateMovelistNode(moves->move,wM);
				wF=CreateRemotenesslistNode(GetFremoteness(cdat),wF);
			}
			break;
		case lose:
			if(GetCorruptionLevel(cdat)<loseCorrMin || (!collectingFringes && GetCorruptionLevel(cdat)==loseCorrMin && GetFringe(cdat)))
			{
				FreeMoveList(lM);
				FreeRemotenessList(lF);
				loseCorrMin=GetCorruptionLevel(cdat);
				lM=NULL;
				lF=NULL;
				collectingFringes=GetFringe(cdat);
			}
			if((!collectingFringes && GetCorruptionLevel(cdat)==loseCorrMin) || (collectingFringes && GetCorruptionLevel(cdat)==loseCorrMin && GetFringe(cdat)))
			{
				lM=CreateMovelistNode(moves->move,lM);
				lF=CreateRemotenesslistNode(GetFringe(cdat) ? 0 : GetFremoteness(cdat),lF);
			}
			break;
		case tie:
			dM=CreateMovelistNode(moves->move,dM);
			dF=CreateRemotenesslistNode(GetFremoteness(cdat),dF);
			break;
		}
	}
	if(lM)
	{
		m=RandomSmallestRemotenessMove(lM,lF);
	}
	else if(wM)
	{
		m=RandomLargestRemotenessMove(wM,wF);
	}
	else if(dM)
	{
		m=RandomSmallestRemotenessMove(dM,dF);
	}
	else {
		BadElse("GetSmartComputerMove");
		m=0;
	}
	FreeMoveList(lM);
	FreeMoveList(wM);
	FreeMoveList(dM);
	FreeRemotenessList(lF);
	FreeRemotenessList(wF);
	FreeRemotenessList(dF);
	printf("\n\n\nChoosing...\n\n\n");
	PrintSingleOpenData(DoMove(from,m));
	return m;
}

/**
 * Functions to save and load the open positions database.
 */

BOOLEAN SaveOpenPositionsData()
{
	char openDataFileName[256];
	char version = OPEN_FILE_VER;
	POSITION arraypos;

	if(gZeroMemPlayer)  //we don't save the db if playing on zero memory, but we will say that the db is saved
		return TRUE;

	Stopwatch();
	mkdir("data", 0755);
	sprintf(openDataFileName, "./data/m%s_%d_opendb.dat", kDBName, getOption());
	printf("\nSaving Open Positions DB for %s...", kGameName);

	/* Open file for reading */
	if((openData=fopen(openDataFileName, "wb")) == NULL) {
		printf("Failed!");
		Stopwatch();
		return FALSE;
	}

	/* Write file version */
	if(fwrite(&version, sizeof(char), 1, openData) != 1) {
		printf("Failed!");
		Stopwatch();
		return FALSE;
	}

	/* Write array */
	for(arraypos = 0; arraypos < openPosArrLen; arraypos++) {
		openPosData[arraypos] = htonl(openPosData[arraypos]);
		if(fwrite(&openPosData[arraypos], sizeof(OPEN_POS_DATA), 1, openData) != 1) {
			printf("Failed!");
			Stopwatch();
			return FALSE;
		}
		openPosData[arraypos] = ntohl(openPosData[arraypos]);
	}

	fclose(openData);
	gOpenDataLoaded = TRUE;
	printf("done in %u seconds!", Stopwatch());
	return TRUE;
}

BOOLEAN LoadOpenPositionsData()
{
	char openDataFileName[256];
	char version;
	POSITION arraypos;

	if(!gUseOpen) {
		return TRUE;
	}

	if(gZeroMemPlayer)  //we don't load the db if playing on zero memory, but we will say that the db is loaded
		return TRUE;

	Stopwatch();
	mkdir("data", 0755);
	sprintf(openDataFileName, "./data/m%s_%d_opendb.dat", kDBName, getOption());
	printf("\nLoading Open Positions DB for %s...", kGameName);

	/* Open file for reading */
	if((openData=fopen(openDataFileName, "rb")) == NULL) {
		printf("Failed!");
		Stopwatch();
		return FALSE;
	}

	/* Read file version */
	if(fread(&version, sizeof(char), 1, openData) != 1 || version != OPEN_FILE_VER) {
		printf("Failed!");
		Stopwatch();
		return FALSE;
	}

	/* Write array */
	for(arraypos = 0; arraypos < openPosArrLen; arraypos++) {
		if(fread(&openPosData[arraypos], sizeof(OPEN_POS_DATA), 1, openData) != 1) {
			printf("Failed!");
			Stopwatch();
			return FALSE;
		}
		openPosData[arraypos] = ntohl(openPosData[arraypos]);
	}

	fclose(openData);
	gOpenDataLoaded = TRUE;
	printf("done in %u seconds!", Stopwatch());
	return TRUE;
}

/**
 * Function for purity check
 * Hopefully able to reduce this to replace all open position data eventually
 */




















/*
** Globals
*/

REMOTENESS*  		gPositionLevel = NULL;  /* A list of each position's draw level */
VALUE*  			gPositionValue = NULL;	/* A list of each position's value (win/lose/undecided/draw) */
int 				level = 0;
int 				maxLevel = 0;
FRnode*         	gHeadWinDR = NULL;      /* The FRontier Win Queue */
FRnode*         	gTailWinDR = NULL;
FRnode*         	gHeadLoseDR = NULL;     /* The FRontier Lose Queue */
FRnode*         	gTailLoseDR = NULL;
FRnode*         	gHeadTieDR = NULL;      /* The FRontier Tie Queue */
FRnode*         	gTailTieDR = NULL;
POSITIONLIST**  	gDrawParents = NULL;        /* The Parent of each node in a list */
char*           	gDrawNumberChildren = NULL; /* The Number of children (used for Loopy games) */
char*       		gDrawNumberChildrenOriginal = NULL;
POSITION 			gNumWins[101]; // The reason we use 101 is because we set a hard max of 100 as the largest draw level and we needed a high bound on the array.
POSITION 			gNumLoses[101];


/*
** Local function prototypes
*/



BOOLEAN DeterminePure(POSITION position)
{
	BOOLEAN keepgoing = TRUE;
	BOOLEAN tmp = TRUE;

	/* initialize */
	InitializeDR();
	DrawParentInitialize();
	DrawNumberChildrenInitialize();
	/* Set each positions parent's list */
	SetDrawParents(kBadPosition,gInitialPosition);

	while(keepgoing) {

		DeterminePure1(gInitialPosition); /* Solve all positions we can */
		SetDrawParents(kBadPosition,gInitialPosition); 	/* Must be reset because the gDrawParents variable is manipulated in the DeterminePure1 function */
		SetNewLevelFringe(); /* Set all undecided positions with winning children to loses */
		level++;
		/* Check if we should stop */
		if (!ExistsUnsolvedPosition() || level > 100) {
			if (level > 100) {
				printf("Level over 100 break\n");
			}
			keepgoing = FALSE;
		}
	}

	/* Run the actual Purity check */
	tmp = isPure();

	/* This is for post-run Analysis */
	SetAnalysisOfDrawPositions();

	/* free */
	DrawNumberChildrenFree();  
	DrawParentFree();

	PrintDrawAnalysis();

	return tmp;
}

void DeterminePure1(POSITION position)
{
	POSITION child=kBadPosition, parent;
	POSITIONLIST *ptr;
	VALUE childValue;
	POSITION i;
	POSITION F0EdgeCount = 0;
	POSITION F0NodeCount = 0;
	POSITION F0DrawEdgeCount = 0;

	/* Do DFS to set up Parent pointers and initialize KnownList w/Primitives */

	if(kDebugDetermineValue) {
		printf("---------------------------------------------------------------\n");
		printf("Number of Positions = [" POSITION_FORMAT "]\n",gNumberOfPositions);
		printf("---------------------------------------------------------------\n");
		// MyPrintParents();
		printf("---------------------------------------------------------------\n");
		//MyPrintFR();
		printf("---------------------------------------------------------------\n");
	}

	/* Now, the fun part. Starting from the children, work your way back up. */
	//@@ separate lose/win frontiers
	while ((gHeadLoseDR != NULL) ||
	       (gHeadWinDR != NULL)) {

		if ((child = DeQueueLoseDR()) == kBadPosition)
			child = DeQueueWinDR();

		/* Might as well grab these now, they'll be used later */
		childValue = gPositionValue[child];

		/* If debugging, print who's in list */
		if(kDebugDetermineValue)
			printf("Grabbing " POSITION_FORMAT " (%s) remoteness = 0 off of FR\n",
			       child,gValueString[childValue]);

		/* With losing children, every parent is winning, so we just go through
		** all the parents and declare them winning */

		if (childValue == lose) {
			ptr = gDrawParents[child];
			while (ptr != NULL) {

				/* Make code easier to read */
				parent = ptr->position;

				/* Skip if this is the initial position (parent is kBadPosition) */
				if (parent != kBadPosition) {
					if (gPositionValue[parent] == undecided) {
						/* This is the first time we know the parent is a win */
						InsertWinDR(parent);
						if(kDebugDetermineValue) printf("Inserting " POSITION_FORMAT " (%s) remoteness = 0 into win DR\n",parent,"win");
						gPositionValue[parent] = win;
						gPositionLevel[parent] = level;
						if (level > maxLevel) {
							maxLevel = level;
						}
					}
					else {
						/* We already know the parent is a winning position. */

						if (gPositionValue[parent] != win) {
							// printf(POSITION_FORMAT " should be win.  Instead it is %d.\n", parent, gPositionValue[parent]);
							// if (gPositionValue[parent] == lose) {
							// 	printf("This is an early indicator of Impurity. We might be able to stop here. I'll ask Dan Tomorrow\n");
							// 	// PrintPosition(child, "child", TRUE);
							// 	// PrintPosition(parent, "parent", TRUE);
							// }
							// BadElse("DetermineLoopyValue");
						}

						/* This should always hold because the frontier is a queue.
						** We always examine losing nodes with less remoteness first */
					}
				}
				ptr = ptr->next;
			} /* while there are still parents */
			if (gDrawParents[child] == NULL) {
				printf("gParents is NULL. The copy didn't work\n");
			}

			/* With winning children */
		} else if (childValue == win) {
			ptr = (gDrawParents[child]);
			while (ptr != NULL) {

				/* Make code easier to read */
				parent = ptr->position;
				//printf("parent: %llu NumberOfChildren: %d\n", parent, gDrawNumberChildren[parent]);

				/* Skip if this is the initial position (parent is kBadPosition) */
				/* If this is the last unknown child and they were all wins, parent is lose */
				if(parent != kBadPosition && --gDrawNumberChildren[parent] == 0) {
					/* no more kids, it's not been seen before, assign it as losing, put at head */
					// assert(gPositionValue[parent] == undecided);
					if (gPositionValue[parent] != undecided) {
						if (gPositionValue[parent] != lose) {
							printf("Parent's Value: %d. Parent: %llu. Child: %llu\n", gPositionValue[parent], parent, child);
							assert(FALSE);
						}
					}
					F0EdgeCount -= (gDrawNumberChildrenOriginal[parent] - 1);
					InsertLoseDR(parent);
					if(kDebugDetermineValue) printf("Inserting " POSITION_FORMAT " (%s) into DR head\n",parent,"lose");
					/* We always need to change the remoteness because we examine winning node with
					** less remoteness first. */
					gPositionValue[parent] = lose;
					gPositionLevel[parent] = level;
					if (level > maxLevel) {
						maxLevel = level;
					}
				} else if (parent != kBadPosition) {
					F0EdgeCount++;
				}
				ptr = ptr->next;
			} /* while there are still parents */
			if (gDrawParents[child] == NULL) {
				printf("gParents is NULL. The copy didn't work\n");
			}

			/* With children set to other than win/lose. So stop */
		} else {
			BadElse("DetermineLoopyValue found DR member with other than win/lose value");
		} /* else */

		/* We are done with this position and no longer need to keep around its list of parents
		** The tie frontier will not need this, either, because this child's value has already
		** been determined.  It cannot be a tie. */

	} /* while still positions in FR */

	/* Now process the tie frontier */
	while(gHeadTieDR != NULL) {
		child = DeQueueTieDR();

		ptr = gDrawParents[child];

		while (ptr != NULL) {
			parent = ptr->position;

			if(parent != kBadPosition && gPositionValue[parent] == undecided) {
				/* this position has no losing children but has a tieing position so it must be a
				 * tie. Assign its value and set its remoteness.  Note that
				 * we give ties with lowest remoteness priority (i.e. if a
				 * position has no losing children, a tieing child of
				 * remoteness 2, and a tieing child of remoteness 10, the
				 * position will be a tie of remoteness 3, not 11.  This
				 * decision is pretty arbitrary.  We did it this way to be
				 * consistent with DetermineValue for non-loopy games. */

				InsertTieDR(parent);
				if(kDebugDetermineValue) printf("Inserting " POSITION_FORMAT " (%s) remoteness = 0 into win FR\n",parent,"tie");
				gPositionValue[parent] = tie;
				gPositionLevel[parent] = level;
				if (level > maxLevel) {
					maxLevel = level;
				}
				/*
				   gNumberChildren[parent] -= 1;
				   gNumberChildrenOriginal[parent] -=1; //As it is now, fringe0 can't have tie children
				 */
			}
			ptr = ptr->next;
		}
		if (gDrawParents[child] == NULL) {
				printf("gParents is NULL. The copy didn't work\n");
			}
	}

	/* Now set all remaining positions to tie with remoteness of REMOTENESS_MAX */

	if(kDebugDetermineValue) {
		printf("---------------------------------------------------------------\n");
		//MyPrintFR();
		printf("---------------------------------------------------------------\n");
		MyPrintParents();
		printf("---------------------------------------------------------------\n");
		printf("TIE cleanup\n");
	}

	for (i = 0; i < gNumberOfPositions; i++)
		if(Visited(i)) {
			if(kDebugDetermineValue)
				printf(POSITION_FORMAT " was visited...",i);
			if(gPositionValue[i] == undecided) {
				// gPositionValue[(POSITION)i] = tie;
				// gPositionLevel[(POSITION)i] = level;
				// if (gDrawNumberChildren[i] < gDrawNumberChildrenOriginal[i]) {
				// 	F0DrawEdgeCount += gDrawNumberChildren[i];
				// 	F0NodeCount+=1;
				// }
				//we are done with this position and no longer need to keep around its list of parents
				/*if (gParents[child])
				   FreePositionList(gParents[child]); */                                       // is this a memory leak?
				if(kDebugDetermineValue)
					printf("and was undecided, setting to draw\n");
			} else {
				if(kDebugDetermineValue)
					printf("but was decided, ignoring\n");
			}
			UnMarkAsVisited((POSITION)i);
		}

	if (gInterestingness) {
		DetermineInterestingness(position);
	}
}

/*
** Requires: the root has not been visited yet
** (We do not check to see if its been visited)
*/

void SetDrawParents (POSITION parent, POSITION root)
{
	MOVELIST*       moveptr;
	MOVELIST*       movehead;
	POSITIONLIST*   posptr;
	POSITIONLIST*   thisLevel;
	POSITIONLIST*   nextLevel;
	POSITION pos;
	POSITION child;
	VALUE value;

	posptr = thisLevel = nextLevel = NULL;
	moveptr = movehead = NULL;

	// Check if the top is primitive.
	// MarkAsVisited(root);
	// gDrawParents[root] = StorePositionInList(parent, gDrawParents[root]);
	// if ((value = Primitive(root)) != undecided) {
	// 	switch (value) {
	// 	case lose: InsertLoseDR(root); break;
	// 	case win:  InsertWinDR(root); break;
	// 	case tie:  InsertTieDR(root); break;
	// 	default:   BadElse("SetParents found primitive with value other than win/lose/tie");
	// 	}

	// 	// DOUBLE CHECK
	// 	gPositionValue[root] = value;
	// 	gPositionLevel[root] = level;
	// 	return;
	// }

	thisLevel = StorePositionInList(root, thisLevel);

	while (thisLevel != NULL) {
		POSITIONLIST* next;

		for (posptr = thisLevel; posptr != NULL; posptr = next) {
			next = posptr->next;
			pos = posptr->position;

			movehead = GenerateMoves(pos);

			for (moveptr = movehead; moveptr != NULL; moveptr = moveptr->next) {
				child = DoMove(pos, moveptr->move);
				if (gSymmetries)
					child = gCanonicalPosition(child);

				if (child >= gNumberOfPositions)
					FoundBadPosition(child, pos, moveptr->move);
				++gDrawNumberChildren[(int)pos];
				++gDrawNumberChildrenOriginal[(int)pos];
				gDrawParents[(int)child] = StorePositionInList(pos, gDrawParents[(int)child]);

				if (Visited(child)) continue;
				MarkAsVisited(child);

				if ((value = Primitive(child)) != undecided) {
					switch (value) {
					case lose: InsertLoseDR(child); break;
					case win: InsertWinDR(child);  break;
					case tie: InsertTieDR(child);  break;
					default: BadElse("SetParents found bad primitive value");
					}
					gPositionValue[child] = value;
					if (level > maxLevel) {
						maxLevel = level;
					}
				} else {
					nextLevel = StorePositionInList(child, nextLevel);
				}
				gTotalMoves++;
			}
			FreeMoveList(movehead);

			/* Free as we go */
			free(posptr);
		}

		thisLevel = nextLevel;
		nextLevel = NULL;
	}
}


//void InitializeVisitedArray()
//{
//    size_t sz = (gNumberOfPositions >> 3) + 1;
//    gVisited = (char*) SafeMalloc (sz);
//    memset(gVisited, 0, sz);
//}

//void FreeVisitedArray()
//{
//    if (gVisited) SafeFree(gVisited);
//    gVisited = NULL;
//}

void DrawParentInitialize()
{
	POSITION i;

	gDrawParents = (POSITIONLIST **) SafeMalloc (gNumberOfPositions * sizeof(POSITIONLIST *));
	gPositionValue = (VALUE *) SafeMalloc (gNumberOfPositions * sizeof(VALUE));
	gPositionLevel = (REMOTENESS *) SafeMalloc (gNumberOfPositions * sizeof(REMOTENESS));
	for(i = 0; i < gNumberOfPositions; i++) {
		gDrawParents[i] = NULL;
		gPositionValue[i] = GetValueOfPosition((POSITION) i);
		gPositionLevel[i] = 0;
		if (gPositionValue[i] == tie) {
			gPositionValue[i] = undecided;
		}
	}
}

void DrawParentFree()
{
	POSITION i;

	for (i = 0; i < gNumberOfPositions; i++) {
		FreePositionList(gDrawParents[i]);
	}

	SafeFree(gDrawParents);
}

void DrawNumberChildrenInitialize()
{
	POSITION i;

	gDrawNumberChildren = (char *) SafeMalloc (gNumberOfPositions * sizeof(signed char));
	gDrawNumberChildrenOriginal = (char *) SafeMalloc (gNumberOfPositions * sizeof(signed char));
	if (gInterestingness) {
		gAnalysis.Interestingness = (float *) SafeMalloc (gNumberOfPositions * sizeof(float)); /* Interestingness */
	}
	if (gInterestingness) {
		for(i = 0; i < gNumberOfPositions; i++) {
			gDrawNumberChildren[i] = 0;
			gDrawNumberChildrenOriginal[i] = 0;
			gAnalysis.Interestingness[i] = 0.0;
		}
	} else {
		for(i = 0; i < gNumberOfPositions; i++) {
			gDrawNumberChildren[i] = 0;
			gDrawNumberChildrenOriginal[i] = 0;
		}
	}
}

void DrawNumberChildrenFree()
{
	SafeFree(gDrawNumberChildren);
	SafeFree(gDrawNumberChildrenOriginal);
	SafeFree(gPositionLevel);
	SafeFree(gPositionValue);
}

void InitializeDR()
{
	gHeadWinDR = NULL;
	gTailWinDR = NULL;
	gHeadLoseDR = NULL;
	gTailLoseDR = NULL;
	gHeadTieDR = NULL;
	gTailTieDR = NULL;
}

static POSITION DeQueueDR(FRnode **gHeadFR, FRnode **gTailFR)
{
	POSITION position;
	FRnode *tmp;

	if (*gHeadFR == NULL)
		return kBadPosition;
	else {
		position = (*gHeadFR)->position;
		tmp = *gHeadFR;
		(*gHeadFR) = (*gHeadFR)->next;
		SafeFree(tmp);

		if (*gHeadFR == NULL)
			*gTailFR = NULL;
	}
	return position;
}

POSITION DeQueueWinDR()
{
	return DeQueueDR(&gHeadWinDR, &gTailWinDR);
}

POSITION DeQueueLoseDR()
{
	return DeQueueDR(&gHeadLoseDR, &gTailLoseDR);
}

POSITION DeQueueTieDR()
{
	return DeQueueDR(&gHeadTieDR, &gTailTieDR);
}

static void InsertDR(POSITION position, FRnode **firstnode,
                     FRnode **lastnode)
{
	FRnode *tmp = (FRnode *) SafeMalloc(sizeof(FRnode));
	tmp->position = position;
	tmp->next = NULL;

	if (*lastnode == NULL) {
		assert(*firstnode == NULL);
		*firstnode = tmp;
		*lastnode = tmp;
	} else {
		assert((*lastnode)->next == NULL);
		(*lastnode)->next = tmp;
		*lastnode = tmp;
	}
}

void InsertWinDR(POSITION position)
{
	/* printf("Inserting WinFR...\n"); */
	InsertDR(position, &gHeadWinDR, &gTailWinDR);
}

void InsertLoseDR(POSITION position)
{
	/* printf("Inserting LoseFR...\n"); */
	InsertDR(position, &gHeadLoseDR, &gTailLoseDR);
}

void InsertTieDR(POSITION position)
{
	InsertDR(position, &gHeadTieDR, &gTailTieDR);
}

void SetNewLevelFringe()
{
	POSITIONLIST* ptr = NULL;
	POSITION 	  parent;
	POSITION 	  i;
	for(i = 0; i < gNumberOfPositions; i++) {
		if (gPositionValue[i] == win) {
			ptr = gDrawParents[i];
			while (ptr != NULL) {
				parent = ptr->position;
				if (gPositionValue[parent] == undecided) {
					gPositionValue[parent] = lose;
					gPositionLevel[parent] = level + 1;
					if (level + 1 > maxLevel) {
						maxLevel = level + 1;
					}
					InsertLoseDR(parent);
				}
				ptr = ptr->next;
			}
		}
	}
}

BOOLEAN ExistsUnsolvedPosition()
{
	POSITION i;

	for (i=0; i<gNumberOfPositions; i++) {
		if (GetValueOfPosition(i) != undecided) {
			if (gPositionValue[i] == undecided || gPositionValue[i] == tie) {
				return TRUE;
			}
		}
	} 
	return FALSE;
}

BOOLEAN isPure()
{
	POSITION i, parent;
	POSITIONLIST* ptr;

	SetDrawParents(kBadPosition, gInitialPosition);
	for(i=0; i<gNumberOfPositions; i++) {
		if (gPositionValue[i] == lose) {
			ptr = gDrawParents[i];
			while (ptr != NULL) {
				parent = ptr->position;
				if(gPositionValue[parent] == lose) {
					// DrawNumberChildrenFree();
					// DrawParentFree();
					// PrintPosition(parent, "parent", TRUE);
					// PrintPosition(i, "child", TRUE);
					return FALSE;
				}
				ptr = ptr->next;
			}
		}
	}

	return TRUE;
}

void SetAnalysisOfDrawPositions() {
	POSITION i;
	int k;
	
	// numWins = (int *) SafeMalloc ((maxLevel + 1) * sizeof(int));
	// numLoses = (int *) SafeMalloc ((maxLevel + 1) * sizeof(int));

	for (k=0; k<=maxLevel; k++) {
		gNumWins[k] = 0;
		gNumLoses[k] = 0;
	}

	for (i=0; i < gNumberOfPositions; i++) {
		if (gPositionValue[i] == win) {
			gNumWins[gPositionLevel[i]]++;
		} else if (gPositionValue[i] == lose) {
			gNumLoses[gPositionLevel[i]]++;
		}
	}

	// SET SOMETHING ELSE

	// SafeFree(numWins);
	// SafeFree(numLoses);
}

void PrintDrawAnalysis() {
	int k;

	for (k=0; k <= maxLevel; k++) {
	printf("Draw Level Analysis:\n");
	printf("\tDraw Level          Win         Lose          Total\n");
	printf("\t------------------------------------------------------------------------------\n");
	printf("\t         0   %10llu   %10llu   %10llu\n", gNumWins[0], gNumLoses[0], gNumWins[0] + gNumLoses[0]);
	for(k=1; k <= maxLevel; k++) {
		printf("\t%10d   %10llu   %10llu   %10llu\n", k, gNumWins[k], gNumLoses[k], gNumWins[k] + gNumLoses[k]);
		}
	}
	printf("\n");
}

// End Loopy


/* End of purity functions */
