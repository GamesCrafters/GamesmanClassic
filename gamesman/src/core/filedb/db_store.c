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
  
  db->total_pages = 0;
  
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
	if (page != db->current_page)
		db_seek(db, page);
  return gzwrite(db->filep, buf, sizeof(db_buffer_page));
}

int db_read(db_store* db, page_id page, db_buffer_page* buf){
	if (page != db->current_page)
		db_seek(db, page);
  return gzread(db->filep, buf, sizeof(db_buffer_page));
}

void db_seek(db_store* db, page_id page){
	db_offset new;
	if(page != db->current_page) {
		int from = SEEK_SET;
		if(page > db->current_page) {
		  	from = SEEK_CUR;
		  	page -= db->current_page;
		}
		
		new = (db_offset)gzseek(db->filep,page*sizeof(db_buffer_page),from);
	
		if (new == -1) {//this means we are rewinding in write mode
		  	//close and reopen the zip file
		  	gzclose(db->filep);
		  	db->filep = gzopen(db->filename, "r+");
		  	if (from == SEEK_CUR)
		  		page += db->current_page;
		  	new = (db_offset)gzseek(db->filep, page*sizeof(db_buffer_page), SEEK_SET);
		}

		//assert(new != -1);
	}
  
	if(page > db->total_pages)
		db->total_pages = page;
		
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
