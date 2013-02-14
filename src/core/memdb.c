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

#include <zlib.h>
#include <netinet/in.h>
#include "gamesman.h"
#include "memdb.h"

/*internal declarations and definitions*/

#define FILEVER 1

typedef short cellValue;

BOOLEAN dirty;
POSITION CurrentPosition;
cellValue CurrentValue;

void            memdb_free                      ();
void            memdb_close_file                ();

/* Value */
VALUE           memdb_get_value                 (POSITION pos);
VALUE           memdb_get_value_file            (POSITION pos);
VALUE           memdb_set_value                 (POSITION pos, VALUE val);

/* Remoteness */
REMOTENESS      memdb_get_remoteness            (POSITION pos);
REMOTENESS      memdb_get_remoteness_file       (POSITION pos);
void            memdb_set_remoteness            (POSITION pos, REMOTENESS val);

/* Visited */
BOOLEAN         memdb_check_visited             (POSITION pos);
BOOLEAN         memdb_check_visited_file        (POSITION pos);
void            memdb_mark_visited              (POSITION pos);
void            memdb_unmark_visited            (POSITION pos);

/* Mex */
MEX             memdb_get_mex                   (POSITION pos);
MEX             memdb_get_mex_file              (POSITION pos);
void            memdb_set_mex                   (POSITION pos, MEX mex);

/* saving to/reading from a file */
BOOLEAN         memdb_save_database             ();
BOOLEAN         memdb_load_database             ();

cellValue*      (*memdb_get_raw)(POSITION pos);

cellValue*      memdb_get_raw_ptr               (POSITION pos);
cellValue*      memdb_get_raw_file              (POSITION pos);

cellValue*      memdb_array;

char outfilename[80];
gzFile         filep;
short dbVer[1];
POSITION numPos[1];
int goodCompression, goodDecompression, goodClose;

/*
** Code
*/

void memdb_init(DB_Table *new_db)
{
	POSITION i;
	BOOLEAN useFile = gZeroMemPlayer;

	if(useFile) {
		//try openning the data file
		sprintf(outfilename, "./data/m%s_%d_memdb.dat.gz", kDBName, getOption());
		if((filep = gzopen(outfilename, "rb")) == NULL) {
			useFile = FALSE;
		} else {
			goodDecompression = gzread(filep,dbVer,sizeof(short));
			goodDecompression = gzread(filep,numPos,sizeof(POSITION));
			*dbVer = ntohs(*dbVer);
			*numPos = ntohl(*numPos);
			if((*numPos != gNumberOfPositions) || (*dbVer != FILEVER)) {
				printf("\n\nError in file decompression: Stored gNumberOfPositions differs from internal gNumberOfPositions or incorrect version\n\n");
				useFile = FALSE;
			}
		}
	}
	//set function pointers
	if(useFile) {
		memdb_get_raw = memdb_get_raw_file;

		new_db->put_value = NULL;
		new_db->put_remoteness = NULL;
		new_db->mark_visited = NULL;
		new_db->unmark_visited = NULL;
		new_db->put_mex = NULL;
		new_db->free_db = memdb_close_file;

		dirty = TRUE;
	} else {
		memdb_get_raw = memdb_get_raw_ptr;

		//setup internal memory table
		memdb_array = (cellValue *) SafeMalloc (gNumberOfPositions * sizeof(cellValue));

		for(i = 0; i< gNumberOfPositions; i++)
			memdb_array[i] = undecided;

		new_db->put_value = memdb_set_value;
		new_db->put_remoteness = memdb_set_remoteness;
		new_db->mark_visited = memdb_mark_visited;
		new_db->unmark_visited = memdb_unmark_visited;
		new_db->put_mex = memdb_set_mex;
		new_db->put_winby = (void (*)(POSITION, WINBY))memdb_set_mex;
		new_db->free_db = memdb_free;
	}

	new_db->get_value = memdb_get_value;
	new_db->get_remoteness = memdb_get_remoteness;
	new_db->check_visited = memdb_check_visited;
	new_db->get_mex = memdb_get_mex;
	new_db->get_winby = (WINBY (*)(POSITION))memdb_get_mex;
	new_db->save_database = memdb_save_database;
	new_db->load_database = memdb_load_database;
}

void memdb_free()
{
	if(memdb_array)
		SafeFree(memdb_array);
}

void memdb_close_file()
{
	goodClose = gzclose(filep);
}

cellValue* memdb_get_raw_ptr(POSITION pos)
{
	return (&memdb_array[pos]);
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

VALUE memdb_set_value(POSITION pos, VALUE val)
{
	cellValue *ptr;

	ptr = memdb_get_raw(pos);

	/* put it in the right position, but we have to blank field and then
	** add new value to right slot, keeping old slots */
	return (VALUE)((*ptr = (((int)*ptr & ~VALUE_MASK) | (val & VALUE_MASK))) & VALUE_MASK);
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

void memdb_set_remoteness (POSITION pos, REMOTENESS val)
{
	cellValue *ptr;

	ptr = memdb_get_raw(pos);

	if(val > REMOTENESS_MAX) {
		printf("Remoteness request (%d) for " POSITION_FORMAT  " larger than Max Remoteness (%d)\n",val,pos,REMOTENESS_MAX);
		ExitStageRight();
		exit(0);
	}

	/* blank field then add new remoteness */
	*ptr = (VALUE)(((int)*ptr & ~REMOTENESS_MASK) | (val << REMOTENESS_SHIFT));
}

BOOLEAN memdb_check_visited(POSITION pos)
{
	cellValue *ptr;

	ptr = memdb_get_raw(pos);

	//printf("check pos: %llu\n", pos);

	return (BOOLEAN)((((int)*ptr & VISITED_MASK) == VISITED_MASK)); /* Is bit set? */
}

void memdb_mark_visited (POSITION pos)
{
	cellValue *ptr;

	ptr = memdb_get_raw(pos);

	//printf("mark pos: %llu\n", pos);

	*ptr = (VALUE)((int)*ptr | VISITED_MASK);       /* Turn bit on */
}

void memdb_unmark_visited (POSITION pos)
{
	cellValue *ptr;

	ptr = memdb_get_raw(pos);

	//printf("unmark pos: %llu\n", pos);

	*ptr = (VALUE)((int)*ptr & ~VISITED_MASK);      /* Turn bit off */
}

void memdb_set_mex(POSITION pos, MEX mex)
{
	cellValue *ptr;

	ptr = memdb_get_raw(pos);

	*ptr = (VALUE)(((int)*ptr & ~MEX_MASK) | ((mex & (MEX_MASK >> MEX_SHIFT)) << MEX_SHIFT));

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
	unsigned long i;
	goodCompression = 1;
	goodClose = 0;
	POSITION tot = 0,sTot = gNumberOfPositions;

	if (gTwoBits)   /* TODO: Make db's compatible with 2-bits */
		return FALSE;   /* 0 is error, because it means FALSE. -JJ */
	if(!memdb_array)
		return FALSE;
	if(gZeroMemPlayer)  //we don't save the db if playing on zero memory, but we will say that the db is saved
		return TRUE;

	mkdir("data", 0755);
	sprintf(outfilename, "./data/m%s_%d_memdb.dat.gz", kDBName, getOption());

	if((filep = gzopen(outfilename, "wb")) == NULL) {
		if(kDebugDetermineValue) {
			printf("Unable to create compressed data file\n");
		}
		return FALSE;
	}

	dbVer[0] = htons(FILEVER);
	numPos[0] = htonl(gNumberOfPositions);
	goodCompression = gzwrite(filep, dbVer, sizeof(short));
	goodCompression = gzwrite(filep, numPos, sizeof(POSITION));
	for(i=0; i<gNumberOfPositions && goodCompression; i++) { //convert to network byteorder for platform independence.
		memdb_array[i] = htons(memdb_array[i]);
		goodCompression = gzwrite(filep, memdb_array+i, sizeof(cellValue));
		tot += goodCompression;
		memdb_array[i] = ntohs(memdb_array[i]);
		//gzflush(filep,Z_FULL_FLUSH);
	}
	goodClose = gzclose(filep);

	if(goodCompression && (goodClose == 0)) {
		if(kDebugDetermineValue && !gJustSolving) {
			printf("File Successfully compressed\n");
		}
		return TRUE;
	} else {
		if(kDebugDetermineValue) {
			fprintf(stderr, "\nError in file compression.\n Error codes:\ngzwrite error: %d\ngzclose error:%d\nBytes To Be Written: " POSITION_FORMAT "\nBytes Written: " POSITION_FORMAT "\n",goodCompression, goodClose,sTot*4,tot);
		}
		remove
		        (outfilename);
		return FALSE;
	}

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
	POSITION i;
	goodDecompression = 1;
	goodClose = 1;
	BOOLEAN correctDBVer;

	if (gTwoBits)   /* TODO: Same here */
		return FALSE;
	if(!memdb_array && !gZeroMemPlayer)
		return FALSE;
	else if(gZeroMemPlayer)  // we don't load the db to memory, but still report that we have loaded the db
		return TRUE;

	sprintf(outfilename, "./data/m%s_%d_memdb.dat.gz", kDBName, getOption());
	if((filep = gzopen(outfilename, "rb")) == NULL)
		return FALSE;

	goodDecompression = gzread(filep,dbVer,sizeof(short));
	goodDecompression = gzread(filep,numPos,sizeof(POSITION));
	*dbVer = ntohs(*dbVer);
	*numPos = ntohl(*numPos);
	if(*numPos != gNumberOfPositions && kDebugDetermineValue) {
		printf("\n\nError in file decompression: Stored gNumberOfPositions differs from internal gNumberOfPositions\n\n");
		return FALSE;
	}
	/***
	** Database Ver. 1 Decompress
	***/
	correctDBVer = (*dbVer == FILEVER);

	if (correctDBVer) {
		showDBLoadingStatus (Clean);
		for(i = 0; i < gNumberOfPositions && goodDecompression; i++) {
			goodDecompression = gzread(filep, memdb_array+i, sizeof(cellValue));
			memdb_array[i] = ntohs(memdb_array[i]);
			showDBLoadingStatus (Update);
		}
	}
	/***
	** End Ver. 1
	***/


	goodClose = gzclose(filep);


	if(goodDecompression && (goodClose == 0) && correctDBVer) {
		if(kDebugDetermineValue) {
			printf("File Successfully Decompressed\n");
		}
		return TRUE;
	} else {
		for(i = 0; i < gNumberOfPositions; i++)
			memdb_array[i] = undecided;
		if(kDebugDetermineValue) {
			printf("\n\nError in file decompression:\ngzread error: %d\ngzclose error: %d\ndb version: %d\n",goodDecompression,goodClose,*dbVer);
		}
		return FALSE;
	}

}
