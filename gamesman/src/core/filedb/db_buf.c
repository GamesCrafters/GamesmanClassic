/************************************************************************
**
** NAME:	db_buf.c
**
** DESCRIPTION:	Buffer factory and modifiers.
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
#include "db_buf.h"
#include "db_malloc.h"
#include "db_store.h"
#include <stdio.h>
#include <string.h>

// a buffer contains at least one record.
//each record has rec_size bytes, along with one extra byte to keep track of valid bit
gamesdb_buffer* gamesdb_buf_init(int rec_size, gamesdb_pageid num_buf){
  	gamesdb_buffer* bufp = (gamesdb_buffer*) gamesdb_SafeMalloc(sizeof(gamesdb_buffer));
  
  	//bufp->filep = filep;
  	bufp->n_buf = num_buf;
  
  	//a page contains as many records as possible, up to 4K-9 bytes.
  	//The 9 bytes are for book-keeping. One extra byte per record is used to denote whether it's
  	//been written to or not
  	//There will be wasted space in the mem array but shouldn't be too great if you have huge pages
  	bufp->rec_size = rec_size + 1; //one byte for use by the db code (valid bit, etc)
  	bufp->buf_size = MEM_ARRAY_SIZE / (rec_size+1); //this is how many records are in one page
  	bufp->dirty = (gamesdb_boolean*) gamesdb_SafeMalloc (sizeof(gamesdb_boolean) * num_buf);
  	//bufp->chances = (gamesdb_counter*) gamesdb_SafeMalloc (sizeof(gamesdb_counter) * num_buf);
  	bufp->buffers = (gamesdb_bufferpage*) gamesdb_SafeMalloc (sizeof(gamesdb_bufferpage) * num_buf);
 
	//zeroes out the buffers for use
	gamesdb_pageid i,j;
	gamesdb_bufferpage *buf;
	for(i=0;i<num_buf;i++){
//    bufp->buffers[i]->mem = (char*) SafeMalloc(sizeof(char) * rec_size);
		buf = bufp->buffers+i;
		for (j=0; j<MEM_ARRAY_SIZE; j++)
			buf->mem[j] = 0;
    	buf->tag = 0;
    	buf->valid = FALSE;
    	buf->chances = 0;
    	bufp->dirty[i] = FALSE;
    	//bufp->chances[i] = 0;
  	}

	return bufp;
}

int gamesdb_buf_flush_all(gamesdb* db) {
	//with the assumption that the db_store is clustered
	//a straight squential scan will cause ordered forward access to the db_store
	//this will be as fast as it gets
	int i;
	gamesdb_buffer* bufp = db->buffers;
	gamesdb_store* storep = db->store;
	
	for (i = 0; i < bufp->n_buf; i++) {
		if (bufp->dirty[i] == TRUE) {
			gamesdb_buf_write(bufp, i, storep);
		}
	}
	return 0;
}

//reads a page from disk
int gamesdb_buf_read(gamesdb_buffer* bufp, gamesdb_frameid spot, gamesdb_store* filep, gamesdb_pageid mytag) {
	gamesdb_bufferpage* buf = bufp->buffers + spot;
	//load in the new page
	gamesdb_read(filep, mytag, buf);

	//validate the entire page first
	buf->valid = TRUE;
	if (buf->tag != mytag)
		//the buffer is uninitialized, this means no record exists in the page
		buf->tag = mytag;

	if (DEBUG) {
		printf("buf_read: spot = %llu, buf_tag = %llu, mytag = %llu\n", spot, buf->tag, mytag);
	}
	
  	return 0;
}

//writes a page to disk
int gamesdb_buf_write(gamesdb_buffer* bufp, gamesdb_frameid spot, gamesdb_store* filep){
  //get the buffer page
  gamesdb_bufferpage* buf = bufp->buffers + spot;
  
  //we don't have to write the page if it's clean
  if(bufp->dirty[spot] == TRUE){
	//write will do the seek for you
	//no error checks
    buf->chances = 0;

    gamesdb_write(filep, buf->tag, buf);

    bufp->dirty[spot] = FALSE;
  }
  
  if (DEBUG) {
		printf("buf_write: spot = %llu, buf_tag = %llu\n", spot, buf->tag);
  }
  
  return 0;
}

int gamesdb_buf_destroy(gamesdb* db){
/*  frame_id i;
  
  for(i=0;i<bufp->n_buf;i++){
    SafeFree(bufp->buffers[i].mem);
  }
  */
  gamesdb_buffer* bufp = db->buffers;
  
  gamesdb_buf_flush_all(db);
  gamesdb_SafeFree(bufp->dirty);
  //gamesdb_SafeFree(bufp->chances);
  gamesdb_SafeFree(bufp->buffers);
//  SafeFree(bufp->buf_off);
//  db_close(bufp->filep);
  gamesdb_SafeFree(bufp);
  return 0; // no error checking;
}
