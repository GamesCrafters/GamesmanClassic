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
#include "db_types.h"
#include "db_bman.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

gamesdb_frameid gamesdb_translate(gamesdb* db, gamesdb_pageid vpn) {
	//the thing it returns must be a frame identified by ppn in the physical buffer
	gamesdb_frameid ppn = gamesdb_bman_find(db, vpn); //see if it is in physical memory
	
	if (ppn == -1) { //the page is not present in physical memory
		ppn = gamesdb_bman_replace(db, vpn); //get a replacement page, change TLB(the hash) in the process
		//flush the current page
		gamesdb_buffer* bufp = db->buffers;
		gamesdb_bufferpage* buf = bufp->buffers + ppn;
	
		if (buf->valid == TRUE) {
			//if this page is not the one I want
			if (buf->tag != vpn) {
				//buffer page is valid but not the one we want
				//if (bufp->dirty[bufpage]) //if the page is dirty flush it
				gamesdb_buf_write(db, ppn);
				//load in the new page
				gamesdb_buf_read(db, ppn, vpn);
				//set the tag where it is
				if (buf->tag != vpn)
					//the buffer is uninitialized, this means no records exists in the page
					buf->tag = vpn;
				buf->valid = TRUE;
			}
		} else {
			//load in the new page
			gamesdb_buf_read(db, ppn , vpn);
			//set the tag where it is
			if (buf->tag != vpn)
				//the buffer is uninitialized, this means no record exists in the page
				buf->tag = vpn;
			buf->valid = TRUE;
		}
	}
	
	if (DEBUG) {
		printf("translate: vpn = %llu, ppn = %llu\n", vpn, ppn);
	}
	
	return ppn;	
}

/*
 * allocates memory and sets up a gamescrafters db. Must call destructive
 * function when done to free up memory.
 */
gamesdb* gamesdb_create(int rec_size, gamesdb_pageid max_pages, int max_mem, int cluster_size, char* db_name){
  gamesdb_bman* bman;
  gamesdb_buffer* bufp;
  gamesdb* data;
  gamesdb_store* storep = gamesdb_open(db_name, cluster_size);

  bufp = gamesdb_buf_init(rec_size, max_pages, max_mem);

  bman = gamesdb_bman_init(bufp);

  data = (gamesdb*) gamesdb_SafeMalloc(sizeof(gamesdb));

  data->buf_man = bman;
  data->buffers = bufp;
  data->store = storep;
  data->num_page = max_pages;

  return data;
} 

void gamesdb_destroy(gamesdb* data){

  gamesdb_bman_destroy(data->buf_man);
  gamesdb_buf_destroy(data);
  gamesdb_close(data->store);
  gamesdb_SafeFree(data);

}

  
void gamesdb_get(gamesdb* gdb, char* mem, gamesdb_position pos){
	gamesdb_buffer* bufp = gdb->buffers;

	gamesdb_pageid vpn = pos / (bufp->buf_size);
	
	gamesdb_frameid ppn = gamesdb_translate(gdb, vpn);
	
	gamesdb_bufferpage* buf = bufp->buffers + ppn;

	//gamesdb_buf_read(gdb->buffers, ppn, mem);

	//byte offset of the db record (the extra byte + the actual record)
	gamesdb_offset off = (pos % (bufp->buf_size)) * bufp->rec_size;
	
	//char valid;
	//copy the first byte and see if it is valid
	//memcpy(&valid, buf->mem+off, 1);
	
	memcpy(mem, buf->mem+off, bufp->rec_size);
	
/*	if(mem[0] == ) {
		
	} else {
		if (DEBUG)
			printf("gamesdb: get() missed.\n");
		memset(mem, 0, bufp->rec_size);
	}
*/	
}


void gamesdb_put(gamesdb* gdb, char* mem, gamesdb_position pos){
	gamesdb_buffer* bufp = gdb->buffers;
	
	gamesdb_pageid vpn = pos / (bufp->buf_size);
	
	gamesdb_frameid ppn = gamesdb_translate(gdb, vpn); 
	
	//get the specific record from ppn;
	gamesdb_bufferpage* buf = bufp->buffers + ppn;

	//byte offset of the db record (the extra byte + the actual record)
	gamesdb_offset off = (pos % (bufp->buf_size)) * bufp->rec_size;
	
	//actually write the record
	//char valid = TRUE;
	//memcpy(buf->mem+off, &valid, 1);
	memcpy(buf->mem+off, mem, bufp->rec_size);
	
	buf->dirty = TRUE;
	buf->chances = 0; 
}
