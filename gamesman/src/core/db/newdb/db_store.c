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
db_store* db_open(char* filename, char* options){
  db_store* db = (db_store*) SafeMalloc(sizeof(db_store));
  db->filep = gzopen(filename,options);
  db->filename = (char*) SafeMalloc (sizeof(char)*strlen(filename));
  strcpy(filename,db->filename);
  db->offset = 0;
  if(db->filep)
    return db;
  
  db_close(db);
  return NULL;
}


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



//Reads / Writes the 'len' number of bytes.

int db_write(db_store* db, const void* buf,unsigned len){
  return gzwrite(db->filep,buf,len);
}

int db_read(db_store* db, void* buf,unsigned len){
  return gzread(db->filep,buf,len);
}

//WARNING. DOES NOT SUPPORT SEEK_END
db_offset db_seek(db_store* db,db_offset off,int from){
  if(from == SEEK_END)
    return db->offset;
  db_offset new = gzseek(db->filep,off,from);//wow we dont catch -1!
  //Also bad because its the wrong type you dip!
  
  db->offset = new; //So bad. So lazy. 
  return new;
}

db_offset db_tell(db_store* db){
  return db->offset;
}

db_offset db_nextBlock(db_store* db, db_offset inc){
  db_offset ret = db->next_off;
  db->next_off += inc;
  return ret;
}

void db_store_hideMagic(db_store* filep){
  //yeah... ill do this later. we can assume its right.
  filep->next_off = 0;
}

void db_store_discoverMagic(db_store* filep){
  //do it later.
  filep->next_off = 0;
}
