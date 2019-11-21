/************************************************************************
**
** NAME:	networkdb.c
**
** DESCRIPTION:	Accessor functions for the network database.
**
** AUTHOR:	GamesCrafters Networking Group, UC Berkeley
**		Supervised by Dan Garcia <ddgarcia@cs.berkeley.edu>
**
** DATE:	2006-04-20
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

#include <zlib.h>
#include <netinet/in.h>
/*#include "gamesman.h"*/
/*#include "memdb.h"*/

/*internal declarations and definitions*/

#define FILEVER 1

typedef short	cellValue;

BOOLEAN		dirty;
POSITION	CurrentPosition;
cellValue	CurrentValue;

void       	memdb_free 			();
void		memdb_close_file		();

/* Value */
VALUE		memdb_get_value	        	(POSITION pos);
VALUE		memdb_get_value_file		(POSITION pos);
VALUE		memdb_set_value	        	(POSITION pos, VALUE val);

/* Remoteness */
REMOTENESS	memdb_get_remoteness		(POSITION pos);
REMOTENESS	memdb_get_remoteness_file	(POSITION pos);
void		memdb_set_remoteness		(POSITION pos, REMOTENESS val);

/* Visited */
BOOLEAN		memdb_check_visited		(POSITION pos);
BOOLEAN		memdb_check_visited_file	(POSITION pos);
void		memdb_mark_visited		(POSITION pos);
void		memdb_unmark_visited		(POSITION pos);

/* Mex */
MEX		memdb_get_mex			(POSITION pos);
MEX		memdb_get_mex_file		(POSITION pos);
void		memdb_set_mex			(POSITION pos, MEX mex);

/* saving to/reading from a file */
BOOLEAN		memdb_save_database		();
BOOLEAN		memdb_load_database		();

cellValue*	(*memdb_get_raw)		(POSITION pos);

cellValue* 	memdb_get_raw_ptr		(POSITION pos);
cellValue*	memdb_get_raw_file		(POSITION pos);

cellValue*	memdb_array;

char		outfilename[80];
gzFile* 	filep;
short		dbVer[1];
POSITION	numPos[1];
int		goodCompression, goodDecompression, goodClose;

/*
** Code
*/

void memdb_init(DB_Table *new_db)
{
        POSITION i;

        //set function pointers
        memdb_get_raw = memdb_get_raw_file;

        new_db->put_value = NULL;
        new_db->put_remoteness = NULL;
        new_db->mark_visited = NULL;
        new_db->unmark_visited = NULL;
        new_db->put_mex = NULL;
        new_db->free_db = memdb_close_file;

	dirty = TRUE;

        new_db->get_value = memdb_get_value;
        new_db->get_remoteness = memdb_get_remoteness;
        new_db->check_visited = memdb_check_visited;
        new_db->get_mex = memdb_get_mex;
        new_db->save_database = memdb_save_database;
        new_db->load_database = memdb_load_database;


}

void memdb_close_file()
{
	goodClose = true;
}

cellValue* memdb_get_raw_file(POSITION pos)
{
        if(dirty || (pos != CurrentPosition)) {
		dirty = FALSE;
		CurrentPosition = pos;
                z_off_t offset = sizeof(short) + sizeof(POSITION) + sizeof(cellValue)*pos;
                z_off_t zoffset = gztell(filep);
                if(offset >= zoffset)
                        gzseek(filep, offset-zoffset, SEEK_CUR);
                else
                        gzseek(filep, offset, SEEK_SET);
                gzread(filep, &CurrentValue, sizeof(cellValue));
		CurrentValue = ntohs(CurrentValue);
		//printf("reading pos = "POSITION_FORMAT", value = %u\n", pos, CurrentValue);

        }
        return &CurrentValue;
}

VALUE memdb_get_value(POSITION pos)
{
        cellValue *ptr;

        ptr = memdb_get_raw(pos);

        return((VALUE)((int)*ptr & VALUE_MASK)); /* return pure value */
}

REMOTENESS memdb_get_remoteness(POSITION pos)
{
        cellValue *ptr;

        ptr = memdb_get_raw(pos);

        return (REMOTENESS)((((int)*ptr & REMOTENESS_MASK) >> REMOTENESS_SHIFT));
}

BOOLEAN memdb_check_visited(POSITION pos)
{
        cellValue *ptr;

        ptr = memdb_get_raw(pos);

        //printf("check pos: %llu\n", pos);

        return (BOOLEAN)((((int)*ptr & VISITED_MASK) == VISITED_MASK)); /* Is bit set? */
}

MEX memdb_get_mex(POSITION pos)
{
        cellValue *ptr;

        ptr = memdb_get_raw(pos);

        return (MEX)(((int)*ptr & MEX_MASK) >> MEX_SHIFT);
}


/***********
************
**	Database functions.
**
**	Name: saveDatabase()
**
**	Description: writes memdb to a compressed file in gzip format.
**
**	Inputs: none
**
**	Outputs: none
**
**	Calls:	(In libz libraries)
**		gzopen 
**		gzclose
**		gzwrite
**		(In std libraries)
**		htonl
**		ntohl
**
**	Requirements:	memdb_array contains a valid database of positions
**			gNumberOfPositions stores the correct number of positions in memdb_array
**			kDBName is set correctly.
**			getOption() returns the correct option number
**					
*/


BOOLEAN memdb_save_database ()
{
	printf("Someone just tried to make me save the DB. It's not happening, FYI.\n"); 
	return TRUE; 
}

/*
**	Name: loadDatabase()
**
**	Description: loads the compressed file in gzip format into memdb_array.
**
**	Inputs: none
**
**	Outputs: none
**
**	Calls:	(In libz libraries)
**			gzopen 
**			gzclose
**			gzread
**			(In std libraries)
**			ntohl
**
**	Requirements:	memdb_array has enough space malloced to store uncompressed database
**			gNumberOfPositions stores the correct number of uncompressed positions in memdb_array
**			kDBName is set correctly.
**		        getOption() returns the correct option number
**
**		~Scott
************
***********/

BOOLEAN memdb_load_database()
{
	printf("Someone just tried to make me load the DB. It's not happening, FYI.\n"); 
	return TRUE; 
}
