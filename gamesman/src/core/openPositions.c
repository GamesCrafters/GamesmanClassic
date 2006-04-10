/*
* Experimental Open Positions code by David Eitan Poll
* Exploring re-evaluation of Draw Positions
*/
#include <stdlib.h>
#include <stdio.h>
#include "openPositions.h"
#include "solveloopy.h"

OPEN_POS_DATA* openPosData;
POSITION openPosArrLen;
POSITIONLIST* headNodeDP;
POSITIONLIST* tailNodeDP;
char* corruptedPositions;
char* fringePositions;
extern char* gNumberChildren;
extern char* gNumberChildrenOriginal;
extern POSITION gNumberOfPositions;
extern POSITIONLIST** gParents;

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
void CleanupOpenPositions(void)
{
	if(!openPosData) return;
	//printf("HERE!!!!!\n");
	SafeFree(openPosData);
	//printf("HERE!!!!!\n");
	SafeFree(corruptedPositions);
	//printf("HERE!!!!!\n");
	SafeFree(fringePositions);
	openPosData=NULL;
	//printf("HERE!!!!!\n");
	while(headNodeDP) DequeueDP();
	//printf("HERE!!!!!\n");
	tailNodeDP=NULL;
	return;
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
	openPosData[pos]=value;
}
OPEN_POS_DATA GetOpenData(POSITION pos)
{
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
		if(fringe && !GetFringe(pdat)) continue;
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
					if(!fringe) pdat=SetFringe(pdat,0);
				}
				else if(GetCorruptionLevel(dat)==GetCorruptionLevel(pdat) && GetFremoteness(dat)+1>GetFremoteness(pdat))
				{
					pdat=SetFremoteness(pdat,GetFremoteness(dat)+1);
					if(!fringe) pdat=SetFringe(pdat,0);
				}
				break;
			}
			break;
		case lose:
			if(GetDrawValue(pdat)==win)
			{
				if(GetCorruptionLevel(dat)<GetCorruptionLevel(pdat))
				{
					pdat=SetCorruptionLevel(pdat,GetCorruptionLevel(dat));
					pdat=SetFremoteness(pdat,GetFremoteness(dat)+1);
					if(!fringe) pdat=SetFringe(pdat,0);
				}
				else if(GetCorruptionLevel(dat)==GetCorruptionLevel(pdat) && GetFremoteness(dat)+1<GetFremoteness(pdat))
				{
					pdat=SetFremoteness(pdat,GetFremoteness(dat)+1);
					if(!fringe) pdat=SetFringe(pdat,0);
				}
			}
			break;
		}
		if(!fringe && fringePositions[parents->position] && GetDrawValue(pdat)==lose)
		{
			pdat=SetFremoteness(pdat,0);
			pdat=SetFringe(pdat,1);
		}
		if(pdat!=old)
		{
			SetOpenData(parents->position,pdat);
			PropogateFreAndCorUpFringe(parents->position,fringe);
		}
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
	POSITIONLIST* ptr;
	int curLevel=1;
	POSITION iter;

	if(!openPosData) return;
	//PrintChildrenCounts();

	InitializeFR();

	while(1)
	{
		int fringePosCount=0;
		int maxCorruption=0;
		int i;
		
		//printf("Doing this\n");
		/* first, find all fringe positions */
		for(iter=0;iter<gNumberOfPositions;iter++)
		{
			OPEN_POS_DATA dat=GetOpenData(iter);
			/* if the number of children of an undecided value is less than the original number of children but >0, it
			   has a winning child and is therefore a fringe */
			if(gNumberChildren[iter]&&gNumberChildren[iter]<gNumberChildrenOriginal[iter]&&GetDrawValue(GetOpenData(iter))==undecided && GetValueOfPosition(iter)==tie && Remoteness(iter)==REMOTENESS_MAX)
			{
				if(curLevel==1) dat=SetCorruptionLevel(dat,0);
				SetOpenData(iter,SetFringe(SetLevelNumber(SetFremoteness(SetDrawValue(dat,lose),0),curLevel),1));
				InsertLoseFR(iter);
				fringePosCount++;
				fringePositions[iter]=1;
			}
		}
		/* if we didn't find any fringe positions, we just label everyone else as pure ties */
		if(fringePosCount==0) break;
		/* next, identify corruption (losing nodes with losing children) */
		for(ptr=gHeadLoseFR;ptr;ptr=ptr->next)
		{
			OPEN_POS_DATA dat;
			POSITIONLIST* parents;
			dat=GetOpenData(ptr->position);
			parents=gParents[ptr->position];
			//printf("Child: %d\n",ptr->position);
			if(GetDrawValue(dat)!=lose) continue;
			for(;parents;parents=parents->next)
			{
				OPEN_POS_DATA pdat;
				//printf("Parent: %d\n",parents->position);
				if(corruptedPositions[parents->position]) continue;
				pdat=GetOpenData(parents->position);
				if(GetDrawValue(pdat)==lose)
				{
					corruptedPositions[parents->position]=1;
					SetOpenData(parents->position,SetCorruptionLevel(pdat,GetCorruptionLevel(pdat)+1));
					if(GetCorruptionLevel(pdat)+1>maxCorruption)
						maxCorruption=GetCorruptionLevel(pdat)+1;
				}
			}
		}
		//printf("UGH!\n");
		//PrintOpenDataFormatted();
		/* do paper-ish solving of the level.  Corruption and Fremoteness propogate up here */
		while(gHeadLoseFR || gHeadWinFR)
		{
			//printf("HERE\n");
			while(gHeadLoseFR)
			{
				POSITION pos=DeQueueLoseFR();
				POSITIONLIST* parents=gParents[pos];
				OPEN_POS_DATA dat=GetOpenData(pos);
				for(;parents;parents=parents->next)
				{
					OPEN_POS_DATA pdat;
					OPEN_POS_DATA old;
					if(!(GetValueOfPosition(parents->position)==tie && Remoteness(parents->position)==REMOTENESS_MAX)) continue;
					pdat=GetOpenData(parents->position);
					//printf("Parent: %d\n",parents->position);
					if(fringePositions[parents->position]) continue;
					old=pdat;
					pdat=SetDrawValue(pdat,win);
					pdat=SetLevelNumber(pdat,curLevel);
					if(GetFremoteness(pdat)>GetFremoteness(dat)+1 || GetDrawValue(old)==undecided)
						pdat=SetFremoteness(pdat,GetFremoteness(dat)+1);
					if(GetCorruptionLevel(pdat)>GetCorruptionLevel(dat) || GetCorruptionLevel(pdat)==CORRUPTION_MAX)
						pdat=SetCorruptionLevel(pdat,GetCorruptionLevel(dat));
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
					if(fringePositions[parents->position]) continue;
					pdat=GetOpenData(parents->position);
					if(--gNumberChildren[parents->position]==0)
					{
						pdat=SetDrawValue(pdat,lose);
						pdat=SetLevelNumber(pdat,curLevel);
						SetOpenData(parents->position,pdat);
						InsertLoseFR(parents->position);
						timeToBreak=1;
					}
					old=pdat;
					if(GetFremoteness(pdat)<GetFremoteness(dat)+1 && !fringePositions[parents->position])
						pdat=SetFremoteness(pdat,GetFremoteness(dat)+1);
					if(GetCorruptionLevel(pdat)>GetCorruptionLevel(dat) || GetCorruptionLevel(pdat)==CORRUPTION_MAX)
						pdat=SetCorruptionLevel(pdat,GetCorruptionLevel(dat));
					SetOpenData(parents->position,pdat);
					if(pdat!=old) PropogateFreAndCorUp(parents->position);
				}
				if(timeToBreak) break;
			}
		}
		/* all that's left is to do fixing at each corruption level. */
		for(i=0;i<curLevel;i++)
		{
			/* load the win/lose frontier and subtract off children from the counts of their parents if they're not in the
			   current level and they are losing */
			for(iter=0;iter<gNumberOfPositions;iter++)
			{
				OPEN_POS_DATA dat=GetOpenData(iter);
				if((GetCorruptionLevel(dat)>i && GetDrawValue(dat)==lose && GetLevelNumber(dat)==curLevel) || corruptedPositions[iter])
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
			while(gHeadLoseFR || gHeadWinFR)
			{
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
						pdat=SetDrawValue(pdat,win);
						pdat=SetLevelNumber(pdat,curLevel);
						fringePositions[parents->position]=0;
						if(GetCorruptionLevel(pdat)<i) continue;
						if(GetCorruptionLevel(pdat)>i)
						{
							pdat=SetCorruptionLevel(pdat, i);
							pdat=SetFremoteness(pdat, GetFremoteness(dat)+1);
						}
						if((GetFremoteness(pdat)>GetFremoteness(dat)+1 || GetDrawValue(old)!=GetDrawValue(pdat)) && !fringePositions[parents->position])
							pdat=SetFremoteness(pdat,GetFremoteness(dat)+1);
						SetOpenData(parents->position,pdat);
						if(pdat!=old) PropogateFreAndCorUp(parents->position);
						if(GetDrawValue(pdat)!=GetDrawValue(old))
						{
							InsertWinFR(parents->position);
						}
						if(GetDrawValue(pdat)==win && GetDrawValue(old)==lose)
							AddToParentsChildrenCount(parents->position,1);
					}
				}
				while(gHeadWinFR)
				{
					POSITION pos=DeQueueWinFR();
					POSITIONLIST* parents=gParents[pos];
					OPEN_POS_DATA dat=GetOpenData(pos);
					char timeToBreak=0;

					//printf("Fixing:\n");

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
							SetOpenData(parents->position,pdat);
							fringePositions[parents->position]=0;
							InsertLoseFR(parents->position);
							timeToBreak=1;
						}
						
						if(GetCorruptionLevel(pdat)>GetCorruptionLevel(dat))
						{
							pdat=SetCorruptionLevel(pdat,GetCorruptionLevel(dat));
							pdat=SetFremoteness(pdat,GetFremoteness(dat)+1);
						}
						else if(GetCorruptionLevel(pdat)==GetCorruptionLevel(dat) && GetFremoteness(pdat)<GetFremoteness(dat)+1 && !fringePositions[parents->position] && !(GetDrawValue(pdat)==win && GetCorruptionLevel(pdat)==i))
							pdat=SetFremoteness(pdat,GetFremoteness(dat)+1);
						SetOpenData(parents->position,pdat);
						if(pdat!=old) PropogateFreAndCorUp(parents->position);
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
		}
		//PrintChildrenCounts();
		curLevel++;
	}
	/* finally, everything that is still unmarked is a drawdraw */
	for(ptr=headNodeDP;ptr;ptr=ptr->next)
	{
		OPEN_POS_DATA dat=GetOpenData(ptr->position);
		if(GetValueOfPosition(ptr->position)==tie && Remoteness(ptr->position)==REMOTENESS_MAX && GetDrawValue(dat)==undecided)
		{
			dat=SetDrawValue(dat,tie);
			dat=SetFremoteness(dat,0xFFFFFFFF);
			SetOpenData(ptr->position,dat);
		}
	}
	/* One more thing... find loop lengths for fringe positions */
	for(iter=0;iter<gNumberOfPositions;iter++)
	{
		int maxFremote=0;
		OPEN_POS_DATA dat=GetOpenData(iter);
		MOVELIST* moves;
		if(!GetFringe(dat)) continue;
		moves=GenerateMoves(iter);
		for(;moves;moves=moves->next)
		{
			POSITION child=DoMove(iter,moves->move);
			OPEN_POS_DATA cdat=GetOpenData(iter);
			if(!GetFringe(cdat) && GetLevelNumber(cdat)==GetLevelNumber(dat) && GetFremoteness(cdat)>maxFremote)
				maxFremote=GetFremoteness(cdat);
		}
		dat=SetFremoteness(dat,maxFremote+1);
		SetOpenData(iter,dat);
	}
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
		OPEN_POS_DATA dat=GetOpenData(i);
		if(GetDrawValue(dat)==undecided) continue;
		PrintPosition(i, "", 0);
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

