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
#include "assert.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stddef.h>
#include <dirent.h>
/*
**Basic wrapper for the libz functions. wrapped so that future students
**can implement it however they choose.
*/

/*
** Opens a database with filename.
** returns db_store pointer on success (freed in db_close)
** null on failure. 
*/
gamesdb_store* gamesdb_open(char* filename){
  gamesdb_store* db = (gamesdb_store*) gamesdb_SafeMalloc(sizeof(gamesdb_store));
  
  db->filename = (char*) gamesdb_SafeMalloc (sizeof(char)*strlen(filename));
  
  strcpy(db->filename, filename);
  
  db->current_page = 0;
  
  //this is the last page of the chain
  //it has not been written, so be sure to write it when expanding
  db->last_page = 0;
  
  char *dirname = (char *) gamesdb_SafeMalloc (sizeof(char) * (strlen(filename) + 10));
  
  sprintf(dirname, "./data/%s", filename);
  
  DIR *data_dir = opendir("./data");
  
  if (data_dir == NULL) {
  	mkdir ("./data", 0755);
  	mkdir (dirname, 0755);
  }
  
  closedir (data_dir);
  
  if ((data_dir = opendir(dirname)) == NULL && mkdir(dirname, 0755) == -1) {
  	printf ("filedb: Cannot make output directory. Aborting.");
  	exit(1);
  }
  
  closedir(data_dir); 
  free(dirname);
  
  return db; //used to be NULL
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
gamesdb_pageid gamesdb_checkpath(char *base_path, gamesdb_pageid page_no) {
	
	unsigned int current_pos = 0;
	gamesdb_pageid this_cluster = 0;
	char temp[MAX_FILENAME_LEN] = "";
	
	//while (base_path[current_pos] != 0 && current_pos < MAX_FILENAME_LEN) {
	//	current_pos++;
	//}
	
	while (page_no >= (1 << CLUSTER_SIZE)) {
		
//		base_path[current_pos] = '/';
//		current_pos ++;
		
		this_cluster = page_no && ((1 << CLUSTER_SIZE) - 1);
		
//		do {
		sprintf(temp, "/%llu", this_cluster);
		strcat(base_path, temp);
//		this_cluster /= 10;
//			current_pos ++;
//		} while (this_cluster > 0);
		
		DIR *data_dir = opendir(base_path);
  
  		if (data_dir == NULL) {
  			mkdir (base_path, 0755);
  		}
  
  		closedir (data_dir);
  		
  		page_no >>= CLUSTER_SIZE;
	}
	
	this_cluster = page_no;
		
//	do {
		sprintf(temp, "/%llu", this_cluster);
		strcat(base_path, temp);
//		this_cluster /= 10;
//	} while (this_cluster > 0);
	
//	base_path[current_pos] = 0; //don't forget to terminate string
	
//	printf("%s\n", base_path);
	
	return page_no;
}

//writes a page into the database
int gamesdb_write(gamesdb* db, gamesdb_pageid page, gamesdb_bufferpage* buf){

	char filename[MAX_FILENAME_LEN] = "";
	
	gamesdb_store *dbfile = db->store;
	
	sprintf(filename, "./data/%s", dbfile->filename);
	gamesdb_checkpath(filename, page);

	gzFile pagefile = gzopen(filename, "w+");
	
	if (DEBUG)
		printf ("db_write: page = %llu, last_page = %llu\n", page, dbfile->last_page);

	//write data
	//Remember to write data in the same order and sizes as when you read it
	gzwrite(pagefile, (void*)buf->mem, sizeof(char) * db->buffers->buf_size * db->buffers->rec_size);
    gzwrite(pagefile, (void*)&buf->chances, sizeof(gamesdb_counter));
    gzwrite(pagefile, (void*)&buf->tag, sizeof(gamesdb_pageid));
    gzwrite(pagefile, (void*)&buf->valid, sizeof(gamesdb_boolean));

    gzclose(pagefile);
    return 0;
}

int gamesdb_read(gamesdb* db, gamesdb_pageid page, gamesdb_bufferpage* buf){

	char filename[MAX_FILENAME_LEN] = "";
	
	gamesdb_store *dbfile = db->store;
	
	sprintf(filename, "./data/%s", dbfile->filename);
	gamesdb_checkpath(filename, page);

	gzFile pagefile = gzopen(filename, "r+");
	
	if (DEBUG) {
		printf ("db_read: page = %llu, last_page = %llu\n", page, dbfile->last_page);
	}
	
	if(pagefile != NULL) {
		//read data
		//Remember to read data in the same order and sizes as when you wrote it
        gzread(pagefile, (void*)buf->mem, sizeof(char) * db->buffers->buf_size * db->buffers->rec_size);
	    gzread(pagefile, (void*)&buf->chances, sizeof(gamesdb_counter));
	    gzread(pagefile, (void*)&buf->tag, sizeof(gamesdb_pageid));
	    gzread(pagefile, (void*)&buf->valid, sizeof(gamesdb_boolean));

	} else { //page does not exist in disk
		if (DEBUG) {
			printf ("db_read: starting a fresh page.\n");
		}
		memset(buf->mem, 0, sizeof(char) * db->buffers->buf_size * db->buffers->rec_size);
		buf->chances = 0;
		buf->tag = 0;
		buf->valid = FALSE;
	}

	//the caller will take care of the dirty bit

    gzclose(pagefile);

    return 0;

}

//seeks in the file to the beginning of a given page.
//the page must already exist in the file.
//that is, page < last_page (because the page with index last_page is not written)
//NOT USED
//void gamesdb_seek(gamesdb_store* db, gamesdb_pageid page){
//	gamesdb_offset new;
//	int from = SEEK_SET;
	
//	if(page != db->current_page) {
//		from = SEEK_SET;
		//if(page > db->current_page) {
		  //	from = SEEK_CUR;
		  //	page -= db->current_page;
		//}
		
	  	//printf("db_seek: offset = %llu, page = %llu, current_page = %llu, last page = %llu\n", page*sizeof(db_buffer_page), page, db->current_page, db->last_page);
		//new = (db_offset)gzseek(db->filep,page*sizeof(db_buffer_page),from);
/*	
	if (page <= db->current_page) {//this means we are rewinding in write mode
	  	//close and reopen the zip file
	  	gzclose(db->filep);
	  	db->filep = gzopen(db->filename, "r+");
	  	//if (from == SEEK_CUR)
	  	//	page += db->current_page;
		printf("db_seek: BACKWARD offset = %llu, page = %llu, current_page = %llu, last page = %llu\n", page*sizeof(gamesdb_bufferpage), page, db->current_page, db->last_page);
	} else {
		printf("db_seek: FORWARD offset = %llu, page = %llu, current_page = %llu, last page = %llu\n", page*sizeof(gamesdb_bufferpage), page, db->current_page, db->last_page);
	}
	
	gamesdb_pageid i;
	
	while(gztell(db->filep) < page*sizeof(gamesdb_bufferpage)) {
		new = (gamesdb_offset)gzseek(db->filep, sizeof(gamesdb_bufferpage), SEEK_CUR);

		//printf("%d\n", new);

		assert(new != -1);
	}
//	}
  
	//if(page > db->total_pages)
	//	db->total_pages = page;
	//if(from == SEEK_CUR)
	//	page += db->current_page;
		
	db->current_page = page; //So bad. So lazy. 
	
	//return new;
}
//NOT USED
gamesdb_pageid db_tell(gamesdb_store* db){
  return db->current_page;
}

//gives the position of a new page in the end of the fisk file, in bytes
db_offset db_nextBlock(db_store* db, db_offset inc){
  db_offset ret = db->next_off;
  db->next_off += inc;
  return ret;
}*/

/*void db_store_hideMagic(db_store* filep){
  //yeah... ill do this later. we can assume its right.
  filep->next_off = 0;
}

void db_store_discoverMagic(db_store* filep){
  //do it later.
  filep->next_off = 0;
}*/
