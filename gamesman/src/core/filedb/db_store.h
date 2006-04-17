
/************************************************************************
**
** NAME:	db_store.h
**
** DESCRIPTION:	Gamescrafters Database File Functions
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

//a db store implementation. it only knows about pages, not records.
//all read/write ops are on pages, which are groups of more than one records.

#ifndef GMCORE_DB_FILE_H
#define GMCORE_DB_FILE_H

#include "db_globals.h"

#include <zlib.h>

typedef struct dbfile_struct{
  char* filename;   //disk file name
 // page_id* pagemap;
  gzFile* filep;	//the disk file descriptor
  page_id current_page;  //the current page
  page_id total_pages; //offset of the next page after the end
}db_store;

db_store* 	db_open		(char* filename);
int 		db_close	(db_store* db);
void		db_seek		(db_store* db, page_id page);
int 		db_read		(db_store* db, page_id page, db_buffer_page* buf);
int 		db_write	(db_store* db, page_id page, db_buffer_page* buf);
//page_id 	db_newPage  (db_store* db);

#endif /* GMCORE_DB_FILE_H */
