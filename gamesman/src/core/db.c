/************************************************************************
**
** NAME:	db.c
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
** Needs to be built up to implement The new DB Class abstraction as is
** Found in the Expeimental directory. However we first need to make
** The existing functions abstract.
*/


#include "gamesman.h"
#include "db.h"
#include "memdb.h"
#include "twobitdb.h"
#include "colldb.h"
#include "globals.h"

DB_Table *db_functions;

/*
** Code
*/

void db_initialize(){

    if(db_functions){
        db_functions->free_db();
    }
    if (gTwoBits) {
        db_functions = twobitdb_init();
    } else if(gCollDB){
      db_functions = colldb_init();
    } else {
        db_functions = memdb_init();
    }
}

void db_free(){
    if(db_functions) {
	db_functions->free_db();
	SafeFree(db_functions);
    }
}

VALUE StoreValueOfPosition(POSITION position, VALUE value)
{
    return db_functions->put_value(position,value);
}


// This is it
VALUE GetValueOfPosition(POSITION position)
{
  if(gMenuMode == Evaluated && gSymmetries)
    position = gCanonicalPosition(position);

  return db_functions->get_value(position);
}


REMOTENESS Remoteness(POSITION position)
{ 
  if(gMenuMode == Evaluated && gSymmetries)
	position = gCanonicalPosition(position);
  
  return db_functions->get_remoteness(position);
}
    

void SetRemoteness (POSITION position, REMOTENESS remoteness)
{
     if(db_functions->put_remoteness != NULL)
        db_functions->put_remoteness(position,remoteness);
}
 

BOOLEAN Visited(POSITION position)
{
    return db_functions->check_visited(position);
}


void MarkAsVisited (POSITION position)
{
    return db_functions->mark_visited(position);
}

void UnMarkAsVisited (POSITION position)
{
    db_functions->unmark_visited(position);
}

void UnMarkAllAsVisited()
{
    POSITION i = 0;
    
    for(i = 0; i < gNumberOfPositions; i++)
    {
        db_functions->unmark_visited(i);
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
