/************************************************************************
**
** NAME:	memdb.c
**
** DESCRIPTION:	Accessor functions for the in-memory database.
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

#include "gamesman.h"
#include "memdb.h"
#include "db.h"

VALUE* memdb_database;
/*
** Code
*/


DB_Table* memdb_init(){
  int i;
  //Make db_table
  DB_Table *new_db = (DB_Table *) SafeMalloc(sizeof(DB_Table));
  //setup internal memory table
  memdb_database = (VALUE *) SafeMalloc (gNumberOfPositions * sizeof(VALUE));
  
  for(i = 0; i< gNumberOfPositions;i++)
    memdb_database[i] = undecided;
  
  //set function pointers
  new_db->get_value = memdb_get_value;
  new_db->put_value = memdb_set_value;
  new_db->get_remoteness = memdb_get_remoteness;
  new_db->put_remoteness = memdb_set_remoteness;
  new_db->check_visited = memdb_check_visited;
  new_db->mark_visited = memdb_mark_visited;
  new_db->unmark_visited = memdb_unmark_visited;
  
  new_db->free_db = memdb_free;
  
  return new_db;
}

void memdb_free(){
    if(memdb_database)
        SafeFree(memdb_database);

}


VALUE* memdb_get_raw_ptr(POSITION pos){
    return (&memdb_database[pos]);
}

VALUE memdb_set_value(POSITION position, VALUE value)
{
    VALUE *ptr;
    
    ptr = memdb_get_raw_ptr(position);
    
    /* put it in the right position, but we have to blank field and then
    ** add new value to right slot, keeping old slots */
    return((*ptr = ((*ptr & ~VALUE_MASK) | (value & VALUE_MASK))) & VALUE_MASK); 
}

// This is it
VALUE memdb_get_value(POSITION position)
{
    //Gameline code removed
    VALUE *ptr;
    ptr = memdb_get_raw_ptr(position);
    return((VALUE)((int)*ptr & VALUE_MASK)); /* return pure value */
}

REMOTENESS memdb_get_remoteness(POSITION position)
{
    //Gameline code removed
    VALUE *ptr;

    ptr = memdb_get_raw_ptr(position);
    return((((int)*ptr & REMOTENESS_MASK) >> REMOTENESS_SHIFT));
}

void memdb_set_remoteness (POSITION position, REMOTENESS remoteness)
{
    VALUE *ptr;
    
    ptr = memdb_get_raw_ptr(position);
    
    if(remoteness > REMOTENESS_MAX) {
        printf("Remoteness request (%d) for " POSITION_FORMAT  " larger than Max Remoteness (%d)\n",remoteness,position,REMOTENESS_MAX);
        ExitStageRight();
        exit(0);
    }
    
    /* blank field then add new remoteness */
    *ptr = (VALUE)(((int)*ptr & ~REMOTENESS_MASK) | 
		   (remoteness << REMOTENESS_SHIFT));       
}

BOOLEAN memdb_check_visited(POSITION position)
{
    VALUE *ptr;  
    ptr = memdb_get_raw_ptr(position);
    return((((int)*ptr & VISITED_MASK) == VISITED_MASK)); /* Is bit set? */
}

void memdb_mark_visited (POSITION position)
{
    VALUE *ptr;
    
    showStatus(0);
    
    ptr = memdb_get_raw_ptr(position);
    
    *ptr = (VALUE)((int)*ptr | VISITED_MASK);       /* Turn bit on */
}

void memdb_unmark_visited (POSITION position)
{
    VALUE *ptr;
    
    ptr = memdb_get_raw_ptr(position);
    
    *ptr = (VALUE)((int)*ptr & ~VISITED_MASK);      /* Turn bit off */
}

/* Not implemented yet. Will implement once actually using real db classes
void MexStore(POSITION position, MEX theMex)
{
    if (!gTwoBits)
        gDatabase[position] |= ((theMex % 32) * 8) ;
}

MEX MexLoad(POSITION position)
{
#ifdef SYMMETRY_REVISITED
    position = GetCanonicalPosition(position);
#endif
    //Gameline code removed
    return (gTwoBits ? -1 : (gDatabase[position]/8) % 32);
}
*/

