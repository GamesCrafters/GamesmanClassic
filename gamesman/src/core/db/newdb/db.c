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

#define RECS_PER_PAGE 100

/*
 * allocates memory and sets up a gamescrafters db. Must call destructive
 * function when done to free up memory.
 */
games_db* db_create(Position num_rec, int rec_size, Position max_mem, char* db_name){
  db_store* filep;
  db_bman* bman;
  db_buf_head* bufp;
  games_db* data;
  boolean olddb = TRUE;
  frame_id num_buf = max_mem / (rec_size * RECS_PER_PAGE);

  filep = db_open(db_name,"r+");
  if(filep == NULL){
    filep = db_open(db_name,"w+");
    olddb = FALSE;
  }
  if(filep == NULL)
    return NULL;

  
  
  if(olddb){
    db_store_discoverMagic(filep); // find header
    bufp = db_buf_fileinit(filep);
  } else {
    db_store_hideMagic(filep); //Tack on the header to the file.
    bufp = db_buf_init(rec_size,num_buf, RECS_PER_PAGE,filep);
    //db_buf_filesetu(bufp);
  }
  bman = bman_init(bufp);//,NULL);//need to make a buffer replacement strat.

  data = (games_db*) SafeMalloc(sizeof(games_db));

  data->buf_man = bman;
  data->buffers = bufp;
  data->filep = filep;
  data->num_buf = num_buf;
  //data->rec_size = rec_size;
  //data->pg_size = RECS_PER_PAGE * rec_size;
  data->num_page = num_rec / (rec_size*RECS_PER_PAGE);
  if(num_rec % (rec_size*RECS_PER_PAGE) != 0)
    data->num_page++;



  return data;
} 

void db_destroy(games_db* data){

  bman_destroy(data->buf_man);
  db_buf_destroy(data->buffers);
  db_close(data->filep);
  SafeFree(data);

}

  
void db_get(games_db* gdb, char* mem, Position pos){
  page_id pid = pos / RECS_PER_PAGE;
  frame_id fid;
  int off = pos - pid*RECS_PER_PAGE; 
  int byte_off = off;// * gdb->rec_size;
  
  if(off > RECS_PER_PAGE)
    fprintf(stderr,"Error in database. db.c db_get >>  offset way large");
  
  
  fid = bman_find(gdb->buf_man,pid);
  //  db_buf_copyOut(gdb->bufp,mem,fid,byte_off,gdb->rec_size);

}


void db_put(games_db* gdb, char* mem, Position pos){
  page_id pid = pos / RECS_PER_PAGE;
  frame_id fid;
  int off = pos - pid*RECS_PER_PAGE; 
  int byte_off = off; //* gdb->rec_size;
  
  if(off > RECS_PER_PAGE)
    fprintf(stderr,"Error in database. db.c db_get >>  offset way large");
  
  
  fid = bman_find(gdb->buf_man,pid);
  //db_buf_copyIn(gdb->bufp,mem,fid,byte_off,gdb->rec_size);

}
