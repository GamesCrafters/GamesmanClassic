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

#ifndef GMCORE_DB_FILE_H
#define GMCORE_DB_FILE_H

#include <zlib.h>

typedef unsigned long long db_offset;

typedef struct dbfile_struct{
  char* filename;
  gzFile* filep;
  db_offset offset;
  db_offset next_off;
}db_store;

db_store* db_open(char* filename,char* options);
int db_close(db_store* db);
int db_write(db_store* db, const void* buf,unsigned len);
int db_read(db_store* db, void* buf,unsigned len);
db_offset db_seek(db_store* db, db_offset off,int from);
db_offset db_tell(db_store* db);
db_offset db_nextBlock(db_store* db,db_offset inc);

#endif /* GMCORE_DB_FILE_H */
