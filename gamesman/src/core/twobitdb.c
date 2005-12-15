/************************************************************************
**
** NAME:	twobitdb.c
**
** DESCRIPTION:	Accessor functions for the twobit in-memory database.
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
#include "db.h"
#include "twobitdb.h"

//THIS IS NOT A 2-BIT DB, but a 4-BIT ONE!!!!!!!!!!!

VALUE* twobitdb_database;
char* twobitdb_visited;
/*
** Code
*/

DB_Table* twobitdb_init() {
  DB_Table *new_db = (DB_Table *) SafeMalloc(sizeof(DB_Table));

  size_t dbSize = sizeof(VALUE) * (1+gNumberOfPositions) / (sizeof(VALUE)*4);
  twobitdb_database = (VALUE*) SafeMalloc(dbSize);
  memset(twobitdb_database, 0xff,dbSize);
    
  //set function pointers
  new_db->get_value = twobitdb_get_value;
  new_db->put_value = twobitdb_set_value;
  new_db->get_remoteness = twobitdb_get_remoteness;
  new_db->put_remoteness = twobitdb_set_remoteness;
  new_db->check_visited = twobitdb_check_visited;
  new_db->mark_visited = twobitdb_mark_visited;
  new_db->unmark_visited = twobitdb_unmark_visited;
  
  new_db->free_db = twobitdb_free;

  return new_db;

}

void twobitdb_free(){
  if(twobitdb_visited)
    SafeFree(twobitdb_visited);
  if(twobitdb_database)
    SafeFree(twobitdb_database);
}

VALUE* twobitdb_get_raw_ptr(POSITION pos){
  return (&twobitdb_database[pos]);
}

VALUE twobitdb_set_value(POSITION position, VALUE value)
{
    int shamt;
    VALUE* ptr;

    ptr = twobitdb_get_raw_ptr(position >> 4);
    shamt = (position & 0xf) * 2;

    *ptr = (*ptr & ~(0x3 << shamt)) | ((0x3 & value) << shamt);
    return value;
}

// This is it
VALUE twobitdb_get_value(POSITION position)
{
   // values are always 32 bits 
  VALUE* ptr;
  ptr = twobitdb_get_raw_ptr(position >> 4);
  return (VALUE)(3 & ((int)*ptr >> ((position & 0xf) * 2)));
}


void twobitdb_set_remoteness(POSITION position, REMOTENESS rem)
{
 
}

REMOTENESS twobitdb_get_remoteness(POSITION position)
{
    return REMOTENESS_TWOBITS;
}

BOOLEAN twobitdb_check_visited(POSITION position)
{
  if (twobitdb_visited){
    return (twobitdb_visited[position >> 3] >> (position & 7)) & 1;
  }
    return FALSE;
}

void twobitdb_mark_visited (POSITION position)
{
  if (!twobitdb_visited){
    twobitdb_visited = (char*) SafeMalloc((gNumberOfPositions>>3) +1);
    memset(twobitdb_visited, 0, (gNumberOfPositions>>3) +1);
  }
  twobitdb_visited[position >> 3] |= 1 << (position & 7);
}

void twobitdb_unmark_visited (POSITION position)
{
  if (twobitdb_visited){
    twobitdb_visited[position >> 3] &= ~(1 << (position & 7));
  }
    return;
}


