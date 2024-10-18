/************************************************************************
**
** NAME:	db_store.c
**
** DESCRIPTION:	Creates and opens database file handlers.
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

#include "db_types.h"
#include "db_store.h"
#include "db_malloc.h"
#include "db_buf.h"
#include "assert.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stddef.h>
#include <dirent.h>
#include <assert.h>

/*
** Opens a database with filename.
** returns db_store pointer on success (freed in db_close)
** null on failure.
*/
gamesdb_store* gamesdb_open(gamesdb* db, char* filename, int cluster_size){

	char *dirname = (char *) gamesdb_SafeMalloc (sizeof(char) * (strlen(filename) + 10));
	sprintf(dirname, "./data/%s", filename);
	mkdir ("./data", 0755);
	mkdir (dirname, 0755);

	gamesdb_boolean verify_ok = GAMESDB_FALSE;

	//verify geometry parameters - sizes for things (records, buffers, and dir clusters)
	char geometry_filename[GAMESDB_MAX_FILENAME_LEN] = "\0";
	sprintf(geometry_filename, "%s/%s", dirname, GAMESDB_GEOMETRY_FILENAME);
	FILE *geometry_file = fopen(geometry_filename, "r");

	if (geometry_file == NULL) {
		//create a new geometry file, and write out the info
		geometry_file = fopen(geometry_filename, "w");
		fprintf(geometry_file, "%d\n%d\n%d",
		        db->buffer->rec_size,
		        db->buffer->buf_size,
		        cluster_size);
		fclose(geometry_file);
		verify_ok = GAMESDB_TRUE;
	} else { //otherwise, get the data and check
		int frec_size = -1, fbuf_size = -1, fcluster_size = -1;
		fscanf(geometry_file, "%d\n%d\n%d",
		       &frec_size,
		       &fbuf_size,
		       &fcluster_size);
		if ((frec_size == db->buffer->rec_size) &&
		    (fbuf_size == db->buffer->buf_size) &&
		    (fcluster_size == cluster_size)) {
			verify_ok = GAMESDB_TRUE;
		}
		fclose(geometry_file);
	}

	if (!verify_ok) {
		printf("Failed to open game database: geometry mismatch.\n");
		return NULL;
	}

	gamesdb_SafeFree(dirname);

	gamesdb_store* db_store = (gamesdb_store*) gamesdb_SafeMalloc(sizeof(gamesdb_store));

	db_store->filename = (char*) gamesdb_SafeMalloc (sizeof(char)*strlen(filename));
	strcpy(db_store->filename, filename);

	db_store->dir_size = cluster_size;
	db_store->current_page = 0;

	return db_store;
}

/*
** Closes the db file handle.
** frees up allocated memory.
** returns 0 on success. Anything else on error.
*/
int gamesdb_close(gamesdb_store* db){
	gamesdb_SafeFree(db->filename);
	gamesdb_SafeFree(db);

	return 0;
}

//makes all directory levels necessary to house the page with tag page_no
static gamesdb_pageid gamesdb_checkpath(char *base_path, gamesdb_pageid page_no, int dir_size) {

	//unsigned int current_pos = 0;
	gamesdb_pageid this_cluster = 0;
	char temp[GAMESDB_MAX_FILENAME_LEN] = "";
	//DIR *data_dir = NULL;

	while (page_no >= (1 << dir_size)) {

		this_cluster = page_no & ((1 << dir_size) - 1);

		sprintf(temp, "/%llu", this_cluster);
		strcat(base_path, temp);

		//DIR *data_dir = opendir(base_path);

		//if (data_dir == NULL) {
		mkdir (base_path, 0755);
		//}

		//closedir (data_dir);
		//data_dir = NULL;

		page_no >>= dir_size;
	}

	this_cluster = page_no;

	sprintf(temp, "/%llu.dat", this_cluster);
	strcat(base_path, temp);

	return page_no;
}

//writes a page into the database
int gamesdb_write(gamesdb* db, gamesdb_pageid page, gamesdb_bufferpage* buf){

	assert(buf->valid == GAMESDB_TRUE);
	assert(buf->tag == page);

	char filename[GAMESDB_MAX_FILENAME_LEN] = "";

	gamesdb_store *dbfile = db->store;
	gamesdb_buffer *bufp = db->buffer;

	sprintf(filename, "./data/%s", dbfile->filename);
	gamesdb_checkpath(filename, page, dbfile->dir_size);

	gzFile pagefile = gzopen(filename, "w+");

	if (GAMESDB_DEBUG)
		printf ("db_write: path = %s, page = %llu\n", filename, page);

	//write data
	//Remember to write data in the same order and sizes as when you read it
	gzwrite(pagefile, (void*)(buf->mem), sizeof(char) * bufp->buf_size * bufp->rec_size);
	gzwrite(pagefile, (void*)&(buf->chances), sizeof(gamesdb_counter));
	gzwrite(pagefile, (void*)&(buf->tag), sizeof(gamesdb_pageid));
	//gzwrite(pagefile, (void*)&(buf->valid), sizeof(gamesdb_boolean));

	gzclose(pagefile);
	return 0;
}

int gamesdb_read(gamesdb* db, gamesdb_pageid page, gamesdb_bufferpage* buf){

	char filename[GAMESDB_MAX_FILENAME_LEN] = "";

	gamesdb_store *dbfile = db->store;
	gamesdb_buffer *bufp = db->buffer;

	sprintf(filename, "./data/%s", dbfile->filename);
	gamesdb_checkpath(filename, page, dbfile->dir_size);

	gzFile pagefile = gzopen(filename, "r+");

	if (GAMESDB_DEBUG) {
		printf ("db_read: path = %s, page = %llu\n", filename, page);
	}

	if(pagefile != NULL) {
		//read data
		//Remember to read data in the same order and sizes as when you wrote it
		gzread(pagefile, (void*)(buf->mem), sizeof(char) * bufp->buf_size * bufp->rec_size);
		gzread(pagefile, (void*)&(buf->chances), sizeof(gamesdb_counter));
		gzread(pagefile, (void*)&(buf->tag), sizeof(gamesdb_pageid));
		//gzread(pagefile, (void*)&(buf->valid), sizeof(gamesdb_boolean));
		buf->valid = GAMESDB_TRUE;
		//assert(buf->valid == TRUE);

	} else { //page does not exist in disk
		if (GAMESDB_DEBUG) {
			printf ("db_read: starting a fresh page.\n");
		}
		memset(buf->mem, 0, sizeof(char) * bufp->buf_size * bufp->rec_size);
		buf->chances = 0;
		buf->tag = 0;
		buf->valid = GAMESDB_FALSE;
	}

	//the caller will take care of the dirty bit

	gzclose(pagefile);

	return 0;

}
