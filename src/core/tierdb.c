/************************************************************************
**
** NAME:	tierdb.c
**
** DESCRIPTION:	Accessor functions for the tierified in-memory database.
**
** AUTHOR:	GamesCrafters Research Group, UC Berkeley
**		Supervised by Dan Garcia <ddgarcia@cs.berkeley.edu>
**
** DATE:	2005-10-15
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
   DISCLAIMER: This is basically a copy+paste of memdb.c, with saving and loading
   changed so that it handles many files (databases) instead of just one.
   Hence, even though the files are called "tierdb", they are EXACTLY like
   memdbs.
   -Max
 */

#include <zlib.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include "gamesman.h"
#include <dirent.h>
#include "tierdb.h"

/*internal declarations and definitions*/

#define tierdb_FILEVER 1
#define FILESIZE 1048576L

POSITION offsets[50000];
unsigned long numOffsets = 0;
TIER tierForWhichOffsetsLoaded = -1;
BOOLEAN alreadyReinitialized = FALSE;

typedef short tierdb_cellValue;

BOOLEAN tierdb_dirty;
POSITION tierdb_CurrentPosition;
tierdb_cellValue tierdb_CurrentValue;

void            tierdb_free                     ();
void            tierdb_close_file               ();

/* Value */
VALUE           tierdb_get_value                (POSITION pos);
VALUE           tierdb_get_value_from_lookup_table           (POSITION pos);
VALUE           tierdb_set_value                (POSITION pos, VALUE val);

/* Remoteness */
REMOTENESS      tierdb_get_remoteness           (POSITION pos);
REMOTENESS      tierdb_get_remoteness_from_lookup_table (POSITION pos);
void            tierdb_set_remoteness           (POSITION pos, REMOTENESS val);

/* Visited */
BOOLEAN         tierdb_check_visited            (POSITION pos);
BOOLEAN         tierdb_check_visited_from_lookup_table       (POSITION pos);
void            tierdb_mark_visited             (POSITION pos);
void            tierdb_unmark_visited           (POSITION pos);

/* Mex */
MEX             tierdb_get_mex                  (POSITION pos);
MEX             tierdb_get_mex_from_lookup_table             (POSITION pos);
void            tierdb_set_mex                  (POSITION pos, MEX mex);

/* saving to/reading from a file */
BOOLEAN         tierdb_save_database            ();
BOOLEAN         tierdb_load_database            ();

tierdb_cellValue*       (*tierdb_get_raw)(POSITION pos);
tierdb_cellValue*       tierdb_get_raw_ptr      (POSITION pos);
tierdb_cellValue        tierdb_get_raw_from_lookup_table (POSITION pos);
void                    load_offsets (TIER tier);

tierdb_cellValue*       tierdb_array;

#define TIERDB_OUTFILENAME_PARTIAL_LENGTH_MAX 80
#define TIERDB_OUTFILENAME_LENGTH_MAX 160
char tierdb_outfilename[TIERDB_OUTFILENAME_LENGTH_MAX];
char tierdb_lookupfilename[80];
gzFile         tierdb_filep;
short tierdb_dbVer[1];
POSITION tierdb_numPos[1];
int tierdb_goodCompression, tierdb_goodDecompression, tierdb_goodClose;

/*
** Code
*/

void tierdb_init(DB_Table *new_db)
{
	if (alreadyReinitialized) {
		return;
	}

	POSITION i;
	tierdb_get_raw = tierdb_get_raw_ptr;

	//setup internal memory table
	if (gLoadTierdbArray) {
		tierdb_array = (tierdb_cellValue *) SafeMalloc (gNumberOfPositions * sizeof(tierdb_cellValue));

		for(i = 0; i< gNumberOfPositions; i++)
			tierdb_array[i] = undecided;
	}

	new_db->put_value = tierdb_set_value;
	new_db->put_remoteness = tierdb_set_remoteness;
	new_db->mark_visited = tierdb_mark_visited;
	new_db->unmark_visited = tierdb_unmark_visited;
	new_db->put_mex = tierdb_set_mex;
	new_db->free_db = tierdb_free;

	new_db->get_value = tierdb_get_value;
	new_db->get_remoteness = tierdb_get_remoteness;
	new_db->check_visited = tierdb_check_visited;
	new_db->get_mex = tierdb_get_mex;
	new_db->save_database = tierdb_save_database;
	new_db->load_database = tierdb_load_database;
}

/*
Use getter functions that access the file directly if there
exists a lookup table. If there is no lookup table, doesn't
do anything.
*/
BOOLEAN tierdb_reinit(DB_Table *new_db)
{
	if (alreadyReinitialized) {
		return TRUE;
	}
	sprintf(tierdb_outfilename, "./data/m%s_%d_tierdb/lookup", kDBName, getOption());
	DIR* dir = opendir(tierdb_outfilename);
	if (dir) {
    	/* Directory exists. */
    	closedir(dir);
		new_db->get_value = tierdb_get_value_from_lookup_table;
		new_db->get_remoteness = tierdb_get_remoteness_from_lookup_table;
		new_db->check_visited = tierdb_check_visited_from_lookup_table;
		new_db->get_mex = tierdb_get_mex_from_lookup_table;
		alreadyReinitialized = TRUE;
		return TRUE;
	} else {
		return FALSE;
	}
}

void tierdb_free_childpositions()
{
	if (tierdb_array)
		tierdb_array = (tierdb_cellValue *) SafeRealloc(tierdb_array, gCurrentTierSize * sizeof(tierdb_cellValue));
}

void tierdb_free()
{
	if(tierdb_array)
		SafeFree(tierdb_array);
}

void tierdb_close_file()
{
	tierdb_goodClose = gzclose(tierdb_filep);
}

tierdb_cellValue* tierdb_get_raw_ptr(POSITION pos)
{
	return (&tierdb_array[pos]);
}

void load_offsets(TIER tier) {
	if (tierForWhichOffsetsLoaded == tier) {
		return;
	}
    FILE *fp;
	sprintf(tierdb_outfilename, "./data/m%s_%d_tierdb/lookup/m%s_%d_%llu_tierdb.dat.gz.idx",
		        kDBName, getOption(), kDBName, getOption(), tier);
    fp = fopen(tierdb_outfilename, "r");
    if (fp == NULL) {
        printf("Can't open %s\n", tierdb_outfilename);
    	exit(1);
    }

	offsets[0] = 0;
    numOffsets = 1; // will be # of lines in the idx file
    POSITION offset = 0, b;
    while (!feof(fp)) {
        int n = fscanf(fp, "%llu", &b);
        if (n == 1) {
            offset += b;
            offsets[numOffsets++] = offset;
        }
    }
    fclose(fp);
	tierForWhichOffsetsLoaded = tier;
}

tierdb_cellValue tierdb_get_raw_from_lookup_table(POSITION pos)
{
	TIER tier;
	TIERPOSITION tierposition;
	gUnhashToTierPosition(pos, &tierposition, &tier);
	load_offsets(tier);

	sprintf(tierdb_outfilename, "./data/m%s_%d_tierdb/m%s_%d_%llu_tierdb.dat.gz",
		        kDBName, getOption(), kDBName, getOption(), tier);

	unsigned long long chunk = (tierposition * 2 + sizeof(short) + sizeof(POSITION)) / FILESIZE;
    unsigned long seekTo = (tierposition * 2 + sizeof(short) + sizeof(POSITION)) % FILESIZE;

    // Open file using open
    int fd = open(tierdb_outfilename, O_RDONLY);
    if (fd < 0) {
        printf("Unable to open %s\n", tierdb_outfilename);
        exit(1);
    }

    // Seek to the correct chunk
    lseek(fd, offsets[chunk], SEEK_SET);

    // open using gzdopen
	gzFile gzf = gzdopen(fd, "rb");
    if (gzf == Z_NULL) {
        printf("Unable to gzdopen %s\n", tierdb_outfilename);
        exit(1);
    }

    // Seek to the right position
    gzseek(gzf, seekTo, SEEK_CUR);
    unsigned short buf;
    gzread(gzf, &buf, 2);
    gzclose(gzf);
    close(fd);
    buf = ntohs(buf);
    return buf;
}

VALUE tierdb_set_value(POSITION pos, VALUE val)
{
	tierdb_cellValue *ptr;

	ptr = tierdb_get_raw(pos);

	/* put it in the right position, but we have to blank field and then
	** add new value to right slot, keeping old slots */
	return (VALUE)((*ptr = (((int)*ptr & ~VALUE_MASK) | (val & VALUE_MASK))) & VALUE_MASK);
}

VALUE tierdb_get_value(POSITION pos)
{
	tierdb_cellValue *ptr;

	ptr = tierdb_get_raw(pos);

	return((VALUE)((int)*ptr & VALUE_MASK)); /* return pure value */
}

VALUE tierdb_get_value_from_lookup_table(POSITION pos)
{
	return (VALUE) (tierdb_get_raw_from_lookup_table(pos) & VALUE_MASK);
}

REMOTENESS tierdb_get_remoteness(POSITION pos)
{
	tierdb_cellValue *ptr;

	ptr = tierdb_get_raw(pos);

	return (REMOTENESS)((((int)*ptr & REMOTENESS_MASK) >> REMOTENESS_SHIFT));
}

REMOTENESS tierdb_get_remoteness_from_lookup_table(POSITION pos)
{
	return (REMOTENESS) ((tierdb_get_raw_from_lookup_table(pos) & REMOTENESS_MASK) >> REMOTENESS_SHIFT);
}

void tierdb_set_remoteness (POSITION pos, REMOTENESS val)
{
	tierdb_cellValue *ptr;

	ptr = tierdb_get_raw(pos);

	if(val > REMOTENESS_MAX) {
		printf("Remoteness request (%d) for " POSITION_FORMAT  " larger than Max Remoteness (%d)\n",val,pos,REMOTENESS_MAX);
		ExitStageRight();
		exit(0);
	}

	/* blank field then add new remoteness */
	*ptr = (VALUE)(((int)*ptr & ~REMOTENESS_MASK) | (val << REMOTENESS_SHIFT));
}

BOOLEAN tierdb_check_visited(POSITION pos)
{
	tierdb_cellValue *ptr;

	ptr = tierdb_get_raw(pos);

	return (BOOLEAN)((((int)*ptr & VISITED_MASK) == VISITED_MASK)); /* Is bit set? */
}

BOOLEAN tierdb_check_visited_from_lookup_table(POSITION pos)
{
	return (BOOLEAN) (((tierdb_get_raw_from_lookup_table(pos) & VISITED_MASK) == VISITED_MASK));
}

void tierdb_mark_visited (POSITION pos)
{
	tierdb_cellValue *ptr;

	ptr = tierdb_get_raw(pos);

	*ptr = (VALUE)((int)*ptr | VISITED_MASK);       /* Turn bit on */
}

void tierdb_unmark_visited (POSITION pos)
{
	tierdb_cellValue *ptr;

	ptr = tierdb_get_raw(pos);

	//printf("unmark pos: %llu\n", pos);

	*ptr = (VALUE)((int)*ptr & ~VISITED_MASK);      /* Turn bit off */
}

void tierdb_set_mex(POSITION pos, MEX mex)
{
	tierdb_cellValue *ptr;

	ptr = tierdb_get_raw(pos);

	*ptr = (VALUE)(((int)*ptr & ~MEX_MASK) | (mex << MEX_SHIFT));
}

MEX tierdb_get_mex(POSITION pos)
{
	tierdb_cellValue *ptr;

	ptr = tierdb_get_raw(pos);

	return (MEX)(((int)*ptr & MEX_MASK) >> MEX_SHIFT);
}

MEX tierdb_get_mex_from_lookup_table(POSITION pos)
{
	return (MEX) ((tierdb_get_raw_from_lookup_table(pos) & MEX_MASK) >> MEX_SHIFT);
}


/***********
 ************
 **	Database functions.
 **
 **	Name: saveDatabase()
 **
 **	Description: writes tierdb to a compressed file in gzip format.
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
 **	Requirements:	tierdb_array contains a valid database of positions
 **			gNumberOfPositions stores the correct number of positions in tierdb_array
 **			kDBName is set correctly.
 **			getOption() returns the correct option number
 **			The Hash Window is initialized
 **
 */


BOOLEAN tierdb_save_database ()
{
	char tierdb_outfilename_partial[TIERDB_OUTFILENAME_PARTIAL_LENGTH_MAX];
	struct stat statbuf;
	statbuf.st_size = 0;

	if(!gHashWindowInitialized)
		return FALSE;

	unsigned long i;
	tierdb_goodCompression = 1;
	tierdb_goodClose = 0;
	POSITION tot = 0,sTot = gCurrentTierSize;

	POSITION start = 0, finish = gCurrentTierSize;

	if(!tierdb_array)
		return FALSE;

	// Make the directory for this game's tierdb's
	mkdir("data", 0755);
	snprintf(tierdb_outfilename_partial, TIERDB_OUTFILENAME_PARTIAL_LENGTH_MAX, "./data/m%s_%d_tierdb",kDBName,getOption());
	mkdir(tierdb_outfilename_partial, 0755);
	sprintf(tierdb_lookupfilename,"./data/m%s_%d_tierdb/lookup", kDBName, getOption());
	mkdir(tierdb_lookupfilename, 0755);

	if (gDBTierStart != -1ULL && gDBTierEnd != -1ULL) { // we're creating a partial tier file!
		snprintf(tierdb_outfilename, TIERDB_OUTFILENAME_LENGTH_MAX, "%s/m%s_%d_%llu__%llu_%llu_minitierdb.dat.gz",
		        tierdb_outfilename_partial, kDBName, getOption(), gCurrentTier, gDBTierStart, gDBTierEnd);
		start = gDBTierStart;
		finish = gDBTierEnd;
		// reset the vars
		gDBTierStart = gDBTierEnd = -1;
	} else {
		snprintf(tierdb_outfilename, TIERDB_OUTFILENAME_LENGTH_MAX, "%s/m%s_%d_%llu_tierdb.dat.gz",
		        tierdb_outfilename_partial, kDBName, getOption(), gCurrentTier);
	}
	sprintf(tierdb_lookupfilename, "./data/m%s_%d_tierdb/lookup/m%s_%d_%llu_tierdb.dat.gz.idx",
		        kDBName, getOption(), kDBName, getOption(), gCurrentTier);
	FILE *indexFP = fopen(tierdb_lookupfilename, "wb");

	if((tierdb_filep = gzopen(tierdb_outfilename, "wb")) == NULL) {
		if(kDebugDetermineValue) {
			printf("Unable to create compressed data file\n");
		}
		return FALSE;
	}

	tierdb_dbVer[0] = htons(tierdb_FILEVER);
	tierdb_numPos[0] = htonl(gMaxPosOffset[1]) | (((POSITION) htonl(gMaxPosOffset[1] >> 32)) << 32);
	tierdb_goodCompression = gzwrite(tierdb_filep, tierdb_dbVer, sizeof(short));
	tierdb_goodCompression = gzwrite(tierdb_filep, tierdb_numPos, sizeof(POSITION));
	for(i=start; i<finish && tierdb_goodCompression; i++) { //convert to network byteorder for platform independence.
		tierdb_array[i] = htons(tierdb_array[i]);
		tierdb_goodCompression = gzwrite(tierdb_filep, tierdb_array+i, sizeof(tierdb_cellValue));
		tot += tierdb_goodCompression;
		tierdb_array[i] = ntohs(tierdb_array[i]);
		
		if ((sizeof(short) + sizeof(POSITION) + (i + 1) * sizeof(tierdb_cellValue)) % FILESIZE == 0 || i + 1 == finish) {
			gzclose(tierdb_filep);
			off_t prevsize = statbuf.st_size;
			stat(tierdb_outfilename, &statbuf);
			fprintf(indexFP, "%ld\n",(long unsigned int)(statbuf.st_size - prevsize));
			if((tierdb_filep = gzopen(tierdb_outfilename, "ab")) == NULL) {
				if(kDebugDetermineValue) {
					printf("Unable to create compressed data file\n");
				}
				return FALSE;
			}
		}
	}
	tierdb_goodClose = gzclose(tierdb_filep);
	fclose(indexFP);

	if(tierdb_goodCompression && (tierdb_goodClose == 0)) {
		if(kDebugDetermineValue && !gJustSolving) {
			printf("File Successfully compressed\n");
		}
		return TRUE;
	} else {
		if(kDebugDetermineValue) {
			fprintf(stderr, "\nError in file compression.\n Error codes:\ngzwrite error: %d\ngzclose error:%d\nBytes To Be Written: " POSITION_FORMAT "\nBytes Written: " POSITION_FORMAT "\n",tierdb_goodCompression, tierdb_goodClose,sTot*4,tot);
		}
		remove (tierdb_outfilename);
		return FALSE;
	}

}

/*
**	Name: loadDatabase()
**
**	Description: loads the compressed file in gzip format into tierdb_array.
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
**	Requirements:	tierdb_array has enough space malloced to store uncompressed database
**			gNumberOfPositions stores the correct number of uncompressed positions in tierdb_array
**			kDBName is set correctly.
**		        getOption() returns the correct option number
**			The Hash Window is initialized
**
**		~Scott
************
***********/

BOOLEAN tierdb_load_database()
{
	if(!gHashWindowInitialized)
		return FALSE;

	POSITION i, maxpos; POSITION j;
	tierdb_goodDecompression = 1;
	tierdb_goodClose = 1;
	BOOLEAN correctDBVer;

	if(!tierdb_array && !gZeroMemPlayer)
		return FALSE;

	int index;
	// always load current tier at BOTTOM, thus it being first
	for (index = 1; index < gNumTiersInHashWindow; index++) {
		if (index == 1 && !gDBLoadMainTier) {         // if solving, DON'T load from file
			maxpos = gMaxPosOffset[index];
			for(j = 0; j < maxpos; j++)
				tierdb_array[j] = undecided;
			continue;
		}
		sprintf(tierdb_outfilename, "./data/m%s_%d_tierdb/m%s_%d_%llu_tierdb.dat.gz",
		        kDBName, getOption(), kDBName, getOption(), gTierInHashWindow[index]);
		if((tierdb_filep = gzopen(tierdb_outfilename, "rb")) == NULL) {
			if (gOpponent == AgainstEvaluator) { // go ahead and ignore the loading of the DB
				maxpos = gMaxPosOffset[index];
				for(j = 0; j < maxpos; j++)
					tierdb_array[j] = undecided;
				continue;
			} else return FALSE;
		}
		tierdb_goodDecompression = gzread(tierdb_filep,tierdb_dbVer,sizeof(short));
		tierdb_goodDecompression = gzread(tierdb_filep,tierdb_numPos,sizeof(POSITION));
		*tierdb_dbVer = ntohs(*tierdb_dbVer);
		*tierdb_numPos = ntohl(*tierdb_numPos) | (((POSITION) ntohl(*tierdb_numPos >> 32)) << 32);
		if(*tierdb_numPos != (gMaxPosOffset[index]-gMaxPosOffset[index-1])) {
			if (kDebugDetermineValue)
				printf("\n\nError in file decompression: Stored gNumberOfPositions differs from internal gNumberOfPositions\n\n");
			return FALSE;
		}
		correctDBVer = (*tierdb_dbVer == tierdb_FILEVER);
		if (correctDBVer) {
			maxpos = gMaxPosOffset[index];
			for(i = gMaxPosOffset[index-1]; i < maxpos && tierdb_goodDecompression; i++) {
				tierdb_goodDecompression = gzread(tierdb_filep, tierdb_array+i, sizeof(tierdb_cellValue));
				tierdb_array[i] = ntohs(tierdb_array[i]);
			}
		}
		tierdb_goodClose = gzclose(tierdb_filep);
		if(!(tierdb_goodDecompression && (tierdb_goodClose == 0) && correctDBVer)) {
			if(kDebugDetermineValue)
				printf("\n\nError in file decompression:\ngzread error: %d\ngzclose error: %d\ndb version: %d\n",tierdb_goodDecompression,tierdb_goodClose,*tierdb_dbVer);
			return FALSE;
		}
		gTierDBExists[index] = TRUE; // lets static evaluator know that this tierdb actually exists!
	}
	if(kDebugDetermineValue)
		printf("Files Successfully Decompressed\n");
	return TRUE;
}

/* A helper to solveretrograde which simply checks for the existance of a DB.
 * Error Codes: 0 = Doesn't exist, -1 = Incorrect/corrupted, 1 = Exists. */
int CheckTierDB(TIER tier, int variant) {
	sprintf(tierdb_outfilename, "./data/m%s_%d_tierdb/m%s_%d_%llu_tierdb.dat.gz",
	        kDBName, variant, kDBName, variant, tier);
	if((tierdb_filep = gzopen(tierdb_outfilename, "rb")) == NULL) {
		return 0;
	}
	tierdb_goodDecompression = gzread(tierdb_filep,tierdb_dbVer,sizeof(short));
	tierdb_goodDecompression = gzread(tierdb_filep,tierdb_numPos,sizeof(POSITION));
	*tierdb_dbVer = ntohs(*tierdb_dbVer);
	*tierdb_numPos = ntohl(*tierdb_numPos) | (((POSITION) ntohl(*tierdb_numPos >> 32)) << 32);
	tierdb_goodClose = gzclose(tierdb_filep);
	if(!tierdb_goodDecompression || (*tierdb_numPos != gNumberOfTierPositionsFunPtr(tier))
	   || (*tierdb_dbVer != tierdb_FILEVER) || (tierdb_goodClose != 0)) {
		return -1;
	}
	return 1;
}

/* Yet another helper. Overrides the positions between gDBTierStart and
 * gDBTierEnd with the values from the minifile. */
BOOLEAN tierdb_load_minifile(char* filename)
{
	if(!gHashWindowInitialized && !tierdb_array)
		return FALSE;

	POSITION i;
	tierdb_goodDecompression = 1;
	tierdb_goodClose = 1;
	BOOLEAN correctDBVer;

	sprintf(tierdb_outfilename, "./data/m%s_%d_tierdb/%s",
	        kDBName, getOption(), filename);
	if((tierdb_filep = gzopen(tierdb_outfilename, "rb")) == NULL)
		return FALSE;
	tierdb_goodDecompression = gzread(tierdb_filep,tierdb_dbVer,sizeof(short));
	tierdb_goodDecompression = gzread(tierdb_filep,tierdb_numPos,sizeof(POSITION));
	*tierdb_dbVer = ntohs(*tierdb_dbVer);
	*tierdb_numPos = ntohl(*tierdb_numPos) | (((POSITION) ntohl(*tierdb_numPos >> 32)) << 32);
	if(*tierdb_numPos != gCurrentTierSize)
		return FALSE;
	correctDBVer = (*tierdb_dbVer == tierdb_FILEVER);
	if (correctDBVer) {
		for(i = gDBTierStart; i < gDBTierEnd && tierdb_goodDecompression; i++) {
			tierdb_goodDecompression = gzread(tierdb_filep, tierdb_array+i, sizeof(tierdb_cellValue));
			tierdb_array[i] = ntohs(tierdb_array[i]);
		}
	}
	tierdb_goodClose = gzclose(tierdb_filep);
	if(!(tierdb_goodDecompression && (tierdb_goodClose == 0) && correctDBVer))
		return FALSE;
	return TRUE;
}
