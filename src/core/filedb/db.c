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
#include <assert.h>

static gamesdb_frameid gamesdb_translate(gamesdb* db, gamesdb_pageid vpn) {
	//the thing it returns must be a frame identified by ppn in the physical buffer
	gamesdb_frameid ppn = gamesdb_bman_find(db, vpn); //see if it is in physical memory

	if (ppn == NULL) { //the page is not present in physical memory
		ppn = gamesdb_bman_replace(db, vpn); //get a replacement page, change page table in the process

		if (ppn->valid == GAMESDB_TRUE) {
			//this page got kicked out by n-chance
			if (GAMESDB_DEBUG) printf("translate: replaced = %u\n", (unsigned int)ppn);
			assert(ppn->tag != vpn);
			if (ppn->tag != vpn) {
				//buffer page is valid but not the one we want
				//if (bufp->dirty[bufpage]) //if the page is dirty flush it
				gamesdb_buf_write(db, ppn);
				//load in the new page
				gamesdb_buf_read(db, ppn, vpn);
			}
		} else {
			//load in the new page
			gamesdb_buf_read(db, ppn, vpn);
			//set the tag where it is
			//if (ppn->tag != vpn)
			//the buffer is uninitialized, this means no record exists in the page
		}
	}

	if (GAMESDB_DEBUG) {
		printf("translate: vpn = %llu, ppn = %u tag = %llu\n", vpn, (unsigned int)ppn, ppn->tag);
	}

	assert (ppn->tag == vpn);
	assert (ppn->valid == GAMESDB_TRUE);
	return ppn;
}

/*
 * allocates memory and sets up a gamescrafters db. Must call destructive
 * function when done to free up memory.
 */
gamesdb* gamesdb_create(int rec_size, gamesdb_pageid max_recs, gamesdb_pageid max_pages, int cluster_size, char* db_name){
	gamesdb_bman* bman;
	gamesdb_buffer* bufp;
	gamesdb_store* storep;
	gamesdb* data;

	bufp = gamesdb_buf_init(rec_size, max_recs, max_pages);
	bman = gamesdb_bman_init(bufp);

	data = (gamesdb*) gamesdb_SafeMalloc(sizeof(gamesdb));

	data->buf_man = bman;
	data->buffer = bufp;

	storep = gamesdb_open(data, db_name, cluster_size);
	if (storep == NULL) {
		printf("A fatal error occured when trying to open the database. Aborting.\n");
		exit(1);
	}
	data->store = storep;

	//create initial page
	gamesdb_buf_addpage(data);
	bman->clock_hand = bufp->pages;

	return data;
}

void gamesdb_destroy(gamesdb* data){

	gamesdb_bman_destroy(data->buf_man);
	gamesdb_buf_destroy(data);
	gamesdb_close(data->store);
	gamesdb_SafeFree(data);

}


void gamesdb_get(gamesdb* gdb, char* mem, gamesdb_position pos){
	gamesdb_buffer* bufp = gdb->buffer;

	gamesdb_pageid vpn = pos / (bufp->buf_size);

	gamesdb_frameid ppn = gamesdb_translate(gdb, vpn);

	//byte offset of the db record (the extra byte + the actual record)
	gamesdb_offset off = (pos % (bufp->buf_size)) * bufp->rec_size;

	memcpy(mem, ppn->mem+off, bufp->rec_size);
}


void gamesdb_put(gamesdb* gdb, char* mem, gamesdb_position pos){
	gamesdb_buffer* bufp = gdb->buffer;

	gamesdb_pageid vpn = pos / (bufp->buf_size);

	gamesdb_frameid ppn = gamesdb_translate(gdb, vpn);

	//byte offset of the db record (the extra byte + the actual record)
	gamesdb_offset off = (pos % (bufp->buf_size)) * bufp->rec_size;

	memcpy(ppn->mem+off, mem, bufp->rec_size);

	ppn->dirty = GAMESDB_TRUE;
	ppn->chances = 0;
}
