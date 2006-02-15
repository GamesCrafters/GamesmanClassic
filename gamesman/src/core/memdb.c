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

void       	memdb_free 		();

/* Value */
VALUE		memdb_get_value	        (POSITION pos);
VALUE		memdb_set_value	        (POSITION pos, VALUE val);

/* Remoteness */
REMOTENESS	memdb_get_remoteness	(POSITION pos);
void		memdb_set_remoteness	(POSITION pos, REMOTENESS val);

/* Visited */
BOOLEAN		memdb_check_visited    	(POSITION pos);
void		memdb_mark_visited     	(POSITION pos);
void		memdb_unmark_visited	(POSITION pos);

/* Mex */
MEX		memdb_get_mex		(POSITION pos);
void		memdb_set_mex		(POSITION pos, MEX mex);

/* saving to/reading from a file */
BOOLEAN		memdb_save_database	();
BOOLEAN		memdb_load_database	();

cellValue* memdb_array;

/*
** Code
*/

void memdb_init(DB_Table *new_db)
{
        POSITION i;

        //setup internal memory table
        memdb_array = (cellValue *) SafeMalloc (gNumberOfPositions * sizeof(cellValue));

        for(i = 0; i< gNumberOfPositions; i++)
                memdb_array[i] = undecided;

        //set function pointers
        new_db->get_value = memdb_get_value;
        new_db->put_value = memdb_set_value;
        new_db->get_remoteness = memdb_get_remoteness;
        new_db->put_remoteness = memdb_set_remoteness;
        new_db->check_visited = memdb_check_visited;
        new_db->mark_visited = memdb_mark_visited;
        new_db->unmark_visited = memdb_unmark_visited;
        new_db->get_mex = memdb_get_mex;
        new_db->put_mex = memdb_set_mex;
        new_db->save_database = memdb_save_database;
        new_db->load_database = memdb_load_database;

        new_db->free_db = memdb_free;
}


void memdb_free()
{
        if(memdb_array)
                SafeFree(memdb_array);
}


cellValue* memdb_get_raw_ptr(POSITION pos)
{
        return (&memdb_array[pos]);
}

VALUE memdb_set_value(POSITION pos, VALUE val)
{
        cellValue *ptr;

        ptr = memdb_get_raw_ptr(pos);

        /* put it in the right position, but we have to blank field and then
        ** add new value to right slot, keeping old slots */
        return (VALUE)((*ptr = (((int)*ptr & ~VALUE_MASK) | (val & VALUE_MASK))) & VALUE_MASK);
}

VALUE memdb_get_value(POSITION pos)
{
        cellValue *ptr;

        ptr = memdb_get_raw_ptr(pos);

        return((VALUE)((int)*ptr & VALUE_MASK)); /* return pure value */
}

REMOTENESS memdb_get_remoteness(POSITION pos)
{
        cellValue *ptr;

        ptr = memdb_get_raw_ptr(pos);

        return (REMOTENESS)((((int)*ptr & REMOTENESS_MASK) >> REMOTENESS_SHIFT));
}

void memdb_set_remoteness (POSITION pos, REMOTENESS val)
{
        cellValue *ptr;

        ptr = memdb_get_raw_ptr(pos);

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

        ptr = memdb_get_raw_ptr(pos);

        //printf("check pos: %llu\n", pos);

        return (BOOLEAN)((((int)*ptr & VISITED_MASK) == VISITED_MASK)); /* Is bit set? */
}

void memdb_mark_visited (POSITION pos)
{
        cellValue *ptr;

        ptr = memdb_get_raw_ptr(pos);

        //printf("mark pos: %llu\n", pos);

        *ptr = (VALUE)((int)*ptr | VISITED_MASK);       /* Turn bit on */
}

void memdb_unmark_visited (POSITION pos)
{
        cellValue *ptr;

        ptr = memdb_get_raw_ptr(pos);

        //printf("unmark pos: %llu\n", pos);

        *ptr = (VALUE)((int)*ptr & ~VISITED_MASK);      /* Turn bit off */
}

void memdb_set_mex(POSITION pos, MEX mex)
{
        cellValue *ptr;

        ptr = memdb_get_raw_ptr(pos);

        *ptr = (VALUE)(((int)*ptr & ~MEX_MASK) | (mex << MEX_SHIFT));
}

MEX memdb_get_mex(POSITION pos)
{
        cellValue *ptr;

        ptr = memdb_get_raw_ptr(pos);

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
        short dbVer[1];
        POSITION numPos[1];
        unsigned long i;
        gzFile * filep;
        char outfilename[256] ;
        int goodCompression = 1;
        int goodClose = 0;
        POSITION tot = 0,sTot = gNumberOfPositions;

        if (gTwoBits)	/* TODO: Make db's compatible with 2-bits */
                return FALSE;	/* for some reason, 0 is error, because it means FALSE. -JJ */
        if(!memdb_array)
                return FALSE;

        mkdir("data", 0755) ;
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
                if(kDebugDetermineValue && ! gJustSolving) {
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
        short dbVer[1];
        POSITION numPos[1];
        POSITION i;
        gzFile * filep ;
        char outfilename[256] ;
        int goodDecompression = 1;
        int goodClose = 1;
        BOOLEAN correctDBVer;

        if (gTwoBits)	/* TODO: Same here */
                return FALSE;
        if(!memdb_array)
                return FALSE;

        sprintf(outfilename, "./data/m%s_%d_memdb.dat.gz", kDBName, getOption()) ;
        if((filep = gzopen(outfilename, "rb")) == NULL)
                return 0 ;

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
                for(i = 0; i < gNumberOfPositions && goodDecompression; i++) {
                        goodDecompression = gzread(filep, memdb_array+i, sizeof(cellValue));
                        memdb_array[i] = ntohs(memdb_array[i]);
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
                for(i = 0 ; i < gNumberOfPositions ; i++)
                        memdb_array[i] = undecided ;
                if(kDebugDetermineValue) {
                        printf("\n\nError in file decompression:\ngzread error: %d\ngzclose error: %d\ndb version: %d\n",goodDecompression,goodClose,*dbVer);
                }
                return FALSE;
        }

}
