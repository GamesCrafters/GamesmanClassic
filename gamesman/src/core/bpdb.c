/************************************************************************
**
** NAME:	bpdb.c
**
** DESCRIPTION:	Accessor functions for the bit-perfect database.
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
#include <sys/stat.h>
#include "gamesman.h"
#include "bpdb.h"
#include "bpdb_schemes.h"

/*internal declarations and definitions*/

/* List structure for schemes */
typedef struct Schemelist {
	// numeric identifier
	int			scheme;

	// pointers to load and save functions
    BOOLEAN		(*save_database)	( int schemeversion, char *outfilename);
    BOOLEAN		(*load_database)	();

	// next scheme
	struct Schemelist *next;
} *Scheme_List;

/* create new scheme list */
Scheme_List scheme_list_new() {
	return NULL;
}

/* add scheme to list */
Scheme_List scheme_list_add(Scheme_List sl, int schemenum, BOOLEAN (*save_database)( int schemeversion, char *outfilename), BOOLEAN (*load_database)()) {
	Scheme_List cur = sl;
	Scheme_List temp;

	if(sl == NULL) {
		// if no schemes, add first scheme
		temp = (Scheme_List) SafeMalloc(sizeof(struct Schemelist));
		temp->next = NULL;
		temp->scheme = schemenum;
		temp->save_database = save_database;
		temp->load_database = load_database;
		return temp;
	} else {
		// find end of list
		while(cur->next != NULL) {
			cur = cur->next;
		}

		// add scheme
		cur->next = (Scheme_List) SafeMalloc(sizeof(struct Schemelist));
		cur->next->next = NULL;
		cur->next->scheme = schemenum;
		cur->next->save_database = save_database;
		cur->next->load_database = load_database;
		return sl;
	}
	
	// return pointer to start of list
	return sl;
}

// pointer to array of slices
cellValue*	bpdb_array;

// pointer to scheme list
Scheme_List	bpdb_list;

// size of scheme list
int bpdb_list_size;

void       	bpdb_free			();

/* Value */
VALUE		bpdb_get_value		(POSITION pos);
VALUE		bpdb_set_value		(POSITION pos, VALUE val);

/* Remoteness */
REMOTENESS	bpdb_get_remoteness	(POSITION pos);
void		bpdb_set_remoteness	(POSITION pos, REMOTENESS val);

/* Visited */
BOOLEAN		bpdb_check_visited	(POSITION pos);
void		bpdb_mark_visited	(POSITION pos);
void		bpdb_unmark_visited	(POSITION pos);

/* Mex */
MEX			bpdb_get_mex		(POSITION pos);
void		bpdb_set_mex		(POSITION pos, MEX mex);

/* saving to/reading from a file */
BOOLEAN		bpdb_save_database	();
BOOLEAN		bpdb_load_database	();

/*
** Code
*/


void bpdb_init(DB_Table *new_db)
{
    POSITION i;

    // setup internal memory table
    bpdb_array = (cellValue *) SafeMalloc (gNumberOfPositions * sizeof(cellValue));
  
    for(i = 0; i< gNumberOfPositions; i++)
	bpdb_array[i] = undecided;
    
    // set function pointers
    new_db->get_value = bpdb_get_value;
    new_db->put_value = bpdb_set_value;
    new_db->get_remoteness = bpdb_get_remoteness;
    new_db->put_remoteness = bpdb_set_remoteness;
    new_db->check_visited = bpdb_check_visited;
    new_db->mark_visited = bpdb_mark_visited;
    new_db->unmark_visited = bpdb_unmark_visited;
    new_db->get_mex = bpdb_get_mex;
    new_db->put_mex = bpdb_set_mex;
    new_db->save_database = bpdb_save_database;
    new_db->load_database = bpdb_load_database;
    
    new_db->free_db = bpdb_free;

	// setup schemes list
	bpdb_list = scheme_list_new();

	/*
	 * ADD SCHEMES HERE
	 * scheme_list_add(bpdb_list, number identifier in header, save function, load function); bpdb_list_size++;
	 */
	bpdb_list = scheme_list_add(bpdb_list, 0, bpdb_save_database_mem, bpdb_load_database_mem); bpdb_list_size++;
	bpdb_list = scheme_list_add(bpdb_list, 1, bpdb_save_database_dan, bpdb_load_database_dan); bpdb_list_size++;
	bpdb_list = scheme_list_add(bpdb_list, 2, bpdb_save_database_ken, bpdb_load_database_ken); bpdb_list_size++;
	//bpdb_list = scheme_list_add(bpdb_list, 3, bpdb_save_database_scott, bpdb_load_database_scott); bpdb_list_size++;
}


void bpdb_free()
{
    if(bpdb_array)
        SafeFree(bpdb_array);
}


cellValue* bpdb_get_raw_ptr(POSITION pos)
{
    return (&bpdb_array[pos]);
}

VALUE bpdb_set_value(POSITION pos, VALUE val)
{
    cellValue *ptr;
    
    ptr = bpdb_get_raw_ptr(pos);
    
    /* put it in the right position, but we have to blank field and then
    ** add new value to right slot, keeping old slots */
    return (VALUE)((*ptr = (((int)*ptr & ~VALUE_MASK) | (val & VALUE_MASK))) & VALUE_MASK); 
}

VALUE bpdb_get_value(POSITION pos)
{
    cellValue *ptr;

    ptr = bpdb_get_raw_ptr(pos);

    return((VALUE)((int)*ptr & VALUE_MASK)); /* return pure value */
}

REMOTENESS bpdb_get_remoteness(POSITION pos)
{
    cellValue *ptr;

    ptr = bpdb_get_raw_ptr(pos);

    return (REMOTENESS)((((int)*ptr & REMOTENESS_MASK) >> REMOTENESS_SHIFT));
}

void bpdb_set_remoteness (POSITION pos, REMOTENESS val)
{
    cellValue *ptr;
    
    ptr = bpdb_get_raw_ptr(pos);
    
    if(val > REMOTENESS_MAX) {
        printf("Remoteness request (%d) for " POSITION_FORMAT  " larger than Max Remoteness (%d)\n",val,pos,REMOTENESS_MAX);
        ExitStageRight();
        exit(0);
    }
    
    /* blank field then add new remoteness */
    *ptr = (VALUE)(((int)*ptr & ~REMOTENESS_MASK) | (val << REMOTENESS_SHIFT));
}

BOOLEAN bpdb_check_visited(POSITION pos)
{
    cellValue *ptr;  

    ptr = bpdb_get_raw_ptr(pos);

    //printf("check pos: %llu\n", pos);

    return (BOOLEAN)((((int)*ptr & VISITED_MASK) == VISITED_MASK)); /* Is bit set? */
}

void bpdb_mark_visited (POSITION pos)
{
    cellValue *ptr;
        
    ptr = bpdb_get_raw_ptr(pos);
    
    //printf("mark pos: %llu\n", pos);

    *ptr = (VALUE)((int)*ptr | VISITED_MASK);       /* Turn bit on */
}

void bpdb_unmark_visited (POSITION pos)
{
    cellValue *ptr;
    
    ptr = bpdb_get_raw_ptr(pos);

    //printf("unmark pos: %llu\n", pos);
    
    *ptr = (VALUE)((int)*ptr & ~VISITED_MASK);      /* Turn bit off */
}

void bpdb_set_mex(POSITION pos, MEX mex)
{
    cellValue *ptr;

    ptr = bpdb_get_raw_ptr(pos);

    *ptr = (VALUE)(((int)*ptr & ~MEX_MASK) | (mex << MEX_SHIFT));
}

MEX bpdb_get_mex(POSITION pos)
{
    cellValue *ptr;

    ptr = bpdb_get_raw_ptr(pos);

    return (MEX)(((int)*ptr & MEX_MASK) >> MEX_SHIFT);
}


/***********
************
**	Database functions. THIS NEEDS TO BE UPDATED
**
**	Name: saveDatabase()
**
**	Description: writes bpdb to a compressed file in gzip format.
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
**	Requirements:	bpdb_array contains a valid database of positions
**			gNumberOfPositions stores the correct number of positions in bpdb_array
**			kDBName is set correctly.
**			getOption() returns the correct option number
**					
*/

BOOLEAN bpdb_save_database()
{
	// counter
	int i;
	Scheme_List cur;
	BOOLEAN success = FALSE;

	// file names of saved files
	char outfilenames[bpdb_list_size][256];

	// final file name
	char outfilename[256];

	// track smallest file
	int smallestscheme = 0;
	int smallestsize = -1;

	// struct for fileinfo
	struct stat fileinfo;

	if (gTwoBits)		/* TODO: Make db's compatible with 2-bits */
		return FALSE;	/* 0 is error, because it means FALSE. -JJ */
	if(!bpdb_array)
		return FALSE;

	printf("\n");

	// set counters
	i = 0;
	cur = bpdb_list;

	// save file under each encoding scheme
	while(cur != NULL) {
		// saves with encoding scheme and returns filename
		success = cur->save_database(cur->scheme, outfilenames[i]);

		if(success) {
			// get size of file
			stat(outfilenames[i], &fileinfo);

			printf("Scheme: %d. Wrote %s with size of %d\n", cur->scheme, outfilenames[i], (int)fileinfo.st_size);

			// if file is a smaller size, set min
			if(smallestsize == -1 || fileinfo.st_size < smallestsize) {
				smallestscheme = i;
				smallestsize  = fileinfo.st_size;
			}
		}
		cur = cur->next;
		i++;
	}

	printf("Choosing scheme: %d\n", smallestscheme);
	
	// for each file, delete if not the smallest encoding,
	// and if it is, rename it to the final file name.
	for(i = 0; i < bpdb_list_size; i++) {
		if(i == smallestscheme) {

			// rename smallest file to final file name
			sprintf(outfilename, "./data/m%s_%d_bpdb.dat.gz", kDBName, getOption());
			printf("Renaming %s to %s\n", outfilenames[i], outfilename);
			rename(outfilenames[i], outfilename);
		} else {

			// delete files that are not the smallest
			printf("Removing %s\n", outfilenames[i]);
			remove(outfilenames[i]);
		}
	}

	printf("\n");

	return TRUE;
}


/* Writes byte to buffer */
int bpdb_write_byte(gzFile *outFile, BYTE r) {
	int ret = 0;
	ret = gzwrite(outFile, &r, 1);
	if(ret != 1) {
		printf("Most likely bad compression! %d\n", ret);
		//exit(0);
	}
	return ret;
}

/*
**	Name: loadDatabase() THIS NEEDS TO BE UPDATED
**
**	Description: loads the compressed file in gzip format into bpdb_array.
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
**	Requirements:	bpdb_array has enough space malloced to store uncompressed database
**			gNumberOfPositions stores the correct number of uncompressed positions in bpdb_array
**			kDBName is set correctly.
**		        getOption() returns the correct option number
**
**		~Scott
************
***********/



BOOLEAN bpdb_load_database()
{
	Scheme_List cur;

	// filename
	char outfilename[256];

    //BOOLEAN correctDBVer = FALSE;
	BOOLEAN success = FALSE;
	int goodClose = 1;
  
	// gzip file ptr
	gzFile inFile;

	// file information
	int fileformat = 0;

	// open file
	sprintf(outfilename, "./data/m%s_%d_bpdb.dat.gz", kDBName, getOption());
    if((inFile = gzopen(outfilename, "rb")) == NULL) return FALSE;
	
	// read file header
	fileformat = bpdb_read_byte(inFile);
	
	// stop if using Two-Bit DB
    if (gTwoBits)
        return FALSE;

	// stop if bpdb_array NULL
    if(!bpdb_array)
		return FALSE;

	// print fileinfo
	printf("\nEncoding Scheme: %d\n", fileformat);
	
	cur = bpdb_list;

	while(cur->scheme != fileformat) {
		cur = cur->next;
	}
	
	success = cur->load_database( inFile );

	// close file
	goodClose = gzclose(inFile);

	if(goodClose == 0) {
		return success;
	} else {
		return FALSE;
	}
}



BYTE bpdb_read_byte( gzFile *inFile ) {
	BYTE readbyte = 0;
	int ret;

	ret = gzread(inFile, &readbyte,sizeof(BYTE));

	if(ret <= 0) {
		printf("Warning: Most likely bad decompression.");
		//exit(0);
	}
	return readbyte;
}
