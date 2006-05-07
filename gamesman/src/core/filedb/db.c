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
gamesdb* gamesdb_create(int rec_size, gamesdb_pageid max_pages, char* db_name){
//  db_bman* bman;
  gamesdb_buffer* bufp;
  gamesdb* data;
  gamesdb_store* storep = gamesdb_open(db_name);

    bufp = gamesdb_buf_init(rec_size, max_pages, storep);

//  bman = bman_init(bufp);

  data = (gamesdb*) gamesdb_SafeMalloc(sizeof(gamesdb));

//  data->buf_man = bman;
  data->buffers = bufp;
  data->store = storep;
  //data->rec_size = rec_size;
  //data->pg_size = RECS_PER_PAGE * rec_size;
  data->num_page = max_pages;

  return data;
} 

void gamesdb_destroy(gamesdb* data){

//  bman_destroy(data->buf_man);
  gamesdb_buf_destroy(data->buffers);
  gamesdb_close(data->store);
  gamesdb_SafeFree(data);

}

  
void gamesdb_get(gamesdb* gdb, char* mem, gamesdb_position pos){

	gamesdb_buf_read(gdb->buffers, pos, mem);
}


void gamesdb_put(gamesdb* gdb, char* mem, gamesdb_position pos){

	gamesdb_buf_write(gdb->buffers, pos, mem);
}
