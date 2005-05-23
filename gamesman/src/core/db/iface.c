/************************************************************************
**
** NAME:	iface.c
**
** DESCRIPTION:	Generic Database Functions and Database Class Accessors
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

/*
** Needs to be built up to implement The new iface Class abstraction as is
** Found in the Expeimental directory. However we first need to make
** The existing functions abstract.
*/


#include "gamesman.h"
#include "iface.h"
#include "memdb.h"
#include "twobitdb.h"
#include "colldb.h"
#include "globals.h"

iface_Table *iface_functions;
int accesscount;
int remotecount;
int visitcount;
int unmarkcount;
int markcount;
int truecount;
/*
** Code
*/

void iface_initialize(){
	accesscount = 0;
	remotecount = 0;
	visitcount = 0;
	unmarkcount = 0;
	markcount = 0;
	truecount = 0;
    if(iface_functions){
        iface_functions->free_iface();
    }
    if (gTwoBits) {
        iface_functions = twobitiface_init();
    } else if(gColliface){
      iface_functions = colliface_init();
    } else {
        iface_functions = memiface_init();
    }
}

void iface_free(){
    if(iface_functions) {
	iface_functions->free_iface();
	SafeFree(iface_functions);
    }
}

VALUE StoreValueOfPosition(POSITION position, VALUE value)
{
	accesscount++;
	if(accesscount%1000 == 0)
	  printf("accesscount: %d\n",accesscount);
    return iface_functions->put_value(position,value);
}


// This is it
VALUE GetValueOfPosition(POSITION position)
{
  accesscount++;
  if(accesscount%1000 == 0)
	  printf("accesscount: %d\n",accesscount);
  if(gMenuMode == Evaluated && gSymmetries)
    position = gCanonicalPosition(position);

  return iface_functions->get_value(position);
}


REMOTENESS Remoteness(POSITION position)
{ 
  remotecount++;
  if(remotecount%1000 == 0)
	  printf("remotecount: %d\n",remotecount);
  if(gMenuMode == Evaluated && gSymmetries)
	position = gCanonicalPosition(position);
  
  return iface_functions->get_remoteness(position);
}
    

void SetRemoteness (POSITION position, REMOTENESS remoteness)
{
	remotecount++;
  if(remotecount%1000 == 0)
	  printf("remotecount: %d\n",remotecount);
     if(iface_functions->put_remoteness != NULL)
        iface_functions->put_remoteness(position,remoteness);
}
 

BOOLEAN Visited(POSITION position)
{
	BOOLEAN temp;
	temp = iface_functions->check_visited(position);
	visitcount++;
	if(temp)
		truecount++;
  if(visitcount%1000 == 0)
	  printf("visitcount: %d unmarkcount: %d markcount: %d truecount: %d\n",visitcount,unmarkcount,markcount,truecount);
    
	return temp;
}


void MarkAsVisited (POSITION position)
{
	visitcount++;
	markcount++;
  if(visitcount%1000 == 0)
	  printf("visitcount: %d unmarkcount: %d markcount: %d truecount: %d\n",visitcount,unmarkcount,markcount,truecount);
    return iface_functions->mark_visited(position);
}

void UnMarkAsVisited (POSITION position)
{
	visitcount++;
	unmarkcount++;
  if(visitcount%1000 == 0)
	  printf("visitcount: %d unmarkcount: %d markcount: %d truecount: %d\n",visitcount,unmarkcount,markcount,truecount);
    iface_functions->unmark_visited(position);
}

void UnMarkAllAsVisited()
{
    POSITION i = 0;
    
    for(i = 0; i < gNumberOfPositions; i++)
    {
        iface_functions->unmark_visited(i);
    }

}


void MexStore(POSITION position, MEX theMex)
{
    //not implemented yet.
}

MEX MexLoad(POSITION position)
{
    //not implmented yet.
    return 0;
}
