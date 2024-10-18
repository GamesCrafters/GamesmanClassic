/************************************************************************
**
** NAME:	filedb.c
**
** DESCRIPTION:	Functions to load and store databases from/to
**		compressed files.
**
** AUTHOR:	Scott Lindeneau, Evan Huang
**		GamesCrafters Research Group, UC Berkeley
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

#include "filedb.h"
#include "filedb/db.h"
#include "gamesman.h"

typedef short cellValue;

BOOLEAN start;
POSITION mypos;
cellValue myvalue;

void            filedb_free                     ();

/* Value */
VALUE           filedb_get_value                        (POSITION pos);
VALUE           filedb_set_value                        (POSITION pos, VALUE val);

/* Remoteness */
REMOTENESS      filedb_get_remoteness           (POSITION pos);
void            filedb_set_remoteness           (POSITION pos, REMOTENESS val);

/* Visited */
BOOLEAN         filedb_check_visited            (POSITION pos);
void            filedb_mark_visited             (POSITION pos);
void            filedb_unmark_visited           (POSITION pos);

/* Mex */
MEX                     filedb_get_mex                  (POSITION pos);
void            filedb_set_mex                  (POSITION pos, MEX mex);

cellValue*      filedb_get_raw                  (POSITION pos);

/* saving to/reading from a file */
BOOLEAN     filedb_save_database    ();
BOOLEAN     filedb_load_database    ();

gamesdb*   mydb;

/*
 * code
 */

void filedb_init(DB_Table *new_db)
{
	char dirname[80];

	//set function pointers
	new_db->get_value = filedb_get_value;
	new_db->put_value = filedb_set_value;
	new_db->check_visited = filedb_check_visited;
	new_db->get_remoteness = filedb_get_remoteness;
	new_db->put_remoteness = filedb_set_remoteness;
	new_db->mark_visited = filedb_mark_visited;
	new_db->unmark_visited = filedb_unmark_visited;
	new_db->get_mex = filedb_get_mex;
	new_db->put_mex = filedb_set_mex;
	new_db->save_database = filedb_save_database;
	new_db->load_database = filedb_load_database;
	new_db->free_db = filedb_free;

	sprintf(dirname, "m%s_%d_filedb", kDBName, getOption());

	gamesdb_pageid max_pages = 0;

	if (gZeroMemPlayer) max_pages = 1;

	mydb = gamesdb_create(sizeof(cellValue), 4196, max_pages, 10, dirname);

	start = FALSE;
	mypos = 0;
	myvalue = 0;
}

void filedb_free()
{
	gamesdb_put(mydb, (char *)&myvalue, mypos);
	gamesdb_destroy(mydb);
}

cellValue* filedb_get_raw(POSITION pos)
{
	if(start == TRUE) {
		if(mypos != pos) {
			gamesdb_put(mydb, (char *)&myvalue, mypos);
			gamesdb_get(mydb, (char *)&myvalue, pos);
			mypos = pos;
		}
	} else {
		start = TRUE;
		gamesdb_get(mydb, (char *)&myvalue, pos);
		mypos = pos;
	}
	return &myvalue;
}

VALUE filedb_set_value(POSITION pos, VALUE val)
{
	cellValue *ptr;

	ptr = filedb_get_raw(pos);

	/* put it in the right position, but we have to blank field and then
	** add new value to right slot, keeping old slots */
	return (VALUE)((*ptr = (((int)*ptr & ~VALUE_MASK) | (val & VALUE_MASK))) & VALUE_MASK);
}

VALUE filedb_get_value(POSITION pos)
{
	cellValue *ptr;

	ptr = filedb_get_raw(pos);

	return((VALUE)((int)*ptr & VALUE_MASK)); /* return pure value */
}

REMOTENESS filedb_get_remoteness(POSITION pos)
{
	cellValue *ptr;

	ptr = filedb_get_raw(pos);

	return (REMOTENESS)((((int)*ptr & REMOTENESS_MASK) >> REMOTENESS_SHIFT));
}

void filedb_set_remoteness (POSITION pos, REMOTENESS val)
{
	cellValue *ptr;

	ptr = filedb_get_raw(pos);

	if(val > REMOTENESS_MAX) {
		printf("Remoteness request (%d) for " POSITION_FORMAT  " larger than Max Remoteness (%d)\n",val,pos,REMOTENESS_MAX);
		ExitStageRight();
		exit(0);
	}

	/* blank field then add new remoteness */
	*ptr = (VALUE)(((int)*ptr & ~REMOTENESS_MASK) | (val << REMOTENESS_SHIFT));
}

BOOLEAN filedb_check_visited(POSITION pos)
{
	cellValue *ptr;

	ptr = filedb_get_raw(pos);

	//printf("check pos: %llu\n", pos);

	return (BOOLEAN)((((int)*ptr & VISITED_MASK) == VISITED_MASK)); /* Is bit set? */
}

void filedb_mark_visited (POSITION pos)
{
	cellValue *ptr;

	ptr = filedb_get_raw(pos);

	//printf("mark pos: %llu\n", pos);

	*ptr = (VALUE)((int)*ptr | VISITED_MASK);       /* Turn bit on */
}

void filedb_unmark_visited (POSITION pos)
{
	cellValue *ptr;

	ptr = filedb_get_raw(pos);

	//printf("unmark pos: %llu\n", pos);

	*ptr = (VALUE)((int)*ptr & ~VISITED_MASK);      /* Turn bit off */
}

void filedb_set_mex(POSITION pos, MEX mex)
{
	cellValue *ptr;

	ptr = filedb_get_raw(pos);

	*ptr = (VALUE)(((int)*ptr & ~MEX_MASK) | (mex << MEX_SHIFT));
}

MEX filedb_get_mex(POSITION pos)
{
	cellValue *ptr;

	ptr = filedb_get_raw(pos);

	return (MEX)(((int)*ptr & MEX_MASK) >> MEX_SHIFT);
}

BOOLEAN filedb_save_database()
{
	gamesdb_buf_flush_all(mydb);
	return TRUE;
}

BOOLEAN filedb_load_database()
{
	return TRUE;
}
