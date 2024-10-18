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
#include "twobitdb.h"

void            twobitdb_free ();

/* Value */
VALUE           twobitdb_get_value              (POSITION position);
VALUE           twobitdb_set_value              (POSITION position, VALUE value);

/* Visited */
BOOLEAN         twobitdb_check_visited          (POSITION position);
void            twobitdb_mark_visited           (POSITION position);
void            twobitdb_unmark_visited         (POSITION position);


int* twobitdb_database; //a cell has 8 bits
int* twobitdb_visited;  //a cell has 8 bits

/*
** Code
*/

void twobitdb_init(DB_Table *new_db) {

	//we need two bits per position, so 4 values per byte
	size_t dbSize = (gNumberOfPositions >> 2) + 1;
	//we need one bit per position, so 8 visited values per byte
	size_t visitedSize = (gNumberOfPositions >> 3) + 1;

	twobitdb_database = (int*) SafeMalloc(dbSize);
	memset(twobitdb_database, 0xffff, dbSize);
	twobitdb_visited = (int*) SafeMalloc(visitedSize);
	memset(twobitdb_visited, 0, visitedSize);

	//set function pointers
	new_db->get_value = twobitdb_get_value;
	new_db->put_value = twobitdb_set_value;
	new_db->check_visited = twobitdb_check_visited;
	new_db->mark_visited = twobitdb_mark_visited;
	new_db->unmark_visited = twobitdb_unmark_visited;

	new_db->free_db = twobitdb_free;

	return;

}

void twobitdb_free(){
	if(twobitdb_visited)
		SafeFree(twobitdb_visited);
	if(twobitdb_database)
		SafeFree(twobitdb_database);
}

int* twobitdb_get_raw_ptr(POSITION position){
	return (&twobitdb_database[position>>4]);
}

VALUE twobitdb_set_value(POSITION position, VALUE value)
{
	int shamt;
	int* ptr;

	ptr = twobitdb_get_raw_ptr(position);
	shamt = (position & 15) << 1;

	//printf("set --- pos: %llu, before: %d, value: %d, shamt: %d", position, *ptr, value, shamt);

	*ptr = ((*ptr & ~(3 << shamt)) | ((3 & value) << shamt));

	//printf(" after: %d\n", *ptr);

	return value;
}

// This is it
VALUE twobitdb_get_value(POSITION position)
{
	int* ptr;
	int shamt, value;

	ptr = twobitdb_get_raw_ptr(position);
	shamt = (position & 15) << 1;
	value = (VALUE)(3 & (*ptr >> shamt));
	//printf("get --- pos: %llu, shamt: %d, value: %d\n", position, shamt, value);
	return value;

}

BOOLEAN twobitdb_check_visited(POSITION position)
{
	return ((twobitdb_visited[position >> 5] >> (position & 31)) & 1);
}

void twobitdb_mark_visited (POSITION position)
{
	twobitdb_visited[position >> 5] |= 1 << (position & 31);
	return;
}

void twobitdb_unmark_visited (POSITION position)
{
	twobitdb_visited[position >> 5] &= ~(1 << (position & 31));
	return;
}
