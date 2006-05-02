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
	for(p=openPosData;p<openPosData+numPossiblePositions;p++)
		*p=init;
	for(c=corruptedPositions;c<corruptedPositions+numPossiblePositions;c++)
		*c=0;
	for(c=fringePositions;c<fringePositions+numPossiblePositions;c++)
		*c=0;
	openPosArrLen=numPossiblePositions;
	while(headNodeDP) DequeueDP();
	return;
}
int OpenIsInitialized(void)
{
	return openPosData?1:0;
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
	for(x=0;x<gNumberOfPositions;x++)
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
	for(;moves;moves=moves->next)
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
	for(;parents;parents=parents->next)
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
					if(!fringe) pdat=SetFringe(pdat,0);/*
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
	for(;parents;parents=parents->next)
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
		for(iter=0;iter<gNumberOfPositions;iter++)
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
					for(;moves;moves=moves->next)
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
			gAnalysis.LargestFoundLevel = curLevel-1;				//MATT
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
				for(;parents;parents=parents->next)
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
				for(;parents;parents=parents->next)
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
		for(i=0;i<curLevel;i++)
		{
			/* load the win/lose frontier and subtract off children from the counts of their parents if they're not in the
			   current level and they are losing */
			for(iter=0;iter<gNumberOfPositions;iter++)
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
					for(;parents;parents=parents->next)
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

					for(;parents;parents=parents->next)
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
						else*/ if(GetCorruptionLevel(pdat)==GetCorruptionLevel(dat) && GetFremoteness(pdat)<GetFremoteness(dat)+1 && !GetFringe(pdat) && !(GetDrawValue(pdat)==win && GetCorruptionLevel(pdat)==i))
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
			for(iter=0;iter<gNumberOfPositions;iter++)
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
	for(iter=0;iter<gNumberOfPositions;iter++)
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
				gAnalysis.DrawDraws +=1;					//MATT
			} else {
				gAnalysis.DetailedOpenSummary[GetLevelNumber(dat)][GetCorruptionLevel(dat)][GetFremoteness(dat)][GetDrawValue(dat)]+=1;
				gAnalysis.OpenSummary[GetDrawValue(dat)]+=1;
				if(GetCorruptionLevel(dat)>gAnalysis.LargestFoundCorruption) gAnalysis.LargestFoundCorruption=GetCorruptionLevel(dat);	//MATT/David
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
		for(;moves;moves=moves->next)
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
	for(i=0;i<openPosArrLen;i++)
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
	printf("Fringe?: %s\n",GetFringe(dat)?"yes":"no");
}
void PrintOpenAnalysis(void)
{
	POSITION i;
	int wins=0, losses=0, ties=0;
	for(i=0;i<openPosArrLen;i++)
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
	wM=lM=dM=NULL;wF=lF=dF=NULL;
	/* If I'm not at a draw position, don't try to handle it like it is, just default */
	if(!(GetValueOfPosition(from)==tie && Remoteness(from)==REMOTENESS_MAX) || !OpenIsInitialized())
		return RandomSmallestRemotenessMove(moves, remotenesses);
	printf("\n\n\nChoosing From:\n");
	for(;moves;moves=moves->next)
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
				lF=CreateRemotenesslistNode(GetFringe(cdat)?0:GetFremoteness(cdat),lF);
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
	else
		BadElse("GetSmartComputerMove");
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
