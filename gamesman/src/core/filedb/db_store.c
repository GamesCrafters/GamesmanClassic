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

#include "db_store.h"
#include "db_malloc.h"
#include "assert.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*
**Basic wrapper for the libz functions. wrapped so that future students
**can implement it however they choose.
*/

/*
** Opens a database with filename.
** returns db_store pointer on success (freed in db_close)
** null on failure. 
*/
db_store* db_open(char* filename){
  db_store* db = (db_store*) SafeMalloc(sizeof(db_store));
  
  boolean olddb = TRUE;

  db->filep = gzopen(filename,"r+");
  if(db->filep == NULL){
    db->filep = gzopen(filename,"w+");
    olddb = FALSE;
  }
  if(db->filep == NULL)
    return NULL;
  
//  get the magic number - the offset at which the pagemap begins
// not used right now, we depend on zlib to minimize cost of gaps
//	page_id magic = 0;
//  gzread(db->filep,

  db->filename = (char*) SafeMalloc (sizeof(char)*strlen(filename));
  
  strcpy(db->filename, filename);
  
  db->current_page = 0;
  
  //this is the last page of the chain
  //it has not been written, so be sure to write it when expanding
  db->last_page = 0;
  
  if(db->filep)
    return db;
  
  db_close(db);
  
  return NULL;
}

//page_id getMagicNumber(gzFile* filep) {
//	return 
//	
//}

/*
** Closes the db file handle.
** frees up allocated memory.
** returns 0 on success. Anything else on error.
*/
int db_close(db_store* db){
  SafeFree(db->filename);
  if(db->filep)
    gzclose(db->filep);
  SafeFree(db);

  return 0;
}

//writes a page into the database
int db_write(db_store* db, page_id page, db_buffer_page* buf){
	//grows the file on-demand
	if (page >= db->last_page) {
		//breakage of abstraction, beware
		void* temp = calloc(sizeof(db_buffer_page), sizeof(char));
		gzseek(db->filep, db->last_page*sizeof(db_buffer_page), SEEK_SET); //go to the end of the file
		
		while (db->last_page <= page) { //fill zeroes
			gzwrite(db->filep, temp, sizeof(db_buffer_page));
			gzflush(db->filep, Z_FULL_FLUSH); //flush it to allow random access
			db->last_page++;
		}
		
		free(temp);
	}

	//even if you are writing to the same page,
	//you still have to seek back a page
	db_seek(db, page);
	
	printf ("db_write: page = %llu, last_page = %llu\n", page, db->last_page);
	//write data
    gzwrite(db->filep, (void*)buf, sizeof(db_buffer_page));
	//flush all data so that decompression can restart at this point    
    gzflush(db->filep, Z_FULL_FLUSH);
    return 0;
}

int db_read(db_store* db, page_id page, db_buffer_page* buf){
/*	if (page >= db->last_page) { // if we are reading past the file, just return 0's
			memset((void*)buf, 0x00000000, sizeof(db_buffer_page));
			db->current_page = page;
			return 0;
	}
*/	

	//grows the file on-demand
	if (page >= db->last_page) {
		//breakage of abstraction, beware
		void* temp = calloc(sizeof(db_buffer_page), sizeof(char));
		gzseek(db->filep, db->last_page*sizeof(db_buffer_page), SEEK_SET); //go to the end of the file
		
		while (db->last_page <= page) { //fill zeroes
			gzwrite(db->filep, temp, sizeof(db_buffer_page));
			gzflush(db->filep, Z_FULL_FLUSH); //flush it to allow random access
			db->last_page++;
		}
		
		free(temp);
	}

	db_seek(db, page);
	printf ("db_read: page = %llu, last_page = %llu\n", page, db->last_page);
	return gzread(db->filep, (void*)buf, sizeof(db_buffer_page));
}

//seeks in the file to the beginning of a given page.
//the page must already exist in the file.
//that is, page < last_page (because the page with index last_page is not written)
void db_seek(db_store* db, page_id page){
	db_offset new;
	int from = SEEK_SET;
	
//	if(page != db->current_page) {
//		from = SEEK_SET;
		//if(page > db->current_page) {
		  //	from = SEEK_CUR;
		  //	page -= db->current_page;
		//}
		
	  	//printf("db_seek: offset = %llu, page = %llu, current_page = %llu, last page = %llu\n", page*sizeof(db_buffer_page), page, db->current_page, db->last_page);
		//new = (db_offset)gzseek(db->filep,page*sizeof(db_buffer_page),from);
	
	if (page <= db->current_page) {//this means we are rewinding in write mode
	  	//close and reopen the zip file
	  	gzclose(db->filep);
	  	db->filep = gzopen(db->filename, "r+");
	  	//if (from == SEEK_CUR)
	  	//	page += db->current_page;
		printf("db_seek: BACKWARD offset = %llu, page = %llu, current_page = %llu, last page = %llu\n", page*sizeof(db_buffer_page), page, db->current_page, db->last_page);
	} else {
		printf("db_seek: FORWARD offset = %llu, page = %llu, current_page = %llu, last page = %llu\n", page*sizeof(db_buffer_page), page, db->current_page, db->last_page);
	}
	
	page_id i;
	
	while(gztell(db->filep) < page*sizeof(db_buffer_page)) {
		new = (db_offset)gzseek(db->filep, sizeof(db_buffer_page), SEEK_CUR);

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

page_id db_tell(db_store* db){
  return db->current_page;
}

/*//gives the position of a new page in the end of the fisk file, in bytes
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
