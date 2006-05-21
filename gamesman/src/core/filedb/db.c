/************************************************************************
**
** NAME:	db.c
**
** DESCRIPTION:	Gamescrafters Database Creation and Modification
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
#include "db.h"
#include "db_globals.h"
#include <stdio.h>
/*
 * allocates memory and sets up a gamescrafters db. Must call destructive
 * function when done to free up memory.
 */
games_db* db_create(int rec_size, page_id max_pages, char* db_name){
  db_store* filep;
//  db_bman* bman;
  db_buffer* bufp;
  games_db* data;
  filep = db_open(db_name);

    bufp = db_buf_init(rec_size, max_pages, filep);

//  bman = bman_init(bufp);

  data = (games_db*) SafeMalloc(sizeof(games_db));

//  data->buf_man = bman;
  data->buffers = bufp;
  data->filep = filep;
  //data->rec_size = rec_size;
  //data->pg_size = RECS_PER_PAGE * rec_size;
  data->num_page = max_pages;

  return data;
} 

void db_destroy(games_db* data){

//  bman_destroy(data->buf_man);
  db_buf_destroy(data->buffers);
  db_close(data->filep);
  SafeFree(data);

}

  
void db_get(games_db* gdb, char* mem, Position pos){

	db_buf_read(gdb->buffers, pos, mem);
}


void db_put(games_db* gdb, char* mem, Position pos){

	db_buf_write(gdb->buffers, pos, mem);
}
