/************************************************************************
**
** NAME:	db_basichash.h
**
** DESCRIPTION:	Basic Hash Table implementation.
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

#ifndef GMCORE_DB_BASICHASH_H
#define GMCORE_DB_BASICHASH_H

#include  "db_buf.h"

typedef struct db_bhashin_struct {
  int num;
  frame_id* loc;
  page_id* id;
  struct db_bhashin_struct* next;
} db_bhashin;


typedef struct db_bhash_struct {
  int size;
  db_bhashin* rows;

} db_bhash;


db_bhash* 	db_basichash_create		(int num_rows,int num_in);
frame_id 	db_basichash_get		(db_bhash* hash, page_id id);
frame_id 	db_basichash_remove		(db_bhash* hash, page_id id);
int 		db_basichash_put		(db_bhash* hash, page_id, frame_id );
void 		db_basichash_destroy	(db_bhash* hash);

//Helpers
db_bhashin* db_basichash_getin(db_bhash* hash, page_id id);


#endif /* GMCORE_DB_BASICHASH_H */
