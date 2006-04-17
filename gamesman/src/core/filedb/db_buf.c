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

//a direct-mapped cache of the db_store

#include "db_buf.h"
#include "db_malloc.h"
#include <stdio.h>
#include <string.h>

// a buffer contains at least one record. buf_size is the number of records in one buffer
db_buffer* db_buf_init(int rec_size, page_id num_buf, db_store* filep){
  db_buffer* bufp = (db_buffer*) SafeMalloc(sizeof(db_buffer));
  
  bufp->filep = filep;
  bufp->n_buf = num_buf;
  //a page contains as many records as possible, up to 4K bytes.
  //records with a size of multiples of 2 will produce no wasted space
  bufp->rec_size = rec_size;
  bufp->dirty = (boolean*) SafeMalloc (sizeof(boolean) * num_buf);
  bufp->buffers = (db_buffer_page*) SafeMalloc (sizeof(db_buffer_page) * num_buf);
  //bufp->buf_off = (db_offset*) SafeMalloc(sizeof(db_offset) * num_buf);

	page_id i,j;					  
  for(i=0;i<num_buf;i++){
//    bufp->buffers[i]->mem = (char*) SafeMalloc(sizeof(char) * rec_size);
	for (j=0; j<PAGE_SIZE; j++)
		bufp->buffers[i].mem[j] = 0;
    bufp->buffers[i].id = -1;
    bufp->buffers[i].valid = FALSE;
    bufp->dirty[i] = FALSE;
  }

  return bufp;
}

/*db_buf_head* db_buf_fileinit(db_store* filep){
  //Not going to read from file yet ok? deal with it.
  return NULL;
}*/
/*void db_buf_copyOut(db_buffer* bufp, char* mem,frame_id fid, int off, int amt){
  memcpy(mem,bufp->buffers[fid].mem+off,amt);
}

void db_buf_copyIn(db_buffer* bufp, char* mem,frame_id fid, int off, int amt){
  memcpy(bufp->buffers[fid].mem+off,mem,amt);
}
*/
int db_buf_flush(db_buffer* bufp, page_id page){
  //maybe we should write a flush all in here?
  // What logic do we need to use for fast (all) flushing?
  // --- see above
//  db_offset off;
  //get the frame
  db_buffer_page* buf = bufp->buffers + page;
  
  //we don't have to write the page if it's invalid or clean
  if(buf->valid && bufp->dirty[page]){
    db_store* filep = bufp->filep;
    
//    off = buf->off;
/*    if(buf->off == (db_offset) -1){
    	//this buffer is not on the disk file
      off = db_nextBlock(filep,(db_offset)bufp->buf_size);
      bufp->buf_off[buf->id] = off;
    }*/

	//    db_seek(filep,off,SEEK_SET);//maybe a seek write would be nice?
	//write will do the seek for you
    buf->id = db_write(filep, page, buf);//error check not in.
    //what if it isnt a forward seek?? rewrite seek to allow backwards
    // seeking on open files. (i.e. close and reopen the file... but how?)
    //seek will take care of this for you, no worries
    bufp->dirty[page] = FALSE;
    return 0;

  }else{
    return 0;
  }
}

int db_buf_flush_all(db_buffer* bufp) {
	//with the assumption that the db_store is clustered
	//a straight squential scan will cause ordered forward access to the db_store
	//this will be as fast as it gets
	int i;
	for (i = 0; i < bufp->n_buf; i++)
		db_buf_flush(bufp, i);
	return 0;
}

//reads a record, value will be NULL if the db does not have the record

int db_buf_read(db_buffer* bufp, Position spot, void *value){
	page_id page = spot / (PAGE_SIZE / bufp->rec_size);
	db_store* filep;

    db_buf_flush(bufp,page);
  
	db_buffer_page* buf = bufp->buffers + page;
	filep = bufp->filep;
    db_read(filep, page ,buf);

	if(bufp->buffers[page].valid) {
		db_offset off = (spot & (PAGE_SIZE * bufp->rec_size)) * bufp->rec_size;
	
		memcpy(value, bufp->buffers[page].mem+off, bufp->rec_size);
	} else
		value = NULL;
	
  	return 0;
}

int db_buf_write(db_buffer* bufp, Position spot, const void *value){
	page_id page = spot / (PAGE_SIZE / bufp->rec_size);
	db_store* filep;

    db_buf_flush(bufp,page);
	db_buffer_page* buf = bufp->buffers + page;
	filep = bufp->filep;
    db_read(filep, page ,buf);

	db_offset off = (spot & (PAGE_SIZE * bufp->rec_size)) * bufp->rec_size;
	
	memcpy(buf->mem+off, value, bufp->rec_size);
	
	buf->valid = TRUE;
	bufp->dirty[page] = TRUE;

  	return 0;
}

int db_buf_destroy(db_buffer* bufp){
/*  frame_id i;
  
  for(i=0;i<bufp->n_buf;i++){
    SafeFree(bufp->buffers[i].mem);
  }
  */
  SafeFree(bufp->dirty);
  SafeFree(bufp->buffers);
//  SafeFree(bufp->buf_off);
  db_close(bufp->filep);
  SafeFree(bufp);
  return 0; // no error checking;
}
