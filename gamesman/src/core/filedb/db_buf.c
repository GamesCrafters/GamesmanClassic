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

#include "db_types.h"
#include "db_buf.h"
#include "db_malloc.h"
#include "db_store.h"
#include <stdio.h>
#include <string.h>

// a buffer contains at least one record.
gamesdb_buffer* gamesdb_buf_init(int rec_size, gamesdb_pageid num_buf, gamesdb_store* filep){
  gamesdb_buffer* bufp = (gamesdb_buffer*) gamesdb_SafeMalloc(sizeof(gamesdb_buffer));
  
  bufp->filep = filep;
  bufp->n_buf = num_buf;
  
  //a page contains as many records as possible, up to 4K-9 bytes.
  //The 9 bytes are for book-keeping. One extra byte per record is used to denote whether it's
  //been written to or not
  //There will be wasted space in the mem array but shouldn't be too great if you have huge pages
  bufp->rec_size = rec_size + 1; //one byte for use by the db code (valid bit, etc)
  bufp->buf_size = MEM_ARRAY_SIZE / (rec_size+1);
  bufp->dirty = (gamesdb_boolean*) gamesdb_SafeMalloc (sizeof(gamesdb_boolean) * num_buf);
  bufp->buffers = (gamesdb_bufferpage*) gamesdb_SafeMalloc (sizeof(gamesdb_bufferpage) * num_buf);
  //bufp->buf_off = (db_offset*) SafeMalloc(sizeof(db_offset) * num_buf);

	//zeroes out the boffers for use
	gamesdb_pageid i,j;
	gamesdb_bufferpage *buf;
  for(i=0;i<num_buf;i++){
//    bufp->buffers[i]->mem = (char*) SafeMalloc(sizeof(char) * rec_size);
	buf = bufp->buffers+i;
	for (j=0; j<MEM_ARRAY_SIZE; j++)
		buf->mem[j] = 0;
    buf->tag = 0;
    buf->valid = FALSE;
    bufp->dirty[i] = FALSE;
  }

  return bufp;
}

//must be a valid page to enter this function
int gamesdb_buf_flush(gamesdb_buffer* bufp, gamesdb_pageid bufpage){
  //get the buffer page
  gamesdb_bufferpage* buf = bufp->buffers + bufpage;
  
  //we don't have to write the page if it's clean
  if(bufp->dirty[bufpage] == TRUE){
    gamesdb_store* filep = bufp->filep;

	//write will do the seek for you
	//no error checks
    gamesdb_write(filep, buf->tag, buf);

    //int i;
    //for(i = 0; i<mem_array_size; i++)
    //	buf->mem[i] = 0;
    bufp->dirty[bufpage] = FALSE;
    return 0;

  }else{
    return 0;
  }
}

int gamesdb_buf_flush_all(gamesdb_buffer* bufp) {
	//with the assumption that the db_store is clustered
	//a straight squential scan will cause ordered forward access to the db_store
	//this will be as fast as it gets
	int i;
	for (i = 0; i < bufp->n_buf; i++)
		gamesdb_buf_flush(bufp, i);
	return 0;
}

//reads a record, value will be NULL if the db does not have the record
int gamesdb_buf_read(gamesdb_buffer* bufp, gamesdb_position spot, void *value){
	//TODO: this should be in buf_mgr
	gamesdb_pageid page = spot / (bufp->buf_size); //the disk page index
	//the buffer page index (can be changed to do different associativities)
	gamesdb_pageid bufpage = page % bufp->n_buf;
	gamesdb_pageid mytag = page; //the page id (the tag to compare with)
	gamesdb_store* filep = bufp->filep;
	gamesdb_bufferpage* buf = bufp->buffers + bufpage;

	if (buf->valid == TRUE) {//if this page is not the one I want
		if (buf->tag != mytag) { //buffer page is valid but not the one we want
			//if (bufp->dirty[bufpage]) //if the page is dirty flush it
			gamesdb_buf_flush(bufp, bufpage);
			//load in the new page
			gamesdb_read(filep, page ,buf);
			//set the tag where it is
			if (buf->tag != mytag)
				//the buffer is uninitialized, this means no record exists in the page
				buf->tag = mytag;
			buf->valid = TRUE;
		}
	} else {
		//load in the new page
		gamesdb_read(filep, page ,buf);
		//set the tag where it is
		if (buf->tag != mytag)
			//the buffer is uninitialized, this means no record exists in the page
			buf->tag = mytag;
		buf->valid = TRUE;
	}
	
	if (DEBUG)
		printf("buf_read: spot = %llu, page = %llu, bufpage = %llu buf_tag = %llu, mytag = %llu\n", spot, page, bufpage, buf->tag, mytag);

	//byte offset of the db record (the extra byte + the actual record)
	gamesdb_offset off = (spot % (bufp->buf_size)) * bufp->rec_size;
	
	char valid;
	//copy the first byte and see if it is valid
	memcpy(&valid, buf->mem+off, 1);
	
	if(valid == TRUE) {
		memcpy(value, buf->mem+off+1, bufp->rec_size-1);
	} else {
		if (DEBUG)
			printf("gamesdb: buf_read missed.\n");
		memset(value, 0x00000000, bufp->rec_size-1); // 4 is undecided... OMG THIS IS BAD!
	}
	
  	return 0;
}

int gamesdb_buf_write(gamesdb_buffer* bufp, gamesdb_position spot, const void *value){
	gamesdb_pageid page = spot / (bufp->buf_size); //the disk page index
	//the buffer page index (can be changed to do different associativities)
	gamesdb_pageid bufpage = page % bufp->n_buf;
	gamesdb_pageid mytag = page; //the page id (the tag to compare with)
	
	gamesdb_store* filep = bufp->filep;

	gamesdb_bufferpage* buf = bufp->buffers + bufpage;

	if (buf->valid == TRUE) {//if this page is not the one I want
		if (buf->tag != mytag) { //buffer page is valid but not the one we want
			//if (bufp->dirty[bufpage]) //if the page is dirty flush it
			gamesdb_buf_flush(bufp, bufpage);
			//load in the new page
			gamesdb_read(filep, page ,buf);
			//set the tag where it is
			if (buf->tag != mytag)
				//the buffer is uninitialized, this means no record exists in the page
				buf->tag = mytag;
			buf->valid = TRUE;
		}
	} else {
		//load in the new page
		gamesdb_read(filep, page ,buf);
		//set the tag where it is
		if (buf->tag != mytag)
			//the buffer is uninitialized, this means no record exists in the page
			buf->tag = mytag;
		buf->valid = TRUE;
	}

	//byte offset of the db record (the extra byte + the actual record)
	gamesdb_offset off = (spot % (bufp->buf_size)) * bufp->rec_size;
	
	//actually write the record
	char valid = TRUE;
	memcpy(buf->mem+off, &valid, 1);
	memcpy(buf->mem+off+1, value, bufp->rec_size-1);
	
	bufp->dirty[bufpage] = TRUE;

  	return 0;
}

int gamesdb_buf_destroy(gamesdb_buffer* bufp){
/*  frame_id i;
  
  for(i=0;i<bufp->n_buf;i++){
    SafeFree(bufp->buffers[i].mem);
  }
  */
  gamesdb_buf_flush_all(bufp);
  gamesdb_SafeFree(bufp->dirty);
  gamesdb_SafeFree(bufp->buffers);
//  SafeFree(bufp->buf_off);
//  db_close(bufp->filep);
  gamesdb_SafeFree(bufp);
  return 0; // no error checking;
}
